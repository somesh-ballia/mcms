#include "BridgePartyVideoRelayOut.h"
#include "VideoHardwareInterface.h"
#include "MrcStructs.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "RelayMediaStream.h"
#include "H264Util.h"



PBEGIN_MESSAGE_MAP(CBridgePartyVideoRelayOut)

	ONEVENT(CONNECT_VIDEO_OUT,			IDLE,               CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectIDLE)
	ONEVENT(CONNECT_VIDEO_OUT,			SLIDE,     			CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectSLIDE)
	ONEVENT(CONNECT_VIDEO_OUT,			STOP_SLIDE,     	CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectSTOPSLIDE)
	ONEVENT(CONNECT_VIDEO_OUT,			SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectSETUP)
	ONEVENT(CONNECT_VIDEO_OUT,			CONNECTED,          CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectCONNECTED)
	ONEVENT(CONNECT_VIDEO_OUT,			CHANGE_STREAMS,     CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectCHANGESTREAMS)


	ONEVENT(DISCONNECT_VIDEO_OUT, 		IDLE,               CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectIDLE)
	ONEVENT(DISCONNECT_VIDEO_OUT,		SLIDE, 				CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectSLIDE)
	ONEVENT(DISCONNECT_VIDEO_OUT, 		STOP_SLIDE,         CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectSLIDE)
	ONEVENT(DISCONNECT_VIDEO_OUT,		SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectSETUP)
	ONEVENT(DISCONNECT_VIDEO_OUT,		CONNECTED,          CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectCONNECTED)
	ONEVENT(DISCONNECT_VIDEO_OUT,		CHANGE_STREAMS, 	CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectCHANGESTREAMS)


	ONEVENT(UPDATE_VIDEO_OUT_PARAMS, 	IDLE,               CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsIDLE)
	ONEVENT(UPDATE_VIDEO_OUT_PARAMS, 	SLIDE,              CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSLIDE)
	ONEVENT(UPDATE_VIDEO_OUT_PARAMS, 	STOP_SLIDE,         CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSTOPSLIDE)
	ONEVENT(UPDATE_VIDEO_OUT_PARAMS, 	SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSETUP)
	ONEVENT(UPDATE_VIDEO_OUT_PARAMS, 	CONNECTED,          CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsCONNECTED)
	ONEVENT(UPDATE_VIDEO_OUT_PARAMS, 	CHANGE_STREAMS,     CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsCHANGESTREAMS)


	 ONEVENT(ACK_IND,                   IDLE,               CBridgePartyVideoRelayOut::OnMplAckIDLE)
	ONEVENT(ACK_IND,                    SLIDE,              CBridgePartyVideoRelayOut::OnMplAckSLIDE)
	 ONEVENT(ACK_IND,                   SETUP,              CBridgePartyVideoRelayOut::OnMplAckSETUP)
	 ONEVENT(ACK_IND,                   CONNECTED,          CBridgePartyVideoRelayOut::OnMplAckCONNECTED)
	 ONEVENT(ACK_IND,                   CHANGE_STREAMS,     CBridgePartyVideoRelayOut::OnMplAckCHANGESTREAMS)


	ONEVENT(ADDIMAGE, 					IDLE,               CBridgePartyVideoRelayOut::NullActionFunction)
	ONEVENT(ADDIMAGE,					SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageSLIDE)
	ONEVENT(ADDIMAGE,					STOP_SLIDE,			CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageSTOPSLIDE)
	ONEVENT(ADDIMAGE,					SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageSETUP)
	ONEVENT(ADDIMAGE,					CONNECTED,			CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageCONNECTED)
	ONEVENT(ADDIMAGE,					CHANGE_STREAMS,		CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageCHANGESTREAMS)


	ONEVENT(DELIMAGE, 					IDLE,               CBridgePartyVideoRelayOut::NullActionFunction)
	ONEVENT(DELIMAGE,					SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageSLIDE)
	ONEVENT(DELIMAGE,					SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageSTOPSLIDE)
	ONEVENT(DELIMAGE,					SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageSETUP)
	ONEVENT(DELIMAGE,					CONNECTED,			CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageCONNECTED)
	ONEVENT(DELIMAGE,					CHANGE_STREAMS,		CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageCHANGESTREAMS)


	ONEVENT(MUTEIMAGE, 					IDLE,               CBridgePartyVideoRelayOut::NullActionFunction)
	ONEVENT(MUTEIMAGE,					SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageSLIDE)
	ONEVENT(MUTEIMAGE,					STOP_SLIDE,			CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageSTOPSLIDE)
	ONEVENT(MUTEIMAGE,					SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageSETUP)
	ONEVENT(MUTEIMAGE,					CONNECTED,			CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageCONNECTED)
	ONEVENT(MUTEIMAGE,					CHANGE_STREAMS,		CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageCHANGESTREAMS)


	ONEVENT(UNMUTEIMAGE, 				IDLE,               CBridgePartyVideoRelayOut::NullActionFunction)
	ONEVENT(UNMUTEIMAGE,				SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageSLIDE)
	ONEVENT(UNMUTEIMAGE,				STOP_SLIDE,			CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageSTOPSLIDE)
	ONEVENT(UNMUTEIMAGE,				SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageSETUP)
	ONEVENT(UNMUTEIMAGE,				CONNECTED,			CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageCONNECTED)
	ONEVENT(UNMUTEIMAGE,				CHANGE_STREAMS,		CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageCHANGESTREAMS)


	ONEVENT(SPEAKERS_CHANGED, 			IDLE,               CBridgePartyVideoRelayOut::NullActionFunction)
	ONEVENT(SPEAKERS_CHANGED,			SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersSLIDE)
	ONEVENT(SPEAKERS_CHANGED,			STOP_SLIDE,			CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersSTOPSLIDE)
	ONEVENT(SPEAKERS_CHANGED,			SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersSETUP)
	ONEVENT(SPEAKERS_CHANGED,			CONNECTED,			CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersCONNECTED)
	ONEVENT(SPEAKERS_CHANGED,			CHANGE_STREAMS,		CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersCHANGESTREAMS)


	ONEVENT(AUDIO_SPEAKER_CHANGED, 		IDLE,               CBridgePartyVideoRelayOut::NullActionFunction)
	ONEVENT(AUDIO_SPEAKER_CHANGED,		SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerSLIDE)
	ONEVENT(AUDIO_SPEAKER_CHANGED,		STOP_SLIDE,			CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerSTOPSLIDE)
	ONEVENT(AUDIO_SPEAKER_CHANGED,		SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerSETUP)
	ONEVENT(AUDIO_SPEAKER_CHANGED,		CONNECTED,			CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerCONNECTED)
	ONEVENT(AUDIO_SPEAKER_CHANGED,		CHANGE_STREAMS,		CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerCHANGESTREAMS)


	ONEVENT(UPDATEIMAGE, 				IDLE,               CBridgePartyVideoRelayOut::NullActionFunction)
	ONEVENT(UPDATEIMAGE,				SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageSLIDE)
	ONEVENT(UPDATEIMAGE,				STOP_SLIDE,			CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageSTOPSLIDE)
	ONEVENT(UPDATEIMAGE,				SETUP,              CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageSETUP)
	ONEVENT(UPDATEIMAGE,				CONNECTED,			CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageCONNECTED)
	ONEVENT(UPDATEIMAGE,				CHANGE_STREAMS,		CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageCHANGESTREAMS)


	ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA, IDLE,             CBridgePartyVideoRelayOut::OnEpAskForIntraIDLE)
	ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA, SLIDE,            CBridgePartyVideoRelayOut::OnVideoBridgePartyFastUpdateSLIDE)
	ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA, STOP_SLIDE,       CBridgePartyVideoRelayOut::OnEpAskForIntraSTOP_SLIDE)
	ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA, SETUP,            CBridgePartyVideoRelayOut::OnEpAskForIntraSETUP)
	ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA, CONNECTED,        CBridgePartyVideoRelayOut::OnEpAskForIntraCONNECTED)
	ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA, CHANGE_STREAMS,   CBridgePartyVideoRelayOut::OnEpAskForIntraCHANGE_STREAMS)

	ONEVENT(VIDEO_OUT_CHANGESTREAMS_TIMEOUT, CHANGE_STREAMS,CBridgePartyVideoRelayOut::OnVideoBridgePartyToutCHANGESTREAMS)

	ONEVENT(UPDATEONIMAGEAVCTOSVC,		IDLE,				CBridgePartyVideoRelayOut::NullActionFunction)
	ONEVENT(UPDATEONIMAGEAVCTOSVC,		SLIDE,				CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcSLIDE)
	ONEVENT(UPDATEONIMAGEAVCTOSVC,		SETUP,				CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcSETUP)
	ONEVENT(UPDATEONIMAGEAVCTOSVC,		CONNECTED,			CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcCONNECTED)
	ONEVENT(UPDATEONIMAGEAVCTOSVC,		CHANGE_STREAMS,		CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcCHANGESTREAMS)


	ONEVENT(IVR_SHOW_SLIDE_REQ                ,IDLE           ,CBridgePartyVideoRelayOut::OnVideoBridgePartyShowSlideIDLE)
	ONEVENT(IVR_SHOW_SLIDE_REQ                ,SLIDE          ,CBridgePartyVideoRelayOut::OnVideoBridgePartyShowSlideSLIDE)

	ONEVENT(IVR_STOP_SHOW_SLIDE_REQ           ,IDLE           ,CBridgePartyVideoRelayOut::OnVideoBridgePartyStopShowSlideIDLE)
	ONEVENT(IVR_STOP_SHOW_SLIDE_REQ           ,SLIDE          ,CBridgePartyVideoRelayOut::OnVideoBridgePartyStopShowSlideSLIDE)

	ONEVENT(ACK_ON_IVR_SCP_SHOW_SLIDE,       SLIDE,         CBridgePartyVideoRelayOut::OnPartyCntlAckOnIvrScpShowSlideSLIDE)
	ONEVENT(ACK_ON_IVR_SCP_STOP_SHOW_SLIDE,  STOP_SLIDE,    CBridgePartyVideoRelayOut::OnPartyCntlAckOnIvrScpStopShowSlideSTOPSLIDE)
	ONEVENT(ACK_ON_IVR_SCP_STOP_SHOW_SLIDE,  ANYCASE,    	CBridgePartyVideoRelayOut::NullActionFunction)

	ONEVENT(STOP_SHOW_SLIDE_TIMEOUT,         STOP_SLIDE,    CBridgePartyVideoRelayOut::OnTimerStopShowSlideSTOPSLIDE)


PEND_MESSAGE_MAP(CBridgePartyVideoRelayOut,CBridgePartyMediaUniDirection);

/////////////////////////////////////////////////////
CBridgePartyVideoRelayOut::CBridgePartyVideoRelayOut():CBridgePartyMediaUniDirection()
{
	m_isChangeStreamWhileWaitingForAck = false;
	m_pIVRPlayMessage = new CIVRPlayMessage;
	m_pVideoOutStreamsHandler = NULL;
}

/////////////////////////////////////////////////////
CBridgePartyVideoRelayOut::CBridgePartyVideoRelayOut(const CBridgePartyVideoRelayOut& rOtherBridgePartyVideRelayoOut):CBridgePartyMediaUniDirection(rOtherBridgePartyVideRelayoOut)
{
	m_isChangeStreamWhileWaitingForAck = rOtherBridgePartyVideRelayoOut.m_isChangeStreamWhileWaitingForAck;
	m_pVideoOutStreamsHandler = NULL;
	m_pIVRPlayMessage = new CIVRPlayMessage;
}

/////////////////////////////////////////////////////
CBridgePartyVideoRelayOut::~CBridgePartyVideoRelayOut()
{
    POBJDELETE(m_pVideoOutStreamsHandler);
    POBJDELETE(m_pIVRPlayMessage);
}

///////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::Create (const CBridgePartyCntl*	pBridgePartyCntl)
{

}

///////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::Create(const CBridgePartyCntl* pBridgePartyCntl,
                                       const CRsrcParams* pRsrcParams,
                                       const CBridgePartyMediaParams* pBridgePartyMediaParams)
{
	m_pHardwareInterface = new CVideoHardwareInterface;//KEREN?
	CBridgePartyMediaUniDirection::Create(pBridgePartyCntl, pRsrcParams);

	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayOut::Create : Name - ",m_pBridgePartyCntl->GetFullName());
//TODO
	SaveUpdatedVideoParams((CBridgePartyVideoRelayMediaParams*)pBridgePartyMediaParams);

	const CVideoBridgeCP* pVideoBridge = (CVideoBridgeCP*)pBridgePartyCntl->GetBridge();
    m_pVideoOutStreamsHandler = new CVideoRelayOutStreamsHandler(pVideoBridge,((CVideoRelayBridgePartyCntl*)pBridgePartyCntl));
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::Connect()
{
	DispatchEvent(CONNECT_VIDEO_OUT, NULL);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::DisConnect()
{
	DispatchEvent(DISCONNECT_VIDEO_OUT, NULL);

}
/////////////////////////////////////////////////////
//This function is called in the flow connect while disconnecting
void CBridgePartyVideoRelayOut::UpdatePartyOutParams(CUpdatePartyVideoRelayInitParams* updatePartyVideoRelayInitParams)
{
    SaveUpdatedVideoParams((CBridgePartyVideoRelayMediaParams*) updatePartyVideoRelayInitParams->GetMediaOutParams());
	//TODO
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::AddImage()
{
	CSegment* pSeg = new CSegment;
	DispatchEvent(ADDIMAGE,pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::DelImage()
{
	CSegment* pSeg = new CSegment;
	DispatchEvent(DELIMAGE,pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::MuteImage()
{
	CSegment* pSeg = new CSegment;
	DispatchEvent(MUTEIMAGE,pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::UnMuteImage()
{
	CSegment* pSeg = new CSegment;
	DispatchEvent(UNMUTEIMAGE,pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::ChangeSpeakers()
{
	CSegment* pSeg = new CSegment;
	DispatchEvent(SPEAKERS_CHANGED,pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::ChangeAudioSpeaker()
{
	CSegment* pSeg = new CSegment;
	DispatchEvent(AUDIO_SPEAKER_CHANGED,pSeg);
	POBJDELETE(pSeg);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::UpdateImage()
{
	CSegment* pSeg = new CSegment;
	DispatchEvent(UPDATEIMAGE,pSeg);
	POBJDELETE(pSeg);
}


/////////////////////////////////////////////////////
//This function is called in the flow of UpdatePartyVideoIn
void CBridgePartyVideoRelayOut::UpdateVideoParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams)
{
	SaveUpdatedVideoParams(pBridgePartyVideoRelayParams);
	CSegment* pSeg = new CSegment;
 	DispatchEvent(UPDATE_VIDEO_OUT_PARAMS,pSeg);
 	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
//StateMachine
void  CBridgePartyVideoRelayOut::OnVideoBridgePartyConnect(CSegment* pParam)
{
	m_state = CONNECTED;
	PTRACE(eLevelError, "CBridgePartyVideoRelayOur::OnVideoBridgePartyConnect");
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)statOK;
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);

	// Eitan - for move
	bool bImmediately = true;//in case MRE connected we will need to send intra because its not related to the other EPs flow
	StartChangeStreams(bImmediately);
}

/////////////////////////////////////////////////////

void CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectIDLE(CSegment* pParam)
{
	if(!m_isReady)
	{
		m_state = SETUP;
		PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectIDLE set party to Setup");
	}
	else
	{
		OnVideoBridgePartyConnect(pParam);
	}
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////

void  CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectSETUP(CSegment* pParam)
{
	if(!m_isReady)
	{
		PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectSETUP according to the flow we were supposed to be in Idle");

	}
	else
	{
		OnVideoBridgePartyConnect(pParam);
	}

}
/////////////////////////////////////////////////////

void  CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectCONNECTED ignore already connected");

}

/////////////////////////////////////////////////////
void  CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectCHANGESTREAMS ignore already connected");

}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::CBridgePartyVideoRelayOut::OnVideoBridgePartyConnectSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();
	SetIsConnectDuringStopSlideState(true);
}

///////////////////////////////////////////////////////

void CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnect(CSegment* pParam)
{
	m_state = IDLE;
	BYTE	responseStatus = statOK;
	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)responseStatus;
	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_DISCONNECTED);
	POBJDELETE(pSeg);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectIDLE(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectIDLE ignore already disconnected");
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectSETUP ");
	OnVideoBridgePartyDisconnect(pParam);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectCONNECTED ");
	OnVideoBridgePartyDisconnect(pParam);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectCHANGESTREAMS ");
	OnVideoBridgePartyDisconnect(pParam);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDisconnectSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << ", state=" << m_state;
	if(m_videoOutParamsStore.m_bIsShowSlideToHardwareSent)
	{
		((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopShowSlide(pParam);
		m_videoOutParamsStore.InitAllIvrSlideParams();
	}
	OnVideoBridgePartyDisconnect(pParam);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsIDLE ", m_pBridgePartyCntl->GetFullName());

	EStat responseStatus = statOK;
	BOOL IsAckParams = FALSE;
	CSegment *pMsg = new CSegment;
	*pMsg << (BYTE)responseStatus;
	*pMsg << (BYTE) IsAckParams;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
	POBJDELETE(pMsg);


}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSLIDE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSLIDE ", m_pBridgePartyCntl->GetFullName());

	if( !m_videoOutParamsStore.m_bIsShowSlideToHardwareSent )
	{
		if( CanSlideBeSent() )
		{
			SendShowSlide();
		}
	}
	else
	{
		int layerIdForIVRSlide = GetLayerIdForUpdateIvrSlide();
		if( layerIdForIVRSlide >= 0 )
		{
			SendUpdateSlideToHardware(layerIdForIVRSlide);
		}
		else
		{
			TRACEINTO << " layerIdForIVRSlide = " << layerIdForIVRSlide;
		}

		if( !IsOneStreamWithIVRPipeIdInStraemsList() )
		{
			//in case number of streams > 1 and slide was sent then send notify to all non IVRpipe ID streams.
			SendNotificationIfNeededInSlideStage();
			TRACEINTO << " IsOneStreamWithIVRPipeIdInStraemsList() return false!!!" ;
		}
	}


}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	if( CanSlideStopStateBeFinished() )
	{
		DeleteTimer(STOP_SHOW_SLIDE_TIMEOUT);
		if( m_videoOutParamsStore.m_bIsConnectDuringStopSlideState )
		{
			OnVideoBridgePartyConnect(pParam);
		}
		else
		{
			m_state = IDLE;
		}
	}
	else
	{
		TRACEINTO << "m_bIsAckOnIvrScpStopSlideReceived = " << (WORD)m_videoOutParamsStore.m_bIsAckOnIvrScpStopSlideReceived;
	}
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSETUP",m_pBridgePartyCntl->GetFullName() );

	if(!m_isReady)
	{
		PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsSETUP not ready, do nothing");

	}
	else
	{
		if(m_videoOutParamsStore.m_bIsSCPShowSlideReqCanBeSent)
		{
			SendScpIvrShowSlideReqToPartyCntl();
		}
		else
		{
		// change state to connected
		m_state = CONNECTED;
		// inform video bridge party control that video out is connected
		CSegment* pSeg = new CSegment;
		*pSeg << (BYTE)statOK;
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_OUT_CONNECTED);
		POBJDELETE(pSeg);
	    // send change streams request
	    bool bImmediately = true;//in case MRE connected we will need to send intra because its not related to the other EPs flow
	    StartChangeStreams(bImmediately);
		}
	}

	EStat responseStatus = statOK;
	BOOL IsAckParams = FALSE;
	CSegment *pMsg = new CSegment;
	*pMsg << (BYTE)responseStatus;
    *pMsg << (BYTE) IsAckParams;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
	POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam)
{
	TRACEINTO << " name: " << m_pBridgePartyCntl->GetFullName();
	bool bImmediately = true;//in case MRE connected we will need to send intra because its not related to the other EPs flow
	StartChangeStreams(bImmediately);

	EStat responseStatus = statOK;
	BOOL IsAckParams = FALSE;
	CSegment *pMsg = new CSegment;
	*pMsg << (BYTE)responseStatus;
	*pMsg << (BYTE) IsAckParams;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
	POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsCHANGESTREAMS(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateVideoParamsCHANGESTREAMS",m_pBridgePartyCntl->GetFullName() );
	m_isChangeStreamWhileWaitingForAck = true;

	EStat responseStatus = statOK;
	BOOL IsAckParams = FALSE;
	CSegment *pMsg = new CSegment;
	*pMsg << (BYTE)responseStatus;
    *pMsg << (BYTE) IsAckParams;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_OUT_UPDATED);
	POBJDELETE(pMsg);
}


/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnMplAckIDLE(CSegment* pParam)
{
	STATUS status;
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	*pParam >> AckOpcode >> ack_seq_num >> status;
//	CMedString cstr;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();
//	cstr << "CBridgePartyVideoRelayOut::OnMplAckIDLE not supposed to receive ACK , opcode: " << AckOpcode << " name: "<< m_pBridgePartyCntl->GetFullName();
//	PTRACE(eLevelInfoNormal,cstr.GetString());

	switch (AckOpcode)
	{
		case IVR_STOP_SHOW_SLIDE_REQ: // CAM sends StopShowSlide we change the state from Slide to Idle
		{
			CSegment* pSeg = new CSegment;
			*pSeg << AckOpcode << ack_seq_num << status << *pParam;

			((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
			POBJDELETE(pSeg);
			break;
		}
		default:
		{
			TRACEINTO << "ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}

}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnMplAckSLIDE(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	switch (AckOpcode)
	{
		case IVR_SHOW_SLIDE_REQ:
		{
			CSegment* pSeg = new CSegment;
			*pSeg << AckOpcode << ack_seq_num << status << *pParam;

			((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
			POBJDELETE(pSeg);
			break;
		}
		default:
		{
			TRACEINTO <<"ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnMplAckSETUP(CSegment* pParam)
{
	STATUS status;
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	switch (AckOpcode)
	{
		case IVR_SHOW_SLIDE_REQ:
		case IVR_STOP_SHOW_SLIDE_REQ:
		{
			CSegment* pSeg = new CSegment;
			*pSeg << AckOpcode << ack_seq_num << status << *pParam;

			((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
			POBJDELETE(pSeg);
			break;
		}
		default:
		{
			TRACEINTO << "ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}
//	CMedString cstr;
//	cstr << "CBridgePartyVideoRelayOut::OnMplAckSETUP not supposed to receive ACK , opcode: " << AckOpcode << " name: "<< m_pBridgePartyCntl->GetFullName();
//	PTRACE(eLevelInfoNormal,cstr.GetString());

}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnMplAckCONNECTED(CSegment* pParam)
{
	STATUS status;
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	switch (AckOpcode)
	{
		case IVR_SHOW_SLIDE_REQ:
		case IVR_STOP_SHOW_SLIDE_REQ:
		{
			CSegment* pSeg = new CSegment;
			*pSeg << AckOpcode << ack_seq_num << status << *pParam;

			((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
			POBJDELETE(pSeg);
			break;
		}
		default:
		{
			TRACEINTO << "ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}
//	CMedString cstr;
//	cstr << "CBridgePartyVideoRelayOut::OnMplAckCONNECTED not supposed to receive ACK , opcode: " << AckOpcode << " name: "<< m_pBridgePartyCntl->GetFullName();
//	PTRACE(eLevelInfoNormal,cstr.GetString());
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnMplAckCHANGESTREAMS(CSegment* pParam)
{
	STATUS status;
	OPCODE AckOpcode;
	DWORD ack_seq_num;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	switch (AckOpcode)
	{
		case IVR_SHOW_SLIDE_REQ:
		case IVR_STOP_SHOW_SLIDE_REQ:
		{
			CSegment* pSeg = new CSegment;
			*pSeg << AckOpcode << ack_seq_num << status << *pParam;

			((CVideoBridgePartyCntl*) m_pBridgePartyCntl)->IvrNotification(ACK_IND, pSeg);
			POBJDELETE(pSeg);
			break;
		}
		case CONF_PARTY_MRMP_VIDEO_SOURCES_REQ:
		{
			OnMplVideoSourcesAck(status);
			break;
		}
		default:
		{
			TRACEINTO << "ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
			break;
		}
	}

//	if( AckOpcode == CONF_PARTY_MRMP_VIDEO_SOURCES_REQ)
//	{
//		OnMplVideoSourcesAck(status);
//	}
//	else
//	{
//		CMedString cstr;
//		cstr << "CBridgePartyVideoRelayOut::OnMplAckCHANGESTREAMS not supposed to receive ACK for opcode: " << AckOpcode << " name: "<< m_pBridgePartyCntl->GetFullName();
//		PTRACE(eLevelInfoNormal,cstr.GetString());
//	}
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnMplVideoSourcesAck(STATUS status)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnMplVideoSourcesAck");

	DeleteTimer(VIDEO_OUT_CHANGESTREAMS_TIMEOUT);
	if (status != STATUS_OK)
	{
			DBGPASSERT(status);
	}

	m_state = CONNECTED;

	StartChangeStreamsIsChangeStreamWhileWaitingForAck();
}
/////////////////////////////////////////////////////

void CBridgePartyVideoRelayOut::StartChangeStreamsIsChangeStreamWhileWaitingForAck()
{
	if(m_isChangeStreamWhileWaitingForAck)
	{
		m_isChangeStreamWhileWaitingForAck = false;
		PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnMplVideoSourcesAck Start change streams process");
		bool bImmediately = true;//in case MRE connected we will need to send intra because its not related to the other EPs flow
		StartChangeStreams(bImmediately);
	}

}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageSETUP ignore ");
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageCONNECTED");
	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
	StartChangeStreams(bImmediately);

}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageCHANGESTREAMS");
	m_isChangeStreamWhileWaitingForAck = true;
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyAddImageSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageSETUP ignore ");
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageCONNECTED");
	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
	StartChangeStreams(bImmediately);

}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
//	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageSLIDE");
//	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
//	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyDelImageCHANGESTREAMS");
	m_isChangeStreamWhileWaitingForAck = true;
}


/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageSETUP ignore ");
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageCONNECTED");
	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
	StartChangeStreams(bImmediately);

}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
//	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageSLIDE");
//	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
//	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyMuteImageCHANGESTREAMS");
	m_isChangeStreamWhileWaitingForAck = true;
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageSETUP ignore ");
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageCONNECTED");
	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
	StartChangeStreams(bImmediately);

}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
//	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageSLIDE");
//	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
//	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUnMuteImageCHANGESTREAMS");
	m_isChangeStreamWhileWaitingForAck = true;
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersSETUP ignore ");
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersCONNECTED");
	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
//	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersSLIDE");
//	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
//	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeSpeakersCHANGESTREAMS");
	m_isChangeStreamWhileWaitingForAck = true;
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerSETUP ignore ");
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerCONNECTED");
	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
//	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerSLIDE");
//	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
//	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyChangeAudioSpeakerCHANGESTREAMS");
	m_isChangeStreamWhileWaitingForAck = true;
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageSETUP ignore ");
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageCONNECTED");
	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
//	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageSLIDE");
//	bool bImmediately = false;//In case new image was added to vector of seen image will ask intra after all the BridgePartyOut handled the event
//	StartChangeStreams(bImmediately);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " Event is ignored !!!";
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageCHANGESTREAMS(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateImageCHANGESTREAMS");
	m_isChangeStreamWhileWaitingForAck = true;
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SaveUpdatedVideoParams(const CBridgePartyVideoRelayMediaParams* bridgePartyVideoParams)
{
    //DEBUG(bridgePartyVideoParams);
    if(bridgePartyVideoParams)
    {
        m_isReady = bridgePartyVideoParams->m_bIsReady;
        m_videoOutParamsStore.Init( bridgePartyVideoParams );
    }
}
/////////////////////////////////////////////////////

bool CBridgePartyVideoRelayOut::StartChangeStreams(bool bImmediately)
{
	PartyRsrcID partyId = m_pBridgePartyCntl->GetPartyRsrcID();
	TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : -----> begin StartChangeStreams - party id = " << partyId;
    bool succeeded = true;
    AskForRelayIntra epIntraParams;
    epIntraParams.m_isImmediately = bImmediately;
    succeeded = CalculateChangeStreamsRequest(epIntraParams);
//    TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : end StartChangeStreams - succeeded = " << (WORD)succeeded << ", epIntraParams size=" << epIntraParams.m_relayIntraParameters.size() << " bImmediately: " << epIntraParams.m_isImmediately;
    if(succeeded)
    {
    	TRACEINTO << " - DEBUGA - succeeded, partyId= " << partyId;
        SendVideoSourcesRequest();
        if(epIntraParams.m_relayIntraParameters.size()>0)
        {
        	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->AskEpsForIntra(epIntraParams);

        }

    }
    else
    {
    	TRACEINTO << "don't send VideoSourcesRequest to party id = " << partyId;
    }

    if( m_videoOutParamsStore.m_needToSendScpStreamsNotification &&
        m_videoOutParamsStore.m_pCurrentVideoScpNotificationRequest->m_numOfPipes > 0 )
    {
    	TRACEINTO << "send Scp Streams Notification to party id = " << partyId;
        ((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->SendScpNotificationReqToPartyCntl( *m_videoOutParamsStore.m_pCurrentVideoScpNotificationRequest );
    }
    SendUpdateToAudioBridgeOnSeenImageIfNeeded();

    TRACESTRFUNC(eLevelDebug) << "DEBUG_SCP : -----> End StartChangeStreams - party id = " << partyId;

    return succeeded;
}


/////////////////////////////////////////////////////

bool CBridgePartyVideoRelayOut::CalculateChangeStreamsRequest(AskForRelayIntra& rEpIntraParams)
{
    bool isSucceeded = true;
    if(!m_pVideoOutStreamsHandler)
    {
        PASSERTMSG_AND_RETURN_VALUE(1, "CalculateChangeStreamsRequest : VideoOutStreamsHandler was not created ", false );
    }
    //TO send only if there was a change TODO
//    bool isSlideState = false;
//    if(m_state == SLIDE)
//    {
//    	isSlideState = true;
//    }

    isSucceeded = m_pVideoOutStreamsHandler->BuildVideoSourcesRequest( m_videoOutParamsStore, rEpIntraParams);

    return isSucceeded;
}

//-----------------------------------------------------------------------------
bool CBridgePartyVideoRelayOut::SendVideoSourcesRequest()
{
    bool bRet = true;
    if(m_pBridgePartyCntl)
    {
        CVideoRelaySourcesParams videoSourcesRequest;
        videoSourcesRequest =  *m_videoOutParamsStore.m_pCurrentVideoSourcesRequest;
//        TRACEINTO << "party id = " << m_pBridgePartyCntl->GetPartyRsrcID();
        ((CVideoHardwareInterface*)m_pHardwareInterface)->SendVideoRelaySourcesRequest(&videoSourcesRequest);   //bRet = m_pBridgePartyCntl->SendActionExternalToPartyTask(partyVideoSourcesRequest);
        int numSources = videoSourcesRequest.GetNumSources();
        if(numSources>0)
        {
        	CScpPipeMappingNotification scpPipeMappingNotification;
        	scpPipeMappingNotification.InitFromSourcesParams(videoSourcesRequest);
        	scpPipeMappingNotification.SetRemoteSequenceNumber(m_videoOutParamsStore.m_pCurrentMediaParam->m_scpRequestSequenceNumber);

        	scpPipeMappingNotification.Dump();
        	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->SendScpPipeMappingToPartyCntl(scpPipeMappingNotification);
        }
        else
        {
        	TRACEINTO << "CBridgePartyVideoRelayOut::SendVideoSourcesRequest the video sources request is empty no need to send scp pipe mapping notification - party id = " << m_pBridgePartyCntl->GetPartyRsrcID();

        }
        if( bRet )
        {
            m_state = CHANGE_STREAMS;//SetState(ChangeStreams);
            StartTimer(VIDEO_OUT_CHANGESTREAMS_TIMEOUT, VIDEO_OUT_CHANGE_STREAMS_TOUT);   //  ChangeVideoSourcesTimeoutIndication timer;
//            AddTimer(timer,Tick(CHANGE_STREAMS_TOUT,0));
        }
        else
        {
            PASSERTMSG(m_pBridgePartyCntl->GetPartyRsrcID(), "failed to send VideoSourcesRequest to party" );//<< LOG_PARTY_RSRC_ID(m_pBridgePartyCntl->GetPartyRsrcID()) );
        }
    }
    else
        PASSERTMSG(1,"m_pBridgePartyCntl is NULL");

    return bRet;
}
//-----------------------------------------------------------------------------

void CBridgePartyVideoRelayOut::EpAskForIntra(const RelayIntraParam& intraParam)
{
	CSegment seg;
	intraParam.Serialize(&seg);
	DispatchEvent(RELAY_ENDPOINT_ASK_FOR_INTRA, &seg);
}

//-----------------------------------------------------------------------------

void CBridgePartyVideoRelayOut::OnEpAskForIntraCONNECTED(CSegment* pParam)
{
	OnEpAskForIntra(pParam);

}
//-----------------------------------------------------------------------------
void CBridgePartyVideoRelayOut::OnEpAskForIntra(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:"<< m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	//DEBUG(epAskForIntraReq);
	//Go over the last sent video streams request.
	//For each pipe id (ssrc) the EP asked from Intra request we will find what is the
	//source of that party and add to list
    if(!m_pVideoOutStreamsHandler)
    {
    	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << "VideoOutStreamsHandler was not created";
        return;
    }
    RelayIntraParam intraParam;
    intraParam.DeSerialize(pParam);
	int numPipesNeedIntra = intraParam.m_listSsrc.size();
	if(numPipesNeedIntra==0)
	{
		TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << " List is empty no need to send request.";
		return;
	}

	AskForRelayIntra epIntraParams;

	std::list<unsigned int>::const_iterator itr = intraParam.m_listSsrc.begin();
	for ( ; itr != intraParam.m_listSsrc.end(); itr++)
	{
		unsigned int requestedPipeIdForIntra = (*itr);
		unsigned int csrc = 0;
		DWORD dwPartyRsrcID = 0;
		bool bIsGdr = intraParam.m_bIsGdr;


		if(m_pVideoOutStreamsHandler->GetCsrsForPipeId(*(m_videoOutParamsStore.m_pCurrentVideoSourcesRequest), requestedPipeIdForIntra, csrc))
		{
			if(m_pVideoOutStreamsHandler->GetPartyIdFromSSRC(csrc, dwPartyRsrcID))
			{
				//temp for GDR integration
				TRACESTRFUNC(eLevelDebug) << "DEBUG_GDR : party id = " << dwPartyRsrcID << ", CSRC: " <<csrc << ", isGDR: " << bIsGdr ;

				m_pVideoOutStreamsHandler->AddIntraReq(epIntraParams.m_relayIntraParameters, dwPartyRsrcID, csrc, bIsGdr);
			}
			else
			{
				TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << "Can't find party with SSRC = "<< csrc;
			}
		}
		else
		{
			TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() <<"Can't find pipe id in the last sent request, required pipe id: "<< requestedPipeIdForIntra;
		}

	}

	if(epIntraParams.m_relayIntraParameters.size()>0)
	{
		epIntraParams.m_isImmediately = true;
		epIntraParams.m_isAllowSuppression = true;
		((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->AskEpsForIntra(epIntraParams);
	}
}
//-----------------------------------------------------------------------------

void  CBridgePartyVideoRelayOut::OnEpAskForIntraIDLE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:"<< m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << "Ignored";
}
//-----------------------------------------------------------------------------
void  CBridgePartyVideoRelayOut::OnEpAskForIntraSTOP_SLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:"<< m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << "Ignored";

}
//-----------------------------------------------------------------------------

void  CBridgePartyVideoRelayOut::OnEpAskForIntraSETUP(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:"<< m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << "Ignored";

}
//-----------------------------------------------------------------------------
void  CBridgePartyVideoRelayOut::OnEpAskForIntraCHANGE_STREAMS(CSegment* pParam)
{
	OnEpAskForIntra(pParam);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyFastUpdateSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();
	if(m_videoOutParamsStore.m_bIsShowSlideToHardwareSent)
	{
		((CVideoHardwareInterface*)m_pHardwareInterface)->SendIvrFastUpdate();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::UpdateOnImageAvcToSvcTranslate()
{
	TRACEINTO << "CCBridgePartyVideoRelayOut::UpdateOnImageAvcToSvcTranslate - UPDATEONIMAGEAVCTOSVC" << m_pBridgePartyCntl->GetName();
	 DispatchEvent(UPDATEONIMAGEAVCTOSVC,NULL);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcCONNECTED(CSegment* pParam)
{
	TRACEINTO << "CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcCONNECTED - run StartChangeStreams" << m_pBridgePartyCntl->GetName();
	StartChangeStreams(true); // BuildLayout();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcSETUP(CSegment* pParam)
{
	TRACEINTO << "CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcSETUP - Error" << m_pBridgePartyCntl->GetName();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " ignore !!!";
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcCHANGESTREAMS(CSegment* pParam)
{
	TRACEINTO << "CBridgePartyVideoRelayOut::OnVideoBridgePartyUpdateOnImageAvcToSvcCHANGESTREAMS - Error" << m_pBridgePartyCntl->GetName();

	m_isChangeStreamWhileWaitingForAck = TRUE;
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::ShowSlide(CSegment *pDataSeg)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " - IVR_SHOW_SLIDE_REQ ";
	DispatchEvent(IVR_SHOW_SLIDE_REQ, pDataSeg);
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyShowSlideSLIDE(CSegment *pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << ", Event is ignored";
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyShowSlideIDLE(CSegment *pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	m_pIVRPlayMessage->DeSerialize(pParam);

	if(m_isReady)
	{
		SendScpIvrShowSlideReqToPartyCntl();
	}
	else
	{
		m_state = SETUP;
		m_videoOutParamsStore.m_bIsSCPShowSlideReqCanBeSent = true;
	}

}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::StopShowSlide(CSegment *pDataSeg)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " - IVR_STOP_SHOW_SLIDE_REQ ";
	DispatchEvent(IVR_STOP_SHOW_SLIDE_REQ, pDataSeg);
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyStopShowSlideSLIDE(CSegment *pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();
	m_state = STOP_SLIDE;
	m_videoOutParamsStore.InitAllIvrSlideParams();
	if(m_videoOutParamsStore.m_pCurrentMediaParam)
	{
		m_videoOutParamsStore.m_showSlideSCPSeqNumber = m_videoOutParamsStore.m_pCurrentMediaParam->GetScpRequestSequenceNumber();
	}
	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->SendScpIvrStopShowSlideReqToPartyCntl();
	((CVideoHardwareInterface*) m_pHardwareInterface)->SendStopShowSlide(pParam);
	StartTimer(STOP_SHOW_SLIDE_TIMEOUT, STOP_SHOW_SLIDE_TIMER);
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnVideoBridgePartyStopShowSlideIDLE(CSegment *pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << ", Event is ignored";
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::AckOnIvrScpShowSlide(BYTE bIsAck)
{
	CSegment *pDataSeg = new CSegment;
	*pDataSeg << bIsAck;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " - ACK_ON_IVR_SCP_SHOW_SLIDE ";
	DispatchEvent(ACK_ON_IVR_SCP_SHOW_SLIDE, pDataSeg);
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnPartyCntlAckOnIvrScpShowSlideSLIDE(CSegment *pDataSeg)
{
	BYTE bIsAck = YES;
	if(pDataSeg)
		*pDataSeg >> bIsAck;
	else
		PASSERT(1);
	WORD ack =  bIsAck;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << ", bIsAck:" <<ack;
	if(bIsAck)
		SetIsAckOnIvrScpShowSlideReceived(true);
	else
		TRACEINTO << "received NACK on the SCP IVR notification we wont send slide";
	if( CanSlideBeSent() )
	{
		SendShowSlide();
	}
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SendShowSlide()
{
	SendShowSlideToHardware();
	//SendEmptyVideoSourcesReqIfNeeded -In case in future there will be a slide during the conference,
	//we need to ensure that we didn’t already send VideSources req to MRMP, this function should call a function in m_pVideoOutStreamsHandler
	//and update the relevant members as m_pCurrentVideoSourcesRequest and m_pCurrentVideoScpNotificationRequest of the m_videoOutParamsStore
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SendShowSlideToHardware()
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	STATUS                         status = STATUS_OK;
	eVideoResolution               videoResolution = eVideoResolutionDummy;
	DWORD                          videoAlg = SVC;
	DWORD                          rVideoBitRate = 0;
	DWORD                          rMBPS = 0; // MBPS - units of 500 macro blocks per second
	DWORD                          rFS = 0; // FS - units of 256 luma macro blocks
	CVideoRelayMediaStream*        pIvrSlideVideoRelayMediaStream = NULL;


	//1. Get 1x1 CVideoRelayMediaStream from Streams list and Update pipeIdForIvrSlide
	CRelayMediaStream* pRelayMediaStream = m_videoOutParamsStore.m_pCurrentMediaParam->m_pStreamsList.front();
	pIvrSlideVideoRelayMediaStream = (CVideoRelayMediaStream*)pRelayMediaStream;
	//m_videoOutParamsStore.m_pipeIdForIvrSlide = pRelayMediaStream->GetSsrc();

	//2. GetOpeartionPoint From List by LayerID for calculating mbps, fs and videoBitRate which we need to send for ShowSlide
	if(pIvrSlideVideoRelayMediaStream)
	{
		if (CalcSlideParamsForHardware(pIvrSlideVideoRelayMediaStream->GetLayerId(), rFS, rMBPS, rVideoBitRate))
		{
			CSegment*                      pDataSeg = new CSegment;		
			m_pIVRPlayMessage->Serialize(pDataSeg);
			status = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendShowSlide(pDataSeg, videoResolution, videoAlg, rVideoBitRate, rFS, rMBPS);
			delete pDataSeg;
		}
		else
		{
			TRACEINTO << "Operation Point with layerId= " << (WORD)pIvrSlideVideoRelayMediaStream->GetLayerId() << ", Not found";
		}
	}

	SetIsShowSlideToHardwareSent(true);
}
///////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoRelayOut::CanSlideBeSent()
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << " ,ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " - IsOneStreamInStraemsList() = " << (WORD)IsOneStreamWithIVRPipeIdInStraemsList();
	return (m_videoOutParamsStore.m_bIsAckOnIvrScpShowSlideReceived && IsOneStreamWithIVRPipeIdInStraemsList());
}
///////////////////////////////////////////////////////////////////////////////
void  CBridgePartyVideoRelayOut::SetIsAckOnIvrScpShowSlideReceived(bool bYesNo)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " - IsAckOnIvrScpShowSlideReceived = " << (WORD)bYesNo;
	m_videoOutParamsStore.m_bIsAckOnIvrScpShowSlideReceived = bYesNo;
}
///////////////////////////////////////////////////////////////////////////////
void  CBridgePartyVideoRelayOut::SetIsShowSlideToHardwareSent(bool bYesNo)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " - SetIsShowSlideToHardwareSent = " << (WORD)bYesNo;
	m_videoOutParamsStore.m_bIsShowSlideToHardwareSent = bYesNo;
}
///////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoRelayOut::IsOneStreamWithIVRPipeIdInStraemsList()
{
	if(m_videoOutParamsStore.m_pCurrentMediaParam)
	{
		if(m_videoOutParamsStore.m_pCurrentMediaParam->m_pStreamsList.size() == 1)
		{
			CRelayMediaStream* pRelayMediaStream = m_videoOutParamsStore.m_pCurrentMediaParam->m_pStreamsList.front();
			DWORD ivrSsrc = m_videoOutParamsStore.m_pCurrentMediaParam->m_pipeIdForIvrSlide;
			if(ivrSsrc == pRelayMediaStream->GetSsrc())
			{
				return true;
			}
			else
			{
				TRACEINTO << "There is only one stream but SSRC = " << pRelayMediaStream->GetSsrc()<< " !=  " << ivrSsrc ;
			}
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SendUpdateSlideToHardware(int layerIdForIvrSlide)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	STATUS                         status = STATUS_OK;
	eVideoResolution               videoResolution = eVideoResolutionDummy;
	DWORD                          videoAlg = SVC;
	DWORD                          rVideoBitRate;
	DWORD                          rMBPS; // MBPS - units of 500 macro blocks per second
	DWORD                          rFS; // FS - units of 256 luma macro blocks


	if(CalcSlideParamsForHardware(layerIdForIvrSlide, rFS, rMBPS, rVideoBitRate))
	{
		status = ((CVideoHardwareInterface*) m_pHardwareInterface)->SendUpdateSlide(videoResolution, videoAlg, rVideoBitRate, rFS, rMBPS);
	}
	else
	{
		TRACEINTO << "Operation Point with layerId= " << (WORD)layerIdForIvrSlide << ", Not found";
	}

}
///////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoRelayOut::CalcSlideParamsForHardware(int layerId, DWORD &fs, DWORD &mbps, DWORD &videoBitRate)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	CVideoOperationPointsSet* pVideoOperationPointsSet = ((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->GetConfVideoOperationPointsSet();
	VideoOperationPoint rFoundVideoOperationPoint;
	bool bFound = pVideoOperationPointsSet->GetOperationPointFromList(layerId, rFoundVideoOperationPoint);
	if( bFound )
	{
		videoBitRate = rFoundVideoOperationPoint.m_maxBitRate*1000;
//		fs = (rFoundVideoOperationPoint.m_frameWidth*rFoundVideoOperationPoint.m_frameHeight)/CUSTOM_MAX_FS_FACTOR;
//		mbps = (fs*rFoundVideoOperationPoint.m_frameRate)/CUSTOM_MAX_MBPS_FACTOR;
//		fs = fs/CUSTOM_MAX_FS_FACTOR;
		mbps = (::CalcOperationPointMBPS(rFoundVideoOperationPoint))/CUSTOM_MAX_MBPS_FACTOR;
		fs = (::CalcOperationPointFS(rFoundVideoOperationPoint)); // FS/CUSTOM_MAX_FS_FACTOR  when CUSTOM_MAX_FS_FACTOR=256
		fs = GetMaxFsAsDevision(fs);
		TRACEINTO << "layerId = " << layerId << "MBPS = " << mbps << ", FS = " << fs << ", videoBitRate = " << videoBitRate;
		return true;
	}
	else
	{
		TRACEINTO << "Operation Point with layerId= " << layerId << ", Not found";
		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SendNotificationIfNeededInSlideStage()
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	DWORD ivrSsrc =  m_videoOutParamsStore.m_pCurrentMediaParam->m_pipeIdForIvrSlide;
	m_pVideoOutStreamsHandler->UpdateNotificationsOnAllNonIVRRequestedStreamsCantBeProvided(m_videoOutParamsStore, ivrSsrc);

	if( m_videoOutParamsStore.m_needToSendScpStreamsNotification &&
	    m_videoOutParamsStore.m_pCurrentVideoScpNotificationRequest->m_numOfPipes > 0 )
	{
		TRACEINTOFUNC << "DEBUG_SCP : Send Scp Streams Notification ";
		((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->SendScpNotificationReqToPartyCntl( *m_videoOutParamsStore.m_pCurrentVideoScpNotificationRequest );
	}
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::AckOnIvrScpStopShowSlide(BYTE bIsAck)
{
	CSegment *pDataSeg = new CSegment;
	*pDataSeg << bIsAck;
	WORD ack = bIsAck;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << ", bIsAck= "<<ack;
	DispatchEvent(ACK_ON_IVR_SCP_STOP_SHOW_SLIDE, pDataSeg);
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnPartyCntlAckOnIvrScpStopShowSlideSTOPSLIDE(CSegment *pDataSeg)
{
	BYTE bIsAck = YES;
	if(pDataSeg)
		*pDataSeg >> bIsAck;
	else
		PASSERT(1);
	WORD ack = bIsAck;
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", PartyName:" << m_pBridgePartyCntl->GetFullName() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << ", bIsAck:" <<ack;

	if(bIsAck)
		SetIsAckOnIvrScpStopShowSlideReceived(true);
	else
		TRACEINTO << "received NACK";

	if( CanSlideStopStateBeFinished() )
	{
		DeleteTimer(STOP_SHOW_SLIDE_TIMEOUT);
		if( m_videoOutParamsStore.m_bIsConnectDuringStopSlideState )
		{
			OnVideoBridgePartyConnect(pDataSeg);
		}
		else
		{
			m_state = IDLE;
		}
	}
	else
	{
		TRACEINTO << "Update SCPReq AfterStop Slide Not Received , m_showSlideSCPSeqNumber = " << (WORD)m_videoOutParamsStore.m_showSlideSCPSeqNumber;
	}
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SetIsAckOnIvrScpStopShowSlideReceived(bool bYesNo)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " - IsAckOnIvrScpStopShowSlideReceived = " << (WORD)bYesNo;
	m_videoOutParamsStore.m_bIsAckOnIvrScpStopSlideReceived = bYesNo;
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SetIsConnectDuringStopSlideState(bool bYesNo)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID() << " - IsAckOnIvrScpStopShowSlideReceived = " << (WORD)bYesNo;
	m_videoOutParamsStore.m_bIsConnectDuringStopSlideState = bYesNo;
}
///////////////////////////////////////////////////////////////////////////////
bool CBridgePartyVideoRelayOut::CanSlideStopStateBeFinished()
{
	if((m_videoOutParamsStore.m_pCurrentMediaParam->GetScpRequestSequenceNumber() != m_videoOutParamsStore.m_showSlideSCPSeqNumber) && m_videoOutParamsStore.m_bIsAckOnIvrScpStopSlideReceived)
	{
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////
int CBridgePartyVideoRelayOut::GetLayerIdForUpdateIvrSlide()
{
	CVideoRelayMediaStream*        pIvrSlideVideoRelayMediaStream = NULL;

	std::list<CRelayMediaStream *>::const_iterator itr_list = m_videoOutParamsStore.m_pCurrentMediaParam->m_pStreamsList.begin();
	for ( ; itr_list != m_videoOutParamsStore.m_pCurrentMediaParam->m_pStreamsList.end(); ++itr_list)
	{
		CRelayMediaStream* pRelayMediaStream = (*itr_list);
		DWORD ivrSsrc = m_videoOutParamsStore.m_pCurrentMediaParam->m_pipeIdForIvrSlide;
		if(pRelayMediaStream->GetSsrc() == ivrSsrc)
		{
			pIvrSlideVideoRelayMediaStream = (CVideoRelayMediaStream*)pRelayMediaStream;
		}
	}
	if(pIvrSlideVideoRelayMediaStream)
	{
		return pIvrSlideVideoRelayMediaStream->GetLayerId();
	}
	return -1;
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::OnTimerStopShowSlideSTOPSLIDE(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << m_pBridgePartyCntl->GetPartyRsrcID() << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();
	PASSERT(m_pBridgePartyCntl->GetPartyRsrcID());
	if( m_videoOutParamsStore.m_bIsConnectDuringStopSlideState )
	{
		OnVideoBridgePartyConnect(pParam);
	}
	else
	{
		m_state = IDLE;
	}
	m_videoOutParamsStore.InitAllIvrSlideParams();
}
///////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SendScpIvrShowSlideReqToPartyCntl()
{
	m_state = SLIDE;
	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->SendScpIvrShowSlideReqToPartyCntl();
}
///////////////////////////////////////////////////////////////////////////////

void CBridgePartyVideoRelayOut::OnVideoBridgePartyToutCHANGESTREAMS(CSegment* pParam)
{
	m_state = CONNECTED;
	PartyRsrcID partyID = m_pBridgePartyCntl->GetPartyRsrcID();
	DBGPASSERT(partyID);
	TRACESTRFUNC(eLevelError) << "CBridgePartyVideoRelayOur::OnVideoBridgePartyToutCHANGESTREAMS - ACK is not received : PartyId:" << partyID << ", ConfId:" << m_pBridgePartyCntl->GetConfRsrcID();

	StartChangeStreamsIsChangeStreamWhileWaitingForAck();
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)
{
	//has no real resources
}
////////////////////////////////////////////////////////////////////////////
BOOL CBridgePartyVideoRelayOut::IsConnected()
{
	if (m_state == CONNECTED || m_state == CHANGE_STREAMS)
		return TRUE;
	else
		return FALSE;
}
////////////////////////////////////////////////////////////////////////////

bool CBridgePartyVideoRelayOut::IsStateIsConnected()
{
	return m_state == CONNECTED;
}
////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoRelayOut::SendUpdateToAudioBridgeOnSeenImageIfNeeded()
{
	if(m_videoOutParamsStore.m_updateOnSeenImageStruct.bIsNeedToSendUpdate)
	{
		if(m_pBridgePartyCntl)
		{
			TRACEINTO<<"";
			((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->SendUpdateToAudioBridgeOnSeenImage(m_videoOutParamsStore.m_updateOnSeenImageStruct.idOfPartyToUpdate,m_videoOutParamsStore.m_updateOnSeenImageStruct.idOfSeenParty);

		}
		else
			PASSERT_AND_RETURN(1);
	}

}
