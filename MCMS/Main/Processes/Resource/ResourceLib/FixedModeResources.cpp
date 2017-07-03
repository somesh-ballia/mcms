#include "TraceStream.h"
#include "FixedModeResources.h"
#include "HelperFuncs.h"
#include "EnumsAndDefines.h"
#include "SystemResources.h"
#include "InternalProcessStatuses.h"
#include "CardResourceConfig.h"
#include "EnhancedConfigResponse.h"
#include "FaultsDefines.h"
#include "ResourceManager.h"
#include "CRsrcDetailGet.h"
#include "SharedRsrcList.h"
#include "AutoModeResources.h"
#include <memory>

#undef min
#include "PrettyTable.h"

////////////////////////////////////////////////////////////////////////////
//                        CFixedModeResources
////////////////////////////////////////////////////////////////////////////
CFixedModeResources::CFixedModeResources(eSystemCardsMode cardMode) : CBaseModeResources(cardMode)
{
	m_dongleNumParties = 0;
	m_totalPromillesAccordingToCards = 0;

	memset(m_numOfCurrentPartiesPerType, 0, sizeof(m_numOfCurrentPartiesPerType));
	memset(m_numOfCurrentGlidesPerType, 0, sizeof(m_numOfCurrentGlidesPerType));
	memset(m_MaximumPartiesAccordingToDongle, 0, sizeof(m_MaximumPartiesAccordingToDongle));
	memset(m_MaximumPartiesAccordingToCards, 0, sizeof(m_MaximumPartiesAccordingToCards));

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		eVideoPartyType videoPartyType = PartyResourceTypeToVideoPartyType((ePartyResourceTypes)i, cardMode);
		m_VideoPromillesOfParty[i] = CHelperFuncs::GetNumPhysicalHD720Ports(videoPartyType) * m_VID_TOTAL_HD720_PROMILLES;
		m_TotalPromillesOfParty[i] = m_VideoPromillesOfParty[i] + m_ART_PROMILLES;
	}

	PrintToTrace(m_TotalPromillesOfParty, "TotalPromillesOfPartyPerType");
	PrintToTrace(m_VideoPromillesOfParty, "VideoPromillesOfPartyPerType");
}

//--------------------------------------------------------------------------
CFixedModeResources::~CFixedModeResources()
{
}

//--------------------------------------------------------------------------
void CFixedModeResources::AddParty(BasePartyDataStruct& rPartyData)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();

	PASSERT_AND_RETURN(!pSystemResources);
	PASSERT_AND_RETURN(!pResourceManager);

	ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(rPartyData.m_videoPartyType);

	if (m_numOfCurrentPartiesPerType[partyResourceType] < m_enhancedConfig.GetConfiguration(partyResourceType))
		m_numOfCurrentPartiesPerType[partyResourceType]++;
	else
	{
		for (int j = partyResourceType+1; j < NUM_OF_PARTY_RESOURCE_TYPES; j++)
		{
			if (m_numOfCurrentPartiesPerType[j] < m_enhancedConfig.GetConfiguration(j))
			{
				ePartyResourceTypes glidingPartyResourceType = (ePartyResourceTypes)j;
				TRACEINTO << "Gliding from " << partyResourceType << " to " << glidingPartyResourceType;
				m_numOfCurrentPartiesPerType[j]++;
				m_numOfCurrentGlidesPerType[partyResourceType][j]++;
				break;
			}
		}
	}

	DWORD portGauge = pSystemResources->GetPortGauge();
	BOOL  portGaugeAlarm = NO;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("PORT_GAUGE_ALARM", portGaugeAlarm);

	TRACEINTO << "videoPartyType:" << eVideoPartyTypeNames[rPartyData.m_videoPartyType] << ", portGauge:" << portGauge << "%, portGaugeAlarm:" << (int)portGaugeAlarm;

	if (portGaugeAlarm)
		CheckIfNeedGaugeAlarm(pResourceManager, rPartyData.m_videoPartyType, portGauge, true);
}

//--------------------------------------------------------------------------
void CFixedModeResources::RemoveParty(BasePartyDataStruct& rPartyData)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();

	PASSERTMSG_AND_RETURN(!pSystemResources, "Failed - pSystemResources is NULL");
	PASSERTMSG_AND_RETURN(!pResourceManager, "Failed - pResourceManager is NULL");

	ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(rPartyData.m_videoPartyType);

	bool IsPartyRemoved = false; // first check if there are some glides
	for (int j = NUM_OF_PARTY_RESOURCE_TYPES-1; j > partyResourceType; j--)
	{
		if (m_numOfCurrentGlidesPerType[partyResourceType][j] > 0)
		{
			ePartyResourceTypes glidingPartyResourceType = (ePartyResourceTypes)j;
			TRACEINTO << "Removing gliding from " << partyResourceType << " to " << glidingPartyResourceType;
			m_numOfCurrentGlidesPerType[partyResourceType][j]--;
			PASSERT_AND_RETURN(m_numOfCurrentPartiesPerType[j] <= 0);
			m_numOfCurrentPartiesPerType[j]--;
			IsPartyRemoved = true;
		}
	}

	if (!IsPartyRemoved)  // if no glide, remove regular one
	{
		PASSERT_AND_RETURN(m_numOfCurrentPartiesPerType[partyResourceType] <= 0);
		m_numOfCurrentPartiesPerType[partyResourceType]--;
	}

	WORD portGauge = pSystemResources->GetPortGauge();

	TRACEINTO << "videoPartyType:" << eVideoPartyTypeNames[rPartyData.m_videoPartyType] << ", portGauge:" << portGauge << "%";

  CheckIfNeedGaugeAlarm(pResourceManager, rPartyData.m_videoPartyType, portGauge, false);
}

//--------------------------------------------------------------------------
void CFixedModeResources::CheckIfNeedGaugeAlarm(CResourceManager* pResourceManager, eVideoPartyType videoPartyType, DWORD portGauge, bool bAddParty)
{
	std::auto_ptr<CRsrcReport> pReport(new CRsrcReport());
	CalculateResourceReport(pReport.get());

	char* message = NULL;
	if (videoPartyType == eVideo_party_type_none)
	{
		float numMaxAudParties = pReport->GetNumParties(e_Audio, TYPE_TOTAL);
		float numCurAudParties = pReport->GetNumParties(e_Audio, TYPE_OCCUPIED);

		if (ceil((numCurAudParties*100)/numMaxAudParties) >= portGauge)
		{
			if (bAddParty)
			{
				message = "System resources of Audio ports usage has exceeded Port Gauge threshold";
				pResourceManager->AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT, AUDIO_PORT_GAUGE_THRESHOLD_REACHED, MAJOR_ERROR_LEVEL, message, true, true);
			}
		}
		else
		{
			if (!bAddParty)
			{
				message = "System resources of Audio ports usage not exceeded Port Gauge threshold";
				pResourceManager->RemoveActiveAlarm(AUDIO_PORT_GAUGE_THRESHOLD_REACHED);
			}
		}

		if (message)
		{
			TRACEINTO << "numCurAudParties:" << numCurAudParties << ", numMaxAudParties:" << numMaxAudParties << " - " << message;
		}
	}
	else
	{
		float numMaxVidParties = pReport->GetNumParties(e_Cif,       TYPE_TOTAL   ) * m_LogicalHD720WeightAvcParty[eNonMix][e_Cif      ]+
		                         pReport->GetNumParties(e_SD30,      TYPE_TOTAL   ) * m_LogicalHD720WeightAvcParty[eNonMix][e_SD30     ]+
		                         pReport->GetNumParties(e_HD720,     TYPE_TOTAL   ) * m_LogicalHD720WeightAvcParty[eNonMix][e_HD720    ]+
		                         pReport->GetNumParties(e_HD1080p30, TYPE_TOTAL   ) * m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p30];

		float numCurVidParties = pReport->GetNumParties(e_Cif,       TYPE_OCCUPIED) * m_LogicalHD720WeightAvcParty[eNonMix][e_Cif      ]+
		                         pReport->GetNumParties(e_SD30,      TYPE_OCCUPIED) * m_LogicalHD720WeightAvcParty[eNonMix][e_SD30     ]+
		                         pReport->GetNumParties(e_HD720,     TYPE_OCCUPIED) * m_LogicalHD720WeightAvcParty[eNonMix][e_HD720    ]+
		                         pReport->GetNumParties(e_HD1080p30, TYPE_OCCUPIED) * m_LogicalHD720WeightAvcParty[eNonMix][e_HD1080p30];

		if (ceil((numMaxVidParties*100)/numCurVidParties) >= portGauge)
		{
			if (bAddParty)
			{
				message = "System resources of Video ports usage has exceeded Port Gauge threshold";
				pResourceManager->AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT, VIDEO_PORT_GAUGE_THRESHOLD_REACHED, MAJOR_ERROR_LEVEL, message, true, true);
			}
		}
		else
		{
			if (!bAddParty)
			{
				message = "System resources of Video ports usage not exceeded Port Gauge threshold";
				pResourceManager->RemoveActiveAlarm(VIDEO_PORT_GAUGE_THRESHOLD_REACHED);
			}
		}

		if (message)
		{
			TRACEINTO << "numCurVidParties:" << numCurVidParties << ", numMaxVidParties:" << numMaxVidParties << " - " << message;
		}
	}
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::IsThereAnyParty()
{
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		if (m_numOfCurrentPartiesPerType[i] != 0)
			return TRUE;

	return FALSE;
}

//--------------------------------------------------------------------------
DWORD CFixedModeResources::GetOccupiedNumberOfParties()
{
	DWORD num_parties = 0;
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		num_parties += m_numOfCurrentPartiesPerType[i];

	TRACEINTO << "num_parties:" << num_parties;
	return num_parties;
}

//--------------------------------------------------------------------------
float CFixedModeResources::GetTotalOccupiedPromilles(BOOL bAlsoAudio)
{
	float totalOccupied = 0;
	for (int i = (!bAlsoAudio) ? 1 : 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		totalOccupied += m_TotalPromillesOfParty[i]*m_numOfCurrentPartiesPerType[i];
	return totalOccupied;
}

//--------------------------------------------------------------------------
float CFixedModeResources::GetTotalOccupiedLogicalWeight(BOOL bAlsoAudio)
{
	float totalOccupied = 0;
	for (int i = (!bAlsoAudio) ? 1 : 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		totalOccupied += m_LogicalHD720WeightAvcParty[eNonMix][i] * m_numOfCurrentPartiesPerType[i];
	return totalOccupied;
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckIfOneMorePartyCanBeAddedToIpService(eVideoPartyType& videoPartyType,
                                                                   ePartyRole& partyRole,
                                                                   EAllocationPolicy allocPolicy,
                                                                   ALLOC_PARTY_IND_PARAMS_S* pResult,
                                                                   float service_factor,
                                                                   BOOL round_up,
                                                                   BOOL* pbAddAudioAsVideo /*= NULL*/,
                                                                   eConfModeTypes confModeType /*= eNonMix*/,
                                                                   BOOL countPartyAsICEinMFW /*= FALSE*/)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", TRUE);

	WORD numHD720PortsAccordingToCards = pSystemResources->GetHD720PortsAccordingToCards();
	WORD service_numHD720PortsAccordingToCards = (WORD) CHelperFuncs::CalcIpServicePart((DWORD)numHD720PortsAccordingToCards, service_factor, round_up);

	CEnhancedConfig ip_service_enhancedConfig = m_enhancedConfig;
	ip_service_enhancedConfig.SetIpServicePartConfig(service_factor, round_up);

	return CheckIfOneMorePartyCanBeAdded(videoPartyType, partyRole, allocPolicy, pResult, service_numHD720PortsAccordingToCards, ip_service_enhancedConfig, FALSE, confModeType, countPartyAsICEinMFW);
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType,
                                                        ePartyRole& partyRole,
                                                        EAllocationPolicy allocPolicy,
                                                        ALLOC_PARTY_IND_PARAMS_S* pResult,
                                                        BYTE i_rmxPortGaugeThreshold /*= FALSE*/,
                                                        BOOL* pbAddAudioAsVideo /*= NULL*/,
                                                        eConfModeTypes confModeType /*= eNonMix*/,
                                                        BOOL countPartyAsICEinMFW /*= FALSE*/)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", TRUE);

	WORD numHD720PortsAccordingToCards = pSystemResources->GetHD720PortsAccordingToCards();
	return CheckIfOneMorePartyCanBeAdded(videoPartyType, partyRole, allocPolicy, pResult, numHD720PortsAccordingToCards, m_enhancedConfig, i_rmxPortGaugeThreshold, confModeType, countPartyAsICEinMFW);
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType,
                                                        ePartyRole& partyRole,
                                                        EAllocationPolicy allocPolicy,
                                                        ALLOC_PARTY_IND_PARAMS_S* pResult,
                                                        WORD numHD720PortsAccordingToCards,
                                                        CEnhancedConfig& enhancedConfig,
                                                        BYTE i_rmxPortGaugeThreshold /*= FALSE*/,
                                                        eConfModeTypes confModeType /*= eNonMix*/,
                                                        BOOL countPartyAsICEinMFW /*= FALSE*/)
{
#define COMMON_TRACE(s) \
	TRACEINTO << s \
	          << "\n  videoPartyType                :" << eVideoPartyTypeNames[videoPartyType] << " (" << videoPartyType << ")" \
	          << "\n  partyRole                     :" << ePartyRoleNames[partyRole] << " (" << partyRole << ")" \
	          << "\n  allocPolicy                   :" << AllocationPolicyToString(allocPolicy) << " (" << allocPolicy << ")"\
	          << "\n  numHD720PortsAccordingToCards :" << numHD720PortsAccordingToCards \
	          << "\n  downgradedType                :" << to_string((ePartyResourceTypes)i) \
	          << "\n  numCurPartiesType             :" << m_numOfCurrentPartiesPerType[i] \
	          << "\n  numMaxPartiesType             :" << enhancedConfig.GetConfiguration(i) \
	          << "\n  HD720WeightOfParty            :" << m_LogicalHD720WeightAvcParty[confModeType][i] \
	          << "\n  totalWithNewParty             :" << totalWithNewParty;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();

	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", FALSE);
	PASSERTMSG_AND_RETURN_VALUE(!pResourceManager, "Failed, pResourceManager is NULL", FALSE);

	eSystemCardsMode systemCardsMode = pSystemResources->GetSystemCardsMode();

	// check related to RamSize
	if (eSystemRamSize_half == pSystemResources->GetRamSize()) // only for 0.5GB
	{
		if (MAX_PARTIES_FOR_RAM_HALF_SIZE <= GetOccupiedNumberOfParties())
		{
			pResult->allocIndBase.status = STATUS_INSUFFICIENT_RSRC;
			TRACEINTO << "Status:STATUS_INSUFFICIENT_RSRC - Failed due to 0.5 RAM size limitation";
			return FALSE;
		}
	}

	float total = 0;
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		total += m_LogicalHD720WeightAvcParty[confModeType][i]*m_numOfCurrentPartiesPerType[i];

	// Check with enhanced configuration, and with cards, if necessary glide up
	BOOL canBeAllocatedAccordingToEnhancedConfigAndCards = FALSE;
	ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(videoPartyType);
	for (int i = partyResourceType; i < NUM_OF_PARTY_RESOURCE_TYPES; i++) // try partyResourceType and up
	{
		float totalWithNewParty = total+m_LogicalHD720WeightAvcParty[confModeType][i];
		if (m_numOfCurrentPartiesPerType[i] < enhancedConfig.GetConfiguration(i) && numHD720PortsAccordingToCards+SMALL_ERROR >= totalWithNewParty)
		{
			canBeAllocatedAccordingToEnhancedConfigAndCards = TRUE;
			break;
		}
		else
		{
			COMMON_TRACE("Can not use video type 'downgradedType', number of parties from this type passed the allowed number configured or ports are occupied")
		}
	}

	if (canBeAllocatedAccordingToEnhancedConfigAndCards == FALSE)
	{
		// Try to down-grade
		for (int i = partyResourceType-1; i >= 0; i--)
		{
			float totalWithNewParty = total+m_LogicalHD720WeightAvcParty[confModeType][i];
			if (m_numOfCurrentPartiesPerType[i] < enhancedConfig.GetConfiguration(i) && numHD720PortsAccordingToCards+SMALL_ERROR >= totalWithNewParty)
			{
				canBeAllocatedAccordingToEnhancedConfigAndCards = TRUE;
				if (i == e_Audio && allocPolicy != eAllowDowngradingToAudioOnly)
				{
					COMMON_TRACE("Can not down-grade to Audio due to allocPolicy")
					pResult->allocIndBase.status = (CHelperFuncs::IsAudioParty(videoPartyType)) ? STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED : STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED;
					return FALSE;
				}

				partyResourceType = (ePartyResourceTypes)i;
				videoPartyType = PartyResourceTypeToVideoPartyType(partyResourceType, systemCardsMode);
				canBeAllocatedAccordingToEnhancedConfigAndCards = TRUE;
				break;
			}
			else
			{
				COMMON_TRACE("Can not down-grade, we will pass the down-grade type's permitted ports or we will pass allowed CIF number")
			}
		}
	}

	if (canBeAllocatedAccordingToEnhancedConfigAndCards == FALSE)
	{
		if (CHelperFuncs::IsAudioParty(videoPartyType))
		{
			pResult->allocIndBase.status = STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED;
			TRACEINTO << "Status:NUMBER_OF_AUDIO_PARTIES_EXCEEDED - Failed due to enhanced configuration";
		}
		else
		{
			pResult->allocIndBase.status = STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED;
			TRACEINTO << "Status:STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED - Failed due to enhanced configuration";
		}
		return FALSE;
	}

	// Still we will also check with logical and physical, because maybe there's some problem here
	float oldTotalOccupiedLogicalWeight  = GetTotalOccupiedLogicalWeight(TRUE);
	float oldTotalOccupiedPromilles      = GetTotalOccupiedPromilles(TRUE);
	float newTotalOccupiedLogicalWeight  = oldTotalOccupiedLogicalWeight+m_LogicalHD720WeightAvcParty[confModeType][partyResourceType];
	float newTotalOccupiedPromilles      = oldTotalOccupiedPromilles+m_TotalPromillesOfParty[partyResourceType];

	TRACEINTO << "\n  VideoPartyType                :" << eVideoPartyTypeNames[videoPartyType] << " (" << videoPartyType << ")"
	          << "\n  PartyRole                     :" << ePartyRoleNames[partyRole] << " (" << partyRole << ")"
	          << "\n  PartyResourceType             :" << partyResourceType
	          << "\n  AllocPolicy                   :" << allocPolicy
	          << "\n  NumHD720PortsAccordingToCards :" << numHD720PortsAccordingToCards
	          << "\n  OldTotalOccupiedLogicalWeight :" << oldTotalOccupiedLogicalWeight
	          << "\n  OldTotalOccupiedPromilles     :" << oldTotalOccupiedPromilles
	          << "\n  NewTotalOccupiedLogicalWeight :" << newTotalOccupiedLogicalWeight
	          << "\n  NewTotalOccupiedPromilles     :" << newTotalOccupiedPromilles
	          << "\n  TotalPromillesAccordingToCards:" << m_totalPromillesAccordingToCards;

	// PrintToTrace(m_numOfCurrentPartiesPerType, "m_numOfCurrentPartiesPerType");

	if (newTotalOccupiedLogicalWeight > m_dongleNumParties + SMALL_ERROR || newTotalOccupiedPromilles > m_totalPromillesAccordingToCards + SMALL_ERROR)
	{
		// If there's not enough, try to down-grade
		BOOL bFound = FALSE;
		ePartyResourceTypes downgradedType;
		int intDowngradedType;
		for (intDowngradedType = partyResourceType-1; intDowngradedType >= 0; intDowngradedType--)
		{
			newTotalOccupiedLogicalWeight = oldTotalOccupiedLogicalWeight + m_LogicalHD720WeightAvcParty[confModeType][intDowngradedType];
			newTotalOccupiedPromilles     = oldTotalOccupiedPromilles + m_TotalPromillesOfParty[intDowngradedType];
			if (newTotalOccupiedLogicalWeight <= m_dongleNumParties + SMALL_ERROR && newTotalOccupiedPromilles <= m_totalPromillesAccordingToCards + SMALL_ERROR)
			{
				TRACEINTO << "Down-graded to " << (ePartyResourceTypes)intDowngradedType;
				bFound = TRUE;
				break;
			}
		}

		if (bFound == FALSE)
		{
			pResult->allocIndBase.status = (CHelperFuncs::IsAudioParty(videoPartyType)) ? STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED : STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED;
			TRACEINTO << "Status:" << pResult->allocIndBase.status << " - Failed, All types failed down-grade due to insufficient resources dongleNumParties or Promilles limitations";
			return FALSE;
		}

		downgradedType = (ePartyResourceTypes)intDowngradedType;

		// If only audio could be allocated but we can't down-grade to audio only
		if (downgradedType == e_Audio && allocPolicy != eAllowDowngradingToAudioOnly)
		{
			pResult->allocIndBase.status = (CHelperFuncs::IsAudioParty(videoPartyType)) ? STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED : STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED;
			TRACEINTO << "Status:" << pResult->allocIndBase.status << " - Failed, AllocPolicy not allow down-grading to AudioOnly";
			return FALSE;
		}

		videoPartyType = PartyResourceTypeToVideoPartyType(downgradedType, systemCardsMode);
	}

	if (i_rmxPortGaugeThreshold)
	{
		std::auto_ptr<CRsrcReport> pReport(new CRsrcReport());
		STATUS status = pSystemResources->CalculateResourceReport(pReport.get());
		PASSERTSTREAM_AND_RETURN_VALUE(status != STATUS_OK, "Status:" << status, FALSE);

		DWORD portGauge = pSystemResources->GetPortGauge();

		float numMaxAudParties = pReport->GetNumParties(e_Audio, TYPE_TOTAL);
		float numCurAudParties = pReport->GetNumParties(e_Audio, TYPE_OCCUPIED)+1;
		if (((numCurAudParties*100)/numMaxAudParties) >= portGauge)
		{
			pResult->allocIndBase.status = STATUS_RMX_PORT_GAUGE_THRESHOLD_EXCEEDED;
			TRACEINTO << "numCurAudParties:" << numCurAudParties << ", numMaxAudParties:" << numMaxAudParties << " - Failed, system resources of Audio ports usage has exceeded Port Gauge threshold";
			return FALSE;
		}
	}

	return TRUE;
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType,
                                                      ePartyRole oldPartyRole,
                                                      eVideoPartyType& newVideoPartyType,
                                                      ePartyRole& newPartyRole,
                                                      BOOL* pbAddAudioAsVideo /*= NULL*/,
                                                      eConfModeTypes confModeType /*= eNonMix*/)
{
	return CheckIfOnePartyCanBechanged(oldVideoPartyType, oldPartyRole, newVideoPartyType, newPartyRole, m_enhancedConfig);
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType, ePartyRole oldPartyRole, eVideoPartyType& newVideoPartyType, ePartyRole& newPartyRole, CEnhancedConfig& enhancedConfig)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", FALSE);

	eSystemCardsMode systemCardsMode = pSystemResources->GetSystemCardsMode();

	// try this party type and up, see if there are enough resources to add one
	ePartyResourceTypes newPartyResourceType = VideoPartyTypeToPartyResourceType(newVideoPartyType);
	ePartyResourceTypes oldPartyResourceType = VideoPartyTypeToPartyResourceType(oldVideoPartyType);

	// Dynamic Ports Allocation v7.1 - For reallocate to audio
	if (CHelperFuncs::IsAudioParty(newVideoPartyType) &&
	    m_numOfCurrentPartiesPerType[newPartyResourceType]+1 > m_enhancedConfig.GetConfiguration(newPartyResourceType))
	{
		TRACEINTO << "CanBechanged:FALSE - No available Audio ports in configuration";
		return FALSE;
	}

	if (oldVideoPartyType >= newVideoPartyType)
	{
		// for v to v when ports of newVideoPartyType exist.
		// or only higher exist - return ok. occupy the higher in rsrc but party think the req was allocated.
		TRACEINTO << "CanBechanged:FALSE - oldVideoPartyType >= newVideoPartyType";
		return TRUE;
	}

	// Dynamic Ports Allocation v7.1 - low V to high V - req until orig is legal  //type and up
	for (int i = newPartyResourceType; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		// check if there's simply enough resources for the new party type and up
		if (m_numOfCurrentPartiesPerType[i] < enhancedConfig.GetConfiguration(i))
			return TRUE;

		// check if there's a party from the old party type, gliding up to the new party type (or more), then it's also OK
		if (m_numOfCurrentGlidesPerType[oldPartyResourceType][i] > 0)
			return TRUE;
	}

	// Dynamic Ports Allocation v7.1 - downgrade to lower video resource.
	for (int i = newPartyResourceType-1; i > oldPartyResourceType; i--) // If was'n found the type and up -- downgrade.
	{
		TRACEINTO << "PartyResourceType:" << i
		          << ", numOfCurrentPartiesPerType[i]+1=" << m_numOfCurrentPartiesPerType[i]+1
		          << ", enhancedConfig.GetConfiguration(i)=" << m_enhancedConfig.GetConfiguration(i);

		// check if there's simply enough resources for the new party type and down
		if (m_numOfCurrentPartiesPerType[i]+1 < m_enhancedConfig.GetConfiguration(i))
		{
			newVideoPartyType = PartyResourceTypeToVideoPartyType((ePartyResourceTypes)i, systemCardsMode); // for dwngrd
			TRACEINTO << "CanBechanged:TRUE";
			return TRUE;
		}
	}

	TRACEINTO << "CanBechanged:FALSE";

	return FALSE;
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckIfOnePartyCanBechangedInIpService(eVideoPartyType oldVideoPartyType,
                                                                 ePartyRole oldPartyRole,
                                                                 eVideoPartyType& newVideoPartyType,
                                                                 ePartyRole& newPartyRole,
                                                                 float service_factor,
                                                                 BOOL round_up,
                                                                 BOOL* pbAddAudioAsVideo /*= NULL*/,
                                                                 eConfModeTypes confModeType /*= eNonMix*/)
{
	CEnhancedConfig ip_service_enhancedConfig = m_enhancedConfig;
	ip_service_enhancedConfig.SetIpServicePartConfig(service_factor, round_up);

	return CheckIfOnePartyCanBechanged(oldVideoPartyType, oldPartyRole, newVideoPartyType, newPartyRole, ip_service_enhancedConfig);
}

//--------------------------------------------------------------------------
void CFixedModeResources::CalculateResourceReport(CRsrcReport* pReport)
{
	CPrettyTable<const char*, DWORD, DWORD, DWORD> tbl("Type", "Total", "Occupied", "Free");

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
	{
		ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
		DWORD total = m_enhancedConfig.GetConfiguration(partyResourceType);
		pReport->SetNumParties(partyResourceType, TYPE_TOTAL, total);
		pReport->SetNumParties(partyResourceType, TYPE_OCCUPIED, m_numOfCurrentPartiesPerType[i]);
		pReport->SetNumParties(partyResourceType, TYPE_RESERVED, 0);
		pReport->SetNumParties(partyResourceType, TYPE_FREE, total-m_numOfCurrentPartiesPerType[i]);

		tbl.Add(to_string(partyResourceType), total, m_numOfCurrentPartiesPerType[i], total-m_numOfCurrentPartiesPerType[i]);
	}
	TRACEINTO << tbl.Get();
}

//--------------------------------------------------------------------------
void CFixedModeResources::CalculateConfResourceReport(CSharedRsrcConfReport* pReport)
{
	int numOfConf = pReport->GetNumConf();

	TRACEINTO << "NumberOfConferences:" << numOfConf;

	if (0 == numOfConf)
		return;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	DBGPASSERT_AND_RETURN(!pConfRsrcDB);

	const std::set<CConfRsrc>* pConfRsrcsList = pConfRsrcDB->GetConfRsrcsList();
	std::set<CConfRsrc>::iterator confItr;
	CConfRsrc* pConfRsrc = NULL;

	if (1 == numOfConf && (0xFFFFFFFF == pReport->GetConfIdByIndex(0)))    // if confID=-1 it means to collect info for all ongoing conferences
	{
		TRACEINTO << "Do for all ongoing conferences";

		pReport->Init(); // need to remove this id = 0xFFFFFFFF

		for (confItr = pConfRsrcsList->begin(); confItr != pConfRsrcsList->end(); confItr++)
		{
			pConfRsrc = (CConfRsrc*)&(*confItr);
			CountConfResourceReport(pReport, pConfRsrc);
		}
	}
	else
	{
		std::ostringstream msg;
		msg.setf(ios::left, ios::adjustfield);
		for (int i = 0; i < numOfConf; i++)
		{
			DWORD confID = pReport->GetConfIdByIndex(i);
			msg << confID << ", ";
			for (confItr = pConfRsrcsList->begin(); confItr != pConfRsrcsList->end(); confItr++)
			{
				pConfRsrc = (CConfRsrc*)&(*confItr);
				if (pConfRsrc->GetMonitorConfId() == confID)
					CountConfResourceReport(pReport, pConfRsrc);
			}
		}

		TRACEINTO << "ConferenceIDs:" << msg.str().c_str();
	}
}

//--------------------------------------------------------------------------
void CFixedModeResources::CountConfResourceReport(CSharedRsrcConfReport* pReport, CConfRsrc* pConfRsrc)
{
	const std::set<CPartyRsrc>* pPartyRsrcList = pConfRsrc ? pConfRsrc->GetPartiesList() : NULL;
	if (!pPartyRsrcList || !pReport)
		return;

	WORD numParties[NUM_OF_PARTY_RESOURCE_TYPES], i = 0;

	for (i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		numParties[i] = 0;

	std::set<CPartyRsrc>::iterator partyRsrcItr;

	for (partyRsrcItr = pPartyRsrcList->begin(); partyRsrcItr != pPartyRsrcList->end(); partyRsrcItr++)
	{
		CPartyRsrc* pPartyRsrc = ((CPartyRsrc*)(&(*partyRsrcItr)));
		ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(pPartyRsrc->GetVideoPartyType());
		numParties[partyResourceType]++;
	}

	for (i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		pReport->SetNumPartiesByConfID(pConfRsrc->GetMonitorConfId(), (ePartyResourceTypes)i, TYPE_OCCUPIED, numParties[i]);
}

//--------------------------------------------------------------------------
WORD CFixedModeResources::GetMaxNumberOfParties()
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, 0);

	// Tsahi - change value for MPM-Rx and for Mixed Mode
	WORD numPartiesPerHD720Port = (eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode()) ? 12 : 2;

	// 1 video HD720 licensing party = 12 audio in MPMx
	// 1 video HD720 licensing party = 2 CIF in MPM-Rx
	WORD maxParties = m_dongleNumParties * numPartiesPerHD720Port;
	TRACEINTO << "maxParties: " << maxParties;
	return maxParties; //maximum parties is when all are audio
}

//--------------------------------------------------------------------------
WORD CFixedModeResources::GetNumberOfRequiredUDPPortsPerBoard(eIceEnvironmentType isIceEnv) // ICE 4 ports
{
	// in pure barak mode, this number should be the worst case. This is when all units are Audio units
	// in this case we need: 16 x 2 x 32 (16 audio ports, 2 channels (in + out), on 32 units) = 1024
	// or MPMX we need:  9*4*40   (9 audio ports, 4 channels (in + out), on 40 units) =1440
	// 2048 covers all cases
	if (isIceEnv == eIceEnvironment_None || isIceEnv == eIceEnvironment_Standard)
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		if (pSystemResources && eProductTypeSoftMCUMfw == pSystemResources->GetProductType())
			return 10000;

		return 1024;
	}
	else
		return 2048;
}

//--------------------------------------------------------------------------
STATUS CFixedModeResources::IsRsrcEnough(CBoardsStatistics* pBoardsStatistics)
{
	UpdateInternalDataFromIsRsrcEnough(pBoardsStatistics);
	UpdateMaximumPartiesPerTypeAccordingToCards(pBoardsStatistics);

	if (CheckEnhancedConfigurationAccordingToEnabledUnits(&m_enhancedConfig) == FALSE)
	{
		TRACEINTO << "Failed";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CFixedModeResources::InitDongleRestriction(DWORD& num_parties, BOOL bRecalculateReservationPartyResources)
{
	BaseInitDongleRestriction(num_parties);

	m_dongleNumParties = num_parties;

	float floatMaxParties;

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		floatMaxParties = (((float)m_dongleNumParties) / m_LogicalHD720WeightAvcParty[eNonMix][i]);
		m_MaximumPartiesAccordingToDongle[i] = (DWORD)floatMaxParties;
	}

	PrintToTrace(m_MaximumPartiesAccordingToDongle, "MaximumPartiesPerTypeAccordingToDongle");
	PASSERTSTREAM(m_MaximumPartiesAccordingToDongle[e_HD720] != m_dongleNumParties, "MaximumPartiesPerTypeAccordingToDongle:" << m_MaximumPartiesAccordingToDongle[e_HD720] << ", dongleNumParties:" << m_dongleNumParties);

	m_enhancedConfig.ReadFromProcessSetting();

	BOOL anythingConfigured = FALSE;
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		if (m_enhancedConfig.GetConfiguration((ePartyResourceTypes)i) != 0)
		{
			anythingConfigured = TRUE;
			break;
		}
	}

	m_enhancedConfig.DumpToTrace();

	if (FALSE == anythingConfigured) // nothing configured
	{
		TRACEINTO << "No enhanced configuration, setting default";
		SetConfigurationAccordingToDongle();
	}

	if (CheckEnhancedConfigurationAccordingToDongle(&m_enhancedConfig) == FALSE)
	{
		PASSERTMSG(1, "Enhanced configuration is more than dongle, setting default");
		SetConfigurationAccordingToDongle();
		m_enhancedConfig.DumpToTrace();
	}

	InitReservatorAndStaticCardConfigParams(bRecalculateReservationPartyResources);
}

//--------------------------------------------------------------------------
void CFixedModeResources::InitReservatorAndStaticCardConfigParams(BOOL bRecalculateReservationPartyResources)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(pSystemResources == NULL);

	eSystemCardsMode systemCardsMode = pSystemResources->GetSystemCardsMode();

	// PTRACE(eLevelInfoNormal,"RSRV_LOG: CFixedModeResources::InitReservatorAndStaticCardConfigParams");
	UpdateReservatorWithLogicalConfiguration(bRecalculateReservationPartyResources);

	DWORD totalNumberOfHD720Parties = 0, totalNumberOfVideoParties = 0;
	eVideoPartyType videoPartyType;
	ePartyResourceTypes partyResourceType;
	for (int i = 1; i < NUM_OF_PARTY_RESOURCE_TYPES; i++) // i starts from 1 because we don't want to count audio parties
	{
		partyResourceType = (ePartyResourceTypes)i;
		videoPartyType = PartyResourceTypeToVideoPartyType(partyResourceType, systemCardsMode);
		totalNumberOfHD720Parties += (DWORD)ceil(m_enhancedConfig.GetConfiguration(partyResourceType) * CHelperFuncs::GetNumPhysicalHD720Ports(videoPartyType));
		totalNumberOfVideoParties += m_enhancedConfig.GetConfiguration(partyResourceType);
	}

	CCardResourceConfig::SetNumAudPortsLeftToConfig(m_enhancedConfig.GetConfiguration(e_Audio) + totalNumberOfVideoParties - pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS));
	CCardResourceConfig::SetNumVidHD720PortsLeftToConfig(totalNumberOfHD720Parties - pSystemResources->GetNumConfiguredVideoPorts(ALL_BOARD_IDS));

	float num_Needed_ART_prml = (m_enhancedConfig.GetConfiguration(e_Audio) +  totalNumberOfVideoParties) * m_ART_PROMILLES;
	float num_Needed_VID_prml = (totalNumberOfHD720Parties) * m_VID_TOTAL_HD720_PROMILLES;

	float configProportion = (num_Needed_ART_prml == 0) ? 0xFFFF : (((float)num_Needed_VID_prml) / num_Needed_ART_prml);
	TRACEINTO << "\n  AudioPartiesLeft    :" << m_enhancedConfig.GetConfiguration(e_Audio)
	          << "\n  VideoPartiesLeft    :" << totalNumberOfHD720Parties
	          << "\n  Needed_ART_Promilles:" << num_Needed_ART_prml
	          << "\n  Needed_VID_Promilles:" << num_Needed_VID_prml
	          << "\n  ConfigProportion    :" << configProportion;

	CCardResourceConfigBreeze::SetConfigProportion(configProportion);
}

//--------------------------------------------------------------------------
STATUS CFixedModeResources::UpdateMaximumPartiesPerTypeAccordingToCards(CBoardsStatistics* pBoardsStatistics)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	float hd720_ports = (pSystemResources) ? pSystemResources->GetHD720PortsAccordingToCards() : 0.f;
	TRACEINTO << "pSystemResources:" << pSystemResources << ", hd720_ports:" << hd720_ports;

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		m_MaximumPartiesAccordingToCards[i] = (WORD)(hd720_ports/m_LogicalHD720WeightAvcParty[eNonMix][i]);
/*
	DWORD approximatedNumberOfParties;
	DWORD numOfArtUnitsNeeded = 0, numOfVideoUnitsNeeded = 0;

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		m_MaximumPartiesPerTypeAccordingToCards[i] = 0;

		for (int boardId = 0; boardId < BOARDS_NUM; boardId++)
		{
			if (m_PromillesPerCard[boardId] == 0)
				continue;

			approximatedNumberOfParties = (DWORD)floor(m_PromillesPerCard[boardId]/m_AudioVideoPhysicalPromillesOfPartyPerType[i]);

			// now we have to check if this indeed can enter on units
			for (; approximatedNumberOfParties > 0; approximatedNumberOfParties--)
			{
				numOfArtUnitsNeeded   = (DWORD)ceil((float)(approximatedNumberOfParties * m_ART_PROMILLES) /1000);
				numOfVideoUnitsNeeded = (DWORD)ceil((float)(approximatedNumberOfParties * m_VideoPhysicalPromillesOfPartyPerType[i]) / 1000);

				if (numOfArtUnitsNeeded + numOfVideoUnitsNeeded <= pBoardsStatistics->m_NumOfUnits[boardId]) // found appropriate number of parties, for all units
				{
					m_MaximumPartiesPerTypeAccordingToCards[i] += approximatedNumberOfParties;
					break;
				}
			}
		}
	}
*/
	PrintToTrace(m_MaximumPartiesAccordingToCards, "MaximumPartiesPerTypeAccordingToCards");
	return STATUS_OK;
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckEnhancedConfigurationAccordingToDongleAndCards(const CEnhancedConfig* pEnhancedConfig) const
{
	if (CheckEnhancedConfigurationAccordingToDongle(pEnhancedConfig) == FALSE)
		return FALSE;

	if (CheckEnhancedConfigurationAccordingToCardsPerLicense(pEnhancedConfig) == FALSE)
		return FALSE;

	return TRUE;
	//return CheckEnhancedConfigurationAccordingToEnabledUnits(pEnhancedConfig);
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckEnhancedConfigurationAccordingToDongle(const CEnhancedConfig* pEnhancedConfig) const
{
	float total = 0;

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		DWORD numOfParties = pEnhancedConfig->GetConfiguration(i);
		if (numOfParties > m_MaximumPartiesAccordingToDongle[i])
		{
			ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
			TRACEINTO << "PartyResourceType:" << partyResourceType << ", numOfParties:" << numOfParties << ", numOfPartiesAccordingToDongle:" << m_MaximumPartiesAccordingToDongle[i] << " - Failed";
			return FALSE;
		}

		total += m_LogicalHD720WeightAvcParty[eNonMix][i] * numOfParties;
	}

	if (total > m_dongleNumParties)
	{
		TRACEINTO << "numOfParties:" << total << ", numOfPartiesAccordingToDongle:" << m_dongleNumParties << " - Failed";
		return FALSE;
	}

	TRACEINTO << "numOfParties:" << total << ", numOfPartiesAccordingToDongle:" << m_dongleNumParties << " - OK";
	((CEnhancedConfig*)pEnhancedConfig)->DumpToTrace();
	return TRUE;
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckEnhancedConfigurationAccordingToCardsPerLicense(const CEnhancedConfig* pEnhancedConfig) const
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	WORD hd720_ports = (pSystemResources) ? pSystemResources->GetHD720PortsAccordingToCards() : 0;

	float total = 0;

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		DWORD numOfParties = pEnhancedConfig->GetConfiguration(i);
		total += m_LogicalHD720WeightAvcParty[eNonMix][i] * numOfParties;
	}

	if (total > hd720_ports)
	{
		TRACEWARN << "numOfParties:" << total << ", numOfPartiesAccordingToCards:" << hd720_ports << " - Failed";
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckEnhancedConfigurationAccordingToEnabledUnits(const CEnhancedConfig* pEnhancedConfig) const
{
	float totalVideoPromilles = 0;
	float totalAudioPromilles = 0;
	CalculateNeededAudioAndVideoPromilles(pEnhancedConfig, totalAudioPromilles, totalVideoPromilles);

	DWORD totalNeededAudioUnits = (DWORD)ceil((float)(totalAudioPromilles/1000));
	DWORD totalNeededVideoUnits = (DWORD)ceil((float)(totalVideoPromilles/1000));

	if (totalNeededAudioUnits > m_BoardsStatistics.GetTotalNumOfEnabledArtUnits())
	{
		TRACEWARN << "NeededAudioUnits:" << totalNeededAudioUnits << ", EnabledAudioUnits:" << m_BoardsStatistics.GetTotalNumOfEnabledArtUnits() << " - Failed";
		return FALSE;
	}

	if (totalNeededVideoUnits > m_BoardsStatistics.GetTotalNumOfEnabledVideoUnits())
	{
		TRACEWARN << "NeededVideoUnits:" << totalNeededVideoUnits << ", EnabledVideoUnits:" << m_BoardsStatistics.GetTotalNumOfEnabledVideoUnits() << " - Failed";
		return FALSE;
	}

	TRACEINTO << "NeededAudioUnits:" << totalNeededAudioUnits << ", NeededVideoUnits:" << totalNeededVideoUnits;

	return CheckVideoUnitsWithEnhancedConfig(m_BoardsStatistics.m_NumOfEnabledUnits, pEnhancedConfig, TRUE);
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckEnhancedConfigurationWithCurUnitsConfig()
{
	float totalVideoPromilles = 0;
	float totalAudioPromilles = 0;
	CalculateNeededAudioAndVideoPromilles(&m_enhancedConfig, totalAudioPromilles, totalVideoPromilles);

	DWORD totalNeededART   = (DWORD)ceil((float)(totalAudioPromilles/1000));
	DWORD totalNeededVideo = (DWORD)ceil((float)(totalVideoPromilles/1000));

	DWORD numOfEnabledArtUnits = 0;
	DWORD numOfEnabledVidUnits = 0;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		for (WORD boardId = 1; boardId <= BOARDS_NUM; boardId++)
		{
			CBoard* pBoard = pSystemResources->GetBoard(boardId);
			if (!pBoard)
			continue;

			if (!pBoard->GetCardsStartup(MFA_COMPLETE))
				continue;
		
			CMediaUnitsList* pMediaUnitslist = pBoard->GetMediaUnitsList();
			if (!pMediaUnitslist)
				continue;

			CMediaUnitsList::iterator _itr, _end = pMediaUnitslist->end();
			for (_itr = pMediaUnitslist->begin(); _itr != _end; ++_itr)
			{
				if (_itr->GetIsEnabled() != TRUE)
				continue;

				if (eUnitType_Art == _itr->GetUnitType())
				numOfEnabledArtUnits++;

				if (eUnitType_Video == _itr->GetUnitType())
				numOfEnabledVidUnits++;
			}
		}
	}

	TRACEINTO << "totalNeededART:"         << totalNeededART
	          << ", totalNeededVideo:"     << totalNeededVideo
	          << ", numOfEnabledArtUnits:" << numOfEnabledArtUnits
	          << ", numOfEnabledVidUnits:" << numOfEnabledVidUnits;

	if (totalNeededART > numOfEnabledArtUnits || totalNeededVideo > numOfEnabledVidUnits)
		return FALSE;

	return TRUE;
}

//--------------------------------------------------------------------------
void CFixedModeResources::CalculateNeededAudioAndVideoPromilles(const CEnhancedConfig* pEnhancedConfig, float& totalAudioPromilles, float& totalVideoPromilles) const
{
	totalVideoPromilles = 0;
	totalAudioPromilles = 0;
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		totalVideoPromilles += ((float)pEnhancedConfig->GetConfiguration(i)) * m_VideoPromillesOfParty[i];
		totalAudioPromilles += ((float)pEnhancedConfig->GetConfiguration(i)) * m_ART_PROMILLES;
	}
}

//--------------------------------------------------------------------------
STATUS CFixedModeResources::GetOrCheckEnhancedConfiguration(const CEnhancedConfig* pEnhancedCfg, CEnhancedConfigResponse* pResponse) const
{
	STATUS status = STATUS_OK;
	if (pEnhancedCfg == NULL) // meaning that we are in "get"
	{
		pEnhancedCfg = &m_enhancedConfig;
		if (CheckEnhancedConfigurationAccordingToDongleAndCards(pEnhancedCfg) == FALSE)
		{
			TRACEINTO << "Status:STATUS_CURRENT_SLIDER_SETTINGS_REQUIRES_MORE_RESOURCES_THAN_AVAILBLE - Failed";
			status = STATUS_CURRENT_SLIDER_SETTINGS_REQUIRES_MORE_RESOURCES_THAN_AVAILBLE | WARNING_MASK;
		}
	}
	else
	{
		if (CheckEnhancedConfigurationAccordingToDongleAndCards(pEnhancedCfg) == FALSE)
		{
			TRACEINTO << "Status:STATUS_ILLEGAL_PORTS_CONFIGURATION - Failed";
			return STATUS_ILLEGAL_PORTS_CONFIGURATION;
		}
	}

	float totalVideoPromilles = 0;
	float totalAudioPromilles = 0;
	CalculateNeededAudioAndVideoPromilles(pEnhancedCfg, totalAudioPromilles, totalVideoPromilles);

	float remainingPromilles  = m_totalPromillesAccordingToCards - (totalVideoPromilles + totalAudioPromilles);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "CFixedModeResources::GetOrCheckEnhancedConfiguration - Failed, pSystemResources is NULL", STATUS_FAIL);

	WORD hd720_ports = pSystemResources->GetHD720PortsAccordingToCards();

	eSystemCardsMode systemCardsMode = pSystemResources->GetSystemCardsMode();

	CPrettyTable<const char*, WORD, WORD, WORD, float, float, float, float, float> tbl("Type", "Parties", "MaxParties (Cards)", "MaxParties (Dongle)", "LogicalWeight", "NumPorts", "AudioPromilles", "VideoPromilles", "TotalPromilles");

	for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
	{
		ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
		tbl.Add(to_string(partyResourceType),
		        pEnhancedCfg->GetConfiguration(i),
		        m_MaximumPartiesAccordingToCards[i],
		        m_MaximumPartiesAccordingToDongle[i],
		        m_LogicalHD720WeightAvcParty[eNonMix][i],
		        CHelperFuncs::GetNumPhysicalHD720Ports(PartyResourceTypeToVideoPartyType(partyResourceType, systemCardsMode)),
		        m_ART_PROMILLES,
		        m_VideoPromillesOfParty[i],
		        m_TotalPromillesOfParty[i]);
	}

	TRACEINTO << "\n  PortsNumberAccordingToLicense  :" << m_dongleNumParties
	          << "\n  PortsNumberAccordingToCards    :" << hd720_ports
	          << "\n  TotalPromillesAccordingToCards :" << m_totalPromillesAccordingToCards
	          << "\n  TotalVideoPromilles            :" << totalVideoPromilles
	          << "\n  TotalAudioPromilles            :" << totalAudioPromilles
	          << "\n  RemainingPromilles             :" << remainingPromilles
	          << tbl.Get();

	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		ePartyResourceTypes partyResourceTypeIndex = ePartyResourceTypes(i);
		CEnhancedConfigResponseItem* pEnhancedConfigResponseItem = pResponse->GetConfigItem(partyResourceTypeIndex);

		pEnhancedConfigResponseItem->SetCurrent(pEnhancedCfg->GetConfiguration(partyResourceTypeIndex));

		pEnhancedConfigResponseItem->SetSystemMaximum(min(m_MaximumPartiesAccordingToDongle[i], m_MaximumPartiesAccordingToCards[i]));

		if (remainingPromilles <= 0)                                                                      // if there's no place on the boards
		{
			pEnhancedConfigResponseItem->SetOptionalMaximum(0);
		}
		else if (pEnhancedConfigResponseItem->GetCurrent() == m_MaximumPartiesAccordingToDongle[i]) // we got to maximum of dongle
		{
			pEnhancedConfigResponseItem->SetOptionalMaximum(pEnhancedConfigResponseItem->GetCurrent());
		}
		else if (pEnhancedConfigResponseItem->GetCurrent() == m_MaximumPartiesAccordingToCards[i]) // we got to maximum of cards
		{
			pEnhancedConfigResponseItem->SetOptionalMaximum(pEnhancedConfigResponseItem->GetCurrent());
		}
		else
		{
			WORD maximumPartiesPerType = CalculateMaximumPartiesPerType(pEnhancedCfg, partyResourceTypeIndex);
			pEnhancedConfigResponseItem->SetOptionalMaximum(maximumPartiesPerType);
		}
	}

	pResponse->DumpToTrace();

	return status;
}

//--------------------------------------------------------------------------
WORD CFixedModeResources::CalculateMaximumPartiesPerType(const CEnhancedConfig* pEnhancedConfig, ePartyResourceTypes type) const
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	DWORD hd720PortsAccordingToCards = (pSystemResources) ? pSystemResources->GetHD720PortsAccordingToCards() : 0;

	float maxHD720Ports = min(m_dongleNumParties, hd720PortsAccordingToCards);

	// Calculate total logical weight for all party types excluding specified
	float totalLogicalWeight = 0;
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		if (i != type)
		{
			DWORD numOfParties = pEnhancedConfig->GetConfiguration(i);
			totalLogicalWeight += m_LogicalHD720WeightAvcParty[eNonMix][i] * numOfParties;
		}
	}
	// Calculate remaining logical weight for calculated type
	float remainingLogicalWeight = maxHD720Ports - totalLogicalWeight;

	// Calculate maximum parties per type according remaining logical weight
	WORD maxPartiesPerType = (WORD)(floor(remainingLogicalWeight/m_LogicalHD720WeightAvcParty[eNonMix][type]));

	// Maximum parties per type according should not exceed license
	if ( e_Audio <= type && NUM_OF_PARTY_RESOURCE_TYPES > type ) // to please klocwork ID : 4761 & 4762
	{
		maxPartiesPerType = min(maxPartiesPerType, m_MaximumPartiesAccordingToDongle[type]);
		maxPartiesPerType = min(maxPartiesPerType, m_MaximumPartiesAccordingToCards[type]);
	}

	return maxPartiesPerType;
}

//--------------------------------------------------------------------------
STATUS CFixedModeResources::CanSetConfigurationNow()
{
	return BaseCanSetConfigurationNow();
}

//--------------------------------------------------------------------------
STATUS CFixedModeResources::SetEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg, BOOL bIsIpService, DWORD ip_service_id)
{
	STATUS status = CanSetConfigurationNow();
	if (status != STATUS_OK)
		return status;

	// check that it's valid according to cards and dongle
	if (CheckEnhancedConfigurationAccordingToDongleAndCards(pEnhancedCfg) == FALSE)
		return STATUS_ILLEGAL_PORTS_CONFIGURATION;

	// set + write to file
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
	{
		m_enhancedConfig.SetConfiguration(i, pEnhancedCfg->GetConfiguration(i));
	}

	m_enhancedConfig.WriteToProcessSetting();

	InitReservatorAndStaticCardConfigParams();

	// check if now resources are enough
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();
	PASSERT_AND_RETURN_VALUE(pResourceManager == NULL, STATUS_FAIL);
	pResourceManager->CheckResourceEnoughAndAddOrRemoveAciveAlarm(TRUE);

	// send to function in base
	PureModeReconfigureUnitsAccordingToProportion();

	// check if one more audio party can be added, this means that we are not fully using the resources...
	CEnhancedConfig configWithOneMoreAudioParty;
	configWithOneMoreAudioParty = *pEnhancedCfg;
	configWithOneMoreAudioParty.SetConfiguration(e_Audio, pEnhancedCfg->GetConfiguration(e_Audio) + 1);
	if (CheckEnhancedConfigurationAccordingToDongleAndCards(&configWithOneMoreAudioParty) == TRUE)
	{
		return STATUS_PORTS_CONFIGURATION_DOES_NOT_USE_FULL_SYSTEM_CAPACITY | WARNING_MASK;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CFixedModeResources::FineTuneUnitsConfiguration()
{
	DWORD numVideoUnitsPerBoard[BOARDS_NUM];
	DWORD numArtUnitsPerBoard[BOARDS_NUM];

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(pSystemResources == NULL);

	for (int i = 0; i < BOARDS_NUM; i++)
	{
		numVideoUnitsPerBoard[i] = 0;
		numArtUnitsPerBoard[i]   = 0;

		int oneBasedBoardId = i + 1;
		if (!pSystemResources->IsBoardIdExists(oneBasedBoardId))
			continue;

		CBoard* pBoard = pSystemResources->GetBoard(oneBasedBoardId);
		if (pBoard == NULL)
			continue;

		pBoard->CountUnits(numArtUnitsPerBoard[i], numVideoUnitsPerBoard[i], TRUE);
	}

	if (CheckVideoUnitsWithEnhancedConfig(numVideoUnitsPerBoard) == FALSE)
	{
		// this means we have to find a board which has at least two ART units, and change it to a video unit
		for (int bid_to_remove_art = 0; bid_to_remove_art < BOARDS_NUM; bid_to_remove_art++)
		{
			if (numArtUnitsPerBoard[bid_to_remove_art] >= 2)
			{
				// now try to find a board where art can be added: this is a board that exists (it has art units, and that is different than the one to remove from)
				for (int bid_to_add_art = 0; bid_to_add_art < BOARDS_NUM; bid_to_add_art++)
				{
					if (numArtUnitsPerBoard[bid_to_add_art] > 0 && bid_to_add_art != bid_to_remove_art)
					{
						// try and see if now it's OK
						numVideoUnitsPerBoard[bid_to_remove_art]++;
						numVideoUnitsPerBoard[bid_to_add_art]--;
						if (CheckVideoUnitsWithEnhancedConfig(numVideoUnitsPerBoard) == TRUE)
						{
							// we found a better way to configure the units
							TRACEINTO << "Removing one ART unit from board " << bid_to_remove_art + 1
							          << " and adding one ART unit to board " << bid_to_add_art + 1;

							// add one video unit to bid_to_remove_art
							int oneBasedBoardId = bid_to_remove_art + 1;
							CBoard* pBoard = pSystemResources->GetBoard(oneBasedBoardId);
							PASSERT_AND_RETURN(pBoard == NULL);
							pBoard->ChangeUnits(1, eUnitType_Video);

							// add one ART unit to bid_to_add_art
							oneBasedBoardId = bid_to_add_art + 1;
							pBoard = pSystemResources->GetBoard(oneBasedBoardId);
							PASSERT_AND_RETURN(pBoard == NULL);
							pBoard->ChangeUnits(1, eUnitType_Art);
							return;
						}
						else
						{
							// it's not the correct one, continue (but first change the numbers back to their original ones)
							numVideoUnitsPerBoard[bid_to_remove_art]--;
							numVideoUnitsPerBoard[bid_to_add_art]++;
						}
					}
				}
			}
		}
		TRACEINTO << "The configuration isn't OK, but didn't found any other better configuration";
	}
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckVideoUnitsWithEnhancedConfig(const DWORD numUnitsPerBoard[BOARDS_NUM], const CEnhancedConfig* pEnhancedConfig, BOOL removeAudioControllerUnit) const
{
	// check parties that take more than one DSP
	if (pEnhancedConfig == NULL)
		pEnhancedConfig = &m_enhancedConfig;

	float numFloatVideoUnitsPerBoard[BOARDS_NUM];
	for (int i = 0; i < BOARDS_NUM; i++)
	{
		if (removeAudioControllerUnit == FALSE)
		{
			numFloatVideoUnitsPerBoard[i] = numUnitsPerBoard[i];
		}
		else
		{
			if (numUnitsPerBoard[i] > 0)
				numFloatVideoUnitsPerBoard[i] = numUnitsPerBoard[i]-1; // remove 1 for audio controller unit
			else
				numFloatVideoUnitsPerBoard[i] = 0;
		}
	}

	for (int i = NUM_OF_PARTY_RESOURCE_TYPES-1; i >= 0; i--)
	{
		int   requiredNumOfParties = pEnhancedConfig->GetConfiguration(i);
		float numVideoUnitsForType = m_VideoPromillesOfParty[i] / 1000;

		// only check parties that are more than one unit
		if (numVideoUnitsForType <= 1)
			continue;

		BOOL bfound = FALSE;
		for (int j = 0; j < requiredNumOfParties; j++)
		{
			bfound = FALSE;
			for (int board = 0; board < BOARDS_NUM; board++)
			{
				if (numFloatVideoUnitsPerBoard[board] >= numVideoUnitsForType)
				{
					numFloatVideoUnitsPerBoard[board] -= numVideoUnitsForType;
					bfound = TRUE;
					break;
				}
			}

			if (bfound == FALSE)
				return FALSE;
		}
	}

	return TRUE;
}

//--------------------------------------------------------------------------
void CFixedModeResources::SetConfigurationAccordingToDongle()
{
	// PTRACE(eLevelInfoNormal,"RSRV_LOG: CFixedModeResources::SetConfigurationAccordingToDongle");
	// set defaults, all HD720
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		m_enhancedConfig.SetConfiguration(i, 0);

	m_enhancedConfig.SetConfiguration(e_HD720, (int)floor(m_dongleNumParties/m_LogicalHD720WeightAvcParty[eNonMix][e_HD720]));

	UpdateReservatorWithLogicalConfiguration();

	// save it
	m_enhancedConfig.WriteToProcessSetting();
}

//--------------------------------------------------------------------------
void CFixedModeResources::FillRequiredPartyResourcesForConference(WORD minNumAudioPartiesInConf, WORD minNumVideoPartiesInConf, eVideoPartyType maxVideoPartyTypeInConf, CPartiesResources& partiesResourcesToFill)
{
	partiesResourcesToFill.m_physical_audio_ports = minNumAudioPartiesInConf + minNumVideoPartiesInConf;
	partiesResourcesToFill.m_physical_hd720_ports = (WORD)ceil(minNumVideoPartiesInConf * CHelperFuncs::GetNumPhysicalHD720Ports(maxVideoPartyTypeInConf));
	partiesResourcesToFill.m_logical_num_parties[e_Audio] = minNumAudioPartiesInConf;

	// D.K. VNGFE-5706
	std::string downgrade_reason;
	if (maxVideoPartyTypeInConf == eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type && m_enhancedConfig.GetConfiguration(e_HD720) == 0)
	{
		// The maxVideoPartyTypeInConf involves the possibility of using e_HD720 resolution, but Video/Voice Port Configuration does not allow e_HD720
		// So we downgrade the maxVideoPartyTypeInConf to use e_SD30 only
		downgrade_reason = " (e_HD720 is 0, so downgrade to eCP_H264_upto_SD30_video_party_type)";
		maxVideoPartyTypeInConf = eCP_H264_upto_SD30_video_party_type;
	}

	if (maxVideoPartyTypeInConf == eCP_H261_H263_upto_CIF_video_party_type && m_enhancedConfig.GetConfiguration(e_SD30) == 0)
	{
		// The maxVideoPartyTypeInConf involves the possibility of using e_SD30 resolution, but Video/Voice Port Configuration does not allow e_SD30
		// So we downgrade the maxVideoPartyTypeInConf to use e_Cif only
		downgrade_reason = " (e_SD30 is 0, so downgrade to eCP_H264_upto_CIF_video_party_type)";
		maxVideoPartyTypeInConf = eCP_H264_upto_CIF_video_party_type;
	}

	if (!downgrade_reason.empty())
	{
		TRACEINTO << "\n  minNumAudioPartiesInConf      :" << minNumAudioPartiesInConf
		          << "\n  minNumVideoPartiesInConf      :" << minNumVideoPartiesInConf
		          << "\n  maxVideoPartyTypeInConf       :" << eVideoPartyTypeNames[maxVideoPartyTypeInConf] << downgrade_reason
		          << "\n  VideoVoiceConfig[e_Audio]     :" << m_enhancedConfig.GetConfiguration(e_Audio)
		          << "\n  VideoVoiceConfig[e_Cif]       :" << m_enhancedConfig.GetConfiguration(e_Cif)
		          << "\n  VideoVoiceConfig[e_SD30]      :" << m_enhancedConfig.GetConfiguration(e_SD30)
		          << "\n  VideoVoiceConfig[e_HD720]     :" << m_enhancedConfig.GetConfiguration(e_HD720)
		          << "\n  VideoVoiceConfig[e_HD1080p30] :" << m_enhancedConfig.GetConfiguration(e_HD1080p30);
	}

	ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(maxVideoPartyTypeInConf);
	if (partyResourceType != e_Audio)
		partiesResourcesToFill.m_logical_num_parties[partyResourceType] = minNumVideoPartiesInConf;
	else
		PASSERTMSG(minNumVideoPartiesInConf != 0, "Video participants does not allowed in audio only conference");
}

//--------------------------------------------------------------------------
void CFixedModeResources::UpdateReservatorWithLogicalConfiguration(BOOL bRecalculateReservationPartyResources)
{
	TRACEINTO << "RecalculateReservationPartyResources:" << (int)bRecalculateReservationPartyResources;

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN(!pReservator);

	WORD logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES];
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		logical_num_parties[i] = m_enhancedConfig.GetConfiguration(i);

	pReservator->SetLogicalResources(logical_num_parties, bRecalculateReservationPartyResources);
}

//--------------------------------------------------------------------------
BOOL CFixedModeResources::CheckVideoUnitsWithConfig(const DWORD numUnitsPerBoard[BOARDS_NUM]) const
{
	return CheckVideoUnitsWithEnhancedConfig(numUnitsPerBoard);
}

//--------------------------------------------------------------------------
STATUS CFixedModeResources::ReconfigureUnitsAccordingToProportion(BOOL bForceReconfigure)
{
	STATUS sts = BaseCanSetConfigurationNow();
	if (sts == STATUS_OK)
	{
		PureModeReconfigureUnitsAccordingToProportion();
		return STATUS_OK;
	}
	else
	{
		return sts;
	}
}

//--------------------------------------------------------------------------
STATUS CFixedModeResources::CheckSetEnhancedConfiguration(BOOL bIsIpService, DWORD ip_service_id)
{
	STATUS ret_val = STATUS_OK;
	std::ostringstream msg;

	if (CheckEnhancedConfigurationAccordingToDongleAndCards(&m_enhancedConfig) == FALSE)
	{
		WORD numBoards = 0;
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		TRACECOND_AND_RETURN_VALUE(!pSystemResources, "pSystemResources is NULL", STATUS_FAIL);

		WORD  hd720_ports = pSystemResources->GetHD720PortsAccordingToCards(&numBoards);

		float total = 0, newTotal = 0;
		DWORD numOfParties;
		for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		{
			numOfParties = m_enhancedConfig.GetConfiguration(i);
			total += m_LogicalHD720WeightAvcParty[eNonMix][i] * numOfParties;
		}

		if ((total > hd720_ports) && (hd720_ports != 0))
		{
			WORD numBoardsWithLicense = (m_dongleNumParties * numBoards) / hd720_ports;
			msg << "numBoardsWithLicense:" << numBoardsWithLicense << ", numBoards:" << numBoards;
			for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
			{
				numOfParties = m_enhancedConfig.GetConfiguration(i);
				if (numOfParties)
				{
					WORD newNumOfParties = (numOfParties*numBoards)/ numBoardsWithLicense; // it is called after board removing
					m_enhancedConfig.SetConfiguration(i, newNumOfParties);
					newTotal += m_LogicalHD720WeightAvcParty[eNonMix][i] * newNumOfParties;
				}
			}

			if (newTotal < hd720_ports) // add leftover to cif-ports
			{
				WORD cif_config = m_enhancedConfig.GetConfiguration(e_Cif);
				WORD diff = (WORD)(m_LogicalHD720WeightAvcParty[eNonMix][e_Cif] * (hd720_ports - newTotal));
				m_enhancedConfig.SetConfiguration(e_Cif, cif_config+diff);
			}
		}
		else if (0 == hd720_ports)
		{
			for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
				m_enhancedConfig.SetConfiguration(i, 0);
		}

		ret_val = SetEnhancedConfiguration(&m_enhancedConfig, bIsIpService, ip_service_id);
	}
	TRACEINTO << msg.str().c_str() << ", status:" << ret_val;

	return ret_val;
}

void  CFixedModeResources::OnLicenseExpired()
{
	TRACEINTO << "Fixed mode is blocked since ";
}
