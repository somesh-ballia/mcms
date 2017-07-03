#include "IsdnPartyCntl.h"
#include "BridgeMoveParams.h"
#include "AudioBridgeInterface.h"
#include "PartyApi.h"
#include "TraceStream.h"
#include "IpCommon.h"
#include "H264Util.h"
#include "ContentBridgePartyInitParams.h"
#include "H320ComMode.h"
#include "H263VideoMode.h"

const WORD REMOTE_XMIT_MODE = RMTXFRMODE;
const WORD REMOTE_BAS_CAPS  = RMTCAP;

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

PBEGIN_MESSAGE_MAP(CIsdnPartyCntl)
	ONEVENT(REALLOCATE_PARTY_RSRC_IND         ,ANYCASE,      CIsdnPartyCntl::OnRsrcReAllocatePartyRspReAllocate)
	ONEVENT(PARTY_IN_CONF_IND                 ,ANYCASE,      CIsdnPartyCntl::OnCAMUpdatePartyInConf)
	ONEVENT(PARTY_AUDIO_DISCONNECTED          ,ANYCASE,      CIsdnPartyCntl::OnAudBrdgDisconnect)
	ONEVENT(AUDIOMUTE                         ,ANYCASE,      CIsdnPartyCntl::OnPartyMuteAudioAnycase)
	ONEVENT(REMOTE_XMIT_MODE                  ,ANYCASE,      CIsdnPartyCntl::OnRmtXmitMode)
	ONEVENT(TMPPHONENUMBER_FREE               ,ANYCASE,      CIsdnPartyCntl::OnPartyDeallocateBondingTmpPhoneNumber)
	ONEVENT(UPDATENETCHANNEL                  ,ANYCASE,      CIsdnPartyCntl::NullActionFunction)
	ONEVENT(SMART_RECOVERY                    ,ANYCASE,      CIsdnPartyCntl::OnSmartRecovery)
	ONEVENT(VIDREFRESH                        ,ANYCASE,      CIsdnPartyCntl::NullActionFunction)
	ONEVENT(SET_PARTY_AVC_SVC_MEDIA_STATE     ,ANYCASE,      CIsdnPartyCntl::OnSetPartyAvcSvcMediaState)

PEND_MESSAGE_MAP(CIsdnPartyCntl,CPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
CIsdnPartyCntl::CIsdnPartyCntl()
{
	m_networkType                                  = E_NETWORK_TYPE_DUMMY;
	ReAllocateRtmRequest                           = false;
	m_pLocalCap                                    = NULL;
	m_pRemoteCap                                   = NULL;
	m_pCurrentReceiveScm                           = NULL;
	m_pCurrentTransmitScm                          = NULL;
	m_pTargetReceiveScm                            = NULL;
	m_pTargetTransmitScm                           = NULL;
	m_pConfScm                                     = NULL;
	m_bIsAckOnCreatePartyReceived                  = false;
	m_bIsAlreadyUpgradeToMixAvcToSvc               = false;
	m_ssrcAudio                                    = 0;
	m_bIsAckOnAvcToSvcArtTransalatorReceived       = false;

	OFF(m_WaitForEndChangeModeToConnectContent);
	OFF(m_isSmartRecovery);
	OFF(m_EpDidNotOpenContent);
	OFF(m_presentationStreamOutIsUpdated);
}

//--------------------------------------------------------------------------
CIsdnPartyCntl::~CIsdnPartyCntl()
{
}

//--------------------------------------------------------------------------
CIsdnPartyCntl& CIsdnPartyCntl::operator=(const CIsdnPartyCntl& other)
{
	if (this != &other)
	{
		m_netSetUp                               = other.m_netSetUp;
		m_networkType                            = other.m_networkType;
		ReAllocateRtmRequest                     = other.ReAllocateRtmRequest;
		m_pLocalCap                              = other.m_pLocalCap;
		m_pRemoteCap                             = other.m_pRemoteCap;
		m_pCurrentReceiveScm                     = other.m_pCurrentReceiveScm;  // EP --> MCU
		m_pCurrentTransmitScm                    = other.m_pCurrentTransmitScm; // MCU --> EP
		m_pTargetReceiveScm                      = other.m_pTargetReceiveScm;   // the target scm we would like to receive from EP after change mode ends
		m_pTargetTransmitScm                     = other.m_pTargetTransmitScm;  // the target scm we would like to transmit to EP after change mode ends
		m_pConfScm                               = other.m_pConfScm;
		m_presentationStreamOutIsUpdated         = other.m_presentationStreamOutIsUpdated;
		m_isSmartRecovery                        = other.m_isSmartRecovery;
		m_EpDidNotOpenContent                    = other.m_EpDidNotOpenContent;
		m_ssrcAudio                              = other.m_ssrcAudio;
		m_bIsAlreadyUpgradeToMixAvcToSvc         = other.m_bIsAlreadyUpgradeToMixAvcToSvc;
		m_bIsAckOnCreatePartyReceived            = other.m_bIsAckOnCreatePartyReceived;
		m_bIsAckOnAvcToSvcArtTransalatorReceived = other.m_bIsAckOnAvcToSvcArtTransalatorReceived;

		(CPartyCntl&) *this = (CPartyCntl&)other;
	}

	return *this;
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::Destroy()
{
	POBJDELETE(m_pLocalCap);
	POBJDELETE(m_pRemoteCap);
	POBJDELETE(m_pCurrentReceiveScm);
	POBJDELETE(m_pCurrentTransmitScm);
	POBJDELETE(m_pTargetReceiveScm);
	POBJDELETE(m_pTargetTransmitScm);
	POBJDELETE(m_pConfScm);

	CPartyCntl::Destroy();
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::SetDataForImportVoicePartyCntl(CIsdnPartyCntl& OtherPartyCntl)
{
	m_pInfoSeg                = OtherPartyCntl.m_pInfoSeg; // moves the segment pointer to the new party
	OtherPartyCntl.m_pInfoSeg = NULL;                      // for not delete it twice.

	m_voice = OtherPartyCntl.m_voice;

	POBJDELETE(m_pPartyAllocatedRsrc);
	m_pPartyAllocatedRsrc = new CPartyRsrcDesc(*(OtherPartyCntl.m_pPartyAllocatedRsrc));
	m_pPartyAllocatedRsrc->SetConfRsrcId(OtherPartyCntl.m_destResourceConfId);

	POBJDELETE(m_pBridgeMoveParams);
	m_pBridgeMoveParams = new CBridgeMoveParams(*(OtherPartyCntl.m_pBridgeMoveParams));

	if (OtherPartyCntl.m_service_provider != NULL)
	{
		m_service_provider = new char[NET_SERVICE_PROVIDER_NAME_LEN]; // ????
		strncpy(m_service_provider, OtherPartyCntl.m_service_provider, NET_SERVICE_PROVIDER_NAME_LEN-1);
		m_service_provider[NET_SERVICE_PROVIDER_NAME_LEN-1] = '\0';
	}
	else
		m_service_provider = NULL;

	m_pParty                      = OtherPartyCntl.m_pParty;
	m_numChnl                     = OtherPartyCntl.m_numChnl;
	m_chnlWidth                   = OtherPartyCntl.m_chnlWidth;
	m_type                        = OtherPartyCntl.m_type;
	m_monitorPartyId              = OtherPartyCntl.m_monitorPartyId;
	m_monitorConfId               = OtherPartyCntl.m_monitorConfId;
	m_moveType                    = OtherPartyCntl.m_moveType;
	m_isFullBitRateConnect        = OtherPartyCntl.m_isFullBitRateConnect;


	m_name = new char[H243_NAME_LEN]; // ????
	strncpy(m_name, OtherPartyCntl.m_name, H243_NAME_LEN-1);
	m_name[H243_NAME_LEN-1] = '\0';

	m_pPartyApi = new CPartyApi;      // ????
	CPartyApi* pPartyCntl = OtherPartyCntl.m_pPartyApi;
	PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyCntl));

	m_pPartyApi->CreateOnlyApi(pPartyCntl->GetRcvMbx());

	m_interfaceType                  = OtherPartyCntl.m_interfaceType;
	m_isUndefParty                   = OtherPartyCntl.m_isUndefParty;
	m_isRemoteDBC2                   = OtherPartyCntl.m_isRemoteDBC2;
	m_monitorConfId                  = OtherPartyCntl.m_destMonitorConfId;
	m_monitorPartyId                 = OtherPartyCntl.m_destMonitorPartyId;
	m_destResourceConfId             = OtherPartyCntl.m_destResourceConfId;
	m_presentationStreamOutIsUpdated = OtherPartyCntl.m_presentationStreamOutIsUpdated;
	m_EpDidNotOpenContent            = OtherPartyCntl.m_EpDidNotOpenContent;
	m_netSetUp                       = OtherPartyCntl.m_netSetUp;
	m_networkType                    = OtherPartyCntl.m_networkType;
	m_bIsAckOnCreatePartyReceived    = OtherPartyCntl.m_bIsAckOnCreatePartyReceived;
	m_bIsAlreadyUpgradeToMixAvcToSvc = OtherPartyCntl.m_bIsAlreadyUpgradeToMixAvcToSvc;
	m_ssrcAudio                      = OtherPartyCntl.m_ssrcAudio;

	if (m_pBridgeMoveParams != NULL)
		POBJDELETE(m_pBridgeMoveParams);

	m_pBridgeMoveParams = new CBridgeMoveParams(*(OtherPartyCntl.m_pBridgeMoveParams));

	POBJDELETE(m_pLocalCap);
	if (OtherPartyCntl.m_pLocalCap != NULL)
		m_pLocalCap = new CCapH320(*(OtherPartyCntl.m_pLocalCap));

	POBJDELETE(m_pRemoteCap);
	if (OtherPartyCntl.m_pRemoteCap != NULL)
		m_pRemoteCap = new CCapH320(*(OtherPartyCntl.m_pRemoteCap));

	POBJDELETE(m_pCurrentReceiveScm);
	if (OtherPartyCntl.m_pCurrentReceiveScm != NULL)
		m_pCurrentReceiveScm = new CComMode(*(OtherPartyCntl.m_pCurrentReceiveScm));

	POBJDELETE(m_pCurrentTransmitScm);
	if (OtherPartyCntl.m_pCurrentTransmitScm != NULL)
		m_pCurrentTransmitScm = new CComMode(*(OtherPartyCntl.m_pCurrentTransmitScm));

	POBJDELETE(m_pTargetReceiveScm);
	if (OtherPartyCntl.m_pTargetReceiveScm != NULL)
		m_pTargetReceiveScm = new CComMode(*(OtherPartyCntl.m_pTargetReceiveScm));

	POBJDELETE(m_pTargetTransmitScm);
	if (OtherPartyCntl.m_pTargetTransmitScm != NULL)
		m_pTargetTransmitScm = new CComMode(*(OtherPartyCntl.m_pTargetTransmitScm));

	POBJDELETE(m_pConfScm);
	if (OtherPartyCntl.m_pConfScm != NULL)
		m_pConfScm = new CComMode(*(OtherPartyCntl.m_pConfScm));
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::OnCAMUpdatePartyInConf(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	ON(m_isPartyInConf);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::OnAudBrdgDisconnect(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam);
	if (bIsDisconnectOk == FALSE)
	{
		BYTE mipHwConn    = (BYTE) eMipBridge;
		BYTE mipMedia     = (BYTE) eMipAudio;
		BYTE mipDirect    = 0;
		BYTE mipTimerStat = 0;
		BYTE mipAction    = 0;
		*pParam >> mipDirect >> mipTimerStat >> mipAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;

		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, MpiErrorNumber);
		POBJDELETE(pSeg);
	}
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::OnPartyMuteAudioAnycase(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	WORD   onOff;
	*pParam >> onOff;
	EOnOff eOnOff = onOff ? eOn : eOff;
	if (IsInDirectionConnectedToAudioBridge())
		m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, eOnOff, PARTY);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::RedialIfNeeded()
{
	TRACEINTO << m_partyConfName;

	// redial if its dial out and the disconnection is because network problems.
	DWORD bIsEnableRedial = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_ISDN_REDIAL);
	DWORD numOfRedials    = GetSystemCfgFlagInt<DWORD>(CFG_KEY_NUMBER_OF_REDIAL);

	// Make sure redail is allowed
	if (GetDialType() != DIALOUT || !bIsEnableRedial)
		return;

	// DialOut call  - if we do not need or finished Redialing return
	if (!IsNetworkProblemsDisconnection() || m_redial <= 0)
	{
		TRACEINTO << m_partyConfName << ", redialCounter:" << m_redial << " - Not redialing";

		m_redial = numOfRedials;
		m_pTaskApi->UpdateDB(m_pParty, NUMRETRY, (DWORD)m_redial, 1);
		return;
	}

	TRACEINTO << m_partyConfName << ", redialCounter:" << m_redial << " - Redialing...";

	m_redial--;
	m_pTaskApi->UpdateDB(m_pParty, NUMRETRY, (DWORD) m_redial, 0);
	m_pTaskApi->RedailParty(m_pParty);
}

//--------------------------------------------------------------------------
bool CIsdnPartyCntl::IsNetworkProblemsDisconnection() const
{
	if (STATUS_OK != m_disconnectionCause)
		return true;

	return false;
}

//--------------------------------------------------------------------------
eVideoPartyType CIsdnPartyCntl::GetVideoPartyTypeAccordingToCapabilities(CCapH320* pCap, bool isLocalCaps)
{
	eVideoPartyType  videoPartyType  = eVideo_party_type_none;
	eSystemCardsMode systemCardsMode = GetSystemCardsBasedMode();
	if (!m_voice)
	{
		if (m_pConfScm->IsFreeVideoRate() == TRUE)
		{
			eVideoQuality vidQuality            = m_pConf->GetCommConf()->GetVideoQuality();
			DWORD         callRate              = m_numChnl * rate64K;
			APIS8         h263_4CifMpi          = CH263VideoMode::GetH263Cif4VideoCardMPI(callRate/100 -1, vidQuality);
			BYTE          isH2634Cif15Supported = NO;

			if (h263_4CifMpi == 2)
			{
				// at the moment we don't support H.2634cif in cascade isdn
				if ((WORD)CASCADE_MODE_MCU != GetNodeType())
				{
					TRACEINTO << "H263 4CIF 15 is supported";
					isH2634Cif15Supported = YES;
				}
			}

			if (isLocalCaps)
			{
				TRACEINTO << "Using local caps";
				videoPartyType = pCap->GetCPVideoPartyTypeAccordingToLocalCapabilities(isH2634Cif15Supported);
			}
			else
			{
				videoPartyType = pCap->GetCPVideoPartyTypeAccordingToCapabilities(isH2634Cif15Supported);
			}
		}
		else
		{
			// VSW
			if (pCap->IsCapableOfVideo())
			{
				videoPartyType = eVSW_video_party_type;
			}
		}
	}

	TRACEINTO << m_partyConfName << ", VideoPartyType:" << eVideoPartyTypeNames[videoPartyType];

	return videoPartyType;
}

//--------------------------------------------------------------------------
bool CIsdnPartyCntl::IsCpH263_4Cif_SupportedByRateAndVideoQuality()
{
	bool isSupported = false;

	if (!m_voice)                                // not relevant to voice
	{
		if (m_pConfScm->IsFreeVideoRate() == TRUE) // CP check
		{
			eVideoQuality vidQuality   = m_pConf->GetCommConf()->GetVideoQuality();
			DWORD         callRate     = m_numChnl * rate64K;
			APIS8         h263_4CifMpi = CH263VideoMode::GetH263Cif4VideoCardMPI(callRate/100 -1, vidQuality);
			if (h263_4CifMpi == 2)
			{
				// at the moment we don't support H.2634cif in cascade isdn
				if ((WORD)CASCADE_MODE_MCU != GetNodeType())
				{
					TRACEINTO << "H263 4CIF 15 is supported";
					isSupported = true;
				}
			}
		}
	}

	return isSupported;
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::ReAllocatePartyResourcesIfNeeded()
{
	eVideoPartyType eRemoteVideoType = GetVideoPartyTypeAccordingToCapabilities(m_pRemoteCap);
	eVideoPartyType eLocalVideoType  = GetVideoPartyTypeAccordingToCapabilities(m_pLocalCap);

	std::ostringstream msg;
	msg.precision(0);
	msg << m_partyConfName
			<< "\n  VideoPartyType[remote capabilities] :" << eVideoPartyTypeNames[eRemoteVideoType]
			<< "\n  VideoPartyType[local capabilities ] :" << eVideoPartyTypeNames[eLocalVideoType]
			<< "\n  VideoPartyType[currently allocated] :" << eVideoPartyTypeNames[m_eLastAllocatedVideoPartyType];

	// VNGR-25474 & VNGR-25727 D.K.
	// CIF_H263 need 1.5 port
	// CIF_H264 need 1 port
	// It was decided with Ron to reallocate resources for H264_CIF_video_party_type only
	if (/*GetMaxVideoModeBySysCfg() == eCIF30 && */ eLocalVideoType == eCP_H264_upto_CIF_video_party_type && m_pRemoteCap->IsH264())
	{
		msg << "\n  ---------------------------------------------------------------------------"
				<< "\n  Decision #1 - Reallocate to " << eVideoPartyTypeNames[eLocalVideoType];
		TRACEINTO <<  msg.str().c_str();
		CreateAndSendReAllocatePartyResources(eISDN_network_party_type, eLocalVideoType, eAllocateAllRequestedResources, FALSE, YES);
		return TRUE;
	}

	if ((m_eLastAllocatedVideoPartyType > eRemoteVideoType) && (eCP_H261_CIF_equals_H264_HD1080_video_party_type != m_eLastAllocatedVideoPartyType)) // we allocated more then remote has
	{
		msg << "\n  ---------------------------------------------------------------------------"
				<< "\n  Decision #2 - Reallocate to " << eVideoPartyTypeNames[eRemoteVideoType];
		TRACEINTO <<  msg.str().c_str();
		CreateAndSendReAllocatePartyResources(eISDN_network_party_type, eRemoteVideoType, eAllocateAllRequestedResources, FALSE, YES);
		return TRUE;
	}

    eSystemCardsMode systemCardsBasedMode = GetSystemCardsBasedMode();
	if (systemCardsBasedMode == eSystemCardsMode_mpmrx)
    {
        if (m_eLastAllocatedVideoPartyType == eCP_H261_CIF_equals_H264_HD1080_video_party_type || eRemoteVideoType == eCP_H261_CIF_equals_H264_HD1080_video_party_type)
        {
            msg << "\n  ---------------------------------------------------------------------------"
    				<< "\n  Decision #3 - Reallocate to " << eVideoPartyTypeNames[eCP_H261_CIF_equals_H264_HD1080_video_party_type];

            TRACEINTO <<  msg.str().c_str();
    		CreateAndSendReAllocatePartyResources(eISDN_network_party_type, eCP_H261_CIF_equals_H264_HD1080_video_party_type, eAllocateAllRequestedResources, FALSE, YES);
    		return TRUE;
        }
    }
    
	TRACEINTO << msg.str().c_str();
	return FALSE;
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CIsdnPartyRsrcDesc* pTempPartyAllocatedRsrc = new CIsdnPartyRsrcDesc;
	pTempPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	pTempPartyAllocatedRsrc->DumpToTrace();

	BYTE             bSameVideoPartyType  = (m_eLastAllocatedVideoPartyType == m_eLastReAllocRequestVideoPartyType);
	BYTE             bAllocationFailed    = IsReallocateResponseValid(pTempPartyAllocatedRsrc);
	BYTE             bCapsChanged         = FALSE;


	if (bAllocationFailed != TRUE && !bSameVideoPartyType)
	{
		// update local caps and target scm according to new allocation
		bCapsChanged = UpdateH264ModeInLocalCaps();

		if (m_eLastAllocatedVideoPartyType < eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type)
			RemoveH2634CifFromTargetScm();
	}

	if (ReAllocateRtmRequest == true)
	{
		m_pPartyApi->OnRsrcAllocatorReAllocateRTMAck(pTempPartyAllocatedRsrc, bAllocationFailed);
		if (bCapsChanged)
			m_pPartyApi->UpdateLocalCaps(m_pLocalCap);

		m_pTaskApi->UpdateDB(m_pParty, SETPARTYCHNLNUM, m_numChnl, 1); // VNGR-7708
		ReAllocateRtmRequest = false;
	}
	else
	{
		// this function should be executed in child according to the state we are in (add \ change mode)
		HandleReallocateResponse(bAllocationFailed, bCapsChanged);
	}

	POBJDELETE(pTempPartyAllocatedRsrc);
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::UpdateH264ModeInLocalCaps()
{
	if (!m_pLocalCap->IsH264())
		return FALSE;

	TRACEINTO << m_partyConfName;

	H264VideoModeDetails h264VidModeDetails;
	Eh264VideoModeType   resourceMaxVideoMode = TranslateCPVideoPartyTypeToMaxH264VideoModeType(m_eLastAllocatedVideoPartyType);
	eVideoQuality        vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
	DWORD                callRate   = m_numChnl * rate64K;

	// isdn behaves like it does not support high profile in low rates
	// high profile resolution slider threshold for version 7.0.1
	// GetH264VideoParams(h264VidModeDetails, callRate, vidQuality, resourceMaxVideoMode,FALSE);

	// for version 7.0.2 it depends on system flag SUPPORT_HIGH_PROFILE_WITH_ISDN
	BOOL bEnableHighfProfileInIsdn = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_HIGH_PROFILE_WITH_ISDN);
	GetH264VideoParams(h264VidModeDetails, callRate, vidQuality, resourceMaxVideoMode, bEnableHighfProfileInIsdn);

	m_pLocalCap->RemoveH264Caps();

	BYTE newLevel = (BYTE)h264VidModeDetails.levelValue;
	WORD newMbps  = (WORD)h264VidModeDetails.maxMBPS;
	WORD newFs    = (WORD)h264VidModeDetails.maxFS;
	WORD newDpb   = (WORD)h264VidModeDetails.maxDPB;
	WORD newBrCpb = ConvertMaxBrToMaxBrAndCpb((DWORD)h264VidModeDetails.maxBR);

	BYTE prof =  H264_Profile_BaseLine;
	if(bEnableHighfProfileInIsdn)
		prof = H264_Profile_High;

	CCapSetH264* pCapSetH264 = new CCapSetH264;
	pCapSetH264->Create(newLevel, newMbps, newFs, newDpb, newBrCpb,	H264_ALL_LEVEL_DEFAULT_SAR, prof);

	m_pLocalCap->AddH264CapSet(pCapSetH264);

	// if local cap has changed -> we have to update the ConfScm and the target Scm
	if (m_pConfScm->GetVidMode() == H264)
	{
		m_pConfScm->m_vidMode.SetH264VidMode(*pCapSetH264);
	}

	if (m_pTargetReceiveScm->GetVidMode() == H264)
	{
		PTRACE(eLevelInfoNormal, "**m_pTargetReceiveScm Video mode before update:");
		m_pTargetReceiveScm->m_vidMode.Dump();

		CVidMode TargetReceiveVideoMode = m_pTargetReceiveScm->m_vidMode;
		WORD     IsAutoVidScm           = TargetReceiveVideoMode.IsAutoVidScm();

		CVidMode IntersectVidMode;
		TargetReceiveVideoMode.Intersect(m_pLocalCap, IsAutoVidScm, IntersectVidMode);
		m_pTargetReceiveScm->SetVidMode(IntersectVidMode);

		PTRACE(eLevelInfoNormal, "**m_pTargetReceiveScm Video mode After update:");
		m_pTargetReceiveScm->m_vidMode.Dump();
	}

	PTRACE(eLevelInfoNormal, "**m_pTargetTransmitScm Video mode before update:");
	m_pTargetTransmitScm->m_vidMode.Dump();
	m_pTargetTransmitScm->m_vidMode.m_H264VidMode.CalcAndSetCommonLevelAndParam(m_pConfScm->m_vidMode.GetH264VidMode());
	PTRACE(eLevelInfoNormal, "**m_pTargetTransmitScm Video mode After update:");
	m_pTargetTransmitScm->m_vidMode.Dump();

	return TRUE;
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::IsReallocateResponseValid(CPartyRsrcDesc* pTempPartyAllocatedRsrc)
{
	DWORD status            = pTempPartyAllocatedRsrc->GetStatus();
	BYTE  bAllocationFailed = FALSE;

	if (status != STATUS_OK)
	{
		TRACEINTO << m_partyConfName << ", status:" << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << " - Reallocation failed, do not continue process";
		bAllocationFailed = TRUE;
	}
	else
	{
		eVideoPartyType requestedVideoPartyType = m_eLastReAllocRequestVideoPartyType;
		eVideoPartyType allocatedVideoPartyType = pTempPartyAllocatedRsrc->GetVideoPartyType();
		if (requestedVideoPartyType >= allocatedVideoPartyType)
		{
			m_eLastAllocatedVideoPartyType = allocatedVideoPartyType;
			TRACEINTO << m_partyConfName << " - Reallocation is OK";
		}
		else // if (requestedPartyType < allocatedPartyType)
		{
			TRACEINTO << m_partyConfName << " - Reallocation failed, higher allocation than requested";
			bAllocationFailed = TRUE;
		}
	}

	return bAllocationFailed;
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::DispatchChangeModeEvent()
{
	CSegment* pParam = NULL;
	DispatchEvent(SCMCHANGEMODE, pParam);
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::IsRemoteAndLocalCapSetHasContent(eToPrint toPrint) const
{
	return IsPartyH239();
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::IsRemoteContentFailed() const
{
	// we currently check that m_pCurrentTransmitScm is OFF, means we remove content from transmit scm
	// as set in function CIsdnChangeModeCntl::OnTimerChangeContent
	// in isdn we open content at 0k at the start, and don't close it during the call
	if (m_EpDidNotOpenContent)
	{
		TRACEINTO << m_partyConfName;
		return TRUE;
	}
	return FALSE;
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::IsPartyH239() const
{
	if (m_pRemoteCap && m_pLocalCap)
	{
		if (m_pRemoteCap->IsH239Cap() && m_pLocalCap->IsH239Cap() && IsRemoteContentFailed() == FALSE)
			return TRUE;
	}

	return FALSE;
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::OnRmtXmitMode(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CSegment rmtXmitSeg; // (*pParam); /* this segment is empty after using copy constructor */
	CComMode newScm;
	newScm.DeSerialize(NATIVE, *pParam);
	ostringstream ostr;
	newScm.Dump(ostr);
	newScm.Serialize(NATIVE, rmtXmitSeg);

	// Support for RV P20 GW behavior(bug) -The RadVision does NOT send Presentation Token Release when it stop the content
	if (FALSE == newScm.m_contentMode.IsContentModeOn())                 // EP close the content.
	{
		if (m_pContentBridge && m_pContentBridge->IsTokenHolder(m_pParty)) // The EP is the token holder and close the content.
		{
			CSegment* pParam = new CSegment;
			*pParam << (BYTE)(Start_Mbe | ESCAPECAPATTR);
			*pParam << (BYTE)0 << (BYTE)H239_messsage;
			*pParam << (BYTE) PresentationTokenRelease;
			BYTE mcuNum = (BYTE)1;
			BYTE terminalNum = (BYTE)m_termNum;
			SerializeGenericParameter(TerminalLabelParamIdent,((mcuNum*TERMINAL_LABEL_MULTIPLIER)+terminalNum),pParam);
			SerializeGenericParameter(ChannelIdParamIdent,SecondVideoChannel,pParam);
			TRACEINTO << m_partyConfName << " - Content speaker close content, so simulate PresentationTokenRelease arrived from party";
			PTRACE2(eLevelInfoNormal, "CIsdnPartyCntl::OnRmtXmitMode Content speaker close content ==> Simulate PresentationTokenRelease arrived from party: Name - ", m_partyConfName);
			m_pPartyApi->MuxRmtH230(pParam);
			POBJDELETE(pParam);
		}
	}

	// End of RV bug

	*m_pCurrentReceiveScm = newScm;

	// Because we don't know from the scm which custom parameters remote opened
	// and since we have to tell to the video bridge those parameters
	// we will set them as RemoteVideoMode /\ LocalCap (Eitan 01/2008)
	WORD     IsAutoVidScm    = m_pConfScm->m_vidMode.IsAutoVidScm();
	CVidMode RemoteVideoMode = m_pCurrentReceiveScm->m_vidMode;
	CVidMode IntersectVidMode;
	RemoteVideoMode.Intersect(m_pLocalCap, IsAutoVidScm, IntersectVidMode);
	m_pCurrentReceiveScm->SetVidMode(IntersectVidMode);

	m_pTaskApi->UpdateDB(m_pParty, REMOTECOMMODE, (DWORD)0, 1, &rmtXmitSeg);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::OnPartyDeallocateBondingTmpPhoneNumber(CSegment* pParam)
{
	CConfParty* pConfParty = 0;
	if (!(m_pConf->GetCommConf()) || !(pConfParty = (m_pConf->GetCommConf())->GetCurrentParty(m_name))) {
		PASSERTMSG_AND_RETURN(1, "Can not find party");
	}

	if (!strcmp(pConfParty->GetBondingTmpNumber(), ""))
		return;

	DEALLOCATE_BONDING_TEMP_PHONE_S deAllocateBondingTmpPhone;
	memset(&deAllocateBondingTmpPhone, 0, sizeof(DEALLOCATE_BONDING_TEMP_PHONE_S));

	deAllocateBondingTmpPhone.monitor_conf_id  = m_monitorConfId;  // Or m_pPartyAllocatedRsrc->GetConfRsrcId()??
	deAllocateBondingTmpPhone.monitor_party_id = m_monitorPartyId; // Or m_pPartyAllocatedRsrc->GetPartyRsrcId()??

	strncpy(deAllocateBondingTmpPhone.BondingTemporaryPhoneNumber, ((CIsdnPartyRsrcDesc*)m_pPartyAllocatedRsrc)->GetTmpPhoneNumber(), sizeof(deAllocateBondingTmpPhone.BondingTemporaryPhoneNumber)-1);
	deAllocateBondingTmpPhone.BondingTemporaryPhoneNumber[sizeof(deAllocateBondingTmpPhone.BondingTemporaryPhoneNumber)-1] = '\0';

	if (m_service_provider)
	{
		strncpy((char*)deAllocateBondingTmpPhone.serviceName, m_service_provider, sizeof(deAllocateBondingTmpPhone.serviceName)-1);
		deAllocateBondingTmpPhone.serviceName[sizeof(deAllocateBondingTmpPhone.serviceName)-1] = '\0';
	}
	else
	{
		deAllocateBondingTmpPhone.serviceName[0] = '\0';
	}

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&deAllocateBondingTmpPhone), sizeof(DEALLOCATE_BONDING_TEMP_PHONE_S));

	STATUS res = SendReqToResourceAllocator(seg, DEALLOCATE_BONDING_TEMP_PHONE_REQ);
	pConfParty->SetBondingTmpNumber("");
	PASSERT(res);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::HandleReallocateResponse(BYTE bAllocationFailed, BYTE bLocalCapsChanged)
{
	FTRACESTRFUNC(eLevelError) << "Failed, should be implemented at derived class";
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::DisconnectPartyFromVideoOut()
{
	EMediaDirection eDisconnectedDirection = eNoDirection;

	eDisconnectedDirection |= eMediaOut;
	int eOutDisconnecting = ~eOutConnected;
	m_eVidBridgeConnState &= (EBridgeConnectionState)eOutDisconnecting;

	if (eDisconnectedDirection == eNoDirection)
		return FALSE;

	TRACEINTO
		<< m_partyConfName
		<< ", PartyId:" << GetPartyId()
		<< ", MediaDirection:" << eDisconnectedDirection
		<< ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState);

	CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId(), eDisconnectedDirection);
	m_pConfAppMngrInterface->DisconnectPartyVideo(&bridgePartyDisconnectParams);
	return TRUE;
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::DisconnectPartyFromVideoIn()
{
	EMediaDirection eDisconnectedDirection = eNoDirection;

	eDisconnectedDirection |= eMediaIn;
	int eInDisconnecting = ~eInConnected;
	m_eVidBridgeConnState &= (EBridgeConnectionState)eInDisconnecting;

	if (eDisconnectedDirection == eNoDirection)
		return FALSE;

	TRACEINTO
		<< m_partyConfName
		<< ", PartyId:" << GetPartyId()
		<< ", MediaDirection:" << eDisconnectedDirection
		<< ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState);

	CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId(), eDisconnectedDirection);
	m_pConfAppMngrInterface->DisconnectPartyVideo(&bridgePartyDisconnectParams);
	return TRUE;
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::DisconnectPartyFromVideoInAndOut()
{
	EMediaDirection eDisconnectedDirection = eNoDirection;

	eDisconnectedDirection |= eMediaIn;
	int eInDisconnecting = ~eInConnected;
	m_eVidBridgeConnState &= (EBridgeConnectionState)eInDisconnecting;

	eDisconnectedDirection |= eMediaOut;
	int eOutDisconnecting = ~eOutConnected;
	m_eVidBridgeConnState &= (EBridgeConnectionState)eOutDisconnecting;

	if (eDisconnectedDirection == eNoDirection)
		return FALSE;

	TRACEINTO
		<< m_partyConfName
		<< ", PartyId:" << GetPartyId()
		<< ", MediaDirection:" << eDisconnectedDirection
		<< ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState);

	m_pTaskApi->UpdateDB(m_pParty, PARTYHIGHPROFILE, FALSE);

	CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId(), eDisconnectedDirection);
	m_pConfAppMngrInterface->DisconnectPartyVideo(&bridgePartyDisconnectParams);
	return TRUE;
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::RemoveH2634CifFromTargetScm()
{
	TRACEINTO << m_partyConfName;

	WORD  highestResolution = H263_CIF;
	APIS8 buffer[highestResolution+1];
	memset(buffer, -1, highestResolution+1);

	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
	DWORD         vidRate    = m_numChnl * rate64K;
	CH263VideoMode::Get263VideoCardMPI(vidRate/100 - 1, buffer, vidQuality);

	// when we update the h263 resolution and frame rate the video mode in the scm
	// changes to h263, so we must keep the original video mode and set it later on

	WORD originalVideoMode = m_pTargetTransmitScm->m_vidMode.GetVidMode();
	for (int i = H263_QCIF_SQCIF; i <= highestResolution; i++)
	{
		if (buffer[i] != -1)
		{
			// from H263.h
			// i = 263 Format (H263_QCIF_SQCIF = 0, H263_CIF = 1, ...)
			// buffer[i] - 263 MPI (frame rate) per format (MPI_1 = 0 (30 fps) , MPI_2 = 1 (15 fps), ...)
			m_pTargetTransmitScm->m_vidMode.SetH263VidMode(1, i, buffer[i]-1);
		}
	}

	m_pTargetTransmitScm->m_vidMode.SetVidMode(originalVideoMode);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::OnSmartRecovery(CSegment* pParam)
{
	FTRACESTRFUNC(eLevelError) << m_partyConfName << " - Inform conference and send kill port";
	ON(m_isSmartRecovery);    // in case of smart recovery send kill port without close port
	m_isFaulty = 1;           // in order to send kill port

	BYTE mipHwConn    = (BYTE)eMipMux;
	BYTE mipMedia     = (BYTE)eMipNoneMedia;
	BYTE mipDirect    = 0;
	BYTE mipTimerStat = (BYTE)eMipStatusFail;
	BYTE mipAction    = 0;

	CSegment* pSeg = new CSegment;
	*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
	DWORD MipErrorNumber = GetMpiErrorNumber(pSeg);
	m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, MipErrorNumber);
	POBJDELETE(pSeg);
}
//--------------------------------------------------------------------------
void CIsdnPartyCntl::OnSetPartyAvcSvcMediaState(CSegment* pParam)
{
	m_bPartyInUpgradeProcess = true;
}
//--------------------------------------------------------------------------
void CIsdnPartyCntl::DisableHD720AsymmetricFromTargetScm()
{
	TRACEINTO << m_partyConfName;
	m_pTargetTransmitScm->SetHd720Enabled(NO);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::DisableHD1080AsymmetricFromTargetScm()
{
	TRACEINTO << m_partyConfName;
	m_pTargetTransmitScm->SetHd1080Enabled(NO);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::DisableHD720At60AsymmetricFromTargetScm()
{
	TRACEINTO << m_partyConfName;
	m_pTargetTransmitScm->SetHd720At60Enabled(NO);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::DisableHD1080At60AsymmetricFromTargetScm()
{
	TRACEINTO << m_partyConfName;
	m_pTargetTransmitScm->SetHd1080At60Enabled(NO);
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::InitVideoParams(CComMode* pScm, CBridgePartyVideoParams* pMediaParams)
{
	WORD                      videoBridgeAlgorithm     = 0;
	DWORD                     videoBridgeBitRate       = 0;
	WORD                      videoBridgeFormat        = 0;
	int                       videoBridgeFrameRate     = 0;
	DWORD                     videoBridgeFs            = 0;
	DWORD                     videoBridgeMbps          = 0;
	DWORD                     videoBridgeSar           = 0;
	DWORD                     videoBridgeStaticMB      = DEFAULT_STATIC_MB; // static MB is currently not supported in ISDN
	eVideoProfile             videoBridgeProfile       = eVideoProfileBaseline;
	eVideoPacketPayloadFormat videoBridgePacketFormat  = eVideoPacketPayloadFormatSingleUnit;
	DWORD                     videoBridgeDPB           = 0;
	int                       videoBridgeQcifFrameRate = eVideoFrameRateDUMMY;
	int                       videoBridgeCifFrameRate  = eVideoFrameRateDUMMY;
	int                       videoBridge4CifFrameRate = eVideoFrameRateDUMMY;

	bool bOut = false;
	bool b264 = false;

	CMedString str;
	str << "CIsdnPartyCntl::InitVideoParams:";

	videoBridgeBitRate = VideoCanBeOpened(pScm);
	str << "\nDirection      - ";
	if (!strcmp(pMediaParams->NameOf(), "CBridgePartyVideoOutParams"))
	{
		// Eitan - according to Algorithm request reduce the video bitrate in 7%
		videoBridgeBitRate = videoBridgeBitRate * 93 /*BCH_FACTOR*/ / 100;
		str << "Out";
		bOut = true;
	}
	else
	{
		str << "In";
	}

	str << "\nBit rate       - " << (int)videoBridgeBitRate;


	BYTE  level   = 0;
	DWORD fs      = 0;
	DWORD mbps    = 0;
	DWORD sar     = 0;
	DWORD profile = 0;
	DWORD dpb     = 0;

	CVidMode ScmVidMode = pScm->m_vidMode;
	videoBridgeAlgorithm = ScmVidMode.GetVidMode();

	switch (videoBridgeAlgorithm)
	{
		case H264:
		{
			CH264VidMode Scm264VidMode = ScmVidMode.GetH264VidMode();
			level = Scm264VidMode.GetLevelValue();
			CH264Details thisH264Details(level);
			fs = Scm264VidMode.GetMaxFSasCustomWord();
			if (fs == 0xFFFF)
				fs = thisH264Details.GetDefaultFsAsDevision();

			mbps = Scm264VidMode.GetMaxMBPSasCustomWord();
			if (mbps == 0xFFFF)
				mbps = thisH264Details.GetDefaultMbpsAsDevision();

			sar = Scm264VidMode.GetSAR();
			if (sar == H264_ALL_LEVEL_DEFAULT_SAR)
				sar = 0;

			profile = Scm264VidMode.GetProfileValue();
			if (profile == H264_Profile_High)
			{
				videoBridgeProfile = eVideoProfileHigh;
				m_pTaskApi->UpdateDB(m_pParty, PARTYHIGHPROFILE, TRUE);
			}
			else
				m_pTaskApi->UpdateDB(m_pParty, PARTYHIGHPROFILE, FALSE);

			dpb = Scm264VidMode.GetMaxDPB();
			if (dpb == 0xFFFF)
				dpb = thisH264Details.GetDefaultDpbAsProduct();

			videoBridgeFs   = fs;
			videoBridgeMbps = mbps;
			videoBridgeSar  = sar;
			if (videoBridgeProfile == eVideoProfileHigh)
				videoBridgePacketFormat = eVideoPacketPayloadFormatFragmentationUnit;

			videoBridgeDPB = dpb;

			str << "\nVideo Alg      - H264";
			str << "\nMBPS           - " << mbps;
			str << "\nFS             - " << fs;
			str << "\nSAR            - " << sar;
			str << "\nProfile        - " << profile;
			str << "\nDPB            - " << dpb;

			b264 = true;
			break;
		}

		case H263:
		{ // format, frame rate
			WORD      format    = ScmVidMode.GetVidFormat();
			WORD      frameRate = ScmVidMode.GetH263Mpi();

			CCapH263* remoteH263Cap          = m_pRemoteCap->GetCapH263();
			BYTE      remoteCapQcifFrameRate = remoteH263Cap->GetFormatMPI(H263_QCIF_SQCIF);
			BYTE      remoteCapCifFrameRate  = remoteH263Cap->GetFormatMPI(H263_CIF);

			str << "\nVideo Alg      - H263";

			// VNGR-22728 - open port in H263 4 sif although allocation is h264 sd
			eVideoPartyType  videoPartyType4Cif = eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type;

			if (H263_CIF_4 == format && m_eLastAllocatedVideoPartyType < videoPartyType4Cif)
			{
				str << "\n";
				str << "Alg format     - 4CIF Removed due to allocation\n";
				format = H263_CIF;
			}

			videoBridgeFormat = TranslateIsdnFormatToVideoBridgeFormat(format, H263);
			switch (format)
			{
				case (H263_QCIF_SQCIF):
				{
					str << "Alg format     - QCIF, ";
					videoBridgeQcifFrameRate = TranslateIsdnFrameRateToVideoBridgeFrameRate(frameRate, H263, str);
					break;
				}
				case (H263_CIF):
				{
					str << "Alg format     - CIF, ";
					videoBridgeCifFrameRate = TranslateIsdnFrameRateToVideoBridgeFrameRate(frameRate, H263, str);
					str << "Alg format     - QCIF, ";
					videoBridgeQcifFrameRate = TranslateIsdnFrameRateToVideoBridgeFrameRate(remoteCapQcifFrameRate, H263, str);
					break;
				}
				case (H263_CIF_4):
				{
					str << "Alg format     - 4CIF, ";
					videoBridge4CifFrameRate = TranslateIsdnFrameRateToVideoBridgeFrameRate(frameRate, H263, str);
					str << "Alg format     - CIF, ";
					videoBridgeCifFrameRate = TranslateIsdnFrameRateToVideoBridgeFrameRate(remoteCapCifFrameRate, H263, str);
					str << "Alg format     - QCIF, ";
					videoBridgeQcifFrameRate = TranslateIsdnFrameRateToVideoBridgeFrameRate(remoteCapQcifFrameRate, H263, str);
					break;
				}
			} // switch

			break;
		}

		case H261:
		{
			WORD format = ScmVidMode.GetVidFormat();
			videoBridgeFormat = TranslateIsdnFormatToVideoBridgeFormat(format, H261);
			WORD frameRate = 0;

			str << "\n";
			str << "Video Alg      - H261" << "\n";

			switch (format)
			{
				case (V_Cif):
				{
					frameRate = ScmVidMode.GetCifMpi();
					str << "Alg format     - CIF  ,\t";
					str << "Alg frame rate - ";
					videoBridgeCifFrameRate = TranslateIsdnFrameRateToVideoBridgeFrameRate(frameRate, H261, str);
				}
				case (V_Qcif):
				{
					frameRate = ScmVidMode.GetQcifMpi();
					str << "Alg format     -   QCIF  ,\t";
					str << "Alg frame rate - ";
					videoBridgeQcifFrameRate = TranslateIsdnFrameRateToVideoBridgeFrameRate(frameRate, H261, str);
					break;
				}
			} // switch

			break;
		}

		default:
		{
			PTRACE(eLevelError, "CIsdnPartyCntl::InitVideoParams with no video Algorithm!!!");
			return;
		}
	} // switch

	PTRACE(eLevelInfoNormal, str.GetString());

	pMediaParams->SetVideoAlgorithm(videoBridgeAlgorithm);
	pMediaParams->SetVideoBitRate(videoBridgeBitRate);
	pMediaParams->SetVideoFrameRate(eVideoResolutionQCIF, eVideoFrameRate(videoBridgeQcifFrameRate));
	pMediaParams->SetVideoFrameRate(eVideoResolutionCIF, eVideoFrameRate(videoBridgeCifFrameRate));
	pMediaParams->SetVideoFrameRate(eVideoResolution4CIF, eVideoFrameRate(videoBridge4CifFrameRate));
	pMediaParams->SetVideoResolution(eVideoResolution(videoBridgeFormat));
	pMediaParams->SetMBPS(videoBridgeMbps);
	pMediaParams->SetFS(videoBridgeFs);
	pMediaParams->SetSampleAspectRatio(videoBridgeSar);
	pMediaParams->SetStaticMB(videoBridgeStaticMB);
	pMediaParams->SetProfile(videoBridgeProfile);
	pMediaParams->SetPacketFormat(videoBridgePacketFormat);
	pMediaParams->SetVideoPartyType(m_eLastAllocatedVideoPartyType);
	pMediaParams->SetMaxDPB(videoBridgeDPB);
	if (b264)
	{
		pMediaParams->SetUseIntermediateSDResolution(TRUE);
	}

	if (bOut && b264)
	{
		if (m_pConfScm->IsFreeVideoRate())
			UpdateAudioDelay(pMediaParams);
	}
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::UpdateAudioDelay(CBridgePartyVideoParams* pMediaParams)
{
	PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateAudioDelay - begin");
	DWORD dwFS   = pMediaParams->GetFS();
	DWORD dwMBPS = pMediaParams->GetMBPS();
	if (m_iAudioDelayUpdated < 2 && ((dwFS >= 15 && dwMBPS >= 216) || IsConfTelePresenceMode()))
	{
		DWORD audioDecoderCompressedDelay = 0; // default value
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("AUDIO_DECODER_COMPRESSED_DELAY", audioDecoderCompressedDelay);

		// Set---decoder
		TAudioUpdateCompressedAudioDelayReq st;
		st.bnCompressedAudioDelay     = 1;
		st.nCompressedAudioDelayValue = audioDecoderCompressedDelay;
		PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateAudioDelay - (dwFS >= 15 && dwMBPS >= 216 || m_eTelePresenceMode != eTelePresencePartyNone) && m_iAudioDelayUpdated < 2");
		if (m_pAudioInterface->UpdateAudioDelay(GetPartyRsrcId(), &st))
		{
			m_iAudioDelayUpdated = 2;
			PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateAudioDelay - OK");
		}
	}
	else if (m_iAudioDelayUpdated == 2 && (dwFS < 15 || dwMBPS < 216) && !IsConfTelePresenceMode())
	{
		TAudioUpdateCompressedAudioDelayReq st;
		st.bnCompressedAudioDelay     = 0;
		st.nCompressedAudioDelayValue = 0;
		PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateAudioDelay - (dwFS < 15 || dwMBPS < 216) && m_eTelePresenceMode != eTelePresencePartyNone && m_iAudioDelayUpdated == 2");
		if (m_pAudioInterface->UpdateAudioDelay(GetPartyRsrcId(), &st))
		{
			m_iAudioDelayUpdated = 1;
			PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateAudioDelay - OK");
		}
	}

	PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateAudioDelay - end");
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::UpdateCurrComMode(CComMode* pScm)
{
	CSegment oldCurComSeg;
	pScm->UpdateH221string(IsPartyH239()); // to avoid incorrect update (because - H221string is sent to DB)
	pScm->Serialize(NATIVE, oldCurComSeg);
	m_pTaskApi->UpdateDB(m_pParty, CURCOMMODE, (DWORD) 0, 1, &oldCurComSeg);

	if (m_pCurrentTransmitScm != pScm)
		*m_pCurrentTransmitScm = *pScm;
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::GetPartyCascadeType() const
{
	BYTE cascadeType = CASCADE_NONE;
	if ((WORD)CASCADE_MODE_MCU == m_nodeType)
	{
		cascadeType = CASCADE_MCU;
	}

	return cascadeType;
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::UpdateH264BaseProfileTresholdInLocalCaps()
{
	PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateH264BaseProfileTresholdInLocalCaps");

	if (!m_pLocalCap->IsH264())
	{
		PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateH264BaseProfileTresholdInLocalCaps - no h264caps");
		return FALSE;
	}

	H264VideoModeDetails h264VidModeDetails_baseProf;

	eVideoPartyType    localVideoPartyType = GetVideoPartyTypeAccordingToCapabilities(m_pLocalCap);
	Eh264VideoModeType localVideoType      = TranslateCPVideoPartyTypeToMaxH264VideoModeType(localVideoPartyType);

	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
	DWORD         callRate   = m_numChnl * rate64K;

	GetH264VideoParams(h264VidModeDetails_baseProf, callRate, vidQuality, localVideoType, FALSE);

	m_pLocalCap->RemoveH264Caps();

	BYTE newLevel = (BYTE)h264VidModeDetails_baseProf.levelValue;
	WORD newMbps  = (WORD)h264VidModeDetails_baseProf.maxMBPS;
	WORD newFs    = (WORD)h264VidModeDetails_baseProf.maxFS;
	WORD newDpb   = (WORD)h264VidModeDetails_baseProf.maxDPB;
	WORD newBrCpb = ConvertMaxBrToMaxBrAndCpb((DWORD)h264VidModeDetails_baseProf.maxBR);

	CCapSetH264* pCapSetH264 = new CCapSetH264;
	pCapSetH264->Create(newLevel, newMbps, newFs, newDpb, newBrCpb);

	m_pLocalCap->AddH264CapSet(pCapSetH264);

	// if local cap has changed -> we have to update the ConfScm and the target Scm
	if (m_pConfScm->GetVidMode() == H264)
	{
		m_pConfScm->m_vidMode.SetH264VidMode(*pCapSetH264);
	}

	if (m_pTargetReceiveScm->GetVidMode() == H264)
	{
		PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateH264BaseProfileTresholdInLocalCaps : **m_pTargetReceiveScm Video mode before update:");
		m_pTargetReceiveScm->m_vidMode.Dump();

		CVidMode TargetReceiveVideoMode = m_pTargetReceiveScm->m_vidMode;
		WORD IsAutoVidScm = TargetReceiveVideoMode.IsAutoVidScm();
		CVidMode IntersectVidMode;
		TargetReceiveVideoMode.Intersect(m_pLocalCap, IsAutoVidScm, IntersectVidMode);
		m_pTargetReceiveScm->SetVidMode(IntersectVidMode);

		PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateH264BaseProfileTresholdInLocalCaps : **m_pTargetReceiveScm Video mode After update:");
		m_pTargetReceiveScm->m_vidMode.Dump();
	}

	PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateH264BaseProfileTresholdInLocalCaps : **m_pTargetTransmitScm Video mode before update:");
	m_pTargetTransmitScm->m_vidMode.Dump();
	m_pTargetTransmitScm->m_vidMode.m_H264VidMode.CalcAndSetCommonLevelAndParam(m_pConfScm->m_vidMode.GetH264VidMode());
	PTRACE(eLevelInfoNormal, "CIsdnPartyCntl::UpdateH264BaseProfileTresholdInLocalCaps : **m_pTargetTransmitScm Video mode After update:");
	m_pTargetTransmitScm->m_vidMode.Dump();

	return TRUE;
}

//--------------------------------------------------------------------------
BYTE CIsdnPartyCntl::IsLegacyContentParty()
{
	ePresentationProtocol contentProtocolMode = (ePresentationProtocol)(m_pConf->GetCommConf()->GetPresentationProtocol());
	if (contentProtocolMode == eH264Fix || contentProtocolMode == eH264Dynamic)
	{
		return TRUE;
	}

	return !IsRemoteAndLocalCapSetHasContent();
}
//--------------------------------------------------------------------------
bool CIsdnPartyCntl::StartAvcToSvcArtTranslator()
{
	bool bIsTranslatorStarted = false;
	if (IsAllPartyRsrcForTranslatorExist())
	{
		FillTranslatorRsrcParamsAndStartCreateTranslator();
		bIsTranslatorStarted = true;
	}

	return bIsTranslatorStarted;
}
//--------------------------------------------------------------------------
void CIsdnPartyCntl::DisconnectAvcToSvcArtTranslator()
{
	TRACEINTO << "partyRsrcId:" << GetPartyRsrcId() << " ,confRsrcId:" << GetConfRsrcId();

	m_pPartyApi->DisconnectAvcToSvcArtTranslator();
}
//--------------------------------------------------------------------------
bool CIsdnPartyCntl::IsAllPartyRsrcForTranslatorExist()
{
	ConfRsrcID confRsrcId = GetConfRsrcId();
	PartyRsrcID partyRsrcId = GetPartyRsrcId();

	CRsrcDesc *pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyRsrcId, eLogical_relay_rtp);
	PASSERT_AND_RETURN_VALUE(!pRsrcDesc,false);

	pRsrcDesc = ::GetpConfPartyRoutingTable()->GetPartyRsrcDesc(partyRsrcId, eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
	PASSERT_AND_RETURN_VALUE(!pRsrcDesc,false);

	return true;
}

//--------------------------------------------------------------------------
void CIsdnPartyCntl::FillTranslatorRsrcParamsAndStartCreateTranslator()
{
    ConfRsrcID confRsrcId = GetConfRsrcId();
    PartyRsrcID partyRsrcId = GetPartyRsrcId();

    ConnectionID artConnectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
    CRsrcParams artTranslatorRsrcParams(artConnectionId, partyRsrcId,confRsrcId, eLogical_relay_avc_to_svc_rtp_with_audio_encoder);

    ConnectionID mrmpConnectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_relay_rtp);
    CRsrcParams mrmpArtTranslatorRsrcParams(mrmpConnectionId, partyRsrcId,confRsrcId, eLogical_relay_rtp);

    m_bPartyInUpgradeProcess = true;
    m_bIsAlreadyUpgradeToMixAvcToSvc = true;

    TRACEINTO << "\nartConnectionId:" << artConnectionId
                                      << "\nmrmpConnectionId:" << mrmpConnectionId
                                      << "\nssrcAudio:" << m_ssrcAudio
                                      << "\npartyRsrcId:" << partyRsrcId
                                      << "\nconfRsrcId:" << confRsrcId
                                      << "\nroomId:" << m_RoomId
                                      << "\npartyInUpgradeProcess:" << (WORD)m_bPartyInUpgradeProcess
                                      << "\nm_bIsAlreadyUpgradeToMixAvcToSvc:" << (WORD)m_bIsAlreadyUpgradeToMixAvcToSvc;

    m_pPartyApi->StartAvcToSvcArtTranslator(m_ssrcAudio, m_RoomId, artTranslatorRsrcParams, mrmpArtTranslatorRsrcParams);
}
