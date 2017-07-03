#ifndef _H264UTIL
#define _H264UTIL

#include "PObject.h"
#include  "H264.h"
#include "Capabilities.h"

#ifndef NA
  #define NA                    (-1)                            // not available
#endif // ifndef NA

#define  H264_MAX_NUM_OF_LEVELS 16

//--------------------------------------------------------------------------
inline DWORD ConvertMaxMbpsToProduct(WORD mbps)
{
  return(((signed short)mbps != NA) ? (mbps*CUSTOM_MAX_MBPS_FACTOR) : (DWORD)NA);
}

//--------------------------------------------------------------------------
inline WORD GetMaxMbpsAsDevision(DWORD mbps)
{
  if ((signed long)mbps != NA)
    return (WORD)((mbps % CUSTOM_MAX_MBPS_FACTOR) ? (mbps/CUSTOM_MAX_MBPS_FACTOR + 1) : (mbps/CUSTOM_MAX_MBPS_FACTOR));
  else
    return (WORD)NA;
}

//--------------------------------------------------------------------------
inline DWORD ConvertMaxFsToProduct(WORD fs)
{
  return(((signed short)fs != NA) ? (fs*CUSTOM_MAX_FS_FACTOR) : (DWORD)NA);
}

//--------------------------------------------------------------------------
inline WORD GetMaxFsAsDevision(DWORD fs)
{
  if ((signed long)fs != NA)
    return (WORD)((fs % CUSTOM_MAX_FS_FACTOR) ? (fs/CUSTOM_MAX_FS_FACTOR + 1) : (fs/CUSTOM_MAX_FS_FACTOR));
  else
    return (WORD)NA;
}

//--------------------------------------------------------------------------
inline DWORD ConvertMaxDpbToProduct(WORD dpb)
{
  return (((signed short)dpb != NA) ? (dpb*CUSTOM_MAX_DPB_FACTOR) : (DWORD)NA);
}

//--------------------------------------------------------------------------
inline WORD GetMaxDpbAsDevision(DWORD dpb)
{
  if ((signed long)dpb != NA)
    return (WORD)((dpb % CUSTOM_MAX_DPB_FACTOR) ? (dpb/CUSTOM_MAX_DPB_FACTOR + 1) : (dpb/CUSTOM_MAX_DPB_FACTOR));
  else
    return (WORD)NA;
}

//--------------------------------------------------------------------------
// BASIC FORMULA: BrAndCpb * CUSTOM_MAX_BR_FACTOR = BR
inline DWORD ConvertMaxBrAndCpbToMaxBrProduct(WORD brAndCpb)    // translates BrAndCpb -> Br
{
  return (((signed short)brAndCpb != NA) ? (brAndCpb*CUSTOM_MAX_BR_FACTOR) : (DWORD)NA);
}

//--------------------------------------------------------------------------
inline WORD GetMaxBrAsDevision(DWORD br)
{
  if ((signed long)br != NA)
    return (WORD)((br % CUSTOM_MAX_BR_FACTOR) ? (br/CUSTOM_MAX_BR_FACTOR + 1) : (br/CUSTOM_MAX_BR_FACTOR));
  else
    return (WORD)NA;
}

//--------------------------------------------------------------------------
inline WORD ConvertMaxBrToMaxBrAndCpb(DWORD br)                // translates Br -> BrAndCpb
{
  return GetMaxBrAsDevision(br);
}

//--------------------------------------------------------------------------
inline DWORD ConvertVideoResolution2FS(APIS32 width, APIS32 height)
{
  return (width*height)/(CUSTOM_MAX_FS_FACTOR*2);
}

//--------------------------------------------------------------------------
inline DWORD ConvertVideoResolution2FS(APIS32 width, APIS32 height, float frameRate)
{
  float FS = (width*height)/CUSTOM_MAX_FS_FACTOR;
  return (DWORD)((FS*frameRate)/CUSTOM_MAX_MBPS_FACTOR+0.5f);
}

typedef enum
{
  eCustomProduct,
  eCustomDevision
} EH264CustomViewTypes;

typedef enum
{
  eLevel_1   = 0,
  eLevel_1_1 = 1,
  eLevel_1_2 = 2,
  eLevel_1_3 = 3,
  eLevel_2   = 4,
  eLevel_2_1 = 5,
  eLevel_2_2 = 6,
  eLevel_3   = 7,
  eLevel_3_1 = 8,
  eLevel_3_2 = 9,
  eLevel_4   = 10,
  eLevel_4_1 = 11,
  eLevel_4_2 = 12,
  eLevel_5   = 13,
  eLevel_5_1 = 14,
  eLastLevel = 15
} EH264Levels;

typedef struct
{
  EH264Levels levelNumber;
  WORD        levelValue;
  DWORD       defaultMBPS;
  DWORD       defaultFS;
  DWORD       defaultDPB;
  DWORD       defaultBR;
  DWORD       defaultCPB;
  WORD        packetizationMode;
} H264DetailsStruct;


////////////////////////////////////////////////////////////////////////////
//                        CH264Details
////////////////////////////////////////////////////////////////////////////
class CH264Details : public CPObject
{
  CLASS_TYPE_1(CH264Details, CPObject)

public:
                      CH264Details(WORD levelValue);
  virtual            ~CH264Details() { }
  virtual const char* NameOf() const                   { return "CH264Details"; }

  // translation between parameters:
  DWORD               ConvertMaxBrAndCpbToMaxCpbProduct(WORD brAndCpb) const;
  DWORD               ConvertMaxFsToProduct(WORD fs) const;

  // getting defaults values from the table:
  DWORD               GetLevelNumber()    const        { return g_H264DetailsTbl[m_index].levelNumber; }

  // those functions return values with the custom common product
  DWORD               GetDefaultMbpsAsProduct()  const { return g_H264DetailsTbl[m_index].defaultMBPS; }
  DWORD               GetDefaultFsAsProduct()    const { return g_H264DetailsTbl[m_index].defaultFS;   }
  DWORD               GetDefaultDpbAsProduct()   const { return g_H264DetailsTbl[m_index].defaultDPB;  }
  DWORD               GetDefaultBrAsProduct()    const { return g_H264DetailsTbl[m_index].defaultBR;   }
  DWORD               GetDefaultCpbAsProduct()   const { return g_H264DetailsTbl[m_index].defaultCPB;  }
  DWORD               GetParamDefaultValueAsProductForLevel(BYTE customType) const;

  // those functions return values without the custom common product
  WORD                GetDefaultMbpsAsDevision()  const;
  WORD                GetDefaultFsAsDevision()    const;
  WORD                GetDefaultDpbAsDevision()   const;
  WORD                GetDefaultBrAsDevision()    const;
  WORD                GetParamDefaultValueAsDevisionForLevel(BYTE customType) const;

  // comparing a given parameter to the default value from the table:
  BYTE                IsParamSameAsDefaultForLevel(BYTE customType, DWORD param, EH264CustomViewTypes paramViewType) const;
  BYTE                IsMbpsSameAsDefaultForLevel(DWORD maxMBPS) const;
  BYTE                IsFsSameAsDefaultForLevel(DWORD maxFS)     const;
  BYTE                IsDpbSameAsDefaultForLevel(DWORD maxDPB)   const;
  BYTE                IsBrSameAsDefaultForLevel(DWORD maxBR)     const;
  BYTE                IsCpbSameAsDefaultForLevel(DWORD maxCPB)   const;
  H264DetailsStruct   GetDetailsStructForLevel(DWORD level)      const;

  static void         AlignH264LevelsToKnownLevelIfNeeded(h264CapStruct* pStruct);
  static WORD         GetLevelFromMaxFs(DWORD fs);
  static DWORD        GetMaxDpbFromMaxFs(DWORD fs);

private:
  static H264DetailsStruct g_H264DetailsTbl[H264_MAX_NUM_OF_LEVELS];
  int                      m_index;
};

#endif // _H264UTIL

