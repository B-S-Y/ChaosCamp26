#ifndef CRT_VECTOR_H
#define CRT_VECTOR_H

#include <cmath>


// New addition Week 5 :dot, scalar operator*
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

    
    CRTVector operator-(const CRTVector& o) const {
        return CRTVector(x - o.x, y - o.y, z - o.z);
    }

    CRTVector operator+(const CRTVector& o) const {
        return CRTVector(x + o.x, y + o.y, z + o.z);
    }

    CRTVector operator*(float s) const {
        return CRTVector(x * s, y * s, z * s);
    }
    CRTVector cross(const CRTVector& o) const {
        return CRTVector(
            y * o.z - z * o.y,
            z * o.x - x * o.z,
            x * o.y - y * o.x
        );
    }

    float dot(const CRTVector& o) const {
        return x * o.x + y * o.y + z * o.z;
    }
};

#endif 