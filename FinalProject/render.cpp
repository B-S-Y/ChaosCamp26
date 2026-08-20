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
#include <random>
#include <sstream>
#include <iomanip>
#include "CRTScene.h"
#include "CRTAccTree.h"

static const int   MAXC = 255;
static const float PI = 3.14159265358979f;
static const float BIAS = 1e-3f;

// --- Feature knobs (set per shot) ---
static CRTAccTree g_tree;
static int   g_maxDepth = 6;        //max ray bounces (raise for mirror rooms)
static float g_fovScale = 1.0f;     //tan(fov/2); 1.0 = 90 degrees
static int   g_spp = 1;             //samples per pixel (anti-aliasing)
static bool  g_giOn = false;        //global illumination on/off
static int   g_giRays = 8;          //GI bounce rays per hit
static int   g_giMaxDepth = 2;      //GI bounce depth
static float g_aperture = 0.0f;     //depth-of-field lens radius (0 = pinhole)
static float g_focusDist = 3.0f;    //depth-of-field focus distance

int toByte(float v){ if(!(v==v)) v=0; if(v<0)v=0; if(v>1)v=1; return (int)(v*MAXC); }
CRTVector mulc(const CRTVector&a,const CRTVector&b){ return CRTVector(a.x*b.x,a.y*b.y,a.z*b.z); }
CRTVector reflect(const CRTVector&I,const CRTVector&N){ return (I - N*(2.0f*I.dot(N))).normalize(); }

float rand01(){ static thread_local std::mt19937 rng(std::random_device{}()); return std::uniform_real_distribution<float>(0,1)(rng); }

// Uniform random direction in the hemisphere around N.
CRTVector randomHemisphere(const CRTVector& N){
    for(int k=0;k<16;++k){
        CRTVector v(rand01()*2-1, rand01()*2-1, rand01()*2-1);
        float l=v.length();
        if(l>0.0001f && l<=1.0f){ v=v.normalize(); if(v.dot(N)<0) v=v*-1.0f; return v; }
    }
    return N;
}

int closestHit(const CRTVector& o,const CRTVector& d,float& t){ return g_tree.intersect(o,d,t); }
bool inShadow(const CRTScene&,const CRTVector& o,const CRTVector& L,float dist){ float t; int i=closestHit(o,L,t); return i>=0 && t<dist; }

CRTVector directLight(const CRTScene& s,const CRTVector& P,const CRTVector& N,const CRTVector& albedo){
    CRTVector so=P+N*BIAS, col(0,0,0);
    for(size_t i=0;i<s.lights.size();++i){
        CRTVector L=s.lights[i].position-P; float sr=L.length(); L=L.normalize();
        if(inShadow(s,so,L,sr)) continue;
        float f=s.lights[i].intensity/(4.0f*PI*sr*sr)*std::max(0.0f,N.dot(L));
        col=col+mulc(albedo,s.lights[i].color)*f;      // tint by the light's color
    }
    return col;
}

CRTVector trace(const CRTScene& s,const CRTVector& origin,const CRTVector& I,int depth){
    CRTVector bg((float)s.background.r/255,(float)s.background.g/255,(float)s.background.b/255);
    if(depth>g_maxDepth) return bg;
    float t; int idx=closestHit(origin,I,t);
    if(idx<0) return bg;

    const CRTTriangle& tri=s.triangles[idx];
    const CRTMaterial& m=s.materials[tri.materialIndex];
    CRTVector P=origin+I*t; float bu,bv; tri.barycentric(P,bu,bv);
    CRTVector N=tri.hitNormal(bu,bv,m.smooth);

    if(m.reflective) return mulc(trace(s,P+N*BIAS,reflect(I,N),depth+1),m.albedo);
    if(m.refractive){
        float e1=1.0f,e2=m.ior; CRTVector n=N; float dIN=I.dot(n);
        if(dIN>0){ n=n*-1.0f; std::swap(e1,e2); dIN=I.dot(n); }
        float cosI=-dIN, eta=e1/e2;
        CRTVector rc=trace(s,P+n*BIAS,reflect(I,n),depth+1);
        float k=1-eta*eta*(1-cosI*cosI);
        if(k<0) return rc;
        CRTVector rd=(I*eta+n*(eta*cosI-std::sqrt(k))).normalize();
        CRTVector tc=trace(s,P+(n*-1.0f)*BIAS,rd,depth+1);
        float f=0.5f*std::pow(1+I.dot(n),5.0f);
        return rc*f+tc*(1-f);
    }

    // diffuse: albedo from texture or constant
    CRTVector albedo=m.albedo;
    if(m.textureIndex>=0) albedo=s.textures[m.textureIndex].sample(bu,bv,tri.interpUV(bu,bv));

    CRTVector direct=directLight(s,P,N,albedo);
    if(!g_giOn || depth>=g_giMaxDepth) return direct;

    // Global illumination: gather indirect light with random bounce rays.
    CRTVector indirect(0,0,0);
    for(int i=0;i<g_giRays;++i)
        indirect=indirect+trace(s,P+N*BIAS,randomHemisphere(N),depth+1);
    indirect=mulc(albedo,indirect*(1.0f/g_giRays));
    return direct+indirect;
}

// --- Camera helpers ---
void lookAt(CRTCamera& cam,const CRTVector& pos,const CRTVector& target,const CRTVector& up){
    cam.position=pos;
    CRTVector f=(target-pos).normalize();
    CRTVector r=f.cross(up).normalize();
    CRTVector u=r.cross(f);
    CRTMatrix& m=cam.rotation;
    m.m[0][0]=r.x; m.m[1][0]=r.y; m.m[2][0]=r.z;   // right
    m.m[0][1]=u.x; m.m[1][1]=u.y; m.m[2][1]=u.z;   // up
    m.m[0][2]=-f.x;m.m[1][2]=-f.y;m.m[2][2]=-f.z;  // -forward
}

void renderRegion(const CRTScene& s,std::vector<CRTColor>& buf,int x0,int y0,int rw,int rh){
    float aspect=(float)s.width/s.height;
    for(int row=y0;row<y0+rh && row<s.height;++row){
        for(int col=x0;col<x0+rw && col<s.width;++col){
            // camera right/up axes (for depth-of-field lens sampling)
            CRTVector camRight=s.camera.rotation.mul(CRTVector(1,0,0));
            CRTVector camUp   =s.camera.rotation.mul(CRTVector(0,1,0));
            CRTVector acc(0,0,0);
            for(int sidx=0;sidx<g_spp;++sidx){
                float jx = g_spp>1 ? rand01() : 0.5f;
                float jy = g_spp>1 ? rand01() : 0.5f;
                float x=(2.0f*(col+jx)/s.width-1.0f)*aspect*g_fovScale;
                float y=(1.0f-2.0f*(row+jy)/s.height)*g_fovScale;
                CRTVector dir=s.camera.rayDirection(CRTVector(x,y,-1.0f));
                CRTVector origin=s.camera.position;
                if(g_aperture>0.0f){
                    // Depth of field: aim through a random point on the lens at the focus plane.
                    CRTVector focal=origin+dir*g_focusDist;
                    float a=2.0f*PI*rand01(), r=g_aperture*std::sqrt(rand01());
                    origin=origin+camRight*(r*std::cos(a))+camUp*(r*std::sin(a));
                    dir=(focal-origin).normalize();
                }
                acc=acc+trace(s,origin,dir,0);
            }
            acc=acc*(1.0f/g_spp);
            buf[row*s.width+col]=CRTColor(toByte(acc.x),toByte(acc.y),toByte(acc.z));
        }
    }
}

struct Bucket{int x,y,w,h;};
void renderMT(const CRTScene& s,std::vector<CRTColor>& buf){
    std::vector<Bucket> bk; int bs=s.bucketSize>0?s.bucketSize:24;
    for(int y=0;y<s.height;y+=bs) for(int x=0;x<s.width;x+=bs) bk.push_back({x,y,bs,bs});
    std::mutex mtx; size_t next=0;
    auto worker=[&](){ while(true){ Bucket b; { std::lock_guard<std::mutex> lk(mtx); if(next>=bk.size())return; b=bk[next++]; } renderRegion(s,buf,b.x,b.y,b.w,b.h); } };
    unsigned n=std::thread::hardware_concurrency(); if(!n)n=1;
    std::vector<std::thread> th; for(unsigned i=0;i<n;++i) th.emplace_back(worker); for(auto&t:th)t.join();
}

void writePPM(const std::string& f,const CRTScene& s,const std::vector<CRTColor>& buf){
    std::ofstream p(f,std::ios::out|std::ios::binary);
    p<<"P3\n"<<s.width<<" "<<s.height<<"\n"<<MAXC<<"\n";
    for(int r=0;r<s.height;++r){ for(int c=0;c<s.width;++c){ const CRTColor& px=buf[r*s.width+c]; p<<px.r<<" "<<px.g<<" "<<px.b<<"\t"; } p<<"\n"; }
}

float toRad(float deg){ return deg*PI/180.0f; }

// ---------- Procedural "infinity mirror room" scene ----------
void addQuad(CRTScene& s,const CRTVector& a,const CRTVector& b,const CRTVector& c,const CRTVector& d,int mat){
    CRTTriangle t1(a,b,c); t1.materialIndex=mat; s.triangles.push_back(t1);
    CRTTriangle t2(a,c,d); t2.materialIndex=mat; s.triangles.push_back(t2);
}
void addSphere(CRTScene& s,const CRTVector& c,float R,int mat,int stacks=22,int slices=30){
    for(int i=0;i<stacks;++i){
        float p0=PI*i/stacks, p1=PI*(i+1)/stacks;
        for(int j=0;j<slices;++j){
            float t0=2*PI*j/slices, t1=2*PI*(j+1)/slices;
            auto P=[&](float p,float t){ return c+CRTVector(R*std::sin(p)*std::cos(t),R*std::cos(p),R*std::sin(p)*std::sin(t)); };
            CRTVector a=P(p0,t0),b=P(p1,t0),cc=P(p1,t1),d=P(p0,t1);
            CRTVector na=(a-c).normalize(),nb=(b-c).normalize(),nc=(cc-c).normalize(),nd=(d-c).normalize();
            CRTTriangle T1(a,b,cc); T1.materialIndex=mat; T1.n0=na;T1.n1=nb;T1.n2=nc; s.triangles.push_back(T1);
            CRTTriangle T2(a,cc,d); T2.materialIndex=mat; T2.n0=na;T2.n1=nc;T2.n2=nd; s.triangles.push_back(T2);
        }
    }
}

// The dragon mesh (loaded once from scene1), centered + scaled to fit the room.
static std::vector<CRTTriangle> g_dragon;
static CRTVector g_dCenter(0,0,0);
static float g_dScale=1.0f;
void loadDragon(const std::string& file,float targetSize){
    CRTScene d; if(!d.load(file)){ std::cout<<"dragon scene missing: "<<file<<"\n"; return; }
    CRTVector mn(1e30f,1e30f,1e30f), mx(-1e30f,-1e30f,-1e30f);
    for(size_t i=0;i<d.triangles.size();++i){
        const CRTTriangle& t=d.triangles[i];
        if(d.materials[t.materialIndex].reflective) continue;   // skip the mirror floor, keep the dragon
        g_dragon.push_back(t);
        const CRTVector* v[3]={&t.v0,&t.v1,&t.v2};
        for(int k=0;k<3;++k){ mn.x=std::min(mn.x,v[k]->x);mn.y=std::min(mn.y,v[k]->y);mn.z=std::min(mn.z,v[k]->z);
                              mx.x=std::max(mx.x,v[k]->x);mx.y=std::max(mx.y,v[k]->y);mx.z=std::max(mx.z,v[k]->z); }
    }
    g_dCenter=(mn+mx)*0.5f;
    float ext=std::max(mx.x-mn.x,std::max(mx.y-mn.y,mx.z-mn.z));
    g_dScale = ext>0? targetSize/ext : 1.0f;
    std::cout<<"dragon loaded: "<<g_dragon.size()<<" triangles\n";
}
// Add the dragon, spun around Y by 'ang', placed at 'at'.
void addDragon(CRTScene& s,float ang,const CRTVector& at,int mat){
    float ca=std::cos(ang), sa=std::sin(ang);
    auto rotY=[&](const CRTVector& p){ return CRTVector(p.x*ca+p.z*sa, p.y, -p.x*sa+p.z*ca); };
    for(size_t i=0;i<g_dragon.size();++i){
        const CRTTriangle& t=g_dragon[i];
        CRTVector a=rotY((t.v0-g_dCenter)*g_dScale)+at;
        CRTVector b=rotY((t.v1-g_dCenter)*g_dScale)+at;
        CRTVector c=rotY((t.v2-g_dCenter)*g_dScale)+at;
        CRTTriangle nt(a,b,c);
        nt.n0=rotY(t.n0); nt.n1=rotY(t.n1); nt.n2=rotY(t.n2);
        nt.materialIndex=mat;
        s.triangles.push_back(nt);
    }
}

// Rebuild the whole animated scene for time u in [0,1].
void buildInfinity(CRTScene& s,float u,int W,int H){
    s.width=W; s.height=H; s.bucketSize=24; s.background=CRTColor(2,2,6);
    s.triangles.clear(); s.materials.clear(); s.lights.clear();
    // materials: 0 mirror, 1 glass, 2 chrome, 3/4/5 red/green/blue diffuse gems
    CRTMaterial mirror; mirror.reflective=true; mirror.albedo=CRTVector(0.82f,0.82f,0.88f);
    CRTMaterial glass;  glass.refractive=true;  glass.ior=1.5f; glass.smooth=true;
    CRTMaterial chrome; chrome.reflective=true; chrome.albedo=CRTVector(0.95f,0.95f,0.97f); chrome.smooth=true;
    CRTMaterial red;    red.albedo=CRTVector(0.9f,0.15f,0.15f);
    CRTMaterial grn;    grn.albedo=CRTVector(0.15f,0.9f,0.2f);
    CRTMaterial blu;    blu.albedo=CRTVector(0.2f,0.3f,0.95f);
    CRTMaterial ground; ground.albedo=CRTVector(0.75f,0.75f,0.78f);   // lit diffuse floor
    s.materials={mirror,glass,chrome,red,grn,blu,ground};

    // Mirror room: x[-3,3] y[-3,3] z[-6,0], open front at z=0.
    float X=3,Y=3,Zb=-6,Zf=0;
    addQuad(s,{-X,-Y,Zf},{X,-Y,Zf},{X,-Y,Zb},{-X,-Y,Zb},6);   // floor (diffuse, catches light)
    addQuad(s,{-X, Y,Zf},{-X,Y,Zb},{X,Y,Zb},{X,Y,Zf},0);       // ceiling
    addQuad(s,{-X,-Y,Zb},{X,-Y,Zb},{X,Y,Zb},{-X,Y,Zb},0);      // back
    addQuad(s,{-X,-Y,Zf},{-X,-Y,Zb},{-X,Y,Zb},{-X,Y,Zf},0);    // left
    addQuad(s,{X,-Y,Zf},{X,Y,Zf},{X,Y,Zb},{X,-Y,Zb},0);        // right
    addQuad(s,{-X,-Y,Zf},{-X,Y,Zf},{X,Y,Zf},{X,-Y,Zf},0);      // front (closed -> camera is inside)

    CRTVector center(0,0,-3);
    float a=u*2*PI;
    // Chrome dragon centrepiece, STATIONARY (only the camera moves around it).
    addDragon(s, 2.1f, center+CRTVector(0,-0.1f,0), 2);
    // One glass sphere orbiting the dragon (shows refraction + geometry animation).
    addSphere(s, center+CRTVector(2.0f*std::cos(a), 0.4f+0.5f*std::sin(a*2), 2.0f*std::sin(a)), 0.55f, 1);

    // Lighting changes with the story: opening orbit + closing crane = colored
    // lights; the middle hold/zoom = a single calm white light.
    if(u < 5.0f/15 || u >= 10.0f/15){
        float la=u*2*PI;
        auto L=[&](float ang,CRTVector col){ CRTLight l(center+CRTVector(2.3f*std::cos(ang),1.6f,2.3f*std::sin(ang)),260.0f); l.color=col; s.lights.push_back(l); };
        L(la,           CRTVector(1,0.25f,0.25f));
        L(la+2*PI/3,    CRTVector(0.25f,1,0.35f));
        L(la+4*PI/3,    CRTVector(0.35f,0.45f,1));
        CRTLight key(center+CRTVector(0,2.6f,2.0f),200.0f); s.lights.push_back(key);
    } else {
        CRTLight w1(center+CRTVector(0,2.4f,1.8f),420.0f);   s.lights.push_back(w1);  // single white
        CRTLight w2(center+CRTVector(0,0.0f, 2.6f),260.0f);  s.lights.push_back(w2);  // front fill
    }
}

std::string frameName(const std::string& prefix,int i){ std::ostringstream o; o<<prefix<<std::setw(3)<<std::setfill('0')<<i<<".ppm"; return o.str(); }

// Renders one still frame with the current global settings.
void renderStill(CRTScene& s,const std::string& out){ std::vector<CRTColor> buf(s.width*s.height); renderMT(s,buf); writePPM(out,s,buf); }

//Shot A: Cornell box, GI, camera orbits + pushes in, light gently sways ---
void shotCornell(const std::string& sceneFile,int frames){
    CRTScene s; if(!s.load(sceneFile)){ std::cout<<"missing "<<sceneFile<<"\n"; return; }
    g_tree.build(s.triangles);
    g_giOn=true; g_giRays=16; g_giMaxDepth=1; g_spp=3; g_fovScale=std::tan(toRad(45));  // 90 deg FOV
    CRTVector center(0,0,-3);
    CRTVector light0 = s.lights.empty()? CRTVector(0,1.3,-2) : s.lights[0].position;
    for(int f=0;f<frames;++f){
        float t=frames>1? (float)f/(frames-1):0.0f;
        float ang = toRad(-20.0f + 40.0f*t);         // orbit -20 -> +20 degrees
        float R   = 4.6f - 0.9f*t;                    // push in a little
        lookAt(s.camera, CRTVector(R*std::sin(ang), 0.4f, -3 + R*std::cos(ang)), center, CRTVector(0,1,0));
        if(!s.lights.empty()) s.lights[0].position = light0 + CRTVector(1.0f*std::sin(t*2*PI), 0, 0); // sway
        auto t0=std::chrono::high_resolution_clock::now();
        renderStill(s, frameName("frames/A_",f));
        std::cout<<"A "<<f<<"/"<<frames<<" "<<std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-t0).count()<<"s\n";
    }
}

//Shot B: dragon, dolly-zoom (Vertigo effect): camera moves in while FOV widens ---
void shotDragon(const std::string& sceneFile,int frames){
    CRTScene s; if(!s.load(sceneFile)){ std::cout<<"missing "<<sceneFile<<"\n"; return; }
    g_tree.build(s.triangles);
    g_giOn=false; g_spp=2;
    CRTVector target(0,2,0);
    CRTVector back = (CRTVector(0,14,26)-target).normalize();   // camera-to-target line
    for(int f=0;f<frames;++f){
        float t=frames>1? (float)f/(frames-1):0.0f;
        float dist = 30.0f - 12.0f*t;                 // dolly in: 30 -> 18
        float konst = 30.0f*std::tan(toRad(20));      // keep subject size: dist*tan(fov/2)=const
        g_fovScale = konst / dist;                     // FOV widens as we move in -> Vertigo
        lookAt(s.camera, target + back*dist, target, CRTVector(0,1,0));
        auto t0=std::chrono::high_resolution_clock::now();
        renderStill(s, frameName("frames/B_",f));
        std::cout<<"B "<<f<<"/"<<frames<<" "<<std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-t0).count()<<"s\n";
    }
}

// Infinity mirror room: mirror walls, orbiting glass+chrome spheres, colored lights, DOF.
void infinityDriver(int frames,int W,int H,int spp){
    g_maxDepth=6; g_giOn=false; g_spp=spp; g_aperture=0.0f;   // sharp (no DOF blur)
    loadDragon("scene1.crtscene", 2.6f);            // chrome dragon centrepiece
    CRTVector center(0,0,-3);
    CRTVector subject=center+CRTVector(0,-0.1f,0);      // the dragon
    // Story beats as fractions of the clip (total ~15 s):
    const float P1=5.0f/15, P2=7.0f/15, P3=10.0f/15;   // orbit / hold / zoom-in / inside-spin
    for(int f=0;f<frames;++f){
        float u=frames>1?(float)f/(frames-1):0.0f;
        CRTScene s; buildInfinity(s,u,W,H);
        g_tree.build(s.triangles);

        CRTVector pos, look;
        g_fovScale=std::tan(toRad(32));
        g_aperture=0.0f;                                // sharp by default
        if(u<P1){
            // 0-5s: sharp, start in FRONT of the dragon and orbit one full turn.
            float th=(u/P1)*2*PI, R=2.7f;
            pos=subject+CRTVector(R*std::sin(th),0.5f,R*std::cos(th)); look=subject;
        } else if(u<P2){
            // 5-7s: hold in front, single-colour light (set in buildInfinity).
            pos=subject+CRTVector(0,0.5f,2.7f); look=subject;
        } else if(u<P3){
            // 7-10s: dolly in toward the dragon (medium speed).
            float p=(u-P2)/(P3-P2);                     // 0..1
            float R=2.7f-2.35f*p;                       // 2.7 -> ~0.35 (just outside the surface)
            pos=subject+CRTVector(0,0.5f-0.5f*p,R); look=subject;
        } else {
            // 10-15s: CRANE reveal - camera rises and circles while pulling back to
            // its widest, colored lights return, shallow depth of field on the dragon.
            float p=(u-P3)/(1.0f-P3);                   // 0..1
            float th=p*(2*PI*0.75f);                    // three-quarter turn
            float R=0.35f+2.45f*p;                      // pull back out from the surface
            float y=0.0f+2.4f*p;                        // crane up toward the ceiling
            pos=subject+CRTVector(R*std::sin(th),y,R*std::cos(th)); look=subject;
            g_aperture=0.045f; g_focusDist=(pos-subject).length();   // bokeh on the reflections
        }
        lookAt(s.camera, pos, look, CRTVector(0,1,0));

        auto t0=std::chrono::high_resolution_clock::now();
        renderStill(s, frameName("frames/INF_",f));
        std::cout<<"INF "<<f<<"/"<<frames<<" "<<std::chrono::duration<double>(std::chrono::high_resolution_clock::now()-t0).count()<<"s\n";
    }
}

int main(int argc,char** argv){
    std::string mode = argc>1?argv[1]:"animate";
    if(mode=="infinity"){
        int frames=argc>2?std::atoi(argv[2]):20;
        int W=argc>3?std::atoi(argv[3]):260, H=argc>4?std::atoi(argv[4]):260;
        int spp=argc>5?std::atoi(argv[5]):3;
        infinityDriver(frames,W,H,spp);
        return 0;
    }
    if(mode=="animate"){
        int fA = argc>2?std::atoi(argv[2]):20;   // Cornell frames
        int fB = argc>3?std::atoi(argv[3]):20;   // dragon frames
        shotCornell("cornell.crtscene", fA);
        shotDragon("scene1.crtscene", fB);
        return 0;
    }
    //Single-frame test: render <scene> <out.ppm> <gi|plain>
    std::string sceneFile=argc>1?argv[1]:"cornell.crtscene";
    std::string outFile  =argc>2?argv[2]:"frame.ppm";
    std::string m        =argc>3?argv[3]:"plain";
    CRTScene s; if(!s.load(sceneFile)){ std::cout<<"not found: "<<sceneFile<<"\n"; return 1; }
    g_tree.build(s.triangles);
    if(m=="gi"){ g_giOn=true; g_giRays=16; g_giMaxDepth=1; g_spp=3; g_fovScale=std::tan(toRad(45)); }
    else { g_spp=2; g_fovScale=1.0f; }
    renderStill(s,outFile);
    return 0;
}
