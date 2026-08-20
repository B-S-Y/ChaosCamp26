#include <fstream>
#include <string>
#include <iostream>
#include "CRTScene.h"

static const int maxColorComponent = 255;

int toByte(float v) { if (v < 0) v = 0; if (v > 1) v = 1; return (int)(v * maxColorComponent); }

int closestHit(const CRTScene& scene, const CRTVector& o, const CRTVector& d, float& outT) {
    float best = -1.0f; int idx = -1;
    for (int i = 0; i < (int)scene.triangles.size(); ++i) {
        float t = scene.triangles[i].intersect(o, d);
        if (t > 0.0f && (best < 0.0f || t < best)) { best = t; idx = i; }
    }
    outT = best;
    return idx;
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
            if (idx < 0) {
                ppm << scene.background.r << " " << scene.background.g << " " << scene.background.b << "\t";
                continue;
            }
            CRTVector P = scene.camera.position + dir * t;
            float u, v;
            scene.triangles[idx].barycentric(P, u, v);
            ppm << toByte(u) << " " << toByte(v) << " " << 0 << "\t";
        }
        ppm << "\n";
    }
    ppm.close();
    std::cout << "rendered: " << outFile << "\n";
}

int main() {
    renderScene("scenes/scene0.crtscene", "out_scene0.ppm");
    renderScene("scenes/scene1.crtscene", "out_scene1.ppm");
    return 0;
}