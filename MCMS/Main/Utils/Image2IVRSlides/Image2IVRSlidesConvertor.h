#ifndef IMAGE_2_IVR_SLIDES_CONVERTOR_H
#define IMAGE_2_IVR_SLIDES_CONVERTOR_H

#include "video_ivr_convert.h"
#include "GeneratedStatus.h"

class CImage2IVRSlidesConvertor
{
public:
    
    typedef enum
    {
        LOW_RES = 0
        , HIGH_RES
        , LOW_HIGH_RES
    }ConversionMethod;
    
    // This function is used to generate IVR slides
    // Input value:
    //      pOutSlidePath: the output path for the generated slides
    //      pSrcImageFile: the source image file
    //      isOnlyForH264: If only generate the slides for H.264 or for all protocols, such as H.263, H.264, RTV and TIP
    // Return value: the status code for generating slides
    static int GenerateIVRSlides(   char const * const pOutSlidePath
                                                    , char const * const pSrcImageFile
                                                    , int isOnlyForH264
                                                    , ConversionMethod conversionMethod);

    
private:
    typedef struct
    {
        VPMEX_PT            videoAlgo; // video protocol
        IVRH264Profile      videoProfile; // video profile
        int                 width; // width of slides
        int                 height; // height of slides
        int                 bitRate; // bit rate of slides
        int                 fps; // frame rate
        char const *        pSlideFileName; // file name of slide
    }IVRSlideType;
    
    static const IVRSlideType   m_ivrSlidesParams[];
    static const int            m_nMinBitRate;
    static const int            m_nMaxBitRate;
    static const int            m_nMinFps; // minimum frame rate
    static const int            m_nMaxFps; // maximum frame rate
    static const int            m_nMinWidthOfHighRes;
    static const int            m_nMinHeightOfHighRes;

    // Check if a file exist
    static int IsRegularFileExisting(char const * pFilename);
    // Check if a video dimension is valid
    static int IsVideoDimenInBoundary(int w, int h, int maxW, int maxH);
    // Check if a file is YUV or not
    static int IsNonYUV420Image(char const * pFilename);
};

#endif

