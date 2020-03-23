#pragma once

namespace Luma {

// Linearly interpolate between two values.
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
    // NOTE: This is the technique used in "Ray Tracing in One Weekend."

    // Create a random point inside the unit hemisphere. This is done by using a unit cube, and
    // and rejecting any points outside the sphere.
    Vec3 offset;
    do
    {
        offset = 2.0f * Vec3(Random(), Random(), Random()) - Vec3(1.0f, 1.0f, 1.0f);
    }
    while (offset.Length() > 1.0f);

    // Offset the point by the normal to produce a direction.
    return (normal + offset).Normalize();
}

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
