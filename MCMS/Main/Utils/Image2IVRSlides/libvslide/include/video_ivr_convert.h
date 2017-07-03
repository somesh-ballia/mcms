#ifndef __VIDEO_IVR_CONVERT_H__
#define __VIDEO_IVR_CONVERT_H__

#include "codec_algo_enum.h"
#include "coding_res_enum.h"

#include <stdint.h>

int GetVideoResolutionByDimension(int inWidth, int inHeight, VPMEX_FORMAT & resolution);

enum IVRH264Profile
{
      IVR_H264_PROFILE_BASELINE = 66
    , IVR_H264_PROFILE_HIGH = 100
    , IVR_H264_PROFILE_MAIN = 77
};

int GenerateVideoIvr(
      uint8_t const * yuvImage, int srcW, int srcH
    , VPMEX_PT algo
    , IVRH264Profile profile   // IVR_H264_PROFILE_XXX
    , VPMEX_FORMAT resolution
    , int bitRate
    , int fps
    , char const * ivrFileName
    , int sleepIntervalMs   // based on CIF's performance
    , int useNpf
);

#endif

