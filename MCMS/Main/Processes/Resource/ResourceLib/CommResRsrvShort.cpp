#include "CommResRsrvShort.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "H221.h"
#include "TraceStream.h"
#include "FaultsDefines.h"
#include "InternalProcessStatuses.h"

CCommResRsrvShort::CCommResRsrvShort() :
	CPObject()
{
	m_H243confName[0]  = '\0';
	m_H243_password[0] = '\0';

	memset(m_H243confName, '\0', H243_NAME_LEN);
	memset(m_confDisplayName, '\0', H243_NAME_LEN);
	memset(m_H243_password, '\0', H243_NAME_LEN);

	m_status                       = eStsOK;
	m_confId                       = 0xFFFFFFFF;
	m_repSchedulingId              = 0;
	m_dwAdHocProfileId             = 0xFFFFFFFF;
	m_meetingRoomState             = MEETING_ROOM_PASSIVE_STATE;
	m_webReservUId                 = 0;
	m_webOwnerUId                  = 0;
	m_webDBId                      = 0;
	m_webReserved                  = 0;
	m_dwRsrvFlags                  = 0x0;
	m_dwRsrvFlags2                 = 0x0;
	m_entry_password[0]            = '\0';
	m_NumericConfId[0]             = '\0';
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
	m_operatorConf                 = 0;
	m_numServicePhoneStr           = 0;
	m_ind_service_phone            = 0;
	m_eEncryptionType              = eEncryptNone;

	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
		m_pServicePhoneStr[i] = NULL;
}

///////////////////////////////////////////////////////////////////////////////
CCommResRsrvShort::CCommResRsrvShort(const CCommResRsrvShort& other)
	: CPObject(other)
{
	strcpy_safe(m_H243confName, other.m_H243confName);
	strcpy_safe(m_confDisplayName, other.m_confDisplayName);
	strcpy_safe(m_H243_password, other.m_H243_password);
	strcpy_safe(m_entry_password, other.m_entry_password);
	strcpy_safe(m_NumericConfId, other.m_NumericConfId);

	m_confId                       = other.m_confId;
	m_repSchedulingId              = other.m_repSchedulingId;
	m_startTime                    = other.m_startTime;
	m_duration                     = other.m_duration;
	m_status                       = other.m_status;
	m_endTime                      = other.m_endTime;
	m_meetingRoomState             = other.m_meetingRoomState;
	m_webReservUId                 = other.m_webReservUId;
	m_webOwnerUId                  = other.m_webOwnerUId;
	m_webDBId                      = other.m_webDBId;
	m_webReserved                  = other.m_webReserved;
	m_dwRsrvFlags                  = other.m_dwRsrvFlags;
	m_dwRsrvFlags2                 = other.m_dwRsrvFlags2;
	m_eEncryptionType              = other.m_eEncryptionType;
	m_slowInfoMask                 = other.m_slowInfoMask;
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
	m_operatorConf                 = other.m_operatorConf;
	m_numServicePhoneStr           = other.m_numServicePhoneStr;
	m_ind_service_phone            = other.m_ind_service_phone;
	m_eEncryptionType              = other.m_eEncryptionType;

	for (WORD i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
	{
		POBJDELETE(m_pServicePhoneStr[i]);
		if (other.m_pServicePhoneStr[i])
			m_pServicePhoneStr[i] = new CServicePhoneStr(*other.m_pServicePhoneStr[i]);
	}
}

/////////////////////////////////////////////////////////////////////////////
CCommResRsrvShort & CCommResRsrvShort::operator = (const CCommResRsrvShort& other)
{
	strcpy_safe(m_H243confName, other.m_H243confName);
	strcpy_safe(m_confDisplayName, other.m_confDisplayName);
	strcpy_safe(m_H243_password, other.m_H243_password);
	strcpy_safe(m_entry_password, other.m_entry_password);
	strcpy_safe(m_NumericConfId, other.m_NumericConfId);

	m_confId                       = other.m_confId;
	m_repSchedulingId              = other.m_repSchedulingId;
	m_startTime                    = other.m_startTime;
	m_duration                     = other.m_duration;
	m_status                       = other.m_status;
	m_endTime                      = other.m_endTime;
	m_meetingRoomState             = other.m_meetingRoomState;
	m_webReservUId                 = other.m_webReservUId;
	m_webOwnerUId                  = other.m_webOwnerUId;
	m_webDBId                      = other.m_webDBId;
	m_webReserved                  = other.m_webReserved;
	m_dwRsrvFlags                  = other.m_dwRsrvFlags;
	m_dwRsrvFlags                  = other.m_dwRsrvFlags2;
	m_eEncryptionType              = other.m_eEncryptionType;
	m_slowInfoMask                 = other.m_slowInfoMask;
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
	m_operatorConf                 = other.m_operatorConf;
	m_numServicePhoneStr           = other.m_numServicePhoneStr;
	m_ind_service_phone            = other.m_ind_service_phone;
	m_eEncryptionType              = other.m_eEncryptionType;

	for (WORD i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
	{
		POBJDELETE(m_pServicePhoneStr[i]);
		if (other.m_pServicePhoneStr[i])
			m_pServicePhoneStr[i] = new CServicePhoneStr(*other.m_pServicePhoneStr[i]);
	}
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
CCommResRsrvShort::~CCommResRsrvShort()
{
	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
		POBJDELETE(m_pServicePhoneStr[i]);
}
//
/////////////////////////////////////////////////////////////////////////////
void CCommResRsrvShort::SerializeXml(CXMLDOMElement *pNode,int opcode,WORD type)
{
	CXMLDOMElement *pResNode,*pDurationNode;

	if(type == MEETING_ROOM_DATABASE)
		pResNode=pNode->AddChildNode("MEETING_ROOM_SUMMARY");
	else if(type == PROFILES_DATABASE)
		pResNode=pNode->AddChildNode("PROFILE_SUMMARY");
	else
		pResNode=pNode->AddChildNode("RES_SUMMARY");

	if (opcode!=CONF_NOT_CHANGED)
		pResNode->AddChildNode("NAME",m_H243confName);

	pResNode->AddChildNode("ID",m_confId);

	pResNode->AddChildNode("RES_CHANGE",opcode,CHANGE_STATUS_ENUM);

	if (opcode==CONF_NOT_CHANGED)
		return;

	//Status Field - also for profiles. if(type != PROFILES_DATABASE)
		pResNode->AddChildNode("RES_STATUS",m_status,RESERVATION_STATUS_ENUM);

    GetEndTime();

	pResNode->AddChildNode("START_TIME",m_startTime);
	pResNode->AddChildNode("END_TIME",m_endTime);

	pDurationNode=pResNode->AddChildNode("DURATION");

	pDurationNode->AddChildNode("HOUR",m_duration.m_hour);
	pDurationNode->AddChildNode("MINUTE",m_duration.m_min);
	pDurationNode->AddChildNode("SECOND",m_duration.m_sec);

	if (m_pServicePhoneStr[0])
	{
	  Phone* pFirstPhone = m_pServicePhoneStr[0]->GetFirstPhoneNumber();
	  if (pFirstPhone)
		pResNode->AddChildNode("MEET_ME_PHONE",pFirstPhone->phone_number);
	  else
		pResNode->AddChildNode("MEET_ME_PHONE","");
	}
	else
		pResNode->AddChildNode("MEET_ME_PHONE","");

	if(type == MEETING_ROOM_DATABASE)
		pResNode->AddChildNode("MR_STATE",m_meetingRoomState,MR_STATE_ENUM);

	pResNode->AddChildNode("REPEATED_ID",m_repSchedulingId);

	pResNode->AddChildNode("ENTRY_QUEUE",m_dwRsrvFlags& ENTRY_QUEUE,_BOOL);
	pResNode->AddChildNode("ENTRY_PASSWORD",m_entry_password);
	pResNode->AddChildNode("PASSWORD",m_H243_password);
	pResNode->AddChildNode("NUMERIC_ID",m_NumericConfId);

	pResNode->AddChildNode("NUM_PARTIES",m_numParties);
	pResNode->AddChildNode("NUM_UNDEFINED_PARTIES",m_numUndefParties);

	if (type == MEETING_ROOM_DATABASE)
	{
		pResNode->AddChildNode("WEB_RESERVED_UID", m_webReservUId);
		pResNode->AddChildNode("WEB_OWNER_UID", m_webOwnerUId);
		pResNode->AddChildNode("WEB_DB_ID", m_webDBId);
		pResNode->AddChildNode("WEB_RESERVED", m_webReserved, _BOOL);
	}
	pResNode->AddChildNode("OPERATOR_CONF",m_operatorConf,_BOOL);
	pResNode->AddChildNode("ENCRYPTION",m_dwRsrvFlags& ENCRYPTION,_BOOL);
	pResNode->AddChildNode("ENCRYPTION_TYPE",m_eEncryptionType,ENCRYPTION_TYPE_ENUM);
  	if (type == PROFILES_DATABASE)
	{
		pResNode->AddChildNode("TRANSFER_RATE",m_confTransferRate,TRANSFER_RATE_ENUM);
		pResNode->AddChildNode("LAYOUT",m_contPresScreenNumber,LAYOUT_ENUM);
		pResNode->AddChildNode("AUTO_LAYOUT",m_isAutoLayout,_BOOL);

	}
	pResNode->AddChildNode("SIP_FACTORY",m_dwRsrvFlags& SIP_FACTORY,_BOOL);

	pResNode->AddChildNode("AD_HOC_PROFILE_ID",m_dwAdHocProfileId);
	pResNode->AddChildNode("DISPLAY_NAME",m_confDisplayName);
}

int CCommResRsrvShort::DeSerializeXml(CXMLDOMElement *pSummaryNode,int nType,char* pszError)
{
  TRACESTR(eLevelError) << "CCommResRsrvShort::DeSerializeXml This Code Should not be invoked !!!!! " ;
  PASSERT(1);
  return STATUS_FAIL;
 }

/////////////////////////////////////////////////////////////////////////////
const char*  CCommResRsrvShort::GetName () const
{
    return m_H243confName;
}

///////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetName(const char* name)
{

  strncpy(m_H243confName, name, sizeof(m_H243confName) - 1);
  m_H243confName[sizeof(m_H243confName) - 1]='\0';

}

/////////////////////////////////////////////////////////////////////////////
const char*  CCommResRsrvShort::GetDisplayName () const
{
    return m_confDisplayName;
}

///////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetDisplayName(const char* name)
{

  strncpy(m_confDisplayName, name, sizeof(m_confDisplayName) -1);
  m_confDisplayName[sizeof(m_confDisplayName) -1]='\0';

}

/////////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetConferenceId(const DWORD confId)
{
  m_confId=confId;
}


///////////////////////////////////////////////////////////////////////////////
DWORD  CCommResRsrvShort::GetConferenceId () const
{
  return m_confId;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetStartTime(const CStructTm &other)
{
  m_startTime=other;
}


///////////////////////////////////////////////////////////////////////////
const CStructTm*  CCommResRsrvShort::GetStartTime()  const
{
     return &m_startTime;
}


/////////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetDurationTime(const CStructTm &other)
{
  m_duration=other;
}


/////////////////////////////////////////////////////////////////////////////
const CStructTm*  CCommResRsrvShort::GetDurationTime()  const

{
    return &m_duration;
}
///////////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetStatus(BYTE status)
{
  m_status=status;
}
///////////////////////////////////////////////////////////////////////////////
BYTE  CCommResRsrvShort::GetStatus()
{
  return m_status;
}

///////////////////////////////////////////////////////////////////////////////
const char*  CCommResRsrvShort::NameOf() const
{
  return "CCommResRsrvShort";
}


///////////////////////////////////////////////////////////////////////////////
const CStructTm*  CCommResRsrvShort::GetEndTime() // const
{
    m_endTime = m_startTime + m_duration;
	return &m_endTime;
}

///////////////////////////////////////////////////////////////////////////////
const CStructTm*  CCommResRsrvShort::GetCalculatedEndTime()
{
    GetEndTime();
    return &m_endTime;
}


////////////////////////////////////////////////////////////////////////////////
BYTE   CCommResRsrvShort::GetMeetingRoomState() const
{
	return m_meetingRoomState;
}
//
////////////////////////////////////////////////////////////////////////////////
void   CCommResRsrvShort::SetMeetingRoomState(const BYTE meetingRoomState)
{
	m_meetingRoomState = meetingRoomState;
}

/////////////////////////////////////////////////////////////////////////////
void CCommResRsrvShort::SetSlowInfoMask(WORD onOff)
{
	if (onOff)
		m_slowInfoMask.SetAllBitsOn();
	else
		m_slowInfoMask.SetAllBitsOff();
}
///////////////////////////////////////////////////////////////////////////////
DWORD  CCommResRsrvShort::GetRsrvFlags () const
{
	return m_dwRsrvFlags;
}

////////////////////////////////////////////////////////////////////////////
void CCommResRsrvShort::SetRsrvFlags (const DWORD flags)
{
	m_dwRsrvFlags=flags;
}
////////////////////////////////////////////////////////////////////////////
void CCommResRsrvShort::SetRsrvFlags2(const DWORD flags2)
{
	m_dwRsrvFlags2=flags2;
}
//----------------------------------------------------------------------------
void CCommResRsrvShort::SetEncryptionType (BYTE confEncryptionType)
{
	m_eEncryptionType=confEncryptionType;
}

BYTE CCommResRsrvShort::GetEncryptionType() const
{
	return m_eEncryptionType;
}
void   CCommResRsrvShort::SetPassw(const char* szPassw)
{
	if(szPassw!=NULL)
	{
		WORD wLen = strlen(szPassw);
		if(wLen > (H243_NAME_LEN-1)){
			PASSERT(1);
			return;
		}
		strncpy(m_H243_password,szPassw,sizeof(m_H243_password) -1);
		m_H243_password[sizeof(m_H243_password) - 1] = '\0';
	}
}

//----------------------------------------------------------------------------

const char*  CCommResRsrvShort::GetPassw () const{
	return m_H243_password;
}
//----------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////
void   CCommResRsrvShort::SetEntryPassword(const char*  entry_password)
{

  strncpy(m_entry_password, entry_password, sizeof(m_entry_password) - 1);
  m_entry_password[sizeof(m_entry_password) - 1]='\0';
}
//////////////////////////////////////////////////////////////////////////////
const char*   CCommResRsrvShort::GetEntryPassword () const
{
	return m_entry_password;
}
////////////////////////////////////////////////////////////////////////////
void   CCommResRsrvShort::SetNumericConfId(const char*  numericConfId)
{
	memset(m_NumericConfId, 0, NUMERIC_CONFERENCE_ID_LEN);
	strncpy(m_NumericConfId, numericConfId, sizeof(m_NumericConfId) - 1);
	m_NumericConfId[sizeof(m_NumericConfId) - 1]='\0';
}
//////////////////////////////////////////////////////////////////////////////
const  char*   CCommResRsrvShort::GetNumericConfId () const
{
	return m_NumericConfId;
}
////////////////////////////////////////////////////////////////////////////
BYTE  CCommResRsrvShort::GetMeetMePerEntryQ () const
{
  return m_meetMePerEntryQ;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetMeetMePerEntryQ(const BYTE meetMePerEntryQ)
{
  m_meetMePerEntryQ = meetMePerEntryQ;
}
////////////////////////////////////////////////////////////////////////////
BYTE  CCommResRsrvShort::GetOperatorConf () const
{
  return m_operatorConf;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetOperatorConf(BYTE operatorConf)
{
  m_operatorConf = operatorConf;
}
/////////////////////////////////////////////////////////////////////////////

DWORD CCommResRsrvShort::GetSummeryCreationUpdateCounter()const
{
	return m_SummeryCreationUpdateCounter;
}

void CCommResRsrvShort::SetSummeryCreationUpdateCounter(DWORD summaryDBCounter)
{
	m_SummeryCreationUpdateCounter = summaryDBCounter;
}

DWORD CCommResRsrvShort::GetSummeryUpdateCounter()
{
	return m_SummeryUpdateCounter;
}

void CCommResRsrvShort::SetSummeryUpdateCounter(WORD summaryDBCounter)
{
	m_SummeryUpdateCounter = summaryDBCounter;
}


DWORD  CCommResRsrvShort::GetNumParties()const
{
	return m_numParties;
}
void   CCommResRsrvShort::SetNumParties(const DWORD NumParties)
{
	m_numParties=NumParties;
}


DWORD  CCommResRsrvShort::GetNumUndefParties()const
{
	return m_numUndefParties;
}
void   CCommResRsrvShort::SetNumUndefParties(const DWORD NumUndefParties)
{
	m_numUndefParties=NumUndefParties;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CCommResRsrvShort::GetConfTransferRate () const
{
	return m_confTransferRate;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetConfTransferRate(const BYTE confTransferRate)
{
	m_confTransferRate = confTransferRate;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CCommResRsrvShort::GetNetwork() const
{
    return m_network;
}

/////////////////////////////////////////////////////////////////////////////
void  CCommResRsrvShort::SetNetwork(const BYTE network)
{
    m_network = network;
}

///////////////////////////////////////////////////////////////////////////
WORD   CCommResRsrvShort::GetInfoOpcode() const
{
    return m_infoOpcode;
}
///////////////////////////////////////////////////////////////////////////
 void  CCommResRsrvShort::SetAdHocProfileId(const DWORD profileId)
{
 	m_dwAdHocProfileId = profileId;
}
 ///////////////////////////////////////////////////////////////////////////
DWORD CCommResRsrvShort::GetAdHocProfileId() const
{
  	return m_dwAdHocProfileId;
}
 /////////////////////////////////////////////////////////////////////////////
void CCommResRsrvShort::SetContinuousPresenceScreenNumber(const BYTE contPresScreenNumber)
{
	m_contPresScreenNumber=contPresScreenNumber;
}
 /////////////////////////////////////////////////////////////////////////////
BYTE  CCommResRsrvShort::GetContinuousPresenceScreenNumber () const
{
	return m_contPresScreenNumber;
}
 /////////////////////////////////////////////////////////////////////////////
void CCommResRsrvShort::SetAutoLayoutFlag(const BYTE isAutoLayout)
{
	m_isAutoLayout=isAutoLayout;
}
 /////////////////////////////////////////////////////////////////////////////
BYTE  CCommResRsrvShort::isAutoLayout () const
{
	return m_isAutoLayout;
}
//////////////////////////////////////////////////////////////////////////////
DWORD  CCommResRsrvShort::GetRepSchedulingId () const
{
    return m_repSchedulingId;
}
//////////////////////////////////////////////////////////////////////////////
void   CCommResRsrvShort::SetRepSchedulingId(const DWORD repSchedulingId)
{
	 m_repSchedulingId = repSchedulingId;
}
//////////////////////////////////////////////////////////////////////////////
WORD CCommResRsrvShort::GetNumServicePhone() const
{
  return m_numServicePhoneStr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////
int CCommResRsrvShort::FindServicePhone(const CServicePhoneStr &other)
{
    for (int i=0; i<(int)m_numServicePhoneStr; i++)
    {
        // find a service that "larger(have the same and all phones us "other"
        // but can be more phones ) or equal to the given
        if(m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) >= other)
		    return i;

		if(m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) < other){
		    *m_pServicePhoneStr[i] = other;
			return i;
		}
    }
	return NOT_FIND;
}
//////////////////////////////////////////////////////////////////////////////

int CCommResRsrvShort::AddServicePhone(const CServicePhoneStr &other)
{
  if (m_numServicePhoneStr>=MAX_NET_SERV_PROVIDERS_IN_LIST)
	 return  STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED;

  if (FindServicePhone(other)!=NOT_FIND)
    return STATUS_SERVICE_PROVIDER_NAME_EXISTS;

  m_pServicePhoneStr[m_numServicePhoneStr] = new CServicePhoneStr(other);

  m_numServicePhoneStr++;
  return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr* CCommResRsrvShort::GetFirstServicePhone()
{
   m_ind_service_phone=1;
   return m_pServicePhoneStr[0];
}

/////////////////////////////////////////////////////////////////////////////
CServicePhoneStr* CCommResRsrvShort::GetNextServicePhone()
{
   if (m_ind_service_phone>=m_numServicePhoneStr) return NULL;
   return m_pServicePhoneStr[m_ind_service_phone++];
}
/////////////////////////////////////////////////////////////////////////////
