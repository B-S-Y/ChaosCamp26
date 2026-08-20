#ifndef CRT_COLOR_H
#define CRT_COLOR_H

// A pixel/triangle color: r, g, b in 0-255. (Carried over from Week 3.)
struct CRTColor {
    int r, g, b;
    CRTColor() : r(0), g(0), b(0) {}
    CRTColor(int r, int g, int b) : r(r), g(g), b(b) {}
};

#endif // CRT_COLOR_H
