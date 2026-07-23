#include <sstream>
#include <iomanip>
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
    // Task 5 an animation with 72 frames, panning the camera 5 degrees each frame
    // (72 * 5 = 360 degrees = a full turn)
    const int frames = 72;
    const int width = 480;
    const int height = 270;

    std::vector<CRTTriangle> tris = makeScene();
    std::vector<CRTColor> cols = sceneColors();

    CRTCamera camera;
    for (int f = 0; f < frames; ++f) {
        std::ostringstream name;
        name << "frame_" << std::setw(3) << std::setfill('0') << f << ".ppm";

        render(name.str(), camera, tris, cols, width, height);

        camera.pan(5.0f);   
    }

    return 0;
}