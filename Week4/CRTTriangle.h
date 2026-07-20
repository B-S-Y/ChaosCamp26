#ifndef CRT_TRIANGLE_H
#define CRT_TRIANGLE_H
 
#include "CRTVector.h"
 //Task1

struct CRTTriangle {
    CRTVector v0, v1, v2;
 
    CRTTriangle(const CRTVector& a, const CRTVector& b, const CRTVector& c)
        : v0(a), v1(b), v2(c) {}
 
    // Unit normal vector.
    CRTVector normal() const {
        CRTVector e1 = v1 - v0;
        CRTVector e2 = v2 - v0;
        return e1.cross(e2).normalize();
    }
 
    // Triangle area =half of the parallelogram spanned by the two edges.
    float area() const {
        CRTVector e1 = v1 - v0;
        CRTVector e2 = v2 - v0;
        return e1.cross(e2).length() / 2.0f;
    }
};
 
#endif // CRT_TRIANGLE_H