#include "LegacyToSacTranslator.h"
#include "AudioHardwareInterface.h"
#include "ConfPartyOpcodes.h"
#include "OpcodesMcmsCommon.h"
#include "BridgePartyAudioInOut.h"
#include "HlogApi.h"
#include "OpcodesMcmsAudio.h"
#include "BridgePartyCntl.h"


// Action Function table
PBEGIN_MESSAGE_MAP(CLegacyToSacTranslator)

	ONEVENT(CONNECT_LEGACY_TO_SAC_TRANSLATOR,			IDLE,               CLegacyToSacTranslator::OnLegacyToSacTranslatorConnectIDLE)
	ONEVENT(CONNECT_LEGACY_TO_SAC_TRANSLATOR,			SETUP,              CLegacyToSacTranslator::OnLegacyToSacTranslatorConnectSETUP)
	ONEVENT(CONNECT_LEGACY_TO_SAC_TRANSLATOR,			CONNECTED,          CLegacyToSacTranslator::OnLegacyToSacTranslatorConnectCONNECTED)
	ONEVENT(CONNECT_LEGACY_TO_SAC_TRANSLATOR,			DISCONNECTING,      CLegacyToSacTranslator::OnLegacyToSacTranslatorConnectDISCONNECTING)

	ONEVENT(DISCONNECT_LEGACY_TO_SAC_TRANSLATOR,        IDLE,               CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnectIDLE)
	ONEVENT(DISCONNECT_LEGACY_TO_SAC_TRANSLATOR,        SETUP,              CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnectSETUP)
	ONEVENT(DISCONNECT_LEGACY_TO_SAC_TRANSLATOR,        CONNECTED,          CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnectCONNECTED)
	ONEVENT(DISCONNECT_LEGACY_TO_SAC_TRANSLATOR,        DISCONNECTING,      CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnectDISCONNECTING)


	ONEVENT(ACK_IND,                   			        IDLE,               CLegacyToSacTranslator::OnMplAckIDLE)
	ONEVENT(ACK_IND,                  		            SETUP,              CLegacyToSacTranslator::OnMplAckSETUP)
	ONEVENT(ACK_IND,                           		    CONNECTED,          CLegacyToSacTranslator::OnMplAckCONNECTED)
	ONEVENT(ACK_IND,          		                    DISCONNECTING,      CLegacyToSacTranslator::OnMplAckDISCONNECTING)

	ONEVENT(UPDATE_RELAY_AUDIO_PARAMETERS, 				IDLE,               CLegacyToSacTranslator::OnLegacyToSacTranslatorUpdateRelayParamsIDLE)
	ONEVENT(UPDATE_RELAY_AUDIO_PARAMETERS, 				SETUP,              CLegacyToSacTranslator::OnLegacyToSacTranslatorUpdateRelayParamsSETUP)
	ONEVENT(UPDATE_RELAY_AUDIO_PARAMETERS, 				CONNECTED,          CLegacyToSacTranslator::OnLegacyToSacTranslatorUpdateRelayParamsCONNECTED)
	ONEVENT(UPDATE_RELAY_AUDIO_PARAMETERS, 				DISCONNECTING,      CLegacyToSacTranslator::OnLegacyToSacTranslatorUpdateRelayParamsDISCONNECTING)

	ONEVENT(LEGACY_TO_SAC_OPEN_ENCODER_TOUT,            IDLE,               CLegacyToSacTranslator::OnLegacyToSacTranslatorOpenEncoderToutIDLE)
	ONEVENT(LEGACY_TO_SAC_OPEN_ENCODER_TOUT,            SETUP,              CLegacyToSacTranslator::OnLegacyToSacTranslatorOpenEncoderToutSETUP)
	ONEVENT(LEGACY_TO_SAC_OPEN_ENCODER_TOUT,            CONNECTED,          CLegacyToSacTranslator::OnLegacyToSacTranslatorOpenEncoderToutCONNECTED)
	ONEVENT(LEGACY_TO_SAC_OPEN_ENCODER_TOUT,            DISCONNECTING,      CLegacyToSacTranslator::OnLegacyToSacTranslatorOpenEncoderToutDISCONNECTING)

	ONEVENT(LEGACY_TO_SAC_CLOSE_ENCODER_TOUT,            IDLE,               CLegacyToSacTranslator::OnLegacyToSacTranslatorCloseEncoderToutIDLE)
	ONEVENT(LEGACY_TO_SAC_CLOSE_ENCODER_TOUT,            SETUP,              CLegacyToSacTranslator::OnLegacyToSacTranslatorCloseEncoderToutSETUP)
	ONEVENT(LEGACY_TO_SAC_CLOSE_ENCODER_TOUT,            CONNECTED,          CLegacyToSacTranslator::OnLegacyToSacTranslatorCloseEncoderToutCONNECTED)
	ONEVENT(LEGACY_TO_SAC_CLOSE_ENCODER_TOUT,            DISCONNECTING,      CLegacyToSacTranslator::OnLegacyToSacTranslatorCloseEncoderToutDISCONNECTING)




PEND_MESSAGE_MAP(CLegacyToSacTranslator,CStateMachineValidation);

// ------------------------------------------------------------
CLegacyToSacTranslator::CLegacyToSacTranslator ()
{
	m_pBridgePartyAudioIn       = NULL;
	m_pHardwareInterface        = NULL;
	m_lastReqId					= 0xFFFFFFFF;
	m_lastReq					= 0xFFFFFFFF;
	m_bIsRelayParamsUpdated     = false;

}


// ------------------------------------------------------------
CLegacyToSacTranslator::~CLegacyToSacTranslator ()
{
	POBJDELETE(m_pHardwareInterface);
}


// ------------------------------------------------------------
void  CLegacyToSacTranslator::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	  DispatchEvent(opCode,pMsg);
}

// ------------------------------------------------------------
void*  CLegacyToSacTranslator::GetMessageMap()
{
  return (void*)m_msgEntries;
}

//----------------------------------------------------------------
void CLegacyToSacTranslator::Create( const CBridgePartyAudioIn*	pBridgePartyAudioIn, const CRsrcParams* pRsrcParamsAudioEncoder)
{
	m_pBridgePartyAudioIn = (CBridgePartyAudioIn*)pBridgePartyAudioIn;
	// create Hardware interface
	if (m_pHardwareInterface)
			POBJDELETE(m_pHardwareInterface);
	m_pHardwareInterface = new CAudioHardwareInterface();

	if (NULL != m_pHardwareInterface)
		m_pHardwareInterface->Create((CRsrcParams*)pRsrcParamsAudioEncoder);
	else
	{
		TRACEINTO << " LegacyToSac-Translator-Error - m_pHardwareInterface == NULL, PartyName:" << GetFullName();
		PASSERT(1);
	}

}

//----------------------------------------------------------------
void CLegacyToSacTranslator::Connect()
{
	DispatchEvent(CONNECT_LEGACY_TO_SAC_TRANSLATOR, NULL);

}

//----------------------------------------------------------------
void CLegacyToSacTranslator::Disconnect()
{
	DispatchEvent(DISCONNECT_LEGACY_TO_SAC_TRANSLATOR);

}
//----------------------------------------------------------------
void CLegacyToSacTranslator::UpdateAudioRelayParams()
{
	DispatchEvent(UPDATE_RELAY_AUDIO_PARAMETERS);
}
//----------------------------------------------------------------
void CLegacyToSacTranslator::SendOpenAudioSacEncoder()
{
	PASSERTSTREAM_AND_RETURN(!m_pHardwareInterface, "Hardware Interface is NULL");

	WORD networkInterface = H323_INTERFACE_TYPE; //m_pBridgePartyAudioIn->GetNetworkInterface();
	BYTE confSampleRate = m_pBridgePartyAudioIn->GetConfSampleRate();
	BYTE numberOfChannels = 1;
	DWORD volume = m_pBridgePartyAudioIn->GetVolume();
	BOOL isVideoParticipant = m_pBridgePartyAudioIn->GetIsVideoParticipant();
	DWORD ssrc = m_pBridgePartyAudioIn->GetSSRC();
	DWORD ssrcArray[1];
	ssrcArray[0]=ssrc;
	EMixModeGet eMixModeSet = E_MIX_MODE_GET_MYSELF;

	m_lastReqId = ((CAudioHardwareInterface *) m_pHardwareInterface)->OpenEncoder(
		networkInterface,
		confSampleRate,
		numberOfChannels,
		Au_SirenLPR_Scalable_48k,
		FALSE/*isMuted()*/,
		volume,
		isVideoParticipant,
		FALSE/* isStandalone*/,
		100, // volume adjustment (default)
		1 /*m_numOfSsrcIds*/,
		ssrcArray,
		eMixModeSet,
		0/*ivrSSRC not relevant for translator*/,
		FALSE/*isUseSpeakerSsrcForTx*/,
		FALSE/*bIsCallGenerator*/,
		FALSE /*isRelayToMix*/,
		0);

	m_lastReq = AUDIO_OPEN_ENCODER_REQ;
}
//----------------------------------------------------------------
void CLegacyToSacTranslator::SendCloseAudioSacEncoder()
{
	if(!m_pHardwareInterface)
	{
		PASSERT_AND_RETURN(101);
	}

	m_lastReqId = ((CAudioHardwareInterface *) m_pHardwareInterface)->CloseEncoder();
	m_lastReq = AUDIO_CLOSE_ENCODER_REQ;
}
// ------------------------------------------------------------
const char*  CLegacyToSacTranslator::GetFullName()
{
	if (m_pBridgePartyAudioIn)
	{
		CBridgePartyCntl* pBridgePartyCntlPtr = m_pBridgePartyAudioIn->GetBridgePartyCntlPtr();
		if (pBridgePartyCntlPtr)
			return pBridgePartyCntlPtr->GetFullName();
	}
	return "Error-Name ";	// return constant
}


// ------------------------------------------------------------
const char*  CLegacyToSacTranslator::GetConfName()
{
	if (m_pBridgePartyAudioIn)
	{
		CBridgePartyCntl* pBridgePartyCntlPtr = m_pBridgePartyAudioIn->GetBridgePartyCntlPtr();
		if (pBridgePartyCntlPtr)
			return pBridgePartyCntlPtr->GetConfName();
	}
	return "Error-Name ";	// return constant
}
// ------------------------------------------------------------
PartyRsrcID CLegacyToSacTranslator::GetPartyRsrcID()
{
	PartyRsrcID partyId = (PartyRsrcID)0;
	if (m_pBridgePartyAudioIn)
	{
		CBridgePartyCntl* pBridgePartyCntlPtr = m_pBridgePartyAudioIn->GetBridgePartyCntlPtr();
		if (pBridgePartyCntlPtr)
			partyId = pBridgePartyCntlPtr->GetPartyRsrcID();
	}
	else
		PASSERT(1);
	return partyId;

}
//----------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorConnectIDLE(CSegment* pParam)
{
	TRACEINTO << " state changed to SETUP - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	// changing the state
	m_state = SETUP;

	// Open audio SAC encoder
	SendOpenAudioSacEncoder();


	// start Open encoder timer
	StartTimer( LEGACY_TO_SAC_OPEN_ENCODER_TOUT, LEGACY_TO_SAC_OPEN_ENCODER_TOUT_VALUE );


}
//----------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorConnectSETUP(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName() << " ,already in connect process";

}
//----------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName() << ", translator already connected";

	//inform BridgePartyAudoiIn that the translator connected
	m_pBridgePartyAudioIn->LegacyToSacTranslatorConnected(statOK);
}
//----------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();

}
//----------------------------------------------------------------


//----------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnectIDLE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName()<< " , translator already disconnected";
	m_pBridgePartyAudioIn->LegacyToSacTranslatorDisconnected(statOK);
}
//----------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnectSETUP(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	OnLegacyToSacTranslatorDisconnect();
}
//----------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	OnLegacyToSacTranslatorDisconnect();
}
//----------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
}
//----------------------------------------------------------------

void CLegacyToSacTranslator::OnLegacyToSacTranslatorDisconnect()
{
	TRACEINTO << " state changed to DISCONNECTING - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	// changing the state
	m_state = DISCONNECTING;

	// Close audio SAC encoder
	SendCloseAudioSacEncoder();

	// start Close encoder timer
	StartTimer( LEGACY_TO_SAC_CLOSE_ENCODER_TOUT, LEGACY_TO_SAC_CLOSE_ENCODER_TOUT_VALUE );

}

//--------------------------------------------------------------
void CLegacyToSacTranslator::OnMplAckIDLE(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
}
//----------------------------------------------------------------
void  CLegacyToSacTranslator::OnMplAckSETUP( CSegment* pParam )
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch(AckOpcode)
	{
		case	AUDIO_OPEN_ENCODER_REQ:
		{
			OnMplOpenAudioEncoderAck(status);
			break;
		}

		default:
		{
			TRACEINTO << " LegacyToSac-Translator-Error - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName() << " - ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}// end switch
}
//----------------------------------------------------------------
void  CLegacyToSacTranslator::OnMplOpenAudioEncoderAck(STATUS status)
{
	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 21, 1021)	// simulate Ack error from Open Encoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 22)			// simulate no Ack received	from Open Encoder

	TRACEINTO << "PartyId: " << GetPartyRsrcID() << ", PartyName: " << GetFullName() 
		<< ", status = "<< status;
	
	DeleteTimer(LEGACY_TO_SAC_OPEN_ENCODER_TOUT);

	BYTE	responseStatus = statOK;
	
	if (status != STATUS_OK)
	{
		TRACEINTO << "LegacyToSac-Translator-Error - Ack Error - PartyId: " << GetPartyRsrcID() 
			<< ", PartyName: " << GetFullName() << ", status = " << status;
		
		m_pBridgePartyAudioIn->LegacyToSacTranslatorConnected(statAudioInOutResourceProblem);
	}
	else
	{
		SetConnected();
	}
}
//----------------------------------------------------------------

void CLegacyToSacTranslator::SetConnected()
{
	TRACEINTO << "state changed to CONNECTED, PartyId: " << GetPartyRsrcID() 
		<< ", PartyName: " << GetFullName();
	
	m_state = CONNECTED;
	m_pBridgePartyAudioIn->LegacyToSacTranslatorConnected(statOK);
	
	if (m_bIsRelayParamsUpdated)
	{
		SendUpdateRelayParams();
		m_bIsRelayParamsUpdated = false;
	}
}
//-------------------------------------------------------------------
void CLegacyToSacTranslator::OnMplAckCONNECTED(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
}
//-------------------------------------------------------------------
void CLegacyToSacTranslator::OnMplAckDISCONNECTING(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch(AckOpcode)
	{
		case	AUDIO_CLOSE_ENCODER_REQ:
		{

			OnMplCloseAudioEncoderAck(status);
			break;
		}

		default:
		{
			TRACEINTO << " LegacyToSac-Translator-Error - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName() << " - ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}// end switch


}
//-------------------------------------------------------------------


void CLegacyToSacTranslator::OnLegacyToSacTranslatorUpdateRelayParamsIDLE(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();

}
//-------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorUpdateRelayParamsSETUP(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	m_bIsRelayParamsUpdated = true;

}
//-------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorUpdateRelayParamsCONNECTED(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	SendUpdateRelayParams();
}
//-------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorUpdateRelayParamsDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();

}
//------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorOpenEncoderToutIDLE(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
}
//------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorOpenEncoderToutSETUP(CSegment* pParam)
{
	TRACEINTO << " LegacyToSac-Translator-Error - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	PASSERT(GetPartyRsrcID());
	m_pBridgePartyAudioIn->LegacyToSacTranslatorConnected(statAudioInOutResourceProblem);

}

//------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorOpenEncoderToutCONNECTED(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();

}
//------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorOpenEncoderToutDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();

}
//------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorCloseEncoderToutIDLE(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();

}

//------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorCloseEncoderToutSETUP(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();

}

//------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorCloseEncoderToutCONNECTED(CSegment* pParam)
{
	TRACEINTO << " LegacyToSacTranslator-Ignored - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();

}

//------------------------------------------------------------------
void CLegacyToSacTranslator::OnLegacyToSacTranslatorCloseEncoderToutDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << " LegacyToSac-Translator-Error - PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	m_state = IDLE;
	EStat	responseStatus = statAudioInOutResourceProblem;

	//inform BridgePartyAudoiIn that the translator connected & status.
	m_pBridgePartyAudioIn->LegacyToSacTranslatorDisconnected(responseStatus);

}
//-------------------------------------------------------------------
void CLegacyToSacTranslator::SendUpdateRelayParams()
{
	if(!m_pHardwareInterface)
	{
		PASSERT_AND_RETURN(101);
	}
	TRtpUpdateRelayReq stRelayParams;
	stRelayParams.unChannelType = kIpAudioChnlType;
	stRelayParams.unChannelDirection = cmCapReceive;
	DWORD ssrc = m_pBridgePartyAudioIn->GetSSRC();
	stRelayParams.nSSRC.numOfSSRC = 1;
	stRelayParams.nSSRC.ssrcList[0] = ssrc;
	stRelayParams.unIvrSsrc = 0; //relevant only for audio out
	stRelayParams.unIvrCsrc = 0; //relevant only for audio out
	((CAudioHardwareInterface *) m_pHardwareInterface)->UpdateAudioRelayParamsOut( &stRelayParams );
}



//----------------------------------------------------------------
void  CLegacyToSacTranslator::OnMplCloseAudioEncoderAck(STATUS status)
{
	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 23, 1023)	// simulate Ack error from Close Encoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 24)			// simulate no Ack received	from Close Encoder

	DeleteTimer(LEGACY_TO_SAC_CLOSE_ENCODER_TOUT);

	EStat	responseStatus = statOK;
	if ( status != STATUS_OK )
	{
		TRACEINTO << " LegacyToSac-Translator-Error - PartyId: " << GetPartyRsrcID() << " ,status=" << (DWORD)status;
		responseStatus = statAudioInOutResourceProblem; // statAudioOutResourceProblem -> Inorder to initiate Kill port on AudioDecoder AudioEncoder
	}
	TRACEINTO << " state changed to IDLE, PartyId:" << GetPartyRsrcID() << ", PartyName:" << GetFullName();
	m_state = IDLE;


	//inform BridgePartyAudoiIn that the translator connected & status.
	m_pBridgePartyAudioIn->LegacyToSacTranslatorDisconnected(responseStatus);
}
// ------------------------------------------------------------
bool CLegacyToSacTranslator::IsActive()
{
	return (m_state != IDLE);
}
// ------------------------------------------------------------
bool CLegacyToSacTranslator::IsConnected()
{
	return (m_state == CONNECTED);
}
// ------------------------------------------------------------
CRsrcParams* CLegacyToSacTranslator::GetRsrcParams()
{
	if(m_pHardwareInterface)
		return(m_pHardwareInterface->GetRsrcParams());

	return NULL;
}
// ------------------------------------------------------------
void CLegacyToSacTranslator::UpdateNewConfParams (DWORD confRsrcId)
{
	m_pHardwareInterface->SetConfRsrcId(confRsrcId);
}
