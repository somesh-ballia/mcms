#ifndef _COMM_RES_SHORT_H_
#define _COMM_RES_SHORT_H_

#include "PObject.h"
#include "OperMask.h"
#include "StructTm.h"
#include "ConfPartySharedDefines.h"
#include "DefinesIpServiceStrings.h"
#include "AllocateStructs.h"
#include "SystemFunctions.h"
#include "ConfContactInfo.h"

class CXMLDOMElement;
class CServicePrefixStr;
class CServicePhoneStr;
class CConfContactInfo;

#define COPERBITOFFINRSRVSHORT(a, bit)  if (a) a->SetSlowInfoBit(bit, FALSE);
#define COPERMASKONINRSRVSHORT(a)       if (a) a->SetSlowInfoMask(TRUE);

////////////////////////////////////////////////////////////////////////////
//                        CCommResShort
////////////////////////////////////////////////////////////////////////////
class CCommResShort : public CPObject
{
  CLASS_TYPE_1(CCommResShort, CPObject)

public:
                             CCommResShort();
                             CCommResShort(const CCommResShort& other);
  virtual                   ~CCommResShort();
  
  CCommResShort&             operator=(const CCommResShort& other);
  void                       SerializeXml(CXMLDOMElement* pActionNode, int opcode);
  int                        DeSerializeXml(CXMLDOMElement* pSummaryNOperMaskode, int nType, char* pszError);

  const char*                NameOf() const                                                      { return "CCommResShort"; }

  void                       SetName(const char* name)                                           { strcpy_safe(m_H243confName, name); }
  const char*                GetName() const                                                     { return m_H243confName; }

  void                       SetDisplayName(const char* name)                                    { strcpy_safe(m_confDisplayName, name); }
  const char*                GetDisplayName() const                                              { return m_confDisplayName; }

  void                       SetConferenceId(const DWORD confId)                                 { m_confId = confId; }
  DWORD                      GetConferenceId() const                                             { return m_confId; }

  void                       SetAdHocProfileId(const DWORD profileId)                            { m_dwAdHocProfileId = profileId; }
  DWORD                      GetAdHocProfileId() const                                           { return m_dwAdHocProfileId; }

  const char*                GetBaseProfileName() const;
  
  void                       SetContinuousPresenceScreenNumber(const BYTE contPresScreenNumber)  { m_contPresScreenNumber = contPresScreenNumber; }
  BYTE                       GetContinuousPresenceScreenNumber() const                           { return m_contPresScreenNumber; }

  void                       SetAutoLayoutFlag(const BYTE isAutoLayout)                          { m_isAutoLayout = isAutoLayout; }
  BYTE                       isAutoLayout() const                                                { return m_isAutoLayout; }

  void                       SetStartTime(const CStructTm& other)                                { m_startTime = other; }
  const CStructTm*           GetStartTime() const                                                { return &m_startTime; }

  void                       SetEndTime(const CStructTm& other);
  const CStructTm*           GetEndTime();

  void                       SetDurationTime(const CStructTm& other)                             { m_duration = other; }
  const CStructTm*           GetDurationTime() const                                             { return &m_duration; }

  const CStructTm*           GetCalculatedEndTime();

  void                       SetPermanent(DWORD prm)                                             { m_isPermanent = prm; }
  DWORD                      IsPermanent() const                                                 { return m_isPermanent; }

  void                       SetMeetingRoomState(const BYTE meetingRoomState)                    { m_meetingRoomState = meetingRoomState; }
  BYTE                       GetMeetingRoomState() const                                         { return m_meetingRoomState; }

  WORD                       GetNumServicePhone() const                                          { return m_numServicePhoneStr; }
  int                        AddServicePhone(const CServicePhoneStr& other);
  int                        FindServicePhone(const CServicePhoneStr& other);
  void                       FillStructWithPhones(MR_MONITOR_NUMERIC_ID_S& phoneStruct);
  int                        AddServicePrefix(const CServicePrefixStr& other);
  WORD                       IsRoomPhone(char* telNumber);

  void                       SetMeetMePerEntryQ(const BYTE meetMePerEntryQ)                      { m_meetMePerEntryQ = meetMePerEntryQ; }
  BYTE                       GetMeetMePerEntryQ() const                                          { return m_meetMePerEntryQ; }

  void                       SetOperatorConf(const BYTE operator_conf)                           { m_operatorConf = operator_conf; }
  BYTE                       GetOperatorConf() const                                             { return m_operatorConf; }

  void                       SetEntryPassword(const char* leader_password)                       { strcpy_safe(m_entry_password, leader_password); }
  const char*                GetEntryPassword() const                                            { return m_entry_password; }

  void                       SetNumericConfId(const char* numericConfId)                         { memset(m_NumericConfId, 0, NUMERIC_CONFERENCE_ID_LEN); strcpy_safe(m_NumericConfId, numericConfId); }
  const char*                GetNumericConfId() const                                            { return m_NumericConfId; }

  void                       SetConfContactInfo(const char* ContactInfo, int ContactNumber)      { m_pConfContactInfo->SetContactInfo(ContactInfo, ContactNumber); }
  const char*                GetConfContactInfo(int ContactNumber) const                         { return m_pConfContactInfo->GetContactInfo(ContactNumber); }

  void                       SetRsrvFlags(const DWORD dwResFlags)                                { m_dwRsrvFlags = dwResFlags; }
  void                       SetRsrvFlags2(const DWORD dwResFlags2)                              { m_dwRsrvFlags2 = dwResFlags2; }
  DWORD                      GetRsrvFlags() const                                                { return m_dwRsrvFlags; }

  void                       SetRsrvEncryptionType(BYTE eEncType)                                { m_eEncryptionType = eEncType; }
  BYTE                       GetRsrvEncryptionType() const                                       { return m_eEncryptionType; }

  void                       SetPassw(const char* szPassw)                                       { strcpy_safe(m_H243_password, szPassw); }
  const char*                GetPassw() const                                                    { return m_H243_password; }

  void                       SetSummeryCreationUpdateCounter(DWORD summaryDBCounter)             { m_SummeryCreationUpdateCounter = summaryDBCounter; }
  DWORD                      GetSummeryCreationUpdateCounter() const                             { return m_SummeryCreationUpdateCounter; }

  void                       SetSummeryUpdateCounter(WORD summaryDBCounter)                      { m_SummeryUpdateCounter = summaryDBCounter; }
  DWORD                      GetSummeryUpdateCounter()                                           { return m_SummeryUpdateCounter; }

  DWORD                      GetNumParties() const                                               { return m_numParties; }
  void                       SetNumParties(const DWORD NumParties)                               { m_numParties = NumParties; }

  DWORD                      GetNumUndefParties() const                                          { return m_numUndefParties; }
  void                       SetNumUndefParties(const DWORD NumUndefParties)                     { m_numUndefParties = NumUndefParties; }

  BYTE                       GetConfTransferRate() const                                         { return m_confTransferRate; }
  void                       SetConfTransferRate(const BYTE confTransferRate)                    { m_confTransferRate = confTransferRate; }

  BYTE                       GetNetwork() const                                                  { return m_network; }
  void                       SetNetwork(const BYTE network)                                      { m_network = network; }

  BYTE                       GetIsTelePresenceMode() const                                       { return m_isTelePresenceMode; }
  void                       SetIsTelePresenceMode(const BYTE newIsTelePresenceMode)             { m_isTelePresenceMode = newIsTelePresenceMode; }

  void                       SetHDVSW(const BYTE isHD)                                           { m_HD = isHD; }
  BYTE                       GetIsHDVSW() const                                                  { return m_HD; }

  void                       SetFileUniqueName(std::string commResFileName)                      { m_commResFileName = commResFileName; }
  std::string                GetFileUniqueName() const                                           { return m_commResFileName; }

  void                       SetResSts(DWORD i_ResSts)                                           { m_ResSts = i_ResSts; }
  DWORD                      GetResSts()                                                         { return m_ResSts; }

  DWORD                      GetSipRegistrationTotalSts()                                        { return m_SipRegistrationTotalSts; }
  void                       SetSipRegistrationTotalSts(DWORD i_SipRegistrationTotalSts)         { m_SipRegistrationTotalSts = i_SipRegistrationTotalSts; }

  DWORD                      GetSipRegistrationSts(int serId)                                    { return m_ServiceRegistrationContent[serId].status; }
  void                       SetSipRegistrationSts(int serId, DWORD status)                      { m_ServiceRegistrationContent[serId].status = status; }
  void                       SetServiceRegistrationContentReg(int serId, BOOL reg)               { m_ServiceRegistrationContent[serId].sip_register = reg; }
  void                       SetServiceRegistrationContentAccept_Call(int serId, BOOL accept)    { m_ServiceRegistrationContent[serId].accept_call = accept; }
  void                       SetServiceRegistrationContentServiceNm(int serId, const char* name) { strcpy_safe(m_ServiceRegistrationContent[serId].service_name, name); }

  void                       SetIsTipCompatible(BYTE tipCompatible)                              { m_TipCompatibility = tipCompatible; }
  void                       SetNatKAPeriod(DWORD period)                                        { m_natKeepAlivePeriod = period; }

  bool                       IsEntryQ() const                                                    { return ((m_dwRsrvFlags&ENTRY_QUEUE) == ENTRY_QUEUE); }
  bool                       IsSIPFactory() const                                                { return (m_dwRsrvFlags& SIP_FACTORY) == SIP_FACTORY; }
  bool                       IsGateWay() const                                                   { return (m_dwRsrvFlags& RMX_GATEWAY) == RMX_GATEWAY; }
  WORD                       GetInfoOpcode() const                                               { return m_infoOpcode; }
  DWORD                      GetNatKAPeriod()                                                    { return m_natKeepAlivePeriod; }
  void                       SetSlowInfoMask(WORD onOff);
  void                            SetconfMediaType( BYTE confMediaType ) {m_confMediaType = confMediaType;}

protected:
  char                       m_H243confName[H243_NAME_LEN];                   // conferences name
  char                       m_confDisplayName[H243_NAME_LEN];                // conferences name
  char                       m_H243_password[H243_NAME_LEN];                  // password
  DWORD                      m_confId;                                        // conferences Id
  CStructTm                  m_startTime;                                     // start conferences time
  CStructTm                  m_duration;                                      // conferences duration time
  CStructTm                  m_endTime;                                       // end   conferences time
  BYTE                       m_meetingRoomState;
  WORD                       m_numServicePhoneStr;
  CServicePhoneStr*          m_pServicePhoneStr[MAX_NET_SERV_PROVIDERS_IN_LIST];
  WORD                       m_numServicePrefixStr;
  CServicePrefixStr*         m_pServicePrefixStr[MAX_NET_SERV_PROVIDERS_IN_LIST];
  DWORD                      m_webReservUId;
  DWORD                      m_webOwnerUId;
  DWORD                      m_webDBId;
  BYTE                       m_webReserved;
  BYTE                       m_operatorConf;
  COperMask                  m_slowInfoMask;
  DWORD                      m_dwRsrvFlags;
  DWORD                      m_dwRsrvFlags2;
  char                       m_entry_password[CONFERENCE_ENTRY_PASSWORD_LEN]; // conference entry password
  BYTE                       m_meetMePerEntryQ;                               // YES, NO
  char                       m_NumericConfId[NUMERIC_CONFERENCE_ID_LEN];
  CConfContactInfo*          m_pConfContactInfo;
  DWORD                      m_SummeryCreationUpdateCounter;
  DWORD                      m_SummeryUpdateCounter;
  DWORD                      m_numParties;
  DWORD                      m_numUndefParties;
  BYTE                       m_confTransferRate;
  BYTE                       m_network;
  DWORD                      m_dwAdHocProfileId;
  char	                     m_baseProfileName[H243_NAME_LEN];  
  BYTE                       m_contPresScreenNumber;
  BYTE                       m_isAutoLayout;
  BYTE                       m_isTelePresenceMode;
  BYTE                       m_HD;
  WORD                       m_infoOpcode;
  std::string                m_commResFileName;
  BYTE                       m_ResSts;
  ServiceRegistrationContent m_ServiceRegistrationContent[NUM_OF_IP_SERVICES];
  BYTE                       m_SipRegistrationTotalSts;
  BYTE                       m_eEncryptionType;
  BYTE                       m_isPermanent;
  BYTE                       m_TipCompatibility;
  BYTE                       m_avMcuCascadeVideoMode;
  DWORD                      m_natKeepAlivePeriod;
  DWORD	                     m_confMediaType;  // SVC-Only, AVC-Only, Mix(AVC or/and SVC)
  char                       m_FocusUriScheduling[ONE_LINE_BUFFER_LEN];   //For AV_MCU conf MS
};

#endif // _COMM_RES_SHORT_H_

