//+========================================================================+
//                         CopVideoParams.h                                |
//               Copyright 1995 Pictel Technologies Ltd.                   |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CopVideoParams.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: 		                                                         |
//-------------------------------------------------------------------------|
// Who  | Date    | Description			                                       |
//-------------------------------------------------------------------------|
//                                                                         |
//+========================================================================+
#ifndef COPVIDEOPARAMS_H_
#define COPVIDEOPARAMS_H_

#include "PObject.h"
#include "psosxml.h"

struct CopLevelInfo {
  BYTE videoProtocol;   // VIDEO_PROTOCOL_ENUM
  BYTE videoFormat;     // COP_VIDEO_FORMAT_ENUM
  BYTE videoFrameRate;  // COP_VIDEO_FRAME_RATE_ENUM
  BYTE videoBitRate;    // TRANSFER_RATE_ENUM
};

class CCopVideoParams : public CPObject, private CopLevelInfo
{
  CLASS_TYPE_1(CCopVideoParams, CPObject)

public:
  CCopVideoParams();
  CCopVideoParams(BYTE protocol, BYTE format, BYTE frameRate, BYTE bitRate);
  CCopVideoParams(const CCopVideoParams &other);
  CCopVideoParams& operator=(const CCopVideoParams &other);
  BOOL operator==(const CCopVideoParams &other) const;
  BOOL operator!=(const CCopVideoParams &other) const;
  virtual ~CCopVideoParams();

  // Serialization
  void SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int  DeSerializeXml(CXMLDOMElement *pFatherNode, char *pszError);
  void Serialize(WORD format, std::ostream &ostr) const;
  void DeSerialize(WORD format, std::istream &istr);
  void Serialize(WORD format, CSegment& seg) const;
  void DeSerialize(WORD format, CSegment &seg);

  // Dump
  void  Dump(std::ostream & msg) const;

  // Operations
  virtual const char* NameOf() const  {return "CCopVideoParams";}

  BYTE GetProtocol() const            { return videoProtocol; }
  void SetProtocol(BYTE protocol)     { videoProtocol = protocol; }

  BYTE GetFormat() const              { return videoFormat; }
  void SetFormat(BYTE format)         { videoFormat = format; }

  BYTE GetFrameRate() const           { return videoFrameRate; }
  void SetFrameRate(BYTE frameRate)   { videoFrameRate = frameRate; }

  BYTE GetBitRate() const             { return videoBitRate; }
  void SetBitRate(BYTE bitRate)       { videoBitRate = bitRate; }

  STATUS CheckReservRangeValidity(BYTE &errorCode);
  void FixReservParams();
};

#endif /*COPVIDEOPARAMS_H_*/
