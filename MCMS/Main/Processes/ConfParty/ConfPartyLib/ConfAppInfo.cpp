//+========================================================================+
//            Copyright 2005 Polycom Networking Ltd.                       |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Networking Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConfAppInfo.cpp	                                   |
// SUBSYSTEM:  ConfParty                                                   |
//+========================================================================+


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

#include "ConfAppMngr.h"
#include "StatusesGeneral.h"
#include "ConfApi.h"
#include "TaskApp.h"
#include "ConfAppMngrInitParams.h"
#include "BridgePartyInitParams.h"
#include "ConfAppBridgeParams.h"
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "ConfPartyGlobals.h"
#include "IVRService.h"
#include "IVRServiceList.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "AudHostApiDefinitions.h"
//#include "ProductType.h"

//extern const char* ProductTypeToString(eProductType productType);
//extern DWORD GetCurrentLoggerNumber();

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// ##############################################################
// ################									#############
// ################		CConfAppInfo				#############
// ################									#############
// ##############################################################
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////




/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CConfAppInfo::CConfAppInfo()
{
	m_firstPartyMessagePlayed	= 0;
	m_confWithIVR				= 1;
	m_confActiveEventsCounter	= 0;
	m_confActiveExitToneCounter	= 0;
	m_confStartIvrMode			= START_IVR_CONF_STOPPED;
	m_confState					= eAPP_CONF_STATE_CONNECT;
	m_confAudioVideoType		= APP_CONF_TYPE_AUDIO;
	m_isSinglePartyNow			= 0;
	m_isRingToneOn				= 0;
	m_singlePartyRsrcId			= (DWORD)(-1);
	m_isWaitForChair			= 1;
	m_isTerminateConfAfterChairDropped=1;
	m_inWaitForChairNow			= 0;
	m_isConfPastWaitForChairStage	= 0;
	m_pConfApi					= NULL;
	m_pAudBrdgInterface			= NULL;
	m_pVideoBridgeInterface		= NULL;
	m_ivrName 					= NULL;
	m_pConfName					= NULL;
	m_confRsrcId				= (DWORD)(-1);
	m_isChairInConf				= 0;
	m_isEntryQueue				= 0;
	m_isInMuteIncomingParties	= NO;
	m_isEnableNoisyLineDetection = eOff;
	m_defualtNoisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_5;
	m_rollCallAnnounceYesNo		= 1;
	m_terminateAfterChairExits	= 0;
	m_isCascadeEQ				= 0;
	m_enableRecording			  = NO;
	m_enableRecordingIcon   = YES;
	m_enableRecNotify		=YES;
	m_startRecordingPolicy	= START_RECORDING_IMMEDIATELY;
	m_recordingFailedFlag		= NO;
	m_recordingLinkInConf		= 0;
	m_recordingLinkRsrcId		= (DWORD)(-1);
	m_recordingInitiatorId		= (DWORD)(-1);
	m_startRecDtmfWasSent		= 0;
	m_lastReqRecAction			= SET_START_RECORDING;
	m_rlDisconnectionWasReceived = NO;
	m_ivrMessageVolume			= E_AUDIO_GAIN_PRESET_4;
	m_ivrMusicVolume			= E_AUDIO_GAIN_PRESET_5;
	m_ivrRollCallVolume			= E_AUDIO_GAIN_PRESET_4;
	m_isGateWay					= NO;
	m_isOperatorConf = NO;
	m_isExternalIVRInConf		= NO;
	m_MCUproductType			= CProcessBase::GetProcess()->GetProductType();
	m_isMuteAllAudioButLeader   = NO;
    m_loggerConfStarted			= 0;
    m_loggerConfLastReport		= 0;
	m_recordingControlInProgress	=0;
	m_recordingControlActionSaved = eStopRecording;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
CConfAppInfo::~CConfAppInfo()
{
	POBJDELETE(m_pConfApi);
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::Create(const CConfAppMngrInitParams* pConfAppMngrInitParams)
{
	m_pConfName             = pConfAppMngrInitParams->m_pConfName;
	m_confRsrcId            = pConfAppMngrInitParams->m_confRsrcId;
	m_pAudBrdgInterface     = pConfAppMngrInitParams->m_pAudBrdgInterface;
	m_pVideoBridgeInterface = pConfAppMngrInitParams->m_pVideoBridgeInterface;
	m_confWithIVR           = pConfAppMngrInitParams->m_isIvrInConf;
	m_isWaitForChair        = pConfAppMngrInitParams->m_isWaitForChair;
	m_isTerminateConfAfterChairDropped= pConfAppMngrInitParams->m_isTerminateConfAfterChairDropped;
	m_ivrName               = pConfAppMngrInitParams->m_confIvrName;
	m_isEntryQueue          = pConfAppMngrInitParams->m_isEQConf;
	m_isCascadeEQ           = pConfAppMngrInitParams->m_isCascadeEQ;
	m_enableRecording       = pConfAppMngrInitParams->m_enableRecording;
	m_enableRecordingIcon   = pConfAppMngrInitParams->m_enableRecordingIcon;
	m_enableRecNotify	     = pConfAppMngrInitParams->m_enablerecNotify;
	m_startRecordingPolicy  = pConfAppMngrInitParams->m_startRecordingPolicy;
	m_isGateWay             = pConfAppMngrInitParams->m_isGateWay;
	m_isOperatorConf        = pConfAppMngrInitParams->m_isOperatorConf;
	m_isExternalIVRInConf       = pConfAppMngrInitParams->m_isExternalIVRInConf;
	m_isMuteAllAudioButLeader           = pConfAppMngrInitParams->m_isMuteAllPartiesAudioExceptLeader;
//	m_pConf								= (CConf*)pConfAppMngrInitParams->m_pConf;

	if ((NULL == m_ivrName) || (0 == strcmp(m_ivrName, "\0")))
	{
		PTRACE(eLevelError, "CConfAppInfo::Create - Illegal IVR service name");
		m_confWithIVR = 0;
	}

	if (m_isEntryQueue)
	{
		if (m_isCascadeEQ)
			PTRACE(eLevelInfoNormal, "CConfAppInfo::Create - This is a cascade EQ");
		else
			PTRACE(eLevelInfoNormal, "CConfAppInfo::Create - This is a regular EQ (not cascade)");
	}

	// Update IsEntryQ
	UpdateIsEntryQueue();

	// conf type: audio or video
	m_confAudioVideoType = APP_CONF_TYPE_AUDIO;
	if (1 == pConfAppMngrInitParams->m_isVideoConf)
		m_confAudioVideoType = APP_CONF_TYPE_VIDEO;

	// create conf-API
	CConf* pConf = (CConf*)pConfAppMngrInitParams->m_pConf;

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(pConf->GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif
	m_pConfApi->CreateOnlyApi(pConf->GetRcvMbx());
	m_pConfApi->SetLocalMbx(pConf->GetLocalQueue());

	// noisy line detection
	UpdateNoisyLineParams();

	// Conference requires chairperson
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	//BOOL bTerminateConfAfterChairDropped;
	//sysConfig->GetBOOLDataByKey(CFG_KEY_TERMINATE_CONF_AFTER_CHAIR_DROPPED, bTerminateConfAfterChairDropped);

	if ((m_isWaitForChair) && (m_isTerminateConfAfterChairDropped))
			SetTerminateAfterChairExits( 1 );

	// IVR Volume (message)
	DWORD systemFlag = 0;
	sysConfig->GetDWORDDataByKey(CFG_IVR_MESSAGE_VOLUME, systemFlag);
	SetIvrMessageVolume(systemFlag);

	// IVR Volume (Music)
	sysConfig->GetDWORDDataByKey(CFG_IVR_MUSIC_VOLUME, systemFlag);
	SetIvrMusicVolume(systemFlag);

	// IVR Volume (RollCall)
	sysConfig->GetDWORDDataByKey(CFG_IVR_ROLL_CALL_VOLUME, systemFlag);
	SetIvrRollCallVolume(systemFlag);

	// get is tone instead of Roll-Call voice
	BOOL bIvrIsRollCallTone = FALSE;
	CAVmsgService* pAvService = (NULL != m_ivrName) ? ::GetpAVmsgServList()->GetCurrentAVmsgService(m_ivrName) : NULL;
	if (pAvService)
	{
		CIVRService* pIVRService = (CIVRService*) pAvService->GetIVRService();
		if (pIVRService)
		{
			const CIVRRollCallFeature* pRollCallFeature = pIVRService->GetRollCallFeature();
			if (pRollCallFeature != NULL)
			{
				bIvrIsRollCallTone = pRollCallFeature->GetUseTones();
			}
		}
	}

	if (bIvrIsRollCallTone)
	{
		PTRACE(eLevelInfoNormal, "CConfAppInfo::Create - roll call using tones instead of messages");
	}

	m_loggerConfStarted = GetCurrentLoggerNumber();
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
int CConfAppInfo::IsFirstPartyMessagePlayed()
{
	return	m_firstPartyMessagePlayed;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetFirstPartyMessagePlayed( WORD action )
{
	m_firstPartyMessagePlayed = action;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::IncConfActiveEventsCounter()
{
	m_confActiveEventsCounter++;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::DecConfActiveEventsCounter()
{
	if (m_confActiveEventsCounter > 0)
	m_confActiveEventsCounter--;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetConfActiveEventsCounter()
{
	return m_confActiveEventsCounter;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetConfActiveEventsCounter(WORD numOfActiveEvents)
{
	m_confActiveEventsCounter = numOfActiveEvents;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::IncConfActiveExitToneCounter()
{
	m_confActiveExitToneCounter++;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::DecConfActiveExitToneCounter()
{
	if (m_confActiveExitToneCounter > 0)
	m_confActiveExitToneCounter--;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetConfActiveExitToneCounter()
{
	return m_confActiveExitToneCounter;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetConfStartIVRMode()
{
	return m_confStartIvrMode;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetConfStartIVRMode( WORD confStartIvrMode)
{
	m_confStartIvrMode = confStartIvrMode;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetConfState( TAppConfState confState )
{
	if ((confState > eAPP_CONF_STATE_MIN) && (confState < eAPP_CONF_STATE_MAX))
		m_confState = confState;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetConfState()
{
	return m_confState;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetConfWithIVR()
{
	return m_confWithIVR;
}

BYTE CConfAppInfo::GetIsExternalIVRInConf()
{
	return m_isExternalIVRInConf;
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetConfAudioVideoType( WORD confType )
{
	m_confAudioVideoType = confType;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetConfAudioVideoType()
{
	return m_confAudioVideoType;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetWaitForChair( WORD waitForChairYesNo )
{
	TRACEINTO << " Set m_isWaitForChair: " << (DWORD)waitForChairYesNo;
	m_isWaitForChair = waitForChairYesNo;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::IsWaitForChair()
{
	return m_isWaitForChair;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIvrServiceName( const char* ivrName )
{
	m_ivrName = ivrName;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
const char* CConfAppInfo::GetIvrServiceName()
{
	return m_ivrName;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetInWaitForChairNow( WORD inWaitForChairNow, BOOL onDisconnect )
{
	TRACEINTO << " Set Wait For Chair: " << (DWORD)inWaitForChairNow;
	// BRIDGE-9978 after chairperson leaves, if conference is not configured to disconnect, chairperson is no longer required.
	BOOL isResetting = (m_inWaitForChairNow != 0) && (inWaitForChairNow == 0);
	if (isResetting && !onDisconnect)	// disconnecting means we did not really go past the 'wait for chair' stage
		SetIsConfPastWaitForChairStage(TRUE);
	m_inWaitForChairNow = inWaitForChairNow;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::IsInWaitForChairNow()
{
	return m_inWaitForChairNow;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIsConfPastWaitForChairStage( WORD isConfPastWaitForChairStage )
{
	TRACEINTO << " No longer in 'Wait For Chair': " << (DWORD)isConfPastWaitForChairStage;

	m_isConfPastWaitForChairStage = isConfPastWaitForChairStage;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::IsConfPastWaitForChairStage()
{
	return m_isConfPastWaitForChairStage;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetLeaderInConfNow( WORD isChairInConf )
{
	m_isChairInConf = isChairInConf;
	// BRIDGE-9978 when entering chair pwd for conf pwd, the 'm_isConfPastWaitForChairStage' didn't get set
	if (m_isChairInConf && m_isWaitForChair)
	{
		TRACEINTO << "setting m_isConfPastWaitForChairStage to 1";
		m_isConfPastWaitForChairStage = TRUE;
	}
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetIsLeaderInConfNow()
{
	return m_isChairInConf;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetSinglePartyNow( WORD isSinglePartyNow )
{
	m_isSinglePartyNow = isSinglePartyNow;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetSinglePartyNow()
{
	return m_isSinglePartyNow;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIsRingToneOn( WORD isRingToneOn )
{
	m_isRingToneOn = isRingToneOn;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetIsRingToneOn()
{
	return m_isRingToneOn;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetSinglePartyRsrcId( DWORD singlePartyRsrcId )
{
	m_singlePartyRsrcId = singlePartyRsrcId;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
DWORD CConfAppInfo::GetSinglePartyRsrcId()
{
	return m_singlePartyRsrcId;
}


////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetTerminateAfterChairExits( WORD terminateAfterChairExits )
{
	m_terminateAfterChairExits = terminateAfterChairExits;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetTerminateAfterChairExits()
{
	return m_terminateAfterChairExits;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIsEntryQueue( WORD isEntryQueue )
{
	m_isEntryQueue = isEntryQueue;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetIsEntryQueue()
{
	return m_isEntryQueue;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
const char* CConfAppInfo::GetConfName()
{
	return m_pConfName;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIsInMuteIncomingParties( WORD isInMuteIncomingParties )
{
	m_isInMuteIncomingParties = isInMuteIncomingParties;
	if (YES == isInMuteIncomingParties)
	{
		PTRACE(eLevelInfoNormal, "CConfAppInfo::SetIsInMuteIncomingParties - Set to YES");
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CConfAppInfo::SetIsInMuteIncomingParties - Set to NO");
	}	
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetIsInMuteIncomingParties()
{
	return m_isInMuteIncomingParties;
}
void CConfAppInfo::SetIsMuteAllAudioButLeader(BOOL isMuteAllAudioButLader)
{
	m_isMuteAllAudioButLeader = isMuteAllAudioButLader;
}
BOOL CConfAppInfo::GetIsMuteAllAudioButLeader ()
{
	return m_isMuteAllAudioButLeader;
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIsEnableNoisyLineDetection( EOnOff isEnableNoisyLineDetection )
{
	m_isEnableNoisyLineDetection = isEnableNoisyLineDetection;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
EOnOff CConfAppInfo::GetIsEnableNoisyLineDetection()
{
	return m_isEnableNoisyLineDetection;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetDefualtNoisyLineThresholdLevel( BYTE defualtNoisyLineThresholdLevel )
{
	m_defualtNoisyLineThresholdLevel = defualtNoisyLineThresholdLevel;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
BYTE CConfAppInfo::GetDefualtNoisyLineThresholdLevel()
{
	return m_defualtNoisyLineThresholdLevel;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetRollCallAnnounceYesNo( DWORD yesNo )
{
	m_rollCallAnnounceYesNo = (BYTE)yesNo;
	
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
BYTE CConfAppInfo::GetRollCallAnnounceYesNo()
{
	return m_rollCallAnnounceYesNo;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
BYTE CConfAppInfo::IsCascadeEQ()
{
	return m_isCascadeEQ;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetEnableRecording(WORD enableRecording)
{
	m_enableRecording = enableRecording;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetEnableRecording()
{
	return m_enableRecording;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetEnableRecordingIcon(WORD enableRecordingIcon)
{
  m_enableRecordingIcon = enableRecordingIcon;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetEnableRecordingIcon()
{
  return m_enableRecordingIcon;
}

//eFeatureRssDialin
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetEnableRecordingNotify(WORD enableRecordingNotify)
{
  m_enableRecNotify= enableRecordingNotify;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetEnableRecordingNotify()
{
  return m_enableRecNotify;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetStartRecordingPolicy(WORD startRecordingPolicy)
{
	m_startRecordingPolicy = startRecordingPolicy;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetStartRecordingPolicy()
{
	return m_startRecordingPolicy;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetRecordingFailedFlag(WORD recordingFailed)
{
	m_recordingFailedFlag = recordingFailed;
}
	
	
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetRecordingFailedFlag()
{
	return m_recordingFailedFlag;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetRecordingLinkInConf(WORD recordingLinkInConf)
{
	m_recordingLinkInConf = recordingLinkInConf;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetRecordingLinkInConf()
{
	return m_recordingLinkInConf;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetRecordingLinkRsrcId( DWORD recordingLinkRsrcId )
{
	m_recordingLinkRsrcId = recordingLinkRsrcId;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
DWORD CConfAppInfo::GetRecordingLinkRsrcId()
{
	return m_recordingLinkRsrcId;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetRecordingInitiatorId(DWORD partyRsrcID)
{
	m_recordingInitiatorId = partyRsrcID;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
DWORD CConfAppInfo::GetRecordingInitiatorId()
{
	return m_recordingInitiatorId;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetStartRecDtmfWasSent(WORD startRecDtmfWasSent)
{
	m_startRecDtmfWasSent = startRecDtmfWasSent;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetStartRecDtmfWasSent()
{
	return m_startRecDtmfWasSent;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetLastReqRecAction(WORD lastReqRecAction)
{
	m_lastReqRecAction = lastReqRecAction;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetLastReqRecAction()
{
	return m_lastReqRecAction;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetRlDisconnectionWasReceived(WORD rlDisconnectionWasReceived)
{
	m_rlDisconnectionWasReceived = rlDisconnectionWasReceived;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetRlDisconnectionWasReceived()
{
	return m_rlDisconnectionWasReceived;
}

/////////////////////////////////////////////////////////////////
const char* CConfAppInfo::GetStringFromOpcode(TConfAppEvents eOpcode)
{
	switch (static_cast<DWORD>(eOpcode))
	{
	case eCAM_EVENT_MIN:                                return "Illegal: eCAM_EVENT_MIN";
	case eCAM_EVENT_PARTY_END_IVR:                      return "eCAM_EVENT_PARTY_END_IVR";
	case eCAM_EVENT_PARTY_DELETED:                      return "eCAM_EVENT_PARTY_DELETED";
	case eCAM_EVENT_PARTY_PLAY_MSG_IND:                 return "eCAM_EVENT_PARTY_PLAY_MSG_IND";
	case eCAM_EVENT_PARTY_AUDIO_CONNECTED:              return "eCAM_EVENT_PARTY_AUDIO_CONNECTED";
	case eCAM_EVENT_PARTY_VIDEO_CONNECTED:              return "eCAM_EVENT_PARTY_VIDEO_CONNECTED";
	case eCAM_EVENT_PARTY_VIDEO_IVR_MODE_CONNECTED:     return "eCAM_EVENT_PARTY_VIDEO_IVR_MODE_CONNECTED";
	case eCAM_EVENT_PARTY_AUDIO_MOVE_OUT:               return "eCAM_EVENT_PARTY_AUDIO_MOVE_OUT";
	case eCAM_EVENT_PARTY_VIDEO_MOVE_OUT:               return "eCAM_EVENT_PARTY_VIDEO_MOVE_OUT";
	case eCAM_EVENT_PARTY_AUDIO_DISCONNECTING:          return "eCAM_EVENT_PARTY_AUDIO_DISCONNECTING";
	case eCAM_EVENT_LAST_PARTY_DISCONNECTING:           return "eCAM_EVENT_LAST_PARTY_DISCONNECTING";
	case eCAM_EVENT_PARTY_VIDEO_DISCONNECTING:          return "eCAM_EVENT_PARTY_VIDEO_DISCONNECTING";
	case eCAM_EVENT_CONF_TERMINATING:                   return "eCAM_EVENT_CONF_TERMINATING";
	case eCAM_EVENT_CONF_TERMINATED:                    return "eCAM_EVENT_CONF_TERMINATED";
	case eCAM_EVENT_CONF_ALERT_TONE:                    return "eCAM_EVENT_CONF_ALERT_TONE";
	case eCAM_EVENT_CONF_CHAIR_DROPPED:                 return "eCAM_EVENT_CONF_CHAIR_DROPPED";
	case eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED:  return "eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED";
	case eCAM_EVENT_CONF_LOCK:                          return "eCAM_EVENT_CONF_LOCK";
	case eCAM_EVENT_CONF_UNLOCK:                        return "eCAM_EVENT_CONF_UNLOCK";
	case eCAM_EVENT_CONF_SECURE:                        return "eCAM_EVENT_CONF_SECURE";
	case eCAM_EVENT_CONF_UNSECURE:                      return "eCAM_EVENT_CONF_UNSECURE";
	case eCAM_EVENT_CONF_MUTE_ALL:                      return "eCAM_EVENT_CONF_MUTE_ALL";
	case eCAM_EVENT_CONF_UNMUTE_ALL:                    return "eCAM_EVENT_CONF_UNMUTE_ALL";
	case eCAM_EVENT_CONF_MUTE_INCOMING_PARTIES:         return "eCAM_EVENT_CONF_MUTE_INCOMING_PARTIES";
	case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:              return "eCAM_EVENT_CONF_ROLL_CALL_REVIEW";
	case eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW:          return "eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW";
	case eCAM_EVENT_CONF_EXIT_TONE:                     return "eCAM_EVENT_CONF_EXIT_TONE";
	case eCAM_EVENT_CONF_ENTRY_TONE:                    return "eCAM_EVENT_CONF_ENTRY_TONE";
	case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:                return "eCAM_EVENT_CONF_WAIT_FOR_CHAIR";
	case eCAM_EVENT_CONF_END_WAIT_FOR_CHAIR:            return "eCAM_EVENT_CONF_END_WAIT_FOR_CHAIR";
	case eCAM_EVENT_CONF_ONHOLD:                        return "eCAM_EVENT_CONF_ONHOLD";
	case eCAM_EVENT_CONF_END_ONHOLD:                    return "eCAM_EVENT_CONF_END_ONHOLD";
	case eCAM_EVENT_CONF_NOISY_LINE_DETECTION:          return "eCAM_EVENT_CONF_NOISY_LINE_DETECTION";
	case eIVR_SHOW_PARTICIPANTS:                        return "eIVR_SHOW_PARTICIPANTS";
	case eIVR_SHOW_GATHERING:                           return "eIVR_SHOW_GATHERING";
	case eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK:          return "eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK";
	case eCAM_EVENT_PARTY_FIRST_TO_JOIN:                return "eCAM_EVENT_PARTY_FIRST_TO_JOIN";
	case eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC:           return "eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC";
	case eCAM_EVENT_PARTY_END_SINGLE_PARTY_MUSIC:       return "eCAM_EVENT_PARTY_END_SINGLE_PARTY_MUSIC";
	case eCAM_EVENT_PARTY_WAIT_FOR_CHAIR:               return "eCAM_EVENT_PARTY_WAIT_FOR_CHAIR";
	case eCAM_EVENT_PARTY_END_ALL_WAIT_FOR_CHAIR:       return "eCAM_EVENT_PARTY_END_ALL_WAIT_FOR_CHAIR";
	case eCAM_EVENT_PARTY_PARTY_ROLL_CALL_REVIEW:       return "eCAM_EVENT_PARTY_PARTY_ROLL_CALL_REVIEW";
	case eCAM_EVENT_PARTY_END_ROLL_CALL_REVIEW:         return "eCAM_EVENT_PARTY_END_ROLL_CALL_REVIEW";
	case eCAM_EVENT_PARTY_CHANGE_PW:                    return "eCAM_EVENT_PARTY_CHANGE_PW";
	case eCAM_EVENT_PARTY_SILENCE_IT:                   return "eCAM_EVENT_PARTY_SILENCE_IT";
	case eCAM_EVENT_PARTY_CHANGE_TO_LEADER:             return "eCAM_EVENT_PARTY_CHANGE_TO_LEADER";
	case eCAM_EVENT_PARTY_PLAY_MENU:                    return "eCAM_EVENT_PARTY_PLAY_MENU";
	case eCAM_EVENT_PARTY_MUTE:                         return "eCAM_EVENT_PARTY_MUTE";
	case eCAM_EVENT_PARTY_UNMUTE:                       return "eCAM_EVENT_PARTY_UNMUTE";
	case eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE:          return "eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE";
	case eCAM_EVENT_PARTY_OVERRIDE_MUTEALL:             return "eCAM_EVENT_PARTY_OVERRIDE_MUTEALL";
	case eCAM_EVENT_PARTY_PLC:                          return "eCAM_EVENT_PARTY_PLC";
	case eCAM_EVENT_PARTY_CANCEL_PLC:                   return "eCAM_EVENT_PARTY_CANCEL_PLC";
	case eCAM_EVENT_PARTY_CHANGE_LAYOUT_PLC:            return "eCAM_EVENT_PARTY_CHANGE_LAYOUT_PLC";
	case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_PLC:       return "eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_PLC";
	case eCAM_EVENT_PARTY_FORCE_PLC:                    return "eCAM_EVENT_PARTY_FORCE_PLC";
	case eCAM_EVENT_PARTY_VENUS:                        return "eCAM_EVENT_PARTY_VENUS";
	case eCAM_EVENT_PARTY_CANCEL_VENUS:                 return "eCAM_EVENT_PARTY_CANCEL_VENUS";
	case eCAM_EVENT_PARTY_CHANGE_LAYOUT_VENUS:          return "eCAM_EVENT_PARTY_CHANGE_LAYOUT_VENUS";
	case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_VENUS:     return "eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_VENUS";
	case eCAM_EVENT_PARTY_FORCE_VENUS:                  return "eCAM_EVENT_PARTY_FORCE_PLC";
	case eCAM_EVENT_PARTY_INC_VOLUME:                   return "eCAM_EVENT_PARTY_INC_VOLUME";
	case eCAM_EVENT_PARTY_DEC_VOLUME:                   return "eCAM_EVENT_PARTY_DEC_VOLUME";
	case eCAM_EVENT_PARTY_START_IVR:                    return "eCAM_EVENT_PARTY_START_IVR";
	case eCAM_EVENT_PARTY_SHOW_SLIDE:                   return "eCAM_EVENT_PARTY_SHOW_SLIDE";
	case eCAM_EVENT_PARTY_JOIN_CONF_VIDEO:              return "eCAM_EVENT_PARTY_JOIN_CONF_VIDEO";
	case eCAM_EVENT_PARTY_END_VIDEO_IVR:                return "eCAM_EVENT_PARTY_END_VIDEO_IVR";
	case SET_PARTY_AS_LEADER:                           return "SET_PARTY_AS_LEADER";
	case eCAM_EVENT_PARTY_IN_CONF_IND:                  return "eCAM_EVENT_PARTY_IN_CONF_IND";
	case eCAM_EVENT_PARTY_PLAY_RINGING_TONE:            return "eCAM_EVENT_PARTY_PLAY_RINGING_TONE";
	case eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE:       return "eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE";
	case eIVR_REQUEST_TO_SPEAK:                         return "eIVR_REQUEST_TO_SPEAK";
	case eCAM_EVENT_PARTY_CLIENT_SEND_DTMF:             return "eCAM_EVENT_PARTY_CLIENT_SEND_DTMF";
	case eCAM_EVENT_PARTY_END_FEATURE:                  return "eCAM_EVENT_PARTY_END_FEATURE";
	case eCAM_EVENT_PARTY_IVR_ENTRANCE:                 return "eCAM_EVENT_PARTY_IVR_ENTRANCE";
	case eCAM_EVENT_PARTY_END_LAST_FEATURE:             return "eCAM_EVENT_PARTY_END_LAST_FEATURE";
	case eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE:           return "eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE";
	case eCAM_EVENT_ADD_FEATURE_TO_WAIT_LIST:           return "eCAM_EVENT_ADD_FEATURE_TO_WAIT_LIST";// internal general events
	case eCAM_EVENT_PARTY_VIDEO_END_SHOW_SLIDE:         return "eCAM_EVENT_PARTY_VIDEO_END_SHOW_SLIDE";
	case eCAM_EVENT_TIMER_END_FEATURE:                  return "eCAM_EVENT_TIMER_END_FEATURE";
	case eCAM_EVENT_PARTY_ROLL_CALL_YESNO:              return "eCAM_EVENT_PARTY_ROLL_CALL_YESNO";
	case IVR_PARTY_READY_FOR_SLIDE:                     return "IVR_PARTY_READY_FOR_SLIDE";
	case eCAM_EVENT_PARTY_SEND_DTMF:                    return "eCAM_EVENT_PARTY_SEND_DTMF";
	case eCAM_EVENT_PARTY_RECORDING_FAILED:             return "eCAM_EVENT_PARTY_RECORDING_FAILED";
	case eCAM_EVENT_CONF_RECORDING_FAILED:              return "eCAM_EVENT_CONF_RECORDING_FAILED";
	case eCAM_EVENT_PARTY_RECORDING_IN_PROGRESS:        return "eCAM_EVENT_PARTY_RECORDING_IN_PROGRESS";
	case eCAM_EVENT_INVITE_PARTY:                       return "eCAM_EVENT_INVITE_PARTY";
	case eCAM_EVENT_CONF_DISCONNECT_INVITED_PARTICIPANT:return "eCAM_EVENT_CONF_DISCONNECT_INVITED_PARTICIPANT";
	case eCAM_EVENT_PARTY_PLAY_BUSY_MSG:                return "eCAM_EVENT_PARTY_PLAY_BUSY_MSG";
	case eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG:            return "eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG";
	case eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG:        return "eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG";

	case e_ACK_IND:                                     return "e_ACK_IND";
	case eCAM_EVENT_CONF_DTMF_FORWARDING:               return "eCAM_EVENT_CONF_DTMF_FORWARDING";
	case SET_START_RECORDING:                           return "SET_START_RECORDING";
	case SET_STOP_RECORDING:                            return "SET_STOP_RECORDING";
	case SET_PAUSE_RECORDING:                           return "SET_PAUSE_RECORDING";
	case SET_RESUME_RECORDING:                          return "SET_RESUME_RECORDING";
	case eCAM_EVENT_PARTY_UPDATE_GW_TYPE:               return "eCAM_EVENT_PARTY_UPDATE_GW_TYPE";

	case eCAM_EVENT_CONF_START_RESUME_RECORDING:        return "eCAM_EVENT_CONF_START_RESUME_RECORDING";
	case eCAM_EVENT_CONF_STOP_RECORDING:                return "eCAM_EVENT_CONF_STOP_RECORDING";
	case eCAM_EVENT_CONF_PAUSE_RECORDING:               return "eCAM_EVENT_CONF_PAUSE_RECORDING";
	case eCAM_EVENT_SET_AS_LEADER_FROM_API:             return "eCAM_EVENT_SET_AS_LEADER_FROM_API";

	case eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL:        return "eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL";
	case eCAM_EVENT_PARTY_REMOVE_FEATURE:               return "eCAM_EVENT_PARTY_REMOVE_FEATURE";

	case eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER:  return "eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER";

	case ECAM_EVENT_MAX:                                return "Illegal: ECAM_EVENT_MAX";
	}

	return "Illegal Opcode Value";
}

/////////////////////////////////////////////////////////////////
const char* CConfAppInfo::GetCamString()
{
	return "--->CAM";
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::UpdateNoisyLineParams()
{
	DWORD noisyLineThresholdLevel = 0;	// not implemented in Carmel GL1
	if (0 == noisyLineThresholdLevel)
		m_isEnableNoisyLineDetection = eOff;
	else 
		m_isEnableNoisyLineDetection = eOn;
		
	switch(noisyLineThresholdLevel)
	{
		case (0):  // Noise detection is disabled
			{
				PTRACE(eLevelInfoNormal, "CConfAppInfo::UpdateNoisyLineParams - Noise detection is disabled");
				m_defualtNoisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_1; 	// dummy parameter to prevent bad request indication
				break;
			}
			
		case (1):  // Noise detection Lowest sensitivity
			{
				m_defualtNoisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_9;
				break;
			}
			
		case (2):
			{
				m_defualtNoisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_7;
				break;
			}
			
		case (3):  // Noise detection Medium sensitivity
			{
				m_defualtNoisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_5;
				break;
			}
			
		case (4):
			{
				m_defualtNoisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_3;
				break;

			}
			
		case (5):  // Noise detection Highest sensitivity
			{
				m_defualtNoisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_1;
				break;
			}
			
		default:
			{
				PTRACE(eLevelError, "CConfAppInfo::UpdateNoisyLineParams - Invalid noise detection values in SYSTEM.CFG");
				m_isEnableNoisyLineDetection = eOff;
				m_defualtNoisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_1; 	// dummy parameter to prevent bad request indication
				DBGPASSERT(noisyLineThresholdLevel);
				break;
			}
	}
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::UpdateIsEntryQueue()
{
	CAVmsgService* pAvService = ::GetpAVmsgServList()->GetCurrentAVmsgService(m_ivrName);
	if (pAvService) 
	{
		CIVRService* pIVRService = (CIVRService*) pAvService->GetIVRService();
		if (pIVRService)
		{
			m_isEntryQueue = pIVRService->GetEntryQueueService();
		}
		else
			PTRACE(eLevelError, "CConfAppInfo::UpdateIsEntryQueue - Error: CIVRService = NULL");
	}
	else
		PTRACE(eLevelError, "CConfAppInfo::UpdateIsEntryQueue - Error: CAVmsgService = NULL");
	
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetRecordingStatus()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetConfName());
	if(pCommConf)
	{
		return (WORD)pCommConf->GetRecordingLinkControl();
	}
	else
	{
		PTRACE(eLevelError, "CConfAppInfo::GetRecordingStatus - pCommConf NULL pointer");
		return (WORD)(-1);
	}
}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetIvrPermission( WORD wEventOpCode )
{
	CAVmsgService* pAvService = ::GetpAVmsgServList()->GetCurrentAVmsgService(m_ivrName);
	if (pAvService) 
	{
		CIVRService* pIVRService = (CIVRService*) pAvService->GetIVRService();
		if (pIVRService)
		{
			WORD dtmfCodePermission = pIVRService->GetDTMFCodePermission( wEventOpCode );	
			return dtmfCodePermission; // OK
		}
		else
			{PTRACE(eLevelError, "CConfAppInfo::GetIvrPermission - Error: pAvService = NULL");}
	}
	else
		{PTRACE(eLevelError, "CConfAppInfo::GetIvrPermission - Error: CAVmsgService = NULL");}
		
	return (WORD)(-1); // error	
}




/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIvrMessageVolume(DWORD ivrVolume)
{
	ivrVolume++;	// The ++ comes to align the user numbers (0=no volume) to the eNum definition (1=no volume)
	if ((ivrVolume >= E_AUDIO_GAIN_PRESET_0) && (ivrVolume <= E_AUDIO_GAIN_PRESET_10))
		m_ivrMessageVolume = ivrVolume;
}



/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
DWORD CConfAppInfo::GetIvrMessageVolume()
{
	return m_ivrMessageVolume;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIvrMusicVolume(DWORD ivrVolume)
{
	ivrVolume++;	// The ++ comes to align the user numbers (0=no volume) to the eNum definition (1=no volume)
	if ((ivrVolume >= E_AUDIO_GAIN_PRESET_0) && (ivrVolume <= E_AUDIO_GAIN_PRESET_10))
		m_ivrMusicVolume = ivrVolume;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
DWORD CConfAppInfo::GetIvrMusicVolume()
{
	return m_ivrMusicVolume;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIvrRollCallVolume(DWORD ivrVolume)
{
	ivrVolume++;	// The ++ comes to align the user numbers (0=no volume) to the eNum definition (1=no volume)
	if ((ivrVolume >= E_AUDIO_GAIN_PRESET_0) && (ivrVolume <= E_AUDIO_GAIN_PRESET_10))
		m_ivrRollCallVolume = ivrVolume;
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
DWORD CConfAppInfo::GetIvrRollCallVolume()
{
	return m_ivrRollCallVolume;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
BOOL CConfAppInfo::IsRollCallToneInsteadVoice()
{
  // return m_bIvrRollCallToneInsteadVoice;

	// get is tone instead of Roll-Call voice
	BOOL bIvrIsRollCallTone = FALSE;
	CAVmsgService* pAvService = ::GetpAVmsgServList()->GetCurrentAVmsgService(m_ivrName);
	if (pAvService) 
	{
		CIVRService* pIVRService = (CIVRService*) pAvService->GetIVRService();
		if (pIVRService)
		{

		  const CIVRRollCallFeature* pRollCallFeature = pIVRService->GetRollCallFeature();
		  if(pRollCallFeature != NULL){
		    bIvrIsRollCallTone = pRollCallFeature->GetUseTones();
		  }
		}
	}

	return bIvrIsRollCallTone;

}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
// void CConfAppInfo::SetIsToneInsteadVoice(BOOL ivrIsRollCallTone)
// {
// 	if (ivrIsRollCallTone)
// 		PTRACE(eLevelInfoNormal, "CConfAppInfo::SetIsToneInsteadVoice - TRUE");
// 	else
// 		PTRACE(eLevelInfoNormal, "CConfAppInfo::SetIsToneInsteadVoice - FALSE");
		
		
// 	m_bIvrRollCallToneInsteadVoice = ivrIsRollCallTone;	
// }
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

DWORD CConfAppInfo::GetCachePriority( DWORD op_code, DWORD mediaType )
{
	// case of Video Slide
	if (IVR_MEDIA_TYPE_VIDEO == mediaType)
		return PRIORITY_2_CACHE_IVR;
	
	// case of Audio IVR message
	DWORD priority = PRIORITY_1_CACHE_IVR;
	
	switch( op_code )
	{
		case eCAM_EVENT_PARTY_FIRST_TO_JOIN:
		case eCAM_EVENT_CONF_ALERT_TONE:
		case eCAM_EVENT_CONF_WAIT_FOR_CHAIR:
			priority = PRIORITY_2_CACHE_IVR;
			break;
		
		case eCAM_EVENT_CONF_ENTRY_TONE:
		case eCAM_EVENT_CONF_EXIT_TONE:
			priority = PRIORITY_5_CACHE_IVR;
			break;
	}
	
	return priority;
}

/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetIsGateWay(WORD isGateWay)
{
	m_isGateWay = isGateWay;
}
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::GetIsGateWay()
{
	return m_isGateWay;
}
/////////////////////////////////////////////////////////////////
void CConfAppInfo::SetOperatorConf(WORD isOperatorConf)
{
	m_isOperatorConf = isOperatorConf;
}
/////////////////////////////////////////////////////////////////
WORD CConfAppInfo::IsOperatorConf()
{
	return m_isOperatorConf;
}
/////////////////////////////////////////////////////////////////
bool  CConfAppInfo::IsNeedToBlockIVR()
{
	// EE-462 enable IVR for MFW
//	if (eProductTypeSoftMCUMfw == m_MCUproductType)
//		return true;
	return false;
}
