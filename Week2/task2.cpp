#include <fstream>
#include <cstdlib>

static const int imageWidth = 1920;
static const int  imageHeight = 1080;

static const int maxColorComponent = 255;

//for ellipse representation the formula is (x-h)^2/center_x^2 + (y-k)^2/center_y^2 = 1 where (center_x,center_y) is the center of the ellipse x and y are the length of semi major and semi minor axes.
static const int center_x = 960;
static const int center_y = 540;
static const int x = 800;
static const int y = 300;



int main(){


    std::ofstream ppmFileStream("crt_output_image.ppm",std::ios::out|std::ios::binary);
    ppmFileStream <<"P3\n";
    ppmFileStream<< imageWidth << " " <<imageHeight<< "\n";
    ppmFileStream << maxColorComponent<< "\n";
    int r = 0;
    int g = 0;
    int b = 0;

    for(int rowIdx = 0; rowIdx< imageHeight ; rowIdx++){
        for(int colIdx = 0; colIdx< imageWidth ; colIdx++){
            
            double dx = colIdx - center_x;
            double dy = rowIdx - center_y;

            if ((dx * dx) / (x * x) + (dy * dy) / (y * y) <= 1.0) {
                // Inside the ellipse
                r = 0;
                g = 239;
                b = 255;
            } 
            else {
                // Outside the ellipse
                r = 0;
                g = 0;
                b = 0;
            }
            

            ppmFileStream << r << " "<< g << " " << b << " ";
            }
            ppmFileStream << "\n";

            
        }

    

    return 0;


}