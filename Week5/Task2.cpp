#include <fstream>
#include "CRTVector.h"
#include "CRTTriangle.h"

static const int imageWidth = 1920;
static const int imageHeight = 1080;
static const int maxColorComponent = 255;

int main() {
    // Task 2: a different triangle (an upside-down one).
    CRTTriangle tri(
        CRTVector(-1.5f,  1.5f, -3.0f),
        CRTVector( 1.5f,  1.5f, -3.0f),
        CRTVector( 0.0f, -1.5f, -3.0f)
    );

    CRTVector cameraOrigin(0.0f, 0.0f, 0.0f);

    std::ofstream ppm("out_task2.ppm", std::ios::out | std::ios::binary);
    ppm << "P3\n" << imageWidth << " " << imageHeight << "\n" << maxColorComponent << "\n";

    for (int rowIdx = 0; rowIdx < imageHeight; ++rowIdx) {
            float y = (rowIdx + 0.5f) / imageHeight;
            y = 1.0f - 2.0f * y;
        for (int colIdx = 0; colIdx < imageWidth; ++colIdx) {
            float x = (colIdx + 0.5f) / imageWidth;
            x = 2.0f * x - 1.0f;
            x *= static_cast<float>(imageWidth) / imageHeight;
            CRTVector rayDir = CRTVector(x, y, -1.0f).normalize();

            float t = tri.intersect(cameraOrigin, rayDir);
            if (t > 0.0f) {
                ppm << "60 190 190\t";   // if hit then triangle color
            } else {
                ppm << "30 30 40\t";     // if miss then background
            }
        }
        ppm << "\n";
    }

    ppm.close();
    return 0;
}