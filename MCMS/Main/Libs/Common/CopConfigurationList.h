//+========================================================================+
//                       CopConfigurationList.h                            |
//               Copyright 1995 Pictel Technologies Ltd.                   |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CopConfigurationList.h                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who  | Date    | Description                                            |
//-------------------------------------------------------------------------|
//                                                                         |
//+========================================================================+
#ifndef COPCONFIGURATIONLIST_H_
#define COPCONFIGURATIONLIST_H_

#include "PObject.h"
#include "ConfPartySharedDefines.h"
#include "CopVideoParams.h"
#include "psosxml.h"

struct CopProfileInfo {
  char*             profileName;
  CopLevelInfo      levelInfo[4];
};

class CCOPConfigurationList : public CPObject
{
  CLASS_TYPE_1(CCOPConfigurationList, CPObject)

public:
  CCOPConfigurationList();
  CCOPConfigurationList(int profileIndex);
  CCOPConfigurationList(const CCOPConfigurationList &other);
  CCOPConfigurationList& operator=(const CCOPConfigurationList &other);
  virtual ~CCOPConfigurationList();

  // Serialization
  void SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int  DeSerializeXml(CXMLDOMElement *pFatherNode, char *pszError);
  void Serialize(WORD format, std::ostream &ostr) const;
  void DeSerialize(WORD format, std::istream &istr);
  void Serialize(WORD format, CSegment& seg) const;
  void DeSerialize(WORD format, CSegment &seg);

  // Dump
  void Dump(const char* title, WORD level) const;
  void Dump(std::ostream & msg) const;

  // Operations
  virtual const char* NameOf() const                  {return "CCOPConfigurationList";}
  CCopVideoParams* GetVideoMode(int index) const      { return (index < NUMBER_OF_COP_LEVELS) ? m_pVideoParams[index] : NULL; }
  BYTE GetAspectRatio() const                         { return m_aspectRatio; }
  STATUS CheckReservRangeValidity(BYTE &errorCode);
  BYTE IsHighProfileFoundInOneLevelAtLeast();
  void SetVideoModeForTest(BYTE videoProtocol, BYTE format, BYTE framerate, BYTE bitRate, BYTE index);

  static int              GetProfilesCount();
  static CopProfileInfo*  GetProfileInfo(int profileIndex);
  static CopProfileInfo*  GetProfileInfo(const char* profileName);

private:
  CCopVideoParams*  m_pVideoParams[NUMBER_OF_COP_LEVELS];
  BYTE              m_aspectRatio;  // ASPECT_RATIO_ENUM
};

#endif /*COPCONFIGURATIONLIST_H_*/
