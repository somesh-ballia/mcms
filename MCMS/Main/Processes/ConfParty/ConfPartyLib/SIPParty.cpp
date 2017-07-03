//+========================================================================+
//                            SIPParty.cpp                                 |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPParty.cpp                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+
#include "Segment.h"
#include "StateMachine.h"

#include "Trace.h"

#include "SysConfigKeys.h"
#include "SysConfig.h"

#include "NStream.h"

#include "NetSetup.h"
#include "StatusesGeneral.h"
#include "Conf.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"

#include "IpMfaOpcodes.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "MediaTypeManager.h"
#include "TaskApi.h"
#include "PartyApi.h"
#include "ConfApi.h"
#include "SIPParty.h"
#include "CsInterface.h"
#include "SipDefinitions.h"
#include "SIPCommon.h"
#include "IpNetSetup.h"
#include "SipNetSetup.h"
#include "SipScm.h"
#include "SIPControl.h"
#include "IpWebRtcReq.h"
#include "IpWebRtcInd.h"
#include "SIPControlWebRtc.h"

#include "SIPTransaction.h"
#include "SIPTransReInviteNoSdpInd.h"
#include "SIPTransReInviteWithSdpInd.h"
#include "SIPTransInviteNoSdpInd.h"
#include "SIPTransInviteWithSdpInd.h"
#include "SIPTransInviteWithSdpReq.h"
#include "SIPTransInviteMrcWithSdpInd.h"
#include "SIPTransInviteMrcNoSdpInd.h"
#include "SIPTransInviteMrcSlaveWithSdpReq.h"
#include "SIPTransRTCPVideoUpdateInd.h"
#include "SIPTransRTCPVsrInd.h"
#include "SIPTransInviteWebRtcWithSdpInd.h"
#include "IpServiceListManager.h"
#include "SystemFunctions.h"
#include "H323StrCap.h"
#include "SIPTransReInviteWithSdpReq.h"
#include "IVRCntl.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "TipUtils.h"

#include "OpcodesMcmsCardMngrTIP.h"
#include "ScpHandler.h"
#include "ScpNotificationWrapper.h"
#include "StlUtils.h"
#include "ScpPipeMappingNotification.h"
#include "SipTransSvcUpgradeToMixed.h"
#include "SipTransAvcUpgradeToMixed.h"

#include "IVRCntlLocal.h"
#include "IVRService.h"

#include "PrecedenceSettings.h"
#include "SIPTransReInviteMrcWithSdpReq.h"  //FSN-613: Dynamic Content for SVC/Mix Conf

extern CPrecedenceSettings* GetpPrecedenceSettingsDB();

#include <stdlib.h>
#include "EnumsToStrings.h"
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern CIpServiceListManager* GetIpServiceListMngr();
extern const char* feccKeyToString(feccKeyEnum key);
extern WORD IsValidIpV4Address(const char* pIP);

PBEGIN_MESSAGE_MAP(CSipParty)

ONEVENT(SIP_CONF_DISCONNECT_CHANNELS,	PARTYCONNECTED,			CSipParty::OnConfDisconnectChannels)
ONEVENT(RMTH230,						PARTYCONNECTED ,		CSipParty::OnPartyRemoteH230)
ONEVENT(FORWARD_RMTH230,				PARTYCONNECTED ,		CSipParty::OnPartyForwardRemoteH230)
ONEVENT(STREAMS_INTRA_REQ,				PARTYCONNECTED ,		CSipParty::OnPartyStreamsIntraReq)


ONEVENT(SINGE_INTRA_AV_MCU,						PARTYCONNECTED ,		CSipParty::OnPartySinglIntraAvMcu)
ONEVENT(SINGE_INTRA_AV_MCU,						ANYCASE ,				CSipParty::NullActionFunction)

// handle re-caps (Remote Re-Invite)
ONEVENT(SIP_PARTY_RECEIVED_REINVITE,	    PARTYCONNECTED,		CSipParty::OnPartyReceivedReInviteConnected)
ONEVENT(SIP_PARTY_UPDATE_VIDEO_PREFERENCE,	PARTYCONNECTED,		CSipParty::OnPartyReceivedVideoPreference)
ONEVENT(VIDEO_BRIDGE_UPDATED_WITH_NEW_RES,	ANYCASE,		    CSipParty::OnConfVidBrdgUpdatedWithNewRes)
ONEVENT(SIP_PARTY_VSR_MSG_IND,				PARTYCONNECTED,		CSipParty::OnPartyReceivedVsrInd)
ONEVENT(SIP_MSFT_OUTSLAVES_CREATED,			PARTYCONNECTED,		CSipParty::OnPartyMsftOutslavesCreated)

//BRIDGE-6350
ONEVENT(SIP_SEND_VIDEO_PREFERENCE_END_TRANS_TOUT   ,PARTYSETUP,			CSipParty::OnSendVideoPreferenceReqTout)
ONEVENT(SIP_SEND_VIDEO_PREFERENCE_END_TRANS_TOUT   ,PARTYCONNECTED,		CSipParty::OnSendVideoPreferenceReqTout)

ONEVENT(SIP_PARTY_CALL_CLOSED,			PARTYDISCONNECTING,		CSipParty::OnPartyCallClosed)
ONEVENT(SIP_PARTY_CALL_FAILED,			PARTYDISCONNECTING,		CSipParty::OnPartyCallFailedDisconnecting)
ONEVENT(SIP_PARTY_CALL_REINVITE,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_PARTY_REINVITE_RESPONSE,	PARTYDISCONNECTING,		CSipParty::OnPartyReInviteResponseDisconnecting)
ONEVENT(SIP_PARTY_RECEIVED_ACK,			PARTYDISCONNECTING,		CSipParty::OnPartyReceivedAckDisconnecting)
ONEVENT(SIP_PARTY_TRANSPORT_ERROR,		PARTYDISCONNECTING,		CSipParty::OnPartyTransportErrorDisconnecting)
ONEVENT(PARTYDISCONNECTTOUT,			PARTYDISCONNECTING,		CSipParty::OnPartyDisconnectToutDisconnecting)
ONEVENT(SIP_PARTY_BAD_STATUS,			PARTYDISCONNECTING,		CSipParty::OnPartyBadStatusDisconnecting)
ONEVENT(PARTYCONNECTTOUT,				PARTYDISCONNECTING,		CSipParty::OnPartyConnectToutDisconnecting)
ONEVENT(IP_RTP_DIFF_PAYLOAD_TYPE,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,   PARTYDISCONNECTING,     CSipParty::NullActionFunction)
ONEVENT(SIP_PARTY_CHANS_UPDATED,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_SEND_INTRA_AFTER_TRANS_TOUT   ,PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_PARTY_UNMUTE_CHANNEL          ,PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(START_VIDEO_PREFERENCE            ,PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(UNMUTE_MEDIA_AFTER_RESUME_TIMER   ,PARTYDISCONNECTING,		CSipParty::NullActionFunction)

// BRIDGE-2017
ONEVENT(SIP_PARTY_BFCP_MSG_IND            ,PARTYDISCONNECTING,		CSipParty::NullActionFunction)

//ONEVENT(SIP_PARTY_CHANS_DISCONNECTED            ,PARTYCONNECTED,      CSipParty::NullActionFunction)

// Most of the events caught in the ANYCASE state actually can arrive only in some sub-state group
// (like connecting, change mode and open out channels) but the handle is the same in all of the cases and if there
// is a different treatment in special state it is handled before the ANYCASE state.
ONEVENT(IP_RTP_DIFF_PAYLOAD_TYPE,		ANYCASE,				CSipParty::OnPartyDifferentPayloadAnycase)

ONEVENT(IPPARTYMONITORING,				ANYCASE,				CSipParty::OnSipPartyMonitoring)
ONEVENT(IPLOGICALCHANNELCONNECT,		ANYCASE,				CSipParty::OnSipLogicalChannelConnect)
ONEVENT(VIDREFRESH,						ANYCASE,				CSipParty::OnVidBrdgRefreshAnycase)
ONEVENT(IPPARTYMONITORINGREQ,			ANYCASE,				CSipParty::OnMcuMngrPartyMonitoringReqAnycase)
ONEVENT(IP_DTMF_INPUT_IND,				ANYCASE,				CSipParty::OnPartyDtmfIndAnycase)
ONEVENT(NUMBERINGMESSAGE,				ANYCASE,				CSipParty::OnBridgeNumberingMessageAnycase)
ONEVENT(SETMOVEPARAMS,					ANYCASE,				CSipParty::OnConfSetMoveParams)
ONEVENT(UPDATERSRCCONFID,				ANYCASE,				CSipParty::OnPartyUpdateConfRsrcIdForInterfaceAnycase)
ONEVENT(PARTY_FAULTY_RSRC,				ANYCASE,				CSipParty::OnPartySendFaultyMfaToPartyCntlAnycase)
ONEVENT(VIDBRDGCONNECT,					ANYCASE,				CSipParty::OnConfPartyReceiveVidBridgeConnectedAnycase)
ONEVENT(SIP_PARTY_CALL_FAILED,			ANYCASE,				CSipParty::OnPartyCallFailed)
ONEVENT(SIP_PARTY_CALL_REINVITE,		ANYCASE,				CSipParty::OnPartyCallReinvite)
ONEVENT(SIP_PARTY_TRANSPORT_ERROR,		ANYCASE,				CSipParty::OnPartyTransportError)
ONEVENT(SIP_PARTY_RMT_CLOSE_CALL,		ANYCASE,				CSipParty::OnPartyRemoteCloseCall)
ONEVENT(SIP_PARTY_BAD_STATUS,			ANYCASE,				CSipParty::OnPartyBadStatus)
ONEVENT(PARTYCONNECTTOUT,				ANYCASE,				CSipParty::OnPartyConnectTout)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,		ANYCASE,				CSipParty::OnPartyChannelsConnected)
ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,   ANYCASE,                CSipParty::OnPartyChannelsDisConnected)
ONEVENT(SIP_PARTY_CHANS_UPDATED,		ANYCASE,				CSipParty::UpdateDbOnChannelsConnected)
ONEVENT(SIP_RATE_CHANGE_BY_REMOTE,		ANYCASE,				CSipParty::OnChangeRate)
ONEVENT(SIP_PARTY_UNMUTE_CHANNEL        ,ANYCASE,		        CSipParty::OnSipPartyUnMuteClosingChannel)
ONEVENT(START_VIDEO_PREFERENCE          ,ANYCASE,		        CSipParty::OnPartyCntlStartVideoPreference)
ONEVENT(SIP_PARTY_SCP_REQUEST_FROM_EP,	 ANYCASE,				CSipParty::OnScpRequestFromEp)
ONEVENT(SIP_PARTY_SCP_NOTIFICATION_IND_FROM_EP,	 ANYCASE,		CSipParty::OnScpNotificationIndFromEp)
ONEVENT(UPDATE_SCM_STREAMS,              ANYCASE,               CSipParty::OnUpdateScmStreams)
ONEVENT(UNMUTE_MEDIA_AFTER_RESUME_TIMER   ,ANYCASE,		        CSipParty::ResumeMediaAndSendReinviteForTip)
ONEVENT(UPDATE_ART_WITH_SSRC_REQ,        ANYCASE,               CSipParty::OnUpdateArtWithSsrc)

// null function calls to prevent exception trace.
ONEVENT(RMTH230,						ANYCASE ,				CSipParty::NullActionFunction)
ONEVENT(FORWARD_RMTH230,				ANYCASE ,				CSipParty::NullActionFunction)

ONEVENT(AUDBRDGCONNECT,					PARTYDISCONNECTING,		CSipParty::NullActionFunction)

// if conf doesn't respond (timer pop up) after telling him to start disconnection (TellConfOnDisconnecting),
// start party disconnection process.
ONEVENT(CONFDISCONNECTOUT,				PARTYDISCONNECTING,		CSipParty::OnConfReadyToCloseCall)

// Transaction messages:
ONEVENT(SIP_TRANS_END_TRANSACTION,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_MUTE_MEDIA,			PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_PARTY_RECEIVE_RECAP,	PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_UPDATE_CHANS_STATUS,	PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_CHANS_CONNECTED,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_REMOTE_CAPS_RECEIVED,	PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_REMOTE_CONNECTED,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_ORIGINAL_RMOTCAP,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_SITE_VISUAL_NAME,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_UPDATE_ENC_STATUS,	PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_PARTY_CHANGE_VIDEO_AFTER_VSR_MSG, PARTYDISCONNECTING,			CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_SINGLE_UPDATE_PACSI_INFO_MSG, PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_PARTY_CHANGE_VIDEO_RES, PARTYDISCONNECTING,			CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_UPDATE_DB,			PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_DISCONNECT_BRIDGES,	PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_CONNECT_BRIDGES,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_PARTY_LAST_TARGET_MODE_MSG, PARTYDISCONNECTING,			CSipParty::NullActionFunction)


ONEVENT(SIP_TRANS_END_TRANSACTION,		ANYCASE,				CSipParty::OnTransEndTransactionAnycase)
ONEVENT(SIP_TRANS_MUTE_MEDIA,			ANYCASE,				CSipParty::OnTransMuteMediaAnycase)
ONEVENT(SIP_TRANS_PARTY_RECEIVE_RECAP,	ANYCASE,				CSipParty::OnTransReceiveReCapAnycase)
ONEVENT(SIP_TRANS_UPDATE_CHANS_STATUS,	ANYCASE,				CSipParty::OnTransUptateChannelsStatusAnycase)
ONEVENT(SIP_TRANS_CHANS_CONNECTED,		ANYCASE,				CSipParty::OnTransChannelsConnectedAnycase)
ONEVENT(SIP_TRANS_REMOTE_CAPS_RECEIVED,	ANYCASE,				CSipParty::OnTransRemoteCapsReceivedAnycase)
ONEVENT(SIP_TRANS_REMOTE_CONNECTED,		ANYCASE,				CSipParty::OnTransRemoteConnectedAnycase)
ONEVENT(SIP_TRANS_SEND_CHANNEL_HANDLE,	ANYCASE,				CSipParty::OnTransSendChannelHandle)
ONEVENT(SIP_TRANS_ORIGINAL_RMOTCAP,		ANYCASE,				CSipParty::OnTransOriginalRemoteCapsAnycase)
ONEVENT(SIP_TRANS_SITE_VISUAL_NAME,		ANYCASE,				CSipParty::OnTransSiteVisualNameAnycase)
ONEVENT(SIP_TRANS_UPDATE_ENC_STATUS,	ANYCASE,				CSipParty::OnTransUpdateEncStatusAnycase)
ONEVENT(SIP_TRANS_PARTY_CHANGE_VIDEO_RES, ANYCASE,  			CSipParty::OnTransChangeVideoResAnycase)
ONEVENT(SIP_TRANS_PARTY_CHANGE_VIDEO_AFTER_VSR_MSG, ANYCASE,	CSipParty::OnTransChangeVideoAfterVsrMsgAnycase)
ONEVENT(SIP_TRANS_SINGLE_UPDATE_PACSI_INFO_MSG,     ANYCASE,	CSipParty::OnTransSendSingleUpdatePacsiInfoAnycase)
ONEVENT(SIP_TRANS_UPDATE_DB,			ANYCASE,				CSipParty::OnTransUpdateDbAnycase)
ONEVENT(SIP_TRANS_DISCONNECT_BRIDGES,	ANYCASE,				CSipParty::OnTransDisconnectBridgesAnycase)
ONEVENT(SIP_TRANS_CONNECT_BRIDGES,		ANYCASE,				CSipParty::OnTransConnectBridgesAnycase)
ONEVENT(SIP_TRANS_PARTY_LAST_TARGET_MODE_MSG, ANYCASE,			CSipParty::OnTransLastTargetModeMsgAnycase)

//for TIP
ONEVENT(SIP_TRANS_SEND_TOKEN_RELEASE_FOR_HOLD,	ANYCASE,        CSipParty::OnTransSendTokenReleaseForHoldAnycase)

// Not relevat to SIP party only in ISDN
ONEVENT(FREEZPIC, 						ANYCASE,				CSipParty::NullActionFunction)
// Token request/release event
ONEVENT(IPDATATOKENMSG,	    			PARTYCONNECTED,			CSipParty::OnIpDataTokenMsg)
ONEVENT(IPDATATOKENMSG,	    			ANYCASE,				CSipParty::NullActionFunction)

ONEVENT(IPFECCKEYMSG,		    		PARTYCONNECTED,			CSipParty::OnIpFeccKeyMsg)
ONEVENT(IPFECCKEYMSG,	   	 			ANYCASE,				CSipParty::NullActionFunction)

//Data token events from fecc bridge
ONEVENT(DATATOKENREQ,		    		PARTYCONNECTED,     	CSipParty::OnFeccBridgeTokenRequest)  // not in use
ONEVENT(DATATOKENACCEPT,				PARTYCONNECTED,     	CSipParty::OnFeccBridgeTokenAccept)
ONEVENT(DATATOKENREJECT,				PARTYCONNECTED,     	CSipParty::OnFeccBridgeTokenReject)
ONEVENT(DATATOKENWITHDRAW,				PARTYCONNECTED,     	CSipParty::OnFeccBridgeTokenWithdraw)
ONEVENT(DATATOKENRELEASEREQ,			PARTYCONNECTED,     	CSipParty::OnFeccBridgeTokenReleaseRequest)
ONEVENT(DATATOKENRELEASE,	    		PARTYCONNECTED,     	CSipParty::OnFeccBridgeTokenRelease)

ONEVENT(DATATOKENREQ,		    		ANYCASE,     			CSipParty::NullActionFunction)  // not in use
ONEVENT(DATATOKENACCEPT,				ANYCASE,     			CSipParty::NullActionFunction)
ONEVENT(DATATOKENREJECT,				ANYCASE,     			CSipParty::NullActionFunction)
ONEVENT(DATATOKENWITHDRAW,				ANYCASE,     			CSipParty::NullActionFunction)
ONEVENT(DATATOKENRELEASEREQ,			ANYCASE,     			CSipParty::NullActionFunction)
ONEVENT(DATATOKENRELEASE,	    		ANYCASE,     			CSipParty::NullActionFunction)

//vsw/cop
ONEVENT(PARTY_FLOWCONTROL,              PARTYDISCONNECTING,     CSipParty::NullActionFunction)
ONEVENT(PARTY_FLOWCONTROL,              ANYCASE,        		CSipParty::SendFlowControlToCs)
ONEVENT(PARTY_REMOVE_SELF_FLOWCONTROL_CONSTRAINT, PARTYDISCONNECTING, CSipParty::NullActionFunction)
ONEVENT(PARTY_REMOVE_SELF_FLOWCONTROL_CONSTRAINT, ANYCASE,		CSipParty::OnRemoveSelfFlowControlConstraint)

ONEVENT(START_PREVIEW_PARTY,			ANYCASE, 				CSipParty::OnMcuMngrStartPartyPreviewReq)
ONEVENT(STOP_PREVIEW_PARTY,				ANYCASE,				CSipParty::OnMcuMngrStopPartyPreviewReq)
ONEVENT(REQUEST_PREVIEW_INTRA,			ANYCASE,				CSipParty::OnMcuMngrIntraPreviewReq)

ONEVENT(UPDATE_VIDEO_RATE ,				ANYCASE,				CSipParty::UpdatePartyVideoBitRate)
//Lpr
ONEVENT(LPR_CHANGE_RATE	 				,ANYCASE,				CSipParty::UpdatePartySipLprVideoBitRate)
ONEVENT(LPR_VIDEO_RATE_UPDATED			,ANYCASE,				CSipParty::OnLprUpdatedPartySIPVideoBitRate)
ONEVENT(LPR_PARTY_FLOWCONTROL			,ANYCASE,				CSipParty::SendLprFlowControlToCs)
// Change mode
ONEVENT(CONFCHANGEMODE,					PARTYCONNECTED,			CSipParty::OnConfChangeModeConnect)
// ppc
ONEVENT(UPDATE_PRESENTATION_OUT_STREAM,	PARTYCONNECTED,			CSipParty::OnPartyCntlUpdatePresentationOutStream)
ONEVENT(PRESENTATION_OUT_STREAM_UPDATED,PARTYCONNECTED,			CSipParty::OnPartyUpdatedPresentationOutStream)
ONEVENT(SIP_PARTY_BFCP_MSG_IND,			PARTYCONNECTED,			CSipParty::OnPartyBfcpMsgInd)
ONEVENT(SIP_PARTY_BFCP_DELAY_ACK_MSG,	sPARTY_CONNECTING,		CSipParty::OnPartyBfcpMsgDelayAckTimeout)//BRIDGE-13253
ONEVENT(SIP_PARTY_BFCP_DELAY_ACK_MSG,	PARTYCONNECTED,			CSipParty::OnPartyBfcpMsgDelayAck)
ONEVENT(SIP_PARTY_BFCP_TRANSPORT_IND,	PARTYCONNECTED,			CSipParty::OnPartyBfcpTransportInd)
ONEVENT(CONFCONTENTTOKENMSG,			PARTYCONNECTED,			CSipParty::OnContentBrdgTokenMsg)
ONEVENT(HW_CONTENT_ON_OFF_ACK,			PARTYCONNECTED,			CSipParty::OnRtpAckForContentOnOff)
ONEVENT(HW_CONTENT_ON_OFF_ACK,			ANYCASE,				CSipParty::NullActionFunction)
ONEVENT(RTP_EVACUATE_ON_OFF_ACK,		PARTYCONNECTED,			CSipParty::OnRtpAckForEvacuateContentStream)
ONEVENT(RTP_EVACUATE_ON_OFF_ACK,		ANYCASE,				CSipParty::NullActionFunction)
ONEVENT(RTPCONTENTACKTOUT,				PARTYCONNECTED,			CSipParty::OnRtpAckForContentTout)
ONEVENT(RTPCONTENTACKTOUT,				ANYCASE,				CSipParty::NullActionFunction)
ONEVENT(RTPEVACUATEACKTOUT,				PARTYCONNECTED,			CSipParty::OnRtpAckForEvacuateTout)
ONEVENT(RTPEVACUATEACKTOUT,				ANYCASE,				CSipParty::NullActionFunction)
ONEVENT(CONTENTVIDREFRESH,				ANYCASE,				CSipParty::OnContentBrdgRefreshVideo)
ONEVENT(CONTENTFREEZEPIC,				ANYCASE,				CSipParty::NullActionFunction) // Not relevant to Sip
ONEVENT(SIP_CONTENT_PROVIDER_TOUT,		PARTYCONNECTED,			CSipParty::OnContentProviderIdentityTimer)
ONEVENT(SIP_CONTENT_PROVIDER_TOUT,		ANYCASE,				CSipParty::NullActionFunction)
ONEVENT(CONTENTVIDSIMINDREFRESH,		ANYCASE,				CSipParty::OnContentBrdgSimulateIndRefreshVideo)

// Glare
ONEVENT(SIP_GLARE_TIMER,				PARTYCONNECTED,			CSipParty::OnSipGlareTimer)
ONEVENT(SIP_GLARE_TIMER,	    		ANYCASE,	     		CSipParty::NullActionFunction)
//Timer to send intra in case of pause/resume, stop/start video
ONEVENT(SIP_SEND_INTRA_AFTER_TRANS_TOUT,ANYCASE,				CSipParty::OnSendIntraAfterTransactionTout)

ONEVENT(LEADER_CHANGED,					PARTYDISCONNECTING,	CSipParty::NullActionFunction)
ONEVENT(LEADER_CHANGED,					ANYCASE,			CSipParty::OnSetPartyToLeader)
ONEVENT(TRANS_ICE_CONN_CHECK_COMPLETE_IND   , ANYCASE,      	CSipParty::NullActionFunction)
ONEVENT(SIP_TRANS_HANDLE_CONNECTIVITY_CHECK , ANYCASE,      	CSipParty::OnTransHandleIceConnectivityCheckComplete)
ONEVENT(ICE_CONN_CHECK_COMPLETE_IND , 	ANYCASE,      			CSipParty::OnTransHandleIceConnectivityCheckComplete)
ONEVENT(ICE_INSUFFICIENT_BANDWIDTH , 	ANYCASE,      			CSipParty::OnIceInsufficientBandwidthEvent)
ONEVENT(SIP_CONF_NID_CONFIRM_REQ , 		ANYCASE,      			CSipParty::OnSipConfNIDivrProviderEQ)

ONEVENT(UPDATE_PRESENTATION_OUT_STREAM,	ANYCASE,				CSipParty::OnPartyCntlUpdatePresentationOutStream)
ONEVENT(PRESENTATION_OUT_STREAM_UPDATED,ANYCASE,				CSipParty::OnPartyUpdatedPresentationOutStream)

// TIP
ONEVENT(TIP_CNTL_NEGOTIATION_RESULT,	PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(TIP_CNTL_NEGOTIATION_RESULT,	ANYCASE,				CSipParty::OnTipCntlNegotiationResult)
ONEVENT(TIP_CNTL_LAST_ACK_RECEIVED ,	ANYCASE,				CSipParty::OnReceivedTipLastAck)
ONEVENT(PARTYCONTROL_PARTY_MASTER_TO_SLAVE,	ANYCASE,			CSipParty::OnMessageFromMasterToSlave)
ONEVENT(PARTYCONTROL_PARTY_SLAVE_TO_MASTER,	ANYCASE,			CSipParty::OnMessageFromSlaveToMaster)
ONEVENT(SLAVE_SEND_RTCP_FAST_UPDATE,	ANYCASE,				CSipParty::OnSlaveSendRtcpFastUpdate)
ONEVENT(NEWNAMEWITHROOMID,				ANYCASE,				CSipParty::OnSendNewNameWithRoomId)
ONEVENT(SLAVEPARTY_RSRCID,				ANYCASE,				CSipParty::SetSlavePartyRsrcId)
ONEVENT(SLAVE_SEND_MONITORING_REQ,		ANYCASE,				CSipParty::OnSlaveSendMonitoringReq)
ONEVENT(CREATE_SLAVE_ACK_TOUT,			ANYCASE,				CSipParty::OnTimerCreateSlaveAck)
ONEVENT(TIP_SLAVE_RECAP_TOUT,			ANYCASE,				CSipParty::OnTimerSlaveRecapAck)
ONEVENT(SIP_CONF_BRIDGES_UPDATED,   	PARTYCONNECTED,    	    CSipParty::OnConfBridgesUpdated)
ONEVENT(TIP_CNTL_CONTENT_MSG_IND,		PARTYCONNECTED,		    CSipParty::OnTipCntlContentMsgInd)
ONEVENT(ICE_BANDWIDTH_IND,				ANYCASE,				CSipParty::OnIceBandwidthInd)
ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION,  ANYCASE,			CSipParty::OnConfSetCapsAccordingToNewAllocation)
ONEVENT(DISCONNECT_SLAVE_PARTY_TOUT,	ANYCASE,				CSipParty::OnTimerDisconnectSlaveParty)
ONEVENT(SIPALLOCATEDBANDWIDTHSTATUS		  ,ANYCASE,				CSipParty::OnSipBandwidthAllocationStatus)
ONEVENT(SIPBANDWIDTHREINVITENEEDED		  ,ANYCASE,				CSipParty::OnSipBandwidthReInviteNeeded)
ONEVENT(TIP_GRADUAL_TIMER,				ANYCASE,				CSipParty::OnGradualTout)
ONEVENT(TIP_GRADUAL_TIMER,				PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(TIP_WAIT_FALLBACK_TOUT,				PARTYDISCONNECTING,	CSipParty::NullActionFunction)
ONEVENT(TIP_WAIT_FALLBACK_TOUT,				ANYCASE,			CSipParty::OnWaitToStartTipFallBackTout)
ONEVENT(BFCP_START_REESTABLISH_CONNECTION,	PARTYDISCONNECTING,	CSipParty::NullActionFunction)
ONEVENT(BFCP_START_REESTABLISH_CONNECTION,		ANYCASE,		CSipParty::OnBfcpStartReestablishConnection)
ONEVENT(BFCP_END_REESTABLISH_CONNECTION,		PARTYDISCONNECTING,		CSipParty::NullActionFunction)
ONEVENT(BFCP_END_REESTABLISH_CONNECTION,		ANYCASE,		CSipParty::OnBfcpEndReestablishConnection)
ONEVENT(MAKE_OFFER_IND,				PARTYDISCONNECTING,				CSipParty::IceMakeAnswerIndWhileDisconnecting)
ONEVENT(CONTENT_WAIT_REINVITE_TOUT,				PARTYDISCONNECTING,	CSipParty::NullActionFunction)
ONEVENT(CONTENT_WAIT_REINVITE_TOUT,				ANYCASE,			CSipParty::OnWaitToStartReinviteForContentTout)

//LYNC2013_FEC_RED:
ONEVENT(UPDATE_NEW_FEC_RED_RATE,			PARTYCONNECTED,		CSipParty::UpdateVideoRateForFECorRED)
ONEVENT(FEC_RED_VIDEO_RATE_UPDATED,			ANYCASE,			CSipParty::VideoRateForFecOrRedUpdated)
ONEVENT(SIP_PARTY_SINGLE_FEC_RED_MSG,		ANYCASE, 			CSipParty::OnPartyReceivedAvMcuSingleFecOrRedMsg)

ONEVENT(UPDATE_MUTE_ICON,                    ANYCASE,    		CSipParty::OnPartyUpdateMuteIcon)

ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYSETUP,         CSipParty::OnTimerContentSpeakerIntraRequest)
ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYCHANGEMODE,    CSipParty::OnTimerContentSpeakerIntraRequest)
ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYCONNECTED,     CSipParty::OnTimerContentSpeakerIntraRequest)
ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYIDLE,          CSipParty::NullActionFunction)
ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYDISCONNECTING, CSipParty::NullActionFunction)

ONEVENT(CONTENT_INTRA_TOUT,  				ANYCASE, 				CSipParty::SendIntraToEP)

//CDR_MCCF:
ONEVENT(SIP_PARTY_STATISTIC_INFO,   IP_CM_PARTY_WAITING_FOR_STATISTIC_INFO_IND,  CSipParty::ContinueToCloseTIPcallReq)

ONEVENT(RELAY_ASK_ENDPOINT_FOR_INTRA, PARTYDISCONNECTING,		CSipParty::OnRelayAskEndpointForIntra)
ONEVENT(RELAY_ASK_ENDPOINT_FOR_INTRA, ANYCASE,		CSipParty::OnRelayAskEndpointForIntra)
ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA, ANYCASE, 		CSipParty::OnMrmpRtcpFirInd)
ONEVENT(SIP_PARTY_ACK_FOR__SCP_REQ,	ANYCASE,			CSipParty::OnAckForScpStreamsReq)
ONEVENT(SCP_NOTIFICATION_TO_MRMP_REQ,	ANYCASE,		CSipParty::OnScpNotificationToEpReq)
ONEVENT(SCP_IVR_STATE_NOTIFICATION_REQ,	ANYCASE,		CSipParty::OnScpIvrStateNotificationReqToEp)
ONEVENT(SIP_PARTY_SCP_NOTIFICATION_ACK_FROM_EP,	ANYCASE,CSipParty::OnSipPartyNotificationAckFromEP)
ONEVENT(SIP_PARTY_ACK_FOR_SCP_NOTIFICATION_IND,	ANYCASE,CSipParty::OnAckForScpNotificationInd)
ONEVENT(FIR_PEOPLE_TIMER		,ANYCASE,		CSipParty::OnFirPeopleTimer)
ONEVENT(FIR_TREATMENT_MAP_TIMER	,ANYCASE,		CSipParty::OnFirTreatmentMapTimer)
ONEVENT(SCP_PIPES_MAPPING_NOTIFICATION_TO_MRMP_REQ,				ANYCASE,	CSipParty::OnScpPipesMappingNotificationToEpReq)
//LyncCCS
ONEVENT(PARTY_CONF_PWD_MSG_IND   ,ANYCASE,		CSipParty::OnConfPwdInd)
ONEVENT(SIP_PARTY_AUTH_PWD_STATUS	 ,ANYCASE,		CSipParty::OnPartyPwdReq)
ONEVENT(SET_PARTY_AS_LEADER,          sPARTY_CONNECTING,     CParty::OnConfLeaderChangeStatusConnect)
ONEVENT(PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED	 ,PARTYCONNECTED,		CSipParty::OnPartyUpgradeToMixChannelsUpdated)

ONEVENT(SEND_MRMP_STREAM_IS_MUST_REQ ,ANYCASE,CSipParty::OnSendMrmpStreamIsMustReq)
ONEVENT(SEND_MRMP_STREAM_IS_MUST_ACK ,ANYCASE,CSipParty::OnMrmpStreamIsMustAck)

ONEVENT(VIDEO_IN_SYNCED, PARTYDISCONNECTING, CSipParty::NullActionFunction)
ONEVENT(VIDEO_IN_SYNCED, ANYCASE, CSipParty::OnVidBrdgVideoInSyncAnycase)

ONEVENT(SIP_PARTY_MULTI_VSR_REQ, PARTYCONNECTED, CSipParty::OnVideoBridgeMultiVSRReq)
ONEVENT(SIP_PARTY_MULTI_VSR_REQ, PARTYCHANGEMODE,CSipParty::OnVideoBridgeMultiVSRReq)
ONEVENT(SIP_PARTY_MULTI_VSR_REQ, ANYCASE, CSipParty::NullActionFunction)
ONEVENT(SIP_PARTY_SINGLE_VSR_MSG_IND,				ANYCASE, 	CSipParty::OnPartyReceivedAvMcuSingleVsrInd)
ONEVENT(ACTIVE_MEDIA_FOR_AVMCU_LYNC,				ANYCASE, 	CSipParty::OnConfActiveMediaForAVMCULync)
ONEVENT(MS_SLAVE_VIDREFRESH,						ANYCASE,				CSipParty::OnMSSlaveRefreshAnycase)
ONEVENT(MS_SLAVE_VIDEO_IN_SYNCED, 					PARTYDISCONNECTING, 	CSipParty::NullActionFunction)
ONEVENT(MS_SLAVE_VIDEO_IN_SYNCED, 					ANYCASE, 				CSipParty::OnMSSlaveVideoInSyncAnycase)

//eFeatureRssDialin
ONEVENT(SRS_RECORDING_CONTROL_IND, PARTYCONNECTED, CSipParty::OnConfRecordingControlConnected)
ONEVENT(SRS_RECORDING_CONTROL_IND, PARTYCHANGEMODE, CSipParty::OnConfRecordingControlConnected)
ONEVENT(SRS_RECORDING_CONTROL_IND, ANYCASE, CSipParty::OnConfRecordingControlAnycase)
ONEVENT(SRS_RECORDING_CONTROL_STATUS, ANYCASE, CSipParty::OnPartyRecordingControlStatusAnycase)
ONEVENT(SRS_LAYOUT_CONTROL_PARTY_LOCAL, ANYCASE, CSipParty::OnPartyLayoutControlAnycase)

/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
ONEVENT(VIDEORECOVERYBYE,						ANYCASE,				CSipParty::SendByeToPartyAnycase)
/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

ONEVENT(TOKENRECAPCOLLISIONEND, PARTYDISCONNECTING,	CSipParty::NullActionFunction)
ONEVENT(TOKENRECAPCOLLISIONEND, ANYCASE, 			CSipParty::OnTokenRecapCollisionEndAnycase)


PEND_MESSAGE_MAP(CSipParty, CIpParty);



#define SIP_MAX_INTRA_NUM	5
#define SIP_INTRA_INTERVAL	3

#define NumOfBfcpReestablishment 3

#define MASK_FOR_VSW_AVC_PIPEID 0x000FFFFF

/////////////////////////////////////////////////////////////////////////////
CSipParty::CSipParty(ESipPartyCntlType sipCntlType)
:CIpParty(new CSipComMode, new CSipComMode)
{
	m_interfaceType                             = SIP_INTERFACE_TYPE;
	if (sipCntlType == eSipPartyCntlWebRtc)
		m_pSipCntl = new CSipWebRtcCntl(this);
	else
	m_pSipCntl = new CSipCntl(this);
	m_eDialState                                = kNotInDialState;
	m_pSipTransaction                           = new CSipTransaction(this);
	m_eActiveTransactionType                    = kSipTransNone;
	m_bIsAdvancedVideoFeatures                  = NO;
	m_alternativeAddrStr                        = NULL;
	m_state                                     = PARTYIDLE;
	m_maskRequiredChangeMode                    = eChangeModeMask_None;
	m_pUpdateTargetMode                         = new CComModeH323;
	m_pUpgradeParams                            = NULL;
	m_lastConstraintRate                        = 0;
	m_bIsPreviewVideoIn                         = FALSE;
	m_bIsPreviewVideoOut                        = FALSE;
	m_minValForGlareTimer                       = 0;
	m_maxValForGlareTimer                       = 0;
	m_bIsGlareStatus                            = FALSE;
	m_bIsConfChangeMode                         = FALSE;
	m_bSendIntraOnEndOfTransaction              = FALSE;
	m_bIsCallConnected                          = FALSE;
	m_bIsRcvCheckCompleteAndNeedToStartIVR      = FALSE;
	m_pTargetModeMaxAllocation                  = NULL;
	m_pH239TokenMsgMngr                         = new CTokenMsgMngr;

	m_pH239TokenMsgMngr->EnableTokenMsgMngr();

	m_bBfcpConnected                            = FALSE;
	m_ProviderMessageCounter                    = 0;
	m_repeatIntra                               = 0;
	m_repeatIntraNumber                         = 0;
	m_repeatContentIntra                        = 0;
	m_repeatContentIntraNumber                  = SIP_MAX_INTRA_NUM;
	m_contentIntraInterval                      = SIP_INTRA_INTERVAL;
	m_lastActiveContentSenderID                 = 0;
	m_RTVLastVideoPreferenceWidth               = 0;
	m_RTVLastVideoPreferenceHeigh               = 0;
	m_bIsRtvPreferenceMsgSent                   = FALSE;
	m_bMsftRecevVsrInActiveTrans                = FALSE;
	m_bIsTipCall                                = FALSE;
	m_IsPartialConnectionForVSW                 = FALSE;
	m_tipPartyType                              = eTipNone;
	m_TipNumOfStreams                           = 0;
	m_bIsAudioAux                               = 0;
	m_bIsVideoAux                               = 0;
	m_SlaveAuxRsrcId                            = 0;
	m_SlaveLeftRsrcId                           = 0;
	m_SlaveRightRsrcId                          = 0;
	m_NumOfSlaves                               = 0;
	m_NumOfAckFromSlaves                        = 0;
	m_bHandlingTipNegotiationResult             = FALSE;
	m_bIsPolycomFromRTCP                        = FALSE;
	m_nSlavesReturnRecapAck                     = -1;
	m_TipNotSentIntraResponseCounter            = 0;
	m_bIsNeedToStartIVRAfterFallbackFromTip     = FALSE;
	m_bIsNeedTipReinviteAfterNegotiation        = 0;
	m_disconnectInitiator                       = eUnKnown;
	m_firPeopleCounter                          = 0;
	m_pFirPeopleSegnent                         = NULL;
	m_numOfBfcpReestablish                      = 0;
	m_bIsNeedToUpdateContentOnTheTransactionEnd = false;
	m_bTransactionSetContentOn                  = FALSE;

	for (int i = 0; i < MAX_NUM_RECV_STREAMS_FOR_FIR_FILTER; i++)
	{
		m_FirFilter[i].active      = FALSE;
		m_FirFilter[i].ssrc        = 0;
		m_FirFilter[i].lastFirTime = 0;
	}
	m_bIsFirFilterCreated = FALSE;

	m_lastTimeFirTreated.InitDefaults();

	m_isContentSuggested                   = FALSE; // Added by Efi
	m_isActiveContentForTip                = FALSE;
	m_isNeedToSendReInviteAfterTIPFallBack = FALSE;
	m_bTipLastAckReceived                  = FALSE;
	m_bTipEndSuccessSent                   = FALSE;
	m_bIsTipResumed                        = FALSE;
	m_bIsTipPutOnHold                      = FALSE;
	m_num_content_intra_filtered           = 0;
	m_nIntraCounter                        = 0;
	m_nIntraAfterTimer                     = 0;
	m_nIntraAfterTransaction               = 0;
	m_BfcpMsgCachedType                    = 0;
	m_BfcpMsgCachedTime                    = 0;
	m_numOfIframesSent                     = 0;
	m_eNonTipOnHold                        = eMediaOnHoldNon;
	m_changeModeState                      = eNotNeeded;
	m_PartyContentRate                     = 0;
	m_pLastTxVsr                           = NULL;
	m_isAllOutSlavesConnected              = FALSE;
	m_bIsIncomingFIR 					   = FALSE ;
	m_bIsDelayedOnce					   = false;
	VALIDATEMESSAGEMAP
}


/////////////////////////////////////////////////////////////////////////////
CSipParty::~CSipParty()
{
	POBJDELETE(m_pSipTransaction);
	POBJDELETE(m_pSipCntl);
	PDELETEA(m_alternativeAddrStr);
	POBJDELETE(m_pUpdateTargetMode);
	POBJDELETE(m_pUpgradeParams);
	POBJDELETE(m_pTargetModeMaxAllocation);
	POBJDELETE(m_pH239TokenMsgMngr);
	POBJDELETE(m_pFirPeopleSegnent);
	POBJDELETE(m_pLastTxVsr);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::Create(CSegment& appParam)
{
	CIpParty::Create(appParam);
	m_pTargetModeMaxAllocation = new CIpComMode(*m_pTargetMode);
	if (IsValidPObjectPtr(m_pSipTransaction))
	m_pSipTransaction->InitTransaction(this, m_pSipCntl, m_pCurrentMode, m_pTargetMode, &m_eDialState, m_partyConfName, m_voice, m_alternativeAddrStr,m_pTargetModeMaxAllocation);
	else
		DBGPASSERT(1);

	srand((unsigned)time(0)); // for glare random time.
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
	if (opCode == SIP_TRANSACTION_MSG)
	{
		OPCODE wrappedOpcode;
		*pMsg >> wrappedOpcode;
		CIpParty::HandleEvent(pMsg, msgLen, wrappedOpcode);
	}
	else
	{
		if (IsValidPObjectPtr(m_pSipTransaction))
		{
			m_pSipTransaction->HandleSipPartyEvent(pMsg, msgLen, opCode);
		}
		else
			DBGPASSERT(opCode);
	}
}

/////////////////////////////////////////////////////////////////////////////
BOOL  CSipParty::DispatchEvent(OPCODE event,CSegment* pParam)
{
	BOOL bActiveTrans = IsActiveTransaction();
	if (bActiveTrans && m_pSipTransaction->DispatchEvent(event, pParam))
	{
		return TRUE;
	}

	if (CIpParty::DispatchEvent(event, pParam))
		return TRUE;

	if (bActiveTrans)
		PTRACE2INT(eLevelInfoNormal, "CSipParty::DispatchEvent - transaction state: ", m_pSipTransaction->GetState());

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnMcuMngrPartyMonitoringReqAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnMcuMngrPartyMonitoringReqAnycase - ",PARTYNAME);
	OnMcuMngrPartyMonitoringReq(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnMcuMngrPartyMonitoringReq(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipParty::OnMcuMngrPartyMonitoringReq");

	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	APIU32 RefreshPeriodForPMinMiliSeconds = 0;
	std::string key = "PARTY_MONITORING_REFRESH_PERIOD";
	pSysConfig->GetDWORDDataByKey(key, RefreshPeriodForPMinMiliSeconds);
	RefreshPeriodForPMinMiliSeconds = RefreshPeriodForPMinMiliSeconds * 1000; // Convert to mili seconds
	if (RefreshPeriodForPMinMiliSeconds < 500)
		RefreshPeriodForPMinMiliSeconds = 500; // Minimum monitoring interval is 500 mili seconds.
	TICKS curTicks;
	CTaskApp *pTaskApp;

	*pParam >> (void *&)pTaskApp;

	//PartyMonitoring requests will be sent to a party
	//in a minimum interval at least 0.5 second
	curTicks = SystemGetTickCount();
	TICKS diff;
	diff = curTicks - m_lastPmTicks;

	//GetIntegerPartForTrace return value is 1/100 of a second
	if (diff.GetIntegerPartForTrace() * 10 >= RefreshPeriodForPMinMiliSeconds)
	{
		BYTE isMonitorReqSent = 0;

		bool bIsTipOrMrc = false;
		if ((GetIsTipCall()) || (m_pSipCntl->GetIsMrcCall()))
		{
			bIsTipOrMrc = true;
		}

		PTRACE(eLevelInfoNormal, "CSipParty::OnMcuMngrPartyMonitoringReq - SIP control will send monitoring req");

		int partyid_to_send = bIsTipOrMrc ? GetPartyRsrcID() : 0;
		CCommConf* pCommConf 	= ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
		 CConfParty* pConfParty  =NULL;
		if (pCommConf)
		{
		  pConfParty 	= pCommConf->GetCurrentParty(GetMonitorPartyId());
		}

		if( m_pSipCntl->isMs2013Active() != eMsft2013AvMCU || ( pConfParty && pConfParty->GetAvMcuLinkType() != eAvMcuLinkMain) )
			isMonitorReqSent = m_pSipCntl->PartyMonitoringReq(partyid_to_send);
		else
		{
			isMonitorReqSent = m_pSipCntl->PartyMonitoringForAvMcuReq(partyid_to_send);
			TRACEINTOFUNC << "AV-MCU main forward to slaves controller "<<"partyid_to_send: " << partyid_to_send;

		}

		TRACEINTOFUNC << "partyid_to_send: " << partyid_to_send;

		if (isMonitorReqSent)
		{
			PTRACE(eLevelInfoNormal, "CSipParty::OnMcuMngrPartyMonitoringReq - No request was sent - Party is not connected yet");
		}

		m_lastPmTicks = curTicks;
	}

	else
	{
		CMedString msg;
		msg << "Do not send PartyMonitor - less then = " << RefreshPeriodForPMinMiliSeconds << " miliseconds\n";
		PTRACE2(eLevelInfoNormal, "CSipParty::OnMcuMngrPartyMonitoringReq: ", msg.GetString());
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::TellConfOnDisconnecting(int reason,const char* alternativeAddrStr,DWORD MipErrorNumber)
{
	TRACEINTO << m_partyConfName << ", DisconnectCause:" << reason;

	m_disconnectCause = reason;
	m_state = PARTYDISCONNECTING;
	EndTransactionByPartyIfNeeded();
	LogicalChannelDisconnect(SDP);

	// in case of conf  is responding, starting a disconnecting timer
	// BRIDGES_DISCONNECT_TIME max time is BRIDGES_DISCONNECT_TIME + 15. An extra 10 seconds is added for this timer.
	StartTimer(CONFDISCONNECTOUT, ( (BRIDGES_DISCONNECT_TIME+25) * SECOND) );

	m_pConfApi->PartyDisConnect(reason, this, alternativeAddrStr,MipErrorNumber);

	if ((reason == SIP_REMOTE_CLOSED_CALL) || (reason == SIP_REMOTE_CANCEL_CALL))
		m_pConfApi->UpdateDB(this, DISCAUSE, PARTY_HANG_UP, 1);
	else if((SIP_REMOTE_NO_ANSWER == reason)&&(FALSE == m_pSipCntl->GetRecevRingback()))
	{
		// When MCMS timeout, but we didn't receive 18X, set cause as TIMER_POPPED_OUT
		m_pConfApi->UpdateDB(this, DISCAUSE, SIP_TIMER_POPPED_OUT, 1);
	}
	else if((SIP_REQUEST_TIMEOUT == reason)&&(m_pSipCntl->GetRecevRingback()))
	{
		//When CS time out or Remote sends back 408, only if we have received 18X, set cause as NO_ANSWER
		m_pConfApi->UpdateDB(this, DISCAUSE, SIP_REMOTE_NO_ANSWER, 1);
	}
	else
		m_pConfApi->UpdateDB(this, DISCAUSE, reason, 1);

	if(m_tipPartyType == eTipMasterCenter && m_pTargetMode->IsTipNegotiated())
	{
		PTRACE(eLevelInfoNormal,"CSipParty::TellConfOnDisconnecting:TIP master ");
	}
}
///////////////////////////////////////////////////////////////////////////
void CSipParty::SendTipCallEndMsgAndDisconnectToSlaves()
{
	PTRACE(eLevelInfoNormal,"CSipParty::SendTipCallEndMsgAndDisconnectToSlaves:TIP master ");
	m_pConfApi->SendDisconnectMessageFromMasterPartyControlToAllToSlaves(this);
	m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_CLOSE_TIP_CALL_REQ);
}

/////////////////////////////////////////////////////////////////////////////
//CDR_MCCF:
void CSipParty::ContinueToCloseTIPcallReq()
{
   m_state = m_oldstate;
   if  (m_tipPartyType == eTipMasterCenter && m_pTargetMode->IsTipNegotiated())
   {
       PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipParty::ContinueToCloseTIPcallReq");
       m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_CLOSE_TIP_CALL_REQ);
	}
	else
		PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipParty::ContinueToCloseTIPcallReq - not sending IP_MSG_CLOSE_TIP_CALL_REQ!");

}

////////////////////////////////////////////////////////////////////////////////////////
void CSipParty::DestroyPartyTask()
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	// remove disconnecting timer
	if (IsValidTimer(PARTYDISCONNECTTOUT))
		DeleteTimer(PARTYDISCONNECTTOUT);

	if (IsValidTimer(DISCONNECT_SLAVE_PARTY_TOUT))  // TIP timer
		DeleteTimer(DISCONNECT_SLAVE_PARTY_TOUT);

	m_pSipCntl->Suspend();                          // Ignore all events
	m_pConfApi->PartyEndDisConnect(GetPartyId(), m_disconnectCause);
	PartySelfKill();
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfPartyReceiveVidBridgeConnectedAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnConfPartyReceiveVidBridgeConnectedAnycase - second video bridge connection indication (ignore)");
	//BYTE bIsEndAddParty = YES;
	if(GetIsTipCall() && (m_tipPartyType == eTipSlaveLeft || m_tipPartyType == eTipSlaveRigth  ))
	{
	    PTRACE(eLevelInfoNormal,"CSipParty::OnConfPartyReceiveVidBridgeConnectedAnycase - slave just answer ack");
	    m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pTargetMode, TRUE);
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfReadyToCloseCall(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnConfReadyToCloseCall - ", m_partyConfName);

	// remove disconnecting timer
	if (IsValidTimer(CONFDISCONNECTOUT))
		DeleteTimer(CONFDISCONNECTOUT);

	BYTE bIsCloseInitiator = NO;
	if((m_disconnectCause == SIP_REMOTE_STOP_RESPONDING) ||
	   (m_disconnectCause == SIP_CARD_REJECTED_CHANNELS) ||
	   (m_disconnectCause == SIP_TIMER_POPPED_OUT))
		bIsCloseInitiator = YES;

	if (m_eDialState == kNotInDialState && m_isPreSignalingFlowProb == 0)
		m_pSipCntl->CloseCall(YES);// I changed this line since any disconnection arrive from the party control when the dial state is kNotInDialState
									// is because of internal MCMS decision. I can add a disconnection cause to the if sentence (sip_caps_dont_match) but
									// I think this way its better (is it smart?)
	else
		CleanUp();
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfDisconnectChannels(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnConfDisconnectChannels - ", m_partyConfName);
	m_pTargetMode->DeSerialize(NATIVE,*pParam);
	DisconnectChannels();
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnRefreshAnycase(WORD ignore_filtering, DWORD remoteSSRC, DWORD priorityID, DWORD msslaveIndex, DWORD isSlaveRTV)
{
	TRACEINTO << PARTYNAME << ", IgnoreFiltering:" << ignore_filtering << ", RemoteSSRC:" << remoteSSRC << ", PriorityID:" << priorityID << ", MsSlaveIndex:" << msslaveIndex << ", IsSlaveRTV:" << isSlaveRTV;

	if (ignore_filtering)
		m_pSipCntl->FastUpdateReq(kRolePeople, eTipVideoPosCenter, remoteSSRC, priorityID, msslaveIndex, isSlaveRTV);
	else
		IntraRequestFiltering(3, FALSE, remoteSSRC, priorityID, msslaveIndex);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnInSyncAnycase(DWORD msslaveIndex)
{
	CapEnum algorithm = (CapEnum)(m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive));

	TRACEINTO << PARTYNAME << ", Algorithm:" << algorithm << ", MsSlaveIndex:" << msslaveIndex;

	if (eMsSvcCapCode != algorithm)
		return;

	//VIDEO_IN_SYNCED -> turn off K bit of sender ssrc
	DWORD partyIndex = 1 + msslaveIndex; //Shmulik: TBD when AVMCU 1-5
	m_pSipCntl->videoInSynched(partyIndex);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnMSSlaveRefreshAnycase(CSegment* pParam)
{
	DWORD temp_mainPartyId = 0;
	DWORD temp_opcod1 = 0;
	DWORD temp_opcode2 = 0;
	DWORD temp_slavePartyId = 0;
	DWORD temp_opcode3 = 0;
	DWORD temp_opcode4 = 0;
	WORD  param_ignore_filtering = 0;
	DWORD param_remoteSSRC = 0;
	DWORD param_priorityID = 0;
	DWORD param_isRtv = FALSE;

	DWORD param_msSlaveIndex = 0;

	if (pParam != NULL)
	{
		*pParam >>  temp_mainPartyId;
		*pParam >>  temp_opcod1;
		*pParam >>  temp_opcode2;
		*pParam >>  temp_slavePartyId;
		*pParam >>  temp_opcode3;
		*pParam >>  temp_opcode4;
		*pParam >>  param_msSlaveIndex;
		*pParam >>  param_ignore_filtering;
		*pParam >>  param_remoteSSRC;
		*pParam >>  param_priorityID;
		*pParam >>  param_isRtv;
	}
	std::ostringstream msg;
	msg
		<< "\n  temp_mainPartyId       :" << temp_mainPartyId
		<< "\n  temp_opcod1            :" << temp_opcod1
		<< "\n  temp_opcode2           :" << temp_opcode2
		<< "\n  temp_slavePartyId      :" << temp_slavePartyId
		<< "\n  temp_opcode3           :" << temp_opcode3
		<< "\n  temp_opcode4           :" << temp_opcode4
		<< "\n  param_msSlaveIndex     :" << param_msSlaveIndex
		<< "\n  param_ignore_filtering :" << param_ignore_filtering
		<< "\n  param_remoteSSRC       :" << param_remoteSSRC
		<< "\n  param_priorityID       :" << param_priorityID
		<< "\n  param_isRtv            :" << param_isRtv;

	TRACEINTO << GetFullName() << msg.str().c_str();

	OnRefreshAnycase(param_ignore_filtering, param_remoteSSRC, param_priorityID,param_msSlaveIndex, param_isRtv);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnMSSlaveVideoInSyncAnycase(CSegment* pParam)
{
	DWORD temp_mainPartyId = 0;
	DWORD temp_opcod1 = 0;
	DWORD temp_opcode2 = 0;
	DWORD temp_slavePartyId = 0;
	DWORD temp_opcode3 = 0;
	DWORD temp_opcode4 = 0;

	DWORD param_msSlaveIndex = 0;

	if(pParam != NULL)
	{
	  *pParam >>  temp_mainPartyId;
	  *pParam >>  temp_opcod1;
	  *pParam >>  temp_opcode2;
	  *pParam >>  temp_slavePartyId;
	  *pParam >>  temp_opcode3;
	  *pParam >>  temp_opcode4;
	  *pParam >>  param_msSlaveIndex;
	}
	std::ostringstream msg;
	msg
		<< "\n  temp_mainPartyId   :" << temp_mainPartyId
		<< "\n  temp_opcod1        :" << temp_opcod1
		<< "\n  temp_opcode2       :" << temp_opcode2
		<< "\n  temp_slavePartyId  :" << temp_slavePartyId
		<< "\n  temp_opcode3       :" << temp_opcode3
		<< "\n  temp_opcode4       :" << temp_opcode4
		<< "\n  param_msSlaveIndex :" << param_msSlaveIndex;

	TRACEINTO << GetFullName() << msg.str().c_str();
	OnInSyncAnycase(param_msSlaveIndex);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnVidBrdgRefreshAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnVidBrdgRefreshAnycase");

	WORD ignore_filtering = FALSE;
	DWORD remoteSSRC = 0;
	DWORD priorityID = 0;

	if(pParam != NULL)
	{
	  *pParam >> ignore_filtering >> remoteSSRC >>  priorityID;
	}

	OnRefreshAnycase(ignore_filtering, remoteSSRC, priorityID);

}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnVidBrdgVideoInSyncAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnVidBrdgVideoInSyncAnycase - ", GetFullName());

	OnInSyncAnycase();
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyCallClosed(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyCallClosed - ", m_partyConfName);
	// update communication mode
	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
	m_state = IP_DISCONNECTED;
	LogicalChannelDisconnect(SIGNALING);
	UpdateDbOnChannelsDisconnected();
	StopAllPreviews();
	DestroyPartyTask();
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyRemoteCloseCall(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyRemoteCloseCall - ", m_partyConfName);
	DWORD reason = 0xFFFFFFFF;
	*pParam >> reason;
	// in case of cross message between re-invite to bye ind. re-invite was droped by the card
	switch (m_eDialState)
	{
	case kReInviteSent:
		m_eDialState = kNotInDialState;
		break;
	case kNotInDialState:
		m_eDialState = kByeArrived;
		break;
	default:
		if (reason == SIP_REMOTE_CLOSED_CALL)
			m_eDialState = kTerminateByRemote;
		else if(reason == SIP_REMOTE_CANCEL_CALL)
			m_eDialState = kCancelArrived;
		else
			PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyRemoteCloseCall - Reason:",reason);
		break;
	}
	TellConfOnDisconnecting(reason);
}


	// Never called!
/////////////////////////////////////////////////////////////////////////////
// info about channels
void CSipParty::OnSipLogicalChannelConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnSipLogicalChannelConnect - ", PARTYNAME);
	CSegment* pSeg = new CSegment;

	CPrtMontrBaseParams *pPrtMonitrParams = NULL;

	DWORD  vendorType;
	DWORD  channelType;

	*pParam >> vendorType >> channelType;

	EIpChannelType channelTypeE = (EIpChannelType)channelType;

	pPrtMonitrParams = CPrtMontrBaseParams::AllocNewClass(channelTypeE);
	if (pPrtMonitrParams)
		pPrtMonitrParams->DeSerialize(NATIVE,*pParam);

	*pSeg << vendorType << channelType;

	if (pPrtMonitrParams)
		pPrtMonitrParams->Serialize(NATIVE,*pSeg);

	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELCONNECT,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);
	POBJDELETE(pPrtMonitrParams);
}

/////////////////////////////////////////////////////////////////////////////BRIDGE-15265
BOOL CSipParty::IsPayloadTypeIsForFecc(WORD payloadType)
{
	PASSERTMSG_AND_RETURN_VALUE(!m_pSipCntl, "!m_pSipCntl", FALSE);

	BOOL 		bIsCurrPayloadTypeIsForFecc = FALSE;
	CSipChannel *pDataChannel 				= m_pSipCntl->GetChannel(cmCapData, cmCapReceive);

	if(pDataChannel)
	{
		DWORD nFeccPayloadType = pDataChannel->GetPayloadType();
		if(payloadType == nFeccPayloadType)
		{
			TRACEINTO << "The payload type " << payloadType << " is the FECC payload type!";
			bIsCurrPayloadTypeIsForFecc = TRUE;
		}
	}

	return bIsCurrPayloadTypeIsForFecc;
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyDifferentPayloadAnycase(CSegment* pParam)
{
	WORD 			tempChannelType;
	cmCapDataType 	channelType;

	WORD 			tempChannelDirection;
	cmCapDirection	channelDirection;
	WORD 			payload;

	WORD 			tempRole;
	ERoleLabel		roleLabel;

	CapEnum 		reVal 			= eUnknownAlgorithemCapCode;
	BYTE 			bUpdateBridges 	= FALSE;

	*pParam
		>> tempChannelType
		>> tempChannelDirection
		>> payload
		>> tempRole;

	channelType			= (cmCapDataType)tempChannelType;
	channelDirection	= (cmCapDirection)tempChannelDirection;
	roleLabel			= (ERoleLabel) tempRole;

	// when this indication arrive the following steps are needed:
	// 1. check it come for incoming channel.
	// 2. check that the payload is legal (one of the payload we declare on in the SDP).
	// 3. I decided to ignore checking with remote SDP since it can in a transaction when the remote changing its SDP.
	// 4. check that the channel is not disconnected/ing.
	// 5. save the payload - no need to save info before since it will take additional transaction in order to open channels.
	// 6. make best mode with the new payload
	// 7. update RTP and bridge (we can ignore mute).

	if(channelDirection != cmCapReceive)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyDifferentPayloadAnycase. not receive direction - ",channelDirection);
		return;
	}

	EConnectionState ChannelState = m_pSipCntl->GetChannelConnectionState(channelType, channelDirection, roleLabel);

	if ((ChannelState == kDisconnecting) || (ChannelState == kDisconnected) || (ChannelState == kUnknown))
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyDifferentPayloadAnycase - Channel state doesn't enable update. state - ", ChannelState);
		return;
	}

	if (roleLabel & kRoleContentOrPresentation)
	{
		//BRIDGE-13949 - allowing update - we support just one content algorithm in this case
		if(IsPreferTip() && !IsPayloadTypeIsForFecc(payload)) //BRIDGE-15265 added IsPayloadtypeIsForFecc check
		{
			TRACEINTO << "Arrived new Payload number - " << payload << ". "  << " (content channel on prefer TIP conf)" ;

			EIpChannelType chanArr;
			chanArr = ::CalcChannelType(channelType, (channelDirection == cmCapTransmit), roleLabel);
			m_pSipCntl->SipUpdateChannelReq((CSipComMode*)m_pTargetMode, chanArr, kChannelParams, YES, NO, payload);
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnPartyDifferentPayloadAnycase - Diff payload type on content isn't supported");
		}

		return; //End of the road for content here.
	}

	if (IsActiveTransaction() || GetIsTipNegotiationActive())
	{
		CSipChannel *pChannel = m_pSipCntl->GetChannel(channelType, channelDirection);
		if (pChannel)
		{
		    PTRACE(eLevelInfoNormal,"CSipParty::OnPartyDifferentPayloadAnycase - not handled during active transaction");
		    pChannel->SetDiffPayloadState(kDiffPayload_NeedToSendUpdate);
		}
		else
		    PTRACE(eLevelError,"CSipParty::OnPartyDifferentPayloadAnycase - pChannel is NULL - can not SetDiffPayloadState!");
		return;
	}

	CMedString cLog;
	cLog << "Arrived new Payload number - " << payload << ".";

	const CSipCaps*	pLocalCaps  = m_pSipCntl->GetLocalCaps();
	reVal = pLocalCaps->FindAlgAccordingToPayload(channelType, payload, roleLabel);

	/* In case of SirenLPR Update the localCaps with the minimum cap between the local and last Remote caps */
	if ((reVal >= eSirenLPR_32kCapCode) && (reVal <= eSirenLPRStereo_128kCapCode))
	{
		m_pSipCntl->IntersectSirenLPRLocalAndRemoteCaps(channelType, payload, roleLabel);
		reVal = pLocalCaps->FindAlgAccordingToPayload(channelType, payload, roleLabel);
	}


	if(reVal != eUnknownAlgorithemCapCode && reVal != eLPRCapCode && reVal != eFECCapCode && reVal != eREDCapCode) //LYNC2013_FEC_RED
	{
		CCapSetInfo capInfo	= reVal;// for monitoring
		cLog << " Payload is exist in local caps, name - " << capInfo.GetH323CapName() << ".";

		APIU8 localprofile 				= 0,
			  localPacketizationMode 	= 0;
		APIS32 localFs					= 0;
		APIU8 targetmodeprofile 		= 0,
			  targetPacketizationMode 	= 0;
		APIS32 targetFs					= 0;

		if(reVal == eH264CapCode)
		{
			localprofile = pLocalCaps->FindH264ProfileFromPayload(channelType, payload);
			localPacketizationMode = pLocalCaps->FindH264PacketizationModeFromPayload(channelType, payload);
			localFs = pLocalCaps->FindMaxFsFromPayload(channelType, payload);
			PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyDifferentPayloadAnycase -h264-profile is ",localprofile);
		}

		if(m_pTargetMode->GetMediaType(channelType, channelDirection, roleLabel) == eH264CapCode)
		{
			targetmodeprofile = m_pTargetMode->GetH264Profile(cmCapReceive);
			targetPacketizationMode = m_pTargetMode->GetH264PacketizationMode(cmCapReceive);
			APIU16 profile = 0;
			long mbps = 0;
			APIU8 level = 0;
			long sar = 0;
			long staticMB = 0;
			long dpb = 0;
			cmCapDirection direction = cmCapReceive;
			m_pTargetMode->GetFSandMBPS(direction, profile, level, targetFs, mbps, sar, staticMB, dpb);
		}

		if (( m_pTargetMode->GetMediaType(channelType, channelDirection, roleLabel) != reVal ) ||
			(localprofile != targetmodeprofile ) ||
			(localFs != targetFs ) ||
			(localPacketizationMode != targetPacketizationMode))
		{
			cLog << " New payload is different from the current payload. ";

			CBaseCap* pMedia = pLocalCaps->GetCapSetAccordingToPayload(reVal,payload); AUTO_DELETE(pMedia);
			if(pMedia)
			{
				//check for change in rate in case of change in audio codec -> update video rate
				if (cmCapAudio == channelType && kCp == m_pTargetMode->GetConfType() &&
				        !m_pSipCntl->GetIsMrcCall() && m_pTargetMode->GetConfMediaType()!=eMixAvcSvcVsw)
				{
					CMedString cLog2;
					CSipChannel *pAudChannel = m_pSipCntl->GetChannel(cmCapAudio, cmCapReceive);
					DWORD oldAudRate = 0;
					if (pAudChannel)
					    oldAudRate = pAudChannel->GetCurrentRate()*10;
					else
					{
					    PTRACE(eLevelError,"CSipParty::OnPartyDifferentPayloadAnycase - pAudChannel is NULL - set oldAudRate with 0");
					    return;
					}
					DWORD newAudRate = pMedia->GetBitRate()/100;
					cLog2 << "oldAudRate=" << oldAudRate << ",newAudRate="<<newAudRate;
					if (oldAudRate != newAudRate)
					{
						CSipChannel *pVidChannel = m_pSipCntl->GetChannel(cmCapVideo, cmCapReceive);
						if(pVidChannel)
						{
							DWORD oldVidRate = pVidChannel->GetCurrentRate();
							DWORD newVidRate = 0;
							if (oldAudRate > newAudRate)
							{
								newVidRate = oldVidRate + (oldAudRate - newAudRate);
							}
							else
							{
								newVidRate = oldVidRate - (newAudRate - oldAudRate);
							}
							cLog2 <<",oldVidRate="<<oldVidRate<<",newVidRate="<<newVidRate;
							PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyDifferentPayloadAnycase - ", cLog2.GetString());

							//update video rate + send flow control
							m_pTargetMode->SetVideoBitRate(newVidRate, cmCapReceive);
							m_pSipCntl->SendFlowControlReq(mainType, cmCapReceive, newVidRate);
						}
					}
				}
				cLog << " Change Target mode.";
				m_pTargetMode->SetMediaMode(pMedia, channelType, channelDirection);
				m_pCurrentMode->SetMediaMode(pMedia, channelType, channelDirection);
				bUpdateBridges = TRUE;
				POBJDELETE(pMedia);
			}
			else
			{
				cLog << " pMedia is Null.";
			}
		}
	}

	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyDifferentPayloadAnycase - ", cLog.GetString());

	if (bUpdateBridges)
	{
		// Currently, there is no needed for resources reallocation as a result of the update. If Dynamic Port Allocation will be implemented we should rethinking about it.
		m_pConfApi->IpUpdateBridges(GetPartyId(), m_pTargetMode, channelType, channelDirection);
		UpdateDbOnChannelsConnected(); // update monitoring.
	}

	// We reply to RTP in any case, even if we don't change anything and the payload remains as it was before,
	// otherwise - the RTP will not send any other payload indication for this channel any more.
	EIpChannelType chanArr;
	chanArr = ::CalcChannelType(channelType, (channelDirection == cmCapTransmit), roleLabel);
	BOOL bKeepCurrentPayload = bUpdateBridges ? NO : YES;  // If payload type wasn't changed, we need to keep payload number that we already use and don't take the one from local caps because it may be not symmetric to remote payload number as we found before.
	m_pSipCntl->SipUpdateChannelReq((CSipComMode*)m_pTargetMode, chanArr, kChannelParams, YES, bKeepCurrentPayload, payload);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSipPartyMonitoring(CSegment* pParam)
{
	CSegment* pSeg = new CSegment;

	BYTE channelType;
	BYTE intraSyncFlag;
	BYTE videoBCHSyncFlag;
	BYTE protocolSyncFlag;
	WORD bchOutOfSyncCount;
	WORD protocolOutOfSyncCount;

	*pParam
		>> channelType
		>> intraSyncFlag
		>> videoBCHSyncFlag
		>> bchOutOfSyncCount
		>> protocolSyncFlag
		>> protocolOutOfSyncCount;

	*pSeg
		<< (BYTE)channelType
		<< (BYTE)intraSyncFlag
		<< (BYTE)videoBCHSyncFlag
		<< (WORD)bchOutOfSyncCount
		<< (BYTE)protocolSyncFlag
		<< (WORD)protocolOutOfSyncCount
		<< (BYTE)1; //bShowMonitoring

  m_pConfApi->UpdateDB(this,IPPARTYMONITORING,(DWORD) 0,1,pSeg);
  POBJDELETE(pSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyChannelsConnected(CSegment* pParam)
{
	EConfType eConfType = m_pTargetMode->GetConfType();
	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pCurrentMode->SetConfType(eConfType);

	UpdateDbOnChannelsConnected();

	if (GetIsTipNegotiationActive() && m_pTargetMode->IsTipNegotiated())
		UpdateBridgesForTipNegotiation();
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyChannelsDisConnected(CSegment* pParam)
{
	/*Port fix of BRIDGE-3638 from CCS 8.1.1 to V100 */
	PTRACE(eLevelInfoNormal,"IS_PREFER_TIP_MODE: CSipParty::OnPartyChannelsDisConnected ---");
	EConfType eConfType = m_pTargetMode->GetConfType();
	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pCurrentMode->SetConfType(eConfType);

    if (GetIsTipNegotiationActive())
    {
        PTRACE(eLevelInfoNormal,"IS_PREFER_TIP_MODE: CSipParty::OnPartyChannelsDisConnected - after tip negotiation");

	    CSipCall *pCall = m_pSipCntl->GetCallObj();
	    if (pCall)
	    {
	    	    CSipChannel* pContentChannelIn = pCall->GetChannel(VIDEO_CONT_IN);
	    	    CSipChannel* pContentChannelOut = pCall->GetChannel(VIDEO_CONT_OUT);
		        if (!pContentChannelIn && !pContentChannelOut)
		        {
		            PTRACE(eLevelInfoNormal,"CSipParty::OnPartyChannelsDisConnected - open content channels");
					CSipComMode* pNewMediaMode = new CSipComMode;
					m_pSipCntl->SetRemoteCapsTipAuxFPS(eTipAux5FPS);

					const CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
					if (pCommConf && pCommConf->GetIsPreferTIP())
					{
						m_pTargetMode->SetTIPContent(0, cmCapReceiveAndTransmit,FALSE);
						m_pTargetModeMaxAllocation->SetTIPContent(0, cmCapReceiveAndTransmit,FALSE);
						pNewMediaMode->SetTIPContent(0, cmCapReceiveAndTransmit,FALSE);
					}
					else //eTipCompatibleVideoAndContent
					{
						m_pTargetMode->SetTIPContent(0, cmCapReceiveAndTransmit);
						m_pTargetModeMaxAllocation->SetTIPContent(0, cmCapReceiveAndTransmit);
						pNewMediaMode->SetTIPContent(0, cmCapReceiveAndTransmit);
					}

					SetContentEncryptionForTipVideoAux(pNewMediaMode);
					m_pSipCntl->SipOpenChannelsReq(pNewMediaMode, cmCapReceiveAndTransmit, FALSE, eTipMasterCenter);
					POBJDELETE(pNewMediaMode);
		        }
		        else
		        {
		            PTRACE(eLevelInfoNormal,"CSipParty::OnPartyChannelsDisConnected - update bridges");
		        	UpdateBridgesForTipNegotiation();
		        }
	    }
    }
    else
        PTRACE(eLevelInfoNormal,"IS_PREFER_TIP_MODE: CSipParty::OnPartyChannelsDisConnected");
}
//////////////////////////////////////////////////////
void CSipParty::OnScpRequestFromEp(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"_scp_flow_ CSipParty::OnScpRequestFromEp");

	CMrmpScpStreamsRequestStructWrap scpStreamReq;

	MrmpScpStreamsRequestStruct *pStruct = (MrmpScpStreamsRequestStruct *)pParam->GetPtr(1);
	scpStreamReq = *pStruct;

	m_pConfApi->ScpRequestFromEp(GetPartyId(), scpStreamReq);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnScpNotificationIndFromEp(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnScpNotificationIndFromEp");

	CScpNotificationWrapper scpStreamNotificationInd;

	MrmpScpStreamsNotificationStruct *pStruct = (MrmpScpStreamsNotificationStruct *)pParam->GetPtr(1);
	scpStreamNotificationInd = *pStruct;
	m_pConfApi->ScpNotificationIndFromEp(GetPartyId(), scpStreamNotificationInd);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnUpdateScmStreams(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal,"CSipParty::OnUpdateScmStreams");

    m_pTargetMode->DeSerialize(NATIVE, *pParam);
    //FSN-613: Dynamic Content for SVC/Mix Conf, direct copy leads to content declaration in re-invite fails.
   // *m_pCurrentMode = *m_pTargetMode;
   std::list <StreamDesc> videoStreams = m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);
   m_pCurrentMode->SetStreamsListForMediaMode(videoStreams, cmCapVideo, cmCapTransmit, kRolePeople);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateDbOnChannelsConnected()
{
	PTRACE2(eLevelInfoNormal,"CSipParty::UpdateDbOnChannelsConnected - ", m_partyConfName);

	mcTransportAddress localIp;
	memset(&localIp,0,sizeof(mcTransportAddress));
	mcTransportAddress remoteIp;
	memset(&remoteIp,0,sizeof(mcTransportAddress));
	m_pSipCntl->GetLocalMediaIpAsTrAddr(localIp);

	//ICE
	chosenCandidatesSt ChosenCandidates;
	memset(&ChosenCandidates,0,sizeof(chosenCandidatesSt));
	mcTransportAddress IceLocalIp ;
	mcTransportAddress IceRemoteIp ;
	memset(&IceRemoteIp,0,sizeof(mcTransportAddress));
	memset(&IceLocalIp,0,sizeof(mcTransportAddress));
    BYTE IsIce = 0;

	BYTE bIsTransmit;
	EIpChannelType eChanType;
	cmCapDataType mediaType;
	ERoleLabel eRole;

	for (int i=0 ; i<MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		m_pSipCntl->GetRemoteMediaIpAsTrAddr(mediaType,eRole,remoteIp);

		for (int j=0; j<2; j++)
		{
			bIsTransmit = (globalDirectionArr[j] == cmCapTransmit);
			eChanType   = ::CalcChannelType(mediaType,bIsTransmit,eRole);

			if (m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole))
			{
				DWORD actualRate	= 0xFFFFFFFF;
				localIp.port			= m_pSipCntl->GetPort(mediaType, cmCapReceive,eRole);
				remoteIp.port			= m_pSipCntl->GetPort(mediaType, cmCapTransmit,eRole);

				m_pSipCntl->OverwriteMonitoring(eChanType, remoteIp, localIp, IsIce, IceRemoteIp, IceLocalIp);

				//if we receive Check complete in ICE call we need to
				// get the chosen candidates of the ICE
				if (m_bIsRcvCheckCompleteAndNeedToStartIVR && mediaType != cmCapBfcp)
				{
					PTRACE(eLevelInfoNormal,"CSipParty::UpdateDbOnChannelsConnected m_bIsRcvCheckCompleteAndNeedToStartIVR");

					CMedString str;

					IsIce = 1;

					memset(&remoteIp,0,sizeof(mcTransportAddress));
					m_pSipCntl->GetOriginalRmtIpAddress(mediaType,remoteIp,eRole);

					char* IceRmtIPStr = m_pSipCntl->GetMediaChosenRemoteIpAddress(mediaType);
					char* IceLocalIPStr = m_pSipCntl->GetMediaChosenLocalIpAddress(mediaType);

					str << " \n  local Ice addr - " << IceLocalIPStr << " remote Ice addr - " << IceRmtIPStr ;

					if((IceRmtIPStr) && ::IsValidIpV4Address(IceRmtIPStr) && (IceLocalIPStr) && ::IsValidIpV4Address(IceLocalIPStr))
					{
						::stringToIpV4 (&IceRemoteIp, IceRmtIPStr);
						::stringToIpV4 (&IceLocalIp, IceLocalIPStr);
					}

					EIceConnectionType ChosenConnectionType = m_pSipCntl->GetMediaChosenRemoteType(mediaType);

					PTRACE2INT(eLevelInfoNormal,"CSipParty::UpdateDbOnChannelsConnected ChosenConnectionType",ChosenConnectionType);


					if(!(isApiTaNull(&IceRemoteIp) || isApiTaNull(&IceLocalIp)))
					{
						IceRemoteIp.port = m_pSipCntl->GetMediaChosenRemotePort(mediaType);
						IceLocalIp.port =  m_pSipCntl->GetMediaChosenLocalPort(mediaType);
					}

					 str << " \n Connection type - " << ChosenConnectionType
						 << " local Ice addr - " << IceLocalIPStr << ":" << IceLocalIp.port
						 << " remote Ice addr - " << IceRmtIPStr << ":" <<IceRemoteIp.port;

					 PTRACE (eLevelInfoNormal, str.GetString());

					SetPartyMonitorBaseParamsAndConnectChannel(eChanType, actualRate, &remoteIp,&localIp,(DWORD)eUnknownAlgorithemCapCode,0,0,IsIce,&IceRemoteIp,&IceLocalIp,ChosenConnectionType);

				}
				else
				{
					DWORD channelType = eChanType;

					if (mediaType == cmCapBfcp)
						channelType = (m_pSipCntl->GetBfcpType() == eMediaLineSubTypeUdpBfcp) ? BFCP_UDP:BFCP;

					SetPartyMonitorBaseParamsAndConnectChannel(channelType, actualRate, &remoteIp,&localIp,(DWORD)eUnknownAlgorithemCapCode,0,0,IsIce,&IceRemoteIp,&IceLocalIp);
				}

			}
		}

	}


	UpdateDbScm();
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateDbOnChannelsDisconnected()
{
	PTRACE2(eLevelInfoNormal,"CSipParty::UpdateDbOnChannelsDisconnected - ", m_partyConfName);
	BYTE bIsTransmit;
	EIpChannelType eChanType;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	m_pCurrentMode->Dump("CSipParty::UpdateDbOnChannelsDisconnected - current mode is : ",eLevelInfoNormal);
	for(int i=0 ; i<MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		for (int j=0; j<2; j++)
		{
			bIsTransmit = (globalDirectionArr[j] == cmCapTransmit);
			eChanType   = ::CalcChannelType(mediaType,bIsTransmit,eRole);
			if (m_pCurrentMode->IsMediaOff(mediaType,globalDirectionArr[j],eRole))
			{
				PTRACE2INT(eLevelInfoNormal,"CSipParty::UpdateDbOnChannelsDisconnected: channel is  ",(DWORD)eChanType);
				LogicalChannelDisconnect(eChanType);
			}
		}
	}

	UpdateDbScm();
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateDbScm()
{
	//comm mode:
	CSegment localScmSeg;
	CSegment remoteScmSeg;
	const CSipCaps*	pLocalCaps = NULL;
	const CSipCaps*	pRemoteCaps = NULL;

	TRACECOND_AND_RETURN(!m_pSipCntl, "CSipParty::UpdateDbScm: m_pSipCntl is NULL!!");

	pLocalCaps  = m_pSipCntl->GetLocalCaps();
	pRemoteCaps  = m_pSipCntl->GetLastRemoteCaps();

	if(pLocalCaps) {
		m_pCurrentMode->Serialize(localScmSeg,cmCapTransmit,YES, const_cast<CSipCaps*>(pLocalCaps));
		m_pConfApi->UpdateDB(this,LOCAL323COMMODE,0,1,&localScmSeg);
	} else {
	m_pCurrentMode->Serialize(localScmSeg,cmCapTransmit,YES);
	m_pConfApi->UpdateDB(this,LOCAL323COMMODE,0,1,&localScmSeg);
	}

	if(pRemoteCaps) {
		m_pCurrentMode->Serialize(remoteScmSeg,cmCapReceive,YES, const_cast<CSipCaps*>(pRemoteCaps));
		m_pConfApi->UpdateDB(this,RMOT323COMMODE,0,1,&remoteScmSeg);
	} else {
	m_pCurrentMode->Serialize(remoteScmSeg,cmCapReceive,YES);
	m_pConfApi->UpdateDB(this,RMOT323COMMODE,0,1,&remoteScmSeg);
	}

	DWORD rmtSetupRate = 0;
	CSipCall *pCall = m_pSipCntl->GetCallObj();
	if (pCall && pCall->IsCallInitiator() == NO)
		rmtSetupRate = m_pSipCntl->GetNetSetup()->GetRemoteSetupRate();
	
	//total rate
	DWORD totalRate = 0;
	CSegment* pRcveiveSeg  = new CSegment;
	totalRate =  min( m_pSipCntl->GetNetSetup()->GetMaxRate(), m_pCurrentMode->GetTotalBitRate(cmCapReceive));	
	if (rmtSetupRate)
		totalRate =  min(rmtSetupRate, totalRate);	
	*pRcveiveSeg << totalRate;
	m_pConfApi->UpdateDB(this,RECEIVEBAUDRATE,(DWORD) 0,1,pRcveiveSeg);
	POBJDELETE(pRcveiveSeg);

    CSegment* pTransmitSeg = new CSegment;
   	totalRate =  min( m_pSipCntl->GetNetSetup()->GetMaxRate(), m_pCurrentMode->GetTotalBitRate(cmCapTransmit));
	if (rmtSetupRate)
		totalRate =  min(rmtSetupRate, totalRate);	
	*pTransmitSeg << totalRate;
	m_pConfApi->UpdateDB(this,TRANSMITBAUDRATE,(DWORD) 0,1,pTransmitSeg);
	POBJDELETE(pTransmitSeg);
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateDbOnSipPrivateExtension()
{
	if (GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_SIP_IMS))
	{
		CSegment* pSeg = new CSegment;
		CSipHeaderList* pCdrHeaders = m_pSipCntl->GetCdrHeaders();
		if (pCdrHeaders)
		{
			pCdrHeaders->Serialize(NATIVE,*pSeg);
			m_pConfApi->UpdateDB(this,SIPPRIVATEEXTENSION,(DWORD)0,1,pSeg);
		}
		POBJDELETE(pSeg);
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyBadStatus(CSegment* pParam)
{
	DWORD opcode;
	DWORD len;
	char* strDescription; AUTO_DELETE_ARRAY(strDescription);
	*pParam >> opcode;
	*pParam >> len;
	strDescription = new char[len+1];
	pParam->Get((BYTE*)strDescription,len);
	strDescription[len] = 0;
	CSmallString str;
	str << "Bad status opcode " << opcode << ", description: " <<strDescription;
	PTRACE2(eLevelError,"CSipParty::OnPartyBadStatus ",str.GetString());
	m_eDialState = kBadStatusArrived;
	TellConfOnDisconnecting(SIP_BAD_STATUS);
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyBadStatusDisconnecting(CSegment* pParam)
{
	DWORD opcode;
	DWORD len;
	char* strDescription;
	*pParam >> opcode;
	*pParam >> len;
	strDescription = new char[len+1]; AUTO_DELETE_ARRAY(strDescription);
	pParam->Get((BYTE*)strDescription,len);
	strDescription[len] = 0;
	CSmallString str;
	str << "Bad status opcode " << opcode << ", description: " <<strDescription;
	PTRACE2(eLevelError,"CSipParty::OnPartyBadStatusDisconnecting: ",str.GetString());
	m_eDialState = kBadStatusArrived;
	//The call is already being closed, no need to close it again - BRIDGE-12305
	PTRACE(eLevelError,"CSipParty::OnPartyBadStatusDisconnecting: The call is already being closed");
	//CleanUp();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnConfSetMoveParams(CSegment* pParam)
{
	WORD confType, mcuNumber, terminalNumber;

	WORD tempWordForEnumConfType;
	BYTE bIsCopVideoTxModes = FALSE;
	*pParam >> tempWordForEnumConfType
			>> m_mcuNum
			>> m_termNum
			>> bIsCopVideoTxModes;
	if (bIsCopVideoTxModes)
	{
		CCopVideoTxModes* pTempCopVideoTxModes = new CCopVideoTxModes;
		pTempCopVideoTxModes->DeSerialize(NATIVE,*pParam);
		m_pSipCntl->SetCopVideoTxModes(pTempCopVideoTxModes);
		POBJDELETE(pTempCopVideoTxModes);
	}

    BYTE bIsOpPointsSet = FALSE;
    *pParam >> bIsOpPointsSet;
    if (bIsOpPointsSet)
    {
        CVideoOperationPointsSet* pVideoOperationPointsSet = new CVideoOperationPointsSet;
        pVideoOperationPointsSet->DeSerialize(*pParam);
        m_pTargetMode->SetOperationPoints(pVideoOperationPointsSet);
        m_pCurrentMode->SetOperationPoints(pVideoOperationPointsSet);
        POBJDELETE(pVideoOperationPointsSet);
    }
	// init 'speaker' params upon move, so FECC won't be operated on a wrong participant
	//      (the correct params will be updated on the next 'VIN' message).
	m_pSipCntl->InitSpeakerParams();

	m_pTargetMode->SetConfType((EConfType)tempWordForEnumConfType);


	m_pSipCntl->SendRemoteNumbering();
}

//BRIDGE-13154
BOOL CSipParty::GetBOOLDataByKey(const std::string& key)
{
	BOOL bRetVal = FALSE;

	CSysConfig* sysConfig = NULL;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();
	else
		PASSERTMSG(!pProcess,"!pProcess");

	if(sysConfig)
		sysConfig->GetBOOLDataByKey(key, bRetVal);
	else
		PASSERTMSG(!sysConfig,"!sysConfig");

	return bRetVal;
}
/////////////////////////////////////////////////////////////////////////////
//upon info flow control on people from remote
void CSipParty::OnChangeRate(CSegment* pParam)
{
	DWORD newRate;
	WORD channelDir;
	WORD roleLabel;
	*pParam >> newRate >> channelDir >> roleLabel;

	const CSipCaps* pRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
	const CSipCaps* pLocalCaps = m_pSipCntl->GetLocalCaps();

	CapEnum protocol = eUnknownAlgorithemCapCode;
	if (m_pCurrentMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople))
		protocol = (CapEnum)m_pCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople);

	//BRIDGE-13154
	const CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
    DWORD AudRate 	  = m_pTargetMode->GetMediaBitRate(cmCapAudio, cmCapTransmit) * 10;
    DWORD contentRate = m_pTargetMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
    DWORD totalRate   = m_pTargetMode->GetTotalBitRate(cmCapTransmit) / 100 ;
    BOOL  bIsActiveContent = GetIsActiveContent();

    if(bIsActiveContent && contentRate == 0)
    	contentRate = m_pSipCntl->GetFullContentRate();
    TRACEINTO << "newRate " << (int)newRate << " AudRate " << (int)AudRate << " bIsActiveContent " << (int)bIsActiveContent << " contentRate " << (int)contentRate  << " totalRate " << (int)totalRate;
    if (((cmCapDirection)channelDir == cmCapTransmit) && ((ERoleLabel)roleLabel == kRolePeople) && pRemoteCaps && (newRate <= pRemoteCaps->GetMaxVideoBitRate(protocol)) && pLocalCaps && (newRate <= pLocalCaps->GetMaxVideoBitRate(protocol)) &&
	   ((newRate + contentRate + AudRate <= totalRate) || (totalRate < 7680 && GetBOOLDataByKey(CFG_KEY_ENABLE_CONTENT_IN_PREFER_TIP_FOR_CALL_RATES_LOWER_THAN_768K) && pCommConf && pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP)))//BRIDGE-13154
	{
		if ((m_pCurrentMode->GetConfType() == kVideoSwitch) || (m_pCurrentMode->GetConfType() == kVSW_Fixed))
		{
			// VSW:
			BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
			if (!bEnableFlowControlVSW)
			{
				TRACEINTO << PARTYNAME << " - SUPPORT_VSW_FLOW_CONTROL is false";
				return;
			}
			if ((newRate < (VSW_FLOW_CONTROL_RATE_THRESHOLD * (m_pCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)roleLabel)))) && (m_pSipCntl->GetRemoteIdent() != PolycomMGC))
			{
				TRACEINTO << PARTYNAME << " - Flow control bit rate is lower than the threshold";
				return;
			}
			m_pCurrentMode->SetFlowControlRateConstraint(newRate);
			m_pTargetMode->SetVideoBitRate(newRate, cmCapTransmit, (ERoleLabel)kRolePeople);
		}
		else
		{
			// CP:
			m_pCurrentMode->SetVideoBitRate(newRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)roleLabel);
			m_pTargetMode->SetVideoBitRate(newRate, cmCapTransmit, (ERoleLabel)kRolePeople);
			if (m_pCurrentMode->GetConfType() == kCop)
			{
				const CCommConf* pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
				DBGPASSERT_AND_RETURN(!pCommCurrConf);

				DWORD newPeopleRate = m_pCurrentMode->CalcCopMinFlowControlRate(pCommCurrConf, newRate);
				m_pCurrentMode->SetFlowControlRateConstraint(newPeopleRate);
				m_pTargetMode->SetVideoBitRate(newRate, cmCapTransmit, (ERoleLabel)kRolePeople);
				m_pConfApi->UpdatePartyControlOnNewRate(GetPartyId(), newPeopleRate, (WORD)cmCapTransmit, (WORD)kRolePeople);
				return;
			}
		}

		m_pConfApi->UpdatePartyControlOnNewRate(GetPartyId(), newRate, (WORD)cmCapTransmit, (WORD)kRolePeople);
	}
	else if ((ERoleLabel)roleLabel & kRoleContentOrPresentation)
		TRACEINTO << PARTYNAME << " - Role Presentation, ignored";
	else
		TRACEINTO << PARTYNAME << " - Was not handled";

}

/////////////////////////////////////////////////////////////////////////////
//upon info flow control on people from remote
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyReceivedVideoPreference(CSegment* pParam)
{
	ESipTransactionType eSipTransType;
	PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedVideoPreference ");

	CapEnum protocol = eUnknownAlgorithemCapCode;
	if (m_pCurrentMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople))
				protocol = (CapEnum)m_pCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople);

	if(m_pCurrentMode->GetConfType() == kCp && protocol == eRtvCapCode)
	{
		//Shmulik Y: need to check if there is a change in pref. resolution -> if not ignore! it can cause intra over flooding
		CRtvVideoCap* pCurrentRtvCap = (CRtvVideoCap*) m_pCurrentMode->GetMediaAsCapClass(cmCapVideo,cmCapTransmit, kRolePeople);
		if (pCurrentRtvCap)
		{
			DWORD Width=0,Height=0,FR=0,BitRate=0;
			*pParam >> Width >> Height >> FR >> BitRate;

			RTVVideoModeDetails rtvVidModeDetails;
			DWORD CurrentBitRate = 0;
			pCurrentRtvCap->GetRtvCap(rtvVidModeDetails,CurrentBitRate);

			const CSipCaps* pLocalCaps = m_pSipCntl->GetLocalCaps();
			RTVVideoModeDetails localCapsRtvVidModeDetails;
			pLocalCaps->GetRtvCap(localCapsRtvVidModeDetails,BitRate);

        	if (m_pSipCntl->GetIsFecOn() || m_pSipCntl->GetIsRedOn())
        	{
        		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << ", CurrentBitRate:" << CurrentBitRate;
        		BitRate = min( BitRate, CurrentBitRate);
        	}
            if(m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU || m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU2013 || m_pSipCntl->GetRemoteIdent() == MicrosoftEP_MAC_Lync)
			{
				TRACEINTO << "MAC or AV-MCU  - changing rate according to asked resolution";
				const CSipCaps* pRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
				if(pRemoteCaps)
				{
						DWORD remoteBitRate = -1;
						if(pRemoteCaps->GetRtvCapBitRateAccordingToResolution(Width, Height, remoteBitRate))
							BitRate = min( BitRate, remoteBitRate);
				}
			}

			if ((Width == rtvVidModeDetails.Width && Height == rtvVidModeDetails.Height) ||
			   (rtvVidModeDetails.Width == localCapsRtvVidModeDetails.Width && rtvVidModeDetails.Height == localCapsRtvVidModeDetails.Height &&
				Width*Height >= rtvVidModeDetails.Width*rtvVidModeDetails.Height))
			{
				//if requested pref is the smae as current mode, or current mode is equal to max caps mode ->do nothing
				PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedVideoPreference - NO CHANGE in preference -> IGNORING.");
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedVideoPreference - Change in preference -> will SEND INTRA");
				*pParam << Width << Height << FR << BitRate;

				eSipTransType = kSipTransRTCPVideoUpdateInd;
				StartTransaction(eSipTransType, SIP_TRANS_UPDATE_VIDEO_PREFERENCE, pParam);
			}
		}
	}
	else // incase this is not CP or RTV ignore this req.
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyReceivedVideoPreference - Protocol:", protocol);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTransChangeVideoResAnycase(CSegment* pParam)
{
	DWORD Width = 0;
	DWORD Height = 0;
	DWORD FR = 0;
	DWORD BitRate = 0;

	*pParam >> Width >> Height >> FR >> BitRate;

	TRACEINTO << "Width:" << Width << ", Height:" << ", FR:" << FR << ", BitRate:" << BitRate;

	m_pConfApi->SendVideoPreferencesToPartyControl(GetPartyId(), Width, Height, FR, BitRate);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnConfVidBrdgUpdatedWithNewRes(CSegment* pParam)
{

	PTRACE(eLevelInfoNormal,"CSipParty::OnConfVidBrdgUpdatedWithNewRes");

	DWORD status = STATUS_OK;
	*pParam >> status;

	CSegment * pSeg = new CSegment;
	CPartyApi api;  // CTaskApi api;
	api.SetLocalMbx(GetLocalQueue());
	api.SendLocalMessage(pSeg, SIP_CONF_BRIDGES_UPDATED_RES);
}
/////////////////////////////////////////////////////////////////////////////
//Receive from chair control bridge - in cascade mode when the party in one of the links.
//Need to update the link about the terminal and MCU number that the chair control assignment.

void CSipParty::OnBridgeNumberingMessageAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnBridgeNumberingMessageAnycase - ", PARTYNAME);

	OPCODE opcode;
	WORD mcuNum, terminalNum;
	PartyRsrcID partyId;

	*pParam >> opcode >> mcuNum >> terminalNum >> partyId;

	switch(opcode)
	{
		case SEND_VIN:
		{
			m_pSipCntl->UpdateSeeingMcuTerminalNum(mcuNum, terminalNum, partyId);
			TRACEINTOFUNC << "mcuNum: " << mcuNum << ", terminalNum: " << terminalNum << ", partyId: " << partyId;
			break;
		}

		default:
		{
			PTRACE2INT(eLevelInfoNormal,"CSipParty::OnBridgeNumberingMessageAnycase, No such opcode = ", opcode);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////////////Data Token///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnFeccBridgeTokenRequest(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnFeccBridgeTokenRequest - ", PARTYNAME);
	m_pSipCntl->OnPartyFeccTokenReq(kTokenRequest);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnFeccBridgeTokenAccept(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnFeccBridgeTokenAccept - ", PARTYNAME);
	WORD isCameraControl = 0;
	*pParam >> isCameraControl;
	m_pSipCntl->OnPartyFeccTokenReq(kTokenAccept,isCameraControl);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnFeccBridgeTokenReject(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnFeccBridgeTokenReject - ", PARTYNAME);
	m_pSipCntl->OnPartyFeccTokenReq(kTokenReject);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnFeccBridgeTokenWithdraw(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnFeccBridgeTokenWithdraw - ", PARTYNAME);
	m_pSipCntl->OnPartyFeccTokenReq(kTokenWithdrow);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnFeccBridgeTokenReleaseRequest(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnFeccBridgeTokenReleaseRequest - ", PARTYNAME);
	m_pSipCntl->OnPartyFeccTokenReq(kTokenReleaseRequest);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnFeccBridgeTokenRelease(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnDatBrdgDatTokenRelease - ", PARTYNAME);
	m_pSipCntl->OnPartyFeccTokenReq(kTokenRelease);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnIpDataTokenMsg(CSegment* pParam)
{
	WORD msgType, bitRate;
	WORD isCameraControl = 0;

	*pParam >> msgType >> bitRate >> isCameraControl;

	switch (msgType)
	{
		case kTokenRequest:
			TRACEINTO << PARTYNAME << ", MsgType:kTokenRequest" << ", BitRate:" << bitRate << ", IsCameraControl:" << isCameraControl;
			m_pConfApi->DataTokenRequest(this, bitRate, isCameraControl);
			break;
		case kTokenRelease:
			TRACEINTO << PARTYNAME << ", MsgType:kTokenRelease" << ", IsCameraControl:" << isCameraControl;
			m_pConfApi->DataTokenRelease(this, isCameraControl);
			break;
		default:
			TRACEINTO << PARTYNAME << ", MsgType:" << msgType << " - Invalid message type";
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnIpFeccKeyMsg(CSegment* pParam)
{
	WORD msgType;
	*pParam >> msgType;

	TRACEINTO << PARTYNAME << ", MsgType:" << msgType << ", Key:" << ::feccKeyToString((feccKeyEnum)msgType);

	m_pConfApi->FeccKeyMsg(GetPartyRsrcID(), msgType);

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyUpdateConfRsrcIdForInterfaceAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyUpdateConfRsrcIdForInterfaceAnycase - ", PARTYNAME);
	DWORD confRsrcId;

	*pParam >> confRsrcId;
	m_ConfRsrcId = confRsrcId;
	m_pSipCntl->SetNewConfRsrcId(confRsrcId);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyDtmfIndAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyDtmfInd - ", PARTYNAME);
	CSegment *pSeg = new CSegment;
	DWORD len = 0;
	DWORD dtmfOpdoce = 0;
	unsigned char* tempArray = NULL;

	*pParam >> len;
	tempArray = new unsigned char[len];
	pParam->Get(tempArray, len);
	*pParam >> dtmfOpdoce;

	if (IsValidPObjectPtr(m_ivrCtrl) && m_ivrCtrl->IsExternalIVR())
	{
		*pSeg << dtmfOpdoce;
		*pSeg << len;
		pSeg->Put(tempArray,len);
		m_ivrCtrl->HandleEvent(pSeg, pSeg->GetLen(), IP_DTMF_INPUT_IND);
	}
	else
	{
		*pSeg << len;
		pSeg->Put(tempArray,len);
	m_pConfApi->IvrPartyNotification(GetPartyRsrcID(), this, GetName(), dtmfOpdoce, pSeg);
	}
	PDELETEA(tempArray);
	POBJDELETE(pSeg);
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyRemoteH230(CSegment* pParam)
{
	WORD  opcode		= 0;
	WORD  videoSyncLost = 0;
	BYTE  bIsGradualIntra = FALSE;
	ERoleLabel eRole = kRolePeople;
	WORD tempRole;
	DWORD remoteSsrc = NON_SSRC;
	DWORD prID = INVALID;

	ForwardRemoteH230ToMsSlavesControllerIfNeeded(pParam);

	*pParam >> opcode;

	TRACEINTO << GetFullName() << " , PartyId:" << GetPartyRsrcID() << ", Opcode:" << (DWORD)opcode;

	switch ( opcode )
	{
		case Fast_Update:
		{
			*pParam >> tempRole;
			*pParam >> videoSyncLost;
			*pParam >> bIsGradualIntra;
			if (!pParam->EndOfSegment())
			{
				*pParam >> remoteSsrc;
				*pParam >> prID;
				TRACEINTO << "remoteSsrc:" << remoteSsrc << ", prID:" << prID;
			}

			eRole = (ERoleLabel)tempRole;

			if(videoSyncLost == eReceiveIntra)// ask intra from CS (goes to the remote) through the filter mechanisem
			{
				if (eRole & kRoleContentOrPresentation)
					ContentIntraRequestFiltering();
				else
				{
					TRACEINTO << "dbg 1";
					if(!eAvMcuLinkSlaveIn == GetAvMcuLinkType())
						IntraRequestFiltering(videoSyncLost, bIsGradualIntra, remoteSsrc,prID);
					else
					{
						WORD ignore_filtering = FALSE;
						CSegment*  pSeg = new CSegment;
						*pSeg << ignore_filtering << remoteSsrc << prID;
						OnVidBrdgRefreshAnycase(pSeg);
						POBJDELETE(pSeg);
					}
				}
			}
			else
			{
				if (eRole & kRoleContentOrPresentation)
				{
					BYTE controlId  = 1;
					m_pConfApi->ContentVideoRefresh(controlId,YES,this);
				}
				else
				{
					TRACEINTO << "dbg 2";
					IntraRequestFiltering(videoSyncLost, bIsGradualIntra,remoteSsrc,prID);//m_pConfApi->VideoRefresh(this);
					if (videoSyncLost==1)
						m_rcvVcuCounter++;
				}
			}
		}
		break;

		default:
			break;
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartySinglIntraAvMcu(CSegment* pParam)
{
		TRACEINTO;
		IntraRequestFiltering(eTransmitIntra);

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyStreamsIntraReq(CSegment* pParam)
{
	if(m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU2013  && m_pSipCntl->isMs2013Active())
	{
		TRACEINTO << " forward streams intra request message to slaves controller";
		CSegment* pCopyParam = new CSegment();
		pCopyParam->CopySegmentFromReadPosition(*pParam);
		//pCopyParam->DumpHex();
		m_pConfApi->SendMsgToSlavesController(GetPartyId(),STREAMS_INTRA_REQ, pCopyParam);
	}

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::ForwardRemoteH230ToMsSlavesControllerIfNeeded(CSegment* pParam)
{
	if(/*m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU || */m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU2013  && m_pSipCntl->isMs2013Active())
	{
		TRACEINTO << " forward Remote H230 message to slaves controller";
		CSegment* pCopyParam = new CSegment();
		pCopyParam->CopySegmentFromReadPosition(*pParam);
		//pCopyParam->DumpHex();
		m_pConfApi->SendMsgToSlavesController(GetPartyId(),RMTH230, pCopyParam);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyCallFailed(CSegment* pParam)
{
	DWORD reason = 0xFFFFFFFF;
	DWORD MipErrorNumber = 0;

	*pParam >> reason;
	PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyCallFailed: Reason - ",reason);

	switch (reason)
	{
	case SIP_CARD_REJECTED_CHANNELS:
		PTRACE2(eLevelInfoNormal, "CSipParty::OnPartyCallFailed: Update channel failed", PARTYNAME);
		reason = MCU_INTERNAL_PROBLEM;//change for disconnect cause in GUI
		*pParam >> MipErrorNumber;
		break;
	case SIP_REMOTE_STOP_RESPONDING:
		PTRACE2(eLevelInfoNormal, "CSipParty::OnPartyCallFailed: Remote stopped responding", PARTYNAME);
		break;
	default:;
		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyCallFailed: Reason - ",reason);
	}

	TellConfOnDisconnecting(reason,NULL,MipErrorNumber);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyCallReinvite(CSegment* pParam)
{
	if (!IsActiveTransaction()) {
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyCallReinvite: no active transactions, start re-invite for session timer");
		StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, pParam);
	}
	else
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyCallReinvite: active transactions exists, ignore re-invite for session timer");
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyCallFailedDisconnecting(CSegment* pParam)
{
	if(m_eDialState==kReInviteSent)
	{
		//waited for the response even if the response is fail.
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyCallFailedDisconnecting-waiting to re-invite response");
		m_pSipCntl->SipInviteAckReq();	// for the response
		m_pSipCntl->CloseCall(YES);		// disconnect call
	}

	PTRACE(eLevelError,"CSipParty::OnPartyCallFailedDisconnecting");
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyReInviteResponseDisconnecting(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReInviteResponseDisconnecting : Waited for that message. Now can disconnect.");
	m_pSipCntl->SipInviteAckReq();	// for the response
	m_pSipCntl->CloseCall(YES);		// disconnect call
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyReceivedAckDisconnecting(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedAckDisconnecting");
	//ack for ok or re-invite ok/reject. it is ok to close now
	m_pSipCntl->CloseCall(YES);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::SerializeNetSetup(DWORD channelType,CSegment* pSeg)
{
	// we send the NetSetup only in the connecting of SIGNALING for CDR use!
	if((EIpChannelType)channelType == SIGNALING)
	{
		CSipNetSetup* pNetSetup = new CSipNetSetup;
		*pNetSetup = *(m_pSipCntl->GetNetSetup());
		pNetSetup->Serialize(NATIVE,*pSeg);
		POBJDELETE(pNetSetup);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyTransportError(CSegment* pParam)
{
	PTRACE2(eLevelError,"CSipParty::OnPartyTransportError - ", m_partyConfName);
	DWORD expectedReq;
	DWORD disconnectionCause = SIP_TRANSPORT_ERROR;

	*pParam >> expectedReq;
	if (m_eDialState == kInviteSent && m_pSipCntl->GetTransportProtocol() == eTransportTypeTcp)
		disconnectionCause = SIP_TRANS_ERROR_TCP_INVITE;
	if ((sipTransportErrorExpectedReq)expectedReq == SipTransportErrorDelete)
		m_eDialState = kTransportErrorArrived;

	TellConfOnDisconnecting(disconnectionCause);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyTransportErrorDisconnecting(CSegment* pParam)
{
	DWORD expectedReq;
	*pParam >> expectedReq;
	TRACEINTO << m_partyConfName << ", expectedReq:" << expectedReq;

	if ((sipTransportErrorExpectedReq)expectedReq == SipTransportErrorDelete)
	{
		m_eDialState = kTransportErrorArrived;
		CleanUp();
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::SetPartyToSecondary(WORD reason, CSecondaryParams* pSecParamps)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", Reason:" << reason;

	m_pTargetMode->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit);

	m_pConfApi->PartyMoveToSecondery(GetPartyId(), reason, pSecParamps);
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::DisconnectChannels() // according to target and current modes
{
	BYTE bIsTransmit	= NO;
	EIpChannelType chanType;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for (int i=0 ; i<MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		for (int j=0; j<2; j++)
		{
			if (m_pCurrentMode->IsMediaOn(mediaType, globalDirectionArr[j], eRole) &&
				m_pTargetMode->IsMediaOff(mediaType, globalDirectionArr[j], eRole))
			{
				bIsTransmit = (globalDirectionArr[j] == cmCapTransmit);
				chanType = ::CalcChannelType(mediaType,bIsTransmit,eRole);
				m_pSipCntl->SipCloseChannelReq(chanType);
			}
		}
	}
	DBGPASSERT(YES);// Check it - need to be done with StartTransacion
	m_pSipCntl->SipReInviteReq(0);
	m_eDialState = kReInviteSent;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::CleanUp()
{
	PTRACE(eLevelInfoNormal, "CSipParty::CleanUp");
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipParty::IsMediaContaining(cmCapDataType dataType, cmCapDirection direction)
{
	DWORD videoValuesToCompare = kCapCode|kFormat|kFrameRate|kH264Profile|kH264Level|kH264Additional|kBitRate;
	DWORD audioValuesToCompare = kCapCode|kFrameRate;
	DWORD valuesToCompare	  = 0;
	DWORD details			  = 0;
	BYTE bMediaContaining = YES;
	PTRACE(eLevelInfoNormal, "CSipParty::IsMediaContaining");
	if(m_pTargetMode->IsMediaOn(dataType,direction) && m_pCurrentMode->IsMediaOn(dataType, direction))
	{
		valuesToCompare = (dataType == cmCapAudio)?audioValuesToCompare:videoValuesToCompare;
		if(direction == cmCapTransmit)
			bMediaContaining = m_pTargetMode->IsMediaContaining(*m_pCurrentMode, valuesToCompare,&details,dataType,direction);
		else
			bMediaContaining = m_pTargetMode->IsMediaContaining(*m_pCurrentMode, kCapCode,&details,dataType,direction);
	}
	else if(m_pTargetMode->IsMediaOn(dataType,direction) || m_pCurrentMode->IsMediaOn(dataType, direction))
		bMediaContaining = NO;

	return bMediaContaining;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendSiteAndVisualNamePlusProductIdToPartyControl(CSegment* pParam)
{
	BYTE  isCascadeToCopMcu;
	BYTE  isSiteName;
	DWORD lenSiteName;
	BYTE  isProductId;
	DWORD lenProductId;
	BYTE  isVersionId;
	DWORD lenVersionId;

	*pParam
		>> isCascadeToCopMcu
		>> isSiteName
		>> lenSiteName
		>> isProductId
		>> lenProductId
		>> isVersionId
		>> lenVersionId;

	TRACEINTO << "lenSiteName:" << lenSiteName << ", lenProductId:" << lenProductId << ", lenVersionId:" << lenVersionId;

	if (lenSiteName || lenProductId || lenVersionId)
	{
		char* strSiteName  = NULL;
		char* strProductId = NULL;
		char* strVersionId = NULL;

		if (lenSiteName)
		{
			strSiteName  = new char[lenSiteName]; // lenSiteName can be 0 or string length + 1 for null termination
			pParam->Get((unsigned char*)strSiteName, lenSiteName);
			strSiteName[lenSiteName-1] = '\0';
		}

		if (lenProductId)
		{
			strProductId = new char[lenProductId]; // lenProductId can be 0 or string length + 1 for null termination
			pParam->Get((unsigned char*)strProductId, lenProductId);
			strProductId[lenProductId-1] = '\0';
		}

		if (lenVersionId)
		{
			strVersionId = new char[lenVersionId]; // lenVersionId can be 0 or string length + 1 for null termination
			pParam->Get((unsigned char*)strVersionId, lenVersionId);
			strVersionId[lenVersionId-1] = '\0';
		}

		BYTE tmp;
		*pParam >> tmp;
		eTelePresencePartyType telePresencePartyType = (eTelePresencePartyType)tmp;

		// speakerIndication
		BYTE tmpRemoteVendorIdent;
		*pParam >> tmpRemoteVendorIdent;
		RemoteIdent eRemoteVendorIdent = (RemoteIdent)tmpRemoteVendorIdent;

		TRACEINTO << "strSiteName:" << DUMPSTR(strSiteName) << ", strProductId:" << DUMPSTR(strProductId) << ", strVersionId:" << DUMPSTR(strVersionId) << ", DialInName:" << m_name << ", TelePresencePartyType:" << (DWORD)telePresencePartyType << ", RemoteVendorIdent:" << (DWORD)eRemoteVendorIdent;

		m_pConfApi->SendSiteAndVisualNamePlusProductIdToPartyControl(this, isSiteName, lenSiteName, strSiteName, isProductId, lenProductId, strProductId, isVersionId, lenVersionId, strVersionId, telePresencePartyType, isCascadeToCopMcu, eRemoteVendorIdent);

		delete[] strSiteName;
		delete[] strProductId;
		delete[] strVersionId;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::IntraRequestFiltering(WORD videoSyncLost, BYTE bIsGradualIntra, DWORD remoteSSRC, DWORD priorityID, DWORD msSlavePartyIndex)
{
	TICKS curTimer;
	RemoteIdent remoteIdent = m_pSipCntl->GetRemoteIdent();
	BOOL isLync2010	= remoteIdent == MicrosoftEP_Lync_R1 ? YES:NO;

	DWORD tipPackLossInterval = 0;
	CSysConfig* sysConfig = NULL;
	CProcessBase *pProcess = CProcessBase::GetProcess();
	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();
	if(sysConfig)
		sysConfig->GetDWORDDataByKey(CFG_KEY_TIP_PACKET_LOSS_SEND_INTRA_INTERVAL_SEC, tipPackLossInterval);

	//Fast Update requests will be sent to a party
	//in a minimum interval of 1 seconds
	curTimer = SystemGetTickCount();

	if( videoSyncLost == eReceiveIntra )
	{
	     if (curTimer < m_lastVcuTime)
	     	     m_lastVcuTime = 0; //rollover fixup

	     if ((m_lastVcuTime == 0) || (curTimer - m_lastVcuTime > SECOND*1 ))
	     {
	     	     SendFastUpdateReq(kRolePeople, remoteSSRC, priorityID, msSlavePartyIndex);
	     	     m_lastVcuTime = curTimer;
	     }
	     else if( isLync2010 && IsLyncRTCPIntraEnabled())
	     {
	    	 PTRACE(eLevelInfoNormal,"CSipParty::IntraRequestFiltering - we enable intra for Lync");
	    	 SendFastUpdateReq(kRolePeople); //m_pSipCntl->FastUpdateReq(kRolePeople);
	    	 m_lastVcuTime = curTimer;
	     }
	     else if (m_pSipCntl->isMs2013Active())
	     {
     	     SendFastUpdateReq(kRolePeople, remoteSSRC, priorityID, msSlavePartyIndex);
     	     m_lastVcuTime = curTimer;
	     }
	}
	else
	{
		// Gradual intra requests can be received in very small intervals and because currently we don't support gradual intra but only full intra,
		// we don't want to send full intra for each gradual intra request so we filter these requests with 3 seconds filter.
		// Currently gradual Intra requests are received only from TIP devices.
		DWORD filterTime = SECOND*1; // regular filter

		if( GetIsTipCall() )
		    filterTime = SECOND*0; // do not filter any IDR intra request in TIP call

		if (bIsGradualIntra)
			filterTime = SECOND*tipPackLossInterval;   // filter for gradual intra. //BRIDGE-4459 (tipPackLossInterval ) :

	     if (curTimer < m_lastBridgeRefreshTime)
	     	     m_lastBridgeRefreshTime = 0; //rollover fixup

	 	CLargeString str;
	 	str << "CSipParty::IntraRequestFiltering ";

	 	str << " curTimer=" << curTimer.GetMiliseconds();
	 	str << " m_lastBridgeRefreshTime=" << m_lastBridgeRefreshTime.GetMiliseconds();
	 	str << " diffInMilSec=" << (curTimer - m_lastBridgeRefreshTime).GetMiliseconds();


	     if ((m_lastBridgeRefreshTime == 0) || (curTimer - m_lastBridgeRefreshTime > filterTime ))
	     {
	    	 m_nIntraCounter++;

			 m_pConfApi->VideoRefresh(GetPartyId());
			 m_lastBridgeRefreshTime = curTimer;

			 str << " Refresh was sent " << ",\n Counter: id:" << GetPartyId() <<", intra request counter:" << m_nIntraCounter;
			 str << ",\n num of intras request after timer:" << m_nIntraAfterTimer;
			 str << ",\n num of intras request after transaction:" << m_nIntraAfterTransaction;

			 if(GetIsTipCall() && bIsGradualIntra)
			 {
				 m_TipNotSentIntraResponseCounter = 0;

//				 str << " Refresh was sent " << " , intra request counter:" << m_nIntraCounter;
//				 str << ", intra request after timer:" << m_nIntraAfterTimer;

				 if(IsValidTimer(TIP_GRADUAL_TIMER))
				 {
					 DeleteTimer(TIP_GRADUAL_TIMER);
					 str << " TIP_GRADUAL_TIMER was deleted";
				 }
			 }
	     }
	     else if (GetIsTipCall() && bIsGradualIntra)
	     {
	    	 m_TipNotSentIntraResponseCounter++;

	    	 if(m_TipNotSentIntraResponseCounter == 1)
	    	 {
	    		 m_nIntraAfterTimer++;
	    		 DWORD timerInterval = (filterTime * 10 - (curTimer - m_lastBridgeRefreshTime).GetMiliseconds())/10 + 10;
	    		 StartTimer(TIP_GRADUAL_TIMER, timerInterval);
	    		 str << " TIP_GRADUAL_TIMER was started for ";
	    		 str << " timerInterval="<<timerInterval;
	    	 }

	     }

	     PTRACE(eLevelInfoNormal,str.GetString());

	}
	//else
	//   timer has not elapsed, don't sendelse
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::FirFilterCreate()
{
	if (m_bIsFirFilterCreated)
		return;

	int i = 0;
	const std::list <StreamDesc> streamsDescList = GetCurrentMode()->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
	std::list <StreamDesc>::const_iterator itr;

	if (streamsDescList.size() == 0)
		return;

	m_bIsFirFilterCreated = TRUE;

	for (itr = streamsDescList.begin(); itr != streamsDescList.end(); itr++)
	{
		if (i < MAX_NUM_RECV_STREAMS_FOR_FIR_FILTER)
		{
			m_FirFilter[i].active = TRUE;
			m_FirFilter[i].ssrc = itr->m_pipeIdSsrc;
			m_FirFilter[i].lastFirTime = 0;

			CLargeString str;
		 	str << "CSipParty::FirFilterCreate";
		 	str << ", i = " << i;
		 	str << ", ssrc = " << itr->m_pipeIdSsrc;
		    PTRACE(eLevelInfoNormal,str.GetString());

		    i++;
		}
		else
		{
			CLargeString str;
		 	str << "CSipParty::FirFilterCreate failed";
		 	str << ", i = " << i;
		 	str << ", ssrc = " << itr->m_pipeIdSsrc;
		    PTRACE(eLevelError,str.GetString());
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipParty::FirFilter(unsigned int ssrc, TICKS curTimer)
{
	if (!m_bIsFirFilterCreated) {
		CLargeString str;
	 	str << "CSipParty::FirFilter failed, not created";
	 	str << ", ssrc = " << ssrc;
	    PTRACE(eLevelError,str.GetString());
		return FALSE;
	}

	for (int i = 0; i < MAX_NUM_RECV_STREAMS_FOR_FIR_FILTER; i++) {

		//ssrc found
		if (m_FirFilter[i].active == TRUE && m_FirFilter[i].ssrc == ssrc) {

			if (curTimer < m_FirFilter[i].lastFirTime)
				m_FirFilter[i].lastFirTime = 0;

			if (m_FirFilter[i].lastFirTime == 0 || (curTimer - m_FirFilter[i].lastFirTime > SECOND*1)) {
				m_FirFilter[i].lastFirTime = curTimer;
			    return TRUE;
			}
			else {
				return FALSE;
			}
		}
	}

	CLargeString str;
 	str << "CSipParty::FirFilter failed";
 	str << ", ssrc = " << ssrc;
    PTRACE(eLevelError,str.GetString());

    return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnGradualTout()
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnGradualTout");
	IntraRequestFiltering(2,true);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendFastUpdateReq(ERoleLabel eRole, DWORD remoteSSRC, DWORD priorityID, DWORD msSlavePartyIndex )
{
	ETipVideoPosition tipPos = eTipVideoPosCenter;
	if (eRole & kRoleContentOrPresentation)
		tipPos = eTipVideoPosAux5Fps;
	m_pSipCntl->FastUpdateReq(eRole, tipPos, remoteSSRC, priorityID, msSlavePartyIndex );
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyDisconnectToutDisconnecting(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyDisconnectToutDisconnecting - ", m_partyConfName);
	DBGPASSERT(YES);

	// erase channels
	if (m_pSipCntl->GetNumOfChannels())
	{
		EConfType eConfType = m_pTargetMode->GetConfType();
		m_pCurrentMode->SetAllModesOff(cmCapReceiveAndTransmit);
		m_pCurrentMode->SetConfType(eConfType);
		UpdateDbOnChannelsDisconnected();
	}

	if ( m_eDialState == kRejectedByLobby ||
    	     m_eDialState == kFailedInLobby ||
    	     m_eDialState == kTransferFailed )
	     m_eDialState = kDisconnectTimerAfterRejectedByLobby;
    	else
	    m_eDialState = kDisconnectTimer;

	CleanUp();
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartySendFaultyMfaToPartyCntlAnycase(CSegment *pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartySendFaultyMfaToPartyCntlAnycase - ", PARTYNAME);
	m_pConfApi->SendFaultyMfaNoticeToPartyCntl(GetPartyId(),STATUS_FAIL);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::DispatchPartyCallClosed()
{
	CSegment *pSeg = new CSegment;

	CSipComMode* pCurrentMode = new CSipComMode;
	pCurrentMode->Create(*m_pSipCntl->GetCallObj());
	pCurrentMode->CopyStaticAttributes(*m_pTargetMode);
	pCurrentMode->Serialize(NATIVE, *pSeg);

    DispatchEvent(SIP_PARTY_CALL_CLOSED,pSeg);
	POBJDELETE(pSeg);
	POBJDELETE(pCurrentMode);
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyConnectTout(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipParty::OnPartyConnectTout: Disconnect call");
	WORD callIndex = m_pSipCntl->GetCallIndex();
	DBGPASSERT(callIndex);
	m_eDialState = kConnectTimer;
	TellConfOnDisconnecting(SIP_TIMER_POPPED_OUT);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyConnectToutDisconnecting(CSegment* pParam)
{
	PTRACE(eLevelError,"CSipParty::OnPartyConnectToutDisconnecting");
	switch (m_eDialState)
	{
	case kOkSent:
	case kReInviteSent:
	case kReInviteArrived:
	case kReInviteRejected:
	case kReInviteAccepted:

		DBGPASSERT(m_eDialState+100);
		m_eDialState = kConnectTimer;
		//stop waiting for call to be connected
		CleanUp();
		break;

	default:
		PTRACE(eLevelError,"CSipParty::OnPartyConnectToutDisconnecting: Shouldn't have got here!");
		DBGPASSERT(m_eDialState);
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::InformConfRemoteConnect()
{
	PTRACE2(eLevelInfoNormal,"CSipParty::InformConfRemoteConnect - ", m_partyConfName);
	m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pTargetMode, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::GetIpCallIdentifiers (IP_EXT_DB_STRINGS* ipStringsStruct)
{
    if(!ipStringsStruct)
    {
        PASSERTMSG(m_pSipCntl->GetConId(), "CSipParty::GetIpCallIdentifiers - No ipStringsStruct!!");
        return;
    }

	const CSipNetSetup *pNetSetup = m_pSipCntl->GetNetSetup();
    strncpy (ipStringsStruct->identifier[0], pNetSetup->GetRemoteSipAddress(), IDENTIFIER_STR_SIZE);
    // ToGuy - remove member and switch to mcTransportAddr
    mcTransportAddress ip;
    memset(&ip,0,sizeof(mcTransportAddress));

    if (!strcmp(NameOf(),"CSipPartyIn"))
        memcpy(&ip,pNetSetup->GetTaSrcPartyAddr(),sizeof(mcTransportAddress));
    else
    	memcpy(&ip,pNetSetup->GetTaDestPartyAddr(),sizeof(mcTransportAddress));

    ::ipToString(ip, ipStringsStruct->ipAddress,1);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::ConnectCall(BYTE isInitialConnection)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::ConnectCall - ", m_partyConfName);

	m_state = PARTYCONNECTED;
//	m_bIsCallConnected = TRUE;

	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
	if (pServiceParams == NULL)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::ConnectCall: IP Service does not exist!!! ServiceID = ",m_serviceId);//m_pSipCntl->GetServiceId());
		return;
	}

	ipAddressIf localAddress;
	mcTransportAddress localIp;
	memset(&localIp,0,sizeof(mcTransportAddress));
	mcTransportAddress remoteIp;
	memset(&remoteIp,0,sizeof(mcTransportAddress));

	CSipNetSetup* pNetSetup = const_cast<CSipNetSetup*>(m_pSipCntl->GetNetSetup());
	mcTransportAddress* pDestTaAddr = const_cast<mcTransportAddress*>(pNetSetup->GetTaDestPartyAddr());
	if (pNetSetup->GetIpVersion() == eIpVersion4)
	{
		localAddress = pServiceParams->GetIpV4Address();
		localIp.addr.v4.ip = localAddress.v4.ip;
	}
	else
	{
		BYTE place = ::FindIpVersionScopeIdMatchBetweenPartyAndService(pDestTaAddr, pServiceParams);
		if (place == 0xFF)
		{
			PASSERTMSG(4,"CSipParty::ConnectCall - No IpV6 in Service");
			return;
		}
		localAddress = pServiceParams->GetIpV6Address((int)place);
		memcpy(&localIp.addr,&localAddress,sizeof(ipAddressIf));
		localIp.ipVersion = eIpVersion6;
	}

	if(m_bIsRcvCheckCompleteAndNeedToStartIVR)
		m_pSipCntl->GetOriginalRmtIpAddress(cmCapAudio,remoteIp);
	else
		m_pSipCntl->GetRemoteMediaIpAsTrAddr(cmCapAudio,kRolePeople,remoteIp);

	if (pServiceParams->GetSipTransportType() == eTransportTypeTls) {
		localIp.port = TLS_PORT;
		remoteIp.port = TLS_PORT;
	}
	else {
		localIp.port = 5060;
		remoteIp.port = 5060;
	}

	localIp.transportType = pServiceParams->GetSipTransportType();
	remoteIp.transportType = pServiceParams->GetSipTransportType();

	DWORD actualRate = 0xFFFFFFFF;
	if(isInitialConnection)
		SetPartyMonitorBaseParamsAndConnectChannel(SIGNALING,actualRate,&remoteIp,&localIp);

//	PDELETEA(connectType);

	UpdateDbOnSipPrivateExtension();
	const CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);

    if (m_pSipCntl->GetIsIceCall()) // on ICE calls we will start the IVR when the connectivity check is complete
    {
    	/* Check IVR ice-check-complete code change. */
	//BRIDGE12419: start IVR for plugin in EQ
	if(!m_pSipCntl->GetIsCCSPlugin())
	{
		StartIvr();
	}
	else
	{
		if(pCommConf && pCommConf->GetEntryQ())
		{
			PTRACE(eLevelInfoNormal,"CSipParty::ConnectCall: StartIvr for Css plugin in EQ" );
			StartIvr();
		}
	}

    }
//   	else if (GetIsTipCall())// in case of TIP can't start IVR until TIP negotiation is finished.
//   	{
//
//   	}
   	else
   	{
   		PTRACE(eLevelInfoNormal,"CSipParty::ConnectCall: Not ICE Call " );

   		if(!GetIsTipCall() && (m_IsPartialConnectionForVSW == FALSE))
   		{
			//BRIDGE12419: start IVR for plugin in EQ
			if(!m_pSipCntl->GetIsCCSPlugin())
			{
				StartIvr();
			}
			else
			{
				if(pCommConf && pCommConf->GetEntryQ())
				{
					PTRACE(eLevelInfoNormal,"CSipParty::ConnectCall: StartIvr for Css plugin in EQ" );
					StartIvr();
				}
			}
   		}
   		else
   		{
    		PTRACE(eLevelInfoNormal,"CSipParty::ConnectCall: or Tip call or m_IsPartialConnectionForVSW, IVR not started yet" );
   		}
   	}

	
	//Change the Conf layout when RSS-Playback dial in
	if (pCommConf)
	{
		BYTE  initLayout = 0;
		CConfParty*  pConfParty = NULL;
     		pConfParty= pCommConf->GetCurrentParty(GetMonitorPartyId());
		if(pConfParty && YES == pConfParty->GetPlaybackLinkParty())
		{
			initLayout =	pConfParty->GetLastLayoutForRL();			
			PTRACE2INT(eLevelInfoNormal,"CSipParty::ConnectCall: initial layout from RSS playback session - ",(DWORD) initLayout);
			if(initLayout == eSrsVideoLayoutAuto || initLayout == eSrsVideoLayoutLecture)
				m_pConfApi->SendLayoutControlToConf(GetPartyId(), initLayout);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
/////                    Sip Transaction functions                         //////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::StartTransaction(ESipTransactionType eTransactionType, OPCODE opcode, CSegment * pParam)
{
	DBGPASSERT_AND_RETURN(IsActiveTransaction());

	TRACEINTO << m_partyConfName << ", TransactionType:" << GetTransactionTypeAsString(eTransactionType);

	POBJDELETE(m_pSipTransaction);

	if(eTransactionType == kSipTransInviteWithSdpReq)
		m_pSipTransaction = new CSipTransInviteWithSdpReq(this);
	else if(eTransactionType == kSipTransInviteWithSdpInd)
		m_pSipTransaction = new CSipTransInviteWithSdpInd(this);
	else if(eTransactionType == kSipTransInviteNoSdpInd)
		m_pSipTransaction = new CSipTransInviteNoSdpInd(this);
	else if(eTransactionType == kSipTransReInviteWithSdpInd)
		m_pSipTransaction = new CSipTransReInviteWithSdpInd(this);
	else if(eTransactionType == kSipTransReInviteNoSdpInd)
		m_pSipTransaction = new CSipTransReInviteNoSdpInd(this);
	else if(eTransactionType == kSipTransReInviteWithSdpReq)
		m_pSipTransaction = new CSipTransReInviteWithSdpReq(this);
	else if(eTransactionType == kSipTransRTCPVideoUpdateInd)
		m_pSipTransaction = new CSipTransRTCPVideoUpdateInd(this);
	else if(eTransactionType == kSipTransInviteMrcWithSdpInd)
		m_pSipTransaction = new CSipTransInviteMrcWithSdpInd(this);
	else if(eTransactionType == kSipTransInviteMrcNoSdpInd)
		m_pSipTransaction = new CSipTransInviteMrcNoSdpInd(this);
	else if(eTransactionType == kSipTransInviteMrcSlaveWithSdpReq)
		m_pSipTransaction = new CSipTransInviteMrcSlaveWithSdpReq(this);
	else if (eTransactionType == kSipTransUpgradeAvcOnlyToMixReq)
		m_pSipTransaction = new CSipTransAvcUpgradeToMixed(this);  // ey_20866 - temporary - to change to CSipTransAvcUpgradeToMixed
	else if (eTransactionType == kSipTransUpgradeSvcOnlyToMixReq)
		m_pSipTransaction = new CSipTransSvcUpgradeToMixed(this);  // ey_20866
	else if (eTransactionType == kSipTransRTCPVsrInd)
		m_pSipTransaction = new CSIPTransRTCPVsrInd(this);
	else if (eTransactionType == kSipTransInviteWebRtcWithSdpInd)
		m_pSipTransaction = new CSipTransInviteWebRtcWithSdpInd(this);
	else if (eTransactionType == kSipTransReInviteMrcWithSdpReq)    //FSN-613: Dynamic Content for SVC/Mix Conf
		m_pSipTransaction = new CSipTransReInviteMrcWithSdpReq(this);
	else
	{
		DBGPASSERT(eTransactionType+100);
		m_pSipTransaction = new CSipTransaction(this);
	}

	DBGPASSERT_AND_RETURN(!m_pSipTransaction);

	m_eActiveTransactionType = eTransactionType;

	// At start transaction, the Updated target mode is set to the active mode
	m_maskRequiredChangeMode = eChangeModeMask_None;
	*m_pUpdateTargetMode = *m_pTargetMode;

	m_pSipTransaction->InitTransaction(this, m_pSipCntl, m_pCurrentMode, m_pTargetMode, &m_eDialState, m_partyConfName, m_voice, m_alternativeAddrStr,m_pTargetModeMaxAllocation, m_bTransactionSetContentOn, m_isContentSuggested, m_changeModeState == eFallBackFromTipToSip , IsGlareStatus()); //BRIDGE-12961 IsGlareStatus
	m_changeModeState = eNotNeeded;

	if (opcode != DEFAULT_OPCODE)
		DispatchEvent(opcode, pParam);

	m_bTransactionSetContentOn = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
ESipTransactionType CSipParty::EndTransaction(EPendingTransType &bPendingTrns)
{
	CConfParty* pConfParty = GetConfPartyNonConst();
	PASSERT_AND_RETURN_VALUE(!pConfParty, kSipTransNone);
	bPendingTrns = eNoPendTrans;

	TRACEINTO << m_partyConfName << ", TransactionType:" << GetTransactionTypeAsString(m_eActiveTransactionType);

	ESipTransactionType retTransType = m_eActiveTransactionType;

	m_isContentSuggested = m_pSipTransaction->IsContentSuggested(); // Added by Efi

	if ((m_eActiveTransactionType == kSipTransReInviteWithSdpInd) ||
	    (m_eActiveTransactionType == kSipTransReInviteNoSdpInd) )
		m_pSipCntl->SetIsReInviteTransaction(NO);

	// Although the re invite (of PolycomRMX and Polycom EP) did not influence the rate of the localCap,  we want to make sure that the new rate was send by info msg
	if ((m_eActiveTransactionType == kSipTransReInviteWithSdpReq) && (m_pSipCntl->GetRemoteIdent() == PolycomRMX || m_pSipCntl->GetRemoteIdent() == PolycomEp) && (m_pSipCntl->IsPendingTrns() == eNoPendTrans))
	{
		if (m_pTargetMode->GetFlowControlRateConstraint())
			m_pSipCntl->SendFlowControlReq(mainType, cmCapReceive, m_pTargetMode->GetFlowControlRateConstraint());

		if (m_pTargetMode->GetFlowControlRateConstraintForPresentation())
			m_pSipCntl->SendFlowControlReq(slideType, cmCapReceive, m_pTargetMode->GetFlowControlRateConstraintForPresentation());
	}


	m_eActiveTransactionType = kSipTransNone;

	CIceParams* pIceParams = NULL;
	BYTE bIsIceParty = m_pSipCntl->GetIsEnableICE();

	if (m_pSipCntl->GetICEParams() && bIsIceParty)
	{
		CIceParams* tmpIceParams = m_pSipCntl->GetICEParams();

		pIceParams  = new CIceParams;
		*pIceParams = (CIceParams&)*tmpIceParams;

		m_pConfApi->UpdatePartyCntlOnICEParams(GetPartyId(), bIsIceParty, pIceParams);
	}
	else
		m_pConfApi->UpdatePartyCntlOnICEParams(GetPartyId(), bIsIceParty, NULL);

	if (m_bIsNeedToUpdateContentOnTheTransactionEnd)
	{
		m_bIsNeedToUpdateContentOnTheTransactionEnd = false;
		m_pSipCntl->UpdatePresentationOutStream();
		PTRACE(eLevelInfoNormal, "CSipParty::EndTransaction -update content on ");
	}

	POBJDELETE(pIceParams);
	EPendingTransType pendTrans = m_pSipCntl->IsPendingTrns();
	if (pendTrans != eNoPendTrans)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::EndTransaction -need to start pending reinvite ");
		if (pendTrans == etransReinvite)
			m_pSipCntl->OnSipReInviteIndConnected(NULL);
		else if (pendTrans == etransBye)
			m_pSipCntl->OnSipByeInd(NULL);

		bPendingTrns = pendTrans;
	}
	else
	{
		const eTokenRecapCollisionDetectionType trcdState 	= pConfParty->GetTokenRecapCollisionDetection();
		const BOOL								trcdPending	= pConfParty->IsTokenRecapPendedDueToCollisionDetection();
		if (trcdState == etrcdRecapInProgress && trcdPending)
		{
			//=======================================================================
			// A token has been pended due to the ongoing reinvite, handling it now
			//=======================================================================
			PTRACE(eLevelInfoNormal, "CSipParty::EndTransaction - a token has been pended, handling it now");
			pConfParty->SetTokenRecapCollisionDetection(etrcdAvailable);
			pConfParty->UnpendTokenRecapDueToCollisionDetection();
			SpreadAllH239Msgs(&m_pendedToken, m_pendedTokenDirection, m_pendedTokenIsOldToken, m_pendedTokenOpcode);
		}
	    else if (trcdState == etrcdRecapInProgress)
		{
			//=============================================================================
			// No pends happened, still need to reset the flags as this change-mode ended
			//=============================================================================
	    	PTRACE(eLevelInfoNormal, "CSipParty::EndTransaction - no pends, just reset the flags");
			pConfParty -> SetTokenRecapCollisionDetection(etrcdAvailable);
		}
	    else if (trcdState == etrcdAvailable && trcdPending)
	    {
			//====================================================
			// Not expecting a pend while nothing is in progress
			//====================================================
			pConfParty -> UnpendTokenRecapDueToCollisionDetection();
			PTRACE(eLevelError,"CSipParty::EndTransaction - a trcd pend got noted, with nothing in progress");
	    }
	}

	return retTransType;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::EndTransactionByPartyIfNeeded()
{
	PTRACE2(eLevelInfoNormal,"CSipParty::EndTransactionByPartyIfNeeded - ", m_partyConfName);
	if(IsActiveTransaction())
	{
		m_pSipTransaction->EndTransactionByParty();
		EPendingTransType bPendTrans;
		EndTransaction(bPendTrans);
	}
}

/////////////////////////////////////////////////////////////////////////////
char* CSipParty::GetTransactionTypeAsString(ESipTransactionType type)
{
	switch( type )
	{
		case kSipTransNone:    				{ return "No transaction"; }
		case kSipTransInviteNoSdpReq:   	{ return "Invite no SDP request"; }
		case kSipTransInviteWithSdpReq:  	{ return "Invite with SDP request"; }
		case kSipTransReInviteNoSdpReq:  	{ return "ReInvite no SDP request"; }
		case kSipTransReInviteWithSdpReq:	{ return "ReInvite with SDP request"; }
		case kSipTransInviteNoSdpInd:  		{ return "Invite no SDP indication"; }
		case kSipTransInviteWithSdpInd:  	{ return "Invite with SDP indication"; }
		case kSipTransReInviteNoSdpInd:  	{ return "ReInvite no SDP indication"; }
		case kSipTransReInviteWithSdpInd: 	{ return "ReInvite with SDP indication"; }
		case kSipTransRTCPVideoUpdateInd:   { return "RTCP video update Indication"; }
		case kSipTransInviteMrcWithSdpInd:  { return "Invite MRC with SDP indication"; }
		case kSipTransInviteMrcNoSdpInd: 	{ return "Invite MRC no SDP indication"; }
		case kSipTransInviteMrcSlaveWithSdpReq:{ return "Invite MRC Slave SDP request"; }
		case kSipTransUpgradeSvcOnlyToMixReq: { return "Upgrade Svc to mixed request"; }
		case kSipTransUpgradeAvcOnlyToMixReq: { return "Upgrade Avc to mixed request"; }
		case kSipTransRTCPVsrInd: 			  { return "RTCP VSR indication"; }
		case kSipTransInviteWebRtcWithSdpInd:  	{ return "Invite WebRtc with SDP indication"; }
		case kSipTransReInviteMrcWithSdpReq:  	{ return "ReInvite MRC with SDP request"; }  //FSN-613: Dynamic Content for SVC/Mix Conf


	}
	return "Unknown type";
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipParty::IsActiveTransaction() const
{
	if ((m_eActiveTransactionType != kSipTransNone) && (IsValidPObjectPtr(m_pSipTransaction)))
		return TRUE;
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipParty::IsNeedToPendNewTransaction() const
{
	if (IsActiveTransaction() && m_pSipTransaction && m_pSipTransaction->IsNeedToPendOtherTransaction())
		return TRUE;
	return FALSE ;

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyReceivedReInviteConnected(CSegment* pParam)
{
	if (IsGlareStatus())
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedReInviteConnected. Glare status is true");
		// Clear glare status:
		SetGlareStatus(FALSE);
		if (IsValidTimer(SIP_GLARE_TIMER))
			DeleteTimer(SIP_GLARE_TIMER);

		// If we are during conf change mode, we will use this ReInvite Ind for doing the change mode, so we can clear the change mode mask accordingly:
		if (m_bIsConfChangeMode)
			m_maskRequiredChangeMode = eChangeModeMask_None;
	}

	// we need to identify the two case here
	// 1. ReInvite with SDP.
	// 2. ReInvite without SDP.
	BYTE isReInviteWithSdp = YES;
	*pParam >> isReInviteWithSdp;

	//TODO (URI) - maybe to fix remote rate (ERICSSON MOBILE)

	ESipTransactionType eSipTransType;

	if(isReInviteWithSdp)
	{
		TRACEINTO << "case of Re-Invite with SDP (MrcCall: " << ( (m_pSipCntl->GetIsMrcCall()) ? "yes)" : "no)" );
		eSipTransType = kSipTransReInviteWithSdpInd;
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedReInviteConnected, case of Re-Invite WITHOUT SDP");
		eSipTransType = kSipTransReInviteNoSdpInd;
	}

	StartTransaction(eSipTransType, SIP_PARTY_RECEIVED_REINVITE, pParam);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipParty::HandleIceFallbacks()
{


	//FE-8327
	TRACEINTO << "Ice fallbacks";
	CConfParty* pConfParty = NULL;
	CCommConf* pCommConf 	= ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
	if (pCommConf)
	    pConfParty 	= pCommConf->GetCurrentParty(GetMonitorPartyId());
	DBGPASSERT_AND_RETURN(!(m_pSipCntl && m_pTargetMode && pConfParty));
	WORD reqMask 			= pConfParty->GetPlcmRequireMask();
	BOOL bVeqFlowEnableBFCP = (!reqMask ||
							  ((reqMask && (reqMask & m_plcmRequireBfcpUdp || reqMask & m_plcmRequireBfcpTcp))));
	BOOL bIcCCSPlugin 		= m_pSipCntl->GetIsCCSPlugin();
	BOOL bIsVideoOn   		= m_pTargetMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople);

	TRACEINTO << " bVeqFlowEnableBFCP " << (int)bVeqFlowEnableBFCP <<" bIcCCSPlugin " << (int)bIcCCSPlugin <<" bIsVideoOn " << (int)bIsVideoOn <<" reqMask " << (int)reqMask;
	if( ((bIsVideoOn && bVeqFlowEnableBFCP) || bIcCCSPlugin) && (CProcessBase::GetProcess()->GetProductType() != eProductTypeSoftMCUMfw))
	{
		PTRACE(eLevelInfoNormal,"CSipParty::HandleIceFallbacks : Adding bfcp on ICE fallback");
		m_pSipCntl->AddBfcpOnFallback(m_pTargetMode, m_pTargetModeMaxAllocation);
	}

	m_bIsRcvCheckCompleteAndNeedToStartIVR = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
ESipTransactionType CSipParty::IsNeedToSendReInvite(BOOL& iceChange)
{
	BYTE bRealloc 				= m_pSipTransaction->IsNeedReInviteForReAlloc();
	BYTE bSecondary 			= m_pSipTransaction->IsNeedReInviteForSecondary();
	iceChange 					= m_pSipTransaction->IsNeedReInviteForIce();
	BYTE bIsVideoFormatChange 	= m_pSipTransaction->IsVideoFormatChangeOnly();
	BOOL bNeedToSendReInviteWithFullAudioCaps = m_pSipTransaction->IsNeedToSendReInviteWithFullAudioCaps();
	BYTE bAddContent 			= m_pSipTransaction->IsNeedReInviteForAddContent();
	BYTE bIsNoneTip 			= m_pSipTransaction->IsNeedReInviteForNoneTipCall();
	BYTE bIsRemoveRTV 			= m_pSipTransaction->IsNeedReinviteForRemoveRtv();
	BYTE bIsBandwidth 			= m_pSipTransaction->IsNeedReInviteForBandwidth();
	BYTE bIsBfcp				= m_pSipTransaction->IsNeedReInviteForBfcp();
	BYTE bIsNeedToFixContentAlg = m_pSipTransaction->IsNeedReInviteToFixContentAlg();
	BYTE bIsSecondaryContent 	= m_pSipTransaction->IsNeedReInviteForSecondaryContent();
	BYTE bIsActiveMedai         = m_pSipTransaction->IsNeedReInviteToActiveMedia();
	ESipTransactionType eReInviteTransType = kSipTransNone;

	CMedString str;
	str << " \n for realloc 				= " << bRealloc
		<< ",\n for Secondary 				= " << bSecondary
		<< ",\n for ICE 					= " << iceChange
		<< ",\n for Full Audio Caps 		= " << bNeedToSendReInviteWithFullAudioCaps
		<< ",\n for Content 				= " << bAddContent
		<< ",\n for change of video format  = " << bIsVideoFormatChange
		<< ",\n for Remove RTV 			= " << bIsRemoveRTV
		<< ",\n for None TIP 				= " << bIsNoneTip
		<< ",\n for Bandwidth 				= " << bIsBandwidth
		<< ",\n for BFCP 					= " << bIsBfcp
		<< ",\n to fix content alg 		= " << bIsNeedToFixContentAlg
		<< ",\n for secondary content 		= " << bIsSecondaryContent;

	PTRACE2(eLevelInfoNormal,"CSipParty::IsNeedToSendReInvite : ",str.GetString());
/*
	if(bSecondary || bIce ||bRealloc)
		return TRUE;
	else
		return FALSE;

*/
	if (m_IsPartialConnectionForVSW == TRUE)
	{
		m_IsPartialConnectionForVSW = FALSE;
		StartIvr();
	}

	if(bSecondary || iceChange || bIsRemoveRTV ||(bRealloc && !bIsVideoFormatChange)|| bNeedToSendReInviteWithFullAudioCaps || bAddContent || bIsBandwidth || bIsBfcp || bIsNeedToFixContentAlg || bIsSecondaryContent)
	{
/*	Angelina - no difference between SVC and AVC in respect of re-invite
 * 		//FSN-613: Dynamic Content for SVC/Mix Conf
		 if(m_pSipCntl->GetIsMrcCall())
			eReInviteTransType = kSipTransReInviteMrcWithSdpReq;
		else*/
			eReInviteTransType = kSipTransReInviteWithSdpReq;
	}

	else if(bRealloc && bIsVideoFormatChange)
	{
		OnPartyCntlStartVideoPreference(NULL);
	}
	return eReInviteTransType;
}

BYTE CSipParty::IsNeedToSendReInviteAfterTIPFallback()
{
	BYTE bRealloc 				= m_pSipTransaction->IsNeedReInviteForReAlloc();
	BYTE bSecondary 			= m_pSipTransaction->IsNeedReInviteForSecondary();
	BYTE bAddContent 			= m_pSipTransaction->IsNeedReInviteForAddContent();
	BYTE bIsBfcp				= m_pSipTransaction->IsNeedReInviteForBfcp();
	BYTE bIsSecondaryContent 	= m_pSipTransaction->IsNeedReInviteForSecondaryContent();

	CMedString str;
	str << " \n for realloc 				= " << bRealloc
		<< ",\n for Secondary 				= " << bSecondary
		<< ",\n for Content 				= " << bAddContent
		<< ",\n for BFCP 					= " << bIsBfcp
		<< ",\n for secondary content 		= " << bIsSecondaryContent;

	PTRACE2(eLevelInfoNormal,"CSipParty::IsNeedToSendReInviteAfterTIPFallback : ",str.GetString());

	if(bSecondary || bRealloc || bAddContent || bIsBfcp || bIsSecondaryContent)
		return TRUE;

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
/////             Sip Transaction handling messages functions              //////
/////////////////////////////////////////////////////////////////////////////


void CSipParty::SetIsTipNegotiationActive(BYTE bIsActive)
{

	if(!m_bHandlingTipNegotiationResult && bIsActive)
	{
		//(Start new TIP negotiation)
		m_bTipEndSuccessSent  = FALSE;
		m_bTipLastAckReceived = FALSE;
		m_bIsTipResumed		  = FALSE;
	}

	m_bHandlingTipNegotiationResult = bIsActive;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTransEndTransactionAnycase(CSegment* pParam)
{
	DWORD retStatus = STATUS_OK;
	*pParam >> (DWORD&)retStatus;

	TRACEINTO << "Status:" << retStatus;

	DBGPASSERT(!IsActiveTransaction());
	BYTE bIsEndedTransOfferer = m_pSipTransaction->IsOfferer();
	BYTE bIsEndedTransReInvite = m_pSipTransaction->IsReInvite();
	EPendingTransType bPendTrans;
	ESipTransactionType eTransType = m_eActiveTransactionType;
	m_pSipCntl->SetStateOnEndOfTransaction();
	EndTransaction(bPendTrans);

	if (m_pTargetMode->GetIsTipMode() == FALSE && m_bIsTipCall)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Set to non tip noa temp");
		m_bIsTipCall = FALSE;
	}
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string key = "REPEAT_INTRA_NUMBER";
	pSysConfig->GetDWORDDataByKey(key, m_repeatIntraNumber);
	CapEnum algorithm = (CapEnum)(m_pCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit));
	if (algorithm == eMsSvcCapCode)
		m_repeatIntraNumber = 0;
	//update here

	if (retStatus == STATUS_OK)
	{
		//In case of SRTP or MOC will send intra in end of transaction
		if ((m_pCurrentMode->GetIsEncrypted() == Encryp_On && GetIsTipCall() == FALSE) || (
				m_pSipCntl->IsRemoteMicrosoft() ||
				m_pSipCntl->IsSameTimeEP() ||
				m_pSipCntl->GetRemoteIdent() == IbmSametimeEp_Legacy))
		{
			PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - SRTP or MOC, will send Intra");
			if (algorithm != eMsSvcCapCode)
				StartTimer(SIP_SEND_INTRA_AFTER_TRANS_TOUT, 2 * SECOND);
			SetSendIntraOnEndOfTransaction(FALSE);
		}

		if (bIsEndedTransReInvite) // End of ReInvite
		{
			BYTE bIsEndConfChangeMode = FALSE;
			// check existence of conf change modes only (not flow control)
			if (m_bIsConfChangeMode && !(m_maskRequiredChangeMode & eChangeModeMask_Incoming) && !(m_maskRequiredChangeMode & eChangeModeMask_ContentChangeRate) && !(m_maskRequiredChangeMode & eChangeModeMask_ContentChangeProtocol)) // When eChangeModeMask_Incoming is Off, it means that we have finished change incoming mode. If it is On it means that we haven't started the change mode transaction yet.
			{
				bIsEndConfChangeMode = TRUE;
				m_bIsConfChangeMode = FALSE;
			}
			PTRACE2INT(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - IsEndConfChangeMode:", bIsEndConfChangeMode);
			m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pCurrentMode, bIsEndConfChangeMode);
			BYTE isVidoOutMuted = FALSE;
			CSipCall *pCall = m_pSipCntl->GetCallObj();
			CSipChannel* videoOutCh = pCall->GetChannel(VIDEO_OUT);
			if (videoOutCh && videoOutCh->IsMuted())
			{
				PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - OUT VIDEO MUTED");
				isVidoOutMuted = TRUE;

			}
			if (GetIsTipCall() && m_pCurrentMode && isVidoOutMuted && m_pSipCntl->GetIsNeedToReleaseTokenForHoldTip())
			{
				PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Release the Token in HOLD");
				m_pSipCntl->SetIsNeedToReleaseTokenForHoldTip(FALSE);

				// Updating the stream status
				CSegment * pSeg = new CSegment;
				*pSeg << (DWORD)PARTY_TOKEN_RELEASE;
				*pSeg << (DWORD)STATUS_OK;
				*pSeg << ((BYTE)(GetMcuNum()));
				*pSeg << ((BYTE)(GetTerminalNum()));
				OnTipCntlContentMsgInd(pSeg);
			}

			if (((eTransType == kSipTransReInviteNoSdpInd) || (eTransType == kSipTransReInviteWithSdpInd)) && GetIsTipCall() && m_pSipTransaction->GetIsTipResumeMedia())
			{
				if (m_pSipTransaction->IsNeedToDropCallForLowBitRateForTip())
					TellConfOnDisconnecting(TIP_VIDEO_BIT_RATE_TOO_LOW);
				else
				{
					//MuteMediaIfNeeded(cmCapTransmit, TRUE);

					if ((bPendTrans != etransReinvite) || ((bPendTrans == etransReinvite) && !GetTipPartyOnHold()))
					{
						m_pSipCntl->CloseTipSessionIfNeeded();

						PTRACE2INT(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Resume:", m_pSipTransaction->GetIsTipResumeMedia());

						m_pSipTransaction->SetIsTipResumeMedia(FALSE);
						SetIsTipNegotiationActive(TRUE);
						m_pSipCntl->StartTipNegotiation();
					}
				}
			}
			if (m_bIsNeedToStartIVRAfterFallbackFromTip)    //Inorder to start IVR we must wait for TIP to SIP fall back to end (video channels should open).
			{
				StartIvr();
				m_bIsNeedToStartIVRAfterFallbackFromTip = FALSE;
			}
			//only if we need to remove content -> send reinvite
			if (bPendTrans == eNoPendTrans && m_pSipTransaction->IsNeedReInviteForSecondaryContent())
			{
				PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Reinvite is ended, need to send Reinvite to remove content");
				m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
				m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
				m_pSipCntl->RemoveBfcpAndContentCaps();
				m_maskRequiredChangeMode = eChangeModeMask_None;
				*m_pUpdateTargetMode = *m_pTargetMode;
			}

			if (eTransType == kSipTransReInviteWithSdpReq)
			{
				if (m_pSipTransaction->IsNeedReInviteForBfcp())
				{
					PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Reinvite is ended, need to send Reinvite to change BFCP");
					HandleBfcpScmAndCaps();
				}

				if (m_pSipTransaction->IsNeedReInviteForAddContent())
				{
					PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Reinvite is ended, need to send Reinvite to add content");
					HandleContentScmAndCaps();
				}

				if (m_pSipTransaction->IsNeedToToCloseBfcpAndContentWithoutReinvite())
				{
					PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Reinvite is ended, need to remove BFCP and content without reinvite");

					m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
					m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
					m_pSipCntl->RemoveBfcpAndContentCaps();
				}
			}
		}
		else // End of Invite
		{
			if (!bIsEndedTransOfferer)
				m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pCurrentMode, FALSE);

			BYTE isInitialConnection = NO;
			if (kSipTransInviteNoSdpReq == eTransType ||
					kSipTransInviteWithSdpReq == eTransType ||
					kSipTransInviteMrcSlaveWithSdpReq == eTransType ||
					kSipTransInviteNoSdpInd == eTransType ||
					kSipTransInviteWithSdpInd == eTransType ||
					kSipTransInviteMrcWithSdpInd == eTransType ||
					kSipTransInviteMrcNoSdpInd == eTransType)
			{
				isInitialConnection = YES;
			}
			ConnectCall(isInitialConnection);

			if ((bIsEndedTransOfferer || eTransType == kSipTransInviteWithSdpReq || eTransType == kSipTransInviteWithSdpInd) && GetIsTipCall())
			{
				if (m_pSipTransaction->IsNeedToDropCallForLowBitRateForTip())
					TellConfOnDisconnecting(TIP_VIDEO_BIT_RATE_TOO_LOW);
				else if (!GetIsTipNegotiationActive())
				{
					SetIsTipNegotiationActive(TRUE);
					m_pSipCntl->StartTipNegotiation();
				}
				else
					PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - TIP negotiation was started");
			}

			if (IsNeedToSendRtcpVideoPreference())
			{
				PTRACE(eLevelInfoNormal, "CCSipParty::OnTransEndTransactionAnycase - Start Timer SIP_SEND_VIDEO_PREFERENCE_END_TRANS_TOUT for 1 second");
				StartTimer(SIP_SEND_VIDEO_PREFERENCE_END_TRANS_TOUT, 1 * SECOND);
			}

			if (bIsEndedTransOfferer)
			{
				if (m_pSipTransaction->IsNeedReInviteForBfcp())
				{
					PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Invite is ended, need to send Reinvite to change BFCP");
					HandleBfcpScmAndCaps();
				}

				if (m_pSipTransaction->IsNeedReInviteForAddContent())
				{
					PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Invite is ended, need to send Reinvite to add content");
					HandleContentScmAndCaps();
				}

				if (m_pSipTransaction->IsNeedReInviteForSecondaryContent())
				{
					PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Invite is ended, need to send Reinvite to remove content");

					m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
					m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
					m_pSipCntl->RemoveBfcpAndContentCaps();
					m_maskRequiredChangeMode = eChangeModeMask_None;
					*m_pUpdateTargetMode = *m_pTargetMode;
				}

				if (m_pSipTransaction->IsNeedToToCloseBfcpAndContentWithoutReinvite())
				{
					PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Invite is ended, need to remove BFCP and content without reinvite");

					m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
					m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
					m_pSipCntl->RemoveBfcpAndContentCaps();
				}
			}
		}

		if (GetDialState() == kTerminateByConf)
		{
			if (bIsEndedTransReInvite)
				m_pSipCntl->CloseCall(YES);
		}
		else
		{
			SetDialState(kNotInDialState);
			if (GetSendIntraOnEndOfTransaction())
			{
				if (algorithm != eMsSvcCapCode)
					StartTimer(SIP_SEND_INTRA_AFTER_TRANS_TOUT, 1 * SECOND);
				SetSendIntraOnEndOfTransaction(FALSE);
			}
			// Check if content protocol or rate was changed by conf as part of transaction flow:
			if (bIsEndedTransOfferer && m_pSipTransaction->IsContentProtocolWasChanged())
			{
				PTRACE2INT(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Content protocol was changed during transaction, Rate:", m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePresentation));
				m_pUpdateTargetMode->CopyMediaMode(*m_pTargetMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation); // ***ppc to check
				AddToChangeModeMask(eChangeModeMask_ContentChangeProtocol); // ***ppc to check
			}
			else if (m_pSipTransaction->IsContentRateWasChanged()) // content rate was changed by conf as part of transaction flow
			{
				PTRACE2INT(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Content Rate was changed during transaction, Rate:", m_pTargetMode->GetContentBitRate(cmCapReceive));
				m_pUpdateTargetMode->SetContentBitRate(m_pTargetMode->GetContentBitRate(cmCapReceive), cmCapReceiveAndTransmit); // ***ppc to check
				AddToChangeModeMask(eChangeModeMask_ContentChangeRate); // ***ppc to check
			}

			BYTE bSendReInvite = FALSE;
			ESipTransactionType needToActivateSubsequentTransection = kSipTransNone;
			if (bPendTrans == eNoPendTrans && !((eTransType == kSipTransInviteWithSdpReq || eTransType == kSipTransInviteNoSdpInd || eTransType == kSipTransInviteWithSdpInd) && GetIsTipCall()))
			{
				//bSendReInvite = SendReInviteAfterEndTransactionIfNeeded();
				needToActivateSubsequentTransection = SendReInviteAfterEndTransactionIfNeeded();
			}
			else
			{
				PTRACE(eLevelInfoNormal, "CSipParty::OnTransEndTransactionAnycase - Do not send Reinvite there is pending trans");
				needToActivateSubsequentTransection = kSipTransNone;
				bSendReInvite = FALSE;
			}
			TRACEINTO << "NeedToActivateSubsequentTransection:" << (int)needToActivateSubsequentTransection;
			if (needToActivateSubsequentTransection == kSipTransNone)
			{
				m_pSipCntl->SendUpdateChannelForDiffPayloadIfNeeded((CSipComMode*)m_pTargetMode);
			}

			if (m_pSipCntl->isMs2013Active())
			{
				this->HandleMsftVsrOnEndTransaction(needToActivateSubsequentTransection, eTransType);
			}
		}
	}
	else // Failure Status
	{
		PTRACE2INT(eLevelError, "CSipParty::OnTransEndTransactionAnycase - Status:", retStatus);
		if (retStatus == SIP_CLIENT_ERROR_491)
		{
			if (GetDialState() == kTerminateByConf)
				m_pSipCntl->CloseCall(YES);
			else
				HandleGlareStatus();
		}
		else
			TellConfOnDisconnecting(retStatus); // Disconnect the call. Disconnection cause is the retStatus:
	}
}

/////////////////////////////////////////////////
void  CSipParty::HandleMsftVsrOnEndTransaction(bool isNeedToSendReinvite,ESipTransactionType endedTransType)
{
	PTRACE(eLevelInfoNormal,"CSipParty::HandleMsftVsrOnEndTransaction");

	const CSipCaps* pRemoteCaps	= m_pSipCntl->GetLastRemoteCaps();
	DWORD videoSsrc = pRemoteCaps->getMsftSsrcVideoFirst(1);
	bool is_audio_only =  ((0 == videoSsrc || VSR_SOURCE_NONE == videoSsrc));
	bool is_avmcu2013 = Microsoft_AV_MCU2013 == m_pSipCntl->GetRemoteIdent();

    bool is_MuteChannelIn = false;
    CSipCall *pCall = m_pSipCntl->GetCallObj();
    CSipChannel* videoInCh =  pCall->GetChannel(VIDEO_IN);
    if(videoInCh && videoInCh->IsMuted())
    {
		is_MuteChannelIn = true;
		PTRACE(eLevelInfoNormal,"CSipParty::EndTransaction: do not send vsr as in channel is muted" );
    }


	if(!is_audio_only &&
	  ((endedTransType == kSipTransInviteNoSdpReq) || (endedTransType == kSipTransInviteWithSdpReq) ||
	  (endedTransType == kSipTransInviteNoSdpInd) || (endedTransType == kSipTransInviteWithSdpInd)))
	{
		m_pSipCntl->initVsrCtrl();
	}
	else if (!is_audio_only &&
		((endedTransType == kSipTransReInviteWithSdpReq) || (endedTransType == kSipTransReInviteWithSdpInd)))
	{

		if (!m_pSipCntl->isVsrCtrlInitialized())
		{
			m_pSipCntl->initVsrCtrl();
		}

        if(!is_avmcu2013 && !is_MuteChannelIn)
        {
        	if(m_pSipCntl->isMs2013Active() == eMsft2013AvMCU)
        	{
        		ST_VSR_MUTILPLE_STREAMS  vsrs;
        		memset(&vsrs,0,sizeof(ST_VSR_MUTILPLE_STREAMS));
        		vsrs.num_vsrs_streams = MAX_STREAM_LYNC_2013_CONN;
        		for (DWORD i = 0; i < min(vsrs.num_vsrs_streams, MAX_STREAM_LYNC_2013_CONN); ++i)
        		{
        			ST_VSR_SINGLE_STREAM& vsr1 = vsrs.st_vsrs_single_stream[i];
        			vsr1.num_vsrs_params	= 0;
        			vsr1.msi = VSR_SOURCE_NONE;
        			vsr1.sender_ssrc	= m_pSipCntl->GetLocalCaps()->getMsftSsrcVideoFirst(i+1);
        		}
        		m_pSipCntl->SendMultiVsr(vsrs);
        	}
        	else
        		m_pSipCntl->SendSingleVsr();
        }

		if (m_bMsftRecevVsrInActiveTrans && !isNeedToSendReinvite)
		{
			m_pSipCntl->trigerMsftRcvVsr();
			m_bMsftRecevVsrInActiveTrans = FALSE;

		}
	}

	//AVMCU2013- notify sparty control
	if (is_avmcu2013 && (endedTransType == kSipTransInviteWithSdpReq))
	{
		m_pConfApi->SendAvmcu2013Detected(GetPartyId());
	}
	if (endedTransType == kSipTransRTCPVsrInd && m_pSipCntl->isMs2013Active() == eMsft2013AvMCU)
	{
		// if there is a saved VSR then start a transaction using this new VSR.
		//if (m_bMsftSlaveRecevVsrInActiveTrans)
		if (m_pLastTxVsr)
		{
			TRACEINTO<<"Resend VsrMsgInd";
			CSegment* pParam = new CSegment;
			pParam->Put(reinterpret_cast<const BYTE*>(m_pLastTxVsr), sizeof(ST_VSR_SINGLE_STREAM));

			ESipTransactionType eSipTransType = kSipTransRTCPVsrInd;
			StartTransaction(eSipTransType, SIP_TRANS_VSR_MSG_IND, pParam);
			POBJDELETE(m_pLastTxVsr);
			m_pLastTxVsr = NULL;
		}
	}
}


/////////////////////////////////////////////////////////////////////////////////
ESipTransactionType  CSipParty::SendReInviteAfterEndTransactionIfNeeded()
{
	PTRACE(eLevelInfoNormal,"CSipParty::SendReInviteAfterEndTransactionIfNeeded");
	ESipTransactionType needToActivateSubsequentTransection;
	ESipTransactionType finalDecision=kSipTransNone;
	// Check if need to send ReInvite:
//	BYTE bNeedToSendReInvite = IsNeedToSendReInvite(); // Reasons: Set Secondary or Send new caps after Realloc
	BYTE iceChange;
	needToActivateSubsequentTransection = IsNeedToSendReInvite(iceChange);

	// Handle any ICE fall back preparations if required
	if (iceChange && !m_pSipCntl -> GetIsIceCall())
		HandleIceFallbacks();

	// check if in the middle of this transaction we received an update from conference
	if (m_maskRequiredChangeMode != eChangeModeMask_None) //we did receive
	{
		BYTE bSendFlowControl = needToActivateSubsequentTransection/*bNeedToSendReInvite*/ ? FALSE : TRUE; // if we anyway send Reinvite, don't send separately the flow control.
//		bNeedToSendReInvite |= UniteConfAndTransModes(bSendFlowControl); //compare m_pUpdateTargetMode and m_TargetMode in order to create another action
		if(needToActivateSubsequentTransection==kSipTransNone)
		{
			needToActivateSubsequentTransection =(ESipTransactionType)(UniteConfAndTransModes(bSendFlowControl));
		}
	}

	// At end of transaction, after updating the active target mode according to Updated mode, the Updated mode is reset to the active mode.
	m_maskRequiredChangeMode 	= eChangeModeMask_None;
	*m_pUpdateTargetMode 		= *m_pTargetMode;

	if ( m_pSipTransaction->IsNeedReInviteForNoneTipCall() )
	{
	    PTRACE(eLevelInfoNormal,"CSipParty::SendReInviteAfterEndTransactionIfNeeded - TIP_WAIT_FALLBACK_TOUT");
	    StartTimer(TIP_WAIT_FALLBACK_TOUT,SECOND*3);
	    if ( IsNeedToSendReInviteAfterTIPFallback() )
	    	m_isNeedToSendReInviteAfterTIPFallBack = TRUE;
	    return kSipTransNone;
		//return FALSE; //bNeedToSendReInvite;
	}

	if ( m_pSipTransaction->IsNeedReInviteForAddContent() )
	{
		CSipCall* currCall = m_pSipCntl->GetCallObj();
		if(currCall && currCall->IsCallInitiator() == NO) // If we are the in Dial In, wait to get Re-Invite with content
		{
			StartTimer(CONTENT_WAIT_REINVITE_TOUT,SECOND*1);
		    return kSipTransNone;
		}
	}

	if (m_pSipCntl->isMs2013Active() && Microsoft_AV_MCU2013 == m_pSipCntl->GetRemoteIdent())
	{
	    PTRACE(eLevelInfoNormal,"CSipParty::SendReInviteAfterEndTransactionIfNeeded - AVMCU2013 no reinvite");
	    return kSipTransNone;
	}

	// Send ReInvite if needed:
/*	if (bNeedToSendReInvite)
		StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);*/
	OPCODE opcode;
	if(needToActivateSubsequentTransection ==kSipTransReInviteWithSdpReq ||  needToActivateSubsequentTransection ==kSipTransReInviteMrcWithSdpReq) //FSN-613: Dynamic Content for SVC/Mix Conf
	{
		finalDecision=needToActivateSubsequentTransection;
		opcode= SIP_PARTY_SEND_REINVITE;
	}
	else if(needToActivateSubsequentTransection ==kSipTransUpgradeSvcOnlyToMixReq)
	{
		finalDecision=kSipTransUpgradeSvcOnlyToMixReq;
		opcode= PARTY_UPGRADE_TO_MIXED;
	}
	else if(needToActivateSubsequentTransection ==kSipTransUpgradeAvcOnlyToMixReq)
	{
		finalDecision=	kSipTransUpgradeAvcOnlyToMixReq;
		opcode= PARTY_UPGRADE_TO_MIXED;
	}
	if(finalDecision)
	{
		StartTransaction(finalDecision,opcode, m_pUpgradeParams);
        POBJDELETE(m_pUpgradeParams);
	}


	return finalDecision;
}
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransMuteMediaAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransMuteMediaAnycase");

	DWORD eDirection;
	*pParam >> eDirection;
//	*pParam >> bMuteAudioIn >> bMuteAudioOut >> bMuteVideoIn >> bMuteVideoOut
//			>> bMuteContentIn >> bMuteContentOut >> bMuteFeccIn >> bMuteFeccOut;

	BOOL bIsForceMuteForTip = FALSE;

	if ( (m_pTargetMode->GetIsTipMode()== TRUE) && (m_pSipTransaction->GetIsMuteForTip()) )
	{
		bIsForceMuteForTip = TRUE;
	}

	//if(eDirection == cmCapTransmit) !!!!!!!! Michael V.
  if( eDirection == cmCapTransmit )
    m_pSipTransaction->SetIsMuteForTip(FALSE);

	MuteMediaIfNeeded((cmCapDirection)eDirection,bIsForceMuteForTip);

//	m_pConfApi->IpMuteMedia(this, bMuteAudioIn,bMuteAudioOut,bMuteVideoIn,bMuteVideoOut,bMuteContentIn,bMuteContentOut,bMuteFeccIn,bMuteFeccOut);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransReceiveReCapAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransReceiveReCapAnycase");

	CSipCaps* pCurRemoteCaps = new CSipCaps;
	CIpComMode* pBestMode = new CIpComMode;

	pCurRemoteCaps->DeSerialize(NATIVE,*pParam);
	pBestMode->DeSerialize(NATIVE,*pParam);
	BYTE bAtomicAction = HandleAtomicAction(pCurRemoteCaps,pBestMode);
	WORD bIsFallBack = FALSE;
	WORD bIsGlare = FALSE;
	*pParam >> bIsFallBack;
	*pParam >> bIsGlare; //BRIDGE-12961
	TRACEINTO << " bIsFallBack " << (int)bIsFallBack << " bIsGlare " << (int)bIsGlare; //BRIDGE-12961 bIsGlare
	if(!bAtomicAction || bIsGlare)//this was not handled as an atomic action(flow control) //BRIDGE-12961 bIsGlare
		m_pConfApi->SendPartyReceiveReCapsToPartyControl(GetPartyId(), pCurRemoteCaps, pBestMode,bIsFallBack);

	// TIP
	if (GetIsTipCall() && m_tipPartyType == eTipMasterCenter && m_pTargetMode->IsTipNegotiated())
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnTransReceiveReCapAnycase, aux slave");

        const CCommConf*  pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
        if (pCommCurrConf && !pCommCurrConf->GetEntryQ())
        {
		// transaction need to wait before sending message for all slaves to finish change mode
		m_pSipTransaction->SetIsNeedToWaitForSlavesEndChangeMode(TRUE);

		StartTimer(TIP_SLAVE_RECAP_TOUT,SECOND*30);
        }

		CSegment *pSeg = new CSegment;

		pCurRemoteCaps->Serialize(NATIVE,*pSeg);
		pBestMode->Serialize(NATIVE,*pSeg);

		// init ack counter before sending recap to slaves
		m_nSlavesReturnRecapAck = 1;

		WORD numOfSlaves = m_TipNumOfStreams -1 + m_bIsAudioAux;

		if (numOfSlaves > 1)
			m_nSlavesReturnRecapAck += 2;

		SendMessageFromMasterToSlave(eTipSlaveAux, TIP_REMOTE_SENT_RE_CAPS_FOR_SLAVE, pSeg);

		if (numOfSlaves > 1)
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnTransReceiveReCapAnycase, right + left slaves");

			SendMessageFromMasterToSlave(eTipSlaveLeft, TIP_REMOTE_SENT_RE_CAPS_FOR_SLAVE, pSeg);
			SendMessageFromMasterToSlave(eTipSlaveRigth, TIP_REMOTE_SENT_RE_CAPS_FOR_SLAVE, pSeg);
		}

		POBJDELETE(pSeg);
	}

	POBJDELETE(pCurRemoteCaps);
	POBJDELETE(pBestMode);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTransUpdateDbAnycase(CSegment* pParam)
{
	DWORD partyState = PARTY_IDLE;
	*pParam >> partyState;

	TRACEINTO << "partyState: " << partyState;

	m_pConfApi->UpdateDB(this,PARTYSTATE,PARTY_CONNECTING);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTransDisconnectBridgesAnycase(CSegment* pParam)
{
	WORD isDisconnectAudio = TRUE;
	WORD isDisconnectVideo = TRUE;
	*pParam >> isDisconnectAudio >> isDisconnectVideo;

	TRACEINTO << "isDisconnectAudio:"   << (isDisconnectAudio ? "yes" : "no")
	          << ", isDisconnectVideo:" << (isDisconnectVideo ? "yes" : "no");

	m_pConfApi->SendDisconnectBridgesToPartyControl(GetPartyId(), isDisconnectAudio, isDisconnectVideo);

// m_pConfApi->UpdateDB(this,MUTE_STATE, 0xF0000011, 1);	// indicate party is audio (0x00000001) and video (0x00000010) self muted
}

/////////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTransConnectBridgesAnycase(CSegment* pParam)
{
	WORD isConnectAudio = TRUE;
	WORD isConnectVideo = TRUE;

	unsigned int incomingVideoChannelHandle = 0;
	unsigned int outgoingVideoChannelHandle = 0;

	*pParam >> isConnectAudio >> isConnectVideo;

	CIpComMode* pNewMode  = new CIpComMode;
	pNewMode->DeSerialize(NATIVE, *pParam);

	CSipCaps* pRemoteCaps = new CSipCaps;
	pRemoteCaps->DeSerialize(NATIVE, *pParam);

	*pParam >> incomingVideoChannelHandle;
	*pParam >> outgoingVideoChannelHandle;


	TRACEINTO << "isConnectAudio:" << (isConnectAudio ? "yes" : "no")
	          << ", isConnectVideo:" << (isConnectVideo ? "yes" : "no")
	          << ", incomingVideoChannelHandle:" << incomingVideoChannelHandle
	          << ", outgoingVideoChannelHandle:" << outgoingVideoChannelHandle;

	pNewMode->Dump("pNewMode", eLevelInfoNormal);

	m_pConfApi->SendConnectBridgesToPartyControl(GetPartyId(),
												  isConnectAudio, isConnectVideo,
												  pNewMode, pRemoteCaps,
												  incomingVideoChannelHandle, outgoingVideoChannelHandle );

	POBJDELETE(pNewMode);
}

////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTransSendTokenReleaseForHoldAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipParty::OnTransSendTokenReleaseForHoldAnycase");
	m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_RELEASE,this, GetMcuNum(), GetTerminalNum(), LABEL_CONTENT);
}

/////////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransUptateChannelsStatusAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransUptateChannelsStatusAnycase");
	BYTE bIsConnected;
	*pParam >> bIsConnected;

	if (bIsConnected)						// channels connect
		UpdateDbOnChannelsConnected();
	else  									// channels disconneced
		UpdateDbOnChannelsDisconnected();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransChannelsConnectedAnycase(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipParty::OnTransChannelsConnectedAnycase tipmode ", m_pCurrentMode->GetIsTipMode());

	unsigned int channelHandle[2];
	channelHandle[0] = INVALID_CHANNEL_HANDLE;
	channelHandle[1] = INVALID_CHANNEL_HANDLE;

	CSipChannel *pChannelIn=NULL;
	CSipChannel *pChannelOut=NULL;

	if ((m_pCurrentMode->GetConfMediaType()==eMixAvcSvcVsw) && !m_pSipCntl->GetIsMrcCall())
	{
		pChannelIn = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapReceive, kRolePeople);
		pChannelOut = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapTransmit, kRolePeople);
	}
	else if ((GetTargetMode()->GetConfMediaType() == eMixAvcSvc) && !m_pSipCntl->GetIsMrcCall())
	{
		pChannelIn = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapReceive, kRolePeople);
	}
	else
	{
		pChannelIn = m_pSipCntl->GetChannel(cmCapVideo, cmCapReceive, kRolePeople);
		pChannelOut = m_pSipCntl->GetChannel(cmCapVideo, cmCapTransmit, kRolePeople);
	}

	if (pChannelIn)
	{
		channelHandle[0]=pChannelIn->GetChannelHandle();
		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnTransChannelsConnectedAnycase channelHandle[0] ",channelHandle[0]);
	}

	if (pChannelOut)
	{
		channelHandle[1]=pChannelOut->GetChannelHandle();
		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnTransChannelsConnectedAnycase channelHandle[1] ",channelHandle[1]);
	}

	m_pConfApi->SipPartyChannelsConnected(GetPartyId(), m_pCurrentMode, channelHandle);

	if (!m_pSipCntl->GetIsLprModeOn())
	{
		DWORD rate = m_pCurrentMode->GetMediaBitRate(cmCapVideo,cmCapReceive , (ERoleLabel)kRolePeople);
		m_pSipCntl->SetLastRateBeforeLpr(rate);
	}

	UpdateDbOnChannelsConnected();

	FirFilterCreate();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransRemoteCapsReceivedAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransRemoteCapsReceivedAnycase");

	mcTransportAddress localIp;
	memset(&localIp,0,sizeof(mcTransportAddress));
	mcTransportAddress remoteIp;
	memset(&remoteIp,0,sizeof(mcTransportAddress));

	if(m_bIsRcvCheckCompleteAndNeedToStartIVR)
		m_pSipCntl->GetOriginalRmtIpAddress(cmCapAudio,remoteIp);
	else
		m_pSipCntl->GetRemoteMediaIpAsTrAddr(cmCapAudio,kRolePeople,remoteIp); // take the audio ip as default

	m_pSipCntl->GetLocalMediaIpAsTrAddr(localIp);

	localIp.port = 5060;
	remoteIp.port = 5060;
	DWORD actualRate=0xFFFFFFFF;

	SetPartyMonitorBaseParamsAndConnectChannel(SDP,actualRate,&remoteIp,&localIp);

	//PDELETEA(connectType);

	const CSipCaps* pRemoteCaps	= m_pSipCntl->GetLastRemoteCaps();
	//update RemoteCaps member with remote side capabilities
	m_pConfApi->SipPartyRemoteCapsRecieved(GetPartyId(),const_cast<CSipCaps*>(pRemoteCaps),m_pCurrentMode);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransRemoteConnectedAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransRemoteConnectedAnycase");

	InformConfRemoteConnect();
}
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransSendChannelHandle(CSegment* pParam)
{
	unsigned int incomingVideoChannelHandle = INVALID_CHANNEL_HANDLE;
	unsigned int outgoingVideoChannelHandle = INVALID_CHANNEL_HANDLE;

	CSipChannel *pChannelIn = NULL;
	CSipChannel *pChannelOut = NULL;

	if((m_pCurrentMode->GetConfMediaType()==eMixAvcSvcVsw) && !m_pSipCntl->GetIsMrcCall())
	{
		pChannelIn = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapReceive, kRolePeople);
		pChannelOut = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapTransmit, kRolePeople);
	}
	else if((GetTargetMode()->GetConfMediaType() == eMixAvcSvc) && !m_pSipCntl->GetIsMrcCall())
    {
        pChannelIn = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapReceive, kRolePeople);
	}
	else
	{
		pChannelIn = m_pSipCntl->GetChannel(cmCapVideo, cmCapReceive, kRolePeople);
		pChannelOut = m_pSipCntl->GetChannel(cmCapVideo, cmCapTransmit, kRolePeople);
	}

	if (pChannelIn)
		incomingVideoChannelHandle=pChannelIn->GetChannelHandle();


	if (pChannelOut)
		outgoingVideoChannelHandle=pChannelOut->GetChannelHandle();

	TRACEINTO <<  "incomingVideoChannelHandle = " << incomingVideoChannelHandle << ", outgoingVideoChannelHandle = " << outgoingVideoChannelHandle;
	m_pConfApi->SipPartySendChannelHandle(GetPartyId(), incomingVideoChannelHandle, outgoingVideoChannelHandle);

}
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransOriginalRemoteCapsAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransOriginalRemoteCapsAnycase");
	CSipCaps* pRemoteCaps = new CSipCaps;

	if (pRemoteCaps)
	{
		pRemoteCaps->DeSerialize(NATIVE, *pParam);
		CSegment remoteCapsSeg;
		pRemoteCaps->SerializeCapArrayOnly(remoteCapsSeg, YES);
		m_pConfApi->UpdateDB(this, RMOT323CAP, (DWORD) 0, 1, &remoteCapsSeg);
	}
	else
		PASSERTMSG(1, "new pRemoteCaps failed");

	POBJDELETE(pRemoteCaps);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransSiteVisualNameAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransSiteVisualNameAnycase");
	SendSiteAndVisualNamePlusProductIdToPartyControl(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransUpdateEncStatusAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransUpdateEncStatusAnycase");
	OnUpdateEncryptionState(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendFlowControlMessage(APIU32 videoType, cmCapDirection mediaDirection,APIU32 rate)
{
	ERoleLabel 		eRole = (videoType == mainType) ? kRolePeople: kRolePresentation;
	BYTE 			bSendFlowReinvite = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_FLOW_CONTROL_REINVITE);
	DWORD newRateToSend = rate;
	CSipChannel 	*pChannel;
	CBaseVideoCap	*pCap;

	PTRACE2INT(eLevelError,"CSipParty::SendFlowControlMessage: role = ",eRole);
	if(m_state == PARTYDISCONNECTING)
	{
		PTRACE(eLevelError,"CSipParty::SendFlowControlMessage: Party is disconnecting ");
		return ;
	}

	/* Determine the new rate */
	if (eRole == kRolePeople)
	{
	      newRateToSend = rate ? rate : m_lastConstraintRate;
	      if(m_lastConstraintRate != rate )
	    	  m_lastConstraintRate = rate;
	}

	/* set the rate for people or presentation */
	if (eRole == kRolePeople)
	{
		m_pTargetMode->SetFlowControlRateConstraint(newRateToSend);
	}
	else if (eRole == kRolePresentation)
	{
			m_pTargetMode->SetFlowControlRateConstraintForPresentation(newRateToSend);
	}

	pChannel = m_pSipCntl->GetChannel(cmCapVideo,cmCapReceive, eRole);
	pCap = pChannel ? (CBaseVideoCap*)pChannel->GetDataAsCapClass() : NULL;

	/* First we need to check if the remote EP support Flow Control over RTCP */
	if (((pCap && pCap->IsSupportTMMBR()) || (( m_pSipCntl->GetRemoteIdent() == PolycomRMX || m_pSipCntl->GetRemoteIdent() == PolycomEp) && (!IsActiveTransaction())))) {
		/* In case of Polycom EP always send Flow control (RTCP/INFO) - verify the EP identity because CFG_KEY_ENABLE_FLOW_CONTROL_REINVITE can be turned on*/
		PTRACE(eLevelInfoNormal,"CSipParty::SendFlowControlMessage - info");

		m_pSipCntl->SendFlowControlReq(videoType, mediaDirection, newRateToSend);
	}
	else if(m_pSipCntl->IsRemoteMicrosoft() && (!IsActiveTransaction()) && (m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople) == eRtvCapCode))
	{
		//In MOC RTV we will send flow control in RTCP extension
		PTRACE(eLevelInfoNormal,"CSipParty::SendFlowControlMessage - MS RTCP");

		m_pSipCntl->SendRTCPBandwidthLimitation(newRateToSend);
	}
	else if( bSendFlowReinvite )
	{
		//our first priorety is to send flow control message in INFO message
		//if the Ep does not support this message (meaning it is not a polycom ep)
		//we will send re-invite with only different rate

		PTRACE(eLevelInfoNormal,"CSipParty::SendFlowControlMessage -re-invite");
		if (!IsActiveTransaction() && !IsGlareStatus())
		{
			StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
		}
		else
			PTRACE(eLevelInfoNormal,"CSipParty::SendFlowControlMessage - you can not call this function with an active trans");
	}
	else if ( !IsActiveTransaction() )
	{
			PTRACE(eLevelInfoNormal,"CSipParty::SendFlowControlMessage - info");

			m_pSipCntl->SendFlowControlReq(videoType, mediaDirection, newRateToSend);
	}
	else
		PTRACE(eLevelInfoNormal,"CSipParty::SendFlowControlMessage - you can not call this function with an active trans");

	/*need to free the pcap here*/
	POBJDELETE(pCap);

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendFlowControlToCs(CSegment* pParam)
{
	DWORD newVidRate = 0;
	BYTE outChannel;
	BYTE isLpr = 0;
	*pParam >> newVidRate;
	*pParam >> outChannel;
	DWORD lossProtection = 0;
	DWORD mtbf = 0;
	DWORD congestionCeiling = 0;
	DWORD fill = 0;
	DWORD modeTimeout = 0;
	*pParam >> isLpr;

    if (0 == newVidRate)
        newVidRate = m_pSipCntl->GetLastRateBeforeLpr();

	if (outChannel)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::SendFlowControlToCs : message recive on transmit channel an can't be sent !!!-the rate recieved is:  ", newVidRate);
		return;
	}
	if (isLpr == TRUE)
	{
		*pParam >> lossProtection >> mtbf >> congestionCeiling >> fill >> modeTimeout;

		CSegment* pSeg = new CSegment;
		*pSeg << newVidRate << outChannel << lossProtection << mtbf << congestionCeiling
			<< fill << modeTimeout;

		DispatchEvent(LPR_PARTY_FLOWCONTROL,pSeg);

		POBJDELETE(pSeg);
		return;

	}

	if (IsActiveTransaction())
	{
		PTRACE2(eLevelInfoNormal,"CSipParty::SendFlowControlToCs- there is an active trans - ",PARTYNAME);
		AddToChangeModeMask(eChangeModeMask_FlowControl);
		m_pUpdateTargetMode->SetVideoBitRate(newVidRate, cmCapReceive , (ERoleLabel)kRolePeople);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::SendFlowControlToCs : no active trans-the rate recieved is:  ", newVidRate);
		m_pTargetMode->SetVideoBitRate(newVidRate, cmCapReceive , (ERoleLabel)kRolePeople);
		m_pUpdateTargetMode->SetVideoBitRate(newVidRate, cmCapReceive , (ERoleLabel)kRolePeople);
		cmCapDirection mediaDir = cmCapReceive;
		SendFlowControlMessage(mainType/*main video type-people*/,mediaDir,newVidRate );
	}

}
/////////////////////////////////////////////////////////////////////////////
ESipTransactionType CSipParty::UniteConfAndTransModes(BYTE bSendFlowControl)
{
//	BYTE bSendReinvite = FALSE;
	ESipTransactionType needToActivateSubsequentTransection=kSipTransNone;
	// Currently, an overlapping between eChangeModeMask_FlowControl and eChangeModeMask_Incoming can't happen. because eChangeModeMask_FlowControl is only in VSW and eChangeModeMask_Incoming is only in COP.

	if (m_maskRequiredChangeMode & eChangeModeMask_FlowControl)	// Currently, only in VSW flow control or LPR.
	{
		TRACEINTO << PARTYNAME << " - State flow control in";
		int newVideoRate = m_pUpdateTargetMode->GetVideoBitRate(cmCapReceive,(ERoleLabel)kRolePeople);
		m_pTargetMode->SetVideoBitRate(newVideoRate, cmCapReceive , (ERoleLabel)kRolePeople);
		cmCapDirection mediaDir = cmCapReceive;
		if (bSendFlowControl)
			SendFlowControlMessage(mainType/*main video type-people*/,mediaDir,newVideoRate );
	}
	else if (m_maskRequiredChangeMode & eChangeModeMask_Incoming) // Currently, the only case is cop change decoder
	{
		TRACEINTO << PARTYNAME << " - Change incoming";
		m_pTargetMode->CopyMediaMode(*m_pUpdateTargetMode, cmCapVideo, cmCapReceive, kRolePeople);
		if (m_pTargetMode->GetConfType() == kCop)
			m_pSipCntl->SetLocalVideoCapsExactlyAccordingToScm(m_pTargetMode);
//		bSendReinvite = TRUE;
		needToActivateSubsequentTransection = kSipTransReInviteWithSdpReq;
	}
	else if (m_maskRequiredChangeMode & eChangeModeMask_ContentChangeProtocol) // Currently, only in CP. Can't occur with other change modes.
	{
		TRACEINTO << PARTYNAME << " - Change content protocol";
		m_pTargetMode->CopyMediaMode(*m_pUpdateTargetMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
//		bSendReinvite = TRUE;
		needToActivateSubsequentTransection = kSipTransReInviteWithSdpReq;
	}
	else if (m_maskRequiredChangeMode & eChangeModeMask_ContentChangeRate) // Currently, only in CP. Can't occur with other change modes.
	{
		TRACEINTO << PARTYNAME << " - Change content rate";
		DWORD newContentRate = m_pUpdateTargetMode->GetContentBitRate(cmCapReceive);
		OnConfChangeModeContentRate(newContentRate);
		// bSendReinvite = TRUE; ***ppc maybe for tandberg we need to set it to true, or call the reinvite inside OnConfChangeModeContentRate
	}
	else if (m_maskRequiredChangeMode & eChangeModeMask_UpgradeToMixed) // Currently, only in CP. Can't occur with other change modes.
	{
		TRACEINTO << PARTYNAME << " - Start upgrade to mixed";
		m_pTargetMode->CopyMediaMode(*m_pUpdateTargetMode, cmCapVideo, cmCapReceive, kRolePeople);
		m_pTargetMode->CopyMediaMode(*m_pUpdateTargetMode, cmCapVideo, cmCapTransmit, kRolePeople);
		m_pTargetMode->CopyMediaMode(*m_pUpdateTargetMode, cmCapAudio, cmCapReceive, kRolePeople);
		m_pTargetMode->CopyMediaMode(*m_pUpdateTargetMode, cmCapAudio, cmCapTransmit, kRolePeople);
		if(m_pSipCntl->GetIsMrcCall())
		{
			needToActivateSubsequentTransection =kSipTransUpgradeSvcOnlyToMixReq;
		}
		else
		{
			needToActivateSubsequentTransection =kSipTransUpgradeAvcOnlyToMixReq;
		}
		// bSendReinvite = TRUE; ***ppc maybe for tandberg we need to set it to true, or call the reinvite inside OnConfChangeModeContentRate
	}
	else
		DBGPASSERT(m_maskRequiredChangeMode+100);
	PTRACE2INT(eLevelInfoNormal,"CSipParty::UniteConfAndTransModes - Mask:", m_maskRequiredChangeMode);
	return needToActivateSubsequentTransection;
	//return bSendReinvite;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipParty::IsCurrentModeIncludedInCapsWithoutRate(CSipCaps* pCaps)
{
	DWORD details =  0;
	int arrInd 	 = 0;
	DWORD videoValuesToCompare 		= kCapCode|kFormat|kFrameRate|kH264Profile|kH264Level|kH264Additional;
	DWORD audioValuesToCompare 		= kCapCode|kFrameRate;
	CBaseCap* CurrentAudioBaseCapTransmit = m_pCurrentMode->GetMediaAsCapClass(cmCapAudio,cmCapTransmit);
	CBaseCap* CurrentVideoBaseCapTransmit = m_pCurrentMode->GetMediaAsCapClass(cmCapVideo,cmCapTransmit);
	CBaseCap* CurrentAudioBaseCapRec = m_pCurrentMode->GetMediaAsCapClass(cmCapAudio,cmCapReceive);
	CBaseCap* CurrentVideoBaseCapRec = m_pCurrentMode->GetMediaAsCapClass(cmCapVideo,cmCapReceive);
    if (CurrentAudioBaseCapTransmit && CurrentVideoBaseCapTransmit && CurrentAudioBaseCapRec && CurrentVideoBaseCapRec)
    {
		BYTE isVideoContainedTransmit = pCaps->IsContainedInCapSet(*CurrentVideoBaseCapTransmit, videoValuesToCompare, &details, &arrInd);
		BYTE isAudioContainedTransmit = pCaps->IsContainedInCapSet(*CurrentAudioBaseCapTransmit, audioValuesToCompare, &details, &arrInd);
		BYTE isVideoContainedRec = pCaps->IsContainedInCapSet(*CurrentVideoBaseCapRec, videoValuesToCompare, &details, &arrInd);
		BYTE isAudioContainedRec = pCaps->IsContainedInCapSet(*CurrentAudioBaseCapRec, audioValuesToCompare, &details, &arrInd);
		/*Need to free these caps since GetMediaAsCapClass will allocate them*/
		POBJDELETE(CurrentAudioBaseCapTransmit);
		POBJDELETE(CurrentVideoBaseCapTransmit);
		POBJDELETE(CurrentAudioBaseCapRec);
		POBJDELETE(CurrentVideoBaseCapRec);
		if (isVideoContainedTransmit && isAudioContainedTransmit && isVideoContainedRec && isAudioContainedRec)
			return TRUE;
		else
			return FALSE;
    }
    else
    {
        CMedString str;
        str << "CSipParty::IsCurrentModeIncludedInCapsWithoutRate Cap is NULL - CurrentAudioBaseCapTransmit = " << (DWORD)CurrentAudioBaseCapTransmit
            << " CurrentVideoBaseCapTransmit = " << (DWORD)CurrentVideoBaseCapTransmit
            << " CurrentAudioBaseCapRec = " << (DWORD)CurrentAudioBaseCapRec
            << " CurrentVideoBaseCapRec = " << (DWORD)CurrentVideoBaseCapRec;
        PTRACE (eLevelError, str.GetString());
		/*Need to free these caps since GetMediaAsCapClass will allocate them*/
		POBJDELETE(CurrentAudioBaseCapTransmit);
		POBJDELETE(CurrentVideoBaseCapTransmit);
		POBJDELETE(CurrentAudioBaseCapRec);
		POBJDELETE(CurrentVideoBaseCapRec);
        return FALSE;
    }

}
/////////////////////////////////////////////////////////////////////////////
/*
 this function do the following:
 1.check if this is an atomic action(flow control for example)
 2.if this is an atomic action it handles it.
 3.the return value is TRUE if this was an atomic action and FALSE if it wasn't.
 * */
BYTE CSipParty::HandleAtomicAction(CSipCaps* pCurRemoteCaps , CIpComMode* pBestMode)
{
	// TIP
	// slaves don't have atomic action
	if (m_tipPartyType != eTipMasterCenter && m_tipPartyType != eTipNone)
	{
		return FALSE;
	}

	if (m_pSipCntl->GetIsMrcCall() || m_pCurrentMode->GetConfMediaType()==eMixAvcSvcVsw)
	{
		return FALSE;
	}

	if ((m_pCurrentMode->GetConfType() == kVideoSwitch) || (m_pCurrentMode->GetConfType() == kVSW_Fixed))
	{
		BYTE isNewModeContainedInCurrent =IsCurrentModeIncludedInCapsWithoutRate(pCurRemoteCaps);
		if (isNewModeContainedInCurrent
			&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation)) && (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapData, cmCapReceiveAndTransmit)) )
		{
			//meaning this is only flow control message.
			BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
			if(!bEnableFlowControlVSW)
			{
				PTRACE(eLevelInfoNormal,"CSipParty::HandleAtomicAction: SUPPORT_VSW_FLOW_CONTROL is false -not atomic action");
				return FALSE;
			}
			else //we allow flow control vsw
			{
				//there is a new bit rate as flow control take it from new caps
				CBaseCap* CurrentVideoBaseCapTransmit = m_pCurrentMode->GetMediaAsCapClass(cmCapVideo,cmCapTransmit); AUTO_DELETE(CurrentVideoBaseCapTransmit);
				if (CurrentVideoBaseCapTransmit == NULL)
				{
					PTRACE(eLevelInfoNormal,"CSipParty::HandleAtomicAction: CurrentVideoBaseCapTransmit not exists ");
					return FALSE;
				}
				DWORD newBitRate=pCurRemoteCaps->GetVideoRateInMatchingCap((CBaseVideoCap*)CurrentVideoBaseCapTransmit);
				PTRACE2INT(eLevelInfoNormal,"CSipParty::HandleAtomicAction: VSW flow control sent to party control with rate",newBitRate);
				m_pCurrentMode->SetFlowControlRateConstraint(newBitRate);
				m_pTargetMode->SetVideoBitRate(newBitRate, cmCapTransmit , (ERoleLabel)kRolePeople);
				if (IsActiveTransaction() && m_pSipTransaction->IsNeedToUpdateFlowControlInVideoBridge())
				m_pConfApi->UpdatePartyControlOnNewRate(GetPartyId(),newBitRate , (WORD)cmCapTransmit ,(WORD)kRolePeople);

				CSegment * pSeg = new CSegment;
				DWORD status = STATUS_OK;
				*pSeg << (DWORD) status;
				m_pTargetMode->Serialize(NATIVE,*pSeg);
				int sizeOfUdps = sizeof(UdpAddresses);
				//pSeg->Put(m_pSipCntl->GetUdpAddress(), sizeOfUdps);
				UdpAddresses tempUdp;
				tempUdp = m_pSipCntl->GetUdpAddress();
				pSeg->Put((BYTE*)(&tempUdp), sizeOfUdps);
				BYTE contentSpeaker = 0;	//NA - Assert Fix
				BYTE ShouldPartyRemoveContent = 0;
                BYTE bUpdateMixModeResources = FALSE;
				*pSeg << (BYTE)contentSpeaker;
				*pSeg << (BYTE)ShouldPartyRemoveContent;
			    *pSeg << (BYTE)bUpdateMixModeResources;

				CPartyApi api;  // CTaskApi api;
				api.SetLocalMbx(GetLocalQueue());
				api.SendLocalMessage(pSeg, SIP_CONF_BRIDGES_UPDATED);
				return TRUE;

			}
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipParty::HandleAtomicAction:  -VSW not atomic action");
			return FALSE;
		}
	}
	else /*CP conf*/
	{
		if ((m_pTargetMode->IsMediaEquals(*pBestMode, cmCapAudio, cmCapReceiveAndTransmit))
			&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapData, cmCapReceiveAndTransmit))
			&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceive))
			&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
			&& (!(m_pTargetMode->GetConfType() == kCop && m_pTargetMode->GetCopTxLevel() != pBestMode->GetCopTxLevel())))
		{
			if (GetIsTipCall() && m_pSipTransaction->GetIsTipResumeMedia())
			{
				PTRACE(eLevelInfoNormal,"CSipParty::HandleAtomicAction: tip call is resumed - need to remove dtls from target mode if exist");
				RemoveDtlsFromTargetModeIfNeeded(pBestMode);
			}
			//meaning the only different is in the video-this could be flow control if the change is only in rate
			if( m_pTargetMode->IsMediaOn(cmCapVideo,cmCapTransmit) )
			{
				DWORD valuesToCompare	  = 0;
				DWORD details			  = 0;
				DWORD videoValuesToCompare = kCapCode|kFormat|kFrameRate|kH264Profile|kH264Level|kH264Additional|kPacketizationMode;
				BYTE bMediaContaining = m_pTargetMode->IsMediaContaining(*pBestMode, videoValuesToCompare,&details,cmCapVideo,cmCapTransmit);
				if (bMediaContaining)
					bMediaContaining = pBestMode->IsMediaContaining(*m_pTargetMode, videoValuesToCompare,&details,cmCapVideo,cmCapTransmit);
				if(bMediaContaining )//the only video change is rate out
				{
					//there is a new bit rate as flow control take it from new caps
					CBaseCap* CurrentVideoBaseCapTransmit = m_pCurrentMode->GetMediaAsCapClass(cmCapVideo,cmCapTransmit); AUTO_DELETE(CurrentVideoBaseCapTransmit);
					if ( (!pCurRemoteCaps) || (!CurrentVideoBaseCapTransmit) )
					{
					    DBGPASSERT(1204);
					    PTRACE(eLevelInfoNormal,"CSipParty::HandleAtomicAction - pCurRemoteCaps || CurrentVideoBaseCapTransmit is NULL");
					    return FALSE;
					}
					DWORD newBitRate = pCurRemoteCaps->GetVideoRateInMatchingCap((CBaseVideoCap*)CurrentVideoBaseCapTransmit);
					POBJDELETE(CurrentVideoBaseCapTransmit);
                    if (kCop == m_pCurrentMode->GetConfType())
                    {
                        const CCommConf*  pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
                        if (pCommCurrConf == NULL) {
                        	PTRACE(eLevelInfoNormal,"CSipParty::HandleAtomicAction: pCommCurrConf not exists in DB");
                        	return FALSE;
                        }
                        newBitRate = m_pCurrentMode->CalcCopMinFlowControlRate(pCommCurrConf, newBitRate);
                        m_pCurrentMode->SetFlowControlRateConstraint(newBitRate);
                    }

					PTRACE2INT(eLevelInfoNormal,"CSipParty::HandleAtomicAction: CP/COP flow control sent to party control with rate",newBitRate);
					m_pTargetMode->SetVideoBitRate(newBitRate, cmCapTransmit , (ERoleLabel)kRolePeople);
					m_pCurrentMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
					m_pConfApi->UpdatePartyControlOnNewRate(GetPartyId(),newBitRate , (WORD)cmCapTransmit ,(WORD)kRolePeople);
					CSegment * pSeg = new CSegment;
					DWORD status = STATUS_OK;
					*pSeg << (DWORD) status;
					m_pTargetMode->Serialize(NATIVE,*pSeg);
					int sizeOfUdps = sizeof(UdpAddresses);
					//pSeg->Put(m_pSipCntl->GetUdpAddress(), sizeOfUdps);
					UdpAddresses tempUdp;
					tempUdp = m_pSipCntl->GetUdpAddress();
					pSeg->Put((BYTE*)(&tempUdp), sizeOfUdps);
					BYTE contentSpeaker = 0;
					BYTE ShouldPartyRemoveContent = 0;
                    BYTE bUpdateMixModeResources = FALSE;
					*pSeg << (BYTE)contentSpeaker;
					*pSeg << (BYTE)ShouldPartyRemoveContent;
	                *pSeg << (BYTE)bUpdateMixModeResources;

					CPartyApi api;  // CTaskApi api;
					api.SetLocalMbx(GetLocalQueue());
					api.SendLocalMessage(pSeg, SIP_CONF_BRIDGES_UPDATED);
					return TRUE;

				}
				else
					return FALSE;
			}
			else
				return FALSE;
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipParty::HandleAtomicAction:  -CP/COP- not atomic action -not only video out changed");
			return FALSE;

		}

	}

}
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::UpdatePartySipLprVideoBitRate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::UpdatePartySipLprVideoBitRate : Name - ",PARTYNAME);

	DWORD newPeopleRate;
	WORD  channelDirection;
	DWORD lossProtection;
	DWORD mtbf;
	DWORD congestionCeiling;
	DWORD fill;
	DWORD modeTimeout;
	DWORD totalVideoRate;
	DWORD bLprOnContent;;
	DWORD newContentRate;

	*pParam >> newPeopleRate;
	*pParam >> channelDirection;
	*pParam >> lossProtection;
	*pParam >> mtbf;
	*pParam >> congestionCeiling;
	*pParam >> fill;
	*pParam >> modeTimeout;
	*pParam >> totalVideoRate;
	*pParam >> bLprOnContent;
	*pParam >> newContentRate;

	DWORD totalCallVideoRate = 0;
	CSipCall* currCall = m_pSipCntl->GetCallObj();
	if(currCall)
	{
		totalCallVideoRate = currCall->GetVideoCallRate((cmCapDirection)cmCapTransmit);
		if (MicrosoftEP_Lync_CCS == m_pSipCntl->GetRemoteIdent())
		{
			DWORD videorate	= 0;
			CSipChannel* pVidContChannel = currCall->GetChannel(true, cmCapVideo,(cmCapDirection)cmCapTransmit, kRolePresentation);
			if (pVidContChannel)
			{
				CBaseCap* pVidCap = pVidContChannel->GetDataAsCapClass();
				if (pVidCap)
					videorate = pVidCap->GetBitRate();
				POBJDELETE(pVidCap);
			}
			PTRACE2INT(eLevelInfoNormal,"CSipParty::UpdatePartySipLprVideoBitRate : FOR CCSPlugIn, Content Video Rate = ",videorate);
			totalCallVideoRate = videorate;

		}
	}
	else
		PASSERT_AND_RETURN(105);
	if ( newPeopleRate <= totalCallVideoRate )
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::UpdatePartySipLprVideoBitRate : totalvideorate inside - ",totalCallVideoRate);
        if (m_pCurrentMode->GetConfType() == kCop)
        {
            const CCommConf*  pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
            if (pCommCurrConf == NULL)
            {
            	PTRACE(eLevelInfoNormal,"CSipParty::UpdatePartySipLprVideoBitRate: pCommCurrConf not exists in DB");
            	PASSERT_AND_RETURN(106);
            }
            newPeopleRate = m_pCurrentMode->CalcCopMinFlowControlRate(pCommCurrConf, newPeopleRate);
            m_pCurrentMode->SetFlowControlRateConstraint(newPeopleRate);
        }

	    DWORD lastRate = m_pTargetMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
	    lastRate += m_pTargetMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePresentation);
	    PTRACE2INT(eLevelInfoNormal,"CSipParty::UpdatePartySipLprVideoBitRate : lastRate=",lastRate);
	    m_pSipCntl->SetLastRateBeforeLpr(lastRate)	;
		if ( MicrosoftEP_Lync_CCS == m_pSipCntl->GetRemoteIdent() )
		{
			lastRate = m_pTargetMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePresentation);
			if (m_pCurrentMode->GetConfType() != kCop)
		    	m_pTargetMode->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePresentation);
		    else
		    	m_pTargetMode->SetVideoBitRate((newPeopleRate /100), (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePresentation);

		}
		else
		{
			if (m_pCurrentMode->GetConfType() != kCop)
		    	m_pTargetMode->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);
		    else
		    	m_pTargetMode->SetVideoBitRate((newPeopleRate /100), (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);
		}
		//do we need here also to change current?


		m_pCurrentMode->SetTotalVideoRate(totalVideoRate);
		m_pTargetMode->SetTotalVideoRate(totalVideoRate);


		m_pConfApi->UpdatePartyLprVideoBitRate(GetPartyId(), newPeopleRate, channelDirection, lossProtection
				, mtbf, congestionCeiling, fill, modeTimeout,totalVideoRate, m_pSipCntl->GetRemoteIdent());	//!!!!!!!!!!!!!!!!!!!!
	}

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendLprFlowControlToCs(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::SendLprFlowControlToCs : Name - ",PARTYNAME);
	DWORD newVidRate = 0;
	BYTE outChannel;
	lPRModeChangeParams lprModeChangeData;

	*pParam >> newVidRate;
	*pParam >> outChannel;
	*pParam >> lprModeChangeData.lossProtection;
	*pParam >> lprModeChangeData.mtbf;
	*pParam >> lprModeChangeData.congestionCeiling;
	*pParam >> lprModeChangeData.fill;
	*pParam >> lprModeChangeData.modeTimeout;
	BOOL bIsGoodDetail = TRUE;
	bIsGoodDetail = CheckFlowControlDetails(newVidRate, outChannel);
	if(!bIsGoodDetail)
	{
		PTRACE2(eLevelError,"CH323Party::SendLprFlowControlToCard : not good params! returning! - ",PARTYNAME);
		return;
	}
	DWORD lastRate = m_pSipCntl->GetLastRateBeforeLpr();
	//restore last rate
	DWORD newVidRateToSend = 0;
	if(outChannel)
		newVidRateToSend = newVidRate ? newVidRate : lastRate;
	else
		newVidRateToSend = newVidRate ? newVidRate : m_pCurrentMode->GetVideoBitRate((cmCapDirection)cmCapReceive, (ERoleLabel)kRolePeople);

	if(  (!outChannel && !newVidRate) || (outChannel && newVidRate && (newVidRate > lastRate ) ) )
	{
		SendFlowControlMessageIfPossible(newVidRateToSend);
	}
	else if ( (!outChannel && newVidRate && newVidRate < m_pCurrentMode->GetVideoBitRate((cmCapDirection)cmCapReceive, (ERoleLabel)kRolePeople)) || (outChannel && !newVidRate))
	{
		SendFlowControlMessageIfPossible(newVidRateToSend);

	}//lower the incoming rate in vsw due to lpr
	/* For incoming channel, the rates at the party's level aren't updated.
	   The reason is that the party should store the originate rates, so in case a request
	   to return to the origin incoming rate is sent, the origin outgoing rates must be
	   restored as well. */
	if (outChannel && newVidRate) //outgoing channel
	{ //update the rates at the party's level
		m_pSipCntl->SetLastRateBeforeLpr(newVidRateToSend);
	}
	m_pSipCntl->SendLprReqToMfa((WORD)statOK, lprModeChangeData.lossProtection, lprModeChangeData.mtbf,lprModeChangeData.congestionCeiling
					, lprModeChangeData.fill, lprModeChangeData.modeTimeout);



}
/////////////////////////////////////////////////////////////////////////////
BOOL  CSipParty::CheckFlowControlDetails(DWORD newVidRate, BYTE direction)
{
//Cases, in which the mesage isn't been sent:
/*1*/
	if(direction==FALSE /*incoming*/ && (newVidRate > m_pCurrentMode->GetVideoBitRate((cmCapDirection)cmCapReceive, (ERoleLabel)kRolePeople)) )
	{//send this request for incoming video channel
		PTRACE(eLevelError,"CSipParty::CheckFlowControlDetails - We don't send flow control to increase incoming channel rate");
		return FALSE;
	}

/*2*/
	else if (direction==TRUE /*outgoing*/ && newVidRate && (newVidRate < m_pCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople)) )
	{//send this request for outgoing video channel
		PTRACE(eLevelError,"CSipParty::CheckFlowControlDetails - We don't send flow control to decrease outgoing channel rate");
		return FALSE;
	}

	return TRUE;
}
//this function checks if no active trans is active-if not a flow control message will be sent immidiatly
//else we will save the rate and sent it after the end of trans
void CSipParty::SendFlowControlMessageIfPossible(DWORD newVidRate)
{
	if (!IsActiveTransaction())
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::SendFlowControlMessageIfPossible : no active trans-the rate recieved is:  ", newVidRate);
		m_pTargetMode->SetVideoBitRate(newVidRate, cmCapReceive , (ERoleLabel)kRolePeople);
		m_pUpdateTargetMode->SetVideoBitRate(newVidRate, cmCapReceive , (ERoleLabel)kRolePeople);
		cmCapDirection mediaDir = cmCapReceive;
		SendFlowControlMessage(mainType/*main video type-people*/,mediaDir,newVidRate );
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CSipParty::SendFlowControlMessageIfPossible- there is an active trans : Name - ",PARTYNAME);
		AddToChangeModeMask(eChangeModeMask_FlowControl);
		m_pUpdateTargetMode->SetVideoBitRate(newVidRate, cmCapReceive , (ERoleLabel)kRolePeople);

	}

}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnLprUpdatedPartySIPVideoBitRate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnLprUpdatedPartySIPVideoBitRate : Name - ",PARTYNAME);

	WORD  status;
	DWORD lossProtection;
	DWORD mtbf;
	DWORD congestionCeiling;
	DWORD fill;
	DWORD modeTimeout;

	*pParam >> status;
	*pParam >> lossProtection;
	*pParam >> mtbf;
	*pParam >> congestionCeiling;
	*pParam >> fill;
	*pParam >> modeTimeout;
	DWORD totalCallVideoRate = 0;
	// Currently the Total allowed video rate is the congestionCeiling
	CSipCall* currCall = m_pSipCntl->GetCallObj();
	if(currCall)
		totalCallVideoRate = currCall->GetVideoCallRate((cmCapDirection)cmCapTransmit);
	else
		PASSERT_AND_RETURN(105);

	DWORD newTotalRate = 0;
	if(totalCallVideoRate)
		newTotalRate = min(congestionCeiling,totalCallVideoRate);
	else
		newTotalRate = congestionCeiling;

	GetTargetMode()->SetTotalVideoRate(newTotalRate);
	GetCurrentMode()->SetTotalVideoRate(newTotalRate);
	// Sending new rates to video and content bridges
	m_pSipCntl->SetIsLprModeOn(YES);
	m_pSipCntl->SendLprReqToMfa(status, lossProtection, mtbf, congestionCeiling, fill, modeTimeout);

}
/////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED: update the current mode with new rate only (the resolution is not updated, it will stay with the maximum)
void CSipParty::VideoRateForFecOrRedUpdated(CSegment* pParam)
{
	WORD  status = statIllegal; //default value
	DWORD type = cmCapEmpty;  //default value

	*pParam >> status >> type;
	TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << ", Name:" << PARTYNAME << ", status:" << status << ", type:" << type;

	DWORD newPeopleRate = 0;
	DWORD CurrentVideoRate = 0;
	DWORD TargetVideoRate  = 0;
	CurrentVideoRate = m_pCurrentMode->GetVideoBitRate(cmCapTransmit, kRolePeople);
	TargetVideoRate  = m_pTargetMode->GetVideoBitRate(cmCapTransmit, kRolePeople);


	if(status == statOK)
	{
		if (type == cmCapVideo)
		{
			if(m_pSipCntl->GetIsFecOn())
			{
				//newPeopleRate = m_pSipCntl->GetBitRateWithFEC();
				newPeopleRate = CurrentVideoRate - (m_pSipCntl->GetTheChangeOfRateForFEC());
				TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - FEC is ON: UPDATE_CURRENTMODE with new rate. Name:"
						  << PARTYNAME << ", CurrentVideoRate:" << CurrentVideoRate << ", TheChangeOfRateForFEC:"
						  << m_pSipCntl->GetTheChangeOfRateForFEC() << ", newPeopleRate:" << newPeopleRate;
			}
		else
			{
				//newPeopleRate = m_pSipCntl->GetLastRateBeforeFEC();
				newPeopleRate = CurrentVideoRate + (m_pSipCntl->GetTheChangeOfRateForFEC());
				TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - FEC is OFF: UPDATE_CURRENTMODE with new rate. Name:"
						  << PARTYNAME << ", CurrentVideoRate:" << CurrentVideoRate << ", TheChangeOfRateForFEC:"
						  << m_pSipCntl->GetTheChangeOfRateForFEC() << ", newPeopleRate:" << newPeopleRate;
			}

			m_pCurrentMode->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);
			m_pSipCntl->SendFecOrRedReqToART(type,status);

		}
		else if (type == cmCapAudio) // type == 1 == cmCapAudio
		{
			if(m_pSipCntl->GetIsRedOn())
			{
				//newPeopleRate = m_pSipCntl->GetBitRateWithRED();  //-audio
				newPeopleRate = CurrentVideoRate - (m_pSipCntl->GetTheChangeOfRateForRED());
				TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - RED is ON: UPDATE_CURRENTMODE with new rate. Name:"
						  << PARTYNAME << ", CurrentVideoRate:" << CurrentVideoRate << ", TheChangeOfRateForRED:"
						  << m_pSipCntl->GetTheChangeOfRateForRED() << ", newPeopleRate:" << newPeopleRate;
			}
			else
			{
				//newPeopleRate = m_pSipCntl->GetLastRateBeforeRED(); //+audio..
				newPeopleRate = CurrentVideoRate + (m_pSipCntl->GetTheChangeOfRateForRED());
				TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - RED is OFF: UPDATE_CURRENTMODE with new rate. Name:"
						  << PARTYNAME << ", CurrentVideoRate:" << CurrentVideoRate << ", TheChangeOfRateForRED:"
						  << m_pSipCntl->GetTheChangeOfRateForRED() << ", newPeopleRate:" << newPeopleRate;
			}

			m_pCurrentMode->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);
			m_pSipCntl->SendFecOrRedReqToART(type,status);

		}
		else TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - ERROR! (type=cmCapEmpty)";

	}
	else
	{
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - ERROR! (status=statIllegal) Name:" << PARTYNAME << ", CurrentVideoRate:" << CurrentVideoRate << ", type:" << type;
		m_pSipCntl->SendFecOrRedReqToART(type,status);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdatePartyVideoBitRate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::UpdatePartyVideoBitRate : Name - ",PARTYNAME);


		DWORD newBitRate;
		WORD channelDirection;
		WORD roleLabel;

		*pParam >> newBitRate;
		*pParam >> channelDirection;
		*pParam >> roleLabel;

		if ( ((cmCapDirection)channelDirection == cmCapTransmit) && ((ERoleLabel)roleLabel == kRolePeople) &&
	         newBitRate <= m_pSipCntl->GetLastRateBeforeLpr() )
		{
			if ((m_pCurrentMode->GetConfType() == kVideoSwitch) || (m_pCurrentMode->GetConfType() == kVSW_Fixed))
			{
				// VSW:
				BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
				if(!bEnableFlowControlVSW)
				{
					PTRACE(eLevelError,"CSipParty::UpdatePartyVideoBitRate: SUPPORT_VSW_FLOW_CONTROL is false");
					return;
				}
				m_pCurrentMode->SetFlowControlRateConstraint(newBitRate);
			}
            else if (m_pCurrentMode->GetConfType() == kCop)
            {
                const CCommConf*  pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
				if (pCommCurrConf == NULL)
				{
					PTRACE(eLevelInfoNormal,"CSipParty::UpdatePartyVideoBitRate: pCommCurrConf not exists in DB");
					return;
				}
                newBitRate = m_pCurrentMode->CalcCopMinFlowControlRate(pCommCurrConf, newBitRate);
                m_pCurrentMode->SetFlowControlRateConstraint(newBitRate);
            }

			else
			{
				// CP:
				m_pCurrentMode->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)roleLabel);
		        //m_pH323Cntl->m_pCurrentModeH323->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)roleLabel);
			}
			m_pConfApi->UpdatePartyControlOnNewRate(GetPartyId(), newBitRate, (cmCapDirection)channelDirection, (ERoleLabel)roleLabel);
		}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnMcuMngrStartPartyPreviewReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnMcuMngrStartPartyPreviewReq : Name - ",PARTYNAME);

	EConnectionState ChannelState;
	CSipChannel *Channel = NULL;
	BOOL IsPossibleToSendReq = FALSE;
	CapEnum capEnum;

	CIstrStream istream(*pParam);
	CPartyPreviewDrv* tmpPreviewReqParams = new CPartyPreviewDrv; AUTO_DELETE(tmpPreviewReqParams);
	tmpPreviewReqParams->DeSerialize(NATIVE,istream);

	cmCapDirection Direction = cmCapReceive;

	if(tmpPreviewReqParams->GetDirection())
		Direction = cmCapTransmit;

	if((cmCapDirection)Direction == cmCapReceive)
	{
		Channel = m_pSipCntl->GetChannel(cmCapVideo,cmCapReceive);
		if(Channel && !m_bIsPreviewVideoIn)
		{
			ChannelState = m_pSipCntl->GetChannelConnectionState(cmCapVideo, cmCapReceive, kRolePeople);
			if ((ChannelState == kDisconnecting) || (ChannelState == kDisconnected) || (ChannelState == kUnknown))
			{
				PTRACE2INT(eLevelInfoNormal,"CSipParty::OnMcuMngrStartPartyPreviewReq - Channel in state doesn't enable Preview. state - ", ChannelState);
				return;
			}
			else
			{
				IsPossibleToSendReq = TRUE;
				m_bIsPreviewVideoIn = TRUE;

				if(m_RcvPreviewReqParams)
						POBJDELETE(m_RcvPreviewReqParams);
				m_RcvPreviewReqParams = new CPartyPreviewDrv(*tmpPreviewReqParams);

			}
		}
	 }
	else  //cmCapTransmit
	{
		Channel = m_pSipCntl->GetChannel(cmCapVideo,cmCapTransmit);
		if(Channel && !m_bIsPreviewVideoOut)
		{
			ChannelState = m_pSipCntl->GetChannelConnectionState(cmCapVideo, cmCapTransmit, kRolePeople);
			if ((ChannelState == kDisconnecting) || (ChannelState == kDisconnected) || (ChannelState == kUnknown))
			{
				PTRACE2INT(eLevelInfoNormal,"CSipParty::OnMcuMngrStartPartyPreviewReq - Channel out state doesn't enable Preview. state - ", ChannelState);
				return;
			}
			else
			{
				IsPossibleToSendReq = TRUE;
				m_bIsPreviewVideoOut = TRUE;

				if(m_TxPreviewReqParams)
					POBJDELETE(m_TxPreviewReqParams);
				m_TxPreviewReqParams = new CPartyPreviewDrv(*tmpPreviewReqParams);

			}
		}
	}

	if (NULL == Channel)
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnMcuMngrStartPartyPreviewReq - No channels for preview");
	}
	else if(IsPossibleToSendReq)
	{
		capEnum = Channel->GetAlgorithm();
		m_pSipCntl->SendStartPartyPreviewReqToCM(tmpPreviewReqParams->GetRemoteIP(),tmpPreviewReqParams->GetVideoPort(),Direction,capEnum);
	}


	POBJDELETE(tmpPreviewReqParams);

}
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnMcuMngrStopPartyPreviewReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnMcuMngrStopPartyPreviewReq : Name - ",PARTYNAME);

	WORD Direction;
	EConnectionState ChannelState;
	CSipChannel* Channel = NULL;
	BOOL IsPossibleToSendReq = FALSE;
	CapEnum capEnum;

	*pParam >> Direction;

	if((cmCapDirection)Direction == cmCapReceive)
	{
		Channel = m_pSipCntl->GetChannel(cmCapVideo,cmCapReceive);
		if(Channel && m_bIsPreviewVideoIn)
		{
			ChannelState = m_pSipCntl->GetChannelConnectionState(cmCapVideo, cmCapReceive, kRolePeople);
			if ((ChannelState == kDisconnecting) || (ChannelState == kDisconnected) || (ChannelState == kUnknown))
			{
				PTRACE2INT(eLevelInfoNormal,"CSipParty::OnMcuMngrStopPartyPreviewReq - Channel In state doesn't enable Preview. state - ", ChannelState);
				return;
			}
			else
			{
				IsPossibleToSendReq = TRUE;
				m_bIsPreviewVideoIn = FALSE;
				if(m_RcvPreviewReqParams)
					POBJDELETE(m_RcvPreviewReqParams);
			}
		}
		else
			if(m_RcvPreviewReqParams)
				POBJDELETE(m_RcvPreviewReqParams);
	}
	else  //cmCapTransmit
	{
		Channel = m_pSipCntl->GetChannel(cmCapVideo,cmCapTransmit);
		if (Channel && m_bIsPreviewVideoOut)
		{
			ChannelState = m_pSipCntl->GetChannelConnectionState(cmCapVideo, cmCapTransmit, kRolePeople);
			if ((ChannelState == kDisconnecting) || (ChannelState == kDisconnected) || (ChannelState == kUnknown))
			{
				PTRACE2INT(eLevelInfoNormal,"CSipParty::OnMcuMngrStopPartyPreviewReq - Channel out state doesn't enable Preview. state - ", ChannelState);
				return;
			}
			else
			{
				IsPossibleToSendReq = TRUE;
				m_bIsPreviewVideoOut = FALSE;
				if(m_TxPreviewReqParams)
					POBJDELETE(m_TxPreviewReqParams);
			}
		}
		else
			if(m_TxPreviewReqParams)
				POBJDELETE(m_TxPreviewReqParams);
	}

	if(IsPossibleToSendReq)
	    m_pSipCntl->SendStopPartyPreviewReqToCM((cmCapDirection)Direction);

}
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnMcuMngrIntraPreviewReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnMcuMngrIntraPreviewReq : Name - ",PARTYNAME);

	WORD Direction;
	CSipChannel* pChannel = NULL;
	EConnectionState ChannelState;
	BOOL IsPossibleToSendReq = FALSE;
	BYTE bIsGradualIntra = FALSE;
	CapEnum capEnum;
	CSegment*  seg = NULL;

	*pParam >> Direction;
	PTRACE2INT(eLevelInfoNormal,"CSipParty::OnMcuMngrIntraPreviewReq dir is  :  - ",Direction);
	 if((cmCapDirection)Direction == cmCapReceive)
	 {
		pChannel = m_pSipCntl->GetChannel(cmCapVideo,cmCapReceive);
		if (pChannel /*&& m_bIsPreviewVideoIn*/)
		{
			ChannelState = m_pSipCntl->GetChannelConnectionState(cmCapVideo, cmCapReceive, kRolePeople);
			if ((ChannelState == kDisconnecting) || (ChannelState == kDisconnected) || (ChannelState == kUnknown))
			{
				PTRACE2INT(eLevelInfoNormal,"CSipParty::OnMcuMngrIntraPreviewReq - Channel In state doesn't enable intra Preview. state - ", ChannelState);
				return;
			}
			else
			{
				//Intra for incoming channel - Ask from EP
				 seg = new CSegment;
				*seg << (WORD)Fast_Update << (WORD)pChannel->GetRoleLabel() << (WORD)3 << bIsGradualIntra;
				DispatchEvent(RMTH230,seg);
				POBJDELETE(seg);
			}
		}
	 }
	 else  //cmCapTransmit
	 {

	 	pChannel = m_pSipCntl->GetChannel(cmCapVideo,cmCapTransmit);
	 	if (pChannel /*&&*m_bIsPreviewVideoOut*/)
	 	{
	 		ChannelState = m_pSipCntl->GetChannelConnectionState(cmCapVideo, cmCapTransmit, kRolePeople);
	 		if ((ChannelState == kDisconnecting) || (ChannelState == kDisconnected) || (ChannelState == kUnknown))
	 		{
	 			PTRACE2INT(eLevelInfoNormal,"CSipParty::OnMcuMngrIntraPreviewReq - Channel out state doesn't enable intra Preview. state - ", ChannelState);
	 			return;
	 		}
	 		else
	 		{
	 			PTRACE2INT(eLevelInfoNormal,"CSipParty::OnMcuMngrIntraPreviewReq transmit : Name - ",m_state);
	 			//18541 in case of cop  we will send a different request so we will know not to suppress this message.
	 			DWORD confType = m_pTargetMode->GetConfType();
	 			if (confType == kCop)
	 			{
	 				m_pConfApi->EventModeIntraPreviewReq(GetPartyId());
	 			}
	 			else
	 			{
	 				//Intra for outgoing channel - Ask from EP
	 				seg = new CSegment;
	 				*seg << (WORD)Fast_Update << (WORD)pChannel->GetRoleLabel() << (WORD)2 << bIsGradualIntra;
	 				DispatchEvent(RMTH230,seg);
	 				POBJDELETE(seg);
	 			}
	 		}
	 	}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::StopAllPreviews()
{
	m_bIsPreviewVideoIn = FALSE;
	m_bIsPreviewVideoOut = FALSE;
	if(m_RcvPreviewReqParams)
		POBJDELETE(m_RcvPreviewReqParams);
	if(m_TxPreviewReqParams)
		POBJDELETE(m_TxPreviewReqParams);
	//No need to send stop preview tp CM because channel disconnects
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnConfChangeModeConnect(CSegment* pParam)
{
	m_bIsConfChangeMode = TRUE;
	DWORD status=STATUS_OK;
	eChangeModeState changeModeState = eNotNeeded;

	BYTE tempChangeMode = 0, bIsIpComMode = FALSE;

	*pParam >> tempChangeMode;

	if(tempChangeMode < eLastChangeModeState)
		changeModeState = (eChangeModeState)tempChangeMode;

	CIpComMode* pNewMode = new CIpComMode;

	*pParam >> bIsIpComMode;

	if (bIsIpComMode)
		pNewMode->DeSerialize(NATIVE,*pParam);

	BYTE bIsContentSpeaker = 0;

	*pParam >> bIsContentSpeaker;

	m_pSipCntl->SetIsContentSpeaker(bIsContentSpeaker);

	CSipCaps *pNewLocalCaps = new CSipCaps;

	BYTE bIsLocalCaps;

	*pParam >> bIsLocalCaps;

	if (bIsLocalCaps)
		pNewLocalCaps->DeSerialize(NATIVE, *pParam);

	BYTE bIsASSIPContentEnable;
	*pParam >> bIsASSIPContentEnable;

	m_pSipCntl->SetASSIPContent(bIsASSIPContentEnable);

    CRsrcParams* pMrmpRsrcParams = NULL;
    DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams, "mix_mode: MRMP");

    CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
    for(int i=0; i < NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS; ++i)
    {
        avcToSvcTranslatorRsrcParams[i] = NULL;
        DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i], "mix_mode: translator");
    }

	CCommConf* pCommConf 	= ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
	if (pCommConf)
	{
		if(pCommConf->GetIsAsSipContent() && bIsASSIPContentEnable)
		{
			m_bTransactionSetContentOn = TRUE;
			PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeConnect  - Set m_bTransactionSetContentOn as ON");
		}
	}

	CMedString str;
	str << "Change mode state = " << GetChangeModeStateStr(changeModeState) << ", Content Speaker = " << bIsContentSpeaker << ", bIsLocalCaps = " << bIsLocalCaps << ", bIsASSIPContentEnable = " << bIsASSIPContentEnable;
	PTRACE2(eLevelInfoNormal,"***CSipParty::OnConfChangeModeConnect : ", str.GetString());

	pNewMode->Dump("***CSipParty::OnConfChangeModeConnect new mode", eLevelInfoNormal);

	//FSN-613: Dynamic Content for SVC/Mix Conf test
	m_pCurrentMode->Dump("***CSipParty::OnConfChangeModeConnect current:",eLevelInfoNormal);

	if (changeModeState == eChangeIncoming || changeModeState == eFallBackFromTipToSip || changeModeState == eFallBackFromIceToSip)
	{
		if (IsActiveTransaction() || IsGlareStatus())
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeConnect : Active transaction");
			m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapVideo, cmCapReceive, kRolePeople);
			AddToChangeModeMask(eChangeModeMask_Incoming);
		}
		else
		{
			DWORD totalVideoRate = m_pTargetMode->GetTotalVideoRate();
			*m_pTargetMode = *pNewMode;

			POBJDELETE(m_pTargetModeMaxAllocation);
			m_pTargetModeMaxAllocation = new CIpComMode(*m_pTargetMode);

			if(changeModeState == eFallBackFromTipToSip)
			{
				PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeConnect -fall back from TIP to SIP");

				m_changeModeState = eFallBackFromTipToSip;
				const CSipCaps* pRemoteCaps	= m_pSipCntl->GetLastRemoteCaps();

				if( pRemoteCaps && pRemoteCaps->GetIsContainingCapCode(cmCapAudio, eAAC_LDCapCode) )
				{
					PTRACE2INT(eLevelInfoNormal,"CSipParty::OnConfChangeModeConnect -change remote caps to local, bfcp transport ", pNewLocalCaps->GetBfcpTransportType());
					if (bIsLocalCaps)
					{
						//pNewLocalCaps->SetBfcpTransportType(pRemoteCaps->GetBfcpTransportType());
                        if (pRemoteCaps->GetBfcpTransportType() != eUnknownTransportType)
                        	pNewLocalCaps->SetBfcpTransportType(pRemoteCaps->GetBfcpTransportType());

						m_pSipCntl->SetRemoteCaps(*pNewLocalCaps);
					}
				}

				UpdateCurrentRcvModeAccordingToTarget();

				CSipComMode* pBestMode =NULL;
				pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, TRUE,TRUE/*intersect with max caps*/);  //DPA
				if (pBestMode)
				{
				    pBestMode->Dump("CSipParty::OnConfChangeModeConnect BEST MODE",eLevelInfoNormal);
				    PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeConnect -fall back from TIP to SIP 2");
				    *m_pTargetMode = *pBestMode;//in fall back to SIP we already have remote caps and want to start according to best mode from the start.
				    m_pSipCntl->AdjustLocalCapToNonTip(m_pTargetMode);
				    m_pTargetMode->SetTotalVideoRate(totalVideoRate);
				    //Need to remove H264 main cap
				    m_bIsNeedToStartIVRAfterFallbackFromTip = TRUE;
				}
				else
				    PTRACE(eLevelError,"CSipParty::OnConfChangeModeConnect - pBestMode is NULL - can not be fall back from TIP to SIP 2");
			}



			if (changeModeState == eFallBackFromIceToSip)
			{
				if (bIsLocalCaps)
					m_pSipCntl->SetLocalCaps(*pNewLocalCaps);

				m_pSipCntl->SetIceConnectivityStatus(eIceNotConnected);
			}

			if (m_pTargetMode->GetConfType() == kCop)
				m_pSipCntl->SetLocalVideoCapsExactlyAccordingToScm(m_pTargetMode);

			StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
		}
	}
	else if (changeModeState == eChangeContentRate)
	{
		DWORD newContentRate = pNewMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRolePresentation);
		if (IsActiveTransaction() || IsGlareStatus())
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeConnect : Active transaction");
			m_pUpdateTargetMode->SetContentBitRate(newContentRate, cmCapReceiveAndTransmit);
			AddToChangeModeMask(eChangeModeMask_ContentChangeRate);
		}
		else
			OnConfChangeModeContentRate(newContentRate);
	}
	else if (changeModeState == eChangeContentInAndOut)
	{
		if (IsActiveTransaction() || IsGlareStatus())
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeConnect : Active transaction");
			m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
			AddToChangeModeMask(eChangeModeMask_ContentChangeProtocol);
		}
		else
			OnConfChangeModeContentProtocol(pNewMode);
	}
	else if(changeModeState==eConfRequestMoveToMixed)
	{
		TRACEINTO << "NewModeConfMediaType:" << ConfMediaTypeToString(pNewMode->GetConfMediaType());
		//m_pSipCntl->SetConfMediaType(m_pCurrentMode->GetConfMediaType()); ey_20866 ask noa why this doesn't work
		m_pSipCntl->SetConfMediaType(pNewMode->GetConfMediaType());
		// BRIDGE-13341
		m_pTargetModeMaxAllocation->SetConfMediaType(pNewMode->GetConfMediaType());
		if (IsActiveTransaction()) // ey_20866
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeConnect : Active transaction");
			m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapVideo, cmCapReceive, kRolePeople);
			m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapVideo, cmCapTransmit, kRolePeople);
			m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapAudio, cmCapReceive, kRolePeople);
			m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapAudio, cmCapTransmit, kRolePeople);

			// create the upgrade params
            POBJDELETE(m_pUpgradeParams);
			m_pUpgradeParams = new CSegment;
            if (!m_pSipCntl->GetIsMrcCall())
            {// serialize resources
                SerializeNonMandatoryRsrcParams(m_pUpgradeParams, pMrmpRsrcParams);
                SerializeNonMandatoryRsrcParamsArray(m_pUpgradeParams, avcToSvcTranslatorRsrcParams, NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS, "translator");
            }

			AddToChangeModeMask(eChangeModeMask_UpgradeToMixed); // ey_20866 need to decide how to handle transaction
		}
		else
		{
			*m_pTargetMode = *pNewMode;
			m_pCurrentMode->SetConfMediaType(m_pTargetMode->GetConfMediaType());
			TRACEINTO << "TargetModeConfMediaType:" << ConfMediaTypeToString(m_pTargetMode->GetConfMediaType());

			ESipTransactionType eTransactionType;
		    CSegment *pSeg = new CSegment;

		    if (!m_pSipCntl->GetIsMrcCall())
			{
                // serialize resources
                SerializeNonMandatoryRsrcParams(pSeg, pMrmpRsrcParams);
                SerializeNonMandatoryRsrcParamsArray(pSeg, avcToSvcTranslatorRsrcParams, NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS, "!@#  translator" );
			}

			if(m_pSipCntl->GetIsMrcCall())
			{
				eTransactionType=kSipTransUpgradeSvcOnlyToMixReq;
			}
			else
			{
				eTransactionType=kSipTransUpgradeAvcOnlyToMixReq;
			}

			StartTransaction(eTransactionType, PARTY_UPGRADE_TO_MIXED, pSeg);

			POBJDELETE(pSeg);
		}
	}

			POBJDELETE(pMrmpRsrcParams);
			for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
			{
				 POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
			}

	POBJDELETE(pNewMode);
	POBJDELETE(pNewLocalCaps);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::AddToChangeModeMask(EChangeModeMask newChangeMode)
{
	/* Currently, these two types of change mode can't happen in the same party because eChangeModeMask_FlowControl is only in VSW and eChangeModeMask_Incoming is only in COP,
	   so we can set m_maskRequiredChangeMode to the new value without checking the existing value of m_maskRequiredChangeMode. */
	if (newChangeMode == eChangeModeMask_FlowControl || newChangeMode == eChangeModeMask_Incoming  || newChangeMode ==eChangeModeMask_UpgradeToMixed)
		m_maskRequiredChangeMode = newChangeMode;
	else if (newChangeMode == eChangeModeMask_ContentChangeProtocol)
		m_maskRequiredChangeMode = (EChangeModeMask)(m_maskRequiredChangeMode | newChangeMode);
	else if (newChangeMode == eChangeModeMask_ContentChangeRate)
	{
		if (!(m_maskRequiredChangeMode & eChangeModeMask_ContentChangeProtocol)) // if we already wait for change protocol, we will do the change rate as part of it.
			m_maskRequiredChangeMode = (EChangeModeMask)(m_maskRequiredChangeMode | newChangeMode);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSetPartyToLeader(CSegment* pParam)
{
	BYTE isLeader;
	*pParam >> isLeader;

	if (isLeader != m_feccPartyType)
	{
		m_feccPartyType = isLeader;
		m_pSipCntl->UpdateRtpOnLeaderStatus(isLeader);
	}
}
/////////////////////////////////////////////////////////////////////////////
DWORD CSipParty::GetTimerValueForGlare()
{
	DWORD retRandom = 0;
	int range=(m_maxValForGlareTimer-m_minValForGlareTimer)+1;
	//retRandom = m_minValForGlareTimer + int(range*rand()/(RAND_MAX + 1.0));
	retRandom = m_minValForGlareTimer + (rand() % range);
	return retRandom;
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSipGlareTimer(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnSipGlareTimer");
	if (IsGlareStatus())
	{
		//SetGlareStatus(FALSE);
		StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
		SetGlareStatus(FALSE); //BRIDGE-12961
	}
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipParty::IsGlareStatus()
{
	if (m_bIsGlareStatus)
	{
		PTRACE(eLevelInfoNormal,"CSipParty::IsGlareStatus - TRUE");
		return TRUE;
	}
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::HandleGlareStatus()
{
	DWORD dwGlareTimer = GetTimerValueForGlare();
	PTRACE2INT(eLevelInfoNormal,"CSipParty::HandleGlareStatus - start timer: ", dwGlareTimer);

	if (IsValidTimer(SIP_GLARE_TIMER))
		DeleteTimer(SIP_GLARE_TIMER);
	StartTimer(SIP_GLARE_TIMER, dwGlareTimer);

	SetGlareStatus(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSendIntraAfterTransactionTout()
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnSendIntraAfterTransactionTout");
	m_pConfApi->VideoRefresh(GetPartyId());

	m_nIntraAfterTransaction++;

	if(m_repeatIntra < m_repeatIntraNumber && ( m_pSipCntl->GetRemoteIdent() == MicrosoftEP_R1 || m_pSipCntl->GetRemoteIdent() == MicrosoftEP_R2 || m_pSipCntl->GetRemoteIdent() == MicrosoftEP_Lync_R1 || m_pSipCntl->GetRemoteIdent() == MicrosoftEP_Lync_2013 || m_pSipCntl->GetRemoteIdent() == MicrosoftEP_MAC || m_pSipCntl->GetRemoteIdent() == MicrosoftEP_MAC_Lync || m_pSipCntl->IsSameTimeEP() || m_pSipCntl->GetRemoteIdent() == IbmSametimeEp_Legacy|| GetIsTipCall() ))
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnSendIntraAfterTransactionTout - set timer to repeat intra");
		StartTimer(SIP_SEND_INTRA_AFTER_TRANS_TOUT, 3*SECOND);
		m_repeatIntra++;
	}
	else if(m_repeatIntra == m_repeatIntraNumber)
	{
		m_repeatIntra 		= 0;
		m_repeatIntraNumber = 0;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSipPartyUnMuteClosingChannel(CSegment* pParam)
{
	DWORD ChanType;
	*pParam >> ChanType;

	PTRACE2INT(eLevelInfoNormal,"CSipParty::OnSipPartyUnMuteClosingChannel channel ", ChanType);


	if( (EIpChannelType)ChanType == AUDIO_IN || (EIpChannelType)ChanType == AUDIO_OUT )
	{
	    m_pConfApi->UpdateDB(this,MUTE_STATE, 0x00000000, 1); //indicate party audio is not muted by operator
	    m_pConfApi->UpdateDB(this,MUTE_STATE, 0xF0000000, 1); //indicate party is not audio self muted
	    m_pConfApi->UpdateDB(this,MUTE_STATE, 0x0F000000, 1); //indicate party audio is not muted by MCU

	}
	else if( (EIpChannelType)ChanType == VIDEO_IN || (EIpChannelType)ChanType == VIDEO_OUT )
	{
	    m_pConfApi->UpdateDB(this,MUTE_STATE, 0x0000000E, 1); //indicate party video is not muted by operator
	    m_pConfApi->UpdateDB(this,MUTE_STATE, 0xF000000E, 1); //indicate party is not video self muted
	    m_pConfApi->UpdateDB(this,MUTE_STATE, 0x0F00000E, 1); //indicate party video is not muted by MCU
	}

 	m_pConfApi->IpMuteMedia(GetPartyId(),
			    ((EIpChannelType)ChanType == AUDIO_IN)?eOff:AUTO , ((EIpChannelType)ChanType == AUDIO_OUT)?eOff:AUTO,
				((EIpChannelType)ChanType == VIDEO_IN)?eOff:AUTO , ((EIpChannelType)ChanType == VIDEO_OUT)?eOff:AUTO,
				AUTO,AUTO,AUTO,AUTO);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnRemoveSelfFlowControlConstraint(CSegment* pParam)
{
    PTRACE (eLevelInfoNormal, "CSipParty::OnRemoveSelfFlowControlConstraint");
    m_pCurrentMode->SetFlowControlRateConstraint(0);
}
/////////////////////////////////////////////////////////////////////////////

void CSipParty::OnSipConfNIDivrProviderEQ (CSegment * pParam)
{

	// TIP
	// slaves don't have signaling
	if (m_tipPartyType != eTipMasterCenter && m_tipPartyType != eTipNone)
		return;
	char numericConfId[NUMERIC_CONFERENCE_ID_LEN];
	memset(numericConfId, '\0', NUMERIC_CONFERENCE_ID_LEN);
	WORD len;

	*pParam >> len;

	if (len)
		pParam->Get( (BYTE*)&numericConfId, len );

	if(m_state == PARTYDISCONNECTING)
	{
		PTRACE(eLevelError,"CSipParty::OnSipConfNIDivrProviderEQ: Party is disconnecting ");
		return ;
	}

	m_pSipCntl->SendIvrProviderEQReq(numericConfId);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnVideoBridgeMultiVSRReq(CSegment * pParam)
{
	ST_VSR_MUTILPLE_STREAMS vsr;
	pParam->Get(reinterpret_cast<BYTE*>(&vsr), sizeof(ST_VSR_MUTILPLE_STREAMS));
	TRACEINTO <<  " Receiving Multi VSR from VB, party Name["  << PARTYNAME << "] Num of streams[" << vsr.num_vsrs_streams << "]";
	m_pSipCntl -> SendMultiVsr(vsr);
}

/////////////////////////////////////////////////////////////////////////////

void CSipParty::OnTransHandleIceConnectivityCheckComplete (CSegment * pParam)
{
    ICE_CHECK_COMPLETE_IND_S* pConnCheckStruct = (ICE_CHECK_COMPLETE_IND_S*) pParam->GetPtr(1);
    int status = pConnCheckStruct->status;
    PTRACE2INT (eLevelInfoNormal, "CSipParty::OnTransHandleIceConnectivityCheckComplete - ", status);

	const CSipCaps* pRemoteCaps	= m_pSipCntl->GetLastRemoteCaps();
	const CSipCaps* pLocalCaps	= m_pSipCntl->GetLocalCaps();

	CapEnum protocol = eUnknownAlgorithemCapCode;
	if (m_pCurrentMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople))
		protocol = (CapEnum)m_pCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople);


    m_bIsRcvCheckCompleteAndNeedToStartIVR = TRUE;

    if (STATUS_OK == status)
    {
    	mcTransportAddress localIp;
    	memset(&localIp,0,sizeof(mcTransportAddress));
    	mcTransportAddress remoteIp;
    	memset(&remoteIp,0,sizeof(mcTransportAddress));
    	m_pSipCntl->GetLocalMediaIpAsTrAddr(localIp);

    	remoteIp.transportType = eTransportTypeUdp;
    	localIp.transportType = eTransportTypeUdp;

        mcTransportAddress IceLocalIp;
        mcTransportAddress IceRemoteIp;
        memset(&IceRemoteIp,0,sizeof(mcTransportAddress));
        memset(&IceLocalIp,0,sizeof(mcTransportAddress));
        BYTE bIsTransmit;
        EIpChannelType eChanType;
        CMedString str;

        //Check connectivity test result (local or remote)
        char* localConnectType = new char[CandidateTypeLen];
        char* rmtConnectType = new char[CandidateTypeLen];

        memset (localConnectType, '\0', CandidateTypeLen);
        memset (rmtConnectType, '\0', CandidateTypeLen);
		memset (localConnectType, '\0', CandidateTypeLen);
		memset (rmtConnectType, '\0', CandidateTypeLen);

		// AN - vngr-17005 - Dial out to federated HDX fail because AFE return that remote use its local address and RMX use relay address
		// The previous strncpy copy the remote candidate type and that caused RMX to roll back to non ICE when HDX is federated and to connect with problem.
		// The current strncpy copy the local candidate type and if its relay we continue the call as ICE call.
		strncpy (localConnectType, pConnCheckStruct->chosen_candidates[0].local_candidate.type, CandidateTypeLen-1);
		strncpy (rmtConnectType, pConnCheckStruct->chosen_candidates[0].remote_candidate.type, CandidateTypeLen-1);

		if ((!strncmp (localConnectType, "local", CandidateTypeLen) && (!strncmp (rmtConnectType, "local", CandidateTypeLen))))
		{
			m_pSipCntl->SetIceConnectivityStatus (eIceConnectedLocal);
			PTRACE(eLevelInfoNormal, "SetIceConnectivityStatus: eIceConnectedLocal");
		}
        else
        {
            m_pSipCntl->SetIceConnectivityStatus (eIceConnectedRemote);
            PTRACE(eLevelInfoNormal, "SetIceConnectivityStatus: eIceConnectedRemote");
        }

        str << "CSipParty::OnTransHandleIceConnectivityCheckComplete local type:" << localConnectType << " remote type:" << rmtConnectType << " connectivity status: " << m_pSipCntl->GetIceConnectivityStatus() << " ip received ";

        PDELETEA(localConnectType);
        PDELETEA(rmtConnectType);

    	cmCapDataType mediaType;
    	ERoleLabel eRole;
        for(int i=0 ; i<NumOfMediaTypes-1; i++)
        {
        	GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

				m_pSipCntl->GetOriginalRmtIpAddress(mediaType,remoteIp,eRole);
			//	m_pSipCntl->GetRemoteMediaIpAsTrAddr(globalMediaArr[i],remoteIp);

				cmCapDataType mediaType = (cmCapDataType)(pConnCheckStruct->chosen_candidates[i].local_candidate.mediaType);

				if(::IsValidIpV4Address(pConnCheckStruct->chosen_candidates[i].remote_candidate.ip) && ::IsValidIpV4Address(pConnCheckStruct->chosen_candidates[i].local_candidate.ip))
				{
					::stringToIpV4 (&IceRemoteIp, pConnCheckStruct->chosen_candidates[i].remote_candidate.ip);
					::stringToIpV4 (&IceLocalIp, pConnCheckStruct->chosen_candidates[i].local_candidate.ip);
					IceRemoteIp.transportType = pConnCheckStruct->chosen_candidates[i].remote_candidate.transportType;
					IceLocalIp.transportType = pConnCheckStruct->chosen_candidates[i].local_candidate.transportType;
			    	remoteIp.transportType = IceRemoteIp.transportType;
					localIp.transportType = IceLocalIp.transportType;
				}
				if (mediaType && (!isApiTaNull(&IceRemoteIp) || !isApiTaNull(&IceLocalIp)))
					str << "\n media type - " << pConnCheckStruct->chosen_candidates[i].local_candidate.mediaType
						<< " local addr - " << pConnCheckStruct->chosen_candidates[i].local_candidate.ip << ":" << pConnCheckStruct->chosen_candidates[i].local_candidate.port
						<< " remote addr - " << pConnCheckStruct->chosen_candidates[i].remote_candidate.ip << ":" << pConnCheckStruct->chosen_candidates[i].remote_candidate.port;
				for (int j=0; j<2; j++)
				{
					if (cmCapEmpty != mediaType)
					{
						bIsTransmit = (globalDirectionArr[j] == cmCapTransmit);
						eChanType   = ::CalcChannelType(mediaType, bIsTransmit, kRolePeople);
						if (m_pCurrentMode->IsMediaOn(mediaType, globalDirectionArr[j])&& !(isApiTaNull(&IceRemoteIp) || isApiTaNull(&IceLocalIp)))
						{
							m_pSipCntl->SetChosenCandidates(pConnCheckStruct->chosen_candidates[i],mediaType);
							EIceConnectionType ChosenConnectionType = m_pSipCntl->GetMediaChosenRemoteType(mediaType);
							DWORD actualRate	= 0xFFFFFFFF;
							actualRate	= pConnCheckStruct->allocatedBandwidth;
							if(actualRate!= 0xFFFFFFFF && actualRate > 0)
								actualRate = actualRate*10;



					    	if ( actualRate!= 0xFFFFFFFF && actualRate > 0 &&((cmCapDirection)globalDirectionArr[j] == cmCapTransmit) && (cmCapVideo == mediaType)
					    			&& pRemoteCaps && (actualRate < pRemoteCaps->GetMaxVideoBitRate(protocol))
					    			&& pLocalCaps && (actualRate < pLocalCaps->GetMaxVideoBitRate(protocol)))
					    	{
					    		if ((m_pCurrentMode->GetConfType() == kVideoSwitch) || (m_pCurrentMode->GetConfType() == kVSW_Fixed))
					    			{
					    				// VSW:
					    				BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
					    				if(!bEnableFlowControlVSW)
					    				{
					    					PTRACE(eLevelError,"CSipParty::OnChangeRate: SUPPORT_VSW_FLOW_CONTROL is false");
					    					return;
					    				}
					    			        if ( (actualRate < (VSW_FLOW_CONTROL_RATE_THRESHOLD * (m_pCurrentMode->GetVideoBitRate((cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople)))) &&
					    				     (m_pSipCntl->GetRemoteIdent() != PolycomMGC) )
					    			        {
					    				         PTRACE2(eLevelError,"CSipParty::OnChangeRate: Flow control bit rate is lower than the threshold,  Name - ",PARTYNAME);
					    				         return;
					    			        }
					    				m_pCurrentMode->SetFlowControlRateConstraint(actualRate);
					    			}
					    			else
					    			{
					    				// CP:
					    				m_pCurrentMode->SetVideoBitRate(actualRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)kRolePeople);
					    			}
					    		m_pTargetMode->SetVideoBitRate(actualRate, cmCapTransmit , (ERoleLabel)kRolePeople);
					    		m_pConfApi->UpdatePartyControlOnNewRate(GetPartyId(),actualRate , (WORD)cmCapTransmit ,(WORD)kRolePeople);
					    	}
							localIp.port			= m_pSipCntl->GetPort(mediaType, cmCapReceive, eRole);
							//    remoteIp.port			= m_pSipCntl->GetPort(globalMediaArr[i], cmCapTransmit);

							IceLocalIp.port			= pConnCheckStruct->chosen_candidates[i].local_candidate.port;
							IceRemoteIp.port			= pConnCheckStruct->chosen_candidates[i].remote_candidate.port;

							SetPartyMonitorBaseParamsAndConnectChannel(eChanType, actualRate,&remoteIp,&localIp,(DWORD)eUnknownAlgorithemCapCode,0,0,1,&IceRemoteIp,&IceLocalIp,ChosenConnectionType);
						}
					}
				}

				memset(&remoteIp,0,sizeof(mcTransportAddress));
				memset(&localIp,0,sizeof(mcTransportAddress));
				memset(&IceRemoteIp,0,sizeof(mcTransportAddress));
				memset(&IceLocalIp,0,sizeof(mcTransportAddress));

        }

        PTRACE (eLevelInfoNormal, str.GetString());

        if (m_bIsRtvPreferenceMsgSent)
        {
        	PTRACE(eLevelInfoNormal,"ICE connected, resending Preference message");
        	this->OnPartyCntlStartVideoPreference(NULL);
        }

        //Inorder to start IVR we must wait for Connectivity check and Call connect (video channels should open).
        if(m_bIsCallConnected)
        	StartIvr();
     //   else
        //	m_bIsRcvCheckCompleteAndNeedToStartIVR = TRUE;
    }
    else
    {
    	CMedString str;
    	str << "Sip call terminated:  "<< PARTYNAME;
    	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, CALL_END_DUE_TO_ICE_CONNECTIVITY_CHECK_FAILURE, MAJOR_ERROR_LEVEL,str.GetString(), TRUE);

    	TellConfOnDisconnecting (SIP_NO_ADDR_FOR_MEDIA);
    }
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SipConfNIDConfirmationInd(DWORD sts)
{
	TRACEINTO<<"CSipParty::SipConfNIDConfirmationInd- Start";

	if (m_ivrCtrl)
		m_ivrCtrl->SipConfNIDConfirmationInd(sts);
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyCntlUpdatePresentationOutStream(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyCntlUpdatePresentationOutStream : Name - ",PARTYNAME);

	BYTE IsAsSipContentChannelOpen = TRUE;

	CCommConf* pCommConf 	= ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
	if(pCommConf)
		if(pCommConf->GetIsAsSipContent())
		{
			EConnectionState ChannelState = m_pSipCntl->GetChannelConnectionState(cmCapVideo, cmCapTransmit, kRolePresentation);
			if((ChannelState == kUnknown) || (ChannelState == kConnecting))
			{
				IsAsSipContentChannelOpen = FALSE;
			}

		}


	if(IsActiveTransaction() || !IsAsSipContentChannelOpen)
	{
	    m_bIsNeedToUpdateContentOnTheTransactionEnd = true;
	    PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyCntlUpdatePresentationOutStream will update later (after trans ends): Name - ",PARTYNAME);
	}
	else
	    m_pSipCntl->UpdatePresentationOutStream();
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyUpdatedPresentationOutStream(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyUpdatedPresentationOutStream : Name - ",PARTYNAME);
	if(GetIsTipCall() && m_tipPartyType == eTipMasterCenter )
	{
		if( m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit) )
		{
			CSipChannel *pChannel = NULL;
			CSipCall *pCall = m_pSipCntl->GetCallObj();
			if(pCall)
			{
				pChannel = pCall->GetChannel(VIDEO_CONT_OUT);
				if(pChannel &&  pChannel->IsMuted())
				{
					PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyUpdatedPresentationOutStream unmute content: Name - ",PARTYNAME);
					m_pSipCntl->SendStreamOnReq(pChannel);
				}
			}
		}

	}
	m_pConfApi->UpdatePresentationRes(GetPartyId());
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnContentProviderIdentityTimer(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnContentProviderIdentityTimer");
	// Send role provider identity:
	CSegment* pSeg = new CSegment;
	*pSeg << (BYTE)m_mcuNum ;
	*pSeg << (BYTE)m_termNum ;
	SpreadAllH239Msgs(pSeg, eMsgIn, FALSE, ROLE_PROVIDER_IDENTITY);
	POBJDELETE(pSeg);
	// Send channel active:
	BYTE channelID = 0;
	BYTE bIsActive = 1;
	m_pConfApi->ContentMediaProducerStatus(this, channelID, bIsActive);

	DWORD nextTimerVal = 10;
	if (m_ProviderMessageCounter<4)
	{
		m_ProviderMessageCounter++;
		nextTimerVal = 1; // 1 second interval in the first 4 times (because of EPs bug of long sych time)
	}

	StartTimer(SIP_CONTENT_PROVIDER_TOUT, nextTimerVal*SECOND);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::DeleteProviderIdentityTimerIfNeeded(DWORD opcode, BYTE mcuNum, BYTE terminalNum)
{
	if ((!m_bBfcpConnected && !GetIsTipCall())
			|| (opcode==CONTENT_NO_ROLE_PROVIDER)
			|| (opcode==PARTY_TOKEN_RELEASE)
			|| (opcode==CONTENT_ROLE_TOKEN_WITHDRAW)
			|| (opcode==CONTENT_ROLE_TOKEN_ACQUIRE_NAK)
			|| (opcode==CONTENT_ROLE_TOKEN_RELEASE_ACK)
			|| ((opcode==CONTENT_ROLE_PROVIDER_IDENTITY) && !(m_mcuNum==mcuNum && m_termNum==terminalNum)))
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::DeleteProviderIdentityTimerIfNeeded : opcode=", opcode);
		DeleteTimer(SIP_CONTENT_PROVIDER_TOUT);
	}

}

/////////////////////////////////////////////////////////////////////////////
/* Delay bfcp ACK till bfcp TX channel is open */
void CSipParty::OnPartyBfcpMsgDelayAckTimeout()//BRIDGE-13253
{
	if (IsValidTimer(SIP_PARTY_BFCP_DELAY_ACK_MSG))
		DeleteTimer(SIP_PARTY_BFCP_DELAY_ACK_MSG);

	StartTimer(SIP_PARTY_BFCP_DELAY_ACK_MSG, 1 * SECOND); /* 1 second */
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyBfcpMsgDelayAck()
{

	CSegment pSeg;

	pSeg << (DWORD)CONTENT_ROLE_BFCP_HELLO;
	pSeg << (DWORD)bfcp_msg_status_ok;
	pSeg << (BYTE)0;
	pSeg << (BYTE)0;

	OnPartyBfcpMsgInd(&pSeg);

}

////////////////////////////////////////////////////////////////////////////////
void CSipParty::SendIntraToEP()
{
	/* Send iframe for TX content channel */
	PTRACE(eLevelInfoNormal,"CSipParty::SendIntraToEP");
	m_pSipCntl->SendH230FastUpdateToParty(kIpContentChnlType, cmCapTransmit);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyBfcpMsgInd(CSegment* pParam)
{
	DWORD opcode = 0, status = 0;
	*pParam >> opcode >> status;
	CMedString msg;
	msg << "opcode=" << opcode << ", status=" << status << ", Name=" << PARTYNAME;
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : ",msg.GetString());

	m_bBfcpConnected = TRUE;

	if (status != bfcp_msg_status_ok)
	{
		DBGPASSERT(status);
	}
	else if (opcode == CONTENT_ROLE_BFCP_HELLO)
	{
		CSipCall *pCall = m_pSipCntl->GetCallObj();
		CSipChannel* bfcpOutCh =  pCall->GetChannel(BFCP_OUT);

		if (bfcpOutCh)
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : receive CONTENT_ROLE_BFCP_HELLO. Send CONTENT_ROLE_BFCP_HELLO_ACK");
			m_pSipCntl->SendBfcpMessageReq(CONTENT_ROLE_BFCP_HELLO_ACK, m_mcuNum, m_termNum);
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : postpone CONTENT_ROLE_BFCP_HELLO_ACK till BFCP TX channel is open");
			if (IsValidTimer(SIP_PARTY_BFCP_DELAY_ACK_MSG))
				DeleteTimer(SIP_PARTY_BFCP_DELAY_ACK_MSG);
			StartTimer(SIP_PARTY_BFCP_DELAY_ACK_MSG, 1 * SECOND); /* 1 second */
		}
	}
	else if (opcode == CONTENT_ROLE_BFCP_GOODBYE)
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : receive CONTENT_ROLE_BFCP_BYE. Send CONTENT_ROLE_BFCP_BYE_ACK");

		m_bBfcpConnected = FALSE;

		m_pSipCntl->SendBfcpMessageReq(CONTENT_ROLE_BFCP_GOODBYE_ACK, m_mcuNum, m_termNum);
	}
	else if ((opcode == CONTENT_ROLE_BFCP_FLOOR_REQ_STATUS_ACK) ||
			 (opcode == CONTENT_ROLE_BFCP_ERROR_ACK)			||
			 (opcode == CONTENT_ROLE_BFCP_FLOOR_STATUS_ACK)		||
			 (opcode == CONTENT_ROLE_BFCP_GOODBYE_ACK))
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : receive ACK opcode: ", opcode);
		/**
		 * Incase call is VEQ->VMR, Tandberg is having trouble to sync on the content
		 * in case it is being transmitted while Tandberg joins the conference.
		 * This is a patch for bridge-7236
		 * Send only one time when the Tandberg EP joins the conference.
		 * *  */
		if ((opcode == CONTENT_ROLE_BFCP_FLOOR_STATUS_ACK) &&
			(m_pSipCntl->GetRemoteIdent() == TandbergEp) &&
			m_pSipCntl->GetPlcmRequireMask() & m_plcmRequireAudio &&
			m_numOfIframesSent < 2)
		{
			const CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
			if (pCommConf)
			{
				m_numOfIframesSent++;
				/* if token is being held value will not be 0xFFFFFFFF */
				if (pCommConf->GetEPCContentSourceId() != 0xFFFFFFFF)
				{
					/* for Tandberg after hold resume wait 8 seconds and send intra frame */
					StartTimer(CONTENT_INTRA_TOUT,  8*SECOND);
				}
			}
		}

	}
	else if (opcode == CONTENT_ROLE_BFCP_ERROR)
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : receive CONTENT_ROLE_BFCP_ERROR");//. Send CONTENT_ROLE_BFCP_ERROR_ACK");
		//m_pSipCntl->SendBfcpMessageReq(CONTENT_ROLE_BFCP_ERROR_ACK, m_mcuNum, m_termNum);
	}
	else if (opcode == PARTY_BFCP_TOKEN_QUERY)
	{

		/*
		 * This is a patch we encountered during the investigation of bridge-7201
		 * We need to respond to Tandberg EPs with floor status, and send an iframe
		 * or the content won't be displayed by the EP.
		 * */
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : receive PARTY_BFCP_TOKEN_QUERY. Send floor status and an iframe ");
		//m_pSipCntl->SendBfcpMessageReq(CONTENT_ROLE_PROVIDER_IDENTITY, m_mcuNum, m_termNum);
		const CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
		if (pCommConf)
		{
			/* if token is being held value will not be 0xFFFFFFFF */
			DWORD isTokenBeingHeld = pCommConf->GetEPCContentSourceId();
			DWORD opcodeToSend;

			m_isActiveContentForTip = (isTokenBeingHeld == 0xFFFFFFFF)?  FALSE : TRUE ; //BRIDGE-12961
			opcodeToSend = (isTokenBeingHeld == 0xFFFFFFFF)?  CONTENT_NO_ROLE_PROVIDER : CONTENT_ROLE_PROVIDER_IDENTITY ;
			m_pSipCntl->SendBfcpMessageReq(opcodeToSend, m_mcuNum, m_termNum);

			/* for Tandberg after hold resume wait 2 seconds and send intra frame */
			StartTimer(CONTENT_INTRA_TOUT,  2*SECOND);
		}
	}
	else
		SpreadAllH239Msgs(pParam, eMsgIn, FALSE, opcode);


	if(0 != GetBfcpCachedMsgType())
	{
		DWORD currentTime = SystemGetTickCount().GetSeconds();
		if(currentTime - GetBfcpCachedMsgTime() < 500)
		{
			if(opcode == CONTENT_ROLE_BFCP_HELLO)
			{
				m_pSipCntl->SendBfcpMessageReq(GetBfcpCachedMsgType(), m_mcuNum, m_termNum);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : Clear Cached BFCP message!! -- NOT sent, we need HELLO!!");
			}
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnPartyBfcpMsgInd : Clear Cached BFCP message!! -- Time tooooooo long");
		}

		SetBfcpCachedMsgType( 0);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyBfcpTransportInd(CSegment* pParam)
{
	/*DWORD status = 0;
	*pParam >> status;
	CMedString msg;
	msg << ", status=" << status << ", Name=" << PARTYNAME;
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyBfcpTransportInd : ",msg.GetString());

	mcTransportAddress	bfcpLocalAddress;
	mcTransportAddress	bfcpRemoteAddress;

	if (status == bfcp_msg_status_connected)
	{
		m_bBfcpConnected = TRUE;

		m_pSipCntl->GetBfcpAddress(bfcpLocalAddress, bfcpRemoteAddress);
		SetPartyMonitorBaseParamsAndConnectChannel(BFCP_IN, 0, &bfcpLocalAddress, &bfcpRemoteAddress, (DWORD)eUnknownAlgorithemCapCode);
		SetPartyMonitorBaseParamsAndConnectChannel(BFCP_OUT, 0, &bfcpLocalAddress, &bfcpRemoteAddress, (DWORD)eUnknownAlgorithemCapCode);

		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyBfcpTransportInd : BFCP connected");
	}
	else
	{
		DBGPASSERT(status);
	}*/
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTipCntlContentMsgInd(CSegment* pParam)
{
	DWORD opcode = 0, status = 0;
	*pParam >> opcode >> status;
	CMedString msg;
	msg << "opcode=" << opcode << ", status=" << status << ", Name=" << PARTYNAME;
	PTRACE2(eLevelInfoNormal,"CSipParty::OnTipCntlContentMsgInd : ",msg.GetString());

	if (status != STATUS_OK)
	{
		DBGPASSERT(status);
	}
	else if (!GetIsTipCall())
	{
		DBGPASSERT(1);
	}
	else
		SpreadAllH239Msgs(pParam, eMsgIn, FALSE, opcode);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnContentBrdgTokenMsg(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal,"CSipParty::OnContentBrdgTokenMsg : Name - ",PARTYNAME);

 	DWORD opcode;
	*pParam >> opcode;

	PTRACE2INT(eLevelInfoNormal,"CSipParty::OnContentBrdgTokenMsg : opcode - ",opcode);

	EMsgDirection direction = eMsgOut;
	SpreadAllH239Msgs(pParam,direction,0,opcode);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SpreadAllH239Msgs(CSegment* pParam, EMsgDirection direction, BYTE isOldTokenMsg, DWORD opcode, EMsgStatus msgStat)
{
	CConfParty* pConfParty = GetConfPartyNonConst();
	DBGPASSERT_AND_RETURN(!pConfParty);

	CTokenMsg* pTokenMsg = NULL;
	EMsgStatus eMsgStat = eMsgInvalid;
	EHwStreamState eStreamState = eHwStreamStateNone;
	BYTE mcuNum = 0, terminalNum = 0, lable = LABEL_CONTENT;
	BYTE IsMsgToParty = false;
	int symmetryBreaking = 0;

	if (m_bBfcpConnected || GetIsTipCall())
	{
		CMedString msg;
		msg << "CSipParty::SpreadAllH239Msgs : ";
		//If this is first msg after update we don't need to go through TMM
		// Need to send it to its destination.
		if (isOldTokenMsg)
		{
			msg <<  "Old Opcode From list - " << msgStat;
			eMsgStat = msgStat;
		}
		else
		{
			msg <<  "New Opcode: Opcode - " << opcode << ", ContentInStreamState - " << (DWORD)m_pSipCntl->GetContentInStreamState();

			// 1. Set correct stream state for TMM
			eStreamState = SetCorrectStreamStateForTMM();

			//Send new Token Msg to TMM
			pTokenMsg = new CTokenMsg(opcode, direction, pParam);
			eMsgStat = m_pH239TokenMsgMngr->NewTokenMsg(pTokenMsg, eStreamState);
		}
		msg <<  ", HwStreamState - " << eStreamState << ", MsgStat - " << eMsgStat << ", Name - ";

		PTRACE2(eLevelInfoNormal,msg.GetString(),PARTYNAME);

		CMedString str;
		str << "CSipParty::SpreadAllH239Msgs : ";

		switch (opcode)
		{
			// From Party to CB
			case PARTY_TOKEN_ACQUIRE:
			{
				if(eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum >> terminalNum;
					str << "PARTY_TOKEN_ACQUIRE:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_ACQUIRE,this, mcuNum, terminalNum, lable, (BYTE)symmetryBreaking);
				}
				break;
			}
			// From CB to Party
			case CONTENT_ROLE_TOKEN_ACQUIRE_NAK:
			{
				if(eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum >> terminalNum;
					str << "CONTENT_ROLE_TOKEN_ACQUIRE_NAK:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					IsMsgToParty = true;
				}
				break;
			}
			// From CB to Party
			case CONTENT_ROLE_TOKEN_ACQUIRE_ACK:
			{
				if (eMsgStat != eMsgInvalid)
				{
					*pParam >> mcuNum >> terminalNum;
					str << "CONTENT_ROLE_TOKEN_ACQUIRE_ACK: ";
					if (pConfParty -> GetTokenRecapCollisionDetection() != etrcdRecapInProgress)
					{
						//================================
						// A reinvite is not in progress
						//================================
						if (!m_bIsDelayedOnce && !(m_isActiveContentForTip && m_pSipCntl->GetIsContentSpeaker() == FALSE))
						{
							str << "set Token Handling In Progress to pend another change mode.";
						pConfParty -> SetTokenRecapCollisionDetection(etrcdTokenHandlingInProgress);
							m_bIsDelayedOnce = true;
						}
						if(eMsgStat == eMsgFree)
						{
							str << "start content token handling.";
						IsMsgToParty = true;
							m_bIsDelayedOnce = false;
						m_ProviderMessageCounter = 0;
						m_pSipCntl->SetIsContentSpeaker(YES);//Notice:here in case of snatch content we should have used a function similar to OnPartyContentSpeakerChange on H323!!!!!!!
						StartTimer(SIP_CONTENT_PROVIDER_TOUT, 10); // first time after token acquire: 100 millisecond.


						DWORD curConfContRateTarget = m_pTargetMode->GetContentBitRate(cmCapReceive);
						PTRACE2INT(eLevelInfoNormal,"CSipParty::SpreadAllH239Msgs  curConfContRateTarget", curConfContRateTarget);
						PTRACE2INT(eLevelInfoNormal,"CSipParty::SpreadAllH239Msgs curConfContRateCurrent", m_pTargetMode->GetContentBitRate(cmCapReceive));
						m_pSipCntl->OnSIPPartyContentSpeakerChange(true, curConfContRateTarget); // We want to send the flow control on token snatch weather or not the EP content rate is higher than the current content rate
						//StartTimer(SIP_CONTENT_PROVIDER_TOUT, 10);
						}

					}
					else
					{
						//====================================================
						// A reinvite is in progress, pending token handling
						//====================================================
						pConfParty -> PendTokenRecapDueToCollisionDetection();
						str << "--PENDED-- due to collision with recap.";
						*pParam << mcuNum << terminalNum;
						m_pendedToken			= *pParam;
						m_pendedTokenOpcode		= opcode;
						m_pendedTokenDirection	= direction;
						m_pendedTokenIsOldToken	= isOldTokenMsg;
					}
					str << " MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
				}
				break;
			}
			// From CB to Party
			case CONTENT_ROLE_TOKEN_WITHDRAW:
			{
				if(eMsgStat == eMsgFree)
				{
					BYTE  isSpeakerChange;
					*pParam >> isSpeakerChange;
					*pParam >> mcuNum >> terminalNum;
					str << "CONTENT_ROLE_TOKEN_WITHDRAW:  isSpeakerChange: " <<isSpeakerChange<<" MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					IsMsgToParty = true;
					m_pSipCntl->SetIsContentSpeaker(NO);//Notice:here in case of snatch content we should have used a function similar to OnPartyContentSpeakerChange on H323!!!!!!!
					if (!GetIsTipCall()) // Tip call wait for remote ack
						m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_WITHDRAW_ACK,this, m_mcuNum, m_termNum, lable);

					if (isSpeakerChange && !m_pSipCntl->GetIsMrcCall())  //send flow control to old content speaker
					{
						DWORD newPeopleRate = m_pCurrentMode->GetTotalVideoRate();
						SendFlowControlMessage(mainType, cmCapReceive, newPeopleRate);
					}
					else if (m_pSipCntl->GetIsMrcCall())
	            				PTRACE(eLevelInfoNormal,"CSipParty::SpreadAllH239Msgs : For MRC call not needed to send flow control for the video");	           
				}
				break;
			}
			// From Party to CB (for TIP call)
			case PARTY_TOKEN_WITHDRAW_ACK:
			{
				if(eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum >> terminalNum;
					str << "PARTY_TOKEN_WITHDRAW_ACK: MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_WITHDRAW_ACK,this, mcuNum, terminalNum, lable);
				}
				break;
			}
			// From Party to CB
			case PARTY_TOKEN_RELEASE:
			{
				if (eMsgStat != eMsgInvalid)
				{
					*pParam >> mcuNum >> terminalNum;
					str << "PARTY_TOKEN_RELEASE: ";
					if (pConfParty -> GetTokenRecapCollisionDetection() != etrcdRecapInProgress)
					{
						//==============================
						// Reinvite is not in progress
						//==============================
						if (!m_bIsDelayedOnce && !(m_isActiveContentForTip && m_pSipCntl->GetIsContentSpeaker() == FALSE))
						{
							str << "set Token Handling In Progress to pend another change mode.";
						pConfParty -> SetTokenRecapCollisionDetection(etrcdTokenHandlingInProgress);
							m_bIsDelayedOnce = true;
						}
						if(eMsgStat == eMsgFree)
						{
							str << "start content token handling.";
							m_bIsDelayedOnce = false;
						m_pConfApi->SendContentTokenMessage(PARTY_TOKEN_RELEASE,this, mcuNum, terminalNum, lable);
						m_isActiveContentForTip = FALSE;
						}
					}
					else
					{
						//====================================================
						// A reinvite is in progress, pending token handling
						//====================================================
						pConfParty -> PendTokenRecapDueToCollisionDetection();
						str << "--PENDED-- due to collision with recap.";
						*pParam << mcuNum << terminalNum;
						m_pendedToken			= *pParam;
						m_pendedTokenOpcode		= opcode;
						m_pendedTokenDirection	= direction;
						m_pendedTokenIsOldToken	= isOldTokenMsg;
					}
					str << " MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
				}
				break;
			}
			// From CB to Party
			case CONTENT_ROLE_TOKEN_RELEASE_ACK:
			{
				if(eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum >> terminalNum;
					str << "CONTENT_ROLE_TOKEN_RELEASE_ACK: MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					IsMsgToParty = true;
					m_isActiveContentForTip = FALSE;
				}
				break;
			}
			// From Party to CB
			case ROLE_PROVIDER_IDENTITY:
			{
				BYTE tempSize = 0;
				BYTE* pData = NULL;

				if(eMsgStat == eMsgFree)
				{
//					terminalLabel = MbeBytesToInt(pParam);
//					channelId = MbeBytesToInt(pParam);
//					terminalNum = terminalLabel%TERMINAL_LABEL_MULTIPLIER;
//					mcuNum = (terminalLabel-terminalNum)/TERMINAL_LABEL_MULTIPLIER;
					*pParam >> mcuNum >> terminalNum;
					str << "ROLE_PROVIDER_IDENTITY: MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					m_pConfApi->ContentTokenRoleProviderMessage( this, mcuNum, terminalNum, lable, tempSize, pData);
				}
				break;
			}
			// From CB to Party
			case CONTENT_ROLE_PROVIDER_IDENTITY:
			{
				if(eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum
							>> terminalNum;

					str << "CONTENT_ROLE_PROVIDER_IDENTITY:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					IsMsgToParty = true;
					m_isActiveContentForTip = TRUE;

					/** work around for Avaya - (AVA-1350,AVA-1349)
					 *  Due to the fact they do not support INTRA request using INFO messages,
					 *  we'll start a timer and send 4 intra requests every 3 seconds
					 *  */
					if (m_pSipCntl->GetRemoteIdent() == AvayaEP) {

						WORD currentContentSenderID = (mcuNum<<8) + terminalNum;

						m_repeatContentIntraNumber = GetSystemCfgFlagInt<DWORD>(m_serviceId, CFG_KEY_REPEAT_INTRA_NUMBER);
						m_repeatContentIntraNumber= m_repeatContentIntraNumber ? m_repeatContentIntraNumber : SIP_MAX_INTRA_NUM;

						m_contentIntraInterval = GetSystemCfgFlagInt<DWORD>(m_serviceId, CFG_KEY_SIP_FAST_UPDATE_INTERVAL_ENV);
						m_contentIntraInterval = m_contentIntraInterval ? m_contentIntraInterval : SIP_INTRA_INTERVAL;

						str << " currentContentSenderID: " << currentContentSenderID << " lastActiveContentSenderID:" << m_lastActiveContentSenderID;

						if (currentContentSenderID != m_lastActiveContentSenderID) {

							m_lastActiveContentSenderID = currentContentSenderID;
							StartTimer(CONTENTVIDSIMINDREFRESH, m_contentIntraInterval*SECOND);
							str << " ** simulate receiving INFO request **";
						}
					}
				}
				break;
			}
			// From CB to Party
			case CONTENT_MEDIA_PRODUCER_STATUS:
			{

				BYTE  status,ChannelId;
				*pParam >> ChannelId
						>> status;
				eMsgStat = eMsgFree;
				str << " CONTENT_MEDIA_PRODUCER_STATUS Status is " << status;
				if (!status) // inactive
				{
					opcode = CONTENT_NO_ROLE_PROVIDER;
					IsMsgToParty = true;
					m_isActiveContentForTip = FALSE;
				}

				break;
			}
			// From Party to CB
			case PARTY_BFCP_TOKEN_QUERY:
			{
				BYTE tempSize 	= 0;
				BYTE* pData 	= NULL;

				if(eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum >> terminalNum;
					str << "PARTY_BFCP_TOKEN_QUERY:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					m_pConfApi->SendBfcpContentTokenQueryMessage(this, mcuNum, terminalNum, lable, tempSize, pData);
				}
				break;
			}

			// From CB to Party
			case CONTENT_NO_ROLE_PROVIDER:
			{
				if(eMsgStat == eMsgFree)
				{
					*pParam >> mcuNum
							>> terminalNum;

					str << "CONTENT_NO_ROLE_PROVIDER:  MCUNumber: "<<mcuNum<<" terminalNumber: "<<terminalNum;
					IsMsgToParty = true;
				}

				break;
			}

			default:
			{
				str << " Unknown Sub Message Identifier ";
				break;
			}

		}//switch

		if(eMsgStat != eMsgFree)
		{
			//In case of Delay- we need to check if we need to send
			if(eMsgStat == eMsgDelayed)
			{
				SendContentOnOff();
				PTRACE2(eLevelInfoNormal,"CSipParty::SpreadAllH239Msgs : Delay msg case , Name - ", PARTYNAME);
			}
			else if (opcode != CONTENT_ROLE_BFCP_HELLO_ACK)
			{	// eMsgInvalid case
				PASSERTMSG(opcode,"CSipParty::SpreadAllH239Msgs: Msg invalid");
			}
			else
			    PTRACE2(eLevelInfoNormal,"CSipParty::SpreadAllH239Msgs : skip Hello Ack from client , Name - ", PARTYNAME);
		}

		PTRACE(eLevelInfoNormal,str.GetString());

		if (IsMsgToParty)
		{
			m_pSipCntl->SendTokenMessageReq(opcode, mcuNum, terminalNum);
		}

		POBJDELETE(pTokenMsg);
	}
	else if(((opcode == CONTENT_NO_ROLE_PROVIDER)||(opcode == CONTENT_ROLE_PROVIDER_IDENTITY))
		&& (m_pSipCntl->GetBfcpType() == eMediaLineSubTypeUdpBfcp)
		&& (!m_bBfcpConnected))
		// Not connected, AND FloorStatus-BRIDGE-5956
	{
		PTRACE2(eLevelError,"CSipParty::SpreadAllH239Msgs : Cache FloorStatus: BFCP isn't connected - ",PARTYNAME);
		SetBfcpCachedMsgTime();
		SetBfcpCachedMsgType(opcode);
	}
	else
		PTRACE2(eLevelError,"CSipParty::SpreadAllH239Msgs : BFCP isn't connected - ",PARTYNAME);


	DeleteProviderIdentityTimerIfNeeded(opcode, mcuNum, terminalNum);
}

/////////////////////////////////////////////////////////////////////////////
EHwStreamState CSipParty::SetCorrectStreamStateForTMM()
{
	EHwStreamState eStreamStateForTmm = eHwStreamStateNone;

	switch (m_pSipCntl->GetContentInStreamState())
	{
		case eStreamOn:
		case eSendStreamOn:
		case eSendStreamOff:
		{
			eStreamStateForTmm = eHwStreamStateOn;
			break;
		}

		case eStreamOff:
		{
			eStreamStateForTmm = eHwStreamStateOff;
			break;
		}
		case eNoChannel:
		case eWaitToSendStreamOn:
		{
			eStreamStateForTmm = eHwStreamStateNone;
			break;
		}
		default:
		{
			PASSERTMSG( (DWORD)m_pSipCntl->GetContentInStreamState(),"CSipParty::SetCorrectStreamStateForTMM - Stream state not valid ");
		}
	}
	return eStreamStateForTmm;
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendContentOnOff()
{
	if (m_pSipCntl->GetContentInStreamState() == eSendStreamOn || m_pSipCntl->GetContentInStreamState() == eSendStreamOff)
	{
		// In this case we do nothing
		// Since we are in the middle of another Content On/Off req from the RTP
	    PTRACE2(eLevelInfoNormal,"CSipParty::SendContentOnOff : Do nothing!! In the middle of other Contnet ON/OFF, Name - ", PARTYNAME);
		return;
	}
	// Updating the stream status
	if (m_pSipCntl->GetContentInStreamState() == eStreamOn)
		m_pSipCntl->SetContentInStreamState(eSendStreamOff);
	else if (m_pSipCntl->GetContentInStreamState() == eStreamOff)
		m_pSipCntl->SetContentInStreamState(eSendStreamOn);

	// Sending Content On/Off to the RTP.
	m_pSipCntl->SendContentOnOffReqToRtp();
	StartTimer(RTPCONTENTACKTOUT,250);// =SECOND*2.5
}
/////////////////////////////////////////////////////////////////////////////
//After ack for Content ON/OFF we need to send Update to TMM
//Update give us the tokens list
//After update we need to send the first msg to it's destination - party or CB
void CSipParty::OnRtpAckForContentOnOff(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnRtpAckForContentOnOff - Name - ",PARTYNAME);
	DWORD status;
	EHwStreamState eStreamStateForTmm;
	EMsgStatus eMsgStat;
	BYTE isFirstMsgDelay = 0;
	DWORD tempContentInState = (DWORD)eNoChannel;
	CTokenMsg* pTokenDelayMsg = NULL;

	*pParam >> status >> tempContentInState;;

	DeleteTimer(RTPCONTENTACKTOUT);

	if (status != STATUS_OK)
	{
		PTRACE2(eLevelInfoNormal,"CSipParty::OnRtpAckForContentOnOff - Name - ",PARTYNAME);
		// Error handling -
		// There are 2 different paths:
		// 1. In case that we wait for stream ON ack and received NACK
		// 2. In case we wait for Stream OFF ack and receive NACK
		if (m_pSipCntl->GetContentInStreamState() == eSendStreamOn)
		{
			// Case 1 - Stream ON
			DisconnectPartyDueToProblemsWithH239RtpStream();
		}
		else if (m_pSipCntl->GetContentInStreamState() == eSendStreamOff)
		{
			// Case 2 - Stream OFF
			HandleRtpProblemsDuringClosingContentStream();
		}
		else
			PASSERTMSG((DWORD)m_pSipCntl->GetContentInStreamState(), "CSipParty::OnRtpAckForContentOnOff - Wrong stream state");

		return;
	}

	 // Forward Msg to ContentBrdg
	 eContentState ePState = eStreamOff;
	 if (m_pSipCntl->GetContentInStreamState() == eStreamOn)
	 {
	 	ePState = eStreamOn;
	 	 m_pConfApi->HWConetntOnOffAck(this,ePState);
	 }

	// Send Update stream to TMM - Handling the list...
	CTokenMsgMngr	*tokenMsgList = new CTokenMsgMngr;
	tokenMsgList->EnableTokenMsgMngr();

	m_pH239TokenMsgMngr->StreamUpdate(tokenMsgList);

	if (tokenMsgList->Size() == 0)
	{
		tokenMsgList->Clear();
		POBJDELETE(tokenMsgList);
		return;
		//Assert???
	}
	// Handling first message
	TOKEN_MSG_LIST::iterator itr =  tokenMsgList->Begin();
	CTokenMsg* pTokenMsg = (CTokenMsg*)(*itr);

	SpreadAllH239Msgs(pTokenMsg->GetMsgSegment(),pTokenMsg->GetMsgDirection(),1,pTokenMsg->GetMsgOpcode(),eMsgFree);

	HandleTMMList(tokenMsgList);

	tokenMsgList->ClearAndDestroy();
	POBJDELETE(tokenMsgList);
}
//////////////////////// /////////////////////////////////////////////////////////
void CSipParty::HandleTMMList(CTokenMsgMngr* tokenMsgList)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::HandleTMMList - Name - ",PARTYNAME);
	CTokenMsg* pTokenMsg = NULL;
	CTokenMsg* pDelayedTokenMsg = NULL;
	BYTE isDelayed = 0;
	EHwStreamState eStreamStateForTmm;
	EMsgStatus eMsgStat;
	TOKEN_MSG_LIST::iterator itr =  tokenMsgList->Begin();
	itr++; // Start from the second message

	while (itr != tokenMsgList->End())
	{
		pTokenMsg = (CTokenMsg*)(*itr);
		if(IsValidPObjectPtr(pTokenMsg))
		{
			eStreamStateForTmm  = SetCorrectStreamStateForTMM();
			eMsgStat = m_pH239TokenMsgMngr->NewTokenMsg(pTokenMsg,eStreamStateForTmm);

			// In this case we will send all messages till we come across a delay one and then we will
			// 1. Insert all messages to the list
			// 2. Handle the Delay message.
			if ((eMsgStat == eMsgDelayed) && isDelayed == 0)
			{
				pDelayedTokenMsg = new CTokenMsg(*pTokenMsg);
				isDelayed = 1;
			}
			if (isDelayed)
			{
				if (eMsgStat != eMsgDelayed)
				{
					PASSERTMSG((DWORD)eMsgStat,"CSipParty::HandleTMMList - Status was suppose to be DELAY (Regular case)");
				}
			}
			else
				SpreadAllH239Msgs(pTokenMsg->GetMsgSegment(),pTokenMsg->GetMsgDirection(),1,pTokenMsg->GetMsgOpcode(),eMsgFree);


		}
		else
			PASSERTMSG(tokenMsgList->Size(),"CSipParty::HandleTMMList - TMM list is corrupted - Token msg not valid !!");

		itr++;

	}

	if (isDelayed)
		SpreadAllH239Msgs(pDelayedTokenMsg->GetMsgSegment(),pDelayedTokenMsg->GetMsgDirection(),1,pDelayedTokenMsg->GetMsgOpcode(),eMsgDelayed);

	POBJDELETE(	pDelayedTokenMsg);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::DisconnectPartyDueToProblemsWithH239RtpStream()
{
	// In this case - we will send evacuate towards the RTP and disconnect the party
	PTRACE2(eLevelInfoNormal,"CSipParty::DisconnectPartyDueToProblemsWithH239RtpStream - Name - ",PARTYNAME);
	m_pSipCntl->SendEvacuateReqForRtpOnH239Stream();
	m_pConfApi->PartyDisConnect(IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR,this);
	m_pConfApi->UpdateDB(this,DISCAUSE,IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR,1); // Disconnnect cause
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::HandleRtpProblemsDuringClosingContentStream()
{
	PTRACE2(eLevelInfoNormal,"CSipParty::HandleRtpProblemsDuringClosingContentStream - Name - ",PARTYNAME);
	// In this case we will:
	// 1. Send Evacuate and wait for the Ack
	// 1.1 In case we don't receive an Ack(Tout)/Receive Nack - Disconnect the party.
	// 1.2 If we receive an Ack from the Rtp - Change stream state to eStreamOff and release all queued token messages (Update).
	m_pSipCntl->SendEvacuateReqForRtpOnH239Stream();
	StartTimer(RTPEVACUATEACKTOUT,2*SECOND);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnRtpAckForEvacuateContentStream(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnRtpAckForEvacuateContentStream - Name - ",PARTYNAME);
	DeleteTimer(RTPEVACUATEACKTOUT);
	DWORD status;
	*pParam >> status;

	if (status != STATUS_OK)
	{ // Disconnect the party
		OnRtpAckForEvacuateTout(pParam);
		return;
	}

	m_pSipCntl->SetContentInStreamState(eStreamOff);

	// Update stream to TMM - Handling the list...
	CTokenMsgMngr	*tokenMsgList = new CTokenMsgMngr;
	tokenMsgList->EnableTokenMsgMngr();


	m_pH239TokenMsgMngr->StreamUpdate(tokenMsgList);

	if (tokenMsgList->Size() == 0)
	{
		PASSERTMSG((DWORD)m_pSipCntl->GetContentInStreamState(),"CSipParty::OnRtpAckForEvacuateContentStream - List is empty ");
		tokenMsgList->Clear();
		POBJDELETE(tokenMsgList);
		return;
	}

	// Handling first message
	TOKEN_MSG_LIST::iterator itr =  tokenMsgList->Begin();
	CTokenMsg* pTokenMsg = (CTokenMsg*)(*itr);
	// If the first message id Delay - We need to issue an Assert and send all other messages to
	// the TMM (Ignore all replys till list is empty

	SpreadAllH239Msgs(pTokenMsg->GetMsgSegment(),pTokenMsg->GetMsgDirection(),1,pTokenMsg->GetMsgOpcode(),eMsgFree);

	HandleTMMList(tokenMsgList);

	tokenMsgList->ClearAndDestroy();
	POBJDELETE(tokenMsgList);

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnRtpAckForContentTout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnRtpAckForContentTout - Name - ",PARTYNAME);
	// Evacuate (Err handling)
	// There are 2 different paths:
	// 1. In case that we wait for stream ON ack and received NACK
	// 2. In case we wait for Stream OFF ack and receive NACK
	if (m_pSipCntl->GetContentInStreamState() == eSendStreamOn)
	{
		// Case 1 - Stream ON
		DisconnectPartyDueToProblemsWithH239RtpStream();
	}
	else if (m_pSipCntl->GetContentInStreamState() == eSendStreamOff)
	{
		// Case 2 - Stream OFF
		HandleRtpProblemsDuringClosingContentStream();
	}
	else
		PASSERTMSG((DWORD)m_pSipCntl->GetContentInStreamState(), "CSipParty::OnRtpAckForContentTout - Wrong stream state");
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnRtpAckForEvacuateTout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnRtpAckForEvacuateTout - Name - ",PARTYNAME);
	CSegment *pSeg = new CSegment;
	*pSeg << (DWORD)IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR;
	OnPartySendFaultyMfaToPartyCntlAnycase(pSeg);
	m_pConfApi->UpdateDB(this,DISCAUSE,IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR,1); // Disconnnect cause
	POBJDELETE(	pSeg );
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnContentBrdgSimulateIndRefreshVideo(CSegment * pParam)
{
	APIS8 videoLabel[32];

	CSegment *pSeg = new CSegment;

	snprintf(videoLabel, 32, STREAM_LABEL_CONTENT);

	pSeg->Put((BYTE*)videoLabel, 32);

	m_pSipCntl->OnSipCsVideoUpdatePicInd(pSeg);
	POBJDELETE(pSeg);

	if(m_repeatContentIntra < m_repeatContentIntraNumber)
	{

		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnContentBrdgSimulateIndRefreshVideo - set timer to repeat simulation of receiving intra for content, m_repeatContentIntra = ", m_repeatContentIntra);
		StartTimer(CONTENTVIDSIMINDREFRESH, m_contentIntraInterval*SECOND);
		m_repeatContentIntra++;
	}
	else if(m_repeatContentIntra == m_repeatContentIntraNumber)
	{
		m_repeatContentIntra = 0;
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnContentBrdgRefreshVideo(CSegment * pParam)
{
	ContentIntraRequestFiltering();
}
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::ContentIntraRequestFiltering()
{
	PTRACE2(eLevelInfoNormal,"CSipParty::ContentIntraRequestFiltering, Name - ",PARTYNAME);
	TICKS curTimer;

	//Fast Update requests will be sent to a party
	//in a minimum interval of 1 seconds
	curTimer = SystemGetTickCount();

	if (curTimer < m_lastContentRefreshTime)
		m_lastContentRefreshTime = 0; //rollover fixup

	DWORD suppresionIntervalInSeconds = 2;

	// we use the same flags as in COP
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetDWORDDataByKey("CONTENT_SPEAKER_INTRA_SUPPRESSION_IN_SECONDS", suppresionIntervalInSeconds);

	if ((m_lastContentRefreshTime == 0) || (curTimer - m_lastContentRefreshTime > SECOND*suppresionIntervalInSeconds ))
	{
		SendFastUpdateReq(kRoleContentOrPresentation); //m_pSipCntl->FastUpdateReq(kRoleContentOrPresentation);
		m_lastContentRefreshTime = curTimer;
		StartTimer(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,SECOND*suppresionIntervalInSeconds);
		m_num_content_intra_filtered=0;
	}
	else
	{
		// if timer
		m_num_content_intra_filtered++;
		PTRACE2(eLevelInfoNormal,"CSipParty::ContentIntraRequestFiltering,-do not send intra  Name - ",PARTYNAME);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTimerContentSpeakerIntraRequest(CSegment* pParam)
{
	if (m_num_content_intra_filtered > 0)
	{
		m_num_content_intra_filtered = 0;
		SendFastUpdateReq(kRoleContentOrPresentation);
		TICKS curTimer;
		curTimer = SystemGetTickCount();
		if (curTimer < m_lastContentRefreshTime)
			m_lastContentRefreshTime = 0; //rollover fixup
		m_lastContentRefreshTime = curTimer;

		DWORD suppresionIntervalInSeconds = 2;

		// we use the same flags as in COP
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		sysConfig->GetDWORDDataByKey("CONTENT_SPEAKER_INTRA_SUPPRESSION_IN_SECONDS", suppresionIntervalInSeconds);

		StartTimer(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER, SECOND* suppresionIntervalInSeconds);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfChangeModeContentRate(DWORD newContentRate)
{
	DWORD newPeopleRate = m_pCurrentMode->GetVideoBitRate(cmCapReceive, kRolePeople);

	if (m_pTargetMode->GetIsTipMode())
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeContentRate : Tip call");
		// not need to reduce video rate. Content rate is constant, not shared with video rate.
	}
	else if (m_pSipCntl->GetIsContentSpeaker())
	{
		if(m_pTargetMode->GetTotalVideoRate() > newContentRate)
		newPeopleRate = m_pTargetMode->GetTotalVideoRate() - newContentRate;
		if (m_pSipCntl->GetIsMrcCall())
			PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeContentRate : For MRC call not needed to send flow control for the video");
		else
		    SendFlowControlMessage(mainType, cmCapReceive, newPeopleRate);
		SendFlowControlMessage(slideType, cmCapReceive, newContentRate);
	}
	else
	{
		if (newPeopleRate != m_pTargetMode->GetTotalVideoRate())  // This means that party was the content speaker, and now it became non speaker - need to return to full rate for people.
		{
			PTRACE2INT(eLevelInfoNormal,"CSipParty::OnConfChangeModeContentRate : current video bit rate: ", newPeopleRate);
			newPeopleRate = m_pTargetMode->GetTotalVideoRate();
	        if (m_pSipCntl->GetIsMrcCall())
	            PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeContentRate : For MRC call not needed to send flow control for the video");
	        else
	            SendFlowControlMessage(mainType, cmCapReceive, newPeopleRate);
		}
	}

	m_pTargetMode->SetContentBitRate(newContentRate, cmCapReceiveAndTransmit);
	m_pCurrentMode->SetContentBitRate(newContentRate, cmCapReceiveAndTransmit);
	m_pTargetMode->SetVideoBitRate(newPeopleRate, cmCapReceiveAndTransmit, kRolePeople);
	m_pCurrentMode->SetVideoBitRate(newPeopleRate, cmCapReceiveAndTransmit, kRolePeople);

	CMedString str;
	str <<"Content new rate: " << newContentRate << ", People new rate: " << newPeopleRate << ", Content speaker: " << m_pSipCntl->GetIsContentSpeaker()
		<< ", TotalVideoRate: " << m_pTargetMode->GetTotalVideoRate();
	PTRACE2(eLevelInfoNormal,"***CSipParty::OnConfChangeModeContentRate : ", str.GetString());

	//FSN-613: Dynamic Content for SVC/Mix Conf
	if (m_pSipCntl->GetIsContentSpeaker() && m_pSipCntl->GetContentCapsCountInSentSdp() > 1)  //m_bNeedReInviteToFixContentAlg = TRUE;
	{
		OnConfChangeModeContentProtocol(m_pTargetMode);
		return;
	}

	// Need to send end change mode to party control:
	BYTE bIsEndConfChangeMode = TRUE;
    	m_pCurrentMode->Dump("CSipParty::OnConfChangeModeContentRate current",eLevelInfoNormal);
	m_pConfApi->SipPartyRemoteConnected(GetPartyId(), m_pCurrentMode, bIsEndConfChangeMode);
	m_maskRequiredChangeMode = eChangeModeMask_None;
	m_bIsConfChangeMode = FALSE;
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfChangeModeContentProtocol(CIpComMode *pNewMode)
{
	//pNewMode->Dump("***CSipParty::OnConfChangeModeContentProtocol : New mode:",eLevelInfoNormal);
	//FSN-613: Dynamic Content for SVC/Mix Conf test
	//m_pTargetMode->Dump("***CSipParty::OnConfChangeModeContentProtocol : Target mode before copy content:",eLevelInfoNormal);
	// Set the new content mode:

	m_pTargetMode->CopyMediaMode(*pNewMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);

	//BRIDGE-15059
	/*if (m_pSipCntl->GetIsMrcCall() && pNewMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePresentation))
	{
		m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		m_pSipCntl->RemoveBfcpAndContentCaps();
	}*/

	// As part of change content protocol, the content is stopped so we need to set the people rate back to total video rate.
	DWORD newPeopleRate = m_pTargetMode->GetTotalVideoRate();
	PTRACE2INT(eLevelInfoNormal,"***CSipParty::OnConfChangeModeContentProtocol : return people rate to total video rate: ", newPeopleRate);
	m_pTargetMode->SetVideoBitRate(newPeopleRate, cmCapReceiveAndTransmit, kRolePeople);
	DWORD newContentRate = m_pTargetMode->GetVideoBitRate(cmCapReceive, kRoleContentOrPresentation);
	DWORD rateConstraint = 0;


	DWORD ratesDiff = newPeopleRate - newContentRate;
	DWORD minRate = 640;

	if(ratesDiff < minRate)
	{
		//In CCS Plugin SDP, content rate is equal to people video rate. The video rate here is meaningless.
		PASSERTMSG(!m_pSipCntl->GetIsCCSPlugin(),"CSipParty::OnConfChangeModeContentProtocol - (newPeopleRate - newContentRate) < 640 ");
		ratesDiff = minRate;
	}

	if(m_pTargetMode->GetFlowControlRateConstraint())
		rateConstraint = min(m_pTargetMode->GetFlowControlRateConstraint(), ratesDiff);
	else
		rateConstraint = ratesDiff;

	//FSN-613: Dynamic Content for SVC/Mix Conf
	DWORD declareContentRate = pNewMode->GetDeclareContentRate();
	
	if (m_pSipCntl->GetIsMrcCall())
	{
		if (newContentRate)
			m_pSipCntl->SetFullContentRate(newContentRate);
		else if (declareContentRate)
			m_pSipCntl->SetFullContentRate(declareContentRate);
	}

	CLargeString cstr;
	cstr << "CCSipParty::OnConfChangeModeContentProtocol ";
	cstr << " newPeopleRate=" << newPeopleRate << " newContentRate=" << newContentRate << " declareContentRate=" << declareContentRate<< " orig constraint=" << m_pTargetMode->GetFlowControlRateConstraint();
	cstr << " new constraint=" << rateConstraint << "\n";

	m_pTargetMode->SetFlowControlRateConstraint(rateConstraint);

	PTRACE(eLevelInfoNormal,cstr.GetString());
	 CSipCall *pCall = m_pSipCntl->GetCallObj();
	 CSipChannel* videoOutCh =  pCall->GetChannel(VIDEO_OUT);
	 BYTE isVidoOutMuted = FALSE;
	 if(videoOutCh && videoOutCh->IsMuted())
	 {
		 PTRACE(eLevelInfoNormal,"CSipParty::OnConfChangeModeContentProtocol OUT VIDEO MUTED");
		 isVidoOutMuted = TRUE;

	 }

	 m_pSipCntl->DeclareOnContentFromScmOnly(TRUE);

/*	Angelina - no difference between SVC and AVC in respect of re-invite
	 //FSN-613: Dynamic Content for SVC/Mix Conf
	 ESipTransactionType eTransactionType = kSipTransNone;
	 if(m_pSipCntl->GetIsMrcCall())
	{
		eTransactionType=kSipTransReInviteMrcWithSdpReq;
	}
	else
	{
		eTransactionType=kSipTransReInviteWithSdpReq;
	}
*/
	ESipTransactionType eTransactionType=kSipTransReInviteWithSdpReq;
	StartTransaction(eTransactionType, SIP_PARTY_SEND_REINVITE, NULL);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyCntlStartVideoPreference(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyCntlStartVideoPreference, Name - ",PARTYNAME);

	if (m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive))
	{
		CCapSetInfo TargetCapInfo	= (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive);
		CCapSetInfo CurrenttCapInfo	= (CapEnum)m_pCurrentMode->GetMediaType(cmCapVideo, cmCapReceive);

		if(TargetCapInfo == CurrenttCapInfo && ((CapEnum)TargetCapInfo == eRtvCapCode )&&
			(m_pSipCntl->GetRemoteIdent() == MicrosoftEP_R1 ||
			 m_pSipCntl->GetRemoteIdent() == MicrosoftEP_R2 ||
			 m_pSipCntl->GetRemoteIdent() == MicrosoftEP_Lync_R1 ||
			 m_pSipCntl->GetRemoteIdent() == MicrosoftEP_MAC ||
			 m_pSipCntl->GetRemoteIdent() == MicrosoftEP_MAC_Lync ||
			 m_pSipCntl->GetRemoteIdent() == MicrosoftEP_Lync_2013 ||
			 m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU ||
			 m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU2013))
		{
			if(m_RTVLastVideoPreferenceWidth && m_RTVLastVideoPreferenceHeigh)
			{
				DWORD details	= 0;
				int  arrIndex	= 0;
				DWORD ValuesToCompare = kCapCode|kFormat;
				CSipCaps*	pRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
				const CSipCaps* pLocalCaps = m_pSipCntl->GetLocalCaps();

				CComModeH323* pScmWithNewRes =  new CComModeH323;
				*pScmWithNewRes = *m_pTargetMode;
				pScmWithNewRes->SetRtvScm(m_RTVLastVideoPreferenceWidth,m_RTVLastVideoPreferenceHeigh,0,cmCapTransmit,0);
				const CBaseCap* pModeNewRes = pScmWithNewRes->GetMediaAsCapClass(cmCapVideo,cmCapTransmit);
				BYTE IsSupportNewRes = pRemoteCaps->IsContainingCapSet(cmCapReceive, *pModeNewRes, ValuesToCompare, &details, &arrIndex);
				BYTE isLocalSupportNewRes = pLocalCaps->IsContainingCapSet(cmCapReceive, *pModeNewRes, ValuesToCompare, &details, &arrIndex);

				if(IsSupportNewRes && isLocalSupportNewRes)
					m_pSipCntl->SendRtcpVideoPreferenceReq(m_RTVLastVideoPreferenceWidth,m_RTVLastVideoPreferenceHeigh);
				else
					m_pSipCntl->SendRtcpVideoPreferenceReq(0,0);
			}
			else
			{
			    CComModeH323* pScmWithNewRes =  new CComModeH323;
			    *pScmWithNewRes = *m_pTargetMode;
			    const CRtvVideoCap *pRtvCap = (const CRtvVideoCap *) pScmWithNewRes->GetMediaAsCapClass(cmCapVideo,cmCapReceive);
			    if (pRtvCap)
			    {
			        PTRACE(eLevelInfoNormal,"CSipParty::OnPartyCntlStartVideoPreference, sending by SCM");
			        rtvCapStruct* pRtvCapStruct = (rtvCapStruct *)pRtvCap->GetStruct();
			        rtvCapItemS rtvMaxCapItem;
			        DWORD RtvVideoPreferenceWidth = 0;
			        DWORD RtvVideoPreferenceHeigh = 0;
			        EResult eRes = pRtvCap->FindMaxCapInCapSet(pRtvCapStruct,rtvMaxCapItem);
			        if (eRes)
			        {
			            RtvVideoPreferenceWidth = rtvMaxCapItem.widthVF;
			            RtvVideoPreferenceHeigh = rtvMaxCapItem.heightVF;
			        }
			        m_pSipCntl->SendRtcpVideoPreferenceReq(RtvVideoPreferenceWidth,RtvVideoPreferenceHeigh);
			        POBJDELETE(pRtvCap);
			    }
			    else
			        PTRACE(eLevelInfoNormal,"CSipParty::OnPartyCntlStartVideoPreference - pRtvCap is NULL - not sending by SCM");
			}

			m_bIsRtvPreferenceMsgSent = TRUE;
		}
	}

}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendUpdateChannelsRtpIfNeeded()
{
	PTRACE(eLevelInfoNormal, "CSipParty::SendUpdateChannelsRtpIfNeeded");
	CSipChannel *pChannel = NULL;
	int 			numOfUpdateDtlsChannels = 0;
	cmCapDataType 	mediaType;
	ERoleLabel 		eRole;
	CDtlsCap 	*pDtlsCap = NULL;
	CSdesCap *pSdesCap = NULL;

	CSipCall *pCall = m_pSipCntl->GetCallObj();
	if(!pCall)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::SendUpdateChannelsRtpIfNeeded failed to get call object");
		return;
	}

	for (int i = 0 ; i < CHANNEL_TYPES_COUNT; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if(mediaType == cmCapBfcp)
			continue;


		for (int j = 0; j < 2; j++)// 2 = number of direction (receive, transmit).
		{
			{
				// Adding update of change of IP/UDP in case of TIP.
				// When TIP EP hold, CUCM send it's IP:port and send media, so we need to receive media from CUCM.
				pChannel = pCall->GetChannel(true, mediaType, globalDirectionArr[j], eRole);

				if (pChannel && m_pTargetMode->IsMediaOn(mediaType, globalDirectionArr[j],eRole) )
				{
					CMedString str;
					str << "mediaType:" << mediaType << ", channelDirection:" << globalDirectionArr[j];
					PTRACE2(eLevelInfoNormal, "CSipParty::SendUpdateChannelsRtpIfNeeded - ", str.GetString());

					BYTE bChangeParams = 0;
					if( m_pTargetMode->GetIsDtlsAvailable() )
					{
						pDtlsCap = m_pTargetMode->GetSipDtls(mediaType,globalDirectionArr[j],eRole);

						pChannel->SetChannelDtls(pDtlsCap);



						bChangeParams |= kChangeDtls;
					}

					else if( m_pTargetMode->GetIsEncrypted() == Encryp_On)
					{
						pSdesCap = m_pTargetMode->GetSipSdes(mediaType,globalDirectionArr[j],eRole);
						pChannel->SetChannelSdes(pSdesCap);
						bChangeParams |= kChangeSdes;
					}
					if( bChangeParams )
					{
						EIpChannelType chanArr = static_cast<EIpChannelType>(-1);
						if((globalDirectionArr[j] == cmCapReceive))
						{
							chanArr = ::CalcChannelType(mediaType, NO, eRole);

							TRACEINTO << ", change receive " << ((bChangeParams && kChangeDtls)?"dtls":"sdes") << ", mediaType=" << (int)mediaType;
						}
						else if (globalDirectionArr[j] == cmCapTransmit)
						{
							chanArr = ::CalcChannelType(mediaType,YES, eRole);
							TRACEINTO << ", change transmit " << ((bChangeParams && kChangeDtls)?"dtls":"sdes") << ", mediaType=" << (int)mediaType;
						}

						if(chanArr != static_cast<EIpChannelType>(-1) && m_pSipCntl->SipUpdateChannelReq((CSipComMode*)m_pTargetMode, chanArr, bChangeParams))
							numOfUpdateDtlsChannels++;
					}
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////
void CSipParty::UpdateDtlsCapsForAllSlaves()
{
	PTRACE(eLevelInfoNormal, "CSipParty::UpdateDtlsCapsForAllSlaves");

	WORD numOfSlaves				= 0;
	CSegment *pSeg 					=  new CSegment;

	m_pTargetMode->Serialize(NATIVE,*pSeg);

	if (m_TipNumOfStreams)
	{
		UpdateDtlsCapsForSlave(eTipSlaveAux, pSeg);
		numOfSlaves = m_TipNumOfStreams -1 + m_bIsAudioAux;
		if (numOfSlaves > 1)
		{
			UpdateDtlsCapsForSlave(eTipSlaveLeft, pSeg);
			UpdateDtlsCapsForSlave(eTipSlaveRigth, pSeg);
		}
	}
	POBJDELETE(pSeg);
}
/////////////////////////////////////////////////////////////////////
void CSipParty::UpdateDtlsCapsForSlave(ETipPartyTypeAndPosition slaveType , CSegment *pSeg)
{
	PTRACE2INT(eLevelInfoNormal, "CSipParty::UpdateDtlsCapsForSlave, TIP slaves , type and position: ", slaveType);
	SendMessageFromMasterToSlave(slaveType, TIP_UPDATE_RTP_FOR_SLAVE, pSeg);
}
/////////////////////////////////////////////////////////////////////
void CSipParty::MuteMediaIfNeeded(cmCapDirection eDirection, BYTE bTipForceMuteChannels, BYTE channelsMask)
{
	PTRACE2INT(eLevelInfoNormal, "CSipParty::MuteMediaIfNeeded, direction: ", eDirection);
	PTRACE2INT(eLevelInfoNormal, "CSipParty::MuteMediaIfNeeded, bTipForceMuteChannels: ", bTipForceMuteChannels);

	BYTE bAudioMuteIn		= AUTO;
	BYTE bAudioMuteOut		= AUTO;
	BYTE bVideoMuteIn		= AUTO;
	BYTE bVideoMuteOut		= AUTO;
	BYTE bFeccMuteOut		= AUTO;
	BYTE bFeccMuteIn		= AUTO;
	BYTE bContentMuteOut	= AUTO;
	BYTE bContentMuteIn		= AUTO;
	BYTE bDummyBfcpMuteOut	= AUTO;// no real mute for bfcp. it's just to fill the structure
	BYTE bDummyBfcpMuteIn	= AUTO;

	BYTE *MuteArray[MAX_SIP_MEDIA_TYPES][2] = {{&bAudioMuteIn, &bAudioMuteOut}, {&bVideoMuteIn, &bVideoMuteOut}, {&bFeccMuteIn, &bFeccMuteOut}, {&bContentMuteIn, &bContentMuteOut}
	, {&bDummyBfcpMuteIn, &bDummyBfcpMuteOut}};

	BYTE bIsMute = NO;
	BYTE bIsChanged = NO;

	// check mute by party for in channels only. out channels will be checked after connected
	CSipChannel* pChannel = NULL;
	cmCapDataType mediaType;
	ERoleLabel eRole;

	// TIP
	if (bTipForceMuteChannels )
	{
		if (eDirection & cmCapTransmit)
		{
			PTRACE(eLevelInfoNormal, "CSipPaty::MuteMediaIfNeeded, TIP force mute audio and video out");
			if (channelsMask & MASK_AUDIO)
				bAudioMuteOut = YES;
			if (channelsMask & MASK_VIDEO)
				bVideoMuteOut = YES;
			if (channelsMask & MASK_CONTENT)
				bContentMuteOut = YES;
		}
		if (eDirection & cmCapReceive)
		{
			PTRACE(eLevelInfoNormal, "CSipPaty::MuteMediaIfNeeded, TIP force mute audio and video in");
			if(GetIsTipCall() && m_bIsPolycomFromRTCP && (m_tipPartyType == eTipSlaveLeft || m_tipPartyType == eTipSlaveRigth))
			{
				if (channelsMask & MASK_AUDIO)
					bAudioMuteIn = YES;
			}
			if (channelsMask & MASK_VIDEO)
				bVideoMuteIn = YES;
			if (channelsMask & MASK_CONTENT)
				bContentMuteIn = YES;
		}
	}

	for(int i=0 ; i<MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if (mediaType == cmCapBfcp)
			continue;

		for (int j=0; j < 2; j++)
		{
			if (eDirection & globalDirectionArr[j])
			{
				// If TIP force mute, ignore remote SDP and force mute
				if (bTipForceMuteChannels)
				{
					bIsChanged = YES;
				}
				else
				{
					/* SetMedia unmute state only in case of a non-tip call or in case of a TIP call */
					if (GetIsTipCall() && (m_tipPartyType == eTipSlaveLeft || m_tipPartyType == eTipSlaveRigth))
					{
						if (mediaType == cmCapAudio && (globalDirectionArr[j] &  cmCapReceive) && m_bIsPolycomFromRTCP)
						{
							/* Do not unmute RX audio channel in slaves for polycom products */
							TRACEINTO << "Do not unmute TPX Slave- Tip Call m_tipPartyType=" << m_tipPartyType << ", mediaType=" << (int)mediaType <<
									" m_bIsPolycomFromRTCP = " << m_bIsPolycomFromRTCP << " direction = " << eDirection;
						}
						else if (mediaType == cmCapVideo && (globalDirectionArr[j] &  cmCapTransmit))
						{
							if (m_pSipCntl->SetMediaMuteState(mediaType, globalDirectionArr[j], MuteArray[i][j],eRole))
								bIsChanged = YES;
							/*if (bVideoMuteOut == NO)
							{
								PTRACE2INT(eLevelInfoNormal, "CSipPaty::MuteMediaIfNeeded, Send Intra for Slave video out m_tipPartyType=", m_tipPartyType);
								CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
								std::string key = "REPEAT_INTRA_NUMBER";
								pSysConfig->GetDWORDDataByKey(key, m_repeatIntraNumber);
								StartTimer(SIP_SEND_INTRA_AFTER_TRANS_TOUT, 1*SECOND);
							}*/
						}
						else
						{
							if (m_pSipCntl->SetMediaMuteState(mediaType, globalDirectionArr[j], MuteArray[i][j],eRole))
							{
								bIsChanged = YES;
							}
						}
					}
					else
					{
						if (m_pSipCntl->SetMediaMuteState(mediaType, globalDirectionArr[j], MuteArray[i][j],eRole))
							bIsChanged = YES;
					}
				}

				if(*(MuteArray[i][j]) != AUTO)
				{
					PTRACE(eLevelError, "CSipParty::MuteMediaIfNeeded channel change mute array");
					MuteMediaByParty(*(MuteArray[i][j]), mediaType, globalDirectionArr[j],eRole);
				}
			}
		}
	}

	if (bIsChanged)
	{
		PTRACE(eLevelError, "CSipParty::MuteMediaIfNeeded channel is changed it true ");
		m_pConfApi->IpMuteMedia(GetPartyId(), bAudioMuteIn,bAudioMuteOut,bVideoMuteIn,bVideoMuteOut,bContentMuteIn,bContentMuteOut,bFeccMuteIn,bFeccMuteOut);

		bIsMute = NO;

		if (GetIsTipCall() && m_tipPartyType == eTipMasterCenter && m_SlaveAuxRsrcId != eTipNone)
		{
			for(int i=0 ; i<MAX_SIP_MEDIA_TYPES; i++)
			{
				GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
				if (mediaType == cmCapBfcp)
					continue;

				for (int j=0; j < 2; j++)
				{
					if(*(MuteArray[i][j]) == YES)
					{
						bIsMute = YES;
						break;
					}
				}
			}

			CSegment *pSeg = new CSegment;

			DWORD direction = (int) eDirection;
			BYTE muteMask = 0xFF;

			if ((bIsMute == YES) || bTipForceMuteChannels)
				bTipForceMuteChannels = TRUE;

			if ((m_bIsPolycomFromRTCP) && (eDirection & cmCapReceive))
				BYTE muteMask = MASK_VIDEO | MASK_CONTENT;

			*pSeg << (DWORD) direction << (BYTE)bTipForceMuteChannels << muteMask << m_bIsPolycomFromRTCP;//bAudioMuteIn << bAudioMuteOut;

			TRACEINTO << "TIP slaves aux, direction=" << direction << ", bTipForceMuteChannels=" << (int)bTipForceMuteChannels << " muteMask = " << muteMask;

			SendMessageFromMasterToSlave(eTipSlaveAux, TIP_MUTE_SLAVE_IF_NEEDED, pSeg);

			WORD numOfSlaves = m_TipNumOfStreams -1 + m_bIsAudioAux;

			if (numOfSlaves > 1)
			{
				PTRACE(eLevelInfoNormal, "CSipParty::MuteMediaIfNeeded, TIP slaves right & left");

				SendMessageFromMasterToSlave(eTipSlaveLeft, TIP_MUTE_SLAVE_IF_NEEDED, pSeg);
				SendMessageFromMasterToSlave(eTipSlaveRigth, TIP_MUTE_SLAVE_IF_NEEDED, pSeg);
			}

			POBJDELETE(pSeg);
		}
	}
//		SendMuteMediaToParty(bAudioMuteIn,bAudioMuteOut,bVideoMuteIn,bVideoMuteOut,bContentMuteIn,bContentMuteOut,bFeccMuteIn,bFeccMuteOut);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::MuteMediaByParty(BYTE bIsMute,cmCapDataType eMedia,cmCapDirection eDirection, ERoleLabel eRole)
{// we only mute the party DB, the mute of bridges is done by API.
	PTRACE(eLevelInfoNormal,"CSipParty::MuteMediaByParty");
	if (eDirection == cmCapReceiveAndTransmit)
	{
		EIpChannelType chanArr[2];
		chanArr[0] = ::CalcChannelType(eMedia, NO, eRole);
		chanArr[1] = ::CalcChannelType(eMedia, YES, eRole);
		m_pSipCntl->MuteChannels(bIsMute,2,chanArr);
	}
	else
	{
		EIpChannelType chan = ::CalcChannelType(eMedia,eDirection==cmCapTransmit,eRole);
		m_pSipCntl->MuteChannels(bIsMute,1,&chan);
	}
}
/////////////////////////////////////////////////////////////////////////////
// TIP --------------------------------------------------------------
void CSipParty::SetIsTipCall(BYTE isTipCall)
{
	m_bIsTipCall = isTipCall;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::SetTipPartyTypeAndPosition(ETipPartyTypeAndPosition tipPartyType)
{
	m_tipPartyType = tipPartyType;

	BOOL bIsTIPCall = m_tipPartyType != eTipNone ? TRUE : FALSE;
	SetIsTipCall(bIsTIPCall);

	if (!bIsTIPCall)
	{
		if (IsValidTimer(CREATE_SLAVE_ACK_TOUT))
			DeleteTimer(CREATE_SLAVE_ACK_TOUT);
	}

	CSipCall* pSipCall = NULL;
	if(m_pSipCntl && (pSipCall=m_pSipCntl->GetCallObj()))
		pSipCall->SetIsTipCall(bIsTIPCall);

	if( eTipMasterCenter == m_tipPartyType )
	{
		CCommConf* pCommConf 	= ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
		if (pCommConf)
		{
		    CConfParty* pConfParty 	= pCommConf->GetCurrentParty(GetMonitorPartyId());
		    if( pConfParty )
		    {
		        pConfParty->SetTIPPartyType( eTipPartyMaster );
		        pConfParty->SetTIPPartySubType(eTipMasterCenter);
		        pConfParty->SetRoomId(m_RoomId);

		    }
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
ETipPartyTypeAndPosition CSipParty::GetTipPartyTypeAndPosition()
{
	return m_tipPartyType;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTipCntlNegotiationResult(CSegment * pParam)
{
	//static int nTipNeg = 0;

	DWORD status 		= 0;
//	DWORD doVideoReInvite 	= 0;
	WORD numOfStreams 	= 0;
	BYTE bIsAudioAux 	= FALSE;
	BYTE bIsVideoAux 	= FALSE;

	*pParam >> status;
	*pParam >> numOfStreams;
	*pParam >> bIsAudioAux;
	*pParam >> bIsVideoAux;
	*pParam >> m_bIsNeedTipReinviteAfterNegotiation;

	CMedString str;
	str << "Status = " << status << ", number of streams = " << numOfStreams << ", AudioAux=" << bIsAudioAux << ", VideoAux=" << bIsVideoAux << ", doVideoReInvite=" << m_bIsNeedTipReinviteAfterNegotiation;

	PTRACE2(eLevelInfoNormal, "CSipParty::OnTipCntlNegotiationResult : ", str.GetString());

	if (status == eTipNegSuccess)
	{
		m_pTargetMode->SetTipMode(eTipModeNegotiated);
		m_pTargetModeMaxAllocation->SetTipMode(eTipModeNegotiated);

		// first time TIP is connected
		if (!m_TipNumOfStreams)
		{
			m_TipNumOfStreams 	= numOfStreams;
			m_bIsAudioAux 		= bIsAudioAux;
			m_bIsVideoAux 		= bIsVideoAux;

			m_NumOfAckFromSlaves= 0;
			SetScmAndChannelsForTipNegotiation();

			CCommConf* pCommConf       = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
			if (pCommConf)
			{
				//send event
				PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
				ConfUserDataUpdateEvent.m_participantMaxUsage.m_name = GetName();
				ConfUserDataUpdateEvent.m_participantMaxUsage.m_partyId = GetMonitorPartyId();
				ConfUserDataUpdateEvent.m_participantMaxUsage.m_numberOfScreens = numOfStreams;
				pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, GetMonitorPartyId());
			}
//			numOfSlaves 		= m_TipNumOfStreams -1 + m_bIsAudioAux;
//			PTRACE2INT(eLevelInfoNormal, "CSipParty::OnTipCntlNegotiationResult : numOfSlaves ", numOfSlaves);
//
//			if (numOfSlaves == 0)
//			{
//				m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_UPDATE_ON_TIP_CALL_REQ);
//				m_pSipCntl->EndTipNegotiation(eTipNegSuccess);
//			}
//
//			m_NumOfAckFromSlaves = 0;
//
//			if (bIsAudioAux)
//				AddSlaveParty(eTipSlaveAux);
//
//			if (numOfStreams == 3)
//			{
//				AddSlaveParty(eTipSlaveLeft);
//				AddSlaveParty(eTipSlaveRigth);
//			}
//
//			StartTimer(CREATE_SLAVE_ACK_TOUT,SECOND*50);
		}
		else
		{
			m_bIsTipResumed = TRUE;
			if (m_TipNumOfStreams == numOfStreams)
			{
				m_pSipCntl->EndTipNegotiation(eTipNegSuccess);

				PTRACE2INT(eLevelInfoNormal, "CSipParty::OnTipCntlNegotiationResult : no change in num of streams: ", numOfStreams);

				//SetIsTipNegotiationActive(FALSE);
				m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_UPDATE_ON_TIP_CALL_REQ);
				//MuteMediaIfNeeded(cmCapReceiveAndTransmit, FALSE);

				/*
				DWORD	timebeforeresume = 0;
				CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
				std::string key1 = "UNMUTE_MEDIA_AFTER_RESUME_IN_SEC";
				pSysConfig->GetDWORDDataByKey(key1, timebeforeresume);
				StartTimer(UNMUTE_MEDIA_AFTER_RESUME_TIMER, timebeforeresume*SECOND);
				PTRACE2INT(eLevelInfoNormal, "CSipParty::OnTipCntlNegotiationResult : timebeforeresume ", timebeforeresume);
				*/

				//CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
				//std::string key = "REPEAT_INTRA_NUMBER";
				//pSysConfig->GetDWORDDataByKey(key, m_repeatIntraNumber);
				//StartTimer(SIP_SEND_INTRA_AFTER_TRANS_TOUT, 3*SECOND);
				UpdateDtlsCapsForAllSlaves();

				m_bTipEndSuccessSent = TRUE;

				ResumeMediaAndSendReinviteForTip();
			}
			else if (m_TipNumOfStreams > numOfStreams)
			{
				m_pSipCntl->EndTipNegotiation(eTipNegSuccess);

				PTRACE(eLevelInfoNormal, "CSipParty::OnTipCntlNegotiationResult : num of old streams >  num of new streams");

				m_TipNumOfStreams 	= numOfStreams;
				m_bIsAudioAux 		= bIsAudioAux;
				m_bIsVideoAux 		= bIsVideoAux;

				SendMessageFromMasterToSlave(eTipSlaveLeft, PARTYDISCONNECT, NULL);
				SendMessageFromMasterToSlave(eTipSlaveRigth, PARTYDISCONNECT, NULL);

				m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_CLOSE_TIP_CALL_REQ);

				m_SlaveLeftRsrcId 	= 0;
				m_SlaveRightRsrcId 	= 0;

				m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_UPDATE_ON_TIP_CALL_REQ);

				MuteMediaIfNeeded(cmCapReceiveAndTransmit, FALSE);

				UpdateDtlsCapsForAllSlaves();

				if( m_bIsNeedTipReinviteAfterNegotiation )
					m_pSipCntl->TipReInviteReq(m_TipNumOfStreams, m_bIsVideoAux);
			}
			else // if (m_TipNumOfStreams < numOfStreams)
			{
				PTRACE(eLevelInfoNormal, "CSipParty::OnTipCntlNegotiationResult : num of old streams <  num of new streams");

				m_TipNumOfStreams 	= numOfStreams;
				m_bIsAudioAux 		= bIsAudioAux;
				m_bIsVideoAux 		= bIsVideoAux;
				m_NumOfAckFromSlaves = 0;

				m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_CLOSE_TIP_CALL_REQ);

				AddSlaveParty((DWORD)eTipSlaveLeft);
				AddSlaveParty((DWORD)eTipSlaveRigth);

				StartTimer(CREATE_SLAVE_ACK_TOUT,SECOND*30);
			}
		}
	}
	else
	{
		m_pSipCntl->EndTipNegotiation(eTipNegError);

		SetIsTipNegotiationActive(FALSE);

		if (status == eTipNegErrorHdxRemEp) // currently we do the same for regular error status, and remote HDX status.
			PTRACE(eLevelInfoNormal,"CSipParty::OnTipCntlNegotiationResult : HDX EP");

		// in case TIP call was active and after hold/resume TIP negotiation failed
		// we need to close the slaves and fall back to SIP
		CloseTipSessionAndSendMuxDisconnect();

		if (status == eTipNegErrorHdxRemEp) // check for Dial In/OUT.
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnTipCntlNegotiationResult : HDX EP");
			m_pSipCntl->SipCloseAllDtlsChannelsReq();
			CSipCall* currCall = m_pSipCntl->GetCallObj();

			if(currCall && currCall->IsCallInitiator() == YES) // If we are the in Dial Out start fall back immediately
				FallBackToRegularSip(PolycomEp);
			else  // wait for Re-Invite from HDX
			{
				m_pSipCntl->SetRemoteIdent(PolycomEp);

				StartTimer(TIP_WAIT_FALLBACK_TOUT,SECOND*3);
			}
		}
		else
		{
			//FallBackToRegularSip(Regular);
			TellConfOnDisconnecting(TIP_NEGOTIATION_FAILURE);
		}

	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::AddTipSlaves()
{
	if( !(m_pTargetMode->IsTipNegotiated()) )
	{
		PTRACE(eLevelInfoNormal,"CSipParty::AddTipSlaves - still negotiating");
		return;
	}

	WORD numOfSlaves 	= m_TipNumOfStreams -1 + m_bIsAudioAux;
	PTRACE2INT(eLevelInfoNormal, "CSipParty::AddTipSlaves : numOfSlaves ", numOfSlaves);

	if( numOfSlaves == 0xFFFF )
	{
		m_pSipCntl->EndTipNegotiation(eTipNegSuccess);

		m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_UPDATE_ON_TIP_CALL_REQ);

		SetIsTipNegotiationActive(FALSE);

		m_pSipCntl->SendUpdateChannelForDiffPayloadIfNeeded((CSipComMode*)m_pTargetMode);
	}

	if (m_bIsAudioAux)
		AddSlaveParty((DWORD)eTipSlaveAux);

	if (m_TipNumOfStreams == 3)
	{
		AddSlaveParty((DWORD)eTipSlaveLeft);
		AddSlaveParty((DWORD)eTipSlaveRigth);
	}
	StartTimer(CREATE_SLAVE_ACK_TOUT,SECOND*15);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::ResumeMediaAndSendReinviteForTip()
{
	TRACEINTO << "IsLastAckReceived:" << (int)m_bTipLastAckReceived << ", IsEndSuccessSent:" << (int)m_bTipEndSuccessSent << ", IsActiveContentForTip:" << (int)m_isActiveContentForTip;

	if (!m_bTipLastAckReceived || !m_bTipEndSuccessSent)
		return;

	DBGPASSERT(IsActiveTransaction());

	SetIsTipNegotiationActive(FALSE);
	m_pSipCntl->SetIsResuming(FALSE); // turn off when TIP negotiation is done

	MuteMediaIfNeeded(cmCapReceiveAndTransmit, FALSE);

	m_pSipCntl->ReActivateContentNotificationForTip();
	m_pSipCntl->UpdateRemoteCapsForTipNegRes();

	//send party control the remote caps
	CSipCaps* pCurRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
	if (pCurRemoteCaps)
		m_pConfApi->UpdatePartyControlOnRemoteCaps(GetPartyId(), pCurRemoteCaps);

	if (m_bIsNeedTipReinviteAfterNegotiation)
		m_pSipCntl->TipReInviteReq(m_TipNumOfStreams, m_bIsVideoAux);

	if (m_isActiveContentForTip && m_pSipCntl->GetIsContentSpeaker() == FALSE)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::ResumeMediaAndSendReinviteForTip send role_provider_identity");
		m_pSipCntl->SendTokenMessageReq(CONTENT_ROLE_PROVIDER_IDENTITY, m_mcuNum, m_termNum);
	}

	// BRIDGE-8051 resume IVR 'waiting for chair' after hold
	if (m_pConfApi)
	{
		CSegment seg;
		GetRcvMbx().Serialize(seg);
		seg << (WORD)m_ivrCtrl->IsIvrOnHold();
		m_pConfApi->IvrPartyNotification(GetPartyRsrcID(), this, GetName(), PARTY_IVR_MODE_ON_RESUME, &seg, eMediaInAndOut);
	}

	BOOL bIsResume = TRUE;
	StartIvr(bIsResume);
	CSegment seg;
	seg << bIsResume;
	ForwardEventToTipSlaves(&seg, START_IVR);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyForwardRemoteH230(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnPartyForwardRemoteH230");
	WORD tipPosition;
	*pParam >> tipPosition;
	ETipPartyTypeAndPosition partyPosition = ::GetPartyPosition((ETipVideoPosition)tipPosition);
	if (partyPosition != eTipNone)
	{
		WORD  opcode		= 0;
		WORD  videoSyncLost = 0;
		BYTE  bIsGradualIntra = FALSE;
		WORD  role = 0;

		CSegment*  pSeg = new CSegment;
		*pParam >> opcode;
		*pParam >> role;
		*pParam >> videoSyncLost;
		*pParam >> bIsGradualIntra;

		*pSeg << (WORD)Fast_Update << (WORD)role << videoSyncLost << bIsGradualIntra;// RTP report on Intra
		SendMessageFromMasterToSlave(partyPosition, RMTH230, pSeg);
		POBJDELETE(pSeg);
	}
	else
		DBGPASSERT(tipPosition+100);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSlaveSendRtcpFastUpdate(CSegment* pParam)
{
	ERoleLabel 	eRole;
//	DWORD 		remoteSSRC = 0;
//	DWORD 		priorityID = 0;

	ETipPartyTypeAndPosition sourceParty = eTipNone;
	*pParam >> (DWORD&)sourceParty;
	*pParam >> (DWORD&)eRole;
//	*pParam >> (DWORD&)remoteSSRC;
//	*pParam >> (DWORD&)priorityID;

	CSmallString str;
	str << "sourceParty = " << sourceParty << ", role = " << eRole;// << ", remoteSSRC=" << remoteSSRC << ", priorityID=" << priorityID;
	PTRACE2(eLevelInfoNormal, "CSipParty::OnSlaveSendRtcpFastUpdate", str.GetString());

	ETipVideoPosition videoPos = ::GetVideoPosition(sourceParty);

	if (eRole & kRoleContentOrPresentation)
		videoPos = eTipVideoPosAux5Fps;

	m_pSipCntl->FastUpdateReq(eRole, videoPos);//, remoteSSRC, priorityID);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendMessageFromMasterToSlaveByID(DWORD rsrcId, DWORD opcode, CSegment *pMsg)
{
	ETipPartyTypeAndPosition tiptype=eTipNone;
	if( rsrcId == m_SlaveLeftRsrcId && m_SlaveLeftRsrcId !=0 )
		tiptype = eTipSlaveLeft;
	else if( rsrcId == m_SlaveRightRsrcId && m_SlaveRightRsrcId!=0)
		tiptype = eTipSlaveRigth;
	else if( rsrcId ==  m_SlaveAuxRsrcId && m_SlaveAuxRsrcId !=0)
		tiptype = eTipSlaveAux;

	if( tiptype != eTipNone )
		m_pConfApi->PartytoPartyCntlMsgFromMasterToSlave(GetPartyId(), tiptype, opcode, pMsg);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendMessageFromMasterToSlave(WORD destTipPartyType, DWORD opcode, CSegment *pMsg)
{
	m_pConfApi->PartytoPartyCntlMsgFromMasterToSlave(GetPartyId(), destTipPartyType, opcode, pMsg);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendMessageFromSlaveToMaster(WORD srcTipPartyType, DWORD opcode, CSegment *pMsg)
{
	m_pConfApi->PartyToPartyCntlMsgFromSlaveToMaster(GetPartyId(), srcTipPartyType, opcode, pMsg);

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::AddSlaveParty(DWORD srcTipPartyType)
{
	PTRACE2INT(eLevelInfoNormal, "CSipParty::AddSlaveParty ", srcTipPartyType);

	DWORD slaveRsrcId = GetSlaveRsrcIdAccordingToSlaveType(srcTipPartyType);
	PTRACE2INT(eLevelInfoNormal, "CSipParty::AddSlaveParty slaveRsrcId= ", slaveRsrcId);

	if(!slaveRsrcId)
	m_pConfApi->AddSlaveParty(GetPartyId(), srcTipPartyType);
	else
	{
		if (IsValidTimer(CREATE_SLAVE_ACK_TOUT))
			DeleteTimer(CREATE_SLAVE_ACK_TOUT);
		PASSERTMSG_AND_RETURN(1, "CSipParty::AddSlaveParty Slave Already Exists!!!");
	}

}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnMessageFromMasterToSlave(CSegment * pParam)
{
	DWORD opcode1, rsrcId, opcode2, temp[4];
	*pParam >> rsrcId >> opcode2 >> opcode1;

	TRACEINTO << "Opcode1:" << CProcessBase::GetProcess()->GetOpcodeAsString(opcode1) << ", Opcode2:" << opcode2 << ", rsrcId:" << rsrcId;

	if (opcode1 != PARTYDISCONNECT)
		*pParam >> temp[0] >> temp[1] >> temp[2] >> temp[3];

	switch (opcode1)
	{
		case PARTYDISCONNECT:
		{
			//Olga - send ACK to master
			SendMessageFromSlaveToMaster(m_tipPartyType, DISCONNECT_SLAVE_ACK, NULL);
			StartTimer(DISCONNECT_SLAVE_PARTY_TOUT, 15 * SECOND);
			TellConfOnDisconnecting(NO_DISCONNECTION_CAUSE, NULL);
			break;
		}

		case IP_CM_PARTY_MONITORING_IND:
		case CONF_PARTY_MRMP_PARTY_MONITORING_IND:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam);  //Olga: need to copy because pParam read offset is not 0
			m_pSipCntl->OnPartyMonitoringInd(&newSeg);
			break;
		}

		case TIP_REMOTE_SENT_RE_CAPS_FOR_SLAVE:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam);  //Olga: need to copy because pParam is not good!

			CSipCaps *pCurRemoteCaps = new CSipCaps;
			CIpComMode *pBestMode = new CIpComMode;

			pCurRemoteCaps->DeSerialize(NATIVE, newSeg);
			pBestMode->DeSerialize(NATIVE, newSeg);

			// remove from slave unrelaMuteMediaIfNeededted media type before sending recaps to PartyControl
			if (GetTipPartyTypeAndPosition() == eTipSlaveAux)
			{
				TRACEINTO << "Remove video from aux caps";
				pCurRemoteCaps->CleanMedia(cmCapVideo, kRolePeople);
				pBestMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
			}

			if (pBestMode->IsContent(cmCapReceiveAndTransmit))
			{
				TRACEINTO << "Remove content from slaves caps";
				pCurRemoteCaps->CleanMedia(cmCapVideo, kRoleContent);
				pBestMode->RemoveContent(cmCapReceiveAndTransmit);
			}

			if (pBestMode->IsMediaOn(cmCapData, cmCapReceive))
				pBestMode->SetMediaOff(cmCapData, cmCapReceive);

			if (pBestMode->IsMediaOn(cmCapData, cmCapReceive))
				pBestMode->SetMediaOff(cmCapData, cmCapTransmit);

			CSegment seg;
			pCurRemoteCaps->Serialize(NATIVE, seg);
			pBestMode->Serialize(NATIVE, seg);

			WORD isFallBckFromTip = 0;// slave doesn't performe fallback
			seg << (WORD)isFallBckFromTip;

			seg << (WORD)0; //bIsGlare

			DispatchEvent(SIP_TRANS_PARTY_RECEIVE_RECAP, &seg);

			POBJDELETE(pCurRemoteCaps);
			POBJDELETE(pBestMode);
			break;
		}

		case TIP_MUTE_SLAVE_IF_NEEDED:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam);  //Olga: need to copy because pParam is not good!

			BYTE bIsForceMute;
			BYTE muteMask;
			DWORD eDirection;

			*pParam >> eDirection >> bIsForceMute >> muteMask >> m_bIsPolycomFromRTCP;

			TRACEINTO << "eDirection:" << eDirection << ", IsForceMute:" << (int)bIsForceMute << ", MuteMask" << (int)muteMask << ", IsPolycomFromRTCP:" << (int)m_bIsPolycomFromRTCP;

			MuteMediaIfNeeded((cmCapDirection)eDirection, bIsForceMute, muteMask);
			break;
		}

		case RMTH230:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam);  //Olga: need to copy because pParam read offset is not 0
			DispatchEvent(RMTH230, &newSeg);
			break;
		}

		case DTMF_STRING_IDENT:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam);  //Olga: need to copy because pParam read offset is not 0
			if (m_ivrCtrl)
			{
				//m_ivrCtrl->HandleEvent(newSeg, newSeg->GetLen(), opcode1);
			}
			break;
		}

		case SIP_TRANS_SLAVE_CLOSE_CHANS:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam);  //Olga: need to copy because pParam read offset is not 0
			DispatchEvent(SIP_TRANS_SLAVE_CLOSE_CHANS, &newSeg);
			break;
		}

		case SIP_TRANS_SLAVE_OPEN_CHANS:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam);  //Olga: need to copy because pParam read offset is not 0

			CIpComMode *pBestMode = new CIpComMode;
			DWORD eDirection;

			newSeg >> eDirection;
			pBestMode->DeSerialize(NATIVE, newSeg);

			// remove from slave unrelaMuteMediaIfNeededted media type before sending recaps to PartyControl
			if (GetTipPartyTypeAndPosition() == eTipSlaveAux)
			{
				TRACEINTO << "Remove video from aux caps";
				pBestMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
			}

			if (pBestMode->IsContent(cmCapReceiveAndTransmit))
			{
				TRACEINTO << "Remove content from slaves caps";
				pBestMode->RemoveContent(cmCapReceiveAndTransmit);
			}

			CSegment seg;
			seg << eDirection;
			pBestMode->Serialize(NATIVE, seg);

			DispatchEvent(SIP_TRANS_SLAVE_OPEN_CHANS, &seg);

			POBJDELETE(pBestMode);
			break;
		}

		case TIP_UPDATE_RTP_FOR_SLAVE: //_dtls_
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam); //Olga: need to copy because pParam is not good!

			m_pTargetMode->DeSerialize(NATIVE, newSeg);
			SendUpdateChannelsRtpIfNeeded();
			break;
		}

		case EXTERNAL_IVR_DIALOG_START: //BRIDGE-8290
		case EXTERNAL_IVR_MESSAGE_FOR_TIP_SLAVE: //BRIDGE-3510
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam); //Olga: need to copy because pParam is not good!

			DialogState state;
			newSeg >> state;

			ExternalIvrStartDialog(state);
			break;
		}

		case EXTERNAL_IVR_DIALOG_TIMEOUT_MASTER_TO_SLAVE:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam); //Olga: need to copy because pParam is not good!

			if (m_ivrCtrl)
				m_ivrCtrl->HandleEvent(&newSeg, newSeg.GetLen(), opcode1);

			break;
		}

		case EXTERNAL_IVR_DTMF_BARGE_IN_MASTER_TO_SLAVE:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam); //Olga: need to copy because pParam is not good!

			if (m_ivrCtrl)
				m_ivrCtrl->HandleEvent(&newSeg, newSeg.GetLen(), opcode1);

			break;
		}

		case EXTERNAL_IVR_PLAY_MESSAGE_AUX: //BRIDGE-3510
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam); //Olga: need to copy because pParam is not good!

			DialogState state;
			MediaElementType* media;
			newSeg >> state >> (void*&)media;
			PASSERT_AND_RETURN(!media);

			PlayFileExternalIvr(state, *media);
			break;
		}

		case CHANGE_VIDEO_OUT_TIP_POLYCOM: //_t_p_
		{
			m_pConfApi->ChangeVideoOutForTipPolycom(GetPartyId());
			break;
		}

		case START_IVR:
		{
			BOOL isResume = FALSE;
			if (pParam)
			{
				CSegment newSeg;
				newSeg.CopySegmentFromReadPosition(*pParam); //Olga: need to copy because pParam read offset is not 0
				newSeg >> isResume;
			}
			StartIvr(isResume);
			break;
		}

		case TIP_MASTER_STARTED_IVR:
		{
			PASSERTMSG_AND_RETURN(!IsValidPObjectPtr(m_ivrCtrl), "Party's IVR control ptr is invalid");

			if (GetIsTipCall() && m_tipPartyType >= eTipMasterCenter)
			{
				if ((m_ivrCtrl->GetState() == ACTIVE) && (m_ivrCtrl->GetStage() > 0))
				{
					TRACEINTO << "IVR is already started, nothing to do";
				}
				else
					m_ivrCtrl->HandleEvent(NULL, 0, opcode1);
			}
			else
			{
				TRACEINTO << "TipPartyType:" << m_tipPartyType << " - Invalid TIP EP type, nothing to do";
				return;
			}
			break;
		}

		case PLAY_MESSAGE_AUX:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam); //Olga: need to copy because pParam is not good!
			if (m_pConfApi)
				m_pConfApi->StartMessageTipAux(m_pParty->GetPartyRsrcID(), &newSeg);
			break;
		}

		case PLAY_ROLL_CALL_AUX:
		{
			CSegment newSeg;
			newSeg.CopySegmentFromReadPosition(*pParam); //Olga: need to copy because pParam is not good!

			WORD NumberOfMessagesToPlay;
			WORD cachePriority;
			IVRMsgDescriptor* arrayOfMessagesToPlay;

			newSeg >> (void*&)arrayOfMessagesToPlay >> NumberOfMessagesToPlay >> cachePriority;

			if (m_pConfApi)
				m_pConfApi->StartRecordMessage(m_pParty->GetPartyRsrcID(), arrayOfMessagesToPlay, NumberOfMessagesToPlay, cachePriority, false);
			break;
		}

		case STOP_PLAY_MESSAGE_AUX:
		{
			if (m_pConfApi)
				m_pConfApi->StopMessage(m_pParty->GetPartyRsrcID());
			break;
		}

		case MASTER_END_FEATURES:
		{
			if (m_ivrCtrl)
				m_ivrCtrl->HandleEvent(NULL, 0, opcode1);
			break;
		}

		case TIP_MASTER_PARTY_MUTE:
		{
			m_pConfApi->SendCAMGeneralActionCommand(m_pParty->GetPartyRsrcID(), EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_MUTE, 0);
			break;
		}

		case TIP_MASTER_PARTY_UNMUTE:
		{
			m_pConfApi->SendCAMGeneralActionCommand(m_pParty->GetPartyRsrcID(), EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_UNMUTE, 0);
			break;
		}

		case TIP_MASTER_PARTY_PLAY_MENU:
		{
			m_pConfApi->SendCAMGeneralActionCommand(m_pParty->GetPartyRsrcID(), EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_PLAY_MENU, 0);
			break;
		}

		case TIP_MASTER_PARTY_INC_VOLUME:
		{
			BYTE bVolumeInOut;
			*pParam >> bVolumeInOut;
			m_pConfApi->SendCAMGeneralActionCommand(m_pParty->GetPartyRsrcID(), EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_INC_VOLUME, bVolumeInOut);
			break;
		}

		case TIP_MASTER_PARTY_DEC_VOLUME:
		{
			BYTE bVolumeInOut;
			*pParam >> bVolumeInOut;
			m_pConfApi->SendCAMGeneralActionCommand(m_pParty->GetPartyRsrcID(), EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_DEC_VOLUME, bVolumeInOut);
			break;
		}

		default:
			PTRACE(eLevelInfoNormal, "CSipParty::OnMessageFromMasterToSlave - unknown opcode");
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnMessageFromSlaveToMaster(CSegment * pParam)
{
    DWORD rsrcId, temp1, temp2, opcode, tiptype;
	//ETipPartyTypeAndPosition tiptype=eTipNone;

	*pParam >> temp1 >> temp2 >> tiptype >> opcode;

	if( eTipSlaveLeft == tiptype )
		rsrcId = m_SlaveLeftRsrcId;
	else if( eTipSlaveRigth == tiptype )
		rsrcId = m_SlaveRightRsrcId;
	else if( eTipSlaveAux == tiptype )
		rsrcId = m_SlaveAuxRsrcId;
	else
	{
		PASSERTSTREAM(TRUE, "tiptype " << tiptype << " is invalid??");
		rsrcId = 0xFFFFFFFF;
	}

	TRACEINTO << "CSipParty::OnMessageFromSlaveToMaster: temp1=" << temp1 << ", temp2="
			<< temp2 << ", opcode=" << opcode<< ", tip type=" << tiptype << ", rsrcId=" << rsrcId;

	switch (opcode)
	{
		case SLAVE_SEND_MONITORING_REQ:
		{
			PTRACE2INT(eLevelInfoNormal, "CSipParty::OnMessageFromSlaveToMaster : monitoring of ", tiptype);

			BYTE isMonitorReqSent = m_pSipCntl->PartyMonitoringReq(rsrcId);

			if (isMonitorReqSent)
				PTRACE(eLevelInfoNormal,"CSipParty::OnMessageFromSlaveToMaster - No request was sent - Party is not connected yet");

			break;
		}

		case SLAVE_SEND_RTCP_FAST_UPDATE:
		{
			ERoleLabel 					eRole;
//			DWORD 						remoteSSRC = 0;
//			DWORD 						priorityID = 0;
			ETipPartyTypeAndPosition 	sourceParty = eTipNone;

			DWORD  temp1, temp2, temp3, temp4, temp5;

			*pParam >> temp3 >> temp4;
			*pParam >> (DWORD&)sourceParty;
			*pParam >> temp5;
			*pParam >> (DWORD&)eRole;
//			*pParam >> (DWORD&)remoteSSRC;
//			*pParam >> (DWORD&)priorityID;

			CSmallString str;
			str << "sourceParty = " << sourceParty << ", role = " << eRole;// << ", remoteSSRC=" << remoteSSRC << ", priorityID=" << priorityID;

			TRACEINTO << " CSipParty::OnMessageFromSlaveToMaster : FastUpdate " << ", temp3=" << temp3 << ", temp4="<< temp4 << ", temp5=" << temp5;
			PTRACE2(eLevelInfoNormal, "CSipParty::OnSlaveSendRtcpFastUpdate ", str.GetString());

			ETipVideoPosition videoPos = ::GetVideoPosition(sourceParty);

			if (eRole & kRoleContentOrPresentation)
				videoPos = eTipVideoPosAux5Fps;

			m_pSipCntl->FastUpdateReq(eRole, videoPos);//, , remoteSSRC, priorityID);

			break;
		}

		case PARTY_SLAVE_TO_MASTER_RECAP_ACK:
		{
			PTRACE2INT(eLevelInfoNormal, "CSipParty::OnMessageFromSlaveToMaster : PARTY_SLAVE_TO_MASTER_RECAP_ACK, tip type: ", tiptype);

			m_nSlavesReturnRecapAck -= 1;

			PTRACE2INT(eLevelInfoNormal, "CSipParty::OnMessageFromSlaveToMaster : PARTY_SLAVE_TO_MASTER_RECAP_ACK, m_nSlavesReturnRecapAck: ", m_nSlavesReturnRecapAck);

			if (!m_nSlavesReturnRecapAck)//IsAllSlavesReturnRecapAck())
			{
				PTRACE(eLevelInfoNormal, "CSipParty::OnMessageFromSlaveToMaster : kill TIP_SLAVE_RECAP_TOUT");

				if (IsValidTimer(TIP_SLAVE_RECAP_TOUT))
					DeleteTimer(TIP_SLAVE_RECAP_TOUT);

				CSegment* pSeg = new CSegment;

				DispatchEvent(SIP_PARTY_SLAVES_RECAP_FINISHED, pSeg);

				POBJDELETE(pSeg);

				m_nSlavesReturnRecapAck = -1;

			}

			break;
		}
		case EXTERNAL_IVR_PLAY_MEDIA_COMPLETE_SLAVE_TO_MASTER:
		{
			TRACEINTO << "EXTERNAL_IVR_PLAY_MEDIA_COMPLETE_SLAVE_TO_MASTER";
			std::string dialogId, status;
			DWORD  temp1, temp2, temp3, temp4;

			*pParam >> temp1 >> temp2 >> temp3 >> temp4;
			*pParam >> dialogId >> status;
			CSegment *newParam = new CSegment();
			*newParam << dialogId << status;
			m_ivrCtrl->HandleEvent(newParam, newParam->GetLen(), EXTERNAL_IVR_PLAY_MEDIA_COMPLETE_SLAVE_TO_MASTER);
			POBJDELETE(newParam);
			break;
		}
		case PLAY_MESSAGE_ACK_SLAVE_TO_MASTER:
		{
			TRACEINTO << "PLAY_MESSAGE_ACK_SLAVE_TO_MASTER";
			m_ivrCtrl->RecivedPlayMessageAck();
			break;
		}

		case RECORD_PLAY_MESSAGE_ACK_SLAVE_TO_MASTER:
		{
			TRACEINTO << "RECORD_PLAY_MESSAGE_ACK_SLAVE_TO_MASTER";


			CIVRService* pIvrService = ((CIvrCntlLocal*)m_ivrCtrl)->GetIVRService();
			CIvrSubBaseSM* pIvrSubGenSM = m_ivrCtrl->GetIvrSubGenSM();

			if(pIvrSubGenSM == NULL)
				TRACEINTO << "pIvrSubGenSM is NULL";
			else
			{
				((CIvrSubRollCall*)pIvrSubGenSM)->PlayOnlyRecordMessageWithoutPlayMessage();
			}

			break;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSendNewNameWithRoomId(CSegment * pParam)
{
	  char name[H243_NAME_LEN];
	  char confName[H243_NAME_LEN];
	  *pParam >> name >> confName;
	  strncpy(m_name, name, H243_NAME_LEN);
	  PTRACE2 (eLevelInfoNormal, "CSipParty::OnSendNewNameWithRoomId ", name);
	  PTRACE2 (eLevelInfoNormal, "CSipParty::OnSendNewNameWithRoomId ", confName);

	  CCommConf* pCommConf 	= ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
	  if (pCommConf)
	  {
	      CConfParty* pConfParty = pCommConf->GetCurrentParty(GetMonitorPartyId());
	      if (pConfParty)
	          pConfParty->SetTelePresenceMode( (BYTE)eTelePresencePartyCTS );
	      else
	          PTRACE(eLevelError,"CSipParty::OnSendNewNameWithRoomId - pConfParty is NULL - can not SetTelePresenceMode");
	  }
	  else
	      PTRACE(eLevelError,"CSipParty::OnSendNewNameWithRoomId - pCommConf is NULL - can not SetTelePresenceMode");
	//  SetFullName(m_name,confName);
}

//_e_m_
/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateVidBrdgTelepresenseEPInfo(eTelePresencePartyType telePresencePartyType)
{
    PASSERTMSG_AND_RETURN(!m_pConfApi, "m_pConfApi == NULL");

	CTelepresenseEPInfo telepresenseEPInfo;

	telepresenseEPInfo.SetEPtype(telePresencePartyType);
	telepresenseEPInfo.SetLinkNum(0);
	telepresenseEPInfo.SetLinkRole(0);
	telepresenseEPInfo.SetNumOfLinks(m_TipNumOfStreams);
	telepresenseEPInfo.SetPartyMonitorID(GetMonitorPartyId());
	telepresenseEPInfo.SetRoomID(m_RoomId);

	//temp printing
	CMedString str = "";
	str << " EPtype " 		  << (int)telepresenseEPInfo.GetEPtype()
		<< " LinkNum " 		  << (int)telepresenseEPInfo.GetLinkNum()
		<< " LinkRole " 	  << (int)telepresenseEPInfo.GetLinkRole()
		<< " NumOfLinks " 	  << (int)telepresenseEPInfo.GetNumOfLinks()
		<< " PartyMonitorID " << (int)telepresenseEPInfo.GetPartyMonitorID()
		<< " RoomID " 		  << (int)telepresenseEPInfo.GetRoomID();

	PTRACE2(eLevelInfoNormal, "EMB_MLA : CSipParty::UpdateVidBrdgTelepresenseEPInfo - telepresenseEPInfo: ", str.GetString());

	m_pConfApi->UpdateVidBrdgTelepresenseEPInfo(GetPartyRsrcID(), &telepresenseEPInfo);
}
/////////////////////////////////////////////////////////////////////////////
DWORD CSipParty::GetSlaveRsrcIdAccordingToSlaveType(DWORD tipType)// N.A.
{
	DWORD  rsrcId = 0;
	switch (tipType)
	{
		case eTipSlaveLeft:

			rsrcId = m_SlaveLeftRsrcId;
			PTRACE2INT (eLevelInfoNormal, "CSipParty::GetSlavePartySlaveType m_SlaveLeftRsrcId = ", m_SlaveLeftRsrcId);
			break;

		case eTipSlaveRigth:

			rsrcId = m_SlaveRightRsrcId;
			PTRACE2INT (eLevelInfoNormal, "CSipParty::GetSlavePartySlaveType m_SlaveRightRsrcId = ", m_SlaveRightRsrcId);
			break;

		case eTipSlaveAux:

			rsrcId = m_SlaveAuxRsrcId;
			PTRACE2INT (eLevelInfoNormal, "CSipParty::GetSlavePartySlaveType m_SlaveAuxRsrcId = ", m_SlaveAuxRsrcId);
			break;
	}


	return rsrcId;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::SetSlavePartyRsrcId(CSegment * pParam)
{
	DWORD tipType, rsrcId;
	WORD NumOfAckFromSlaves = 0;

	*pParam >> tipType >> rsrcId;

	TRACEINTO << " CSipParty::SetSlavePartyRsrcId : type="<< tipType << ", rsrcId=" << rsrcId;

	switch (tipType)
	{
	case eTipSlaveLeft:

		m_SlaveLeftRsrcId = rsrcId;
		//m_NumOfAckFromSlaves++;
		PTRACE2INT (eLevelInfoNormal, "CSipParty::SetSlavePartyRsrcId m_NumOfAckFromSlaves - 1 - slave id=", m_SlaveLeftRsrcId);

		break;

	case eTipSlaveRigth:

		m_SlaveRightRsrcId = rsrcId;
		//m_NumOfAckFromSlaves++;
		PTRACE2INT (eLevelInfoNormal, "CSipParty::SetSlavePartyRsrcId m_NumOfAckFromSlaves - 2 - slave id=", m_SlaveRightRsrcId);

		break;

	case eTipSlaveAux:

		m_SlaveAuxRsrcId = rsrcId;
		//m_NumOfAckFromSlaves++;
		PTRACE2INT (eLevelInfoNormal, "CSipParty::SetSlavePartyRsrcId m_NumOfAckFromSlaves - 3 - slave id=", m_SlaveAuxRsrcId);

		break;
	}

    WORD numOfSlaves = m_TipNumOfStreams -1 + m_bIsAudioAux;

	if (((numOfSlaves == 3) &&
		 (m_PartyRsrcID != 0 && m_SlaveLeftRsrcId != 0 && m_SlaveRightRsrcId != 0 && m_SlaveAuxRsrcId != 0 && (0 == m_NumOfAckFromSlaves))) || // video
		 ((numOfSlaves == 1) &&
		 (m_PartyRsrcID != 0 && m_SlaveAuxRsrcId!=0) && (0 == m_NumOfAckFromSlaves)))//audio only

	{
		TRACEINTO << "CSipParty::SetSlavePartyRsrcId numOfSlaves " << numOfSlaves << " m_NumOfAckFromSlaves "<< (WORD)m_NumOfAckFromSlaves << " " << m_TipNumOfStreams << " " << (WORD)m_bIsAudioAux;

		m_pSipCntl->EndTipNegotiation(eTipNegSuccess);

		m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_UPDATE_ON_TIP_CALL_REQ);

		if(m_TipNumOfStreams > 1)
			UpdateVidBrdgTelepresenseEPInfo(eTelePresencePartyCTS); //_e_m_

		m_NumOfAckFromSlaves = 1;

		if (IsValidTimer(CREATE_SLAVE_ACK_TOUT))
			DeleteTimer(CREATE_SLAVE_ACK_TOUT);

		m_bTipEndSuccessSent = TRUE;

		UnmuteMediaAndSendReinviteForTipIfNeeded();
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnReceivedTipLastAck()
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnReceivedTipLastAck");

	m_bTipLastAckReceived  = TRUE;

	if(m_bIsTipResumed)
		ResumeMediaAndSendReinviteForTip();
	else
		UnmuteMediaAndSendReinviteForTipIfNeeded();

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SetTipPartyOnHold(BOOL isOnHold)
{
	m_bIsTipPutOnHold = isOnHold;
	if (isOnHold && IsValidPObjectPtr(m_ivrCtrl) && !m_ivrCtrl->IsExternalIVR())
	{
		// BRIDGE-8051 resume IVR 'waiting for chair' after hold
		m_ivrCtrl->HandleEvent(NULL, 0, PARTY_ON_HOLD_IND);
		if (m_pConfApi)
		{
			CSegment seg;
			GetRcvMbx().Serialize(seg);
			seg << (WORD)m_ivrCtrl->IsIvrOnHold();
			m_pConfApi->IvrPartyNotification(GetPartyRsrcID(), this, GetName(), PARTY_ON_HOLD_IND, &seg, eMediaInAndOut);
		}
		TRACEINTO << " party " << GetName() << " is put on hold- informing ConfAppMngr and party IVR control.";
	}
}
/////////////////////////////////////////////////////////////////////////////
BOOL CSipParty::GetTipPartyOnHold()
{
	return m_bIsTipPutOnHold;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::UnmuteMediaAndSendReinviteForTipIfNeeded()
{
	TRACEINTO << "IsLastAckReceived:" << (int)m_bTipLastAckReceived << ", IsEndSuccessSent:" << (int)m_bTipEndSuccessSent;

	if (!m_bTipLastAckReceived || !m_bTipEndSuccessSent)
		return;

	DBGPASSERT(IsActiveTransaction());

	SetIsTipNegotiationActive(FALSE);

	MuteMediaIfNeeded(cmCapReceiveAndTransmit, FALSE);
	m_pSipCntl->SetIsResuming(FALSE); // turn off when TIP negotiation is done

	if (m_bIsNeedTipReinviteAfterNegotiation)
		m_pSipCntl->TipReInviteReq(m_TipNumOfStreams, m_bIsVideoAux);

	BOOL bIsResume = FALSE;
	StartIvr(bIsResume);
	CSegment seg;
	seg << bIsResume;
	ForwardEventToTipSlaves(&seg, START_IVR);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::FallBackToRegularSip(RemoteIdent rmtIdent)
{
	PTRACE(eLevelInfoNormal, "CSipParty::FallBackToRegularSip");
	m_bIsTipCall = NO;

	if (IsValidTimer(CREATE_SLAVE_ACK_TOUT))
		DeleteTimer(CREATE_SLAVE_ACK_TOUT);

	CCommConf* pCommConf 	= ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
	if (pCommConf)
	{
	    CConfParty* pConfParty 	= pCommConf->GetCurrentParty(GetMonitorPartyId());
	    if (pConfParty)
	    {
	        pConfParty->SetIsTipCall(FALSE);
	        pConfParty->SetTIPPartyType(eTipPartyNone);
	    }
	    else
	        PTRACE(eLevelInfoNormal, "CSipParty::FallBackToRegularSip - pConfParty is NULL");
	}
	else
	    PTRACE(eLevelInfoNormal, "CSipParty::FallBackToRegularSip - pCommConf is NULL");

	SetTipPartyTypeAndPosition(eTipNone);
	MuteMediaIfNeeded(cmCapReceiveAndTransmit, FALSE);
	SetIsTipCall(FALSE);
	m_pTargetMode->SetTipAuxFPS(eTipAuxNone);
	m_pTargetModeMaxAllocation->SetTipAuxFPS(eTipAuxNone);
	m_pTargetMode->SetTipMode(eTipModeNone);
	m_pTargetModeMaxAllocation->SetTipMode(eTipModeNone);


	CSipCall* currCall = m_pSipCntl->GetCallObj();

	if(currCall)
	{
		currCall->SetIsTipCall(FALSE);
        if( currCall->GetChannel(FECC_IN) == NULL &&  currCall->GetChannel(FECC_OUT) == NULL)
        {
             PTRACE(eLevelInfoNormal, "CSipParty::FallBackToRegularSip -remove FECC ");
             m_pSipCntl->RemoveFeccCaps();
        }

	}

	if (m_pSipCntl->GetRemoteIdent() == CiscoCucm)
	{
		CLargeString str;

		str << "curr remote ident: " << m_pSipCntl->GetRemoteIdent() << " new remote ident: " << rmtIdent;
		PTRACE2(eLevelInfoNormal, "CSipParty::FallBackToRegularSip - ", str.GetString());

		m_pSipCntl->SetRemoteIdent(rmtIdent);
	}

	m_pSipCntl->AddBfcpOnFallback(m_pTargetMode, m_pTargetModeMaxAllocation);

	//TEMP NOA CAUSE A CORE DUMP TO CHECK
	//PTRACE(eLevelInfoNormal, "CSipParty::FallBackToRegularSip - pCommConf is NULL -HAVE A CORE DUMP SO NOT FALLING BACK -NEED TO CHECK");
	m_pConfApi->SipPartySendFallBackToSIP(GetPartyId(), m_pTargetMode);

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSlaveSendMonitoringReq(CSegment* pParam)
{
	ETipPartyTypeAndPosition sourceParty = eTipNone;
	*pParam >> (DWORD&)sourceParty;
	PTRACE2INT(eLevelInfoNormal, "CSipParty::OnSlaveSendRtcpFastUpdate sourceParty = ", sourceParty);
//	ETipVideoPosition videoPos = ::GetVideoPosition(sourceParty);
//	m_pSipCntl->FastUpdateReq(eRole, videoPos);
}

//////////////////////////////////////////////////////////////////////
void CSipParty::SetContentEncryptionForTipVideoAux(CSipComMode* pNewMediaMode)
{
	PASSERT_AND_RETURN(pNewMediaMode == NULL);

	PTRACE(eLevelInfoNormal, "CSipParty::SetContentEncryptionForTipVideoAux");

	pNewMediaMode->SetIsEncrypted(m_pTargetMode->GetIsEncrypted(), m_pTargetMode->GetIsDisconnectOnEncryptionFailure());
	pNewMediaMode->SetDtlsEncryption(m_pTargetMode->GetIsDtlsEncrypted());
	pNewMediaMode->SetDtlsAvailable(m_pTargetMode->GetIsDtlsAvailable());

	//Get video modes
	CMediaModeH323& videoModeTx = m_pTargetMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);
	CMediaModeH323& videoModeRx = m_pTargetMode->GetMediaMode(cmCapVideo, cmCapReceive, kRolePeople);

	//Transmit - Set content encrypt same as video encrypt
	CMediaModeH323& newContentModeTx = pNewMediaMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	CMediaModeH323& targetNewContentModeTx = m_pTargetMode->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	newContentModeTx.SetDtlsCap(videoModeTx.GetDtlsCap());
	newContentModeTx.SetSdesCap(videoModeTx.GetSdesCap());
	targetNewContentModeTx.SetDtlsCap(videoModeTx.GetDtlsCap());
	targetNewContentModeTx.SetSdesCap(videoModeTx.GetSdesCap());

	//Receive - Set content encrypt same as video encrypt
	CMediaModeH323& newContentModeRx = pNewMediaMode->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	CMediaModeH323& targetNewContentModeRx = m_pTargetMode->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
	newContentModeRx.SetDtlsCap(videoModeRx.GetDtlsCap());
	newContentModeRx.SetSdesCap(videoModeRx.GetSdesCap());
	targetNewContentModeRx.SetDtlsCap(videoModeRx.GetDtlsCap());
	targetNewContentModeRx.SetSdesCap(videoModeRx.GetSdesCap());
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::SetScmAndChannelsForTipNegotiation()
{
	PTRACE(eLevelInfoNormal, "CSipParty::UpdateScmAndChannelsForTipNegotiation");

	//remove from pcall bfcp
    CSipCall *pCall = m_pSipCntl->GetCallObj();
    PASSERTMSG_AND_RETURN(NULL == pCall, "CSipParty::SetScmAndChannelsForTipNegotiation - GetCallObj() return NULL!");

	// remove BFCP in TIP call
	m_pSipCntl->RemoveBfcpCaps();

	if(pCall && pCall->GetChannel(FECC_IN) == NULL &&  pCall->GetChannel(FECC_OUT) == NULL)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::UpdateScmAndChannelsForTipNegotiation -remove FECC ");
		m_pSipCntl->RemoveFeccCaps();

	}

	m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
	m_pTargetModeMaxAllocation->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);

	if (m_bIsVideoAux)
	{
	    PTRACE(eLevelInfoNormal, "IS_PREFER_TIP_MODE: CSipParty::UpdateScmAndChannelsForTipNegotiation - first, close BFCP channels");

	    CSipChannel* pBfcpChannelIn;
	    CSipChannel* pBfcpChannelOut;

	    if (pCall)
	    {
			pBfcpChannelIn = pCall->GetChannel(BFCP_IN);
			pBfcpChannelOut = pCall->GetChannel(BFCP_OUT);

			if (pBfcpChannelIn)
				m_pSipCntl->SipCloseChannelReq(BFCP_IN);

			if (pBfcpChannelOut)
				m_pSipCntl->SipCloseChannelReq(BFCP_OUT);

			if (!pBfcpChannelIn && !pBfcpChannelOut)
			{
				CSipChannel* pContentChannelIn = pCall->GetChannel(VIDEO_CONT_IN);
				CSipChannel* pContentChannelOut = pCall->GetChannel(VIDEO_CONT_OUT);

				if (!pContentChannelIn && !pContentChannelOut)
				{
					PTRACE(eLevelInfoNormal,"CSipParty::UpdateScmAndChannelsForTipNegotiation - open content channels");
					CSipComMode* pNewMediaMode = new CSipComMode;

				m_pSipCntl->SetRemoteCapsTipAuxFPS(eTipAux5FPS);

				const CCommConf*  pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);

				if (pCommConf && pCommConf->GetIsPreferTIP())
				{
					m_pTargetMode->SetTIPContent(0, cmCapReceiveAndTransmit, FALSE);

					m_pTargetModeMaxAllocation->SetTIPContent(0, cmCapReceiveAndTransmit, FALSE);

					pNewMediaMode->SetTIPContent(0, cmCapReceiveAndTransmit, FALSE);
				}
				else //TipCompatibility:video&content!
				{
					m_pTargetMode->SetTIPContent(0, cmCapReceiveAndTransmit);

					m_pTargetModeMaxAllocation->SetTIPContent(0, cmCapReceiveAndTransmit);

					pNewMediaMode->SetTIPContent(0, cmCapReceiveAndTransmit);
				}

			SetContentEncryptionForTipVideoAux(pNewMediaMode);

					m_pSipCntl->SipOpenChannelsReq(pNewMediaMode, cmCapReceiveAndTransmit, FALSE, eTipMasterCenter);

					POBJDELETE(pNewMediaMode);
				}
				else
				{
					PTRACE(eLevelInfoNormal,"CSipParty::UpdateScmAndChannelsForTipNegotiation - update bridges");
					UpdateBridgesForTipNegotiation();
				}
			}
	}
	}
	else
	{
		m_pTargetMode->SetTipAuxFPS(eTipAuxNone);

		m_pTargetModeMaxAllocation->SetTipAuxFPS(eTipAuxNone);

		m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

		m_pTargetModeMaxAllocation->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

		m_pSipCntl->SetRemoteCapsTipAuxFPS(eTipAuxNone);

		 //mid call changes todo - need to set presentation mode off and close the content channels if needed. // tiptmp
		UpdateBridgesForTipNegotiation();
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateBridgesForTipNegotiation()
{
	PTRACE(eLevelInfoNormal, "CSipParty::UpdateBridgesForTipNegotiation");

	CSipCaps*	pCurRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());

	PTRACE2INT(eLevelInfoNormal, "CSipParty::UpdateBridgesForTipNegotiation pCurRemoteCaps->IsCapableTipAux5Fps ", pCurRemoteCaps->IsCapableTipAux5Fps());

	m_pConfApi->SendPartyReceiveReCapsToPartyControl(GetPartyId(), pCurRemoteCaps, m_pTargetMode); // TODO tmptip add timer
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfBridgesUpdated(CSegment * pParam)
{
	DWORD status = STATUS_OK;

	*pParam >> status;
	PTRACE2INT(eLevelInfoNormal, "CSipParty::OnConfBridgesUpdated : status - ", status);
	if (status == STATUS_OK)
	{
		m_pTargetMode->DeSerialize(NATIVE,*pParam);
		UdpAddresses sUdpAddressesParams;
		pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));
		m_pSipCntl->SetNewUdpPorts(sUdpAddressesParams);
		BYTE bIsContentSpeaker = FALSE;
		*pParam >> bIsContentSpeaker;
		// TODO tiptmp - check if target was changed and update channels if needed
		InformConfRemoteConnect();

		AddTipSlaves();
	}
	else
		DBGPASSERT(status);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTimerCreateSlaveAck(CSegment* pParam)
{
	TRACEINTO << " CSipParty::OnTimerCreateSlaveAck";
	m_eDialState = kConnectTimer;
	TellConfOnDisconnecting(TIP_CREATE_TIMER_POPPED_OUT);//OnPartyConnectTout(pParam);
}
/////////////////////////////////////////////////////////////////////////////

void CSipParty::OnTimerDisconnectSlaveParty(CSegment* pParam)
{
	TRACEINTO << " CSipParty::OnTimerDisconnectSlaveParty : name=" << GetName();
	m_eDialState = kConnectTimer;
	TellConfOnDisconnecting(SIP_TIMER_POPPED_OUT);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipParty::IsOfferer() const
{
	if (IsActiveTransaction() && m_pSipTransaction->IsOfferer())
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfSetCapsAccordingToNewAllocation(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::SetCapsAccordingToNewAllocation:" );
	WORD len = 0;
	BYTE bIsAudioOnly = FALSE;
	BYTE bIsRtv = FALSE;
	DWORD videoRate = 0;
	BYTE cif4Mpi = FALSE;
	BYTE bIsMsSvc= FALSE;
	*pParam >> bIsAudioOnly;
	*pParam >> bIsRtv;
	*pParam >> bIsMsSvc;
	*pParam >> videoRate;
	*pParam >> cif4Mpi;

	if(bIsAudioOnly)
		m_pSipCntl->SetLocalCapToAudioOnly();
	else if (bIsMsSvc == FALSE)
	{
		*pParam >> len;
		H264VideoModeDetails h264VidModeDetails;
		pParam->Get((BYTE*)&h264VidModeDetails,len);
		m_pSipCntl->SetVideoParamInCaps(h264VidModeDetails, cif4Mpi,bIsRtv,videoRate);
	}
	else
	{
		*pParam >> len;
		MsSvcVideoModeDetails MsSvcVidModeDetails;
	     pParam->Get((BYTE*)&MsSvcVidModeDetails,len);
		 m_pSipCntl->SetMsSvcVideoParamInCaps(MsSvcVidModeDetails, cif4Mpi,bIsRtv,videoRate);

	}

}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnTimerSlaveRecapAck(CSegment* pParam)
{
	TRACEINTO << "CSipParty::OnTimerSlaveRecapAck";
	m_eDialState = kConnectTimer;
	TellConfOnDisconnecting(TIP_SLAVE_RECAP_TOUT);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnIceBandwidthInd(CSegment* pParam)
{
	DWORD videoRate;
	*pParam >> (DWORD&)videoRate;
	PTRACE2INT(eLevelInfoNormal, "CSipParty::OnIceBandwidthInd videoRate = ", videoRate);

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSipBandwidthAllocationStatus(CSegment* pParam)
{
	CSegment* pSeg = new CSegment;

	DWORD reqBandwidth, allocBandwidth;

	*pParam >> reqBandwidth
			>> allocBandwidth;

	*pSeg  << reqBandwidth
		   << allocBandwidth;

	m_pConfApi->UpdateDB(this, SIPALLOCATEDBANDWIDTHSTATUS,(DWORD)0, 1, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSipBandwidthReInviteNeeded(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipParty::OnSipBandwidthReInviteNeeded");
	if( !IsActiveTransaction())
		StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
	else
		m_pSipTransaction->SetNeedReInviteForBandwidth(TRUE);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::IceFallbackToSip()
{
	TRACEINTO << "PartyId:" << GetPartyId();

	m_pConfApi->SipPartySendFallbackFromIceToSip(GetPartyId());
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnIceInsufficientBandwidthEvent(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal, "CSipParty::OnIceInsufficientBandwidthEvent");
	CMedString str;
	str << "Sip call terminated:  "<< PARTYNAME;
	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, CALL_END_DUE_TO_ICE_INSUFFICIENT_BANDWIDTH, MAJOR_ERROR_LEVEL,str.GetString(), TRUE);

	m_pSipCntl->CloseIceSession();
	TellConfOnDisconnecting (SIP_INSUFFICIENT_BANDWIDTH);

}

/////////////////////////////////////////////////////////////////////////////
WORD CSipParty::TipGetNumOfStreams()
{
	return m_TipNumOfStreams;
}
/////////////////////////////////////////////////////////////////////////////
/*
 * The function Mutes the audio of the slaves in case of TIP call for TPX.
 * TPX sends the same audio streams for master/slaves and due to sync error echo is heard.
*/
void CSipParty::SendMuteToSlaves()
{
	DWORD direction = cmCapReceive;
	BYTE bTipForceMuteChannels = TRUE;
	WORD numOfSlaves = m_TipNumOfStreams -1 + m_bIsAudioAux;
	CSegment *pSeg = new CSegment;
	BYTE muteMask = MASK_AUDIO;
	m_bIsPolycomFromRTCP = TRUE; /* Setting the flag to TRUE in order to avoid un-muting the Audio channels. */

	*pSeg << (DWORD) direction << (BYTE)bTipForceMuteChannels << (BYTE)muteMask << (BYTE)m_bIsPolycomFromRTCP;

	if (numOfSlaves > 1)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::MuteMediaIfNeeded, TIP slaves right & left");

		SendMessageFromMasterToSlave(eTipSlaveLeft, TIP_MUTE_SLAVE_IF_NEEDED, pSeg);
		SendMessageFromMasterToSlave(eTipSlaveRigth, TIP_MUTE_SLAVE_IF_NEEDED, pSeg);
	}

	POBJDELETE(pSeg);
}
///////////////////////////////////////////////////////////////////////////
void CSipParty::OnWaitToStartTipFallBackTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipParty::OnWaitToStartTipFallBackTout");
	BYTE isNeedToFallBack =FALSE;
	if (m_pCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive) &&  m_pCurrentMode->GetH264Profile(cmCapReceive) == H264_Profile_Main)
	    isNeedToFallBack = TRUE;
	if( ( GetIsTipCall() || isNeedToFallBack ) && !IsActiveTransaction() )
		FallBackToRegularSip(Regular);
	else if (m_isNeedToSendReInviteAfterTIPFallBack && !IsActiveTransaction())
	{
			PTRACE(eLevelInfoNormal, "CSipParty::OnWaitToStartTipFallBackTout sending reinvite after 2 sec");
			m_isNeedToSendReInviteAfterTIPFallBack = FALSE;
			StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
	}
	else
		m_isNeedToSendReInviteAfterTIPFallBack = FALSE;

}

///////////////////////////////////////////////////////////////////////////
void CSipParty::OnWaitToStartReinviteForContentTout(CSegment* pParam)
{
	if (!IsActiveTransaction())
	{
		if(m_pSipCntl->IsPartyInDiffPayLoadType())
		{   // wait another second for diff paylaod will over
			StartTimer(CONTENT_WAIT_REINVITE_TOUT,SECOND*1);
			TRACEINTO << "In middle of Diff Payload Type. Send Re-Invite in 1 sec";
		}
		else
		{
			TRACEINTO << " sending Re-Invite after 1 sec";
			StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
		}
	}
	else
		TRACEINTO << "we have active transaction running";

}

/////////////////////////////////////////////////////////////////////////////
#define WAIT_300_MS 30
void CSipParty::OnRelayAskEndpointForIntra(CSegment* pParam)
{

	TRACEINTO<<"inside OnRelayAskEndpointForIntra";

	if ( IsValidTimer(FIR_PEOPLE_TIMER) &&
	     (m_pTargetMode->GetConfMediaType()!=eMixAvcSvcVsw || m_pSipCntl->GetIsMrcCall()))
	{
		SaveIncomingIntraRequest(pParam);
		TRACEINTO<<"fir people timer already exists. Intra request was saved";
		return;
	}

	m_firPeopleCounter = 0;
//	POBJDELETE(m_pFirPeopleSegnent);
	CSegment*				pFirPeopleSegment;
	DWORD channelHandle = GetFirChannelHandle();
	if (channelHandle == INVALID_CHANNEL_HANDLE) // party creation
	{
		TRACEINTO<<"allocating fir people timer due to invalid channel handle";
		pFirPeopleSegment = new CSegment(*pParam);
		StartTimer(FIR_PEOPLE_TIMER, ( 1 * SECOND),pFirPeopleSegment );
		return;
	}

	if (m_pTargetMode->GetConfMediaType()==eMixAvcSvcVsw && !m_pSipCntl->GetIsMrcCall())
	{
		if (!IsValidTimer (FIR_PEOPLE_TIMER))
		{
			pFirPeopleSegment = new CSegment(*pParam);
			StartTimer(FIR_PEOPLE_TIMER, ( 1 * WAIT_300_MS),pFirPeopleSegment ); // temp to resolve bug in VSW conference between MRMP and conference
		}
	// the bug: ihntra request is sent too soon ,before video sourcesw request , and mrmp is stuck in wait for intra
		return;
	}

	DoSendRelayAskEndpointForIntra(pParam, channelHandle);
}
//////////////////////////////////////////////////////////////////////////
void CSipParty::SaveIncomingIntraRequest(CSegment* pParam)
{
	CSegment* newSeg = new CSegment;
	if(newSeg)
	{
		newSeg->CopySegmentFromReadPosition(*pParam);
		if(m_bIsIncomingFIR == FALSE)
		{
			m_incomingIntraParam.DeSerialize(newSeg);
			m_bIsIncomingFIR = TRUE ;
		}
		else
		{ // case of more than 2 FIRs
			RelayIntraParam tempIntraParam;
			tempIntraParam.DeSerialize(newSeg);
			m_incomingIntraParam.m_listSsrc.merge(tempIntraParam.m_listSsrc); // merge 2 lists
			m_incomingIntraParam.m_listSsrc.unique(); // remove duplicates
		}
		POBJDELETE(newSeg);
	}
}

/////////////////////////////////////////////////////////////////////////////
DWORD CSipParty::GetFirChannelHandle()
{
	CSipCall *pCall = m_pSipCntl->GetCallObj();
	CSipChannel *pChannel;
	if (!pCall) {
		PTRACE(eLevelInfoNormal,"CSipParty::GetFirChannelHandle pCall = NULL");
		return INVALID_CHANNEL_HANDLE;
	}

	if(m_pCurrentMode->GetConfMediaType()==eMixAvcSvcVsw && (!m_pSipCntl->GetIsMrcCall()))
	{
		PTRACE(eLevelInfoNormal,"avc_vsw_relay CSipParty::GetFirChannelHandle GetchannelEx performed");
		pChannel = pCall->GetChannel(false, cmCapVideo, cmCapTransmit, kRolePeople);
	}
	else
	{
//		TRACEINTOFUNC<<CSipParty::GetFirChannelHandle GetChannel performed";
		pChannel = pCall->GetChannel(true, cmCapVideo, cmCapTransmit, kRolePeople);
	}
//	CSipChannel *pChannel = pCall->GetChannel(cmCapVideo, cmCapTransmit, kRolePeople);
	if (!pChannel) {
		PTRACE(eLevelInfoNormal,"CSipParty::GetFirChannelHandle pChannel = NULL");
		return INVALID_CHANNEL_HANDLE;
	}

	return pChannel->GetChannelHandle();
}
/////////////////////////////////////////////////////////////////////////////
BOOL CSipParty::IsSsrcEqual(RelayIntraParam& currentIntraParam)
{
	return (std::equal(currentIntraParam.m_listSsrc.begin(),
			currentIntraParam.m_listSsrc.end(),
			m_incomingIntraParam.m_listSsrc.begin())) ? TRUE : FALSE;

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::CheckIfIncomingNewFIRisPending(RelayIntraParam& currentIntraParam)
{
	if(m_bIsIncomingFIR && !(IsSsrcEqual(currentIntraParam)))
	{//if new Intra request arrived, merge both ssrcs requested to one FIR
		TRACEINTOFUNC << "New Intra Request is pending for different ssrc. merging the requests" ;

		currentIntraParam.m_listSsrc.merge(m_incomingIntraParam.m_listSsrc); // merge 2 lists
		currentIntraParam.m_listSsrc.unique(); // remove duplicates

		m_bIsIncomingFIR = FALSE ;
	}

}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::DoSendRelayAskEndpointForIntra(CSegment* pParam, DWORD channelHandle)
{
	CSegment* newSeg = new CSegment;

	newSeg->CopySegmentFromReadPosition(*pParam);//Olga: need to copy because pParam's read offset is not 0

	RelayIntraParam intraParam;
    intraParam.DeSerialize(newSeg);
    CheckIfIncomingNewFIRisPending(intraParam);

    std::string  strParams =  CStlUtils::ContainerToString(intraParam.m_listSsrc);
    TRACEINTOFUNC << "DEBUG_INTRA: partyRsrcId=" << intraParam.m_partyRsrcId << ", size=" << intraParam.m_listSsrc.size() << ", list=" << strParams.c_str();
	m_pSipCntl->FillAndSendMrmpRtcpFirStruct(&intraParam, channelHandle);

	POBJDELETE(newSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnFirPeopleTimer(CSegment* pParam)
{

	TRACEINTO<<"inside OnFirPeopleTimer";
	DWORD channelHandle = GetFirChannelHandle();
	CSegment* pFirPeopleSegment;
	if (channelHandle==INVALID_CHANNEL_HANDLE) {
		if (m_firPeopleCounter < 10) {
			TRACEINTO<<"reallocate fir people timer due to invalid channel handle"<<m_firPeopleCounter;
//			if (!IsValidTimer (FIR_PEOPLE_TIMER))
//			{
			m_firPeopleCounter++;
				pFirPeopleSegment = new CSegment(*pParam);
				StartTimer(FIR_PEOPLE_TIMER, ( 1 * SECOND),pFirPeopleSegment);
//			}
		}
		else {
			TRACEINTO<<"failure to send intra request for 10 times";
			m_firPeopleCounter = 0;
//			POBJDELETE(m_pFirPeopleSegnent);
		}

		return;
	}

	DoSendRelayAskEndpointForIntra(pParam, channelHandle);

	m_firPeopleCounter = 0;
//	POBJDELETE(m_pFirPeopleSegnent);
}

/////////////////////////////////////////////////////////////////////////////
// fir_filter
void CSipParty::OnFirTreatmentMapTimer(CSegment* pParam)
{
	TRACEINTO;

	CStructTm curTime, lastTreatedTime;
	STATUS timeStatus = SystemGetTime(curTime);

	FirTreatmentMap::iterator mapItr;

	FirTreatmentMap::iterator nextItr = m_firTreatmentMap.begin();
	while (nextItr != m_firTreatmentMap.end() )
	{
		mapItr = nextItr;
		++nextItr;

		lastTreatedTime = (*mapItr).second;

		// if more than 1 sec passed since last time treated
		if ( 1 <= curTime - lastTreatedTime )
		{
			m_firTreatmentMap.erase(mapItr);
		}
	}

	if ( !m_firTreatmentMap.empty() )
	{
		StartTimer(FIR_TREATMENT_MAP_TIMER, (30*SECOND), NULL);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateCurrentRcvModeAccordingToTarget()
{
	if (m_pCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive))
	{
		//We want to update every thing except bit rate. in cop we will update the bitrate too.
		DWORD bitRate = m_pCurrentMode->GetMediaBitRate(cmCapVideo,cmCapReceive);

    	const CMediaModeH323& RxTargetMode = m_pTargetMode->GetMediaMode(cmCapVideo, cmCapReceive);
		m_pCurrentMode->SetMediaMode(RxTargetMode, cmCapVideo, cmCapReceive);
		//m_pCurrentMode->SetMediaMode(m_pTargetMode, cmCapAudio, cmCapReceive);


        if (m_pCurrentMode->GetConfType() != kCop)
        {
            m_pCurrentMode->SetVideoBitRate(bitRate);
        }

		PTRACE2INT(eLevelInfoNormal,"CSipParty::UpdateCurrentRcvModeAccordingToTarget - : bitRate ",bitRate);
	}
	else
		PTRACE2(eLevelInfoNormal,"CSipParty::UpdateCurrentRcvModeAccordingToTarget - receive current mode is off, so it won't be updated : Name - ",PARTYNAME);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnBfcpStartReestablishConnection(CSegment* pParam)
{
	//BRIDGE-13250 (mrc check removed)

	enMediaOnHold eMediaOnHold = GetNonTipPartyOnHold();
	if ( eMediaOnHold )
	{
		PTRACE2INT(eLevelInfoNormal,"CSipParty::OnBfcpStartReestablishConnection - MediaOnHeld: ",(WORD)eMediaOnHold);
		return;
	}
	m_numOfBfcpReestablish++;

	// after 3 retries to reestablish bfcp connection we close bfcp and content
	if (m_numOfBfcpReestablish == NumOfBfcpReestablishment)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::OnBfcpStartReestablishConnection - close BFCP and content");

		m_bBfcpConnected 			= false;
		m_maskRequiredChangeMode 	= eChangeModeMask_None;

		m_pSipCntl->RemoveBfcpAndContentCaps();

		if (!IsActiveTransaction())
		{
			// if not in a middle of transaction, actions are done on m_pTargetMode
			m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
			m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);

			StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
		}
		else
		{
			// if in a middle of transaction, actions are done on m_pUpdateTargetMode (not allowed to change m_pTargetMode)
			m_pUpdateTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
			m_pUpdateTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);

			m_pSipTransaction->SetNeedReInviteForBfcp(TRUE);
		}
	}
	else
	{
		if (!IsActiveTransaction())
		{
			PTRACE(eLevelInfoNormal, "CSipParty::OnBfcpStartReestablishConnection - start");
			m_pSipCntl->SipBfcpReconnect();
		}
		else
		{
			// in the middle of transaction we can't close ports, so we ignore the request to reestablish bfcp connection in this time
			m_numOfBfcpReestablish--;

			PTRACE(eLevelInfoNormal, "CSipParty::OnBfcpStartReestablishConnection - can't close BFCP. In the middle of transaction");
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnBfcpEndReestablishConnection(CSegment* pParam)
{
	if (!IsActiveTransaction())
	{
		//==========================================================================================================================================
		// Allowing disconnection based on media in a PENDING transaction might result in an illegitimate disconnection if the CURRENT transaction
		// holds/mutes media for instance, therefore - preferring to risk NOT disconnecting a call then to risk wrongly disconnecting a call.
		//==========================================================================================================================================
		m_pSipCntl -> AllowMediaBasedDisconnectionInReinvite();
		StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
	}
	else
		m_pSipTransaction->SetNeedReInviteForBfcp(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::CloseTipSessionAndSendMuxDisconnect()
{
	WORD numOfSlaves	= 0;

	if (m_TipNumOfStreams)
	{
		SendMessageFromMasterToSlave(eTipSlaveAux, PARTYDISCONNECT, NULL);

		numOfSlaves = m_TipNumOfStreams -1 + m_bIsAudioAux;

		if (numOfSlaves > 1)
		{
			SendMessageFromMasterToSlave(eTipSlaveLeft, PARTYDISCONNECT, NULL);
			SendMessageFromMasterToSlave(eTipSlaveRigth, PARTYDISCONNECT, NULL);
		}

		m_TipNumOfStreams 	= 0;
		m_tipPartyType 		= eTipNone;
		m_bIsAudioAux		= FALSE;
		m_bIsVideoAux		= FALSE;

		//we want to start fall back to regular SIP EP using reinvite
		m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_CLOSE_TIP_CALL_REQ);
	}

	m_pSipCntl->CloseTipSessionIfNeeded();
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::IceMakeAnswerIndWhileDisconnecting()
{
	PTRACE(eLevelInfoNormal,"CSipParty::IceMakeAnswerIndWhileDisconnecting : Doing Nothing ");
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateVideoRateForFECorRED(CSegment* pParam)
{
	DWORD mediaType = cmCapEmpty;
	DWORD ssrc = 0;
	DWORD newFecRedPercent = 0;

	*pParam >> mediaType >> ssrc >> newFecRedPercent;

	TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - Name:" << PARTYNAME << ", mediaType:"
			  << mediaType << ", ssrc:" << ssrc << ", newFecRedPercent:" << newFecRedPercent;

	//Explanation for (ssrc != 0):
	//At the first time, when !main! link gets IP_CM_RTCP_PACKET_LOSS_STATUS_IND message regarding other !slave! link, this function gets ssrc!=0 and move the details
	//to slaves controller. Slaves controller sends the details (mediaType, newFecRedPercent) to the right slave with ssrc=0.
	//So at the second time, sipcontrol of the !slave! link sends to this function ssrc=0 and !slave! link starts to handle the event (decrease rate and resolution).
	if (m_pSipCntl && m_pSipCntl->GetRemoteIdent()==Microsoft_AV_MCU2013 && m_pSipCntl->isMs2013Active() && mediaType==cmCapVideo && (ssrc != 0) )
	{
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - forward FEC or RED message to slaves controller";
		CSegment* pCopyParam = new CSegment();
		*pParam << mediaType << ssrc << newFecRedPercent;
		pCopyParam->CopySegmentFromReadPosition(*pParam);
		m_pConfApi->SendMsgToSlavesController(GetPartyId(),HANDLE_SINGEL_FEC_OR_RED_WITH_AV_MCU2013, pCopyParam);
	}
	else if ( mediaType == cmCapAudio)
	{
		//not matter if it is rtv/mssvc/audio only (because we want to update the art regarding this RED
		UpdateVideoRateForRED();
	}
	else  //mediaType == cmCapVideo
	{
		//MS RTV FEC:
		if ( m_pCurrentMode && m_pCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople) == eRtvCapCode )
			UpdateVideoRateForMSrtvFEC();

		//eMsSvcCapCode:
		else if ( m_pCurrentMode && m_pCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople) == eMsSvcCapCode )
			UpdateVideoRateForMSsvcFEC(newFecRedPercent);
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipParty::UpdateVideoRateForRED()
{
	PASSERTMSG_AND_RETURN(!m_pSipCntl, "LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForRED - m_pSipCntl is NULL");
	PASSERTMSG_AND_RETURN(!m_pConfApi, "LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForRED - m_pConfApi is NULL");
	PASSERTMSG_AND_RETURN(!m_pTargetMode, "LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForRED - m_pTargetMode is NULL");

	DWORD unREDBitRate = 0;  //newPeopleRate

	//we are taking from the target instead of current because we update the target (here)
	//before the current (partycontrol) and in case FEC and RED come together current is not updated. same for vsr_ind cases
	DWORD targetVideoRate = 0;
	targetVideoRate = m_pTargetMode->GetVideoBitRate(cmCapTransmit, kRolePeople);

	BYTE isVidoOutMuted = FALSE;
	CSipCall *pCall = m_pSipCntl->GetCallObj();
	CSipChannel* videoOutCh =  pCall->GetChannel(VIDEO_OUT);
	if(videoOutCh && videoOutCh->IsMuted())
		isVidoOutMuted = TRUE;

	if(m_pSipCntl->GetIsRedOn()) //Start RED - RED is only for Transmit.
	{

		unREDBitRate = CalcVideoRateForRED(targetVideoRate);
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - Start RED! old targetVideoRate:" << targetVideoRate
				  << ", will be updated with new unREDBitRate:" << unREDBitRate;
		SetMoreDetailsForMsSvcIfNeeded(unREDBitRate);

		m_pConfApi->UpdatePartyControlRegardingChangeOfTxVideoCausedByFecOrRed(m_pTargetMode, GetPartyId(), unREDBitRate, (DWORD)cmCapAudio, (BYTE)isVidoOutMuted ); //1=cmCapAudio
	}
	else //Stop RED
	{
		unREDBitRate = targetVideoRate + (m_pSipCntl->GetTheChangeOfRateForRED());

		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - Stop RED! old targetVideoRate:" << targetVideoRate << ", will be updated with new unREDBitRate:" << unREDBitRate;

		SetMoreDetailsForMsSvcIfNeeded(unREDBitRate);

		m_pConfApi->UpdatePartyControlRegardingChangeOfTxVideoCausedByFecOrRed(m_pTargetMode, GetPartyId(), unREDBitRate, (DWORD)cmCapAudio, (BYTE)isVidoOutMuted);  //1=cmCapAudio
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
DWORD CSipParty::CalcVideoRateForRED(DWORD videoRateBeforeChange)
{
	PASSERTMSG_AND_RETURN_VALUE(!m_pSipCntl, "LYNC2013_FEC_RED: CSipParty::CalcVideoRateForRED - m_pSipCntl is NULL", videoRateBeforeChange*10);
	PASSERTMSG_AND_RETURN_VALUE(!m_pCurrentMode , "LYNC2013_FEC_RED: CSipParty::CalcVideoRateForRED - m_pCurrentMode is NULL", videoRateBeforeChange*10);

	DWORD   unREDBitRate     = 0;  //newPeopleRate
	//EFormat eFormat          = kUnknownFormat;
	DWORD   CurrentAudioRate = 0;
	DWORD   ChangeOfRate     = 0;

	CurrentAudioRate = m_pCurrentMode->GetMediaBitRate(cmCapAudio,cmCapTransmit, kRolePeople);
	CurrentAudioRate = CurrentAudioRate*10;

	unREDBitRate = videoRateBeforeChange - CurrentAudioRate;

	TRACEINTO << "LYNC2013_FEC_RED:DEBUG: PartyID:" << GetPartyId() << " - CurrentAudioRate(=ChangeOfRate):" << CurrentAudioRate
			  << ", videoRateBeforeChange:" << videoRateBeforeChange << ", unREDBitRate:" << unREDBitRate;

	DWORD minRate = 640;

	if(unREDBitRate < minRate)
	{
		PASSERTMSG(true,"LYNC2013_FEC_RED: CSipParty::CalcVideoRateForRED - (unREDBitRate < 640 ");
		unREDBitRate = minRate;
	}

	ChangeOfRate = CurrentAudioRate;

	m_pSipCntl->SetTheChangeOfRateForRED(ChangeOfRate);

	return unREDBitRate;
}
///////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipParty::UpdateVideoRateForMSsvcFEC(DWORD newFecPercent)
{
	PASSERTMSG_AND_RETURN(!m_pSipCntl, "LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForMSsvcFEC - m_pSipCntl is NULL");
	PASSERTMSG_AND_RETURN(!m_pConfApi, "LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForMSsvcFEC - m_pConfApi is NULL");
	PASSERTMSG_AND_RETURN(!m_pTargetMode, "LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForMSsvcFEC - m_pTargetMode is NULL");

	DWORD unFECBitRate = 0;  //newPeopleRate

	//we are taking from the target instead of current because we update the target (here)
	//before the current (partyControl) and in case FEC and RED come together current is not updated. same for vsr_ind cases
	DWORD targetVideoRate = 0;
	targetVideoRate = m_pTargetMode->GetVideoBitRate(cmCapTransmit, kRolePeople);

	TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - Name:" << PARTYNAME << ", targetVideoRate1:" << targetVideoRate;

	BYTE isVidoOutMuted = FALSE;
	CSipCall *pCall = m_pSipCntl->GetCallObj();
	CSipChannel* videoOutCh =  pCall->GetChannel(VIDEO_OUT);
	if(videoOutCh && videoOutCh->IsMuted())
		isVidoOutMuted = TRUE;

	if(m_pSipCntl->GetIsFecOn()) //Start FEC - FEC is only for Transmit.
	{

		EFormat eFormat = m_pCurrentMode->GetVideoFormat(cmCapTransmit,kRolePeople);
		DWORD   CurrentFrameRate = m_pCurrentMode->GetFrameRate(eFormat,cmCapTransmit,kRolePeople);

		targetVideoRate = targetVideoRate/10;
		unFECBitRate = CalcVideoRateForMSsvcFEC(targetVideoRate,newFecPercent,CurrentFrameRate);
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - Start FEC! old targetVideoRate2:" << targetVideoRate << ", will be updated with new unFECBitRate:" << unFECBitRate;

		SetMoreDetailsForMsSvcIfNeeded(unFECBitRate);

		m_pConfApi->UpdatePartyControlRegardingChangeOfTxVideoCausedByFecOrRed(m_pTargetMode, GetPartyId(), unFECBitRate, (DWORD)cmCapVideo, (BYTE)isVidoOutMuted); //2=cmCapVideo
	}
	else //Stop FEC
	{
		unFECBitRate = targetVideoRate + (m_pSipCntl->GetTheChangeOfRateForFEC());

		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - Stop FEC! old targetVideoRate:" << targetVideoRate << ", will be updated with new unFECBitRate:" << unFECBitRate;

		SetMoreDetailsForMsSvcIfNeeded(unFECBitRate);

		m_pConfApi->UpdatePartyControlRegardingChangeOfTxVideoCausedByFecOrRed(m_pTargetMode, GetPartyId(), unFECBitRate, (DWORD)cmCapVideo, (BYTE)isVidoOutMuted );  //2=cmCapVideo
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
DWORD CSipParty::CalcVideoRateForMSsvcFEC(DWORD videoRateBeforeChange, DWORD newFecPercent, DWORD CurrentFrameRate)
{
	PASSERTMSG_AND_RETURN_VALUE(!m_pSipCntl, "LYNC2013_FEC_RED: CSipParty::CalcVideoRateForMSsvcFEC - m_pSipCntl is NULL", videoRateBeforeChange*10);
	PASSERTMSG_AND_RETURN_VALUE(!m_pCurrentMode , "LYNC2013_FEC_RED: CSipParty::CalcVideoRateForMSsvcFEC - m_pCurrentMode is NULL", videoRateBeforeChange*10);

	DWORD 	unFECBitRate           = 0;  //newPeopleRate
	//DWORD   CurrentFrameRate 	   = 0;
	//EFormat eFormat          	   = kUnknownFormat;
	DWORD   unOrigT0BitRate  	   = 0;
	DWORD   unAvgT0FrmSize     	   = 0;
	DWORD   unAvgT0FrmVideoPktNum  = 0;
	DWORD   unAvgT0FrmFecPktNum    = 0;
	DWORD   ChangeOfRate           = 0;
	DWORD   unLossFactor           = 0;
	DWORD   unT0FrameRate          = 15;

	//eFormat = m_pCurrentMode->GetVideoFormat(cmCapTransmit,kRolePeople);
	//CurrentFrameRate = m_pCurrentMode->GetFrameRate(eFormat,cmCapTransmit,kRolePeople);

	if (newFecPercent >= 1 && newFecPercent <= 3)
		unLossFactor = 6;
	else if (newFecPercent >= 4 && newFecPercent <= 6)
		unLossFactor = 2;
	else if (newFecPercent >= 7)
		unLossFactor = 1;

	if (CurrentFrameRate == 30) // 2 Layers
	{
		unOrigT0BitRate = (50 * videoRateBeforeChange) / 100;   // For 2 layers, T0 will consume 50% of the total bitrate
	}
	else if (CurrentFrameRate == 15)  // 1 Layer
	{
		unOrigT0BitRate = videoRateBeforeChange;    // 1 Layer of only T0 frames. All of the bitrate is consumed by T0.
	}
	else // We shouldn't get here, default value will be set to 15
	{
		unOrigT0BitRate = videoRateBeforeChange;
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - ERROR - Frame rate is different from 15/30. ";
	}

    unAvgT0FrmSize = unOrigT0BitRate / (unT0FrameRate * 8);   // For 2 layers, 30 fps, we will have 15 T0 frames. For 1 layer, 15 fps, all of them are T0. We divide by 8 in order to get Bytes value

	unAvgT0FrmVideoPktNum = (unAvgT0FrmSize+999)/1000;        // Average size of packet is ~1000 bytes
	if (unAvgT0FrmVideoPktNum == 0)
	{
		unAvgT0FrmVideoPktNum = 1;
		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - ERROR - unAvgT0FrmVideoPktNum is changed from 0 to 1!!";
	}

	if (unLossFactor>0)
		unAvgT0FrmFecPktNum = min(unAvgT0FrmVideoPktNum/unLossFactor,15);
	else
		unAvgT0FrmFecPktNum = min(unAvgT0FrmVideoPktNum,15);

	if (unAvgT0FrmFecPktNum == 0)      // Making sure that we always give some amount of protection.
		unAvgT0FrmFecPktNum = 1;

	ChangeOfRate = (unOrigT0BitRate * unAvgT0FrmFecPktNum) / (unAvgT0FrmVideoPktNum + unAvgT0FrmFecPktNum);

	unFECBitRate = videoRateBeforeChange - ChangeOfRate;
	unFECBitRate = unFECBitRate * 10;

	TRACEINTO << "LYNC2013_FEC_RED:DEBUG PartyID:" << GetPartyId() << " - videoRateBeforeChange:" << videoRateBeforeChange << ", CurrentFrameRate:"
			  << CurrentFrameRate << ", unOrigT0BitRate:" << unOrigT0BitRate << ", unAvgT0FrmSize:" << unAvgT0FrmSize
			  << ", newFecPercent:" << newFecPercent << ", unLossFactor:" << unLossFactor << ", unAvgT0FrmVideoPktNum:"
			  << unAvgT0FrmVideoPktNum << ", unAvgT0FrmFecPktNum:" << unAvgT0FrmFecPktNum << ", unFECBitRate:"
			  << ChangeOfRate*10 << ", CurrentVideoRate-unFECBitRate:" << unFECBitRate;

	DWORD minRate = 640;

	if(unFECBitRate < minRate)
	{
		PASSERTMSG(true,"LYNC2013_FEC_RED: CSipParty::CalcVideoRateForMSsvcFEC - (unFECBitRate < 640 ");
		unFECBitRate = minRate;
	}

	ChangeOfRate = ChangeOfRate*10;

	m_pSipCntl->SetTheChangeOfRateForFEC(ChangeOfRate);

	return unFECBitRate;

}
///////////////////////////////////////////////////////////////////////////////////////////////
void CSipParty::UpdateVideoRateForMSrtvFEC()
{
	// Entry conditions
	//==================
	PASSERTMSG_AND_RETURN(!m_pSipCntl, "CSipParty::UpdateVideoRateForMSrtvFEC - m_pSipCntl is NULL");
	PASSERTMSG_AND_RETURN(!m_pCurrentMode, "CSipParty::UpdateVideoRateForMSrtvFEC - m_pCurrentMode is NULL");
	PASSERTMSG_AND_RETURN(!m_pTargetMode, "CSipParty::UpdateVideoRateForMSrtvFEC - m_pTargetMode is NULL");
	PASSERTMSG_AND_RETURN(!m_pConfApi, "CSipParty::UpdateVideoRateForMSrtvFEC - m_pConfApi is NULL");

	DWORD newPeopleRate = 0;
	DWORD CurrentVideoRate = 0;
	DWORD CurrentFrameRate = 0;
	DWORD FEC_SIZE = 1000;
	EFormat eFormat = kUnknownFormat;
	//CurrentVideoRate = m_pCurrentMode->GetVideoBitRate(cmCapTransmit, kRolePeople);
	CurrentVideoRate = m_pTargetMode->GetVideoBitRate(cmCapTransmit, kRolePeople);

	TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - CurrentVideoRate1:" << CurrentVideoRate;

	if(m_pSipCntl->GetIsFecOn()) //Start FEC - FEC is only for Transmit.
	{
		DWORD ChangeOfRate = 0;

		//eFormat = m_pCurrentMode->GetVideoFormat(cmCapTransmit,kRolePeople);

		CurrentVideoRate = CurrentVideoRate*100; //moving to bitFerSec

		//CurrentFrameRate = m_pCurrentMode->GetFrameRateForRTV(cmCapTransmit,kRolePeople);
		CurrentFrameRate = m_pTargetMode->GetFrameRateForRTV(cmCapTransmit,kRolePeople);

		if (CurrentFrameRate == 0)
		{
			m_pTargetMode->Dump("LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForMSrtvFEC: ERROR m_pTargetMode with TX FR=0, will be set to 30",eLevelInfoNormal);
			CurrentFrameRate = 30;
		}

		DWORD AverageFrameSize = CurrentVideoRate/CurrentFrameRate/ 8;//  (bytes)

		DWORD AverageVideoPktNumPerFrame = AverageFrameSize/1000;
		AverageVideoPktNumPerFrame += (AverageFrameSize%1000) ? 1 : 0;    //Round Up

		DWORD AverageFecPktNumPerFrame = min(4, AverageVideoPktNumPerFrame);      //The maximum FEC packet per frame we supported is 4.

		if (AverageFecPktNumPerFrame==0)
		{
			TRACEINTO << "LYNC2013_FEC_RED: ERROR AverageFecPktNumPerFrame==0, will be set to 1";
			AverageFecPktNumPerFrame = 1;
		}

		newPeopleRate = CurrentVideoRate * AverageVideoPktNumPerFrame/(AverageVideoPktNumPerFrame + AverageFecPktNumPerFrame );

		//PTRACE2INT(eLevelInfoNormal,"LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForMSrtvFEC : newPeopleRate :",newPeopleRate);

		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - Start FEC: CurrentVideoRate2:" << CurrentVideoRate  << ", CurrentFrameRate:" << CurrentFrameRate << ", AverageVideoPktNumPerFrame:" << AverageVideoPktNumPerFrame << ", AverageFecPktNumPerFrame:" << AverageFecPktNumPerFrame <<  ", newPeopleRate:" << newPeopleRate;
/*
		DWORD OverheadFECRate = CurrentFrameRate * FEC_SIZE * 8;
		PTRACE2INT(eLevelInfoNormal,"CSipParty::UpdateVideoRateForMSrtvFEC : OverheadFECRate :",OverheadFECRate);

		newPeopleRate = CurrentVideoRate - OverheadFECRate;
		PTRACE2INT(eLevelInfoNormal,"CSipParty::UpdateVideoRateForMSrtvFEC : newPeopleRate :",newPeopleRate);
*/
		newPeopleRate = newPeopleRate/100;

		DWORD minRate = 640;

		if(newPeopleRate < minRate)
		{
			PASSERTMSG(true,"LYNC2013_FEC_RED: CSipParty::UpdateVideoRateForMSrtvFEC - (newPeopleRate < 640 ");
			newPeopleRate = minRate;
		}

		ChangeOfRate = (CurrentVideoRate/100) - newPeopleRate;

		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - ChangeOfRate:" << ChangeOfRate;
		//m_pSipCntl->SetLastRateBeforeFEC(CurrentVideoRate/100); //Old Max rate with no FEC
		//m_pSipCntl->SetBitRateWithFEC(newPeopleRate); //low rate with FEC
		m_pSipCntl->SetTheChangeOfRateForFEC(ChangeOfRate);

		m_pTargetMode->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);

		m_pConfApi->UpdatePartyControlRegardingChangeOfTxVideoCausedByFecOrRed(NULL, GetPartyId(), newPeopleRate, (DWORD)cmCapVideo, (BYTE)FALSE); //2=cmCapVideo

	}
	else //Stop FEC
	{
		//newPeopleRate = m_pSipCntl->GetLastRateBeforeFEC();
		newPeopleRate = CurrentVideoRate + (m_pSipCntl->GetTheChangeOfRateForFEC());

		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - Stop FEC, newPeopleRate:" << newPeopleRate;

		//m_pSipCntl->SetBitRateWithFEC(0);
		//m_pSipCntl->SetTheChangeOfRateForFEC(0);

		m_pTargetMode->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);

		m_pConfApi->UpdatePartyControlRegardingChangeOfTxVideoCausedByFecOrRed(NULL, GetPartyId(), newPeopleRate, (DWORD)cmCapVideo, (BYTE)FALSE); //2=cmCapVideo
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
DWORD CSipParty::CalcVideoRateForMSrtvFEC(DWORD CurrentVideoRate, DWORD frameRate)
{
	DWORD newPeopleRate = 0;
	DWORD CurrentFrameRate = frameRate; //m_pTargetMode->GetFrameRateForRTV(cmCapTransmit,kRolePeople);
	DWORD FEC_SIZE = 1000;
	EFormat eFormat = kUnknownFormat;
	DWORD ChangeOfRate = 0;

	if (CurrentFrameRate == 0)
	{
		m_pTargetMode->Dump("LYNC2013_FEC_RED: CSipParty::CalcVideoRateForMSrtvFEC: ERROR m_pTargetMode with TX FR=0, will be set to 30",eLevelInfoNormal);
		CurrentFrameRate = 30;
	}

	DWORD AverageFrameSize = CurrentVideoRate/CurrentFrameRate/ 8;//  (bytes)

	DWORD AverageVideoPktNumPerFrame = AverageFrameSize/1000;
	AverageVideoPktNumPerFrame += (AverageFrameSize%1000) ? 1 : 0;    //Round Up

	DWORD AverageFecPktNumPerFrame = min(4, AverageVideoPktNumPerFrame);      //The maximum FEC packet per frame we supported is 4.

	if (AverageFecPktNumPerFrame==0)
	{
		TRACEINTO << "LYNC2013_FEC_RED: ERROR AverageFecPktNumPerFrame==0, will be set to 1";
		AverageFecPktNumPerFrame = 1;
    }


	newPeopleRate = CurrentVideoRate * AverageVideoPktNumPerFrame/(AverageVideoPktNumPerFrame + AverageFecPktNumPerFrame );

	TRACEINTO << "LYNC2013_FEC_RED:DEBUG: PartyID:" << GetPartyId() << " - Start FEC: CurrentVideoRate2:" << CurrentVideoRate
			  << ", CurrentFrameRate:" << CurrentFrameRate << ", AverageVideoPktNumPerFrame:" << AverageVideoPktNumPerFrame
			  << ", AverageFrameSize:" << AverageFrameSize << ", AverageFecPktNumPerFrame:" << AverageFecPktNumPerFrame
			  <<  ", newPeopleRate:" << newPeopleRate;

	newPeopleRate = newPeopleRate/100;

	DWORD minRate = 640;

	if(newPeopleRate < minRate)
	{
		PASSERTMSG(true,"LYNC2013_FEC_RED: CSipParty::CalcVideoRateForMSrtvFEC - (newPeopleRate < 640 ");
		newPeopleRate = minRate;
	}

	ChangeOfRate = (CurrentVideoRate/100) - newPeopleRate;

	TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - ChangeOfRate:" << ChangeOfRate << ", newPeopleRate:" << newPeopleRate;

	m_pSipCntl->SetTheChangeOfRateForFEC(ChangeOfRate);

	return newPeopleRate;

}
///////////////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipParty::SetMoreDetailsForMsSvcIfNeeded(DWORD newPeopleRate)
{
	//PTRACE(eLevelInfoNormal,"LYNC2013_FEC_RED: CSipParty::SetMoreDetailsForMsSvcIfNeeded");

	const CSipCaps*	pRemoteCaps = NULL;
	pRemoteCaps = m_pSipCntl->GetLastRemoteCaps();

	if (pRemoteCaps)
	{
		MsSvcVideoModeDetails MsSvcDetailsOfRemote;
		pRemoteCaps->GetMsSvcVidMode(MsSvcDetailsOfRemote);

		CMsSvcVideoMode* MsSvcVidMode = new CMsSvcVideoMode();
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
		CConfParty* pConfParty = NULL;
		Eh264VideoModeType maxH264Mode;
		MsSvcVideoModeDetails newMsSvcDetails;

		if (pCommConf)
			pConfParty = pCommConf->GetCurrentParty(GetMonitorPartyId());

		if (pConfParty)
			maxH264Mode = GetMaxH264VideoModeForMsSvcAccordingToSettings(pConfParty, pCommConf);
		else
			maxH264Mode = eLasth264VideoMode;

		if (MsSvcDetailsOfRemote.aspectRatio == E_VIDEO_RES_ASPECT_RATIO_16_9)
			MsSvcVidMode->GetMsSvcVideoParamsByRate(newMsSvcDetails,(newPeopleRate*100),maxH264Mode,E_VIDEO_RES_ASPECT_RATIO_DUMMY);
		else
			MsSvcVidMode->GetMsSvcVideoParamsByRate(newMsSvcDetails,(newPeopleRate*100),maxH264Mode,E_VIDEO_RES_ASPECT_RATIO_4_3);

		if ( newMsSvcDetails.maxWidth<MsSvcDetailsOfRemote.maxWidth && newMsSvcDetails.maxHeight<MsSvcDetailsOfRemote.maxHeight )
		{
			newMsSvcDetails.maxFrameRate = min(newMsSvcDetails.maxFrameRate , MsSvcDetailsOfRemote.maxFrameRate);
			m_pTargetMode->SetMsSvcScm(newMsSvcDetails,cmCapTransmit,newPeopleRate);
		}
		else
			m_pTargetMode->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit,(ERoleLabel)kRolePeople);

		TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId() << " - UPDATE_TARGETMODE videoTx rate is updated to:"
				  << m_pTargetMode->GetVideoBitRate(cmCapTransmit,kRolePeople);
	}

}
///////////////////////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyUpdateMuteIcon(CSegment* pParam)
{
	WORD onOff;

	PASSERTMSG_AND_RETURN(!pParam, "!pParam");

	*pParam >> onOff;

	TRACEINTO << " unIsMuteOn " << (int)(onOff);

	UpdateMuteIconState((EOnOff)onOff);
}
void CSipParty::OnAckForScpStreamsReq(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"_scp_flow_ CSipParty::OnAckForScpStreamsReq ");

	unsigned int sequenceNumber;
	*pParam>>sequenceNumber;

	m_pSipCntl->SendAckForScpReq(sequenceNumber);
}



/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnScpNotificationToEpReq(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"_scp_flow_ CSipParty::OnScpNotificationToEpReq");

	m_pSipCntl->SendScpNotificationReq(pParam);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnScpIvrStateNotificationReqToEp(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"_scp_flow_ CSipParty::OnScpIvrStateNotificationReqToEp");

	m_pSipCntl->SendScpIvrStateNotificationReq(pParam);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSipPartyNotificationAckFromEP(CSegment* pParam)
{
	APIU32 channelHandle = 0;
	APIU32 remoteSequenseNumber = 0;
	APIUBOOL bIsAck = FALSE;

	*pParam >> channelHandle >> remoteSequenseNumber >> bIsAck;

	TRACEINTO << "channelHandle:" << channelHandle << ", remoteSeqNum:" << remoteSequenseNumber << ", IsAck:" << bIsAck;

	m_pConfApi->ScpNotificationAckFromEP(GetPartyId(), channelHandle, remoteSequenseNumber, bIsAck);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnAckForScpNotificationInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnAckForScpNotificationInd ");

	APIU32 sequenceNum;
	*pParam >> sequenceNum;

	CSegment* pSeg = new CSegment;
	if (pSeg == NULL)
		abort();

	*pSeg << sequenceNum;

	m_pSipCntl->SendAckFroScpNotificationInd(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
#define PATCH_FOR_MRMP_WAIT_FOR_INTRA 10000
void CSipParty::OnMrmpRtcpFirInd(CSegment* pParam)
{
	MrmpRtcpFirStruct* pStruct = (MrmpRtcpFirStruct*)pParam->GetPtr(1);

	RelayIntraParam intraParam;

	intraParam.m_partyRsrcId = GetPartyId();

	CCommConf* pComConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

	//TRACEINTO << "productType:" << CProcessBase::GetProcess()->GetProductType() << ", confMediaType:" << pComConf->GetConfMediaType() << ", pStruct->unSequenseNumber:" << pStruct->unSequenseNumber;

	if (pComConf && pComConf->GetConfMediaType() == eMixAvcSvcVsw)
	{
		intraParam.m_bIsGdr = false;
	}
	else
	{
		// ((intraParam.m_bIsGdr==TRUE ? "YES":"NO"));
		intraParam.m_bIsGdr = ((TRUE == pStruct->bIsGdr) ? true : false);
		//TRACEINTO << "Not supposed to be here in case of IBM vsw";
	}

	if (pComConf && pComConf->GetConfMediaType() == eMixAvcSvcVsw && (!m_pSipCntl->GetIsMrcCall()) && pStruct->unSequenseNumber == PATCH_FOR_MRMP_WAIT_FOR_INTRA)
	{
		TRACEINTO << "Special case of wait for intra starting intra request";
		m_pSipCntl->FillAndSendMrmpRtcpFirStruct(NULL, 0);
	}
	else
	{
		unsigned int curSsrc = 0;

		for (int i = 0; i < pStruct->nNumberOfSyncSources; i++)
		{
			curSsrc = pStruct->syncSources[i];
			//TRACEINTO << "CSipParty::OnMrmpRtcpFirInd syncSources: " << pStruct->syncSources[i];

			// fir_filter
			if (true == FirShouldBeHandledForSpecSsrc(curSsrc))
				intraParam.m_listSsrc.push_back(curSsrc);
		}

		if (pComConf && pComConf->GetConfMediaType() == eMixAvcSvc && !m_pSipCntl->GetIsMrcCall() && (pStruct->unSequenseNumber == PATCH_FOR_MRMP_WAIT_FOR_INTRA))
			intraParam.m_bIsSsrc = true;
		else
			intraParam.m_bIsSsrc = false;

		TRACEINTO << "PartyId:" << intraParam.m_partyRsrcId << ", IsGdr:" << ((intraParam.m_bIsGdr == TRUE ? "YES" : "NO")) << " - Sending request to bridge";

		if (0 < intraParam.m_listSsrc.size())
		{
			m_pConfApi->HandleMrmpRtcpFirInd(GetPartyId(), &intraParam);

			if (!IsValidTimer(FIR_TREATMENT_MAP_TIMER))
			{
				StartTimer(FIR_TREATMENT_MAP_TIMER, (30 * SECOND), NULL);
				TRACEINTO << "Starting FIR_TREATMENT_MAP_TIMER";
			}
		}
		else
		{
			TRACEINTO << m_partyConfName << " - FIR should not be treated for any Ssrc";
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// fir_filter
bool CSipParty::FirShouldBeHandledForSpecSsrc(unsigned int specSsrc)
{
	bool retVal = false;

	CStructTm curTime;
		STATUS timeStatus = SystemGetTime(curTime);

	FirTreatmentMap::iterator mapItr = m_firTreatmentMap.find(specSsrc);
	if ( mapItr == m_firTreatmentMap.end() ) // specSsrc does not exist in m_firTreatmentMap yet
	{
		// add specSsrc to map
		m_firTreatmentMap.insert( pair<unsigned int,CStructTm>(specSsrc, curTime) );
		retVal = true;
	}

	else // specSsrc exists in m_firTreatmentMap already
	{
		// check if specSsrc was treated within last sec
		CStructTm specSsrcLastTreated = (*mapItr).second;
		if ( 1 <= curTime - specSsrcLastTreated )
		{
			// update LastTreated with curTime
			(*mapItr).second = curTime;
			retVal = true;
		}
		else
		{
			TRACEINTO << m_partyConfName << ", Ssrc:" << specSsrc << " - Was filtered (was received less than 1 sec since last treated)";
			retVal = false;
		}
	}

	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CSipParty::GetSSRcIdsForAvc(int ind, cmCapDirection direction, cmCapDataType aDataType)
{
    DWORD ssrc = 0;
    ssrc = m_SsrcIdsForAvcParty.GetSsrcId(ind, aDataType);

    CCommConf* pComConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

    if (pComConf && pComConf->GetConfMediaType()==eMixAvcSvcVsw)
    {
        if (direction == cmCapTransmit)
        {
            ssrc = MASK_FOR_VSW_AVC_PIPEID & ssrc;
        }
    }
    return ssrc;
}

void  CSipParty::SetConfMediaType(eConfMediaType aConfMediaType)
{
	if (m_pSipCntl)
		m_pSipCntl->SetConfMediaType(aConfMediaType);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnScpPipesMappingNotificationToEpReq(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"_scp_flow_ CSipParty::OnScpPipesMappingNotificationToEpReq");

	CScpPipeMappingNotification pipeMappingNotifyReq;
	pipeMappingNotifyReq.DeSerialize(NATIVE,*pParam);
	// pipeMappingNotifyReq.Dump();


	CSegment* pSeg = new CSegment;
	if (pSeg == NULL)
		abort();

	pipeMappingNotifyReq.Serialize(NATIVE,*pSeg);
	m_pSipCntl->SendScpPipesMappingNotificationReq(pSeg);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::HandleBfcpScmAndCaps()
{
	const CSipCaps* pLocalCaps	= m_pSipCntl->GetLocalCaps();

	CSuperLargeString strCaps1;
	pLocalCaps ->DumpToString(strCaps1);
	PTRACE2(eLevelInfoNormal,"CSipParty::HandleBfcpScmAndCaps - Dump regular caps pLocalCaps =",strCaps1.GetString());

	PTRACE2INT(eLevelInfoNormal,"CSipParty::HandleBfcpScmAndCaps - pLocalCaps->GetBfcpTransportType:", pLocalCaps->GetBfcpTransportType());

	if (pLocalCaps->IsMedia(cmCapBfcp))
	{
		CBaseCap* pCap = pLocalCaps->GetCapSet(cmCapBfcp, 0);

		if (pCap)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipParty::HandleBfcpScmAndCaps - set bfcp, transport type:", pLocalCaps->GetBfcpTransportType());
			POBJDELETE(pCap);
		}

		enTransportType 	transType 	= pLocalCaps->GetBfcpTransportType();
		eBfcpSetup 			bfcpSetup 	= pLocalCaps->GetBfcpSetupAttribute();
		eBfcpConnection 	connection 	= pLocalCaps->GetBfcpConnectionAttribute();
		eBfcpFloorCtrl 		floorCtrl   = pLocalCaps->GetBfcpFloorCtrlAttribute();
		eBfcpMStreamType 	mstreamType = pLocalCaps->GetBfcpMStreamType();

		WORD				confId		= pLocalCaps->GetBfcpConfId();
		WORD				userId		= pLocalCaps->GetBfcpUserId();

		m_pTargetMode->SetBfcp(transType);
		m_pTargetMode->SetBfcpParameters(bfcpSetup, connection, floorCtrl, mstreamType);

		m_pTargetMode->SetConfUserIdForBfcp(confId, userId);
		m_pTargetMode->SetFloorIdParamsForBfcp(BFCP_FLOOR_ID_PPC, STREAM_LABEL_CONTENT);

		//BRIDGE-5407 //*m_pUpdateTargetMode 			= *m_pTargetMode what is below is instead of this line!!;
		m_pUpdateTargetMode->SetBfcp(transType);
		m_pUpdateTargetMode->SetBfcpParameters(bfcpSetup, connection, floorCtrl, mstreamType);

		m_pUpdateTargetMode->SetConfUserIdForBfcp(confId, userId);
		m_pUpdateTargetMode->SetFloorIdParamsForBfcp(BFCP_FLOOR_ID_PPC, STREAM_LABEL_CONTENT);

		m_pTargetModeMaxAllocation->SetBfcp(transType);
		m_pTargetModeMaxAllocation->SetBfcpParameters(bfcpSetup, connection, floorCtrl, mstreamType);

		m_pTargetModeMaxAllocation->SetConfUserIdForBfcp(confId, userId);
		m_pTargetModeMaxAllocation->SetFloorIdParamsForBfcp(BFCP_FLOOR_ID_PPC, STREAM_LABEL_CONTENT);

	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::HandleContentScmAndCaps()
{
	PTRACE(eLevelInfoNormal,"CSipParty::HandleContentScmAndCaps");

	if (m_pTargetMode->IsMediaOff(cmCapVideo, cmCapReceiveAndTransmit))
	{
		PTRACE(eLevelInfoNormal,"CSipParty::HandleContentScmAndCaps: Seconday. Remove Bfcp and content");

		m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
	}
	else if (m_pTargetMode->IsMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
	{
		PTRACE(eLevelInfoNormal,"CSipParty::HandleContentScmAndCaps : Reinvite for dial out - add content");

		const CSipCaps* pLocalCaps = m_pSipCntl->GetLocalCaps();

		if (pLocalCaps)
		{
			// Set content on in target mode, in order to open content receive channel and send the reinvite with content in the first reinvite in Dial-Out.
			// Content bridge still not open - it will be open as a result of find best mode if needed.
			CapEnum contentProtocol = pLocalCaps->GetBestContentProtocol();
			//DWORD contentRate = pLocalCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation);

			BYTE hd1080Mpi = 0;
            BYTE isHD1080Supported = FALSE;
            BYTE hdMpi = 0;
            BYTE hd720Mpi = 0;
			BYTE isHighProfileContent = FALSE;

            hd1080Mpi = pLocalCaps->IsCapableOfHDContent1080();

            if(!hd1080Mpi)
            {
            	hd720Mpi = pLocalCaps->IsCapableOfHDContent720();
            	hdMpi = hd720Mpi;
            }
            else
            {
            	isHD1080Supported = TRUE;
            	hdMpi = hd1080Mpi;
            }

            isHighProfileContent = pLocalCaps->IsHighProfileContent();

            CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());

			if (!IsTIPContentEnable())
			{
				m_pTargetMode->SetContent(/*contentRate*/0, cmCapReceiveAndTransmit,contentProtocol,isHD1080Supported,hdMpi,isHighProfileContent);
				m_pTargetModeMaxAllocation->SetContent(/*contentRate*/0, cmCapReceiveAndTransmit,contentProtocol,isHD1080Supported,hdMpi,isHighProfileContent);
			}
			else if (pCommConf && pCommConf->GetIsPreferTIP())
			{
				m_pTargetMode->SetTIPContent(0,cmCapReceiveAndTransmit,FALSE);
				m_pTargetModeMaxAllocation->SetTIPContent(0,cmCapReceiveAndTransmit,FALSE);
			}
			else //eTipCompatibleVideoAndContent
			{
				m_pTargetMode->SetTIPContent(0,cmCapReceiveAndTransmit);
				m_pTargetModeMaxAllocation->SetTIPContent(0,cmCapReceiveAndTransmit);
			}
			if( m_pSipCntl->GetFullContentRate() == 0 )
			{
				PTRACE(eLevelInfoNormal,"CSipParty::HandleContentScmAndCaps : SetFullContentRate");
				m_pSipCntl->SetFullContentRate(m_PartyContentRate);
				m_pSipCntl->AddContentCapIfNeeded(m_pTargetMode, (CapEnum)(m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePresentation)));
				m_pSipCntl->SetLocalSdesKeysAndTagByHost(m_pTargetMode, m_pTargetModeMaxAllocation, cmCapVideo, kRolePresentation);
				m_pSipCntl->RemoveUnsupportedSdesCapsForCiscoCallIfNeeded();
				pLocalCaps = m_pSipCntl->GetLocalCaps();
			}

			CSdesCap *pSdesCap = pLocalCaps->GetSdesCap(cmCapVideo, kRolePresentation); AUTO_DELETE(pSdesCap);
			if(pSdesCap == NULL)
				TRACEINTO << "pSdesCap is NULL";

			if ((m_pTargetMode->GetIsEncrypted() == Encryp_On) && pSdesCap)
			{
				m_pTargetMode->SetSipSdes(cmCapVideo, cmCapReceive, kRolePresentation, pSdesCap); // Sdes for receive direction will be updated after remote will send its 200ok
				m_pTargetMode->SetSipSdes(cmCapVideo, cmCapTransmit, kRolePresentation, pSdesCap);
			}

			m_bTransactionSetContentOn = TRUE;

			PTRACE2INT(eLevelInfoNormal,"CSipParty::HandleContentScmAndCaps : Reinvite for dial out - m_bTransactionSetContentOn:", m_bTransactionSetContentOn);
		}
		m_pTargetMode->Dump("CSipParty::HandleContentScmAndCaps : set content in target:",eLevelInfoNormal);
	}
	else if (m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePresentation) == eH263CapCode) // target content is on, support only H263.
	{
		PTRACE(eLevelInfoNormal,"CSipParty::HandleContentScmAndCaps : Set H263 content in caps");

		m_pSipCntl->RemoveCapSet(eH264CapCode, kRolePresentation);
		m_pSipCntl->AddContentCapIfNeeded(m_pTargetMode, eH263CapCode); // in case that we had only H264 before, as a result of change mode to H264
	}
	else if (m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePresentation) == eH264CapCode) // target content is on, support H264.
	{
		PTRACE(eLevelInfoNormal,"CSipParty::HandleContentScmAndCaps : Set H264 content in caps");

		m_pSipCntl->RemoveCapSet(eH263CapCode, kRolePresentation);
		m_pSipCntl->AddContentCapIfNeeded(m_pTargetMode, eH264CapCode); // in case that we had only H263 before, as a result of change mode to H263
	}
}
/////////////////////////////////////////////////////////////////////////////
BOOL CSipParty::IsTIPContentEnable() const
{
    BOOL isTIPContentEnable = FALSE;
    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
    if (pCommConf)
        isTIPContentEnable = pCommConf->GetIsTipCompatibleContent();
    else
        PTRACE(eLevelError,"CSipParty::IsTIPContentEnable - pCommConf is NULL");
    return isTIPContentEnable;
}

//==================================================================================================================================
// ParseIncomingPrecedenceInfo - (Currently intended for INCOMING call use)
// ==========================================================================
//
// Parses the incoming headers to find whether resource-priority header (RFC-4412) is present, and tries to match it against
// configured diffserv tables.  A mismatch, if caller requests to check it, will have significance only if a require header
// contained resource-priority as a requirement.
//
// (Input - pHeaders from sip message, output - processed qos, and if pRequireHeaderFailure is not NULL, it will be raised if a
//   mismatch failed the above described require-header check).
//==================================================================================================================================
BOOL CSipParty::ParseIncomingPrecedenceInfo(const sipMessageHeaders* const pHeaders, CQoS& qos, BOOL* pRequireHeaderFailure) const
{
	//=======================
	// Initializing outputs
	//=======================
	BOOL qosMatched = FALSE;
	if (pRequireHeaderFailure) *pRequireHeaderFailure = FALSE;

	//==========================================================================
	// Checking if this call is a dynamic precedence call (R-Priority headers)
	//==========================================================================
	const CPrecedenceSettings* pPrecedSettings = GetpPrecedenceSettingsDB();
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
	CConfParty* pConfParty = NULL;
    if (pCommConf)  pConfParty = pCommConf->GetCurrentParty(GetMonitorPartyId());
	if (IsPartyIn() && pConfParty && pHeaders && pPrecedSettings && pPrecedSettings -> IsPrecedenceEnabled())
	{
	  //=======================================
	  // RMX should regard precedence headers
	  //=======================================
	  PTRACE(eLevelInfoNormal,"CSipParty::ParseIncomingPrecedenceInfo - checking for precedence headers");
	  CSipHeaderList headerList(*pHeaders);
	  const CSipHeader* pQosSipHeader = headerList.GetNextPrivateOrProprietyHeader(kProprietyHeader, strlen(QOS_HEADER_PREFIX), QOS_HEADER_PREFIX);
	  const char* strPreced = NULL;
	  if (pQosSipHeader) strPreced = pQosSipHeader->GetHeaderStr();
	  if (strPreced)
	  {
		  PTRACE2(eLevelInfoNormal,"CSipParty::ParseIncomingPrecedenceInfo - full precedence header extracted - ", strPreced);
		  strPreced += sizeof(QOS_HEADER_PREFIX) + 1; 	// (skipping header, semicolon & space)
	  }

	  if(strPreced && strPreced[0])
	  {
		  //==========================
		  // Found R-Priority header
		  //==========================
		  TRACEINTO << "CSipParty::ParseIncomingPrecedenceInfo - parsing QoS field - " << strPreced;
		  char szDomain[ONE_LINE_BUFFER_LEN];
		  szDomain[0] = '\0';
		  const char* precedSeperator = strchr(strPreced, '.');
		  int domainLen = precedSeperator - strPreced;
		  WORD	dRPrio = DEFAULT_PRECEDENCE_R_PRIORITY;
		  if (domainLen <= int(sizeof(szDomain) - 2) && precedSeperator && domainLen > 0)
		  {
              int maxLen = min(domainLen, ONE_LINE_BUFFER_LEN - 1);
			  strncpy(szDomain, strPreced, maxLen);
			  szDomain[maxLen] = '\0';
			  ++precedSeperator;
			  dRPrio = atoi(precedSeperator);
			  if (dRPrio == 0)
			  {
				  const char* RPrioNdx;
				  for (RPrioNdx = precedSeperator; *RPrioNdx != '\0' && !isalpha(*RPrioNdx) ; ++RPrioNdx) {}
				  if (*RPrioNdx != '\0')
				  {
					  dRPrio = DEFAULT_PRECEDENCE_R_PRIORITY;
					  PTRACE2(eLevelError, "CSipParty::ParseIncomingPrecedenceInfo - failed parsing priority-value: ", precedSeperator);
				  }
			  }
		  }
		  else
		  {
			  CMedString log;
			  log << "CSipParty::ParseIncomingPrecedenceInfo - failed parsing resource-priority header, seperator " << (precedSeperator? "": "not ") << "found";
			  if (precedSeperator) log << " domain Len is " << domainLen;
			  PTRACE(eLevelError, log.GetString());
		  }

		  if (dRPrio != DEFAULT_PRECEDENCE_R_PRIORITY)
		  {
			  //======================================================
			  // QoS params retrieved successfully - validating them
			  //======================================================
			  BYTE precedLevel = NUM_PRECEDENCE_LEVELS;
			  TRACEINTO << "CSipParty::ParseIncomingPrecedenceInfo - received domain[" << szDomain << "]  R-Priority[" << dRPrio << "]";
			  BYTE domainId = pPrecedSettings -> GetDomainId(szDomain);
			  if (domainId < NUM_SINGLE_DOMAINS) precedLevel = pPrecedSettings -> GetPrecedenceLevelForRPrio(domainId, (BYTE) dRPrio);

			  //==============================================
			  // If R-Prio fully matched, updating databases
			  //==============================================
			  if (precedLevel < NUM_PRECEDENCE_LEVELS)
			  {
				  	PTRACE2INT(eLevelInfoNormal, "CSipParty::ParseIncomingPrecedenceInfo - matched precedence level updating databases.  precedLevel:", precedLevel);
					qosMatched = TRUE;
					qos.AssembleValFromRPrio(szDomain, (BYTE)dRPrio);
					pConfParty -> SetPrecedenceDomain(szDomain);
					pConfParty -> SetPrecedenceLevel(precedLevel);
			  }
		  }
	  }

	  //================================================================================================
	  // Checking whether caller wished to check for require header
	  // (A precedence mismatch is unimportant, unless a resource-priority was REQUIRED by the remote)
	  //================================================================================================
	  if (!qosMatched && pRequireHeaderFailure)
	  {
		  const CSipHeader* require = headerList.GetNextHeader(kRequire);
		  const char* strRequire = (require? require -> GetHeaderStr() : NULL);
		  PTRACE2(eLevelInfoNormal, "CSipParty::ParseIncomingPrecedenceInfo - require string:", strRequire);
		  if (strRequire && strstr(strRequire, "resource-priority"))
		  {
			PTRACE(eLevelError, "CSipParty::ParseIncomingPrecedenceInfo - could not match precedence level for a REQUIRED r-priority");
			*pRequireHeaderFailure = TRUE;
		  }
	  }
	}
	else if (!IsPartyIn())
	{
		PTRACE(eLevelError, "CSipParty::ParseIncomingPrecedenceInfo - Intended for incoming party use");
	}
	else if (!pConfParty)
	{
		PTRACE(eLevelError, "CSipParty::ParseIncomingPrecedenceInfo - pConfParty NULL");
	}
	else if (!pHeaders)
	{
		PTRACE(eLevelError, "CSipParty::ParseIncomingPrecedenceInfo - pHeaders NULL");
	}
	else if (!pPrecedSettings)
	{
		PTRACE(eLevelError, "CSipParty::ParseIncomingPrecedenceInfo - pPrecedSettings NULL");
	}
	else if (!pPrecedSettings -> IsPrecedenceEnabled())
	{
		PTRACE(eLevelInfoNormal, "CSipParty::ParseIncomingPrecedenceInfo - dynamic precedence not enabled");
	}

	return qosMatched;
}


/////LyncCCS///////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfPwdInd(CSegment* pParam)
{

	char strPwd[64];

	DWORD len;
	//PTRACE(eLevelInfoNormal, "CSipParty::OnConfPwdInd");
	memset(strPwd, '\0', 64);

	*pParam >> len;
	if (len)
	{
		pParam->Get( (BYTE*)&strPwd, len );
		PTRACE2INT(eLevelInfoNormal, "CSipParty::OnConfPwdInd - pwd len: ", len);

		//BRIDGE-7651/BRIDGE-7720  IVR service not enbaled
		if((m_ivrCtrl)&&(0 == m_ivrCtrl->GetConfPwdEnabled()))
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnConfPwdInd - pwd IVR service is not enabled!");
			m_pConfApi->ConfAuthStatusNotify(GetPartyId(), 1);
		}
	}
	//If the len == 0
	else
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnConfPwdInd - pwd length is zero!");
		m_pConfApi->ConfAuthStatusNotify(GetPartyId(), 1);
		return;
	}

	if((m_ivrCtrl)&&(m_ivrCtrl->GetConfPwdEnabled()))
	{
		PTRACE2(eLevelInfoNormal, "CSipParty::OnConfPwdInd - pwd: ", strPwd);
		m_pSipCntl->PartyAuthIndReceiveFromConf(strPwd);
	}
}
////////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyPwdReq(CSegment* pParam)
{
	BYTE   status;
	*pParam >> status;
	PTRACE2INT(eLevelInfoNormal, "CSipParty::OnPartyPwdReq-status: ", status);

	m_pConfApi->ConfAuthStatusNotify(GetPartyId(), status);

}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::CollectDigitsExternalIvr(DialogState& state, const CollectElementType& collect)
{
	TRACEINTO << state;

	if (m_ivrCtrl)
	{
		if (m_ivrCtrl->GetState() == NOTACTIVE)
		{
			PASSERTMSG(TRUE, "CSipParty::CollectDigitsExternalIvr - attempting to collect dtmf while IVR was not started! starting IVR now.");
			StartIvrFinally();
		}
		CSegment* pParam = new CSegment;
		*pParam << state << (void*)&collect;
		//TODO check which params we should send. if the party should know on fail - send params, else - send object
		//m_ivrCntl->OnCollectDigits(pParam); //pParam - Num of digits ,PointerFor response ,TimeOut....
		m_ivrCtrl->HandleEvent(pParam, 0, EXTERNAL_IVR_COLLECT_DIGITS);
	}

}

////////////////////////////////////////////////////////////////////////////////
void CSipParty::PlayFileExternalIvr(DialogState& state, const MediaElementType& media)
{
	MediaFileTypeEnum type = CMediaTypeManager::DeriveMediaType(media.m_type, media.m_loc);
	OPCODE opcode = 0;

	switch (type)
	{
	case mft_Audio:

		//BRIDGE-3510
		TRACEINTO << "is Tip call: " << (int)GetIsTipCall() << ", is master: " << m_tipPartyType;
		if(GetIsTipCall() && m_tipPartyType == eTipMasterCenter && m_pTargetMode->IsTipNegotiated() && m_SlaveAuxRsrcId != 0 ) //IN CASE THIS IS NOT TRUE IN THE FUTURE WE WANT  TO SEND ERROR TO MCCF channel
		{
			CSegment* pSeg = new CSegment;
			*pSeg << state << (void*)&media;
			ForwardEventToTipSlaves(pSeg, EXTERNAL_IVR_PLAY_MESSAGE_AUX);
		}


		if (m_ivrCtrl && m_bCAMReadyForIVR)
			opcode = EXTERNAL_IVR_PALY_MUSIC;
		break;

	case mft_Image:
			if (m_ivrCtrl /*&& m_bCAMReadyForSlide*/)
				opcode = EXTERNAL_IVR_SHOW_SLIDE;
		break;

	case mft_Video:
			if (m_ivrCtrl /*&& m_bCAMReadyForSlide*/)
				opcode = EXTERNAL_IVR_SHOW_SLIDE; //just for now
		break;

	case mft_Unknown:
		TRACEINTO << "Cannot derive media type either for MIME '" << media.m_type << "' or by URL:" << media.m_loc;
		break;

	default:
		PASSERTSTREAM(true, "Unexpected media type derived:" << type);
	}

	TRACEINTO << "opcode:" << opcode;

	if (opcode)
	{
		if (m_ivrCtrl->GetState() == NOTACTIVE)
		{
			PASSERTSTREAM(TRUE, "CSipParty::PlayFileExternalIvr - attempting to " << ((opcode == EXTERNAL_IVR_PALY_MUSIC)? "play music " : "show image ") << "while IVR was not started! starting IVR now.");
			StartIvrFinally();
		}
		CSegment* pParam = new CSegment;
		*pParam << state << (void*)&media;
		m_ivrCtrl->HandleEvent(pParam, 0, opcode);
	}
}

void CSipParty::ExternalIvrStartDialog(DialogState& state, DWORD mcmsDelayInMsecs)
{

	TRACEINTO << "opcode:" << EXTERNAL_IVR_DIALOG_START;
	//BRIDGE-3510
	TRACEINTO << "is Tip call: " << (int)GetIsTipCall() << "is master: " << m_tipPartyType;
//	if(GetIsTipCall() && m_tipPartyType == eTipMasterCenter && m_pTargetMode->IsTipNegotiated())	//IN CASE THIS IS NOT TRUE IN THE FUTURE WE WANT  TO SEND ERROR TO MCCF channel
//	{
//		if (m_SlaveAuxRsrcId != 0 )
//		{
//			DialogState new_state = state;
//			new_state.baseObject = ((MscIvr*)state.baseObject)->NewCopy();
//			CSegment pSeg;
//			pSeg << new_state;
//			ForwardEventToTipSlaves(&pSeg, EXTERNAL_IVR_MESSAGE_FOR_TIP_SLAVE);
//		}
//		else
//			PASSERTSTREAM(TRUE, "CSipParty::ExternalIvrStartDialog - TIP master EP is not updated with TIP slave aux- cannot forward external IVR dialog.");
//	}
	if (m_ivrCtrl->GetState() == NOTACTIVE)
	{
		PASSERTSTREAM(TRUE, "CSipParty::ExternalIvrStartDialog - attempting to start external IVR dialog while IVR was not started! starting IVR now.");
		StartIvrFinally();
	}
	CSegment* pParam = new CSegment;
	*pParam << state << mcmsDelayInMsecs;
	m_ivrCtrl->HandleEvent(pParam, 0, EXTERNAL_IVR_DIALOG_START);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::ForwardEventToTipSlaves(CSegment* pParam, OPCODE opCode)
{
	if (!GetIsTipCall() || m_tipPartyType != eTipMasterCenter)
		return;

	TRACEINTO << "Opcode:" << opCode;

	CSegment seg;
	if (pParam)
		seg.CopySegmentFromReadPosition(*pParam);

	WORD numOfSlaves = m_TipNumOfStreams - 1 + m_bIsAudioAux;
	SendMessageFromMasterToSlave(eTipSlaveAux, opCode, &seg);
	BOOL is_external_ivr_message =
			opCode == EXTERNAL_IVR_MESSAGE_FOR_TIP_SLAVE ||
			opCode == EXTERNAL_IVR_DTMF_BARGE_IN_MASTER_TO_SLAVE ||
			opCode == EXTERNAL_IVR_DIALOG_TIMEOUT_MASTER_TO_SLAVE ||
			opCode == EXTERNAL_IVR_DIALOG_START;

	if (numOfSlaves > 1 && !is_external_ivr_message && (
			opCode != EXTERNAL_IVR_PLAY_MESSAGE_AUX &&
			opCode != PLAY_MESSAGE_AUX &&
			opCode != STOP_PLAY_MESSAGE_AUX &&
			opCode != PLAY_ROLL_CALL_AUX)) //opcodes to aux only
	{
		PTRACE(eLevelInfoNormal, "CSipParty::ForwardEventToTipSlaves - Right + Left slaves");
		SendMessageFromMasterToSlave(eTipSlaveLeft, opCode, &seg);
		SendMessageFromMasterToSlave(eTipSlaveRigth, opCode, &seg);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::ChangeVideoOutForPolycomTip()
{
	PTRACE(eLevelInfoNormal, "CSipParty::ChangeVideoOutForPolycomTip");

	if (!m_bIsPolycomFromRTCP)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::ChangeVideoOutForPolycomTip - Not polycom EP");
		return;
	}

	WORD numOfSlaves 		= m_TipNumOfStreams -1 + m_bIsAudioAux;
	OPCODE opCode 			= CHANGE_VIDEO_OUT_TIP_POLYCOM;

	m_pConfApi->ChangeVideoOutForTipPolycom(GetPartyId());

	if (numOfSlaves > 1)
	{
		PTRACE(eLevelInfoNormal, "CSipParty::ChangeVideoOutForPolycomTip, TIP slaves right & left");
		SendMessageFromMasterToSlave(eTipSlaveLeft, opCode, NULL);
		SendMessageFromMasterToSlave(eTipSlaveRigth, opCode, NULL);
	}
}

//////////////////////////////////////////////////////
void CSipParty::OnUpdateArtWithSsrc(CSegment* pParam)
{
	DWORD ssrc;
	*pParam >> (DWORD &)ssrc;

	TRACEINTO << "MixModeSsrc:" << ssrc;
	m_pSipCntl->UpdateArtWithSsrc(ssrc);
}

//////////////////////////////////////////////////////
void CSipParty::OnPartyUpgradeToMixChannelsUpdated(CSegment* pParam)
{
	BOOL isVideoParty = FALSE;

	if (m_pTargetMode->IsMediaOn(cmCapVideo, cmCapTransmit, kRolePeople) ||
		m_pTargetMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePeople))
	{
		isVideoParty = TRUE;

		TRACEINTO << "Video participant";
	}
	else
	{
		TRACEINTO << "Audio only participant";
	}

	unsigned int channelHandle = INVALID_CHANNEL_HANDLE;

	if (!m_pSipCntl->GetIsMrcCall() && isVideoParty)
	{
		CSipChannel *pChannelIn=NULL;

		pChannelIn = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapReceive, kRolePeople);

		if (pChannelIn)
		{
			channelHandle = pChannelIn->GetChannelHandle();

			if (channelHandle== INVALID_CHANNEL_HANDLE)
			{
				TRACEINTO << "dynMixedErr invalid channelHandle";
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal, "CSipParty::OnTransChannelsConnectedAnycase channelHandle[0] ", channelHandle);
			}
		}
		else
		{
			TRACEINTO << "dynMixedErr pChannelIn is NULL";
		}
	}

	m_pConfApi->PartyUpgradeToMixChannelsUpdated(m_PartyRsrcID, channelHandle);
}

void CSipParty::OnSendMrmpStreamIsMustReq(CSegment* pParam)
{
	TRACEINTO<<"got req CONF_PARTY_MRMP_STREAM_IS_MUST_REQ";
//	CSegment* pseg=new CSegment(*pParam);
	m_pSipCntl->MrmpStreamIsMustReq(pParam);
}

void CSipParty::OnMrmpStreamIsMustAck()
{
	TRACEINTO<<"got ack CONF_PARTY_MRMP_STREAM_IS_MUST_ACK";
	m_pConfApi->UpdateMrmpStreamIsMustAck(m_PartyRsrcID,STATUS_OK);
}

void	 CSipParty::SetBfcpCachedMsgTime()
{
	DWORD currentTime = SystemGetTickCount().GetSeconds();

	m_BfcpMsgCachedTime = currentTime;
}

////////////////////////////////////////////////////////////////////////////
BOOL CSipParty::IsNeedToSendRtcpVideoPreference() const
{
	if (m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive))
	{
		CCapSetInfo TargetCapInfo	= (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive);
		CCapSetInfo CurrenttCapInfo	= (CapEnum)m_pCurrentMode->GetMediaType(cmCapVideo, cmCapReceive);

		if(TargetCapInfo == CurrenttCapInfo && ((CapEnum)TargetCapInfo == eRtvCapCode )/*&& (m_pSipCntl->GetRemoteIdent() == MicrosoftEP_R1 ||m_pSipCntl->GetRemoteIdent() == MicrosoftEP_R2 || m_pSipCntl->GetRemoteIdent() == MicrosoftEP_Lync_R1 || m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU )*/)
			return TRUE;
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
DWORD CSipParty::GetRtcpLyncPreferenseReqIntervalFromFlag()
{
	DWORD         nInterval  = 3; //Default
	CSysConfig*   sysConfig  = NULL;
	CProcessBase* pProcess   = CProcessBase::GetProcess();

	if(pProcess)
		sysConfig =	pProcess->GetSysConfig();

	if(sysConfig)
		sysConfig->GetDWORDDataByKey(CFG_KEY_RTCP_LYNC_PREFERENCE_REQ_INTERVAL, nInterval);

	return nInterval;
}

//BRIDGE-6350
////////////////////////////////////////////////////////////////////////////
void CSipParty::OnSendVideoPreferenceReqTout()
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnSendVideoPreferenceReqTout ");
	this->OnPartyCntlStartVideoPreference(NULL);

	if(IsNeedToSendRtcpVideoPreference())
	{
		DWORD nInterval = GetRtcpLyncPreferenseReqIntervalFromFlag();
		if(nInterval != 0)
		{
			TRACEINTO << " StartTimer(SIP_SEND_VIDEO_PREFERENCE_END_TRANS_TOUT, SECOND*" << (int)nInterval <<")";
			StartTimer(SIP_SEND_VIDEO_PREFERENCE_END_TRANS_TOUT, ((int)nInterval)*SECOND);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////
//upon RTCP VSR MSG IND
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyReceivedVsrInd(CSegment* pParam)
{
	ESipTransactionType eSipTransType;

	//if ice call -> check ice completed
	bool isIceCompleted =  m_pSipCntl->GetIsIceCall() ? (eIceNotConnected != m_pSipCntl->GetIceConnectivityStatus()) : true;

	PTRACE2INT(eLevelError,"CSipParty::OnPartyReceivedVsrInd, isIceCompleted=", isIceCompleted);

	if(!IsActiveTransaction())
	{
		if(/*m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU || */ m_pSipCntl->isMs2013Active() == eMsft2013AvMCU && isIceCompleted && m_isAllOutSlavesConnected)
		{
			TRACEINTO << " forward message to slaves controller";
			m_pConfApi->SendVsrMsgIndToSlavesController(GetPartyId(),pParam);
		}
		else if( m_pSipCntl->isMs2013Active() == eMsft2013AvMCU)
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedVsrInd -av-mcu 2013 and ICE is not ready yet! ");
			m_bMsftRecevVsrInActiveTrans = TRUE;
		}
		else{
			eSipTransType = kSipTransRTCPVsrInd;
			StartTransaction(eSipTransType, SIP_TRANS_VSR_MSG_IND, pParam);
		}
	}
	else// handle transaction in transaction
	{
		PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedVsrInd -transaction already active-pending or ice not completed");
		m_bMsftRecevVsrInActiveTrans = TRUE;
	}
}


////////////////////////////////////////////////////////////////////////////
void CSipParty::SendAvcToSvcArtTranslatorDisconnectedToPartyControl(STATUS status)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", ConfId:" << GetConfId() << ", status:" << status;
	m_pConfApi->SendAvcToSvcArtTranslatorDisconnectedToPartyControl(this, status);
}

////////////////////////////////////////////////////////////////////////////
void CSipParty::RemoveDtlsFromTargetModeIfNeeded( CIpComMode* pBestMode)
{
	if (m_pTargetMode->IsMediaOn(cmCapAudio,cmCapTransmit))
	{
		if (m_pTargetMode->IsDtlsChannelEnabled(cmCapAudio,cmCapTransmit, kRolePeople) && !pBestMode->IsDtlsChannelEnabled(cmCapAudio,cmCapTransmit, kRolePeople))
		{
			PTRACE(eLevelInfoNormal,"CSipParty::RemoveDtlsFromTargetModeIfNeeded: remove dtls from audio transmit target mode");
			m_pTargetMode->RemoveSipDtls(cmCapAudio,cmCapTransmit, kRolePeople);
		}

		if (m_pTargetMode->IsDtlsChannelEnabled(cmCapAudio,cmCapReceive, kRolePeople) && !pBestMode->IsDtlsChannelEnabled(cmCapAudio,cmCapReceive, kRolePeople))
		{
			PTRACE(eLevelInfoNormal,"CSipParty::RemoveDtlsFromTargetModeIfNeeded: remove dtls from audio receive target mode");
			m_pTargetMode->RemoveSipDtls(cmCapAudio,cmCapReceive, kRolePeople);
		}

	}

	if (m_pTargetMode->IsMediaOn(cmCapVideo,cmCapTransmit) )
	{
		if (m_pTargetMode->IsDtlsChannelEnabled(cmCapVideo,cmCapTransmit, kRolePeople) && !pBestMode->IsDtlsChannelEnabled(cmCapVideo,cmCapTransmit, kRolePeople))
		{
			PTRACE(eLevelInfoNormal,"CSipParty::RemoveDtlsFromTargetModeIfNeeded: remove dtls from video transmit target mode");
			m_pTargetMode->RemoveSipDtls(cmCapVideo,cmCapTransmit, kRolePeople);
		}

		if (m_pTargetMode->IsDtlsChannelEnabled(cmCapVideo,cmCapReceive, kRolePeople) && !pBestMode->IsDtlsChannelEnabled(cmCapVideo,cmCapReceive, kRolePeople))
		{
			PTRACE(eLevelInfoNormal,"CSipParty::RemoveDtlsFromTargetModeIfNeeded: remove dtls from video receive target mode");
			m_pTargetMode->RemoveSipDtls(cmCapVideo,cmCapReceive, kRolePeople);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransChangeVideoAfterVsrMsgAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransChangeVideoResAnycase");
	CIpComMode* pNewMode  = new CIpComMode;
	pNewMode->DeSerialize(NATIVE, *pParam);
	m_pConfApi->SendVideoUpdateAfterVsrMsgToPartyControl(GetPartyId(),pNewMode);
	POBJDELETE(pNewMode);

}

/////////////////////////////////////////////////////////////////////////////
// single pacsi from transaction to partycontrol
void  CSipParty::OnTransSendSingleUpdatePacsiInfoAnycase(CSegment* pParam)
{
	TRACEINTO;
	CIpComMode* pNewMode  = new CIpComMode;
	pNewMode->DeSerialize(NATIVE, *pParam);

	BYTE isMute = TRUE;
	*pParam >> isMute;

	m_pConfApi->SendSingleUpdatePacsiInfoToPartyControl(GetPartyId(), pNewMode, isMute);

	POBJDELETE(pNewMode);
}


/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnPartyMsftOutslavesCreated(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnPartyMsftOutslavesCreated");
	WORD  msgType = 0;
	DWORD PartyId = 0;
	DWORD status = statOK;
	DWORD outRate = 0;
	mcMuxLync2013InfoReq msSvcMuxMsg;
	memset(&msSvcMuxMsg, 0, sizeof(mcMuxLync2013InfoReq));

	m_isAllOutSlavesConnected = TRUE;

	*pParam >> PartyId;
	*pParam >> msgType;
	*pParam >> status;
	*pParam >> outRate;
	pParam->Get(reinterpret_cast<BYTE*> (&msSvcMuxMsg), sizeof(mcMuxLync2013InfoReq));

	PTRACE2INT(eLevelInfoNormal,"CSipParty::OnPartyMsftOutslavesCreated, rate=", outRate);

	m_pSipCntl->SetMsftTotalBW(outRate);

	StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);

	m_pSipCntl->MsftSendMsgToMux(msSvcMuxMsg);
}
/////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipParty::OnPartyReceivedAvMcuSingleFecOrRedMsg(CSegment* pParam)
{
	DWORD  mediaType = cmCapEmpty;
	DWORD  newFecRedPercent = 0;

	*pParam >> mediaType >> newFecRedPercent;

	 CSipCall *pCall = m_pSipCntl->GetCallObj();
	 CSipChannel* videoOutCh =  pCall->GetChannel(VIDEO_OUT);
	 if(videoOutCh && !videoOutCh->IsMuted())
	 {
		 TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId()
				   << " - party is not muted - send to sipControl HandleFecPacketLossInd. mediaType:" << mediaType
				   << " ,newFecRedPercent:" << newFecRedPercent;
		 m_pSipCntl->HandleFecPacketLossInd(newFecRedPercent, mediaType, 0);
	 }
	 else
		 TRACEINTO << "LYNC2013_FEC_RED: PartyID:" << GetPartyId()
		 << " - party is muted - do not send to sipControl HandleFecPacketLossInd. mediaType:"
		 << mediaType << " ,newFecRedPercent:" << newFecRedPercent;

}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnPartyReceivedAvMcuSingleVsrInd(CSegment* pParam)
{
	DWORD  msgType = 0;
	DWORD PartyId = 0;
	*pParam >> PartyId;
	*pParam >> msgType;

	TRACEINTO << "State:" << m_state << ", PartyId:" << PartyId << ", msgType:" << msgType;

	//if (GetMSSlavePartyType() == eMsSlaveDirection_out)
//	{
		ESipTransactionType eSipTransType;

		//if ice call -> check ice completed
		bool isIceCompleted =  m_pSipCntl->GetIsIceCall() ? (eIceNotConnected != m_pSipCntl->GetIceConnectivityStatus()) : true;
		if(!IsActiveTransaction())
		{
			PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedAvMcuSingleVsrInd -start");
			eSipTransType = kSipTransRTCPVsrInd;
			StartTransaction(eSipTransType, SIP_TRANS_VSR_MSG_IND, pParam);
		}
		else// handle transaction in transaction
		{
			//save VSR in sip party in case there is an active transaction. and handle it when this VSR transaction is ended
			PTRACE(eLevelInfoNormal,"CSipParty::OnPartyReceivedAvMcuSingleVsrInd -transaction already active-pending or ice not completed");
			POBJDELETE(m_pLastTxVsr);
			m_pLastTxVsr = new ST_VSR_SINGLE_STREAM;
			ST_VSR_SINGLE_STREAM vsr;
			pParam->Get(reinterpret_cast<BYTE*>(&vsr), sizeof(ST_VSR_SINGLE_STREAM));
			memcpy(m_pLastTxVsr,&vsr,sizeof(ST_VSR_SINGLE_STREAM));
			TRACEINTOFUNC << "Cache Vsr ( num_vsrs_params:" << m_pLastTxVsr->num_vsrs_params << ", msi: " << m_pLastTxVsr->msi;
		}
	//}
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::OnConfActiveMediaForAVMCULync()
{
	PTRACE2(eLevelInfoNormal,"CSipParty::OnConfActiveMediaForAVMCULync - ", m_partyConfName);
	if (!IsActiveTransaction())
	{
		StartTransaction(kSipTransReInviteWithSdpReq, SIP_PARTY_SEND_REINVITE, NULL);
	}
	else
		m_pSipTransaction->SetNeedReInviteToActiveMedia(TRUE);
}
/////////////////////////////////////////////////////////////////////////////
void CSipParty::SendMessageFromMSSlaveToMain(DWORD opcode, CSegment *pMsg)
{
	m_pConfApi->PartyToPartyCntlMsgFromMSSlaveToMain(GetPartyId(), opcode, pMsg);

}
//////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
BOOL  CSipParty::IsThereActiveInviteReq() const
{
	if ((m_eActiveTransactionType == kSipTransReInviteWithSdpReq ||m_eActiveTransactionType == kSipTransReInviteNoSdpReq)
		&& (IsValidPObjectPtr(m_pSipTransaction)))
		return TRUE;
	return FALSE;

}
////////////////////////////////////////////////////
//eFeatureRssDialin
void CSipParty::OnConfRecordingControlConnected(CSegment* pParam)
{

	PTRACE(eLevelInfoNormal,"CSipParty::OnConfRecordingControlConnected");

	if(NULL == pParam)
	{
		return;
	}
	std::string srsControlBuffer;
	*pParam >> srsControlBuffer;

	PTRACE2(eLevelInfoNormal,"CSipParty::OnConfRecordingControlConnected, Recording control string - ", srsControlBuffer.c_str());
	m_pSipCntl->SendRecordingControlCmd(srsControlBuffer.c_str());
	return;
}
////////////////////////////////////////////////////
//eFeatureRssDialin
void CSipParty::OnConfRecordingControlAnycase(CSegment* pParam)
{

	PTRACE2INT(eLevelInfoNormal,"CSipParty::OnConfRecordingControlAnycase, state - ", m_state);
	return;
}

////////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
void CSipParty::OnPartyRecordingControlStatusAnycase(CSegment* pParam)
{
	BYTE   status = 0;
	*pParam >> status;
	PTRACE2INT(eLevelInfoNormal, "CSipParty::OnPartyRecordingControlStatusAnycase - status: ", status);

	m_pConfApi->SendRecordingControlAckToConf(GetPartyId(), status);
	return;
}
////////////////////////////////////////////////////////////////////////////////
//eFeatureRssDialin
void CSipParty::OnPartyLayoutControlAnycase(CSegment* pParam)
{
	BYTE   layout = 0;
	*pParam >> layout;
	PTRACE2INT(eLevelInfoNormal, "CSipParty::OnPartyLayoutControlAnycase - layout: ", layout);

	m_pConfApi->SendLayoutControlToConf(GetPartyId(), layout);
	return;
}


////////////////////////////////////////////////////////////////////////////////
void  CSipParty::SetIceCheckCompleteState(BYTE bCheckCompleteState)
{
	m_bIsRcvCheckCompleteAndNeedToStartIVR = bCheckCompleteState;
}


/////////////////////////////////////////////////////////////////////////////
void  CSipParty::PartyUpdatePartyControlOnRemoteCaps()
{
	//send party control the remote caps
	TRACEINTO;
	CSipCaps*	pCurRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
	if(pCurRemoteCaps)
		m_pConfApi->UpdatePartyControlOnRemoteCaps(GetPartyId(),pCurRemoteCaps);
}

/////////////////////////////////////////////////////////////////////////////
void CSipParty::StopWaitForContentReInvite()
{
	TRACEINTO;
	if (IsValidTimer(CONTENT_WAIT_REINVITE_TOUT))  // Content timer
		DeleteTimer(CONTENT_WAIT_REINVITE_TOUT);
}
/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
void CSipParty::SendByeToPartyAnycase(CSegment* pParam)
{
	PartyRsrcID PartyId;
	*pParam >> PartyId;
	PTRACE2INT(eLevelInfoNormal, "CSipParty::OnPartyLayoutControlAnycase - PartyId: ", PartyId);

	m_pSipCntl->SipByeReq();
	return;
}
/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/
/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTransLastTargetModeMsgAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipParty::OnTransLastTargetModeMsgAnycase");
	CIpComMode* pNewMode  = new CIpComMode;
	pNewMode->DeSerialize(NATIVE, *pParam);

	//BRIDGE-12961
	DWORD contentRateTx 	   = pNewMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	DWORD contentRateRx 	   = pNewMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
    BOOL  bIsActiveContent = GetIsActiveContent();

    if( bIsActiveContent && (contentRateTx == 0 || contentRateRx == 0) )
    {
       DWORD contentRate = m_pSipCntl->GetFullContentRate();
       pNewMode->SetVideoBitRate(contentRate, cmCapTransmit, kRoleContentOrPresentation);
       pNewMode->SetVideoBitRate(contentRate, cmCapReceive, kRoleContentOrPresentation);
    }
	
	m_pConfApi->SendLastTargetModeMsgToPartyControl(GetPartyId(),pNewMode);
	POBJDELETE(pNewMode);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipParty::OnTokenRecapCollisionEndAnycase(CSegment* pParam)
{
	CConfParty* pConfParty = GetConfPartyNonConst();
	PASSERT_AND_RETURN(!pConfParty);
	PASSERT_AND_RETURN(!m_pSipCntl);

	BOOL 								bTokenRecapPendedDueToCollisionDetection	= pConfParty -> IsTokenRecapPendedDueToCollisionDetection();
	eTokenRecapCollisionDetectionType	eTokenRecapCollisionDetection 				= pConfParty -> GetTokenRecapCollisionDetection();

	TRACEINTO << "Reset all pends";
	//==========================================
	// Resetting pends (note: also for errors)
	//==========================================
	pConfParty -> SetTokenRecapCollisionDetection(etrcdAvailable);
	pConfParty -> UnpendTokenRecapDueToCollisionDetection();

	if (bTokenRecapPendedDueToCollisionDetection && eTokenRecapCollisionDetection == etrcdTokenHandlingInProgress)
	{
		//=========================================
		// A reinvite got pended, handling it now
		//=========================================
		PTRACE(eLevelInfoNormal,"CSipParty::OnTokenRecapCollisionEndAnycase - Invoking OnSipReInviteIndConnected for pended reinvite");
		m_pSipCntl -> OnSipReInviteIndConnected(&m_recapPendedOnToken);
	}
	else
	{
		//=======================================================
		// This event should not have been sent in these states
		//=======================================================
		CSmallString log;
		log << "CSipParty::OnTokenRecapCollisionEndAnycase - called in invalid state.  Pend state is [" <<
				bTokenRecapPendedDueToCollisionDetection << "] collision state is [" << eTokenRecapCollisionDetection << "]";
		PTRACE(eLevelError, log.GetString());
	}


}
