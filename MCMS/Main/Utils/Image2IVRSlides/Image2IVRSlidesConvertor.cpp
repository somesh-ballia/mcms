#include "Image2IVRSlidesConvertor.h"

#include "malloc_memory_guard.h"
#include "file_handle_guard.h"
#include "image_to_yuv420p_loader.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>

#include "Macros.h"

const CImage2IVRSlidesConvertor::IVRSlideType CImage2IVRSlidesConvertor::m_ivrSlidesParams[]
    =   {
            // videoAlgo        videoProfile               width height  bitRate  fps pSlideFileName
            {VPMEX_PT_H263,     IVR_H264_PROFILE_BASELINE,   176,  144,  384000,  30, "S_PTC263_RESQCIF.slide"},
            {VPMEX_PT_H263,     IVR_H264_PROFILE_BASELINE,   352,  288,  384000,  30, "S_PTC263_RESCIF.slide"},
            {VPMEX_PT_H263,     IVR_H264_PROFILE_BASELINE,   704,  576,  384000,  30, "S_PTC263_RES4CIF.slide"},
            {VPMEX_PT_H264,     IVR_H264_PROFILE_BASELINE,   176,  144,  384000,  30, "S_PTC264_RESQCIF.slide"},
            {VPMEX_PT_H264,     IVR_H264_PROFILE_BASELINE,   352,  288,  384000,  30, "S_PTC264_RESCIF.slide"},
            {VPMEX_PT_H264,     IVR_H264_PROFILE_BASELINE,   704,  576,  384000,  30, "S_PTC264_RESSD.slide"},
            {VPMEX_PT_H264,     IVR_H264_PROFILE_BASELINE,   1280, 720,  1024000, 30, "S_PTC264_RESHD720.slide"},
            {VPMEX_PT_H264,     IVR_H264_PROFILE_BASELINE,   1920, 1080, 2048000, 30, "S_PTC264_RESHD1080.slide"},
            {VPMEX_PT_H264,     IVR_H264_PROFILE_MAIN,       1280, 720,  1024000, 30, "S_TIP264_RESHD720.slide"},
            {VPMEX_PT_H264,     IVR_H264_PROFILE_MAIN,       1920, 1072, 2048000, 30, "S_TIP264_RESHD1080.slide"},
            {VPMEX_PT_VC1,      IVR_H264_PROFILE_BASELINE,   176,  144,  384000,  30, "S_PTCRTV_RESQCIF.slide"},
            {VPMEX_PT_VC1,      IVR_H264_PROFILE_BASELINE,   352,  288,  384000,  30, "S_PTCRTV_RESCIF.slide"},
            {VPMEX_PT_VC1,      IVR_H264_PROFILE_BASELINE,   640,  480,  384000,  30, "S_PTCRTV_RESSD.slide"},
            {VPMEX_PT_VC1,      IVR_H264_PROFILE_BASELINE,   1280, 720,  1024000, 30, "S_PTCRTV_RESHD720.slide"}
        };


const int CImage2IVRSlidesConvertor::m_nMinBitRate  = 32000;
const int CImage2IVRSlidesConvertor::m_nMaxBitRate  = 8 * 1024 * 1000;
const int CImage2IVRSlidesConvertor::m_nMinFps      = 2;
const int CImage2IVRSlidesConvertor::m_nMaxFps      = 60;
const int CImage2IVRSlidesConvertor::m_nMinWidthOfHighRes      = 1280;
const int CImage2IVRSlidesConvertor::m_nMinHeightOfHighRes     = 720;

int CImage2IVRSlidesConvertor::GenerateIVRSlides(   char const * const pOutSlidePath
                                                        , char const * const pSrcImageFile
                                                        , int isOnlyForH264
                                                        , ConversionMethod conversionMethod )
{
    if (!IsRegularFileExisting(pSrcImageFile))
    {
        fprintf(stderr, "Invalid input image file %s\n", pSrcImageFile);
        return STATUS_INVALID_INPUT_FILE;
    }

    int srcWidth = 1280;
    int srcHeight = 720;

    /*
    // PartA:Disable this part for loading image is transfered to another program
    
    int const inImageBufSize = srcWidth * srcHeight * 3 / 2;
    uint8_t * pImageBuf = 0;

    ImageToYUV420pLoader imageLoader;
    if (IsNonYUV420Image(pSrcImageFile))
    {
        pImageBuf = imageLoader.LoadImageToYuvBuffer(pSrcImageFile, srcWidth, srcHeight);
        if (!pImageBuf)
        {
            fprintf(stderr, "failed to load image file %s\n", pSrcImageFile);
            return STATUS_FAILED_TO_LOAD_FILE;
        }
    }
    else
    {
        pImageBuf = (uint8_t *)malloc(inImageBufSize);
        FILE * fpImage = fopen(pSrcImageFile, "rb");
        if (!fpImage)
        {
            fprintf(stderr, "failed to open file %s\n", pSrcImageFile);
            return STATUS_FAILED_TO_OPEN_FILE;
        }
        memset(pImageBuf, 128, inImageBufSize);
        fread(pImageBuf, 1, inImageBufSize, fpImage);
        fclose(fpImage);
    }
    */
        
    for (int i=0; i<static_cast<int>(ARRAYSIZE(m_ivrSlidesParams)); ++i)
    {
        if (isOnlyForH264 && (VPMEX_PT_H264!=m_ivrSlidesParams[i].videoAlgo || IVR_H264_PROFILE_BASELINE!=m_ivrSlidesParams[i].videoProfile))
        {
            continue;
        }

        if (LOW_RES==conversionMethod && m_ivrSlidesParams[i].width>=m_nMinWidthOfHighRes && m_ivrSlidesParams[i].height>=m_nMinHeightOfHighRes)
        {
            continue;
        }
        else if (HIGH_RES==conversionMethod && m_ivrSlidesParams[i].width<m_nMinWidthOfHighRes && m_ivrSlidesParams[i].height<m_nMinHeightOfHighRes)
        {
            continue;
        }
        
        std::string outFile = pOutSlidePath;
        outFile += "/";
        outFile += m_ivrSlidesParams[i].pSlideFileName;
        
        VPMEX_PT videoAlgo = m_ivrSlidesParams[i].videoAlgo;
        if ((videoAlgo<0) || (videoAlgo>=VPMEX_PT_NUM))
        {
            fprintf(stderr, "invalid video algo %d\n", videoAlgo);
            return STATUS_INVALID_VIDEO_ALGO;
        }

        IVRH264Profile videoProfile = m_ivrSlidesParams[i].videoProfile;
        if (   (IVR_H264_PROFILE_BASELINE != videoProfile) 
            && (IVR_H264_PROFILE_MAIN     != videoProfile) 
            && (IVR_H264_PROFILE_HIGH     != videoProfile))
        {
            fprintf(stderr, "invalid video profile %d [ 66, 77, 100 ]\n", videoProfile);
            return STATUS_INVALID_VIDEO_PROFILE;
        }

        int dstWidth = m_ivrSlidesParams[i].width;
        int dstHeight = m_ivrSlidesParams[i].height;

        /*
        // PartB:Disable this part for loading image is transfered to another program
        
        VPMEX_FORMAT resolution;
        if (-1==GetVideoResolutionByDimension(dstWidth, dstHeight, resolution))
        {
            fprintf(stderr, "invalid target resolution [ %d x %d ]\n", dstWidth, dstHeight);
            return STATUS_INVALID_TARGET_RESOLUTION;
        }
        */
            
        int bitRate = m_ivrSlidesParams[i].bitRate;
        if (bitRate<m_nMinBitRate || bitRate>m_nMaxBitRate)
        {
            fprintf(stderr, "invalid target bit rate %d\n", bitRate);
            return STATUS_INVALID_TARGET_BIT_RATE;
        }

        int fps = m_ivrSlidesParams[i].fps;
        if (fps<m_nMinFps || fps>m_nMaxFps)
        {
            fprintf(stderr, "invalid fps %d\n", fps);
            return STATUS_INVALID_TARGET_FRAME_RATE;
        }

        static int sleepIntervalMs = 0;
        
        static int useNpf = 0;
        
        fprintf(stdout, "input=%s, output=%s, in [ %d x %d ], out dimension [ %d x %d ], algo %d, profile %d, bit rate %d, fps %d, sleepInverval=%d ms, useNpf=%d\n"
            , pSrcImageFile
            , outFile.c_str()
            , srcWidth, srcHeight
            , dstWidth, dstHeight
            , videoAlgo, videoProfile
            , bitRate, fps
            , sleepIntervalMs
            , useNpf
            );

        ////////////////////////////////////////////////////////////////////////////////////////
        // For some unknow reason, the following comment function will throw fragment error in the loop in RMX1500/2000/4000
        // however, it works well in soft platform.
        // So we convert the slide with another program
//        GenerateVideoIvr(pImageBuf, srcWidth, srcHeight, videoAlgo, videoProfile, resolution, bitRate, fps, outFile.c_str(), sleepIntervalMs, useNpf);

        std::stringstream sstr;
        sstr    << "Bin/GenerateIVRSlide "
                << pSrcImageFile << " "
                << srcWidth << " "
                << srcHeight << " "
                << videoAlgo << " "
                << videoProfile << " "
                << dstWidth << " "
                << dstHeight << " "
                << bitRate << " "
                << fps << " "
                << outFile.c_str() << " "
                << sleepIntervalMs << " "
                << useNpf << " "
                << "&> /dev/null";

        int res = system(sstr.str().c_str());
        fprintf(stdout, "GenerateIVRSlide result %d\n", res);
        if (STATUS_SUCCESS != res)
        {
            return WEXITSTATUS(res);
        }
        /////////////////////////////////////////////////////////////////////////////////////////////////
    }


    return STATUS_SUCCESS;
}


int CImage2IVRSlidesConvertor::IsRegularFileExisting(char const * pFilename)
{
    struct stat statbuf;
    if (-1==stat(pFilename, &statbuf))
    {
        return false;
    }

    return (S_IFREG==(statbuf.st_mode & S_IFMT));
}


int CImage2IVRSlidesConvertor::IsVideoDimenInBoundary(int w, int h, int maxW, int maxH)
{
    return ((w>=16) && (w<=maxW) && (h>=16) && (h<=maxH));
}


int CImage2IVRSlidesConvertor::IsNonYUV420Image(char const * pFilename)
{
    if (strstr(pFilename, ".png") 
        || strstr(pFilename, ".jpg") 
        || strstr(pFilename, ".jpeg") 
        || strstr(pFilename, ".bmp")
        )
    {
        return true;
    }
    else
    {
        return false;
    }
}

