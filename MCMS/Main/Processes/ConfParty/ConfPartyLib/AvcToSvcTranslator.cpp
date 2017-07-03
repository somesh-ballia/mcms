//+========================================================================+
//                CAvcToSvcTranslator.CPP                                  +
//+========================================================================+

//includes
#include "AvcToSvcTranslator.h"
#include "ProcessBase.h"
#include "HlogApi.h"
#include "AvcToSvcParams.h"
#include "BridgePartyVideoIn.h"
#include "BridgePartyCntl.h"
#include "VideoHardwareInterface.h"
#include "RsrcParams.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "VideoOperationPointsSet.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"

#include "VideoBridge.h"

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
// ------------------------------------------------------------

// defines

// defines TOUT
#define	OPEN_AVC_TO_SVC_ENCODER_TOUT_VALUE		1500
#define	CLOSE_ENCODER_TOUT_VALUE				1500
#define CONNECT_VIDEO_ENCODER_TOUT_VALUE		1500
#define	CHANGE_LAYOUT_1X1_TOUT_VALUE			1000
#define	FAST_UPDATE_TEST_TIMER_VALUE			20

// defined to be replaced
#define OPEN_AVC_TO_SVC_ENCODER_TOUT    	((WORD)302)
#define	CHANGE_LAYOUT_1X1_TOUT				((WORD)303)
#define	CLOSE_ENCODER_TOUT					((WORD)304)
#define	FAST_UPDATE_TEST_TIMER				((WORD)305)
#define CONNECT_VIDEO_ENCODER_TOUT			((WORD)306)


CAvcToSvcOpenEncoder::CAvcToSvcOpenEncoder()
{
	memset ( &oe, 0, sizeof (OPEN_TRANSLATOR_ENCODER) );
}


// Action Function table
PBEGIN_MESSAGE_MAP(CAvcToSvcTranslator)


	ONEVENT(CONNECT_VIDEO_OUT                 ,IDLE           ,CAvcToSvcTranslator::OnVideoBridgePartyConnectIDLE)
	ONEVENT(CONNECT_VIDEO_OUT                 ,SETUP          ,CAvcToSvcTranslator::OnVideoBridgePartyConnectSETUP)
	ONEVENT(CONNECT_VIDEO_OUT                 ,CHANGE_LAYOUT  ,CAvcToSvcTranslator::OnVideoBridgePartyConnectCHANGELAYOUT)
	ONEVENT(CONNECT_VIDEO_OUT                 ,CONNECTED      ,CAvcToSvcTranslator::OnVideoBridgePartyConnectCONNECTED)
	ONEVENT(CONNECT_VIDEO_OUT                 ,DISCONNECTING  ,CAvcToSvcTranslator::OnVideoBridgePartyConnectDISCONNECTING)

	ONEVENT(DISCONNECT_VIDEO_OUT              ,IDLE           ,CAvcToSvcTranslator::OnVideoBridgePartyDisConnectIDLE)
	ONEVENT(DISCONNECT_VIDEO_OUT              ,SETUP          ,CAvcToSvcTranslator::OnVideoBridgePartyDisConnectSETUP)
	ONEVENT(DISCONNECT_VIDEO_OUT              ,CHANGE_LAYOUT  ,CAvcToSvcTranslator::OnVideoBridgePartyDisConnectCHANGELAYOUT)
	ONEVENT(DISCONNECT_VIDEO_OUT              ,CONNECTED      ,CAvcToSvcTranslator::OnVideoBridgePartyDisConnectCONNECTED)
	ONEVENT(DISCONNECT_VIDEO_OUT              ,DISCONNECTING  ,CAvcToSvcTranslator::OnVideoBridgePartyDisConnectDISCONNECTING)

	ONEVENT(ACK_IND                           ,IDLE           ,CAvcToSvcTranslator::OnMplAckIDLE)
	ONEVENT(ACK_IND                           ,SETUP          ,CAvcToSvcTranslator::OnMplAckSETUP)
	ONEVENT(ACK_IND                           ,CHANGE_LAYOUT  ,CAvcToSvcTranslator::OnMplAckCHANGELAYOUT)
	ONEVENT(ACK_IND                           ,CONNECTED      ,CAvcToSvcTranslator::OnMplAckCONNECTED)
	ONEVENT(ACK_IND                           ,DISCONNECTING  ,CAvcToSvcTranslator::OnMplAckDISCONNECTING)

	ONEVENT(CONNECT_VIDEO_ENCODER_TOUT        ,SETUP          ,CAvcToSvcTranslator::OnTimerConnectEncoderSETUP)
	ONEVENT(CONNECT_VIDEO_ENCODER_TOUT        ,ANYCASE  	  ,CAvcToSvcTranslator::NullActionFunction)

	ONEVENT(OPEN_AVC_TO_SVC_ENCODER_TOUT      ,IDLE           ,CAvcToSvcTranslator::OnTimerOpenEncoderIDLE)
	ONEVENT(OPEN_AVC_TO_SVC_ENCODER_TOUT      ,SETUP          ,CAvcToSvcTranslator::OnTimerOpenEncoderSETUP)
	ONEVENT(OPEN_AVC_TO_SVC_ENCODER_TOUT      ,CHANGE_LAYOUT  ,CAvcToSvcTranslator::OnTimerOpenEncoderCHANGELAYOUT)
	ONEVENT(OPEN_AVC_TO_SVC_ENCODER_TOUT      ,CONNECTED      ,CAvcToSvcTranslator::OnTimerOpenEncoderCONNECTED)
	ONEVENT(OPEN_AVC_TO_SVC_ENCODER_TOUT      ,DISCONNECTING  ,CAvcToSvcTranslator::OnTimerOpenEncoderDISCONNECTING)

	ONEVENT(CHANGE_LAYOUT_1X1_TOUT            ,IDLE           ,CAvcToSvcTranslator::OnTimerChangeLayoutIDLE)
	ONEVENT(CHANGE_LAYOUT_1X1_TOUT            ,SETUP          ,CAvcToSvcTranslator::OnTimerChangeLayoutSETUP)
	ONEVENT(CHANGE_LAYOUT_1X1_TOUT            ,CHANGE_LAYOUT  ,CAvcToSvcTranslator::OnTimerChangeLayoutCHANGELAYOUT)
	ONEVENT(CHANGE_LAYOUT_1X1_TOUT            ,CONNECTED      ,CAvcToSvcTranslator::OnTimerChangeLayoutCONNECTED)
	ONEVENT(CHANGE_LAYOUT_1X1_TOUT            ,DISCONNECTING  ,CAvcToSvcTranslator::OnTimerChangeLayoutDISCONNECTING)

	ONEVENT(CLOSE_ENCODER_TOUT   	          ,IDLE           ,CAvcToSvcTranslator::OnTimerCloseEncoderIDLE)
	ONEVENT(CLOSE_ENCODER_TOUT         	   	  ,SETUP          ,CAvcToSvcTranslator::OnTimerCloseEncoderSETUP)
	ONEVENT(CLOSE_ENCODER_TOUT        	      ,CHANGE_LAYOUT  ,CAvcToSvcTranslator::OnTimerCloseEncoderCHANGELAYOUT)
	ONEVENT(CLOSE_ENCODER_TOUT        	      ,CONNECTED      ,CAvcToSvcTranslator::OnTimerCloseEncoderCONNECTED)
	ONEVENT(CLOSE_ENCODER_TOUT         		  ,DISCONNECTING  ,CAvcToSvcTranslator::OnTimerCloseEncoderDISCONNECTING)

	ONEVENT(FASTUPDATE                        ,IDLE           ,CAvcToSvcTranslator::NullActionFunction)
	ONEVENT(FASTUPDATE                        ,SETUP          ,CAvcToSvcTranslator::NullActionFunction)
	ONEVENT(FASTUPDATE                        ,CHANGE_LAYOUT  ,CAvcToSvcTranslator::OnVideoBridgePartyFastUpdateCHANGELAYOUT)
	ONEVENT(FASTUPDATE                        ,CONNECTED      ,CAvcToSvcTranslator::OnVideoBridgePartyFastUpdateCONNECTED)
	ONEVENT(FASTUPDATE                        ,DISCONNECTING  ,CAvcToSvcTranslator::OnVideoBridgePartyFastUpdateDISCONNECTING)

	ONEVENT(REQUEST_SEND_CHANGE_LAYOUT        ,IDLE           ,CAvcToSvcTranslator::NullActionFunction)
	ONEVENT(REQUEST_SEND_CHANGE_LAYOUT        ,SETUP          ,CAvcToSvcTranslator::IgnoreFunction)
	ONEVENT(REQUEST_SEND_CHANGE_LAYOUT        ,CHANGE_LAYOUT  ,CAvcToSvcTranslator::OnEncoderResolutionChangedCHANGELAYOUT)
	ONEVENT(REQUEST_SEND_CHANGE_LAYOUT        ,CONNECTED      ,CAvcToSvcTranslator::OnEncoderResolutionChangedCONNECTED)
	ONEVENT(REQUEST_SEND_CHANGE_LAYOUT        ,DISCONNECTING  ,CAvcToSvcTranslator::IgnoreFunction)

	ONEVENT(FAST_UPDATE_TEST_TIMER            ,CONNECTED  ,CAvcToSvcTranslator::OnTimerSendRelayIntraRequestToAvcToSvcTranslator)

PEND_MESSAGE_MAP(CAvcToSvcTranslator,CStateMachine);

// General flow:
// 1. connect Encoder to the suitable ART
// 2. upon Ack, Open Encoder
// 3. Upon Ack, change layout (1x1)
// 4. Upon Ack, moves to CONNECTED
// 5. Update Avc Image with SVC stream params
// 6. Send UpdateImage to all Relay Participants
// 7. Send Notification to all Relay EPs about new streams



// ------------------------------------------------------------
CAvcToSvcTranslator::CAvcToSvcTranslator ()
{
	m_pBridgePartyCntl			= NULL;
	m_pHardwareInterfaceEncoder = NULL;
	m_pBridgePartyVideoIn		= NULL;
	m_lastReqId					= 0xFFFFFFFF;
	m_lastReq					= 0xFFFFFFFF;
	m_layerId					= 0;
	m_resHight					= 0;
	m_resWidth					= 0;
	m_ssrc						= 0;
	m_channelHandle				= 0;
	m_EncoderConnectionId		= 0;
	m_tDelayIntraTime			= GetDelayIntraTime();
	m_bConnectToSACEncoder		= false;
	m_bIsReadyToBeKilled		= false;
	m_bIsTranslatorError		= false;
	m_iChangeLayoutCounter		= 0;
	m_partyRsrcId				= 0;
	m_bResendChangeLayout		= false;	// request to send change-layout again (real change or just Decoder changed)
	m_bSentOpenEncoder			= false;
	m_bIsVswStream				= false;
}

// ------------------------------------------------------------
CAvcToSvcTranslator::~CAvcToSvcTranslator ()
{
	POBJDELETE(m_pHardwareInterfaceEncoder);
	TRACEINTO << " Translator Distracted";
}

// ------------------------------------------------------------
CAvcToSvcTranslator::CAvcToSvcTranslator (const CAvcToSvcTranslator& rOtherAvcToSvcTranslator) : CStateMachine()
{
	m_pBridgePartyCntl      = rOtherAvcToSvcTranslator.m_pBridgePartyCntl;
	m_pBridgePartyVideoIn	= rOtherAvcToSvcTranslator.m_pBridgePartyVideoIn;

	if (NULL != rOtherAvcToSvcTranslator.m_pHardwareInterfaceEncoder)
	{
		m_pHardwareInterfaceEncoder = new CVideoHardwareInterface();
		//(CHardwareInterface)(*m_pHardwareInterfaceEncoder) = (CHardwareInterface)(*(rOtherAvcToSvcTranslator.m_pHardwareInterfaceEncoder));
		CRsrcParams* pRsrcParamsVideo = rOtherAvcToSvcTranslator.m_pHardwareInterfaceEncoder->GetRsrcParams();
		if (pRsrcParamsVideo)
			m_pHardwareInterfaceEncoder->Create( pRsrcParamsVideo );

//		m_pHardwareInterfaceEncoder = new CVideoHardwareInterface();
//		(CHardwareInterface)(*m_pHardwareInterfaceEncoder) = (CHardwareInterface)(*(rOtherAvcToSvcTranslator.m_pHardwareInterfaceEncoder));
	}
	else
		m_pHardwareInterfaceEncoder = NULL;

	m_lastReqId				= rOtherAvcToSvcTranslator.m_lastReqId;
	m_lastReq				= rOtherAvcToSvcTranslator.m_lastReq;
	m_resolution			= rOtherAvcToSvcTranslator.m_resolution;
	m_layerId				= rOtherAvcToSvcTranslator.m_layerId;
	m_resHight				= rOtherAvcToSvcTranslator.m_resHight;
	m_resWidth				= rOtherAvcToSvcTranslator.m_resWidth;
	m_ssrc					= rOtherAvcToSvcTranslator.m_ssrc;
	m_channelHandle			= rOtherAvcToSvcTranslator.m_channelHandle;
	m_EncoderConnectionId	= rOtherAvcToSvcTranslator.m_EncoderConnectionId;
	m_tDelayIntraTime		= rOtherAvcToSvcTranslator.m_tDelayIntraTime;
	m_bConnectToSACEncoder	= rOtherAvcToSvcTranslator.m_bConnectToSACEncoder;
	m_bIsReadyToBeKilled	= rOtherAvcToSvcTranslator.m_bIsReadyToBeKilled;
	m_bIsTranslatorError	= rOtherAvcToSvcTranslator.m_bIsTranslatorError;
	m_iChangeLayoutCounter	= rOtherAvcToSvcTranslator.m_iChangeLayoutCounter;
	m_partyRsrcId			= rOtherAvcToSvcTranslator.m_partyRsrcId;
	m_bResendChangeLayout	= rOtherAvcToSvcTranslator.m_bResendChangeLayout;
	m_bSentOpenEncoder		= rOtherAvcToSvcTranslator.m_bSentOpenEncoder;
	m_bIsVswStream			= rOtherAvcToSvcTranslator.m_bIsVswStream;
}

//--------------------------------------------------------------------------
CAvcToSvcTranslator& CAvcToSvcTranslator::operator=(const CAvcToSvcTranslator& rOtherAvcToSvcTranslator)
{
	if (&rOtherAvcToSvcTranslator == this) 
		return *this;

	m_state = rOtherAvcToSvcTranslator.m_state;

	m_pBridgePartyCntl      = rOtherAvcToSvcTranslator.m_pBridgePartyCntl;
	m_pBridgePartyVideoIn	= rOtherAvcToSvcTranslator.m_pBridgePartyVideoIn;

	POBJDELETE( m_pHardwareInterfaceEncoder );
	if (NULL != rOtherAvcToSvcTranslator.m_pHardwareInterfaceEncoder)
	{
		m_pHardwareInterfaceEncoder = new CVideoHardwareInterface();
		//(CHardwareInterface)(*m_pHardwareInterfaceEncoder) = (CHardwareInterface)(*(rOtherAvcToSvcTranslator.m_pHardwareInterfaceEncoder));
		CRsrcParams* pRsrcParamsVideo = rOtherAvcToSvcTranslator.m_pHardwareInterfaceEncoder->GetRsrcParams();
		if (pRsrcParamsVideo)
			m_pHardwareInterfaceEncoder->Create( pRsrcParamsVideo );

	}

	m_lastReqId				= rOtherAvcToSvcTranslator.m_lastReqId;
	m_lastReq				= rOtherAvcToSvcTranslator.m_lastReq;
	m_resolution			= rOtherAvcToSvcTranslator.m_resolution;
	m_layerId				= rOtherAvcToSvcTranslator.m_layerId;
	m_resHight				= rOtherAvcToSvcTranslator.m_resHight;
	m_resWidth				= rOtherAvcToSvcTranslator.m_resWidth;
	m_ssrc					= rOtherAvcToSvcTranslator.m_ssrc;
	m_channelHandle			= rOtherAvcToSvcTranslator.m_channelHandle;
	m_EncoderConnectionId	= rOtherAvcToSvcTranslator.m_EncoderConnectionId;
	m_tDelayIntraTime		= rOtherAvcToSvcTranslator.m_tDelayIntraTime;
	m_bConnectToSACEncoder	= rOtherAvcToSvcTranslator.m_bConnectToSACEncoder;
	m_bIsReadyToBeKilled	= rOtherAvcToSvcTranslator.m_bIsReadyToBeKilled;
	m_bIsTranslatorError	= rOtherAvcToSvcTranslator.m_bIsTranslatorError;
	m_iChangeLayoutCounter	= rOtherAvcToSvcTranslator.m_iChangeLayoutCounter;
	m_partyRsrcId			= rOtherAvcToSvcTranslator.m_partyRsrcId;
	m_bResendChangeLayout	= rOtherAvcToSvcTranslator.m_bResendChangeLayout;
	m_bSentOpenEncoder		= rOtherAvcToSvcTranslator.m_bSentOpenEncoder;
	m_bIsVswStream			= rOtherAvcToSvcTranslator.m_bIsVswStream;

	return *this;
}

void  CAvcToSvcTranslator::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	  DispatchEvent(opCode,pMsg);
}

// ------------------------------------------------------------
void*  CAvcToSvcTranslator::GetMessageMap()
{
  return (void*)m_msgEntries;
}

//// ------------------------------------------------------------
//DWORD CAvcToSvcTranslator::GetMyId()
//{
//	return m_ssrc;
//}

// ------------------------------------------------------------
bool  CAvcToSvcTranslator::IsConnected()
{
	return (CONNECTED == m_state);
}

// ------------------------------------------------------------
bool CAvcToSvcTranslator::IsActive()
{
	return (m_state != IDLE);
}

// ------------------------------------------------------------
DWORD CAvcToSvcTranslator::GetEncoderConnectionId()
{
	return m_EncoderConnectionId;
}

// ------------------------------------------------------------
bool  CAvcToSvcTranslator::IsReadyToBeKilled()
{
	return m_bIsReadyToBeKilled;
}

// ------------------------------------------------------------
DWORD CAvcToSvcTranslator::GetDelayIntraTime()
{
	DWORD tDelayIntra = FAST_UPDATE_TEST_TIMER_VALUE;	// currently 200 ms
	std::string key = CFG_KEY_AVC_TO_SVC_INTRA_DELAY;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    pSysConfig->GetDWORDDataByKey(key, tDelayIntra);
    if (tDelayIntra > 100)
    	tDelayIntra = 100;	// in any case not more than 1 second

	return tDelayIntra;
}


// ======================================================= Public Functions from PartyVideoIn =====================================================================

// ------------------------------------------------------------
void CAvcToSvcTranslator::Create (const CBridgePartyCntl* pBridgePartyCntl, const CBridgePartyVideoIn* pBridgePartyVideoIn, const CRsrcParams* pRsrcParamsVideo, bool bConnectToSACEncoder)
{
	m_pBridgePartyCntl = (CBridgePartyCntl*)pBridgePartyCntl;
	m_pBridgePartyVideoIn = (CBridgePartyVideoIn*)pBridgePartyVideoIn;
	m_bConnectToSACEncoder = bConnectToSACEncoder;
	m_partyRsrcId = pRsrcParamsVideo->GetPartyRsrcId();

	if (FALSE == IsVswStream())
	{
	// create HW I/F Decoder
	if (NULL == m_pHardwareInterfaceEncoder)
		m_pHardwareInterfaceEncoder = new CVideoHardwareInterface;
	m_pHardwareInterfaceEncoder->Create((CRsrcParams*)pRsrcParamsVideo);
	m_EncoderConnectionId = pRsrcParamsVideo->GetConnectionId();
		//m_partyRsrcId = m_pHardwareInterfaceEncoder->GetPartyRsrcId();
	}

	TRACEINTO << " (state set to IDLE) , PartyRsrcId: " << m_partyRsrcId << ", Party Name: " << GetMyFullName() << ", Encoder ConId: " << m_EncoderConnectionId;
	m_state = IDLE;
}

// ------------------------------------------------------------
void CAvcToSvcTranslator::UpdateParams ( int layerId, DWORD resHight, DWORD resWidth, DWORD ssrc, DWORD channelHandle )
{
	m_layerId = layerId;
	m_resHight = resHight;
	m_resWidth = resWidth;
	m_ssrc = ssrc;
	m_channelHandle = channelHandle;

	TRACEINTO 	<< "PartyRsrcId: " << m_partyRsrcId
						<< " \n Encoder Connection ID = " << m_EncoderConnectionId
						<< " \n m_layerId = " << m_layerId
						<< " \n ssrc = " << m_ssrc;

}


///############################ calls from outside start ###########################################
//

// ------------------------------------------------------------
void CAvcToSvcTranslator::Connect ()
{
	TRACEINTO << " - name: " << GetMyFullName() << ", PartyRsrcId: " << (DWORD)m_partyRsrcId;
	  DispatchEvent(CONNECT_VIDEO_OUT, NULL);
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::Disconnect()
{
	TRACEINTO << " - name: " << GetMyFullName() << ", PartyRsrcId: " << (DWORD)m_partyRsrcId;
	DispatchEvent(DISCONNECT_VIDEO_OUT, NULL);
}

//// ------------------------------------------------------------
//void  CAvcToSvcTranslator::InformOnDecoderSync()
//{
//	TRACEINTO << " - " << GetMyFullName();
////	DispatchEvent(VIDEO_DECODER_SYNC_IND, NULL);
//}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::UpdateImage()
{
	UpdateImageUponAVCtoSVCReady();
}

// ------------------------------------------------------------
void CAvcToSvcTranslator::RemoveConfParams()
{
	if (m_pHardwareInterfaceEncoder)
	m_pHardwareInterfaceEncoder->SetConfRsrcId( INVALID );
}

// ------------------------------------------------------------
void CAvcToSvcTranslator::UpdateNewConfParams (const CBridgePartyVideoIn* pBridgePartyVideoIn, DWORD confRsrcId)
{
	if (m_pHardwareInterfaceEncoder)
	m_pHardwareInterfaceEncoder->SetConfRsrcId( confRsrcId );
	m_pBridgePartyVideoIn = (CBridgePartyVideoIn*)pBridgePartyVideoIn;
}

void CAvcToSvcTranslator::RequestChangeLayout()
{
	TRACEINTO << " PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
	DispatchEvent(REQUEST_SEND_CHANGE_LAYOUT, NULL);
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::NullActionFunction( CSegment* pParam )
{
	TRACEINTO << " - AVC-Translator-Error - !!! (unexpected state, probably an error), state: " << (DWORD)m_state << ", - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
}
void  CAvcToSvcTranslator::IgnoreFunction( CSegment* pParam )
{
	TRACEINTO << " - AVC-Translator-Ignore - (expected state, not an error), state: " << (DWORD)m_state << ", - " << GetMyFullName();
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyConnectIDLE( CSegment* pParam )
{
	TRACEINTO << " (state changed to SETUP) , Encoder Connection ID = " << m_EncoderConnectionId << " - " << GetMyFullName() << ", m_layerId = " << m_layerId;
	m_state = SETUP;

	m_bIsTranslatorError = false;	// reset the flag each time we start the translator

	// connect Encoder to the suitable ART
	EStat status = ConnectEncoderToART();
	if (statOK != status)
	{
		TRACEINTO << " - AVC-Translator-Error - in connecting Encoder to ART, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
		InformConnectEnded( statVideoInOutResourceProblem );
		return;
	}

	// start timer for connect encoder
	StartTimer( CONNECT_VIDEO_ENCODER_TOUT, CONNECT_VIDEO_ENCODER_TOUT_VALUE );
}

// ------------------------------------------------------------
EStat  CAvcToSvcTranslator::ConnectEncoderToART()
{
	ConnectionID rtpConnectionId 	 = DUMMY_CONNECTION_ID;
	ConnectionID encoderConnectionId = m_pHardwareInterfaceEncoder->GetConnectionId();

	DWORD partyRsrcId = m_pHardwareInterfaceEncoder->GetPartyRsrcId();

	enum eLogicalResourceTypes eRsrcType;
	if (m_bConnectToSACEncoder)
		eRsrcType = eLogical_relay_avc_to_svc_rtp_with_audio_encoder;
	else
		eRsrcType = eLogical_relay_avc_to_svc_rtp;

	CRsrcDesc* pRsrcDesc =  ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc( partyRsrcId, eRsrcType );

	// ISDEBUGMODE_RETURN_STAT("TRANSLATOR", 11, statInconsistent) // simulate error getting resources from routing table
	ISDEBUGMODE_SET_VAL("TRANSLATOR", 11, pRsrcDesc, 0)

	if(pRsrcDesc)
	{
		rtpConnectionId = pRsrcDesc->GetConnectionId();
		m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterfaceEncoder)->SendConnect( rtpConnectionId, encoderConnectionId, partyRsrcId, partyRsrcId );
		m_lastReq = TB_MSG_CONNECT_REQ;
	}
	else
	{
		DBGPASSERT(101);	// not return as we want to fail on timer in case of error
		TRACEINTO << " - AVC-Translator-Error - pRsrcDesc=NULL, - " << GetMyFullName() << ", eRsrcType: " << (DWORD)eRsrcType << ", EncoderConnectionID: " << m_EncoderConnectionId;
		return statInconsistent;
	}


	return statOK;
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplConnectEncoderArtAck(STATUS status)
{
	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 12,1012)	// simulate Ack error from Connect Encoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 13)			// simulate no Ack received	from Connect Encoder

	// stop Connect Encoder-ART Timer
	DeleteTimer( CONNECT_VIDEO_ENCODER_TOUT );


	if (status != STATUS_OK)
	{
		// inform connect AvcToSvc Translator failed
		InformConnectEnded( statVideoInOutResourceProblem );
		TRACEINTO << " - AVC-Translator-Error - Ack on connecting Encoder to ART, - " << GetMyFullName() << ", Status: " << (DWORD)status << ", EncoderConnectionID: " << m_EncoderConnectionId;
		DBGPASSERT(101);
		return;
	}

	// Prepare Open Encoder params
	CAvcToSvcOpenEncoder openEncoder;
	EStat prepareStatus = PrepareOpenTranslatorEncoder( &openEncoder.oe );
	if (statOK != prepareStatus)
	{
		TRACEINTO << " - AVC-Translator-Error - Ack on connecting Encoder to ART, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
		InformConnectEnded( statVideoInOutResourceProblem );
		return;
	}

	// Send Open Encoder AVC to SVC (TB_MSG_OPEN_PORT_REQ)
	if (m_pHardwareInterfaceEncoder)
	{
		TRACEINTO << " - AvcToSvcTranslator Open Encoder (and saved the action), -: " << GetMyFullName() << ", LayerId: " << m_layerId;
		m_lastReqId = m_pHardwareInterfaceEncoder->SendOpenEncoderAvcToSvc( &openEncoder );
		m_lastReq = TB_MSG_OPEN_PORT_REQ;
		m_bSentOpenEncoder = true;
	}
	else
	{
		TRACEINTO << " - AVC-Translator-Error - m_pHardwareInterfaceEncoder=NULL, Party Name: " << GetMyFullName() << ", LayerId: " << m_layerId << ", EncoderConnectionID: " << m_EncoderConnectionId;
	}

	// start OpenEncoder Timer
	StartTimer(OPEN_AVC_TO_SVC_ENCODER_TOUT ,OPEN_AVC_TO_SVC_ENCODER_TOUT_VALUE);
}

// ------------------------------------------------------------
EStat  CAvcToSvcTranslator::PrepareOpenTranslatorEncoder(OPEN_TRANSLATOR_ENCODER *ote)
{
	memset( ote, 0, sizeof(OPEN_TRANSLATOR_ENCODER) );

	// get params
	const CAvcToSvcParams* pAvcToSvcParams = NULL;
	if (m_pBridgePartyVideoIn)
		pAvcToSvcParams = m_pBridgePartyVideoIn->GetpAvcToSvcParams();
	if (!pAvcToSvcParams)
	{
		DBGPASSERT(101);
		TRACEINTO << " - AVC-Translator-Error - pAvcToSvcParams=NULL, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
		return statInconsistent;
	}
	CVideoOperationPointsSet* pVOP = pAvcToSvcParams->GetVideoOperationPointsSet();
	if (!pVOP)
	{
		DBGPASSERT(102);
		TRACEINTO << " - AVC-Translator-Error - pVOP=NULL, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
		return statInconsistent;
	}

	VideoOperationPoint videoOperationPoint;
	bool bFound = pVOP->GetOperationPointFromList(m_layerId, videoOperationPoint);
	if (!bFound)
	{
		DBGPASSERT(103);
		TRACEINTO << " - AVC-Translator-Error - bFound=NULL, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
		return statInconsistent;
	}
	BYTE tid = videoOperationPoint.m_tid+1;
	if ((tid) > MAX_NUMBER_OF_TEMPORAL_LAYERS)
	{
		DBGPASSERT(104);
		TRACEINTO << " - AVC-Translator-Error - tid illegal, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId ;
		return statInconsistent;
	}

	BYTE did = videoOperationPoint.m_did;

	ote->videoConfType 					= eVideoConfTypeCP;
	ote->nVideoEncoderType 				= E_VIDEO_ENCODER_NORMAL;
	ote->videoBitRate 					= 360000;	// xxx - not really important, there is a bit-rate per temporalLayer
	ote->videoAlg 						= SVC;		// E_VIDEO_PROTOCOL_H264_SVC ?
	ote->sampleAspectRatio 				= E_VIDEO_RES_ASPECT_RATIO_16_9;	// (0 was not good for mpproxy, it translate it to 3x4)
	ote->videoResolutionTableType 		= E_VIDEO_RESOLUTION_TABLE_REGULAR;
	ote->parsingMode 					= E_PARSING_MODE_CP;
	ote->nFpsMode 						= E_FPS_AUTO;
	ote->bEnableMbRefresh 				= FALSE;
	ote->bIsTipMode 					= FALSE;
	ote->bIsLinkEncoder 				= FALSE;
	ote->bUseIntermediateSDResolution 	= FALSE;
	ote->tCroppingParams.nHorizontalCroppingPercentage 	= 50;
	ote->tCroppingParams.nVerticalCroppingPercentage 	= 50;

	ote->tH264SvcVideoParams.unSsrcID 					= m_ssrc;
	ote->tH264SvcVideoParams.nResolutionWidth 			= m_resWidth;
	ote->tH264SvcVideoParams.nResolutionHeight 			= m_resHight;
	ote->tH264SvcVideoParams.unNumberOfTemporalLayers 	= tid;
	ote->tH264SvcVideoParams.nProfile 					= GetProfile(videoOperationPoint.m_videoProfile);
	ote->tH264SvcVideoParams.unPacketPayloadFormat 		= eVideoPacketPayloadFormatSingleUnit;

	for (DWORD i=0; i<tid; i++)	// tid=number of temporal layers (can be 1-3)
	{
		bFound = pVOP->GetOperationPointFromListTidDid( did, i, videoOperationPoint);
		if (!bFound)
		{
			DBGPASSERT(105);
			TRACEINTO << " - AVC-Translator-Error - bFound=NULL, - " << GetMyFullName()<< ", EncoderConnectionID: " << m_EncoderConnectionId;
			return statInconsistent;
		}
		ote->tH264SvcVideoParams.atSvcTemporalLayer[i].nResolutionFrameRate = GetFameRate(videoOperationPoint.m_frameRate);  	// should it be simply  videoOperationPoint.m_frameRate/256?
		ote->tH264SvcVideoParams.atSvcTemporalLayer[i].nBitRate 			= videoOperationPoint.m_maxBitRate * 1000;			// e.g. 512 * 1000 = 512000
	}

	ote->bIsCallGenerator = m_pBridgePartyCntl->GetIsCallGeneratorConference();

	return statOK;
}

// ------------------------------------------------------------
eVideoFrameRate CAvcToSvcTranslator::GetFameRate(DWORD OperationPointFrameRate)
{
	switch (OperationPointFrameRate)
	{
	case 1920: return eVideoFrameRate7_5FPS;
	case 3840: return eVideoFrameRate15FPS;
	case 7680: return eVideoFrameRate30FPS;
	default:{ PASSERT(105); break;}
	}

	TRACEINTO << " - AVC-Translator-Error - unexpected framerate, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;

	return eVideoFrameRateDUMMY;	// error
}

// ------------------------------------------------------------
eVideoProfile  CAvcToSvcTranslator::GetProfile( WORD videoProfile)
{
	switch (videoProfile)
	{
		case SVC_Profile_High: 		return eVideoProfileHigh;
		case SVC_Profile_BaseLine: 	return eVideoProfileBaseline;
		default: { PASSERT(104); break;}
	}

	TRACEINTO << " - AVC-Translator-Error - unexpected Profile, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;

	return eVideoProfileDummy;	// error
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyConnectSETUP( CSegment* pParam )
{
	TRACEINTO << " - Connect while in connecting , nothing to do, PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}
void  CAvcToSvcTranslator::OnVideoBridgePartyConnectCHANGELAYOUT( CSegment* pParam )
{
	TRACEINTO << " - Connect while in connecting , nothing to do, PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}
void  CAvcToSvcTranslator::OnVideoBridgePartyConnectCONNECTED( CSegment* pParam )
{
	TRACEINTO << " (unexpected state for Connect) , PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
	InformConnectEnded( statOK );
}
void  CAvcToSvcTranslator::OnVideoBridgePartyConnectDISCONNECTING( CSegment* pParam )
{
	TRACEINTO << " (unexpected state for Connect) , PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
	// Amir: maybe to inform: connectEnded with error? (currently the decision is not to reply)
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyDisConnectIDLE( CSegment* pParam )
{
	TRACEINTO << " (unexpected state for Disconnect: ignore, PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyDisConnectSETUP( CSegment* pParam )
{
	TRACEINTO << " (Disconnect while SETUP) ,  PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;

	// stop Open/connect Encoder-AVC-to-SVC Timer
	DeleteTimer( CONNECT_VIDEO_ENCODER_TOUT );
	DeleteTimer( OPEN_AVC_TO_SVC_ENCODER_TOUT );

	// Close Encoder-AVC-to-SVC
	if (false == m_bSentOpenEncoder)
	{
		InformDisconnectEnded( statOK );
		TRACEINTO << " Not Sending Close Encoder, name: " << GetMyFullName() << ", PartyRsrcId: " << m_partyRsrcId << ", m_layerId: " << m_layerId;
	}
	else
		CloseEncoderAvcToSvc();
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyDisConnectCHANGELAYOUT( CSegment* pParam )
{
	TRACEINTO << " (Disconnect while CHANGELAYOUT) , PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;

	// stop timer
	DeleteTimer( CHANGE_LAYOUT_1X1_TOUT );

	// Close Encoder-AVC-to-SVC
	CloseEncoderAvcToSvc();
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyDisConnectCONNECTED( CSegment* pParam )
{
	TRACEINTO << " , PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;

	// xxx - what about only this Encoder???? (Old: notify all SVC EPS about stop this "virtual" SVC)
	NotifySvcEpsOnDisconnectAvcToSvcTranslator();

	// update Image (remove all streams if exist)
	UpdateImageUponAVCtoSVCClosing();

	// Close Encoder-AVC-to-SVC
	CloseEncoderAvcToSvc();
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::UpdateImageUponAVCtoSVCClosing()
{
	PTRACE(eLevelInfoNormal,"CAvcToSvcTranslator::UpdateImageUponAVCtoSVCClosing ");

	if (!m_pBridgePartyVideoIn)
		{PASSERT_AND_RETURN(101);}


	// xxx - at this time, we close ALL the streams (decided) as we don't have a situation of request to close only 1 encoder
	CImage* pImage = (CImage*)m_pBridgePartyVideoIn->GetPartyImage();
	if (!pImage)
		{PASSERT_AND_RETURN(101);}

	pImage->DeleteAndChangeRelayParams(); // set the image as "no Relay"
	// pImage->DeleteAndChangeRelayParams( m_ssrc ); // set the image as "no Relay"
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyDisConnectDISCONNECTING( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnVideoBridgePartyDisConnectDISCONNECTING - (request for disconnect while disconnecting, ignore) , name: ", GetMyFullName());
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::CloseEncoderAvcToSvc()
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::CloseEncoderAvcToSvc - (state changed to DISCONNECTING) , - ", GetMyFullName());
	m_state = DISCONNECTING;

	// Close Encoder (TB_MSG_CLOSE_PORT_REQ)
	if (m_pHardwareInterfaceEncoder)
	{
		m_lastReqId = m_pHardwareInterfaceEncoder->SendCloseEncoder();
		m_lastReq = TB_MSG_CLOSE_PORT_REQ;
		m_bSentOpenEncoder = false;	// reset this parameter for future
	}
	else
	{
		TRACEINTO << " - AVC-Translator-Error - m_pHardwareInterfaceEncoder=NULL " << ", EncoderConnectionID: " << m_EncoderConnectionId;
		PASSERT(101);
	}

	StartTimer( CLOSE_ENCODER_TOUT, CLOSE_ENCODER_TOUT_VALUE );	// xxx -not sure we need it
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::NotifySvcEpsOnDisconnectAvcToSvcTranslator()
{
	if (m_pHardwareInterfaceEncoder)
	{
		DWORD partyRsrcId = m_pHardwareInterfaceEncoder->GetPartyRsrcId();
		if (m_pBridgePartyVideoIn)
		{
			m_pBridgePartyVideoIn->NotifyOnRemovedAvcSvcVideoStreams( partyRsrcId );
			//m_pBridgePartyVideoIn->NotifyOnRemovedAvcSvcVideoStreams( partyRsrcId, m_ssrc);	// send SSRC to notify on this one only
		}
	}
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplAckIDLE( CSegment* pParam )
{
	TRACEINTO << " (unexpected state for Ack), PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplAckSETUP( CSegment* pParam )
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

//	TRACEINTO << " ack_seq_num: " <<  ack_seq_num << "Encoder Connection ID = " << m_EncoderConnectionId;

	if (m_bIsTranslatorError)
	{
		TRACEINTO << " This translator already report on error, - " << GetMyFullName() << ", Encoder Connection ID = " << m_EncoderConnectionId;
		return;
	}

	switch (AckOpcode)
	{
		case TB_MSG_CONNECT_REQ:
		{
			OnMplConnectEncoderArtAck(status);
			break;
		}

		case TB_MSG_OPEN_PORT_REQ:
		{
			OnMplOpenPortAck(status);
			break;
		}

		default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << " ACK_IND Ignored! - AVC-Translator-Error - Ack Opcode: " << str.c_str() << " - "  << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
		}
	}// end switch
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplAckCHANGELAYOUT( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CCAvcToSvcTranslator::OnMplAckCHANGELAYOUT , - ", GetMyFullName());

	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	if (m_bIsTranslatorError)
	{
		TRACEINTO << " This translator already reported on error, - " << GetMyFullName() << ", Encoder Connection ID = " << m_EncoderConnectionId;
		return;
	}


	switch (AckOpcode)
	{
		case VIDEO_ENCODER_CHANGE_LAYOUT_REQ:
		{
			OnMplChangeLayoutAck(status);
			break;
		}
		default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << " ACK_IND Ignored! - Ack Opcode: " << str.c_str() << "Encoder Connection ID = " << m_EncoderConnectionId << " - "  << GetMyFullName();
		}
	}// end switch

}


// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplOpenPortAck(STATUS status)
{
	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 14,1012)	// simulate Ack error from Open Encoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 15)			// simulate no Ack received	from Open Encoder

	// stop Open Encoder Timer
	DeleteTimer( OPEN_AVC_TO_SVC_ENCODER_TOUT );

	if (status != STATUS_OK)
	{
		// inform connect AvcToSvc Translator failed
		InformConnectEnded( statVideoInOutResourceProblem );
		TRACEINTO << " - AVC-Translator-Error - Ack on Open Encoder, - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
		DBGPASSERT(101);
		return;
	}

	TRACEINTO << " Open OK: (state changed to CHANGE_LAYOUT),  Encoder Connection ID = " << m_EncoderConnectionId << "  - ",GetMyFullName();
	m_state = CHANGE_LAYOUT;

	// send change layout 1x1 (VIDEO_ENCODER_CHANGE_LAYOUT_REQ)
	EStat changeLayoutStatus = SendChangeLayout_1x1();
	if (statOK != changeLayoutStatus)
	{
		InformConnectEnded( statVideoInOutResourceProblem );
		TRACEINTO << " AVC-Translator-Error - SendChangeLayout_1x1,  Encoder Connection ID = " << m_EncoderConnectionId << " - ",GetMyFullName();
		return;
	}

	// start ChangeLayout 1x1 Timer
	StartTimer(CHANGE_LAYOUT_1X1_TOUT ,CHANGE_LAYOUT_1X1_TOUT_VALUE);
	m_iChangeLayoutCounter = 0;
}


// ------------------------------------------------------------
EStat  CAvcToSvcTranslator::SendChangeLayout_1x1()
{
	if (!m_pHardwareInterfaceEncoder)
		{ DBGPASSERT(101); return statInconsistent; }
	if (!m_pBridgePartyVideoIn)
		{ DBGPASSERT(102); return statInconsistent; }

	//send change layout 1x1 to the AvcToSvc Encoder (always will be 1x1)

	// prepare layout parameters
	CLayout* pLayout = new CLayout( CP_LAYOUT_1X1, GetMyConfName() );
	bool bResult = PrepareLayoutParameters( pLayout );
	if (!bResult) {
		delete pLayout;
		TRACEINTO << " AVC-Translator-Error - bResult error" << ", EncoderConnectionID: " << m_EncoderConnectionId;
		return statInconsistent;
	}

	// prepare other parameters
	CVisualEffectsParams 			sVisualEffects;	// currently leaves the default
	CSiteNameInfo 					sSiteNameInfo;	// currently leaves the default
	AVC_TO_SVC_CHANGE_LAYOUT_ST 	cl;
	PrepareChangeLayoutRequest( &cl, &sVisualEffects, &sSiteNameInfo );

	// send change layout request (VIDEO_ENCODER_CHANGE_LAYOUT_REQ)
	TRACEINTO << " - Encoder Connection ID = " << m_EncoderConnectionId;


	//get if CONF is in telepresence mode
	BYTE isTelePresenceMode = 0;
	CVideoBridge* pBridge = (CVideoBridge*)m_pBridgePartyCntl->GetBridge();
	if (pBridge)
	{
		CConf* pConf = pBridge->GetConf();
		if (pConf)
		{
			const CCommConf* pCommConf = pConf->GetCommConf();
			if (pCommConf)
			{
				isTelePresenceMode = pCommConf->GetIsTelePresenceMode();
			}
		}
	}


	m_pHardwareInterfaceEncoder->ChangeLayoutSendOrUpdate(	pLayout,
													&sVisualEffects,
													&sSiteNameInfo,
													cl.speakerPlaceInLayout, cl.videoEncoderResolution, cl.decoderDetectedModeWidth,
													cl.decoderDetectedModeHeight, cl.decoderDetectedSampleAspectRatioWidth,
													cl.decoderDetectedSampleAspectRatioHeight, cl.videoAlg, cl.fs, cl.mbps,
													cl.videoConfType, cl.isSiteNamesEnabled, isTelePresenceMode, cl.bUseSharedMemForChangeLayoutReq, cl.isVSW );

	delete pLayout;
	m_lastReqId = 0;
	m_lastReq = VIDEO_ENCODER_CHANGE_LAYOUT_REQ;

	return statOK;
}

// ------------------------------------------------------------
bool  CAvcToSvcTranslator::PrepareLayoutParameters( CLayout* pLayout )
{
   CVidSubImage* pVidSubImage = (*pLayout)[0];
	if (!pVidSubImage)
		{ DBGPASSERT(101); return false; }

	if (m_pHardwareInterfaceEncoder)
		pVidSubImage->SetImageId( m_pHardwareInterfaceEncoder->GetPartyRsrcId() );
	else
		return false;

	return true;
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::PrepareChangeLayoutRequest( AVC_TO_SVC_CHANGE_LAYOUT_ST *cl, CVisualEffectsParams* sVisualEffects, CSiteNameInfo* sSiteNameInfo )
{
	// filling other params
	cl->speakerPlaceInLayout = 	 INVALID;
	cl->videoEncoderResolution = eVideoResolutionDummy;

	cl->decoderDetectedModeWidth = 					DEFAULT_DECODER_DETECTED_MODE_WIDTH;
	cl->decoderDetectedModeHeight =					DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
	cl->decoderDetectedSampleAspectRatioWidth =		DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH;
	cl->decoderDetectedSampleAspectRatioHeight =	DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT;

	cl->videoAlg = 				SVC;
	cl->fs = 					m_pHardwareInterfaceEncoder->GetFsForAvcToSvc( m_resWidth, m_resHight );
	cl->mbps = 					0;		// not in use for Avc to Svc
	cl->videoConfType = 		eVideoConfTypeCP;
	cl->isSiteNamesEnabled = 	NO;
	cl->bUseSharedMemForChangeLayoutReq = FALSE;
	cl->isVSW = 				NO;

	// filling Visual Effects Params:/
	CVisualEffectsParams*  pVisualEffectsParams = m_pBridgePartyVideoIn->GetVisualEffects();
	if (pVisualEffectsParams)
	{
		DWORD dwColor = 0xFF519077;		// old skin
		DWORD colorID = 0;				// old skin ID

		sVisualEffects->SetBackgroundColorYUV( dwColor );
		sVisualEffects->SetBackgroundImageID( colorID );
	}
	else
		{PASSERT(101);}

}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplChangeLayoutAck(STATUS status)
{
	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 16,1012)	// simulate Ack error from change layout
	ISDEBUGMODE_RETURN("TRANSLATOR", 17)			// simulate no Ack received	from change layout

	if (status != STATUS_OK)
	{
		TRACEINTO << " - AVC-Translator-Error - " << GetMyFullName() << ", EnConnectionID = " << m_EncoderConnectionId;
		return;		// return without stopping the timer means we waiting the timer to "jump" for trying again
	}

	// stop Change Layout Timer
	DeleteTimer( CHANGE_LAYOUT_1X1_TOUT );

	ISDEBUGMODE_SET_VAL("TRANSLATOR", 28, m_bResendChangeLayout, true);
	if (m_bResendChangeLayout)
	{
		TRACEINTO << " - Send again ChangeLayout - " << GetMyFullName() << ", EnConnectionID = " << m_EncoderConnectionId;
		m_bResendChangeLayout = false;
		m_iChangeLayoutCounter = 0;
		EStat changeLayoutStatus = SendChangeLayoutAgain();
		if (statOK == changeLayoutStatus)
			StartTimer(CHANGE_LAYOUT_1X1_TOUT ,CHANGE_LAYOUT_1X1_TOUT_VALUE);
		else
			InformConnectEnded( statVideoInOutResourceProblem );
		return;	// re-send the Change-Layout, not changing the current state
	}
	m_state = CONNECTED;
	TRACEINTO << " - (state changed to CONNECTED) , - " << GetMyFullName();

	// inform Bridge Party to keep with connection
	InformConnectEnded( statOK );
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::UpdateImageUponAVCtoSVCReady()
{
	// updating only this stream (encoder with a certain layer ID)
	// 2 encoders will update the image separately, each upon readiness.

	if (!m_pBridgePartyVideoIn)
		{PASSERT_AND_RETURN(101);}

	// get image
	CImage* pImage = (CImage*)m_pBridgePartyVideoIn->GetPartyImage();
	if (!pImage)
	{
		TRACEINTO << " - AVC-Translator-Error - pImage=NULL, - " << GetMyFullName() << ", LayerId: " << m_layerId << ", EncoderConnectionID: " << m_EncoderConnectionId;
		PASSERT_AND_RETURN(101);
	}

	TRACEINTO << " - Update Image, - " << GetMyFullName() << ", LayerId: " << m_layerId;

	CVideoRelayInMediaStream myRelayStream;
	myRelayStream.m_scpPipe.m_notificationType = eStreamCanNowBeProvided;

	myRelayStream.SetSsrc( m_ssrc );
	myRelayStream.SetLayerId( m_layerId );
	myRelayStream.SetResolutionHeight( m_resHight );
	myRelayStream.SetResolutionWidth(m_resWidth);
//	myRelayStream.SetIsAvailable(true);

	bool bIsVideoRelayImage = true;
	pImage->AddAndChangeRelayParams( m_channelHandle, &myRelayStream, bIsVideoRelayImage );
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplAckCONNECTED( CSegment* pParam )
{
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;

	*pParam >> AckOpcode >> ack_seq_num >> status;

	TRACEINTO << " Ack Opcode: " << AckOpcode << " - ack_seq_num: " << ack_seq_num << ", m_layerId: " << m_layerId << ", PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
	switch (AckOpcode)
	{
		case VIDEO_ENCODER_CHANGE_LAYOUT_REQ:
		{
			DeleteTimer( CHANGE_LAYOUT_1X1_TOUT );
			break;
		}
		default:
		{
			TRACEINTO << " unexpected Ack, PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
			break;
		}
	}// end switch
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplAckDISCONNECTING( CSegment* pParam )
{
	TRACEINTO << " - " << GetMyFullName() << ", m_layerId: " << m_layerId;

	OPCODE AckOpcode;
	DWORD ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
		case TB_MSG_CLOSE_PORT_REQ:
		{
			OnMplCloseEncoderAck(status);
			break;
		}
		default:
		{
			CProcessBase * process = CProcessBase::GetProcess();
			std::string str = process->GetOpcodeAsString(AckOpcode);
			TRACEINTO << " ACK_IND Ignored! - AVC-Translator-Error - Ack Opcode: " << str.c_str() << " - "  << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;
		}
	}// end switch

}


// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnMplCloseEncoderAck(STATUS status)
{
	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 18,1018)	// simulate Ack error from Close Encoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 19)			// simulate no Ack received	from Close Encoder

	// stop Open Encoder Timer
	DeleteTimer( CLOSE_ENCODER_TOUT );

	EStat eStatus = statOK;

	if (status != STATUS_OK)
	{
		TRACEINTO << " - AVC-Translator-Error - closing Encoder, status=" << (DWORD)status << ", EncoderConnectionID: " << m_EncoderConnectionId;
		eStatus = statVideoInOutResourceProblem;
	}

	InformDisconnectEnded( eStatus );
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnTimerOpenEncoderIDLE( CSegment* pParam )
{
	TRACEINTO << " Timer: unexpected state (IDLE) for this opcode, ignored! , PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}
void  CAvcToSvcTranslator::OnTimerOpenEncoderCHANGELAYOUT( CSegment* pParam )
{
	TRACEINTO << " Timer: unexpected state (CHANGELAYOUT) for this opcode, ignored! , PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}
void  CAvcToSvcTranslator::OnTimerOpenEncoderCONNECTED( CSegment* pParam )
{
	TRACEINTO << " Timer: unexpected state (CHANGELAYOUT) for this opcode, ignored! , PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}
void  CAvcToSvcTranslator::OnTimerOpenEncoderDISCONNECTING( CSegment* pParam )
{
	TRACEINTO << " PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}
// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnTimerOpenEncoderSETUP( CSegment* pParam )
{
	TRACEINTO << " - AVC-Translator-Error - inform on failure,  - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;

	// inform connect AvcToSvc Translator failed
	InformConnectEnded( statVideoInOutResourceProblem );

	DBGPASSERT(101);
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnTimerChangeLayoutIDLE( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnTimerChangeLayoutIDLE - (Timer: unexpected state (IDLE) for this opcode, ignored!!) , - ", GetMyFullName());
}
void  CAvcToSvcTranslator::OnTimerChangeLayoutSETUP( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnTimerChangeLayoutSETUP - (Timer: unexpected state (SETUP) for this opcode, ignored!!) , - ", GetMyFullName());
}
void  CAvcToSvcTranslator::OnTimerChangeLayoutCONNECTED( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnTimerChangeLayoutCONNECTED - (Timer: unexpected state (CONNECTED) for this opcode, ignored!!) , - ", GetMyFullName());
	m_iChangeLayoutCounter++;
	DBGPASSERT(m_iChangeLayoutCounter);

	if (m_iChangeLayoutCounter > 3)	//  try 7 times(16 seconds)
	{
		TRACEINTO << " - AVC-Translator-Error - not inform on failure,  - " << GetMyFullName() << ", ConnectionID = " << m_EncoderConnectionId;
	}
	else
	{
		SendChangeLayout_1x1();
		StartTimer(CHANGE_LAYOUT_1X1_TOUT ,CHANGE_LAYOUT_1X1_TOUT_VALUE);
		TRACEINTO << " - Timeout Error: try again,  - " << GetMyFullName() << ", ConnectionID = " << m_EncoderConnectionId;
	}
}
void  CAvcToSvcTranslator::OnTimerChangeLayoutDISCONNECTING( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnTimerChangeLayoutDISCONNECTING - (Timer: unexpected state (DISCONNECTING) for this opcode, ignored!!) , - ", GetMyFullName());
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnTimerChangeLayoutCHANGELAYOUT( CSegment* pParam )
{
	m_iChangeLayoutCounter++;
	DBGPASSERT(m_iChangeLayoutCounter);

	if ((m_iChangeLayoutCounter > 3) && (m_bResendChangeLayout == false))	//  try 7 times
	{
		// inform connect AvcToSvc Translator failed
		InformConnectEnded( statVideoInOutResourceProblem );
		TRACEINTO << " - AVC-Translator-Error - inform on failure,  - " << GetMyFullName() << ", ConnectionID = " << m_EncoderConnectionId;
	}
	else
	{
		// try again
		m_bResendChangeLayout  =false;
		SendChangeLayout_1x1();
		StartTimer(CHANGE_LAYOUT_1X1_TOUT ,CHANGE_LAYOUT_1X1_TOUT_VALUE);
		TRACEINTO << " - Timeout Error: try again,  - " << GetMyFullName() << ", ConnectionID = " << m_EncoderConnectionId;
	}
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnTimerCloseEncoderIDLE( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnTimerCloseEncoderIDLE - (Timer: unexpected state (IDLE) for this opcode, ignored!!) , - ", GetMyFullName());
}
void  CAvcToSvcTranslator::OnTimerCloseEncoderSETUP( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnTimerCloseEncoderSETUP - (Timer: unexpected state (SETUP) for this opcode, ignored!!) , - ", GetMyFullName());
}
void  CAvcToSvcTranslator::OnTimerCloseEncoderCHANGELAYOUT( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnTimerCloseEncoderCHANGELAYOUT - (Timer: unexpected state (CHANGELAYOUT) for this opcode, ignored!!) , - ", GetMyFullName());
}
void  CAvcToSvcTranslator::OnTimerCloseEncoderCONNECTED( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnTimerCloseEncoderCONNECTED - (Timer: unexpected state (CONNECTED) for this opcode, ignored!!) , - ", GetMyFullName());
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnTimerCloseEncoderDISCONNECTING( CSegment* pParam )
{
	// change state
	TRACEINTO << " AVC-Translator-Error - Timeout: (state changed to IDLE),  Encoder Connection ID = " << m_EncoderConnectionId << "  - ",GetMyFullName();
	m_state = IDLE;

	bool bDebugRetVal = false;
	ISDEBUGMODE_SET_VAL("TRANSLATOR", 29, bDebugRetVal, true)
	if (bDebugRetVal)
	{
		InformDisconnectEnded( statVideoInOutResourceProblem );
		return;
	}

	InformDisconnectEnded( statVideoInOutResourceProblem );
}


// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnTimerConnectEncoderSETUP( CSegment* pParam )
{
	TRACEINTO << " - AVC-Translator-Error - inform on failure,  - " << GetMyFullName() << ", EncoderConnectionID: " << m_EncoderConnectionId;

	// inform connect AvcToSvc Translator failed
	InformConnectEnded( statVideoInOutResourceProblem );

	DBGPASSERT(101);
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyFastUpdateCHANGELAYOUT( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnVideoBridgePartyFastUpdateCHANGELAYOUT  (probably error), - ", GetMyFullName() );
}
void  CAvcToSvcTranslator::OnVideoBridgePartyFastUpdateDISCONNECTING( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CAvcToSvcTranslator::OnVideoBridgePartyFastUpdateDISCONNECTING, - ", GetMyFullName() );
	// not an error but nothing to do
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::OnVideoBridgePartyFastUpdateCONNECTED( CSegment* pParam )
{
	TRACEINTO << "  PartyRsrcId: " << m_partyRsrcId << ", Encoder ConId: " << m_EncoderConnectionId;
}


// ------------------------------------------------------------
const char*  CAvcToSvcTranslator::GetMyFullName()
{
	if (m_pBridgePartyVideoIn)
	{
		CBridgePartyCntl* pBridgePartyCntlPtr = m_pBridgePartyVideoIn->GetBridgePartyCntlPtr();
		if (pBridgePartyCntlPtr)
			return pBridgePartyCntlPtr->GetFullName();
	}
	return "Error-Name ";	// return constant
}


// ------------------------------------------------------------
const char*  CAvcToSvcTranslator::GetMyConfName()
{
	if (m_pBridgePartyVideoIn)
	{
		CBridgePartyCntl* pBridgePartyCntlPtr = m_pBridgePartyVideoIn->GetBridgePartyCntlPtr();
		if (pBridgePartyCntlPtr)
			return pBridgePartyCntlPtr->GetConfName();
	}
	return "Error-Name ";	// return constant
}

// ------------------------------------------------------------
DWORD CAvcToSvcTranslator::GetSsrc()
{
	return m_ssrc;
}

// ------------------------------------------------------------
void CAvcToSvcTranslator::SendRelayIntraRequestToAvcToSvcTranslator()
{
	TRACEINTO << " - " << GetMyFullName() << ", m_tDelayIntraTime = " << m_tDelayIntraTime << " - LayerID = "  << m_layerId;

	StartTimer( FAST_UPDATE_TEST_TIMER, m_tDelayIntraTime );	// wait 200 ms before first intra request from SVC-Encoder
}


// ------------------------------------------------------------
void CAvcToSvcTranslator::OnTimerSendRelayIntraRequestToAvcToSvcTranslator( CSegment* pParam )
{
	TRACEINTO << " - LayerID = " << m_layerId;

	if (m_pHardwareInterfaceEncoder)
		m_pHardwareInterfaceEncoder->SendFastUpdate();
}

// ------------------------------------------------------------
void CAvcToSvcTranslator::OnEncoderResolutionChangedCHANGELAYOUT( CSegment* pParam )
{
	TRACEINTO << " - PartyRsrcId: " << m_partyRsrcId << ", LayerID = " << m_layerId;
	m_bResendChangeLayout = true;	// will cause to re-send Change Layout upon Ack or Timer
}
void CAvcToSvcTranslator::OnEncoderResolutionChangedCONNECTED( CSegment* pParam )
{
	TRACEINTO << " - PartyRsrcId: " << m_partyRsrcId << ",  LayerID = " << m_layerId;
	EStat changeLayoutStatus = SendChangeLayoutAgain();
	m_iChangeLayoutCounter = 0;
	m_bResendChangeLayout = false;
	if (statOK == changeLayoutStatus)
		StartTimer(CHANGE_LAYOUT_1X1_TOUT ,CHANGE_LAYOUT_1X1_TOUT_VALUE);
}
EStat CAvcToSvcTranslator::SendChangeLayoutAgain()
{
	EStat changeLayoutStatus = SendChangeLayout_1x1();
	if (statOK != changeLayoutStatus)
	{
		TRACEINTO << " AVC-Translator-Error - SendChangeLayout_1x1,  Encoder Connection ID = " << m_EncoderConnectionId << " - ",GetMyFullName();
		return changeLayoutStatus;
	}
	else
	{
		TRACEINTO << " Send again was succeeded";
	}
	return statOK;
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::InformConnectEnded( EStat status )
{
	if (statOK != status) {
		m_bIsTranslatorError = true;	// sign that this translator have an error
		UpdateLastReqUponError();
	}

	if (m_pBridgePartyVideoIn)
		m_pBridgePartyVideoIn->AvcToSvcTranslatorConnected( status );
	else
		{PASSERT_AND_RETURN(1);}
}

// ------------------------------------------------------------
void  CAvcToSvcTranslator::InformDisconnectEnded( EStat status )
{
	m_state = IDLE;
	TRACEINTO << " - (state changed to IDLE) , - " << GetMyFullName();

	if (statOK != status)
		UpdateLastReqUponError();

	if (m_pBridgePartyVideoIn)
		m_pBridgePartyVideoIn->AvcToSvcTranslatorDisconnected( status, m_EncoderConnectionId, m_bIsVswStream );
	else
		{PASSERT(1);}

	m_bIsReadyToBeKilled = true;
}

void CAvcToSvcTranslator::UpdateLastReqUponError()
{
	if (m_pBridgePartyVideoIn)
		m_pBridgePartyVideoIn->UpdateLastReqUponTranslatorError( GetLastReqId(), GetLastReq() );
}

void CAvcToSvcTranslator::Dump() const
{
	std::ostringstream str;
	str << "CAvcToSvcTranslator::Dump :";
	str << "\n m_pBridgePartyVideoIn       = " << (DWORD)m_pBridgePartyVideoIn;
	str << "\n m_pHardwareInterfaceEncoder = " << (DWORD)m_pHardwareInterfaceEncoder;
	str << "\n m_layerId		           = " << m_layerId;
	str << "\n m_ssrc	                   = " << (DWORD)m_ssrc;
	str << "\n m_channelHandle	           = " << (DWORD)m_channelHandle;
	str << "\n m_EncoderConnectionId	   = " << (DWORD)m_EncoderConnectionId;
	str << "\n ------------------------- --- ";
	PTRACE(eLevelInfoNormal,str.str().c_str());
}

// ########################################################################################################################
// ########################################################################################################################
//
//											CAvcToSvcTranslatorVsw
//
// ########################################################################################################################
// ########################################################################################################################


// new translator, VSW (no Encoder, VSW path)

PBEGIN_MESSAGE_MAP(CAvcToSvcTranslatorVsw)

ONEVENT(CONNECT_VIDEO_OUT                 ,IDLE           ,CAvcToSvcTranslatorVsw::OnVideoBridgePartyConnectIDLE)
ONEVENT(CONNECT_VIDEO_OUT                 ,CONNECTED      ,CAvcToSvcTranslatorVsw::OnVideoBridgePartyConnectCONNECTED)

ONEVENT(DISCONNECT_VIDEO_OUT              ,IDLE           ,CAvcToSvcTranslatorVsw::OnVideoBridgePartyDisConnectIDLE)
ONEVENT(DISCONNECT_VIDEO_OUT             ,CONNECTED      ,CAvcToSvcTranslatorVsw::OnVideoBridgePartyDisConnectCONNECTED)

PEND_MESSAGE_MAP(CAvcToSvcTranslatorVsw, CAvcToSvcTranslator);

// General flow:
// 1. connect in IDLE goes to CONNECTED
// 2. disconnect in CONNECTED goes to IDLE

CAvcToSvcTranslatorVsw::CAvcToSvcTranslatorVsw ()
{
	m_bIsVswStream = true;
	m_EncoderConnectionId = 0xFFFFFFFF;
}

CAvcToSvcTranslatorVsw::~CAvcToSvcTranslatorVsw ()
{
}

// ------------------------------------------------------------
void CAvcToSvcTranslatorVsw::RequestChangeLayout()
{
	TRACEINTO << " nothing to do in VSW Stream";
}
// ------------------------------------------------------------
void  CAvcToSvcTranslatorVsw::OnVideoBridgePartyConnectIDLE( CSegment* pParam )
{
	TRACEINTO << " (state changed to SETUP), VSW Stream, name: " << GetMyFullName() << ", m_layerId = " << m_layerId << ", PartyRsrcId = " << (DWORD)m_partyRsrcId;
	m_state = CONNECTED;
	InformConnectEnded( statOK );
}

// ------------------------------------------------------------
void  CAvcToSvcTranslatorVsw::OnVideoBridgePartyConnectCONNECTED( CSegment* pParam )
{
	TRACEINTO << " (unexpected state for Connect) , name: " << GetMyFullName() << ", PartyRsrcId = " << (DWORD)m_partyRsrcId;
	InformConnectEnded( statOK );
}

// ------------------------------------------------------------
void  CAvcToSvcTranslatorVsw::OnVideoBridgePartyDisConnectIDLE( CSegment* pParam )
{
	TRACEINTO << " (unexpected state for Disconnect, ignore) , name: " << GetMyFullName() << ", PartyRsrcId = " << (DWORD)m_partyRsrcId;
}

// ------------------------------------------------------------
void  CAvcToSvcTranslatorVsw::OnVideoBridgePartyDisConnectCONNECTED( CSegment* pParam )
{
	TRACEINTO << " - Closing Encoder, status = OK";
	InformDisconnectEnded( statOK );
}

// ------------------------------------------------------------
void CAvcToSvcTranslatorVsw::SendRelayIntraRequestToAvcToSvcTranslator()
{
	TRACEINTO << " - LayerID = " << m_layerId <<  ", PartyRsrcId = " << (DWORD)m_partyRsrcId;

	if (m_pBridgePartyVideoIn)
		m_pBridgePartyVideoIn->AvcToSvcVswRequestIntraFromEP(); // requests from EP, not from Encoder
	else
		{PASSERT_AND_RETURN(1);}
}


