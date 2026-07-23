#include <fstream>
#include <vector>
#include "CRTVector.h"
#include "CRTTriangle.h"
#include "CRTColor.h"

static const int imageWidth = 1920;
static const int imageHeight = 1080;
static const int maxColorComponent = 255;

int main() {
    //Task 4:: a shape made of several triangles
    
    CRTVector center(0.0f, 0.0f, -3.0f);
    CRTVector p0( 1.75f,  0.000f, -3.0f);
    CRTVector p1( 0.875f, 1.515f, -3.0f);
    CRTVector p2(-0.875f, 1.515f, -3.0f);
    CRTVector p3(-1.75f,  0.000f, -3.0f);
    CRTVector p4(-0.875f,-1.515f, -3.0f);
    CRTVector p5( 0.875f,-1.515f, -3.0f);

    std::vector<CRTTriangle> triangles = {
        CRTTriangle(center, p0, p1),
        CRTTriangle(center, p1, p2),
        CRTTriangle(center, p2, p3),
        CRTTriangle(center, p3, p4),
        CRTTriangle(center, p4, p5),
        CRTTriangle(center, p5, p0),
    };
    std::vector<CRTColor> colors = {
        CRTColor(230, 80, 80), CRTColor(230, 160, 60), CRTColor(230, 220, 70),
        CRTColor(90, 200, 90), CRTColor(80, 140, 230), CRTColor(170, 90, 210),
    };

    CRTVector cameraOrigin(0.0f, 0.0f, 0.0f);
    CRTColor background(30, 30, 40);

    std::ofstream ppm("out_task4.ppm", std::ios::out | std::ios::binary);
    ppm << "P3\n" << imageWidth << " " << imageHeight << "\n" << maxColorComponent << "\n";

    for (int rowIdx = 0; rowIdx < imageHeight; ++rowIdx) {
            float y = (rowIdx + 0.5f) / imageHeight;
            y = 1.0f - 2.0f * y;
        for (int colIdx = 0; colIdx < imageWidth; ++colIdx) {
            float x = (colIdx + 0.5f) / imageWidth;
            x = 2.0f * x - 1.0f;
            x *= static_cast<float>(imageWidth) / imageHeight;
            CRTVector rayDir = CRTVector(x, y, -1.0f).normalize();

            // Try every triangle, keep the closest hit.
            float closestT = -1.0f;
            int closestIdx = -1;
            for (int i = 0; i < static_cast<int>(triangles.size()); ++i) {
                float t = triangles[i].intersect(cameraOrigin, rayDir);
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
    return 0;
}