#include "NStream.h"
#include "PartyCntl.h"
#include "ObjString.h"
#include "PartyApi.h"
#include "Conf.h"
#include "VideoBridgeInterface.h"
#include "BridgePartyInitParams.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgeMoveParams.h"
#include "FECCBridge.h"
#include "ContentBridge.h"
#include "AudioBridgeInterface.h"
#include "ConfAppMngrInterface.h"
#include "VideoBridgeInterface.h"
#include "HostCommonDefinitions.h"
#include "ManagerApi.h"
#include "IpCommon.h"
#include "ConfPartyDefines.h"
#include "TraceStream.h"
#include "CDRUtils.h"
#include "H264Util.h"
#include "AllocateStructs.h"
#include "AvcToSvcParams.h"

class CAvcToSvcParams;


extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );
// ~~~~~~~~~~~~~~~~~~~~~~~~~~ global function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
LayoutType GetNewLayoutType(const BYTE oldLayoutType);

BYTE CPartyCntl::IsTipSlavePartyType() const
{
  if(eTipMasterCenter == m_TipPartyType || eTipNone == m_TipPartyType)
    return FALSE;
  return TRUE;
}

const TFormat CPartyCntl::g_formatTbl[MAX_VIDEO_BRIDGE_VIDEO_FORMATS] =
{
	{eVideoResolutionQCIF, kQCif, H263_QCIF_SQCIF, V_Qcif},
	{eVideoResolutionCIF, kCif, H263_CIF, V_Cif},
	{eVideoResolutionVGA, kVGA, VGA, 0},
	{eVideoResolution4CIF, k4Cif, H263_CIF_4, 0},
	{eVideoResolution525SD, k525SD, 0 /*UNKNOWN FORMAT FOR 320?*/, 0},
	{eVideoResolution625SD, k625SD, 0 /*UNKNOWN FORMAT FOR 320?*/, 0},
	{eVideoResolutionSVGA, kSVGA, SVGA, 0},
	{eVideoResolutionXGA, kXGA, XGA, 0},
	{eVideoResolutionHD720, k720p, 0 /*UNKNOWN FORMAT FOR 320?*/, 0},  // to be added in IP H264 translation
	{eVideoResolution16CIF, k16Cif, H263_CIF_16, 0},
	{eVideoResolutionSIF, kSIF, 0 /*UNKNOWN FORMAT FOR 320?*/, 0},
	{eVideoResolutionQVGA, kQVGA, 0 /*UNKNOWN FORMAT FOR 320?*/, 0}
};
const TFrameRate_toMPI CPartyCntl::g_frameRateToMpiTbl[MAX_VIDEO_BRIDGE_FRAME_RATES_MPI] =
{
	{eVideoFrameRate30FPS, 30, MPI_1, V_1_29_97},
	{eVideoFrameRate25FPS, 25, MPI_1, V_1_29_97},
	{eVideoFrameRate15FPS, 15, MPI_2, V_2_29_97},
	{eVideoFrameRate12_5FPS, 15, MPI_2, V_2_29_97},
	{eVideoFrameRate10FPS, 10, MPI_3, V_3_29_97},
	{eVideoFrameRate7_5FPS, 7, MPI_4, V_4_29_97},
	{eVideoFrameRate6FPS, 6, MPI_5, 0},
	{eVideoFrameRate5FPS, 5, MPI_6, 0},
	{eVideoFrameRate3FPS, 3, MPI_10, 0}
};

const TAudioAlg_Caps_toScm CPartyCntl::g_AudioAlg_Caps_toScmTbl[MAX_STANDARD_AUDIO_ALGORITHMS] =
{
	{e_Neutral, Au_Neutral},
	{e_A_Law, A_Law_OU},
	{e_U_Law, U_Law_OU},
	{e_G722_64, G722_m1},
	{e_G722_56, G722_m2},
	{e_G722_48, G722_m3},
	{e_G722_1_32, Au_32k},
	{e_G722_1_24, Au_24k},
	{e_G722_1_Annex_C_48, G7221_AnnexC_48k},
	{e_G722_1_Annex_C_32, G7221_AnnexC_32k},
	{e_G722_1_Annex_C_24, G7221_AnnexC_24k},
};

#define WAIT_FOR_RSRC_AND_ASK_AGAIN_TOUT      2* SECOND // DSP reconfiguration
#define WAIT_FOR_RSRC_AND_ASK_AGAIN_TOUT_FIPS 12* SECOND

#define DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT_VALUE  3*SECOND


extern CConfPartyRoutingTable* GetpConfPartyRoutingTable(void);
extern const char* MediaStateToString(eConfMediaState confState);

PBEGIN_MESSAGE_MAP(CPartyCntl)
	ONEVENT(PARTY_FAULTY_RSRC,       ANYCASE, CPartyCntl::OnPartyReceivedFaultyRsrc)
	ONEVENT(UPDATEVISUALNAME,        ANYCASE, CPartyCntl::OnPartyUpdateVisualNameAndProductId)
	ONEVENT(PARTYLAYOUTCHANGED,      ANYCASE, CPartyCntl::NullActionFunction)
	ONEVENT(ACK_IND,                 ANYCASE, CPartyCntl::NullActionFunction)
	ONEVENT(PCM_PARTY_STATE_CHANGED, ANYCASE, CPartyCntl::OnPartyPcmStateChangedAnycase)
	ONEVENT(SETNODETYPE,             ANYCASE, CPartyCntl::OnPartySetNodeType)
	ONEVENT(SET_PARTY_AVC_SVC_MEDIA_STATE, ANYCASE, CPartyCntl::OnConfSetPartyAvcSvcModeAnycase)
    ONEVENT(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT,  ANYCASE,    CPartyCntl::OnDisconnectAvcToSvcArtTranslatorCloseTout)

PEND_MESSAGE_MAP(CPartyCntl, CStateMachine);


////////////////////////////////////////////////////////////////////////////
//                        CPartyCntl
////////////////////////////////////////////////////////////////////////////
CPartyCntl::CPartyCntl()
{
	m_pPartyHWInterface              = NULL;
	m_pPartyAllocatedRsrc            = NULL;
	m_eAudBridgeConnState            = eBridgeDisconnected;
	m_eVidBridgeConnState            = eBridgeDisconnected;
	m_bIsMemberInAudBridge           = 0;
	m_bIsMemberInVidBridge           = 0;
	m_isChairConn                    = FALSE;
	m_isFeccConn                     = FALSE;
	m_isContentConn                  = FALSE;
	m_isXCodeConn                    = FALSE;
	m_isTerminalNumberingConn        = FALSE;
	m_voice                          = NO;
	m_pPartyApi                      = NULL;
	m_pParty                         = NULL;
	m_pTaskApi                       = NULL;
	m_pAudioInterface                = NULL;
	m_pVideoBridgeInterface          = NULL;
	m_pConfAppMngrInterface          = NULL;
	m_pBridgeMoveParams              = NULL;
	m_pFECCBridge                    = NULL;
	m_pContentBridge                 = NULL;
	m_pXCodeBridgeInterface          = NULL;
	m_pTerminalNumberingManager      = NULL;
	m_name                           = NULL;
	m_service_provider               = NULL;
	m_serviceId                      = 0;
	m_connectDelay                   = 0;
	m_connectDelayCounter            = 0;
	m_avServiceName                  = NULL;
	m_welcome_msg_time               = 0;
	m_welcome_music_volume           = 0;
	m_avConnectionMode               = 0;
	m_isMultiRate                    = 0;
	m_isRemoteCapReady               = 0;
	m_disconnectMode                 = 0;
	m_isChairEnabled                 = 1;
	m_IsCascadeToCopMcu              = FALSE;
	m_isAutoDetect                   = NO;
	m_isWaitingForHotSwap            = NO;
	m_pInfoSeg                       = NULL;
	m_isChangeFromVSToTR             = 0;
	m_IsGateWay                      = 0;
	m_TcMode                         = 0;
	m_DownSpeedStatus                = 0;
	m_sourceId                       = 0;
	m_IsMovedParty                   = FALSE;
	m_isUndefParty                   = NO;
	m_isDisconnect                   = NO;
	m_eLastAllocatedVideoPartyType   = eVideo_party_type_dummy;
	m_isFullBitRateConnect           = 0;
	m_confWaitToEndChangeModeForMove = NO;
	m_disconnectState                = 0;
	m_isRecover                      = FALSE;
	m_destMonitorConfId              = 0;
	m_destResourceConfId             = 0;
	m_destMonitorPartyId             = 0;
	m_isPartyEndRAMoveOK             = 0;
	m_pDestConfMbx                   = NULL;
	m_type                           = 0;
	m_pComConf                       = NULL;
	m_pConf                          = NULL;
	m_isFaulty                       = 0;
	m_isRecovery                     = 0;
	m_disconnectionCause             = 0;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
		m_redial = GetSystemCfgFlagInt<DWORD>(CFG_KEY_NUMBER_OF_REDIAL);
	else
		m_redial = 3;      // as the defasult value in sysconfig

	m_isAutoVidBitRate                  = 1;
	m_isPartyInConf                     = 0;
	m_eLastAllocRequestVideoPartyType   = eVideo_party_type_dummy;
	m_eLastReAllocRequestVideoPartyType = eVideo_party_type_dummy;
	m_eTelePresenceMode                 = eTelePresencePartyNone;
	m_telepresenseEPInfo                = new CTelepresenseEPInfo();
	m_subServiceId                      = 0;
	m_artCapacity                       = 0;
	m_isCDRPartyConnected               = 0;
	m_OldState                          = IDLE;
	m_disconnectDelay                   = 0;
	m_isWaitingForRsrcAndAskAgain       = 1;
	m_bNoVideRsrcForVideoParty          = FALSE;
	m_lastReqId                         = (DWORD)-1;
	m_isPcmConnected                    = 0;
	m_iAudioDelayUpdated                = 0;
	m_bIsVideoMuted                     = NO;
	m_RoomId                            = 0xFFFF;
	m_nodeType                          = CASCADE_MODE_NONE;
	m_maxConnectionRateCurrently        = 0;
	m_maxFormatCurrently                = kCif;
	m_maxFrameRateCurrently             = 0;
	m_bIsMrcCall                        = FALSE;
	m_bIsWebRtcCall                     = FALSE;
	m_deferUpgrade                      = false;
	m_bPartyInUpgradeProcess            = false;

	memset(m_siteName, 0, sizeof(m_siteName));
	memset(m_productId, 0, sizeof(m_productId));
	memset(m_VersionId, 0, sizeof(m_VersionId));
	memset(m_partyConfName, 0, sizeof(m_partyConfName));
	memset(m_password, 0, sizeof(m_password));

	m_bIsTipCall           = FALSE;
	m_TipPartyType         = eTipNone;
	m_MSSlaveIndex         = 0;
	m_AvMcuLinkType        = eAvMcuLinkNone;
	m_MSaudioLocalMsi      = 0;
	m_MasterRsrcId  		= 0;

	m_eRemoteVendorIdent   = Regular;

	m_isMsftEnv            = FALSE;

	memset(&m_reallocatePartyParams, 0, sizeof(ALLOC_PARTY_REQ_PARAMS_S));
	m_reAllocRtm           = FALSE;
	m_isReallocSyncMessage = FALSE;
	m_reallocOpcode        = REALLOCATE_PARTY_RSRC_REQ;
	m_moveType             = eMoveDummy;

	VALIDATEMESSAGEMAP;
}


//--------------------------------------------------------------------------
// Description: This function allocates buffer, which contains the party and the class details.
// NOTE: The function that calls to this function must deallocate this buffer!!
char* CPartyCntl::GetPartyAndClassDetails()
{
	const char* className = NameOf();
	DWORD       msgLen    = SmallPrintLen + strlen(m_partyConfName)+1 + strlen(className)+1;
	ALLOCBUFFER(strDetails, msgLen);
	sprintf(strDetails, "Name - %s, Class - %s", m_partyConfName, className);
	return strDetails;
}

//--------------------------------------------------------------------------
int CPartyCntl::TranslateIpFormatToVideoBridgeFormat(int format) const
{
	int reservationFormat = eVideoResolutionDummy;

	if (format == kUnknownFormat)
	{
		PASSERTMSG(1, "Invalid Input = kUnknownFormat");
		return (reservationFormat);
	}

	for (int i = 0; (i < MAX_VIDEO_BRIDGE_VIDEO_FORMATS) && (reservationFormat == eVideoResolutionDummy); i++)
	{
		if (g_formatTbl[i].ipFormat == format)
			reservationFormat = g_formatTbl[i].videoBridgeFormat;
	}

	return reservationFormat;
}

//--------------------------------------------------------------------------
int CPartyCntl::TranslateIpMpiToVideoBridgeFrameRate(int Mpi) const
{
	int videoBridgeFrameRate = eVideoFrameRateDUMMY;

	if (Mpi == -1)
	{
		return (videoBridgeFrameRate);
	}

	for (int i = 0; (i < MAX_VIDEO_BRIDGE_FRAME_RATES_MPI) && (videoBridgeFrameRate == eVideoFrameRateDUMMY); i++)
	{
		if (g_frameRateToMpiTbl[i].ipMpi == Mpi)
			videoBridgeFrameRate = g_frameRateToMpiTbl[i].videoBridgeFrameRate;
	}

	return videoBridgeFrameRate;
}

//--------------------------------------------------------------------------
int CPartyCntl::TranslateVideoBridgeFrameRateToIpMpi(eVideoFrameRate frameRate) const
{
	int Mpi = -1;
	for (int i = 0; i < MAX_VIDEO_BRIDGE_FRAME_RATES_MPI; i++)
	{
		if (g_frameRateToMpiTbl[i].videoBridgeFrameRate == frameRate)
			Mpi = g_frameRateToMpiTbl[i].ipMpi;
	}

	return Mpi;
}

//--------------------------------------------------------------------------
int CPartyCntl::TranslateIsdnFormatToVideoBridgeFormat(int format, WORD VideoAlgorithm) const
{
	int reservationFormat = eVideoResolutionDummy;

	if (format == -1)
	{
		PASSERTMSG(1, "Invalid Input = -1");
		return (reservationFormat);
	}

	for (int i = 0; (i < MAX_VIDEO_BRIDGE_VIDEO_FORMATS) && (reservationFormat == eVideoResolutionDummy); i++)
	{
		switch (VideoAlgorithm)
		{
			case (H263):
			{
				if (g_formatTbl[i].isdn263Format == format)
					reservationFormat = g_formatTbl[i].videoBridgeFormat;
				break;
			}

			case (H261):
			{
				if (g_formatTbl[i].isdn261Format == format)
					reservationFormat = g_formatTbl[i].videoBridgeFormat;
				break;
			}

			default:
			{
				PASSERTMSG(VideoAlgorithm, "Invalid Algorithm - should be H263 (2) or H261 (1)");
				return (reservationFormat);
			}
		} // switch
	}

	return reservationFormat;
}

//--------------------------------------------------------------------------
int CPartyCntl::TranslateIsdnFrameRateToVideoBridgeFrameRate(int Mpi, WORD VideoAlgorithm, CObjString& msg) const
{
	int videoBridgeFrameRate = eVideoFrameRateDUMMY;

	if (Mpi == -1)
	{
		PASSERTMSG(1, "Invalid Input = -1");
		return (videoBridgeFrameRate);
	}

	for (int i = 0; (i < MAX_VIDEO_BRIDGE_FRAME_RATES_MPI) && (videoBridgeFrameRate == eVideoFrameRateDUMMY); i++)
	{
		switch (VideoAlgorithm)
		{
			case (H263):
			{
				if (g_frameRateToMpiTbl[i].isdn263Mpi == Mpi)
				{
					videoBridgeFrameRate = g_frameRateToMpiTbl[i].videoBridgeFrameRate;
					msg << g_frameRateToMpiTbl[i].ipMpi << " frames per second " << "\n";
				}
				break;
			}

			case (H261):
			{
				if (g_frameRateToMpiTbl[i].isdn261FrameRate == Mpi)
				{
					videoBridgeFrameRate = g_frameRateToMpiTbl[i].videoBridgeFrameRate;
					msg << g_frameRateToMpiTbl[i].ipMpi << " frames per second " << "\n";
				}
				break;
			}

			default:
			{
				PASSERTMSG(VideoAlgorithm, "Invalid Algorithm - should be H263 (2) or H261 (1)");
				return (videoBridgeFrameRate);
			}
		} // switch
	}

	return videoBridgeFrameRate;
}
eVideoFrameRate CPartyCntl::TranslateIntegerFrameRateToVideoBridgeFrameRate(DWORD frameRate)
{
	eVideoFrameRate videoBridgeFrameRate = eVideoFrameRateDUMMY;
	if(frameRate <=3)
	{
		videoBridgeFrameRate = eVideoFrameRate3FPS;
	}
	else if(frameRate<=6)
	{
		videoBridgeFrameRate = eVideoFrameRate6FPS;
	}
	else if(frameRate <= 8)
	{
		videoBridgeFrameRate = eVideoFrameRate7_5FPS;

	}
	else if(frameRate<= 15)
	{
		videoBridgeFrameRate = eVideoFrameRate15FPS;
	}
	else if (frameRate <= 30 )
	{
		videoBridgeFrameRate = eVideoFrameRate30FPS;
	}
	else
	{
		videoBridgeFrameRate = eVideoFrameRate60FPS;
	}
	return videoBridgeFrameRate;
}



//--------------------------------------------------------------------------
WORD CPartyCntl::TranslateAudioAlgCapsToScm(EAudioCapAlgorithm AudioAlg)
{
	WORD audioModeInScm = Au_Neutral;

	for (int i = 0; (i < MAX_STANDARD_AUDIO_ALGORITHMS) && (audioModeInScm == Au_Neutral); i++)
	{
		if (g_AudioAlg_Caps_toScmTbl[i].audioAlgFromCaps == AudioAlg)
			audioModeInScm = g_AudioAlg_Caps_toScmTbl[i].audioAlgFromCommMode;
	}

	return audioModeInScm;
}

//--------------------------------------------------------------------------
EAudioCapAlgorithm CPartyCntl::TranslateAudioAlgScmToCaps(WORD AudioMode)
{
	EAudioCapAlgorithm audioAlgInCaps = e_Neutral;

	for (int i = 0; (i < MAX_STANDARD_AUDIO_ALGORITHMS) && (audioAlgInCaps == e_Neutral); i++)
	{
		if (g_AudioAlg_Caps_toScmTbl[i].audioAlgFromCommMode == AudioMode)
			audioAlgInCaps = g_AudioAlg_Caps_toScmTbl[i].audioAlgFromCaps;
	}

	return audioAlgInCaps;
}

//--------------------------------------------------------------------------
const char* CPartyCntl::GetBridgeConnectionStateStr(EBridgeConnectionState eBridgeConnectionState) const
{
	switch (eBridgeConnectionState)
	{
		case eBridgeDisconnected    : return "eBridgeDisconnected";
		case eSendOpenIn            : return "eSendOpenIn";
		case eSendOpenOut           : return "eSendOpenOut";
		case eSendOpenInAndOut      : return "eSendOpenInAndOut";
		case eInConnected           : return "eInConnected";
		case eOutConnected          : return "eOutConnected";
		case eSendOpenOutInConnected: return "eSendOpenOutInConnected";
		case eSendOpenInOutConnected: return "eSendOpenInOutConnected";
		case eInAndOutConnected     : return "eInAndOutConnected";
	}
	return "INVALID";
}

//--------------------------------------------------------------------------
CPartyCntl::~CPartyCntl() // destructor
{
	POBJDELETE(m_pDestConfMbx);
	POBJDELETE(m_telepresenseEPInfo);
}

//--------------------------------------------------------------------------
WORD CPartyCntl::IsDisconnectState()
{
	WORD rc;
	m_disconnectState == DISCONNECTED ? rc = 1 : rc = 0;
	return rc;
}

//--------------------------------------------------------------------------
void CPartyCntl::InsertPartyResourcesToGlobalRsrcRoutingTbl()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
	if (pRoutingTbl == NULL)
	{
		PASSERT_AND_RETURN(GetPartyRsrcId());
	}

	// Add general Party Entry to Routing Table
	CPartyRsrcRoutingTblKey routingKey = CPartyRsrcRoutingTblKey(DUMMY_CONNECTION_ID, GetPartyRsrcId(), eLogical_res_none);
	pRoutingTbl->AddPartyRsrcDesc(routingKey);

	// Add ptr to Conf for general party entry
	CTaskApi* pTaskApi = new CTaskApi(*m_pTaskApi);
	pTaskApi->CreateOnlyApi(m_pTaskApi->GetRcvMbx(), NULL);
	pRoutingTbl->AddStateMachinePointerToRoutingTbl(GetPartyRsrcId(), eLogical_res_none, pTaskApi);

	// Add for each party resource entry to Routing Table
	m_pPartyAllocatedRsrc->InsertToGlobalRsrcRoutingTbl();

	POBJDELETE(pTaskApi);
}

//--------------------------------------------------------------------------
DWORD CPartyCntl::GetPartyRsrcId() const
{
	if (m_pPartyAllocatedRsrc)
		return m_pPartyAllocatedRsrc->GetPartyRsrcId();
	else
		return DEFAULT_PARTY_ID;
}

//--------------------------------------------------------------------------
DWORD CPartyCntl::GetConfRsrcId() const
{
	if (m_pPartyHWInterface)
		return m_pPartyHWInterface->GetConfRsrcId();
	else
		return INVALID;
}

//--------------------------------------------------------------------------
WORD CPartyCntl::ArePartyResourcesAllocated() const
{
	if (m_pPartyAllocatedRsrc)
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
void CPartyCntl::SetFullName(const char* partyName, const char* confName)
{
	if (partyName && confName)
		snprintf(m_partyConfName, sizeof(m_partyConfName), "ConfName:%s, PartyName:%s", confName, partyName);
	else if (partyName)
		snprintf(m_partyConfName, sizeof(m_partyConfName), "PartyName:%s", partyName);
	else if (confName)
		snprintf(m_partyConfName, sizeof(m_partyConfName), "ConfName:%s", confName);
}

//--------------------------------------------------------------------------
void CPartyCntl::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	switch (opCode)
	{
		case PARTYDISCONNECT:
		{
			// OnPartyDisConnect(pMsg);
			break;
		}

		default:
			DispatchEvent(opCode, pMsg);
			break;
	} // switch
}

//--------------------------------------------------------------------------
void CPartyCntl::HandlePartyExternalEvent(CSegment* pMsg, OPCODE opCode)
{
	if (m_pPartyApi && CPObject::IsValidPObjectPtr(m_pPartyApi))
		m_pPartyApi->HandlePartyExternalEvent(pMsg, opCode);
}

//--------------------------------------------------------------------------
void CPartyCntl::Destroy()
{
	POBJDELETE(m_pPartyHWInterface);
	POBJDELETE(m_pPartyAllocatedRsrc);
	POBJDELETE(m_pBridgeMoveParams);

	PDELETEA(m_name);
	if (m_pTaskApi)
	{
		m_pTaskApi->DestroyOnlyApi();
		POBJDELETE(m_pTaskApi);
	}

	if (m_pPartyApi)
	{
		m_pPartyApi->Destroy();
		POBJDELETE(m_pPartyApi);
	}

	if (m_pComConf)
		delete ((CConfParty*)m_pComConf);

	POBJDELETE(m_pDestConfMbx);
	PDELETEA(m_service_provider);
	PDELETEA(m_avServiceName);
	POBJDELETE(m_pInfoSeg);
}

//--------------------------------------------------------------------------
void CPartyCntl::SetParty(CTaskApp* pParty)
{
	m_pParty = pParty;
}

//--------------------------------------------------------------------------
void CPartyCntl::SetPartyRsrcId(DWORD partyRsrcId)
{
	if (m_pPartyHWInterface)
		m_pPartyHWInterface->SetPartyRsrcId(partyRsrcId);
}

//--------------------------------------------------------------------------
void CPartyCntl::SetConfRsrcId(DWORD confRsrcId)
{
	if (m_pPartyHWInterface)
		m_pPartyHWInterface->SetConfRsrcId(confRsrcId);
}

//--------------------------------------------------------------------------
void CPartyCntl::Disconnect(WORD mode)
{
	ON(m_isDisconnect);
	m_disconnectMode = mode;
	OFF(m_DownSpeedStatus); // In case of disconnect/delete the down speed stauts should be reset.
	memset(m_productId, '\0', MAX_SITE_NAME_ARR_SIZE);
	memset(m_VersionId, '\0', MAX_SITE_NAME_ARR_SIZE);
	m_IsCascadeToCopMcu = FALSE;
}

//--------------------------------------------------------------------------
WORD CPartyCntl::IsDisconnect()
{
	return m_isDisconnect;
}

// ----------------------------------------------------------------------------
void CPartyCntl::InitVideoInParams(CBridgePartyVideoInParams* pMediaInParams)
{
	PASSERTMSG(GetPartyRsrcId(), "CPartyCntl::InitVideoInParams : Function must be overwritten in derived classes");
}

// ----------------------------------------------------------------------------
void CPartyCntl::InitVideoOutParams(CBridgePartyVideoOutParams* pMediaOutParams)
{
	PASSERTMSG(GetPartyRsrcId(), "CPartyCntl::InitVideoOutParams : Function must be overwritten in derived classes");
}

//----------------------------------------------------------------------------
void CPartyCntl::InitVideoLayoutParams(CBridgePartyVideoOutParams* pMediaOutParams, CConfParty* pConfParty)
{
	//eFeatureRssDialin  -- set initial video layout for Incoming SRS
	if(pConfParty && pConfParty->GetRecordingLinkParty())
	{
		enSrsVideoLayoutType  videoLayout = (enSrsVideoLayoutType) pConfParty->GetLastLayoutForRL();
		TRACEINTO << "Initial video layout for RL: " << (int)videoLayout;

		if(eSrsVideoLayoutAuto !=  videoLayout)
		{
			CVideoLayout*	 pIniVideoLayout =    new CVideoLayout;
			pIniVideoLayout->SetLayoutForRecording(videoLayout);
			pConfParty->RemoveAllLayouts();
			pConfParty->SetVideoLayout(*pIniVideoLayout);

			LayoutType inilayoutType = ::GetNewLayoutType(pIniVideoLayout->GetScreenLayout());

			CLayout* pIniPrivateReservation = new CLayout(inilayoutType, m_pConf->GetName());
			pIniPrivateReservation->SetCurrActiveLayout(pIniVideoLayout->IsActive());
			pIniPrivateReservation->SetLayout(*pIniVideoLayout, PARTY_lev, TRUE);
			pMediaOutParams->SetPrivateReservationLayout(pIniPrivateReservation, inilayoutType);

			delete pIniVideoLayout;
			return;
		}
		// else : continue
	}

	CVideoLayout* pVideoLayout = m_pConf->GetGatheringLayout(pConfParty);
	if (pConfParty && !pVideoLayout)
	{
		pVideoLayout = pConfParty->GetFirstPrivateVideoLayout();
		if (!pVideoLayout)
		{
			TRACEINTO << "PartyId:" << GetPartyRsrcId() << " - No private layout";
			return;
		}
	}

	if (pVideoLayout)
	{
		TRACEINTO << "PartyId: " << GetPartyRsrcId() << ", FirstPrivateLayout:" << (int)pVideoLayout->GetScreenLayout();
	}
	else
	{
		TRACEINTO << "PartyId: " << GetPartyRsrcId();
	}

	while (pVideoLayout)
	{
		LayoutType layoutType = ::GetNewLayoutType(pVideoLayout->GetScreenLayout());

		CLayout* pPrivateReservation = new CLayout(layoutType, m_pConf->GetName());
		pPrivateReservation->SetCurrActiveLayout(pVideoLayout->IsActive());
		pPrivateReservation->SetLayout(*pVideoLayout, PARTY_lev, TRUE);
		pMediaOutParams->SetPrivateReservationLayout(pPrivateReservation, layoutType);
		if (pConfParty) // Klw pConfParty was checked for NULL
			pVideoLayout = pConfParty->GetNextPrivateVideoLayout();
		else			// Klw
		{
			TRACEINTO << " Warning/Error: pConfParty == NULL, PartyId: " << GetPartyRsrcId();
			pVideoLayout = NULL;
		}
	}
}

//--------------------------------------------------------------------------
void CPartyCntl::DisconnectPartyFromVideoBridge()  // disconnect both directions
{
	if (!(m_eVidBridgeConnState & eSendOpenIn) && !(m_eVidBridgeConnState & eSendOpenOut))
	{
		TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << " - Not needed";
		return;
	}

	m_eVidBridgeConnState = eBridgeDisconnected;

	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << " - Disconnect party Video";

	CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId());
	m_pConfAppMngrInterface->DisconnectPartyVideo(&bridgePartyDisconnectParams);
}
//----------------------------------------------------------------------------
void  CPartyCntl::DisconnectPartyFromXCodeBridge() //disconnect both directions
{
 	if(!CPObject::IsValidPObjectPtr(m_pXCodeBridgeInterface))
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::DisconnectPartyFromXCodeBridge - Interface to XCode bridge is Null!!!! Not needed : ",m_partyConfName, GetPartyRsrcId());
		return;
	}
	if(!m_isXCodeConn)
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::DisconnectPartyFromXCodeBridge - Party is not connected to XCode Bridge - Not needed : ",m_partyConfName, GetPartyRsrcId());
		return;
	}

	PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::DisconnectPartyFromXCodeBridge - ",m_partyConfName, GetPartyRsrcId());

	ALLOCBUFFER(tmp,(2*H243_NAME_LEN+150));	//max size of m_partyConfName is (2*H243_NAME_LEN+50)
	sprintf(tmp, "[%s] - Disconnecting Party XCode...", m_partyConfName);
	PTRACE2PARTYID(eLevelInfoNormal," ---> ",tmp, GetPartyRsrcId());
	DEALLOCBUFFER(tmp)

	CBridgePartyDisconnectParams* pBrdgPartyDisconnecetParams = new CBridgePartyDisconnectParams(GetPartyRsrcId(),eMediaOut);
	m_pXCodeBridgeInterface->DisconnectParty(pBrdgPartyDisconnecetParams);
	POBJDELETE(pBrdgPartyDisconnecetParams);
}
//----------------------------------------------------------------------------
void  CPartyCntl::DisconnectPartyFromFECCBridge()
{
	if (!m_isFeccConn)
	{
		TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << " - No FECC connection, do nothing";
		return;
	}
	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyRsrcId() << " - Disconnect party FECC";

	if (m_pFECCBridge)
	{
		CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId());
		m_pFECCBridge->DisconnectParty(&bridgePartyDisconnectParams);
	}
}

//--------------------------------------------------------------------------
void CPartyCntl::DisconnectPartyFromContentBridge()
{
	if (!m_isContentConn)
	{
		TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyRsrcId() << " - No Content connection, do nothing";
		return;
	}
	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << " - Disconnect party Content";

	// Remove "exclusive content" state for disconnected party
	m_pTaskApi->UpdateDB(m_pParty, UPDATE_EXCLUSIVE_CONTENT, FALSE);

	if (m_pContentBridge)
	{
		CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId());
		if (CASCADE_MCU == GetPartyCascadeType())
		{
			CSegment* pParam = new CSegment;
			bridgePartyDisconnectParams.Serialize(NATIVE, *pParam);
			m_pTaskApi->LinkTryToDisconnect(pParam);
			POBJDELETE(pParam);
		}
		else
		{
			m_pContentBridge->DisconnectParty(&bridgePartyDisconnectParams);
		}
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "CPartyCntl::DisconnectPartyFromContentBridge - ConfParty=[" << m_partyConfName << "], PartyRsrcId=" << GetPartyRsrcId() << " - <NO NEED TO DISCONNECT THE CONTENT BRIDGE>" << endl;
	}


	if(m_pXCodeBridgeInterface && m_isXCodeConn)
		DisconnectPartyFromXCodeBridge();
	else
	{
		if(!m_pXCodeBridgeInterface)
			TRACEINTO << "CPartyCntl::DisconnectPartyFromContentBridge, No Interface to XCode Bridge, Party Name: " << GetFullName() << "\n";
		if(!m_isXCodeConn)
			TRACEINTO << "CPartyCntl::DisconnectPartyFromContentBridge, No Connection to XCode bridge, Party Name: " << GetFullName() << "\n";
	}

}
void CPartyCntl::DisconnectPartyFromAudioBridge()  // disconnect both directions
{
	if (!(m_eAudBridgeConnState & eSendOpenIn) && !(m_eAudBridgeConnState & eSendOpenOut))
	{
		TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << " - Not needed";
		return;
	}

	m_eAudBridgeConnState = eBridgeDisconnected;

	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << " - Disconnect party Audio";

	CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId());
	m_pConfAppMngrInterface->DisconnectPartyAudio(&bridgePartyDisconnectParams);
}

//--------------------------------------------------------------------------
EBridgeConnectionState CPartyCntl::TranslateMediaDirectionToBridgeConnectionState(EMediaDirection eBridgeConnState)
{
	EBridgeConnectionState eBridgeConnectionState = eBridgeDisconnected;

	if (eBridgeConnState == eMediaIn)
		eBridgeConnectionState = eInConnected;
	else if (eBridgeConnState == eMediaOut)
		eBridgeConnectionState = eOutConnected;
	else if (eBridgeConnState == eMediaInAndOut)
		eBridgeConnectionState = eInAndOutConnected;

	return eBridgeConnectionState;
}

//--------------------------------------------------------------------------
void CPartyCntl::UpdateAudioBridgeConnectionState(EMediaDirection eBridgeConnState)
{
	EBridgeConnectionState eBridgeConnectionState = TranslateMediaDirectionToBridgeConnectionState(eBridgeConnState);
	m_eAudBridgeConnState |= eBridgeConnectionState;
}

//--------------------------------------------------------------------------/
void CPartyCntl::UpdateAudioBridgeDisconnectionState(EMediaDirection eBridgeConnState)
{
	if ((eBridgeConnState == eNoDirection) && (m_eAudBridgeConnState == eBridgeDisconnected)) // if bridge is disconnected, and we didn't ask to connect again -
		OFF(m_bIsMemberInAudBridge);
}

//--------------------------------------------------------------------------
void CPartyCntl::UpdateVideoBridgeConnectionState(EMediaDirection eBridgeConnState)
{
	EBridgeConnectionState eBridgeConnectionState = TranslateMediaDirectionToBridgeConnectionState(eBridgeConnState);
	m_eVidBridgeConnState |= eBridgeConnectionState;
}

//--------------------------------------------------------------------------
void CPartyCntl::UpdateVideoBridgeDisconnectionState(EMediaDirection eBridgeConnState)
{
	if ((eBridgeConnState == eNoDirection) && (m_eVidBridgeConnState == eBridgeDisconnected))
		OFF(m_bIsMemberInVidBridge);
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::IsAtLeastOneDirectionConnectedToAudioBridge()
{
	return (((m_eAudBridgeConnState & eInConnected) == eInConnected) ||
	        ((m_eAudBridgeConnState & eOutConnected) == eOutConnected) ||
	        AreTwoDirectionsConnectedToAudioBridge());
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::AreTwoDirectionsConnectedToAudioBridge()
{
	return (m_eAudBridgeConnState == eInAndOutConnected);
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::IsOutDirectionConnectedToAudioBridge()
{
	return (((m_eAudBridgeConnState & eOutConnected) == eOutConnected) || AreTwoDirectionsConnectedToAudioBridge());
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::IsInDirectionConnectedToAudioBridge()
{
	return (((m_eAudBridgeConnState & eInConnected) == eInConnected) || AreTwoDirectionsConnectedToAudioBridge());
}

//--------------------------------------------------------------------------
EMediaDirection CPartyCntl::HandleAudioBridgeConnectedInd(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CPartyCntl::HandleAudioBridgeConnectedInd", GetPartyRsrcId());

	EMediaDirection eAudBridgeConnState = eNoDirection;
	*pParam >> (WORD&)eAudBridgeConnState;
	UpdateAudioBridgeConnectionState(eAudBridgeConnState);

	if (AreTwoDirectionsConnectedToAudioBridge())
	{
		PTRACE2PARTYID(eLevelInfoNormal, " ---> Audio Connection Established ", m_partyConfName, GetPartyRsrcId());
		m_pTaskApi->UpdateDB(m_pParty, AUDCON, TRUE);
	}

	return eAudBridgeConnState;
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::HandleAudioBridgeDisconnectedInd(CSegment* pParam)
{
	WORD status;
	*pParam >> status;
	PASSERTMSG(status, "Status fail");

	EMediaDirection eAudBridgeConnState = eNoDirection;
	*pParam >> (WORD&)eAudBridgeConnState;

	BYTE bIsDisconnectOk = TRUE;

	if (status == statAudioInOutResourceProblem)
	{
		m_isFaulty      = 1;
		bIsDisconnectOk = FALSE;
	}

	// since bridge clears party, even after a problem on disconnection procedure (such as, not receiving a disconnection ack from MFA)
	// but only after handling both direction, update A.B connection state even if status is statAudioInOutResourceProblem.
	UpdateAudioBridgeDisconnectionState(eAudBridgeConnState);

	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		PTRACE2PARTYID(eLevelInfoNormal, " ---> Audio Disconnected ", m_partyConfName, GetPartyRsrcId());
		m_pTaskApi->UpdateDB(m_pParty, AUDCON, FALSE);
	}

	return bIsDisconnectOk;
}

//--------------------------------------------------------------------------
void CPartyCntl::HandleVideoBridgeConnectedInd(CSegment* pParam)
{
	EMediaDirection eVidBridgeConnState = eNoDirection;
	*pParam >> (WORD&)eVidBridgeConnState;
	UpdateVideoBridgeConnectionState(eVidBridgeConnState);

	if (AreTwoDirectionsConnectedToVideoBridge())
	{
		PTRACE2PARTYID(eLevelInfoNormal, " ---> Video Connection Established ", m_partyConfName, GetPartyRsrcId());
		m_pTaskApi->UpdateDB(m_pParty, VIDCON, TRUE);
	}

	UpdateBridgeFlowControlRateIfNeeded();
}
//--------------------------------------------------------------------------
void CPartyCntl::UpdateBridgeFlowControlRateIfNeeded()
{
	PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::UpdateBridgeFlowControlRateIfNeeded - Error. Should call to derived class's	function. ", m_partyConfName, GetPartyRsrcId());
}

//--------------------------------------------------------------------------
EStat CPartyCntl::HandleVideoBridgeDisconnectedInd(CSegment* pParam)
{
	WORD status;
	*pParam >> status;
	PASSERTMSG(status, "Status fail");
	EMediaDirection eVidBridgeConnState = eNoDirection;
	*pParam >> (WORD&)eVidBridgeConnState;
	EStat resStat = (EStat)status;

	if (resStat == statVideoInOutResourceProblem ||
		resStat == statInvalidPartyInitParams) //BRIDGE-14724
	{
		m_isFaulty = 1;
	}
	// since bridge clears party, even after a problem on disconnection procedure (such as, not recieving a disconnection ack from MFA)
	// but only after handling both direction, update v.b connection state even if status is statVideoInOutResourceProblem.
	UpdateVideoBridgeDisconnectionState(eVidBridgeConnState);

	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		PTRACE2PARTYID(eLevelInfoNormal, " ---> Video Disconnected ", m_partyConfName, GetPartyRsrcId());
		m_pTaskApi->UpdateDB(m_pParty, VIDCON, FALSE);
	}

	return resStat;
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::IsAtLeastOneDirectionConnectedToVideoBridge()
{
	return (((m_eVidBridgeConnState & eInConnected) == eInConnected) ||
	        ((m_eVidBridgeConnState & eOutConnected) == eOutConnected) ||
	        AreTwoDirectionsConnectedToVideoBridge());
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::AreTwoDirectionsConnectedToVideoBridge()
{

	return (m_eVidBridgeConnState == eInAndOutConnected);
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::IsOutDirectionConnectedToVideoBridge()
{
	return (((m_eVidBridgeConnState & eOutConnected) == eOutConnected) || AreTwoDirectionsConnectedToVideoBridge());
}
//--------------------------------------------------------------------------
BYTE CPartyCntl::IsInDirectionConnectedToVideoBridge()
{
	return (((m_eVidBridgeConnState & eInConnected) == eInConnected) || AreTwoDirectionsConnectedToVideoBridge());
}

//--------------------------------------------------------------------------
void CPartyCntl::SetPartyName(char* name)
{
	if (!m_name) m_name = new char[H243_NAME_LEN];

	strncpy(m_name, name, H243_NAME_LEN-1);
	m_name[H243_NAME_LEN-1] = '\0';
}

//--------------------------------------------------------------------------
void CPartyCntl::operator=(const CPartyCntl& other)
{
	if (&other == this)
		return;

	m_pPartyHWInterface              = other.m_pPartyHWInterface;
	m_pPartyAllocatedRsrc            = other.m_pPartyAllocatedRsrc;
	m_pBridgeMoveParams              = other.m_pBridgeMoveParams;
	m_termNum                        = other.m_termNum;
	m_type                           = other.m_type;
	m_name                           = other.m_name;
	m_pConf                          = other.m_pConf;
	m_pPartyApi                      = other.m_pPartyApi;
	m_pParty                         = other.m_pParty;
	m_pTaskApi                       = other.m_pTaskApi;
	m_pAudioInterface                = other.m_pAudioInterface;
	m_pVideoBridgeInterface          = other.m_pVideoBridgeInterface;
	m_pFECCBridge                    = other.m_pFECCBridge;
	m_pContentBridge                 = other.m_pContentBridge;
  m_pXCodeBridgeInterface             = other.m_pXCodeBridgeInterface;
	m_pTerminalNumberingManager      = other.m_pTerminalNumberingManager;
	m_pConfAppMngrInterface          = other.m_pConfAppMngrInterface;
	m_eAudBridgeConnState            = other.m_eAudBridgeConnState;
	m_eVidBridgeConnState            = other.m_eVidBridgeConnState;
	m_bIsMemberInAudBridge           = other.m_bIsMemberInAudBridge;
	m_bIsMemberInVidBridge           = other.m_bIsMemberInVidBridge;
	m_isvidTSConn                    = other.m_isvidTSConn;
	m_isChairConn                    = other.m_isChairConn;
	m_isFeccConn                     = other.m_isFeccConn;
	m_isContentConn                  = other.m_isContentConn;
  m_isXCodeConn                       = other.m_isXCodeConn;
	m_isTerminalNumberingConn        = other.m_isTerminalNumberingConn;
	m_isDisconnect                   = other.m_isDisconnect;
	m_disconnectMode                 = other.m_disconnectMode;
	m_disconnectState                = other.m_disconnectState;
	m_monitorConfId                  = other.m_monitorConfId;
	m_monitorPartyId                 = other.m_monitorPartyId;
	m_cascadeMode                    = other.m_cascadeMode;
	m_nodeType                       = other.m_nodeType;
	m_voice                          = other.m_voice;
	m_service_provider               = other.m_service_provider;
	m_serviceId                      = other.m_serviceId;
	m_numChnl                        = other.m_numChnl;
	m_chnlWidth                      = other.m_chnlWidth;
	m_isFullBitRateConnect           = other.m_isFullBitRateConnect;
	m_isAudRecover                   = other.m_isAudRecover;
	m_isVidRecover                   = other.m_isVidRecover;
	m_isMlpRecover                   = other.m_isMlpRecover;
	m_isRecover                      = other.m_isRecover;
	m_connectDelay                   = other.m_connectDelay;
	m_connectDelayCounter            = other.m_connectDelayCounter;
	m_avServiceName                  = other.m_avServiceName;
	m_avConnectionMode               = other.m_avConnectionMode;
	m_welcome_msg_time               = other.m_welcome_msg_time;
	m_moveType                       = other.m_moveType;
	m_isMultiRate                    = other.m_isMultiRate;
	m_videoProtocol                  = other.m_videoProtocol;
	m_isRemoteCapReady               = other.m_isRemoteCapReady;
	m_interfaceType                  = other.m_interfaceType;
	m_isChairEnabled                 = other.m_isChairEnabled;
	m_IsCascadeToCopMcu              = other.m_IsCascadeToCopMcu;
	m_isAutoDetect                   = other.m_isAutoDetect;
	m_pInfoSeg                       = other.m_pInfoSeg;
	m_isChangeFromVSToTR             = other.m_isChangeFromVSToTR;
	m_IsGateWay                      = other.m_IsGateWay;
	m_TcMode                         = other.m_TcMode;
	m_DownSpeedStatus                = other.m_DownSpeedStatus;
	m_sourceId                       = other.m_sourceId;
	m_IsMovedParty                   = other.m_IsMovedParty;
	m_isUndefParty                   = other.m_isUndefParty;
	m_confWaitToEndChangeModeForMove = other.m_confWaitToEndChangeModeForMove;
	m_isWaitingForHotSwap            = other.m_isWaitingForHotSwap;
	m_destMonitorConfId              = other.m_destMonitorConfId;
	m_isPartyEndRAMoveOK             = other.m_isPartyEndRAMoveOK;
	m_destResourceConfId             = other.m_destResourceConfId;
	m_destMonitorPartyId             = other.m_destMonitorPartyId;
	m_isFaulty                       = other.m_isFaulty;
	m_isRecovery                     = other.m_isRecovery;
	m_isAutoVidBitRate               = other.m_isAutoVidBitRate;
	m_isPartyInConf                  = other.m_isPartyInConf;
	m_redial                         = other.m_redial;
	m_disconnectionCause             = other.m_disconnectionCause;

	m_eTelePresenceMode = other.m_eTelePresenceMode;
	if (CPObject::IsValidPObjectPtr(m_telepresenseEPInfo))
		POBJDELETE(m_telepresenseEPInfo);

	if (other.m_telepresenseEPInfo != NULL)
	{
		PTRACE2(eLevelInfoNormal, "CPartyCntl::operator= ", m_name);
		m_telepresenseEPInfo  = new CTelepresenseEPInfo();
		*m_telepresenseEPInfo = *(other.m_telepresenseEPInfo);
	}

	m_subServiceId                      = other.m_subServiceId;
	m_disconnectDelay                   = other.m_disconnectDelay;                                                                                    // VNGR-6603
	m_isWaitingForRsrcAndAskAgain       = other.m_isWaitingForRsrcAndAskAgain;
	m_OldState                          = other.m_OldState;
	m_eLastAllocatedVideoPartyType      = other.m_eLastAllocatedVideoPartyType;
	m_eLastAllocRequestVideoPartyType   = other.m_eLastAllocRequestVideoPartyType;
	m_eLastReAllocRequestVideoPartyType = other.m_eLastReAllocRequestVideoPartyType;
	m_artCapacity                       = other.m_artCapacity;
	m_iAudioDelayUpdated                = other.m_iAudioDelayUpdated;
	m_lastReqId                         = other.m_lastReqId;
	m_isPcmConnected                    = other.m_isPcmConnected;
	m_bNoVideRsrcForVideoParty          = other.m_bNoVideRsrcForVideoParty;
	m_bIsVideoMuted                     = other.m_bIsVideoMuted;
	m_RoomId                            = other.m_RoomId;
	m_bIsMrcCall                        = other.m_bIsMrcCall;
	m_bIsWebRtcCall                     = other.m_bIsWebRtcCall;
	m_maxConnectionRateCurrently        = other.m_maxConnectionRateCurrently;
	m_maxFormatCurrently                = other.m_maxFormatCurrently;
	m_maxFrameRateCurrently             = other.m_maxFrameRateCurrently;
	m_deferUpgrade                      = other.m_deferUpgrade;
	m_bPartyInUpgradeProcess            =  other.m_bPartyInUpgradeProcess;
	strncpy(m_password, other.m_password, sizeof(m_password)-1); m_password[sizeof(m_password)-1] = '\0';
	strncpy(m_siteName, other.m_siteName, sizeof(m_siteName)-1); m_siteName[sizeof(m_siteName)-1] = '\0';
	strncpy(m_productId, other.m_productId, sizeof(m_productId)-1); m_productId[sizeof(m_productId)-1] = '\0';
	strncpy(m_VersionId, other.m_VersionId, sizeof(m_VersionId)-1); m_VersionId[sizeof(m_VersionId)-1] = '\0';
	strncpy(m_partyConfName, other.m_partyConfName, sizeof(m_partyConfName)-1); m_partyConfName[sizeof(m_partyConfName)-1] = '\0';

	m_eRemoteVendorIdent    = other.m_eRemoteVendorIdent;
	m_isMsftEnv             = other.m_isMsftEnv;
	m_reallocatePartyParams = other.m_reallocatePartyParams;
	m_reAllocRtm            = other.m_reAllocRtm;
	m_isReallocSyncMessage  = other.m_isReallocSyncMessage;
	m_reallocOpcode         = other.m_reallocOpcode;

#ifdef PERFORMANCE_TEST
	m_Stopper = other.m_Stopper;
#endif //PERFORMANCE_TEST
}

//--------------------------------------------------------------------------
void CPartyCntl::SetInterfaceType(WORD interfaceType)
{
	m_interfaceType = interfaceType;
}

//--------------------------------------------------------------------------
CConfParty* CPartyCntl::GetConfParty()
{
	const CCommConf*	pCommConf =	m_pConf		? 	m_pConf->GetCommConf()				: NULL;
	return							pCommConf	? 	pCommConf->GetCurrentParty(m_name)	: NULL;
}

//--------------------------------------------------------------------------
void CPartyCntl::SetSiteName(const char* pName)
{
	// If we have valid 'visual name', then we use it.
	// Otherwise, for defined we take the name of the party,
	// and for undefined we take the siteName as we get it from the EP.

	CMedString cstr;
	cstr << "CPartyCntl::SetSiteName,   pName: ";
	if (pName)
		cstr << pName << "\n";
	else
		cstr << "INVALID!\n";

	CConfParty* pConfParty       = m_pConf->GetCommConf()->GetCurrentParty(m_name);
	char*       pPartyVisualName = NULL;
	if (pConfParty)
		pPartyVisualName = pConfParty->GetVisualPartyName();

	if (pPartyVisualName && *pPartyVisualName)
	{
		cstr << "party visual name from DB (pConfParty) is valid, name is: " << pPartyVisualName << " setting site name to visual name\n";
		strncpy(m_siteName, pPartyVisualName, MAX_SITE_NAME_ARR_SIZE - 1);
		m_siteName[MAX_SITE_NAME_ARR_SIZE - 1] = 0;
	}
	else
	{
		cstr << "party visual name from DB (pConfParty) is not valid\n";
		if (IsUndefinedParty() && pName && '\0' != *pName)
		{
			// for dial in party, leave site name empty.
			// if EP has a name, this member will get it later
			if (DIALIN == GetDialType())
			{
				cstr << "party is undefined dial in, keep site name empty\n";
				*m_siteName = '\0';
			}
			else
			{
				cstr << "party is undefined but call type is not dial in, setting site name to pName\n";
				SAFE_COPY(m_siteName, pName);
			}
		}
		else
		{
			cstr << "party is defined or pName is Invalid, setting site name to m_name (" << m_name << ") \n";
			SAFE_COPY(m_siteName, m_name);
		}

		m_siteName[MAX_SITE_NAME_ARR_SIZE-1] = '\0';
	}

	PTRACEPARTYID(eLevelInfoNormal, cstr.GetString(), GetPartyRsrcId());
}

//--------------------------------------------------------------------------
void CPartyCntl::OnPartyUpdateVisualNameAndProductId(CSegment* pParam)
{
	BYTE bIsCascadetoCopMcu;
	*pParam >> bIsCascadetoCopMcu;
	m_IsCascadeToCopMcu = bIsCascadetoCopMcu;
	DWORD len;
	*pParam >> len;
	DWORD len2;
	*pParam >> len2;
	DWORD len3;
	*pParam >> len3;
	eTelePresencePartyType eLocalTelePresencePartyType = eTelePresencePartyNone;
	RemoteIdent eRemoteVendorIdent = Regular;

	PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::OnPartyUpdateVisualNameAndProductId : Name - ", m_partyConfName, GetPartyRsrcId());
	bool has_sitename_changed = false;
	
	if (len)
	{
		char visualName[len + 1];
		memset(visualName, '\0', len + 1);
		pParam->Get((unsigned char*)visualName, len);
		visualName[len] = '\0';

		if(IsNeedToUpdateVisualName())
		{
			CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());

			PASSERTSTREAM_AND_RETURN(!pCommConf, "MonitorConfId:" << GetMonitorConfId() << " - does not exist");

			CConfParty* pConfParty = pCommConf->GetCurrentParty(GetName());

			PASSERTSTREAM_AND_RETURN(!pConfParty, "PartyName:" << GetName() << " - does not exist");

			std::string old_sitename = GetSiteName();
			std::string new_sitename = "";

			char* pSipUserName = strstr(visualName, "#");

			if(pSipUserName)
			{
				*pSipUserName = '\0';
				pConfParty->SetSipUsrName(pSipUserName+1);
			}

			pConfParty->SetRemoteName(visualName);

			PTRACE2(eLevelInfoNormal, "CPartyCntl::OnPartyUpdateVisualNameAndProductId : visualName - ", visualName);
			BYTE isInvitedByPcm  = (strstr(pConfParty->GetName(), "_InvitedByPcm") != NULL);
			BYTE isInvitedByDtmf = (strstr(pConfParty->GetName(), "_Invited") != NULL);

			// Multiple links for ITP in cascaded conference feature: CPartyCntl::OnPartyUpdateVisualNameAndProductId
			BOOL isMainLinkDefined = FALSE;
			if (pConfParty->GetPartyType() != eRegularParty)
			{
				char findMainPartyNameOfThisSubLink[H243_NAME_LEN];   // we don't need this info
				isMainLinkDefined = pCommConf->GetMainLinkNameAccordingToMainPartiesCounterAndReturnIsMainLinkDefined(pConfParty->GetMainPartyNumber(), (char*)findMainPartyNameOfThisSubLink);
			}

			if ((IsUndefinedParty() || (isMainLinkDefined)) &&
				(m_IsGateWay || isInvitedByPcm || m_type == DIALIN) &&
				(!IsFoundInAddressBook() || pConfParty->GetPartyType() != eRegularParty))
			{
				PTRACE(eLevelError, "CPartyCntl::OnPartyUpdateVisualNameAndProductId 1");

				BYTE isEQConf = NO;

				if (pCommConf != NULL && IsValidPObjectPtr(pCommConf))
					isEQConf = pCommConf->GetEntryQ();

				STATUS isVisualNameConflict = STATUS_OK;
				isVisualNameConflict = IsVisualNameConflict(m_pConf->GetName(), visualName, isEQConf, GetMonitorPartyId());
				if ((isVisualNameConflict == STATUS_PARTY_VISUAL_NAME_EXISTS) || (isVisualNameConflict == STATUS_PARTY_NAME_EXISTS)) // This name exists in a party name or visual name
				{
					char updatedVisualName[H243_NAME_LEN];
					memset(updatedVisualName, '\0', ARRAYSIZE(updatedVisualName));

					if (isEQConf)
						::GetUpdatedVisualNameForPartyInEQ(visualName, updatedVisualName);
					else
						::GetUpdatedVisualName(m_pConf->GetName(), visualName, updatedVisualName);

					updatedVisualName[ARRAYSIZE(updatedVisualName) - 1] = '\0';
					size_t updatedVisualNameLen = strlen(updatedVisualName);

					CSegment* pSeg2 = new CSegment;
					*pSeg2 << updatedVisualNameLen;
					pSeg2->Put((unsigned char*)updatedVisualName, updatedVisualNameLen);
					SetSiteName(updatedVisualName);
					m_pTaskApi->UpdateDB(m_pParty, UPDATEVISUALNAME, (DWORD)0, 1, pSeg2);
					POBJDELETE(pSeg2)
				}
				else
				{
					SetSiteName(visualName);
					CSegment* pSeg = new CSegment;
					*pSeg << len;
					pSeg->Put((unsigned char*)visualName, len);
					m_pTaskApi->UpdateDB(m_pParty, UPDATEVISUALNAME, (DWORD)0, 1, pSeg);
					POBJDELETE(pSeg);
				}
				new_sitename = GetSiteName();
				has_sitename_changed = new_sitename != old_sitename;
				PTRACE2(eLevelInfoNormal, "CPartyCntl::OnPartyUpdateVisualNameAndProductId : updated visualName - ", visualName);
			}
		}
	}

	// product ID
	if (len2)
	{
		memset(m_productId, '\0', len2);
		pParam->Get((unsigned char*)m_productId, len2);
	}

	// Version ID
	if (len3)
	{
		memset(m_VersionId, '\0', len3);
		pParam->Get((unsigned char*)m_VersionId, len3);
	}

	BYTE tmp;
	*pParam >>  tmp;
	eLocalTelePresencePartyType = (eTelePresencePartyType)tmp;

	UpdateVidBrdgTelepresenceEPInfoIfNeeded(eLocalTelePresencePartyType, has_sitename_changed); //_e_m_

	BYTE tmpRemoteVendorIdent;
	*pParam >>  tmpRemoteVendorIdent;
	eRemoteVendorIdent = (RemoteIdent)tmpRemoteVendorIdent;

	m_eTelePresenceMode = eLocalTelePresencePartyType;
	m_telepresenseEPInfo->SetEPtype(eLocalTelePresencePartyType);

	// speakerIndication
	m_eRemoteVendorIdent = eRemoteVendorIdent;

	ALLOCBUFFER(ITPStr, 200);
	sprintf(ITPStr, "Party Telepresence mode: %d ", int(eLocalTelePresencePartyType));
	PTRACE2(eLevelInfoNormal, "CPartyCntl::OnPartyUpdateVisualNameAndProductId :Telepresence mode: ", ITPStr);
	DEALLOCBUFFER(ITPStr);
	m_pTaskApi->UpdateDB(m_pParty, PARTYTELEPRESENCEMODE, (DWORD)eLocalTelePresencePartyType, 1, NULL);
}

//_e_m_
//--------------------------------------------------------------------------
void CPartyCntl::UpdateVidBrdgTelepresenceEPInfoIfNeeded(eTelePresencePartyType eLocalTelePresencePartyType, bool isSiteNameChanged /*= false*/)
{
	BOOL bNeedToUpdate  = FALSE;
	BOOL bWaitForUpdate = FALSE;

	PASSERTMSG_AND_RETURN(!m_telepresenseEPInfo || !m_pVideoBridgeInterface, "!m_telepresenseEPInfo || !m_pVideoBridgeInterface");

	TRACEINTO << " m_telepresenseEPInfo->GetEPtype() " << (int)(m_telepresenseEPInfo->GetEPtype())
			  << " eLocalTelePresencePartyType " << (int)eLocalTelePresencePartyType
			  << " AreTwoDirectionsConnectedToVideoBridge() " << (int)(AreTwoDirectionsConnectedToVideoBridge());
	if(m_telepresenseEPInfo->GetEPtype() != eLocalTelePresencePartyType && eLocalTelePresencePartyType != eTelePresencePartyNone)
	{
		m_telepresenseEPInfo->SetEPtype(eLocalTelePresencePartyType);
		bNeedToUpdate = TRUE;
	}
	// bridge-14660 sitename updated -> notify v. bridge (relevant for identifying multi-screen EPs)
	bNeedToUpdate |= isSiteNameChanged;

	bWaitForUpdate = m_telepresenseEPInfo->GetWaitForUpdate();

	CMedString str = "";
	str << "bNeedToUpdate " << (int)bNeedToUpdate << " bWaitForUpdate " << (int)bWaitForUpdate << " eLocalTelePresencePartyType " << (int)eLocalTelePresencePartyType;
	PTRACE2(eLevelInfoNormal, "CPartyCntl::UpdateVidBrdgTelepresenceEPInfoIfNeeded : ", str.GetString());

	if(bWaitForUpdate && bNeedToUpdate && AreTwoDirectionsConnectedToVideoBridge())
	{
		m_telepresenseEPInfo->SetWaitForUpdate(FALSE);

		CMedString str;
		str << " SiteName "			<< m_siteName
			<< " EPtype "  			<< (int)m_telepresenseEPInfo->GetEPtype()
			<< " LinkNum " 			<< (int)m_telepresenseEPInfo->GetLinkNum()
			<< " LinkRole "			<< (int)m_telepresenseEPInfo->GetLinkRole()
			<< " NumOfLinks " 		<< (int)m_telepresenseEPInfo->GetNumOfLinks()
			<< " PartyMonitorID " 	<< (int)m_telepresenseEPInfo->GetPartyMonitorID()
			<< " RTType " 			<< (int)m_telepresenseEPInfo->GetRTType()
			<< " RoomID " 			<< (int)m_telepresenseEPInfo->GetRoomID()
			<< " WaitForUpdate " 	<< (int)m_telepresenseEPInfo->GetWaitForUpdate();
		PTRACE2(eLevelInfoNormal, "EMB_MLA : CPartyCntl::UpdateVidBrdgTelepresenceEPInfoIfNeeded - send update to bridge m_telepresenseEPInfo: ", str.GetString());

		m_pVideoBridgeInterface->UpdateVidBrdgTelepresenseEPInfo(GetPartyRsrcId(), m_telepresenseEPInfo, m_siteName);
	}
}

//--------------------------------------------------------------------------
void CPartyCntl::OnPartyReceivedFaultyRsrc(CSegment* pSeg)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::OnPartyReceivedFaultyRsrc: Name - ", m_partyConfName, GetPartyRsrcId());
	m_isFaulty = 1;
}

//--------------------------------------------------------------------------
void CPartyCntl::OnPartyPcmStateChangedAnycase(CSegment* pParam)
{
	BYTE isConnected = FALSE;
	*pParam >> isConnected;

	if (isConnected)
		ON(m_isPcmConnected);
	else
		OFF(m_isPcmConnected);
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::IsUndefinedParty() const
{
	BYTE   retVal = FALSE;
	CConf* pConf  = GetConf();
	if (CPObject::IsValidPObjectPtr(pConf) && pConf->GetCommConf())
	{
		CConfParty* pConfParty = pConf->GetCommConf()->GetCurrentParty(GetName());
		if (CPObject::IsValidPObjectPtr(pConfParty))
			retVal = pConfParty->IsUndefinedParty();
		else
		{
			DBGPASSERT(1);
			PTRACE2(eLevelError, "CPartyCntl::IsUndefinedParty - No pConfParty ptr ! - ", GetName());
			retVal = FALSE;
		}
	}
	else
	{
		DBGPASSERT(2);
		PTRACE2(eLevelError, "CPartyCntl::IsUndefinedParty - No Conf Ptr or CommConf ptr ! - ", GetName());
		retVal = FALSE;
	}

	return retVal;
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::IsFoundInAddressBook() const
{
	BYTE   retVal = FALSE;
	CConf* pConf  = GetConf();
	if (CPObject::IsValidPObjectPtr(pConf) && pConf->GetCommConf())
	{
		CConfParty* pConfParty = pConf->GetCommConf()->GetCurrentParty(GetName());
		if (CPObject::IsValidPObjectPtr(pConfParty))
			retVal = pConfParty->IsFoundInAddressBook();
		else
		{
			DBGPASSERT(1);
			PTRACE2(eLevelError, "CPartyCntl::IsFoundInAddressBook - No pConfParty ptr ! - ", GetName());
			retVal = FALSE;
		}
	}
	else
	{
		DBGPASSERT(2);
		PTRACE2(eLevelError, "CPartyCntl::IsFoundInAddressBook - No Conf Ptr or CommConf ptr ! - ", GetName());
		retVal = FALSE;
	}
	return retVal;
}

//--------------------------------------------------------------------------
eSessionType CPartyCntl::GetSessionTypeForResourceAllocator()
{
	eSessionType rval = esession_type_none;

	const CCommConf* pCurrentConf = m_pConf->GetCommConf();
	if (pCurrentConf)
	{
		if (pCurrentConf->GetEntryQ())
		{
			rval = eSTANDALONE_session;
		}
		else if (pCurrentConf->IsAudioConf())
		{
			rval = eVOICE_session;
		}
		else if (pCurrentConf->GetVideoSession() == CONTINUOUS_PRESENCE)
		{
			rval = eCP_session;
		}
		else if (pCurrentConf->GetIsHDVSW()) // In case of HD conference the video session is VSW but we allocate resources like CP
		{
			rval = eVSW_Auto_session;
		}
		else if (pCurrentConf->GetVideoSession() == VIDEO_SESSION_COP)
		{
			rval = GetSessionTypeForCop();
		}
		else
		{
			PASSERT(GetPartyRsrcId());
		}
	}
	else
	{
		PASSERT(GetPartyRsrcId());
	}

	return rval;
}

//--------------------------------------------------------------------------
eSessionType CPartyCntl::GetSessionTypeForCop() const
{
	eSessionType retSession = eCOP_HD1080_session;

	// Get the highest cop level params:
	const CCommConf*       pCommConf              = m_pConf->GetCommConf();
	CCOPConfigurationList* pCOPConfigurationList  = pCommConf->GetCopConfigurationList();
	CCopVideoParams*       pCopHighestLevelParams = pCOPConfigurationList->GetVideoMode(0);

	// set session type according to highest level parameters:
	if ((pCopHighestLevelParams->GetFormat() == eCopLevelEncoderVideoFormat_HD720p)
	    && (pCopHighestLevelParams->GetFrameRate() > eCopVideoFrameRate_50))
		retSession = eCOP_HD720_50_session;

	return retSession;
}

//--------------------------------------------------------------------------
STATUS CPartyCntl::SendReqToResourceAllocator(CSegment* seg, OPCODE opcode)
{
	CProcessBase* process = CProcessBase::GetProcess();
	if (!process)
	{
		PASSERT(101);
	}

	CManagerApi api(eProcessResource);
	const StateMachineDescriptor stateMachine = GetStateMachineDescriptor();

	STATUS res = api.SendMsg(seg, opcode, &m_pTaskApi->GetRcvMbx(), &stateMachine);

	return res;
}

//--------------------------------------------------------------------------
STATUS CPartyCntl::SendSyncReqToResourceAllocator(CSegment* seg, OPCODE opcode)
{
	CProcessBase* process = CProcessBase::GetProcess();
	if (!process)
	{
		PASSERT(101);
	}

	CManagerApi api(eProcessResource);

	CSegment* rspMsg = new CSegment;
	OPCODE resOpcode;

	PTRACEPARTYID(eLevelInfoNormal, "***Eitan Debug Before Sync Call!!!", GetPartyRsrcId());
	STATUS       res = api.SendMessageSync(seg, opcode, 5*SECOND, resOpcode, *rspMsg);
	CSmallString str;
	str << res;
	PTRACE2PARTYID(eLevelInfoNormal, "***Eitan Debug After Sync Call!!! res = ", str.GetString(), GetPartyRsrcId());

	DispatchEvent(resOpcode, rspMsg);

	POBJDELETE(rspMsg);


	return res;
}

//--------------------------------------------------------------------------
void CPartyCntl::CreateAndSendAllocatePartyResources( eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType, EAllocationPolicy allocationPolicy,
		                                              BYTE isPortGaugeFlagOn, BYTE IsEnableSipICE,DWORD artCapacity, ETipPartyTypeAndPosition tipPartyType, WORD room_id,
		                                              BOOL isBfcpEnabled, WORD tipNumOfScreens)
{
	eSessionType sessionType = m_pConf->GetSessionTypeForResourceAllocator();
	PASSERT(sessionType == esession_type_none);

	if (strstr(GetName(), "FORCE_108060_AS"))
	{
		TRACEINTO << "FORCE_108060_AS selected, so force video party type to eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type";
		videoPartyType = eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type;
	}

	ALLOC_PARTY_REQ_PARAMS_S allocatePartyParams;
	memset(&allocatePartyParams, 0, sizeof(allocatePartyParams));

//#ifdef LOOKUP_TABLE_DEBUG_TRACE
//	TRACESTRFUNC(eLevelError) << "D.K. m_pParty:" << std::hex << m_pParty << " (" << std::dec << (DWORD)m_pParty << ")";
//#endif

	if ((DWORD)m_pParty == m_monitorPartyId+100 || m_pParty == NULL ||(CPObject::IsValidPObjectPtr(m_pParty) == FALSE))
	{
		allocatePartyParams.party_id = GetLookupIdParty()->Alloc();
		if (0 == allocatePartyParams.party_id)
		{
			TRACEINTO << "Resource party id allocation failed, clear unused party id from LookupId table";
			WORD num_cleaned = CleanLookupIdTablefromLookupPartyTable();
			if (num_cleaned > 0)
			{
				allocatePartyParams.party_id = GetLookupIdParty()->Alloc();
			}
		}
	}
	else
	{
		allocatePartyParams.party_id = m_pParty->GetPartyId();
	}

	allocatePartyParams.monitor_conf_id                = m_monitorConfId;
	allocatePartyParams.monitor_party_id               = m_monitorPartyId;
	allocatePartyParams.confMediaType                  = m_pConf->GetCommConf()->GetConfMediaType();
	allocatePartyParams.room_id                        = room_id;
	allocatePartyParams.networkPartyType               = networkPartyType;
	allocatePartyParams.videoPartyType                 = videoPartyType;
	allocatePartyParams.sessionType                    = sessionType;
	allocatePartyParams.serviceId                      = m_serviceId;
	allocatePartyParams.subServiceId                   = m_subServiceId;
	allocatePartyParams.optionsMask                    = 0;
	allocatePartyParams.bRmxPortGaugeThresholdExceeded = isPortGaugeFlagOn;
	allocatePartyParams.isIceParty                     = IsEnableSipICE;
	allocatePartyParams.artCapacity                    = artCapacity;
	allocatePartyParams.tipPartyType                   = tipPartyType;
	allocatePartyParams.isBFCP                         = isBfcpEnabled;
	allocatePartyParams.reqMsSsrc                      = (m_isMsftEnv && isMsftSvc2013Supported());
	allocatePartyParams.tipNumOfScreens                = tipNumOfScreens;
	allocatePartyParams.avMcuLinkType                  = GetAvMcuLinkType();


	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD confRate = lCapInfo.TranslateReservationRateToIpRate(m_pConf->GetCommConf()->GetConfTransferRate());
	
	BOOL  is1080pVswSupported = Is1080pSupportedInOperationPoint(confRate);
	
    if (m_bIsMrcCall)
    {
    	if(m_pConf->GetCommConf()->GetEnableHighVideoResInSvcToAvcMixMode())
			allocatePartyParams.HdVswTypeInMixAvcSvcMode = is1080pVswSupported ? e_HdVsw1080 : e_HdVsw720;
		else
			allocatePartyParams.HdVswTypeInMixAvcSvcMode = eHdVswNon;

		TRACEINTO << "allocatePartyParams.HdVswTypeInMixAvcSvcMode is:" << (int)(allocatePartyParams.HdVswTypeInMixAvcSvcMode);
    }
    else
    {
        if(m_pConf->GetCommConf()->GetEnableHighVideoResInAvcToSvcMixMode())
			allocatePartyParams.HdVswTypeInMixAvcSvcMode = is1080pVswSupported ? e_HdVsw1080 : e_HdVsw720;
		else
			allocatePartyParams.HdVswTypeInMixAvcSvcMode = eHdVswNon;

		TRACEINTO << "allocatePartyParams.HdVswTypeInMixAvcSvcMode is:" << (int)(allocatePartyParams.HdVswTypeInMixAvcSvcMode);
    }
	


	switch (allocatePartyParams.avMcuLinkType)
	{
		case eAvMcuLinkSlaveOut: allocatePartyParams.partyRole = eParty_Role_AvMcuLink_SlaveOut; break;
		case eAvMcuLinkSlaveIn : allocatePartyParams.partyRole = eParty_Role_AvMcuLink_SlaveIn ; break;
		default                : allocatePartyParams.partyRole = eParty_Role_regular_party     ; break;
	}
	if(allocatePartyParams.avMcuLinkType == eAvMcuLinkSlaveOut || allocatePartyParams.avMcuLinkType == eAvMcuLinkSlaveIn)
	{
		//TRACEINTO << "slave av-mcu ";
		allocatePartyParams.mainPartyRsrcID = m_MasterRsrcId;
	}
	if(allocatePartyParams.avMcuLinkType == eAvMcuLinkMain)
	{
		//TRACEINTO << "main av-mcu ";
		allocatePartyParams.mainPartyRsrcID = allocatePartyParams.party_id;
	}

	if (!m_bIsMrcCall) // not SVC
	{
		allocatePartyParams.allocationPolicy         = allocationPolicy;
		allocatePartyParams.isWaitForRsrcAndAskAgain = m_isWaitingForRsrcAndAskAgain;

	}
	else // SVC
	{
		allocatePartyParams.allocationPolicy         = eAllocateAllRequestedResources;
		allocatePartyParams.isWaitForRsrcAndAskAgain = NO;
	}

	// Setting Specified party params
	SetPartyTypeRelevantInfo(allocatePartyParams);

	m_eLastAllocRequestVideoPartyType = videoPartyType;

	TRACEINTO << m_partyConfName << allocatePartyParams;

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&allocatePartyParams), sizeof(allocatePartyParams));
	STATUS res = SendReqToResourceAllocator(seg, ALLOCATE_PARTY_RSRC_REQ);
	PASSERT(res);
}

//--------------------------------------------------------------------------
void CPartyCntl::CreateAndSendReAllocatePartyResources(eNetworkPartyType networkPartyType,
                                                       eVideoPartyType videoPartyType,
                                                       EAllocationPolicy allocationPolicy,
                                                       WORD reAllocRtm,
                                                       WORD isSyncMessage,
                                                       BYTE IsEnableSipICE,
                                                       DWORD artCapacity,
                                                       ETipPartyTypeAndPosition tipPartyType,
                                                       WORD room_id,
                                                       OPCODE opcode)
{
	memset(&m_reallocatePartyParams, 0, sizeof(m_reallocatePartyParams));

	if (strstr(GetName(), "FORCE_108060_AS"))
	{
		TRACEINTO << "FORCE_108060_AS selected, so force video party type to eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type";
		videoPartyType = eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type;
	}

	m_reallocatePartyParams.monitor_conf_id  = m_monitorConfId;       // must
	m_reallocatePartyParams.monitor_party_id = m_monitorPartyId;      // must
	m_reallocatePartyParams.party_id = GetPartyRsrcId();
	m_reallocatePartyParams.confMediaType    = m_pConf->GetCommConf()->GetConfMediaType();
	m_reallocatePartyParams.networkPartyType = networkPartyType;
	m_reallocatePartyParams.videoPartyType   = videoPartyType;
	m_reallocatePartyParams.sessionType      = (eSessionType)0;       // not needed
	m_reallocatePartyParams.serviceId        = m_serviceId;           // not needed
	m_reallocatePartyParams.subServiceId     = m_subServiceId;
	m_reallocatePartyParams.optionsMask      = 0;                     // not needed
	m_reallocatePartyParams.isIceParty       = IsEnableSipICE;
	m_reallocatePartyParams.artCapacity      = artCapacity;
	m_reallocatePartyParams.tipPartyType     = tipPartyType;
	m_reallocatePartyParams.room_id          = room_id;
	m_reallocatePartyParams.reqMsSsrc        = m_isMsftEnv;
	m_reallocatePartyParams.avMcuLinkType    = GetAvMcuLinkType();

	
	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD confRate = lCapInfo.TranslateReservationRateToIpRate(m_pConf->GetCommConf()->GetConfTransferRate());


    BOOL  is1080pVswSupported = Is1080pSupportedInOperationPoint(confRate);
    if (m_bIsMrcCall)
    {
    	if(m_pConf->GetCommConf()->GetEnableHighVideoResInSvcToAvcMixMode())
			m_reallocatePartyParams.HdVswTypeInMixAvcSvcMode = is1080pVswSupported ? e_HdVsw1080 : e_HdVsw720;
		else
			m_reallocatePartyParams.HdVswTypeInMixAvcSvcMode = eHdVswNon;
    }
    else
    {
        if(m_pConf->GetCommConf()->GetEnableHighVideoResInAvcToSvcMixMode())
			m_reallocatePartyParams.HdVswTypeInMixAvcSvcMode = is1080pVswSupported ? e_HdVsw1080 : e_HdVsw720;
		else
			m_reallocatePartyParams.HdVswTypeInMixAvcSvcMode = eHdVswNon;
    }
	

	switch (m_reallocatePartyParams.avMcuLinkType)
	{
		case eAvMcuLinkSlaveOut: m_reallocatePartyParams.partyRole = eParty_Role_AvMcuLink_SlaveOut; break;
		case eAvMcuLinkSlaveIn : m_reallocatePartyParams.partyRole = eParty_Role_AvMcuLink_SlaveIn ; break;
		default                : m_reallocatePartyParams.partyRole = eParty_Role_regular_party     ; break;
	}
	if(m_reallocatePartyParams.avMcuLinkType == eAvMcuLinkSlaveOut || m_reallocatePartyParams.avMcuLinkType == eAvMcuLinkSlaveIn)
	{
		//TRACEINTO << "slave av-mcu ";
		m_reallocatePartyParams.mainPartyRsrcID = m_MasterRsrcId;
	}
	if(m_reallocatePartyParams.avMcuLinkType == eAvMcuLinkMain)
	{
		//TRACEINTO << "main av-mcu ";
		m_reallocatePartyParams.mainPartyRsrcID = GetPartyRsrcId() ;
	}

	if (m_reallocatePartyParams.confMediaType == eMixAvcSvc)
	    m_reallocatePartyParams.isBFCP = TRUE;

	if (!m_bIsMrcCall) // not SVC
	{
	    m_reallocatePartyParams.allocationPolicy = allocationPolicy;
	}
	else               // SVC
	{
	    m_reallocatePartyParams.allocationPolicy         = eAllocateAllRequestedResources;
		m_reallocatePartyParams.isWaitForRsrcAndAskAgain = NO;
	}

	// this section is for Isdn parties
	if (networkPartyType == eISDN_network_party_type)
	{
		// Setting Specified party params
		SetPartyTypeRelevantInfo(m_reallocatePartyParams);

		// in realloc we don't need temp phone number
		m_reallocatePartyParams.isdn_span_params.isBondingTemporaryPhoneNumberNeeded = 0;
	}

	// check if this is deescalation from HD->SD then maybe a translator ART should be
	// disconnected
	int numOfArtsToClose = IsNeedToCloseInternalArt(m_reallocatePartyParams.videoPartyType);

	if (numOfArtsToClose) // Send message to the party to close the ART
	{
		m_reAllocRtm = reAllocRtm;
		m_isReallocSyncMessage = isSyncMessage;
		m_reallocOpcode = opcode;
		StartTimer(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT, DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT_VALUE);

		return Deescalate(numOfArtsToClose);
	}

	SendReAllocatePartyResources(reAllocRtm, isSyncMessage, opcode);
}

//--------------------------------------------------------------------------
void CPartyCntl::SendReAllocatePartyResources(WORD reAllocRtm, WORD isSyncMessage, OPCODE opcode)
{
	m_eLastReAllocRequestVideoPartyType = m_reallocatePartyParams.videoPartyType;

	TRACEINTO << m_partyConfName << m_reallocatePartyParams;

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&m_reallocatePartyParams), sizeof(m_reallocatePartyParams));

	// sending REALLOC to rsrc allocator - No need for additional rsrc
	STATUS res = STATUS_OK;
	if (isSyncMessage != 0)
		res = SendSyncReqToResourceAllocator(seg, opcode /*REALLOCATE_PARTY_RSRC_REQ*/);
	else
		res = SendReqToResourceAllocator(seg, opcode /* REALLOCATE_PARTY_RSRC_REQ*/);

	PASSERT(res);
}

//--------------------------------------------------------------------------
void CPartyCntl::OnEndAvcToSvcArtTranslatorDisconnected(CSegment* pParam)
{
    STATUS status;
    *pParam >> status;
    DeleteTimer(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT);

    UpdateStreamsListOfCurMode();
    // continue realloc
    SendReAllocatePartyResources(m_reAllocRtm, m_isReallocSyncMessage, m_reallocOpcode);
}

//--------------------------------------------------------------------------
void CPartyCntl::OnDisconnectAvcToSvcArtTranslatorCloseTout(CSegment* pParam)
{
    TRACEINTO << "Disconnect party";

    // disconnect party
    CSegment *pSeg = new CSegment;
    *pSeg << m_name;
    m_pTaskApi->UpdateDB(NULL,DISCAUSE,MCU_INTERNAL_PROBLEM,1,pSeg); // Disconnnect cause
    m_pTaskApi->EndAddParty(m_pParty,statIllegal);
    POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CPartyCntl::CreateAndSendReAllocateArtForParty(eNetworkPartyType networkPartyType,
                                                    eVideoPartyType videoPartyType,
                                                    EAllocationPolicy allocationPolicy,
                                                    BYTE IsEnableSipICE,
                                                    DWORD artCapacity,
                                                    ETipPartyTypeAndPosition tipPartyType)
{
	ALLOC_PARTY_REQ_PARAMS_S allocatePartyParams;
	memset(&allocatePartyParams, 0, sizeof(allocatePartyParams));

	allocatePartyParams.monitor_conf_id  = m_monitorConfId;       // must
	allocatePartyParams.monitor_party_id = m_monitorPartyId;      // must
	allocatePartyParams.confMediaType    = m_pConf->GetCommConf()->GetConfMediaType();
	allocatePartyParams.networkPartyType = networkPartyType;
	allocatePartyParams.videoPartyType   = videoPartyType;
	allocatePartyParams.sessionType      = (eSessionType)0;       // not needed
	allocatePartyParams.serviceId        = m_serviceId;           // not needed
	allocatePartyParams.subServiceId     = m_subServiceId;
	allocatePartyParams.optionsMask      = 0;                     // not needed
	allocatePartyParams.isIceParty       = IsEnableSipICE;
	allocatePartyParams.artCapacity      = artCapacity;
	allocatePartyParams.tipPartyType     = tipPartyType;
	allocatePartyParams.reqMsSsrc        = m_isMsftEnv;
	allocatePartyParams.avMcuLinkType    = GetAvMcuLinkType();
	
	CCapSetInfo  lCapInfo = eUnknownAlgorithemCapCode;
	DWORD confRate = lCapInfo.TranslateReservationRateToIpRate(m_pConf->GetCommConf()->GetConfTransferRate());
    BOOL  is1080pVswSupported = Is1080pSupportedInOperationPoint(confRate);
	
	if (m_bIsMrcCall)
	{
		if(m_pConf->GetCommConf()->GetEnableHighVideoResInSvcToAvcMixMode())
			m_reallocatePartyParams.HdVswTypeInMixAvcSvcMode = is1080pVswSupported ? e_HdVsw1080 : e_HdVsw720;
		else
			m_reallocatePartyParams.HdVswTypeInMixAvcSvcMode = eHdVswNon;
	}
	else
	{
		if(m_pConf->GetCommConf()->GetEnableHighVideoResInAvcToSvcMixMode())
			m_reallocatePartyParams.HdVswTypeInMixAvcSvcMode = is1080pVswSupported ? e_HdVsw1080 : e_HdVsw720;
		else
			m_reallocatePartyParams.HdVswTypeInMixAvcSvcMode = eHdVswNon;
	}
	
	switch (allocatePartyParams.avMcuLinkType)
	{
		case eAvMcuLinkSlaveOut: allocatePartyParams.partyRole = eParty_Role_AvMcuLink_SlaveOut; break;
		case eAvMcuLinkSlaveIn : allocatePartyParams.partyRole = eParty_Role_AvMcuLink_SlaveIn ; break;
		default                : allocatePartyParams.partyRole = eParty_Role_regular_party     ; break;
	}

	if (!m_bIsMrcCall)                                            // not SVC
	{
		allocatePartyParams.allocationPolicy = allocationPolicy;
	}
	else                                                          // SVC
	{
		allocatePartyParams.allocationPolicy         = eAllocateAllRequestedResources;
		allocatePartyParams.isWaitForRsrcAndAskAgain = NO;
	}

	// this section is for Isdn parties
	m_eLastReAllocRequestVideoPartyType = videoPartyType;

	TRACEINTO << m_partyConfName << allocatePartyParams;

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&allocatePartyParams), sizeof(allocatePartyParams));

	// sending REALLOC to rsrc allocator - No need for additional rsrc
	SendReqToResourceAllocator(seg, REALLOCATE_ART_PARTY_REQ);
}

//--------------------------------------------------------------------------
void CPartyCntl::CreateAndSendDeallocatePartyResources(BYTE IsEnableSipICE, DWORD rtpArr[], DWORD rtcpArr[], BOOL bIsPartyAllocated)
{
	DEALLOC_PARTY_REQ_PARAMS_S deallocatePartyParams;
	memset(&deallocatePartyParams, 0, sizeof(deallocatePartyParams));
	deallocatePartyParams.monitor_conf_id      = m_monitorConfId;
	deallocatePartyParams.monitor_party_id     = m_monitorPartyId;
	deallocatePartyParams.force_kill_all_ports = FALSE;

	if (m_service_provider)
	{
		strncpy((char*)deallocatePartyParams.serviceName, m_service_provider, GENERAL_SERVICE_PROVIDER_NAME_LEN);
		deallocatePartyParams.serviceName[GENERAL_SERVICE_PROVIDER_NAME_LEN - 1] = '\0';
	}

	// =============================================================================
	// (VNGR-24038)  Unconditionally terminating the CONNECTTOUT timer,
	// for a case where an ongoing call establishment failed resource allocation.
	// =============================================================================
	if (IsValidTimer(CONNECTTOUT)) DeleteTimer(CONNECTTOUT);

	if (bIsPartyAllocated)   // VNGR-23685
	{
		if (m_isFaulty == 1)
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::CreateAndSendDeallocatePartyResources : Faulty flag is ON. Name - ", m_partyConfName, GetPartyRsrcId());
			DWORD numOfResourcesWithProblem = 2; // (eLogical_audio_encoder , eLogical_audio_decoder )

			// Faulty resources
			DWORD connectionId = 0;
			connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_audio_encoder);
			deallocatePartyParams.rsrcsWithProblems[eLogical_audio_encoder].connectionId    = connectionId;
			deallocatePartyParams.rsrcsWithProblems[eLogical_audio_encoder].logicalRsrcType = eLogical_audio_encoder;

			connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_audio_decoder);
			deallocatePartyParams.rsrcsWithProblems[eLogical_audio_decoder].connectionId    = connectionId;
			deallocatePartyParams.rsrcsWithProblems[eLogical_audio_decoder].logicalRsrcType = eLogical_audio_decoder;

			if (GetInterfaceType() != ISDN_INTERFACE_TYPE)
			{
				connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_rtp);
				deallocatePartyParams.rsrcsWithProblems[eLogical_rtp].connectionId    = connectionId;
				deallocatePartyParams.rsrcsWithProblems[eLogical_rtp].logicalRsrcType = eLogical_rtp;
				numOfResourcesWithProblem++;
			}

			if (!m_voice)
			{
				connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_video_encoder);
				deallocatePartyParams.rsrcsWithProblems[eLogical_video_encoder].connectionId    = connectionId;
				deallocatePartyParams.rsrcsWithProblems[eLogical_video_encoder].logicalRsrcType = eLogical_video_encoder;
				numOfResourcesWithProblem++;

				connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_video_decoder);
				deallocatePartyParams.rsrcsWithProblems[eLogical_video_decoder].connectionId    = connectionId;
				deallocatePartyParams.rsrcsWithProblems[eLogical_video_decoder].logicalRsrcType = eLogical_video_decoder;
				numOfResourcesWithProblem++;

				if (GetInterfaceType() == ISDN_INTERFACE_TYPE)
				{
					connectionId = m_pPartyAllocatedRsrc->GetConnectionId(eLogical_mux);
					deallocatePartyParams.rsrcsWithProblems[eLogical_mux].connectionId    = connectionId;
					deallocatePartyParams.rsrcsWithProblems[eLogical_mux].logicalRsrcType = eLogical_mux;
					numOfResourcesWithProblem++;
				}
			}

			deallocatePartyParams.is_problem_with_UDP_ports = 1;
			deallocatePartyParams.numOfRsrcsWithProblems    = numOfResourcesWithProblem;

			if (IsEnableSipICE)
			{
				for (int i = 0; i < MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS; i++)
				{
					deallocatePartyParams.rtp_ice_channels[i]  = 0;
					deallocatePartyParams.rtcp_ice_channels[i] = 0;
				}
			}

			// Incase of Kill port and ice call, CM doesn't know the 2 allocated ports (2 ports per ice call)
			// but knows the ice channels Ids. -->RA will forward the channels Ids to the CM in Kill port Msg.
			if (IsEnableSipICE)
			{
				PTRACE(eLevelInfoNormal, "CPartyCntl::CreateAndSendDeallocatePartyResources :ICE is ON");

				for (int i = 0; i < MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS; i++)
				{
					deallocatePartyParams.rtp_ice_channels[i]  = rtpArr[i];
					deallocatePartyParams.rtcp_ice_channels[i] = rtcpArr[i];
				}
			}

			if (1 == m_isRecovery)   // Invoke DSP recovery in case of Timer on Open CardManager ART
			{
				deallocatePartyParams.resetArtUnitOnKillPort = YES;
				m_isRecovery = FALSE;
			}

			else
				deallocatePartyParams.resetArtUnitOnKillPort = NO;
		}
	}

	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyRsrcId() << deallocatePartyParams;

	CSegment* seg = new CSegment;
	seg->Put((BYTE*)(&deallocatePartyParams), sizeof(deallocatePartyParams));

	STATUS res = SendReqToResourceAllocator(seg, DEALLOCATE_PARTY_RSRC_REQ);
	PASSERT(res);
}

//--------------------------------------------------------------------------
void CPartyCntl::UpdatePartyEntryInGlobalRsrcRoutingTblAfterMove()
{
	CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();

	if (pRoutingTbl == NULL)
	{
		PASSERT_AND_RETURN(101);
	}

	CTaskApi* pTaskApi = new CTaskApi(*m_pTaskApi);

	pTaskApi->CreateOnlyApi(m_pTaskApi->GetRcvMbx(), NULL);

	pRoutingTbl->AddStateMachinePointerToRoutingTbl(GetPartyRsrcId(), eLogical_res_none, pTaskApi);  // function deletes old and adds new

	POBJDELETE(pTaskApi);
}

//--------------------------------------------------------------------------
void CPartyCntl::SetDataForExportPartyFail()
{
	if (m_isPartyEndRAMoveOK == FALSE)
		return;

	// Here we set the faulty flag to ON and replace the conference Id's. The party is now in destination conf.
	m_isFaulty = 1;
	// In case the import failed due to wrong params, we need to dealloc the resources
	// From the destination conf.
	TRACEINTO
		<< "\n  ConfId            :" << m_pPartyHWInterface->GetConfRsrcId()
		<< "\n  DestConfId        :" << m_destResourceConfId
		<< "\n  MonitorConfId     :" << m_monitorConfId
		<< "\n  DestMonitorConfId :" << m_destMonitorConfId
		<< "\n  ConnectionId      :" << m_pPartyHWInterface->GetConnectionId()
		<< "\n  PartyId           :" << m_pPartyHWInterface->GetPartyRsrcId();

	m_pPartyAllocatedRsrc->SetConfRsrcId(m_destResourceConfId);
	m_pPartyHWInterface->SetConfRsrcId(m_destResourceConfId);
	m_monitorConfId  = m_destMonitorConfId;
	m_monitorPartyId = m_destMonitorPartyId;
	if (m_pPartyApi)
		m_pPartyApi->SetRsrcConfIdForInterface(m_destResourceConfId);
	else
		TRACEINTO << " Error: m_pPartyApi == NULL";
}

//--------------------------------------------------------------------------
void CPartyCntl::SetPartyVideoMute(CDwordBitMask& muteMask)
{
	muteMask.ResetMask();
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if (CPObject::IsValidPObjectPtr(pConfParty))
	{
		if (pConfParty->IsVideoMutedByOperator())
			muteMask.SetBit(OPERATOR_Prior);

		if (pConfParty->IsVideoMutedByParty())
			muteMask.SetBit(PARTY_Prior);

		if (pConfParty->IsVideoMutedByMCU())
			muteMask.SetBit(MCMS_Prior);
	}
}

//--------------------------------------------------------------------------
void CPartyCntl::SetPartyAudioMute(CDwordBitMask& muteMask)
{
	muteMask.ResetMask();
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if (CPObject::IsValidPObjectPtr(pConfParty))
	{
		if (pConfParty->IsAudioMutedByOperator())
			muteMask.SetBit(OPERATOR);

		if (pConfParty->IsAudioMutedByParty())
			muteMask.SetBit(PARTY);

		if (pConfParty->IsAudioMutedByMCU())
			muteMask.SetBit(MCMS);
	}
}

//--------------------------------------------------------------------------
void CPartyCntl::SetPartyAudioBlock(CDwordBitMask& blockMask)
{
	blockMask.ResetMask();
	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
	if (CPObject::IsValidPObjectPtr(pConfParty))
	{
		if (pConfParty->IsAudioBlocked())
			blockMask = 0x00000001;
	}
}

//--------------------------------------------------------------------------
BOOL CPartyCntl::IsAgcOn() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return FALSE;

	CConfParty* pConfParty = pConf->GetCommConf()->GetCurrentParty(GetName());
	if (!pConfParty)
		return FALSE;

	return (pConfParty->GetAGC() ? TRUE : FALSE);
}

//--------------------------------------------------------------------------
BOOL CPartyCntl::GetEchoSuppression() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return FALSE;

	return (pConf->GetCommConf()->GetEchoSuppression() ? TRUE : FALSE);
}

//--------------------------------------------------------------------------
BOOL CPartyCntl::IsConfTelePresenceMode() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return FALSE;

	return (pConf->GetCommConf()->GetIsTelePresenceMode() ? TRUE : FALSE);
}

//--------------------------------------------------------------------------
BOOL CPartyCntl::GetKeyboardSuppression() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return FALSE;

	return (pConf->GetCommConf()->GetKeyboardSuppression() ? TRUE : FALSE);
}
//--------------------------------------------------------------------------
BOOL CPartyCntl::GetAutoMuteNoisyParties() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return FALSE;

	return (pConf->GetCommConf()->GetAutoMuteNoisyParties() ? TRUE : FALSE);
}
//--------------------------------------------------------------------------
BOOL CPartyCntl::GetAudioClarity() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return FALSE;

	return (pConf->GetCommConf()->GetIsAudioClarity() ? TRUE : FALSE);
}

//--------------------------------------------------------------------------
WORD CPartyCntl::GetConfSpeakerChangeMode() const
{
	WORD   confSpeakerChangeMode = E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;
	CConf* pConf = GetConf();

	if (pConf)
		confSpeakerChangeMode = pConf->GetCommConf()->GetConfSpeakerChangeMode();

	return confSpeakerChangeMode;
}

//--------------------------------------------------------------------------
BOOL CPartyCntl::GetAutoBrightness() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return FALSE;

	return (pConf->GetCommConf()->GetAutoBrightness() ? TRUE : FALSE);
}

//--------------------------------------------------------------------------
DWORD CPartyCntl::GetAudioVolume() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return 0;

	CConfParty* pConfParty = pConf->GetCommConf()->GetCurrentParty(GetName());
	if (!pConfParty)
		return 0;

	return (pConfParty->GetAudioVolume());
}

//--------------------------------------------------------------------------
DWORD CPartyCntl::GetListeningAudioVolume() const
{
	CConf* pConf = GetConf();
	if (!pConf)
		return 0;

	CConfParty* pConfParty = pConf->GetCommConf()->GetCurrentParty(GetName());
	if (!pConfParty)
		return 0;

	return (pConfParty->GetListeningAudioVolume());
}

//--------------------------------------------------------------------------
void CPartyCntl::SetIsAutoVidBitRate(BYTE isAutoVidBitRate)
{
	m_isAutoVidBitRate = isAutoVidBitRate;
}

//--------------------------------------------------------------------------
// content brdg disconnection ack
int CPartyCntl::OnContentBrdgDisconnected(CSegment* pParam)
{
	int   isErr      = 0;
	char* strDetails = GetPartyAndClassDetails();
	PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::OnContBrdgDisConContent - ", strDetails, GetPartyRsrcId());
	DEALLOCBUFFER(strDetails);

	WORD status;
	*pParam >> status;
	DBGPASSERT(status);
	OFF(m_isContentConn);
	m_pTaskApi->UpdateDB(m_pParty, CONTENTCON, FALSE);
	if (status)
	{
		PTRACE(eLevelInfoNormal, "CPartyCntl::OnContBrdgDisConContent - failure status");
		if (status == statContentInOutResourceProblem)
			m_isFaulty = 1;

		isErr = 1;
	}
	return isErr;
}

//////////////////////////////////////////////////////////////////
void  CPartyCntl::OnXCodeBrdgDisconnected(CSegment* pParam)
{
	WORD  status;
	*pParam >> status;
	DBGPASSERT(status);
	OFF(m_isXCodeConn);
}
/////////////////////////////////////////////////////////////////
int CPartyCntl::OnContentBrdgConnected(CSegment* pParam)
{
	WORD status;
	int  isErr = 0;
	*pParam >> status;
	if (status)
	{
		PASSERT(status);
		if (status == statContentInOutResourceProblem)
			m_isFaulty = 1;

		isErr = 1;
		OFF(m_isContentConn);
	}

	else
	{
		PTRACE2PARTYID(eLevelInfoNormal, " ---> Content Connection Established ", m_partyConfName, GetPartyRsrcId());
		m_pTaskApi->UpdateDB(m_pParty, CONTENTCON, TRUE);
	}

	return isErr;
}

//--------------------------------------------------------------------------
/////////////////////////////////////////////////////////////////////////////
void CPartyCntl::OnXCodeBrdgConnected(CSegment* pParam)
{
	WORD  status;
	*pParam >> status;
	if ( status )
	{
		PASSERT(status);
		OFF(m_isXCodeConn);
	}

	else
	{
		PTRACE2PARTYID(eLevelInfoNormal," ---> XCode Connection Established ",m_partyConfName, GetPartyRsrcId());
	}
}
/////////////////////////////////////////////////////////////////////////////
DWORD CPartyCntl::GetMpiErrorNumber(CSegment* pParam)
{
	BYTE  hardWareConnectionDigit = 0;
	BYTE  mediaDigit              = 0;
	BYTE  dirctionDigit           = 0;
	BYTE  timerStatusDigit        = 0;
	BYTE  actionDigit             = 0;
	DWORD MipErrorNumber          = 0;

	*pParam >> hardWareConnectionDigit;

	if (hardWareConnectionDigit != 0)
	{
		*pParam >> mediaDigit >> dirctionDigit >> timerStatusDigit >> actionDigit;

		MipErrorNumber = ::CalculateMcuInternalProblemErrorNumber(hardWareConnectionDigit, mediaDigit, dirctionDigit,
		                                                          timerStatusDigit, actionDigit);
	}

	CSmallString str;
	str << MipErrorNumber;
	PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::GetMpiErrorNumber = ", str.GetString(), GetPartyRsrcId());

	return MipErrorNumber;
}

//--------------------------------------------------------------------------
DWORD CPartyCntl::VideoCanBeOpened(CComMode* pScm) const
{
	DWORD aud_bitrate     = 0;
	DWORD vid_bitrate     = 0;
	DWORD lsd_bitrate     = 0;
	DWORD hsd_bitrate     = 0;
	DWORD mlp_bitrate     = 0;
	DWORD hmlp_bitrate    = 0;
	DWORD content_bitrate = 0;

	pScm->GetMediaBitrate(aud_bitrate, vid_bitrate, lsd_bitrate,
	                      hsd_bitrate, mlp_bitrate, hmlp_bitrate, content_bitrate);

	return vid_bitrate;
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::isConfH239Cascade() const
{
	if (CPObject::IsValidPObjectPtr(m_pContentBridge) &&
	    (!strcmp(m_pContentBridge->NameOf(), "CContentBridgeSlave") || !strcmp(m_pContentBridge->NameOf(), "CContentBridgeMaster")))
		return TRUE;
	else if (CASCADE_MCU == GetPartyCascadeType())
		// if the party is link it might still not be connected to the content bridge - so bridge might be regular
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------------
void CPartyCntl::UpdatePartyStateInCdr()
{
	if (m_isCDRPartyConnected == 0 && GetAvMcuLinkType() != eAvMcuLinkSlaveOut && GetAvMcuLinkType() != eAvMcuLinkSlaveIn)
	{
		// This is the first time we update the CDR
		m_pTaskApi->UpdatePartyStateInCdr(m_pParty);
		ON(m_isCDRPartyConnected);
	}
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::CheckDisconnectCases(WORD mode, DWORD disconnectionDelay)
{
	BYTE bContinueDisconnect = TRUE;
	if ((mode == DELETE_MODE) && m_isDisconnect && (m_disconnectState != DISCONNECTED))
	{
		PTRACE2PARTYID(eLevelError, "CPartyCntl::CheckDisconnectCases - req while deleting", m_partyConfName, GetPartyRsrcId());
		CPartyCntl::Disconnect(mode);
		bContinueDisconnect = FALSE;
	}

	else if ((mode == DISCONNECT_MODE) && m_isDisconnect)
	{
		if (m_disconnectMode)
			PTRACE2PARTYID(eLevelError, "CPartyCntl::CheckDisconnectCases - req while disconnecting", m_partyConfName, GetPartyRsrcId());
		else
			PTRACE2PARTYID(eLevelError, "CPartyCntl::CheckDisconnectCases - req while deleting", m_partyConfName, GetPartyRsrcId());

		bContinueDisconnect = FALSE;
	}

	else if (m_disconnectState == DISCONNECTED)
	{
		PTRACE2PARTYID(eLevelError, "CPartyCntl::CheckDisconnectCases - m_disconnectState == DISCONNECTED : Name - ", m_partyConfName, GetPartyRsrcId());
		if (mode == DELETE_MODE)
			CPartyCntl::Disconnect(mode);

		UpdateConfEndDisconnect(statOK);
		bContinueDisconnect = FALSE;
	}

	else if (IsValidTimer(DISCONNECTDELAY))
	{
		// bug fix (reproduces by call generator), disconnect all parties and immediately terminate conf
		// if terminate arrived during timer - party and conf stack and can't be deleted
		if (mode == DELETE_MODE)
		{
			if (disconnectionDelay)
			{
				PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::CheckDisconnectCases - delete party called while DISCONNECTDELAY timer already activated - restart timer. Name - ", m_partyConfName, GetPartyRsrcId());
				CSegment* pPartyDetails = new CSegment;
				*pPartyDetails << mode;
				DeleteTimer(DISCONNECTDELAY);
				StartTimer(DISCONNECTDELAY, disconnectionDelay, pPartyDetails);
				// VNGR-6603
				m_disconnectDelay   = 1;
				bContinueDisconnect = FALSE;
			}
			else
			{
				DeleteTimer(DISCONNECTDELAY);
				PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::CheckDisconnectCases - delete party called while DISCONNECTDELAY timer already activated - continue delete. Name - ", m_partyConfName, GetPartyRsrcId());
			}
		}
		else
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::CheckDisconnectCases - DISCONNECTDELAY timer already activated. Name - ", m_partyConfName, GetPartyRsrcId());
			CPartyCntl::Disconnect(mode);
			bContinueDisconnect = FALSE;
		}
	}

	else if (disconnectionDelay)
	{
		CSegment* pPartyDetails = new CSegment;
		*pPartyDetails << mode;

		PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::CheckDisconnectCases - disconnectionDelay. Name - ", m_partyConfName, GetPartyRsrcId());
		StartTimer(DISCONNECTDELAY, disconnectionDelay, pPartyDetails);
		// VNGR-6603
		m_disconnectDelay   = 1;
		bContinueDisconnect = FALSE;
	}

	return bContinueDisconnect;
}

//--------------------------------------------------------------------------
void CPartyCntl::SendStartMoveToResourceProcess()
{
	m_state = EXPORT_RESOURCE;

	// 3)
	// A. Start moving the resources from 1 conf to the other
	// B. Start a timer for the first "start move" resourse stage.

	// Here we will move the resources in RA
	PARTY_MOVE_RSRC_REQ_PARAMS_S* pReq = new PARTY_MOVE_RSRC_REQ_PARAMS_S;
	pReq->source_monitor_conf_id  = m_monitorConfId;
	pReq->target_monitor_conf_id  = m_destMonitorConfId;
	pReq->source_monitor_party_id = m_monitorPartyId;
	pReq->target_monitor_party_id = 0XFFFFFFFF;  // m_destPartyId; // TBD 0xfffffff ?
	if (m_service_provider)
		strncpy((char*)pReq->serviceName, m_service_provider, GENERAL_SERVICE_PROVIDER_NAME_LEN);

	pReq->serviceName[GENERAL_SERVICE_PROVIDER_NAME_LEN - 1] = '\0';

	CMedString* pStrReq = new CMedString;
	*pStrReq <<"CPartyCntl::SendStartMoveToResourceProcess - START_PARTY_MOVE_RSRC_REQ:\n"
	         << "source_monitor_conf_id	   =   "<< pReq->source_monitor_conf_id <<",  "
	         << "target_monitor_conf_id     =   "<< pReq->target_monitor_conf_id <<'\n'
	         << "source_monitor_party_id	   =   "<< pReq->source_monitor_party_id <<",  "
	         << "target_monitor_party_id    =   "<< pReq->target_monitor_party_id<<'\n'
	         << "serviceName                =   ";
	if (m_service_provider)
		*pStrReq << (char*)pReq->serviceName << '\n';
	else
		*pStrReq << "NULL\n";

	PTRACE(eLevelInfoNormal, pStrReq->GetString());
	POBJDELETE(pStrReq);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)(pReq), sizeof(PARTY_MOVE_RSRC_REQ_PARAMS_S));
	SendReqToResourceAllocator(pSeg, START_PARTY_MOVE_RSRC_REQ);
// POBJDELETE(pSeg);
	PDELETE(pReq);

	StartTimer(EXPORT_RSRC_TOUT, 5*SECOND);
}

//--------------------------------------------------------------------------
void CPartyCntl::UpdateConfEndDisconnect(WORD status)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::UpdateConfEndDisconnect  ", m_partyConfName, GetPartyRsrcId());
}

// Start a timer to resend allocation request from the resource allocator,
// in this time the Resources Allocator tried to reconfigure DSP so he can allocate the requested resources.
// We will resend the allocation request just one time,  by setting the m_isWaitingForRsrcAndAskAgain to NO;
//--------------------------------------------------------------------------/
void CPartyCntl::WaitForRsrcAndAskAgain()
{
	if (1 == m_isWaitingForRsrcAndAskAgain)
	{
		m_isWaitingForRsrcAndAskAgain = 2;  // VNGFE-4083
		PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::WaitForRsrcAndAskAgain, Start Timer <<twice>> , Name - ", m_partyConfName, GetPartyRsrcId());
	}
	else
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CPartyCntl::WaitForRsrcAndAskAgain, Start Timer, Name - ", m_partyConfName, GetPartyRsrcId());
		m_isWaitingForRsrcAndAskAgain = 0;
	}

	BOOL        isJitcMode = NO;
	std::string key        = "ULTRA_SECURE_MODE";
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, isJitcMode);

	if (isJitcMode)
		StartTimer(WAIT_FOR_RSRC_AND_ASK_AGAIN, WAIT_FOR_RSRC_AND_ASK_AGAIN_TOUT_FIPS);
	else
		StartTimer(WAIT_FOR_RSRC_AND_ASK_AGAIN, WAIT_FOR_RSRC_AND_ASK_AGAIN_TOUT);
}

//--------------------------------------------------------------------------
void CPartyCntl::OnPartySetNodeType(CSegment* pSeg)
{
	DWORD len;
	*pSeg >> m_nodeType;
}

//--------------------------------------------------------------------------
BYTE CPartyCntl::GetConfAudioSampleRate()
{
	// Tsahi TODO: consider remove this function
	return AUDIO_SAMPLE_RATE_48KHZ;
}

//--------------------------------------------------------------------------
DWORD CPartyCntl::SendCreateParty(ENetworkType networkType, BYTE bIsMrcCall) const
{
	PASSERTMSG_AND_RETURN_VALUE(!m_pPartyHWInterface, "CPartyCntl::SendCreateParty failed - Null pointer operation (m_pPartyHWInterface)", 0);

	CConf* pConf = GetConf();
	PASSERTMSG_AND_RETURN_VALUE(!pConf, "CPartyCntl::SendCreateParty failed - Null pointer operation (pConf)", 0);

	const CCommConf* pCommConf = pConf->GetCommConf();
	PASSERTMSG_AND_RETURN_VALUE(!pCommConf, "CPartyCntl::SendCreateParty failed - Null pointer operation (pCommConf)", 0);

	unsigned short lineRate = 0;
	CCDRUtils::Get_Transfer_Rate_Command_BitRate(pCommConf->GetConfTransferRate(), &lineRate);
	PASSERTMSG_AND_RETURN_VALUE(lineRate == 0, "CPartyCntl::SendCreateParty failed - Invalid (lineRate)", 0);

	CSysConfig* pSysConfig            = CProcessBase::GetProcess()->GetSysConfig();
	DWORD       maxVideoTxBitsPer10ms = 64000;

	// According to VNGFE-3582
	// The value The value of this threshold should be kept as 64000 when the bit rate is lower than 4M (in CP), otherwise in order to avoid the stream
	// overwritten in higher bit rates (for VSW) there is a need to increase the traffic shaping threshold to 80000.
	pSysConfig->GetDWORDDataByKey((lineRate < 40960) ? "MAX_VIDEO_TX_BITS_PER_10_MILLI_SECONDS" : "MAX_HD_VIDEO_TX_BITS_PER_10_MILLI_SECONDS", maxVideoTxBitsPer10ms);

	return m_pPartyHWInterface->SendCreateParty(networkType, maxVideoTxBitsPer10ms, bIsMrcCall, pCommConf->GetConfMediaType(), pCommConf->GetEnableHighVideoResInAvcToSvcMixMode());
}

//--------------------------------------------------------------------------
// According to PARTICIPANT_MAX_USAGE_INFO event, need to update maxConnectionRate,maxResolution,maxFrameRate
void CPartyCntl::UpdateDetailsForParticipantDisconnectInfoCDREvent(CIpComMode* pScm)
{
	// Update maxConnectionRate:
	DWORD maxConnectionRateCurrently = GetConnectionRate(pScm);
	if (maxConnectionRateCurrently > m_maxConnectionRateCurrently)
	{
		// PTRACE2INT(eLevelInfoNormal,"CPartyCntl::UpdateDetailsForParticipantDisconnectInfoCDREvent - different connection rate. Rate - ",maxConnectionRateCurrently);
		m_maxConnectionRateCurrently = maxConnectionRateCurrently;
	}

	// Update maxResolution & maxFrameRate:
	SetMaxResolutionAndMaxFrameRate(pScm);
}

//--------------------------------------------------------------------------
// this function is for cdr event: PARTICIPANT_MAX_USAGE_INFO
DWORD CPartyCntl::GetConnectionRate(CIpComMode* pScm)
{
	DWORD connectionRate = 0;

	DWORD audioOut        = 0;
	DWORD audioIn         = 0;
	DWORD videoPeopleOut  = 0;
	DWORD videoPeopleIn   = 0;
	DWORD videoContentOut = 0;
	DWORD videoContentIn  = 0;
	if (!pScm->IsMediaOff(cmCapAudio, cmCapTransmit))
	{
		audioOut = pScm->GetMediaBitRate(cmCapAudio, cmCapTransmit);
	}

	if (!pScm->IsMediaOff(cmCapAudio, cmCapReceive))
	{
		audioIn = pScm->GetMediaBitRate(cmCapAudio, cmCapReceive);
	}

	if (!pScm->IsMediaOff(cmCapVideo, cmCapReceive, kRolePeople))
	{
		videoPeopleIn = (DWORD)((pScm->GetMediaBitRate(cmCapVideo, cmCapReceive, kRolePeople)) / 10);
	}

	if (!pScm->IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople))
	{
		videoPeopleOut = (DWORD)((pScm->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople))/10);
	}

	if (!pScm->IsMediaOff(cmCapVideo, cmCapReceive, kRoleContentOrPresentation))
	{
		videoContentIn = (DWORD)((pScm->GetMediaBitRate(cmCapVideo, cmCapReceive, kRoleContentOrPresentation)) /10);
	}

	if (!pScm->IsMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
	{
		videoContentOut = (DWORD)((pScm->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation)) /10);
	}

	connectionRate = max((audioOut + videoPeopleOut + videoContentOut), (audioIn + videoPeopleIn + videoContentIn));

	return connectionRate;
}
//--------------------------------------------------------------------------
// this function is for cdr event: PARTICIPANT_MAX_USAGE_INFO
void CPartyCntl::SetMaxResolutionAndMaxFrameRate(CIpComMode* pScm)
{
	CBaseCap* videoTx  = pScm->GetMediaAsCapClass(cmCapVideo, cmCapTransmit, kRolePeople);
	CBaseCap* videoRec = pScm->GetMediaAsCapClass(cmCapVideo, cmCapReceive, kRolePeople);

	WORD      frameRateTx;
	EFormat   formatTx;
	if (videoTx)
	{
		if (videoTx->GetCapCode() == eH264CapCode)
		{
			CH264VideoCap* pVideoTx = (CH264VideoCap*)videoTx;
			APIS32         fsTx     = pVideoTx->GetFs();

			if (fsTx == -1)
			{
				CH264Details details = pVideoTx->GetLevel();
				fsTx = details.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_FS_CODE);
				if(0 == fsTx)
					PTRACE2INT(eLevelInfoNormal, "CPartyCntl::SetMaxResolutionAndMaxFrameRate, level value in Video Tx is:", pVideoTx->GetLevel());
			}
			else
				fsTx = fsTx * CUSTOM_MAX_FS_FACTOR;

			if ((fsTx >= 99) && (fsTx < 660))
				formatTx = kCif;
			else if ((fsTx >= 660) && (fsTx < 3600))
				formatTx = k625SD;
			else if ((fsTx >= 3600) && (fsTx < 6336))
				formatTx = k720p;
			else if (fsTx >= 6336)
				formatTx = k1080p;
			else
				formatTx = kUnknownFormat;

			APIS32 mbpsTx = pVideoTx->GetMbps();
			if (mbpsTx == -1)
			{
				CH264Details details = pVideoTx->GetLevel();
				mbpsTx = details.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_MBPS_CODE);
			}
			else
				mbpsTx = mbpsTx * CUSTOM_MAX_MBPS_FACTOR;

			PASSERTMSG(fsTx == 0, "FrameSize TX equals to 0 - this is unexpected");
			if (fsTx)
				frameRateTx = mbpsTx/fsTx;
			else
				frameRateTx = mbpsTx/1;
		}
		else // H261,H263
		{
			formatTx    = videoTx->GetFormat();
			frameRateTx = videoTx->GetFrameRate(formatTx);
		}

		POBJDELETE(videoTx);
	}
	else
	{
		formatTx    = kCif;
		frameRateTx = 0;
	}

	WORD    frameRateRec;
	EFormat formatRec;
	if (videoRec)
	{
		if (videoRec->GetCapCode() == eH264CapCode)
		{
			CH264VideoCap* pVideoRec = (CH264VideoCap*)videoRec;
			APIS32         fsRec     = pVideoRec->GetFs();
			if (fsRec == -1)
			{
				CH264Details details = pVideoRec->GetLevel();
				fsRec = details.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_FS_CODE);
				if(0 == fsRec)
					PTRACE2INT(eLevelInfoNormal, "CPartyCntl::SetMaxResolutionAndMaxFrameRate, level value in Video Rec is:", pVideoRec->GetLevel());
			}
			else
				fsRec = fsRec * CUSTOM_MAX_FS_FACTOR;

			if ((fsRec >= 99) && (fsRec < 660))
				formatRec = kCif;
			else if ((fsRec >= 660) && (fsRec < 3600))
				formatRec = k625SD;
			else if ((fsRec >= 3600) && (fsRec < 6336))
				formatRec = k720p;
			else if (fsRec >= 6336)
				formatRec = k1080p;
			else
				formatRec = kUnknownFormat;

			APIS32 mbpsRec = pVideoRec->GetMbps();
			if (mbpsRec == -1)
			{
				CH264Details details = pVideoRec->GetLevel();
				mbpsRec = details.GetParamDefaultValueAsProductForLevel(CUSTOM_MAX_MBPS_CODE);
			}
			else
				mbpsRec = mbpsRec * CUSTOM_MAX_MBPS_FACTOR;

			PASSERTMSG(fsRec == 0, "FrameSize Rec equals to 0 - this is unexpected");

			if (fsRec)
				frameRateRec= mbpsRec/fsRec;
			else
				frameRateRec= mbpsRec/1;

		}
		else // H261,H263
		{
			formatRec    = videoRec->GetFormat();
			frameRateRec = videoRec->GetFrameRate(formatRec);
		}

		POBJDELETE(videoRec);
	}
	else
	{
		formatRec    = kCif;
		frameRateRec = 0;
	}

	EFormat maxFormat = max(formatRec, formatTx);

	WORD maxFrameRate = max(frameRateRec, frameRateTx);

	// Update maxResolution:
	if (maxFormat > m_maxFormatCurrently)
	{
		m_maxFormatCurrently = maxFormat;
	}

	// Update maxFrameRate:
	if (maxFrameRate > m_maxFrameRateCurrently)
	{
		m_maxFrameRateCurrently = maxFrameRate;
	}
}
/////////////////////////////////////////////////////////////////////////////////
void CPartyCntl::SetNewXCodeBridgeInterface(CVideoBridgeInterface* pXCodeBridgeInterface)
{
	m_pXCodeBridgeInterface = pXCodeBridgeInterface;
	TRACEINTO << "CPartyCntl::SetNewXCodeBridgeInterface << Turning on m_isXCodeConn, Party Name: " << GetFullName() << "\n";
	ON(m_isXCodeConn);
}

void CPartyCntl::SetNewXCodeBridgeInterfaceOff()
{
	m_pXCodeBridgeInterface = NULL;
	TRACEINTO << "CPartyCntl::SetNewXCodeBridgeInterfaceOff << Turning off m_isXCodeConn, Party Name: " << GetFullName() << "\n";
	OFF(m_isXCodeConn);
}

//--------------------------------------------------------------------------
void CPartyCntl::SetDataForImportPartyCntl(CPartyCntl* apOtherPartyCntl)
{
	CPartyCntl* pOtherPartyCntl = apOtherPartyCntl;

	m_pInfoSeg = pOtherPartyCntl->m_pInfoSeg; // moves the segment pointer to the new party
	pOtherPartyCntl->m_pInfoSeg = NULL;       // for not delete it twice.

	m_voice                = pOtherPartyCntl->m_voice;
	m_isFullBitRateConnect = pOtherPartyCntl->m_isFullBitRateConnect;
	m_videoProtocol        = pOtherPartyCntl->m_videoProtocol;
	m_isRemoteCapReady     = pOtherPartyCntl->m_isRemoteCapReady;
	m_isMultiRate          = pOtherPartyCntl->m_isMultiRate;
	m_sourceId             = pOtherPartyCntl->m_sourceId;
	m_subServiceId         = pOtherPartyCntl->m_subServiceId;

	POBJDELETE(m_pPartyAllocatedRsrc);
	m_pPartyAllocatedRsrc = new CPartyRsrcDesc(*(pOtherPartyCntl->m_pPartyAllocatedRsrc));
	m_pPartyAllocatedRsrc->SetConfRsrcId(pOtherPartyCntl->m_destResourceConfId);

	POBJDELETE(m_pBridgeMoveParams);
	m_pBridgeMoveParams = new CBridgeMoveParams(*(pOtherPartyCntl->m_pBridgeMoveParams));

	if (pOtherPartyCntl->m_service_provider != NULL)
	{
		PDELETEA(m_service_provider);
		m_service_provider = new char[NET_SERVICE_PROVIDER_NAME_LEN];
		strcpy_safe(m_service_provider, NET_SERVICE_PROVIDER_NAME_LEN, pOtherPartyCntl->m_service_provider);
	}
	else
		m_service_provider = NULL;

	m_serviceId      = pOtherPartyCntl->m_serviceId;
	m_pParty         = pOtherPartyCntl->m_pParty;
	m_numChnl        = pOtherPartyCntl->m_numChnl;
	m_chnlWidth      = pOtherPartyCntl->m_chnlWidth;
	m_type           = pOtherPartyCntl->m_type;
	m_monitorPartyId = pOtherPartyCntl->m_monitorPartyId;
	m_monitorConfId  = pOtherPartyCntl->m_monitorConfId;
	m_moveType       = pOtherPartyCntl->m_moveType;

	PDELETEA(m_name);
	m_name = new char[H243_NAME_LEN];
	strcpy_safe(m_name, H243_NAME_LEN, pOtherPartyCntl->m_name);

	CPartyApi* pPartyCntl = pOtherPartyCntl->m_pPartyApi;
	PASSERT_AND_RETURN(!pPartyCntl);

	if (m_pPartyApi)
	{
		m_pPartyApi->Destroy();
		POBJDELETE(m_pPartyApi);
	}
	m_pPartyApi = new CPartyApi;
	m_pPartyApi->CreateOnlyApi(pPartyCntl->GetRcvMbx());

	strcpy_safe(m_productId, pOtherPartyCntl->m_productId);
	strcpy_safe(m_VersionId, pOtherPartyCntl->m_VersionId);

	m_interfaceType      = pOtherPartyCntl->m_interfaceType;
	m_isUndefParty       = pOtherPartyCntl->m_isUndefParty;
	m_isRemoteDBC2       = pOtherPartyCntl->m_isRemoteDBC2;
	m_monitorConfId      = pOtherPartyCntl->m_destMonitorConfId;
	m_monitorPartyId     = pOtherPartyCntl->m_destMonitorPartyId;
	m_destResourceConfId = pOtherPartyCntl->m_destResourceConfId;
	m_bIsMrcCall         = pOtherPartyCntl->m_bIsMrcCall;
	m_bIsWebRtcCall      = pOtherPartyCntl->m_bIsWebRtcCall;
    m_deferUpgrade = pOtherPartyCntl->m_deferUpgrade;
    m_bPartyInUpgradeProcess =pOtherPartyCntl->m_bPartyInUpgradeProcess;

    m_reallocatePartyParams = pOtherPartyCntl->m_reallocatePartyParams;
    m_reAllocRtm = pOtherPartyCntl->m_reAllocRtm;
    m_isReallocSyncMessage = pOtherPartyCntl->m_isReallocSyncMessage;
    m_reallocOpcode = pOtherPartyCntl->m_reallocOpcode;


}
//////////////////////////////////////////////////
void CPartyCntl::OnConfSetPartyAvcSvcModeAnycase(CSegment* pParam)
{
	DWORD MointorpartyId = (DWORD)(-1);
	DWORD confID = (DWORD)(-1);
	eConfMediaState confMediaState = eMediaStateEmpty;
	DWORD tmp=(DWORD)confMediaState;
	*pParam >> tmp  >> confID  >> MointorpartyId;

	TRACEINTO << "conference type: " << MediaStateToString((eConfMediaState)tmp ) << " conference id: " << confID << " monitor party id: " << MointorpartyId << " m_state: " << m_state;


	TRACEINTO<<"!@# upgrade transaction should be deferred";
	m_deferUpgrade=true;
	m_bPartyInUpgradeProcess = false;
	// todo:  future use for Eyal and Keren

}


//////////////////////////////////////////////////
void CPartyCntl::SetPartyAvcSvcMode(eConfMediaState confMediaState, DWORD confID, DWORD MonitorPartyId)
{
	TRACEINTO << "!@# conference type: "<< MediaStateToString(confMediaState) << " conference id: "<< confID << " Monitor party id: "<< MonitorPartyId<<" m_state:"<<m_state;;
	//m_pAudioInterface->
	//m_pVideoBridgeInterface;
	// todo:  future use for Eyal and Keren

	// test
	if (!m_bIsMrcCall)
	{
		CAvcToSvcParams *pAvcToSvcParams = new CAvcToSvcParams();	// need to fill all parameters....
		pAvcToSvcParams->SetIsSupportAvcSvcTranslate( true );
		m_pVideoBridgeInterface->UpgradeToMixAvcSvcForNonRelayParty( GetPartyRsrcId(), pAvcToSvcParams );
		POBJDELETE( pAvcToSvcParams );
	}
	else
	{
		m_pVideoBridgeInterface->UpgradeToMixAvcSvcForRelayParty( GetPartyRsrcId() );
	}
}

//////////////////////////////////////////////////
void CPartyCntl::UpdatePartyAvcSvcMode(eConfMediaState confState,DWORD confID, DWORD MointorPartyId)
{
	if (m_bPartyInUpgradeProcess==true)
	{
		if (m_bIsMrcCall)
		{
			TRACEINTO<<"dynmixedErr svc party has already been upgraded";
		}
		else
		{
			TRACEINTO<<"dynMixedErr avc party has already been upgraded";
		}

		return;
	}

	m_bPartyInUpgradeProcess=true;

	if (m_bIsMrcCall)
	{
		TRACEINTO << "dynMixedPosAck svc party: commence upgrade: conference type: "
			<< MediaStateToString(confState) << " conference id: " << confID
			<< " monitor party id: " << MointorPartyId;
	}
	else
	{
		TRACEINTO << "dynMixedPosAck avc party: commence upgrade: conference type: "
			<< MediaStateToString(confState) << " conference id: " << confID
			<< " monitor party id: " << MointorPartyId;
	}

	CSegment* pSeg = new CSegment;

	*pSeg << (DWORD)confState << confID << MointorPartyId;
	DispatchEvent(SET_PARTY_AVC_SVC_MEDIA_STATE,pSeg);
	POBJDELETE(pSeg);
}
//////////////////////////////////////////////////
bool CPartyCntl::ClearUnusedPartyId(DWORD rsrcPartyId)
{
  bool rVal = false;
  CLookupIdParty* pLookupIdParty = GetLookupIdParty();
  CLookupTableParty* pLookupTableParty = GetLookupTableParty();
  if(pLookupTableParty && pLookupIdParty){
      pLookupIdParty->Clear(rsrcPartyId);
      TRACEINTO << " Clear rsrcPartyId = " << rsrcPartyId;
      rVal = true;
  }
  return rVal;

}

//////////////////////////////////////////////////
void CPartyCntl::OnPartyChangeBridgesMuteState(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CPartyCntl::OnPartyChangeBridgesMuteState: Name - ",m_partyConfName, GetPartyRsrcId());

	BYTE bMuteAudioIn;
	BYTE bMuteAudioOut;
	BYTE bMuteVideoIn;
	BYTE bMuteVideoOut;
	BYTE bMuteContentIn;
	BYTE bMuteContentOut;
	BYTE bMuteFeccIn;
	BYTE bMuteFeccOut;

	*pParam >> bMuteAudioIn
			>> bMuteAudioOut
			>> bMuteVideoIn
			>> bMuteVideoOut
			>> bMuteContentIn
			>> bMuteContentOut
			>> bMuteFeccIn
			>> bMuteFeccOut;

	TRACEINTO << "bMuteAudioIn: " << (DWORD)bMuteAudioIn <<", bMuteAudioOut: " << (DWORD)bMuteAudioOut
			  << ", bMuteVideoIn: " << (DWORD)bMuteVideoIn << ", bMuteVideoOut: "<<(DWORD)bMuteVideoOut
			  << ", bMuteContentIn: " << (DWORD)bMuteContentIn << ", bMuteContentOut: " <<(DWORD)bMuteContentOut
			  << ", bMuteContentOut: " << (DWORD)bMuteFeccIn << ", bMuteFeccOut: " << (DWORD)bMuteFeccOut;

	if (IsInDirectionConnectedToAudioBridge())
	{
		if (bMuteAudioIn != AUTO)
			m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaIn, (EOnOff)bMuteAudioIn, PARTY);
	}

	if (IsOutDirectionConnectedToAudioBridge())
	{
		if(bMuteAudioOut != AUTO)
			m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaOut, (EOnOff)bMuteAudioOut, PARTY);
	}

	if (IsInDirectionConnectedToVideoBridge())
	{
		if (bMuteVideoIn != AUTO)
			m_pVideoBridgeInterface->UpdateMute(GetPartyRsrcId(), (EOnOff)bMuteVideoIn, PARTY);
	}

	if (IsOutDirectionConnectedToVideoBridge())
	{
		if (bMuteVideoOut != AUTO ) //only do this if it is not sip
			m_pVideoBridgeInterface->UpdateMute(GetPartyRsrcId(), (EOnOff)bMuteVideoOut, PARTY);

	}

	if (m_isFeccConn)
	{
		if (bMuteFeccIn != AUTO || bMuteFeccOut != AUTO)
			PTRACEPARTYID(eLevelInfoNormal, "Mute Fecc - not implemented", GetPartyRsrcId());
	}
}

