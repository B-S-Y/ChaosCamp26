#ifndef CRT_SCENE_H
#define CRT_SCENE_H

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include "rapidjson/document.h"
#include "CRTVector.h"
#include "CRTTriangle.h"
#include "CRTColor.h"
#include "CRTMatrix.h"
#include "CRTCamera.h"
#include "CRTTexture.h"

struct CRTLight {
    CRTVector position; float intensity;
    CRTVector color;                              // light color (default white)
    CRTLight() : intensity(0.0f), color(1,1,1) {}
    CRTLight(const CRTVector& p, float i) : position(p), intensity(i), color(1,1,1) {}
};

struct CRTMaterial {
    bool reflective = false, refractive = false, smooth = false;
    float ior = 1.0f;
    int textureIndex = -1;
    CRTVector albedo = CRTVector(0.5f, 0.5f, 0.5f);
};

struct CRTScene {
    int width = 0, height = 0;
    int bucketSize = 24;                    // region size for bucket rendering
    CRTColor background;
    CRTCamera camera;
    std::vector<CRTTriangle> triangles;
    std::vector<CRTLight> lights;
    std::vector<CRTMaterial> materials;
    std::vector<CRTTexture> textures;
    CRTVector aabbMin, aabbMax;             // scene bounding box

    static CRTVector readVec(const rapidjson::Value& v) {
        return CRTVector(v[0].GetFloat(), v[1].GetFloat(), v[2].GetFloat());
    }
    int findTexture(const std::string& name) const {
        for (int i = 0; i < (int)textures.size(); ++i) if (textures[i].name == name) return i;
        return -1;
    }

    bool load(const std::string& filename) {
        std::ifstream in(filename);
        if (!in) return false;
        std::stringstream buffer; buffer << in.rdbuf();
        std::string json = buffer.str();
        std::string baseDir;
        size_t slash = filename.find_last_of("/\\");
        if (slash != std::string::npos) baseDir = filename.substr(0, slash);

        rapidjson::Document doc;
        doc.Parse(json.c_str());

        const rapidjson::Value& settings = doc["settings"];
        const rapidjson::Value& bg = settings["background_color"];
        background = CRTColor((int)(bg[0].GetFloat()*255), (int)(bg[1].GetFloat()*255), (int)(bg[2].GetFloat()*255));
        width  = settings["image_settings"]["width"].GetInt();
        height = settings["image_settings"]["height"].GetInt();
        if (settings["image_settings"].HasMember("bucket_size"))
            bucketSize = settings["image_settings"]["bucket_size"].GetInt();

        const rapidjson::Value& cam = doc["camera"];
        camera.position = readVec(cam["position"]);
        const rapidjson::Value& mat = cam["matrix"];
        for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
            camera.rotation.m[j][i] = mat[i * 3 + j].GetFloat();

        if (doc.HasMember("lights")) {
            const rapidjson::Value& ls = doc["lights"];
            for (rapidjson::SizeType i = 0; i < ls.Size(); ++i) {
                CRTLight L(readVec(ls[i]["position"]), ls[i]["intensity"].GetFloat());
                if (ls[i].HasMember("color")) L.color = readVec(ls[i]["color"]);
                lights.push_back(L);
            }
        }

        if (doc.HasMember("textures")) {
            const rapidjson::Value& ts = doc["textures"];
            for (rapidjson::SizeType i = 0; i < ts.Size(); ++i) {
                CRTTexture tex; tex.name = ts[i]["name"].GetString();
                std::string type = ts[i]["type"].GetString();
                if (type == "albedo") { tex.type = CRTTexture::ALBEDO; tex.albedo = readVec(ts[i]["albedo"]); }
                else if (type == "edges") { tex.type = CRTTexture::EDGES; tex.edgeColor = readVec(ts[i]["edge_color"]); tex.innerColor = readVec(ts[i]["inner_color"]); tex.edgeWidth = ts[i]["edge_width"].GetFloat(); }
                else if (type == "checker") { tex.type = CRTTexture::CHECKER; tex.colorA = readVec(ts[i]["color_A"]); tex.colorB = readVec(ts[i]["color_B"]); tex.squareSize = ts[i]["square_size"].GetFloat(); }
                else if (type == "bitmap") { tex.type = CRTTexture::BITMAP; std::string fp = ts[i]["file_path"].GetString(); if (!fp.empty() && (fp[0]=='/'||fp[0]=='\\')) fp = fp.substr(1); tex.loadBitmap(baseDir.empty() ? fp : baseDir + "/" + fp); }
                textures.push_back(tex);
            }
        }

        if (doc.HasMember("materials")) {
            const rapidjson::Value& ms = doc["materials"];
            for (rapidjson::SizeType i = 0; i < ms.Size(); ++i) {
                CRTMaterial m;
                if (ms[i].HasMember("type")) { std::string type = ms[i]["type"].GetString(); m.reflective = (type=="reflective"); m.refractive = (type=="refractive"); }
                if (ms[i].HasMember("albedo")) { const rapidjson::Value& a = ms[i]["albedo"]; if (a.IsString()) m.textureIndex = findTexture(a.GetString()); else if (a.IsArray()) m.albedo = readVec(a); }
                if (ms[i].HasMember("smooth_shading")) m.smooth = ms[i]["smooth_shading"].GetBool();
                if (ms[i].HasMember("ior")) m.ior = ms[i]["ior"].GetFloat();
                materials.push_back(m);
            }
        }
        if (materials.empty()) materials.push_back(CRTMaterial());

        const rapidjson::Value& objects = doc["objects"];
        for (rapidjson::SizeType o = 0; o < objects.Size(); ++o) {
            const rapidjson::Value& verts = objects[o]["vertices"];
            const rapidjson::Value& tris  = objects[o]["triangles"];
            int matIdx = objects[o].HasMember("material_index") ? objects[o]["material_index"].GetInt() : 0;

            std::vector<CRTVector> vertices;
            for (rapidjson::SizeType i = 0; i < verts.Size(); i += 3)
                vertices.push_back(CRTVector(verts[i].GetFloat(), verts[i+1].GetFloat(), verts[i+2].GetFloat()));

            std::vector<CRTVector> uvs(vertices.size(), CRTVector(0,0,0));
            if (objects[o].HasMember("uvs")) {
                const rapidjson::Value& uv = objects[o]["uvs"];
                for (rapidjson::SizeType i = 0; i + 2 < uv.Size(); i += 3)
                    uvs[i/3] = CRTVector(uv[i].GetFloat(), uv[i+1].GetFloat(), uv[i+2].GetFloat());
            }

            std::vector<CRTVector> vN(vertices.size(), CRTVector(0,0,0));
            for (rapidjson::SizeType i = 0; i < tris.Size(); i += 3) {
                int i0=tris[i].GetInt(), i1=tris[i+1].GetInt(), i2=tris[i+2].GetInt();
                CRTVector fn = (vertices[i1]-vertices[i0]).cross(vertices[i2]-vertices[i0]).normalize();
                vN[i0]=vN[i0]+fn; vN[i1]=vN[i1]+fn; vN[i2]=vN[i2]+fn;
            }
            for (size_t i = 0; i < vN.size(); ++i) vN[i] = vN[i].normalize();

            for (rapidjson::SizeType i = 0; i < tris.Size(); i += 3) {
                int i0=tris[i].GetInt(), i1=tris[i+1].GetInt(), i2=tris[i+2].GetInt();
                CRTTriangle t(vertices[i0], vertices[i1], vertices[i2]);
                t.n0=vN[i0]; t.n1=vN[i1]; t.n2=vN[i2];
                t.uv0=uvs[i0]; t.uv1=uvs[i1]; t.uv2=uvs[i2];
                t.materialIndex = matIdx;
                triangles.push_back(t);
            }
        }

        // Scene AABB: min/max over every triangle vertex.
        aabbMin = CRTVector( 1e30f,  1e30f,  1e30f);
        aabbMax = CRTVector(-1e30f, -1e30f, -1e30f);
        for (size_t i = 0; i < triangles.size(); ++i) {
            const CRTVector* vs[3] = { &triangles[i].v0, &triangles[i].v1, &triangles[i].v2 };
            for (int k = 0; k < 3; ++k) {
                aabbMin.x = std::min(aabbMin.x, vs[k]->x); aabbMax.x = std::max(aabbMax.x, vs[k]->x);
                aabbMin.y = std::min(aabbMin.y, vs[k]->y); aabbMax.y = std::max(aabbMax.y, vs[k]->y);
                aabbMin.z = std::min(aabbMin.z, vs[k]->z); aabbMax.z = std::max(aabbMax.z, vs[k]->z);
            }
        }
        return true;
    }
};

#endif // CRT_SCENE_H
