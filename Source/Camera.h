#pragma once

#include "Ray.h"
#include "Vec3.h"

namespace Luma {

// A camera for generating primary rays for rendering.
class Camera
{
public:
    // Constructor.
    Camera(float aspect) : m_aspect(aspect) {}

    // Computes a ray from the camera with the specified U (horizontal) and V (vertical) offsets in
    // the camera image plane.
    Ray GetRay(float u, float v) const
    {
        // Define the camera with an eye position at the origin, and facing the -Z axis.
        // TODO: The camera is currently hardcoded, but will later support standard properties.
        static Vec3 origin(0.0f, 0.0f, 0.0f);
        static Vec3 start(-m_aspect, -1.0f, -1.0f);
        static float viewWidth = m_aspect * 2.0f;
        static float viewHeight = 2.0f;

        // Prepare a ray direction offset from the start (lower left) corner of the image plane.
        Vec3 direction(
            start.x() + u * viewWidth,
            start.y() + v * viewHeight,
            start.z());
        direction.Normalize();
        
        // Return a ray with the direction and the camera's origin.
        return Ray(origin, direction);
    }

private:
    float m_aspect;
};

} // namespace Luma
