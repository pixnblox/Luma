#pragma once

#include "Vec3.h"

namespace Luma {

// Ray with origin, direction, and min/max range.
class Ray
{
public:
    // Constructor.
    Ray(const Vec3& origin, const Vec3& direction, float tMin = 0.0f, float tMax = FLT_MAX) :
        m_origin(origin), m_direction(direction), m_tMin(tMin), m_tMax(tMax) {}

    // Returns the origin (start point) of the ray.
    const Vec3& Origin() const { return m_origin; }

    // Returns the direction of the ray.
    const Vec3& Direction() const { return m_direction; }

    // Returns the minimum distance from the origin for ray intersections.
    // NOTE: The distance is a multiple of the direction length.
    const float TMin() const { return m_tMin; }

    // Returns the maximum distance from the origin for ray intersections.
    // NOTE: The distance is a multiple of the direction length.
    const float TMax() const { return m_tMax; }

    // Computes a point along the ray at the specified distance.
    // NOTE: The distance is a multiple of the direction length.
    const Vec3 Point(float t) const { return m_origin + m_direction * t; }

private:
    Vec3 m_origin;
    Vec3 m_direction;
    float m_tMin;
    float m_tMax;
};

} // namespace Luma
