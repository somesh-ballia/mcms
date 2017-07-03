//+========================================================================+
//                        CopVideoParams.cpp                               |
//               Copyright 1995 Pictel Technologies Ltd.                   |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CopVideoParams.cpp                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date    | Description                                            |
//-------------------------------------------------------------------------|
//                                                                         |
//+========================================================================+
#include "CopVideoParams.h"
#include "ConfPartyApiDefines.h"
#include "IpChannelParams.h"
#include "H221.h"
#include "StringsMaps.h"
#include "StatusesGeneral.h"

//=====================================================================================================================================//
CCopVideoParams::CCopVideoParams()
{
  videoProtocol   = VIDEO_PROTOCOL_H264;
  videoFormat     = eCopLevelEncoderVideoFormat_HD1080p;
  videoFrameRate  = eCopVideoFrameRate_25;
  videoBitRate    = Xfer_4096;
}
//=====================================================================================================================================//
CCopVideoParams::CCopVideoParams(BYTE protocol, BYTE format, BYTE frameRate, BYTE bitRate)
{
  videoProtocol  = protocol;
  videoFormat    = format;
  videoFrameRate = frameRate;
  videoBitRate   = bitRate;
}
//=====================================================================================================================================//
CCopVideoParams::CCopVideoParams(const CCopVideoParams &other) :
  CPObject(other)
{
  *this = other;
}
//=====================================================================================================================================//
CCopVideoParams& CCopVideoParams::operator=(const CCopVideoParams &other)
{
  if (this != &other)
  {
    videoProtocol   = other.videoProtocol;
    videoFormat     = other.videoFormat;
    videoFrameRate  = other.videoFrameRate;
    videoBitRate    = other.videoBitRate;
  }
  return *this;
}
//=====================================================================================================================================//
BOOL CCopVideoParams::operator==(const CCopVideoParams &other) const
{
  if ((videoProtocol  == other.videoProtocol) &&
      (videoFormat    == other.videoFormat) &&
      (videoFrameRate == other.videoFrameRate) &&
      (videoBitRate   == other.videoBitRate))
    return TRUE;
  return FALSE;
}
//=====================================================================================================================================//
BOOL CCopVideoParams::operator!=(const CCopVideoParams &other) const
{
  return (*this == other) ? FALSE : TRUE;
}
//=====================================================================================================================================//
CCopVideoParams::~CCopVideoParams()
{
}
//=====================================================================================================================================//
void CCopVideoParams::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
  CXMLDOMElement *pTempNode = pFatherNode->AddChildNode("COP_CONFIGURATION");

  pTempNode->AddChildNode("VIDEO_PROTOCOL", videoProtocol, VIDEO_PROTOCOL_ENUM);
  pTempNode->AddChildNode("COP_VIDEO_FORMAT", videoFormat, COP_VIDEO_FORMAT_ENUM);
  pTempNode->AddChildNode("FRAME_RATE", videoFrameRate, COP_VIDEO_FRAME_RATE_ENUM);
  pTempNode->AddChildNode("TRANSFER_RATE", videoBitRate, TRANSFER_RATE_ENUM);
}
//=====================================================================================================================================//
int CCopVideoParams::DeSerializeXml(CXMLDOMElement *pFatherNode, char *pszError)
{
  int nStatus = STATUS_OK;
  GET_VALIDATE_CHILD(pFatherNode, "VIDEO_PROTOCOL", &videoProtocol, VIDEO_PROTOCOL_ENUM);
  GET_VALIDATE_CHILD(pFatherNode, "COP_VIDEO_FORMAT", &videoFormat, COP_VIDEO_FORMAT_ENUM);
  GET_VALIDATE_CHILD(pFatherNode, "FRAME_RATE", &videoFrameRate, COP_VIDEO_FRAME_RATE_ENUM);
  GET_VALIDATE_CHILD(pFatherNode, "TRANSFER_RATE", &videoBitRate, TRANSFER_RATE_ENUM);
  return nStatus;
}
//=====================================================================================================================================//
void CCopVideoParams::Serialize(WORD format, std::ostream &ostr) const
{
  ostr << (WORD)videoProtocol << "\n";
  ostr << (WORD)videoFormat << "\n";
  ostr << (WORD)videoFrameRate << "\n";
  ostr << (WORD)videoBitRate << "\n";
}
//=====================================================================================================================================//
void CCopVideoParams::DeSerialize(WORD format, std::istream &istr)
{
  WORD tmp;

  istr >> tmp; videoProtocol  = (BYTE)tmp;
  istr >> tmp; videoFormat    = (BYTE)tmp;
  istr >> tmp; videoFrameRate = (BYTE)tmp;
  istr >> tmp; videoBitRate   = (BYTE)tmp;
}
//=====================================================================================================================================//
void CCopVideoParams::Serialize(WORD format, CSegment& seg) const
{
  if (format != NATIVE)
    PASSERT(1);
  COstrStream pOstr;
  Serialize(format, pOstr);
  pOstr.Serialize(seg);
}
//=====================================================================================================================================//
void CCopVideoParams::DeSerialize(WORD format, CSegment &seg)
{
  if (format != NATIVE)
    PASSERT(1);
  CIstrStream istr(seg);
  DeSerialize(format, istr);
}
//=====================================================================================================================================//
STATUS CCopVideoParams::CheckReservRangeValidity(BYTE &errorCode)
{
  errorCode = 0;
  STATUS status = STATUS_OK;
  switch (videoProtocol)
  {
    case VIDEO_PROTOCOL_H261:
    case VIDEO_PROTOCOL_H263:
    case VIDEO_PROTOCOL_H264:
    case VIDEO_PROTOCOL_H264_HIGH_PROFILE:
      break;
    default:
    {
      errorCode = 201;
      return STATUS_OUT_OF_RANGE;
    }
  }
  switch (videoFormat)
  {
    case eCopLevelEncoderVideoFormat_QCIF:
    case eCopLevelEncoderVideoFormat_CIF:
    case eCopLevelEncoderVideoFormat_4CIF:
    case eCopLevelEncoderVideoFormat_4CIF_16_9:
    case eCopLevelEncoderVideoFormat_HD720p:
    case eCopLevelEncoderVideoFormat_HD1080p:
      break;
    default:
    {
      errorCode = 202;
      return STATUS_OUT_OF_RANGE;
    }
  }
  switch (videoFrameRate)
  {
    case eCopVideoFrameRate_12_5:
    case eCopVideoFrameRate_15:
    case eCopVideoFrameRate_25:
    case eCopVideoFrameRate_30:
    case eCopVideoFrameRate_50:
    case eCopVideoFrameRate_60:
      break;
    default:
    {
      errorCode = 203;
      return STATUS_OUT_OF_RANGE;
    }
  }
  switch (videoBitRate)
  {
    case Xfer_64:
    case Xfer_96:
    case Xfer_128:
    case Xfer_192:
    case Xfer_256:
    case Xfer_320:
    case Xfer_384:
    case Xfer_512:
    case Xfer_768:
    case Xfer_832:
    case Xfer_1024:
    case Xfer_1152:
    case Xfer_1280:
    case Xfer_1472:
    case Xfer_1536:
    case Xfer_1728:
    case Xfer_1920:
    case Xfer_2048:
    case Xfer_2560:
    case Xfer_3072:
    case Xfer_3584:
    case Xfer_4096:
    case Xfer_6144:
    case Xfer_8192:
      break;
    default:
    {
      errorCode = 204;
      return STATUS_OUT_OF_RANGE;
    }
  }

  return STATUS_OK;
}
//=====================================================================================================================================//
void CCopVideoParams::Dump(std::ostream & msg) const
{
  msg << "\nVideo Protocol : " << (WORD)videoProtocol;
  msg << "\nVideo Format   : " << (WORD)videoFormat;
  msg << "\nVideo FrameRate: " << (WORD)videoFrameRate;
  msg << "\nVideo BitRate  : " << (WORD)videoBitRate;
}
