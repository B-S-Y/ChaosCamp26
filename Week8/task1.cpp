#include <fstream>
#include <string>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "CRTScene.h"

static const int maxColorComponent = 255;
static const float PI = 3.14159265358979f;
static const CRTVector albedo(0.5f, 0.5f, 0.5f);
static const float SHADOW_BIAS = 1e-3f;


int toByte(float v) {
    if (v < 0.0f) v = 0.0f;
    if (v > 1.0f) v = 1.0f;
    return static_cast<int>(v * maxColorComponent);
}


bool inShadow(const CRTVector& origin, const CRTVector& L, float lightDist,
              const CRTScene& scene) {
    for (size_t i = 0; i < scene.triangles.size(); ++i) {
        float t = scene.triangles[i].intersect(origin, L);
        if (t > 0.0f && t < lightDist) return true;   // something blocks the light
    }
    return false;
}

CRTColor shade(const CRTVector& P, const CRTVector& N, const CRTScene& scene) {
    CRTVector shadowOrigin = P + N * SHADOW_BIAS;

    float r = 0.0f, g = 0.0f, b = 0.0f;
    for (size_t i = 0; i < scene.lights.size(); ++i) {
        CRTVector L = scene.lights[i].position - P;   // direction to the light
        float sr = L.length();                         // sphere radius = distance
        L = L.normalize();

        if (inShadow(shadowOrigin, L, sr, scene)) continue;

        float cosLaw = std::max(0.0f, N.dot(L));       // facing the light -> brighter
        float sphereArea = 4.0f * PI * sr * sr;        // light spreads over a sphere
        float f = scene.lights[i].intensity / sphereArea * cosLaw;

        r += albedo.x * f;
        g += albedo.y * f;
        b += albedo.z * f;
    }
    return CRTColor(toByte(r), toByte(g), toByte(b));
}

void renderScene(const std::string& sceneFile, const std::string& outFile) {
    CRTScene scene;
    if (!scene.load(sceneFile)) {
        std::cout << "skipped (not found): " << sceneFile << "\n";
        return;
    }

    std::ofstream ppm(outFile, std::ios::out | std::ios::binary);
    ppm << "P3\n" << scene.width << " " << scene.height << "\n" << maxColorComponent << "\n";

    for (int rowIdx = 0; rowIdx < scene.height; ++rowIdx) {
        float y = (rowIdx + 0.5f) / scene.height;
        y = 1.0f - 2.0f * y;
        for (int colIdx = 0; colIdx < scene.width; ++colIdx) {
            float x = (colIdx + 0.5f) / scene.width;
            x = 2.0f * x - 1.0f;
            x *= static_cast<float>(scene.width) / scene.height;

            CRTVector rayDir = scene.camera.rayDirection(CRTVector(x, y, -1.0f));
            CRTVector rayOrigin = scene.camera.position;

            float closestT = -1.0f;
            int closestIdx = -1;
            for (int i = 0; i < static_cast<int>(scene.triangles.size()); ++i) {
                float t = scene.triangles[i].intersect(rayOrigin, rayDir);
                if (t > 0.0f && (closestT < 0.0f || t < closestT)) {
                    closestT = t;
                    closestIdx = i;
                }
            }

            CRTColor c;
            if (closestIdx >= 0) {
                CRTVector P = rayOrigin + rayDir * closestT;
                CRTVector N = scene.triangles[closestIdx].normal();
                c = shade(P, N, scene);
            } else {
                c = scene.background;
            }
            ppm << c.r << " " << c.g << " " << c.b << "\t";
        }
        ppm << "\n";
    }
    ppm.close();
    std::cout << "rendered: " << outFile << "\n";
}

int main() {
    for (int i = 0; i <= 3; ++i) {
        std::string sceneFile = "scenes/scene" + std::to_string(i) + ".crtscene";
        std::string outFile   = "out_scene" + std::to_string(i) + ".ppm";
        renderScene(sceneFile, outFile);
    }
    return 0;
}