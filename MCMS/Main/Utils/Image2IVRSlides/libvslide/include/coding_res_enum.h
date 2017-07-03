#ifndef _CODING_RES_ENUM_HEADER_
#define _CODING_RES_ENUM_HEADER_

typedef enum
{
    VPMEX_FORMAT_SQCIF       = 0,    // 128x96    4:2:0
    VPMEX_FORMAT_QCIF        = 1,    // 176x144   4:2:0
    VPMEX_FORMAT_CIF        = 2,    // 352x288   4:2:0
    VPMEX_FORMAT_CIF4        = 3,    // 704x576   4:2:0
    VPMEX_FORMAT_CIF16       = 4,    // 1408x1152 4:2:0
    VPMEX_FORMAT_NTSC        = 5,    // 704x480   4:2:0
    VPMEX_FORMAT_NTSC_DIV4   = 6,    // 352x240   4:2:0
    VPMEX_FORMAT_SIF         = 6,    // 352x240   4:2:0
    VPMEX_FORMAT_VGA         = 7,    // 640x480   4:2:0
    VPMEX_FORMAT_VGA_DIV4    = 8,    // 320x240   4:2:0
    VPMEX_FORMAT_SVGA        = 9,    // 800x600   4:2:0
    VPMEX_FORMAT_SVGA_DIV4   = 10,   // 400x300   4:2:0
    VPMEX_FORMAT_XGA         = 11,   // 1024x768  4:2:0
    VPMEX_FORMAT_XGA_DIV4    = 12,   // 512x384   4:2:0
    VPMEX_FORMAT_XGA_DIV16   = 13,   // 256x192   4:2:0
    VPMEX_FORMAT_HHRVGA      = 14,   // 320x480   4:2:0
    VPMEX_FORMAT_2VRCIF      = 15,   // 352x576   4:2:0
    VPMEX_FORMAT_2VRSIF      = 16,   // 352x480   4:2:0
    VPMEX_FORMAT_SXGA        = 17,   // 1280x1024 4:2:0
    VPMEX_FORMAT_1280x720P   = 18,   // 1280x720  4:2:0
    VPMEX_FORMAT_512x288     = 19,   //  512x288  4:2:0
    VPMEX_FORMAT_672x384     = 20,   //  672x384  4:2:0
    VPMEX_FORMAT_768x448     = 21,   //  768x448  4:2:0
    VPMEX_FORMAT_848x480     = 22,   //  848x480  4:2:0
    VPMEX_FORMAT_1024x576    = 23,   // 1024x576  4:2:0
    VPMEX_FORMAT_704x240     = 24,   //  704x240  4:2:0
    VPMEX_FORMAT_740x288     = 25,   //  740x288  4:2:0
    VPMEX_FORMAT_640x368     = 26,   //  640x360  4:2:0
    VPMEX_FORMAT_432x240     = 27,   //  432x240  4:2:0
    VPMEX_FORMAT_1920x1080= 28,   // extended resolution
    VPMEX_FORMAT_320x180      = 29,
    VPMEX_FORMAT_640x360      = 30,
    VPMEX_FORMAT_960x540      = 31
    
    , VPMEX_FORMAT_1440x1080 = 32
    , VPMEX_FORMAT_960x720 = 33
    , VPMEX_FORMAT_864x480 = 34
    , VPMEX_FORMAT_480x352 = 35
    , VPMEX_FORMAT_736x400 = 36
    , VPMEX_FORMAT_448x336 = 37
    , VPMEX_FORMAT_448x240 = 38

    , VPMEX_FORMAT_212x160 = 39 //4:3 for lync2013
    , VPMEX_FORMAT_424x240 = 40 //16:9 for lync2013
    , VPMEX_FORMAT_480x270 = 41 //16:9 for lync2013
    , VPMEX_FORMAT_424x320 = 42 //4:3 for lync2013

    , VPMEX_FORMAT_1920x1072 = 43

    , VPMEX_FORMAT_COUNT,
} VPMEX_FORMAT;

typedef enum
{
    VPMEX_ASPECT_RATIO_DEFAULT     = 0,
    VPMEX_ASPECT_RATIO_4_TO_3      = 1,
    VPMEX_ASPECT_RATIO_16_TO_9     = 2,
    NUM_VPMEX_ASPECT_RATIO_MODES   = 3,
    VPMEX_ASPECT_RATIO_OTHER       = 3
} VPMEX_ASPECT_RATIO;

typedef enum
{
      VPMEX_SAMPLE_ASPECT_RATIO_UNSPECIFIED = 0
    , VPMEX_SAMPLE_ASPECT_RATIO_1V1 = 1
    , VPMEX_SAMPLE_ASPECT_RATIO_12V11 = 2
    , VPMEX_SAMPLE_ASPECT_RATIO_10V11 = 3
    , VPMEX_SAMPLE_ASPECT_RATIO_16V11 = 4
    , VPMEX_SAMPLE_ASPECT_RATIO_40V33 = 5
    , VPMEX_SAMPLE_ASPECT_RATIO_24V11 = 6
    , VPMEX_SAMPLE_ASPECT_RATIO_20V11 = 7
    , VPMEX_SAMPLE_ASPECT_RATIO_32V11 = 8
    , VPMEX_SAMPLE_ASPECT_RATIO_80V33 = 9
    , VPMEX_SAMPLE_ASPECT_RATIO_18V11 = 10
    , VPMEX_SAMPLE_ASPECT_RATIO_15V11 = 11
    , VPMEX_SAMPLE_ASPECT_RATIO_64V33 = 12
    , VPMEX_SAMPLE_ASPECT_RATIO_160V99 = 13
    , VPMEX_SAMPLE_ASPECT_RATIO_4V3 = 14
    , VPMEX_SAMPLE_ASPECT_RATIO_3V2 = 15
    , VPMEX_SAMPLE_ASPECT_RATIO_2V1 = 16

    , VPMEX_SAMPLE_ASPECT_RATIO_NORMAL_END
    , VPMEX_SAMPLE_ASPECT_RATIO_NORMAL_BEGIN = VPMEX_SAMPLE_ASPECT_RATIO_1V1
    , VPMEX_SAMPLE_ASPECT_RATIO_DEFAULT = VPMEX_SAMPLE_ASPECT_RATIO_1V1
    
    , VPMEX_SAMPLE_ASPECT_RATIO_EXTENDED = 255
} VPMEX_SAMPLE_ASPECT_RATIO;

//store if default is 16x9
extern int const g_16x9ResolutionTable[VPMEX_FORMAT_COUNT];


#endif
