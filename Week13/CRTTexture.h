#ifndef CRT_TEXTURE_H
#define CRT_TEXTURE_H

#include <string>
#include <vector>
#include <cmath>
#include "stb_image.h"     
#include "CRTVector.h"

struct CRTTexture {
    enum Type { ALBEDO, EDGES, CHECKER, BITMAP };
    Type type = ALBEDO;
    std::string name;

    CRTVector albedo = CRTVector(1, 1, 1);                 // albedo
    CRTVector edgeColor, innerColor; float edgeWidth = 0.f; // edges
    CRTVector colorA, colorB; float squareSize = 1.f;       // checker

    std::vector<unsigned char> pixels; int w = 0, h = 0;    // bitmap

    void loadBitmap(const std::string& path) {
        int channels;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 3);
        if (data) {
            pixels.assign(data, data + (size_t)w * h * 3);
            stbi_image_free(data);
        }
    }
    CRTVector sample(float baryU, float baryV, const CRTVector& uv) const {
        switch (type) {
            case ALBEDO:
                return albedo;

            case EDGES: {
                float w3 = 1.0f - baryU - baryV;
                if (baryU < edgeWidth || baryV < edgeWidth || w3 < edgeWidth)
                    return edgeColor;
                return innerColor;
            }

            case CHECKER: {
                int x = (int)std::floor(uv.x / squareSize);
                int y = (int)std::floor(uv.y / squareSize);
                return (((x + y) % 2 + 2) % 2 == 0) ? colorA : colorB;
            }

            case BITMAP: {
                if (pixels.empty()) return CRTVector(0, 0, 0);
                int col = (int)(uv.x * w);
                int row = (int)((1.0f - uv.y) * h);   // flip V: image row 0 is at the top
                if (col < 0) col = 0; if (col >= w) col = w - 1;
                if (row < 0) row = 0; if (row >= h) row = h - 1;
                size_t i = ((size_t)row * w + col) * 3;
                return CRTVector(pixels[i] / 255.0f, pixels[i + 1] / 255.0f, pixels[i + 2] / 255.0f);
            }
        }
        return CRTVector(0, 0, 0);
    }
};

#endif // CRT_TEXTURE_H