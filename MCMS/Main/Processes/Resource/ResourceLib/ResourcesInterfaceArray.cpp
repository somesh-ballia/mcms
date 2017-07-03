#include "ResourcesInterfaceArray.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "FixedModeResources.h"
#include "AutoModeResources.h"
#include "HelperFuncs.h"
#include "CRsrcDetailGet.h"
#include "InternalProcessStatuses.h"
#include "ResRsrcCalculator.h"

////////////////////////////////////////////////////////////////////////////
//                        CResourcesInterfaceArray
////////////////////////////////////////////////////////////////////////////
CResourcesInterfaceArray::CResourcesInterfaceArray()
{
	for (int i = 0; i < MAX_NUMBER_OF_RESOURCES_TYPE; i++)
		m_pResourcesInterfaceArray[i] = NULL;

	m_CurrentResourcesInterfaceIndex = -1;
	m_ResourceAllocationType = eNoMode;
}

////////////////////////////////////////////////////////////////////////////
CResourcesInterfaceArray::~CResourcesInterfaceArray()
{
	m_CurrentResourcesInterfaceIndex = -1;
	for (int i = 0; i < MAX_NUMBER_OF_RESOURCES_TYPE; i++)
	{
		POBJDELETE(m_pResourcesInterfaceArray[i]);
		m_pResourcesInterfaceArray[i] = NULL;
	}
}

////////////////////////////////////////////////////////////////////////////
CResourcesInterfaceArray::CResourcesInterfaceArray(const CResourcesInterfaceArray& other) :
		CPObject(other), ResourcesInterface(other)
{
	m_CurrentResourcesInterfaceIndex = other.m_CurrentResourcesInterfaceIndex;
	m_ResourceAllocationType = other.m_ResourceAllocationType;

	for (int i = 0; i < MAX_NUMBER_OF_RESOURCES_TYPE; i++)
	{
		if (other.m_pResourcesInterfaceArray[i] != NULL)
			m_pResourcesInterfaceArray[i] = other.m_pResourcesInterfaceArray[i]->NewCopy();

	}
}

////////////////////////////////////////////////////////////////////////////
const CResourcesInterfaceArray& CResourcesInterfaceArray::operator=(const CResourcesInterfaceArray& other)
{
	m_CurrentResourcesInterfaceIndex = other.m_CurrentResourcesInterfaceIndex;
	m_ResourceAllocationType = other.m_ResourceAllocationType;

	for (int i = 0; i < MAX_NUMBER_OF_RESOURCES_TYPE; i++)
	{
		POBJDELETE(m_pResourcesInterfaceArray[i]);
		if (other.m_pResourcesInterfaceArray[i] != NULL)
			m_pResourcesInterfaceArray[i] = other.m_pResourcesInterfaceArray[i]->NewCopy();
	}
	return *this;
}

////////////////////////////////////////////////////////////////////////////
ResourcesInterface* CResourcesInterfaceArray::GetCurrentResourcesInterface() const
{
	PASSERT_AND_RETURN_VALUE(m_CurrentResourcesInterfaceIndex == -1, NULL);
	PASSERT_AND_RETURN_VALUE(m_CurrentResourcesInterfaceIndex >= MAX_NUMBER_OF_RESOURCES_TYPE, NULL);
	PASSERT_AND_RETURN_VALUE(m_pResourcesInterfaceArray[m_CurrentResourcesInterfaceIndex] == NULL, NULL);

	return m_pResourcesInterfaceArray[m_CurrentResourcesInterfaceIndex];
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::InitResourceAllocationMode(eResourceAllocationTypes resourceAllocationType)
{
	std::ostringstream msg;
	msg << "ResourceAllocationType:" << resourceAllocationType << ", ";
	m_ResourceAllocationType = resourceAllocationType;
	switch (resourceAllocationType)
	{
		case eFixedBreezeMode:
			m_pResourcesInterfaceArray[0] = new CFixedModeResources(eSystemCardsMode_breeze);
			m_pResourcesInterfaceArray[1] = new CAutoModeResources(eSystemCardsMode_breeze);
			m_CurrentResourcesInterfaceIndex = 0;
			msg << "ResourcesInterfaceIndex:0";
			break;
		case eAutoBreezeMode:
			m_pResourcesInterfaceArray[0] = new CFixedModeResources(eSystemCardsMode_breeze);
			m_pResourcesInterfaceArray[1] = new CAutoModeResources(eSystemCardsMode_breeze);
			m_CurrentResourcesInterfaceIndex = 1;
			msg << "ResourcesInterfaceIndex:1";
			break;
		case eFixedMpmRxMode:
			m_pResourcesInterfaceArray[0] = new CFixedModeResources(eSystemCardsMode_mpmrx);
			m_pResourcesInterfaceArray[1] = new CAutoModeResources(eSystemCardsMode_mpmrx);
			m_CurrentResourcesInterfaceIndex = 0;
			msg << "ResourcesInterfaceIndex:0";
			break;
		case eAutoMpmRxMode:
			m_pResourcesInterfaceArray[0] = new CFixedModeResources(eSystemCardsMode_mpmrx);
			m_pResourcesInterfaceArray[1] = new CAutoModeResources(eSystemCardsMode_mpmrx);
			m_CurrentResourcesInterfaceIndex = 1;
			msg << "ResourcesInterfaceIndex:1";
			break;
		case eAutoMixedMode:
			m_pResourcesInterfaceArray[0] = new CFixedModeResources(eSystemCardsMode_mixed_mode);
			m_pResourcesInterfaceArray[1] = new CAutoModeResources(eSystemCardsMode_mixed_mode);
			m_CurrentResourcesInterfaceIndex = 1;
			msg << "ResourcesInterfaceIndex:1";
			break;
		default:
			msg << (DWORD)resourceAllocationType << " - Illegal";
			PASSERT(1);
			break;
	}
	TRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourcesInterfaceArray::ChangeResourceAllocationMode(eResourceAllocationTypes resourceAllocationType)
{
	TRACEINTO << "OldResourceAllocationType:" << m_ResourceAllocationType << ", NewResourceAllocationType:" << resourceAllocationType;

	if (m_ResourceAllocationType == resourceAllocationType)
	{
		//Unneccesary call to ChangeResourceAllocationMode, because old mode and new mode are equal
		return STATUS_OK;
	}

	switch (m_ResourceAllocationType)
	{
		case eNoMode:
			//this means we didn't use InitResourceAllocationMode
			PASSERTMSG(1, "InitResourceAllocationMode should be called before ChangeResourceAllocationMode");
			InitResourceAllocationMode(resourceAllocationType);
			return STATUS_OK;

		case eFixedBreezeMode:
			if (resourceAllocationType == eAutoBreezeMode)
			{
				m_CurrentResourcesInterfaceIndex = 1;
				m_ResourceAllocationType = resourceAllocationType;
				break; //continue after switch
			}
			PASSERTSTREAM_AND_RETURN_VALUE(1, "Cannot change", STATUS_FAIL);
			break;

		case eAutoBreezeMode:
			if (resourceAllocationType == eFixedBreezeMode)
			{
				m_CurrentResourcesInterfaceIndex = 0;
				m_ResourceAllocationType = resourceAllocationType;
				break; //continue after switch
			}
			PASSERTSTREAM_AND_RETURN_VALUE(1, "Cannot change", STATUS_FAIL);
			break;

		case eFixedMpmRxMode:
			if (resourceAllocationType == eAutoMpmRxMode)
			{
				m_CurrentResourcesInterfaceIndex = 1;
				m_ResourceAllocationType = resourceAllocationType;
				break; //continue after switch
			}
			PASSERTSTREAM_AND_RETURN_VALUE(1, "Cannot change", STATUS_FAIL);
			break;

		case eAutoMpmRxMode:
			if (resourceAllocationType == eFixedMpmRxMode)
			{
				m_CurrentResourcesInterfaceIndex = 0;
				m_ResourceAllocationType = resourceAllocationType;
				break; //continue after switch
			}
			PASSERTSTREAM_AND_RETURN_VALUE(1, "Cannot change", STATUS_FAIL);
			break;

		case eAutoMixedMode:
			PASSERTSTREAM_AND_RETURN_VALUE(1, "Cannot change to Fixed Mode when system is in Mixed Mode (MPM-Rx + MPMx", STATUS_FAIL);
			break;

		default:
			PASSERT(1);
			return STATUS_FAIL;
	}

	//if we got until here, it means that it is a valid change, set dongle restriction in current configuration, update configuration proportions, and update units
	InitDongleRestriction(m_dongleNumParties, TRUE);

	// Tsahi - this is temporary until we have Mixed Mode enabled. then remove this and change also ReconfigureUnitsAccordingToProportion.
	if (resourceAllocationType == eAutoMpmRxMode || resourceAllocationType == eFixedMpmRxMode)
		return STATUS_OK;

	STATUS status = ReconfigureUnitsAccordingToProportion();
	if (status != STATUS_OK)
	{
		TRACEINTO << "Status:" << (DWORD)status;
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::CalculateResourceReport(CRsrcReport* pReport)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->CalculateResourceReport(pReport);
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::CalculateConfResourceReport(CSharedRsrcConfReport* pReport)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->CalculateConfResourceReport(pReport);
}

////////////////////////////////////////////////////////////////////////////
BOOL CResourcesInterfaceArray::CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
															BYTE i_rmxPortGaugeThreshold /*= FALSE*/, BOOL* pbAddAudioAsVideo /*= NULL*/,
															eConfModeTypes confModeType /*= eNonMix*/, BOOL countPartyAsICEinMFW /*= FALSE*/)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->CheckIfOneMorePartyCanBeAdded(videoPartyType, partyRole, allocPolicy, pResult, i_rmxPortGaugeThreshold, pbAddAudioAsVideo, confModeType, countPartyAsICEinMFW);
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CResourcesInterfaceArray::CheckIfOneMorePartyCanBeAddedCOP(ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->CheckIfOneMorePartyCanBeAddedCOP(pResult);
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CResourcesInterfaceArray::CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole,
														   BOOL* pbAddAudioAsVideo /*= NULL*/, eConfModeTypes confModeType /*= eNonMix*/)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->CheckIfOnePartyCanBechanged(oldVideoPartyType, oldPartyRole, newVideoPartyType, newPartyRole, pbAddAudioAsVideo, confModeType);
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::AddParty(BasePartyDataStruct& rPartyData)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->AddParty(rPartyData);
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::RemoveParty(BasePartyDataStruct& rPartyData)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->RemoveParty(rPartyData);
}

////////////////////////////////////////////////////////////////////////////
BOOL CResourcesInterfaceArray::IsThereAnyParty()
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->IsThereAnyParty();
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
WORD CResourcesInterfaceArray::GetMaxNumberOfParties()
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->GetMaxNumberOfParties();
	return 0;
}

////////////////////////////////////////////////////////////////////////////
DWORD CResourcesInterfaceArray::GetOccupiedNumberOfParties()
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->GetOccupiedNumberOfParties();
	return 0;
}

////////////////////////////////////////////////////////////////////////////
WORD CResourcesInterfaceArray::GetNumberOfRequiredUDPPortsPerBoard(eIceEnvironmentType isIceEnv)//ICE 4 ports
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->GetNumberOfRequiredUDPPortsPerBoard(isIceEnv);
	return 0;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourcesInterfaceArray::IsRsrcEnough(CBoardsStatistics* pBoardsStatistics)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->IsRsrcEnough(pBoardsStatistics);
	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::InitDongleRestriction(DWORD& num_parties, BOOL bRecalculateReservationPartyResources)
{
	m_dongleNumParties = num_parties;
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->InitDongleRestriction(num_parties, bRecalculateReservationPartyResources);
}

////////////////////////////////////////////////////////////////////////////
int CResourcesInterfaceArray::SetSvcDongleRestriction(DWORD num_parties)
{
	m_dongleNumParties = num_parties;
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->SetSvcDongleRestriction(num_parties);
	return num_parties;
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::FillRequiredPartyResourcesForConference(WORD minNumAudioPartiesInConf, WORD minNumVideoPartiesInConf, eVideoPartyType maxVideoPartyTypeInConf, CPartiesResources& partiesResourcesToFill)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->FillRequiredPartyResourcesForConference(minNumAudioPartiesInConf, minNumVideoPartiesInConf, maxVideoPartyTypeInConf, partiesResourcesToFill);
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::UpdateReservatorWithLogicalConfiguration(BOOL bRecalculateReservationPartyResources)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->UpdateReservatorWithLogicalConfiguration(bRecalculateReservationPartyResources);
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::InitReservatorAndStaticCardConfigParams(BOOL bRecalculateReservationPartyResources)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->InitReservatorAndStaticCardConfigParams(bRecalculateReservationPartyResources);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourcesInterfaceArray::ReconfigureUnitsAccordingToProportion(BOOL bForceReconfigure)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->ReconfigureUnitsAccordingToProportion(bForceReconfigure);
	return -1;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourcesInterfaceArray::CanSetConfigurationNow()
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->CanSetConfigurationNow();
	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourcesInterfaceArray::UpdatePortWeightsTo1500Q()
{
	STATUS status = STATUS_OK;

	PTRACE(eLevelInfoNormal, "CResourcesInterfaceArray::UpdatePortWeightsTo1500Q");

	if (CResRsrcCalculator::IsRMX1500QRatios())
	{
		// update port weights for Fixed Mode
		CBaseModeResources* pBaseModeResources = dynamic_cast<CBaseModeResources*>(m_pResourcesInterfaceArray[0]);
		PASSERT_AND_RETURN_VALUE(pBaseModeResources==NULL, STATUS_FAIL);
		pBaseModeResources->UpdatePortWeightsTo1500Q();

		// update port weights for Auto Mode
		pBaseModeResources = dynamic_cast<CBaseModeResources*>(m_pResourcesInterfaceArray[1]);
		PASSERT_AND_RETURN_VALUE(pBaseModeResources==NULL, STATUS_FAIL);
		pBaseModeResources->UpdatePortWeightsTo1500Q();
	}

	CAutoModeResources* pAutoModeResources = dynamic_cast<CAutoModeResources*>(m_pResourcesInterfaceArray[1]);
	PASSERT_AND_RETURN_VALUE(pAutoModeResources==NULL, STATUS_FAIL);

	pAutoModeResources->InitDongleRestriction(m_dongleNumParties, FALSE);
	status = pAutoModeResources->ReconfigureUnitsAccordingToProportion(TRUE);
	if (status != STATUS_OK)
		PTRACE2INT(eLevelError, "CResourcesInterfaceArray::UpdatePortWeightsTo1500Q - ReconfigureUnitsAccordingToProportion FAILED! status=", status);

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::UpdateLogicalWeightsAndValues(eSystemCardsMode cardsMode)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
	{
		CBaseModeResources* pBaseModeResources = dynamic_cast<CBaseModeResources*>(pCurent);
		pBaseModeResources->UpdateLogicalWeightsAndValues(cardsMode);
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourcesInterfaceArray::OnLicenseExpired()
{
	TRACEINTO << "Shoudn't reach here ";
}

////////////////////////////////////////////////////////////////////////////
BOOL CResourcesInterfaceArray::CheckIfOneMorePartyCanBeAddedToIpService(eVideoPartyType& videoPartyType,
																		ePartyRole& partyRole,
																		EAllocationPolicy allocPolicy,
																		ALLOC_PARTY_IND_PARAMS_S* pResult,
																		float service_factor,
																		BOOL round_up,
																		BOOL* pbAddAudioAsVideo /*= NULL*/,
																		eConfModeTypes confModeType /*= eNonMix*/,
																		BOOL countPartyAsICEinMFW /*= FALSE*/)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->CheckIfOneMorePartyCanBeAddedToIpService(videoPartyType, partyRole, allocPolicy, pResult, service_factor, round_up, pbAddAudioAsVideo, confModeType, countPartyAsICEinMFW);

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
BOOL CResourcesInterfaceArray::CheckIfOnePartyCanBechangedInIpService(eVideoPartyType oldVideoPartyType,
																	ePartyRole oldPartyRole,
																	eVideoPartyType& newVideoPartyType,
																	ePartyRole& newPartyRole,
																	float service_factor,
																	BOOL round_up,
																	BOOL* pbAddAudioAsVideo /*= NULL*/,
																	eConfModeTypes confModeType /*= eNonMix*/)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		return pCurent->CheckIfOnePartyCanBechangedInIpService(oldVideoPartyType, oldPartyRole, newVideoPartyType, newPartyRole, service_factor, round_up, pbAddAudioAsVideo, confModeType);
	return FALSE;
}


///////////////////////////////////////////////////////////////////////////
STATUS CResourcesInterfaceArray::SetEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg)
{
	ResourcesInterface* pCurrentResourcesInterface = GetCurrentResourcesInterface();
	PASSERT_AND_RETURN_VALUE(!pCurrentResourcesInterface, STATUS_FAIL);

	CFixedModeResources* pFixedModeResources = dynamic_cast<CFixedModeResources*>(pCurrentResourcesInterface);
	PASSERT_AND_RETURN_VALUE(!pFixedModeResources, STATUS_FAIL);

	return pFixedModeResources->SetEnhancedConfiguration(pEnhancedCfg,FALSE,(DWORD)-1);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourcesInterfaceArray::CheckSetEnhancedConfiguration()
{
	if (CHelperFuncs::IsFixedModeAllocationType())
	{
		ResourcesInterface* pCurrentResourcesInterface = GetCurrentResourcesInterface();
		PASSERT_AND_RETURN_VALUE(!pCurrentResourcesInterface, STATUS_FAIL);
		CFixedModeResources* pFixedModeResources = dynamic_cast<CFixedModeResources*>(pCurrentResourcesInterface);
		PASSERT_AND_RETURN_VALUE(!pFixedModeResources, STATUS_FAIL);

		return pFixedModeResources->CheckSetEnhancedConfiguration(FALSE, (DWORD)-1);
	}
	return STATUS_FAIL;
}


////////////////////////////////////////////////////////////////////////////
//                        CIpServiceResourcesInterfaceArray
////////////////////////////////////////////////////////////////////////////
CIpServiceResourcesInterfaceArray::CIpServiceResourcesInterfaceArray(WORD serviceId,const char* serviceName, float  service_factor, BOOL round_up)
	:m_serviceId(serviceId),m_service_factor(service_factor),m_round_up(round_up)
{
	SetServiceName(serviceName);

	TRACEINTO << "ServiceName:" << GetServiceName();
}

////////////////////////////////////////////////////////////////////////////
CIpServiceResourcesInterfaceArray::CIpServiceResourcesInterfaceArray(const CIpServiceResourcesInterfaceArray& other)
                                  :CResourcesInterfaceArray(other)
{
	SetServiceName(other.GetServiceName());

	m_serviceId      = other.m_serviceId;
	m_service_factor = other.m_service_factor;
	m_round_up       = other.m_round_up;
	m_partyIdList    = other.m_partyIdList;
}

////////////////////////////////////////////////////////////////////////////
const CIpServiceResourcesInterfaceArray& CIpServiceResourcesInterfaceArray::operator=(const CIpServiceResourcesInterfaceArray& other)
{
	SetServiceName(other.GetServiceName());

	m_serviceId      = other.m_serviceId;
	m_service_factor = other.m_service_factor;
	m_round_up       = other.m_round_up;
	m_partyIdList    = other.m_partyIdList;

	return *this;
}

////////////////////////////////////////////////////////////////////////////
CIpServiceResourcesInterfaceArray::~CIpServiceResourcesInterfaceArray()
{
	TRACEINTO << "ServiceName:" << GetServiceName();
}

////////////////////////////////////////////////////////////////////////////
bool operator==(const CIpServiceResourcesInterfaceArray& lhs,const CIpServiceResourcesInterfaceArray& rhs)
{
	return (lhs.m_serviceId == rhs.m_serviceId);
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CIpServiceResourcesInterfaceArray& lhs, const CIpServiceResourcesInterfaceArray& rhs)
{
	return (lhs.m_serviceId < rhs.m_serviceId);
}

////////////////////////////////////////////////////////////////////////////
void CIpServiceResourcesInterfaceArray::UpdateServiceWeight(float service_factor, BOOL round_up)
{
	if (service_factor >= 0 && service_factor <= 1)
	{
		m_service_factor = service_factor;
	}
	else
	{
		DBGPASSERT(101);
	}
	m_round_up = round_up;
}

////////////////////////////////////////////////////////////////////////////
void CIpServiceResourcesInterfaceArray::CalculateResourceReportPerService(CRsrcReport* pReport)
{
	CalculateResourceReport(pReport);

	TRACEINTO << "ServiceName:" << m_serviceName << ", ServiceId:" << m_serviceId << ", ServiceFactor:" << m_service_factor << ", RoundUp:" << (int)m_round_up << ", " << *pReport;

	int numOfTypes = 0;
	if (CHelperFuncs::IsFixedModeAllocationType())
	{
		numOfTypes = NUM_OF_PARTY_RESOURCE_TYPES;
	}
	else
	{
		numOfTypes = 2; //audio + video
	}

	WORD ipServiceTotal = 0;
	WORD ipServiceFree = 0;
	WORD ipServiceOccupied = 0;
	WORD ipServiceReserved = 0;

	for (int i = 0; i < numOfTypes; i++)
	{
		float floatIpServiceTotal = pReport->GetNumParties((ePartyResourceTypes)i, TYPE_TOTAL) * m_service_factor;

		if (m_round_up)
		{
			if (floatIpServiceTotal != floor(floatIpServiceTotal))
			{
				floatIpServiceTotal = floor(floatIpServiceTotal) + 1;
			}
		}
		else
		{
			floatIpServiceTotal = floor(floatIpServiceTotal);
		}

		ipServiceTotal    = pReport->GetNumParties((ePartyResourceTypes)i, TYPE_TOTAL);
		ipServiceFree     = pReport->GetNumParties((ePartyResourceTypes)i, TYPE_FREE);
		ipServiceOccupied = pReport->GetNumParties((ePartyResourceTypes)i, TYPE_OCCUPIED);
		ipServiceReserved = pReport->GetNumParties((ePartyResourceTypes)i, TYPE_RESERVED);

		ipServiceTotal = (WORD)floatIpServiceTotal;
		ipServiceFree = ipServiceTotal - ipServiceOccupied - ipServiceReserved;

		pReport->SetNumParties((ePartyResourceTypes)i, TYPE_TOTAL, ipServiceTotal);
		pReport->SetNumParties((ePartyResourceTypes)i, TYPE_OCCUPIED, ipServiceOccupied);
		pReport->SetNumParties((ePartyResourceTypes)i, TYPE_FREE, ipServiceFree);
	}

	TRACEINTO << "Calculation results:" << *pReport;
}

////////////////////////////////////////////////////////////////////////////
WORD CIpServiceResourcesInterfaceArray::GetMaxNumberOfParties()
{
	WORD MaxNumberOfParties = CResourcesInterfaceArray::GetMaxNumberOfParties();
	DWORD ipServiceMaxNumberOfParties = CHelperFuncs::CalcIpServicePart((DWORD)MaxNumberOfParties, m_service_factor, m_round_up);
	return (WORD)ipServiceMaxNumberOfParties;
}

////////////////////////////////////////////////////////////////////////////
BOOL CIpServiceResourcesInterfaceArray::CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType,
																		ePartyRole& partyRole,
																		EAllocationPolicy allocPolicy,
																		ALLOC_PARTY_IND_PARAMS_S* pResult,
																		BOOL* pbAddAudioAsVideo /*= NULL*/,
																		eConfModeTypes confModeType /*= eNonMix*/,
																		BOOL countPartyAsICEinMFW /*= FALSE*/)
{
	return CResourcesInterfaceArray::CheckIfOneMorePartyCanBeAddedToIpService(videoPartyType, partyRole, allocPolicy, pResult, m_service_factor, m_round_up, pbAddAudioAsVideo, confModeType, countPartyAsICEinMFW);
}

////////////////////////////////////////////////////////////////////////////
BOOL CIpServiceResourcesInterfaceArray::CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole,
																	BOOL* pbAddAudioAsVideo /*= NULL*/, eConfModeTypes confModeType /*= eNonMix*/)
{
	return CResourcesInterfaceArray::CheckIfOnePartyCanBechangedInIpService(oldVideoPartyType, oldPartyRole, newVideoPartyType, newPartyRole, m_service_factor, m_round_up, pbAddAudioAsVideo, confModeType);
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, const CIpServiceResourcesInterfaceArray& val)
{
	os
		<< "\n  ServiceName      :" << val.m_serviceName
		<< "\n  ServiceId        :" << val.m_serviceId
		<< "\n  ServiceFactor    :" << val.m_service_factor
		<< "\n  RoundUp          :" << (int)val.m_round_up
		<< "\n  DongleNumParties :" << val.GetDongleNumOfParties()
		<< "\n  IpServicePart    :" << CHelperFuncs::CalcIpServicePart(val.GetDongleNumOfParties(), val.m_service_factor, val.m_round_up);
	return os;
}

////////////////////////////////////////////////////////////////////////////
void CIpServiceResourcesInterfaceArray::AddParty(BasePartyDataStruct& rPartyData)
{
	if (STATUS_OK == AddPartyId(rPartyData.m_partyId))
		CResourcesInterfaceArray::AddParty(rPartyData);
	else
		TRACEINTO << "PartyId:" << rPartyData.m_partyId << " - Failed to add party";
}

////////////////////////////////////////////////////////////////////////////
void CIpServiceResourcesInterfaceArray::RemoveParty(BasePartyDataStruct& rPartyData)
{
	if (STATUS_OK == RemovePartyId(rPartyData.m_partyId))
		CResourcesInterfaceArray::RemoveParty(rPartyData);
	else
		TRACEINTO << "PartyId:" << rPartyData.m_partyId << " - Failed to remove party";
}

////////////////////////////////////////////////////////////////////////////
STATUS CIpServiceResourcesInterfaceArray::AddPartyId(PartyRsrcID partyId)
{
	if (FindPartyId(partyId))
	{
		PASSERTSTREAM(1, "PartyId:" << partyId);
		return STATUS_PARTY_ALREADY_EXISTS;
	}
	m_partyIdList.insert(partyId);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CIpServiceResourcesInterfaceArray::RemovePartyId(PartyRsrcID partyId)
{
	if (!FindPartyId(partyId))
	{
		PASSERTSTREAM(1, "PartyId:" << partyId);
		return STATUS_NOT_FOUND;
	}
	m_partyIdList.erase(partyId);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
BOOL CIpServiceResourcesInterfaceArray::FindPartyId(PartyRsrcID partyId) const
{
	PartyIdList::iterator index_itr = m_partyIdList.find(partyId);
	return (index_itr != m_partyIdList.end()) ? TRUE : FALSE;
}

////////////////////////////////////////////////////////////////////////////
STATUS CIpServiceResourcesInterfaceArray::SetEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg)
{
	PASSERT_AND_RETURN_VALUE(!m_pResourcesInterfaceArray[m_CurrentResourcesInterfaceIndex], STATUS_FAIL);
	CFixedModeResources* pFixedModeResources = dynamic_cast<CFixedModeResources*>(m_pResourcesInterfaceArray[m_CurrentResourcesInterfaceIndex]);
	PASSERT_AND_RETURN_VALUE(!pFixedModeResources, STATUS_FAIL);

	return pFixedModeResources->SetEnhancedConfiguration(pEnhancedCfg, TRUE, m_serviceId);
}

////////////////////////////////////////////////////////////////////////////
STATUS CIpServiceResourcesInterfaceArray::CheckSetEnhancedConfiguration()
{
	if (CHelperFuncs::IsFixedModeAllocationType())
	{
		PASSERT_AND_RETURN_VALUE(!m_pResourcesInterfaceArray[m_CurrentResourcesInterfaceIndex], STATUS_FAIL);
		CFixedModeResources* pFixedModeResources = dynamic_cast<CFixedModeResources*>(m_pResourcesInterfaceArray[m_CurrentResourcesInterfaceIndex]);
		PASSERT_AND_RETURN_VALUE(!pFixedModeResources, STATUS_FAIL);

		return pFixedModeResources->CheckSetEnhancedConfiguration(TRUE, m_serviceId);
	}
	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
STATUS CIpServiceResourcesInterfaceArray::SetPortConfiguration(WORD index)
{
	CAutoModeResources* pAutoModeResources = dynamic_cast<CAutoModeResources*>(m_pResourcesInterfaceArray[m_CurrentResourcesInterfaceIndex]);
	PASSERT_AND_RETURN_VALUE(!pAutoModeResources, STATUS_FAIL);

	CPortsConfig* pPortsCfg = pAutoModeResources->GetPortsConfig();
	if (pPortsCfg)
	{
		CAudioVideoConfig* pAudVidCfg = pPortsCfg->FindPortsConfigurationConfigByIndex(index);
		if (pAudVidCfg)
		{
			pPortsCfg->SetSelectedIndex(index);
			pAutoModeResources->UpdateAudioVideoConfigProportion();
			return STATUS_OK;
		}
	}
	return STATUS_FAIL;
}

void CResourcesInterfaceArray::GetLogicalPartyWeight(eVideoPartyType videoPartyType, float& logicalPartyWeightInNonMix, float& logicalPartyWeightInMix)
{
	ResourcesInterface* pCurent = GetCurrentResourcesInterface();
	if (pCurent)
		pCurent->GetLogicalPartyWeight(videoPartyType, logicalPartyWeightInNonMix, logicalPartyWeightInMix);
}
