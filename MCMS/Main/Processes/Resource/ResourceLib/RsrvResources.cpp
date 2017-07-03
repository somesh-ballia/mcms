// RsrvResources.cpp:
//
//////////////////////////////////////////////////////////////////////

#include <string>
#include "TraceStream.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "SystemResources.h"
#include "RsrvResources.h"
#include "ResourceProcess.h"
#include "Reservator.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "HelperFuncs.h"
#include "CommResApi.h"
#include "ProfilesDB.h"
#include "ResRsrcCalculator.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////
//                        CConfRsrvRsrc
////////////////////////////////////////////////////////////////////////////
CConfRsrvRsrc::CConfRsrvRsrc(CCommResApi* pCommResApi, BOOL isFromPassive)
{
	SetNID(pCommResApi->GetNumericConfId());

	m_monitorConfId            = pCommResApi->GetMonitorConfId();
	m_repeatedConfId           = pCommResApi->GetRepSchedulingId();
	m_startInterval            = 0;
	m_endInterval              = 0;
	m_ResStatus                = STATUS_OK;
	m_isFromPassive            = isFromPassive;
	m_minNumAudioPartiesInConf = 0;
	m_minNumVideoPartiesInConf = 0;
	m_maxVideoPartyTypeInConf  = eVideo_party_type_none;
	m_isPermanent              = pCommResApi->IsPermanent();
	m_ipServiceIdForMinParties = ID_ALL_IP_SERVICES;
	m_isVSW_AUTO               = pCommResApi->GetIsHDVSW();
	m_ind_service_phone        = 0;
	m_numServicePhoneStr       = 0;
	memset(m_pServicePhoneStr, 0, sizeof(m_pServicePhoneStr));
}

//--------------------------------------------------------------------------
CConfRsrvRsrc::CConfRsrvRsrc(const CConfRsrvRsrc& other) : CPObject(other),
	m_monitorConfId(other.m_monitorConfId),
	m_repeatedConfId(other.m_repeatedConfId),
	m_minNumAudioPartiesInConf(other.m_minNumAudioPartiesInConf),
	m_minNumVideoPartiesInConf(other.m_minNumVideoPartiesInConf),
	m_maxVideoPartyTypeInConf(other.m_maxVideoPartyTypeInConf),
	m_startInterval(other.m_startInterval),
	m_endInterval(other.m_endInterval),
	m_isFromPassive(other.m_isFromPassive),
	m_isPermanent(other.m_isPermanent),
	m_isVSW_AUTO(other.m_isVSW_AUTO),
	m_ipServiceIdForMinParties(other.m_ipServiceIdForMinParties)
{
	SetNID(other.m_NID);
	m_PartiesResources   = other.m_PartiesResources;
	m_ResStatus          = other.m_ResStatus;
	m_numServicePhoneStr = other.m_numServicePhoneStr;
	m_ind_service_phone  = other.m_ind_service_phone;

	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
	{
		if (other.m_pServicePhoneStr[i] == NULL)
			m_pServicePhoneStr[i] = NULL;
		else
			m_pServicePhoneStr[i] = new CServicePhoneStr(*other.m_pServicePhoneStr[i]);
	}
}

//--------------------------------------------------------------------------
CConfRsrvRsrc::~CConfRsrvRsrc()
{
	for (int i = 0; i < MAX_NET_SERV_PROVIDERS_IN_LIST; i++)
		POBJDELETE(m_pServicePhoneStr[i]);
}

//--------------------------------------------------------------------------
WORD operator==(const CConfRsrvRsrc& lhs, const CConfRsrvRsrc& rhs)
{
	return (lhs.m_monitorConfId == rhs.m_monitorConfId);
}

//--------------------------------------------------------------------------
bool operator<(const CConfRsrvRsrc& lhs, const CConfRsrvRsrc& rhs)
{
	return (lhs.m_monitorConfId < rhs.m_monitorConfId);
}

//--------------------------------------------------------------------------
void CConfRsrvRsrc::GetConferenceNeededAmount(CIntervalRsrvAmount& confNeededAmount, BOOL bCountPartyResourcesOnlyIfStatusOK)
{
	confNeededAmount.m_bIsRealConference = TRUE;   // all reservations are real conferences, and not sleeping meeting rooms
	confNeededAmount.SetNID(m_NID);
	confNeededAmount.m_VSW_AUTO = m_isVSW_AUTO;
	if (bCountPartyResourcesOnlyIfStatusOK == FALSE || m_ResStatus == STATUS_OK)
	{
		confNeededAmount.m_PartiesResources = m_PartiesResources;
		CServicePhoneStr* pServicePhoneStr = GetFirstServicePhone(); // olga
		if (pServicePhoneStr && pServicePhoneStr->m_netServiceName[0] != '\0')
			confNeededAmount.AddServicePhone(*pServicePhoneStr);
	}
	else
		confNeededAmount.m_PartiesResources.Init();
}

//--------------------------------------------------------------------------
void CConfRsrvRsrc::SetNID(const char* pNID)
{
	strcpy_safe(m_NID, pNID);
}


////////////////////////////////////////////////////////////////////////////
//                        CInterval
////////////////////////////////////////////////////////////////////////////
CInterval::CInterval(DWORD interval, CPartiesResources& totalPartiesResources, WORD total_conferences, WORD total_auto_vsw)
{
	m_interval              = interval;
	m_totalPartiesResources = totalPartiesResources;
	m_total_conferences     = total_conferences;
	m_total_auto_vsw_conf   = total_auto_vsw;
}

//--------------------------------------------------------------------------
CInterval::~CInterval()
{
}

//--------------------------------------------------------------------------
bool operator==(const CInterval& lhs, const CInterval& rhs)
{
	return (lhs.m_interval == rhs.m_interval);
}

//--------------------------------------------------------------------------
bool operator<(const CInterval& lhs, const CInterval& rhs)
{
	return (lhs.m_interval < rhs.m_interval);
}

//--------------------------------------------------------------------------
STATUS CInterval::Reserve(CIntervalRsrvAmount& amount)
{
	float copyLogicalNumparties[NUM_OF_PARTY_RESOURCE_TYPES];
	memcpy(copyLogicalNumparties, m_totalPartiesResources.m_logical_num_parties, sizeof(copyLogicalNumparties));

	if (amount.m_PartiesResources.HasParties())
	{
		float neededparties;
		if (!CHelperFuncs::IsMode2C()) // carmit-fix2
		{
			CMedString sstr;
			for(int i=0; i<NUM_OF_PARTY_RESOURCE_TYPES; i++)
			{
				neededparties = amount.m_PartiesResources.m_logical_num_parties[i];

				sstr << "\ntype:" << i << ", needed-parties:" << neededparties << ", num-parties:" << copyLogicalNumparties[i];

				if (copyLogicalNumparties[i] >= neededparties)
				{
					copyLogicalNumparties[i] -= neededparties;
					continue;
				}

				if (i == 0) // audio only can't be upgraded
				{
					TRACEINTO << "CInterval::Reserve - Failed, status:STATUS_INSUFFICIENT_RSRC" << sstr.GetString();
					return STATUS_INSUFFICIENT_RSRC;
				}

				// for video parties try upgrade
				for(int j=i; j<NUM_OF_PARTY_RESOURCE_TYPES; j++)
				{
					// try taking all needed parties on current level
					if (copyLogicalNumparties[j] >= neededparties)
					{
						copyLogicalNumparties[j] -= neededparties;
						neededparties             = 0;
						break;
					}

					// take all copyLogicalNumparties[j] and continue
					neededparties           -= copyLogicalNumparties[j];
					copyLogicalNumparties[j] = 0;
				}

				if (neededparties > 0)  // if not all could be reserved
				{
					sstr << "\nm_interval:" << m_interval;
					TRACEINTO << "CInterval::Reserve - Failed, status:STATUS_INSUFFICIENT_RSRC" << sstr.GetString();
					return STATUS_INSUFFICIENT_RSRC;
				}
			}
		}
		else
		{
			neededparties = amount.m_PartiesResources.m_logical_COP_num_parties;
			if (m_totalPartiesResources.m_logical_COP_num_parties >= neededparties)
			{
				m_totalPartiesResources.m_logical_COP_num_parties -= neededparties;
				neededparties = 0;
			}
			else
			{
				neededparties -= m_totalPartiesResources.m_logical_COP_num_parties;
				m_totalPartiesResources.m_logical_COP_num_parties = 0;
				return STATUS_INSUFFICIENT_RSRC;
			}
		}
	}

	if (amount.m_bIsRealConference == TRUE)
	{
		if (!amount.m_VSW_AUTO)
		{
			if (m_total_conferences <= 0)
			{
				TRACEINTO << "CInterval::Reserve - Failed, status:STATUS_MAX_RESERVATIONS_EXCEEDED, m_total_conferences:" << m_total_conferences;
				return STATUS_MAX_RESERVATIONS_EXCEEDED;
			}
		}
		else
		{
			if (m_total_auto_vsw_conf <= 0)
			{
				TRACEINTO << "CInterval::Reserve - Failed, status:STATUS_MAX_RESERVATIONS_EXCEEDED, m_total_auto_vsw_conf:" << m_total_auto_vsw_conf;
				return STATUS_MAX_RESERVATIONS_EXCEEDED;
			}
		}
	}

	if (amount.m_PartiesResources.HasParties())
	{
		if (!CHelperFuncs::IsMode2C())
			memcpy(m_totalPartiesResources.m_logical_num_parties, copyLogicalNumparties, sizeof(m_totalPartiesResources.m_logical_num_parties));
	}

	if (amount.m_bIsRealConference == TRUE)
	{
		if (!amount.m_VSW_AUTO)
			m_total_conferences--;
		else
			m_total_auto_vsw_conf--;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CInterval::UnReserve(CIntervalRsrvAmount& amount)
{
	for(int i=0; i<NUM_OF_PARTY_RESOURCE_TYPES; i++)
		m_totalPartiesResources.m_logical_num_parties[i] += amount.m_PartiesResources.m_logical_num_parties[i];

	if (amount.m_bIsRealConference == TRUE)
		m_total_conferences++;

	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////
//                        CIntervalRsrvAmount
////////////////////////////////////////////////////////////////////////////
CIntervalRsrvAmount::CIntervalRsrvAmount()
{
	m_pServicePhoneStr = NULL;
	Init();
}

//--------------------------------------------------------------------------
void CIntervalRsrvAmount::Init()
{
	m_bIsRealConference = TRUE;
	m_VSW_AUTO          = FALSE;
	memset(&m_NID, 0, NUMERIC_CONF_ID_MAX_LEN);
	m_PartiesResources.Init();

	POBJDELETE(m_pServicePhoneStr);
}

//--------------------------------------------------------------------------
CIntervalRsrvAmount::~CIntervalRsrvAmount()
{
	POBJDELETE(m_pServicePhoneStr);
}

//--------------------------------------------------------------------------
void CIntervalRsrvAmount::SetNID(const char* pNID)
{
	strcpy_safe(m_NID, pNID);
}

//--------------------------------------------------------------------------
STATUS CIntervalRsrvAmount::AddServicePhone(const CServicePhoneStr& other)
{
	m_pServicePhoneStr = new CServicePhoneStr(other);
	return STATUS_OK;
}

//--------------------------------------------------------------------------
CServicePhoneStr* CIntervalRsrvAmount::GetServicePhone() const
{
	return m_pServicePhoneStr;
}

//--------------------------------------------------------------------------
STATUS CConfRsrvRsrc::FillFieldsNeededForReservator(CCommResApi* pCommResApi)
{
	if (!pCommResApi)
		return STATUS_FAIL;

	STATUS       status      = STATUS_OK;
	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN_VALUE(!pReservator, STATUS_FAIL);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	// getting genesis and initiating start/end intervals
	const CStructTm* start = pCommResApi->GetStartTime();
	m_startInterval = pReservator->StructTm2IntervalForStart(start);    // check offset - like in MGC???

	const CStructTm* end = pCommResApi->GetEndTime();
	m_endInterval = pReservator->StructTm2IntervalForEnd(end);          // check offset - like in MGC???

	CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	PASSERT_AND_RETURN_VALUE(!pProcess, STATUS_FAIL);

	CSysConfig* pSysConfig = pProcess->GetSysConfig();
	PASSERT_AND_RETURN_VALUE(!pSysConfig, STATUS_FAIL);

	BOOL is_alloc_perm_rsrc;
	pSysConfig->GetBOOLDataByKey(CFG_KEY_PERMANENT_CP_CONF_RESOURCE_ALLOC, is_alloc_perm_rsrc);

	m_minNumAudioPartiesInConf = pCommResApi->GetNumOfMinAudioParties();
	m_minNumVideoPartiesInConf = pCommResApi->GetNumOfMinVideoParties();
	m_ipServiceIdForMinParties = pReservator->GetIpServiceIdForMinParties(pCommResApi);

	TRACEINTO
		<< "\n  ConfName                 :" << pCommResApi->GetName()
		<< "\n  MinNumAudioPartiesInConf :" << m_minNumAudioPartiesInConf
		<< "\n  MinNumVideoPartiesInConf :" << m_minNumVideoPartiesInConf
		<< "\n  IpServiceIdForMinParties :" << m_ipServiceIdForMinParties;

	if (pCommResApi->IsPermanent() && !CHelperFuncs::IsMode2C() && !is_alloc_perm_rsrc)
	{
		TRACEINTO << "This is a permanent CP conference. Don't allow resources reservation, so set minNumVideoPartiesInConf to 0";
		m_minNumVideoPartiesInConf = 0;
	}

	if (pCommResApi->GetAdHocProfileId() != 0 && pCommResApi->GetAdHocProfileId() != 0xFFFFFFFF)
	{
		CProfilesDB* pProfilesDB = CHelperFuncs::GetProfilesDB();
		if (pProfilesDB != NULL)
			m_maxVideoPartyTypeInConf = pProfilesDB->GetMaxVideoPartyTypeByProfileId(pCommResApi->GetAdHocProfileId());
		else
			PASSERT(1);
	}
	else
	{
		CResRsrcCalculator rsrcCalc;
		eSystemCardsMode   systemCardsBasedMode = pSystemResources->GetSystemCardsMode();
		m_maxVideoPartyTypeInConf = rsrcCalc.GetRsrcVideoType(systemCardsBasedMode, pCommResApi);
		TRACEINTO << "No profile, so use the system defined maxVideoPartyTypeInConf:" << eVideoPartyTypeNames[m_maxVideoPartyTypeInConf];
	}

	status = CalculateReservationPartyResources();

	CServicePhoneStr* pPhoneStr = pCommResApi->GetFirstServicePhone();
	while (pPhoneStr != NULL && pPhoneStr->m_netServiceName[0] != '\0')
	{
		AddServicePhone(*pPhoneStr);
		pPhoneStr = pCommResApi->GetNextServicePhone();
	}

	return status;
}

//--------------------------------------------------------------------------
STATUS CConfRsrvRsrc::CalculateReservationPartyResources()
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();
	PASSERT_AND_RETURN_VALUE(!pResourcesInterface, STATUS_FAIL);

	m_PartiesResources.Init();
	pResourcesInterface->FillRequiredPartyResourcesForConference(m_minNumAudioPartiesInConf, m_minNumVideoPartiesInConf, m_maxVideoPartyTypeInConf, m_PartiesResources);
	m_PartiesResources.CalculateHasParties();
	m_PartiesResources.DumpToTrace("CConfRsrvRsrc::CalculateReservationPartyResources");

	return STATUS_OK;
}

//--------------------------------------------------------------------------
STATUS CConfRsrvRsrc::AddServicePhone(const CServicePhoneStr& other)
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
STATUS CConfRsrvRsrc::DeleteServicePhone(const CServicePhoneStr& other)
{
	int i = 0;
	STATUS status = STATUS_OK;

	if ((i = FindServicePhone(other)) == NOT_FIND)
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;

	if(i > MAX_NET_SERV_PROVIDERS_IN_LIST -1)
		return STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED;	

	if (m_pServicePhoneStr[i])
		POBJDELETE(m_pServicePhoneStr[i]);

	m_numServicePhoneStr--;

	if (m_numServicePhoneStr > MAX_NET_SERV_PROVIDERS_IN_LIST)
		status = STATUS_FAIL;

	// Need to close the gap that might have been created
	for (int j = i; j < MAX_NET_SERV_PROVIDERS_IN_LIST - 1; j++)
		m_pServicePhoneStr[j] = m_pServicePhoneStr[j+1];

	return status;
}

//--------------------------------------------------------------------------
void CConfRsrvRsrc::AddServicePhoneNumber(const char* netServiceName, const char* phoneNumber)
{

	PASSERT_AND_RETURN(m_numServicePhoneStr > MAX_NET_SERV_PROVIDERS_IN_LIST);
	
	if (NULL == netServiceName || NULL == phoneNumber)
		return;

	for (int i = 0; i < (int)m_numServicePhoneStr; i++)
	{
		if (m_pServicePhoneStr[i] != NULL && !strcmp(m_pServicePhoneStr[i]->GetNetServiceName(), netServiceName))
		{
			m_pServicePhoneStr[i]->AddPhoneNumber(phoneNumber);
			break;
		}
	}
}

//--------------------------------------------------------------------------
int CConfRsrvRsrc::FindServicePhone(const CServicePhoneStr& other)
{
	PASSERT_AND_RETURN_VALUE(m_numServicePhoneStr > MAX_NET_SERV_PROVIDERS_IN_LIST, STATUS_NUMBER_OF_SERVICE_PROVIDERS_EXCEEDED); 
	
	for (int i = 0; i < (int)m_numServicePhoneStr; i++)
	{
		// find a service that "larger(have the same and all phones us "other"
		// but can be more phones ) or equal to the given
		if (m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) >= other)
			return i;

		if (m_pServicePhoneStr[i] != NULL && (*(m_pServicePhoneStr[i])) < other)
		{
			*m_pServicePhoneStr[i] = other;
			return i;
		}
	}

	return NOT_FIND;
}

//--------------------------------------------------------------------------
CServicePhoneStr* CConfRsrvRsrc::GetFirstServicePhone()
{
	m_ind_service_phone = 0;
	return m_pServicePhoneStr[0];
}

//--------------------------------------------------------------------------
CServicePhoneStr* CConfRsrvRsrc::GetNextServicePhone()
{
	PASSERT_AND_RETURN_VALUE(m_numServicePhoneStr > MAX_NET_SERV_PROVIDERS_IN_LIST, NULL); 
	m_ind_service_phone++;
	if (m_ind_service_phone >= m_numServicePhoneStr)
		return NULL;

	return m_pServicePhoneStr[m_ind_service_phone];
}

//--------------------------------------------------------------------------
WORD CConfRsrvRsrc::GetLogicalWeightOfMinNumVideoParties() const
{
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		if (m_PartiesResources.m_logical_num_parties[i] != 0)
		{
			if (CHelperFuncs::IsFixedModeAllocationType())
				return m_PartiesResources.m_physical_hd720_ports;
			else
			{
				WORD min_parties = (WORD)m_PartiesResources.m_logical_num_parties[i];
				return min_parties;
			}
		}
	}

	return m_PartiesResources.m_physical_hd720_ports;
}

