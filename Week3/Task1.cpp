#include <fstream>
#include <cmath>
#include "CRTVector.h"
#include "CRTColor.h"

//image res
static const int imageWidth = 1920;
static const int imageHeight = 1080;
static const int maxColorComponent = 255;

int clampByte(float value) {
    int c = static_cast<int>(value);
    if (c < 0)   c = 0;
    if (c > 255) c = 255;
    return c;
}


void writeHeader(std::ofstream& out) {
    out << "P3\n" << imageWidth << " " << imageHeight << "\n" << maxColorComponent << "\n";
}


//image 1:Ray direction, mapped to [0, 255] range
// R = |x|, G = |y|, B =|z| of the normalized ray direction.
// abs() mirrors negatives onto positives, so the picture is symmetric
// dark in the center and brighter toward every edge

void renderRayDirRef() {
    std::ofstream out("out_1_raydir.ppm", std::ios::out | std::ios::binary);
    writeHeader(out);
    for (int rowIdx = 0; rowIdx < imageHeight; ++rowIdx) {
        for (int colIdx = 0; colIdx < imageWidth; ++colIdx) {
            float x = (colIdx + 0.5f) / imageWidth;
            float y = (rowIdx + 0.5f) / imageHeight;
            x = 2.0f * x - 1.0f;
            y = 1.0f - 2.0f * y;
            x *= static_cast<float>(imageWidth) / imageHeight;

            CRTVector dir = CRTVector(x, y, -1.0f).normalize();

            CRTColor color(
                clampByte(std::fabs(dir.x) * 255.0f),
                clampByte(std::fabs(dir.y) * 255.0f),
                clampByte(std::fabs(dir.z) * 255.0f)
            );
            out << color.r << " " << color.g << " " << color.b << "\t";
        }
        out << "\n";
    }
    out.close();
}



//image 2:Ray direction, shifted to [0, 1] range
void renderRayDirShifted() {
    std::ofstream out("out_2_shifted.ppm", std::ios::out | std::ios::binary);
    writeHeader(out);
    for (int rowIdx = 0; rowIdx < imageHeight; ++rowIdx) {
        for (int colIdx = 0; colIdx < imageWidth; ++colIdx) {
            float x = (colIdx + 0.5f) / imageWidth;
            float y = (rowIdx + 0.5f) / imageHeight;
            x = 2.0f * x - 1.0f;
            y = 1.0f - 2.0f * y;
            x *= static_cast<float>(imageWidth) / imageHeight;

            CRTVector dir = CRTVector(x, y, -1.0f).normalize();

            CRTColor color(
                clampByte((dir.x + 1.0f) * 0.5f * 255.0f),
                clampByte((dir.y + 1.0f) * 0.5f * 255.0f),
                clampByte((dir.z + 1.0f) * 0.5f * 255.0f)
            );
            out << color.r << " " << color.g << " " << color.b << "\t";
        }
        out << "\n";
    }
    out.close();
}


//image 3:Raster grid, color based on pixel indices
// Color(x % 256, y % 256, 0) straight from the pixel indices.
// The modulo makes the color wrap every 256 pixels

void renderRaster() {
    std::ofstream out("out_3_raster.ppm", std::ios::out | std::ios::binary);
    writeHeader(out);
    for (int rowIdx = 0; rowIdx < imageHeight; ++rowIdx) {
        for (int colIdx = 0; colIdx < imageWidth; ++colIdx) {
            CRTColor color(colIdx % 256, rowIdx % 256, 0);
            out << color.r << " " << color.g << " " << color.b << "\t";
        }
        out << "\n";
    }
    out.close();
}

int main() {
    renderRayDirRef();      
    renderRayDirShifted();  
    renderRaster();         
    return 0;
}