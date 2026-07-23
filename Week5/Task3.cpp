#include <fstream>
#include <vector>
#include "CRTVector.h"
#include "CRTTriangle.h"
#include "CRTColor.h"

static const int imageWidth = 1920;
static const int imageHeight = 1080;
static const int maxColorComponent = 255;

int main() {
    // Task 3 Two triiangles which overlap on screen but sit at different depths so when they overlap the closest one must be drawn
    std::vector<CRTTriangle> triangles = {
        CRTTriangle(CRTVector(-1.75f, -1.75f, -4.0f),   // far (red)because z is more negative
                    CRTVector( 1.75f, -1.75f, -4.0f),
                    CRTVector( 0.0f,   1.75f, -4.0f)),
        CRTTriangle(CRTVector( 0.0f,  -1.75f, -2.0f),   // near (green) because z is less negative
                    CRTVector( 2.0f,   1.75f, -2.0f),
                    CRTVector(-2.0f,   1.75f, -2.0f)),
    };
    std::vector<CRTColor> colors = {
        CRTColor(220, 60, 60),
        CRTColor(60, 200, 90),
    };

    CRTVector cameraOrigin(0.0f, 0.0f, 0.0f);
    CRTColor background(30, 30, 40);

    std::ofstream ppm("out_task3.ppm", std::ios::out | std::ios::binary);
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