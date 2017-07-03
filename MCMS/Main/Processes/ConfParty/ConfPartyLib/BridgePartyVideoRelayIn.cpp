
#include <memory>

#include "BridgePartyVideoRelayIn.h"
#include "BridgePartyVideoUniDirection.h"
#include "BridgePartyVideoRelayMediaParams.h"
#include "SVCToAVCTranslator.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"

//#define WAIT_FOR_TRANSLATOR		1
//#define NO_WAIT_FOR_TRANSLATOR  2
//#define TRANSLATOR_ERROR        3

#define SVCAVC_TRANSLATOR_DISCONNECT						((WORD)201)
#define	SVCAVC_KILL_TRANSLATOR_TIMER_0						((WORD)202)

// temp!!!
#define VIDEO_IN_SVCAVCTRANSLATOR_CONNECTED 999999

// ~~~~~~~~~~~~~~ Global functions ~~~~~~~~~~~~~~~~~~~~~~~~~~
extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);

PBEGIN_MESSAGE_MAP(CBridgePartyVideoRelayIn)

	ONEVENT(CONNECT_VIDEO_IN,			IDLE,               CBridgePartyVideoRelayIn::OnVideoBridgePartyConnectIDLE)
	ONEVENT(CONNECT_VIDEO_IN,			SETUP,              CBridgePartyVideoRelayIn::OnVideoBridgePartyConnectSETUP)
	ONEVENT(CONNECT_VIDEO_IN,			CONNECTED,          CBridgePartyVideoRelayIn::OnVideoBridgePartyConnectCONNECTED)
	ONEVENT(CONNECT_VIDEO_IN,			DISCONNECTING,      CBridgePartyVideoRelayIn::OnVideoBridgePartyConnectDISCONNECTING)

	ONEVENT(UPDATE_VIDEO_IN_PARAMS, 	IDLE,               CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsIDLE)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,		SETUP,              CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsSETUP)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,		CONNECTED,          CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsCONNECTED)
	ONEVENT(UPDATE_VIDEO_IN_PARAMS,		DISCONNECTING,      CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsCONNECTING)

	ONEVENT(DISCONNECT_VIDEO_IN, 		IDLE,               CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectIDLE)
	ONEVENT(DISCONNECT_VIDEO_IN,		SETUP,              CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectSETUP)
	ONEVENT(DISCONNECT_VIDEO_IN,		CONNECTED,          CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectCONNECTED)
	ONEVENT(DISCONNECT_VIDEO_IN,		DISCONNECTING,      CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectDISCONNECTING)

	ONEVENT(ADD_AVC_IMAGE, 				IDLE,               CBridgePartyVideoRelayIn::NullActionFunction)
	ONEVENT(ADD_AVC_IMAGE,				SETUP,              CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageSETUP)
	ONEVENT(ADD_AVC_IMAGE,				CONNECTED,          CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageCONNECTED)
	ONEVENT(ADD_AVC_IMAGE,				DISCONNECTING,      CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageDISCONNECTING)

	ONEVENT(LAST_AVC_IMAGE_REMOVED, 	IDLE,               CBridgePartyVideoRelayIn::NullActionFunction)
	ONEVENT(LAST_AVC_IMAGE_REMOVED,		SETUP,              CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedSETUP)
	ONEVENT(LAST_AVC_IMAGE_REMOVED,		CONNECTED,          CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedCONNECTED)
	ONEVENT(LAST_AVC_IMAGE_REMOVED,		DISCONNECTING,      CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedDISCONNECTING)

	ONEVENT(SVC_AVC_TRANSLATOR_CONNECTED, 	IDLE,               CBridgePartyVideoRelayIn::NullActionFunction)
	ONEVENT(SVC_AVC_TRANSLATOR_CONNECTED, 	SETUP,              CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorConnectedSETUP)
	ONEVENT(SVC_AVC_TRANSLATOR_CONNECTED, 	CONNECTED,          CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorConnectedCONNECTED)
	ONEVENT(SVC_AVC_TRANSLATOR_CONNECTED, 	DISCONNECTING,      CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorConnectedDISCONNECTING)

	ONEVENT(SVC_AVC_TRANSLATOR_DISCONNECTED, IDLE,               CBridgePartyVideoRelayIn::NullActionFunction)
	ONEVENT(SVC_AVC_TRANSLATOR_DISCONNECTED, SETUP,              CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorDisconnectedSETUP)
	ONEVENT(SVC_AVC_TRANSLATOR_DISCONNECTED, CONNECTED,          CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorDisconnectedCONNECTED)
	ONEVENT(SVC_AVC_TRANSLATOR_DISCONNECTED, DISCONNECTING,      CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorDisconnectedDISCONNECTING)

	ONEVENT(SVCAVC_KILL_TRANSLATOR_TIMER_0, ANYCASE, 			CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorKillANYCASE)

	// dynamic disconnect req
	ONEVENT(SVCAVC_TRANSLATOR_DISCONNECT, 	SETUP,               CBridgePartyVideoRelayIn::OnVideoBridgeTraslatorDisconnectSETUP)
	ONEVENT(SVCAVC_TRANSLATOR_DISCONNECT, 	CONNECTED,           CBridgePartyVideoRelayIn::OnVideoBridgeTraslatorDisconnectCONNECTED)
	ONEVENT(SVCAVC_TRANSLATOR_DISCONNECT, 	ANYCASE,          	 CBridgePartyVideoRelayIn::NullActionFunction)

	ONEVENT(UPGRADE_TO_MIX_AVC_SVC, 		IDLE,                CBridgePartyVideoRelayIn::OnVideoBridgeUpgradeTraslateIDLE)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC, 		SETUP,               CBridgePartyVideoRelayIn::OnVideoBridgeUpgradeTraslateSETUP)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC, 		CONNECTED,           CBridgePartyVideoRelayIn::OnVideoBridgeUpgradeTraslateCONNECTED)
	ONEVENT(UPGRADE_TO_MIX_AVC_SVC, 		DISCONNECTING,     	 CBridgePartyVideoRelayIn::OnVideoBridgeUpgradeTraslateDISCONNECTING)

PEND_MESSAGE_MAP(CBridgePartyVideoRelayIn,CBridgePartyMediaUniDirection);

/////////////////////////////////////////////////////
CBridgePartyVideoRelayIn::CBridgePartyVideoRelayIn():CBridgePartyMediaUniDirection()
{
	m_pImage = NIL(CImage);
  	m_isReady= FALSE;
  	m_bIsImageUpdated = false;
  	m_pSvcToAvcParams = NULL;
  	m_pSvcToAvcTranslator = NULL;
  	m_bSvcAvcTranslatorReady = false;
  	m_bNeedToConnectSvcAvcTranslator = false;	// the default is not to connect, only upon specific request (e.g. first AVC entered)
  	m_bNeedToReplayConnectSvcToAvcTranslator = false;
  	m_bNeedToReplayDisconnectSvcToAvcTranslator = false;
}
CBridgePartyVideoRelayIn::CBridgePartyVideoRelayIn(const CBridgePartyVideoRelayIn& rOtherBridgePartyVideRelayoIn):CBridgePartyMediaUniDirection(rOtherBridgePartyVideRelayoIn)
{
	m_pImage = rOtherBridgePartyVideRelayoIn.m_pImage;
	m_bIsImageUpdated = rOtherBridgePartyVideRelayoIn.m_bIsImageUpdated;
	if(IsValidPObjectPtr(rOtherBridgePartyVideRelayoIn.m_pSvcToAvcParams))
		m_pSvcToAvcParams = new CSvcToAvcParams();
	else
		m_pSvcToAvcParams = NULL;
	if(IsValidPObjectPtr(rOtherBridgePartyVideRelayoIn.m_pSvcToAvcTranslator))	// xxx
		m_pSvcToAvcTranslator = new CSVCToAVCTranslator();
	else
		m_pSvcToAvcTranslator = NULL;
	m_bSvcAvcTranslatorReady = rOtherBridgePartyVideRelayoIn.m_bSvcAvcTranslatorReady;
	m_bNeedToConnectSvcAvcTranslator = rOtherBridgePartyVideRelayoIn.m_bNeedToConnectSvcAvcTranslator;
	m_bNeedToReplayConnectSvcToAvcTranslator = rOtherBridgePartyVideRelayoIn.m_bNeedToReplayConnectSvcToAvcTranslator;
	m_bNeedToReplayDisconnectSvcToAvcTranslator = rOtherBridgePartyVideRelayoIn.m_bNeedToReplayDisconnectSvcToAvcTranslator;
}

/////////////////////////////////////////////////////
CBridgePartyVideoRelayIn::~CBridgePartyVideoRelayIn ()
{
	POBJDELETE(m_pImage);
	POBJDELETE(m_pSvcToAvcParams);
	POBJDELETE(m_pSvcToAvcTranslator);

}

///////////////////////////////////////////////////////
//CBridgePartyVideoRelayIn& CBridgePartyVideoRelayIn::operator =(const CBridgePartyVideoRelayIn&)
//{
//
//}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::Create (const CBridgePartyCntl*	pBridgePartyCntl)
{
	m_pBridgePartyCntl		=	(CBridgePartyCntl*)pBridgePartyCntl;
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::Create(const CBridgePartyCntl* pBridgePartyCntl, const CBridgePartyMediaParams* pBridgePartyMediaParams, const char* pSitename)
{
	m_pBridgePartyCntl = (CBridgePartyCntl*)pBridgePartyCntl;
	char siteName[MAX_SITE_NAME_ARR_SIZE] = {0};
	if (pSitename) {
		strncpy(siteName, pSitename, MAX_SITE_NAME_ARR_SIZE-1 );
		siteName[MAX_SITE_NAME_ARR_SIZE-1] = '\0';
	}

	CDwordBitMask muteMask = ((CBridgePartyVideoRelayMediaParams*)pBridgePartyMediaParams)->GetMuteMask();
	CRelayMediaStreamList videoRelayMediaStreamList = ((CBridgePartyVideoRelayMediaParams*)pBridgePartyMediaParams)->GetRelayMediaStream();
	m_pImage = new CImage(INVALID /*connection id*/, m_pBridgePartyCntl->GetPartyRsrcID(),
	                      m_pBridgePartyCntl->GetPartyTaskApp(), siteName, m_pBridgePartyCntl->GetName(),
	                      DEFAULT_DECODER_DETECTED_MODE_WIDTH, DEFAULT_DECODER_DETECTED_MODE_HEIGHT,
	                      DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_WIDTH, DEFAULT_DECODER_DETECTED_SAMPLE_ASPECT_RATIO_HEIGHT,
	                      eVideoResolutionDummy, 0, 0, 0, muteMask, INVALID, INVALID, videoRelayMediaStreamList,
	                      eCascadeNone, TRUE /*ISRELAY*/, ((CBridgePartyVideoRelayMediaParams*)pBridgePartyMediaParams)->GetChannelHandle());

	m_isReady = ((CBridgePartyVideoRelayMediaParams*)pBridgePartyMediaParams)->GetIsReady();
	m_bNeedToConnectSvcAvcTranslator = ((CBridgePartyVideoRelayMediaParams*)pBridgePartyMediaParams)->GetSupportSvcAvcTranslate();

	TRACEINTO << " Full Name: " << m_pBridgePartyCntl->GetFullName() << " m_isReady=" <<  (DWORD)m_isReady << ", Need to Connect Translator=" << (DWORD)m_bNeedToConnectSvcAvcTranslator;
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::Connect()
{
	DispatchEvent(CONNECT_VIDEO_IN, NULL);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::DisConnect()
{
	DispatchEvent(DISCONNECT_VIDEO_IN, NULL);
}

///////////////////////////////////////////////////////
//void CBridgePartyVideoRelayIn::ConnectDynamicMix()
//{
//	DispatchEvent(SVCAVC_TRANSLATOR_CONNECT, NULL);
//}

///////////////////////////////////////////////////////
//void CBridgePartyVideoRelayIn::DisconnectDynamicMix()
//{
//	DispatchEvent(SVCAVC_TRANSLATOR_DISCONNECT, NULL);
//}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::AddImageToConfMix()
{
	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->AddImageToConfMix();
}
///////////////////////////////////////////////////////
BOOL CBridgePartyVideoRelayIn::IsVideoMediaStreamListEmpty()
{
	if((NULL==m_pImage) || ((m_pImage) && m_pImage->GetVideoRelayMediaStreamsList().size()== 0))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
/////////////////////////////////////////////////////
//This function is called in the flow connect while disconnecting
void CBridgePartyVideoRelayIn::UpdatePartyInParams(CUpdatePartyVideoRelayInitParams* updatePartyVideoRelayInitParams)
{
	CBridgePartyVideoRelayMediaParams * pBridgePartyVideoRelayParams = (CBridgePartyVideoRelayMediaParams *) updatePartyVideoRelayInitParams->GetMediaInParams();
	UpdateVideoParams(pBridgePartyVideoRelayParams);
}
/////////////////////////////////////////////////////
//This function is called in the flow of UpdatePartyVideoIn
void CBridgePartyVideoRelayIn::UpdateVideoParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams)
{
	m_bIsImageUpdated  = SaveUpdatedVideoParams(pBridgePartyVideoRelayParams);
	//CSegment* pSeg = new CSegment;
 	DispatchEvent(UPDATE_VIDEO_IN_PARAMS,NULL);
 	//POBJDELETE(pSeg);
 	m_bIsImageUpdated = false;
}
/////////////////////////////////////////////////////
bool CBridgePartyVideoRelayIn::SaveUpdatedVideoParams(CBridgePartyVideoRelayMediaParams* pBridgePartyVideoRelayParams)
{
	m_isReady 						 = pBridgePartyVideoRelayParams->GetIsReady();
	m_bNeedToConnectSvcAvcTranslator = pBridgePartyVideoRelayParams->GetSupportSvcAvcTranslate();

	TRACEINTO << " m_isReady=" <<  (DWORD)m_isReady << ", Need to Connect Translator=" << (DWORD)m_bNeedToConnectSvcAvcTranslator;

//	std::list < CRelayMediaStream *>  listVideoMediaStreams;
//	for(WORD i=0; i<3; i++) //temp for integration
//	{
//        CVideoRelayMediaStream* videoMediaStream = new CVideoRelayMediaStream();
//        int ssrc = (0==i) ? 10 : (1==i ? 11 : 12);
//        char layer = (0==i) ? 2 : (1==i ? 5 : 8);
//        DWORD width = (0==i) ? 320 : (1==i ? 640 : 1280);
//        DWORD height = (0==i) ? 180 : (1==i ? 360 : 720);
//        videoMediaStream->SetSsrc(ssrc);
//        videoMediaStream->SetLayerId(layer);
//        videoMediaStream->SetResolutionHeight(height);
//        videoMediaStream->SetResolutionWidth(width);
//        listVideoMediaStreams.push_back(videoMediaStream);
//	}
	bool isImageUpdated  = false;
	if(m_pImage->UpdateRelayParams(pBridgePartyVideoRelayParams->GetChannelHandle(), pBridgePartyVideoRelayParams->GetRelayMediaStream(), pBridgePartyVideoRelayParams->GetMuteMask()))
		isImageUpdated=true;

	return isImageUpdated;
}

/////////////////////////////////////////////////////
void  CBridgePartyVideoRelayIn::SetVideoInConnected( EStat status )
{
	CSegment* pSeg = new CSegment;

	if (status == statOK)
	{
		*pSeg << (BYTE)statOK;
		m_state = CONNECTED;
		TRACEINTO << " State changed to CONNECTED, Party Name: " << m_pBridgePartyCntl->GetFullName() << " Status==OK";
	}
	else
	{
		*pSeg << (BYTE)status << (BYTE)eMipIn << (BYTE)eMipStatusFail << (BYTE) eMipConnect;
		TRACEINTO << " State stays SETUP as result of error, Party Name: " << m_pBridgePartyCntl->GetFullName() << " Status=" << (DWORD)status;
	}

	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_CONNECTED);

	if (status == statOK)
		if(!IsVideoMediaStreamListEmpty())
			AddImageToConfMix();

	POBJDELETE(pSeg);
}


/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgePartyConnectIDLE(CSegment* pParam)
{
	m_state = SETUP;

	if(!m_isReady)
	{
		TRACEINTO << " state changed to SETUP, but not ready, Party Name: " << m_pBridgePartyCntl->GetFullName();
		return;
	}

	OnVideoBridgePartyConnect();

}
/////////////////////////////////////////////////////

void  CBridgePartyVideoRelayIn::OnVideoBridgePartyConnectSETUP(CSegment* pParam)
{
	if(!m_isReady)
	{
		TRACEINTO << " not ready, nothing to do, Party Name: " << m_pBridgePartyCntl->GetFullName();
		return;
	}

	OnVideoBridgePartyConnect();
}
/////////////////////////////////////////////////////

void  CBridgePartyVideoRelayIn::OnVideoBridgePartyConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << " msg ignored, party already connected, Part NAme: " << m_pBridgePartyCntl->GetFullName();
}

///////////////////////////////////////////////////////
void  CBridgePartyVideoRelayIn::OnVideoBridgePartyConnectDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << " connect Request ignored in disconnecting, Part NAme: " << m_pBridgePartyCntl->GetFullName();
}

/////////////////////////////////////////////////////
void  CBridgePartyVideoRelayIn::OnVideoBridgePartyConnect()
{
	//Translate Svc to Avc

	int translatorStatus = InitOrUpdateSvcToAvcTranslator();
	if (WAIT_FOR_TRANSLATOR == translatorStatus)
	{
		TRACEINTO << " waiting for SvcToAvcTranslator to connect, Party Name: " << m_pBridgePartyCntl->GetFullName();
		return;	// keep with the party-in-video connection after the SvcToAvc Translator will finish to connect
	}

	if (TRANSLATOR_ERROR == translatorStatus)
	{
		TRACEINTO << " SVC-Translator-Error - Party Name: " << m_pBridgePartyCntl->GetFullName();
		return;	// waiting for the global "Connect Party timer" to jump, stays in SETUP state
	}

	// Party moves to CONNECT state and inform Party Cntl
	SetVideoInConnected( statOK );
}

/////////////////////////////////////////////////////
void  CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsIDLE(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsIDLE ",m_pBridgePartyCntl->GetFullName());
	EStat responseStatus = statOK;
	CSegment *pMsg = new CSegment;
	*pMsg << (BYTE)responseStatus;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
	POBJDELETE(pMsg);

}
/////////////////////////////////////////////////////
void  CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsSETUP(CSegment* pParam)
{
	if(!m_isReady)
	{
		TRACEINTO << " not ready, do nothing ",m_pBridgePartyCntl->GetFullName();
	}
	else
	{
		OnVideoBridgePartyConnect();
	}

	EStat responseStatus = statOK;
	CSegment *pMsg = new CSegment;
	*pMsg << (BYTE)responseStatus;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
	POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////
int CBridgePartyVideoRelayIn::InitOrUpdateSvcToAvcTranslator()
{
	int isNeedToWait = NO_WAIT_FOR_TRANSLATOR;

	if (m_pSvcToAvcTranslator)
	{
		isNeedToWait = UpdateSvcToAvcTranslatorImageUpdatedIfNeeded();
	}
	else
	{
		InitSvcToAvcParams();
		if(IsTranslateSvcToAvcSupported())
		{
			TRACEINTO << " Starting SvcToAvcTranslator ";
			isNeedToWait = TranslateSvcToAvc();
		}
	}
	return isNeedToWait;
}


/////////////////////////////////////////////////////
void  CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsCONNECTED ",m_pBridgePartyCntl->GetFullName());
	if(m_bIsImageUpdated)
	{
	    PTRACE2(eLevelInfoNormal, "CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsCONNECTED update on image updated to VB",m_pBridgePartyCntl->GetFullName());
	    //TODO
//	    ((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->UpdateBridgeOnUpdateImageParams(m_pImage->GetPartyRsrcId());

	    UpdateSvcToAvcTranslatorImageUpdatedIfNeeded();

        {//OLGA
            CSegment *pSeg = new CSegment;
            *pSeg << (WORD)STATUS_OK;
            m_pBridgePartyCntl->HandleEvent(pSeg, 0, PARTY_IMAGE_UPDATED);
            POBJDELETE(pSeg);
        }
	}
	EStat responseStatus = statOK;
	CSegment *pMsg = new CSegment;
	*pMsg << (BYTE)responseStatus;
	m_pBridgePartyCntl->HandleEvent(pMsg, 0, PARTY_VIDEO_IN_UPDATED);
	POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////
int CBridgePartyVideoRelayIn::UpdateSvcToAvcTranslatorImageUpdatedIfNeeded()
{
	if (!m_pSvcToAvcTranslator || !m_pSvcToAvcParams)
		return TRANSLATOR_ERROR;

	UpdateSvcParamsWithImageParams();	// the result can be NULL media Streams, it is legal

	m_pSvcToAvcTranslator->UpdateVideoRelaySsrc();	// update ART (via translator) on new SSRC if needed

	return WAIT_FOR_TRANSLATOR;		// need to wait until the translator will finish the connection
}

/////////////////////////////////////////////////////
void  CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CBridgePartyVideoRelayIn::OnVideoBridgePartyUpdateVideoParamsCONNECTING - Ignored at this stage, name: ",m_pBridgePartyCntl->GetFullName());
}

void CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnect(CSegment* pParam)
{

	if(m_pSvcToAvcTranslator)
	 {
		 m_state = DISCONNECTING;
		 m_pSvcToAvcTranslator->Disconnect();
	 }
	else
	{
		m_state = IDLE;
		BYTE	responseStatus = statOK;

		CSegment* pSeg = new CSegment;
		*pSeg  << (BYTE)responseStatus;
		// Inform BridgePartyCntl
		m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_DISCONNECTED);	// nothing permitted after this action as it leads to self-destroy this class!!
		POBJDELETE(pSeg);
	}
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectIDLE(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectIDLE ignore already disconnected");
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectSETUP(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectSETUP ");
	OnVideoBridgePartyDisconnect(pParam);
}
/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectCONNECTED(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectCONNECTED ");
	OnVideoBridgePartyDisconnect(pParam);
}
void CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectDISCONNECTING(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgePartyDisconnectDISCONNECTING - ignored at this stage ");
}

/////////////////////////////////////////////////////
bool CBridgePartyVideoRelayIn::IsTranslateSvcToAvcSupported()
{
	bool isTranslateSvcToAvcSupported = false;
	if(IsValidPObjectPtr(m_pSvcToAvcParams))
	{
		if(m_pSvcToAvcParams->IsSupportSvcAvcTranslate())
		{
			isTranslateSvcToAvcSupported = true;
		}
	}
	else
	{
		TRACEINTO << " m_pSvcToAvcParams is not valid = " << (DWORD)(m_pSvcToAvcParams);
	}
	return isTranslateSvcToAvcSupported;
}

/////////////////////////////////////////////////////
bool CBridgePartyVideoRelayIn::UpdateSvcParamsWithImageParams()
{
	if (!m_pSvcToAvcParams)
	{
		TRACEINTO << " SVC-Translator-Error: NULL m_pSvcToAvcParams ";
		return false;
	}
	const CVideoOperationPointsSet* pVideoOperationPointsSet = m_pSvcToAvcParams->GetVideoOperationPointsSet();
	if (!pVideoOperationPointsSet)
	{
		TRACEINTO << " SVC-Translator-Error: NULL pVideoOperationPointsSet ";
		return false;
	}

	unsigned int resultSsrc = 0;
	int resultLayerId = 0;

	int requestedMaxLayerId = m_pSvcToAvcParams->GetAllowedLayerId();	// get the MAX allowed Layer-Id for this Party
	bool bFoundBestLayerId = GetBestSsrcAndLayerIdFromImage(requestedMaxLayerId, m_pImage, resultSsrc, resultLayerId,pVideoOperationPointsSet);
	if (bFoundBestLayerId)
	{
		CVideoRelayInMediaStream* videoMediaStream = NULL;
		bool foundVideoMediaStream = m_pImage->GetVideoMediaStreamInListAccordingToSsrc(resultSsrc, videoMediaStream);
		if(foundVideoMediaStream)
		{
			m_pSvcToAvcParams->UpdateTranslatedVideoRelayMediaStream(videoMediaStream);
		}
		else
		{
			m_pSvcToAvcParams->UpdateTranslatedVideoRelayMediaStream( NULL );
			TRACEINTO << " SVC-Translator-Error: didn't find video media according to ssrc: " << resultSsrc << "  Name: " << m_pBridgePartyCntl->GetFullName();
		}
	}
	else
	{
		m_pSvcToAvcParams->UpdateTranslatedVideoRelayMediaStream( NULL );
		TRACEINTO << " SVC-Translator-Error: didn't find best layer id, Requested Layer: " << requestedMaxLayerId << "  Name: "<< m_pBridgePartyCntl->GetFullName();
	}

	return 	true;
}

/////////////////////////////////////////////////////
int CBridgePartyVideoRelayIn::TranslateSvcToAvc()
{
	if (m_pSvcToAvcTranslator)	// normally not happens
	{
		TRACEINTO << " SVC-Translator-Error - Already Started!!?? ";
		return NO_WAIT_FOR_TRANSLATOR;
	}

	ISDEBUGMODE_RETURN_STAT("TRANSLATOR", 41, TRANSLATOR_ERROR )

	TRACEINTO << " - Party Name: ", m_pBridgePartyCntl->GetFullName();

	bool bSucceeded = UpdateSvcParamsWithImageParams();
	if (!bSucceeded)
		return TRANSLATOR_ERROR;

	bSucceeded = CreateAndConnectSvcToAvcTranslator();
	if (!bSucceeded)
		return TRANSLATOR_ERROR;

	m_pSvcToAvcParams->Dump();

	return WAIT_FOR_TRANSLATOR;
}

/////////////////////////////////////////////////////
bool CBridgePartyVideoRelayIn::CreateAndConnectSvcToAvcTranslator()
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	if(!pRoutingTable || !m_pBridgePartyCntl)
	{
		PASSERT(1);
		return false;
	}
	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
	DWORD confRsrcId = m_pBridgePartyCntl->GetConfRsrcID();
	CConfApi* pConfApi = m_pBridgePartyCntl->GetConfApi();

	NewSvcToAvcTranslator();	// creating the translator

	std::auto_ptr<CTaskApi> pTaskApiTranslator(new CTaskApi(*pConfApi));
	pTaskApiTranslator->CreateOnlyApi(pConfApi->GetRcvMbx(), m_pSvcToAvcTranslator);

	//create CRsrcParams for the decoder
	CRsrcParams decoderRsrcParams(DUMMY_CONNECTION_ID, partyRsrcId,confRsrcId);
	TRACEINTO << " PartyRsrcID: " << (DWORD)partyRsrcId;
    CRsrcDesc* pDecoderRsrcDesc = pRoutingTable->AddStateMachinePointerToRoutingTbl(partyRsrcId, eLogical_video_decoder, pTaskApiTranslator.get());
	if (!pDecoderRsrcDesc)   // Entry not found in Routing Table
	{
		POBJDELETE(m_pSvcToAvcTranslator);
		PASSERT(1);
		return false;
	}
	decoderRsrcParams.SetRsrcDesc(*pDecoderRsrcDesc);

	TRACEINTO << " -  Starting SvcToAvcTranslator, connID = " << decoderRsrcParams.GetConnectionId() << "  Party Name: " << m_pBridgePartyCntl->GetFullName();

	m_pSvcToAvcTranslator->Create(m_pBridgePartyCntl, this, &decoderRsrcParams);
	m_pSvcToAvcTranslator->Connect();

	return true;
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::NewSvcToAvcTranslator()
{
	POBJDELETE(m_pSvcToAvcTranslator);
	m_pSvcToAvcTranslator = new CSVCToAVCTranslator();
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::UpdateImageOnTranslateSvcToAvc()
{
	//the update of the image with the connection id & decoder params in translator.
	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->UpdateVideoBridgeOnRelayImageSvcToAvcTranslated();
}

////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::SvcAvcTranslatorDecoderOutOfSync()
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn:: SvcAvcTranslatorDecoderOutOfSync()",m_pBridgePartyCntl->GetFullName());
	SvcToAvcTranslatorAskEpForIntra();
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::SvcToAvcTranslatorAskEpForIntra()
{
	if(IsValidPObjectPtr(m_pSvcToAvcParams))
	{
		CVideoRelayMediaStream * pVideoRelayMediaStream = m_pSvcToAvcParams->GetVideoRelayMediaStream();
		if(IsValidPObjectPtr( pVideoRelayMediaStream) )
		{
			AskForRelayIntra epIntraParams;
			RelayIntraParam epintraParam;
			epintraParam.m_partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
			epintraParam.m_listSsrc.push_back( pVideoRelayMediaStream->GetSsrc() );
			// GDR??

			epIntraParams.m_relayIntraParameters.push_back(epintraParam);
			epIntraParams.m_isImmediately = true;

			((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->AskEpsForIntra( epIntraParams );

		}
		else
		{
			TRACEINTO << " - pVideoRelayMediaStream NOT VALID, Party Name: ", m_pBridgePartyCntl->GetFullName();
			PASSERT(101);
		}

	}
	else
	{
		TRACEINTO << " - m_pSvcToAvcParams NOT VALID, Party Name: ", m_pBridgePartyCntl->GetFullName();
		PASSERT(101);
	}

}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::UpdateSvcToAvcTranslatorOnFirstAvcImage()
{
	DispatchEvent(ADD_AVC_IMAGE, NULL);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::UpdateSvcToAvcTranslatorOnNoAvcImageInConf()
{
	DispatchEvent(LAST_AVC_IMAGE_REMOVED, NULL);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageSETUP ", m_pBridgePartyCntl->GetFullName());
	if(IsValidPObjectPtr(m_pSvcToAvcParams))
	{
		m_pSvcToAvcParams->SetIsAvcInConf(true);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageSETUP SVCAVCTranslator not valid", m_pBridgePartyCntl->GetFullName());
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageCONNECTED ", m_pBridgePartyCntl->GetFullName());
	if(IsValidPObjectPtr(m_pSvcToAvcParams))
	{
		m_pSvcToAvcParams->SetIsAvcInConf(true);
		if(IsValidPObjectPtr(m_pSvcToAvcTranslator))
		{
				m_pSvcToAvcTranslator->UpdateAvcImageInConf();
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageCONNECTED SVCAVCTranslator not valid", m_pBridgePartyCntl->GetFullName());
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeAddAvcImageDISCONNECTING ignore", m_pBridgePartyCntl->GetFullName());

}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedSETUP ", m_pBridgePartyCntl->GetFullName());
	if(IsValidPObjectPtr(m_pSvcToAvcParams))
	{
		m_pSvcToAvcParams->SetIsAvcInConf(false);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedSETUP SVCAVCTranslator not valid", m_pBridgePartyCntl->GetFullName());
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedCONNECTED(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedCONNECTED ", m_pBridgePartyCntl->GetFullName());
	if(IsValidPObjectPtr(m_pSvcToAvcParams))
	{
		m_pSvcToAvcParams->SetIsAvcInConf(false);
		if(IsValidPObjectPtr(m_pSvcToAvcTranslator))
		{
			m_pSvcToAvcTranslator->UpdateAvcImageInConf();
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedCONNECTED SVCAVCTranslator not valid", m_pBridgePartyCntl->GetFullName());
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedDISCONNECTING(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnVideoBridgeLastAvcImageRemovedDISCONNECTING ignore", m_pBridgePartyCntl->GetFullName());

}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::SvcToAvcTranslatorDisconnected(EStat status)
{
	TRACEINTO << " - SvcAvc Translator Disconnected, Party Name: " << m_pBridgePartyCntl->GetFullName() << " Status=" << (DWORD)status;
	RemoveSvcToAvcStateMachineFromRoutingTable();
	StartTimer( SVCAVC_KILL_TRANSLATOR_TIMER_0, 0 );	// just to DELETE the Translator Object (timer-time=0, do it immediately)

	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)status;
	DispatchEvent(SVC_AVC_TRANSLATOR_DISCONNECTED, pSeg);	// in case of disconnecting state, this function leads to delete this class!
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorDisconnectedSETUP(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorDisconnectedSETUP ", m_pBridgePartyCntl->GetFullName());

	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorDisconnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << " - Party Name: ", m_pBridgePartyCntl->GetFullName();

	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;

	if (m_bNeedToReplayDisconnectSvcToAvcTranslator)
	{
		//ReplayDisconnectSvcToAvcTranslator( receivedStatus ); // downgrade is not supported yet!!
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorDisconnectedDISCONNECTING(CSegment* pParam)
{
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;

	// change state to IDLE
	TRACEINTO << " - State changed to IDLE, Party Name: " << m_pBridgePartyCntl->GetFullName();
	m_state = IDLE;

	// Update videoRelayPartyControl of end disconnect
	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)receivedStatus;
	m_pBridgePartyCntl->HandleEvent(pSeg, 0, VIDEO_IN_DISCONNECTED);	// nothing permitted after this action as it leads to self-destroy this class!!
	POBJDELETE(pSeg);
}


/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorConnectedSETUP(CSegment* pParam)
{
	TRACEINTO << " - SvcToAvcTranslator connected, Party Name: " << m_pBridgePartyCntl->GetFullName();
	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;
	if (statOK == receivedStatus)
		m_bSvcAvcTranslatorReady = true;	// update about translator is ready

	SetVideoInConnected( receivedStatus );
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorConnectedCONNECTED(CSegment* pParam)
{
	TRACEINTO << " - SvcToAvcTranslator connected, Party Name: " << m_pBridgePartyCntl->GetFullName()<<"m_bNeedToReplayConnectSvcToAvcTranslator:"<<(int)m_bNeedToReplayConnectSvcToAvcTranslator;

	EStat receivedStatus = statOK;
	*pParam >> (BYTE&)receivedStatus;

	// if dynamic connect, inform sender
	if (m_bNeedToReplayConnectSvcToAvcTranslator)
		ReplayConnectSvcToAvcTranslator( receivedStatus );

	if (statOK != receivedStatus)
		if (m_pSvcToAvcTranslator)
			m_pSvcToAvcTranslator->Disconnect();	// disconnect Translator (SVC to AVC) upon connecting error
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorConnectedDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << " - in disconnecting process, probably nothing to do";
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::SvcToAvcTranslatorConnected( EStat status )
{
	CSegment* pSeg = new CSegment;
	*pSeg  << (BYTE)status;
	DispatchEvent(SVC_AVC_TRANSLATOR_CONNECTED, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::UpdateLastReqUponTranslatorError( DWORD lastRequestId, DWORD lastReqOp)
{
	m_lastReqId = lastRequestId;
	m_lastReq   = lastReqOp;
}
void CBridgePartyVideoRelayIn::OnSvcToAvcTraslatorKillANYCASE(CSegment* pParam)
{
	if(IsValidPObjectPtr(m_pSvcToAvcTranslator))
	{
		POBJDELETE(m_pSvcToAvcTranslator);
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::RemoveSvcToAvcStateMachineFromRoutingTable()
{
	CConfPartyRoutingTable* pRoutingTable = ::GetpConfPartyRoutingTable();
	PASSERT_AND_RETURN(!pRoutingTable);
	PASSERT_AND_RETURN(!m_pBridgePartyCntl);
	DWORD partyRsrcId = m_pBridgePartyCntl->GetPartyRsrcID();
	DWORD confRsrcId  = m_pBridgePartyCntl->GetConfRsrcID();
	ConnectionID decoderConnectionId = DUMMY_CONNECTION_ID;
	if (m_pSvcToAvcTranslator)
		decoderConnectionId = m_pSvcToAvcTranslator->GetConnectionId();

	CRsrcParams decoderRsrcParams( decoderConnectionId, partyRsrcId, confRsrcId, eLogical_video_decoder );	// there is only one such resource
	int  status = pRoutingTable->RemoveStateMachinePointerFromRoutingTbl( decoderRsrcParams );
	if (status != STATUS_OK)
	{
		PTRACE2(eLevelInfoNormal,"CBridgePartyVideoRelayIn::RemoveSvcToAvcStateMachineFromRoutingTable - SVC-Translator-Error Failed to remove StateMachine, Name - ",m_pBridgePartyCntl->GetFullName());
	}
	else
	{
		TRACEINTO << " SvcToAvc State Machine Removed from RoutingTbl";
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::InitSvcToAvcParams()
{
	PDELETE(m_pSvcToAvcParams);
	m_pSvcToAvcParams = new CSvcToAvcParams();

//	// set SVC to AVC supports
	DWORD confMediaType = m_pBridgePartyCntl->GetConfMediaType();

	m_pSvcToAvcParams->SetIsSupportSvcAvcTranslate( (eMixAvcSvc == confMediaType) && m_bNeedToConnectSvcAvcTranslator);
	// set Operation Points
	m_pSvcToAvcParams->SetVideoOperationPointsSet( GetConfVideoOperationPointsSet() );

	// set max allowed Layer ID
	int maxAllowedLayerId = ((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->GetMaxAllowedLayerId();
	m_pSvcToAvcParams->SetMaxAllowedLayerId( maxAllowedLayerId );
	// set max allowed Layer ID VSW
	int maxAllowedLayerIdHighRes = ((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->GetMaxAllowedLayerIdHighRes();
	m_pSvcToAvcParams->SetMaxAllowedLayerIdHighRes( maxAllowedLayerIdHighRes );

	BOOL bIsHighResStreamEnable = ((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->IsEnableHighVideoResInSvcToAvcMixMode();	// get system.cfg flag from start of Conference
	m_pSvcToAvcParams->SetEnableDisableHighResFlag( bIsHighResStreamEnable );

	// set AVC in Conf
	DWORD numberOfNonRelayImagesInConf =  GetNumberOfNonRelayImages();
	if(numberOfNonRelayImagesInConf>0)
		m_pSvcToAvcParams->SetIsAvcInConf( true );
	else
		m_pSvcToAvcParams->SetIsAvcInConf( false );

	TRACEINTO << " - maxAllowedLayerId: " << maxAllowedLayerId << ", maxAllowedLayerIdHighRes: " << maxAllowedLayerIdHighRes << ", #AVC in Conf: " << numberOfNonRelayImagesInConf << ", ConfType: " << (DWORD)confMediaType << ", NeedTranslator: " << (DWORD)m_bNeedToConnectSvcAvcTranslator;
}

////////////////////////////////////////////////////////////////////////////////
CVideoOperationPointsSet* CBridgePartyVideoRelayIn::GetConfVideoOperationPointsSet()const
{
	if(IsValidPObjectPtr(m_pBridgePartyCntl))
	{
		return ((CVideoRelayBridgePartyCntl * )m_pBridgePartyCntl)->GetConfVideoOperationPointsSet();

	}
	PASSERT(101);
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
DWORD CBridgePartyVideoRelayIn::GetNumberOfNonRelayImages()const
{
	if(IsValidPObjectPtr(m_pBridgePartyCntl))
	{
		return ((CVideoRelayBridgePartyCntl * )m_pBridgePartyCntl)->GetNumberOfNonRelayImages();

	}
	PASSERT(101);
	return 0;
}


// ------------------------------------------------------------
void CBridgePartyVideoRelayIn::UpdateSelfMute(RequestPriority who, EOnOff eOnOff)
{
	if(eOn == eOnOff)
		MuteYourSelf(who);
	else
		UnMuteYourSelf(who);
}
// ------------------------------------------------------------
void CBridgePartyVideoRelayIn::UnMuteYourSelf(RequestPriority who)
{
	switch(who){
	case PARTY_Prior:{
		m_pImage->UnMuteByPartyMCV();
		break;
					 }
	case CHAIRMAN_Prior:{
		m_pImage->UnMuteByChairman();
		break;
						}
	case OPERATOR_Prior:{
		m_pImage->UnMuteByOperator();
		break;
						}
    case MCMS_Prior:{
        m_pImage->UnMuteByMcms();
        break;
                    }
	case SYNC_LOST_Prior:{
		m_pImage->SyncFound();
		break;
						 }

	default:{
		DBGPASSERT(1);
			}
	}
}

// ------------------------------------------------------------
void CBridgePartyVideoRelayIn::MuteYourSelf(RequestPriority who)
{
	switch(who){
	case PARTY_Prior:{
		m_pImage->MuteByPartyMCV();
		break;
					 }
	case CHAIRMAN_Prior:{
		m_pImage->MuteByChairman();
		break;
						}
	case OPERATOR_Prior:{
		m_pImage->MuteByOperator();
		break;
						}
    case MCMS_Prior :{
        m_pImage->MuteByMcms();
        break;
                     }
	case SYNC_LOST_Prior:{
		m_pImage->SyncLost();
		break;
						 }

	default:{
		DBGPASSERT(1);
			}
	}
}

// ------------------------------------------------------------
CBridgePartyCntl* CBridgePartyVideoRelayIn::GetBridgePartyCntlPtr()const
{
	return m_pBridgePartyCntl;
}

// ------------------------------------------------------------
bool  CBridgePartyVideoRelayIn::IsMuted() const
{
	bool ans = false;
	if(IsValidPObjectPtr( m_pImage))
	{
		ans = (m_pImage->isMuted())? true :false;
	}
	return ans;

}
//--------------------------------------------------------------------------

CDwordBitMask CBridgePartyVideoRelayIn::GetMuteMask() const
{
	CDwordBitMask muteMask;
	muteMask.ResetMask();
	if(IsValidPObjectPtr( m_pImage))
		muteMask = m_pImage->GetMuteMask();
	return muteMask;

}
//--------------------------------------------------------------------------
bool CBridgePartyVideoRelayIn::IsMuteByOperator()  const
{
	bool ans = false;
	if(IsValidPObjectPtr( m_pImage))
	{
		ans = (m_pImage->IsMuteByOperator())?true:false;
	}
	return ans;
}
//--------------------------------------------------------------------------

bool CBridgePartyVideoRelayIn::IsMuteByMCMS() const
{

	bool ans = false;
	if(IsValidPObjectPtr( m_pImage))
	{
		ans = (m_pImage->IsMuteByMCMS())?true:false;
	}
	return ans;
}
//--------------------------------------------------------------------------
bool CBridgePartyVideoRelayIn::IsMuteByParty() const
{

	bool ans = false;
	if(IsValidPObjectPtr( m_pImage))
	{
		ans = (m_pImage->IsMuteByParty())?true:false;
	}
	return ans;
}
//--------------------------------------------------------------------------
void CBridgePartyVideoRelayIn::UpdateArtOnTranslateVideoSSRC( DWORD dwSSRC )
{
	TRACEINTO << " Update SSRC=" << dwSSRC << " Party Name = " << m_pBridgePartyCntl->GetFullName();
	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->UpdateArtOnTranslateVideoSSRC( dwSSRC );
}

// ------------------------------------------------------------
void CBridgePartyVideoRelayIn::UpdateArtOnTranslateVideoSSRCAck( DWORD status )
{
	if (m_pSvcToAvcTranslator)
		m_pSvcToAvcTranslator->UpdateArtOnTranslateVideoSSRCAck( status );
	else
	{
		TRACEINTO << " m_pSvcToAvcTranslator=NULL, Party: " << m_pBridgePartyCntl->GetFullName() << ", status: " << status;
	}
}

// ------------------------------------------------------------
void CBridgePartyVideoRelayIn::UpdateMrmpStreamIsMust( DWORD dwSSRC, DWORD videoRelayInChannelHandle, BOOL bIsMustSsrc )
{
	TRACEINTO << " Update SSRC=" << dwSSRC << " Party Name = " << m_pBridgePartyCntl->GetFullName();
	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->UpdateMrmpStreamIsMust( dwSSRC, videoRelayInChannelHandle, bIsMustSsrc );
}

// ------------------------------------------------------------
void CBridgePartyVideoRelayIn::UpdateMrmpStreamIsMustAck( DWORD status )
{
	if (m_pSvcToAvcTranslator)
		m_pSvcToAvcTranslator->UpdateMrmpStreamIsMustAck( status );
	else
	{
		TRACEINTO << " m_pSvcToAvcTranslator=NULL, Party: " << m_pBridgePartyCntl->GetFullName() << ", status: " << status;
	}
}


// ------------------------------------------------------------
//void CBridgePartyVideoRelayIn::SvcToAvcTranslatorEnded()
//{
//	// delete SvcToAvcTranslator routing table
//	RemoveSvcToAvcStateMachineFromRoutingTable();
//}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::NullActionFunction(CSegment* pParam)
{
	TRACEINTO << " - Party Name: " << m_pBridgePartyCntl->GetFullName();
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeUpgradeTraslateIDLE(CSegment* pParam)
{
	TRACEINTO << " - save the command in parameter, Party Name: " << m_pBridgePartyCntl->GetFullName();
	m_bNeedToConnectSvcAvcTranslator = true;	// Update the parameter on doing the Translator
	ReplayConnectSvcToAvcTranslator( statOK );	// replay to Mix connect request, as agreed: OK in IDLE state
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeUpgradeTraslateSETUP(CSegment* pParam)
{
	TRACEINTO << " - Party Name: " << m_pBridgePartyCntl->GetFullName();

	if (!m_pSvcToAvcTranslator)	// translator not exists
	{
		// there are 2 options:
		//	1. Not Ready (m_isReady=false) yet: Just update Translator parameter and reply OK
		if (!m_isReady)
		{
			m_bNeedToConnectSvcAvcTranslator = true;
			ReplayConnectSvcToAvcTranslator( statOK );
			return;
		}
		DBGPASSERT(101);
		ReplayConnectSvcToAvcTranslator( statIllegal );
		TRACEINTO << " - SVC-Translator-Error - unexpected situation for start mix: <no translator> and <SETUP state>, Party Name: " << m_pBridgePartyCntl->GetFullName();
	}
	else // translator exists
	{
		// The translator should be in connecting process, we will send the real connection status as party of connection report upon end connect.
		ReplayConnectSvcToAvcTranslator( statOK );
	}
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeUpgradeTraslateCONNECTED(CSegment* pParam)
{
	if (m_pSvcToAvcTranslator)
		OnConnectSvcAvcTranslatorReqExistingTranslator();
	else
		OnConnectSvcAvcTranslatorReqNotExistingTranslator();
}

////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnConnectSvcAvcTranslatorReqExistingTranslator()
{
	TRACEINTO << " - Upgrade to an existing translator, Party Name: " << m_pBridgePartyCntl->GetFullName();
	m_bNeedToReplayConnectSvcToAvcTranslator = true;
	m_pSvcToAvcTranslator->Connect();

}

////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnConnectSvcAvcTranslatorReqNotExistingTranslator()
{
	TRACEINTO << " - Need to start Translator, replay later after connect, Party Name: " << m_pBridgePartyCntl->GetFullName();
	m_bNeedToConnectSvcAvcTranslator = true;
	int translatorStatus = InitOrUpdateSvcToAvcTranslator();
	if (WAIT_FOR_TRANSLATOR == translatorStatus)
	{
		TRACEINTO << " - waiting for SvcToAvcTranslator to connect, Party Name: " << m_pBridgePartyCntl->GetFullName();
		m_bNeedToReplayConnectSvcToAvcTranslator = true;
		return;
	}
	if (TRANSLATOR_ERROR == translatorStatus)
	{
		TRACEINTO << " SVC-Translator-Error - Party Name: " << m_pBridgePartyCntl->GetFullName();
		ReplayConnectSvcToAvcTranslator( statInconsistent );
		return;	// waiting for the global "Connect Party timer" to jump, stays in SETUP state
	}

	// normally should not happened, e.g. not MixMode conference
	ReplayConnectSvcToAvcTranslator( statInconsistent );
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeUpgradeTraslateDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << " - ignore connect translator at this stage, Party Name: " << m_pBridgePartyCntl->GetFullName();

	// currently the decision: do not reply
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::ReplayConnectSvcToAvcTranslator( EStat status )
{
	m_bNeedToReplayConnectSvcToAvcTranslator = false;
	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->ReplayUpgradeSvcToAvcTranslate( status );
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::ReplayDisconnectSvcToAvcTranslator( EStat status )
{
	m_bNeedToReplayDisconnectSvcToAvcTranslator = false;
	((CVideoRelayBridgePartyCntl*)m_pBridgePartyCntl)->ReplayDowngradeSvcToAvcTranslate( status );
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeTraslatorDisconnectSETUP(CSegment* pParam)
{
	TRACEINTO << " - dynamic disconnect";

	// xxx - does it mean not to connect in the future by self-decision (e.g. upon UpdateParams)? Only by dynamic connect?

	// disconnect Translator
	if (!m_pSvcToAvcTranslator)
	{
		TRACEINTO << " - SVC-Translator-Error - Party Name: " << m_pBridgePartyCntl->GetFullName();
		ReplayDisconnectSvcToAvcTranslator( statOK );
		return;
	}

	// m_translatorState = dynamicDisconnectByBridge;	// xxx - need to know that it is dynamic disconnect (not upon error)
	m_pSvcToAvcTranslator->Disconnect();
	m_bSvcAvcTranslatorReady = false;	// update about translator is not ready (for the next possible connect translator)
	m_bNeedToReplayDisconnectSvcToAvcTranslator = true;

	// xxx - Keep connect Video-In if ready (e.g. if the connection was waiting for the Translator
	if (m_isReady)
		SetVideoInConnected( statOK );

}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::OnVideoBridgeTraslatorDisconnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << " - dynamic disconnect";

	// m_translatorState = dynamicDisconnectByBridge;

	// just disconnect the translator
	if (!m_pSvcToAvcTranslator)
	{
		TRACEINTO << " - SVC-Translator-Error - Party Name: " << m_pBridgePartyCntl->GetFullName();
		ReplayDisconnectSvcToAvcTranslator( statOK );
		return;
	}

	// m_translatorState = dynamicDisconnectByBridge;	// xxx - need to know that it is dynamic disconnect (not upon error)
	m_pSvcToAvcTranslator->Disconnect();
	m_bSvcAvcTranslatorReady = false;	// update about translator is not ready (for the next possible connect translator)
	m_bNeedToReplayDisconnectSvcToAvcTranslator = true;
}

/////////////////////////////////////////////////////
void CBridgePartyVideoRelayIn::UpgradeSvcToAvcTranslator()
{
	DispatchEvent(UPGRADE_TO_MIX_AVC_SVC, NULL);
}
/////////////////////////////////////////////////////

void CBridgePartyVideoRelayIn::GetPortsOpened(std::map<eLogicalResourceTypes,bool>& isOpenedRsrcMap)
{
	if (m_pSvcToAvcTranslator && m_bSvcAvcTranslatorReady)
	{
		isOpenedRsrcMap[eLogical_video_decoder] = true;
	}
}
