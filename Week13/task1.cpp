#define STB_IMAGE_IMPLEMENTATION

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include "CRTScene.h"

static const int   MAXC = 255;
static const float PI = 3.14159265358979f;
static const float BIAS = 1e-3f;
static const int   MAX_DEPTH = 5;

static bool g_useAABB = true; 

int toByte(float v) { if (!(v == v)) v = 0; if (v < 0) v = 0; if (v > 1) v = 1; return (int)(v * MAXC); }
CRTVector mulc(const CRTVector& a, const CRTVector& b) { return CRTVector(a.x*b.x, a.y*b.y, a.z*b.z); }
CRTVector reflect(const CRTVector& I, const CRTVector& N) { return (I - N * (2.0f * I.dot(N))).normalize(); }

bool rayHitsBox(const CRTVector& o, const CRTVector& d, const CRTVector& lo, const CRTVector& hi) {
    float tmin = 0.0f, tmax = 1e30f;
    float ix = 1.0f/d.x, tx1 = (lo.x-o.x)*ix, tx2 = (hi.x-o.x)*ix; if (tx1>tx2) std::swap(tx1,tx2); tmin=std::max(tmin,tx1); tmax=std::min(tmax,tx2);
    float iy = 1.0f/d.y, ty1 = (lo.y-o.y)*iy, ty2 = (hi.y-o.y)*iy; if (ty1>ty2) std::swap(ty1,ty2); tmin=std::max(tmin,ty1); tmax=std::min(tmax,ty2);
    float iz = 1.0f/d.z, tz1 = (lo.z-o.z)*iz, tz2 = (hi.z-o.z)*iz; if (tz1>tz2) std::swap(tz1,tz2); tmin=std::max(tmin,tz1); tmax=std::min(tmax,tz2);
    return tmax >= tmin;
}

int closestHit(const CRTScene& scene, const CRTVector& o, const CRTVector& d, float& outT) {
    if (g_useAABB && !rayHitsBox(o, d, scene.aabbMin, scene.aabbMax)) { outT = -1; return -1; }
    float best = -1.0f; int idx = -1;
    for (int i = 0; i < (int)scene.triangles.size(); ++i) {
        float t = scene.triangles[i].intersect(o, d);
        if (t > 0.0f && (best < 0.0f || t < best)) { best = t; idx = i; }
    }
    outT = best; return idx;
}

bool inShadow(const CRTScene& scene, const CRTVector& origin, const CRTVector& L, float dist) {
    if (g_useAABB && !rayHitsBox(origin, L, scene.aabbMin, scene.aabbMax)) return false;
    for (int i = 0; i < (int)scene.triangles.size(); ++i) {
        float t = scene.triangles[i].intersect(origin, L);
        if (t > 0.0f && t < dist) return true;
    }
    return false;
}

CRTVector shade(const CRTScene& scene, const CRTVector& P, const CRTVector& N, const CRTVector& albedo) {
    CRTVector shadowOrigin = P + N * BIAS, col(0,0,0);
    for (size_t i = 0; i < scene.lights.size(); ++i) {
        CRTVector L = scene.lights[i].position - P; float sr = L.length(); L = L.normalize();
        if (inShadow(scene, shadowOrigin, L, sr)) continue;
        col = col + albedo * (scene.lights[i].intensity / (4.0f*PI*sr*sr) * std::max(0.0f, N.dot(L)));
    }
    return col;
}

CRTVector trace(const CRTScene& scene, const CRTVector& origin, const CRTVector& I, int depth) {
    CRTVector bg((float)scene.background.r/255, (float)scene.background.g/255, (float)scene.background.b/255);
    if (depth > MAX_DEPTH) return bg;
    float t; int idx = closestHit(scene, origin, I, t);
    if (idx < 0) return bg;

    const CRTTriangle& tri = scene.triangles[idx];
    const CRTMaterial& m = scene.materials[tri.materialIndex];
    CRTVector P = origin + I * t;
    float bu, bv; tri.barycentric(P, bu, bv);
    CRTVector N = tri.hitNormal(bu, bv, m.smooth);

    if (m.reflective) {
        return mulc(trace(scene, P + N*BIAS, reflect(I, N), depth+1), m.albedo);
    }
    if (m.refractive) {
        float eta1 = 1.0f, eta2 = m.ior; CRTVector n = N; float dotIN = I.dot(n);
        if (dotIN > 0.0f) { n = n*-1.0f; std::swap(eta1,eta2); dotIN = I.dot(n); }
        float cosI = -dotIN, eta = eta1/eta2;
        CRTVector reflCol = trace(scene, P + n*BIAS, reflect(I, n), depth+1);
        float k = 1.0f - eta*eta*(1.0f - cosI*cosI);
        if (k < 0.0f) return reflCol;
        CRTVector refrDir = (I*eta + n*(eta*cosI - std::sqrt(k))).normalize();
        CRTVector refrCol = trace(scene, P + (n*-1.0f)*BIAS, refrDir, depth+1);
        float f = 0.5f * std::pow(1.0f + I.dot(n), 5.0f);
        return reflCol*f + refrCol*(1.0f - f);
    }
    CRTVector albedo = m.albedo;
    if (m.textureIndex >= 0) albedo = scene.textures[m.textureIndex].sample(bu, bv, tri.interpUV(bu, bv));
    return shade(scene, P, N, albedo);
}

void renderRegion(const CRTScene& scene, std::vector<CRTColor>& buffer, int x0, int y0, int rw, int rh) {
    for (int row = y0; row < y0 + rh && row < scene.height; ++row) {
        float y = 1.0f - 2.0f * (row + 0.5f) / scene.height;
        for (int col = x0; col < x0 + rw && col < scene.width; ++col) {
            float x = (2.0f * (col + 0.5f) / scene.width - 1.0f) * scene.width / scene.height;
            CRTVector dir = scene.camera.rayDirection(CRTVector(x, y, -1.0f));
            CRTVector c = trace(scene, scene.camera.position, dir, 0);
            buffer[row * scene.width + col] = CRTColor(toByte(c.x), toByte(c.y), toByte(c.z));
        }
    }
}

struct Bucket { int x, y, w, h; };

void renderMultithreaded(const CRTScene& scene, std::vector<CRTColor>& buffer) {
    std::vector<Bucket> buckets;
    int bs = scene.bucketSize > 0 ? scene.bucketSize : 24;
    for (int y = 0; y < scene.height; y += bs)
        for (int x = 0; x < scene.width; x += bs)
            buckets.push_back({x, y, bs, bs});

    std::mutex mtx;
    size_t next = 0;
    auto worker = [&]() {
        while (true) {
            Bucket b;
            {
                std::lock_guard<std::mutex> lock(mtx);   // lock the bucket queue
                if (next >= buckets.size()) return;       // no buckets left -> stop
                b = buckets[next++];                      // pop a bucket
            }                                             // unlock
            renderRegion(scene, buffer, b.x, b.y, b.w, b.h);
        }
    };

    unsigned n = std::thread::hardware_concurrency(); if (n == 0) n = 1;
    std::vector<std::thread> threads;
    for (unsigned i = 0; i < n; ++i) threads.emplace_back(worker);
    for (auto& t : threads) t.join();
}

void writePPM(const std::string& file, const CRTScene& scene, const std::vector<CRTColor>& buffer) {
    std::ofstream ppm(file, std::ios::out | std::ios::binary);
    ppm << "P3\n" << scene.width << " " << scene.height << "\n" << MAXC << "\n";
    for (int r = 0; r < scene.height; ++r) {
        for (int c = 0; c < scene.width; ++c) {
            const CRTColor& p = buffer[r*scene.width + c];
            ppm << p.r << " " << p.g << " " << p.b << "\t";
        }
        ppm << "\n";
    }
}

double seconds(std::function<void()> fn) {
    auto t0 = std::chrono::high_resolution_clock::now();
    fn();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(t1 - t0).count();
}

int main() {
    CRTScene scene;
    if (!scene.load("scene0.crtscene")) { std::cout << "scene0.crtscene not found\n"; return 1; }
    std::cout << "triangles: " << scene.triangles.size()
              << ", threads: " << std::thread::hardware_concurrency()
              << ", bucket: " << scene.bucketSize << "\n";

    std::vector<CRTColor> buffer(scene.width * scene.height);

    g_useAABB = false;
    double t1 = seconds([&]() { renderRegion(scene, buffer, 0, 0, scene.width, scene.height); });
    std::cout << "1) baseline (1 thread, no AABB): " << t1 << " s\n";

    g_useAABB = true;
    double t2 = seconds([&]() { renderRegion(scene, buffer, 0, 0, scene.width, scene.height); });
    std::cout << "2) + AABB (1 thread):            " << t2 << " s\n";

    double t3 = seconds([&]() { renderMultithreaded(scene, buffer); });
    std::cout << "3) + AABB + buckets + threads:   " << t3 << " s\n";
    std::cout << "speedup total: " << (t1 / t3) << "x\n";

    writePPM("out_scene0.ppm", scene, buffer);
    std::cout << "rendered: out_scene0.ppm\n";
    return 0;
}