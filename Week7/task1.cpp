#include <fstream>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>
#include "CRTScene.h"

static const int maxColorComponent = 255;

CRTColor randomColor() {
    int r = std::rand() % 256;
    int g = std::rand() % 256;
    int b = std::rand() % 256;
    return CRTColor(r, g, b);
}

void renderScene(const std::string& sceneFile, const std::string& outFile) {
    CRTScene scene;
    if (!scene.load(sceneFile)) {
        std::cout << "skipped (not found): " << sceneFile << "\n";
        return;
    }

    // Give each triangle  random color, decided once 
    std::srand(123);   // fixed seed so that same colors expected every run
    std::vector<CRTColor> triColors;
    for (int i = 0; i < static_cast<int>(scene.triangles.size()); ++i)
        triColors.push_back(randomColor());

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

            //Coloring the closest triangle with its random color background otherwise.
            CRTColor c = (closestIdx >= 0) ? triColors[closestIdx] : scene.background;
            ppm << c.r << " " << c.g << " " << c.b << "\t";
        }
        ppm << "\n";
    }
    ppm.close();
    std::cout << "rendered: " << outFile << "\n";
}

int main() {

    for (int i = 0; i <= 4; ++i) {
        std::string sceneFile = "scenes/scene" + std::to_string(i) + ".crtscene";
        std::string outFile   = "out_scene" + std::to_string(i) + ".ppm";
        renderScene(sceneFile, outFile);
    }
    return 0;
}