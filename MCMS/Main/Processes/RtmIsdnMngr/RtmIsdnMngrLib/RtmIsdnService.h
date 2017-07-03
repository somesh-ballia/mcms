#ifndef RTMISDNSERVICE_H_
#define RTMISDNSERVICE_H_

#include "SerializeObject.h"
#include "RtmIsdnSpanMap.h"
#include "RtmIsdnPhoneNumberRange.h"
#include "NStream.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "RtmIsdnMngrInternalStructs.h"

class CRtmIsdnSpan;
class CRtmIsdnSpanDefinition;
class CRtmIsdnPhoneNumberRange;

////////////////////////////////////////////////////////////////////////////
//                        CRtmIsdnService
////////////////////////////////////////////////////////////////////////////
class CRtmIsdnService : public CSerializeObject
{
  CLASS_TYPE_1(CRtmIsdnService, CPObject)

public:
                            CRtmIsdnService();
                            CRtmIsdnService(const CRtmIsdnService& other);
  virtual                  ~CRtmIsdnService();

  CRtmIsdnService&          operator=(const CRtmIsdnService& other);

  void                      SerializeXml(CXMLDOMElement*& pFatherNode) const;
  void                      SerializeXml(CXMLDOMElement*& pFatherNode, DWORD ObjToken) const;
  int                       DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);

  CSerializeObject*         Clone()                                                     { return new CRtmIsdnService(); }
  const char*               NameOf() const                                              { return "CRtmIsdnService"; }

  void                      SetIpAddressesList(int i);

  const char*               GetName() const                                             { return m_serviceName; }
  void                      SetName(const char* name)                                   { strcpy_safe(m_serviceName, name); }

  eDfltNumType              GetDefaultNumType() const                                   { return m_dfltNumType; }
  void                      SetDefaultNumType(const eDfltNumType numType)               { m_dfltNumType = numType; }

  eNumPlanType              GetNumPlan() const                                          { return m_numPlan; }
  void                      SetNumPlan(const eNumPlanType numPlan)                      { m_numPlan = numPlan; }

  eVoiceType                GetVoice() const                                            { return m_voice; }
  void                      SetVoice(const eVoiceType voice)                            { m_voice = voice; }

  eNetSpecFacilityType      GetNetSpecFacility() const                                  { return m_netSpecFacility; }
  void                      SetNetSpecFacility(const eNetSpecFacilityType specFacility) { m_netSpecFacility = specFacility; }

  const char*               GetDialOutPrefix() const                                    { return m_dialOutPrefix; }
  void                      SetDialOutPrefix(const char* thePrefix)                     { strcpy_safe(m_dialOutPrefix, thePrefix); }

  const char*               GetMcuCli() const                                           { return m_mcuCli; }
  void                      SetMcuCli(const char* newCli)                               { strcpy_safe(m_mcuCli, newCli); }

  void                      SetSpanDef(const CRtmIsdnSpanDefinition& other);
  CRtmIsdnSpanDefinition*   GetSpanDef()                                                { return m_pSpanDef; }

  WORD                      GetNumbOfPhoneNumRanges() const                             { return m_numb_of_phoneNumRanges; }
  void                      SetNumbOfPhoneNumRanges(const WORD numOfPhoneNum)           { m_numb_of_phoneNumRanges = numOfPhoneNum; }

  int                       AddPhoneNumRange(const CRtmIsdnPhoneNumberRange& other);
  int                       CancelPhoneNumRange(const CRtmIsdnPhoneNumberRange& other);

  int                       FindPhoneRange(const CRtmIsdnPhoneNumberRange& other);
  int                       FindPhoneRange(const char* firstNum, const char* lastNum);
  bool                      IsPhonesAlreadyExistsInServiceRanges(const char* firstNum, const char* lastNum);
  bool                      IsPhonesAlreadyExistsInServiceRanges(const CRtmIsdnPhoneNumberRange& other);

  CRtmIsdnPhoneNumberRange* GetFirstPhoneRange();
  CRtmIsdnPhoneNumberRange* GetNextPhoneRange();
  CRtmIsdnPhoneNumberRange* GetPhoneRangeInIndex(const int index);

  void                      ConvertToRtmIsdnParamsMcmsStruct(RTM_ISDN_PARAMS_MCMS_S& isdnParamsStruct);

  DWORD                     GetIpAddress_Rtm(WORD boardId) const;
  void                      SetIpAddress_Rtm(const WORD boardId, const DWORD ipAddress);

  DWORD                     GetIpAddress_RtmMedia(WORD boardId) const;
  void                      SetIpAddress_RtmMedia(const WORD boardId, const DWORD ipAddress);


  BYTE                      GetChanged() const       { return m_bChanged; }
  DWORD                     GetUpdateCounter() const { return m_updateCounter; }
  void                      IncreaseUpdateCounter();

protected:
  CRtmIsdnSpanDefinition*   m_pSpanDef;

  char                      m_serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
  eDfltNumType              m_dfltNumType;
  eNumPlanType              m_numPlan;
  eVoiceType                m_voice;
  eNetSpecFacilityType      m_netSpecFacility;

  char                      m_dialOutPrefix[ISDN_PHONE_NUMBER_DIGITS_LEN];
  char                      m_mcuCli[ISDN_PHONE_NUMBER_DIGITS_LEN];

  WORD                      m_numb_of_phoneNumRanges;
  CRtmIsdnPhoneNumberRange* m_pPhoneNumRange[MAX_ISDN_PHONE_NUMBER_IN_SERVICE];
  WORD                      m_ind_phoneRange;

  RTM_ISDN_IP_ADDRESSES_S   m_ipAddressesList[MAX_NUM_OF_BOARDS];

private:
  DWORD                     m_updateCounter;
  BYTE                      m_bChanged;
};


////////////////////////////////////////////////////////////////////////////
//                        CRtmIsdnSpanDefinition
////////////////////////////////////////////////////////////////////////////
class CRtmIsdnSpanDefinition : public CSerializeObject
{
  CLASS_TYPE_1(CRtmIsdnSpanDefinition, CPObject)

public:
                          CRtmIsdnSpanDefinition();
                          CRtmIsdnSpanDefinition(const CRtmIsdnSpanDefinition& other);
  virtual                ~CRtmIsdnSpanDefinition();

  CRtmIsdnSpanDefinition& operator =(const CRtmIsdnSpanDefinition& other);

  void                    SerializeXml(CXMLDOMElement*& pFatherNode) const;
  int                     DeSerializeXml(CXMLDOMElement* pActionNode, char* pszError, const char* action);


  CSerializeObject*       Clone()                                         { return new CRtmIsdnSpanDefinition(); }
  const char*             NameOf() const                                  { return "CRtmIsdnSpanParam"; }

  eSpanType               GetSpanType() const                             { return m_spanType; }
  void                    SetSpanType(const eSpanType spanType)           { m_spanType = spanType; }

  eServiceType            GetServiceType() const                          { return m_serviceType; }
  void                    SetServiceType(const eServiceType serviceType)  { m_serviceType = serviceType; }

  eFramingType            GetFraming() const                              { return m_framing; }
  void                    SetFraming(const eFramingType framing)          { m_framing = framing; }

  eSideType               GetSide() const                                 { return m_side; }
  void                    SetSide(const eSideType side)                   { m_side = side; }

  eLineCodingType         GetLineCoding() const                           { return m_lineCoding; }
  void                    SetLineCoding(const eLineCodingType lineCoding) { m_lineCoding = lineCoding; }

  eSwitchType             GetSwitchType() const                           { return m_switchType; }
  void                    SetSwitchType(const eSwitchType switchType)     { m_switchType = switchType; }

protected:
  eSpanType               m_spanType;
  eServiceType            m_serviceType;
  eFramingType            m_framing;
  eSideType               m_side;
  eLineCodingType         m_lineCoding;
  eSwitchType             m_switchType;

  // V4.5 testings - NI-1 does not work at RMX
  // NI-2 working and including NI-1 ==>  change internaly NI-1 to NI-2
  WORD                    m_switch_type_NI1_NI2;
};

#endif /*RTMISDNSERVICE_H_*/

