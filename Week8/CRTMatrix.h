#ifndef CRT_MATRIX_H
#define CRT_MATRIX_H

#include <cmath>
#include "CRTVector.h"

static const float CRT_PI = 3.14159265358979f;

// Turns degrees into radians
inline float toRadians(float degrees) {
    return degrees * CRT_PI / 180.0f;
}

// A 3x3 matrix, used to rotate vectors (camera orientation)
// m[row][col] 
struct CRTMatrix {
    float m[3][3];

    // Default the identity matrix (multiplying by it changes nothing)
    CRTMatrix() {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = (i == j) ? 1.0f : 0.0f;
    }

    // Matrix * vector -> a new, transformed vector.
    CRTVector mul(const CRTVector& v) const {
        return CRTVector(
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z
        );
    }

    // Matrix * matrix -> combine two rotations into one.
    CRTMatrix operator*(const CRTMatrix& o) const {
        CRTMatrix r;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) {
                r.m[i][j] = 0.0f;
                for (int k = 0; k < 3; ++k)
                    r.m[i][j] += m[i][k] * o.m[k][j];
            }
        return r;
    }

    //Rotation matrices

    // Around X axis -> tilt (pitch)
    static CRTMatrix rotationX(float a) {
        CRTMatrix r;
        float c = std::cos(a), s = std::sin(a);
        r.m[0][0] = 1; r.m[0][1] = 0; r.m[0][2] = 0;
        r.m[1][0] = 0; r.m[1][1] = c; r.m[1][2] = -s;
        r.m[2][0] = 0; r.m[2][1] = s; r.m[2][2] = c;
        return r;
    }

    // Around Y axis -> pan (yaw)
    static CRTMatrix rotationY(float a) {
        CRTMatrix r;
        float c = std::cos(a), s = std::sin(a);
        r.m[0][0] = c;  r.m[0][1] = 0; r.m[0][2] = s;
        r.m[1][0] = 0;  r.m[1][1] = 1; r.m[1][2] = 0;
        r.m[2][0] = -s; r.m[2][1] = 0; r.m[2][2] = c;
        return r;
    }

    // Around Z axis -> roll
    static CRTMatrix rotationZ(float a) {
        CRTMatrix r;
        float c = std::cos(a), s = std::sin(a);
        r.m[0][0] = c; r.m[0][1] = -s; r.m[0][2] = 0;
        r.m[1][0] = s; r.m[1][1] = c;  r.m[1][2] = 0;
        r.m[2][0] = 0; r.m[2][1] = 0;  r.m[2][2] = 1;
        return r;
    }
};

#endif // CRT_MATRIX_H