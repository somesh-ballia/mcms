//+========================================================================+
//                            H323Party.CPP                                |
//            Copyright 1995 Polycom Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       H323Party.CPP                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 18/7/05     |                                                     |
//+========================================================================+

#include "H323Party.h"
#include "H323Caps.h"
#include "H323NetSetup.h"
#include "ConfApi.h"
#include "ConfPartyOpcodes.h"
#include "H323CsReq.h"
#include "StructTm.h"
#include "SystemFunctions.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "Trace.h"
#include "RsrcParams.h"
#include "AllocateStructs.h"
#include "H323Util.h"
#include "EncryptionKey.h"
#include "ConfPartyTimeOut.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "ManagerApi.h"
#include "IpCommon.h"
#include "SysConfigKeys.h"
#include "IpAddressDefinitions.h"
#include <vector>
#include "IVRCntl.h"
#include "TerminalListManager.h"
#include "ConfPartyProcess.h"

extern "C" long GetMCUIP();
extern const char* feccKeyToString(feccKeyEnum key);

#ifndef PARTY
#define PARTY 2
#endif

const char* PartyStateStrings[PartyStateNo] = {"PARTYIDLE","PARTYSETUP","PARTYCHANGEMODE","CONNECT","DISCONNECTING"};

//Party monitoring values
#define NUM_OF_PM_REQ_IN_SEC 10
#define MASK_FOR_VSW_AVC_PIPEID 0x000FFFFF

// RTP ack for COntent On/Off tout
//const WORD RTPCONTENTACKTOUT 		= 20;
//const WORD RTPEVACUATEACKTOUT		= 21;

PBEGIN_MESSAGE_MAP(CH323Party)

  ONEVENT(INCREASE_DISCONNECT_TIMER,                PARTYDISCONNECTING, CH323Party::IncreaseDisconnctingTimerInPartyCntl)

  ONEVENT(IPLOGICALCHANNELCONNECT,                  ANYCASE,            CH323Party::OnH323LogicalChannelConnect)

  ONEVENT(IPLOGICALCHANNELUPDATE,                   ANYCASE,            CH323Party::OnH323LogicalChannelUpdate)

  ONEVENT(IPLOGICALCHANNELDISCONNECT,               PARTYCHANGEMODE,    CH323Party::OnH323LogicalChannelDisConnectChangeMode)
  ONEVENT(IPLOGICALCHANNELDISCONNECT,               PARTYSETUP,         CH323Party::OnH323LogicalChannelDisConnect)
  ONEVENT(IPLOGICALCHANNELDISCONNECT,               PARTYCONNECTED,     CH323Party::OnH323LogicalChannelDisConnect)
  ONEVENT(IPLOGICALCHANNELDISCONNECT,               PARTYDISCONNECTING, CH323Party::OnH323LogicalChannelDisConnect)

  ONEVENT(H323GATEKEEPERSTATUS,                     ANYCASE,            CH323Party::OnH323GateKeeperStatus)

  ONEVENT(IPPARTYMONITORING,                        ANYCASE,            CH323Party::OnH323PartyMonitoring)

  ONEVENT(RMT323COMMODUPDATE,                       ANYCASE,            CH323Party::OnH323Rmt323ComModUpdateDB)

  ONEVENT(ENDCHANNELCONNECT,                        PARTYSETUP,         CH323Party::OnH323EndChannelConnectSetupOrConnect)
  ONEVENT(ENDCHANNELCONNECT,                        PARTYCONNECTED,     CH323Party::OnH323EndChannelConnectSetupOrConnect)
  ONEVENT(ENDCHANNELCONNECT,                        PARTYCHANGEMODE,    CH323Party::OnH323EndChannelConnectChangeMode)
  ONEVENT(ENDCHANNELCONNECT,                        PARTYDISCONNECTING, CH323Party::NullActionFunction)

  ONEVENT(PARTY_CLOSE_CHANNEL,                      PARTYIDLE,          CH323Party::SendCloseChannelToConfLevel)
  ONEVENT(PARTY_CLOSE_CHANNEL,                      PARTYCONNECTED,     CH323Party::SendCloseChannelToConfLevel)
  ONEVENT(PARTY_CLOSE_CHANNEL,                      PARTYCHANGEMODE,    CH323Party::SendCloseChannelToConfLevel)
  ONEVENT(PARTY_CLOSE_CHANNEL,                      PARTYSETUP,         CH323Party::SendCloseChannelToConfLevel)
  ONEVENT(PARTY_CLOSE_CHANNEL,                      PARTYDISCONNECTING, CH323Party::OnH323DisconectChannelDisconnecting)

  ONEVENT(RMTH230,                                  PARTYSETUP,         CH323Party::OnPartyRemoteH230)
  ONEVENT(RMTH230,                                  PARTYCHANGEMODE,    CH323Party::OnPartyRemoteH230)
  ONEVENT(RMTH230,                                  PARTYCONNECTED,     CH323Party::OnPartyRemoteH230)
  ONEVENT(RMTH230,                                  ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(VIDEOMUTE,                                PARTYSETUP,         CH323Party::OnH323VideoMuteSetup)
  ONEVENT(VIDEOMUTE,                                PARTYCHANGEMODE,    CH323Party::OnH323VideoMuteChangeMode)
  ONEVENT(VIDEOMUTE,                                PARTYCONNECTED,     CH323Party::OnH323VideoMuteConnect)

  ONEVENT(ADDPROTOCOL,                              PARTYIDLE,          CH323Party::SendAddedProtocolToConfLevel)
  ONEVENT(ADDPROTOCOL,                              PARTYSETUP,         CH323Party::SendAddedProtocolToConfLevel)

  ONEVENT(REMOVEPROTOCOL,                           PARTYIDLE,          CH323Party::SendRemovedProtocolToConfLevel)
  ONEVENT(REMOVEPROTOCOL,                           PARTYSETUP,         CH323Party::SendRemovedProtocolToConfLevel)

  ONEVENT(IP_DTMF_INPUT_IND,                        PARTYCONNECTED,     CH323Party::OnH323DTMFInd)
  ONEVENT(IP_DTMF_INPUT_IND,                        PARTYSETUP,         CH323Party::OnH323DTMFInd)

  ONEVENT(SET_SITE_AND_VISUAL_NAME,                 PARTYIDLE,          CH323Party::SendSiteAndVisualNamePlusProductIdToPartyControl)
  ONEVENT(SET_SITE_AND_VISUAL_NAME,                 PARTYSETUP,         CH323Party::SendSiteAndVisualNamePlusProductIdToPartyControl)
  ONEVENT(SET_SITE_AND_VISUAL_NAME,                 PARTYCONNECTED,     CH323Party::SendSiteAndVisualNamePlusProductIdToPartyControl)
  ONEVENT(SET_SITE_AND_VISUAL_NAME,                 PARTYDISCONNECTING, CH323Party::NullActionFunction)

  ONEVENT(SECONDARYCAUSEH323,                       PARTYSETUP,         CH323Party::SetPartySecondaryCause)
  ONEVENT(SECONDARYCAUSEH323,                       PARTYCONNECTED,     CH323Party::SetPartySecondaryCause)
  ONEVENT(SECONDARYCAUSEH323,                       PARTYCHANGEMODE,    CH323Party::SetPartySecondaryCause)

  ONEVENT(IPPARTYMSECONDARY,                        PARTYIDLE,          CH323Party::OnPartyDowngradeToSecondary)
  ONEVENT(IPPARTYMSECONDARY,                        ANYCASE,            CH323Party::OnPartyDowngradeToSecondary)

  //Multiple links for ITP in cascaded conference feature:
  ONEVENT(ITPSPEAKERIND,                            ANYCASE,            CH323Party::OnPartyUpdateITPSpeaker)
  ONEVENT(ITPSPEAKERACKIND,                         ANYCASE,            CH323Party::OnPartyUpdateITPSpeakerAck)
  ONEVENT(PARTY_SEND_ITPSPEAKER_ACKREQ,             ANYCASE,            CH323Party::OnPartySendITPSpeakerAckReq)
  ONEVENT(VB_SEND_ITPSPEAKER_REQ,                   ANYCASE,            CH323Party::OnVBSendITPSpeakerReq)

  ONEVENT(UPDATE_CAPS,                              PARTYSETUP,         CH323Party::UpdateLocalCapsInConfLevel)
  ONEVENT(UPDATE_CAPS,                              PARTYCONNECTED,     CH323Party::UpdateLocalCapsInConfLevel)
  ONEVENT(UPDATE_CAPS,                              PARTYCHANGEMODE,    CH323Party::UpdateLocalCapsInConfLevel)
  ONEVENT(UPDATE_CAPS,                              PARTYDISCONNECTING, CH323Party::NullActionFunction)

  ONEVENT(UPDATE_VIDEO_RATE,                        ANYCASE,            CH323Party::UpdatePartyH323VideoBitRate)

  ONEVENT(IPDISCONNECTCHANNEL,                      PARTYIDLE,          CH323Party::OnConfDisconnectMediaChannel)
  ONEVENT(IPDISCONNECTCHANNEL,                      ANYCASE,            CH323Party::OnConfDisconnectMediaChannel)

  ONEVENT(INACTIVATE_CHANNEL,                       IDLE,               CH323Party::OnConfDeActiveMediaChannel)
  ONEVENT(INACTIVATE_CHANNEL,                       ANYCASE,            CH323Party::OnConfDeActiveMediaChannel)

  ONEVENT(PARTY_FLOWCONTROL,                        PARTYCONNECTED,     CH323Party::SendFlowControlToCs)
  ONEVENT(PARTY_FLOWCONTROL,                        PARTYCHANGEMODE,    CH323Party::SendFlowControlToCs)
  ONEVENT(PARTY_FLOWCONTROL,                        PARTYDISCONNECTING, CH323Party::NullActionFunction)
  ONEVENT(PARTY_FLOWCONTROL,                        IDLE,               CH323Party::NullActionFunction)
  ONEVENT(PARTY_FLOWCONTROL,                        PARTYSETUP,         CH323Party::NullActionFunction)

  ONEVENT(IPPARTYMONITORINGREQ,                     ANYCASE,            CH323Party::OnMcuMngrPartyMonitoringReq)

  ONEVENT(CONTENTVIDREFRESH,                        PARTYCONNECTED,     CH323Party::OnContentBrdgRefreshVideo)
  ONEVENT(CONTENTVIDREFRESH,                        PARTYCHANGEMODE,    CH323Party::OnContentBrdgRefreshVideo)

  ONEVENT(VIDREFRESH,                               PARTYCONNECTED,     CH323Party::OnVidBrdgRefresh)
  ONEVENT(VIDREFRESH,                               PARTYCHANGEMODE,    CH323Party::OnVidBrdgRefresh)

  ONEVENT(CONTENTFREEZEPIC,                         ANYCASE,            CH323Party::NullActionFunction)
  ONEVENT(FREEZPIC,                                 ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(CONFCONTENTTOKENMSG,                      PARTYCHANGEMODE,    CH323Party::OnConfContentTokenMessage)
  ONEVENT(CONFCONTENTTOKENMSG,                      PARTYCONNECTED,     CH323Party::OnConfContentTokenMessage)

  ONEVENT(CONTENTRATECHANGE,                        PARTYCHANGEMODE,    CH323Party::OnConfContentRateChange)
  ONEVENT(CONTENTRATECHANGE,                        PARTYCONNECTED,     CH323Party::OnConfContentRateChange)

  ONEVENT(CONFCONTENTCHANGEMODE,                    PARTYCHANGEMODE,    CH323Party::OnConfContentChangeMode)
  ONEVENT(CONFCONTENTCHANGEMODE,                    PARTYCONNECTED,     CH323Party::OnConfContentChangeMode)
  ONEVENT(CONFCONTENTCHANGEMODE,                    PARTYDISCONNECTING, CH323Party::OnConfContentChangeModeDisconnecting)

  ONEVENT(CONTENT_MESSAGE_FROM_MASTER,              PARTYCHANGEMODE,    CH323Party::OnContentMsgFromMaster)
  ONEVENT(CONTENT_MESSAGE_FROM_MASTER,              PARTYCONNECTED,     CH323Party::OnContentMsgFromMaster)

  ONEVENT(CONTENTRATECHANGEDONE,                    PARTYCHANGEMODE,    CH323Party::OnContentRateChangeDone)
  ONEVENT(CONTENTRATECHANGEDONE,                    PARTYCONNECTED,     CH323Party::OnContentRateChangeDone)

  ONEVENT(HW_CONTENT_ON_OFF_ACK,                    PARTYCONNECTED,     CH323Party::OnRtpAckForContentOnOff)
  ONEVENT(HW_CONTENT_ON_OFF_ACK,                    PARTYCHANGEMODE,    CH323Party::OnRtpAckForContentOnOff)
  ONEVENT(HW_CONTENT_ON_OFF_ACK,                    PARTYDISCONNECTING, CH323Party::OnRtpAckForContentOnOffWhileDisconnecting)

  ONEVENT(RTPCONTENTACKTOUT,                        PARTYCONNECTED,     CH323Party::OnRtpAckForContentTout)
  ONEVENT(RTPCONTENTACKTOUT,                        PARTYCHANGEMODE,    CH323Party::OnRtpAckForContentTout)
  ONEVENT(RTPCONTENTACKTOUT,                        PARTYDISCONNECTING, CH323Party::NullActionFunction)

  ONEVENT(RTPEVACUATEACKTOUT,                       PARTYCONNECTED,     CH323Party::OnRtpAckForEvacuateTout)
  ONEVENT(RTPEVACUATEACKTOUT,                       PARTYCHANGEMODE,    CH323Party::OnRtpAckForEvacuateTout)
  ONEVENT(RTPEVACUATEACKTOUT,                       PARTYDISCONNECTING, CH323Party::OnRtpAckForEvacuateTout)

  ONEVENT(RTP_EVACUATE_ON_OFF_ACK,                  PARTYCONNECTED,     CH323Party::OnRtpAckForEvacuateContentStream)
  ONEVENT(RTP_EVACUATE_ON_OFF_ACK,                  PARTYCHANGEMODE,    CH323Party::OnRtpAckForEvacuateContentStream)
  ONEVENT(RTP_EVACUATE_ON_OFF_ACK,                  PARTYDISCONNECTING, CH323Party::NullActionFunction)

  ONEVENT(PARTY_TOKEN_MESSAGE,                      ANYCASE,            CH323Party::SendTokenMessageToConfLevel)
  ONEVENT(CG_PARTY_TOKEN_MESSAGE,                   ANYCASE,            CH323Party::CallGeneratorRecieveTokenFromRemote)
  ONEVENT(MEDIA_PRODUCER_STATUS,                    ANYCASE,            CH323Party::SendMediaProducerStatusToConfLevel)

  ONEVENT(PARTYENDCHANGEMODE,                       PARTYSETUP,         CH323Party::SendEndChangeContentToConfLevel)
  ONEVENT(PARTYENDCHANGEMODE,                       PARTYCHANGEMODE,    CH323Party::SendEndChangeContentToConfLevel)
  ONEVENT(PARTYENDCHANGEMODE,                       PARTYCONNECTED,     CH323Party::SendEndChangeContentToConfLevel)

  ONEVENT(PARTY_RECEIVE_ECS,                        IDLE,               CH323Party::SendECSToPartyControl)
  ONEVENT(PARTY_RECEIVE_ECS,                        PARTYSETUP,         CH323Party::SendECSToPartyControl)
  ONEVENT(PARTY_RECEIVE_ECS,                        PARTYCONNECTED,     CH323Party::SendECSToPartyControl)
  ONEVENT(PARTY_RECEIVE_ECS,                        PARTYCHANGEMODE,    CH323Party::SendECSToPartyControl)

  ONEVENT(CONF_UPDATE_CAPS_AND_AUDIORATE,           ANYCASE,            CH323Party::ConfUpdatedCapsAndAudioRate)
  ONEVENT(CONFCHANGEMODE,                           PARTYCONNECTED,     CH323Party::OnConfChangeModeConnect)
  ONEVENT(CONFCHANGEMODE,                           PARTYCHANGEMODE,    CH323Party::OnConfChangeModeConnect)
  ONEVENT(SETMOVEPARAMS,                            ANYCASE,            CH323Party::OnConfSetMoveParams)
  ONEVENT(REJECT_NEW_CHANNEL,                       PARTYCHANGEMODE,    CH323Party::OnH323LogicalChannelRejectChangeMode)

  ONEVENT(IP_STREAM_VIOLATION,                      PARTYCHANGEMODE,    CH323Party::OnH323StreamViolationChangeMode)
  ONEVENT(IP_STREAM_VIOLATION,                      PARTYCONNECTED,     CH323Party::OnH323StreamViolationConnect)

  ONEVENT(REMOTE_SENT_RE_CAPS,                      PARTYSETUP,         CH323Party::NullActionFunction)
  ONEVENT(REMOTE_SENT_RE_CAPS,                      PARTYCONNECTED,     CH323Party::SendReCapsToPartyControl)
  ONEVENT(REMOTE_SENT_RE_CAPS,                      PARTYCHANGEMODE,    CH323Party::SendReCapsToPartyControl)
  ONEVENT(REMOTE_SENT_RE_CAPS,                      PARTYDISCONNECTING, CH323Party::NullActionFunction)

  ONEVENT(PARTYCHANGEMODETOUT,                      PARTYCHANGEMODE,    CH323Party::OnTimerChangeModeTimeOut)

  ONEVENT(NUMBERINGMESSAGE,                         PARTYIDLE,          CH323Party::OnBridgeNumberingMessage)
  ONEVENT(NUMBERINGMESSAGE,                         ANYCASE,            CH323Party::OnBridgeNumberingMessage)

  ONEVENT(DATATOKENREQ,                             PARTYCONNECTED,     CH323Party::OnFeccBridgeTokenRequest)
  ONEVENT(DATATOKENREQ,                             PARTYCHANGEMODE,    CH323Party::OnFeccBridgeTokenRequest)
  ONEVENT(DATATOKENREQ,                             ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(DATATOKENACCEPT,                          PARTYCONNECTED,     CH323Party::OnFeccBridgeTokenAccept)
  ONEVENT(DATATOKENACCEPT,                          PARTYCHANGEMODE,    CH323Party::OnFeccBridgeTokenAccept)
  ONEVENT(DATATOKENACCEPT,                          ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(DATATOKENREJECT,                          PARTYCONNECTED,     CH323Party::OnFeccBridgeTokenReject)
  ONEVENT(DATATOKENREJECT,                          PARTYCHANGEMODE,    CH323Party::OnFeccBridgeTokenReject)
  ONEVENT(DATATOKENREJECT,                          ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(DATATOKENWITHDRAW,                        PARTYCONNECTED,     CH323Party::OnFeccBridgeTokenWithdraw)
  ONEVENT(DATATOKENWITHDRAW,                        PARTYCHANGEMODE,    CH323Party::OnFeccBridgeTokenWithdraw)
  ONEVENT(DATATOKENWITHDRAW,                        ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(DATATOKENRELEASEREQ,                      PARTYCONNECTED,     CH323Party::OnFeccBridgeTokenReleaseRequest)
  ONEVENT(DATATOKENRELEASEREQ,                      PARTYCHANGEMODE,    CH323Party::OnFeccBridgeTokenReleaseRequest)
  ONEVENT(DATATOKENRELEASEREQ,                      ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(DATATOKENRELEASE,                         PARTYCONNECTED,     CH323Party::OnFeccBridgeTokenRelease)
  ONEVENT(DATATOKENRELEASE,                         PARTYCHANGEMODE,    CH323Party::OnFeccBridgeTokenRelease)
  ONEVENT(DATATOKENRELEASE,                         ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(IPDATATOKENMSG,                           PARTYCONNECTED,     CH323Party::OnIpDataTokenMsg)
  ONEVENT(IPDATATOKENMSG,                           PARTYCHANGEMODE,    CH323Party::OnIpDataTokenMsg)
  ONEVENT(IPDATATOKENMSG,                           ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(IPFECCKEYMSG,                             PARTYCONNECTED,     CH323Party::OnIpFeccKeyMsg)
  ONEVENT(IPFECCKEYMSG,                             PARTYCHANGEMODE,    CH323Party::OnIpFeccKeyMsg)
  ONEVENT(IPFECCKEYMSG,                             ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(UPDATERSRCCONFID,                         PARTYIDLE,          CH323Party::OnPartyUpdateConfRsrcIdForInterface)
  ONEVENT(UPDATERSRCCONFID,                         PARTYCONNECTED,     CH323Party::OnPartyUpdateConfRsrcIdForInterface)
  ONEVENT(UPDATERSRCCONFID,                         PARTYCHANGEMODE,    CH323Party::OnPartyUpdateConfRsrcIdForInterface)
  ONEVENT(UPDATERSRCCONFID,                         PARTYSETUP,         CH323Party::OnPartyUpdateConfRsrcIdForInterface)

  ONEVENT(PARTY_FAULTY_RSRC,                        PARTYIDLE,          CH323Party::OnSendFaultyMfaToPartyCntl)
  ONEVENT(PARTY_FAULTY_RSRC,                        ANYCASE,            CH323Party::OnSendFaultyMfaToPartyCntl)

  ONEVENT(UPDATE_GK_CALL_ID,                        PARTYIDLE,          CH323Party::OnH323UpdateGkCallId)
  ONEVENT(UPDATE_GK_CALL_ID,                        ANYCASE,            CH323Party::OnH323UpdateGkCallId)

  ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION,     ANYCASE,            CH323Party::OnConfSetCapsAccordingToNewAllocation)
  ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,         PARTYCONNECTED,     CH323Party::OnRemoveAvcToSvcArtTranslatorAnycase)


  ONEVENT(UPDATE_PRESENTATION_OUT_STREAM,           ANYCASE,            CH323Party::OnPartyCntlUpdatePresentationOutStream)
  ONEVENT(PRESENTATION_OUT_STREAM_UPDATED,          ANYCASE,            CH323Party::OnPartyUpdatedPresentationOutStream)

  ONEVENT(EXPORT,                                   PARTYCHANGEMODE,    CH323Party::OnConfExportChangeMode)

  ONEVENT(SEND_INFO_TO_RSS,                         PARTYSETUP,         CH323Party::OnConfSendRssRequest)
  ONEVENT(SEND_INFO_TO_RSS,                         ANYCASE,            CH323Party::NullActionFunction)

  ONEVENT(UPDATE_FLOW_CONTROL_RATE,                 ANYCASE,            CH323Party::OnSendNewContentRateToConfLevel)
  ONEVENT(LPR_CHANGE_RATE,                          ANYCASE,            CH323Party::UpdatePartyH323LprVideoBitRate)
  ONEVENT(LPR_VIDEO_RATE_UPDATED,                   ANYCASE,            CH323Party::OnLprUpdatedPartyH323VideoBitRate)
  ONEVENT(LPR_PARTY_FLOWCONTROL,                    ANYCASE,            CH323Party::SendLprFlowControlToCard)
  ONEVENT(PARTY_CG_START_CONTENT,                   ANYCASE,            CH323Party::OnCGStartContent)
  ONEVENT(PARTY_CG_STOP_CONTENT,                    ANYCASE,            CH323Party::OnCGStopContent)

  ONEVENT(START_PREVIEW_PARTY,                      ANYCASE,            CH323Party::OnMcuMngrStartPartyPreviewReq)
  ONEVENT(STOP_PREVIEW_PARTY,                       ANYCASE,            CH323Party::OnMcuMngrStopPartyPreviewReq)
  ONEVENT(REQUEST_PREVIEW_INTRA,                    ANYCASE,            CH323Party::OnMcuMngrIntraPreviewReq)

  ONEVENT(LEADER_CHANGED,                           PARTYDISCONNECTING, CH323Party::NullActionFunction)
  ONEVENT(LEADER_CHANGED,                           ANYCASE,            CH323Party::OnSetPartyToLeader)

  ONEVENT(PARTY_REMOVE_SELF_FLOWCONTROL_CONSTRAINT, PARTYDISCONNECTING, CH323Party::NullActionFunction)
  ONEVENT(PARTY_REMOVE_SELF_FLOWCONTROL_CONSTRAINT, ANYCASE,            CH323Party::OnRemoveSelfFlowControlConstraint)

  ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYSETUP,         CH323Party::OnTimerContentSpeakerIntraRequest)
  ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYCHANGEMODE,    CH323Party::OnTimerContentSpeakerIntraRequest)
  ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYCONNECTED,     CH323Party::OnTimerContentSpeakerIntraRequest)
  ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYIDLE,          CH323Party::NullActionFunction)
  ONEVENT(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,  PARTYDISCONNECTING, CH323Party::NullActionFunction)
  ONEVENT(SEND_MUTE_VIDEO,                          ANYCASE,            CH323Party::OnH323SendVideoMute)

  ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,             ANYCASE,            CH323Party::OnMrmpRtcpFirInd)

  ONEVENT(PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED,	PARTYCHANGEMODE,	CH323Party::OnPartyUpgradeToMixChannelsUpdated)
//  ONEVENT(PARTY_UPGRADE_TO_MIX_CHANNELS_UPDATED,	ANYCASE,	CH323Party::OnPartyUpgradeToMixChannelsUpdated) // @#@ - this is safer
  ONEVENT(H323_TRANS_END_TRANSACTION,				ANYCASE,			CH323Party::OnTransEndTransactionAnycase)

  ONEVENT(TOKENRECAPCOLLISIONEND, PARTYDISCONNECTING,	CH323Party::NullActionFunction)
  ONEVENT(TOKENRECAPCOLLISIONEND, ANYCASE, 				CH323Party::OnTokenRecapCollisionEndAnycase)

  /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
  ONEVENT(VIDEORECOVERYBYE,				ANYCASE,			CH323Party::SendCallDropToPartyAnycase)
  /*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

PEND_MESSAGE_MAP(CH323Party,CParty);


	// Constructors
/////////////////////////////////////////////////////////////////////////////
CH323Party::CH323Party()
{
    m_AudioRate     = 0;
	m_videoRate		= 0;
	m_contentRate   = 0;
	m_initialTotalVideoRate = 0;

	m_isIpOnlyConf  = FALSE;
	m_confDualStreamMode = confTypeUnspecified;

	m_pH323Cntl		= new CH323Cntl(this);
	m_pH323NetSetup = new CH323NetSetup;

	m_serviceName   = NULL;
	m_contentState  = TokenIdle;
	m_changeModeState = eNotNeeded;
	m_changeContentModeState = eNotNeeded;

	m_pLocalCapH323 = new CCapH323;
	m_pRmtCapH323	= new CCapH323;
	m_pCurrentModeH323 = new CComModeH323;
	m_pInitialModeH323 = new CComModeH323;

	m_pH239TokenMsgMngr = new CTokenMsgMngr;

  	m_isECS = FALSE;
  	m_cgContentState   = eStreamOff;	// for Call Generator

  	m_ContentChangeModeRate = 0;
  	m_ContentChangeModeSpeaker = 0;

  	m_bIsPreviewVideoOut = FALSE;
  	m_bIsPreviewVideoIn	 = FALSE;

  	m_RcvPreviewReqParams = NULL;
  	m_TxPreviewReqParams=NULL;

  	m_bIsVideoCapEqualScm = FALSE;
  	m_pTerminalNumberingManager = NULL;

	m_num_content_intra_filtered = 0;
		
	m_linkType = eRegularParty;

	m_pDisconnectParams = NULL;
	m_eActiveTransactionType = kTransNone;
	m_pTransaction = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CH323Party::~CH323Party()
{
	POBJDELETE(m_pH323Cntl);
	POBJDELETE(m_pH323NetSetup);
	POBJDELETE(m_pLocalCapH323);
	POBJDELETE(m_pRmtCapH323);
	POBJDELETE(m_pCurrentModeH323);
	POBJDELETE(m_pInitialModeH323);
	PDELETEA(m_serviceName);
	POBJDELETE(m_pH239TokenMsgMngr);
	POBJDELETE(m_RcvPreviewReqParams);
	POBJDELETE(m_TxPreviewReqParams);
    POBJDELETE(m_pDisconnectParams);
	POBJDELETE(m_pTransaction);
}


/////////////////////////////////////////////////////////////////////////////
void CH323Party::Create(CSegment& appParam)
{
	CParty::Create(appParam);

	// for Call Generator, CG_SoftMCU
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	if ( eProductFamilyCallGenerator == CProcessBase::GetProcess()->GetProductFamily() ||
	      (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilySoftMcu && pCommConf && pCommConf->GetDisplayName() && strstr(pCommConf->GetDisplayName(), "##FORCE_CG")) ||
		(CProcessBase::GetProcess()->GetProductType() == eProductTypeCallGeneratorSoftMCU))
		m_isCallGeneratorParty = TRUE;
	else
		m_isCallGeneratorParty = FALSE;
}

////////////////////////////////////////////////////////////////////////////
void  CH323Party::InitTask()
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    DWORD partyTotalConnectionTimer = 0;
    pSysConfig->GetDWORDDataByKey("IP_PARTY_CONN_TIME_TILL_SIGNALING", partyTotalConnectionTimer);

    StartTimer(PARTYCONTOUT, partyTotalConnectionTimer*SECOND);
}
/////////////////////////////////////////////////////////////////////////////
const char* CH323Party::GetPartyStateAsString(int PartyStateNumber)
{
	if((m_state > PartyStateNumber) || (m_state == 0))
		return "Error";
	else
		return PartyStateStrings[m_state-1];
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::UpdateVideoRate(int newVidRate)
{
	if( newVidRate < 0 )
	{
		m_videoRate = 0;
		return;
	}

	m_videoRate = (DWORD)newVidRate;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::UpdateContentRate(int newContRate)
{
	DWORD newContentRate = newContRate;
	m_contentRate = ((newContRate >= 0) && (newContentRate <= m_videoRate))? newContentRate: m_contentRate;
}

/*
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::UpdateContTdmRate(DWORD newContTdmRate)
{
	m_contTdmRate = (((int)newContTdmRate >= 0)&&(newContTdmRate <= m_tdmRate))? newContTdmRate: m_contTdmRate;
}
*/
////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323LogicalChannelUpdate(CSegment* pParam)
{
	DWORD  vendorType;
	DWORD  channelType;

	*pParam >> vendorType >> channelType;
	 m_pH323NetSetup->DeSerialize(NATIVE,*pParam);

	OnH323LogicalChannelUpdateDB(channelType,vendorType);
}

////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323LogicalChannelUpdateDB(DWORD channelType, DWORD vendorType)
{
	CSegment* pSeg = new CSegment;

	//VNGFE-6008
	//*pSeg << PARTYNAME << vendorType << channelType;
	*pSeg << m_pParty->GetName() << vendorType << channelType;

	EIpChannelType channelTypeE = (EIpChannelType)channelType;

	if(channelTypeE == H225)// we send the NetSetup only in the connecting of H225 for CDR use!
	{
		if(vendorType == EricssonVIG)
		{// right now Ericsson only so I don't check this value
			int numOfSrcAlias;
			CH323Alias* PartySrcAliasList    = m_pH323NetSetup->GetSrcPartyAliasList(&numOfSrcAlias);
			if(numOfSrcAlias > 1)// more than one alias
			{
				CH323Alias* pToPartySrcAliasList = PartySrcAliasList;
				pToPartySrcAliasList++;

				ALLOCBUFFER(sPartyTA, IP_STRING_LEN);
				m_pH323NetSetup->GetH323srcPartyTA(sPartyTA);
				strcat(sPartyTA, ",");
				strcat(sPartyTA, pToPartySrcAliasList->GetAliasName());
				m_pH323NetSetup->SetSrcPartyAddress(sPartyTA);
				m_pH323NetSetup->SetH323PartyAlias(pToPartySrcAliasList->GetAliasName());
				m_pH323NetSetup->SetH323PartyAliasType(pToPartySrcAliasList->GetAliasType());
				DEALLOCBUFFER(sPartyTA);
			}
			PDELETEA(PartySrcAliasList);
		}
		m_pH323NetSetup->Serialize(NATIVE,*pSeg);
	}

	//VNGR-24921
	//m_pConfApi->UpdateDB(this,IPLOGICALCHANNELUPDATE,(DWORD) 0,1,pSeg);
	m_pConfApi->UpdateDB(NULL,IPLOGICALCHANNELUPDATE,(DWORD) 0,1,pSeg);

	POBJDELETE(pSeg);
}


void CH323Party::OnH323LogicalChannelConnect(CSegment* pParam)
{
    CSegment* pSeg = new CSegment;

    DWORD vendorType;
    DWORD channelType;
    *pParam >> vendorType >> channelType;

    EIpChannelType channelTypeE = (EIpChannelType)channelType;

    CPrtMontrBaseParams* pPrtMonitrParams =
          CPrtMontrBaseParams::AllocNewClass(channelTypeE);

    if (pPrtMonitrParams)
    	pPrtMonitrParams->DeSerialize(NATIVE, *pParam);

    *pSeg << vendorType << channelType;
    if (pPrtMonitrParams)
    {
        pPrtMonitrParams->Serialize(NATIVE, *pSeg);

        TRACEINTO << "Party " << PARTYNAME
            << ", type " << channelTypeE
            << ", object" << (pPrtMonitrParams? pPrtMonitrParams->NameOf() : " NULL")
            << ", m_state " << m_state;
    }
	
    if (VIDEO_CONT_IN == channelType && m_changeContentModeState == eChangeContentOut)
    {
        BOOL bContentOutChannelConnected = FALSE;
        CChannel* pOutChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);
        
        if (pOutChannel)
        {
            if (pOutChannel->GetCsChannelState() == kConnectedState)
            {
                bContentOutChannelConnected = TRUE;
            }
        }

        if (!bContentOutChannelConnected)
        {
            TRACESTRFUNC(eLevelWarn) << "Content out channel is not connected yet!!!! "
                "Remain in changemode state";
        }
        else
        {
            const CMediaModeH323& rcvCurContentMode =
                    m_pCurrentModeH323->GetMediaMode(cmCapVideo,
                                     cmCapReceive,
                                     kRoleContentOrPresentation);

            const CMediaModeH323& txCurContentMode =
                    m_pCurrentModeH323->GetMediaMode(cmCapVideo,
                                     cmCapTransmit,
                                     kRoleContentOrPresentation);

            if (rcvCurContentMode.GetType() == txCurContentMode.GetType())
            {
                TRACEINTO << "content types of rx and tx are the same - end changemode - "
                    << txCurContentMode.GetType();
                SendEndChangeVideoToConfLevel();
            }
            else
            {
                m_pCurrentModeH323->Dump("CH323Party::OnH323LogicalChannelConnect - content types "
                    "of rx and tx are different",
                    eLevelInfoNormal);
            }
        }
    }

    m_pConfApi->UpdateDB(this, IPLOGICALCHANNELCONNECT, (DWORD) 0, 1, pSeg);

	//BRIDGE-14845
	CComModeH323* pCurrentMode = m_pH323Cntl->GetCurrentMode();
	CComModeH323* pTargetMode = m_pH323Cntl->GetTargetMode();
	
    	if(VIDEO_CONT_IN == channelType)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelConnect check whether need to close content in channel. Name - ",PARTYNAME);

		CCommConf*  pCommConf  = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
		CConfParty* pConfParty = (pCommConf) ? pCommConf->GetCurrentParty(GetMonitorPartyId()) : NULL;
		
		if (pCommConf && pConfParty)
		{
			BYTE partySecondaryReason = pConfParty->GetSecondaryCause();
			TRACEINTO << "partySecondaryReason:" << (int)partySecondaryReason;
			if ((partySecondaryReason == SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD || partySecondaryReason == SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD)
				&& pCurrentMode && pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation) 
				&& pTargetMode && pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation))
			{
				pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

				m_pH323Cntl->UpdateLocalCapsFromTargetMode(m_bIsVideoCapEqualScm);
            			POBJDELETE(m_pLocalCapH323);
            			m_pLocalCapH323 = new CCapH323(*m_pH323Cntl->m_pLocalCapH323);
				StartReCapsProcess();
			}
		}
	}
	
    POBJDELETE(pSeg);
    POBJDELETE(pPrtMonitrParams);
}

void CH323Party::H323LogicalChannelDisConnect(DWORD channelType, WORD dataType, WORD bTransmitting, BYTE roleLabel, BYTE bUpdateCommMode)
{
//1: Update Monitoring:
	//1.1 Update channel monitoring:
	CSegment*  pSeg = new CSegment;
	*pSeg   << channelType;
	m_pConfApi->UpdateDB(this,IPLOGICALCHANNELDISCONNECT,(DWORD) 0,1,pSeg);
	POBJDELETE(pSeg);

	//1.2 Update communication mode monitoring:
	if (bUpdateCommMode)
	{
		CComModeH323* pCurComH323 = new CComModeH323;
		*pCurComH323 = *m_pCurrentModeH323;
		CSegment* pSeg = new CSegment;

		cmCapDirection direction = CalcCmCapDirection(bTransmitting);

		pCurComH323->SetMediaOff((cmCapDataType)dataType ,direction, (ERoleLabel)roleLabel);

		// Adding LPR capabilities for monitoring
		if (m_pLocalCapH323->IsLPR() && m_pRmtCapH323->IsLPR())
		{
			if (direction == cmCapReceive)
				pCurComH323->Serialize(*pSeg,direction,YES,m_pRmtCapH323);
			else
				pCurComH323->Serialize(*pSeg,direction,YES,m_pLocalCapH323);
		}
		else
		pCurComH323->Serialize(*pSeg,direction,YES);

		if (direction == cmCapReceive)
			m_pConfApi->UpdateDB(this,RMOT323COMMODE,(DWORD) 0,1,pSeg);

		else if (direction == cmCapTransmit)
			m_pConfApi->UpdateDB(this,LOCAL323COMMODE,(DWORD) 0,1,pSeg);

		POBJDELETE(pSeg);
		POBJDELETE(pCurComH323);
	}

//2: Update current mode:
	cmCapDirection direction = CalcCmCapDirection(bTransmitting);
	m_pCurrentModeH323->SetMediaOff((cmCapDataType)dataType ,direction, (ERoleLabel)roleLabel);

	// Remove video out rate limitation (flow control limitation) of this party
	if ((dataType == cmCapVideo) && (roleLabel == kRolePeople) && bTransmitting && m_pCurrentModeH323->GetFlowControlRateConstraint())
	{
		m_pCurrentModeH323->SetFlowControlRateConstraint(0);
		m_pH323Cntl->m_pCurrentModeH323->SetFlowControlRateConstraint(0);
		m_pConfApi->CleanBitRateLimitation(GetPartyId());
	}


}

//////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323LogicalChannelDisConnectChangeMode(CSegment* pParam)
{
	DWORD channelType;
	WORD dataType, roleLabel;
	BYTE bTransmitting;
	*pParam >> channelType >> dataType >> bTransmitting >> roleLabel;

	BYTE bUpdateCommMode = (channelType == VIDEO_CONT_IN) || (channelType == VIDEO_CONT_OUT);
	//in the other cases, updating the DB is the conf3ctl responsibility

	H323LogicalChannelDisConnect(channelType, dataType, bTransmitting, roleLabel, bUpdateCommMode);

	if(channelType == VIDEO_CONT_OUT)
	{
		if(m_changeContentModeState == eChangeContentOut)
		{
			PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelDisConnectChangeMode - content out. Name - ",PARTYNAME);

			m_pH323Cntl->UpdateLocalCapsFromTargetMode(m_bIsVideoCapEqualScm);
            POBJDELETE(m_pLocalCapH323);
            m_pLocalCapH323 = new CCapH323(*m_pH323Cntl->m_pLocalCapH323);
			StartReCapsProcess();
			m_pH323Cntl->OpenContentOutChannelFromMcms();
		}
	}
	else if( (channelType != VIDEO_IN) && (channelType != VIDEO_OUT) )
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelDisConnectChangeMode - not a video channel. Name - ",PARTYNAME);
		return;
	}
	else if (m_changeModeState == eNotNeeded)
		return; //this is change content mode process

	if (channelType == VIDEO_OUT)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelDisConnectChangeMode - video out. Name - ",PARTYNAME);
		switch (m_changeModeState)
		{
			case eReopenOut:
            case eFlowControlInAndReopenOut:
            case eChangeInAndReopenOut:
			case eReopenInAndOut:
			{
				//StartReCapsProcess();
				m_pH323Cntl->OpenVideoOutChannelFromMcms();
                ReActivateIncomingChannelIfNeeded();
				break;
			}
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
	}

	else if (channelType == VIDEO_IN)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelDisConnectChangeMode - video in. Name - ",PARTYNAME);
		switch (m_changeModeState)
		{
			case eReopenIn:
			case eReopenInAndOut:
			{//Do nothing: wait to the reconnection of the in channel by the endpoint
				break;
			}

			//Cases that the endpoint disconnected the incoming, instead of just change the
			//stream. So it's a new state now:
			case eChangeIncoming:
			{
				m_changeModeState = eReopenIn;
				//no we wait to the reconnection of the in channel by the endpoint
				break;
			}
			case eChangeInAndReopenOut:
			{
				m_changeModeState = eReopenInAndOut;
				//no we wait to the reconnection of the in channel by the endpoint
				break;
			}
			default:
				// Note: some enumeration value are not handled in switch. Add default to suppress warning.
				break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323LogicalChannelDisConnect(CSegment* pParam)
{
	BYTE bDisconnecting = FALSE;
	// In disconnecting process we don't change the party state and the communication mode
	if (m_pH323Cntl->m_pmcCall->GetIsClosingProcess())
		bDisconnecting = TRUE;

	DWORD channelType;
	WORD dataType, roleLabel;
	BYTE bTransmitting;
	*pParam >> channelType >> dataType >> bTransmitting >> roleLabel;

	BYTE bIsSecondary = FALSE;

	// if the two video channel disconnected (video in and out)
	// and the audio is connected change call status to secondary
	if( !bDisconnecting && (dataType == cmCapVideo) && (roleLabel == kRolePeople) )
	{
		WORD VideoInIndex  = m_pH323Cntl->GetChannelIndexInList(true, cmCapVideo,FALSE);
		WORD VideoOutIndex = m_pH323Cntl->GetChannelIndexInList(true, cmCapVideo,TRUE);
		WORD AudioInIndex  = m_pH323Cntl->GetChannelIndexInList(true, cmCapAudio,FALSE);
		WORD AudioOutIndex = m_pH323Cntl->GetChannelIndexInList(true, cmCapAudio,TRUE);
		if((AudioOutIndex < m_pH323Cntl->m_maxCallChannel) && (AudioInIndex < m_pH323Cntl->m_maxCallChannel) &&
		   (VideoOutIndex == m_pH323Cntl->m_maxCallChannel) && (VideoInIndex == m_pH323Cntl->m_maxCallChannel))
		{
			PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelDisConnect change party state to secondary in party level only. Name - ",PARTYNAME);
			bIsSecondary = TRUE;
		}
	}

	BYTE bUpdateCommMode = !bIsSecondary && !bDisconnecting;//in case of secondary, updating the DB is the conf3ctl responsibility

	H323LogicalChannelDisConnect(channelType, dataType, bTransmitting, roleLabel, bUpdateCommMode);

	//BRIDGE-14845
	CComModeH323* pCurrentMode = m_pH323Cntl->GetCurrentMode();
	CComModeH323* pTargetMode = m_pH323Cntl->GetTargetMode();

	if(channelType == VIDEO_CONT_OUT)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelDisConnect check whether need to close content in channel. Name - ",PARTYNAME);

		CCommConf*  pCommConf  = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
		CConfParty* pConfParty = (pCommConf) ? pCommConf->GetCurrentParty(GetMonitorPartyId()) : NULL;
		
		if (pCommConf && pConfParty)
		{
			BYTE partySecondaryReason = pConfParty->GetSecondaryCause();
			TRACEINTO << "partySecondaryReason:" << (int)partySecondaryReason;
			if ((partySecondaryReason == SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD || partySecondaryReason == SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD)
				&& pCurrentMode && pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation) 
				&& pTargetMode && pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation))
			{
				pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

				m_pH323Cntl->UpdateLocalCapsFromTargetMode(m_bIsVideoCapEqualScm);
            			POBJDELETE(m_pLocalCapH323);
            			m_pLocalCapH323 = new CCapH323(*m_pH323Cntl->m_pLocalCapH323);
				StartReCapsProcess();
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323DisconectChannelDisconnecting(CSegment* pParam)
{
	WORD dataType;
	WORD direction;
	WORD roleLabel;

	*pParam >> dataType >> direction >> roleLabel;
}
////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323Party::UpdateConfMainLinkIsConnected shiraITP - 18
void CH323Party::UpdateConfMainLinkIsConnected()
{
	PASSERTSTREAM_AND_RETURN(!m_pConfApi, "PartyId:" << GetPartyId());

	TRACEINTO << "PartyId:" << GetPartyId();
	m_pConfApi->AddSubLinksAfterMainConnected(GetPartyId());     // shiraITP - 19
}

//////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::SendCloseChannelToConfLevel(CSegment* pParam)
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	WORD dataType;
	WORD direction;
	WORD roleLabel;

	*pParam >> dataType >> direction >> roleLabel;
	m_pConfApi->SendCloseChannelToConfLevel(GetPartyId(), dataType, direction, roleLabel);
}

////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323GateKeeperStatus(CSegment* pParam)
{
	CSegment* pSeg = new CSegment;

	BYTE gkState, gkRouted;
	DWORD reqBandwidth, allocBandwidth;
	WORD requestInfoInterval;

	*pParam >> gkState
			>> reqBandwidth
			>> allocBandwidth
			>> requestInfoInterval
			>> gkRouted;

	*pSeg  << gkState
		   << reqBandwidth
		   << allocBandwidth
		   << requestInfoInterval
		   << gkRouted;

	m_pConfApi->UpdateDB(this, H323GATEKEEPERSTATUS,(DWORD)0, 1, pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnH323PartyMonitoring(CSegment* pParam)
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
void CH323Party::OnH323Rmt323ComModUpdateDB(CSegment *pParam)
{
	CComModeH323* pCurrentMode = new CComModeH323;
	pCurrentMode->DeSerialize(NATIVE,*pParam);

	//Update content channel to confLevel - in the operator.
	CSegment      rmtCurComSeg;
	CComModeH323 *pRmtCurComH323 = new CComModeH323;

	*pRmtCurComH323 = *pCurrentMode;
	// Adding LPR capabilities for monitoring
	if (m_pLocalCapH323->IsLPR() && m_pRmtCapH323->IsLPR())
	{
		pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES,m_pRmtCapH323);
	}
	else
		pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES);

	pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES);
	m_pConfApi->UpdateDB(this,RMOT323COMMODE,(DWORD) 0,1,&rmtCurComSeg);

	POBJDELETE(pRmtCurComH323);
	POBJDELETE(pCurrentMode);
}


/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnConfEstablishH323Call(CSegment* pParam, WORD &encAlg, WORD &halfKeyAlg,
										  CH323NetSetup* pH323NetSetup,CRsrcParams** avcToSvcTranslatorRsrcParams)
{

	BYTE bIsTranslatorExists = FALSE;

    *pParam >> m_PartyRsrcID;
	*pParam >> m_cascadeMode;
	*pParam >> m_nodeType;
	*pParam >> m_videoRate; // 100 bits/sec unit.
	*pParam >> encAlg;
	*pParam >> halfKeyAlg;
	*pParam >> m_mcuNum;
	*pParam >> m_termNum;
	*pParam >> m_bNoVideRsrcForVideoParty;
	*pParam >> (DWORD&)m_pTerminalNumberingManager;

	TRACESTR (eLevelInfoNormal) << " CH323Party::OnConfEstablishH323Call terminalNum= " << m_termNum <<
	  ", mcuNumber= " << m_mcuNum << ", partyRsrcId= " << m_PartyRsrcID 
	  << ", bNoVideRsrcForVideoParty= " << (WORD)m_bNoVideRsrcForVideoParty;

	if (m_ivrCtrl)
		m_ivrCtrl->setNoVideRsrcForVideoParty(m_bNoVideRsrcForVideoParty);

	pH323NetSetup->DeSerialize(NATIVE,*pParam);
	m_pInitialModeH323->DeSerialize(NATIVE,*pParam);
	m_pLocalCapH323->DeSerialize(NATIVE,*pParam);
	DWORD totalVideoRate = m_pInitialModeH323->GetTotalVideoRate();
	m_pCurrentModeH323->SetTotalVideoRate(totalVideoRate);
	//PTRACE2INT(eLevelInfoNormal,"CH323Party::OnConfEstablishH323Call :  Party total video rate - ",totalVideoRate);
	m_pH323Cntl->SetTotalVideoRate(totalVideoRate);

	if(m_cascadeParty)
	{// if its the auto cascade feature, remove the G7231 and G729a audio algorithms to enable inband DTMF (bug#19941)
		PTRACE2(eLevelInfoNormal,"CH323Party::OnConfEstablishH323Call, remove G7231 and G729 to enable cascade DTMF. : Name - ",PARTYNAME);
		m_pLocalCapH323->RemoveProtocolFromCapSet(_G7231);
		m_pLocalCapH323->RemoveProtocolFromCapSet(_G729);
	}
	m_pH239TokenMsgMngr->DisableTokenMsgMngr();
//#ifdef __VCU_TOUT__
//	WORD  isVidTran = 0;
//	*pParam >> isVidTran;
//	m_isTranscoded = isVidTran;
//	if ( isVidTran )
//	{
//		m_pVidMcmsDesc->DeSerialize(NATIVE,*pParam);
		//vcu req filter values for transcoding or C.P
		m_rcvVcuCounterThreshHold = 2;
		m_rcvVcuCounterWindowTout = 15;
/*	}
	else
	{
		//vcu req filter values for video switching
		m_rcvVcuCounterThreshHold = 3;
		m_rcvVcuCounterWindowTout = 5;
	}
#endif*/

	m_pH323Cntl->SetConfType(m_pInitialModeH323->GetConfType());
	// update party id and streams too
    m_pH323Cntl->GetCurrentMode()->SetPartyId(m_pInitialModeH323->GetPartyId());
    m_pH323Cntl->GetCurrentMode()->SetConfMediaType(m_pInitialModeH323->GetConfMediaType());
//    m_pH323Cntl->GetCurrentMode()->SetStreamsListForMediaMode(m_pInitialModeH323->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople), cmCapVideo, cmCapReceive, kRolePeople);
    m_pH323Cntl->GetCurrentMode()->Dump("mix_mode: CH323Party::OnConfEstablishH323Call H323Control current SCM" , eLevelInfoNormal);

	UpdateInitialContentModeIfNeeded();

	CQoS* pQos = new CQoS;
	pQos->DeSerialize(NATIVE,*pParam);
    m_pH323Cntl->SetQualityOfService(*pQos);
	POBJDELETE(pQos);

    CRsrcParams* pRtpRsrcParams = new CRsrcParams;
	pRtpRsrcParams->DeSerialize(NATIVE,*pParam);
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	pCsRsrcParams->DeSerialize(NATIVE,*pParam);
	m_ConfRsrcId = pCsRsrcParams->GetConfRsrcId();

    CRsrcParams* pMrmpRsrcParams = NULL;
	DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams);
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i]);
		}

	UdpAddresses sUdpAddressesParams;
	pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));
    m_pH323Cntl->SetControllerResource(pRtpRsrcParams, pCsRsrcParams, sUdpAddressesParams);
    m_pH323Cntl->SetInternalControllerResource(avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);
	POBJDELETE(pRtpRsrcParams);
    POBJDELETE(pMrmpRsrcParams);
	POBJDELETE(pCsRsrcParams);

	BYTE bIsCopVideoTxModes = FALSE;
	*pParam >> bIsCopVideoTxModes;
	if (bIsCopVideoTxModes)
	{
		CCopVideoTxModes* pTempCopVideoTxModes = new CCopVideoTxModes;
		pTempCopVideoTxModes->DeSerialize(NATIVE,*pParam);
		m_pH323Cntl->SetCopVideoTxModes(pTempCopVideoTxModes);
		POBJDELETE(pTempCopVideoTxModes);
	}

	*pParam >> m_RoomId;
	 *pParam >> (WORD&)m_linkType;
	m_pH323Cntl->SetTipRoomId(m_RoomId);

    if (m_pInitialModeH323->GetConfMediaType() == eMixAvcSvc)
    {
        TRACEINTO << "mix_mode: bIsActiveAvcToSvc=TRUE";
        m_SsrcIdsForAvcParty.DeSerialize(*pParam);
    }

	//set conf type:
	if (m_pLocalCapH323->IsH239())
		m_confDualStreamMode = confTypeH239;

    CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());

    if (!pCommConf)
    {
        PTRACE(eLevelInfoNormal,"CH323Party::OnConfEstablishH323Call - pCommConf is NULL - can not check encryption");
        return;
    }

    CConfParty* pConfParty = pCommConf->GetCurrentParty(GetMonitorPartyId());
    // if initial encryption mode has been set to "off", reset the enc. params
	BOOL IsNonEncryParty = m_pInitialModeH323->GetIsEncrypted() == Encryp_Off ;
    BOOL DisconnectIfEncryptionFails = m_pInitialModeH323->GetIsDisconnectOnEncryptionFailure();
	if (IsNonEncryParty)
    {
		PTRACE (eLevelInfoNormal,"CH323Party::OnConfEstablishH323Call - non encrypted party");
        encAlg = kUnKnownMediaType;
        halfKeyAlg = kHalfKeyUnKnownType;
    }

    else
    {
    	const char* pTmp = ( !pConfParty ) ? "pConfParty is NULL!!" : (pConfParty->GetIsEncrypted()? "yes":"no");

        CSmallString str;
        str << "CH323Party::OnConfEstablishH323Call - conf encrypted = " << pCommConf->GetIsEncryption()
            << " Party encrypted = " << pTmp
            << " DisconnectIfEncryptionFails = " << DisconnectIfEncryptionFails;

        PTRACE (eLevelInfoNormal, str.GetString());

    }


}


///////////////////////////////////////////////////////////////////////////////////
//In the conf level, the content starts with rate 0. But in the party level we need the rate.
//(for example, in case we need to send re_caps, which are sent from the target mode).
void  CH323Party::UpdateInitialContentModeIfNeeded()
{
	if (m_pInitialModeH323->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation))
	{
		DWORD contentRate = m_pLocalCapH323->GetMaxContRate();
		m_pInitialModeH323->SetVideoBitRate(contentRate, cmCapReceive, kRoleContentOrPresentation);
		m_pInitialModeH323->SetVideoBitRate(contentRate, cmCapTransmit, kRoleContentOrPresentation);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnH323EndChannelConnectSetupOrConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"***CH323Party::OnH323EndChannelConnectSetupOrConnect : Name - ",PARTYNAME);
	m_pInitialModeH323->Dump("mix_mode: CH323Party::OnH323EndChannelConnectSetupOrConnect initial ", eLevelInfoNormal);
	m_pCurrentModeH323->Dump("mix_mode: CH323Party::OnH323EndChannelConnectSetupOrConnect current ", eLevelInfoNormal);

	BYTE bLateReleaseResourcesInConfCntl = FALSE;

    BYTE bAudioIsDisconnectedInOldMode = (m_pCurrentModeH323->IsMediaOff(cmCapAudio,cmCapTransmit) ||  m_pCurrentModeH323->IsMediaOff(cmCapAudio,cmCapReceive)); //The Audio state in the old mode
	BYTE bOnlyAudioConnected = H323EndChannelConnect(pParam, &bLateReleaseResourcesInConfCntl);

	// VNGFE-787
	BYTE isCodianVcr = 0;
	*pParam >> isCodianVcr;
	BYTE cascadeType = CASCADE_NONE;
	if((m_pH323Cntl->m_pmcCall->GetRmtType() == cmEndpointTypeMCU ||
			(m_pH323Cntl->GetRemoteIdent() == PolycomRMX || (m_pH323Cntl->GetRemoteIdent() == PolycomMGC && m_pH323Cntl->m_pmcCall->GetRmtType() != cmEndpointTypeGateway))) && !IsCallGeneratorParty())
		//We set the cascade type via the 'sourceInfo' only from remote messages

    {
        PTRACE(eLevelInfoNormal, "CH323Party::OnH323EndChannelConnectSetupOrConnect - Cascade!");
		cascadeType = CASCADE_MCU;
    }
    else if (m_pH323Cntl->m_pmcCall->GetRmtType() == cmEndpointTypeGateway &&
    		(m_pH323Cntl->GetRemoteIdent() == PolycomRMX || m_pH323Cntl->GetRemoteIdent() == PolycomMGC))
    {
        PTRACE(eLevelInfoNormal, "CH323Party::OnH323EndChannelConnectSetupOrConnect - CASCADE_GW!");
        cascadeType = CASCADE_GW;
    }

    int  MSDStatus =  CASCADE_MODE_MASTER;
    if (m_pH323Cntl->m_pmcCall->GetMasterSlaveStatus() == cmMSSlave)
        MSDStatus =  CASCADE_MODE_SLAVE;


	if ( (m_status != statOK) && (m_status != statSecondary) && (m_status != statVideoBeforeAudio) )
	{
		PTRACE2(eLevelInfoNormal,"mix_mode: CH323Party::OnH323EndChannelConnectSetupOrConnect : FAILED TO ESTABLISH CALL Name - ",PARTYNAME);
		m_pConfApi->H323PartyConnect(GetPartyId(), m_status, cascadeType, MSDStatus, m_pH323Cntl->GetMrmpChannelHandle(cmCapVideo)); // @#@ support audio!!!
		return; // failed to establish connection
	}

	CCapH323 *pTmpRmtCaps = new CCapH323;
	*pTmpRmtCaps = *m_pRmtCapH323;
	// Additional conditioning added because bug 19716.
	// 19716 - In case of 3G environment it was decided that NetMeeting will work Qcif symmetrically due to 2 problems
	// 1. NM can't except 2 resolutions on the same outgoing channel (And due to the way we're building the H221 caps
	// All H263 resolutions are under the same SCM). NM declares each resolution in a differnt set.
	// 2. The NM opens a Qcif channel toward the MCU anyway.
	DWORD mobilePhoneRate = GetSystemCfgFlagInt<DWORD>(CFG_KEY_IP_MOBILE_PHONE_RATE);

	if ((m_pCurrentModeH323->GetConfType() == kVideoSwitch) ||
	 ((m_pCurrentModeH323->GetConfType() == kCp) && /*IsPartyInConfOnPort() &&*/ (mobilePhoneRate != 0) && (m_pH323Cntl->GetRemoteIdent() == NetMeeting)))
		m_pH323Cntl->UpdateRemoteCapsAccordingToRemoteType(*pTmpRmtCaps);//temp!!

	if (m_status == statSecondary)  //request for audio came first
	{
        m_status = statSecondary;
        PTRACE2(eLevelInfoNormal,"mix_mode: CH323Party::OnH323EndChannelConnectSetupOrConnect : Status secondary ",PARTYNAME);
        m_pCurrentModeH323->Dump("mix_mode: CH323Party::OnH323EndChannelConnectSetupOrConnect current ", eLevelInfoNormal);
		m_pConfApi->H323PartyConnect(GetPartyId(), statSecondary,cascadeType, MSDStatus, m_pH323Cntl->GetMrmpChannelHandle(cmCapVideo),  pTmpRmtCaps, m_pCurrentModeH323, bLateReleaseResourcesInConfCntl);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"mix_mode: CH323Party::OnH323EndChannelConnectSetupOrConnect : \'PARTY CONNECTED !!!\' - ",PARTYNAME);
        if (bAudioIsDisconnectedInOldMode || bOnlyAudioConnected)
            m_pConfApi->H323PartyConnect(GetPartyId(), m_status,cascadeType,MSDStatus, m_pH323Cntl->GetMrmpChannelHandle(cmCapVideo),  pTmpRmtCaps, m_pCurrentModeH323,  bLateReleaseResourcesInConfCntl);
        if (!bOnlyAudioConnected)
        {
            BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_H323_FREE_VIDEO_RESOURCES);

            if (m_pH323Cntl->IsCallConnectedAudioOnly() && bEnableFreeVideoResources && IsUndefinedParty() && !bLateReleaseResourcesInConfCntl)
                m_pH323Cntl->SetLocalCapToAudioOnly();
            m_pConfApi->H323PartyConnectAll(GetPartyId(), m_pCurrentModeH323, m_videoRate, pTmpRmtCaps, isCodianVcr, m_pH323Cntl->GetMrmpChannelHandle(cmCapVideo));

            PTRACE2INT(eLevelInfoNormal,"CH323Party::OnH323EndChannelConnectSetupOrConnect : m_videoRate - ",m_videoRate);
            m_pCurrentModeH323->SetTotalVideoRate(m_videoRate);
            m_pInitialModeH323->SetTotalVideoRate(m_videoRate);
            m_pH323Cntl->SetTotalVideoRate(m_videoRate);
	// VNGFE-7077, update BaudRate in EMA/GUI
	H323UpdateBaudRate();
        }

	}
	if (!m_isECS)
	{
        StartIvr();		// starting the IVR part
        m_state = PARTYCONNECTED;
		if(m_pH323Cntl->IsCallConnectedAudioOnly() == FALSE)// in case of audio only no need to start the VCU timer (which is only monitoring issue)
			StartTimer(VCUTOUT,10*SECOND);
    }
	POBJDELETE(pTmpRmtCaps);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnH323EndChannelConnectChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"***CH323Party::OnH323EndChannelConnectChangeMode : \'PARTY END CHANGED MODE !!!\' - ",PARTYNAME);

	BYTE bLateReleaseResourcesInConfCntl = FALSE;
	BYTE bOnlyAudioConnected = H323EndChannelConnect(pParam, &bLateReleaseResourcesInConfCntl);

    CCapH323 *pTmpRmtCaps = new CCapH323;
    *pTmpRmtCaps = *m_pRmtCapH323;

    BYTE cascadeType = CASCADE_NONE;
	if((m_pH323Cntl->m_pmcCall->GetRmtType() == cmEndpointTypeMCU ||
				(m_pH323Cntl->GetRemoteIdent() == PolycomRMX || (m_pH323Cntl->GetRemoteIdent() == PolycomMGC && m_pH323Cntl->m_pmcCall->GetRmtType() != cmEndpointTypeGateway))) && !IsCallGeneratorParty())
		//We set the cascade type via the 'sourceInfo' only from remote messages

    {
        PTRACE(eLevelInfoNormal, "CH323Party::OnH323EndChannelConnectChangeMode - Cascade!");
		cascadeType = CASCADE_MCU;
    }
    else if (m_pH323Cntl->m_pmcCall->GetRmtType() == cmEndpointTypeGateway &&
            (m_pH323Cntl->GetRemoteIdent() == PolycomRMX || m_pH323Cntl->GetRemoteIdent() == PolycomMGC))
    {
        PTRACE(eLevelInfoNormal, "CH323Party::OnH323EndChannelConnectChangeMode - CASCADE_GW!");
        cascadeType = CASCADE_GW;
    }

    int MSDStatus = m_pH323Cntl->m_pmcCall->GetMasterSlaveStatus();
//    m_pCurrentModeH323->Dump("CH323Party::OnH323EndChannelConnectChangeMode m_pCurrentModeH323", eLevelInfoNormal);
    m_pConfApi->H323PartyConnect(GetPartyId(), m_status,cascadeType,MSDStatus, m_pH323Cntl->GetMrmpChannelHandle(cmCapVideo),  pTmpRmtCaps, m_pCurrentModeH323, bLateReleaseResourcesInConfCntl);
    StartIvr();

	if (!bOnlyAudioConnected)
	{
		/*if ((m_changeModeState == eChangeInAndReopenOut) || (m_changeModeState == eChangeIncoming))
		{
		    TRACESTR(eLevelInfoNormal) <<" CH323Party::OnH323EndChannelConnectChangeMode: Need to wait for card indication. m_changeModeState = "
								   << GetChangeModeStateStr(m_changeModeState) << ",  Name - " << PARTYNAME;
			m_changeModeState = eChangeIncoming;
			return; //wait for the card indication (stream status / reopen incoming)
		}*/

		// check if its content out connected indication (current is on and memeber is in change content out state)
		if((m_changeContentModeState == eChangeContentOut) &&
		 		m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation))
		{

			PTRACE(eLevelInfoNormal,"CH323Party::OnH323EndChannelConnectChangeMode - content out reopend");
			// Before sending End Change Mode to Conf, we call ContentRateChange in order to complete the change content
			// with the saved values that were received from P.C in the change content mode request.
			ContentRateChange(m_ContentChangeModeRate, m_ContentChangeModeSpeaker);

//			CSegment* pSeg = new CSegment;
//			*pSeg << (DWORD)statOK;
//			m_pCurrentModeH323->Serialize(NATIVE,*pSeg);
//			SendEndChangeContentToConfLevel(pSeg);
//			POBJDELETE(pSeg);
		}
		else if((m_changeContentModeState == eChangeContentOut) &&
				m_pInitialModeH323->IsMediaOn(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation) &&
				m_pCurrentModeH323->IsMediaOff(cmCapVideo, cmCapTransmit,kRoleContentOrPresentation) &&
				m_pH323Cntl->IsWaitForContentOutToBeOpen() &&
		 		m_pH323Cntl->IsContentRejected())
		{
			// We arrive to here when we are in change content out, and the the content-In channel was rejected by us, but the content-Out wasn't rejected. So we should wait for complete opening the Out channel in order to complete the change content out.
			PTRACE(eLevelInfoNormal,"CH323Party::OnH323EndChannelConnectChangeMode - We rejected Content In channel. We still wait for open content out");
		}
		else
		SendEndChangeVideoToConfLevel();
	}
    POBJDELETE(pTmpRmtCaps);

//     else
//     {
//         //Connect Only the Audio - change mode is not over yet
//         CCapH323 *pTmpRmtCaps = new CCapH323;
//         *pTmpRmtCaps = *m_pRmtCapH323;
//         BYTE bIsCascade = FALSE;
//         if(m_pH323Cntl->m_pmcCall->GetRmtType() == cmEndpointTypeMCU)
//             bIsCascade = TRUE;
//         m_pConfApi->H323PartyConnect(this, m_status,bIsCascade, pTmpRmtCaps, m_pCurrentModeH323, bReleaseResourcesInConfCntl);
//     }

}

/////////////////////////////////////////////////////////////////////////////
BYTE CH323Party::H323EndChannelConnect(CSegment* pParam, BYTE* pIsLateReleaseResourcesInConfCntl)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::H323EndChannelConnect : Name  - ",PARTYNAME);
	m_pRmtCapH323->DeSerialize(NATIVE,*pParam);
	m_pCurrentModeH323->DeSerialize(NATIVE,*pParam);

	*pParam >> m_status;
	*pParam >> *pIsLateReleaseResourcesInConfCntl;
	BYTE bOnlyAudioConnected = FALSE;
	*pParam >> bOnlyAudioConnected;

	CSegment rmtCapSeg;
	m_pRmtCapH323->SerializeCapArrayOnly(rmtCapSeg,TRUE);
	m_pConfApi->UpdateDB(this,RMOT323CAP,(DWORD) 0,1,&rmtCapSeg);

	if (m_contentRate == 0)
		m_pCurrentModeH323->SetVideoBitRate(0, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

	// Updates the video in channel.
	CSegment      rmtCurComSeg;
	CComModeH323 *pRmtCurComH323 = new CComModeH323;

	// In case of video in channel frame rate or resolution is differ from
	// video out channel, we get the real values from the segment and update
	// correspondingly.
	*pRmtCurComH323 = *m_pCurrentModeH323;
	// Adding LPR capabilities for monitoring
	if (m_pLocalCapH323->IsLPR() && m_pRmtCapH323->IsLPR())
	{
			pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES,m_pRmtCapH323);
	}
	else
		pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES);

//	pRmtCurComH323->Serialize(rmtCurComSeg,cmCapReceive,YES);
	m_pConfApi->UpdateDB(this,RMOT323COMMODE,(DWORD) 0,1,&rmtCurComSeg);

	POBJDELETE(pRmtCurComH323);

	CSegment localComSeg;
	CComModeH323* pLocalScmH323 = new CComModeH323;
	*pLocalScmH323 = *m_pCurrentModeH323;

	if ((m_confDualStreamMode == confTypeDuoVideo) && (m_contentRate == 0) )
		pLocalScmH323->SetMediaOff(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation);

	// Adding LPR capabilities for monitoring
	if (m_pLocalCapH323->IsLPR() && m_pRmtCapH323->IsLPR())
	{
		pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES,m_pLocalCapH323);
	}
	else
		pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES);

	pLocalScmH323->Serialize(localComSeg,cmCapTransmit,YES);

	m_pConfApi->UpdateDB(this,LOCAL323COMMODE,(DWORD) 0,1,&localComSeg);
	H323UpdateBaudRate();
	POBJDELETE(pLocalScmH323);

	return bOnlyAudioConnected;
}


/*
/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323EndContentReConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323EndContentReConnect: Name  - ",PARTYNAME);

	CVidModeH323* pTempContentReceiveMode = new CVidModeH323;
	pTempContentReceiveMode->DeSerialize(NATIVE,*pParam);

	m_pConfApi->H323ReconnectContent(this, *pTempContentReceiveMode);

	POBJDELETE(pTempContentReceiveMode);
}
*/
/////////////////////////////////////////////////////////////////////////////
void   CH323Party::H323UpdateBaudRate()
{
	DWORD totalRate = 0;
	CSegment* pSeg1 = new CSegment;
	CSegment* pSeg2 = new CSegment;
	DWORD MaxCallRate = m_pCurrentModeH323->GetTotalVideoRate();
	if (m_pH323Cntl && m_pH323Cntl->m_pH323NetSetup)
		MaxCallRate = m_pH323Cntl->m_pH323NetSetup->GetMaxRate();
	TRACEINTO << "m_pH323Cntl->m_pH323NetSetup->GetMaxRate(): " << MaxCallRate;
	totalRate =  min(MaxCallRate, m_pCurrentModeH323->GetTotalBitRate(cmCapReceive));
	*pSeg1 << totalRate;
	m_pConfApi->UpdateDB(this,RECEIVEBAUDRATE,(DWORD) 0,1,pSeg1);
	totalRate =  min(MaxCallRate, m_pCurrentModeH323->GetTotalBitRate(cmCapTransmit));
	*pSeg2 << totalRate;
	m_pConfApi->UpdateDB(this,TRANSMITBAUDRATE,(DWORD) 0,1,pSeg2);
	POBJDELETE(pSeg1);
	POBJDELETE(pSeg2);
}
/*

/////////////////////////////////////////////////////////////////////////////
void   CH323Party::OnH323H230SetUp(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323H230SetUp : Name - ",PARTYNAME);
	OnH323H230(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void   CH323Party::OnH323H230ChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323H230ChangeMode : Name - ",PARTYNAME);
	OnH323H230(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void   CH323Party::OnH323H230Connect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323H230Connect : Name - ",PARTYNAME);
	OnH323H230(pParam);
}
*/

/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnPartyRemoteH230(CSegment* pParam)
{
	// PTRACE2(eLevelInfoNormal,"CH323Party::OnPartyRemoteH230 : Name - ",PARTYNAME);
	WORD opcode          = 0;
	WORD rVideoSyncLost  = 0;
	BYTE bIsGradualIntra = FALSE;
	ERoleLabel eRole     = kRolePeople;
	*pParam >> opcode;
	switch (opcode)
	{
		case Fast_Update:
		{
			WORD tempRole, tempRtpReport;
			*pParam >> tempRole;
			*pParam >> tempRtpReport;
			*pParam >> bIsGradualIntra;
			eRole = (ERoleLabel)tempRole;
			if (eRole & kRoleContentOrPresentation)
			{
				if (tempRtpReport == 3)
				{
					ContentIntraRequestFiltering();
				}
				else if (!m_isCallGeneratorParty)
				{
					// atara: I send it even though the content bridge doesn't use it.
					BYTE controlId = 1;
					m_pConfApi->ContentVideoRefresh(controlId, YES, this);
				}
				else // in case this is a Call Generator system
				{
					PTRACE2(eLevelInfoNormal, "CH323Party::OnPartyRemoteH230 -content cg need to send intra : Name - ", PARTYNAME);
				}
			}
			// if party video connected in self loop, and not connected to the video bridge
			// an intra request from the party will be send to itself.
			else // (eRole == kRolePeople)
			{
				if (tempRtpReport == eReceiveIntra)
				{
					PeopleIntraRequestFiltering();
				}
				else
				{
					if (m_pH323Cntl->GetCallParams()->GetIsClosingProcess() != TRUE) // the call is not in disconnecting process we can send fast update.
					{
						PeopleIntraRequestFiltering(tempRtpReport);
					}

					if (rVideoSyncLost == 1)
						m_rcvVcuCounter++;
				}
			}
			break;
		}

		case AIM:
		{
			// should send mute to audio codec when AIM received according to SYSTEM.CFG flag
			CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			BOOL isIgnoreMuteOnAim = NO;
			std::string key = "IGNORE_AIM";
			pSysConfig->GetBOOLDataByKey(key, isIgnoreMuteOnAim);

			if (!isIgnoreMuteOnAim)
				m_pConfApi->AudioMute(GetPartyId(), eOn);
			else                                                          // vngr -9077
				//m_pConfApi->UpdateDB(this, MUTE_STATE, eOn |0xF0000000, 1); // only update the icon.
				UpdateMuteIconState(eOn);
			break;
		}

		case AIA:
		{
			m_pConfApi->AudioMute(GetPartyId(), eOff);
			break;
		}

		default:
		{
			break;
		}
	} // switch
}


/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnH323VideoMuteSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323VideoMuteSetup : Name - ",PARTYNAME);
	H323VideoMute(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnH323VideoMuteConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323VideoMuteConnect : Name - ",PARTYNAME);
	H323VideoMute(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnH323VideoMuteChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323VideoMuteChangeMode : Name - ",PARTYNAME);
	H323VideoMute(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::H323VideoMute(CSegment* pParam)
{
	WORD onOff;
	*pParam >> onOff;
	m_pConfApi->VideoMute(GetPartyId(), onOff);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::IncreaseDisconnctingTimerInPartyCntl()
{
	m_pConfApi->PartyIncreaseDisconnctingTimer(GetPartyId());
}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::SendSiteAndVisualNamePlusProductIdToPartyControl(CSegment* pParam)
{
	BYTE  isCopMcu;
	BYTE  isSiteName;
	DWORD lenSiteName;
	BYTE  isProductId;
	DWORD lenProductId;
	BYTE  isVersionId;
	DWORD lenVersionId;

	*pParam
		>> isCopMcu
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

		TRACEINTO << "strSiteName:" << DUMPSTR(strSiteName) << ", strProductId:" << DUMPSTR(strProductId) << ", strVersionId:" << DUMPSTR(strVersionId) << ", DialInName:" << m_name << ", TelePresencePartyType:" << telePresencePartyType;

		std::string siteNameTemp = strSiteName ? strSiteName : "";

		CCommConf*  pCommConf  = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
		CConfParty* pConfParty = (pCommConf) ? pCommConf->GetCurrentParty(GetMonitorPartyId()) : NULL;

		if (pCommConf && pConfParty && (pConfParty->GetConnectionType() == DIAL_IN) && (pConfParty->GetPartyType() != eRegularParty))
		{
			char findMainPartyNameOfThisSubLink[H243_NAME_LEN]; // we don't need this info
			BOOL isMainLinkDefined = pCommConf->GetMainLinkNameAccordingToMainPartiesCounterAndReturnIsMainLinkDefined(pConfParty->GetMainPartyNumber(), (char*)findMainPartyNameOfThisSubLink);

			TRACEINTO << "IsMainLinkDefined:" << (int)isMainLinkDefined << ", MainPartyNumber:" << pConfParty->GetMainPartyNumber();

			if (IsUndefinedParty() && isMainLinkDefined == FALSE)
			{
				// for main undefined and their subs:
				char* partyIndex = strrchr(m_name, '(');
				if (partyIndex)
				{
					siteNameTemp += "_";
					siteNameTemp += partyIndex;
				}
				else
				{
					partyIndex = strrchr(m_name, '_');
					if (partyIndex)
					{
						siteNameTemp += partyIndex;
					}
					else
					{
						siteNameTemp = m_name;
					}
				}
			}
			else
			{
				// for main Link defined dial in and their sublinks:
				siteNameTemp = m_name;
			}
		}
		else
		{
			// for all links dial out we do not change the site name
		}

		char siteName[MAX_SITE_NAME_ARR_SIZE];
		siteName[0] = '\0';
		strcpy_safe(siteName, siteNameTemp.c_str());

		lenSiteName = (siteName[0] != '\0') ? strlen(siteName)+1 : 0;

		TRACEINTO << "siteNameTemp:" << siteNameTemp.c_str() << ", SiteName:" << siteName;

		m_pConfApi->SendSiteAndVisualNamePlusProductIdToPartyControl(this, isSiteName, lenSiteName, siteName, isProductId, lenProductId, strProductId, isVersionId, lenVersionId, strVersionId, telePresencePartyType, isCopMcu);

		delete[] strSiteName;
		delete[] strProductId;
		delete[] strVersionId;
	}
}
///////////////////////////////////////////////////////////////////////////
void CH323Party::OnPartyDowngradeToSecondary(CSegment* pParam)
{
	WORD reason;
	*pParam >> reason;
	SetPartyToSecondary(reason);
}

/////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323Party::OnPartyUpdateITPSpeaker shiraITP - 91
void CH323Party::OnPartyUpdateITPSpeaker(CSegment* pParam)
{
	BYTE  itpType;
	DWORD numOfActiveLinks;

	*pParam >> numOfActiveLinks;
	*pParam >> itpType;

	TRACEINTO << "PartyId:" << GetPartyId() << ", numOfActiveLinks:" << numOfActiveLinks << ", itpType:" << (int)itpType;

	PASSERT_AND_RETURN(!m_pConfApi);

	m_pConfApi->UpdateMainPartyOnITPSpeaker(GetPartyId(), numOfActiveLinks, (eTelePresencePartyType)itpType);
}

/////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323Party::OnPartyUpdateITPSpeakerAck shiraITP - 112
void CH323Party::OnPartyUpdateITPSpeakerAck(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId();

	PASSERT_AND_RETURN(!m_pConfApi);

	m_pConfApi->UpdateMainPartyOnITPSpeakerAck(GetPartyId());
}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::SetPartySecondaryCause(CSegment* pParam)
{
	WORD reason;
	*pParam >> reason;

	TRACEINTO << "PartyId:" << GetPartyId() << ", Reason:" << reason;

	CSecondaryParams secParams;
	secParams.DeSerialize(NATIVE, *pParam);

	m_pConfApi->SetPartySecondaryCause(GetPartyId(), reason, secParams);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnConfDisconnectMediaChannel(CSegment* pParam)
{
	WORD channelType;
	WORD channelDirection;
	WORD roleLabel;

	*pParam >> channelType;
	*pParam >> channelDirection;
	*pParam >> roleLabel;

	if((ERoleLabel)roleLabel == kRolePeople)
		PTRACE2(eLevelInfoNormal,"CH323Party::OnConfDisconnectMediaChannel people: Name - ",PARTYNAME);
	else
		PTRACE2(eLevelInfoNormal,"CH323Party::OnConfDisconnectMediaChannel content: Name - ",PARTYNAME);

	//we need to set the mode to off, because when we will get a new scm, before all the requested channels have disconnected, we need to send in the answer to the conf, the current mode without those channels
	m_pCurrentModeH323->SetMediaOff((cmCapDataType)channelType, (cmCapDirection)channelDirection, (ERoleLabel)roleLabel);

	m_pH323Cntl->OnConfOrPartyDisconnectMediaChannel(channelType,(cmCapDirection)channelDirection,(ERoleLabel)roleLabel);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnConfDeActiveMediaChannel(CSegment* pParam)
{
	// this method is being called only where implementing secondary. thus, if cascade is between RMX & MGC,
	// in order to get MGC moving to secondary too, different behavior is required buy RMX.
	// therefore, in such case, we disconnect the channel instead of deactivating it
	if( m_pH323Cntl->GetRemoteIdent() == PolycomMGC &&
	   (m_pCurrentModeH323->GetConfType() == kVideoSwitch || m_pCurrentModeH323->GetConfType() == kVSW_Fixed) )
	{
	    PTRACE2(eLevelInfoNormal,"CH323Party::OnConfDeActiveMediaChannel - cascade between RMX & MGC disconnect video channel instead of deactivating it. Name - ",PARTYNAME);
		OnConfDisconnectMediaChannel(pParam);
		return;
	}

	WORD channelType;
	WORD channelDirection;
	WORD roleLabel;

	*pParam >> channelType;
	*pParam >> channelDirection;
	*pParam >> roleLabel;

	if (((cmCapDataType)channelType == cmCapVideo) && ((cmCapDirection)channelDirection == cmCapReceive) && ((ERoleLabel)roleLabel == kRolePeople))
	{
		m_pCurrentModeH323->SetVideoBitRate(0, (cmCapDirection)channelDirection, (ERoleLabel)roleLabel);
		m_pH323Cntl->SendFlowControlReq((cmCapDataType)channelType, channelDirection, (ERoleLabel)roleLabel, 0);
		(m_pH323Cntl->GetCurrentMode())->SetVideoBitRate(0, cmCapReceive, kRolePeople);
	}
}

//To recover from Secondary only
/////////////////////////////////////////////////////////////////////////////
void CH323Party::ReActivateIncomingChannelIfNeeded()
{
    if (m_pCurrentModeH323->GetMediaBitRate(cmCapVideo, cmCapReceive) == 0)
    {
        DWORD curPeopleRate = m_pH323Cntl->GetCurrentPeopleRate();
        m_pCurrentModeH323->SetVideoBitRate(curPeopleRate, cmCapReceive, kRolePeople);
        m_pH323Cntl->SendFlowControlReq(cmCapVideo, FALSE, kRolePeople, curPeopleRate);
		(m_pH323Cntl->GetCurrentMode())->SetVideoBitRate(curPeopleRate, cmCapReceive, kRolePeople);
    }
}

/*
//////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnH323PartyDisconnectMmlp(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323PartyDisconnectMmlp : Name - ",PARTYNAME);

	m_pConfApi->PartyDisconnectMmlp(this);
}
*/
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::SendAddedProtocolToConfLevel(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendAddedProtocolToConfLevel : Name - ",PARTYNAME);

	CSegment* pSeg = new CSegment;

	WORD NoAddedProtocols;
	WORD ProtocolCapEnum;

	*pParam >> NoAddedProtocols
			>> ProtocolCapEnum;

	*pSeg << NoAddedProtocols
		  << ProtocolCapEnum;

	m_pConfApi->AddProtocolToH323Party(GetPartyId(), pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::SendRemovedProtocolToConfLevel(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendRemovedProtocolToConfLevel : Name - ",PARTYNAME);

	CSegment* pSeg = new CSegment;

	WORD NoremovedProtocols;
	WORD ProtocolPayLoadType;
	WORD bIsCapEnum;

	*pParam >> NoremovedProtocols
			>> ProtocolPayLoadType
			>> bIsCapEnum;

	*pSeg << NoremovedProtocols
		  << ProtocolPayLoadType
		  << bIsCapEnum;

	m_pConfApi->RemoveProtocolFromH323Party(GetPartyId(), pSeg);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::UpdateLocalCapsInConfLevel(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::UpdateLocalCapsInConfLevel : Name - ",PARTYNAME);

	CSegment* pSeg = new CSegment;

	CCapH323* pTmpLocalCaps = new CCapH323;
	pTmpLocalCaps->DeSerialize(NATIVE,*pParam);

	m_pConfApi->UpdateLocalCapsInConfLevel(GetPartyId(), *pTmpLocalCaps);
	POBJDELETE(pTmpLocalCaps);
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::UpdatePartyH323VideoBitRate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::UpdatePartyH323VideoBitRate : Name - ",PARTYNAME);


	DWORD newBitRate;
	WORD channelDirection;
	WORD roleLabel;

	*pParam >> newBitRate;
	*pParam >> channelDirection;
	*pParam >> roleLabel;

    DWORD curBitRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapTransmit);
    DWORD curBitRateContent = m_contentRate;// m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapTransmit,kRoleContentOrPresentation);
    DWORD totalAllowedRate = 0;
    CChannel* pOutChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRolePeople);
    if(curBitRateContent == 0)
    {
    	if(pOutChannel)
    		totalAllowedRate = pOutChannel->GetRate();
    	else
    		DBGPASSERT(5);
    }
    else
    {
    	if(pOutChannel)
    		totalAllowedRate = pOutChannel->GetRate() - curBitRateContent;
    	else
    		DBGPASSERT(6);
    }
    DWORD newConstraintRate = newBitRate;
    if(newBitRate > totalAllowedRate)
    {
    	PTRACE(eLevelInfoNormal,"CH323Party::UpdatePartyH323VideoBitRate got new rate higher than allowed rate - change to allowed");
    	newBitRate = totalAllowedRate;
    }
    //PTRACE2INT(eLevelInfoNormal,"CH323Party::UpdatePartyH323VideoBitRate total rate is: ",totalAllowedRate);
    DWORD confType = m_pInitialModeH323->GetConfType();
    m_pCurrentModeH323->SetConfType((EConfType)confType);
    PTRACE2INT(eLevelInfoNormal,"CH323Party::UpdatePartyH323VideoBitRate new rate is: ",newBitRate);
    PTRACE2INT(eLevelInfoNormal,"CH323Party::UpdatePartyH323VideoBitRate new Constraint rate is: ",newConstraintRate);
	if ( ((cmCapDirection)channelDirection == cmCapTransmit) && ((ERoleLabel)roleLabel == kRolePeople) &&
         (newBitRate <= totalAllowedRate || m_pCurrentModeH323->GetFlowControlRateConstraint()))
	{
		if ((confType == kVideoSwitch) || (confType == kVSW_Fixed))
		{
			// VSW:

			newBitRate = newConstraintRate; // In VSW, we need to update the Bridge on the real new constraint video rate of this party, regardless having content right now. So don't use the totalAllowedRate, because it is only temporary rate limitation as a result of content.
			                                // This way, when the content will stop, the constraint rate will of this party will be the right constraint regardless content and so we will be able to return to the correct rate that this party allow.
											// Consideration of having content, should be regard only when actually sending the flow control to party (in SendFlowControlToCs function).
			BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
			if(!bEnableFlowControlVSW)
			{
				PTRACE(eLevelError,"CH323Party::UpdatePartyH323VideoBitRate: SUPPORT_VSW_FLOW_CONTROL is false");
				return;
			}
			if ((newBitRate < (VSW_FLOW_CONTROL_RATE_THRESHOLD * GetVideoRate())) && (m_pH323Cntl->GetRemoteIdent() != PolycomMGC))
			{
				PTRACE(eLevelError,"CH323Party::UpdatePartyH323VideoBitRate: Flow control bit rate is lower than the threshold");
				return;
			}
			m_pCurrentModeH323->SetFlowControlRateConstraint(newBitRate);
			m_pH323Cntl->m_pCurrentModeH323->SetFlowControlRateConstraint(newBitRate);
		}
        else if (confType == kCop)
        {
            const CCommConf*  pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);

            if (pCommCurrConf == NULL) {
            	DBGPASSERT_AND_RETURN(7);
            }
            
            DWORD newBitRateWithoutTreshold =newBitRate;
            newBitRate = m_pCurrentModeH323->CalcCopMinFlowControlRate(pCommCurrConf, newBitRate,m_contentRate);
            m_pCurrentModeH323->SetFlowControlRateConstraint(newBitRate);
            m_pH323Cntl->m_pCurrentModeH323->SetFlowControlRateConstraint(newBitRate);
            if( !IsRemoteIsSlaveMGCWithContent() )
            	m_pH323Cntl->SendFlowControlReq (cmCapVideo, TRUE , kRolePeople ,newBitRateWithoutTreshold);

        }
		else if (confType == kCp)
		{
			// CP:
			m_pCurrentModeH323->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)roleLabel);
	        m_pH323Cntl->m_pCurrentModeH323->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)roleLabel);

	        //VNGFE-8824 / BRIDGE-14975 - Set correct rate for the encoder to be updated.
	        m_pH323Cntl->m_pCurrentModeH323->SetTotalVideoRate(newBitRate);
	        m_pH323Cntl->m_pTargetModeH323->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapTransmit, (ERoleLabel)roleLabel);
	        UpdateVideoRate(newBitRate);

	        //VNGFE-8988 - Keeping the rates symmetric setting the same for our decoder receive channel to be updated.
	        m_pCurrentModeH323->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapReceive, (ERoleLabel)roleLabel);
	        m_pH323Cntl->m_pCurrentModeH323->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapReceive, (ERoleLabel)roleLabel);
	        m_pH323Cntl->m_pTargetModeH323->SetVideoBitRate(newBitRate, (cmCapDirection)cmCapReceive, (ERoleLabel)roleLabel);

	        //If the media is off when update video rate. we need to indicate the remote EP
			if (TRUE == m_pCurrentModeH323->IsMediaOff(cmCapVideo, (cmCapDirection)cmCapTransmit,  (ERoleLabel)roleLabel))
			{
				PTRACE(eLevelInfoNormal,"CH323Party::UpdatePartyH323VideoBitRate - Current video bitrate is 0!");
				m_pH323Cntl->SendFlowControlReq(cmCapVideo,  TRUE,   (ERoleLabel)roleLabel, 0);
			}
		}
		else
		{
			PTRACE(eLevelError,"CH323Party::UpdatePartyH323VideoBitRate: Need implementation for this type of conference");
		}
		//VNGFE-7077
		if((ERoleLabel)roleLabel == kRolePeople)
		{
			PTRACE(eLevelInfoNormal,"CH323Party::UpdatePartyH323VideoBitRate - UpdateVideoRate In EMA/GUI");
			H323UpdateBaudRate();
		}
		m_pConfApi->UpdatePartyH323VideoBitRate(GetPartyId(), newBitRate, (cmCapDirection)channelDirection, (ERoleLabel)roleLabel);
	}
}
/*
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::SendNewVideoRateToConflevel(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendNewVideoRateToConflevel : Name - ",PARTYNAME);

	CSegment* pSeg = new CSegment;
	DWORD vidRate;
	DWORD tdmRate;
	BYTE  oldRateCalc = FALSE; // in the case this function is called we cascade between CP
							   // and some kind of VSW (Old, Mixed, IP only). since we cascade
							   // it from CP the flag of oldRateCalc, that in the conf level
							   // use for flow control mechanize in IP only case, is set to FALSE.
	*pParam >> vidRate;
	*pParam >> tdmRate;

	m_videoRate = vidRate;// except from updating the conf level, we have to update also the party video rate
	m_tdmRate = tdmRate;

	*pSeg << vidRate;
	*pSeg << tdmRate;
	*pSeg << oldRateCalc;

	m_pConfApi->SendNewRatesFromH323Party(this, pSeg); //send to party control
	POBJDELETE(pSeg);
}

*/
/////////////////////////////////////////////////////////////////////////////
void CH323Party::SendFlowControlToCs(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendFlowControlToCs : Name - ",PARTYNAME);
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

    if (!outChannel)
    {
        PTRACE2INT(eLevelInfoNormal,"CH323Party::SendFlowControlToCs: Constraint set to - ", newVidRate);
        m_pH323Cntl->SetConfPeopleFlowControlConstraint (newVidRate);
    }

    // In order to prevent exceeding of the video rate when new constraint rate is sent by VSW bridge, when content is active from this party:
	if (((m_pCurrentModeH323->GetConfType() == kVideoSwitch) || (m_pCurrentModeH323->GetConfType() == kVSW_Fixed)) && !outChannel)
	{
		DWORD curBitRateContent = m_contentRate;// m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapTransmit,kRoleContentOrPresentation);
		DWORD totalAllowedRate = 0;
		CChannel* pInChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, FALSE, kRolePeople);
		if((curBitRateContent == 0) || !m_pH323Cntl->IsContentSpeaker())
		{
			if(pInChannel)
				totalAllowedRate = pInChannel->GetRate();
		}
		else
		{
			if(pInChannel)
				totalAllowedRate = pInChannel->GetRate() - curBitRateContent;
		}

		if(totalAllowedRate && (newVidRate > totalAllowedRate))
		{
			PTRACE2INT(eLevelInfoNormal,"CH323Party::SendFlowControlToCs  : got new rate higher than allowed rate - change to allowed. newVidRate=", newVidRate);
			newVidRate = totalAllowedRate;
		}
	}

    // In order to prevent exceeding of the video rate when new constraint rate is sent by VSW bridge, when content is active from this party:
    if (((m_pCurrentModeH323->GetConfType() == kVideoSwitch) || (m_pCurrentModeH323->GetConfType() == kVSW_Fixed)) && !outChannel)
    {
		DWORD curBitRateContent = m_contentRate;// m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapTransmit,kRoleContentOrPresentation);
		DWORD totalAllowedRate = 0;
		CChannel* pInChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, FALSE, kRolePeople);
		if((curBitRateContent == 0) || !m_pH323Cntl->IsContentSpeaker())
		{
			if(pInChannel)
				totalAllowedRate = pInChannel->GetRate();
		}
		else
		{
			if(pInChannel)
				totalAllowedRate = pInChannel->GetRate() - curBitRateContent;
		}

		if(totalAllowedRate && (newVidRate > totalAllowedRate))
		{
			PTRACE2INT(eLevelInfoNormal,"CH323Party::SendFlowControlToCs  : got new rate higher than allowed rate - change to allowed. newVidRate=", newVidRate);
			newVidRate = totalAllowedRate;
		}
    }


	*pParam >> isLpr;
	if (isLpr == TRUE)
	{
		*pParam >> lossProtection >> mtbf >> congestionCeiling >> fill >> modeTimeout;

		CSegment* pSeg = new CSegment;
		*pSeg << newVidRate << outChannel << lossProtection << mtbf << congestionCeiling
			<< fill << modeTimeout;

		DispatchEvent(LPR_PARTY_FLOWCONTROL,pSeg);

		POBJDELETE(pSeg);

	}
	else
	m_pH323Cntl->OnConfFlowControlReq(newVidRate, outChannel);
}

/*
/////////////////////////////////////////////////////////////////////////////
WORD CH323Party::GetMaxOfPmReq(CTaskApp *pTaskApp,int *pTotalLoad)
{
	//I will check the load on the card and if there is a load I will decrease the number of PM requests.
	DWORD		boardId   = m_pIpDesc->m_boardId;
	int			higherLoad = (int)(MAX_LOAD * 0.75);

	if(pTaskApp)
		*pTotalLoad =  ((CLoadMngrTask *)pTaskApp)->GetBoardLoad(boardId);

	if(*pTotalLoad < higherLoad)
		return NUM_OF_PM_REQ_IN_SEC;
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CH323Party::GetMaxOfPmReq: we in load of %d \n",*pTotalLoad);

		int		percentage = (*pTotalLoad * 100) / MAX_LOAD;

		return  (((100 - percentage) + 50) / 100 * NUM_OF_PM_REQ_IN_SEC);
	}

}
*/


/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnMcuMngrPartyMonitoringReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnMcuMngrPartyMonitoringReq : Name - ",PARTYNAME);


	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	APIU32 RefreshPeriodForPMinMiliSeconds = 0;
	std::string key = "PARTY_MONITORING_REFRESH_PERIOD";
	pSysConfig->GetDWORDDataByKey(key, RefreshPeriodForPMinMiliSeconds);
	RefreshPeriodForPMinMiliSeconds = RefreshPeriodForPMinMiliSeconds * 1000; // Convert to mili seconds
	if (RefreshPeriodForPMinMiliSeconds < 500)
		RefreshPeriodForPMinMiliSeconds = 500; // Minimum monitoring interval is 500 mili seconds.
	TICKS		curTicks;
	CTaskApp	*pTaskApp;


	*pParam >> (void *&)pTaskApp;

    //PartyMonitoring requests will be sent to a party
    //in a minimum interval defined in the system.cfg
	curTicks = SystemGetTickCount();
    TICKS diff;
    diff = curTicks - m_lastPmTicks;

    //GetIntegerPartForTrace return value is 1/100 of a second
    if (diff.GetIntegerPartForTrace() * 10 >= RefreshPeriodForPMinMiliSeconds)
	{
        PTRACE2(eLevelInfoNormal,"CH323Party::OnMcuMngrPartyMonitoringReq, name - ",PARTYNAME);

        m_pH323Cntl->PartyMonitoringReq();
        m_lastPmTicks = curTicks;
    }
	else
	{
		CMedString msg;
		msg << "Do not send PartyMonitor - less then = " << RefreshPeriodForPMinMiliSeconds << " miliseconds ,  Name - " << PARTYNAME << "\n";
		PTRACE2(eLevelInfoNormal,"CH323Party::OnMcuMngrPartyMonitoringReq: ", msg.GetString());
	}
}

/*
/////////////////////////////////////////////////////////////////////////////////////////////
void  CH323Party::SendFlowControlPartyToConflevel(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendFlowControlPartyToConflevel : Name - ",PARTYNAME);

	CSegment* pSeg = new CSegment;
	DWORD vidRate;
	DWORD tdmRate;

	*pParam >> vidRate;
	*pParam >> tdmRate;

	m_videoRate = vidRate;// except from updating the conf level, we have to update also the party video rate
	m_tdmRate = tdmRate;

	*pSeg << vidRate;
	*pSeg << tdmRate;

	m_pConfApi->SetFlowControlPartyAndSendToConf(this, pSeg); //send to party control
	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::IsPossibleToChangeCaps(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::IsPossibleToChangeCaps : Name - ",PARTYNAME);

	DWORD vidRate;
    *pParam >> vidRate;

    CSegment* pRspMsg = NULL;
	BYTE isOtherPartyConnected = 0;

	WORD rc = m_pConfApi->CheckForAnotherConnectedParty(this, &pRspMsg, 30*SECOND);

	if(!rc)
		if(pRspMsg != NULL )
			 *pRspMsg >> isOtherPartyConnected;

	if (!rc && !isOtherPartyConnected) //this is the first party which is connecting to the conference
	{
		//update rates at the party level
		m_videoRate = vidRate;
		m_tdmRate   = vidRate;

		//updating the conf level
		CSegment* pSeg = new CSegment;
		BYTE oldRateCalc = TRUE; // in the case this function is called we cascade between VSW
							   // and some kind of VSW (Old, Mixed, IP only). The flag of
							   // oldRateCalc, that in the conf level use for flow control
							   // mechanize in IP only case, is set to TRUE.
		*pSeg << vidRate;
		*pSeg << vidRate; //tdm rate is the same as video rate in this case
		*pSeg << oldRateCalc;

		m_pConfApi->SendNewRatesFromH323Party(this, pSeg); //send to party control
		POBJDELETE(pSeg);
		m_pH323Cntl->ChangeCapsAndCreateControl(vidRate);
	}
	m_pH323Cntl->OnPartyCreateControl();

	if(pRspMsg)
		POBJDELETE(pRspMsg);
}


/////////////////////////////////////////////////////////////////////////////*/
void  CH323Party::OnVidBrdgRefresh(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CH323Party::OnVidBrdgRefresh : Name - ",PARTYNAME);
  WORD ignore_filtering = FALSE;
  if(pParam != NULL){
    *pParam >> ignore_filtering;
  }
  if(ignore_filtering){
    PTRACE2(eLevelInfoNormal,"CH323Party::OnVidBrdgRefresh ignore filtering: Name - ",PARTYNAME);
    m_pH323Cntl->FastUpdate();
  }else{
	PeopleIntraRequestFiltering();
  }
}

/////////////////////////////////////////////////////////////////////////////*/
void  CH323Party::PeopleIntraRequestFiltering(WORD videoDirection)
{
    // vngr-7017 "bombing" logger on change layout - change trace to DEBUG level
	PTRACE2(eLevelInfoNormal,"CH323Party::PeopleIntraRequestFiltering: Name - ",PARTYNAME);
	TICKS curTimer;

	//Fast Update requests will be sent to a party
	//in a minimum interval of 1 seconds
	curTimer = SystemGetTickCount();

	if( videoDirection == eReceiveIntra )
	{
		if (curTimer < m_lastVcuTime)
			m_lastVcuTime = 0; //rollover fixup

		if ((m_lastVcuTime == 0) || (curTimer - m_lastVcuTime > SECOND*1 ))
		{
			m_pH323Cntl->FastUpdate();//default role: people
			m_lastVcuTime = curTimer;
		}
	}
	else
	{
		DWORD filterTime = SECOND*1; // regular filter

		if (curTimer < m_lastBridgeRefreshTime)
	     	  m_lastBridgeRefreshTime = 0; //rollover fixup

	 	CLargeString str;
	 	str << "CSipParty::IntraRequestFiltering ";

	 	str << " curTimer=" << curTimer.GetMiliseconds();
	 	str << " m_lastBridgeRefreshTime=" << m_lastBridgeRefreshTime.GetMiliseconds();
	 	str << " diffInMilSec=" << (curTimer - m_lastBridgeRefreshTime).GetMiliseconds();


	     if ((m_lastBridgeRefreshTime == 0) || (curTimer - m_lastBridgeRefreshTime > filterTime ))
	     {
			 m_pConfApi->VideoRefresh(GetPartyId());
			 m_lastBridgeRefreshTime = curTimer;

	     }

	}
	//else
	//   timer has not elapsed, don't send
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnContentBrdgRefreshVideo(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnContentBrdgRefreshVideo, Name - ",PARTYNAME);
	WORD ignore_filtering = FALSE;
	if (pParam != NULL)
	{
		*pParam >> ignore_filtering;
	}

	if (ignore_filtering)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnContentBrdgRefreshVideo ignore filtering: Name - ",PARTYNAME);
		m_pH323Cntl->FastUpdate(kRoleContentOrPresentation);
	}
	else
	ContentIntraRequestFiltering();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::ContentIntraRequestFiltering()
{

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
		m_pH323Cntl->FastUpdate(kRoleContentOrPresentation);
		m_lastContentRefreshTime = curTimer;
		StartTimer(CONTENT_SPEAKER_INTRA_SUPPRESSION_TIMER,SECOND*suppresionIntervalInSeconds);
		m_num_content_intra_filtered=0;
	}
	else
	{
		// if timer
		m_num_content_intra_filtered++;
		PTRACE2(eLevelInfoNormal,"CH323Party::ContentIntraRequestFiltering,-do not send intra  Name - ",PARTYNAME);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnTimerContentSpeakerIntraRequest(CSegment* pParam)
{
	if (m_num_content_intra_filtered > 0)
	{
		m_num_content_intra_filtered = 0;
		m_pH323Cntl->FastUpdate(kRoleContentOrPresentation);
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

/*
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnAudBrdgValidation(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnAudBrdgValidation : Name - ",PARTYNAME);
}


/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnMlpBrdgValidation(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CH323Party::OnMlpBrdgValidation : Name - ",PARTYNAME);
  WORD onOff = 0;
  *pParam >> onOff;
}

*/
////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323DTMFInd(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnPartyDtmfInd : Name - ",PARTYNAME);
	CSegment *pSeg = new CSegment;
	DWORD len = 0;
	DWORD dtmfOpdoce = 0;
	unsigned char* tempArray = NULL;

	*pParam >> len;
	tempArray = new unsigned char[len];
	pParam->Get(tempArray, len);
	*pParam >> dtmfOpdoce;

    *pSeg << len;
	pSeg->Put(tempArray,len);

	m_pConfApi->IvrPartyNotification(GetPartyRsrcID(), this, GetName(), dtmfOpdoce, pSeg);
	PDELETEA(tempArray);
	POBJDELETE(pSeg);
}


/////////////////////////////////////////////////////////////////////////////
// EPC functions
/////////////////////////////////////////////////////////////////////////////

void CH323Party::SendTokenMessageToConfLevel(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendTokenMessageToConfLevel : Name - ",PARTYNAME);
	if(m_pH239TokenMsgMngr->isEnable())
		SpreadAllH239Msgs(pParam,eMsgIn);
	else
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::SendTokenMessageToConfLevel-token mngr not on yet : Name - ",PARTYNAME);
		DWORD subOpcode;
		EHwStreamState eStreamStateForTmm = eHwStreamStateNone;
		*pParam >> subOpcode;
		if(subOpcode==PARTY_TOKEN_ACQUIRE)
		{
		// Sending New token msg to TMM
			CTokenMsg* pTokenMsg = new CTokenMsg(PARTY_TOKEN_ACQUIRE, eMsgIn, pParam);
			//EMsgStatus eMsgStat = m_pH239TokenMsgMngr->NewTokenMsg(pTokenMsg,((EMsgStatus)0));
			EMsgStatus eMsgStat = m_pH239TokenMsgMngr->NewTokenMsg(pTokenMsg,eStreamStateForTmm);
			PTRACE2(eLevelInfoNormal,"CH323Party::SendTokenMessageToConfLevel -outchannel not updated yet: Name - ",PARTYNAME);
			POBJDELETE(pTokenMsg);

		}
	}

}

//////////////////////////////////////////////////////////////////////////////
void CH323Party::CallGeneratorRecieveTokenFromRemote(CSegment* pParam)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CH323Party::CallGeneratorRecieveTokenFromRemote - ERROR - system is not CG!!");
		return;
	}

	PTRACE2(eLevelInfoNormal,"CH323Party::CallGeneratorRecieveTokenFromRemote : Name - ",PARTYNAME);
	DWORD opCode = 0;
	BYTE isAck = 0;
	*pParam >> opCode;
	*pParam >> isAck;
	switch (opCode)
	{
		case kPresentationTokenResponse:
		{
			if( m_cgContentState == eWaitToSendStreamOn && isAck)
			{
				m_pH323Cntl->OnPartyMediaProducerStatusReq(YES);
				m_pH323Cntl->SendCGContentOnOffReqForRtp(YES);
				m_cgContentState = eStreamOn;
			}
			else if (m_cgContentState != eWaitToSendStreamOn)
			{
				PTRACE2(eLevelInfoNormal,"CH323Party::CallGeneratorRecieveTokenFromRemote-recieve token respone not in right time : Name - ",PARTYNAME);
			}
			else if(!isAck)
				PTRACE2(eLevelInfoNormal,"CH323Party::CallGeneratorRecieveTokenFromRemote-recieved NACK on token request : Name - ",PARTYNAME);
			break;
		}
		case kPresentationTokenRequest:
		{
			BYTE randNum = rand() % 100 + 1;
			if( m_cgContentState == eStreamOn )
			{
				PTRACE2(eLevelInfoNormal,"CH323Party::CallGeneratorRecieveTokenFromRemote -recieved withdraw -stop contnet : Name - ",PARTYNAME);
				m_pH323Cntl->OnPartyMediaProducerStatusReq(NO);
				m_pH323Cntl->SendCGContentOnOffReqForRtp(NO);
				m_cgContentState = eStreamOff;
				m_pH323Cntl->OnPartyRoleTokenReq(kPresentationTokenResponse,m_pH323Cntl->GetMcuNumFromMaster(),m_pH323Cntl->GetTerminalNumFromMaster(),randNum/* V4.1c <--> V6 merge, 1 */);

			}
			else
			{
				m_cgContentState = eStreamOff;
				m_pH323Cntl->OnPartyRoleTokenReq(kPresentationTokenResponse,m_pH323Cntl->GetMcuNumFromMaster(),m_pH323Cntl->GetTerminalNumFromMaster(),randNum/* V4.1c <--> V6 merge, 1 */);
			}

			break;

		}
		default:
		{
			PTRACE2INT(eLevelInfoNormal,"CH323Party::CallGeneratorRecieveTokenFromRemote -recieved withdraw -Wrong opcode for call generator : opcode is - ",opCode);

		}
	}

}



/////////////////////////////////////////////////////////////////////////////
EHwStreamState CH323Party::SetCorrectStreamStateForTMM()
{
	EHwStreamState eStreamStateForTmm = eHwStreamStateNone;

	switch (m_pH323Cntl->GetContentInStreamState())
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
			PASSERTMSG( (DWORD)m_pH323Cntl->GetContentInStreamState(),"CH323Party::SetCorrectStreamStateForTMM - Stream state not valid ");

		}
	}
	return eStreamStateForTmm;
}
/////////////////////////////////////////////////////////////////////////////
void CH323Party::SpreadAllH239Msgs(CSegment* pParam,EMsgDirection direction,WORD isFirstMsgAfterUpdate, DWORD OpCode,EMsgStatus msgStat)
{
	CMedString msg;
	msg <<  "Name - " << PARTYNAME << "\n\t\t\t\t";

	DWORD subOpcode;
	BYTE  McuNumber;
	BYTE  terminalNumber;
	BYTE  label;
	BYTE  randomNumber;
	EMsgStatus eMsgStat = eMsgInvalid;
	EHwStreamState eStreamStateForTmm;
	CTokenMsg* pTokenMsg = NULL;
	// In case isFirstMsgAfterUpdate is on this means we need to deal strait away with the message (Send it)
	// Without sending it to the TMM
	if (isFirstMsgAfterUpdate)
	{
		msg <<  ", First Msg after Update - " << msgStat;
		subOpcode = OpCode;
		eMsgStat = msgStat;
	}
	else
		*pParam >> subOpcode;
	// >> McuNumber >> terminalNumber >> label >> randomNumber;

//	if (!isFirstMsgAfterUpdate)
//	{
		// Phase 1:
		// When receiving an H239 opcode, we do the following

//	}
	msg <<  ", SubOpcode - " << subOpcode << ", StreamState - " << (DWORD)m_pH323Cntl->GetContentInStreamState();
	// There are 2 cases we will need to change our stream state.
	if (subOpcode == CONTENT_ROLE_TOKEN_ACQUIRE_ACK &&
        m_pH323Cntl->GetContentInStreamState() == eNoChannel)
	{
		// CASE 1:
		// If our stream state is eNoChannel and we receive Acquire Ack from the CB
		// We need to update our stream state to Waiting
		m_pH323Cntl->SetContentInStreamState(eWaitToSendStreamOn);
	}
	if ((subOpcode == CONTENT_ROLE_TOKEN_WITHDRAW ||
         subOpcode == CONTENT_ROLE_TOKEN_RELEASE ||
         subOpcode == CONTENT_ROLE_TOKEN_WITHDRAW_ACK))
	{
		// CASE 2:
		// If the stream state was eWaitToSendStreamOn and we get releaseReq from the EP
		// Or WithdrawReq from CB, We need to chek the content channel in state - If it is not opened yet
		// We need to change the state to eNoChannel.
		if (m_pH323Cntl->GetContentInStreamState() == eWaitToSendStreamOn)
			m_pH323Cntl->SetContentInStreamState(eNoChannel);
	}

	// 1. Set correct stream state for TMM
	//
	eStreamStateForTmm = SetCorrectStreamStateForTMM();

	if (!isFirstMsgAfterUpdate)
	{
		msg <<  ", not First Msg After Update. eStreamStateForTmm  - " << eStreamStateForTmm;
		// Phase 2: Sending New token msg to TMM
		pTokenMsg = new CTokenMsg(subOpcode, direction, pParam);
		eMsgStat = m_pH239TokenMsgMngr->NewTokenMsg(pTokenMsg,eStreamStateForTmm);
		msg <<  ", not First Msg After Update. eMsgStat  - " << eMsgStat;
	}
	PTRACE2(eLevelInfoNormal,"CH323Party::SpreadAllH239Msgs:: ", msg.GetString());

	// Phase 3: Treat according to eMsgStat
	// Phase 3.1: eMsgFree reply
	// In this case we simply send the Msg to it's destination CB/EP
	if (eMsgStat == eMsgFree)
	{
		if (direction == eMsgIn)
		{	// Msg from EP to CB
			PTRACE2(eLevelInfoNormal,"CH323Party::SpreadAllH239Msgs : Send msg case - Towards Content bridge, Name - ",PARTYNAME);
			*pParam >> McuNumber >> terminalNumber >> label >> randomNumber;
			if(subOpcode == ROLE_PROVIDER_IDENTITY)
			{
				BYTE* pData = NULL;
				DWORD  size;
				*pParam >> size;

				if( size > 0 )
				{
					pData = new BYTE[size];
					for( WORD i=0; i<size; i++ )
						*pParam >> pData[i];
				}
				m_pConfApi->ContentTokenRoleProviderMessage(this,McuNumber,terminalNumber,label,size,pData);
				PDELETEA(pData);
			}
			else
			{
                if (subOpcode == PARTY_TOKEN_RELEASE && m_pH323Cntl->IsSlaveCascadeModeForH239())
                    m_pConfApi->MasterContentMessage (MASTER_RATE_CHANGE,(CTaskApp*)this, m_mcuNum, m_termNum, 0);
                else if(m_pH323Cntl->GetCallParams()->GetRmtType() == cmEndpointTypeTerminal) // PVX bug - PVX send us mcu and terminal number (0,0)
				{
					m_pConfApi->SendContentTokenMessage(subOpcode,this,m_mcuNum,m_termNum,label,randomNumber);
					PTRACE2INT(eLevelInfoNormal,"CH323Party::SpreadAllH239Msgs: Party is Terminal and m_mcuNum is - :", m_mcuNum );
				}
				else
					m_pConfApi->SendContentTokenMessage(subOpcode,this,McuNumber,terminalNumber,label,randomNumber);
			}
		}
		else
		{ 	// Msg from CB to EP
			// For EPC and Duo ??
			PTRACE2(eLevelInfoNormal,"CH323Party::SpreadAllH239Msgs : Send msg case towards remote (H323cntl), Name - ",PARTYNAME);
			BYTE bIsSpeakerChange = FALSE;
			if (subOpcode == CONTENT_ROLE_TOKEN_WITHDRAW)
				*pParam >> bIsSpeakerChange;
			else if( subOpcode == CONTENT_MEDIA_PRODUCER_STATUS )
			{
				BYTE  channelId, status;
				*pParam >> channelId
						>> status;
				m_pH323Cntl->OnPartyMediaProducerStatusReq(status);
				if (pTokenMsg)
					POBJDELETE(pTokenMsg);
				return;
			}

			TranslateTokenMessageToStandardContentForH323Cntl(subOpcode, pParam, bIsSpeakerChange);
		}
	}
	// Phase 3.2: eMsgDelay reply
	// In this case we simply send the Msg to it's destination CB/EP
	else if (eMsgStat == eMsgDelayed)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::SpreadAllH239Msgs : Delay msg case, Name - ",PARTYNAME);
		if (m_pH323Cntl->GetContentInStreamState() == eSendStreamOn || m_pH323Cntl->GetContentInStreamState() == eSendStreamOff)
		{
			// In this case we do nothing
			// Since we are in the middle of another Content On/Off req from the RTP
			if (!isFirstMsgAfterUpdate)
			{
				POBJDELETE(pTokenMsg);
			}
			return;
		}
		// Updating the stream status
		if (m_pH323Cntl->GetContentInStreamState() == eStreamOn)
			m_pH323Cntl->SetContentInStreamState(eSendStreamOff);
		else if (m_pH323Cntl->GetContentInStreamState() == eStreamOff)
			m_pH323Cntl->SetContentInStreamState(eSendStreamOn);

		// Sending Content On/Off to the RTP.
		m_pH323Cntl->SendContentOnOffReqForRtp();
		StartTimer(RTPCONTENTACKTOUT,SECOND*2);
	}
	else
	{	// eMsgInvalid case
		PASSERTMSG(subOpcode,"H323Party::SpreadAllH239Msgs: Msg invalid");
	}

	if (!isFirstMsgAfterUpdate)
		POBJDELETE(pTokenMsg);
}


/////////////////////////////////////////////////////////////////////////////
void CH323Party::SendMediaProducerStatusToConfLevel(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendMediaProducerStatusToConfLevel : Name - ",PARTYNAME);
	BYTE channelID;
	BYTE status;
	*pParam >> channelID >> status;
	m_pConfApi->ContentMediaProducerStatus(this,channelID,status);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH323Party::DisconnectPartyDueToProblemsWithH239RtpStream()
{
	// In this case - we will send evacuate towards the RTP and disconnect the party
	PTRACE2(eLevelInfoNormal,"CH323Party::DisconnectPartyDueToProblemsWithH239RtpStream - Name - ",PARTYNAME);
	m_pH323Cntl->SendEvacuateReqForRtpOnH239Stream();
	m_pConfApi->PartyDisConnect(IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR,this);
	m_pConfApi->UpdateDB(this,DISCAUSE,IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR,1); // Disconnnect cause

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::HandleRtpProblemsDuringClosingContentStream()
{
	PTRACE2(eLevelInfoNormal,"CH323Party::HandleRtpProblemsDuringClosingContentStream - Name - ",PARTYNAME);
	// In this case we will:
	// 1. Send Evacuate and wait for the Ack
	// 1.1 In case we don't receive an Ack(Tout)/Receive Nack - Disconnect the party.
	// 1.2 If we receive an Ack from the Rtp - Change stream state to eStreamOff and release all queued token messages (Update).
	m_pH323Cntl->SendEvacuateReqForRtpOnH239Stream();
	StartTimer(RTPEVACUATEACKTOUT,2*SECOND);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnRtpAckForEvacuateTout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnRtpAckForEvacuateTout - Name - ",PARTYNAME);
	CSegment *pSeg = new CSegment;
	*pSeg << (DWORD)IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR;
	OnSendFaultyMfaToPartyCntl(pSeg);
	m_pConfApi->UpdateDB(this,DISCAUSE,IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR,1); // Disconnnect cause
	POBJDELETE(	pSeg );

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnRtpAckForEvacuateContentStream(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnRtpAckForEvacuateContentStream - Name - ",PARTYNAME);
	DeleteTimer(RTPEVACUATEACKTOUT);
	DWORD status;
	*pParam >> status;

	if (status != STATUS_OK)
	{ // Disconnect the party
		OnRtpAckForEvacuateTout(pParam);
		return;
	}

	m_pH323Cntl->SetContentInStreamState(eStreamOff);

	// Update stream to TMM - Handling the list...
	CTokenMsgMngr	*tokenMsgList = new CTokenMsgMngr;

	m_pH239TokenMsgMngr->StreamUpdate(tokenMsgList);

	if (tokenMsgList->Size() == 0)
	{
		PASSERTMSG((DWORD)m_pH323Cntl->GetContentInStreamState(),"CH323Party::OnRtpAckForEvacuateContentStream - List is empty ");
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
/////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnRtpAckForContentTout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnRtpAckForContentTout - Name - ",PARTYNAME);
	// Evacuate (Err handling)
	// There are 2 different paths:
	// 1. In case that we wait for stream ON ack and received NACK
	// 2. In case we wait for Stream OFF ack and receive NACK
	if (m_pH323Cntl->GetContentInStreamState() == eSendStreamOn)
	{
		// Case 1 - Stream ON
		DisconnectPartyDueToProblemsWithH239RtpStream();
	}
	else if (m_pH323Cntl->GetContentInStreamState() == eSendStreamOff)
	{
		// Case 2 - Stream OFF
		HandleRtpProblemsDuringClosingContentStream();
	}
	else
		PASSERTMSG((DWORD)m_pH323Cntl->GetContentInStreamState(), "CH323Party::OnRtpAckForContentTout - Wrong stream state");

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnRtpAckForContentOnOffWhileDisconnecting(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnRtpAckForContentOnOffWhileDisconnecting - Name - Do Nothing ",PARTYNAME);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnRtpAckForContentOnOff(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnRtpAckForContentOnOff - Name - ",PARTYNAME);
	DWORD status;
	EHwStreamState eStreamStateForTmm;
	EMsgStatus eMsgStat;
	BYTE isFirstMsgDelay = 0;
	DWORD tempContentInState = (DWORD)eNoChannel;
	*pParam >> status >> tempContentInState;;
	CTokenMsg* pTokenDelayMsg = NULL;
	DeleteTimer(RTPCONTENTACKTOUT);
	if (status != STATUS_OK)
	{
		// Error handling - Evacuate
		// There are 2 different paths:
		// 1. In case that we wait for stream ON ack and received NACK
		// 2. In case we wait for Stream OFF ack and receive NACK
		if (m_pH323Cntl->GetContentInStreamState() == eSendStreamOn)
		{
			// Case 1 - Stream ON
			DisconnectPartyDueToProblemsWithH239RtpStream();
		}
		else if (m_pH323Cntl->GetContentInStreamState() == eSendStreamOff)
		{
			// Case 2 - Stream OFF
			HandleRtpProblemsDuringClosingContentStream();
		}
		else
			PASSERTMSG((DWORD)m_pH323Cntl->GetContentInStreamState(), "CH323Party::OnRtpAckForContentOnOff - Wrong stream state");

		return;
	}

//	*pParam >>
	m_pH323Cntl->SetContentInStreamState((eContentState)tempContentInState);

	if (m_pH323Cntl->GetContentInStreamState() != eStreamOn)
	{
	 	if( m_pH323Cntl->GetContentInStreamState() != eStreamOff && m_pH323Cntl->GetContentInStreamState() != eNoChannel)
		{
			PASSERTMSG((DWORD)m_pH323Cntl->GetContentInStreamState(),"CH323Party::OnRtpAckForContentOnOff - Wrong stream state");
			return;
			// Error handling - TBD
		}
	}

	 // Add confApi Content On/Off
	 eContentState ePState = eStreamOff;
	 if (m_pH323Cntl->GetContentInStreamState() == eStreamOn)
	 {
	 	ePState = eStreamOn;
	 	 m_pConfApi->HWConetntOnOffAck(this,ePState);
	 }

	// Update stream to TMM - Handling the list...
	CTokenMsgMngr	*tokenMsgList = new CTokenMsgMngr;

	m_pH239TokenMsgMngr->StreamUpdate(tokenMsgList);

	if (tokenMsgList->Size() == 0)
	{
//		PASSERTMSG((DWORD)m_pH323Cntl->GetContentInStreamState(),"CH323Party::OnRtpAckForContentOnOff - List is empty ");
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

/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::HandleTMMList(CTokenMsgMngr* tokenMsgList)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::HandleTMMList - Name - ",PARTYNAME);
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
					PASSERTMSG((DWORD)eMsgStat,"CH323Party::OnRtpAckForContentOnOff - Status was suppose to be DELAY (Regular case)");
				}
			}
			else
				SpreadAllH239Msgs(pTokenMsg->GetMsgSegment(),pTokenMsg->GetMsgDirection(),1,pTokenMsg->GetMsgOpcode(),eMsgFree);


		}
		else
			PASSERTMSG(tokenMsgList->Size(),"CH323Party::OnRtpAckForContentOnOff - TMM list is corrupted - Token msg not valid !!");

		itr++;

	}

	if (isDelayed)
		SpreadAllH239Msgs(pDelayedTokenMsg->GetMsgSegment(),pDelayedTokenMsg->GetMsgDirection(),1,pDelayedTokenMsg->GetMsgOpcode(),eMsgDelayed);

	POBJDELETE(	pDelayedTokenMsg);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::SendEndChangeContentToConfLevel(CSegment* pParam)
{
	WORD bConfChangeMode = (m_state == PARTYCHANGEMODE)? TRUE: FALSE;
	m_state = PARTYCONNECTED;

	DWORD st = 0;
	EStat  status;

	*pParam >> (DWORD&)st;
	status = (EStat)st;

	CComModeH323* pCurrentMode = new CComModeH323;
	pCurrentMode->DeSerialize(NATIVE,*pParam);

	// if this was a conf change mode (or duo) - send the answer to conf level
	if (bConfChangeMode)
	{
	    CMedString msg;
	    msg << "***CH323Party::SendEndChangeContentToConfLevel - end change mode : Name - " << PARTYNAME
	    		<< ", TrContentRtae - " << pCurrentMode->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation)
	    		<< ", RcvContentRtae - " << pCurrentMode->GetMediaBitRate(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);

		PTRACE(eLevelInfoNormal, msg.GetString());
		// in EPC we set mode only if outgoing content is connected
		if( (m_pH323Cntl->GetCurrentMode()->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation)))
		{
			DWORD currConfContRate = m_pH323Cntl->GetCurConfContRate();
			pCurrentMode->SetVideoBitRate(currConfContRate,cmCapReceive,kRoleContentOrPresentation);
		}

		m_pConfApi->PartyEndChangeModeIp(GetPartyId(),*pCurrentMode,status);
		m_pH323Cntl->UpdateRtpWithLprInfo();
	}

	// else if it was a change speaker state and the conf is not waiting for an answer - don't send it
	else if (m_contentState == ChangeSpeaker)
	{
	    TRACESTR(eLevelInfoNormal) << " CH323Party::SendEndChangeContentToConfLevel: Didn't send to conf level in EPC change speaker! status "
							   << (DWORD)status << ",  Name - " << PARTYNAME;
		m_contentState = TokenIdle;
	}
	else
	{
		ALLOCBUFFER(str, MediumPrintLen);
		sprintf(str,"Unexpected condition: m_state %d, m_contentState %d, status %d", m_state,m_contentState,status);
		TRACESTR(eLevelInfoNormal) << " CH323Party::SendEndChangeContentToConfLevel: " << str << ",  Name - " << PARTYNAME;
		DEALLOCBUFFER(str);
	}

	m_changeContentModeState = eNotNeeded;
	POBJDELETE(pCurrentMode);
}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnConfContentTokenMessage(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentTokenMessage : Name - ",PARTYNAME);
	SpreadAllH239Msgs(pParam,eMsgOut);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::ContentRateChange(DWORD newRate, BYTE bIsSpeaker)
{
	PTRACE(eLevelInfoNormal,"CH323Party::ContentRateChange");
	m_state = PARTYCHANGEMODE;
	m_pH323Cntl->OnPartyContentRateChange(newRate,bIsSpeaker);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnConfContentRateChange(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentRateChange : Name - ",PARTYNAME);
	DWORD  subOpcode;
	DWORD newRate;
	BYTE  bIsSpeaker;
	*pParam >> subOpcode >> newRate >> bIsSpeaker;


	if (subOpcode == CONTENT_RATE_CHANGE)
	{
		ContentRateChange(newRate, bIsSpeaker);
	}
	else
		PTRACE2INT(eLevelInfoNormal,"CH323Party::OnConfContentRateChange : unknown sub opcode %d",subOpcode);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnConfContentChangeMode(CSegment* pParam)
{
	if (m_pRmtCapH323->IsECS())
	{
		PTRACE (eLevelError, "CH323Party::OnConfContentChangeMode ChangeMode recieved after ECS - Ignore!");
		return;
	}
    if (m_pH323Cntl->m_pmcCall->GetIsClosingProcess())
    {
		PTRACE (eLevelError, "CH323Party::OnConfContentChangeMode ChangeMode recieved when closing - Ignore!");
		return;
	}

	PTRACE2(eLevelInfoNormal,"***CH323Party::OnConfContentChangeMode : Name - ",PARTYNAME);
	PTRACE2INT(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode : state - ",m_state);
	WORD tempWordForEnum;
	BYTE bIsSpeaker;
	CComModeH323* pNewScm = new CComModeH323;
	eChangeModeState typeOfChange = eNotNeeded;

	*pParam >> tempWordForEnum;
	if( (((short)tempWordForEnum) >= 0) && (tempWordForEnum < eLastChangeModeState) ) //check validity
		typeOfChange = (eChangeModeState)tempWordForEnum;

	pNewScm->DeSerialize(NATIVE,*pParam);
	*pParam >> bIsSpeaker;
	m_changeContentModeState = typeOfChange;

	DWORD newRate = pNewScm->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
	PTRACE2INT(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode, change content rate : newRate - ",newRate);
	PTRACE2INT(eLevelInfoNormal,"***CH323Party::OnConfContentChangeMode, change content rate : m_changeContentModeState - ",m_changeContentModeState);

	if(m_changeContentModeState == eChangeContentRate)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode, change only content rate : Name - ",PARTYNAME);
		ContentRateChange(newRate, bIsSpeaker);
	}
	else //eChangeContentOut or eChangeContentIn
	{	// change the content mode:
		// 1. check if the new mode is different in content.
		// if different:
		// 2. close outgoing.
		// 3. send re-caps (after close out response arrive).
		// 4. Open new outgoing channel with the new mode
		// 5. Call ContentRateChange.
		PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode, change content protocol : Name - ",PARTYNAME);
		BYTE bNotNeedToChange = IsNewContentModeEqualToCurContentMode(pNewScm, cmCapReceiveAndTransmit);//
		DWORD contentRate =  m_pInitialModeH323->GetMediaBitRate(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

		if (bNotNeedToChange)
		{
			PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode - New scm is equal to current mode - Name - ",PARTYNAME);
			CSegment* pSeg = new CSegment;
			*pSeg << (DWORD)statOK;
			m_pCurrentModeH323->Serialize(NATIVE,*pSeg);
			*m_pInitialModeH323 = *m_pCurrentModeH323;
			m_pInitialModeH323->SetVideoBitRate(contentRate, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			SendEndChangeContentToConfLevel(pSeg);
			POBJDELETE(pSeg);
			POBJDELETE(pNewScm);
			return;
		}

		PTRACE(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode, In #1");
		// update target mode
		BYTE bUpdateSucceeded = m_pH323Cntl->UpdateTargetMode(pNewScm, cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		if (bUpdateSucceeded == FALSE)
		{
			PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode - Failed to update target mode, Name - ",PARTYNAME);
			m_pCurrentModeH323->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			CSegment* pSeg = new CSegment;
			*pSeg << (DWORD)statIllegal;
			m_pCurrentModeH323->Serialize(NATIVE,*pSeg);
			*m_pInitialModeH323 = *m_pCurrentModeH323;
			if(newRate == 0)
				m_pInitialModeH323->SetVideoBitRate(contentRate, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
			SendEndChangeContentToConfLevel(pSeg);
			POBJDELETE(pSeg);
			POBJDELETE(pNewScm);
			return;
		}
		PTRACE(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode, In #2");
		m_pInitialModeH323->SetMediaMode(pNewScm->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation), cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		if(newRate == 0)
			m_pInitialModeH323->SetVideoBitRate(contentRate, cmCapReceiveAndTransmit, kRoleContentOrPresentation);
		else
			m_pInitialModeH323->SetVideoBitRate(newRate, cmCapReceiveAndTransmit, kRoleContentOrPresentation);


		m_ContentChangeModeSpeaker = bIsSpeaker;
		m_ContentChangeModeRate = newRate;

		// close outgoing
		//m_pInitialModeH323->Dump("CH323Party::OnConfChangeModeConnect m_pInitialModeH323", eLevelInfoNormal);
		//m_pCurrentModeH323->Dump("CH323Party::OnConfChangeModeConnect m_pCurrentModeH323", eLevelInfoNormal);
        CChannel* pOutChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRoleContentOrPresentation);
        if (pOutChannel && m_changeContentModeState != eChangeContentIn)
        {
        	PTRACE(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode, In #3");

        	if (pOutChannel->GetCsChannelState() == kConnectedState)
        	{
	        	m_state = PARTYCHANGEMODE;
	        	m_pH323Cntl->OnConfOrPartyDisconnectMediaChannel(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
        	}
        	else
				PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode - Presentation out channel already closed!! Name - ",PARTYNAME);
        }

        else// out is close
        {// send re-caps
         // open out
        	PTRACE(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode, In #4");

        	const CMediaModeH323& RxTargetMode = m_pInitialModeH323->GetMediaMode(cmCapVideo, cmCapReceive,kRoleContentOrPresentation);
        	m_pCurrentModeH323->SetMediaMode(RxTargetMode, cmCapVideo, cmCapReceive,kRoleContentOrPresentation);
        	CComModeH323* currentMode = m_pH323Cntl->GetCurrentMode();
        	currentMode->SetMediaMode(RxTargetMode, cmCapVideo, cmCapReceive,kRoleContentOrPresentation);
//        	m_pCurrentModeH323->Dump("CH323Party::OnConfChangeModeConnect before recap m_pCurrentModeH323", eLevelInfoNormal);
        	m_state = PARTYCHANGEMODE;
			m_pH323Cntl->UpdateLocalCapsFromTargetMode(m_bIsVideoCapEqualScm);
            POBJDELETE(m_pLocalCapH323);
            m_pLocalCapH323 = new CCapH323(*m_pH323Cntl->m_pLocalCapH323);
            StartReCapsProcess();
        	m_pH323Cntl->OpenContentOutChannelFromMcms();
		if( m_pCurrentModeH323->IsMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation) )
		{
			PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode - end change mode 1 - ",PARTYNAME);
			m_state = PARTYCONNECTED;
	        m_pConfApi->PartyEndChangeModeIp(GetPartyId(), *m_pCurrentModeH323, statOK);
		}
		if(m_changeContentModeState == eChangeContentIn && !m_pCurrentModeH323->IsMediaOff(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation) )
			{

		     	PTRACE(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode, In #5");
				if(newRate)
				{
					PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode - change in and also content rate, Name - ",PARTYNAME);
					ContentRateChange(newRate, bIsSpeaker);
				}
				else
				{
					PTRACE2(eLevelInfoNormal,"CH323Party::OnConfContentChangeMode - end change mode 2 - ",PARTYNAME);
					m_state = PARTYCONNECTED;
					m_pConfApi->PartyEndChangeModeIp(GetPartyId(), *m_pCurrentModeH323, statOK);
					m_pH323Cntl->UpdateRtpWithLprInfo();
				}
			}
        }
	}
	POBJDELETE(pNewScm);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnConfContentChangeModeDisconnecting(CSegment* pParam)
{
	PTRACE (eLevelError, "CH323Party::OnConfContentChangeModeDisconnecting - Ignore!");

}
//////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::TranslateTokenMessageToStandardContentForH323Cntl(OPCODE subOpcode,CSegment* pParam, BYTE isSpeakerChange)
{
	BYTE mcuNum,terminalNum;
	CConfParty* pConfParty = GetConfPartyNonConst();
	DBGPASSERT_AND_RETURN(!pConfParty);
	*pParam >> mcuNum >> terminalNum;

	PTRACE2INT(eLevelInfoNormal,"CH323Party::TranslateTokenMessageToStandardContentForH323Cntl - Opcode to party - ",subOpcode);
	switch(subOpcode)
	{
	case CONTENT_ROLE_TOKEN_WITHDRAW:

		m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_ROLE_TOKEN_WITHDRAW,mcuNum,terminalNum, 0);
		if (isSpeakerChange) //change the speaker:
		{
			m_contentState = ChangeSpeaker;
			m_pH323Cntl->OnPartyContentSpeakerChange(NO); //no longer the speaker
		}
		break;
//	case NS_COM_ROLE_TOKEN_WITHDRAW_ACK:
// not liggle in H239
//		m_pH323Cntl->OnPartyRoleTokenReq(kRoleTokenWithdrawAckReq,mcuNum,terminalNum);
//		break;
//	case NS_COM_ROLE_TOKEN_RELEASE:
//		{// not liggle in H239
//			BYTE randomnum;
//			*pParam >> randomnum;
//
//			m_pH323Cntl->OnPartyRoleTokenReq(kRoleTokenReleaseReq,mcuNum,terminalNum,randomnum);
//		}
//		break;
	case CONTENT_ROLE_TOKEN_RELEASE_ACK:
		{	//not supported in H239
			m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_ROLE_TOKEN_RELEASE_ACK,mcuNum,terminalNum,0);
		break;
		}
	case CONTENT_ROLE_PROVIDER_IDENTITY:
		{
			m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_ROLE_PROVIDER_IDENTITY,mcuNum,terminalNum, 0);
		break;
		}
	case CONTENT_NO_ROLE_PROVIDER:
		{
		// not supported in H239
			m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_NO_ROLE_PROVIDER,mcuNum,terminalNum);
			break;
		}
	case CONTENT_ROLE_TOKEN_ACQUIRE: //In cascade mode the slave send the acquire to the master (link)
		{
			BYTE randomNum;

			*pParam >> randomNum;
			m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_ROLE_TOKEN_ACQUIRE,mcuNum,terminalNum,randomNum);
		}
		break;

	case CONTENT_ROLE_TOKEN_ACQUIRE_ACK:
    {
		if (pConfParty -> GetTokenRecapCollisionDetection() != etrcdRecapInProgress)
		{
			//=============================
			// A recap is not in progress
			//=============================
			PTRACE(eLevelInfoNormal,"CH323Party::TranslateTokenMessageToStandardContentForH323Cntl - handling ACQUIRE_ACK");
			pConfParty -> SetTokenRecapCollisionDetection(etrcdTokenHandlingInProgress);
			DWORD curConfContRate = m_pH323Cntl->GetCurConfContRate();
			if(!(m_pH323Cntl->m_pmcCall->GetRmtType() == cmEndpointTypeMCU && m_pH323Cntl->m_pmcCall->GetMasterSlaveStatus() == cmMSSlave))
				m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_ROLE_TOKEN_ACQUIRE_ACK,mcuNum,terminalNum, 0);

			// if this is not the first speaker the conf content rate is not zero
			if (curConfContRate)
			{
				m_contentState = ChangeSpeaker;
				m_pH323Cntl->OnPartyContentSpeakerChange(YES,curConfContRate); //the new speaker
			}
		}
		else
		{
			//=================================================
			// A recap is in progress, pending token handling
			//=================================================
			PTRACE(eLevelInfoNormal,"CH323Party::TranslateTokenMessageToStandardContentForH323Cntl - ACQUIRE_ACK --PENDED-- due to collision with recap");
			pConfParty -> PendTokenRecapDueToCollisionDetection();
			*pParam << mcuNum << terminalNum;
			m_pendedToken 					= *pParam;
			m_pendedTokenOpcode				= subOpcode;
			m_pendedTokenIsSpeakerChange	= isSpeakerChange;
		}
		break;
	}
    case CONTENT_ROLE_TOKEN_ACQUIRE_NAK:
    {
    	m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_ROLE_TOKEN_ACQUIRE_NAK,mcuNum,terminalNum, 0);
        break;
    }
    case CONTENT_ROLE_TOKEN_WITHDRAW_ACK:
    {
    	m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_ROLE_TOKEN_WITHDRAW_ACK,mcuNum,terminalNum, 0);
        break;
    }
    case CONTENT_ROLE_TOKEN_RELEASE:
    {
		if (pConfParty -> GetTokenRecapCollisionDetection() != etrcdRecapInProgress)
		{
			//=============================
			// A recap is not in progress
			//=============================
			PTRACE(eLevelInfoNormal,"CH323Party::TranslateTokenMessageToStandardContentForH323Cntl - handling RELEASE");
			pConfParty -> SetTokenRecapCollisionDetection(etrcdTokenHandlingInProgress);
			m_pH323Cntl->OnPartyRoleTokenReq(CONTENT_ROLE_TOKEN_RELEASE,mcuNum,terminalNum, 0);
		}
		else
		{
			//=================================================
			// A recap is in progress, pending token handling
			//=================================================
			PTRACE(eLevelInfoNormal,"CH323Party::TranslateTokenMessageToStandardContentForH323Cntl - RELEASE --PENDED-- due to collision with recap");
			pConfParty -> PendTokenRecapDueToCollisionDetection();
			m_pendedToken 					= *pParam;
			m_pendedTokenOpcode				= subOpcode;
			m_pendedTokenIsSpeakerChange	= isSpeakerChange;
		}
        break;
    }

	default:
		PTRACE2INT(eLevelInfoNormal,"CH323Party::TranslateTokenMessageToStandardContentForH323Cntl: Unknown sub opcode %d",subOpcode);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnPartyCntlUpdatePresentationOutStream(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnPartyCntlUpdatePresentationOutStream : Name - ",PARTYNAME);

	if( m_changeContentModeState == eChangeContentOut )
	{
	     m_pConfApi->PartyEndChangeModeIp(GetPartyId(), *m_pCurrentModeH323, statOK);
	     m_pH323Cntl->UpdateRtpWithLprInfo();
	}

	m_pH323Cntl->UpdatePresentationOutStream();
}

//////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnPartyUpdatedPresentationOutStream(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnPartyUpdatedpresentationOutStream : Name - ",PARTYNAME);
	m_pConfApi->UpdatePresentationRes(GetPartyId());
	CTokenMsg* pTokenMsg = NULL;

	WORD isDelayedReqFound=0;
	// Update stream to TMM - Handling the list...
	CTokenMsgMngr	*tokenMsgList = new CTokenMsgMngr;
	m_pH239TokenMsgMngr->StreamUpdate(tokenMsgList);
	m_pH239TokenMsgMngr->EnableTokenMsgMngr();

	BYTE  McuNumber;
	BYTE  terminalNumber;
	BYTE  label;
	BYTE  randomNumber;

	if (tokenMsgList->Size() == 0)
	{
	//		PASSERTMSG((DWORD)m_pH323Cntl->GetContentInStreamState(),"CH323Party::OnRtpAckForContentOnOff - List is empty ");
		tokenMsgList->Clear();
		POBJDELETE(tokenMsgList);

		return;
	}
	else
	{
		CSegment*  seg = new CSegment;
		PTRACE2(eLevelInfoNormal,"CH323Party::OnPartyUpdatedpresentationOutStream - messages in line: Name - ",PARTYNAME);
		TOKEN_MSG_LIST::iterator itr =  tokenMsgList->Begin();
		while (itr != tokenMsgList->End())
		{
			pTokenMsg = (CTokenMsg*)(*itr);
			if(IsValidPObjectPtr(pTokenMsg))
			{
				if(pTokenMsg->GetMsgOpcode()==PARTY_TOKEN_ACQUIRE && pTokenMsg->GetMsgDirection()== eMsgIn && isDelayedReqFound==FALSE )
				{
					PTRACE2(eLevelInfoNormal,"CH323Party::OnPartyUpdatedpresentationOutStream - token request found: Name - ",PARTYNAME);
					isDelayedReqFound=TRUE;
					PTRACE2INT(eLevelInfoNormal,"CH323Party::OnPartyUpdatedpresentationOutStream opcode is = ", (pTokenMsg->GetMsgOpcode()));
					*(pTokenMsg->GetMsgSegment()) >> McuNumber >> terminalNumber >> label >> randomNumber;

				   *seg << (DWORD)(pTokenMsg->GetMsgOpcode())
				  	    << (BYTE)McuNumber
						<< (BYTE)terminalNumber
						<< (BYTE)label
						<< (BYTE)randomNumber
						<< (DWORD)0;
					break;
				}
			}
			++itr;
		}
		if(TRUE)
		{
			SpreadAllH239Msgs(seg,eMsgIn);
		}

		tokenMsgList->ClearAndDestroy();
		POBJDELETE(tokenMsgList);
		POBJDELETE(seg);
	}
}


//Call generator functions
/////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnCGStartContent(CSegment* pParam)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CH323Party::CallGeneratorRecieveTokenFromRemote - ERROR - system is not CG!!");
		return;
	}

	PTRACE2(eLevelInfoNormal,"CH323Party::OnCGStartContent : Name - ",PARTYNAME);
	m_isCallGeneratorParty = TRUE;
	if( m_cgContentState != eStreamOff)
		PASSERTMSG(m_pH323Cntl->GetConnectionId(),"CH323Party::OnCGStartContent - starting content-on cg when content state is not off");
	BYTE randNum = rand() % 100 + 1;
	//if there is no content we start activating contnet
	m_cgContentState = eWaitToSendStreamOn;//third rand
	m_pH323Cntl->OnPartyRoleTokenReq(kPresentationTokenRequest,m_pH323Cntl->GetMcuNumFromMaster(),m_pH323Cntl->GetTerminalNumFromMaster(),randNum/* V4.1c <--> V6 merge, 0 */);

}


/////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnCGStopContent(CSegment* pParam)
{
	if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
	{
		PTRACE(eLevelInfoNormal,"CH323Party::CallGeneratorRecieveTokenFromRemote - ERROR - system is not CG!!");
		return;
	}

	BYTE randNum = rand() % 100 + 1;
	if(m_cgContentState == eStreamOn )
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnCGStopContent -recieved stop content from gui -stop contnet : Name - ",PARTYNAME);
		m_pH323Cntl->OnPartyMediaProducerStatusReq(NO);
		m_pH323Cntl->SendCGContentOnOffReqForRtp(NO);
		m_cgContentState = eStreamOff;
		m_pH323Cntl->OnPartyRoleTokenReq(kPresentationTokenRelease,m_pH323Cntl->GetMcuNumFromMaster(),m_pH323Cntl->GetTerminalNumFromMaster(),randNum/* V4.1c <--> V6 merge, 0 */);
	}
	else
	{
		m_cgContentState = eStreamOff;
		m_pH323Cntl->OnPartyRoleTokenReq(kPresentationTokenRelease,m_pH323Cntl->GetMcuNumFromMaster(),m_pH323Cntl->GetTerminalNumFromMaster(),randNum/* V4.1c <--> V6 merge, 0 */);
	}
}


//////////////////////////////////////////////////////////////////////////////////////////
//ECS FUNCTIONS:
/////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::SendECSToPartyControl()
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendECSToPartyControl : Name - ",PARTYNAME);
	m_isECS = TRUE;
    if (m_state == PARTYCHANGEMODE)
        m_state = PARTYCONNECTED;
    if (IsValidTimer(PARTYCHANGEMODETOUT))
    	DeleteTimer(PARTYCHANGEMODETOUT);
	m_pConfApi->SendPartyReceiveEcsToPartyControl(GetPartyId());
}

/////////////////////////////////////////////////////////////////////////////////////////
//HIGHEST COMMON FUNCTIONS:
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnConfChangeModeConnect(CSegment* pParam)
{
	TRACEINTO << "mix_mode: CONFCHANGEMODE arrived.";

	if (m_pRmtCapH323->IsECS())
	{
		PTRACE2(eLevelError, "CH323Party::OnConfChangeModeConnect ChangeMode recieved after ECS - Ignore! Name - ",PARTYNAME);
		return;
	}
    if (m_pH323Cntl->m_pmcCall->GetIsClosingProcess())
    {
		PTRACE2(eLevelError, "CH323Party::OnConfChangeModeConnect ChangeMode recieved when closing - Ignore! Name - ",PARTYNAME);
		return;
	}

	PTRACE2(eLevelInfoNormal,"CH323Party::OnConfChangeModeConnect : Name - ",PARTYNAME);

	m_state = PARTYCHANGEMODE;

	WORD tempWordForEnum;
	*pParam >> tempWordForEnum;
	if( (((short)tempWordForEnum) >= 0) && (tempWordForEnum < eLastChangeModeState) ) //check validity
		m_changeModeState = (eChangeModeState)tempWordForEnum;
	else
	{
		PTRACE2(eLevelError,"CH323Party::OnConfChangeModeConnect : Illegal change mode state Name - ",PARTYNAME);
		m_changeModeState = eNotNeeded;
	}

	TRACESTR(eLevelInfoNormal)<< "***CH323Party::OnConfChangeModeConnect : Change mode state = " << GetChangeModeStateStr(m_changeModeState) << ",  Name - " << PARTYNAME;

	if (m_isRecordingPort)
	{
		if (m_changeModeState == eChangeInAndReopenOut || m_changeModeState == eReopenInAndOut || m_changeModeState == eFlowControlInAndReopenOut)
			m_changeModeState = eReopenOut;
		else if (m_changeModeState == eChangeInAndOut)
			m_changeModeState = eChangeOutgoing;
		else if (m_changeModeState == eChangeContentInAndOut)
			m_changeModeState = eChangeContentOut;
		else if (m_changeModeState == eChangeIncoming || m_changeModeState == eReopenIn || m_changeModeState == eFlowControlIn || m_changeModeState == eChangeContentRate || m_changeModeState == eChangeContentIn)
			m_changeModeState = eNotNeeded;
		else if (m_changeModeState == eFlowControlInAndOut || m_changeModeState == eFlowControlOutAndChangeIn || m_changeModeState == eFlowControlOutAndReopenIn)
			m_changeModeState = eFlowControlOut;
	}

	if (m_changeModeState != eNotNeeded)
	{
		BYTE bIsLocalCapH323;
		*pParam >> bIsLocalCapH323;

		CComModeH323* pNewScm = new CComModeH323;
		pNewScm->DeSerialize(NATIVE,*pParam);
        	pNewScm->Dump("CH323Party::OnConfChangeModeConnect new mode", eLevelInfoNormal);
		m_pInitialModeH323->Dump("CH323Party::OnConfChangeModeConnect Initial mode", eLevelInfoNormal);
		m_pCurrentModeH323->Dump("CH323Party::OnConfChangeModeConnect Current mode", eLevelInfoNormal);

        if (m_pCurrentModeH323->GetConfType() == kCop)
        {
        	BYTE newCopLevel = pNewScm->GetCopTxLevel();
        	if (m_pCurrentModeH323->GetCopTxLevel() != newCopLevel)
            {
        		m_pCurrentModeH323->SetCopTxLevel(newCopLevel);
        		m_pH323Cntl->GetCurrentMode()->SetCopTxLevel(newCopLevel);
        		m_pH323Cntl->GetTargetMode()->SetCopTxLevel(newCopLevel);
        		PTRACE2INT(eLevelInfoNormal,"CH323Party::OnConfChangeModeConnect - Set current cop level to New scm level: ", newCopLevel);
                m_pH323Cntl->CancelLpr();
            }
        }

       	BYTE bNotNeedToChange = !bIsLocalCapH323 && IsNewModeEqualToCurMode(pNewScm) && (m_changeModeState!=eConfRequestMoveToMixed);//if we have the local caps, it's change CP mode, so the scm isn't important
		if (bNotNeedToChange)
		{
			PTRACE2(eLevelInfoNormal,"CH323Party::OnConfChangeModeConnect - New scm is equal to current mode - Name - ",PARTYNAME);
			SendEndChangeVideoToConfLevel();
			POBJDELETE(pNewScm);
			return;
		}

// 		CCapH323* pNewLocalCaps = NULL;
// 		if (bIsLocalCapH323)
// 		{
// 			pNewLocalCaps = new CCapH323;
// 			pNewLocalCaps->DeSerialize(NATIVE,*pParam);
// 		}


		RemoteIdent rmtIdent = m_pH323Cntl->GetRemoteIdent();
        if (m_changeModeState == eChangeOutgoing && m_pCurrentModeH323->IsMediaOff(cmCapVideo, cmCapTransmit))
            m_changeModeState =  eReopenOut;
        else if (m_changeModeState == eChangeInAndOut && m_pCurrentModeH323->IsMediaOff(cmCapVideo, cmCapTransmit))
            m_changeModeState = eChangeInAndReopenOut;
		switch (m_changeModeState)
		{
			case eReopenOut:
            case eFlowControlInAndReopenOut:
			{
				WORD differentValue = 0;
				//Target mode had been update via the new capabilities.
				differentValue = m_pH323Cntl->GetChangeFromNewScmMode(pNewScm,cmCapTransmit,cmCapVideo);
				differentValue &= DETAILS_WITHOUT_FORMAT;// Gets the details without the format
			}
			case eChangeIncoming:
			case eChangeInAndReopenOut:
			case eReopenIn:
			case eReopenInAndOut:
			case eFlowControlOutAndChangeIn:
			case eFlowControlOutAndReopenIn:
			case eChangeOutgoing:
            case eChangeInAndOut:
			{
				if(rmtIdent == NetMeeting)
				{
					//In case of netmeeting and the change it because we need to change the incoming from cif to Qcif
					//we should change either the outgoing to Qcif because of net meeting problem: she can;t receive qcif on
					//cif channel.
					WORD differentValue = 0;
					//Target mode had been update via the new capabilities.
					differentValue = m_pH323Cntl->GetChangeFromNewScmMode(pNewScm,cmCapReceive,cmCapVideo);
					differentValue &= DETAILS_WITHOUT_FORMAT;// Gets the details without the format

					if(differentValue == HIGHER_FORMAT)
					{
						if(m_changeModeState == eChangeIncoming)
							m_changeModeState = eChangeInAndReopenOut;
						else if(m_changeModeState == eReopenIn)
							m_changeModeState = eReopenInAndOut;
					}
				}
				cmCapDirection direction;
				GetDirectionByChangeModeState(m_changeModeState, direction);

				BYTE bUpdateSucceeded = m_pH323Cntl->UpdateTargetMode(pNewScm, cmCapVideo, direction);
				if (bUpdateSucceeded == FALSE)
				{
					m_pCurrentModeH323->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit);
					SendEndChangeVideoToConfLevel(statIllegal);
					POBJDELETE(pNewScm);
					return;
				}

				//updating local caps in h323cntl only in vase we need to change receive or receive + transmit
				//if (direction != cmCapTransmit)
				//	m_pH323Cntl->BuildNewCapsFromNewTargetModeAndCaps(m_pH323Cntl->m_pLocalCapH323,pNewLocalCaps);

				//Start timer:
				DWORD ticks = PARTY_CHANGEVIDEO_TIME;
				if  (IsChangeIncomingState())
				{
				 //If the change mode arrives before the party connects to the video bridge,
				 //no connect_ts has been sent, and therefore the RTP can't report if there
				 //was a violation. Consequently, we don't wait for this information, and only
				 //give the chance to the ep to reopen the channel.
				 //If the Call is EPC one and the EP open content we don't decrease the timer
				 //(according to iPower behavior that also close the content channel)
					if ((! IsH239Conf()	&&
						  m_pH323Cntl->IsChannelConnected(cmCapVideo,cmCapReceive,kRoleContentOrPresentation) &&
						  m_pH323Cntl->IsChannelConnected(cmCapVideo,cmCapTransmit,kRoleContentOrPresentation)))
							ticks = 3*SECOND;
				}
				StartTimer(PARTYCHANGEMODETOUT, ticks);
				// change decoder:
				if ((m_changeModeState != eChangeOutgoing) && (m_changeModeState != eReopenOut))
				{
					m_bIsVideoCapEqualScm = TRUE; // When cop change decoder, the local video caps are set to one video cap according to scm. Should not return the removed video caps when sending re-cap for content change mode.
					if( m_pCurrentModeH323->GetConfType()== kCop )
                                            m_pH323Cntl->UpdateLocalCapsFromTargetMode(m_bIsVideoCapEqualScm);
					else
					    m_pH323Cntl->UpdateLocalCapsFromTargetMode(FALSE);
					POBJDELETE(m_pLocalCapH323);
					m_pLocalCapH323 = new CCapH323(*m_pH323Cntl->m_pLocalCapH323);

                    // Send new Re-Cap
                    StartReCapsProcess();



					// Update current recv mode with the new target recv mode before end change mode. (we don't wait to remote reopen the channel, because remote doesn't have to do so).
					WORD currMediaType = m_pH323Cntl->GetCurrentMode()->GetMediaType(cmCapVideo,cmCapReceive,kRolePeople);
					WORD targetMediaType = m_pH323Cntl->GetTargetMode()->GetMediaType(cmCapVideo,cmCapReceive,kRolePeople);
					if(currMediaType == targetMediaType)
						UpdateCurrentRcvModeAccordingToTarget();

					// Send end change mode:
					if (m_changeModeState == eChangeIncoming)
					{
						SendEndChangeVideoToConfLevel();
						POBJDELETE(pNewScm);
					}
				}


				if((m_changeModeState == eReopenOut) || (m_changeModeState == eChangeInAndReopenOut) ||
                   (m_changeModeState == eReopenInAndOut) || (m_changeModeState == eFlowControlInAndReopenOut))
				{//Close outgoing channel:
					TRACEINTO << "Close the channel m_changeModeState=" << m_changeModeState;
					DWORD newVidRate = pNewScm->GetMediaBitRate(cmCapVideo, cmCapTransmit);
					if (newVidRate != m_videoRate)
					{//in cop it can happen in case we switch from different protocols, and they have differnet rates:
						m_videoRate = newVidRate;
					}

					//In case we need to change or reopen the incoming, but also to reopen
					//the outgoing, we wait for the out channel to reconnect and only then we
					//send re-caps
					/*BYTE bIsTransmitting = TRUE;
					if (m_pH323Cntl->FindChannelInList(cmCapVideo, bIsTransmitting, kRolePeople))
                    {
                        DWORD newVidRate = pNewScm->GetMediaBitRate(cmCapVideo, direction);
                        if (m_pH323Cntl->OnConfFlowControlReq(newVidRate, FALSE))
                        {
                            m_pH323Cntl->OnConfOrPartyDisconnectMediaChannel(cmCapVideo, cmCapTransmit, kRolePeople);
                            m_pCurrentModeH323->SetVideoBitRate(newVidRate,direction);
                        }

                        else
                            SendEndChangeVideoToConfLevel(statIllegal);
                    }*/
                    if (m_changeModeState == eFlowControlInAndReopenOut)
                    {
                    	m_pH323Cntl->OnConfFlowControlReq(newVidRate, FALSE);//flow control to video in
                    	m_pCurrentModeH323->SetVideoBitRate(newVidRate, cmCapReceive);
                    }

                    CChannel* pOutChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRolePeople);
                    if (pOutChannel)
                    {
                    	PTRACE2INT (eLevelInfoNormal, "CH323Party::OnConfChangeModeConnect - closing the channel. channel state = ", pOutChannel->GetCsChannelState());
                    	if (pOutChannel->GetCsChannelState() == kConnectedState)
	                    	m_pH323Cntl->OnConfOrPartyDisconnectMediaChannel(cmCapVideo, cmCapTransmit, kRolePeople);
                    }

					else // video out is closed
					{	//upgrade from secondary, but the incoming channel wasn't closed properly
						/*bIsTransmitting = FALSE;
						BYTE bVideoInIsOpened = (m_pH323Cntl->FindChannelInList(cmCapVideo, bIsTransmitting, kRolePeople) != NULL);
						if (bVideoInIsOpened)
						{//we se the video to off when we downgrade to secondary, and now we need to turn is to on
							CComModeH323* pCurrMode = m_pH323Cntl->GetCurrentMode();
							const CMediaModeH323& rcvCurrMode = pCurrMode->GetMediaMode(cmCapVideo, cmCapReceive);
							if ( rcvCurrMode.IsMediaOn() )
								m_pCurrentModeH323->SetMediaMode(rcvCurrMode, cmCapVideo, cmCapReceive);
						}*/

						//StartReCapsProcess();//to force the endpoint to re-open the incoming

						/*if( (m_changeModeState == eReopenOut) || (m_changeModeState == eReopenInAndOut) ||
						   (m_changeModeState == eChangeInAndReopenOut) ||
						   bVideoInIsOpened || (m_changeModeState == eFlowControlInAndReopenOut))*/
							m_pH323Cntl->OpenVideoOutChannelFromMcms();
                            ReActivateIncomingChannelIfNeeded();
					}
				}

				//Change or reopen only the incoming:
				//else if( (m_changeModeState == eChangeIncoming) || (m_changeModeState == eReopenIn) ||
                //    (m_changeModeState == eFlowControlOutAndChangeIn) || (m_changeModeState == eFlowControlOutAndReopenIn))
				//	StartReCapsProcess();
                else if (m_changeModeState == eChangeOutgoing || m_changeModeState == eChangeInAndOut)
                {

                    CChannel *pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRolePeople);
                    if (pChannel)
                    {
                        if (m_pH323Cntl->UpdateVideoOutgoingChannelAccordingToTargetScm())
                        {
                            m_pH323Cntl->UpdateOutStream(kRolePeople);
                            UpdateCurrentTxModeAccordingToTarget();
                        }
                        else
                        {
                            PTRACE2(eLevelError, "CH323Party::OnConfChangeModeConnect Failed to Update channel params,  Name - ", PARTYNAME);
                            SendEndChangeVideoToConfLevel(statIllegal);
                        }

                    }
                }
        /*        if(m_changeModeState == eChangeInAndOut || m_changeModeState == eChangeIncoming)
                {
                	if (bIsLocalCapH323)
                	{
                		// update party caps
                		POBJDELETE(m_pLocalCapH323);
                		m_pLocalCapH323	= new CCapH323;
                		m_pLocalCapH323->DeSerialize(NATIVE,*pParam);

                		// update H323Cntl caps
                		POBJDELETE(m_pH323Cntl->m_pLocalCapH323);
                		m_pH323Cntl->m_pLocalCapH323 = new CCapH323(*m_pLocalCapH323);

                		// Send new Re-Cap
                		StartReCapsProcess();
                	}

                	if(m_changeModeState == eChangeInAndOut)
                	{

                		CChannel *pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRolePeople);
                		if (pChannel)
                		{
                		      if (m_pH323Cntl->UpdateVideoOutgoingChannelAccordingToTargetScm())
                		         m_pH323Cntl->UpdateOutStream(kRolePeople);
 		                      else
							    {
									  PTRACE2(eLevelError, "CH323Party::OnConfChangeModeConnect Failed to Update channel params,  Name - ", PARTYNAME);
									  SendEndChangeVideoToConfLevel(statIllegal);
							    }
                		}

                   		else
                			DBGPASSERT(1);
                	}
               	// Update current recv mode with the new target recv mode before end change mode. (we don't wait to remote reopen the channel, because remote doesn't have to do so).
                	WORD currMediaType = m_pH323Cntl->GetCurrentMode()->GetMediaType(cmCapVideo,cmCapReceive,kRolePeople);
                	WORD targetMediaType = m_pH323Cntl->GetTargetMode()->GetMediaType(cmCapVideo,cmCapReceive,kRolePeople);

                	if(currMediaType == targetMediaType)
                		UpdateCurrentRcvModeAccordingToTarget();
              }
*/
				//flow control out:
				if ((m_changeModeState == eFlowControlOutAndChangeIn) || (m_changeModeState == eFlowControlOutAndReopenIn))
				{
					DWORD newVidRate = pNewScm->GetMediaBitRate(cmCapVideo, cmCapTransmit);
					BYTE bFlowControlSucceeded = TRUE;
					bFlowControlSucceeded &= m_pH323Cntl->OnConfFlowControlReq(newVidRate, TRUE);
					if (bFlowControlSucceeded)
					{
						m_pCurrentModeH323->SetVideoBitRate(newVidRate,cmCapTransmit);
						//16.08.2006 Changes by VK. Bug VNGM-947. Difference between current transmit rate at Control and Party
						(m_pH323Cntl->m_pCurrentModeH323)->SetVideoBitRate(newVidRate, cmCapTransmit);
					}
				}

				break;
			}

			case eFlowControlIn:
			case eFlowControlOut:
			case eFlowControlInAndOut:
			{
				cmCapDirection direction;
				GetDirectionByChangeModeState(m_changeModeState, direction);

				DWORD newVidRate = pNewScm->GetMediaBitRate(cmCapVideo, direction);
				BYTE bFlowControlSucceeded = TRUE;
				if (direction & cmCapReceive)
					bFlowControlSucceeded = m_pH323Cntl->OnConfFlowControlReq(newVidRate, FALSE);
				if (direction & cmCapTransmit)
					bFlowControlSucceeded &= m_pH323Cntl->OnConfFlowControlReq(newVidRate, TRUE);

				if (bFlowControlSucceeded)
					m_pCurrentModeH323->SetVideoBitRate(newVidRate,direction);

				EStat eStatus = bFlowControlSucceeded ? statOK : statIllegal;
                SendEndChangeVideoToConfLevel(eStatus);
				break;
			}
			case eConfRequestMoveToMixed:
			{
				UpgradeAvcToMixed(pParam, pNewScm);
				break;
			}
			default:
			    TRACESTR(eLevelInfoNormal)<<" CH323Party::OnConfChangeModeConnect - Illegal change mode state = " << m_changeModeState << ",  Name - " << PARTYNAME;
		}

		POBJDELETE(pNewScm);
		//POBJDELETE(pNewLocalCaps);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsNewModeEqualToCurMode(CComModeH323* pNewScm)
{
	if (m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRolePeople) == NULL)
		return FALSE;	//upgrade from secondary

	BYTE bIsEqualToCurMode = TRUE;


	const CMediaModeH323& rcvCurMode  = m_pCurrentModeH323->GetMediaMode(cmCapVideo, cmCapReceive);
	const CMediaModeH323& rcvNewMode  = pNewScm->GetMediaMode(cmCapVideo, cmCapReceive);

	bIsEqualToCurMode &= (rcvCurMode == rcvNewMode);

	const CMediaModeH323& transCurMode = m_pCurrentModeH323->GetMediaMode(cmCapVideo, cmCapTransmit);
	const CMediaModeH323& tansNewMode  = pNewScm->GetMediaMode(cmCapVideo, cmCapTransmit);

	bIsEqualToCurMode &= (transCurMode == tansNewMode);

	return bIsEqualToCurMode;
}

/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsNewContentModeEqualToCurContentMode(CComModeH323* pNewScm, cmCapDirection direction)
{
	BYTE bIsTransmitDirection = direction & cmCapTransmit;
	if (m_pH323Cntl->FindChannelInList(cmCapVideo, bIsTransmitDirection, kRoleContentOrPresentation) == NULL)
		return FALSE;	//upgrade from secondary

	BYTE bIsEqualToCurMode = TRUE;

	if(direction & cmCapReceive)
	{
		const CMediaModeH323& rcvCurMode  = m_pCurrentModeH323->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);
		const CMediaModeH323& rcvNewMode  = pNewScm->GetMediaMode(cmCapVideo, cmCapReceive, kRoleContentOrPresentation);

		bIsEqualToCurMode &= (rcvCurMode == rcvNewMode);
	}

	if(direction & cmCapTransmit)
	{
		const CMediaModeH323& transCurMode = m_pCurrentModeH323->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);
		const CMediaModeH323& tansNewMode  = pNewScm->GetMediaMode(cmCapVideo, cmCapTransmit, kRoleContentOrPresentation);

		bIsEqualToCurMode &= (transCurMode == tansNewMode);
	}

	return bIsEqualToCurMode;
}

/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsPartyInChangeVideoMode() const
{
	return( (m_state == PARTYCHANGEMODE) && (m_changeModeState != eNotNeeded) );
}
/*
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsPartyInChangeCopMode() const
{
	return IsPartyInChangeVideoMode() && IsPartyInConfOnPort() && (m_changeModeState == eReopenOut);
}
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsPartyInChangeCopModeOfOutgoing() const
{
	return IsPartyInChangeVideoMode() && IsPartyInConfOnPort() &&
		((m_changeModeState == eReopenOut) || (m_changeModeState == eChangeInAndReopenOut) ||
         (m_changeModeState == eReopenInAndOut) || (m_changeModeState == eFlowControlInAndReopenOut);
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsPartyInChangeCpVideoMode() const
{
	/*return ( (m_pInitialModeH323->GetConfType() == kCp) && IsPartyInChangeVideoMode()  &&
		    !IsPartyInChangeCopMode() );*/
    return ( (m_pInitialModeH323->GetConfType() == kCp) && IsPartyInChangeVideoMode());
}

/////////////////////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsChangeIncomingState() const
{
//	BYTE bRes = FALSE;
//	if ( (m_changeModeState == eChangeIncoming) || (m_changeModeState == eChangeInAndReopenOut)
//		|| (m_changeModeState == eFlowControlOutAndChangeIn))
//		bRes = TRUE;
	return IsChangeModeOfIncomingState(m_changeModeState);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::StartReCapsProcess()
{
	m_pH323Cntl->OnPartyCreateControl(TRUE); //TRUE=> this is re-cap
/*	CChannel* pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo,FALSE,kRolePeople); //incoming channel
	if (pChannel)
	{
		 if ((m_changeModeState != eReopenIn) && (m_changeModeState != eReopenInAndOut) )
			 m_pH323Cntl->UpdateRateIfNeeded(pChannel,TRUE); //will be sent when the new channel is opened
	}*/ //Carmel - no defense rate

	BYTE bSendNewModeReq = TRUE;
	if ((((m_pInitialModeH323->GetConfType() == kCp) || (m_pInitialModeH323->GetConfType() == kCop))
													&& ((m_changeModeState == eReopenIn) ||
														(m_changeModeState == eReopenInAndOut) ||
														(m_changeModeState == eChangeIncoming) ||
														(m_changeContentModeState == eChangeContentOut))))
		bSendNewModeReq = FALSE;//in this case we don't know what is the requested mode, because the scm here is only a recommendation. However, if the incoming channel is reopened, the stack automatically recognize the new mode

	if (bSendNewModeReq)
		m_pH323Cntl->OnPartyNewChannelModeReq();
}

//////////////////////////////////////////////////////////////////////////////////////////////
//Description: This function is been called in the following caese:
//1) The remote rejected the openning of the new channel
//2) The remote rejected the closing of the old channel
void CH323Party::OnH323LogicalChannelRejectChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelRejectChangeMode. Name - ",PARTYNAME);

	WORD channelType;
	WORD channelDirection;
	WORD roleLabel;

	*pParam >> channelType;
	*pParam >> channelDirection;
	*pParam >> roleLabel;

	if (channelType != cmCapVideo)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelRejectChangeMode - not a video channel. Name - ",PARTYNAME);
		return;
	}

	else if (roleLabel != kRolePeople)
	{
		PTRACE2(eLevelInfoNormal,"CH323Party::OnH323LogicalChannelRejectChangeMode - not a people channel. Name - ",PARTYNAME);
		return;
	}
	cmCapDirection changeModeDirection;
	if (GetDirectionByChangeModeState(m_changeModeState, changeModeDirection) == FALSE)
		PTRACE2(eLevelError, "CH323Party - Incorrect change mode state - Name ", PARTYNAME);
	else if (changeModeDirection != channelDirection)
		PTRACE2(eLevelError, "CH323Party::OnH323LogicalChannelRejectChangeMode - a problem with the other direction. Name - ",PARTYNAME);
	else
		SendEndChangeVideoToConfLevel(statIllegal);
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnTimerChangeModeTimeOut(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnTimerChangeModeTimeOut. Name - ",PARTYNAME);

	EStat eStatus = statIllegal;

	//Stream status not have to come so in case the receive channel is ON we should copy the target to current.
	//We are doing that because the remote did not do any crime and we do not need to change the incomin yet.
	//In case she we do a crime - stream status will occur then we will do changes.
	//If the change mode arrives before the party connects to the video bridge,
	//no connect_ts has been sent, and therefore the RTP can't report if there
	//was a violation. Consequently, we won't get the information from the card .
	//The channel status is also check to insure that the incoming video is connected

	if (m_pCurrentModeH323->IsMediaOn(cmCapVideo,cmCapReceive,kRolePeople))
	{
		WORD currMediaType = m_pH323Cntl->GetCurrentMode()->GetMediaType(cmCapVideo,cmCapReceive,kRolePeople);
		WORD targetMediaType = m_pH323Cntl->GetTargetMode()->GetMediaType(cmCapVideo,cmCapReceive,kRolePeople);

		if(currMediaType == targetMediaType)
		{
			UpdateCurrentRcvModeAccordingToTarget();
			eStatus = statOK;
			//m_pH323Cntl->SendBrqIfNeeded();//it is needed since we didn't passed through ConnectPartyToConf SOAK W6
		}
	}

	SendEndChangeVideoToConfLevel(eStatus);
}

//////////////////////////////////////////////////////////////////////////////////
void CH323Party::SendEndChangeVideoToConfLevel(EStat eStatus,WORD reason,CSecondaryParams *pSecParamps)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendEndChangeVideoToConfLevel. Name - ",PARTYNAME);
	DeleteTimer(PARTYCHANGEMODETOUT);

	m_status = eStatus;
	m_state  = PARTYCONNECTED;
	m_changeModeState = eNotNeeded;

	// In case there content is ON we should tell the conference level the content rate even if this party is not
	// the speaker (and the real rate is zero).
	// we create a temporary communication mode and set it's video content bit rate
	// to the values include tdm because these are the values we received from the conference level.
	CComModeH323* pTmpModeH323 = new CComModeH323;
	*pTmpModeH323 = *m_pCurrentModeH323;
	// in EPC we set mode only if outgoing content is connected
	if( (m_pH323Cntl->GetCurrentMode()->IsMediaOn(cmCapVideo,cmCapReceive,kRoleContentOrPresentation)))
	{
		DWORD currConfContRate = m_pH323Cntl->GetCurConfContRate();
		pTmpModeH323->SetVideoBitRate(currConfContRate,cmCapReceive,kRoleContentOrPresentation);
	}

	m_pConfApi->PartyEndChangeModeIp(GetPartyId(), *pTmpModeH323, m_status,reason,pSecParamps);
	m_pH323Cntl->UpdateRtpWithLprInfo();

	POBJDELETE (pTmpModeH323);

	if (m_status == statOK)
	{
		StartTimer(VCUTOUT,10*SECOND);
		m_lastVcuTime = 0; //in order to send fast update requet, when the bridge will send video refresh to the party, as a result of re-opening the bridge.
	}
}

//////////////////////////////////////////////////////////////////////////////////
void CH323Party::UpdateCurrentRcvModeAccordingToTarget()
{
	if (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapReceive))
	{
		//We want to update every thing except bit rate. in cop we will update the bitrate too.
		DWORD bitRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo,cmCapReceive);
		CComModeH323* pTargetMode = m_pH323Cntl->GetTargetMode();
		const CMediaModeH323& rcvTargetMode = pTargetMode->GetMediaMode(cmCapVideo, cmCapReceive);
		m_pCurrentModeH323->SetMediaMode(rcvTargetMode, cmCapVideo, cmCapReceive);


		//if we set in h323part, we should set in h323cntl as well.
		CComModeH323* pH323CntlCurrMode = m_pH323Cntl->GetCurrentMode();
		pH323CntlCurrMode->SetMediaMode(rcvTargetMode, cmCapVideo, cmCapReceive);
        if (m_pCurrentModeH323->GetConfType() != kCop)
        {
            m_pCurrentModeH323->SetVideoBitRate(bitRate);
		pH323CntlCurrMode->SetVideoBitRate(bitRate);
	}

		PTRACE2INT(eLevelInfoNormal,"CH323Party::UpdateCurrentRcvModeAccordingToTarget - : bitRate ",bitRate);
	}
	else
		PTRACE2(eLevelInfoNormal,"CH323Party::UpdateCurrentRcvModeAccordingToTarget - receive current mode is off, so it won't be updated : Name - ",PARTYNAME);
}
//////////////////////////////////////////////////////////////////////////////////
void CH323Party::UpdateCurrentTxModeAccordingToTarget()
{
	if (m_pCurrentModeH323->IsMediaOn(cmCapVideo, cmCapReceive))
	{
		CComModeH323* pTargetMode = m_pH323Cntl->GetTargetMode();
		const CMediaModeH323& txTargetMode = pTargetMode->GetMediaMode(cmCapVideo, cmCapTransmit);
		m_pCurrentModeH323->SetMediaMode(txTargetMode, cmCapVideo, cmCapTransmit);


		//if we set in h323part, we should set in h323cntl as well.
		CComModeH323* pH323CntlCurrMode = m_pH323Cntl->GetCurrentMode();
		pH323CntlCurrMode->SetMediaMode(txTargetMode, cmCapVideo, cmCapTransmit);
	}
	else
		PTRACE2(eLevelError,"CH323Party::UpdateCurrentTxModeAccordingToTarget - transmit current mode is off, so it won't be updated : Name - ",PARTYNAME);
}

//////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323StreamViolationChangeMode(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323StreamViolationChangeMode : Name - ",PARTYNAME);
	WORD cardStatus;
	WORD reason;
	CSecondaryParams secParamps;
	*pParam >> cardStatus >> reason;
	secParamps.DeSerialize(NATIVE,*pParam);

	if (m_changeModeState != eNotNeeded)
	{
		EStat eStatus = (cardStatus == 0) ? statOK : statIncopatibleVideo;

		if (eStatus == statOK)
		{
			if ( (m_pInitialModeH323->GetConfType() == kCp) &&
				( (m_changeModeState == eReopenIn) || (m_changeModeState == eReopenInAndOut)
                  || (m_changeModeState == eReopenOut) || (m_changeModeState == eFlowControlInAndReopenOut)))
				return; //in this case, the channel has to be closed, so we ignore the stream violation

			WORD currMediaType   = m_pH323Cntl->GetCurrentMode()->GetMediaType(cmCapVideo, cmCapReceive);
			WORD targetMediaType = m_pH323Cntl->GetTargetMode()->GetMediaType(cmCapVideo, cmCapReceive);

			if(currMediaType == targetMediaType)
			{
				UpdateCurrentRcvModeAccordingToTarget();

				//In case we need to reopen the out, we can sent "end change mode" to cont level,
				//only if the out is already reopened.
				if( (m_changeModeState == eChangeInAndReopenOut) || (m_changeModeState == eReopenInAndOut) )
				{
				    TRACESTR(eLevelInfoNormal)<< " CH323Party::OnH323StreamViolationChangeMode : Need to close outgoing. m_changeModeState = "
										  << GetChangeModeStateStr(m_changeModeState) << ",  Name - " << PARTYNAME;
					m_changeModeState = eReopenOut;
					return;
				}
			}
			else
			{
				PTRACE2(eLevelInfoNormal,"CH323Party::OnH323StreamViolationChangeMode: Types are different - ignore message,  Name - ", PARTYNAME);
				return;
			}
		}

		SendEndChangeVideoToConfLevel(eStatus,reason,&secParamps);
	}

	else //this is change content mode process
		SetPartyToSecondary(reason,&secParamps);

}

//////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323StreamViolationConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnH323StreamViolationConnect : Name - ",PARTYNAME);

	WORD cardStatus;
	WORD reason;
	CSecondaryParams secParamps;
	*pParam >> cardStatus >> reason;
	secParamps.DeSerialize(NATIVE,*pParam);

	if (cardStatus)
		SetPartyToSecondary(reason,&secParamps);
}

//////////////////////////////////////////////////////////////////////////////////
void CH323Party::SetPartyToSecondary(WORD reason, CSecondaryParams* pSecParamps)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", Reason:" << reason;

	m_pH323Cntl->OnConfOrPartyDisconnectMediaChannel(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
	m_pH323Cntl->OnConfOrPartyDisconnectMediaChannel(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

	m_pCurrentModeH323->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);  // we do it here, because when we will get a new scm, before all the video channels have disconnected, we need to send in the answer to the conf, the current mode without the video

	m_pConfApi->PartyMoveToSecondery(GetPartyId(), reason, pSecParamps);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::SendReCapsToPartyControl(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendReCapsToPartyControl : Name - ",PARTYNAME);
	m_pRmtCapH323->DeSerialize(NATIVE,*pParam);
    if ((m_pRmtCapH323->GetNumOfVideoCap() == 0) && (m_state == PARTYCHANGEMODE))
        m_state = PARTYCONNECTED;
	CSegment rmtCapSeg;
	m_pRmtCapH323->SerializeCapArrayOnly(rmtCapSeg,TRUE);
	m_pConfApi->UpdateDB(this,RMOT323CAP,(DWORD) 0,1,&rmtCapSeg);

	m_pConfApi->SendPartyReceiveReCapsToPartyControl(GetPartyId(), m_pRmtCapH323);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::EndH323Disconnect(WORD isRejected)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::EndH323Disconnect : Name - ",PARTYNAME);
	if (isRejected) // In this case we allocated only the CS rsrcDesc therefore we deallocate onlt it.
		m_pH323Cntl->RemoveCsControllerResource();
	else
    	m_pH323Cntl->RemoveFromRsrcTbl();

	DeleteAllTimers();
	StopAllPreviews();
	m_pH323Cntl->Suspend(); //ingore all events
	PartySelfKill();
}

/*
/////////////////////////////////////////////////////////////////////////////////////////
//Right now, until we'll have a change mode for Data, changing data mode is only by turning it from on to off
void CH323Party::OnConfChangeDataMode(CSegment* pParam)
{
	CComModeH323* pNewScm = new CComModeH323;
	*pNewScm = *m_pCurrentModeH323;
	pNewScm->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);
	m_pH323Cntl->UpdateTargetMode(pNewScm, cmCapData);
	POBJDELETE(pNewScm);
}
*/
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnConfSetMoveParams(CSegment* pParam)
{
	WORD confType, mcuNumber, terminalNumber;

	WORD tempWordForEnumConfType;
	BYTE bIsCopVideoTxModes = FALSE;
    BYTE bIsOpPointsSet = FALSE;
	*pParam >> tempWordForEnumConfType
			>> m_mcuNum
			>> m_termNum
			>> bIsCopVideoTxModes;

	if (bIsCopVideoTxModes)
	{
		CCopVideoTxModes* pTempCopVideoTxModes = new CCopVideoTxModes;
		pTempCopVideoTxModes->DeSerialize(NATIVE,*pParam);
		m_pH323Cntl->SetCopVideoTxModes(pTempCopVideoTxModes);
		POBJDELETE(pTempCopVideoTxModes);
	}
    *pParam >> bIsOpPointsSet;
    if (bIsOpPointsSet)
    {
        CVideoOperationPointsSet* pVideoOperationPointsSet = new CVideoOperationPointsSet;
        pVideoOperationPointsSet->DeSerialize(*pParam);
        m_pInitialModeH323->SetOperationPoints(pVideoOperationPointsSet);
        m_pCurrentModeH323->SetOperationPoints(pVideoOperationPointsSet);
        POBJDELETE(pVideoOperationPointsSet);
	}

	// init 'speaker' params upon move, so FECC won't be operated on a wrong participant
	//      (the correct params will be updated on the next 'VIN' message).
	m_pH323Cntl->InitSpeakerParams();

	m_pH323Cntl->m_pTargetModeH323->SetConfType((EConfType)tempWordForEnumConfType);
	m_pInitialModeH323->SetConfType((EConfType)tempWordForEnumConfType);

	m_pH323Cntl->SendRemoteNumbering();

    const CCommConf*  pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
    const char* sConfNumeriID =pCommCurrConf->GetNumericConfId();
    m_pH323Cntl->SendAVFFacilityEndOfMove (sConfNumeriID);
}

////////////////////////////////////////////////////////////////////////////////
//////Chair Control/////////////////

//////////////////////////////////////////////////////////////////////////////////////////
//Receive from chair control bridge - in cascade mode when the party in one of the links.
//Need to update the link about the terminal and MCU number that the chair control assignment.

void CH323Party::OnBridgeNumberingMessage(CSegment* pParam)
{
    // vngr-7017 "bombing" logger on change layout - change trace to DEBUG level
	PTRACE2(eLevelInfoNormal,"CH323Party::OnBridgeNumberingMessage: Name - ",PARTYNAME);

	OPCODE opcode;
	WORD mcuNum, terminalNum;
	PartyRsrcID partyId;

	*pParam >> opcode >> mcuNum >> terminalNum >> partyId;

	switch(opcode)
	{
		case SEND_VIN:
		{
			m_pH323Cntl->SendConferenceIndReq(opcode, mcuNum, terminalNum, partyId);
			TRACEINTOFUNC << "mcuNum: " << mcuNum << ", terminalNum: " << terminalNum << ", partyId: " << partyId;
			break;
		}

		default:
		{
		    TRACESTR(eLevelInfoNormal)<< " CH323Party::OnBridgeNumberingMessage, No such opcode = " << opcode << ",  Name - " << PARTYNAME;
		    break;
		}
	}
}

//////////////////////////////////////////////////////////////////
/////////////Data Token///////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnFeccBridgeTokenRequest(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal,"CH323Party::OnFeccBridgeTokenRequest : Name - ",PARTYNAME);
	m_pH323Cntl->OnPartyFeccTokenReq(kTokenRequest);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnFeccBridgeTokenAccept(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal,"CH323Party::OnFeccBridgeTokenAccept : Name - ",PARTYNAME);
 	WORD isCameraControl = 0;
 	*pParam >> isCameraControl;
	m_pH323Cntl->OnPartyFeccTokenReq(kTokenAccept,isCameraControl);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnFeccBridgeTokenReject(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal,"CH323Party::OnFeccBridgeTokenReject : Name - ",PARTYNAME);
	m_pH323Cntl->OnPartyFeccTokenReq(kTokenReject);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnFeccBridgeTokenWithdraw(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal,"CH323Party::OnFeccBridgeTokenWithdraw : Name - ",PARTYNAME);
	m_pH323Cntl->OnPartyFeccTokenReq(kTokenWithdrow);
}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnFeccBridgeTokenReleaseRequest(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal,"CH323Party::OnFeccBridgeTokenReleaseRequest : Name - ",PARTYNAME);
	m_pH323Cntl->OnPartyFeccTokenReq(kTokenReleaseRequest);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnFeccBridgeTokenRelease(CSegment* pParam)
{
 	PTRACE2(eLevelInfoNormal,"CH323Party::OnDatBrdgDatTokenRelease : Name - ",PARTYNAME);
	m_pH323Cntl->OnPartyFeccTokenReq(kTokenRelease);
}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnIpDataTokenMsg(CSegment* pParam)
{
	WORD msgType, bitRate, isCameraControl;

	*pParam >> msgType >> bitRate >> isCameraControl;

	switch(msgType)
	{
		case kTokenRequest:
			PTRACE2(eLevelInfoNormal,"CH323Party::OnIpDataTokenMsg - request : Name - ",PARTYNAME);
			m_pConfApi->DataTokenRequest(this, bitRate,isCameraControl);
			break;
		case kTokenRelease:
			PTRACE2(eLevelInfoNormal,"CH323Party::OnIpDataTokenMsg - release : Name - ",PARTYNAME);
			m_pConfApi->DataTokenRelease(this, isCameraControl);
			break;
		default:
			PTRACE2INT(eLevelInfoNormal,"CH323Party::OnIpDataTokenMsg - not handle message type : Name - ",msgType);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnIpFeccKeyMsg(CSegment* pParam)
{
	WORD msgType;

	*pParam >> msgType;
	CSmallString cstr;
	cstr << "CH323Party::OnIpFeccKeyMsg KEY = " << ::feccKeyToString((feccKeyEnum)msgType) << " , party: " << PARTYNAME;
	PTRACE(eLevelInfoNormal,cstr.GetString());

	m_pConfApi->FeccKeyMsg(GetPartyRsrcID(), msgType);

}
/////////////////////////////////////////////////////////////////////////////
BYTE CH323Party::AllocateLocalHalfKey(WORD isDialOut)
{	
	PTRACE2(eLevelInfoNormal,"CH323Party::AllocateLocalHalfKey : Name - ",PARTYNAME);
		
	
	EncyptedSharedMemoryTables 	*pEncyptedSharedMemoryTables= ((CConfPartyProcess*) CProcessBase::GetProcess())->GetEncryptionKeysSharedMemory();;
		
	FPASSERTMSG_AND_RETURN_VALUE((pEncyptedSharedMemoryTables == NULL),"Failed getting pEncyptedSharedMemoryTables" ,statIllegal);
			
	EncryptedKey encryptedKey; 
	
	DWORD generatorId = m_pH323Cntl->m_pDHKeyManagement->GetGenerator();
		
	STATUS statusQ = pEncyptedSharedMemoryTables->DequeuEncryptedKey(generatorId, encryptedKey);
	
	if (statusQ != STATUS_OK)
	{
		PASSERTSTREAM(true, "Failed to get key from shared memory for generator " << generatorId);
		return statIllegal;
	}
		
	// TRACESTR(eLevelInfoNormal) << " After getting key for generator " << generatorId << " KEY " << encryptedKey;
	
	WORD fips140Status = statOK;
	
	m_pH323Cntl->m_pDHKeyManagement->InitLocalHalfKey(encryptedKey);	
		
	return statOK;
}


/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnPartyUpdateConfRsrcIdForInterface(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnPartyUpdateConfRsrcIdForInterface : Name - ",PARTYNAME);
	DWORD confRsrcId;

	*pParam >> confRsrcId;
	m_ConfRsrcId = confRsrcId;
	m_pH323Cntl->SetNewConfRsrcId(confRsrcId);
	//Update GK Manager
	CSegment* seg = new CSegment;
	*seg << (DWORD)m_pH323Cntl->GetConnectionId() << (DWORD)confRsrcId;
	m_pH323Cntl->SendReqToGkManager(seg, PARTY_GK_UPDATE_CONF_ID);

}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnSendFaultyMfaToPartyCntl(CSegment *pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnSendFaultyMfaToPartyCntl : Name - ",PARTYNAME);
	DWORD reason = 0;
	BYTE 	mipHwConn=eMipNoneHw;
	if (pParam)
	{
		//===============================================
		// Not all parameters are assured to be present
		//===============================================
		pParam->ReadAlign(sizeof(reason));
		if (pParam-> GetWrtOffset() >= pParam -> GetRdOffset() + sizeof(reason)) 	*pParam >> reason;
		pParam->ReadAlign(sizeof(mipHwConn));
		if (pParam-> GetWrtOffset() >= pParam -> GetRdOffset() + sizeof(mipHwConn)) *pParam >> mipHwConn;
	}
	PTRACE2INT(eLevelInfoNormal,"CH323Party::OnSendFaultyMfaToPartyCntl :Reason - ",reason);
	if ( (reason == STATUS_FAIL) && (mipHwConn!=eMipNoneHw) )
	{
		BYTE	mipMedia;
		BYTE	mipDirect;
		BYTE	mipTimerStat;
		BYTE	mipAction;
		*pParam >>  mipMedia >> mipDirect >> mipTimerStat >> mipAction;

		m_pConfApi->SendFaultyMfaNoticeToPartyCntl(GetPartyId(), reason,mipHwConn,mipMedia,mipDirect,mipTimerStat,mipAction);
	}
	else
		m_pConfApi->SendFaultyMfaNoticeToPartyCntl(GetPartyId(), reason);

}

////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323UpdateGkCallId(CSegment* pParam)
{
	BYTE gkCallId[SIZE_OF_CALL_ID];
	memset(gkCallId,'\0',SIZE_OF_CALL_ID);
	pParam->Get((unsigned char*)gkCallId, SIZE_OF_CALL_ID);
	m_pConfApi->UpdateGkCallIdInCdr(this, gkCallId);
}

////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnConfSetCapsAccordingToNewAllocation(CSegment* pParam)
{
	WORD len = 0;
	BYTE bIsAudioOnly = FALSE;
	BYTE Dummy = FALSE;
	DWORD VideoRate = 0;
	BYTE cif4Mpi = (BYTE)-1;
	*pParam >> bIsAudioOnly;
	*pParam >> Dummy; // For RTV in SIP
	*pParam >> Dummy; // For RTV in SIP -is msvc
	*pParam >> VideoRate; // For RTV in SIP
	*pParam >> cif4Mpi;

	PTRACE2INT(eLevelInfoNormal,"CH323Party::OnConfSetCapsAccordingToNewAllocation rate is ", VideoRate);

	if(bIsAudioOnly)
		m_pH323Cntl->SetLocalCapToAudioOnly();
	else
	{
		*pParam >> len;
		H264VideoModeDetails h264VidModeDetails;
	    memset(&h264VidModeDetails, '\0', sizeof(H264VideoModeDetails));// fixing BRIDGE-8430 NT: Valgrind detected memory errors in ConfParty when running script AddRemoveConfTemplateNew.py
		pParam->Get((BYTE*)&h264VidModeDetails,len);
		m_pH323Cntl->SetTotalVideoRate(VideoRate);
		m_pH323Cntl->SetVideoParamInCaps(h264VidModeDetails, cif4Mpi, VideoRate);
		m_pH323Cntl->SetH264VideoParams(h264VidModeDetails, H264_ALL_LEVEL_DEFAULT_SAR,cmCapReceive);
		if (m_pH323Cntl->ShouldUpdateMrmpPhysicalIdInfo())
		    m_pH323Cntl->UpdateMrmpInternalChannelIfNeeded();
	}
}


/////////////////////////////////////////////////////////////////////////////////
void CH323Party::ConfUpdatedCapsAndAudioRate(CSegment* pParam)
{
	*pParam >> m_AudioRate;
	TRACESTR(eLevelInfoNormal) << " CH323Party::ConfUpdatedCapsAndAudioRate AudioRate = " << (DWORD)m_AudioRate << ",  Name - " << PARTYNAME;
	BYTE bUpdateCaps = FALSE;
	*pParam >> bUpdateCaps;

	if (bUpdateCaps)
	{
		//Party caps
		POBJDELETE(m_pLocalCapH323);
		m_pLocalCapH323		=	new CCapH323;
		m_pLocalCapH323->DeSerialize(NATIVE,*pParam);

		//H323Cntl caps
		POBJDELETE(m_pH323Cntl->m_pLocalCapH323);
		m_pH323Cntl->m_pLocalCapH323 = new CCapH323;
		*m_pH323Cntl->m_pLocalCapH323 = *m_pLocalCapH323;
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnConfExportChangeMode(CSegment* pParam)
{
	 PTRACE2(eLevelInfoNormal,"CH323Party::OnConfExportChangeMode : Name - ",PARTYNAME);
	 m_state = PARTYCONNECTED;
	 OnConfExport(pParam);
}

////////////////////////////////////////////////////////////////////////////////
void  CH323Party::GetIpCallIdentifiers (IP_EXT_DB_STRINGS* ipStringsStruct)
{

     if(!ipStringsStruct)
    {
        PASSERTMSG(m_pH323Cntl->GetConnectionId(),"CH323Party::GetIpCallIdentifiers - No ipStringsStruct!!");
        return;
    }

	DWORD controlIp = 0;
	int numOfAlias = 0;
    CH323Alias* pAliasList = NULL;
    if (m_pH323Cntl->m_pmcCall->GetIsOrigin())
    {
        pAliasList = m_pH323NetSetup->GetDestPartyAliasList(&numOfAlias);
        if (m_pH323Cntl->m_pmcCall->GetAnswerH245Address()->ipVersion == eIpVersion4)
            controlIp = (m_pH323Cntl->m_pmcCall->GetAnswerH245Address()->addr.v4.ip);
    }
    else
    {
        pAliasList = m_pH323NetSetup->GetSrcPartyAliasList(&numOfAlias);
        if (m_pH323Cntl->m_pmcCall->GetSetupH245Address()->ipVersion == eIpVersion4)
            controlIp = (m_pH323Cntl->m_pmcCall->GetSetupH245Address()->addr.v4.ip);
    }

    char szIp [20];
    ::SystemDWORDToIpString(controlIp, szIp);
    strncpy (ipStringsStruct->ipAddress, szIp, IDENTIFIER_STR_SIZE);
    //PTRACE2INT (eLevelInfoNormal,"CH323Party::GetIpCallIdentifiers ip - ", controlIp);
    //PTRACE2 (eLevelInfoNormal,"CH323Party::GetIpCallIdentifiers ip string - ", ipStringsStruct->ipAddress);
    numOfAlias = min(numOfAlias, NUM_OF_IDENTIFIERS);
    for(int i=0; i<numOfAlias; i++)
        strncpy (ipStringsStruct->identifier[i], pAliasList[i].GetAliasName(), IDENTIFIER_STR_SIZE);

    if (pAliasList)
        PDELETEA(pAliasList);
}
////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnContentMsgFromMaster (CSegment* pParam)
{
    DWORD opcode = 0;
    DWORD rate = 0;

    *pParam >> opcode >> rate;
    m_pConfApi->MasterContentMessage ( opcode,(CTaskApp*)this, m_mcuNum, m_termNum, rate);
}

////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnContentRateChangeDone (CSegment* pParam)
{
    if (m_pH323Cntl->IsContentSpeaker())
        //request came from the master - FlowControl Command
        m_pH323Cntl->SendFlowControlReq(cmCapVideo, FALSE, kRoleContentOrPresentation, m_contentRate);
    else
        //request came from the slave - FlowControl indication
        m_pH323Cntl->SendFlowControlReq(cmCapVideo, TRUE, kRoleContentOrPresentation, m_contentRate);
}

////////////////////////////////////////////////////////////////////////////////
void  CH323Party::UpdatePartyH323LprVideoBitRate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::UpdatePartyH323LprVideoBitRate : Name - ",PARTYNAME);

	DWORD newPeopleRate;
	WORD  channelDirection;
	DWORD lossProtection;
	DWORD mtbf;
	DWORD congestionCeiling;
	DWORD fill;
	DWORD modeTimeout;
    DWORD totalVideoRate;
    DWORD bLprOnContent;
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


    DWORD curBitRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapTransmit);
    DWORD curBitRateContent = m_contentRate;// m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapTransmit,kRoleContentOrPresentation);

    TRACEINTO << "newPeopleRate = " << newPeopleRate << "  curBitRate = " << curBitRate;
	if ( newPeopleRate )
	{
		if ((m_pCurrentModeH323->GetConfType() == kVideoSwitch) || (m_pCurrentModeH323->GetConfType() == kVSW_Fixed))
		{
			// VSW:
			BOOL bEnableFlowControlVSW = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SUPPORT_VSW_FLOW_CONTROL);
			if(!bEnableFlowControlVSW)
			{
				PTRACE(eLevelError,"CH323Party::UpdatePartyH323LprVideoBitRate: SUPPORT_VSW_FLOW_CONTROL is false");
				return;
			}
			if ((newPeopleRate < (VSW_FLOW_CONTROL_RATE_THRESHOLD * GetVideoRate())) && (m_pH323Cntl->GetRemoteIdent() != PolycomMGC))
			{
				PTRACE(eLevelError,"CH323Party::UpdatePartyH323LprVideoBitRate: Flow control bit rate is lower than the threshold");
				return;
			}
			m_pCurrentModeH323->SetFlowControlRateConstraint(newPeopleRate);
			m_pH323Cntl->m_pCurrentModeH323->SetFlowControlRateConstraint(newPeopleRate);
		}
        else if (m_pCurrentModeH323->GetConfType() == kCop)
        {
            const CCommConf*  pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);

            if (pCommCurrConf == NULL) 
            {
            	DBGPASSERT_AND_RETURN(7);
            }
            
            newPeopleRate = m_pCurrentModeH323->CalcCopMinFlowControlRate(pCommCurrConf, newPeopleRate,curBitRateContent);
            m_pCurrentModeH323->SetFlowControlRateConstraint(newPeopleRate);
            m_pH323Cntl->m_pCurrentModeH323->SetFlowControlRateConstraint(newPeopleRate);
        }

		else
		{
			// CP:
			m_pCurrentModeH323->SetTotalVideoRate(totalVideoRate);
			m_pInitialModeH323->SetTotalVideoRate(totalVideoRate);
			m_pCurrentModeH323->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit);
	        m_pH323Cntl->m_pCurrentModeH323->SetVideoBitRate(newPeopleRate, (cmCapDirection)cmCapTransmit);
		}

		m_pConfApi->UpdatePartyLprVideoBitRate(GetPartyId(), newPeopleRate, channelDirection, lossProtection
				, mtbf, congestionCeiling, fill, modeTimeout, totalVideoRate);

		DWORD isDba = FALSE;// in case of LPR we shut down the DBA to have only one flag active.
		if(bLprOnContent)
			SendNewContentRateToConfLevel(newContentRate, isDba);
	}
	else // ( newPeopleRate > GetVideoRate())
	{
		PTRACE(eLevelError,"CH323Party::UpdatePartyH323LprVideoBitRate: No need to update video rate");
	}
}

////////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnSendNewContentRateToConfLevel(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323Party::OnSendNewContentRateToConfLevel", GetPartyId());
	DWORD newContentRate;
	DWORD isDba;
    *pParam >> newContentRate;
    *pParam >> isDba;

	SendNewContentRateToConfLevel(newContentRate, isDba);
}

////////////////////////////////////////////////////////////////////////////////
void  CH323Party::SendNewContentRateToConfLevel(DWORD newContentRate, DWORD isDba)
{
	TRACEINTO << "VNGFE-8204 - newContentRate:" << newContentRate;

	if (newContentRate > 0)
		m_pConfApi->ChangePartyContentBitRateByLpr(GetPartyId(), newContentRate, isDba);
}

////////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnLprUpdatedPartyH323VideoBitRate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnLprUpdatedPartyH323VideoBitRate : Name - ",PARTYNAME);

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

	m_pH323Cntl->SendLprReqToMfa(status, lossProtection, mtbf, congestionCeiling, fill, modeTimeout);

}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::SendLprFlowControlToCard(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::SendLprFlowControlToCard : Name - ",PARTYNAME);
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

	m_pH323Cntl->OnConfFlowControlReq(newVidRate, outChannel, &lprModeChangeData);
}
/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnSetPartyToLeader(CSegment* pParam)
{
	BYTE isLeader;
	*pParam >> isLeader;

	if (isLeader != m_feccPartyType)
	{
		m_feccPartyType = isLeader;
		m_pH323Cntl->UpdateRtpOnLeaderStatus(isLeader);
	}

}
/////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsCascadeToPolycomBridge ()
{
    BYTE retVAL = FALSE;
    if(m_pH323Cntl->m_pmcCall->GetRmtType() == cmEndpointTypeMCU ||
       m_pH323Cntl->GetRemoteIdent() == PolycomRMX || m_pH323Cntl->GetRemoteIdent() == PolycomMGC ||
        GetCascadeMode() == CASCADE_MODE_MASTER || GetCascadeMode() == CASCADE_MODE_SLAVE)
        retVAL =  TRUE;

    return retVAL;
}
/////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsRemoteIsSlaveMGCWithContent()
{
	return m_pH323Cntl->IsRemoteIsSlaveMGCWithContent();
}
/////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsRemoteIsMGC ()
{
    BYTE retVAL = FALSE;
    if(m_pH323Cntl->GetRemoteIdent() == PolycomMGC)
        retVAL =  TRUE;

    return retVAL;
}
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnMcuMngrStartPartyPreviewReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnMcuMngrStartPartyPreviewReq : Name - ",PARTYNAME);

	CChannel* pChannel = NULL;
	BOOL IsPossibleToSendReq = FALSE;
	CapEnum capEnum;

	CIstrStream istream(*pParam);

	CPartyPreviewDrv* tmpPreviewReqParams = new CPartyPreviewDrv;
	tmpPreviewReqParams->DeSerialize(NATIVE,istream);
	WORD channelDirection = tmpPreviewReqParams->GetDirection();

	cmCapDirection Direction;

	// 0 -> in     1 -> out
	if(channelDirection)
	{
		PTRACE(eLevelInfoNormal,"CH323Party::OnMcuMngrStartPartyPreviewReq - Direction cmCapTransmit   ");
		Direction = cmCapTransmit;
	}
	else
	{
		Direction = cmCapReceive;
		PTRACE(eLevelInfoNormal,"CH323Party::OnMcuMngrStartPartyPreviewReq - Direction cmCapReceive  ");
	}

	if(Direction == cmCapReceive)
	{
		pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, FALSE, kRolePeople);
		if (pChannel && !m_bIsPreviewVideoIn)
		{
			if (!pChannel->IsCsChannelStateDisconnecting() && pChannel->GetCsChannelState() != kDisconnectedState)
			{
				IsPossibleToSendReq = TRUE;
				m_bIsPreviewVideoIn = TRUE;

				if(m_RcvPreviewReqParams)
					POBJDELETE(m_RcvPreviewReqParams);
				m_RcvPreviewReqParams = new CPartyPreviewDrv(*tmpPreviewReqParams);
			}
			capEnum = pChannel->GetCapNameEnum();
		}
	 }
	else  //cmCapTransmit
	{
		pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRolePeople);
		if (pChannel && !m_bIsPreviewVideoOut)
		{
			if (!pChannel->IsCsChannelStateDisconnecting() && pChannel->GetCsChannelState() != kDisconnectedState)
			{
				IsPossibleToSendReq = TRUE;
				m_bIsPreviewVideoOut = TRUE;

				if(m_TxPreviewReqParams)
					POBJDELETE(m_TxPreviewReqParams);
				m_TxPreviewReqParams = new CPartyPreviewDrv(*tmpPreviewReqParams);
			}
			capEnum = pChannel->GetCapNameEnum();
	}
	}


	if(IsPossibleToSendReq)
		m_pH323Cntl->SendStartPartyPreviewReqToCM(tmpPreviewReqParams->GetRemoteIP(),tmpPreviewReqParams->GetVideoPort(),Direction,capEnum);
	else
		PTRACE(eLevelInfoNormal,"CH323Party::OnMcuMngrStartPartyPreviewReq - Impossible to send Start req to party");

	POBJDELETE(tmpPreviewReqParams);

}
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnMcuMngrStopPartyPreviewReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnMcuMngrStopPartyPreviewReq : Name - ",PARTYNAME);

	WORD Direction;
	CChannel* pChannel = NULL;
	BOOL IsPossibleToSendReq = FALSE;

	*pParam >> Direction;

	if((cmCapDirection)Direction == cmCapReceive)
	{
		pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, FALSE, kRolePeople);
		if (pChannel && m_bIsPreviewVideoIn)
		{
			if (!pChannel->IsCsChannelStateDisconnecting() && pChannel->GetCsChannelState() != kDisconnectedState)
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
		pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRolePeople);
		if (pChannel && m_bIsPreviewVideoOut)
		{
			if (!pChannel->IsCsChannelStateDisconnecting() && pChannel->GetCsChannelState() != kDisconnectedState)
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
		m_pH323Cntl->SendStopPartyPreviewReqToCM((cmCapDirection)Direction);
	else
		PTRACE(eLevelInfoNormal,"CH323Party::OnMcuMngrStopPartyPreviewReq - Impossible to send stop req to party");

}
void  CH323Party::OnConfSendRssRequest(CSegment* pParam)
{
	m_pH323Cntl->SetRssInfo(pParam);
}
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnMcuMngrIntraPreviewReq(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CH323Party::OnMcuMngrIntraPreviewReq : Name - ",PARTYNAME);

	WORD Direction;
	CChannel* pChannel = NULL;
	BOOL IsPossibleToSendReq = FALSE;
	CSegment*  seg = NULL;

	*pParam >> Direction;

	if((cmCapDirection)Direction == cmCapReceive)
	{
		pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, FALSE, kRolePeople);
		if (pChannel /*&& m_bIsPreviewVideoIn*/)
			if (!pChannel->IsCsChannelStateDisconnecting() && pChannel->GetCsChannelState() != kDisconnectedState)
			{
				//Intra for incoming channel - Ask from EP
				BYTE bIsGradualIntra = FALSE;
				 seg = new CSegment;
				*seg << (WORD)Fast_Update << (WORD)kRolePeople << (WORD)3 << bIsGradualIntra;
				DispatchEvent(RMTH230,seg);
				POBJDELETE(seg);

			}

	}
	else  //cmCapTransmit
	{
		pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, TRUE, kRolePeople);
		if (pChannel /*&& m_bIsPreviewVideoOut*/)
			if (!pChannel->IsCsChannelStateDisconnecting() && pChannel->GetCsChannelState() != kDisconnectedState)
			{
				//18541 in case of cop  we will send a different request so we will know not to suppress this message.
				DWORD confType = m_pInitialModeH323->GetConfType();
				if (confType == kCop)
				{
					PTRACE2(eLevelInfoNormal,"CH323Party::OnMcuMngrIntraPreviewReq  transmit event mode: Name - ",PARTYNAME);
					m_pConfApi->EventModeIntraPreviewReq(GetPartyId());
				}
				else
				{
				//Intra for outgoing channel - ask from VB
				BYTE bIsGradualIntra = FALSE;
				seg = new CSegment;
				*seg << (WORD)Fast_Update << (WORD)kRolePeople << (WORD)2 << bIsGradualIntra;
				DispatchEvent(RMTH230,seg);
				POBJDELETE(seg);
				}




			}
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::StopAllPreviews()
{
	m_bIsPreviewVideoIn = FALSE;
	m_bIsPreviewVideoOut = FALSE;
	if(m_RcvPreviewReqParams)
		POBJDELETE(m_RcvPreviewReqParams);
	if(m_TxPreviewReqParams)
		POBJDELETE(m_TxPreviewReqParams);
	//No need to send stop preview to CM because channel disconnects
}
/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnRemoveSelfFlowControlConstraint(CSegment* pParam)
{
    PTRACE (eLevelInfoNormal, "CH323Party::OnRemoveSelfFlowControlConstraint");
    DWORD newVidRate =  m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapTransmit);
    if (newVidRate)
    {
        //remove constraints from the SCMs
        m_pCurrentModeH323->SetFlowControlRateConstraint(0);
        m_pH323Cntl->m_pCurrentModeH323->SetFlowControlRateConstraint(0);
        //send flow control to notify the remote that the rate is going up
        m_pH323Cntl->OnConfFlowControlReq(newVidRate, TRUE);
    }

    else
        PTRACE (eLevelError, "CH323Party::OnRemoveSelfFlowControlConstraint current people tx is ZERO?! ");

}
/////////////////////////////////////////////////////////////////////////////
WORD  CH323Party::AllocateMcuNumber()
{
	WORD refMcuNumber = 0;
	WORD refTerminalNumber = 0;
	if(IsValidPObjectPtr(m_pTerminalNumberingManager))
	{
		m_pTerminalNumberingManager->AllocateMcuNumber(this,refMcuNumber,refTerminalNumber);
	}
	else
	{
		PASSERT(1);
	}
	return refMcuNumber;
}
////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
BYTE  CH323Party::IsRemoteIsRVpresentation()
{
	return m_pH323Cntl->IsRemoteIsRVpresentation();
}


/*
/////////////////////////////////////////////////////////////////////////////
void  CH323Party::RestorePreviewReqIfNeeded(EIpChannelType channelTypeE)
{

	CChannel* pChannel = NULL;
	CapEnum capEnum;

	if(channelTypeE == VIDEO_IN)
	{
		if(m_bIsPreviewVideoIn)
			if(m_RcvPreviewReqParams)
			{
				pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, FALSE, kRolePeople);
				capEnum = pChannel->GetCapNameEnum();
				m_pH323Cntl->SendStartPartyPreviewReqToCM(m_TxPreviewReqParams->GetRemoteIP(),m_TxPreviewReqParams->GetVideoPort(),cmCapRecieve,capEnum);

			}
	}
	else
		if(channelTypeE == VIDEO_OUT)
		{
			if(m_bIsPreviewVideoOut)
				if(m_TxPreviewReqParams)
				{
					pChannel = m_pH323Cntl->FindChannelInList(cmCapVideo, FALSE, kRolePeople);
					capEnum = pChannel->GetCapNameEnum();
					m_pH323Cntl->SendStartPartyPreviewReqToCM(m_TxPreviewReqParams->GetRemoteIP(),m_TxPreviewReqParams->GetVideoPort(),cmCapTransmit,capEnum);
				}
		}

}

*/
//////////////////////////////////////////////////////////////////////////////////////////
void CH323Party::OnH323SendVideoMute(CSegment* pParam)
{
	BYTE isActive;
	*pParam >> isActive;
    PTRACE2INT (eLevelInfoNormal, "CH323Party::OnH323SendVideoMute: isActive ", isActive);

	m_pH323Cntl->SendMuteForVideoChannel(isActive);
}
///////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323Party::OnPartySendITPSpeakerAckReq //shiraITP - 95
void CH323Party::OnPartySendITPSpeakerAckReq(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal, "ITP_CASCADE: CH323Party::OnPartySendITPSpeakerAckReq");

    if (m_pH323Cntl)
        m_pH323Cntl->SendNewITPSpeakerAckToParty(); //shiraITP - 96
    else
    {
        PTRACE(eLevelError, "ITP_CASCADE: CH323Party::OnPartySendITPSpeakerAckReq ERROR - m_pH323Cntl is NULL");
        PASSERTMSG(1,"ITP_CASCADE: CH323Party::OnPartySendITPSpeakerAckReq ERROR - m_pH323Cntl is NULL");
    }
}
///////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323Party::OnVBSendITPSpeakerReq //shiraITP - 102
void CH323Party::OnVBSendITPSpeakerReq(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal, "ITP_CASCADE: CH323Party::OnVBSendITPSpeakerReq");

    DWORD numOfActiveLinks;
    BYTE  itpType;

    *pParam >> numOfActiveLinks;
    *pParam >> itpType;

    PTRACE2INT(eLevelInfoNormal,"ITP_CASCADE: CH323Party::OnVBSendITPSpeakerReq - numOfActiveLinks:",numOfActiveLinks);
    PTRACE2INT(eLevelInfoNormal,"ITP_CASCADE: CH323Party::OnVBSendITPSpeakerReq - itpType:",itpType);

    if (m_pH323Cntl)
        m_pH323Cntl->SendNewITPSpeakerToParty((eTelePresencePartyType)itpType,numOfActiveLinks); //shiraITP - 103
    else
    {
        PTRACE(eLevelError, "ITP_CASCADE: CH323Party::OnVBSendITPSpeakerReq ERROR - m_pH323Cntl is NULL");
        PASSERTMSG(1,"ITP_CASCADE: CH323Party::OnVBSendITPSpeakerReq ERROR - m_pH323Cntl is NULL");
    }
}

DWORD CH323Party::GetSSRcIdsForAvc(int ind, cmCapDirection direction, cmCapDataType aDataType)
{
    DWORD ssrc = 0;
    ssrc = m_SsrcIdsForAvcParty.GetSsrcId(ind, aDataType);

    CCommConf* pComConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

    TRACECOND_AND_RETURN_VALUE(!pComConf, "CH323Party::GetSSRcIdsForAvc: pComConf is NULL!!", ssrc);

    return ssrc;
}

////////////////////////////////////////////////////////////////////////////////
#define PATCH_FOR_MRMP_WAIT_FOR_INTRA 10000
void CH323Party::OnMrmpRtcpFirInd(CSegment* pParam)
{
	// relevant for mix-mode only
	CCommConf* pComConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());

	TRACECOND_AND_RETURN(!pComConf, "CH323Party::OnMrmpRtcpFirInd: pComConf is NULL!!");
	if (pComConf->GetConfMediaType() != eMixAvcSvc)
		return;

	MrmpRtcpFirStruct* pStruct = (MrmpRtcpFirStruct*)pParam->GetPtr(1);
	RelayIntraParam intraParam;
	intraParam.m_partyRsrcId = GetPartyId();
	intraParam.m_bIsGdr = ((TRUE == pStruct->bIsGdr) ? true : false);

	TRACEINTO << "productType:" << CProcessBase::GetProcess()->GetProductType() << ", confMediaType:" << pComConf->GetConfMediaType() << ", pStruct->unSequenseNumber:" << pStruct->unSequenseNumber;
	for (int i = 0; i < pStruct->nNumberOfSyncSources; i++)
	{
		TRACEINTO << "syncSources:" << pStruct->syncSources[i];
		intraParam.m_listSsrc.push_back(pStruct->syncSources[i]);
	}

	if (pComConf->GetConfMediaType() == eMixAvcSvc && (pStruct->unSequenseNumber == PATCH_FOR_MRMP_WAIT_FOR_INTRA))
		intraParam.m_bIsSsrc = true;
	else
		intraParam.m_bIsSsrc = false;

	m_pConfApi->HandleMrmpRtcpFirInd(GetPartyId(), &intraParam);
}

////////////////////////////////////////////////////////////////////////////////
void CH323Party::UpgradeAvcToMixed(CSegment* pParam, CComModeH323* pNewMode)
{
	TRACEINTO<<"mix_mode: pNewMode->GetConfMediaType():" << (int)pNewMode->GetConfMediaType();
//	m_pH323Cntl->SetConfMediaType(pNewMode->GetConfMediaType());
//	if (m_state == PARTYCHANGEMODE) //@#@ - noa - need to move to another place - we are getting to this function in PARTYCHANGEMODE state already.
//	{
//		TRACEINTO << "@#@ mix_mode: already in change mode! Save the request for further use.";
//		m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapVideo, cmCapReceive, kRolePeople);
//		m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapVideo, cmCapTransmit, kRolePeople);
//		m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapAudio, cmCapReceive, kRolePeople);
//		m_pUpdateTargetMode->CopyMediaMode(*pNewMode, cmCapAudio, cmCapTransmit, kRolePeople);
//		AddToChangeModeMask(eChangeModeMask_UpgradeToMixed); // ey_20866 need to decide how to handle transaction
//		return;
//	}

	// ey_20866 desirialize avcToSvcTranslatorRsrcParams and deliver to transaction

//			PTRACE2INT(eLevelInfoNormal,"CSipParty::SendFlowControlToCs : no active trans-the rate recieved is:  ", newVidRate);
//			m_pTargetMode->SetVideoBitRate(newVidRate, cmCapReceive , (ERoleLabel)kRolePeople);
//			m_pUpdateTargetMode->SetVideoBitRate(newVidRate, cmCapReceive , (ERoleLabel)kRolePeople);
//			cmCapDirection mediaDir = cmCapReceive;
//			SendFlowControlMessage(mainType/*main video type-people*/,mediaDir,newVidRate );

	//FSN-613: Dynamic Content for SVC/Mix Conf
	/*DWORD newVideoRcvRate = pNewMode->GetMediaBitRate(cmCapVideo ,cmCapReceive);
	DWORD curVideoRcvRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapReceive);
	if (curVideoRcvRate != 0 && newVideoRcvRate != curVideoRcvRate)
		 m_pH323Cntl->SendFlowControlReq(cmCapVideo, FALSE, kRolePeople, newVideoRcvRate);

	DWORD newVideoTxRate = pNewMode->GetMediaBitRate(cmCapVideo ,cmCapTransmit);
	DWORD curVideoTxRate = m_pCurrentModeH323->GetMediaBitRate(cmCapVideo ,cmCapTransmit);
	if (curVideoTxRate != 0 && newVideoTxRate != curVideoTxRate)
		m_pH323Cntl->SendFlowControlReq(cmCapVideo, TRUE, kRolePeople, newVideoTxRate);*/

	*m_pInitialModeH323 = *pNewMode;
	TRACEINTO<<"mix_mode: m_pInitialModeH323->GetConfMediaType()" << m_pInitialModeH323->GetConfMediaType();

	CSegment *pSeg = new CSegment;
	CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		avcToSvcTranslatorRsrcParams[i]=NULL;
	}

	BYTE bIsTranslatorExists;
	CRsrcParams* pMrmpRsrcParams = NULL;

	DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams, "mix_mode: MRMP");

	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i], "mix_mode: translator");
	}

	// need to insert mrmp
	SerializeNonMandatoryRsrcParams(pSeg, pMrmpRsrcParams, "mix_mode: MRMP");
	SerializeNonMandatoryRsrcParamsArray(pSeg, avcToSvcTranslatorRsrcParams, NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS, "mix_mode: translator");

	POBJDELETE(pMrmpRsrcParams);
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		 POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
	}

	StartTransaction(kTransUpgradeAvcOnlyToMixReq, PARTY_UPGRADE_TO_MIXED, pSeg);

	POBJDELETE(pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::StartTransaction(ETransactionType eTransactionType, OPCODE opcode, CSegment * pParam)
{
	DBGPASSERT_AND_RETURN(IsActiveTransaction());
	TRACEINTO << "Transaction type = " << GetTransactionTypeAsString(eTransactionType) << ", Name = " << m_partyConfName;

	POBJDELETE(m_pTransaction);

	if (eTransactionType == kTransUpgradeAvcOnlyToMixReq)
		m_pTransaction = new CH323TransUpgradeToMixed(this);
	else
	{
		DBGPASSERT(eTransactionType+100);
	}

	DBGPASSERT_AND_RETURN(!m_pTransaction);

	m_eActiveTransactionType = eTransactionType;

//	// At start transaction, the Updated target mode is set to the active mode
//	m_maskRequiredChangeMode = eChangeModeMask_None;
//	*m_pUpdateTargetMode = *m_pTargetMode;

	m_pTransaction->InitTransaction(this, m_pH323Cntl, m_pCurrentModeH323, m_pInitialModeH323, m_partyConfName/*, m_voice, m_alternativeAddrStr,m_pTargetModeMaxAllocation, m_bTransactionSetContentOn, m_isContentSuggested*/);

//	if (opcode != DEFAULT_OPCODE)
//		DispatchEvent(opcode, pParam);

	// send opcode to the transaction
	m_pTransaction->DispatchEvent(opcode, pParam);

//	m_bTransactionSetContentOn = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
ETransactionType CH323Party::EndTransaction(BYTE &bPendingTrns)
{
	bPendingTrns = FALSE;
	TRACEINTO << "Transaction type = " << GetTransactionTypeAsString(m_eActiveTransactionType) << ", Name = " << m_partyConfName;
	ETransactionType retTransType = m_eActiveTransactionType;

	m_eActiveTransactionType = kTransNone;

	return retTransType;
}


BYTE CH323Party::IsActiveTransaction() const
{
	if ((m_eActiveTransactionType != kTransNone) && (IsValidPObjectPtr(m_pTransaction)))
		return TRUE;
	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
char* CH323Party::GetTransactionTypeAsString(ETransactionType type)
{
	switch( type )
	{
		case kTransNone:    				{ return "No transaction"; }
		case kTransUpgradeAvcOnlyToMixReq:  { return "Upgrade H.323 AVC to mixed request"; }
		default:
			return "Unknown type";
	}
	return "Unknown type";
}
/*
/////////////////////////////////////////////////////////////////////////////
void CH323Party::HandleEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
	if (opCode == SIP_TRANSACTION_MSG)
	{
		OPCODE wrappedOpcode;
		*pMsg >> wrappedOpcode;
		CParty::HandleEvent(pMsg, msgLen, wrappedOpcode);
	}
	else
	{
		if (IsValidPObjectPtr(m_pTransaction))
			m_pTransaction->HandlePartyEvent(pMsg, msgLen, opCode);
		else
			DBGPASSERT(opCode);
	}
}
*/
/////////////////////////////////////////////////////////////////////////////
BOOL  CH323Party::DispatchEvent(OPCODE event,CSegment* pParam)
{
	TRACESTR(eLevelError) << "@#@ opcode=" << event << ", and state - "<< m_state;

	BOOL bActiveTrans = IsActiveTransaction();
	//BOOL bIsEventExists = m_pSipTransaction->IsEventExist(event);
	if (bActiveTrans && m_pTransaction->DispatchEvent(event, pParam))
	{
		return TRUE;
	}

	if (CParty::DispatchEvent(event, pParam))
		return TRUE;

	if (bActiveTrans)
		TRACEINTO << "Transaction state: ", m_pTransaction->GetState();

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnPartyUpgradeToMixChannelsUpdated(CSegment* pParam)
{
	TRACEINTO<<"mix_mode: started";

    unsigned int channelHandle;
    channelHandle = INVALID_CHANNEL_HANDLE;
    channelHandle = m_pH323Cntl->GetMrmpChannelHandle(cmCapVideo);
	if(channelHandle == INVALID_CHANNEL_HANDLE)
	{
		TRACEINTO << "mix_mode: dynMixedErr invalid channelHandle";
	}
	else
	{
		TRACEINTO << "mix_mode: channelHandle[0] " << channelHandle;
	}
	m_pConfApi->PartyUpgradeToMixChannelsUpdated(m_PartyRsrcID, channelHandle);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnTransEndTransactionAnycase(CSegment* pParam)
{
	TRACEINTO<<"mix_mode: ";

	m_eActiveTransactionType = kTransNone;
	POBJDELETE(m_pTransaction);
}

/////////////////////////////////////////////////////////////////////////////
void CH323Party::OnPartyTranslatorArtsDisconnected(CSegment* pParam)
{
	TRACEINTO << "mix_mode:";

	OnEndH323Disconnect(m_pDisconnectParams);
    POBJDELETE(m_pDisconnectParams);
}

//--------------------------------------------------------------------------
void CH323Party::OnRemoveAvcToSvcArtTranslatorAnycase(CSegment* pParam)
{
    TRACEINTO << "PartyId: " << GetPartyRsrcID() << ", ConfId:" << GetConfId() << ", m_state:" << m_state;

    m_pInitialModeH323->DeSerialize(NATIVE, *pParam);
    m_pH323Cntl->Deescalate(m_pInitialModeH323);

}

//--------------------------------------------------------------------------
void CH323Party::OnPartyTranslatorArtsDisconnectedDeescalating(CSegment* pParam)
{
    TRACEINTO << "PartyId: " << GetPartyRsrcID() << ", ConfId:" << m_pParty->GetConfId() << ", m_state:" << GetPartyStateStr(m_state);

    // update streams in the current mode
    m_pCurrentModeH323->SetStreamsListForMediaMode(m_pInitialModeH323->GetStreamsListForMediaMode(cmCapAudio, cmCapTransmit, kRolePeople), cmCapAudio, cmCapTransmit, kRolePeople);
    m_pCurrentModeH323->SetStreamsListForMediaMode(m_pInitialModeH323->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople), cmCapVideo, cmCapTransmit, kRolePeople);

    // update party control
    m_pConfApi->SendAvcToSvcArtTranslatorDisconnectedToPartyControl(this, STATUS_OK);
}

//--------------------------------------------------------------------------
/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
void CH323Party::SendCallDropToPartyAnycase(CSegment* pParam)
{
	PartyRsrcID PartyId;
	*pParam >> PartyId;
	PTRACE2INT(eLevelInfoNormal, "CH323Party::SendCallDropToPartyAnycase - PartyId: ", PartyId);

	m_pH323Cntl->VideoRecoverSendCallDrop();
	return;
}
/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

/////////////////////////////////////////////////////////////////////////////
void  CH323Party::OnTokenRecapCollisionEndAnycase(CSegment* pParam)
{
	CConfParty* pConfParty = GetConfPartyNonConst();
	PASSERT_AND_RETURN(!pConfParty);
	PASSERT_AND_RETURN(!m_pH323Cntl);

	if (pConfParty -> IsTokenRecapPendedDueToCollisionDetection())
	{
		switch (pConfParty -> GetTokenRecapCollisionDetection())
		{
			case etrcdTokenHandlingInProgress:
			{
				//=========================================
				// A reinvite got pended, handling it now
				//=========================================
				PTRACE(eLevelInfoNormal,"CH323Party::OnTokenRecapCollisionEndAnycase - Invoking OnH323CapIndication for pended recap");
				m_pH323Cntl -> OnH323CapIndication(&m_recapPendedOnToken);
			}
			break;

			case etrcdRecapInProgress:
			{
				//=========================================
				// A reinvite got pended, handling it now
				//=========================================
				PTRACE(eLevelInfoNormal,"CH323Party::OnTokenRecapCollisionEndAnycase - Invoking OnH323CapIndication for pended recap");
				TranslateTokenMessageToStandardContentForH323Cntl(m_pendedTokenOpcode, &m_pendedToken, m_pendedTokenIsSpeakerChange);
			}
			break;

			default:
			{
				//=======================================================
				// This event should not have been sent in these states
				//=======================================================
				PTRACE2INT(eLevelError, "CH323Party::OnTokenRecapCollisionEndAnycase - usage illegal, collision state is ", pConfParty -> GetTokenRecapCollisionDetection());
			}
		}
	}
	else
	{
		//=======================================================
		// This event should not have been sent in these states
		//=======================================================
		PTRACE(eLevelError, "CH323Party::OnTokenRecapCollisionEndAnycase - usage illegal, nothing is pending");
	}

	//==========================================
	// Resetting pends (note: also for errors)
	//==========================================
	pConfParty -> SetTokenRecapCollisionDetection(etrcdAvailable);
	pConfParty -> UnpendTokenRecapDueToCollisionDetection();
}
