//+========================================================================+
//                CSVCToAVCTranslator.CPP                                  +
//+========================================================================+


//includes
#include "SVCToAVCTranslator.h"
#include "ProcessBase.h"
#include "HlogApi.h"
#include "SvcToAvcParams.h"
#include "BridgePartyVideoIn.h"
#include "BridgePartyVideoRelayIn.h"
#include "BridgePartyCntl.h"
#include "VideoHardwareInterface.h"
#include "RsrcParams.h"
#include "OpcodesMcmsCardMngrTB.h"

#include "VideoRelaySourceApi.h"
#include "MrcApiDefinitions.h"
#include "CapInfo.h"
#include "ConfPartyGlobals.h"
#include "H264Util.h"

#include "TraceHeader.h"
#include "FilterTraceContainer.h"
#include "OpcodesMcmsAudio.h"

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
// ------------------------------------------------------------

// defines
#define OUT_CHANNEL_CONST_SSRC 						7777

// defines TOUT
#define	VIDEO_DECODER_SVC_SYNC_TOUT_VALUE			1*SECOND
#define OPEN_VIDEO_DECODER_SSRC_ART_TOUT_VALUE 	 	20*SECOND
#define DECODER_SYNC_FAILURE_WAIT_TOUT_VALUE		1*SECOND
#define CONNECT_VIDEO_DECODER_TOUT_VALUE			20*SECOND
#define UPDATE_ART_WITH_SSRC_TOUT_VALUE				20*SECOND
#define DISCONNECT_TOUT_VALUE						20*SECOND
#define MRM_STREAM_IS_MUST_TOUT_VALUE				5*SECOND

// defined to be replaced
#define UPDATE_AVC_IN_CONF						((WORD)4000)
#define UPDATE_SSRC								((WORD)4006)

#define OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT	((WORD)4001)
#define DECODER_SYNC_FAILURE_WAIT_TOUT			((WORD)4003)
#define CONNECT_VIDEO_DECODER_TOUT				((WORD)4004)
#define UPDATE_ART_WITH_SSRC_TOUT				((WORD)4005)
#define DISCONNECT_TOUT							((WORD)4007)
#define MRM_STREAM_IS_MUST_ACK_TOUT				((WORD)4008)


#define MY_TRACEINTO2(level)   if (!(CIsLoggerToSend().IsLoggerLevelNotToSend(level))) void(0); else TRACESTRFUNC(level)
//TICKS gltimeOpen = 0;



// ACtion Function table
PBEGIN_MESSAGE_MAP(CSVCToAVCTranslator)

	ONEVENT(CONNECT_VIDEO_IN,			IDLE,               CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectIDLE)
	ONEVENT(CONNECT_VIDEO_IN,			SETUP,              CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectSETUP)
	ONEVENT(CONNECT_VIDEO_IN,			CONNECTED_WITHOUT_AVC,	CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectCONNECTED_WITHOUT_AVC)
	ONEVENT(CONNECT_VIDEO_IN,			CONNECTED,          CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectCONNECTED)
	ONEVENT(CONNECT_VIDEO_IN,			DISCONNECTING,      CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectDISCONNECTING)

	ONEVENT(UPDATE_AVC_IN_CONF,			IDLE,				CSVCToAVCTranslator::OnUpdateAVCInConfIDLE)
	ONEVENT(UPDATE_AVC_IN_CONF,			SETUP,				CSVCToAVCTranslator::OnUpdateAVCInConfSETUP)
	ONEVENT(UPDATE_AVC_IN_CONF,			CONNECTED_WITHOUT_AVC,	CSVCToAVCTranslator::OnUpdateAVCInConfCONNECTED_WITHOUT_AVC)
	ONEVENT(UPDATE_AVC_IN_CONF,			CONNECTED,			CSVCToAVCTranslator::OnUpdateAVCInConfCONNECTED)
	ONEVENT(UPDATE_AVC_IN_CONF,			DISCONNECTING,		CSVCToAVCTranslator::OnUpdateAVCInConfDISCONNECTING)

	ONEVENT(DISCONNECT_VIDEO_IN,		IDLE,               CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectIDLE)
	ONEVENT(DISCONNECT_VIDEO_IN,		SETUP,              CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectSETUP)
	ONEVENT(DISCONNECT_VIDEO_IN,		CONNECTED_WITHOUT_AVC,	CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectCONNECTED_WITHOUT_AVC)
	ONEVENT(DISCONNECT_VIDEO_IN,		CONNECTED,          CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectCONNECTED)
	ONEVENT(DISCONNECT_VIDEO_IN,		DISCONNECTING,      CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectDISCONNECTING)

	ONEVENT(UPDATE_SSRC,				SETUP,							CSVCToAVCTranslator::OnUpdateSsrcSETUP)
	ONEVENT(UPDATE_SSRC,				CONNECTED,						CSVCToAVCTranslator::OnUpdateSsrcCONNECTED)
	ONEVENT(UPDATE_SSRC,				ANYCASE,						CSVCToAVCTranslator::OnUpdateSsrcANYCASE)

	ONEVENT(ACK_IND,					IDLE,				CSVCToAVCTranslator::OnSVCToAVCTranslatorAckIDLE)
	ONEVENT(ACK_IND,					SETUP,				CSVCToAVCTranslator::OnSVCToAVCTranslatorAckSETUP)
	ONEVENT(ACK_IND,					CONNECTED_WITHOUT_AVC,	CSVCToAVCTranslator::OnSVCToAVCTranslatorAckCONNECTED_WITHOUT_AVC)
	ONEVENT(ACK_IND,					CONNECTED,			CSVCToAVCTranslator::OnSVCToAVCTranslatorAckCONNECTED)
	ONEVENT(ACK_IND,					DISCONNECTING,		CSVCToAVCTranslator::OnSVCToAVCTranslatorAckDISCONNECTING)

	ONEVENT(VIDEO_DECODER_SYNC_IND,		IDLE,				CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncIDLE)
	ONEVENT(VIDEO_DECODER_SYNC_IND,		SETUP,				CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncSETUP)
	ONEVENT(VIDEO_DECODER_SYNC_IND,		CONNECTED_WITHOUT_AVC,	CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncCONNECTED_WITHOUT_AVC)
	ONEVENT(VIDEO_DECODER_SYNC_IND,		CONNECTED,			CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncCONNECTED)
	ONEVENT(VIDEO_DECODER_SYNC_IND,		DISCONNECTING,		CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncDISCONNECTING)

	ONEVENT(UPDATE_ART_WITH_SSRC_ACK,	SETUP,							CSVCToAVCTranslator::OnUpdateArtSsrcAckSETUP)
	ONEVENT(UPDATE_ART_WITH_SSRC_ACK,	CONNECTED_WITHOUT_AVC,			CSVCToAVCTranslator::OnUpdateArtSsrcAckCONNECTED_WITHOUT_AVC)
	ONEVENT(UPDATE_ART_WITH_SSRC_ACK,	ANYCASE,						CSVCToAVCTranslator::OnUpdateArtSsrcAckANYCASE)

	ONEVENT(SEND_MRMP_STREAM_IS_MUST_ACK,	SETUP,							CSVCToAVCTranslator::OnUpdateMrmpStreamIsMustAckSETUP)
	ONEVENT(SEND_MRMP_STREAM_IS_MUST_ACK,	CONNECTED,						CSVCToAVCTranslator::OnUpdateMrmpStreamIsMustAckCONNECTED)
	ONEVENT(SEND_MRMP_STREAM_IS_MUST_ACK,	CONNECTED_WITHOUT_AVC,			CSVCToAVCTranslator::OnUpdateMrmpStreamIsMustAckCONNECTED_WITHOUT_AVC)
	ONEVENT(SEND_MRMP_STREAM_IS_MUST_ACK,	ANYCASE,						CSVCToAVCTranslator::OnUpdateMrmpStreamIsMustAckANYCASE)

	ONEVENT(OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT,	IDLE,				CSVCToAVCTranslator::OnTimerOpenConnectDecoderIDLE)
	ONEVENT(OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT,	SETUP,				CSVCToAVCTranslator::OnTimerOpenConnectDecoderSETUP)
	ONEVENT(OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT,	CONNECTED_WITHOUT_AVC,	CSVCToAVCTranslator::OnTimerOpenConnectDecoderCONNECTED_WITHOUT_AVC)
	ONEVENT(OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT,	CONNECTED,			CSVCToAVCTranslator::OnTimerOpenConnectDecoderCONNECTED)
	ONEVENT(OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT,	DISCONNECTING,		CSVCToAVCTranslator::OnTimerOpenConnectDecoderDISCONNECTING)

	ONEVENT(CONNECT_VIDEO_DECODER_TOUT,	SETUP,				CSVCToAVCTranslator::OnTimerConnectDecoderSETUP)
	ONEVENT(CONNECT_VIDEO_DECODER_TOUT,	ANYCASE,			CSVCToAVCTranslator::NullActionFunction)

	ONEVENT(UPDATE_ART_WITH_SSRC_TOUT,	CONNECTED_WITHOUT_AVC,			CSVCToAVCTranslator::OnTimerUpdateSsrcCONNECTED_WITHOUT_AVC)
	ONEVENT(UPDATE_ART_WITH_SSRC_TOUT,	ANYCASE,						CSVCToAVCTranslator::OnTimerUpdateSsrcANYCASE)

	ONEVENT(DISCONNECT_TOUT,			DISCONNECTING,		CSVCToAVCTranslator::OnTimerDisconnectDISCONNECTING)
	ONEVENT(DISCONNECT_TOUT,			ANYCASE,			CSVCToAVCTranslator::NullActionFunction)

	ONEVENT(VIDEO_DECODER_SYNC_TOUT,	IDLE,				CSVCToAVCTranslator::OnTimerDecoderSyncIDLE)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,	SETUP,				CSVCToAVCTranslator::OnTimerDecoderSyncSETUP)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,	CONNECTED_WITHOUT_AVC,	CSVCToAVCTranslator::OnTimerDecoderSyncCONNECTED_WITHOUT_AVC)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,	CONNECTED,			CSVCToAVCTranslator::OnTimerDecoderSyncCONNECTED)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,	DISCONNECTING,		CSVCToAVCTranslator::OnTimerDecoderSyncDISCONNECTING)

	ONEVENT(DECODER_SYNC_FAILURE_WAIT_TOUT,			CONNECTED,			CSVCToAVCTranslator::OnTimerDecoderSyncFailureWaitCONNECTED)
	ONEVENT(DECODER_SYNC_FAILURE_WAIT_TOUT,			ANYCASE,			CSVCToAVCTranslator::OnTimerDecoderSyncFailureWaitANYCASE)

	ONEVENT(MRM_STREAM_IS_MUST_ACK_TOUT,			ANYCASE,			CSVCToAVCTranslator::OnTimerStreamIsMustANYCASE)



PEND_MESSAGE_MAP(CSVCToAVCTranslator,CStateMachine);



// ------------------------------------------------------------
CSVCToAVCTranslator::CSVCToAVCTranslator ()
{
	m_pBridgePartyCntl			= NULL;
	m_pBridgePartyVideoRelayIn  = NULL;
	m_pHardwareInterfaceDecoder = NULL;
	m_closePortAckStatus    	= STATUS_OK;	// we need to save the status from the Ack to the close port request.
	m_ackOpenDecoderRecieved 	= FALSE;
	m_ackConnectDecoderRecieved = FALSE;
	m_ackUpdateArtSsrcRecieved	= FALSE;
	m_ackUpdateMrmpStreamIsMust = FALSE;
	m_ackCloseDecoderRecieved	= FALSE;
	m_ackCloseOutChannelRecieved= FALSE;
	m_channelHandle				= 0;
	m_closeStatus				= STATUS_OK;
	m_bDecoderSynced			= FALSE;
	m_lastVideoStreamsReq		= eEmptyStreams;
	m_dDebugIntraCounter		= 0;
	m_dDebugDecoderSyncFailureCounter		= 0;
	m_dDebugDecoderSyncFailureCounterAcc	= 0;
	m_lastReqSSRC				= 0xAEAEAEAE;	// just a dummy number
	m_lastIsMustSsrc			= INVALID;
	m_connectionStatus			= STATUS_OK;
	m_fs						= 0;
	m_mbps						= 0;
	m_partyRsrcId				= 0;
	m_lastReqId					= 0xFFFFFFFF;
	m_lastReq					= 0xFFFFFFFF;
}


// ------------------------------------------------------------
CSVCToAVCTranslator::~CSVCToAVCTranslator ()
{
	POBJDELETE(m_pHardwareInterfaceDecoder);
}


// ------------------------------------------------------------
void  CSVCToAVCTranslator::HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	  DispatchEvent(opCode,pMsg);
}

// ------------------------------------------------------------
void*  CSVCToAVCTranslator::GetMessageMap()
{
  return (void*)m_msgEntries;
}

ConnectionID CSVCToAVCTranslator::GetConnectionId()
{
	if (m_pHardwareInterfaceDecoder)
		return m_pHardwareInterfaceDecoder->GetConnectionId();
	return (ConnectionID)DUMMY_CONNECTION_ID;
}

// ------------------------------------------------------------
void CSVCToAVCTranslator::Create (const CBridgePartyCntl* pBridgePartyCntl, const CBridgePartyVideoRelayIn* pBridgePartyVideoRelayIn, const CRsrcParams* pRsrcParamsVideo)
{
	m_pBridgePartyCntl = (CBridgePartyCntl*)pBridgePartyCntl;
	m_pBridgePartyVideoRelayIn = (CBridgePartyVideoRelayIn*)pBridgePartyVideoRelayIn;

	// create HW I/F Decoder
	if (NULL == m_pHardwareInterfaceDecoder)
		m_pHardwareInterfaceDecoder = new CVideoHardwareInterface;
	m_pHardwareInterfaceDecoder->Create((CRsrcParams*)pRsrcParamsVideo);

	m_partyRsrcId = m_pHardwareInterfaceDecoder->GetPartyRsrcId();


	//MyTest();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::Connect()
{
	DispatchEvent(CONNECT_VIDEO_IN, NULL);
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::Disconnect()
{
	DispatchEvent(DISCONNECT_VIDEO_IN, NULL);
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::UpdateAvcImageInConf()
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::UpdateAvcImageInConf - AVC enter-exit to Conf - need to update VideoStream)");

	DispatchEvent(UPDATE_AVC_IN_CONF, NULL);
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::UpdateArtOnTranslateVideoSSRCAck( DWORD status )
{
	CSegment seg;
	seg << status;
	DispatchEvent(UPDATE_ART_WITH_SSRC_ACK, &seg);
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::UpdateVideoRelaySsrc()
{
	DispatchEvent(UPDATE_SSRC, NULL);
}

void  CSVCToAVCTranslator::UpdateMrmpStreamIsMustAck( DWORD status )
{
	CSegment seg;
	seg << status;
	DispatchEvent(SEND_MRMP_STREAM_IS_MUST_ACK, &seg);
}
//
///############################ calls from outside end ###########################################


// ------------------------------------------------------------
void  CSVCToAVCTranslator::NullActionFunction( CSegment* pParam )
{
	TRACEINTO << " - SVC-Translator-Error - (unexpected state, probably an error), Party Name: "<< GetMyFullName() << " , PartyRsrcId: " << m_partyRsrcId;
}

// ------------------------------------------------------------
int  CSVCToAVCTranslator::SendOpenDecoder()
{
/*	/// test -------
	if (0 == gltimeOpen.GetIntegerPartForTrace()) {
		gltimeOpen = SystemGetTickCount();
		TRACEINTO << " First Amir Ticks Test: " << (DWORD)gltimeOpen.GetIntegerPartForTrace();
	}*/


	if(!m_pHardwareInterfaceDecoder)
	{
		DBGPASSERT(101);
		return STATUS_FAIL;
	}

	stOpenDecoderParams od;
	FillOpenDecoderParameters( &od );

	m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterfaceDecoder)->SendOpenDecoder(
			od.videoAlg,
			od.videoBitRate,
			od.videoResolution,
			od.videoQCifFrameRate,
			od.videoCifFrameRate,
			od.video4CifFrameRate,
			od.mbps,
			od.fs,
			od.sampleAspectRatio,
			od.staticMB,
			od.isVSW,
			od.backgroundImageID,
			od.isVideoClarityEnabled,
			od.videoConfType,
			od.dpb,
			od.parsingMode,
			od.eTelePresenceMode,
			od.isAutoBrightness,
			od.videoVGAFrameRate,
			od.videoSVGAFrameRate,
			od.videoXGAFrameRate,
			od.decoderType,
			od.resolutionFrameRate,
			od.profile,
			od.packetPayloadFormat,
			od.bIsTipMode,
			od.bIsCallGenerator);
	m_lastReq = TB_MSG_OPEN_PORT_REQ;

	return STATUS_OK;
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::FillOpenDecoderParameters( stOpenDecoderParams *od)
{
	DWORD videoBitRate=0, mbps=0, fs=0;
	FindSvcDecoderParams( videoBitRate, mbps, fs );
	m_fs	= (WORD)fs;
	m_mbps	= (WORD)mbps;

	DWORD dpb = CH264Details::GetMaxDpbFromMaxFs(fs * 256);
	od->videoAlg 			= SVC;
	od->videoBitRate 		= videoBitRate;					// xxx Call rate (Erez)(Keren: 768 for now, to be corrected (get from conf)
	od->videoResolution 	= eVideoResolutionDummy;
	od->videoQCifFrameRate 	= eVideoFrameRateDUMMY;
	od->videoCifFrameRate 	= eVideoFrameRateDUMMY;
	od->video4CifFrameRate 	= eVideoFrameRateDUMMY;
	od->mbps 				= mbps;
	od->fs 					= fs;
	od->sampleAspectRatio 	= DEFAULT_SAMPLE_ASPECT_RATIO;
	od->staticMB 			= DEFAULT_STATIC_MB;
	od->isVSW 				= NO;
	od->backgroundImageID 	= 1;						// xxx temporary set to 1 ;(Erez:old skin (not used by VMP decoder)) ()
	od->isVideoClarityEnabled = FALSE;
	od->videoConfType 		= eVideoConfTypeCP;
	od->dpb 				= dpb;						// fixed for BRIDGE-5989
	od->parsingMode 		= E_PARSING_MODE_CP;
	od->eTelePresenceMode 	= eTelePresencePartyNone;
	od->isAutoBrightness 	= FALSE;
	od->videoVGAFrameRate 	= eVideoFrameRateDUMMY;
	od->videoSVGAFrameRate 	= eVideoFrameRateDUMMY;
	od->videoXGAFrameRate 	= eVideoFrameRateDUMMY;
	od->decoderType 		= E_VIDEO_DECODER_NORMAL;
	od->resolutionFrameRate = eVideoFrameRateDUMMY;
	od->profile 			= eVideoProfileHigh;
	od->packetPayloadFormat = eVideoPacketPayloadFormatSingleUnit;	// should be the value for normal packetization (no fragmentation) (Erez)
	od->bIsTipMode 			= FALSE;
	od->bIsCallGenerator    = m_pBridgePartyCntl->GetIsCallGeneratorConference();

}
// ------------------------------------------------------------
void  CSVCToAVCTranslator::FindSvcDecoderParams( DWORD &videoBitRate, DWORD &mbps, DWORD &fs )
{
	// set default params
	videoBitRate 		= 768000;
	mbps 				= H264_HD720_30_MBPS;
	fs 					= H264_HD720_FS;

	// get operationPoints
	CSvcToAvcParams* pSvcToAvcParams = (CSvcToAvcParams*)m_pBridgePartyVideoRelayIn->GetpSvcToAvcParams();;
	if (!pSvcToAvcParams)
		{PASSERT_AND_RETURN(101);}

	CVideoOperationPointsSet* videoOpPointsSet = pSvcToAvcParams->GetVideoOperationPointsSet();
	if (!videoOpPointsSet)
		{PASSERT_AND_RETURN(101);}

	VideoOperationPoint sVideoOperationPoint;
	bool bResult = videoOpPointsSet->GetOperationPointFromList( /*pSvcToAvcParams->GetMaxAllowedLayerId()*/ pSvcToAvcParams->GetAllowedLayerId(), sVideoOperationPoint );
	if (!bResult)
		{PASSERT_AND_RETURN(101);}

	// assign the parameters
	videoBitRate = sVideoOperationPoint.m_maxBitRate * 1000;
	DWORD tSize	 = (sVideoOperationPoint.m_frameWidth * sVideoOperationPoint.m_frameHeight) / 256;
//	fs 			 = tSize / CUSTOM_MAX_FS_FACTOR;
//	mbps 		 = (tSize *  (sVideoOperationPoint.m_frameRate/256)) / CUSTOM_MAX_MBPS_FACTOR;
	fs           = (DWORD)GetMaxFsAsDevision( tSize );
	mbps         = (DWORD)GetMaxMbpsAsDevision(tSize *  (sVideoOperationPoint.m_frameRate/256));

	//TRACEINTO << "CSVCToAVCTranslator::FindSvcDecoderParams - qqqAmir  fsw = "<< sVideoOperationPoint.m_frameWidth << " fsh = "<< sVideoOperationPoint.m_frameHeight << " Max BitRate = " << (DWORD)sVideoOperationPoint.m_maxBitRate ;
	TRACEINTO << " \n fs = "<< fs << "\n mbps = " << mbps
			  << " \n videoBitRate = " << videoBitRate
			  << " \n FrameWidth = " << (DWORD)sVideoOperationPoint.m_frameWidth
			  << " \n FrameHeight = " << (DWORD)sVideoOperationPoint.m_frameHeight
			  << " \n layerID = " << (DWORD)sVideoOperationPoint.m_layerId << " \n Name: " << GetMyFullName();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectIDLE( CSegment* pParam )
{
	// changing the state
	TRACEINTO << " (state changed to SETUP)" << " Part Name = " << GetMyFullName();
	m_state = SETUP;

	m_connectionStatus = STATUS_OK;

	// Open Decoder
	int status = SendOpenDecoder();
	if (STATUS_OK == status)
		status = UpdateArtWIthSSRC( 0 );

	// start Open Decoder and Update ART with SSRC Timer
	if (STATUS_OK == status)
		StartTimer( OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT, OPEN_VIDEO_DECODER_SSRC_ART_TOUT_VALUE );
	else
	{

		TRACEINTO << " - SVC-Translator-Error , Party Name = " << GetMyFullName();
		InformConnectEnded( statVideoInOutResourceProblem );	// stays in SETUP state
	}
}


// ------------------------------------------------------------
int  CSVCToAVCTranslator::UpdateArtWIthSSRC( bool bStartIsMustTimer )
{
	// get SSRC
	if (!m_pBridgePartyVideoRelayIn)
	{
		DBGPASSERT(101);
		return STATUS_FAIL;
	}
	CSvcToAvcParams* pSvcToAvcParams = (CSvcToAvcParams*)m_pBridgePartyVideoRelayIn->GetpSvcToAvcParams();;
	if (!pSvcToAvcParams)
	{
		DBGPASSERT(102);
		TRACEINTO << " - SVC-Translator-Error, no Params, Party Name = " << GetMyFullName();
		return STATUS_FAIL;
	}

	// for Must to have
	CImage* pImage = (CImage*)m_pBridgePartyVideoRelayIn->GetPartyImage();
	if (NULL == pImage)
	{
		DBGPASSERT(103);
		TRACEINTO << " - SVC-Translator-Error, no image, Party Name = " << GetMyFullName();
		return STATUS_FAIL;
	}

	DWORD videoRelaySsrc = INVALID;
	CVideoRelayMediaStream *pVideoRelayMediaStreams = pSvcToAvcParams->GetVideoRelayMediaStream();
	if (pVideoRelayMediaStreams)
		videoRelaySsrc = pVideoRelayMediaStreams->GetSsrc();


	DWORD curSsrcToSend = (pSvcToAvcParams->GetIsAvcInConf()) ? videoRelaySsrc : INVALID;

	if (m_lastReqSSRC != curSsrcToSend)
	{
		TRACEINTO << " Sends SSRC to ART - SSRC: " << (DWORD)curSsrcToSend << ", Party Name: " << GetMyFullName() << " , PartyRsrcId: " << m_partyRsrcId;
		m_pBridgePartyVideoRelayIn->UpdateArtOnTranslateVideoSSRC( curSsrcToSend );
		m_lastReqSSRC = curSsrcToSend;

		if (videoRelaySsrc != INVALID)
		{
			BOOL bIsMustSsrc = (INVALID == curSsrcToSend) ? FALSE : TRUE;
			DWORD channelRelayInHandle = pImage->GetVideoRelayInChannelHandle();
			TRACEINTO << " Stream Is Must sent, SSRC: " << videoRelaySsrc << ", bIsMust: " << (DWORD)bIsMustSsrc << ", Channel: " << channelRelayInHandle << " , PartyRsrcId: " << m_partyRsrcId;
			m_pBridgePartyVideoRelayIn->UpdateMrmpStreamIsMust( videoRelaySsrc, channelRelayInHandle, bIsMustSsrc );
			if (bStartIsMustTimer)
				StartTimer( MRM_STREAM_IS_MUST_ACK_TOUT, MRM_STREAM_IS_MUST_TOUT_VALUE );
			if ((m_lastIsMustSsrc != INVALID) && (m_lastIsMustSsrc != videoRelaySsrc))	// just in case there was "UpdateSSRC", need to cancel the MUST from the old one
			{// happens in "update SSRC", need to "release" the old SSRC
				TRACEINTO << " Old last Stream Is Must, SSRC: " << m_lastIsMustSsrc << ", bIsMust: FALSE, Channel: " << channelRelayInHandle << " , PartyRsrcId: " << m_partyRsrcId;
				m_pBridgePartyVideoRelayIn->UpdateMrmpStreamIsMust( m_lastIsMustSsrc, channelRelayInHandle, FALSE );	// setting the old SSRC to "not must"
			}
			m_lastIsMustSsrc = videoRelaySsrc;
		}
		else
		{
			TRACEINTO << " SVC-Translator-Error, Not sent SSRC to 'Stream Is Must', ssrc==INVALID! PartyRsrcId: " << m_partyRsrcId;
		}
	}
	else
	{
		TRACEINTO << " No Need to send Update SSRC, last is the same as current, curSsrcToSend=" << curSsrcToSend << ", PartyRsrcId: " << m_partyRsrcId;
	}

	return STATUS_OK;
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectSETUP( CSegment* pParam )
{
	TRACEINTO << " - CONNECT_VIDEO_IN while in connecting process, nothing to do, Party Name: " << GetMyFullName();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectCONNECTED( CSegment* pParam )
{
	TRACEINTO << " - CONNECT_VIDEO_IN while already connected, Party Name: " << GetMyFullName();

	InformConnectEnded( statOK );
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectDISCONNECTING( CSegment* pParam )
{
	TRACEINTO << " - SVC-Translator-Error: CONNECT_VIDEO_IN: while in disconnecting process, Party NAme: " << GetMyFullName();
}

void  CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorConnectCONNECTED_WITHOUT_AVC - (CONNECT_VIDEO_IN: unexpected state (CONNECTED_WITHOUT_AVC) for this opcode!!) ");

	InformConnectEnded( statOK );
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorAckSETUP( CSegment* pParam )
{
 /* 	TICKS timeNow = SystemGetTickCount();
  	TICKS temp = timeNow - gltimeOpen;
    DWORD difference = temp.GetIntegerPartForTrace() / 100;
   	TRACEINTO << " Amir Ticks Test, open: " << gltimeOpen.GetIntegerPartForTrace() << ", now: " << timeNow.GetIntegerPartForTrace() << ", diff: " << difference;
*/

	ACK_IND_S sAckIndStruct;
	*pParam >> sAckIndStruct.ack_base.ack_opcode
			>> sAckIndStruct.ack_base.ack_seq_num
			>> sAckIndStruct.ack_base.status
			>> sAckIndStruct.ack_base.reason
			>> sAckIndStruct.media_type
			>> sAckIndStruct.media_direction
			>> sAckIndStruct.channelHandle;

	OPCODE	AckOpcode = sAckIndStruct.ack_base.ack_opcode;
	STATUS  status = sAckIndStruct.ack_base.status;

	switch( AckOpcode ) {
	case TB_MSG_OPEN_PORT_REQ:{
		OnMplOpenPortAck(status);
		break;
		}
	case TB_MSG_CONNECT_REQ:
		{
		OnMplConnectDecoderAck(status);
		break;
		}

	default:{
		CProcessBase * process = CProcessBase::GetProcess();
		std::string str = process->GetOpcodeAsString(AckOpcode);
		TRACEINTO << " - ACK_IND Ignored! - SVC-Translator-Error, Ack Opcode: "<< str.c_str() << " - Party Name: " << GetMyFullName();
		}
	}// end switch
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorAckIDLE( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorAckIDLE - (ACK_IND: unexpected state (IDLE) for this opcode!!) ");
}
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorAckCONNECTED( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorAckCONNECTED - (ACK_IND: unexpected state (CONNECTED) for this opcode!!) ");
}

void  CSVCToAVCTranslator::OnSVCToAVCTranslatorAckCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorAckCONNECTED_WITHOUT_AVC - (ACK_IND: unexpected state (CONNECTED_WITHOUT_AVC) for this opcode!!) ");
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorAckDISCONNECTING( CSegment* pParam )
{
	OPCODE	AckOpcode;
	DWORD  ack_seq_num;
	STATUS  status;

	*pParam >> AckOpcode >> ack_seq_num >> status;

	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 5,1005)	// simulate Ack error from Close Decoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 6)				// simulate no Ack received	on Close Decoder

	switch(AckOpcode){

	case	TB_MSG_CLOSE_PORT_REQ:
		{
			SetAckOpenDecoder( FALSE );	// set Open Decoder Flag to FALSE
			if (STATUS_OK != status)
			{
				TRACEINTO << " - SVC-Translator-Error , TB_MSG_CLOSE_PORT_REQ, Party Name = " << GetMyFullName();
				m_closeStatus = status;
			}
			else
			{
				TRACEINTO << " TB_MSG_CLOSE_PORT_REQ - OK ";
			}
			break;
		}

	default:{
		CProcessBase * process = CProcessBase::GetProcess();
		std::string str = process->GetOpcodeAsString(AckOpcode);
		TRACEINTO << " - ACK_IND Ignored! - SVC-Translator-Error, Ack Opcode: "<< str.c_str() << " - Party Name: " << GetMyFullName();
		return;
			}
	}// end switch

	//	if (IsAllClosed())
	if (FALSE == m_ackOpenDecoderRecieved)	// the decoder is closed, we don't wait for the Ack from ART on update SSRC
	{
		// Stop Failure Timer if exists
		DeleteTimer( DISCONNECT_TOUT );

		// changing the state
		m_state = IDLE;

		TRACEINTO << " (state changed to IDLE)" << " Part Name = " << GetMyFullName();

		// last command as it leads to destroy this class!
		if (STATUS_OK != status)
			InformDisconnectEnded( statVideoInOutResourceProblem );
		else
			InformDisconnectEnded( statOK );
	}
}

//void  CSVCToAVCTranslator::TranslatorEnded()
//{
//	if(m_pBridgePartyVideoRelayIn)
//		m_pBridgePartyVideoRelayIn->SvcToAvcTranslatorEnded();
//}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerFailureIDLE( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerFailureIDLE (received Timer in state IDLE, probably an error)");
}
void  CSVCToAVCTranslator::OnTimerFailureSETUP( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerFailureIDLE (received Timer in state SETUP, probably an error)");
}
/*
void  CSVCToAVCTranslator::OnTimerFailureCHANGE_STREAMS( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerFailureCHANGE_STREAMS (received Timer in state CHANGE_STREAMS, probably an error)");
}*/
void  CSVCToAVCTranslator::OnTimerFailureCONNECTED( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerFailureCONNECTED (received Timer in state CONNECTED, probably an error)");
}
void  CSVCToAVCTranslator::OnTimerFailureCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerFailureCONNECTED_WITHOUT_AVC (received Timer in state CONNECTED_WITHOUT_AVC, probably an error)");
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerFailureDISCONNECTING( CSegment* pParam )
{
	if (STATUS_OK == m_closeStatus)
		m_closeStatus = (STATUS)statTout;

	// changing the state
	m_state = IDLE;

	TRACEINTO << " SVC-Translator-Error - (state changed to IDLE)" << " Part Name = " << GetMyFullName();

	//TranslatorEnded();

	// this will cause the parent to destroy this class!!
	if(m_pBridgePartyVideoRelayIn)
		m_pBridgePartyVideoRelayIn->SvcToAvcTranslatorDisconnected( (EStat) m_closeStatus );
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerOpenConnectDecoderIDLE( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerOpenConnectDecoderIDLE - expected to get it on SETUP state, probably handled, ignored ");
}
void  CSVCToAVCTranslator::OnTimerOpenConnectDecoderCONNECTED( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerOpenConnectDecoderCONNECTED - expected to get it on SETUP state, probably handled, ignored ");
}
void  CSVCToAVCTranslator::OnTimerOpenConnectDecoderDISCONNECTING( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerOpenConnectDecoderDISCONNECTING - expected to get it on SETUP state, probably handled, ignored ");
}
void  CSVCToAVCTranslator::OnTimerOpenConnectDecoderCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimervCONNECTED_WITHOUT_AVC - expected to get it on SETUP state, probably handled, ignored ");
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerOpenConnectDecoderSETUP( CSegment* pParam )
{
	CMedString str = "CSVCToAVCTranslator::OnTimerOpenConnectDecoderSETUP - SVC-Translator-Error - ";
	str << " Party Name: " << GetMyFullName();
	if (FALSE == m_ackOpenDecoderRecieved)
		str << "\n - Not recieved Ack from Open Decoder";
	if (FALSE == m_ackUpdateArtSsrcRecieved)
		str << "\n - Not recieved Ack from UpdateArtWIthSSRC";
	if (FALSE == m_ackUpdateMrmpStreamIsMust)
		str << "\n - Not recieved Ack from UpdateStreamIsMust";
	PTRACE(eLevelInfoNormal, str.GetString());


	// updating last request in case we got the Open Decoder Ack
	if (TRUE == m_ackOpenDecoderRecieved)
	{
		m_lastReqId = 0;	// we don't have the last request id of Update Art with SSRC or of IsMust
		if (FALSE == m_ackUpdateArtSsrcRecieved)
			m_lastReq = ART_UPDATE_RELAY_PARAMS_REQ;		// update Art with SSRC error, no Ack
		else
			m_lastReq = CONF_PARTY_MRMP_STREAM_IS_MUST_REQ;	// Is Must error, no Ack
	}

	// Inform BridgePartyVideoRelayIn on connection error
	InformConnectEnded( statVideoInOutResourceProblem );	// stays in SETUP state

}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncCONNECTED( CSegment* pParam )
{
	//PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncCONNECTED ");

	STATUS  status = STATUS_FAIL;
	DWORD  decoderDetectedModeWidth 				= DEFAULT_DECODER_DETECTED_MODE_WIDTH;
	DWORD  decoderDetectedModeHeight 				= DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
	DWORD  decoderDetectedSampleAspectRatioWidth 	= DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH;
	DWORD  decoderDetectedSampleAspectRatioHeight 	= DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT;

	*pParam >>  decoderDetectedModeWidth >> decoderDetectedModeHeight >> decoderDetectedSampleAspectRatioWidth >> decoderDetectedSampleAspectRatioHeight >> status;

	if(status != STATUS_OK)
	{
		m_dDebugDecoderSyncFailureCounter++;
		m_dDebugDecoderSyncFailureCounterAcc++;

		TRACEINTO << " - status != STATUS_OK - Name: " << GetMyFullName() << " - OutOfSync=" << m_dDebugDecoderSyncFailureCounter << " (" << m_dDebugDecoderSyncFailureCounterAcc << ")";

		// means decoder out of sync
		if (m_bDecoderSynced == TRUE)	// it was synced before this message
		{
			if(IsValidTimer( DECODER_SYNC_FAILURE_WAIT_TOUT ))
			{
				//TRACEINTO << " - Decoder sync: -----> in wait-Timer";
			}
			else
			{
				TRACEINTO << " - Decoder sync: =====> Start Wait Timer, Party Name: " << GetMyFullName();
				StartTimer( DECODER_SYNC_FAILURE_WAIT_TOUT, DECODER_SYNC_FAILURE_WAIT_TOUT_VALUE );
				if (m_pBridgePartyVideoRelayIn)	// request for Intra, only when starting the timer.
					m_pBridgePartyVideoRelayIn->SvcToAvcTranslatorAskEpForIntra();
			}
		}
		else
		{
			//TRACEINTO << " - Decoder sync: m_bDecoderSynced == FALSE ";
		}
		return;
	}


	DeleteTimer( DECODER_SYNC_FAILURE_WAIT_TOUT );
	DeleteTimer( VIDEO_DECODER_SYNC_TOUT );
	m_dDebugDecoderSyncFailureCounter=0;

	// checks if the Decoder already in sync mode (means all set from previous sync, image is set and parties already informed)
	if (m_bDecoderSynced)
	{
		// check if need to update the image parameters
		bool bParamChanged = false;

		if (0!=decoderDetectedModeWidth && 0!=decoderDetectedModeHeight)
		{
			if (!m_pBridgePartyVideoRelayIn)
				return; // shoudn't happen
			CImage* pImage = (CImage*)m_pBridgePartyVideoRelayIn->GetPartyImage();
			if (NULL != pImage)
			{
				if ((decoderDetectedModeWidth  != pImage->GetDecoderDetectedModeWidth()) ||
					(decoderDetectedModeHeight != pImage->GetDecoderDetectedModeHeight()) ||
					(decoderDetectedSampleAspectRatioWidth  != pImage->GetDecoderDetectedSampleAspectRatioWidth()) ||
					(decoderDetectedSampleAspectRatioHeight != pImage->GetDecoderDetectedSampleAspectRatioHeight()))
				{
					bParamChanged = true;
				}
			}
		}
		if (false == bParamChanged)
		{
			TRACEINTO << " - Decoder Sync status=OK, Decoder is already sync, nothing to do, Party Name: " << GetMyFullName();
			return; // nothing to do
		}


	}

	TRACEINTO << " - Decoder Sync OK, Connecting! Party Name: " << GetMyFullName();

	// Update Image parameters
	UpdateImageUponSVCtoAVCReady( decoderDetectedModeWidth, decoderDetectedModeHeight, decoderDetectedSampleAspectRatioWidth, decoderDetectedSampleAspectRatioHeight );

	// Send Update Image to all Non-Relay parties
	UpdateAllNonSVConSVCImage();

	// update that the decoder is sync, for the case it get another Decoder_Sync Ind
	// it may go (un-synced) again in the future (that situation was not treated or defined yet)
	m_bDecoderSynced = TRUE;
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::UpdateImageUponSVCtoAVCReady(DWORD decoderDetectedModeWidth,
														DWORD decoderDetectedModeHeight,
														DWORD decoderDetectedSampleAspectRatioWidth,
														DWORD decoderDetectedSampleAspectRatioHeight)
{
	if (!m_pBridgePartyVideoRelayIn)
		return; // shoudn't happen

	// get image
	CImage* pImage = (CImage*)m_pBridgePartyVideoRelayIn->GetPartyImage();
	if (NULL != pImage)
	{
		if (m_pHardwareInterfaceDecoder)
		{
			pImage->SetConnectionId( m_pHardwareInterfaceDecoder->GetConnectionId() );
		}
		TRACEINTO << " - sitename: " << pImage->GetSiteName();
		pImage->UpdateVideoParams(	H264,eVideoResolutionDummy,	m_mbps, m_fs );										//When updating the decoder we need to update it's image as well
		pImage->SetDecoderDetectedParams(decoderDetectedModeWidth, decoderDetectedModeHeight,decoderDetectedSampleAspectRatioWidth,decoderDetectedSampleAspectRatioHeight);
	}
	else{
		TRACEINTO << " SVC-Translator-Error - Image is NULL, Party Name: " << GetMyFullName();
	}
}


void  CSVCToAVCTranslator::UpdateImageUponSVCtoAVCNotReady()
{
	if (!m_pBridgePartyVideoRelayIn)
		{PASSERT_AND_RETURN(101);}

	// get image
	CImage* pImage = (CImage*)m_pBridgePartyVideoRelayIn->GetPartyImage();
	if (NULL != pImage)
	{
		PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::UpdateImageUponSVCtoAVCNotReady - INVALID ConnectionID ");
		pImage->SetConnectionId( INVALID );
	}
	else {
		TRACEINTO << " SVC-Translator-Error - Image is NULL, Party Name: " << GetMyFullName();
	}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::UpdateAllNonSVConSVCImage()
{
	// Send Update Image to all Non-Relay parties
	if (m_pBridgePartyVideoRelayIn != NULL) {
		TRACEINTO << " - Update All Non-SVC ";
		m_pBridgePartyVideoRelayIn->UpdateImageOnTranslateSvcToAvc();
	}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerDecoderSyncCONNECTED( CSegment* pParam )
{
	RequestIntraAndStartTimer( " - OnTimerDecoderSyncCONNECTED " );
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerDecoderSyncDISCONNECTING( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerDecoderSyncDISCONNECTING  - not activating timer again");
}
// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerDecoderSyncIDLE( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerDecoderSyncIDLE  - not expected - not activating timer again");
}
// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerDecoderSyncSETUP( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerDecoderSyncSETUP  - not expected - not activating timer again");
}
// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerDecoderSyncCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerDecoderSyncCONNECTED_WITHOUT_AVC  - not expected - not activating timer again");
}


// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncIDLE( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncIDLE (received Sync in state IDLE, ignored)");
}
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncSETUP( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncSETUP (received Sync in state SETUP, probably an error)");
}
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncDISCONNECTING( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncDISCONNECTING (received Sync in state DISCONNECTING, currently ignored)");
}

void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnSVCToAVCTranslatorDecoderSyncCONNECTED_WITHOUT_AVC (received Sync in state CONNECTED_WITHOUT_AVC, currently ignored)");
}


// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateAVCInConfIDLE( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnUpdateAVCInConfIDLE (received UPDATE_AVC_IN_CONF in state IDLE, ignored)");
}
void  CSVCToAVCTranslator::OnUpdateAVCInConfSETUP( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnUpdateAVCInConfSETUP (received UPDATE_AVC_IN_CONF in state SETUP)");
}
void  CSVCToAVCTranslator::OnUpdateAVCInConfDISCONNECTING( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnUpdateAVCInConfDISCONNECTING (received UPDATE_AVC_IN_CONF in state DISCONNECTING, ignored)");
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateAVCInConfCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	TRACEINTO << " First AVC Entered to conf or last leaved";
	bool isAVCInConf = IsAvcInConf();
	if (!isAVCInConf && (m_lastReqSSRC == INVALID))
	{
		TRACEINTO << "- No AVC in conf, state not expected to this action" << GetMyFullName();
		return;
	}

	// send Update ART
	UpdateArtWIthSSRC();

	// start Timer for SSRC request only
	StartTimer( UPDATE_ART_WITH_SSRC_TOUT, UPDATE_ART_WITH_SSRC_TOUT_VALUE);
}


// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateAVCInConfCONNECTED( CSegment* pParam )
{
	// get if there is AVC in conference
	bool isAVCInConf = IsAvcInConf();
	if (isAVCInConf == true)
	{
		TRACEINTO << " - AVC in conf, state not expected to this action- Name: " << GetMyFullName();
		return;
	}

	// change state
	m_state = CONNECTED_WITHOUT_AVC;

	TRACEINTO << " Last AVC disconnected, (state changed to CONNECTED_WITHOUT_AVC)" << " Part Name = " << GetMyFullName();

	// send Update ART with INVALID SSRC
	UpdateArtWIthSSRC();

	// Update Image parameters (SetConnectionId( INVALID ))
	UpdateImageUponSVCtoAVCNotReady();

	m_bDecoderSynced = FALSE;

}


// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnMplOpenPortAck(STATUS  status)
{
	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 1,1001)	// simulate Ack error from Open Decoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 2)				// simulate no Ack received	on Open

	// treats error case
	if(status != STATUS_OK)
	{
		DeleteTimer( OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT );

		InformConnectEnded( statVideoInOutResourceProblem );	// stays in SETUP state

		m_connectionStatus = STATUS_FAIL;

		TRACEINTO << " - SVC-Translator-Error - Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();
		return;
	}

	SetAckOpenDecoder( TRUE ); //sign that we got the OpenDecoder Ack OK

	if (IsAllOpenReqGotAck() && (m_connectionStatus == STATUS_OK))	// we are ready to send the Connect SVC Decoder
	{
		TRACEINTO << " - Ack OK, Next: connect SVC Decoder to ART, Party Name: " << GetMyFullName();
		OnOpenAndUpdateAck();
	}
	else
	{
		TRACEINTO << " - Ack on Open SVC Decoder is OK, Party Name: " << GetMyFullName();
	}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnMplConnectDecoderAck(STATUS  status)
{
	TRACEINTO << " - ";

	// close Connect Decoder Timer

	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 3,1003)	// simulate Ack error from Connect Decoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 4)				// simulate no Ack received	on Connect
	DeleteTimer( CONNECT_VIDEO_DECODER_TOUT );

	if (status != STATUS_OK)
	{
		InformConnectEnded( statVideoInOutResourceProblem );	// stays in SETUP state

		TRACEINTO << " - SVC-Translator-Error -  Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();

		return;
	}

	SetAckConnectDecoder( TRUE ); //sign that we got the ConnectDecoder Ack OK

	// do things upon Decoder connected OK
	OnStartTranslatorCompleted();
}


// ------------------------------------------------------------
void  CSVCToAVCTranslator::SetAckOpenDecoder( BOOL YesNo )
{
	m_ackOpenDecoderRecieved = YesNo;
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::SetAckConnectDecoder( BOOL YesNo )
{
	m_ackConnectDecoderRecieved = YesNo;
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::SetAckUpdateArtSsrc( BOOL YesNo )
{
	m_ackUpdateArtSsrcRecieved = YesNo;
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::SetAckUpdateMrmpStreamIsMust( BOOL YesNo )
{
	m_ackUpdateMrmpStreamIsMust = YesNo;
}

// ------------------------------------------------------------
BOOL  CSVCToAVCTranslator::IsAllOpenReqGotAck()
{
	return (m_ackOpenDecoderRecieved && m_ackUpdateArtSsrcRecieved && m_ackUpdateMrmpStreamIsMust);
}

// ------------------------------------------------------------
BOOL  CSVCToAVCTranslator::IsAllClosed()
{
	return (!m_ackOpenDecoderRecieved && !m_ackUpdateArtSsrcRecieved /*&& !m_ackxxxRecieved */);
}


// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnStartTranslatorCompleted()
{
	// Decoder connected OK

	// Update Bridge Party Relay In on Translator Connected (still we waiting for Decoder Sync)
	InformConnectEnded( statOK );

	bool bIsAvcInConf = IsAvcInConf();

	if (bIsAvcInConf && (INVALID != m_lastReqSSRC))	// there is at least 1 AVC party, and last SSRC Update was not INVALID
	{
		// change state
		m_state = CONNECTED;

		// request Intra for the SVC-Decoder (with repeated timer)
		RequestIntraAndStartTimer( "  OnStartTranslatorCompleted - (state changed to CONNECTED)" );
	}
	else
	{
		// change state
		m_state = CONNECTED_WITHOUT_AVC;

		if ((!bIsAvcInConf && (INVALID != m_lastReqSSRC)) ||
			(bIsAvcInConf && (INVALID == m_lastReqSSRC)))	// AVC entered during SETUP state
		{
			TRACEINTO << " need to update SSRC, (state changed to CONNECTED_WITHOUT_AVC) " << " Part Name = " << GetMyFullName();

			// send Update ART
			UpdateArtWIthSSRC();
			// start Timer for SSRC request only
			StartTimer( UPDATE_ART_WITH_SSRC_TOUT, UPDATE_ART_WITH_SSRC_TOUT_VALUE);

		}
		else
		{
			TRACEINTO << " No AVC in Conf, (state changed to CONNECTED_WITHOUT_AVC)" << " Part Name = " << GetMyFullName();
		}
	}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::RequestIntraAndStartTimer(const char* str)
{

	// we may need to limit the number of printing
	TRACEINTO << str << " , Part Name = " << GetMyFullName();

	// request intra from EP for the video Decoder stream
	if(m_pBridgePartyVideoRelayIn)
		m_pBridgePartyVideoRelayIn->SvcToAvcTranslatorAskEpForIntra();

	// start Video-Decoder-Sync-Timer (for every 1 second until VideoDecoder will be sync)
	StartTimer( VIDEO_DECODER_SYNC_TOUT, VIDEO_DECODER_SVC_SYNC_TOUT_VALUE );
}

// ------------------------------------------------------------
bool CSVCToAVCTranslator::IsAvcInConf()
{
	if (!m_pBridgePartyVideoRelayIn){
		PASSERTMSG_AND_RETURN_VALUE(101, "CSVCToAVCTranslator::IsAvcInConf - SVC-Translator-Error - Pointer error", false);
	}
	CSvcToAvcParams* pSvcToAvcParams = (CSvcToAvcParams*)m_pBridgePartyVideoRelayIn->GetpSvcToAvcParams();;
	if (!pSvcToAvcParams){
		PASSERTMSG_AND_RETURN_VALUE(101, "CSVCToAVCTranslator::IsAvcInConf - SVC-Translator-Error - Pointer error", false);
	}
	return pSvcToAvcParams->GetIsAvcInConf();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectIDLE( CSegment* pParam )
{
	TRACEINTO << " - Inform Disconnected Completed ";
	InformDisconnectEnded( statOK );
}

void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectDISCONNECTING( CSegment* pParam )
{
	TRACEINTO << " - Ignore, as already in process of Disconnecting ";
}

void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	// close all
	SendCloseAll();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectSETUP( CSegment* pParam )
{
	TRACEINTO << " - Party Name: " << GetMyFullName();

	// stop Timer (we don't know which of them is active)
	DeleteTimer( OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT );
	DeleteTimer( CONNECT_VIDEO_DECODER_TOUT );

	// close all
	SendCloseAll();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnSVCToAVCTranslatorDisconnectCONNECTED( CSegment* pParam )
{
	TRACEINTO << " - Disconnecting, Party Name: " << GetMyFullName();

	// delete timer if exists
	DeleteTimer( VIDEO_DECODER_SYNC_TOUT );
	DeleteTimer( DECODER_SYNC_FAILURE_WAIT_TOUT );

	// close all
	SendCloseAll();

	// Update Image (SetConnectionId( INVALID ))
	UpdateImageUponSVCtoAVCNotReady();

	// Send Update Image to all Non-Relay parties
	UpdateAllNonSVConSVCImage();

}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::SendCloseAll()
{
	// change the state

	TRACEINTO << " - (state changed to DISCONNECTING)" << GetMyFullName();
	m_state = DISCONNECTING;

	// closing Decoder
	int status = SendCloseDecoder();
	if (status != STATUS_OK)
	{

		TRACEINTO << " - SVC-Translator-Error - status = " << status << ", Party Name: " << GetMyFullName();
		InformDisconnectEnded( statVideoInOutResourceProblem );
		return;
	}

	if (m_pBridgePartyVideoRelayIn)
	{
		if (INVALID != m_lastReqSSRC)
		{
			m_pBridgePartyVideoRelayIn->UpdateArtOnTranslateVideoSSRC( (DWORD)INVALID );
			m_lastReqSSRC = INVALID;
		}
		else
		{
			SetAckUpdateArtSsrc( FALSE ); //sign that we set the ART to INVALID
		}
	}

	StartTimer( DISCONNECT_TOUT, DISCONNECT_TOUT_VALUE );

}

// ------------------------------------------------------------
int  CSVCToAVCTranslator::SendCloseDecoder()
{
	if(!m_pHardwareInterfaceDecoder)
	{
		DBGPASSERT(101);
		return STATUS_FAIL;
	}

	m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterfaceDecoder)->SendCloseDecoder();
	m_lastReq = TB_MSG_CLOSE_PORT_REQ;

	m_bDecoderSynced = FALSE;

	return STATUS_OK;
}

// ------------------------------------------------------------
const char*  CSVCToAVCTranslator::GetMyFullName()
{
	if (m_pBridgePartyVideoRelayIn)
	{
		CBridgePartyCntl* pBridgePartyCntlPtr = m_pBridgePartyVideoRelayIn->GetBridgePartyCntlPtr();
		if (pBridgePartyCntlPtr)
			return pBridgePartyCntlPtr->GetFullName();
	}
	return "None ";	// return constant
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerDecoderSyncFailureWaitCONNECTED( CSegment* pParam )
{
	m_dDebugIntraCounter++;	// just for statistics, if we want to know how many times the party was un-synced
	TRACEINTO << " - Party Name: " << GetMyFullName() << " - Debug-SVC-DecoderSyncFailureCounter=" << m_dDebugIntraCounter;

	if (m_pBridgePartyVideoRelayIn)
		m_pBridgePartyVideoRelayIn->SvcToAvcTranslatorAskEpForIntra();

	// according to BRIDGE-6161, we requested not to move it out of the layout but just to ask for intra
	StartTimer( DECODER_SYNC_FAILURE_WAIT_TOUT, DECODER_SYNC_FAILURE_WAIT_TOUT_VALUE );
	return;


	// sign to this object that we out of sync
	m_bDecoderSynced = FALSE;

	// Update Image (SetConnectionId( INVALID )) qqqAmir ???
	UpdateImageUponSVCtoAVCNotReady();

	// Send Update Image to all Non-Relay parties
	UpdateAllNonSVConSVCImage();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerDecoderSyncFailureWaitANYCASE( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnTimerDecoderSyncFailureWaitANYCASE - nothing to do ");
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerStreamIsMustANYCASE( CSegment* pParam )
{
	TRACEINTO << " SVC-Translator-Error - Currently do nothing with no Ack, Part Name: " << GetMyFullName();
}
// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateArtSsrcAckSETUP( CSegment* pParam )
{
	//check the Ack state
	STATUS  status;
	*pParam >> status;
	if (status != STATUS_OK)
	{
		DeleteTimer( OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT );

		TRACEINTO << " - SVC-Translator-Error, Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();

		InformConnectEnded( statVideoInOutResourceProblem );	// stays in SETUP state

		m_connectionStatus = STATUS_FAIL;

		return;
	}

	SetAckUpdateArtSsrc( TRUE ); //sign that we got the OpenDecoder Ack OK

	if (IsAllOpenReqGotAck() && (m_connectionStatus == STATUS_OK))	// we are ready to send the VideoStreamReq
	{
		TRACEINTO << " - Ack OK, Next: connect SVC Decoder to ART, Party Name: " << GetMyFullName();
		OnOpenAndUpdateAck();
	}
	else
	{
		TRACEINTO << " - Ack on UpdateSSRC is OK, Party Name: " << GetMyFullName();
	}
}


// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateArtSsrcAckCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	TRACEINTO << "  ";

	DeleteTimer( UPDATE_ART_WITH_SSRC_TOUT );


	STATUS  status;
	*pParam >> status;
	if (status != STATUS_OK)
	{
		TRACEINTO << " - SVC-Translator-Error - Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();

		return; // xxx need to decide what to do in this case!!!
	}

	if (m_lastReqSSRC == INVALID) // can happens in 2 cases, 1. only sent INVALID, 2. Sent INVALID after SSRC but got first SSRC ack
	{
		return;	// nothing to do
	}

	// 1. change state to CONNECTED
	m_state = CONNECTED;

	// 2. ask for Intra and start timer
	RequestIntraAndStartTimer( "CSVCToAVCTranslator::OnStartTranslatorCompleted - (state changed to CONNECTED)" );
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateArtSsrcAckANYCASE( CSegment* pParam )
{
	PTRACE(eLevelInfoNormal,"CSVCToAVCTranslator::OnUpdateArtSsrcAckANYCASE - nothing to do ");
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateMrmpStreamIsMustAckSETUP( CSegment* pParam )
{
	//check the Ack state
	STATUS  status;
	*pParam >> status;

	ISDEBUGMODE_SET_STATUS("TRANSLATOR", 25,1003)	// simulate Ack error from Connect Decoder
	ISDEBUGMODE_RETURN("TRANSLATOR", 26)			// simulate no Ack received	on Connect

	if (status != STATUS_OK)
	{
		DeleteTimer( OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT );

		TRACEINTO << " - SVC-Translator-Error, Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();

		InformConnectEnded( statVideoInOutResourceProblem );	// stays in SETUP state

		m_connectionStatus = STATUS_FAIL;

		return;
	}

	SetAckUpdateMrmpStreamIsMust( TRUE ); //sign that we got the OpenDecoder Ack OK

	if (IsAllOpenReqGotAck() && (m_connectionStatus == STATUS_OK))	// we are ready to send the VideoStreamReq
	{
		TRACEINTO << " - Ack OK, Next: connect SVC Decoder to ART, Party Name: " << GetMyFullName();
		OnOpenAndUpdateAck();
	}
	else
	{
		TRACEINTO << " - Ack on UpdateMrmpStreamIsMust is OK, Party Name: " << GetMyFullName();
	}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateMrmpStreamIsMustAckCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	DeleteTimer( MRM_STREAM_IS_MUST_ACK_TOUT );

	STATUS  status;
	*pParam >> status;
	if (status != STATUS_OK)
	{
		TRACEINTO << " - SVC-Translator-Error - Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();

		return; // xxx need to decide what to do in this case!!!
	}
	else
	{
		TRACEINTO << "  Got Ack on UpdateMrmpStreamIsMust - OK, Party Name: " << GetMyFullName();
	}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateMrmpStreamIsMustAckCONNECTED( CSegment* pParam )
{
	DeleteTimer( MRM_STREAM_IS_MUST_ACK_TOUT );

	STATUS  status;
	*pParam >> status;
	if (status != STATUS_OK)
	{
		TRACEINTO << " - SVC-Translator-Error - Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();

		return; // xxx need to decide what to do in this case!!!
	}
	else
	{
		TRACEINTO << " - Got Ack on UpdateMrmpStreamIsMust - OK, Party Name: " << GetMyFullName();
	}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateMrmpStreamIsMustAckANYCASE( CSegment* pParam )
{
	DeleteTimer( MRM_STREAM_IS_MUST_ACK_TOUT );

	STATUS  status;
	*pParam >> status;
	if (status != STATUS_OK)
	{
		TRACEINTO << " - SVC-Translator-Error - Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();

		return; // xxx need to decide what to do in this case!!!
	}
	else
	{
		TRACEINTO << "  Got Ack on UpdateMrmpStreamIsMust - OK, Party Name: " << GetMyFullName();
	}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnOpenAndUpdateAck()
{
	// close Open Decoder and Update ART with SSRC timer
	DeleteTimer( OPEN_VIDEO_DECODER_AND_SSRC_ART_TOUT );

	int status = ConnectDecoderToArt();
	if (STATUS_OK != status)
	{
		TRACEINTO << " - SVC-Translator-Error - Status: " << (DWORD)status << " , Party Name: " << GetMyFullName();
		InformConnectEnded( statVideoInOutResourceProblem );	// stays in SETUP state
	}
}

// ------------------------------------------------------------
int  CSVCToAVCTranslator::ConnectDecoderToArt()
{
	ConnectionID rtpConnectionId 	 = DUMMY_CONNECTION_ID;
	ConnectionID decoderConnectionId = m_pHardwareInterfaceDecoder->GetConnectionId();

	DWORD partyRsrcId = m_pHardwareInterfaceDecoder->GetPartyRsrcId();
	DWORD decoderRsrcPartyId = partyRsrcId;
	DWORD rtpRsrcPartyId 	 = partyRsrcId;

	CRsrcDesc* pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc( partyRsrcId, eLogical_rtp );
	if(pRsrcDesc)
	{
		TRACEINTO << " - Party Name: " << GetMyFullName();
		rtpConnectionId = pRsrcDesc->GetConnectionId();
		m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterfaceDecoder)->SendConnect( decoderConnectionId, rtpConnectionId, decoderRsrcPartyId, rtpRsrcPartyId );
		m_lastReq = TB_MSG_CONNECT_REQ;
	}
	else
	{
		TRACEINTO << " - SVC-Translator-Error - pRsrcDesc==NULL, Party Name: " << GetMyFullName();
		DBGPASSERT(101);	// not return as we want to fail on timer in case of error
		return STATUS_FAIL;
	}

	StartTimer( CONNECT_VIDEO_DECODER_TOUT, CONNECT_VIDEO_DECODER_TOUT_VALUE );

	return STATUS_OK;
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateSsrcSETUP( CSegment* pParam )
{
	TRACEINTO << " -  Party Name: " << GetMyFullName();

	// need to update ART with the new SSRC	xxx need to think on the correct flow. we already sent UpdateART, we may
	// get OK on the first but error on the current...
	UpdateArtWIthSSRC();
	// do not send IntraReq now as it will be send when moving from SETUP to CONNECTED (if there is AVC in the meeting)
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateSsrcCONNECTED( CSegment* pParam )
{
	TRACEINTO << " -  Party Name: " << GetMyFullName();

	// 1. need to update ART with the new SSRC
	UpdateArtWIthSSRC();

	// 2. need to send Intra Request from the SVC EP
	if (m_pBridgePartyVideoRelayIn)
		m_pBridgePartyVideoRelayIn->SvcToAvcTranslatorAskEpForIntra();

	// 3. xxx should we start timer to get Decoder Sync Ind ?
	StartTimer( VIDEO_DECODER_SYNC_TOUT, VIDEO_DECODER_SVC_SYNC_TOUT_VALUE );
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnUpdateSsrcANYCASE( CSegment* pParam )
{
	TRACEINTO << " - other states, nothing to do, Party Name: " << GetMyFullName();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerUpdateSsrcCONNECTED_WITHOUT_AVC( CSegment* pParam )
{
	TRACEINTO << " - SVC-Translator-Error - Party Name: " << GetMyFullName();
	// we didn't get Ack on updateSSRC from ART - xxx should we try again???
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerUpdateSsrcANYCASE( CSegment* pParam )
{
	TRACEINTO << " - nothing to do, Party Name: " << GetMyFullName();
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerConnectDecoderSETUP( CSegment* pParam )
{
	TRACEINTO << " - SVC-Translator-Error - Timer On Connect Decoder, Party Name: " << GetMyFullName();

	InformConnectEnded( statVideoInOutResourceProblem );	// stays in SETUP state
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::OnTimerDisconnectDISCONNECTING( CSegment* pParam )
{
	m_state = IDLE;
	TRACEINTO << " SVC-Translator-Error - (state changed to IDLE)" << " Part Name = " << GetMyFullName();

	InformDisconnectEnded( statVideoInOutResourceProblem );
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::InformConnectEnded( EStat status )
{
	if (statOK != status)
		UpdateLastReqUponError();
	if (m_pBridgePartyVideoRelayIn)
		m_pBridgePartyVideoRelayIn->SvcToAvcTranslatorConnected( status );
	else
		{PASSERT_AND_RETURN(1);}
}

// ------------------------------------------------------------
void  CSVCToAVCTranslator::InformDisconnectEnded( EStat status )
{
	if (statOK != status)
		UpdateLastReqUponError();
	if (m_pBridgePartyVideoRelayIn)
		m_pBridgePartyVideoRelayIn->SvcToAvcTranslatorDisconnected( status );	// leads to destroy this class!!
	else
		{PASSERT_AND_RETURN(1);}
}
void CSVCToAVCTranslator::UpdateLastReqUponError()
{
	if (m_pBridgePartyVideoRelayIn)
		m_pBridgePartyVideoRelayIn->UpdateLastReqUponTranslatorError( GetLastReqId(), GetLastReq() );
}

void  CSVCToAVCTranslator::MyTest()
{
	return;
//	MY_TRACEINTO2(eLevelFatal) 			<< "MY_TRACEINTO2 = This Is My eLevelFatal     private test" << "-->11111";
//	MY_TRACEINTO2(eLevelTrace) 			<< "MY_TRACEINTO2 = This Is My eLevelTrace     private test" << "-->22222";
}




/*
 things to do
=========================
  1. xxx parameters
  3. Do Scripts for testing
  4. Replace all Hard-Coded parameters with suitable values
*/


/*
/////////////////////////////////////////////////////////////////////////////
CIsLoggerToSend::CIsLoggerToSend()
{
	m_level=eLevelWarn;
}

CIsLoggerToSend&  CIsLoggerToSend::operator<<(const char* str)
{
	TRACEINTO << " - MyTest: replaced!!! ";

	return *this;
}

bool CIsLoggerToSend::IsLoggerLevelNotToSend( int level)
{
	TRACE_HEADER_S traceHeader;
	traceHeader.m_level = level;

	CProcessBase * process = CProcessBase::GetProcess();
	CFilterTraceContainer *fltContainer = process->GetTraceFilterContainer();

	bool resFilter = fltContainer->CheckFilter(traceHeader);
	if (false == resFilter)
	{
		return false;
	}

	return true;

}

*/
