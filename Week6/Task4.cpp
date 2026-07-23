#include "CRTRenderer.h"

static std::vector<CRTTriangle> makeScene() {
    return {
        CRTTriangle(CRTVector(-1.0f, -1.0f, -3.0f),
                    CRTVector( 1.0f, -1.0f, -3.0f),
                    CRTVector( 0.0f,  1.0f, -3.0f)),
        CRTTriangle(CRTVector(-4.0f, -1.0f, -4.0f),
                    CRTVector(-2.0f, -1.0f, -4.0f),
                    CRTVector(-3.0f,  1.0f, -4.0f)),
        CRTTriangle(CRTVector( 2.0f, -1.0f, -5.0f),
                    CRTVector( 4.0f, -1.0f, -5.0f),
                    CRTVector( 3.0f,  1.0f, -5.0f)),
    };
}
static std::vector<CRTColor> sceneColors() {
    return { CRTColor(225, 255, 0), CRTColor(60, 200, 200), CRTColor(210, 80, 200) };
}

int main() {
    // Task 4 a combinatio of movements  shown before and after.
    std::vector<CRTTriangle> tris = makeScene();
    std::vector<CRTColor> cols = sceneColors();

    CRTCamera camera;
    render("out_task4_before.ppm", camera, tris, cols, 1920, 1080);

    // Apply several movements in sequence: pan, then tilt, then truck.
    camera.pan(25.0f);
    camera.tilt(-10.0f);
    camera.truck(1.0f);
    render("out_task4_after.ppm", camera, tris, cols, 1920, 1080);

    return 0;
}