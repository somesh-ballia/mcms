#include "Image2IVRSlidesConvertor.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: ./ConvertImage2IVRSlide outSlidePath srcImageFile isOnlyForH.264 conversionMethod(0-low, 1-high, 2-low&high)\n");
        return -1;
    }

    char const * pOutSlidePath = argv[1]; // Output path for slides
    char const * pSrcImageFile = argv[2]; // the source image file
    int isOnlyForH264 = 0;
    CImage2IVRSlidesConvertor::ConversionMethod conversionMethod = CImage2IVRSlidesConvertor::LOW_HIGH_RES;
    if (argc > 3)
    {
        isOnlyForH264 = atoi(argv[3]); // If only generate the slides for H.264 or for all protocols
    }

    if (argc > 4)
    {
        conversionMethod = static_cast<CImage2IVRSlidesConvertor::ConversionMethod>(atoi(argv[4]));
    }

    nice(10);
    
    int result = CImage2IVRSlidesConvertor::GenerateIVRSlides(  pOutSlidePath
                                                                , pSrcImageFile
                                                                , isOnlyForH264
                                                                , conversionMethod);
    
    fprintf(stdout, "ConvertImage2IVRSlide result:%d\n", result); 

    return result < 256 ? result : (result % 255); // for system() fork
}


