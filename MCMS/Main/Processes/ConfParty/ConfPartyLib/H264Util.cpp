//+========================================================================+
//                            H264Util.CPP                                 |
//            Copyright 2003 Polycom Israel Ltd.                           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Ltd. and is protected by law.            |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H264UTIL.CPP                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yael                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |21/09/03    |                                                      |
//+========================================================================+

#include "H264Util.h"
#include "ObjString.h"
#include "TraceStream.h"

H264DetailsStruct CH264Details::g_H264DetailsTbl[H264_MAX_NUM_OF_LEVELS] =
{ //levelNumber  //levelValue    //defaultMBPS           //defaultFS           //defaultDPB           //defaultBR           //defaultCPB
  { eLevel_1,    H264_Level_1,   H264_L1_DEFAULT_MBPS,   H264_L1_DEFAULT_FS,   H264_L1_DEFAULT_DPB,   H264_L1_DEFAULT_BR,   H264_L1_DEFAULT_CPB,   H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_1_1,  H264_Level_1_1, H264_L1_1_DEFAULT_MBPS, H264_L1_1_DEFAULT_FS, H264_L1_1_DEFAULT_DPB, H264_L1_1_DEFAULT_BR, H264_L1_1_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_1_2,  H264_Level_1_2, H264_L1_2_DEFAULT_MBPS, H264_L1_2_DEFAULT_FS, H264_L1_2_DEFAULT_DPB, H264_L1_2_DEFAULT_BR, H264_L1_2_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_1_3,  H264_Level_1_3, H264_L1_3_DEFAULT_MBPS, H264_L1_3_DEFAULT_FS, H264_L1_3_DEFAULT_DPB, H264_L1_3_DEFAULT_BR, H264_L1_3_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_2,    H264_Level_2,   H264_L2_DEFAULT_MBPS,   H264_L2_DEFAULT_FS,   H264_L2_DEFAULT_DPB,   H264_L2_DEFAULT_BR,   H264_L2_DEFAULT_CPB,   H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_2_1,  H264_Level_2_1, H264_L2_1_DEFAULT_MBPS, H264_L2_1_DEFAULT_FS, H264_L2_1_DEFAULT_DPB, H264_L2_1_DEFAULT_BR, H264_L2_1_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_2_2,  H264_Level_2_2, H264_L2_2_DEFAULT_MBPS, H264_L2_2_DEFAULT_FS, H264_L2_2_DEFAULT_DPB, H264_L2_2_DEFAULT_BR, H264_L2_2_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_3,    H264_Level_3,   H264_L3_DEFAULT_MBPS,   H264_L3_DEFAULT_FS,   H264_L3_DEFAULT_DPB,   H264_L3_DEFAULT_BR,   H264_L3_DEFAULT_CPB,   H264_NON_INTERLEAVED_PACKETIZATION_MODE}  ,
  { eLevel_3_1,  H264_Level_3_1, H264_L3_1_DEFAULT_MBPS, H264_L3_1_DEFAULT_FS, H264_L3_1_DEFAULT_DPB, H264_L3_1_DEFAULT_BR, H264_L3_1_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_3_2,  H264_Level_3_2, H264_L3_2_DEFAULT_MBPS, H264_L3_2_DEFAULT_FS, H264_L3_2_DEFAULT_DPB, H264_L3_2_DEFAULT_BR, H264_L3_2_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_4,    H264_Level_4,   H264_L4_DEFAULT_MBPS,   H264_L4_DEFAULT_FS,   H264_L4_DEFAULT_DPB,   H264_L4_DEFAULT_BR,   H264_L4_DEFAULT_CPB,   H264_NON_INTERLEAVED_PACKETIZATION_MODE}  ,
  { eLevel_4_1,  H264_Level_4_1, H264_L4_1_DEFAULT_MBPS, H264_L4_1_DEFAULT_FS, H264_L4_1_DEFAULT_DPB, H264_L4_1_DEFAULT_BR, H264_L4_1_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_4_2,  H264_Level_4_2, H264_L4_2_DEFAULT_MBPS, H264_L4_2_DEFAULT_FS, H264_L4_2_DEFAULT_DPB, H264_L4_2_DEFAULT_BR, H264_L4_2_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLevel_5,    H264_Level_5,   H264_L5_DEFAULT_MBPS,   H264_L5_DEFAULT_FS,   H264_L5_DEFAULT_DPB,   H264_L5_DEFAULT_BR,   H264_L5_DEFAULT_CPB,   H264_NON_INTERLEAVED_PACKETIZATION_MODE}  ,
  { eLevel_5_1,  H264_Level_5_1, H264_L5_1_DEFAULT_MBPS, H264_L5_1_DEFAULT_FS, H264_L5_1_DEFAULT_DPB, H264_L5_1_DEFAULT_BR, H264_L5_1_DEFAULT_CPB, H264_NON_INTERLEAVED_PACKETIZATION_MODE},
  { eLastLevel,  0,              0,                      0,                    0,                     0,                    0,                     0} // Must be the last one.
};

////////////////////////////////////////////////////////////////////////////
//                        CH264Details
////////////////////////////////////////////////////////////////////////////
CH264Details::CH264Details(WORD levelValue)
{
  m_index = eLastLevel;

  if (levelValue != H264_MAX_NUM_OF_LEVELS)
  {
    for (int i = 0; i < eLastLevel; i++)
    {
      if (levelValue == g_H264DetailsTbl[i].levelValue)
      {
        m_index = i;
        break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////
// Description: Get CPB from MaxBrAndCpb, according to the formula in table 9
DWORD CH264Details::ConvertMaxBrAndCpbToMaxCpbProduct(WORD brAndCpb) const
{
  if (brAndCpb == (WORD)NA)
    return (DWORD)NA;

  if (m_index >= eLastLevel){
    PASSERT_AND_RETURN_VALUE(m_index, (DWORD)NA);
  }

  DWORD defaultCPBForLevel = g_H264DetailsTbl[m_index].defaultCPB;
  DWORD defaultBRForLevel  = g_H264DetailsTbl[m_index].defaultBR;
  DWORD numerator          = brAndCpb * CUSTOM_MAX_BR_FACTOR;
  DWORD maxCPB             = numerator / defaultBRForLevel;
  if ((numerator % defaultBRForLevel) > 0)
    maxCPB += 1;

  maxCPB *= defaultCPBForLevel;
  return maxCPB;
}

////////////////////////////////////////////////////////////////////////////
DWORD CH264Details::ConvertMaxFsToProduct(WORD fs) const
{
  return(((signed short)fs != NA) ? (fs*CUSTOM_MAX_FS_FACTOR) : (DWORD)NA);
}

////////////////////////////////////////////////////////////////////////////
WORD CH264Details:: GetDefaultMbpsAsDevision() const
{
  DWORD defaultMbps  = g_H264DetailsTbl[m_index].defaultMBPS;
  WORD  devisionMbps = GetMaxMbpsAsDevision(defaultMbps);
  return devisionMbps;
}

////////////////////////////////////////////////////////////////////////////
WORD CH264Details:: GetDefaultFsAsDevision() const
{
  DWORD defaultFs  = g_H264DetailsTbl[m_index].defaultFS;
  WORD  devisionFs = GetMaxFsAsDevision(defaultFs);
  return devisionFs;
}

////////////////////////////////////////////////////////////////////////////
WORD CH264Details:: GetDefaultDpbAsDevision() const
{
  DWORD defaultDpb  = g_H264DetailsTbl[m_index].defaultDPB;
  WORD  devisionDpb = GetMaxDpbAsDevision(defaultDpb);
  return devisionDpb;
}

////////////////////////////////////////////////////////////////////////////
WORD CH264Details:: GetDefaultBrAsDevision() const
{
  DWORD defaultBr  = g_H264DetailsTbl[m_index].defaultBR;
  WORD  devisionBr = GetMaxBrAsDevision(defaultBr);
  return devisionBr;
}

////////////////////////////////////////////////////////////////////////////
DWORD CH264Details::GetParamDefaultValueAsProductForLevel(BYTE customType) const
{
  switch (customType)
  {
    case CUSTOM_MAX_MBPS_CODE: return g_H264DetailsTbl[m_index].defaultMBPS;
    case CUSTOM_MAX_FS_CODE  : return g_H264DetailsTbl[m_index].defaultFS;
    case CUSTOM_MAX_DPB_CODE : return g_H264DetailsTbl[m_index].defaultDPB;
    case CUSTOM_MAX_BR_CODE  : return g_H264DetailsTbl[m_index].defaultBR;
    case CUSTOM_MAX_CPB_CODE : return g_H264DetailsTbl[m_index].defaultCPB;
  }
  PASSERTMSG((DWORD)customType, "Custom type unknown");
  return (DWORD)NA;
}

////////////////////////////////////////////////////////////////////////////
WORD CH264Details::GetParamDefaultValueAsDevisionForLevel(BYTE customType) const
{
  switch (customType)
  {
    case CUSTOM_MAX_MBPS_CODE: return GetDefaultMbpsAsDevision();
    case CUSTOM_MAX_FS_CODE  : return GetDefaultFsAsDevision();
    case CUSTOM_MAX_DPB_CODE : return GetDefaultDpbAsDevision();
    case CUSTOM_MAX_BR_CODE  : return GetDefaultBrAsDevision();
    case CUSTOM_MAX_CPB_CODE : PASSERTMSG_AND_RETURN_VALUE((DWORD)customType, "There is no devision for CPB", (WORD)NA);
  }
  PASSERTMSG((DWORD)customType, "Custom type unknown");
  return (WORD)NA;
}

////////////////////////////////////////////////////////////////////////////
BYTE CH264Details::IsParamSameAsDefaultForLevel(BYTE customType, DWORD param, EH264CustomViewTypes paramViewType) const
{
  BYTE bRes = FALSE;
  if (paramViewType == eCustomDevision)
  {
    WORD defualtDevisionVal = GetParamDefaultValueAsDevisionForLevel(customType);
    bRes = (defualtDevisionVal == param);
  }
  else
  {
    DWORD defualtProductVal = GetParamDefaultValueAsProductForLevel(customType);
    bRes = (defualtProductVal == param);
  }
  return bRes;
}

////////////////////////////////////////////////////////////////////////////
BYTE CH264Details::IsMbpsSameAsDefaultForLevel(DWORD maxMBPS) const
{
  DWORD defaultMbpsForLevel = g_H264DetailsTbl[m_index].defaultMBPS;
  return (defaultMbpsForLevel == maxMBPS);
}

////////////////////////////////////////////////////////////////////////////
BYTE CH264Details::IsFsSameAsDefaultForLevel(DWORD maxFS) const
{
  DWORD defaultFsForLevel = g_H264DetailsTbl[m_index].defaultFS;
  return (defaultFsForLevel == maxFS);
}

////////////////////////////////////////////////////////////////////////////
BYTE CH264Details::IsDpbSameAsDefaultForLevel(DWORD maxDPB) const
{
  DWORD defaultDpbForLevel = g_H264DetailsTbl[m_index].defaultDPB;
  return (defaultDpbForLevel == maxDPB);
}

////////////////////////////////////////////////////////////////////////////
BYTE CH264Details::IsBrSameAsDefaultForLevel(DWORD maxBR) const
{
  DWORD defaultBrForLevel = g_H264DetailsTbl[m_index].defaultBR;
  return (defaultBrForLevel == maxBR);
}

////////////////////////////////////////////////////////////////////////////
BYTE CH264Details::IsCpbSameAsDefaultForLevel(DWORD maxCPB) const
{
  DWORD defaultCpbForLevel = g_H264DetailsTbl[m_index].defaultCPB;
  return (defaultCpbForLevel == maxCPB);
}

////////////////////////////////////////////////////////////////////////////
// Description: check if the struct contain a known H264 level and if not - align to the closest lower level
void CH264Details::AlignH264LevelsToKnownLevelIfNeeded(h264CapStruct* pStruct)
{
  if (pStruct->levelValue < H264_Level_1)
    return;

  UINT8 lastKnownLevelValue = H264_Level_1;
  BYTE  bFoundMatchingLevel = FALSE;
  for (int i = 0; i < H264_MAX_NUM_OF_LEVELS && !bFoundMatchingLevel; i++)
  {
    H264DetailsStruct tmpStruct = g_H264DetailsTbl[i];
    if (pStruct->levelValue == tmpStruct.levelValue)
      bFoundMatchingLevel = TRUE;                 // Found Equal level - OK, no need to change
    else if (pStruct->levelValue > tmpStruct.levelValue)
      lastKnownLevelValue = tmpStruct.levelValue; // Found lower level, update the closest lower level
    else if (pStruct->levelValue < tmpStruct.levelValue)
    {
      bFoundMatchingLevel = TRUE;                 // Found Higher level, Exit - update the struct as there is no match
      pStruct->levelValue = lastKnownLevelValue;
    }
  }
}

////////////////////////////////////////////////////////////////////////////
H264DetailsStruct CH264Details::GetDetailsStructForLevel(DWORD level) const
{
  if (level > eLastLevel) {
    PASSERT_AND_RETURN_VALUE(level, g_H264DetailsTbl[eLastLevel]);
  }

  return g_H264DetailsTbl[level];
}
////////////////////////////////////////////////////////////////////////////
WORD CH264Details::GetLevelFromMaxFs(DWORD fs)
{
  // return the last level fs conains ; for example FS = 3840 (declares in custom) => level 3.1 (FS 3600) = HD 720
  WORD level_index = 0;
  while (level_index < (WORD)eLastLevel && fs >=  g_H264DetailsTbl[level_index].defaultFS)
    level_index++;

  // we take last level that contains
  WORD found_level = level_index;
  if (found_level > 0)
    found_level--;

  FTRACEINTO << "CH264Details::GetLevelFromMaxFs - fs:" << fs << ", level = " << g_H264DetailsTbl[found_level].levelValue << ", level_fs = " << g_H264DetailsTbl[found_level].defaultFS;
  return (found_level);
}
////////////////////////////////////////////////////////////////////////////
DWORD CH264Details::GetMaxDpbFromMaxFs(DWORD fs)
{
  WORD  found_level = GetLevelFromMaxFs(fs);
  DWORD max_dpb     = g_H264DetailsTbl[found_level].defaultDPB;
  float table_dpb   = max_dpb/1024;

  FTRACEINTO << "CH264Details::GetMaxDpbFromMaxFs - fs:" << fs << " => max_dpb:"<<  max_dpb << " (" << table_dpb << ")";
  return max_dpb;
}
