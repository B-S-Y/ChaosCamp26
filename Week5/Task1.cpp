#include <fstream>
#include "CRTVector.h"
#include "CRTTriangle.h"

static const int imageWidth = 1920;
static const int imageHeight = 1080;
static const int maxColorComponent = 255;

int main() {
    // Task 1 the triangle
    CRTTriangle tri(
        CRTVector(-1.75f, -1.75f, -3.0f),
        CRTVector( 1.75f, -1.75f, -3.0f),
        CRTVector( 0.0f,   1.75f, -3.0f)
    );

    CRTVector cameraOrigin(0.0f, 0.0f, 0.0f);   //camera at origin looking down -Z

    std::ofstream ppm("out_task1.ppm", std::ios::out | std::ios::binary);
    ppm << "P3\n" << imageWidth << " " << imageHeight << "\n" << maxColorComponent << "\n";

    for (int rowIdx = 0; rowIdx < imageHeight; ++rowIdx) {
            float y = (rowIdx + 0.5f) / imageHeight;   // depends only on the row
            y = 1.0f - 2.0f * y;
        for (int colIdx = 0; colIdx < imageWidth; ++colIdx) {
            float x = (colIdx + 0.5f) / imageWidth;
            x = 2.0f * x - 1.0f;
            x *= static_cast<float>(imageWidth) / imageHeight;
            CRTVector rayDir = CRTVector(x, y, -1.0f).normalize();

            float t = tri.intersect(cameraOrigin, rayDir);
            if (t > 0.0f) {
                ppm << "225 255 0\t";    // if hit then triangle color
            } else {
                ppm << "30 30 40\t";     // if miss then background
            }
        }
        ppm << "\n";
    }

    ppm.close();
    return 0;
}