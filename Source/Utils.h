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

// Generates a random 32-bit integer from a seed value, using an LCG.
//
// NOTE: Based on http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11.
uint32_t randomLCG(uint32_t seed)
{
    return 1664525 * seed + 1013904223;
}

// Generates a random 32-bit integer from a seed value, using XOR shifts.
//
// NOTE: Based on http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11.
uint32_t randomXORShift(uint32_t x)
{
    x ^= (x << 13);
    x ^= (x >> 17);
    x ^= (x << 5);

    return x;
}

// Hashes a 32-bit integer, which can be used to randomize a seed for an RNG, or directly as an RNG.
//
// NOTE: Based on http://www.reedbeta.com/blog/quick-and-easy-gpu-random-numbers-in-d3d11.
uint32_t wangHash(uint32_t x)
{
    x = (x ^ 61) ^ (x >> 16);
    x *= 9;
    x ^= x >> 4;
    x *= 0x27d4eb2d;
    x ^= x >> 15;

    return x;
}

// Computes the entry in the base 2 Halton sequence at the specified index.
//
// NOTE: Based on PBRT at https://github.com/mmp/pbrt-v3/blob/master/src/core/lowdiscrepancy.h.
inline float halton2(uint32_t index)
{
    index = (index << 16) | (index >> 16);
    index = ((index & 0x00ff00ff) << 8) | ((index & 0xff00ff00) >> 8);
    index = ((index & 0x0f0f0f0f) << 4) | ((index & 0xf0f0f0f0) >> 4);
    index = ((index & 0x33333333) << 2) | ((index & 0xcccccccc) >> 2);
    index = ((index & 0x55555555) << 1) | ((index & 0xaaaaaaaa) >> 1);

    return index * 1.0f / (1ull << 32);
}

// Computes the entry in the base 3 Halton sequence at the specified index.
inline float halton3(uint32_t index)
{
    float result = 0.0f;
    float scale = 1.0f;
    while (index != 0)
    {
        scale /= 3;
        result += (index % 3) * scale;
        index /= 3;
    }

    return result;
}

// Generates a uniformly distributed pseudorandom random number in the range [0.0, 1.0) using the
// Mersenne Twister. This gives very high quality numbers.
float randomMT()
{
    static std::mt19937 generator;
    static std::uniform_real_distribution<float> distribution;
    static std::function<float()> func = std::bind(distribution, generator);

    return func();
}

// Gets two uniformly distributed quasirandom numbers in the range [0.0, 1.0), using Halton (2,3)
// sequences with the specified index. 
//
// NOTE: The use of *quasirandom* (low discrepancy) numbers can substantially improve the rate of
// convergence for path tracing, compared to *pseudorandom* numbers. Try using MT nubmers here to
// see the difference. See PBRT and https://en.wikipedia.org/wiki/Halton_sequence for more
// information.
inline void getRandom2D(float& u1, float& u2, uint32_t& index)
{
    u1 = randomMT();
    u2 = randomMT();
}

// Generates a random direction in the cosine-weighted hemisphere above the specified normal. This
// provides a PDF value ("probability density function") which is the *relative* probability that
// the returned direction will be chosen.
Vec3 randomDirection(float u1, float u2, const Vec3& normal, float& pdf)
{
    // Create a point on the unit sphere, i.e. a direction, from the uniform random variables.
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
