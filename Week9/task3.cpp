#include <fstream>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "CRTScene.h"

static const int   maxColorComponent = 255;
static const float PI = 3.14159265358979f;
static const float SHADOW_BIAS = 1e-3f;
static const int   MAX_DEPTH = 5;

int toByte(float v) { if (v < 0) v = 0; if (v > 1) v = 1; return (int)(v * maxColorComponent); }
CRTVector mulc(const CRTVector& a, const CRTVector& b) { return CRTVector(a.x*b.x, a.y*b.y, a.z*b.z); }

int closestHit(const CRTScene& scene, const CRTVector& o, const CRTVector& d, float& outT) {
    float best = -1.0f; int idx = -1;
    for (int i = 0; i < (int)scene.triangles.size(); ++i) {
        float t = scene.triangles[i].intersect(o, d);
        if (t > 0.0f && (best < 0.0f || t < best)) { best = t; idx = i; }
    }
    outT = best; return idx;
}

bool inShadow(const CRTScene& scene, const CRTVector& origin, const CRTVector& L, float dist) {
    for (int i = 0; i < (int)scene.triangles.size(); ++i) {
        float t = scene.triangles[i].intersect(origin, L);
        if (t > 0.0f && t < dist) return true;
    }
    return false;
}

CRTVector shade(const CRTScene& scene, const CRTVector& P, const CRTVector& N, const CRTVector& albedo) {
    CRTVector shadowOrigin = P + N * SHADOW_BIAS;
    CRTVector col(0, 0, 0);
    for (size_t i = 0; i < scene.lights.size(); ++i) {
        CRTVector L = scene.lights[i].position - P;
        float sr = L.length();
        L = L.normalize();
        if (inShadow(scene, shadowOrigin, L, sr)) continue;
        float cosLaw = std::max(0.0f, N.dot(L));
        float sa = 4.0f * PI * sr * sr;
        col = col + albedo * (scene.lights[i].intensity / sa * cosLaw);
    }
    return col;
}

CRTVector trace(const CRTScene& scene, const CRTVector& origin, const CRTVector& dir, int depth) {
    CRTVector bg((float)scene.background.r/255, (float)scene.background.g/255, (float)scene.background.b/255);
    float t;
    int idx = closestHit(scene, origin, dir, t);
    if (idx < 0) return bg;

    const CRTTriangle& tri = scene.triangles[idx];
    const CRTMaterial& mat = scene.materials[tri.materialIndex];
    CRTVector P = origin + dir * t;
    float u, v; tri.barycentric(P, u, v);
    CRTVector N = tri.hitNormal(u, v, mat.smooth);

    if (mat.reflective) {
        if (depth >= MAX_DEPTH) return bg;
        CRTVector R = (dir - N * (2.0f * dir.dot(N))).normalize();
        return mulc(trace(scene, P + N * SHADOW_BIAS, R, depth + 1), mat.albedo);
    }
    return shade(scene, P, N, mat.albedo);
}

void renderScene(const std::string& sceneFile, const std::string& outFile) {
    CRTScene scene;
    if (!scene.load(sceneFile)) { std::cout << "skipped (not found): " << sceneFile << "\n"; return; }

    std::ofstream ppm(outFile, std::ios::out | std::ios::binary);
    ppm << "P3\n" << scene.width << " " << scene.height << "\n" << maxColorComponent << "\n";

    for (int rowIdx = 0; rowIdx < scene.height; ++rowIdx) {
        float y = 1.0f - 2.0f * (rowIdx + 0.5f) / scene.height;
        for (int colIdx = 0; colIdx < scene.width; ++colIdx) {
            float x = (2.0f * (colIdx + 0.5f) / scene.width - 1.0f) * scene.width / scene.height;
            CRTVector dir = scene.camera.rayDirection(CRTVector(x, y, -1.0f));
            CRTVector c = trace(scene, scene.camera.position, dir, 0);
            ppm << toByte(c.x) << " " << toByte(c.y) << " " << toByte(c.z) << "\t";
        }
        ppm << "\n";
    }
    ppm.close();
    std::cout << "rendered: " << outFile << "\n";
}

int main() {
    renderScene("scenes/scene4.crtscene", "out_scene4.ppm");
    renderScene("scenes/scene5.crtscene", "out_scene5.ppm");
    return 0;
}