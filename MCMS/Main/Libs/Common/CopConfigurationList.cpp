//+========================================================================+
//=====================================================================================================================================////                      CopConfigurationList.cpp                           |
//               Copyright 1995 Pictel Technologies Ltd.                   |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CopConfigurationList.cpp                                    |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date    | Description                                            |
//-------------------------------------------------------------------------|
//                                                                         |
//+========================================================================+

#include "CopConfigurationList.h"
#include "ConfPartyApiDefines.h"
#include "H221.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"

static CopProfileInfo g_CopProfileInfo[] =
  {{"Event_Mode_1080_1728Kb",
     {{VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_HD1080p  , eCopVideoFrameRate_25, Xfer_1728},
      {VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_HD720p   , eCopVideoFrameRate_25, Xfer_832 },
      {VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_512 },
      {VIDEO_PROTOCOL_H263, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_384 }}},
  {"Event_Mode_720P_832Kb",
     {{VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_HD720p   , eCopVideoFrameRate_25, Xfer_832 },
      {VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_4CIF     , eCopVideoFrameRate_25, Xfer_768 },
      {VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_512 },
      {VIDEO_PROTOCOL_H263, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_384 }}},
  {"Event_Mode_4CIF_768Kb_4x3",
     {{VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_4CIF     , eCopVideoFrameRate_25, Xfer_768 },
      {VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_4CIF     , eCopVideoFrameRate_25, Xfer_512 },
      {VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_384 },
      {VIDEO_PROTOCOL_H263, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_256 }}},
  {"Event_Mode_4CIF_768Kb_16x9",
     {{VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_4CIF_16_9, eCopVideoFrameRate_25, Xfer_768 },
      {VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_4CIF_16_9, eCopVideoFrameRate_25, Xfer_384 },
      {VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_384 },
      {VIDEO_PROTOCOL_H263, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_256 }}},
  {"Event_Mode_1080p_HP_1024Kb",
     {{VIDEO_PROTOCOL_H264_HIGH_PROFILE, eCopLevelEncoderVideoFormat_HD1080p  , eCopVideoFrameRate_25, Xfer_1024},
      {VIDEO_PROTOCOL_H264_HIGH_PROFILE, eCopLevelEncoderVideoFormat_HD720p   , eCopVideoFrameRate_25, Xfer_512 },
      {VIDEO_PROTOCOL_H264_HIGH_PROFILE, eCopLevelEncoderVideoFormat_4CIF     , eCopVideoFrameRate_25, Xfer_384 },
      {VIDEO_PROTOCOL_H263, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_384 }}},
  {"Event_Mode_720P_HP_512Kb",
     {{VIDEO_PROTOCOL_H264_HIGH_PROFILE, eCopLevelEncoderVideoFormat_HD720p   , eCopVideoFrameRate_25, Xfer_512 },
      {VIDEO_PROTOCOL_H264_HIGH_PROFILE, eCopLevelEncoderVideoFormat_4CIF     , eCopVideoFrameRate_25, Xfer_384 },
      {VIDEO_PROTOCOL_H264_HIGH_PROFILE, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_256 },
      {VIDEO_PROTOCOL_H263, eCopLevelEncoderVideoFormat_CIF      , eCopVideoFrameRate_25, Xfer_256 }}}
};
//=====================================================================================================================================//
CCOPConfigurationList::CCOPConfigurationList()
{
  m_pVideoParams[0] = new CCopVideoParams(VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_HD1080p, eCopVideoFrameRate_25, Xfer_1728);
  m_pVideoParams[1] = new CCopVideoParams(VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_HD720p , eCopVideoFrameRate_25, Xfer_832 );
  m_pVideoParams[2] = new CCopVideoParams(VIDEO_PROTOCOL_H264, eCopLevelEncoderVideoFormat_CIF    , eCopVideoFrameRate_25, Xfer_512 );
  m_pVideoParams[3] = new CCopVideoParams(VIDEO_PROTOCOL_H263, eCopLevelEncoderVideoFormat_CIF    , eCopVideoFrameRate_25, Xfer_384 );
  m_aspectRatio= AspectRatio_16X9;
}
//=====================================================================================================================================//
CCOPConfigurationList::CCOPConfigurationList(int profileIndex)
{
  CopProfileInfo* profileInfo = GetProfileInfo(profileIndex);

  for (int level = 0; level < NUMBER_OF_COP_LEVELS; ++level)
    m_pVideoParams[level] = new CCopVideoParams(profileInfo->levelInfo[level].videoProtocol,
                                                profileInfo->levelInfo[level].videoFormat,
                                                profileInfo->levelInfo[level].videoFrameRate,
                                                profileInfo->levelInfo[level].videoBitRate);
  m_aspectRatio = AspectRatio_16X9;
}
//=====================================================================================================================================//
CCOPConfigurationList::CCOPConfigurationList(const CCOPConfigurationList &other)
                      :CPObject(other)
{
  for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
    m_pVideoParams[i] = NULL;
  *this = other;
}
//=====================================================================================================================================//
CCOPConfigurationList& CCOPConfigurationList::operator=(const CCOPConfigurationList &other)
{
  if(this != &other)
  {
    for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
    {
      POBJDELETE(m_pVideoParams[i]);
      if (other.m_pVideoParams[i])
        m_pVideoParams[i] = new CCopVideoParams(*(other.m_pVideoParams[i]));
    }
    m_aspectRatio = other.m_aspectRatio;
  }
  return *this;
}
//=====================================================================================================================================//
CCOPConfigurationList::~CCOPConfigurationList()
{
  for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
    POBJDELETE(m_pVideoParams[i]);
}
//=====================================================================================================================================//
void CCOPConfigurationList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  CXMLDOMElement* pTempNode = pFatherNode->AddChildNode("COP_CONFIGURATION_LIST");

  pTempNode->AddChildNode("ASPECT_RATIO", m_aspectRatio, ASPECT_RATIO_ENUM);

  for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
    if (m_pVideoParams[i])
      m_pVideoParams[i]->SerializeXml(pTempNode);
}
//=====================================================================================================================================//
int CCOPConfigurationList::DeSerializeXml(CXMLDOMElement *pFatherNode, char *pszError)
{
  int nStatus = STATUS_OK;
  CXMLDOMElement *pChildNode = NULL;

  GET_VALIDATE_CHILD(pFatherNode, "ASPECT_RATIO", &m_aspectRatio, ASPECT_RATIO_ENUM);

  if (nStatus != STATUS_OK)
    return nStatus;

  GET_FIRST_CHILD_NODE(pFatherNode, "COP_CONFIGURATION", pChildNode);
  int i = 0;
  while (pChildNode && (i < NUMBER_OF_COP_LEVELS))
  {
    if (m_pVideoParams[i])
    {
      nStatus = m_pVideoParams[i]->DeSerializeXml(pChildNode, pszError);
      if (nStatus != STATUS_OK)
        return nStatus;
    }
    i++;
    GET_NEXT_CHILD_NODE(pFatherNode, "COP_CONFIGURATION", pChildNode);

  }

  return nStatus;
}
//=====================================================================================================================================//
void CCOPConfigurationList::Serialize(WORD format, std::ostream &ostr) const
{
  ostr << (WORD)m_aspectRatio << "\n";
  BYTE bIsVideoParams;
  for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
  {
    bIsVideoParams = (m_pVideoParams[i]) ? TRUE : FALSE;
    ostr << (WORD)bIsVideoParams << "\n";
    if (bIsVideoParams)
      m_pVideoParams[i]->Serialize(format, ostr);
  }
}
//=====================================================================================================================================//
void CCOPConfigurationList::DeSerialize(WORD format, std::istream &istr)
{
  WORD tmp;
  BYTE bIsVideoParams;

  istr >> tmp;
  m_aspectRatio = (BYTE)tmp;

  for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
  {
    istr >> tmp;
    bIsVideoParams = (BYTE)tmp;
    if (bIsVideoParams)
      m_pVideoParams[i]->DeSerialize(format, istr);
  }
}
//=====================================================================================================================================//
void CCOPConfigurationList::Serialize(WORD format, CSegment& seg) const
{
  if (format != NATIVE)
    PASSERT(1);
  COstrStream pOstr;
  Serialize(format, pOstr);
  pOstr.Serialize(seg);
}
//=====================================================================================================================================//
void CCOPConfigurationList::DeSerialize(WORD format, CSegment &seg)
{
  if (format != NATIVE)
    PASSERT(1);
  CIstrStream istr(seg);
  DeSerialize(format, istr);
}
//=====================================================================================================================================//
STATUS CCOPConfigurationList::CheckReservRangeValidity(BYTE &errorCode)
{
  errorCode = 0;
  STATUS status = STATUS_OK;
  switch (m_aspectRatio)
  {
    case AspectRatio_4X3:
    case AspectRatio_16X9:
      break;
    default:
    {
      errorCode = 200;
      return STATUS_OUT_OF_RANGE;
    }
  }

  for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
  {
    status = m_pVideoParams[i]->CheckReservRangeValidity(errorCode);
    if (STATUS_OK != status)
    {
      PTRACE2INT(eLevelInfoNormal,"CCOPConfigurationList::CheckReservRangeValidity : failure status, indx = ",i);
      return status;
    }
  }
  return STATUS_OK;
}
//=====================================================================================================================================//
void CCOPConfigurationList::Dump(const char* title, WORD level) const
{
  COstrStream msg;
  if (title != NULL)
    msg << title;
  Dump(msg);
  PTRACE(level,msg.str().c_str());
}
//=====================================================================================================================================//
void CCOPConfigurationList::Dump(std::ostream& msg) const
{
  msg << "\nAspect Ratio: " << (WORD)m_aspectRatio;
  for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
  {
    msg << "\n\nLevel number: " << i;
    m_pVideoParams[i]->Dump(msg);
  }
}
//=====================================================================================================================================//
BYTE CCOPConfigurationList::IsHighProfileFoundInOneLevelAtLeast()
{
  for (int i = 0; i < NUMBER_OF_COP_LEVELS; i++)
    if (m_pVideoParams[i] && m_pVideoParams[i]->GetProtocol() == VIDEO_PROTOCOL_H264_HIGH_PROFILE)
      return TRUE;
  return FALSE;
}
//=====================================================================================================================================//
void CCOPConfigurationList::SetVideoModeForTest(BYTE videoProtocol, BYTE format, BYTE framerate, BYTE bitRate, BYTE index)
{
  m_pVideoParams[index]->SetProtocol(videoProtocol);
  m_pVideoParams[index]->SetFormat(format);
  m_pVideoParams[index]->SetFrameRate(framerate);
  m_pVideoParams[index]->SetBitRate(bitRate);
}
//=====================================================================================================================================//
int CCOPConfigurationList::GetProfilesCount()
{
  return sizeof(g_CopProfileInfo)/sizeof(CopProfileInfo);
}
//=====================================================================================================================================//
CopProfileInfo* CCOPConfigurationList::GetProfileInfo(int profileIndex)
{
  return &g_CopProfileInfo[profileIndex];
}
//=====================================================================================================================================//
CopProfileInfo* CCOPConfigurationList::GetProfileInfo(const char* profileName)
{
  for (int profileIndex = 0; profileIndex < GetProfilesCount(); ++profileIndex)
    if (strncmp(GetProfileInfo(profileIndex)->profileName, profileName, H243_NAME_LEN) == 0)
      return GetProfileInfo(profileIndex);
  return NULL;
}
