#include "CRTRenderer.h"

int main() {
    // Task 2the camera is NOT at (0,0,0), but the triangle is still visible.
    // We move the camera to the right and back, then pan left to keep the triangle in view.
    CRTCamera camera;
    camera.truck(1.5f);    // move right
    camera.dolly(-1.0f);   // move back (away from the scene)
    camera.pan(20.0f);     // look a bit to the left, back toward the triangle

    std::vector<CRTTriangle> triangles = {
        CRTTriangle(CRTVector(-1.75f, -1.75f, -3.0f),
                    CRTVector( 1.75f, -1.75f, -3.0f),
                    CRTVector( 0.0f,   1.75f, -3.0f)),
    };
    std::vector<CRTColor> colors = { CRTColor(225, 255, 0) };

    render("out_task2.ppm", camera, triangles, colors, 1920, 1080);
    return 0;
}