#include "NStream.h"
#include "AddIsdnVoicePartyCntl.h"
#include "PartyApi.h"
#include "Conf.h"
#include "ObjString.h"
#include "AudioBridgeInterface.h"
#include "BridgePartyInitParams.h"
#include "BridgePartyAudioParams.h"
#include "StatusesGeneral.h"
#include "ConfPartySharedDefines.h"
#include "BridgeMoveParams.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "TraceStream.h"
#include "IsdnNetSetup.h"
#include "SysConfigKeys.h"
#include "Q931Structs.h"

extern "C" void PartyVoiceOutEntryPoint(void* appParam);

#define CONNECT_TIME 50        // (*seconds) timer for error handling

// CAddIsdnVoicePartyCntl states
const WORD ALLOCATE     = 1;   // waiting for resource allocator response
const WORD CREATE       = 2;   // waiting for create ack from mpl
const WORD CONNECTAUDIO = 3;   // waiting for connect ack from both audio bridge and MPL
const WORD SETUP        = 4;   // party was asked to establish call
const WORD CONNECT      = 5;   // party connected

PBEGIN_MESSAGE_MAP(CAddIsdnVoicePartyCntl)

	ONEVENT(ALLOCATE_PARTY_RSRC_IND,     ALLOCATE,     CAddIsdnVoicePartyCntl::OnRsrcAllocatePartyRspAllocate)
	ONEVENT(PARTY_AUDIO_CONNECTED,       CONNECTAUDIO, CAddIsdnVoicePartyCntl::OnAudConnectConnectAudio)
	ONEVENT(ACK_IND,                     CONNECTAUDIO, CAddIsdnVoicePartyCntl::OnMplAckConnectAudio)
	ONEVENT(ACK_IND,                     CREATE,       CAddIsdnVoicePartyCntl::OnMplAckCreate)

	// party events
	ONEVENT(PARTYCONNECT,                SETUP,        CAddIsdnVoicePartyCntl::OnPartyConnectSetup)
	ONEVENT(UPDATENETCHANNEL,            SETUP,        CAddIsdnVoicePartyCntl::OnPartyUpdateRTMChannel)
	ONEVENT(CONNECTDELAY,                IDLE,         CAddIsdnVoicePartyCntl::OnPartyConnectDelayIdle)

	// self timers
	ONEVENT(CONNECTTOUT,                 ALLOCATE,     CAddIsdnVoicePartyCntl::OnConnectToutAllocate)
	ONEVENT(CONNECTTOUT,                 CREATE,       CAddIsdnVoicePartyCntl::OnConnectToutCreate)
	ONEVENT(CONNECTTOUT,                 CONNECTAUDIO, CAddIsdnVoicePartyCntl::OnConnectToutConnectAudio)
	ONEVENT(CONNECTTOUT,                 SETUP,        CAddIsdnVoicePartyCntl::OnConnectToutSetup)
	ONEVENT(CONNECTTOUT,                 CONNECT,      CAddIsdnVoicePartyCntl::OnConnectToutConnect)

	ONEVENT(WAIT_FOR_RSRC_AND_ASK_AGAIN, ALLOCATE,     CAddIsdnVoicePartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate)
	ONEVENT(UPDATE_RTM_PORT_IND,         ANYCASE,      CAddIsdnVoicePartyCntl::NullActionFunction)

	ONEVENT(END_AVC_TO_SVC_ART_TRANSLATOR_CONNECT,         CREATE,      CAddIsdnVoicePartyCntl::OnEndAvcToSvcArtTranslatorConnectCreate)

PEND_MESSAGE_MAP(CAddIsdnVoicePartyCntl, CIsdnPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CAddIsdnVoicePartyCntl
////////////////////////////////////////////////////////////////////////////
CAddIsdnVoicePartyCntl::CAddIsdnVoicePartyCntl()
{
	m_isNetAudioConnected               = FALSE;
	m_isIncomingAudioMutedByAudioBridge = FALSE;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CAddIsdnVoicePartyCntl::~CAddIsdnVoicePartyCntl()
{
}

//--------------------------------------------------------------------------
void* CAddIsdnVoicePartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
CAddIsdnVoicePartyCntl& CAddIsdnVoicePartyCntl::operator =(const CAddIsdnVoicePartyCntl& other)
{
	(CIsdnPartyCntl&)*this = (CIsdnPartyCntl&)other;
	return *this;
}

//--------------------------------------------------------------------------
const char* CAddIsdnVoicePartyCntl::NameOf() const
{
	return "CAddIsdnVoicePartyCntl";
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::Create(CConf* pConf, CIsdnNetSetup* pNetSetUp,
                                    COsQueue* pConfRcvMbx, CAudioBridgeInterface* pAudioBridgeInterface, CConfAppMngrInterface* pConfAppMngrInterface, COsQueue* pPartyRcvMbx,
                                    CTaskApp* pParty, WORD termNum, const char* telNum, WORD type, const char* partyName,
                                    const char* confName, const char* password, DWORD monitorConfId,
                                    DWORD monitorPartyId, ENetworkType networkType, BYTE voice, BYTE audioVolume,
                                    const char* service_provider, WORD stby, WORD connectDelay,
                                    const char* AV_service_name, WORD welcome_msg_time,
                                    BYTE IsRecording)
{
	TRACEINTO << "PartyName:" << partyName;

	m_pConf                 = pConf;
	m_netSetUp              = *pNetSetUp;
	m_pPartyApi             = NULL;
	m_pParty                = pParty;
	m_type                  = type;
	m_termNum               = termNum;
	m_networkType           = networkType;
	m_monitorConfId         = monitorConfId;
	m_monitorPartyId        = monitorPartyId;
	m_state                 = IDLE;
	m_connectDelay          = connectDelay;
	m_voice                 = 1;
	m_isRemoteCapReady      = 0;
	m_pAudioInterface       = pAudioBridgeInterface;
	m_pConfAppMngrInterface = pConfAppMngrInterface;

	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi(pConf->GetRcvMbx(), this);
	m_pTaskApi->SetLocalMbx(pConf->GetLocalQueue());

	m_service_provider = new char[NET_SERVICE_PROVIDER_NAME_LEN];
	strcpy_safe(m_service_provider, NET_SERVICE_PROVIDER_NAME_LEN, service_provider);

	m_name = new char[H243_NAME_LEN];
	strcpy_safe(m_name, H243_NAME_LEN, partyName);

	SetFullName(partyName, confName);

	m_pBridgeMoveParams = new CBridgeMoveParams;

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
			// just for "Invited"feature:I suppose that if the party
			// trying to connect with dialing number "0" - it must be
			// standby party and the number will be updated
			if (!stby)
			{
				DialOut(confName, password, pConfRcvMbx, termNum, IsRecording);
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
			TRACEINTO << "PartyName:" << partyName << ", Type:" << m_type << " - Failed, invalid dial type";
			break;
		}
	}
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::DialOut(const char* confName, const char* password, COsQueue* pConfRcvMbx, WORD termNum, BYTE IsRecording, WORD redialInterval)
{
	TRACEINTO << m_partyConfName;

	if (m_connectDelay)
	{
		StartTimer(CONNECTDELAY, m_connectDelay);
	}
	else if (redialInterval)  // Re-dialing
	{
		TRACEINTO << "PartyName:" << m_partyConfName << " - StartTimer(CONNECTDELAY) for " << redialInterval << " ms";
		StartTimer(CONNECTDELAY, redialInterval);
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_REDIALING);
	}
	else
		StartPartyConnection();
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::StartPartyConnection()
{
	// VNGR-25962: The connect time is depending on channel connect time
	DWORD ringingDuration          = 0;
	DWORD connectTime              = 0;
	DWORD extraTimeForTotalConnect = 10;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("PSTN_RINGING_DURATION_SECONDS", ringingDuration);

	if (ringingDuration < CONNECT_TIME)
		connectTime = CONNECT_TIME;
	else
		connectTime = ringingDuration+ extraTimeForTotalConnect;

	StartTimer(CONNECTTOUT, connectTime*SECOND);

	AllocatePartyResources();
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::AllocatePartyResources()
{
	TRACEINTO << m_partyConfName;

	CreateAndSendAllocatePartyResources(eISDN_network_party_type, eVideo_party_type_none, eNoAllocationPolicy);
	m_state = ALLOCATE;
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::SendPartyEstablishCall()
{
	TRACEINTO << m_partyConfName;

	m_state = SETUP;
	CRsrcParams* pNetRsrcParams = new CRsrcParams;
	eLogicalResourceTypes lrt   = eLogical_net;

	*pNetRsrcParams = m_pPartyAllocatedRsrc->GetRsrcParams(lrt);

	m_RoomId = m_pPartyAllocatedRsrc->GetRoomId();

	CIsdnPartyRsrcDesc* desc = (CIsdnPartyRsrcDesc*)m_pPartyAllocatedRsrc;
	m_pPartyAllocatedRsrc->DumpToTrace();
	CIsdnPartyRsrcDesc* pPartyAllocatedRsrc = new CIsdnPartyRsrcDesc(*desc);
	pPartyAllocatedRsrc->DumpToTrace();

	m_pPartyApi->EstablishCallPstn(GetPartyRsrcId(), &m_netSetUp, pNetRsrcParams, pPartyAllocatedRsrc, m_RoomId);

	POBJDELETE(pNetRsrcParams);
	POBJDELETE(pPartyAllocatedRsrc);

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTING);
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::SendConnectAudioNet()
{
	TRACEINTO << m_partyConfName;

	OFF(m_isNetAudioConnected);

	DWORD audioEncConnectionId, netConnectionId;
	DWORD partyRsrcId = m_pPartyAllocatedRsrc->GetPartyRsrcId();

	audioEncConnectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_audio_encoder);
	netConnectionId      = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_net);

	m_pPartyHWInterface->SendConnect(audioEncConnectionId, netConnectionId, partyRsrcId, partyRsrcId);
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::ConnectPartyToAudioBridge()
{
	TRACEINTO << m_partyConfName;

	m_eAudBridgeConnState = eBridgeDisconnected;
	ON(m_bIsMemberInAudBridge);

	// MOTI: temp hard-coded params
	DWORD audioAlg = 0;
	if (E_NETWORK_TYPE_PSTN_E1 == m_networkType)
		audioAlg = A_Law_OU;
	else
		audioAlg = U_Law_OU;

	// !! Tmp for testing the Audio on T1
	BOOL isE1_NetworkType = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("SET_NETWORK_TYPE_TO_E1", isE1_NetworkType);

	if (TRUE == isE1_NetworkType)
	{
		TRACEINTO << m_partyConfName << " - ConnectPartyToAudioBridge T1 Audio";
		audioAlg = A_Law_OU;
	}

	BOOL isVideoParticipant = GetVoice() ? FALSE : TRUE;
	BOOL isVtxSupport       = FALSE;
	BOOL bIsSupportLegacyToSacTranslate = (m_pPartyAllocatedRsrc->GetConfMediaType()== eMixAvcSvc
			                               && CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyRMX) ? TRUE : FALSE;

	CDwordBitMask muteMask;
	muteMask.SetBit(MCMS);

	CBridgePartyAudioInParams audioDecoderParams;

	/*Initialize AudioIn*/
	audioDecoderParams.InitParams(audioAlg, muteMask,
			 	 	 	 	 	  GetAudioVolume(), AUDIO_MONO_NUM_OF_CHANNELS,
	                              GetConfAudioSampleRate(), isVideoParticipant,
	                              IsAgcOn(), isVtxSupport,
	                              GetDialType(), 0,
	                              GetEchoSuppression(), GetKeyboardSuppression(),
	                              GetAutoMuteNoisyParties(), GetAudioClarity(), m_ssrcAudio,
	                              GetConfSpeakerChangeMode(), bIsSupportLegacyToSacTranslate);

	CBridgePartyAudioOutParams audioEncoderParams;

	/*Initialize AudioOut*/
	audioEncoderParams.InitParams(audioAlg, muteMask, GetListeningAudioVolume(), AUDIO_MONO_NUM_OF_CHANNELS,
	                              GetConfAudioSampleRate(), isVideoParticipant);

	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(m_name, m_pParty, GetPartyRsrcId(),
                                                                              m_RoomId, GetInterfaceType(),
	                                                                          &audioDecoderParams, &audioEncoderParams);

	TRACEINTO << m_partyConfName << " - Establishing Audio Connection";

	m_pConfAppMngrInterface->ConnectPartyAudio(pBrdgPartyInitParams);
	m_isIncomingAudioMutedByAudioBridge = (BYTE)pBrdgPartyInitParams->GetConfAppParams()->IsMuteIncoming();
	m_eAudBridgeConnState               = eSendOpenInAndOut;
	ON(m_bIsMemberInAudBridge);

	POBJDELETE(pBrdgPartyInitParams);
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::Reconnect(const char* confName, const char* password, COsQueue* pConfRcvMbx,
                                       WORD termNum, WORD WelcomMode, BYTE isRecording, WORD redialInterval)
{
	TRACEINTO << m_partyConfName;

	m_termNum                     = termNum;
	m_disconnectMode              = 0;
	m_disconnectState             = 0;
	m_isDisconnect                = 0;
	m_isWaitingForRsrcAndAskAgain = YES;
	m_isFullBitRateConnect        = 0;
	m_isCDRPartyConnected         = 0;

	DialOut(confName, password, pConfRcvMbx, termNum, isRecording, redialInterval);
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnRsrcAllocatePartyRspAllocate(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	m_pPartyAllocatedRsrc = new CIsdnPartyRsrcDesc; // rons
	m_pPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	m_pPartyAllocatedRsrc->DumpToTrace();


	if (m_pPartyHWInterface)
		POBJDELETE(m_pPartyHWInterface);

	eNetworkPartyType networkPartyType = m_pPartyAllocatedRsrc->GetNetworkPartyType();
	m_RoomId = m_pPartyAllocatedRsrc->GetRoomId();

	DWORD status = m_pPartyAllocatedRsrc->GetStatus();
	if ((status != STATUS_OK) || (networkPartyType != eISDN_network_party_type))
	{
		if (status != STATUS_OK)
		{
			ClearUnusedPartyId(m_pPartyAllocatedRsrc->GetPartyRsrcId());

			if (STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN == status)
			{
				if (m_isWaitingForRsrcAndAskAgain)
				{
					WaitForRsrcAndAskAgain();
					return;
				}
				else
				{
					TRACEINTO << m_partyConfName << ", Status:STATUS_WAIT_FOR_RSRC_AND_ASK_AGAIN, m_isWaitingForRsrcAndAskAgain:NO";
					PASSERT(1);
				}
			}
			TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(m_pPartyAllocatedRsrc->GetStatus()).c_str() << " - Allocation failed, do not continue process";
		}
		else
		{
			TRACEINTO << "eNetworkPartyType!= eISDN_network_party_type, eNetworkPartyType=" << eNetworkPartyTypeNames[networkPartyType];
			PASSERT(1);
		}

#ifdef LOOKUP_TABLE_DEBUG_TRACE
		TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
		m_pParty = (CTaskApp*)(m_monitorPartyId + 100);     // Get unique party id

		if (DIALIN == m_type)
		{
			if (m_pPartyApi)
			{
				// dial in clear the net cntl
				m_pPartyApi->LogicalDelNetChannel(caus_USER_BUSY_VAL);
				POBJDELETE(m_pPartyApi);
			}
			else
			{
				// might cause a problem to free the net
				PASSERTMSG(101, "m_pPartyApi is NULL");
			}
		}

		m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, RESOURCES_DEFICIENCY, 1);  // Disconnect cause
		POBJDELETE(m_pPartyAllocatedRsrc);
		m_pTaskApi->EndAddParty(m_pParty, statIllegal);

		return;
	}

	// Create the task in Dialout only after Rsrc Ind
	if (DIALOUT == m_type)
	{
		COsQueue* pConfRcvMbx = (COsQueue*)(&(m_pTaskApi->GetRcvMbx()));
		void (*entryPoint)(void*) = PartyVoiceOutEntryPoint;

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

	// we can now send msgs to hw
	m_pPartyHWInterface = new CPartyInterface(m_pPartyAllocatedRsrc->GetPartyRsrcId(), m_pPartyAllocatedRsrc->GetConfRsrcId());
	m_pPartyHWInterface->SetRoomId(m_RoomId);
	m_state = CREATE;

	// !! Tmp for testing the Audio on T1
	BOOL isE1_NetworkType = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("SET_NETWORK_TYPE_TO_E1", isE1_NetworkType);

	if (TRUE == isE1_NetworkType)
	{
		SendCreateParty(E_NETWORK_TYPE_PSTN_E1);
	}
	else
		SendCreateParty(m_networkType);

	TRACEINTO << m_partyConfName << "ConfMediaType: " << m_pPartyAllocatedRsrc->GetConfMediaType() << " ,ProductFamily:" << CProcessBase::GetProcess()->GetProductFamily();
	
//	m_pPartyAllocatedRsrc->SetConfMediaType(eMixAvcSvc); //TEMP - need to remove, only for debugging
	if (m_pPartyAllocatedRsrc->GetConfMediaType() == eMixAvcSvc && CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyRMX)	
	{
		m_ssrcAudio = ((CIsdnPartyRsrcDesc*)m_pPartyAllocatedRsrc)->GetSsrcAudio();
		bool bIsTranslatorStarted = StartAvcToSvcArtTranslator();
		if (!bIsTranslatorStarted)
		{
			DBGPASSERT(status);
			m_pTaskApi->EndAddParty(m_pParty, statIllegal);
			m_isFaulty = 1; // Invoking KillPort process in RA
		}

	}
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnMplAckCreate(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	eConfMediaType confMediaType = m_pPartyAllocatedRsrc->GetConfMediaType();

	TRACEINTO
		<< "PartyId:" << GetPartyRsrcId()
		<< ", ConfId:" << GetConfRsrcId()
		<< ", AckOpcode:"<< AckOpcode
		<< ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")"
		<< ", ConfMediaType:" << confMediaType
		<< ", IsAckOnAvcToSvcArtTransalatorReceived:" << (WORD)m_bIsAckOnAvcToSvcArtTransalatorReceived;
	eProductType prodType = CProcessBase::GetProcess()->GetProductType();
	if (AckOpcode == CONF_MPL_CREATE_PARTY_REQ)
	{
		if (status == STATUS_OK)
		{
			m_bIsAckOnCreatePartyReceived = true;

			if ((confMediaType != eMixAvcSvc) || (confMediaType == eMixAvcSvc && (m_bIsAckOnAvcToSvcArtTransalatorReceived || prodType == eProductTypeNinja)))
			{
				m_state = CONNECTAUDIO;
				ConnectPartyToAudioBridge();
			}
		}
		else
		{
		    DBGPASSERT(status);
			m_pTaskApi->EndAddParty(m_pParty, statIllegal);
			m_isFaulty = 1; // Invoking KillPort process in RA.
			//If eMixAvcSvc -> In DelIsdnVoicePartyCntl need to Destroy ArtTranslator.
		}
	}
	else
	{
		PASSERTSTREAM(1, m_partyConfName << ", AckOpcode:" << AckOpcode << " - Invalid opcode");
	}
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnMplAckConnectAudio(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	if (AckOpcode == TB_MSG_CONNECT_REQ)
	{
		TRACEINTO << m_partyConfName << ", AckOpcode:TB_MSG_CONNECT_REQ, status:" << status;
		if (status == STATUS_OK)
		{
			m_isNetAudioConnected = TRUE;
			if (AreTwoDirectionsConnectedToAudioBridge())
			{
				SendPartyEstablishCall();
			}
		}
		else
		{
			DBGPASSERT(status);
			m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		}
	}
	else
	{
		PASSERTSTREAM(1, m_partyConfName << ", AckOpcode:" << AckOpcode << " - Invalid opcode");
	}
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnPartyConnectSetup(CSegment* pParam)
{
	ON(m_isRemoteCapReady);

	WORD status;
	*pParam >> status;

	TRACEINTO << m_partyConfName << ", Status:" << status;

	if (status)
	{
		WORD status_for_disconnecting = status;
		if (status_for_disconnecting == NO_NET_CONNECTION)
			status_for_disconnecting = statBusy;

		m_pTaskApi->EndAddParty(m_pParty, status_for_disconnecting);
		m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, status);
	}
	else
	{
		m_state = CONNECT;

		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTED_PARTIALY);

		// UnMute Audio Enc/Dec
		EOnOff eMuteOnOff = eOff;
		// check for mute by other parties
		if (m_isIncomingAudioMutedByAudioBridge)
			eMuteOnOff = eOn;

		// unmute MCMS outgoing audio (=> EP)
		m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaOut, eOff, MCMS);
		// update MCMS incoming audio (<= EP)
		m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eMuteOnOff, MCMS);

		DeleteTimer(CONNECTTOUT);
		ON(m_isFullBitRateConnect);
		m_pTaskApi->EndAddParty(m_pParty, status);
	}
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnAudConnectConnectAudio(CSegment* pParam)
{
	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		TRACEINTO << m_partyConfName << " - Connect has received after disconnect";
		DBGPASSERT(GetPartyRsrcId());
	}

	else
	{
		HandleAudioBridgeConnectedInd(pParam);
		TRACEINTO << m_partyConfName << " - Audio Connected";
		SendPartyEstablishCall();
	}
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnPartyConnectDelayIdle(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	StartPartyConnection();
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnPartyUpdateRTMChannel(CSegment* pParam)
{
	BYTE  board_id    = 0;
	DWORD span_id     = 0;
	WORD  num_channel = 0;

	DWORD connection_id = 0; // DIAL-IN should 0xffffffff

	*pParam >> board_id >> span_id >> connection_id >> num_channel;

	TRACEINTO
		<< "board_id:"        << (DWORD)board_id
		<< ", span_id:"       << span_id
		<< ", connection_id:" << connection_id
		<< ", channel_num:"   << num_channel;

	// update the connectionId - updated

	UPDATE_ISDN_PORT_S updateIsdnPort;
	memset(&updateIsdnPort, 0, sizeof(UPDATE_ISDN_PORT_S));

	updateIsdnPort.monitor_conf_id  = m_monitorConfId;  // Or m_pPartyAllocatedRsrc->GetConfRsrcId();//
	updateIsdnPort.monitor_party_id = m_monitorPartyId; // Or m_pPartyAllocatedRsrc->GetPartyRsrcId();//??
	updateIsdnPort.board_id         = board_id;
	updateIsdnPort.span_id          = span_id;
	updateIsdnPort.connection_id    = connection_id;
	updateIsdnPort.channel_index    = num_channel;

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&updateIsdnPort), sizeof(UPDATE_ISDN_PORT_S));

	// Eitan ToDo Change to a-sync call
	STATUS res = SendReqToResourceAllocator(seg, UPDATE_RTM_PORT_REQ);
	PASSERT(res);
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnConnectToutAllocate(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Failed, Timer expired (no response from Resource Allocator)";
	DBGPASSERT(101);

	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty = 1; // Invoking KillPort process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnConnectToutCreate(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Failed, Timer expired (no ACK from MPL on CREATE request)";
	DBGPASSERT(101);

	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty   = 1; // Invoking KillPort process in RA.
	m_isRecovery = 1; // Invoking Recovery process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnConnectToutConnectAudio(CSegment* pParam)
{
	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		TRACEINTO << m_partyConfName << " - Failed, Timer expired (no response from Audio Bridge)";
		DBGPASSERT(101);
	}

	if (!m_isNetAudioConnected)
	{
		TRACEINTO << m_partyConfName << " - Failed, Timer expired (no ACK from MPL Audio/Net connection)";
		DBGPASSERT(102);
	}

	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty = 1; // Invoking KillPort process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnConnectToutSetup(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Failed, Timer expired (no response from Party)";
	DBGPASSERT(101);

	m_pTaskApi->UpdateDB(m_pParty, DISCAUSE, NO_DISCONNECTION_CAUSE);
	m_pTaskApi->EndAddParty(m_pParty, statTout);
	m_isFaulty = 1; // Invoking KillPort process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnConnectToutConnect(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Failed, Timer expired (should have been deleted)";
	PASSERT(1);

	m_isFaulty = 1; // Invoking KillPort process in RA.
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::SetPartyTypeRelevantInfo(ALLOC_PARTY_REQ_PARAMS_S& allocatePartyParams)
{
	CConfParty* pConfParty = 0;
	if (!(m_pConf->GetCommConf()) || !(pConfParty = (m_pConf->GetCommConf())->GetCurrentParty(m_name)))
	{
		TRACESTRFUNC(eLevelError) << "Can not find party";
		PASSERT(1);
		return;
	}

	WORD spanId  = DUMMY_SPAN_ID;
	WORD boardId = DUMMY_BOARD_ID;

	if (DIALIN == m_type)
	{
		boardId = m_netSetUp.m_boardId;
		spanId  = m_netSetUp.m_spanId[0]; // olga - actually spanID that was got from CLobby
	}

	// Setting the service name
	strcpy((char*)(allocatePartyParams.isdn_span_params.serviceName), pConfParty->GetServiceProviderName());

	// Setting the ISDN span params
	allocatePartyParams.isdn_span_params.span_id           = spanId;
	allocatePartyParams.isdn_span_params.board_id          = boardId;
	allocatePartyParams.isdn_span_params.num_of_isdn_ports = 1; // rons
}

//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnTimerWaitForRsrcAndAskAgainAllocate(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Resend the allocate request";

	StartPartyConnection();
}
//--------------------------------------------------------------------------
void CAddIsdnVoicePartyCntl::OnEndAvcToSvcArtTranslatorConnectCreate(CSegment* pParam)
{
	STATUS status;
	*pParam >> status;

	TRACEINTO
		<< "PartyId:" << GetPartyRsrcId()
		<< ", PartyName:" << m_partyConfName
		<< ", ConfId:" << GetConfRsrcId()
		<< ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")"
		<< ", IsAckOnCreatePartyReceived" << (WORD)m_bIsAckOnCreatePartyReceived;

	if (STATUS_OK == status)
	{
		m_bIsAckOnAvcToSvcArtTransalatorReceived = true;

		if (m_bIsAckOnCreatePartyReceived)
		{
			m_state = CONNECTAUDIO;
			ConnectPartyToAudioBridge();
		}
	}
	else
	{
		DBGPASSERT(status);
		m_pTaskApi->EndAddParty(m_pParty, statIllegal);
		m_isFaulty = 1; // Invoking KillPort process in RA
	}
}
