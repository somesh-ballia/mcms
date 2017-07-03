#include <stdlib.h>
#include "CommResShort.h"
#include "psosxml.h"
#include "ConfPartyDefines.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "H221.h"
#include "TraceStream.h"
#include "ServicePrefixStr.h"
#include "InternalProcessStatuses.h"
#include "CdrApiClasses.h"
#include "ProcessBase.h"

////////////////////////////////////////////////////////////////////////////
//                        CCommResShort
////////////////////////////////////////////////////////////////////////////
CCommResShort::CCommResShort()
              :CPObject()
{
  memset(m_H243confName     , 0, sizeof(m_H243confName));
  memset(m_confDisplayName  , 0, sizeof(m_confDisplayName));
  memset(m_H243_password    , 0, sizeof(m_H243_password));
  memset(m_entry_password   , 0, sizeof(m_entry_password));
  memset(m_NumericConfId    , 0, sizeof(m_NumericConfId));
  memset(m_pServicePhoneStr , 0, sizeof(m_pServicePhoneStr));
  memset(m_pServicePrefixStr, 0, sizeof(m_pServicePrefixStr));

  m_confId                       = 0xFFFFFFFF;
  m_dwAdHocProfileId             = 0xFFFFFFFF;
  m_isPermanent                  = FALSE;
  m_meetingRoomState             = MEETING_ROOM_PASSIVE_STATE;
  m_numServicePhoneStr           = 0;
  m_numServicePrefixStr          = 0;
  m_webReservUId                 = 0;
  m_webOwnerUId                  = 0;
  m_webDBId                      = 0;
  m_webReserved                  = 0;
  m_operatorConf                 = 0;
  m_dwRsrvFlags                  = 0x0;
  m_dwRsrvFlags2                 = 0X0;
  m_eEncryptionType              = eEncryptNone;
  m_pConfContactInfo             = new CConfContactInfo;
  m_meetMePerEntryQ              = NO;
  m_SummeryCreationUpdateCounter = 0;
  m_SummeryUpdateCounter         = 0;
  m_numParties                   = 0;
  m_numUndefParties              = 0;
  m_confTransferRate             = Xfer_64;
  m_network                      = NETWORK_H320_H323;
  m_infoOpcode                   = CONF_COMPLETE_INFO;
  m_contPresScreenNumber         = ONE_ONE;
  m_isAutoLayout                 = NO;
  m_commResFileName              = "";
  m_isTelePresenceMode           = NO;
  m_HD                           = NO;
  m_ResSts                       = 0;                                                          // Status Field
  m_SipRegistrationTotalSts      = eSipRegistrationTotalStatusTypeNotConfigured;               // sipProxySts
  m_TipCompatibility             = eTipCompatibleNone;
  m_natKeepAlivePeriod           = 1;//30;
  m_avMcuCascadeVideoMode        = eMsSvcResourceOptimize;

  for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
  {
    m_ServiceRegistrationContent[i].service_name[0] = '\0';
    m_ServiceRegistrationContent[i].sip_register    = FALSE;
    m_ServiceRegistrationContent[i].accept_call     = TRUE;
    m_ServiceRegistrationContent[i].status          = eSipRegistrationStatusTypeNotConfigured; // sipProxySts1
  }
	eProductType ePT = CProcessBase::GetProcess()->GetProductType();
	//	if (eProductTypeSoftMCU == ePT || eProductTypeGesher == ePT || eProductTypeNinja == ePT)
	//	    m_confMediaType = eAvcOnly;
	//	else
		    m_confMediaType = eAvcOnly;

	m_FocusUriScheduling[0] = '\0';
}

//--------------------------------------------------------------------------
CCommResShort::CCommResShort(const CCommResShort& other)
              :CPObject(other)
{
  strcpy_safe(m_H243confName   , other.m_H243confName);
  strcpy_safe(m_confDisplayName, other.m_confDisplayName);
  strcpy_safe(m_H243_password  , other.m_H243_password);
  strcpy_safe(m_entry_password , other.m_entry_password);
  strcpy_safe(m_NumericConfId  , other.m_NumericConfId);

  memset(m_pServicePhoneStr, 0, sizeof(m_pServicePhoneStr));
  memset(m_pServicePrefixStr, 0, sizeof(m_pServicePrefixStr));

  m_confId                       = other.m_confId;
  m_startTime                    = other.m_startTime;
  m_duration                     = other.m_duration;
  m_isPermanent                  = other.m_isPermanent;
  m_endTime                      = other.m_endTime;
  m_meetingRoomState             = other.m_meetingRoomState;
  m_numServicePhoneStr           = other.m_numServicePhoneStr;
  m_numServicePrefixStr          = other.m_numServicePrefixStr;
  m_webReservUId                 = other.m_webReservUId;
  m_webOwnerUId                  = other.m_webOwnerUId;
  m_webDBId                      = other.m_webDBId;
  m_webReserved                  = other.m_webReserved;
  m_operatorConf                 = other.m_operatorConf;
  m_dwRsrvFlags                  = other.m_dwRsrvFlags;
  m_dwRsrvFlags2                 = other.m_dwRsrvFlags2;
  m_slowInfoMask                 = other.m_slowInfoMask;
  m_pConfContactInfo             = new CConfContactInfo(*other.m_pConfContactInfo);
  m_meetMePerEntryQ              = other.m_meetMePerEntryQ;
  m_SummeryCreationUpdateCounter = other.m_SummeryCreationUpdateCounter;
  m_SummeryUpdateCounter         = other.m_SummeryUpdateCounter;
  m_numParties                   = other.m_numParties;
  m_numUndefParties              = other.m_numUndefParties;
  m_confTransferRate             = other.m_confTransferRate;
  m_contPresScreenNumber         = other.m_contPresScreenNumber;
  m_network                      = other.m_network;
  m_infoOpcode                   = other.m_infoOpcode;
  m_commResFileName              = other.m_commResFileName;
  m_dwAdHocProfileId             = other.m_dwAdHocProfileId;
  m_isAutoLayout                 = other.m_isAutoLayout;
  m_isTelePresenceMode           = other.m_isTelePresenceMode;
  m_HD                           = other.m_HD;
  m_ResSts                       = other.m_ResSts;                                               // Status Field
  m_eEncryptionType              = other.m_eEncryptionType;
  m_SipRegistrationTotalSts      = other.m_SipRegistrationTotalSts;                              // sipProxySts
  m_TipCompatibility             = other.m_TipCompatibility;
  m_avMcuCascadeVideoMode        = other.m_avMcuCascadeVideoMode;
  m_natKeepAlivePeriod           = other.m_natKeepAlivePeriod;

  for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
  {
    if (other.m_pServicePhoneStr[i])
      m_pServicePhoneStr[i] = new CServicePhoneStr(*other.m_pServicePhoneStr[i]);

    if (other.m_pServicePrefixStr[i])
      m_pServicePrefixStr[i] = new CServicePrefixStr(*other.m_pServicePrefixStr[i]);
  }

  memcpy(m_ServiceRegistrationContent, other.m_ServiceRegistrationContent, sizeof(m_ServiceRegistrationContent));
  m_confMediaType = other.m_confMediaType;

  strcpy_safe(m_FocusUriScheduling, other.m_FocusUriScheduling);
}

//--------------------------------------------------------------------------
CCommResShort& CCommResShort::operator=(const CCommResShort& other)
{
  strcpy_safe(m_H243confName, other.m_H243confName);
  strcpy_safe(m_confDisplayName, other.m_confDisplayName);
  strcpy_safe(m_H243_password, other.m_H243_password);
  strcpy_safe(m_entry_password, other.m_entry_password);
  strcpy_safe(m_NumericConfId, other.m_NumericConfId);

  m_confId                       = other.m_confId;
  m_startTime                    = other.m_startTime;
  m_duration                     = other.m_duration;
  m_isPermanent                  = other.m_isPermanent;
  m_endTime                      = other.m_endTime;
  m_meetingRoomState             = other.m_meetingRoomState;
  m_numServicePhoneStr           = other.m_numServicePhoneStr;
  m_numServicePrefixStr          = other.m_numServicePrefixStr;
  m_webReservUId                 = other.m_webReservUId;
  m_webOwnerUId                  = other.m_webOwnerUId;
  m_webDBId                      = other.m_webDBId;
  m_webReserved                  = other.m_webReserved;
  m_operatorConf                 = other.m_operatorConf;
  m_dwRsrvFlags                  = other.m_dwRsrvFlags;
  m_dwRsrvFlags2                 = other.m_dwRsrvFlags2;
  m_slowInfoMask                 = other.m_slowInfoMask;
  m_meetMePerEntryQ              = other.m_meetMePerEntryQ;
  *m_pConfContactInfo            = *other.m_pConfContactInfo;
  m_SummeryCreationUpdateCounter = other.m_SummeryCreationUpdateCounter;
  m_SummeryUpdateCounter         = other.m_SummeryUpdateCounter;
  m_numParties                   = other.m_numParties;
  m_numUndefParties              = other.m_numUndefParties;
  m_confTransferRate             = other.m_confTransferRate;
  m_contPresScreenNumber         = other.m_contPresScreenNumber;
  m_network                      = other.m_network;
  m_infoOpcode                   = other.m_infoOpcode;
  m_commResFileName              = other.m_commResFileName;
  m_dwAdHocProfileId             = other.m_dwAdHocProfileId;
  m_isAutoLayout                 = other.m_isAutoLayout;
  m_isTelePresenceMode           = other.m_isTelePresenceMode;
  m_HD                           = other.m_HD;
  m_ResSts                       = other.m_ResSts;                                               // Status Field
  m_eEncryptionType              = other.m_eEncryptionType;
  m_SipRegistrationTotalSts      = other.m_SipRegistrationTotalSts;                              // sipProxySts
  m_TipCompatibility             = other.m_TipCompatibility;
  m_natKeepAlivePeriod           = other.m_natKeepAlivePeriod;
  m_avMcuCascadeVideoMode        = other.m_avMcuCascadeVideoMode;

  for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
  {
    POBJDELETE(m_pServicePhoneStr[i]);
    if (other.m_pServicePhoneStr[i])
      m_pServicePhoneStr[i] = new CServicePhoneStr(*other.m_pServicePhoneStr[i]);

    POBJDELETE(m_pServicePrefixStr[i]);
    if (other.m_pServicePrefixStr[i])
      m_pServicePrefixStr[i] = new CServicePrefixStr(*other.m_pServicePrefixStr[i]);
  }

  memcpy(m_ServiceRegistrationContent, other.m_ServiceRegistrationContent, sizeof(m_ServiceRegistrationContent));
  m_confMediaType = other.m_confMediaType;

  strcpy_safe(m_FocusUriScheduling, other.m_FocusUriScheduling);

  return *this;
}

//--------------------------------------------------------------------------
CCommResShort::~CCommResShort()
{
  for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
  {
    POBJDELETE(m_pServicePhoneStr[i]);
    POBJDELETE(m_pServicePrefixStr[i]);
  }

  POBJDELETE(m_pConfContactInfo);
}

//--------------------------------------------------------------------------
void CCommResShort::SerializeXml(CXMLDOMElement* pNode, int opcode)
{
  CXMLDOMElement* pResNode      = NULL;
  CXMLDOMElement* pDurationNode = NULL;
  CXMLDOMElement* pTempNode     = NULL;
  CXMLDOMElement* pTempNode2    = NULL;
  char*           nodeName      = NULL;

  pNode->get_nodeName(&nodeName);

  bool isMeetingRoomDB  = (strcmp(nodeName, "MEETING_ROOM_SUMMARY_LS") == 0);        // MeetingRoom DB
  bool isConfTemplateDB = (strcmp(nodeName, "CONFERENCE_TEMPLATE_SUMMARY_LS") == 0); // Conference Template DB"

  if (isMeetingRoomDB)
  {
    pResNode = pNode->AddChildNode("MEETING_ROOM_SUMMARY");
  }
  else if (isConfTemplateDB)
  {
    pResNode = pNode->AddChildNode("CONFERENCE_TEMPLATE_SUMMARY");
    if (opcode != CONF_NOT_CHANGED)
      pResNode->AddChildNode("DISPLAY_NAME", m_confDisplayName);

    pResNode->AddChildNode("ID", m_confId);
    pResNode->AddChildNode("OPERATOR_CONF", m_operatorConf, _BOOL);
    pResNode->AddChildNode("RES_CHANGE", opcode, CHANGE_STATUS_ENUM);
    pResNode->AddChildNode("RES_STATUS", m_ResSts, RESERVATION_STATUS_ENUM); // Status Field
    return;
  }
  else
    pResNode = pNode->AddChildNode("PROFILE_SUMMARY");                       // Profile DB

  if (opcode != CONF_NOT_CHANGED)
    pResNode->AddChildNode("NAME", m_H243confName);

  pResNode->AddChildNode("ID", m_confId);
  pResNode->AddChildNode("RES_CHANGE", opcode, CHANGE_STATUS_ENUM);

  if (opcode == CONF_NOT_CHANGED)
    return;

  pDurationNode = pResNode->AddChildNode("DURATION");
  pDurationNode->AddChildNode("HOUR", m_duration.m_hour);
  pDurationNode->AddChildNode("MINUTE", m_duration.m_min);
  pDurationNode->AddChildNode("SECOND", m_duration.m_sec);
  pResNode->AddChildNode("PERMANENT", IsPermanent() ? "true" : "false");

  if (m_pServicePhoneStr[0])
  {
    Phone* pFirstPhone = m_pServicePhoneStr[0]->GetFirstPhoneNumber();
    if (pFirstPhone)
      pResNode->AddChildNode("MEET_ME_PHONE", pFirstPhone->phone_number);
    else
      pResNode->AddChildNode("MEET_ME_PHONE", "");
  }
  else
    pResNode->AddChildNode("MEET_ME_PHONE", "");

  if (isMeetingRoomDB)
    pResNode->AddChildNode("MR_STATE", m_meetingRoomState, MR_STATE_ENUM);

  pResNode->AddChildNode("OPERATOR_CONF", m_operatorConf, _BOOL);
  pResNode->AddChildNode("ENTRY_QUEUE", m_dwRsrvFlags & ENTRY_QUEUE, _BOOL);
  pResNode->AddChildNode("ENTRY_PASSWORD", m_entry_password);
  pResNode->AddChildNode("PASSWORD", m_H243_password);
  pResNode->AddChildNode("NUMERIC_ID", m_NumericConfId);

  if (m_pConfContactInfo)
    m_pConfContactInfo->SerializeXml(pResNode);

  pResNode->AddChildNode("NUM_PARTIES", m_numParties);
  pResNode->AddChildNode("NUM_UNDEFINED_PARTIES", m_numUndefParties);
  if (m_numServicePrefixStr > 0)
  {
    CXMLDOMElement* pTempNode = pResNode->AddChildNode("DIAL_IN_H323_SRV_PREFIX_LIST");
    for (int i = 0; i < (int)m_numServicePrefixStr; i++)
      m_pServicePrefixStr[i]->SerializeXml(pTempNode);
  }

  pResNode->AddChildNode("ENCRYPTION", m_dwRsrvFlags & ENCRYPTION, _BOOL);
  pResNode->AddChildNode("ENCRYPTION_TYPE", m_eEncryptionType, ENCRYPTION_TYPE_ENUM);
  pResNode->AddChildNode("SIP_FACTORY", m_dwRsrvFlags & SIP_FACTORY, _BOOL);

  if (!isMeetingRoomDB)                                                                                                 // Profile DB
  {
    pResNode->AddChildNode("TRANSFER_RATE", m_confTransferRate, TRANSFER_RATE_ENUM);
    pResNode->AddChildNode("AD_HOC_PROFILE_ID", m_dwAdHocProfileId);
    pResNode->AddChildNode("LAYOUT", m_contPresScreenNumber, LAYOUT_ENUM);
    pResNode->AddChildNode("AUTO_LAYOUT", m_isAutoLayout, _BOOL);
    pResNode->AddChildNode("HD", m_HD, _BOOL);
  }
  else                                                                                                                    // MRDB
  {
    pResNode->AddChildNode("AD_HOC_PROFILE_ID", m_dwAdHocProfileId);
  }

  pResNode->AddChildNode("DISPLAY_NAME", m_confDisplayName);
  pResNode->AddChildNode("IS_TELEPRESENCE_MODE", m_dwRsrvFlags & TELEPRESENCE_MODE, _BOOL);
  pResNode->AddChildNode("GATEWAY", m_dwRsrvFlags & RMX_GATEWAY, _BOOL);
  pResNode->AddChildNode("RES_STATUS", m_ResSts, RESERVATION_STATUS_ENUM);                                                // Status Field

  pTempNode = pResNode->AddChildNode("SERVICE_REGISTRATION_LIST");
  if (pTempNode)                                                    // sipProxySts1
  {
    for (int i = 0; i < NUM_OF_IP_SERVICES; i++)
    {
      if (m_ServiceRegistrationContent[i].service_name[0] != '\0')
      {
        pTempNode2 = pTempNode->AddChildNode("SERVICE_REGISTRATION_CONTENT");
        pTempNode2->AddChildNode("SERVICE_NAME", m_ServiceRegistrationContent[i].service_name, ONE_LINE_BUFFER_LENGTH);
        pTempNode2->AddChildNode("SIP_REGISTRATION", m_ServiceRegistrationContent[i].sip_register, _BOOL);
        pTempNode2->AddChildNode("ACCEPT_CALLS", m_ServiceRegistrationContent[i].accept_call, _BOOL);
        pTempNode2->AddChildNode("SIP_REGISTRATION_STATUS", m_ServiceRegistrationContent[i].status, SIP_REG_STATUS_ENUM); // sipProxySts
      }
    }
  }

  pResNode->AddChildNode("SIP_REGISTRATIONS_STATUS", m_SipRegistrationTotalSts, SIP_REG_TOTAL_STATUS_ENUM);               // sipProxySts
  pResNode->AddChildNode("TIP_COMPATIBILITY", m_TipCompatibility, TIP_COMPATIBILITY_ENUM);
  pResNode->AddChildNode("NAT_KEEP_ALIVE_PERIOD", m_natKeepAlivePeriod, _0_TO_MAXKAVALUE);
  pResNode->AddChildNode("AV_MCU_CASCADE_MODE", m_avMcuCascadeVideoMode,AV_MCU_CASCADE_MODE_ENUM);
  
   pResNode->AddChildNode("CONF_MEDIA_TYPE", m_confMediaType, CONF_MEDIA_TYPE_ENUM);	// AVC/SVC/Mix

   pResNode->AddChildNode("AV_MCU_FOCUS_URI", m_FocusUriScheduling, ONE_LINE_BUFFER_LENGTH);
}

//--------------------------------------------------------------------------
int CCommResShort::DeSerializeXml(CXMLDOMElement* pSummaryNode, int nType, char* pszError)
{
  PASSERTMSG_AND_RETURN_VALUE(1, "CCommResShort::DeSerializeXml - Failed, This code should not be invoked", STATUS_FAIL);
}

//--------------------------------------------------------------------------
void CCommResShort::SetEndTime(const CStructTm& other)
{
  m_endTime = other;
}

//--------------------------------------------------------------------------
const CStructTm* CCommResShort::GetEndTime()
{
  m_endTime = m_startTime + m_duration;
  return &m_endTime;
}

//--------------------------------------------------------------------------
const CStructTm* CCommResShort::GetCalculatedEndTime()
{
  GetEndTime();
  return &m_endTime;
}

//--------------------------------------------------------------------------
int CCommResShort::AddServicePhone(const CServicePhoneStr& other)
{
  if (m_numServicePhoneStr >= MAX_NET_SERV_PROVIDERS_IN_LIST)
    return STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED;

  if (FindServicePhone(other) != NOT_FIND)
    return STATUS_SERVICE_PROVIDER_NAME_EXISTS;

  m_pServicePhoneStr[m_numServicePhoneStr] = new CServicePhoneStr(other);

  m_numServicePhoneStr++;
  return STATUS_OK;
}

//--------------------------------------------------------------------------
int CCommResShort::FindServicePhone(const CServicePhoneStr& other)
{
  for (int i = 0; i < (int)m_numServicePhoneStr; i++)
  {
    // find a service that "larger(have the same and all phones us "other"
    // but can be more phones ) or equal to the given
    if (m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) >= other)
    {
      return i;
    }
  }
  return NOT_FIND;
}

//--------------------------------------------------------------------------
int CCommResShort::AddServicePrefix(const CServicePrefixStr& other)
{
  if (m_numServicePrefixStr >= MAX_NET_SERV_PROVIDERS_IN_LIST)
    return STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED;

  m_pServicePrefixStr[m_numServicePrefixStr] = new CServicePrefixStr(other);

  m_numServicePrefixStr++;
  return STATUS_OK;
}

//--------------------------------------------------------------------------
WORD CCommResShort::IsRoomPhone(char* telNumber)
{
  Phone* phone     = NULL;
  WORD   numPhones = 0;

  for (int i = 0; i < (int) m_numServicePhoneStr; i++)
  {
    BYTE useServicePhonesAsRange = m_pServicePhoneStr[i]->IsUseServicePhonesAsRange();

    if (useServicePhonesAsRange)
    {
      BOOL   foundNumber = FALSE;
      Phone* phone1 = m_pServicePhoneStr[i]->m_pPhoneNumberList[0];
      Phone* phone2 = m_pServicePhoneStr[i]->m_pPhoneNumberList[1];
      if (phone1)
      {
        ULONGLONG phone1Num = atoll(phone1->phone_number);
        if (phone2)
        {
        	ULONGLONG phone2Num = atoll(phone2->phone_number);

        	//VNGFE-5750 || VNGR-25886: phone2Num < phone1Num
        	//if we run to this situation we will use abs value and connect the call and print assert
        	if (phone2Num < phone1Num)
        	{
        		PTRACE(eLevelInfoNormal, "CCommResShort::IsRoomPhone : PROBLEM - phone2Num < phone1Num and we use Service Phones As Range. we calculate numPhones with abs");
        		PASSERT(1);
        	}
        	numPhones = abs((WORD)(phone2Num - phone1Num));
        }
        else
        {
        	numPhones = 0;
        }

        TRACEINTO << "CCommResShort::IsRoomPhone - Using Service Phones as range "
                  << "- phone1:"    << phone1->phone_number
                  << ", phone2:"    << (phone2 ? phone2->phone_number : "-")
                  << ", numPhones:" << numPhones;

        char currPhoneNumber[PHONE_NUMBER_DIGITS_LEN];
        for (WORD j = 0; j < numPhones+1; j++)
        {
          ULONGLONG currPhone = phone1Num + j;
          memset(currPhoneNumber, 0, PHONE_NUMBER_DIGITS_LEN);
          sprintf(currPhoneNumber, "%llu", currPhone);

          char* suffix = telNumber;
          if (strlen(telNumber) > strlen(currPhoneNumber))
            suffix = &telNumber[strlen(telNumber)-strlen(currPhoneNumber)];

          if (strlen(suffix) == 0 || strlen(currPhoneNumber) == 0 || !strcmp(suffix, currPhoneNumber))
          {
            foundNumber = TRUE;
            break;
          }
        }
      }
      return foundNumber;
    }
    else
    {
      numPhones = m_pServicePhoneStr[i]->GetNumPhoneNumbers();
      if (numPhones > MAX_PHONE_NUMBERS_IN_CONFERENCE)		//vNGFE-6610: >= was changed to >
      {
        PASSERTMSG(numPhones, "numPhone exceed MAX_PHONE_NUMBERS_IN_CONFERENCE");
        return  FALSE;
      }

      for (int j = 0; j < numPhones; j++)
      {
        phone = m_pServicePhoneStr[i]->m_pPhoneNumberList[j];
        if (phone != NULL)
        {
          // Compare phones
          char* suffix = telNumber;
          if (strlen(telNumber) > strlen(phone->phone_number))
            suffix = &telNumber[strlen(telNumber)-strlen(phone->phone_number)];

          if (strlen(suffix) == 0 || strlen(phone->phone_number) == 0)
            return 1;

          if (!strcmp(suffix, phone->phone_number))
            return TRUE;
        }
      }
    }
  }
  return FALSE;
}

//--------------------------------------------------------------------------
void CCommResShort::SetSlowInfoMask(WORD onOff)
{
  if (onOff)
    m_slowInfoMask.SetAllBitsOn();
  else
    m_slowInfoMask.SetAllBitsOff();
}

//--------------------------------------------------------------------------
void CCommResShort::FillStructWithPhones(MR_MONITOR_NUMERIC_ID_S& phoneStruct)
{
  if (0 == m_numServicePhoneStr)
    return;

  // Copy the service Name
  const char* serviceName = m_pServicePhoneStr[0]->GetNetServiceName();
  if (!strcmp(serviceName, "[Default Service]"))
    phoneStruct.serviceName[0] = '\0';
  else
    strcpy_safe(phoneStruct.serviceName, serviceName);

  strcpy_safe(phoneStruct.firstPhoneNumber, m_pServicePhoneStr[0]->m_pPhoneNumberList[0]->phone_number);
  strcpy_safe(phoneStruct.secondPhoneNumber, m_pServicePhoneStr[0]->m_pPhoneNumberList[1]->phone_number);
}
