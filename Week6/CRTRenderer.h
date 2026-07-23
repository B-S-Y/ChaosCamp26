#ifndef CRT_RENDERER_H
#define CRT_RENDERER_H

#include <fstream>
#include <vector>
#include <string>
#include "CRTVector.h"
#include "CRTTriangle.h"
#include "CRTColor.h"
#include "CRTCamera.h"

static const int maxColorComponent = 255;

//Renders one scene through a camera into a .ppm file.
// Same ray/closest-triangle logic as Week 5, but the ray now comes from the camera, not the origin.
inline void render(const std::string& filename,
                   const CRTCamera& camera,
                   const std::vector<CRTTriangle>& triangles,
                   const std::vector<CRTColor>& colors,
                   int width, int height) {
    CRTColor background(30, 30, 40);

    std::ofstream ppm(filename, std::ios::out | std::ios::binary);
    ppm << "P3\n" << width << " " << height << "\n" << maxColorComponent << "\n";

    for (int rowIdx = 0; rowIdx < height; ++rowIdx) {
        float y = (rowIdx + 0.5f) / height;
        y = 1.0f - 2.0f * y;
        for (int colIdx = 0; colIdx < width; ++colIdx) {
            float x = (colIdx + 0.5f) / width;
            x = 2.0f * x - 1.0f;
            x *= static_cast<float>(width) / height;

            // Ray in camera space, then transformed to the world by the camera.
            CRTVector camDir(x, y, -1.0f);
            CRTVector rayDir = camera.rayDirection(camDir);
            CRTVector rayOrigin = camera.position;

            float closestT = -1.0f;
            int closestIdx = -1;
            for (int i = 0; i < static_cast<int>(triangles.size()); ++i) {
                float t = triangles[i].intersect(rayOrigin, rayDir);
                if (t > 0.0f && (closestIdx == -1 || t < closestT)) {
                    closestT = t;
                    closestIdx = i;
                }
            }

            CRTColor c = (closestIdx == -1) ? background : colors[closestIdx];
            ppm << c.r << " " << c.g << " " << c.b << "\t";
        }
        ppm << "\n";
    }
    ppm.close();
}

#endif // CRT_RENDERER_H