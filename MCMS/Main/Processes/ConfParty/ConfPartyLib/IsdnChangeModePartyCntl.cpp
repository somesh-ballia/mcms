
#include "IsdnChangeModePartyCntl.h"
#include "ConfPartyOpcodes.h"
#include "Conf.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "H323Caps.h"
#include "ConfPartyGlobals.h"
#include "VideoBridgeInterface.h"
#include "AudioBridgeInterface.h"
#include "ContentBridge.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "TraceStream.h"
#include "H264Util.h"
#include "BridgeMoveParams.h"
#include "H320ComMode.h"
#include "CDRUtils.h"

/*
	ISDN_CHANGEMODE_CNTL STATES:
	---------------------------

  IDLE --> CHANGECONTENT --> CHANGEVIDEO

*/
const WORD REMOTE_XMIT_MODE = RMTXFRMODE;
const WORD REMOTE_BAS_CAPS  = RMTCAP;

#define ISDN_PARTYCNTL_CHANGE_TIME      CHANGERATE_TIME
#define ISDN_PARTYCNTL_CHANGEVIDEO_TIME 10*SECOND

using namespace std;

PBEGIN_MESSAGE_MAP(CIsdnChangeModeCntl)
  // conf events
  ONEVENT(SCMCHANGEMODE,                   IDLE,               CIsdnChangeModeCntl::OnConfChangeModeIdle)
  ONEVENT(SCMCHANGEMODE,                   CHANGEVIDEO,        CIsdnChangeModeCntl::OnConfChangeModeChangeVideo)
  ONEVENT(SCMCHANGEMODE,                   CHANGECONTENT,      CIsdnChangeModeCntl::OnConfChangeModeChangeContent)

  // brdg events
  ONEVENT(PARTY_VIDEO_CONNECTED,           IDLE,               CIsdnChangeModeCntl::OnVidBrdgConVideoIdle)
  ONEVENT(PARTY_VIDEO_CONNECTED,           CHANGEVIDEO,        CIsdnChangeModeCntl::OnVidBrdgConChangeVideo)
  ONEVENT(PARTY_VIDEO_CONNECTED,           CHANGECONTENT,      CIsdnChangeModeCntl::OnVidBrdgConChangeContent)

  ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,  IDLE,               CIsdnChangeModeCntl::NullActionFunction)
  ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,  CHANGEVIDEO,        CIsdnChangeModeCntl::OnVidBrdgConChangeVideo)
  ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,  CHANGECONTENT,      CIsdnChangeModeCntl::OnVidBrdgConChangeContent)

  ONEVENT(PARTY_VIDEO_IN_UPDATED,          CHANGEVIDEO,        CIsdnChangeModeCntl::OnVideoInBrdgUpdatedChangeVideo)
  ONEVENT(PARTY_VIDEO_IN_UPDATED,          ANYCASE,            CIsdnChangeModeCntl::OnVideoInBrdgUpdatedAnycase)
  ONEVENT(PARTY_VIDEO_OUT_UPDATED,         CHANGEVIDEO,        CIsdnChangeModeCntl::OnVideoOutBrdgUpdatedChangeVideo)
  ONEVENT(PARTY_VIDEO_OUT_UPDATED,         ANYCASE,            CIsdnChangeModeCntl::OnVideoOutBrdgUpdatedAnycase)

  ONEVENT(PARTY_VIDEO_DISCONNECTED,        ANYCASE,            CIsdnChangeModeCntl::OnVidBrdgDisconnectAnycase)

  ONEVENT(PARTY_CONTENT_CONNECTED,         CHANGECONTENT,      CIsdnChangeModeCntl::OnContentBrdgConnectedChangeContent)
  ONEVENT(PARTY_CONTENT_CONNECTED,         ANYCASE,            CIsdnChangeModeCntl::OnContentBrdgConnectedAnycase)
  ONEVENT(PARTY_CONTENT_DISCONNECTED,      ANYCASE,            CIsdnChangeModeCntl::OnContentBrdgDisconnected)
  ONEVENT(PARTY_XCODE_DISCONNECTED,        ANYCASE,            CIsdnChangeModeCntl::OnXCodeBrdgDisconnected)

  ONEVENT(PARTYENDCHANGEMODE,              CHANGEVIDEO,        CIsdnChangeModeCntl::OnPartyEndChangeVideo)
  ONEVENT(PARTYENDCHANGEMODE,              CHANGECONTENT,      CIsdnChangeModeCntl::OnPartyEndChangeContent)

  ONEVENT(REMOTE_XMIT_MODE,                CHANGECONTENT,      CIsdnChangeModeCntl::OnRmtXmitModeChangeContent)
  ONEVENT(REMOTE_XMIT_MODE,                ANYCASE,            CIsdnChangeModeCntl::OnRmtXmitModeAnycase)

  ONEVENT(REMOTE_BAS_CAPS,                 IDLE,               CIsdnChangeModeCntl::OnRmtCapsIdle)
  ONEVENT(REMOTE_BAS_CAPS,                 CHANGECONTENT,      CIsdnChangeModeCntl::OnRmtCapsChangeContent)
  ONEVENT(REMOTE_BAS_CAPS,                 CHANGEVIDEO,        CIsdnChangeModeCntl::OnRmtCapsChangeVideo)

  ONEVENT(VIDREFRESH,                      ANYCASE,            CIsdnChangeModeCntl::OnPartyRefreshVideoAnycase)

  ONEVENT(CHANGETOUT,                      CHANGECONTENT,      CIsdnChangeModeCntl::OnTimerChangeContent)
  ONEVENT(CHANGETOUT,                      CHANGEVIDEO,        CIsdnChangeModeCntl::OnTimerChangeVideo)

  ONEVENT(VIDEO_UPDATE_TIMEOUT,            IDLE,               CIsdnChangeModeCntl::OnTimerVideoUpdate)

  ONEVENT(PRESENTATION_OUT_STREAM_UPDATED, IDLE,               CIsdnChangeModeCntl::OnPartyPresentationOutStreamUpdatedIdle)
  ONEVENT(PRESENTATION_OUT_STREAM_UPDATED, CHANGEVIDEO,        CIsdnChangeModeCntl::OnPartyPresentationOutStreamUpdatedCHANGEVIDEO)
  ONEVENT(PRESENTATION_OUT_STREAM_UPDATED, CHANGECONTENT,      CIsdnChangeModeCntl::OnPartyPresentationOutStreamUpdatedCHANGECONTENT)
  ONEVENT(PRESENTATION_OUT_STREAM_UPDATED, ANYCASE,            CIsdnChangeModeCntl::OnPartyPresentationOutStreamUpdated)

  ONEVENT(PARTY_IN_CONF_IND,               ANYCASE,            CIsdnChangeModeCntl::OnCAMUpdatePartyInConf)

  ONEVENT(WAIT_BEFORE_CONNECT_TO_VB_TIMER, ANYCASE,            CIsdnChangeModeCntl::OnTimerConnectPartyToVideoBridge)

PEND_MESSAGE_MAP(CIsdnChangeModeCntl,CIsdnPartyCntl);


////////////////////////////////////////////////////////////////////////////
//                        CIsdnChangeModeCntl
////////////////////////////////////////////////////////////////////////////
CIsdnChangeModeCntl::CIsdnChangeModeCntl()
{
	m_hmlpChangeModeRequest    = 0;
	m_mlpConnectRequest        = 0;
	m_lsdChangeModeRequest     = 0;
	m_hsdChangeModeRequest     = 0;
	m_mlpChangeModeRequest     = 0;
	m_hmlpChangeModeRequest    = 0;
	m_isSentH239Out            = 0;
	m_needToCreateNewLocalCaps = 0;
	m_waitForAckFromVideoOut   = 0;
	m_waitForAckFromVideoIn    = 0;
	m_needToChangeVideoIn      = 0;
	m_myTest                   = 1;

	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIsdnChangeModeCntl& CIsdnChangeModeCntl::operator =(const CIsdnPartyCntl& other)
{
	(CIsdnPartyCntl&)*this = (CIsdnPartyCntl&)other;
	return *this;
}

//--------------------------------------------------------------------------
CIsdnChangeModeCntl& CIsdnChangeModeCntl::operator=(const CIsdnChangeModeCntl& other)
{
	m_hmlpChangeModeRequest    = other.m_hmlpChangeModeRequest;
	m_mlpConnectRequest        = other.m_mlpConnectRequest;
	// For FECC - currently not in use
	m_lsdChangeModeRequest     = other.m_lsdChangeModeRequest;
	m_hsdChangeModeRequest     = other.m_hsdChangeModeRequest;
	m_mlpChangeModeRequest     = other.m_mlpChangeModeRequest;
	m_hmlpChangeModeRequest    = other.m_hmlpChangeModeRequest;
	m_isSentH239Out            = other.m_isSentH239Out;

	m_needToCreateNewLocalCaps = other.m_needToCreateNewLocalCaps;
	m_waitForAckFromVideoOut   = other.m_waitForAckFromVideoOut;
	m_waitForAckFromVideoIn    = other.m_waitForAckFromVideoIn;
	m_needToChangeVideoIn      = other.m_needToChangeVideoIn;

	(CIsdnPartyCntl&)*this     = (CIsdnPartyCntl&)other;
	return *this;
}

// EntryPoint for ChangeMode procedure  from conf\party(after re-cap)
// 1) update data members with new scm
// 2) dispatch change mode event if needed
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::ChangeScm(CComMode* pTargetTransmitScm, CComMode* pTargetReceiveScm)
{
	BYTE current_encryption = (m_pLocalCap->IsEncrypCapOn(Encryp_Cap))? Encryp_On : Encryp_Off;
	CCapECS* curr_ECS = m_pLocalCap->GetCapECS();

	if (pTargetReceiveScm)
	{
		// m_pTargetReceiveScm should contain scm that equivalent to local caps that were sent to party
		// Eitan To Do: implement operator !=
		if (pTargetReceiveScm->m_vidMode != m_pTargetReceiveScm->m_vidMode)
		{
			TRACEINTO << m_partyConfName << " - Target SCM for Receive not equal to current, generate new caps from new SCM and send to party";
			// raise a flag that will indicate us to send re-cap to party with new video caps
			ON(m_needToCreateNewLocalCaps);
			*m_pTargetReceiveScm = *pTargetReceiveScm;
		}
		else
		{
			TRACEINTO << m_partyConfName << " - Target SCM for Receive equal to current";
		}

		// VNGBE-360 can't connect concorde EP ( it can't receive long cap set)
		// VNGR-10975 - don't send recap with only h261 to EP's that support h239 (if there would be complains from old EP's that support H.239 - we should remove it)
		if (CASCADE_MODE_MCU != GetNodeType() && m_pRemoteCap->IsCapableOfVideo() && !m_pRemoteCap->IsH263() && !m_pRemoteCap->IsH264() && m_pCurrentReceiveScm->GetVidMode() == Video_Off && !m_pRemoteCap->IsH239Cap())
		{
			TRACEINTO << m_partyConfName << " - Remote support only H.261(Probably old EP), set local caps to H.261 and send re-cap";

			CCapH320 TargetCap;

			// TargetCap
			OFF(m_needToCreateNewLocalCaps);
			WORD newAudioMode = m_pCurrentReceiveScm->GetAudMode();
			switch (newAudioMode)
			{
				case (Au_Siren14_48k):
				{
					TargetCap.SetNScapSiren1448();
					break;
				}

				case (Au_Siren14_32k):
				{
					TargetCap.SetNScapSiren1432();
					break;
				}

				case (Au_Siren14_24k):
				{
					TargetCap.SetNScapSiren1424();
					break;
				}

				default:
				{
					EAudioCapAlgorithm selectedAudioAlg = TranslateAudioAlgScmToCaps(newAudioMode);
					if (selectedAudioAlg != e_Neutral)
						TargetCap.SetAudioCap(selectedAudioAlg);
					break;
				}
			} // switch

			TargetCap.SetH261Caps(V_Cif, V_1_29_97, V_1_29_97);

			*m_pLocalCap = TargetCap;
			if(current_encryption == Encryp_Cap)
			{
				CSegment dummySeg;
				m_pLocalCap->HandleBas(DATAVIDCAPATTR | Encryp_Cap, dummySeg);
				if (IsValidPObjectPtr(curr_ECS))
					m_pLocalCap->CreateCCapECS(curr_ECS);
			}
			else // If in scm Encryption is set to OFF - Remove Encryption Caps - Bug#17918 - Talya
			{
				m_pLocalCap->RemoveEncCaps();
			}
			m_pLocalCap->Dump();
			m_pPartyApi->ExchangeCap(m_pLocalCap);
		}

		m_pTargetReceiveScm->m_contentMode = pTargetReceiveScm->m_contentMode;
		m_pTargetReceiveScm->SetOtherEncrypMode(current_encryption);
	}
	else
	{
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - No Target SCM for Receive";
	}

	if (pTargetTransmitScm)
	{
		// intersect between remoteVideoMode & localVideoMode
		// put the result in pTargetTransmitScm

		TRACEINTO << m_partyConfName << " - Target SCM for Transmit mode before update:";
		pTargetTransmitScm->m_vidMode.Dump();
		pTargetTransmitScm->UpdateH221string(IsPartyH239());
		pTargetTransmitScm->Dump(1);

		CVidMode ConfVideoMode  = m_pConfScm->m_vidMode;
		WORD     IsAutoVidScm   = ConfVideoMode.IsAutoVidScm();
		CVidMode LocalVideoMode = pTargetTransmitScm->m_vidMode;
		CVidMode IntersectVidMode;
		BOOL     toPrefferH2634Cif15 = FALSE;
		toPrefferH2634Cif15 = IsH2634CifPreffered(pTargetTransmitScm);
		if (toPrefferH2634Cif15)
		{
			TRACEINTO << m_partyConfName << " - Prefer H2634CIF over H264";
			pTargetTransmitScm->m_vidMode.SetVidMode(H263);
			LocalVideoMode = pTargetTransmitScm->m_vidMode;

			// in case we want to transmit H.263 and receive H.264 we should send to remote Cancel_MMS
			m_pPartyApi->SendMMStoParty(0);
		}

		eSystemCardsMode systemCardsMode = GetSystemCardsBasedMode();
		// In case of HD720 asymmetric mode, (target transmit is update with HD720) we don't support intermediate modes between SD30 and HD720
		// we need to see if remote supports at least HD720 15fps, if not we shall do the intersect between the target receive(and not transmit) scm and the remote cap and update the target transmit
		if ((systemCardsMode == eSystemCardsMode_mpm) && LocalVideoMode.IsCapableOfHD720_15fps() && (!m_pRemoteCap->IsCapableOfHD720_15()))
		{
			TRACEINTO << m_partyConfName << " - Remote doesn't supports HD720 with at least 15fps";
			CVidMode TargetReceiveVideoMode = m_pTargetReceiveScm->m_vidMode;
			TargetReceiveVideoMode.Intersect(m_pRemoteCap, IsAutoVidScm, IntersectVidMode);
			m_pTargetTransmitScm->SetHd720Enabled(FALSE);
		}
		// In case of HD1080 asymmetric mode, (target transmit is update with HD1080) we don't support intermediate modes between HD720 and HD1080
		// we need to see if remote supports at least HD1080 15fps, if not we shall do the intersect between the target receive(and not transmit) scm and the remote cap and update the target transmit
		else if (LocalVideoMode.IsCapableOfHD1080_15fps() && (!m_pRemoteCap->IsCapableOfHD1080_15()))
		{
			TRACEINTO << m_partyConfName << " - Remote doesn't supports HD1080 with at least 15fps";
			CVidMode TargetReceiveVideoMode = m_pTargetReceiveScm->m_vidMode;
			TargetReceiveVideoMode.Intersect(m_pRemoteCap, IsAutoVidScm, IntersectVidMode);
			m_pTargetTransmitScm->SetHd1080Enabled(FALSE);
		}

		// In case of HD720 60 asymmetric mode, (target transmit is update with HD720At60) we don't support intermediate modes between SD60 and HD72060
		// we need to see if remote supports at least HD720 50fps, if not we shall do the intersect between the target receive(and not transmit) scm and the remote cap and update the target transmit
		if (LocalVideoMode.IsCapableOfHD720_50fps() && (!m_pRemoteCap->IsCapableOfHD720_50()))
		{
			TRACEINTO << m_partyConfName << " - Remote doesn't supports HD720 with at least 50fps";
			CVidMode TargetReceiveVideoMode = m_pTargetReceiveScm->m_vidMode;
			TargetReceiveVideoMode.Intersect(m_pRemoteCap, IsAutoVidScm, IntersectVidMode);
			m_pTargetTransmitScm->SetHd720At60Enabled(FALSE);
		}
		else
		{
			LocalVideoMode.Intersect(m_pRemoteCap, IsAutoVidScm, IntersectVidMode);
		}

		pTargetTransmitScm->SetVidMode(IntersectVidMode);

		*m_pTargetTransmitScm = *pTargetTransmitScm;
		m_pTargetTransmitScm->SetOtherEncrypMode(current_encryption);
		m_pTargetTransmitScm->UpdateH221string(IsPartyH239());
		TRACEINTO << m_partyConfName << " - Target SCM for Transmit mode after intersect with remote caps:";
		m_pTargetTransmitScm->m_vidMode.Dump();
		m_pTargetTransmitScm->Dump(1);
		DWORD tmp = VideoCanBeOpened(m_pTargetTransmitScm);
	}
	{
		TRACESTRFUNC(eLevelError) << m_partyConfName << " - No Target SCM for Transmit";
	}

	DispatchChangeModeEvent();
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnConfChangeModeIdle(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	PASSERT(!m_pTargetTransmitScm);
	PASSERT(!m_pTargetReceiveScm);

	if (m_voice)
	{
		// voice party should not be secondary, if their are special cases we will handle them seperatly
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTED);
		if ((m_pTargetTransmitScm) && (m_pCurrentTransmitScm))
			*m_pTargetTransmitScm = *m_pCurrentTransmitScm; // in order to end the change mode

		EndChangeMode();
		return;
	}

	if (!AreRemoteCapsEnableChangeMode())
	{
		SetPartyToSecondaryAndEndChangeMode();
	}
	else
	{
		if (m_pTargetTransmitScm->GetOtherEncrypMode() == Encryp_On)
		{
			TRACEINTO << m_partyConfName << " - Update m_pCurrentTransmitScm Encryp_On [enc_trace]";
			m_pCurrentTransmitScm->m_otherMode.SetEncrypMode(Encryp_On);
		}

		if (!(m_pTargetTransmitScm->m_contentMode == m_pCurrentTransmitScm->m_contentMode) && IsRemoteContentFailed() == FALSE)
		{
			ePresentationProtocol contentProtocolMode = (ePresentationProtocol)(m_pConf->GetCommConf()->GetPresentationProtocol());
			// if party without EPC caps is in EPC conference
			if (m_pConfScm->IsContentOn() && (!m_pRemoteCap->IsH239Cap()))
			{
				// In VSW conf party will be Secondary
				if (!(m_pConfScm->IsFreeVideoRate()))
				{
					TRACEINTO << m_partyConfName << " - Remote without 239 caps in VSW - SECONDARY";
					SetPartyToSecondaryAndEndChangeMode();
					return;
				}
				else
				{
					// In TR/CP switch off content mode
					m_pTargetTransmitScm->m_contentMode = m_pCurrentTransmitScm->m_contentMode;
				}
			}
			else if (contentProtocolMode == eH264Fix || contentProtocolMode == eH264Dynamic)
			{
				PTRACE2(eLevelInfoNormal, "CIsdnChangeModeCntl::OnConfChangeModeIdle : Conference is in H264 Content mode - SECONDARY content - close content, ", m_partyConfName);
				CContentMode NoContentMode;
				m_pTargetTransmitScm->SetContentMode(NoContentMode);
				ChangeContent();
				return;
			}
			else
			{
				ChangeContent();
				if (m_pCurrentTransmitScm->m_contentMode == m_pCurrentReceiveScm->m_contentMode)
				{
					TRACEINTO << m_partyConfName << " - Party content is already open (don't wait for remote_xmit)";
					OnPartyEndChangeContent((CSegment*)NULL);
				}

				return;
			}
		}

		// party capabilities enable mode change
		// SelectT120Rate(); // change T120 rate in C.P. if necessary
		// we always call ChangeVideo() after move in order to reconnect to the video bridge
		BYTE bVideoOutDiff = NO;
		bVideoOutDiff = (m_pTargetTransmitScm->m_vidMode != m_pCurrentTransmitScm->m_vidMode);
		if (bVideoOutDiff == FALSE)
		{
			bVideoOutDiff =  (m_pTargetTransmitScm->GetMediaBitrate(CComMode::eMediaTypeVideo) != m_pCurrentTransmitScm->GetMediaBitrate(CComMode::eMediaTypeVideo));
			TRACEINTO << m_partyConfName << " - Change Video needed, video rates are different";
		}

		if (bVideoOutDiff || m_IsMovedParty)
			ChangeVideo();
		else
			TRACEINTO << m_partyConfName << " - Change Video Is not needed, video modes are the same (target == current)";
	}
}
// the new scm will be recorded at m_pTargetTransmitScm.
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnConfChangeModeChangeVideo(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Do nothing";
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::ChangeVideo()
{
	TRACEINTO << m_partyConfName;

	WORD isModeChangeRequest;
	OFF(isModeChangeRequest);

	BYTE changeVideoIn       = (m_pCurrentReceiveScm->VideoOn() && m_pTargetReceiveScm->VideoOn()) ? TRUE : FALSE;
	BYTE changeVideoOut      = (m_pTargetTransmitScm->VideoOn() || m_pCurrentTransmitScm->VideoOn()) ? TRUE : FALSE;
	BYTE changeVideoInAndOut = (/*(changeVideoIn && changeVideoOut) || */ m_IsMovedParty) ? TRUE : FALSE;

	if (!isModeChangeRequest) // no other mode change
	{
		if (!VideoCanBeOpened(m_pTargetTransmitScm))
			m_pTargetTransmitScm->m_vidMode.SetVidMode(Video_Off);

		if (changeVideoInAndOut == TRUE)
		{
			TRACEINTO << m_partyConfName << " - Establishing Video In and Out Connection...";
			StartTimer(CHANGETOUT, ISDN_PARTYCNTL_CHANGEVIDEO_TIME);
			m_state = CHANGEVIDEO;
			UpdateCurrComMode(m_pTargetTransmitScm);
			m_pPartyApi->ChangeMode(m_pTargetTransmitScm);

			m_waitForAckFromVideoOut = TRUE;
			m_waitForAckFromVideoIn  = TRUE;

			ConnectPartyToVideoBridge(eSendOpenInAndOut);

			if (m_IsMovedParty)
				m_IsMovedParty = FALSE;
		}
		else
		{
			if (changeVideoOut == TRUE)
			{
				// Send open/update video out
				m_waitForAckFromVideoOut = ChangeVideoBridgeVideoOutState();  // true in case of connect / disconnect / update video bridge

				if (m_waitForAckFromVideoOut)
				{
					StartTimer(CHANGETOUT, ISDN_PARTYCNTL_CHANGEVIDEO_TIME);
					m_state = CHANGEVIDEO;
					UpdateCurrComMode(m_pTargetTransmitScm);
					m_pPartyApi->ChangeMode(m_pTargetTransmitScm);
				}
			}
			else // video is not to be connected
			{
				TRACEINTO << m_partyConfName << " - No video to be opened";
				SetPartyToSecondaryAndEndChangeMode();
			}

			if (changeVideoIn == TRUE)
			{
				TRACEINTO << m_partyConfName << " - Remote video is open, connect to video bridge-in";
				m_waitForAckFromVideoIn = ChangeVideoBridgeVideoInState();    // true in case of connect / disconnect / update video bridge
			}
		}
	}

	// we should set m_pLocalCap according to m_pTargetReceiveScm
	// and send to party re-cap with new cap
	if (m_needToCreateNewLocalCaps)
	{
		CCapH320 TargetCap;
		TargetCap.Create(*m_pTargetReceiveScm);
		TRACEINTO << m_partyConfName << " - Send re-cap to party, new cap is:";
		TargetCap.Dump();

		*m_pLocalCap = TargetCap;
		m_pPartyApi->ExchangeCap(m_pLocalCap);
		OFF(m_needToCreateNewLocalCaps);
	}
}

// returned value: wait for bridge indication (yes/no)
//--------------------------------------------------------------------------
BYTE CIsdnChangeModeCntl::ChangeVideoBridgeVideoOutState()
{
	// (1) If the party isn't connected to the video bridge but has video:
	if ((!(m_eVidBridgeConnState & eSendOpenOut)) && m_pTargetTransmitScm->VideoOn()) // party connected with video
	{
		if (m_isFaulty)
		{
			TRACESTRFUNC(eLevelError) << m_partyConfName << " - Can't connect to video bridge, because of faulty flag";
			return FALSE;
		}
		else
		{
			TRACEINTO << m_partyConfName << " - Establishing Video Out Connection...";
			m_state = CHANGEVIDEO;
			ConnectPartyToVideoBridge(eSendOpenOut, 200);
			return TRUE;
		}
	}

	// (2) If the party is connected to the video bridge
	if (m_eVidBridgeConnState & eSendOpenOut)
	{
		// Set the party to secondary as a result of a new scm, after it has already been connected
		if (m_pCurrentTransmitScm->GetVidMode() == Video_Off)
		{
			TRACEINTO << m_partyConfName << " - Disconnect from the video bridge";
			BYTE bIsDisconnectFromVideoBridge = DisconnectPartyFromVideoOut();
			return bIsDisconnectFromVideoBridge;
		}

		// (3) update bridge
		if (UpdateVideoOutBridgeIfNeeded())
		{
			m_eUpdateState = eUpdateVideoOut;
			return TRUE;   // need to wait
		}
	}

	return FALSE;
}

// returned value: wait for bridge indication (yes/no)
//--------------------------------------------------------------------------
BYTE CIsdnChangeModeCntl::ChangeVideoBridgeVideoInState()
{
	// (1) If the party isn't connected to the video bridge but has video:
	if ((!(m_eVidBridgeConnState & eSendOpenIn)) && m_pTargetReceiveScm->VideoOn()) // party connected with video
	{
		if (m_isFaulty)
		{
			TRACESTRFUNC(eLevelError) << m_partyConfName << " - Can't connect to video bridge, because of faulty flag";
			return FALSE;
		}
		else
		{
			TRACEINTO << m_partyConfName << " - Establishing Video In Connection...";
			// m_state = CHANGEVIDEO;
			ConnectPartyToVideoBridge(eSendOpenIn);
			return TRUE;
		}
	}

	// (2) If the party is connected to the video bridge
	if (m_eVidBridgeConnState & eSendOpenIn)
	{
		// Set the party to secondary as a result of a new scm, after it has already been connected
		if (m_pCurrentReceiveScm->GetVidMode() == Video_Off)
		{
			TRACEINTO << m_partyConfName << " - Remote video mode is Video_Off - IGNORE!";
			return FALSE;
		}
		// (3) update bridge
		if (UpdateVideoInBridgeIfNeeded())
		{
			m_eUpdateState = eUpdateVideoIn;
			return TRUE;   // need to wait
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::ConnectPartyToVideoBridge(EBridgeConnectionState eDirection, DWORD wait_before_connect)
{
	if (wait_before_connect == 0)
	{
		ConnectPartyToVideoBridge(eDirection);
	}
	else
	{
		TRACEINTO << m_partyConfName << ", wait_before_connect:" << wait_before_connect << " ticks";
		CSegment* pConnectToVideoBridgeParams = new CSegment;
		*pConnectToVideoBridgeParams << (DWORD)eDirection;
		StartTimer(WAIT_BEFORE_CONNECT_TO_VB_TIMER, wait_before_connect, pConnectToVideoBridgeParams);
	}
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnTimerConnectPartyToVideoBridge(CSegment* pParam)
{
	EBridgeConnectionState eDirection;
	DWORD dir;
	*pParam >> dir;
	eDirection = (EBridgeConnectionState)dir;
	TRACEINTO << m_partyConfName << ", BridgeConnectionState:" << GetBridgeConnectionStateStr(eDirection);
	ConnectPartyToVideoBridge(eDirection);
	if (m_eVidBridgeConnState & eDirection)
		m_pPartyApi->VideoReadyForSlide();
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::ConnectPartyToVideoBridge(EBridgeConnectionState eDirection)
{
	if (m_eVidBridgeConnState & eDirection)
	{
		TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << ", BridgeConnectionState:" << GetBridgeConnectionStateStr(eDirection) << " - Not needed";
		return;
	}

	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << ", BridgeConnectionState:" << GetBridgeConnectionStateStr(eDirection);

	CBridgePartyVideoInParams*  pInVideoParams  = NULL;
	CBridgePartyVideoOutParams* pOutVideoParams = NULL;

	SetSiteName(m_name);

	if ((eDirection & eSendOpenIn) && m_pCurrentReceiveScm->VideoOn() && !(m_eVidBridgeConnState & eSendOpenIn))
	{
		pInVideoParams = new CBridgePartyVideoInParams();
		m_eVidBridgeConnState |= eSendOpenIn;
		InitVideoParams(m_pCurrentReceiveScm, pInVideoParams);
		pInVideoParams->SetSiteName(m_siteName);

		pInVideoParams->SetTelePresenceMode(m_eTelePresenceMode);
		pInVideoParams->SetTelePresenceEPInfo(m_telepresenseEPInfo);
	}

	if ((eDirection & eSendOpenOut) && m_pTargetTransmitScm->VideoOn() && !(m_eVidBridgeConnState & eSendOpenOut))
	{
		pOutVideoParams = new CBridgePartyVideoOutParams();

		pOutVideoParams->SetTelePresenceMode(m_eTelePresenceMode);
		pOutVideoParams->SetTelePresenceEPInfo(m_telepresenseEPInfo);

		m_eVidBridgeConnState |= eSendOpenOut;
		// Init Party Video Layout Params
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
		InitVideoLayoutParams(pOutVideoParams, pConfParty);

		BOOL bEnableAltResForISDN = FALSE;
		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("SEND_WIDE_RES_TO_ISDN", bEnableAltResForISDN);

		pOutVideoParams->SetIsCascadeLink(GetPartyCascadeType() == CASCADE_MCU);

		if (bEnableAltResForISDN)
			pOutVideoParams->SetVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_REGULAR);
		else
			pOutVideoParams->SetVideoResolutionTableType(E_VIDEO_RESOLUTION_TABLE_ALTERNATIVE);

		InitVideoParams(m_pTargetTransmitScm, pOutVideoParams);
	}

	if (!pInVideoParams && !pOutVideoParams)
	{
		TRACEINTO << "PartyId:" << GetPartyId() << " - No Video to open...";
		return;
	}

	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(m_name, m_pParty,
	                                                                          GetPartyId(), m_RoomId,
	                                                                          GetInterfaceType(),
	                                                                          pInVideoParams, pOutVideoParams,
	                                                                          m_pBridgeMoveParams->GetAndResetVideoBridgePartyCntlOnImport(),
	                                                                          m_siteName);

	TRACEINTO << "PartyId:" << GetPartyId() << " - Establishing Video connection...";

	m_pConfAppMngrInterface->ConnectPartyVideo(pBrdgPartyInitParams);
	ON(m_bIsMemberInVidBridge);

	POBJDELETE(pInVideoParams);
	POBJDELETE(pOutVideoParams);
	POBJDELETE(pBrdgPartyInitParams);
}

//--------------------------------------------------------------------------
int CIsdnChangeModeCntl::UpdateVideoInBridgeIfNeeded()
{
	WORD details      = 0;
	BYTE bVideoInDiff = NO;
	if (m_pTargetReceiveScm->VideoOn() && m_pCurrentReceiveScm->VideoOn())
	{
		// we will always end update to bridge with new (current) scm
		// (Eitan 04/2008)
		bVideoInDiff = TRUE;  // (m_pTargetReceiveScm->m_vidMode != m_pCurrentReceiveScm->m_vidMode);

		if (bVideoInDiff)
		{
			if (!m_pCurrentReceiveScm->IsFreeVideoRate())
			{
				TRACESTRFUNC(eLevelError) << m_partyConfName << ", PartyId:" << GetPartyId() << " - Can not send update in case of VSW fixed";
				DBGPASSERT(1);
				return NO;
			}

			TRACEINTO << m_partyConfName;
			CBridgePartyVideoInParams* inVideoParams = new CBridgePartyVideoInParams;
			InitVideoParams(m_pCurrentReceiveScm, inVideoParams);

			m_pVideoBridgeInterface->UpdateVideoInParams(m_pParty, inVideoParams);

			POBJDELETE(inVideoParams);
		}
		else
			TRACEINTO << m_partyConfName << " - No update";
	}
	else
		TRACEINTO << m_partyConfName << " - One SCM (Target or Current) media is OFF";

	return bVideoInDiff;
}

//--------------------------------------------------------------------------
int CIsdnChangeModeCntl::UpdateVideoOutBridgeIfNeeded()
{
	BYTE bVideoOutDiff = NO;
	if (m_pTargetTransmitScm->VideoOn() && m_pCurrentTransmitScm->VideoOn())
	{
		bVideoOutDiff = (m_pTargetTransmitScm->m_vidMode != m_pCurrentTransmitScm->m_vidMode);

		if (bVideoOutDiff == FALSE)
			bVideoOutDiff =  (m_pTargetTransmitScm->GetMediaBitrate(CComMode::eMediaTypeVideo) != m_pCurrentTransmitScm->GetMediaBitrate(CComMode::eMediaTypeVideo));

		if (bVideoOutDiff)
		{
			if (!m_pConfScm->IsFreeVideoRate())
			{
				TRACESTRFUNC(eLevelError) << m_partyConfName << ", PartyId:" << GetPartyId() << " - Can not send update in case of VSW fixed";
				DBGPASSERT(1);
				return NO;
			}

			TRACEINTO << m_partyConfName << " - Update Bridge";
			CBridgePartyVideoOutParams* outVideoParams = new CBridgePartyVideoOutParams;
			InitVideoParams(m_pTargetTransmitScm, outVideoParams);
			outVideoParams->SetIsCascadeLink(GetPartyCascadeType() == CASCADE_MCU);
			m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);
			POBJDELETE(outVideoParams);
		}
		else
			TRACEINTO << m_partyConfName << " - No update";
	}
	else
		TRACEINTO << m_partyConfName << " - One SCM (Target or Current) media is OFF";

	return bVideoOutDiff;
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnVidBrdgConChangeVideo(CSegment* pParam)
{
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << " - Connect has received after disconnect";
		DBGPASSERT(1);
	}
	else
	{
		DeleteTimer(CHANGETOUT);
		HandleVideoBridgeConnectedInd(pParam);

		TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState);

		m_waitForAckFromVideoOut &= !((m_eVidBridgeConnState & eOutConnected) == eOutConnected);
		m_waitForAckFromVideoIn  &= !((m_eVidBridgeConnState & eInConnected) == eInConnected);
	}

	if (AreTwoDirectionsConnectedToVideoBridge())
	{
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTED);
		UpdatePartyStateInCdr();
	}

	EndChangeMode();
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnVidBrdgConChangeContent(CSegment* pParam)
{
	HandleVideoBridgeConnectedInd(pParam);

	m_waitForAckFromVideoIn &= !((m_eVidBridgeConnState & eInConnected) == eInConnected);

	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState);

	if (AreTwoDirectionsConnectedToVideoBridge())
	{
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTED);
		UpdatePartyStateInCdr();
	}
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::EndChangeMode()
{
	CMedString cstr;
	cstr << "CIsdnChangeModeCntl::EndChangeMode - " << m_partyConfName <<"\n";
	cstr << "m_state(when enter func)= ";
	switch (m_state)
	{
		case (IDLE):
		{
			cstr << "IDLE";
			break;
		}

		case (CHANGECONTENT):
		{
			cstr << "CHANGECONTENT";
			break;
		}

		case (CHANGEVIDEO):
		{
			cstr << "CHANGEVIDEO";
			break;
		}

		default:
		{
			cstr << "Unknown state!!" << m_state;
			break;
		}
	} // switch

	cstr <<"\n";

	if (m_WaitForEndChangeModeToConnectContent)
	{
		cstr << "Need to connect to content bridge!!! - change to CHANGECONTENT";
		PTRACE(eLevelInfoNormal, cstr.GetString());
		ConnectToContentBridgeIfPossible();
		return;
	}

	// change mode requests received while in process.
	if ((!(*m_pCurrentTransmitScm == *m_pTargetTransmitScm)) || m_IsMovedParty)
	{
		if (m_IsMovedParty)
			cstr << "party has been moved -> need to change video mode";
		else
			cstr << "CurrentScm != Target Scm in Transmit -> redo the new scm";

		// first change m_state and then dispatch the change mode event
		m_state = IDLE;
		PTRACE(eLevelInfoNormal, cstr.GetString());
		DispatchChangeModeEvent();  // redo the new scm
		return;
	}

	if (!(*m_pCurrentReceiveScm == *m_pTargetReceiveScm))
	{
		cstr << "CurrentScm != TargetScm in Receive remote opened differnt video alg than we expected";
	}
	else
	{
		cstr << "CurrentScm == TargetScm in Receive&Transmit --> EndChangeMode ";
	}

	WORD endChangeMode = TRUE;

	if (m_waitForAckFromVideoOut)
	{
		cstr << "\n Wait for ACK from VideoBridge (VideoOut)";
		if (m_state != CHANGEVIDEO)
		{
			m_state = CHANGEVIDEO;
			cstr << "\n change m_state to CHANGEVIDEO";
		}

		endChangeMode = FALSE;
	}

	if (m_waitForAckFromVideoIn)
	{
		cstr << "\n Wait for ACK from VideoBridge (VideoIn)";
	}

	if (endChangeMode)
	{
		m_state = IDLE;
		cstr << "\n m_state = IDLE , Exit from change mode";
	}

	PTRACE(eLevelInfoNormal, cstr.GetString());
}

// CONTENT SECTION
// - ChangeContent() - open the content channel in xmit_mode
// - OnRmtXmitModeChangeContent() - wait until  content channel is opened in REMOTE_XMIT_MODE
// - ConnectToContentBridgeIfPossible() - connect party to content brigde only if IVR is finished
// and presentation_out received from party
// - ConnectPartyToContentBridge() - connects the party to content bridge
// - OnContentBrdgConnectedChangeContent - receive ack from bridge
//
// CAM/PRESENTATION_OUT
// - OnCAMUpdatePartyInConf() - received indication from CAM - party finished IVR
// - OnPartyPresentationOutStreamUpdated() - received indication from party PresentationOutStreamUpdated
// - OnPartyPresentationOutStreamUpdatedIdle() - received indication from party PresentationOutStreamUpdated,
// try connect party to content bridge
//
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::ChangeContent()
{
	DeleteTimer(CHANGETOUT);

	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId();
	m_pTargetTransmitScm->Dump(1);
	m_pCurrentTransmitScm->Dump(1);

	CComMode TempTargetTransmitScm(*m_pCurrentTransmitScm);
	TempTargetTransmitScm.m_contentMode = m_pTargetTransmitScm->m_contentMode;

	m_state = CHANGECONTENT;
	StartTimer(CHANGETOUT, 15*SECOND /*CHANGE_TOUT*/);

	TRACEINTO << m_partyConfName << " - Selected SCM is:";
	TempTargetTransmitScm.UpdateH221string(1);
	TempTargetTransmitScm.Dump(1);

	m_pPartyApi->ChangeMode(&TempTargetTransmitScm);

	if (IsPartyH239() && m_pTargetTransmitScm->IsContentOn()) // H.239
	{
		if (m_pCurrentTransmitScm->GetContentMode() == 0)       // only on connection send AMC_C&I MCS
			m_pPartyApi->SendH239MCS(TempTargetTransmitScm.m_contentMode.GetControlID());

		if (m_pTargetTransmitScm->GetContentModeContentRate() != AMSC_0k) // send AMC CI Video Mode
		{
			m_pPartyApi->SendH239ContentVideoMode(&(TempTargetTransmitScm.m_contentMode));
		}
		else                                                    // send logical channel inactive
		{
			m_pPartyApi->SendContentTokenMediaProducerStatus(TempTargetTransmitScm.m_contentMode.GetControlID(), CHANNEL_INACTIVE);
		}
	}

	UpdateCurrComMode(&TempTargetTransmitScm);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnRmtXmitModeChangeContent(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	CComMode OldComMode = *m_pCurrentReceiveScm;
	OnRmtXmitMode(pParam);
	WORD onOff = 0;
	CompareNewScmToOld(OldComMode);

	OnPartyEndChangeContent((CSegment*)NULL);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnPartyEndChangeContent(CSegment* pParam)
{
	if (m_pCurrentTransmitScm->m_contentMode == m_pCurrentReceiveScm->m_contentMode)
	{
		TRACEINTO << m_partyConfName << " - Remote and Transmit mode are equal";

		if (m_pCurrentTransmitScm->IsContentOn() && m_pCurrentReceiveScm->IsContentOn())
		{
			TRACEINTO << m_partyConfName << " - Content is ON";
			DeleteTimer(CHANGETOUT);
			// If not connected to content bridge yet.....
			if (!m_isContentConn)
			{
				TRACEINTO << m_partyConfName << " - Remote opened Content channel";
				m_state = IDLE;
				ConnectToContentBridgeIfPossible();
			}
			else
			{
				TRACEINTO << m_partyConfName << " - Party already connected to Content bridge";
				if (m_pContentBridge->IsPartyWaitForRateChange(m_pParty))
				{
					TRACEINTO << m_partyConfName << " - Need to update Content bridge";
					// because in content bridge the rate is in ip terms we should
					// translate it from AMSC (as in CComMode) to IP
					BYTE CurrentContentRateAmsc = m_pCurrentTransmitScm->GetContentModeContentRate();
					CCapSetInfo lCapInfo(eH263CapCode);
					DWORD CurrentContentRate = lCapInfo.GetEpcBitRate(CurrentContentRateAmsc)/100;
					BYTE parameterID = m_pCurrentTransmitScm->GetContentModeControlID();
					// Update CB that Party change rate
					m_pTaskApi->PartyContentRateChanged(m_pParty, parameterID, CurrentContentRate);

					if (IsOutDirectionConnectedToVideoBridge())
					{
						// In this case we need to inform the video bridge to change its rate towards the ep (Encoder - Video out).
						if (m_pCurrentTransmitScm->IsFreeVideoRate())
							UpdateVideoOutBridgeH239Case();
					}

					if (IsPartyH239() && m_pTargetTransmitScm->IsContentOn()) // H.239
					{
						if (m_pTargetTransmitScm->GetContentModeContentRate() != AMSC_0k) // send AMC CI Video Mode
						{
							m_pPartyApi->SendH239ContentVideoMode(&m_pTargetTransmitScm->m_contentMode);
						}
					}
					EndChangeMode();
				}
			}
		}
		else
		{
			EndChangeMode();
			PTRACE2(eLevelInfoNormal, "CIsdnChangeModeCntl::OnPartyEndChangeContent - Content is OFF: Name - ", m_partyConfName);
			DeleteTimer(CHANGETOUT);
			DisconnectPartyFromContentBridge();
		}
	}
	else
	{
		CMedString details;
		details << m_partyConfName << " - Remote Content mode is different then expected";
		details << "\n  remote content rate :";

		CContentMode cm        = m_pCurrentReceiveScm->m_contentMode;
		WORD content_bitrate   = 0;
		BYTE contentRateOpcode = AMSC_0k;
		if (cm.IsContentModeOn())
			contentRateOpcode = cm.GetContentRate();

		details << CCDRUtils::Get_Content_command_Bitrate((BYTE)contentRateOpcode, &content_bitrate);

		details << "\n  local content rate  :";
		CContentMode lm = m_pCurrentTransmitScm->m_contentMode;
		contentRateOpcode = AMSC_0k;
		if (lm.IsContentModeOn())
			contentRateOpcode = lm.GetContentRate();

		details << CCDRUtils::Get_Content_command_Bitrate((BYTE)contentRateOpcode, &content_bitrate);

		TRACEINTO << details.GetString();
	}
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnPartyPresentationOutStreamUpdatedIdle(CSegment* pParam)
{
	// if we receive this opcode and we are in IDLE (meaning we finish change content
	// state and we after IVR)
	// we can try to connect to content brdg
	TRACEINTO << m_partyConfName;
	OnPartyPresentationOutStreamUpdated(pParam);

	ConnectToContentBridgeIfPossible();
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnPartyPresentationOutStreamUpdatedCHANGEVIDEO(CSegment* pParam)
{
	// if we receive this opcode and we are in CHANGEVIDEO (meaning we finish change
	// content state and we are after IVR)
	// we need to wait for change video
	TRACEINTO << m_partyConfName;
	OnPartyPresentationOutStreamUpdated(pParam);

	if ((m_pTargetTransmitScm->m_contentMode == m_pCurrentTransmitScm->m_contentMode) &&
			m_pTargetTransmitScm->IsContentOn())
		ON(m_WaitForEndChangeModeToConnectContent);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnPartyPresentationOutStreamUpdatedCHANGECONTENT(CSegment* pParam)
{
	// if we receive this opcode and we are in HANGECONTENT (meaning didn't finish change
	// content state yet and we are after IVR)
	// we need to wait for change content.
	TRACEINTO << m_partyConfName;
	OnPartyPresentationOutStreamUpdated(pParam);

	if ((m_pTargetTransmitScm->m_contentMode == m_pCurrentTransmitScm->m_contentMode) &&
			m_pTargetTransmitScm->IsContentOn())
		ON(m_WaitForEndChangeModeToConnectContent);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnPartyPresentationOutStreamUpdated(CSegment* pParam)
{
	// if we receive this opcode, we already received PARTY_IN_CONF from CAM.
	TRACEINTO << m_partyConfName << ", state:" << m_state;
	ON(m_presentationStreamOutIsUpdated);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnCAMUpdatePartyInConf(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	ON(m_isPartyInConf);

	// check if the content channels were opened
	if (!m_isSentH239Out)
	{
		if (m_pCurrentTransmitScm->m_contentMode.IsContentModeOn())
		{
			ON(m_isSentH239Out);
			m_pPartyApi->UpdatePresentationOutStream();
		}
	}

	ConnectToContentBridgeIfPossible();
}

// In order to connect to the content bridge we need to receive 2 opcodes:
// 1) From CAM - PARTY_IN_CONF
// 2) From PARTY - PRESENTATION_OUT_STR_UPDATE
// This function check both flags and connect to content bridge or
// send UpdatePresentationOutStream to party if necessary...
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::ConnectToContentBridgeIfPossible()
{
	if (m_isContentConn)
	{
		TRACEINTO << m_partyConfName << " - Party is already connected to Content bridge";
		return;
	}

	if (m_isPartyInConf)
	{
		if (m_presentationStreamOutIsUpdated)
		{
			m_state = CHANGECONTENT;
			TRACEINTO << m_partyConfName << " - Connect the content bridge";
			OFF(m_WaitForEndChangeModeToConnectContent);
			ConnectPartyToContentBridge();
		}
		else
		{
			TRACEINTO << m_partyConfName << " - Content channels aren't open yet";
			// Should we send UpdateStream ??
			EndChangeMode();
		}
	}
	else
	{
		TRACEINTO << m_partyConfName << " - Still In IVR";
		EndChangeMode();
	}
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::ConnectPartyToContentBridge()
{
	TRACEINTO << m_partyConfName;

	if (m_pContentBridge)
	{
		if (!m_isContentConn)
		{
			ON(m_isContentConn);

			BYTE bCascade = FALSE;
			if ((WORD)CASCADE_MODE_MCU == GetNodeType())
				bCascade = TRUE;

			// because in content bridge the rate is in ip terms we should
			// translate it from AMSC (as in CComMode) to IP
			BYTE CurrentContentRateAmsc = m_pCurrentTransmitScm->GetContentModeContentRate();
			CCapSetInfo lCapInfo(eH263CapCode);
			DWORD CurrentContentRate = lCapInfo.GetEpcBitRate(CurrentContentRateAmsc) / 100;
			WORD CurrentContentProtocol = H263;

			CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
			TRACECOND_AND_RETURN(!pConfParty , "pConfParty is NULL");

			int masterSlaveStatus = (int)pConfParty->GetCascadeMode();

			CContentBridgePartyInitParams bridgePartyInitParams(m_name, m_pParty, GetPartyId(), GetInterfaceType(), CurrentContentRate, CurrentContentProtocol, FALSE, NULL, NULL, NULL, NULL, bCascade, masterSlaveStatus);  //HP content

			if (TRUE == bCascade)
			{
				TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << ", ContentRate:" << CurrentContentRate << " - Inform Conference On LinkTryToConnect";

				CSegment* pParam = new CSegment;
				bridgePartyInitParams.Serialize(NATIVE, *pParam);
				m_pTaskApi->LinkTryToConnect(pParam);
				POBJDELETE(pParam);
			}
			else
			{
				TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << ", ContentRate:" << CurrentContentRate << " - Establishing Content Party connection";
				m_pContentBridge->ConnectParty(&bridgePartyInitParams);
			}
		}
	}
}

//--------------------------------------------------------------------------
int CIsdnChangeModeCntl:: OnContentBrdgConnectedChangeContent(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	CPartyCntl::OnContentBrdgConnected(pParam);
	if (!m_isContentConn)
	{
		DBGPASSERT(1);
		TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << " - Problems while connecting";
		if (m_isFaulty)
		{
			m_pTaskApi->PartyDisConnect(H323_CALL_CLOSED_PROBLEM_WITH_CONTENT_CONNECTION_TO_MCU, m_pParty);
			return -1;
		}
	}
	EndChangeMode();
	return 0;
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::UpdateVideoOutBridgeH239Case()
{
	TRACEINTO << m_partyConfName;

	CBridgePartyVideoOutParams* outVideoParams = new CBridgePartyVideoOutParams;

	InitVideoParams(m_pCurrentTransmitScm, outVideoParams);
	outVideoParams->SetIsCascadeLink(GetPartyCascadeType() == CASCADE_MCU);
	m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);

	POBJDELETE(outVideoParams);
}

// if any change mode occurred during the change mode process
// the new scm will be recorded at m_pTargetTransmitScm.
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnConfChangeModeChangeContent(CSegment* pParam)
{
	TRACEINTO << m_partyConfName << " - Do nothing";
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnRmtXmitModeAnycase(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CComMode OldComMode = *m_pCurrentReceiveScm;
	OnRmtXmitMode(pParam);

	CompareNewScmToOld(OldComMode);
	if (m_needToChangeVideoIn == TRUE)
	{
		ChangeVideoBridgeVideoInState();
		OFF(m_needToChangeVideoIn);
	}
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::CompareNewScmToOld(CComMode& OldComMode)
{
	if (OldComMode.GetVidMode() != m_pCurrentReceiveScm->GetVidMode())
	{
		if (OldComMode.GetVidMode() == Video_Off)
			TRACEINTO << m_partyConfName << " - Remote opened Video";
		else
			TRACEINTO << m_partyConfName << " - Remote changed it's Video mode";

		if (m_pCurrentReceiveScm->GetVidMode() != m_pTargetReceiveScm->GetVidMode())
			TRACEINTO << m_partyConfName << " - Remote opened different video algorithm then expected";

		m_waitForAckFromVideoIn = ChangeVideoBridgeVideoInState();
	}

	// simulate party AIM /AIA
	WORD onOff = 0;
	if (OldComMode.GetAudMode() != m_pCurrentReceiveScm->GetAudMode())
	{
		if (m_pCurrentReceiveScm->GetAudMode() != m_pTargetReceiveScm->GetAudMode())
		{
			TRACEINTO << m_partyConfName << " - Remote Audio mode changed, so mute Audio";
			onOff = 1;
		}
		else
		{
			TRACEINTO << m_partyConfName << " - Remote and Target Audio mode same, so un-mute Audio";
			onOff = 0;
		}

		m_pTaskApi->AudioActive(GetPartyId(), onOff, MCMS);
	}
}

// REMOTE CAPS SECTION
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnRmtCaps(CSegment* pParam)
{
	BYTE NeedChangeMode = FALSE;
	CMedString cstr;
	CComMode   newComMode;

	if (UpdateRemoteCaps(pParam))
	{
		UpdateTargetTransmitScm(); // set m_pTargetTransmit = m_pTargetTransmit /\ remote caps

		if (IsChangeModeNeeded())
		{
			TRACEINTO << m_partyConfName << " - Remote cap has been updated, change mode is needed";
			NeedChangeMode = TRUE;
		}
		else
		{
			TRACEINTO << m_partyConfName << " - Remote cap has been updated, change mode is NOT needed";
		}
	}
	else
	{
		TRACEINTO << m_partyConfName << " - Remote cap has NOT been updated";
	}

	if (NeedChangeMode != FALSE)
		m_pTaskApi->PartyChangeVidMode(m_pParty, FALSE);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnRmtCapsIdle(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnRmtCaps(pParam);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnRmtCapsChangeContent(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnRmtCaps(pParam);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnRmtCapsChangeVideo(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
	OnRmtCaps(pParam);
}

//--------------------------------------------------------------------------
BOOL CIsdnChangeModeCntl::UpdateRemoteCaps(CSegment* pParam)
{
	CCapH320 newRmtCap;
	newRmtCap.DeSerialize(NATIVE, *pParam);

	TRACEINTO << m_partyConfName << " - New remote caps received";
	newRmtCap.Dump();

	BOOL updateRmtCaps = FALSE;

	// Here we receive second capSet from end-point after exchangeCap.
	// If there is mbe_cap and H263/H264/H239_caps on both sides(MCU <-> end-point) we must
	// update remote caps to H263/H264/H239
	if (newRmtCap.IsMBECapOn(Mbe_Cap) && m_pLocalCap->IsMBECapOn(Mbe_Cap) &&
			((m_pLocalCap->IsH263() && newRmtCap.IsH263()) ||
			 (m_pLocalCap->IsH264() && newRmtCap.IsH264()) ||
			 (m_pLocalCap->IsH239Cap() && newRmtCap.IsH239Cap())))
	{
		updateRmtCaps = TRUE;
	}
	else
	{
		// Correction for PictureTel
		// If the new remote capset is different from the current remote
		// capset in Mlp/Hmlp caps, update only the relevant fields and
		// send the updated remote capset to the Party task.
		if ((newRmtCap.GetHsdHmlpBas() != m_pLocalCap->GetHsdHmlpBas()) ||
				(newRmtCap.GetMlpBas() != m_pLocalCap->GetMlpBas()))
		{
			m_pRemoteCap->SetHsdHmlpCap(newRmtCap);
			m_pRemoteCap->SetMlpCap(newRmtCap);
		}

		// change - add video on condition for updating capabilities.
		// fixed bug: after mux sync lost FX sends caps with zero mode and we update remote caps (Ron)
		if (!newRmtCap.IsDataCap(Dxfer_Cap_6400) && m_pRemoteCap->IsDataCap(Dxfer_Cap_6400) &&
				(newRmtCap.IsH261VideoCap(V_Qcif) || newRmtCap.IsH261VideoCap(V_Cif) ||
				(newRmtCap.IsMBECapOn(Mbe_Cap) && (newRmtCap.IsH263() || newRmtCap.IsH264()))))
			updateRmtCaps = TRUE;

		// In case e.p. remover video caps in previos set
		if ((newRmtCap.IsH261VideoCap(V_Qcif) || newRmtCap.IsH261VideoCap(V_Cif)) &&
				(!(m_pRemoteCap->IsH261VideoCap(V_Qcif) && m_pRemoteCap->IsH261VideoCap(V_Cif))))
			updateRmtCaps = TRUE;

		if ((newRmtCap.OnAudioCap(e_G722_64) && !m_pRemoteCap->OnAudioCap(e_G722_64)) ||
				(newRmtCap.OnAudioCap(e_G722_48) && !m_pRemoteCap->OnAudioCap(e_G722_48)))
			updateRmtCaps = TRUE;
	}

	if (!updateRmtCaps && CASCADE_MODE_MCU == GetNodeType())
	{
		updateRmtCaps = TRUE;
	}

	if (updateRmtCaps)
	{
		/* We don't get encryption/H.239 caps now because they come
			 separately from others caps, so before copying we have to store
			 old encryption/H.239 caps */

		CCapECS*  pOldECSCap  = NULL;
		CCapH239* pOldH239Cap = NULL;

		if (m_pRemoteCap->GetCapECS())
		{
			pOldECSCap  = new CCapECS;
			*pOldECSCap = *(m_pRemoteCap->GetCapECS());
		}

		if (m_pRemoteCap->IsH239Cap())
		{
			pOldH239Cap  = new CCapH239;
			*pOldH239Cap = *(m_pRemoteCap->GetH239Caps());
		}

		/* copy new remote caps */
		*m_pRemoteCap = newRmtCap;

		/* Now copy old encryption/H.239 to new rmtCap */
		if (pOldECSCap)
		{
			m_pRemoteCap->CreateCCapECS(pOldECSCap);
			POBJDELETE(pOldECSCap);
		}

		if (pOldH239Cap)
		{
			m_pRemoteCap->SetOnlyExtendedVideoCaps(pOldH239Cap);
			POBJDELETE(pOldH239Cap);
		}
	}
	return updateRmtCaps;
}

//--------------------------------------------------------------------------
BYTE CIsdnChangeModeCntl::IsChangeModeNeeded()
{
	BYTE retVal = FALSE;
	CMedString details;

	if (!(*m_pCurrentTransmitScm == *m_pTargetTransmitScm))
	{
		retVal |= TRUE;
		details << "[CurrentScm != TargetScm in Transmit]";
	}
	else
	{
		retVal |= FALSE;
		details << "[CurrentScm == TargetScm in Transmit]";
	}

	if (!(*m_pCurrentReceiveScm == *m_pTargetReceiveScm))
	{
		retVal |= TRUE;
		details << ", [CurrentScm != TargetScm in Receive]";
	}
	else
	{
		retVal |= FALSE;
		details << ", [CurrentScm == TargetScm in Receive]";
	}

	details << ", retVal:";
	if (retVal)
		details << "TRUE";
	else
		details << "FALSE";

	TRACEINTO << m_partyConfName << " - " << details.GetString();

	return retVal;
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::UpdateTargetTransmitScm()
{
	// TargetTransmitScm = Local mode /\ remote caps
	CVidMode TargetTransmitVideoMode = m_pConfScm->m_vidMode; // conf scm is reflecting the allocated resource
	WORD     IsAutoVidScm = TargetTransmitVideoMode.IsAutoVidScm();
	CVidMode IntersectVidMode;

	// In case of HD720 asymmetric mode, (target transmit is update with HD720) we don't support intermediate modes between SD30 and HD720
	// we need to see if remote supports at least HD72015fps, if not we shall do the intersect between the target receive scm and the remote cap
	eSystemCardsMode systemCardsMode = GetSystemCardsBasedMode();
	if ((systemCardsMode == eSystemCardsMode_mpm) && TargetTransmitVideoMode.IsCapableOfHD720_15fps() && (!m_pRemoteCap->IsCapableOfHD720_15()))
	{
		TRACEINTO << m_partyConfName << " - Remote doesn't supports HD720 with at least 15fps";
		CVidMode TargetReceiveVideoMode = m_pTargetReceiveScm->m_vidMode;
		TargetReceiveVideoMode.Intersect(m_pRemoteCap, IsAutoVidScm, IntersectVidMode);
		m_pTargetTransmitScm->SetHd720Enabled(FALSE);
	}
	// In case of HD1080 asymmetric mode, (target transmit is update with HD1080) we don't support intermediate modes between HD720 and HD1080
	// we need to see if remote supports at least HD1080 15fps, if not we shall do the intersect between the target receive scm and the remote cap
	else if (TargetTransmitVideoMode.IsCapableOfHD1080_15fps() && (!m_pRemoteCap->IsCapableOfHD1080_15()))
	{
		TRACEINTO << m_partyConfName << " - Remote doesn't supports HD1080 with at least 15fps";
		CVidMode TargetReceiveVideoMode = m_pTargetReceiveScm->m_vidMode;
		TargetReceiveVideoMode.Intersect(m_pRemoteCap, IsAutoVidScm, IntersectVidMode);
		m_pTargetTransmitScm->SetHd1080Enabled(FALSE);
	}
	// In case of HD720 60fps asymmetric mode, (target transmit is update with HD720 60) we don't support intermediate modes between SD60 and HD720 60
	// we need to see if remote supports at least HD720 50fps, if not we shall do the intersect between the target receive scm and the remote cap
	else if (TargetTransmitVideoMode.IsCapableOfHD720_50fps() && (!m_pRemoteCap->IsCapableOfHD720_50()))
	{
		TRACEINTO << m_partyConfName << " - Remote doesn't supports HD720 with at least 50fps";
		CVidMode TargetReceiveVideoMode = m_pTargetReceiveScm->m_vidMode;
		TargetReceiveVideoMode.Intersect(m_pRemoteCap, IsAutoVidScm, IntersectVidMode);
		m_pTargetTransmitScm->SetHd720At60Enabled(FALSE);
	}
	else
	{
		TargetTransmitVideoMode.Intersect(m_pRemoteCap, IsAutoVidScm, IntersectVidMode);
	}

	m_pTargetTransmitScm->SetVidMode(IntersectVidMode);

	if (m_pRemoteCap->IsH239Cap())
		m_pTargetTransmitScm->m_contentMode = m_pConfScm->m_contentMode;
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnPartyRefreshVideoAnycase(CSegment* pParam)
{
	if (m_pCurrentTransmitScm->VideoOn())
	{
		TRACEINTO << m_partyConfName << " - Send to bridge";
		m_pVideoBridgeInterface->VideoRefresh(GetPartyId());
	}
	else
	{
		TRACEINTO << m_partyConfName << " - Transmit video-out is OFF";
	}
}

// Video bridge indications
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnVideoInBrdgUpdatedChangeVideo(CSegment* pParam)
{
	WORD status;
	*pParam >> status;
	TRACEINTO << m_partyConfName << ", status:" << status;
	DBGPASSERT(status);

	OFF(m_waitForAckFromVideoIn);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnVideoInBrdgUpdatedAnycase(CSegment* pParam)
{
	WORD status;
	*pParam >> status;
	TRACEINTO << m_partyConfName << ", status:" << status;
	DBGPASSERT(status);

	OFF(m_waitForAckFromVideoIn);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnVideoOutBrdgUpdatedChangeVideo(CSegment* pParam)
{
	WORD status;
	*pParam >> status;
	TRACEINTO << m_partyConfName << ", status:" << status;
	DBGPASSERT(status);

	DeleteTimer(CHANGETOUT);
	OFF(m_waitForAckFromVideoOut);
	EndChangeMode();
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnVideoOutBrdgUpdatedAnycase(CSegment* pParam)
{
	WORD status;
	*pParam >> status;
	TRACEINTO << m_partyConfName << ", status:" << status;
	DBGPASSERT(status);
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnVidBrdgConVideoIdle(CSegment* pParam)
{
	HandleVideoBridgeConnectedInd(pParam);

	m_waitForAckFromVideoIn &= !((m_eVidBridgeConnState & eInConnected) == eInConnected);

	TRACEINTO << m_partyConfName << ", PartyId:" << GetPartyId() << ", BridgeConnectionState:" << GetBridgeConnectionStateStr(m_eVidBridgeConnState);

	if (AreTwoDirectionsConnectedToVideoBridge())
	{
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_CONNECTED);
		UpdatePartyStateInCdr();
	}
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnVidBrdgDisconnectAnycase(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	DeleteTimer(CHANGETOUT);

	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);

	if (resStat == statVideoInOutResourceProblem)
	{
		BYTE mipHwConn    = (BYTE)eMipBridge;
		BYTE mipMedia     = (BYTE)eMipVideo;
		BYTE mipDirect    = 0;
		BYTE mipTimerStat = 0;
		BYTE mipAction    = 0;
		*pParam >>  mipDirect >> mipTimerStat >> mipAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, MpiErrorNumber);
		POBJDELETE(pSeg);
		return;
	}
	else if (resStat != statOK)
	{
		// if we receive disconnect from the bridge with status that is not OK we should change call the to secondary
		SetPartyToSecondaryAndEndChangeMode(); // SECONDARY_CAUSE_NO_VIDEO_CONNECTION
		DisconnectPartyFromVideoInAndOut();
	}
}

// Content bridge indications
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnContentBrdgConnectedAnycase(CSegment* pParam)
{
}

//--------------------------------------------------------------------------
int CIsdnChangeModeCntl:: OnContentBrdgDisconnected(CSegment* pParam)
{
	return CPartyCntl::OnContentBrdgDisconnected(pParam);
}
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnXCodeBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CIsdnChangeModeCntl::OnXCodeBrdgDisconnected", GetPartyRsrcId());
	CPartyCntl::OnXCodeBrdgDisconnected(pParam);
}
// party indications
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnPartyEndChangeVideo(CSegment* pParam)
{
}

// self timers
//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnTimerChangeContent(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Remote did not open Content, remove Content mode from SCM and try to open Video");
	CContentMode NoContentMode;
	m_pTargetTransmitScm->SetContentMode(NoContentMode);
	m_pTargetReceiveScm->SetContentMode(NoContentMode);
	OFF(m_WaitForEndChangeModeToConnectContent);
	ON(m_EpDidNotOpenContent);
	m_state = IDLE;
	DispatchChangeModeEvent();
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnTimerChangeVideo(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - ChangeVideo not completed");
	m_state = IDLE;
	DispatchChangeModeEvent();
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::OnTimerVideoUpdate(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;
}

//--------------------------------------------------------------------------
void CIsdnChangeModeCntl::SetPartyToSecondaryAndEndChangeMode()
{
	TRACEINTO << m_partyConfName;

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_SECONDARY);

	m_pTargetTransmitScm->m_vidMode.SetVidMode(Video_Off);

	UpdateCurrComMode(m_pTargetTransmitScm);
	*m_pCurrentTransmitScm = *m_pTargetTransmitScm;
	m_pPartyApi->ChangeMode(m_pTargetTransmitScm);

	m_state = IDLE;
}

//--------------------------------------------------------------------------
BYTE CIsdnChangeModeCntl::AreRemoteCapsEnableChangeMode()
{
	BYTE rval = TRUE;
	CMedString cstr;
	cstr << "CIsdnChangeModeCntl::AreRemoteCapsEnableChangeMode - " << m_partyConfName << "\n";

	CVidMode ScmVidMode = m_pTargetTransmitScm->m_vidMode;
	WORD videoBridgeAlgorithm = ScmVidMode.GetVidMode();

	switch (videoBridgeAlgorithm)
	{
		case H264:
		{
			cstr << "Target video Alg - H264 \t";
			if (!m_pRemoteCap->IsH264())
			{
				cstr << "Remote Cap does not contain H264\n";
				rval = FALSE;
			}
			break;
		}

		case H263:
		{
			cstr << "Target video Alg - H263 \t";
			if (!m_pRemoteCap->IsH263())
			{
				cstr << "Remote Cap does not contain H263\n";
				rval = FALSE;
			}
			break;
		}

		case H261:
		{
			WORD format             = ScmVidMode.GetVidFormat();
			WORD RemoteCapFrameRate = m_pRemoteCap->GetH261CapMpi(format);
			WORD ScmCIFFrameRate    = ScmVidMode.GetCifMpi();
			WORD ScmQCIFFrameRate   = ScmVidMode.GetQcifMpi();
			cstr << "Target video Alg - H261, ";
			switch (format)
			{
				case (V_Cif):
				{
					cstr << "CIF \t";
					if (!m_pRemoteCap->IsH261VideoCap(format))
					{
						cstr << "Remote Cap does not contain V_Cif \n";
						rval = FALSE;
					}
					else if (ScmCIFFrameRate < RemoteCapFrameRate)
					{
						cstr << "Remote Cap Cif frame rate: ";
						int tmp = TranslateIsdnFrameRateToVideoBridgeFrameRate(RemoteCapFrameRate, H261, cstr);
						cstr << "Target scm Cif frame rate: ";
						tmp  = TranslateIsdnFrameRateToVideoBridgeFrameRate(ScmCIFFrameRate, H261, cstr);
						rval = FALSE;
					}

					break;
				}

				case (V_Qcif):
				{
					cstr << "QCIF \t";
					if (!m_pRemoteCap->IsH261VideoCap(format))
					{
						cstr << "Remote Cap does not contain V_Qcif \n";
						rval = FALSE;
					}
					else if (ScmQCIFFrameRate < RemoteCapFrameRate)
					{
						cstr << "Remote Cap QCif frame rate: ";
						int tmp = TranslateIsdnFrameRateToVideoBridgeFrameRate(RemoteCapFrameRate, H261, cstr);
						cstr << "Target scm QCif frame rate: ";
						tmp  = TranslateIsdnFrameRateToVideoBridgeFrameRate(ScmQCIFFrameRate, H261, cstr);
						rval = FALSE;
					}

					break;
				}
			} // switch

			break;
		}

		case (Video_Off):
		{
			cstr << "Target video mode is Video_Off";
			rval = FALSE;
		}
	} // switch

	DBGPASSERT(!rval);

	PTRACE(eLevelInfoNormal, cstr.GetString());
	return rval;
}

//--------------------------------------------------------------------------
BOOL CIsdnChangeModeCntl::IsH2634CifPreffered(CComMode* pTargetTransmitScm)
{
	// Private version for CASCADE ISDN between RMX and MGC ==> To be able to open the MGC(MCU) Video protocol do NOT change
	// The video protocol in sharpness calls if MCC was sent !!!
	if ((WORD)CASCADE_MODE_MCU == GetNodeType())
		return FALSE;

	// Conference Video Quality - relevant only in sharpness calls
	eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();
	if (vidQuality != eVideoQualitySharpness)
	{
		TRACEINTO << m_partyConfName << ", VideoQuality:" << vidQuality << " - False, Conference video quality is not sharpness";
		return FALSE;
	}

	// /Target Transmit video parameters
	CVidMode LocalVideoMode = pTargetTransmitScm->m_vidMode;
	WORD targetTransmitAlgorithm = LocalVideoMode.GetVidMode();
	if (targetTransmitAlgorithm != H264)
	{
		TRACEINTO << m_partyConfName << " - False, Target video algorithm isn't H264";
		return FALSE;
	}

	WORD targetTransmitH263format = LocalVideoMode.GetVidFormat();
	WORD targetTransmitframeRate  = LocalVideoMode.GetH263Mpi();
	if ((targetTransmitH263format != H263_CIF_4) || ((targetTransmitH263format == H263_CIF_4) && (targetTransmitframeRate > MPI_2)))
	{
		TRACEINTO
			<< m_partyConfName
			<< ", TargetTransmitH263Format:" << NumberOfFormat[targetTransmitH263format]
			<< ", TargetTransmitFrameRate:" << targetTransmitframeRate
			<< " - False, Target video algorithm doesn't include H263 4CIF with rate >=15";
		return FALSE;
	}

	// /Remote video parameters
	BOOL isRemoteSupportH264 = m_pRemoteCap->IsH264();
	BOOL isRemoteSupportH263 = m_pRemoteCap->IsH263();
	if ((!isRemoteSupportH264) || (!isRemoteSupportH263))
	{
		TRACEINTO << m_partyConfName << " - False, The remote caps doesn't include both H264 and H263";
		return FALSE;
	}

	CCapH263* remoteH263Cap = m_pRemoteCap->GetCapH263();
	if (IsValidPObjectPtr(remoteH263Cap))
	{
		BYTE highestH263Format = remoteH263Cap->GetHighestFormat();
		if (highestH263Format < H263_CIF_4)
		{
			TRACEINTO << m_partyConfName << " - False, The remote caps doesn't include H2634CIF";
			return FALSE;
		}
	}

	CCapH264* capH264 = m_pRemoteCap->GetCapH264();
	CCapSetH264* pCapSetH264 = capH264->GetLastCapSet(); // the last set is with the highest level
	if (!IsValidPObjectPtr(pCapSetH264))
	{
		TRACEINTO << m_partyConfName << " - False, H264 cap not valid";
		return FALSE;
	}

	WORD thisLevel = pCapSetH264->GetCapH264LevelValue();
	CH264Details thisH264Details(thisLevel);

	WORD  fs = pCapSetH264->GetCapH264CustomMaxFS();
	WORD  mbps = pCapSetH264->GetCapH264CustomMaxMBPS();
	DWORD maxFS = 0;
	DWORD maxMBPS = 0;
	if (fs == 0xFFFF)
		maxFS = thisH264Details.GetDefaultFsAsProduct();
	else
		maxFS = fs* CUSTOM_MAX_FS_FACTOR;

	if (mbps == 0xFFFF)
		maxMBPS = thisH264Details.GetDefaultMbpsAsProduct();
	else
		maxMBPS = mbps * CUSTOM_MAX_MBPS_FACTOR;

	BOOL toPrefferH2634Cif = IsH2634Cif15PreferedOverH264InSharpnessConf(maxFS, maxMBPS);

	TRACEINTO << m_partyConfName <<  ", maxFS:" << maxFS << ", maxMBPS:" << maxMBPS << ", toPrefferH2634Cif:" << toPrefferH2634Cif;
	return toPrefferH2634Cif;
}
