#include "TraceStream.h"
#include "AutoModeResources.h"
#include "HelperFuncs.h"
#include "ResourceManager.h"
#include "ProcessSettings.h"
#include "CardResourceConfig.h"
#include "CRsrcDetailGet.h"
#include "InternalProcessStatuses.h"
#include "SharedRsrcList.h"
#include "ResRsrcCalculator.h"
#include <memory>
#include <numeric>

#define MEDIA_RELAY_PORTS_RATIO 3  //from SRS: Media-Relay in RMX does not require licensing. The maximum number of Media Relay ports is (3 * HD720p30 ports)

#undef TRACEINTO
#define TRACEINTO TRACESTRFUNC(eLevelInfoHigh)
//////////////////////////////////////////////////////////////////////////////////
CAutoModeResources::CAutoModeResources(eSystemCardsMode cardMode) : CBaseModeResources(cardMode)
{
	m_dongleNumParties = 0;
	memset(m_numCurrentAVCParties, 0, sizeof(m_numCurrentAVCParties));
	memset(m_numCurrentSVCParties, 0, sizeof(m_numCurrentSVCParties));
	memset(m_numAudioPartiesAllocatedAsVideo, 0, sizeof(m_numAudioPartiesAllocatedAsVideo));
	m_pPortsConfig = new CPortsConfig;

	memset(m_numDonglePartiesSVC, 0, NUM_OF_CONF_MODE_TYPES * NUM_OF_PARTY_RESOURCE_TYPES);
}
//////////////////////////////////////////////////////////////////////////////////
CAutoModeResources::CAutoModeResources(const CAutoModeResources& other) : CPObject(other), ResourcesInterface(other), CBaseModeResources(other)
{
	m_pPortsConfig = NULL;
	*this = other;
}
//////////////////////////////////////////////////////////////////////////////////
const CAutoModeResources& CAutoModeResources::operator=(const CAutoModeResources& other)
{
	if (this != &other)
	{
		memcpy(m_numCurrentAVCParties, other.m_numCurrentAVCParties, sizeof(m_numCurrentAVCParties));
		memcpy(m_numCurrentSVCParties, other.m_numCurrentSVCParties, sizeof(m_numCurrentSVCParties));
		memcpy(m_numAudioPartiesAllocatedAsVideo, other.m_numAudioPartiesAllocatedAsVideo, sizeof(m_numAudioPartiesAllocatedAsVideo));
		m_audVidConfig = other.m_audVidConfig;
		POBJDELETE(m_pPortsConfig);
		if (other.m_pPortsConfig != NULL)
			m_pPortsConfig = new CPortsConfig(*(other.m_pPortsConfig));

		memcpy(m_numDonglePartiesSVC, other.m_numDonglePartiesSVC, NUM_OF_CONF_MODE_TYPES * NUM_OF_PARTY_RESOURCE_TYPES);
	}
	return *this;
}
//////////////////////////////////////////////////////////////////////////////////
CAutoModeResources::~CAutoModeResources()
{
	POBJDELETE(m_pPortsConfig);
}

//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::CalculateResourceReport(CRsrcReport* pReport)
{
	WORD numAudParties = 0, numVidCifParties = 0, numVidHD720Parties = 0;
	DWORD ppm = 0;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	WORD hd720_ports_according_to_cards = pSystemResources->GetHD720PortsAccordingToCards();

	for (WORD i = 0; i < NUM_RPRT_TYPES; i++)
	{
		CountReportPorts((eRPRTtypes)(i), hd720_ports_according_to_cards, numVidHD720Parties, numVidCifParties, numAudParties, ppm); //numParties filled by ref.
		pReport->SetNumParties(e_Audio, (eRPRTtypes)(i), numAudParties);
		pReport->SetNumParties(e_Cif, (eRPRTtypes)(i), numVidCifParties);
		pReport->SetNumParties(e_HD720, (eRPRTtypes)(i), numVidHD720Parties);
		if (TYPE_FREE == i)
			pReport->SetAvailablePortionPPM(ppm);
	}
	static int _reportCounter = 0;
	_reportCounter++;
	if (_reportCounter%20)
		TRACEINTO << *pReport;
}

//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::CalculateConfResourceReport(CSharedRsrcConfReport* pReport)
{
	int numOfConf = pReport->GetNumConf();
	TRACEINTO << "num of conferences = " << numOfConf;
	if (0 == numOfConf)
		return;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (!pConfRsrcDB)
	{
		PTRACE(eLevelError, "CAutoModeResources::CalculateConfResourceReport - CHelperFuncs::GetConfRsrcDB returns NULL!!");
		return;
	}
	const std::set<CConfRsrc>* pConfRsrcsList = pConfRsrcDB->GetConfRsrcsList();
	std::set<CConfRsrc>::iterator confItr;
	CConfRsrc* pConfRsrc = NULL;

	if (1 == numOfConf && (0xFFFFFFFF == pReport->GetConfIdByIndex(0))) //if -1 then collect info for all ongoing conferences
	{
		TRACEINTO << "do for all ongoing conferences ";

		pReport->Init(); //need to remove this id = 0xFFFFFFFF

		for (confItr = pConfRsrcsList->begin(); confItr != pConfRsrcsList->end(); confItr++)
		{
			pConfRsrc = (CConfRsrc*)&(*confItr);
			CountConfResourceReport(pReport, pConfRsrc);
		}
	}
	else
	{
		std::ostringstream msgString;
		msgString.setf(ios::left, ios::adjustfield);
		for (int i = 0; i < numOfConf; i++)
		{
			DWORD confID = pReport->GetConfIdByIndex(i);
			msgString << confID << ", ";
			for (confItr = pConfRsrcsList->begin(); confItr != pConfRsrcsList->end(); confItr++)
			{
				pConfRsrc = (CConfRsrc*)&(*confItr);
				if (pConfRsrc->GetMonitorConfId() == confID)
					CountConfResourceReport(pReport, pConfRsrc);
			}
		}
		TRACEINTO << "list of conference IDs = " << msgString.str().c_str();
	}
}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType,
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
	WORD numAudPartiesInConfig = (WORD)m_audVidConfig.GetAudio();
	WORD numVidPartiesInConfig = (WORD)m_audVidConfig.GetVideo();

	return CheckIfOneMorePartyCanBeAdded(videoPartyType, partyRole, allocPolicy, pResult, numHD720PortsAccordingToCards, numAudPartiesInConfig, numVidPartiesInConfig, i_rmxPortGaugeThreshold, pbAddAudioAsVideo, confModeType, countPartyAsICEinMFW);
}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::CheckIfOneMorePartyCanBeAddedCOP(ALLOC_PARTY_IND_PARAMS_S* pResult)
{
	//Breeze-COP
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", FALSE);

	DWORD max_cop_parties = (eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode()) ? 160 : 128;
	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator)
		max_cop_parties = pReservator->GetDongleRestriction();
	TRACEINTO << "max_cop_parties=" << max_cop_parties;

	return (GetTotalAllocatedAudioParties(TRUE) + 1 > max_cop_parties) ? FALSE : TRUE;
}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType,
													 ePartyRole oldPartyRole,
													 eVideoPartyType& newVideoPartyType,
													 ePartyRole& newPartyRole,
													 BOOL* pbAddAudioAsVideo /*= NULL*/,
													 eConfModeTypes confModeType /*= eNonMix*/)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", TRUE);
	WORD numHD720PortsAccordingToCards = pSystemResources->GetHD720PortsAccordingToCards();
	WORD numAudPartiesInConfig = (WORD)m_audVidConfig.GetAudio();
	WORD numVidPartiesInConfig = (WORD)m_audVidConfig.GetVideo();

	return CheckIfOnePartyCanBechanged(oldVideoPartyType, oldPartyRole, newVideoPartyType, newPartyRole,numHD720PortsAccordingToCards, numAudPartiesInConfig, numVidPartiesInConfig, pbAddAudioAsVideo, confModeType);
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::AddParty(BasePartyDataStruct& rPartyData)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();

	PASSERTMSG_AND_RETURN(!pSystemResources, "Failed, pSystemResources is NULL");
	PASSERTMSG_AND_RETURN(!pResourceManager, "Failed, pResourceManager is NULL");

	// Board id is 1 based
	WORD reqBoardId = 0;
	if (CHelperFuncs::IsVideoRelayParty(rPartyData.m_videoPartyType))
	{
		reqBoardId = rPartyData.m_artBoardId;
	}
	else if (rPartyData.m_videoBoardId != 0xFFFF && rPartyData.m_videoBoardId != 0)
	{
		reqBoardId = rPartyData.m_videoBoardId;
	}
	else
	{
		reqBoardId = rPartyData.m_artBoardId;
	}

	if (reqBoardId > 0 && reqBoardId <= BOARDS_NUM)
		reqBoardId--;
	else
		PASSERTSTREAM_AND_RETURN(1, "Failed, reqBoardId=" << reqBoardId << " is out of range!");

	eVideoPartyType vptype = rPartyData.m_videoPartyType;

	// If Audio Only party, and we don't have free audio ports (MPMx), allocate one CIF port instead.
	// In case of MPM-Rx card we count the audio only parties
	if ((rPartyData.m_bAddAudioAsVideo) && (CHelperFuncs::IsAudioParty(rPartyData.m_videoPartyType) || CHelperFuncs::IsAudioContentParty(rPartyData.m_videoPartyType)))
	{
		// In case there are no audio ports available, we allocate one CIF port for audio only party
		if (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_mpmrx != pSystemResources->GetSystemCardsMode())
			vptype = eCP_H264_upto_CIF_video_party_type;

		m_numAudioPartiesAllocatedAsVideo[reqBoardId]++;
	}

	rPartyData.m_resourcePartyType = VideoPartyTypeToPartyResourceType(vptype);

	// Add party to matrix
	AddPartyToMatrix(vptype, rPartyData.m_partyRole, rPartyData.m_confModeType, reqBoardId);

	DWORD portGauge = pSystemResources->GetPortGauge();
	BOOL portGaugeAlarm = NO;
	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("PORT_GAUGE_ALARM", portGaugeAlarm);

	DumpTotalAllocatedParties();//TEMP

	std::ostringstream msg;
	msg << "CAutoModeResources::AddParty:"
		<< "\n  VideoPartyType          :" << eVideoPartyTypeNames[rPartyData.m_videoPartyType]
		<< "\n  PartyResourceType       :" << rPartyData.m_resourcePartyType
		<< "\n  PartyRole               :" << ePartyRoleNames[rPartyData.m_partyRole]
		<< "\n  ArtBoardId              :" << rPartyData.m_artBoardId
		<< "\n  VideoBoardId            :" << rPartyData.m_videoBoardId
		<< "\n  NumVideoParties         :" << GetTotalAllocatedVideoParties()
		<< "\n  NumAudioParties         :" << GetTotalAllocatedAudioParties(isMPMX)
		<< "\n  NumAudioPartiesAsVideo  :" << GetTotalAudioPartiesAllocatedAsVideo()
		<< "\n  PortGauge               :" << portGauge << "%"
		<< "\n  PortGaugeAlarm          :" << ((portGaugeAlarm == NO) ? "NO" : "YES")
		<< "\n  IsMixedConf             :" << ((rPartyData.m_confModeType == eMix) ? "YES" : "NO")
		<< "\n  AsCifParty              :" << ((rPartyData.m_bAddAudioAsVideo) ? "YES" : "NO");

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	if (portGaugeAlarm)
		CheckIfNeedGaugeAlarm(pResourceManager, rPartyData.m_videoPartyType, portGauge, true);
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::RemoveParty(BasePartyDataStruct& rPartyData)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();

	PASSERTMSG_AND_RETURN(!pSystemResources, "Failed, pSystemResources is NULL");
	PASSERTMSG_AND_RETURN(!pResourceManager, "Failed, pResourceManager is NULL");

	// Board id is 1 based
	WORD reqBoardId = 0;
	if (CHelperFuncs::IsVideoRelayParty(rPartyData.m_videoPartyType))
	{
		reqBoardId = rPartyData.m_artBoardId;
	}
	else if (rPartyData.m_videoBoardId != 0xFFFF && rPartyData.m_videoBoardId != 0)
	{
		reqBoardId = rPartyData.m_videoBoardId;
	}
	else
	{
		reqBoardId = rPartyData.m_artBoardId;
	}

	if (reqBoardId > 0 && reqBoardId <= BOARDS_NUM)
		reqBoardId--;
	else
		PASSERTSTREAM_AND_RETURN(1, "Failed, reqBoardId=" << reqBoardId << " is out of range!");

	eVideoPartyType vptype = rPartyData.m_videoPartyType;

	// If Audio Only party, and we don't have free audio ports (MPMx), allocate one CIF port instead.
	if ((m_numAudioPartiesAllocatedAsVideo[reqBoardId] >= 1) && (CHelperFuncs::IsAudioParty(rPartyData.m_videoPartyType) || CHelperFuncs::IsAudioContentParty(rPartyData.m_videoPartyType)))
	{
		// In case there are no audio ports available, we allocate one CIF port for audio only party
		if (eSystemCardsMode_mpmrx != pSystemResources->GetSystemCardsMode() && eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
		{
			vptype = eCP_H264_upto_CIF_video_party_type;
			rPartyData.m_bAddAudioAsVideo = TRUE;
		}

		m_numAudioPartiesAllocatedAsVideo[reqBoardId]--;
	}

	// Remove party to matrix
	RemovePartyFromMatrix(vptype, rPartyData.m_partyRole, rPartyData.m_confModeType, reqBoardId);
	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());
	WORD portGauge = pSystemResources->GetPortGauge();

	std::ostringstream msg;
	msg << "CAutoModeResources::RemoveParty:"
		<< "\n  VideoPartyType          :" << eVideoPartyTypeNames[rPartyData.m_videoPartyType]
		<< "\n  PartyResourceType       :" << rPartyData.m_resourcePartyType
		<< "\n  PartyRole               :" << ePartyRoleNames[rPartyData.m_partyRole]
		<< "\n  ArtBoardId              :" << rPartyData.m_artBoardId
		<< "\n  VideoBoardId            :" << rPartyData.m_videoBoardId
		<< "\n  NumVideoParties         :" << GetTotalAllocatedVideoParties()
		<< "\n  NumAudioParties         :" << GetTotalAllocatedAudioParties(isMPMX)
		<< "\n  NumAudioPartiesAsVideo  :" << GetTotalAudioPartiesAllocatedAsVideo()
		<< "\n  PortGauge               :" << portGauge << "%"
		<< "\n  IsMixedConf             :" << ((rPartyData.m_confModeType == eMix) ? "YES" : "NO")
		<< "\n  AsCifParty              :" << ((rPartyData.m_bAddAudioAsVideo) ? "YES" : "NO");

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	CheckIfNeedGaugeAlarm(pResourceManager, rPartyData.m_videoPartyType, portGauge, false);
}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::IsThereAnyParty()
{
	return (GetTotalAllocatedVideoParties() != 0 || GetTotalAllocatedAudioParties(TRUE) != 0);
}
//////////////////////////////////////////////////////////////////////////////////
WORD CAutoModeResources::GetMaxNumberOfParties()
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", 0);

	// Max parties per HD720 port is 12 in MPMx, MPM-Rx, Ninja and SoftMCU (HD:Audio Only ratio is 1:12).
	WORD numPartiesPerHD720Port = 12;

	if (eProductTypeSoftMCUMfw == pSystemResources->GetProductType())
		numPartiesPerHD720Port = 15; //license in CP HD units, so 67*3=200HD720, 200*5=1000CIF

	WORD maxParties = m_dongleNumParties * numPartiesPerHD720Port;

	eProductType prodType = pSystemResources->GetProductType();

	switch (prodType)
	{
		case eProductTypeRMX1500:
			if (maxParties > MAX_PARTIES_PER_SYSTEM_RMX1500)
				maxParties = MAX_PARTIES_PER_SYSTEM_RMX1500;
			break;

		case eProductTypeRMX2000:
			if (maxParties > MAX_PARTIES_PER_SYSTEM_RMX2000)
				maxParties = MAX_PARTIES_PER_SYSTEM_RMX2000;
			break;

		case eProductTypeCallGenerator:
		case eProductTypeRMX4000:
			if (maxParties > MAX_PARTIES_PER_SYSTEM_RMX4000)
				maxParties = MAX_PARTIES_PER_SYSTEM_RMX4000;
			break;

		case eProductTypeNinja:
			if (maxParties > MAX_PARTIES_PER_SYSTEM_NINJA)
				maxParties = MAX_PARTIES_PER_SYSTEM_NINJA;
			break;

		case eProductTypeSoftMCUMfw:
			if (maxParties > MAX_PARTIES_PER_SYSTEM_SOFT_MFW)
				maxParties = MAX_PARTIES_PER_SYSTEM_SOFT_MFW;
			break;

		case eProductTypeSoftMCU:
		case eProductTypeGesher:
		case eProductTypeEdgeAxis:
		case eProductTypeCallGeneratorSoftMCU:
			if (maxParties > MAX_PARTIES_PER_SYSTEM_SOFT_MCU)
				maxParties = MAX_PARTIES_PER_SYSTEM_SOFT_MCU;
			break;

		case eProductTypeNPG2000:
		default:
			break;
	}

	TRACEINTO << "maxParties: " << maxParties;
	return maxParties; //maximum parties is when all are audio
}
//////////////////////////////////////////////////////////////////////////////////
DWORD CAutoModeResources::GetOccupiedNumberOfParties()
{
	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(pSystemResources == NULL, 0);
	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());
	DWORD num_parties = (DWORD)ceil(GetTotalAllocatedVideoParties() + GetTotalAllocatedAudioParties(isMPMX));
	TRACEINTO << "num_parties:" << num_parties << ", AudPartiesAllocateddAsVideo:" << GetTotalAudioPartiesAllocatedAsVideo();
	return num_parties;
}
//////////////////////////////////////////////////////////////////////////////////
WORD CAutoModeResources::GetNumberOfRequiredUDPPortsPerBoard(eIceEnvironmentType isIceEnv)//ICE 4 ports
{
	//in auto barak mode, this number should be the worst case. This is when all units are Audio units
	//in this case we need: 16 x 2 x 32 (16 audio ports, 2 channels (in + out), on 32 units) = 1024
	//or MPMX we need:  9*4*40   (9 audio ports, 4 channels (in + out), on 40 units) =1440
	//In MPMRx: 300 parties per board * (2 (rtcp +rtp) * 5 channels (audio + video + content + FECC + BFCP) = 3000
	//In MPMRx with ICE: 300 parties per board * (2 for ICE * 2 (rtcp +rtp) * 5 channels (audio + video + FECC + BFCP + Content) = 6000
	//In SoftMCU: 450 SVC parties * 2 (rtcp +rtp) * 5 channels (audio + video + FECC + BFCP + Content) = 4500
	//In SoftMCU with ICE: 450 SVC parties * (2 for ICE * 2 (rtcp +rtp) * 5 channels (audio + video + FECC + BFCP + Content) = 9000
	//We allocate BFCP and Content for ICE for future use.

	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(pSystemResources == NULL, 2048);

	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();
	eProductType prodType = pSystemResources->GetProductType();

	//Special case
	if (eProductTypeSoftMCUMfw == prodType)
		return 10000;

	//If non ICE
	if (isIceEnv == eIceEnvironment_None || isIceEnv == eIceEnvironment_Standard)
	{
		if (eProductFamilySoftMcu == curProductFamily)
			return 4500;

		return 3000;
	}
	//ICE
	if (eProductFamilySoftMcu == curProductFamily)
		return 9000;

	return 6000;
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CAutoModeResources::IsRsrcEnough(CBoardsStatistics* pBoardsStatistics)
{
	UpdateInternalDataFromIsRsrcEnough(pBoardsStatistics);

	DWORD configAudioPorts = m_audVidConfig.GetAudio();
	DWORD configVideoPorts = m_audVidConfig.GetVideo();

	DWORD totalConfiguredAudioPortsOnCards = pBoardsStatistics->GetTotalCunfiguredAudioPortsOnCards();
	DWORD totalConfiguredVideoPortsOnCards = pBoardsStatistics->GetTotalCunfiguredVideoPortsOnCards();

	TRACEINTO
			<< "\n  totalConfiguredAudioPortsOnCards :" << totalConfiguredAudioPortsOnCards
			<< "\n  totalConfiguredVideoPortsOnCards :" << totalConfiguredVideoPortsOnCards
			<< "\n  configAudioPorts (license)       :" << configAudioPorts
			<< "\n  configVideoPorts (license)       :" << configVideoPorts;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", STATUS_FAIL);

	WORD numHD720PortsAccordingToCards = pSystemResources->GetHD720PortsAccordingToCards();

	CPortsConfig* pPortsConfig = GetPortsConfig();
	if (pPortsConfig != NULL)
		pPortsConfig->SetMaxHD720PortsAccordingToCards(numHD720PortsAccordingToCards);
	else
		PASSERTMSG(1, "Failed, pPortsConfig is NULL");

	if (configAudioPorts > totalConfiguredAudioPortsOnCards || configVideoPorts > totalConfiguredVideoPortsOnCards)
		return STATUS_CURRENT_SLIDER_SETTINGS_REQUIRES_MORE_RESOURCES_THAN_AVAILBLE;

	if (eProductTypeNinja == pSystemResources->GetProductType())
	{
		if (numHD720PortsAccordingToCards < configVideoPorts)
			return STATUS_FAIL;
	}
	else if (numHD720PortsAccordingToCards < configVideoPorts + (WORD)ceil(configAudioPorts * m_LogicalHD720WeightAvcParty[eNonMix][e_Audio]))
		return STATUS_FAIL;

	if (!CHelperFuncs::IsSoftMCU(pSystemResources->GetProductType()))
	{
		eSystemCardsMode systemCardsMode = pSystemResources->GetSystemCardsMode();

		DWORD dongle_ports = pPortsConfig ? pPortsConfig->GetDonglePortsRestriction() : 0;

		DWORD svc_by_dongle_ports = 0;
		if (eSystemCardsMode_mpmrx == systemCardsMode)
			svc_by_dongle_ports = dongle_ports * 2; // For MPM-Rx, SVC license ports are twice than HD (dongle) ports.
		else
			svc_by_dongle_ports = (DWORD)(dongle_ports/m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]);

		DWORD svc_ports_per_cards = pSystemResources->GetSvcPortsPerCards();
		if (pSystemResources->GetSvcFlag())
			m_numDonglePartiesSVC[eNonMix][e_Cif] = (svc_by_dongle_ports > svc_ports_per_cards) ? svc_ports_per_cards : svc_by_dongle_ports;

		TRACEINTO << "cp license=" << dongle_ports << ", svc ports by cp dongle=" << svc_by_dongle_ports << ", svc ports per cards=" << svc_ports_per_cards << ", num dongle SVC=" << m_numDonglePartiesSVC[eNonMix][e_Cif];
	}

	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::InitDongleRestriction(DWORD& num_parties,BOOL bRecalculateReservationPartyResources)
{
	// Ports Configuration
	// Read and update Ports Configuration
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	CProcessSettings* pProcessSettings = CHelperFuncs::GetProcessSettings();
	PASSERT_AND_RETURN(!pProcessSettings);
	PASSERT_AND_RETURN(!m_pPortsConfig);

	//if license expired on startup
	if (pSystemResources->isLicenseExpired())
	{
		TRACEINTO << "License is expired no need to set dongle restriction";

		if (pSystemResources->GetCurrentResourcesInterface())
			pSystemResources->GetCurrentResourcesInterface()->OnLicenseExpired();

		return;
	}

	BaseInitDongleRestriction(num_parties);

	m_dongleNumParties = num_parties;

	if (STATUS_FAIL == m_pPortsConfig->SetDongleRestriction(num_parties))
		TRACEINTO << "Failed, SetDongleRestriction = STATUS_FAIL";// need to consider what to do in this case

	CAudioVideoConfig l_aud_vid_cfg(0, num_parties); // Default Value (maximum video)
	DWORD audio = 0, video = 0;

	if (pProcessSettings->GetSettingDWORD("audio", audio))
	{
		l_aud_vid_cfg.SetAudio(audio);

		if (pProcessSettings->GetSettingDWORD("video", video))
			l_aud_vid_cfg.SetVideo(video);
	}

	eProductType product_type = pSystemResources->GetProductType();
	if (pSystemResources->GetSvcFlag() && (!pSystemResources->IsSystemCpuProfileDefined() || bRecalculateReservationPartyResources))
	{
		num_parties = SetSvcDongleRestriction(num_parties);
	}
	// Check the validity of the values against the list
	size_t index;
	if (!m_pPortsConfig->FindPortsConfigurationIndexByConfig(l_aud_vid_cfg, index)) //BRIDGE-2644
	{
		TRACEINTO << "Unable to find configuration in list, setting default"
				<< "\n  Audio :" << l_aud_vid_cfg.GetAudio()
				<< "\n  Video :" << l_aud_vid_cfg.GetVideo();

		index = 0;
		l_aud_vid_cfg = CAudioVideoConfig(0, num_parties); // If values are non-valid, set default

		audio = l_aud_vid_cfg.GetAudio();
		video = l_aud_vid_cfg.GetVideo();
	}
	m_audVidConfig = l_aud_vid_cfg;
	m_pPortsConfig->SetSelectedIndex(index);

	TRACEINTO << "Audio video configuration"
			<< "\n  Audio :" << l_aud_vid_cfg.GetAudio()
			<< "\n  Video :" << l_aud_vid_cfg.GetVideo();

	InitReservatorAndStaticCardConfigParams(bRecalculateReservationPartyResources);
}

////////////////////////////////////////////////////////////////////////////
int CAutoModeResources::SetSvcDongleRestriction(DWORD num_parties)
{
	int retNumParties = num_parties;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, retNumParties);

	eProductType productType = pSystemResources->GetProductType();
	switch(productType)
	{
		case eProductTypeSoftMCUMfw:
		{
			DWORD numCifSvc = (DWORD)ceil(num_parties/PORT_WEIGHT_SVC_CIF_SOFT_MFW);
			if (numCifSvc >= MAX_NUMBER_SVC_PARTICIPANTS_SOFT_MFW_HIGH)
			{
				InitHighCapacityAndWeightsSoftMFW();
				retNumParties = MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW;
			}
			else if (numCifSvc >= MAX_NUMBER_SVC_PARTICIPANTS_SOFT_MFW_LOW)
			{
				InitLowCapacityAndWeightsSoftMFW();
				retNumParties = MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW_16;
			}
			else
			{
				InitDemoCapacityAndWeightsSoftMFW();
				retNumParties = MAX_NUMBER_HD_PARTICIPANTS_SOFT_MFW_8;
			}

			PrintToTracePartyLogicalWeight("SetSvcDongleRestriction");
			break;
		}

		case eProductTypeNinja:
		case eProductTypeSoftMCU:
		case eProductTypeGesher:
		case eProductTypeEdgeAxis:
			m_numDonglePartiesSVC[eNonMix][e_Cif] = (WORD) (num_parties * MEDIA_RELAY_PORTS_RATIO);//from SRS: Media-Relay in RMX does not require licensing.
			break;//The HD ports value in license should be multiplied by 3 to achieve the licensed number of SVC ports.

		case eProductTypeCallGeneratorSoftMCU:
			m_numDonglePartiesSVC[eNonMix][e_Cif] = 0;
			break;

		default:
			m_numDonglePartiesSVC[eNonMix][e_Cif] = num_parties;
			break;
	}

	TRACEINTO << "NumParties:" << num_parties << ", NumDonglePartiesSVC[eNonMix][e_Cif]:" << m_numDonglePartiesSVC[eNonMix][e_Cif] << ", ReturnNumParties:" << retNumParties;
	return retNumParties;
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::FillRequiredPartyResourcesForConference(WORD minNumAudioPartiesInConf, WORD minNumVideoPartiesInConf,eVideoPartyType maxVideoPartyTypeInConf,CPartiesResources& partiesResourcesToFill)
{
	if (CHelperFuncs::IsMode2C())
	{
		partiesResourcesToFill.m_physical_audio_ports = 0;
		partiesResourcesToFill.m_physical_hd720_ports = 0;
		partiesResourcesToFill.m_logical_COP_num_parties = minNumVideoPartiesInConf;//carmit-fix
	}
	else
	{
		partiesResourcesToFill.m_physical_audio_ports = minNumAudioPartiesInConf + minNumVideoPartiesInConf;
		partiesResourcesToFill.m_physical_hd720_ports = (WORD)ceil(minNumVideoPartiesInConf * CHelperFuncs::GetNumPhysicalHD720Ports(maxVideoPartyTypeInConf));
		partiesResourcesToFill.m_logical_num_parties[e_Audio] = minNumAudioPartiesInConf;
		partiesResourcesToFill.m_logical_num_parties[e_HD720] = ((float)minNumVideoPartiesInConf) * GetLogicalWeightForResourceCalculations(maxVideoPartyTypeInConf, eParty_Role_regular_party, eNonMix);
		TRACEINTO << "logical_num_parties[e_HD720]:" << partiesResourcesToFill.m_logical_num_parties[e_HD720];
	}
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::UpdateReservatorWithLogicalConfiguration(BOOL bRecalculateReservationPartyResources)
{
	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERTMSG_AND_RETURN(!pReservator, "Failed, pReservator is NULL");

	DWORD num_dngl = pReservator->GetDongleRestriction();
	WORD logical_num_parties[NUM_OF_PARTY_RESOURCE_TYPES];
	WORD logical_COP_num_parties[1];//carmit-fix
	for (int i = 0; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
		logical_num_parties[i] = 0;

	if (CHelperFuncs::IsMode2C())//carmit-fix
	{
		logical_COP_num_parties[0] = (WORD)num_dngl;
		pReservator->SetLogicalResources(logical_COP_num_parties, bRecalculateReservationPartyResources);
	}
	else
	{//carmit-fix-end
		logical_num_parties[e_Audio] = m_audVidConfig.GetAudio();
		logical_num_parties[e_HD720] = m_audVidConfig.GetVideo();
		pReservator->SetLogicalResources(logical_num_parties, bRecalculateReservationPartyResources);
	}
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::InitReservatorAndStaticCardConfigParams(BOOL bRecalculateReservationPartyResources)
{
	UpdateReservatorWithLogicalConfiguration(bRecalculateReservationPartyResources);

	DWORD audio = m_audVidConfig.GetAudio();
	DWORD video = m_audVidConfig.GetVideo();

	// Init information needed for unit configuration
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN(!pSystemResources, "Failed, pSystemResources is NULL");

	// Here we use e_Cif to calculate the number of required audio ports according to the lowest video.
	CCardResourceConfig::SetNumAudPortsLeftToConfig(audio + (DWORD)(video/m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]) - pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS));
	CCardResourceConfig::SetNumVidHD720PortsLeftToConfig(video - pSystemResources->GetNumConfiguredVideoPorts(ALL_BOARD_IDS));

	// the physical and logical weights are contradicted. In Breeze the dynamic reconfiguration doesn't exist,
	// that is why we need to compute the worst case for audio and video promilles.
	float artPromilsBreeze = CHelperFuncs::GetArtPromilsBreeze();
	float num_Needed_ART_prml = (audio + (video/m_LogicalHD720WeightAvcParty[eNonMix][e_Cif])) * artPromilsBreeze;
	float num_Needed_VID_prml = video * VID_TOTAL_HD720_30FS_PROMILLES_BREEZE;

	DWORD num_Needed_ART_Units = (DWORD)ceil(num_Needed_ART_prml / 1000);
	DWORD num_Needed_VID_Units = (DWORD)ceil(num_Needed_VID_prml / 1000);

	float configProportion = (num_Needed_ART_Units == 0) ? 0xFFFF : (((float)num_Needed_VID_Units)/num_Needed_ART_Units);
	CCardResourceConfigBreeze::SetConfigProportion(configProportion);

	TRACEINTO << "num_Needed_ART_Units:" << num_Needed_ART_Units <<  ", num_Needed_VID_Units:" << num_Needed_VID_Units << ", BreezeConfigProportion:" << configProportion;

	CCardResourceConfigSoft::SetConfigProportion(configProportion);//OLGA - SoftMCU

	CCardResourceConfigNinja::SetConfigProportion(configProportion);
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CAutoModeResources::ReconfigureUnitsAccordingToProportion(BOOL bForceReconfigure)
{
	STATUS sts;
	sts = BaseCanSetConfigurationNow();
	if (sts == STATUS_OK || bForceReconfigure)
	{
		PureModeReconfigureUnitsAccordingToProportion();
		return STATUS_OK;
	}else{
		return sts;
	}
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CAutoModeResources::CanSetConfigurationNow()
{
	return BaseCanSetConfigurationNow();
}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::CheckIfOneMorePartyCanBeAddedToIpService(eVideoPartyType& videoPartyType,
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
	int numAudPartiesInConfig = m_audVidConfig.GetAudio();
	int numVidPartiesInConfig = m_audVidConfig.GetVideo();

	WORD service_numHD720PortsAccordingToCards = (WORD)CHelperFuncs::CalcIpServicePart((DWORD)numHD720PortsAccordingToCards, service_factor,round_up);
	WORD service_numAudPartiesInConfig = (WORD)CHelperFuncs::CalcIpServicePart((DWORD)numAudPartiesInConfig, service_factor,round_up);
	WORD service_numVidPartiesInConfig = (WORD)CHelperFuncs::CalcIpServicePart((DWORD)numVidPartiesInConfig, service_factor,round_up);

	return CheckIfOneMorePartyCanBeAdded(videoPartyType, partyRole, allocPolicy, pResult, service_numHD720PortsAccordingToCards, service_numAudPartiesInConfig, service_numVidPartiesInConfig, FALSE, pbAddAudioAsVideo, confModeType, countPartyAsICEinMFW);
}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::CheckIfOnePartyCanBechangedInIpService(eVideoPartyType oldVideoPartyType,
																ePartyRole oldPartyRole,
																eVideoPartyType& newVideoPartyType,
																ePartyRole& newPartyRole,
																float service_factor,
																BOOL round_up,
																BOOL* pbAddAudioAsVideo /*= NULL*/,
																eConfModeTypes confModeType /*= eNonMix*/)
{
	WORD numAudPartiesInConfig = (WORD)m_audVidConfig.GetAudio();
	WORD numVidPartiesInConfig = (WORD)m_audVidConfig.GetVideo();

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", TRUE);
	WORD numHD720PortsAccordingToCards = pSystemResources->GetHD720PortsAccordingToCards();

	WORD service_numAudPartiesInConfig = (WORD)CHelperFuncs::CalcIpServicePart((DWORD)numAudPartiesInConfig, service_factor, round_up);
	WORD service_numVidPartiesInConfig = (WORD)CHelperFuncs::CalcIpServicePart((DWORD)numVidPartiesInConfig, service_factor, round_up);

	return CheckIfOnePartyCanBechanged(oldVideoPartyType, oldPartyRole, newVideoPartyType, newPartyRole,numHD720PortsAccordingToCards, service_numAudPartiesInConfig, service_numVidPartiesInConfig, pbAddAudioAsVideo, confModeType);
}
//////////////////////////////////////////////////////////////////////////////////
float CAutoModeResources::CalculateAdditionalLicenseForMixMode(CConfRsrc& rConfRsrc, PartyDataStruct& rPartyRsrc, bool isNewParty)
{
	//DumpTotalAllocatedParties();

	const std::set<CPartyRsrc>* pPartiesList = rConfRsrc.GetPartiesList();
	PASSERT_AND_RETURN_VALUE(!pPartiesList, 0);

	float additionalLicense = 0;

	std::set<CPartyRsrc>::iterator _iiEnd = pPartiesList->end();
	for (std::set<CPartyRsrc>::iterator _ii = pPartiesList->begin(); _ii != _iiEnd; ++_ii)
	{
		PartyRsrcID partyId = _ii->GetRsrcPartyId();
		CPartyRsrc* pParty = const_cast<CPartyRsrc*>(rConfRsrc.GetPartyRsrcByRsrcPartyId(partyId));
		if (pParty)
		{
			ePartyResourceTypes partyResourceType = pParty->GetPartyResourceType();
			if (CHelperFuncs::IsVideoRelayParty(pParty->GetVideoPartyType()))
			{
				additionalLicense += (m_logicalHD720WeightSvcParty[eMix][partyResourceType]-m_logicalHD720WeightSvcParty[eNonMix][partyResourceType]);
			}
			else
			{
				additionalLicense += (m_LogicalHD720WeightAvcParty[eMix][partyResourceType]-m_LogicalHD720WeightAvcParty[eNonMix][partyResourceType]);
			}
		}
	}

	if (isNewParty)
	{
		ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(rPartyRsrc.m_videoPartyType);
		if (CHelperFuncs::IsVideoRelayParty(rPartyRsrc.m_videoPartyType))
			additionalLicense += m_logicalHD720WeightSvcParty[eMix][partyResourceType];
		else
			additionalLicense += m_LogicalHD720WeightAvcParty[eMix][partyResourceType];
	}

	return additionalLicense;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Additional functions
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
STATUS CAutoModeResources::UserChangedPortConfiguration()
{
	CProcessSettings *pProcessSettings = CHelperFuncs::GetProcessSettings();
	if ( pProcessSettings )
	{
		UpdateAudioVideoConfigProportion();

		InitReservatorAndStaticCardConfigParams();

		//send to function in base
		PureModeReconfigureUnitsAccordingToProportion();

		return STATUS_OK;
	}
	else
	{
		PASSERTMSG(1, "pProcessSettings is NULL");
		return STATUS_FAIL;
	}
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::UpdateAudioVideoConfigProportion()
{
	CProcessSettings *pProcessSettings = CHelperFuncs::GetProcessSettings();
	if ( pProcessSettings )
	{
		CAudioVideoConfig l_aud_vid_cfg(0, 0);
		DWORD audio = 0, video = 0;

		if ( pProcessSettings->GetSettingDWORD("audio", audio))
		{
			l_aud_vid_cfg.SetAudio( audio );

			if ( pProcessSettings->GetSettingDWORD("video", video))
			{
				l_aud_vid_cfg.SetVideo( video );
			}
		}

		m_audVidConfig = l_aud_vid_cfg;
		TRACEINTO << "Audio:" << audio << ", Video:" << video;
	}
}
//////////////////////////////////////////////////////////////////////////////////
STATUS CAutoModeResources::ResetPortConfiguration()
{
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();
	PASSERTMSG_AND_RETURN_VALUE(!pResourceManager, "Failed, pResourceManager is NULL", STATUS_FAIL);
	PASSERTMSG_AND_RETURN_VALUE(!m_pPortsConfig, "Failed, m_pPortsConfig is NULL", STATUS_FAIL);

	CProcessSettings *pProcessSettings = CHelperFuncs::GetProcessSettings();
	PASSERTMSG_AND_RETURN_VALUE(!pProcessSettings, "Failed, pProcessSettings is NULL", STATUS_FAIL);

	STATUS status = m_pPortsConfig->ResetConfigurationListAccordingToLicenseAndCards();
	if (status == STATUS_OK)
	{
		m_pPortsConfig->SetSelectedIndex(0); //default, all video
		CAudioVideoConfig* pAudVidCfg = m_pPortsConfig->FindPortsConfigurationConfigByIndex(0);

		if (pAudVidCfg)
		{
			char audio[32];
			sprintf(audio, "%d", pAudVidCfg->GetAudio());
			char video[32];
			sprintf(video, "%d", pAudVidCfg->GetVideo());

			pProcessSettings->SetSetting("audio", audio);
			pProcessSettings->SetSetting("video", video);
		}
		else
			PTRACE(eLevelError, "CAutoModeResources::UpdateAudioVideoConfigProportion");

		status = UserChangedPortConfiguration();

		pResourceManager->CheckResourceEnoughAndAddOrRemoveAciveAlarm(FALSE); //false: because it should always remove the active alarm
	}
	return status;
}
//////////////////////////////////////////////////////////////////////////////////
	void CAutoModeResources::CheckIfNeedGaugeAlarm(CResourceManager* pResourceManager, eVideoPartyType videoPartyType, DWORD portGauge, bool bAddParty)
	{
	std::auto_ptr<CRsrcReport> pReport(new CRsrcReport());
	CalculateResourceReport(pReport.get());

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN(!pSystemResources, "CAutoModeResources::CheckIfNeedGaugeAlarm - Failed, pSystemResources is NULL");
	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());

	float totalAllocatedVideoParties = GetTotalAllocatedVideoParties();
	float totalAllocatedAudioParties = GetTotalAllocatedAudioParties(isMPMX);

	char* message = NULL;
	if (videoPartyType == eVideo_party_type_none)
	{
		float numMaxAudParties = pReport->GetNumParties(e_Audio, TYPE_TOTAL);

		if (ceil((totalAllocatedAudioParties*100)/numMaxAudParties) >= portGauge)
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
			std::ostringstream msg;
			msg << "CAutoModeResources::CheckIfNeedGaugeAlarm - " << message
					<< "\n  numCurAudParties  = " << totalAllocatedAudioParties
					<< "\n  numMaxAudParties  = " << numMaxAudParties;
			PTRACE(eLevelInfoNormal, msg.str().c_str());
		}
	}
	else
	{
		float numMaxVidParties = pReport->GetNumParties(e_HD720, TYPE_TOTAL);

		if (ceil((totalAllocatedVideoParties*100)/numMaxVidParties) >= portGauge)
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
				message = "System resources of Video ports usage not exceeded Port Gauge threshold, remove Alarm";
				pResourceManager->RemoveActiveAlarm(VIDEO_PORT_GAUGE_THRESHOLD_REACHED);
			}
		}
		if (message)
		{
			std::ostringstream msg;
			msg << "CAutoModeResources::CheckIfNeedGaugeAlarm - " << message
					<< "\n  numCurVidParties  = " << totalAllocatedVideoParties
					<< "\n  numMaxVidParties  = " << numMaxVidParties;
			PTRACE(eLevelInfoNormal, msg.str().c_str());
		}
	}
	}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType,
													   ePartyRole& partyRole,
													   EAllocationPolicy allocPolicy,
													   ALLOC_PARTY_IND_PARAMS_S* pResult,
													   WORD numHD720PortsAccordingToCards,
													   WORD numMaxAudParties,
													   WORD numMaxVidParties,
													   BYTE i_rmxPortGaugeThreshold /*= FALSE*/,
													   BOOL* pbAddAudioAsVideo /*= NULL*/,
													   eConfModeTypes confModeType /*= eNonMix*/,
													   BOOL countPartyAsICEinMFW /*= FALSE*/)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CResourceManager* pResourceManager = CHelperFuncs::GetResourceManager();

	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", FALSE);
	PASSERTMSG_AND_RETURN_VALUE(!pResourceManager, "Failed, pResourceManager is NULL", FALSE);

	eProductType prodType = pSystemResources->GetProductType();

	BOOL bIsSoftMCU = CHelperFuncs::IsSoftMCU(pSystemResources->GetProductType());
	// check related to RamSize
	if (eSystemRamSize_half == pSystemResources->GetRamSize()) // only for 0.5GB
	{
		if (MAX_PARTIES_FOR_RAM_HALF_SIZE <= GetOccupiedNumberOfParties())
		{
			pResult->allocIndBase.status = STATUS_INSUFFICIENT_RSRC;
			TRACEINTO << "Failed due to 0.5 RAM size limitation";
			return FALSE;
		}
	}

	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());
	DWORD portGauge = pSystemResources->GetPortGauge();
	//CSS plugin would be treated as an audio only party since it doesn't consume video resource
	float num_ports = (isMPMX && (videoPartyType == eVideo_party_type_none || CHelperFuncs::IsAudioContentParty(videoPartyType))) ? 1 : GetLogicalWeightForResourceCalculations(videoPartyType, partyRole, confModeType);
	float num_hd720_ports = GetLogicalWeightForResourceCalculations(videoPartyType, partyRole, confModeType);

	DumpTotalAllocatedParties();

	float totalAllocatedVideoParties = GetTotalAllocatedVideoParties();
	float totalAllocatedAudioParties = GetTotalAllocatedAudioParties(FALSE);
	DWORD totalAudioPartiesAllocatedAsVideo = GetTotalAudioPartiesAllocatedAsVideo();

	std::ostringstream msg;
	msg << "CAutoModeResources::CheckIfOneMorePartyCanBeAdded:"
		<< "\n  videoPartyType                 :" << eVideoPartyTypeNames[videoPartyType] << " (" << videoPartyType << ")"
		<< "\n  partyRole                      :" << ePartyRoleNames[partyRole] << " (" << partyRole << ")"
		<< "\n  allocPolicy                    :" << AllocationPolicyToString(allocPolicy) << " (" << allocPolicy << ")"
		<< "\n  portGauge                      :" << portGauge << "%"
		<< "\n  numCurVidParties               :" << totalAllocatedVideoParties
		<< "\n  numMaxVidParties               :" << numMaxVidParties
		<< "\n  numCurAudParties               :" << totalAllocatedAudioParties
		<< "\n  numAudioPartiesAllocatedAsVideo:" << totalAudioPartiesAllocatedAsVideo
		<< "\n  numMaxAudParties               :" << numMaxAudParties
		<< "\n  numPorts                       :" << num_ports
		<< "\n  num_hd720_ports                :" << num_hd720_ports
		<< "\n  numHD720PortsAccordingToCards  :" << numHD720PortsAccordingToCards;

	PTRACE(eLevelInfoNormal, msg.str().c_str());

	if (CHelperFuncs::IsAudioParty(videoPartyType) || CHelperFuncs::IsAudioContentParty(videoPartyType)) //Audio only party case
	{
		if (numHD720PortsAccordingToCards+SMALL_ERROR < totalAllocatedAudioParties + num_hd720_ports + totalAllocatedVideoParties)
		{
			pResult->allocIndBase.status = STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED;
			TRACEINTO << "Failed, Case=1, Status=STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED";
			return FALSE;
		}

		// If we don't have a free Audio port, then try to allocate the Audio party using the lowest video resource - CIF.
		if ((totalAllocatedAudioParties / m_LogicalHD720WeightAvcParty[confModeType][e_Audio]) + num_ports > numMaxAudParties + FLOATING_POINT_ERROR)
		{
			eVideoPartyType allocAsVideoPartyType = !isMPMX ? videoPartyType : eCP_H264_upto_CIF_video_party_type;
			float newLogicalWeight = GetLogicalWeightForResourceCalculations(allocAsVideoPartyType, partyRole, confModeType);

			if ((numHD720PortsAccordingToCards+SMALL_ERROR < totalAllocatedAudioParties + totalAllocatedVideoParties + newLogicalWeight)
				     || (isMPMX && (totalAllocatedVideoParties + newLogicalWeight > numMaxVidParties+SMALL_ERROR))
				     || (!isMPMX && (totalAllocatedVideoParties + totalAllocatedAudioParties + newLogicalWeight > numMaxVidParties+SMALL_ERROR)))
			{
				pResult->allocIndBase.status = STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED;
				TRACEINTO << "Failed, Case=2, Status=STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED";
				return FALSE;
			}

			if (pbAddAudioAsVideo != NULL)
				*pbAddAudioAsVideo = TRUE;

			TRACEINTO << "Allocating audio as video\n";
		}
	}
	else //Video Party case
	{
		// Check hardware ports limitation according to cards, and check license limitations (resource slider config)
		if ((numHD720PortsAccordingToCards+SMALL_ERROR >= totalAllocatedAudioParties + totalAllocatedVideoParties + num_hd720_ports) &&
	      ((isMPMX && (totalAllocatedVideoParties + num_hd720_ports <= numMaxVidParties+SMALL_ERROR)) ||
	       (!isMPMX && (totalAllocatedVideoParties + totalAllocatedAudioParties + num_hd720_ports <= numMaxVidParties+SMALL_ERROR))))
		{
			goto Finalize;
		}

		// Not enough resources, so we try to down-grade
		else
		{
			// Try to down-grade
			while (videoPartyType != eVideo_party_type_none)
			{
				videoPartyType = CHelperFuncs::GetNextLowerVideoPartyType(videoPartyType);
				if (videoPartyType == eVideo_party_type_none) //got as low as possible
				{
					TRACEINTO << "Can't down-grade, got the lowest video party type";
					break;
				}

				num_hd720_ports = GetLogicalWeightForResourceCalculations(videoPartyType, partyRole, confModeType);
				if ((numHD720PortsAccordingToCards+SMALL_ERROR >= totalAllocatedAudioParties + totalAllocatedVideoParties + num_hd720_ports) &&
						((isMPMX && (totalAllocatedVideoParties + num_hd720_ports <= numMaxVidParties+SMALL_ERROR)) ||
						 (!isMPMX && (totalAllocatedVideoParties + totalAllocatedAudioParties + num_hd720_ports <= numMaxVidParties+SMALL_ERROR))))
				{
					goto Finalize;
				}
			}

			// If we got until here, it means that we try to down-grade all video types
			if (allocPolicy == eAllowDowngradingToAudioOnly)
			{
				// Try to down-grade to audio only
				num_ports = (isMPMX) ? 1 : GetLogicalWeightForResourceCalculations(eVideo_party_type_none, partyRole, confModeType);
				num_hd720_ports = GetLogicalWeightForResourceCalculations(eVideo_party_type_none, partyRole, confModeType);

				if (!isMPMX) // For non MPMX case - no pool of audio resources
				{
					if ((numHD720PortsAccordingToCards+SMALL_ERROR >= totalAllocatedAudioParties + totalAllocatedVideoParties + num_hd720_ports) &&
						(totalAllocatedVideoParties + totalAllocatedAudioParties + num_hd720_ports <= numMaxVidParties+SMALL_ERROR))
					{
						if (pbAddAudioAsVideo != NULL)
							*pbAddAudioAsVideo = TRUE;

						TRACEINTO << "bIsSoftMCU == TRUE : Allocating audio as video\n";
						goto Finalize;
					}
				}
				else if ((numHD720PortsAccordingToCards+SMALL_ERROR >= totalAllocatedAudioParties + num_hd720_ports + totalAllocatedVideoParties) &&
					((totalAllocatedAudioParties / m_LogicalHD720WeightAvcParty[confModeType][e_Audio]) + num_ports <= numMaxAudParties + FLOATING_POINT_ERROR))
				{
					goto Finalize;
				}

				pResult->allocIndBase.status = STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED;
				TRACEINTO << "Failed, Case=3, Status=STATUS_NUMBER_OF_AUDIO_PARTIES_EXCEEDED";
				return FALSE;
			}
			else
			{
				pResult->allocIndBase.status = STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED;
				TRACEINTO << "Failed, Case=4, Status=STATUS_NUMBER_OF_VIDEO_PARTIES_EXCEEDED (policy is not allow down-grading to AudioOnly)";
				return FALSE;
			}
		}
	}

	Finalize:

	if (i_rmxPortGaugeThreshold)
	{
		std::auto_ptr<CRsrcReport> pReport(new CRsrcReport());
		STATUS status = pSystemResources->CalculateResourceReport(pReport.get());
		PASSERTMSG_AND_RETURN_VALUE(status != STATUS_OK, "CalculateResourceReport return invalid status", FALSE);

		float numMaxAudParties = pReport->GetNumParties(e_Audio, TYPE_TOTAL);
		float numCurAudParties = (totalAllocatedAudioParties / m_LogicalHD720WeightAvcParty[confModeType][e_Audio]) + num_ports;
		if (((numCurAudParties * 100) / numMaxAudParties) >= portGauge)
		{
			pResult->allocIndBase.status = STATUS_RMX_PORT_GAUGE_THRESHOLD_EXCEEDED;
			char* message = "Failed, system resources of Audio ports usage has exceeded Port Gauge threshold";

			std::ostringstream msg;
			msg << "CAutoModeResources::CheckIfOneMorePartyCanBeAdded - " << message
				<< "\n  numCurAudParties        = " << numCurAudParties
				<< "\n  numMaxAudParties        = " << numMaxAudParties
				<< "\n  AudPartiesAllocdAsVideo = " << totalAudioPartiesAllocatedAsVideo;
			PTRACE(eLevelInfoNormal, msg.str().c_str());
			return FALSE;
		}
	}

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::CheckIfOnePartyCanBechanged(eVideoPartyType oldVideoPartyType,
													 ePartyRole oldPartyRole,
													 eVideoPartyType& newVideoPartyType,
													 ePartyRole& newPartyRole,
													 WORD numHD720PortsAccordingToCards,
													 WORD numMaxAudParties,
													 WORD numMaxVidParties,
													 BOOL* pbAddAudioAsVideo /*= NULL*/,
													 eConfModeTypes confModeType /*= eNonMix*/)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(oldVideoPartyType == newVideoPartyType, "Failed, same video party type", TRUE);
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", FALSE);

	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());

	float totalAllocatedVideoParties = GetTotalAllocatedVideoParties();
	float totalAllocatedAudioParties = GetTotalAllocatedAudioParties(isMPMX);

	std::ostringstream msg;
	msg << "CAutoModeResources::CheckIfOnePartyCanBechanged:"
		<< "\n  OldVideoPartyType             :" << eVideoPartyTypeNames[oldVideoPartyType] << " (" << oldVideoPartyType << ")"
		<< "\n  OldPartyRole                  :" << ePartyRoleNames[oldPartyRole] << " (" << oldPartyRole << ")"
		<< "\n  NewVideoPartyType             :" << eVideoPartyTypeNames[newVideoPartyType] << " (" << newVideoPartyType << ")"
		<< "\n  NewPartyRole                  :" << ePartyRoleNames[newPartyRole] << " (" << newPartyRole << ")"
		<< "\n  NumCurVidParties              :" << totalAllocatedVideoParties
		<< "\n  NumMaxVidParties              :" << numMaxVidParties
		<< "\n  NumCurAudParties              :" << totalAllocatedAudioParties
		<< "\n  NumMaxAudParties              :" << numMaxAudParties
		<< "\n  AudPartiesAllocdAsVideo       :" << GetTotalAudioPartiesAllocatedAsVideo()
		<< "\n  NumHD720PortsAccordingToCards :" << numHD720PortsAccordingToCards;

	BOOL RC = FALSE;

	// bridge-10240
	if(numMaxVidParties > numHD720PortsAccordingToCards)
	{
		TRACEINTO << " set numMaxVidParties = numHD720PortsAccordingToCards";
		numMaxVidParties = numHD720PortsAccordingToCards;
	}

	if (eVideo_party_type_none == newVideoPartyType || eVoice_relay_party_type == newVideoPartyType)
	{
		float num_ports = (isMPMX) ? 1 : GetLogicalWeightForResourceCalculations(eVideo_party_type_none, newPartyRole, confModeType);
		//changing to audio, check audio
		if (totalAllocatedAudioParties + num_ports > numMaxAudParties) // We check here whether it's possible to allocate audio party as video (port)
		{
			eVideoPartyType allocAsVideoPartyType = (!isMPMX ? eVideo_party_type_none : eCP_H264_upto_CIF_video_party_type);
			float numPortsOfOldType = GetLogicalWeightForResourceCalculations(oldVideoPartyType, oldPartyRole, confModeType);

			if (totalAllocatedVideoParties - numPortsOfOldType + GetLogicalWeightForResourceCalculations(allocAsVideoPartyType, newPartyRole, confModeType) > numMaxVidParties)
				RC = FALSE;
			else
			{
				RC = TRUE;
				*pbAddAudioAsVideo = TRUE;
			}

			goto Finalize;
		}
		else
		{
			RC = TRUE;
			goto Finalize;
		}
	}
	else
	{
		// down-grade
		if (oldVideoPartyType > newVideoPartyType)
		{
			RC = TRUE; goto Finalize;
		}

		int currentFreeports = (int)floor(numMaxVidParties - totalAllocatedVideoParties);
		if (currentFreeports <= 0)
		{
			RC = FALSE; goto Finalize;
		}

		//upgrade video, check video
		float numPortsOfOldType = GetLogicalWeightForResourceCalculations(oldVideoPartyType, oldPartyRole, confModeType);
		if (totalAllocatedVideoParties - numPortsOfOldType + GetLogicalWeightForResourceCalculations(newVideoPartyType, newPartyRole, confModeType) <= numMaxVidParties)
		{
			RC = TRUE; goto Finalize;
		}

		while (oldVideoPartyType < newVideoPartyType)
		{
			newVideoPartyType = CHelperFuncs::GetNextLowerVideoPartyType(newVideoPartyType);
			if (oldVideoPartyType >= newVideoPartyType) //got as low as possible
			{
				RC = FALSE; goto Finalize;
			}

			if (totalAllocatedVideoParties - numPortsOfOldType + GetLogicalWeightForResourceCalculations(newVideoPartyType, newPartyRole, confModeType) <= numMaxVidParties)
			{
				RC = TRUE; goto Finalize;
			}
		}
	}

Finalize:
	if (RC)
		msg << "\n  RC                            :TRUE, party can be changed to " << eVideoPartyTypeNames[newVideoPartyType] << " (" << newVideoPartyType << ")";
	else
		msg << "\n  RC                            :FALSE, party cannot be changed";

	PTRACE(eLevelInfoNormal, msg.str().c_str());
	return RC;
}

#undef min
#include "PrettyTable.h"

//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::DumpTotalAllocatedParties()
{
	bool isPrintTable = false;

	CPrettyTable<WORD, const char*, const char*, const char*> tbl("BoardId", "ConfMode", "AVC Parties", "SVC Parties");

	for (WORD boardId = 0; boardId < BOARDS_NUM; ++boardId)
	{
		for (WORD confMode = 0; confMode < NUM_OF_CONF_MODE_TYPES; ++confMode)
		{
			std::ostringstream msg_avc;
			std::ostringstream msg_svc;

			for (int i = e_Audio; i < NUM_OF_PARTY_RESOURCE_TYPES; ++i)
			{
				ePartyResourceTypes partyResourceType = (ePartyResourceTypes)i;
				if (m_numCurrentAVCParties[boardId][confMode][partyResourceType])
				{
					if (msg_avc.str().length())
						msg_avc << " + ";
					msg_avc << to_string(partyResourceType) << "(" << m_numCurrentAVCParties[boardId][confMode][partyResourceType] << ")*" << m_LogicalHD720WeightAvcParty[confMode][partyResourceType];
				}
				if (m_numCurrentSVCParties[boardId][confMode][partyResourceType])
				{
					if (msg_svc.str().length())
						msg_svc << " + ";
					msg_svc << to_string(partyResourceType) << "(" << m_numCurrentSVCParties[boardId][confMode][partyResourceType] << ")*" << m_logicalHD720WeightSvcParty[confMode][partyResourceType];
				}
			}
			std::string str_avc = msg_avc.str();
			std::string str_svc = msg_svc.str();
			if (str_avc.length() || str_svc.length())
			{
				isPrintTable = true;
				tbl.Add(boardId+1, to_string((eConfModeTypes)confMode), str_avc.c_str(), str_svc.c_str());
			}
		}
	}
	if (isPrintTable)
		TRACEINTO << tbl.Get();
}

#ifndef min
	#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

//////////////////////////////////////////////////////////////////////////////////
float CAutoModeResources::GetTotalAllocatedVideoParties()
{
	float fTotal = 0;

	for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		for (WORD confMode = 0; confMode < NUM_OF_CONF_MODE_TYPES; confMode++)
		{
			for (WORD partyResourceType = e_Cif; partyResourceType < NUM_OF_PARTY_RESOURCE_TYPES; partyResourceType++)
			{
				fTotal += (m_numCurrentAVCParties[boardId][confMode][partyResourceType] * m_LogicalHD720WeightAvcParty[confMode][partyResourceType]);
				fTotal += (m_numCurrentSVCParties[boardId][confMode][partyResourceType] * m_logicalHD720WeightSvcParty[confMode][partyResourceType]);
			}
		}
	}

	return fTotal;
}
//////////////////////////////////////////////////////////////////////////////////
float CAutoModeResources::GetTotalAllocatedAudioParties(BOOL isSeperatePools)
{
	float fTotal = 0;

	for (WORD boardId = 0; boardId < BOARDS_NUM; boardId++)
	{
		for (WORD confMode = 0; confMode < NUM_OF_CONF_MODE_TYPES; confMode++)
		{
			if(isSeperatePools)
			{
				fTotal += m_numCurrentAVCParties[boardId][confMode][e_Audio];
				fTotal += m_numCurrentSVCParties[boardId][confMode][e_Audio];
			}
			else
			{
				fTotal += (m_numCurrentAVCParties[boardId][confMode][e_Audio] * m_LogicalHD720WeightAvcParty[confMode][e_Audio]);
				fTotal += (m_numCurrentSVCParties[boardId][confMode][e_Audio] * m_logicalHD720WeightSvcParty[confMode][e_Audio]);
			}
		}
	}

	return fTotal;
}
//////////////////////////////////////////////////////////////////////////////////
DWORD CAutoModeResources::GetTotalAudioPartiesAllocatedAsVideo()
{
	DWORD uTotal = 0;

	for (WORD i = 0; i < BOARDS_NUM; i++)
	{
		uTotal += m_numAudioPartiesAllocatedAsVideo[i];
	}

	return uTotal;
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::CountReportPorts(eRPRTtypes rprt_type, WORD hd720_ports_according_to_cards, WORD& numVidHD720Parties, WORD& numVidCifParties, WORD& numAudParties, DWORD& ppm)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN(!pSystemResources, "Failed, pSystemResources is NULL");

	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());
	eProductType productType = pSystemResources->GetProductType();

	//calculate really configured parties - to take MIN with license restriction.
	WORD numTotalVidParties = 0, numTotalAudParties= 0;

	if (CHelperFuncs::IsMode2C())
	{
		numTotalVidParties = 0;
		numTotalAudParties = (m_dongleNumParties < pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS)) ? m_dongleNumParties : pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS);
	}
	else
	{
		// Get the number of configured Video ports and Audio ports according to cards.
		DWORD numConfiguredVideoPorts = pSystemResources->GetNumConfiguredVideoPorts(ALL_BOARD_IDS);
		DWORD numConfiguredAudioPorts = pSystemResources->GetNumConfiguredArtPorts(ALL_BOARD_IDS) - (DWORD)floor(numConfiguredVideoPorts / m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]);
		// Get the minimum between the resource slider config (license) and Video/Audio ports according to cards.
		numTotalVidParties = (m_audVidConfig.GetVideo() < numConfiguredVideoPorts) ? m_audVidConfig.GetVideo() : numConfiguredVideoPorts;
		numTotalAudParties = (m_audVidConfig.GetAudio() < numConfiguredAudioPorts) ? m_audVidConfig.GetAudio() : numConfiguredAudioPorts;
	}

	//calculate really configured parties - to take MIN with license restriction.
	WORD min_video_parties = min( numTotalVidParties, hd720_ports_according_to_cards);
	WORD aud_parties = (WORD)floor(hd720_ports_according_to_cards / m_LogicalHD720WeightAvcParty[eNonMix][e_Audio]);
	WORD min_audio_parties = min(numTotalAudParties, aud_parties);

	float totalAllocatedVideoParties = GetTotalAllocatedVideoParties();
	float totalAllocatedAudioParties = GetTotalAllocatedAudioParties(isMPMX);

	if (CHelperFuncs::IsMode2C())
		min_audio_parties = min(numTotalAudParties, hd720_ports_according_to_cards);

	switch (rprt_type)
	{
		case TYPE_TOTAL:
		{
			numVidHD720Parties = min_video_parties;
			if (eProductTypeSoftMCUMfw == productType)
				numVidCifParties = (WORD)floor(numVidHD720Parties / m_logicalHD720WeightSvcParty[eNonMix][e_Cif]); //VNGSWIBM-1616: IBM is working via DMA, which shows resource report in CIF units
			else
				numVidCifParties = (WORD)floor(numVidHD720Parties / m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]);
			numAudParties = min_audio_parties;
			break;
		}
		case TYPE_OCCUPIED:
		{
			if (!isMPMX)
			{
				numVidHD720Parties = (WORD)ceil(totalAllocatedVideoParties + totalAllocatedAudioParties);
				numVidCifParties = (WORD)ceil((totalAllocatedVideoParties + totalAllocatedAudioParties) / m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]);
				numAudParties = 0;
			}
			else
			{
				numVidHD720Parties = min_video_parties < (WORD)ceil(totalAllocatedVideoParties) ? (WORD)floor(totalAllocatedVideoParties) : (WORD)ceil(totalAllocatedVideoParties);
				numVidCifParties = (WORD)ceil(totalAllocatedVideoParties / m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]);
				numAudParties = (WORD)ceil(totalAllocatedAudioParties);
			}
			break;
		}
		case TYPE_RESERVED:
		{
			numVidHD720Parties = 0; //m_numUndefReservedParties; //TBD for undefined separate field
			numVidCifParties = 0;
			numAudParties = 0;
			break;
		}
		case TYPE_FREE:
		{
			float free_hd_ports = (min_video_parties >= totalAllocatedVideoParties) ? ( ((float)min_video_parties) - totalAllocatedVideoParties) : 0;

			if (!isMPMX)
			{
				numVidHD720Parties = (WORD)floor(min_video_parties - totalAllocatedVideoParties - totalAllocatedAudioParties);
				if (eProductTypeSoftMCUMfw == productType)
					numVidCifParties = (WORD)floor((min_video_parties - totalAllocatedVideoParties - totalAllocatedAudioParties) / m_logicalHD720WeightSvcParty[eNonMix][e_Cif]); //VNGSWIBM-1616: IBM is working via DMA, which shows resource report in CIF units
				else
					numVidCifParties = (WORD)floor((min_video_parties - totalAllocatedVideoParties - totalAllocatedAudioParties) / m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]);
				free_hd_ports = free_hd_ports - totalAllocatedAudioParties;
			}
			else
			{
				numVidHD720Parties = (min_video_parties >= totalAllocatedVideoParties) ? (WORD)floor(min_video_parties - totalAllocatedVideoParties) : 0;
				numVidCifParties = (WORD)floor((min_video_parties - totalAllocatedVideoParties) / m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]);
			}

			float ppm_f = 0;

			if (isMPMX)
			{
				ppm_f = ceil(totalAllocatedVideoParties) - totalAllocatedVideoParties;
			}
			else
			{
				ppm_f = ceil(totalAllocatedVideoParties + totalAllocatedAudioParties) - totalAllocatedVideoParties - totalAllocatedAudioParties;
			}

			ppm = (DWORD)(ppm_f < MIN_PPM_FLOAT_VALUE ? 0 : (ppm_f*PARTS_PER_MILLION));

			numAudParties = (min_audio_parties >= totalAllocatedAudioParties) ? (WORD)floor(min_audio_parties - totalAllocatedAudioParties) : 0;
			break;
		}
		default:
		{
			PASSERT(1);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::CountConfResourceReport(CSharedRsrcConfReport* pReport, CConfRsrc* pConfRsrc)
{
	const std::set<CPartyRsrc>* pPartyRsrcList = pConfRsrc ? pConfRsrc->GetPartiesList() : NULL;
	if (!pPartyRsrcList || !pReport)
		return;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN(!pSystemResources, "Failed, pSystemResources is NULL");
	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());

	WORD numAudParties = 0, numVidCifParties = 0, numVidHDParties = 0;
	std::set<CPartyRsrc>::iterator partyRsrcItr;
	float num_hd720_vid_ports = 0;
	for (partyRsrcItr = pPartyRsrcList->begin(); partyRsrcItr != pPartyRsrcList->end(); partyRsrcItr++)
	{
		CPartyRsrc* pPartyRsrc = ((CPartyRsrc*)(&(*partyRsrcItr)));
		eVideoPartyType partyType = pPartyRsrc->GetVideoPartyType();
		if (isMPMX && (CHelperFuncs::IsAudioParty(partyType) || CHelperFuncs::IsAudioContentParty(partyType)))
			numAudParties++;
		else
		{
			eConfModeTypes confModeType = (eMediaStateMixAvcSvc == pConfRsrc->GetConfMediaState()) ? eMix : eNonMix;
			num_hd720_vid_ports += GetLogicalWeightForResourceCalculations(partyType, pPartyRsrc->GetPartyRole(), confModeType);
		}
	}

	numVidCifParties = (WORD)ceil(num_hd720_vid_ports / m_LogicalHD720WeightAvcParty[eNonMix][e_Cif]);
	numVidHDParties  = (WORD)ceil(num_hd720_vid_ports);
	float ppm_f = ((float)numVidHDParties) - num_hd720_vid_ports;
	DWORD ppm = (DWORD)(ppm_f < MIN_PPM_FLOAT_VALUE ? 0 : (ppm_f * PARTS_PER_MILLION));
	TRACEINTO << "NumAudParties:" << numAudParties << ", NumVidCifParties:" << numVidCifParties << ", Num_hd720_vid_ports:" << num_hd720_vid_ports << ", Ppm:" << ppm;
	pReport->SetNumPartiesByConfID(pConfRsrc->GetMonitorConfId(), e_Audio, TYPE_OCCUPIED, numAudParties);
	pReport->SetNumPartiesByConfID(pConfRsrc->GetMonitorConfId(), e_Cif, TYPE_OCCUPIED, numVidCifParties);
	pReport->SetNumPartiesByConfID(pConfRsrc->GetMonitorConfId(), e_HD720, TYPE_OCCUPIED, numVidHDParties);

	pReport->SetAvailablePortionPPM(ppm);
}
//////////////////////////////////////////////////////////////////////////////////
BOOL CAutoModeResources::CheckVideoUnitsWithConfig(const DWORD numUnitsPerBoard[BOARDS_NUM]) const
{
	//always true, because we are making all configurations according to CIFs, who are less than a full unit
	return TRUE;
}
//////////////////////////////////////////////////////////////////////////////////
float CAutoModeResources::GetLogicalWeightForResourceCalculations(eVideoPartyType videoPartyType, ePartyRole partyRole, eConfModeTypes confModeType)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	eProductType productType = pSystemResources ? pSystemResources->GetProductType() : eProductTypeUnknown;

	if (eCOP_party_type == videoPartyType) //or CHelperFuncs::IsMode2C()
	{
		return m_logicalHD720WeightCopParty;
	}
	else if (CHelperFuncs::IsVideoRelayParty(videoPartyType) || eVoice_relay_party_type == videoPartyType)
	{
		ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(videoPartyType);

		// MFW should be differentiated by logical port weight
		if (eProductTypeSoftMCUMfw == productType && CHelperFuncs::IsVswRelayParty(videoPartyType))
		{
			return m_LogicalHD720WeightAvcParty[confModeType][partyResourceType];
		}
		else
		{
			return m_logicalHD720WeightSvcParty[confModeType][partyResourceType];
		}
	}
	else
	{
		float fLicenseFactor = GetLicenseFactorAccordingToPartyRole(videoPartyType, partyRole);
		ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(videoPartyType);

		return (fLicenseFactor * m_LogicalHD720WeightAvcParty[confModeType][partyResourceType]);
	}
}
//////////////////////////////////////////////////////////////////////////////////
float CAutoModeResources::GetLicenseFactorAccordingToPartyRole(eVideoPartyType videoPartyType, ePartyRole partyRole)
{
	// In some cases we need to allocate only encoder or decoder, for example in content transcoding.
	// 1. License factor of 1 originally used for allocating both encoder + decoder.
	// 2. License factor of 0.5 used for allocating only a decoder.
	// 3. License factor of 0.5 used for allocating only an encoder, in case we can allocate another encoder on the same unit,
	//    or the encoder occupies a full unit/accelerator (for example HD1080p30 encoder in MPMx).

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERTMSG_AND_RETURN_VALUE(!pSystemResources, "Failed, pSystemResources is NULL", 1);

	float fLicenseFactor = 1;

	eSystemCardsMode systemCardsMode = pSystemResources->GetSystemCardsMode();
	BOOL isSoftMCU = (eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily() && eProductTypeNinja != pSystemResources->GetProductType());
	BOOL isMPMX = (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily() && eSystemCardsMode_breeze == pSystemResources->GetSystemCardsMode());

	switch (partyRole)
	{
		case eParty_Role_regular_party:
		{
			if (eCP_Content_for_Legacy_Decoder_HD1080_video_party_type == videoPartyType)
			{
				fLicenseFactor = 0.5;
			}
			else
			{
				fLicenseFactor = 1;
			}
			break;
		}

		case eParty_Role_content_decoder:
		{
			fLicenseFactor = 0.5;
			break;
		}

		case eParty_Role_content_encoder:
		{
			if (eSystemCardsMode_breeze == systemCardsMode)
			{
				if (eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type == videoPartyType)
				{
					fLicenseFactor = 0.5;
				}
				else
				{
					fLicenseFactor = 1;
				}
			}
			else if (eSystemCardsMode_mpmrx == systemCardsMode)
			{
				if (eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type == videoPartyType || eCP_H264_upto_HD720_30FS_Symmetric_video_party_type == videoPartyType)
				{
					fLicenseFactor = 0.5;
				}
				else
				{
					fLicenseFactor = 1;
				}
			}
			break;
		}

			// HD slave in: factor is 0.5 for all platforms.
			// SD slave in: factor is 0.5 for SoftMCU, 0.6666 for MPMx, MPM-Rx and Ninja (0.6666 x 0.5 = 0.3333)
			// CIF slave in: factor is 1 for MPMx and 0.5 for SoftMCU, 0.6666 for MPM-Rx and Ninja (0.6666 x 0.5 = 0.3333)
		case eParty_Role_AvMcuLink_SlaveIn:
		{
			if (eCP_H264_upto_HD720_30FS_Symmetric_video_party_type == videoPartyType)
			{
				fLicenseFactor = LICENSE_FACTOR_SLAVE_IN_HD;
			}
			else
			{
				if (isSoftMCU)
				{
					fLicenseFactor = LICENSE_FACTOR_SLAVE_IN_CIF_SD_SOFT;
				}
				else if (isMPMX)
				{
					if (eCP_H264_upto_CIF_video_party_type == videoPartyType) // CIF MPMx
					{
						fLicenseFactor = LICENSE_FACTOR_SLAVE_IN_CIF_MPMX;
					}
					else // SD MPMx
					{
						fLicenseFactor = LICENSE_FACTOR_SLAVE_IN_CIF_SD;
					}
				}
				else // MPM-Rx, Ninja
				{
					fLicenseFactor = LICENSE_FACTOR_SLAVE_IN_CIF_SD;
				}
			}
			break;
		}

			// CIF/SD slave out: factor is 1 for MPMx and 0.5 for SoftMCU, 0.6666 for MPM-Rx and Ninja (0.6666 x 0.5 = 0.3333)
			// HD slave out: factor is 0.5 for MPMx, and 1 for the rest
		case eParty_Role_AvMcuLink_SlaveOut:
		{
			if (eCP_H264_upto_HD720_30FS_Symmetric_video_party_type == videoPartyType)
			{
				fLicenseFactor = (isMPMX) ? LICENSE_FACTOR_SLAVE_OUT_HD_MPMX : LICENSE_FACTOR_SLAVE_OUT_HD;
			}
			else
			{
				if (isMPMX)
					fLicenseFactor = LICENSE_FACTOR_SLAVE_OUT_CIF_SD_MPMX;
				else if (isSoftMCU)
					fLicenseFactor = LICENSE_FACTOR_SLAVE_OUT_CIF_SD_SOFT;
				else
					fLicenseFactor = LICENSE_FACTOR_SLAVE_OUT_CIF_SD_MPMRX_NINJA;
			}
			break;
		}

		default:
			break;
	}

	return fLicenseFactor;
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::AddPartyToMatrix(eVideoPartyType videoPartyType, ePartyRole partyRole, eConfModeTypes confModeType, WORD reqBoardId)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	eProductType productType = pSystemResources ? pSystemResources->GetProductType() : eProductTypeUnknown;
	bool bIncludeRelayVSW = eProductTypeSoftMCUMfw == productType ? false : true;

	ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(videoPartyType);
	float fLicenseFactor = GetLicenseFactorAccordingToPartyRole(videoPartyType, partyRole);

	if (CHelperFuncs::IsVideoRelayParty(videoPartyType, bIncludeRelayVSW) || eVoice_relay_party_type == videoPartyType)
	{
		TRACEINTO <<
				"VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] <<
				", ConfType:" << confModeType <<
				", NumCurrentSVCParties:" << m_numCurrentSVCParties[reqBoardId][confModeType][partyResourceType] <<
				", fLicenseFactor:" << fLicenseFactor;
		m_numCurrentSVCParties[reqBoardId][confModeType][partyResourceType] += fLicenseFactor;
	}
	else
	{
		TRACEINTO <<
				"VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] <<
				", ConfType:" << confModeType <<
				", NumCurrentAVCParties:" << m_numCurrentAVCParties[reqBoardId][confModeType][partyResourceType] <<
				", fLicenseFactor:" << fLicenseFactor;
		m_numCurrentAVCParties[reqBoardId][confModeType][partyResourceType] += fLicenseFactor;
	}
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::RemovePartyFromMatrix(eVideoPartyType videoPartyType, ePartyRole partyRole, eConfModeTypes confModeType, WORD reqBoardId)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

	eProductType productType = pSystemResources ? pSystemResources->GetProductType() : eProductTypeUnknown;
	bool bIncludeRelayVSW = eProductTypeSoftMCUMfw == productType ? false : true;

	ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(videoPartyType);
	float fLicenseFactor = GetLicenseFactorAccordingToPartyRole(videoPartyType, partyRole);

	float *pNumPartiesToUpdate = NULL;

	if (CHelperFuncs::IsVideoRelayParty(videoPartyType, bIncludeRelayVSW) || eVoice_relay_party_type == videoPartyType)
	{
		pNumPartiesToUpdate = &m_numCurrentSVCParties[reqBoardId][confModeType][partyResourceType];
	}
	else
	{
		pNumPartiesToUpdate = &m_numCurrentAVCParties[reqBoardId][confModeType][partyResourceType];
	}

	if (*pNumPartiesToUpdate >= fLicenseFactor)
	{
		*pNumPartiesToUpdate -= fLicenseFactor;

		if (*pNumPartiesToUpdate < SMALL_ERROR)
			*pNumPartiesToUpdate = 0;
	}
	else
	{
		if ((fLicenseFactor - *pNumPartiesToUpdate) < SMALL_ERROR)
		{
			*pNumPartiesToUpdate = 0;
		}
		else
		{
			TRACEINTOLVLERR << "VideoPartyType:" << videoPartyType << ", PartyRole:" << partyRole << ", ConfModeType:" << confModeType << ", ReqBoardId:" << reqBoardId << ", LicenseFactor:" << fLicenseFactor << ", NumPartiesToUpdate:" << *pNumPartiesToUpdate;
			*pNumPartiesToUpdate = 0;
			DBGPASSERT(1);
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::OnLicenseExpired()
{
	m_audVidConfig.SetAudio(0);
	m_audVidConfig.SetVideo(0);
}
//////////////////////////////////////////////////////////////////////////////////
void CAutoModeResources::GetLogicalPartyWeight(eVideoPartyType videoPartyType, float& logicalPartyWeightInNonMix, float& logicalPartyWeightInMix)
{
	ePartyResourceTypes partyResourceType = VideoPartyTypeToPartyResourceType(videoPartyType);

	if (CHelperFuncs::IsVideoRelayParty(videoPartyType, false))
	{
		logicalPartyWeightInNonMix = m_logicalHD720WeightSvcParty[eNonMix][partyResourceType];
		logicalPartyWeightInMix = m_logicalHD720WeightSvcParty[eMix][partyResourceType];
	}
	else
	{
		logicalPartyWeightInNonMix = m_LogicalHD720WeightAvcParty[eNonMix][partyResourceType];
		logicalPartyWeightInMix = m_LogicalHD720WeightAvcParty[eMix][partyResourceType];
	}
}

#undef TRACEINTO
#define TRACEINTO TRACESTRFUNC(eLevelInfoNormal)

