// Textures: the material albedo is read from a texture (albedo / edges / checker / bitmap).
// Define the stb_image implementation once, here, before CRTScene.h pulls in the header.
#define STB_IMAGE_IMPLEMENTATION

#include <fstream>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "CRTScene.h"

static const int   maxColorComponent = 255;
static const float PI = 3.14159265358979f;
static const float SHADOW_BIAS = 1e-3f;

int toByte(float v) { if (!(v == v)) v = 0; if (v < 0) v = 0; if (v > 1) v = 1; return (int)(v * maxColorComponent); }

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

// Diffuse shading with the albedo that was sampled from the texture.
CRTVector shade(const CRTScene& scene, const CRTVector& P, const CRTVector& N, const CRTVector& albedo) {
    CRTVector shadowOrigin = P + N * SHADOW_BIAS;
    CRTVector col(0, 0, 0);
    for (size_t i = 0; i < scene.lights.size(); ++i) {
        CRTVector L = scene.lights[i].position - P;
        float sr = L.length(); L = L.normalize();
        if (inShadow(scene, shadowOrigin, L, sr)) continue;
        float cosLaw = std::max(0.0f, N.dot(L));
        col = col + albedo * (scene.lights[i].intensity / (4.0f * PI * sr * sr) * cosLaw);
    }
    return col;
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

            float t;
            int idx = closestHit(scene, scene.camera.position, dir, t);
            CRTVector c;
            if (idx < 0) {
                c = CRTVector((float)scene.background.r/255, (float)scene.background.g/255, (float)scene.background.b/255);
            } else {
                const CRTTriangle& tri = scene.triangles[idx];
                const CRTMaterial& m = scene.materials[tri.materialIndex];
                CRTVector P = scene.camera.position + dir * t;
                float bu, bv; tri.barycentric(P, bu, bv);

                // Albedo comes from the material's texture (or a constant fallback).
                CRTVector albedo = m.albedo;
                if (m.textureIndex >= 0)
                    albedo = scene.textures[m.textureIndex].sample(bu, bv, tri.interpUV(bu, bv));

                CRTVector N = tri.hitNormal(bu, bv, m.smooth);
                c = shade(scene, P, N, albedo);
            }
            ppm << toByte(c.x) << " " << toByte(c.y) << " " << toByte(c.z) << "\t";
        }
        ppm << "\n";
    }
    ppm.close();
    std::cout << "rendered: " << outFile << "\n";
}

int main() {
    // Scene files and the textures/ folder live in the same directory.
    for (int i = 0; i <= 4; ++i)
        renderScene("scenes/scene" + std::to_string(i) + ".crtscene",
                    "out_scene" + std::to_string(i) + ".ppm");
    return 0;
}