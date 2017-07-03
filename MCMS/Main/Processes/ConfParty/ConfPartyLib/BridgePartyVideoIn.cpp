#include "BridgePartyVideoIn.h"
#include "BridgePartyVideoParams.h"
#include "BridgePartyVideoUniDirection.h"
#include "VideoHardwareInterface.h"
#include "TaskApi.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "HostCommonDefinitions.h"
#include "VideoBridge.h"
#include "AvcToSvcParams.h"
#include "AvcToSvcTranslator.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"

#define DECODER_UPDATE_PARAM_TOUT              ((WORD)301)
//#define AVCSVC_ENCODER_READY_WAIT_TIMER        ((WORD)302)
#define	AVCSVC_KILL_TRANSLATOR_TIMER_0		   ((WORD)303)

#define VIDEO_DECODER_SYNC_TOUT_VALUE          100
//#define AVCSVC_ENCODER_READY_WAIT_TIMER_VALUE  100

//~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
// ------------------------------------------------------------

PBEGIN_MESSAGE_MAP(CBridgePartyVideoIn)

	ONEVENT(CONNECT_VIDEO_IN,                    IDLE,          CBridgePartyVideoIn::OnVideoBridgePartyConnectIDLE)
	ONEVENT(CONNECT_VIDEO_IN,                    SETUP,         CBridgePartyVideoIn::OnVideoBridgePartyConnectSETUP)
	ONEVENT(CONNECT_VIDEO_IN,                    CONNECTED,     CBridgePartyVideoIn::OnVideoBridgePartyConnectCONNECTED)
	ONEVENT(CONNECT_VIDEO_IN,                    DISCONNECTING, CBridgePartyVideoIn::OnVideoBridgePartyConnectDISCONNECTING)

	ONEVENT(DISCONNECT_VIDEO_IN,                 IDLE,          CBridgePartyVideoIn::OnVideoBridgePartyDisConnectIDLE)
	ONEVENT(DISCONNECT_VIDEO_IN,                 SETUP,         CBridgePartyVideoIn::OnVideoBridgePartyDisConnectSETUP)
	ONEVENT(DISCONNECT_VIDEO_IN,                 CONNECTED,     CBridgePartyVideoIn::OnVideoBridgePartyDisConnectCONNECTED)
	ONEVENT(DISCONNECT_VIDEO_IN,                 DISCONNECTING, CBridgePartyVideoIn::OnVideoBridgePartyDisConnectDISCONNECTING)

	ONEVENT(ACK_IND,                             IDLE,          CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(ACK_IND,                             SETUP,         CBridgePartyVideoIn::OnMplAckSETUP)
	ONEVENT(ACK_IND,                             CONNECTED,     CBridgePartyVideoIn::OnMplAckCONNECTED)
	ONEVENT(ACK_IND,                             DISCONNECTING, CBridgePartyVideoIn::OnMplAckDISCONNECTING)

	ONEVENT(UPDATE_VIDEO_IN_PARAMS,              IDLE,          CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsIDLE)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,              SETUP,         CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsSETUP)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,              CONNECTED,     CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsCONNECTED)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,              DISCONNECTING, CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsDISCONNECTING)

	ONEVENT(VIDEO_DECODER_SYNC_IND,              IDLE,          CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(VIDEO_DECODER_SYNC_IND,              SETUP,         CBridgePartyVideoIn::OnMplDecoderSyncSETUP)
	ONEVENT(VIDEO_DECODER_SYNC_IND,              CONNECTED,     CBridgePartyVideoIn::OnMplDecoderSyncCONNECTED)
	ONEVENT(VIDEO_DECODER_SYNC_IND,              DISCONNECTING, CBridgePartyVideoIn::NullActionFunction)

	ONEVENT(VIDEO_DECODER_SYNC_TOUT,             IDLE,          CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,             SETUP,         CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,             CONNECTED,     CBridgePartyVideoIn::OnTimerDecoderSyncCONNECTED)
	ONEVENT(VIDEO_DECODER_SYNC_TOUT,             DISCONNECTING, CBridgePartyVideoIn::NullActionFunction)

	ONEVENT(DECODER_RECURRENT_INTRA_REQ_TIMEOUT, IDLE,          CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(DECODER_RECURRENT_INTRA_REQ_TIMEOUT, CONNECTED,     CBridgePartyVideoIn::OnTimerRecurrentIntraRequest)
	ONEVENT(DECODER_RECURRENT_INTRA_REQ_TIMEOUT, DISCONNECTING, CBridgePartyVideoIn::NullActionFunction)

	ONEVENT(UPDATE_VIDEO_CLARITY,                IDLE,          CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoClarityIDLE)
	ONEVENT(UPDATE_VIDEO_CLARITY,                SETUP,         CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoClaritySETUP)
	ONEVENT(UPDATE_VIDEO_CLARITY,                CONNECTED,     CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoClarityCONNECTED)
	ONEVENT(UPDATE_VIDEO_CLARITY,                DISCONNECTING, CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoClarityDISCONNECTING)

	ONEVENT(UPDATE_AUTO_BRIGHTNESS,              IDLE,          CBridgePartyVideoIn::OnVideoBridgePartyUpdateAutoBrightnessIDLE)
	ONEVENT(UPDATE_AUTO_BRIGHTNESS,              SETUP,         CBridgePartyVideoIn::OnVideoBridgePartyUpdateAutoBrightnessSETUP)
	ONEVENT(UPDATE_AUTO_BRIGHTNESS,              CONNECTED,     CBridgePartyVideoIn::OnVideoBridgePartyUpdateAutoBrightnessCONNECTED)
	ONEVENT(UPDATE_AUTO_BRIGHTNESS,              DISCONNECTING, CBridgePartyVideoIn::OnVideoBridgePartyUpdateAutoBrightnessDISCONNECTING)

	ONEVENT(DECODER_UPDATE_PARAM_TOUT,           IDLE,          CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(DECODER_UPDATE_PARAM_TOUT,           SETUP,         CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(DECODER_UPDATE_PARAM_TOUT,           CONNECTED,     CBridgePartyVideoIn::OnTimerDecoderUpdateCONNECTED)
	ONEVENT(DECODER_UPDATE_PARAM_TOUT,           DISCONNECTING, CBridgePartyVideoIn::NullActionFunction)

	ONEVENT(UPDATEVISUALEFFECTS,                 IDLE,          CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsIDLE)
	ONEVENT(UPDATEVISUALEFFECTS,                 SETUP,         CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsSETUP)
	ONEVENT(UPDATEVISUALEFFECTS,                 CONNECTED,     CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsCONNECTED)
	ONEVENT(UPDATEVISUALEFFECTS,                 DISCONNECTING, CBridgePartyVideoIn::NullActionFunction)

	ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE,      IDLE,          CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeIDLE)
	ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE,      SETUP,         CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeSETUP)
	ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE,      CONNECTED,     CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeCONNECTED)
	ONEVENT(UPDATE_PARTY_TELEPRESENCE_MODE,      DISCONNECTING, CBridgePartyVideoIn::NullActionFunction)


	ONEVENT(AVC_SVC_TRANSLATOR_CONNECTED, 		IDLE,           CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(AVC_SVC_TRANSLATOR_CONNECTED, 		SETUP,          CBridgePartyVideoIn::OnAvcToSvcTraslatorConnectedSETUP)
	ONEVENT(AVC_SVC_TRANSLATOR_CONNECTED, 		CONNECTED,      CBridgePartyVideoIn::OnAvcToSvcTraslatorConnectedCONNECTED)
	ONEVENT(AVC_SVC_TRANSLATOR_CONNECTED, 		DISCONNECTING,  CBridgePartyVideoIn::NullActionFunction)

	ONEVENT(AVC_SVC_TRANSLATOR_DISCONNECTED, 	IDLE,           CBridgePartyVideoIn::NullActionFunction)
	ONEVENT(AVC_SVC_TRANSLATOR_DISCONNECTED, 	SETUP,          CBridgePartyVideoIn::OnAvcToSvcTraslatorDisconnectedSETUP)
	ONEVENT(AVC_SVC_TRANSLATOR_DISCONNECTED, 	CONNECTED,      CBridgePartyVideoIn::OnAvcToSvcTraslatorDisconnectedCONNECTED)
	ONEVENT(AVC_SVC_TRANSLATOR_DISCONNECTED, 	DISCONNECTING,  CBridgePartyVideoIn::OnAvcToSvcTraslatorDisconnectedDISCONNECTING)

//	ONEVENT(AVCSVC_ENCODER_READY_WAIT_TIMER, 	CONNECTED,      CBridgePartyVideoIn::OnAvcToSvcEncoderReadyWaitTimerCONNECTED)
//	ONEVENT(AVCSVC_ENCODER_READY_WAIT_TIMER, 	ANYCASE,		CBridgePartyVideoIn::OnAvcToSvcEncoderReadyWaitTimerANYCASE)

	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,           	IDLE,         	CBridgePartyVideoIn::OnUpgradeToMixAvcToSvcIDLE)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,           	SETUP,         	CBridgePartyVideoIn::OnUpgradeToMixAvcToSvcSETUP)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,           	CONNECTED,     	CBridgePartyVideoIn::OnUpgradeToMixAvcToSvcCONNECTED)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC,           	DISCONNECTING, 	CBridgePartyVideoIn::OnUpgradeToMixAvcToSvcDISCONNECTING)

	ONEVENT(AVCSVC_KILL_TRANSLATOR_TIMER_0, 	ANYCASE, 		CBridgePartyVideoIn::OnAvcToSvcTraslatorKillANYCASE)



PEND_MESSAGE_MAP(CBridgePartyVideoIn,CBridgePartyMediaUniDirection);

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoIn
////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoIn::CBridgePartyVideoIn() : CBridgePartyVideoUniDirection()
{
	m_state                   = IDLE;
	m_isConnected             = FALSE;
	m_pImage                  = NULL;
	m_pDecoderSyncSegm        = NULL;
	m_eTelePresenceMode       = eTelePresencePartyNone;
	m_backgroundImageID       = 0;
	m_videoConfType           = eVideoConfTypeCP;
	m_isVsw                   = NO;
	m_last_sync_status        = (DWORD)(-1);
	m_last_sync_resolution    = eVideoResolutionDummy;
	m_waitForUpdateDecoderAck = FALSE;
	m_pAvcToSvcParams         = NULL;
	m_bDecoderClosed		  = FALSE;
	m_closeAvcSvcTranslatorStatus = statOK;

	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		m_pAvcToSvcTranslator[i] = NULL;
	m_confMediaType = 0;		// temporary until the merge, the type is not defined in this stream yet. Should be eAvcConfMediaType
	m_bIsTranslatorCreated = FALSE;
	m_pPartyVisualEffects = NULL;
	m_bNeedToReplayUpgradeAvcToSvcTranslator = false;
	m_DecoderConnected = false;
	m_bTranslatorReportedOnConnectionError = false;

 	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CBridgePartyVideoIn& CBridgePartyVideoIn::operator=(const CBridgePartyVideoIn& rOtherBridgePartyVideoIn)
{
	if (&rOtherBridgePartyVideoIn == this)
		return *this;
	CBridgePartyMediaUniDirection::operator=(rOtherBridgePartyVideoIn);
	if (NULL == rOtherBridgePartyVideoIn.m_pPartyVisualEffects)
		m_pPartyVisualEffects = NULL;
	else
	{
		m_pPartyVisualEffects = new CVisualEffectsParams();
		*m_pPartyVisualEffects = *rOtherBridgePartyVideoIn.m_pPartyVisualEffects;
	}
	m_isConnected = 			rOtherBridgePartyVideoIn.m_isConnected;
	m_pImage = 					rOtherBridgePartyVideoIn.m_pImage;
	m_pDecoderSyncSegm = 		NULL;
	m_eTelePresenceMode = 		rOtherBridgePartyVideoIn.m_eTelePresenceMode;
	m_backgroundImageID = 		rOtherBridgePartyVideoIn.m_backgroundImageID;
	m_isVsw =					rOtherBridgePartyVideoIn.m_isVsw;
	m_last_sync_status = 		rOtherBridgePartyVideoIn.m_last_sync_status;
	m_last_sync_resolution = 	rOtherBridgePartyVideoIn.m_last_sync_resolution;
	m_waitForUpdateDecoderAck = rOtherBridgePartyVideoIn.m_waitForUpdateDecoderAck;
	m_bDecoderClosed = 			rOtherBridgePartyVideoIn.m_bDecoderClosed;
	m_confMediaType = 			rOtherBridgePartyVideoIn.m_confMediaType;
	POBJDELETE( m_pAvcToSvcParams );
	if (NULL != rOtherBridgePartyVideoIn.m_pAvcToSvcParams)
		m_pAvcToSvcParams = new CAvcToSvcParams( *rOtherBridgePartyVideoIn.m_pAvcToSvcParams);
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
	{
		POBJDELETE( m_pAvcToSvcTranslator[i] );
		if (NULL != rOtherBridgePartyVideoIn.m_pAvcToSvcTranslator[i])
				m_pAvcToSvcTranslator[i] = new CAvcToSvcTranslator( *rOtherBridgePartyVideoIn.m_pAvcToSvcTranslator[i] );
	}
	m_bIsTranslatorCreated = 					rOtherBridgePartyVideoIn.m_bIsTranslatorCreated;
	m_closeAvcSvcTranslatorStatus = 			rOtherBridgePartyVideoIn.m_closeAvcSvcTranslatorStatus;
  	m_bNeedToReplayUpgradeAvcToSvcTranslator = 	rOtherBridgePartyVideoIn.m_bNeedToReplayUpgradeAvcToSvcTranslator;
  	m_DecoderConnected = 						rOtherBridgePartyVideoIn.m_DecoderConnected;
  	m_bTranslatorReportedOnConnectionError = 	rOtherBridgePartyVideoIn.m_bTranslatorReportedOnConnectionError;
	return *this;
}
void CBridgePartyVideoIn::CopyAvcToSvcTranslatorsParams( CBridgePartyVideoIn& rOtherBridgePartyVideoIn )
{
	POBJDELETE( m_pAvcToSvcParams );
	if (NULL != rOtherBridgePartyVideoIn.m_pAvcToSvcParams)
		m_pAvcToSvcParams = new CAvcToSvcParams( *rOtherBridgePartyVideoIn.m_pAvcToSvcParams);
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
	{
		POBJDELETE( m_pAvcToSvcTranslator[i] );
		if (NULL != rOtherBridgePartyVideoIn.m_pAvcToSvcTranslator[i])
				m_pAvcToSvcTranslator[i] = new CAvcToSvcTranslator( *rOtherBridgePartyVideoIn.m_pAvcToSvcTranslator[i] );
	}
	m_bIsTranslatorCreated = 					rOtherBridgePartyVideoIn.m_bIsTranslatorCreated;
}
CBridgePartyVideoIn::~CBridgePartyVideoIn()
{
	POBJDELETE(m_pImage);
	POBJDELETE(m_pDecoderSyncSegm);
	POBJDELETE(m_pAvcToSvcParams);
	for (int i=0; i< MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		POBJDELETE(m_pAvcToSvcTranslator[i]);
	POBJDELETE(m_pPartyVisualEffects);

}

//--------------------------------------------------------------------------
void* CBridgePartyVideoIn::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyVideoInParams)
{
	CBridgePartyVideoUniDirection::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyVideoInParams);

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " \n ";

	char siteName[MAX_SITE_NAME_ARR_SIZE] = {0};

	// VNGFE-3127, VNGR-02237 - do not update site name for link to Master/Slave
	if (pBridgePartyCntl->GetCascadeLinkMode() == NONE)
	{
		if ('\0' != *((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetSiteName())
			strcpy_safe(siteName, ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetSiteName());
		else
			strcpy_safe(siteName, m_pBridgePartyCntl->GetName());

		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", SiteName:" << siteName;
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Site name is not updated for Link to Master/Slave.";
	}

	CDwordBitMask muteMask = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetMuteMask();

	CRelayMediaStreamList listVideoMediaStreams;
	listVideoMediaStreams.clear();
	m_pImage = new CImage(
		pRsrcParams->GetConnectionId(), pRsrcParams->GetPartyRsrcId(),
		m_pBridgePartyCntl->GetPartyTaskApp(), siteName, m_pBridgePartyCntl->GetName(),
		DEFAULT_DECODER_DETECTED_MODE_WIDTH, DEFAULT_DECODER_DETECTED_MODE_HEIGHT,
		DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH, DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT,
		m_videoResolution, m_videoAlg, m_FS, m_MBPS, muteMask, INVALID, INVALID, listVideoMediaStreams, m_eCascadeMode);

	m_pImage->SyncLost();
	m_last_sync_status     = (DWORD)(-1);
	m_eTelePresenceMode    = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetTelePresenceMode();
	m_backgroundImageID    = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetBackgroundImageID();
	m_last_sync_resolution = eVideoResolutionDummy;
	m_isAutoBrightness =  ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetIsAutoBrightness();

	m_confMediaType = m_pBridgePartyCntl->GetConfMediaType();
	
	if (eMixAvcSvc == m_confMediaType)
	{
		TRACEINTO << " - Mix Mode Conference";
		CVisualEffectsParams* pVisualEffects = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetVisualEffects();

		if (IsValidPObjectPtr(pVisualEffects))
			m_pPartyVisualEffects = new CVisualEffectsParams(*pVisualEffects);
		else
			m_pPartyVisualEffects = new CVisualEffectsParams();

		if((strlen(siteName) > 0) && !strncmp(siteName, "##I_AM_THE_CONTENT_DECODER", strlen("##I_AM_THE_CONTENT_DECODER")))	
		{
			TRACEINTO << "content decoder have no AVC to SVC translator";
		}
		else
		{

			CAvcToSvcParams* pAvcToSvcParams = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetAvcToSvcParams();
			SaveAvcToSvcParams(pAvcToSvcParams);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::CreateForMove(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyVideoInParams)
{
	CBridgePartyVideoUniDirection::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyVideoInParams);

	char siteName[MAX_SITE_NAME_ARR_SIZE] = {0};

	// VNGFE-3127, VNGR-02237 - do not update site name for link to Master/Slave
	if (pBridgePartyCntl->GetCascadeLinkMode() == NONE)
	{
		if ('\0' != *((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetSiteName())
			strcpy_safe(siteName, ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetSiteName());
		else
			strcpy_safe(siteName, m_pBridgePartyCntl->GetName());

		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", SiteName:" << siteName;
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Site name is not updated for Link to Master/Slave.";
	}

	CDwordBitMask muteMask = ((CBridgePartyVideoInParams*) pBridgePartyVideoInParams)->GetMuteMask();

	CRelayMediaStreamList listVideoMediaStreams;
	listVideoMediaStreams.clear();
	m_pImage = new CImage(
		pRsrcParams->GetConnectionId(), pRsrcParams->GetPartyRsrcId(),
		m_pBridgePartyCntl->GetPartyTaskApp(), siteName, m_pBridgePartyCntl->GetName(),
		DEFAULT_DECODER_DETECTED_MODE_WIDTH, DEFAULT_DECODER_DETECTED_MODE_HEIGHT,
		DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH, DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT,
		m_videoResolution, m_videoAlg, m_FS, m_MBPS, muteMask, INVALID, INVALID, listVideoMediaStreams, m_eCascadeMode);

	m_eTelePresenceMode = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetTelePresenceMode();
	m_backgroundImageID = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetBackgroundImageID();
	if (IsAllTranslatorsConnectedOK())	// checks if Translators are connected OK
		AllTranslatorsUpdateImage();	// each translator updates the image with its parameters
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::Connect()
{
	DispatchEvent(CONNECT_VIDEO_IN, NULL);
}

//--------------------------------------------------------------------------
void	CBridgePartyVideoIn::DisConnect()
{
	DispatchEvent(DISCONNECT_VIDEO_IN, NULL);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdateSelfMute(RequestPriority who, EOnOff eOnOff)
{
	if(eOn == eOnOff)
		MuteYourSelf(who);
	else
		UnMuteYourSelf(who);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UnMuteYourSelf(RequestPriority who)
{
	switch(who)
	{
		case PARTY_Prior:
		{
			m_pImage->UnMuteByPartyMCV();
			break;
		}
		case CHAIRMAN_Prior:
		{
			m_pImage->UnMuteByChairman();
			break;
		}
		case OPERATOR_Prior:
		{
			m_pImage->UnMuteByOperator();
			break;
		}
		case MCMS_Prior:
		{
			m_pImage->UnMuteByMcms();
			break;
		}
		case SYNC_LOST_Prior:
		{
			m_pImage->SyncFound();
			break;
		}
		default:
		{
			DBGPASSERT(1);
		}
	} // switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::MuteYourSelf(RequestPriority who)
{
	switch(who)
	{
		case PARTY_Prior:
		{
			m_pImage->MuteByPartyMCV();
			break;
		}
		case CHAIRMAN_Prior:
		{
			m_pImage->MuteByChairman();
			break;
		}
		case OPERATOR_Prior:
		{
			m_pImage->MuteByOperator();
			break;
		}
		case MCMS_Prior :
		{
			m_pImage->MuteByMcms();
			break;
		}
		case SYNC_LOST_Prior:
		{
			m_pImage->SyncLost();
			break;
		}
		default:
		{
			DBGPASSERT(1);
		}
	} // switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::SetSiteName(const char* visualName)
{
	if(m_pBridgePartyCntl->GetCascadeLinkMode()==NONE || ((CVideoBridgePartyCntl*)m_pBridgePartyCntl)->IsAVMCUParty())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", SiteName:" << visualName;
		m_pImage->SetSiteName(visualName);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", SiteName:" << visualName << ", Party is link to Master/Slave, site name is ignored";
	}
}
//--------------------------------------------------------------------------
const char* CBridgePartyVideoIn::GetSiteName() const
{
	char* siteName = NULL;
	if(!m_pImage)
	{
		return siteName;
	}
	return  m_pImage->GetSiteName();
}

//--------------------------------------------------------------------------
void  CBridgePartyVideoIn::SendOpenDecoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ON(m_isPortOpened);

	BYTE isVsw = NO;
	DWORD parsingMode = E_PARSING_MODE_CP;
	if(IsVsw())
	{
		isVsw = YES;
		parsingMode = E_PARSING_MODE_PSEUDO_VSW;
	}
	else
	{
		// VNGR-15880 -
		// in case current video params are lower than the actual allocated port,
		// --> send open_port with parameters of allocated port, and then send update with current params
		// (dsp can not update parameters to higher parameters than in open_port)
		FixCurrentVideoParamsAccordingToAllocationAndUpdateIfNeeded();
	}

	// Tsahi - Call Generator SoftMCU
	BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

	TRACEINTO << " Send Open AVC Decoder, PartyRsrcId: " << (DWORD)m_pHardwareInterface->GetPartyRsrcId() << ", Algo: " << (DWORD)m_videoAlg;

	m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterface)->SendOpenDecoder(m_videoAlg,
	                                                                                m_videoBitRate,
	                                                                                m_videoResolution,
	                                                                                m_videoQcifFrameRate,
	                                                                                m_videoCifFrameRate,
	                                                                                m_video4CifFrameRate,
	                                                                                m_MBPS, m_FS,
	                                                                                m_sampleAspectRatio,
	                                                                                m_staticMB,
	                                                                                isVsw,
	                                                                                m_backgroundImageID,
	                                                                                m_isVideoClarityEnabled,
	                                                                                m_videoConfType,
	                                                                                m_maxDPB,
	                                                                                parsingMode,
	                                                                                m_eTelePresenceMode,
	                                                                                m_isAutoBrightness,
	                                                                                eVideoFrameRateDUMMY,
	                                                                                eVideoFrameRateDUMMY,
	                                                                                eVideoFrameRateDUMMY,
	                                                                                E_VIDEO_DECODER_NORMAL,
	                                                                                m_videoFrameRate,
	                                                                                m_profile,
	                                                                                m_packetPayloadFormat,
	                                                                                m_bIsTipMode,
	                                                                                m_isH263Plus,
	                                                                                isCallGeneratorConf,
	                                                                                m_msftSvcParamsStruct.ssrc,
	                                                                                m_msftSvcParamsStruct.nHeight,
	                                                                                m_msftSvcParamsStruct.nWidth);

	m_lastReq = TB_MSG_OPEN_PORT_REQ;
}

//--------------------------------------------------------------------------
void  CBridgePartyVideoIn::SendCloseDecoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	OFF(m_isConnected);
	OFF(m_isPortOpened);
	m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterface)->SendCloseDecoder();
	m_lastReq = TB_MSG_CLOSE_PORT_REQ;
}

//--------------------------------------------------------------------------
void  CBridgePartyVideoIn::SendConnectToRtp()
{
	ConnectionID decoderConnectionId, rtpConnectionId;
	decoderConnectionId = rtpConnectionId = DUMMY_CONNECTION_ID;

	DWORD decoderRsrcPartyId, rtpRsrcPartyId;
	decoderRsrcPartyId = rtpRsrcPartyId = DUMMY_PARTY_ID;

	decoderConnectionId = m_pHardwareInterface->GetConnectionId();
	decoderRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();
	rtpRsrcPartyId = m_pHardwareInterface->GetPartyRsrcId();

	CRsrcDesc* pRsrcDesc;
	// Eitan - ISDN party has no RTP - connect MUX instead?
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(m_pHardwareInterface->GetPartyRsrcId(), eLogical_rtp);
	if(pRsrcDesc)
	{
		rtpConnectionId = pRsrcDesc->GetConnectionId();
	}
	else
	{
		DBGPASSERT(101);
	}
	ON(m_isConnected);
	m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterface)->SendConnect(decoderConnectionId, rtpConnectionId, decoderRsrcPartyId, rtpRsrcPartyId);
	m_lastReq = TB_MSG_CONNECT_REQ;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdateVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	TRACEINTO << " Update AVC Params - need to handle AvcToSVcTRanslator params as well";
	CSegment* pSeg = new CSegment;

	pBridgePartyVideoParams->Serialize(NATIVE,*pSeg);

	DispatchEvent(UPDATE_VIDEO_IN_PARAMS,pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdateNewConfParamsForOpenedPortAfterMove(DWORD confRsrcId, const CBridgePartyMediaParams * pBridgePartyVideoInParams)
{
	CBridgePartyMediaUniDirection::UpdateNewConfParams(confRsrcId);
	SaveAndSendUpdatedVideoParams((CBridgePartyVideoParams*)pBridgePartyVideoInParams);
	UpdateTranslatorsWithNewConfParamsAfterMove( confRsrcId );
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdateNewConfParams(DWORD confRsrcId, const CBridgePartyMediaParams * pBridgePartyVideoParams)
{
	CBridgePartyVideoUniDirection::UpdateNewConfParams(confRsrcId,pBridgePartyVideoParams);
	m_pImage->UpdateVideoParams(m_videoAlg, m_videoResolution,m_MBPS, m_FS);
	UpdateTranslatorsWithNewConfParamsAfterMove( confRsrcId );
}
// ------------------------------------------------------------
 void CBridgePartyVideoIn::UpdateVisualEffects(CVisualEffectsParams* pVisualEffects, BYTE bInternalUpdateOnly)
 {
	 CSegment* pSeg = new CSegment;
	 pVisualEffects->Serialize(NATIVE,*pSeg);
	 *pSeg << (BYTE) bInternalUpdateOnly;
	 DispatchEvent(UPDATEVISUALEFFECTS,pSeg);
	 POBJDELETE(pSeg);
 }
// ------------------------------------------------------------
void CBridgePartyVideoIn::UpdatePartyTelePresenceMode(eTelePresencePartyType partyNewTelePresenceMode)
{
	CSegment* pSeg = new CSegment;

	*pSeg << (WORD)partyNewTelePresenceMode;

	DispatchEvent(UPDATE_PARTY_TELEPRESENCE_MODE,pSeg);
	POBJDELETE(pSeg)
}
//--------------------------------------------------------------------------
void CBridgePartyVideoIn::DumpAllInfoOnConnectionState(CMedString* pMedString,bool isShortPrint)
{
	if(!isShortPrint && m_state == IDLE)
		*pMedString << "IDLE";
	else if(!isShortPrint && m_state == CONNECTED)
		*pMedString << "CONNECTED";
	else if(!isShortPrint && m_state == DISCONNECTING)
		*pMedString << "DISCONNECTING";
	else if(m_state == SETUP)
	{
		if (!isShortPrint)
			*pMedString << "SETUP : ";

		if(!m_isConnected)
			*pMedString << "No ack TB_MSG_OPEN_PORT_REQ";
		else
			*pMedString << "No ack TB_MSG_CONNECT_REQ";
	}
	else
		*pMedString << "UNKNOWN STATE";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdateAutoBrightness(WORD isAutoBrightnessEnabled)
{
	CSegment* pSeg = new CSegment;
	*pSeg << isAutoBrightnessEnabled;
	DispatchEvent(UPDATE_AUTO_BRIGHTNESS,pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyConnectIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	m_state = SETUP;
	SetClosePortAckStatus(statOK);
	ResetCloseAvcSvcTranslatorStatus();// set to statOK
	SendOpenDecoder();
	m_bDecoderClosed = FALSE;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyConnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Already connected";

	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_CONNECTED);

	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyDisConnectIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Already disconnected";

	CSegment* pSeg = new CSegment;

	*pSeg  << (BYTE)statOK;

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_DISCONNECTED);

	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyDisConnectSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	if(!m_isPortOpened)
	{
		//Port never opened - Disconnecting while setup or because of no ack or bad status in ack
		m_state = IDLE;
		CSegment *pMsg = new CSegment;
		*pMsg << (BYTE)statOK;
		m_pBridgePartyCntl->HandleEvent(pMsg,0,VIDEO_IN_DISCONNECTED);
		POBJDELETE(pMsg);
	}
	else
		OnVideoBridgePartyDisConnect(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyDisConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	OnVideoBridgePartyDisConnect(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyDisConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyDisConnect(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	m_state = DISCONNECTING;

	m_pImage->SyncLost();

	SendCloseDecoder();

	// close Avc to Svc translators
	for (int i=0; i< MAX_ENCODERS_PER_AVC_TO_SVC; i++)
	{
		if (m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->Disconnect();
	}

}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnMplAckSETUP(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch(AckOpcode)
	{
		case	TB_MSG_OPEN_PORT_REQ:
		{
			OnMplOpenPortAck(status);
			break;
		}
		case	TB_MSG_CONNECT_REQ:
		{
			OnMplConnectAck(status);
			break;
		}
		default:
		{
			TRACEINTO << m_pBridgePartyCntl->GetName() << " - ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
		}
	}// end switch
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnMplAckCONNECTED(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
		case VIDEO_DECODER_UPDATE_PARAM_REQ:
		{
			DeleteTimer(DECODER_UPDATE_PARAM_TOUT);

			TRACEINTO << m_pBridgePartyCntl->GetFullName()
			          << ", AckOpcode:VIDEO_DECODER_UPDATE_PARAM_REQ, waitForUpdateDecoderAck:" << (int)m_waitForUpdateDecoderAck
			          << ", Status:" << status;

			if (m_waitForUpdateDecoderAck == TRUE)
			{
				RequestChangeLayoutFromAvcToSvcTranslators();
				m_waitForUpdateDecoderAck = FALSE;
				if (status == STATUS_OK)
				{
					CSegment* pSeg = new CSegment;
					*pSeg << (BYTE)status;
					m_pBridgePartyCntl->HandleEvent(pSeg, 0, PARTY_IMAGE_UPDATED);
					POBJDELETE(pSeg);
				}
				else
				{
					// at this stage (V5.0.1) we do nothing, add error handling later if we need
					PASSERT(status);
				}
			}
			break;
		}
		default:
		{
			// do nothing
			break;
		}
	} // end switch
}

// ------------------------------------------------------------
void  CBridgePartyVideoIn::OnMplOpenPortAck(STATUS  status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	ISDEBUGMODE_SET_STATUS("VIDEOBRIDGEIN", 1 ,1001)	// simulate Ack error from Open Decoder
	ISDEBUGMODE_RETURN("VIDEOBRIDGEIN", 2)				// simulate no Ack received	on Open Decoder


	if (status != STATUS_OK)
	{
		InformConnectEndedWithError( statVideoInOutResourceProblem );
		return;
	}

	if(eMixAvcSvc == m_confMediaType)
	{
		// at this stage, we start to connect the AvcToSvc Translator if needed
		if(IsTranslateAvcToSvcSupported() && !m_bIsTranslatorCreated)
		{
			TRACEINTO << " - AVC-Translator-Start... ";
			int status = TranslateAvcToSvc();
			if(status != WAIT_FOR_TRANSLATOR)
			{

				TRACEINTO << " - AVC-Translator-Error ";
				InformConnectEndedWithError( statVideoInOutResourceProblem );
				return;
			}
		}
		else
		{
			TRACEINTO << " - AvcToSvcTranslator not started: Either not Mix-Mode or already started, IsAlreadyCreated:" << (bool)m_bIsTranslatorCreated;
		}
	}
	else
	{
		TRACEINTO << " - not Mix-Mode conference ";
	}

	// connect decoder to RTP
	m_DecoderConnected = false;
	SendConnectToRtp();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnMplConnectAck(STATUS status)
{
	ISDEBUGMODE_SET_STATUS("VIDEOBRIDGEIN", 3 ,1001)	// simulate Ack error from Open Decoder
	ISDEBUGMODE_RETURN("VIDEOBRIDGEIN", 4)				// simulate no Ack received	on Open Decoder

	if (TB_MSG_OPEN_PORT_REQ == m_lastReq) // BRIDGE-12215 (was in OUT, but it is true also for IN)
	{
		// Ack on CONNECT while the last request was OPEN
		// this may happen in case of fast connect-disconnect-connect and the Ack from the first connect arrives after the second connect
		TRACEINTO << " Warning: Old Ack in new connect, ignore this Ack, PartyRsrcId: " << (DWORD)m_pBridgePartyCntl->GetPartyRsrcID();
		return;
	}
	if (status != STATUS_OK)
	{
		InformConnectEndedWithError( statVideoInOutResourceProblem );
	}
	else
	{
		m_DecoderConnected = true;

		if (m_bIsTranslatorCreated)
			if (!IsAllTranslatorsConnectedOK())
			{
				TRACEINTO << ", Status OK, waiting for Translators, Party Name: " << m_pBridgePartyCntl->GetFullName();
				return;
			}

		SetVideoInConnected();
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::InformConnectEndedWithError( EStat receivedStatus )
{
	CSegment* pSeg = new CSegment;

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status not OK - send answer to VideoBridge";

	// Add assert to EMA in case of NACK
	// AddFaultAlarm("NACK on Connect video decoder",m_pHardwareInterface->GetPartyRsrcId(),status);
	*pSeg << (BYTE)receivedStatus << (BYTE)eMipIn << (BYTE)eMipStatusFail << (BYTE) eMipConnect;

	// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoDecoder VideoEncoder
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_CONNECTED);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::SetVideoInConnected()
{
	TRACEINTO << " - State changed to CONNECTED, Party Name: " << m_pBridgePartyCntl->GetFullName();
	m_state = CONNECTED;

	if (IsFeatureSupportedBySystem(eFeatureDecoderRecurrentIntra) && ((m_videoConfType == eVideoConfTypeCP) || (m_videoConfType == eVideoConfTypeCopHD108025fps) || (m_videoConfType == eVideoConfTypeCopHD72050fps)))
	{
		DWORD       decoderRequestIntraToutValue = 0;
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		sysConfig->GetDWORDDataByKey("DECODER_RECURRENT_INTRA_REQ_MINUTES", decoderRequestIntraToutValue);
		if (decoderRequestIntraToutValue)
			StartTimer(DECODER_RECURRENT_INTRA_REQ_TIMEOUT, decoderRequestIntraToutValue*60*SECOND);
	}

	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)statOK;
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_CONNECTED);

	// ask for fast update from ep every 1 second, until sync ind received from decode
	m_pBridgePartyCntl->HandleEvent(NULL, 0, VIDREFRESH);
	StartTimer(VIDEO_DECODER_SYNC_TOUT, VIDEO_DECODER_SYNC_TOUT_VALUE);

	// check if update required
	if (m_pWaitingForUpdateParams)
	{
		TRACEINTO << "Save and Send Update Video Params";
		SaveAndSendUpdatedVideoParams(m_pWaitingForUpdateParams);
		POBJDELETE(m_pWaitingForUpdateParams);
	}

	if (m_pDecoderSyncSegm)
	{
		// initiate VIDEO_DECODER_SYNC_IND because it came earlier (before ACK)
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Initiate VIDEO_DECODER_SYNC_IND";
		DispatchEvent(VIDEO_DECODER_SYNC_IND, m_pDecoderSyncSegm);
		POBJDELETE(m_pDecoderSyncSegm);
	}

	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnMplAckDISCONNECTING(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	switch (AckOpcode)
	{
		case  TB_MSG_CLOSE_PORT_REQ:
		{
			OnMplClosePortAck(status);
			break;
		}
		default:
		{
			TRACEINTO << m_pBridgePartyCntl->GetName() << ", ACK_IND ignored, AckOpcode:" << CProcessBase::GetProcess()->GetOpcodeAsString(AckOpcode);
		}
	} // end switch
		}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnMplClosePortAck(STATUS status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	ISDEBUGMODE_SET_STATUS("VIDEOBRIDGEIN", 5 ,1001)	// simulate Ack error from Open Decoder
	ISDEBUGMODE_RETURN("VIDEOBRIDGEIN", 6)				// simulate no Ack received	on Open Decoder

	BYTE	responseStatus = statOK;

	if(status!=STATUS_OK)
	{
		TRACESTRFUNC(eLevelError) << m_pBridgePartyCntl->GetFullName() << ", Status:" << status << " - Failed, bad status";

		//Add assert to EMA in case of NACK
		//AddFaultAlarm("NACK on Close video decoder",m_pHardwareInterface->GetPartyRsrcId(),status);

		responseStatus = statVideoInOutResourceProblem;		// statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoDecoder VideoEncoder
	}
	SetClosePortAckStatus(responseStatus);

	m_bDecoderClosed = TRUE;

	// check if Decoder and Encoders closed, and if so change to IDLE and inform Bridge Party Cntl
	CheckAndInformAllClosed();
}

// ------------------------------------------------------------
void  CBridgePartyVideoIn::CheckAndInformAllClosed()
{
	// waiting for Avc-to-Svc close encoders
	bool bAllAvcToSvcClosed = true;
	for (int i=0; i< MAX_ENCODERS_PER_AVC_TO_SVC; i++)
	{
		if (m_pAvcToSvcTranslator[i])
			if (m_pAvcToSvcTranslator[i]->IsActive())
			{
				bAllAvcToSvcClosed = false;
				break;
			}
	}

	if (bAllAvcToSvcClosed && m_bDecoderClosed)
		InformInClosed();
}


// ------------------------------------------------------------
void  CBridgePartyVideoIn::InformInClosed()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - State changed to IDLE";
	m_state = IDLE;

	BYTE responseStatus = statOK;
	BYTE responseDecoderStatus = GetClosePortAckStatus();
	BYTE responseAvcSvcTranslatorStatus = GetCloseAvcSvcTranslatorStatus();

	if (statOK != responseAvcSvcTranslatorStatus)
		responseStatus = responseAvcSvcTranslatorStatus;
	if (statOK != responseDecoderStatus)
		responseStatus = responseDecoderStatus;	// the Decoder Error is "stronger" than the Translator error (even so it is not really matter)
	else
		SetClosePortAckStatus(responseStatus);	// in case the Close-Decoder was OK but the Close Translator is not, we need to sign as the decoder was not closed OK for MIP


	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)responseStatus;
	if(responseStatus == statVideoInOutResourceProblem)
		*pSeg  << (BYTE)eMipIn << (BYTE)eMipStatusFail << (BYTE)eMipClose;

	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_DISCONNECTED);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnMplDecoderSyncSETUP(CSegment* pParam)
{
	if (m_isConnected)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName();
		POBJDELETE(m_pDecoderSyncSegm);
		m_pDecoderSyncSegm = new CSegment(*pParam);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored (DECODER_SYNC_IND arrived when video in is not connected yet)";
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnMplDecoderSyncCONNECTED(CSegment* pParam)
{
	STATUS status;
	DWORD  decoderDetectedModeWidth               = DEFAULT_DECODER_DETECTED_MODE_WIDTH;
	DWORD  decoderDetectedModeHeight              = DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
	DWORD  decoderDetectedSampleAspectRatioWidth  =  DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH;
	DWORD  decoderDetectedSampleAspectRatioHeight = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT;
	DWORD  decoderMSSvcSsrcID = INVALID;
	DWORD  decoderMSSvcPriorityID   = INVALID;

	//CSegment *paramForMsSvcSync = (m_videoAlg == MS_SVC)? new CSegment(*pParam) : NULL;
	*pParam >>  decoderDetectedModeWidth >> decoderDetectedModeHeight >> decoderDetectedSampleAspectRatioWidth >> decoderDetectedSampleAspectRatioHeight >> status >> decoderMSSvcSsrcID  >> decoderMSSvcPriorityID;

	m_last_sync_status = (DWORD)status;
	//BYTE isImageSyncBefore = !m_pImage->isSyncLost();

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	if (status != STATUS_OK)
	{
		m_pImage->SetOutOfSync(TRUE);

		if (IsValidTimer(VIDEO_DECODER_SYNC_TOUT))
			return;

		UpdateDBLocalVideoSyncState(NO);

		WORD ignore_filtering       = FALSE;
		WORD isRequestFromRemoteMGC = FALSE;
		CSegment* pSeg = new CSegment;
		*pSeg << ignore_filtering << isRequestFromRemoteMGC << decoderMSSvcSsrcID << decoderMSSvcPriorityID;
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDREFRESH);
		POBJDELETE(pSeg);

		StartTimer(VIDEO_DECODER_SYNC_TOUT, VIDEO_DECODER_SYNC_TOUT_VALUE);

		//POBJDELETE(paramForMsSvcSync);

	}
	else
	{
		if (m_pImage->isSyncLost()) // initial synchronization of party
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Initial SYNC"
			          << ", decoderDetectedModeWidth:"               << decoderDetectedModeWidth
			          << ", decoderDetectedModeHeight:"              << decoderDetectedModeHeight
			          << ", decoderDetectedSampleAspectRatioWidth:"  << decoderDetectedSampleAspectRatioWidth
			          << ", decoderDetectedSampleAspectRatioHeight:" << decoderDetectedSampleAspectRatioHeight
			          << ", decoderSvcSsrcID:"                       << decoderMSSvcSsrcID
			          << ", decoderSvcPriorityID:"                   << decoderMSSvcPriorityID;

			m_pImage->SyncFound(decoderDetectedModeWidth, decoderDetectedModeHeight, decoderDetectedSampleAspectRatioWidth, decoderDetectedSampleAspectRatioHeight,decoderMSSvcSsrcID,decoderMSSvcPriorityID);

			// tell video bridge to enter party to mix
			CSegment *paramForMsSvcSync =  new CSegment(*pParam);
			m_pBridgePartyCntl->HandleEvent(paramForMsSvcSync, 0, VIDEO_IN_SYNCED);
			POBJDELETE(paramForMsSvcSync);

		}
		else
		{
			if ((decoderDetectedModeWidth != m_pImage->GetDecoderDetectedModeWidth()) ||
					(decoderDetectedModeHeight != m_pImage->GetDecoderDetectedModeHeight()) ||
					(decoderDetectedSampleAspectRatioWidth != m_pImage->GetDecoderDetectedSampleAspectRatioWidth()) ||
					(decoderDetectedSampleAspectRatioHeight != m_pImage->GetDecoderDetectedSampleAspectRatioHeight()))
			{
				TRACEINTO << m_pBridgePartyCntl->GetFullName()
				        				  << "\n  Old decoder detected mode width                 :" <<  m_pImage->GetDecoderDetectedModeWidth()
				        				  << "\n  New decoder detected mode width                 :" << decoderDetectedModeWidth
				        				  << "\n  Old decoder detected mode height                :" <<  m_pImage->GetDecoderDetectedModeHeight()
				        				  << "\n  New decoder detected mode height                :" << decoderDetectedModeHeight
				        				  << "\n  Old decoder detected sample aspect ratio width  :" << m_pImage->GetDecoderDetectedSampleAspectRatioWidth()
				        				  << "\n  New decoder detected sample aspect ratio width  :" << decoderDetectedSampleAspectRatioWidth
				        				  << "\n  Old decoder detected sample aspect ratio height :" << m_pImage->GetDecoderDetectedSampleAspectRatioHeight()
				        				  << "\n  New decoder detected sample aspect ratio height :" << decoderDetectedSampleAspectRatioHeight;

				m_pImage->SetDecoderDetectedParams(decoderDetectedModeWidth, decoderDetectedModeHeight, decoderDetectedSampleAspectRatioWidth, decoderDetectedSampleAspectRatioHeight, decoderMSSvcSsrcID,decoderMSSvcPriorityID);

				m_pBridgePartyCntl->HandleEvent(NULL, 0, UPDATE_DECODER_DETECTED_MODE);

			}

			m_pBridgePartyCntl->HandleEvent(NULL, 0, SEND_H239_VIDEO_CAPS);
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Back to SYNC";
		}

		m_pImage->SetOutOfSync(FALSE);
		DeleteTimer(VIDEO_DECODER_SYNC_TOUT);

		UpdateDBLocalVideoSyncState(YES);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::SaveAvcToSvcParams(const CAvcToSvcParams*  other)
{
	DWORD confMediaType = m_pBridgePartyCntl->GetConfMediaType();

	if (!other)
	{
		// xxxAmir - what about Upgrade, will it stay with current params or need to delete existing params?
		if (eMixAvcSvc == confMediaType)
		{
			PASSERTSTREAM_AND_RETURN(101, m_pBridgePartyCntl->GetFullName() << " - AVC-Translator-Error, No AVC-to-SVC params, Translator not started");
		}
		else
			return;
	}

	PDELETE(m_pAvcToSvcParams);
	m_pAvcToSvcParams = new CAvcToSvcParams(*other);

	if (eMixAvcSvc != confMediaType)
		m_pAvcToSvcParams->SetIsSupportAvcSvcTranslate(false);

	if (m_pAvcToSvcParams->IsSupportAvcSvcTranslate())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - AVC-to-SVC Translate is Active";
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - AVC-to-SVC Translate is not Active";
	}
}

//--------------------------------------------------------------------------
CVideoOperationPointsSet* CBridgePartyVideoIn::GetConfVideoOperationPointsSet()const
{
	if(IsValidPObjectPtr(m_pBridgePartyCntl))
	{
		return ((CVideoBridgePartyCntl * )m_pBridgePartyCntl)->GetConfVideoOperationPointsSet();
	}
	PASSERT(101);
	return NULL;
}

//--------------------------------------------------------------------------
bool CBridgePartyVideoIn::IsTranslateAvcToSvcSupported()
{
	if(IsValidPObjectPtr(m_pAvcToSvcParams))
		if(m_pAvcToSvcParams->IsSupportAvcSvcTranslate())
			return true;

	return false;
}

//--------------------------------------------------------------------------
int CBridgePartyVideoIn::TranslateAvcToSvc()
{
	bool updateOk = UpdateAvcSvcParams();
	if(updateOk)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName();
		int status = CreateAndConnectAvcToSvcTranslator();
		if (status != STATUS_OK)
		{
			return TRANSLATOR_ERROR;
		}
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - AVC-Translator-Error - Failed to update SVC to AVC parameters";
		return TRANSLATOR_ERROR;
	}

	return WAIT_FOR_TRANSLATOR;
}

//--------------------------------------------------------------------------
bool CBridgePartyVideoIn::UpdateAvcSvcParams()
{
	if (!m_pAvcToSvcParams)	// should not happened
		return false;

	// get OperationPoints
	m_pAvcToSvcParams->SetVideoOperationPointsSet(GetConfVideoOperationPointsSet());

	m_pAvcToSvcParams->Dump();
	return true;
}

//--------------------------------------------------------------------------
int CBridgePartyVideoIn::CreateAndConnectAvcToSvcTranslator()
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	if (!pRoutingTable) {
		PASSERT(101);
		return STATUS_FAIL;
	}
	if (!m_pBridgePartyCntl) {
		PASSERT(102);
		return STATUS_FAIL;
	}

    const std::list<CVideoRelayMediaStream*>& lVideoRelayMediaStream = m_pAvcToSvcParams->GetVideoRelayMediaStream();
	std::list<CVideoRelayMediaStream*>::const_iterator itVideoMedia = lVideoRelayMediaStream.begin();
	if (itVideoMedia == lVideoRelayMediaStream.end()) {
		TRACESTRFUNC(eLevelError) << m_pBridgePartyCntl->GetFullName() << " AVC-Translator-Error: No video streams";
		return STATUS_FAIL;
	}

	ISDEBUGMODE_RETURN_STAT("TRANSLATOR", 31, STATUS_FAIL) // simulate error initialize translators


	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
	DWORD confRsrcId  	= m_pBridgePartyCntl->GetConfRsrcID();
	int status = STATUS_OK;
	int enNum=0;
	int vswStreamCount = 0;
	for ( ; ((enNum < MAX_ENCODERS_PER_AVC_TO_SVC) && (itVideoMedia != lVideoRelayMediaStream.end())); enNum++, ++itVideoMedia)
	{
		// check if it Vsw Path or regular)
		BOOL bIsVswStream = (*itVideoMedia)->IsVswStream();

		CRsrcDesc* pEncoderRsrcDesc = NULL;
		if (FALSE == bIsVswStream)
		{
			pEncoderRsrcDesc = pRoutingTable->GetPartyRsrcDesc(partyRsrcId, (0==enNum ? eLogical_relay_avc_to_svc_video_encoder_1 : eLogical_relay_avc_to_svc_video_encoder_2));
			if (!pEncoderRsrcDesc)
			{
				TRACESTRFUNC(eLevelError) << "AVC-Translator-Error: GetPartyRsrcDesc - partyRsrcId=" << partyRsrcId << ", enNum=" << (DWORD)enNum;
				status = STATUS_FAIL;
				break;
			}
			TRACEINTO << " Stream index (" << (DWORD)enNum << ") is NOT VSW";
		}
		else
		{
			TRACEINTO << " Stream index (" << (DWORD)enNum << ") is VSW";
			vswStreamCount++;
			if (vswStreamCount > 1)
			{
				TRACESTRFUNC(eLevelError) << "AVC-Translator-Error: More than one VSW stream, partyRsrcId=" << partyRsrcId;
				status = STATUS_FAIL;
				break;
			}
		}

		POBJDELETE(m_pAvcToSvcTranslator[enNum]);
		ConnectionID encoderConnectionId = 0xFFFFFFFF;
		if (FALSE == bIsVswStream)
		{
			m_pAvcToSvcTranslator[enNum] = new CAvcToSvcTranslator();
			encoderConnectionId = pEncoderRsrcDesc->GetConnectionId();

			status = AddTranslaytorToRoutingTbl( partyRsrcId, m_pAvcToSvcTranslator[enNum], encoderConnectionId );
			if (status != STATUS_OK)
			{
				TRACESTRFUNC(eLevelError) << "AVC-Translator-Error: Add To RoutingTbl failed - partyRsrcId=" << partyRsrcId << ", enNum=" << (DWORD)enNum;
				POBJDELETE(m_pAvcToSvcTranslator[enNum]);
				break;
			}
		}
		else
			m_pAvcToSvcTranslator[enNum] = new CAvcToSvcTranslatorVsw();


		CRsrcParams encoderRsrcParams( encoderConnectionId, partyRsrcId, confRsrcId );
		if (FALSE == bIsVswStream)
		{
			encoderRsrcParams.SetRsrcDesc(*pEncoderRsrcDesc);
			encoderRsrcParams.SetConnectionId(encoderConnectionId);
		}

		// Currently:  (temporary solution) the first one is the low resolution and will be connected to SAC Encoder.
		// xxxAmir: need to pass parameter via streams indicating this instead of select the first one.
		bool bConnectToSACEncoder = (0 == enNum);

		m_pAvcToSvcTranslator[enNum]->Create(m_pBridgePartyCntl, this, &encoderRsrcParams, bConnectToSACEncoder );
		m_pAvcToSvcTranslator[enNum]->UpdateParams(	(*itVideoMedia)->GetLayerId(), (*itVideoMedia)->GetResolutionHeight(),
													(*itVideoMedia)->GetResolutionWidth(), (*itVideoMedia)->GetSsrc(), m_pAvcToSvcParams->GetChannelHandle() );
	}

	if (status != STATUS_OK)
	{
		for (int i = 0; i < enNum; i++) // remove what which already created/allocated
		{
			if (m_pAvcToSvcTranslator[i])
			{
				if (FALSE == m_pAvcToSvcTranslator[i]->IsVswStream())
						RemoveAvcToSvcStateMachineFromRoutingTable(m_pAvcToSvcTranslator[i]->GetEncoderConnectionId());
				POBJDELETE(m_pAvcToSvcTranslator[i]);
			}
		}
		PASSERT(104);
		return STATUS_FAIL;
	}

	for (int i=0; i<enNum; i++)	// connecting all the Encoders (Avc-To-Svc Translators)
		if (m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->Connect();

	m_bIsTranslatorCreated = TRUE;
	m_bTranslatorReportedOnConnectionError = false;

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam)
{
	TRACEINTO <<  " PartyRsrdId: " << (DWORD)m_pBridgePartyCntl->GetPartyRsrcID() << "  " << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoParams* pBridgePartyVideoParams = new CBridgePartyVideoParams;
	pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);

	EStat responseStatus = statOK;

	if (!CPObject::IsValidPObjectPtr(pBridgePartyVideoParams))
	{
		PASSERTMSG(1, "Internal Error receive invalid params");
		responseStatus = statIllegal;
	}
	else
	{
		m_videoAlg     = pBridgePartyVideoParams->GetVideoAlgorithm();
		m_videoBitRate = pBridgePartyVideoParams->GetVideoBitRate();
		m_eCascadeMode = pBridgePartyVideoParams->GetPartyCascadeType();
		if (m_videoAlg == H264 || m_videoAlg == MS_SVC)
		{
			m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
			m_videoCifFrameRate   = eVideoFrameRateDUMMY;
			m_video4CifFrameRate  = eVideoFrameRateDUMMY;
			m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
			m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoResolution     = eVideoResolutionDummy;
			m_MBPS                = pBridgePartyVideoParams->GetMBPS();
			m_FS                  = pBridgePartyVideoParams->GetFS();
			m_staticMB            = pBridgePartyVideoParams->GetStaticMB();
			m_profile             = pBridgePartyVideoParams->GetProfile();
			m_packetPayloadFormat = pBridgePartyVideoParams->GetPacketFormat();
			m_maxDPB              = pBridgePartyVideoParams->GetMaxDPB();
			m_bIsTipMode          = pBridgePartyVideoParams->GetIsTipMode();
			m_videoFrameRate      = pBridgePartyVideoParams->GetVidFrameRate();
			m_msftSvcParamsStruct = pBridgePartyVideoParams->MsSvcParams();
			TRACEINTO << m_msftSvcParamsStruct;
		}
		else if (m_videoAlg == VP8)
		{
			TRACEINTO << " VP8 Protocol, PartyRsrcId: " << (DWORD)m_pBridgePartyCntl->GetPartyRsrcID();

			m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
			m_videoCifFrameRate   = eVideoFrameRateDUMMY;
			m_video4CifFrameRate  = eVideoFrameRateDUMMY;
			m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
			m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoResolution     = eVideoResolutionDummy;
			m_MBPS                = pBridgePartyVideoParams->GetMBPS();
			m_FS                  = pBridgePartyVideoParams->GetFS();
			m_staticMB            = pBridgePartyVideoParams->GetStaticMB();
			m_profile             = pBridgePartyVideoParams->GetProfile();
			m_packetPayloadFormat = pBridgePartyVideoParams->GetPacketFormat();
			m_maxDPB              = pBridgePartyVideoParams->GetMaxDPB();
			m_bIsTipMode          = pBridgePartyVideoParams->GetIsTipMode();
			m_videoFrameRate      = pBridgePartyVideoParams->GetVidFrameRate();
		}
		else if (m_videoAlg == RTV)
		{
			m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
			m_videoCifFrameRate   = eVideoFrameRateDUMMY;
			m_video4CifFrameRate  = eVideoFrameRateDUMMY;
			m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
			m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoResolution     = eVideoResolutionDummy;
			m_MBPS                = pBridgePartyVideoParams->GetMBPS();
			m_FS                  = pBridgePartyVideoParams->GetFS();
			m_staticMB            = DEFAULT_STATIC_MB;
			m_profile             = eVideoProfileDummy;
			m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
			m_maxDPB              = INVALID;
			m_msftSvcParamsStruct = pBridgePartyVideoParams->MsSvcParams();
			TRACEINTO << m_msftSvcParamsStruct;
		}
		else
		{
			m_videoQcifFrameRate  = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionQCIF);
			m_videoCifFrameRate   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionCIF);
			m_video4CifFrameRate  = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolution4CIF);
			m_videoVGAFrameRate   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionVGA);
			m_videoSVGAFrameRate  = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionSVGA);
			m_videoXGAFrameRate   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionXGA);
			m_videoResolution     = pBridgePartyVideoParams->GetVideoResolution();
			m_MBPS                = INVALID;
			m_FS                  = INVALID;
			m_staticMB            = DEFAULT_STATIC_MB;
			m_profile             = eVideoProfileDummy;
			m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
			m_maxDPB              = INVALID;
			m_bIsTipMode          = FALSE;
		}

		m_isH263Plus        = pBridgePartyVideoParams->GetIsH263Plus();
		m_sampleAspectRatio = pBridgePartyVideoParams->GetSampleAspectRatio();  // the sample aspect ratio is relevant for H263 as well from cop version

		// Update the m_pImage as well with the updated parameters
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Update the image video params as well";
		m_pImage->UpdateVideoParams(m_videoAlg, m_videoResolution, m_MBPS, m_FS);
	}

	CSegment* pMsg = new CSegment;
	*pMsg << (BYTE)responseStatus;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
	POBJDELETE(pMsg);
	POBJDELETE(pBridgePartyVideoParams);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoParams* pBridgePartyVideoParams = new CBridgePartyVideoParams;
	pBridgePartyVideoParams->DeSerialize(NATIVE, *pParam);

	if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "Internal Error receive invalid params");

		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE)statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
		POBJDELETE(pMsg);
		POBJDELETE(pBridgePartyVideoParams);
		return;
	}

	m_pWaitingForUpdateParams  = new CBridgePartyVideoParams;
	*m_pWaitingForUpdateParams = *pBridgePartyVideoParams;

	POBJDELETE(pBridgePartyVideoParams);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	CBridgePartyVideoUniDirection::OnVideoBridgePartyUpdateVideoParamsCONNECTED(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{

	if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "Internal Error received invalid params");
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE)statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
		POBJDELETE(pMsg);
		return;
	}

	TRACEINTO << " Frame-Rate = " << pBridgePartyVideoParams->GetVidFrameRate() << " - " << m_pBridgePartyCntl->GetFullName();

	ECascadePartyType         newCascadeMode         = pBridgePartyVideoParams->GetPartyCascadeType();
	DWORD                     newVideoAlg            = pBridgePartyVideoParams->GetVideoAlgorithm();
	DWORD                     newVideoBitRate        = pBridgePartyVideoParams->GetVideoBitRate();
	eVideoFrameRate           newVideoQCifFrameRate  = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionQCIF);
	eVideoFrameRate           newVideoCifFrameRate   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionCIF);
	eVideoFrameRate           newVideo4CifFrameRate  = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolution4CIF);
	eVideoFrameRate           newVideoVGAFrameRate   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionVGA);
	eVideoFrameRate           newVideoSVGAFrameRate  = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionSVGA);
	eVideoFrameRate           newVideoXGAFrameRate   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionXGA);
	eVideoResolution          newVideoResolution     = pBridgePartyVideoParams->GetVideoResolution();
	DWORD                     newMBPS                = pBridgePartyVideoParams->GetMBPS();
	DWORD                     newFS                  = pBridgePartyVideoParams->GetFS();
	DWORD                     newSampleAspectRatio   = pBridgePartyVideoParams->GetSampleAspectRatio();
	DWORD                     newStaticMB            = pBridgePartyVideoParams->GetStaticMB();
	eVideoProfile             newProfile             = pBridgePartyVideoParams->GetProfile();
	eVideoPacketPayloadFormat newPacketPayloadFormat = pBridgePartyVideoParams->GetPacketFormat();
	DWORD                     newMaxDPB              = pBridgePartyVideoParams->GetMaxDPB();
	bool                      newAutoBrightness      = pBridgePartyVideoParams->GetIsAutoBrightness();
	BYTE                      bIsTipMode             = pBridgePartyVideoParams->GetIsTipMode();
	const MsSvcParamsStruct&  newMsSvcParams         = pBridgePartyVideoParams->MsSvcParams();
	eVideoFrameRate           newVideoFrameRate      = pBridgePartyVideoParams->GetVidFrameRate();
	BOOL ms_svc_diff = FALSE;
	if (newVideoAlg == MS_SVC &&(newMsSvcParams.ssrc != 0))
	{
		ms_svc_diff = m_msftSvcParamsStruct.ssrc != newMsSvcParams.ssrc ||
				m_msftSvcParamsStruct.pr_id != newMsSvcParams.pr_id ||
				m_msftSvcParamsStruct.nHeight != newMsSvcParams.nHeight ||
				m_msftSvcParamsStruct.nWidth != newMsSvcParams.nWidth;
		if (ms_svc_diff)
		{
			TRACEINTO << "received updated MS SVC parameters: " << newMsSvcParams;
		}
	}
	if (m_videoAlg == newVideoAlg &&
	    m_videoBitRate == newVideoBitRate &&
	    m_videoQcifFrameRate == newVideoQCifFrameRate &&
	    m_videoCifFrameRate == newVideoCifFrameRate &&
	    m_video4CifFrameRate == newVideo4CifFrameRate &&
	    m_videoVGAFrameRate == newVideoVGAFrameRate &&
	    m_videoSVGAFrameRate == newVideoSVGAFrameRate &&
	    m_videoXGAFrameRate == newVideoXGAFrameRate &&
	    m_videoResolution == newVideoResolution &&
	    m_MBPS == newMBPS &&
	    m_FS == newFS &&
	    m_sampleAspectRatio == newSampleAspectRatio &&
	    m_staticMB == newStaticMB &&
	    m_profile == newProfile &&
	    m_packetPayloadFormat == newPacketPayloadFormat &&
	    m_maxDPB == newMaxDPB &&
	    m_isAutoBrightness == newAutoBrightness &&
	    m_bIsTipMode == bIsTipMode &&
	    ms_svc_diff == FALSE)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - No real change in Params";
	}
	else
	{
		PASSERT_AND_RETURN(!m_pHardwareInterface);

		// Tsahi - Call Generator SoftMCU
		BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();
		m_msftSvcParamsStruct = newMsSvcParams;
		if (newVideoAlg == MS_SVC)
			{TRACEINTO << m_msftSvcParamsStruct;}
		((CVideoHardwareInterface*)m_pHardwareInterface)->SendUpdateDecoder(newVideoAlg, newVideoBitRate, newVideoResolution,
		                                                                    newVideoQCifFrameRate, newVideoCifFrameRate, newVideo4CifFrameRate,
		                                                                    newMBPS, newFS, newSampleAspectRatio, newStaticMB, m_backgroundImageID,
		                                                                    m_isVideoClarityEnabled, m_videoConfType, newMaxDPB, m_eTelePresenceMode, newAutoBrightness,
		                                                                    newVideoVGAFrameRate, newVideoSVGAFrameRate, newVideoXGAFrameRate, E_VIDEO_DECODER_NORMAL, newVideoFrameRate, newProfile,
		                                                                    newPacketPayloadFormat, bIsTipMode, m_isH263Plus, isCallGeneratorConf, m_msftSvcParamsStruct.ssrc, m_msftSvcParamsStruct.nHeight, m_msftSvcParamsStruct.nWidth);

		DWORD oldResolutionRatio = ((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(m_videoAlg, m_videoResolution, m_FS, m_MBPS, m_videoConfType);
		DWORD newResolutionRatio = ((CVideoHardwareInterface*)m_pHardwareInterface)->TranslateToVideoResolutionRatio(newVideoAlg, newVideoResolution, newFS, newMBPS, m_videoConfType);

		m_videoAlg         = newVideoAlg;
		m_videoBitRate     = newVideoBitRate;
		m_isAutoBrightness = newAutoBrightness;
		m_isH263Plus       = pBridgePartyVideoParams->GetIsH263Plus();

		if (m_videoAlg == H264 || m_videoAlg == MS_SVC)
		{
			m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
			m_videoCifFrameRate   = eVideoFrameRateDUMMY;
			m_video4CifFrameRate  = eVideoFrameRateDUMMY;
			m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
			m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoResolution     = eVideoResolutionDummy;
			m_MBPS                = newMBPS;
			m_FS                  = newFS;
			m_staticMB            = newStaticMB;
			m_profile             = newProfile;
			m_packetPayloadFormat = newPacketPayloadFormat;
			m_maxDPB              = newMaxDPB;
			m_bIsTipMode          = bIsTipMode;
			m_videoFrameRate      = newVideoFrameRate;
		}
		else if (m_videoAlg == VP8)
		{
			m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
			m_videoCifFrameRate   = eVideoFrameRateDUMMY;
			m_video4CifFrameRate  = eVideoFrameRateDUMMY;
			m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
			m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoResolution     = eVideoResolutionDummy;
			m_MBPS                = newMBPS;
			m_FS                  = newFS;
			m_staticMB            = newStaticMB;
			m_profile             = newProfile;
			m_packetPayloadFormat = newPacketPayloadFormat;
			m_maxDPB              = newMaxDPB;
			m_bIsTipMode          = bIsTipMode;
			m_videoFrameRate      = newVideoFrameRate;
		}
		else if (m_videoAlg == RTV)
		{
			m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
			m_videoCifFrameRate   = eVideoFrameRateDUMMY;
			m_video4CifFrameRate  = eVideoFrameRateDUMMY;
			m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
			m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoResolution     = eVideoResolutionDummy;
			m_MBPS                = newMBPS;
			m_FS                  = newFS;
			m_staticMB            = DEFAULT_STATIC_MB;
			m_profile             = eVideoProfileDummy;
			m_packetPayloadFormat = newPacketPayloadFormat;
		}
		else
		{
			m_videoQcifFrameRate  = newVideoQCifFrameRate;
			m_videoCifFrameRate   = newVideoCifFrameRate;
			m_video4CifFrameRate  = newVideo4CifFrameRate;
			m_videoVGAFrameRate   = newVideoVGAFrameRate;
			m_videoSVGAFrameRate  = newVideoSVGAFrameRate;
			m_videoXGAFrameRate   = newVideoXGAFrameRate;
			m_videoResolution     = newVideoResolution;
			m_MBPS                = INVALID;
			m_FS                  = INVALID;
			m_profile             = eVideoProfileDummy;
			m_packetPayloadFormat = eVideoPacketPayloadFormatDummy;
			m_staticMB            = DEFAULT_STATIC_MB;
			m_maxDPB              = INVALID;
			m_bIsTipMode          = FALSE;
		}

		m_sampleAspectRatio = newSampleAspectRatio;

		// Update the m_pImage as well with the updated parameters

		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - UpdateVideoParams of image as well";
		m_pImage->UpdateVideoParams(m_videoAlg, m_videoResolution, m_MBPS, m_FS);

		ISDEBUGMODE_SET_VAL("TRANSLATOR", 27, oldResolutionRatio, 99)
		if (oldResolutionRatio != newResolutionRatio)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Decoder resolution ratio has changed (Old:" << oldResolutionRatio << ", New:" << newResolutionRatio << ")";
			m_waitForUpdateDecoderAck = TRUE;
			StartTimer(DECODER_UPDATE_PARAM_TOUT, 8*SECOND);
		}
	}

	m_eCascadeMode = newCascadeMode;
	m_pImage->SetCascadeMode(m_eCascadeMode);

	CSegment* pMsg = new CSegment;
	*pMsg << (BYTE)statOK;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
	POBJDELETE(pMsg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoParamsDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnTimerDecoderUpdateCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	DBGPASSERT(m_waitForUpdateDecoderAck);

	if (m_waitForUpdateDecoderAck)
		RequestChangeLayoutFromAvcToSvcTranslators();
	m_waitForUpdateDecoderAck = FALSE;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnTimerDecoderSyncCONNECTED(CSegment* pParam)
{
	// VNGR-7017 "bombing" logger on change layout - change trace to DEBUG level
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pBridgePartyCntl));

	m_pBridgePartyCntl->HandleEvent(NULL, 0, VIDREFRESH);
	StartTimer(VIDEO_DECODER_SYNC_TOUT, VIDEO_DECODER_SYNC_TOUT_VALUE);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnTimerRecurrentIntraRequest(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	// Sending intra request to decoder
	m_pBridgePartyCntl->HandleEvent(NULL,0,VIDREFRESH);

	DWORD decoderRequestIntraToutValue = 0;
	CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("DECODER_RECURRENT_INTRA_REQ_MINUTES", decoderRequestIntraToutValue);
	if (decoderRequestIntraToutValue)
		StartTimer(DECODER_RECURRENT_INTRA_REQ_TIMEOUT, decoderRequestIntraToutValue*60*SECOND);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdateDBLocalVideoSyncState(BYTE isSynced)
{
	CSegment* pSeg = new CSegment;
	*pSeg << (WORD)isSynced;
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_DECODER_SYNC);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::ReSync()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CSegment* pSeg = new CSegment;
	STATUS status  = 101;

	DWORD decoderDetectedModeWidth               = DEFAULT_DECODER_DETECTED_MODE_WIDTH;
	DWORD decoderDetectedModeHeight              = DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
	DWORD decoderDetectedSampleAspectRatioWidth  = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH;
	DWORD decoderDetectedSampleAspectRatioHeight = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT;
	// ms svc parameters - dummy values are used since 'resync' is relevant to VSW, not to SVC.
	DWORD dummy_ssrc = 0;
	DWORD dummy_prId = 0;

	*pSeg << decoderDetectedModeHeight << decoderDetectedModeWidth << decoderDetectedSampleAspectRatioWidth << decoderDetectedSampleAspectRatioHeight << status << dummy_ssrc << dummy_prId;
	DispatchEvent(VIDEO_DECODER_SYNC_IND, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdatePartyInParams(CUpdatePartyVideoInitParams* pUpdatePartyVideoInitParams)
{
	CBridgePartyVideoInParams* pUpdatePartyVideoInitInParams = (CBridgePartyVideoInParams*) (pUpdatePartyVideoInitParams->GetMediaInParams());
	UpdatePartyParams(pUpdatePartyVideoInitInParams);

	char siteName[MAX_SITE_NAME_ARR_SIZE];

	// VNGFE-3127, VNGR-02237 - do not update site name for link to Master/Slave
	siteName[0] = '\0';
	if (m_pBridgePartyCntl->GetCascadeLinkMode() == NONE)
	{
		if ('\0' != *(pUpdatePartyVideoInitInParams->GetSiteName()))
			strcpy_safe(siteName, pUpdatePartyVideoInitInParams->GetSiteName());
		else
			strcpy_safe(siteName, m_pBridgePartyCntl->GetName());

		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", SiteName:" << siteName;
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Site name is not updated for Link to Master/Slave.";
	}

	CDwordBitMask muteMask = ((CBridgePartyVideoInParams*)pUpdatePartyVideoInitParams)->GetMuteMask();

	CRelayMediaStreamList listVideoMediaStreams;
	listVideoMediaStreams.clear();
	m_pImage = new CImage(
		m_pHardwareInterface->GetConnectionId(), m_pHardwareInterface->GetPartyRsrcId(),
		m_pBridgePartyCntl->GetPartyTaskApp(), siteName, m_pBridgePartyCntl->GetName(),
		DEFAULT_DECODER_DETECTED_MODE_WIDTH, DEFAULT_DECODER_DETECTED_MODE_HEIGHT,
		DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH, DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT,
		m_videoResolution, m_videoAlg, m_FS, m_MBPS, muteMask, INVALID, INVALID, listVideoMediaStreams, m_eCascadeMode);

	m_pImage->SyncLost();
	m_eTelePresenceMode = pUpdatePartyVideoInitInParams->GetTelePresenceMode();
	m_backgroundImageID = pUpdatePartyVideoInitInParams->GetBackgroundImageID();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdateVideoClarity(WORD isVideoClarityEnabled)
{
	CSegment* pSeg = new CSegment;
	*pSeg << isVideoClarityEnabled;
	DispatchEvent(UPDATE_VIDEO_CLARITY,pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoClarityIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateVideoClarity(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoClaritySETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BYTE oldVideoClarity = m_isVideoClarityEnabled;
	OnVideoBridgePartyUpdateVideoClarity(pParam);
	if(oldVideoClarity != m_isVideoClarityEnabled)
	{
		if(CPObject::IsValidPObjectPtr(m_pWaitingForUpdateParams))
		{
			//We will save the updated video clarity as part of update parameters
			m_pWaitingForUpdateParams->SetIsVideoClarityEnabled(m_isVideoClarityEnabled);
		}
		else
		{
			//We will create update parameters with the video clarity value
			m_pWaitingForUpdateParams = new CBridgePartyVideoInParams(m_videoAlg,
			                                                          m_videoBitRate,
			                                                          m_videoQcifFrameRate,
			                                                          m_videoCifFrameRate,
			                                                          m_video4CifFrameRate,
			                                                          m_videoResolution,
			                                                          m_MBPS,
			                                                          m_FS,
			                                                          m_pImage->GetSiteName(),
			                                                          m_pImage->GetMuteMask(),
			                                                          m_eTelePresenceMode,
			                                                          m_backgroundImageID,
			                                                          m_isAutoBrightness);

			m_pWaitingForUpdateParams->SetPartyCascadeMode(m_eCascadeMode);
			m_pWaitingForUpdateParams->SetIsH263Plus(m_isH263Plus);
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoClarityCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BYTE oldVideoClarity = m_isVideoClarityEnabled;
	OnVideoBridgePartyUpdateVideoClarity(pParam);
	if(oldVideoClarity != m_isVideoClarityEnabled)
	{
		// Tsahi - Call Generator SoftMCU
		BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

		((CVideoHardwareInterface*)m_pHardwareInterface)->SendUpdateDecoder(m_videoAlg,
		                                                                    m_videoBitRate,
		                                                                    m_videoResolution,
		                                                                    m_videoQcifFrameRate,
		                                                                    m_videoCifFrameRate,
		                                                                    m_video4CifFrameRate,
		                                                                    m_MBPS,
		                                                                    m_FS,
		                                                                    m_sampleAspectRatio,
		                                                                    m_staticMB,
		                                                                    m_backgroundImageID,
		                                                                    m_isVideoClarityEnabled,
		                                                                    m_videoConfType,
		                                                                    m_maxDPB,
		                                                                    m_eTelePresenceMode,
		                                                                    m_isAutoBrightness,
		                                                                    eVideoFrameRateDUMMY,
		                                                                    eVideoFrameRateDUMMY,
		                                                                    eVideoFrameRateDUMMY,
		                                                                    E_VIDEO_DECODER_NORMAL,
		                                                                    m_videoFrameRate,
		                                                                    m_profile,
		                                                                    m_packetPayloadFormat,
		                                                                    m_bIsTipMode,
		                                                                    m_isH263Plus,
 											isCallGeneratorConf,
		                                                                    m_msftSvcParamsStruct.ssrc,
		                                                                    m_msftSvcParamsStruct.nHeight,
		                                                                    m_msftSvcParamsStruct.nWidth);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateVideoClarityDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::FillAllocStatus(ALLOC_STATUS_PER_PORT_S& allocStatusPerPort)
{
	allocStatusPerPort.tPortPhysicalId.physical_id.resource_type = ePhysical_video_decoder;
	allocStatusPerPort.tPortPhysicalId.connection_id             = m_pHardwareInterface->GetConnectionId();
	allocStatusPerPort.tPortPhysicalId.party_id                  = m_pHardwareInterface->GetPartyRsrcId();

	DWORD parsingMode = (IsVsw() != TRUE) ? E_PARSING_MODE_CP : E_PARSING_MODE_PSEUDO_VSW;
	((CVideoHardwareInterface*) m_pHardwareInterface)->FillDecoderParams(allocStatusPerPort.tDecoderParam,
	                                                                     m_videoAlg,
	                                                                     m_videoBitRate,
	                                                                     m_videoResolution,
	                                                                     m_videoQcifFrameRate,
	                                                                     m_videoCifFrameRate,
	                                                                     m_video4CifFrameRate,
	                                                                     m_MBPS,
	                                                                     m_FS,
	                                                                     m_sampleAspectRatio,
	                                                                     m_staticMB,
	                                                                     m_backgroundImageID,
	                                                                     m_isVideoClarityEnabled,
	                                                                     m_videoConfType,
	                                                                     parsingMode,
	                                                                     m_maxDPB,
	                                                                     m_eTelePresenceMode,
	                                                                     m_isAutoBrightness,
	                                                                     eVideoFrameRateDUMMY,
	                                                                     eVideoFrameRateDUMMY,
	                                                                     eVideoFrameRateDUMMY,
	                                                                     E_VIDEO_DECODER_NORMAL,
	                                                                     m_profile,
	                                                                     m_packetPayloadFormat,
	                                                                     m_bIsTipMode);

	portStatusEnum portStatus = IsConnected() ? ePortOpen : ePortClose;

	allocStatusPerPort.nPortStatus = portStatus;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateAutoBrightnessIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgePartyUpdateAutoBrightness(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateAutoBrightnessSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	BYTE oldAutoBrightness = m_isAutoBrightness;
	OnVideoBridgePartyUpdateAutoBrightness(pParam);
	if(oldAutoBrightness != m_isAutoBrightness)
	{
		if(CPObject::IsValidPObjectPtr(m_pWaitingForUpdateParams))
		{
			//We will save the updated video clarity as part of update parameters
			m_pWaitingForUpdateParams->SetIsAutoBrightness(m_isAutoBrightness);
		}
		else
		{
			//We will create update parameters with the video clarity value
			m_pWaitingForUpdateParams = new CBridgePartyVideoInParams(m_videoAlg, m_videoBitRate, m_videoQcifFrameRate, m_videoCifFrameRate, m_video4CifFrameRate, m_videoResolution,m_MBPS, m_FS, m_pImage->GetSiteName(),m_pImage->GetMuteMask(), m_eTelePresenceMode, m_backgroundImageID,m_isAutoBrightness);
		}
		m_pWaitingForUpdateParams->SetIsH263Plus(m_isH263Plus);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateAutoBrightnessCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	BYTE oldAutoBrightness = m_isAutoBrightness;
	OnVideoBridgePartyUpdateAutoBrightness(pParam);
	if(oldAutoBrightness != m_isAutoBrightness)
	{
		// Tsahi - Call Generator SoftMCU
		BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

		((CVideoHardwareInterface*)m_pHardwareInterface)->SendUpdateDecoder(m_videoAlg,
		                                                                    m_videoBitRate,
		                                                                    m_videoResolution,
		                                                                    m_videoQcifFrameRate,
		                                                                    m_videoCifFrameRate,
		                                                                    m_video4CifFrameRate,
		                                                                    m_MBPS,
		                                                                    m_FS,
		                                                                    m_sampleAspectRatio,
		                                                                    m_staticMB,
		                                                                    m_backgroundImageID,
		                                                                    m_isVideoClarityEnabled,
		                                                                    m_videoConfType,
		                                                                    m_maxDPB,
		                                                                    m_eTelePresenceMode,
		                                                                    m_isAutoBrightness,
		                                                                    eVideoFrameRateDUMMY,
		                                                                    eVideoFrameRateDUMMY,
		                                                                    eVideoFrameRateDUMMY,
		                                                                    E_VIDEO_DECODER_NORMAL,
		                                                                    m_videoFrameRate,
		                                                                    m_profile,
		                                                                    m_packetPayloadFormat,
		                                                                    m_bIsTipMode,
		                                                                    m_isH263Plus,isCallGeneratorConf,
		                                                                    m_msftSvcParamsStruct.ssrc,
		                                                                    m_msftSvcParamsStruct.nHeight,
		                                                                    m_msftSvcParamsStruct.nWidth);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyUpdateAutoBrightnessDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Event is ignored";
}
//
//// ------------------------------------------------------------
//void CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsIDLE(CSegment* pParam)
//{
//	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//
//	CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffects(pParam);
//}
//
//// ------------------------------------------------------------
//void CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsSETUP(CSegment* pParam)
//{
//	TRACEINTO <<  m_pBridgePartyCntl->GetFullName();
//
//	CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffects(pParam);
//}
//
//// ------------------------------------------------------------
//void CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsCONNECTED(CSegment* pParam)
//{
//	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//
//	CVisualEffectsParams visualEffects;
//	visualEffects.DeSerialize(NATIVE, *pParam);
//
//	BYTE isChanged = TRUE;
//
//	if(m_pPartyVisualEffects)
//	{
//		if (visualEffects == (*m_pPartyVisualEffects))
//			isChanged = FALSE;
//	}
//	else
//	{
//		m_pPartyVisualEffects = new CVisualEffectsParams();
//		TRACEINTO << "m_pPartyVisualEffects is NULL, Allocate it- " << m_pBridgePartyCntl->GetFullName();
//
//	}
//
//	if (isChanged)
//	{
//
//		if (m_backgroundImageID != visualEffects.GetBackgroundImageID())
//		{
//			m_backgroundImageID = visualEffects.GetBackgroundImageID();
//			*m_pPartyVisualEffects = visualEffects;
//			((CVideoHardwareInterface*)m_pHardwareInterface)->SendUpdateDecoder(m_videoAlg,
//					m_videoBitRate,
//					m_videoResolution,
//					m_videoQcifFrameRate,
//					m_videoCifFrameRate,
//					m_video4CifFrameRate,
//					m_MBPS,
//					m_FS,
//					m_sampleAspectRatio,
//					m_staticMB,
//					m_backgroundImageID,
//					m_isVideoClarityEnabled,
//					m_videoConfType,
//					m_maxDPB,
//					m_eTelePresenceMode,
//					m_isAutoBrightness,
//					eVideoFrameRateDUMMY,
//					eVideoFrameRateDUMMY,
//					eVideoFrameRateDUMMY,
//					E_VIDEO_DECODER_NORMAL,
//					m_videoFrameRate,
//					m_profile,
//					m_packetPayloadFormat,
//					m_bIsTipMode);
//		}
//		else
//		{
//			*m_pPartyVisualEffects = visualEffects;
//		}
//	}
//}
////--------------------------------------------------------------------------
//void CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffects(CSegment* pParam)
//{
//	TRACEINTO <<  m_pBridgePartyCntl->GetFullName();
//	CVisualEffectsParams visualEffects;
//	visualEffects.DeSerialize(NATIVE, *pParam);
//
//
//	if(!m_pPartyVisualEffects)
//	{
//		m_pPartyVisualEffects = new CVisualEffectsParams();
//		TRACEINTO << "CBridgePartyVideoIn::::OnVideoBridgePartyChangeVisualEffects - m_pPartyVisualEffects is NULL, Allocate it - " << m_pBridgePartyCntl->GetFullName();
//	}
//
//	if (m_backgroundImageID != visualEffects.GetBackgroundImageID())
//	{
//		m_backgroundImageID = visualEffects.GetBackgroundImageID();
//	}
//	*m_pPartyVisualEffects = visualEffects;
//}
//// ------------------------------------------------------------
//void CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceMode(CSegment* pParam)
//{
//	WORD tmpITelePresenceModeType = eTelePresencePartyNone;
//	*pParam >> tmpITelePresenceModeType;
//	if ((eTelePresencePartyType)tmpITelePresenceModeType != m_eTelePresenceMode)
//		m_eTelePresenceMode = (eTelePresencePartyType)tmpITelePresenceModeType;
//}
//// ------------------------------------------------------------
//void CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeIDLE(CSegment* pParam)
//{
//	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
//}
//// ------------------------------------------------------------
//void CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeSETUP(CSegment* pParam)
//{
//	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//	BYTE oldTelePresenceMode = m_eTelePresenceMode;
//	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
//	if (oldTelePresenceMode != m_eTelePresenceMode)
//	{
//		if (m_isPortOpened)  // means we got Ack on ConnectReq and already send the open request- we need to update the encoder in update request. In the other case the updated value will be send part of the open request cause it wasnt sent yet
//		{
//			if (CPObject::IsValidPObjectPtr(m_pWaitingForUpdateParams))
//			{
//				// We will save the updated TelePresence mode as part of update parameters
//				((CBridgePartyVideoInParams*)m_pWaitingForUpdateParams)->SetTelePresenceMode(m_eTelePresenceMode);
//			}
//			else
//			{
//				// We will create update parameters with the updated TelePresenceMode value
//				m_pWaitingForUpdateParams = new CBridgePartyVideoInParams(m_videoAlg,
//						m_videoBitRate,
//						m_videoQcifFrameRate,
//						m_videoCifFrameRate,
//						m_video4CifFrameRate,
//						m_videoResolution,
//						m_MBPS,
//						m_FS,
//						m_pImage->GetSiteName(),
//						m_pImage->GetMuteMask(),
//						m_eTelePresenceMode,
//						m_backgroundImageID,
//						m_isAutoBrightness);
//			}
//		}
//	}
//}
//// ------------------------------------------------------------
//void CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeCONNECTED(CSegment* pParam)
//{
//	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//	SaveAndSendUpdatePartyTelePresenceMode(pParam);
//}
//// ------------------------------------------------------------
//void CBridgePartyVideoIn::SaveAndSendUpdatePartyTelePresenceMode(CSegment* pParam)
//{
//	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//
//	BYTE oldTelePresenceMode = m_eTelePresenceMode;
//	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
//	if (oldTelePresenceMode != m_eTelePresenceMode)
//	{
//		if (m_isPortOpened)
//		{
//			((CVideoHardwareInterface*)m_pHardwareInterface)->SendUpdateDecoder(m_videoAlg,
//							m_videoBitRate,
//							m_videoResolution,
//							m_videoQcifFrameRate,
//							m_videoCifFrameRate,
//							m_video4CifFrameRate,
//							m_MBPS,
//							m_FS,
//							m_sampleAspectRatio,
//							m_staticMB,
//							m_backgroundImageID,
//							m_isVideoClarityEnabled,
//							m_videoConfType,
//							m_maxDPB,
//							m_eTelePresenceMode,
//							m_isAutoBrightness,
//							eVideoFrameRateDUMMY,
//							eVideoFrameRateDUMMY,
//							eVideoFrameRateDUMMY,
//							E_VIDEO_DECODER_NORMAL,
//							m_videoFrameRate,
//							m_profile,
//							m_packetPayloadFormat,
//							m_bIsTipMode);
//				}
//	}
//}

//--------------------------------------------------------------------------
CBridgePartyCntl* CBridgePartyVideoIn::GetBridgePartyCntlPtr()const
{
	return m_pBridgePartyCntl;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::AvcToSvcTranslatorConnected( EStat status )
{
	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)status;
	DispatchEvent(AVC_SVC_TRANSLATOR_CONNECTED, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::UpdateLastReqUponTranslatorError( DWORD lastRequestId, DWORD lastReqOp)
{
	m_lastReqId = lastRequestId;
	m_lastReq   = lastReqOp;
}

//--------------------------------------------------------------------------
//void CBridgePartyVideoIn::UpdateVideoBridgeOnNonRelayImageAvcToSvcTranslated()
//{
	// here we prefer to send the update when all encoders are ready (assuming all become ready in about the same time
	// in order to save additional "change layout"
//	int encodersCount = 0;
//	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
//		if (NULL != m_pAvcToSvcTranslator[i])
//			if (m_pAvcToSvcTranslator[i]->IsInConnectingStage())
//				encodersCount++;
//
//	if (encodersCount > 1 && m_pAvcToSvcParams)
//	{
//		if (m_pAvcToSvcParams->GetEncodersConnectedCount() == 0)	// means this is the first encoder who is ready
//		{
//			// we will start timer, and then we send the update, hoping till then the other encoders will be ready
//			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", ConnectedCount:0 - Start timer AVCSVC_ENCODER_READY_WAIT_TIMER";
//			m_pAvcToSvcParams->IncEncodersConnectedCount();
//			StartTimer( AVCSVC_ENCODER_READY_WAIT_TIMER, AVCSVC_ENCODER_READY_WAIT_TIMER_VALUE );
//			return;
//		}
//		else
//		{
//			// this is the second encoder that is ready, need to delete the timer initiated by the first encoder in case the timer was not "jump" yet
//			DeleteTimer(AVCSVC_ENCODER_READY_WAIT_TIMER);
//			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", ConnectedCount>0 - Delete timer AVCSVC_ENCODER_READY_WAIT_TIMER";
//		}
//	}
//
//	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//
//	((CVideoBridgePartyCntl*)m_pBridgePartyCntl)->UpdateVideoBridgeOnNonRelayImageAvcToSvcTranslated();
//}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::NotifyOnRemovedAvcSvcVideoStreams( DWORD partyRsrcId, DWORD ssrc )
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	((CVideoBridgeCP*) m_pBridgePartyCntl->GetBridge())->NotifyOnRemovedVideoInStreams( partyRsrcId, ssrc );
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::AvcToSvcTranslatorDisconnected(EStat receivedStatus, DWORD encoderConnectionId, bool bIsVswStream)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	if (false == bIsVswStream)
		RemoveAvcToSvcStateMachineFromRoutingTable( encoderConnectionId );
	StartTimer( AVCSVC_KILL_TRANSLATOR_TIMER_0, 0 );	// just to DELETE the Translator Object (timer-time=0, do it immediately)

	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)receivedStatus;
	*pSeg  << (DWORD)encoderConnectionId;
	DispatchEvent(AVC_SVC_TRANSLATOR_DISCONNECTED, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnAvcToSvcTraslatorConnectedSETUP(CSegment* pParam)
{
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;

	if (statOK != receivedStatus)
	{
		TRACEINTO << " - status not OK, closing Video IN, Party Name: " << m_pBridgePartyCntl->GetFullName();
		if (!m_bTranslatorReportedOnConnectionError)
		{
			InformConnectEndedWithError( receivedStatus );
			m_bTranslatorReportedOnConnectionError = true;
		}
		if (m_bNeedToReplayUpgradeAvcToSvcTranslator)	// this in case the upgrade request come in SETUP state
			ReplayUpgradeAvcToSvcTranslate( receivedStatus );
	}
	else
	{
		bool bAllTranslatorsConnected = IsAllTranslatorsConnectedOK();
		if (bAllTranslatorsConnected)
		{
			AllTranslatorsUpdateImage();					// each translator updates the image with its parameters
			if (m_bNeedToReplayUpgradeAvcToSvcTranslator)
				ReplayUpgradeAvcToSvcTranslate( statOK );	// this in case the upgrade request come in SETUP state
		}

		if (m_DecoderConnected && bAllTranslatorsConnected)
			SetVideoInConnected();
		else
		{
			TRACEINTO << " - Not all connected yet, Party Name: " << m_pBridgePartyCntl->GetFullName();
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnAvcToSvcTraslatorConnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;

	if (statOK != receivedStatus)
	{
		if (m_bNeedToReplayUpgradeAvcToSvcTranslator)	// this can be done even before receiving all translators
			ReplayUpgradeAvcToSvcTranslate( receivedStatus );
		DisconnectAvcToSvcTranslators();
	}
	else
	{
		if (IsAllTranslatorsConnectedOK())
		{
			if (m_bNeedToReplayUpgradeAvcToSvcTranslator)
				ReplayUpgradeAvcToSvcTranslate( statOK );
			AllTranslatorsUpdateImage();// UpdateImage
			((CVideoBridgePartyCntl*)m_pBridgePartyCntl)->UpdateVideoBridgeOnNonRelayImageAvcToSvcTranslated();
		}
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::AvcToSvcVswRequestIntraFromEP()
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	// ask for fast update from ep every 1 second, until sync ind received from decode
	m_pBridgePartyCntl->HandleEvent(NULL, 0, VIDREFRESH);

}

// ------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffects(pParam);
}

// ------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsSETUP(CSegment* pParam)
{
	TRACEINTO <<  m_pBridgePartyCntl->GetFullName();

	CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffects(pParam);
}

// ------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffectsCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	CVisualEffectsParams visualEffects;
	visualEffects.DeSerialize(NATIVE, *pParam);

	BYTE isChanged = TRUE;

	if(m_pPartyVisualEffects)
	{
		if (visualEffects == (*m_pPartyVisualEffects))
			isChanged = FALSE;
	}
	else
	{
		m_pPartyVisualEffects = new CVisualEffectsParams();
		TRACEINTO << "m_pPartyVisualEffects is NULL, Allocate it- " << m_pBridgePartyCntl->GetFullName();

	}

	if (isChanged)
	{

		if (m_backgroundImageID != visualEffects.GetBackgroundImageID())
		{
			m_backgroundImageID = visualEffects.GetBackgroundImageID();
			*m_pPartyVisualEffects = visualEffects;
			((CVideoHardwareInterface*)m_pHardwareInterface)->SendUpdateDecoder(m_videoAlg,
					m_videoBitRate,
					m_videoResolution,
					m_videoQcifFrameRate,
					m_videoCifFrameRate,
					m_video4CifFrameRate,
					m_MBPS,
					m_FS,
					m_sampleAspectRatio,
					m_staticMB,
					m_backgroundImageID,
					m_isVideoClarityEnabled,
					m_videoConfType,
					m_maxDPB,
					m_eTelePresenceMode,
					m_isAutoBrightness,
					eVideoFrameRateDUMMY,
					eVideoFrameRateDUMMY,
					eVideoFrameRateDUMMY,
					E_VIDEO_DECODER_NORMAL,
					m_videoFrameRate,
					m_profile,
					m_packetPayloadFormat,
					m_bIsTipMode,
                    m_msftSvcParamsStruct.ssrc,
                    m_msftSvcParamsStruct.nHeight,
                    m_msftSvcParamsStruct.nWidth);
		}
		else
		{
			*m_pPartyVisualEffects = visualEffects;
		}
	}
}
//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgePartyChangeVisualEffects(CSegment* pParam)
{
	TRACEINTO <<  m_pBridgePartyCntl->GetFullName();
	CVisualEffectsParams visualEffects;
	visualEffects.DeSerialize(NATIVE, *pParam);


	if(!m_pPartyVisualEffects)
	{
		m_pPartyVisualEffects = new CVisualEffectsParams();
		TRACEINTO << "m_pPartyVisualEffects is NULL, Allocate it - " << m_pBridgePartyCntl->GetFullName();
	}

	if (m_backgroundImageID != visualEffects.GetBackgroundImageID())
	{
		m_backgroundImageID = visualEffects.GetBackgroundImageID();
	}
	*m_pPartyVisualEffects = visualEffects;
}
// ------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceMode(CSegment* pParam)
{
	WORD tmpITelePresenceModeType = eTelePresencePartyNone;
	*pParam >> tmpITelePresenceModeType;
	if ((eTelePresencePartyType)tmpITelePresenceModeType != m_eTelePresenceMode)
		m_eTelePresenceMode = (eTelePresencePartyType)tmpITelePresenceModeType;
}
// ------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
}
// ------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	BYTE oldTelePresenceMode = m_eTelePresenceMode;
	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
	if (oldTelePresenceMode != m_eTelePresenceMode)
	{
		if (m_isPortOpened)  // means we got Ack on ConnectReq and already send the open request- we need to update the encoder in update request. In the other case the updated value will be send part of the open request cause it wasnt sent yet
		{
			if (CPObject::IsValidPObjectPtr(m_pWaitingForUpdateParams))
			{
				// We will save the updated TelePresence mode as part of update parameters
				((CBridgePartyVideoInParams*)m_pWaitingForUpdateParams)->SetTelePresenceMode(m_eTelePresenceMode);
			}
			else
			{
				// We will create update parameters with the updated TelePresenceMode value
				m_pWaitingForUpdateParams = new CBridgePartyVideoInParams(m_videoAlg,
						m_videoBitRate,
						m_videoQcifFrameRate,
						m_videoCifFrameRate,
						m_video4CifFrameRate,
						m_videoResolution,
						m_MBPS,
						m_FS,
						m_pImage->GetSiteName(),
						m_pImage->GetMuteMask(),
						m_eTelePresenceMode,
						m_backgroundImageID,
						m_isAutoBrightness);
			}
		}
	}
}
// ------------------------------------------------------------
void CBridgePartyVideoIn::OnVideoBridgeUpdatePartyTelePresenceModeCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	SaveAndSendUpdatePartyTelePresenceMode(pParam);
}
// ------------------------------------------------------------
void CBridgePartyVideoIn::SaveAndSendUpdatePartyTelePresenceMode(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BYTE oldTelePresenceMode = m_eTelePresenceMode;
	OnVideoBridgeUpdatePartyTelePresenceMode(pParam);
	if (oldTelePresenceMode != m_eTelePresenceMode)
	{
		if (m_isPortOpened)
		{
			((CVideoHardwareInterface*)m_pHardwareInterface)->SendUpdateDecoder(m_videoAlg,
					m_videoBitRate,
					m_videoResolution,
					m_videoQcifFrameRate,
					m_videoCifFrameRate,
					m_video4CifFrameRate,
					m_MBPS,
					m_FS,
					m_sampleAspectRatio,
					m_staticMB,
					m_backgroundImageID,
					m_isVideoClarityEnabled,
					m_videoConfType,
					m_maxDPB,
					m_eTelePresenceMode,
					m_isAutoBrightness,
					eVideoFrameRateDUMMY,
					eVideoFrameRateDUMMY,
					eVideoFrameRateDUMMY,
					E_VIDEO_DECODER_NORMAL,
					m_videoFrameRate,
					m_profile,
					m_packetPayloadFormat,
					m_bIsTipMode,
                    m_msftSvcParamsStruct.ssrc,
                    m_msftSvcParamsStruct.nHeight,
                    m_msftSvcParamsStruct.nWidth);
		}
	}
}


//--------------------------------------------------------------------------
bool CBridgePartyVideoIn::IsAllTranslatorsConnectedOK()
{
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (NULL != m_pAvcToSvcTranslator[i])
			if (!m_pAvcToSvcTranslator[i]->IsConnected())
				return false;
	return true;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::ConnectAvcToSvcTranslators()
{
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (NULL != m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->Connect();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::DisconnectAvcToSvcTranslators()
{
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (NULL != m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->Disconnect();
}

////--------------------------------------------------------------------------
//void CBridgePartyVideoIn::InformAvcToSvcTranslatorsDecoderSyncInd()
//{
//	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
//		if (NULL != m_pAvcToSvcTranslator[i])
//			m_pAvcToSvcTranslator[i]->Disconnect();
//}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::AllTranslatorsUpdateImage()
{
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (NULL != m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->UpdateImage();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnAvcToSvcTraslatorDisconnectedSETUP(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Request ignored";
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnAvcToSvcTraslatorDisconnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Need to treat this situation";

	EStat receivedStatus = statOK;
	DWORD encoderConnectionId = -1;
	*pParam >> (BYTE&)receivedStatus;
	*pParam >> (DWORD&)encoderConnectionId;

	SetCloseAvcSvcTranslatorStatus( receivedStatus );	// set the status from this translator (internally updated only if != OK)
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnAvcToSvcTraslatorDisconnectedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	EStat receivedStatus = statOK;
	DWORD encoderConnectionId = -1;
	*pParam >> (BYTE&)receivedStatus;
	*pParam >> (DWORD&)encoderConnectionId;
	PASSERT(receivedStatus);

	SetCloseAvcSvcTranslatorStatus( receivedStatus );	// set the status from this translator (internally updated only if != OK)
	CheckAndInformAllClosed();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::RemoveAvcToSvcStateMachineFromRoutingTable( DWORD encoderConnectionId )
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);
	PASSERT_AND_RETURN(!m_pBridgePartyCntl);
	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
	DWORD confRsrcId  = m_pBridgePartyCntl->GetConfRsrcID();

	CRsrcParams encoderRsrcParams( encoderConnectionId, partyRsrcId, confRsrcId );
	int  status = pRoutingTable->RemoveStateMachinePointerFromRoutingTbl( encoderRsrcParams );
	if (status != STATUS_OK)
	{
		TRACEINTO << " AVC-Translator-Error - Party Name: " << m_pBridgePartyCntl->GetFullName()  << ", Status: " << status << "- Failed to remove StateMachine";
	}
}

//--------------------------------------------------------------------------
bool CBridgePartyVideoIn::IsTranslatorAvcSvcExists()
{
	if(IsTranslateAvcToSvcSupported() && m_bIsTranslatorCreated)
		return true;
	return false;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::SendRelayIntraRequestToAvcToSvcTranslator( DWORD ssrc)
{
	if(IsTranslateAvcToSvcSupported() && m_bIsTranslatorCreated)
	{
		for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		{
			if (m_pAvcToSvcTranslator[i])
			{
				if (ssrc == m_pAvcToSvcTranslator[i]->GetSsrc())
				{
					m_pAvcToSvcTranslator[i]->SendRelayIntraRequestToAvcToSvcTranslator();
					return;
				}
			}
		}
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - AVC-Translator-Error - not found ssrc = " << ssrc;
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Failed, Translator not supported or not created";
	}
}

////--------------------------------------------------------------------------
//void CBridgePartyVideoIn::OnAvcToSvcEncoderReadyWaitTimerCONNECTED(CSegment* pParam)
//{
//	TRACEINTO << m_pBridgePartyCntl->GetFullName();
//
//	((CVideoBridgePartyCntl*)m_pBridgePartyCntl)->UpdateVideoBridgeOnNonRelayImageAvcToSvcTranslated();
//}

//--------------------------------------------------------------------------
//void CBridgePartyVideoIn::OnAvcToSvcEncoderReadyWaitTimerANYCASE(CSegment* pParam)
//{
//	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Nothing to do";
//}

/////////////////////////////////////////////////////
bool  CBridgePartyVideoIn::IsMuted() const
{
	bool ans = false;
	if(IsValidPObjectPtr( m_pImage))
	{
		ans = (m_pImage->isMuted())? true :false;
	}
	return ans;

}
//--------------------------------------------------------------------------

CDwordBitMask CBridgePartyVideoIn::GetMuteMask() const
{
	CDwordBitMask muteMask;
	muteMask.ResetMask();
	if(IsValidPObjectPtr( m_pImage))
		muteMask = m_pImage->GetMuteMask();
	return muteMask;

}
//--------------------------------------------------------------------------
bool CBridgePartyVideoIn::IsMuteByOperator()  const
{
	bool ans = false;
	if(IsValidPObjectPtr( m_pImage))
	{
		ans = (m_pImage->IsMuteByOperator())?true:false;
	}
	return ans;
}
//--------------------------------------------------------------------------

bool CBridgePartyVideoIn::IsMuteByMCMS() const
{

	bool ans = false;
	if(IsValidPObjectPtr( m_pImage))
	{
		ans = (m_pImage->IsMuteByMCMS())?true:false;
	}
	return ans;
}

//--------------------------------------------------------------------------

bool CBridgePartyVideoIn::IsMuteByParty() const
{

	bool ans = false;
	if(IsValidPObjectPtr( m_pImage))
	{
		ans = (m_pImage->IsMuteByParty())?true:false;
	}
	return ans;
}
//--------------------------------------------------------------------------
void CBridgePartyVideoIn::OnAvcToSvcTraslatorKillANYCASE(CSegment* pParam)
{
	bool allTranslatorsClosed = true;
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
	{
		if (m_pAvcToSvcTranslator[i])
		{
			if (m_pAvcToSvcTranslator[i]->IsReadyToBeKilled())
				{ POBJDELETE(m_pAvcToSvcTranslator[i]);}
			else
				allTranslatorsClosed = false;
		}
	}
	if (allTranslatorsClosed)
		m_bIsTranslatorCreated = false;	// sign as no translators, important for the next "upgrade mix translators"
}

/////////////////////////////////////////////////////
void CBridgePartyVideoIn::UpgradeAvcToSvcTranslator( CAvcToSvcParams *pAvcToSvcParams )
{
	TRACEINTO << " - connect AVC to SVC Translator";

	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)pAvcToSvcParams;
	DispatchEvent(UPGRADE_TO_MIX_AVC_SVC, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoIn::OnUpgradeToMixAvcToSvcDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << " - AvcToSvcParams not saved, nothing to do";
}

/////////////////////////////////////////////////////
void CBridgePartyVideoIn::OnUpgradeToMixAvcToSvcIDLE(CSegment* pParam)
{
	SaveUpgradeAvcToSvcTranslatorAndReplyOk(pParam);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoIn::OnUpgradeToMixAvcToSvcSETUP(CSegment* pParam)
{
	SaveUpgradeAvcToSvcTranslatorAndReplyOk(pParam);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoIn::SaveUpgradeAvcToSvcTranslatorAndReplyOk(CSegment* pParam)
{
	TRACEINTO << " - AvcToSvcParams Saved";
	CAvcToSvcParams *pAvcToSvcParams;
	*pParam >> (DWORD&)pAvcToSvcParams;
	SaveAvcToSvcParams( pAvcToSvcParams );
	ReplayUpgradeAvcToSvcTranslate( statOK );
}

/////////////////////////////////////////////////////
void CBridgePartyVideoIn::OnUpgradeToMixAvcToSvcCONNECTED(CSegment* pParam)
{
	CAvcToSvcParams *pAvcToSvcParams;
	*pParam >> (DWORD&)pAvcToSvcParams;

	if (IsTranslatorAvcSvcExists())
	{
		OnUpgradeAvcToSvcTranslatorExistingTranslator();
	}
	else
	{
		OnUpgradeAvcToSvcTranslatorNotExistingTranslator( pAvcToSvcParams );
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoIn::OnUpgradeAvcToSvcTranslatorExistingTranslator()
{
	// xxxAmir: what should we do with the new parameters received now???

	m_bNeedToReplayUpgradeAvcToSvcTranslator = true;
	ConnectAvcToSvcTranslators();	// let the translators response to connect command
}

/////////////////////////////////////////////////////
void CBridgePartyVideoIn::OnUpgradeAvcToSvcTranslatorNotExistingTranslator( CAvcToSvcParams *pAvcToSvcParams )
{
	m_bIsTranslatorCreated = false;
	SaveAvcToSvcParams( pAvcToSvcParams );	// save the new parameters


	//TRACEINTO << " - Test Test Test - need to be removed <======================================";
	m_pAvcToSvcParams->SetIsSupportAvcSvcTranslate( true );
	//TRACEINTO << " - Test Test Test - need to be removed <======================================";


	if (!IsTranslateAvcToSvcSupported())
	{
		TRACEINTO << " - AVC-Translator-Error - Not supported - Party Name: " << m_pBridgePartyCntl->GetFullName();
		ReplayUpgradeAvcToSvcTranslate( statInconsistent );
		return;
	}

	int translatorStatus = TranslateAvcToSvc();	// create the translators
	if (WAIT_FOR_TRANSLATOR == translatorStatus)
	{
		TRACEINTO << " - waiting for AvcToSvcTranslator to connect, Party Name: " << m_pBridgePartyCntl->GetFullName();
		m_bNeedToReplayUpgradeAvcToSvcTranslator = true;
		return;
	}
	if (TRANSLATOR_ERROR == translatorStatus)
	{
		TRACEINTO << " AVC-Translator-Error - Party Name: " << m_pBridgePartyCntl->GetFullName();
		ReplayUpgradeAvcToSvcTranslate( statInconsistent );
		return;	// waiting for the global "Connect Party timer" to jump, stays in SETUP state
	}

	TRACEINTO << " AVC-Translator-Error - Party Name: " << m_pBridgePartyCntl->GetFullName();
	ReplayUpgradeAvcToSvcTranslate( statInconsistent );		// normally should not happened
}


/////////////////////////////////////////////////////
void CBridgePartyVideoIn::ReplayUpgradeAvcToSvcTranslate( EStat status )
{
	TRACEINTO << " Inform AvcToSvcTranslators, status=" << (DWORD)status;
	if (m_pBridgePartyCntl)
		((CVideoBridgePartyCntl*)m_pBridgePartyCntl)->ReplayUpgradeSvcToAvcTranslate( status );
}

/////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoIn::Export()
{
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
	{
		if (m_pAvcToSvcTranslator[i])
		{
			if(m_pAvcToSvcTranslator[i]->IsConnected()||(!m_pAvcToSvcTranslator[i]->IsActive()))
			{
				if (FALSE == m_pAvcToSvcTranslator[i]->IsVswStream())
				{
					DWORD encoderConnectionId = m_pAvcToSvcTranslator[i]->GetEncoderConnectionId();
					RemoveAvcToSvcStateMachineFromRoutingTable( encoderConnectionId );
				}
			}
			else
			{
				TRACEINTO << "We shouldn't move during upgrade";
				PASSERT(100);
			}
		}
	}
}

// ///////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoIn::UnregisterInTask()
{
	TRACEINTO << " Un register In and Translators, Party Mane: " << m_pBridgePartyCntl->GetFullName();

	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->UnregisterInTask();

	CStateMachine::UnregisterInTask();
}

// ------------------------------------------------------------
void CBridgePartyVideoIn::RemoveConfParams ()
{
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->RemoveConfParams();

	CBridgePartyMediaUniDirection::RemoveConfParams();

}


////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoIn::Import()
{
	AddAvcToSvcTranslatorsToRoutingTable();
}

////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoIn::RegisterInTask(CTaskApp* myNewTask)
{
	TRACEINTO << " Register In and Translators, Party Mane: " << m_pBridgePartyCntl->GetFullName();

	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->RegisterInTask(myNewTask);

	CStateMachine::RegisterInTask(myNewTask);
}

////////////////////////////////////////////////////////////
int CBridgePartyVideoIn::AddAvcToSvcTranslatorsToRoutingTable()
{
	if (!m_pBridgePartyCntl) {
		PASSERT(102);
		return STATUS_FAIL;
	}
	DWORD partyRsrcId 	= m_pBridgePartyCntl->GetPartyRsrcID();
	TRACEINTO << " - Party Name: " << m_pBridgePartyCntl->GetFullName() << ", PartyRsrcId: " << partyRsrcId;

	int status = STATUS_OK;
	int enNum = 0;
	for ( ; enNum < MAX_ENCODERS_PER_AVC_TO_SVC; enNum++)
	{
		if (m_pAvcToSvcTranslator[enNum])
		{
			if (FALSE == m_pAvcToSvcTranslator[enNum]->IsVswStream())
			{
				status = AddTranslaytorToRoutingTbl( partyRsrcId, m_pAvcToSvcTranslator[enNum], m_pAvcToSvcTranslator[enNum]->GetEncoderConnectionId() );
				if (status != STATUS_OK)
					break;
			}
		}
	}

	if (status != STATUS_OK)
	{
		for (int i = 0; i < enNum; i++)	// deleting all translators that already created
		{
			if (FALSE == m_pAvcToSvcTranslator[i]->IsVswStream())
				RemoveAvcToSvcStateMachineFromRoutingTable(m_pAvcToSvcTranslator[i]->GetEncoderConnectionId());
		}
		PASSERT(104);
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
int CBridgePartyVideoIn::AddTranslaytorToRoutingTbl( DWORD partyRsrcId, CAvcToSvcTranslator* pAvcToSvcTranslator, ConnectionID encoderConnectionId )
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	if (!pRoutingTable)
		return STATUS_FAIL;

	DWORD confRsrcId  	= m_pBridgePartyCntl->GetConfRsrcID();
	CConfApi* pConfApi  = m_pBridgePartyCntl->GetConfApi();

	std::auto_ptr<CTaskApi> pTaskApiTranslator(new CTaskApi(*pConfApi));
	pTaskApiTranslator->CreateOnlyApi( pConfApi->GetRcvMbx(), pAvcToSvcTranslator );

	CRsrcParams encoderRsrcParams( encoderConnectionId, partyRsrcId, confRsrcId );
	return pRoutingTable->AddStateMachinePointerToRoutingTbl( encoderRsrcParams, pTaskApiTranslator.get() );
}

////////////////////////////////////////////////////////////////////////////
int CBridgePartyVideoIn::UpdateTranslatorsWithNewConfParamsAfterMove( DWORD confRsrcId )
{
	for (int enNum=0; enNum < MAX_ENCODERS_PER_AVC_TO_SVC; enNum++)
		if (m_pAvcToSvcTranslator[enNum])
			m_pAvcToSvcTranslator[enNum]->UpdateNewConfParams( this, confRsrcId );
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CBridgePartyVideoIn::GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)
{
	if (IsConnected())
		isOpenedRsrcMap[eLogical_video_decoder] = true;

	if (IsTranslatorAvcSvcExists()) {
		for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++) {
			if (m_pAvcToSvcTranslator[i] && m_pAvcToSvcTranslator[i]->IsConnected() && (FALSE == m_pAvcToSvcTranslator[i]->IsVswStream() ))
			{
				if (m_pAvcToSvcTranslator[i]->IsConnectToSACEncoder())
				{
					isOpenedRsrcMap[eLogical_relay_avc_to_svc_video_encoder_1] = true;	//180p / 360p
				} else {
					isOpenedRsrcMap[eLogical_relay_avc_to_svc_video_encoder_2] = true;
				}
			}
		}
	}
}

void CBridgePartyVideoIn::Dump() const
{
	if (m_pAvcToSvcParams)
		m_pAvcToSvcParams->Dump();
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->Dump();
}

void CBridgePartyVideoIn::RequestChangeLayoutFromAvcToSvcTranslators()
{
	for (int i = 0; i < MAX_ENCODERS_PER_AVC_TO_SVC; i++)
		if (m_pAvcToSvcTranslator[i])
			m_pAvcToSvcTranslator[i]->RequestChangeLayout();
}

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoInVSW
////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoInVSW::CBridgePartyVideoInVSW():CBridgePartyVideoIn()
{
	m_videoConfType = eVideoConfTypeVSW;
}

//--------------------------------------------------------------------------
CBridgePartyVideoInVSW::~CBridgePartyVideoInVSW()
{
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInVSW::SendOpenDecoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ON(m_isPortOpened);
	BYTE isVsw = YES;
	BYTE isVideoClarityEnabled = NO; //the video clarity feature is relevant only in CP

	// Tsahi - Call Generator SoftMCU
	BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

	m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterface)->SendOpenDecoder(m_videoAlg,
	                                                                                m_videoBitRate,
	                                                                                m_videoResolution,
	                                                                                m_videoQcifFrameRate,
	                                                                                m_videoCifFrameRate,
	                                                                                m_video4CifFrameRate,
	                                                                                m_MBPS,
	                                                                                m_FS,
	                                                                                DEFAULT_SAMPLE_ASPECT_RATIO,
	                                                                                m_staticMB,
	                                                                                isVsw,
	                                                                                m_backgroundImageID,
	                                                                                isVideoClarityEnabled,
	                                                                                m_videoConfType,
	                                                                                m_maxDPB,
	                                                                                E_PARSING_MODE_PSEUDO_VSW,
	                                                                                eTelePresencePartyNone,
	                                                                                FALSE,
	                                                                                eVideoFrameRateDUMMY,
	                                                                                eVideoFrameRateDUMMY,
	                                                                                eVideoFrameRateDUMMY,
	                                                                                E_VIDEO_DECODER_NORMAL,
	                                                                                eVideoFrameRateDUMMY,
	                                                                                eVideoProfileBaseline,
	                                                                                eVideoPacketPayloadFormatSingleUnit,
	                                                                                FALSE,
	                                                                                m_isH263Plus,
	                                                                                isCallGeneratorConf);
	m_lastReq = TB_MSG_OPEN_PORT_REQ;
}


//--------------------------------------------------------------------------
// 1) in vsw we dont ask for intra after decoder connected,
// but only after both decoder and encoder connected and we perform change layout.
// this will prevent intra request block om change layout
// 2) update params not used in vsw - deleted
void CBridgePartyVideoInVSW::OnMplConnectAck(STATUS  status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	CSegment* pSeg = new CSegment;
	if(status != STATUS_OK)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status not OK - send answer to VideoBridge";

		//Add assert to EMA in case of NACK
		//AddFaultAlarm("NACK on Connect video decoder",m_pHardwareInterface->GetPartyRsrcId(),status);
		// Inform BridgePartyCntl of statVideoInOutResourceProblem -> Inorder to initiate Kill port on VideoDecoder VideoEncoder
		*pSeg  << (BYTE)statVideoInOutResourceProblem << (BYTE)eMipIn << (BYTE)eMipStatusFail << (BYTE) eMipConnect;
	}
	else
	{
		m_state = CONNECTED;
		*pSeg << (BYTE)statOK;
		// we mark image as synced although no sync (intra) received yet - to enable successful build layout
		m_pImage->SyncFound();
		UpdateDBLocalVideoSyncState(YES);
	}

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_CONNECTED);
	POBJDELETE(pSeg);
}


PBEGIN_MESSAGE_MAP(CBridgePartyVideoInContent)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS, IDLE,          CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParamsIDLE)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS, SETUP,         CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParamsSETUP)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS, CONNECTED,     CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParamsCONNECTED)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS, DISCONNECTING, CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParamsDISCONNECTING)
PEND_MESSAGE_MAP(CBridgePartyVideoInContent,CBridgePartyVideoIn);

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoInContent
////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoInContent::CBridgePartyVideoInContent():CBridgePartyVideoIn()
{
}

//--------------------------------------------------------------------------
CBridgePartyVideoInContent::~CBridgePartyVideoInContent()
{
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::SendOpenDecoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);

	ON(m_isPortOpened);
	BYTE isVsw = NO;

	// Tsahi - Call Generator SoftMCU
	BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

	m_lastReqId = ((CVideoHardwareInterface*)m_pHardwareInterface)->SendOpenDecoder(m_videoAlg,
	                                                                                m_videoBitRate,
	                                                                                m_videoResolution,
	                                                                                m_videoQcifFrameRate,
	                                                                                m_videoCifFrameRate,
	                                                                                m_video4CifFrameRate,
	                                                                                m_MBPS,
	                                                                                m_FS,
	                                                                                m_sampleAspectRatio,
	                                                                                m_staticMB,
	                                                                                isVsw,
	                                                                                m_backgroundImageID,
	                                                                                m_isVideoClarityEnabled,
	                                                                                m_videoConfType,
	                                                                                m_maxDPB,
	                                                                                E_PARSING_MODE_CP,
	                                                                                m_eTelePresenceMode,
	                                                                                FALSE,
	                                                                                m_videoVGAFrameRate,
	                                                                                m_videoSVGAFrameRate,
	                                                                                m_videoXGAFrameRate,
	                                                                                E_VIDEO_DECODER_CONTENT,
	                                                                                eVideoFrameRateDUMMY,
	                                                                                m_profile,
	                                                                                m_packetPayloadFormat,
	                                                                                m_bIsTipMode,
	                                                                                m_isH263Plus,
	                                                                                isCallGeneratorConf);
	m_lastReq = TB_MSG_OPEN_PORT_REQ;
}

//--------------------------------------------------------------------------
// In Legacy:
// 1) Ack On OPEN port means that the decoder was opened and ALSO connected.
// 2) The stream that have to send the intra is ContentSpeaker ==>
//    we have to ask Conference to ask ContentBridge for intra.
void CBridgePartyVideoInContent::OnMplOpenPortAck(STATUS  status)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;
	if (status != STATUS_OK)
	{
		InformConnectEndedWithError( statVideoInOutResourceProblem );
		return;
	}

	// not doing CONNECT in case of Content Decoder
	SetVideoInConnected();
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::OnMplDecoderSyncCONNECTED(CSegment* pParam)
{
	STATUS status;
	DWORD  decoderDetectedModeWidth               = DEFAULT_DECODER_DETECTED_MODE_WIDTH;
	DWORD  decoderDetectedModeHeight              = DEFAULT_DECODER_DETECTED_MODE_HEIGHT;
	DWORD  decoderDetectedSampleAspectRatioWidth  =  DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH;
	DWORD  decoderDetectedSampleAspectRatioHeight = DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT;
	DWORD  decoderMSSvcSsrcID = INVALID;
	DWORD  decoderMSSvcPriorityID   = INVALID;
	
	*pParam >>  decoderDetectedModeWidth >> decoderDetectedModeHeight >> decoderDetectedSampleAspectRatioWidth >> decoderDetectedSampleAspectRatioHeight >> status;

	TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Status:" << status;

	if (status != STATUS_OK)
	{
		if (IsValidTimer(VIDEO_DECODER_SYNC_TOUT))
			return;

		StartTimer(VIDEO_DECODER_SYNC_TOUT, VIDEO_DECODER_SYNC_TOUT_VALUE);
	}
	else
	{
		if (m_pImage->isSyncLost()) // initial synchronization of party
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Initial SYNC"
			          << ", decoderDetectedModeWidth:"               << decoderDetectedModeWidth
			          << ", decoderDetectedModeHeight:"              << decoderDetectedModeHeight
			          << ", decoderDetectedSampleAspectRatioWidth:"  << decoderDetectedSampleAspectRatioWidth
			          << ", decoderDetectedSampleAspectRatioHeight:" << decoderDetectedSampleAspectRatioHeight;

			m_pImage->SyncFound(decoderDetectedModeWidth, decoderDetectedModeHeight, decoderDetectedSampleAspectRatioWidth, decoderDetectedSampleAspectRatioHeight);

			// tell video bridge to enter party to mix

			m_pBridgePartyCntl->HandleEvent(NULL, 0, VIDEO_IN_SYNCED);
		}
		else if ((decoderDetectedModeWidth != m_pImage->GetDecoderDetectedModeWidth()) ||
				(decoderDetectedModeHeight != m_pImage->GetDecoderDetectedModeHeight()) ||
				(decoderDetectedSampleAspectRatioWidth != m_pImage->GetDecoderDetectedSampleAspectRatioWidth()) ||
				(decoderDetectedSampleAspectRatioHeight != m_pImage->GetDecoderDetectedSampleAspectRatioHeight()))
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName()
    				  << "\n  Old decoder detected mode width                 :" <<  m_pImage->GetDecoderDetectedModeWidth()
    				  << "\n  New decoder detected mode width                 :" << decoderDetectedModeWidth
    				  << "\n  Old decoder detected mode height                :" <<  m_pImage->GetDecoderDetectedModeHeight()
    				  << "\n  New decoder detected mode height                :" << decoderDetectedModeHeight
    				  << "\n  Old decoder detected sample aspect ratio width  :" << m_pImage->GetDecoderDetectedSampleAspectRatioWidth()
    				  << "\n  New decoder detected sample aspect ratio width  :" << decoderDetectedSampleAspectRatioWidth
    				  << "\n  Old decoder detected sample aspect ratio height :" << m_pImage->GetDecoderDetectedSampleAspectRatioHeight()
    				  << "\n  New decoder detected sample aspect ratio height :" << decoderDetectedSampleAspectRatioHeight;

			m_pImage->SetDecoderDetectedParams(decoderDetectedModeWidth, 
				decoderDetectedModeHeight, 
				decoderDetectedSampleAspectRatioWidth, 
				decoderDetectedSampleAspectRatioHeight, 
				decoderMSSvcSsrcID,decoderMSSvcPriorityID);

			m_pBridgePartyCntl->HandleEvent(NULL, 0, UPDATE_DECODER_DETECTED_MODE);
		}
			
		DeleteTimer(VIDEO_DECODER_SYNC_TOUT);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParamsIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();
	OnVideoBridgeConfUpdateVideoParams(pParam);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParamsSETUP(CSegment* pParam)
{
	if(!((CVideoBridgeCPContent*) m_pBridgePartyCntl->GetBridge())->IsXCodeConf())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Should not arrive in this state";
		PASSERT(1);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Allow update of Content Protocol in XCode Conf";
		m_pWaitingForUpdateParams  = new CBridgePartyVideoParams;
		DWORD videoAlg = 0;
		DWORD videoBitRate = 0;
		DWORD contentFs = 0;
		BYTE isContentHD1080Supported;
		*pParam >> videoAlg >> videoBitRate >> isContentHD1080Supported;
		if(isContentHD1080Supported)
			contentFs = H264_HD1080_FS_AS_DEVISION;
		else
			contentFs = H264_HD720_FS_AS_DEVISION;
		m_pWaitingForUpdateParams->SetVideoAlgorithm(videoAlg);
		m_pWaitingForUpdateParams->SetVideoBitRate(videoBitRate);
		m_pWaitingForUpdateParams->SetFS(contentFs);
	}
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParamsCONNECTED(CSegment* pParam)
{
	if(!((CVideoBridgeCPContent*) m_pBridgePartyCntl->GetBridge())->IsXCodeConf())
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Should not arrive in this state";
		PASSERT(1);
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Allow update of Content Protocol in XCode Conf";
		BYTE oldVideoAlg = m_videoAlg;
		OnVideoBridgeConfUpdateVideoParams(pParam);
		if(m_videoAlg != oldVideoAlg)
		{
			TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Allow update of Content Protocol in XCode Conf in Case of ContentProtocol change" << "\n";
			SendUpdateDecoder();

		}
	}
}

//--------------------------------------------------------------------------
void  CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParamsDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - Should not arrive in this state";
	PASSERT(1);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::OnVideoBridgeConfUpdateVideoParams(CSegment* pParam)
{
	BYTE isContentHD1080Supported = FALSE;
	DWORD contentFS   = 0;
	DWORD contentMBPS = 0;
	eVideoResolution videoResolution = eVideoResolutionDummy;

	*pParam >> m_videoAlg >> m_videoBitRate >> isContentHD1080Supported;

	if (isContentHD1080Supported)
	{
		videoResolution = eVideoResolutionHD1080;
		contentFS   = H264_HD1080_FS_AS_DEVISION;
		contentMBPS = 490;
	//	contentMBPS = 245;   //The division operation is heavy(H264_L4_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)/2=245760/500/2=245
							 // we divide by CUSTOM_MAX_MBPS_FACTOR and also by 2 because only frame rate of 15 is supported for content.
	}
	else
	{
		videoResolution = eVideoResolutionSVGA;
		contentFS   = H264_HD720_FS_AS_DEVISION;
		contentMBPS = 108;   //The division operation is heavy(H264_L3_1_DEFAULT_MBPS/CUSTOM_MAX_MBPS_FACTOR)/2=108000/500/2=108
							 // we divide by CUSTOM_MAX_MBPS_FACTOR and also by 2 because only frame rate of 15 is supported for content.
	}

  if (m_videoAlg == H264)
  {
    m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
    m_videoCifFrameRate   = eVideoFrameRateDUMMY;
    m_video4CifFrameRate  = eVideoFrameRateDUMMY;
    m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
    m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
    m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
    m_videoResolution     = videoResolution; //eVideoResolutionDummy;
    m_MBPS                = contentMBPS;
    m_FS                  = contentFS;
    m_sampleAspectRatio   = DEFAULT_SAMPLE_ASPECT_RATIO;
    m_staticMB            = DEFAULT_STATIC_MB;
  }

  else
  {
    m_videoQcifFrameRate  = eVideoFrameRate30FPS;
    m_videoCifFrameRate   = eVideoFrameRate30FPS;
    m_video4CifFrameRate  = eVideoFrameRate15FPS;
    m_videoVGAFrameRate   = eVideoFrameRate15FPS;
    m_videoSVGAFrameRate  = eVideoFrameRate10FPS;
    m_videoXGAFrameRate   = eVideoFrameRate7_5FPS;
    m_videoResolution     = videoResolution;
    m_MBPS                = INVALID;
    m_FS                  = INVALID;
    m_sampleAspectRatio   = DEFAULT_SAMPLE_ASPECT_RATIO;
    m_staticMB            = DEFAULT_STATIC_MB;
    m_maxDPB              = INVALID;
  }

  m_pImage->UpdateVideoParams(m_videoAlg, m_videoResolution, m_MBPS, m_FS);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::UpdateVideoParams(CSegment* pParam)
{
	CSegment* pSeg = new CSegment;
	*pSeg = *pParam;

	DispatchEvent(UPDATE_VIDEO_IN_PARAMS,pSeg);
	POBJDELETE(pSeg);
}
//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	if (!(CPObject::IsValidPObjectPtr(pBridgePartyVideoParams)))
	{
		PASSERTMSG(1, "Internal Error received invalid params");
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE)statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
		POBJDELETE(pMsg);
		return;
	}

	BYTE isContentHD1080Supported = FALSE;
	DWORD contentFS   = 0;
	contentFS = pBridgePartyVideoParams->GetFS();
	DWORD newVideoAlg = pBridgePartyVideoParams->GetVideoAlgorithm();
	DWORD newVideoBitRate = pBridgePartyVideoParams->GetVideoBitRate();

	if(m_videoAlg != newVideoAlg && m_videoAlg > 0 && newVideoAlg > 0)
	{
		if(newVideoAlg == H264)
		{
			if(pBridgePartyVideoParams->GetFS() == H264_HD1080_FS_AS_DEVISION)
				ON(isContentHD1080Supported);
		}

		CSegment* pSeg = new CSegment;
		*pSeg << newVideoAlg << newVideoBitRate << isContentHD1080Supported;
		OnVideoBridgeConfUpdateVideoParams(pSeg);
		SendUpdateDecoder();

		POBJDELETE(pSeg);
	}
}
//--------------------------------------------------------------------------
void CBridgePartyVideoInContent::SendUpdateDecoder()
{
	PASSERT_AND_RETURN(!m_pHardwareInterface);
	BYTE isVsw = NO;

	// Tsahi - Call Generator SoftMCU
	BOOL isCallGeneratorConf = m_pBridgePartyCntl->GetIsCallGeneratorConference();

	((CVideoHardwareInterface*)m_pHardwareInterface)->SendUpdateDecoder(m_videoAlg,
			m_videoBitRate,
			m_videoResolution,
			m_videoQcifFrameRate,
			m_videoCifFrameRate,
			m_video4CifFrameRate,
			m_MBPS,
			m_FS,
			m_sampleAspectRatio,
			m_staticMB,
			m_backgroundImageID,
			m_isVideoClarityEnabled,
			m_videoConfType,
			m_maxDPB,
			m_eTelePresenceMode,
			FALSE,
			m_videoVGAFrameRate,
			m_videoSVGAFrameRate,
			m_videoXGAFrameRate,
			E_VIDEO_DECODER_CONTENT,
			m_videoFrameRate,
			m_profile,
			m_packetPayloadFormat,
			m_bIsTipMode,
			m_isH263Plus,
			isCallGeneratorConf);
	m_waitForUpdateDecoderAck = TRUE;
	StartTimer(DECODER_UPDATE_PARAM_TOUT, 8*SECOND);
}

//--------------------------------------------------------------------------

PBEGIN_MESSAGE_MAP(CBridgePartyVideoInCOP)
PEND_MESSAGE_MAP(CBridgePartyVideoInCOP,CBridgePartyVideoIn);

////////////////////////////////////////////////////////////////////////////
//                        CBridgePartyVideoInCOP
////////////////////////////////////////////////////////////////////////////
CBridgePartyVideoInCOP::CBridgePartyVideoInCOP():CBridgePartyVideoIn()
{
	m_videoConfType = eVideoConfTypeCopHD108025fps;//This is the default if it eVideoConfTypeCopHD72050fps we will receive via the initparams
	m_isVsw = NO;
	m_IsRemoteUseSmartSwitchAccordingToVendor = FALSE;
}

//--------------------------------------------------------------------------
CBridgePartyVideoInCOP::~CBridgePartyVideoInCOP()
{
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::Create(const CBridgePartyCntl* pBridgePartyCntl, const CRsrcParams* pRsrcParams, const CBridgePartyMediaParams* pBridgePartyVideoInParams)
{
	CBridgePartyVideoUniDirection::Create(pBridgePartyCntl, pRsrcParams, pBridgePartyVideoInParams);

	char siteName[MAX_SITE_NAME_ARR_SIZE] = {0};

	// VNGFE-3127, VNGR-02237 - do not update site name for link to Master/Slave
	if (pBridgePartyCntl->GetCascadeLinkMode() == NONE)
	{
		if ('\0' != *((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetSiteName())
			strcpy_safe(siteName, ((CBridgePartyVideoInParams*) pBridgePartyVideoInParams)->GetSiteName());
		else
			strcpy_safe(siteName, m_pBridgePartyCntl->GetName());

		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", SiteName:" << siteName;
	}
	else
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << ", Site name is not updated for Link to Master/Slave.";
	}

	CDwordBitMask muteMask = ((CBridgePartyVideoInParams*) pBridgePartyVideoInParams)->GetMuteMask();
	CRelayMediaStreamList listVideoMediaStreams;
	m_pImage = new CImage(
		pRsrcParams->GetConnectionId(), pRsrcParams->GetPartyRsrcId(),
		m_pBridgePartyCntl->GetPartyTaskApp(), siteName, m_pBridgePartyCntl->GetName(),
		DEFAULT_DECODER_DETECTED_MODE_WIDTH, DEFAULT_DECODER_DETECTED_MODE_HEIGHT,
		DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH, DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT,
		m_videoResolution, m_videoAlg, m_FS, m_MBPS, muteMask, INVALID, INVALID, listVideoMediaStreams, m_eCascadeMode);

	m_backgroundImageID                       = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetBackgroundImageID();
	m_videoConfType                           = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetVideoConfType();
	m_IsRemoteUseSmartSwitchAccordingToVendor = ((CBridgePartyVideoInParams*)pBridgePartyVideoInParams)->GetIsRemoteNeedSmartSwitchAccordingToVendor();
	DWORD dummy;
	GetCurrentCopDecoderResolution(dummy, m_initialDecoderMode);
}

//--------------------------------------------------------------------------
WORD CBridgePartyVideoInCOP::IsValidState(WORD state)const
{
	WORD valid_state = TRUE;
	switch(state)
	{
		case SETUP:
		{
			valid_state = FALSE;
			PASSERTSTREAM(1, "CBridgePartyVideoInCOP::IsValidState - " << m_pBridgePartyCntl->GetFullName());
			break;
		}
	} // switch

	return valid_state;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::SetDecoderConnectionIdInImage(DWORD decoderConnectionId)
{
	m_pImage->SetCopDecoderConnectionID(decoderConnectionId);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::SetDecoderPartyIdInImage(DWORD decoderPartyId)
{
	m_pImage->SetCopDecoderEntityID(decoderPartyId);
}

//--------------------------------------------------------------------------
DWORD CBridgePartyVideoInCOP::GetDecoderConnectionIdInImage()
{
	DWORD connId = INVALID;
	if(m_pImage)
		connId = m_pImage->GetCopDecoderConnectionID();
	return connId;
}

//--------------------------------------------------------------------------
DWORD CBridgePartyVideoInCOP::GetDecoderPartyIdInImage()
{
	DWORD partyId = INVALID;
	if(m_pImage)
		partyId = m_pImage->GetCopDecoderEntityID();
	return partyId;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::SetDspSmartSwitchConnectionId(DWORD decoderConnectionId)
{
	if (m_pImage)
		m_pImage->SetDspSmartSwitchConnectionId(decoderConnectionId);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::SetDspSmartSwitchEntityId(DWORD decoderPartyId)
{
	if (m_pImage)
		m_pImage->SetDspSmartSwitchEntityId(decoderPartyId);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::OnVideoBridgePartyConnectIDLE(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	m_state = CONNECTED;
	SetClosePortAckStatus(statOK);
	CSegment *pMsg = new CSegment;
	*pMsg << (BYTE)statOK;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, VIDEO_IN_CONNECTED);
	POBJDELETE(pMsg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::OnVideoBridgePartyDisConnect(CSegment* pParam)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	BYTE	responseStatus = statOK;
	SetClosePortAckStatus(responseStatus);
	m_state = IDLE;

	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)responseStatus;
	// Inform BridgePartyCntl
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_DISCONNECTED);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
///Keren maybe change the function with new name (just save)
void CBridgePartyVideoInCOP::SaveAndSendUpdatedVideoParams(CBridgePartyVideoParams* pBridgePartyVideoParams)
{
	TRACEINTO << m_pBridgePartyCntl->GetFullName();

	if (!CPObject::IsValidPObjectPtr(pBridgePartyVideoParams))
	{
		PASSERTMSG(1, "Internal Error received invalid params");
		CSegment* pMsg = new CSegment;
		*pMsg << (BYTE)statIllegal;
		m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
		POBJDELETE(pMsg);
		return;
	}

	ECascadePartyType         newCascadeMode                          = pBridgePartyVideoParams->GetPartyCascadeType();
	DWORD                     newVideoAlg                             = pBridgePartyVideoParams->GetVideoAlgorithm();
	DWORD                     newVideoBitRate                         = pBridgePartyVideoParams->GetVideoBitRate();
	eVideoFrameRate           newVideoQCifFrameRate                   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionQCIF);
	eVideoFrameRate           newVideoCifFrameRate                    = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionCIF);
	eVideoFrameRate           newVideo4CifFrameRate                   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolution4CIF);
	eVideoFrameRate           newVideoVGAFrameRate                    = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionVGA);
	eVideoFrameRate           newVideoSVGAFrameRate                   = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionSVGA);
	eVideoFrameRate           newVideoXGAFrameRate                    = pBridgePartyVideoParams->GetVideoFrameRate(eVideoResolutionXGA);
	eVideoResolution          newVideoResolution                      = pBridgePartyVideoParams->GetVideoResolution();
	DWORD                     newMBPS                                 = pBridgePartyVideoParams->GetMBPS();
	DWORD                     newFS                                   = pBridgePartyVideoParams->GetFS();
	DWORD                     newSampleAspectRatio                    = pBridgePartyVideoParams->GetSampleAspectRatio();
	DWORD                     newStaticMB                             = pBridgePartyVideoParams->GetStaticMB();
	DWORD                     newMaxDPB                               = pBridgePartyVideoParams->GetMaxDPB();
	eVideoProfile             profile                                 = pBridgePartyVideoParams->GetProfile();
	eVideoPacketPayloadFormat packetpayloadformat                     = pBridgePartyVideoParams->GetPacketFormat();
	BYTE                      isRemoteUseSmartSwitchAccordingToVendor = pBridgePartyVideoParams->GetIsRemoteNeedSmartSwitchAccordingToVendor();

	if (m_videoAlg == newVideoAlg &&
	    m_videoBitRate == newVideoBitRate &&
	    m_videoQcifFrameRate == newVideoQCifFrameRate &&
	    m_videoCifFrameRate == newVideoCifFrameRate &&
	    m_video4CifFrameRate == newVideo4CifFrameRate &&
	    m_videoVGAFrameRate == newVideoVGAFrameRate &&
	    m_videoSVGAFrameRate == newVideoSVGAFrameRate &&
	    m_videoXGAFrameRate == newVideoXGAFrameRate &&
	    m_videoResolution == newVideoResolution &&
	    m_MBPS == newMBPS &&
	    m_FS == newFS &&
	    m_sampleAspectRatio == newSampleAspectRatio &&
	    m_staticMB == newStaticMB &&
	    m_maxDPB == newMaxDPB &&
	    m_profile == profile &&
	    m_packetPayloadFormat == packetpayloadformat &&
	    m_IsRemoteUseSmartSwitchAccordingToVendor == isRemoteUseSmartSwitchAccordingToVendor)
	{
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - No real change in Params";
	}
	else
	{
		m_videoAlg     = newVideoAlg;
		m_videoBitRate = newVideoBitRate;
		if (m_videoAlg == H264)
		{
			m_videoQcifFrameRate  = eVideoFrameRateDUMMY;
			m_videoCifFrameRate   = eVideoFrameRateDUMMY;
			m_video4CifFrameRate  = eVideoFrameRateDUMMY;
			m_videoVGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoSVGAFrameRate  = eVideoFrameRateDUMMY;
			m_videoXGAFrameRate   = eVideoFrameRateDUMMY;
			m_videoResolution     = eVideoResolutionDummy;
			m_MBPS                = newMBPS;
			m_FS                  = newFS;
			m_staticMB            = newStaticMB;
			m_maxDPB              = newMaxDPB;
			m_profile             = profile;
			m_packetPayloadFormat = packetpayloadformat;
		}
		else
		{
			m_videoQcifFrameRate = newVideoQCifFrameRate;
			m_videoCifFrameRate  = newVideoCifFrameRate;
			m_video4CifFrameRate = newVideo4CifFrameRate;
			m_videoVGAFrameRate  = newVideoVGAFrameRate;
			m_videoSVGAFrameRate = newVideoSVGAFrameRate;
			m_videoXGAFrameRate  = newVideoXGAFrameRate;
			m_videoResolution    = newVideoResolution;
			m_isH263Plus         = pBridgePartyVideoParams->GetIsH263Plus();
			m_MBPS               = INVALID;
			m_FS                 = INVALID;
			m_staticMB           = DEFAULT_STATIC_MB;
			m_maxDPB             = INVALID;
		}

		m_sampleAspectRatio                       = newSampleAspectRatio;
		m_IsRemoteUseSmartSwitchAccordingToVendor = isRemoteUseSmartSwitchAccordingToVendor;

		// Update the m_pImage as well with the updated parameters
		TRACEINTO << m_pBridgePartyCntl->GetFullName() << " - UpdateVideoParams of image as well";
		m_pImage->UpdateVideoParams(m_videoAlg, m_videoResolution, m_MBPS, m_FS);
	}

	m_pImage->SetCascadeMode(m_eCascadeMode);

	CSegment* pMsg = new CSegment;
	*pMsg << (BYTE)statOK;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
	POBJDELETE(pMsg);
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::GetCurrentCopDecoderResolution(DWORD& algorithm, ECopDecoderResolution& copDecoderResolution)
{
	algorithm = m_videoAlg;
	if(m_videoAlg==H264)
	{
		CCopVideoModeTable table;
		table.GetDecoderH264ResolustionFromMbpsFs(copDecoderResolution,m_FS,m_MBPS);
	}
	else
	{
		switch(m_videoResolution)
		{
			case eVideoResolutionQCIF:
			case eVideoResolutionQVGA: // ???
			case eVideoResolutionCIF:
			case eVideoResolutionSIF:
			{
				copDecoderResolution = COP_decoder_resolution_CIF25;
				break;
			}
			case eVideoResolutionVGA:
			case eVideoResolution4SIF:
			case eVideoResolution4CIF:
			{
				copDecoderResolution = COP_decoder_resolution_4CIF25;
				break;
			}
			case eVideoResolution525SD:
			case eVideoResolution625SD:
			{
				copDecoderResolution = COP_decoder_resolution_W4CIF25;
				break;
			}
			case eVideoResolutionSVGA:
			case eVideoResolutionXGA:
			case eVideoResolutionHD720:
			case eVideoResolution16CIF:
			{
				copDecoderResolution = COP_decoder_resolution_HD720p25;
				break;
			}
			case eVideoResolutionHD1080:
			{
				copDecoderResolution = COP_decoder_resolution_HD108030;
				break;
			}
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}//else
	}//switch
}

//--------------------------------------------------------------------------
void  CBridgePartyVideoInCOP::GetInitialCopDecoderResolution(DWORD& algorithm, ECopDecoderResolution& copDecoderResolution)
{
	algorithm = m_videoAlg;
	copDecoderResolution = m_initialDecoderMode;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoInCOP::GetInVidParams(CBridgePartyVideoInParams* pInVideoParams)
{
	pInVideoParams->SetPartyCascadeMode(m_eCascadeMode);
	pInVideoParams->SetVideoAlgorithm(m_videoAlg);
	pInVideoParams->SetVideoBitRate(m_videoBitRate);
	pInVideoParams->SetVideoResolution(m_videoResolution);
	pInVideoParams->SetVideoFrameRate(eVideoResolutionQCIF,m_videoQcifFrameRate);
	pInVideoParams->SetVideoFrameRate(eVideoResolutionCIF,m_videoCifFrameRate);
	pInVideoParams->SetVideoFrameRate(eVideoResolution4CIF,m_video4CifFrameRate);
	pInVideoParams->SetMBPS(m_MBPS);
	pInVideoParams->SetFS(m_FS);
	pInVideoParams->SetSampleAspectRatio(m_sampleAspectRatio);
	pInVideoParams->SetStaticMB(m_staticMB);
	pInVideoParams->SetVideConfType(m_videoConfType);
	pInVideoParams->SetMaxDPB(m_maxDPB);
	pInVideoParams->SetProfile(m_profile);
	pInVideoParams->SetPacketFormat(m_packetPayloadFormat);
	pInVideoParams->SetIsH263Plus(m_isH263Plus);

	DWORD partyId = m_pBridgePartyCntl->GetPartyRsrcID();
	DWORD artConnectionId = DUMMY_CONNECTION_ID;

	CRsrcDesc *pRsrcDesc;
	// Eitan - ISDN party has no rtp - connect MUX instead?
	if (m_pBridgePartyCntl->GetNetworkInterface() == ISDN_INTERFACE_TYPE)
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyId, eLogical_mux);
	else
		pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyId, eLogical_rtp);
	if(pRsrcDesc)
	{
		artConnectionId = pRsrcDesc->GetConnectionId();
	}
	else
	{
		DBGPASSERT(101);
	}

	pInVideoParams->SetCopConnectionId(artConnectionId);
	pInVideoParams->SetCopPartyId(partyId);
}

//--------------------------------------------------------------------------
BYTE CBridgePartyVideoIn::IsVsw()const
{
	return m_isVsw;
}

//--------------------------------------------------------------------------
void CBridgePartyVideoIn::SetVsw(BYTE isVSW)
{
	m_isVsw = isVSW;
}

//--------------------------------------------------------------------------
eVideoResolution CBridgePartyVideoIn::GetResolutionFromDetectedMode(DWORD  decoderDetectedModeWidth,DWORD  decoderDetectedModeHeight)
{
	eVideoResolution videoResolution = eVideoResolutionDummy;
	WORD found_index = 0;
	for (WORD resolution_index = 0; resolution_index < eVideoResolutionLast; resolution_index++)
	{
		if (decoderDetectedModeWidth == ResoltionWidthAndHeight[resolution_index].resWidth && decoderDetectedModeHeight == ResoltionWidthAndHeight[resolution_index].resHeight)
		{
			videoResolution = ResoltionWidthAndHeight[resolution_index].eResolution;
			found_index = resolution_index ;
			break;
		}
	}

	TRACEINTO << m_pBridgePartyCntl->GetFullName()
	          << ", VideoResolution:"           << VideoResolutionAsString[found_index]
	          << ", decoderDetectedModeWidth:"  << decoderDetectedModeWidth
	          << ", decoderDetectedModeHeight:" << decoderDetectedModeHeight;

	return videoResolution;
}

//--------------------------------------------------------------------------
ECopDecoderResolution CBridgePartyVideoInCOP::GetCopDecoderResolutionFromDetectedMode(DWORD  decoderDetectedModeWidth,DWORD  decoderDetectedModeHeight)
{
  ECopDecoderResolution copDecoderResolution = COP_decoder_resolution_Last;
	eVideoResolution videoResolution = GetResolutionFromDetectedMode(decoderDetectedModeWidth,decoderDetectedModeHeight);
  copDecoderResolution = GetCopDecoderResolutionFromVideoResolution(videoResolution);

  return copDecoderResolution;
}

//--------------------------------------------------------------------------
ECopDecoderResolution CBridgePartyVideoInCOP::GetCopDecoderResolutionFromVideoResolution(eVideoResolution videoResolution)
{
	ECopDecoderResolution copDecoderResolution = COP_decoder_resolution_Last;
	switch (videoResolution)
	{
		case eVideoResolutionQCIF:
		case eVideoResolutionQVGA:
		case eVideoResolutionCIF:
		case eVideoResolutionSIF:
		{
			copDecoderResolution = COP_decoder_resolution_CIF25;
			break;
		}
		case eVideoResolutionVGA:
		case eVideoResolution4SIF:
		case eVideoResolution4CIF:
		{
			copDecoderResolution = COP_decoder_resolution_4CIF25;
			break;
		}
		case eVideoResolution525SD:
		case eVideoResolution625SD:
		{
			copDecoderResolution = COP_decoder_resolution_W4CIF25;
			break;
		}
		case eVideoResolutionSVGA:
		case eVideoResolutionXGA:
		case eVideoResolutionHD720:
		case eVideoResolution16CIF:
		{
			copDecoderResolution = COP_decoder_resolution_HD720p25;
			break;
		}
		case eVideoResolutionHD1080:
		{
			copDecoderResolution = COP_decoder_resolution_HD108030;
			break;
		}
		default:
		{
			copDecoderResolution = COP_decoder_resolution_Last; // not found
			break;
		}
	}

	return copDecoderResolution;
}

//--------------------------------------------------------------------------

BYTE CBridgePartyVideoInCOP::IsSyncWithDecoderResolution()
{
  BYTE isSync = NO;
	if(m_last_sync_status == STATUS_OK)
	{
    // temporary only status
    isSync = YES;
	}
  return isSync;
}

//--------------------------------------------------------------------------
BYTE CBridgePartyVideoInCOP::IsRemoteNeedSmartSwitchAccordingToVendor()
{
	return m_IsRemoteUseSmartSwitchAccordingToVendor;
}



