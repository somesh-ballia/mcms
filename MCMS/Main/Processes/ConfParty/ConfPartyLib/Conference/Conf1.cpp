//                            CONF1.CPP                                       |
//            Copyright 1995 Pictel Technologies Ltd.                         |
//                   All Rights Reserved.                                     |
//----------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary        |
// information of Pictel Technologies Ltd. and is protected by law.           |
// It may not be copied or distributed in any form or medium, disclosed       |
// to third parties, reverse engineered or used in any manner without         |
// prior written authorization from Pictel Technologies Ltd.                  |
//----------------------------------------------------------------------------|
// FILE:       CONF1.CPP                                                      |
// SUBSYSTEM:  MCMS                                                           |
// PROGRAMMER: Sami                                                           |
//----------------------------------------------------------------------------|
// Who | Date       | Description                                             |
//----------------------------------------------------------------------------|
//     | 7/6/95     |                                                         |
//Ori P| 16.4.01    | Voting functions                                        |
//Ori P| 20.5.01    | Q&A functions                                           |
//Zohar| 15.1.04    | Dividing Conf.cpp into 2 files.                         |
//     |            | Conf.cpp contains all and only reqular CConf functions. |
//     |            | Conf1.cpp contains all and only CConf action functions. |
//Talya| 18.5.05    | Moved to Carmel                                         |
//+===========================================================================+

#include <math.h>

#include "NStream.h"
#include "Conf.h"
#include "Macros.h"
#include "DataTypes.h"
#include "ObjString.h"
#include "ConfDef.h"
#include "AudioBridgeInterface.h"
#include "AudioBridgeInitParams.h"
#include "VideoBridgeInterface.h"
#include "StatusesGeneral.h"
#include "H323NetSetup.h"
#include "H323Caps.h"
#include "ConfPartyGlobals.h"
#include "SysConfig.h"
#include "ConfPartyProcess.h"
#include "LobbyApi.h"
#include "IVRService.h"
#include "Lobby.h"
#include "IpCsOpcodes.h"
#include "SIPConfPack.h"
#include "SvcEventPackage.h"
#include "HostCommonDefinitions.h"
#include "FECCBridge.h"
#include "ContentBridge.h"
#include "ConfAppFeatureObject.h"
#include "IVRServiceList.h"
#include "ApiStatuses.h"
#include "HlogApi.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "SipDefinitions.h"
#include "SipNetSetup.h"
#include "SipStructures.h"
#include "SipCaps.h"
#include "SIPPartyControl.h"
#include "OpcodesMcmsCommon.h"
#include "IpCommon.h"
#include "SystemFunctions.h"
#include "StructTm.h"
#include "BondingCntl.h"
#include "ConfPartiesDialingSequence.h"
#include "TextOnScreenMngr.h"
#include "Gathering.h"
#include "McmsPCMManager.h"
#include "McmsPCMManagerInitParams.h"
#include "IpService.h"
#include "AddressBook.h"
#include "CustomizeDisplaySettingForOngoingConfConfiguration.h"
#include "H323DelPartyControl.h"
#include "CommModeInfo.h"
#include "EventPackage.h"
#include "EnumsToStrings.h"


#define DTMF_INVITE_PARTY_NAME_SUFFIX "_Invited"

extern const CLobbyApi*       GetpLobbyApi();
extern CAVmsgServiceList*     GetpAVmsgServList();
extern CIpServiceListManager* GetIpServiceListMngr();
extern const char* MediaStateToString(eConfMediaState confState);
extern void InitiateAVMCUDB(char* fileName,PartyRsrcID id,const char* callId);

PBEGIN_MESSAGE_MAP(CConf)
	ONEVENT(STARTCONF,                          IDLE,         CConf::OnMcuStartConfIdle)
	ONEVENT(ADDPARTY,                           CONNECT,      CConf::OnMcuAddPartyConnect)
	ONEVENT(ADDPARTY,                           TERMINATION,  CConf::OnMcuAddPartyTermination)
	ONEVENT(ADDPARTY,                           IDLE,         CConf::OnMcuAddPartyIdle)
	ONEVENT(ADDPARTY,                           SETUP,        CConf::OnMcuAddPartySetup)
	ONEVENT(START_CONF_RSRC_IND,                ANYCASE,      CConf::OnMcuStartConfInd)

	ONEVENT(DELPARTY,                           CONNECT,      CConf::OnMcuDelPartyConnect)
	ONEVENT(DELPARTY,                           TERMINATION,  CConf::OnMcuDelPartyTermination)

	ONEVENT(DELPARTYVIOLENT,                    CONNECT,      CConf::OnMcuDelPartyCViolent)

	ONEVENT(RECONPARTY,                         CONNECT,      CConf::OnMcuReconnectPartyConnect)
	ONEVENT(SETENDTIME,                         CONNECT,      CConf::OnMcuSetEndTimeConnect)
	ONEVENT(SETAUTOREDIAL,                      CONNECT,      CConf::OnMcuSetAutoRedial)

	ONEVENT(DESTROY,                            ANYCASE,      CConf::OnMcuTerminateConfConnect)
	ONEVENT(FORCE_KILL,                         ANYCASE,      CConf::OnMcuForceKillAnyCase)

	// lobby events
	ONEVENT(ADDINPARTY,                         CONNECT,      CConf::OnLobbyAddPartyConnect)
	ONEVENT(ADDPARTYCHNLDESC,                   CONNECT,      CConf::OnLobbyAddPartyChannelDescConnect)
	ONEVENT(H323ADDINPARTY,                     CONNECT,      CConf::OnH323LobbyAddPartyConnect)
	ONEVENT(H323ADDINPARTY,                     ANYCASE,      CConf::OnH323LobbyRejectParty)
	ONEVENT(SIPADDINPARTY,                      CONNECT,      CConf::OnSipLobbyAddPartyConnect)
	ONEVENT(SIPADDINPARTY,                      ANYCASE,      CConf::OnSipLobbyRejectParty)

	// party control events
	ONEVENT(ENDADDPARTY,                        CONNECT,      CConf::OnPartyEndAddConnect)
	ONEVENT(ENDDELPARTY,                        CONNECT,      CConf::OnPartyEndDelConnect)
	ONEVENT(ENDDELPARTY,                        TERMINATION,  CConf::OnPartyEndDelTerminate)
	ONEVENT(REDAILPARTY,                        CONNECT,      CConf::OnPartyRedialConnect)

	ONEVENT(PARTYLAYOUTCHANGED,                 CONNECT,      CConf::OnPartyLayoutChanged)
	ONEVENT(PARTYLAYOUTCHANGED,                 ANYCASE,      CConf::NullActionFunction)

	ONEVENT(PARTYDISCONNECT,                    CONNECT,      CConf::OnPartyDisConnectConnect)
	ONEVENT(PARTYDISCONNECT,                    TERMINATION,  CConf::NullActionFunction)

	ONEVENT(EXPPARTY,                           CONNECT,      CConf::OnMcuExportParty)
	ONEVENT(EXPPARTY,                           TERMINATION,  CConf::NullActionFunction)
	ONEVENT(ENDEXPPARTY,                        CONNECT,      CConf::OnPartyEndExport)
	ONEVENT(ENDIMPPARTY,                        CONNECT,      CConf::OnPartyEndImport)
	ONEVENT(IMPPARTY,                           CONNECT,      CConf::OnPartyImportParty)

	ONEVENT(ISCONFREADYFORMOVE,                 SETUP,        CConf::OnSourcePartyIsConfReadyForMoveConnect)
	ONEVENT(ISCONFREADYFORMOVE,                 CONNECT,      CConf::OnSourcePartyIsConfReadyForMoveConnect)
	ONEVENT(ISCONFREADYFORMOVE,                 IDLE,         CConf::OnSourcePartyIsConfReadyForMoveIdle)
	ONEVENT(ISCONFREADYFORMOVE,                 TERMINATION,  CConf::OnSourcePartyIsConfReadyForMoveIdle)

	ONEVENT(PARTY_CHANEG_VID_MODE,              CONNECT,      CConf::OnPartyChangeVidMode)
	ONEVENT(PARTY_RECEIVE_RECAPS,               CONNECT,      CConf::OnPartyReceiveReCaps)
	ONEVENT(PARTY_RECEIVE_RECAPS,               TERMINATION,     CConf::NullActionFunction)

	// audio brdg events*/
	ONEVENT(AUDBRDGCONNECT,                     SETUP,        CConf::OnAudBrdgConnectSetup)
	ONEVENT(AUDBRDGDISCONNECT,                  TERMINATION,  CConf::OnAudBrdgDisConnectTerminate)

	// video brdg events
	ONEVENT(VIDBRDGCONNECT,                     SETUP,        CConf::OnVidBrdgConnectSetup)
	ONEVENT(VIDBRDGDISCONNECT,                  TERMINATION,  CConf::OnVidBrdgDisConnectTerminate)
	//LEGACY
	ONEVENT(CONTENT_DECODER_ALLOC_RSRC_FAIL,    ANYCASE,      CConf::OnVideoBridgeContentDecoderAllocFail)
	ONEVENT(CONTENT_DECODER_RESET_FAIL_STATUS,  ANYCASE,      CConf::OnVideoBridgeContentDecoderResetFailStatus)

	// fecc brdg events
	ONEVENT(FECCBRDGCONNECT,                    SETUP,        CConf::OnFeccBrdgConnectSetup)
	ONEVENT(FECCBRDGDISCONNECT,                 TERMINATION,  CConf::OnFeccBrdgDisConnectTerminate)

	// Content Bridge events
	ONEVENT(CONTENTBRDGCONNECT,                 SETUP,        CConf::OnContentBrdgConnectSetup)
	ONEVENT(CONTENTBRDGDISCONNECT,              TERMINATION,  CConf::OnContentBrdgDisConnectTerminate)
	ONEVENT(CONTENTBRDGDISCONNECT,              CONNECT,      CConf::OnContentBrdgDisConnectConnect)
	ONEVENT(STARTCONTENT,                       SETUP,        CConf::NullActionFunction)
	ONEVENT(STARTCONTENT,                       CONNECT,      CConf::OnContentBrdgStartContentCONNECT)
	ONEVENT(STARTCONTENT,                       TERMINATION,  CConf::NullActionFunction)

	ONEVENT(STOPCONTENT,                        SETUP,        CConf::NullActionFunction)
	ONEVENT(STOPCONTENT,                        CONNECT,      CConf::OnContentBrdgStopContentCONNECT)
	ONEVENT(STOPCONTENT,                        TERMINATION,  CConf::NullActionFunction)

	// local events
	ONEVENT(CONFTOUT,                           CONNECT,      CConf::OnTimerConnect)
	ONEVENT(CONFTOUT,                           IDLE,         CConf::OnTimerIdle)
	ONEVENT(ALERTOUT,                           CONNECT,      CConf::OnTimerRemindConnect)
	ONEVENT(AUTOTERMINATE,                      CONNECT,      CConf::OnTimerConnectAuto)
	ONEVENT(GWESTABLISHEDTOUT,                  CONNECT,      CConf::CheckGWConfEstablishedWith2Parties)
	ONEVENT(GWESTABLISHEDTOUT,                  TERMINATION,  CConf::NullActionFunction)
	ONEVENT(GW_TEXT_ON_SCREEN_TOUT,             CONNECT,      CConf::StopTextOnScreenMessages)
	ONEVENT(GW_TEXT_ON_SCREEN_TOUT,             TERMINATION,  CConf::NullActionFunction)
	ONEVENT(DTMF_FWD_ALL_TOUT,                  CONNECT,      CConf::OnTimerDtmfFwdAll )
	ONEVENT(DELAY_ISDN_LINK_DISCONNECT_TOUT,    CONNECT,      CConf::OnTimerDelayIsdnLinkDisconnect)

	ONEVENT(REMOVPARTYCONNECTION,               CONNECT,      CConf::OnConfRemovePartyConnection)
	ONEVENT(REMOVPARTYCONNECTION,               TERMINATION,  CConf::OnConfRemovePartyConnectionTerminate)

	ONEVENT(CONF_AUDIO_NUMBER_INDICATION_GAP_TOUT,	   CONNECT, 	CConf::OnTimerAudioNumberChanged)
	ONEVENT(CONF_AUDIO_NUMBER_INDICATION_GAP_TOUT, TERMINATION, 	CConf::NullActionFunction)
	ONEVENT(CONF_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT, CONNECT,    	CConf::OnTimerAudioNumberHidden)
	ONEVENT(CONF_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT, TERMINATION, CConf::NullActionFunction)

	// IBM - EVENT PACKAGE
	ONEVENT(SIP_CS_SIG_SUBSCRIBE_IND,           CONNECT,      CConf::OnSipSubscribeIndCONNECT)
	ONEVENT(SIP_CS_SIG_SUBSCRIBE_IND,           ANYCASE,      CConf::NullActionFunction)
	ONEVENT(SIP_CS_SIG_REFER_IND,               CONNECT,      CConf::OnSipReferIndCONNECT)
	ONEVENT(SIP_CS_SIG_NOTIFY_RESPONSE_IND,     CONNECT,      CConf::OnSipNotifyResponseIndCONNECT)
	ONEVENT(SIP_CS_SIG_NOTIFY_RESPONSE_IND,     ANYCASE,      CConf::NullActionFunction)

	// terminal events
	ONEVENT(HANDLE_TERMINAL_EVENT,              CONNECT,      CConf::OnTerminalHandleEventConnect)
	ONEVENT(TERMINAL_RECURRENT_INTRA_REQ_TOUT,  CONNECT,      CConf::OnTerminalRecurrentIntraReqTout)

	// ivr events
	ONEVENT(IVR_CONFTERMINATE,                  CONNECT,      CConf::OnIvrConfTerminate)
	ONEVENT(DTMF_FORWARDING_TIMER,              CONNECT,      CConf::OnIvrTimerConfTerminate)
	ONEVENT(IVR_PARTY_UPDATE_STATUS,            CONNECT,      CConf::OnIvrUpdatePartyIvrStatus)
	ONEVENT(IVR_PARTY_UPDATE_STATUS,            TERMINATION,  CConf::NullActionFunction)
	ONEVENT(IVR_SHOW_PARTICIPANTS,              CONNECT,      CConf::OnIvrShowParticipants)
	ONEVENT(PARTY_PASSED_IVR,                   CONNECT,      CConf::OnIVRPartyPassedEntranceProcedCONNECT)
	ONEVENT(PARTY_PASSED_IVR,                   TERMINATION,  CConf::NullActionFunction)

	ONEVENT(IVR_SHOW_GATHERING,                 SETUP,        CConf::NullActionFunction)
	ONEVENT(IVR_SHOW_GATHERING,                 TERMINATION,  CConf::NullActionFunction)
	ONEVENT(IVR_SHOW_GATHERING,                 IDLE,         CConf::NullActionFunction)
	ONEVENT(IVR_SHOW_GATHERING,                 CONNECT,      CConf::OnIvrShowGathering)

	ONEVENT(IPPARTYMONITORINGREQ,               ANYCASE,      CConf::OnSendIpMonitorReqToParty)
	ONEVENT(UPDATEVISUALNAME,                   ANYCASE,      CConf::OnUpdateVisualNameForParty)
	ONEVENT(MUTE_ALL_BUT_X,                     CONNECT,      CConf::OnIvrMuteAllButX)
	ONEVENT(CHAIR_DROPPED_TERMINATE,            CONNECT,      CConf::OnChairDroppedTerminate)

	ONEVENT(IVR_INVITE_PARTY_TO_CONF,           CONNECT,      CConf::OnDtmfInviteParty)
	ONEVENT(IVR_INVITE_PARTY_TO_CONF,           ANYCASE,      CConf::NullActionFunction)

	ONEVENT(DISCONNECT_INVITED_PARTICIPANT_REQ, CONNECT,      CConf::OnDisconnectInvitedParticipantReq)
	ONEVENT(DISCONNECT_INVITED_PARTICIPANT_REQ, ANYCASE,      CConf::NullActionFunction)

	ONEVENT(PARTYENDINITCOM,                    CONNECT,      CConf::OnPartyEndInitComm)

	ONEVENT(LINK_CONNECT,                       SETUP,        CConf::NullActionFunction)
	ONEVENT(LINK_CONNECT,                       CONNECT,      CConf::OnPartyLinkConnectCONNECT)
	ONEVENT(LINK_CONNECT,                       TERMINATION,  CConf::NullActionFunction)

	ONEVENT(LINK_DISCONNECT,                    SETUP,        CConf::NullActionFunction)
	ONEVENT(LINK_DISCONNECT,                    CONNECT,      CConf::OnPartyLinkDisconnectCONNECT)
	ONEVENT(LINK_DISCONNECT,                    TERMINATION,  CConf::OnPartyLinkDisconnectTERMINATION)

	ONEVENT(UPDATE_CONTENT_PROTOCOL,            CONNECT,      CConf::OnPartyUpdateContentProtocolCONNECT)
	ONEVENT(UPDATE_CONTENT_PROTOCOL,            ANYCASE,      CConf::NullActionFunction)
	ONEVENT(DELAY_UPDATE_CONTENT_TOUT,          CONNECT,      CConf::OnDelayUpdateContentTimeout)
	ONEVENT(DELAY_UPDATE_CONTENT_TOUT,          ANYCASE,      CConf::NullActionFunction)

	ONEVENT(ADD_RECORDING_LINK_PARTY,           CONNECT,      CConf::OnAddAndConnectRecordingLink)
	ONEVENT(DISCONNECT_RECORDING_LINK_PARTY,    IDLE,         CConf::OnDisconnectRecordingLink)
	ONEVENT(DISCONNECT_RECORDING_LINK_PARTY,    SETUP,        CConf::OnDisconnectRecordingLink)
	ONEVENT(DISCONNECT_RECORDING_LINK_PARTY,    CONNECT,      CConf::OnDisconnectRecordingLink)
	ONEVENT(UPDATE_RECORDING_CONTROL,           CONNECT,      CConf::OnUpdateRecordingControlCONNECT)

	ONEVENT(ACTIVATE_AUTO_TERMINATION_CHECK_FOR_AV_MCU,    CONNECT,         CConf::OnActivateAutoTerminationTestForAvMcu)
	ONEVENT(ACTIVATE_AUTO_TERMINATION_CHECK_FOR_AV_MCU,    ANYCASE,         CConf::NullActionFunction)


	ONEVENT(OPERATOR_ASSIST,                    CONNECT,      CConf::OnOperatorAssistance)
	ONEVENT(ADDDIALSTRING,                      ANYCASE,      CConf::OnUpdateDialString)
	ONEVENT(DIALOUTSERVICENAME,                 ANYCASE,      CConf::OnUpdateIsdnDialOutServiceNameForGW)
	ONEVENT(ENDGWSETUP,                         ANYCASE,      CConf::OnGateWayConfEndSetup)
	ONEVENT(ENDDTMFINVITEPARTYSETUP,            ANYCASE,      CConf::OnDtmfInvitePartyConfEndSetup)

	ONEVENT(DECSYNC,                            ANYCASE,      CConf::OnVideoDecoderInitialSync)

	ONEVENT(VIDEOREFRESHBEFORERECORDING,        IDLE,         CConf::NullActionFunction)
	ONEVENT(VIDEOREFRESHBEFORERECORDING,        SETUP,        CConf::NullActionFunction)
	ONEVENT(VIDEOREFRESHBEFORERECORDING,        CONNECT,      CConf::OnMcuVideoRefreshBeforeRecording)
	ONEVENT(VIDEOREFRESHBEFORERECORDING,        TERMINATION,  CConf::NullActionFunction)

	ONEVENT(GATHERING_UPDATE_TIMER,             IDLE,         CConf::GatheringUpdateTimerHandler)
	ONEVENT(GATHERING_UPDATE_TIMER,             CONNECT,      CConf::GatheringUpdateTimerHandler)
	ONEVENT(GATHERING_UPDATE_TIMER,             SETUP,        CConf::NullActionFunction)
	ONEVENT(GATHERING_UPDATE_TIMER,             TERMINATION,  CConf::NullActionFunction)

	ONEVENT(GATHERING_PARTY_CONNECTING_TIMER,   IDLE,         CConf::GatheringConnectingTimerHandler)
	ONEVENT(GATHERING_PARTY_CONNECTING_TIMER,   CONNECT,      CConf::GatheringConnectingTimerHandler)
	ONEVENT(GATHERING_PARTY_CONNECTING_TIMER,   SETUP,        CConf::NullActionFunction)
	ONEVENT(GATHERING_PARTY_CONNECTING_TIMER,   TERMINATION,  CConf::NullActionFunction)

	ONEVENT(GATHERING_PARTY_CONNECTED_MSG,      IDLE,         CConf::OnGatheringPartyConnected)
	ONEVENT(GATHERING_PARTY_CONNECTED_MSG,      CONNECT,      CConf::OnGatheringPartyConnected)
	ONEVENT(GATHERING_PARTY_CONNECTED_MSG,      SETUP,        CConf::NullActionFunction)
	ONEVENT(GATHERING_PARTY_CONNECTED_MSG,      TERMINATION,  CConf::NullActionFunction)

	//Video Preview reqs from EMA
	ONEVENT(START_PREVIEW_PARTY,                ANYCASE,      CConf::OnSendStartPreviewReqToParty)
	ONEVENT(STOP_PREVIEW_PARTY,                 ANYCASE,      CConf::OnSendStopPreviewReqToParty)
	ONEVENT(REQUEST_PREVIEW_INTRA,              ANYCASE,      CConf::OnMcuIntraPreviewReq)
	ONEVENT(SET_EXCLUSIVE_CONTENT,              CONNECT,      CConf::OnExclusiveContentSet)
	ONEVENT(REMOVE_EXCLUSIVE_CONTENT,           CONNECT,      CConf::OnRemoveExclusiveContent)
	ONEVENT(SET_EXCLUSIVE_CONTENT_MODE,         CONNECT,      CConf::OnExclusiveContentModeSet)//Restricted content

	//SVC AVC
	ONEVENT(SET_CONF_AVC_SVC_MEDIA_STATE,       IDLE,         CConf::OnConfAvcSvcModeSet)
	ONEVENT(SET_CONF_AVC_SVC_MEDIA_STATE,       CONNECT,      CConf::OnConfAvcSvcModeSet)
	ONEVENT(SET_CONF_AVC_SVC_MEDIA_STATE,       SETUP,        CConf::OnConfAvcSvcModeSet)
	ONEVENT(SET_CONF_AVC_SVC_MEDIA_STATE,       TERMINATION,  CConf::NullActionFunction)

	ONEVENT(SET_PARTY_AVC_SVC_MEDIA_STATE,      ANYCASE,      CConf::OnPartyAvcSvcModeSet ) // only terminal event

	ONEVENT(SET_MUTE_INCOMING_LECTURE_MODE,     CONNECT,      CConf::OnMuteIncomingLectureModeSet)
	ONEVENT(PCM_INVITE_PARTY,                   SETUP,        CConf::RespondPCMManagerInviteFailed)
	ONEVENT(PCM_INVITE_PARTY,                   CONNECT,      CConf::OnPCMInviteParty)
	ONEVENT(PCM_INVITE_PARTY,                   TERMINATION,  CConf::RespondPCMManagerInviteFailed)

	ONEVENT(PARTY_CG_START_CONTENT,             CONNECT,      CConf::OnCGStartContent)
	ONEVENT(PARTY_CG_STOP_CONTENT,              CONNECT,      CConf::OnCGStoptContent)

	// TIP add slave party
	ONEVENT(ADDSLAVEPARTY,                      CONNECT,      CConf::OnAddSlavePartyConnect)

	//Multiple links for ITP in cascaded conference feature:
	ONEVENT(ADDSUBLINKSPARTIES,                 CONNECT,      CConf::OnAddSubLinksPartiesConnect)

	ONEVENT(FALL_BACK_FROM_TIP_TO_REGULAR_SIP,  CONNECT,      CConf::FallBackToRegularSip)
	ONEVENT(FALL_BACK_FROM_ICE_TO_SIP,          CONNECT,      CConf::FallBackFromIceToRegularSip)

	ONEVENT(MESSAGE_QUEUE_TOUT,                 ANYCASE,      CConf::OnTimerMessageQueue)

	//  XCode Brdg
	ONEVENT(XCODEBRDGCONNECT,                   SETUP,        CConf::OnXCodeBrdgConnectSetup)
	ONEVENT(XCODEBRDGCONNECT,                   CONNECT,      CConf::OnXCodeBrdgConnectConnect)
	ONEVENT(XCODEBRDGCONNECT,                   ANYCASE,      CConf::NullActionFunction)
	ONEVENT(XCODEBRDGDISCONNECT,                TERMINATION,  CConf::OnXCodeBrdgDisConnectTerminate)
	ONEVENT(XCODEBRDGDISCONNECT,                CONNECT,      CConf::OnXCodeBrdgDisConnectConnect)

	// MS Lync add slave party
	ONEVENT(ADDMSSLAVEPARTY,                    CONNECT,      CConf::OnAddMsSlavePartyConnect)
	ONEVENT(RELEASE_AVMCU_PARTY,                CONNECT,      CConf::OnLobbyRelaseAvMcuPartyConnect)

	//MS EP
	ONEVENT(LYNC_CONF_INFO_UPDATED,							ANYCASE,			CConf::HandleEventPackageEvent)
	ONEVENT(LYNC_CONF_INFO_UPDATED,							TERMINATION,			CConf::NullActionFunction)

       /*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
       ONEVENT(VIDEORECOVERYDISCONNECTPARTYCORDING,        IDLE,         CConf::VideoRecoverySendByeToParty)
	ONEVENT(VIDEORECOVERYDISCONNECTPARTYCORDING,        SETUP,        CConf::VideoRecoverySendByeToParty)
	ONEVENT(VIDEORECOVERYDISCONNECTPARTYCORDING,        CONNECT,      CConf::VideoRecoverySendByeToParty)
	ONEVENT(VIDEORECOVERYDISCONNECTPARTYCORDING,        TERMINATION,  CConf::VideoRecoverySendByeToParty)
	/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

	// TELEPRESENCE_LAYOUTS
	// IDLE - new already updated in conf DB, do nothing ; TERMINATION - do nothing ; CONNECT - do change ; SETUP (bridge connection)
	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,        IDLE,  	   		CConf::NullActionFunction)
	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,        SETUP,  	   		CConf::OnSetTelepresenceLayoutModeSetup)
	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,        CONNECT,  	   		CConf::OnSetTelepresenceLayoutModeConnected)
	ONEVENT(SET_TELEPRESENCE_LAYOUT_MODE,        TERMINATION,  	   	CConf::NullActionFunction)

PEND_MESSAGE_MAP(CConf,CStateMachine);


////////////////////////////////////////////////////////////////////////////
void CConf::OnMcuStartConfInd(CSegment* pParam)
{
  PTRACE(eLevelInfoNormal, "CConf::OnMcuStartConfInd - I got the response here NOT in the right MBOX");
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuStartConfIdle(CSegment* pParam)
{
	CCommConfDB* pCommConfDB = ::GetpConfDB();
	CCommRes* pComRes = new CCommRes;
	pComRes->DeSerialize(NATIVE,*pParam);
    BYTE bIsAutoRedial = pComRes->GetIsAutoRedial();
    PTRACE2INT (eLevelInfoNormal, "CConf::OnMcuStartConfIdle - bIsAutoRedial = ", bIsAutoRedial);

	// set timer for conference duration
	BYTE lsdRate = LSD_Off;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	BOOL IsEnableAutoExtension = FALSE;
	std::string key = "ENABLE_AUTO_EXTENSION";
	sysConfig->GetBOOLDataByKey(key, IsEnableAutoExtension);

	DWORD ExtensionTimeInterval  = 0;
	key = "EXTENSION_TIME_INTERVAL";
	sysConfig->GetDWORDDataByKey(key, ExtensionTimeInterval);

    DWORD durationTime = pComRes->ExtendDurationIfNeed(IsEnableAutoExtension,ExtensionTimeInterval);

	DWORD trimmed_duration = (durationTime >= 5) ? (durationTime - 5) : durationTime;
	// we trim the duration in 5 seconds in order to prevent resource release lateness (sagi)

	if( !pComRes->IsPermanent() )
	{
		CSmallString sstr;
		sstr << "time out set for " << trimmed_duration << " seconds, " << m_name;
		PTRACE2(eLevelInfoNormal,"CConf::OnMcuStartConfIdle : StartTimer(CONFTOUT) ",sstr.GetString());

	    StartTimer(CONFTOUT,trimmed_duration * SECOND);
	}

  if (pCommConfDB == NULL)
  {
		//Free MonitorId (problems anyway....)
		POBJDELETE(pComRes);
		return;
	}

	AllocPartyList();

	//Get the conf and update the mail box to the task mail box
	m_pCommConf=pCommConfDB->GetCurrentConf(pComRes->GetMonitorConfId());

	if(m_pCommConf == NULL)
	{
		PTRACE(eLevelInfoNormal,"CConf::OnMcuStartConfIdle, do not find conference in Conf DB - exit function");
		POBJDELETE(pComRes);
		return;
	}

	m_pCommConf->SetCurrentConfCascadeMode(CASCADE_MODE_NONE);
	if (m_pCommConf->GetIsGateway())
	{
		m_isGateWay = TRUE;
		CMedString cstr;
		cstr << "GATEWAY_LOG: CConf::OnMcuStartConfIdle - Creating GW conf";
		cstr << "\nsetting time before first join to 30 seconds";
		PTRACE(eLevelInfoNormal,cstr.GetString());
		// if in 30 seconds no party came from lobby terminate the conf
		m_AutoTerminateBeforeFirstJoin = 30;
  }


//	key = "FECC";
//	BOOL bIsFECC;
//	sysConfig->GetBOOLDataByKey(key, bIsFECC);
//	if(bIsFECC)
	if (m_pCommConf->GetFECCEnabled())
	   lsdRate =LSD_6400;

	m_pCommConf->SetLSDRate(lsdRate);

//	if (::GetIsCOPdongleSysMode())
//	{
//		if (m_pCommConf->IsAudioConf())
//		{
//			PASSERT(1, "Audio only conference in COP is disabled");
//		}
//	}
	// no Spreading Request for Standalone Conference
	// ConfRsrcId is const and the same for all Standalone Conferences


	// Initiate m_pUnifiedComMode  before Allocate AS-SIP Resources
	m_pUnifiedComMode = new CUnifiedComMode(pComRes->GetEnterpriseModeFixedRate(), pComRes->GetConfTransferRate(),pComRes->GetIsHighProfileContent());
	BYTE lConfRate = m_pCommConf->GetConfTransferRate();
	m_pUnifiedComMode->SetCallRate(lConfRate);
	WORD audRate = pComRes->GetAudioRate();
	audRate = 0;
	m_pUnifiedComMode->SetAudModeAndAudBitRate(audRate);
	WORD confType = GetVideoSession();
	m_pUnifiedComMode->SetConfType(confType);


	TRACEINTO << "CONF CONTENT TYPE -TEMP "<<((int) m_pCommConf->GetPresentationProtocol());
	PTRACE2INT(eLevelError," CConf::OnMcuStartConfIdle -CONF CONTENT TYPE -TEMP ", m_pCommConf->GetPresentationProtocol());
	if(m_pCommConf->GetEntryQ())
	{
		m_ConfRsrcId = STANDALONE_CONF_ID;
	}
	else
	{
		STATUS allocateConfStatus =	SyncRsrcAllocConfSpreading(START_CONF_RSRC_REQ);

		if(STATUS_OK != allocateConfStatus)
		{
			if (!(GetVideoSession() == VIDEO_SESSION_COP))//in cop conference it's not rare that there are no resources, we will send a fault message when that is the case
			PASSERT(allocateConfStatus);

			PTRACE2(eLevelInfoNormal,"CConf::OnMcuStartConfIdle : Failed to Allocate Conf Resources ",m_name);
			DWORD updatedStatus = 0;
			updatedStatus = (CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
			UpdateConfStatus(updatedStatus,YES);
			SetConfTerminateCause(CONFERENCE_NEVER_STARTED);
			POBJDELETE(pComRes);
			m_isAllocateConfResourcesFailed =TRUE;
      if (GetVideoSession() == VIDEO_SESSION_COP || GetVideoSession() == VIDEO_SWITCH_FIXED) // VIDEO_SWITCH_FIXED related only for 2000C. In general case we should  distinguish between CP an COP(VSW)
      {         // In case we fail to create cause there are no resources we will terminate the conference
				//1. faults
				CMedString str;
				str << "Fail to allocate ";
				// VNGR-12816 Wrong alert when trying to add more VSW conferences that allowed by resources
				if (GetVideoSession() == VIDEO_SESSION_COP)
					str << "COP";
				else
					str << "Video switch";

				str << " conference resources for conf: " << m_name;
				CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, INSUFFICIENT_EVENT_MODE_CONFERENCE_RESOURCES, MAJOR_ERROR_LEVEL,str.GetString(), TRUE);

				//2. remove conference
				CleanUp();
			}
			return;
		}

		if(m_pCommConf->GetContentMultiResolutionEnabled() && m_pCommConf->GetIsAsSipContent())
		{
			allocateConfStatus =	SyncRsrcAllocConfSpreading(ALLOCATE_CONTENT_XCODE_REQ);
			if(STATUS_OK != allocateConfStatus)
			{
				PASSERT(allocateConfStatus);
				PTRACE2(eLevelInfoNormal,"CConf::OnMcuStartConfIdle : Failed to Allocate XCode Conf Resources ",m_name);
				DWORD updatedStatus = 0;
				updatedStatus = (CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
				UpdateConfStatus(updatedStatus,YES);
				SetConfTerminateCause(CONFERENCE_NEVER_STARTED);
				POBJDELETE(pComRes);
				m_isAllocateConfResourcesFailed =TRUE;
				//1. faults
				CMedString str;
				str << "Conference creation failed due to lack of content transcode DSP resources for conf: " << m_name
				    << " Conference Monitor ID: " << m_pCommConf->GetMonitorConfId() << ", Conference Rsrc ID: " << m_ConfRsrcId;
				CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, INSUFFICIENT_RESOURCES, MAJOR_ERROR_LEVEL,str.GetString(), TRUE);
				//2. remove conference
				CleanUp();
				return;
			}
		}
		AddConfEntryToRsrcRoutingTbl();
	}

	DWORD updatedStatus = 0;
	updatedStatus = (CONFERENCE_EMPTY | CONFERENCE_NOT_FULL);
	UpdateConfStatus(updatedStatus,YES);
	if(m_pCommConf->IsAutomaticTermination ())
	{
		if (!m_isGateWay && !m_isDtmfInviteParty)
			m_AutoTerminateBeforeFirstJoin = 60 * (DWORD)m_pCommConf->GetTimeBeforeFirstJoin();

		m_AutoTerminateAfterLastQuit = 60 * (DWORD)m_pCommConf->GetTimeAfterLastQuit();
	}

	InitConfMediaState();

	// Dima:if for starting the conference we need leader
	// that we make the conference "dialout manually"- for
	// eleminate scenario: meet.room with dial out parties->
	// someone wrong connected->conf up->automatic redial to parties
  if (m_pCommConf->IsMeetingRoom() && m_pCommConf->IsDefinedIVRService())
  {
		m_StandByStart = YES;

		PTRACE2INT(eLevelInfoNormal,"CConf::OnMcuStartConfIdle call m_StandByStart: ",m_StandByStart);
  }
  else
		m_StandByStart = pComRes->IsStandBy();

  	//tmp should be from commres, after it will be updated from commres need just to update the operation point set id as the conference id
  	SetConfOperationPoints();

  	SetIsEnableHdInAvcSvcMixMode();


	if ( ! pComRes->IsAudioConf() )
	{

		// Update General Video related params
		if (confType == VIDEO_SESSION_COP)
		{
			PTRACE2INT(eLevelInfoNormal,"CConf::OnMcuStartConfIdle call rate:",m_pUnifiedComMode->GetCallRate());
			SetCopComMode();

			CComModeH323* pTempMode = m_pUnifiedComMode->GetIPComMode();
			pTempMode->Dump("CConf::OnMcuStartConfIdle - GetIPComMode is: ",eLevelInfoNormal);
			CCopVideoTxModes* pTempCopModes = m_pUnifiedComMode->GetCopVideoTxModes();
			pTempCopModes->Dump("CConf::OnMcuStartConfIdle - GetCopVideoTxModes is: ",eLevelInfoNormal);
		}
		else if(!IsHDVSW())
		{
			m_pUnifiedComMode->SetIsAutoVidProtocol(pComRes->GetVideoProtocol());
			m_pUnifiedComMode->SetIsAutoVidRes(pComRes->GetVideoPictureFormat());
			m_pUnifiedComMode->SetOperationPointPreset(pComRes->GetOperationPointPreset());
		}
		else
		{
			PTRACE2(eLevelInfoNormal,"CConf::OnMcuStartConfIdle : HD Conference = ", m_name);
			SetHDVSWComMode();
		}

		// if TR/CP conference
		if (GetVideoSession() != VIDEO_SWITCH && GetVideoSession() != VIDEO_SWITCH_FIXED && GetVideoSession() != VIDEO_SESSION_COP)
		{
			PTRACE(eLevelInfoNormal,"N.A. DEBUG CConf::OnMcuStartConfIdle : before AdjustVideoCapToHW ");//N.A. DEBUG VP8
			AdjustVideoCapToHW(UpdateSCM);
		}
	}
	m_pVideoBridgeInterface = new CVideoBridgeInterface;
	// FECC
	BYTE confLSDRate = m_pCommConf->GetLSDRate();
	if (confLSDRate != 0)
		m_pUnifiedComMode->SetFECCMode(confLSDRate);

	 if(m_pCommConf->GetFECCEnabled())
		m_pFECCBridge = new CFECCBridge;

	m_pTerminalNumberingManager = new CTerminalNumberingManager;

	//Encryption
	BYTE encryption_mode = pComRes->GetIsEncryption();

	if(encryption_mode)
	{
		PTRACE(eLevelInfoNormal,"CConf::OnMcuStartConfIdle : Encrypted Conference");
		m_pUnifiedComMode->SetEncrypMode(Encryp_On);
	}
	else
		m_pUnifiedComMode->SetEncrypMode(Encryp_Off);

	//LPR
	m_pUnifiedComMode->SetIsLpr(pComRes->GetIsLPR());

	//Content
	m_pContentBridge = new CContentBridge;

	if(IsEnableH239())
	{
	    BYTE bContentAsVideo = m_pCommConf->IsLegacyShowContentAsVideo();
		eEnterpriseMode ContRatelevel = (eEnterpriseMode)pComRes->GetEnterpriseMode();
		ePresentationProtocol ContProtocol = (ePresentationProtocol)pComRes->GetPresentationProtocol();
		//Set max content rate by conf rate
	  	BOOL isHDContent1080Supported = FALSE;
	    BYTE ConfAMCRate = m_pUnifiedComMode->GetContentModeAMC(lConfRate,ContRatelevel, ContProtocol, m_pCommConf->GetCascadeOptimizeResolution(), m_pCommConf->GetConfMediaType());
		BYTE HDResMpi = 0;
		isHDContent1080Supported = SelectContentHDResolution(ConfAMCRate,H264,HDResMpi,FALSE);
		SetContentMode(lConfRate,ContRatelevel,ContProtocol,m_pCommConf->GetCascadeOptimizeResolution(), cmCapReceiveAndTransmit,bContentAsVideo,isHDContent1080Supported,HDResMpi);
		// set current content rate in conf
		SetNewContentBitRate(AMC_0k);
	}
  else
  {
		if(!IsEnableH239())
		      PTRACE2(eLevelInfoNormal,"CConf::OnMcuStartConfIdle H.239 is disabled : Name - ",m_name);
	}

	CComModeH323* pH323NewMode = NULL;
	pH323NewMode = m_pUnifiedComMode->GetIPComMode();
	pH323NewMode->Dump("CConf::OnMcuStartConfIdle - IpComMode is: ",eLevelInfoNormal);

	CComMode* pIsdnComMode = NULL;
	pIsdnComMode = m_pUnifiedComMode->GetIsdnComMode();
	pIsdnComMode->UpdateH221string(1);
	pIsdnComMode->Dump(1);
	pIsdnComMode->m_vidMode.Dump();
	TRACESTR (eLevelInfoNormal) <<"*****TRANSMIT MEDIA BIT RATE IS:";
	DWORD aud_bitrate = 0;
	DWORD vid_bitrate = 0;
	DWORD lsd_bitrate = 0;
	DWORD hsd_bitrate = 0;
	DWORD mlp_bitrate = 0;
	DWORD hmlp_bitrate= 0;
	DWORD content_bitrate = 0;

	pIsdnComMode->GetMediaBitrate(aud_bitrate,vid_bitrate,lsd_bitrate,
			hsd_bitrate,mlp_bitrate,hmlp_bitrate,content_bitrate);

//CreateVideoBridge
	if (GetVideoSession() == VIDEO_SESSION_COP)
	{
		if(m_pCommConf->GetEntryQ())
		{
			CreateVideoBridge(eVideoCOPEq_Bridge_V1);
		}
		else
			CreateVideoBridge(eVideoCOP_Bridge_V1);
	}
  else if (YES == IsHDVSW())
  {
	CreateVideoBridge(eVideoSW_Bridge_V1);
	}
else
{
	if( YES == IsLegacyShowContentAsVideo())
			CreateVideoBridge(eVideoCPContent_Bridge_V1);
	else CreateVideoBridge();
}

  if(m_pCommConf->GetFECCEnabled())
	 CreateFECCBridge();

  if (IsEnableH239())
    CreateContentBridge();
  else
    CreateInActiveContentBridge(); //Create a CONNECT_INACTIVE content bridge to allow support of token MSGs

  if (!pComRes->IsPermanent())
  {
    if (m_AutoTerminateBeforeFirstJoin)// m_AutoTerminateBeforeFirstJoin = 0 means that auto termination is disable and termination can be done only from the operator or chair.
    {
      DWORD timeout = m_AutoTerminateBeforeFirstJoin*SECOND;
      TRACEINTO << "StartTimer(AUTOTERMINATE BeforeFirstJoin)=" << timeout << " (seconds), ConfName=" << m_name;
      StartTimer(AUTOTERMINATE, timeout);
    }

    if (IsEnableAutoExtension)
    {
      if (durationTime <= ExtensionTimeInterval*60)
      {
        PTRACE2(eLevelError, "CConf::OnMcuStartConfIdle - Can not start timer ALERTOUT because duration time is less than alert time, ConfName=", m_name);
      }
      else
      {
        DWORD timeout = (durationTime-(ExtensionTimeInterval*60))*SECOND;
        m_pCommConf->ExtendDurationIfNeed(IsEnableAutoExtension, ExtensionTimeInterval);
        StartTimer(ALERTOUT, timeout);
        TRACEINTO << "StartTimer(ALERTOUT)=" << timeout << " (seconds), ConfName=" << m_name;
      }
    }
    else
    {
      DWORD alertTomeTimeInterval = (DWORD)(m_pCommConf->GetAlertToneTiming()*60);
      DWORD timeout = (durationTime > alertTomeTimeInterval) ? durationTime-alertTomeTimeInterval : 0;
      if (timeout > 0)
      {
        TRACEINTO << "StartTimer(ALERTOUT)=" << timeout*SECOND << " (seconds), ConfName=" << m_name;
        StartTimer(ALERTOUT, timeout*SECOND);
      }
    }
  }
  else
    PTRACE2(eLevelInfoNormal, "CConf::OnMcuStartConfIdle - Permanent Conference starting..., ConfName=", m_name);

  // -------  AB I/F ------------
 		m_pAudBrdgInterface = new CAudioBridgeInterface;
		CAudioBridgeInitParams initParams(this, m_name, m_ConfRsrcId, eAudio_Bridge_V1,
										  pComRes->GetTalkHoldTime(), pComRes->GetAudioMixDepth(),
										  (BOOL)(pComRes->GetAutoMuteNoisyParties()));

		m_pAudBrdgInterface->Create(&initParams);

		// -------  SIP --------------
	    PTRACE(eLevelInfoNormal, "CConf::OnMcuStartConfIdle - CSIPEventPackageDispatcher");
		m_pSipEventPackage = new CSIPEventPackageDispatcher(m_pRcvMbx);
		DWORD confMediaType = m_pCommConf->GetConfMediaType();// AVC/SVC/Mix-CP/Mix-VSW
		if (confMediaType == eSvcOnly || confMediaType == eMixAvcSvc || confMediaType == eMixAvcSvcVsw)
		m_pSvcEventPackage = new CSvcEventPackageDispatcher(this, m_pRcvMbx);

		// -------  CAM I/F ------------
		m_pConfAppMngrInterface = new CConfAppMngrInterface;
		WORD isIvrInConf = CheckIfIVRServiceIsOn();
		WORD isVideoConf = 1;
		WORD isWaitForChair = m_pCommConf->GetStartConfRequiresLeaderOnOff();
		WORD isTerminateConfAfterChairDropped=m_pCommConf-> GetTerminateConfAfterChairDroppedOnOff();
		CAvMsgStruct* pAvMsgStruct = m_pCommConf->GetpAvMsgStruct();
		const char*  ivrName = NULL;
		if (pAvMsgStruct)
			ivrName = pAvMsgStruct->GetAvMsgServiceName();

		WORD isEQConf = (WORD)m_pCommConf->GetEntryQ();
		WORD isCascadeEQ = (WORD)m_pCommConf->GetCascadeEQ();
		WORD enableRecording = m_pCommConf->GetEnableRecording();
		WORD enableRecordingIcon = m_pCommConf->GetEnableRecordingIcon();
		WORD  enableRecNotify	= m_pCommConf->GetEnableRecordingNotify();
		WORD startRecordingPolicy =	m_pCommConf->GetStartRecPolicy();
		WORD isOperatorConf = m_pCommConf->GetOperatorConf();
		BYTE isExternalIvrControl = m_pCommConf->isExternalIvrControl();
		BYTE isMuteAllPartiesAudioExceptLeader = m_pCommConf->GetMuteAllPartiesAudioExceptLeader();
		CConfAppMngrInitParams initAppParams( this, m_name, m_ConfRsrcId, ivrName,
											 m_pAudBrdgInterface, m_pVideoBridgeInterface,
											 isIvrInConf, isVideoConf, isWaitForChair,isTerminateConfAfterChairDropped,
											 isEQConf, isCascadeEQ,
											 enableRecording, enableRecordingIcon, enableRecNotify, startRecordingPolicy,
											 m_isGateWay, isOperatorConf,isExternalIvrControl, isMuteAllPartiesAudioExceptLeader );
		m_pConfAppMngrInterface->Create( &initAppParams );

		if (!isEQConf)
		{
			if (GetVideoSession() == VIDEO_SESSION_COP )//In case of EQ we dont allocate COP resources
			{
				m_pMcmsPCMManager = new CMcmsPCMManagerCOP;
				CMcmsPCMMngrInitParams initPcmParams(this, m_name, m_ConfRsrcId,
													 m_pAudBrdgInterface, m_pVideoBridgeInterface,m_pConfAppMngrInterface);
				m_pMcmsPCMManager->Create(&initPcmParams);
			}
			else if(GetVideoSession() == CONTINUOUS_PRESENCE)
			{
				m_pMcmsPCMManager = new CMcmsPCMManagerCP;
				CMcmsPCMMngrInitParams initPcmParams(this, m_name, m_ConfRsrcId,
													 m_pAudBrdgInterface, m_pVideoBridgeInterface,m_pConfAppMngrInterface);
				m_pMcmsPCMManager->Create(&initPcmParams);
			}
		}

		// state of conf
		m_state = SETUP;
		//pComRes->SetFocusUri("sip:siguser18@dev13.std;gruu;opaque=app:conf:focus:id:3Q60K2FL");
		if (strlen(pComRes->GetFocusUriScheduling()))
		{
			AddAvMCUParty(pComRes->GetFocusUriScheduling());
		}
		// SIP Proxy
		AnnounceNewConfToSipProxy();
		//InitiateAVMCUDB("/nethome/romem/226_romem_MCMS-V100.0_dev/vob/MCMS/Main/Processes/ConfParty/Tests/TestLync.Sample.Basic.xml",m_ConfRsrcId ,NULL);
		POBJDELETE(pComRes);
}

void CConf::SetIsEnableHdInAvcSvcMixMode()
{
	eConfMediaType confMediaType = m_pCommConf->GetConfMediaType();
	if(eMixAvcSvc != confMediaType)
	{
		TRACEINTO << " Not Avc Svc Mix Mode, HD enable is not relevant";
		return;
	}
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL isEnable = FALSE;
	std::string key = "ENABLE_HIGH_VIDEO_RES_AVC_TO_SVC_IN_MIXED_MODE";
	sysConfig->GetBOOLDataByKey(key, isEnable);
	m_pCommConf->SetEnableHighVideoResInAvcToSvcMixMode( isEnable );
	isEnable = FALSE;
	key = "ENABLE_HIGH_VIDEO_RES_SVC_TO_AVC_IN_MIXED_MODE";
	sysConfig->GetBOOLDataByKey(key, isEnable);
	m_pCommConf->SetEnableHighVideoResInSvcToAvcMixMode( isEnable );
	BOOL ba2s = m_pCommConf->GetEnableHighVideoResInAvcToSvcMixMode();
	BOOL bs2a = m_pCommConf->GetEnableHighVideoResInSvcToAvcMixMode();
	TRACEINTO << " EnableHighVideoResInAvcToSvcMixMode = " << (DWORD)ba2s << ", EnableHighVideoResInSvcToAvcMixMode = " << (DWORD)bs2a;
}
//////////////////////////////////////////////////////////////////////////////
void CConf::AddAvMCUParty(const char* avMcuFocusUri)
{
	PASSERT_AND_RETURN(!avMcuFocusUri);

	TRACEINTO << "FocusUri:" << avMcuFocusUri;

	const char* conf_serv_name = m_pCommConf->GetServiceNameForMinParties();
	CConfIpParameters* pIpParams = ::GetIpServiceListMngr()->GetRelevantService(conf_serv_name, SIP_INTERFACE_TYPE);
	PASSERT_AND_RETURN(!pIpParams);

	const char* sipUri = GetSipUriAddressType(avMcuFocusUri);
	PASSERT_AND_RETURN(!sipUri);

	std::auto_ptr<CRsrvParty> pAvMcuParty(new CRsrvParty);

	pAvMcuParty->SetNetInterfaceType(SIP_INTERFACE_TYPE);
	pAvMcuParty->SetUndefinedType(UNRESERVED_PARTY);
	pAvMcuParty->SetPartyId(0xFFFFFFFF);
	pAvMcuParty->SetMsftAvmcuState(eMsftAvmcuUnkown);
	pAvMcuParty->SetIsDMAAVMCUParty(TRUE);
	pAvMcuParty->SetFocusUri(avMcuFocusUri);
	pAvMcuParty->SetName("AV_MCU_Party");
	pAvMcuParty->SetServiceProviderName((char*)pIpParams->GetServiceName());
	pAvMcuParty->SetConnectionType(DIAL_OUT);
	pAvMcuParty->SetSipPartyAddressType(PARTY_SIP_SIPURI_ID_TYPE);
	pAvMcuParty->SetSipPartyAddress(sipUri);
	pAvMcuParty->SetCascadeMode(CASCADE_MODE_MASTER);
	pAvMcuParty->SetListeningAudioVolume(AUDIO_VOLUME_MAX); //BRIDGE-12426

	delete[] sipUri;

	std::auto_ptr<CConfParty> pConfParty(new CConfParty(*pAvMcuParty.get()));

	STATUS status = m_pCommConf->Add(*pConfParty);
	PASSERTSTREAM_AND_RETURN(status != STATUS_OK, "Status:" << status);

	CConfParty* pNewConfParty = m_pCommConf->GetCurrentParty(pConfParty->GetName());
	PASSERTSTREAM_AND_RETURN(!pNewConfParty, "PartyName:" << pConfParty->GetName() << " - Party is not in ConfDB");


}

/////////////////////////////////////////////////////////////////////////////
char* CConf::GetSipUriAddressType(const char* avMcuFocusUri)
{
	PASSERT_AND_RETURN_VALUE(!avMcuFocusUri, NULL);

	//sip:siguser18@dev13.std;gruu;opaque=app:conf:focus:id:3Q60K2FL
	const char* startPos = strstr(avMcuFocusUri, "sip:");
	if (startPos)
	{
		startPos += strlen("sip:");
		const char* endPos = strstr(avMcuFocusUri, ";gruu;");
		if (endPos)
		{
			int len = endPos-startPos;
			char* sipUri = new char[len+1];
			strcpy_safe(sipUri, len+1, startPos);
			TRACEINTO << "SipUri:" << sipUri;
			return sipUri;
		}
	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::SetMuteIncomingForLectureMode(const char* lecturerName)
{
	PartyRsrcID lecturerId = -1;

	if (lecturerName && lecturerName[0])
	{
		CPartyConnection* pPartyConnection = GetPartyConnection(lecturerName);

		if (!pPartyConnection)
		{
			TRACEINTOLVLERR << "LecturerName:" << lecturerName << " - Failed, Party connection was not found";
		}
		else
		{
			lecturerId = pPartyConnection->GetPartyRsrcId();

			TRACEINTO << "LecturerName:" << lecturerName << ", LecturerId:" << lecturerId;
		}
	}

	SetMuteIncomingForLectureMode(lecturerId);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::SetMuteIncomingForLectureMode(PartyRsrcID lecturerId)
{
	m_pConfAppMngrInterface->SetLecturerParty(lecturerId, m_pCommConf->GetMuteIncomingPartiesLectureMode());
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnAddAndConnectRecordingLink(CSegment* pParam)
{
	BYTE rMuteVideo = NO;

	CConfPartyManagerLocalApi* pConfPartyMngrApi = (CConfPartyManagerLocalApi*)CProcessBase::GetProcess()->GetManagerApi();

	PTRACE2(eLevelInfoNormal,"CConf::OnAddAndConnectRecordingLink : Name - ",m_name);

	*pParam >> rMuteVideo;

	pConfPartyMngrApi->AddRecordingLinkParty(m_monitorConfId, rMuteVideo);
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnDisconnectRecordingLink(CSegment* pParam)
{
	CConfPartyManagerLocalApi* pConfPartyMngrApi = (CConfPartyManagerLocalApi*)CProcessBase::GetProcess()->GetManagerApi();

	PTRACE2(eLevelInfoNormal,"CConf::OnDisconnectRecordingLink : Name - ",m_name);

	pConfPartyMngrApi->DisconnectRecordingLinkParty(m_monitorConfId);
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnCGStartContent(CSegment* pParam)
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	PTRACE(eLevelInfoNormal,"CConf::OnCGStartContent - ERROR - system is not CG!!");
    	return;
    }

	char  name[H243_NAME_LEN];
    *pParam >> name;
    CPartyConnection*  pPartyConnection = NULL;
    pPartyConnection = GetPartyConnection(name);

    if (pPartyConnection == NULL)
    {
        PTRACE2(eLevelInfoNormal,"CConf::OnCGStartContent: faild to get pPartyConnection, ",m_name);
        return;
    }
    PTRACE2(eLevelInfoNormal,"CConf::OnCGStartContent : Name - ",name);
    pPartyConnection->StartCGContent();
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnCGStoptContent(CSegment* pParam)
{
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator)
    {
    	PTRACE(eLevelInfoNormal,"CConf::OnCGStoptContent - ERROR - system is not CG!!");
    	return;
    }

    char  name[H243_NAME_LEN];
    *pParam >> name;
    CPartyConnection*  pPartyConnection = NULL;
	pPartyConnection = GetPartyConnection(name);

	if (pPartyConnection == NULL)
	{
	        PTRACE2(eLevelInfoNormal,"CConf::OnCGStopContent: faild to get pPartyConnection, ",m_name);
	        return;
	}
	PTRACE2(eLevelInfoNormal,"CConf::OnCGStopContent : Name - ",name);
	pPartyConnection->StopCGContent();
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnMcuAddPartyConnect(CSegment* pParam)   // shiraITP - 2  //shiraITP - 47
{
	PTRACE2(eLevelInfoNormal, "CConf::OnMcuAddPartyConnect : Name - ", m_name);
	DWORD listId    = 0;
	DWORD partyType = 0;

	*pParam >> partyType;

	CRsrvParty* pRsrvParty = new CRsrvParty;

	CIstrStream istream(*pParam);
	pRsrvParty->DeSerialize(NATIVE, istream);
	istream >> listId;

	CConfParty* pConfParty = new CConfParty(*(CRsrvParty*)pRsrvParty);
	pConfParty->SetLobbyId((DWORD)listId);
	pConfParty->SetPartyType((eTypeOfLinkParty)partyType);

	PTRACE2(eLevelInfoNormal, "CConf::OnMcuAddPartyConnect : MsftAvmcuState= ", enMsftAvmcuStateNames[pConfParty->GetMsftAvmcuState()]);

	// check consistancy between conf reservation and auto party add

	const char* name = pConfParty->GetName();

	char* pStr = (char*)strstr(name, "_InvitedByPcm");
	BYTE  bRecordingLinkParty = pConfParty->GetRecordingLinkParty();
	if (pStr != NULL)
		SetVisualNameForInvitedParty(pConfParty);

	PASSERT(!m_isGateWay && (pRsrvParty->GetUndefinedType() == UNRESERVED_PARTY) && (listId == 0) && (pStr == NULL) && (pRsrvParty->GetRefferedToUri() == '\0') && bRecordingLinkParty == NO);

	if (m_isGateWay)
	{
		if (m_pCommConf->GetNumParties() == 0)
		{
			TRACEINTO << "PartyName:" << pConfParty->GetName() << ", PartyId:" << pConfParty->GetPartyId() << " - Added first party to GW session";
			// first gw party - set as initator
			if (m_pCommConf->GetIsVideoInvite())
				pConfParty->SetGatewayPartyType(eInviter);
			else
				pConfParty->SetGatewayPartyType(eInitiatorNotInviter);

			// init local "message board"
			POBJDELETE(m_pTextOnScreenMngrForGwSession);
			m_pTextOnScreenMngrForGwSession = new CTextOnScreenMngrForGwSession(this);

			// init status
			POBJDELETE(m_pGWPartiesState);
			m_pGWPartiesState = new GW_PARTIES_STATUS;

			// set the inviter state as setup
			m_pGWPartiesState->insert(GW_PARTIES_STATUS::value_type(pConfParty->GetPartyId(), GW_SETUP));
		}
		else
		{
			TRACEINTO << "PartyName:" << pConfParty->GetName() << ", PartyId:" << pConfParty->GetPartyId() << " - Added party " << m_pCommConf->GetNumParties()+1  << " to GW session";
			pConfParty->SetGatewayPartyType(eNormalGWPartyType);
		}
	}
	else if (m_isDtmfInviteParty)
	{
		if (m_pCommConf->GetNumParties() == 0)
		{
			// first gw party - set as initator
			if (m_pCommConf->GetIsVideoInvite())
				pConfParty->SetGatewayPartyType(eInviter);
			else
				pConfParty->SetGatewayPartyType(eInitiatorNotInviter);

			// init local "message board"
			POBJDELETE(m_pTextOnScreenMngrForInvitedSession);
			m_pTextOnScreenMngrForInvitedSession = new CTextOnScreenMngrForInvitedSession(this);

			// init status
			POBJDELETE(m_pInvitedPartiesState);
			m_pInvitedPartiesState = new INVITED_PARTIES_STATUS;

			// set the inviter state as setup
			m_pInvitedPartiesState->insert(INVITED_PARTIES_STATUS::value_type(pConfParty->GetPartyId(), GW_SETUP));
		}
	}

	CCommConfDB* p = ::GetpConfDB();
	WORD addStatus = m_pCommConf->Add(*pConfParty);

	POBJDELETE(pRsrvParty);
	if (addStatus)
	{
		TRACESTRFUNC(eLevelError) << "PartyName:" << pConfParty->GetName() << ", Status:" << addStatus << " - Failed to add party to DB";
		PASSERT(addStatus);
		POBJDELETE(pConfParty);
		return;
	}

	DWORD connectionDelay = 0;

	CConfParty* pTempConfParty  = m_pCommConf->GetCurrentParty(pConfParty->GetName());
	if (NULL == pTempConfParty)
	{
		TRACESTRFUNC(eLevelError) << "PartyName:" << pConfParty->GetName() << " - Party not found";
		POBJDELETE(pConfParty);
		return;
	}

	POBJDELETE(pConfParty);
	pConfParty = pTempConfParty;


	// If Video Mute is enabled then update the Video MUTE in the VideoBridge
	if (pConfParty->GetVideoMute() == YES)
	{
		pConfParty->SetVideoMuteByOperator(YES);
	}

	if (pConfParty->GetConnectionType() == DIAL_OUT)
	{
		if (m_isGateWay)
			TRACEINTO << "PartyName:" << pConfParty->GetName() << ", PartyId:" << pConfParty->GetPartyId() << " - Dialing out";

		connectionDelay = ComputeConnectingConnectionDelay(pConfParty->GetVoice(), pConfParty->GetName(), pConfParty->GetNetInterfaceType());
		ConnectParty(pConfParty, connectionDelay); // shiraITP - 3
	}

	if (pConfParty->GetConnectionType() == DIAL_IN)
	{
		if (m_isGateWay)
			TRACEINTO << "PartyName:" << pConfParty->GetName() << ", PartyId:" << pConfParty->GetPartyId() << " - Update dial-in status";

		UpdateConfStatus();
	}

	if (listId != 0)
	{
		// send message to lobby to release the suspended call
		TRACEINTO << "PartyListId:" << listId << " - Auto add party";

		CLobbyApi* pLobbyApi = (CLobbyApi*)::GetpLobbyApi();
		pLobbyApi->ReleaseUnreservedParty(m_pCommConf->GetMonitorConfId(), pConfParty->GetPartyId(), listId); // shiraITP - 48
	}
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuAddPartyTermination(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnMcuAddPartyTermination : Name - ",m_name);
	DWORD listId = 0;
	DWORD partyType = 0;

	*pParam >> partyType;

	CRsrvParty* pRsrvParty = new CRsrvParty;

	CIstrStream istream(*pParam);
    pRsrvParty->DeSerialize(NATIVE,istream);
	istream >> listId;
	if (listId && m_pCommConf)
	{
	  //send message to lobby to reject the suspended call
	  char s[ONE_LINE_BUFFER_LEN];
	  sprintf(s,"auto add party, party list id = %u "	,listId);
	  PTRACE2(eLevelInfoNormal,"CConf::OnMcuAddPartyTermination : ",s);

	  CLobbyApi* pLobbyApi = (CLobbyApi*)::GetpLobbyApi();
	  pLobbyApi->RejectUnreservedParty(m_pCommConf->GetMonitorConfId(),listId,0);
	}

	POBJDELETE(pRsrvParty);
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuAddPartyIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnMcuAddPartyIdle - Ignored! : Name - ",m_name);
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuAddPartySetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnMcuAddPartySetup : Name - ",m_name);
		DWORD listId = 0;
    DWORD partyType = 0;

    *pParam >> partyType;

		CRsrvParty* pRsrvParty = new CRsrvParty;

		CIstrStream istream(*pParam);
	    pRsrvParty->DeSerialize(NATIVE,istream);
		istream >> listId;

		CConfParty* pConfParty = new CConfParty(*(CRsrvParty*)pRsrvParty);
		pConfParty->SetLobbyId((DWORD)listId);

		//check consistancy between conf reservation and auto party add
		const char* name = pConfParty->GetName();
		char * pStr = (char*)strstr( name, "Invited" );
		BYTE bRecordingLinkParty = pConfParty->GetRecordingLinkParty();

		PASSERT( !m_isGateWay && (pRsrvParty->GetUndefinedType()==UNRESERVED_PARTY) && (listId ==0) && (pStr == NULL) && (pRsrvParty->GetRefferedToUri() == '\0') && bRecordingLinkParty == NO);

		if (m_isGateWay)
		{
			if (m_pCommConf->GetNumParties() == 0)
			{
				//first gw party - set as initator
				if(m_pCommConf->GetIsVideoInvite())
					pConfParty->SetGatewayPartyType(eInviter);
				else
					pConfParty->SetGatewayPartyType(eInitiatorNotInviter);

				// init local "message board"
				m_pTextOnScreenMngrForGwSession = new CTextOnScreenMngrForGwSession(this);

				// init status
				m_pGWPartiesState = new GW_PARTIES_STATUS;
				// set the inviter state as setup
				m_pGWPartiesState->insert(GW_PARTIES_STATUS::value_type(pConfParty->GetPartyId(),GW_SETUP));
			}

			if (m_isDtmfInviteParty)
			{
				if (m_pCommConf->GetNumParties() == 0)
				{
					//first gw party - set as initator
					if(m_pCommConf->GetIsVideoInvite())
						pConfParty->SetGatewayPartyType(eInviter);
					else
						pConfParty->SetGatewayPartyType(eInitiatorNotInviter);

					// init local "message board"
					m_pTextOnScreenMngrForInvitedSession = new CTextOnScreenMngrForInvitedSession(this);

					// init status
					m_pInvitedPartiesState = new INVITED_PARTIES_STATUS;
					// set the inviter state as setup
					m_pInvitedPartiesState->insert(INVITED_PARTIES_STATUS::value_type(pConfParty->GetPartyId(),GW_SETUP));
				}
			}
			else
				pConfParty->SetGatewayPartyType(eNormalGWPartyType);
		}

		CCommConfDB* p = ::GetpConfDB();
		WORD addStatus = m_pCommConf->Add(*pConfParty);

		POBJDELETE(pRsrvParty);

		if (addStatus)
		{
			PASSERT(addStatus);
			ALLOCBUFFER(buf,H243_NAME_LEN*4);
			sprintf(buf, "CConf::OnMcuAddPartySetup : Failed to add party to DB, Conf Name: %s, Party Name - %s, Status: %d",m_name, pConfParty->GetName(),addStatus);
			PTRACE2(eLevelInfoNormal," ---> ",buf);
			DEALLOCBUFFER(buf);
			POBJDELETE(pConfParty);
			return;
		}

		if (listId != 0)
		{
			// save info to later send message to lobby to release the suspended call
			char s[ONE_LINE_BUFFER_LEN];
			sprintf(s, "save release info, party id = %u, party list id = %u ",
				pConfParty->GetPartyId(),
				listId);
			PTRACE2(eLevelInfoNormal, "CConf::OnMcuAddPartySetup : ", s);

			m_PartiesToReleaseFromLobby[pConfParty->GetPartyId()] = listId;
		}

		POBJDELETE(pConfParty);
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnMcuDelPartyConnect(CSegment* pParam)
{
	char name[H243_NAME_LEN];
	WORD mode, discCause;
	*pParam >> name >> mode >> discCause;

	TRACEINTO << "ConfName:" << m_name << ", PartyName: " << name << ", Mode:" << mode << ", DiscCause:" << discCause;

	std::ostringstream msg;
	msg << "Start Disconnection From EMA/DMA - confRsrcId: " << m_ConfRsrcId;

	CPartyConnection* pPartyConnection = GetPartyConnection(name);
	if (pPartyConnection)
	{
		CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
		// Send Content to Lync when plugin leaves
		if (pConfParty && pConfParty->GetIsLyncPlugin())
		{
			// Send content to Lync
			LegacyOnDemandCheck(pPartyConnection, FALSE);
		}
		msg << " , partyRsrcId: " << pPartyConnection->GetPartyRsrcId();
	}
	else
		msg << " , pPartyConnection is NULL";

	TRACEINTO << msg.str().c_str();

	DelParty(name, mode, discCause);
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnMcuDelPartyTermination(CSegment* pParam)
{
	PTRACE(eLevelError,"CConf::OnMcuDelPartyTermination-trying to del party while conf terminates-ignore. ");
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuDelPartyCViolent(CSegment* pParam)
{
  char name[H243_NAME_LEN];
  WORD mode, discCause;
  DWORD taskId;
  *pParam >> name >> mode >> discCause >> taskId;

  CSmallString sstr;
  sstr << "ConfName: " << m_name << ", PartyName: " << name;
  PTRACE2(eLevelError, "CConf::OnMcuDelPartyCViolent - ", sstr.GetString());

  DelParty(name, mode, discCause,true, taskId);
  //DelParty(name, mode, discCause);
  /*
  CPartyConnection* pPartyConnection = GetPartyConnection(name);
  if (pPartyConnection->GetInterfaceType() == H323_INTERFACE_TYPE)
  {

	 CH323DelPartyCntl* pPartyCntl = (CH323DelPartyCntl*)pPartyConnection->GetPartyCntl();
	 pPartyCntl->SetIsViolentDestroy(true);
  }
 // pPartyConnection->GetPartyCntl()->SetIs*/

}
////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuReconnectPartyConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnMcuReconnectPartyConnect : Name - ",m_name);

	char  name[H243_NAME_LEN];
	BYTE  bUseDelay;
	WORD  temp;
	*pParam >> name
		>> temp;
	bUseDelay = (BYTE)temp;

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(name);
	// Romem klocwork
	if(pConfParty == NULL)
	{
		CMedString msg1;
		msg1 << "Name - " << m_name << "," << name;
		PTRACE2(eLevelInfoNormal,"CConf::OnMcuReconnectPartyConnect  : Party is not in Conf DB ", msg1.GetString());
		return;
	}
	DWORD redialInterval = 0;
	DWORD connectDelay = ComputeConnectingConnectionDelay(pConfParty->GetVoice(),pConfParty->GetName(),pConfParty->GetNetInterfaceType());

	CPartyConnection*  pPartyConnection = NULL;
	pPartyConnection = GetPartyConnection(name);
	if (!pPartyConnection)
	{
		if (pConfParty->IsFirstConnectionAfterHotBackupRestore())
		{
			/*VNGR-26431: David Liang   2012/7/27 */
			/*The pPartyConnection can be NULL under hotbackup scenario*/
			/*When a disconnected party is synced from master, the party will without pPartyConnection on Slave*/
			/*When the slave becomes master, it will not dialout for disconnected party*/
			/*When the operator connected the party from EMA again, we should permit to connect part again*/
			pConfParty->SetFirstConnectionAfterHobackupRestore(FALSE);
			ConnectParty(pConfParty, connectDelay);
			return;
		}

		CMedString trace_str;
		trace_str << "Party name = " << name;
		trace_str << ". Conf name = "  << m_name << "\n";
		PTRACE2(eLevelError,"CConf::OnMcuReconnectPartyConnect: Party connection wasn't found. ",trace_str.GetString());
		DBGPASSERT_AND_RETURN(1);
	}

  	if (!m_pCommConf->IsConfSecured())
  	{
		if(pConfParty->IsIpNetInterfaceType())
		{
			ReconnectIpParty(pConfParty,pPartyConnection,redialInterval,connectDelay);
			return;
		}
		// for PSTN / ISDN
		else if (pConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE)
		{
			ReconnectPstnIsdnParty(pConfParty,pPartyConnection, redialInterval, connectDelay);
			return;
		}
  	}
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuSetEndTimeConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnMcuSetEndTimeConnect : Name - ",m_name);
	SetEndTime(pParam);
}

////////////////////////////////////////////////////////////////////////////
void  CConf::SetEndTime(CSegment* pParam, BOOL bSetAlertTout)
{
	PTRACE2(eLevelInfoNormal,"CConf::SetEndTime : DeleteTimer(CONFTOUT) ",m_name);

	DeleteTimer(CONFTOUT);

	CStructTm  curTime,endTime;
	STATUS timeStatus = SystemGetTime(curTime);
  if (STATUS_OK == timeStatus)
  {
		CIstrStream  Istr(*pParam);
		endTime.DeSerialize(Istr);

		m_pCommConf->SetEndTime(endTime);
		DWORD  durationTime = endTime - curTime;
		PTRACE2(eLevelInfoNormal,"CConf::SetEndTime : StartTimer(CONFTOUT) ",m_name);

		DWORD trimmed_duration = (durationTime >= 5) ? (durationTime - 5) : durationTime;
		// we trim the duration in 5 seconds in order to prevent resource release lateness (sagi)

		CSmallString sstr;
		sstr << "time out set for " << trimmed_duration << " seconds, " << m_name;
		PTRACE2(eLevelInfoNormal,"CConf::SetEndTime : StartTimer(CONFTOUT) ",sstr.GetString());

		StartTimer(CONFTOUT,trimmed_duration * SECOND);

		if (bSetAlertTout)
		{
			PTRACE2(eLevelInfoNormal,"CConf::SetEndTime : DeleteTimer(ALERTOUT) ",m_name);
			CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			BOOL IsEnableAutoExtension = FALSE;
			std::string key = "ENABLE_AUTO_EXTENSION";
			sysConfig->GetBOOLDataByKey(key, IsEnableAutoExtension);

			DWORD ExtensionTimeInterval  = 0;
			key = "EXTENSION_TIME_INTERVAL";
			sysConfig->GetDWORDDataByKey(key, ExtensionTimeInterval);
			if( IsEnableAutoExtension )
			{
				DeleteTimer(ALERTOUT);
				if( durationTime <= (DWORD)ExtensionTimeInterval*60 )
				{
					PTRACE2(eLevelError,"CConf::SetEndTime : Can not start timer ALERTOUT - duration time is less than alert time - ",m_name);
				}
				else
				{
					PTRACE2(eLevelInfoNormal,"CConf::SetEndTime : StartTimer(ALERTOUT) ",m_name);
					StartTimer(ALERTOUT,(durationTime-(ExtensionTimeInterval*60))*SECOND);
				}
			}
			else
			{
				StartTimer(ALERTOUT,(durationTime-(m_pCommConf->GetAlertToneTiming()*60))*SECOND);
				PTRACE2(eLevelInfoNormal,"CConf::SetEndTime : StartTimer(ALERTOUT)2 ",m_name);
			}
		}
	}
  else
  {
		PTRACE(eLevelError,"CConf::SetEndTime : GetTime failed - end time not set");
		PASSERT(1);
  }
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnMcuSetAutoRedial (CSegment* pParam)
{
    BYTE bAutoRedial = m_pCommConf->GetIsAutoRedial();
    PTRACE2INT (eLevelInfoNormal, "CConf::OnMcuSetAutoRedial bAutoRedial=", bAutoRedial);
    *pParam >> bAutoRedial;
    m_pCommConf->SetAutoRedial(bAutoRedial);
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuTerminateConfConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnMcuTerminateConfConnect : Name - ",m_name);

	//In case of GW call two terminate commands might be sent.
	//We accept only the first one.
    WORD numOfNonDisconnectedParties = 0;
	if(m_state == TERMINATION)
		return;

	DeleteTimer(ALERTOUT);
	DeleteTimer(CONFTOUT);
	Destroy();

	SetConfTerminateCause(TERMINATE_BY_OPERATOR);
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnMcuForceKillAnyCase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnMcuForceKillAnyCase : Name - ",m_name);

	 m_selfKill = TRUE;
}

////////////////////////////////////////////////////////////////////////////
// Ask dest Conf it is is ready for a move (has a RSRC ID)
void CConf::OnSourcePartyIsConfReadyForMoveConnect(CSegment* pParam)
{
	STATUS status = STATUS_OK;
	PTRACE2(eLevelInfoNormal,"OnSourcPartyIsConfReadyForMoveConnect : Name - ",m_name);
	ResponedClientRequest(status);
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnSourcePartyIsConfReadyForMoveIdle(CSegment* pParam)
{
	STATUS status = STATUS_FAIL;
	PTRACE2(eLevelInfoNormal,"OnSourcPartyIsConfReadyForMoveIdle : Name - ",m_name);
	ResponedClientRequest(status);
}

//--------------------------------------------------------------------------
// this function is starting the move process in the source conference
// the function changed to support move of H323 party move
void CConf::OnMcuExportParty(CSegment* pParam)
{
	ConfMonitorID  monitorDestConfId;
	PartyMonitorID monitorPartyId;
	EMoveType      eMoveType = eMoveDummy;
	WORD           wMoveType = 0xFF;
	*pParam >> monitorPartyId >> monitorDestConfId >> wMoveType;
	eMoveType = (EMoveType)wMoveType;

	TRACEINTO << "PartyMonitorId:" << monitorPartyId << ", DestConfMonitorId:" << monitorDestConfId << ", MoveType:" << eMoveType;

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(monitorPartyId);
	PASSERTSTREAM_AND_RETURN(!pConfParty, "Failed, Party can not be found in DB, monitorPartyId:" << monitorPartyId);

	// Remove "exclusive content" state for moved party
	pConfParty->SetExclusiveContentOwner(FALSE);

	ExportParty(monitorPartyId, monitorDestConfId, eMoveType);
}

////////////////////////////////////////////////////////////////////////////
void  CConf::OnH323LobbyAddPartyConnect(CSegment* pParam)//shiraITP - 60
{
    DeleteTimer(AUTOTERMINATE);
    CTaskApp*  pParty;
    ALLOCBUFFER(name,H243_NAME_LEN);

    *pParam >> (DWORD&)pParty;
    *pParam >> name;

	CMedString msg;
	msg << "Name - " << m_name << "," << name;
	PTRACE2(eLevelInfoNormal,"CConf::OnH323LobbyAddPartyConnect : ", msg.GetString());

	//this is like the NET connection need one day to be remove or change to H323
	CreateConfPartyIfNeeded(name);
    CConfParty* pConfParty = m_pCommConf->GetCurrentParty(name);
    // Romem clokwork
    if(pConfParty == NULL)
    {
    	CMedString msg1;
    	msg1 << "Name - " << m_name << "," << name;
    	PTRACE2(eLevelInfoNormal,"CConf::OnH323LobbyAddPartyConnect : Party is not in Conf DB ", msg1.GetString());
		DEALLOCBUFFER(name);
    	return;
    }


	const char* serviceName  = pConfParty->GetServiceProviderName();//"sip and 323";//
	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
	CConfIpParameters* pServiceParams = pIpServiceListManager->GetRelevantService(serviceName, pConfParty->GetNetInterfaceType());
	if (pServiceParams == NULL)
	{
	    PASSERTMSG(1,"CConf::OnH323LobbyAddPartyConnect - No IP Service exsists!!!");
	}

	mcTransportAddress partyAddr = pConfParty->GetIpAddress();
	if (pServiceParams && (!::isApiTaNull(&partyAddr)) )
	{
		// Validity check to see that we support correct IP Vs. the EP
		BYTE  isMatch = ::isIpVersionMatchBetweenPartyAndService(&partyAddr, pServiceParams);
		if (isMatch == FALSE)
		{
			PASSERTMSG(2,"CConf::OnH323LobbyAddPartyConnec - No match between service and EP ip types!!!");
			DEALLOCBUFFER(name);
		   	return;
		}
	}

	CH323NetSetup* pIpNetSetup  = new CH323NetSetup;
    pIpNetSetup->DeSerialize(NATIVE,*pParam);
	COsQueue*  pPartyRcvMbx = new COsQueue;
	pPartyRcvMbx->DeSerialize(*pParam);

	SetupConfPartyOnH323(pConfParty,pIpNetSetup);

	CPartyConnection* pPartyConnection = new CPartyConnection;
	PartyControlDataParameters			partyControlDataParams;
	PartyControlInitParameters			partyControInitParam;

	memset( &partyControlDataParams, 0, sizeof (PartyControlDataParameters) );
	memset( &partyControInitParam, 0, sizeof (PartyControlInitParameters) );

	SetControlDataParams(partyControlDataParams, pConfParty,pIpNetSetup,pParty,NULL,FALSE,FALSE,FALSE,Regular,No_Lync,0,pConfParty->GetPartyType(),pIpNetSetup->GetRemoteSetupRate());
	AjustPartyAndRoomIdByLinkType(partyControlDataParams, pConfParty);
	SetControlInitParams(partyControInitParam,pConfParty,FALSE,"",0,0,0,pPartyRcvMbx);
	pPartyConnection->ConnectIP(partyControInitParam,partyControlDataParams);
	InsertPartyConnection(pPartyConnection);

	DEALLOCBUFFER(name);

	UpdateDB(pParty,PARTYSTATE,PARTY_CONNECTING);
	UpdateDB(pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);

	CSegment* pRspMsg = new CSegment;
	*pRspMsg << (WORD)statOK;
	ResponedClientRequest(H323ADDINPARTY, pRspMsg);
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnH323LobbyRejectParty(CSegment* pParam)
{
     CTaskApp*  pParty;
     ALLOCBUFFER(name,H243_NAME_LEN);

     *pParam >> (DWORD&)pParty;
     *pParam >> name;

	CMedString msg;
	msg << "conf name: " << m_name << ", Party name: " << name << "Arrive at state: " << m_state;
	PTRACE2(eLevelInfoNormal,"CConf::OnH323LobbyRejectParty : Reject because of - ",msg.GetString());

	CSegment* pRspMsg = new CSegment;
	*pRspMsg << (WORD)statIllegal;
	ResponedClientRequest(H323ADDINPARTY, pRspMsg);
	DEALLOCBUFFER(name);
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnSipLobbyAddPartyConnect(CSegment* pParam)
{
	CTaskApp* pParty = NULL;
  ALLOCBUFFER(name, H243_NAME_LEN);

  CConfParty* pConfParty;
  BYTE IsOfferer = FALSE;
  BYTE bIsMrcHeader = FALSE;
  BYTE bIsWebRtcCall = FALSE;
  BYTE bIsMrcCall = FALSE;
  RemoteIdent epType = Regular;
  LyncConnType lyncEpType = No_Lync;
  eIsUseOperationPointsPreset originalIsUseOperationPointesPresets = eIsUseOPP_No;
  eIsUseOperationPointsPreset isUseOperationPointesPresets = eIsUseOPP_No;
  BOOL bIsRemoteSlave = FALSE;
  BYTE	initVideoLayout = 0;

  *pParam >> (DWORD&)pParty;
  *pParam >> name;
  *pParam >> IsOfferer;
  *pParam >> bIsMrcHeader;
  *pParam >> bIsWebRtcCall;
  *pParam >> (BYTE&)lyncEpType;
  *pParam >> (BYTE&)epType;
  *pParam >> (BYTE&)originalIsUseOperationPointesPresets;
  *pParam >> (BYTE&)bIsRemoteSlave;
  *pParam >> initVideoLayout;

//  bIsMrcHeader=TRUE;

  eConfMediaType confMediaType = (eConfMediaType)m_pCommConf->GetConfMediaType();
  if (confMediaType == eMixAvcSvcVsw )
  {
	  TRACEINTO<<"avc_vsw_relay: confMediaType is:"<<confMediaType<<" bIsMrcHeader is:"<<((bIsMrcHeader==TRUE ? "YES":"NO"));
	  if (bIsMrcHeader)
		  bIsMrcCall = TRUE;
  }
  else
  {
	  if (confMediaType == eSvcOnly)
	  {
		  bIsMrcCall = TRUE;
		  if (!bIsMrcHeader) // This case is blocked in ConfPartyManager level.
			  PASSERTMSG(1,"CConf::OnSipLobbyAddPartyConnect - AVC call in SVC only conf.");
	  }
	  else if (confMediaType == eMixAvcSvc)
	  {
		  if (bIsMrcHeader)
			  bIsMrcCall = TRUE;
	  }
  }

  // BRIDGE-6844
  // if RemoteIdent is unrecognized but it's SVC call, then do not use OperationPointesPresets
  isUseOperationPointesPresets = originalIsUseOperationPointesPresets;
  if ( (eIsUseOPP_Yes_Unrecognized == originalIsUseOperationPointesPresets) && (TRUE == bIsMrcCall) )
  {
	  isUseOperationPointesPresets = eIsUseOPP_No;
  }

  TRACEINTO << "ConfName=" << m_name << ", PartyName=" << name << ", isOfferer=" << (IsOfferer ? "yes" : "no" )
		    << ", bIsMrcCall=" << (bIsMrcCall ? "yes" : "no") << ", bIsWebRtcCall=" << (bIsWebRtcCall ? "yes" : "no")
		    << ", lyncEpType   "<<lyncEpType<<", epType=" << epType
		    << "\noriginalIsUseOperationPointesPresets=" << originalIsUseOperationPointesPresets
		    << ", isUseOperationPointesPresets=" << isUseOperationPointesPresets
		    << " [No=0, Yes_SameTime=1, Yes_Unrecognized=3]"
                    << ", bIsRemoteSlave=" << (bIsRemoteSlave ? "TRUE" : "FALSE");


	CreateConfPartyIfNeeded(name);
	pConfParty = m_pCommConf->GetCurrentParty(name);
	// Romem clokwork
	if(pConfParty == NULL)
	{
		CMedString msg1;
		msg1 << "Name - " << m_name << "," << name;
		PTRACE2(eLevelInfoNormal,"CConf::OnSipLobbyAddPartyConnect : Party is not in Conf DB ", msg1.GetString());
		CSegment* pRspMsg = new CSegment;
		*pRspMsg << (WORD)statIllegal;
		ResponedClientRequest(SIPADDINPARTY, pRspMsg);
		return;
	}


	//contniue the Auto-Termination if the RL dials-in/CSS plugin/CSS Gateway
	if(YES != pConfParty->GetRecordingLinkParty()
		&& (epType != MicrosoftEP_Lync_CCS)
		&&(epType != Polycom_Lync_CCS_Gw)) //equal to (lyncEpType != Lync_Addon)
	{
		DeleteTimer(AUTOTERMINATE);
	}

	pConfParty->SetIsLyncPlugin((lyncEpType == Lync_Addon));
	CSipNetSetup * pNetSetup  = new CSipNetSetup;

	// update party ip address
	pNetSetup->DeSerialize(NATIVE,*pParam);
	EStat setupStat = SetupConfPartyOnSIP(pConfParty, pNetSetup, bIsMrcHeader, bIsRemoteSlave);

	// attach resources to conf
	COsQueue*	pPartyRcvMbx = new COsQueue;
	pPartyRcvMbx->DeSerialize(*pParam);

	// VNGR-6679 - Solution
	CSipCaps* pRmtCaps = new CSipCaps;

	if ( !pRmtCaps )
	{
		PTRACE(eLevelError,"CConf::OnSipLobbyAddPartyConnect : NULL pointer to CSipCaps  remote caps object");
	}
	else
	{
		// update party ip address
		pRmtCaps->DeSerialize(NATIVE,*pParam);
		CSuperLargeString strCaps1;//N.A. DEBUG VP8
		pRmtCaps->DumpToString(strCaps1);
		PTRACE2(eLevelInfoNormal, "N.A. DEBUG pRmtCaps->Dump", strCaps1.GetString());
	}

	//eFeatureRssDialin
	if(pConfParty && (pConfParty->GetRecordingLinkParty()||pConfParty->GetPlaybackLinkParty()))
	{
		pConfParty->SetLastLayoutForRL(initVideoLayout);
	 	TRACEINTO<<"intiVideolayout for Recording link is: " << initVideoLayout;
	}

	CPartyConnection* pPartyConnection = new CPartyConnection;
	PartyControlDataParameters			partyControlDataParams;
	PartyControlInitParameters			partyControInitParam;

	memset( &partyControlDataParams, 0, sizeof (PartyControlDataParameters) );
	memset( &partyControInitParam, 0, sizeof (PartyControlInitParameters) );

	SetControlDataParams(partyControlDataParams, pConfParty,pNetSetup,pParty,pRmtCaps,IsOfferer,bIsMrcHeader,bIsWebRtcCall,epType,lyncEpType,pConfParty->GetRoomId(),pConfParty->GetPartyType(),pNetSetup->GetRemoteSetupRate(), isUseOperationPointesPresets);
	AjustPartyAndRoomIdByLinkType(partyControlDataParams, pConfParty);
	SetControlInitParams(partyControInitParam,pConfParty,FALSE,"",0,0,0,pPartyRcvMbx);
	pPartyConnection->ConnectIP(partyControInitParam,partyControlDataParams);
	InsertPartyConnection(pPartyConnection);

	DEALLOCBUFFER(name);
	UpdateDB(pParty,PARTYSTATE,PARTY_CONNECTING);
	UpdateDB(pParty,DISCAUSE,NO_DISCONNECTION_CAUSE);

	CSegment* pRspMsg = new CSegment;

	if (statOK == setupStat)
	{
		*pRspMsg << (WORD)statOK;
	}
	else
	{
		*pRspMsg << (WORD)statIllegal;
		TRACEINTO << "statIllegal";
	}
	ResponedClientRequest(SIPADDINPARTY, pRspMsg);
}


////////////////////////////////////////////////////////////////////////////
void  CConf::OnSipLobbyRejectParty(CSegment* pParam)
{
     CTaskApp*  pParty;
     ALLOCBUFFER(name,H243_NAME_LEN);

     *pParam >> (DWORD&)pParty;
     *pParam >> name;

	CMedString msg;
	msg << "conf name: " << m_name << ", Party name: " << name << "Arrive at state: " << m_state;
	PTRACE2(eLevelInfoNormal,"CConf::OnSipLobbyRejectParty : Reject because of - ",msg.GetString());

	CSegment* pRspMsg = new CSegment;
	*pRspMsg << (WORD)statIllegal;
	ResponedClientRequest(SIPADDINPARTY, pRspMsg);
	DEALLOCBUFFER(name);
}

//////////////////////////////////////////////////////////////////////////
void CConf::OnPartyEndAddConnect(CSegment* pParam)  // shiraITP - 30
{
	CTaskApp* pParty;
	WORD status;

	*pParam >> (DWORD&)pParty >> status;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);

	if (pPartyConnection)
	{

		TRACEINTO << "PartyId:" << pPartyConnection->GetPartyRsrcId() << ", Status:" << status << ", IsDisconnected:" << (int)pPartyConnection->IsDisconnect() << ", InterfaceType:" << pPartyConnection->GetInterfaceType();

		CConfParty* pConfParty = GetCommConf()->GetCurrentParty(pPartyConnection->GetMonitorPartyId());

		switch (status)
		{
			case STATUS_OK:
			{ // add o.k.
				if (pPartyConnection->IsDisconnect())  // if delete request while adding to conf
				{
					if (pPartyConnection->GetInterfaceType() == H323_INTERFACE_TYPE)
						pPartyConnection->DisconnectH323(pPartyConnection->GetDisconnectMode());
					else if (pPartyConnection->GetInterfaceType() == SIP_INTERFACE_TYPE)
						pPartyConnection->DisconnectSip(pPartyConnection->GetDisconnectMode());
					else if (pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE)
					{
						if (pPartyConnection->GetVoice())
							pPartyConnection->DisconnectPstn();
						else
							pPartyConnection->DisconnectIsdn();
					}
				}
				else
				{
					// VNGR-26449 - unencrypted conference message
					SendSecureMessageWhenAddParty(pPartyConnection);

					if (m_isGateWay && pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE && pPartyConnection->GetDialType() == DIALOUT && !pPartyConnection->GetVoice())
					{
						CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
						PASSERT_AND_RETURN(!pConfParty);
						CSmallString cstr;
						cstr << pConfParty->GetPhoneNumber() << "[90%]";
						AddPartyMsgOnScreenForGWConf(pConfParty->GetPartyId(), cstr.GetString());
					}

					if (m_isDtmfInviteParty && pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE && pPartyConnection->GetDialType() == DIALOUT && !pPartyConnection->GetVoice())
					{
						CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
						PASSERT_AND_RETURN(!pConfParty);
						CSmallString cstr;
						cstr << pConfParty->GetPhoneNumber() << "[90%]";
						AddPartyMsgOnScreenForInvitedConf(pConfParty->GetPartyId(), cstr.GetString());
					}

					if (pConfParty)
					{
						m_pCommConf->OnPartyAdd(*pConfParty);
						pConfParty->SetTask(pParty);
					}

					if (m_pCommConf->GetIsAsSipContent() && m_IsAsSipContentEnable)
					{
						pPartyConnection->SetTimerForAsSipAddContentIfNeeded();
					}
					if (pConfParty && pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone)
						TRACEINTO << "PartyId:" << pPartyConnection->GetPartyRsrcId() << ", AvmcuState:" << pConfParty->GetMsftAvmcuState() << ", FocusUri:" <<m_pCommConf->GetFocusUriScheduling();
					if (pConfParty && pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone &&  pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveOut && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveIn )
					{
						if(GetCommConf() && !strlen(GetCommConf()->GetFocusUriScheduling()) )
						{
							TRACEINTO << "PartyId:" << m_AvMcuPartyRsrcId << " - av-mcu register - meet now";
							EventPackage::Manager& lyncEventManager = EventPackage::Manager::Instance();
							lyncEventManager.AddSubscriber(pPartyConnection->GetPartyRsrcId(), EventPackage::eEventType_MediaDeleted, std::make_pair((COsQueue*)&(GetRcvMbx()), MS_LYNC_CONF_PARTY_CNTL_MSG));
						}
						else
							TRACEINTO << "PartyId:" << m_AvMcuPartyRsrcId << " - av-mcu not register - schedule";
						m_AvMcuPartyRsrcId = pPartyConnection->GetPartyRsrcId();
					}
					else if( pConfParty && (pConfParty->GetMsftAvmcuState() == eMsftAvmcuNone ) && strlen(m_pCommConf->GetFocusUriScheduling()) )
					{
						if (m_AvMcuPartyRsrcId == 0)
						{
							std::vector<DWORD> partyStates;
							partyStates.push_back(PARTY_CONNECTED);
							partyStates.push_back(PARTY_CONNECTING);
							partyStates.push_back(PARTY_CONNECTED_PARTIALY);
							partyStates.push_back(PARTY_SECONDARY);
							partyStates.push_back(PARTY_CONNECTED_WITH_PROBLEM);

							DWORD NumOfConnectedParties = GetNumberOfParticpants(partyStates);
							if (NumOfConnectedParties == 1)
							{
								TRACEINTO << "reconnect av-mcu link for schedule! ";
								AddAvMCUParty(m_pCommConf->GetFocusUriScheduling());

								for (WORD i = 0; i < m_pCommConf->GetNumParties(); i++)        // dial out loop
								{
									if (pConfParty->CanConnectParty())
									{
										DWORD connectDelay = 0;
										connectDelay = ComputeConnectingConnectionDelay(pConfParty->GetVoice(), pConfParty->GetName(), pConfParty->GetNetInterfaceType());
										if(pConfParty->GetIsDMAAVMCUParty())
										{
											TRACEINTO << "reconnect av-mcu link found connecting!! ";
											ConnectDMAAVMCUParty(pConfParty, connectDelay);
											break;
										}

									}

									WORD numOfParties = m_pCommConf->GetNumParties();
									if (i+1 < numOfParties)
									{
										int next = i+1;
										pConfParty = m_pCommConf->GetNextParty(next);
										PASSERT(!pConfParty);
										if (!pConfParty) break;
									}
								}
							}

						}
					}


					// to replace the above scm
					SetCapCommonDenominator(pParty, YES /*,HC_ADD*/);

					// Add to operate Auto terminate of meeting room for H323 calls
					m_IsAnyPartyWasConnected = TRUE;

					if (GetIsGateWay())
					{
						SetGwDtmfForwardIfNeeded();
					}
					else if (m_invitePartyMap.size() > 0
						&& (CProcessBase::GetProcess()->GetProductType() == eProductTypeSoftMCUMfw || (pConfParty && pConfParty->GetCascadeMode() != CASCADE_MODE_NONE)))
					{
						SetInvitorPartyDtmfForward();
					}
					else
					{
						CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
						if (IsValidPObjectPtr(pPartyCntl))
						{
							// Multiple links for ITP in cascaded conference feature: CConf::OnPartyEndAddConnect (SetDtmfForwardAllIfNeeded)
							BYTE isH323MainOrRegularTypeInCascade = TRUE;

							if (pPartyConnection->GetInterfaceType() == H323_INTERFACE_TYPE)
							{
								CH323PartyCntl* pH323PartyCntl = (CH323PartyCntl*)(pPartyConnection->GetPartyCntl());
								if (IsValidPObjectPtr(pH323PartyCntl) && pH323PartyCntl->GetPartyLinkType() != eRegularParty)
									isH323MainOrRegularTypeInCascade = FALSE;
							}

							if (CASCADE_MCU == pPartyCntl->GetPartyCascadeType() && isH323MainOrRegularTypeInCascade) // was before ITP feature
							{
								TRACEINTO << "PartyId:" << pPartyConnection->GetPartyRsrcId() << ", PartyCascadeType:" << (int)pPartyCntl->GetPartyCascadeType() << " - Set DTMF Forward All if needed";
								SetDtmfForwardAllIfNeeded();
							}
						}
					}
					SetMediaStateOnPartyDisconnectConnct(PARTY_CONNECTED);
				}
				PASSERT_AND_RETURN(!pConfParty);

				if(pPartyConnection->GetVoice())
				{
					TRACEINTO << "The connected party with PartyId:" << pPartyConnection->GetPartyRsrcId()
								<< "is a audio only party, update the audio indication in layout";
					UpdateAudioParticipantsCount(pConfParty,YES);
				}

				// After Plugin connected, change the matched Lync to non-legacy!
				if (pConfParty->GetIsLyncPlugin())
				{
					TRACEINTO << "PartyId:" << pPartyConnection->GetPartyRsrcId() << " - Block content when plug-in arrives";
					LegacyOnDemandCheck(pPartyConnection, TRUE);
				}
				break;
			}

			case PARTY_STAND_BY:
			{ // standby connect
				if (pConfParty)
					m_pCommConf->OnPartyAdd(*pConfParty);

				UpdateDB(pParty, PARTYSTATE, PARTY_STAND_BY);
				break;
			}

			default:
			{ // failed to add party
				TRACEINTO << "PartyId:" << pPartyConnection->GetPartyRsrcId() << " - Failed to add party";

				WORD mode = DISCONNECT_MODE; // disconnect as default
				CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
				PASSERT(!pConfParty);
				if (pConfParty)
				{
					if (pConfParty->GetUndefinedType() == UNRESERVED_PARTY)
						mode = DELETE_MODE;
					else
						mode = DISCONNECT_MODE;
				}

				if (pPartyConnection->GetInterfaceType() == H323_INTERFACE_TYPE)
				{
					pPartyConnection->DisconnectH323(mode); // AUTO ADD NOT SUUPOTED IN 323 YET

					// Multiple links for ITP in cascaded conference feature: CConf::OnPartyEndAddConnect SUB_OR_MAIN_LINK_IS_SECONDARY or RESOURCES_DEFICIENCY - disconnect mainLink -> will disconnect all room
					if (pConfParty && CPObject::IsValidPObjectPtr(pConfParty) && (pConfParty->GetPartyType() == eSubLinkParty) &&
					    (pConfParty->GetDisconnectCause() == RESOURCES_DEFICIENCY || pConfParty->GetDisconnectCause() == SUB_OR_MAIN_LINK_IS_SECONDARY))
					{
						TRACEINTO << "SubLink:" << m_name << " - Failed to connect all links, Status:RESOURCES_DEFICIENCY";

						// disconnect mainLink:
						char mainPartyName[H243_NAME_LEN];
						::GetMainLinkName(pPartyConnection->GetName(), (char*)mainPartyName);

						CPartyConnection* pMainPartyConnection = GetPartyConnection(mainPartyName);
						CPartyCntl* pMainPartyCntl = NULL;

						if (pMainPartyConnection)
							pMainPartyCntl = pMainPartyConnection->GetPartyCntl();

						if (pMainPartyCntl && CPObject::IsValidPObjectPtr(pMainPartyCntl))
						{
							CTaskApp* pMainParty = pMainPartyCntl->GetPartyTaskApp();
							if (pMainParty)
							{
								UpdateDB(pMainParty, DISCAUSE, RESOURCES_DEFICIENCY /*cause*/, NULL, 0 /*MipErrorNumber*/);
							}
							else
								PASSERTMSG(1, "MainLink has no pMainParty");
						}
						else
							PASSERTMSG(1, "MainLink has no pMainPartyCntl");

						if (pMainPartyConnection)
							pMainPartyConnection->DisconnectH323(1);       // mode = 1 = disconnect party
						else
							PASSERTMSG(1, "pMainPartyConnection is NULL");

						return;
					}
				}
				else if (pPartyConnection->GetInterfaceType() == SIP_INTERFACE_TYPE)
					pPartyConnection->DisconnectSip(mode);
				else if (pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE)
				{
					if (pPartyConnection->GetVoice())
						pPartyConnection->DisconnectPstn(mode);
					else
						pPartyConnection->DisconnectIsdn(mode);
				}
				break;
			}
		} // switch
	}
	else
		PTRACE2(eLevelError, "CConf::OnPartyEndAddConnect : \'PARTY NOT EXISTS\' ", m_name);

	// Multiple links for ITP in cascaded conference feature: CConf::OnPartyEndAddConnect (SendToMainLinkThatSubWasAdded)
	if ((pPartyConnection != NULL) && (pPartyConnection->GetInterfaceType() == H323_INTERFACE_TYPE))
	{
		CH323PartyCntl* pPartyCntl = (CH323PartyCntl*)(pPartyConnection->GetPartyCntl());

		if (IsValidPObjectPtr(pPartyCntl))
		{
			if (pParty)
			{
				if (pPartyCntl->GetPartyLinkType() == eSubLinkParty)
				{
					CPartyRsrcDesc* partyCntlAllocatedRsrc = pPartyCntl->GetPartyCntlAllocatedRsrc();
					DWORD partyRsrcID = 0;
					if (partyCntlAllocatedRsrc)
						partyRsrcID = (DWORD)(partyCntlAllocatedRsrc->GetPartyRsrcId());
					else
						TRACESTRFUNC(eLevelError) << "partyCntlAllocatedRsrc is NULL";

					SendToMainLinkThatSubWasAdded(pParty, pPartyConnection, partyRsrcID);
				}
			}
			else
				TRACESTRFUNC(eLevelError) << "LinkType:" << pPartyCntl->GetPartyLinkType() << " - pParty is NULL";
		}
	}
}

////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: SendToMainLinkThatSubWasAdded
void CConf::SendToMainLinkThatSubWasAdded(CTaskApp* pParty, CPartyConnection* pPartyConnection, DWORD subMailBox)
{
	// send message to MainLink:
	const char* subPartyName = pPartyConnection->GetName();
	char mainPartyName[H243_NAME_LEN];
	::GetMainLinkName(subPartyName, (char*)mainPartyName);

	TRACEINTO << "SubLink:" << subPartyName;

	CPartyConnection* pMainPartyConnection = GetPartyConnection(mainPartyName);
	if (pMainPartyConnection != NULL)
	{
		CH323PartyCntl* pPartyCntl = (CH323PartyCntl*)(pMainPartyConnection->GetPartyCntl());
		if (pPartyCntl && CPObject::IsValidPObjectPtr(pPartyCntl))
		{
			char* pch1 = (char*)strrchr(subPartyName, '_');
			if (pch1 && pch1+1)
			{
				DWORD indexOfSub = (DWORD)atoi(pch1+1);
				pPartyCntl->OnAddSubLinkToRoomControl(subPartyName, indexOfSub-1, eSubLinkParty, subMailBox);
			}
		}
	}
	else
	{
		// disconnect subLink:
		PASSERTSTREAM(1, "SubLink:" << subPartyName << " - MainParty was not found, disconnect subLink");

		UpdateDB(pParty, PARTYSTATE, PARTY_DISCONNECTING);
		UpdateDB(pParty, DISCAUSE, MCU_INTERNAL_PROBLEM /*cause*/, NULL, 0 /*MipErrorNumber*/);

		pPartyConnection->DisconnectH323(0);     // mode = 0 = delete party
	}
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnPartyEndDelConnect(CSegment* pParam)
{

	OnPartyEndDel(pParam);

	SetMediaStateOnPartyDisconnectConnct(PARTY_DISCONNECTED);

}
////////////////////////////////////////////////////////////////////////////
void CConf::OnPartyEndDelTerminate(CSegment* pParam)
{
	OnPartyEndDel(pParam);

	WORD numParties = m_pPartyList ? m_pPartyList->entries() : 0;
	if ( numParties == 0 )  // no active parties in conf
		OnEndDelAllParties();
}

//--------------------------------------------------------------------------
void CConf::OnPartyRedialConnect(CSegment* pParam)
{
	CTaskApp*	pParty = NULL;
	WORD IsHotBackupRedial;
	*pParam >> (DWORD&)pParty >> IsHotBackupRedial;

	TRACEINTO << "PartyId:" << pParty << ", IsHotBackupRedial:" << IsHotBackupRedial;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	PASSERT_AND_RETURN(!pPartyConnection);

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	PASSERT_AND_RETURN(!pConfParty);

	DWORD redialIntervalInSeconds = GetSystemCfgFlagInt<DWORD>(CFG_KEY_REDIAL_INTERVAL_IN_SECONDS)*SECOND;
	if (IsHotBackupRedial)
		redialIntervalInSeconds = GetSystemCfgFlagInt<DWORD>(CFG_KEY_HOT_BACKUP_REDIAL_INTERVAL_IN_SECONDS)*SECOND;

	if (pConfParty->IsIpNetInterfaceType())
		ReconnectIpParty(pConfParty, pPartyConnection, redialIntervalInSeconds, 0);
	else if (pConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE) // for PSTN / ISDN
		ReconnectPstnIsdnParty(pConfParty, pPartyConnection, redialIntervalInSeconds);
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnPartyLayoutChanged(CSegment* pParam)
{
}

////////////////////////////////////////////////////////////////////////////
void CConf::OnPartyDisConnectConnect(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CConf::OnPartyDisConnectConnect : ");

	CConfParty* pConfParty = NULL;
	CTaskApp* pParty = NULL;
	WORD cause = 0;
	WORD mode = 0;
	DWORD PartyID = 0;
	DWORD addrLen = 0;
	char* alternativeAddrStr = NULL;
	DWORD MipErrorNumber = 0;

	*pParam >> (DWORD&)pParty >> cause;
	*pParam >> addrLen;
	if (addrLen)
	{
		alternativeAddrStr = new char[addrLen];
		pParam->Get((BYTE*)alternativeAddrStr, addrLen);
		alternativeAddrStr[addrLen - 1] = 0;
	}

	if (cause == MCU_INTERNAL_PROBLEM)
		*pParam >> MipErrorNumber;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	if (pPartyConnection)
	{
		PTRACE2(eLevelInfoNormal, "CConf::OnPartyDisConnectConnect : Party Name - ", pPartyConnection->GetName());
		pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());

		if (!pConfParty)
			PDELETEA(alternativeAddrStr);

		PASSERT_AND_RETURN(!pConfParty);
		UpdateDB(pParty, PARTYSTATE, PARTY_DISCONNECTING);
		UpdateDB(pParty, DISCAUSE, cause, NULL, MipErrorNumber);
		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();

		if (pConfParty->GetUndefinedType() == UNRESERVED_PARTY)  //shiraITP - 160
		{
			mode = 0;   //delete party
		}
		else
		{
			mode = 1; //disconnect party
		}
		//Send content to Lync when plugin leave
		if (pConfParty->GetIsLyncPlugin())
		{
			PTRACE(eLevelInfoNormal, "CConf::OnPartyDisConnectConnect - Send content to Lync when plugin leaves");
			LegacyOnDemandCheck(pPartyConnection, FALSE);
		}
		if (pPartyCntl->GetInterfaceType() == H323_INTERFACE_TYPE)
			pPartyConnection->DisconnectH323(mode); //UNRESERVED PARTY NOT SUPPORTED YET IN 323
		else if (pPartyCntl->GetInterfaceType() == SIP_INTERFACE_TYPE)
		{
			if (alternativeAddrStr)
			{
				pConfParty->SetSipPartyAddress(alternativeAddrStr);
				pConfParty->SetSipPartyAddressType(PARTY_SIP_SIPURI_ID_TYPE);
			}
			else
			{
				CSipNetSetup* pNetSetup = (CSipNetSetup*)((CSipPartyCntl*)pPartyCntl)->GetSipNetSetup();
				if (pNetSetup)
				{
					const char* strRemoteAddr = pNetSetup->GetRemoteSipAddress();
					pConfParty->SetSipPartyAddress(strRemoteAddr);
					pConfParty->SetSipPartyAddressType(PARTY_SIP_SIPURI_ID_TYPE);
				}
			}
			pPartyConnection->DisconnectSip(mode, cause, alternativeAddrStr);
		}
		else
		{
			if (pPartyConnection->GetVoice())
				pPartyConnection->DisconnectPstn(mode);
			else
				pPartyConnection->DisconnectIsdn(mode);
		}

		//FSN-613: Dynamic Content for SVC/Mix Conf
		// check new content protocol when parties disconnect
		if (IsEnableH239())
		{
			UpdateContentProtocolOnDisconnectParty();
		}

	}
	else
		PTRACE(eLevelError, "CConf::OnPartyDisConnectConnect : \'PARTY NOT VALID !!! \'");

	PDELETEA(alternativeAddrStr);
}

//--------------------------------------------------------------------------
void CConf::OnPartyEndExport(CSegment* pParam)
{
	PartyRsrcID partyId;
	WORD  status;

	*pParam >> partyId >> status;

	TRACEINTO << "ConfName:" << m_name << ", PartyId:" << partyId << ", Status:" << status;

	EndExportParty(partyId, status);

	SetMediaStateOnPartyDisconnectConnct(PARTY_DISCONNECTED);

}

//--------------------------------------------------------------------------
void CConf::OnPartyEndImport(CSegment* pParam)
{
	PartyRsrcID partyId;
	WORD  status;
	*pParam >> partyId >> status;

	TRACEINTO << "ConfName:" << m_name << ", PartyId:" << partyId << ", Status:" << status;

	EndImportParty(partyId, status);

	SetMediaStateOnPartyDisconnectConnct(PARTY_CONNECTED);
}

//--------------------------------------------------------------------------
DWORD CConf::GetNumberOfParticpants(std::vector<DWORD>& partyStates)
{
	DWORD numOfConnectedParticpants = 0;

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (pPartyConnection)
		{
			CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
			if (!pConfParty)
			{
				PASSERTSTREAM(1, "MonitorPartyId:" << pPartyConnection->GetMonitorPartyId() << " - Failed, party does not exist");
				continue;
			}
			DWORD partyState = pConfParty->GetPartyState();
			std::vector<DWORD>::iterator _state = std::find(partyStates.begin(), partyStates.end(), partyState);
			if (_state != partyStates.end())
				++numOfConnectedParticpants;
		}
	}
	return numOfConnectedParticpants;
}

//--------------------------------------------------------------------------
void CConf::SetMediaStateOnPartyDisconnectConnct(int partyState)
{
	switch (partyState)
	{
		case PARTY_CONNECTED:
		{
			if (eMediaStateEmpty == GetMediaState())
			// bridge-6789 - if parties connected in STAND_BY - SetMediaStateOnPartyDisconnectConnct no called, but parties added to m_pPartyList->entries()
			// condition changed from m_pPartyList->entries() == 1 to eMediaStateEmpty == GetMediaState()
			{
				PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin();
				PASSERT_AND_RETURN(_itr == m_pPartyList->m_PartyList.end());

				CPartyConnection* pPartyConnection = _itr->second;
				PASSERT_AND_RETURN(!pPartyConnection);

				CConfParty* pConfParty = GetCommConf()->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
				PASSERT_AND_RETURN(!pConfParty);

				if (pConfParty->GetPartyMediaType() == eAvcPartyType)
				{
					UpdateConfMediaState(eMediaStateAvcOnly);
				}
				else if (pConfParty->GetPartyMediaType() == eSvcPartyType)
				{
					UpdateConfMediaState(eMediaStateSvcOnly);
				}
				std::vector<DWORD> partyStates;
				partyStates.push_back(PARTY_CONNECTED);
				DWORD NumOfConnectedParties = GetNumberOfParticpants(partyStates);

				TRACEINTO << "State:PARTY_CONNECTED, NumOfParties:" << m_pPartyList->entries() << ", NumOfConnectedParties:" << NumOfConnectedParties << ", MediaState:" << MediaStateToString(GetMediaState());
			}
			break;
		}

		case PARTY_DISCONNECTED:
		{
			std::vector<DWORD> partyStates;
			partyStates.push_back(PARTY_CONNECTED);
			DWORD NumOfConnectedParties = GetNumberOfParticpants(partyStates);
			if (NumOfConnectedParties == 0)
				UpdateConfMediaState(eMediaStateEmpty);

			TRACEINTO << "State:PARTY_DISCONNECTED, NumOfParties:" << m_pPartyList->entries() << ", NumOfConnectedParties:" << NumOfConnectedParties << ", MediaState:" << MediaStateToString(GetMediaState());
			break;
		}
		default:
			TRACEINTO << " not expected ??";
			return;
	}
}

//--------------------------------------------------------------------------
//BOOL isAddAudioOnly :   TRUE for add an audio only party, FALSE for remove an audio only party
void CConf::UpdateAudioParticipantsCount(CConfParty *pConfParty,BOOL isAddAudioOnly)
{
	//1. check audio participants indication is enabled or not.
	if(!(m_pCommConf->GetEnableAudioParticipantsIcon()))
		return;

	//2. audio only participants connected on AVMCU is not counted
	if(pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone)
		return;

	//3. Recording link party is not counted
	if(pConfParty->GetRecordingLinkParty())
		return;

	// Bridge-12209: Lync CCS plugin, TIP Slave and Cascade links are not counted
	if (pConfParty->GetRemoteIdent() == MicrosoftEP_Lync_CCS)
	{
		TRACEINTO << "Lync_CCS: " << pConfParty->GetName();

		return;
	}

	if (pConfParty->IsTIPSlaveParty())
	{
		TRACEINTO << "TIPSlave: " << pConfParty->GetName();

		return;
	}


	//4. Update the audio participant count
	if(isAddAudioOnly)    //In case add a audio only party
	{
		TRACEINTO << " , increase the audio only participants count";
		if(!(pConfParty->IsCountedInAudioIndication()))
		{
			pConfParty->SetCountedInAudioIndication(YES);
			m_AudioParticipantsNumber++;
			if(m_AudioParticipantsNumber >= 101)  //change from 100 to 100+, no need to update the indication icon
				return;
		}
		else
			return;      //this audio only party is already counted
	}
	else //In case delete audio only party
	{
		TRACEINTO << " , decrease the audio only participants count";
		if(pConfParty->IsCountedInAudioIndication())
		{
			PASSERT_AND_RETURN(0 == m_AudioParticipantsNumber);
			pConfParty->SetCountedInAudioIndication(NO);  //change from
			m_AudioParticipantsNumber--;
			if(m_AudioParticipantsNumber >= 100)  //change from 100+ to 100, no need to update the indication icon
				return;
		}
		else
			return;   //this audio only party is already not counted.
	}

	//5. Send INDICATION_ICONS_CHANGE to video bridge
	SendIndicationIconChange();

}


////////////////////////////////////////////////////////////////////////////
void CConf::SendIndicationIconChange()
{

	//1. check whether the icon indication is updated within 1 sec or not.
	if(IsValidTimer(CONF_AUDIO_NUMBER_INDICATION_GAP_TOUT))
	{
		TRACEINTO << "timer CONF_AUDIO_NUMBER_INDICATION_GAP_TOUT is still running, wait it time out to update the indication";
		m_IsAudioParticipantsNumberNotSent = YES;
		return;
	}


	TRACEINTO << " , current audio participants number: " << m_AudioParticipantsNumber;

	//2. When audio number change to 0
	//    1) if display mode is "display on changes", then display icon with number 0 for the given duration
	//    2) otherwise, hidded the icon
	WORD isActiveIcon = NO;
	BYTE iconDisplayMode = m_pCommConf->GetAudioParticipantsIconDisplayMode();
	if(0 == m_AudioParticipantsNumber)
	{
		if(eIconDisplayOnChange == iconDisplayMode)
			isActiveIcon = YES;
	}
	else
	{
		isActiveIcon = YES;
	}

	//3. Send INDICATION_ICONS_CHANGE to video bridge
	CSegment* seg = new CSegment;
	*seg << (OPCODE)INDICATION_ICONS_CHANGE << (WORD)eIconAudioPartiesCount
			<< m_AudioParticipantsNumber << isActiveIcon;

	if (m_pVideoBridgeInterface)
		m_pVideoBridgeInterface->HandleEvent(seg);
	POBJDELETE(seg);


	//4. start timer and avoid to update the icon before timer timeout
	StartTimer(CONF_AUDIO_NUMBER_INDICATION_GAP_TOUT,SECOND);

	//5. If display mode is "display on change", then start a timer to hidden the icon after the given duration
	if(eIconDisplayOnChange == iconDisplayMode)
	{
		if(IsValidTimer(CONF_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT))
			DeleteTimer(CONF_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT);

		WORD timeInterval = m_pCommConf->GetAudioParticipantsIconDuration();
		TRACEINTO << ",start timer CONF_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT, duration is: "<< timeInterval;

		StartTimer(CONF_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT,timeInterval * SECOND);
	}

}


////////////////////////////////////////////////////////////////////////////
void CConf::OnPartyImportParty(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_name;
	DeleteTimer(AUTOTERMINATE);

	// get parameters and validity tests
	CPartyCntl* pImpConfPartyCntl = NULL;
	CConfParty* pImpConfPartyDesc = NULL;
	CCommConf* pSourceComConf = NULL;
	char* pDestName;
	CConfParty* pConfParty = NULL; //added by Jason Zhu for VNGR-23097

	*pParam >> (void*&)pImpConfPartyCntl >> (void*&)pImpConfPartyDesc >> (void*&)pDestName;

	if (!CPObject::IsValidPObjectPtr(pImpConfPartyCntl))
	{
		TRACESTRFUNC(eLevelError) << "ConfName:" << m_name << " - Failed, invalid Party Control";
		ResponedClientRequest(STATUS_MOVE_PARTY_NOT_COMPLETED);
		return;
	}
	if (!CPObject::IsValidPObjectPtr(pImpConfPartyDesc))
	{
		TRACESTRFUNC(eLevelError) << "ConfName:" << m_name << " - Failed, invalid destination conference";
		ResponedClientRequest(STATUS_MOVE_PARTY_NOT_COMPLETED);
		return;
	}

	//modified by Jason Zhu for VNGR-23097 begin
	DWORD partyIDInDestConf;
	bool isPartyNameSame = true;
	mcTransportAddress partyAddress = pImpConfPartyDesc->GetIpAddress();
	bool isPartyDefinedInTargetConf = m_pCommConf->IsPartyDefined(&partyAddress, pImpConfPartyDesc->GetH323PartyAlias(), partyIDInDestConf, pImpConfPartyDesc->GetNetInterfaceType());
	if (isPartyDefinedInTargetConf)
	{
		pConfParty = m_pCommConf->GetCurrentParty(partyIDInDestConf);
		PTRACE2INT(eLevelInfoNormal, "CConf::OnPartyImportParty party already defined in conf, party id: ", partyIDInDestConf);
		if (strncmp(pConfParty->GetName(), pImpConfPartyDesc->GetName(), H243_NAME_LEN))
		{
			PTRACE2(eLevelInfoNormal, "Conf::OnPartyImportParty Party name in Dest conf is different from in Source Conf ", pConfParty->GetName());
			isPartyNameSame = false;
			pConfParty = NULL;
		}

	}

	if (!isPartyDefinedInTargetConf || !isPartyNameSame)
	{
		pConfParty = new CConfParty(*pImpConfPartyDesc);
	}

	if (!CPObject::IsValidPObjectPtr(pConfParty))
	{
		PASSERT(1);
		PTRACE2(eLevelError, "CConf::OnPartyImportParty Party-Move-Error, CConfParty allocation failure ", m_name);
		ResponedClientRequest(STATUS_MOVE_PARTY_NOT_COMPLETED);
		return;
	}
	//modified by Jason Zhu for VNGR-23097 end

	//get flags
	BYTE networkInterface = pImpConfPartyDesc->GetNetInterfaceType();
	BYTE voice_party = pConfParty->GetVoice();

	WORD numberOfB0Channels = 0;
	DWORD partyId = pConfParty->GetPartyId();

	if (networkInterface == SIP_INTERFACE_TYPE)
	{
		DWORD dwPartRcrsID = pImpConfPartyCntl->GetPartyRsrcId();
		CConf* pConfPrev = pImpConfPartyCntl->GetConf();
		TRACEINTO << "PartyId:" << dwPartRcrsID;
		if (IsValidPObjectPtr(pConfPrev))
		{
			CSegment segSubscriber;
			if (pConfPrev->DisconnectSubscriberByRsrcID(dwPartRcrsID, segSubscriber))
				ChainSubscriber(segSubscriber, partyId);
		}
	}

	PartyMonitorID oldMonitorPartyId = pImpConfPartyCntl->GetMonitorPartyId();
	pImpConfPartyCntl->SetMonitorPartyId(partyId);
	// Obtain DB object of Source Conf
	pSourceComConf = ::GetpConfDB()->GetCurrentConf(pImpConfPartyCntl->GetMonitorConfId());

	// Block for calculating number of B channels
	//CCapH221*      pCap 		= new CCapH221;
	CUnifiedComMode* pPartyScm = new CUnifiedComMode(*m_pUnifiedComMode);
	DWORD vid_bitrate;
	if (networkInterface == H323_INTERFACE_TYPE || networkInterface == SIP_INTERFACE_TYPE)
	{
		CMedString trace_str;
		WORD videoSession = GetVideoSession();

		// seting party local capabilities and communication mode from the target conf m_pScm
		if (voice_party)
			pPartyScm->SetVideoOff(42);

		BYTE partyVideoProtocol;

		//calculating video bitrate.
		//we will compare it to the source conf video bitrate
		//if the target conf is vsw and we changed the rate we will go to secondery
		//because we can't change the channel
		DWORD confRate = 0;
		int val;
	}

	WORD sourceId = 0;
	BYTE dest_conf_video_session = m_pCommConf->GetVideoSession();
	BYTE is_video_resources_required = NO;
	if (voice_party == NO && (dest_conf_video_session == CONTINUOUS_PRESENCE || dest_conf_video_session == VIDEO_TRANSCODING || dest_conf_video_session == ADVANCED_LAYOUTS))
		is_video_resources_required = YES;

	WORD welcomMode;
	char AvServiceName[AV_SERVICE_NAME];
	WORD welcomeMsgTime = 0;
	AvServiceName[0] = '\0';

	CAVmsgServiceList* pAVmsgServiceList = ::GetpAVmsgServList();
	CAvMsgStruct* pAvMsgStruct = m_pCommConf->GetpAvMsgStruct();

	//support for Welcome Message
	PASSERT(!pAvMsgStruct);
	PASSERT(!pAVmsgServiceList);
	// check if party is the Operator
	// Romem - not necessary for GL1
	BYTE isRecording = pConfParty->GetRecordingPort();
	BYTE bCascadeMode = pConfParty->GetCascadeMode();

	if (pAvMsgStruct && pAVmsgServiceList && !isRecording && (CASCADE_MODE_NONE == bCascadeMode))
	{
		welcomMode = pAvMsgStruct->GetAttendedWelcome(); //welcome wait or no-wait
		AvServiceName[0] = '\0';
		strncpy(AvServiceName, pAvMsgStruct->GetAvMsgServiceName(), sizeof(AvServiceName) - 1);
		AvServiceName[sizeof(AvServiceName) - 1] = '\0';
	}
	else
		welcomMode = STATUS_PARTY_INCONF;

	pConfParty->SetVideo_Member(FALSE);
	pConfParty->SetAudio_Member(FALSE);
	pConfParty->SetContent_Member(FALSE);
	pConfParty->SetIsPrivateLayout(NO);

	// Party should be partially connected at this stage
	pConfParty->SetPartyState(PARTY_CONNECTED_PARTIALY, m_pCommConf->GetMonitorConfId());
	pConfParty->SetAddPartyMask(YES); //Added in order to perform correct update of image in operator
	pConfParty->SetInfoOpcode(PARTY_COMPLETE_INFO);

	if (ISDEBUGMODE("MOVE", 8))
	{
		TRACEINTO << " Party-Move-Error Debug Mode";
		ResponedClientRequest(STATUS_MOVE_PARTY_NOT_COMPLETED);
		pImpConfPartyCntl->SetMonitorPartyId(oldMonitorPartyId);
		POBJDELETE(pPartyScm);
		return;
	}

	//modified by Jason Zhu for VNGR-23097 begin
	if (!isPartyDefinedInTargetConf || !isPartyNameSame)
	{
		WORD addStatus = m_pCommConf->Add(*pConfParty);
		if (addStatus != STATUS_OK)
		{
			PASSERT(addStatus);
			ALLOCBUFFER(buf, H243_NAME_LEN*4);
			sprintf(buf, "CConf::OnPartyImportParty: Failed to add party to DB, Conf Name: %s, Party Name - %s, Status: %d", m_name, pConfParty->GetName(), addStatus);
			PTRACE2(eLevelInfoNormal, " Party-Move-Error ---> ", buf);
			DEALLOCBUFFER(buf);
			POBJDELETE(pConfParty);
			POBJDELETE(pPartyScm);
			ResponedClientRequest(STATUS_MOVE_PARTY_NOT_COMPLETED);
			pImpConfPartyCntl->SetMonitorPartyId(oldMonitorPartyId);
			return;
		}
	}
	//modified by Jason Zhu for VNGR-23097 end
	if (pConfParty->GetTelePresenceMode() != eTelePresencePartyNone)
		DetermineTelePresenceConfMode(1);

	m_IsAnyPartyWasConnected = TRUE;

	// Romem - TBD
	// in EPC mcuNumber / terminalNumber always 0
	UdpAddresses tmpUdpAdd;
	if (pImpConfPartyDesc->GetNetInterfaceType() == SIP_INTERFACE_TYPE)
	{
		tmpUdpAdd = ((CSipPartyCntl*)pImpConfPartyCntl)->GetUdpAddress();
	}

	CMoveIPImportParams* pMoveIPImportParams = new CMoveIPImportParams(this, pImpConfPartyCntl, m_name, m_monitorConfId, m_pAudBrdgInterface, m_pVideoBridgeInterface, m_pConfAppMngrInterface, m_pFECCBridge, m_pContentBridge, m_pTerminalNumberingManager, m_pUnifiedComMode->GetCopVideoTxModes());

	CPartyConnection* pPartyConnection = new CPartyConnection;

	if (networkInterface != ISDN_INTERFACE_TYPE && pImpConfPartyDesc->IsIpNetInterfaceType() == NO)
		PASSERT(1);
	else if (pImpConfPartyDesc->GetNetInterfaceType() == H323_INTERFACE_TYPE)
		pPartyConnection->ImportIp(pMoveIPImportParams);
	else if (pImpConfPartyDesc->GetNetInterfaceType() == SIP_INTERFACE_TYPE)
	{
		//BRIDGE-12419: CSS Adhoc enhancement
		pPartyConnection->ImportIp(pMoveIPImportParams, pImpConfPartyDesc->GetIsLyncPlugin());
		((CSipPartyCntl*)(pPartyConnection->GetPartyCntl()))->SetUdpAddress(tmpUdpAdd);
	}
	else if (networkInterface == ISDN_INTERFACE_TYPE)
	{
		if (pConfParty->GetVoice())
			pPartyConnection->ImportPSTN(pMoveIPImportParams);
		else
			pPartyConnection->ImportISDN(pMoveIPImportParams);
	}

	InsertPartyConnection(pPartyConnection);

	pImpConfPartyCntl->SetMonitorPartyId(oldMonitorPartyId);

	POBJDELETE(pMoveIPImportParams);

	strcpy_safe(pDestName, H243_NAME_LEN, m_name);

	ResponedClientRequest(STATUS_OK);
	if (!isPartyDefinedInTargetConf || !isPartyNameSame)     	//modified by Jason Zhu for VNGR-23097
	{
		POBJDELETE(pConfParty);
	}
	POBJDELETE(pPartyScm);
}

/////////////////////////////////////////////////////////////////////////////
//Description: For H323 parties: This function is called in 2 cases:
//1) Party is downgrade to secondary
//2) Remote sent re-capabilities.
//
void  CConf::OnPartyChangeVidMode(CSegment* pParam)
{
	CTaskApp*  pParty;

	*pParam >> (DWORD&)pParty;
	WORD is_set_to_secondary = FALSE;
	*pParam >> is_set_to_secondary;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);

	if (pPartyConnection)
	{
		PTRACE2(eLevelInfoNormal,"CConf::OnPartyChangeVidMode : Party Name - ",pPartyConnection->GetFullName());
		if (pPartyConnection->IsDisconnect() == FALSE)
		{
			SetCapCommonDenominator(pParty, FALSE);
		}
	}
	else
		PTRACE2(eLevelError,"CConf::OnPartyChangeVidMode : \'PARTY NOT EXISTS\' ",m_name);
}

/////////////////////////////////////////////////////////////////////////////
//Description: When party receive re-caps it inform the conf level that decided what to do with it
// currently it only initiate SetCapCommonDenominator (but in the future it can also initiate Content change mode, etc.
void  CConf::OnPartyReceiveReCaps(CSegment* pParam)
{
	CTaskApp*  pParty;

	*pParam >> (DWORD&)pParty;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);

	if (pPartyConnection)
	{
		PTRACE2(eLevelInfoNormal,"CConf::OnPartyReceiveReCaps : Party Name - ",pPartyConnection->GetFullName());
		if (pPartyConnection->IsDisconnect() == FALSE)
		{
			SetCapCommonDenominator(pParty, NO);
		}
	}
	else
		PTRACE2(eLevelError,"CConf::OnPartyReceiveReCaps : \'PARTY NOT EXISTS\' ",m_name);
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnAudBrdgConnectSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnAudBrdgConnectSetup : Name - ",m_name);
	WORD  status;
	*pParam >> status;
  if (status)
  {
		// failed to connect audio bridge. action - drop conference,reallocate audio bridge
		ON(m_isBridgeError);

		DWORD updatedStatus = 0;
		updatedStatus = (CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
		UpdateConfStatus(updatedStatus,YES);
		//m_pCommConf->SetStatus(CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
		PTRACE(eLevelError,"CConf::OnAudBrdgConnectSetup : failed to connect audio bridge");
	}

	ON(m_isAudConnected);
	BridgeConnectionCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnAudBrdgDisConnectTerminate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnAudBrdgDisConnectTerminate : Name - ",m_name);
	WORD  status;
	*pParam >> status;
	PASSERT(status);
	OFF(m_isAudConnected);

	char tmp[H243_NAME_LEN+100];
	snprintf(tmp, sizeof(tmp), "[%s] - Conf Audio Brdg disconnected.", m_name);
	PTRACE2(eLevelInfoNormal," ---> ",tmp);

	BridgeDisConnectionCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnVidBrdgConnectSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnVidBrdgConnectSetup : Name - ",m_name);
	WORD  status;
	*pParam >> status;
	if (status)
	{
		// failed to connect video bridge in case of C.P.
		// action - reallocate video bridge.
		//		- if video bridge reallocation fails the video control will be switch only
		ON(m_isBridgeError);

		DWORD updatedStatus = 0;
		updatedStatus = (CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
		UpdateConfStatus(updatedStatus,YES);
		//m_pCommConf->SetStatus(CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
		PTRACE(eLevelError,"CConf::OnVidBrdgConnectSetup : failed to connect video bridge");
	}
	else
	{
		if(m_pCommConf->GetIsAsSipContent() &&  !m_pCommConf->GetEntryQ())
		{
			PTRACE2(eLevelInfoNormal,"CConf::OnVidBrdgConnectSetup : Create XCode bridge in Case of AS-Sip conference, Conf Name:",m_name);
			CreateVideoBridge(eVideoXcode_Bridge_V1);
		}
	}
	BOOL isTelepresenceActivated = (m_pCommConf->GetTelePresenceModeConfiguration() != NO) && m_pCommConf->GetIsTelePresenceMode();

	if (m_pVideoBridgeInterface)
		m_pVideoBridgeInterface->TurnOnOffTelePresence(isTelepresenceActivated, NULL);
	ON(m_isVidConnected);
	BridgeConnectionCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnVidBrdgDisConnectTerminate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnVidBrdgDisConnectTerminate : Name - ",m_name);
	WORD  status;
	*pParam >> status;
	PASSERT(status);
	// In case we are in COP conference and the disconnection status is not OK we will need to
	// Update the resource allocator in the deallocate stage that the status is not OK so it will
	// initiate Kill Port flows for all the conference resources
	if(status && GetVideoSession() == VIDEO_SESSION_COP)
	{
		PTRACE2(eLevelInfoNormal,"CConf::OnVidBrdgDisConnectTerminate disconnect with resource problem in COP conference: Name - ",m_name);
		m_rsrcDeallocateStatus = STATUS_FAIL;
	}

	OFF(m_isVidConnected);

	char tmp[H243_NAME_LEN+100];
	snprintf(tmp, sizeof(tmp), "[%s] - Conf Video Brdg disconnected.", m_name);
	PTRACE2(eLevelInfoNormal," ---> ",tmp);

	BridgeDisConnectionCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnFeccBrdgConnectSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnFeccBrdgConnectSetup : Name - ",m_name);
	WORD  status;
	*pParam >> status;
  if (status)
  {
		// failed to connect fecc bridge
		ON(m_isBridgeError);

		DWORD updatedStatus = 0;
		updatedStatus = (CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
		UpdateConfStatus(updatedStatus,YES);
		PTRACE(eLevelError,"CConf::OnFeccBrdgConnectSetup : failed to connect fecc bridge");
	}

	ON(m_isFeccConnected);
	BridgeConnectionCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnFeccBrdgDisConnectTerminate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnFeccBrdgDisConnectTerminate : Name - ",m_name);
	WORD  status;
	*pParam >> status;
	PASSERT(status);
	OFF(m_isFeccConnected);

	char tmp[H243_NAME_LEN+100];
	snprintf(tmp, sizeof(tmp), "[%s] - Conf Fecc Brdg disconnected.", m_name);
	PTRACE2(eLevelInfoNormal," ---> ",tmp);

	BridgeDisConnectionCompleted();
}
/////////////////////////////////////////////////////////////////////////////
void  CConf::OnTimerConnect(CSegment* pParam)
{
	TRACEINTO << "Name:" << m_name << ", state:" << m_state;

	if (m_state != TERMINATION)
	{
		// VNGFE-6358 - For EQ:
		// - Check that there are no participants in EQ before terminating it.
		// - If not empty, extend the EQ duration (no need for resource process confirmation since it is an EQ - no resource to reserve).

		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pCommConf));

		WORD dummy = 0;
		if (m_pCommConf->GetEntryQ() && (!AreAllPartiesDisconnected(dummy)))
		{
			DWORD ExtensionTimeInterval  = 0;
			CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("EXTENSION_TIME_INTERVAL", ExtensionTimeInterval);
			if (0 == ExtensionTimeInterval)
				ExtensionTimeInterval = 10;

			m_AccumulatedExtensionTime += ExtensionTimeInterval;

			// Calculate new end time
			CStructTm OldEndTime,ExtentionTime(0,0,0,0,ExtensionTimeInterval);
			CStructTm* NewEndTime = new CStructTm;

			OldEndTime = *(m_pCommConf->GetEndTime()); // Local time

			*NewEndTime = m_pCommConf->CalculateConfEndTime(OldEndTime,ExtentionTime);

			m_pCommConf->OperatorSetEndTime(*NewEndTime,"AUTO_EXTENSION");

			TRACEINTO << "ConfName:" << m_pCommConf->GetName() << " - End Conference Time was updated due to participants existence in EQ";

			CSegment*  seg = new CSegment;
			COstrStream msg;
			NewEndTime->Serialize(msg);
			*seg << msg.str().c_str();

			BOOL bSetAlertTout = FALSE;	// in order to avoid double auto extension
			SetEndTime(seg, bSetAlertTout);
			return;
		}

		Destroy(CONF_END_TIME);
		SetConfTerminateCause(TERMINATED_END_TIME_PASSED);
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CConf::OnTimerConnectAuto(CSegment* pParam)
{

	TRACEINTO << "ConfName:" << m_name << ", ConfState:" << m_state;
	if (m_state != TERMINATION)
	{
		//D.K.VNGR-21532/VNGR-21570:
		//Do not to terminate the conference despite the fact that timer is triggered because the conference is not empty.
		WORD numOfNonDisconnectedParties = 0;
		BOOL areAllDisconnected = AreAllPartiesDisconnected(numOfNonDisconnectedParties);
		// VNGR-22462 - Return only when there are parties connected and it is not GW, and not in case of numOfNonDisconnectedParties=1 and eTerminateWithLastRemains.
		ELastQuitType lastQuitType = (ELastQuitType)m_pCommConf->GetLastQuitType();
		if (!areAllDisconnected && !m_isGateWay && !(lastQuitType==eTerminateWithLastRemains && numOfNonDisconnectedParties<2))
			return;

		Destroy();
		SetConfTerminateCause(AUTO_TERMINATION);
	}
}
/////////////////////////////////////////////////////////////////////////////
void CConf::OnTimerIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnTimerIdle : Name - ",m_name);
	OnTimerConnect(pParam);
}
/////////////////////////////////////////////////////////////////////////////
void  CConf::OnTimerRemindConnect( CSegment* pSeg )
{
	PTRACE2(eLevelInfoNormal,"CConf::OnTimerRemindConnect : Name - ",m_name);
	//Extending conf time
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL IsEnableAutoExtension = FALSE;
	std::string key = "ENABLE_AUTO_EXTENSION";
	sysConfig->GetBOOLDataByKey(key, IsEnableAutoExtension);

	DWORD MaxExtentionTime = 0;
	key = "MAX_EXTENSION_TIME";
	sysConfig->GetDWORDDataByKey(key, MaxExtentionTime);

	DWORD ExtensionTimeInterval  = 0;
	key = "EXTENSION_TIME_INTERVAL";
	sysConfig->GetDWORDDataByKey(key, ExtensionTimeInterval);

	m_AccumulatedExtensionTime += ExtensionTimeInterval;

	if(IsEnableAutoExtension && (m_AccumulatedExtensionTime <= MaxExtentionTime))
	{
		WORD dummy;
		if(!AreAllPartiesDisconnected(dummy))
		{
			CStructTm OldEndTime,ExtentionTime(0,0,0,0,ExtensionTimeInterval);
			CStructTm* NewEndTime = new CStructTm;

			OldEndTime = *(m_pCommConf->GetEndTime()); // Local time

			*NewEndTime = m_pCommConf->CalculateConfEndTime(OldEndTime,ExtentionTime);

			// ===== 1. send request to Resource process // Romem - 15.7.08
			CSegment *pSeg = new CSegment;
			*pSeg << m_pCommConf->GetMonitorConfId();
			NewEndTime->Serialize(NATIVE, *pSeg);

			CTaskApi resourceManagerApi(eProcessResource, eManager);
			resourceManagerApi.SendMsg(pSeg,SET_AUTO_EXTEND_CONF_ENDTIME_REQ);
			POBJDELETE(NewEndTime);
		}
		return;
	}

	//Send Alert to IVR only if it is not an EQ
	if (m_pConfAppMngrInterface && !m_pCommConf->GetEntryQ())
	{
		std::vector<DWORD> partyStates;
		partyStates.push_back(PARTY_CONNECTED);
		DWORD NumOfConnectedParties = GetNumberOfParticpants(partyStates);
		if (NumOfConnectedParties)
		{
			TRACESTR (eLevelInfoNormal) << "CConf::OnTimerRemindConnect Sending eCAM_EVENT_CONF_ALERT_TONE on conf: " << m_name ;
			CSegment *seg = new CSegment;
			*seg << (DWORD)EVENT_ACTION
					 << (DWORD)EVENT_CONF_REQUEST
					 << (DWORD)eCAM_EVENT_CONF_ALERT_TONE;
			m_pConfAppMngrInterface->HandleEvent(seg);
			POBJDELETE(seg);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnTimerAudioNumberChanged(CSegment* pSeg)
{
	TRACEINTO << ", 1 sec after the last audio participant indication update, a new update is needed?: " <<
				m_IsAudioParticipantsNumberNotSent? "YES" : "NO";
	if(m_IsAudioParticipantsNumberNotSent)
	{
		m_IsAudioParticipantsNumberNotSent = NO;
		SendIndicationIconChange();
	}

}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnTimerAudioNumberHidden(CSegment* pSeg)
{
	TRACEINTO << ", timer CONF_AUDIO_NUMBER_INDICATION_DISPLAY_TOUT timeout, hidden the audio indication icon";
	WORD isIconActive = NO;
	CSegment* seg = new CSegment;

	*seg << (OPCODE)INDICATION_ICONS_CHANGE << (WORD)eIconAudioPartiesCount
		 << GetAudioParticipantsNum() << isIconActive;    //hidden the icon

	if (m_pVideoBridgeInterface)
		m_pVideoBridgeInterface->HandleEvent(seg);

	POBJDELETE(seg);

}


/////////////////////////////////////////////////////////////////////////////
void CConf::OnConfRemovePartyConnection(CSegment* pParam)
{
	CTaskApp* pParty;
	BYTE isCancel;

	*pParam >> (DWORD&)pParty;
	*pParam >> isCancel;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	PASSERT_AND_RETURN(!pPartyConnection);

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	PASSERT_AND_RETURN(!pConfParty);

	BOOL bDelayedRemoveIsdnLink = FALSE;

	TRACEINTO << "ConfName:" << m_name << ", PartyName:" << pPartyConnection->GetName() << ", MonitorPartyId:" << pPartyConnection->GetMonitorPartyId();

	PartyMonitorID party_id = pConfParty->GetPartyId();
	BYTE connectionType = pConfParty->GetConnectionType();
	eGatewayPartyType gatewayPartyType = pConfParty->GetGatewayPartyType();
	eGeneralDisconnectionCause discCauseForGW = eGeneralDisconnectionCauseLast;
	CMedString discCauseString;
	// create the message to be displayed on screen reporting on the party disconnection
	if (m_isGateWay || m_isDtmfInviteParty)
	{
		PTRACE(eLevelInfoNormal, "CConf::OnConfRemovePartyConnection m_isGateWay || m_isDtmfInviteParty");
		GenerateGeneralDisconnectionCause(pConfParty, discCauseForGW);
	}

	BYTE inviteNextParty = FALSE;
	if (strstr(pConfParty->GetName(), "_InvitedByPcm") != NULL && m_pInvitedDialingSequence->IsMemberInSequence(party_id))
		inviteNextParty = TRUE;

	if (strstr(pConfParty->GetAdditionalInfo(), "InvitedByPcm") != NULL)
	{
		pConfParty->SetAdditionalInfo("");
		RespondPCMManagerInviteFailed((CSegment*)NULL);
	}

	if (isCancel)
	{
		COsQueue * pConfRcvMailBox = m_pCommConf->GetRcvMbx();

		m_pCommConf->SetRcvMbx(m_pRcvMbx);
		DWORD cancel_status = m_pCommConf->Cancel(pPartyConnection->GetName());
		m_pCommConf->SetRcvMbx(pConfRcvMailBox);

		PASSERT(cancel_status);
		UpdateConfStatus();
	}

	if (!GetIsGateWay() && pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE)
	{
		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (pPartyCntl)
		{
			if (CASCADE_NONE != pPartyCntl->GetPartyCascadeType())
			{
				CSegment* savedSeg = new CSegment;
				*savedSeg << (DWORD)pParty;

				OnTimerDtmfFwdAll(NULL);
				StartTimer(DELAY_ISDN_LINK_DISCONNECT_TOUT, SECOND,savedSeg); //VNGR-16314 to allow the *87 to be activate on the other side before the link is disconnected
				bDelayedRemoveIsdnLink = TRUE;
			}
		}
	}

	if (m_isDtmfInviteParty && pPartyConnection->GetInterfaceType() == ISDN_INTERFACE_TYPE)
	{
		PTRACE(eLevelInfoNormal, "CConf::OnConfRemovePartyConnection : ISDN_INTERFACE_TYPE - ");
		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (pPartyCntl)
		{
			if (CASCADE_NONE != pPartyCntl->GetPartyCascadeType())
			{
				CSegment* savedSeg = new CSegment;
				*savedSeg << (DWORD)pParty;

				OnTimerDtmfFwdAll(NULL);
				StartTimer(DELAY_ISDN_LINK_DISCONNECT_TOUT, SECOND,savedSeg); //VNGR-16314 to allow the *87 to be activate on the other side before the link is disconnected
				bDelayedRemoveIsdnLink = TRUE;
			}
		}
	}

	if (pPartyConnection == m_pGwDtmfForwarderConnection)
		m_pGwDtmfForwarderConnection = NULL;

	WORD interfaceType = pPartyConnection->GetInterfaceType();

	if (bDelayedRemoveIsdnLink == FALSE)
	{
		pPartyConnection = RemovePartyConnection(pParty);
		if (pPartyConnection)
		{
			pPartyConnection->Destroy();
			POBJDELETE(pPartyConnection);
		}
	}

	if (m_isGateWay && isCancel && !GwPartiesStateFlag)
	{
		if (gatewayPartyType <= eNormalGWPartyType)
		{
			if (m_pGWPartiesState && (m_pGWPartiesState->find(party_id) != m_pGWPartiesState->end()))
			{
				if ((*m_pGWPartiesState)[party_id] == GW_SETUP)
				{
					// Added for Redial on Wrong Number, save the invite result
					SaveInviteResult(party_id, interfaceType, discCauseForGW);
					GenerateGeneralDisconnectionMessageFromCause(pConfParty, m_PartiesInviteResults[party_id].eOverallInviteResult, discCauseString);
					// a dial out gw party has been disconnected --> try to dial the next protocol
					AddNextPartyForGWSession(party_id, (BYTE)discCauseForGW, discCauseString);
				}
			}
		}
		else
		{
			// the initiator of the call has been disconnected before the setup has been completed
			GwPartiesStateFlag = GW_CONNECT;
			CalcGwPartiesStateFlag();
			m_IsAnyPartyWasConnected = TRUE;
			ActivateAutoTermenation("CConf::OnConfRemovePartyConnection - GW initiator has been disconnected before setup is completed");
		}
	}

	if (m_isDtmfInviteParty && isCancel && !m_invitedPartiesStateFlag)
	{
		PTRACE(eLevelInfoNormal, "CConf::OnConfRemovePartyConnection : m_isDtmfInviteParty  && isCancel && !m_invitedPartiesStateFlag - ");
		if (gatewayPartyType <= eNormalGWPartyType)
		{
			if (m_pInvitedPartiesState && (m_pInvitedPartiesState->find(party_id) != m_pInvitedPartiesState->end()))
			{
				if ((*m_pInvitedPartiesState)[party_id] == GW_SETUP)
				{
					SaveInviteResult(party_id, interfaceType, discCauseForGW);
					GenerateGeneralDisconnectionMessageFromCause(pConfParty, m_PartiesInviteResults[party_id].eOverallInviteResult, discCauseString);
					// a dial out gw party has been disconnected --> try to dial the next protocol
					AddNextPartyForInvitePartySession(party_id, (BYTE)discCauseForGW, discCauseString);
				}
			}
		}
		else
		{
			// the initiator of the call has been disconnected before the setup has been completed
			m_invitedPartiesStateFlag = GW_CONNECT;
			//CalcGwPartiesStateFlag();
			m_IsAnyPartyWasConnected = TRUE;
			ActivateAutoTermenation("CConf::OnConfRemovePartyConnection - GW initiator has been disconnected before setup is completed");
		}
	}

	if (inviteNextParty == TRUE && isCancel)
		AddNextInvitedParty(party_id);

	if (GetIsGateWay())
		SetGwDtmfForwardIfNeeded();
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnConfRemovePartyConnectionTerminate(CSegment* pParam)
{
	OnConfRemovePartyConnection(pParam);

	WORD numParties = m_pPartyList->entries();
	if ( numParties == 0 )  // no active parties in conf
		OnEndDelAllParties();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnSipSubscribeIndCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnSipSubscribeIndCONNECT, Name - ",m_name);
	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIS32 status = 0;
	APIU16 srcUnitId = 0;
	APIU32 csServiceId = 0;

	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId >> csServiceId;
/* IBM - EVENT PACKAGE */
	mcIndSubscribe * pSubscribeMsg = (mcIndSubscribe*)pParam->GetPtr(1);


	// commented by Boris Guelfand. Subscribe from NOT SVC EPs doesn't supported
	//m_pSipEventPackage->Subscribe(pSubscribeMsg, callIndex, srcUnitId, csServiceId);
	if(IsValidPObjectPtr(m_pSvcEventPackage))
	m_pSvcEventPackage->Subscribe(pSubscribeMsg, callIndex, srcUnitId, csServiceId);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnSipReferIndCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnSipReferIndCONNECT, Name - ",m_name);

	APIU32 callIndex = 0;
	APIU32 channelIndex = 0;
	APIU32 mcChannelIndex = 0;
	APIU32 stat1 = 0;
	APIS32 status = 0;
	APIU16 srcUnitId = 0;
	APIU32 csServiceId = 0;

	*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId >> csServiceId;

	mcIndRefer* pReferMsg = (mcIndRefer*)pParam->GetPtr(1);
	m_pSipEventPackage->Refer(pReferMsg, callIndex, srcUnitId, csServiceId);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnSipNotifyResponseIndCONNECT(CSegment* pParam)
{
	if(IsValidPObjectPtr(m_pSvcEventPackage))
	{
		APIU32 callIndex = 0;
		APIU32 channelIndex = 0;
		APIU32 mcChannelIndex = 0;
		APIU32 stat1 = 0;
		APIS32 status = 0;
		APIU16 srcUnitId = 0;
		APIU32 csServiceId = 0;
		*pParam >> callIndex >> channelIndex >> mcChannelIndex >> stat1 >> srcUnitId >> csServiceId;
		mcIndNotifyResp* pIndNotifyResp = (mcIndNotifyResp*)pParam->GetPtr(1);
		TRACEINTO << "ConfId:" << m_ConfRsrcId << ", ConfName:" << m_name << ", status:" << pIndNotifyResp->status << ", id:" << pIndNotifyResp->id;
		m_pSvcEventPackage->OnIndNotifyResp(pIndNotifyResp);
	}
}

bool CConf::DisconnectSubscriberByRsrcID(DWORD dwPartyRsrcId, CSegment& seg)
{
	if(IsValidPObjectPtr(m_pSvcEventPackage))
		return m_pSvcEventPackage->DisconnectSubscriberByRsrcID(dwPartyRsrcId, seg);
	else
		return false;
}
bool CConf::ConnectSubscriberByRsrcID(DWORD dwPartyRsrcId)
{
	if(IsValidPObjectPtr(m_pSvcEventPackage))
		return m_pSvcEventPackage->ConnectSubscriberByRsrcID(dwPartyRsrcId);
	else
		return false;
}
bool CConf::DeleteSubscriberByRsrcID(DWORD dwPartyRsrcId)
{
	if(IsValidPObjectPtr(m_pSvcEventPackage))
		return m_pSvcEventPackage->DeleteSubscriberByRsrcID(dwPartyRsrcId);
	else
		return false;
}
bool CConf::UnchainSubscriberByRsrcID(DWORD dwPartyRsrcId, CSegment& seg)
{
	if(IsValidPObjectPtr(m_pSvcEventPackage))
		return m_pSvcEventPackage->UnchainSubscriberByRsrcID(dwPartyRsrcId, seg);
	else
		return false;
}
bool CConf::UnchainSubscriberByMonitorID(DWORD dwPartyMonitorId, CSegment& seg)
{
	if(IsValidPObjectPtr(m_pSvcEventPackage))
		return m_pSvcEventPackage->UnchainSubscriberByMonitorID(dwPartyMonitorId, seg);
	else
		return false;
}

bool CConf::ChainSubscriber(CSegment& seg, DWORD dwNewPartyMonitorId)
{
	if(IsValidPObjectPtr(m_pSvcEventPackage))
		return m_pSvcEventPackage->ChainSubscriber(seg, dwNewPartyMonitorId);
	else
		return false;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::OnTerminalHandleEventConnect(CSegment* pParam)
{
	const WORD MAX_COMMAND_LEN = 80;

	char partyName[MAX_COMMAND_LEN];
	WORD req = 0;

	*pParam >> req >> partyName;

	TRACEINTO << "ConfName:" << m_name << ", PartyName:" << partyName << ", Request:" << req;

	CPartyConnection* pPartyConnection = GetPartyConnection(partyName);
	TRACECOND_AND_RETURN(!pPartyConnection, "Failed, invalid pointer");

	TRACECOND_AND_RETURN(pPartyConnection->IsDisconnect(), "Failed, party is disconnected");

	CTaskApp* pParty = pPartyConnection->GetPartyTaskApp();
	TRACECOND_AND_RETURN(!pParty, "Failed, invalid pointer");

	CConfApi* pConfApi = new CConfApi;
	pConfApi->CreateOnlyApi(GetRcvMbx());

	CPartyApi* pPartyApi = new CPartyApi;
	pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());

	switch (req)
	{
		case 0:
		{
			pConfApi->DataTokenRequest(pParty, LSD_4800);
			break;
		}

		case 1:
		{
			pConfApi->DataTokenRequest(pParty, LSD_6400);
			break;
		}

		case 2:
		{
			pConfApi->DataTokenRelease(pParty);
			break;
		}

		case 3:
		{
			pPartyApi->OnIpDataTokenMsg(kTokenRequest);
			break;
		}

		case 4:
		{
			pPartyApi->OnIpDataTokenMsg(kTokenRelease);
			break;
		}

		case 5:
		case 6:
		{
			CSegment* segTemp = new CSegment;
			*segTemp << (BYTE)H239_messsage;

			BYTE McuNum      = ((CParty*)pParty)->GetMcuNum();
			BYTE TerminalNum = ((CParty*)pParty)->GetTerminalNum();

			if (req == 5)
			{
				TRACEINTO << "PartyId:" << pParty->GetPartyId() << " - Send Aquire req to Party";
				*segTemp<< (BYTE)PresentationTokenRequest;
				SerializeGenericParameter(TerminalLabelParamIdent, ((McuNum*TERMINAL_LABEL_MULTIPLIER)+TerminalNum), segTemp);
				SerializeGenericParameter(ChannelIdParamIdent, SecondVideoChannel, segTemp);
				SerializeGenericParameter(SymmetryBreakingParamIdent, SYMMETRY_BREAKING_FOR_AQCUIRE_TOKEN, segTemp);
			}
			else
			{
				TRACEINTO << "PartyId:" << pParty->GetPartyId() << " - Send Release req to Party";
				*segTemp<< (BYTE)PresentationTokenRelease;
				SerializeGenericParameter(TerminalLabelParamIdent, ((McuNum*TERMINAL_LABEL_MULTIPLIER)+TerminalNum), segTemp);
				SerializeGenericParameter(ChannelIdParamIdent, SecondVideoChannel, segTemp);
			}

			BYTE msgLength = (BYTE)segTemp->GetWrtOffset();


			CSegment* seg = new CSegment;
			*seg << (BYTE)(Start_Mbe | ESCAPECAPATTR);
			*seg << msgLength;
			*seg << *segTemp;
			pPartyApi->MuxRmtH230(seg);

			POBJDELETE(seg);
			POBJDELETE(segTemp);
			break;
		}

		case 7:
		{
			TRACEINTO << "PartyId:" << pParty->GetPartyId() << " - Send LPR req to Party";
			DWORD newPeopleRate      = 1920;
			DWORD lossProtection     = 3;
			DWORD lmtbf              = 10;
			DWORD lcongestionCeiling = 19200;
			DWORD fill               = 0;
			DWORD modeTimeout        = 0;
			DWORD newContentRate	 = 0;

			pPartyApi->UpdatePeopleLprRate(newPeopleRate, cmCapTransmit, lossProtection, lmtbf, lcongestionCeiling, fill, modeTimeout, lcongestionCeiling, FALSE, newContentRate);
			break;
		}

		case 8:
		{
			TRACEINTO << "PartyId:" << pParty->GetPartyId() << " - Send Intra request from EP to Video Bridge";
			pConfApi->VideoRefresh(pParty->GetPartyId());
			break;
		}

		case 9:
		{
			DWORD timeInterval = 0;
			DWORD numRequests  = 0;
			DWORD channelId    = 1;
			*pParam >> timeInterval >> numRequests >> channelId;
			if (channelId == 1)   // video intra
			{
				TRACEINTO << "PartyId:" << pParty->GetPartyId() << ", timeInterval:" << timeInterval << ", numRequests:" << numRequests << " - Send recurrent Intra request from EP to Video Bridge";
				pConfApi->VideoRefresh(pParty->GetPartyId());
			}
			else    // content intra
			{
				TRACEINTO << "PartyId:" << pParty->GetPartyId() << ", timeInterval:" << timeInterval << ", numRequests:" << numRequests << " - Send recurrent Intra request from EP to Content Bridge";
				pConfApi->ContentVideoRefresh(1, YES, pParty);
			}

			numRequests = numRequests-1;
			if (numRequests > 0)
			{
				CSegment* pSeg = new CSegment;
				*pSeg << partyName << (DWORD)timeInterval << (DWORD)numRequests << (DWORD)channelId;
				StartTimer(TERMINAL_RECURRENT_INTRA_REQ_TOUT, timeInterval*SECOND, pSeg);
			}
			break;
		}

		default:
		{
			TRACEINTO << "PartyId:" << pParty->GetPartyId() << ", Request:" << req << " - Failed, unknown request";
			break;
		}
	} // switch

	pConfApi->DestroyOnlyApi();
	POBJDELETE(pConfApi);

	pPartyApi->DestroyOnlyApi();
	POBJDELETE(pPartyApi);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnIvrConfTerminate( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CConf::OnIvrConfTerminate : Name - ", m_name);

	WORD wIsDtmfForwarding = 0;
	*pParam >> wIsDtmfForwarding;

	if(wIsDtmfForwarding && m_initTimerDtmfForwarding == 0)
	{
		m_initTimerDtmfForwarding = 1;
		StartTimer(DTMF_FORWARDING_TIMER, SECOND);
		return;
	}

	IvrConfTerminate();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnChairDroppedTerminate( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CConf::OnChairDroppedTerminate : Name - ",m_name);
	IvrConfTerminate();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnIvrUpdatePartyIvrStatus(CSegment* pParam)
{
	PartyRsrcID partyRsrcId;
	WORD partyIvrStatus;
	*pParam >> partyRsrcId >> partyIvrStatus;

	#undef INFO_LOG
	#define INFO_LOG "PartyId:" << partyRsrcId << ", PartyIvrStatus:" << partyIvrStatus

	if (partyIvrStatus != STATUS_PARTY_IVR && partyIvrStatus != STATUS_PARTY_INCONF)
	{
		TRACEINTO << INFO_LOG << " - Failed, Illegal state";
		return;
	}

	// find ConfParty and set the state (IVR or InConf)
	CPartyConnection* pPartyConnection = m_pPartyList->find(partyRsrcId);
	if (!pPartyConnection)
	{
		TRACEINTO << INFO_LOG << " - Failed, Party connection does not exist";
		return;
	}

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	if (!pConfParty)
	{
		TRACEINTO << INFO_LOG << " - Failed, ConfParty does not exist";
		return;
	}

	TRACEINTO << INFO_LOG;

	// set the new state
	pConfParty->SetOrdinaryParty(partyIvrStatus);
}


//////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnIvrTimerConfTerminate( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CConf::OnIvrTimerConfTerminate : Name - ",m_name);
	m_initTimerDtmfForwarding = 0;
	IvrConfTerminate();
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConf::IvrConfTerminate()
{
	PTRACE2(eLevelInfoNormal,"CConf::IvrConfTerminate : Name - ",m_name);

  if (m_state == TERMINATION)
  {
		PTRACE2(eLevelError,"CConf::IvrConfTerminate : Already in TERMINATION state, Name - ",m_name);
		return;
	}

	DeleteTimer(ALERTOUT);
	DeleteTimer(CONFTOUT);

	// destroy the conference
	// VNGR-5479
	Destroy(DISCONNECTED_BY_CHAIR);
	SetConfTerminateCause(TERMINATE_BY_USER);
}

////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnSetAutoTerminateTimerCONNECT(CSegment* pParam)
{
  ActivateAutoTermenation("CConf::OnSetAutoTerminateTimerCONNECT");
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnSendIpMonitorReqToParty( CSegment* pParam )
{
  CPartyApi partyApi;
  char	partyName[H243_NAME_LEN];
  DWORD PartyId=-0xffffffff;

  *pParam >> PartyId >> partyName;

  CPartyConnection*  pPartyConnection = GetPartyConnection(partyName);
  if( !CPObject::IsValidPObjectPtr(pPartyConnection))
  {
      PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : party name was not found : ",partyName);
      return;
  }

  //Do not send message when party is disconnected
  if (pPartyConnection->IsDisconnect() == YES || pPartyConnection->IsDisconnectDelay())
  {
	  PTRACE(eLevelInfoNormal,"CConf::OnSendIpMonitorReqToParty - IsDisconnect or IsDisconnectDelay");
	  return;
  }

  if (pPartyConnection->GetConnectionDelayTime() > 0)
  {
      PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : party is in delay connection state : ",partyName);
      return;
  }

  if (pPartyConnection->GetPartyControlState() == ALLOCATE_RSC)
  {
      PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : party is in Allocate resource state : ",partyName);
      return;
  }

  if(pPartyConnection->IsTaskAppIsValid() == FALSE)
  {
      PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : m_party is not valid : ",partyName);
      return;
  }

  CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyName);
  if ( !pConfParty )
  {
      PTRACE(eLevelInfoNormal,"CConf::OnSendIpMonitorReqToParty - pConfParty == NULL");
	  return;
  }

  if (pConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE)
  {
      TRACESTR (eLevelInfoNormal) << "CConf::OnSendIpMonitorReqToParty PSTN/ISDN party- no monitoring" ;
      return;
  }

  if(pConfParty->GetPartyState() == PARTY_STAND_BY)// The party in stand_by.
   {
       PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : party still in standby state : ",partyName);
       return;
   }

  if (pConfParty->GetPartyState() == PARTY_REDIALING)
  {
      PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : party is in redialing state : ",partyName);
      return;
  }

  CTaskApp* pPartyObject = NULL;
  pPartyObject = (CTaskApp*)GetLookupTableParty()->Get(pPartyConnection->GetPartyRsrcId());

  PASSERT_AND_RETURN(!pPartyObject);

  if( (&(pPartyObject->GetRcvMbx()))==NULL)
  {
	  PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : party's RcvMbx is NULL : ",partyName);
	  DBGPASSERT((DWORD)pPartyObject);
      return;
  }

  if( (&(pPartyObject->GetRcvMbx()))==NULL)
  {
	  PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : party's RcvMbx is NULL : ",partyName);
	  DBGPASSERT((DWORD)pPartyObject);
      return;
  }

  partyApi.CreateOnlyApi(pPartyObject->GetRcvMbx());
  partyApi.SendPartyMonitoringReq(pPartyObject);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnUpdateVisualNameForParty(CSegment* pParam)
{
	DWORD partyId = 0;
	WORD len =0;
	*pParam >> partyId >> len;
	ALLOCBUFFER(visualName, H243_NAME_LEN);
	memset(visualName,'\0',len);
	pParam->Get((unsigned char*)visualName, len);
	PTRACE2(eLevelInfoNormal, "CConf::OnUpdateVisualNameForParty: ",visualName);
	CPartyConnection*  pPartyConnection = NULL;
	CConfParty *pConfParty = m_pCommConf->GetCurrentParty(partyId);
	if(pConfParty)
	{
		pPartyConnection = ((CConf*)this)->GetPartyConnection(pConfParty->GetName());
		if( !CPObject::IsValidPObjectPtr(pPartyConnection) ) //vngr-10186
		{
		    PTRACE2(eLevelInfoNormal,"CConf::OnUpdateVisualNameForParty : party connection was not found : ", pConfParty->GetName());
			DEALLOCBUFFER(visualName);
			return;
		}

		PTRACE2(eLevelInfoNormal, "CConf::OnUpdateVisualNameForParty, party name is ", pConfParty->GetName());

		BOOL IsVideoParty = m_pVideoBridgeInterface->IsPartyConnected((CTaskApp*)pPartyConnection->GetPartyTaskApp());
		if(!pConfParty->GetVoice() && IsVideoParty)
		{
			m_pVideoBridgeInterface->SetSiteName(pConfParty->GetName(), visualName);
		}
		else
		{
			PTRACE2(eLevelInfoNormal, "CConf::OnUpdateVisualNameForParty, party is audio only or not connected to video bridge ",visualName);
		}
	}

	DEALLOCBUFFER(visualName);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::OnLobbyAddPartyConnect(CSegment* pParam)
{
  PTRACE2(eLevelInfoNormal,"CConf::OnLobbyAddPartyConnect - ConfName=", m_name);
  DeleteTimer(AUTOTERMINATE);

  CTaskApp*  pParty=0;
  char	   name[H243_NAME_LEN];

   *pParam >> (DWORD&)pParty;
   *pParam >> name;

  if (!strcmp(name, "UNKNOWN"))                                 // not meet me per party
  {
     PTRACE2(eLevelError,"CConf::OnLobbyAddPartyConnect : \'MEET ME PER CONF NOT SUPPORTED !!!\' ",m_name);
     CConfParty*  pConfParty = new CConfParty;
     char  countStr[20];
     sprintf(countStr,"%d",m_inCallCounter++);
     strncat(name,countStr,H243_NAME_LEN-1-strlen(name));
     name[H243_NAME_LEN-1]='\0';

     pConfParty->SetPartyState(PARTY_CONNECTING,m_pCommConf->GetMonitorConfId());
     pConfParty->SetDisconnectCause(0xFFFFFFFF);
     pConfParty->SetQ931DisconnectCause(0);

     m_pCommConf->Add(*pConfParty);  //?? Update

     POBJDELETE(pConfParty);
   }

   CConfParty*  pConfParty = m_pCommConf->GetCurrentParty(name);
   // Romem clokwork
   if(pConfParty == NULL)
   {
	   CMedString msg1;
	   msg1 << "Name - " << m_name << "," << name;
	   PTRACE2(eLevelInfoNormal,"CConf::OnLobbyAddPartyConnect : Party is not in Conf DB ", msg1.GetString());
	   return;
   }
   if(pConfParty->GetVoice ())
     {
       LobbyAddPartyVoiceConnect( pParam, pParty, name, pConfParty );
   }
   else  // ISDN
   {
	   LobbyAddPartyIsdnVideoConnect( pParam, pParty, name, pConfParty );
   }

   // Write to CDR - Romem
   if(pConfParty->IsUndefinedParty())
         m_pCommConf->NewUndefinedParty(pConfParty,EVENT_NEW_UNDEFINED_PARTY);
}

/////////////////////////////////////////////////////////////////////////////
// in case of net channel descriptor transfer -
// identication done by callee number or meet me by channel.
void  CConf::OnLobbyAddPartyChannelDescConnect(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnLobbyAddPartyChannelDescConnect : Name - ",m_name);
	WORD channelNum = 0;
//	CNetConnRsrcDesc*  pNetConnDesc = NULL;
	CIsdnNetSetup *		pNetSetUp = NULL;
	char  partyName[H243_NAME_LEN];

	*pParam >> partyName
		>> channelNum;

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyName);
	PASSERT(!pConfParty);

	pNetSetUp = new CIsdnNetSetup;

	pNetSetUp->DeSerialize(NATIVE,*pParam);
//
	CPartyConnection*  pPartyConnection;
	pPartyConnection = GetPartyConnection(partyName);

	if( !pPartyConnection )
	{
		POBJDELETE(pNetSetUp);
		PASSERT_AND_RETURN(1);
	}
	else
	{
		pPartyConnection->AddPartyChannel(/**pNetConnDesc,*/ *pNetSetUp,channelNum);
	}

	POBJDELETE(pNetSetUp);
}

/////////////////////////////////////////////////////////////////////////////
void  CConf::OnContentBrdgConnectSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgConnectSetup : Name - ",m_name);
	WORD  status;
	*pParam >> status;
  if (status)
  {
		// failed to connect content bridge
		// action - reallocate content bridge.
		ON(m_isBridgeError);

		DWORD updatedStatus = 0;
		updatedStatus = (CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
		UpdateConfStatus(updatedStatus,YES);
		PTRACE(eLevelError,"CConf::OnContentBrdgConnectSetup : failed to connect content bridge");
	}

	ON(m_isContentConnected);
	BridgeConnectionCompleted();
}
/////////////////////////////////////////////////////////////////////////////
void CConf::OnContentBrdgDisConnectTerminate( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgDisConnectTerminate : Name - ",m_name);
	OnContentBrdgDisConnect(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CConf::OnContentBrdgDisConnectConnect( CSegment* pParam )
{
	PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgDisConnectSetup : Name - ",m_name);
	OnContentBrdgDisConnect(pParam);
}
/////////////////////////////////////////////////////////////////////////////
void CConf::OnContentBrdgDisConnect( CSegment* pParam )
{
	WORD  status;
	*pParam >> status;
	PASSERT(status);

	OFF(m_isContentConnected);

	char tmp[H243_NAME_LEN+100];
	snprintf(tmp, sizeof(tmp), "[%s] - Conf Content Brdg disconnected.", m_name);
	PTRACE2(eLevelInfoNormal," ---> ",tmp);

	BridgeDisConnectionCompleted();
}

//////////////////////////////////////////////////////////////////////////////////////
void CConf::OnContentBrdgStartContentCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgStartContentCONNECT : Name - ",m_name);
	BYTE isContentSnatching = 0; // For Content-TX
	// 1. Select Content Rate
	//  Highest Common mechanism for content rate
	BYTE selectedContentRateAMC = SelectContentRate(TRUE); //TRUE->update actual ip rate for content

	// if there is no parties able to open content
	if (selectedContentRateAMC == AMC_0k)
	{
		CConfApi *pSelfApi = new CConfApi;
		pSelfApi->CreateOnlyApi(GetRcvMbx(), this);
		pSelfApi->ContentTokenWithdraw();
		pSelfApi->DestroyOnlyApi();
		POBJDELETE(pSelfApi);
		return;
	}

	// since no legacy in cop we use InformVideoBridgeStartContentPresentation to ignore intra suppression
	if (GetVideoSession() == VIDEO_SESSION_COP) // to ignore intra suppression
	{
		PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgStartContentCONNECT - InformVideoBridgeStartContentPresentation : Name - ",m_name);
		BYTE activeProtocol = GetCurrentContentProtocolInConfValues();
		DWORD activeContentRate = 100*TranslateAMCRateIPRate(selectedContentRateAMC);
		InformVideoBridgeStartContentPresentation(activeProtocol,activeContentRate);
	}

	if (m_pCommConf->GetContentMultiResolutionEnabled())
	{
		BYTE currContentProtocol = 0;
		BYTE oldContentPtotocol = m_contentXcodeSrcProtocol;
		*pParam >> currContentProtocol;

		if(currContentProtocol != H264 && currContentProtocol != H263)
			PASSERT(currContentProtocol+1);
		else
			m_contentXcodeSrcProtocol = currContentProtocol;


		if(m_mapXCodeRsrc.size() == 0)
		{
			STATUS allocateConfStatus =	SyncRsrcAllocConfSpreading(ALLOCATE_CONTENT_XCODE_REQ);

			// Rollback to VSW // Yiye
			if (allocateConfStatus != STATUS_OK)
			{
				DowngradeConferenceFromXCodeToVSW();
			}
			else
			{
				PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgStartContentCONNECT - Creating XCode bridge: Name - ",m_name);
				CreateVideoBridge(eVideoXcode_Bridge_V1);
			}

			return;
		}
		else
		{
			if (m_pCommConf->GetIsAsSipContent() && !m_IsAsSipContentEnable)
			{
				ON(m_IsAsSipContentEnable);
				TRACEINTO << "Start Content in AS-SIP Conference, Conf Name:"  <<  m_name << "\n";
			}

			if (oldContentPtotocol  != m_contentXcodeSrcProtocol)
			{
				*pParam >> isContentSnatching;

				if(isContentSnatching)
				{
					TRACEINTO << " Content Snatching in XCode Conf, New Content Protocol: " << (int)m_contentXcodeSrcProtocol <<
							" Old Content Protocol: " << (int)oldContentPtotocol <<
							" Conf Name: " << m_name << "\n";
				}
			}
		}
	}

	if(!isContentSnatching)
	{
		// 2. set new content rate to UnifideComMode
		SetNewContentBitRate(selectedContentRateAMC);

		//3 . Update Content Bridge - CHANGE_RATE
		if( !CPObject::IsValidPObjectPtr(m_pContentBridge) ) {
			PASSERT_AND_RETURN(1);
		}

		m_pContentBridge->ContentRate(selectedContentRateAMC);

		// 4. ChangeMode for all parties
		ChangeContentMode(NULL);
	}

	// 5. StartContent to VideoBridge in Legacy content as video conference
	BYTE activeProtocol = GetCurrentContentProtocolInConfValues();
	DWORD activeContentRate = 100*TranslateAMCRateIPRate(selectedContentRateAMC);

	if(m_pCommConf->GetContentMultiResolutionEnabled())
	{
		activeProtocol = m_contentXcodeSrcProtocol;
		activeContentRate = SelectMaxContentRateForXCodeDecoder();
	}

	InformVideoBridgeStartContentPresentation(activeProtocol,activeContentRate);
}
////////////////////////////////////////////////////////////////////////////////////////
void CConf::UpdateUnifiedAndContentBrdg(BYTE newProtocol,CPartyConnection* pPartyConnection,BOOL updateContentBrdg, BOOL bZeroContentRate, BOOL isContentRateChanged)
{
	// Set Content Rate to 0 (AMC_0k)
	if(bZeroContentRate)
	{
	  SetNewContentBitRate(AMC_0k);
	  SetActualIpRateForContentSession(0);
	}

	//1) Update the unifiedComMode with the new protocol
	SetNewContentProtocol(newProtocol,cmCapReceiveAndTransmit);

    //2)send new protocol to Content Bridge
	if(updateContentBrdg)
	{
	    //HP content
	    BYTE 	isH264HighProfile = FALSE;
	    if (newProtocol == H264)
	        isH264HighProfile = GetCurrentContentIsHighProfile();

	     m_pContentBridge->ContentProtocol(newProtocol, isH264HighProfile);
	}

	 //3)send change mode to all parties
	  ChangeContentMode(pPartyConnection,isContentRateChanged,TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnContentBrdgStopContentCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgStopContentCONNECT : Name - ",m_name);
	m_contentXcodeSrcProtocol = 0; // Reset sender protocol
	// First Send Stop Content to VideoBridge - For Legacy needed

	// send intra to all EPs after start content - Merge LGT changes to 7.2.2
	//if(IsLegacyShowContentAsVideo())
	 InformVideoBridgeStopContentPresentation();

	// since no legacy in cop we use InformVideoBridgeStopContentPresentation to ignore intra suppression
	if(GetVideoSession() == VIDEO_SESSION_COP) // to ignore intra suppression
	{
		PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgStopContentCONNECT - InformVideoBridgeStopContentPresentation : Name - ",m_name);
		InformVideoBridgeStopContentPresentation();
	}


	if(m_IsAsSipContentEnable)
		OFF(m_IsAsSipContentEnable);

	BYTE IsProtocolChanged = FALSE;
    	BYTE selectedContentProtocol = SelectContentProtocol(FALSE);

	if( !CPObject::IsValidPObjectPtr(m_pContentBridge) ) {
		PASSERT_AND_RETURN(1);
	}

	if (AMC_0k != GetCurrentContentBitRateAMC())
	{
		//1. Set Content Rate to 0 (AMC_0k)
		SetNewContentBitRate(AMC_0k);
		SetActualIpRateForContentSession(0);

		//HP content
		BYTE isCurContentHighProfile = FALSE;
		BYTE isSelectContentHighProfile = FALSE;
		BYTE isHighProfileChanged = FALSE;
		if (!IsTIPContentEnable() && selectedContentProtocol == H264)
		{
			if (selectedContentProtocol == GetCurrentContentProtocolInConfValues())
				isCurContentHighProfile = GetCurrentContentIsHighProfile();
			isSelectContentHighProfile = SelectContentH264HighProfile();
			PTRACE2INT(eLevelInfoNormal,"CConf::OnContentBrdgStopContentCONNECT, selected profile - ",isSelectContentHighProfile);
			PTRACE2INT(eLevelInfoNormal,"CConf::OnContentBrdgStopContentCONNECT, current profile - ",isCurContentHighProfile);
		}
		isHighProfileChanged = (isCurContentHighProfile != isSelectContentHighProfile);

		//if protocol was changed we need to change content protocl to all parties !!
		if(selectedContentProtocol != GetCurrentContentProtocolInConfValues() || isHighProfileChanged)
		{
			IsProtocolChanged = TRUE;

			//HP content
			if (isHighProfileChanged && isSelectContentHighProfile)
			{
				SetCurrentContentIsHighProfile(TRUE);
				PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgStopContentCONNECT :  Changing Content Profile from BaseProfile to HighProfile, Conf Name  - ",m_name);
			}

			// Update the unifiedComMode with the new protocol
			SetNewContentProtocol(selectedContentProtocol,cmCapReceiveAndTransmit);

			// Update Content Bridge - CHANGE_PROTOCOL
			m_pContentBridge->ContentProtocol(selectedContentProtocol, GetCurrentContentIsHighProfile()); //HP content

			PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgStopContentCONNECT - Content protocol and rate was changed! : Name - ",m_name);

			//ChangeMode (Protocol and rate) for all parties
	        	ChangeContentMode(NULL,TRUE,IsProtocolChanged);
    		}
    		else    // If only rate changed
    		{
			PTRACE2(eLevelInfoNormal,"CConf::OnContentBrdgStopContentCONNECT - Content rate was changed! : Name - ",m_name);
			m_pContentBridge->ContentRate(AMC_0k);
			//ChangeMode (rate) for all parties
	        	ChangeContentMode(NULL,TRUE,IsProtocolChanged);
		}
	}
}
////////////////////////////////////////////////////////////////////
void CConf::ConnectPartyToXcodeBridge(CTaskApp *pParty)
{
	if (!CPObject::IsValidPObjectPtr(pParty))
		return;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	if (pPartyConnection)
	{
		CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
		if (pConfParty)
		{
			TRACEINTO << "MonitorPartyId:" << pPartyConnection->GetMonitorPartyId() << " - Connect party to XCode bridge";
			ConnectPartyToXCodeEncoder(pPartyConnection, pConfParty);
		}
	}
}

////////////////////////////////////////////////////////////////////
void CConf::OnXCodeBrdgConnectConnect(CSegment* pParam)
{
	WORD status = STATUS_OK;
	*pParam >> status;

	TRACEINTO << "ConfName:" << m_name << ", Status:" << status;

	if (status == STATUS_OK)
	{
		ON(m_isContentXCodeConnected);

		CBridgePartyVideoOutParams bridgePartyVideoOutParams;

		// Send update to all relevant XCode encoders
		for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end(); _ii++)
		{
			eXcodeRsrcType ePartyXCodeEncoderType = _ii->first;
			if (ePartyXCodeEncoderType < eXcodeH264LinksEncoder)
			{
				InitPartyXCodeParamsAccordingToEncoderType(ePartyXCodeEncoderType, &bridgePartyVideoOutParams, TRUE);
				m_pContentXcodeBridgeInterface->UpdateVideoOutParams(ePartyXCodeEncoderType, &bridgePartyVideoOutParams);
			}
		}

		PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
		for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			if (pPartyConnection)
			{
				CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
				if (pPartyCntl)
				{
					CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
					if (pConfParty)
					{
						DWORD partyState = pConfParty->GetPartyState();
						if (partyState != PARTY_CONNECTED && partyState != PARTY_CONNECTED_WITH_PROBLEM)
							continue;

						TRACEINTO << pPartyCntl->GetFullName() << " - Connect party to XCode bridge";
						ConnectPartyToXCodeEncoder(pPartyConnection, pConfParty);
					}
				}
			}
		}
		BYTE selectedContentRateAMC = SelectContentRate(TRUE);
		if (selectedContentRateAMC == AMC_0k)
		{
			CConfApi *pSelfApi = new CConfApi;
			pSelfApi->CreateOnlyApi(GetRcvMbx(), this);
			pSelfApi->ContentTokenWithdraw();
			pSelfApi->DestroyOnlyApi();
			POBJDELETE(pSelfApi);
			return;
		}
		// 2. set new content rate to UnifideComMode
		SetNewContentBitRate(selectedContentRateAMC);

		//3 . Update Content Bridge - CHANGE_RATE
		if (!CPObject::IsValidPObjectPtr(m_pContentBridge))
		{
			PASSERT_AND_RETURN(1);
		}

		m_pContentBridge->ContentRate(selectedContentRateAMC);

		// 4. ChangeMode for all parties
		ChangeContentMode(NULL);
		// 5. StartContent to VideoBridge in Legacy content as video confrences
		BYTE activeProtocol = GetCurrentContentProtocolInConfValues();
		DWORD activeContentRate = 100 * TranslateAMCRateIPRate(selectedContentRateAMC);
		if (m_pCommConf->GetContentMultiResolutionEnabled())
		{
			activeProtocol = m_contentXcodeSrcProtocol;
			activeContentRate = SelectMaxContentRateForXCodeDecoder();
		}
		InformVideoBridgeStartContentPresentation(activeProtocol, activeContentRate);
	}
	else
	{
		// Error Handling
		m_pContentXcodeBridgeInterface->Disconnect();
		m_pContentXcodeBridgeInterface->Terminate();
	}
}

////////////////////////////////////////////////////////////////////
void CConf::OnXCodeBrdgConnectSetup(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnXCodeBrdgConnectSetup : Name - ",m_name);
	WORD status = STATUS_OK;
	*pParam >> status;

	if (status)
	{
		// failed to connect audio bridge. action - drop conference,reallocate audio bridge
		ON(m_isBridgeError);

		DWORD updatedStatus = 0;
		updatedStatus = (CONFERENCE_EMPTY | CONFERENCE_NOT_FULL | CONFERENCE_RESOURCES_DEFICIENCY);
		UpdateConfStatus(updatedStatus,YES);
		PTRACE(eLevelError,"CConf::OnXCodeBrdgConnectSetup : failed to connect XCode bridge");
	}

	ON(m_isContentXCodeConnected);
	BridgeConnectionCompleted();
}
/////////////////////////////////////////////////////////////////////
void CConf::OnXCodeBrdgDisConnectTerminate(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnXCodeBrdgDisConnectTerminate : Name - ",m_name);
	OFF(m_isContentXCodeConnected);
	BridgeDisConnectionCompleted();
}
/////////////////////////////////////////////////////////////////////
void CConf::OnXCodeBrdgDisConnectConnect(CSegment* pParam)
{
	WORD status;
	PTRACE2(eLevelInfoNormal,"CConf::OnXCodeBrdgDisConnectConnect : Name - ",m_name);
	*pParam >>  status;
	PASSERT(status);
	OFF(m_isContentXCodeConnected);
	POBJDELETE(m_pContentXcodeBridgeInterface );
	UpdateXCodeBrdgInterfaceForEachPartyCntl();
	DowngradeConferenceFromXCodeToVSW();
}

void CConf::UpdateXCodeBrdgInterfaceForEachPartyCntl()
{
	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (pPartyConnection)
		{
			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (pPartyCntl)
				pPartyCntl->SetNewXCodeBridgeInterfaceOff();
		}
	}
}

////////////////////////////////////////////////////////////////////
void CConf::OnPartyLinkConnectCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnPartyLinkConnectCONNECT : Name - ",m_name);
	BYTE linkType = 0;

	CContentBridgePartyInitParams* pContentBridgePartyInitParams = new CContentBridgePartyInitParams;
	pContentBridgePartyInitParams->DeSerialize(NATIVE, *pParam);
	BYTE isLinkToMasterConnectWhileContent = NO;

	//When SlaveLink connected to a conference the Bridge should changed to SlaveBridge
	if(CASCADE_MODE_SLAVE == pContentBridgePartyInitParams->GetCascadeLinkMode()) //Slave Link
	{
		PTRACE(eLevelInfoNormal,"CConf::OnPartyLinkConnectCONNECT - CASCADE_MODE_SLAVE Link");
		if ( YES == (m_pContentBridge->AmISlaveBridge()) ) //Another linkToMaster try to connect
		{
			PTRACE(eLevelInfoNormal,"CConf::OnPartyLinkConnectCONNECT : Configuration ERROR !!! another SLAVE Link is trying to connect Slave can`t be connected to 2 Masters");
			delete pContentBridgePartyInitParams;
			PASSERT_AND_RETURN(222);
		}


		if(m_pContentBridge->IsTokenHeld())
		{
			//Blocked for China
			//isLinkToMasterConnectWhileContent = YES; //To send disconnect from here??
			PTRACE2(eLevelInfoNormal,"CConf::OnPartyLinkConnectCONNECT : There is Content on the going to be SLAVE bridge For China usages WITHDRAW the token in this tree!! ConfName - ", m_name);
			BYTE isImmediate = YES;
			((CContentBridge*)m_pContentBridge)->ContentTokenWithdraw(isImmediate);
			//((CContentBridge*)m_pContentBridge)->AbortPresentation();
		}
     	const CTaskApp *pParty = pContentBridgePartyInitParams->GetParty();
		SetLastRateFromMaster(pParty);

		ChangeContentBridgeToSlave();
		m_pCommConf->SetCurrentConfCascadeMode(CASCADE_MODE_SLAVE);
	}

	//MasterLink connect to SlaveBridge ==> Bridge stay Slave
	//MasterLink connect to MasterBridge ==> Bridge stay Master
	//MasterLink connect to BasicBridge ==> Bridge change to Master
	else if (CASCADE_MODE_MASTER == pContentBridgePartyInitParams->GetCascadeLinkMode()) //Master Link
	{
		PTRACE(eLevelInfoNormal,"CConf::OnPartyLinkConnectCONNECT - CASCADE_MODE_MASTER Link");
		if ((NO == m_pContentBridge->AmISlaveBridge()) && (NO == m_pContentBridge->AmIMasterBridge()))//Basic Bridge
		{
			ChangeContentBridgeToMaster();
			m_pCommConf->SetCurrentConfCascadeMode(CASCADE_MODE_MASTER);
		}
	}

	m_pContentBridge->ConnectParty(pContentBridgePartyInitParams);
	//Blocked for China
//	if (YES == isLinkToMasterConnectWhileContent)
//	{
//		PTRACE(eLevelInfoNormal,"CConf::OnPartyLinkConnectCONNECT :LinkToMasterConnectWhileContent -> back to basic ContentBridge !!");
//		ChangeContentBridgeToBasicBridge();
//	}

	POBJDELETE(pContentBridgePartyInitParams);
}

//--------------------------------------------------------------------------
void CConf::OnPartyLinkDisconnectCONNECT(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID PartyId = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << PartyId << ", MediaDirection:" << eMediaDirection;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(PartyId);
	PASSERT_AND_RETURN(!pParty);

	CContentBridgePartyCntl* pPartyCntl = (CContentBridgePartyCntl*)m_pContentBridge->GetPartyCntl(PartyId);
	PASSERT_AND_RETURN(!pPartyCntl);

	BYTE isSlave = m_pContentBridge->AmISlaveBridge();
	BYTE isBridgeWithAnotherLink = m_pContentBridge->HaveIAnotherLink(pPartyCntl);
	BYTE disconnectLinkCascadeMode = pPartyCntl->GetCascadeLinkMode(); // can be only on SlaveConf

	if (isSlave)
	{
		if (CASCADE_MODE_SLAVE == disconnectLinkCascadeMode)             // LinkToMaster disconnecting on Slave conf
		{
			if (NO == isBridgeWithAnotherLink)
			{
				ChangeContentBridgeToBasicBridge();                          // Back to regular bridge
				m_pCommConf->SetCurrentConfCascadeMode(CASCADE_MODE_NONE);
			}
			else
			{
				ChangeContentBridgeToMaster();
				m_pCommConf->SetCurrentConfCascadeMode(CASCADE_MODE_MASTER);
			}

			TRACEINTO << "PartyId:" << PartyId << " - Disconnect LinkToMaster ==> for China usages, force WITHRAW content from the slaves";
			CConfApi* pSelfApi = new CConfApi;
			pSelfApi->CreateOnlyApi(GetRcvMbx());
			pSelfApi->ContentTokenWithdraw();
			pSelfApi->DestroyOnlyApi();
			POBJDELETE(pSelfApi);
		}
	}
	else
	{
		if (NO == isBridgeWithAnotherLink)    // MasterConference and Not in Star topology Master has only Master links
		{
			ChangeContentBridgeToBasicBridge(); // Back to regular bridge
			m_pCommConf->SetCurrentConfCascadeMode(CASCADE_MODE_NONE);
		}
	}
	m_pContentBridge->DisconnectParty(&partyDisconnectParams);
}

//--------------------------------------------------------------------------
void CConf::OnPartyLinkDisconnectTERMINATION(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID PartyId = partyDisconnectParams.GetPartyId();
	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();

	TRACEINTO << "PartyId:" << PartyId << ", MediaDirection:" << eMediaDirection;

	m_pContentBridge->DisconnectParty(&partyDisconnectParams);
}

///////////////////////////////////////////////////////////////////////////////
void CConf::ChangeContentBridgeToSlave()
{
	PTRACE2(eLevelInfoNormal,"CConf::ChangeContentBridgeToSlave : Name - ",m_name);

	CContentBridgeSlave* pTempBridge = new CContentBridgeSlave ;
	pTempBridge->Create(m_pContentBridge);
	POBJDELETE(m_pContentBridge);
	m_pContentBridge = new CContentBridgeSlave;
	((CContentBridgeSlave*)m_pContentBridge)->Create((CContentBridge*)pTempBridge);
    SetTheNewBridgeForAllParties();
    POBJDELETE(pTempBridge);
}

///////////////////////////////////////////////////////////////////////////////
void CConf::ChangeContentBridgeToMaster()
{
	PTRACE2(eLevelInfoNormal,"CConf::ChangeContentBridgeToMaster : Name - ",m_name);

	CContentBridgeMaster* pTempBridge = new CContentBridgeMaster ;
	pTempBridge->Create(m_pContentBridge);
	POBJDELETE(m_pContentBridge);
	m_pContentBridge = new CContentBridgeMaster;
	((CContentBridgeMaster*)m_pContentBridge)->Create((CContentBridge*)pTempBridge);
    SetTheNewBridgeForAllParties();
    POBJDELETE(pTempBridge);
}
///////////////////////////////////////////////////////////////////////////////
void CConf::ChangeContentBridgeToBasicBridge()
{
	PTRACE2(eLevelInfoNormal,"CConf::ChangeContentBridgeToBasicBridge : Name - ",m_name);

	CContentBridge* pTempBridge = new CContentBridge ;
	pTempBridge->Create(m_pContentBridge);
	POBJDELETE(m_pContentBridge);
	m_pContentBridge = new CContentBridge;
	((CContentBridge*)m_pContentBridge)->Create(pTempBridge);
	SetTheNewBridgeForAllParties();

	//Chack if need to change content protocol
    if ( IsEnableH239() )
        DispatchEvent(UPDATE_CONTENT_PROTOCOL);

    POBJDELETE(pTempBridge);
}

////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetTheNewBridgeForAllParties()
{
	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (pPartyConnection)
		{
			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (pPartyCntl && pPartyCntl->GetContentBridge())
				pPartyCntl->SetNewContentBridge(m_pContentBridge);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnOperatorAssistance(CSegment *pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnOperatorAssistance [operator_assistance_trace] : conf_name = ",m_name);
	DWORD partyId = (DWORD)-1;
	BYTE action = 0;
	BYTE mode = 0;
	BYTE init_by = 0;

	*pParam >> partyId >> action >> mode >> init_by;

	UpdateConfStatus();
}

/////////////////////////////////////////////////////////////////////////////
void CConf::OnIvrMuteAllButX(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnIvrMuteAllButX : Name - ",m_name);

	DWORD partyRsrcID = 0;
	BYTE  yesNo = 0;
	int val = -1;

	*pParam >> partyRsrcID;
	*pParam >> yesNo;

  if (yesNo)
  {
		PTRACE2(eLevelInfoNormal,"CConf::OnIvrMuteAllButX - Set Mute Party - Name - ",m_name);
		m_pCommConf->SetIsMuteAllButX(partyRsrcID);
	}
  else
  {
		PTRACE2(eLevelInfoNormal,"CConf::OnIvrMuteAllButX - Set UnMute (-1) - Name - ",m_name);
		m_pCommConf->SetIsMuteAllButX(val);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnIvrShowParticipants(CSegment* pParam)
{
	DWORD actionSrc;
	PartyRsrcID partyId;
	DWORD permission;
	*pParam >> actionSrc;   // DTMF_SHOW_PARTICIPANTS, DTMF_SECURE_CONF, DTMF_UNSECURE_CONF
	*pParam >> partyId;
	*pParam >> permission;  // DTMF_LEADER_ACTION, DTMF_USER_ACTION

	TRACEINTO << "PartyId:" << partyId << ", ActionSrc:" << actionSrc << ", Permission:" << permission;

	if (m_pGatheringManager && m_pGatheringManager->IsGatheringEnabled())
	{
		CPartyCntl* pPartyCntl = GetPartyCntl(partyId);
		if (pPartyCntl)
		{
			TRACEINTO << "PartyId:" << partyId << " - Show Gathering to party";
			m_pGatheringManager->ShowGatheringToParty(pPartyCntl->GetName());
			return;
		}
	}

	BOOL bEnableTextOnScreen = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("ENABLE_TEXTUAL_CONFERENCE_STATUS", bEnableTextOnScreen);
	if (!bEnableTextOnScreen)
	{
		TRACEINTO << "PartyId:" << partyId << " - ENABLE_TEXTUAL_CONFERENCE_STATUS is disable in systemCfg";
		return;
	}

	CPartyCntl* pPartyCntl = GetPartyCntl(partyId);
	PASSERTSTREAM_AND_RETURN(!pPartyCntl, "PartyId:" << partyId);

	// here we should send a command to VB to send "text to screen"
	BYTE IsConfSecure = FALSE;
	BYTE IsVideoParty = FALSE;
	WORD AudioCounter = 0;
	WORD VideoCounter = 0;

	char** VideoArr = new char*[MAX_TEXT_LEN]; // 800
	char** AudioArr = new char*[MAX_TEXT_LEN];

	// Find total number of connected parties
	// Fill Video Arr and Audio Arr with parties names

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (pPartyConnection)
		{
			CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
			if (pPartyCntl)
			{
				CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());

				if (pConfParty)
				{
					// VNGFE_8038
					BOOL isLyncCcs = FALSE;
					BOOL isTipSlave = FALSE;
					BOOL isCascadeLink = FALSE;

					if (pConfParty->GetRemoteIdent() == MicrosoftEP_Lync_CCS)
					{
						TRACEINTO << "Lync_CCS: " << pPartyCntl->GetSiteName();

						isLyncCcs = TRUE;
					}

					if (pConfParty->IsTIPSlaveParty())
					{
						TRACEINTO << "TIPSlave: " << pPartyCntl->GetSiteName();

						isTipSlave = TRUE;
					}

					if (pConfParty->GetCascadeMode() != CASCADE_MODE_NONE &&
                                             !pConfParty->GetIsCallFromGW())
					{
						TRACEINTO << "CascadeLink: " << pPartyCntl->GetSiteName();

						isCascadeLink = TRUE;
					}

					DWORD partyState = pConfParty->GetPartyState();

					if (partyState != PARTY_CONNECTED &&
					    partyState != PARTY_CONNECTED_PARTIALY &&
					    partyState != PARTY_SECONDARY &&
					    partyState != PARTY_CONNECTED_WITH_PROBLEM)
						continue;
					else if (isLyncCcs || isTipSlave || isCascadeLink)
						continue;
					else
					{
						IsVideoParty = m_pVideoBridgeInterface->IsPartyConnected(pPartyConnection->GetPartyTaskApp());

						if ((IsVideoParty) && (partyState == PARTY_CONNECTED ||
						                       (partyState == PARTY_CONNECTED_WITH_PROBLEM && pPartyCntl->GetmIsMemberInVidBridge()) ||
						                       (partyState == PARTY_CONNECTED_PARTIALY && pPartyCntl->GetmIsMemberInVidBridge()))) // Video party
						{
							VideoArr[VideoCounter] = new char[MAX_SITE_NAME_ARR_SIZE];
							memset(VideoArr[VideoCounter], '\0', MAX_SITE_NAME_ARR_SIZE);
							strncpy(VideoArr[VideoCounter], pPartyCntl->GetSiteName(), MAX_SITE_NAME_ARR_SIZE - 1);

							// VNGFE_8038
							if (pConfParty->IsTIPMasterParty())
							{
								WORD lenName = strlen(VideoArr[VideoCounter]);

								if (lenName >= 3 && VideoArr[VideoCounter][lenName - 1] == '1'
									&& VideoArr[VideoCounter][lenName - 2] == '_')
								{
									VideoArr[VideoCounter][lenName - 2] = '\0';
								}
							}

							VideoCounter++;
						}
						else if (!(IsVideoParty) && (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY ||
						                             (partyState == PARTY_CONNECTED_WITH_PROBLEM && !pPartyCntl->GetmIsMemberInVidBridge()) ||
						                             (partyState == PARTY_CONNECTED_PARTIALY && !pPartyCntl->GetmIsMemberInVidBridge()))) // Audio party
						{
							AudioArr[AudioCounter] = new char[MAX_SITE_NAME_ARR_SIZE];
							memset(AudioArr[AudioCounter], '\0', MAX_SITE_NAME_ARR_SIZE);
							strncpy(AudioArr[AudioCounter], pPartyCntl->GetSiteName(), MAX_SITE_NAME_ARR_SIZE);
							AudioCounter++;
						}
					}
				}
			}
		}
	}

	// Incase this is DTMF_SHOW_PARTICIPANTS action we need to know if this is secured conf.
	IsConfSecure = m_pCommConf->IsConfSecured();

	if (permission == DTMF_USER_ACTION && (actionSrc == DTMF_SECURE_CONF || actionSrc == DTMF_UNSECURE_CONF))               // Show to all video parties
		m_pVideoBridgeInterface->DisplayTextOnScreen(0, IsConfSecure, AudioArr, AudioCounter, VideoArr, VideoCounter, FALSE); // 0 stand for invalid party rsource ID
	else if (permission == DTMF_LEADER_ACTION && (actionSrc == DTMF_SECURE_CONF || actionSrc == DTMF_UNSECURE_CONF))        // Show only to chair
		m_pVideoBridgeInterface->DisplayTextOnScreen(partyId, IsConfSecure, AudioArr, AudioCounter, VideoArr, VideoCounter, TRUE);
	else if (actionSrc == DTMF_SHOW_PARTICIPANTS)
		m_pVideoBridgeInterface->DisplayTextOnScreen(partyId, IsConfSecure, AudioArr, AudioCounter, VideoArr, VideoCounter, TRUE);

	// delete allocated arrays
	if (VideoCounter)
	{
		for (int j = 0; j < VideoCounter; j++)
			PDELETEA(VideoArr[j]);
	}

	if (AudioCounter)
	{
		for (int k = 0; k < AudioCounter; k++)
			PDELETEA(AudioArr[k]);
	}

	PDELETEA(VideoArr);
	PDELETEA(AudioArr);
}

//////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnIvrShowGathering( CSegment* pParam )
{
	if (m_pGatheringManager && m_pGatheringManager->IsGatheringEnabled())
	{
		DWORD actionSrc;
		PartyRsrcID partyRsrcID;
		DWORD permission;

		*pParam >> actionSrc;	// DTMF_SHOW_GATHERING, DTMF_SHOW_PARTICIPANTS, DTMF_SECURE_CONF, DTMF_UNSECURE_CONF
		*pParam >> partyRsrcID;
		*pParam >> permission;	// DTMF_LEADER_ACTION, DTMF_USER_ACTION

		CPartyCntl*  pPartyCntl = GetPartyCntl(partyRsrcID);

		PASSERTSTREAM_AND_RETURN(!pPartyCntl, "PartyId:" << partyRsrcID << " - Failed. party does not exist.");

		m_pGatheringManager->ShowGatheringToParty(pPartyCntl->GetName());

	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnUpdateRecordingControlCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnUpdateRecordingControlCONNECT : Name - ",m_name);

	WORD wRecordingCommand = 0;
	CTaskApp* pPartyToReply = NULL;
	*pParam >> wRecordingCommand
			>> (DWORD&)pPartyToReply;

	CPartyConnection*	pPartyConnection = NULL;
	CPartyConnection*	pRecordingPartyConnection = NULL;
	CConfParty*			pConfParty = NULL;
	WORD numParties = m_pCommConf->GetNumParties();
	int status = STATUS_OK;

		m_pCommConf->SetRecordingLinkControl(wRecordingCommand);

		if (m_pMcmsPCMManager)
			m_pMcmsPCMManager->SendRecordingStateIndication(wRecordingCommand);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// we arrive here only in case of disconnet party when we can upgrade the content protocol from H263 to H264
void CConf::OnPartyUpdateContentProtocolCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT : Name - ",m_name);
	// In Case of XCode update all Content Encoders and return
	if(m_pCommConf->GetContentMultiResolutionEnabled())
	{
		if(m_mapXCodeRsrc.size() > 0)
		{
			for (XCODE_RSRC::iterator _ii = m_mapXCodeRsrc.begin(); _ii != m_mapXCodeRsrc.end() ; _ii++)
			{
				eXcodeRsrcType ePartyXCodeEncoderType = _ii->first;
				if(ePartyXCodeEncoderType < eXcodeH264LinksEncoder)
				{
					CBridgePartyVideoOutParams* pBridgePartyVideoOutParams = new CBridgePartyVideoOutParams;
					InitPartyXCodeParamsAccordingToEncoderType(ePartyXCodeEncoderType, pBridgePartyVideoOutParams,TRUE);
					m_pContentXcodeBridgeInterface ->UpdateVideoOutParams(ePartyXCodeEncoderType,pBridgePartyVideoOutParams);
					POBJDELETE(pBridgePartyVideoOutParams);
				}
			}
		}
		return;
	}
	//we will update content protocol only If content is closed and we want to upgrade protocol...
	//if( GetCurrentContentBitRateAMC() == AMSC_0k )
	{
		BYTE selectedContentProtocol = SelectContentProtocol(FALSE);
		BYTE currentContentProtocol = GetCurrentContentProtocolInConfValues();

		PTRACE2INT(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT, selected protocol- ",selectedContentProtocol);
		PTRACE2INT(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT, current protocol- ",currentContentProtocol);

		//HP content:
		BYTE isContentProtocolChanged = (selectedContentProtocol != currentContentProtocol);

		BYTE isCurContentHighProfile = FALSE;
		BYTE isSelectContentHighProfile = FALSE;
		BYTE isHighProfileChanged = FALSE;
		if (!IsTIPContentEnable() && selectedContentProtocol == H264)
		{
			if (!isContentProtocolChanged)
				isCurContentHighProfile = GetCurrentContentIsHighProfile();
			isSelectContentHighProfile = SelectContentH264HighProfile();
			PTRACE2INT(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT, selected profile - ",isSelectContentHighProfile);
			PTRACE2INT(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT, current profile - ",isCurContentHighProfile);
		}
		isHighProfileChanged = (isCurContentHighProfile != isSelectContentHighProfile);

		//if protocol was changed we need to change content mode to all parties !!
		if (isContentProtocolChanged || isHighProfileChanged)
		{
			// In case of protocol change if we did do not stop content session
			if (GetCurrentContentBitRateAMC() == AMSC_0k )//VNGR- 23745 to avoid changing and stopping the content in case that the content is ON (this was added by Romem/Yoella/Inga as a fix for the commented first line in this function
			{
				if (isHighProfileChanged)
				{
					if (isSelectContentHighProfile)
					{
						SetCurrentContentIsHighProfile(TRUE);
						PTRACE2(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT :  Changing Content Profile from BaseProfile to HighProfile, Conf Name  - ",m_name);
					}
					else
						PTRACE2(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT :  Changing Content Profile from HP to BP should not happen, Conf Name  - ",m_name);
				}
				UpdateUnifiedAndContentBrdg(selectedContentProtocol);
			}
			else
				PTRACE2INT(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT - Not changed during the active content:  - ", GetCurrentContentBitRateAMC());
		}
		else
			PTRACE2INT(eLevelInfoNormal,"CConf::OnPartyUpdateContentProtocolCONNECT - Protocol and profile remain the same:  - ",selectedContentProtocol);

			BOOL isContenRateChanged = FALSE;

            // Support content rate change when Content Protcol is H263
			if(currentContentProtocol == H263)
			{
				TRACEINTO << "H263 content protocol. Conference Name - " << m_pCommConf->GetName();
				if (GetCurrentContentBitRateAMC() != AMSC_0k)
				{
					BYTE newContentRateAMC  = SelectContentRate();

					if (newContentRateAMC != GetCurrentContentBitRateAMC())	// VNGFE-6589: != changed to < - blocks content rate upgrade
					{
						// send new rate to Content Bridge
						if( CPObject::IsValidPObjectPtr(m_pContentBridge) )
							m_pContentBridge->ContentRate(newContentRateAMC);

						SetNewContentBitRate(newContentRateAMC);
						DWORD H323ContentRate = m_pUnifiedComMode->TranslateAMCRateIPRate(newContentRateAMC);
						((CConf*)this)->SetActualIpRateForContentSession(H323ContentRate);

						UpdateUnifiedAndContentBrdg(currentContentProtocol,NULL,FALSE,FALSE,isContenRateChanged);
					}
				}
				return;
			}

		    if(currentContentProtocol == H264 && !isHighProfileChanged)  //HP content: only profile unchanged
		    {
              // Romem TBD - 23.1.11
		    	BYTE curHD1080ContentMpi = 0;
		    	BYTE curHD720ContentMpi	 = 0;
		        BYTE newHDContentMpi	 = 0;
		        BYTE isHD1080WillBeSupported = FALSE;
		        BYTE IsHDResChanged 		 = FALSE;

		        // Romem 3.4.11
		        BYTE newContentRateAMC  = SelectContentRate();
		        if (GetCurrentContentBitRateAMC() != AMSC_0k)
		        {
					if (newContentRateAMC != GetCurrentContentBitRateAMC())	// VNGFE-6589: != changed to < - blocks content rate upgrade
					{
						isContenRateChanged = TRUE;
						// send new rate to Content Bridge
						if( CPObject::IsValidPObjectPtr(m_pContentBridge) )
							 m_pContentBridge->ContentRate(newContentRateAMC);

						SetNewContentBitRate(newContentRateAMC);
						DWORD H323ContentRate = m_pUnifiedComMode->TranslateAMCRateIPRate(newContentRateAMC);
						((CConf*)this)->SetActualIpRateForContentSession(H323ContentRate);
					}
		        }

		    	isHD1080WillBeSupported = SelectContentHDResolution(newContentRateAMC,currentContentProtocol,newHDContentMpi);

		    	curHD1080ContentMpi = m_pUnifiedComMode->isHDContent1080Supported(cmCapTransmit);

			//HP content: solve previous bug
			curHD720ContentMpi  = m_pUnifiedComMode->isHDContent720Supported(cmCapTransmit);
			if ((isHD1080WillBeSupported && !curHD1080ContentMpi) || (!isHD1080WillBeSupported && curHD1080ContentMpi))
			{
				IsHDResChanged = TRUE;
				if (isHD1080WillBeSupported)
					TRACEINTO << "Changing Content HD Resolution from HD720 to HD1080";
				else
					TRACEINTO << "Changing Content HD Resolution from HD1080 to HD720";
			}
			if (!IsHDResChanged && isHD1080WillBeSupported && curHD1080ContentMpi)
			{
				if(newHDContentMpi != curHD1080ContentMpi)
				{
					TRACEINTO << "NewHD1080ContentMPI:" << (int)newHDContentMpi  << ", OldHD1080ContentMPI:" << (int)curHD1080ContentMpi << " - Changing Content HD 1080 MPI";
					IsHDResChanged = TRUE;
				}
			}
			if (!IsHDResChanged && !isHD1080WillBeSupported)
			{
				if(newHDContentMpi != curHD720ContentMpi)
				{
					TRACEINTO << "NewHD720ContentMPI:" << (int)newHDContentMpi  << ", OldHD720ContentMPI:" << (int)curHD720ContentMpi << " - Changing Content HD 720 MPI";
					IsHDResChanged = TRUE;
				}
			}

		    	if (IsHDResChanged)
		    	{
		    		//HP content: fix previous bug: 720p5=>720p30, but content rate (384=>2M) not changed.
				/*if (newContentRateAMC > GetCurrentContentBitRateAMC() && (GetCurrentContentBitRateAMC() != AMSC_0k))
				{
					isContenRateChanged = TRUE;

					if( CPObject::IsValidPObjectPtr(m_pContentBridge) )
						 m_pContentBridge->ContentRate(newContentRateAMC);

					SetNewContentBitRate(newContentRateAMC);
					DWORD H323ContentRate = m_pUnifiedComMode->TranslateAMCRateIPRate(newContentRateAMC);
					((CConf*)this)->SetActualIpRateForContentSession(H323ContentRate);
				}*/

                     	if (GetCurrentContentBitRateAMC() == AMSC_0k )
                     		UpdateUnifiedAndContentBrdg(currentContentProtocol,NULL,FALSE,TRUE);
                     	else if (isContenRateChanged)  //HP content: solve previous bug, currently RMX does not support HDResChange due to content rate upgrade.
                    	 	UpdateUnifiedAndContentBrdg(currentContentProtocol,NULL,FALSE,FALSE,isContenRateChanged);
		        }
		    }
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//
void CConf::OnDelayUpdateContentTimeout(CSegment* pParam)
{
	if (m_isUpdateContentPending)
	{
		m_isUpdateContentPending = FALSE;
		DispatchEvent(UPDATE_CONTENT_PROTOCOL);
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////
//
void CConf::CheckGWConfEstablishedWith2Parties(CSegment* pParam)
{
	PASSERT_AND_RETURN(!m_pCommConf->GetIsGateway());
	TerminateConfWith2PartiesOrLess();
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::TerminateConfWith2PartiesOrLess()
{
	if (m_pCommConf->GetConnectedPartiesNumber() < 2 || (m_pCommConf->IncludeRecordingParty() && m_pCommConf->GetConnectedPartiesNumber() == 2))
	{
		PTRACE(eLevelInfoNormal,"CConf::TerminateConfWith2PartiesOrLess - num connected parties in conf < 2 --> Tesrminate conf!");

		CConfApi confApi;
		confApi.CreateOnlyApi(GetRcvMbx());
		confApi.Destroy();
	}
}

//--------------------------------------------------------------------------
void CConf::OnPartyEndInitComm(CSegment* pParam)
{
	PartyRsrcID PartyId;
	*pParam >> PartyId;

	CPartyConnection* pPartyConnection = GetPartyConnection(PartyId);
	TRACECOND_AND_RETURN(!pPartyConnection, "PartyId:" << PartyId << " - Failed, Party does not exist");

	if (pPartyConnection->GetDialType() == DIALOUT)
	{
		CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
		PASSERT_AND_RETURN(!pConfParty);

		CSmallString cstr;
		cstr << pConfParty->GetPhoneNumber() << "[80%]";
		if (m_isGateWay)
			AddPartyMsgOnScreenForGWConf(pConfParty->GetPartyId(), cstr.GetString());
		if (m_isDtmfInviteParty)
			AddPartyMsgOnScreenForInvitedConf(pConfParty->GetPartyId(), cstr.GetString());
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::StopTextOnScreenMessages(CSegment* pParam)
{
	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (!pPartyConnection)
			continue;

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (!pPartyCntl)
			continue;

		if ((pPartyCntl->GetDialType() == DIALIN && pPartyCntl->GetmIsMemberInVidBridge()) || m_isDtmfInviteParty)
		{
			PartyRsrcID partyId = pPartyCntl->GetPartyRsrcId();
			m_pVideoBridgeInterface->StopDisplayPartyTextOnScreen(partyId);
			break;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnUpdateIsdnDialOutServiceNameForGW(CSegment* pParam)
{
	PASSERT_AND_RETURN(!m_pCommConf->GetIsGateway());
	CLargeString cstr;
	cstr << "CConf::OnUpdateIsdnDialOutServiceNameForGW ";
	STATUS status = STATUS_OK;

	WORD serviceNameLen = 0;

	*pParam >> serviceNameLen;
	if (!serviceNameLen)
	{
		cstr << "service name string is empty - !!!";
		DBGPASSERT(1);
	}
	else
	{
		*pParam >> serviceName;
		cstr << "service name is: " << serviceName;
	}

	PTRACE(eLevelInfoNormal,cstr.GetString());
}
//////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnUpdateDialString(CSegment* pParam)
{
	TRACEINTO << "IsGateway:" << m_pCommConf->GetIsGateway();

	PASSERT_AND_RETURN(!m_pCommConf->GetIsGateway());

	WORD dialStringLen = 0;
	char dialString[MaxAddressListSize];
	memset(dialString, '\0', MaxAddressListSize);

	STATUS status = STATUS_OK;
	*pParam >> dialStringLen;
	if (!dialStringLen)
	{
		TRACEINTO << "dial string is empty, disconnect";
		ActivateAutoTermenation("CConf::OnUpdateDialString");
		PASSERT_AND_RETURN(1);
	}

	*pParam >> dialString;

	if (dialString[dialStringLen - 1] == '*')
	{
		TRACEINTO << "dial string is invalid, final char shouldn't be '*', disconnect";
		TRACEINTO << "DialString:" << dialString;
		ActivateAutoTermenation("CConf::OnUpdateDialString");
		PASSERT_AND_RETURN(1);
	}

	CLargeString cstr;
	cstr << "DialString:" << dialString;

	/*If m_pDialingSequence is not NULL, we need to free it first*/
	if (m_pDialingSequence != NULL)
		POBJDELETE(m_pDialingSequence);

	m_pDialingSequence = new CConfPartiesDialingSequence();
	InitPartiesFromString(m_pDialingSequence, m_pCommConf, dialString);

	cstr << "\nm_pDialingSequence is: \n";
	m_pDialingSequence->DumpToTrace(cstr);

	TRACEINTO << cstr.GetString();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnGateWayConfEndSetup(CSegment* pParam)
{
	CalcGwPartiesStateFlag();

	if (GwPartiesStateFlag)
	{
		PTRACE(eLevelInfoNormal,"CConf::OnGateWayConfEndSetup");
		// some actions after all parties connected / disconnected
		// turn off text messages
		// disconnect if numParties < 2

		StopTextOnScreenMessages(NULL);//StartTimer(StopTextOnScreenMessages,5*SECOND);
		POBJDELETE(m_pDialingSequence);
		POBJDELETE(m_pGWPartiesState);
		POBJDELETE(m_pTextOnScreenMngrForGwSession);

		ActivateAutoTermenation("CConf::OnGateWayConfEndSetup");
  }
}
void CConf::OnDtmfInvitePartyConfEndSetup(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConf::OnDtmfInvitePartyConfEndSetup");

	if(m_isGateWay)
		CalcGwPartiesStateFlag();

	// some actions after all parties connected / disconnected
	// turn off text messages
	// disconnect if numParties < 2

	StopTextOnScreenMessages(NULL);
	POBJDELETE(m_pDtmfInvitedPartyDialingSequence);
	POBJDELETE(m_pInvitedPartiesState);
	POBJDELETE(m_pTextOnScreenMngrForInvitedSession);

	ActivateAutoTermenation("CConf::OnDtmfInvitePartyConfEndSetup");
	m_isDtmfInviteParty = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnMcuVideoRefreshBeforeRecording(CSegment* pParam)
{
	char partyName[H243_NAME_LEN];
	WORD direction;

	*pParam >> partyName >> direction;

	TRACEINTO << "ConfName:" << m_name << ", PartyName:" << partyName << ", Direction:" << direction;

	CPartyConnection* pPartyConnection = GetPartyConnection(partyName);
	TRACECOND_AND_RETURN(!pPartyConnection, "Failed, invalid pointer");

	CTaskApp* pParty = pPartyConnection->GetPartyTaskApp();
	TRACECOND_AND_RETURN(!pParty, "Failed, invalid pointer");

	if (direction) // out
	{
		CConfApi* pConfApi = new CConfApi;
		pConfApi->CreateOnlyApi(GetRcvMbx());
		pConfApi->VideoRefresh(pParty->GetPartyId());
		pConfApi->DestroyOnlyApi();
		POBJDELETE(pConfApi);
	}
	else // in
	{
		CPartyApi* pPartyApi = new CPartyApi;
		pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());
		pPartyApi->VideoRefresh();
		pPartyApi->DestroyOnlyApi();
		POBJDELETE(pPartyApi);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnVideoDecoderInitialSync(CSegment* pParam)
{
	if (!m_isGateWay && !m_isDtmfInviteParty)
		return;

	CTaskApp* pParty;
	*pParam >> (DWORD&)pParty;

	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	PASSERT_AND_RETURN(!pPartyConnection);

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	PASSERT_AND_RETURN(!pConfParty);

	if (m_isGateWay && !m_initialSync && pConfParty->GetGatewayPartyType() > eNormalGWPartyType)
	{
		ON(m_initialSync);
		PTRACE2(eLevelInfoNormal, "CConf::OnVideoDecoderInitialSync - video party is synched: Name - ", pPartyConnection->GetName());
		OnGateWayPartyConnected(pConfParty);
	}

	if (m_isDtmfInviteParty && !m_initialSync && pConfParty->GetGatewayPartyType() > eNormalGWPartyType)
	{
		ON(m_initialSync);
		PTRACE2(eLevelInfoNormal, "CConf::OnVideoDecoderInitialSync - video party is synched: Name - ", pPartyConnection->GetName());
		OnDtmfInvitedPartyConnected(pConfParty);
	}
}

//--------------------------------------------------------------------------
void CConf::OnGateWayPartyConnected(CConfParty* pConfParty)
{
	bool isPartyConnected = false;
	DWORD partyState = pConfParty->GetPartyState();
	if (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY || partyState == PARTY_CONNECTED_WITH_PROBLEM)
		isPartyConnected = true;

	PartyMonitorID partyId = pConfParty->GetPartyId();

	std::ostringstream msg;
	msg << "PartyMonitorId:" << partyId;

	if (IsGWPartyInSetupStage(partyId, msg) && isPartyConnected)
	{
		TRACEINTO << msg.str().c_str();

		// dial out party connected - send text message to inviter
		if (pConfParty->GetConnectionType() == DIAL_OUT)
		{
			PASSERT(!m_initialSync);

			CSmallString cstr;
			GenerateGeneralConnectedMessage(pConfParty, cstr);
			AddPartyMsgOnScreenForGWConf(partyId, cstr.GetString(), 5 * SECOND, FALSE, TRUE);
			GateWayPartyEndSetup(partyId);
		}
		else if (pConfParty->GetGatewayPartyType() > eNormalGWPartyType && m_initialSync)
		{
			// the call initiator is connected and synced
			// start dial out for all other parties and display messages
			TRACEINTO << msg.str().c_str() << " - Start dial-out loop for GateWay";
			StartDialOutLoopForGw(pConfParty->GetPartyId());
			SetPartyAsLeader(pConfParty->GetName(), eOn);
		}
	}
	else
	{
		if (!isPartyConnected)
			TRACEINTO << msg.str().c_str() << " - Party is not connected yet in DB, do nothing";

		TRACEINTO << msg.str().c_str();
	}
}

//--------------------------------------------------------------------------

void CConf::OnDtmfInvitedPartyConnected(CConfParty* pConfParty)
{
	BYTE IsPartyConnected = FALSE;
	BYTE partyState = pConfParty->GetPartyState();
	if (partyState == PARTY_CONNECTED || partyState == PARTY_SECONDARY || partyState == PARTY_CONNECTED_WITH_PROBLEM)
		IsPartyConnected = TRUE;

	std::ostringstream msg;

	PartyMonitorID partyId = pConfParty->GetPartyId();
	if (IsInvitedPartyInSetupStage(partyId, msg) && IsPartyConnected)
	{
		TRACEINTO << msg.str().c_str();

		// dial out party connected - send text message to inviter
		if (pConfParty->GetConnectionType() == DIAL_OUT)
		{
			DWORD invitorId = m_PartiesInvitorId[partyId];
			m_PartiesRedialNum.erase(invitorId);
			m_PartiesInviteType.erase(invitorId);

			CSmallString cstr;
			GenerateGeneralConnectedMessage(pConfParty, cstr);
			AddPartyMsgOnScreenForInvitedConf(partyId, cstr.GetString(), 5 * SECOND, FALSE, TRUE);
			SetVisualNameForInvitedParty(pConfParty);
			InvitedPartyEndSetup(partyId);
		}
	}
	else
	{
		if (!IsPartyConnected)
			TRACEINTO << msg.str().c_str() << "\nParty is not connected yet in DB, do nothing";
		else
			TRACEINTO << msg.str().c_str();
	}
}

//--------------------------------------------------------------------------
void CConf::OnInvitedPartyConnected(CConfParty* pConfParty)
{
	CMedString cMedstr;
	cMedstr << "CConf::OnInvitedPartyConnected, party: " << pConfParty->GetName();

	DWORD partyId = pConfParty->GetPartyId();
	if (m_pInvitedDialingSequence->IsMemberInSequence(partyId))
	{
		m_pInvitedDialingSequence->RemoveMapEntry(partyId);
		RespondPCMManagerInviteSucceed();
	}
	else if (strstr(pConfParty->GetAdditionalInfo(), "InvitedByPcm") ||
			strstr(pConfParty->GetAdditionalInfo(), DTMF_INVITE_PARTY_NAME_SUFFIX))
	{
		cMedstr << " party was invited from Address book";
		pConfParty->SetAdditionalInfo("");
		RespondPCMManagerInviteSucceed();
	}
	else
	{
		cMedstr << " party is not member in sequence!!!";
	}

	PTRACE(eLevelInfoNormal,cMedstr.GetString());
}
//LEGACY
/////////////////////////////////////////////////////////////////////
void CConf::InformVideoBridgeStartContentPresentation(DWORD activeProtocol,DWORD activeContentRate)
{
	CSegment*  seg = new CSegment;
	BYTE legacyContentHD1080Supported = 0; // non-zero values: e_res_1080_15fps, e_res_1080_30fps, e_res_1080_60fps

	if (!IsFeatureSupportedBySystem(eFeatureLegacyContentHD1080))
	{
		TRACEINTO << "Legacy content HD1080 not supported : feature not supported by system card\n";
		legacyContentHD1080Supported = FALSE;
	}
	else
	{
		legacyContentHD1080Supported = GetContentHD1080SupportedByConfSettings(0);
	}

	*seg << (OPCODE) CONTENT_BRIDGE_START_PRESENTATION;
	*seg << activeProtocol << activeContentRate << legacyContentHD1080Supported;

	m_pVideoBridgeInterface->HandleEvent(seg);
	POBJDELETE(seg);
}

////////////////////////////////////////////////////////////////////
void CConf::InformVideoBridgeStopContentPresentation()
{
	CSegment*  seg = new CSegment;

	*seg << (OPCODE) CONTENT_BRIDGE_STOP_PRESENTATION;

	m_pVideoBridgeInterface->HandleEvent(seg);
	POBJDELETE(seg);
}

////////////////////////////////////////////////////////////////////
void CConf::OnVideoBridgeContentDecoderAllocFail()
{
	UpdateConfStatus(CONFERENCE_CONTENT_RESOURCES_DEFICIENCY,YES);
}

////////////////////////////////////////////////////////////////////
void CConf::OnVideoBridgeContentDecoderResetFailStatus()
{
	UpdateConfStatus(CONFERENCE_CONTENT_RESOURCES_DEFICIENCY,NO);
}

//--------------------------------------------------------------------------
void CConf::OnSendStartPreviewReqToParty(CSegment* pParam)
{
	CIstrStream istream(*pParam);

	CPartyPreviewDrv partyPreviewReq;
	partyPreviewReq.DeSerialize(NATIVE, istream);

	DWORD PartyId = partyPreviewReq.GetPartyID();
	TRACEINTO << "PartyId:" << PartyId;

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(PartyId);
	TRACECOND_AND_RETURN(!pConfParty, "CConf::OnSendStartPreviewReqToParty - Failed, 'pConfParty' is not valid");

	CPartyConnection* pPartyConnection = GetPartyConnection(pConfParty->GetName());
	TRACECOND_AND_RETURN(!pPartyConnection, "CConf::OnSendStartPreviewReqToParty - Failed, 'pPartyConnection' is not valid");

	TRACECOND_AND_RETURN(pPartyConnection->IsDisconnect(), "CConf::OnSendStartPreviewReqToParty - Failed, Party in disconnected state");
	TRACECOND_AND_RETURN(pPartyConnection->IsDisconnectDelay(), "CConf::OnSendStartPreviewReqToParty - Failed, Party in delay disconnected state");
	TRACECOND_AND_RETURN(ISDN_INTERFACE_TYPE == pConfParty->GetNetInterfaceType(), "CConf::OnSendStartPreviewReqToParty - Failed, Video preview disabled for PSTN/ISDN party");
	TRACECOND_AND_RETURN(pConfParty->GetPartyState() == PARTY_STAND_BY, "CConf::OnSendStartPreviewReqToParty - Failed, Party in standby state");
	TRACECOND_AND_RETURN(pConfParty->GetPartyState() == PARTY_REDIALING, "CConf::OnSendStartPreviewReqToParty - Failed, Party in re-dialing state");
	TRACECOND_AND_RETURN(pPartyConnection->GetConnectionDelayTime() > 0, "CConf::OnSendStartPreviewReqToParty - Failed, Party in delay connection state");
	TRACECOND_AND_RETURN(pPartyConnection->GetPartyControlState() == ALLOCATE_RSC, "CConf::OnSendStartPreviewReqToParty - Failed, Party in allocate resources state");
	TRACECOND_AND_RETURN(pPartyConnection->IsTaskAppIsValid() == FALSE, "CConf::OnSendStartPreviewReqToParty - Failed, 'm_party' is not valid");

	CTaskApp* pPartyObject = pPartyConnection->GetPartyTaskApp();
	TRACECOND_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyObject), "CConf::OnSendStartPreviewReqToParty - Failed, Party task is not valid");

	PASSERTMSG_AND_RETURN(!(&(pPartyObject->GetRcvMbx())), "CConf::OnSendStartPreviewReqToParty - Failed, 'RcvMbx' is not valid");

	CPartyApi partyApi;
	partyApi.CreateOnlyApi(pPartyObject->GetRcvMbx());
	partyApi.SendPartyStartPreviewReq(&partyPreviewReq);
}

//--------------------------------------------------------------------------
void CConf::OnSendStopPreviewReqToParty(CSegment* pParam)
{
	char partyName[H243_NAME_LEN];
	WORD Direction = 0;

	*pParam >> partyName >> Direction;

	TRACEINTO << "PartyName:" << partyName;

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyName);
	TRACECOND_AND_RETURN(!pConfParty, "CConf::OnSendStopPreviewReqToParty - Failed, 'pConfParty' is not valid");

	CPartyConnection* pPartyConnection = GetPartyConnection(partyName);
	TRACECOND_AND_RETURN(!pPartyConnection, "CConf::OnSendStopPreviewReqToParty - Failed, 'pPartyConnection' is not valid");

	TRACECOND_AND_RETURN(pPartyConnection->IsDisconnect(), "CConf::OnSendStopPreviewReqToParty - Failed, Party in disconnected state");
	TRACECOND_AND_RETURN(pPartyConnection->IsDisconnectDelay(), "CConf::OnSendStopPreviewReqToParty - Failed, Party in delay disconnected state");
	TRACECOND_AND_RETURN(ISDN_INTERFACE_TYPE == pConfParty->GetNetInterfaceType(), "CConf::OnSendStopPreviewReqToParty - Failed, Video preview disabled for PSTN/ISDN party");
	TRACECOND_AND_RETURN(pConfParty->GetPartyState() == PARTY_STAND_BY, "CConf::OnSendStopPreviewReqToParty - Failed, Party in standby state");
	TRACECOND_AND_RETURN(pConfParty->GetPartyState() == PARTY_REDIALING, "CConf::OnSendStopPreviewReqToParty - Failed, Party in re-dialing state");
	TRACECOND_AND_RETURN(pPartyConnection->GetConnectionDelayTime() > 0, "CConf::OnSendStopPreviewReqToParty - Failed, Party in delay connection state");
	TRACECOND_AND_RETURN(pPartyConnection->GetPartyControlState() == ALLOCATE_RSC, "CConf::OnSendStopPreviewReqToParty - Failed, Party in allocate resources state");
	TRACECOND_AND_RETURN(pPartyConnection->IsTaskAppIsValid() == FALSE, "CConf::OnSendStopPreviewReqToParty - Failed, 'm_party' is not valid");

	CTaskApp* pPartyObject = pPartyConnection->GetPartyTaskApp();
	TRACECOND_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyObject), "CConf::OnSendStopPreviewReqToParty - Failed, Party task is not valid");

	PASSERTMSG_AND_RETURN(!(&(pPartyObject->GetRcvMbx())), "CConf::OnSendStopPreviewReqToParty - Failed, 'RcvMbx' is not valid");

	CPartyApi partyApi;
	partyApi.CreateOnlyApi(pPartyObject->GetRcvMbx());
	partyApi.SendPartyStopPreviewReq(Direction);
}

////////////////////////////////////////////////////////////////////
void CConf::OnMcuIntraPreviewReq(CSegment *pParam)
{
	CPartyApi partyApi;
	char	partyName[H243_NAME_LEN];

	WORD Direction = 0;

	*pParam >> partyName>>Direction;

	PTRACE2(eLevelError,"CConf::OnMcuIntraPreviewReq name:  ",partyName);

	CTaskApp* pPartyObject = NULL;
	CPartyConnection*  pPartyConnection = GetPartyConnection(partyName);
	if( !CPObject::IsValidPObjectPtr(pPartyConnection))
	{
	    PTRACE2(eLevelError,"CConf::OnMcuIntraPreviewReq : party name was not found : ",partyName);
	    return;
	}

	//Do not send message when party is disconnected
	if (YES == pPartyConnection->IsDisconnect() || pPartyConnection->IsDisconnectDelay())
	{
		PTRACE(eLevelInfoNormal,"CConf::OnMcuIntraPreviewReq - IsDisconnect or IsDisconnectDelay");
		return;
	}

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyName);
	if ( !pConfParty )
	{
	    PTRACE(eLevelInfoNormal,"CConf::OnMcuIntraPreviewReq - pConfParty == NULL");
	    return;
	}

	if (ISDN_INTERFACE_TYPE == pConfParty->GetNetInterfaceType())
	{
	    TRACESTR (eLevelInfoNormal) << "CConf::OnMcuIntraPreviewReq PSTN/ISDN party- no video preview" ;
	    return;
	}

	if(pConfParty->GetPartyState() == PARTY_STAND_BY)// The party in stand_by.
	{
	    PTRACE2(eLevelError,"CConf::OnMcuIntraPreviewReq : party still in standby state : ",partyName);
	    return;
	}

	if (pConfParty->GetPartyState() == PARTY_REDIALING)
	{
	   PTRACE2(eLevelError,"CConf::OnMcuIntraPreviewReq : party is in redialing state : ",partyName);
	   return;
	}

	if (pPartyConnection->GetConnectionDelayTime() > 0)
	{
	    PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : party is in delay connection state : ",partyName);
	    return;
    }

	if (pPartyConnection->GetPartyControlState() == ALLOCATE_RSC)
	{
	    PTRACE2(eLevelError,"CConf::OnMcuIntraPreviewReq : party is in Allocate resource state : ",partyName);
	    return;
	}

	pPartyObject = pPartyConnection->GetPartyTaskApp();
	if(pPartyConnection->IsTaskAppIsValid() == FALSE)
	{
	   PTRACE2(eLevelError,"CConf::OnSendIpMonitorReqToParty : m_party is not valid : ",partyName);
	   return;
	}

	if( !CPObject::IsValidPObjectPtr(pPartyObject))
	{
	    PTRACE2(eLevelError,"CConf::OnMcuIntraPreviewReq : party is not valid : ",partyName);
	    return;
	}

	if( (&(pPartyObject->GetRcvMbx()))==NULL)
	{
		PTRACE2(eLevelError,"CConf::OnMcuIntraPreviewReq : party's RcvMbx is NULL : ",partyName);
	    DBGPASSERT((DWORD)pPartyObject);
	    return;
	}

	partyApi.CreateOnlyApi(pPartyObject->GetRcvMbx());
	partyApi.SendPartyIntraPreviewReq(Direction);
}


void CConf::GatheringUpdateTimerHandler(CSegment* pParam)
{
	if (m_pGatheringManager)
	{
		m_pGatheringManager->OnUpdateByTimer();
	}
}

void CConf::GatheringConnectingTimerHandler(CSegment* pParam)
{
	if (m_pGatheringManager)
	{
		m_pGatheringManager->OnConnectingTimer();
	}
}

void CConf::OnGatheringPartyConnected(CSegment* pParam)
{
	if (m_pGatheringManager)
	{
		if (m_pGatheringManager->IsGatheringEnabled())
		{
			CTaskApp*  pParty;
			*pParam >> (DWORD&)pParty;
			CPartyConnection*  pPartyConnection = NULL;
			if ( (pPartyConnection = GetPartyConnection(pParty)) != NULL )
			{
				CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
				if (pConfParty)
				{
					m_pGatheringManager->OnPartyConnecting(pConfParty);
				}
			}
		}
	}
}

void CConf::StartGatheringUpdateTimer(unsigned int iDelay /*= GATHERING_UPDATE_TIMER_5_SEC*/)
{
	StartTimer(GATHERING_UPDATE_TIMER, iDelay);
}

void CConf::StartGatheringPartyConnectingTimer()
{
	StartTimer(GATHERING_PARTY_CONNECTING_TIMER, GATHERING_CONNECTING_TIMER);
}

int	CConf::GetExceptionPartList(std::set<std::string>& setParts)
{
	int n = 0;
	if (m_pGatheringManager)
		n = m_pGatheringManager->GetExceptionPartList(setParts);

	return n;
}

CGathering* CConf::GetGathering(const char* pszPartyName) const
{
	if (m_pGatheringManager)
		  return m_pGatheringManager->GetGathering(pszPartyName);
	return NULL;
}

CVideoLayout* CConf::GetGatheringLayout(CConfParty* pConfParty /*= NULL*/)
{
	if (m_pGatheringManager)
		return m_pGatheringManager->GetGatheringLayout(pConfParty);
	return NULL;
}

CVisualEffectsParams* CConf::GetGatheringVisualEffects()
{
	if (m_pGatheringManager && m_pGatheringManager->IsGatheringEnabled())
		return m_pGatheringManager->GetGatheringVisualEffects();
	return NULL;
}

void CConf::StopGathering(const char* pszPartyName)
{
	if (m_pGatheringManager && m_pGatheringManager->IsGatheringEnabled())
		m_pGatheringManager->StopGathering(pszPartyName);
}

//added for BRIDGE-13167
void CConf::OnPartyDisConnected(CConfParty* pConfParty)
{
	if (m_pGatheringManager && m_pGatheringManager->IsGatheringEnabled())
		m_pGatheringManager->OnPartyDisConnected(pConfParty);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnIVRPartyPassedEntranceProcedCONNECT(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CConf::OnIVRPartyPassedEntranceProcedCONNECT, Name - ", m_name);
	if (m_isGateWay || m_isDtmfInviteParty)
	{
		char party_name[H243_NAME_LEN];
		*pParam >> party_name;

		CConfParty* pConfParty = NULL;
		pConfParty = m_pCommConf->GetCurrentParty(party_name);
		PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pConfParty));

		if (pConfParty->GetVoice())
		{
			if (m_isGateWay)
			{
				if (!m_initialSync && pConfParty->GetGatewayPartyType() > eNormalGWPartyType)
				{
					ON(m_initialSync);

					// BRIDGE-13471 - check if GW profile has the option of PSTN checked in "GW dial-out protocols"
                    BOOL isPstnAllowed = m_pCommConf->GetIsGWDialOutToPSTN();

					PTRACE2(eLevelInfoNormal, "CConf::OnIVRPartyPassedEntranceProcedCONNECT, Call initiator is Audio only, making all dial out parties Audio only as well, Name - ", m_name);
					PASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(m_pDialingSequence));
					m_pDialingSequence->SetVoice(YES, isPstnAllowed);
				}

				OnGateWayPartyConnected(pConfParty);
			}

			if (m_isDtmfInviteParty)
			{
				if (!m_initialSync && pConfParty->GetGatewayPartyType() > eNormalGWPartyType)
				{
					ON(m_initialSync);

					PTRACE2(eLevelInfoNormal, "CConf::OnIVRPartyPassedEntranceProcedCONNECT, Call initiator is Audio only, making all dial out parties Audio only as well, Name - ", m_name);
					m_pDtmfInvitedPartyDialingSequence->SetVoice(YES);
				}

				OnDtmfInvitedPartyConnected(pConfParty);
			}
		}
	}

	if (m_pCommConf->IsMeetingRoom() && m_pCommConf->IsDefinedIVRService() && !m_pCommConf->IsMeetingRoomUp())
	{
		m_StandByStart = m_pCommConf->IsStandBy();
		m_pCommConf->SetMeetingRoomUp();

		CConfApi confApi;                   //Create  confApi object to the conference
		confApi.CreateOnlyApi(GetRcvMbx()); //Get the mailbox ID of the conference task

		PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
		for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			if (pPartyConnection)
			{
				if (pPartyConnection->IsDisconnectState() && pPartyConnection->GetDialType() == DIALOUT &&(!m_StandByStart))
				{
					confApi.ReconnectParty(pPartyConnection->GetName(), 0);
				}
			}
		}
		confApi.DestroyOnlyApi();           //Release the confApi object we created
	}
}

//=====================================================================================================================================//
void CConf::OnExclusiveContentSet(CSegment* pParam)//Restricted content
{
  char partyName[H243_NAME_LEN];

  *pParam >> partyName;

  PTRACE2(eLevelInfoNormal,"CConf::OnRestrictContentSet - partyName: ", partyName);

  CPartyConnection* pPartyConnection = GetPartyConnection(partyName);

  if (pPartyConnection == NULL)
  {
	  PTRACE2(eLevelInfoNormal,"CConf::OnExclusiveContentSet: faild to get pPartyConnection, ",m_name);
  } else {
	  UpdateDB(pPartyConnection->GetPartyTaskApp(), UPDATE_EXCLUSIVE_CONTENT, TRUE);
	  m_pContentBridge->UpdateExclusiveContent();
  }
}
//=====================================================================================================================================//
void CConf::OnRemoveExclusiveContent(CSegment* pParam)//Restricted content
{
  PTRACE(eLevelInfoNormal,"CConf::OnRemoveRestrictContent - partyName: <any Party that has an exclusive content>");

  UpdateDB((CTaskApp*)0xffff, UPDATE_EXCLUSIVE_CONTENT, FALSE);
}
/////////////////////////////////////////////////////////////////////////////
void CConf::OnExclusiveContentModeSet(CSegment* pParam)//Restricted content
{
    BOOL isExclusiveContentMode;

    *pParam	>> isExclusiveContentMode;

    PTRACE2INT(eLevelInfoNormal,"CConf::OnExclusiveContentModeSet:: new setting is ",isExclusiveContentMode);
    m_pCommConf->SetExclusiveContentMode(isExclusiveContentMode);

    UpdateDB((CTaskApp*)0xffff,UPDATE_EXCLUSIVE_CONTENT_MODE,isExclusiveContentMode);
    m_pContentBridge->UpdateExclusiveContentMode(isExclusiveContentMode);

}
/////////////////////////////////////////////////////////////////////////////
void CConf::OnConfAvcSvcModeSet(CSegment* pParam)
{
	ConfMonitorID confId = 0;
	DWORD ConfMediaState = 0;

	*pParam >> ConfMediaState >> confId;

	SetConfAvcSvcMode((eConfMediaState)ConfMediaState, confId);
}

//--------------------------------------------------------------------------
void CConf::SetConfAvcSvcMode(eConfMediaState confMediaState, ConfMonitorID confId)
{
	eConfMediaState currentMediaState = GetMediaState();

	TRACEINTO << "MonitorConfId:" << confId << ", CurrMediaState:" << MediaStateToString(currentMediaState) << ", ConfMediaState: " << MediaStateToString(confMediaState);

	if ((confMediaState == eMediaStateMixAvcSvc) && (currentMediaState == eMediaStateAvcOnly || currentMediaState == eMediaStateSvcOnly || currentMediaState == eMediaStateEmpty))
	{
		PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
		for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
		{
			CPartyConnection* pPartyConnection = _itr->second;
			if (pPartyConnection)
			{
				CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
				if (!pPartyCntl)
				{
					PASSERTSTREAM(1, "MonitorPartyId:" << pPartyConnection->GetMonitorPartyId() << " - Failed, party does not exist");
					continue;
				}
				pPartyCntl->UpdatePartyAvcSvcMode(confMediaState, confId, pPartyConnection->GetMonitorPartyId());
			}
		}
		UpdateConfMediaState(eMediaStateMixAvcSvc);
		return;
	}

	//Down-grade
	if (currentMediaState == eMediaStateMixAvcSvc)
	{
		TRACEINTO << "Conference is already in 'eMediaStateMixAvcSvc' state, down-grade not supported";
	}
}

//--------------------------------------------------------------------------
ePMediaType CConf::ConvertConfMediaTypeToPartyType(eConfMediaState confMediaState)
{
	switch(confMediaState)
	{
		case eMediaStateAvcOnly: return eAvcPartyType;
		case eMediaStateSvcOnly: return eSvcPartyType;
		default:
			// Note: some enumeration value are not handled in switch. Add default to suppress warning.
			break;
	}
	return ePartyMediaType_dummy;
}
//////////////////////////////////////////////////////////////////////////////
void CConf::OnPartyAvcSvcModeSet(CSegment* pParam)
{

	DWORD partyMonitorId = (DWORD)(-1);
	DWORD confID = (DWORD)(-1);
	eConfMediaState confMediaState = eMediaStateEmpty;
	DWORD ConfMediaState=(DWORD)confMediaState;
	const char* partyName;
	*pParam >> ConfMediaState  >> confID  >> partyMonitorId;

	TRACEINTO << "ConfId:" << confID << ", confMediaState:" << MediaStateToString((eConfMediaState)ConfMediaState) << ", MonitorPartyId:" << partyMonitorId;

	CPartyConnection*  pPartyConnection=NULL;
	partyName=::GetpConfDB()->GetPartyName(confID,partyMonitorId);
	pPartyConnection = GetPartyConnection(partyName);

	PASSERTSTREAM_AND_RETURN(!pPartyConnection, "PartyName:" << partyName << " - Party name was not found");

	CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();

	PASSERTSTREAM_AND_RETURN(!pPartyCntl, "MonitorPartyId:" << partyMonitorId << "- Failed, invalid value of pPartyCntl");

	pPartyCntl->UpdatePartyAvcSvcMode((eConfMediaState)ConfMediaState, confID , partyMonitorId);

}
//////////////////////////////////////////////////////////////////////////////////////
void CConf::OnMuteIncomingLectureModeSet(CSegment* pParam)//Restricted content
{
    BOOL isMuteIncomingLectureMode;

    *pParam	>> isMuteIncomingLectureMode;

    PTRACE2INT(eLevelInfoNormal,"CConf::OnMuteIncomingLectureModeSet:: new setting is ",isMuteIncomingLectureMode);

    UpdateDB((CTaskApp*)0xffff,UPDATE_MUTE_INCOMING_LECTURE_MODE,isMuteIncomingLectureMode);
    m_pConfAppMngrInterface->SetMuteIncomingLectureMode(isMuteIncomingLectureMode);

}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::RespondPCMManagerInviteFailed(CSegment* pParam)
{
	if (m_pMcmsPCMManager)
		m_pMcmsPCMManager->OnConfInviteResult(false);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::RespondPCMManagerInviteSucceed(CSegment* pParam)
{
	if (m_pMcmsPCMManager)
		m_pMcmsPCMManager->OnConfInviteResult(true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnPCMInviteParty(CSegment* pParam)
{
	CLargeString cstr;
	cstr << "CConf::OnPCMInviteParty ";

	WORD dialStringLen = 0;
	WORD interfaceType = 0;
	char dialString[MaxAddressListSize];
	memset(dialString,'\0',MaxAddressListSize);

	STATUS status = STATUS_OK;
	*pParam >> dialStringLen;
	if (!dialStringLen)
	{
		cstr << "dial string is empty - disconnect!";
		PTRACE(eLevelInfoNormal,cstr.GetString());
		RespondPCMManagerInviteFailed((CSegment*)NULL);
		//PASSERT_AND_RETURN(1);
	}
	else
	{
		*pParam >> dialString;
		cstr << "dial string is: " << dialString << "\n";

		*pParam >> interfaceType;

		DWORD party_id = InsertInvitedPartyToSequence(dialString,interfaceType);

		if (party_id == 0xFFFFFFFF)
		{
			cstr << "InsertInvitedPartyToSequence returned with status fail";
			PTRACE(eLevelInfoNormal,cstr.GetString());
			RespondPCMManagerInviteFailed((CSegment*)NULL);
		}

		cstr << "m_pInvitedDialingSequence is: \n";
		m_pInvitedDialingSequence->DumpToTrace(cstr);

		PTRACE(eLevelInfoNormal,cstr.GetString());

		AddNextInvitedParty(party_id);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetVisualNameForInvitedParty(CConfParty* pConfParty)
{
	PTRACE(eLevelInfoNormal,"CConf::SetVisualNameForInvitedParty");

	ALLOCBUFFER(visualName,H243_NAME_LEN);
	memset(visualName,'\0',H243_NAME_LEN);
	char* pReadPtr = (char*)pConfParty->GetName();
	char* pReadPtr1 = strstr(pReadPtr,"_InvitedByPcm" );
	if(pReadPtr1 == NULL)
	{
		pReadPtr1 = strstr(pReadPtr,"_Invited" );
	}

	if (pReadPtr1 )
	{
		int len = pReadPtr1 - pReadPtr ;
		strncpy(visualName,pReadPtr,len);
		visualName[len] = '\0';
		if (strchr(visualName,'_') != NULL)
			ReplaceChar(visualName, '_', '.');

		pConfParty->SetVisualPartyName(visualName);
		PTRACE2(eLevelInfoNormal,"CConf::SetVisualNameForInvitedParty visual name: ",visualName);
    }

	DEALLOCBUFFER(visualName);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnTerminalRecurrentIntraReqTout(CSegment* pParam)
{
	const WORD MAX_COMMAND_LEN = 80;

	char  partyName[MAX_COMMAND_LEN];
	WORD  req          = 0;
	DWORD timeInterval = 0;
	DWORD numRequests  = 0;
	DWORD channelId    = 1; // video

	*pParam >> partyName >> timeInterval >> numRequests >> channelId;

	TRACEINTO
		<< "ConfName:"       << m_name
		<< ", PartyName:"    << partyName
		<< ", Request:"      << req
		<< ", timeInterval:" << timeInterval
		<< ", numRequests:"  << numRequests
		<< ", channelId:"    << channelId;

	CPartyConnection* pPartyConnection = GetPartyConnection(partyName);
	TRACECOND_AND_RETURN(!pPartyConnection, "Failed, invalid pointer");

	TRACECOND_AND_RETURN(pPartyConnection->IsDisconnect(), "Failed, party is disconnected");

	CTaskApp* pParty = pPartyConnection->GetPartyTaskApp();
	TRACECOND_AND_RETURN(!pParty, "Failed, invalid pointer");

	CConfApi* pConfApi = new CConfApi;
	pConfApi->CreateOnlyApi(GetRcvMbx());

	if (channelId == 1) // video intra
	{
		TRACEINTO << "PartyId:" << pParty->GetPartyId() << ", timeInterval:" << timeInterval << ", numRequests:" << numRequests << " - Send recurrent Intra request from EP to Video Bridge";
		pConfApi->VideoRefresh(pParty->GetPartyId());
	}
	else  // content intra
	{
		TRACEINTO << "PartyId:" << pParty->GetPartyId() << ", timeInterval:" << timeInterval << ", numRequests:" << numRequests << " - Send recurrent Intra request from EP to Content Bridge";
		pConfApi->ContentVideoRefresh(1, YES, pParty);
	}

	numRequests = numRequests-1;
	if (numRequests > 0)
	{
		CSegment* pSeg = new CSegment; const WORD MAX_COMMAND_LEN = 80;

		*pSeg << partyName << (DWORD)timeInterval << (DWORD)numRequests << (DWORD)channelId;
		StartTimer(TERMINAL_RECURRENT_INTRA_REQ_TOUT, timeInterval*SECOND, pSeg);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetCopCascadeLinkAsLecturer(CTaskApp* pParty)
{
	CPartyConnection* pPartyConnection = GetPartyConnection(pParty);
	PASSERT_AND_RETURN(!pPartyConnection);

	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
	PASSERT_AND_RETURN(!pConfParty);

	WORD confType = GetVideoSession();
	// Update General Video related params
	if (confType == VIDEO_SESSION_COP)
	{
		CLectureModeParams* currentLectureModeParams = m_pCommConf->GetLectureMode();
		if (currentLectureModeParams == NULL)
		{
			PTRACE2(eLevelInfoNormal, "CConf::SetCopCascadeLinkAsLecturer: faild to get currentLectureModeParams, ", m_name);
			return;
		}

		const char* lecturerName = currentLectureModeParams->GetLecturerName();
		WORD len1 = strlen(lecturerName);
		WORD len2 = strlen(pConfParty->GetName());
		if (len2 == len1)
		{
			if (!strncmp(lecturerName, pConfParty->GetName(), len1))
			{
				PTRACE2(eLevelInfoNormal, "CConf::SetCopCascadeLinkAsLecturer: lecturer already updated, ", m_name);
				return;
			}
		}

		if (pConfParty->GetCascadeMode() == CASCADE_MODE_SLAVE && !pConfParty->GetVoice())
		{
			PTRACE2(eLevelInfoNormal, "CConf::SetCopCascadeLinkAsLecturer: updateding party to lecturer, ", m_name);
			// set db
			m_pCommConf->UpdateLectureModeAndLayoutBecauseSlaveInConf(pConfParty->GetName());
			// update conf from db
			CConfApi selfApi;
			selfApi.CreateOnlyApi(GetRcvMbx(), this);
			selfApi.SetVideoConfLayoutSeeMeAll(*m_pCommConf->GetVideoLayout());
			selfApi.UpdateLectureMode(m_pCommConf->GetLectureMode());
			selfApi.DestroyOnlyApi();
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::SetLastRateFromMaster(const CTaskApp* pParty)
{
	CPartyCntl* pPartyCntl = GetPartyCntl(pParty);

	if (pPartyCntl && pPartyCntl->GetInterfaceType() != ISDN_INTERFACE_TYPE)
	{
		DWORD  partyConfRate = 0;

		partyConfRate = ((CH323PartyCntl*)pPartyCntl)->GetMinContentPartyRate();
		m_lastContentRateFromMaster = (partyConfRate/100);

		PTRACE2INT(eLevelInfoNormal,"CConf::SetLastRateFromMaster : Min content rate -", partyConfRate);
	}
}
/////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: OnAddSubLinksPartiesConnect  shiraITP - 22
void CConf::OnAddSubLinksPartiesConnect(CSegment* pParam)
{
	DWORD room_Id = 0;
	DWORD cascadedLinksNumber = 1;
	char partyName[H243_NAME_LEN];
	memset(partyName, 0, sizeof(partyName));

	*pParam >> cascadedLinksNumber >> room_Id >> partyName;

	TRACEINTO << "PartyName:" << partyName << ", RoomId:" << room_Id << ", CascadedLinksNumber:" << cascadedLinksNumber;

	if (m_pCommConf == NULL)
	{
		TRACESTRFUNC(eLevelError) << "Failed, m_pCommConf is NULL, no subs will be created";
		return;
	}

	CConfParty* pConfPartyOfMain = m_pCommConf->GetCurrentParty(partyName);

	if (pConfPartyOfMain)
	{
		for (DWORD i = 0; i < cascadedLinksNumber-1; i++)
		{
			CConfParty* pConfPartyNewSub = new CConfParty(*pConfPartyOfMain);

			DWORD nextPartyId = m_pCommConf->NextPartyId();

			if (pConfPartyNewSub)
			{
				pConfPartyNewSub->SetPartyId(nextPartyId);

				pConfPartyNewSub->SetPartyType(eSubLinkParty);
				pConfPartyNewSub->SetRoomId(pConfPartyOfMain->GetRoomId()); // pConfPartyOfMain->GetRoomId() is same as room_Id - we can remove room_Id from this msg..

				// change name:
				char subPartyName[H243_NAME_LEN];
				::GetSubLinkName(partyName, i+2, (char*)subPartyName);

				TRACEINTO << "SubLink:" << subPartyName;

				pConfPartyNewSub->SetName(subPartyName);
				pConfPartyNewSub->SetVisualPartyName(subPartyName);

				// mute the audio transmit:
				pConfPartyNewSub->SetAudioBlocked(TRUE);

				WORD addStatus = m_pCommConf->Add(*pConfPartyNewSub);

				if (addStatus)
				{
					PASSERTSTREAM(addStatus, "PartyName:" << pConfPartyNewSub->GetName() << " - Failed to add sub-party to DB");
					POBJDELETE(pConfPartyNewSub);
					return;
				}

				DWORD connectionDelay = 0;

				connectionDelay = ComputeConnectingConnectionDelay(pConfPartyNewSub->GetVoice(), pConfPartyNewSub->GetName(), pConfPartyNewSub->GetNetInterfaceType());
				ConnectIpParty(pConfPartyNewSub, '\0', 0, connectionDelay, room_Id, eSubLinkParty);
			}
			else
			{
				TRACESTRFUNC(eLevelError) << "Failed, pConfPartyNewSub is NULL, no subs will be created";
			}

			POBJDELETE(pConfPartyNewSub);
		}
	}
	else
	{
		TRACESTRFUNC(eLevelError) << "Failed, pConfPartyOfMain is NULL, no subs will be created";
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::OnAddSlavePartyConnect(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CConf::OnAddSlavePartyConnect ");

	DWORD listId 				= 0;
	DWORD tipPartyType 			= 0;
	DWORD room_Id 				= 0;
	DWORD peerPartyRsrcId 		= 0;
	DWORD masterVideoPartyType 	= 0;

    CIpComMode *pIpMasterInitialMode = new CIpComMode;
    pIpMasterInitialMode->DeSerialize(NATIVE, *pParam);

    *pParam >> listId
			>> tipPartyType
			>> room_Id
			>> peerPartyRsrcId
			>> masterVideoPartyType;


    CRsrvParty* pRsrvParty = new CRsrvParty;
    pRsrvParty->DeSerialize(NATIVE, *pParam);
    char* name = (char*)pRsrvParty->GetName();

	CLargeString cstr;
	cstr << "CConf::OnAddSlavePartyConnect partyType " << tipPartyType << " room_Id " << room_Id;
	PTRACE(eLevelInfoNormal, cstr.GetString());


	CConfParty* pConfParty = new CConfParty(*(CRsrvParty*)pRsrvParty);
	pConfParty->SetLobbyId((DWORD)listId);

	//check consistancy between conf reservation and auto party add

	CCommConfDB* p = ::GetpConfDB();
	WORD addStatus = m_pCommConf->Add(*pConfParty);

	POBJDELETE(pRsrvParty);

  if (addStatus)
  {
		PASSERT(addStatus);
		ALLOCBUFFER(buf,H243_NAME_LEN*4);
	    sprintf(buf, "CConf::OnAddSlavePartyConnect : Failed to add party to DB, Conf Name: %s, Party Name - %s, Status: %d",m_name, pConfParty->GetName(),addStatus);
	    PTRACE2(eLevelInfoNormal," ---> ",buf);
	    DEALLOCBUFFER(buf);
		POBJDELETE(pConfParty);
		POBJDELETE(pIpMasterInitialMode);
		return;
	}

	DWORD connectionDelay = 0;

	CConfParty* pTempConfParty = m_pCommConf->GetCurrentParty(pConfParty->GetName());
	POBJDELETE(pConfParty);
	pConfParty = pTempConfParty;
	// Romem klocwork
	if(pConfParty == NULL)
	{
		CMedString msg1;
		msg1 << "Name - " << m_name << "," << name;
		PTRACE2(eLevelInfoNormal,"CConf::OnAddSlavePartyConnect : Party is not in Conf DB ", msg1.GetString());

		POBJDELETE(pIpMasterInitialMode);
		return;
	}

	connectionDelay = ComputeConnectingConnectionDelay(pConfParty->GetVoice(),pConfParty->GetName(),pConfParty->GetNetInterfaceType());
	ConnectSipSlaveParty(pConfParty, connectionDelay, tipPartyType, room_Id, peerPartyRsrcId, masterVideoPartyType, pIpMasterInitialMode);

	POBJDELETE(pIpMasterInitialMode);
}

//--------------------------------------------------------------------------
void CConf::OnAddMsSlavePartyConnect(CSegment* pParam)
{
	PartyRsrcID    mainPartyRsrcId  = 0;
	eAvMcuLinkType AvMcuLinkType    = eAvMcuLinkNone;
	DWORD          msSlaveIndex     = 0;
	DWORD          msSsrcRangeStart = 0;
	DWORD 		isLocalSdesCap = 0;

	*pParam
		>> mainPartyRsrcId
		>> (DWORD&)AvMcuLinkType
		>> msSlaveIndex
		>> msSsrcRangeStart
		>> isLocalSdesCap;

	TRACEINTO << "MainPartyId:" << mainPartyRsrcId << ", AvMcuLinkType:" << eAvMcuLinkTypeNames[AvMcuLinkType] << ", isLocalSdesCap" << isLocalSdesCap
			  << ", SlaveIndex:" << msSlaveIndex << ", SsrcRangeStart:" << msSsrcRangeStart << "pParam->GetLen():" << pParam->GetLen();

	//std::auto_ptr<CSipCaps> tmp_localCaps(new CSipCaps);
	CVidModeH323* tmp_localCaps = NULL; AUTO_DELETE(tmp_localCaps);
	if (isLocalSdesCap)
	{
		tmp_localCaps = new CVidModeH323;
		tmp_localCaps->DeSerialize(NATIVE, *pParam);
	}
	std::auto_ptr<CSipCaps> remoteCaps(new CSipCaps);
	remoteCaps->DeSerialize(NATIVE, *pParam);

	std::auto_ptr<CRsrvParty> pRsrvParty(new CRsrvParty);
	pRsrvParty->DeSerialize(NATIVE, *pParam);

	std::auto_ptr<CConfParty> pConfParty(new CConfParty(*pRsrvParty.get()));

	STATUS status = m_pCommConf->Add(*pConfParty);
	PASSERTSTREAM_AND_RETURN(status != STATUS_OK, "Status:" << status);

	CConfParty* pNewConfParty = m_pCommConf->GetCurrentParty(pConfParty->GetName());
	PASSERTSTREAM_AND_RETURN(!pNewConfParty, "PartyName:" << pConfParty->GetName() << " - Party is not in ConfDB");

	DWORD connectionDelay = 0;  // since we create only out slaves and not real participants - no need for connection delay.
	ConnectMsSlaveParty(pNewConfParty, connectionDelay, mainPartyRsrcId, AvMcuLinkType, msSlaveIndex, msSsrcRangeStart, remoteCaps.get(), tmp_localCaps);
	POBJDELETE(tmp_localCaps);
}

//--------------------------------------------------------------------------
void CConf::OnLobbyRelaseAvMcuPartyConnect(CSegment* pParam)
{
	PartyMonitorID partyId = 0;
	ConfMonitorID  confId  = m_pCommConf->GetMonitorConfId();
	DWORD len = 0;

	*pParam >> partyId >> len;

	TRACEINTO << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId;

	std::auto_ptr<CBridgePartyVideoParams> pBridgePartyVideoParams(new CBridgePartyVideoParams);

	BYTE* pSdpAndHeaders = new BYTE[len];
	PASSERT_AND_RETURN(!pSdpAndHeaders);

	pParam->Get(pSdpAndHeaders, len);

	std::auto_ptr<CSipNetSetup> pNetSetup(new CSipNetSetup);

	// update party ip address
	pNetSetup->DeSerialize(NATIVE, *pParam);

	TRACEINTO << "MsConversationId:" << pNetSetup->GetMsConversationId();

	CConfParty* pConfParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(confId, partyId);

	if(NULL == pConfParty){
		delete[] pSdpAndHeaders;
		pSdpAndHeaders = NULL;
		PASSERT(1);

		return;
	}

	pConfParty->SetConnectionType(DIAL_OUT);

	DWORD TmpPartyId = FindMatchingConversationId(pConfParty);
	if(TmpPartyId)
	{
		CConfParty* pTmpConfParty = (CConfParty*)::GetpConfDB()->GetCurrentParty(confId, TmpPartyId);

		if(!pTmpConfParty){
			delete[] pSdpAndHeaders;
			pSdpAndHeaders = NULL;
			PASSERT(1);

			return;
		}

		pConfParty->SetMsMediaEscalationStatus(eMsftEscalationActive);
		pTmpConfParty->SetMsMediaEscalationStatus(eMsftEscalationActive);

		PTRACE2(eLevelInfoNormal,"CConf::OnLobbyRelaseAvMcuPartyConnect : Name - ",m_name);


	}
	else
		pConfParty->SetMsMediaEscalationStatus(eMsftEscalationInActive);


	DWORD connectionDelay = ComputeConnectingConnectionDelay(pConfParty->GetVoice(), pConfParty->GetName(), pConfParty->GetNetInterfaceType());
	ConnectAVMCUParty(pNetSetup.get(), pConfParty, connectionDelay, (sipSdpAndHeaders*)pSdpAndHeaders);

	delete[] pSdpAndHeaders;
	pSdpAndHeaders = NULL;
}

///////////////////////////////////////////////////
PartyMonitorID CConf::FindMatchingConversationId(CConfParty* pAVMCUConfParty)
{
	PartyMonitorID PartyId = 0;

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pPartyConnection = _itr->second;
		if (!pPartyConnection)
			continue;

		CPartyCntl* pPartyCntl = pPartyConnection->GetPartyCntl();
		if (!pPartyCntl)
			continue;

		CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyCntl->GetMonitorPartyId());

		if (!pConfParty || !(pConfParty->GetPartyState() == PARTY_CONNECTED || pConfParty->GetPartyState() == PARTY_CONNECTED_PARTIALY || pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM || (pConfParty->GetPartyState() == PARTY_CONNECTING)))
		{
			TRACEINTO << "PartyName:" << pPartyCntl->GetName() << " - Party is not connected";
			continue;
		}

		if ((pPartyConnection->GetInterfaceType() == SIP_INTERFACE_TYPE))
		{
			if (strcmp(pAVMCUConfParty->GetMsConversationId(), pConfParty->GetMsConversationId()) == 0)
			{
				PartyId = pConfParty->GetPartyId();

				TRACEINTO << "PartyName:" << pPartyCntl->GetName() << ", MonitorPartyId:" << PartyId << " - Found matching conversation id";
			}
		}
	}
	return PartyId;
}

///////////////////////////////////////////////////

void  CConf::FallBackToRegularSip(CSegment* pParam)
{
	char partyName[H243_NAME_LEN] = "";
	*pParam >> partyName;

	TRACECOND_AND_RETURN(!m_pCommConf , "m_pCommConf is NULL");
	CConfParty* pConfParty = m_pCommConf->GetCurrentParty(partyName);
	TRACECOND_AND_RETURN(!pConfParty , "pConfParty is NULL");

	//get party connection and activate the fall back function
	CPartyConnection*  pPartyConnection = GetPartyConnection(partyName);
	PASSERTSTREAM_AND_RETURN(!pPartyConnection, "party name was not found : " << partyName);

	CIpComMode* pTempMode = new CIpComMode;
	pTempMode->DeSerialize(NATIVE, *pParam);

	PartyControlDataParameters partyControlDataParams;
    memset(&partyControlDataParams, 0, sizeof(PartyControlDataParameters));
	SetControlDataParams(partyControlDataParams, pConfParty,NULL,NULL,NULL,TRUE,FALSE,FALSE,Regular,No_Lync,0,eRegularParty);

	PartyControlInitParameters partyControlInitParam;
    memset(&partyControlInitParam, 0, sizeof(PartyControlInitParameters));
	SetControlInitParams(partyControlInitParam,pConfParty,FALSE,NULL,0,0,0,NULL);


	pPartyConnection->ChangeSipfromTipToNonTip(pTempMode,partyControlInitParam,partyControlDataParams);

	POBJDELETE(pTempMode);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CConf::FallBackFromIceToRegularSip(CSegment* pParam)
{
	char partyName[H243_NAME_LEN] = "";
	*pParam >> partyName;

	CConfParty* pConfParty	= m_pCommConf->GetCurrentParty(partyName);
	TRACECOND_AND_RETURN(!pConfParty , "pConfParty is NULL");

	//get party connection and activate the fall back function
	CPartyConnection*  pPartyConnection = GetPartyConnection(partyName);
	PASSERTSTREAM_AND_RETURN(!pPartyConnection, "party name was not found : " << partyName);

	PartyControlDataParameters partyControlDataParams;
    memset(&partyControlDataParams, 0, sizeof(PartyControlDataParameters));
	SetControlDataParams(partyControlDataParams, pConfParty,NULL,NULL,NULL,TRUE,FALSE,FALSE,Regular,No_Lync,0,eRegularParty);

	PartyControlInitParameters partyControInitParam;
    memset(&partyControInitParam, 0, sizeof(PartyControlInitParameters));
	SetControlInitParams(partyControInitParam,pConfParty,FALSE,NULL,0,0,0,NULL);

	pPartyConnection->ChangeSipfromIceToNoneIce(partyControInitParam,partyControlDataParams);
}

//--------------------------------------------------------------------------
DWORD CConf::AddPartyDialingSequence(CConfPartiesDialingSequence* pConfPartiesDialingSequence, CCommRes* pMR, char* dialString,
		map<WORD, WORD> & interfaceOrder)
{
	PARTY_SEQUENCE_LIST* pList = new PARTY_SEQUENCE_LIST;
	DWORD status = 0;
	CLargeString cstr;

	DWORD partyId = pMR->NextPartyId();
	map<WORD, WORD>::const_iterator indexedItr = interfaceOrder.begin();
	map<WORD, WORD>::const_iterator endItr = interfaceOrder.end();

	while(indexedItr != endItr)
	{
		WORD interfaceOrder = (*indexedItr).first ;
		WORD interfaceType = (*indexedItr).second ;

		switch(interfaceType)
		{
			case H323_INTERFACE_TYPE:
				AddIPPartyForAllServicesToSequence(pList, partyId, dialString, interfaceType, 1720);
				cstr << "Adding H323 party with num: " << dialString << " and status: " << status <<"\n";
			break;
			case SIP_INTERFACE_TYPE:
				AddIPPartyForAllServicesToSequence(pList, partyId, dialString, interfaceType, 1720);
				cstr << "Adding SIP party with num: " << dialString << " and status: " << status <<"\n";
			break;

			case PSTN_INTERFACE_TYPE:
				AddISDNPartyForAllServicesToSequence(pList, pMR, partyId, dialString, YES);
				cstr << "Adding PSTN party with num: " << dialString << " and status: " << status <<"\n";
			break;

			case ISDN_INTERFACE_TYPE:
				AddISDNPartyForAllServicesToSequence(pList, pMR, partyId, dialString, NO);
			break;

			default:
			break;
    } // switch

		indexedItr++;
	}

	if (!pList->empty())
		pConfPartiesDialingSequence->AddPartyListToMap(partyId,pList);
	else
	{
		CMedString errorStr;
		errorStr << "CConf::AddPartyDialingSequence, PARTY LIST IS EMPTY!!!, dialString: " << dialString << "\n";
		PTRACE(eLevelInfoNormal,errorStr.GetString());
		delete pList;
	}

	PTRACE(eLevelInfoNormal,cstr.GetString());
	return partyId;
}

//--------------------------------------------------------------------------
void CConf::AddISDNPartyForAllServicesToSequence(PARTY_SEQUENCE_LIST* pList, CCommRes* pMR, DWORD partyId, char* dialString, BYTE isPstn)
{
	CMedString cstr;
	cstr << "CConf::AddISDNPartyForAllServicesToSequence" << "\n";

	WORD NAME_PREFIX_LEN = H243_NAME_LEN - strlen(DTMF_INVITE_PARTY_NAME_SUFFIX);

	ALLOCBUFFER(ISDNServiceNameStr,NET_SERVICE_PROVIDER_NAME_LEN);
	memset(ISDNServiceNameStr,'\0',NET_SERVICE_PROVIDER_NAME_LEN);

	CConfPartyProcess* pConfPartyProcess = (CConfPartyProcess*)CConfPartyProcess::GetProcess();

	const RTM_ISDN_PARAMS_MCMS_S * pIsdnServiceStruct = pConfPartyProcess->GetIsdnService(serviceName);

	if (pIsdnServiceStruct)
	{
		strncpy(ISDNServiceNameStr,(char*)pIsdnServiceStruct->serviceName,NET_SERVICE_PROVIDER_NAME_LEN);
		CRsrvParty* pRsrvParty = new CRsrvParty();

		pRsrvParty->SetServiceProviderName(ISDNServiceNameStr);
		pRsrvParty->SetConnectionType(DIAL_OUT);
		pRsrvParty->SetNetInterfaceType(ISDN_INTERFACE_TYPE);

		ALLOCBUFFER(h243name,H243_NAME_LEN);
		strncpy(h243name,dialString,NAME_PREFIX_LEN);
		strcat(h243name,DTMF_INVITE_PARTY_NAME_SUFFIX);
		pRsrvParty->SetName(h243name);
		DEALLOCBUFFER(h243name);

		if(isPstn == YES)
		{
			pRsrvParty->SetVoice(YES);
		}

		pRsrvParty->SetNetChannelNumber(AUTO);
		pRsrvParty->SetBondingMode1(AUTO);
		pRsrvParty->AddCallingPhoneNumber(dialString);
		pRsrvParty->SetUndefinedType(UNRESERVED_PARTY);
		pRsrvParty->SetPartyId(partyId);

		cstr << "Adding ISDN party with num: " << dialString <<"\n";
		pList->push_back(pRsrvParty);
	}
	else
	{
		cstr << "failed to get pIsdnServiceStruct " << "\n";
	}

	cstr << "Adding PSTN party with num: " << dialString  <<"\n";

	PTRACE(eLevelInfoNormal,cstr.GetString());
	DEALLOCBUFFER(ISDNServiceNameStr);
}

//--------------------------------------------------------------------------
void CConf::AddIPPartyForAllServicesToSequence(PARTY_SEQUENCE_LIST* pList, DWORD partyId, char* dialString, WORD interfaceType, DWORD callSignallingPort)
{
	CMedString cstr;
	PARTY_SEQUENCE_LIST pLocalList;

	eIPProtocolType eIPProtocolTypeEnum = eIPProtocolType_SIP_H323;
	BYTE serviceDefaultType = DEFAULT_SERVICE_BOTH;

	CIpServiceListManager::VectIterator indexedItr = ::GetIpServiceListMngr()->VectBegin();
	CIpServiceListManager::VectIterator endItr = ::GetIpServiceListMngr()->VectEnd();
	DWORD numOfService = ::GetIpServiceListMngr()->numberOfIpServices();

	ALLOCBUFFER(IPServiceNameStr,NET_SERVICE_PROVIDER_NAME_LEN);
	memset(IPServiceNameStr,'\0',NET_SERVICE_PROVIDER_NAME_LEN);


	while(indexedItr != endItr)
	{
		CConfIpParameters* pIpParams = (*indexedItr);
		CONF_IP_PARAMS_S* pConfIpParams =  pIpParams->GetConfIpParamsStruct();

		if(pIpParams == NULL)
		{
		  PASSERT(101);
		  DEALLOCBUFFER(IPServiceNameStr);
		  return;
		}

		serviceDefaultType = (eIPProtocolType)pConfIpParams->service_default_type;
		eIPProtocolTypeEnum = (eIPProtocolType)pConfIpParams->service_protocol_type;
		if((eIPProtocolTypeEnum == eIPProtocolType_SIP && interfaceType == SIP_INTERFACE_TYPE) ||
		   (eIPProtocolTypeEnum == eIPProtocolType_H323 && interfaceType == H323_INTERFACE_TYPE) ||
		   eIPProtocolTypeEnum  == eIPProtocolType_SIP_H323)
		{
			const char* serv_name = (const char*) (pIpParams ? pIpParams->GetServiceName() : NULL);
			if( serv_name )
			{
				strncpy(IPServiceNameStr, serv_name, NET_SERVICE_PROVIDER_NAME_LEN);
			}

			CRsrvParty* pInvitedPartyOut = new CRsrvParty();
			mcTransportAddress sIpAddr;

			pInvitedPartyOut->SetConnectionType(DIAL_OUT);
			pInvitedPartyOut->SetNetInterfaceType(interfaceType);
			pInvitedPartyOut->SetServiceProviderName(IPServiceNameStr);
			pInvitedPartyOut->SetUndefinedType(UNRESERVED_PARTY);

			ALLOCBUFFER(h243name,H243_NAME_LEN);
			strncpy(h243name, dialString, H243_NAME_LEN);
			h243name[H243_NAME_LEN - 1] = '\0';

			memset(&sIpAddr,0,sizeof(mcTransportAddress));

			if (strchr(dialString,'.') != NULL)
			{
				stringToIp(&sIpAddr,dialString);
				ReplaceChar(h243name, '.', '_');
			}
			else
			{
				if(interfaceType == SIP_INTERFACE_TYPE)
				{
					pInvitedPartyOut->SetSipPartyAddress(dialString);
				}
				else if(interfaceType == H323_INTERFACE_TYPE)
				{
					pInvitedPartyOut->SetH323PartyAliasType(PARTY_H323_ALIAS_E164_TYPE);
					pInvitedPartyOut->SetH323PartyAlias(dialString);
				}
			}

			pInvitedPartyOut->SetIpAddress(sIpAddr);
			pInvitedPartyOut->SetCallSignallingPort(callSignallingPort/*5060*/); //??

			strcat(h243name,DTMF_INVITE_PARTY_NAME_SUFFIX);
			pInvitedPartyOut->SetAdditionalInfo(DTMF_INVITE_PARTY_NAME_SUFFIX);

			pInvitedPartyOut->SetName(h243name);
			DEALLOCBUFFER(h243name);

			pInvitedPartyOut->SetPartyId(partyId);

			cstr << "match: eIPProtocolTypeEnum: " << eIPProtocolTypeEnum << " interfaceType:" << interfaceType<<"\n";

			cstr << "serviceDefaultType: " << serviceDefaultType << " interfaceType:" << interfaceType<<"\n";
			if((serviceDefaultType == DEFAULT_SERVICE_SIP && interfaceType == SIP_INTERFACE_TYPE) ||
			   (eIPProtocolTypeEnum == DEFAULT_SERVICE_H323 && interfaceType == H323_INTERFACE_TYPE) ||
			   eIPProtocolTypeEnum  == DEFAULT_SERVICE_BOTH)
			{
				cstr << "pLocalList.push_front "<< "\n";
				pLocalList.push_front(pInvitedPartyOut);
			}
			else
			{
				cstr << "pLocalList.push_back "<< "\n";
				pLocalList.push_back(pInvitedPartyOut);
			}
		}
		else
		{
			cstr << "no match: eIPProtocolTypeEnum: " << eIPProtocolTypeEnum << " interfaceType:" << interfaceType<<"\n";
		}

    indexedItr++;
	}

	pList->insert(pList->end(), pLocalList.begin(), pLocalList.end());
	DEALLOCBUFFER(IPServiceNameStr);
}

//--------------------------------------------------------------------------
void CConf::OnDtmfInviteParty(CSegment* pParam)
{
	CLargeString cstr;

	char dialingString[PARTY_ADDRESS_LEN] = {0};
	WORD numOfDialingOrder;
	DWORD invitorRecId;
	DWORD invitorMonitorId;
	map<WORD, WORD> dailingOrder;
	DWORD partyId = INVALID;

	*pParam >> invitorRecId;
	*pParam >> invitorMonitorId;
	*pParam >> dialingString;
	*pParam >> numOfDialingOrder;

	m_pDtmfInvitedPartyDialingSequence = new CConfPartiesDialingSequence();
	m_pTextOnScreenMngrForInvitedSession = new CTextOnScreenMngrForInvitedSession(this);
	m_pInvitedPartiesState = new INVITED_PARTIES_STATUS;

	if (m_isGateWay)
	{
		ALLOCBUFFER(partyNumOrIp,256); AUTO_DELETE_ARRAY(partyNumOrIp);
		memset(partyNumOrIp,'\0',256);
		char *origDialingString = dialingString;
		// get the number until the first *
		BYTE isPstn = NO;
		if(IsContainingNumbers(origDialingString))
			isPstn = ParseString(partyNumOrIp, origDialingString);
		partyId = AddPartyDialingSequenceForGw(m_pDtmfInvitedPartyDialingSequence,m_pCommConf,partyNumOrIp,isPstn,cstr);
		cstr << "\n";
	}
	else
	{

		cstr << "CConf::OnDtmfInviteParty dialingString: "<< dialingString << " numOfDialingOrder: "<<numOfDialingOrder <<"\n";
		for(int index = 0; index < numOfDialingOrder; index++)
		{
	    		WORD interface;
			WORD order;

			*pParam >> interface;
			*pParam >> order;

			const char *descrInterface = NULL;
			BYTE res = CStringsMaps::GetDescription(INTERFACE_TYPE_ENUM, interface, &descrInterface);

			cstr << "interface: "<< interface << " " << descrInterface << " order: "<< order <<"\n";

			if(order != 0) //insert only interface defined
			{
	    			//not an error in this map we define the order as key because in this way
	    			//we make sure that we will get it by order while iterating.
	    			dailingOrder[order] = interface;
			}
		}
		partyId = AddPartyDialingSequence(m_pDtmfInvitedPartyDialingSequence, m_pCommConf, dialingString, dailingOrder);
	}

	m_invitePartyMap[invitorRecId] = partyId;
	m_PartiesInvitorId[partyId] = invitorMonitorId;
	m_lastInvitedParticipantId = partyId;
	TRACEINTO << "invitorMonitorId:" << invitorMonitorId << ", lastInvitedParticipantId:" << m_lastInvitedParticipantId;

	if (partyId == 0xFFFFFFFF)
	{
		cstr << "AddPartyDialingSequence returned with status fail";
		PTRACE(eLevelInfoNormal,cstr.GetString());
	}

	// BRIDGE-1531 - In case of re-dial in gateway
	if (m_isGateWay)
	{
		CConfParty* pInvitorConfParty = m_pCommConf->GetCurrentParty(invitorMonitorId);
		if (pInvitorConfParty && pInvitorConfParty->GetVoice())
		{
			PTRACE2(eLevelInfoNormal,"CConf::OnDtmfInviteParty, Call initiator is Audio only, making all dial out parties Audio only as well, Name - ", m_name);
			m_pDtmfInvitedPartyDialingSequence->SetVoice(YES);
		}
	}

	cstr << "m_pDtmfInvitedPartyDialingSequence is: \n";
	m_pDtmfInvitedPartyDialingSequence->DumpToTrace(cstr);

	PTRACE(eLevelInfoNormal, cstr.GetString());

	if('\0'==dialingString[0])
	{
		//Only when dtmf reach max retry times, dialing string is null
		//In this case, null dialing string is consider as wrong number other than fail. Also,
		//no need toIVR  retry at conf side.
		m_PartiesRedialNum[invitorMonitorId] = 0;
		SaveInviteResult(partyId, H323_INTERFACE_TYPE, eRemoteWrongNumber);//select H323_INTERFACE_TYPE as a valid interface type
	}

	StartInvitePartyDialOutLoop(invitorMonitorId);
}

//--------------------------------------------------------------------------
void CConf::OnDisconnectInvitedParticipantReq(CSegment* pParam)
{
	PartyRsrcID initiatorRsrcID;

	*pParam >> initiatorRsrcID;

	TRACEINTO << "initiatorRsrcID:" << initiatorRsrcID << ", lastInvitedParticipantId:" << m_lastInvitedParticipantId;

	if (m_lastInvitedParticipantId != DUMMY_PARTY_ID)
	{
		CConfParty* pCurConfParty = m_pCommConf->GetCurrentParty(m_lastInvitedParticipantId);
		if (!pCurConfParty)
		{
			TRACEINTO << "last invited participant is not active, so do nothing";
		}
		else
		{
			char* curPartyName = (char*)pCurConfParty->GetName();

			CConfApi confApi;
			confApi.CreateOnlyApi(*(m_pCommConf->GetRcvMbx()));
			confApi.DropParty(curPartyName);
			confApi.DestroyOnlyApi();

			std::ostringstream msg;
			if (IsInvitedPartyInSetupStage(m_lastInvitedParticipantId, msg))
			{
				TRACEINTO << msg.str().c_str();

				// dial out party connected - send text message to inviter
				if (pCurConfParty->GetConnectionType() == DIAL_OUT)
				{
					DWORD invitorId = m_PartiesInvitorId[m_lastInvitedParticipantId];
					m_PartiesRedialNum.erase(invitorId);
					m_PartiesInviteType.erase(invitorId);

					CSmallString cstr;
					GenerateGeneralDisconnectedMessage(pCurConfParty, cstr);
					AddPartyMsgOnScreenForInvitedConf(m_lastInvitedParticipantId, cstr.GetString(), 5 * SECOND, FALSE, TRUE);
					InvitedPartyEndSetup(m_lastInvitedParticipantId);
				}
			}
		}
		m_lastInvitedParticipantId = DUMMY_PARTY_ID;
	}
}

//--------------------------------------------------------------------------
void CConf::SetInvitorPartyDtmfForward()
{
    TRACEINTO << "ConfName:" << m_name;

    map<DWORD, DWORD>::iterator indexedItr = m_invitePartyMap.begin();
    map<DWORD, DWORD>::iterator endItr = m_invitePartyMap.end();

  while (indexedItr != endItr)
  {
      DWORD partyId = (*indexedItr).first;
	  CPartyCntl* pPartyCntl = GetPartyCntl(partyId);
	  if (!pPartyCntl)
	  {
		  TRACEINTO << "- Failed, invalid value of pPartyCntl. Party Id is: " << partyId;
		  PASSERT(1);
	  }
	  else
	  {
		  CPartyApi* pPartyApi = pPartyCntl->GetPartyApi();
		  if (!pPartyApi)
		  {
			  TRACEINTO << " - invalid value of pPartyApi, Party Name: " << pPartyCntl->GetFullName();
			  PASSERT(1);
		  }
		  else
		  {
			  pPartyApi->SetDTMFForwarding(TRUE, SET_INVITED_DTMF_FORWARD);
		  }
	  }
      indexedItr++;
  }

  m_invitePartyMap.clear();
}

//--------------------------------------------------------------------------
void  CConf::StartInvitePartyDialOutLoop(DWORD inviterPartyId)
{
	CSmallString cstr1;
	cstr1 << "(start) CConf::StartInvitePartyDialOutLoop, partyId: " << inviterPartyId;
	// set the inviter state as setup
	(*m_pInvitedPartiesState)[inviterPartyId] = GW_CONNECT;
	m_isDtmfInviteParty = TRUE;
	//m_invitedPartiesStateFlag = TRUE;

	if (m_pDtmfInvitedPartyDialingSequence)
	{
		SEQUENCE_DIAL_MAP_PER_CONF* partiesMap = m_pDtmfInvitedPartyDialingSequence->GetDialMap();

		if (partiesMap && !partiesMap->empty())
		{

			//InviterPartyId is not in m_PartiesRedialNum, that means dial retry is not proceed for this party
			if(m_PartiesRedialNum.find(inviterPartyId) == m_PartiesRedialNum.end())
			{
				DWORD cfgWrongNumberDialRetries;
				CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
					"WRONG_NUMBER_DIAL_RETRIES", cfgWrongNumberDialRetries);

				m_PartiesRedialNum[inviterPartyId] = cfgWrongNumberDialRetries;
				PTRACE2HINT(eLevelInfoNormal,"CConf::StartInvitePartyDialOutLoop,set the m_PartiesRedialNum to WRONG_NUMBER_DIAL_RETRIES for inviterPartyId:",inviterPartyId);
				m_PartiesInviteType[inviterPartyId] = eInviteTypeDtmf;
			}

			SEQUENCE_DIAL_MAP_PER_CONF::iterator it;
			for (it = partiesMap->begin();it != partiesMap->end();++it)
			{
				DWORD partyId = it->first;
				(*m_pInvitedPartiesState)[partyId] = GW_SETUP;
				cstr1 << "partyId: " << partyId;
				cstr1 << " m_pInvitedPartiesState: " << (*m_pInvitedPartiesState)[partyId];

				eGeneralDisconnectionCause defaultCause = eGeneralDisconnectionCauseLast;
				CMedString defaultCauseStr;

				m_PartiesInvitorId[partyId] = inviterPartyId;

				PTRACE(eLevelInfoNormal,cstr1.GetString());
				AddNextPartyForInvitePartySession(partyId,(BYTE)defaultCause,defaultCauseStr);
			}
			return;
		}
	}

	FailToStartDtmfInvitedSession();
}

//--------------------------------------------------------------------------
void CConf::AddNextPartyForInvitePartySession(DWORD partyId, BYTE discCause, CMedString& discCauseStr, int i)
{
	CRsrvParty* nextParty = m_pDtmfInvitedPartyDialingSequence->GetNextParty(partyId);
	CSmallString cstr1;
	cstr1 << "(start) CConf::AddNextPartyForInvitePartySession, partyId: " << partyId << " iteration: " << i;
	PTRACE(eLevelInfoNormal,cstr1.GetString());
	cstr1.Clear();
	if(discCause > eRemoteBusy && CPObject::IsValidPObjectPtr(nextParty))
	{
		m_pTextOnScreenMngrForInvitedSession->AddParty(nextParty->GetPartyId());

		CConfParty tmpConfParty(*nextParty); // create an instance of ongoing party for some exteernal functions
		// validity test??
		DWORD status = TestDtmfInvitedPartyValidity(nextParty);
		// there is next party with bad status
		if (status != STATUS_OK)
		{
			// in case the last disconnection cause is failed
			// create a new default disconnection string with the correct protocol
			// otherwise keep the last disconnection string
			if (discCause > eRemoteWrongNumber)
			{
				eGeneralDisconnectionCause tmpCause = eRemoteFailed;
				GenerateGeneralDisconnectionCause(&tmpConfParty,tmpCause);
				discCause = (BYTE)tmpCause;
				SaveInviteResult(partyId, nextParty->GetNetInterfaceType(), (eGeneralDisconnectionCause)discCause);
				discCauseStr.Clear();
				GenerateGeneralDisconnectionMessageFromCause(&tmpConfParty,
											m_PartiesInviteResults[partyId].eOverallInviteResult,discCauseStr);
			}
			else
			{
				SaveInviteResult(partyId, nextParty->GetNetInterfaceType(), (eGeneralDisconnectionCause)discCause);
			}

			cstr1 << "CConf::AddNextPartyForDtmfInvitedSession, partyId: " << partyId << " discCauseStr: "<< discCauseStr.GetString() << " iteration: " << i;
			PTRACE(eLevelInfoNormal,cstr1.GetString());

			AddNextPartyForInvitePartySession(partyId,discCause,discCauseStr,i+1);

			cstr1 << "(back from recursive call) CConf::AddNextPartyForDtmfInvitedSession, partyId: " << partyId << " discCauseStr: "<< discCauseStr.GetString() << " iteration: " << i;
			PTRACE(eLevelInfoNormal,cstr1.GetString());
			cstr1.Clear();
		}
		// there is next party with good status
		else
		{

			TRACEINTO <<  "(back from recursive call) CConf::AddNextPartyForDtmfInvitedSession, partyId: " << partyId << " discCauseStr: "<< discCauseStr.GetString() << " iteration: " << i;

			CSmallString cstr;
			GenerateGeneralDialingMessage(nextParty,cstr);
			// add next party to conf
			AddPartyMsgOnScreenForInvitedConf(nextParty->GetPartyId(), cstr.GetString());
			// CDR???
			m_pCommConf->NewUndefinedParty(&tmpConfParty,EVENT_NEW_UNDEFINED_PARTY);
			COstrStream str;
			CRsrvParty* pRsrvParty = new CRsrvParty(*nextParty);
			pRsrvParty->Serialize(NATIVE,str);
			str << (DWORD)0 << "\n";
			CSegment*  pSeg = new CSegment;
			*pSeg << (DWORD)0;
			*pSeg << str.str().c_str();
			DispatchEvent(ADDPARTY,pSeg);
			POBJDELETE(pRsrvParty);
			POBJDELETE(pSeg);
		}

		POBJDELETE(nextParty);
	}
	// no more parties to dial
	else
	{
		BOOL  b_RetryProcessed = HandlePartyDialRetry(partyId);

		cstr1 << "(End) CConf::AddNextPartyForInvitePartySession, partyId: " << partyId << " iteration: " << i;
		PTRACE(eLevelInfoNormal,cstr1.GetString());
		cstr1.Clear();

		if (!discCauseStr.IsEmpty()) // protection - should not happen
		{
			AddPartyMsgOnScreenForInvitedConf(partyId, discCauseStr.GetString()/*,5*SECOND,FALSE,TRUE*/);
		}
		else
		{
			PASSERT(1);
		}

		InvitedPartyEndSetup(partyId,b_RetryProcessed);
	}
}

//--------------------------------------------------------------------------
void CConf::ObtainDisplayNameFromAddressBook(CConfParty* pRsrvParty,CIpNetSetup* pIpNetSetup)
{
	PTRACE2(eLevelInfoNormal,"CConf::ObtainDisplayNameFromAddressBook : - ",pRsrvParty->GetName());

	CCustomizeDisplaySettingForOngoingConfConfiguration* pCustomizeSetting =  ((CConfPartyProcess*) (CConfPartyProcess::GetProcess()))->GetCustomizeDisplaySettingForOngoingConfConfiguration();
	if (pCustomizeSetting->IsObtainDsipalyNamefromAddressBook() == FALSE)
		return;

	if (pRsrvParty->GetConnectionType() == DIAL_IN && pRsrvParty->IsUndefinedParty())
	{
		if (pRsrvParty->GetNetInterfaceType() == H323_INTERFACE_TYPE)
		{
			CH323NetSetup *pH323IPSetup = (CH323NetSetup*)pIpNetSetup;
			int numOfSrcAlias;
			CH323Alias * pH323Alias = pH323IPSetup->GetSrcPartyAliasList(&numOfSrcAlias);

			for (int i = 0 ; i < numOfSrcAlias; i++)
			{
				PTRACE2(eLevelInfoNormal,"CConf::ObtainDisplayNameFromAddressBook : Alias Name of calling party ",pH323Alias[i].GetAliasName());
				if (SearchAddressBookByAlias(pH323Alias[i].GetAliasName(),pRsrvParty) == TRUE)
				{
					delete[] pH323Alias;
					return;
				}
			}

			const mcTransportAddress* currPartyIpAddr = pH323IPSetup->GetTaSrcPartyAddr();
			SearchAddressBookByIPAddress(pRsrvParty,currPartyIpAddr);

			delete[] pH323Alias;
		}
		else if (pRsrvParty->GetNetInterfaceType() == SIP_INTERFACE_TYPE)
		{
			CSipNetSetup *pSipIPSetup = (CSipNetSetup*)pIpNetSetup;
			const mcTransportAddress* mcTransportAddress = pSipIPSetup->GetTaSrcPartyAddr();
			char tempName[IPV6_ADDRESS_LEN];
			memset (&tempName,'\0',IPV6_ADDRESS_LEN);
			ipToString(*mcTransportAddress,tempName,1);
			PTRACE2(eLevelInfoNormal,"CConf::ObtainDisplayNameFromAddressBook : sip src ip address ",tempName);
			PTRACE2(eLevelInfoNormal,"CConf::ObtainDisplayNameFromAddressBook : src sip address ",pSipIPSetup->GetRemoteSipAddress());
			if (isIpV4Str((const char*)tempName) == TRUE)
			{
				if (SearchAddressBookByIPAddress(pRsrvParty,mcTransportAddress) == TRUE)
						return;
			}

			SearchAddressBookBySipAddress(pSipIPSetup->GetRemoteSipAddress(),pRsrvParty);
		}
		else if (pRsrvParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE)
    { }
	}
}

//--------------------------------------------------------------------------
BOOL CConf::SearchAddressBookByAlias(const char* AliasName,CConfParty* pRsrvParty)
{
	PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByAlias : - ",pRsrvParty->GetName());

	CAddressBook* pAddressBook = CAddressBook::Instance();
	multiset<CRsrvParty*, CompareByPartyName>*   pPartiesSet = pAddressBook->GetAddressBookPartiesSet();

	CRsrvParty* pTmpParty = NULL;
	char buf[32];
	snprintf(buf, ARRAYSIZE(buf), "%d",pPartiesSet->size());
	PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByAlias address book size: - ",buf);

	char callingPartyAliasName[IP_STRING_LEN];
	strcpy_safe(callingPartyAliasName,AliasName);
	RemoveSpaceAndHyphenFromString(callingPartyAliasName);
	PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByAlias callingPartyAlias: - ",callingPartyAliasName);

	char PartyInAddressBookAliasName[IP_STRING_LEN];
	multiset<CRsrvParty*, CompareByPartyName>::iterator end = pPartiesSet->end();
	for (multiset<CRsrvParty*, CompareByPartyName>::iterator itr = pPartiesSet->begin(); itr != end; ++itr)
	{
		pTmpParty = *itr;
		PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByAlias : Name in address book ",pTmpParty->GetName());
		strcpy_safe(PartyInAddressBookAliasName,pTmpParty->GetH323PartyAlias());
		RemoveSpaceAndHyphenFromString(PartyInAddressBookAliasName);
		PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByAlias : Alias name in address book ",PartyInAddressBookAliasName);

		if ((IsTwoAliasNamesMatch(callingPartyAliasName,PartyInAddressBookAliasName) == TRUE) || (strncasecmp(callingPartyAliasName,PartyInAddressBookAliasName,IP_STRING_LEN) == 0))
		{
			PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByAlias : Alias Name match ",AliasName);
			pRsrvParty->SetFoundInAddressBook(TRUE);
			pRsrvParty->SetVisualPartyName(pTmpParty->GetName());

			return TRUE;
		}
  }

  return FALSE;
}

//--------------------------------------------------------------------------
BOOL CConf::SearchAddressBookBySipAddress(const char* SipAddress,CConfParty* pRsrvParty)
{
	PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookBySipAddress : - ",pRsrvParty->GetName());
	CAddressBook* pAddressBook = CAddressBook::Instance();
	multiset<CRsrvParty*, CompareByPartyName>*   pPartiesSet = pAddressBook->GetAddressBookPartiesSet();
	multiset<CRsrvParty*, CompareByPartyName>::iterator itr;
	CRsrvParty* pTmpParty = NULL;
	char buf[32];
	memset(buf,0,sizeof(buf));
	sprintf(buf,"%d",pPartiesSet->size());
	PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookBySipAddress address book size: - ",buf);

	for (itr = pPartiesSet->begin();itr != pPartiesSet->end();itr++)
	{
		pTmpParty = *itr;
		PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookBySipAddress : Name in address book ",pTmpParty->GetName());
		PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookBySipAddress : Sip Address in address book ",pTmpParty->GetSipPartyAddress());

		if (strncasecmp(SipAddress,pTmpParty->GetSipPartyAddress(),IP_STRING_LEN) == 0)
		{
			PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookBySipAddress : Alias Name match ",SipAddress);
			pRsrvParty->SetFoundInAddressBook(TRUE);
			pRsrvParty->SetVisualPartyName(pTmpParty->GetName());
			return TRUE;
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------------
BOOL CConf::SearchAddressBookByIPAddress(CConfParty* pRsrvParty,const mcTransportAddress* currPartyIpAddr )
{
	PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByIPAddress : - ",pRsrvParty->GetName());
	CAddressBook* pAddressBook = CAddressBook::Instance();
	multiset<CRsrvParty*, CompareByPartyName>*   pPartiesSet = pAddressBook->GetAddressBookPartiesSet();
	multiset<CRsrvParty*, CompareByPartyName>::iterator itr;
	CRsrvParty* pTmpParty = NULL;

	for (itr = pPartiesSet->begin();itr != pPartiesSet->end();itr++)
	{
		pTmpParty = *itr;

		mcTransportAddress tmp_ipAddress = pTmpParty->GetIpAddress();

		char tempName[64];
		memset (&tempName,'\0',IPV6_ADDRESS_LEN);
		ipToString(tmp_ipAddress,tempName,1);
		PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByIPAddress : Name in addr book ",pTmpParty->GetName());
		PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByIPAddress : ip in addr book ",tempName);

		memset (&tempName,'\0',IPV6_ADDRESS_LEN);
		ipToString(*currPartyIpAddr,tempName,1);
		PTRACE2(eLevelInfoNormal,"CConf::ObtainDisplayNameFromAddressBook : ip of calling party ",tempName);

		if ((!isApiTaNull(&tmp_ipAddress)) && ((!isApiTaNull(currPartyIpAddr))))
	      {
	      		if (tmp_ipAddress.ipVersion == currPartyIpAddr->ipVersion)
	      		{
	      			if ((tmp_ipAddress.ipVersion == (APIU32)eIpVersion4) && (currPartyIpAddr->ipVersion == (APIU32)eIpVersion4))
	      			{
	      				if (tmp_ipAddress.addr.v4.ip == currPartyIpAddr->addr.v4.ip)
					{
						PTRACE2(eLevelInfoNormal,"CConf::SearchAddressBookByIPAddress : ip v4 match ,visual name",pTmpParty->GetName());
						pRsrvParty->SetFoundInAddressBook(TRUE);
						pRsrvParty->SetVisualPartyName(pTmpParty->GetName());
						return TRUE;
					}
	      			}
	      			else if ((tmp_ipAddress.ipVersion == (APIU32)eIpVersion6) && (currPartyIpAddr->ipVersion == (APIU32)eIpVersion6))
	      			{
	      				if (!memcmp((tmp_ipAddress.addr.v6.ip),(currPartyIpAddr->addr.v6.ip),IPV6_ADDRESS_BYTES_LEN))
					{
						PTRACE(eLevelInfoNormal,"CConf::SearchAddressBookByIPAddress : ip v6 match");
						pRsrvParty->SetFoundInAddressBook(TRUE);
						pRsrvParty->SetVisualPartyName(pTmpParty->GetName());
						return TRUE;
					}
	      			}
	      		}
	      }
	}

	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CConf::IsTwoAliasNamesMatch(const char* alias_1, const char* alias_2)
{
	if ((alias_1 == NULL) || (alias_2 == NULL))
		return FALSE;

	if ((strlen(alias_1) == 0) || (strlen(alias_2) == 0))
		return FALSE;

	if (CObjString::IsNumeric(alias_1) && CObjString::IsNumeric(alias_2))
	{
		const char* p = NULL;
		if (strlen(alias_1) >= strlen(alias_2))
		{
			p = strstr(alias_1,alias_2) ;
			if ( p != NULL && (p == alias_1))
				return TRUE;
		}
		else
		{
			 p = strstr(alias_2,alias_1);
			if ( p != NULL && (p == alias_2))
				return TRUE;
		}

		return FALSE;
	}

	return FALSE;
}



//--------------------------------------------------------------------------
void CConf::OnTimerMessageQueue(CSegment* pParam)
{
	StartTimer(MESSAGE_QUEUE_TOUT, SECOND);
}

//--------------------------------------------------------------------------
void  CConf::CreateConfPartyIfNeeded(char* name)
{
	 if (!strcmp(name, "UNKNOWN"))
	 {
	 	PTRACE2(eLevelError,"CConf::CreateConfPartyIfNeeded : \'MEET ME PER CONF NOT SUPPORTED !!!\' ",m_name);
	        CConfParty*  pConfParty = new CConfParty;
	        ALLOCBUFFER(countStr,20);
	        sprintf(countStr,"%d",m_inCallCounter++);
	        strncat(name,countStr,H243_NAME_LEN-1-strlen(name));
	        name[H243_NAME_LEN-1]='\0';
	        DEALLOCBUFFER(countStr);

	        pConfParty->SetName(name);

	        pConfParty->SetPartyState(PARTY_CONNECTING,m_pCommConf->GetMonitorConfId());
	        pConfParty->SetDisconnectCause(0xFFFFFFFF);
	        pConfParty->SetQ931DisconnectCause(0);

	        m_pCommConf->Add(*pConfParty);  //?? Update
	        POBJDELETE(pConfParty);
	 }
}
//////////////////////////////////////////////////////////////////////////////////////////
void  CConf::SetupConfPartyOnH323(CConfParty*pConfParty, CH323NetSetup* pIpNetSetup)
{
	mcTransportAddress ip;
	memset(&ip,0,sizeof(mcTransportAddress));
	memcpy(&ip, (pIpNetSetup->GetTaSrcPartyAddr()), sizeof(mcTransportAddress));

	pConfParty->SetNetInterfaceType(H323_INTERFACE_TYPE);
	pConfParty->SetConnectionType(DIAL_IN);
	pConfParty->SetIpAddress(ip);
	pConfParty->SetCallSignallingPort(1720);
	pConfParty->SetH323PartyAlias(pIpNetSetup->GetH323PartyAlias());
	pConfParty->SetH323PartyAliasType(pIpNetSetup->GetH323PartyAliasType());

	pConfParty->SetBackupIpAddress(pConfParty->GetIpAddress());
	pConfParty->SetBackupH323PartyAlias(pConfParty->GetH323PartyAlias());

	if(pConfParty->IsUndefinedParty())
	{
		PTRACE(eLevelInfoNormal,"CConf::SetupConfPartyOnH323 -undefparty");
		m_pCommConf->NewUndefinedParty(pConfParty,EVENT_NEW_UNDEFINED_PARTY);
		if (ip.ipVersion == eIpVersion6)
		{
			char ipAddrremote[64];
			::ipToString(ip,ipAddrremote,1);
			m_pCommConf->OperatorIpV6PartyCont1(pConfParty, ipAddrremote, EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS);
		}

		PTRACE2(eLevelInfoNormal,"CConf::SetupConfPartyOnH323 : before fetching address book, Party VisualName - ",pConfParty->GetVisualPartyName());

		ObtainDisplayNameFromAddressBook(pConfParty,pIpNetSetup);

		PTRACE2(eLevelInfoNormal,"CConf::SetupConfPartyOnH323 : After fetching address book,Party VisualName - ",pConfParty->GetVisualPartyName());
	}
}
/////////////////////////////////////////////////////////////////////////////
BOOL CConf::GetIsMrcCall(BYTE bIsMrcHeader, CConfParty* pConfParty)
{
	BOOL bIsMrcCall = FALSE;

	eConfMediaType confMediaType = (eConfMediaType)m_pCommConf->GetConfMediaType();
	if ((pConfParty && pConfParty->GetConnectionType()==DIAL_OUT && pConfParty->GetCascadeMode()==CASCADE_MODE_SLAVE)
		&& (CProcessBase::GetProcess()->GetProductType()==eProductTypeSoftMCUMfw))
	{
		TRACEINTO << "Slave Mrc call";
		bIsMrcCall = TRUE;
	}
	else if (confMediaType == eMixAvcSvcVsw )
	{
		  TRACEINTO<<"avc_vsw_relay: confMediaType is:"<<confMediaType<<" bIsMrcHeader is:"<<((bIsMrcHeader==TRUE ? "YES":"NO"));
		  if (bIsMrcHeader)
			  bIsMrcCall = TRUE;
	}
	else
	{
		  if (confMediaType == eSvcOnly) 	// in case no SVC licensing - call will continue connection as SVC and will be rejected by Resource Allocator
		  {
			  bIsMrcCall = TRUE;
			  if (!bIsMrcHeader) // This case is blocked in ConfPartyManager level.
				  PASSERTMSG(1,"CConf::GetIsMrcCall - AVC call in SVC only conf.");
		  }
		  else if (confMediaType == eMixAvcSvc)
		  {
			  if (bIsMrcHeader)
			  {
				if (::GetDongleSvcValue() == TRUE)		// BRIDGE-7424 - SVC call is possible only if it is enabled by the license - otherwise, connect as AVC to mixed mode conf.
					bIsMrcCall = TRUE;
				else
					TRACEINTO << "No SVC licensing! SVC call will connect as AVC to the mixed mode conference.";
			  }
		  }

	}

	return bIsMrcCall;
}
/////////////////////////////////////////////////////////////////////////////
EStat  CConf::SetupConfPartyOnSIP(CConfParty*pConfParty, CSipNetSetup* pNetSetup, BYTE bIsMrcHeader, BYTE bIsRemoteSlave)
{
	EStat retStat = statOK;

	BOOL bIsMrcCall = GetIsMrcCall(bIsMrcHeader, pConfParty);

	mcTransportAddress trIp;
	memset(&trIp,0,sizeof(mcTransportAddress));
	pConfParty->SetIpAddress(trIp);
	pConfParty->SetCallSignallingPort(5060);
	pConfParty->SetSipPartyAddress(pNetSetup->GetRemoteSipAddress());
	pConfParty->SetSipPartyAddressType(pNetSetup->GetRemoteSipAddressType());

	// backup the originate parameters of the dial in party
	pConfParty->SetBackupIpAddress(pConfParty->GetIpAddress());
	pConfParty->SetBackupH323PartyAlias(pConfParty->GetH323PartyAlias());

	TRACEINTO << "BG Audio only conference? pConfParty->SetVoice(YES)\n\tPartyMonitorID = " << pConfParty->GetPartyId()
			<< "\n\tm_pCommConf->GetMedia() = " << (int)(m_pCommConf->GetMedia());
	if (m_pCommConf->GetMedia() == AUDIO_MEDIA)
	{
		pConfParty->SetVoice(YES);
	}

	if(pConfParty->IsUndefinedParty())
	{
		pConfParty->SetIsEncrypted(pNetSetup->GetInitialEncryptionValue());

		if (bIsMrcCall)
        {
			pConfParty->SetPartyMediaType(eSvcPartyType);
            // Set the party encryption to AUTO for SVC party.
            // This is done in order to support EPs that support encryption for AVC but for SVC they do not.
            TRACEINTO << "Set the party encryption to AUTO for SVC party.";
            pConfParty->SetIsEncrypted(AUTO); // as for dial-out
        }

		m_pCommConf->NewUndefinedParty(pConfParty,EVENT_NEW_UNDEFINED_PARTY);
		//  PTRACE2(eLevelInfoNormal,"CConf::OnSipLobbyAddPartyConnect -undefparty ip is, ",pNetSetup->GetRemoteSipAddress());
		char*		strHostIp = NULL;
		char* strAt	= (char*)strstr(pNetSetup->GetRemoteSipAddress(),"[");
		strHostIp= strAt ? strAt : NULL;
		BYTE bIsUriWithIp =FALSE;
		int	hostLen = 0;
		if (strHostIp)
		{
			//PTRACE(eLevelInfoNormal,"CConf::OnSipLobbyAddPartyConnect -strhost true ");
			mcTransportAddress trAddr;
			memset(&trAddr,0,sizeof(mcTransportAddress));
			::stringToIp(&trAddr,strHostIp);
			BYTE isIpAddrValid = ::isApiTaNull(&trAddr);
			if (isIpAddrValid != TRUE)
			{
				isIpAddrValid = ::isIpTaNonValid(&trAddr);
				if (isIpAddrValid != TRUE)
					bIsUriWithIp = TRUE;
			}

			hostLen	= strHostIp ? strlen(strHostIp) : NO;
			if(bIsUriWithIp)
			{
				char strTransportIp[IPV6_ADDRESS_LEN];
				strncpy(strTransportIp, strHostIp, sizeof(strTransportIp) - 1);
				strTransportIp[sizeof(strTransportIp) - 1] = '\0';
				memset(&trAddr,0,sizeof(mcTransportAddress));
				::stringToIp(&trAddr,strTransportIp);
				if (trAddr.ipVersion == eIpVersion6)
				{
					pConfParty->SetIpAddress(trAddr);
					m_pCommConf->OperatorIpV6PartyCont1(pConfParty, strTransportIp, EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS);
				}
			}

			PTRACE2(eLevelInfoNormal,"CConf::SetupConfPartyOnSIP : before fetching address book, Party VisualName - ",pConfParty->GetVisualPartyName());

				ObtainDisplayNameFromAddressBook(pConfParty,pNetSetup);

			PTRACE2(eLevelInfoNormal,"CConf::SetupConfPartyOnSIP : After fetching address book,Party VisualName - ",pConfParty->GetVisualPartyName());
		}


		// check if there is a slave_link in this conf
		//   (if so, it cannot accept a call from another Slave; otherwise it's ok and it becomes Master)
		if (bIsRemoteSlave)
		{
			PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
			for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
			{
				CPartyConnection* pPartyConnection = _itr->second;
				if (pPartyConnection)
				{
					CConfParty* pConfParty = m_pCommConf->GetCurrentParty(pPartyConnection->GetMonitorPartyId());
					if (pConfParty)
					{
						if (true == pConfParty->IsCascadeModeSlave())
						{
							// there is a slave_link in this conf - cannot accept a call from another Slave
							TRACEINTO << "PartyName:" << pPartyConnection->GetName() << " - A Slave already exist in this conference, cannot receive a call from another Slave";
							retStat = statIllegal;
							break;
						}
					}
				}
			} // end loop over parties

			// no slave_link in this conf - set to Master
			if (statOK == retStat)
			{
				TRACEINTO << "---cascade--- Set as Master";
				pConfParty->SetCascadeMode(CASCADE_MODE_MASTER);

//				// Currently, encryption is not supported in Cascade
//				if ( NO != pConfParty->GetIsEncrypted() )
//				{
//					TRACEINTO << "Currently, encryption is not supported in Cascade link";
//					pConfParty->SetIsEncrypted(NO);
//				}
			}

		} // end if(bIsRemoteSlave)

	} // end if(UndefinedParty)

	return retStat;
}

/////////////////////////////////////////////////////////////////////////////
void CConf::LegacyOnDemandCheck(CPartyConnection* pPartyConnection, BYTE isBlockContent)
{
	PTRACE(eLevelInfoNormal, "CConf::LegacyOnDemandCheck ");
	if (!pPartyConnection)
		return;

	CPartyCntl* pCurrentPartyCntl = pPartyConnection->GetPartyCntl();
	CConfParty* pConfParty = GetCommConf()->GetCurrentParty(pPartyConnection->GetMonitorPartyId());

	if (pCurrentPartyCntl == NULL || pConfParty == NULL || TRUE != pConfParty->GetIsLyncPlugin())
		return;

	char myName[MAX_SITE_NAME_SIZE] = { 0 };

	if (pConfParty->GetSipUsrName())
	{
		strncpy(myName, pConfParty->GetSipUsrName(), MAX_SITE_NAME_SIZE - 1);
		myName[MAX_SITE_NAME_SIZE - 1] = '\0';
		PTRACE2(eLevelInfoNormal, "CConf::LegacyOnDemandCheck : Party SipUsrName - ", pConfParty->GetSipUsrName());
	}
	else if (pConfParty->GetRemoteName())
	{
		strncpy(myName, pConfParty->GetRemoteName(), MAX_SITE_NAME_SIZE - 1);
		myName[MAX_SITE_NAME_SIZE - 1] = '\0';
		PTRACE2(eLevelInfoNormal, "CConf::LegacyOnDemandCheck : Party RemoteName - ", pConfParty->GetRemoteName());
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CConf::LegacyOnDemandCheck: Cannot get Plugin Name! ");
		return;
	}

	char* pPrefix = NULL;
	pPrefix = strstr(myName, "_cssplugin");
	if (!pPrefix)
	{
		PTRACE2(eLevelInfoNormal, "CConf::LegacyOnDemandCheck- Invalid name for Plugin - ", myName);
		return;
	}
	else
	{
		//cut off the surfix
		*pPrefix = '\0';
	}

	PARTYLIST::iterator _end = m_pPartyList->m_PartyList.end();
	for (PARTYLIST::iterator _itr = m_pPartyList->m_PartyList.begin(); _itr != _end; ++_itr)
	{
		CPartyConnection* pLyncPartyConnection = _itr->second;
		if (!pLyncPartyConnection)
			continue;

		CPartyCntl* pLyncPartyCntl = pLyncPartyConnection->GetPartyCntl();
		if (!pLyncPartyCntl)
			continue;

		if (pLyncPartyCntl->GetInterfaceType() != SIP_INTERFACE_TYPE)
			continue;

		if (pLyncPartyCntl->IsLync())
		{
			CConfParty* pConfPartyLync = GetCommConf()->GetCurrentParty(pLyncPartyCntl->GetMonitorPartyId());
			if (!pConfPartyLync)
				continue;

			char lyncName[MAX_SITE_NAME_SIZE] = { 0 };
			if (pConfPartyLync->GetSipUsrName())
			{
				strcpy_safe(lyncName, MAX_SITE_NAME_SIZE, pConfPartyLync->GetSipUsrName());
				TRACEINTO << "LyncName:" << lyncName << " - SIP user name";
			}
			else if (pConfPartyLync->GetRemoteName())
			{
				strcpy_safe(lyncName, MAX_SITE_NAME_SIZE, pConfPartyLync->GetRemoteName());
				TRACEINTO << "LyncName:" << lyncName << " - Remote name";
			}
			else
			{
				PTRACE(eLevelInfoNormal, "CConf::LegacyOnDemandCheck: Cannot get Lync valid Name! ");
				continue;
			}

			if (0 == strcasecmp(lyncName, myName))
			{
				pLyncPartyCntl->UpdateLegacyContentStatus(isBlockContent);
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CConf::HandleEventPackageEvent(CSegment* pMsg)
{
	TRACEINTO ;
	PASSERT_AND_RETURN(!pMsg);
	BYTE isOnlyRmxConnected = IsOnlyRmxInAvMcuConf();
	BYTE m_isAvMcuConnected = FALSE;
	if(isOnlyRmxConnected && m_AvMcuPartyRsrcId)
	{
		if ((m_state != TERMINATION) && m_pCommConf->IsAutomaticTermination())
		  {
			CConfParty*  pConfParty = m_pCommConf->GetFirstParty(); // get the first party
			 WORD numParties = m_pCommConf->GetNumParties();
			  for (WORD i = 0; i < numParties; i++)
			 {
				if (pConfParty)
				{
				  	if(pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveOut && pConfParty->GetAvMcuLinkType() != eAvMcuLinkSlaveIn )
				  	{
				  		if(pConfParty->GetPartyState() == PARTY_CONNECTING || pConfParty->GetPartyState() == PARTY_CONNECTED_PARTIALY || pConfParty->GetPartyState() == PARTY_CONNECTED || pConfParty->GetPartyState() == PARTY_CONNECTED_WITH_PROBLEM ||  pConfParty->GetPartyState() == PARTY_SECONDARY )
				  			m_isAvMcuConnected = TRUE;

				  			break;
				  	}
				 }
				pConfParty = m_pCommConf->GetNextParty();
			 }

			  if(m_isAvMcuConnected == FALSE || IsValidTimer(AUTOTERMINATE))
			  	 {
			  	  		return;
			  	  }

			  DelParty(pConfParty->GetName());


		  }
	}


}
/////////////////////////////////////////////////////
BYTE CConf::IsOnlyRmxInAvMcuConf()
{
	TRACEINTO << m_AvMcuPartyRsrcId;
	if (m_AvMcuPartyRsrcId == 0 )
	{
		TRACEINTO << "can't find av-mcu link ";
		return FALSE;
	}
	BYTE isOnlyRmxLeft = FALSE;
	DWORD numOfAVEps = 0;

	EventPackage::Manager& EventPackage = EventPackage::Manager::Instance();

	const EventPackage::Conference* pConference = EventPackage.GetConference(m_AvMcuPartyRsrcId);
	if(pConference == NULL)
	{
		TRACEINTO << "no pConferenceInfo ";
		return FALSE;
	}
	const EventPackage::Users::UsersContainer& users = pConference->m_users.m_users;
	TRACEINTO << users.size();
	if (users.size() == 1)
	{
		TRACEINTO << "only RMX is left in the conf: ";
		isOnlyRmxLeft = TRUE;
	}
	else if (users.size() > 1)
	{
		for (EventPackage::Users::UsersContainer::const_iterator _iuser = users.begin(); _iuser != users.end(); ++_iuser)
		{
			const EventPackage::User::EndpointsContainer& endpoints = _iuser->m_endpoints;
			for (EventPackage::User::EndpointsContainer::const_iterator _iendpoint = endpoints.begin(); _iendpoint != endpoints.end(); ++_iendpoint)
			{
				if (_iendpoint->m_sessionType == EventPackage::eSessionType_AudioVideo)
				{
					numOfAVEps++;
					TRACEINTO<< "_iendpoint->m_sessionType " <<_iendpoint->m_sessionType;
				}
			}
		}
	}

	if (numOfAVEps == 1)///only rmx
		isOnlyRmxLeft = TRUE;

	return isOnlyRmxLeft;
}

//////////////////////////////////////////////////////////////////////////////////
void CConf::OnActivateAutoTerminationTestForAvMcu()
{
	TRACEINTO<< m_AvMcuPartyRsrcId;
	ActivateAutoTerminationForMsAvMcu("CConf::OnActivateAutoTerminationTestForAvMcu");
}

//////////////////////////////////////////////////////////////////////////////////
/*Begin:added by Richer for BRIDGE-12062 ,2014.3.3*/
void CConf::VideoRecoverySendByeToParty(CSegment* pParam)
{
	char partyName[H243_NAME_LEN];
	*pParam >> partyName;

	TRACEINTO << "ConfName:" << m_name << ", PartyName:" << partyName;

	CPartyConnection* pPartyConnection = GetPartyConnection(partyName);
	TRACECOND_AND_RETURN(!pPartyConnection, "Failed, invalid pointer");

    //TRACECOND_AND_RETURN(pPartyConnection->IsDisconnect(), "Failed, party is disconnected");

    if (!(pPartyConnection->IsTaskAppIsValid()))
    {
        TRACEINTO << "pPartyConnection->IsTaskAppIsValid() = 0";
        return;
    }

    CTaskApp* pParty = pPartyConnection->GetPartyTaskApp();
    TRACECOND_AND_RETURN(!pParty, "Failed, invalid pointer");


    if (!CPObject::IsValidPObjectPtr(pParty))
    {
        return;
    }

    TRACECOND_AND_RETURN(!(&(pParty->GetRcvMbx())), "CConf::VideoRecoverySendByeToParty - Failed, 'RcvMbx' is not valid");

	CPartyApi* pPartyApi = new CPartyApi;
	pPartyApi->CreateOnlyApi(pParty->GetRcvMbx());
	pPartyApi->SendByeToParty(0x0);
	pPartyApi->DestroyOnlyApi();
	POBJDELETE(pPartyApi);
}
/*End:added by Richer for BRIDGE-12062 ,2014.3.3*/

//============================================================================================================//
// TELEPRESENCE_LAYOUTS
void CConf::OnSetTelepresenceLayoutModeSetup(CSegment* pParam)
{
	if(NULL == pParam){
		DBGPASSERT(1);
		return;
	}
	DWORD tmpLayoutMode = 0;
	*pParam >> tmpLayoutMode;
	ETelePresenceLayoutMode newLayoutMode = (ETelePresenceLayoutMode)(tmpLayoutMode);

	TRACEINTO << " newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);

	if(m_pCommConf)
		m_pCommConf->SetTelePresenceLayoutMode(newLayoutMode);


	// TODO - Ron
}
//============================================================================================================//
// TELEPRESENCE_LAYOUTS
void CConf::OnSetTelepresenceLayoutModeConnected(CSegment* pParam)
{
	if(NULL == pParam){
		DBGPASSERT(1);
		return;
	}
	DWORD tmpLayoutMode = 0;
	*pParam >> tmpLayoutMode;
	ETelePresenceLayoutMode newLayoutMode = (ETelePresenceLayoutMode)(tmpLayoutMode);

	TRACEINTO << " TELEPRESENCE_LAYOUTS_DEBUG newLayoutMode = " << TelePresenceLayoutModeToString(newLayoutMode);

	if(m_pCommConf)
		m_pCommConf->SetTelePresenceLayoutMode(newLayoutMode);

	if (m_pVideoBridgeInterface)
		m_pVideoBridgeInterface->SetTelepresenceLayoutMode(newLayoutMode);

}
//============================================================================================================//
//////////////////////////////////////////////////////////////
void CConf::DisconnectRdpGw(void)
{
	TRACEINTO << "Disconnect RDPGw";
	CConfParty *   pRdpGw	=  m_pCommConf->GetRDPGwParty();
	if(pRdpGw)
	{
		//Send local message
		TRACEINTO << "Disconnect RDP-Gw when AvMCU is disconnected";
		char* curPartyName = (char*)pRdpGw->GetName();
		CConfApi *pSelfApi = new CConfApi;
		pSelfApi->CreateOnlyApi(GetRcvMbx(), this);
		pSelfApi->DropParty(curPartyName);
		pSelfApi->DestroyOnlyApi();
		POBJDELETE(pSelfApi);
	}
	else
	{
		TRACEINTO << "Disconnect RDPGw - no RDP GW";
	}

}
