#include <fstream>
#include <cstdlib>

static const int imageWidth = 1920;
static const int  imageHeight = 1080;

static const int maxColorComponent = 255;

static const int gridCols = 4;
static const int gridRows = 4;
static const int rectWidth = imageWidth/gridCols;//480
static const int rectHeight = imageHeight/gridRows;//270

static const int totalColorGroups = 6;

//We can see that none of the colors are repeated in the same row or column.

int main(){


    std::ofstream ppmFileStream("crt_output_image.ppm",std::ios::out|std::ios::binary);
    ppmFileStream <<"P3\n";
    ppmFileStream<< imageWidth << " " <<imageHeight<< "\n";
    ppmFileStream << maxColorComponent<< "\n";


    


    for(int rowIdx = 0; rowIdx< imageHeight ; rowIdx++){
        for(int colIdx = 0; colIdx< imageWidth ; colIdx++){
            int rectRowIdx = rowIdx / rectHeight;
            int rectColIdx = colIdx / rectWidth;
            int cellIndex = rectRowIdx * gridCols + rectColIdx;
            int colorGroup = cellIndex % totalColorGroups;

            int r = 0;
            int g = 0;
            int b = 0;

            switch(colorGroup){
                case 0: 
                    r = rand() % (maxColorComponent + 1);
                    break;
                case 1: 
                    g = rand() % (maxColorComponent + 1);
                    break;
                case 2: 
                    r = rand() % (maxColorComponent + 1);
                    g = rand() % (maxColorComponent + 1);
                    break;
                case 3: 
                    b = rand() % (maxColorComponent + 1);
                    break;
                case 4: 
                    r = rand() % (maxColorComponent + 1);
                    b = rand() % (maxColorComponent + 1);
                    break;
                case 5: 
                    g = rand() % (maxColorComponent + 1);
                    b = rand() % (maxColorComponent + 1);
                    break;
            }
            ppmFileStream << r << " "<< g << " " << b << " ";
        }
        ppmFileStream << "\n";

    }

    return 0;


}