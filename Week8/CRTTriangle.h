#ifndef CRT_TRIANGLE_H
#define CRT_TRIANGLE_H

#include "CRTVector.h"

//A triangle defined by 3 vertices. Vertex order matters (decides the normal direction).
struct CRTTriangle {
    CRTVector v0, v1, v2;
    CRTVector edge0, edge1, edge2;   
    CRTVector n; 

    CRTTriangle(const CRTVector& a, const CRTVector& b, const CRTVector& c)
        : v0(a), v1(b), v2(c) {
        edge0 = v1 - v0;
        edge1 = v2 - v1;
        edge2 = v0 - v2;
        n = (v1 - v0).cross(v2 - v0).normalize();
    }

    // Unit normal of the triangle's plane.
    CRTVector normal() const { return n; }

    // Ray-triangle intersection.
    // Returns the distance t along the ray to the hit point (t > 0),
    // or -1 if the ray does NOT hit this triangle.
    float intersect(const CRTVector& rayOrigin, const CRTVector& rayDir) const {
        CRTVector N = normal();

        // 1) Is the ray parallel to the plane? Then it never hits.
        float rProj = N.dot(rayDir);
        if (rProj == 0.0f) return -1.0f;

        // 2) Distance along the ray until it reaches the plane.
        float t = N.dot(v0 - rayOrigin) / rProj;
        if (t < 0.0f) return -1.0f;             // plane is behind the ray

        // 3) The point where the ray meets the plane.
        CRTVector P = rayOrigin + rayDir * t;

        // 4) Is P inside the triangle? It must be on the left of all 3 edges.
        if (N.dot(edge0.cross(P - v0)) < 0.0f) return -1.0f;
        if (N.dot(edge1.cross(P - v1)) < 0.0f) return -1.0f;
        if (N.dot(edge2.cross(P - v2)) < 0.0f) return -1.0f;

        return t;  
    }
};

#endif // CRT_TRIANGLE_H