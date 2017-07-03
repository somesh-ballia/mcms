#include <string>
#include <algorithm>
#include <iomanip>
#include "TraceStream.h"
#include "SystemResources.h"
#include "Trace.h"
#include "CRsrcDetailGet.h"
#include "HostCommonDefinitions.h"
#include "ApiStatuses.h"
#include "InternalProcessStatuses.h"
#include "RsrcAlloc.h"
#include "psosxml.h"
#include "FaultsDefines.h"
#include "ObjString.h"
#include "SysConfig.h"
#include "WrappersResource.h"
#include "SysConfigKeys.h"
#include "ResourceManager.h"
#include "NetServicesDB.h"
#include "HelperFuncs.h"
#include "AllocationDecider.h"
#include "CardResourceConfig.h"
#include "FixedModeResources.h"
#include "AutoModeResources.h"
#include "Reservator.h"
#include "AllocationModeDetails.h"
#include "ResRsrcCalculator.h"
#include "MsSsrc.h"
#include "Unit.h"
#include "PrettyTable.h"

#undef TRACEINTO
#define TRACEINTO TRACESTRFUNC(eLevelInfoHigh)
eUnitType PhysicalToUnitType(eResourceTypes type)
{
	switch (type)
	{
		case ePhysical_res_none:
			return eUnitType_Generic;

		case ePhysical_rtm:
			return eUnitType_Rtm;

		case ePhysical_art:
		case ePhysical_art_light:
			return eUnitType_Art;

		case ePhysical_audio_controller:
			return eUnitType_Art_Control;

		case ePhysical_video_encoder:
		case ePhysical_video_decoder:
			return eUnitType_Video;

		default:
			FPASSERT(1);
			return eUnitType_Generic;
	}
}

ePortType ResourceToPortType(eResourceTypes type)
{
	switch (type)
	{
		case ePhysical_art:           { return PORT_ART; }
		case ePhysical_art_light:     { return PORT_ART_LIGHT; }
		case ePhysical_video_encoder: { return PORT_VIDEO; }
		//(*) function is used for Resource Report only,
		//in phase1 becouse of video ActivePort	duality (enc, dec)
		//we will count only ENCs and ignore DECs in order to get
		//number of video ports allocated.,
		default:                      { return PORT_GENERIC; }
	}
}


////////////////////////////////////////////////////////////////////////////
//                        CPhone
////////////////////////////////////////////////////////////////////////////
CPhone::CPhone()
{
	m_busy    = FALSE;
	m_pNumber = new char[PHONE_NUMBER_DIGITS_LEN];
	memset(m_pNumber, '\0', PHONE_NUMBER_DIGITS_LEN);
}

////////////////////////////////////////////////////////////////////////////
CPhone::CPhone(const char* num)
{
	if (strlen(num) > PHONE_NUMBER_DIGITS_LEN - 1)
	{
		int i = 0;
		i = i + 1;
		i = strlen(num);
	}
	PASSERT(strlen(num) > PHONE_NUMBER_DIGITS_LEN - 1);
	//m_pNumber = new char[strlen(num)+1];
	m_pNumber = new char[PHONE_NUMBER_DIGITS_LEN];
	memset(m_pNumber, '\0', PHONE_NUMBER_DIGITS_LEN);
	WORD length = strlen(num);

	if (length > PHONE_NUMBER_DIGITS_LEN - 1)
	{
		length = PHONE_NUMBER_DIGITS_LEN - 1;
	}

	strncpy(m_pNumber, num, length);
	m_pNumber[length] = '\0';
	m_busy            = FALSE;
}

////////////////////////////////////////////////////////////////////////////
CPhone::~CPhone()
{
	if (m_pNumber)
		delete [] m_pNumber;
}

////////////////////////////////////////////////////////////////////////////
CPhone::CPhone(const CPhone& other)
	: CPObject(other)
{
	m_pNumber = new char[PHONE_NUMBER_DIGITS_LEN];
	memset(m_pNumber, '\0', PHONE_NUMBER_DIGITS_LEN);
	WORD length = strlen(other.m_pNumber);
	strncpy(m_pNumber, other.m_pNumber, length);
	m_pNumber[length] = '\0';
	m_busy            = other.m_busy;
}

////////////////////////////////////////////////////////////////////////////
void CPhone::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left, std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n\n"
	    << "CPhone::Dump\n"
	    << "-----------------------\n";

	msg << std::setw(20) << "Number: " << m_pNumber << "\n"
	    << std::setw(20) << "Is Busy: " << m_busy << "\n";

	msg << "\n\n";
}

////////////////////////////////////////////////////////////////////////////
BOOL CPhone::IsSimilarTo(const char* pSimilarToThisString) const
{
	//this function is built for checking temporary bonding numbers allocated to participants
	//according to bug vngr-7691 - we should allocate temporary bonding phone numbers with the same prefix and length (in case of long numbers) as the initial number.

	PASSERT_AND_RETURN_VALUE(m_pNumber == NULL, FALSE);

	if (pSimilarToThisString == NULL) //if pSimilarToThisString is NULL, we don't mind if it's similar to anything, so return TRUE
		return TRUE;

	int length          = strlen(m_pNumber);
	int similarToLength = strlen(pSimilarToThisString);

	//GW only case
	if (0 == similarToLength)
		return TRUE;

	if (similarToLength <= TEMPORARY_BONDING_NUMBER_LENGTH) // if it's less than TEMPORARY_BONDING_NUMBER_LENGTH, then it's OK
	{
		if (similarToLength == length)                  // required in VNGR 12354
			return TRUE;
	}

	//compare lengths
	if (similarToLength != length)
		return FALSE;

	//compare prefix
	int prefixLength = similarToLength - TEMPORARY_BONDING_NUMBER_LENGTH;
	if (strncmp(m_pNumber, pSimilarToThisString, prefixLength) != 0)
		return FALSE;

	return TRUE;

}

////////////////////////////////////////////////////////////////////////////
WORD operator==(const CPhone& lhs, const CPhone& rhs)
{
	if (strncmp(lhs.m_pNumber, rhs.m_pNumber, PHONE_NUMBER_DIGITS_LEN))
		return FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
bool operator < (const CPhone& lhs, const CPhone& rhs)
{
	return(strncmp( lhs.m_pNumber,
	                rhs.m_pNumber,
	                PHONE_NUMBER_DIGITS_LEN) < 0);
}

////////////////////////////////////////////////////////////////////////////
//9 is enough for suffixlength, because it's the length of phone numbers in a country
//this was also the way it worked in MGC
//suffix length can't be bigger, because it's the maximum length that fits DWORDS, so if bigger suffix length is needed
//the code should be changed accordingly, to manage bigger numbers
#define SUFFIX_LENGTH                 9
#define GLUE_PHONE_STRING_WITH_PREFIX "%s%09d"
#define GLUE_PHONE_STRING_NO_PREFIX   "%0*d"

void PhoneHelper::CutPhone(char* strPhone, char* strPrefix, DWORD& suffix, int& suffixlength)
{
	memset(strPrefix, '\0', PHONE_NUMBER_DIGITS_LEN - 1);

	int phoneLength = strlen(strPhone);
	if (phoneLength > SUFFIX_LENGTH)
	{
		strncpy(strPrefix, strPhone, phoneLength - SUFFIX_LENGTH);
		strPhone += phoneLength - SUFFIX_LENGTH;
	}
	suffixlength = strlen(strPhone); //for treatment of leading zeros
	suffix       = atol(strPhone);
}

////////////////////////////////////////////////////////////////////////////
void PhoneHelper::GluePhone(char* strPhone, char* strPrefix, DWORD suffix, int suffixlength)
{
	memset(strPhone, '\0', PHONE_NUMBER_DIGITS_LEN - 1);

	if (strlen(strPrefix) != 0)
	{
		sprintf(strPhone, GLUE_PHONE_STRING_WITH_PREFIX, strPrefix, suffix);
	}
	else
	{
		sprintf(strPhone, GLUE_PHONE_STRING_NO_PREFIX, suffixlength, suffix);
	}
}


////////////////////////////////////////////////////////////////////////////
//                        CNetServiceRsrcs
////////////////////////////////////////////////////////////////////////////
CNetServiceRsrcs::CNetServiceRsrcs(const char* num, WORD isNFAS)
{
	WORD len = strlen(num);
	PASSERT(len > RTM_ISDN_SERVICE_PROVIDER_NAME_LEN);
	m_pName = new char[len + 1];
	strncpy(m_pName, num, len);
	m_pName[len] = '\0';

	m_isNFAS = isNFAS;

	m_spanType = SPAN_GENERIC; //none, to be set later

	m_pPhoneslist = new std::set<CPhone>;

	for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
	{
		CIPV4Wrapper RTMv4wrapper(m_ipAddressesList[i].ipAdressRTMV4);
		RTMv4wrapper.NullData();
		CIPV4Wrapper Mediav4wrapper(m_ipAddressesList[i].ipAdressMediaV4);
		Mediav4wrapper.NullData();
		m_ipAddressesList[i].boardId = 0;
	}
	m_pNetPortsPerService = new CNetPortsPerService;
}

////////////////////////////////////////////////////////////////////////////
CNetServiceRsrcs::CNetServiceRsrcs( const CNetServiceRsrcs& other) : CPObject(other),
//m_pPhoneslist(other.m_pPhoneslist)
	m_pPhoneslist(new std::set< CPhone > (*(other.m_pPhoneslist))),
	m_pPhonesMap(other.m_pPhonesMap)
{

	WORD len = strlen(other.m_pName);
	m_pName = new char[len + 1];
	strncpy(m_pName, other.m_pName, len);
	m_pName[len] = '\0';

	m_isNFAS = other.m_isNFAS;

	m_spanType = other.m_spanType;

	for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
		m_ipAddressesList[i] = other.m_ipAddressesList[i];

	m_pNetPortsPerService = new CNetPortsPerService(*(other.m_pNetPortsPerService));
}

////////////////////////////////////////////////////////////////////////////
CNetServiceRsrcs::~CNetServiceRsrcs()
{
	if (m_pName)
		delete [] m_pName;

	if (m_pPhoneslist)
	{
		m_pPhoneslist->clear();
		PDELETE( m_pPhoneslist);
		m_pPhoneslist = 0;
	}
	m_pPhonesMap.clear();

	POBJDELETE(m_pNetPortsPerService);
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServiceRsrcs::EnablePhone(char* num)
{
	if (num == NULL)
		return STATUS_FAIL;

	PASSERT(strlen(num) > PHONE_NUMBER_DIGITS_LEN - 1);

	CPhone* pNewPhone = new CPhone( num );

	if (m_pPhoneslist->find(*pNewPhone) != m_pPhoneslist->end())
	{
		POBJDELETE ( pNewPhone);
		PTRACE2(eLevelInfoNormal, "CNetServiceRsrcs::EnablePhone: phone number already exists ", num);
		return STATUS_ALREADY_EXISTS;
	}

	m_pPhoneslist->insert(*pNewPhone);
	POBJDELETE(pNewPhone);

	ULONGLONG phoneNum = atoll(num);
	m_pPhonesMap[phoneNum] = num;
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServiceRsrcs::DisablePhone(char* num)
{
	//error handle??
	if (num == NULL)
		return STATUS_FAIL;

	PASSERT(strlen(num) > PHONE_NUMBER_DIGITS_LEN - 1);

	CPhone    existPhone = CPhone( num );
	ULONGLONG phoneNum   = atoll(num);

	if (m_pPhoneslist->find(existPhone) != m_pPhoneslist->end())
	{
		m_pPhoneslist->erase(existPhone);
		m_pPhonesMap.erase(phoneNum); //VNGFE-7414
		return STATUS_OK;
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CNetServiceRsrcs::DisablePhone: phone not found!");
		return STATUS_NOT_FOUND;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServiceRsrcs::CapturePhone(ULONGLONG num) //VNGFE-7414
{
	std::map<ULONGLONG, std::string>::iterator itrMap = m_pPhonesMap.find(num);
	if (itrMap != m_pPhonesMap.end())
		return CapturePhone(itrMap->second.c_str());
	TRACESTRFUNC(eLevelError) << "Phone number not exist in service " << num;
	return STATUS_PHONE_NUMBER_NOT_EXISTS;
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServiceRsrcs::CapturePhone(const char* num)
{
	CPhone existPhone = CPhone( num );

	std::set<CPhone>::iterator ii = m_pPhoneslist->find(existPhone);

	if (ii == m_pPhoneslist->end())
	{
		PTRACE2(eLevelInfoNormal, "CNetServiceRsrcs::CapturePhone: failed! phone number not exist in service ", num);
		return STATUS_PHONE_NUMBER_NOT_EXISTS;
	}
	else
	{
		if (ii->IsBusy() == TRUE)
		{
			PTRACE2(eLevelInfoNormal, "CNetServiceRsrcs::CapturePhone: failed! phone already occupied ", num);
			return STATUS_PHONE_NUMBER_OCCUPIED; //STATUS_INSUFFICIENT_RTM_RSRC; //temp, new status needed
		}
		else
		{
			// ((CPhone)(*i)).SetIsBusy(TRUE); //mark phone busy
			((CPhone*)(&(*ii)))->SetIsBusy(TRUE);
			return STATUS_OK;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServiceRsrcs::AllocatePhone(char*& num, const char* pSimilarToThisString)
{
	STATUS status = STATUS_PHONE_NUMBER_NOT_EXISTS;

	std::set<CPhone>::iterator i;
	for (i = m_pPhoneslist->begin(); i != m_pPhoneslist->end(); i++)
	{
		if (i->IsBusy() == TRUE)
			continue;
		else
		{
			//**** fill in phone number for return !!!
			char* phoneNum = (char*)(i->GetNumber());

			if (phoneNum && i->IsSimilarTo(pSimilarToThisString))
			{
				WORD length = strlen(phoneNum);
				num = new char[length + 1];
				strncpy(num, phoneNum, length);
				num[length] = '\0';

				((CPhone*)(&(*i)))->SetIsBusy(TRUE);

				status = STATUS_OK;
				break;
			}
			else    //error with phone number - continue search
				PTRACE(eLevelInfoNormal, "CNetServiceRsrcs::AllocatePhone: phone number empty");
		}
	}
	if (status != STATUS_OK)
		PTRACE(eLevelInfoNormal, "CNetServiceRsrcs::AllocatePhone: phone number not found");

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CNetServiceRsrcs::DeAlocatePhone(char* num)
{
	CPhone existPhone(num);

	std::set<CPhone>::iterator ii = m_pPhoneslist->find(existPhone);
	if (ii == m_pPhoneslist->end()) //VNGFE-7414
	{
		ULONGLONG phoneNum = atoll(num);
		std::map<ULONGLONG, std::string>::iterator itrMap = m_pPhonesMap.find(phoneNum);
		if (itrMap != m_pPhonesMap.end())
		{
			existPhone.SetNumber(itrMap->second.c_str());
			ii = m_pPhoneslist->find(existPhone);
		}
	}

	if (ii == m_pPhoneslist->end())
	{
		TRACEINTOLVLERR << "PhoneNum:" << num << " - Failed, phone number not exist in service";
		return STATUS_NOT_FOUND;
	}
	else
	{
		if (ii->IsBusy() == FALSE)
		{
			TRACEINTOLVLERR << "PhoneNum:" << num << " - Failed, phone already deallocated";
			return STATUS_FAIL;                    //temp, new status needed
		}
		else
		{
			((CPhone*)(&(*ii)))->SetIsBusy(FALSE); //mark phone busy
			return STATUS_OK;
		}
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
WORD operator==(const CNetServiceRsrcs& lhs, const CNetServiceRsrcs& rhs)
{
	if (strncmp(lhs.m_pName, rhs.m_pName, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN))
		return FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
bool operator < (const CNetServiceRsrcs& lhs, const CNetServiceRsrcs& rhs)
{
	return (strncmp(lhs.m_pName, rhs.m_pName, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN) < 0);
}


////////////////////////////////////////////////////////////////////////////
//                        CSleepingConference
////////////////////////////////////////////////////////////////////////////
CSleepingConference::CSleepingConference()
{
	m_monitorConfId = 0;

	m_numConfId = new char[NUMERIC_CONF_ID_MAX_LEN + 1];
	memset(m_numConfId, '\0', NUMERIC_CONF_ID_MAX_LEN + 1);

	m_numServicePhoneStr = 0;
	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
		m_pServicePhoneStr[i] = NULL;

	m_ind_service_phone = 0;

	m_conf_type = eConf_type_none;
}

////////////////////////////////////////////////////////////////////////////
CSleepingConference::CSleepingConference(DWORD monitorConfId)
{
	m_monitorConfId = monitorConfId;

	m_numConfId = new char[NUMERIC_CONF_ID_MAX_LEN + 1];
	memset(m_numConfId, '\0', NUMERIC_CONF_ID_MAX_LEN + 1);

	m_numServicePhoneStr = 0;
	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
		m_pServicePhoneStr[i] = NULL;

	m_ind_service_phone = 0;

	m_conf_type = eConf_type_none;
}

////////////////////////////////////////////////////////////////////////////
CSleepingConference::~CSleepingConference()
{
	if (m_numConfId)
		delete [] m_numConfId;

	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
		if (m_pServicePhoneStr[i])
			POBJDELETE( m_pServicePhoneStr[i]);
}

////////////////////////////////////////////////////////////////////////////
CSleepingConference::CSleepingConference(const CSleepingConference& other)
	: CPObject(other)
{
	m_monitorConfId = other.m_monitorConfId;

	m_numConfId = new char[NUMERIC_CONF_ID_MAX_LEN + 1];
	memset(m_numConfId, '\0', NUMERIC_CONF_ID_MAX_LEN + 1);

	WORD length = strlen(other.m_numConfId);

	if (length > NUMERIC_CONF_ID_MAX_LEN || length < NUMERIC_CONF_ID_MIN_LEN)
		PASSERT(1);
	else
	{
		strncpy(m_numConfId, other.m_numConfId, length);
		m_numConfId[length] = '\0';
	}

	m_numServicePhoneStr = other.m_numServicePhoneStr;

	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
	{
		if (other.m_pServicePhoneStr[i] == NULL)
			m_pServicePhoneStr[i] = NULL;
		else
			m_pServicePhoneStr[i] = new CServicePhoneStr(*other.m_pServicePhoneStr[i]);
	}

	m_ind_service_phone = other.m_ind_service_phone;

	m_conf_type = other.m_conf_type;
}

////////////////////////////////////////////////////////////////////////////
const char* CSleepingConference::NameOf(void) const //Override
{
	return "CSleepingConference";
}

////////////////////////////////////////////////////////////////////////////
WORD operator==(const CSleepingConference& lhs, const CSleepingConference& rhs)
{
	return lhs.m_monitorConfId == rhs.m_monitorConfId;
}

////////////////////////////////////////////////////////////////////////////
const CSleepingConference& CSleepingConference::operator=(const CSleepingConference& rhs)
{
	if (*this == rhs)
		return *this;

	m_monitorConfId = rhs.m_monitorConfId;

	if (m_numConfId)
		delete []m_numConfId;

	if (!rhs.m_numConfId)
		m_numConfId = rhs.m_numConfId;
	else
	{
		WORD length = strlen(rhs.m_numConfId);
		m_numConfId = new char[length + 1];
		strncpy(m_numConfId, rhs.m_numConfId, length);
		m_numConfId[length] = '\0';
	}

	m_numServicePhoneStr = rhs.m_numServicePhoneStr;

	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
	{
		if (m_pServicePhoneStr[i])
			POBJDELETE(m_pServicePhoneStr[i]);

		m_pServicePhoneStr[i] = new CServicePhoneStr(*rhs.m_pServicePhoneStr[i]);
	}

	m_ind_service_phone = rhs.m_ind_service_phone;
	m_conf_type         = rhs.m_conf_type;

	return *this;
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CSleepingConference& lhs, const CSleepingConference& rhs)
{
	return lhs.m_monitorConfId < rhs.m_monitorConfId;
}

////////////////////////////////////////////////////////////////////////////
void CSleepingConference::SetNumConfId(char* numConfId)
{
	if (numConfId == NULL)
	{
		PASSERT(1);
		return;
	}

	memset(m_numConfId, '\0', NUMERIC_CONF_ID_MAX_LEN + 1);
	WORD length = strlen(numConfId);

	if (length > NUMERIC_CONF_ID_MAX_LEN || length < NUMERIC_CONF_ID_MIN_LEN)
		PASSERT(1);

	else
	{
		strncpy(m_numConfId, numConfId, length);
		m_numConfId[length] = '\0';
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSleepingConference::AddServicePhone(const CServicePhoneStr& other)
{
	if (m_numServicePhoneStr >= MAX_NET_SERV_PROVIDERS_IN_LIST)
		return STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED;

	if (FindServicePhone(other) != NOT_FIND)
		return STATUS_SERVICE_PROVIDER_NAME_EXISTS;

	m_pServicePhoneStr[m_numServicePhoneStr] = new CServicePhoneStr(other);

	m_numServicePhoneStr++;
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSleepingConference::DeleteServicePhone(const CServicePhoneStr& other)
{
	int i = 0;
	STATUS status = STATUS_OK;

	if ((i = FindServicePhone(other)) == NOT_FIND)
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;

	PASSERTSTREAM_AND_RETURN_VALUE(i >= MAX_NET_SERV_PROVIDERS_IN_LIST, "InvalidIndex:" << (DWORD)i, STATUS_FAIL);

	if (m_pServicePhoneStr[i])
		POBJDELETE(m_pServicePhoneStr[i]);

	m_numServicePhoneStr--;

	if (m_numServicePhoneStr > MAX_NET_SERV_PROVIDERS_IN_LIST)
		status = STATUS_FAIL;

	// Need to close the gap that might have been created
	for (int j = i; j < MAX_NET_SERV_PROVIDERS_IN_LIST - 1; j++)
		m_pServicePhoneStr[j] = m_pServicePhoneStr[j + 1];

	return status;
}

////////////////////////////////////////////////////////////////////////////
int CSleepingConference::FindServicePhone(const CServicePhoneStr& other)
{
	for (int i = 0; i < (int)m_numServicePhoneStr; i++)
	{
		// find a service that "larger(have the same and all phones us "other"
		// but can be more phones ) or equal to the given
		if (m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) >= other)
		{
			return i;
		}
		if (m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) < other)
		{
			*m_pServicePhoneStr[i] = other;
			return i;
		}
	}
	return NOT_FIND;
}

////////////////////////////////////////////////////////////////////////////
CServicePhoneStr* CSleepingConference::GetFirstServicePhone()
{
	m_ind_service_phone = 0;
	return m_pServicePhoneStr[0];
}

////////////////////////////////////////////////////////////////////////////
CServicePhoneStr* CSleepingConference::GetNextServicePhone()
{
	m_ind_service_phone++;
	if (m_ind_service_phone >= m_numServicePhoneStr)
		return NULL;
	return m_pServicePhoneStr[m_ind_service_phone];
}

////////////////////////////////////////////////////////////////////////////
void CSleepingConference::GetConferenceNeededAmount(CIntervalRsrvAmount& confNeededAmount) const
{
	confNeededAmount.m_bIsRealConference = FALSE; //all sleeping meeting rooms are not real conferences
	confNeededAmount.SetNID(m_numConfId);
}


////////////////////////////////////////////////////////////////////////////
//                        CAudioVideoConfig
////////////////////////////////////////////////////////////////////////////
CAudioVideoConfig::CAudioVideoConfig(const CAudioVideoConfig& other)
	: CSerializeObject(other)
{
	m_audio = other.m_audio;
	m_video = other.m_video;
}

////////////////////////////////////////////////////////////////////////////
WORD operator ==(const CAudioVideoConfig& lhs, const CAudioVideoConfig& rhs)
{
	return ((lhs.m_audio == rhs.m_audio) && (lhs.m_video == rhs.m_video));
}

////////////////////////////////////////////////////////////////////////////
void CAudioVideoConfig::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left, std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n\n"
	    << "CAudioVideoConfig::Dump\n"
	    << "-----------------------\n";

	msg << std::setw(20) << ": " << m_audio << "\n"
	    << std::setw(20) << ": " << m_video << "\n";

	msg << "\n\n";
}

////////////////////////////////////////////////////////////////////////////
CAudioVideoConfig& CAudioVideoConfig::operator=(const CAudioVideoConfig& audVidConfig)
{
	m_audio = audVidConfig.m_audio;
	m_video = audVidConfig.m_video;

	return *this;
}

////////////////////////////////////////////////////////////////////////////
void CAudioVideoConfig::SerializeXml(CXMLDOMElement*& thisNode, WORD Id) const
{
	thisNode->AddChildNode("ID", Id);
	thisNode->AddChildNode("NUM_AUDIO_PORTS", m_audio);
	thisNode->AddChildNode("NUM_VIDEO_PORTS", m_video);
}

////////////////////////////////////////////////////////////////////////////
int CAudioVideoConfig::DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;
	return nStatus;
}


////////////////////////////////////////////////////////////////////////////
//                        CPortsConfig
////////////////////////////////////////////////////////////////////////////
CPortsConfig::CPortsConfig()
{
	m_selectedIndex    = 0;
	m_pPortsConfigList = new PORTS_CONFIG_LIST;

	m_DongleRestriction                           = 0;
	m_LastHD720PortsAccordingToCardsRestriction   = 0;
	m_LastGeneratedConfigurationListMaxHD720Ports = 0;
}

////////////////////////////////////////////////////////////////////////////
CPortsConfig::CPortsConfig(const CPortsConfig& other )
	: CSerializeObject(other)
{
	m_selectedIndex = other.m_selectedIndex;

	m_pPortsConfigList = new PORTS_CONFIG_LIST ( *other.m_pPortsConfigList );

	m_DongleRestriction                           = other.m_DongleRestriction;
	m_LastHD720PortsAccordingToCardsRestriction   = other.m_LastHD720PortsAccordingToCardsRestriction;
	m_LastGeneratedConfigurationListMaxHD720Ports = other.m_LastGeneratedConfigurationListMaxHD720Ports;
}

////////////////////////////////////////////////////////////////////////////
CPortsConfig::~CPortsConfig()
{
	m_pPortsConfigList->clear();
	PDELETE( m_pPortsConfigList);
	m_pPortsConfigList = 0;
}

////////////////////////////////////////////////////////////////////////////
void CPortsConfig::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement* pAudVidCfgList = thisNode->AddChildNode("AUDIO_VIDEO_CONFIG_LIST");

	pAudVidCfgList->AddChildNode("SELECTED_ID", m_selectedIndex);

	size_t listSize = m_pPortsConfigList->size();
	for (size_t i = 0; i < listSize; i++)
	{
		CXMLDOMElement* pAudVidCfgNode = pAudVidCfgList->AddChildNode( "AUDIO_VIDEO_CONFIG" );
		(*m_pPortsConfigList)[i].SerializeXml( pAudVidCfgNode, (WORD)i );
	}
}

////////////////////////////////////////////////////////////////////////////
int CPortsConfig::DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////
STATUS CPortsConfig::ResetConfigurationListAccordingToLicenseAndCards()
{
	DWORD max_availble_hd720_ports = min(m_LastHD720PortsAccordingToCardsRestriction, m_DongleRestriction);

	GenerateConfigurationList(max_availble_hd720_ports);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CPortsConfig::GenerateConfigurationList(DWORD max_hd720_ports)
{
	TRACEINTO << "MaxHd720Ports:" << max_hd720_ports;

	CSystemResources* pResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pResources, STATUS_FAIL);

	if (!CResRsrcCalculator::IsRMX1500Q() && (m_LastGeneratedConfigurationListMaxHD720Ports == max_hd720_ports))
		return STATUS_OK;

	// RMX1500Q configuration steps when using license of 25cif/7hd720 ports that uses special ratios.
	WORD  arrRMX1500QConfigList[RMX1500Q_RESOURCE_SLIDER_CONFIG_STEPS][2] =
	{ { 7, 0 }, { 6, 10 }, { 5, 18 }, { 4, 32 }, { 3, 46 }, { 2, 61 }, { 1, 75 }, { 0, 90 } };

	STATUS status = STATUS_OK;

	eProductType productType = pResources->GetProductType();
	// SoftMCU doesn't allow switch between audio and video
	if (CHelperFuncs::IsSoftMCU(productType))
	{
		m_LastGeneratedConfigurationListMaxHD720Ports = max_hd720_ports;
		m_pPortsConfigList->clear();
		m_pPortsConfigList->reserve(1);
		m_pPortsConfigList->push_back(CAudioVideoConfig(0, max_hd720_ports));

		return STATUS_OK;
	}

	int   ports_configuration_step = pResources->GetPortsConfigurationStep();
	float audio_factor = pResources->GetAudioFactor();

	if (max_hd720_ports % ports_configuration_step != 0)
	{
		PASSERT(max_hd720_ports);
		status = STATUS_FAIL;
	}

	if (STATUS_OK == status)
	{
		m_LastGeneratedConfigurationListMaxHD720Ports = max_hd720_ports;
		m_pPortsConfigList->clear();

		if (CResRsrcCalculator::IsRMX1500QRatios())
		{
			int num_of_possible_configuration = RMX1500Q_RESOURCE_SLIDER_CONFIG_STEPS;

			// License should be 25cif/7hd720, so this is only to adjust config steps if using lower license, for example 20cif. (FFU)
			while (max_hd720_ports < arrRMX1500QConfigList[num_of_possible_configuration - 1][0])
			{
				num_of_possible_configuration--;
			}

			TRACEINTO << "IsRMX1500QRatios=1, ConfigSteps:" << num_of_possible_configuration;

			m_pPortsConfigList->reserve(num_of_possible_configuration);     // Sets the vector to fixed size
			for (int i = RMX1500Q_RESOURCE_SLIDER_CONFIG_STEPS - num_of_possible_configuration; i < num_of_possible_configuration; i++)
			{
				m_pPortsConfigList->push_back(CAudioVideoConfig(arrRMX1500QConfigList[i][1], arrRMX1500QConfigList[i][0]));
			}
		}
		else
		{
			int curr_vid = max_hd720_ports, num_of_possible_configuration = (max_hd720_ports / ports_configuration_step) + 1;
			m_pPortsConfigList->reserve(num_of_possible_configuration);     // Sets the vector to fixed size
			for (int i = 0; i < num_of_possible_configuration; i++)
			{
				float num_of_voice = floor(audio_factor * (max_hd720_ports - curr_vid));

				m_pPortsConfigList->push_back(CAudioVideoConfig((WORD)(num_of_voice), curr_vid));
				curr_vid -= ports_configuration_step;
			}
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CPortsConfig::SetDongleRestriction(DWORD dongleRestriction)
{
	m_DongleRestriction = dongleRestriction;
	return GenerateConfigurationList(m_DongleRestriction);
}

////////////////////////////////////////////////////////////////////////////
void CPortsConfig::SetMaxHD720PortsAccordingToCards(DWORD hd720_ports)
{
	if (m_LastHD720PortsAccordingToCardsRestriction == hd720_ports)
		return;  //nothing to be done

	TRACEINTO << "HD720_ports:" << hd720_ports << ", DongleRestriction: " << m_DongleRestriction;
	m_LastHD720PortsAccordingToCardsRestriction = hd720_ports;
}

////////////////////////////////////////////////////////////////////////////
STATUS CPortsConfig::GetStatus()
{
	DWORD max_availble_hd720_ports = min(m_LastHD720PortsAccordingToCardsRestriction, m_DongleRestriction);
	if (max_availble_hd720_ports < m_LastGeneratedConfigurationListMaxHD720Ports)
		return STATUS_CURRENT_SLIDER_SETTINGS_REQUIRES_MORE_RESOURCES_THAN_AVAILBLE | WARNING_MASK;
	else if (max_availble_hd720_ports > m_LastGeneratedConfigurationListMaxHD720Ports)
		return STATUS_PORTS_CONFIGURATION_DOES_NOT_USE_FULL_SYSTEM_CAPACITY | WARNING_MASK;
	return STATUS_OK; //if equal
}

////////////////////////////////////////////////////////////////////////////
// Return Value : FALSE if not found, TRUE if found.
bool CPortsConfig::FindPortsConfigurationIndexByConfig(const CAudioVideoConfig& aud_vid, size_t& index) const
{
	bool ret = FALSE;
	size_t listSize = m_pPortsConfigList->size();
	for (size_t i = 0; i < listSize; i++)
	{
		if (aud_vid == (*m_pPortsConfigList)[i])
		{
			index = i;
			ret = TRUE;
			break;
		}
	}
	return ret;
}

////////////////////////////////////////////////////////////////////////////
CAudioVideoConfig* CPortsConfig::FindPortsConfigurationConfigByIndex(size_t index) const
{
	CAudioVideoConfig* tmp = NULL;
	size_t listSize = m_pPortsConfigList->size();

	if (index >= listSize)
		PTRACE2INT(eLevelInfoHigh, "CPortsConfig::FindPortsConfigurationConfigByIndex", index);
	else
		tmp = &((*m_pPortsConfigList)[index]);

	return tmp;
}


////////////////////////////////////////////////////////////////////////////
//                        CRtmChannelIds
////////////////////////////////////////////////////////////////////////////
CRtmChannelIds::CRtmChannelIds()
{
	m_boxId         = 0;
	m_boardId       = 0;
	m_subBoardId    = 0;
	m_pChannelIds   = new std::bitset<MAX_CHANNEL_IDS>;
	m_nextChannelId = 1;
}

////////////////////////////////////////////////////////////////////////////
CRtmChannelIds::CRtmChannelIds(WORD boxId, WORD bId, WORD subBoardId)
{
	m_boxId         = boxId;
	m_boardId       = bId;
	m_subBoardId    = subBoardId;
	m_pChannelIds   = new std::bitset<MAX_CHANNEL_IDS>;
	m_nextChannelId = 1;
}

////////////////////////////////////////////////////////////////////////////
CRtmChannelIds::~CRtmChannelIds()
{
	if (m_pChannelIds)      //bitVec
		POBJDELETE( m_pChannelIds);
}

////////////////////////////////////////////////////////////////////////////
WORD CRtmChannelIds::Allocate() //circular , -1 - failure return value
{
	if (m_nextChannelId > MAX_CHANNEL_IDS)
	{
		PASSERT(m_nextChannelId);
		m_nextChannelId = 1;
	}

	STATUS status = STATUS_INSUFFICIENT_CHANNEL_ID;

	WORD i;
	WORD next = m_nextChannelId - 1;               //index start

	for (i = next; i < MAX_CHANNEL_IDS; i++)
	{
		if ((*m_pChannelIds)[i])
			continue;                        //already allocated
		else
		{
			(*m_pChannelIds)[i] = 1;         //allocate
			status = STATUS_OK;
			break;
		}
	}

	if (i == MAX_CHANNEL_IDS)                        //reach end, go circular
		for (i = 0; i < next; i++)
		{
			if ((*m_pChannelIds)[i])
				continue;                //already allocated
			else
			{
				(*m_pChannelIds)[i] = 1; //allocate
				status = STATUS_OK;
				break;
			}
		}

	if (status == STATUS_OK)
		m_nextChannelId = (i == MAX_CHANNEL_IDS - 1) ? 1 : i + 2;
	else
		PTRACE(eLevelInfoNormal, "CRtmChannelIds::Allocate : STATUS_INSUFFICIENT_CHANNEL_ID");

	return (status == STATUS_OK) ? i + 1 : 0;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRtmChannelIds::DeAllocate(WORD channelId)
{
	if (channelId > MAX_CHANNEL_IDS || channelId == 0)
	{
		PASSERT(channelId);   //illegal channelId - trace? status?
		return STATUS_FAIL;
	}
	int i = channelId - 1;

	if ((*m_pChannelIds)[i] == 0) //some internal error, already free
		PASSERT(channelId);   // trace? status?

	(*m_pChannelIds)[i] = 0;      //deallocate

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CSystemResources
////////////////////////////////////////////////////////////////////////////
CSystemResources::CSystemResources() :
	m_RamSize(eSystemRamSize_illegal)
{
	//RTM, UDP to be added

	m_pNetServicesDB     = new CNetServicesDB;
	m_pAllocationDecider = new CAllocationDecider(m_pNetServicesDB);

	//udp
	m_pIPServices                  = new std::set<CIPServiceResources>;
	m_IpServicesResourcesInterface = new std::set<CIpServiceResourcesInterfaceArray>;

	m_nextSSRCId = 1;

	m_connectionIds.set(LOBBY_CONNECTION_ID - 1);
	m_connectionIds.set(MCCF_CONNECTION_ID - 1);
	m_connectionIds.set(CONF_PARTY_CONNECTION_ID - 1);

	m_AudioCntrlMasterBid = 0xFFFF;
	m_AudioCntrlSlaveBid  = 0xFFFF;
	m_IVRCntrlBid         = 0xFFFF;

	m_Federal = NO;
	m_eventMode = FALSE;
	m_TIP       = NO;

	m_bIsRsrcEnough = FALSE;

	//**simulate udp resources

	memset(&UdpAdressesArray[0], 0, sizeof(UdpAddresses));
	memset(&UdpAdressesArray[1], 0, sizeof(UdpAddresses));
	memset(&UdpAdressesArray[2], 0, sizeof(UdpAddresses));

	//***set 3 ports simulation values
	WORD i;
	for (i = 0; i < 3; i++)
	{
		//  Ln:0xc9C016Ac  Be:0xac16c0c9
		UdpAdressesArray[i].IpV4Addr.ip = 0xac16b84d; //using the same ip?!
		//TBD ipv6, which address should be put here
		//UdpAdressesArray[i].IpV6Addr.ip = 0;
		//UdpAdressesArray[i].IpV6Addr.scopeId = 0;

		(UdpAdressesArray[i]).AudioChannelPort          = 7000 + 4 * i;
		UdpAdressesArray[i].VideoChannelPort            = 7002 + 4 * i;
		UdpAdressesArray[i].AudioChannelAdditionalPorts = 0; //ICE 4 ports
		UdpAdressesArray[i].VideoChannelAdditionalPorts = 0; //ICE 4 ports
		FreeFixedUDP[i]                                 = 1; //'1' - free
	}

	m_pRecordingJunction = new std::set<CRecordingJunction>;

	for (i = 0; i < NumOfStartupCondTypes; i++)
	{
		m_StartupEndCondArray[i] = FALSE;
	}

	m_ResourcesStartupOver = FALSE;

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		m_pBoards[i] = new CBoard(i + 1);

		// fields needed for RR
		m_num_configured_ART_Ports[i]   = 0;
		m_num_configured_VIDEO_Ports[i] = 0;
	}

	m_ResourceAllocationType        = eAutoMpmRxMode;
	m_FutureMode                    = eAllocationModeNone;
	m_SystemCardsMode               = eSystemCardsMode_illegal;
	m_maxNumberOfOngoingConferences = MAX_NUMBER_OF_ONGOING_CONFERENCES_COP_VSW_BASED; //4.1c <--> v6 merge toDo: decide the correct value!
	m_PortsConfigurationStep        = PORTS_CONFIGURATION_STEP;
	m_AudioFactor                   = AUDIO_FACTOR;
	m_ProductType                   = CProcessBase::GetProcess()->GetProductType();;

	m_SlotNumberingConversionTable.numOfBoardsInTable = 0;
	for (WORD i = 0; i < MAX_NUM_OF_BOARDS; i++)
	{
		m_SlotNumberingConversionTable.conversionTable[i].boardId        = 0;
		m_SlotNumberingConversionTable.conversionTable[i].subBoardId     = 0;
		m_SlotNumberingConversionTable.conversionTable[i].displayBoardId = 0;
	}
	m_portGauge            = 80;
	m_isMultipleIpServices = 1;

	m_CpuSizeDesc.m_CpuSize     = eSystemCPUSize_illegal;
	m_CpuSizeDesc.m_CpuCapacity = 0;

	m_DspAliveBitmap    = 0;
	m_isLicenseExpired  = false;
	m_DspLocationBitmap = 0;
	m_SVC               = FALSE;
	m_isRecording       = 0;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::InitResourceAllocationMode(eSystemCardsMode curMode)
{
	m_SystemCardsMode = curMode;

	if (curMode == eSystemCardsMode_breeze)
	{
		//read from file and then decide if it's eFixedBreezeMode or eAutoBreezeMode
		CAllocationModeDetails allocationMode;
		allocationMode.ReadFromProcessSetting();
		if (allocationMode.GetMode() == eAllocationModeFixed)
		{
			m_ResourceAllocationType = eFixedBreezeMode;
			PASSERT(CHelperFuncs::IsMode2C());                    // due to 2000c
		}
		else
			m_ResourceAllocationType = eAutoBreezeMode;

		m_PortsConfigurationStep = CalculatePortsConfigurationStep(); //PORTS_CONFIGURATION_STEP_BREEZE;
		m_AudioFactor            = CalculateAudioFactorStep();        //AUDIO_FACTOR_BREEZE;
	}
	else if (curMode == eSystemCardsMode_mpmrx)
	{
		//read from file and then decide if it's eFixedMpmRxMode or eAutoMpmRxMode
		CAllocationModeDetails allocationMode;
		allocationMode.ReadFromProcessSetting();
		if (allocationMode.GetMode() == eAllocationModeFixed)
			m_ResourceAllocationType = eFixedMpmRxMode;
		else
			m_ResourceAllocationType = eAutoMpmRxMode;

		m_PortsConfigurationStep = PORTS_CONFIGURATION_STEP;
		m_AudioFactor            = AUDIO_FACTOR;
	}
	else if (curMode == eSystemCardsMode_mixed_mode)
	{
		m_ResourceAllocationType = eAutoMixedMode;
		m_PortsConfigurationStep = PORTS_CONFIGURATION_STEP_BREEZE;
		m_AudioFactor            = AUDIO_FACTOR_BREEZE;
	}

	m_ResourcesInterfaceArray.InitResourceAllocationMode(m_ResourceAllocationType);

	switch (m_ResourceAllocationType)
	{
		case eFixedBreezeMode:
		case eFixedMpmRxMode:
			if (m_ProductType == eProductTypeRMX4000)
				m_maxNumberOfOngoingConferences = MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_AMOS;
			else
				m_maxNumberOfOngoingConferences = MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_RMX2000;
			m_FutureMode = eAllocationModeFixed;
			m_confIds.Limit(MAX_RSRC_CONF_IDS);
			break;
		case eAutoBreezeMode:
		case eAutoMpmRxMode:
		case eAutoMixedMode:
			if (CHelperFuncs::IsSoftMCU(m_ProductType))
			{
				m_maxNumberOfOngoingConferences = (eProductTypeSoftMCUMfw == m_ProductType) ? MAX_NUMBER_OF_ONGOING_CONFERENCES_SOFT_MFW : MAX_NUMBER_OF_ONGOING_CONFERENCES_SOFT_BASED;
				m_confIds.Limit((eProductTypeSoftMCUMfw == m_ProductType) ? MAX_RSRC_CONF_ID_NUMBER_SOFT_MFW : MAX_RSRC_CONF_ID_NUMBER);
			}
			else
			{
				m_maxNumberOfOngoingConferences = (eProductTypeRMX4000 == m_ProductType) ? MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_AMOS : MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_RMX2000;
				m_confIds.Limit(MAX_RSRC_CONF_IDS);
			}
			m_FutureMode = eAllocationModeAuto;
			break;
		default:
			PASSERT(1);
			break;
	}

	TRACEINTO << "ResourceAllocationType:" << m_ResourceAllocationType << ", maxNumberOfOngoingConferences:" << m_maxNumberOfOngoingConferences;
}

////////////////////////////////////////////////////////////////////////////
CSystemResources::~CSystemResources()
{
	POBJDELETE( m_pNetServicesDB);
	POBJDELETE( m_pAllocationDecider);

	for (int i = 0; i < BOARDS_NUM; ++i)
	{
		POBJDELETE(m_pBoards[i]);
		m_pBoards[i] = NULL;
	}

	PDELETE( m_pIPServices); // each of the contained element's destructors.
	m_pIPServices = 0;
	PDELETE( m_IpServicesResourcesInterface);
	m_IpServicesResourcesInterface = 0;

	if (m_pRecordingJunction)
	{
		m_pRecordingJunction->clear();
		POBJDELETE( m_pRecordingJunction);
		m_pRecordingJunction = 0;
	}
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::UnregisterTaskStateMachines()
{
	for (int i = 0; i < BOARDS_NUM; ++i)
	{
		if (!IsBoardIdExists(i + 1))
			continue;
		CBoard* pBoard = GetBoard(i + 1);
		if (pBoard)
			pBoard->UnregisterStateMachine();
	}
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::InitProductType(eProductType productType)
{
	m_ProductType = productType;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetRamSize(eSystemRamSize ramSize)
{
	m_RamSize = ramSize;
}

void CSystemResources::SetCpuSize(WORD cpuCapacity)
{
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();
	PASSERT_AND_RETURN(!pResourceManager);

	if (cpuCapacity < 1 || cpuCapacity > 200) // cpuCapacity must be percent from max capacity, i.e. 1 - 100%
	{
		TRACEWARN << "CpuCapacity:" << cpuCapacity << " - Wrong value";
		return;
	}

	// BRIDGE-11863 - Do not allow cpuCapacity to exceed 100%. We should limit it to the max license ports.
	if (cpuCapacity > 100)
	{
		cpuCapacity = 100;
		TRACEINTO << "cpuCapacity is " << cpuCapacity << ", limit it to 100%. (to not exceed max license ports)";
	}

	WORD maxNumHD720Ports = (eProductTypeSoftMCUMfw == m_ProductType) ? MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW : MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU;
	m_CpuSizeDesc.m_CpuCapacity = (DWORD)ceil((maxNumHD720Ports * cpuCapacity) / 100.);

	bool  isRPPLicense = pResourceManager->isRPPLicense();
	DWORD dongleNumOfParties = GetDongleNumOfParties();
	DWORD cpuCapNumOfParties = m_CpuSizeDesc.m_CpuCapacity;

	TRACEINTO
		<< "ProductType:" << ProductTypeToString(m_ProductType)
		<< ", CpuCapacity:" << cpuCapacity
		<< ", CpuCapNumOfParties:" << m_CpuSizeDesc.m_CpuCapacity
		<< ", DongleNumOfParties:" << dongleNumOfParties
		<< ", IsRPPLicense:" << (int)isRPPLicense;

	//In case RPP license only we should update number of parties with CPU capacity.
	//If we update a-la-cart mode then user can get the full license always
	if (isRPPLicense || cpuCapNumOfParties < dongleNumOfParties)
		m_ResourcesInterfaceArray.InitDongleRestriction(cpuCapNumOfParties, TRUE);
}

////////////////////////////////////////////////////////////////////////////
int CSystemResources::EnableUnit(BoardID boardId, UnitID unitId, eUnitType unitType, bool& isController)
{
	isController = false; // changed to TRUE only at audio controller enabling.

	PASSERT_AND_RETURN_VALUE(unitType != eUnitType_Art && unitType != eUnitType_Video && unitType != eUnitType_Art_Control, STATUS_FAIL);
	PASSERT_AND_RETURN_VALUE(unitId > MAX_NUM_OF_UNITS, STATUS_FAIL);

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, STATUS_FAIL);

	eCardType cardType = pBoard->GetCardType();

	CUnitMFA* pUnit = new CUnitMFA(boardId, unitId, unitType);
	pUnit->UpdateUnitIDToFPGAIndex(unitId, cardType);

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	if (pMediaUnitslist->find(*pUnit) != pMediaUnitslist->end())
	{
		POBJDELETE(pUnit);
		TRACEINTO << "BoardId:" << boardId << ", UnitId:" << unitId << " - Unit already in list";
		return STATUS_FAIL;
	}

	//***audio cntler_path
	// We will always configure the first unit as AC, since the first unit that is
	// being enabled should be ART and any ART unit can be the AC.

	if ((eUnitType_Art == unitType || eUnitType_Art_Control == unitType) && NO_AC_ID == pBoard->GetAudioControllerUnitId())
	{
		isController = true;
		pBoard->SetAudioControllerUnitId(unitId);
	}
	//***audio cntler_path
	pMediaUnitslist->insert(*pUnit);
	POBJDELETE(pUnit);

	return 0;                             //STATUS_OK
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::SetUnitMfaStatus(WORD boardId, WORD unitId, BYTE enbl_st, BYTE isManually /* = FALSE*/, BYTE isFatal /* = FALSE*/)
{
	PASSERT(unitId > MAX_NUM_OF_UNITS); //phase1 temp
	CUnitMFA* pUnitMFA = NULL;
	CSystemResources* pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst && (eProductTypeNinja == pSyst->GetProductType()))
		pUnitMFA = GetNinjaUnit(boardId, unitId);
	else
		pUnitMFA = GetUnit(boardId, unitId);

	PASSERT_AND_RETURN_VALUE(pUnitMFA == NULL, STATUS_FAIL);

	if (isFatal)
	{
		pUnitMFA->SetFatal(!enbl_st);
	}
	else if (isManually)
	{
		pUnitMFA->SetDisabledManually(!enbl_st);
	}
	else
	{
		pUnitMFA->SetEnabled(enbl_st);
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::SetSpanRTMStatus(WORD boardId, WORD unitId, BYTE enbl_st, BYTE isManually /* = FALSE*/, BYTE isFatal /* = FALSE*/)
{
	PASSERT(unitId > MAX_NUM_OF_UNITS); //phase1 temp
	CSpanRTM* pRTM = NULL;
	CSystemResources* pSyst = CHelperFuncs::GetSystemResources();
	if ((pSyst && (eProductTypeNinja != pSyst->GetProductType())) || boardId != ISDN_CARD_SLOT_ID)
	{
		TRACEINTO << " CSystemResources::SetSpanRTMStatus - only Ninja ISDN Card support.";
		return STATUS_FAIL;
	}

	boardId = 1;
	TRACEINTO << "BoardId:" << boardId << ", UnitId:" << unitId << ", IsManually:" << (int)isManually << ", IsEnable:" << (int)enbl_st << ", IsFatal:" << (int)isFatal;

	CBoard* pBoard = GetBoard(boardId);
	if (pBoard == NULL)
	{
		TRACEINTO << "BoardId:" << boardId << ", UnitId:" << unitId;
		return STATUS_FAIL;
	}

	pRTM = (CSpanRTM*)(pBoard->GetRTM(unitId));

	PASSERT_AND_RETURN_VALUE(pRTM == NULL, STATUS_FAIL);

	if (isFatal)
	{
		pRTM->SetFatal(!enbl_st);
	}
	else if (isManually)
	{
		pRTM->SetDisabledManually(!enbl_st);
	}
	else
	{
		pRTM->SetEnabled(enbl_st);
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetSlotNumberingConversionTable(SLOTS_NUMBERING_CONVERSION_TABLE_S* pSlotNumberingConversionTable)
{
	m_SlotNumberingConversionTable.numOfBoardsInTable = pSlotNumberingConversionTable->numOfBoardsInTable;
	for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
	{
		m_SlotNumberingConversionTable.conversionTable[i].boardId        = pSlotNumberingConversionTable->conversionTable[i].boardId;
		m_SlotNumberingConversionTable.conversionTable[i].subBoardId     = pSlotNumberingConversionTable->conversionTable[i].subBoardId;
		m_SlotNumberingConversionTable.conversionTable[i].displayBoardId = pSlotNumberingConversionTable->conversionTable[i].displayBoardId;
	}
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetDisplayBoardId(WORD bid, WORD subBid)
{
	for (WORD i = 0; i < m_SlotNumberingConversionTable.numOfBoardsInTable; i++)
	{
		if (m_SlotNumberingConversionTable.conversionTable[i].boardId == bid
		    && m_SlotNumberingConversionTable.conversionTable[i].subBoardId == subBid)
			return m_SlotNumberingConversionTable.conversionTable[i].displayBoardId;
	}
	return NO_DISPLAY_BOARD_ID;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSystemResources::GetIDsFromDisplayBoardId(WORD displayBID, eProductType prodType, WORD& bid, WORD& subBid)
{
	for (WORD i = 0; i < m_SlotNumberingConversionTable.numOfBoardsInTable; i++)
	{
		if (m_SlotNumberingConversionTable.conversionTable[i].displayBoardId == displayBID)
		{
			bid = m_SlotNumberingConversionTable.conversionTable[i].boardId;
			if (eProductTypeRMX2000 != prodType)
				subBid = m_SlotNumberingConversionTable.conversionTable[i].subBoardId;
			return TRUE;
		}
	}

	bid = NO_DISPLAY_BOARD_ID;
	subBid = NO_DISPLAY_BOARD_ID;
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::SetBoardMfaStatus(BoardID boardId, BYTE enbl_st, BYTE isManually)
{
	STATUS status = STATUS_FAIL;
	FTRACEINTO << "BoardIdL" << boardId;
	if (IsBoardIdExists(boardId))
	{
		CBoard* pBoard = GetBoard(boardId);
		PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);

		FTRACEINTO << "IsBoardIdExists:TRUE";
		for (int i = 0; i < MAX_NUM_OF_UNITS - 1; i++)
		{
			CUnitMFA* pMFA = (CUnitMFA*)pBoard->GetMFA(i + 1);
			if (!pMFA)
				continue;
			if (isManually)
				pMFA->SetDisabledManually(!enbl_st);
			else
				pMFA->SetEnabled(enbl_st);
		}
		status = STATUS_OK;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetUnitMfaStatus(WORD boardId, WORD unitId, BYTE& enbl_st)
{
	STATUS status = STATUS_OK;
	PASSERT(unitId > MAX_NUM_OF_UNITS); //phase1 temp

	CUnitMFA* pUnitMFA = GetUnit(boardId, unitId);
	PASSERT_AND_RETURN_VALUE(pUnitMFA == NULL, STATUS_FAIL);

	enbl_st = pUnitMFA->GetIsEnabled();
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::DumpBrdsStateCmd(std::ostream& answer)
{
	CBoard* pBoard;
	CActivePortsList::iterator itr;
	std::set<CUnitMFA>::iterator i;
	const CActivePortsList* pActivePorts;

	for (int indx = 0; indx < BOARDS_NUM; indx++)
	{
		if (!IsBoardIdExists(indx + 1))
			continue;

		pBoard = GetBoard(indx + 1);
		if (pBoard == NULL)
			continue;

		CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
		if (pMediaUnitslist == NULL)
			continue;

		i = pMediaUnitslist->begin();

		answer << "\n\nRESOURCE_CAUSE: resources map for BoardId=" << i->GetBoardId();

		answer << "\nBoard data and Ports List[PortId,PortType,ConfId,PartyId]:";
		for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
		{
			answer << "\n SunboardId=" << i->GetSubBoardId() << " UnitId=" << i->GetUnitId() << " UnitType=" << i->GetUnitType() << " FreeCapacity=" << i->GetFreeCapacity() << " NumOfActivePorts=" << i->GetPortNumber() << ": ";

			pActivePorts = i->GetActivePorts();

			if (pActivePorts == NULL)
				continue;

			for (itr = pActivePorts->begin(); itr != pActivePorts->end(); itr++)
			{
				answer << " [" << itr->GetPortId() << "," << (eResourceTypes)itr->GetPortType() << "," << itr->GetConfId() << "," << itr->GetPartyId() << "]";
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::DumpBrdsState(DataPerBoardStruct* pAllocDataPerBoardArray = NULL)
{
	CBoard* pBoard;
	CActivePortsList::iterator itr;
	std::set<CUnitMFA>::iterator i;
	const CActivePortsList* pActivePorts;

	ALLOCBUFFER(StrBuf, 1024);

	for (int indx = 0; indx < BOARDS_NUM; indx++)
	{
		if (!IsBoardIdExists(indx + 1))
			continue;

		pBoard = GetBoard(indx + 1);
		if (pBoard == NULL)
			continue;

		CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
		if (pMediaUnitslist == NULL)
			continue;

		std::string trace_string;
		char* pStrBuf = StrBuf;

		i = pMediaUnitslist->begin();

		sprintf(pStrBuf, "\nRESOURCE_CAUSE: resources map for BoardId=%d\n", i->GetBoardId());
		trace_string += pStrBuf;

		if (pAllocDataPerBoardArray)
		{
			sprintf(pStrBuf, "numFreeVideoCapacity:%d, DownGradedPartyType=%s, WasDowngraded=%d, FragmentedUnit=%d, NumVideoUnitsToReconfigure=%d, NumVideoPartiesSameConf=%d.",
			        pAllocDataPerBoardArray[indx].m_VideoData.m_numFreeVideoCapacity,
			        eVideoPartyTypeNames[pAllocDataPerBoardArray[indx].m_VideoData.m_DownGradedPartyType],
			        pAllocDataPerBoardArray[indx].m_VideoData.m_bWasDownGraded,
			        pAllocDataPerBoardArray[indx].m_VideoData.m_bFragmentedUnit,
			        pAllocDataPerBoardArray[indx].m_VideoData.m_NumVideoUnitsToReconfigure,
			        pAllocDataPerBoardArray[indx].m_VideoData.m_NumVideoPartiesSameConf );
			trace_string += pStrBuf;

			WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
			for (int x = 0; x < max_units_video; x++)
			{
				sprintf(pStrBuf, " [choosed %dth unit=%d]", x, pAllocDataPerBoardArray[indx].m_VideoData.m_VideoAlloc.m_unitsList[x].m_UnitId);
				trace_string += pStrBuf;
			}
		}
		sprintf(pStrBuf, "\nBoard data and Ports List[PortId,PortType,ConfId,PartyId]:");
		trace_string += pStrBuf;
		for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
		{
			sprintf(pStrBuf, "\n SunboardId=%d UnitId=%d UnitType=%d FreeCapacity=%0.2f NumOfActivePorts=%d: ", i->GetSubBoardId(), i->GetUnitId(), i->GetUnitType(), i->GetFreeCapacity(), i->GetPortNumber());
			trace_string += pStrBuf;

			pActivePorts = i->GetActivePorts();

			if (pActivePorts == NULL)
				continue;

			for (itr = pActivePorts->begin(); itr != pActivePorts->end(); itr++)
			{
				sprintf(pStrBuf, " [%d,%d,%d,%d]", itr->GetPortId(), (eResourceTypes)itr->GetPortType(), itr->GetConfId(), itr->GetPartyId());
				trace_string += pStrBuf;
			}
		}
		PTRACE(eLevelInfoNormal, trace_string.c_str());
	}

	DEALLOCBUFFER(StrBuf);
}

////////////////////////////////////////////////////////////////////////////
const CNetServiceRsrcs* CSystemResources::findServiceByName(const char* name)
{
	return m_pNetServicesDB->FindServiceByName(name);
}

////////////////////////////////////////////////////////////////////////////
CNetPortsPerService* CSystemResources::FindNetPortsPerServiceByName(const char* name)
{
	return m_pNetServicesDB->FindNetPortsPerServiceByName(name);
}

////////////////////////////////////////////////////////////////////////////
int CSystemResources::EnableNetService(char* name)
{
	/***** Enable with Span?? Enable with Phone?? ****/

	int status;

	//CNetServiceRsrcs* pService = new CNetServiceRsrcs(name);
	const CNetServiceRsrcs* pExistService = findServiceByName(name);

	if (pExistService)
	{
		PASSERT(1);  //already found
		status = -1; //NOT_FIND ->
	}
	else
	{
		CNetServiceRsrcs* pService = new CNetServiceRsrcs(name);
		m_pNetServicesDB->GetNetServices()->insert(*pService);
		status = 0; //success
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
int CSystemResources::DisableNetService(char* name)
{
	/*PASSERT(boardId != 1 || unitId > 26); //phase1 temp
	 *****check if trayng to disable Active Service??? (forbid)****/
	int  status;

	const CNetServiceRsrcs* pService = findServiceByName(name);
	if (pService)
	{
		m_pNetServicesDB->GetNetServices()->erase(*pService);
		POBJDELETE ( pService);
		status = 0;
	}
	else
	{
		PASSERT(1);  //not found
		status = -1; //NOT_FIND ->
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetKeepAliveList(RSRCALLOC_KEEP_ALIVE_S* pKeepAliveList, eChangedStateAC& stateAC)
{
	KEEP_ALIVE_S p_keepAliveStruct = pKeepAliveList->keepAliveStruct;
	WORD bId = pKeepAliveList->physicalHeader.board_id;

	BYTE enbl_st = TRUE;
	DWORD u_stat = 0;
	CBoard* pBoard = GetBoard(bId);
	PASSERT_AND_RETURN(pBoard == NULL);

	eProductType prodType = GetProductType();

	for (int i = 0; i < MAX_NUM_OF_UNITS - 1; i++)
	{
		CUnitMFA* pMFA = (CUnitMFA*)pBoard->GetMFA(i + 1);
		if (!pMFA)
			continue;
		enbl_st = pMFA->GetIsEnabled();
		u_stat = p_keepAliveStruct.statusOfUnitsList[i + 1];
		if (TRUE == enbl_st && u_stat != 0)
		{
			pMFA->SetEnabled(FALSE); // turn unit to disable
			TRACEINTO << " Due to keep alive, the unit being disabled, BoardId = " << bId << " UnitId = " << i + 1 << "\n";
			if (eProductTypeRMX4000 == prodType)
			{
				if (GetAudioCntrlMasterBid() == bId && (pMFA->GetUnitId() == pBoard->GetAudioControllerUnitId()))
				{
					stateAC = eAC_Failed;
					TRACEINTO << " Master AC unit being disabled !!!\n"; //TODO: trace
				}
			}
		}
		else if (TRUE != enbl_st && 0 == u_stat)
		{
			pMFA->SetEnabled(TRUE);                                            // turn back to enable
			TRACEINTO << " Due to keep alive, the unit being enabled, BoardId = " << bId << " UnitId = " << i + 1 << "\n";

			if (eProductTypeRMX4000 == prodType)
			{
				if (GetAudioCntrlMasterBid() == 0xFFFF && (pMFA->GetUnitId() == pBoard->GetAudioControllerUnitId()))
				{
					stateAC = eAC_Back;
					TRACEINTO << " Master AC unit came back !!!\n"; //TODO: trace
				}
			}
		}
		else
			continue;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::SetFipsInd(RSRCALLOC_ART_FIPS_140_IND_S* pFipsInd)
{
	STATUS status = STATUS_FAIL;
	WORD bId = pFipsInd->physicalHeader.board_id;
	WORD uId = pFipsInd->physicalHeader.unit_id;
	DWORD u_stat = pFipsInd->status;

	CBoard* pBoard = GetBoard(bId);

	if (pBoard)
	{
		CUnitMFA* pMFA = (CUnitMFA*)pBoard->GetMFA(uId);

		if (pMFA)
		{
			BYTE stat_flag = (STATUS_OK == u_stat) ? TRUE : FALSE;
			pMFA->SetFipsStat(stat_flag);

			TRACEINTO << "SetFipsInd, BoardId = " << bId << " UnitId = " << uId << " unit status = " << u_stat << "\n";
			status = STATUS_OK;
		}
	}
	else
		PASSERT(1);

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetIpMediaConfigFailure(DWORD boardId, DWORD PQnumber)
{
	// *** since there is only one PQ per board now ( 16/03/06 ) this module should
	// *** disable the whole specific board ( boardId ). In further development the relation between
	// *** PQ and specific units should be established and relevant part of the board should be disabled.

	TRACEINTO << "Due to ip media config failure, the BOARD being disabled, BoardId = " << boardId << "\n";

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN(pBoard == NULL);

	for (int i = 0; i < MAX_NUM_OF_UNITS - 1; i++)
	{
		CUnitMFA* pMFA = (CUnitMFA*)pBoard->GetMFA(i + 1);

		if (!pMFA)
			continue;

		pMFA->SetEnabled(FALSE); // turn unit to disable

	}
	// write infrastructure for disabling only units related to specific PQ
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::IsRsrcEnough(bool isKeepAlive)
{
	m_bIsRsrcEnough = FALSE;

	STATUS status = STATUS_OK;

	std::ostringstream msg;
	msg << "CSystemResources::IsRsrcEnough:";

	CBoardsStatistics boardsStatistics;

	DWORD numOfAudioPortsPerUnit[BOARDS_NUM];
	DWORD numOfVideoPortsPerUnit[BOARDS_NUM];

	for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		numOfAudioPortsPerUnit[boardId] = 0;
		numOfVideoPortsPerUnit[boardId] = 0;

		if (!IsBoardReady(boardId + 1))
			continue;

		CBoard* pBoard = m_pBoards[boardId];
		CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

		for (std::set<CUnitMFA>::iterator i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
		{
			boardsStatistics.m_NumOfUnits[boardId]++;

			if (isKeepAlive && (TRUE != i->GetIsEnabled() || TRUE != i->GetFipsStat()))
				continue;

			if (eUnitType_Art == i->GetUnitType())
				boardsStatistics.m_NumOfEnabledArtUnits[boardId]++;
			if (eUnitType_Video == i->GetUnitType())
				boardsStatistics.m_NumOfEnabledVideoUnits[boardId]++;
		}

		numOfAudioPortsPerUnit[boardId] = pBoard->CalculateNumberOfPortsPerUnit(PORT_ART);
		numOfVideoPortsPerUnit[boardId] = pBoard->CalculateNumberOfPortsPerUnit(PORT_VIDEO);

		boardsStatistics.m_NumConfiguredAudioPorts[boardId] = numOfAudioPortsPerUnit[boardId] * boardsStatistics.m_NumOfEnabledArtUnits[boardId];
		boardsStatistics.m_NumConfiguredVideoPorts[boardId] = numOfVideoPortsPerUnit[boardId] * boardsStatistics.m_NumOfEnabledVideoUnits[boardId];

		boardsStatistics.m_NumOfEnabledUnits[boardId] = boardsStatistics.m_NumOfEnabledArtUnits[boardId] + boardsStatistics.m_NumOfEnabledVideoUnits[boardId];

		msg << "\n --------------------------------"
		    << "\n Board Statistics for boardId = " << boardId << ":"
		    << "\n  numOfAudioPortsPerUnit    = " << numOfAudioPortsPerUnit[boardId]
		    << "\n  numOfVideoPortsPerUnit    = " << numOfVideoPortsPerUnit[boardId]
		    << "\n  m_NumOfEnabledArtUnits    = " << boardsStatistics.m_NumOfEnabledArtUnits[boardId]
		    << "\n  m_NumOfEnabledVideoUnits  = " << boardsStatistics.m_NumOfEnabledVideoUnits[boardId]
		    << "\n  m_NumConfiguredAudioPorts = " << boardsStatistics.m_NumConfiguredAudioPorts[boardId]
		    << "\n  m_NumConfiguredVideoPorts = " << boardsStatistics.m_NumConfiguredVideoPorts[boardId]
		    << "\n  totalNumOfUnits           = " << boardsStatistics.m_NumOfUnits[boardId]
		    << "\n  totalNumOfEnabledUnits    = " << boardsStatistics.m_NumOfEnabledUnits[boardId];
	}
	PTRACE(eLevelInfoNormal, msg.str().c_str());

	status = m_ResourcesInterfaceArray.IsRsrcEnough(&boardsStatistics);
	if (STATUS_OK == status && GetMultipleIpServices())
	{
		for (std::set<CIpServiceResourcesInterfaceArray>::iterator _itr = m_IpServicesResourcesInterface->begin(); _itr != m_IpServicesResourcesInterface->end(); _itr++)
			((CIpServiceResourcesInterfaceArray*)(&(*_itr)))->IsRsrcEnough(&boardsStatistics);
	}

	if (status == STATUS_OK)
		m_bIsRsrcEnough = TRUE;

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::ConfigureIPServicePQResources(IP_SERVICE_UDP_RESOURCES_S* pIPService, UDP_PORT_RANGE_S& udpPortRange)
{
	STATUS status;
	STATUS ret_status = STATUS_OK;
	WORD servId = pIPService->ServId;

	// if service not exists - create service (generally, service may exist)
	CIPServiceResources* pIPServiceResources = (CIPServiceResources*)GetIPService(servId);
	if (pIPServiceResources)
	{
		TRACEINTO << "ServiceId:" << pIPService->ServId << " - Already exist, reconfigure it";
		status = pIPServiceResources->RemoveAllPQM();
	}
	else
	{
		status = AddIPService(servId, pIPService->ServName, pIPService->service_default_type);
	}

	int num = 0;     // number of successful enables
	bool bReachedLimit = false; // Flag to know if reached limit in case of fixed ports

	std::vector<WORD> udpChannelsNumber;
	std::vector<WORD> udpFirstPort;
	std::vector<WORD> udpLastPort;

	CPrettyTable<WORD, WORD, WORD, WORD, WORD, WORD, const char*, WORD, WORD, WORD> tbl("BoxId", "BoardId", "SubBoardId", "SubServiceId", "PqId", "PqNumber", "AllocationType", "ChannelsNumber", "FirstPort", "LastPort");
	for (WORD i = 0; i < pIPService->numPQSactual; ++i)
	{
		IP_SERVICE_UDP_RESOURCE_PER_PQ_S& temp = pIPService->IPServUDPperPQList[i];

		if (eMethodStatic == temp.portsAlloctype)        // Fixed Port Range
		{
			udpChannelsNumber.push_back(m_ResourcesInterfaceArray.GetNumberOfRequiredUDPPortsPerBoard(pIPService->iceEnvironment));
			udpFirstPort.push_back(temp.UdpFirstPort);
			udpLastPort.push_back(temp.UdpLastPort); //VNGFE-7746

			WORD numConfigured = temp.UdpLastPort - udpFirstPort[i] + 1;
			if (numConfigured < udpChannelsNumber[i])
			{
				bReachedLimit = true;
			}
		}
		else
		{
			udpChannelsNumber.push_back(m_ResourcesInterfaceArray.GetNumberOfRequiredUDPPortsPerBoard(eIceEnvironment_ms));
			udpFirstPort.push_back(FIRST_UDP_PORT);
			udpLastPort.push_back(udpFirstPort[i] + udpChannelsNumber[i] - 1);
		}

		tbl.Add(1, temp.boardId, temp.subBoardId, temp.subServiceId, temp.PQid, i, (temp.portsAlloctype ? "Dynamic" : "Static (Fixed)"), udpChannelsNumber[i], udpFirstPort[i], udpLastPort[i]);
	}

	TRACEINTO << "ServiceId:" << pIPService->ServId << ", ServiceName:" << pIPService->ServName << ", IceEnvironment:" << pIPService->iceEnvironment << ", NumOfPQ:" << pIPService->numPQSactual << tbl.Get();

	for (WORD i = 0; i < pIPService->numPQSactual; ++i)
	{
		IP_SERVICE_UDP_RESOURCE_PER_PQ_S& curPQ = pIPService->IPServUDPperPQList[i];

		CPQperSrvResource pq(1, curPQ.boardId, curPQ.subBoardId, curPQ.PQid, curPQ.subServiceId);

		pq.SetServiceId(pIPService->ServId);
		pq.SetType(curPQ.type);
		pq.SetIpType(curPQ.IpType);
		pq.SetIpV4Addr(curPQ.IpV4Addr);
		pq.SetIpV6Addr(curPQ.IpV6Addr);
		pq.SetUDPallocType(curPQ.portsAlloctype);

		if (curPQ.IpType >= NUM_OF_IP_TYPES)
			break;

		pq.SetUDPChannels(udpFirstPort[i], udpLastPort[i]);

		// Update First and Last UDP ports to send to CS.
		udpPortRange.UdpFirstPort = udpFirstPort[i];
		udpPortRange.UdpLastPort = udpLastPort[i];

		if (pq.IsIpConfigured())
		{
			status = EnablePQMonService(servId, &pq);
		}
		else
		{
			TRACEINTO << "BoardId:" << pq.GetBoardId() << ", PqId:" << pq.GetPQMId() << " - Failed, IP address not configured, not added";
			status = STATUS_OK;
		}

		if (status != STATUS_OK)
			ret_status = status;
		else
			++num;
	}

	// Active Alarm in case of UDP ports shortage
	if (bReachedLimit)
		CProcessBase::GetProcess()->AddActiveAlarmFromProcess(FAULT_GENERAL_SUBJECT, INSUFFICIENT_UDP_PORTS, SYSTEM_MESSAGE, "Insufficient UDP ports according to configuration", true, true);

	if (pIPService->numPQSactual) // if service was no empty
		PASSERT(num == 0);    // and no PQ succeeded to enable

	return ret_status;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::IPv6ServiceUpdate(IPV6_ADDRESS_UPDATE_RESOURCES_S* pIPv6AddressUpdate)
{
	WORD servId = pIPv6AddressUpdate->ServId;
	CIPServiceResources* pIPService = (CIPServiceResources*)GetIPService(pIPv6AddressUpdate->ServId);

	if (pIPService)
	{
		for (int spanIndex = 0; spanIndex < MAX_NUM_PQS; spanIndex++)
		{
			CPQperSrvResource  PQM(1, //pIPv6AddressUpdate->IPServUDPperPQList[spanIndex].boxId,
			                       pIPv6AddressUpdate->IPServUDPperPQList[spanIndex].boardId,
			                       pIPv6AddressUpdate->IPServUDPperPQList[spanIndex].subBoardId,
			                       pIPv6AddressUpdate->IPServUDPperPQList[spanIndex].PQid);
			CPQperSrvResource* pPQM = (CPQperSrvResource*)(pIPService->GetPQM(PQM));
			if (pPQM)
			{
				pPQM->SetIpV6Addr(pIPv6AddressUpdate->IPServUDPperPQList[spanIndex].IpV6Addr);
			}
			else
				TRACEINTO << "\nCSystemResources::IPv6ServiceUpdate - No PQM in span:" << spanIndex;
		}
	}
	else
		PASSERT(2);
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeleteIPService(Del_Ip_Service_S* pIPService)
{
	WORD servId = pIPService->service_id;

	STATUS status = RemoveIPService(servId);
	PASSERT(status);                           //+trace?
	return status;

	//*** all for now. Checking if Service active (udp ports allocated?) - later.
}

////////////////////////////////////////////////////////////////////////////
CBoard* CSystemResources::GetBoard(WORD bId) const //1-based index!!!!
{
	if (CHelperFuncs::IsValidBoardId(bId))
		return m_pBoards[bId - 1];

	PTRACE2INT(eLevelInfoNormal, "CSystemResources::GetBoard: not MPM board: ", bId);

	return NULL;
}

////////////////////////////////////////////////////////////////////////////
CBoard* CSystemResources::GetBoardByDisplayId(WORD displayId)
{       // this function not in use
	WORD bid, subBid;
	PASSERT(1);
	if (GetIDsFromDisplayBoardId(displayId, eProductTypeRMX4000, bid, subBid) == FALSE)
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::GetBoardByDisplayId: Invalid displayId: ", displayId);
		return NULL;
	}

	if (subBid != MEDIA_SUB_BOARD_NUM)
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::GetBoardByDisplayId: DisplayId is not for media board - displayId: ", displayId);
		PASSERT(1);
		return NULL;
	}
	return GetBoard(bid);
}

////////////////////////////////////////////////////////////////////////////
CUnitMFA* CSystemResources::GetUnit(WORD bId, WORD uid) //bid: 1-based index!!!!
{
	CBoard* pBoard = GetBoard(bId);
	if (pBoard == NULL)
		return NULL;

	CUnitMFA* pUnitMFA = (CUnitMFA*)(pBoard->GetMFA(uid));
	if (pUnitMFA == NULL)
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::GetUnit: Invalid unit: ", uid);
		PASSERT(1);
	}
	return pUnitMFA;
}

////////////////////////////////////////////////////////////////////////////
CUnitMFA* CSystemResources::GetNinjaUnit(WORD displayBoardId, WORD displayUnitId)
{
	PASSERT_AND_RETURN_VALUE((DSP_CARD_SLOT_ID_0 > displayBoardId || DSP_CARD_SLOT_ID_2 < displayBoardId), NULL);
	PASSERT_AND_RETURN_VALUE(displayUnitId >= NUM_OF_UNIT_PER_BOARD_NINJA, NULL);

	//The mapping from displayBoradId + displayUnitId to    unitId
	//displayBoardId:   7	7	7	7	7	7	8	8	8	8	8	8	6	6	6	6	6	6
	//displayUnitId :       0	1	2	3	4	5	0	1	2	3	4	5	0	1	2	3	4	5
	//UnitId:			6	7	8	9	10	11	12	13	14	15	16	17	18	19	20	21	22	23
	WORD displayBoardOffset[3] = { 2, 0, 1 };

	WORD boardId = 1;                              //boardId is hardcode as 1 in Ninja
	WORD unitId = VIDEO_UNIT_START_NUMBER_NINJA + //start video unit offset
	    NUM_OF_UNIT_PER_BOARD_NINJA * displayBoardOffset[displayBoardId - DSP_CARD_SLOT_ID_0] + displayUnitId;

	CBoard* pBoard = GetBoard(boardId);
	if (pBoard == NULL)
	{
		PASSERT_AND_RETURN_VALUE(1000 + boardId, NULL);
	}

	return (CUnitMFA*)(pBoard->GetMFA(unitId));
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::EnableSpan(WORD boardId, WORD unitId, eRTMSpanType SpanType)
{
	PASSERT(SpanType != TYPE_SPAN_T1 && SpanType != TYPE_SPAN_E1);
	//PASSERT(boardId != 1 || unitId > 26); //phase1 temp
	/**** UNITID FOR SPAN??? ***/

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	CSpanRTM* pNewSpanRTM = new CSpanRTM(boardId, unitId);

	if (pSpanslist->find(*pNewSpanRTM) != pSpanslist->end())
	{
		POBJDELETE(pNewSpanRTM);
		return STATUS_ALREADY_EXISTS;
	}

	pSpanslist->insert(*pNewSpanRTM);
	POBJDELETE(pNewSpanRTM);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DisableSpan(WORD boardId, WORD unitId)
{
	//PASSERT(boardId != 1 || unitId > 26); //phase1 temp
	/**** UNITID FOR SPAN??? ***/

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	STATUS status;

	CSpanRTM* pExistSpanRTM = new CSpanRTM(boardId, unitId);

	std::set<CSpanRTM>::iterator i = pSpanslist->find(*pExistSpanRTM);

	if (i != pSpanslist->end())
	{
		m_pNetServicesDB->SpanRemoved((CSpanRTM*)(&(*i)));
		pSpanslist->erase(*pExistSpanRTM);
		//POBJDELETE ( pRemovedSpanRTM);  //***STL memory management???
		status = STATUS_OK;
	}
	else
		status = STATUS_NOT_FOUND;  //NOT_FIND

	POBJDELETE(pExistSpanRTM);
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::AddIPService(WORD id, char* name, BYTE default_H323_SIP_service)
{
	CIPServiceResources ipService(id, name, default_H323_SIP_service);

	if (m_pIPServices->find(ipService) != m_pIPServices->end())
	{
		PASSERT(1); // already exists, trace?
		return STATUS_FAIL;
	}

	m_pIPServices->insert(ipService);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
const CIPServiceResources* CSystemResources::GetIPService(WORD id) const
{
	CIPServiceResources service(id);
	std::set<CIPServiceResources>::iterator i = m_pIPServices->find(service);

	if (i == m_pIPServices->end())
		return NULL;

	return &(*i);
}

////////////////////////////////////////////////////////////////////////////
const CIPServiceResources* CSystemResources::GetFirstIPService() const
{
	if (m_pIPServices && !m_pIPServices->empty())
	{
		std::set<CIPServiceResources>::iterator i;
		i = m_pIPServices->begin();
		return &(*i);
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////
const CIPServiceResources* CSystemResources::GetIPServiceByBoardId(WORD boardId) const
{
	if (m_pIPServices)
	{
		std::set<CIPServiceResources>::iterator ip_service_itr = m_pIPServices->begin();
		for (; ip_service_itr != m_pIPServices->end(); ++ip_service_itr)
		{
			if (ip_service_itr->IsConfiguredToBoard(boardId))
				return &(*ip_service_itr);
		}
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::RemoveIPService(WORD id)
{
	if (m_pIPServices->find(id) == m_pIPServices->end())
	{
		PASSERT(1); //not found
		return STATUS_FAIL;
	}

	m_pIPServices->erase(id);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::EnablePQMonService(WORD servId, CPQperSrvResource* pPQM)
{
	CIPServiceResources* pIPService = (CIPServiceResources*)GetIPService(servId);
	PASSERT_AND_RETURN_VALUE(!pIPService, STATUS_FAIL); // service not found, trace?

	return pIPService->AddPQM(pPQM);                    // check (and block)if PQM exist - inside
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DisablePQMonService( WORD servId, CPQperSrvResource& PQM )
{
	CIPServiceResources* pIPService = ( CIPServiceResources* )(GetIPService(servId));
	if (!pIPService)
	{
		PASSERT(1);                     //service not found, trace?
		return STATUS_FAIL;
	}
	return (pIPService->RemovePQM( &PQM )); //check (and block)if PQM exist - inside
}

////////////////////////////////////////////////////////////////////////////
CPQperSrvResource* CSystemResources::ARTtoPQM(WORD servId, WORD subServiceId, WORD boxId, WORD boardId, WORD subBoardId, WORD unitId) const
{
	std::ostringstream msg;
	msg << "ServiceId:" << servId << ", SubServiceId:" << subServiceId << ", BoxId:" << boxId << ", BoardId:" << boardId << ", SubBoardId:" << subBoardId << ", UnitId:" << unitId;

	// multiple services
	const CIPServiceResources* pIPService = GetMultipleIpServices() ? GetIPService(servId) : GetIPServiceByBoardId(boardId);
	PASSERT_AND_RETURN_VALUE(!pIPService, NULL);

	msg << ", ServiceName:" << pIPService->GetName();
	TRACEINTO << msg.str().c_str();

	//***logic of "PQM to ART" responsibility should be provided by hardware.

	const CPQperSrvResource* pPQM = NULL;

	// in RMX 1500 with 2 services the second service configured to PQ2
	const WORD MAX_PQMID = 2;

	for (WORD PQMid = 1; PQMid <= MAX_PQMID; ++PQMid)
	{
		pPQM = pIPService->GetPQM(CPQperSrvResource(boxId, boardId, subBoardId, PQMid, subServiceId));

		if (pPQM)
			break;

		TRACEINTO << " failed to find PQ " << PQMid << ", for service: " << pIPService->GetName();
	}

	return const_cast<CPQperSrvResource*>(pPQM); //check not NULL in calling function
}

////////////////////////////////////////////////////////////////////////////
BOOL CSystemResources::IsBoardBlngToSubSevice(WORD boardId, PartyDataStruct& partyData)
{
	CIPServiceResources* pIPService = (CIPServiceResources*)(GetFirstIPService());
	BOOL ret_val = FALSE;
	if (pIPService)
	{
		WORD PQMid = 1; //because while only ONE PQ for MFA (unitId < 5) ? 1 : 2 ; //***18/08 changed!!!
		CPQperSrvResource PQM(1, boardId, 1, PQMid, partyData.m_subServiceId);
		CPQperSrvResource* pPQM = (CPQperSrvResource*)(pIPService->GetPQM(PQM));
		ret_val = (pPQM != NULL) ? TRUE : FALSE;
	}
	else
	{
		PASSERT(1);
	}
	return ret_val;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CheckUDPports(WORD servId, WORD subServiceId, WORD boxId,
                                       WORD boardId, WORD subBoardId, WORD unitId, eVideoPartyType videoPartyType,
                                       BOOL isIceParty, BOOL isBFCPUDP) // ICE 4 ports
{
	TRACEINTO
	<< "ServiceId:" << servId
	<< ", SubServiceId:" << subServiceId
	<< ", BoardId:" << boardId
	<< ", SubBoardId:" << subBoardId
	<< ", UnitId:" << unitId
	<< ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType]
	<< ", IsIceParty:" << (WORD)isIceParty
	<< ", IsBfcpUDP:" << (WORD)isBFCPUDP;

	if (IS_UDP_FIXED_PORT_SIM != 0) // *** if fixed UDP port - allow any allocation without UDP check
		return STATUS_OK;

	const CPQperSrvResource* pPQM = ARTtoPQM(servId, subServiceId, boxId, boardId, subBoardId, unitId);
	if (!pPQM)
	{
		PTRACE(eLevelInfoNormal, "CSystemResources::CheckUDPports ARTtoPQM failed");
		return STATUS_FAIL;
	}

	// eVideoPartyType videoPartyType = (physType == ePhysical_art)? eCP_H264_upto_CIF_video_party_type : eVideo_party_type_none;
	// temp. only for VOIP and VIDEO, no fecc, no content

	STATUS status = pPQM->CanAllocateUDP(videoPartyType, isBFCPUDP, isIceParty);

	if (status != STATUS_OK)
		PTRACE(eLevelInfoNormal, "CSystemResources::CheckUDPports CanAllocateUDP failed");

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetStartupCond(eStartupCondType condType, BYTE value)
{
	if (condType < NumOfStartupCondTypes)
	{
		m_StartupEndCondArray[condType] = value;
	}
	else
	{
		string buff = "Illegal index : ";
		buff += condType;
		PASSERTMSG(1, buff.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::IsStartupOk()
{
	STATUS status = STATUS_OK;
	for (int i = 0; i < NumOfStartupCondTypes; i++)
	{
		if (FALSE == m_StartupEndCondArray[i])
		{
			status = STATUS_FAIL;
			break;
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::IsCondOk(eStartupCondType condType)
{
	STATUS status = STATUS_FAIL;
	if (condType < NumOfStartupCondTypes)
	{
		status = (TRUE == m_StartupEndCondArray[condType] ? STATUS_OK : status);
	}
	else
	{
		string buff = "Illegal index : ";
		buff += condType;
		PASSERTMSG(1, buff.c_str());
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CalculateResourceReport(CRsrcReport* pReport)
{
	PASSERT_AND_RETURN_VALUE(pReport == NULL, STATUS_FAIL);

	if (m_ResourcesInterfaceArray.IsInterfaceArrayInitiated())
	{
		m_ResourcesInterfaceArray.CalculateResourceReport(pReport);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSystemResources::CalculateResourceReport - m_ResourcesInterfaceArray was not initiated");
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CalculateConfResourceReport(CSharedRsrcConfReport* pReport)
{
	PASSERT_AND_RETURN_VALUE(pReport == NULL, STATUS_FAIL);
	PTRACE(eLevelInfoNormal, "CSystemResources::CalculateConfResourceReport");
	if (m_ResourcesInterfaceArray.IsInterfaceArrayInitiated())
	{
		m_ResourcesInterfaceArray.CalculateConfResourceReport(pReport);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CSystemResources::CalculateConfResourceReport - m_ResourcesInterfaceArray was not initiated");
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CalculateServicesResourceReport(CServicesRsrcReport* pReport)
{
	PASSERT_AND_RETURN_VALUE(!pReport, STATUS_FAIL);

	std::set<CIpServiceResourcesInterfaceArray>::iterator _end = m_IpServicesResourcesInterface->end();
	for (std::set<CIpServiceResourcesInterfaceArray>::iterator _itr = m_IpServicesResourcesInterface->begin(); _itr != _end; ++_itr)
	{
		CIpServiceRsrcReport ipServiceRsrcReport(_itr->GetServiceName(), _itr->GetServiceId());

		CIpServiceResourcesInterfaceArray* pResourcesInterface = const_cast<CIpServiceResourcesInterfaceArray*>(&(*_itr));

		TRACEINTO << *pResourcesInterface;
		pResourcesInterface->CalculateResourceReportPerService(&ipServiceRsrcReport);

		pReport->AddIpServiceReport(ipServiceRsrcReport);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSystemResources::GetNumConfiguredArtPorts(WORD boardId)
{
	DWORD numConfiguredArtPorts = 0;

	// boardId of ALL_BOARD_IDS (0xFF) returns total art ports on all boards
	if (boardId == ALL_BOARD_IDS)
	{
		for (int i = 0; i < BOARDS_NUM; i++)
		{
			CBoard* pBoard = GetBoard(i + 1);
			if (pBoard && pBoard->GetCardsStartup(MFA_COMPLETE))
				numConfiguredArtPorts += m_num_configured_ART_Ports[i];
		}
	}
	else
	{
		CBoard* pBoard = GetBoard(boardId);
		if (pBoard && pBoard->GetCardsStartup(MFA_COMPLETE))
			numConfiguredArtPorts = m_num_configured_ART_Ports[boardId - 1];
	}

	return numConfiguredArtPorts;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSystemResources::GetNumConfiguredVideoPorts(WORD boardId)
{
	DWORD numConfiguredVideoPorts = 0;

	// boardId of ALL_BOARD_IDS (0xFF) returns total video ports on all boards
	if (boardId == ALL_BOARD_IDS)
	{
		for (int i = 0; i < BOARDS_NUM; i++)
		{
			CBoard* pBoard = GetBoard(i + 1);
			if (pBoard && pBoard->GetCardsStartup(MFA_COMPLETE))
				numConfiguredVideoPorts += m_num_configured_VIDEO_Ports[i];
		}
	}
	else
	{
		CBoard* pBoard = GetBoard(boardId);
		if (pBoard && pBoard->GetCardsStartup(MFA_COMPLETE))
			numConfiguredVideoPorts = m_num_configured_VIDEO_Ports[boardId - 1];
	}

	return numConfiguredVideoPorts;
}

////////////////////////////////////////////////////////////////////////////
//(***) ivr mount, mfa complete, resource
STATUS CSystemResources::SetMfaStartupComplete(BoardID boardId, DWORD PQ1status, DWORD PQ2status)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, STATUS_FAIL);

	if (PQ1status == STATUS_OK && PQ2status == STATUS_OK)
		pBoard->SetCardsStartup(MFA_COMPLETE, 1);
	else
		pBoard->SetCardsStartup(MFA_COMPLETE, 0);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
bool CSystemResources::IsBoardReady(BoardID boardId)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, false);

	return pBoard->GetCardsStartup(MFA_COMPLETE) ? true : false;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CompleteResourcesStartup()
{
	//(***)enable all ready boards
	for (BoardID boardId = 1; boardId <= BOARDS_NUM; ++boardId)
		if (IsBoardReady(boardId))
			SetBoardMfaStatus(boardId, TRUE);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::PartyParamsToPhysicalLocation(DWORD ConfId, DWORD PartyId, eResourceTypes typePhysical,
                                                       WORD& bId, WORD& uId, WORD& acceleratorId, WORD& portId)
{
	STATUS status = STATUS_NOT_FOUND;

	eUnitType UnitType = PhysicalToUnitType(typePhysical);

	const CActivePortsList* pActivePorts;
	CActivePortsList::iterator i;

	std::set<CUnitMFA>::iterator itr;

	CBoard* pBoard;
	CMediaUnitsList* pMediaUnitslist;

	for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		if (!IsBoardIdExists(boardId + 1))
			continue;

		pBoard = m_pBoards[boardId];
		pMediaUnitslist = pBoard->GetMediaUnitsList();

		for (itr = pMediaUnitslist->begin(); itr != pMediaUnitslist->end(); itr++)
		{
			if (itr->GetUnitType() != UnitType)
				continue;

			pActivePorts = itr->GetActivePorts();

			for (i = pActivePorts->begin(); i != pActivePorts->end(); i++)
			{
				if (ConfId == i->GetConfId() && PartyId == i->GetPartyId())
				{
					bId = itr->GetBoardId();
					uId = itr->GetUnitId();

					acceleratorId = i->GetAcceleratorId();
					portId = i->GetPortId();

					return STATUS_OK;
				}
			}
		}
	}

	if (status != STATUS_OK)
		PTRACE(eLevelInfoHigh, "Debug recording unit was not found");  //more parametrized trace?

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CreateRTMBoard(RTM_ISDN_ENTITY_LOADED_S* pRTMBoard)
{
	WORD bId        = pRTMBoard->boardId;
	WORD subBid     = pRTMBoard->subBoardId;
	WORD numOfSpans = pRTMBoard->numOfSpans;

	TRACEINTO << "BoardId:" << bId << ", SubBoardId:" << subBid << ", NumOfSpans:" << numOfSpans;

	if (RTM_ISDN_SUBBOARD_ID != subBid)
	{
		PTRACE2INT(eLevelError, " CSystemResources::CreateRTMBoard - wrong SubBoardId = ", subBid);
		return STATUS_FAIL;
	}

	CBoard* pBoard = GetBoard(bId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	std::set<CSpanRTM>::iterator itr;

	if (pSpanslist->size() != 0)
	{
		PTRACE2INT(eLevelError, " Spans already created on boardId - ", bId);
		return STATUS_FAIL;
	}

	CSpanRTM* pSpan = NULL;

	for (int i = 1; i < numOfSpans + 1; i++)
	{
		pSpan = new CSpanRTM(bId, i);

		pSpan->SetSubBoardId(subBid);

		pSpanslist->insert(*pSpan);

		POBJDELETE(pSpan);
	}

	pBoard->InitializeChannelsIdsList(subBid);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::ConfigureRTMService(RTM_ISDN_PARAMS_MCMS_S* pRTMService)
{
	PASSERT_AND_RETURN_VALUE(!pRTMService, STATUS_FAIL);

	int i;
	char serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN + 1];
	for (i = 0; i < RTM_ISDN_SERVICE_PROVIDER_NAME_LEN; i++)
		serviceName[i] = pRTMService->serviceName[i];
	serviceName[i] = '\0';

	const CNetServiceRsrcs* pExistService = findServiceByName(serviceName);
	TRACECOND_AND_RETURN_VALUE(pExistService, "ServiceName:" << serviceName << " - Failed, service already exist", STATUS_FAIL);

	CNetServiceRsrcs* pService = new CNetServiceRsrcs(serviceName);
	WORD type = pRTMService->spanDef.spanType;

	if (eSpanTypeT1 == type)
		pService->SetSpanType(TYPE_SPAN_T1);
	else if (eSpanTypeE1 == type)
		pService->SetSpanType(TYPE_SPAN_E1);
	else
		PASSERT(1);

	for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
	{
		pService->m_ipAddressesList[i].boardId            = pRTMService->ipAddressesList[i].boardId;
		pService->m_ipAddressesList[i].ipAdressRTMV4.ip   = pRTMService->ipAddressesList[i].ipAddress_Rtm;
		pService->m_ipAddressesList[i].ipAdressMediaV4.ip = pRTMService->ipAddressesList[i].ipAddress_RtmMedia;
	}

	// Enable all service phones from all ranges
	DWORD first, last;
	DWORD num;

	char s_phone[PHONE_NUMBER_DIGITS_LEN];
	char strPrefixFirst[PHONE_NUMBER_DIGITS_LEN];
	char strPrefixLast[PHONE_NUMBER_DIGITS_LEN];
	memset(s_phone, '\0', PHONE_NUMBER_DIGITS_LEN - 1);
	int suffixLengthFirst, suffixLengthLast;

	for (i = 0; i < MAX_ISDN_PHONE_NUMBER_IN_SERVICE; i++)
	{
		RTM_ISDN_PHONE_RANGE_S& range = pRTMService->phoneRangesList[i];

		if (range.firstPhoneNumber[0] != '\0')
		{
			PhoneHelper::CutPhone((char*)(&range.firstPhoneNumber), strPrefixFirst, first, suffixLengthFirst);
			PhoneHelper::CutPhone((char*)(&range.lastPhoneNumber), strPrefixLast, last, suffixLengthLast);

			TRACEINTO
			<< "\n  Prefix            :" << strPrefixFirst
			<< "\n  First             :" << first
			<< "\n  last              :" << last
			<< "\n  FirstPhoneNumber  :" << (char*)(&(range.firstPhoneNumber))
			<< "\n  LastPhoneNumber   :" << (char*)(&(range.lastPhoneNumber))
			<< "\n  SuffixLengthFirst :" << suffixLengthFirst
			<< "\n  SuffixLengthLast  :" << suffixLengthLast;

			if (strcmp(strPrefixFirst, strPrefixLast) != 0)
			{
				PASSERTMSG(1, "Prefixes are not equal");
				continue;
			}
			if (suffixLengthFirst != suffixLengthLast)
			{
				PASSERTMSG(1, "Prefix lengths are not equal");
				continue;
			}

			//if range not empty ???
			for (num = first; num <= last; num++)
			{
				PhoneHelper::GluePhone(s_phone, strPrefixFirst, num, suffixLengthFirst);
				pService->EnablePhone(s_phone);
			}
		}
	}

	m_pNetServicesDB->GetNetServices()->insert(*pService);
	POBJDELETE(pService);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::TestIfServiceIsRemovable(const char* serviceName, CNetServiceRsrcs*& pService)
{
	STATUS status = STATUS_OK;

	CNetServiceRsrcs* pServiceFound = (CNetServiceRsrcs*)findServiceByName((char*)serviceName);

	if (NULL == pServiceFound) //service already exists
	{
		PTRACE(eLevelInfoNormal, "Service Doesn't Exist");
		return STATUS_FAIL;
	}

	// 1.	Test if can delete the service's spans
	std::set<CSpanRTM>::iterator itrSpan;

	CBoard* pBoard;
	CSpansList* pSpanslist;

	for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		pBoard = m_pBoards[boardId];
		pSpanslist = pBoard->GetSpansRTMlist();

		for (itrSpan = pSpanslist->begin(); STATUS_OK == status && itrSpan != pSpanslist->end(); itrSpan++)
		{
			const char* spanServiceName = ((CSpanRTM*)(&(*itrSpan)))->GetSpanServiceName();
			if (!strcmp(spanServiceName, serviceName))
			{
				status = TestIfSpanIsRemovable((CSpanRTM*)(&(*itrSpan)));
			}
		}
	}

	// 2.	Test if can delete the service's phones
	if (STATUS_OK == status)
	{
		std::set<CPhone>* pPhoneList = pServiceFound->GetPhonesList();

		if (NULL == pPhoneList)
		{
			PTRACE(eLevelInfoNormal, "CSystemResources::TestIfServiceIsRemovable - pServiceFound->m_pPhoneslist is NULL");
			return STATUS_FAIL;
		}

		std::set<CPhone>::iterator itrPhone;

		for (itrPhone = pPhoneList->begin(); STATUS_OK == status && itrPhone != pPhoneList->end(); itrPhone++)
		{
			status = TestIfPhoneNumberIsRemovable((char*)itrPhone->GetNumber());
		}
	}

	pService = pServiceFound;               // pService is being returned in order to save another pass of the list

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeleteRTMService(RTM_ISDN_SERVICE_CANCEL_S* pRTMServiceToDelete)
{
	STATUS status = STATUS_OK;

	int i;
	char serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN + 1];
	for (i = 0; i < RTM_ISDN_SERVICE_PROVIDER_NAME_LEN; i++)
		serviceName[i] = pRTMServiceToDelete->serviceName[i];
	serviceName[i] = '\0';

	// 1. Check if service can be deleted
	CNetServiceRsrcs* pService = NULL;
	status = TestIfServiceIsRemovable(serviceName, pService);

	if (STATUS_OK == status)
	{
		// 2. Delete the service's spans

		std::set<CSpanRTM>::iterator itrSpan;
		CBoard* pBoard;
		CSpansList* pSpanslist;

		for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
		{
			pBoard = m_pBoards[boardId];
			pSpanslist = pBoard->GetSpansRTMlist();

			for (itrSpan = pSpanslist->begin(); itrSpan != pSpanslist->end(); itrSpan++)
			{
				const char* spanServiceName = ((CSpanRTM*)(&(*itrSpan)))->GetSpanServiceName();

				if (!strcmp(spanServiceName, serviceName))
					((CSpanRTM*)(&(*itrSpan)))->SetNullConfiguration();
			}
		}

		// 3. Delete the service's phone ranges (not really needed since the phone list is inside the service which is being deleted ??? )

		// 4. Delete the service
		m_pNetServicesDB->GetNetServices()->erase(*pService);
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::AddRTMPhoneNumberRange(RTM_ISDN_PHONE_RANGE_UPDATE_S* pRTMPhoneNumberRange)
{
	if (NULL == pRTMPhoneNumberRange)
	{
		PTRACE(eLevelInfoNormal, "CSystemResources::AddRTMPhoneNumberRange - pRTMPhoneNumberRange is NULL");
		return STATUS_FAIL;
	}

	CNetServiceRsrcs* pService = (CNetServiceRsrcs*)findServiceByName((char*)pRTMPhoneNumberRange->serviceName);

	if (!pService) //service doesn't exists
	{
		TRACEINTO << "\n CSystemResources::AddRTMPhoneNumberRange - Service Doesn't Exists : " << pRTMPhoneNumberRange->serviceName << "\n\n";
		return STATUS_FAIL;
	}

	RTM_ISDN_PHONE_RANGE_S range = pRTMPhoneNumberRange->phoneRange;

	DWORD first, last;
	DWORD num;

	char s_phone[PHONE_NUMBER_DIGITS_LEN];
	char strPrefixFirst[PHONE_NUMBER_DIGITS_LEN];
	char strPrefixLast[PHONE_NUMBER_DIGITS_LEN];
	memset(s_phone, '\0', PHONE_NUMBER_DIGITS_LEN - 1);
	int suffixLengthFirst, suffixLengthLast;

	if (range.firstPhoneNumber[0] != '\0')
	{
		PhoneHelper::CutPhone((char*)(&(range.firstPhoneNumber)), strPrefixFirst, first, suffixLengthFirst);
		PhoneHelper::CutPhone((char*)(&(range.lastPhoneNumber)), strPrefixLast, last, suffixLengthLast);
		TRACEINTO << "\nPrefix: " << strPrefixFirst << " first =  " << first << " last = " << last
		          << " range.firstPhoneNumber: " << (char*)(&(range.firstPhoneNumber)) << " range.lastPhoneNumber =  " << (char*)(&(range.lastPhoneNumber))
		          << " suffixLengthFirst: " << suffixLengthFirst << " suffixLengthLast: " << suffixLengthLast << "\n\n";
		if (strcmp(strPrefixFirst, strPrefixLast) != 0)
		{
			PASSERTMSG(1, "Prefixes are not equal");
			return STATUS_FAIL;
		}
		if (suffixLengthFirst != suffixLengthLast)
		{
			PASSERTMSG(1, "Prefixe lengths are not equal");
			return STATUS_FAIL;
		}

		//if range not empty ???
		for (num = first; num <= last; num++)
		{
			PhoneHelper::GluePhone(s_phone, strPrefixFirst, num, suffixLengthFirst);

			// We insert the phones "on the fly" and if the phone already exists (means that the range is overlapping)
			// than we roll-back and delete the numbers already inserted (in most cases we won't get to the roll-back, since
			// there is a check for overlapping in the upper level [Rtm Manager])
			if (STATUS_ALREADY_EXISTS == pService->EnablePhone(s_phone))
			{
				for (DWORD i = first; i < num; i++)
				{
					PhoneHelper::GluePhone(s_phone, strPrefixFirst, i, suffixLengthFirst);
					pService->DisablePhone( s_phone );
				}
				break;
			}
		}
	}

	m_pNetServicesDB->GetNetServices()->insert(*pService);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::TestIfPhoneNumberIsRemovable(char* s_phone)
{
	SleepingConferences* pSleepingConferences = CHelperFuncs::GetSleepingConferences();
	PASSERT_AND_RETURN_VALUE(!pSleepingConferences, STATUS_FAIL);

	STATUS status = STATUS_OK;

	SleepingConferences::iterator itrConf;
	eRsrcConfType c_type = eConf_type_none;

	for (itrConf = pSleepingConferences->begin(); STATUS_OK == status && itrConf != pSleepingConferences->end(); itrConf++)
	{
		c_type = itrConf->GetConfType();
		if (eMR_type == c_type || eEQ_type == c_type)
		{
			CServicePhoneStr* pPhoneStr = ((CSleepingConference*)(&(*itrConf)))->GetFirstServicePhone();
			while (pPhoneStr != NULL)                                      //FYI: there is no checking of service name!!!
			{
				if (pPhoneStr->FindPhoneNumber(s_phone) != NOT_FIND) // Phone Number exists in EQ
				{
					status = STATUS_PHONE_NUMBER_CONFIGURED_IN_EQ;
					TRACEINTO << "PhoneNumber:" << s_phone << " exists in EQ with monitor_id = " << itrConf->GetMonitorConfId();
					break;
				}
				pPhoneStr = ((CSleepingConference*)(&(*itrConf)))->GetNextServicePhone();
			}
		}
	}
	if (STATUS_OK != status)
		return status;

	ReservedConferences* pConfRsrvRsrc = CHelperFuncs::GetConfRsrvRsrcs();
	PASSERT_AND_RETURN_VALUE(!pConfRsrvRsrc, STATUS_FAIL);

	ReservedConferences::iterator confRsrvIterator;
	BOOL isFromPassive = FALSE;
	for (confRsrvIterator = pConfRsrvRsrc->begin(); confRsrvIterator != pConfRsrvRsrc->end(); confRsrvIterator++)
	{
		isFromPassive = confRsrvIterator->GetFromPassive();
		if (isFromPassive)
			continue;
		CServicePhoneStr* pPhoneStr = ((CConfRsrvRsrc*)(&(*confRsrvIterator)))->GetFirstServicePhone();
		while (pPhoneStr != NULL)                                      //FYI: there is no checking of service name!!!
		{
			if (pPhoneStr->FindPhoneNumber(s_phone) != NOT_FIND) // Phone Number exists in regular conf
			{
				status = STATUS_PHONE_NUMBER_CONFIGURED_IN_EQ; // TODO: new status
				TRACEINTO << "PhoneNumber:" << s_phone << " exists in regular conf with monitor_id = " << confRsrvIterator->GetMonitorConfId();
				break;
			}
			pPhoneStr = ((CConfRsrvRsrc*)(&(*confRsrvIterator)))->GetNextServicePhone();
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::TestIfPhoneNumberRangeIsRemovable(char* strPrefix, DWORD first, DWORD last, int suffixLength)
{
	// The function checks if there exists any number of the given range
	// which is included in any EQ
	char s_phone[PHONE_NUMBER_DIGITS_LEN];

	STATUS status = STATUS_OK;
	DWORD num;

	for (num = first; STATUS_OK == status && num <= last; num++)
	{
		PhoneHelper::GluePhone(s_phone, strPrefix, num, suffixLength);
		status = TestIfPhoneNumberIsRemovable(s_phone);
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeleteRTMPhoneNumberRange(RTM_ISDN_PHONE_RANGE_UPDATE_S* pRTMPhoneNumberRange)
{
	STATUS status = STATUS_OK;

	if (NULL == pRTMPhoneNumberRange)
	{
		PTRACE(eLevelInfoNormal, "CSystemResources::DeleteRTMPhoneNumberRange - pRTMPhoneNumberRange is NULL");
		return STATUS_FAIL;
	}

	CNetServiceRsrcs* pService = ( CNetServiceRsrcs* )findServiceByName((char*)pRTMPhoneNumberRange->serviceName);

	if (!pService) //service doesn't exists
	{
		TRACEINTO << "\n CSystemResources::DeleteRTMPhoneNumberRange - Service Doesn't Exists : " << pRTMPhoneNumberRange->serviceName << "\n\n";
		return STATUS_FAIL;
	}

	RTM_ISDN_PHONE_RANGE_S range = pRTMPhoneNumberRange->phoneRange;

	DWORD first, last;
	DWORD num;

	char s_phone[PHONE_NUMBER_DIGITS_LEN];
	char strPrefixFirst[PHONE_NUMBER_DIGITS_LEN];
	char strPrefixLast[PHONE_NUMBER_DIGITS_LEN];
	memset(s_phone, '\0', PHONE_NUMBER_DIGITS_LEN - 1);
	int suffixLengthFirst, suffixLengthLast;

	if (range.firstPhoneNumber[0] != '\0')
	{
		PhoneHelper::CutPhone((char*)(&(range.firstPhoneNumber)), strPrefixFirst, first, suffixLengthFirst);
		PhoneHelper::CutPhone((char*)(&(range.lastPhoneNumber)), strPrefixLast, last, suffixLengthLast);
		TRACEINTO << "\nPrefix: " << strPrefixFirst << " first =  " << first << " last = " << last
		          << " range.firstPhoneNumber: " << (char*)(&(range.firstPhoneNumber)) << " range.lastPhoneNumber =  " << (char*)(&(range.lastPhoneNumber))
		          << " suffixLengthFirst: " << suffixLengthFirst << " suffixLengthLast: " << suffixLengthLast << "\n\n";
		if (strcmp(strPrefixFirst, strPrefixLast) != 0)
		{
			PASSERTMSG(1, "Prefixes are not equal");
			return STATUS_FAIL;
		}
		if (suffixLengthFirst != suffixLengthLast)
		{
			PASSERTMSG(1, "Prefix lengths are not equal");
			return STATUS_FAIL;
		}

		status = TestIfPhoneNumberRangeIsRemovable(strPrefixFirst, first, last, suffixLengthFirst);

		if (STATUS_OK == status)
		{
			for (num = first; num <= last; num++)
			{
				PhoneHelper::GluePhone(s_phone, strPrefixFirst, num, suffixLengthFirst);

				pService->DisablePhone(s_phone);
			}
			m_pNetServicesDB->GetNetServices()->insert(*pService);
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::ConfigureRTMSpan(SPAN_ENABLED_S* pRTMSpan)
{
	const CNetServiceRsrcs* pExistService = findServiceByName((char*)(pRTMSpan->serviceName));

	if (!pExistService) //service has to exist
	{
		PTRACE(eLevelInfoNormal, "Service Not Exist");
		PASSERT(1);
		return STATUS_FAIL;
	}

	eRTMSpanType type = ((CNetServiceRsrcs*)(pExistService))->GetSpanType();
	//
	WORD bId = pRTMSpan->boardId;
	WORD uId = pRTMSpan->spanId;

	CBoard* pBoard = GetBoard(bId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	std::set<CSpanRTM>::iterator itr;

	for (itr = pSpanslist->begin(); itr != pSpanslist->end(); itr++)
		if (itr->GetUnitId() == uId)
			break;

	if (itr == pSpanslist->end()) //span has to exist
	{
		PTRACE(eLevelError, " CSystemResources::ConfigureRTMSpan : Span not exist ");
		return STATUS_SPAN_NOT_EXISTS;
	}

	TRACEINTO << "ServiceName " << (char*)(pRTMSpan->serviceName);

	const char* name = ((CSpanRTM*)(&(*itr)))->GetSpanServiceName();
	if (name[0] == '\0')
	{
		((CSpanRTM*)(&(*itr)))->ConfigureServiceOnSpan((char*)(pRTMSpan->serviceName), type, pRTMSpan->spanEnabledStatus);
		m_pNetServicesDB->SpanConfiguredOnService((CSpanRTM*)(&(*itr)));
	}
	else
	{
		if (strcmp((char*)(pRTMSpan->serviceName), name) != 0)
		{
			PTRACE(eLevelError, "CSystemResources::ConfigureRTMSpan : Span configured within another service");
			return STATUS_FAIL;
		}
		else
			//only set enabled status
			((CSpanRTM*)(&(*itr)))->SetEnabled(pRTMSpan->spanEnabledStatus);
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DisableAllRtmSpanPerBoard(RTM_ISDN_BOARD_ID_S* pIsdnBId)
{
	std::set<CSpanRTM> ::iterator itr;

	CBoard* pBoard = GetBoard(pIsdnBId->boardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	for (itr = pSpanslist->begin(); itr != pSpanslist->end(); itr++)
	{
		if (itr->GetSubBoardId() == pIsdnBId->subBoardId)
		{
			((CSpanRTM*)(&(*itr)))->SetEnabled(FALSE);
		}
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::ChangeSpanToNullConfigure( SPAN_DISABLE_S* pSpan )
{
	////// =========== note:
	// if the following status is changed - the following methos should be updated:
	//                         CRtmIsdnMngrManager::SendMsgSyncDisableSpanMapToResourceProcess
	STATUS status = STATUS_SPAN_NOT_EXISTS;
	////// ===========

	CSpanRTM* pSpanFound = NULL;

	status = TestIfSpanIsRemovable( pSpan->boardId, pSpan->spanId, pSpanFound );

	if (STATUS_OK == status)
	{
		m_pNetServicesDB->SpanRemoved(pSpanFound);
		pSpanFound->SetNullConfiguration(); //change span's configure to NULL
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::TestIfSpanIsRemovable ( WORD bId, WORD uId, CSpanRTM*& pSpanFound )
{
	STATUS status = STATUS_SPAN_NOT_EXISTS;

	std::set<CSpanRTM> ::iterator itr;

	CBoard* pBoard = GetBoard(bId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_SPAN_NOT_EXISTS);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	for (itr = pSpanslist->begin(); itr != pSpanslist->end(); itr++)
	{
		if (itr->GetBoardId() == bId && itr->GetUnitId() == uId)
		{
			pSpanFound = (CSpanRTM*)(&*itr);
			return TestIfSpanIsRemovable(pSpanFound);
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::TestIfSpanIsRemovable (CSpanRTM* pSpan)
{
	//Note: meanwhile we don't check that after removing the span,
	//there are still enough RTM's left also for the dial-in reserved ports (on service level)
	if (pSpan->GetNumPorts() == pSpan->GetNumFreePorts()
	    && pSpan->GetNumDialOutReservedPorts() == 0)      //span's free
	{
		return STATUS_OK;
	}
	else
		return STATUS_SPAN_OCCUPIED;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::AllocateServicePhones(CServicePhoneStr& phoneStr, const char* pSimilarToThisString)
{
	STATUS status = STATUS_OK;

	char* service_name = (char*)(phoneStr.GetNetServiceName());

	CNetServiceRsrcs* pService = ( CNetServiceRsrcs* )(findServiceByName(service_name));
	if (!pService)
	{
		PTRACE(eLevelError, "CSystemResources::AllocateServicePhones - Service not found ");
		TRACEINTO << "\n ServiceName: " << service_name;
		return STATUS_NET_SERVICE_NOT_FOUND;
	}

	Phone* phone = phoneStr.GetFirstPhoneNumber();
	WORD count = 0;
	BOOL is_Non_spec = (NULL == phone) ? TRUE : FALSE;

	// SPECIFIC CASE
	if (phone != NULL)
	{
		BYTE isUseAsRange = phoneStr.IsUseServicePhonesAsRange();
		if (isUseAsRange)
		{
			Phone* phone2 = phoneStr.GetNextPhoneNumber();
			ALLOCBUFFER(currPhoneNumber, PHONE_NUMBER_DIGITS_LEN);
			ULONGLONG phone1Num = atoll(phone->phone_number);
			DWORD numPhones = phone2 ? (atoll(phone2->phone_number) - phone1Num) : 0;
			TRACEINTO << "use Service Phones As Range - phone1 = "
			          << phone->phone_number << ", phone2 = " << (phone2 ? phone2->phone_number : "- ") << ", numPhones = " << numPhones;

			for (DWORD i = 0; i < numPhones + 1; i++)
			{
				int currPhone = phone1Num + i;
				status = pService->CapturePhone(currPhone); // currPhoneNumber );
				if (status != STATUS_OK)
				{
					for (WORD j = 0; j < i; j++)        //roll-back
					{
						ULONGLONG currPhone_j = phone1Num + j;
						memset(currPhoneNumber, 0, PHONE_NUMBER_DIGITS_LEN);
						sprintf(currPhoneNumber, "%llu", currPhone_j);

						pService->DeAlocatePhone( currPhoneNumber );
					}
					break; //exit prime loop
				}
			}
			DEALLOCBUFFER(currPhoneNumber);
		}
		else
		{
			while (phone != NULL)
			{
				status = pService->CapturePhone(phone->phone_number);

				if (status != STATUS_OK)
				{
					PTRACE(eLevelError, "CSystemResources::AllocateServicePhones - Allocation failed! ");
					TRACEINTO << "\n Phone number: " << phone->phone_number;

					//roll-back
					phone = phoneStr.GetFirstPhoneNumber();

					while (count > 0)
					{
						pService->DeAlocatePhone(phone->phone_number);
						phone = phoneStr.GetNextPhoneNumber();
						count--;
					}
					//roll-back end.

					break; //exit prime loop
				}
				phone = phoneStr.GetNextPhoneNumber();
				count++;       //for roll-back
			}
		}
	}
	// NON SPECIFIC CASE
	if (TRUE == is_Non_spec)
	{
		char* num = NULL;

		status = pService->AllocatePhone( num, pSimilarToThisString );
		if (STATUS_OK == status)
		{
			status = phoneStr.AddPhoneNumber((char*)(num));
			//if status !=ok - rollback)
		}
		if (NULL != num)
			delete [] num;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeAllocateServicePhones(CServicePhoneStr& phoneStr)
{
	const char* serviceName = phoneStr.GetNetServiceName();

	CNetServiceRsrcs* pService = const_cast<CNetServiceRsrcs*>(findServiceByName(serviceName));
	TRACECOND_AND_RETURN_VALUE(!pService, "ServiceName:" << serviceName << " - Service not found", STATUS_NET_SERVICE_NOT_FOUND);

	bool isUseAsRange = phoneStr.IsUseServicePhonesAsRange();
	if (isUseAsRange)
	{
		Phone* phone1 = phoneStr.GetFirstPhoneNumber();
		Phone* phone2 = phoneStr.GetNextPhoneNumber();
		if (phone1)
		{
			char phoneNumber[PHONE_NUMBER_DIGITS_LEN];
			ULONGLONG phone1Num = atoll(phone1->phone_number);
			DWORD numPhones = phone2 ? (atoll(phone2->phone_number) - phone1Num) : 0;

			TRACEINTO << "ServiceName:" << serviceName << ", Phone1:" << phone1->phone_number << ", Phone2:" << (phone2 ? phone2->phone_number : "-") << ", NumOfPhones:" << numPhones;

			// Klocwork produces an issue report at loop indicating that unvalidated integer 'numPhones' received through a call to 'atoll'
			// can be used in a loop condition. In this case, potentially tainted data is used as a loop boundary, which could be exploited
			// by a malicious user. So, check i less 1000
			for (DWORD i = 0; i < numPhones + 1 && i < 1000; i++)
			{
				ULONGLONG currPhone = phone1Num + i;
				memset(phoneNumber, 0, sizeof(phoneNumber));
				sprintf(phoneNumber, "%llu", currPhone);

				pService->DeAlocatePhone(phoneNumber);
			}
		}
	}
	else
	{
		Phone* phone = phoneStr.GetFirstPhoneNumber();

		while (phone != NULL)
		{
			pService->DeAlocatePhone(phone->phone_number);
			phone = phoneStr.GetNextPhoneNumber();
		}
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
int CSystemResources::ConfigureResourcesNew( WORD num_units,
                                             RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S* pConfigParamsList,
                                             CM_UNITS_CONFIG_S* pRetConfigParams )
{
	STATUS status;
	BYTE isCntler = FALSE;

	eCardType card_type = (eCardType)pConfigParamsList->cardType;
	RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams = pConfigParamsList->unitsConfigParamsList;
	WORD bId, uId;
	bId = pConfigParams[0].physicalHeader.board_id; //***temp???

	CBoard* pBoard = GetBoard(bId);
	if (pBoard != NULL)
	{
		pBoard->SetCardType(card_type);

		//at this point we should have received the m_SlotNumberingConversionTable, so update it
		WORD displayBoardId = GetDisplayBoardId(bId, MEDIA_SUB_BOARD_NUM);
		if (displayBoardId == NO_DISPLAY_BOARD_ID)
			PTRACE(eLevelError, "Display Board Id not found in table ");
		// PASSERTMSG(1, "Display Board Id not found in table");
		else
			pBoard->SetDisplayBoardId(displayBoardId);
	}
	else
		PASSERT(1);

	if (CHelperFuncs::IsBreeze(card_type) && m_ResourceAllocationType != eFixedBreezeMode && m_ResourceAllocationType != eAutoBreezeMode && m_ResourceAllocationType != eAutoMixedMode)
	{
		PTRACE(eLevelError, "CSystemResources::ConfigureResources - Trying to configure Breeze card not in fixed/auto breeze mode");
		PASSERT_AND_RETURN_VALUE(1, STATUS_FAIL);
	}
	if (CHelperFuncs::IsMpmRxOrNinja(card_type) && m_ResourceAllocationType != eFixedMpmRxMode && m_ResourceAllocationType != eAutoMpmRxMode && m_ResourceAllocationType != eAutoMixedMode)
	{
		PTRACE(eLevelError, "CSystemResources::ConfigureResources - Trying to configure MPM-Rx card not in fixed/auto MPM-Rx mode");
		PASSERT_AND_RETURN_VALUE(1, STATUS_FAIL);
	}

	int i = 0;

	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		pRetConfigParams->unitsParamsList[i].pqNumber = 0xff;
		pRetConfigParams->unitsParamsList[i].type = eNotConfigured;
	}

	CCardResourceConfig* cardResourceConfig = NULL;
	if (CHelperFuncs::IsSoftCard((eCardType)card_type))
		cardResourceConfig = new CCardResourceConfigSoft((eCardType)card_type);
	else if (CHelperFuncs::IsBreeze((eCardType)card_type))
		cardResourceConfig = new CCardResourceConfigBreeze((eCardType)card_type);
	else if (CHelperFuncs::IsMpmRx((eCardType)card_type))
		cardResourceConfig = new CCardResourceConfigMpmRx((eCardType)card_type);
	else if (eMpmRx_Ninja == card_type)
		cardResourceConfig = new CCardResourceConfigNinja((eCardType)card_type);

	if (NULL == cardResourceConfig)
	{
		PASSERTMSG(1, "cardResourceConfig is NULL");
		return STATUS_FAIL;
	}

	cardResourceConfig->Config(pConfigParams, pRetConfigParams, card_type);

	if (CHelperFuncs::IsValidBoardId(bId))
	{
		m_num_configured_ART_Ports[bId - 1] += cardResourceConfig->GetNumConfiguredARTPorts();
		m_num_configured_VIDEO_Ports[bId - 1] += cardResourceConfig->GetNumConfiguredVideoPorts();
	}

	POBJDELETE(cardResourceConfig);

	///////////////// creating controllers
	// Update the list of utilizable PQ units per board
	for (i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		if ((ePQ == pConfigParams[i].unitType) && (eNotExist != pConfigParams[i].status)
		    && (eUnitStartup != pConfigParams[i].status)
		    && (eUnknown != pConfigParams[i].status)
		    && (eUnitStartup != pConfigParams[i].status)
		    && (eUnitStartup != pConfigParams[i].status))
		{
			CBoard* pBoard = GetBoard(pConfigParams[i].physicalHeader.board_id);
			if (pBoard != NULL)
				pBoard->SetUtilizablePQUnitId(i);
			else
				PASSERT(1);
		}
	}

	// Set units DISABLED - is board ready being checked later , from calling function

	// !!! suppose , as now - the request is per 1 boardId !!! If not, units should be put into "disabled" state one by one!
	if (IsBoardReady(bId) != TRUE)
		SetBoardMfaStatus(bId, FALSE);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
BYTE CSystemResources::IsSameUnitOnPCI(CM_UNITS_CONFIG_S* pRetConfigParams, WORD i, eCardUnitTypeConfigured unitType)
{
	BYTE ret_status = TRUE;
	if (i >= (MAX_NUM_OF_UNITS - 1) || i == 0)
	{
		PASSERTSTREAM(true, "Unit " << i << " is cancelled");
		ret_status = FALSE;
	}
	if (ret_status)
	{
		//for unit in position 0 - pair is in position 1, and vise versa, e.t.c.
		WORD idx_pair = (i % 2 != 0) ? i + 1 : i - 1;
		DWORD config_type = pRetConfigParams->unitsParamsList[idx_pair].type;
		if (eVideo == unitType)
		{
			if (config_type == (DWORD)unitType) //pair already configured as Video/Art
				ret_status = TRUE;
			else
				ret_status = FALSE;
		}
		else if (eArt == unitType || eArtCntlr == unitType)
		{
			if (config_type == (DWORD)unitType)
				ret_status = TRUE;
			else
				ret_status = FALSE;
		}
		else
		{
			ret_status = FALSE;
			PASSERT(2);
		}
	}

	return ret_status;
}

////////////////////////////////////////////////////////////////////////////
BYTE CSystemResources::IsArtOnSamePCI(WORD boardId, WORD unitId)
{
	BYTE ret_status = TRUE;
	if (unitId >= MAX_NUM_OF_UNITS || unitId == 0)
	{
		PASSERT(1);
		ret_status = FALSE;
	}
	if (TRUE == ret_status)
	{
		WORD idx_pair = (unitId % 2 != 0) ? unitId + 1 : unitId - 1; //for unit in position 0 - pair is in position 1 , and vise versa, e.t.c.

		CBoard* pBoard = GetBoard(boardId);
		PASSERT_AND_RETURN_VALUE(pBoard == NULL, FALSE);

		CUnitMFA* pExistUnitMFA = GetUnit(boardId, idx_pair);

		if (NULL != pExistUnitMFA && (eUnitType_Art == pExistUnitMFA->GetUnitType() || eUnitType_Art_Control == pExistUnitMFA->GetUnitType()))
		{
			ret_status = TRUE;
		}
		else
		{
			ret_status = FALSE; //NOT_FIND
		}
	}

	return ret_status;
}

////////////////////////////////////////////////////////////////////////////
BYTE CSystemResources::IsTheSamePCI(WORD unitId1, WORD unitId2)
{
	BYTE ret_status = FALSE;
	if (unitId1 != 0)
	{
		WORD idx_pair = (unitId1 % 2 != 0) ? unitId1 + 1 : unitId1 - 1;
		ret_status = (idx_pair == unitId2) ? TRUE : FALSE;
	}

	return ret_status;
}

////////////////////////////////////////////////////////////////////////////
BYTE CSystemResources::IsNeighbOnPCIEmpty(WORD boardId, WORD unitId)
{
	BYTE ret_status = FALSE;
	if (unitId != 0)
	{
		WORD idx_pair = (unitId % 2 != 0) ? unitId + 1 : unitId - 1;
		CBoard* pBoard = GetBoard(boardId);
		PASSERT_AND_RETURN_VALUE(pBoard == NULL, ret_status);
		CUnitMFA* pMFA = (CUnitMFA*)pBoard->GetMFA(idx_pair);
		if (pMFA)
		{
			if (1000 == pMFA->GetFreeCapacity())
				ret_status = TRUE;
		}
	}

	return ret_status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetRemainingArtOnBoard(WORD boardId, WORD neededArtChannels, int& num_parties,
                                                DWORD artCapacity, PartyDataStruct& partyData, WORD tipNumOfScreens /* = 0*/, bool neededArtChannelsPerUnit /* =true */)
{
	WORD num_free_parties_art = 0, num_free_art_channels = 0;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_INSUFFICIENT_ART_RSRC);
	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	std::set<CUnitMFA>::iterator i;

	BOOL isSvcOnly = (partyData.m_confMediaType == eSvcOnly) ? TRUE : FALSE;
	BOOL isAudioOnly = (partyData.m_videoPartyType == eVideo_party_type_none) ? TRUE : FALSE;

	float artPromilles = pBoard->CalculateNeededPromilles(PORT_ART, artCapacity, isSvcOnly, isAudioOnly, partyData.m_videoPartyType);
	float videoPromilles = pBoard->CalculateNeededPromilles(PORT_VIDEO, artCapacity);
	PASSERT_AND_RETURN_VALUE(artPromilles == 0, STATUS_INSUFFICIENT_ART_RSRC);
	PASSERT_AND_RETURN_VALUE(videoPromilles == 0, STATUS_INSUFFICIENT_ART_RSRC);
	BOOL thereIsAtLeastOneUnitWithEnoughARTChannels = FALSE;
	WORD num_free_parties_art_on_unit = 0;
	BOOL bCanAllocateTipPartyOnBoard = FALSE;
	BOOL isMpmRx = CHelperFuncs::IsMpmRx(pBoard->GetCardType());
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	BYTE bRestrictArtAllocation = YES;
	BOOL isTrafficShapingEnabled = NO;

	// If traffic shaping is enabled in the system, art must be restricted to 100%
	std::string key = CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING;
	sysConfig->GetBOOLDataByKey(key, isTrafficShapingEnabled);
	if (!isTrafficShapingEnabled)
	{
		sysConfig->GetBOOLDataByKey("RESTRICT_ART_ALLOCATION_ACCORDING_TO_WEIGHT", bRestrictArtAllocation);
	}

	WORD maxNumOfTipScreensPerArt = 0;

	if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART_SOFTMCU;
	}
	else
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART;
	}

	for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
	{
		if (i->GetIsEnabled() != TRUE)
			continue;

		if (eUnitType_Art == i->GetUnitType() || eUnitType_Art_Control == i->GetUnitType())
		{
			if (!isMpmRx || (isMpmRx && (YES == bRestrictArtAllocation)))
			{
				num_free_parties_art_on_unit = (WORD)(i->GetFreeCapacity() / artPromilles);
			}
			//For MPM-Rx, in case RESTRICT_ART_ALLOCATION_ACCORDING_TO_WEIGHT is NO,
			//still allocation on that board even weight exceed the maximum capacity of the board.
			else if (isMpmRx && (NO == bRestrictArtAllocation))
			{
				num_free_parties_art_on_unit = 1000 / ART_PROMILLES_AUDIO_OR_SVC_ONLY_MPMRX - i->GetPortNumber();

				if (num_free_parties_art_on_unit <= 0)
					continue;
			}

			num_free_parties_art += num_free_parties_art_on_unit;
			if (num_free_parties_art_on_unit > 0)
			{
				if (i->GetFreeARTChannels() >= neededArtChannels)
					thereIsAtLeastOneUnitWithEnoughARTChannels = TRUE;

				num_free_art_channels += i->GetFreeARTChannels();
			}
			if (i->GetNumAllocatedTipScreens() + tipNumOfScreens <= maxNumOfTipScreensPerArt)
				bCanAllocateTipPartyOnBoard = TRUE;
		}
	}

	num_parties = num_free_parties_art;

	if (num_free_parties_art == 0) //no free resources
	{
		TRACEINTO << "BoardId: " << boardId << ", num_free_parties_art == 0 ";
		return STATUS_INSUFFICIENT_ART_RSRC;
	}

	if (thereIsAtLeastOneUnitWithEnoughARTChannels == FALSE && (neededArtChannelsPerUnit || num_free_art_channels < neededArtChannels)) //no free ART channels resources
	{
		TRACEINTO << "BoardId: " << boardId << ", thereIsAtLeastOneUnitWithEnoughARTChannels == FALSE " << ", num_free_art_channels=" << num_free_art_channels;
		return STATUS_INSUFFICIENT_ART_CHANNELS_RSRC;
	}

	if (tipNumOfScreens > 0 && !bCanAllocateTipPartyOnBoard)
	{
		TRACEINTO << "BoardId: " << boardId << ", bCanAllocateTipPartyOnBoard == FALSE, tipNumOfScreens=" << tipNumOfScreens << "\n";
		return STATUS_INSUFFICIENT_ART_RSRC;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::AllocateChannelId(BoardID boardId)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, 0);

	WORD allocatedChannelId = 0;

	if (pBoard->GetRTMChannelIds())
		allocatedChannelId = pBoard->GetRTMChannelIds()->Allocate();

	allocatedChannelId += (boardId - 1) * MAX_CHANNEL_IDS; //we want each board to have a separate range

	return allocatedChannelId;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeAllocateChannelId(BoardID boardId, ChannelID channelId)
{
	TRACEINTO << "ChannelId:" << channelId << ", BoardId:" << boardId;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, STATUS_FAIL);

	channelId -= (boardId - 1) * MAX_CHANNEL_IDS; //each board has a separate range

	if (pBoard->GetRTMChannelIds())
		return pBoard->GetRTMChannelIds()->DeAllocate(channelId);

	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
bool CSystemResources::GetIsChannelIdAllocated(BoardID boardId, ChannelID channelId)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, false);

	return pBoard->GetRTMChannelIds() ? pBoard->GetRTMChannelIds()->GetIsAllocated(channelId) : false;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetBestBoards(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, WORD party_ip_service_id)
{
	DataPerBoardStruct* pAllocDataPerBoardArray = new DataPerBoardStruct[BOARDS_NUM];
	bool bWasDowngraded = FALSE;

	TRACEINTO << "IpServiceId:" << party_ip_service_id;

	WORD ip_service_id = party_ip_service_id;
	if (partyData.m_networkPartyType == eISDN_network_party_type)
		ip_service_id = ID_ALL_IP_SERVICES;

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		memset(&(pAllocDataPerBoardArray[i]), 0, sizeof(DataPerBoardStruct));

		if (!IsBoardIdExists(i + 1))
		{
			continue;
		}

		if (IsIpServiceConfiguredToBoard(ip_service_id, i + 1))              //ip_service_id == 0xFFFF ==> all boards
		{
			CollectDataForBoard(pAllocDataPerBoardArray[i], partyData, i + 1); //our array is zero-based but all the code is 1-based

			if (pAllocDataPerBoardArray[i].m_VideoData.m_bWasDownGraded)       //carmit
			{
				bWasDowngraded = TRUE;
			}
		}
	}

	if (bWasDowngraded)
		DumpBrdsState(pAllocDataPerBoardArray);

	STATUS status = m_pAllocationDecider->DecideAboutBestBoards(partyData, bestAllocStruct, pAllocDataPerBoardArray);

	delete[] pAllocDataPerBoardArray;

	if (STATUS_INSUFFICIENT_VIDEO_RSRC == status || STATUS_INSUFFICIENT_ART_RSRC == status) // When the MCU fails to allocate resources and at least one media card is overload,
	{
		for (WORD i = 0; i < BOARDS_NUM; i++) // the disconnect cause should be: "insufficient resources - media card overload" (by Anat L.)
		{
			if (IsBoardIdExists(i + 1) && GetBoard(i + 1)->GetHighUsageCPUstate())
				return STATUS_INSUFFICIENT_RSRC_CARD_OVERLOAD;
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetBestARTBoardCOP(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, int videoBoardId, WORD party_ip_service_id) //Olga
{
	DataPerBoardStruct* pAllocDataPerBoardArray = new DataPerBoardStruct[BOARDS_NUM];

	WORD ip_service_id = party_ip_service_id;
	if (partyData.m_networkPartyType == eISDN_network_party_type)
	{
		ip_service_id = ID_ALL_IP_SERVICES;
	}

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		memset(&(pAllocDataPerBoardArray[i]), 0, sizeof(DataPerBoardStruct));
		if (!IsBoardIdExists(i + 1))
			continue;
		if (IsIpServiceConfiguredToBoard(ip_service_id, i + 1))                    //ip_service_id == 0xFFFF ==> all boards
		{
			CollectDataForBoard(pAllocDataPerBoardArray[i], partyData, i + 1); //our array is zero-based but all the code is 1-based
		}
	}

	STATUS status = m_pAllocationDecider->DecideAboutBestARTBoardCOP(partyData, bestAllocStruct, pAllocDataPerBoardArray, videoBoardId);

	delete[] pAllocDataPerBoardArray;

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetBestBoardsCOP( eSessionType sessnType, eLogicalResourceTypes encType, BestAllocStruct& bestAllocStruct, WORD ip_service_id)
{
	DataPerBoardStruct* pAllocDataPerBoardArray = new DataPerBoardStruct[BOARDS_NUM];

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		memset(&(pAllocDataPerBoardArray[i]), 0, sizeof(DataPerBoardStruct));
		if (!IsBoardIdExists(i + 1))
			continue;
		if (IsIpServiceConfiguredToBoard(ip_service_id, i + 1)) //ip_service_id == 0xFFFF ==> all boards
		{
			CollectVideoAvailabilityForBoard2C(sessnType, encType, pAllocDataPerBoardArray[i].m_VideoData, i + 1); //our array is zero-based but all the code is 1-based
		}
	}

	STATUS status = m_pAllocationDecider->DecideAboutBestBoardsCOP(bestAllocStruct, pAllocDataPerBoardArray);

	delete[] pAllocDataPerBoardArray;

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetBestBoardsVSW(eSessionType sessionType, BestAllocStruct& bestAllocStruct1, BestAllocStruct& bestAllocStruct2, WORD ip_service_id)
{
	DataPerBoardStruct* pAllocDataPerBoardArray = new DataPerBoardStruct[BOARDS_NUM];

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		memset(&(pAllocDataPerBoardArray[i]), 0, sizeof(DataPerBoardStruct));
		if (!IsBoardIdExists(i + 1))
			continue;
		if (IsIpServiceConfiguredToBoard(ip_service_id, i + 1)) //ip_service_id == 0xFFFF ==> all boards
		{
			CollectVideoAvailabilityForBoard2C(sessionType, eLogical_res_none, pAllocDataPerBoardArray[i].m_VideoData, i + 1); //our array is zero-based but all the code is 1-based
		}
	}

	STATUS status = m_pAllocationDecider->DecideAboutBestBoardsCOP(bestAllocStruct1, pAllocDataPerBoardArray);

	if (eVSW_56_session == sessionType && status != STATUS_OK)
	{
		for (int i = 0; i < BOARDS_NUM; i++)
		{
			memset(&(pAllocDataPerBoardArray[i]), 0, sizeof(DataPerBoardStruct));
			if (!IsBoardIdExists(i + 1))
				continue;

			CollectVideoAvailabilityForBoard2C(eVSW_28_session, eLogical_res_none, pAllocDataPerBoardArray[i].m_VideoData, i + 1);
		}

		memset(&bestAllocStruct1, 0, sizeof(bestAllocStruct1));
		status = m_pAllocationDecider->DecideDiffBoardsForVSW56Session(bestAllocStruct1, bestAllocStruct2, pAllocDataPerBoardArray);
	}

	delete[] pAllocDataPerBoardArray;

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::CollectDataForBoard(DataPerBoardStruct& allocStruct, PartyDataStruct& partyData, int boardId)
{
	// 1. ART
	CollectArtAvailabilityForBoard(allocStruct.m_ARTData, partyData, boardId);

	BOOL canVideoRequestBeSatisfied = TRUE; // Not used
	// 2. VIDEO
	if (CHelperFuncs::IsVideoParty(partyData.m_videoPartyType))
	{
		CollectVideoAvailabilityForBoard(allocStruct.m_VideoData, partyData, boardId, canVideoRequestBeSatisfied);
	}

	// 3. RTM - we have to choose RTM boards and spans only for ISDN dial-out participants
	if (CHelperFuncs::IsISDNParty(partyData.m_networkPartyType) && CHelperFuncs::IsDialOutParty(partyData.m_pIsdn_Params_Request))
	{
		CollectRTMAvailabilityForBoard(allocStruct.m_RTMData, partyData, boardId);
	}
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::CollectArtAvailabilityForBoard(ARTDataPerBoardStruct& artAlloc, PartyDataStruct& partyData, int boardId)
{
	int num_parties;
	STATUS status;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN(pBoard == NULL);

	WORD neededArtChannels = CHelperFuncs::GetNumArtChannels(partyData);
	ETipPartyTypeAndPosition partyTypeTIP = partyData.m_partyTypeTIP;
	BOOL isTIP = (partyData.m_room_id != 0xFFFF && (CHelperFuncs::IsTIPSlavePartyType(partyTypeTIP) || eTipVideoAux == partyTypeTIP));
	WORD tipNumOfScreens = (partyData.m_partyTypeTIP == eTipMasterCenter) ? partyData.m_tipNumOfScreens : 0;

	std::ostringstream msg;

	if (pBoard->GetHighUsageCPUstate())     // Control of high usage CPU per board
	{
		msg << "RESOURCE_CAUSE: high usage CPU of this board " << boardId;
		goto Finalize;
	}
	if (partyData.m_reqBoardId != 0xFFFF && partyData.m_reqBoardId != boardId)
	{
		msg << "RESOURCE_CAUSE: " << (isTIP ? "TIP call" : (CHelperFuncs::IsVideoRelayParty(partyData.m_videoPartyType) ? "SVC call" : "call"));
		msg << " can't be allocated with ART on this board " << boardId;
		goto Finalize;
	}

	// In MPMx card, we would like to limit the number of ART ports of Video parties to 100 per card.
	// For example, if all the card is configured to ART units (to Audio Only ports), theoretically we allow 360 ART ports (of any kind)
	// and therefore we need this additional limitation.
	if (CHelperFuncs::IsBreeze(pBoard->GetCardType()) && CHelperFuncs::IsVideoParty(partyData.m_videoPartyType) && pBoard->GetNumVideoParticipantsWithArtOnThisBoard() >= MAX_NUM_VIDEO_PARTICIPANTS_WITH_ART_ON_CARD)
	{
		artAlloc.m_bCanBeAllocated = FALSE;
		artAlloc.m_numFreeArtPorts = 0;
		TRACEINTO << "\nRESOURCE_CAUSE: Maximum number of video participants with ART reached on board " << boardId << " Current number is: " << pBoard->GetNumVideoParticipantsWithArtOnThisBoard();
		return;
	}

	status = GetRemainingArtOnBoard(boardId, neededArtChannels, num_parties, partyData.m_artCapacity, partyData, tipNumOfScreens, true);

	artAlloc.m_numFreeArtPorts = num_parties;

	if (status == STATUS_OK && num_parties > 0)
		artAlloc.m_bCanBeAllocated = TRUE;
	else
	{
		artAlloc.m_bCanBeAllocated = FALSE;

		if (CanUseReconfigurationForAllocation(partyData) == TRUE)
		{
			//we will try to find a unit that can be changed from video to ART
			CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

			std::set<CUnitMFA>::iterator i;
			for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
			{
				if (i->GetIsEnabled() != TRUE)
					continue;
				if (eUnitType_Video == i->GetUnitType() && i->GetFreeCapacity() == 1000)
				{
					artAlloc.m_bCanAndShouldReconfigureVideoUnitToART = TRUE;
					break;
				}
			}
		}
	}
	return;

Finalize: artAlloc.m_bCanBeAllocated = FALSE;
	artAlloc.m_numFreeArtPorts = 0;
	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::CollectBandWidthAvailabilityForBoard(VideoDataPerBoardStruct& videoAlloc, PartyDataStruct& partyData, int boardId)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN(pBoard == NULL);
	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	// Collect free bandwidth per FPGA for MPMx cards.
	// MPM-Rx/Ninja bandwidth calculation is calculated per Netra unit. Each MPM-Rx card has one FPGA.
	if (CHelperFuncs::IsBreeze(pBoard->GetCardType()))
	{
		for (int i = 0; i < NUM_OF_FPGAS_PER_BOARD; i++)
		{
			videoAlloc.m_freeBandwidth_in[i] = BANDWIDTH_PER_FPGA;
			videoAlloc.m_freeBandwidth_out[i] = BANDWIDTH_PER_FPGA;
		}

		std::set<CUnitMFA>::iterator i;

		for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
		{
			if (i->GetIsEnabled() != TRUE)
				continue;
			if (eUnitType_Video == i->GetUnitType())
			{
				WORD sFPGA_Index = i->GetFPGAIndex();

				if (sFPGA_Index < NUM_OF_FPGAS_PER_BOARD)
				{
					videoAlloc.m_freeBandwidth_in[sFPGA_Index] -= i->GetUtilizedBandwidth(TRUE);
					videoAlloc.m_freeBandwidth_out[sFPGA_Index] -= i->GetUtilizedBandwidth(FALSE);
				}
				else
					PASSERT(sFPGA_Index);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::CollectVideoAvailabilityForBoard(VideoDataPerBoardStruct& videoAlloc, PartyDataStruct& partyData, int boardId, BOOL& canVideoRequestBeSatisfied)
{
	STATUS status;
	videoAlloc.m_bCanBeAllocated = FALSE;
	canVideoRequestBeSatisfied = TRUE;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN(pBoard == NULL);

	if (pBoard->GetHighUsageCPUstate())     // Control of high usage CPU per board
	{
		videoAlloc.m_bCanBeAllocated = FALSE;
		canVideoRequestBeSatisfied = FALSE;
		TRACEINTO << "RESOURCE_CAUSE: high usage CPU of this board " << boardId;
		return;
	}

	if (CHelperFuncs::IsVideoRelayParty(partyData.m_videoPartyType) && (partyData.m_reqBoardId != 0xFFFF) && (partyData.m_reqBoardId != boardId))
	{
		TRACEINTO << "RESOURCE_CAUSE:  SVC call can't be allocated with ART on this board " << boardId;
		status = STATUS_FAIL;
	}
	else
	{
		CollectBandWidthAvailabilityForBoard(videoAlloc, partyData, boardId);
		if (!CHelperFuncs::IsNeedAllocateVideo(partyData.m_videoPartyType, partyData.m_confModeType, m_ProductType)) //no need to allocate video resources for SVC only
			status = STATUS_OK;
		else
			status = FindVideoUnits(partyData, videoAlloc, boardId, canVideoRequestBeSatisfied);
	}

	if (STATUS_OK == status)
	{
		float freePortCapacity = 0;
		videoAlloc.m_bCanBeAllocated = TRUE;
		videoAlloc.m_numFreeVideoCapacity = (int)GetFreeVideoCapacity(boardId, &freePortCapacity);
		videoAlloc.m_NumVideoPartiesSameConf = GetNumVideoParties(boardId, partyData.m_monitor_conf_id);
		videoAlloc.m_numFreeVideoPortsCapacity = (int)freePortCapacity;
	}
	else
	{
		TRACEINTO << "RESOURCE_CAUSE: failed -  boardId = " << boardId << ", status = " << status;
		videoAlloc.m_bCanBeAllocated = FALSE;
		canVideoRequestBeSatisfied = FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::CollectVideoAvailabilityForBoard2C(eSessionType sessnType, eLogicalResourceTypes encType, VideoDataPerBoardStruct& videoAlloc, int boardId)
{
	STATUS status = STATUS_FAIL;
	videoAlloc.m_bCanBeAllocated = FALSE;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN(pBoard == NULL);

	if (CHelperFuncs::IsBreeze(pBoard->GetCardType()))
	{
		status = FindVideoUnitsCOP(sessnType, encType, videoAlloc, boardId);
	}

	if (STATUS_OK == status)
	{
		videoAlloc.m_bCanBeAllocated = TRUE;
		videoAlloc.m_numFreeVideoCapacity = (int)GetFreeVideoCapacity(boardId);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::CollectVideoAvailabilityForBoard2C failed - status ", status);
		videoAlloc.m_bCanBeAllocated = FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindVideoUnits(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, BOOL& canVideoRequestBeSatisfied)
{
	eVideoPartyType videoPartyType = partyData.m_videoPartyType;

	STATUS status = FindOptUnits(partyData, videoPartyType, videoAlloc, reqBoardId);
	if (status == STATUS_OK)
		return STATUS_OK;

	while (status != STATUS_OK)
	{
		videoPartyType = CHelperFuncs::GetNextLowerVideoPartyType(videoPartyType);
		if (videoPartyType == eVideo_party_type_none) //got as low as possible
		{
			TRACEINTO << "Down-grade as low as possible";
			return status;
		}

		TRACEINTO << "VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << ", BoardId:" << reqBoardId << " - Try to find unit with free place for down-graded video type";

		status = FindOptUnits(partyData, videoPartyType, videoAlloc, reqBoardId);
	}

	if (status == STATUS_OK)
	{
		videoAlloc.m_bWasDownGraded = TRUE;
		videoAlloc.m_DownGradedPartyType = videoPartyType;
		canVideoRequestBeSatisfied = FALSE;
		TRACEINTO << "VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << ", BoardId:" << reqBoardId << " - Free unit for downg-raded video type was found";
	}
	else
	{
		TRACEINTO << "VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << ", BoardId:" << reqBoardId << " - Failed to find free unit for downgraded video type";
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindVideoUnitsCOP(eSessionType sessnType, eLogicalResourceTypes encType, VideoDataPerBoardStruct& videoAlloc, int reqBoardId)
{
	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);
	if (pBoard->GetNumCOPConfOnThisBoard() >= MAX_NUM_COP_CONF_ON_CARD) //Olga
	{
		TRACEINTO << "CMaximum number of COP/VSW conferences is reached on board " << reqBoardId;
		return STATUS_FAIL;
	}
	STATUS  status = FindOptUnitsCOP(sessnType, encType,  videoAlloc, reqBoardId);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptUnits(PartyDataStruct& partyData, eVideoPartyType videoPartyType, VideoDataPerBoardStruct& videoAlloc, BoardID boardId)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, STATUS_FAIL);

	TRACEINTO << "BoardId:" << boardId << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType];

	videoAlloc.m_VideoAlloc.m_confModeType = partyData.m_confModeType; // AVC-SVC MIX mode

	pBoard->FillVideoUnitsList(videoPartyType, videoAlloc.m_VideoAlloc, partyData.m_partyRole, partyData.m_HdVswTypeInMixAvcSvcMode);

	STATUS status = STATUS_INSUFFICIENT_VIDEO_RSRC;
	CProcessBase* pProcess = CResourceProcess::GetProcess();
	CSysConfig* pSysConfig = pProcess ? pProcess->GetSysConfig() : NULL;
	BOOL is_spread_video_ports = NO;
	if (pSysConfig)
		pSysConfig->GetBOOLDataByKey("SPREADING_VIDEO_PORTS", is_spread_video_ports);

	DumpBoardUnits(videoAlloc, boardId);

	if (videoPartyType == eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type && CHelperFuncs::IsBreeze(pBoard->GetCardType()))
	{
		eLogicalResourceTypes ResourceType0 = videoAlloc.m_VideoAlloc.m_unitsList[0].m_MediaPortsList[0].m_type;
		eLogicalResourceTypes ResourceType1 = videoAlloc.m_VideoAlloc.m_unitsList[1].m_MediaPortsList[0].m_type;
		eLogicalResourceTypes ResourceType2 = videoAlloc.m_VideoAlloc.m_unitsList[2].m_MediaPortsList[0].m_type;

		if (ResourceType0 == eLogical_video_decoder && ResourceType1 == eLogical_video_encoder && ResourceType2 == eLogical_video_encoder)
		{
			status = FindOptBreezeUnit(partyData, videoAlloc, boardId, 0, is_spread_video_ports);
			if (status == STATUS_OK)
			{
				status = FindOptBreezeUnits(partyData, videoAlloc, boardId, 1, 2);
			}
		}

		if (status == STATUS_OK && (eMix == partyData.m_confModeType))
		{
			// BRIDGE-6630 asymmetric 1080p60 + additional avc to svc =>  1 unit for decoder, 2 units for encoder, 1 unit for avc to svc sd+cif encoders
			int unitIndex = 3;
			status = FindOptUnitsStartingFromSpecificUnitId(partyData, videoAlloc, boardId, unitIndex, is_spread_video_ports);
		}
	}
	else
	{
		status = FindOptUnitsStartingFromSpecificUnitId(partyData, videoAlloc, boardId, 0, is_spread_video_ports);
	}

	if (status != STATUS_OK)
	{
		status = STATUS_INSUFFICIENT_VIDEO_RSRC;
		TRACEINTO << "Failed, returning STATUS_INSUFFICIENT_VIDEO_RSRC";
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptUnitsStartingFromSpecificUnitId(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int boardId, int startUnitId, BOOL is_spread_video_ports)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, STATUS_FAIL);

	STATUS status = STATUS_INSUFFICIENT_VIDEO_RSRC;
	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	for (int unitIndex = startUnitId; unitIndex < max_units_video; unitIndex++)
	{
		if (videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededCapacity() > 0)
		{
			TRACEINTO << "UnitId:" << unitIndex << ", NeededCapacity:" << (WORD)videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededCapacity();
			if (CHelperFuncs::IsBreeze(pBoard->GetCardType()) || CHelperFuncs::IsSoftCard(pBoard->GetCardType()))
				status = FindOptBreezeUnit(partyData, videoAlloc, boardId, unitIndex, is_spread_video_ports);
			else if (CHelperFuncs::IsMpmRxOrNinja(pBoard->GetCardType()))
				status = FindOptNetraUnit(partyData, videoAlloc, boardId, unitIndex);

			if (status != STATUS_OK)
				break;
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptUnitsCOP(eSessionType sessnType, eLogicalResourceTypes encType, VideoDataPerBoardStruct& videoAlloc, int reqBoardId)
{
	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);

	//======================================
	pBoard->FillVideoUnitsListCOP(sessnType, encType, videoAlloc.m_VideoAlloc);
	//======================================

	STATUS status = STATUS_INSUFFICIENT_VIDEO_RSRC;

	if ((IsBoardIdExists(reqBoardId)))
	{
		WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
		for (int unitId = 0; unitId < max_units_video; unitId++)
		{
			if (videoAlloc.m_VideoAlloc.m_unitsList[unitId].GetTotalNeededCapacity() > 0)
			{
				if (eSystemCardsMode_breeze == m_SystemCardsMode) //Breeze-COP
					status = FindOptBreezeUnitCOP(videoAlloc, reqBoardId, unitId);

				if (status != STATUS_OK)
					break;
			}
		}
	}

	if (status != STATUS_OK)
	{
		status = STATUS_INSUFFICIENT_VIDEO_RSRC;
		PTRACE(eLevelInfoNormal, "CSystemResources::findOptUnitsCOP returning STATUS_INSUFFICIENT_VIDEO_RSRC");
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptBreezeUnitCOP(VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex)
{                                                       //Breeze-COP
	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);

	float curr_capacity = 0;
	float min_capacity = 1001;
	float max_capacity = 0; // spreading video units
	float neededCapacity = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededCapacity();
	eLogicalResourceTypes logicalRsrcType = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_MediaPortsList[0].m_type;

	int otherUnitsIndex;
	BOOL unitAlreadyAllocated;

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	std::set<CUnitMFA>::iterator i;
	std::set<CUnitMFA>::iterator requestedVideoUnit = pMediaUnitslist->end();
	std::set<CUnitMFA>::iterator freeAudioUnit = pMediaUnitslist->end();

	CLargeString lstr;
	lstr << "board_id: " << reqBoardId << "\n";

	TRACEINTO << "LogicalRsrcType:" << logicalRsrcType;

	for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
	{
		WORD currUnitId = i->GetUnitId();
		lstr << "unit_id: " << currUnitId;

		if (i->GetIsEnabled() == FALSE)
		{
			lstr << " = disabled \n";
			continue;
		}

		//check that we didn't plan to allocate it for the other required units
		unitAlreadyAllocated = FALSE;
		for (otherUnitsIndex = 0; otherUnitsIndex < unitIndex; otherUnitsIndex++)
		{
			if (videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].m_UnitId == i->GetUnitId())
			{
				unitAlreadyAllocated = TRUE;
				break;
			}
		}
		if (unitAlreadyAllocated)
		{
			lstr << " unitAlreadyAllocated \n";
			continue;
		}

		if (i->GetUnitType() != eUnitType_Video)
		{

			lstr << " not video unit";
			if (i->GetUnitType() == eUnitType_Art_Control)
			{
				lstr << " (eUnitType_Art_Control)";
			}
			else if (i->GetUnitType() == eUnitType_Art)
			{
				lstr << " (eUnitType_Art), FreeCapacity = " << i->GetFreeCapacity();

			}
			else
			{
				lstr << " (UNKNOWN UNIT TYPE = " << (WORD)(i->GetUnitType()) << ")";
			}
			if (freeAudioUnit == i)
			{
				lstr << " , free to reconfigure";
			}
			lstr << "\n";
			continue;
		}

		curr_capacity = i->GetFreeCapacity();
		if (curr_capacity < neededCapacity) //Insufficient capacity
		{
			lstr << " Insufficient capacity, free_capacity = " << curr_capacity << " < neededCapacity = " << neededCapacity << "\n";
			continue;
		}

		lstr << " free_capacity = " << curr_capacity;

		BOOL curr_is_turbo = CCardResourceConfigBreeze::IsTurboVideoUnit(currUnitId);
		if (curr_is_turbo)
			lstr << " , Turbo DSP ";

		//HD1080 and HD720--50/60 and PCM
		// solution for Algo in MPMX - always try to allocate the Encoder port of HD720@60 (Full DSP) to a Turbo DSP if available
		if ((eLogical_COP_HD1080_encoder == logicalRsrcType || eLogical_COP_PCM_encoder == logicalRsrcType) && (CCardResourceConfigBreeze::IsTurboVideoUnit(currUnitId)))
		{
			min_capacity = curr_capacity;
			//max_free_bandwidth_in = videoAlloc.m_freeBandwidth_in[i->GetFPGAIndex()];
			//max_free_bandwidth_out = videoAlloc.m_freeBandwidth_out[i->GetFPGAIndex()];
			requestedVideoUnit = i;
			PTRACE2INT(eLevelInfoNormal, " TURBO_DSP : CSystemResources::findOptBreezeUnitCOP : found turbo dsp for video encoder,  unit id = ", currUnitId);
			lstr << "\n";
			break;
		}

		if ((curr_capacity < min_capacity || curr_capacity == min_capacity) // prefer not Turbo DSP
		&& (!CCardResourceConfigBreeze::IsTurboVideoUnit(currUnitId) || requestedVideoUnit == pMediaUnitslist->end()))
		{
			min_capacity = curr_capacity;
			//max_free_bandwidth_in = videoAlloc.m_freeBandwidth_in[i->GetFPGAIndex()];
			//max_free_bandwidth_out = videoAlloc.m_freeBandwidth_out[i->GetFPGAIndex()];
			requestedVideoUnit = i;
		}

		lstr << "\n";
	}

	if (!m_bIsRsrcEnough)
	{
		lstr << "!m_bIsRsrcEnough ";
	}
	lstr << "\n";

	if (requestedVideoUnit == pMediaUnitslist->end()) //not found at all, fail
	{
		lstr << " unit not found \n";
		PTRACE2(eLevelInfoNormal, "CSystemResources::findOptBreezeUnitCOP: \n", lstr.GetString());
		return STATUS_INSUFFICIENT_VIDEO_RSRC;    //status fail
	}

	if (min_capacity < 1000)                          //found unit fragmentated
	{
		lstr << "found unit: " << requestedVideoUnit->GetUnitId() << " , min_capacity = " << min_capacity << "\n";
		videoAlloc.m_bFragmentedUnit = TRUE;
	}
	else
	{
		lstr << "found unit: " << requestedVideoUnit->GetUnitId() << " , empty unit \n";
	}

	PTRACE2(eLevelInfoNormal, "CSystemResources::findOptBreezeUnitCOP: \n", lstr.GetString());

	// fill parameters found for return
	const CUnitMFA* pUnit = (&(*requestedVideoUnit));
	videoAlloc.m_VideoAlloc.m_boardId = pUnit->GetBoardId();
	videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_UnitId = pUnit->GetUnitId();
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptBreezeUnit(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex, BOOL is_spread_video_ports)
{
	if (CHelperFuncs::IsSoftMCU(m_ProductType) && (eProductTypeNinja != m_ProductType))
		return FindOptSoftUnit(partyData, videoAlloc, reqBoardId, unitIndex, is_spread_video_ports);

	BOOL isTurboDspPreferred = FALSE;

	switch (partyData.m_videoPartyType)
	{
		case eCP_H264_upto_HD720_60FS_Symmetric_video_party_type:
		case eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type:
		{
			eLogicalResourceTypes logicalRsrcType = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_MediaPortsList[0].m_type;

			if (eLogical_video_encoder == logicalRsrcType)
			{
				PTRACE(eLevelInfoNormal, "CSystemResources::findOptBreezeUnit - Look for TurboDSP to video encoder type HD1080_30FS / HD720_60FS");
				isTurboDspPreferred = TRUE;
			}
			break;
		}

		case eCP_H264_upto_HD720_30FS_Symmetric_video_party_type:
		{
			if ((eTipMasterCenter == partyData.m_partyTypeTIP) || (eTipSlaveLeft == partyData.m_partyTypeTIP) || (eTipSlaveRigth == partyData.m_partyTypeTIP))
			{
				PTRACE(eLevelInfoNormal, "CSystemResources::findOptBreezeUnit - Look for TurboDSP to video encoder & decoder type HD720_30FS TIP");
				isTurboDspPreferred = TRUE;
			}
			break;
		}

		default:
			break;
	}

	if (TRUE == isTurboDspPreferred)
	{
		STATUS status = FindOptBreezeUnit(partyData, videoAlloc, reqBoardId, unitIndex, is_spread_video_ports, TRUE);
		if (status == STATUS_OK)
			return STATUS_OK;
		PTRACE(eLevelInfoNormal, "CSystemResources::findOptBreezeUnit - TurboDSP not found, therefore look for non-TurboDSP");
	}
	return FindOptBreezeUnit(partyData, videoAlloc, reqBoardId, unitIndex, is_spread_video_ports, FALSE);
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::findTwoSuitableUnitsForSplittedEncoder(CFittedUnitsMap& mapFittedUnits, CFittedUnitsItr (&_iiFound)[2], PriorityType priorityType)
{
	_iiFound[0] = mapFittedUnits.end();
	_iiFound[1] = mapFittedUnits.end();

	for (CFittedUnitsItr _ii = mapFittedUnits.begin(); _ii != mapFittedUnits.end(); ++_ii)
	{
		if (priorityType == PriorityType_TurboDSP && !_ii->first.unit_turbo)
			continue;                                                 // Skip non-Turbo-DSPs

		if (priorityType == PriorityType_NonTurboDSP && _ii->first.unit_turbo)
			continue;                                                 // Skip Turbo-DSPs

		if (_iiFound[0] == mapFittedUnits.end())                          // The first suitable DSP is found
		{
			_iiFound[0] = _ii;
			continue;
		}

		if (_ii->first.fpga_index != _iiFound[0]->second->GetFPGAIndex()) // The second suitable DSP is found, but on other FPGA than the first DSP,
		{                                                                 // so therefore, we consider it as the first.
			_iiFound[0] = _ii;
			continue;
		}
		_iiFound[1] = _ii;                                                // The second suitable DSP is found on the same FPGA as the first DSP
		return STATUS_OK;
	}
	return STATUS_INSUFFICIENT_VIDEO_RSRC;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptBreezeUnits(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndexMaster, int unitIndexSlave)
{
	// We need two empty DSP units for video encoder HD1080@60FS resolution
	// Priority of choice should be the following:
	// 1. Try to allocate both video encoders on Turbo-DSP
	// 2. Try to allocate both video encoders on non-Turbo-DSP
	// 3. Try to allocate one video encoder on non-Turbo-DSP, other on Turbo-DSP
	// 4. In the case we have to choose non-Turbo-DSP the higher priority choice should be VIDEO-DSP than ART-DSP
	// 5. Important: both DSP units should be on the same FPGA

	STATUS status = STATUS_INSUFFICIENT_VIDEO_RSRC;

	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, STATUS_FAIL);

	DWORD need_bandwidth_in = videoAlloc.m_VideoAlloc.m_unitsList[unitIndexMaster].GetTotalNeededBandwidthIn();
	DWORD need_bandwidth_out = videoAlloc.m_VideoAlloc.m_unitsList[unitIndexMaster].GetTotalNeededBandwidthOut();
	float need_capacity = videoAlloc.m_VideoAlloc.m_unitsList[unitIndexMaster].GetTotalNeededCapacity();

	std::ostringstream msg;
	msg << "CSystemResources::findOptBreezeUnits(ENCODER) "
			<< "- BoardId:" << reqBoardId
			<< ", MasterUnitIndex:" << unitIndexMaster
			<< ", SlaveUnitIndex:" << unitIndexSlave
			<< ", NeededBandwidthIn:" << need_bandwidth_in
			<< ", NeededBandwidthOut:" << need_bandwidth_out
			<< ", NeededCapacity:" << need_capacity;

	// Work around the entire media units list and select only those units that satisfy the conditions of allocation - candidates.
	// The candidates units pass into map sorted according with priorities described at the header of the findOptBreezeUnits(..) function.
	CFittedUnitsMap mapFittedUnits;
	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	std::set<CUnitMFA>::iterator _iItr, _iEnd = pMediaUnitslist->end();
	for (_iItr = pMediaUnitslist->begin(); _iItr != _iEnd; ++_iItr)
	{
		WORD  unitId         = _iItr->GetUnitId();
		WORD  physicalUnitId = _iItr->GetPhysicalUnitId();
		float freeCapacity   = _iItr->GetFreeCapacity();
		WORD  fpgaIndex      = _iItr->GetFPGAIndex();
		bool  isUnitEnabled  = _iItr->GetIsEnabled();
		bool  isUnitVideo    = _iItr->GetUnitType() == eUnitType_Video ? true : false;
		bool  isUnitTurbo    = CCardResourceConfigBreeze::IsTurboVideoUnit(physicalUnitId);
		bool  isUnitSelected = IsUnitAlreadySelectedForAllocation(videoAlloc, unitIndexMaster, unitId);

		DWORD free_bandwidth_in = 0;
		DWORD free_bandwidth_out = 0;

		if (fpgaIndex < NUM_OF_FPGAS_PER_BOARD)
		{
			free_bandwidth_in = videoAlloc.m_freeBandwidth_in[fpgaIndex];
			free_bandwidth_out = videoAlloc.m_freeBandwidth_out[fpgaIndex];
		}
		else
		{
			PASSERT(fpgaIndex);
			free_bandwidth_in = 0;
			free_bandwidth_out = 0;
		}

		if (!isUnitEnabled)
			continue;                                       // Unit rejected because not enabled
		if (free_bandwidth_in < need_bandwidth_in)
			continue;                                       // Unit rejected because free bandwidth less than needed
		if (free_bandwidth_out < need_bandwidth_out)
			continue;                                       // Unit rejected because free bandwidth less than needed
		if (freeCapacity < need_capacity)
			continue;                                       // Unit rejected because free capacity less than needed
		if (isUnitSelected)
			continue;                                       // Unit rejected because already selected for allocation
		if (!isUnitVideo)
			continue;                                       // Unit rejected because not video

		CUnitKey unitKey = { fpgaIndex, isUnitTurbo, isUnitVideo };
		mapFittedUnits.insert(CFittedUnitsMap::value_type(unitKey, (CUnitMFA*)&(*_iItr)));
	}

	msg << endl;
	msg << "Sorted units map (units id):";
	for (CFittedUnitsItr _ii = mapFittedUnits.begin(); _ii != mapFittedUnits.end(); ++_ii)
		msg << _ii->second->GetUnitId() << ",";

	msg << endl;

	CFittedUnitsItr _iiFound[2];

	// Try to allocate both video encoders on Turbo-DSP
	status = findTwoSuitableUnitsForSplittedEncoder(mapFittedUnits, _iiFound, PriorityType_TurboDSP);
	if (status == STATUS_OK)
	{
		msg << "Found two suitable units with priorityType:PriorityType_TurboDSP, unit_id:";
		goto Finalize;
	}

	// Try to allocate both video encoders on non-Turbo-DSP
	status = findTwoSuitableUnitsForSplittedEncoder(mapFittedUnits, _iiFound, PriorityType_NonTurboDSP);
	if (status == STATUS_OK)
	{
		msg << "Found two suitable units with priorityType:PriorityType_NonTurboDSP, unit_id:";
		goto Finalize;
	}

	// Try to allocate one video encoder on non-Turbo-DSP, other on Turbo-DSP
	status = findTwoSuitableUnitsForSplittedEncoder(mapFittedUnits, _iiFound, PriorityType_AnyEmptyDSP);
	if (status == STATUS_OK)
	{
		msg << "Found two suitable units with priorityType:PriorityType_AnyEmptyDSP, unit_id:";
		goto Finalize;
	}

	msg << "Failed to find suitable unit - insufficient video resources";
	PTRACE(eLevelInfoNormal, msg.str().c_str());
	return status;

Finalize:
	msg << _iiFound[0]->second->GetUnitId() << "," << _iiFound[1]->second->GetUnitId();

	videoAlloc.m_VideoAlloc.m_boardId = _iiFound[0]->second->GetBoardId();
	videoAlloc.m_VideoAlloc.m_unitsList[unitIndexMaster].m_UnitId = _iiFound[0]->second->GetUnitId();
	videoAlloc.m_VideoAlloc.m_unitsList[unitIndexSlave].m_UnitId = _iiFound[1]->second->GetUnitId();
	PTRACE(eLevelInfoNormal, msg.str().c_str());
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptSoftUnit(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex, BOOL is_spread_video_ports)
{
	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);

	float min_capacity = MAX_FREE_UNIT_CAPACITY + 1; // 1001
	float max_capacity = 0;                          // spreading video units
	float neededCapacity = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededCapacity();

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	std::set<CUnitMFA>::iterator _iiFoundVideoUnit = pMediaUnitslist->end();
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	std::ostringstream msg;
	msg << "CSystemResources::findOptSoftUnit - LogicalResourceTypes: ";

	for (int i = 0; i < max_media_ports; i++)
	{
		if (eLogical_res_none != videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_MediaPortsList[i].m_type)
		{
			msg << videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_MediaPortsList[i].m_type << ", ";
		}
	}

	msg << endl << "board_id:" << reqBoardId << ", unit_index:" << unitIndex << ", is_spread_video_ports:" << (int)is_spread_video_ports << ", neededCapacity:" << neededCapacity << endl;

	for (std::set<CUnitMFA>::iterator _iiUnit = pMediaUnitslist->begin(); _iiUnit != pMediaUnitslist->end(); _iiUnit++)
	{
		WORD unit_id = _iiUnit->GetUnitId();
		WORD physical_unit_id = _iiUnit->GetPhysicalUnitId();
		float free_capacity = _iiUnit->GetFreeCapacity();

		if (_iiUnit->GetIsEnabled() == FALSE)
			continue;                                     // rejected - unit disabled

		// check that we didn't plan to allocate it for the other required units
		float capacity_already_allocated = 0;
		for (int otherUnitsIndex = 0; otherUnitsIndex < unitIndex; otherUnitsIndex++)
		{
			if (videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].m_UnitId == _iiUnit->GetUnitId())
			{
				capacity_already_allocated += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededCapacity();
			}
		}

		if (_iiUnit->GetUnitType() != eUnitType_Video)
			continue;                          // rejected - not a video unit

		if (free_capacity < neededCapacity + capacity_already_allocated)
			continue;                          // rejected - not enough free_capacity

		if (is_spread_video_ports)                 // Spreading CIFs in video units
		{
			if (free_capacity > max_capacity)
			{
				max_capacity = free_capacity;
				_iiFoundVideoUnit = _iiUnit;
				if (free_capacity == 1000) // found empty unit
					break;             // accepted - found empty unit
			}
		}
		else
		{

			if (                               // 1. check if we found better accelerator with less free capacity
			(free_capacity < min_capacity) ||
			// 4. prefer not Turbo DSP
			    (free_capacity == min_capacity && (_iiFoundVideoUnit == pMediaUnitslist->end())))
			{
				min_capacity = free_capacity;
				_iiFoundVideoUnit = _iiUnit;
			}
		}
	}

	if (_iiFoundVideoUnit == pMediaUnitslist->end())  // not found at all, fail
	{
		msg << "Failed to find suitable unit - insufficient video resources";
		PTRACE(eLevelInfoNormal, msg.str().c_str());
		return STATUS_INSUFFICIENT_VIDEO_RSRC;    // status fail
	}

	if (min_capacity < 1000)                          // found unit fragmented
	{
		msg << "Found suitable fragmented unit, unit_id:" << _iiFoundVideoUnit->GetUnitId();
		videoAlloc.m_bFragmentedUnit = TRUE;
	}
	else
	{
		msg << "Found suitable empty unit, unit_id:" << _iiFoundVideoUnit->GetUnitId();
	}

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	// fill parameters found for return
	const CUnitMFA* pUnit = (&(*_iiFoundVideoUnit));
	videoAlloc.m_VideoAlloc.m_boardId = pUnit->GetBoardId();
	videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_UnitId = pUnit->GetUnitId();
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptBreezeUnit(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex, BOOL is_spread_video_ports, BOOL is_turbo_needed)
{
	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);

	// VNGR-21581 - In a system with MPMX, change CIF and SD percentage to CIF=30%, SD=40% (instead of CIF=25%, SD=50%)
	// In RMX1500Q use 3CIF_PORTS_ON_DSP flag to disable/enable allocating 4 CIF on DSP when all DSPs (7 video dsp) are full
	// with 3 CIF on each one. (3*7=21 CIF and the license have 25 CIF / 7 HD720 so we must allocate 4 CIF on DSP to meet license)
	BOOL bLimit3CifPortsOnDsp = FALSE;
	if (CResRsrcCalculator::IsRMX1500Q())
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_3CIF_PORTS_ON_DSP, bLimit3CifPortsOnDsp);

	float min_capacity = MAX_FREE_UNIT_CAPACITY + 1; // 1001
	float min_encoder_weight = MAX_ENCODER_WEIGHT_PER_UNIT;
	float max_capacity = 0;                          // spreading video units
	BOOL selectedUnitContainsFreeDiabledPorts = FALSE;
	DWORD max_free_bandwidth_in = 0;
	DWORD max_free_bandwidth_out = 0;
	DWORD max_free_bandwidth_in_hd1080 = 0;
	DWORD max_free_bandwidth_out_hd1080 = 0;
	DWORD neededBandwidthIn = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededBandwidthIn();
	DWORD neededBandwidthOut = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededBandwidthOut();
	float neededCapacity = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededCapacity();
	float neededEncoderWeight = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededEncoderWeight();
	WORD neededEncoderPorts = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededVideoPortsPerType(TRUE);
	WORD neededDecoderPorts = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededVideoPortsPerType(FALSE);

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	std::set<CUnitMFA>::iterator _iiFoundVideoUnit = pMediaUnitslist->end();
	std::set<CUnitMFA>::iterator _iiFreeAudioUnit = pMediaUnitslist->end();
	std::set<CUnitMFA>::iterator _ii3CIFAllocatedUnit = pMediaUnitslist->end();
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	std::ostringstream msg;
	msg
	<< "\n  LogicalResourceType :";

	for (int i = 0; i < max_media_ports; i++)
	{
		if (eLogical_res_none != videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_MediaPortsList[i].m_type)
		{
			msg << videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_MediaPortsList[i].m_type << ", ";
		}
	}

	msg
	<< "\n  BoardId             :" << reqBoardId
	<< "\n  UnitIndex           :" << unitIndex
	<< "\n  IsSpreadVideoPorts  :" << (int)is_spread_video_ports
	<< "\n  IsTurboNeeded       :" << (int)is_turbo_needed
	<< "\n  NeededBandwidthIn   :" << neededBandwidthIn
	<< "\n  NeededBandwidthOut  :" << neededBandwidthOut
	<< "\n  NeededCapacity      :" << neededCapacity
	<< "\n  NeededEncoderWeight :" << neededEncoderWeight
	<< "\n  NeededEncoderPorts  :" << neededEncoderPorts
	<< "\n  NeededDecoderPorts  :" << neededDecoderPorts;

	for (std::set<CUnitMFA>::iterator _iiUnit = pMediaUnitslist->begin(); _iiUnit != pMediaUnitslist->end(); _iiUnit++)
	{
		WORD unit_id = _iiUnit->GetUnitId();
		WORD physical_unit_id = _iiUnit->GetPhysicalUnitId();
		BOOL unit_turbo = CCardResourceConfigBreeze::IsTurboVideoUnit(physical_unit_id);
		WORD fpga_index = _iiUnit->GetFPGAIndex();
		float free_capacity = _iiUnit->GetFreeCapacity();
		float free_encoder_weight = _iiUnit->GetFreeEncoderWeight();
		WORD all_free_encoder_ports = _iiUnit->GetFreeEncoderPorts(0, TRUE);
		WORD all_free_decoder_ports = _iiUnit->GetFreeDecoderPorts(0, TRUE);
		WORD free_encoder_ports = _iiUnit->GetFreeEncoderPorts(0, FALSE);
		WORD free_decoder_ports = _iiUnit->GetFreeDecoderPorts(0, FALSE);
		DWORD free_bandwidth_in = 0;
		DWORD free_bandwidth_out = 0;

		if (fpga_index < NUM_OF_FPGAS_PER_BOARD)
		{
			free_bandwidth_in = videoAlloc.m_freeBandwidth_in[fpga_index];
			free_bandwidth_out = videoAlloc.m_freeBandwidth_out[fpga_index];
		}
		else
		{
			PASSERT(fpga_index);
			free_bandwidth_in = 0;
			free_bandwidth_out = 0;
		}

		if (_iiUnit->GetIsEnabled() == FALSE)
			continue; // rejected - unit disabled

		if (free_bandwidth_in < neededBandwidthIn)
			continue; // rejected - not enough bandwidth_in

		if (free_bandwidth_out < neededBandwidthOut)
			continue; // rejected - not enough bandwidth_out

		// check that we didn't plan to allocate it for the other required units
		float capacity_already_allocated = 0;
		float encoder_weight_already_allocated = 0;
		WORD encoder_ports_already_allocated = 0;
		WORD decoder_ports_already_allocated = 0;
		for (int otherUnitsIndex = 0; otherUnitsIndex < unitIndex; otherUnitsIndex++)
		{
			if (videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].m_UnitId == _iiUnit->GetUnitId())
			{
				capacity_already_allocated += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededCapacity();
				encoder_weight_already_allocated += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededEncoderWeight();
				encoder_ports_already_allocated += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededVideoPortsPerType(TRUE);
				decoder_ports_already_allocated += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededVideoPortsPerType(FALSE);
			}
		}

		if (_iiUnit->GetUnitType() == eUnitType_Art && free_capacity == 1000)
			_iiFreeAudioUnit = _iiUnit;

		if (_iiUnit->GetUnitType() != eUnitType_Video)
			continue; // rejected - not a video unit

		if (free_capacity < neededCapacity + capacity_already_allocated)
			continue; // rejected - not enough free_capacity

		if ((neededEncoderPorts > 0 && all_free_encoder_ports < neededEncoderPorts + encoder_ports_already_allocated) || (neededDecoderPorts > 0 && all_free_decoder_ports < neededDecoderPorts + decoder_ports_already_allocated))
			continue; // rejected - not enough free encoder/decoder ports

		// in this stage we know that we have enough free encoder/decoder ports but they might contain ports with status PORT_FREE_DISABLED.
		// check if this unit has enough free encoder/decoder ports with status PORT_FREE.
		BOOL currUnitContainsFreeDiabledPorts = FALSE;
		if ((neededEncoderPorts > 0 && free_encoder_ports < neededEncoderPorts + encoder_ports_already_allocated) || (neededDecoderPorts > 0 && free_decoder_ports < neededDecoderPorts + decoder_ports_already_allocated))
			currUnitContainsFreeDiabledPorts = TRUE;

		// check if we have enough free encoder ports weight
		if ((free_encoder_weight + FLOATING_POINT_ERROR) < neededEncoderWeight + encoder_weight_already_allocated)
		{
			msg << "\nLimitation of maximum number of encoders per dsp, UnitId:" << unit_id << ", FreeEncoderWeight:" << free_encoder_weight << ", NeededEncoderWeight:" << neededEncoderWeight << ", EncoderWeightAlreadyAllocated:" << encoder_weight_already_allocated << ", so skip it";
			continue;                                     // rejected - limitation of maximum number of encoders per dsp
		}

		// Check the active ports list to limit allocation to 6 ports (encoders/decoders) per unit.
		WORD numOfVideoEncDecOnUnit = 0;
		const CActivePortsList* pActivePortsList = _iiUnit->GetActivePorts();
		for (std::set<CActivePort>::iterator _iiActivePort = pActivePortsList->begin(); _iiActivePort != pActivePortsList->end(); _iiActivePort++)
			if (_iiActivePort->GetPortType() == ePhysical_video_encoder || _iiActivePort->GetPortType() == ePhysical_video_decoder)
				++numOfVideoEncDecOnUnit;

		// If there are 3 CIF (enc+dec) allocated on the unit, or we have a total of 6 CIF encoders+decodes on this unit.
		if (numOfVideoEncDecOnUnit == 6)
		{
			// VNGR-21581
			// For RMX1500Q - if we have 3 CIF (encoder + decoder) on the unit or if it's SD+CIF.
			if (CResRsrcCalculator::IsRMX1500Q() && !bLimit3CifPortsOnDsp && free_capacity == VID_TOTAL_CIF_PROMILLES_BREEZE && neededCapacity == VID_TOTAL_CIF_PROMILLES_BREEZE)
			{
				if (_ii3CIFAllocatedUnit == pMediaUnitslist->end())
					_ii3CIFAllocatedUnit = _iiUnit;
			} // For RMX1500Q - accepted - sufficient free_capacity, but search for better DSP first

			continue; // Not RMX1500Q - do not allocate more resources on this unit.
		}

		if (is_spread_video_ports) // Spreading CIFs in video units
		{
			if (free_capacity > max_capacity)
			{
				max_capacity = free_capacity - capacity_already_allocated;
				_iiFoundVideoUnit = _iiUnit;
				if (free_capacity == 1000) // found empty unit
					break;             // accepted - found empty unit
			}
		}
		else
		{
			if (is_turbo_needed)         // solution for Algo in MPMX - always try to allocate the Encoder port
			{                            // of HD720@60 and HD1080@30 (Full DSP) to a Turbo DSP if available
				if (unit_turbo &&          // current unit is a turbo DSP.
				                           // check if we have a better unit without ports with status PORT_FREE_DISABLED
					((_iiFoundVideoUnit != pMediaUnitslist->end() && selectedUnitContainsFreeDiabledPorts && !currUnitContainsFreeDiabledPorts)
					 || IsBetterBandWidth(neededBandwidthIn, neededBandwidthOut, free_bandwidth_in, free_bandwidth_out, max_free_bandwidth_in_hd1080, max_free_bandwidth_out_hd1080)))
				{
					min_capacity                         = free_capacity - capacity_already_allocated;
					selectedUnitContainsFreeDiabledPorts = currUnitContainsFreeDiabledPorts;
					max_free_bandwidth_in_hd1080         = free_bandwidth_in;
					max_free_bandwidth_out_hd1080        = free_bandwidth_out;
					_iiFoundVideoUnit                    = _iiUnit;
				}
			}
			else
			{
				if ( // 1. prefer to allocate decoders on a unit with less free encoder weight (a unit with more allocated encoders)
				        (neededEncoderWeight == 0 && (free_encoder_weight - encoder_weight_already_allocated) < min_encoder_weight) ||
				     // 2. check if we found better accelerator with less free capacity
				        (free_capacity - capacity_already_allocated < min_capacity) ||
				     // 3. check if we have a better unit without ports with status PORT_FREE_DISABLED
				        (_iiFoundVideoUnit != pMediaUnitslist->end() && selectedUnitContainsFreeDiabledPorts && !currUnitContainsFreeDiabledPorts) ||
				     // 4. prefer not Turbo DSP
				        (free_capacity == min_capacity && (_iiFoundVideoUnit == pMediaUnitslist->end() || (!unit_turbo && CCardResourceConfigBreeze::IsTurboVideoUnit(_iiFoundVideoUnit->GetPhysicalUnitId())))) ||
				     // 5. check the bandwidth
				        (free_capacity == min_capacity && IsBetterBandWidth(neededBandwidthIn, neededBandwidthOut, free_bandwidth_in, free_bandwidth_out, max_free_bandwidth_in, max_free_bandwidth_out)))
				{
					min_encoder_weight                   = free_encoder_weight - encoder_weight_already_allocated;
					min_capacity                         = free_capacity - capacity_already_allocated;
					selectedUnitContainsFreeDiabledPorts = currUnitContainsFreeDiabledPorts;
					max_free_bandwidth_in                = free_bandwidth_in;
					max_free_bandwidth_out               = free_bandwidth_out;
					_iiFoundVideoUnit                    = _iiUnit;
				}
			}
		}
	}

	if (!is_turbo_needed)
	{
		if (_iiFoundVideoUnit == pMediaUnitslist->end() && _iiFreeAudioUnit != pMediaUnitslist->end())
		{
			if (CanUseReconfigurationForAllocation(partyData))
			{
				msg << "CanUseReconfigurationForAllocation:TRUE, Found unit to reconfigure, UnitId:" << _iiFreeAudioUnit->GetUnitId() << endl;
				_iiFoundVideoUnit = _iiFreeAudioUnit;
				videoAlloc.m_NumVideoUnitsToReconfigure++;
			}
			else
			{
				msg << "CanUseReconfigurationForAllocation:FALSE"
				    << ", allowReconfiguration:" << (int)partyData.m_allowReconfiguration
				    << ", isAutoModeAllocationType:" << (int)CHelperFuncs::IsAutoModeAllocationType()
				    << ", isRsrcEnough:" << (int)m_bIsRsrcEnough << endl;
			}
		}
	}

	if (_iiFoundVideoUnit == pMediaUnitslist->end())  // not found at all, fail
	{
		// VNGR-21581
		// If we found a unit allocated with 3 CIF and we want to allocate another CIF but we couldn't find a better unit
		// with enough free capacity or unit with less then 3 CIF.
		if (_ii3CIFAllocatedUnit != pMediaUnitslist->end())
		{
			_iiFoundVideoUnit = _ii3CIFAllocatedUnit;
			min_capacity      = VID_TOTAL_CIF_PROMILLES_BREEZE;
		}
		else                                           // We couldn't find available unit
		{
			msg << "\n**Failed to find suitable unit - insufficient video resources";
			PTRACE(eLevelInfoNormal, msg.str().c_str());
			return STATUS_INSUFFICIENT_VIDEO_RSRC; // status fail
		}
	}

	if (min_capacity < 1000)                               // found unit fragmented
	{
		msg << "\n**Found suitable fragmented unit, UnitId:" << _iiFoundVideoUnit->GetUnitId();
		videoAlloc.m_bFragmentedUnit = TRUE;
	}
	else
	{
		msg << "\n**Found suitable empty unit, UnitId:" << _iiFoundVideoUnit->GetUnitId();
	}

	TRACEINTO << msg.str().c_str();

	// fill parameters found for return
	const CUnitMFA* pUnit = (&(*_iiFoundVideoUnit));
	videoAlloc.m_VideoAlloc.m_boardId                       = pUnit->GetBoardId();
	videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_UnitId = pUnit->GetUnitId();
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptNetraUnit(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex)
{
	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);

	float min_capacity = MAX_FREE_UNIT_CAPACITY + 1; // 1001
	BOOL enough_free_ports = FALSE;
	BOOL selectedUnitContainsFreeDiabledPorts = FALSE;
	WORD selectedAcceleratorId = 0;
	float neededCapacity = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededCapacity();
	WORD neededEncoderPorts = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededVideoPortsPerType(TRUE);
	WORD neededDecoderPorts = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededVideoPortsPerType(FALSE);
	DWORD neededBandwidthIn = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededBandwidthIn();
	DWORD neededBandwidthOut = videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededBandwidthOut();
	DWORD selectedUnitUtilizedBandwidthOut = 0;
	DWORD selectedUnitUtilizedBandwidthIn = 0;
	WORD reservedUnitIdForSwapRecovery = 0;
	BOOL bSpreadVideoPortsOnNetraUnits = FALSE;
	BOOL selectedUnitIsPreferedForSpreading = FALSE;

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	std::set<CUnitMFA>::iterator _iiFoundVideoUnit = pMediaUnitslist->end();
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();

	std::ostringstream msg;

	msg << "CSystemResources::findOptNetraUnit - LogicalResourceTypes: ";

	CProcessBase* pProcess = CResourceProcess::GetProcess();
	CSysConfig* pSysConfig = pProcess ? pProcess->GetSysConfig() : NULL;
	if (pSysConfig)
		pSysConfig->GetBOOLDataByKey(CFG_KEY_SPREAD_VIDEO_ALLOCATION_ON_NETRA_UNITS, bSpreadVideoPortsOnNetraUnits);

	for (int i = 0; i < max_media_ports; i++)
	{
		if (eLogical_res_none != videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_MediaPortsList[i].m_type)
		{
			msg << videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_MediaPortsList[i].m_type << ", ";
		}
	}

	msg << endl << "board_id:" << reqBoardId << ", unit_index:" << unitIndex << ", neededCapacity:" << neededCapacity << ", neededEncoderPorts:" << neededEncoderPorts << ", neededDecoderPorts:" << neededDecoderPorts << ", neededBandwidthIn:" << neededBandwidthIn << ", neededBandwidthOut:" << neededBandwidthOut << endl;

	for (std::set<CUnitMFA>::iterator _iiUnit = pMediaUnitslist->begin(); _iiUnit != pMediaUnitslist->end(); _iiUnit++)
	{
		// bandwidth limitation is per unit and not per accelerator, so we check it here.
		DWORD utilized_bandwidth_in = _iiUnit->GetUtilizedBandwidth(TRUE);
		DWORD utilized_bandwidth_out = _iiUnit->GetUtilizedBandwidth(FALSE);

		if (_iiUnit->GetIsEnabled() == FALSE)
			continue;                                     // rejected - unit disabled

		if (_iiUnit->GetUnitType() != eUnitType_Video)
			continue;                                     // rejected - not a video unit

		float free_unit_capacity = 0;
		for (WORD currAcceleratorId = 0; currAcceleratorId < ACCELERATORS_PER_UNIT_NETRA; currAcceleratorId++)
		{
			free_unit_capacity += _iiUnit->GetFreeCapacity(currAcceleratorId);
		}

		// If spread video allocation on Netra units (accelerators) flag is ON - save the first video unit for unit swap recovery.
		if (bSpreadVideoPortsOnNetraUnits && reservedUnitIdForSwapRecovery == 0)
		{
			reservedUnitIdForSwapRecovery = _iiUnit->GetUnitId();
		}

		for (WORD currAcceleratorId = 0; currAcceleratorId < ACCELERATORS_PER_UNIT_NETRA; currAcceleratorId++)
		{
			float free_capacity          = _iiUnit->GetFreeCapacity(currAcceleratorId);
			WORD  all_free_encoder_ports = _iiUnit->GetFreeEncoderPorts(currAcceleratorId, TRUE);
			WORD  all_free_decoder_ports = _iiUnit->GetFreeDecoderPorts(currAcceleratorId, TRUE);
			WORD  free_encoder_ports     = _iiUnit->GetFreeEncoderPorts(currAcceleratorId, FALSE);
			WORD  free_decoder_ports     = _iiUnit->GetFreeDecoderPorts(currAcceleratorId, FALSE);

			// check that we didn't plan to allocate this unit/accelerator for the other required units
			float capacity_already_allocated_on_accelerator = 0;
			float capacity_already_allocated_on_unit        = 0;
			WORD  encoder_ports_already_allocated           = 0;
			WORD  decoder_ports_already_allocated           = 0;
			DWORD bandwidth_in_already_allocated            = 0;
			DWORD bandwidth_out_already_allocated           = 0;
			for (int otherUnitsIndex = 0; otherUnitsIndex < unitIndex; otherUnitsIndex++)
			{
				if (videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].m_UnitId == _iiUnit->GetUnitId())
				{
					bandwidth_in_already_allocated  += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededBandwidthIn();
					bandwidth_out_already_allocated += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededBandwidthOut();

					capacity_already_allocated_on_unit += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededCapacity();

					if (videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].m_MediaPortsList[0].m_acceleratorId == currAcceleratorId)
					{
						capacity_already_allocated_on_accelerator += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededCapacity();
						encoder_ports_already_allocated           += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededVideoPortsPerType(TRUE);
						decoder_ports_already_allocated           += videoAlloc.m_VideoAlloc.m_unitsList[otherUnitsIndex].GetTotalNeededVideoPortsPerType(FALSE);
					}
				}
			}

			if (free_capacity < neededCapacity + capacity_already_allocated_on_accelerator)
				continue;                                     // rejected - not enough free_capacity

			if ((neededEncoderPorts > 0 && all_free_encoder_ports < neededEncoderPorts + encoder_ports_already_allocated) ||
			    (neededDecoderPorts > 0 && all_free_decoder_ports < neededDecoderPorts + decoder_ports_already_allocated))
				continue;                                     // rejected - not enough free encoder/decoder ports

			// in this stage we know that we have enough free encoder/decoder ports but they might contain ports with status PORT_FREE_DISABLED.
			// check if this unit has enough free encoder/decoder ports with status PORT_FREE.
			BOOL currUnitContainsFreeDiabledPorts = FALSE;
			if ((neededEncoderPorts > 0 && free_encoder_ports < neededEncoderPorts + encoder_ports_already_allocated) ||
			    (neededDecoderPorts > 0 && free_decoder_ports < neededDecoderPorts + decoder_ports_already_allocated))
				currUnitContainsFreeDiabledPorts = TRUE;

			// check if we have enough free bandwidth in (for encoders)
			if (TOTAL_POSTSCALER_BW_IN < neededBandwidthIn + utilized_bandwidth_in + bandwidth_in_already_allocated)
				continue;                                     // rejected - not enough free bandwidth in

			// check if we have enough free bandwidth out (for decoders)
			if (TOTAL_POSTSCALER_BW_OUT < neededBandwidthOut + utilized_bandwidth_out + bandwidth_out_already_allocated)
				continue;                                     // rejected - not enough free bandwidth out

			BOOL betterAcceleratorWasFound      = FALSE;
			BOOL currUnitIsPreferedForSpreading = FALSE;

			// For Ninja spreading - Prefer to allocate on 66.6% unit first (2 full accelerators), and then go to the next free unit.
			// After all Netra units are 66.6% occupied, start allocating on the 3rd accelerator (or on the rest of 33.3% of the unit in case of fragmentation).
			// Reserve one video unit for unit swap recovery, so allocate on it as last priority.
			if (bSpreadVideoPortsOnNetraUnits)
			{
				if (free_unit_capacity - neededCapacity - capacity_already_allocated_on_unit >= NETRA_PORTS_SPREADING_FREE_CAPACITY_THRESHOLD)
					currUnitIsPreferedForSpreading = TRUE;

				if (selectedUnitIsPreferedForSpreading && !currUnitIsPreferedForSpreading &&
				    (_iiFoundVideoUnit != pMediaUnitslist->end() && _iiFoundVideoUnit->GetUnitId() != reservedUnitIdForSwapRecovery))
					continue;                                     // rejected - we already found a better unit/accelerator for spreading.
			}

			// For Ninja spreading - If we already found a candidate accelerator/unit, but it is not prefered for Netra spreading,
			// and the current acceleretor/unit is better for spreading, then choose the current accelerator.
			// Reserve one video unit for unit swap recovery, so allocate on it as last priority.
			if (bSpreadVideoPortsOnNetraUnits && (_iiFoundVideoUnit != pMediaUnitslist->end()) &&
			    ((currUnitIsPreferedForSpreading && !selectedUnitIsPreferedForSpreading) ||
			     (_iiFoundVideoUnit->GetUnitId() == reservedUnitIdForSwapRecovery && _iiUnit->GetUnitId() != reservedUnitIdForSwapRecovery)))
			{
				betterAcceleratorWasFound = TRUE;
			}
			else if (neededBandwidthIn > neededBandwidthOut)
			{
				if (selectedUnitUtilizedBandwidthOut < utilized_bandwidth_out + bandwidth_out_already_allocated)
				{
					betterAcceleratorWasFound = TRUE;
				}
				else if (selectedUnitUtilizedBandwidthOut == utilized_bandwidth_out + bandwidth_out_already_allocated &&
				         // check if we found better accelerator with less free capacity
				         free_capacity - capacity_already_allocated_on_accelerator < min_capacity)
				{
					betterAcceleratorWasFound = TRUE;
				}
				else if (selectedUnitUtilizedBandwidthOut == utilized_bandwidth_out + bandwidth_out_already_allocated &&
				         free_capacity - capacity_already_allocated_on_accelerator == min_capacity &&
				         // check if we have a better unit without ports with status PORT_FREE_DISABLED
				         (_iiFoundVideoUnit != pMediaUnitslist->end() && selectedUnitContainsFreeDiabledPorts && !currUnitContainsFreeDiabledPorts))
				{
					betterAcceleratorWasFound = TRUE;
				}
			}
			else //if (neededBandwidthOut >= neededBandwidthIn)
			{
				if (selectedUnitUtilizedBandwidthIn < utilized_bandwidth_in + bandwidth_in_already_allocated)
				{
					betterAcceleratorWasFound = TRUE;
				}
				else if (selectedUnitUtilizedBandwidthIn == utilized_bandwidth_in + bandwidth_in_already_allocated &&
				         // check if we found better accelerator with less free capacity
				         free_capacity - capacity_already_allocated_on_accelerator < min_capacity)
				{
					betterAcceleratorWasFound = TRUE;
				}
				else if (selectedUnitUtilizedBandwidthIn == utilized_bandwidth_in + bandwidth_in_already_allocated &&
				         free_capacity - capacity_already_allocated_on_accelerator == min_capacity &&
				         // check if we have a better unit without ports with status PORT_FREE_DISABLED
				         (_iiFoundVideoUnit != pMediaUnitslist->end() && selectedUnitContainsFreeDiabledPorts && !currUnitContainsFreeDiabledPorts))
				{
					betterAcceleratorWasFound = TRUE;
				}
			}

			if (betterAcceleratorWasFound)
			{
				selectedUnitUtilizedBandwidthOut     = utilized_bandwidth_out + bandwidth_out_already_allocated;
				selectedUnitUtilizedBandwidthIn      = utilized_bandwidth_in + bandwidth_in_already_allocated;
				min_capacity                         = free_capacity - capacity_already_allocated_on_accelerator;
				selectedUnitContainsFreeDiabledPorts = currUnitContainsFreeDiabledPorts;
				_iiFoundVideoUnit                    = _iiUnit;
				selectedAcceleratorId                = currAcceleratorId;
				selectedUnitIsPreferedForSpreading   = currUnitIsPreferedForSpreading;
			}
		}
	}

	if (_iiFoundVideoUnit == pMediaUnitslist->end())  // not found at all, fail
	{
		// We couldn't find available unit
		msg << "Failed to find suitable unit - insufficient video resources";
		PTRACE(eLevelInfoNormal, msg.str().c_str());
		return STATUS_INSUFFICIENT_VIDEO_RSRC;    // status fail
	}

	if (min_capacity < 1000)                          // found unit fragmented
	{
		msg << "Found suitable fragmented unit, unit_id:" << _iiFoundVideoUnit->GetUnitId() << ", accelerator_id:" << selectedAcceleratorId;
		videoAlloc.m_bFragmentedUnit = TRUE;
	}
	else
	{
		msg << "Found suitable empty unit, unit_id:" << _iiFoundVideoUnit->GetUnitId() << ", accelerator_id:" << selectedAcceleratorId;
	}

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	// fill parameters found for return
	const CUnitMFA* pUnit = (&(*_iiFoundVideoUnit));
	videoAlloc.m_VideoAlloc.m_boardId                       = pUnit->GetBoardId();
	videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].m_UnitId = pUnit->GetUnitId();
	videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].SetAcceleratorIdForAllPorts(selectedAcceleratorId);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::CollectRTMAvailabilityForBoard(RTMDataPerBoardStruct& rtmAllocStruct, PartyDataStruct& partyData, int boardId)
{
	int zeroBasedBoardId = boardId - 1;

	DWORD numFree, numDialOutReserved;
	STATUS status = m_pNetServicesDB->CountPortsPerBoard(*(partyData.m_pIsdn_Params_Request), numFree, numDialOutReserved, zeroBasedBoardId);
	if (status != STATUS_OK)
		return;

	rtmAllocStruct.m_numFreeRTMPorts = numFree - numDialOutReserved;

	if (rtmAllocStruct.m_numFreeRTMPorts > 0)
		rtmAllocStruct.m_bCanBePartiallyAllocated = TRUE;

	if (rtmAllocStruct.m_numFreeRTMPorts >= partyData.m_pIsdn_Params_Request->num_of_isdn_ports)
		rtmAllocStruct.m_bCanBeAllocated = TRUE;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetBestAllocationForRTMOnly(PartyDataStruct& partyData, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, int numOfRTMPorts, int notOnThisBoard, int preferablyOnThisBoard)
{
	//this function is for realllocate upgrade and board full
	//For reallocate upgrade, we will try to allocate first on the board preferablyOnThisBoard
	//For board full we won't allocate on notOnThisBoard

	//we will use the partyData structure, in order to find the appropriate spans and board
	//but we will change the num_of_isdn_ports, so keep it
	int originalNumOfRTMPorts = partyData.m_pIsdn_Params_Request->num_of_isdn_ports;
	partyData.m_pIsdn_Params_Request->num_of_isdn_ports = numOfRTMPorts;

	DataPerBoardStruct* pAllocDataPerBoardArray = new DataPerBoardStruct[BOARDS_NUM];

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		memset(&(pAllocDataPerBoardArray[i]), 0, sizeof(DataPerBoardStruct));
		if (!IsBoardIdExists(i + 1))
			continue;

		CollectRTMAvailabilityForBoard(pAllocDataPerBoardArray[i].m_RTMData, partyData, i + 1); //our array is zero-based but all the code is 1-based
	}

	STATUS status = m_pAllocationDecider->DecideAboutBestRTMBoards(partyData, pIsdn_Params_Response, pAllocDataPerBoardArray, notOnThisBoard, preferablyOnThisBoard);

	delete[] pAllocDataPerBoardArray;

	//set the num_of_isdn_ports back to what it was
	partyData.m_pIsdn_Params_Request->num_of_isdn_ports = originalNumOfRTMPorts;

	return status;
}

////////////////////////////////////////////////////////////////////////////
char* CSystemResources::GetServiceName(ISDN_SPAN_PARAMS_S& isdn_params)
{
	char* ServiceName = new char[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	WORD len = strlen((char*)isdn_params.serviceName);

	if (RTM_ISDN_SERVICE_PROVIDER_NAME_LEN < len)
	{
		len = RTM_ISDN_SERVICE_PROVIDER_NAME_LEN;
		PASSERT(len);
	}
	strncpy(ServiceName, (char*)isdn_params.serviceName, len);
	ServiceName[len] = '\0';
	return ServiceName;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::RemoveCard(BoardID boardId, SubBoardID subBoardId)
{
	#undef PARAMS
	#define PARAMS "BoardId:" << boardId << ", SubBoardId:" << subBoardId

	CBoard* pBoard = GetBoard(boardId);
	PASSERTSTREAM_AND_RETURN(!pBoard, PARAMS);

	WORD numOfAudPortsPerUnit = pBoard->CalculateNumberOfPortsPerUnit(PORT_ART);
	WORD numOfVidPortsPerUnit = pBoard->CalculateNumberOfPortsPerUnit(PORT_VIDEO);

	//////////////////////////////////
	//remove all MFA units
	//////////////////////////////////
	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	std::set<CUnitMFA>::iterator unitIterator = pMediaUnitslist->begin();
	std::set<CUnitMFA>::iterator tempUnitIterator;

	while (unitIterator != pMediaUnitslist->end())
	{
		tempUnitIterator = unitIterator++;  //increment it before changing it or else there might be problems after erasing it

		//if we are removing the MFA card, then we also remove all of it's other subboards
		if (subBoardId != MFA_SUBBOARD_ID && subBoardId != tempUnitIterator->GetSubBoardId())
			continue;

		//found it
		switch (tempUnitIterator->GetUnitType())
		{
			case eUnitType_Art:
			{
				m_num_configured_ART_Ports[boardId - 1] -= numOfAudPortsPerUnit;
				CCardResourceConfig::SetNumAudPortsLeftToConfig(CCardResourceConfig::GetNumAudPortsLeftToConfig() + numOfAudPortsPerUnit);
				break;
			}
			case eUnitType_Video:
			{
				m_num_configured_VIDEO_Ports[boardId - 1] -= numOfVidPortsPerUnit;
				CCardResourceConfig::SetNumVidHD720PortsLeftToConfig(CCardResourceConfig::GetNumVidHD720PortsLeftToConfig() + numOfVidPortsPerUnit);
				break;
			}
			case eUnitType_Generic:
			case eUnitType_Rtm:
			case eUnitType_Art_Control:
				break;
			default:
				TRACEINTOLVLERR << PARAMS << ", UnitType:" << tempUnitIterator->GetUnitType() << " - Unknown unit type";
				break;
		}
		pMediaUnitslist->erase(tempUnitIterator);
	}

	///////////////////////////////////////////////////////////////////////////////
	//print in trace the number of aud/video ports left to configure
	///////////////////////////////////////////////////////////////////////////////
	TRACEINTO << PARAMS << ", NumVidHD720PortsLeftToConfig:" << CCardResourceConfig::GetNumVidHD720PortsLeftToConfig() << ", NumAudPortsLeftToConfig:" << CCardResourceConfig::GetNumAudPortsLeftToConfig();

	//////////////////////////////////
	//remove all spans
	//////////////////////////////////
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	std::set<CSpanRTM>::iterator spansIterator = pSpanslist->begin();
	std::set<CSpanRTM>::iterator tempSpansIterator;

	while (spansIterator != pSpanslist->end())
	{
		tempSpansIterator = spansIterator++;   //increment it before changing it or else there might be problems after erasing it

		if (boardId != tempSpansIterator->GetBoardId())
			continue;
		//if we are removing the MFA card, then we also remove all of it's other subboards
		if (subBoardId != MFA_SUBBOARD_ID && subBoardId != tempSpansIterator->GetSubBoardId())
			continue;

		m_pNetServicesDB->SpanRemoved((CSpanRTM*)(&(*tempSpansIterator)));

		pSpanslist->erase(tempSpansIterator);
	}

	//////////////////////////////////
	///remove the list of channel ids
	//////////////////////////////////
	pBoard->RemoveChannelsIdsList();

	//////////////////////////////////
	///Set Audio Controller Unit ID back to default
	//////////////////////////////////
	pBoard->SetAudioControllerUnitId(NO_AC_ID);

	if (MFA_SUBBOARD_ID == subBoardId)
		pBoard->SetCardsStartup(MFA_COMPLETE, 0);  //VNGR-16903

	pBoard->SetACType(E_NORMAL);
}

////////////////////////////////////////////////////////////////////////////
BYTE CSystemResources::DisableAllUnitsAndSpans(WORD boardId, WORD subBoardId)
{
	if (!CHelperFuncs::IsValidBoardId(boardId))
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::DisableAllUnitsAndSpans: Invalid boardid: ", boardId);
		PASSERT(1);
		return FALSE;
	}

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, FALSE);
	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	//disable all units
	BYTE isBoardIdExist = FALSE;
	BYTE returnValue = FALSE;
	std::set<CUnitMFA>::iterator i;

	for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
	{
		isBoardIdExist = TRUE;  // Means that we are in case removing RTM only
		//if we are disabling the MFA card, then we also remove all of it's other sub-boards
		if (subBoardId != MFA_SUBBOARD_ID && subBoardId != i->GetSubBoardId())
			continue;

		//found
		SetUnitMfaStatus(boardId, i->GetUnitId(), FALSE);
		returnValue = TRUE;
	}

	//disable all spans
	RTM_ISDN_BOARD_ID_S* pIsdnBId = new RTM_ISDN_BOARD_ID_S;
	pIsdnBId->boardId = boardId;
	pIsdnBId->subBoardId = subBoardId;
	DisableAllRtmSpanPerBoard(pIsdnBId);
	POBJDELETE(pIsdnBId);

	if (MFA_SUBBOARD_ID != subBoardId && TRUE == isBoardIdExist)
		returnValue = TRUE;

	return returnValue;
}

////////////////////////////////////////////////////////////////////////////
BYTE CSystemResources::IsThereExistUtilizableUnitForAudioController()
{
	// We set the master audio cntrlr only if the unit is utilizable,
	// hence we have a utilizable unit if and only if we have the
	// master audio controller set.

	// Ohad, 03/08 : The function is still correct also for Barak
	if (GetAudioCntrlMasterBid() != 0xFFFF)
		return TRUE;
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
// Function treats HowSwap controllers (Master/Slave & Ivr) in case of adding a card (startup or insert)
STATUS CSystemResources::HotSwapIvrAndAudioControllersAdd( DWORD boardId,
                                                           BYTE& wasAudioControllerMasterUpdated,
                                                           BYTE& wasAudioControllerSlaveUpdated,
                                                           CConnToCardManager* pConnToCardManager)
{
	STATUS status = STATUS_OK;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);
	WORD audioCntrlrUnitId = pBoard->GetAudioControllerUnitId();
	FTRACEINTO << " boardId = " << boardId << ", audioCntrlrUnitId = " << (DWORD)audioCntrlrUnitId;   //olga

	CUnitMFA* pUnitMFA = (CUnitMFA*)pBoard->GetMFA(audioCntrlrUnitId);
	eProductType prodType = GetProductType();
	WORD l_slaveBid = 0xFFFF;

	if (pUnitMFA && pUnitMFA->GetIsEnabled())
	{
		WORD l_masterBid = GetAudioCntrlMasterBid();
		if ((eProductTypeRMX2000 == prodType) || (eProductTypeCallGenerator == prodType))
			l_slaveBid = GetAudioCntrlSlaveBid();
		else
			l_slaveBid = ((pBoard->GetACType() == E_AC_RESERVED) ? boardId : 0xFFFF);

		if (l_masterBid == 0xFFFF && l_slaveBid != boardId) /* the 2nd condition is a protection for a case that same bId was sent more than once */
		{
			// Update shared memory
			DWORD connId = AllocateConnId();

			if (connId == 0)
				PASSERT(1);

			else
			{
				STATUS cntl_status = CreateControllerRecord(boardId, audioCntrlrUnitId, connId, ePhysical_audio_controller, E_AC_MASTER);

				if (cntl_status != STATUS_OK)
				{
					PASSERT(1);                  //trace inside Create.
					DeAllocateConnId(connId);
				}
				else
				{
					pUnitMFA->SetConnId(connId); // Unnecessary ???
					SetAudioCntrlMasterBid(boardId);
					pBoard->SetACType(E_AC_MASTER);
					wasAudioControllerMasterUpdated = TRUE;

					PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersAdd , Set Master AC on board = ", boardId);

					// Update IVR controller
					if (GetIvrCntrlBid() != 0xFFFF)
						PASSERT(GetIvrCntrlBid());              // Need to consider extra treatment

					DWORD connIdIvr = AllocateConnId();
					if (connIdIvr == 0)
						PASSERT(1);
					else
					{
						WORD bId = boardId;
						WORD uId = GetUtilizablePQUnitIdPerBoardId(bId);

						if (0xFFFF == uId)
						{
							PASSERT(bId);
							PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersAdd , no utilizable PQ unit for boardId = ", boardId);
						}

						STATUS cntl_status = CreateControllerRecord(bId, uId, connIdIvr, ePhysical_ivr_controller);

						if (cntl_status != STATUS_OK)
						{
							PASSERT(1);          //trace inside Create.
							DeAllocateConnId(connIdIvr);
						}

						SetIvrCntrlBid(bId);
					}
				}
			}
		}
		else
		{
			if (l_slaveBid == 0xFFFF && l_masterBid != boardId) /* the 2nd condition is a protection for a case that same bId was sent more than once */
			{
				// Update shared memory
				DWORD connId = AllocateConnId();

				if (connId == 0)
					PASSERT(1);

				else
				{
					ECntrlType rsrcCntlType = E_AC_RESERVED;
					if ((eProductTypeRMX2000 == prodType) || (eProductTypeCallGenerator == prodType))
						rsrcCntlType = E_AC_SLAVE;

					STATUS cntl_status = CreateControllerRecord(boardId, audioCntrlrUnitId, connId, ePhysical_audio_controller, rsrcCntlType);

					if (cntl_status != STATUS_OK)
					{
						PASSERT(cntl_status);                  //trace inside Create.
						DeAllocateConnId(connId);
					}

					else
					{
						pUnitMFA->SetConnId(connId);           // Unnecessary ???
						if ((eProductTypeRMX2000 == prodType) || (eProductTypeCallGenerator == prodType))
							SetAudioCntrlSlaveBid(boardId);
						else
							pBoard->SetACType(E_AC_RESERVED);

						wasAudioControllerSlaveUpdated = TRUE;
						PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersAdd , Set Slave AC on board = ", boardId);
					}
				}
			}
		}
	}
	else
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersAdd , Unit of Audio Controller is disabled on board ", boardId);

	return status;

}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetUtilizablePQUnitIdPerBoardId( WORD boardId )
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, 0xFFFF);

	return pBoard->GetUtilizablePQUnitId();
}

////////////////////////////////////////////////////////////////////////////
// Function treats HowSwap controllers (Master/Slave & Ivr) in case of updating a card
STATUS CSystemResources::HotSwapIvrAndAudioControllersUpdate()
{
	STATUS status = STATUS_OK;

	// TBD

	return status;

}

////////////////////////////////////////////////////////////////////////////
// Function treats HowSwap controllers (Master/Slave & Ivr) in case of removing a card
STATUS CSystemResources::HotSwapIvrAndAudioControllersRemove( DWORD boardId,
                                                              WORD subBoardId,
                                                              BYTE& wasAudioControllerMasterUpdated,
                                                              BYTE& wasIvrControllerUpdated,
                                                              CConnToCardManager* pConnToCardManager)
{
	STATUS status = STATUS_OK;

	WORD masterBId = GetAudioCntrlMasterBid();
	ECntrlType ac_type = E_NORMAL;
	WORD slaveBId = 0xFFFF;
	CBoard* pBoard = NULL;
	WORD oneBasedBoardId;
	eProductType prodType = GetProductType();
	WORD IvrBId = GetIvrCntrlBid();

	if (prodType == eProductTypeRMX2000)
		slaveBId = GetAudioCntrlSlaveBid();

	if (MFA_SUBBOARD_ID == subBoardId)
	{
		ConnToCardTableEntry Entry;
		int statusSharedMemory = STATUS_OK;

		if (masterBId == boardId)
		{
			// find reserved AC
			if (prodType == eProductTypeRMX4000)
			{
				for (WORD i = 0; i < BOARDS_NUM; i++)
				{
					oneBasedBoardId = i + 1;
					if (boardId == oneBasedBoardId || !IsBoardIdExists(oneBasedBoardId))
						continue;
					pBoard = GetBoard(oneBasedBoardId);
					ac_type = pBoard->GetACType();
					WORD acUnitID = pBoard->GetAudioControllerUnitId();
					CUnitMFA* pMFA = (CUnitMFA*)(pBoard->GetMFA(acUnitID));

					if (E_AC_RESERVED == ac_type && pMFA && TRUE == pMFA->GetIsEnabled())
					{
						slaveBId = oneBasedBoardId;
						break;
					}
				}
			}
			// For later updating/removing the IVR entry
			DWORD ivrConnId = pConnToCardManager->GetConnIdByRsrcType(ePhysical_ivr_controller);
			// For later updating/removing the Master AC entry
			DWORD masterConnId = pConnToCardManager->GetConnIdByRsrcType(ePhysical_audio_controller, E_AC_MASTER);

			if (0xFFFF != slaveBId)
			{
				PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Moved the master AC to bId = ", slaveBId);
				SetAudioCntrlMasterBid(slaveBId);
				wasAudioControllerMasterUpdated = TRUE;

				if ((prodType == eProductTypeRMX2000) || (eProductTypeCallGenerator == prodType))
				{
					SetAudioCntrlSlaveBid(0xFFFF);
					SetIvrCntrlBid(slaveBId);
				}
				else
				{
					pBoard->SetACType(E_AC_MASTER);
					if (IvrBId == masterBId)
						SetIvrCntrlBid(slaveBId);
				}

				masterBId = slaveBId;
				ECntrlType rsrcCntlType = E_AC_RESERVED;
				DWORD slaveConnId;
				// Update the slave entry to be master
				if ((eProductTypeRMX2000 == prodType) || (eProductTypeCallGenerator == prodType))
				{
					rsrcCntlType = E_AC_SLAVE;
					slaveConnId = pConnToCardManager->GetConnIdByRsrcType(ePhysical_audio_controller, rsrcCntlType);
				}
				else
					slaveConnId = pConnToCardManager->GetConnIdByRsrcTypeAndBId(ePhysical_audio_controller, rsrcCntlType, slaveBId);

				statusSharedMemory = pConnToCardManager->Get(slaveConnId, Entry);
				if (STATUS_OK == statusSharedMemory)
				{
					Entry.rsrcCntlType = E_AC_MASTER;
					pConnToCardManager->Update(Entry);
				}
				else
				{
					PASSERT(statusSharedMemory);
					PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Couldn't get slave entry from shared memory. \nConnId = ", slaveConnId);
				}

				if ((eProductTypeRMX2000 == prodType) || (eProductTypeCallGenerator == prodType) || (eProductTypeRMX4000 == prodType && IvrBId == boardId))
				{
					// Update the IVR controller entry with same board as master AC
					if (0xFFFF == GetUtilizablePQUnitIdPerBoardId(masterBId)) // in case RMX4000 coude be found another.TBD
					{
						PASSERT(masterBId);
						PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, no utilizable PQ on board = ", masterBId);
					}
					statusSharedMemory = pConnToCardManager->Get(ivrConnId, Entry);
					if (STATUS_OK == statusSharedMemory)
					{
						Entry.boardId = masterBId;
						pConnToCardManager->Update(Entry);
						wasIvrControllerUpdated = TRUE;
					}
					else
					{
						PASSERT(statusSharedMemory);
						PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Couldn't get IVR controller entry from shared memory. \nConnId = ", slaveConnId);
					}
				}
			}
			else    // It was the last card or the unit of Audio Controller was disabled
			{
				PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Removed the master AC from bId = ", boardId);
				SetAudioCntrlMasterBid(0xFFFF);
				SetIvrCntrlBid(0xFFFF);
				// Remove the IVR entry from Shared Memory
				if (ivrConnId != EMPTY_ENTRY)
				{
					pConnToCardManager->Remove(ivrConnId);
					DeAllocateConnId(ivrConnId);
				}
				else
				{
					PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Couldn't get IVR controller entry from shared memory. \nConnId = ", ivrConnId);
				}

				masterBId = 0xFFFF;
			}

			// Remove master entry from shared memory + deallocate connId
			if (masterConnId != EMPTY_ENTRY)
			{
				pConnToCardManager->Remove(masterConnId);
				DeAllocateConnId(masterConnId);
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Couldn't get master entry from shared memory. \nConnId = ", masterConnId);
			}
		}
		else //if ( slaveBId == boardId )
		{
			PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Removed the slave AC from bId = ", boardId);
			DWORD slaveConnId;
			if ((eProductTypeRMX2000 == prodType) || (eProductTypeCallGenerator == prodType))
			{
				SetAudioCntrlSlaveBid(0xFFFF);
				slaveConnId = pConnToCardManager->GetConnIdByRsrcType(ePhysical_audio_controller, E_AC_SLAVE);
				// Remove from shared memory and deallocate connId
				if (slaveConnId != EMPTY_ENTRY)
				{
					pConnToCardManager->Remove(slaveConnId);
					DeAllocateConnId(slaveConnId);
				}
				else
					PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Couldn't get slave/reserved entry from shared memory. \nConnId = ", slaveConnId);
			}
			else // in case eProductTypeRMX4000 we'll set IVRcntrl on the board with AC Master.
			{
				if (0xFFFF != masterBId)
				{
					slaveConnId = pConnToCardManager->GetConnIdByRsrcTypeAndBId(ePhysical_audio_controller, E_AC_RESERVED, boardId);
					if (IvrBId == boardId)
					{
						DWORD ivrConnId = pConnToCardManager->GetConnIdByRsrcType(ePhysical_ivr_controller);
						statusSharedMemory = pConnToCardManager->Get(ivrConnId, Entry);
						if (STATUS_OK == statusSharedMemory)
						{
							Entry.boardId = masterBId; // assume that master is OK
							pConnToCardManager->Update(Entry);
							SetIvrCntrlBid(masterBId);
							wasIvrControllerUpdated = TRUE;
						}
						else
						{
							PASSERT(statusSharedMemory);
							PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Couldn't get IVR controller entry from shared memory. \nConnId = ", slaveConnId);
						}
					}
					// Remove from shared memory and deallocate connId
					if (slaveConnId != EMPTY_ENTRY)
					{
						pConnToCardManager->Remove(slaveConnId);
						DeAllocateConnId(slaveConnId);
					}
					else
						PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Couldn't get slave/reserved entry from shared memory. \nConnId = ", slaveConnId);
				}
				else //masterBId == 0xFFFF
				{
					//NOTE: we don't consider a case when Reserved AC exists (only as error handling)

					pBoard = GetBoard(boardId);
					ac_type = (NULL != pBoard) ? pBoard->GetACType() : E_NORMAL;
					DWORD connIdToRemove = EMPTY_ENTRY;
					if (E_AC_MASTER == ac_type)
						connIdToRemove = pConnToCardManager->GetConnIdByRsrcType(ePhysical_audio_controller, E_AC_MASTER);
					else if (E_AC_RESERVED == ac_type)
						connIdToRemove = pConnToCardManager->GetConnIdByRsrcTypeAndBId(ePhysical_audio_controller, E_AC_RESERVED, boardId);
					else
					PASSERTSTREAM(TRUE, "ac_type " << (DWORD)ac_type << " is not as expected?.");

					if (connIdToRemove != EMPTY_ENTRY)
					{
						statusSharedMemory = pConnToCardManager->Get(connIdToRemove, Entry);
						if (boardId == Entry.boardId)
						{
							pConnToCardManager->Remove(connIdToRemove);
							DeAllocateConnId(connIdToRemove);
						}
						else
							PASSERT(boardId);
					}

					// IVR treatment
					if (IvrBId == boardId)
					{
						DWORD ivrConnId = pConnToCardManager->GetConnIdByRsrcType(ePhysical_ivr_controller);
						statusSharedMemory = pConnToCardManager->Get(ivrConnId, Entry);
						if (STATUS_OK == statusSharedMemory)
						{
							WORD anotherBoardId = 0xFFFF;
							for (WORD i = 0; i < BOARDS_NUM; i++)
							{
								oneBasedBoardId = i + 1;
								if (boardId == oneBasedBoardId || !IsBoardIdExists(oneBasedBoardId))
									continue;
								if (0xFFFF != GetUtilizablePQUnitIdPerBoardId(oneBasedBoardId))
									anotherBoardId = oneBasedBoardId;
							}
							if (0xFFFF == anotherBoardId)
							{
								SetIvrCntrlBid(0xFFFF);
								// Remove the IVR entry from Shared Memory
								if (ivrConnId != EMPTY_ENTRY)
								{
									pConnToCardManager->Remove(ivrConnId);
									DeAllocateConnId(ivrConnId);
								}
							}
							else
							{
								Entry.boardId = anotherBoardId; // assume that master is OK
								pConnToCardManager->Update(Entry);
								SetIvrCntrlBid(anotherBoardId);
								wasIvrControllerUpdated = TRUE;
							}
						}
						else
							PTRACE2INT(eLevelInfoNormal, "CSystemResources::HotSwapIvrAndAudioControllersRemove, Couldn't get IVR controller entry from shared memory. \nConnId = ", ivrConnId);
					}
				}
			}
		}
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
// The function returns the capacity of a given card by unit id or the capacity of a unit (if a unit id is given)
// The output is in promils terms in case of MFA and in parties terms in case of RTM
WORD CSystemResources::GetCapacity(DWORD boardId, WORD subBoardId, WORD unitId)
{
	float capacity = 0;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, (WORD )capacity);

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
	{

		if (MFA_SUBBOARD_ID != subBoardId)   // RTM
		{
			CSpanRTM* pRTM = (CSpanRTM*)(pBoard->GetRTM(i + 1));
			if (!pRTM)
				continue;

			if (pRTM->GetIsAllocated())
			{
				if (pRTM->GetBoardId() == boardId && pRTM->GetSubBoardId() == subBoardId)
				{
					if (unitId != 0xFFFF)
					{
						if (pRTM->GetUnitId() == unitId)
						{
							capacity = pRTM->GetNumPorts() - pRTM->GetNumFreePorts();
							break;
						}
					}
					else
						capacity += (pRTM->GetNumPorts() - pRTM->GetNumFreePorts());
				}
			}
		}
		else // MFA
		{

			CUnitMFA* pMFA = (CUnitMFA*)(pBoard->GetMFA(i + 1));
			if (!pMFA)
				continue;

			if (pMFA->GetIsAllocated())
			{
				if (pMFA->GetBoardId() == boardId && pMFA->GetSubBoardId() == subBoardId)
				{
					eCardType cardType = pBoard->GetCardType();
					WORD acceleratorsNum = CHelperFuncs::IsMpmRx(cardType) ? ACCELERATORS_PER_UNIT_NETRA : 1;

					if (pMFA->GetUnitId() == unitId || unitId == 0xFFFF)
					{
						for (int i = 0; i < acceleratorsNum; i++)
						{
							capacity += 1000 - pMFA->GetFreeCapacity(i);
						}

						if (pMFA->GetUnitId() == unitId)
							break;
					}
				}
			}
		}
	}

	return (WORD)capacity;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSystemResources::IsBetterBandWidth(DWORD needed_bandwidth_in, DWORD needed_bandwidth_out, DWORD current_bandwidth_in, DWORD current_bandwidth_out, DWORD max_bandwidth_in, DWORD max_bandwidth_out)
{
	if (needed_bandwidth_in == 0) //if only need out, then check only out
	{
		if (current_bandwidth_out > max_bandwidth_out)
			return TRUE;
		else
			return FALSE;
	}
	else if (needed_bandwidth_out == 0) //if only need in, then check only in
	{
		if (current_bandwidth_in > max_bandwidth_in)
			return TRUE;
		else
			return FALSE;
	}
	else //need both, check the sum of both
	{
		if (current_bandwidth_in + current_bandwidth_out > max_bandwidth_in + max_bandwidth_out)
			return TRUE;
		else
			return FALSE;
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CSystemResources::IsThereAnyParty()
{
	return m_ResourcesInterfaceArray.IsThereAnyParty();
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetDongleRestriction(DWORD num_parties)
{
	//  PTRACE(eLevelInfoNormal,"RSRV_LOG: CSystemResources::SetDongleRestriction");
	m_ResourcesInterfaceArray.InitDongleRestriction(num_parties);

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator && CHelperFuncs::IsMode2C() && CHelperFuncs::IsAutoModeAllocationType())
		m_maxNumberOfOngoingConferences = pReservator->GetMaxNumberOngoingConferences2C(FALSE);

	TRACEINTO << "MaxNumberOfOngoingConferences:" << m_maxNumberOfOngoingConferences;
}

////////////////////////////////////////////////////////////////////////////
ResourcesInterface* CSystemResources::GetCurrentResourcesInterface() const
{
	return m_ResourcesInterfaceArray.GetCurrentResourcesInterface();
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetOrCheckEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg, CEnhancedConfigResponse* pResponse) const
{
	//illegal during startup
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessInvalid == curStatus) || (eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		FTRACEINTO << "return status STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP ";

		return STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP;
	}

	//illegal when not in fixed mode
	if (!CHelperFuncs::IsFixedModeAllocationType())
		//tbd zoe - breeze - maybe new status
		return STATUS_ILLEGAL_IN_NOT_FIXED_MPM_PLUS_MODE;

	ResourcesInterface* pCurrentResourcesInterface = GetCurrentResourcesInterface();
	PASSERT_AND_RETURN_VALUE(pCurrentResourcesInterface == NULL, STATUS_FAIL);

	CFixedModeResources* pFixedModeResources = dynamic_cast<CFixedModeResources*>(pCurrentResourcesInterface);
	PASSERT_AND_RETURN_VALUE(pFixedModeResources == NULL, STATUS_FAIL);

	return pFixedModeResources->GetOrCheckEnhancedConfiguration(pEnhancedCfg, pResponse);
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::SetEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg)
{
	//illegal when not in fixed mode
	if (!CHelperFuncs::IsFixedModeAllocationType())
	{
		//tbd zoe - breeze - maybe new status
		return STATUS_ILLEGAL_IN_NOT_FIXED_MPM_PLUS_MODE;
	}

	STATUS ret_val = STATUS_OK;
	ret_val = m_ResourcesInterfaceArray.SetEnhancedConfiguration(pEnhancedCfg);
	if (ret_val != STATUS_OK)
	{
		return ret_val;
	}
	// Tsahi - fix bug: when clicking OK in Video/Audio Port Configuration screen while in Fixed Mode, we get Failure Status message.
	if (GetMultipleIpServices())
	{
		std::set<CIpServiceResourcesInterfaceArray>::iterator interface_itr;
		for (interface_itr = m_IpServicesResourcesInterface->begin(); interface_itr != m_IpServicesResourcesInterface->end(); interface_itr++)
		{
			DWORD st = ((CIpServiceResourcesInterfaceArray*)(&(*interface_itr)))->SetEnhancedConfiguration(pEnhancedCfg);
			if (ret_val == STATUS_OK && st != STATUS_OK)
			{
				ret_val = st;
			}
		}
	}
	return ret_val;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::OnReconfigureUnitsTimer()
{
	ResourcesInterface* pCurrentResourcesInterface = GetCurrentResourcesInterface();
	PASSERT_AND_RETURN(pCurrentResourcesInterface == NULL);

	CBaseModeResources* pPureModeResources = dynamic_cast<CBaseModeResources*>(pCurrentResourcesInterface);
	PASSERT_AND_RETURN(pPureModeResources == NULL);

	pPureModeResources->OnReconfigureUnitsTimer();
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::FineTuneUnitsConfiguration()
{
	if (!CHelperFuncs::IsFixedModeAllocationType())
		return;

	ResourcesInterface*  pCurrentResourcesInterface = GetCurrentResourcesInterface();
	PASSERT_AND_RETURN(pCurrentResourcesInterface == NULL);

	CFixedModeResources* pFixedModeResources = dynamic_cast<CFixedModeResources*>(pCurrentResourcesInterface);
	PASSERT_AND_RETURN(pFixedModeResources == NULL);

	pFixedModeResources->FineTuneUnitsConfiguration();
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CheckEnhancedConfigurationWithCurUnitsConfig()
{
	STATUS status = STATUS_OK;
	if (CHelperFuncs::IsFixedModeAllocationType())
	{
		ResourcesInterface* pCurrentResourcesInterface = GetCurrentResourcesInterface();
		PASSERT_AND_RETURN_VALUE(pCurrentResourcesInterface == NULL, STATUS_FAIL);
		CFixedModeResources* pFixedModeResources = dynamic_cast<CFixedModeResources*>(pCurrentResourcesInterface);
		PASSERT_AND_RETURN_VALUE(pFixedModeResources == NULL, STATUS_FAIL);
		if (FALSE == pFixedModeResources->CheckEnhancedConfigurationWithCurUnitsConfig())
			status = STATUS_FAIL;
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::CheckSetEnhancedConfiguration()
{
	if (CHelperFuncs::IsFixedModeAllocationType())
	{
		m_ResourcesInterfaceArray.CheckSetEnhancedConfiguration();

		std::set<CIpServiceResourcesInterfaceArray>::iterator interface_itr;
		for (interface_itr = m_IpServicesResourcesInterface->begin(); interface_itr != m_IpServicesResourcesInterface->end(); interface_itr++)
		{
			((CIpServiceResourcesInterfaceArray*)(&(*interface_itr)))->CheckSetEnhancedConfiguration();
		}
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CSystemResources::CanUseReconfigurationForAllocation(PartyDataStruct& partyData)
{
	// Tsahi - This feature is disabled for MPMx and MPM-Rx
	return FALSE;

	if (partyData.m_allowReconfiguration && CHelperFuncs::IsAutoModeAllocationType() && m_bIsRsrcEnough == TRUE)
		return TRUE;
	else
		return FALSE;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetAllocationMode(CAllocationModeDetails* pAllocationModeResponse)
{
	switch (m_ResourceAllocationType)
	{
		case eAutoMixedMode:
			pAllocationModeResponse->SetModes(eAllocationModeNone, eAllocationModeNone);
			return STATUS_ILLEGAL_IN_MPM_MODE;         // Tsahi TODO: change this to "illegal in Mixed Mode" status
		case eFixedBreezeMode:
		case eFixedMpmRxMode:
			pAllocationModeResponse->SetModes(eAllocationModeFixed, m_FutureMode);
			return STATUS_OK;
		case eAutoBreezeMode:
		case eAutoMpmRxMode:
			pAllocationModeResponse->SetModes(eAllocationModeAuto, m_FutureMode);
			return STATUS_OK;
		default:
			PASSERT(1);
			return STATUS_FAIL;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::SetAllocationMode(CAllocationModeDetails* pAllocationModeRequest)
{
	STATUS status = STATUS_OK;
	switch (m_ResourceAllocationType)
	{
		case eAutoMixedMode:
		{
			return STATUS_ILLEGAL_IN_MPM_MODE;         // Tsahi TODO: change this to "illegal in Mixed Mode" status
		}
		case eFixedMpmRxMode:
		case eAutoMpmRxMode:
		case eFixedBreezeMode:
		case eAutoBreezeMode:
		{
			status = m_ResourcesInterfaceArray.CanSetConfigurationNow();
			if (status != STATUS_OK)
				return status;

			// Since V8.1 Fixed Mode is not supported.
				#if USE_FLIXIBILE_RESOURCE_CAPACITY_ONLY
			TRACEINTO << "Since V8.1 Fixed Mode is not supported. Operation is blocked";
			if (pAllocationModeRequest->GetMode() == eAllocationModeFixed)
				return OPERATION_BLOCKED;
				#endif

			status = pAllocationModeRequest->WriteToProcessSetting();
			if (status != STATUS_OK)
				return status;

			eResourceAllocationTypes newResourceAllocationType = eNoMode;

			m_FutureMode = pAllocationModeRequest->GetMode();
			//if future mode is different than current mode
			if ((m_ResourceAllocationType == eFixedBreezeMode && m_FutureMode == eAllocationModeAuto) ||
					(m_ResourceAllocationType == eAutoBreezeMode && m_FutureMode == eAllocationModeFixed))
			{
				if (m_FutureMode == eAllocationModeFixed)
					newResourceAllocationType = eFixedBreezeMode;
				else
					newResourceAllocationType = eAutoBreezeMode;
			}
			else if ((m_ResourceAllocationType == eFixedMpmRxMode && m_FutureMode == eAllocationModeAuto) ||
							 (m_ResourceAllocationType == eAutoMpmRxMode && m_FutureMode == eAllocationModeFixed))
			{
				if (m_FutureMode == eAllocationModeFixed)
					newResourceAllocationType = eFixedMpmRxMode;
				else
					newResourceAllocationType = eAutoMpmRxMode;
			}

			if (newResourceAllocationType != eNoMode)
			{
				status = m_ResourcesInterfaceArray.ChangeResourceAllocationMode(newResourceAllocationType);
				if (GetMultipleIpServices())          //VNGR-18905
					ChangeResourceAllocationModePerIpServices(newResourceAllocationType);

				if (status == STATUS_OK)
				{
					m_ResourceAllocationType = newResourceAllocationType;

					//check if now resources are enough, only if not being now in reconfiguration
					if (m_ResourcesInterfaceArray.CanSetConfigurationNow() != STATUS_SYSTEM_BUSY_SETTING_LAST_CONFIGURATION)
					{
						TRACEINTO << "CSystemResources::SetAllocationMode calling CheckResourceEnoughAndAddOrRemoveAciveAlarm";
						CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();
						PASSERT_AND_RETURN_VALUE(pResourceManager == NULL, STATUS_FAIL);
						pResourceManager->CheckResourceEnoughAndAddOrRemoveAciveAlarm();
					}
				}
			}
			break;
		}

		default:
		{
			PASSERT(1);
			status = STATUS_FAIL;
			break;
		}
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::UpdatePortWeightsTo1500Q()
{
	STATUS status = STATUS_OK;

	m_PortsConfigurationStep = CalculatePortsConfigurationStep();
	m_AudioFactor = CalculateAudioFactorStep();

	PTRACE2INT(eLevelInfoNormal, "SystemResources::UpdatePortWeightsTo1500Q m_PortsConfigurationStep=", m_PortsConfigurationStep);

	switch (m_ResourceAllocationType)
	{
		case eFixedBreezeMode:
		case eAutoBreezeMode:
			status = m_ResourcesInterfaceArray.UpdatePortWeightsTo1500Q();
			if (GetMultipleIpServices())           //VNGR-18617
			{
				std::set<CIpServiceResourcesInterfaceArray>::iterator interface_itr;
				for (interface_itr = m_IpServicesResourcesInterface->begin(); interface_itr != m_IpServicesResourcesInterface->end(); interface_itr++)
					((CIpServiceResourcesInterfaceArray*)(&(*interface_itr)))->UpdatePortWeightsTo1500Q();
			}
			break;
		default:
			PASSERT((DWORD )(m_ResourceAllocationType));
			status = STATUS_FAIL;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSystemResources::GetDongleNumOfParties() const
{
	return (m_ResourcesInterfaceArray.GetDongleNumOfParties());
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetDongleNumOfParties(DWORD dongleNumParties)
{
	m_ResourcesInterfaceArray.SetDongleNumOfParties(dongleNumParties);
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetDSPCount()
{
#if 0
	WORD dspCount;
	if (m_DspCount == -1) //dsp count invalid, get it by command
	{
		std::string ans;
		std::string cmd = "sudo " + MCU_MRMX_DIR + "/bin/ninja_diag/ninja_diag GetDspNum | grep ninja_dspCount | awk '{ print $2 }'";

		STATUS      statusCheckSize = SystemPipedCommand(cmd.c_str(), ans);
		dspCount = atoi(ans.c_str());

		if ((0 < dspCount) && (dspCount <= 3 * 6)) //Ninja has 3 dsp board, and 6 dsp in very board at most
		{
			TRACEINTO << "Total dsp number is " << dspCount;
		}
		else //default value
		{
			dspCount = 18;
		}
		m_DspCount = dspCount;
	}

	return m_DspCount;
#else

	ULONG dspAliveBitmap = 0;
	ULONG dspAliveBitmapMerge = 0;
	WORD dspCount = 0;
	for (int boardId = 0; boardId < 3; boardId++)
	{
		DSPMonitorDspList dsp_unit_list;
		memset(&dsp_unit_list, 0, sizeof(dsp_unit_list));

		if (0 == GetDspMonitorStatus(dsp_unit_list, boardId))
		{
			for (int i = 0; i < dsp_unit_list.len; ++i)
			{
				if (DSP_STAT_DEAD != dsp_unit_list.status[i].isFaulty)
				{
					switch (boardId)
					{
						case 0:
							dspAliveBitmap |= (0x00000001 << (6 * 2 + dsp_unit_list.status[i].dspId));
							break;
						case 1:
							dspAliveBitmap |= (0x00000001 << dsp_unit_list.status[i].dspId);
							break;
						case 2:
							dspAliveBitmap |= (0x00000001 << (6 + dsp_unit_list.status[i].dspId));
							break;
						default:
							break;
					}
				}
			}
		}
	}

	ULONG dspLocationBitmap = GetDSPLocationBitmap();
	dspAliveBitmapMerge = dspAliveBitmap & dspLocationBitmap; //remove unused dsp

	WORD dspIdx;
	BOOL isDspLocated = FALSE;
	for (dspIdx = 0; dspIdx < NETRA_DSP_CHIP_COUNT_NINJA; dspIdx++)
	{
		isDspLocated = (dspAliveBitmapMerge >> dspIdx) % 2;
		if (isDspLocated)
			dspCount++;
	}

	if (m_DspAliveBitmap != dspAliveBitmapMerge)
	{
		TRACEINTO << "CSystemResources::GetDSPCount : dspCount: " << dspCount << " m_DspAliveBitmap changed: " << std::hex << m_DspAliveBitmap << "->" << dspAliveBitmapMerge << " (" << dspAliveBitmap << "&" << dspLocationBitmap << ")";
		m_DspAliveBitmap = dspAliveBitmapMerge;
	}

	return dspCount;

#endif
}

////////////////////////////////////////////////////////////////////////////
ULONG CSystemResources::GetDSPLocationBitmap()
{
	if (m_DspLocationBitmap == 0)
	{
		std::string ans;
		std::string cmd = "sudo " + MCU_MRMX_DIR + "/bin/ninja_dsp_check";

		STATUS status = SystemPipedCommand(cmd.c_str(), ans);

		if (status == STATUS_OK)
		{
			m_DspLocationBitmap = strtoul(ans.c_str(), NULL, 0);

			TRACEINTO << "Current dsp location bitmap is : " << std::hex << m_DspLocationBitmap;
		}
		else
		{
			PASSERT(121);
		}
	}

	return m_DspLocationBitmap;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetHD720PortsAccordingToCards(WORD* numBoards)
{
	CBoard* pBoard;
	int oneBasedBoardId;
	WORD numHD720Ports = 0;
	WORD numCurBoards = 0;
	for (int i = 0; i < BOARDS_NUM; i++)
	{
		oneBasedBoardId = i + 1;
		if (!IsBoardIdExists(oneBasedBoardId))
			continue;
		pBoard = GetBoard(oneBasedBoardId);
		eCardType cardType = pBoard->GetCardType();
		numCurBoards++;

		switch (cardType)
		{
			case eMpmx_80:
				numHD720Ports += 30;
				break;
			case eMpmx_40:
				numHD720Ports += 15;
				break;
			case eMpmx_20:
				// requirement: max license for 1500Q is 25 cif / 7 hd720 ports , although it capable only of 90 voice, 27 CIF, 14 SD , 7 HD720 , 3 HD 1080 (with fixed 7 video DSPs, 3 Art DSPs)
				// ( instead of 100 voice, 16 SD , 8 HD720 , 4 HD 1080 by MPMx ratio )
				numHD720Ports += 7;
				break;
			case eMpmx_Soft_Full:         //OLGA - SoftMCU
			case eMpmx_Soft_Half:
				if (eProductTypeSoftMCU == m_ProductType || eProductTypeGesher == m_ProductType)
					numHD720Ports = MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU;
				else if (eProductTypeEdgeAxis == m_ProductType)
				{
					numHD720Ports = IsSystemCpuProfileDefined() ? m_CpuSizeDesc.m_CpuCapacity : MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU;
					TRACEINTO << "eProductTypeEdgeAxis : m_CpuSize=" << m_CpuSizeDesc.m_CpuSize << ", m_CpuCapacity=" << m_CpuSizeDesc.m_CpuCapacity << ", numHD720Ports=" << numHD720Ports;
				}
				else if (eProductTypeSoftMCUMfw == m_ProductType)
				{
					numHD720Ports = IsSystemCpuProfileDefined() ? m_CpuSizeDesc.m_CpuCapacity : MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW;
					TRACEINTO << "eProductTypeSoftMCUMfw : m_CpuSize=" << m_CpuSizeDesc.m_CpuSize << ", m_CpuCapacity=" << m_CpuSizeDesc.m_CpuCapacity << ", numHD720Ports=" << numHD720Ports;
				}
				else if (eProductTypeCallGeneratorSoftMCU == m_ProductType)
				{
					numHD720Ports += MAX_NUMBER_HD_AVC_PARTIES_CALL_GENERATOR_SOFT_MCU;
				}
				else
					// AKASH - temp : need to change according to SRS
					numHD720Ports += 10;
				break;
			case eMpmRx_Ninja:
			{
				WORD ninjaDspCount = GetDSPCount(); //every DSP support 6 HD720 video; Ninja has 1-3 DSP cards, each one with 6 DSPs.
				if (ninjaDspCount > 0 && ninjaDspCount <= 6)
				{
					if (6 == ninjaDspCount)
						numHD720Ports = 35;
					else
						numHD720Ports = 6 * ninjaDspCount;
				}
				else if (ninjaDspCount > 6 && ninjaDspCount <= 12)
				{
					if (12 == ninjaDspCount)
						numHD720Ports = 70;
					else
						numHD720Ports = 6 * ninjaDspCount;
				}
				else if (ninjaDspCount > 12 && ninjaDspCount <= 18)
				{
					if (17 <= ninjaDspCount)
						numHD720Ports = 100;
					else
						numHD720Ports = 6 * ninjaDspCount;
				}
				else
				{
					PASSERTMSG(1, "CSystemResources::GetHD720PortsAccordingToCards - illegal ninjaDspCount");
					numHD720Ports = 100;
				}
			}
				break;
			case eMpmRx_Full:
				numHD720Ports += 100;
				break;
			case eMpmRx_Half:
				numHD720Ports += 30;
				break;
			default:
				break;
		}
	}

	if (numBoards)
		*numBoards = numCurBoards;

	return numHD720Ports;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetSvcPortsPerCards(WORD* numBoards)
{       // This function is relevant only for RMX
	CBoard* pBoard;
	int oneBasedBoardId;
	WORD numSvcPorts = 0, numCurBoards = 0;
	for (int i = 0; i < BOARDS_NUM; i++)
	{
		oneBasedBoardId = i + 1;
		if (!IsBoardIdExists(oneBasedBoardId))
			continue;
		pBoard = GetBoard(oneBasedBoardId);
		eCardType cardType = pBoard->GetCardType();
		numCurBoards++;
		switch (cardType)
		{
			case eMpmx_80:
				numSvcPorts += 90;
				break;
			case eMpmx_40:
				numSvcPorts += 45;
				break;
			case eMpmx_20:
				numSvcPorts += 25;
				break;
			case eMpmRx_Full:
				numSvcPorts += 200;
				break;
			case eMpmRx_Half:
				numSvcPorts += 60;
				break;
			default:
				break;
		}
	}
	if (numBoards)
		*numBoards = numCurBoards;
	return numSvcPorts;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::ShowNumArtChannels(WORD b_id, std::ostream& answer)
{
	if (!CHelperFuncs::IsValidBoardId(b_id))
	{
		answer << "Invalid board id\n";
		return STATUS_FAIL;
	}

	if (!IsBoardIdExists(b_id))
	{
		answer << "Board doesn't exist\n";
		return STATUS_FAIL;
	}

	CBoard* pBoard = GetBoard(b_id);
	if (pBoard == NULL)
	{
		answer << "Board is NULL\n";
		return STATUS_FAIL;
	}

	answer << "GetMaxArtChannelsPerArt() is " << CUnitMFA::GetMaxArtChannelsPerArt() << "\n";

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	for (std::set<CUnitMFA>::iterator i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
	{
		if (i->GetIsEnabled() != TRUE)
			continue;
		if (eUnitType_Art == i->GetUnitType())
		{
			answer << "Unit " << i->GetUnitId() << " Free ART channels: " << i->GetFreeARTChannels() << "\n";
		}
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::FreeAllOccupiedPorts()
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (!pConfRsrcDB)
		return;
	const std::set<CConfRsrc>* pConfRsrcList = pConfRsrcDB->GetConfRsrcsList();
	if ((pConfRsrcDB->GetNumConfRsrcs() > 0) || (pConfRsrcList->size() > 0))
	{
		TRACEINTO << "num of ongoing conferences is not 0 !!!";
		return;
	}
	CBoard* pBoard;
	CMediaUnitsList* pMediaUnitslist;
	std::set<CUnitMFA>::iterator i;

	for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		pBoard = m_pBoards[boardId];
		pMediaUnitslist = pBoard->GetMediaUnitsList();

		for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
		{
			CUnitMFA* pUnit = (CUnitMFA*)&(*i);
			if (pUnit->GetIsAllocated())
				pUnit->FreeAllActivePorts();
		}
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindBestRsrvAC(WORD& bestRsrvACBId, WORD& bestAcUnitID)
{       // should be called only in case of RMX4000

	STATUS status = STATUS_FAIL;
	WORD masterBId = GetAudioCntrlMasterBid();
	WORD acUnitID = 0xFFFF;
	DWORD min_capacity = 1000;
	DWORD promil = 0;

	for (WORD i = 0; i < BOARDS_NUM; i++)
	{
		WORD oneBasedBoardId = i + 1;
		if (masterBId == oneBasedBoardId || !IsBoardIdExists(oneBasedBoardId))
			continue;
		CBoard* pBoard = GetBoard(oneBasedBoardId);
		// find a better reserved AC (i.e. its free capacity is biggest)
		if (pBoard && E_AC_RESERVED == pBoard->GetACType())
		{
			acUnitID = pBoard->GetAudioControllerUnitId();
			CUnitMFA* pMFA = (CUnitMFA*)(pBoard->GetMFA(acUnitID));

			if (pMFA && TRUE == pMFA->GetIsEnabled())
			{
				promil = (DWORD)ceil(1000 - pMFA->GetFreeCapacity());
				if (promil < min_capacity)
				{
					min_capacity = promil;
					bestRsrvACBId = oneBasedBoardId;
					bestAcUnitID = acUnitID;

				}
			}
		}
	}

	if (0xFFFF != bestRsrvACBId)
	{
		status = STATUS_OK;
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SwapMasterAcAndReserveAc(WORD prevMasterBoardID, WORD prevReservedBoardID)
{
	CBoard* pPrevMasterBoard = prevMasterBoardID ? GetBoard(prevMasterBoardID) : NULL;
	CBoard* pPrevReservedBoard = prevReservedBoardID ? GetBoard(prevReservedBoardID) : NULL;
	CUnitMFA* pMFA = NULL;
	if (pPrevReservedBoard)
	{
		WORD prevReservedUid = pPrevReservedBoard->GetAudioControllerUnitId();
		pMFA = (CUnitMFA*)(pPrevReservedBoard->GetMFA(prevReservedUid));
	}
	PASSERT_AND_RETURN(NULL == pMFA);

	DWORD prevReservedConnId = pMFA->GetConnId();

	CConnToCardManager* pConnToCardManager = CHelperFuncs::GetConnToCardManager();
	PASSERT_AND_RETURN(NULL == pConnToCardManager);

	ConnToCardTableEntry EntryMaster, EntryReserved;

	// Update Shared Memory of previous master AC
	DWORD masterConnId = pConnToCardManager->GetConnIdByRsrcType(ePhysical_audio_controller, E_AC_MASTER);
	if (EMPTY_ENTRY != masterConnId)
	{
		PASSERT(pConnToCardManager->Get(masterConnId, EntryMaster));
		EntryMaster.rsrcCntlType = E_AC_RESERVED;
		pConnToCardManager->Update(EntryMaster);
	}

	// Update Shared Memory of previous reserved
	PASSERT(pConnToCardManager->Get(prevReservedConnId, EntryReserved));
	EntryReserved.rsrcCntlType = E_AC_MASTER;
	pConnToCardManager->Update(EntryReserved);

	// Update local variables
	SetAudioCntrlMasterBid(prevReservedBoardID);
	if (pPrevMasterBoard)
		pPrevMasterBoard->SetACType(E_AC_RESERVED);
	if (pPrevReservedBoard)
		pPrevReservedBoard->SetACType(E_AC_MASTER);
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetMaxUnitsNeededForVideo() const
{
	return (m_eventMode ? MAX_UNITS_NEEDED_FOR_VIDEO_COP : MAX_UNITS_NEEDED_FOR_VIDEO);
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetMaxRequiredPortsPerMediaUnit() const
{
	if (CHelperFuncs::IsSoftMCU(m_ProductType))
		return MAX_REQUIRED_PORTS_PER_MEDIA_SOFTMCU_UNIT;  //OLGA - SoftMCU

	return (m_eventMode ? MAX_REQUIRED_PORTS_PER_MEDIA_UNIT_COP : MAX_REQUIRED_PORTS_PER_MEDIA_UNIT);
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetTotalMaxVideoResources() const
{
	return (m_eventMode ? TOTAL_MAX_VIDEO_RESOURCES_COP : TOTAL_MAX_VIDEO_RESOURCES);
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::ReserveRecoveryART(WORD boardId)
{
	STATUS status = STATUS_FAIL;
	std::ostringstream ostr;
	ostr << " CSystemResources::ReserveRecoveryART : boardId = " << boardId;
	if (IsBoardIdExists(boardId))
	{
		CBoard* pBoard = GetBoard(boardId);
		PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);

		CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
		std::set<CUnitMFA>::iterator itr;
		for (itr = pMediaUnitslist->begin(); itr != pMediaUnitslist->end(); itr++)
		{
			if (itr->GetUnitType() != eUnitType_Art ||
			    pBoard->GetAudioControllerUnitId() == itr->GetUnitId() ||
			    (CHelperFuncs::IsBreeze(pBoard->GetCardType()) && !CCardResourceConfigBreeze::ShouldBeARTUnit(itr->GetPhysicalUnitId())) ||
			    (CHelperFuncs::IsBreeze(pBoard->GetCardType()) && !CCardResourceConfigBreeze::CanBeUsedForARTRecovery(itr->GetPhysicalUnitId())))
				continue;
			CUnitMFA* unit = (CUnitMFA*)(&(*itr));
			unit->SetRecoveryReservedUnit(TRUE);
			pBoard->SetRecoveryReservedArtUnitId(itr->GetUnitId());
			ostr << ", reserve the art unit = " << itr->GetUnitId();
			status = STATUS_OK;
			break;
		}
	}
	TRACEINTO << ostr.str().c_str();
	return status;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSystemResources::IsPcmMenuIdExist( DWORD rsrcPartyId ) const
{
	//	for(int i = 0; i < MAX_NUM_OF_BOARDS; i++)
	for (int i = 0; i < BOARDS_NUM; i++)
	{
		CBoard* pBoard = GetBoard(i + 1); //board num is 1-based
		if (pBoard && pBoard->IsPcmMenuIdExist(rsrcPartyId))
			return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetNumOfIpServicesConfiguredToBoard(WORD boardId)
{
	CMedString mstr;
	WORD num_of_ip_services_on_board = 0;
	std::set<CIPServiceResources>::iterator ip_service_itr;
	for (ip_service_itr = m_pIPServices->begin(); ip_service_itr != m_pIPServices->end(); ip_service_itr++)
	{
		if (ip_service_itr->IsConfiguredToBoard(boardId))
		{
			mstr << "service: " << ip_service_itr->GetName() << " , id: " << ip_service_itr->GetServiceId() << " , is configured to board: " << boardId << "\n";
			num_of_ip_services_on_board++;
		}

	}
	mstr << "num_of_ip_services_on_board = " << num_of_ip_services_on_board << "\n";

	return num_of_ip_services_on_board;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetSystemNumOfMfaUnits(BOOL withIpServiceOnly)
{
	CMedString mstr;
	WORD numOfMfaUnits = 0;

	CBoard* pCurrentBoard = NULL;
	for (WORD board_index = 0; board_index < BOARDS_NUM; board_index++)
	{
		WORD bid = board_index + 1;
		if (!IsBoardIdExists(bid))
			continue;

		if (withIpServiceOnly && (0 == GetNumOfIpServicesConfiguredToBoard(bid)))
		{
			mstr << "board_id: " << bid << " has no configured IP service" << "\n";
			continue;
		}
		pCurrentBoard = GetBoard(bid);

		if (pCurrentBoard == NULL)
		{
			PASSERT(bid);
			continue;
		}

		//void CBoard::CountUnits(int &audio, int &video, BOOL bCountDisabledToo)
		DWORD art_units = 0, video_units = 0, mfa_units = 0;
		pCurrentBoard->CountUnits(art_units, video_units, TRUE);
		numOfMfaUnits += art_units + video_units;
		mstr << "board_id: " << bid << " , numOfMfaUnits: " << numOfMfaUnits << "  (art_units: " << art_units << ", video_units: " << video_units << ")\n";

	}
	mstr << "numOfMfaUnits = " << numOfMfaUnits << "\n";

	PTRACE2(eLevelInfoNormal, "CSystemResources::GetSystemNumOfMfaUnits \n", mstr.GetString());

	return numOfMfaUnits;
}

////////////////////////////////////////////////////////////////////////////
float CSystemResources::GetIpServiceNumOfMfaUnits(WORD service_id)
{
	CMedString mstr;
	float numOfMfaUnits = 0;

	const CIPServiceResources* pIpService = GetIpService(service_id);
	if (pIpService == NULL)
	{
		PASSERT(service_id + 1);
		return 0;
	}

	CBoard* pCurrentBoard = NULL;
	for (WORD board_index = 0; board_index < BOARDS_NUM; board_index++)
	{
		if (!IsBoardIdExists(board_index + 1))
		{
			continue;
		}

		pCurrentBoard = GetBoard(board_index + 1);

		if (pCurrentBoard == NULL)
		{
			PASSERT(board_index + 1);
			continue;
		}

		if (pIpService->IsConfiguredToBoard(board_index + 1))
		{
			DWORD art_units = 0, video_units = 0, mfa_units = 0;
			pCurrentBoard->CountUnits(art_units, video_units, TRUE);

			WORD num_of_ip_services_on_board = GetNumOfIpServicesConfiguredToBoard(board_index + 1);

			float numOfMfaUnitsOnBoard = (art_units + video_units);

			numOfMfaUnits += numOfMfaUnitsOnBoard;

			mstr << "service: " << pIpService->GetName() << " has  " << numOfMfaUnitsOnBoard << " from " << (art_units + video_units) << " on board: " << board_index + 1 << "\n";
		}
	}
	mstr << "numOfMfaUnits = " << numOfMfaUnits << "\n";

	PTRACE2(eLevelInfoNormal, "CSystemResources::GetIpServiceNumOfMfaUnits \n", mstr.GetString());

	return numOfMfaUnits;
}

////////////////////////////////////////////////////////////////////////////
const CIPServiceResources* CSystemResources::GetIpService(WORD service_id)
{
	const CIPServiceResources* pSer = NULL;

	std::set<CIPServiceResources>::iterator ip_service_itr;
	for (ip_service_itr = m_pIPServices->begin(); ip_service_itr != m_pIPServices->end(); ip_service_itr++)
	{

		if (ip_service_itr->GetServiceId() == service_id)
		{
			pSer = &(*ip_service_itr);
			break;
		}
	}

	if (pSer == NULL)
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::GetIpService, service not found: id = ", service_id);
	}

	return pSer;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSystemResources::IsIpServiceConfiguredToBoard(WORD service_id, WORD board_id)
{
	BOOL ret_val = FALSE;

	if (service_id == ID_ALL_IP_SERVICES)
	{
		WORD num_of_services_configured = GetNumOfIpServicesConfiguredToBoard(board_id);
		if (num_of_services_configured != 0)
		{
			return TRUE;
		}
	}
	const CIPServiceResources* pIpService = GetIpService(service_id);
	if (pIpService == NULL)
	{
		PASSERT(service_id + 1);
		return FALSE;
	}
	if (pIpService->IsConfiguredToBoard(board_id))
	{
		ret_val = TRUE;
	}
	return ret_val;
}

////////////////////////////////////////////////////////////////////////////
float CSystemResources::GetAllIpServicesNumOfMfaUnits()
{
	float all_ip_services_units = 0;

	std::set<CIPServiceResources>::iterator ip_service_itr;
	for (ip_service_itr = m_pIPServices->begin(); ip_service_itr != m_pIPServices->end(); ip_service_itr++)
	{

		all_ip_services_units += GetIpServiceNumOfMfaUnits(ip_service_itr->GetServiceId());
	}

	CSmallString sstr;
	sstr << "all_ip_services_units = " << all_ip_services_units;
	PTRACE2(eLevelInfoNormal, "CSystemResources::GetAllIpServicesNumOfMfaUnits ", sstr.GetString());

	return all_ip_services_units;
}

////////////////////////////////////////////////////////////////////////////
float CSystemResources::GetIpServicesWeight(WORD service_id)
{
	//  const CIPServiceResources* pSer = NULL;
	float ip_services_weight = 0;

	float all_ip_services_units = GetSystemNumOfMfaUnits(TRUE); //GetAllIpServicesNumOfMfaUnits();
	float ip_service_units = GetIpServiceNumOfMfaUnits(service_id);

	ip_services_weight = ip_service_units / all_ip_services_units;

	CSmallString sstr;
	sstr << "service_id = " << service_id << ", all_ip_services_units = " << all_ip_services_units << " , ip_service_units = " << ip_service_units << " , ip_services_weight = " << ip_services_weight;
	PTRACE2(eLevelInfoNormal, "CSystemResources::GetIpServicesWeight ", sstr.GetString());

	return ip_services_weight;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CreateIpServiceInterface(WORD service_id, const char* service_name)
{
	CIpServiceResourcesInterfaceArray ipServiceResourcesInterface(service_id, service_name);
	if (m_IpServicesResourcesInterface->find(ipServiceResourcesInterface) != m_IpServicesResourcesInterface->end())
	{
		TRACEINTO << "ServiceId:" << service_id << ", ServiceName:" << service_name << " - Already exist";
		return STATUS_OK;  // STATUS_ALREADY_EXISTS;
	}

	TRACEINTO << "ServiceId:" << service_id << ", ServiceName:" << service_name;

	ipServiceResourcesInterface.InitResourceAllocationMode(m_ResourceAllocationType);

	DWORD num_parties = 0;
	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator)
		num_parties = pReservator->GetDongleRestriction();
	else
		PASSERT(1);

	ipServiceResourcesInterface.InitDongleRestriction(num_parties);
	TRACEINTO << ipServiceResourcesInterface;

	m_IpServicesResourcesInterface->insert(ipServiceResourcesInterface);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::InitIpServicesInterfaces()
{
	PASSERT_AND_RETURN_VALUE(!m_pIPServices, STATUS_FAIL);

	for (std::set<CIPServiceResources>::iterator _itr = m_pIPServices->begin(); _itr != m_pIPServices->end(); ++_itr)
	{
		WORD service_id = _itr->GetServiceId();
		const char* service_name = _itr->GetName();

		TRACEINTO << "ServiceId:" << service_id << ", ServiceName:" << service_name;

		STATUS status = CreateIpServiceInterface(service_id, service_name);
		PASSERTSTREAM(status != STATUS_OK, "Status:" << status);
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::UpdateIpServicesDongleRestriction()
{
	PASSERT_AND_RETURN_VALUE(!m_IpServicesResourcesInterface, STATUS_FAIL);

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN_VALUE(!pReservator, STATUS_FAIL);

	BOOL round_up = TRUE;

	for (std::set<CIpServiceResourcesInterfaceArray>::iterator _itr = m_IpServicesResourcesInterface->begin(); _itr != m_IpServicesResourcesInterface->end(); ++_itr)
	{
		DWORD serviceId = _itr->GetServiceId();
		float serviceFactor = GetIpServicesWeight(serviceId);

		TRACEINTO << "ServiceId:" << serviceId << ", ServiceFactor:" << serviceFactor << ", RoundUp:" << (WORD)round_up;

		((CIpServiceResourcesInterfaceArray*)(&(*_itr)))->UpdateServiceWeight(serviceFactor, round_up);
		pReservator->AddIpServiceCalc(serviceId, serviceFactor, round_up);

		round_up = FALSE;
	}

	pReservator->DumpCalculatorTotals();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSystemResources::CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType,
                                                     ePartyRole& partyRole,
                                                     EAllocationPolicy allocPolicy,
                                                     ALLOC_PARTY_IND_PARAMS_S* pResult,
                                                     WORD ipServiceId,
                                                     BYTE rmxPortGaugeThreshold /*= FALSE*/,
                                                     BOOL* pbAddAudioAsVideo /*= NULL*/,
                                                     eConfModeTypes confModeType /*= eNonMix*/,
                                                     BOOL countPartyAsICEinMFW /*= FALSE*/)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	PASSERT_AND_RETURN_VALUE(!pCurent, FALSE);

	std::ostringstream msg;

	// check for system
	BOOL can_be_added = pCurent->CheckIfOneMorePartyCanBeAdded(videoPartyType, partyRole, allocPolicy, pResult, rmxPortGaugeThreshold, pbAddAudioAsVideo, confModeType, countPartyAsICEinMFW);
	msg << "can_be_added:" << (WORD)can_be_added;

	// check for ip service
	if (can_be_added && GetMultipleIpServices() && ipServiceId != ID_ALL_IP_SERVICES)
	{
		msg << ", ipServiceId:" << ipServiceId;
		CIpServiceResourcesInterfaceArray* pIpServiceResourcesInterface = GetIpServiceInterface(ipServiceId);
		if (pIpServiceResourcesInterface)
		{
			can_be_added = pIpServiceResourcesInterface->CheckIfOneMorePartyCanBeAdded(videoPartyType, partyRole, allocPolicy, pResult, pbAddAudioAsVideo, confModeType, countPartyAsICEinMFW);
			msg << ", ipServiceName:" << pIpServiceResourcesInterface->GetServiceName() << ", can_be_added_according_to_service:" << (WORD)can_be_added;
		}
	}

	TRACEINTO << msg.str().c_str();
	return can_be_added;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::AddParty(BasePartyDataStruct& rPartyData)
{
	// add to system
	m_ResourcesInterfaceArray.AddParty(rPartyData);

	bool isMultipleIpServices = GetMultipleIpServices();

	std::ostringstream msg;
	msg << "\n  PartyId                 :" << rPartyData.m_partyId
	    << "\n  VideoPartyType          :" << eVideoPartyTypeNames[rPartyData.m_videoPartyType] << " (" << rPartyData.m_videoPartyType << ")"
	    << "\n  IsISDNParty             :" << (WORD)rPartyData.m_bIsISDNParty
	    << "\n  IsMultipleIpServices    :" << (WORD)isMultipleIpServices;

	// add to ip service
	if (isMultipleIpServices && !rPartyData.m_bIsISDNParty)
	{
		msg << "\n  ServiceId               :" << rPartyData.m_ipServiceId;
		if (rPartyData.m_ipServiceId != ID_ALL_IP_SERVICES)
		{
			CIpServiceResourcesInterfaceArray* pIpServiceResourcesInterface = GetIpServiceInterface(rPartyData.m_ipServiceId);
			if (pIpServiceResourcesInterface != NULL)
			{
				pIpServiceResourcesInterface->AddParty(rPartyData);
				msg << "\n  ServiceName             :" << pIpServiceResourcesInterface->GetServiceName();
			}
			else
			{
				msg << "\n  ServiceName             :" << "NA";
				DBGPASSERT(rPartyData.m_ipServiceId);
			}
		}
		else
		{
			DBGPASSERT(rPartyData.m_ipServiceId);
		}
	}
	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::RemoveParty(BasePartyDataStruct& rPartyData)
{
	// remove from system
	m_ResourcesInterfaceArray.RemoveParty(rPartyData);

	bool isMultipleIpServices = GetMultipleIpServices();

	std::ostringstream msg;
	msg << "\n  PartyId                 :" << rPartyData.m_partyId
	    << "\n  VideoPartyType          :" << eVideoPartyTypeNames[rPartyData.m_videoPartyType] << " (" << rPartyData.m_videoPartyType << ")"
	    << "\n  IsISDNParty             :" << (WORD)rPartyData.m_bIsISDNParty
	    << "\n  IsMultipleIpServices    :" << (WORD)isMultipleIpServices;

	// remove from ip service
	if (isMultipleIpServices && !rPartyData.m_bIsISDNParty)
	{
		WORD ipServiceId = GetPartyIpServiceId(rPartyData.m_partyId);
		msg << "\n  ServiceId               :" << ipServiceId;
		if (ipServiceId != ID_ALL_IP_SERVICES)
		{
			CIpServiceResourcesInterfaceArray* pIpServiceResourcesInterface = GetIpServiceInterface(ipServiceId);
			if (pIpServiceResourcesInterface != NULL)
			{
				pIpServiceResourcesInterface->RemoveParty(rPartyData);
				msg << "\n  ServiceName             :" << pIpServiceResourcesInterface->GetServiceName();
			}
			else
			{
				msg << "\n  ServiceName             :" << "NA";
				DBGPASSERT(ipServiceId);
			}
		}
		else
		{
			DBGPASSERT(ipServiceId);
		}
	}
	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
CIpServiceResourcesInterfaceArray* CSystemResources::GetIpServiceInterface(WORD service_id)
{
	std::set<CIpServiceResourcesInterfaceArray>::iterator interface_itr;
	for (interface_itr = m_IpServicesResourcesInterface->begin(); interface_itr != m_IpServicesResourcesInterface->end(); interface_itr++)
	{

		if (service_id == interface_itr->GetServiceId())
		{
			return (CIpServiceResourcesInterfaceArray*)(&(*interface_itr));
		}
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetPartyIpServiceId(DWORD party_rsrc_id)
{

	WORD service_id = ID_ALL_IP_SERVICES;
	std::set<CIpServiceResourcesInterfaceArray>::iterator interface_itr;
	for (interface_itr = m_IpServicesResourcesInterface->begin(); interface_itr != m_IpServicesResourcesInterface->end(); interface_itr++)
	{

		if (interface_itr->FindPartyId(party_rsrc_id) == TRUE)
		{
			service_id = interface_itr->GetServiceId();
			break;
		}
	}
	if (service_id == ID_ALL_IP_SERVICES)
	{
		DBGPASSERT(party_rsrc_id);
	}

	return service_id;
}

////////////////////////////////////////////////////////////////////////////
const char* CSystemResources::GetIpServiceName(DWORD service_id)
{
	std::set<CIpServiceResourcesInterfaceArray>::iterator interface_itr;
	for (interface_itr = m_IpServicesResourcesInterface->begin(); interface_itr != m_IpServicesResourcesInterface->end(); interface_itr++)
	{
		DWORD current_service_id = interface_itr->GetServiceId();
		if (current_service_id == service_id)
		{
			return (interface_itr->GetServiceName());
		}
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSystemResources::GetIpServiceId(const char* service_name)
{
	if (service_name == NULL)
	{
		PTRACE(eLevelInfoNormal, "CSystemResources::GetIpServiceId - received service_name NULL");
		return ID_ALL_IP_SERVICES;
	}
	DWORD ret_service_id = ID_ALL_IP_SERVICES;

	std::set<CIpServiceResourcesInterfaceArray>::iterator interface_itr;
	for (interface_itr = m_IpServicesResourcesInterface->begin(); interface_itr != m_IpServicesResourcesInterface->end(); interface_itr++)
	{
		const char* current_service_name = interface_itr->GetServiceName();
		if (current_service_name != NULL)
		{
			if (0 == strncmp(current_service_name, service_name, NET_SERVICE_PROVIDER_NAME_LEN))
			{
				ret_service_id = interface_itr->GetServiceId();
				break;
			}
		}
	}
	return ret_service_id;
}

////////////////////////////////////////////////////////////////////////////
const char* CSystemResources::GetDefaultIpService(DWORD& service_id, BYTE default_type)
{
	if (m_pIPServices)
	{
		std::set<CIPServiceResources>::iterator ip_service_itr;
		bool found = false;
		for (ip_service_itr = m_pIPServices->begin(); ip_service_itr != m_pIPServices->end(); ip_service_itr++)
		{
			BYTE service_default_type = ip_service_itr->GetDefaultH323SipService();

			if (service_default_type == DEFAULT_SERVICE_BOTH)
			{
				found = true;
			}
			else if (service_default_type == DEFAULT_SERVICE_SIP && default_type == DEFAULT_SERVICE_SIP)
			{
				found = true;
			}
			else if (service_default_type == DEFAULT_SERVICE_H323 && default_type == DEFAULT_SERVICE_H323)
			{
				found = true;
			}
			if (found)
			{
				service_id = ip_service_itr->GetServiceId();
				return (ip_service_itr->GetName());
			}
		}
		// if we here  - not good - default service always should be defined - we return the first service
		PTRACE(eLevelInfoNormal, "stemResources::GetDefauleIpService - default service not found");
		ip_service_itr = m_pIPServices->begin();
		service_id = ip_service_itr->GetServiceId();
		return (ip_service_itr->GetName());
	}
	DBGPASSERT(1);
	service_id = ID_ALL_IP_SERVICES;
	return "not found";
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetDefaultIpService(const char* defaultH323serv, const char* defaultSIPserv)
{
	BOOL isSameDefault = FALSE;
	if (defaultH323serv && defaultSIPserv && !strcmp(defaultH323serv, defaultSIPserv))
		isSameDefault = TRUE;

	CPrettyTable<WORD, const char*, WORD> tbl("ServiceId", "ServiceName", "DefaultType");

	for (std::set<CIPServiceResources>::iterator _itr = m_pIPServices->begin(); _itr != m_pIPServices->end(); _itr++)
	{
		if (defaultH323serv && !strcmp(_itr->GetName(), defaultH323serv))
		{
			BYTE service_default_type = (isSameDefault ? DEFAULT_SERVICE_BOTH : DEFAULT_SERVICE_H323);
			((CIPServiceResources*)(&(*_itr)))->SetDefaultH323SipService(service_default_type);
		}
		else if (defaultSIPserv && !strcmp(_itr->GetName(), defaultSIPserv))
		{
			BYTE service_default_type = (isSameDefault ? DEFAULT_SERVICE_BOTH : DEFAULT_SERVICE_SIP);
			((CIPServiceResources*)(&(*_itr)))->SetDefaultH323SipService(service_default_type);
		}
		else
			((CIPServiceResources*)(&(*_itr)))->SetDefaultH323SipService(DEFAULT_SERVICE_NONE);

		tbl.Add(_itr->GetServiceId(), _itr->GetName(), (WORD)_itr->GetDefaultH323SipService());
	}
	TRACEINTO << "DefaultH323service:" << DUMPSTR(defaultH323serv) << ", DefaultSIPservice:" << DUMPSTR(defaultSIPserv) << tbl.Get();
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::ChangeResourceAllocationModePerIpServices( eResourceAllocationTypes newResourceAllocationType )
{
	std::set<CIpServiceResourcesInterfaceArray>::iterator interface_itr;
	for (interface_itr = m_IpServicesResourcesInterface->begin(); interface_itr != m_IpServicesResourcesInterface->end(); interface_itr++)
		((CIpServiceResourcesInterfaceArray*)(&(*interface_itr)))->ChangeResourceAllocationMode(newResourceAllocationType);
}

////////////////////////////////////////////////////////////////////////////
BYTE CSystemResources::GetMultipleIpServices() const
{
	if (m_isMultipleIpServices)
		return m_isMultipleIpServices;

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL isJitcModeEnable = NO, isV35Enable = NO;
	sysConfig->GetBOOLDataByKey("ULTRA_SECURE_MODE", isJitcModeEnable);
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35Enable);
	if (isJitcModeEnable && isV35Enable)
		return YES;
	return NO;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetPortConfigurationPerIpServices( WORD selectedIndex )
{
	if (!m_IpServicesResourcesInterface)
		return;
	std::set<CIpServiceResourcesInterfaceArray>::iterator itr;
	for (itr = m_IpServicesResourcesInterface->begin(); itr != m_IpServicesResourcesInterface->end(); itr++)
		((CIpServiceResourcesInterfaceArray*)(&(*itr)))->SetPortConfiguration( selectedIndex );
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetMaxNumberVSWConferencesEventMode() const
{
	WORD max_num_conf = 0;
	BOOL isMPMX       = (eSystemCardsMode_breeze == m_SystemCardsMode);
	switch (m_ProductType)
	{
		case eProductTypeRMX1500:
			if (isMPMX)
				max_num_conf = MAX_NUMBER_OF_ONGOING_CONFERENCES_NxM_MPMX_BASED;
			else
				DBGPASSERT(m_SystemCardsMode);        //according to SRS Event Mode
			break;
		case eProductTypeRMX2000:
		case eProductTypeSoftMCU:
		case eProductTypeGesher:
		case eProductTypeNinja:
		case eProductTypeSoftMCUMfw:
		case eProductTypeEdgeAxis:
		case eProductTypeCallGeneratorSoftMCU:
			max_num_conf = isMPMX ? (MAX_NUMBER_OF_ONGOING_CONFERENCES_NxM_MPMX_BASED * 2) : MAX_NUMBER_OF_ONGOING_CONFERENCES_NxM_MPM_PLUS_BASED;
			break;
		case eProductTypeRMX4000:
			max_num_conf = isMPMX ? (MAX_NUMBER_OF_ONGOING_CONFERENCES_NxM_MPMX_BASED * 4) : (MAX_NUMBER_OF_ONGOING_CONFERENCES_NxM_MPM_PLUS_BASED * 2);
			break;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	//TRACEINTO << " CSystemResources::GetMaxNumberVSWConferencesEventMode : max_num_conf = " << max_num_conf;
	return max_num_conf;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetMaxNumOngoingConfPerCardsInEventMode()
{
	CBoard* pBoard;
	int oneBasedBoardId;
	WORD numConf2C = 0;

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		oneBasedBoardId = i + 1;
		if (!IsBoardIdExists(oneBasedBoardId))
			continue;
		pBoard = GetBoard(oneBasedBoardId);
		eCardType cardType = pBoard->GetCardType();
		switch (cardType)
		{
			case eMpmx_20:         //new type MPMX-Q
				numConf2C += 1;
				break;
			case eMpmx_40:
				numConf2C += 2;
				break;
			case eMpmx_80:
				numConf2C += 4;
				break;
			default:         // other assemblies aren't allowed in V4.7
				TRACEINTO << " SystemResources::GetMaxNumOngoingConfPerCardsInEventMode : unknown card type = " << (WORD)cardType;
				break;
		}
	}
	//TRACEINTO << " SystemResources::GetMaxNumOngoingConfPerCardsInEventMode : max conf num = " << numConf2C;
	return numConf2C;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::CalculatePortsConfigurationStep() const
{
	WORD step = PORTS_CONFIGURATION_STEP; // MPM-Rx

	if (CResRsrcCalculator::IsRMX1500Q())
	{
		step = PORTS_CONFIGURATION_STEP_RMX_1500Q;
	}
	else if (m_SystemCardsMode == eSystemCardsMode_breeze && !CHelperFuncs::IsSoftMCU(m_ProductType)) //OLGA - SoftMCU
	{
		step = PORTS_CONFIGURATION_STEP_BREEZE;
	}
	TRACEINTO << "CardsMode:" << ::GetSystemCardsModeStr(m_SystemCardsMode) << ", Step:" << step;

	return step;
}

////////////////////////////////////////////////////////////////////////////
float CSystemResources::CalculateAudioFactorStep() const
{
	float step = AUDIO_FACTOR; //MPM-Rx

	if (CResRsrcCalculator::IsRMX1500QRatios())
	{
		step = AUDIO_FACTOR_RMX_1500Q;
	}
	else if (m_SystemCardsMode == eSystemCardsMode_breeze)
	{
		if (!CHelperFuncs::IsSoftMCU(m_ProductType))
			step = AUDIO_FACTOR_BREEZE;
		else if (eProductTypeSoftMCUMfw == m_ProductType)
			step = AUDIO_FACTOR_SOFTMCU_MFW;
		else if (eProductTypeNinja == m_ProductType)
			step = AUDIO_FACTOR_SOFTMCU_NINJA;
		else
			step = AUDIO_FACTOR_SOFTMCU;
	}
	return step;
}

////////////////////////////////////////////////////////////////////////////
float CSystemResources::GetAudioFactor()
{
	return m_AudioFactor;
}

////////////////////////////////////////////////////////////////////////////
bool CSystemResources::IsUnitAlreadySelectedForAllocation(VideoDataPerBoardStruct& videoAlloc, int mediaUnitIndex, WORD unitId)
{
	// check that we didn't plan to allocate it for the other required units
	for (int i = 0; i < mediaUnitIndex; ++i)
		if (videoAlloc.m_VideoAlloc.m_unitsList[i].m_UnitId == unitId)
			return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::DumpBoardUnits(VideoDataPerBoardStruct& videoAlloc, BoardID boardId)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN(!pBoard);

	eCardType cardType = pBoard->GetCardType();
	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	std::ostringstream msg;

	msg << "CSystemResources::DumpBoardUnits - BoardId:" << boardId << endl;

	if (CHelperFuncs::IsBreeze(cardType))
	{
		msg.precision(1);
		msg << " -----+------+----------+--------+--------+---------+-------+------+-------+-------+------+------+-----------+" << endl;
		msg << " unit | phys | free     | free   | free   | free    | turbo | fpga | band- | band- | Tip  | unit | unit type |" << endl;
		msg << " id   | id   | capacity | enc /  | dec    | encoder | dsp   |      | width | width | Art  | ena- |           |" << endl;
		msg << "      |      |          | art    | ports  | weight  |       |      | in    | out   | ports| bled |           |" << endl;
		msg << "      |      |          | ports  |        |         |       |      |       |       |      |      |           |" << endl;
		msg << " -----+------+----------+--------+--------+---------+-------+------+-------+-------+------+------+-----------+" << endl;

		std::set<CUnitMFA>::iterator _iiUnit, _iiEnd = pMediaUnitslist->end();
		for (_iiUnit = pMediaUnitslist->begin(); _iiUnit != _iiEnd; ++_iiUnit)
		{
			WORD   unit_id                = _iiUnit->GetUnitId();
			WORD   physical_unit_id       = _iiUnit->GetPhysicalUnitId();
			float  free_capacity          = _iiUnit->GetFreeCapacity(0);
			DWORD  all_free_encoder_ports = 0;
			DWORD  all_free_decoder_ports = 0;
			DWORD  free_encoder_ports     = 0;
			DWORD  free_decoder_ports     = 0;
			BOOL   turbo_dsp              = CCardResourceConfigBreeze::IsTurboVideoUnit(physical_unit_id);
			WORD   fpga_index             = _iiUnit->GetFPGAIndex();
			WORD   free_bandwidth_in      = 0;
			WORD   free_bandwidth_out     = 0;
			WORD   tipArtPorts            = _iiUnit->GetNumAllocatedTipScreens();
			BOOL   unit_enabled           = _iiUnit->GetIsEnabled();
			float  free_encoder_weight    = _iiUnit->GetFreeEncoderWeight();
			string unit_type              = "-VIDEO-";

			eUnitType unitType = _iiUnit->GetUnitType();
			if (unitType == eUnitType_Art || unitType == eUnitType_Art_Control)
			{
				all_free_encoder_ports = _iiUnit->GetFreeArtPorts(TRUE);
				free_encoder_ports     = _iiUnit->GetFreeArtPorts(FALSE);
			}
			else
			{
				all_free_encoder_ports = _iiUnit->GetFreeEncoderPorts(0, TRUE);
				all_free_decoder_ports = _iiUnit->GetFreeDecoderPorts(0, TRUE);
				free_encoder_ports     = _iiUnit->GetFreeEncoderPorts(0, FALSE);
				free_decoder_ports     = _iiUnit->GetFreeDecoderPorts(0, FALSE);
			}

			if (fpga_index < NUM_OF_FPGAS_PER_BOARD)
			{
				free_bandwidth_in  = videoAlloc.m_freeBandwidth_in[fpga_index];
				free_bandwidth_out = videoAlloc.m_freeBandwidth_out[fpga_index];
			}
			else
			{
				PASSERT(fpga_index);
				free_bandwidth_in  = 0;
				free_bandwidth_out = 0;
			}

			switch (unitType)
			{
				case eUnitType_Video: unit_type    = "-VIDEO-"; break;
				case eUnitType_Generic: unit_type  = "-GENERIC-"; break;
				case eUnitType_Rtm: unit_type      = "-RTM-"; break;
				case eUnitType_Art_Control: unit_type = "-ART_CNTL-"; break;
				case eUnitType_Art: unit_type      = "-ART-"; break;
				default: unit_type            = "-UNKNOWN-"; break;
			}

			msg << " " << setw( 4) << right << unit_id << " |"
			    << " " << setw( 4) << right << physical_unit_id << " |"
			    << " " << setw( 8) << fixed << right << (int)free_capacity << " |"
			    << " " << setw( 2) << right << all_free_encoder_ports << "(" << setw(2) << free_encoder_ports << ") |";
			if (unitType == eUnitType_Art || unitType == eUnitType_Art_Control)
				msg << "    N/A |";
			else
				msg << " " << setw( 2) << right << all_free_decoder_ports << "(" << setw(2) << free_decoder_ports << ") |";
			msg << " " << setw( 7) << fixed << right << free_encoder_weight << " |"
			    << " " << setw( 5) << right << (WORD)turbo_dsp << " |"
			    << " " << setw( 4) << right << fpga_index << " |"
			    << " " << setw( 5) << right << free_bandwidth_in << " |"
			    << " " << setw( 5) << right << free_bandwidth_out << " |"
			    << " " << setw( 4) << right << tipArtPorts << " |"
			    << " " << setw( 4) << right << (WORD)unit_enabled << " |"
			    << " " << setw(10) << left << unit_type << "|" << endl;
		}
		msg << " -----+------+----------+--------+--------+---------+-------+------+-------+-------+------+------+-----------+";
	}
	else // MPM-Rx, Ninja, SoftMCU
	{
		if (CHelperFuncs::IsSoftMCU(GetProductType()))
			msg.precision(3);
		else
			msg.precision(0);

		msg << " -----+------+--------------------------------+----------------------------+----------------------------+-----------+-----------+------+------+-----------+" << endl;
		msg << "      |      |         accelerator 0          |       accelerator 1        |       accelerator 2        |           |           |      |      |           |" << endl;
		msg << " -----+------+----------+----------+----------+----------+--------+--------+----------+--------+--------+-----------+-----------+------+------+-----------+" << endl;
		msg << " unit | phys | free     | free     | free     | free     | free   | free   | free     | free   | free   | free      | free      | Tip  | unit | unit type |" << endl;
		msg << " id   | id   | capacity | enc /    | dec      | capacity | enc    | dec    | capacity | enc    | dec    | bandwidth | bandwidth | Art  | ena- |           |" << endl;
		msg << "      |      |          | art      | ports    |          | ports  | ports  |          | ports  | ports  | -in       | -out      | ports| bled |           |" << endl;
		msg << "      |      |          | ports    |          |          |        |        |          |        |        |           |           |      |      |           |" << endl;
		msg << " -----+------+----------+----------+----------+----------+--------+--------+----------+--------+--------+-----------+-----------+------+------+-----------+" << endl;

		std::set<CUnitMFA>::iterator _iiUnit, _iiEnd = pMediaUnitslist->end();
		for (_iiUnit = pMediaUnitslist->begin(); _iiUnit != _iiEnd; ++_iiUnit)
		{
			WORD  unit_id                     = _iiUnit->GetUnitId();
			WORD  physical_unit_id            = _iiUnit->GetPhysicalUnitId();
			DWORD free_bandwidth_in           = TOTAL_POSTSCALER_BW_IN - _iiUnit->GetUtilizedBandwidth(TRUE);
			DWORD free_bandwidth_out          = TOTAL_POSTSCALER_BW_OUT - _iiUnit->GetUtilizedBandwidth(FALSE);
			float free_capacity_acc1          = _iiUnit->GetFreeCapacity(0);
			DWORD all_free_encoder_ports_acc1 = 0;
			DWORD all_free_decoder_ports_acc1 = 0;
			DWORD free_encoder_ports_acc1     = 0;
			DWORD free_decoder_ports_acc1     = 0;
			float free_capacity_acc2          = 0;
			DWORD all_free_encoder_ports_acc2 = 0;
			DWORD all_free_decoder_ports_acc2 = 0;
			DWORD free_encoder_ports_acc2     = 0;
			DWORD free_decoder_ports_acc2     = 0;
			float free_capacity_acc3          = 0;
			DWORD all_free_encoder_ports_acc3 = 0;
			DWORD all_free_decoder_ports_acc3 = 0;
			DWORD free_encoder_ports_acc3     = 0;
			DWORD free_decoder_ports_acc3     = 0;

			eUnitType unitType = _iiUnit->GetUnitType();
			if (unitType == eUnitType_Art || unitType == eUnitType_Art_Control)
			{
				all_free_encoder_ports_acc1 = _iiUnit->GetFreeArtPorts(TRUE);
				free_encoder_ports_acc1     = _iiUnit->GetFreeArtPorts(FALSE);
			}
			else
			{
				all_free_encoder_ports_acc1 = _iiUnit->GetFreeEncoderPorts(0, TRUE);
				all_free_decoder_ports_acc1 = _iiUnit->GetFreeDecoderPorts(0, TRUE);
				free_encoder_ports_acc1     = _iiUnit->GetFreeEncoderPorts(0, FALSE);
				free_decoder_ports_acc1     = _iiUnit->GetFreeDecoderPorts(0, FALSE);
			}

			if (CHelperFuncs::IsMpmRxOrNinja(cardType) &&
			    unitType == eUnitType_Video)
			{
				free_capacity_acc2          = _iiUnit->GetFreeCapacity(1);
				all_free_encoder_ports_acc2 = _iiUnit->GetFreeEncoderPorts(1, TRUE);
				all_free_decoder_ports_acc2 = _iiUnit->GetFreeDecoderPorts(1, TRUE);
				free_encoder_ports_acc2     = _iiUnit->GetFreeEncoderPorts(1, FALSE);
				free_decoder_ports_acc2     = _iiUnit->GetFreeDecoderPorts(1, FALSE);
				free_capacity_acc3          = _iiUnit->GetFreeCapacity(2);
				all_free_encoder_ports_acc3 = _iiUnit->GetFreeEncoderPorts(2, TRUE);
				all_free_decoder_ports_acc3 = _iiUnit->GetFreeDecoderPorts(2, TRUE);
				free_encoder_ports_acc3     = _iiUnit->GetFreeEncoderPorts(2, FALSE);
				free_decoder_ports_acc3     = _iiUnit->GetFreeDecoderPorts(2, FALSE);
			}

			WORD   tipArtPorts  = _iiUnit->GetNumAllocatedTipScreens();
			BOOL   unit_enabled = _iiUnit->GetIsEnabled();
			string unit_type    = "-VIDEO-";

			switch (unitType)
			{
				case eUnitType_Video: unit_type    = "-VIDEO-"; break;
				case eUnitType_Generic: unit_type  = "-GENERIC-"; break;
				case eUnitType_Rtm: unit_type      = "-RTM-"; break;
				case eUnitType_Art_Control: unit_type = "-ART_CNTL-"; break;
				case eUnitType_Art: unit_type      = "-ART-"; break;
				default: unit_type            = "-UNKNOWN-"; break;
			}

			msg << " " << setw( 4) << right << unit_id << " |"
			    << " " << setw( 4) << right << physical_unit_id << " |"
			    << " " << setw( 8) << fixed << right << free_capacity_acc1 << " |"
			    << " " << setw( 3) << right << all_free_encoder_ports_acc1 << "(" << setw(3) << free_encoder_ports_acc1 << ") |";
			if (unitType == eUnitType_Art || unitType == eUnitType_Art_Control)
			{
				msg << "      N/A |      N/A |    N/A |    N/A |      N/A |    N/A |    N/A |";
			}
			else
			{
				msg << " " << setw( 3) << right << all_free_decoder_ports_acc1 << "(" << setw(3) << free_decoder_ports_acc1 << ") |"
				    << " " << setw( 8) << fixed << right << free_capacity_acc2 << " |"
				    << " " << setw( 2) << right << all_free_encoder_ports_acc2 << "(" << setw(2) << free_encoder_ports_acc2 << ") |"
				    << " " << setw( 2) << right << all_free_decoder_ports_acc2 << "(" << setw(2) << free_decoder_ports_acc2 << ") |"
				    << " " << setw( 8) << fixed << right << free_capacity_acc3 << " |"
				    << " " << setw( 2) << right << all_free_encoder_ports_acc3 << "(" << setw(2) << free_encoder_ports_acc3 << ") |"
				    << " " << setw( 2) << right << all_free_decoder_ports_acc3 << "(" << setw(2) << free_decoder_ports_acc3 << ") |";
			}
			msg << " " << setw( 9) << right << free_bandwidth_in << " |"
			    << " " << setw( 9) << right << free_bandwidth_out << " |"
			    << " " << setw( 4) << right << tipArtPorts << " |"
			    << " " << setw( 4) << right << (WORD)unit_enabled << " |"
			    << " " << setw(10) << left << unit_type << "|" << endl;
		}
		msg << " -----+------+----------+----------+----------+----------+--------+--------+----------+--------+--------+-----------+-----------+------+------+-----------+";
	}

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	return;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::DumpRtmSpans(WORD boardId) const
{
	CBoard* pBoard = GetBoard(boardId);
	if (pBoard == NULL)
		return;

	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();
	if (NULL == pSpanslist)
		return;

	if (0 == pSpanslist->size())
		return;

	std::ostringstream msg;
	msg.setf(std::ios::left, std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "CSystemResources::DumpRtmSpans for board " << boardId << ":";

	std::set<CSpanRTM>::iterator i;
	for (i = pSpanslist->begin(); i != pSpanslist->end(); i++)
	{
		if (i->GetIsEnabled() != TRUE)
			continue;

		const CSpanRTM* pSpan = (&(*i));
		if (TYPE_SPAN_T1 != ((CSpanRTM*)(pSpan))->GetSpanType() && TYPE_SPAN_E1 != ((CSpanRTM*)(pSpan))->GetSpanType())
			continue;

		((CSpanRTM*)(pSpan))->Dump(msg);
	}

	PTRACE(eLevelInfoNormal, msg.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::DumpRtmSpans() const
{
	for (WORD boardId = 1; boardId <= BOARDS_NUM; boardId++)
	{
		DumpRtmSpans(boardId);
	}
}

////////////////////////////////////////////////////////////////////////////
DWORD CSystemResources::AllocateSSRCId(unsigned short dmaId, unsigned short mrcMcuId) //circular
{
	TRACEINTO << "MrcMcuId:" << mrcMcuId; //temp for cascade debug
	bool foundSSRC = false;
	while (!foundSSRC)
	{
		if (m_nextSSRCId >= MAX_PARTY_SSRC_IDS - 1)
		{
			PTRACE2INT(eLevelInfoNormal, " CSystemResources::AllocateSSRCId : m_nextSSRCId is reached the max value =", m_nextSSRCId);
			m_nextSSRCId = 1;
		}
		if (CanAllocateSSRC(dmaId, mrcMcuId, m_nextSSRCId))
		{
			foundSSRC = true;
		}
		else
			m_nextSSRCId++;
	}

	DWORD next = m_nextSSRCId;
	m_nextSSRCId++;

	return next;
}

////////////////////////////////////////////////////////////////////////////
bool CSystemResources::CanAllocateSSRC(unsigned short dmaId, unsigned short mrmId, WORD ssrc)
{
	unsigned int audioResult = GetDmaMrmMask(dmaId, mrmId, ssrc, AUDIO_PAYLOAD_TYPE);
	if (IVR_CSRC == audioResult)
	{
		return false;
	}
	int num_video_ssrc = MAX_NUM_RECV_STREAMS_FOR_VIDEO;
	for (int i = 0; i < num_video_ssrc; i++)
	{
		unsigned int result_video = GetDmaMrmMask(dmaId, mrmId, ssrc, (VIDEO_PAYLOAD_TYPE + i));
		if (IVR_CSRC == result_video)
		{
			return false;
		}
	}
	int num_content_ssrc = MAX_NUM_RECV_STREAMS_FOR_CONTENT;
	for (int i = 0; i < num_content_ssrc; i++)
	{
		unsigned int result_content = GetDmaMrmMask(dmaId, mrmId, ssrc, (CONTENT_PAYLOAD_TYPE + i));
		if (IVR_CSRC == result_content)
		{
			return false;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////
unsigned int CSystemResources::GetDmaMrmMask(unsigned short dmaId, unsigned short mrmId, WORD ssrc, WORD mask)
{
	// DMA ID - 6 bits, MCU ID - 10 bits, Stream ID - 16 bits
	// we split the Stream ID to consist of <SRC ID, Payload ID>
	// where the Payload ID is the 6 bits (by Avishay H.)

	unsigned int result = 0;
	result |= (dmaId << 26); // DMA ID
	result |= (mrmId << 16); // MRM ID
	result |= (ssrc << 6);   // SSRC ID
	result  = result | mask;

	return result;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::CollectAdditonalAvcSvcMixVideoAvailabilityForBoard(VideoDataPerBoardStruct& videoAlloc, PartyDataStruct& partyData, BoardID boardId, bool& canVideoRequestBeSatisfied)
{
	canVideoRequestBeSatisfied = false;
	videoAlloc.m_bCanBeAllocated = FALSE;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN(!pBoard);

	CollectBandWidthAvailabilityForBoard(videoAlloc, partyData, boardId);
	STATUS status = FindOptUnitsAdditonalVideoAvcSvcMix(partyData, partyData.m_videoPartyType, videoAlloc, boardId);

	if (STATUS_OK == status)
	{
		float freePortCapacity = 0;
		canVideoRequestBeSatisfied = true;
		videoAlloc.m_bCanBeAllocated = TRUE;
		videoAlloc.m_numFreeVideoCapacity = (int)GetFreeVideoCapacity(boardId, &freePortCapacity);
		videoAlloc.m_NumVideoPartiesSameConf = GetNumVideoParties(boardId, partyData.m_monitor_conf_id);
		videoAlloc.m_numFreeVideoPortsCapacity = (int)freePortCapacity;
		return;
	}

	TRACEINTO << "BoardId:" << boardId << ", Status:" << status << " - Request cannot be satisfied";
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::FindOptUnitsAdditonalVideoAvcSvcMix(PartyDataStruct& partyData, eVideoPartyType videoPartyType, VideoDataPerBoardStruct& videoAlloc, int reqBoardId)
{
	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard == NULL, STATUS_FAIL);

	if (FALSE == IsBoardIdExists(reqBoardId))
	{
		TRACEINTO << " Failed, board not exist ,returning STATUS_INSUFFICIENT_VIDEO_RSRC , reqBoardId = " << reqBoardId;
		return STATUS_INSUFFICIENT_VIDEO_RSRC;
	}

	TRACEINTO << "board_id:" << reqBoardId << ", videoPartyType:" << eVideoPartyTypeNames[videoPartyType];

	pBoard->FillAdditionalAvcSvcMixVideoUnitsList(videoPartyType, videoAlloc.m_VideoAlloc, 0, 0, partyData.m_HdVswTypeInMixAvcSvcMode);

	STATUS status = STATUS_OK;
	CProcessBase* pProcess = CResourceProcess::GetProcess();
	CSysConfig* pSysConfig = pProcess ? pProcess->GetSysConfig() : NULL;
	BOOL is_spread_video_ports = NO;
	if (pSysConfig)
		pSysConfig->GetBOOLDataByKey("SPREADING_VIDEO_PORTS", is_spread_video_ports);

	DumpBoardUnits(videoAlloc, reqBoardId);

	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	for (int unitIndex = 0; unitIndex < max_units_video; unitIndex++)
	{
		if (videoAlloc.m_VideoAlloc.m_unitsList[unitIndex].GetTotalNeededCapacity() > 0)
		{
			if (CHelperFuncs::IsBreeze(pBoard->GetCardType()) || CHelperFuncs::IsSoftCard(pBoard->GetCardType()))
				status = FindOptBreezeUnit(partyData, videoAlloc, reqBoardId, unitIndex, is_spread_video_ports);
			else if (CHelperFuncs::IsMpmRxOrNinja(pBoard->GetCardType()))
				status = FindOptNetraUnit(partyData, videoAlloc, reqBoardId, unitIndex);

			if (status != STATUS_OK)
				break;
		}
	}

	if (status != STATUS_OK)
	{
		status = STATUS_INSUFFICIENT_VIDEO_RSRC;
		TRACEINTO << "Failed, returning STATUS_INSUFFICIENT_VIDEO_RSRC";
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::CheckIfConfVideoCanBeUpgraded(size_t (&additionalPorts)[ePartyType_Max][NUM_OF_PARTY_RESOURCE_TYPES])
{
	BOOL isLimit3CifPortsOnDsp = FALSE;
	if (CResRsrcCalculator::IsRMX1500Q())
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_3CIF_PORTS_ON_DSP, isLimit3CifPortsOnDsp);

	std::ostringstream msg;

	for (BoardID boardId = 1; boardId <= BOARDS_NUM; ++boardId)
	{
		int count = std::count_if(&additionalPorts[ePartyType_Avc][e_Audio], &additionalPorts[ePartyType_Avc][NUM_OF_PARTY_RESOURCE_TYPES], std::bind1st(std::not_equal_to<size_t>(), 0));
		if (!count)
			break;

		if (!IsBoardIdExists(boardId))
			continue;

		CBoard* pBoard = GetBoard(boardId);
		PASSERT_AND_RETURN_VALUE(!pBoard, STATUS_FAIL);

		eCardType cardType = pBoard->GetCardType();

		float cif_capacity = 0, sd_capacity = 0, hd_capacity = 0;
		float cif_encoder_weight = 0, sd_encoder_weight = 0;

		if (CHelperFuncs::IsMpmRxOrNinja(cardType))
		{
			cif_capacity = VID_ENC_CIF_PROMILLES_MPMRX;
			sd_capacity = VID_ENC_SD30_PROMILLES_MPMRX;
			hd_capacity = VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_MPMRX;
		}
		else if (CHelperFuncs::IsBreeze(cardType))
		{
			bool isNotRMX1500QOrFlagIsOn = (isLimit3CifPortsOnDsp || !CResRsrcCalculator::IsRMX1500Q());

			cif_capacity = VID_ENC_CIF_PROMILLES_BREEZE;
			sd_capacity = VID_ENC_SD30_PROMILLES_BREEZE;
			hd_capacity = VID_ENC_HD720_30FS_SYMMETRIC_PROMILLES_BREEZE;

			cif_encoder_weight = isNotRMX1500QOrFlagIsOn ? VID_ENC_CIF_WEIGHT_MODE2_MPMX : VID_ENC_CIF_WEIGHT_MPMX;
			sd_encoder_weight = isNotRMX1500QOrFlagIsOn ? VID_ENC_SD30_WEIGHT_MODE2_MPMX : VID_ENC_SD30_WEIGHT_MPMX;
		}
		else if (CHelperFuncs::IsSoftCard(cardType))
		{
			cif_capacity = VID_ENC_CIF_PROMILLES_SOFT_MPMX;
			sd_capacity = VID_ENC_SD_PROMILLES_SOFT_MPMX;
			hd_capacity = VID_ENC_HD_PROMILLES_SOFT_MPMX;

			cif_encoder_weight = 0;
			sd_encoder_weight = 0;
		}
		else
		{
			PASSERT(1);
		}

		msg << "\n\tBoardId:" << boardId << " capacity [CIF:" << setw(3) << cif_capacity << ", SD:" << setw(3) << sd_capacity << ", HD:" << setw(3) << hd_capacity << "]";

		CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

		std::set<CUnitMFA>::iterator _iiEnd = pMediaUnitslist->end();
		for (std::set<CUnitMFA>::iterator _iiUnit = pMediaUnitslist->begin(); _iiUnit != _iiEnd; ++_iiUnit)
		{
			if (!_iiUnit->GetIsEnabled())
				continue;  // Unit rejected because not enabled
			if (_iiUnit->GetUnitType() != eUnitType_Video)
				continue;  // Unit rejected because not video

			UnitID unit_id = _iiUnit->GetUnitId();
			UnitID physical_unit_id = _iiUnit->GetPhysicalUnitId();

			if (CHelperFuncs::IsBreeze(cardType) || CHelperFuncs::IsSoftCard(cardType))
			{
				float free_encoder_weight = _iiUnit->GetFreeEncoderWeight();
				if ((free_encoder_weight + FLOATING_POINT_ERROR) < cif_encoder_weight)
					continue;  // Unit rejected because free encoder ports weight is less than needed

				float free_capacity = _iiUnit->GetFreeCapacity(0);
				if (free_capacity < cif_capacity)
					continue;  // Unit/accelerator rejected because free capacity less than needed

				msg << "\n\t\tUnitId:" << unit_id << ", PhysUnitId:" << physical_unit_id << ", FreeCapacity:" << free_capacity << ", FreeEncoderWeight:" << free_encoder_weight;

				while (additionalPorts[ePartyType_Avc][e_SD30] && free_capacity >= sd_capacity && (free_encoder_weight + FLOATING_POINT_ERROR) >= sd_encoder_weight)
				{
					--additionalPorts[ePartyType_Avc][e_SD30];
					free_capacity -= sd_capacity;
					free_encoder_weight -= sd_encoder_weight;
					msg << " + SD enc";
					// Try to allocate 2 Cif encoders in the same unit as SD30 encoder
					while (additionalPorts[ePartyType_Avc][e_Cif] && free_capacity >= cif_capacity && (free_encoder_weight + FLOATING_POINT_ERROR) >= cif_encoder_weight)
					{
						--additionalPorts[ePartyType_Avc][e_Cif];
						free_capacity -= cif_capacity;
						free_encoder_weight -= cif_encoder_weight;
						msg << " + CIF enc";
					}
				}
				while (additionalPorts[ePartyType_Avc][e_Cif] && free_capacity >= cif_capacity && (free_encoder_weight + FLOATING_POINT_ERROR) >= cif_encoder_weight)
				{
					--additionalPorts[ePartyType_Avc][e_Cif];
					free_capacity -= cif_capacity;
					free_encoder_weight -= cif_encoder_weight;
					msg << " + CIF enc";
				}
				while (additionalPorts[ePartyType_Svc][e_HD1080p30] && free_capacity >= (2 * hd_capacity))
				{
					--additionalPorts[ePartyType_Svc][e_HD1080p30];
					free_capacity -= (2 * hd_capacity);
					msg << " + 1080p dec";
				}
				while (additionalPorts[ePartyType_Svc][e_HD720] && free_capacity >= hd_capacity)
				{
					--additionalPorts[ePartyType_Svc][e_HD720];
					free_capacity -= hd_capacity;
					msg << " + 720p dec";
				}
				while (additionalPorts[ePartyType_Svc][e_SD30] && free_capacity >= sd_capacity)
				{
					--additionalPorts[ePartyType_Svc][e_SD30];
					free_capacity -= sd_capacity;
					msg << " + SD dec";
				}
			}
			else if (CHelperFuncs::IsMpmRxOrNinja(cardType))
			{
				DWORD free_bandwidth_in = TOTAL_POSTSCALER_BW_IN - _iiUnit->GetUtilizedBandwidth(TRUE);
				DWORD free_bandwidth_out = TOTAL_POSTSCALER_BW_OUT - _iiUnit->GetUtilizedBandwidth(FALSE);

				for (int i = 0; i < ACCELERATORS_PER_UNIT_NETRA; ++i)
				{
					float free_capacity = _iiUnit->GetFreeCapacity(i);
					if (free_capacity < cif_capacity)
						continue;  // Unit/accelerator rejected because free capacity less than needed

					msg << "\n\t\tUnitId:" << unit_id << ", PhysUnitId:" << physical_unit_id << ", AcceleratorId:" << i << ", FreeCapacity:" << free_capacity << ", FreeBandwidthIn:" << free_bandwidth_in << ", FreeBandwidthOut:" << free_bandwidth_out;

					while (additionalPorts[ePartyType_Avc][e_SD30] && free_capacity >= sd_capacity && free_bandwidth_in >= VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC)
					{
						--additionalPorts[ePartyType_Avc][e_SD30];
						free_capacity -= sd_capacity;
						free_bandwidth_in -= VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC;
						msg << " + SD enc";
						// Try to allocate 1 Cif encoders in the same unit as SD30 encoder (should come as a pair)
						if (additionalPorts[ePartyType_Avc][e_Cif] && free_capacity >= cif_capacity && free_bandwidth_in >= VID_CIF_BW_MPMRX_NINJA_ENC_DEC)
						{
							--additionalPorts[ePartyType_Avc][e_Cif];
							free_capacity -= cif_capacity;
							free_bandwidth_in -= VID_CIF_BW_MPMRX_NINJA_ENC_DEC;
							msg << " + CIF enc";
						}
					}
					while (additionalPorts[ePartyType_Avc][e_Cif] && free_capacity >= cif_capacity && free_bandwidth_in >= VID_CIF_BW_MPMRX_NINJA_ENC_DEC)
					{
						--additionalPorts[ePartyType_Avc][e_Cif];
						free_capacity -= cif_capacity;
						free_bandwidth_in -= VID_CIF_BW_MPMRX_NINJA_ENC_DEC;
						msg << " + CIF enc";
					}
					while (additionalPorts[ePartyType_Svc][e_HD1080p30] && free_capacity >= (2 * hd_capacity) && free_bandwidth_out >= (VID_HD_1080_30_BW_MPMRX_NINJA_ENC_DEC))
					{
						--additionalPorts[ePartyType_Svc][e_HD1080p30];
						free_capacity -= (2 * hd_capacity);
						free_bandwidth_out -= (VID_HD_1080_30_BW_MPMRX_NINJA_ENC_DEC);
						msg << " + 1080p dec";
					}
					while (additionalPorts[ePartyType_Svc][e_HD720] && free_capacity >= hd_capacity && free_bandwidth_out >= VID_HD_720_30_BW_MPMRX_NINJA_ENC_DEC)
					{
						--additionalPorts[ePartyType_Svc][e_HD720];
						free_capacity -= hd_capacity;
						free_bandwidth_out -= VID_HD_720_30_BW_MPMRX_NINJA_ENC_DEC;
						msg << " + 720p dec";
					}
					while (additionalPorts[ePartyType_Svc][e_SD30] && free_capacity >= sd_capacity && free_bandwidth_out >= VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC)
					{
						--additionalPorts[ePartyType_Svc][e_SD30];
						free_capacity -= sd_capacity;
						free_bandwidth_out -= VID_WSD_W4CIF_BW_MPMRX_NINJA_ENC_DEC;
						msg << " + SD dec";
					}
				}
			}
			int count = std::count_if(&additionalPorts[ePartyType_Avc][e_Audio], &additionalPorts[ePartyType_Avc][NUM_OF_PARTY_RESOURCE_TYPES], std::bind1st(std::not_equal_to<size_t>(), 0));
			if (!count)
				break;
		}
	}

	int count = std::count_if(&additionalPorts[ePartyType_Avc][e_Audio], &additionalPorts[ePartyType_Avc][NUM_OF_PARTY_RESOURCE_TYPES], std::bind1st(std::not_equal_to<size_t>(), 0));
	if (!count)
	{
		TRACEINTO << msg.str().c_str() << "\nSucceeded, enough video ports";
		return STATUS_OK;
	}
	TRACEINTO << msg.str().c_str() << "\nFailed, not enough video ports";
	return STATUS_INSUFFICIENT_VIDEO_RSRC;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::UpdateLogicalWeightsAndValues()
{
	m_ResourcesInterfaceArray.UpdateLogicalWeightsAndValues(m_SystemCardsMode);

	if (GetMultipleIpServices())
	{
		for (std::set<CIpServiceResourcesInterfaceArray>::iterator _itr = m_IpServicesResourcesInterface->begin(); _itr != m_IpServicesResourcesInterface->end(); _itr++)
			((CIpServiceResourcesInterfaceArray*)(&(*_itr)))->UpdateLogicalWeightsAndValues(m_SystemCardsMode);
	}
}

////////////////////////////////////////////////////////////////////////////
bool CSystemResources::SetLicenseExpired(bool isLicenseExpired)
{
	bool shouldContinue = true;

	if ((!m_isLicenseExpired && isLicenseExpired) || isLicenseExpired)
	{
		m_isLicenseExpired = isLicenseExpired;
		if (m_ResourcesInterfaceArray.IsInterfaceArrayInitiated())
		{
			if (GetCurrentResourcesInterface())
				GetCurrentResourcesInterface()->OnLicenseExpired();
		}
		shouldContinue = false;
	}
	else if (m_isLicenseExpired && !isLicenseExpired)
	{
		m_isLicenseExpired = isLicenseExpired;
	}

	TRACEINTO << "ShouldContinue:" << (DWORD)shouldContinue << ", IsLicenseExpired:" << (DWORD)m_isLicenseExpired;

	return shouldContinue;
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::SetAllFreePortsToFreeDisable(PhysicalPortDesc* pPortDesc)
{
	WORD bId = pPortDesc->m_boardId;
	WORD uId = pPortDesc->m_unitId;

	CSmallString sstr;
	sstr << "bId = " << bId << " , uId = " << uId;
	PTRACE2(eLevelInfoNormal, "CSystemResources::SetAllFreePortsToFreeDisable: ", sstr.GetString());

	CUnitMFA* pUnitMFA = GetUnit(bId, uId);
	PASSERT_AND_RETURN(pUnitMFA==NULL);

	pUnitMFA->SetAllFreePortsToFreeDisable();
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::SimulateAllocateUDP(UdpAddresses& udp)
{
	WORD i;

	for (i = 0; i < 3; i++)
	{
		if (FreeFixedUDP[i])
		{
			FreeFixedUDP[i] = 0;
			memcpy(&udp, &(UdpAdressesArray[i]), sizeof(UdpAddresses));
			return i;
		}
	}

	return 0xFF;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::SimulateDeAllocateUDP(WORD id)
{
	if (id > 2)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	if (FreeFixedUDP[id])
		PASSERT(1);

	FreeFixedUDP[id] = 1;
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::FindARTunits(CBoard* pBoard, PartyDataStruct& partyData, eUnitType needed_type, CUnitMFA**& return_find_arr, BOOL isReqUnitId)
{
	WORD ret_counter = 0;

	BOOL isSvcOnly = (partyData.m_confMediaType == eSvcOnly) ? TRUE : FALSE;
	BOOL isAudioOnly = (partyData.m_videoPartyType == eVideo_party_type_none) ? TRUE : FALSE;

	float needed_capacity = pBoard->CalculateNeededPromilles(PORT_ART, partyData.m_artCapacity, isSvcOnly, isAudioOnly, partyData.m_videoPartyType);
	WORD needed_ART_channels = CHelperFuncs::GetNumArtChannels(partyData);
	BOOL bIsTipMasterAllocation = (partyData.m_tipNumOfScreens > 0 && partyData.m_partyTypeTIP == eTipMasterCenter);
	BOOL isMpmRx = CHelperFuncs::IsMpmRx(pBoard->GetCardType());

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	BYTE bRestrictArtAllocation = YES;
	BOOL isTrafficShapingEnabled = NO;

	// If traffic shaping is enabled in the system, art must be restricted to 100%
	std::string key = CFG_KEY_ENABLE_RTP_TRAFFIC_SHAPING;
	sysConfig->GetBOOLDataByKey(key, isTrafficShapingEnabled);
	if (!isTrafficShapingEnabled)
	{
		sysConfig->GetBOOLDataByKey("RESTRICT_ART_ALLOCATION_ACCORDING_TO_WEIGHT", bRestrictArtAllocation);
	}

	std::set<CUnitMFA>::iterator i;

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	if (!pMediaUnitslist || pMediaUnitslist->size() == 0)
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::FindARTunits - no enable units on the board=", pBoard->GetOneBasedBoardId());
		return ret_counter;
	}

	for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
	{
		if (i->GetUnitType() != needed_type || i->GetIsEnabled() != TRUE)
			continue;

		if (isReqUnitId && partyData.m_reqArtUnitId != i->GetUnitId())  // TIP Cisco: allocate all ART's on same unit
			continue;

		if (isMpmRx)
		{
			//For MPM-Rx card,
			//If RESTRICT_ART_ALLOCATION_ACCORDING_TO_WEIGHT is YES, choose the unit with enough promilles.
			//If NO, choose the unit whose acvite ports doesn't exceed 20.
			if ((YES == bRestrictArtAllocation) && (i->GetFreeCapacity() < needed_capacity))
			{
				PTRACE2INT(eLevelInfoNormal, "CSystemResources::FindARTunits, system flag is YES, this unit have no enough capacity,skip it,unit id: ", i->GetUnitId());
				continue;
			}
			else if ((NO == bRestrictArtAllocation) && i->GetPortNumber() >= 1000 / ART_PROMILLES_AUDIO_OR_SVC_ONLY_MPMRX)
			{
				PTRACE2INT(eLevelInfoNormal, "CSystemResources::FindARTunits, system flag is NO, this unit port exceed 20,skip it,unit id: ", i->GetUnitId());
				continue;
			}
			PTRACE2INT(eLevelInfoNormal, "CSystemResources::FindARTunits, unit is selected as a candidates:", i->GetUnitId());
		}
		else
		{
			if (i->GetFreeCapacity() < needed_capacity) //Insufficient capacity
				continue;
		}

		if (i->GetFreeARTChannels() < needed_ART_channels)
			continue;

		WORD maxNumOfTipScreensPerArt = 0;

		if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
		{
			maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART_SOFTMCU;
		}
		else
		{
			maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART;
		}

		// TIP: There is a limitation of 3 TIP EP screens per ART DSP (i.e. 1 CTS3000 or 3 CTS1000)
		if (bIsTipMasterAllocation && (i->GetNumAllocatedTipScreens() + partyData.m_tipNumOfScreens > maxNumOfTipScreensPerArt))
			continue;

		return_find_arr[ret_counter] = (CUnitMFA*)&(*i);
		ret_counter++;
	}
	return ret_counter;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::AllocateART(ConfRsrcID confId, PartyRsrcID partyId, DWORD partyCapacity, eResourceTypes physType, WORD serviceId, WORD subServiceId, PhysicalPortDesc* pPortDesc, PartyDataStruct& partyData, BOOL isIceParty, BOOL isBFCPUDP, WORD reqBoardId) //ICE 4 ports
{
	std::ostringstream msg;
	msg << "CSystemResources::AllocateART:"
		<< "\n  ConfId              :" << confId
		<< "\n  PartyId             :" << partyId
		<< "\n  PartyCapacity       :" << partyCapacity
		<< "\n  ResourceType        :" << ResourceTypeToString(physType)
		<< "\n  ServiceId           :" << serviceId
		<< "\n  SubServiceId        :" << subServiceId
		<< "\n  ReqBoardId          :" << reqBoardId
		<< "\n  ReqArtUnitId        :" << partyData.m_reqArtUnitId;

	if (confId == 0)
	{
		PTRACE(eLevelInfoNormal, msg.str().c_str());
		return STATUS_FAIL;
	}

	if (physType != ePhysical_art && physType != ePhysical_art_light)
	{
		PTRACE(eLevelInfoNormal, msg.str().c_str());
		PASSERT(physType);
		return STATUS_FAIL;
	}

	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, STATUS_INSUFFICIENT_ART_RSRC);

	BOOL isSvcOnly = (partyData.m_confMediaType == eSvcOnly) ? TRUE : FALSE;
	BOOL isAudioOnly = (partyData.m_videoPartyType == eVideo_party_type_none) ? TRUE : FALSE;

	WORD needed_promilles = (WORD)pBoard->CalculateNeededPromilles(PORT_ART, partyCapacity, isSvcOnly, isAudioOnly, partyData.m_videoPartyType);
	eUnitType needed_type = PhysicalToUnitType(physType);
	WORD needed_ART_channels = CHelperFuncs::GetNumArtChannels(partyData);

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	if (pMediaUnitslist->size() == 0)
	{
		PTRACE(eLevelInfoNormal, msg.str().c_str());
		PASSERTMSG(1, "No Resources Enabled!!!");
	}

	WORD arr_size = pMediaUnitslist->size();

	CUnitMFA** find_arr = new CUnitMFA*[arr_size];
	for (int as = 0; as < arr_size; as++)
		find_arr[as] = NULL; // after I will insert define

	msg
		<< "\n  NeededPromilles     :" << needed_promilles
		<< "\n  NeededChannels      :" << needed_ART_channels
		<< "\n  NumOfUnitsOnBoard   :" << arr_size;

	WORD cntr = 0;

	BOOL isTIPSlave = (partyData.m_room_id != 0xFFFF && CHelperFuncs::IsTIPSlavePartyType(partyData.m_partyTypeTIP));
	BOOL isAvMcuSlaveOut = (eParty_Role_AvMcuLink_SlaveOut == partyData.m_partyRole);

	if (isTIPSlave || isAvMcuSlaveOut)
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::AllocateART - find ART for TIP/AvMcu party from room_id=", partyData.m_room_id);
		cntr = FindARTunits(pBoard, partyData, needed_type, find_arr, TRUE);
	}
	if (0 == cntr)
		cntr = FindARTunits(pBoard, partyData, needed_type, find_arr, FALSE);

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	WORD found_id = 0xFFFF;
	WORD tipNumOfScreens = (partyData.m_tipNumOfScreens > 0 && partyData.m_partyTypeTIP == eTipMasterCenter) ? partyData.m_tipNumOfScreens : 0;
	STATUS status = GetBestARTAndCheckUDP(find_arr, serviceId, subServiceId, arr_size, partyCapacity, needed_promilles, partyData.m_videoPartyType, found_id, isIceParty, isBFCPUDP); //ICE 4 ports

	if (STATUS_OK != status || found_id == 0xFFFF) //not found at all, fail
	{
		delete[] find_arr;
		PTRACE(eLevelInfoNormal, "CSystemResources::AllocateART - failed to allocate 1");
		return STATUS_INSUFFICIENT_ART_RSRC; //status fail
	}

	MediaUnit artUnit;
	memset(&artUnit, 0, sizeof(artUnit));
	artUnit.m_MediaPortsList[0].m_needed_capacity_promilles = needed_promilles;
	artUnit.m_MediaPortsList[0].m_type = eLogical_audio_encoder;
	int ret = find_arr[found_id]->AllocatePorts(confId, partyId, partyCapacity, artUnit, partyData.m_videoPartyType, tipNumOfScreens);

	// DumpAllActivePorts in 4.7.1 if more then 6 parties per art
	find_arr[found_id]->DumpAllActivePorts(6);
	if (ret == -1)
	{
		delete[] find_arr;
		PTRACE(eLevelInfoNormal, "CSystemResources::AllocateART - failed to allocate 2");
		return STATUS_INSUFFICIENT_ART_RSRC;
	}

	if (needed_ART_channels > 0)
		find_arr[found_id]->AllocateARTChannels(needed_ART_channels);

	//***fill physical descriptor of port for output
	pPortDesc->m_boxId = 1;
	pPortDesc->m_subBoardId = 1;
	pPortDesc->m_boardId = find_arr[found_id]->GetBoardId();
	pPortDesc->m_unitId = find_arr[found_id]->GetUnitId();
	pPortDesc->m_portId = artUnit.m_MediaPortsList[0].m_portId;
	pPortDesc->m_acceleratorId = artUnit.m_MediaPortsList[0].m_acceleratorId;

	delete[] find_arr;
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void PrintNumeric(std::ostringstream& msg, int numeric)
{
	//-----------+
	if (numeric == -1)
		msg << " NA        |";
	else if (numeric < 10)
		msg << " " << numeric << "         |";
	else if (numeric < 100)
		msg << " " << numeric << "        |";
	else if (numeric < 1000)
		msg << " " << numeric << "       |";
	else if (numeric < 10000)
		msg << " " << numeric << "      |";
	else if (numeric < 100000)
		msg << " " << numeric << "     |";
	else if (numeric < 1000000)
		msg << " " << numeric << "    |";
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::GetBestARTSpreadUnits(CUnitMFA** find_arr, WORD arr_size, WORD reqTipNumOfScreens, WORD &best_id)
{
	float curr_capacity, max_found_capacity = -2000;

	WORD maxNumOfTipScreensPerArt = 0;

	if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART_SOFTMCU;
	}
	else
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART;
	}

	int curr_num_of_tip_screens = 0;
	WORD min_free_num_of_tip_screens = maxNumOfTipScreensPerArt + 1; // set to maxNumOfTipScreensPerArt+1 because free unit will replace it;

	for (WORD i = 0; i < arr_size; i++)
	{
		if (NULL != find_arr[i])
		{

			curr_num_of_tip_screens = find_arr[i]->GetNumAllocatedTipScreens();
			curr_capacity = find_arr[i]->GetFreeCapacity();

			if (reqTipNumOfScreens > 0)
			{
				// we try to fill unit with tip_screens up to the limit of maxNumOfTipScreensPerArt
				WORD free_num_of_tip_screens = (maxNumOfTipScreensPerArt - curr_num_of_tip_screens);

				if (free_num_of_tip_screens < reqTipNumOfScreens || (free_num_of_tip_screens - reqTipNumOfScreens) > min_free_num_of_tip_screens)
				{
					// 1. if no capacity for reqTipNumOfScreens - continue
					//    if already found better unit (we try to fill units with tip_screens) - continue
					continue;
				}
				else if (free_num_of_tip_screens - reqTipNumOfScreens < min_free_num_of_tip_screens)
				{
					// 2. found unit with better num_of_tip_screens count (we try to fill unit with tip_screens up to the maxNumOfTipScreensPerArt)
					best_id = i;
					min_free_num_of_tip_screens = free_num_of_tip_screens - reqTipNumOfScreens;
					// update for second priority
					max_found_capacity = curr_capacity;
					continue;
				}
				// 3. if unit have same as best num_of_tip_screens count - we continue with current code (spread) policy by capicity and art_channels
				//else if( free_num_of_tip_screens - reqTipNumOfScreens == min_free_num_of_tip_screens)
			}
			if (curr_capacity > max_found_capacity) //found less loaded DSP
			{
				best_id = i;
				max_found_capacity = curr_capacity;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::GetBestARTSmartSpreadUnits(CUnitMFA** find_arr, WORD arr_size, WORD reqTipNumOfScreens, float needed_promilles, WORD& best_id)
{
	best_id = 0xFFFF;

	std::ostringstream msg;
	msg << "CSystemResources::GetBestARTSmartSpreadUnits:\n";
	msg << "  +-----------+-----------+-----------+-----------+-----------+-----------+\n" << "  | ART index | Board Id  | Unit Id   | Promilles | Channels  | Tip ports |\n" << "  +-----------+-----------+-----------+-----------+-----------+-----------+";

	float curr_capacity, max_found_capacity = -2000; // max_found_capacity = 0, since capacity may smaller than 0, change the init value.

	WORD maxNumOfTipScreensPerArt = 0;

	if (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART_SOFTMCU;
	}
	else
	{
		maxNumOfTipScreensPerArt = MAX_NUM_OF_TIP_SCREENS_PER_ART;
	}

	int curr_art_channels = 0, min_occupied_art_channels = 0, curr_num_of_tip_screens = 0;
	WORD min_free_num_of_tip_screens = maxNumOfTipScreensPerArt + 1; // set to maxNumOfTipScreensPerArt+1 because free unit will replace it;

	WORD unitRecoveryReserved = 0xFFFF;
	for (WORD i = 0; i < arr_size; i++)
	{
		if (NULL != find_arr[i])
		{
			WORD unitId = find_arr[i]->GetUnitId();
			WORD boardId = find_arr[i]->GetBoardId();
			msg << "\n  |";
			PrintNumeric(msg, i);
			PrintNumeric(msg, boardId);
			PrintNumeric(msg, unitId);
			if (find_arr[i]->IsRecoveryReservedUnit() && find_arr[i]->GetTotalAllocatedCapacity() == 0)
			{
				msg << " reserved for recovery |";
				unitRecoveryReserved = i;
				continue;
			}
			curr_capacity = find_arr[i]->GetFreeCapacity();
			curr_art_channels = find_arr[i]->GetOccupiedARTChannels();
			curr_num_of_tip_screens = find_arr[i]->GetNumAllocatedTipScreens();
			PrintNumeric(msg, (int)curr_capacity);
			PrintNumeric(msg, (int)curr_art_channels);
			PrintNumeric(msg, (int)curr_num_of_tip_screens);

			if (reqTipNumOfScreens > 0)
			{
				// we try to fill unit with tip_screens up to the limit of maxNumOfTipScreensPerArt
				WORD free_num_of_tip_screens = (maxNumOfTipScreensPerArt - curr_num_of_tip_screens);

				if (free_num_of_tip_screens < reqTipNumOfScreens || (free_num_of_tip_screens - reqTipNumOfScreens) > min_free_num_of_tip_screens)
				{
					// 1. if no capacity for reqTipNumOfScreens - continue
					//    if already found better unit (we try to fill units with tip_screens) - continue
					continue;
				}
				else if (free_num_of_tip_screens - reqTipNumOfScreens < min_free_num_of_tip_screens)
				{
					// 2. found unit with better num_of_tip_screens count (we try to fill unit with tip_screens up to the maxNumOfTipScreensPerArt)
					best_id = i;
					min_free_num_of_tip_screens = free_num_of_tip_screens - reqTipNumOfScreens;

					// update for second priority
					max_found_capacity = curr_capacity;
					min_occupied_art_channels = curr_art_channels;
					continue;
				}
				// 3. if unit have same as best num_of_tip_screens count - we continue with current code (spread) policy by capicity and art_channels
				//else if( free_num_of_tip_screens - reqTipNumOfScreens == min_free_num_of_tip_screens)
			}

			if (curr_capacity > max_found_capacity)
			{ //found less loaded DSP
				best_id = i;
				max_found_capacity = curr_capacity;
				min_occupied_art_channels = curr_art_channels;
			}
			else if (curr_capacity == max_found_capacity && max_found_capacity != 0)
			{ // found same capacity unit with less art channels
				if (curr_art_channels < min_occupied_art_channels)
				{
					best_id = i;
					min_occupied_art_channels = curr_art_channels;
				}
			}
		}
	}
	msg << "\n  +-----------+-----------+-----------+-----------+-----------+-----------+\n";

	//For MPM-Rx, if RESTRICT_ART_ALLOCATION_ACCORDING_TO_WEIGHT is NO, the found capacity may smaller than needed promilles,
	//In this case, the recovery unit will be choosen as the target one.
	//Only after all the recovery unit are full, we will choose the unit that exceed the maximum capacity.
	if ((max_found_capacity < needed_promilles) && (0xFFFF != unitRecoveryReserved))
	{
		msg << " found capacity:" << max_found_capacity << " is smaller than needed_promilles:" << needed_promilles << " ,change found id to 0xffff.";
		best_id = 0xFFFF;
	}

	if (best_id != 0xFFFF && (NULL != find_arr[best_id]))
	{
		msg << "  Found best ART: index=" << best_id << ", free_capacity=" << max_found_capacity << ", art_channels=" << min_occupied_art_channels;
	}
	else
	{
		if (unitRecoveryReserved != 0xFFFF && find_arr[unitRecoveryReserved])
		{
			best_id = unitRecoveryReserved;
			msg << "  Found best ART (Recovery Reserved): index=" << best_id;
		}
		else
			msg << "  Failed to find Best ART find_arr all NULL";
	}
	PTRACE(eLevelInfoNormal, msg.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::GetBestARTCapacitySpreadUnits(CUnitMFA** find_arr, WORD arr_size, DWORD partyCapacity, WORD& best_id)
{
	best_id = 0xFFFF;
	//WORD max_difference_capacity = 0;
	DWORD min_total_capacity = 0xFFFFFFFF;
	WORD min_allocated_ports = 10;
	int unitRecoveryReserved = 0xFFFF;

	std::ostringstream msg;
	msg << "CSystemResources::GetBestARTCapacitySpreadUnits: arr_size = " << arr_size << "\n";
	msg << "  +-----------+-----------+-----------+-----------+-----------+-----------+\n" << "  | ART index | Board Id  | Unit Id   | Ports Sum | Average Kb| Total Kb  |\n" << "  +-----------+-----------+-----------+-----------+-----------+-----------+";

	for (WORD i = 0; i < arr_size; i++)
	{
		if (NULL != find_arr[i])
		{
			WORD unitId = find_arr[i]->GetUnitId();
			WORD boardId = find_arr[i]->GetBoardId();
			msg << "\n  |";
			PrintNumeric(msg, i);
			PrintNumeric(msg, boardId);
			PrintNumeric(msg, unitId);

			// If unit marked as recovery reserved and there is no allocation on it yet, then unit is recovery reserved
			if (find_arr[i]->IsRecoveryReservedUnit() && find_arr[i]->GetTotalAllocatedCapacity() == 0)
			{
				msg << " reserved for recovery             |";
				unitRecoveryReserved = i;
				continue;
			}
			DWORD cur_average_capacity = find_arr[i]->GetAverageAllocatedCapacity();
			DWORD cur_total_capacity = find_arr[i]->GetTotalAllocatedCapacity();
			WORD cur_allocated_ports = find_arr[i]->GetPortNumber();

			PrintNumeric(msg, cur_allocated_ports);
			PrintNumeric(msg, cur_average_capacity);
			PrintNumeric(msg, cur_total_capacity);

			if (cur_allocated_ports == 9)
			{
				continue; // Do not allocate more than 9 ports per ART
			}
			if (cur_total_capacity < min_total_capacity)
			{
				min_total_capacity = cur_total_capacity;
				min_allocated_ports = cur_allocated_ports;
				best_id = i;
				continue;
			}
			if (cur_total_capacity == min_total_capacity)
			{
				if (cur_allocated_ports < min_allocated_ports)
				{
					min_total_capacity = cur_total_capacity;
					min_allocated_ports = cur_allocated_ports;
					best_id = i;
					continue;
				}
			}
		}
	}
	msg << "\n  +-----------+-----------+-----------+-----------+-----------+-----------+\n";

	if (best_id != 0xFFFF && find_arr[best_id])
	{
		const DWORD MAX_ART_CAPACITY = 56 * 1024; // Avihay Barazany asked restrict the ART capacity with 56M and use Recovery ART
		if (find_arr[best_id]->GetTotalAllocatedCapacity() >= MAX_ART_CAPACITY)
			if (unitRecoveryReserved != 0xFFFF && find_arr[unitRecoveryReserved])
				best_id = unitRecoveryReserved;
	}
	else
	{
		if (unitRecoveryReserved != 0xFFFF && find_arr[unitRecoveryReserved])
			best_id = unitRecoveryReserved;
	}
	if (unitRecoveryReserved != 0xFFFF && best_id == unitRecoveryReserved)
		msg << "  Found best ART (Recovery Reserved): index=" << best_id << ", partyCapacity = " << partyCapacity;
	else if (best_id != 0xFFFF)
		msg << "  Found best ART: index=" << best_id << ", partyCapacity = " << partyCapacity;
	else
		msg << "  Failed to find Best ART: partyCapacity = " << partyCapacity;

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	PASSERTMSG(best_id == 0xFFFF || NULL == find_arr[best_id], "Failed to find Best ART");
}

////////////////////////////////////////////////////////////////////////////
void CSystemResources::GetBestARTDontSpreadUnits(CUnitMFA **find_arr, WORD arr_size, WORD &best_id)
{
	//first try to find a unit that is already allocated, between those, choose the one with the most free channels
	//if there's no such unit, take an empty one

	WORD empty_unit_id = 0xFFFF;
	BOOL bFoundNotEmptyUnit = FALSE;
	float curr_capacity;
	int currfreechannels, maxfreechannels = 0;
	for (WORD i = 0; i < arr_size; i++)
	{
		if (NULL != find_arr[i])
		{
			curr_capacity = find_arr[i]->GetFreeCapacity();
			currfreechannels = find_arr[i]->GetFreeARTChannels();

			/*
			 * 12 Dec 2012, Rafi Fellert, JIRA issue #46
			 * According to Olga S. this is relevant only for softMCUMfw.
			 */
			if (eProductTypeSoftMCUMfw == m_ProductType)
			{
				if (curr_capacity == 1000)
				{
					if (0xFFFF == empty_unit_id) //first empty unit
						empty_unit_id = i;
				}
				else if (curr_capacity != 0)
				{
					bFoundNotEmptyUnit = TRUE;
					best_id = i;
					break;
				}
			}
			else
			{
				if (curr_capacity == 1000) //empty unit
				{
					empty_unit_id = i;
				}
				else if (currfreechannels > maxfreechannels) //found not empty unit, with less channels occupied
				{
					bFoundNotEmptyUnit = TRUE;
					best_id = i;
					maxfreechannels = currfreechannels;
				}
			}
		}
	}

	if (!bFoundNotEmptyUnit)
	{
		best_id = empty_unit_id;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::GetBestARTAndCheckUDP(CUnitMFA** find_arr, WORD serviceId, WORD subServiceId, WORD arr_size, DWORD partyCapacity, float needed_promilles, eVideoPartyType videoPartyType, WORD& found_id, BOOL isIceParty, //ICE 4 ports
    BOOL isBFCPUDP, WORD tipNumOfScreens)
{
	PTRACE(eLevelInfoNormal, "CSystemResources::GetBestARTAndCheckUDP");

	WORD bestArtId = 0xFFFF;
	switch (m_ResourceAllocationType)
	{
		case eFixedBreezeMode:
		case eFixedMpmRxMode:
			GetBestARTSpreadUnits(find_arr, arr_size, tipNumOfScreens, bestArtId);
			break;

		case eAutoBreezeMode:
		case eAutoMpmRxMode:
		case eAutoMixedMode:
			// use GetBestARTSpreadUnits for eAutoBreezeMode: v7.0 to prevent DSP real-time problem on high rates HD
			// we have always 10 ART units so we will not use reconfiguration from ART to Video in current capacity.
			if (CHelperFuncs::IsEventMode())
				GetBestARTCapacitySpreadUnits(find_arr, arr_size, partyCapacity, bestArtId);
			else
				GetBestARTSmartSpreadUnits(find_arr, arr_size, tipNumOfScreens, needed_promilles, bestArtId);
			break;

		default:
			PASSERT(1);
			break;
	}

	STATUS status = STATUS_INSUFFICIENT_ART_RSRC;

	if (bestArtId != 0xFFFF)
	{
		if (serviceId != 0xFF) //check relevant for ip parties only
		{
			if (CheckUDPports(serviceId, subServiceId, find_arr[bestArtId]->GetBoxId(), find_arr[bestArtId]->GetBoardId(), find_arr[bestArtId]->GetSubBoardId(), find_arr[bestArtId]->GetUnitId(), videoPartyType, isIceParty, isBFCPUDP) == STATUS_OK) //ICE 4 ports
			{
				found_id = bestArtId;
				status = STATUS_OK;
			}
			else
			{
				find_arr[bestArtId] = NULL;
				status = GetBestARTAndCheckUDP(find_arr, serviceId, subServiceId, arr_size, partyCapacity, needed_promilles, videoPartyType, found_id, isIceParty, isBFCPUDP, tipNumOfScreens);
			}
		}
		else
		{
			found_id = bestArtId;
			status = STATUS_OK;
		}
	}

	if (status != STATUS_OK)
		PTRACE(eLevelInfoNormal, "CSystemResources::GetBestARTAndCheckUDP - failed (max_id is 0xFFFF)");

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeAllocateART(CConfRsrc* pConf, WORD rsrcPartyId, WORD ARTChannels, eResourceTypes physType, eVideoPartyType videoPartyType, PhysicalPortDesc* pPortDesc, BYTE dsbl, WORD numOfTipScreens)
{
	WORD rsrcConfId = pConf->GetRsrcConfId();
	if (rsrcConfId == 0) //not allocaed, some internal error
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	if (physType != ePhysical_art && physType != ePhysical_art_light)
	{
		PASSERT(physType);
		return STATUS_FAIL;
	}

	WORD bId = pPortDesc->m_boardId;
	WORD uId = pPortDesc->m_unitId;
	WORD portId = pPortDesc->m_portId;

	int ret = -1;

	CUnitMFA* pUnitMFA = GetUnit(bId, uId);
	PASSERT_AND_RETURN_VALUE(pUnitMFA==NULL, STATUS_ART_DEALLOCATION_FAIL);

	// ART channels are only used for ISDN. For IP Party it will be 0 (not in use).
	if (ARTChannels > 0)
		pUnitMFA->DeAllocateARTChannels(ARTChannels);

	ret = pUnitMFA->DeAllocatePort(portId, videoPartyType, dsbl, 0);

	pUnitMFA->DecreaseNumAllocatedTipScreens(numOfTipScreens);

	// DumpAllActivePorts in 4.7.1 if more then 6 parties per art
	pUnitMFA->DumpAllActivePorts(6);

	//POBJDELETE(pDesc);
	if (ret == 0)
		return STATUS_OK;
	else
	{
		PTRACE(eLevelInfoNormal, "CSystemResources::DeAllocateART deallocation failes");
		return STATUS_ART_DEALLOCATION_FAIL;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::ReAllocateART(DWORD confId, WORD partyId, ALLOC_PARTY_REQ_PARAMS_S* pParam)
{
	PASSERT_AND_RETURN_VALUE(!pParam, STATUS_FAIL);

	std::ostringstream msg;
	msg
		<< "\n  ConfId                     :" << confId
		<< "\n  PartyId                    :" << partyId
		<< "\n  MonitorConfId              :" << pParam->monitor_conf_id
		<< "\n  MonitorPartyId             :" << pParam->monitor_party_id
		<< "\n  ArtCapacity (Kbytes)       :" << pParam->artCapacity;

	std::vector<CActivePort>::iterator port;
	for (WORD boardId = 0; boardId < BOARDS_NUM; ++boardId)
	{
		CBoard* pBoard = m_pBoards[boardId];
		if (pBoard)
		{
			CMediaUnitsList* pMediaUnitsList = pBoard->GetMediaUnitsList();
			if (pMediaUnitsList)
			{
				for (std::set<CUnitMFA>::iterator mediaUnit = pMediaUnitsList->begin(); mediaUnit != pMediaUnitsList->end(); ++mediaUnit)
				{
					const CActivePortsList* pActivePorts = mediaUnit->GetActivePorts();
					if (pActivePorts)
					{
						for (std::set<CActivePort>::iterator port = pActivePorts->begin(); port != pActivePorts->end(); ++port)
						{
							if (confId == port->GetConfId() && partyId == port->GetPartyId())
							{
								msg
									<< "\n  OldCapacity (Kbytes)       :" << port->GetCapacity()
									<< "\n  UnitId                     :" << mediaUnit->GetUnitId()
									<< "\n  BoardId                    :" << mediaUnit->GetBoardId()
									<< "\n  PortsSum                   :" << mediaUnit->GetPortNumber();

								((CActivePort*)(&(*port)))->SetCapacity(pParam->artCapacity);

								//For MPM-Rx, the promilles is dynamic, it will change when partyCapacity changed.
								if (CHelperFuncs::IsMpmRx(pBoard->GetCardType()))
								{
									float oldNeededPromilles = ((CActivePort*)(&(*port)))->GetPromilUtilized();

									BOOL isSvcOnly = (pParam->confMediaType == eSvcOnly) ? TRUE : FALSE;
									BOOL isAudioOnly = (pParam->videoPartyType == eVideo_party_type_none) ? TRUE : FALSE;

									float newNeededPromilles = CMpmRxHelper::artCapacityToPromilles(pParam->artCapacity, isSvcOnly, isAudioOnly);

									msg << "\n  OldNeededPromilles         :" << oldNeededPromilles;
									msg << "\n  NewNeededPromilles         :" << newNeededPromilles;

									//1. Change the port promilles
									((CActivePort*)(&(*port)))->SetPromilUtilized(newNeededPromilles);

									//2. Change the total free capacity for the unit
									float newTotalFreeCapacity = mediaUnit->GetFreeCapacity() + oldNeededPromilles - newNeededPromilles;

									msg << "\n  OldTotalFreeCapacity       :" << mediaUnit->GetFreeCapacity();
									msg << "\n  NewTotalFreeCapacity       :" << newTotalFreeCapacity;
									((CUnitMFA*)(&(*mediaUnit)))->SetFreeCapacity(newTotalFreeCapacity);
								}

								TRACEINTO << msg.str().c_str();
								return STATUS_OK;
							}
						}
					}
				}
			}
		}
	}
	TRACEINTOLVLERR << "Failed, party not found" << msg.str().c_str();
	return STATUS_PARTY_NOT_EXISTS;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::AllocateVideo(DWORD rsrcConfId, WORD rsrcPartyId, AllocData& videoAlloc, PartyDataStruct& partyData)
{
	if (rsrcConfId == 0)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CBoard* pBoard = GetBoard(videoAlloc.m_boardId);
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, STATUS_INSUFFICIENT_VIDEO_RSRC);

	CUnitMFA *pMFA, *pRollBackMFA;
	BOOL bFailed = FALSE;
	int i;
	int ret = 0;

	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	for (i = 0; i < max_units_video; i++)
	{
		if (videoAlloc.m_unitsList[i].GetTotalNeededCapacity() > 0)
		{
			pMFA = (CUnitMFA*)pBoard->GetMFA(videoAlloc.m_unitsList[i].m_UnitId);
			if (pMFA == NULL)
			{
				TRACEINTO << "videoPartyType:" << partyData.m_videoPartyType << ", unit_id:" << videoAlloc.m_unitsList[i].m_UnitId << ", unit_index:" << i << " - Failed, unit not found";
				bFailed = TRUE;
				break;
			}

			ret = pMFA->AllocatePorts(rsrcConfId, rsrcPartyId, 0, videoAlloc.m_unitsList[i], partyData.m_videoPartyType);
			if (ret == -1) //faiure
			{
				TRACEINTO << "videoPartyType:" << partyData.m_videoPartyType << ", unit_id:" << videoAlloc.m_unitsList[i].m_UnitId << ", unit_index:" << i << " - Failed to allocate ports";
				bFailed = TRUE;
				break;
			}
		}
	}

	if (bFailed == TRUE)
	{
		//rollback
		WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
		for (int j = 0; j < i; j++)
		{
			if (videoAlloc.m_unitsList[j].GetTotalNeededCapacity() > 0)
			{
				if (!(pRollBackMFA = (CUnitMFA*)pBoard->GetMFA(videoAlloc.m_unitsList[j].m_UnitId)))
					continue;
				for (int portIndex = 0; portIndex < max_media_ports; portIndex++)
				{
					//if(videoAlloc.m_unitsList[j].m_MediaPortsList[portIndex].m_portId > 0)
					if (videoAlloc.m_unitsList[j].m_MediaPortsList[portIndex].m_type != eLogical_res_none)
						ret = pRollBackMFA->DeAllocatePort(videoAlloc.m_unitsList[j].m_MediaPortsList[portIndex].m_portId, partyData.m_videoPartyType);
				}
			}
		}
		return STATUS_INSUFFICIENT_VIDEO_RSRC;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::AllocateVideo2C(DWORD rsrcConfId, AllocData& videoAlloc)
{
	if (rsrcConfId == 0)
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	CBoard* pBoard = GetBoard(videoAlloc.m_boardId);
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, STATUS_INSUFFICIENT_VIDEO_RSRC);

	CUnitMFA *pMFA, *pRollBackMFA;
	BOOL bFailed = FALSE;
	int i;
	int ret = 0;

	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	for (i = 0; i < max_units_video; i++)
	{
		if (videoAlloc.m_unitsList[i].GetTotalNeededCapacity() > 0)
		{
			pMFA = (CUnitMFA*)pBoard->GetMFA(videoAlloc.m_unitsList[i].m_UnitId);
			if (pMFA == NULL)
			{
				PTRACE(eLevelInfoNormal, "CSystemResources::AllocateVideo2C - unit not found");
				bFailed = TRUE;
				break;
			}

			ret = pMFA->AllocateVideoPortsCOP(rsrcConfId, videoAlloc.m_unitsList[i]);
			if (ret == -1) //faiure
			{
				PTRACE(eLevelInfoNormal, "CSystemResources::AllocateVideo2C - failed to allocate ports");
				bFailed = TRUE;
				break;
			}
		}
	}

	if (bFailed == TRUE)
	{
		WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
		//rollback
		for (int j = 0; j < i; j++)
		{
			if (videoAlloc.m_unitsList[j].GetTotalNeededCapacity() > 0)
			{
				if (!(pRollBackMFA = (CUnitMFA*)pBoard->GetMFA(videoAlloc.m_unitsList[j].m_UnitId)))
					continue;
				for (int portIndex = 0; portIndex < max_media_ports; portIndex++)
				{
					if (videoAlloc.m_unitsList[j].m_MediaPortsList[portIndex].m_portId > 0)
						ret = pRollBackMFA->DeAllocatePort2C(videoAlloc.m_unitsList[j].m_MediaPortsList[portIndex].m_portId);
				}
			}
		}
		return STATUS_INSUFFICIENT_VIDEO_RSRC;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
float CSystemResources::GetFreeVideoCapacity(WORD boardId, float* freePortCapacity)
{
	float free_capacity = 0, free_ports_capacity = 0;
	std::set<CUnitMFA>::iterator i;

	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, 0);
	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();

	for (i = pMediaUnitslist->begin(); i != pMediaUnitslist->end(); i++)
	{
		if (i->GetUnitType() != eUnitType_Video || i->GetIsEnabled() != TRUE)
			continue;

		if (boardId != i->GetBoardId())
			continue;

		WORD acceleratorsPerUnit = CHelperFuncs::IsMpmRxOrNinja(pBoard->GetCardType()) ? ACCELERATORS_PER_UNIT_NETRA : 1;
		for (WORD currAcceleratorId = 0; currAcceleratorId < acceleratorsPerUnit; currAcceleratorId++)
		{
			// Tsahi - make sure that it's ok to return free capacity of 3000
			free_capacity += i->GetFreeCapacity(currAcceleratorId);

			if (freePortCapacity)
			{
				free_ports_capacity = i->GetFreeVideoPortsCapacity(currAcceleratorId);
				*freePortCapacity += free_ports_capacity;
			}
		}
	}
	return free_capacity;
}

////////////////////////////////////////////////////////////////////////////
WORD CSystemResources::GetNumVideoParties(WORD boardId, DWORD monitor_conf_id)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN_VALUE(pConfRsrcDB == NULL, 0);

	CConfRsrc* pConf = (CConfRsrc*)(pConfRsrcDB->GetConfRsrc(monitor_conf_id));
	PASSERT_AND_RETURN_VALUE(pConf == NULL, 0);

	WORD numVideoParties = pConf->GetNumVideoPartiesPerBoard(boardId);
	return numVideoParties;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeAllocateVideo(CConfRsrc* pConf, WORD rsrcPartyId, AllocData& videoAlloc, eVideoPartyType videoPartyType, BYTE dsbl)
{
	WORD rsrcConfId = pConf->GetRsrcConfId();
	if (rsrcConfId == 0) //not allocated, some internal error
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	WORD bId = videoAlloc.m_boardId;
	CBoard* pBoard = GetBoard(bId);
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, STATUS_FAIL);

	int ret = 0, tempRet = 0;
	WORD uId = 0, portId = 0, acceleratorId = 0;
	CUnitMFA* pUnitMFA = NULL;
	WORD max_units_video = CHelperFuncs::GetMaxUnitsNeededForVideo();
	WORD max_media_ports = CHelperFuncs::GetMaxRequiredPortsPerMediaUnit();
	for (int i = 0; i < max_units_video; i++)
	{
		if (videoAlloc.m_unitsList[i].GetTotalNeededCapacity() > 0)
		{
			uId = videoAlloc.m_unitsList[i].m_UnitId;
			pUnitMFA = GetUnit(bId, uId);
			if (pUnitMFA)
			{
				for (int j = 0; j < max_media_ports; j++)
				{
					if (videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_type != eLogical_res_none)
					{
						portId = videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_portId;
						acceleratorId = videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_acceleratorId;
						if (eCOP_party_type == videoPartyType)
							tempRet = pUnitMFA->DeAllocatePortCOP(portId, videoAlloc.m_unitsList[i].m_MediaPortsList[j].m_needed_capacity_promilles, dsbl);
						else
							tempRet = pUnitMFA->DeAllocatePort(portId, videoPartyType, dsbl, acceleratorId);

						if (tempRet != 0)
							ret = tempRet;
					}
				}
			}
			else
			{
				PASSERT(1);
			}
		}
	}

	if (ret == 0)
	{
		return STATUS_OK;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::DeAllocateVideo failed ret = ", ret);
		return STATUS_VIDEO_DEALLOCATION_FAIL;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeAllocateVideo2C(CConfRsrc* pConf, CRsrcDesc* pDesc, BYTE dsbl)
{
	WORD rsrcConfId = pConf->GetRsrcConfId();
	if (rsrcConfId == 0 || !pDesc) //not allocated, some internal error
	{
		PASSERT(1);
		return STATUS_FAIL;
	}
	WORD bId = pDesc->GetBoardId();
	CBoard* pBoard = GetBoard(bId);
	if (pBoard == NULL)
	{
		TRACEINTOLVLERR << "Failed" << *pDesc;

		if (bId != 0)
		{
			PASSERT(bId);
		}
		else
		{
			PASSERT(20002);
		}
		return STATUS_FAIL;
	}

	int ret = 0, tempRet;
	WORD portId;

	WORD uId = pDesc->GetUnitId();
	CUnitMFA* pUnitMFA = GetUnit(bId, uId);

	if (NULL != pUnitMFA)
	{
		if (pDesc->GetType() != eLogical_res_none)
		{
			portId = pDesc->GetFirstPortId();
			tempRet = pUnitMFA->DeAllocatePort2C(portId, dsbl);
			if (tempRet != 0)
				ret = tempRet;
		}
	}
	else
		PASSERT(1);

	if (ret == 0)
		return STATUS_OK;
	else
	{
		PTRACE2INT(eLevelInfoNormal, "CSystemResources::DeAllocateVideo2C for one descriptor failed, ret = ", ret);
		return STATUS_VIDEO_DEALLOCATION_FAIL;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::UpdateActiveRtmPort(WORD bId, WORD uId, WORD portId, DWORD rsrcConfId, DWORD rsrcPartyId)
{
	STATUS status = STATUS_OK;

	CBoard* pBoard = GetBoard(bId);
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, STATUS_FAIL);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	CSpanRTM* pExistSpanRTM = new CSpanRTM(bId, uId);

	std::set<CSpanRTM>::iterator i;
	i = pSpanslist->find(*pExistSpanRTM);

	if (i != pSpanslist->end())
	{
		const CSpanRTM* pSpan = (&(*i));
		status = ((CSpanRTM*)(pSpan))->UpdateActivePort(portId, rsrcConfId, rsrcPartyId);
	}

	else
		//either internal error
		status = STATUS_FAIL;

	POBJDELETE(pExistSpanRTM);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::UpdateActivePort(WORD bId, WORD uId, WORD portId, DWORD rsrcConfId, DWORD rsrcPartyId)
{
	STATUS status = STATUS_OK;

	CUnitMFA* pUnitMFA = GetUnit(bId, uId);
	PASSERT_AND_RETURN_VALUE(pUnitMFA==NULL, STATUS_FAIL);

	status = pUnitMFA->UpdateActivePort(portId, rsrcConfId, rsrcPartyId);

	return status;
}

////////////////////////////////////////////////////////////////////////////
bool CSystemResources::IsBoardIdExists(BoardID boardId)
{
	CBoard* pBoard = GetBoard(boardId);
	PASSERT_AND_RETURN_VALUE(!pBoard, FALSE);

	CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
	return (pMediaUnitslist->size() != 0) ? true : false;
}

////////////////////////////////////////////////////////////////////////////
//RTM allocation - Capture (Dial-in, fail if race overflow), Allocate (Dial-out, least loaded RTM)
//We get to this funtion after going through CSystemResources::GetBestBoards
//so we already decided on which unit to allocate
////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::AllocateRTMOneChannel(DWORD rsrcConfId, WORD rsrcPartyId, BOOL bIsDialOut, WORD reqBoardId, WORD reqUnitId, PhysicalPortDesc* pPortDesc)
{
	std::set<CSpanRTM>::iterator i;
	STATUS status = STATUS_INSUFFICIENT_RTM_RSRC;
	WORD portId = 0;

	if (DUMMY_BOARD_ID == reqBoardId || DUMMY_SPAN_ID == reqUnitId)
	{
		PASSERT(1); //we shouldn't get to here with this, because this means that GetBestBoards failed...
		PTRACE(eLevelInfoNormal, "CSystemResources::AllocateRTMOneChannel - no Span RTM found");
		return STATUS_INSUFFICIENT_RTM_RSRC;
	}

	CBoard* pBoard = GetBoard(reqBoardId);
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, STATUS_INSUFFICIENT_RTM_RSRC);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	for (i = pSpanslist->begin(); i != pSpanslist->end(); i++)
	{
		if (i->GetIsEnabled() != TRUE)
			continue;
		if (i->GetUnitId() == reqUnitId)
		{
			const CSpanRTM* pSpan = (&(*i));
			bool bSetAsReserved;
			if (bIsDialOut)
				bSetAsReserved = TRUE;
			else
				bSetAsReserved = FALSE;

			status = ((CSpanRTM*)(pSpan))->AllocatePort(rsrcConfId, rsrcPartyId, ePhysical_rtm, portId, bSetAsReserved);

			if (STATUS_OK == status)
			{
				const char* serv_name = ((CSpanRTM*)(&(*i)))->GetSpanServiceName();
				if (serv_name)
				{
					CNetServiceRsrcs* pService = (CNetServiceRsrcs*)(findServiceByName(serv_name));
					if (pService == NULL) //not found, internal error
					{
						return STATUS_FAIL;
					}

					int k = 0;
					for (k = 0; k < MAX_NUM_OF_BOARDS; k++)
					{
						if (pService->m_ipAddressesList[k].boardId == reqBoardId)
						{
							pPortDesc->m_IpAddrV4 = pService->m_ipAddressesList[k].ipAdressRTMV4;
							break;
						}
					}
					if (MAX_NUM_OF_BOARDS == k)
					{
						PTRACE(eLevelError, "CSystemResources::AllocateRTMOneChannel Can not find board ID");
						PASSERT(1);
					}
					else
						break;
				}
			}
		}
	}

	if (STATUS_OK != status) //STATUS NOT OK
	{
		PTRACE(eLevelInfoNormal, "CSystemResources::AllocateRTMOneChannel - no Span RTM found 2");
		return STATUS_INSUFFICIENT_RTM_RSRC;
	}
	else
	{
		pPortDesc->m_portId = portId;
		pPortDesc->m_acceleratorId = 0; /*Tsahi TBD: change accelerator id? */
		pPortDesc->m_boxId = 1;
		pPortDesc->m_subBoardId = 2;
		pPortDesc->m_boardId = reqBoardId;
		pPortDesc->m_unitId = reqUnitId;

		return status;
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CSystemResources::DeAllocateRTMOneChannel(CConfRsrc* pConf, WORD rsrcPartyId, PhysicalPortDesc* pPortDesc, BOOL bIsUpdated)
{
	WORD rsrcConfId = pConf->GetRsrcConfId();
	if (rsrcConfId == 0) //not allocaed, some internal error
		return STATUS_FAIL; //+ PASSERT

	STATUS status = STATUS_FAIL;
	std::set<CSpanRTM>::iterator i;

	WORD bId = pPortDesc->m_boardId;
	WORD uId = pPortDesc->m_unitId;
	WORD portId = pPortDesc->m_portId;

	CBoard* pBoard = GetBoard(bId);
	PASSERT_AND_RETURN_VALUE(pBoard==NULL, STATUS_FAIL);
	CSpansList* pSpanslist = pBoard->GetSpansRTMlist();

	for (i = pSpanslist->begin(); i != pSpanslist->end(); i++)
	{
		if (i->GetUnitId() == uId)
		{
			const CSpanRTM* pSpan = (&(*i));
			status = ((CSpanRTM*)(pSpan))->DeAllocatePort(portId, bIsUpdated);
			break;
			//   status  =  ((CSpanRTM)(*i)).DeAllocatePort(portId);
		}
	}
	if (i == pSpanslist->end())
	{
		status = STATUS_RTM_DEALLOCATION_FAIL;
		PASSERT(1);
	}
	//POBJDELETE(pDesc); //??? return for deleting corresponding data from conn-to-card table?
	return status;
}
#undef TRACEINTO
#define TRACEINTO TRACESTRFUNC(eLevelInfoNormal)
