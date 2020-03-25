#pragma once

namespace Luma {

// Linearly interpolates between two values.
template<class T>
inline T lerp(T a, T b, float t)
{
    return a + t * (b - a);
}

// Clamps a value to the specified range.
template<class T>
inline T clamp(T x, T min, T max)
{
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

// Generates a uniformly distributed random number in the range [0.0, 1.0).
float random()
{
    static std::mt19937 generator;
    static std::uniform_real_distribution<float> distribution;
    static std::function<float()> func = std::bind(distribution, generator);

    return func();
}

// Generates a random direction in the cosine-weighted hemisphere above the specified normal. This
// provides a PDF value ("probability density function") which is the *relative* probability that
// the returned direction will be chosen.
Vec3 randomDirection(const Vec3& normal, float& pdf)
{
    // Create a random uniformly distributed point on the unit sphere, i.e. a direction.
    float u1 = random();
    float u2 = random();
    float cosTheta = 1.0f - 2.0f * u2;
    float sinTheta = sqrt(1.0f - cosTheta * cosTheta);
    float phi = 2.0f * PI * u1;
    Vec3 direction(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);

    // To transform that into a sample from a cosine-weighted hemisphere over the normal, treat the
    // sphere as tangent to the surface: add the normal to the direction and normalize it. The PDF
    // is cos(theta) / PI, so use a dot product to compute cos(theta). See "Ray Tracing in One
    // Weekend" for details.
    direction = (normal + direction).normalize();
    pdf = dot(normal, direction) / PI;

    // To test a uniform hemisphere instead, comment the block above and uncomment the block below.
    // This simply flips the uniform sphere direction if it is on the opposite side of the normal.
    // While this is still correct, for cosine-weighted operations like ambient occlusion or
    // computing outgoing radiance, this requires about twice as many samples to achieve the same
    // level of variance (noise) as the cosine-weighted direction computed above. See "Ray Tracing
    // Gems" chapter 15 for details.
    //
    // if (Dot(normal, direction) < 0.0)
    // {
    //     direction = -direction;
    // }
    // pdf = 1.0f / (2.0f * M_PI_F);

    return direction;
}

// Reports the specified progress on the console, as a progress bar.
void updateProgress(float progress)
{
    const int PROGRESS_SIZE = 75;
    std::cout << "[";
    for (int i = 0; i < PROGRESS_SIZE; i++)
    {
        std::cout << (static_cast<float>(i) / PROGRESS_SIZE < progress ? "#" : " ");
    }
    std::cout << "] " << static_cast<int>(progress * 100.0f) << "%\r" << std::flush;
}

} // namespace Luma
