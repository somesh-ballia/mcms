//+========================================================================+
//                        ConfApplicationsInfo.H                       |
//					      Copyright 2005 Polycom                           |
//                        All Rights Reserved.                             |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ConfAppInfo.H							                   |
// SUBSYSTEM:  MCMS                                                        |
//+========================================================================+

#ifndef __CONF_APP_INFO_H__
#define __CONF_APP_INFO_H__




///////////////////////////////////////////////////////////
// include files
///////////////////////////////////////////////////////////
#include "StateMachine.h"
#include "ConfAppPartiesList.h"
#include "ConfAppWaitEventsList.h"
#include "ConfAppActiveEventsList.h"
#include "IvrApiStructures.h"
#include "ConfPartyDefines.h"
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////
// classes declaration
///////////////////////////////////////////////////////////
class	CConfAppPartiesList;
class	CConfAppWaitEventsList;
class	CConfAppActiveEventsList;
class	CConfAppMngrInitParams;
class	CTaskApi;
class	CAudioBridgeInterface;
class	CVideoBridgeInterface;
class	CSegment;
class 	CBridgePartyInitParams;
class 	CBridgePartyExportParams;
class 	CBridgePartyDisconnectParams;
class	CConfApi;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
class CConfAppInfo : public CPObject
{
	CLASS_TYPE_1(CConfAppInfo,CPObject)

public:
	CConfAppInfo();
	~CConfAppInfo();
	virtual const char* NameOf() const { return "CConfAppInfo";}
	void Create( const CConfAppMngrInitParams* pConfAppMngrInitParams );

	const char* GetStringFromOpcode( TConfAppEvents eOpcode);
	const char* GetCamString();	

	// first to join
	int	 IsFirstPartyMessagePlayed();
	void SetFirstPartyMessagePlayed( WORD action );
	// active conf counter
	void IncConfActiveEventsCounter();
	void DecConfActiveEventsCounter();
	WORD GetConfActiveEventsCounter();
	void SetConfActiveEventsCounter(WORD numOfActiveEvents);
	// active conf exit tone counter
	void IncConfActiveExitToneCounter();
	void DecConfActiveExitToneCounter();
	WORD GetConfActiveExitToneCounter();
	// isConfInIVR
	WORD GetConfStartIVRMode();
	void SetConfStartIVRMode( WORD confStartIvrMode);
	// conf state
	void SetConfState( TAppConfState confState );
	WORD GetConfState();
	// conf attributes
	WORD GetConfWithIVR();
	// conf type (audio, video)
	void SetConfAudioVideoType( WORD confType );
	WORD GetConfAudioVideoType();
	// ivrServiceName
	void SetIvrServiceName( const char* ivrName );
	const char* GetIvrServiceName();
	// wait for chair feature
	void SetWaitForChair( WORD waitForChairYesNo );
	WORD IsWaitForChair();
	// in wait for chair Now
	void SetInWaitForChairNow( WORD inWaitForChairNow, BOOL onDisconnect = FALSE);
	WORD IsInWaitForChairNow();
	// has the conference ended its 'wait for chair' stage
	void SetIsConfPastWaitForChairStage( WORD isConfPastWaitForChairStage );
	WORD IsConfPastWaitForChairStage();
	// leader in conf
	void SetLeaderInConfNow( WORD isChairInConf );
	WORD GetIsLeaderInConfNow();
	// Single Party Now
	void SetSinglePartyNow( WORD isSinglePartyNow );
	WORD GetSinglePartyNow();
	// RingTone is on/off
	void SetIsRingToneOn( WORD isRingToneOn );
	WORD GetIsRingToneOn();	
	// Single Party Resource ID
	void SetSinglePartyRsrcId( DWORD singlePartyRsrcId );
	DWORD GetSinglePartyRsrcId();
	// terminate after chair exists
	void SetTerminateAfterChairExits( WORD terminateAfterChairExits );
	WORD GetTerminateAfterChairExits();
	// isEntryQueue
	void SetIsEntryQueue( WORD isEntryQueue );
	WORD GetIsEntryQueue();
	// Conf Name
	const char* GetConfName();
	// Conf Resource ID
	DWORD GetConfRsrcID() {return m_confRsrcId;}
	// isInMuteIncomingParties
	void SetIsInMuteIncomingParties( WORD isInMuteIncomingParties );
	WORD GetIsInMuteIncomingParties();
	// isEnableNoisyLineDetection
	void SetIsEnableNoisyLineDetection( EOnOff isEnableNoisyLineDetection );
	EOnOff GetIsEnableNoisyLineDetection();
	// defualtNoisyLineThresholdLevel
	void SetDefualtNoisyLineThresholdLevel( BYTE defualtNoisyLineThresholdLevel );
	BYTE GetDefualtNoisyLineThresholdLevel();
	// enable / disable RollCall announcement during the conf
	void SetRollCallAnnounceYesNo( DWORD yesNo );
	BYTE GetRollCallAnnounceYesNo();
	// get CascadeEQ
	BYTE IsCascadeEQ();
	
	// Enable Recording
	void SetEnableRecording(WORD enableRecording);
	WORD GetEnableRecording();
  void SetEnableRecordingIcon(WORD enableRecordingIcon);
  WORD GetEnableRecordingIcon();
  void SetEnableRecordingNotify(WORD enableRecordingNotify);
  WORD GetEnableRecordingNotify();
  
	// Start Recording Policy
	void SetStartRecordingPolicy(WORD startRecordingPolicy);
	WORD GetStartRecordingPolicy();
	// Recording Failed Flag
	void SetRecordingFailedFlag(WORD recordingFailed);
	WORD GetRecordingFailedFlag();
	// Recording Link In Conf
	void SetRecordingLinkInConf(WORD recordingLinkInConf);
	WORD GetRecordingLinkInConf();
	// Recording Link RsrcId
	void SetRecordingLinkRsrcId(DWORD partyRsrcID);
	DWORD GetRecordingLinkRsrcId();
	// Start Recording Initiator RsrcId
	void SetRecordingInitiatorId(DWORD partyRsrcID);
	DWORD GetRecordingInitiatorId();
	// Start Recording Dtmf Was Sent (*2 was already sent to RSS)
	void SetStartRecDtmfWasSent(WORD startRecDtmfWasSent);
	WORD GetStartRecDtmfWasSent();
	// Last Reqested Recoring Action
	void SetLastReqRecAction(WORD lastReqRecAction);
	WORD GetLastReqRecAction();
	// Recording Link Disconnection Was Received
	void SetRlDisconnectionWasReceived(WORD rlDisconnectionWasReceived);
	WORD GetRlDisconnectionWasReceived();
	
	WORD GetRecordingStatus();
	
	// get DTMF code permission
	WORD GetIvrPermission( WORD wEventOpCode );
	
	void SetIvrMessageVolume(DWORD ivrVolume);
	DWORD GetIvrMessageVolume();
	void SetIvrMusicVolume(DWORD ivrVolume);
	DWORD GetIvrMusicVolume();
	void SetIvrRollCallVolume(DWORD ivrVolume);
	DWORD GetIvrRollCallVolume();
	DWORD GetCachePriority( DWORD op_code, DWORD mediaType );
	DWORD GetMCUproductType() { return m_MCUproductType;}
	bool  IsNeedToBlockIVR();
	
	// Gateway
	void SetIsGateWay(WORD isGateWay);
	WORD GetIsGateWay();

        // OperatorConf	
        void SetOperatorConf(WORD isOperatorConf);
        WORD IsOperatorConf();
    void SetIsExternalIVRInConf(BYTE isExternalIvr);
	BYTE GetIsExternalIVRInConf();
	BOOL IsRollCallToneInsteadVoice();
  //	void SetIsToneInsteadVoice(BOOL ivrToneInsteadVoice);
	void SetIsMuteAllAudioButLeader(BOOL isMuteAllAudioButLader);
	BOOL GetIsMuteAllAudioButLeader ();
	// for trace
	DWORD GetLastPrintedLoggerNumber() {return m_loggerConfLastReport;}
	void  SetLastPrintedLoggerNumber( DWORD loggerConfLastReport) {m_loggerConfLastReport = loggerConfLastReport;}
	DWORD GetloggerConfStartedNumber() {return m_loggerConfStarted;}
	void  SetloggerConfStartedNumber( DWORD loggerConfStarted) {m_loggerConfStarted = loggerConfStarted;}
	//eFeatureRssDialin
	WORD	GetIsRecordingControlInProgress() {return m_recordingControlInProgress;}
	void 	SetIsRecordingControlInProgress(WORD  recordingControlInProg = 0) {m_recordingControlInProgress = recordingControlInProg;}
	eRecordingControl	GetRecordingControlActionSaved() {return m_recordingControlActionSaved;}
	void 	SetRecordingControlActionSaved(eRecordingControl   recordingControlAction = eStopRecording) {m_recordingControlActionSaved = recordingControlAction;}
	

	// for internal logic
	
//	CConf* GetConf () { return m_pConf;}
protected:
	void UpdateNoisyLineParams();
	void UpdateIsEntryQueue();
	
	
public:
	TAppConfState	m_confState;
	WORD			m_firstPartyMessagePlayed;
	WORD			m_confActiveEventsCounter;
	WORD			m_confActiveExitToneCounter;
	WORD			m_confStartIvrMode;		// indicates whether START IVR was sent or not and in which mode 
	WORD			m_confAudioVideoType;
	WORD			m_isSinglePartyNow;
	WORD			m_isRingToneOn;
	DWORD			m_singlePartyRsrcId;
	WORD			m_confWithIVR;
	WORD			m_isWaitForChair;		// Is the feature on in the conf (set off when chair enters the conf) 
	WORD			m_isTerminateConfAfterChairDropped;
	WORD			m_inWaitForChairNow;	// Is the conf in wait for chair state now
	WORD			m_isConfPastWaitForChairStage;	// Is the conf in wait for chair state now
	const char*		m_ivrName;
	WORD 			m_isEntryQueue;
	WORD			m_isInMuteIncomingParties;
	EOnOff			m_isEnableNoisyLineDetection;
	BYTE			m_defualtNoisyLineThresholdLevel;
	BOOL			m_isMuteAllAudioButLeader;

	// for outside interface
public:
//	CTaskApi*				m_pConfApi;
	CConfApi*				m_pConfApi;
	CAudioBridgeInterface*	m_pAudBrdgInterface;
	CVideoBridgeInterface*	m_pVideoBridgeInterface;

protected:
	const char* m_pConfName;
	DWORD	m_confRsrcId;
	WORD	m_isChairInConf;
	WORD	m_terminateAfterChairExits;
	BYTE	m_rollCallAnnounceYesNo;
	BYTE    m_isCascadeEQ;
	
	WORD	m_enableRecording;
	WORD    m_enableRecordingIcon;
	WORD	m_enableRecNotify;
	WORD 	m_startRecordingPolicy;
	WORD 	m_recordingFailedFlag;
	WORD	m_recordingLinkInConf;
	DWORD 	m_recordingLinkRsrcId;
	DWORD 	m_recordingInitiatorId;
	WORD	m_startRecDtmfWasSent;
	WORD	m_lastReqRecAction;
	WORD	m_rlDisconnectionWasReceived;
	DWORD 	m_ivrMessageVolume;
	DWORD 	m_ivrMusicVolume;
	DWORD 	m_ivrRollCallVolume;

	WORD 	m_isGateWay;
    WORD 	m_isOperatorConf;
    BYTE    m_isExternalIVRInConf;
    DWORD   m_MCUproductType;
//	CConf*  m_pConf;
    DWORD	m_loggerConfStarted;
    DWORD	m_loggerConfLastReport;
    WORD		m_recordingControlInProgress;
    eRecordingControl		m_recordingControlActionSaved;
};


#endif	// __CONF_APP_INFO_H__
