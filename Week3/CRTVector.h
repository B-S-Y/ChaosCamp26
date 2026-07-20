#ifndef CRT_VECTOR_H
#define CRT_VECTOR_H

#include <cmath>


struct CRTVector {
    float x, y, z;

    
    CRTVector() : x(0.0f), y(0.0f), z(0.0f) {}

    
    CRTVector(float x, float y, float z) : x(x), y(y), z(z) {}

    //vector lenght
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    //normalize vector = length of vector = 1
    CRTVector normalize() const {
        float len = length();
        return CRTVector(x / len, y / len, z / len);
    }
};

#endif // CRT_VECTOR_H