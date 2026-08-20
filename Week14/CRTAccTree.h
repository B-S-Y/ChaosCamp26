#ifndef CRT_ACC_TREE_H
#define CRT_ACC_TREE_H

#include <vector>
#include <algorithm>
#include "CRTVector.h"
#include "CRTTriangle.h"

struct AABBox {
    CRTVector mn, mx;
    AABBox() : mn(1e30f, 1e30f, 1e30f), mx(-1e30f, -1e30f, -1e30f) {}
    AABBox(const CRTVector& a, const CRTVector& b) : mn(a), mx(b) {}
    void expand(const CRTVector& p) {
        mn.x = std::min(mn.x, p.x); mn.y = std::min(mn.y, p.y); mn.z = std::min(mn.z, p.z);
        mx.x = std::max(mx.x, p.x); mx.y = std::max(mx.y, p.y); mx.z = std::max(mx.z, p.z);
    }
    // Do two boxes overlap? (used to place triangles into child boxes)
    bool overlaps(const AABBox& b) const {
        return !(mx.x < b.mn.x || mn.x > b.mx.x ||
                 mx.y < b.mn.y || mn.y > b.mx.y ||
                 mx.z < b.mn.z || mn.z > b.mx.z);
    }
    bool hitRay(const CRTVector& o, const CRTVector& d) const {
        float tmin = 0.0f, tmax = 1e30f;
        float ix = 1.0f/d.x, tx1 = (mn.x-o.x)*ix, tx2 = (mx.x-o.x)*ix; if (tx1>tx2) std::swap(tx1,tx2); tmin=std::max(tmin,tx1); tmax=std::min(tmax,tx2);
        float iy = 1.0f/d.y, ty1 = (mn.y-o.y)*iy, ty2 = (mx.y-o.y)*iy; if (ty1>ty2) std::swap(ty1,ty2); tmin=std::max(tmin,ty1); tmax=std::min(tmax,ty2);
        float iz = 1.0f/d.z, tz1 = (mn.z-o.z)*iz, tz2 = (mx.z-o.z)*iz; if (tz1>tz2) std::swap(tz1,tz2); tmin=std::max(tmin,tz1); tmax=std::min(tmax,tz2);
        return tmax >= tmin;
    }
};

struct AccNode {
    AABBox box;
    int child0 = -1, child1 = -1;
    std::vector<int> tris;
};

struct CRTAccTree {
    std::vector<AccNode> nodes;
    std::vector<AABBox> triBoxes;                 // AABB of each triangle
    const std::vector<CRTTriangle>* T = nullptr;
    int maxDepth = 30, maxLeafTris = 8;

    static float comp(const CRTVector& v, int a) { return a == 0 ? v.x : (a == 1 ? v.y : v.z); }
    static void  setComp(CRTVector& v, int a, float val) { if (a==0) v.x=val; else if (a==1) v.y=val; else v.z=val; }

    int addNode(const AABBox& box) { nodes.push_back(AccNode()); nodes.back().box = box; return (int)nodes.size() - 1; }

    void build(const std::vector<CRTTriangle>& triangles) {
        T = &triangles;
        triBoxes.resize(triangles.size());
        AABBox sceneBox;
        for (size_t i = 0; i < triangles.size(); ++i) {
            AABBox b; b.expand(triangles[i].v0); b.expand(triangles[i].v1); b.expand(triangles[i].v2);
            triBoxes[i] = b;
            sceneBox.expand(triangles[i].v0); sceneBox.expand(triangles[i].v1); sceneBox.expand(triangles[i].v2);
        }
        nodes.clear();
        int root = addNode(sceneBox);
        std::vector<int> all(triangles.size());
        for (size_t i = 0; i < all.size(); ++i) all[i] = (int)i;
        buildRec(root, 0, all);
    }

    void buildRec(int nodeIdx, int depth, std::vector<int>& tri) {
        if (depth >= maxDepth || (int)tri.size() <= maxLeafTris) {
            nodes[nodeIdx].tris = tri;                 // leaf: keep the triangles here
            return;
        }
        int axis = depth % 3;
        AABBox parent = nodes[nodeIdx].box;
        float split = comp(parent.mn, axis) + (comp(parent.mx, axis) - comp(parent.mn, axis)) * 0.5f;

        AABBox A = parent, B = parent;                 // split the box into two halves
        setComp(A.mx, axis, split);
        setComp(B.mn, axis, split);

        std::vector<int> left, right;
        for (size_t i = 0; i < tri.size(); ++i) {
            int ti = tri[i];
            if (triBoxes[ti].overlaps(A)) left.push_back(ti);
            if (triBoxes[ti].overlaps(B)) right.push_back(ti);
        }

        if (!left.empty()) {
            int c = addNode(A);
            nodes[nodeIdx].child0 = c;
            buildRec(c, depth + 1, left);
        }
        if (!right.empty()) {
            int c = addNode(B);
            nodes[nodeIdx].child1 = c;
            buildRec(c, depth + 1, right);
        }
    }

    // Closest triangle the ray hits. Returns its index (or -1) and sets outT.
    int intersect(const CRTVector& o, const CRTVector& d, float& outT) const {
        float best = -1.0f; int bi = -1;
        int stack[256]; int sp = 0;
        stack[sp++] = 0;                               // start at the root
        while (sp > 0) {
            const AccNode& nd = nodes[stack[--sp]];
            if (!nd.box.hitRay(o, d)) continue;         // ray misses this box -> skip subtree
            if (!nd.tris.empty()) {                     // leaf: test its triangles
                for (size_t i = 0; i < nd.tris.size(); ++i) {
                    int ti = nd.tris[i];
                    float t = (*T)[ti].intersect(o, d);
                    if (t > 0.0f && (best < 0.0f || t < best)) { best = t; bi = ti; }
                }
            } else {
                if (nd.child0 != -1) stack[sp++] = nd.child0;
                if (nd.child1 != -1) stack[sp++] = nd.child1;
            }
        }
        outT = best; return bi;
    }
};

#endif // CRT_ACC_TREE_H