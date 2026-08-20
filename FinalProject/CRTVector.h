#ifndef CRT_VECTOR_H
#define CRT_VECTOR_H

#include <cmath>

// 3D (x, y, z) vector.
// Grows each week: Week 3 (length, normalize), Week 4 (operator-, cross),
// Week 5 (dot, operator+, scalar operator*).
struct CRTVector {
    float x, y, z;

    CRTVector() : x(0.0f), y(0.0f), z(0.0f) {}
    CRTVector(float x, float y, float z) : x(x), y(y), z(z) {}

    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    CRTVector normalize() const {
        float len = length();
        return CRTVector(x / len, y / len, z / len);
    }

    // Subtraction: e.g. edge = B - A
    CRTVector operator-(const CRTVector& o) const {
        return CRTVector(x - o.x, y - o.y, z - o.z);
    }

    // Addition: e.g. P = origin + something
    CRTVector operator+(const CRTVector& o) const {
        return CRTVector(x + o.x, y + o.y, z + o.z);
    }

    // Multiply by a scalar: e.g. rayDir * t
    CRTVector operator*(float s) const {
        return CRTVector(x * s, y * s, z * s);
    }

    // Cross product: perpendicular to both vectors.
    CRTVector cross(const CRTVector& o) const {
        return CRTVector(
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        );
    }

    // Dot product: a single number. Zero means the two vectors are perpendicular.
    float dot(const CRTVector& o) const {
        return x * o.x + y * o.y + z * o.z;
    }
};

#endif // CRT_VECTOR_H
