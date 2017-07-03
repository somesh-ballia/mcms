//+========================================================================+
//                            SipPartyControlChangeMode.cpp                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SipPartyControlChangeMode.cpp                               |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: UriA                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 13/11/05   |  Create + Stage 1 - Move between same conferences    |
//+========================================================================+


#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "StateMachine.h"
#include "Segment.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "NStream.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpCommonDefinitions.h"
#include "IpAddressDefinitions.h"
#include "IpScm.h"
#include "ConfPartyOpcodes.h"
#include "ConfDef.h"
#include "Conf.h"
#include "ConfApi.h"
#include "CommModeInfo.h"
#include "Capabilities.h"
#include "TaskApi.h"
#include "TaskApp.h"
#include "Party.h"
#include "PartyApi.h"

#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipCall.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "IpPartyControl.h"
#include "SipNetSetup.h"
#include "SIPCommon.h"
#include "SIPPartyControl.h"
#include "SIPPartyControlChangeMode.h"
#include "PartyApi.h"
#include "PartyRsrcDesc.h"
#include  "H263VideoMode.h"
#include "WrappersResource.h"
#include "IpChannelParams.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
#include  "ConfPartyGlobals.h"
#include  "H263VideoMode.h"
#include "StatusesGeneral.h"

#include "ContentBridge.h"
#include "VideoBridgeInterface.h"
#include "AudioBridgeInterface.h"
#include "AvcToSvcParams.h"

/// ***CREATE OUT SLAVES INTEGRATION  - TEMP CODE - TO BE REMOVED***
#include "MsSlavesController.h"
/// *****

////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipChangeModePartyCntl)
	ONEVENT(SCMCHANGEMODE,						IDLE,							CSipChangeModePartyCntl::OnConfChangeModeIdle)
	ONEVENT(IPPARTYMSECONDARY,					IDLE,							CSipPartyCntl::OnPartyMoveToSecondary)
	ONEVENT(REMOTE_SENT_RE_CAPS,				IDLE,							CSipChangeModePartyCntl::OnPartyReCapsIdle)
	ONEVENT(IPPARTYCONNECTED,					IDLE,							CSipChangeModePartyCntl::OnPartyRemoteConnectedIdle)
	ONEVENT(IPPARTYUPDATEBRIDGES,				IDLE,							CSipPartyCntl::OnPartyUpdateBridges)
	ONEVENT(IPPARTYUPDATEBRIDGES,               WAITING_FOR_MSFT_OUT_SLAVES,    CSipPartyCntl::OnPartyUpdateBridges)
	ONEVENT(REMOTE_SENT_DISCONNECT_BRIDGES,		IDLE,							CSipChangeModePartyCntl::OnPartyDisconnectBridgesIdle)
	ONEVENT(REMOTE_SENT_CONNECT_BRIDGES,		IDLE,							CSipChangeModePartyCntl::OnPartyConnectBridgesIdle)
	ONEVENT(SET_PARTY_AVC_SVC_MEDIA_STATE,		IDLE,							CSipChangeModePartyCntl::OnConfChangeModeUpgradeIdle)


	ONEVENT(PARTY_AUDIO_DISCONNECTED,			PARTY_RE_CAPS,					CSipChangeModePartyCntl::OnAudioBrdgDisconnectedPartyReCaps)
	ONEVENT(PARTY_AUDIO_CONNECTED,				PARTY_RE_CAPS,					CSipChangeModePartyCntl::OnAudConnectPartyReCaps)
	ONEVENT(SCMCHANGEMODE,						PARTY_RE_CAPS,					CSipChangeModePartyCntl::OnConfChangeModePartyReCaps)
	ONEVENT(CHANGEMODETOUT,						PARTY_RE_CAPS,					CSipChangeModePartyCntl::OnChangeModeToutReCaps)

	ONEVENT(PARTY_VIDEO_IN_UPDATED,				CHANGE_BRIDGES,					CSipPartyCntl::OnVideoInBrdgUpdated)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,			CHANGE_BRIDGES,					CSipPartyCntl::OnVideoOutBrdgUpdated)
	ONEVENT(PARTY_VIDEO_CONNECTED,				CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnVideoBrdgConnectedChangeBridges)
	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,		CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnVideoBrdgConnectedChangeBridges)
	ONEVENT(PARTY_VIDEO_DISCONNECTED,			CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnVideoBrdgDisconnectedChangeBridges)
	ONEVENT(END_VIDEO_UPGRADE_TO_MIX_AVC_SVC,	CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnEndVideoUpgradeToMix)
	ONEVENT(END_AUDIO_UPGRADE_TO_MIX_AVC_SVC,	CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnEndAudioUpgradeToMix)
    ONEVENT(PARTY_VIDEO_DISCONNECTED,			REALLOCATE_RSC,					CSipChangeModePartyCntl::OnVideoBrdgDisconnectedReallocRsrc)
    ONEVENT(PARTY_VIDEO_DISCONNECTED,			UPDATE_LEGACY_STATUS,			CSipChangeModePartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus)

	ONEVENT(REMOTE_SENT_RE_CAPS,				CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnPartyReCapsChangeBridges)
	ONEVENT(CHANGEMODETOUT,						CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnChangeModeToutChangeBridges)
	ONEVENT(SIP_PARTY_SEND_CHANNEL_HANDLE,		CONF_RESPONSE_PARTY_RE_CAPS,	CSipChangeModePartyCntl::OnPartySendChannelHandle)
	ONEVENT(IPPARTYCONNECTED,					CONF_RESPONSE_PARTY_RE_CAPS,	CSipChangeModePartyCntl::OnPartyRemoteConnectedResponseReCaps)
	ONEVENT(CHANGEMODETOUT,						CONF_RESPONSE_PARTY_RE_CAPS,	CSipChangeModePartyCntl::OnChangeModeToutResponseParty)

	ONEVENT(PARTY_VIDEO_IN_UPDATED,				UPDATE_BRIDGES_FOR_CHANNEL_HANDLE,	CSipPartyCntl::OnVideoInBrdgUpdatedChannelHandle)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,			UPDATE_BRIDGES_FOR_CHANNEL_HANDLE,	CSipPartyCntl::OnVideoOutBrdgUpdatedChannelHandle)
    ONEVENT(END_VIDEO_UPGRADE_TO_MIX_AVC_SVC,   UPDATE_BRIDGES_FOR_CHANNEL_HANDLE,  CSipPartyCntl::OnVideoInBrdgUpdatedChannelHandle)
    ONEVENT(PRESENTATION_OUT_STREAM_UPDATED,    UPDATE_BRIDGES_FOR_CHANNEL_HANDLE,  CSipChangeModePartyCntl::OnPartyPresentationOutStreamUpdateChannelHandle)
    ONEVENT(IPPARTYCONNECTED,                   UPDATE_BRIDGES_FOR_CHANNEL_HANDLE,  CSipChangeModePartyCntl::OnPartyRemoteConnectedChannelHandle)
	ONEVENT(REMOTE_SENT_RE_CAPS,				UPDATE_BRIDGES_FOR_CHANNEL_HANDLE,	CSipChangeModePartyCntl::OnPartyReCapsChannelHandle)

	ONEVENT(REMOTE_SENT_RE_CAPS,				CONF_REQUEST_PARTY_CHANGE_MODE,	CSipChangeModePartyCntl::OnPartyReCapsPartyChangeMode)
	ONEVENT(IPPARTYCONNECTED,					CONF_REQUEST_PARTY_CHANGE_MODE,	CSipChangeModePartyCntl::OnPartyRemoteConnectedPartyChangeMode)
	ONEVENT(CHANGEMODETOUT,						CONF_REQUEST_PARTY_CHANGE_MODE,	CSipChangeModePartyCntl::OnChangeModeToutPartyChangeMode)
	ONEVENT(PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED,						CONF_REQUEST_PARTY_CHANGE_MODE,	CSipChangeModePartyCntl::OnUpgradePartyToMixed)

	ONEVENT(COP_VIDEO_IN_CHANGE_MODE,			ANYCASE,						CSipChangeModePartyCntl::OnCopVideoBridgeChangeIn)
    ONEVENT(COP_VIDEO_OUT_CHANGE_MODE,			ANYCASE,						CSipChangeModePartyCntl::OnCopVideoBridgeChangeOut)
	ONEVENT(SCMCHANGEMODE,						ANYCASE,						CSipChangeModePartyCntl::OnConfChangeModeAnycase)
	ONEVENT(IPPARTYUPDATEBRIDGES,				ANYCASE,						CSipChangeModePartyCntl::OnPartyUpdateBridgesAnycase)
    ONEVENT(REALLOCATE_PARTY_RSRC_IND,	        REALLOCATE_RSC,                 CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate)
	ONEVENT(IPPARTYCONNECTED,					REALLOCATE_RSC,					CSipChangeModePartyCntl::OnPartyRemoteConnectedReallocRsrc)//????????????????
	ONEVENT(REMOTE_SENT_RE_CAPS,				REALLOCATE_RSC,					CSipChangeModePartyCntl::OnPartyReCapsPartyReallocRsrc)
	ONEVENT(AVC_SVC_ADDITIONAL_PARTY_RSRC_IND,	REALLOCATE_RSC,					CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate)
    ONEVENT(END_AVC_TO_SVC_ART_TRANSLATOR_DISCONNECTED,       REALLOCATE_RSC,   CPartyCntl::OnEndAvcToSvcArtTranslatorDisconnected)
	ONEVENT(PARTY_AUDIO_CONNECTED,				REALLOCATE_RSC,					CSipChangeModePartyCntl::OnAudConnectPartyReCaps)


	ONEVENT(PARTY_AUDIO_DISCONNECTED,			DISCONNECT_BRIDGES,				CSipChangeModePartyCntl::OnAudioBrdgDisconnected)
	ONEVENT(PARTY_VIDEO_DISCONNECTED,			DISCONNECT_BRIDGES,				CSipChangeModePartyCntl::OnVideoBrdgDisconnected)
	ONEVENT(PARTY_CONTENT_DISCONNECTED,			DISCONNECT_BRIDGES,				CSipChangeModePartyCntl::OnContentBrdgDisconnected)
	ONEVENT(FECC_PARTY_BRIDGE_DISCONNECTED,		DISCONNECT_BRIDGES,				CSipChangeModePartyCntl::OnFeccBrdgDisconnected)
	ONEVENT(IPPARTYCONNECTED,					DISCONNECT_BRIDGES,				CSipChangeModePartyCntl::OnPartyRemoteConnectedDisconnectBridges)

	ONEVENT(PARTY_AUDIO_CONNECTED,				CONNECT_BRIDGES,				CSipChangeModePartyCntl::OnAudioBrdgConnectedConnectBridges)
	ONEVENT(PARTY_VIDEO_CONNECTED,				CONNECT_BRIDGES,				CSipChangeModePartyCntl::OnVideoBrdgConnectedConnectBridges)
	ONEVENT(PARTY_CONTENT_CONNECTED,			CONNECT_BRIDGES,				CSipChangeModePartyCntl::OnContentBrdgConnectedConnectBridges)
	ONEVENT(FECC_PARTY_BRIDGE_CONNECTED,		CONNECT_BRIDGES,				CSipChangeModePartyCntl::OnFeccBrdgConnectedConnectBridges)
	ONEVENT(IPPARTYCONNECTED,					CONNECT_BRIDGES,				CSipChangeModePartyCntl::OnPartyRemoteConnectedConnectBridges)

    ONEVENT(SIP_PARTY_SEND_CHANNEL_HANDLE,      ANYCASE,                        CSipChangeModePartyCntl::OnPartySendChannelHandleAnycase)
	ONEVENT(PARTY_IN_CONF_IND,					ANYCASE,						CSipChangeModePartyCntl::OnCAMUpdatePartyInConf)
	ONEVENT(PRESENTATION_OUT_STREAM_UPDATED,	ANYCASE,						CSipChangeModePartyCntl::OnPartyPresentationOutStreamUpdate)
	ONEVENT(PARTY_CONTENT_CONNECTED,			ANYCASE,						CSipChangeModePartyCntl::OnContentBrdgConnectedAnycase)
	ONEVENT(PARTY_CONTENT_DISCONNECTED,			ANYCASE,						CSipChangeModePartyCntl::OnContentBrdgDisconnectedAnycase)
	ONEVENT(FALL_BACK_FROM_ICE_TO_SIP,			ANYCASE,						CSipPartyCntl::OnPartyFallbackFromIceToSip)
	ONEVENT(UPGRADETOMIXTOUT,					ANYCASE,						CSipChangeModePartyCntl::OnPartyUpgradeToMixTout)
//    ONEVENT(SET_PARTY_AVC_SVC_MEDIA_STATE,		IDLE,							CIpPartyCntl::OnConfChangeModeUpgradeIdle)
      ONEVENT(UPDATE_REMOTE_CAPS_FROM_PARTY,		ANYCASE,						CSipChangeModePartyCntl::OnUpdateRemoteCapsFromParty)
      ONEVENT(SEND_MRMP_STREAM_IS_MUST_REQ,ANYCASE,	CSipPartyCntl::OnMrmpStreamIsMustReq)
      ONEVENT(SIP_PARTY_VSR_MSG_IND,			ANYCASE,			CSipPartyCntl::OnPartyVsrMsgInd)
      ONEVENT(UPDATE_LAST_TARGET_MODE_MSG,		ANYCASE,						CSipChangeModePartyCntl::OnPartyLastTargetModeMsg)

      ONEVENT(MSFOCUSENDDISCONNECT,				            ANYCASE,			CSipChangeModePartyCntl::OnMSFocusEndDisConnection)

PEND_MESSAGE_MAP(CSipChangeModePartyCntl,CSipPartyCntl);


////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipChangeModeLyncPartyCntl)
  		ONEVENT(PARTY_VIDEO_DISCONNECTED,			UPDATE_LEGACY_STATUS,			CSipChangeModeLyncPartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus)
		ONEVENT(PARTY_VIDEO_CONNECTED,				UPDATE_LEGACY_STATUS,				CSipChangeModeLyncPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus)
		ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	UPDATE_LEGACY_STATUS,				CSipChangeModeLyncPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus)
PEND_MESSAGE_MAP(CSipChangeModeLyncPartyCntl,CSipChangeModePartyCntl);


extern const char* MediaStateToString(eConfMediaState confState);

///////////////////////////////////////////////////
CSipChangeModePartyCntl::CSipChangeModePartyCntl()
{
	m_eChangeModeState = eNotNeeded;
	m_bPartyControlChangeScm = FALSE;
	m_bStartRecapAfterChangeBridges = FALSE;
	m_bStartRecapAfterUpdateChannelHandle = FALSE;
	m_isSentH239Out = FALSE;
	m_bRestartSipfromTipToNonTip = FALSE;
	m_bShouldNotifyPartyOnSecondaryContent = FALSE;
	m_bIsLegacyContent = TRUE;

	VALIDATEMESSAGEMAP

	m_bIsBridgeDisconnectedAudio	= false;
	m_bIsBridgeDisconnectedVideo	= false;
	m_bIsBridgeDisconnectedContent	= false;
	m_bIsBridgeDisconnectedFecc		= false;

	m_bIsBridgeConnectedAudio		= false;
	m_bIsBridgeConnectedVideo		= false;
	m_bIsBridgeConnectedContent		= false;
	m_bIsBridgeConnectedFecc		= false;

	/// ***CREATE OUT SLAVES INTEGRATION  - TEMP CODE - TO BE REMOVED***
	m_CreateOutSlavesintegrationCounter = 0;
	/// **********

}


///////////////////////////////////////////////////
CSipChangeModePartyCntl::~CSipChangeModePartyCntl()
{
}
////////////////////////////////////////////////
const char*   CSipChangeModePartyCntl::NameOf() const
{
  return "CSipChangeModePartyCntl";
}

///////////////////////////////////////////////////
CSipChangeModePartyCntl& CSipChangeModePartyCntl::operator= (const CSipChangeModePartyCntl& other)
{
	if ( &other == this )
		return *this;

	m_eChangeModeState 				= other.m_eChangeModeState;
	m_bPartyControlChangeScm		= other.m_bPartyControlChangeScm;
	m_bStartRecapAfterChangeBridges = other.m_bStartRecapAfterChangeBridges;
	m_bStartRecapAfterUpdateChannelHandle = other.m_bStartRecapAfterUpdateChannelHandle;
	m_bRestartSipfromTipToNonTip      = other.m_bRestartSipfromTipToNonTip;
	m_bShouldNotifyPartyOnSecondaryContent = other.m_bShouldNotifyPartyOnSecondaryContent;
	m_bIsLegacyContent = other.m_bIsLegacyContent;

	m_bIsBridgeDisconnectedAudio	= other.m_bIsBridgeDisconnectedAudio;
	m_bIsBridgeDisconnectedVideo	= other.m_bIsBridgeDisconnectedVideo;
	m_bIsBridgeDisconnectedContent	= other.m_bIsBridgeDisconnectedContent;
	m_bIsBridgeDisconnectedFecc		= other.m_bIsBridgeDisconnectedFecc;

	m_bIsBridgeConnectedAudio		= other.m_bIsBridgeConnectedAudio;
	m_bIsBridgeConnectedVideo		= other.m_bIsBridgeConnectedVideo;
	m_bIsBridgeConnectedContent		= other.m_bIsBridgeConnectedContent;
	m_bIsBridgeConnectedFecc		= other.m_bIsBridgeConnectedFecc;

	(CSipPartyCntl&)*this = (CSipPartyCntl&)other;
	return *this;
}
/////////////////////////////////////////////////////////////////////////////
void  CSipChangeModePartyCntl::DispatchChangeModeEvent()
{
	CSipPartyCntl::DispatchChangeModeEvent();
	m_bPartyControlChangeScm = FALSE;
}
///////////////////////////////////////////////////
void CSipChangeModePartyCntl::HandlePendingScmIfNeeded()
{
	if(!m_pendingScmForUpgrade)
		return;

	TRACEINTO << "upgrade is done. handling pending Scm";
	//since scm is not updated with video streams and operation points, update initial mode with pending scm content only
	m_pIpInitialMode->SetMediaMode(m_pendingScmForUpgrade->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation), cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	m_pIpInitialMode->SetMediaMode(m_pendingScmForUpgrade->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation), cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

	POBJDELETE(m_pendingScmForUpgrade);
	ChangeScm(m_pIpInitialMode, GetIsAsSipContentEnable() );
}

///////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeScm(CIpComMode* pScm,BYTE IsAsSipContentEnable)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeScm: Name - ",m_partyConfName, GetPartyRsrcId());

	m_IsAsSipContentEnable = IsAsSipContentEnable;
	
	if(m_bPartyInUpgradeProcess)
	{
		//we can't start change scm while upgrade
		MoveScmToPendingWhileUpgrading(pScm);
		SetIsAsSipContentEnable(IsAsSipContentEnable);
		TRACEINTO << "Not allowed while upgrading to mix mode, save Scm";
		return;
	}

	if (pScm)
		ChangeInitialAccordingToNewScm(pScm);
	else
		PTRACE2(eLevelError,"CSipChangeModePartyCntl::ChangeScm - No new Scm: Name - ",m_partyConfName);

	m_bIsNewScm = TRUE;

	DispatchChangeModeEvent();
}

///////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeInitialAccordingToNewScm(CIpComMode* pScm)
{
	// (1)
	// set New SCM according to SIP behavior (Audio is taking from initial, Video in VSW should be set according to initial, Content should be removed)
	if(m_pIpInitialMode->IsMediaOn(cmCapAudio))
		pScm->SetMediaMode(m_pIpInitialMode->GetMediaMode(cmCapAudio),cmCapAudio);// the default is receive
	if(m_pIpInitialMode->IsMediaOn(cmCapAudio,cmCapTransmit))
		pScm->SetMediaMode(m_pIpInitialMode->GetMediaMode(cmCapAudio, cmCapTransmit), cmCapAudio, cmCapTransmit);
	//noa what about cp ?
	if(m_pIpInitialMode->GetConfType() == kCop || m_pIpInitialMode->GetConfType() == kVSW_Fixed  || m_pIpInitialMode->GetConfType() == kVideoSwitch)
	{
		// In Cop we leave the initial video mode as is. It will be changed in the Decide function when the change mode start actually.
		PTRACEPARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeInitialAccordingToNewScm: Leave the original initial video mode", GetPartyRsrcId());
		pScm->SetMediaMode(m_pIpInitialMode->GetMediaMode(cmCapVideo),cmCapVideo);// the default is receive
		pScm->SetMediaMode(m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapTransmit),cmCapVideo, cmCapTransmit);
	}

	m_pIpInitialMode->Dump("CSipChangeModePartyCntl::ChangeInitialAccordingToNewScm - m_pIpInitialMode is: ",eLevelInfoNormal);
	pScm->Dump("CSipChangeModePartyCntl::ChangeInitialAccordingToNewScm before - pScm is: ",eLevelInfoNormal);

	// VSW
	if(m_pIpInitialMode->GetConfType() == kVSW_Fixed || m_pIpInitialMode->GetConfType() == kVideoSwitch)
	{
		// Copy the static attribute from initial:
		pScm->CopyStaticAttributes(*m_pIpInitialMode);

		// Set content off in the scm from conf if initial doesn't contain content.
		if(m_pIpInitialMode->IsMediaOff(cmCapVideo, cmCapReceive, kRoleContentOrPresentation))
			pScm->SetMediaOff(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		if(m_pIpInitialMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
			pScm->SetMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

		pScm->Dump("CSipChangeModePartyCntl::ChangeInitialAccordingToNewScm after - pScm is: ",eLevelInfoNormal);
	}

	if(m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && pScm->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation))
	{
		// Copy content sdes: (Conference send scm with new content mode, without the content sdes)
		if (m_pIpInitialMode->GetIsEncrypted()==Encryp_On && pScm->GetIsEncrypted()==Encryp_On)
		{
			CSdesCap *pSdesCap = m_pIpInitialMode->GetSipSdes(cmCapVideo,cmCapReceive, kRolePresentation);
			pScm->SetSipSdes(cmCapVideo,cmCapReceive,kRolePresentation,pSdesCap);
		}
		pScm->SetRtcpFeedbackMask(m_pIpInitialMode->GetRtcpFeedbackMask(cmCapReceive, kRoleContentOrPresentation), cmCapReceive, kRoleContentOrPresentation);
	}

	if(m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation) && pScm->IsMediaOn(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation))
	{
		// Copy content sdes: (Conference send scm with new content mode, without the content sdes)
		if (m_pIpInitialMode->GetIsEncrypted()==Encryp_On && pScm->GetIsEncrypted()==Encryp_On) {
			CSdesCap *pSdesCap = m_pIpInitialMode->GetSipSdes(cmCapVideo,cmCapTransmit, kRolePresentation);
			pScm->SetSipSdes(cmCapVideo,cmCapTransmit,kRolePresentation,pSdesCap);
		}
		pScm->SetRtcpFeedbackMask(m_pIpInitialMode->GetRtcpFeedbackMask(cmCapReceive, kRoleContentOrPresentation), cmCapTransmit, kRoleContentOrPresentation);
	}

	if (pScm->GetIsTipMode() && !m_pIpInitialMode->GetIsTipMode())
	{
		TRACEINTO << "TIP fallback - Leave the original initial video mode";
		pScm->SetTipMode(eTipModeNone);
		pScm->SetMediaMode(m_pIpInitialMode->GetMediaMode(cmCapVideo),cmCapVideo);// the default is receive
		pScm->SetMediaMode(m_pIpInitialMode->GetMediaMode(cmCapVideo, cmCapTransmit),cmCapVideo, cmCapTransmit);
	}

	if (m_pIpInitialMode->IsMediaOn(cmCapBfcp, cmCapReceive))
		pScm->SetMediaMode(m_pIpInitialMode->GetMediaMode(cmCapBfcp, cmCapReceive), cmCapBfcp, cmCapReceive);

	if (m_pIpInitialMode->IsMediaOn(cmCapBfcp, cmCapTransmit))
		pScm->SetMediaMode(m_pIpInitialMode->GetMediaMode(cmCapBfcp, cmCapTransmit), cmCapBfcp, cmCapTransmit);

	*m_pIpInitialMode = *pScm;
	m_pIpInitialMode->Dump("***CSipChangeModePartyCntl::ChangeInitialAccordingToNewScm - new m_pIpInitialMode is: ",eLevelInfoNormal);
}

///////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeModeIdle(CSegment* pParam)
{// currently use only in end of connecting to finish the connecting loop
	PTRACEPARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeModeIdle", GetPartyRsrcId());
	BYTE bFirstOffererConnection = FALSE;

	/* MSSlave Flora Question: i think we should set the m_connectingState to IP_CONNECTED for MSSlave here, since for TipSlave, it did not worked here */
	/* The m_connectingState is not correct while deleteing then. */
	/* Log: CSipDelPartyCntl::DisconnectParty: m_connectingState == 2 (PartyName:ctx-9000_2) */
	if (m_bIsOfferer && (m_connectingState == IP_CONNECTING)) // when first invite and offering (invite req or invite ind no sdp) we are still connecting.
	{
		bFirstOffererConnection = TRUE;
		m_connectingState = IP_CONNECTED;
	}

	if ( m_pendingScmForChangeMode )
	{
		TRACEINTO << "changemode. handling pending Scm";
		//since scm is not updated with video streams and operation points, update initial mode with pending scm content only
		m_pIpInitialMode->SetMediaMode(m_pendingScmForChangeMode->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation), cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		m_pIpInitialMode->SetMediaMode(m_pendingScmForChangeMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation), cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
		POBJDELETE(m_pendingScmForChangeMode);
	}

	m_bIsNewScm = FALSE;

	ePartyMediaState mediaState = eConnect;
	
	if (m_pIpInitialMode->GetConfType() == kCop)
		mediaState = DecideOnPartyMediaStateForCOP(bFirstOffererConnection);
	
	BYTE bSetChangeModeStateIfNeeded = TRUE; // If needed set the change mode state because we will start change mode process.
	BYTE bChangeContentNeeded = IsChangeContentNeeded(bSetChangeModeStateIfNeeded);
	
	TRACEINTO << "!@# bChangeContentNeeded:" << (int)bChangeContentNeeded 
		<< " mediaState:" << (int)mediaState;
	
	if (bChangeContentNeeded || mediaState == eChangeMode_Must)
	{
		TRACEINTO << "!@# 1";
		
		if (bChangeContentNeeded)
			PTRACEPARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeModeIdle change content is needed", GetPartyRsrcId());
		else
			PTRACEPARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeModeIdle change decoder", GetPartyRsrcId());
		
		m_changeModeInitiator = eConfInitiator;
		StartTimer(CHANGEMODETOUT, CHANGEMODE_TIME*SECOND);
		ChangeMode(pParam);
	}
	else // Can be at end of first connecting flow - finish the connecting loop.
	{
		TRACEINTO << "!@# 2";
		
		EndChangeMode();
	}
}

///////////////////////////////////////////////////
ePartyMediaState CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP(BYTE bCheckSymmetry)
{
	// Trace:
	CCopLayoutResolutionTable table;
    if(eCop_DecoderParams == m_eLastCopChangeModeType)
        PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP :LastCopDecoderResolution = ", table.GetCopDecoderResolutionStr((ECopDecoderResolution)m_eLastCopChangeModeParam), GetPartyRsrcId());
    else if (eCop_EncoderIndex == m_eLastCopChangeModeType)
    {
        CSmallString str;
        str << "CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP :Encoder Index to follow = " << m_eLastCopChangeModeParam;
        PTRACEPARTYID(eLevelInfoNormal, str.GetString(), GetPartyRsrcId());
    }

    // Check change decoder:
	ePartyMediaState retMediaState = eConnect;
	BYTE bCheckChangeDecoder = FALSE;
	if (m_eLastCopChangeModeType == eCop_DecoderParams && (ECopDecoderResolution)m_eLastCopChangeModeParam < COP_decoder_resolution_Last)
	{
		m_pIpInitialMode->SetVideoRxModeAccordingDecoderResolution((ECopDecoderResolution)m_eLastCopChangeModeParam, m_eFirstRxVideoCapCode,m_eFirstRxVideoProfile);
        CapEnum capCode = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive));
        DWORD videoRateToSet = m_pSipLocalCaps->GetMaxVideoBitRate();
        m_pIpInitialMode->SetVideoBitRate(videoRateToSet, cmCapReceive, kRolePeople);
       // SetInitialRecRateAccordingToRes((ECopDecoderResolution)m_eLastCopChangeModeParam,videoRateToSet);
        if(m_pIpInitialMode->GetCopTxLevel()< NUMBER_OF_COP_LEVELS && m_pIpInitialMode->GetCopTxLevel() != INVALID_COP_LEVEL && m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) )
        {
        	DWORD levelvideoraterate = (m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRolePeople) ) / 10;
        	DWORD audioRateAccordingToLevel = CalculateAudioRateAccordingToVideoRateOfCopLevel(levelvideoraterate);
        	//audioRateAccordingToLevel = audioRateAccordingToLevel *10;
        	DWORD maxLevelConfRate = levelvideoraterate + audioRateAccordingToLevel;
        	DWORD RecCallRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRolePeople) /10  + m_pIpInitialMode->GetMediaBitRate(cmCapAudio, cmCapReceive, kRolePeople);
        	if(RecCallRate > maxLevelConfRate)
        	{
        		PTRACE2INT(eLevelInfoNormal,"CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP: cop rate exceed change in AUDIO is ",m_pIpInitialMode->GetMediaBitRate(cmCapAudio, cmCapReceive, kRolePeople));
        		DWORD newVideoRateForRx = maxLevelConfRate - audioRateAccordingToLevel;
        		newVideoRateForRx = newVideoRateForRx *10;
        		PTRACE2INT(eLevelInfoNormal,"CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP: cop rate exceed newVideoRateForRx ",newVideoRateForRx);
        		m_pIpInitialMode->SetVideoBitRate(newVideoRateForRx,cmCapReceive);

        	}
        }
        bCheckChangeDecoder = TRUE;
    }
    else if (m_eLastCopChangeModeType == eCop_EncoderIndex)
    {
        BYTE encoderIndex = m_eLastCopChangeModeParam;
        CVidModeH323* pVidMode = m_pCopVideoTxModes->GetVideoMode(encoderIndex);
		if (pVidMode && pVidMode->IsMediaOn())
        {
            if (pVidMode->GetType() == eH264CapCode)
            {
                CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
                if (!pConfParty)
                {
                    PTRACE(eLevelError,"CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP - pConfParty is NULL");
                    DBGPASSERT(1126);
                    return retMediaState;
                }

                CCOPConfigurationList* pCOPConfigurationList = m_pConf->GetCommConf()->GetCopConfigurationList();
                if (!pCOPConfigurationList)
                {
                	TRACEINTO << "pCOPConfigurationList is NULL";
                    DBGPASSERT(1);
                    return retMediaState;
                }

                CCopVideoParams* pCopLevelParams = pCOPConfigurationList->GetVideoMode(encoderIndex);
                if (!pCopLevelParams)
                {
                	TRACEINTO << "pCopLevelParams is NULL";
                    DBGPASSERT(2);
                    return retMediaState;
                }

                // Change the scm receive video:
                sCopH264VideoMode copH264VideoMode;
                CCopVideoModeTable* pCopTable = new CCopVideoModeTable;
                APIU16 profile =GetProfileAccordingToCopProtocol(pCopLevelParams->GetProtocol());
                pCopTable->GetSignalingH264ModeAccordingToReservationParams(pCopLevelParams, copH264VideoMode, TRUE, pConfParty->GetVideoRate());
                m_pIpInitialMode->SetH264Scm(profile, copH264VideoMode.levelValue, copH264VideoMode.maxMBPS, copH264VideoMode.maxFS, copH264VideoMode.maxDPB, copH264VideoMode.maxBR, H264_ALL_LEVEL_DEFAULT_SAR, copH264VideoMode.maxStaticMbps, cmCapReceive);
                POBJDELETE(pCopTable);
                DWORD videoRateToSet = pVidMode->GetBitRate();
                m_pIpInitialMode->SetVideoBitRate(videoRateToSet, cmCapReceive, kRolePeople);
            }
            else
            	m_pIpInitialMode->SetMediaMode(*pVidMode, cmCapVideo, cmCapReceive, kRolePeople);
            m_pIpInitialMode->SetDirection(cmCapVideo, cmCapReceive, kRolePeople);
            bCheckChangeDecoder = TRUE;
        }
        else
            PASSERTMSG(GetPartyRsrcId(),"CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP -Failed to set receive mode according to tx level");
    }
    else if (bCheckSymmetry)
    {
    	if (((CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople) == eH264CapCode)
    		&& ((CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople) == eH263CapCode)
			&& m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople)
    		&& m_pSipLocalCaps && m_pSipLocalCaps->IsCapSet(eH263CapCode))
		{
    		const capBuffer* newMediaMode = m_pSipLocalCaps->GetCapSetAsCapBuffer(eH263CapCode);
			if (newMediaMode)
			{
				PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP: Change receive mode to h263");
				m_pIpInitialMode->SetMediaMode(newMediaMode, cmCapVideo, cmCapReceive, kRolePeople);
				bCheckChangeDecoder = TRUE;
				m_eFirstRxVideoCapCode = (CapEnum)(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople));
				if(m_eFirstRxVideoCapCode == eH264CapCode)
					m_eFirstRxVideoProfile = m_pIpInitialMode->GetH264Profile(cmCapReceive);
				PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP : m_eFirstRxVideoCapCode = ", m_eFirstRxVideoCapCode);
				PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP : m_eFirstRxVideoProfile = ", m_eFirstRxVideoProfile);
			}
		}
    }
    else
		PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::DecideOnPartyMediaStateForCOP :  m_eLastCopDecoderResolution still hasn't initiated");

    if (bCheckChangeDecoder)
    {
        const CVidModeH323 initRecvVideo = (const CVidModeH323 &)m_pIpInitialMode->GetMediaMode(cmCapVideo,cmCapReceive);
		const CVidModeH323 currRecvVideo = (const CVidModeH323 &)m_pIpCurrentMode->GetMediaMode(cmCapVideo,cmCapReceive);
		if (!(initRecvVideo == currRecvVideo))
		{
			retMediaState = eChangeMode_Must;
			m_eChangeModeState = eChangeIncoming;
		}
    }

	return retMediaState;
}
///////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeMode(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeMode", GetPartyRsrcId());
	// when we receive a request from the conference, or cop video bridge, to change mode
	// if its because of EP re-cap or because of conf HC (currently not supported) or cop change decoder we need to do
	// (1) Set the Initial mode according to the new scm from conf (or cop video bridge)
	// (2) Update the bridges open parameters.
	// (3) Connect or disconnect bridges according to the new mode.
	// (4) Inform the party - 2 options:
	//                                 1. Recap case - Send bridges updated to Party. Always continue to (5).
	//                                 2. Conf Change mode case (currently only Cop change decoder) - Send Conf Change Mode to Party.
	//									  This flow can continue to (5), or start Re-Cap flow (1) as a result of 200ok from EP.
	// (5) When we receive response from the party to End the change mode.

	// (2)
	m_state 			= CHANGE_BRIDGES;
	m_eUpdateState 		= eNoUpdate;

	BYTE bTakeInitial 	= TRUE;

	m_eUpdateState 		= UpdateAudioAndVideoBridgesIfNeeded(bTakeInitial);

	SetInitialVideoRxInCurrentIfNeeded();

	if((m_eUpdateState & eUpdateVideoIn) || (m_eUpdateState & eUpdateVideoOut))
	{// audio bridge has no response on update, wait for video bridge response
		PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeMode: (Bridge mode was different) Name ",m_partyConfName, GetPartyRsrcId());
	}
	else
	{
		// (3)
		ChangeModeChangeVideoAndDataBridgesState();
	}
}

///////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeModeChangeVideoAndDataBridgesState()
{
	BYTE bIsChangeVideoBridge = ChangeVideoBridgeStateAccordingToNewMode();
	BYTE bIsChangeDataBridge  = ChangeDataBridgeStateAccordingToNewMode();

	BYTE bSetChangeModeStateIfNeeded = FALSE; // Don't set change mode state when calling IsChangeContentNeeded function, because it will not start a change mode process, it is only a part of Re-Cap process.
	if (m_eChangeModeState == eChangeContentInAndOut || m_eChangeModeState == eChangeContentRate || IsChangeContentNeeded(bSetChangeModeStateIfNeeded))
		ChangeContentBridgeStateAccordingToNewMode();

	if (bIsChangeVideoBridge /*|| bIsChangeDataBridge  currently the FECC is stateless in terms of call flow*/)
	{// no change in video bridge state, continue the flow by sending the ACK to the party
		PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeModeChangeVideoAndDataBridgesState: connect or disconnect from video or data bridge. Wait for response. Name - ",m_partyConfName, GetPartyRsrcId());
	}
	else
	{
		// (4)
		ChangeModeInformParty();
	}
}

///////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeModeInformParty()
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeModeInformParty: ", m_partyConfName, GetPartyRsrcId());

	PTRACE2INT(eLevelInfoNormal,"***CSipChangeModePartyCntl::ChangeModeInformParty: Change Mode Initiator: ", m_changeModeInitiator);

	BYTE bIsContentSpeaker = (m_pContentBridge && m_pContentBridge->IsTokenHolder(m_pParty)) ? TRUE : FALSE;

	if (m_changeModeInitiator == eConfInitiator)
	{
		if (m_bStartRecapAfterChangeBridges)
		{
			// Re-Cap (Reinvite-Ind) arrived during change bridges for change decoder - suspend the change decoder mode and handle the Re-Cap first.
			StartReCaps();
			m_bStartRecapAfterChangeBridges = FALSE;
		}
		else
		{
			m_state = CONF_REQUEST_PARTY_CHANGE_MODE;
			CMedString str;
			str << "Send Change Mode To Party : Name - " << m_partyConfName << " Change mode state - " << GetChangeModeStateStr(m_eChangeModeState) << ", Content Speaker - " << bIsContentSpeaker;
			PTRACE2PARTYID(eLevelInfoNormal,"***CSipChangeModePartyCntl::ChangeModeInformParty - ",str.GetString(), GetPartyRsrcId());

			m_pPartyApi->ChangeModeIp(m_pIpInitialMode, m_eChangeModeState, bIsContentSpeaker,NULL,NULL,NULL,m_IsAsSipContentEnable);
		}
	}
	else if (m_changeModeInitiator == ePartyInitiator)
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeModeInformParty: Initiator == Party, Name: ",m_partyConfName, GetPartyRsrcId());
		PTRACE2INT(eLevelInfoNormal,"AN DEBUG - CSipChangeModePartyCntl::ChangeModeInformParty: is BFCP:", m_pIpInitialMode->IsMediaOn(cmCapBfcp));
		m_state = CONF_RESPONSE_PARTY_RE_CAPS;

		//escalation
		CRsrcParams *pMrmpRsrcParams = new CRsrcParams;
        WORD found = m_pPartyAllocatedRsrc->GetRsrcParams(*pMrmpRsrcParams, eLogical_relay_rtp); // eyaln
        if (!found)
        {
            POBJDELETE(pMrmpRsrcParams);
        }

        CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
        WORD itemNum = 1;
        for (int i=0; i < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++i)
        {
            avcToSvcTranslatorRsrcParams[i] = new CRsrcParams;
            if (i == 0)
                found = m_pPartyAllocatedRsrc->GetRsrcParams(*avcToSvcTranslatorRsrcParams[i], eLogical_relay_avc_to_svc_rtp_with_audio_encoder, itemNum);
            else
                found = m_pPartyAllocatedRsrc->GetRsrcParams(*avcToSvcTranslatorRsrcParams[i], eLogical_relay_avc_to_svc_rtp, itemNum);
            if (!found)
            {
                TRACEINTO << "Internal ART #" << i << " not found.";
                POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
            }
        }

		m_pPartyApi->BridgesUpdated(m_pIpInitialMode,m_udpAddresses,STATUS_OK, bIsContentSpeaker, TRUE, avcToSvcTranslatorRsrcParams, pMrmpRsrcParams ,m_bShouldNotifyPartyOnSecondaryContent);
        POBJDELETE(pMrmpRsrcParams);
        for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
        {
            POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
        }

		m_bShouldNotifyPartyOnSecondaryContent = FALSE;
	}
	else if(m_changeModeInitiator == eFallBackFromIce)
	{
		m_state = IDLE;
		m_pPartyApi->ChangeModeIp(m_pIpInitialMode, eFallBackFromIceToSip, NO/*content speaker*/,NULL,NULL,NULL,m_IsAsSipContentEnable);
	}
	else if(m_changeModeInitiator == eFallBackFromTip)
	{
		if(m_bRestartSipfromTipToNonTip)
		{
			ChangeSipfromTipToNonTip(m_pIpInitialMode, m_pSipLocalCaps);
			m_bRestartSipfromTipToNonTip = FALSE;
		}
		else
		{
			m_state = IDLE;
			m_pPartyApi->ChangeModeIp(m_pIpInitialMode, eFallBackFromTipToSip, NO/*content speaker*/, m_pSipLocalCaps,NULL,NULL,m_IsAsSipContentEnable);
		}
		//we  got new inital new caps
	}

	else
		DBGPASSERT(GetPartyRsrcId());
}

///////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnConfChangeModeIdle(CSegment* pParam)
{
#ifdef PERFORMANCE_TEST
	m_Stopper.AddTime();
#endif //PERFORMANCE_TEST

	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnConfChangeModeIdle: Name - ",m_partyConfName, GetPartyRsrcId());
	ChangeModeIdle(pParam);
}

///////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnConfChangeModePartyReCaps(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnConfChangeModePartyReCaps: Name - ",m_partyConfName, GetPartyRsrcId());
	/* Only if it is change scm from conf, it is part of Re-Cap process and we continue the Re-Cap process.
	 * Otherwise, is is not part of Re-Cap process, so we do nothing and the change mode will be done after finishing current change mode.
	 */
	if (m_bPartyControlChangeScm == FALSE) // It is change mode from conf, as a part of Re-Cap process.
		ChangeMode(pParam);
}


/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnVideoBrdgDisconnectedChangeBridges(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::OnVideoBrdgDisconnectedChangeBridges : Name - ", m_partyConfName, GetPartyRsrcId());

	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);
        TRACEINTO<<"!@# 1 resStat:"<<(int)resStat;
	if (resStat == statVideoInOutResourceProblem)
	{
		DeleteTimer(CHANGEMODETOUT);
		BYTE 	mipHwConn = (BYTE)eMipBridge;
		BYTE	mipMedia = (BYTE)eMipVideo;
		BYTE	mipDirect = 0;
		BYTE	mipTimerStat = 0;
		BYTE	mipAction = 0;
		*pParam >>  mipDirect >> mipTimerStat >> mipAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;

		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty,NULL,MpiErrorNumber);
		POBJDELETE(pSeg);
                TRACEINTO<<"!@# 2 resStat:"<<(int)resStat;
		return;
	}

	ChangeModeInformParty();
}
/////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnVideoBrdgDisconnectedReallocRsrc(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::OnVideoBrdgDisconnectedReallocRsrc : Name - ", m_partyConfName, GetPartyRsrcId());

	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);
	if (resStat == statVideoInOutResourceProblem)
	{
		DeleteTimer(CHANGEMODETOUT);
		BYTE 	mipHwConn = (BYTE)eMipBridge;
		BYTE	mipMedia = (BYTE)eMipVideo;
		BYTE	mipDirect = 0;
		BYTE	mipTimerStat = 0;
		BYTE	mipAction = 0;
		*pParam >>  mipDirect >> mipTimerStat >> mipAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;

		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty,NULL,MpiErrorNumber);
		POBJDELETE(pSeg);
		return;
	}

	eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode,TRUE);

	DWORD artCapacity = 0;
	artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, TRUE /*add audio + video for current*/);
	m_artCapacity = artCapacity;

	// fix for TIP split DSP's
	BYTE bChangeResources = IsNeedToChangeVideoResourceAllocation(eCurrentVideoType);

	PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::OnVideoBrdgDisconnectedReallocRsrc bChangeResources: ", bChangeResources);

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);

	if (bChangeResources)
		CreateAndSendReAllocatePartyResources(eIP_network_party_type, eCurrentVideoType, eAllocateAllRequestedResources,FALSE,0,m_pSIPNetSetup->GetEnableSipICE(),artCapacity);
	else
		DBGPASSERT(8);
}

/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnVideoBrdgConnectedChangeBridges(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnVideoBrdgConnectedChangeBridges: Name - ",m_partyConfName, GetPartyRsrcId());
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnVideoBrdgConnectedChangeBridges : Connect has received after disconnect. Name - ",m_partyConfName, GetPartyRsrcId());
	}

	else
	{
		HandleVideoBridgeConnectedInd(pParam);
		if( m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople) )
		{
			if( AreTwoDirectionsConnectedToVideoBridge())
				ChangeModeInformParty();
		}
		else if( IsAtLeastOneDirectionConnectedToVideoBridge() )
			ChangeModeInformParty();

	}
}


////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::HandleVideoBridgeUpdate(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges)
{
	BYTE bVideoBridgeEndUpdate = CheckVideoBridgeEndUpdate(status, eUpdatedBridges);
	if (bVideoBridgeEndUpdate)
	{
		ChangeModeChangeVideoAndDataBridgesState();
	}
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyReCapsChangeBridges(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyReCapsChangeBridges: Name - ",m_partyConfName, GetPartyRsrcId());
	// Re-Cap received during change bridges. It can be when during change bridges for change decoder mode, a ReInvite-Ind is received.
	// We need to suspend the change mode after bridges change will be finished, and start handling the Re-Cap. In order to start again the change decoder, after the Re-Cap process, we set m_bIsNewScm to true.

	m_bIsNewScm = TRUE;

	if( m_changeModeInitiator == eFallBackFromTip )
	{
		HandlePartyReCapParamsInFallback(pParam);
		m_bRestartSipfromTipToNonTip = TRUE;
	}
	else
	{
		HandlePartyReCapsParams(pParam);
		m_bStartRecapAfterChangeBridges = TRUE;
	}
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyReCapsChannelHandle(CSegment* pParam)
{
	TRACEINTO << "Name - " << m_partyConfName << "Id=" << GetPartyRsrcId();

	// Re-Cap received during change bridges for ChannelHandle. It can be when during change bridges for change decoder mode, a ReInvite-Ind is received.
	// We need to suspend the change mode after bridges change will be finished, and start handling the Re-Cap. In order to start again the change decoder, after the Re-Cap process, we set m_bIsNewScm to true.

	m_bIsNewScm = TRUE;

	if( m_changeModeInitiator == eFallBackFromTip )
	{// @#@#@#@#@#@#@
		HandlePartyReCapParamsInFallback(pParam);
		m_bRestartSipfromTipToNonTip = TRUE;
	}
	else
	{
		HandlePartyReCapsParams(pParam);
		m_bStartRecapAfterUpdateChannelHandle = TRUE;
	}
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyReCapsIdle(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyReCapsIdle: Name - ",m_partyConfName, GetPartyRsrcId());
	HandlePartyReCapsParams(pParam);
	StartReCaps();
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyDisconnectBridgesIdle(CSegment* pParam)
{
	WORD isDisconnectAudio = TRUE,
		 isDisconnectVideo = TRUE;
	*pParam >> isDisconnectAudio
			>> isDisconnectVideo;

	TRACEINTO << "multi_line - start disconnect bridges - audio " << (isDisconnectAudio ? "needed" : "not needed")
			  << ", video " << (isDisconnectVideo ? "needed" : "not needed")
			  << ", content " << (m_isContentConn ? "needed" : "not needed")
			  << ", fecc " << (m_isFeccConn ? "needed" : "not needed");

	m_state = DISCONNECT_BRIDGES;

	// 1. audio
	if (TRUE == isDisconnectAudio)
	{
		DisconnectPartyFromAudioBridge();
	}
	else
	{
		m_bIsBridgeDisconnectedAudio = true;
	}

	// 2. video
	if (TRUE == isDisconnectVideo)
	{
//	    m_incomingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
//	    m_outgoingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
//	    TRACEINTO << "ibm-492, m_incomingVideoChannelHandle: " << m_incomingVideoChannelHandle << ", m_outgoingVideoChannelHandle: " << m_outgoingVideoChannelHandle;

	    DisconnectPartyFromVideoBridge();
	}
	else
	{
		m_bIsBridgeDisconnectedVideo = true;
	}

	// 3. content
	if (m_isContentConn)
	{
		DisconnectPartyFromContentBridge();
	}
	else
	{
		m_bIsBridgeDisconnectedContent = true;
	}

	// 4. fecc
	if (m_isFeccConn)
	{
		DisconnectPartyFromFECCBridge();
	}
	else
	{
		m_bIsBridgeDisconnectedFecc = true;
	}
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnAudioBrdgDisconnected(CSegment* pParam)
{
	HandleAudioBridgeDisconnectedInd(pParam);

	m_bIsBridgeDisconnectedAudio = true;
	CheckAreAllBridgesDisconnected();
}

////////////////////////////////////////////////////////////
int CSipChangeModePartyCntl::checkShouldDisconnectCall(CSegment* pParam)
{
    int action=0;
    CSegment* pseg=new CSegment(*pParam);
    WORD status;
    *pseg >> status;
    EMediaDirection eVidBridgeConnState = eNoDirection;
    *pseg >> (WORD&)eVidBridgeConnState;
     EStat resStat = (EStat)status;
     TRACEINTO<<"!@# m_bVideoUpdateCount:"<<(int)m_bVideoUpdateCount<<" resStat:"<<(int)resStat;
     if (resStat!=statOK && m_pIpInitialMode->GetConfMediaType()==eMixAvcSvc)
     {
//          if(m_bIsMrcCall && m_bVideoUpdateCount==VIDEO_UPDATE_COUNT0)
//	  {
//	      TRACEINTO<<"!@# m_bVideoUpdateCount: disconnecting";
//	      m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL/*,MpiErrorNumber*/);
//	      action=1;
//	  }
	  /*	  if(!m_bIsMrcCall && m_bVideoUpdateCount==VIDEO_UPDATE_COUNT0)
	  {
	      TRACEINTO<<"!@# m_bVideoUpdateCount: disconnecting";
	      m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL);
	      action=1;
	  }*/
      }


    POBJDELETE(pseg);

    return action;
}
void CSipChangeModePartyCntl::OnVideoBrdgDisconnected(CSegment* pParam)
{
        checkShouldDisconnectCall(pParam);

	HandleVideoBridgeDisconnectedInd(pParam);

	m_bIsBridgeDisconnectedVideo = true;
	CheckAreAllBridgesDisconnected();
}

////////////////////////////////////////////////////////////
int CSipChangeModePartyCntl::OnContentBrdgDisconnected(CSegment* pParam)
{
	CPartyCntl::OnContentBrdgDisconnected(pParam);

	m_bIsBridgeDisconnectedContent = true;
	CheckAreAllBridgesDisconnected();

	return 0;
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnFeccBrdgDisconnected(CSegment* pParam)
{
	CIpPartyCntl::OnFeccBridgeDisConnect(pParam);

	m_bIsBridgeDisconnectedFecc = true;
	CheckAreAllBridgesDisconnected();
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::CheckAreAllBridgesDisconnected()
{
	TRACEINTO << "multi_line - aud: " << (m_bIsBridgeDisconnectedAudio ? "yes" : "no")
			  << ", vid: " << (m_bIsBridgeDisconnectedVideo ? "yes" : "no")
			  << ", content: " << (m_bIsBridgeDisconnectedContent ? "yes" : "no")
			  << ", fecc: " << (m_bIsBridgeDisconnectedFecc ? "yes" : "no")
			  << ", m_state: " << m_state;


	if ( (true == m_bIsBridgeDisconnectedAudio) &&
		 (true == m_bIsBridgeDisconnectedVideo) &&
		 (true == m_bIsBridgeDisconnectedContent) &&
		 (true == m_bIsBridgeDisconnectedFecc) )
	{
		TRACEINTO << "multi_line - all needed bridges are disconnected";

		m_bIsBridgeDisconnectedAudio	= false;
		m_bIsBridgeDisconnectedVideo	= false;
		m_bIsBridgeDisconnectedContent	= false;
		m_bIsBridgeDisconnectedFecc		= false;

		m_pPartyApi->SendBridgesDisconnected();
		//m_state = IDLE;
	}
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyRemoteConnectedDisconnectBridges(CSegment* pParam)
{
	TRACEINTO;

	EndChangeMode();
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyConnectBridgesIdle(CSegment* pParam)
{
	bool bContentNeeded	= (bool)( m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation) ),
		 bFeccNeeded	= (bool)m_isFeccConn;

	WORD isConnectAudio = TRUE,
		 isConnectVideo = TRUE;

	*pParam >> isConnectAudio
			>> isConnectVideo;

	m_pIpInitialMode->DeSerialize(NATIVE,*pParam);
	m_pSipRemoteCaps->DeSerialize(NATIVE,*pParam);

	unsigned int incomingVideoChannelHandle;
	unsigned int outgoingVideoChannelHandle;
	*pParam >> incomingVideoChannelHandle;
	*pParam >> outgoingVideoChannelHandle;

	if (m_incomingVideoChannelHandle != incomingVideoChannelHandle)
	{
		m_incomingVideoChannelHandle = incomingVideoChannelHandle;
		m_bVideoRelayInReady = FALSE;
	}
	if (m_outgoingVideoChannelHandle != outgoingVideoChannelHandle)
	{
		m_outgoingVideoChannelHandle = outgoingVideoChannelHandle;
		m_bVideoRelayOutReady = FALSE;
	}

	TRACEINTO << "multi_line - start connect bridges - audio " << (isConnectAudio ? "needed" : "not needed")
			  << ", video " << (isConnectVideo ? "needed" : "not needed")
			  << ", content " << (bContentNeeded ? "needed" : "not needed")
			  << ", fecc " << (bFeccNeeded ? "needed" : "not needed")
			  << ", incomingVideoChannelHandle: " << m_incomingVideoChannelHandle
			  << ", outgoingVideoChannelHandle: " << m_outgoingVideoChannelHandle;

	m_state = CONNECT_BRIDGES;


//	TRACEINTO << "!@# multi_line - m_pIpInitialMode address: " << m_pIpInitialMode;
//	m_pIpInitialMode->Dump("!@# multi_line - m_pIpInitialMode ", eLevelInfoNormal);

	// temp temp temp
	bContentNeeded=false;

	// 1. audio
	if (isConnectAudio)
	{
		ConnectPartyToAudioBridge(m_pIpInitialMode);
	}
	else
	{
		m_bIsBridgeConnectedAudio = true;
	}

	// 2. video
	if (isConnectVideo)
	{
		ConnectPartyToVideoBridge(m_pIpInitialMode);
	}
	else
	{
		m_bIsBridgeConnectedVideo = true;
	}

	// 3. content
	if (bContentNeeded)
	{
		ConnectPartyToContentBridge();
	}
	else
	{
		m_bIsBridgeConnectedContent = true;
	}

	// 4. fecc
	if (bFeccNeeded)
	{
		ConnectPartyToFECCBridge(m_pIpInitialMode);
	}
	else
	{
		m_bIsBridgeConnectedFecc = true;
	}
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnAudioBrdgConnectedConnectBridges(CSegment* pParam)
{
	m_bIsBridgeConnectedAudio = true;
	HandleAudioBridgeConnectedInd(pParam);
	CheckAreAllBridgesConnected();
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnVideoBrdgConnectedConnectBridges(CSegment* pParam)
{
	TRACEINTO<<"!@# ";
	m_bIsBridgeConnectedVideo = true;
	HandleVideoBridgeConnectedInd(pParam);
	CheckAreAllBridgesConnected();
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnContentBrdgConnectedConnectBridges(CSegment* pParam)
{
	m_bIsBridgeConnectedContent = true;
	CPartyCntl::OnContentBrdgConnected(pParam);
	CheckAreAllBridgesConnected();
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnFeccBrdgConnectedConnectBridges()
{
	m_bIsBridgeConnectedFecc = true;

	CheckAreAllBridgesConnected();
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::CheckAreAllBridgesConnected()
{
	TRACEINTO << "multi_line - aud: " << (m_bIsBridgeConnectedAudio ? "yes" : "no")
			  << ", vid: " << (m_bIsBridgeConnectedVideo ? "yes" : "no")
			  << ", content: " << (m_bIsBridgeConnectedContent ? "yes" : "no")
			  << ", fecc: " << (m_bIsBridgeConnectedFecc ? "yes" : "no")
			  << ", m_state: " << m_state;


	if ( (true == m_bIsBridgeConnectedAudio) &&
		 (true == m_bIsBridgeConnectedVideo) &&
		 (true == m_bIsBridgeConnectedContent) &&
		 (true == m_bIsBridgeConnectedFecc) )
	{
		TRACEINTO << "multi_line - all needed bridges are connected";

		m_bIsBridgeConnectedAudio	= false;
		m_bIsBridgeConnectedVideo	= false;
		m_bIsBridgeConnectedContent	= false;
		m_bIsBridgeConnectedFecc	= false;

		*m_pIpCurrentMode=*m_pIpInitialMode; //in order to end the change mode

//	    if(m_bIsMrcCall && m_bVideoUpdateCount<VIDEO_UPDATE_COUNT1)
//	    {
//	        m_bVideoUpdateCount++;
//	        TRACEINTO<<"!@# updating m_bVideoUpdateCount to: "<<(int)m_bVideoUpdateCount;
//	    }

		m_pPartyApi->SendBridgesConnected();
		//m_state = IDLE;
	}
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyRemoteConnectedConnectBridges(CSegment* pParam)
{
	EndChangeMode();
}

////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::HandlePartyReCapsParams(CSegment* pParam)
{
	BYTE isRecaps = 0, isBestMode = 0;

	*pParam >> isRecaps;
	PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::HandlePartyReCapsParams m_pSipRemoteCaps->IsCapableTipAux5Fps ", m_pSipRemoteCaps->IsCapableTipAux5Fps());
	if(isRecaps)
		m_pSipRemoteCaps->DeSerialize(NATIVE,*pParam);
	PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::HandlePartyReCapsParams : after deserialize - m_pSipRemoteCaps->IsCapableTipAux5Fps ", m_pSipRemoteCaps->IsCapableTipAux5Fps());
	*pParam >> isBestMode;
	if(isBestMode)
		m_pIpInitialMode->DeSerialize(NATIVE,*pParam);

	WORD isTipFallBack = FALSE;
	*pParam >> isTipFallBack;

	if( isTipFallBack )
	    m_bIsTipCall = FALSE;

	//Check upgrade from voice only (re-invite case for undefined parties)
	if(IsUndefinedParty())
	{
		BYTE bIsCurrPartyVoice = FALSE;
		CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
		if (pConfParty)
			bIsCurrPartyVoice = pConfParty->GetVoice();

		if (bIsCurrPartyVoice)
		{
			if ((m_pSipRemoteCaps->GetNumOfMediaCapSets(cmCapVideo) > 0) && (m_pSipLocalCaps->GetNumOfMediaCapSets(cmCapVideo) > 0))
			{
				//PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyReCapsIdle: reseting type to video - ",m_partyConfName, GetPartyRsrcId());
				if(YES == m_voice)
					m_isAudioDecoderUpdateNeeded = eMediaTypeUpdateAudioToVideo;
				m_voice = NO;
				m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_RESET_STATUS);
			}
		}
	}
	if( m_pIpInitialMode->IsMediaOn(cmCapBfcp) && !m_pSipLocalCaps->IsBfcpSupported() )
	{
		PTRACE(eLevelInfoNormal, "CSipChangeModePartyCntl::HandlePartyReCapsParams - adding BFCP and content to local");
		m_pSipLocalCaps->SetBfcp(m_pIpInitialMode, m_name);
		m_pSipLocalCaps->SetContent(m_pIpInitialMode, m_name);
		m_pSipLocalCaps->SetTipAuxFPS(m_pIpInitialMode->GetTipAuxFPS());
	}
    if( m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePresentation) && !m_pSipLocalCaps->GetNumOfMediaCapSets(cmCapVideo,cmCapReceiveAndTransmit, kRolePresentation) )
    {
           PTRACE(eLevelInfoNormal, "CSipChangeModePartyCntl::HandlePartyReCapsParams - adding content to local");
           m_pSipLocalCaps->SetContent(m_pIpInitialMode, m_name);
           m_pSipLocalCaps->SetTipAuxFPS(m_pIpInitialMode->GetTipAuxFPS());
    }
}
////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnUpdateRemoteCapsFromParty(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::OnUpdateRemoteCapsFromParty m_pSipRemoteCaps->IsCapableTipAux5Fps ", m_pSipRemoteCaps->IsCapableTipAux5Fps());
	m_pSipRemoteCaps->DeSerialize(NATIVE,*pParam);
	PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::OnUpdateRemoteCapsFromParty m_pSipRemoteCaps->IsCapableTipAux5Fps ", m_pSipRemoteCaps->IsCapableTipAux5Fps());
}
void CSipChangeModePartyCntl::StartReCaps()
{
	StartTimer(CHANGEMODETOUT, CHANGEMODE_TIME*SECOND);

	m_state = PARTY_RE_CAPS;
	m_changeModeInitiator = ePartyInitiator;

	BYTE bIsNeedToChangeAudio = ChangeAudioBridgeStateAccordingToNewMode();

	BYTE bIsNeedToChangeVideoRsrc = ChangeVideoBrdgRsrcIfNeeded();//DPA
	BYTE bIsNeedToRemoveContetnt = IsLegacyContentParty();
	BYTE bIsNeedToChangeVideoLegacy = FALSE;

	if (bIsNeedToRemoveContetnt)
	{

	    if (bIsNeedToRemoveContetnt & eContentSecondaryCauseBelowRate)
	    {
			 PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::StartReCaps: Setting Cause (Content rate is below minimum required threshold)!");
			 SetPartySecondaryCause(SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD);
	    }
	    else if(bIsNeedToRemoveContetnt & eContentSecondaryCauseBelowResolution)
	    {
			 PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::StartReCaps: Setting Cause (Content resolution is below minimum required threshold");
			 SetPartySecondaryCause(SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD);
	    }


		if (m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePresentation) ||
			m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePresentation))
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::StartReCaps : Legacy content need to close content and bfcp channels Name - ", m_partyConfName, GetPartyRsrcId());
			m_bShouldNotifyPartyOnSecondaryContent = TRUE;
			m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
			// AN - core dump when sending re-INVITE after secondary content
			m_pIpInitialMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		}
	}
	else
	{
		//we now changed from legacy to content
		if (TRUE == m_bIsLegacyContent && !IsTipSlavePartyType() &&(!m_bIsLync))
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::StartReCaps : Change from legacy to content Name - ", m_partyConfName, GetPartyRsrcId());
			bIsNeedToChangeVideoLegacy = TRUE;
			m_bIsLegacyContent = FALSE;
		}
	}


	if (!bIsNeedToChangeVideoRsrc)
		bIsNeedToChangeVideoLegacy = DisconnectForUpdateLegacyStatusIfNeeded(bIsNeedToChangeVideoLegacy);

	TRACEINTO << "bIsNeedToChangeAudio: " << (bIsNeedToChangeAudio? "yes" : "no")
			  << ", bIsNeedToChangeVideoRsrc: " << (bIsNeedToChangeVideoRsrc? "yes" : "no")
			  << ", bIsNeedToChangeVideoLegacy: " << (bIsNeedToChangeVideoLegacy? "yes" : "no")
			  << ", bIsLegacyContent: " << (m_bIsLegacyContent? "yes" : "no");

	if (bIsNeedToChangeAudio == FALSE && bIsNeedToChangeVideoRsrc == FALSE && bIsNeedToChangeVideoLegacy == FALSE)
	{// no change in audio bridge state, continue the flow by sending reques to the conf.
		m_pTaskApi->InformConfOnPartyReCap(m_pParty);
	}
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipChangeModePartyCntl::DisconnectForUpdateLegacyStatusIfNeeded(BYTE bIsNeedToChangeVideoLegacy)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::DisconnectForUpdateLegacyStatusIfNeeded : Name - ", m_partyConfName, GetPartyRsrcId());

	if((m_pIpInitialMode->GetConfMediaType()==eMixAvcSvcVsw) || m_bIsMrcCall || m_bIsLync)
	{
		return FALSE;
	}
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	if( pCommConf && !pCommConf->IsLegacyShowContentAsVideo() )
	{
		PTRACE(eLevelInfoNormal, "CSipChangeModePartyCntl::DisconnectForUpdateLegacyStatusIfNeeded - conf without legacy");
		return FALSE;
	}

	if ( bIsNeedToChangeVideoLegacy ||
		(pCommConf && pCommConf->IsLegacyShowContentAsVideo() && !IsTipSlavePartyType() && m_pIpInitialMode->IsTipNegotiated() && !m_pIpCurrentMode->IsTipNegotiated()))
	{
		CIpComMode* pTmpScm = new CIpComMode;

		*pTmpScm = *m_pIpInitialMode;

		pTmpScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);

		BYTE bIsDisconnectFromVideoBridge = DisconnectPartyFromVideoBridgeIfNeeded(pTmpScm);

		POBJDELETE(pTmpScm);


		if (bIsDisconnectFromVideoBridge)
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::DisconnectForUpdateLegacyStatusIfNeeded : updating video brdige, legacy to content Name - ", m_partyConfName, GetPartyRsrcId());
			m_state = UPDATE_LEGACY_STATUS;
			return TRUE;
		}
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus : Name - ", m_partyConfName, GetPartyRsrcId());

	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);

	m_state = PARTY_RE_CAPS;
	m_pTaskApi->InformConfOnPartyReCap(m_pParty);
}

/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnAudioBrdgDisconnectedPartyReCaps(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::OnAudioBrdgDisconnectedPartyReCaps : Name - ", m_partyConfName, GetPartyRsrcId());
	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam);
	if (bIsDisconnectOk == FALSE)
	{
		DeleteTimer(CHANGEMODETOUT);
		DWORD MpiErrorNumber = GetMpiErrorNumber(pParam);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL,MpiErrorNumber);
		return;
	}
	else if(m_state != REALLOCATE_RSC)
		m_pTaskApi->InformConfOnPartyReCap(m_pParty);
	else
		PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnAudioBrdgDisconnectedPartyReCaps the reallocation will return answer to party : Name - ",m_partyConfName, GetPartyRsrcId());
}

/////////////////////////////////////////////////////////////////////////////
void  CSipChangeModePartyCntl::OnAudConnectPartyReCaps(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnAudConnectPartyReCaps : Name - ",m_partyConfName, GetPartyRsrcId());

	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2PARTYID(eLevelInfoNormal,"CCSipChangeModePartyCntl::OnAudConnectPartyReCaps : Connect has received after disconnect. Name - ",m_partyConfName, GetPartyRsrcId());
	}

	else
	{
		HandleAudioBridgeConnectedInd(pParam);

		if (AreTwoDirectionsConnectedToAudioBridge() && m_state != REALLOCATE_RSC)
			m_pTaskApi->InformConfOnPartyReCap(m_pParty);
		else if(AreTwoDirectionsConnectedToAudioBridge())
			PTRACE2PARTYID(eLevelInfoNormal,"CCSipChangeModePartyCntl::OnAudConnectPartyReCaps the reallocation will return answer to party : Name - ",m_partyConfName, GetPartyRsrcId());
	}
}
///////////////////////////////////////////////////
BYTE CSipChangeModePartyCntl::ChangeVideoBrdgRsrcIfNeeded()  //DPA
{
	BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_FREE_VIDEO_RESOURCES);

	PTRACE2INT(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeVideoBrdgRsrcIfNeeded tip mode: ", m_pIpInitialMode->GetIsTipMode());

	if ((m_pIpInitialMode->GetConfType() != kCp)|| m_pIpInitialMode->GetIsTipMode())
	{
		TRACEINTO << "(InitialMode->ConfType != CP || TIP)... return FALSE";
		return FALSE;
	}

	eVideoPartyType eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode);

	BYTE isMoveAtoV = FALSE;
	if(eCurrentVideoType == eVideo_party_type_none && m_eLastAllocatedVideoPartyType != eVideo_party_type_none)
		isMoveAtoV = TRUE;
	BYTE isMoveVtoA = FALSE;
	if(eCurrentVideoType != eVideo_party_type_none && m_eLastAllocatedVideoPartyType == eVideo_party_type_none)
		isMoveVtoA = TRUE;
	if(!bEnableFreeVideoResources && (isMoveVtoA || isMoveAtoV) )
	{
		TRACEINTO << "(!bEnableFreeVideoResources && (MoveVtoA || MoveAtoV))... return FALSE";
		return FALSE;
	}

	eConfMediaType confMediaType=m_pIpInitialMode->GetConfMediaType();
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	if(isMoveVtoA && /*pCommConf->GetConfMediaType() == eMixAvcSvc*/ confMediaType==eMixAvcSvc && m_eLastAllocatedVideoPartyType != eVideo_party_type_none)
	{
		TRACEINTO << "(!bEnableFreeVideoResources && (MoveVtoA on mixed conf not supported yet)... return FALSE";
		return FALSE;
	}
	BYTE bNeedToChangeRsrs 	= isNeedTochangeAllocation(eCurrentVideoType);  //noa TBD

	//PTRACE2INT(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeVideoBrdgRsrcIfNeeded -last allcat is:, ",m_eLastAllocatedVideoPartyType);
	if(bNeedToChangeRsrs)
	{
		//No need to disconnect - continue in ReAllocation
		CIpComMode* pTmpScm = new CIpComMode;
		*pTmpScm = *m_pIpInitialMode;
		pTmpScm->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
		BYTE bIsDisconnectFromVideoBridge = DisconnectPartyFromVideoBridgeIfNeeded(pTmpScm);
		if (!bIsDisconnectFromVideoBridge)
		 {
			m_OldState = m_state; //noa check maybe should be state changebridge
			m_state = REALLOCATE_RSC; //to make us reallocate after the disconnection.
			DWORD artCapacity = 0;
			artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, TRUE /*add audio + video for current*/);
			m_artCapacity = artCapacity;

		    CreateAndSendReAllocatePartyResources(eIP_network_party_type, eCurrentVideoType, eAllocateAllRequestedResources,FALSE,0,m_pSIPNetSetup->GetEnableSipICE(),artCapacity);
		 }
		else
		{
			PTRACE2PARTYID(eLevelInfoNormal,"CCSipChangeModePartyCntl::ChangeVideoBrdgRsrcIfNeeded need to disconnect from video bridge : Name - ",m_partyConfName, GetPartyRsrcId());
			m_OldState = m_state; //noa check maybe should be state changebridge
			m_state = REALLOCATE_RSC; //to make us reallocate after the disconnection.

		}
		POBJDELETE(pTmpScm);

		TRACEINTO << " - return TRUE";
		return TRUE;
	}

	else // (!bNeedToChangeRsrs)
	{
		TRACEINTO << "(!bNeedToChangeRsrs)... return FALSE";
		return FALSE;
	}
}
///////////////////////////////////////////////////
BYTE CSipChangeModePartyCntl::isNeedTochangeAllocation(eVideoPartyType eCurrentVideoPartyType)
{
	if (eCurrentVideoPartyType == eVideo_party_type_none && !IsUndefinedParty())//we only downgrade undefined parties to audio only
	{
		return FALSE;
	}

    if (m_bIsMrcCall && CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw)
    {// realloc for SVC not supported
        return FALSE;
    }


//    // bridge-8507 - if AVC in mix mode, do not allow upgrade from SD->HD
//    // till this flow is covered, return false so a realloc is not sent
//    if (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc && !m_bIsMrcCall)
//    {
//        if (m_eLastAllocatedVideoPartyType < eCurrentVideoPartyType && eVideo_party_type_none!=m_eLastAllocatedVideoPartyType)
//        {
//            TRACEINTO << "AVC in mix mode: do NOT upgrade from SD->HD, stay in SD. Flow is not supported yet.";
//            return FALSE;
//        }
//    }

	if(eCurrentVideoPartyType != m_eLastAllocatedVideoPartyType)
	{
		return TRUE;
	}

	return FALSE;
}
///////////////////////////////////////////////////
void CSipChangeModePartyCntl::EndChangeMode()
{
	PTRACE2PARTYID(eLevelInfoNormal,"***CSipChangeModePartyCntl::EndChangeMode: Name - ",m_partyConfName, GetPartyRsrcId());
	CConfParty* pConfParty = GetConfParty();
	PASSERT_AND_RETURN(!pConfParty || !m_pPartyApi);

	m_state 				= IDLE;
	m_changeModeInitiator 	= eNoInitiator;
	m_eChangeModeState 		= eNotNeeded;
	eChangeModeInitiator lastChange = m_changeModeInitiator;

	if(lastChange == eFallBackFromTip)
		m_changeModeInitiator = eFallBackFromTip;

	if (IsValidTimer(CHANGEMODETOUT))
		DeleteTimer(CHANGEMODETOUT);

	//This part is for CDR purpose
	UpdateNewRateForCdrIfNeeded();

	if (m_pIpInitialMode && m_pIpCurrentMode)
	{
		*m_pIpInitialMode = *m_pIpCurrentMode; //in order to end the change mode
		DWORD videoRate = m_pIpCurrentMode->GetMediaBitRate(cmCapVideo,cmCapReceive);
		PTRACE2INT(eLevelInfoNormal,"CSipChangeModePartyCntl::EndChangeMode rate is  - ",videoRate);
	}

	// the secondary cause will be use only if the party is in secondary state.
	SetPartyStateUpdateDbAndCdrAfterEndConnected(SECONDARY_CAUSE_NO_VIDEO_CONNECTION);

	if( GetIsTipCall() && ( m_TipNumOfScreens >1 ) )
		m_pTaskApi->UpdateDB(m_pParty, PARTYTELEPRESENCEMODE, (DWORD)m_telepresenseEPInfo->GetEPtype());//It was (DWORD)eTelePresencePartyCTS

	if (m_bIsNewScm)
	{
		m_bPartyControlChangeScm = TRUE;
		DispatchChangeModeEvent();  // redo the new scm
	}
	else if(m_deferUpgrade)
	{
		m_deferUpgrade = false;
		m_bPartyInUpgradeProcess = true;
		TRACEINTO << "generating deferred upgrade to mixed mode";
		CSegment* pSeg = new CSegment;
		DWORD confID = -1; // EY_20866
		DWORD confState=0;
		*pSeg << (DWORD)confState << confID << GetPartyRsrcId();
		DispatchEvent(SET_PARTY_AVC_SVC_MEDIA_STATE,pSeg);
		POBJDELETE(pSeg);
	}
	// ey_20866 - eyal to do if flag is true : start upgrade procedure (send message)

	//=======================================================================
	// This may be the end of token handling on which a reinvite got pended
	//=======================================================================
	eTokenRecapCollisionDetectionType	eTokenRecapCollisionDetection	= pConfParty -> GetTokenRecapCollisionDetection();
	BOOL 								bTokenRecapPended				= pConfParty -> IsTokenRecapPendedDueToCollisionDetection();
	std::ostringstream msg;
	msg << "Check token handling status: ";
	if (eTokenRecapCollisionDetection == etrcdTokenHandlingInProgress)
	{
		msg << "token handling was in progress, ";
		if (bTokenRecapPended)
		{
			//==========================
			// Reinvoking the reinvite
			//==========================
			msg << "recap is pending.";
			TRACEINTO << "recap is pending - Reinvoking the reinvite.";
			m_pPartyApi -> TokenRecapCollisionEnded();
		}
		else
		{
			//=============================================================================
			// No pends happened, still need to reset the flags as this change-mode ended
			//=============================================================================
			msg << "no recap is pending.";
			pConfParty -> SetTokenRecapCollisionDetection(etrcdAvailable);
		}
	}
	else if (eTokenRecapCollisionDetection == etrcdAvailable && bTokenRecapPended)
	{
		//====================================================
		// Not expecting a pend while nothing is in progress
		//====================================================
		pConfParty -> UnpendTokenRecapDueToCollisionDetection();
		msg << "a trcd pend got noted, with nothing in progress";
	}
	TRACEINTO << msg.str().c_str();
}

/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyRemoteConnectedResponseReCaps(CSegment* pParam)
{
	TRACEINTO << "ConfName: " << m_partyConfName << " PartyName: " << m_name << " m_eChangeModeState=" << m_eChangeModeState;

	m_pIpCurrentMode->DeSerialize(NATIVE, *pParam);
	SetInitialVideoRxInCurrentIfNeeded();
	BYTE bIsEndConfChangeMode = FALSE;
	*pParam >> bIsEndConfChangeMode;

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

	if (m_isContentConn == FALSE &&  pCommConf->GetIsAsSipContent() && m_IsAsSipContentEnable)
	{
		PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyRemoteConnectedResponseReCaps: as-sip conf");
		ConnectToContentBridgeIfPossible();
	}

	//FSN-613: Dynamic Content for SVC/Mix Conf
	if (m_isContentConn && m_bIsMrcCall && m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation))
	{
		PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyRemoteConnectedResponseReCaps: disconnect party from CB");
		DisconnectPartyFromContentBridge();
	}

	if ((m_eChangeModeState == eNotNeeded) || bIsEndConfChangeMode) // If not during change mode process, or during change mode and Party end the change mode
	{
	    TRACEINTO << "Check if to update bridges";
		if(!UpdateBridgesForChannelHandle())
		{
	        TRACEINTO << "Start EndChangeMode";
			EndChangeMode(); // (5)
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyRemoteConnectedResponseReCaps: Wait for Party end change mode");
		m_state = CONF_REQUEST_PARTY_CHANGE_MODE;
	}
	UpdateDetailsForParticipantDisconnectInfoCDREvent(m_pIpCurrentMode);
}
////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::HandleVideoBridgeUpdateForChannelHandle(WORD status, EUpdateBridgeMediaAndDirection eUpdatedBridges)
{
	BYTE bVideoBridgeEndUpdate = CheckVideoBridgeEndUpdate(status, eUpdatedBridges);
	TRACEINTO << "bVideoBridgeEndUpdate=" << (int)bVideoBridgeEndUpdate;
	if (bVideoBridgeEndUpdate)
	{
		if (m_bStartRecapAfterUpdateChannelHandle)
		{// Re-Cap (Reinvite-Ind) arrived during bridge update for channel handle - handle the Re-Cap
			TRACEINTO << "Start Re-cap that was received during bridge update for channel handle";
			StartReCaps();
			m_bStartRecapAfterUpdateChannelHandle = FALSE;
		}
		else
		{
		    if (m_presentationStreamOutIsUpdated)
		        ConnectToContentBridgeIfPossible();

		    TRACEINTO << "Start EndChangeMode";
			EndChangeMode(); // (5)
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyRemoteConnectedPartyChangeMode(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyRemoteConnectedPartyChangeMode: Name - ", m_partyConfName, GetPartyRsrcId());

	m_pIpCurrentMode->DeSerialize(NATIVE, *pParam);
	m_pIpCurrentMode->Dump("mix_mode: CSipChangeModePartyCntl::OnPartyRemoteConnectedPartyChangeMode current",eLevelInfoNormal);

	SetInitialVideoRxInCurrentIfNeeded();
	BYTE bIsEndConfChangeMode = FALSE;
	*pParam >> bIsEndConfChangeMode;

	if (m_isContentConn == FALSE)
	{
		ConnectToContentBridgeIfPossible();
	}

	// (5)
	EndChangeMode();
	UpdateDetailsForParticipantDisconnectInfoCDREvent(m_pIpCurrentMode);
}


/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyRemoteConnectedIdle(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyRemoteConnectedIdle: Name - ", m_partyConfName, GetPartyRsrcId());

	m_pIpCurrentMode->DeSerialize(NATIVE, *pParam);

    if (m_isContentConn == FALSE)
    {
        ConnectToContentBridgeIfPossible();
    }


    /// ***CREATE OUT SLAVES INTEGRATION  - TEMP CODE - TO BE REMOVED***
/*
if (m_CreateOutSlavesintegrationCounter == 0)
    {
		TRACEINTO << "***trigger - MsSlavesController***";

    	m_CreateOutSlavesintegrationCounter++;
    	m_pMsSlavesController = new CMsSlavesController();
    	PASSERT_AND_RETURN(!m_pMsSlavesController);
    	m_pMsSlavesController->Create(GetPartyRsrcId(), m_monitorPartyId, m_pConf);
    	m_pMsSlavesController->ConnectOutSlaves(eHD720_Res, m_pSipLocalCaps->getMsftSsrcVideoFirst(1), 0);
    }
    */
  /// ***********
}

/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyReCapsPartyChangeMode(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyReCapsPartyChangeMode : Name - ", m_partyConfName, GetPartyRsrcId());

	/* It can be as a result of two scenarios. In the both scenarios we should handle the ReCap:

	 * 1. It is a Re-Cap as a result of 200OK during ReInvite req for conf change mode -
	 * First, we handle the ReCap, and then, when IPPARTYCONNECTED will be received, it will be with EndConfChangeMode=true and the change mode flow will be finished, and we will set state to IDLE.

	 * 2. It is a Re-Cap as a result of other Party transaction and Party hasn't started yet the conf change mode transaction -
	 * First, we handle the ReCap, and then, when IPPARTYCONNECTED will be received, it will be with EndConfChangeModewe=false and we will need to return to CONF_REQUEST_PARTY_CHANGE_MODE state and continue the flow of change mode. */

	if( m_changeModeInitiator == eFallBackFromTip )
	{
		HandlePartyReCapParamsInFallback(pParam);
		m_bRestartSipfromTipToNonTip = TRUE;
	}
	else
	{
		HandlePartyReCapsParams(pParam);
		StartReCaps();
	}
}

////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnChangeModeToutReCaps(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnChangeModeToutReCaps: Name - ",m_partyConfName, GetPartyRsrcId());
	ChangeModeTout(pParam);
}

////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnChangeModeToutChangeBridges(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnChangeModeToutChangeBridges: Name - ",m_partyConfName, GetPartyRsrcId());
	ChangeModeTout(pParam);
}

////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeModeTout(CSegment* pParam)
{
	DBGPASSERT(GetPartyRsrcId());
	m_isFaulty = 1;
	BYTE bIsContentSpeaker = (m_pContentBridge && m_pContentBridge->IsTokenHolder(m_pParty)) ? TRUE : FALSE;
    BYTE bUpdateMixModeResources = FALSE;
	m_pPartyApi->BridgesUpdated(m_pIpInitialMode,m_udpAddresses , STATUS_FAIL, bIsContentSpeaker, bUpdateMixModeResources);
	m_pTaskApi->PartyDisConnect(statTout, m_pParty);
}

////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnChangeModeToutResponseParty(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnChangeModeToutResponseParty: Name - ",m_partyConfName, GetPartyRsrcId());
	PartyChangeModeTout(pParam);
}
////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnChangeModeToutPartyChangeMode(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::OnChangeModeToutPartyChangeMode: Name - ",m_partyConfName, GetPartyRsrcId());
	PartyChangeModeTout(pParam);
}
////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::PartyChangeModeTout(CSegment* pParam)
{
	DBGPASSERT(GetPartyRsrcId());
	m_pTaskApi->PartyDisConnect(statTout, m_pParty);
}
////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnCopVideoBridgeChangeIn(CSegment* pParam)
{
	CIpComMode* pNewScm = new CIpComMode(*m_pIpInitialMode);
	CopVideoBridgeChangeIn(pParam, pNewScm);
	const CVidModeH323 InitRecvVideo = (const CVidModeH323 &)m_pIpInitialMode->GetMediaMode(cmCapVideo,cmCapReceive);
	const CVidModeH323 newRecvVideo = (const CVidModeH323 &)pNewScm->GetMediaMode(cmCapVideo,cmCapReceive);
	if (InitRecvVideo == newRecvVideo)
	{
		PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::OnCopVideoBridgeChangeIn : Only update the bridge. state = ", m_state);
		UpdateVideoInBridgeIfNeeded(TRUE, TRUE);
	}
	else  // The common flow:
	{
		m_bPartyControlChangeScm = TRUE;
		ChangeScm(pNewScm,m_IsAsSipContentEnable);
	}
	POBJDELETE(pNewScm);
}
////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnCopVideoBridgeChangeOut(CSegment* pParam)
{
    PTRACE (eLevelInfoNormal, "CSipChangeModePartyCntl::OnCopVideoBridgeChangeOut - not supported in SIP");
    //CopVideoBridgeChangeOut(pParam);

}
////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnConfChangeModeAnycase(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipChangeModePartyCntl::OnConfChangeModeAnycase : Just save the pending scm. state = ",m_state);
	MoveScmToPendingWhileChangeMode(m_pIpInitialMode);

	/* If any change mode occurred during another change mode process
	 * the new decoder resolution is recorded and m_pIpInitialMode is updated and m_bIsNewScm is true.
	 * The new change decoder mode will be done and the end of current change mode.
	 */
}
////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::SetInitialVideoRxInCurrentIfNeeded()
{
	// When we are during change incoming, we don't want to update the video In bridge with old mode that can be receive as a result of re-cap during change incoming,
	// so we set the current receive video mode according initial.
	if (m_eChangeModeState == eChangeIncoming)
	{
		if (m_pIpCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople) && m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))
			m_pIpCurrentMode->CopyMediaMode(*m_pIpInitialMode, cmCapVideo, cmCapReceive, kRolePeople);
	}
}
////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyUpdateBridgesAnycase(CSegment* pParam)
{
	// Currently occur only as a result of diff_payload indication. Ignore it during change mode.
	PTRACE(eLevelInfoNormal, "CSipChangeModePartyCntl::OnPartyUpdateBridgesAnycase : Ignore diff payload ind during change mode");
}
///////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate", GetPartyRsrcId());

	m_pIpInitialMode->Dump("CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate m_pIpInitialMode");

	m_state = m_OldState;
	eVideoPartyType eCurrentVideoType;
	eCurrentVideoType = GetMaxCurrentCallVideoPartyTypeForAllocation(m_pIpInitialMode);
	eVideoPartyType lastallocatedPartyBeforeChange = m_eLastAllocatedVideoPartyType;
	eVideoPartyType requestedVideoPartyType = m_eLastReAllocRequestVideoPartyType;
	eConfMediaType confMediaType=m_pIpInitialMode->GetConfMediaType();
	TRACEINTO<<"!!!confMediaType: "<<(int)confMediaType<<" eCurrentVideoType:"<<(int)eCurrentVideoType<<" lastallocatedPartyBeforeChange:"<<(int)lastallocatedPartyBeforeChange<<" requestedVideoPartyType: "<<(int)requestedVideoPartyType;
	// BRIDGE 7094 + BRIDGE 6972
//	if(confMediaType==eMixAvcSvc)
//	{
//
//	    POBJDELETE(m_pMrmpRsrcParams);
//            m_pMrmpRsrcParams=NULL;
//
//	    for (int i=0; i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++i)
//	    {
//                POBJDELETE(m_avcToSvcTranslatorRsrcParams[i]);
//	        m_avcToSvcTranslatorRsrcParams[i] = NULL;
//	    }
//	    CSegment* pParamAdd=new CSegment(*pParam);
//	    DWORD status = CIpPartyCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate(pParamAdd,m_avcToSvcTranslatorRsrcParams,m_pMrmpRsrcParams,ALLOCATION_TYPE_REALLOC);
//	    POBJDELETE(pParamAdd);
//	}

	BYTE bAllocationFailed = HandleReallocateResponseOnChangeMode(pParam);
	if (bAllocationFailed == FALSE)
	{

		DWORD videoRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo,cmCapReceive, kRolePeople);
		eVideoQuality vidQuality = m_pConf->GetCommConf()->GetVideoQuality();

		BYTE bIsAudioOnly = (m_eLastAllocatedVideoPartyType == eVideo_party_type_none) ? 1 : 0;
		if (m_eLastAllocatedVideoPartyType <= requestedVideoPartyType && !bIsAudioOnly )
		{

			H264VideoModeDetails h264VidModeDetails = GetH264ModeAccordingToVideoPartyType(m_eLastAllocatedVideoPartyType);
			PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate -changing local caps and target mode accordingly -this is new type: ", m_eLastAllocatedVideoPartyType);

			UpdateH264ModeInLocalCaps(h264VidModeDetails);

		//	if(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive) == eRtvCapCode)
		//	{
				RTVVideoModeDetails rtvVidModeDetails;
				GetRtvVideoParams(rtvVidModeDetails, videoRate*100, vidQuality, h264VidModeDetails.videoModeType);

				UpdateRtvModeInLocalCaps(rtvVidModeDetails,kRolePeople,videoRate);
		//	}
			BYTE IsChangeBestModeNeededForTx  = checkRsrcLimitationsByPartyType(cmCapTransmit,m_pIpInitialMode,h264VidModeDetails);
			BYTE IsChangeBestModeNeededForRcv = checkRsrcLimitationsByPartyType(cmCapReceive,m_pIpInitialMode,h264VidModeDetails);
			CCapSetInfo capInfo = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople);

			if(m_eLastAllocatedVideoPartyType < requestedVideoPartyType &&  (IsChangeBestModeNeededForTx || IsChangeBestModeNeededForRcv))
			{
				if(capInfo.GetIpCapCode() == eH264CapCode)
				{
					PTRACE(eLevelInfoNormal, "CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate -setting new target mode: ");
					m_pIpInitialMode->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR,cmCapReceive);
					m_pIpInitialMode->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR,cmCapTransmit);
				}
				if(capInfo.GetIpCapCode() == eRtvCapCode)
				{
				//	RTVVideoModeDetails rtvVidModeDetails;

    			//	CRtvVideoMode* pRtvVidMode = new CRtvVideoMode();
				//	GetRtvVideoParams(rtvVidModeDetails, videoRate, vidQuality, h264VidModeDetails.videoModeType);
					//pRtvVidMode->GetRtvVideoParams(rtvVidModeDetails,h264VidModeDetails.videoModeType);
				//	POBJDELETE(pRtvVidMode);

					 m_pIpInitialMode->SetRtvVideoParams(rtvVidModeDetails);
				}
			}
			CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_monitorPartyId);
			FixVideoBitRateIfNeeded( pConfParty, m_pIpInitialMode , m_pSIPNetSetup, FALSE, eCurrentVideoType);

			BYTE cif4MpiRe = FALSE;
			BYTE cif4MpiTr = FALSE;
			if(m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople)  == eH263CapCode)
			{
				cif4MpiRe = m_pIpInitialMode->GetFormatMpi((EFormat)k4Cif, cmCapReceive);
				cif4MpiTr = m_pIpInitialMode->GetFormatMpi((EFormat)k4Cif, cmCapTransmit);
			}
		    if((m_eLastAllocatedVideoPartyType < eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type) && cif4MpiRe)
			{
		    	m_pIpInitialMode->SetFormatMpi((EFormat)k4Cif,-1 ,(cmCapDirection)cmCapReceive);
		    	//m_pIpInitialMode->SetFormatMpi((EFormat)k4Cif,-1 ,(cmCapDirection)cmCapTransmit);
			}
		    if(capInfo.GetIpCapCode() == eH263CapCode && (m_eLastAllocatedVideoPartyType < eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type) && cif4MpiTr)
		    {
		    	m_pIpInitialMode->SetFormatMpi((EFormat)k4Cif,-1 ,(cmCapDirection)cmCapTransmit);//tbd conside res slider
		    }
			if(capInfo.GetIpCapCode() == eRtvCapCode)
			{
				//Updating both RTV and H264 caps
				MsSvcVideoModeDetails mssvcmodedetails;
				m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails, cif4MpiRe, FALSE,TRUE,FALSE,videoRate);
			}
			else if(capInfo.GetIpCapCode() == eMsSvcCapCode)
			{
				MsSvcVideoModeDetails mssvcmodedetails = GetMsSvcModeAccordingToVideoPartyType(m_eLastAllocatedVideoPartyType);
                CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(GetName());
                if (!pConfParty)
                {
                    PTRACE(eLevelError,"CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate - pConfParty is NULL");
                    DBGPASSERT(1126);
                    return;
                }

                bool force_rtv = !(VIDEO_PROTOCOL_MS_SVC == pConfParty->GetVideoProtocol());
                CCOPConfigurationList* pCOPConfigurationList = m_pConf->GetCommConf()->GetCopConfigurationList();
				m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails ,cif4MpiRe, bIsAudioOnly,force_rtv/*RTV*/,TRUE,videoRate);
			}
			else
			{
				MsSvcVideoModeDetails mssvcmodedetails;
				//Update only H264 caps
				m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails, cif4MpiRe, bIsAudioOnly,FALSE,FALSE, videoRate);
			}
		}
		if(m_eLastAllocatedVideoPartyType == eVideo_party_type_none  && requestedVideoPartyType == eVideo_party_type_none)
		{
			//TBD moving V-> A NOA
			//do not update local caps just stay with the new initial maybe there is nothing to do
			PTRACEPARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate - V->A ", GetPartyRsrcId());

		}
		if(m_eLastAllocatedVideoPartyType != eVideo_party_type_none && lastallocatedPartyBeforeChange != eVideo_party_type_none &&  requestedVideoPartyType == eVideo_party_type_none )
		{

			PTRACEPARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate - tried V->A but no audio resources", GetPartyRsrcId());
			const char* MOC_PRODUCT_NAME = "Microsoft Office Communicator";
			char * bIsMsMoc = strstr(m_productId, MOC_PRODUCT_NAME);
			if(m_eLastAllocatedVideoPartyType == GetLowestVideoAllocationAccordingToSystemMode (bIsMsMoc) )//we are already in the lowest video possible
			{
				PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate-already in lowest video possible");
				//SetPartyToAudioOnly();
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspReAllocate-reallocate v to a try lowest video");
				m_OldState = m_state;
				m_state = REALLOCATE_RSC;
				H264VideoModeDetails h264VidModeDetails = GetH264ModeAccordingToVideoPartyType(GetLowestVideoAllocationAccordingToSystemMode(bIsMsMoc));

				if (m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive,kRolePeople) == eRtvCapCode)
				{
					RTVVideoModeDetails rtvVidModeDetails;
					GetRtvVideoParams(rtvVidModeDetails, videoRate*100, vidQuality, h264VidModeDetails.videoModeType);

					UpdateRtvModeInLocalCaps(rtvVidModeDetails,kRolePeople,videoRate);
					MsSvcVideoModeDetails mssvcmodedetails;
					m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails, -1/*no 4cif*/, FALSE/*not caps of audio noly*/,TRUE,FALSE, videoRate);
				}
				UpdateH264ModeInLocalCaps(h264VidModeDetails);

				if (m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive,kRolePeople) == eRtvCapCode)
				{
					MsSvcVideoModeDetails mssvcmodedetails;
					m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails, -1/*no 4cif*/, FALSE/*not caps of audio noly*/,TRUE,FALSE,videoRate);
				}
				else
				{
					MsSvcVideoModeDetails mssvcmodedetails;
					m_pPartyApi->SetCapsValuesAccordingToNewAllocation(h264VidModeDetails,mssvcmodedetails, -1/*no 4cif*/, FALSE/*not caps of audio noly*/,FALSE,FALSE ,videoRate);
				}

				DWORD artCapacity = 0;
				artCapacity = CalculateArtCapacityAccordingToScm(m_pIpInitialMode, TRUE /*add audio + video for current*/);
				m_artCapacity = artCapacity;

				CreateAndSendReAllocatePartyResources(eIP_network_party_type, GetLowestVideoAllocationAccordingToSystemMode(bIsMsMoc), eAllocateAllRequestedResources,FALSE,0,m_pSIPNetSetup->GetEnableSipICE(),artCapacity);
				return;
			}
			//do not change caps but continue with new initial
		}
		m_pTaskApi->InformConfOnPartyReCap(m_pParty);

	//	POBJDELETE(pRtvVidMode);
	}


}
/////////////////////////////////////////////////////////////
BYTE CSipChangeModePartyCntl::HandleReallocateResponseOnChangeMode(CSegment* pParam)
{
	CPartyRsrcDesc* pTempPartyAllocatedRsrc = new CPartyRsrcDesc;
	pTempPartyAllocatedRsrc->DeSerialize(SERIALEMBD, *pParam);
	DWORD status = pTempPartyAllocatedRsrc->GetStatus();
	pTempPartyAllocatedRsrc->DumpToTrace();
	BYTE bAllocationFailed = FALSE;

	eNetworkPartyType networkPartyType =  pTempPartyAllocatedRsrc->GetNetworkPartyType();

	if ((status != STATUS_OK)||(networkPartyType!=eIP_network_party_type))
	{
		if(networkPartyType!=eIP_network_party_type)
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::HandleReallocateResponseOnChangeMode eNetworkPartyType!= eIP_network_party_type, eNetworkPartyType = ",eNetworkPartyTypeNames[networkPartyType], GetPartyRsrcId());
			PASSERT(1);
		}
		PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::HandleReallocateResponseOnChangeMode : REALLOCATION FAILED!!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
		bAllocationFailed = TRUE;
	}

	else
	{

		eVideoPartyType requestedVideoPartyType = m_eLastReAllocRequestVideoPartyType;
		eVideoPartyType allocatedVideoPartyType = pTempPartyAllocatedRsrc->GetVideoPartyType();

		TRACEINTO << "requestedVideoPartyType=" << requestedVideoPartyType << " allocatedVideoPartyType=" << allocatedVideoPartyType;
        UpdateResourceTableAfterRealloc(pTempPartyAllocatedRsrc);

		if (requestedVideoPartyType >= allocatedVideoPartyType)
		{
			if(allocatedVideoPartyType == eVideo_party_type_none)
			{// if its undefined call make it audio only one (remove all none audio media from SCM, set audio only caps, set m_voice member, set ConfParty DB).
				SetPartyToAudioOnly();
			}
			if ((allocatedVideoPartyType != eVideo_party_type_none && m_eLastAllocatedVideoPartyType == eVideo_party_type_none)||(allocatedVideoPartyType != eVoice_relay_party_type && m_eLastAllocatedVideoPartyType == eVoice_relay_party_type))   //forDPA A to V in SIP
			{
				PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::HandleReallocateResponseOnChangeMode : REALLOCATION A to V new resources!!! : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
				UdpAddresses tmpUdpAdd;
				pParam->Get((BYTE*)(&tmpUdpAdd), sizeof(UdpAddresses));
				if ((tmpUdpAdd.AudioChannelPort != 0) || (CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw))
					m_udpAddresses = tmpUdpAdd;
				DumpUdpAddresses();
			   if(pTempPartyAllocatedRsrc->GetConfRsrcId() != m_pPartyAllocatedRsrc->GetConfRsrcId() || pTempPartyAllocatedRsrc->GetPartyRsrcId() != m_pPartyAllocatedRsrc->GetPartyRsrcId())
			   {
				   PASSERTMSG(1, "CSipChangeModePartyCntl::HandleReallocateResponseOnChangeMode : Internal Error receive invalid params not matching conf or party rsrc id");
				   m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,RESOURCES_DEFICIENCY,1); // Disconnnect cause
				   m_pTaskApi->EndAddParty(m_pParty,statIllegal);
			   }
			   m_pTaskApi->UpdateDB(m_pParty,PARTYSTATUS,PARTY_RESET_STATUS);

			   if (YES == m_voice)
			   	   m_isAudioDecoderUpdateNeeded = eMediaTypeUpdateAudioToVideo;
			   m_voice = NO;
	        }

			m_eLastAllocatedVideoPartyType = allocatedVideoPartyType;

	        UpdateScmWithResources(m_SsrcIdsForAvcParty.m_SsrcIds, allocatedVideoPartyType, pTempPartyAllocatedRsrc->GetIsAvcVswInMixedMode());

		}
		else if (requestedVideoPartyType < allocatedVideoPartyType && requestedVideoPartyType != eVideo_party_type_none)
		{
			PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::HandleReallocateResponseOnChangeMode : Higher allocation than requested !!! do not continue process : ",CProcessBase::GetProcess()->GetStatusAsString(status).c_str(), GetPartyRsrcId());
			bAllocationFailed = TRUE;
		}
	}


	POBJDELETE(pTempPartyAllocatedRsrc);

	if (bAllocationFailed)
	{
		m_pTaskApi->UpdateDB(m_pParty,DISCAUSE,RESOURCES_DEFICIENCY,1); // Disconnnect cause
		m_pTaskApi->EndAddParty(m_pParty,statIllegal);
		//m_isFaulty = 1;
	}
	else
		PTRACEPARTYID(eLevelInfoNormal, "CSipChangeModePartyCntl::HandleReallocateResponseOnChangeMode : REAllocation is OK ", GetPartyRsrcId());

	return bAllocationFailed;
}

/////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
//
//		CONTENT SECTION  (Eitan)
//
////////////////////////////////////////////////////////////////////////////////
BYTE CSipChangeModePartyCntl::IsChangeContentNeeded(BYTE bSetChangeModeStateIfNeeded)
{
	CMedString cstr;
	cstr << "***CSipChangeModePartyCntl::IsChangeContentNeeded Name - " << m_partyConfName << ", bSetChangeModeStateIfNeeded=" << bSetChangeModeStateIfNeeded << "\n";

	BYTE bIsSecondaryCondition	= FALSE;
	BYTE bNeedToChangeContent	= FALSE;

	
	//BRIDGE-15059 for SVC
	BYTE bIsNeedToRemoveContetnt = IsLegacyContentParty();
	if (m_bIsMrcCall && bIsNeedToRemoveContetnt && m_pIpInitialMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePresentation))
	{
		cstr <<  "Change content is needed - need to close content ";
		if (bIsNeedToRemoveContetnt & eContentSecondaryCauseBelowRate)
	    	{
			 SetPartySecondaryCause(SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD);
	    	}
	    	else if(bIsNeedToRemoveContetnt & eContentSecondaryCauseBelowResolution)
	    	{
			 SetPartySecondaryCause(SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD);
	    	}

		m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
		m_pIpInitialMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	}
	
	OFF(m_bNoContentChannel);
	BYTE bIsContentConnected  = TRUE;

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();

	if (!m_bNoContentChannel) //party can have content!
	{//check if the content channels are not opened
		if (m_pIpCurrentMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePresentation))
			bIsContentConnected = FALSE;//in case the outgoing channel isn't opened => it always a problem
	}

	TRACEINTO << "bIsContentConnected=" << (int)bIsContentConnected 
		<< " m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRolePresentation)=" 
		<< (int)m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRolePresentation);

	if ((bIsContentConnected == FALSE && m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRolePresentation)) ||
		(!m_isContentConn && pCommConf->GetIsAsSipContent() && IsRemoteAndLocalCapSetHasBfcpUdp() && m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRolePresentation)&& m_IsFirstContentNegotiation ) )
	{
			bNeedToChangeContent = TRUE;
			if (bSetChangeModeStateIfNeeded )
				m_eChangeModeState = eChangeContentInAndOut;
			cstr <<  "Change content is needed - Open content (initial connection to bridge)";
			m_IsFirstContentNegotiation = FALSE;
	}
	else if (IsContentProtocolNeedToBeChanged())
	{
		bNeedToChangeContent = TRUE;
		if (bSetChangeModeStateIfNeeded)
			m_eChangeModeState = eChangeContentInAndOut;
		cstr <<  "Change content is needed - different protocols (change protocol)";
	}
	else if (IsContentRateNeedToBeChanged())
	{
		bNeedToChangeContent = TRUE;
		if (bSetChangeModeStateIfNeeded)
			m_eChangeModeState = eChangeContentRate;
		cstr <<  "Change content is needed - different rates (change rate)";
	}
	
	if(IsContentHDResolutionOrMpiNeedToBeChanged())
	{
		bNeedToChangeContent = TRUE;
		if(m_eChangeModeState == eChangeContentRate)
			PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::IsChangeContentNeeded - change resolution and rate!");
		if (bSetChangeModeStateIfNeeded)
		    m_eChangeModeState = eChangeContentInAndOut;
		cstr <<  "Change content is needed - different HD Res (change Res)";

	}
	//HP content
	else if(IsContentProfileNeedToBeChanged())
	{
		bNeedToChangeContent = TRUE;
		if(m_eChangeModeState == eChangeContentRate)
			PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::IsChangeContentNeeded - change h264 profile and rate!");
		if (bSetChangeModeStateIfNeeded)
		    m_eChangeModeState = eChangeContentInAndOut;
		cstr <<  "Change content is needed - different profile (change profile)";

	}
	else
	{
		if (!bNeedToChangeContent)
			cstr <<  "Change content is not needed";
	}

	PTRACE(eLevelInfoNormal,cstr.GetString());

	return bNeedToChangeContent;
}
//////////////////////////////////////////////////////////////////////////////////
BYTE CSipChangeModePartyCntl::IsContentProtocolNeedToBeChanged(/*BYTE isSecondaryCondition*/) const
{
	//	if(isSecondaryCondition)// in case of secondary we can't change content mode, just continue to be with close
	//		return FALSE;		// outgoing channels and disconnected (or not connected) to the content bridge.

	BYTE  bNeedToChangeContent = FALSE;

	CapEnum initialContentOutType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum currentContentOutType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum initialContentInType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	CapEnum currentContentInType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);

	CMedString msg;
	msg << "Initial: Out=" << initialContentOutType << ", In=" << initialContentInType << ". Current: Out=" << currentContentOutType << ", In=" << currentContentInType;

	PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::IsContentProtocolNeedToBeChanged: ",msg.GetString());
	bNeedToChangeContent =  ((initialContentOutType != currentContentOutType) || (initialContentInType != currentContentInType));

	return bNeedToChangeContent;
}
//////////////////////////////////////////////////////////////////////////////////
BYTE CSipChangeModePartyCntl::IsContentHDResolutionOrMpiNeedToBeChanged(/*BYTE isSecondaryCondition*/) const
{
	//	if(isSecondaryCondition)// in case of secondary we can't change content mode, just continue to be with close
	//		return FALSE;		// outgoing channels and disconnected (or not connected) to the content bridge.

	BYTE  bNeedToChangeContent = FALSE;

	CapEnum initialContentOutType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum currentContentOutType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum initialContentInType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	CapEnum currentContentInType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);

	if((initialContentOutType == initialContentInType) && (initialContentOutType == eH264CapCode))
	 if((initialContentOutType  == currentContentOutType) && (initialContentInType == currentContentInType))
	 {
	   BYTE  initialRcvContentHD10800Mpi = m_pIpInitialMode->isHDContent1080Supported(cmCapReceive);
       BYTE  currentRcvContentHD1080Mpi = m_pIpCurrentMode->isHDContent1080Supported(cmCapReceive);
       BYTE  initialTxContentHD1080Mpi  = m_pIpInitialMode->isHDContent1080Supported(cmCapTransmit);
       BYTE  currentTxContentHD1080Mpi  = m_pIpCurrentMode->isHDContent1080Supported(cmCapTransmit);

       bNeedToChangeContent =  (initialRcvContentHD10800Mpi != currentRcvContentHD1080Mpi) || (initialTxContentHD1080Mpi != currentTxContentHD1080Mpi);
       CMedString msg;
       msg << "Initial Support of HD Content Res 1080: Out=" << (DWORD)initialTxContentHD1080Mpi << ", In=" << (DWORD)initialRcvContentHD10800Mpi << ". Current: Out=" << (int)currentTxContentHD1080Mpi << ", In=" << (int)currentRcvContentHD1080Mpi;
       PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::IsContentHDResolutionOrMpiNeedToBeChanged: ",msg.GetString());
       // Test change of HD 720 MPI
       if(!bNeedToChangeContent && !currentRcvContentHD1080Mpi && !currentTxContentHD1080Mpi)
       {
    	   BYTE  initialRcvContentHD720Mpi = m_pIpInitialMode->isHDContent720Supported(cmCapReceive);
    	   BYTE  currentRcvContentHD720Mpi = m_pIpCurrentMode->isHDContent720Supported(cmCapReceive);
    	   BYTE  initialTxContentHD720Mpi  = m_pIpInitialMode->isHDContent720Supported(cmCapTransmit);
    	   BYTE  currentTxContentHD720Mpi = m_pIpCurrentMode->isHDContent720Supported(cmCapTransmit);
    	   if(/*BRIDGE-15567: do not assume 720p resolution as XGA is used in TIP. -initialRcvContentHD720Mpi && currentRcvContentHD720Mpi &&*/ (initialRcvContentHD720Mpi != currentRcvContentHD720Mpi))
    	   {
    		   CMedString msg1;
    		   msg1 << " RCV MPI of HD 720 Content: Initial=" << (DWORD)initialRcvContentHD720Mpi << ", Current=" << (DWORD)currentRcvContentHD720Mpi;
    		   PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::IsContentHDResolutionOrMpiNeedToBeChanged: ",msg1.GetString());
    		   bNeedToChangeContent = TRUE;
    	   }
    	   if(/*BRIDGE-15567: do not assume 720p resolution as XGA is used in TIP. - initialTxContentHD720Mpi && currentTxContentHD720Mpi &&*/ (initialTxContentHD720Mpi != currentTxContentHD720Mpi))
    	   {
    	       CMedString msg2;
    	       msg2 << " Tx MPI of HD 720 Content: Initial=" << (DWORD)initialTxContentHD720Mpi << ", Current=" << (DWORD)currentTxContentHD720Mpi;
    	       PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::IsContentHDResolutionOrMpiNeedToBeChanged: ",msg2.GetString());
    	       bNeedToChangeContent = TRUE;
    	   }
    	}
	 }

	return bNeedToChangeContent;
}
//////////////////////////////////////////////////////////////////////////////////
//HP content:
BYTE CSipChangeModePartyCntl::IsContentProfileNeedToBeChanged(/*BYTE isSecondaryCondition*/) const
{
	BYTE  bNeedToChangeContent = FALSE;

	CapEnum initialContentOutType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum currentContentOutType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CapEnum initialContentInType = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	CapEnum currentContentInType = (CapEnum)m_pIpCurrentMode->GetMediaType(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);

	if((initialContentOutType == initialContentInType) && (initialContentOutType == eH264CapCode))
		if((initialContentOutType  == currentContentOutType) && (initialContentInType == currentContentInType))
	 	{
	 		BYTE initialContentOutProfile = m_pIpInitialMode->IsH264HighProfileContent(cmCapTransmit);
			BYTE currentContentOutProfile = m_pIpCurrentMode->IsH264HighProfileContent(cmCapTransmit);
			BYTE initialContentInProfile = m_pIpInitialMode->IsH264HighProfileContent(cmCapReceive);
			BYTE currentContentInProfile = m_pIpCurrentMode->IsH264HighProfileContent(cmCapReceive);

			CMedString msg;
			msg << "Initial: Out=" << initialContentOutProfile << ", In=" << initialContentInProfile << ". Current: Out=" << currentContentOutProfile << ", In=" << currentContentInProfile;

			PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::IsContentProfileNeedToBeChanged: ",msg.GetString());
			bNeedToChangeContent =  ((initialContentOutProfile != currentContentOutProfile) || (initialContentInProfile != currentContentInProfile));
	 	}

	return bNeedToChangeContent;
}
//////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeContentBridgeStateAccordingToNewMode()
{
	PTRACE2(eLevelInfoNormal,"***CSipChangeModePartyCntl::ChangeContentBridgeStateAccordingToNewMode : Name - ",m_partyConfName);
	DWORD initialContRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

	if (m_isContentConn == FALSE)
	{
		ConnectToContentBridgeIfPossible();
	}
	else
	{
		// the content rate was changed
	    TRACESTR(eLevelInfoNormal)<< " CSipChangeModePartyCntl::ChangeContentBridgeStateAccordingToNewMode - Content rate changed to " << initialContRate << ",  Name: " << m_partyConfName;

		if (m_pContentBridge->IsPartyWaitForRateChange(m_pParty))
		{
			m_conferenceContentRate = initialContRate;  //BRIDGE-15257
			if (m_eChangeModeState != eChangeContentInAndOut)
			{
				BYTE parameterID = 0; // Used only in ISDN calls
				// inform contetnt bridge about party new rate
				m_pTaskApi->PartyContentRateChanged(m_pParty,parameterID,initialContRate);
			}
			else // The change mode is part of change protocol, will be done by reconnect to content bridge.
			{
				BYTE parameterID = 0; // Used only in ISDN calls
				m_pTaskApi->PartyContentRateChanged(m_pParty,parameterID,initialContRate);
				PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeContentBridgeStateAccordingToNewMode : Not need to inform content bridge on change rate. Name - ",m_partyConfName);
			}

			if (IsOutDirectionConnectedToVideoBridge())
			{
				// In this case we need to inform the video bridge to change its rate towards the ep (Encoder - Video out).
				//BYTE bTakeInitial = FALSE;
				if (m_bIsMrcCall || ((m_pIpInitialMode->GetConfType() == kCp) && m_pIpInitialMode->GetConfMediaType()!=eMixAvcSvcVsw))
					UpdateVideoOutBridgeH239Case(TRUE/*bTakeInitial*/);
			}
		}
		else
		{
		    TRACESTR(eLevelInfoNormal)<< " CSipChangeModePartyCntl::ChangeContentBridgeStateAccordingToNewMode: ChangeContent is needed, party connected to bridge but not waiting for change rate (?!),  Name: " << m_partyConfName;
		}
	}
}
////////////////////////////////////////////////////////////////////////////
//Inorder to connect to the content bridge we need to receive 2 opcodes:
//1) From CAM - PARTY_IN_CONF
//2) From PARTY - PRESENTATION_OUT_STR_UPDATE
//This function check both flags and connect to content bridge or
// send UpdatePresentationOutStream to party if necessary...
void CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible()
{
	PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible : Name - ",m_partyConfName);

	m_pIpInitialMode->Dump("CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible - m_pIpInitialMode is: ",eLevelInfoNormal);
	m_pIpCurrentMode->Dump("CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible - m_pIpCurrentMode is: ",eLevelInfoNormal);

	if(m_isPartyInConf)
	{
	    if(m_presentationStreamOutIsUpdated)
		{
			PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible: Connect the content bridge, Name - ",m_partyConfName);
			ConnectPartyToContentBridge();
		}
		else
		{	// Need to check if presentation chennles are opened
			if ( !m_bNoContentChannel ) //party can have content!
			{//check if the content channels were opened
				if (!m_isSentH239Out)
				{
					if (m_pIpInitialMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRolePresentation))
					{
						PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible: Send update presentation stream out to party, Name - ",m_partyConfName);
						m_pPartyApi->UpdatePresentationOutStream();
						ON(m_isSentH239Out);
					}
					else
						PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible: initial mode has no content, Name - ",m_partyConfName);

				}
				else
					PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible: content channels aren't open yet... Name - ",m_partyConfName);
			}
			else
				PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible: No content channel Name - ",m_partyConfName);
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::ConnectToContentBridgeIfPossible: Still In IVR....!!!");
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CSipChangeModePartyCntl::ConnectPartyToContentBridge()
{
	if (m_pIpCurrentMode->IsMediaOff(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation))
		return;

	TRACEINTO << "m_pContentBridge " << (m_pContentBridge ? "exists" : "null")
			  << ", m_isContentConn " << (m_isContentConn ? "exists" : "null");

	if (m_pContentBridge)
	{
		if (!m_isContentConn)
		{
			ON(m_isContentConn);

			//VNGFE-6111 Set the fix as implementation for ISDN (no need to ask regarding the content holder)
            //BYTE mcuNum;
            //BYTE termNum;
            //const CTaskApp* tokenHolder = m_pContentBridge->GetTokenHolder(mcuNum, termNum);


            WORD partyContentRate = 0;
            //if (IsValidPObjectPtr(tokenHolder))
                partyContentRate = m_pIpInitialMode->GetMediaBitRate(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);
            PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::ConnectPartyToContentBridge : partyContentRate=",partyContentRate);
            CapEnum H323partyProtocol = (CapEnum)m_pIpInitialMode->GetMediaType(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation);
            CComModeInfo cmInfo = H323partyProtocol;
		    WORD partyH239Protocol = cmInfo.GetH320ModeType();
//            if (partyH239Protocol != H264)
//            	PASSERT_AND_RETURN(1);

			//HP content
			BYTE isContentH264HighProfile = FALSE;
			if (partyH239Protocol == H264)
				isContentH264HighProfile = m_pIpInitialMode->IsH264HighProfileContent(cmCapTransmit);

			CContentBridgePartyInitParams* pBrdgPartyInitParams = 	new CContentBridgePartyInitParams(m_name, m_pParty, GetPartyRsrcId(), GetInterfaceType(),partyContentRate,partyH239Protocol,isContentH264HighProfile);

			CLargeString str;
			str << "CSipChangeModePartyCntl::ConnectPartyToContentBridge. rate: " << ((CContentBridgePartyInitParams*)pBrdgPartyInitParams)->GetByCurrentContentRate()
				<< ", Protocol: " << ((CContentBridgePartyInitParams*)pBrdgPartyInitParams)->GetByCurrentContentProtocol()
				<< ", HighProfile: "<< (int)(((CContentBridgePartyInitParams*)pBrdgPartyInitParams)->GetByCurrentContentH264HighProfile());        //HP content
			str << "\n ---> " << m_partyConfName << " - Establishing Content Party Connection";
			//ALLOCBUFFER(tmp,2*H243_NAME_LEN+150); //max size of m_partyConfName is (2*H243_NAME_LEN+50)
			//sprintf(tmp, "%s\n ---> [%s] - Establishing Content Party Connection ", str.GetString(), m_partyConfName);
			PTRACE2PARTYID(eLevelInfoNormal,"",str.GetString(), GetPartyRsrcId());
//			DEALLOCBUFFER(tmp);

			m_pContentBridge->ConnectParty(pBrdgPartyInitParams);

			if (IsOutDirectionConnectedToVideoBridge())
			{
                if (m_bIsMrcCall || ((m_pIpInitialMode->GetConfType() == kCp) && m_pIpInitialMode->GetConfMediaType()!=eMixAvcSvcVsw))
					UpdateVideoOutBridgeH239Case(TRUE); // take initial = TRUE
			}

	      	POBJDELETE(pBrdgPartyInitParams);
		}
	}

}

////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnCAMUpdatePartyInConf(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::OnCAMUpdatePartyInConf : Name - ",m_partyConfName);
	ON(m_isPartyInConf);

	BYTE bRes = FALSE;
		bRes = CheckIfNeedToSendIntra();

	if(bRes)
	{
		SendIntraToParty();
		SendVideoPreferenceToParty();
	}
	ConnectToContentBridgeIfPossible();

}
///////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyPresentationOutStreamUpdate(CSegment* pParam)
{
	//if we receive this opcode, we already received PARTY_IN_CONF from CAM.
    TRACESTR(eLevelInfoNormal)<< " CSipChangeModePartyCntl::OnPartyPresentationOutStreamUpdate : state - " << m_state << ",  Name - " << m_partyConfName;
	ON(m_presentationStreamOutIsUpdated);
	ConnectToContentBridgeIfPossible();

}

///////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyPresentationOutStreamUpdateChannelHandle(CSegment* pParam)
{
    //if we receive this opcode, we already received PARTY_IN_CONF from CAM.
    TRACEINTO << "Name - " << m_partyConfName;
    ON(m_presentationStreamOutIsUpdated);
}

///////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyRemoteConnectedChannelHandle(CSegment* pParam)
{
    TRACEINTO << "Name - " << m_partyConfName << " - Do nothing!!! Change mode was already invoked!!!";
}

///////////////////////////////////////////////////////////////////////////
int  CSipChangeModePartyCntl::OnContentBrdgConnectedAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::OnContentBrdgConnectedAnycase : Name - ",m_partyConfName);
	CPartyCntl::OnContentBrdgConnected(pParam);
	if (!m_isContentConn)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::OnContentBrdgConnectedAnycase : Problems while connecting. Name - ",m_partyConfName);
		if (m_isFaulty)
		{
			m_pTaskApi->PartyDisConnect(H323_CALL_CLOSED_PROBLEM_WITH_CONTENT_CONNECTION_TO_MCU,m_pParty);
			return -1;
		}

	}


	return 0;

}
///////////////////////////////////////////////////////////////////////////
int CSipChangeModePartyCntl::OnContentBrdgDisconnectedAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipChangeModePartyCntl::OnContentBrdgDisconnectedAnycase : Name - ",m_partyConfName);
	int status = CIpPartyCntl::OnContentBrdgDisconnected(pParam);

	// The party was disconnected from content bridge by the bridge as a part of change content protocol. Now we need to reconnect to the bridge if possible.
	if (status==statOK)
		ConnectToContentBridgeIfPossible();
	return status;
}
//////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeSipfromTipToNonTip(CIpComMode* pScm,CSipCaps* pNewLocalCaps)
{
	PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeSipfromTipToNonTip from conf to party in order to start a new reinvite");

	*m_pIpInitialMode 				= *pScm;


	CSipComMode* pBestMode 			= NULL;

	BYTE bWithinProtocolLimitation 	= FALSE;

/*	if( m_pSipRemoteCaps->GetIsContainingCapCode(cmCapAudio, eAAC_LDCapCode) )
	{
		PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeSipfromTipToNonTip - change remote caps to local");

		m_pSipRemoteCaps->CleanAll();

		*m_pSipRemoteCaps = *pNewLocalCaps;
	}*/

	m_pIpInitialMode->SetAudioAlg(eAAC_LDCapCode,cmCapReceiveAndTransmit);
	pBestMode = m_pSipRemoteCaps->FindBestMode(cmCapReceiveAndTransmit, ((const CSipComMode&)*m_pIpInitialMode), ((const CSipCaps&)*pNewLocalCaps),bWithinProtocolLimitation/*fix protocol*/, m_bIsOfferer, FALSE);

	PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeSipfromTipToNonTip from conf to party in order to start a new reinvite best mode with video -non tip ");
	*m_pSipLocalCaps    = *pNewLocalCaps;
	if(pBestMode && pBestMode->IsMediaOn(cmCapAudio,cmCapTransmit) && pBestMode->IsMediaOn(cmCapAudio,cmCapReceive))
	{
	    m_pIpInitialMode->SetMediaMode(pBestMode->GetMediaMode(cmCapAudio, cmCapTransmit),cmCapAudio, cmCapTransmit) ;
	    m_pIpInitialMode->SetMediaMode(pBestMode->GetMediaMode(cmCapAudio, cmCapReceive),cmCapAudio, cmCapReceive) ;
	}
	m_isTipFallbackFlow = YES;
	//VNGR 23501
	/*
	if(pBestMode && pBestMode->IsMediaOn(cmCapVideo,cmCapTransmit) )
	{
		PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeSipfromTipToNonTip from conf to party in order to start a new reinvite best mode with video -non tip ");
		*m_pIpInitialMode 	= *pBestMode;
		*m_pSipLocalCaps 	= *pNewLocalCaps;
	}
	else //no best mode try to cut with TIP SCM
	{
		PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeSipfromTipToNonTip staying with tip caps ");
		pBestMode = m_pSipRemoteCaps->FindBestMode(cmCapReceiveAndTransmit, ((const CSipComMode&)*m_pIpInitialMode), ((const CSipCaps&)*m_pSipLocalCaps),bWithinProtocolLimitation, m_bIsOfferer);
	}

	if (pBestMode)
		*m_pIpInitialMode = *pBestMode;
	*/

	//if there is no best mode stary with original initialmode
	WORD tempState 	= m_state;
	m_state 		= PARTY_RE_CAPS;

	eChangeModeInitiator lastChangeMode = m_changeModeInitiator;
	m_changeModeInitiator 				= eFallBackFromTip;

//	m_pSipLocalCaps->RemoveH264SpecifProfileCapSet(eH264CapCode,kRolePeople,H264_Profile_Main);
//	m_pSipLocalCaps->RemoveCapSet(eAAC_LDCapCode);

//	m_pIpInitialMode->SetTipMode(eTipModeNone);
//	m_pIpInitialMode->SetTipAuxFPS(eTipAuxNone);


	BYTE bIsNeedToChangeVideoRsrc 		= ChangeVideoBrdgRsrcIfNeeded();//DPA

	if(!bIsNeedToChangeVideoRsrc)
	{
		m_changeModeInitiator 	= lastChangeMode;

		m_state 				= tempState;

		m_pPartyApi->ChangeModeIp(m_pIpInitialMode, eFallBackFromTipToSip, NO/*content speaker*/, m_pSipLocalCaps);
	}

	POBJDELETE(pBestMode);
}
//////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyRemoteConnectedReallocRsrc(CSegment* pParam)
{
		PTRACE2INT(eLevelInfoNormal,"CSipChangeModePartyCntl::OnPartyRemoteConnectedReallocRsrc : is ", m_changeModeInitiator);
}

void CSipChangeModePartyCntl::OnPartyReCapsPartyReallocRsrc(CSegment* pParam)
{
	HandlePartyReCapParamsInFallback(pParam);
	m_bRestartSipfromTipToNonTip = TRUE;
}
////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::HandlePartyReCapParamsInFallback(CSegment* pParam)
{
	BYTE isRecaps = 0;

	*pParam >> isRecaps;
	PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::HandlePartyReCapParamsInFallback m_pSipRemoteCaps->IsCapableTipAux5Fps ", m_pSipRemoteCaps->IsCapableTipAux5Fps());
	if(isRecaps)
		m_pSipRemoteCaps->DeSerialize(NATIVE,*pParam);
	PTRACE2INT(eLevelInfoNormal, "CSipChangeModePartyCntl::HandlePartyReCapParamsInFallback : after deserialize - m_pSipRemoteCaps->IsCapableTipAux5Fps ", m_pSipRemoteCaps->IsCapableTipAux5Fps());
}
/////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::OnPartyFallbackFromIceToSip(CSegment* pParam)
{
	m_pTaskApi->SendFallbackFromIceToSipPartyToConf(GetName());
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyCntl::ChangeSipfromIceToNoneIce(PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::ChangeSipfromIceToNoneIce from conf to party in order to start a new reinvite");

	CConfParty *pConfParty =  partyControInitParam.pConfParty;
	CIpComMode*	pPartyScm = NewAndGetPartyCntlScmForFallback(partyControInitParam, partyControlDataParams);

	SetSeviceIdForConfParty(pConfParty);

	CSipCaps* pSipCaps = new CSipCaps;
	DWORD vidBitrate;
	SetSIPPartyCapsAndVideoParam(pPartyScm, pSipCaps, pConfParty, vidBitrate,NO /*((CSipNetSetup*)pIpNetSetup)->GetEnableSipICE()*/,NULL,  0, FALSE, pConfParty->GetServiceId(), partyControInitParam, partyControlDataParams);

	*m_pIpInitialMode 	= *pPartyScm;
	*m_pSipLocalCaps 	= *pSipCaps;

	CSipComMode* pBestMode =NULL;

	BYTE bWithinProtocolLimitation = FALSE;

	pBestMode = m_pSipRemoteCaps->FindBestMode(cmCapReceiveAndTransmit, ((const CSipComMode&)*m_pIpInitialMode), ((const CSipCaps&)*m_pSipLocalCaps), bWithinProtocolLimitation/*fix protocol*/, m_bIsOfferer, FALSE);

	*m_pIpInitialMode = *pBestMode;

	m_pPartyApi->ChangeModeIp(m_pIpInitialMode, eFallBackFromIceToSip, NO/*content speaker*/, m_pSipLocalCaps);

	POBJDELETE(pPartyScm);
	POBJDELETE(pSipCaps);
	POBJDELETE(pBestMode);
}
//////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeSipfromIceToNoneIce(CIpComMode* pScm,CSipCaps* pNewLocalCaps)
{
	PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeSipfromIceToNoneIce from conf to party in order to start a new reinvite");

	*m_pIpInitialMode 		= *pScm;

	CSipComMode* pBestMode 	= NULL;

	BYTE bWithinProtocolLimitation = FALSE;

	pBestMode = m_pSipRemoteCaps->FindBestMode(cmCapReceiveAndTransmit, ((const CSipComMode&)*m_pIpInitialMode), ((const CSipCaps&)*pNewLocalCaps), bWithinProtocolLimitation/*fix protocol*/, m_bIsOfferer, FALSE);

	if(pBestMode)
	{
		*m_pIpInitialMode 	= *pBestMode;
		*m_pSipLocalCaps 	= *pNewLocalCaps;
	}
	else //no best mode try to cut with TIP SCM
	{
		pBestMode = m_pSipRemoteCaps->FindBestMode(cmCapReceiveAndTransmit, ((const CSipComMode&)*m_pIpInitialMode), ((const CSipCaps&)*m_pSipLocalCaps),bWithinProtocolLimitation/*fix protocol*/, m_bIsOfferer, FALSE);
	}

	if(pBestMode)
		*m_pIpInitialMode = *pBestMode;
	//if there is no best mode stary with original initial mode

	WORD tempState 	= m_state;
	m_state 		= PARTY_RE_CAPS;

	eChangeModeInitiator lastChangeMode = m_changeModeInitiator;
	m_changeModeInitiator 				= eFallBackFromIce;

	BYTE bIsNeedToChangeVideoRsrc 		=	ChangeVideoBrdgRsrcIfNeeded();//DPA

	if(!bIsNeedToChangeVideoRsrc)
	{
		m_changeModeInitiator 	= lastChangeMode;

		m_state 				= tempState;

		m_pPartyApi->ChangeModeIp(m_pIpInitialMode, eFallBackFromIceToSip, NO/*content speaker*/);
	}

	POBJDELETE(pBestMode);
}

void CSipChangeModePartyCntl::OnPartySendChannelHandleAnycase(CSegment* pParam)
{
    TRACEINTO << "Not changing: m_incomingVideoChannelHandle:" << m_incomingVideoChannelHandle << " m_outgoingVideoChannelHandle:" << m_outgoingVideoChannelHandle;
}

void CSipChangeModePartyCntl::OnPartySendChannelHandle(CSegment* pParam)
{
	unsigned int incomingVideoChannelHandle;
	unsigned int outgoingVideoChannelHandle;
	*pParam >> incomingVideoChannelHandle;
	*pParam >> outgoingVideoChannelHandle;
	bool incomingVideoChannelHandleChanged = false;

	if (m_incomingVideoChannelHandle != incomingVideoChannelHandle)
	{
		m_incomingVideoChannelHandle = incomingVideoChannelHandle;
		m_bVideoRelayInReady = FALSE;
		incomingVideoChannelHandleChanged = true;
	}
	if (m_outgoingVideoChannelHandle != outgoingVideoChannelHandle)
	{
		m_outgoingVideoChannelHandle = outgoingVideoChannelHandle;
		m_bVideoRelayOutReady = FALSE;
	}
	TRACEINTO << "m_incomingVideoChannelHandle:" << m_incomingVideoChannelHandle << " m_outgoingVideoChannelHandle:" << m_outgoingVideoChannelHandle;

	if (m_pIpInitialMode->GetConfMediaType() == eMixAvcSvc && incomingVideoChannelHandleChanged)
	{
	    m_bIsBridgeUpgradedVideo = false;
        TRACEINTO << "new m_bIsBridgeUpgradedVideo=" << (int)m_bIsBridgeUpgradedVideo;
	}

}

DWORD CSipChangeModePartyCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate(CSegment* pParam)
{
	    POBJDELETE(m_pMrmpRsrcParams);
            m_pMrmpRsrcParams=NULL;

	    for (int i=0; i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++i)
	    {
                POBJDELETE(m_avcToSvcTranslatorRsrcParams[i]);
	        m_avcToSvcTranslatorRsrcParams[i] = NULL;
	    }

	DWORD status = CIpPartyCntl::OnRsrcReAllocatePartyRspAdditionalReAllocate(pParam, m_avcToSvcTranslatorRsrcParams, m_pMrmpRsrcParams,ALLOCATION_TYPE_UPGRADE);

	if (status != STATUS_OK)
	{// disconnect party
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, RESOURCES_DEFICIENCY);

		return status;
	}

	m_eChangeModeState = eConfRequestMoveToMixed;
	m_state = CONF_REQUEST_PARTY_CHANGE_MODE;
	m_pPartyApi->ChangeModeIp(m_pIpInitialMode,m_eChangeModeState, NO/*content speaker*/, m_pSipLocalCaps, m_avcToSvcTranslatorRsrcParams, m_pMrmpRsrcParams);

	return STATUS_OK;
}

void CSipChangeModePartyCntl::OnPartyUpgradeToMixTout()
{
	if(m_bIsMrcCall)
	{
		TRACEINTO<<"!@# dynMixedErr got timeout on svc party upgrade to mixed";
	}
	else
	{
		TRACEINTO<<"!@# dynMixedErr got timeout on avc party upgrade to mixed";
	}
	//m_pPartyApi->SipPartyCallFailed(SIP_INTERNAL_MCU_PROBLEM);//
	m_state = IDLE;
	m_pPartyApi->SipPartyCallFailed(SIP_TOUT_DURING_UPGRADE_TO_MIXED);
	PASSERTMSG((DWORD)statIllegal,"CSipChangeModePartyCntl::OnPartyUpgradeToMixTout");
//	EndTransaction(statIllegal); ey_20866 need error handling here
}


void CSipChangeModePartyCntl::OnUpgradePartyToMixed(CSegment* pParam)
{
	TRACEINTO << "mix_mode: started";
	m_state =CHANGE_BRIDGES;
	CIpPartyCntl::OnUpgradePartyToMixed(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::AddContentToScm()
{
	PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::AddContentToScm");

	if( m_IsAsSipContentEnable  && IsRemoteAndLocalCapSetHasBfcpUdp()
			&& IsFirstContentNegotiation()&& !IsRemoteAndLocalCapSetHasContent(eToPrintOnFalseOnly))
	{
			PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::AddContentToScm - will add content caps");


			if (m_pIpInitialMode->IsMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
			{
				PTRACE(eLevelInfoNormal,"CSipChangeModePartyCntl::AddContentToScm : Startchange mode to add content");

				CapEnum contentProtocol = m_pSipLocalCaps->GetBestContentProtocol();

				BYTE hd1080Mpi = 0;
				BYTE isHD1080Supported = FALSE;
				BYTE hdMpi = 0;
				BYTE hd720Mpi = 0;
				BYTE isHighProfileContent = FALSE;

				hd1080Mpi = m_pSipLocalCaps->IsCapableOfHDContent1080();

				if(!hd1080Mpi)
				{
					hd720Mpi = m_pSipLocalCaps->IsCapableOfHDContent720();
					hdMpi = hd720Mpi;
				}
				else
				{
					isHD1080Supported = TRUE;
					hdMpi = hd1080Mpi;
				}

				isHighProfileContent = m_pSipLocalCaps->IsHighProfileContent();

				m_pIpInitialMode->SetContent(/*contentRate*/0, cmCapReceiveAndTransmit,contentProtocol,isHD1080Supported,hdMpi,isHighProfileContent);

				m_bIsNewScm = TRUE;

				CSdesCap *pSdesCap = m_pSipLocalCaps->GetSdesCap(cmCapVideo, kRolePresentation);

				if ((m_pIpInitialMode->GetIsEncrypted() == Encryp_On) && pSdesCap)
				{
					m_pIpInitialMode->SetSipSdes(cmCapVideo, cmCapReceive, kRolePresentation, pSdesCap); // Sdes for receive direction will be updated after remote will send its 200ok
					m_pIpInitialMode->SetSipSdes(cmCapVideo, cmCapTransmit, kRolePresentation, pSdesCap);
				}

				POBJDELETE(pSdesCap);

				m_pIpInitialMode->Dump("CSipChangeModePartyCntl::AddContentToScm", eLevelInfoNormal);

				DispatchChangeModeEvent();
			}



	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::ChangeSipfromTipToNonTip(CIpComMode* pScm,PartyControlInitParameters& partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	PTRACE(eLevelInfoNormal,"CSipPartyCntl::ChangeSipfromTipToNonTip from conf to party in order to start a new reinvite");

	CConfParty *pConfParty =  partyControInitParam.pConfParty;
	CIpComMode*	pPartyScm = NewAndGetPartyCntlScmForFallback(partyControInitParam, partyControlDataParams);

 	SetSeviceIdForConfParty(pConfParty);

 	EVideoResolutionType videoResolutionType = GetMaxResolutionAccordingToVideoModeType((Eh264VideoModeType)pConfParty->GetMaxResolution());
 	pConfParty->SetMaxResolution(videoResolutionType);

	CSipCaps* pSipCaps = new CSipCaps;
	DWORD vidBitrate;
	SetSIPPartyCapsAndVideoParam(pPartyScm, pSipCaps, pConfParty, vidBitrate,NO /*((CSipNetSetup*)pIpNetSetup)->GetEnableSipICE()*/,NULL,  0, FALSE, pConfParty->GetServiceId(), partyControInitParam, partyControlDataParams);

	if (pScm->GetIsEncrypted())
	{
		CopyNoneTipEncryptionParams(pPartyScm, pScm);
	}

	if (pScm->IsMediaOn(cmCapBfcp, cmCapReceive))
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::ChangeSipfromTipToNonTip: copying bfcp receive");
		pPartyScm->SetMediaMode(pScm->GetMediaMode(cmCapBfcp, cmCapReceive), cmCapBfcp, cmCapReceive);
		pSipCaps->CleanMedia(cmCapBfcp); // bfcp
		pSipCaps->SetBfcp(pPartyScm, GetName());
	}

	if (pScm->IsMediaOn(cmCapBfcp, cmCapTransmit))
	{
		PTRACE(eLevelInfoNormal,"CSipPartyCntl::ChangeSipfromTipToNonTip: copying bfcp transmit");
		pPartyScm->SetMediaMode(pScm->GetMediaMode(cmCapBfcp, cmCapTransmit), cmCapBfcp, cmCapTransmit);
	}

	*m_pIpInitialMode 	= *pPartyScm;
	*m_pSipLocalCaps 	= *pSipCaps;

	if(!m_bIsOfferer)
	{
		CSipComMode* pBestMode = NULL;
		BYTE bWithinProtocolLimitation = FALSE;

		m_pIpInitialMode->SetAudioAlg(eAAC_LDCapCode,cmCapReceiveAndTransmit);

		m_isTipFallbackFlow = YES;

		pBestMode = m_pSipRemoteCaps->FindBestMode(cmCapReceiveAndTransmit, ((const CSipComMode&)*m_pIpInitialMode), ((const CSipCaps&)*m_pSipLocalCaps), bWithinProtocolLimitation/*fix protocol*/, m_bIsOfferer, FALSE);

		*m_pIpInitialMode = *pBestMode;

		POBJDELETE(pBestMode);
	}

	PTRACE2INT(eLevelInfoNormal,"CSipChangeModePartyCntl::ChangeSipfromTipToNonTip - bfcp transport ", m_pSipLocalCaps->GetBfcpTransportType());


	if (m_pPartyApi)
		m_pPartyApi->ChangeModeIp(m_pIpInitialMode, eFallBackFromTipToSip, NO/*content speaker*/, m_pSipLocalCaps);

	POBJDELETE(pPartyScm);
	POBJDELETE(pSipCaps);

}



/////////////////////////////////////////////////////////////////////
CSipChangeModeLyncPartyCntl::CSipChangeModeLyncPartyCntl()
{
	m_bIsUpdateLegacyDueToPlugin = false;

	VALIDATEMESSAGEMAP
}
///////////////////////////////////////////////////
CSipChangeModeLyncPartyCntl::~CSipChangeModeLyncPartyCntl()
{

}
/////////////////////////////////////////////////////////
const char*   CSipChangeModeLyncPartyCntl::NameOf() const
{
  return "CSipChangeModeLyncPartyCntl";
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipChangeModeLyncPartyCntl::DisconnectForUpdateLegacyStatus()
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModeLyncPartyCntl::DisconnectForUpdateLegacyStatus : Name - ", m_partyConfName, GetPartyRsrcId());

	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	if( pCommConf && !pCommConf->IsLegacyShowContentAsVideo() )
	{
		PTRACE(eLevelInfoNormal, "CSipChangeModeLyncPartyCntl::DisconnectForUpdateLegacyStatus - conf without legacy");
		return FALSE;
	}

	BYTE bIsDisconnectFromVideoBridge = DisconnectPartyFromVideoBridgeForLync();

	if (bIsDisconnectFromVideoBridge)
	{
		PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModeLyncPartyCntl::DisconnectForUpdateLegacyStatus: updating video brdige, legacy to content Name - ", m_partyConfName, GetPartyRsrcId());
		m_state = UPDATE_LEGACY_STATUS;
		return TRUE;
	}
	return FALSE;
}


void CSipChangeModeLyncPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipChangeModeLyncPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus: ", m_partyConfName, GetPartyRsrcId());
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CSipChangeModeLyncPartyCntl::OnVideoBrdgConnectedUpdateLegacyStatus : Connect has received after disconnect.", GetPartyRsrcId());
	}
	else
	{
		HandleVideoBridgeConnectedInd(pParam);
	}
	m_state = IDLE;
}

void CSipChangeModeLyncPartyCntl::UpdateLegacyContentStatus(BYTE isBlockContent)
{
	PTRACE(eLevelInfoNormal, "CSipChangeModePartyCntl::UpdateLegacyContentStatus");
	if(isBlockContent == GetIsBlockContentForLegacy())
	{
		PTRACE(eLevelInfoNormal, "CSipChangeModePartyCntl::UpdateLegacyContentStatus -- No need.");
		return;
	}

	if (TRUE == AreTwoDirectionsConnectedToVideoBridge())
	{
		DisconnectForUpdateLegacyStatus();
	}

	SetIsBlockContentForlegacy(isBlockContent);
}

/////////////////////////////////////////////////////////////////////////////
void CSipChangeModeLyncPartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal, "CSipChangeModeLyncPartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus : Name - ", m_partyConfName, GetPartyRsrcId());

	if(m_bIsUpdateLegacyDueToPlugin)
	{
		BYTE bIsConnectToVideoBridge = IsNeedToConnectToVideoBridge(m_pIpCurrentMode);
		if (bIsConnectToVideoBridge)
		{
			ConnectPartyToVideoBridge(m_pIpCurrentMode);
		}
		m_bIsUpdateLegacyDueToPlugin = false;
	}
	else
	{
		CSipChangeModePartyCntl::OnVideoBrdgDisconnectedUpdateLegacyStatus(pParam);
	}
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipChangeModeLyncPartyCntl::DisconnectPartyFromVideoBridgeForLync()
{
	EMediaDirection eDisconnectedDirection = eMediaInAndOut;

	PTRACE2PARTYID(eLevelInfoNormal,"CIpPartyCntl::DisconnectPartyFromVideoBridgeForLync ", m_partyConfName, GetPartyRsrcId());


	if (TRUE != AreTwoDirectionsConnectedToVideoBridge())
	{
		PTRACE(eLevelInfoNormal,"CIpPartyCntl::DisconnectPartyFromVideoBridgeForLync, video not connected!!! ");
		return FALSE;
	}

	eDisconnectedDirection |= eMediaIn;
	int eInDisconnecting = ~eInConnected;
	m_eVidBridgeConnState &= (EBridgeConnectionState)eInDisconnecting;

	eDisconnectedDirection |= eMediaOut;
	int eOutDisconnecting = ~eOutConnected;
	m_eVidBridgeConnState &= (EBridgeConnectionState)eOutDisconnecting;


	CBridgePartyDisconnectParams bridgePartyDisconnectParams(GetPartyId(), eDisconnectedDirection);

	m_bIsUpdateLegacyDueToPlugin = true;
	m_pConfAppMngrInterface->DisconnectPartyVideo(&bridgePartyDisconnectParams);
	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnMSFocusEndDisConnection(CSegment* pParam)
{
	PTRACE(eLevelError, "CSipChangeModePartyCntl::OnMSFocusEndDisConnection");
	m_EndFocus = TRUE;

}
/////////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::ActiveMedia()
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipChangeModePartyCntl::ActiveMedia", GetPartyRsrcId());
	m_pPartyApi->ActiveMediaForAvMcuLync();

}
//////////////////////////////////////////////////////////////////////////
void CSipChangeModePartyCntl::OnPartyLastTargetModeMsg(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnPartyUpdateVideoAfterVsrMsg : Name - ",m_partyConfName, GetPartyRsrcId());
	m_pIpInitialMode->DeSerialize(NATIVE,*pParam);
}
