#pragma once

#include "Utils.h"

namespace Luma {

// A structure storing the data for a hit (ray-element intersection).
struct Hit
{
    float t;
    Vec3 position;
    Vec3 normal;
};

// An interface for any element that can be intersected by a ray.
class Element
{
public:
    // Intersects the ray with the element, returns whether an intersection was found. If so, the
    // hit value is update with properties of the intersection.
    virtual bool Intersect(const Ray& ray, Hit& hit) const = 0;
};

// A scene consisting of multiple elements suitable for rendering.
class Scene : public Element
{
public:
    // Adds an element to the scene.
    void Add(shared_ptr<Element> pElement)
    {
        m_elements.push_back(pElement);
    }

    // Overrides Element.Intersect().
    virtual bool Intersect(const Ray& ray, Hit& hit) const override
    {
        // Initialize the closest hit with the rays TMax value.
        bool anyHit = false;
        Hit closestHit;
        closestHit.t = ray.TMax();

        // Iterate the elements, finding the closest intersection with the ray.
        for (auto element : m_elements)
        {
            // If the ray intersects the element, and the hit is closer that the closest one so far,
            // record it as the closest hit.
            Hit nextHit;
            if (element->Intersect(ray, nextHit) && nextHit.t < closestHit.t)
            {
                anyHit = true;
                closestHit = nextHit;
            }
        }

        // If there was a hit, record that for the caller.
        if (anyHit)
        {
            assert(closestHit.t <= ray.TMax());
            hit = closestHit;
        }

        return anyHit;
    }

private:
    vector<shared_ptr<Element>> m_elements;
};

} // namespace Luma