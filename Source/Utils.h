#pragma once

namespace Luma {

// Linearly interpolates between two values.
template<class T>
inline T Lerp(T a, T b, float t)
{
    return a + t * (b - a);
}

// Generates a uniformly distributed random number in the range [0.0, 1.0).
float Random()
{
    static mt19937 generator;
    static uniform_real_distribution<float> distribution;
    static function<float()> func = bind(distribution, generator);

    return func();
}

// Generates a random direction in the hemisphere above the specified normal.
Vec3 RandomDirection(const Vec3& normal)
{
    // Create a random uniformly distributed point on the unit sphere, i.e. a direction.
    float u1 = Random();
    float u2 = Random();
    float theta = 2.0f * M_PI_F * u1;
    float phi = acos(1.0f - 2.0f * u2);
    Vec3 direction(
        sin(phi) * cos(theta),
        sin(phi) * sin(theta),
        cos(phi)
    );

    // Flip the direction if it is on the opposite side of the normal.
    if (dot(normal, direction) > 0.0)
    {
        return direction;
    }
    else
    {
        return -direction;
    }
}

// Reports the specified progress on the console, as a progress bar.
void UpdateProgress(float progress)
{
    const int PROGRESS_SIZE = 75;
    cout << "[";
    for (int i = 0; i < PROGRESS_SIZE; i++)
    {
        cout << (static_cast<float>(i) / PROGRESS_SIZE < progress ? "#" : " ");
    }
    cout << "] " << static_cast<int>(progress * 100.0f) << "%\r";
}

} // namespace Luma
