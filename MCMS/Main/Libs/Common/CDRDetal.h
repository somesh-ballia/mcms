#ifndef _CDRDETAL
  #define _CDRDETAL
// +========================================================================+
// CDRDETAL.H                                                               |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       CDRDETAL.H                                                   |
// SUBSYSTEM:  MCMSOPER                                                     |
// PROGRAMMER: Michel                                                       |
// -------------------------------------------------------------------------|
// Who | Date       | Description                                           |
// -------------------------------------------------------------------------|
//     |            |                                                       |
// +========================================================================+

#include "PObject.h"
#include "StructTm.h"
#include "CDRDefines.h"

#define CDR_BUF_SIZE              8000
#define FORMATED_FILE_SIZE_FACTOR 10

class CStructTm;
class CConfStart;
class CNetChanlCon;
class CMPIChanlCon;
class CIpChanlCon;
class CNetChannelDisco;
class CPartyConnected;
class CSvcSipPartyConnected;
class CPartyDisconnected;
class CRemoteComMode;
class CPartyErrors;
class COperAddParty;
class COperDelParty;
class COperSetEndTime;
class CCdrShort;
class CConfStartCont1;
class COperAddPartyCont1;
class COperAddPartyCont2;
class COperMoveParty;
class COperMoveToConf;
class CPartyDisconnectedCont1;
class CAddPartyDetailed;
class CConfStartCont2;
class CConfStartCont3;
class CPartyAddBillingCode;
class CPartySetVisualName;
class CCDRPartyCalling_NumMoveToCont1;
class CConfStartCont4;
class CConfStartCont5;
class CConfStartCont10;
class CUpdateUserDefinedInfo;
class CCDRPartyDTMFfailureIndication;
class CCDRPartyCalled_NumMoveToCont2;
class CXMLDOMElement;
class CCDRPartyRecording;
class CCDRPartySystemRecording;
class CCDRSipPrivateExtensions;
class CGkInfo;
class CNewRateInfo;
class COperIpV6PartyCont1;
class CCDRPartyChairPerson;
class CCallInfo;
class CPartyCorrelationData;
class CConfCorrelationData;


////////////////////////////////////////////////////////////////////////////
//                        CCdrEvent
////////////////////////////////////////////////////////////////////////////
class CCdrEvent : public CPObject
{
  CLASS_TYPE_1(CCdrEvent, CPObject)

public:
                                   CCdrEvent();
                                   CCdrEvent(const CCdrEvent& other);
                                  ~CCdrEvent();

  void                             Serialize(WORD format, std::ostream& ostr, DWORD apiNum, BYTE fullformat = 0);
  void                             DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void                             SerializeXml(CXMLDOMElement* pFatherNode) const;
  int                              DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);

  void                             BuildConfEndXml(CXMLDOMElement* pFatherNode) const;
  void                             BuildOperTerminateXml(CXMLDOMElement* pFatherNode) const;

  const char*                      NameOf() const                                    { return "CCdrEvent"; }

  void                             SetCdrEventType(const WORD event_type)            { m_cdr_event_type = event_type; }
  WORD                       	   GetCdrEventType() const                           { return m_cdr_event_type; }

  void                             SetEventStructLength(const DWORD event_structlen) { m_event_struct_length = event_structlen; }
  DWORD                      	   GetEventStructLength() const                      { return m_event_struct_length; }

  void                             SetTimeStamp(const CStructTm& other)              { m_time_stamp = other; }
  const CStructTm*                 GetTimeStamp() const                              { return &m_time_stamp; }

  void                             SetConfEndCause(const BYTE conf_end_cause)        { m_cause_conf_end = conf_end_cause; }
  BYTE                       	   GetConfEndCause() const                           { return m_cause_conf_end; }

  void                             SetOperatorName(const char* name)                 { SAFE_COPY(m_operator_name, name); }
  const char*                      GetOperatorName() const                           { return m_operator_name; }

  void                             SetConferenceStart(const CConfStart* otherConfStart);
  CConfStart*                      GetConferenceStart();

  void                             SetNetChanlConnect(const CNetChanlCon* otherNetChanlCon);
  CNetChanlCon*                    GetNetChanlConnect();

  void                             SetIpChanlConnect(const CIpChanlCon* otherIpChanlCon);
  CIpChanlCon*                     GetIpChanlConnect();

  void                             SetNetChanlDisconnect(const CNetChannelDisco* otherNetChanlDisco);
  CNetChannelDisco*                GetNetChanlDisconnect();

  void                             SetPartyConnect(const CPartyConnected* otherPartyConnect);
  CPartyConnected*                 GetPartyConnect();

  void                             SetSvcSipPartyConnect(const CSvcSipPartyConnected* otherSvcSipPartyConnect);
  CSvcSipPartyConnected*           GetSvcSipPartyConnect();

  void                             SetPartyDisconnect(const CPartyDisconnected* otherPartyDisconnect);
  CPartyDisconnected*              GetPartyDisconnect();

  void                             SetRemoteComMode(const CRemoteComMode* otherRemoteComMode);
  CRemoteComMode*                  GetRemoteComMode();

  void                             SetPartyErrors(const CPartyErrors* otherPartyErrors);
  CPartyErrors*                    GetPartyErrors();

  void                             SetAddReservUpdatParty(const COperAddParty* otherAddParty);
  COperAddParty*                   GetAddReservUpdatParty();

  void                             SetAddUnReservUpdatParty(const CAddPartyDetailed* otherAddParty);
  CAddPartyDetailed*               GetAddUnReservUpdatParty();

  void                             SetDelDisconctReconctParty(const COperDelParty* otherDelParty);
  COperDelParty*                   GetDelDisconctReconctParty();

  void                             SetEndTimeEvent(const COperSetEndTime* otherSetEndTime);
  COperSetEndTime*                 GetEndTimeEvent();

  CCdrEvent& operator              =(const CCdrEvent& other);

  void                             SetMPIChanlCon(const CMPIChanlCon* otherSetMPIChanlCon);
  CMPIChanlCon*                    GetMPIChanlCon();

  void                             SetConfStartCont1(const CConfStartCont1* otherSetConfStartCont1);
  CConfStartCont1*                 GetConfStartCont1();

  void                             SetConfStartCont2(const CConfStartCont2* otherSetConfStartCont2);
  CConfStartCont2*                 GetConfStartCont2();

  void                             SetConfStartCont3(const CConfStartCont3* otherSetConfStartCont3);
  CConfStartCont3*                 GetConfStartCont3();

  void                             SetOperAddPartyCont1(const COperAddPartyCont1* otherOperAddPartyCont1);
  COperAddPartyCont1*              GetOperAddPartyCont1();

  void                             SetOperAddPartyCont2(const COperAddPartyCont2* otherOperAddPartyCont2);
  COperAddPartyCont2*              GetOperAddPartyCont2();

  void                             SetPartyDisconectCont1(const CPartyDisconnectedCont1* otherPartyDisconectCont1);
  CPartyDisconnectedCont1*         GetPartyDisconectCont1();

  void                             SetOperMoveParty(const COperMoveParty* otherOperMoveParty);
  COperMoveParty*                  GetOperMoveParty();

  void                             SetOperMoveToConf(const COperMoveToConf* otherOperMoveToConf);
  COperMoveToConf*                 GetOperMoveToConf();

  void                             SetPartyBillingCode(const CPartyAddBillingCode* otherPartyBillingCode);
  CPartyAddBillingCode*            GetPartyBillingCode();

  void                             SetPartyVisualName(const CPartySetVisualName* otherPartyVisualName);
  CPartySetVisualName*             GetPartyVisualName();

  void                             SetConfStartCont4(const CConfStartCont4* pConfStartCont4);
  CConfStartCont4*                 GetConfStartCont4();

  void                             SetConfStartCont5(const CConfStartCont5* otherSetConfStartCont5);
  CConfStartCont5*                 GetConfStartCont5();

  void                             SetConfStartCont10(const CConfStartCont10* pConfStartCont10);
  CConfStartCont10*                GetConfStartCont10();

  void                             SetUserDefinedInfo(const CUpdateUserDefinedInfo* pUpdateUserDefinedInfo);
  CUpdateUserDefinedInfo*          GetUserDefinedInfo();

  void                             SetPartyCalling_Num(const CCDRPartyCalling_NumMoveToCont1* otherPartyCalling_Num);
  CCDRPartyCalling_NumMoveToCont1* GetPartyPartyCalling_Num();

  void                             SetDTMFfailureInd(const CCDRPartyDTMFfailureIndication* pDTMFfailureInd);
  CCDRPartyDTMFfailureIndication*  GetDTMFfailureInd();

  void                             SetPartyCalled_Num(const CCDRPartyCalled_NumMoveToCont2* otherPartyCalled_Num);
  CCDRPartyCalled_NumMoveToCont2*  GetPartyPartyCalled_Num();

  void                             SetRecording(const CCDRPartyRecording* otherRecording);
  CCDRPartyRecording*              GetRecording();

  void                             SetSystemRecording(const CCDRPartySystemRecording* otherRecording);
  CCDRPartySystemRecording*        GetSystemRecording();

  void                             SetSipPrivateExtensions(const CCDRSipPrivateExtensions* otherPrivate);
  CCDRSipPrivateExtensions*        GetSipPrivateExtensions();

  void                             SetGkInfo(const CGkInfo* otherGkInfo);
  CGkInfo*                         GetGkInfo();

  void                             SetNewRateInfo(const CNewRateInfo* otherNewRateInfo);
  CNewRateInfo*                    GetNewRateInfo();

  void 							   SetCallInfo(const CCallInfo* otherCallInfo);
  CCallInfo*                       GetCallInfo();

  static bool                      IsValidEvent(WORD cdrEventType);

  void                             SetOperIpV6PartyCont1(const COperIpV6PartyCont1* otherOperIpV6PartyCont1);
  COperIpV6PartyCont1*             GetOperIpV6PartyCont1();

  void                             SetPartyChairPerson(const CCDRPartyChairPerson* otherPartyChairPerson);
  CCDRPartyChairPerson*            GetPartyChairPerson();

  void 							   SetCPartyCorrelationData(const CPartyCorrelationData* otherCPartyCorrelationData);
  CPartyCorrelationData*           GetCPartyCorrelationData();

  void 							   SetCConfCorrelationData(const CConfCorrelationData* otherCConfCorrelationData);
  CConfCorrelationData*            GetCConfCorrelationData();

protected:
  WORD                             m_cdr_event_type;
  DWORD                            m_event_struct_length;
  CStructTm                        m_time_stamp;
  BYTE                             m_cause_conf_end;
  char                             m_operator_name[OPERATOR_NAME_LEN];
  CConfStart*                      m_pConfStart;
  CNetChanlCon*                    m_pNetChanlCon;
  CNetChannelDisco*                m_pNetChannelDisco;
  CPartyConnected*                 m_pPartyConnected;
  CSvcSipPartyConnected*           m_pSvcSipPartyConnected;
  CPartyDisconnected*              m_pPartyDisconect;
  CRemoteComMode*                  m_pRemoteComMode;
  CPartyErrors*                    m_pPartyErrors;
  COperAddParty*                   m_pOperAddParty;
  COperDelParty*                   m_pOperDelParty;
  COperSetEndTime*                 m_pOperSetEndTime;
  COperMoveParty*                  m_pOperMoveParty;
  COperMoveToConf*                 m_pOperMoveToConf;
  CMPIChanlCon*                    m_pMPIChanlCon;
  CIpChanlCon*                     m_pIpChanlCon;
  CConfStartCont1*                 m_pConfStartCont1;
  COperAddPartyCont1*              m_pOperAddPartyCont1;
  COperAddPartyCont2*              m_pOperAddPartyCont2;
  CPartyDisconnectedCont1*         m_pPartyDisconectCont1;
  CAddPartyDetailed*               m_pAddPartyDetailed;
  CConfStartCont2*                 m_pConfStartCont2;
  CConfStartCont3*                 m_pConfStartCont3;
  CPartyAddBillingCode*            m_pPartyBillingCode;
  CPartySetVisualName*             m_pPartyVisualName;
  CCDRPartyCalling_NumMoveToCont1* m_partyCallingNum;
  CConfStartCont4*                 m_pConfStartCont4;
  CConfStartCont5*                 m_pConfStartCont5;
  CConfStartCont10*                m_pConfStartCont10;
  CUpdateUserDefinedInfo*          m_pUpdateUserDefinedInfo;
  CCDRPartyDTMFfailureIndication*  m_DTMFfailureInd;
  CCDRPartyCalled_NumMoveToCont2*  m_partyCalledNum;
  CCDRPartyRecording*              m_recording;
  CCDRPartySystemRecording*        m_systemRecording;
  CCDRSipPrivateExtensions*        m_sipPrivateExtensions;
  CGkInfo*                         m_gkInfo;
  CNewRateInfo*                    m_NewRateInfo;
  COperIpV6PartyCont1*             m_pOperIpV6PartyCont1;
  CCDRPartyChairPerson*            m_pCDRPartyChairPerson;
  CCallInfo*                       m_CallInfo;
  CPartyCorrelationData*           m_CPartyCorrelationData;
  CConfCorrelationData*            m_CConfCorrelationData;

};


////////////////////////////////////////////////////////////////////////////
//                        CCdrLongStruct
////////////////////////////////////////////////////////////////////////////
class CCdrLongStruct : public CPObject
{
  CLASS_TYPE_1(CCdrLongStruct, CPObject)

public:
                          CCdrLongStruct();
                         ~CCdrLongStruct();
                          CCdrLongStruct(const CCdrLongStruct& other);
  CCdrLongStruct&         operator=(const CCdrLongStruct& other);

  void                    Serialize(WORD format, std::ostream& ostr);
  void                    SerializeOperWS(WORD format, std::ostream& ostr, DWORD apiNum);
  void                    LongSerializeOperWS(WORD format, std::ostream& ostr, DWORD apiNum);
  void                    DeSerialize(WORD format, std::istream& istr, DWORD apiNum);
  void                    DeSerializeToString(WORD format, std::istream& istr);
  void                    SerializeString(WORD format, std::ostream& ostr, DWORD apiNum = 0);

  void                    SerializePtrStringArray(WORD format, std::ostream& ostr);
  void                    DeSerializePtrStringArray(WORD format, std::istream& istr);

  const char*             NameOf() const { return "CCdrLongStruct"; }
  int                     FindEvent(const WORD cdr_event_type);
  int                     FindEvent(const CCdrEvent& other);
  int                     Add(const CCdrEvent& other);
  int                     Update(const CCdrEvent& other);
  int                     Cancel(const WORD cdr_event_type);

  void                    SetShortCdrStruct(const CCdrShort* otherCdrShort);
  CCdrShort*              GetShortCdrStruct();

  void                    SetArrayOfStringPtr(char** ptrStringArr, long numOfStrings);
  char**                  GetArrayOfStringPtr();

  DWORD                   CalcArrOfStringLength();
  BYTE                    CalcLengthOfFileLength();

  WORD                    GetEventNum()                              { return m_numof_cdr_event_struct; }
  CCdrEvent*              GetEvent(int index)                        { return (index < m_numof_cdr_event_struct) ? m_pCdrEvent[index] : NULL; }

  void                    SerializeXml(CXMLDOMElement* pFatherNode, bool no_partID);
  void                    SerializeXmlUnformat(CXMLDOMElement* pFatherNode);
  int                     DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError);
  int                     DeSerializeXmlUnformat(CXMLDOMElement* pActionNode, char* pszError);

  void                    AddSerializedEvent(char* pString, DWORD strLen);

protected:
  CCdrShort*              m_pCdrShort;
  WORD                    m_numof_cdr_event_struct;     // number of short cdr
  CCdrEvent*              m_pCdrEvent[MAX_CDR_EVENT_IN_LIST];

  DWORD                   m_length;                     // Changed from WORD to DWORD from API_NUM_CDR_HUGH
  char*                   m_string;

  char**                  m_pArrayOfString;
  DWORD                   m_array_of_string_size;
};


////////////////////////////////////////////////////////////////////////////
//                        CCdrDetailRequest
////////////////////////////////////////////////////////////////////////////
class CCdrDetailRequest : CPObject
{
public:
                          CCdrDetailRequest();
                         ~CCdrDetailRequest();
                          CCdrDetailRequest(const CCdrDetailRequest& other);

  void                    Serialize(WORD format, std::ostream& ostr);
  void                    DeSerialize(WORD format, std::istream& istr);

  void                    SetConnectionId(const DWORD connection_id) { m_connection_id = connection_id; }
  DWORD		              GetConnectionId() const                    { return m_connection_id; }
  void                    SetConferenceId(const DWORD conference_id) { m_conference_id = conference_id; }
  DWORD             	  GetConferenceId() const                    { return m_conference_id; }

protected:
  DWORD                   m_connection_id;
  DWORD                   m_conference_id;
};

#endif /* _CDRDETAL */


