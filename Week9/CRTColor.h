#ifndef CRT_COLOR_H
#define CRT_COLOR_H

struct CRTColor {
    int r, g, b;

    CRTColor() : r(0), g(0), b(0) {}
    CRTColor(int r, int g, int b) : r(r), g(g), b(b) {}
};

#endif