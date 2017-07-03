#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "video_ivr_convert.h"
#include "malloc_memory_guard.h"
#include "file_handle_guard.h"
#include "image_to_yuv420p_loader.h"
#include "GeneratedStatus.h"

int IsNonYUV420Image(char const * pFilename)
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

int main(int argc, char * argv[])
{
    if (argc < 11)
    {
        fprintf(stderr, "Usage: ./GenerateIVRSlide pSrcImageFile, srcWidth, srcHeight, videoAlgo, videoProfile, dstWidth, dstHeight, bitRate, fps, pOutSlidePath, sleepIntervalMs, useNpf\n");
        return -1;
    }

    nice(10);
    
    char const * pSrcImageFile      = argv[1]; // the source image file
    int srcWidth                    = atoi(argv[2]);
    int srcHeight                   = atoi(argv[3]);
    VPMEX_PT videoAlgo              = static_cast<VPMEX_PT>(atoi(argv[4]));
    IVRH264Profile videoProfile     = static_cast<IVRH264Profile>(atoi(argv[5]));
    int dstWidth                    = atoi(argv[6]);
    int dstHeight                   = atoi(argv[7]);
    int bitRate                     = atoi(argv[8]);
    int fps                         = atoi(argv[9]);
    char const * pOutSlidePath      = argv[10];
    int sleepIntervalMs = 0;
    if (argc > 11)
    {
        sleepIntervalMs				= atoi(argv[11]);
    }
    int useNpf = 0;
    if (argc > 12)
    {
        sleepIntervalMs				= atoi(argv[12]);
    }

    const int maxWidthOfSrcImage = 2700;
    const int maxHeightOfSrcImage = 1500;
    
    int const inImageBufSize = srcWidth * srcHeight * 3 / 2;
    uint8_t * pImageBuf = NULL;

    ImageToYUV420pLoader imageLoader;
    if (IsNonYUV420Image(pSrcImageFile))
    {
        pImageBuf = imageLoader.LoadImageToYuvBuffer(pSrcImageFile, srcWidth, srcHeight);
        if (!pImageBuf)
        {
            fprintf(stderr, "failed to load image file %s\n", pSrcImageFile);
            return STATUS_FAILED_TO_LOAD_FILE;
        }

        if (srcWidth > maxWidthOfSrcImage || srcHeight > maxHeightOfSrcImage)
        {
            fprintf(stderr, "The resolution of source image[%d, %d] exceeds the highest resolution[%d, %d]\n",
                            srcWidth, srcHeight, maxWidthOfSrcImage, maxHeightOfSrcImage);
            return STATUS_INVALID_RESOLUTION_OF_SOURCE_IMAGE;
        }
    }
    else
    {
        pImageBuf = (uint8_t *)malloc(inImageBufSize);
        if (NULL == pImageBuf)
        {
            fprintf(stderr, "failed to alloc buffer");
            return STATUS_BAD_ALLOC;
        }

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
    
    VPMEX_FORMAT resolution;
    if (-1==GetVideoResolutionByDimension(dstWidth, dstHeight, resolution))
    {
        fprintf(stderr, "invalid target resolution [ %d x %d ]\n", dstWidth, dstHeight);
        return STATUS_INVALID_TARGET_RESOLUTION;
    }
    
    int rst = GenerateVideoIvr(pImageBuf, srcWidth, srcHeight, videoAlgo, videoProfile, resolution, bitRate, fps, pOutSlidePath, sleepIntervalMs, useNpf);
    
    return rst < 256 ? rst : (rst % 255); // for system() fork
}


