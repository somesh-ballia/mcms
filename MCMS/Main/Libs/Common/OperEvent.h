#ifndef _OPREVENT
  #define _OPREVENT
// +========================================================================+
// OPREVENT.H                                                               |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       OPREVENT.H                                                   |
// SUBSYSTEM:  MCMSOPER                                                     |
// PROGRAMMER: Michel                                                       |
// -------------------------------------------------------------------------|
// Who | Date       | Description                                           |
// -------------------------------------------------------------------------|
//     |            |                                                       |
// +========================================================================+
#include "PObject.h"
#include "CDREvent.h"
#include <iostream>

class CXMLDOMElement;

////////////////////////////////////////////////////////////////////////////
//                        COperSetEndTime
////////////////////////////////////////////////////////////////////////////
class COperSetEndTime : public CPObject, public ACCCDREventOperSetEndTime
{
  CLASS_TYPE_1(COperSetEndTime, CPObject)

public:
                     COperSetEndTime() { }
                    ~COperSetEndTime() { }
  const char*        NameOf() const                                         { return "COperSetEndTime"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr);
  void               Serialize(WORD format, std::ostream& ostr, BYTE fullformat);
  void               DeSerialize(WORD format, std::istream& istr);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        COperDelParty
////////////////////////////////////////////////////////////////////////////
class COperDelParty : public CPObject, public ACCCDREventOperDelParty
{
  CLASS_TYPE_1(COperDelParty, CPObject)

public:
                     COperDelParty() { }
                    ~COperDelParty() { }
  const char*        NameOf() const                                         { return "COperDelParty"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        COperAddParty
////////////////////////////////////////////////////////////////////////////
class COperAddParty : public CPObject, public ACCCDREventOperAddParty
{
  CLASS_TYPE_1(COperAddParty, CPObject)

public:
                     COperAddParty() { }
                    ~COperAddParty() { }
  const char*        NameOf() const                                         { return "COperAddParty"; }

  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

  void               AddPartyPhoneNumber(const char* partyphoneNumber);
  int                CancelPartyPhoneNumber(const char* partyphoneNumber);
  void               AddMcuPhoneNumber(const char* mcuphoneNumber);
  int                CancelMcuPhoneNumber(const char* mcuphoneNumber);
};


////////////////////////////////////////////////////////////////////////////
//                        COperAddPartyCont1
////////////////////////////////////////////////////////////////////////////
class COperAddPartyCont1 : public CPObject, public ACCCDREventOperAddPartyCont1
{
  CLASS_TYPE_1(COperAddPartyCont1, CPObject)

public:
                     COperAddPartyCont1() { }
                    ~COperAddPartyCont1() { }
  const char*        NameOf() const                                         { return "COperAddPartyCont1"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

  BOOL               EnterSemiCollom(char* rIpPartyAlias);
  char*              TranslDwordToString(DWORD addr);
};


////////////////////////////////////////////////////////////////////////////
//                        COperAddPartyCont2
////////////////////////////////////////////////////////////////////////////
class COperAddPartyCont2 : public CPObject, public ACCCDREventOperAddPartyCont2
{
  CLASS_TYPE_1(COperAddPartyCont2, CPObject)

public:
                     COperAddPartyCont2() { }
                    ~COperAddPartyCont2() { }
  const char*        NameOf() const                                         { return "COperAddPartyCont2"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE bilflag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        COperMoveParty
////////////////////////////////////////////////////////////////////////////
class COperMoveParty : public CPObject, public ACCCDREventOperMoveParty
{
  CLASS_TYPE_1(COperMoveParty, CPObject)

public:
                     COperMoveParty() { }
                    ~COperMoveParty() { }
  const char*        NameOf() const                                         { return "COperMoveParty"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum);
  void               SerializeShort(WORD format, std::ostream& ostr, DWORD apiNum);
  void               SerializeShort(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType, BYTE isShortSerial);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               DeSerializeShort(WORD format, std::istream& istr, DWORD apiNum);
};


////////////////////////////////////////////////////////////////////////////
//                        CAddPartyDetailed
////////////////////////////////////////////////////////////////////////////
class CAddPartyDetailed : public CPObject, public ACCCDREventAddPartyDetailed
{
  CLASS_TYPE_1(CAddPartyDetailed, CPObject)

public:
                     CAddPartyDetailed() { }
                     CAddPartyDetailed(const CAddPartyDetailed& other);
                    ~CAddPartyDetailed() { }
  const char*        NameOf() const                                         { return "CAddPartyDetailed"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum);
  void               DeSerialize(WORD format, char* msg_info, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

  void               AddPartyPhoneNumber(const char* partyphoneNumber);
  int                CancelPartyPhoneNumber(const char* partyphoneNumber);
  void               AddMcuPhoneNumber(const char* mcuphoneNumber);
  int                CancelMcuPhoneNumber(const char* mcuphoneNumber);
  char*              TranslDwordToString(DWORD addr);
};


////////////////////////////////////////////////////////////////////////////
//                        COperMoveToConf
////////////////////////////////////////////////////////////////////////////
class COperMoveToConf : public CPObject, public ACCCDREventMoveToConf
{
  CLASS_TYPE_1(COperMoveToConf, CPObject)

public:
                     COperMoveToConf() { }
                    ~COperMoveToConf() { }
  const char*        NameOf() const                                         { return "COperMoveToConf"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE fullformat, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

  CAddPartyDetailed* GetAddPartyDetail()                                    { return (CAddPartyDetailed*)GetpAddPartyDetailed(); }
};


////////////////////////////////////////////////////////////////////////////
//                        CPartySetVisualName
////////////////////////////////////////////////////////////////////////////
class CPartySetVisualName : public CPObject, public ACCCDREventSetVisualName
{
  CLASS_TYPE_1(CPartySetVisualName, CPObject)

public:
                     CPartySetVisualName() { }
                    ~CPartySetVisualName() { }
  const char*        NameOf() const                                         { return "CPartySetVisualName"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyCalling_NumMoveToCont1
////////////////////////////////////////////////////////////////////////////
class CCDRPartyCalling_NumMoveToCont1 : public CPObject, public ACCCDREventMoveToConfCont1
{
  CLASS_TYPE_1(CCDRPartyCalling_NumMoveToCont1, CPObject)

public:
                     CCDRPartyCalling_NumMoveToCont1() { }
                    ~CCDRPartyCalling_NumMoveToCont1() { }
  const char*        NameOf() const                                         { return "CCDRPartyCalling_NumMoveToCont1"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CUpdateUserDefinedInfo
////////////////////////////////////////////////////////////////////////////
class CUpdateUserDefinedInfo : public CPObject, public ACCCDREventSetUserDefinedInfo
{
  CLASS_TYPE_1(CUpdateUserDefinedInfo, CPObject)

public:
                     CUpdateUserDefinedInfo() { }
                    ~CUpdateUserDefinedInfo() { }
  const char*        NameOf() const                                         { return "CUpdateUserDefinedInfo"; }

  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyCalled_NumMoveToCont2
////////////////////////////////////////////////////////////////////////////
class CCDRPartyCalled_NumMoveToCont2 : public CPObject, public ACCCDREventMoveToConfCont2
{
  CLASS_TYPE_1(CCDRPartyCalled_NumMoveToCont2, CPObject)

public:
                     CCDRPartyCalled_NumMoveToCont2() { }
                    ~CCDRPartyCalled_NumMoveToCont2() { }
  const char*        NameOf() const                                         { return "CCDRPartyCalled_NumMoveToCont2"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyDTMFfailureIndication
////////////////////////////////////////////////////////////////////////////
class CCDRPartyDTMFfailureIndication : public CPObject, public ACCCDRPartyDTMFfailureIndication
{
  CLASS_TYPE_1(CCDRPartyDTMFfailureIndication, CPObject)

public:
                     CCDRPartyDTMFfailureIndication() { }
                    ~CCDRPartyDTMFfailureIndication() { }
  const char*        NameOf() const                                         { return "CCDRPartyDTMFfailureIndication"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartyRecording
////////////////////////////////////////////////////////////////////////////
class CCDRPartyRecording : public CPObject, public ACCCDRPartyRecording
{
  CLASS_TYPE_1(CCDRPartyRecording, CPObject)

public:
                     CCDRPartyRecording() { }
                    ~CCDRPartyRecording() { }
  const char*        NameOf() const                                         { return "CCDRPartyRecording"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CCDRPartySystemRecording
////////////////////////////////////////////////////////////////////////////
class CCDRPartySystemRecording : public CPObject, public ACCCDRPartySystemRecording
{
  CLASS_TYPE_1(CCDRPartySystemRecording, CPObject)

public:
                     CCDRPartySystemRecording() { }
                    ~CCDRPartySystemRecording() { }
  const char*        NameOf() const                                         { return "CCDRPartySystemRecording"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        CCDRSipPrivateExtensions
////////////////////////////////////////////////////////////////////////////
class CCDRSipPrivateExtensions : public CPObject, public ACCCDRSipPrivateExtensions
{
  CLASS_TYPE_1(ACCCDRSipPrivateExtensions, CPObject)

public:
                     CCDRSipPrivateExtensions() { }
                    ~CCDRSipPrivateExtensions() { }
  const char*        NameOf() const                                         { return "CCDRSipPrivateExtensions"; }

  char*              Serialize(WORD format);
  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               Serialize(WORD format, std::ostream& ostr, BYTE flag, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};


////////////////////////////////////////////////////////////////////////////
//                        COperIpV6PartyCont1
////////////////////////////////////////////////////////////////////////////
class COperIpV6PartyCont1 : public CPObject, public ACCCDRCOperIpV6PartyCont1
{
  CLASS_TYPE_1(COperIpV6PartyCont1, CPObject)

public:
                     COperIpV6PartyCont1() { }
                    ~COperIpV6PartyCont1() { }
  const char*        NameOf() const                                         { return "COperIpV6PartyCont1"; }

  void               Serialize(WORD format, std::ostream& ostr, DWORD apiNum);
  void               DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void               SerializeXml(CXMLDOMElement* pFatherNode, WORD nEventType);
  int                DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
};

////////////////////////////////////////////////////////////////////////////
//                        CConfCorrelationData
////////////////////////////////////////////////////////////////////////////
class CConfCorrelationData  : public CPObject, public CCDRConfCorrelationDataInfo
{
	CLASS_TYPE_1(CConfCorrelationData, CPObject)
public:
	//Constructors
	CConfCorrelationData();
	~CConfCorrelationData();
	CConfCorrelationData& operator=(const CConfCorrelationData &other);
	bool operator == (const CConfCorrelationData &rHnd);

	// Implementation
	void Serialize(WORD format, std::ostream &m_ostr, DWORD apiNum);
	void DeSerialize(WORD format, std::istream &m_istr, DWORD apiNum);

	void SerializeXml(CXMLDOMElement* pFatherNode);
	int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError);

	const char*  NameOf() const;
	void   SetSigUuid(const std::string  sigUuid);



};


#endif /* _OPREVENT */

