#ifndef CRT_VECTOR_H
#define CRT_VECTOR_H

#include <cmath>

// 3D (x, y,z) vector.
// This file is carried over from Week 3 and extended with subtraction and cross product for Week 4.
struct CRTVector {
    float x, y, z;

    CRTVector() : x(0.0f), y(0.0f), z(0.0f) {}
    CRTVector(float x, float y, float z) : x(x), y(y), z(z) {}

    // Length: sqrt(x^2 + y^2 + z^2)
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Unit vector= same direction, length 1.
    CRTVector normalize() const {
        float len = length();
        return CRTVector(x / len, y / len, z / len);
    }
    // Vector subtraction. Used to build triangle edges, e.g. edge = B - A.
    CRTVector operator-(const CRTVector& o) const {
        return CRTVector(x - o.x, y - o.y, z - o.z);
    }

    // Cross product (this x other). The result is perpendicular to both vectors,
    // and its length equals the area of the parallelogram they span.
    CRTVector cross(const CRTVector& o) const {
        return CRTVector(
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        );
    }
};

#endif 