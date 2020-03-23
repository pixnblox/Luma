#include "pch.h"

#include "Camera.h"
#include "Image.h"
#include "Ray.h"
#include "Scene.h"
#include "Sphere.h"
#include "Vec3.h"
#include "Utils.h"
using namespace Luma;

// Computes the radiance incident along the specified ray, for the specified element.
Vec3 Radiance(const Ray& ray, const Element& element)
{
    // Intersect the sphere with the ray, and shade with the hit record if there was an
    // intersection. Otherwise shade with a (vertical) background gradient.
    Vec3 radiance;
    Hit hit;
    if (element.Intersect(ray, hit))
    {
        // Generate a random direction in the hemisphere above the normal.
        Vec3 direction = RandomDirection(hit.normal);

        // Compute the radiance incident from the random direction, and scale by the material color.
        // This renders global illumination (bounced light) which is very difficult to achieve with
        // rasterization on GPUs.
        // NOTE: This is recursive, but will eventually terminate when a ray misses geometry, at
        // least for the geometry used here. Normally the tracing should be terminated after a
        // certain recursion level. A ray offset is used to avoid self-intersection.
        static const float RAY_OFFSET = 1e-4f;
        static const Vec3 materialColor(Vec3(0.75f, 0.75f, 0.75f).Linearize());
        radiance = materialColor * Radiance(Ray(hit.position, direction, RAY_OFFSET), element);

        // AMBIENT OCCLUSION: Uncomment this to render ambient occlusion, i.e. the amount by which a
        // point can see the environment.
        // radiance = element.Intersect(Ray(hit.position, direction, RAY_OFFSET), hit) ? Vec3(0.0f, 0.0f, 0.0f) : Vec3(1.0f, 1.0f, 1.0f);

        // DIRECT LIGHTING: Uncomment this to perform simple direct shading and shadowing with a
        // directional light.
        // Hit shadowHit;
        // static const Vec3 lightDirection(Vec3(1.0f, 1.0f, 1.0f).Normalize());
        // bool visible = element.Intersect(Ray(hit.position, lightDirection, RAY_OFFSET), shadowHit);
        // radiance = materialColor * max(dot(hit.normal, lightDirection), 0.0f);
        // radiance *= visible ? 0.1f : 1.0f;

        // NORMALS: Uncomment this to render the surface normals as colors.
        // radiance = (0.5f * (hit.normal + Vec3(1.0f, 1.0f, 1.0f))).Linearize();
    }
    else
    {
        static const Vec3 topColor(Vec3(0.5f, 0.7f, 1.0f).Linearize());
        static const Vec3 bottomColor(Vec3(1.0f, 1.0f, 1.0f).Linearize());

        float gradientFactor = (ray.Direction().y() + 1.0f) * 0.5f;
        radiance = Lerp(bottomColor, topColor, gradientFactor);
    }

    return radiance;
}

// Computes the radiance for all the pixels in the image buffer with the specified properties, using
// the specified element (scene) and camera.
void Render(
    const Element& element, const Camera& camera, uint8_t* pImageData,
    uint16_t width, uint16_t height, uint16_t samples)
{
    // Report the rendering parameters.
    cout
        << "Rendering " << width << "x" << height
        << " at " << samples << " samples per pixel..." << endl;

    // Record the start time.
    auto startTime = chrono::high_resolution_clock::now();
    auto prevTime = startTime;

    // Iterate the image pixels, starting from the top left (U = 0.0, Y = 1.0) corner, and computing
    // the incident radiance for each one.
    uint8_t* pPixel = pImageData;
    for (uint16_t y = height; y > 0; y--)
    {
        for (uint16_t x = 0; x < width; x++)
        {
            // Accumulate radiance samples for each pixel.
            Vec3 radiance;
            for (uint16_t sample = 0; sample < samples; sample++)
            {
                // Compute the sample position, using a random offset for each sample.
                // NOTE: If only one sample is being taken, use the pixel center.
                float rand_x = samples == 1 ? 0.5f : Random();
                float rand_y = samples == 1 ? 0.5f : Random();
                float u = (x + rand_x) / width;
                float v = (y - rand_y) / height;

                // Compute a camera ray direction based on the current pixel's UV coordinates.
                Ray ray = camera.GetRay(u, v);

                // Compute a color for the ray, i.e. the scene radiance from that direction and add
                // it to the accumulated radiance.
                radiance += ::Radiance(ray, element);
            }

            // Compute the average of the radiance samples to yield the pixel radiance.
            radiance /= samples;

            // Gamma correct the radiance and store it in the image buffer.
            radiance.GammaCorrect();
            const float COMPONENT_SCALE = 255.99f;
            *pPixel++ = static_cast<uint8_t>(radiance.r() * COMPONENT_SCALE);
            *pPixel++ = static_cast<uint8_t>(radiance.g() * COMPONENT_SCALE);
            *pPixel++ = static_cast<uint8_t>(radiance.b() * COMPONENT_SCALE);
        }

        // Update the progress if more than one second has elapsed since the last update.
        auto nextTime = chrono::high_resolution_clock::now();
        auto elapsed = chrono::duration_cast<chrono::seconds>(nextTime - prevTime).count();
        if (elapsed >= 1)
        {
            float progress = static_cast<float>(height - y) / height;
            UpdateProgress(progress);
            prevTime = nextTime;
        }
    }

    // Finish progress updates.
    UpdateProgress(1.0f);
    cout << endl;

    // Report the image dimensions and time spent rendering.
    auto endTime = chrono::high_resolution_clock::now();
    auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    cout << setprecision(3) << "Completed in " << elapsedTime / 1000.0f << " seconds." << endl;
}

// Main entry point.
int main()
{
    // Enable memory leak detection. This will output a memory leak report when the process exits,
    // if there are any detected memory leaks. The report starts with "Detected memory leaks!"
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    // Create scene geometry.
    auto pSphere = make_shared<Sphere>(Vec3(0.0f, 0.0f, -1.0f), 0.5f);
    auto pGround = make_shared<Sphere>(Vec3(0.0f, -100.5f, -1.0f), 100.0f);
    Scene scene;
    scene.Add(pSphere);
    scene.Add(pGround);

    // Create the output image.
    // NOTE: The image can be rendered at a lower resolution and scaled up to the desired image
    // size to make it easier to see the individual pixels and for faster rendering. The settings
    // here take about 5 seconds to render with a Debug build, but only about 200 ms with a Release
    // build on a Core i7-8700 CPU.
    const uint8_t SCALE = 16;
    const uint16_t OUTPUT_WIDTH = 3840;
    const uint16_t OUTPUT_HEIGHT = 2160;
    const uint16_t WIDTH = OUTPUT_WIDTH / SCALE;
    const uint16_t HEIGHT = OUTPUT_HEIGHT / SCALE;
    const uint16_t SPP = 16;
    Image image(WIDTH, HEIGHT);

    // Create a camera.
    // TODO: This will eventually accept typical camera properties: position, direction, FOV, etc.
    Camera camera(static_cast<float>(WIDTH) / HEIGHT);

    // Render the scene with the camera, to the image buffer with the specified properties.
    Render(scene, camera, image.GetImageData(), WIDTH, HEIGHT, SPP);

    // Save the image.
    image.SavePNG("output.png", SCALE);
}
