#ifndef CRT_TRIANGLE_H
#define CRT_TRIANGLE_H

#include "CRTVector.h"

struct CRTTriangle {
    CRTVector v0, v1, v2;
    CRTVector edge0, edge1, edge2;
    CRTVector n;
    CRTVector n0, n1, n2;
    CRTVector uv0, uv1, uv2;         
    int materialIndex = 0;

    CRTTriangle(const CRTVector& a, const CRTVector& b, const CRTVector& c)
        : v0(a), v1(b), v2(c) {
        edge0 = v1 - v0; edge1 = v2 - v1; edge2 = v0 - v2;
        n = (v1 - v0).cross(v2 - v0).normalize();
        n0 = n; n1 = n; n2 = n;
    }

    CRTVector normal() const { return n; }

    float intersect(const CRTVector& rayOrigin, const CRTVector& rayDir) const {
        float rProj = n.dot(rayDir);
        if (rProj == 0.0f) return -1.0f;
        float t = n.dot(v0 - rayOrigin) / rProj;
        if (t < 0.0f) return -1.0f;
        CRTVector P = rayOrigin + rayDir * t;
        if (n.dot(edge0.cross(P - v0)) < 0.0f) return -1.0f;
        if (n.dot(edge1.cross(P - v1)) < 0.0f) return -1.0f;
        if (n.dot(edge2.cross(P - v2)) < 0.0f) return -1.0f;
        return t;
    }

    void barycentric(const CRTVector& P, float& u, float& v) const {
        CRTVector v0v1 = v1 - v0, v0v2 = v2 - v0, v0p = P - v0;
        float areaTri = v0v1.cross(v0v2).length();
        u = v0p.cross(v0v2).length() / areaTri;
        v = v0v1.cross(v0p).length() / areaTri;
    }

    CRTVector hitNormal(float u, float v, bool smooth) const {
        if (!smooth) return n;
        return (n1 * u + n2 * v + n0 * (1.0f - u - v)).normalize();
    }

    CRTVector interpUV(float u, float v) const {
        return uv1 * u + uv2 * v + uv0 * (1.0f - u - v);
    }
};

#endif // CRT_TRIANGLE_H