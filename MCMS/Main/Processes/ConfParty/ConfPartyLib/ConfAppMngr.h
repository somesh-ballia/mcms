#ifndef __CONF_APPLICATIONS_MNGR_H__
#define __CONF_APPLICATIONS_MNGR_H__

// conf type
#define APP_CONF_TYPE_AUDIO              1
#define APP_CONF_TYPE_VIDEO              2

// objections
#define  APP_F_NO_OBJECTION              1
#define  APP_F_OBJECTION                 2

#define WAIT_TO_EVENT                    1
#define NO_WAIT_TO_EVENT                 2

#define EVENT_NOTIFY                     100
#define  EVENT_ACTION                    101

// repeat (loop) for play-message
#define IVR_REPEAT_FOREVER               100000
#define IVR_PLAY_ONCE               	1

// conf or party action
#define CAM_CONF_ACTION                  1
#define CAM_PARTY_ACTION                 2

#define CAM_REPLACE_FIRST_FEATURE        1
#define CAM_APPEND_AFTER_LAST_FEATURE    2
#define CAM_APPEND_AFTER_CURRENT_FEATURE 3
#define CAM_ROLL_CALL_REPLACE_METHOD     4

// search list in participants lists
#define ACTIVE_LIST                      1
#define DISCONNECTED_LIST                2
#define ALL_LISTS                        3

// volume direction
#define CAM_VOLUME_ENCODER               0
#define CAM_VOLUME_DECODER               1

// stop start feature
#define CAM_STOP_FEATURE                 0
#define CAM_START_FEATURE                1

// conf or party layout (for PLC)
#define CAM_PARTY_LAYOUT                 1
#define CAM_CONF_LAYOUT                  2

// Audio or Video
#define CAM_AUDIO                        1
#define CAM_VIDEO                        2

// StartIvrMode
#define START_IVR_CONF_STOPPED           1
#define START_IVR_CONF_SIMPLE_GAINS      2
#define START_IVR_CONF_LOWER_GAINS       3
// for Party
#define START_IVR_PARTY_STOPPED          4
#define START_IVR_PARTY_MESSAGE_GAINS    5
#define START_IVR_PARTY_MUSIC_GAINS      6

// /////////////////////////////////////////////////////////
// enum
// /////////////////////////////////////////////////////////
typedef enum
{
  eAPP_CONF_STATE_MIN                          = 0,
  eAPP_CONF_STATE_CONNECT                      = 1,
  eAPP_CONF_STATE_TERMINATING                  = 2,
  eAPP_CONF_STATE_TERMINATED                   = 3,
  eAPP_CONF_STATE_MAX                          = 4
} TAppConfState;

typedef enum
{
  eAPP_PARTY_STATE_MIN                         = 0,
  eAPP_PARTY_STATE_IDLE                        = 1,
  eAPP_PARTY_STATE_IVR_ENTRY                   = 2,
  eAPP_PARTY_STATE_IVR_FEATURE                 = 3,
  eAPP_PARTY_STATE_CONNECTING_TO_MIX           = 4,
  eAPP_PARTY_STATE_MIX                         = 5,
  eAPP_PARTY_STATE_DISCONNECTED                = 6,
  eAPP_PARTY_STATE_MOVING                      = 7,
  eAPP_PARTY_STATE_DELETED                     = 8,
  eAPP_PARTY_STATE_MAX                         = 9
} TAppPartyState;

typedef enum
{
  eCAM_EVENT_MIN                               = 0,
  eCAM_EVENT_PARTY_END_IVR                     = 1,
  eCAM_EVENT_PARTY_DELETED                     = 2,
  eCAM_EVENT_PARTY_PLAY_MSG_IND                = 4,
  eCAM_EVENT_PARTY_AUDIO_CONNECTED             = 5,
  eCAM_EVENT_PARTY_VIDEO_CONNECTED             = 6,
  eCAM_EVENT_PARTY_VIDEO_IVR_MODE_CONNECTED    = 7,
  eCAM_EVENT_PARTY_AUDIO_MOVE_OUT              = 10,
  eCAM_EVENT_PARTY_VIDEO_MOVE_OUT              = 11,
  eCAM_EVENT_PARTY_AUDIO_DISCONNECTING         = 12,
  eCAM_EVENT_LAST_PARTY_DISCONNECTING          = 13,
  eCAM_EVENT_PARTY_VIDEO_DISCONNECTING         = 14,
  eCAM_EVENT_SET_AS_LEADER                     = 15,
  // events conf
  eCAM_EVENT_CONF_TERMINATING                  = 16,
  eCAM_EVENT_CONF_TERMINATED                   = 17,
  eCAM_EVENT_CONF_ALERT_TONE                   = 18,
  eCAM_EVENT_CONF_CHAIR_DROPPED                = 19,
  eCAM_EVENT_CONF_TERMINATE_UPON_CHAIR_DROPPED = 20,
  eCAM_EVENT_CONF_LOCK                         = 21,
  eCAM_EVENT_CONF_UNLOCK                       = 22,
  eCAM_EVENT_CONF_SECURE                       = 23,
  eCAM_EVENT_CONF_UNSECURE                     = 24,
  eCAM_EVENT_CONF_MUTE_ALL                     = 25,
  eCAM_EVENT_CONF_UNMUTE_ALL                   = 26,
  eCAM_EVENT_CONF_MUTE_INCOMING_PARTIES        = 27,
  eCAM_EVENT_CONF_ROLL_CALL_REVIEW             = 28,
  eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW         = 29,
  eCAM_EVENT_CONF_EXIT_TONE                    = 30,
  eCAM_EVENT_CONF_ENTRY_TONE                   = 31,
  eCAM_EVENT_CONF_WAIT_FOR_CHAIR               = 32,
  eCAM_EVENT_CONF_END_WAIT_FOR_CHAIR           = 33,
  eCAM_EVENT_CONF_ONHOLD                       = 34,
  eCAM_EVENT_CONF_END_ONHOLD                   = 35,
  eCAM_EVENT_CONF_NOISY_LINE_DETECTION         = 36,
  // events party
  eCAM_EVENT_PARTY_FIRST_TO_JOIN               = 37,
  eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC          = 38,
  eCAM_EVENT_PARTY_END_SINGLE_PARTY_MUSIC      = 39,
  eCAM_EVENT_PARTY_WAIT_FOR_CHAIR              = 40,
  eCAM_EVENT_PARTY_END_ALL_WAIT_FOR_CHAIR      = 41,
  eCAM_EVENT_PARTY_PARTY_ROLL_CALL_REVIEW      = 42,
  eCAM_EVENT_PARTY_END_ROLL_CALL_REVIEW        = 43,
  eCAM_EVENT_PARTY_CHANGE_PW                   = 44,
  eCAM_EVENT_PARTY_SILENCE_IT                  = 45,
  eCAM_EVENT_PARTY_CHANGE_TO_LEADER            = 46,
  eCAM_EVENT_PARTY_PLAY_MENU                   = 47,
  eCAM_EVENT_PARTY_MUTE                        = 48,
  eCAM_EVENT_PARTY_UNMUTE                      = 49,
  eCAM_EVENT_PARTY_OVERRIDE_MUTEALL            = 50,
  eCAM_EVENT_PARTY_PLC                         = 51,
  eCAM_EVENT_PARTY_CANCEL_PLC                  = 52,
  eCAM_EVENT_PARTY_CHANGE_LAYOUT_PLC           = 53,
  eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_PLC      = 54,
  eCAM_EVENT_PARTY_FORCE_PLC                   = 55,
  eCAM_EVENT_PARTY_INC_VOLUME                  = 56,
  eCAM_EVENT_PARTY_DEC_VOLUME                  = 57,
  eCAM_EVENT_PARTY_START_IVR                   = 58,
  eCAM_EVENT_PARTY_SHOW_SLIDE                  = 59,
  eCAM_EVENT_PARTY_JOIN_CONF_VIDEO             = 60,
  eCAM_EVENT_PARTY_END_VIDEO_IVR               = 61,
  eCAM_EVENT_PARTY_IN_CONF_IND                 = 62,
  eCAM_EVENT_PARTY_IS_CASCADE_LINK             = 63,
  // internal party events
  eCAM_EVENT_PARTY_END_FEATURE                 = 64,
  eCAM_EVENT_PARTY_IVR_ENTRANCE                = 65,
  eCAM_EVENT_PARTY_END_LAST_FEATURE            = 66,
  eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE          = 67,
  eCAM_EVENT_ADD_FEATURE_TO_WAIT_LIST          = 68,
  // internal general event
  eCAM_EVENT_PARTY_VIDEO_END_SHOW_SLIDE        = 69,
  eCAM_EVENT_TIMER_END_FEATURE                 = 70,
  eCAM_EVENT_PARTY_ROLL_CALL_YESNO             = 71,
  eCAM_EVENT_PARTY_SEND_DTMF                   = 72,
  eCAM_EVENT_PARTY_RECORDING_FAILED            = 73,
  eCAM_EVENT_CONF_RECORDING_FAILED             = 74,
  eCAM_EVENT_PARTY_RECORDING_IN_PROGRESS       = 75,
  eIVR_SHOW_PARTICIPANTS                       = 76,
  e_ACK_IND                                    = 77,
  eCAM_EVENT_CONF_DTMF_FORWARDING              = 78,
  eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE         = 79,
  eCAM_EVENT_PARTY_UPDATE_GW_TYPE              = 80,
  eCAM_EVENT_PARTY_PLAY_RINGING_TONE           = 81,
  eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE      = 82,
  eIVR_SHOW_GATHERING                          = 83,
  eCAM_EVENT_PARTY_CLIENT_SEND_DTMF            = 84,
  eIVR_REQUEST_TO_SPEAK                        = 85,
  // max event (illegal)
  eCAM_EVENT_PARTY_VENUS                       = 86,
  eCAM_EVENT_PARTY_CANCEL_VENUS                = 87,
  eCAM_EVENT_PARTY_CHANGE_LAYOUT_VENUS         = 88,
  eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_VENUS    = 89,
  eCAM_EVENT_PARTY_FORCE_VENUS                 = 90,
  eCAM_EVENT_PARTY_IS_TIP_SLAVE                = 91,
  eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK         = 92,

  // invite Party Feature
  eCAM_EVENT_INVITE_PARTY                      = 100,
  eCAM_EVENT_PARTY_PLAY_BUSY_MSG               = 101,
  eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG           = 102,
  eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG       = 103,
  eCAM_EVENT_CONF_DISCONNECT_INVITED_PARTICIPANT     = 104,

  // bridge-1811
  eCAM_EVENT_SET_AS_LEADER_FROM_API              = 105,

  // Recording Link opcodes
  eCAM_EVENT_CONF_START_RESUME_RECORDING       = 126,
  eCAM_EVENT_CONF_STOP_RECORDING               = 127,
  eCAM_EVENT_CONF_PAUSE_RECORDING              = 128,

  //AT&T
  eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL       = 135,
  eCAM_EVENT_PARTY_REMOVE_FEATURE              = 136,

  //AT&T local IVR for TIP
  eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER       = 137,
  eCAM_EVENT_PARTY_REMOVE_PLAY_MESSAGE_FROM_TIP_MASTER              = 138,

  //support for hold/resume call
  eCAM_EVENT_PARTY_ON_HOLD              = 138,

  ECAM_EVENT_MAX
} TConfAppEvents;

typedef enum
{
  NOISE_ENERGY_MIN                             = 0,
  NOISE_ENERGY_LOW                             = 1,
  NOISE_ENERGY_MIDDLE                          = 2,
  NOISE_ENERGY_HIGH                            = 3,
  NOISE_ENERGY_MAX                             = 4
} TNoiseEnergyLevel;

typedef enum
{
  ePcmNone                                     = 0,
  ePcmForEveryone                              = 1,
  ePcmForChairpersonOnly                       = 2,
  ePcmLevelLast                                = 3
} ePcmAutorizationLevel;


// /////////////////////////////////////////////////////////
// include files
// /////////////////////////////////////////////////////////
#include "StateMachine.h"
#include "ConfAppPartiesList.h"
#include "ConfAppWaitEventsList.h"
#include "ConfAppActiveEventsList.h"
#include "IvrApiStructures.h"
#include "ConfAppInfo.h"

// /////////////////////////////////////////////////////////
// classes declaration
// /////////////////////////////////////////////////////////
class CConfAppPartiesList;
class CConfAppWaitEventsList;
class CConfAppActiveEventsList;
class CConfAppMngrInitParams;
class CTaskApi;
class CAudioBridgeInterface;
class CVideoBridgeInterface;
class CSegment;
class CBridgePartyInitParams;
class CBridgePartyExportParams;
class CBridgePartyDisconnectParams;
class CConfApi;

////////////////////////////////////////////////////////////////////////////
//                        CConfAppMngr
////////////////////////////////////////////////////////////////////////////
class CConfAppMngr : public CStateMachine
{
	CLASS_TYPE_1(CConfAppMngr, CStateMachine)

public:
	                    CConfAppMngr();
	virtual            ~CConfAppMngr();
	virtual const char* NameOf() const   { return "CConfAppMngr"; }

	virtual void*       GetMessageMap();
	virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	void                Create(const CConfAppMngrInitParams* pConfAppMngrInitParams);
	CConfAppInfo*       GetConfAppInfo() { return m_confAppInfo; }

	// action functions
	void                HandleNotifyEvents(CSegment* pParam);
	void                OnActionActionEvents(CSegment* pParam);
	void                CAMNullActionFunction(CSegment* pParam);
	void                ConnectPartyAudio(CBridgePartyInitParams* pBrdgPartyInitParams, bool isIvrOn = true);
	void                ConnectPartyVideo(CBridgePartyInitParams* pBrdgPartyInitParams);
	void                DisconnectPartyAudio(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams);
	void                DisconnectPartyVideo(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams);
	void                ExportPartyAudio(CBridgePartyExportParams* pBridgePartyExportParams);
	void                ExportPartyVideo(CBridgePartyExportParams* pBridgePartyExportParams);
	void                RemovePartyFromCAM(PartyRsrcID partyId, const char* partyName);
	DWORD               GetPartyToForce(PartyRsrcID partyId);
	bool                GetIsPartyCascadeLink(PartyRsrcID partyId);
	void                MuteAllButLecturer(PartyRsrcID partyId, EOnOff onOff);
	void                HandleRecordingControlStatus(BYTE status);

protected:
	// Events
	void                DoPartyRequest(CSegment* pParam);
	void                DoConfRequest(CSegment* pParam);
	void                DoCAMRequest(DWORD confOrParty, DWORD eventUniqueNumber, TConfAppEvents feature, PartyRsrcID partyId);
	void                DoPlayMessage(CSegment* pParam);
	void                DoStopMessage(CSegment* pParam);
	void                DoRecordRollCall(CSegment* pParam);
	void                DoUpdatePartyNoiseDetection(CSegment* pParam);
	void                OnRejectPLC(PartyRsrcID partyId, CSegment* pParam);
	void                DoPartyEndIVRSession(DWORD opcode, PartyRsrcID partyId, DWORD origUniqueEventNumber, CSegment* pParam);
	void                DoPartyOutOfConf(DWORD opcode, PartyRsrcID partyId, DWORD origUniqueEventNumber, CSegment* pParam, WORD firstYesNo);
	DWORD               IsSlideNeeded(PartyRsrcID partyId);
	DWORD               IsSlideNeeded(const char* partyName);
	bool                IsPartyWithIVR(const char* partyName, BOOL isTipResumeCall = FALSE);
	void                OnNoiseDetectionAlarm(CSegment* pParam);
	const char*         CAM_NotifyOpcodeToString(DWORD opcode);
	const char*         CAM_ActionOpcodeToString(DWORD opcode);
	void                UpdateConfPartyDB(PartyRsrcID partyId, BOOL isIvr);
	void                DoSetPartyAsLeader(PartyRsrcID partyId, char* partyName, CSegment* pParam);
	void                DoSendDtmf(PartyRsrcID partyId, CSegment* pParam);
	void                OnNotifyPartyReadyForSlide(PartyRsrcID partyId);
	BOOL                ShouldStartRecordingImm(PartyRsrcID partyId);
	void                OnTimerStartRecording(CSegment* pParam);
	void                OnTimerDisconnectRL(CSegment* pParam);
	void                OnTimerSendStartDtmf(CSegment* pParam);
	bool                IsRecordingLinkParty(PartyRsrcID partyId, char* partyName = NULL);
	bool                IsPartyDialin(PartyRsrcID partyId, char* partyName = NULL);
	void                DoRecordingAction(TConfAppEvents opcode);
	void                SendControlRecordingCDR(WORD recodingCommand, PartyRsrcID partyId);
	void                UpdateRecordingControl(eRecordingControl RecodingCommand);
	WORD                CheckStageOfRecordingLinkConnection();
	WORD                CheckStageOfRecordingLinkConnectionForDialin();
	void                UpdatesUponRlDisconnection(PartyRsrcID partyId, WORD recFailedFlag, WORD recUnexpectedDiscon = NO);
	void                PauseRecordingDuringStarted();
	void                StopRecordingDuringStarted();
	void                OnRecordConfFailure();
	void                SetOldChairAsRegularPartyIfNeeded(PartyRsrcID newChairPartyId, BYTE isLeader = TRUE);
	void                SetPcmAutorizationLevel();
	void                InformPcmPartyAdded(PartyRsrcID partyId);
	void                InformPcmPartyLeft(PartyRsrcID partyId);
	bool                IsNeedToDoSomethingWithEndIVROrPartyEvent(PartyRsrcID partyId); //IVR for TIP
	void                UpdateFirstPartyMessagePlayedIfNeeded();
	BOOL                IsPartyNeedToBeMuted(PartyRsrcID partyId);
	void                OnTimerPrintLoggerNumber(CSegment* pParam);
	void                SendRecordingControlToRL(PartyRsrcID partyId, eRecordingControl recodingCommand);
	void                DoSendDtmfViaSignaling(PartyRsrcID partyId, CSegment* pParam);
	void                OnTimerRecordingControlTOUT(CSegment* pParam);
	// vngfe-8707
    void 				DoStopRollCallRecording(CSegment* pParam);
    void				OnPartyStopRollCallRecordingAck(DWORD PartyId,DWORD status);
    void 				DoStopRollCallRecordingAckTimer(CSegment* pParam);
    void 				ContinueDisconnectPartyAudio(DWORD PartyId);


protected:
	CConfAppPartiesList*      m_partiesList;
	CConfAppWaitEventsList*   m_waitList;
	CConfAppActiveEventsList* m_eventsList;
	CConfAppInfo*             m_confAppInfo;
	ePcmAutorizationLevel     m_pcmAutorizationLevel; // determines which party can activate pcm in conf

	PDECLAR_MESSAGE_MAP
};

#endif  // __CONF_APPLICATIONS_MNGR_H__
