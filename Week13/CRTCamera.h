#ifndef CRT_CAMERA_H
#define CRT_CAMERA_H

#include "CRTVector.h"
#include "CRTMatrix.h"

// A camera has a position and an orientation (rotation matrix).
// Rotations (pan/tilt/roll) change the orientation.
// Translations (truck/pedestal/dolly) move the position along the camera's LOCAL axes.
struct CRTCamera {
    CRTVector position;   
    CRTMatrix rotation;   

    CRTCamera() : position(0.0f, 0.0f, 0.0f) {}                 
    CRTCamera(const CRTVector& pos) : position(pos) {}          

    // Multiplying on the right rotates around the camera's own  axes.
    void pan(float deg)  { rotation = rotation * CRTMatrix::rotationY(toRadians(deg)); }
    void tilt(float deg) { rotation = rotation * CRTMatrix::rotationX(toRadians(deg)); }
    void roll(float deg) { rotation = rotation * CRTMatrix::rotationZ(toRadians(deg)); }

    //translations
    void truck(float d)    { position = position + rotation.mul(CRTVector(d, 0.0f, 0.0f)); }  // +right
    void pedestal(float d) { position = position + rotation.mul(CRTVector(0.0f, d, 0.0f)); }  // +up
    void dolly(float d)    { position = position + rotation.mul(CRTVector(0.0f, 0.0f, -d)); } // +forward (-Z)

    // Turning a camera space direction into a world space ray direction.
    CRTVector rayDirection(const CRTVector& camDir) const {
        return rotation.mul(camDir).normalize();
    }
};

#endif // CRT_CAMERA_H