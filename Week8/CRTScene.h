#ifndef CRT_SCENE_H
#define CRT_SCENE_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "rapidjson/document.h"
#include "CRTVector.h"
#include "CRTTriangle.h"
#include "CRTColor.h"
#include "CRTMatrix.h"
#include "CRTCamera.h"

// A light in the scene: a point with an intensity (how bright it is).
struct CRTLight {
    CRTVector position;
    float intensity;
    CRTLight() : intensity(0.0f) {}
    CRTLight(const CRTVector& p, float i) : position(p), intensity(i) {}
};

struct CRTScene {
    int width = 0, height = 0;
    CRTColor background;
    CRTCamera camera;
    std::vector<CRTTriangle> triangles;
    std::vector<CRTLight> lights;          // new in Week 8

    bool load(const std::string& filename) {
        std::ifstream in(filename);
        if (!in) return false;
        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string json = buffer.str();

        rapidjson::Document doc;
        doc.Parse(json.c_str());

        const rapidjson::Value& settings = doc["settings"];
        const rapidjson::Value& bg = settings["background_color"];
        background = CRTColor(
            static_cast<int>(bg[0].GetFloat() * 255),
            static_cast<int>(bg[1].GetFloat() * 255),
            static_cast<int>(bg[2].GetFloat() * 255));
        const rapidjson::Value& imgs = settings["image_settings"];
        width  = imgs["width"].GetInt();
        height = imgs["height"].GetInt();

        const rapidjson::Value& cam = doc["camera"];
        const rapidjson::Value& pos = cam["position"];
        camera.position = CRTVector(pos[0].GetFloat(), pos[1].GetFloat(), pos[2].GetFloat());
        const rapidjson::Value& mat = cam["matrix"];
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                camera.rotation.m[j][i] = mat[i * 3 + j].GetFloat();

        // Read the lights (each has a position and an intensity).
        if (doc.HasMember("lights")) {
            const rapidjson::Value& ls = doc["lights"];
            for (rapidjson::SizeType i = 0; i < ls.Size(); ++i) {
                const rapidjson::Value& lp = ls[i]["position"];
                CRTVector p(lp[0].GetFloat(), lp[1].GetFloat(), lp[2].GetFloat());
                float intensity = ls[i]["intensity"].GetFloat();
                lights.push_back(CRTLight(p, intensity));
            }
        }

        const rapidjson::Value& objects = doc["objects"];
        for (rapidjson::SizeType o = 0; o < objects.Size(); ++o) {
            const rapidjson::Value& verts = objects[o]["vertices"];
            const rapidjson::Value& tris  = objects[o]["triangles"];

            std::vector<CRTVector> vertices;
            for (rapidjson::SizeType i = 0; i < verts.Size(); i += 3)
                vertices.push_back(CRTVector(
                    verts[i].GetFloat(), verts[i + 1].GetFloat(), verts[i + 2].GetFloat()));

            for (rapidjson::SizeType i = 0; i < tris.Size(); i += 3)
                triangles.push_back(CRTTriangle(
                    vertices[tris[i].GetInt()],
                    vertices[tris[i + 1].GetInt()],
                    vertices[tris[i + 2].GetInt()]));
        }
        return true;
    }
};

#endif // CRT_SCENE_H