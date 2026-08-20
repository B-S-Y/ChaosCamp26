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

struct CRTLight {
    CRTVector position;
    float intensity;
    CRTLight() : intensity(0.0f) {}
    CRTLight(const CRTVector& p, float i) : position(p), intensity(i) {}
};

struct CRTMaterial {
    bool reflective = false;                 // diffuse (false) or reflective (true)
    CRTVector albedo = CRTVector(0.5f, 0.5f, 0.5f);
    bool smooth = false;                     // smooth shading (interpolate vertex normals)?
};

struct CRTScene {
    int width = 0, height = 0;
    CRTColor background;
    CRTCamera camera;
    std::vector<CRTTriangle> triangles;
    std::vector<CRTLight> lights;
    std::vector<CRTMaterial> materials;      // new in Week 9

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

        if (doc.HasMember("lights")) {
            const rapidjson::Value& ls = doc["lights"];
            for (rapidjson::SizeType i = 0; i < ls.Size(); ++i) {
                const rapidjson::Value& lp = ls[i]["position"];
                CRTVector p(lp[0].GetFloat(), lp[1].GetFloat(), lp[2].GetFloat());
                lights.push_back(CRTLight(p, ls[i]["intensity"].GetFloat()));
            }
        }

        if (doc.HasMember("materials")) {
            const rapidjson::Value& ms = doc["materials"];
            for (rapidjson::SizeType i = 0; i < ms.Size(); ++i) {
                CRTMaterial m;
                if (ms[i].HasMember("type"))
                    m.reflective = (std::string(ms[i]["type"].GetString()) == "reflective");
                if (ms[i].HasMember("albedo")) {
                    const rapidjson::Value& a = ms[i]["albedo"];
                    m.albedo = CRTVector(a[0].GetFloat(), a[1].GetFloat(), a[2].GetFloat());
                }
                if (ms[i].HasMember("smooth_shading"))
                    m.smooth = ms[i]["smooth_shading"].GetBool();
                materials.push_back(m);
            }
        }
        if (materials.empty()) materials.push_back(CRTMaterial());   // a default material

        const rapidjson::Value& objects = doc["objects"];
        for (rapidjson::SizeType o = 0; o < objects.Size(); ++o) {
            const rapidjson::Value& verts = objects[o]["vertices"];
            const rapidjson::Value& tris  = objects[o]["triangles"];

            int matIdx = 0;
            if (objects[o].HasMember("material_index"))
                matIdx = objects[o]["material_index"].GetInt();

            // Read this object's vertices.
            std::vector<CRTVector> vertices;
            for (rapidjson::SizeType i = 0; i < verts.Size(); i += 3)
                vertices.push_back(CRTVector(
                    verts[i].GetFloat(), verts[i + 1].GetFloat(), verts[i + 2].GetFloat()));

                    std::vector<CRTVector> vNormals(vertices.size(), CRTVector(0, 0, 0));
            for (rapidjson::SizeType i = 0; i < tris.Size(); i += 3) {
                int i0 = tris[i].GetInt(), i1 = tris[i + 1].GetInt(), i2 = tris[i + 2].GetInt();
                CRTVector faceN = (vertices[i1] - vertices[i0]).cross(vertices[i2] - vertices[i0]).normalize();
                vNormals[i0] = vNormals[i0] + faceN;
                vNormals[i1] = vNormals[i1] + faceN;
                vNormals[i2] = vNormals[i2] + faceN;
            }
            for (size_t i = 0; i < vNormals.size(); ++i) vNormals[i] = vNormals[i].normalize();

            for (rapidjson::SizeType i = 0; i < tris.Size(); i += 3) {
                int i0 = tris[i].GetInt(), i1 = tris[i + 1].GetInt(), i2 = tris[i + 2].GetInt();
                CRTTriangle t(vertices[i0], vertices[i1], vertices[i2]);
                t.n0 = vNormals[i0];
                t.n1 = vNormals[i1];
                t.n2 = vNormals[i2];
                t.materialIndex = matIdx;
                triangles.push_back(t);
            }
        }
        return true;
    }
};

#endif // CRT_SCENE_H