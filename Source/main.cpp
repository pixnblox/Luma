#include "pch.h"

#include "Camera.h"
#include "Image.h"
#include "Ray.h"
#include "Scene.h"
#include "Sphere.h"
#include "Vec3.h"
#include "Utils.h"
using namespace Luma;

#include <ppl.h>

// Computes the radiance incident along the specified ray, for the specified element.
Vec3 radiance(const Ray& ray, const Element& element, int depth, uint32_t& index)
{
    // If the trace depth has been exhausted, simply return black.
    if (depth == 0)
    {
        return Vec3();
    }

    // Intersect the sphere with the ray, and shade with the hit record if there was an
    // intersection. Otherwise shade with a (vertical) background gradient.
    Vec3 radiance;
    Hit hit;
    if (element.intersect(ray, hit))
    {
        // Generate a random direction in the hemisphere above the normal.
        float u1 = 0.0f, u2 = 0.0f;
        float pdf = 1.0f;
        getRandom2D(u1, u2, index);
        Vec3 direction = randomDirection(u1, u2, hit.normal, pdf);
        float cosTheta = dot(hit.normal, direction);
        assert(cosTheta > 0.0f);

        // Compute the Lambertian BRDF, i.e. the amount of light reflected by the material.
        static const Vec3 materialColor(Vec3(0.75f, 0.75f, 0.75f).sRGBToLinear());
        Vec3 brdf = materialColor / PI;

        // Compute the radiance incident from the direction, i.e. the incident light.
        //
        // NOTE: As this is recursive, this renders global illumination (indirect light) which is
        // very difficult to achieve with rasterization on GPUs.
        //
        // NOTE: A small ray offset is used to avoid self-intersection.
        static const float RAY_OFFSET = 1e-4f;
        Ray ray(hit.position, direction, RAY_OFFSET);
        Vec3 light = ::radiance(ray, element, depth - 1, index);

        // Compute the outgoing radiance, as defined by the rendering equation.
        radiance = brdf * light * cosTheta / pdf;

        // DIRECT LIGHTING: Uncomment this to perform simple direct shading and shadowing with a
        // directional light. As there is no random sampling, this will have no noise.
        //
        // Hit shadowHit;
        // static const Vec3 lightDirection(Vec3(1.0f, 1.0f, 1.0f).normalize());
        // Ray shadowRay(hit.position, lightDirection, RAY_OFFSET);
        // float visibility = element.intersect(shadowRay, shadowHit) ? 0.1f : 1.0f;
        // radiance = brdf * visibility * std::max(dot(hit.normal, lightDirection), 0.0f);

        // AMBIENT OCCLUSION: Uncomment this to render ambient occlusion, i.e. the amount by which a
        // point can see the environment.
        //
        // Vec3 visibility = element.intersect(ray, hit) ? Vec3() : Vec3(1.0f, 1.0f, 1.0f);
        // radiance = visibility * cosTheta / PI / pdf;

        // NORMALS: Uncomment this to render the surface normals as colors.
        //
        // radiance = (0.5f * (hit.normal + Vec3(1.0f, 1.0f, 1.0f))).Linearize();
    }
    else
    {
        static const Vec3 topColor(Vec3(0.5f, 0.7f, 1.0f).sRGBToLinear());
        static const Vec3 bottomColor(Vec3(1.0f, 1.0f, 1.0f).sRGBToLinear());

        float gradientFactor = (ray.direction().y() + 1.0f) * 0.5f;
        radiance = lerp(bottomColor, topColor, gradientFactor);
    }

    return radiance;
}

// Computes the radiance for all the pixels in the image buffer with the specified properties, using
// the specified element (scene) and camera.
void render(
    const Element& element, const Camera& camera, uint8_t* pImageData,
    uint16_t width, uint16_t height, uint16_t samples)
{
    // Report the rendering parameters.
    unsigned int threadCount = std::thread::hardware_concurrency();
    std::cout
        << "Rendering " << width << "x" << height
        << " at " << samples << " samples per pixel on "
        << threadCount << " threads..." << std::endl;

    // Record the start time.
    auto startTime = std::chrono::high_resolution_clock::now();
    auto prevTime = startTime;

    // Iterate the image pixels, starting from the top left (U = 0.0, Y = 1.0) corner, and computing
    // the incident radiance for each one. A parallel for loop is used here to support thread
    // concurrency.
    //
    // NOTE: Ray tracing is a naturally parallel algorithm: there is no read / write contention for
    // memory, with the exception of progress reporting.
    static const uint8_t NUM_COMPONENTS = 3;
    const size_t stride = width * NUM_COMPONENTS;
    std::mutex progressMutex;
    std::atomic<uint16_t> completedLines(0);
    Concurrency::parallel_for(uint16_t(0), height, [&](uint16_t line)
    {
        // Get a pointer to the start of the current line.
        uint16_t y = height - line - 1;
        uint8_t* pPixel = pImageData + stride * line;

        // Iterate the pixels of line, computing radiance for each one.
        for (uint16_t x = 0; x < width; x++)
        {
            // Create an index for a sequence of *quasirandom* numbers. Such numbers are used for
            // "random" sampling while path tracing, e.g. selecting a random direction in a
            // hemisphere. The sequence index starts with a unique value for each pixel in the image
            // which is then randomized with a hash.
            //
            // NOTE: Using a constant sequence index leads to total aliasing, but will still
            // converge to the correct result with enough samples. Using only the unique per-pixel
            // starting index will reduce aliasing, but still yields substantial correlation
            // artifacts. Finally, hashing that index yields less objectionable noise, but still
            // with better convergence than using *pseudorandom* numbers.
            // 
            // IMPORTANT: For now the same index is used for all random numbers in this pixel
            // sample. This strangely works quite well, but will likely need to be revisited. The
            // index is only incremented once, when the sample is complete.
            uint32_t sequenceIndex = 0;
            sequenceIndex = samples * (line * height + x);
            sequenceIndex = wangHash(sequenceIndex);

            // Accumulate radiance samples for each pixel.
            Vec3 radiance;
            for (uint16_t sample = 0; sample < samples; sample++)
            {
                // Compute the sample position, using a random offset for each sample. If only one
                // sample is being taken, use the pixel center.
                //
                // NOTE: This uses pseudorandom (MT) numbers because using the quasirandom sequence
                // with the same index as the radiance sampling yields minor edge artifacts.
                float rand_x = samples == 1 ? 0.5f : randomMT();
                float rand_y = samples == 1 ? 0.5f : randomMT();
                float u = (x + rand_x) / width;
                float v = (y - rand_y) / height;

                // Compute a camera ray direction based on the current pixel's UV coordinates.
                Ray ray = camera.getRay(u, v);

                // Compute a color for the ray, i.e. the scene radiance from that direction and add
                // it to the accumulated radiance.
                static const int MAX_DEPTH = 10;
                radiance += ::radiance(ray, element, MAX_DEPTH, sequenceIndex);

                // Increment the sequence index, for the next sample.
                //
                // NOTE: See the "IMPORTANT" note above.
                sequenceIndex++;
            }

            // Compute the average of the radiance samples to yield the pixel radiance.
            radiance /= samples;

            // Gamma correct the radiance and store it in the image buffer.
            radiance.linearTosRGB();
            const float COMPONENT_SCALE = 255.99f;
            uint8_t color[] =
            {
                static_cast<uint8_t>(clamp(radiance.r(), 0.0f, 1.0f) * COMPONENT_SCALE),
                static_cast<uint8_t>(clamp(radiance.g(), 0.0f, 1.0f) * COMPONENT_SCALE),
                static_cast<uint8_t>(clamp(radiance.b(), 0.0f, 1.0f) * COMPONENT_SCALE),
            };
            ::memcpy(pPixel, color, 3);
            pPixel += 3;
        }

        // Increment the (atomic) number of completed lines.
        completedLines++;

        // Update the progress if more than one second has elapsed since the last update.
        //
        // NOTE: A mutex is used to avoid a race condition with multiple threads.
        auto nextTime = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(nextTime - prevTime).count();
        if (elapsed >= 1)
        {
            progressMutex.lock();
            float progress = static_cast<float>(completedLines) / height;
            updateProgress(progress);
            prevTime = nextTime;
            progressMutex.unlock();
        }
    });

    // Finish progress updates.
    ::updateProgress(1.0f);
    std::cout << std::endl;

    // Report the image dimensions and time spent rendering.
    auto endTime = std::chrono::high_resolution_clock::now();
    auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout
        << std::setprecision(3)
        << "Completed in " << elapsedTime / 1000.0f << " seconds." << std::endl;
}

// Main entry point.
int main()
{
    // Enable memory leak detection. This will output a memory leak report when the process exits,
    // if there are any detected memory leaks. The report starts with "Detected memory leaks!"
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Create scene geometry.
    auto pCenter = make_shared<Sphere>(Vec3(0.0f, 0.0f, -1.0f), 0.5f);
    auto pGround = make_shared<Sphere>(Vec3(0.0f, -100.5f, -1.0f), 100.0f);
    Scene scene;
    scene.add(pCenter);
    scene.add(pGround);

    // Create the output image.
    //
    // NOTE: The image can be rendered at a lower resolution and scaled up to the desired image
    // size to make it easier to see the individual pixels and for faster rendering. The settings
    // here take about 4.2 seconds to render with a Debug build, but only about 400 ms with a
    // Release build on a Core i7-8700 CPU with 12 threads.
    static const uint8_t SCALE = 16;
    static const uint16_t OUTPUT_WIDTH = 3840;
    static const uint16_t OUTPUT_HEIGHT = 2160;
    static const uint16_t WIDTH = OUTPUT_WIDTH / SCALE;
    static const uint16_t HEIGHT = OUTPUT_HEIGHT / SCALE;
    static const uint16_t SPP = 16;
    Image image(WIDTH, HEIGHT);

    // Create a camera.
    // TODO: This will eventually accept typical camera properties: position, direction, FOV, etc.
    Camera camera(static_cast<float>(WIDTH) / HEIGHT);

    // Render the scene with the camera, to the image buffer with the specified properties.
    ::render(scene, camera, image.getImageData(), WIDTH, HEIGHT, SPP);

    // Save the image.
    image.savePNG("output.png", SCALE);
}
