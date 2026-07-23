#include <iostream>
#include "CRTVector.h"
#include "CRTMatrix.h"

int main() {
    // Task 1, panning the camera vector (0, 0, -1) by 30 degrees around the Y axis.
    CRTVector forward(0.0f, 0.0f, -1.0f);

    CRTMatrix pan = CRTMatrix::rotationY(toRadians(30.0f));
    CRTVector rotated = pan.mul(forward);

    std::cout << "before: (" << forward.x << ", " << forward.y << ", " << forward.z << ")\n";
    std::cout << "after 30 deg pan: (" << rotated.x << ", " << rotated.y << ", " << rotated.z << ")\n";
    return 0;
}