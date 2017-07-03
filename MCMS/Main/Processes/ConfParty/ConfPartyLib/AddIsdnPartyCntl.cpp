#include "NStream.h"
#include "AddIsdnPartyCntl.h"
#include "PartyApi.h"
#include "Conf.h"
#include "ObjString.h"
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "BridgePartyInitParams.h"
#include "BridgePartyAudioParams.h"
#include "StatusesGeneral.h"
#include "ConfPartySharedDefines.h"
#include "BridgeMoveParams.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "TraceStream.h"
#include "IsdnNetSetup.h"
#include "IsdnVideoPartyOut.h"
#include "SysConfigKeys.h"
#include "H263VideoMode.h"
#include "IpCommon.h"

extern "C" void PartyVideoOutEntryPoint(void* appParam);
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);

#define SETUP_TOUT             10*SECOND     // Allocate + create + connect audio // isdn_encryption valgrind : nothing
#define BONDING_INIT_TOUT      15*SECOND     // bonding
#define PARTY_SETUP_TOUT       10*SECOND     // end init comm
#define AUDIO_NEGOTIATION_TOUT 30*SECOND     // audio negotiation // isdn_encryption valgrind : 20->30

// CAddIsdnPartyCntl states
const WORD ALLOCATE        = 1;              // waiting for resource allocator response
const WORD CREATE          = 2;              // waiting for create ack from mpl
const WORD CONNECTAUDIO    = 3;              // waiting for connect ack from both audio bridge and MPL
const WORD PARTY_CTL_SETUP = 4;              // party was asked to establish call
const WORD CHANGE_AUDIO    = 5;              // received EndInitComm from party, waiting for audio negotiation
const WORD AUDIO_CONNECTED = 6;              // audio connection has been established

// IDLE -> ALLOCATE -> CREATE -> CONNECTAUDIO -> PARTY_CTL_SETUP -> CHANGE_AUDIO -> AUDIO_CONNECTED

PBEGIN_MESSAGE_MAP(CAddIsdnPartyCntl)

	ONEVENT(ALLOCATE_PARTY_RSRC_IND,          ALLOCATE,           CAddIsdnPartyCntl::OnRsrcAllocatePartyRspAllocate)
	ONEVENT(PARTY_AUDIO_CONNECTED,            CONNECTAUDIO,       CAddIsdnPartyCntl::OnAudConnectConnectAudio)
	ONEVENT(ACK_IND,                          CONNECTAUDIO,       CAddIsdnPartyCntl::OnMplAckConnectAudio)
	ONEVENT(ACK_IND,                          CREATE,             CAddIsdnPartyCntl::OnMplAckCreate)

	// party events
	ONEVENT(PARTYENDINITCOM,                  PARTY_CTL_SETUP,    CAddIsdnPartyCntl::OnPartyEndInitComSetup)
	ONEVENT(UPDATENETCHANNEL,                 PARTY_CTL_SETUP,    CAddIsdnPartyCntl::OnPartyUpdateRTMChannel)
	ONEVENT(REALLOCATERTM,                    PARTY_CTL_SETUP,    CAddIsdnPartyCntl::OnPartyReAllocateRTM)
	ONEVENT(PARTYCONNECT,                     CHANGE_AUDIO,       CAddIsdnPartyCntl::OnPartyConnectChangeAudio)
	ONEVENT(PARTYCONNECT,                     ANYCASE,            CAddIsdnPartyCntl::OnPartyConnectAnycase)
	ONEVENT(ALLRMTCAPSRECEIVED,               ANYCASE,            CAddIsdnPartyCntl::OnPartyReceivedFullCapSet)
	ONEVENT(REALLOCATERTM,                    CHANGE_AUDIO,       CAddIsdnPartyCntl::OnPartyReAllocateRTM)
	ONEVENT(BOARDFULL,                        PARTY_CTL_SETUP,    CAddIsdnPartyCntl::OnPartyBoardFull)
	ONEVENT(REALLOCATE_RTM_ON_BOARD_FULL_IND, PARTY_CTL_SETUP,    CAddIsdnPartyCntl::OnRsrcReallocateBoardFull)

	ONEVENT(UPDATE_RTM_PORT_IND,              ANYCASE,            CAddIsdnPartyCntl::OnRsrcAllocatorUpdateRTMChannelAck)
	ONEVENT(CONNECTDELAY,                     IDLE,               CAddIsdnPartyCntl::OnPartyConnectDelayIdle)

	// self timers
	ONEVENT(CONNECTTOUT,                      ALLOCATE,           CAddIsdnPartyCntl::OnConnectToutAllocate)
	ONEVENT(CONNECTTOUT,                      CREATE,             CAddIsdnPartyCntl::OnConnectToutCreate)
	ONEVENT(CONNECTTOUT,                      CONNECTAUDIO,       CAddIsdnPartyCntl::OnConnectToutConnectAudio)
	ONEVENT(CONNECTTOUT,                      PARTY_CTL_SETUP,    CAddIsdnPartyCntl::OnConnectToutSetup)
	ONEVENT(CONNECTTOUT,                      CHANGE_AUDIO,       CAddIsdnPartyCntl::OnConnectToutChangeAudio)
	ONEVENT(CONNECTTOUT,                      AUDIO_CONNECTED,    CAddIsdnPartyCntl::OnConnectToutAudioConnected)
	ONEVENT(WAIT_FOR_RSRC_AND_ASK_AGAIN,      ALLOCATE,           CAddIsdnPartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate)

PEND_MESSAGE_MAP(CAddIsdnPartyCntl, CIsdnPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CAddIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
CAddIsdnPartyCntl::CAddIsdnPartyCntl()
{
	m_isNetAudioConnected               = FALSE;
	m_isUnMuted                         = FALSE;
	m_isIncomingAudioMutedByAudioBridge = FALSE;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CAddIsdnPartyCntl::~CAddIsdnPartyCntl()
{
}

//--------------------------------------------------------------------------
const char* CAddIsdnPartyCntl::NameOf()  const
{
	return "CAddIsdnPartyCntl";
}

//--------------------------------------------------------------------------
void* CAddIsdnPartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
CAddIsdnPartyCntl& CAddIsdnPartyCntl::operator =(const CAddIsdnPartyCntl& other)
{
	(CIsdnPartyCntl&)*this = (CIsdnPartyCntl&)other;
	return *this;
}
//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::Create(CConf* pConf, CIsdnNetSetup* pNetSetUp, CCapH320* pCap, CComMode* pScm,
                               CComMode* pTransmitScm, COsQueue* pConfRcvMbx, CAudioBridgeInterface* pAudioBridgeInterface,
                               CVideoBridgeInterface* pVideoBridgeInterface, CConfAppMngrInterface* pConfAppMngrInterface,
                               CFECCBridge* pFECCBridge, CContentBridge* pContentBridge, CTerminalNumberingManager* pTerminalNumberingManager,
                               COsQueue* pPartyRcvMbx, CTaskApp* pParty, WORD termNum, BYTE chnlWidth, WORD numChnl, WORD type,
                               const char* partyName, const char* confName, DWORD monitorConfId, DWORD monitorPartyId,
                               const char* service_provider, ENetworkType networkType, WORD nodeType,
                               BYTE voice, WORD stby, DWORD connectDelay, eTelePresencePartyType eTelePresenceMode, eSubCPtype bySubCPtype, WORD isUndefParty)
{
	m_state               = IDLE;
	m_pLocalCap           = new CCapH320;
	m_pRemoteCap          = new CCapH320;
	m_pConfScm            = new CComMode;
	m_pCurrentTransmitScm = new CComMode; // MCU --> EP currently open
	m_pCurrentReceiveScm  = new CComMode; // EP --> MCU currently open
	m_pTargetTransmitScm  = new CComMode; // the target scm we would like to transmit to  EP after change mode ends
	m_pTargetReceiveScm   = new CComMode; // the target scm we would like to receive from EP after change mode ends

	// begin: Initialization of CPartyCntl attributes
	m_pConf               = pConf;
	m_netSetUp            = *pNetSetUp;
	*m_pLocalCap          = *pCap;
	*m_pConfScm           = *pScm;
	*m_pTargetTransmitScm = *pTransmitScm;
	*m_pTargetReceiveScm  = *pScm;

	TRACESTRFUNC(eLevelInfoNormal) << "PartyName:" << partyName << ",  m_pLocalCap is:";
	m_pLocalCap->Dump();

	TRACESTRFUNC(eLevelInfoNormal) << "PartyName:" << partyName << ",  m_pTargetTransmitScm is:";
	ostringstream strScm;
	m_pTargetTransmitScm->Dump(strScm);

	VideoCanBeOpened(m_pTargetTransmitScm);

	// init bridges
	m_pAudioInterface           = pAudioBridgeInterface;
	m_pVideoBridgeInterface     = pVideoBridgeInterface;
	m_pConfAppMngrInterface     = pConfAppMngrInterface;
	m_pFECCBridge               = pFECCBridge;
	m_pTerminalNumberingManager = pTerminalNumberingManager;
	m_pContentBridge            = pContentBridge;
	m_pBridgeMoveParams         = new CBridgeMoveParams;
	m_pPartyApi                 = NULL;
	m_pParty                    = pParty;
	m_termNum                   = termNum;
	m_chnlWidth                 = chnlWidth;
	m_numChnl                   = numChnl;
	m_type                      = type;
	m_monitorConfId             = monitorConfId;
	m_monitorPartyId            = monitorPartyId;
	m_networkType               = networkType;
	m_nodeType                  = nodeType;
	m_voice                     = voice;
	m_connectDelay              = connectDelay;
	m_isUndefParty              = isUndefParty;
	m_IsGateWay                 = pConf->GetIsGateWay();
	m_eTelePresenceMode         = eTelePresenceMode;
	m_telepresenseEPInfo->SetEPtype(eTelePresenceMode);

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pTaskApi = new CConfApi(pConf->GetMonitorConfId());
#else
	m_pTaskApi = new CConfApi;
#endif
	m_pTaskApi->CreateOnlyApi(pConf->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx(pConf->GetLocalQueue());

	m_service_provider = new char[NET_SERVICE_PROVIDER_NAME_LEN];
	strcpy_safe(m_service_provider, NET_SERVICE_PROVIDER_NAME_LEN, service_provider);

	m_name = new char[H243_NAME_LEN];
	strcpy_safe(m_name, H243_NAME_LEN, partyName);

	SetFullName(partyName, confName);

	// for version 7.0.2 - depends on system flag SUPPORT_HIGH_PROFILE_WITH_ISDN
	BOOL bEnableHighfProfileInIsdn = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE_WITH_ISDN);
	if (IsFeatureSupportedBySystem(eFeatureH264HighProfile) && !bEnableHighfProfileInIsdn)
		UpdateH264BaseProfileTresholdInLocalCaps();

	switch (m_type)
	{
		case DIALIN:
		{
			m_pPartyApi = new CPartyApi;
			m_pPartyApi->CreateOnlyApi(*pPartyRcvMbx);
			StartPartyConnection();
			break;
		}

		case DIALOUT:
		{
			if (!stby)
			{
				DialOut();
			}
			else // connect and stand by
			{
#ifdef LOOKUP_TABLE_DEBUG_TRACE
				TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
				m_pParty = (CTaskApp*)(m_monitorPartyId + 100); // just to get unique id for party list
				m_disconnectState = DISCONNECTED;
				POBJDELETE(m_pPartyApi);
				m_pTaskApi->EndAddParty(m_pParty, PARTY_STAND_BY);
			}
			break;
		}

		default:
		{
			TRACESTRFUNC(eLevelError) << "PartyName:" << partyName << ", Type:" << m_type << " - Failed, unknown type";
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::DialOut(WORD redialInterval)
{
	// Check from where this attribute should be initialized.
	if (m_connectDelay)
	{
		TRACEINTO << m_partyConfName << ", Blast, StartTimer(CONNECTDELAY) for " << m_connectDelay  << "ms";
		StartTimer(CONNECTDELAY, m_connectDelay);
	}
	else if (redialInterval)  // Re-dialing
	{
		TRACEINTO << m_partyConfName << ", StartTimer(CONNECTDELAY) for " << m_connectDelay  << "ms";
		StartTimer(CONNECTDELAY, redialInterval);
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_REDIALING);
	}
	else
	{
		TRACEINTO << m_partyConfName;
		StartPartyConnection();
	}
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::StartPartyConnection()
{
	StartTimer(CONNECTTOUT, SETUP_TOUT);

	AllocatePartyResources();
}

//--------------------------------------------------------------------------
// Send ALLOCATE_PARTY_RESOURCES_REQ event to resource allocator process
// and changes state to ALLOCATE.
void CAddIsdnPartyCntl::AllocatePartyResources()
{
	TRACEINTO << m_partyConfName;

	eVideoPartyType videoPartyType = GetVideoPartyTypeAccordingToCapabilities(m_pLocalCap, true);
	if (m_pConfScm->IsFreeVideoRate() == TRUE)
	{
		eVideoPartyType targetTransmitVideoPartyType = m_pTargetTransmitScm->GetCPResourceVideoPartyType();
		if (targetTransmitVideoPartyType > videoPartyType)
			videoPartyType = targetTransmitVideoPartyType;
	}
    
    CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
    PASSERT_AND_RETURN(!pConfParty);
    BYTE protocol = pConfParty->GetVideoProtocol();
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	if (protocol == VIDEO_PROTOCOL_H261 && videoPartyType != eVideo_party_type_none && systemCardsBasedMode == eSystemCardsMode_mpmrx)
	{
		videoPartyType = eCP_H261_CIF_equals_H264_HD1080_video_party_type;
		TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType] << " - Force H261 resolution on mpmRx";
	}
            
	if (IsCpH263_4Cif_SupportedByRateAndVideoQuality() == false)
	{
		TRACEINTO << m_partyConfName << " - Remove H263 4Cif from target Scm";
		RemoveH2634CifFromTargetScm();
	}

	CreateAndSendAllocatePartyResources(eISDN_network_party_type, videoPartyType, eNoAllocationPolicy);
	m_state = ALLOCATE;
}

//--------------------------------------------------------------------------
// Fill the ALLOCATE_PARTY_REQ_PARAMS_S with the party event to resource allocator process
// and changes state to ALLOCATE.
void CAddIsdnPartyCntl::SetPartyTypeRelevantInfo(ALLOC_PARTY_REQ_PARAMS_S& allocatePartyParams)
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();
	PASSERT_AND_RETURN(!pCommConf);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_name);
	PASSERT_AND_RETURN(!pConfParty);

	WORD spanId  = DUMMY_SPAN_ID;
	WORD boardId = DUMMY_BOARD_ID;
	BYTE isBondingTmpPhoneNumberRequired = NO;

	if (DIALIN == m_type)
	{
		boardId = m_netSetUp.m_boardId;
		spanId  = m_netSetUp.m_spanId[0];
		isBondingTmpPhoneNumberRequired = YES;
		Phone* phone = ((CCommConf*)m_pConf->GetCommConf())->GetActualConfPhone((char*)(m_netSetUp.m_called.m_digits), pConfParty->GetServiceProviderName());
		if (phone)
			strcpy_safe(allocatePartyParams.isdn_span_params.conferencePhoneNumber, phone->phone_number);
	}

	// Setting the service name
	strcpy((char*)allocatePartyParams.isdn_span_params.serviceName, pConfParty->GetServiceProviderName());

	// Setting the ISDN span params
	allocatePartyParams.isdn_span_params.span_id = spanId;
	allocatePartyParams.isdn_span_params.board_id = boardId;
	allocatePartyParams.isdn_span_params.num_of_isdn_ports = m_numChnl; // m_numChnl = number of channels from confParty
	allocatePartyParams.isdn_span_params.isBondingTemporaryPhoneNumberNeeded = isBondingTmpPhoneNumberRequired;
}

//--------------------------------------------------------------------------
// Change state to setup
// Deserialize response
// Add info to routing table
// Send Create Party to MFA
void CAddIsdnPartyCntl::OnRsrcAllocatePartyRspAllocate(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	const CCommConf* pCommConf = m_pConf->GetCommConf();
	PASSERT_AND_RETURN(!pCommConf);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_name);
	PASSERT_AND_RETURN(!pConfParty);

	POBJDELETE(m_pPartyAllocatedRsrc);
	m_pPartyAllocatedRsrc = new CIsdnPartyRsrcDesc();
	m_pPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	m_pPartyAllocatedRsrc->DumpToTrace();

	DWORD status = m_pPartyAllocatedRsrc->GetStatus();
	eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();

	if (STATUS_OK != status)
		ClearUnusedPartyId(m_pPartyAllocatedRsrc->GetPartyRsrcId());

	if (STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN == status)
	{
		if (m_isWaitingForRsrcAndAskAgain)
		{
			WaitForRsrcAndAskAgain();
			return;
		}

		PASSERTMSG(1, "STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN, but the m_isWaitingForRsrcAndAskAgain is set to NO");
	}

	WORD num_allocated_channels = ((CIsdnPartyRsrcDesc*)m_pPartyAllocatedRsrc)->GetNumAllocatedChannels();

	if (m_numChnl != num_allocated_channels && STATUS_OK == status)
	{
		TRACEINTO << "ChannelsNumberBeforeAlloc:" << m_numChnl << ", ChannelsNumberAfterAlloc:" << num_allocated_channels;
		m_numChnl = num_allocated_channels;

		m_pConfScm->SetXferMode(0, m_numChnl, 1);
		m_pTargetTransmitScm->SetXferMode(0, m_numChnl, 1);
		m_pTargetReceiveScm->SetXferMode(0, m_numChnl, 1);
	}

	m_eLastAllocatedVideoPartyType = m_pPartyAllocatedRsrc->GetVideoPartyType();
	m_RoomId = m_pPartyAllocatedRsrc->GetRoomId();

	if (m_telepresenseEPInfo)
	{
		 m_telepresenseEPInfo->SetRoomID(m_RoomId);
		 m_telepresenseEPInfo->SetPartyMonitorID(m_monitorPartyId);
	}
	else
	{
		 TRACEINTO <<" - m_telepresenseEPInfo is NULL";
	}

	if (m_type == DIALIN)
	{
		const char* tmpNumber = ((CIsdnPartyRsrcDesc*)m_pPartyAllocatedRsrc)->GetTmpPhoneNumber();
		pConfParty->SetBondingTmpNumber(tmpNumber);
	}

	POBJDELETE(m_pPartyHWInterface);

	if (status != STATUS_OK)
	{
		TRACEINTO << "Failed, Status:" << CProcessBase::GetProcess()->GetStatusAsString(m_pPartyAllocatedRsrc->GetStatus()).c_str();

#ifdef LOOKUP_TABLE_DEBUG_TRACE
		TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
		m_pParty = (CTaskApp*)(m_monitorPartyId + 100); // Get unique party id

		if (DIALIN == m_type)
		{
			PASSERT(!m_pPartyApi);

			if (m_pPartyApi)
			{
				// dial in clear the net cntl
				m_pPartyApi->LogicalDelNetChannel(caus_USER_BUSY_VAL);
				POBJDELETE(m_pPartyApi);
			}
		}

		m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, RESOURCES_DEFICIENCY, 1); // Disconnect cause
		POBJDELETE(m_pPartyAllocatedRsrc);
		m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		if (IsValidTimer(CONNECTTOUT))
			DeleteTimer(CONNECTTOUT);
		return;
	}

	if (m_eLastAllocatedVideoPartyType < m_eLastAllocRequestVideoPartyType)
	{
		// update local caps and target scm according to new allocation
		UpdateH264ModeInLocalCaps();
		// /Asymmetric modes:
		// /1. HD720 30fps Asymmetric
//		if (m_eLastAllocRequestVideoPartyType == eCP_H264_upto_HD720_30FS_Asymmetric_video_party_type)
//			DisableHD720AsymmetricFromTargetScm();
		// /2. HD1080 30fps and HD720 60fps Asymmetric
//		if (m_eLastAllocRequestVideoPartyType == eCP_H264_upto_HD1080_30FS_Asymmetric_video_party_type)
//		{
//			DisableHD1080AsymmetricFromTargetScm();
//			DisableHD720At60AsymmetricFromTargetScm();
//		}

		if (m_eLastAllocRequestVideoPartyType == eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type)
		{
			DisableHD1080At60AsymmetricFromTargetScm();
		}

		// /3. H263 4CIF 15
		if (m_eLastAllocatedVideoPartyType < eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
			RemoveH2634CifFromTargetScm();
	}
	else if (m_eLastAllocatedVideoPartyType > m_eLastAllocRequestVideoPartyType)
	{
		PASSERTSTREAM(1, "Failed, Higher allocation than requested, eLastAllocatedVideoPartyType:" << m_eLastAllocatedVideoPartyType);
	}

	// Create the task in Dialout only after Rsrc Ind
	if (DIALOUT == m_type)
	{
		COsQueue* pConfRcvMbx = (COsQueue*)(&(m_pTaskApi->GetRcvMbx()));
		void (*entryPoint)(void*) = PartyVideoOutEntryPoint;

		m_pPartyApi = new CPartyApi;
		m_pPartyApi->Create(entryPoint,
		                    *pConfRcvMbx,
		                    *pConfRcvMbx,
		                    m_pPartyAllocatedRsrc->GetPartyRsrcId(),
		                    m_monitorPartyId,
		                    m_monitorConfId,
		                    m_pConf->GetCommConf()->GetNumericConfId(),
		                    m_name,
		                    m_pConf->GetName(),
		                    m_serviceId,
		                    m_termNum,
		                    1,
		                    m_password,
		                    m_voice,
		                    m_isChairEnabled,
		                    m_IsGateWay,
		                    FALSE);

		m_pParty = m_pPartyApi->GetTaskAppPtr();
	}

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.K."
			<< "  PartyName:"        << m_name
			<< ", PartyId:"          << m_pParty->GetPartyId() << "(" << m_pPartyAllocatedRsrc->GetPartyRsrcId() << ")"
			<< ", IsDialOut:"        << m_type
			<< ", Pointer:"          << std::hex << m_pParty;
#endif

	// Insert resources to routing table
	InsertPartyResourcesToGlobalRsrcRoutingTbl();

	// we can now send messages to hardware
	m_pPartyHWInterface = new CPartyInterface(m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_pPartyAllocatedRsrc->GetConfRsrcId());
	m_pPartyHWInterface->SetRoomId(m_RoomId);
	m_state = CREATE;

	// ISDN network type (PSTN is set in derived classes)
	SendCreateParty(E_NETWORK_TYPE_ISDN);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnMplAckCreate(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	TRACEINTO << m_partyConfName << ", AckOpcode:" << AckOpcode << ", ack_seq_num:" << ack_seq_num << ", status:" << status;

	PASSERT_AND_RETURN(AckOpcode != CONF_MPL_CREATE_PARTY_REQ);

	if (status == STATUS_OK)
	{
		m_state = CONNECTAUDIO;
		ConnectPartyToAudioBridge();
	}
	else
	{
		PASSERTSTREAM(1, "Failed, Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

		m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		m_isFaulty = 1;   // Invoking KillPort process in RA.
		if (IsValidTimer(CONNECTTOUT))
			DeleteTimer(CONNECTTOUT);
	}
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::SendPartyEstablishCall()
{
	TRACEINTO << m_partyConfName;

	m_state = PARTY_CTL_SETUP;

	DeleteTimer(CONNECTTOUT);

	// connection tout - party setup - 10 seconds
	// (setup of the first channel before bonding init + waiting for end init comm after bonding ends)
	// bonding init - 15 seconds
	// bonding negotiation - 20 + 4 * seconds per channel
	DWORD CONNECTION_TOUT = PARTY_SETUP_TOUT + BONDING_INIT_TOUT +((20+m_numChnl*4)*SECOND); // isdn_encryption nothing
	StartTimer(CONNECTTOUT, CONNECTION_TOUT);

	WORD refMcuNumber      = 1;
	WORD refTerminalNumber = 1;
	if (m_pTerminalNumberingManager)
	{
		m_pTerminalNumberingManager->allocatePartyNumber(m_pParty, refMcuNumber, refTerminalNumber);
		ON(m_isTerminalNumberingConn);
	}
	else
		PASSERTMSG(GetPartyRsrcId(), "Terminal numbering manager not valid");

	CRsrcParams* pNetRsrcParams = new CRsrcParams[m_numChnl];
	for (int i = 0; i < m_numChnl; i++)
		m_pPartyAllocatedRsrc->GetRsrcParams(pNetRsrcParams[i], eLogical_net, i+1);

	CIsdnPartyRsrcDesc* desc                = (CIsdnPartyRsrcDesc*)m_pPartyAllocatedRsrc;
	CIsdnPartyRsrcDesc* pPartyAllocatedRsrc = new CIsdnPartyRsrcDesc(*desc);

	m_RoomId = m_pPartyAllocatedRsrc->GetRoomId();

	m_pPartyApi->EstablishVideoCall(GetPartyRsrcId(), &m_netSetUp, pNetRsrcParams, pPartyAllocatedRsrc, m_numChnl, m_pTargetReceiveScm, m_pLocalCap, refMcuNumber, refTerminalNumber, m_RoomId);

	PDELETEA(pNetRsrcParams)
	POBJDELETE(pPartyAllocatedRsrc);

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTING);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::Reconnect(const char* confName, CCapH320* pCap,
                                  CComMode* pScm, CComMode* pTransmitScm, COsQueue* pConfRcvMbx,
                                  WORD termNum, BYTE isRecording, WORD redialInterval, WORD connectDelay)
{
	TRACEINTO << m_partyConfName;

	m_termNum                     = termNum;
	m_disconnectMode              = 0;
	m_disconnectState             = 0;
	m_isDisconnect                = 0;
	m_isFaulty                    = FALSE;
	m_isRecovery                  = FALSE; // Invoking Recovery process in RA.
	m_connectDelay                = connectDelay;
	m_isWaitingForRsrcAndAskAgain = YES;

	OFF(m_isFullBitRateConnect);
	OFF(m_presentationStreamOutIsUpdated);
	OFF(m_isPartyInConf);
	OFF(m_WaitForEndChangeModeToConnectContent);
	OFF(m_isCDRPartyConnected);
	OFF(m_EpDidNotOpenContent);

	POBJDELETE(m_pTargetTransmitScm);
	m_pTargetTransmitScm  = new CComMode;
	*m_pTargetTransmitScm = *pTransmitScm;

	POBJDELETE(m_pTargetReceiveScm);
	m_pTargetReceiveScm  = new CComMode;
	*m_pTargetReceiveScm = *pScm;

	POBJDELETE(m_pCurrentTransmitScm);
	m_pCurrentTransmitScm = new CComMode;

	POBJDELETE(m_pCurrentReceiveScm);
	m_pCurrentReceiveScm = new CComMode;

	POBJDELETE(m_pLocalCap);
	m_pLocalCap  = new CCapH320;
	*m_pLocalCap = *pCap;

	POBJDELETE(m_pRemoteCap);
	m_pRemoteCap = new CCapH320;

	DialOut(redialInterval);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::ConnectPartyToAudioBridge()
{
	TRACEINTO << m_partyConfName;

	m_eAudBridgeConnState = eBridgeDisconnected;
	ON(m_bIsMemberInAudBridge);

	DWORD audioAlg           = U_Law_OF;
	BOOL  isVideoParticipant = GetVoice() ? FALSE : TRUE;
	BOOL  isVtxSupport       = FALSE;

	CDwordBitMask muteMask;
	muteMask.SetBit(MCMS);

	DWORD audioDecoderCompressedDelay = 0; // default value

	// if (m_telepresenseEPInfo->GetEPtype() != eTelePresencePartyNone)
	if (m_eTelePresenceMode != eTelePresencePartyNone)
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("AUDIO_DECODER_COMPRESSED_DELAY", audioDecoderCompressedDelay);

	/*Initialize AudioIn*/
	CBridgePartyAudioInParams audioDecoderParams;
	audioDecoderParams.InitParams(
	  audioAlg,
	  muteMask,
	  GetAudioVolume(),
	  AUDIO_MONO_NUM_OF_CHANNELS,
	  GetConfAudioSampleRate(),
	  isVideoParticipant,
	  IsAgcOn(),
	  isVtxSupport,
	  GetDialType(),
	  audioDecoderCompressedDelay,
	  GetEchoSuppression(),
	  GetKeyboardSuppression(),
	  GetAutoMuteNoisyParties(),
	  GetAudioClarity(),
	  GetConfSpeakerChangeMode());

	/*Initialize AudioOut*/
	CBridgePartyAudioOutParams audioEncoderParams;
	audioEncoderParams.InitParams(
	  audioAlg,
	  muteMask,
	  GetListeningAudioVolume(),
	  AUDIO_MONO_NUM_OF_CHANNELS,
	  GetConfAudioSampleRate(),
	  isVideoParticipant);

	CBridgePartyInitParams bridgePartyInitParams(
	  m_name,
	  m_pParty,
	  GetPartyRsrcId(),
	  m_RoomId,
	  GetInterfaceType(),
	  &audioDecoderParams,
	  &audioEncoderParams);

	m_pConfAppMngrInterface->ConnectPartyAudio(&bridgePartyInitParams);
	m_isIncomingAudioMutedByAudioBridge = (BYTE)bridgePartyInitParams.GetConfAppParams()->IsMuteIncoming();

	m_eAudBridgeConnState = eSendOpenInAndOut;
	ON(m_bIsMemberInAudBridge);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnMplAckConnectAudio(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	TRACEINTO << m_partyConfName << ", AckOpcode:" << ", ack_seq_num:" << ack_seq_num << ", status:" << status;

	PASSERT_AND_RETURN(AckOpcode != TB_MSG_CONNECT_REQ);

	if (status == STATUS_OK)
	{
		m_isNetAudioConnected = TRUE;

		if (AreTwoDirectionsConnectedToAudioBridge())
			SendPartyEstablishCall();
	}
	else
	{
		PASSERTSTREAM(1, "Failed, Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str());

		m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		if (IsValidTimer(CONNECTTOUT))
			DeleteTimer(CONNECTTOUT);
	}
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnPartyEndInitComSetup(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	m_state = CHANGE_AUDIO;

	DeleteTimer(CONNECTTOUT);
	StartTimer(CONNECTTOUT, AUDIO_NEGOTIATION_TOUT);

	m_pTaskApi->EndInitCom(m_pParty->GetPartyId());
}

//--------------------------------------------------------------------------
// Response to	PARTYCONNECT from party. Receives remote caps in segment.
void CAddIsdnPartyCntl::OnPartyConnectChangeAudio(CSegment* pParam)
{
	CCapH320 partyCapsTemp;
	CComMode partyComModeTemp;

	WORD     status;
	*pParam >> status;

	TRACEINTO << m_partyConfName << ", status:" << status;

	if (status)
	{
		TRACEINTO << "Failed, Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

		WORD status_for_disconnecting = status;
		if (status_for_disconnecting == NO_NET_CONNECTION)
			status_for_disconnecting = statBusy;

		m_pTaskApi->EndAddParty(m_pParty, status_for_disconnecting);
		m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, status);
		if (IsValidTimer(CONNECTTOUT))
			DeleteTimer(CONNECTTOUT);
	}
	else
	{
		partyCapsTemp.DeSerialize(NATIVE, *pParam);
		*m_pRemoteCap = partyCapsTemp;

		TRACEINTO << m_partyConfName << ", New remote caps:";
		m_pRemoteCap->Dump();

		partyComModeTemp.DeSerialize(NATIVE, *pParam);
		*m_pCurrentReceiveScm = partyComModeTemp;

		BYTE isEncryptionSetupDone;
		*pParam >> isEncryptionSetupDone;

		// Because we don't know from the scm which custom parameters remote opened
		// and since we have to tell to the video bridge those parameters
		// we will set them as RemoteVideoMode /\ LocalCap (Eitan 01/2008)
		WORD     IsAutoVidScm    = m_pTargetTransmitScm->m_vidMode.IsAutoVidScm();
		CVidMode RemoteVideoMode = m_pCurrentReceiveScm->m_vidMode;
		CVidMode IntersectVidMode;
		RemoteVideoMode.Intersect(m_pLocalCap, IsAutoVidScm, IntersectVidMode);
		m_pCurrentReceiveScm->SetVidMode(IntersectVidMode);

		TRACEINTO << m_partyConfName << ", New current receive Scm:";
		ostringstream strScm;
		m_pCurrentReceiveScm->Dump(strScm);

		// update the SCM with the bitrate that was actually opened by the EP
		CXferMode ActualXferMode = partyComModeTemp.m_xferMode;
		m_pTargetTransmitScm->SetXferMode(ActualXferMode);
		m_pTargetReceiveScm->SetXferMode(ActualXferMode);
		m_pCurrentTransmitScm->SetXferMode(ActualXferMode);

		CAudMode ActualAudioMode = partyComModeTemp.m_audMode;     // The Actual Audio mode that was opened by EP.
		// update the SCM with the audio mode that was actually opened by the EP
		m_pTargetTransmitScm->SetAudMode(ActualAudioMode);
		m_pTargetReceiveScm->SetAudMode(ActualAudioMode);
		m_pCurrentTransmitScm->SetAudMode(ActualAudioMode);

		BYTE audioalg = m_pCurrentReceiveScm->GetAudMode();
		m_pAudioInterface->UpdateAudioAlgorithm(GetPartyRsrcId(), eMediaInAndOut, audioalg);

		m_state = AUDIO_CONNECTED;

		if (isEncryptionSetupDone)
			UnMuteAndUpdateDB();
	}
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::UnMuteAndUpdateDB()
{
	// UnMute Audio Enc/Dec
	if (!m_isUnMuted)
	{
		EOnOff eMuteOnOff = eOff;
		// check for mute by other parties
		if (m_isIncomingAudioMutedByAudioBridge)
			eMuteOnOff = eOn;

		// unmute MCMS outgoing audio (=> EP)
		m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaOut, eOff, MCMS);
		// update MCMS incoming audio (<= EP)
		m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eMuteOnOff, MCMS);
		// validate the audio anyway
		m_pPartyApi->AudioValidation(TRUE); // send AIA ( "Audio indicate active" )

		m_pTaskApi->UpdateDB(m_pParty, AUDCON, TRUE);
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTED_PARTIALY);

		m_isUnMuted = TRUE;
	}
}

//--------------------------------------------------------------------------
// Error handling: if PARTYCONNECT received from party not in state=CHANGE_AUDIO
void CAddIsdnPartyCntl::OnPartyConnectAnycase(CSegment* pParam)
{
	WORD status;
	*pParam >> status;

	TRACEINTO << m_partyConfName << ", status:" << status;

	if (status)
	{
		TRACEINTO << "Failed, Status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str();

		m_pTaskApi->EndAddParty(m_pParty, status);
		m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, status);
		if (IsValidTimer(CONNECTTOUT))
			DeleteTimer(CONNECTTOUT);
	}
	else
	{
		PASSERTSTREAM(1, "Got connect from party in wrong state, m_state:" << m_state << ", " << m_partyConfName);
	}
}

//--------------------------------------------------------------------------
// Handles remote caps, and reallocate resources according to it.
void CAddIsdnPartyCntl::OnPartyReceivedFullCapSet(CSegment* pParam)
{
	CCapH320 partyCapsTest;
	CComMode partyComModeTest;

	partyCapsTest.DeSerialize(NATIVE, *pParam);
	partyComModeTest.DeSerialize(NATIVE, *pParam);

	*m_pRemoteCap         = partyCapsTest;
	*m_pCurrentReceiveScm = partyComModeTest;

	// Because we don't know from the scm which custom parameters remote opened
	// and since we have to tell to the video bridge those parameters
	// we will set them as RemoteVideoMode /\ LocalCap (Eitan 01/2008)
	WORD     IsAutoVidScm    = m_pTargetTransmitScm->m_vidMode.IsAutoVidScm();
	CVidMode RemoteVideoMode = m_pCurrentReceiveScm->m_vidMode;
	CVidMode IntersectVidMode;
	RemoteVideoMode.Intersect(m_pLocalCap, IsAutoVidScm, IntersectVidMode);
	m_pCurrentReceiveScm->SetVidMode(IntersectVidMode);

	// update the SCM with the bitrate that was actually opened by the EP
	CXferMode ActualXferMode = partyComModeTest.m_xferMode;
	if (!(ActualXferMode == m_pTargetTransmitScm->m_xferMode))
	{
		m_pTargetTransmitScm->SetXferMode(ActualXferMode);
		m_pLocalCap->ResetXferCap();     // reset only Xfer-rate cap and not audio cap
		m_pLocalCap->SetXferRateFromScm(partyComModeTest);
	}

	TRACEINTO << m_partyConfName << ", New remote caps:";
	m_pRemoteCap->Dump();

	TRACEINTO << m_partyConfName << ", New current receive Scm:";
	m_pCurrentReceiveScm->UpdateH221string(1);
	ostringstream strScm;
	m_pCurrentReceiveScm->Dump(strScm);

	UnMuteAndUpdateDB();

	ON(m_isRemoteCapReady);

	if (!ReAllocatePartyResourcesIfNeeded()) // ReAllocation is not needed - End add party
	{
		ON(m_isFullBitRateConnect);
		m_pTaskApi->EndAddParty(m_pParty, statOK);
		if (IsValidTimer(CONNECTTOUT))
			DeleteTimer(CONNECTTOUT);
	}
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::HandleReallocateResponse(BYTE bAllocationFailed, BYTE bLocalCapsChanged)
{
	TRACEINTO << m_partyConfName;

	if (bAllocationFailed == TRUE)
	{
		m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, RESOURCES_DEFICIENCY, 1); // Disconnect cause
		m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		m_isFaulty = 1;
		if (IsValidTimer(CONNECTTOUT))
			DeleteTimer(CONNECTTOUT);
	}
	else
	{
		if (bLocalCapsChanged)
		{
			TRACEINTO << "Send new local caps to party";

			BOOL bModifyLegacyIsdnEPCaps = FALSE;
			CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_ISDN_LEGACY_EP_CLOSE_CONTENT_FORCE_H263, bModifyLegacyIsdnEPCaps);

			// if CFG_MODIFY_LEGACY_ISDN_EP_CAPS is set to YES, modify MCU cap set: remove H.261 cap set,
			// and remove H.264 cap set (if remote has no H.264 caps)
			if (bModifyLegacyIsdnEPCaps)
			{
				if (m_pLocalCap->IsH263() || m_pLocalCap->IsH264() || !m_pLocalCap->GetNSCap()->IsEmpty() ||
				    m_pLocalCap->IsPPCap() || m_pLocalCap->IsH239Cap())
				{
					// remove H.261 from second set
					if (m_pLocalCap->IsH263() || m_pLocalCap->IsH264())
					{
						TRACEINTO << "Remove H261 from local cap set";
						m_pLocalCap->RemoveH261Caps();
					}

					// remove H.264 caps from local cap set if remote does not have H.264 caps
					if (!m_pRemoteCap->IsH264())
					{
						TRACEINTO << "Remove H264 from local cap set";
						m_pLocalCap->RemoveH264Caps();
					}

					if (!m_pRemoteCap->IsH239Cap())
					{
						TRACEINTO << "Remove H239 caps before opening the video (Remote does not have H239 caps)";
						m_pLocalCap->RemoveH239Caps();
					}
				}
			}

			// Because we don't know from the scm which custom parameters remote opened
			// and since we have to tell to the video bridge those parameters
			// we will set them as RemoteVideoMode /\ LocalCap (Eitan 01/2008)
			WORD     IsAutoVidScm    = m_pTargetTransmitScm->m_vidMode.IsAutoVidScm();
			CVidMode RemoteVideoMode = m_pCurrentReceiveScm->m_vidMode;
			CVidMode IntersectVidMode;
			RemoteVideoMode.Intersect(m_pLocalCap, IsAutoVidScm, IntersectVidMode);
			m_pCurrentReceiveScm->SetVidMode(IntersectVidMode);

			// update the party about our new capabilities.
			m_pPartyApi->ExchangeCap(m_pLocalCap);
		}

		ON(m_isFullBitRateConnect);
		m_pTaskApi->EndAddParty(m_pParty, statOK);
		if (IsValidTimer(CONNECTTOUT))
			DeleteTimer(CONNECTTOUT);
	}
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnAudConnectConnectAudio(CSegment* pParam)
{
	PASSERTSTREAM_AND_RETURN(m_eAudBridgeConnState == eBridgeDisconnected, "Connect has received after disconnect, PartyId:" << GetPartyRsrcId());

	HandleAudioBridgeConnectedInd(pParam);
	TRACEINTO << m_partyConfName << ", Audio Connected Successfully";
	SendPartyEstablishCall();
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnPartyConnectDelayIdle(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	StartPartyConnection();
}

//--------------------------------------------------------------------------
// Send to RsrcAlloc update with actual spanID, boardId
void CAddIsdnPartyCntl::OnPartyUpdateRTMChannel(CSegment* pParam)
{
	BYTE  board_id      = 0;
	DWORD span_id       = 0;
	WORD  num_channel   = 0;
	DWORD connection_id = 0; // DIAL-IN should 0xffffffff

	*pParam >> board_id >> span_id >> connection_id >> num_channel;

	TRACEINTO << m_partyConfName
	          << ", board_id:"      << (DWORD)board_id
	          << ", span_id:"       << span_id
	          << ", connection_id:" << connection_id
	          << ", channel_num:"   << num_channel;

	// update the connectionId - updated

	UPDATE_ISDN_PORT_S updateIsdnPort;
	memset(&updateIsdnPort, 0, sizeof(UPDATE_ISDN_PORT_S));

	updateIsdnPort.monitor_conf_id  = m_monitorConfId;
	updateIsdnPort.monitor_party_id = m_monitorPartyId;
	updateIsdnPort.board_id         = board_id;
	updateIsdnPort.span_id          = span_id;
	updateIsdnPort.connection_id    = connection_id;
	updateIsdnPort.channel_index    = num_channel;

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&updateIsdnPort), sizeof(UPDATE_ISDN_PORT_S));

	STATUS res = SendReqToResourceAllocator(seg, UPDATE_RTM_PORT_REQ);
	PASSERT(res);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::AddNetConnectionIdToRsrcRoutingTable(DWORD net_connection_id)
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTbl);

	CPartyRsrcRoutingTblKey routingKey = CPartyRsrcRoutingTblKey(net_connection_id, m_pPartyAllocatedRsrc->GetPartyRsrcId(), eLogical_net);
	pRoutingTbl->AddPartyRsrcDesc(routingKey);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnRsrcAllocatorUpdateRTMChannelAck(CSegment* pParam)
{
	DWORD          monitor_conf_id  = 0;
	DWORD          monitor_party_id = 0;
	DWORD          connection_id    = 0;
	DWORD          status           = 0;
	DWORD          num_channel      = 0;
	CIsdnNetSetup* pTempNetSetup    = &m_netSetUp;

	*pParam >> monitor_conf_id >> monitor_party_id >> connection_id >> status >> num_channel;

	TRACEINTO << m_partyConfName
	          << ", monitor_conf_id:"  << monitor_conf_id
	          << ", monitor_party_id:" << monitor_party_id
	          << ", connection_id:"    << connection_id
	          << ", channel_num:"      << num_channel
	          << ", status:"           << status;
	if (status != STATUS_OK)
	{
	    DBGPASSERT(status);
		m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		m_isFaulty = 1; // Invoking KillPort process in RA
		return;
	}

	// in dial in we send AddChannel to party with the parameters from RsrcAlloc + the netSetup we recieved from conf
	if (m_type == DIALIN)
	{
		ADDITIONAL_CHANNELS::iterator it = additionalChannels.find(num_channel);
		if (it != additionalChannels.end())
		{
			pTempNetSetup = &it->second;
		}
		else
		{
			PASSERTSTREAM(1, "Can't find additional channel, channel_num:" << num_channel);
		}

		AddNetConnectionIdToRsrcRoutingTable(connection_id); // add task api to routing table
	}

	m_pPartyApi->OnRsrcAllocatorUpdateRTMChannelAck(m_pPartyAllocatedRsrc->GetConfRsrcId(), m_pPartyAllocatedRsrc->GetPartyRsrcId(), connection_id, status, num_channel, pTempNetSetup);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnPartyReAllocateRTM(CSegment* pParam)
{
	DWORD num_of_ports = 0;
	*pParam >> num_of_ports;

	std::ostringstream msg;
	msg << m_partyConfName;
	msg << "\n  num_of_ports                :" << num_of_ports;

	m_numChnl = num_of_ports;

	eVideoPartyType videoPartyType;
	eVideoQuality   vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
	DWORD           callRate   = m_numChnl * rate64K;
	if (1 == num_of_ports)
		videoPartyType = eVideo_party_type_none;
	else
	{
		if (m_pTargetTransmitScm->GetVidMode() == H264)
		{
			// Get the updated video party type according to the new rate
			DWORD              callRate          = m_numChnl * rate64K;
			CH264VideoMode*    pH264VidMode      = new CH264VideoMode();
			Eh264VideoModeType sysMaxVideoMode   = GetMaxVideoModeBySysCfg(); // max video mode according to system.cfg
			Eh264VideoModeType h264VideoModeType = pH264VidMode->GetH264VideoMode(callRate, vidQuality, sysMaxVideoMode);

			videoPartyType = CResRsrcCalculator::TranslateVideoTypeToResourceType(GetSystemCardsBasedMode(), h264VideoModeType);

			eVideoPartyType origAllocatedVideoPartyType = GetVideoPartyTypeAccordingToCapabilities(m_pLocalCap, true);
			if ((int)videoPartyType > (int)origAllocatedVideoPartyType)
				videoPartyType = origAllocatedVideoPartyType;

			POBJDELETE(pH264VidMode);
			msg << "\n  TargetTransmitScmVidMode    :H264";
		}
		else
		{
			BYTE is4CIF = NO;
			if (m_pTargetTransmitScm->GetVidMode() == H263)
			{
				APIS8 h263_4CifMpi = CH263VideoMode::GetH263Cif4VideoCardMPI(callRate*10 - 1, vidQuality);
				if (h263_4CifMpi == 2)
					is4CIF = YES;

				msg << "\n  TargetTransmitScmVidMode    :H263, H263Cif4MPI:" << h263_4CifMpi << ", is4CIF:" << (int)is4CIF;
			}

			videoPartyType = GetH261H263ResourcesPartyType(is4CIF);
		}
	}

	msg << "\n  VideoPartyType              :" << eVideoPartyTypeNames[videoPartyType]<< " (" << videoPartyType << ")";
	msg << "\n  LastAllocatedVideoPartyType :" << eVideoPartyTypeNames[m_eLastAllocatedVideoPartyType]<< " (" << m_eLastAllocatedVideoPartyType << ")";

	TRACEINTO << msg.str().c_str();

	if ((int)videoPartyType > (int)m_eLastAllocatedVideoPartyType)
	{
		PASSERTMSG(1, "New party type is greater than then allocated party type");
		videoPartyType = m_eLastAllocatedVideoPartyType;
	}

    
    
	ReAllocateRtmRequest = true;
	CreateAndSendReAllocatePartyResources(eISDN_network_party_type, videoPartyType, eNoAllocationPolicy, TRUE);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnRsrcAllocatorUpdateReAllocateRTMAck(CSegment* pParam)
{
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnPartyBoardFull(CSegment* pParam)
{
	DWORD connection_id = 0;

	*pParam >> connection_id;

	BOARD_FULL_REQ_PARAMS_S allocatePartyParams;
	memset(&allocatePartyParams, 0, sizeof(BOARD_FULL_REQ_PARAMS_S));

	for (int i = 0; i < NUM_E1_PORTS; i++)
		allocatePartyParams.connectionIdList[i] = (0 == i) ? connection_id : 0;

	allocatePartyParams.allocPartyReqParams.monitor_conf_id  = m_monitorConfId;
	allocatePartyParams.allocPartyReqParams.monitor_party_id = m_monitorPartyId;
	allocatePartyParams.allocPartyReqParams.networkPartyType = eISDN_network_party_type;
	allocatePartyParams.allocPartyReqParams.videoPartyType   = m_pPartyAllocatedRsrc->GetVideoPartyType(); // m_eLastAllocatedType;

	this->SetPartyTypeRelevantInfo(allocatePartyParams.allocPartyReqParams);

	allocatePartyParams.allocPartyReqParams.isdn_span_params.num_of_isdn_ports = 1; // =1 since we do it "channel by channel"

	std::vector<CIsdnSpanOrderPerConnection*>* connectVector = ((CIsdnPartyRsrcDesc*)m_pPartyAllocatedRsrc)->GetSpanOrderPerConnectionVector();

	WORD                                       connectionSize = connectVector ? connectVector->size() : 0;
	for (WORD i = 0; i < connectionSize; i++)
	{
		CIsdnSpanOrderPerConnection* connection_i = connectVector->at(i);
		DWORD conn_id  = connection_i->GetConnectionId();
		WORD  board_id = connection_i->GetBoardId();

		if (connection_id == conn_id)
		{
			allocatePartyParams.allocPartyReqParams.isdn_span_params.board_id = board_id;
			break;
		}
	}

	TRACEINTO << m_partyConfName
		<< "\n  connection_id     :" << allocatePartyParams.connectionIdList[0]
		<< "\n  monitor_conf_id   :" << allocatePartyParams.allocPartyReqParams.monitor_conf_id
		<< "\n  monitorPartyId    :" << allocatePartyParams.allocPartyReqParams.monitor_party_id
		<< "\n  networkPartyType  :" << allocatePartyParams.allocPartyReqParams.networkPartyType
		<< "\n  cideoPartyType    :" << allocatePartyParams.allocPartyReqParams.videoPartyType
		<< "\n  num_of_isdn_ports :" << allocatePartyParams.allocPartyReqParams.isdn_span_params.num_of_isdn_ports
		<< "\n  serviceName       :" << allocatePartyParams.allocPartyReqParams.isdn_span_params.serviceName
		<< "\n  board_id          :" << allocatePartyParams.allocPartyReqParams.isdn_span_params.board_id;

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&allocatePartyParams), sizeof(BOARD_FULL_REQ_PARAMS_S));

	STATUS res = SendReqToResourceAllocator(seg, REALLOCATE_RTM_ON_BOARD_FULL_REQ);
	PASSERT(res);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnRsrcReallocateBoardFull(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CIsdnPartyRsrcDesc* pTempPartyAllocatedRsrc = new CIsdnPartyRsrcDesc;
	pTempPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	pTempPartyAllocatedRsrc->DumpToTrace();

	BYTE bAllocationFailed = IsReallocateResponseValid(pTempPartyAllocatedRsrc);

	m_pPartyApi->OnRsrcAllocatorBoardFullAck(pTempPartyAllocatedRsrc, bAllocationFailed);
	POBJDELETE(pTempPartyAllocatedRsrc);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::AddPartyChannel(CIsdnNetSetup& netSetUp, WORD channelNum)
{
	TRACEINTO << m_partyConfName << ", channelNum:" << channelNum;

	additionalChannels.insert(ADDITIONAL_CHANNELS::value_type(channelNum, netSetUp));

	CSegment* seg = new CSegment;
	*seg << netSetUp.m_boardId;
	*seg << netSetUp.m_spanId[0]; // rons
	*seg << DUMMY_CONNECTION_ID; // (DWORD)0;
	*seg << channelNum;

	DispatchEvent(UPDATENETCHANNEL, seg);
	POBJDELETE(seg);
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnConnectToutAllocate(CSegment* pParam)
{
	PASSERTSTREAM(1, "Timer expired, there is no response from Resource Allocator, " << m_partyConfName);
	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty = 1; // Invoking KillPort process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnConnectToutCreate(CSegment* pParam)
{
	PASSERTSTREAM(1, "Timer expired, there is no ACK from MPL on create Req, " << m_partyConfName);
	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty   = 1; // Invoking KillPort process in RA.
	m_isRecovery = 1; // Invoking Recovery process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnConnectToutConnectAudio(CSegment* pParam)
{
	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		PASSERTSTREAM(1, "Timer expired, there is no response from Audio Bridge, " << m_partyConfName);
	}

	if (!m_isNetAudioConnected)
	{
		PASSERTSTREAM(1, "Timer expired, there is no ACK from MPL on Audio/Net connection, " << m_partyConfName);
	}

	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty = 1; // Invoking KillPort process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnConnectToutSetup(CSegment* pParam)
{
	PASSERTSTREAM(1, "Timer expired, there is no response from Party, " << m_partyConfName);
	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty = 1; // Invoking KillPort process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnConnectToutChangeAudio(CSegment* pParam)
{
	PASSERTSTREAM(1, "Timer expired, there is no response from Party, " << m_partyConfName);
	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty = 1;
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnConnectToutAudioConnected(CSegment* pParam)
{
	PASSERTSTREAM(1, "Timer expired, we did not get full remote caps, " << m_partyConfName);
	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty = 1; // Invoking KillPort process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnPartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate(CSegment* pParam)
{
	TRACEINTO << "We will res-end the allocate request, " << m_partyConfName;
	StartPartyConnection();
}

