#ifndef _CODEC_ALGO_ENUM_HERDER_
#define _CODEC_ALGO_ENUM_HERDER_

typedef enum
{
    VPMEX_PT_H261 = 0,
    VPMEX_PT_H263,
    VPMEX_PT_H263P,
    VPMEX_PT_H264,
    VPMEX_PT_VC1,
    VPMEX_PT_SVC,
    VPMEX_PT_LYNC_SVC,//Add by Farmerson
    VPMEX_PT_NUM
} VPMEX_PT;

enum
{
      ID_VIDEO_NONE  = -1
    , ID_VIDEO_H261  = 31
    , ID_VIDEO_H263  = 34
    , ID_VIDEO_H263p = 35
    , ID_VIDEO_H264  = 37
    , ID_VIDEO_RTV  = 38
    , ID_VIDEO_SVC  = 39
    , ID_VIDEO_LYNC  = 40
    , ID_VIDEO_MPEG4
};

#endif

