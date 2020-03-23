#pragma once

namespace Luma {

// Vector with three components.
class Vec3
{
public:
    // Constructors.
    Vec3() {}
    Vec3(float x, float y, float z) { m_val[0] = x; m_val[1] = y; m_val[2] = z; }

    // Accessors.
    float x() const { return m_val[0]; }
    float y() const { return m_val[1]; }
    float z() const { return m_val[2]; }
    float r() const { return m_val[0]; }
    float g() const { return m_val[1]; }
    float b() const { return m_val[2]; }

    // Operator overloads.
    Vec3 operator-() const { return Vec3(-m_val[0], -m_val[1], -m_val[2]); }
    Vec3& operator+=(const Vec3& a) { m_val[0] += a.m_val[0]; m_val[1] += a.m_val[1]; m_val[2] += a.m_val[2]; return *this; }
    Vec3& operator-=(const Vec3& a) { m_val[0] -= a.m_val[0]; m_val[1] -= a.m_val[1]; m_val[2] -= a.m_val[2]; return *this; }
    Vec3& operator*=(float a) { m_val[0] *= a; m_val[1] *= a; m_val[2] *= a; return *this; }
    Vec3& operator/=(float a) { m_val[0] /= a; m_val[1] /= a; m_val[2] /= a; return *this; }

    // Computes the length of the vector.
    float Length() const { return sqrt(x() * x() + y() * y() + z() * z()); }

    // Normalizes the vector, i.e. with unit length.
    Vec3& Normalize() { *this /= Length(); return *this; }

    // Linearizes the vector (as a color) in the sRGB color space.
    // NOTE: Colors should be linearized for rendering computations to work correctly. Linearization
    // has the effect of darkening the color. See this chapter for "GPU Gems 3" for details:
    // https://developer.nvidia.com/gpugems/gpugems3/part-iv-image-effects/chapter-24-importance-being-linear
    Vec3& Linearize()
    {
        static const float LINEARIZE = 2.2f;
        m_val[0] = pow(m_val[0], LINEARIZE);
        m_val[1] = pow(m_val[1], LINEARIZE);
        m_val[2] = pow(m_val[2], LINEARIZE);
        return *this;
    }

    // Gamma corrects the vector (as a color) in the sRGB color space.
    // NOTE: Colors computed in rendering (linearized) should be gamma corrected immediately before
    // display or saving to most image file formats. Gamma correction has the effect of darkening
    // the color.
    Vec3& GammaCorrect()
    {
        static const float GAMMA = 1 / 2.2f;
        m_val[0] = pow(m_val[0], GAMMA);
        m_val[1] = pow(m_val[1], GAMMA);
        m_val[2] = pow(m_val[2], GAMMA);
        return *this;
    }

private:
    float m_val[3] = { 0.0f, 0.0f, 0.0f };
};

// Computes the dot product of two vectors.
inline float dot(const Vec3& a, const Vec3& b)
{
    return a.x() * b.x() + a.y() * b.y() + a.z() * b.z();
}

// Overloads the + operator for two vectors.
inline Vec3 operator+(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x() + b.x(), a.y() + b.y(), a.z() + b.z());
}

// Overloads the - operator for two vectors.
inline Vec3 operator-(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x() - b.x(), a.y() - b.y(), a.z() - b.z());
}

// Overloads the * operator for two vectors.
inline Vec3 operator*(const Vec3& a, const Vec3& b)
{
    return Vec3(a.x() * b.x(), a.y() * b.y(), a.z() * b.z());
}

// Overloads the * operator for a scalar and a vector.
inline Vec3 operator*(float a, const Vec3& b)
{
    return Vec3(a * b.x(), a * b.y(), a * b.z());
}

// Overloads the * operator for a vector and a scalar.
inline Vec3 operator*(const Vec3& a, float b)
{
    return Vec3(a.x() * b, a.y() * b, a.z() * b);
}

// Overloads the / operator for a vector and a scalar.
inline Vec3 operator/(const Vec3& a, float b)
{
    return Vec3(a.x() / b, a.y() / b, a.z() / b);
}

} // namespace Luma
