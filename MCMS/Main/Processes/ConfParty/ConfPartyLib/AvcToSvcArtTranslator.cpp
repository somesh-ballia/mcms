//includes
#include "AvcToSvcArtTranslator.h"
#include "Types.h"
#include "ConfPartyOpcodes.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "HardwareInterface.h"
#include "ArtRequestStructs.h"
#include "ArtDefinitions.h"
#include "IpMfaOpcodes.h"
#include "StatusesGeneral.h"
#include "OpcodesMrmcCardMngrMrc.h"
#include "Trace.h"
#include "TraceStream.h"
#include "ProcessBase.h"
#include "ConfPartyGlobals.h"
#include "ConfPartyRoutingTable.h"
#include "PartyApi.h"

//#include "IpQosDefinitions.h"


// defines TOUT
#define	AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT		17890
#define	AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT	17891

#define AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT_VALUE 	3*SECOND
#define AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT_VALUE  3*SECOND

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );


// Action Function table
PBEGIN_MESSAGE_MAP(CAvcToSvcArtTranslator)

	ONEVENT(CONNECT_AVC_TO_SVC_ART_TRANSLATOR,			IDLE,               CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorConnectIDLE)
	ONEVENT(CONNECT_AVC_TO_SVC_ART_TRANSLATOR,			SETUP,              CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorConnectSETUP)
	ONEVENT(CONNECT_AVC_TO_SVC_ART_TRANSLATOR,			CONNECTED,          CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorConnectCONNECTED)
	ONEVENT(CONNECT_AVC_TO_SVC_ART_TRANSLATOR,			DISCONNECTING,      CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorConnectDISCONNECTING)

	ONEVENT(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR,       IDLE,               CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorDisconnectIDLE)
	ONEVENT(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR,       SETUP,              CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorDisconnectSETUP)
	ONEVENT(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR,       CONNECTED,          CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorDisconnectCONNECTED)
	ONEVENT(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR,       DISCONNECTING,      CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorDisconnectDISCONNECTING)


	ONEVENT(ACK_IND,                   			        IDLE,               CAvcToSvcArtTranslator::OnMplAckIDLE)
	ONEVENT(ACK_IND,                  		            SETUP,              CAvcToSvcArtTranslator::OnMplAckSETUP)
	ONEVENT(ACK_IND,                           		    CONNECTED,          CAvcToSvcArtTranslator::OnMplAckCONNECTED)
	ONEVENT(ACK_IND,          		                    DISCONNECTING,      CAvcToSvcArtTranslator::OnMplAckDISCONNECTING)


	ONEVENT(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT,        IDLE,               CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorOpenToutIDLE)
	ONEVENT(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT,        SETUP,              CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorOpenToutSETUP)
	ONEVENT(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT,        CONNECTED,          CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorOpenToutCONNECTED)
	ONEVENT(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT,        DISCONNECTING,      CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorOpenToutDISCONNECTING)

	ONEVENT(AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT,       IDLE,               CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorCloseToutIDLE)
	ONEVENT(AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT,       SETUP,              CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorCloseToutSETUP)
	ONEVENT(AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT,       CONNECTED,          CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorCloseToutCONNECTED)
	ONEVENT(AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT,       DISCONNECTING,      CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorCloseToutDISCONNECTING)


PEND_MESSAGE_MAP(CAvcToSvcArtTranslator,CStateMachineValidation);
// ------------------------------------------------------------
CAvcToSvcArtTranslator::CAvcToSvcArtTranslator()
{
	m_pMfaInterface            = NULL;
	m_pMrmpInterface           = NULL;
	m_pPartyApi                = NULL;
	m_ssrc                     = 0;
	m_lastReqId	               = 0xFFFFFFFF;
	m_lastReq			       = 0xFFFFFFFF;
	m_partyRsrcId              = (PartyRsrcID)0;
	m_confRsrcId               = (ConfRsrcID)0;
	m_bIsReadyToBeKilled       = false;
	m_statusOnCloseMrmpChannel = STATUS_FAIL;
	m_statusOnCloseArt         = STATUS_FAIL;
	m_channelHandle            = 0;
}
// ------------------------------------------------------------
CAvcToSvcArtTranslator::~CAvcToSvcArtTranslator()
{
	POBJDELETE(m_pMfaInterface);
	POBJDELETE(m_pMrmpInterface);
	POBJDELETE(m_pPartyApi);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	  DispatchEvent(opCode,pMsg);
}
// ------------------------------------------------------------
void* CAvcToSvcArtTranslator::GetMessageMap()
{
  return (void*)m_msgEntries;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::Create(CREATE_AVC_TO_SVC_ART_TRANSLATOR_S &rAvcToSvcArtTranslator)
{
	m_ssrc = rAvcToSvcArtTranslator.ssrc;
	m_partyRsrcId = rAvcToSvcArtTranslator.partyRsrcId;
	m_confRsrcId = rAvcToSvcArtTranslator.confRsrcId;

	TRACEINTO << "PartyId: " << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", ssrc: " << m_ssrc << " ,roomId:" << rAvcToSvcArtTranslator.roomId;
	CreateHardwareInterfaces(rAvcToSvcArtTranslator);

	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
	PASSERT_AND_RETURN(!pParty);
	m_pPartyApi = new CPartyApi;
	m_pPartyApi->CreateOnlyApi(pParty->GetRcvMbx(), this);
	m_pPartyApi->SetLocalMbx(pParty->GetLocalQueue());

	AddToRoutingTable(rAvcToSvcArtTranslator);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::CreateHardwareInterfaces(CREATE_AVC_TO_SVC_ART_TRANSLATOR_S &avcToSvcArtTranslator)
{
	TRACEINTO << "PartyId: " << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", ssrc: " << m_ssrc;
	if (m_pMfaInterface)
	{
		POBJDELETE(m_pMfaInterface);
	}
	m_pMfaInterface = new CHardwareInterface();
	if (NULL != m_pMfaInterface)
	{
		m_pMfaInterface->Create(avcToSvcArtTranslator.pMfaRsrcParams);
		m_pMfaInterface->SetRoomId(avcToSvcArtTranslator.roomId);
	}
	else
	{
		PASSERT(1);
	}

	if(m_pMrmpInterface)
	{
		POBJDELETE(m_pMrmpInterface);
	}
	m_pMrmpInterface = new CHardwareInterface();
	if (NULL != m_pMrmpInterface)
	{
		m_pMrmpInterface->Create(avcToSvcArtTranslator.pMrmpRsrcParams);
		m_pMrmpInterface->SetRoomId(avcToSvcArtTranslator.roomId);
	}
	else
	{
		PASSERT(1);
	}
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::AddToRoutingTable(CREATE_AVC_TO_SVC_ART_TRANSLATOR_S &avcToSvcArtTranslator)
{
	TRACEINTO << "PartyId: " << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", ssrc: " << m_ssrc;

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();

	PASSERT_AND_RETURN(!pRoutingTable);

	STATUS status = STATUS_FAIL;
	if (avcToSvcArtTranslator.pMrmpRsrcParams)
	{
//		ConnectionID mrmpConnectionId = avcToSvcArtTranslator.pMrmpRsrcParams->GetRsrcDesc()->GetConnectionId();
//		CPartyRsrcRoutingTblKey mrmpRoutingKey = CPartyRsrcRoutingTblKey(mrmpConnectionId, m_partyRsrcId, eLogical_relay_rtp);
//		pRoutingTable->AddPartyRsrcDesc(mrmpRoutingKey);
		status = pRoutingTable->AddStateMachinePointerToRoutingTbl(*avcToSvcArtTranslator.pMrmpRsrcParams,m_pPartyApi);
		if (status != STATUS_OK)
		{
			PTRACE(eLevelInfoNormal, "FAILED pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pMrmpRsrcDesc,m_pPartyApi)");
			DBGPASSERT(status);
		}
	}
    if (avcToSvcArtTranslator.pMfaRsrcParams)
    {
//    	ConnectionID mfaConnectionId = avcToSvcArtTranslator.pMfaRsrcParams->GetRsrcDesc()->GetConnectionId();
//    	CPartyRsrcRoutingTblKey mfaRoutingKey = CPartyRsrcRoutingTblKey(mfaConnectionId, m_partyRsrcId, eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
//    	pRoutingTable->AddPartyRsrcDesc(mfaRoutingKey);
    	status=pRoutingTable->AddStateMachinePointerToRoutingTbl(*avcToSvcArtTranslator.pMfaRsrcParams,m_pPartyApi);
		if (status != STATUS_OK)
		{
			PTRACE(eLevelInfoNormal, "FAILED pRoutingTbl->AddStateMachinePointerToRoutingTbl(*m_pMfaRsrcDesc,m_pPartyApi)");
			DBGPASSERT(status);
		}
    }
    TRACEINTO << "PartyId: " << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::Connect()
{
	DispatchEvent(CONNECT_AVC_TO_SVC_ART_TRANSLATOR, NULL);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorConnectIDLE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;

	m_state = SETUP;

	SendOpenArtRequest();
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorConnectSETUP(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored already in connect process";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,translator already connected";
	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
	PASSERT_AND_RETURN(!pParty);

	((CIsdnParty*)pParty)->AvcToSvcArtTranslatorConnected(STATUS_OK);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplAckSETUP(CSegment* pParam)
{
	CSegment *pSeg  = new CSegment (*pParam);
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch(AckOpcode)
	{
		case	TB_MSG_OPEN_PORT_REQ:
		{
			OnMplOpenArtAck(status);
			break;
		}
		case	H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ:
		{
			OnMplOpenRtpChannelAck(status);
			break;
		}

		case	CONF_PARTY_MRMP_OPEN_CHANNEL_REQ:
		{
			OnMplOpenMrmpChannelAck(pSeg, status);
			break;
		}
		default:
		{
			TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " - ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}
	POBJDELETE(pSeg);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplAckIDLE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplAckCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplAckDISCONNECTING(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch(AckOpcode)
	{
		case  CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ:
		{
			OnMplCloseMrmpChannelAck(status);
			break;
		}

		case  TB_MSG_CLOSE_PORT_REQ:
		{
			OnMplCloseArtAck(status);
			break;
		}

		default:
		{
			TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " - ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}// end switch
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorOpenToutIDLE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorOpenToutSETUP(CSegment* pParam)
{
	PASSERT(m_partyRsrcId);

	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
	PASSERT_AND_RETURN(!pParty);
	((CIsdnParty*)pParty)->AvcToSvcArtTranslatorConnected(STATUS_FAIL);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorOpenToutCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorOpenToutDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorDisconnectIDLE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << "translator already disconnected";
	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
	PASSERT_AND_RETURN(!pParty);
	((CIsdnParty*)pParty)->AvcToSvcArtTranslatorDisconnected(m_statusOnCloseMrmpChannel, m_statusOnCloseArt);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorDisconnectSETUP(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;
	m_state = DISCONNECTING;
	SendCloseMrmpChannelRequest();
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorDisconnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;
	m_state = DISCONNECTING;
	SendCloseMrmpChannelRequest();
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorDisconnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorCloseToutIDLE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorCloseToutSETUP(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorCloseToutCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << " ,message ignored";
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnAvcToSvcArtTranslatorCloseToutDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;

	m_state = IDLE;

	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
	PASSERT_AND_RETURN(!pParty);
	((CIsdnParty*)pParty)->AvcToSvcArtTranslatorDisconnected(m_statusOnCloseMrmpChannel, m_statusOnCloseArt);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::SendOpenArtRequest()
{
	DWORD       maxAudioTxBitsPer10ms = 64000;
	TOpenArtReq stOpenArtReq;
	memset(&stOpenArtReq, 0, sizeof(TOpenArtReq));
	stOpenArtReq.enNetworkType = E_NETWORK_TYPE_IP;
	stOpenArtReq.unVideoTxMaxNumberOfBitsPer10ms = maxAudioTxBitsPer10ms;
	stOpenArtReq.ConnectContent = FALSE;
	stOpenArtReq.nMediaMode = eMediaModeTranscoding;

	CSegment* pSegParam  = new CSegment;
	pSegParam->Put((BYTE*)(&stOpenArtReq), sizeof(TOpenArtReq));

	m_lastReqId = m_pMfaInterface->SendMsgToMPL(TB_MSG_OPEN_PORT_REQ, pSegParam);
	m_lastReq = TB_MSG_OPEN_PORT_REQ;

	StartTimer(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT, AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT_VALUE);
}
// ------------------------------------------------------------
PartyRsrcID CAvcToSvcArtTranslator::GetPartyRsrcId()
{
	return m_partyRsrcId;
}
// ------------------------------------------------------------
ConfRsrcID CAvcToSvcArtTranslator::GetConfRsrcId()
{
	return m_confRsrcId;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplOpenArtAck(STATUS status)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")";

	if (status != STATUS_OK)
	{
		DeleteTimer(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT);
		CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
		PASSERT_AND_RETURN(!pParty);
		((CIsdnParty*)pParty)->AvcToSvcArtTranslatorConnected(status);
	}
	else
	{
		SendOpenRtpChannelRequest();
	}
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::SendOpenRtpChannelRequest()
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;

	TUpdatePortOpenRtpChannelReq stOpenRtpChannel;
	memset(&stOpenRtpChannel, 0, sizeof(TUpdatePortOpenRtpChannelReq));

	FillOpenRtpChannelStruct(stOpenRtpChannel);

	CSegment* pSegParam  = new CSegment;
	pSegParam->Put((BYTE*)(&stOpenRtpChannel), sizeof(TUpdatePortOpenRtpChannelReq));

	m_lastReqId = m_pMfaInterface->SendMsgToMPL(H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ, pSegParam);
	m_lastReq = H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::FillOpenRtpChannelStruct(TUpdatePortOpenRtpChannelReq &stOpenRtpChannel)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;
	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
	if(pParty)
	{
		stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unDestMcuId                       = pParty->GetMcuNum();
		stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unDestTerminalId                  = pParty->GetTerminalNum();
	}

	stOpenRtpChannel.bunIsRecovery                                                         = FALSE;
	stOpenRtpChannel.unChannelType                                                         = kIpAudioChnlType;
	stOpenRtpChannel.unChannelDirection                                                    = cmCapTransmit;
	stOpenRtpChannel.unCapTypeCode                                                         = eSirenLPR_Scalable_48kCapCode;
	stOpenRtpChannel.unEncryptionType                                                      = kUnKnownMediaType;
	memset(&(stOpenRtpChannel.aucSessionKey), '0', sizeOf128Key);
	stOpenRtpChannel.unSequenceNumber                                                      = 0;
	stOpenRtpChannel.unTimeStamp                                                           = 0;
	stOpenRtpChannel.unSyncSource                                                          = 0;
	stOpenRtpChannel.updateSsrcParams.unUpdatedSSRC                                        = m_ssrc;
	stOpenRtpChannel.updateSsrcParams.bReplaceSSRC                                         = TRUE;
	stOpenRtpChannel.updateSsrcParams.unUpdatedCSRC                                        = 0;
	stOpenRtpChannel.updateSsrcParams.bReplaceCSRC                                         = FALSE;
	stOpenRtpChannel.mediaMode                                                             = eMediaModeTranscoding;

	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.bunIsH263Plus                         = 0;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.bunIsFlipIntraBit                     = 0;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unAnnexesMask                         = 0;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpiCif                             = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpiQCif                            = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpi4Cif                            = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpi16Cif                           = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpiVga                             = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpiNtsc                            = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpiSvga                            = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpiXga                             = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpiSif                             = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.nHcMpiQvga                            = -1;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.b32StreamRequiresEndOfFrameParsing    = FALSE;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unMaxFramesPerPacket                  = 0;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unCustomMaxMbpsValue                  = 0;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.bunIsCCEnabled		                   = NO;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.bunContentEnabled                     = 0;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unPayloadType                         = 213;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unDtmfPayloadType                     = _UnKnown;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unBitRate                             = 48;

	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unPacketPayloadFormat                 = E_PACKET_PAYLOAD_FORMAT_SINGLE_UNIT;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.unMaxFramesPerPacket                  = 0;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.openTipPortParams.bnTipIsEnabled 	   = FALSE;
	stOpenRtpChannel.tUpdateRtpSpecificChannelParams.openTipPortParams.eTipPosition		   = eTipNone;

	stOpenRtpChannel.tLprSpecificParams.bunLprEnabled                                      = 0;
	stOpenRtpChannel.tLprSpecificParams.unVersionID                                        = 0;
	stOpenRtpChannel.tLprSpecificParams.unMinProtectionPeriod                              = 0;
	stOpenRtpChannel.tLprSpecificParams.unMaxProtectionPeriod                              = 0;
	stOpenRtpChannel.tLprSpecificParams.unMaxRecoverySet                                   = 0;
	stOpenRtpChannel.tLprSpecificParams.unMaxRecoveryPackets                               = 0;
	stOpenRtpChannel.tLprSpecificParams.unMaxPacketSize                                    = 0;

	stOpenRtpChannel.sdesCap.bIsSrtpInUse                                                  = 0;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplOpenRtpChannelAck(STATUS status)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")";
	if(status != STATUS_OK)
	{
		DeleteTimer(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT);
		SendCloseArtRequest();
		CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
		PASSERT_AND_RETURN(!pParty);
		((CIsdnParty*)pParty)->AvcToSvcArtTranslatorConnected(status);
	}
	else
	{
		SendOpenMrmpChannelRequest();
	}
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::SendOpenMrmpChannelRequest()
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;

	MrmpOpenChannelRequestStruct stMrmpOpenChannel;
	memset(&stMrmpOpenChannel,0,sizeof(MrmpOpenChannelRequestStruct));

	FillMrmpOpenChannelRequestStruct(stMrmpOpenChannel);

	CSegment* pSegParam  = new CSegment;
	pSegParam->Put((BYTE*)(&stMrmpOpenChannel), sizeof(MrmpOpenChannelRequestStruct));

	m_lastReqId = m_pMrmpInterface->SendMsgToMPL(CONF_PARTY_MRMP_OPEN_CHANNEL_REQ, pSegParam);
	m_lastReq = CONF_PARTY_MRMP_OPEN_CHANNEL_REQ;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::FillMrmpOpenChannelRequestStruct(MrmpOpenChannelRequestStruct &stMrmpOpenChannel)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;

	stMrmpOpenChannel.m_channelType = kAvcToSacChnlType;
	stMrmpOpenChannel.m_channelDirection = cmCapReceive;
	stMrmpOpenChannel.m_partyId = m_partyRsrcId;

	stMrmpOpenChannel.m_localAddress.ipVersion = eIpVersion4;
	stMrmpOpenChannel.m_localAddress.addr.v4.ip = 0;
	stMrmpOpenChannel.m_localAddress.transportType = eTransportTypeUdp;
	stMrmpOpenChannel.m_localAddress.distribution  = eDistributionUnicast;

	stMrmpOpenChannel.m_remoteAddress.ipVersion = eIpVersion4;
	stMrmpOpenChannel.m_remoteAddress.addr.v4.ip = 0;
	stMrmpOpenChannel.m_remoteAddress.distribution = eDistributionUnicast;
	stMrmpOpenChannel.m_remoteAddress.transportType = eTransportTypeUdp;

	stMrmpOpenChannel.m_localRtcpAddress.ipVersion = eIpVersion4;
	stMrmpOpenChannel.m_localRtcpAddress.transportType = eTransportTypeUdp;
	stMrmpOpenChannel.m_localRtcpAddress.addr.v4.ip = 0;
	stMrmpOpenChannel.m_localRtcpAddress.distribution  = eDistributionUnicast;

	stMrmpOpenChannel.m_remoteRtcpAddress.ipVersion = eIpVersion4;
	stMrmpOpenChannel.m_remoteRtcpAddress.addr.v4.ip = 0;
	stMrmpOpenChannel.m_remoteRtcpAddress.transportType = eTransportTypeUdp;
	stMrmpOpenChannel.m_remoteRtcpAddress.distribution = eDistributionUnicast;

	stMrmpOpenChannel.m_localAddress.port = 0;
	stMrmpOpenChannel.m_remoteAddress.port = 0;
	stMrmpOpenChannel.m_localRtcpAddress.port = 0;
	stMrmpOpenChannel.m_remoteRtcpAddress.port = 0;

	memset(&(stMrmpOpenChannel.physicalId), 0, sizeof(PHYSICAL_RESOURCE_INFO_S));
	stMrmpOpenChannel.m_capTypeCode = eSvcCapCode;
	stMrmpOpenChannel.m_PayloadType = 213;
	stMrmpOpenChannel.m_dtmfPayloadType = _UnKnown;

	memset(stMrmpOpenChannel.m_ssrcInfo, 0, (sizeof(IncomingSsrcInfo)*MAX_SSRC_PER_INCOMING_CHANNEL));
	stMrmpOpenChannel.m_operationPointsSetId = m_confRsrcId;
	stMrmpOpenChannel.m_ssrcInfo[0].m_ssrc = m_ssrc;

	CRsrcDesc *pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_partyRsrcId, eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
	PASSERT_AND_RETURN(!pRsrcDesc);
	stMrmpOpenChannel.m_allocatedPhysicalResources = 1;
	stMrmpOpenChannel.physicalId[0].connection_id = pRsrcDesc->GetConnectionId();
	stMrmpOpenChannel.physicalId[0].party_id = m_partyRsrcId;

	stMrmpOpenChannel.m_videoFlag = NO;
	stMrmpOpenChannel.m_maxVideoBR = 0;
	stMrmpOpenChannel.uRtpKeepAlivePeriod = 0;

	for (int i=0; i<NumberOfTosValues; i++)
	{
		stMrmpOpenChannel.tosValue[i] = 0;
	}

	stMrmpOpenChannel.ice_channel_rtp_id  = 0;
	stMrmpOpenChannel.ice_channel_rtcp_id = 0;

	memset(&(stMrmpOpenChannel.m_sDesCapSt), 0, sizeof(sdesCapSt));
	stMrmpOpenChannel.m_sDesCapSt.bIsSrtpInUse = 0;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplOpenMrmpChannelAck(CSegment* pParam, STATUS status)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")";
	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
	PASSERT_AND_RETURN(!pParty);

	if (status != STATUS_OK)
	{
		DeleteTimer(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT);
		SendCloseArtRequest();
	}
	else
	{
		ACK_IND_S stAckInd;
		*pParam >> stAckInd.ack_base.ack_opcode
				>> stAckInd.ack_base.ack_seq_num
				>> stAckInd.ack_base.status
				>> stAckInd.ack_base.reason
				>> stAckInd.media_type
				>> stAckInd.media_direction
				>> m_channelHandle;
		TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", channelHandle:" << m_channelHandle;
		SetConnected();
	}

	((CIsdnParty*)pParty)->AvcToSvcArtTranslatorConnected(status);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::SetConnected()
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;
	DeleteTimer(AVC_TO_SVC_ART_TRANSLATOR_OPEN_TOUT);
	m_state = CONNECTED;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::SendCloseMrmpChannelRequest()
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;

	MrmpCloseChannelRequestStruct stMrmpCloseChannel;
	memset(&stMrmpCloseChannel,0,sizeof(MrmpCloseChannelRequestStruct));

	FillMrmpCloseChannelRequestStruct(stMrmpCloseChannel);

	CSegment* pSegParam  = new CSegment;
	pSegParam->Put((BYTE*)(&stMrmpCloseChannel), sizeof(MrmpCloseChannelRequestStruct));

	m_lastReqId = m_pMrmpInterface->SendMsgToMPL(CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ, pSegParam);
	m_lastReq = CONF_PARTY_MRMP_CLOSE_CHANNEL_REQ;

	StartTimer(AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT, AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT_VALUE);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::FillMrmpCloseChannelRequestStruct(MrmpCloseChannelRequestStruct &stMrmpCloseChannel)
{
	stMrmpCloseChannel.m_allocatedPhysicalResources = 1;
	memset(&(stMrmpCloseChannel.physicalId[0]), 0, sizeof(PHYSICAL_RESOURCE_INFO_S)); // @#@ - physicalId
    stMrmpCloseChannel.physicalId[0].connection_id = m_pMrmpInterface->GetConnectionId();
	stMrmpCloseChannel.physicalId[0].party_id = m_partyRsrcId;
	stMrmpCloseChannel.physicalId[0].physical_id.resource_type = ePhysical_art;

	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.channelHandle = m_channelHandle;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_channelType = kAvcToSacChnlType;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_channelDirection = cmCapReceive;

	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localAddress.addr.v4.ip = 0;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localAddress.ipVersion = eIpVersion4;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localAddress.transportType = eTransportTypeUdp;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localAddress.distribution  = eDistributionUnicast;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localAddress.port = 0;

	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteAddress.addr.v4.ip = 0;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteAddress.ipVersion = eIpVersion4;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteAddress.transportType = eTransportTypeUdp;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteAddress.distribution  = eDistributionUnicast;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteAddress.port = 0;

	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localRtcpAddress.addr.v4.ip = 0;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localRtcpAddress.ipVersion = eIpVersion4;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localRtcpAddress.transportType = eTransportTypeUdp;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localRtcpAddress.distribution  = eDistributionUnicast;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_localRtcpAddress.port = 0;

	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteRtcpAddress.addr.v4.ip = 0;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteRtcpAddress.ipVersion = eIpVersion4;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteRtcpAddress.transportType = eTransportTypeUdp;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteRtcpAddress.distribution  = eDistributionUnicast;
	stMrmpCloseChannel.tMrmpCloseChannelRequestMessage.m_remoteRtcpAddress.port = 0;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplCloseMrmpChannelAck(STATUS status)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")";
	m_statusOnCloseMrmpChannel = status;

	SendCloseArtRequest();
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::SendCloseArtRequest()
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;
	m_lastReqId = m_pMfaInterface->SendMsgToMPL(TB_MSG_CLOSE_PORT_REQ, NULL);
    m_lastReq = TB_MSG_CLOSE_PORT_REQ;
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::OnMplCloseArtAck(STATUS status)
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId << ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")";
	CParty *pParty = GetLookupTableParty()->Get(m_partyRsrcId);
	PASSERT_AND_RETURN(!pParty);

	m_statusOnCloseArt = status;

	m_state = IDLE;

	DeleteTimer(AVC_TO_SVC_ART_TRANSLATOR_CLOSE_TOUT);

	RemoveFromRoutingTable();

	((CIsdnParty*)pParty)->AvcToSvcArtTranslatorDisconnected(m_statusOnCloseMrmpChannel, m_statusOnCloseArt);
}
// ------------------------------------------------------------
void CAvcToSvcArtTranslator::RemoveFromRoutingTable()
{
	TRACEINTO << "PartyId:" << m_partyRsrcId << ", ConfId:" << m_confRsrcId;

	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);

	CRsrcParams* pMrmpRsrcParams = m_pMfaInterface->GetRsrcParams();
	PASSERT_AND_RETURN(!pMrmpRsrcParams);

	DBGPASSERT(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pMrmpRsrcParams));

	CRsrcParams* pMfaRsrcParams = m_pMfaInterface->GetRsrcParams();
	PASSERT_AND_RETURN(!pMfaRsrcParams);

	DBGPASSERT(STATUS_FAIL == pRoutingTable->RemoveStateMachinePointerFromRoutingTbl(*pMfaRsrcParams));
}

// ------------------------------------------------------------
void CAvcToSvcArtTranslator::Disconnect()
{
	DispatchEvent(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR, NULL);
}

// ------------------------------------------------------------
BOOL CAvcToSvcArtTranslator::DispatchEvent(OPCODE event, CSegment* pParam)
{
	TRACEINTO << "Opcode:" << event;
	if (event == ACK_IND)
	{

	}
	return CStateMachineValidation::DispatchEvent(event, pParam);
}
