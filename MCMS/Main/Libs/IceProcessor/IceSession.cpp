#ifndef __DISABLE_ICE__
#define __DISABLE_ICE__
#endif

#ifndef __DISABLE_ICE__

// IceSession.cpp
// Ami Noy

#include <assert.h>

#include "IpChannelParams.h"
//#include "IceDefs.h"
#include "IceSession.h"
#include "AnyFirewallInterface.h"
#include "auto_array.h"
#include "IceCmReq.h"
#include "IceCmInd.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "MplMcmsProtocol.h"
#include "IceMplMsgSender.h"
#include "IceLogAdapter.h"


void CMediaCallback::HandleCallback(int iChannel, char *pData, int iLen, const CAfStdString& sSrcAddress, int iSrcPort, const void *pContext)
{
	string ip = (std::string) sSrcAddress;

//	ICE_LOG_TRACE("channel:%d, len:%d, address:%s, port:%d" , iChannel, iLen, ip.c_str(), iSrcPort);

//	m_pAFEngine->Send(iChannel, pData, iLen, AF_NON_BLOCKING);
	m_pAFEngine->SendTo(iChannel, pData, iLen, sSrcAddress, iSrcPort, AF_NON_BLOCKING);
}

void CSessionCallback::HandleCallback(int iChannel, char *pData, int iLen, const CAfStdString& sSrcAddress, int iSrcPort, const void *pContext)
{
	unsigned char event = (unsigned char)*pData;

	switch (event)
	{
		case EAfEventOfferSDPAvailable:
//			HandleOfferSDPGeneration(iChannel);
			break;
		case EAfEventAnswerSDPAvailable:
//			HandleAnswerSDPGeneration(iChannel);
			break;
		case EAfEventICECheckCompleted:
			ICE_LOG_TRACE("EAfEventICECheckCompleted event" );
			m_pIceSession->SetIceCheckCompleteInd(event);
			break;
		case EAfEventICENeedUpdatedOffer:
			ICE_LOG_TRACE("EAfEventICENeedUpdatedOffer event" );
			m_pIceSession->SendReinvite();// Update C line local reinvite as offerer
			break;
		case EAfEventICECheckFailed:
			ICE_LOG_TRACE("EAfEventICECheckFailed event" );
			m_pIceSession->SetIceCheckCompleteInd(event);
			m_pIceSession->Close();
			break;
		default:
			ICE_LOG_TRACE("Unhandled EAfEvent: %d" , event);
			break;
	}
}

IceSession::IceSession(CAnyFirewallEngine *pAFEngine, int sessId, const IceMplMsgSender &sender)
	:m_pAFEngine(pAFEngine),
	m_sessionId(sessId),
	m_MplMsgSender(sender)
{
	//m_pAFEngine = pAFEngine;

	//m_iControlChannel		= 0;	// The control AFE channel
	m_iAudioChannelRTP		= AF_CHANNEL_INVALID;	// The RTP Audio AFE channel
	m_iAudioChannelRTCP 	= AF_CHANNEL_INVALID;	// The RTCP Audio AFE channel
	m_iVideoChannelRTP 		= AF_CHANNEL_INVALID;	// The RTP Video AFE channel
	m_iVideoChannelRTCP 	= AF_CHANNEL_INVALID;	// The RTCP Video AFE channel
	m_iFeccChannelRTP 		= AF_CHANNEL_INVALID;	// The RTP Fecc AFE channel
	m_iFeccChannelRTCP 		= AF_CHANNEL_INVALID;	// The RTCP Fecc AFE channel
	m_iContentChannelRTP 	= AF_CHANNEL_INVALID;	// The RTP Content AFE channel
	m_iContentChannelRTCP 	= AF_CHANNEL_INVALID;	// The RTCP Content AFE channel
	m_iSession				= AF_CHANNEL_INVALID;

	m_pAudioRtpCallback = new CMediaCallback(m_pAFEngine);
	m_pAudioRtcpCallback = new CMediaCallback(m_pAFEngine);
	m_pVideoRtpCallback = new CMediaCallback(m_pAFEngine);
	m_pVideoRtcpCallback = new CMediaCallback(m_pAFEngine);

	//m_sessionId = sessId;
	ICE_LOG_TRACE("Ice SessionID(CM)=%d" , m_sessionId);
}

IceSession::~IceSession()
{
	ICE_LOG_TRACE("~IceSession\n Ice SessionID(CM)=%d", m_sessionId );
	Close();
	CloseChannels();
	
	delete m_SessionCallback;
	delete m_pAudioRtpCallback;
	delete m_pAudioRtcpCallback;
	delete m_pVideoRtpCallback;
	delete m_pVideoRtcpCallback;
}

#if 0
void *IceSession::SessionRecvThread(void *param) // static
{
	IceSession *pThis = (IceSession*)param;
	pThis->SessionRecvThreadImpl();
	return NULL;
}

void IceSession::SessionRecvThreadImpl()
{
	int iRet;
	int a;
	int	iNumChannelsForSelect = 0;

	char pBuff[2048];

	enum {
		iControlChannelIndex = 0,
//		iSessionIndex,// 1 ??
		iMaxChannelsForSelect
	};

	int aChannels[iMaxChannelsForSelect];
	int aInputEvents[iMaxChannelsForSelect];
	int aOutputEvents[iMaxChannelsForSelect];

	for (a = 0; a < iMaxChannelsForSelect; a++)
	{
		aChannels[a]     = AF_CHANNEL_INVALID;
		aInputEvents[a]  = AF_SELECT_NOEVENT;
		aOutputEvents[a] = AF_SELECT_NOEVENT;
	}

	if (m_iControlChannel) {
		aChannels[iNumChannelsForSelect++]   		= m_iControlChannel;
	}
//	if (m_iSession) {// ??
//		aChannels[iNumChannelsForSelect++]  		= m_iSession;
//	}

	while (m_iSessionThreadID != 0)
	{
		aInputEvents[iControlChannelIndex] = AF_SELECT_READ;
		aOutputEvents[iControlChannelIndex] = AF_SELECT_NOEVENT;

//		for (a = 0; a < iNumChannelsForSelect; a++)
//		{
//			aInputEvents[a]  = AF_SELECT_READ;
//			aOutputEvents[a] = AF_SELECT_NOEVENT;
//		}

		iRet = m_pAFEngine->Select(
			iNumChannelsForSelect,
		    aChannels,
			aInputEvents,
			aOutputEvents,
			AF_TIMEOUT_INFINITE);

		if (iRet <= 0) {
			continue;
		}

		// handle Control events
		if (aOutputEvents[iControlChannelIndex]) {

			if (aOutputEvents[iControlChannelIndex] & AF_SELECT_READ) {
				//refresh aChannels array
				char dummy;
				m_pAFEngine->Recv(m_iControlChannel, &dummy, 1, AF_NON_BLOCKING);
			}
			else if (aOutputEvents[iControlChannelIndex] & AF_SELECT_ERROR) {
				//closed. quit
				ICE_LOG_TRACE("control channel rcv AF_SELECT_ERROR, quit thread" );
				break;
			}
		}
		// Handle Event
		// Handle Session event
//		 else if (aOutputEvents[iSessionIndex]) {
//
//			if (aOutputEvents[iSessionIndex] & AF_SELECT_READ) {
//
//				unsigned long ulEvent;
//
//				int iRet = m_pAFEngine->Recv(m_iSession, (char*)&ulEvent, 4, AF_NON_BLOCKING);
//
//				if (iRet > 0) {
//
//					switch (ulEvent) {
////						case EAfEventICECheckCompleted: //Ring after the Ice Checks complete
////						if (!m_bCaller) {
////							if (!m_pAudio->PlayWavFile(sWavFile))
////						}
////						break;
//
//						case EAfEventICENeedUpdatedOffer:
//						SendReinvite();
//						break;
//
//						default:
//						ICE_LOG_TRACE("session received event:%d" , ulEvent);
//						break;
//					}
//				}
//
//			} else if (aOutputEvents[iSessionIndex] & AF_SELECT_ERROR) {
//				ICE_LOG_TRACE("session channel rcv AF_SELECT_ERROR, quit thread" );
//				//closed. quit
//				break;
//			}
//		}// if (aOutputEvents[iSessionIndex])
	}// while (m_iSessionThreadID != 0)

}// SessionRecvThreadImpl
#endif

void IceSession::PrintLocalAddress()
{
	string sRegIPPort = m_pAFEngine->GetLocalAddress(m_iAudioChannelRTP);

	if (!sRegIPPort.empty()) {

		string m_sMyRaddr = m_pAFEngine->GetHostAddress(sRegIPPort);
		int m_iMyRport = m_pAFEngine->GetHostPort(sRegIPPort);
		ICE_LOG_TRACE("Local Audio RTP- ip:%s, port:%d " , m_sMyRaddr.c_str(), m_iMyRport);
	}
	sRegIPPort = m_pAFEngine->GetLocalAddress(m_iAudioChannelRTCP);

	if (!sRegIPPort.empty()) {

		string m_sMyRaddr = m_pAFEngine->GetHostAddress(sRegIPPort);
		int m_iMyRport = m_pAFEngine->GetHostPort(sRegIPPort);
		ICE_LOG_TRACE("Local Audio RTCP- ip:%s, port:%d " , m_sMyRaddr.c_str(), m_iMyRport);
	}
	sRegIPPort = m_pAFEngine->GetLocalAddress(m_iVideoChannelRTP);

	if (!sRegIPPort.empty()) {

		string m_sMyRaddr = m_pAFEngine->GetHostAddress(sRegIPPort);
		int m_iMyRport = m_pAFEngine->GetHostPort(sRegIPPort);
		ICE_LOG_TRACE("Local Video RTP- ip:%s, port:%d " , m_sMyRaddr.c_str(), m_iMyRport);
	}
	sRegIPPort = m_pAFEngine->GetLocalAddress(m_iVideoChannelRTCP);

	if (!sRegIPPort.empty()) {

		string m_sMyRaddr = m_pAFEngine->GetHostAddress(sRegIPPort);
		int m_iMyRport = m_pAFEngine->GetHostPort(sRegIPPort);
		ICE_LOG_TRACE("Local Video RTCP- ip:%s, port:%d " , m_sMyRaddr.c_str(), m_iMyRport);
	}
}

bool IceSession::Create(mcIceChannelParams *pChannel)
{
	if(m_iSession!=AF_CHANNEL_INVALID) {
		ICE_LOG_TRACE("CreateSession fail. Session was already created before!" );
		assert(0);//if already created before, error!!!
		return false;
	}

	int i;
	int	j;
	//int status;
	int channType;
	int channOp;
	char sMediaStreamSpec[512] = {0};

	//m_iControlChannel 	= m_pAFEngine->Create(AF_CHANNEL_LOOPBACK, 0, 0);

	for (i = 0, j = 0; i < NumOfMediaTypes; i++, pChannel++) {

		channType = pChannel->channelType;
		channOp  = pChannel->channelOperation;

		if (channOp == iceInvalidChannel)
			continue;

		switch (channType) {

			case kIpAudioChnlType:

			m_iAudioChannelRTP 		= m_pAFEngine->Create(AF_CHANNEL_RTP, pChannel->rtp_port[0], pChannel->rtp_port[1], 0);
			m_iAudioChannelRTCP 	= m_pAFEngine->Create(AF_CHANNEL_RTCP, pChannel->rtcp_port[0], pChannel->rtcp_port[1], 0);

			// set callback functions for audio
			m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTP, m_pAudioRtpCallback);
			m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTCP, m_pAudioRtcpCallback);

			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_AUDIO, m_iAudioChannelRTP, m_iAudioChannelRTCP);

			break;

			case kIpVideoChnlType:

			m_iVideoChannelRTP 		= m_pAFEngine->Create(AF_CHANNEL_RTP, pChannel->rtp_port[0], pChannel->rtp_port[1], 0);
			m_iVideoChannelRTCP 	= m_pAFEngine->Create(AF_CHANNEL_RTCP, pChannel->rtcp_port[0], pChannel->rtcp_port[1], 0);

			// set callback functions for video
			m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTP, m_pVideoRtpCallback);
			m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTCP, m_pVideoRtcpCallback);

			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_VIDEO, m_iVideoChannelRTP, m_iVideoChannelRTCP);

			// patch to exit from the loop, because AFE open only 1 audio and 1 video channel.

			break;

			case kIpFeccChnlType:

			m_iFeccChannelRTP 		= m_pAFEngine->Create(AF_CHANNEL_RTP, pChannel->rtp_port[0], pChannel->rtp_port[1], 0);
			m_iFeccChannelRTCP 		= m_pAFEngine->Create(AF_CHANNEL_RTCP, pChannel->rtcp_port[0], pChannel->rtcp_port[1], 0);

			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_APPLICATION, m_iFeccChannelRTP, m_iFeccChannelRTCP);
			//i = NumOfChannels;
			break;

			case kIpContentChnlType:

			m_iContentChannelRTP	= m_pAFEngine->Create(AF_CHANNEL_RTP, pChannel->rtp_port[0], pChannel->rtp_port[1], 0);
			m_iContentChannelRTCP	= m_pAFEngine->Create(AF_CHANNEL_RTCP, pChannel->rtcp_port[0], pChannel->rtcp_port[1], 0);

			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_VIDEO, m_iContentChannelRTP, m_iContentChannelRTCP);

			break;

			case kEmptyChnlType:
				continue;

			default:
			{
			ICE_LOG_TRACE("unknow channel type:%d num media = %d, i = %d" , channType, NumOfMediaTypes,i);
			continue;
			}
		}
	}

	ICE_LOG_TRACE("Trying create session... MediaSpec is %s\n", sMediaStreamSpec);
	m_iSession = m_pAFEngine->CreateSession(sMediaStreamSpec, 0);

	if (m_iSession < 0) {

		CloseChannels();

		ICE_LOG_TRACE("CreateSession fail" );
		return false;
	}

	m_SessionCallback = new CSessionCallback(this);
	m_pAFEngine->SetCallbackHandler(m_iSession, m_SessionCallback);

#if 0
	status = pthread_create(&m_iSessionThreadID, NULL, SessionRecvThread, this);

	if (status) {
		ICE_LOG_TRACE("pthread craete failed. sessionId:%d" , m_iSession);
	}
//	pSessionRcvCallback = new IceSessionRcvCallback(m_iSession, m_pAFEngine);
#endif
	ICE_LOG_TRACE("CreateSession: %s" , sMediaStreamSpec);


	return true;
}

void IceSession::SetAudioRtpCallBack(IAfCallbackHandler* pHandler)
{
	if(!m_pAFEngine->ChannelExists(m_iAudioChannelRTP)) {
		ICE_LOG_TRACE("Set call back failed. Audio Channel is not existed.");
		return;
	}

	IAfCallbackHandler *prev_handler=0;

	prev_handler=m_pAudioRtpCallback;
	m_pAudioRtpCallback=pHandler;

	m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTP, m_pAudioRtpCallback);

	delete prev_handler;
}

void IceSession::SetVideoRtpCallBack(IAfCallbackHandler* pHandler)
{
	if(!m_pAFEngine->ChannelExists(m_iVideoChannelRTP)) {
		ICE_LOG_TRACE("Set call back failed. Video Channel is not existed.");
		return;
	}

	IAfCallbackHandler *prev_handler=0;

	prev_handler=m_pVideoRtpCallback;
	m_pVideoRtpCallback=pHandler;

	m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTP, m_pVideoRtpCallback);

	delete prev_handler;
}

auto_array<char> IceSession::BuildIceSdpIndByReq(const ICE_GENERAL_REQ_S &ICEreq,
											const std::string &localSdp, int &size) const
{
	ICE_GENERAL_IND_S ICEind;
	memset(&ICEind,0,sizeof(ICEind));

	ICEind.status=localSdp.size()>0 ? STATUS_OK : STATUS_FAIL;
	ICEind.ice_session_index=m_sessionId;

	const std::string &SDP_string=localSdp;
	for(int i=0;i<NumOfMediaTypes;++i) {
		if(ICEreq.candidate_list[i].channelType==kIpAudioChnlType) {
			ICEind.ice_channels_id.ice_audio_rtp_id=m_iAudioChannelRTP;
			ICEind.ice_channels_id.ice_audio_rtcp_id=m_iAudioChannelRTCP;

			//ICEind.ice_local_ports.audioRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.audioRtcpPort=ICEreq.candidate_list[i].rtcp_port;
		}
		else if(ICEreq.candidate_list[i].channelType==kIpVideoChnlType) {
			ICEind.ice_channels_id.ice_video_rtp_id=m_iVideoChannelRTP;
			ICEind.ice_channels_id.ice_video_rtcp_id=m_iVideoChannelRTCP;

			//ICEind.ice_local_ports.videoRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.videoRtcpPort=ICEreq.candidate_list[i].rtcp_port;

		}
		else if(ICEreq.candidate_list[i].channelType==kIpFeccChnlType) {
			ICEind.ice_channels_id.ice_data_rtp_id=m_iFeccChannelRTP;
			ICEind.ice_channels_id.ice_data_rtcp_id=m_iFeccChannelRTCP;

			//ICEind.ice_local_ports.dataRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.dataRtcpPort=ICEreq.candidate_list[i].rtcp_port;

		}
		else if(ICEreq.candidate_list[i].channelType==kIpContentChnlType) {
			ICEind.ice_channels_id.ice_content_rtp_id=m_iContentChannelRTP;
			ICEind.ice_channels_id.ice_content_rtcp_id=m_iContentChannelRTCP;

			//ICEind.ice_local_ports.contentRtpPort=ICEreq.candidate_list[i].rtp_port;
			//ICEind.ice_local_ports.contentRtcpPort=ICEreq.candidate_list[i].rtcp_port;

		}
	}

	ICEind.sdp_size=SDP_string.size()+1;//include '\0' size

	size=sizeof(ICEind)+SDP_string.size();
	auto_array<char> msgbuf(new char[size]);
	memset(msgbuf.c_array(), 0, size);

	ICE_GENERAL_IND_S *pICEInd=(ICE_GENERAL_IND_S*)msgbuf.c_array();
	*pICEInd=ICEind;
	strcpy(pICEInd->sdp, SDP_string.c_str());

	return msgbuf;
};

bool IceSession::Offer(CMplMcmsProtocol &mplMsg)
{
	ICE_MAKE_OFFER_REQ_S *pMakeOffer = (ICE_MAKE_OFFER_REQ_S *) mplMsg.GetData();
	mcIceChannelParams *pChannelParams = &pMakeOffer->candidate_list[0];

	bool status=false;

	char 	sHost[64];

	ICE_LOG_TRACE("Creating IceSession. SessionID: %d\n", pMakeOffer->ice_session_index);
	status = Create(pChannelParams);

	int size;
	auto_array<char> msgbuf;
	if (!status) {
		ICE_LOG_TRACE("Create Offer fail" );
		msgbuf=BuildIceSdpIndByReq(*pMakeOffer, std::string(), size);
	}
	else {
		ICE_LOG_TRACE("Offer" );

		m_sMediaSdpInfo = m_pAFEngine->MakeOffer(m_iSession)->sSdp;
		m_sdpSize = m_sMediaSdpInfo.length();

		printf("SDP info:%s", m_sMediaSdpInfo.c_str());

		msgbuf=BuildIceSdpIndByReq(*pMakeOffer, m_sMediaSdpInfo, size);
	}

	mplMsg.AddCommonHeader(ICE_MAKE_OFFER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(size,msgbuf.c_array());

	m_MplMsgSender.SendForMplApi(mplMsg);
//	PrintLocalAddress();

	return status;
}

void IceSession::CloseChannels()
{
	if (m_iAudioChannelRTP!=AF_CHANNEL_INVALID) {
		m_pAFEngine->Close(m_iAudioChannelRTP);
		m_iAudioChannelRTP  = AF_CHANNEL_INVALID;
	}
	if (m_iAudioChannelRTCP!=AF_CHANNEL_INVALID) {
		m_pAFEngine->Close(m_iAudioChannelRTCP);
		m_iAudioChannelRTCP  = AF_CHANNEL_INVALID;
	}
	if (m_iVideoChannelRTP!=AF_CHANNEL_INVALID) {
		m_pAFEngine->Close(m_iVideoChannelRTP);
		m_iVideoChannelRTP = AF_CHANNEL_INVALID;
	}
	if (m_iVideoChannelRTCP!=AF_CHANNEL_INVALID) {
		m_pAFEngine->Close(m_iVideoChannelRTCP);
		m_iVideoChannelRTCP = AF_CHANNEL_INVALID;
	}
	if (m_iFeccChannelRTP!=AF_CHANNEL_INVALID) {
		m_pAFEngine->Close(m_iFeccChannelRTP);
		m_iFeccChannelRTP = AF_CHANNEL_INVALID;
	}
	if (m_iFeccChannelRTCP!=AF_CHANNEL_INVALID) {
		m_pAFEngine->Close(m_iFeccChannelRTCP);
		m_iFeccChannelRTCP = AF_CHANNEL_INVALID;
	}
	if (m_iContentChannelRTP!=AF_CHANNEL_INVALID) {
		m_pAFEngine->Close(m_iContentChannelRTP);
		m_iContentChannelRTP = AF_CHANNEL_INVALID;
	}
	if (m_iContentChannelRTCP!=AF_CHANNEL_INVALID) {
		m_pAFEngine->Close(m_iContentChannelRTCP);
		m_iContentChannelRTCP = AF_CHANNEL_INVALID;
	}
}

void IceSession::CheckChannelsExistence()
{
	if (!m_pAFEngine->ChannelExists(m_iAudioChannelRTP)) {
		m_iAudioChannelRTP  = AF_CHANNEL_INVALID;
	}
	if (!m_pAFEngine->ChannelExists(m_iAudioChannelRTCP)) {
		m_iAudioChannelRTCP  = AF_CHANNEL_INVALID;
	}
	if (!m_pAFEngine->ChannelExists(m_iVideoChannelRTP)) {
		m_iVideoChannelRTP = AF_CHANNEL_INVALID;
	}
	if (!m_pAFEngine->ChannelExists(m_iVideoChannelRTCP)) {
		m_iVideoChannelRTCP = AF_CHANNEL_INVALID;
	}
	if (!m_pAFEngine->ChannelExists(m_iFeccChannelRTP)) {
		m_iFeccChannelRTP = AF_CHANNEL_INVALID;
	}
	if (!m_pAFEngine->ChannelExists(m_iFeccChannelRTCP)) {
		m_iFeccChannelRTCP = AF_CHANNEL_INVALID;
	}
	if (!m_pAFEngine->ChannelExists(m_iContentChannelRTP)) {
		m_iContentChannelRTP = AF_CHANNEL_INVALID;
	}
	if (!m_pAFEngine->ChannelExists(m_iContentChannelRTCP)) {
		m_iContentChannelRTCP = AF_CHANNEL_INVALID;
	}
}

bool IceSession::Close()
{
	//pthread_t iSessionThreadID = m_iSessionThreadID;
	//m_iSessionThreadID = 0;

	ICE_LOG_TRACE("IceSession::Close" );

	if (m_iSession!=AF_CHANNEL_INVALID
		&& !m_pAFEngine->IsClosed(m_iSession)) {
		bool res=m_pAFEngine->CloseSession(m_iSession);
		m_iSession=AF_CHANNEL_INVALID;
		return res;
	}
	// Wake up recv thread to end it
	//char dummy = -1;
	//m_pAFEngine->Send(m_iControlChannel, &dummy, 1, AF_NON_BLOCKING);

	//pthread_join(iSessionThreadID, NULL);
/*
	if (pAudioRtpCallback)
		delete pAudioRtpCallback;

	if (pAudioRtcpCallback)
		delete pAudioRtcpCallback;

	if (pVideoRtpCallback)
		delete pAudioRtpCallback;

	if (pVideoRtcpCallback)
		delete pAudioRtcpCallback;
*/
	return false;
}


bool IceSession::Answer(CMplMcmsProtocol &mplMsg)
{
	bool status=false;
	ICE_MAKE_ANSWER_REQ_S &IceReq=*((ICE_MAKE_ANSWER_REQ_S*)mplMsg.GetData());
	mcIceChannelParams *pChannelParams = &IceReq.candidate_list[0];
	const char *pRemoteSdp=IceReq.sdp;

	StoreLastMplMcmsMsg(mplMsg);

	ICE_LOG_TRACE("Creating IceSession. SessionID: %d\n", IceReq.ice_session_index);
	status = Create(pChannelParams);

	int size;
	auto_array<char> msgbuf;
	if (!status) {
		ICE_LOG_TRACE("Create Answer fail" );
		msgbuf=BuildIceSdpIndByReq(IceReq, std::string(), size);
	}
	else {
		printf("Answer remote SDP info:\n%s", pRemoteSdp);

		ICE_LOG_TRACE("Answer" );

		m_sMediaSdpInfo = m_pAFEngine->MakeAnswer(m_iSession, pRemoteSdp)->sSdp;

		m_sdpSize = m_sMediaSdpInfo.length();

		printf("SDP info:%s", m_sMediaSdpInfo.c_str());

		msgbuf=BuildIceSdpIndByReq(IceReq, m_sMediaSdpInfo, size);
	}


	mplMsg.AddCommonHeader(ICE_MAKE_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(size,msgbuf.c_array());

	m_MplMsgSender.SendForMplApi(mplMsg);

	//PrintLocalAddress();
	return status;
}

bool IceSession::MakeAnswerNoCreate(CMplMcmsProtocol &mplMsg)
{
	bool status=true;
	ICE_MODIFY_SESSION_ANSWER_REQ_S *pMakeAnswer = (ICE_MODIFY_SESSION_ANSWER_REQ_S *) mplMsg.GetData();

	if(pMakeAnswer->isModifyChannels) {
		mcIceChannelParams *pChannelParams = &pMakeAnswer->candidate_list[0];
		//status = ModifySessionAnswerReq(pChannelParams, pMakeAnswer->sdp);
		status = ModifySession(pChannelParams);
	}
	
	int size;
	auto_array<char> msgbuf;
	if(status) {
		AnswerNoCreate(pMakeAnswer->sdp);
		msgbuf=BuildIceSdpIndByReq(*pMakeAnswer, m_sMediaSdpInfo, size);		
	}
	else {
		ICE_LOG_TRACE("MakeAnswerNoCreate  failed" );
		msgbuf=BuildIceSdpIndByReq(*pMakeAnswer, std::string(), size);
	}

	mplMsg.AddCommonHeader(ICE_MODIFY_SESSION_ANSWER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(size,msgbuf.c_array());

	m_MplMsgSender.SendForMplApi(mplMsg);

	return status;
}// MakeAnswerNoCreate

void IceSession::AnswerNoCreate(const char *pRemoteSdp)
{
	printf("AnswerNoCreate remote SDP info:\n%s", pRemoteSdp);

	ICE_LOG_TRACE("Answer" );

	m_sMediaSdpInfo = m_pAFEngine->MakeAnswer(m_iSession, pRemoteSdp)->sSdp;

	m_sdpSize = m_sMediaSdpInfo.length();

	ICE_LOG_TRACE("SDP info:%s", m_sMediaSdpInfo.c_str());
}

bool IceSession::MakeOfferNoCreate(CMplMcmsProtocol &mplMsg)
{
	bool status=true;
	ICE_MODIFY_SESSION_OFFER_REQ_S *pMakeOffer = (ICE_MODIFY_SESSION_OFFER_REQ_S *) mplMsg.GetData();

	if(pMakeOffer->isModifyChannels) {
		mcIceChannelParams *pChannelParams = &pMakeOffer->candidate_list[0];
		//status = ModifySessionOfferReq(pChannelParams);
		status = ModifySession(pChannelParams);
	}

	int size;
	auto_array<char> msgbuf;
	if(status) {
		OfferNoCreate();
		msgbuf=BuildIceSdpIndByReq(*pMakeOffer, m_sMediaSdpInfo, size);		
	}
	else {
		ICE_LOG_TRACE("MakeOfferNoCreate  failed" );
		msgbuf=BuildIceSdpIndByReq(*pMakeOffer, std::string(), size);		
	}

	mplMsg.AddCommonHeader(ICE_MODIFY_SESSION_OFFER_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	mplMsg.AddData(size,msgbuf.c_array());

	m_MplMsgSender.SendForMplApi(mplMsg);

	return status;
}// MakeAnswerNoCreate

void IceSession::OfferNoCreate()
{
	printf("OfferNoCreate\n");

	ICE_LOG_TRACE("Offer" );

	m_sMediaSdpInfo = m_pAFEngine->MakeOffer(m_iSession)->sSdp;

	m_sdpSize = m_sMediaSdpInfo.length();

	ICE_LOG_TRACE("SDP info:%s", m_sMediaSdpInfo.c_str());
}

bool IceSession::ModifySession(mcIceChannelParams *pChannel)
{
	char sMediaStreamSpec[512] = {0};

	for (int i = 0, j = 0; i < NumOfMediaTypes; i++, pChannel++) {

		int channType = pChannel->channelType;
		int channOp  = pChannel->channelOperation;

		if (channOp == iceInvalidChannel)
			continue;

		switch (channType) {

			case kIpAudioChnlType:
				if (!m_pAFEngine->ChannelExists(m_iAudioChannelRTP)) {
	//				if (channOp == iceCloseChannel)	{
	//
	//					m_pAFEngine->Close(m_iAudioChannelRTP);
	//				}
	//			} else {
					if (channOp == iceOpenChannel) {

						m_iAudioChannelRTP = m_pAFEngine->Create(AF_CHANNEL_RTP, pChannel->rtp_port[0], pChannel->rtp_port[1], 0);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTP, m_pAudioRtpCallback);
					}
				}

				if (!m_pAFEngine->ChannelExists(m_iAudioChannelRTCP)) {
	//				if (channOp == iceCloseChannel)	{
	//
	//					m_pAFEngine->Close(m_iAudioChannelRTCP);
	//				}
	//			} else {
					if (channOp == iceOpenChannel) {

						m_iAudioChannelRTCP = m_pAFEngine->Create(AF_CHANNEL_RTCP, pChannel->rtcp_port[0], pChannel->rtcp_port[1], 0);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTCP, m_pAudioRtcpCallback);
					}
				}

				j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_AUDIO, m_iAudioChannelRTP, m_iAudioChannelRTCP);

				break;

			case kIpVideoChnlType:

				if (!m_pAFEngine->ChannelExists(m_iVideoChannelRTP)) {
//					if (channOp == iceCloseChannel)	{
//
//						m_pAFEngine->Close(m_iVideoChannelRTP);
//
//					}
//				} else {
					if (channOp == iceOpenChannel) {

						m_iVideoChannelRTP = m_pAFEngine->Create(AF_CHANNEL_RTP, pChannel->rtp_port[0], pChannel->rtp_port[1], 0);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTP, m_pVideoRtpCallback);
					}
				}

				if (!m_pAFEngine->ChannelExists(m_iVideoChannelRTCP)) {
//					if (channOp == iceCloseChannel)	{
//
//						m_pAFEngine->Close(m_iVideoChannelRTCP);
//					}
//				} else {
					if (channOp == iceOpenChannel) {

						m_iVideoChannelRTCP = m_pAFEngine->Create(AF_CHANNEL_RTCP, pChannel->rtcp_port[0], pChannel->rtcp_port[1], 0);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTCP, m_pVideoRtcpCallback);
					}
				}


				j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_VIDEO, m_iVideoChannelRTP, m_iVideoChannelRTCP);

				// patch to exit from the loop, because AFE open only 1 audio and 1 video channel.
				i = NumOfMediaTypes;
				break;

//			case kIpFeccChnlType:
//
//			m_iFeccChannelRTP 		= m_pAFEngine->Create(AF_CHANNEL_RTP);
//			m_iFeccChannelRTCP 		= m_pAFEngine->Create(AF_CHANNEL_RTCP);
//
//			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_APPLICATION, m_iFeccChannelRTP, m_iFeccChannelRTCP);
//			//i = NumOfChannels;
//			break;
//
//			case kIpContentChnlType:
//
//			m_iContentChannelRTP	= m_pAFEngine->Create(AF_CHANNEL_RTP);
//			m_iContentChannelRTCP	= m_pAFEngine->Create(AF_CHANNEL_RTCP);
//
//			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_VIDEO, m_iContentChannelRTP, m_iContentChannelRTCP);
//
//			break;

			default:
			ICE_LOG_TRACE("unknow channel type:%d" , channType);
			continue;
		}
	}


	bool updateStatus = m_pAFEngine->ModifySession(m_iSession, sMediaStreamSpec);

	ICE_LOG_TRACE("Update candidate: %s [status %s]" , sMediaStreamSpec, (updateStatus ? "OK" : "ERR"));

	//if the channel is closed in ModifySession, update channel member to INVALID
	CheckChannelsExistence();

	return updateStatus;

}

bool IceSession::ProcessAnswer(char *pRemoteSdp)
{
	return m_pAFEngine->ProcessAnswer(m_iSession, pRemoteSdp);
}// RemoteAnswer

void IceSession::SendReinvite()
{
	ICE_LOG_TRACE("SendReinvite" );


	m_sMediaSdpInfo = m_pAFEngine->MakeOffer(m_iSession)->sSdp;
	m_sdpSize = m_sMediaSdpInfo.length();

	printf("SDP info:%s", m_sMediaSdpInfo.c_str());

//	PrintLocalAddress();
	ICE_LOG_TRACE("SendReinvite, connId=%d sdp-%s" , m_sessionId, m_sMediaSdpInfo.c_str());

	//build and send   Re-INVITE  Indication
	if(!m_lastMplMcmsMsg.get()) {
		ICE_LOG_TRACE("IceSession SendReinvite failed, didn't receive MplMcmsProtocol message before." );
		return;
	}

	int size;
	ICE_GENERAL_REQ_S *processAnswerReq=(ICE_GENERAL_REQ_S *)m_lastMplMcmsMsg->GetData();
	auto_array<char> msgbuf=BuildIceSdpIndByReq(*processAnswerReq, m_sMediaSdpInfo, size);
	
	CMplMcmsProtocol sendReinvInd(*m_lastMplMcmsMsg);
	sendReinvInd.AddCommonHeader(ICE_REINVITE_IND,MPL_PROTOCOL_VERSION_NUM,0,(BYTE)eMpl,(BYTE)eMcms);
	sendReinvInd.AddData(size,msgbuf.c_array());

	m_MplMsgSender.SendForMplApi(sendReinvInd);

	return;
}

void IceSession::SetIceCheckCompleteInd(int event)
{
	ICE_CHECK_COMPLETE_IND_S CheckCompleteInd;
	memset(&CheckCompleteInd, 0, sizeof(CheckCompleteInd));

	CheckCompleteInd.status = event==EAfEventICECheckCompleted ? STATUS_OK : STATUS_FAIL;
	CheckCompleteInd.ice_session_index = m_sessionId;

	if(event==EAfEventICECheckCompleted) {
		std::string	localType;
		std::string	localIp;
		int			localPort;
		CAfStdString 	localAddress;

		std::string	remoteType;
		std::string	remoteIp;
		int			remotePort;
		CAfStdString 	remoteAddress;

		std::pair<int, int> channels[NumOfMediaTypes];
		channels[0].first=m_iAudioChannelRTP;
		channels[0].second=m_iAudioChannelRTCP;
		channels[1].first=m_iVideoChannelRTP;
		channels[1].second=m_iVideoChannelRTCP;
		channels[2].first=m_iFeccChannelRTP;
		channels[2].second=m_iFeccChannelRTCP;
		channels[3].first=m_iContentChannelRTP;
		channels[3].second=m_iContentChannelRTCP;

		for(int i=0; i<NumOfMediaTypes; ++i) {
			if (m_pAFEngine->ChannelExists(channels[i].first)) {
				localAddress 	= m_pAFEngine->GetLocalAddress(channels[i].first);
				localType 		= m_pAFEngine->GetHostType(localAddress);
				localIp		= m_pAFEngine->GetHostAddress(localAddress);
				localPort		= m_pAFEngine->GetHostPort(localAddress);

				ICE_LOG_TRACE("Local RTP channel(%d): type:%s, ip:%s, port:%d" , i+1, localType.c_str() , localIp.c_str(), localPort);

				CheckCompleteInd.chosen_candidates[i].local_candidate.mediaType = i+1;
				strncpy (CheckCompleteInd.chosen_candidates[i].local_candidate.type, localType.c_str(), CandidateTypeLen);
				strncpy (CheckCompleteInd.chosen_candidates[i].local_candidate.ip, localIp.c_str(),CandidateIpLen);
				CheckCompleteInd.chosen_candidates[i].local_candidate.port = localPort;


				remoteAddress=m_pAFEngine->GetRemoteAddress(channels[i].first);
				remoteType=m_pAFEngine->GetHostType(remoteAddress);
				remoteIp=m_pAFEngine->GetHostAddress(remoteAddress);
				remotePort=m_pAFEngine->GetHostPort(remoteAddress);

				ICE_LOG_TRACE("Remote RTP channel(%d): type:%s, ip:%s, port:%d" , i+1, remoteType.c_str() , remoteIp.c_str(), remotePort);

				CheckCompleteInd.chosen_candidates[i].remote_candidate.mediaType = i+1;
				strncpy (CheckCompleteInd.chosen_candidates[i].remote_candidate.type, remoteType.c_str(), CandidateTypeLen);
				strncpy (CheckCompleteInd.chosen_candidates[i].remote_candidate.ip, remoteIp.c_str(),CandidateIpLen);
				CheckCompleteInd.chosen_candidates[i].remote_candidate.port = remotePort;

			}

		}

	}

	//build and check complete  Indication
	if(!m_lastMplMcmsMsg.get()) {
		ICE_LOG_TRACE("IceSession SetIceCheckCompleteInd failed, didn't receive MplMcmsProtocol message before." );
		return;
	}

	CMplMcmsProtocol sendchkCompleteInd(*m_lastMplMcmsMsg);
	sendchkCompleteInd.AddCommonHeader(ICE_CHECK_COMPLETE_IND, MPL_PROTOCOL_VERSION_NUM, 0, (BYTE)eMpl, (BYTE)eMcms);
	sendchkCompleteInd.AddData(sizeof(CheckCompleteInd), (char*)&CheckCompleteInd);

	m_MplMsgSender.SendForMplApi(sendchkCompleteInd);

	return;



#if 0

	int status;

	std::string sType 	= pLocalType;
	std::string sIp 	= pLocalIp;

	if (m_pAFEngine->ChannelExists(m_iAudioChannelRTP)) {

		pLocalAddress 	= m_pAFEngine->GetLocalAddress(m_iAudioChannelRTP);
		pLocalType 		= m_pAFEngine->GetHostType(pLocalAddress);
		pLocalIp		= m_pAFEngine->GetHostAddress(pLocalAddress);
		localPort		= m_pAFEngine->GetHostPort(pLocalAddress);

		sType 	= pLocalType;
		sIp 	= pLocalIp;

		ICE_LOG_TRACE("Local Audio RTP: type:%s, ip:%s, port:%d" , sType.c_str() , sIp.c_str(), localPort);

		pRemoteAddress 	= m_pAFEngine->GetRemoteAddress(m_iAudioChannelRTP);
		pRemoteType 	= m_pAFEngine->GetHostType(pRemoteAddress);
		pRemoteIp		= m_pAFEngine->GetHostAddress(pRemoteAddress);
		pRemotePort		= m_pAFEngine->GetHostPort(pRemoteAddress);

		sType 	= pRemoteType;
		sIp 	= pRemoteIp;

		ICE_LOG_TRACE("Remote Audio RTP: type:%s, ip:%s, port:%d" , sType.c_str() , sIp.c_str(), pRemotePort);
	}

	if (m_pAFEngine->ChannelExists(m_iAudioChannelRTCP)) {

		pLocalAddress 	= m_pAFEngine->GetLocalAddress(m_iAudioChannelRTCP);
		pLocalType 		= m_pAFEngine->GetHostType(pLocalAddress);
		pLocalIp		= m_pAFEngine->GetHostAddress(pLocalAddress);
		localPort		= m_pAFEngine->GetHostPort(pLocalAddress);

		sType 	= pLocalType;
		sIp 	= pLocalIp;

		ICE_LOG_TRACE("Local Audio RTCP: type:%s, ip:%s, port:%d" , sType.c_str() , sIp.c_str(), localPort);

		pRemoteAddress 	= m_pAFEngine->GetRemoteAddress(m_iAudioChannelRTCP);
		pRemoteType 	= m_pAFEngine->GetHostType(pRemoteAddress);
		pRemoteIp		= m_pAFEngine->GetHostAddress(pRemoteAddress);
		pRemotePort		= m_pAFEngine->GetHostPort(pRemoteAddress);

		sType 	= pRemoteType;
		sIp 	= pRemoteIp;

		ICE_LOG_TRACE("Remote Audio RTCP: type:%s, ip:%s, port:%d" , sType.c_str() , sIp.c_str(), pRemotePort);
	}
	if (m_pAFEngine->ChannelExists(m_iVideoChannelRTP)) {

		pLocalAddress 	= m_pAFEngine->GetLocalAddress(m_iVideoChannelRTP);
		pLocalType 		= m_pAFEngine->GetHostType(pLocalAddress);
		pLocalIp		= m_pAFEngine->GetHostAddress(pLocalAddress);
		localPort		= m_pAFEngine->GetHostPort(pLocalAddress);

		sType 	= pLocalType;
		sIp 	= pLocalIp;

		ICE_LOG_TRACE("Local Video RTP: type:%s, ip:%s, port:%d" , sType.c_str() , sIp.c_str(), localPort);

		pRemoteAddress 	= m_pAFEngine->GetRemoteAddress(m_iVideoChannelRTP);
		pRemoteType 	= m_pAFEngine->GetHostType(pRemoteAddress);
		pRemoteIp		= m_pAFEngine->GetHostAddress(pRemoteAddress);
		pRemotePort		= m_pAFEngine->GetHostPort(pRemoteAddress);

		sType 	= pRemoteType;
		sIp 	= pRemoteIp;

		ICE_LOG_TRACE("Remote Video RTP: type:%s, ip:%s, port:%d" , sType.c_str() , sIp.c_str(), pRemotePort);
	}

	if (m_pAFEngine->ChannelExists(m_iVideoChannelRTCP)) {

		pLocalAddress 	= m_pAFEngine->GetLocalAddress(m_iVideoChannelRTCP);
		pLocalType 		= m_pAFEngine->GetHostType(pLocalAddress);
		pLocalIp		= m_pAFEngine->GetHostAddress(pLocalAddress);
		localPort		= m_pAFEngine->GetHostPort(pLocalAddress);

		sType 	= pLocalType;
		sIp 	= pLocalIp;

		ICE_LOG_TRACE("Local Video RTP: type:%s, ip:%s, port:%d" , sType.c_str() , sIp.c_str(), localPort);

		pRemoteAddress 	= m_pAFEngine->GetRemoteAddress(m_iVideoChannelRTCP);
		pRemoteType 	= m_pAFEngine->GetHostType(pRemoteAddress);
		pRemoteIp		= m_pAFEngine->GetHostAddress(pRemoteAddress);
		pRemotePort		= m_pAFEngine->GetHostPort(pRemoteAddress);

		sType 	= pRemoteType;
		sIp 	= pRemoteIp;

		ICE_LOG_TRACE("Remote Video RTP: type:%s, ip:%s, port:%d" , sType.c_str() , sIp.c_str(), pRemotePort);
	}

	//status = IceSendCheckCompleteInd();

	return 0;
#endif
}

//int IceSession::UpdateCandidates(char *pRemoteSdp, int *pSdpSize, IceManager *pIceMngr)
//{
//	printf("UpdateCandidates Remote:%s", pRemoteSdp);
//
//	m_sMediaSdpInfo = m_pAFEngine->MakeAnswer(m_iSession, pRemoteSdp);
//
//	m_sdpSize = m_sMediaSdpInfo.length();
//	*pSdpSize = m_sdpSize;
//
//	printf("SDP info:%s", m_sMediaSdpInfo.c_str());
//
//	return 0;
//}

#if 0
// Remote change the SDP
bool IceSession::ModifySessionAnswerReq(mcIceChannelParams *pChannel, const char *pRemoteSdp)
{
	int i;
	int	j;
	int channType;
	int channOp;
	int updateStatus = 0;

	char sMediaStreamSpec[512] = {0};

	for (i = 0, j = 0; i < NumOfMediaTypes; i++, pChannel++) {

		channType = pChannel->channelType;
		channOp  = pChannel->channelOperation;

		if (channOp == iceInvalidChannel)
			continue;

		switch (channType) {

			case kIpAudioChnlType:
				if (!m_pAFEngine->ChannelExists(m_iAudioChannelRTP)) {
	//				if (channOp == iceCloseChannel)	{
	//
	//					m_pAFEngine->Close(m_iAudioChannelRTP);
	//				}
	//			} else {
					if (channOp == iceOpenChannel) {

						m_iAudioChannelRTP = m_pAFEngine->Create(AF_CHANNEL_RTP, pChannel->rtp_port[0], pChannel->rtp_port[1]);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTP, m_pAudioRtpCallback);
					}
				}

				if (!m_pAFEngine->ChannelExists(m_iAudioChannelRTCP)) {
	//				if (channOp == iceCloseChannel)	{
	//
	//					m_pAFEngine->Close(m_iAudioChannelRTCP);
	//				}
	//			} else {
					if (channOp == iceOpenChannel) {

						m_iAudioChannelRTCP = m_pAFEngine->Create(AF_CHANNEL_RTCP, pChannel->rtcp_port[0], pChannel->rtcp_port[1]);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTCP, m_pAudioRtcpCallback);
					}
				}

				j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_AUDIO, m_iAudioChannelRTP, m_iAudioChannelRTCP);

				break;

			case kIpVideoChnlType:

				if (!m_pAFEngine->ChannelExists(m_iVideoChannelRTP)) {
//					if (channOp == iceCloseChannel)	{
//
//						m_pAFEngine->Close(m_iVideoChannelRTP);
//
//					}
//				} else {
					if (channOp == iceOpenChannel) {

						m_iVideoChannelRTP = m_pAFEngine->Create(AF_CHANNEL_RTP, pChannel->rtp_port[0], pChannel->rtp_port[1]);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTP, m_pVideoRtpCallback);
					}
				}

				if (!m_pAFEngine->ChannelExists(m_iVideoChannelRTCP)) {
//					if (channOp == iceCloseChannel)	{
//
//						m_pAFEngine->Close(m_iVideoChannelRTCP);
//					}
//				} else {
					if (channOp == iceOpenChannel) {

						m_iVideoChannelRTCP = m_pAFEngine->Create(AF_CHANNEL_RTCP, pChannel->rtcp_port[0], pChannel->rtcp_port[1]);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTCP, m_pVideoRtcpCallback);
					}
				}


				j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_VIDEO, m_iVideoChannelRTP, m_iVideoChannelRTCP);

				// patch to exit from the loop, because AFE open only 1 audio and 1 video channel.
				i = NumOfMediaTypes;
				break;

//			case kIpFeccChnlType:
//
//			m_iFeccChannelRTP 		= m_pAFEngine->Create(AF_CHANNEL_RTP);
//			m_iFeccChannelRTCP 		= m_pAFEngine->Create(AF_CHANNEL_RTCP);
//
//			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_APPLICATION, m_iFeccChannelRTP, m_iFeccChannelRTCP);
//			//i = NumOfChannels;
//			break;
//
//			case kIpContentChnlType:
//
//			m_iContentChannelRTP	= m_pAFEngine->Create(AF_CHANNEL_RTP);
//			m_iContentChannelRTCP	= m_pAFEngine->Create(AF_CHANNEL_RTCP);
//
//			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_VIDEO, m_iContentChannelRTP, m_iContentChannelRTCP);
//
//			break;

			default:
			ICE_LOG_TRACE("unknow channel type:%d" , channType);
			continue;
		}
	}


	updateStatus = m_pAFEngine->ModifySession(m_iSession, sMediaStreamSpec);

	ICE_LOG_TRACE("Update candidate: %s [status %s]" , sMediaStreamSpec, (updateStatus == 1 ? "OK" : "NOK"));

	m_sMediaSdpInfo = m_pAFEngine->MakeAnswer(m_iSession, pRemoteSdp);

	//?? Koren
	//m_sMediaSdpInfo = m_pAFEngine->GetSessionSDP(m_iSession);
	//??

	m_sdpSize = m_sMediaSdpInfo.length();

	ICE_LOG_TRACE("Update candidate SDP info:%s" , m_sMediaSdpInfo.c_str());

	return true;

}// ModifySessionAnswerReq

// RMX Local SDP changed
bool IceSession::ModifySessionOfferReq(mcIceChannelParams *pChannel)
{
	int i;
	int	j;
	int channType;
	int channOp;
	int updateStatus = 0;

	char sMediaStreamSpec[512] = {0};

	for (i = 0, j = 0; i < NumOfMediaTypes; i++, pChannel++) {

		channType = pChannel->channelType;
		channOp  = pChannel->channelOperation;

		if (channOp == iceInvalidChannel)
			continue;

		switch (channType) {

			case kIpAudioChnlType:
				if (!m_pAFEngine->ChannelExists(m_iAudioChannelRTP)) {
	//				if (channOp == iceCloseChannel)	{
	//
	//					m_pAFEngine->Close(m_iAudioChannelRTP);
	//				}
	//			} else {
					if (channOp == iceOpenChannel) {

						m_iAudioChannelRTP = m_pAFEngine->Create(AF_CHANNEL_RTP, 0, 0);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTP, m_pAudioRtpCallback);
					}
				}

				if (!m_pAFEngine->ChannelExists(m_iAudioChannelRTCP)) {
	//				if (channOp == iceCloseChannel)	{
	//
	//					m_pAFEngine->Close(m_iAudioChannelRTCP);
	//				}
	//			} else {
					if (channOp == iceOpenChannel) {

						m_iAudioChannelRTCP = m_pAFEngine->Create(AF_CHANNEL_RTCP, 0, 0);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iAudioChannelRTCP, m_pAudioRtcpCallback);
					}
				}

				j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_AUDIO, m_iAudioChannelRTP, m_iAudioChannelRTCP);

				break;

			case kIpVideoChnlType:

				if (!m_pAFEngine->ChannelExists(m_iVideoChannelRTP)) {
//					if (channOp == iceCloseChannel)	{
//
//						m_pAFEngine->Close(m_iVideoChannelRTP);
//
//					}
//				} else {
					if (channOp == iceOpenChannel) {

						m_iVideoChannelRTP = m_pAFEngine->Create(AF_CHANNEL_RTP, 0, 0);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTP, m_pVideoRtpCallback);
					}
				}

				if (!m_pAFEngine->ChannelExists(m_iVideoChannelRTCP)) {
//					if (channOp == iceCloseChannel)	{
//
//						m_pAFEngine->Close(m_iVideoChannelRTCP);
//					}
//				} else {
					if (channOp == iceOpenChannel) {

						m_iVideoChannelRTCP = m_pAFEngine->Create(AF_CHANNEL_RTCP, 0, 0);

						// set callback functions for audio
						m_pAFEngine->SetCallbackHandler(m_iVideoChannelRTCP, m_pVideoRtcpCallback);
					}
				}

				j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_VIDEO, m_iVideoChannelRTP, m_iVideoChannelRTCP);

				// patch to exit from the loop, because AFE open only 1 audio and 1 video channel.
				i = NumOfMediaTypes;
				break;

//			case kIpFeccChnlType:
//
//			m_iFeccChannelRTP 		= m_pAFEngine->Create(AF_CHANNEL_RTP);
//			m_iFeccChannelRTCP 		= m_pAFEngine->Create(AF_CHANNEL_RTCP);
//
//			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_APPLICATION, m_iFeccChannelRTP, m_iFeccChannelRTCP);
//			//i = NumOfChannels;
//			break;
//
//			case kIpContentChnlType:
//
//			m_iContentChannelRTP	= m_pAFEngine->Create(AF_CHANNEL_RTP);
//			m_iContentChannelRTCP	= m_pAFEngine->Create(AF_CHANNEL_RTCP);
//
//			j += sprintf(&sMediaStreamSpec[j], "%s %d %d;", AF_MEDIA_STREAM_VIDEO, m_iContentChannelRTP, m_iContentChannelRTCP);
//
//			break;


			default:
			ICE_LOG_TRACE(" UpdateOfferCandidates unknow channel type:%d" , channType);
			continue;
		}
	}


	updateStatus = m_pAFEngine->ModifySession(m_iSession, sMediaStreamSpec);

	ICE_LOG_TRACE("Update candidate: %s [status %s]" , sMediaStreamSpec, (updateStatus == 1 ? "OK" : "NOK"));

	m_sMediaSdpInfo = m_pAFEngine->MakeOffer(m_iSession);

	m_sdpSize = m_sMediaSdpInfo.length();

	ICE_LOG_TRACE("Update candidate SDP info:%s" , m_sMediaSdpInfo.c_str());

	return true;

}// ModifySessionOfferReq
#endif

void IceSession::StoreLastMplMcmsMsg(const CMplMcmsProtocol &mplMsg)
{
	//store the mplMsg, callback function will use it later
	m_lastMplMcmsMsg=std::auto_ptr<CMplMcmsProtocol>(new CMplMcmsProtocol(mplMsg));
}

#endif	//__DISABLE_ICE__
