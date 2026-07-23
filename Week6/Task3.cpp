#include "CRTRenderer.h"

// A little scene with three triangles spread out, so camera movement is easy to see.
static std::vector<CRTTriangle> makeScene() {
    return {
        CRTTriangle(CRTVector(-1.0f, -1.0f, -3.0f),   // center (yellow)
                    CRTVector( 1.0f, -1.0f, -3.0f),
                    CRTVector( 0.0f,  1.0f, -3.0f)),
        CRTTriangle(CRTVector(-4.0f, -1.0f, -4.0f),   // left (cyan)
                    CRTVector(-2.0f, -1.0f, -4.0f),
                    CRTVector(-3.0f,  1.0f, -4.0f)),
        CRTTriangle(CRTVector( 2.0f, -1.0f, -5.0f),   // right (magenta)
                    CRTVector( 4.0f, -1.0f, -5.0f),
                    CRTVector( 3.0f,  1.0f, -5.0f)),
    };
}
static std::vector<CRTColor> sceneColors() {
    return { CRTColor(225, 255, 0), CRTColor(60, 200, 200), CRTColor(210, 80, 200) };
}

int main() {
    // Task 3 one camera movement shown before and after.
    std::vector<CRTTriangle> tris = makeScene();
    std::vector<CRTColor> cols = sceneColors();

    // before default camera at the origin.
    CRTCamera camera;
    render("out_task3_before.ppm", camera, tris, cols, 1920, 1080);

    //afterpan the camera 30 degrees.
    camera.pan(30.0f);
    render("out_task3_after.ppm", camera, tris, cols, 1920, 1080);

    return 0;
}