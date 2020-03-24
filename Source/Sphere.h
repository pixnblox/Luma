#pragma once

#include "Ray.h"
#include "Vec3.h"
#include "Scene.h"

namespace Luma {

// A sphere with a center and radius.
class Sphere : public Element
{
public:
    // Constructor.
    Sphere(const Vec3& center, float radius) : m_center(center), m_radius(radius) {}

    // Override's Element.Intersect().
    virtual bool intersect(const Ray& ray, Hit& hit) const override
    {
        // Compute the components needed to solve the quadratic equation, using the quadratic
        // formula: (-b ± √(b² - 4ac)) / 2a.
        // NOTE: Search online for "ray sphere intersection" for a complete derivation.
        Vec3 delta = ray.origin() - m_center;
        float a = dot(ray.direction(), ray.direction());
        float b = 2.0f * dot(ray.direction(), delta);
        float c = dot(delta, delta) - m_radius * m_radius;

        // Compute the discriminant. If the value is less than zero, then a square root is not
        // possible and the ray misses the sphere.
        float discriminant = b * b - 4 * a * c;
        if (discriminant < 0.0f)
        {
            return false;
        }

        // Evaluate the quadratic formula to get the closer possible intersection point (lower t).
        // If it exceeds the TMax value, the sphere is beyond the ray. If it is less than the TMin
        // value, try the farther possible intersection point (higher t). If this is still not in
        // the ray bounds, return false.
        float t = (-b - sqrt(discriminant)) / (2.0f * a);
        if (t > ray.tMax())
        {
            return false;
        }
        else if (t < ray.tMin())
        {
            t = (-b + sqrt(discriminant)) / (2.0f * a);
            if (t < ray.tMin() || t > ray.tMax())
            {
                return false;
            }
        }

        // The sphere was hit, so update the hit record with the t parameter, hit position, and
        // (normalized) normal at the hit position.
        hit.t = t;
        hit.position = ray.at(t);
        hit.normal = (hit.position - m_center) / m_radius;

        return true;
    }

private:
    Vec3 m_center;
    float m_radius;
};

} // namespace Luma
