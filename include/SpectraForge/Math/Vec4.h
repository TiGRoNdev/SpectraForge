/**
 * @file Vec4.h
 * @brief Simple 4D vector implementation for SpectraForge
 */

#pragma once

#include <cmath>
#include <iostream>

namespace SpectraForge {
namespace Math {

/**
 * @brief Simple 4D vector class (replacement for GLM)
 */
class Vec4 {
public:
    float x, y, z, w;

    // Constructors
    Vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
    Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    Vec4(float scalar) : x(scalar), y(scalar), z(scalar), w(scalar) {}

    // Basic operations
    Vec4 operator+(const Vec4& other) const {
        return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    Vec4 operator-(const Vec4& other) const {
        return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    Vec4 operator*(float scalar) const {
        return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    Vec4 operator/(float scalar) const {
        return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
    }

    // Length and normalization
    float length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    Vec4 normalized() const {
        float len = length();
        if (len > 0.0f) {
            return *this / len;
        }
        return *this;
    }

    // Dot product
    float dot(const Vec4& other) const {
        return x * other.x + y * other.y + z * other.z + w * other.w;
    }
};

// Utility functions
inline Vec4 operator*(float scalar, const Vec4& vec) {
    return vec * scalar;
}

} // namespace Math
} // namespace SpectraForge
