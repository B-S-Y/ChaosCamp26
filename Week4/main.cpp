#include <iostream>
#include "CRTVector.h"
#include "CRTTriangle.h"

// Small helper to print a vector as (x, y, z).
void printVec(const CRTVector& v) {
    std::cout << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

int main() {
    
    std::cout << " Task 2:cross products & parallelogram areas \n";

    CRTVector a1(3.5f, 0.0f, 0.0f), b1(1.75f, 3.5f, 0.0f);
    std::cout << "A x B = "; printVec(a1.cross(b1)); std::cout << "\n";

    CRTVector a2(3.0f, -3.0f, 1.0f), b2(4.0f, 9.0f, 3.0f);
    std::cout << "A x B = "; printVec(a2.cross(b2)); std::cout << "\n";

    // Parallelogram area = length of the cross product.
    std::cout << "Parallelogram area = " << a2.cross(b2).length() << "\n";

    CRTVector a3(3.0f, -3.0f, 1.0f), b3(-12.0f, 12.0f, -4.0f);
    std::cout << "Parallelogram area = " << a3.cross(b3).length()
              << "  (0 -> the vectors are parallel)\n\n";

    std::cout << "Task 3: triangle normals & areas\n";

    CRTTriangle t1(CRTVector(-1.75f, -1.75f, -3.0f),
                   CRTVector( 1.75f, -1.75f, -3.0f),
                   CRTVector( 0.0f,   1.75f, -3.0f));

    CRTTriangle t2(CRTVector(0.0f, 0.0f, -1.0f),
                   CRTVector(1.0f, 0.0f,  1.0f),
                   CRTVector(-1.0f, 0.0f, 1.0f));

    CRTTriangle t3(CRTVector(0.56f,  1.11f,   1.23f),
                   CRTVector(0.44f, -2.368f, -0.54f),
                   CRTVector(-1.56f, 0.15f,  -1.92f));

    CRTTriangle tris[] = { t1, t2, t3 };
    for (int i = 0; i < 3; ++i) {
        std::cout << "Triangle " << (i + 1) << ": normal = ";
        printVec(tris[i].normal());
        std::cout << ", area = " << tris[i].area() << "\n";
    }

    return 0;
}