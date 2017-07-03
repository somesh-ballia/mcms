#ifndef _IVRFEATURES_H__
#define _IVRFEATURES_H__

#include "ConfPartyDefines.h"
#include "StateMachine.h"
#include "ConfPartyOpcodes.h"
//#include "IVRCntl.h"
#include "CommConf.h"
#include "ConfPartyStructs.h"
#include "VideoDefines.h"

// Message Mode
typedef enum
{
  IVR_STATUS_DUMMY       = 0,
  IVR_STATUS_PLAY_ONCE   = 1,
  IVR_STATUS_PLAY_LOOP   = 2,
  IVR_STATUS_PLAY_RECORD = 3,
  IVR_STATUS_RECORD      = 4,
  IVR_STATUS_MAX         = 5
} TIvrStatus, * pTIvrStatus;
#define MAX_DTMF_STRING_LENGTH       32
#define MAX_PASSWORD_LEN             32

#define DISCONNECT_ON_ERROR          0

#define DISCONNECT_IVR_PROVIDER_STS  999 // IvrProviderEQ

#define STATE_ENTER                  0
#define STATE_CODE                   1

#define MSG_WELCOME_TYPE             0
#define MSG_ENTRANCE_TYPE            1

// ------- general types definition ----------
#define GEN_TP_LOCK_SECURE           1
#define GEN_TP_CHANGE_PWDS_MENU      2
#define GEN_TP_CHANGE_CONF_PWD       3
#define GEN_TP_CHANGE_LEADER_PWD     4
#define GEN_TP_CHANGE_PWDS_CONFIRM   5
#define GEN_TP_CHANGE_PWDS_OK        6
#define GEN_TP_CHANGE_PWDS_INVALID   7
#define GEN_TP_MAX_PARTICIPANTS      8
#define GEN_TP_RECORDING_IN_PROGRESS 9
#define GEN_TP_RECORDING_FAILED      10
#define GEN_TP_INVITE_PARTY          11
#define GEN_TP_INVITE_PARTY_ADDR     12
#define GEN_TP_GW_REINVITE_PARTY     13
#define GEN_TP_DTMF_REINVITE_PARTY   14
#define GEN_TP_PLAY_BUSY_MSG         15
#define GEN_TP_PLAY_NOANSWER_MSG     16
#define GEN_TP_PLAY_WRONG_NUMBER_MSG 17

#define DTMF_OK               0
#define DTMF_TIMEOUT_ERROR    1
#define DTMF_STRING_ERROR     2
#define DTMF_IGNORE_ERROR     3
#define DTMF_TIMEOUT_EXTERNAL_IVR_TIMER    4
#define TOO_MANY_ERRORS       17
// ===== classes =====
class CDtmfCollector;
class CParty;
class CPartyApi;
class CConfApi;
class CDTMFCodeList;
class CIVRService;
class CIVRFeature;
class CIVRConfPasswordFeature;
class CIVRLangMenuFeature;
class CIVRWelcomeFeature;
class CIVROperAssistanceFeature;
class CIVRConfLeaderFeature;
class CIVR_BillingCodeFeature;
class CIVRRollCallFeature;
class CIVRNumericConferenceIdFeature;
class CIVRMuteNoisyLineFeature;
class CIVRRecordingFeature;
class CIVRPlaybackFeature;
class CIVRInvitePartyFeature;
class CVCFeatureAPI;
class CIvrCntl;
class CCommRes;

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubBaseSM
////////////////////////////////////////////////////////////////////////////
class CIvrSubBaseSM : public CStateMachine
{
	CLASS_TYPE_1(CIvrSubBaseSM, CStateMachine)

public:
	                    CIvrSubBaseSM();
	                   ~CIvrSubBaseSM();

	virtual const char* NameOf() const { return "CIvrSubBaseSM";}
	virtual void        HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	virtual void*       GetMessageMap();
	virtual WORD        DoSomething();

	// pure virtual functions
	virtual void        Start()                         = 0;
	virtual int         IsLegalString(CSegment* pParam) = 0;
	virtual bool        IsDTMFWODelimeterEnabled() {return false;}

	void                StopPlayTone(void);
	virtual void        OnDTMF() { }
	WORD                IsActive();

	virtual void        PlayMessage(DWORD messageMode, WORD loop_delay = 30);
	virtual void        StopPlayMessage();
	virtual void        PlayRetryMessage(DWORD messageMode) = 0;
	void                SetDtmfP(CDtmfCollector* pDtmf){ m_pDtmf = pDtmf;}
	virtual void        EndFeature(WORD status);
	virtual void        EndFeatureError(WORD status);
	void                StartBase(WORD bRetry = FALSE, DWORD messageMode = IVR_STATUS_PLAY_LOOP, WORD loop_delay = 30);
	void                SetIvrCntl(CIvrCntl* ivrCntl);
	void                SetLanguage(WORD lang);
	void                CreatePartyApi(CParty* pParty, CConfApi* pConfApi);
	virtual int         GetDtmfResult(CSegment* pParam, WORD& opcode, WORD& rDigits);
	void                SetIvrService(CIVRService* pIvrService);
	void                PlayOtherMessage(DWORD messageMode, WORD feature_op_code, const char* file_name);
	void                SendCode(WORD opcode);
	virtual void        OnEndFeature(WORD status);
	int                 IsIvrPtrs();
	WORD                GetMsgDuration(const CIVRFeature* feature, const WORD event_opcode);
	char                GetIvrDelimiter(char defDelimiter);
	void                StartBaseTimer(void);
	int                 GetDebugMsg(WORD event);
	void                ParsePreDefinedIvrString(const char* szPreDefinedIvrString);
	void                CdrPartyDtmfFailureIndication(const char* correctData);

	// action functions
	void                OnDtmfString(CSegment* pParam);
	void                OnIvrSubBaseNull(CSegment* pParam);
	void                OnTimeRetryMessage(CSegment* pParam);

	int                 Get_debug_message_use(void);
	void                TraceInt(WORD trace_flag, const char* str, int val);

	void                ReqPwdConfirmation(WORD reqType, const char* numericConfId,
	                                       DWORD partyId, const char* pwd, Phone* pPhone1, Phone* pPhone2,
	                                       BYTE isLeader, BYTE isGuest, const char* protocolTypeStr = NULL, IP_EXT_DB_STRINGS* ipStringsStruct = NULL);
	void                GetTypeName(char* typeName, WORD reqType);
	virtual int  GetIVRMessageParameters( char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum );
	void                FillIpIdentifiers(CConfParty* pConfParty, IP_EXT_DB_STRINGS* ipStringsStruct);
	virtual void        OnStopIvr();
	WORD                GetCachePriority(WORD feature_opcode, WORD event_opcode);
	void                ReqConfNIDConfirmationSIP(const char* numericConfId); // ivrProviderEQ
	void                OnTimerDiscIvrProviderExtDB(); // ivrProviderEQ
	WORD                getEventOpCode(){return m_event_opcode;}

	std::string         GetDialogID() { return m_dialogID; }
	void                SetAllowDtmfBargeIn(BOOL isDtmfBargeInAllowed) { m_bAllowDtmfBargeIn = isDtmfBargeInAllowed; }
	BOOL                GetAllowDtmfBargeIn() { return m_bAllowDtmfBargeIn;}
	bool                IsDTMFWODelimeterFlagEnabled();

public:

protected:
	CDtmfCollector*     m_pDtmf;
	CIvrCntl*           m_ivrCntl;
	CPartyApi*          m_pPartyApi;
	CParty*             m_pParty;
	CConfApi*           m_pConfApi;

	WORD                m_feature_opcode;
	WORD                m_event_opcode;
	char                m_msg_file_name[NEW_FILE_NAME_LEN]; // temp

	// IVR Service info
	DWORD               m_errorCounter;
	DWORD               m_timeoutCounter;
	DWORD               m_maxRetryTimes;

	DWORD               m_DTMF_digits;
	DWORD               m_DTMF_timeout;

	WORD                m_language;
	WORD                m_retryMessageDuration;

	BYTE                m_dtmf[MAX_DTMF_STRING_LENGTH+1];

	CIVRService*        m_pIvrService;
	char                m_pinCodeDelimiter;
	char*               m_messagesName;

	WORD                m_msgDuration;
	WORD                m_bStopUponDTMF;
	WORD                m_timerRetryNum;
	WORD                m_initTimer;

	std::string         m_dialogID; //AT&T

	char                m_passwordForCascadeLink [MAX_DTMF_STRING_LENGTH];
	char                m_numericIdForCascadeLink[MAX_DTMF_STRING_LENGTH];

	BOOL                m_bEnableExternalDB;
	BOOL                m_bAllowDtmfBargeIn;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubConfPassword
////////////////////////////////////////////////////////////////////////////
class CIvrSubConfPassword : public CIvrSubBaseSM
{
public:
	                    CIvrSubConfPassword(const CIVRConfPasswordFeature* pConfPasswordFeature, const char* pConfPassword, BOOL bEnableExternalDB);
	                   ~CIvrSubConfPassword();

	virtual const char* NameOf() const { return "CIvrSubConfPassword";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);

	WORD                DoSomething();
	DWORD               FindConfOrMeetingRoomAccordingPassw(const char* szPassw, BYTE& wParamOutIsMeetingRoom);
	void                OnTimeNoExtDBResponse(CSegment* pParam);
	int                 OnExtDBResponse(CSegment* pParam);
	void                OnExtDBFailure(CSegment* pParam);
	bool                IsDTMFWODelimeterEnabled();

protected:
	char                     m_pConfPassword[MAX_PASSWORD_LEN+1];
	CIVRConfPasswordFeature* m_pConfPasswordFeature;
	int                      m_internalState;
	BYTE                     m_useExternalDB;
	BOOL                     m_skipPrompt;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubWelcome
////////////////////////////////////////////////////////////////////////////
class CIvrSubWelcome : public CIvrSubBaseSM
{
public:
	                    CIvrSubWelcome(WORD messageType);
	                   ~CIvrSubWelcome();

	virtual const char* NameOf() const { return "CIvrSubWelcome";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);

	// action functions
	void                ContinueWelcome(CSegment* pParam);
	void                OnDTMFReceivedACTIVE(CSegment* pParam);

protected:
	WORD                m_messageType;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubLeader
////////////////////////////////////////////////////////////////////////////
class CIvrSubLeader : public CIvrSubBaseSM
{
public:
	                    CIvrSubLeader(const CIVRConfLeaderFeature* leader, BOOL bEnableExternalDB, BOOL isUserInitiated = FALSE);
	                   ~CIvrSubLeader();

	virtual const char* NameOf() const { return "CIvrSubLeader";}
	virtual void*       GetMessageMap();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        Start();
	virtual void        PlayRetryMessage(DWORD messageMode);
	WORD                DoSomething();
	bool                IsDTMFWODelimeterEnabled();

	// action functions
	void                OnDtmfString(CSegment* pParam);

protected:
	void                StartInternal(WORD digits, WORD op_msg);
	void                EndNotLeader();
	void                OnTimeNoExtDBResponse(CSegment* pParam);
	int                 OnExtDBResponse(CSegment* pParam);
	void                OnExtDBFailure(CSegment* pParam);

protected:
	CIVRConfLeaderFeature* m_pLeaderFeature;
	const char*            m_pConfLeaderPassword;
	int                    m_internalState;
	char                   m_leaderEnterDtmf;
	WORD                   m_leaderIdentError;
	BYTE                   m_useExternalDB;
	BOOL                   m_isUserInitiated;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubBillingCode
////////////////////////////////////////////////////////////////////////////
class CIvrSubBillingCode : public CIvrSubBaseSM
{
public:
	                    CIvrSubBillingCode(const CIVR_BillingCodeFeature* pBillingCodeFeature);
	                   ~CIvrSubBillingCode();

	virtual const char* NameOf() const { return "CIvrSubBillingCode";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);

	WORD                DoSomething();

protected:
	char                     m_pBillingCode[MAX_PASSWORD_LEN+1];
	CIVR_BillingCodeFeature* m_pBillingCodeFeature;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubGeneral
////////////////////////////////////////////////////////////////////////////
class CIvrSubGeneral : public CIvrSubBaseSM
{
public:
	                    CIvrSubGeneral();
	                    CIvrSubGeneral(WORD type);
	                   ~CIvrSubGeneral();
	virtual const char* NameOf() const { return "CIvrSubGeneral";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);
	WORD                DoSomething();
	int                 CheckIfIvrMsgsExistInFeature();
	virtual void        PlayMessage(DWORD messageMode, WORD loop_delay = 30);
	void                PlayGwReinviteMessage(DWORD messageMode, WORD loop_delay);

protected:
	STATUS              CheckLeagalPW();

protected:
	WORD                m_general_type;
	WORD                m_retry_opcode;
	BYTE                m_error;
	BYTE                m_need_to_stop_previous_msg;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubRollCall
////////////////////////////////////////////////////////////////////////////
class CIvrSubRollCall : public CIvrSubBaseSM
{
public:
	                    CIvrSubRollCall(const CIVRRollCallFeature* pRollCall);
	virtual            ~CIvrSubRollCall();

	virtual const char* NameOf() const { return "CIvrSubRollCall";}
	virtual void*       GetMessageMap();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        Start();
	virtual void        PlayRetryMessage(DWORD messageMode);
	void                PlayRecordMessage(CSegment* pParam);
	void                OnDtmfPlayRecordMessage(CSegment* pParam);
	void                OnPartyMessageRecorded(CSegment* pParam);
	void                OnTimerEndRollCallActive(CSegment* pParam);
	void                OnTimerEndRollCallRecording(CSegment* pParam);
	void                OnDtmfStopRecordingByDtmf(CSegment* pParam);
	void                OnDtmfStringIdentNull(CSegment* pParam);
	BYTE                IsValidRecording();
	void                PlayOnlyRecordMessageWithoutPlayMessage(); //IVR for TIP

	void 				OnCamStopRollCallRecordingRECORDING(CSegment* pParam);
	void 				OnCamStopRollCallRecordingACTIVE(CSegment* pParam);
	void 				StopRecording(CSegment* pParam);
	void 				OnCamStopRollCallRecordingAck(CSegment* pParam);
	void 				OnTimerStopRollCallRecordingAck(CSegment* pParam);

protected:
	void                OnConfCreatePartyPREPARE(CSegment* pParam);

private:
	IVRMsgDescriptor*   m_ArrayOfMessages;
	IVRMsgDescriptor*   m_ArrayOfMessagesToPlay;
	char**              m_Msg_Files_Name;
	WORD                m_Messages_Number;
	WORD                m_TimerRollCall;
	bool                m_alreadyRecorded; //IVR for TIP, to know if we already recorded the name

protected:
	CIVRRollCallFeature* m_pRollCallFeature;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubDialOutToRemote
////////////////////////////////////////////////////////////////////////////
class CIvrSubDialOutToRemote : public CIvrSubBaseSM
{
public:
	                    CIvrSubDialOutToRemote(void);
	virtual            ~CIvrSubDialOutToRemote();

	virtual const char* NameOf() const { return "CIvrSubDialOutToRemote";}
	void                OnTimerDTMF(CSegment* pParam);
	virtual void*       GetMessageMap();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        Start();
	virtual void        PlayRetryMessage(DWORD messageMode);

protected:
	void                OnDTMFNumberReceivedCOLLECT(CSegment* pParam);
	void                FindAndSendDigit(WORD opcode, WORD rDigits);

	WORD                m_endDone;

	PDECLAR_MESSAGE_MAP
};



////////////////////////////////////////////////////////////////////////////
//                        CIvrSubNumericConferenceId
////////////////////////////////////////////////////////////////////////////
class CIvrSubNumericConferenceId : public CIvrSubBaseSM
{
public:
	                    CIvrSubNumericConferenceId(const CIVRNumericConferenceIdFeature* pNumericConferenceIdFeature, const char* pNumericId, BOOL bEnableExternalDB);
	                   ~CIvrSubNumericConferenceId();

	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);
	WORD                DoSomething();

protected:
	DWORD               FindConfAccordingToNid(const char* szNid);
	DWORD               FindMRAccordingToNid(const char* szNid, WORD& nidOfEqOrSipFactory);
	int                 IsQuickLeaderPW(CCommRes* pCommConf);
	void                OnTimeNoExtDBResponse(CSegment* pParam);
	int                 OnExtDBResponse(CSegment* pParam);
	void                OnExtDBFailure(CSegment* pParam);
	void                OnSIPconfNIDConfirmInd(CSegment* pParam);   // IvrProviderEQ
	bool                IsDTMFWODelimeterEnabled();

protected:
	IVRMsgDescriptor*               m_ArrayOfMessages;
	char**                          m_Msg_Files_Name;
	WORD                            m_Messages_Number;
	BYTE                            m_useExternalDB;

	char                            m_pNumericConferenceid[NUMERIC_CONFERENCE_ID_LEN];
	CIVRNumericConferenceIdFeature* m_pNumericConferenceIdFeature;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubMuteNoisyLine
////////////////////////////////////////////////////////////////////////////
class CIvrSubMuteNoisyLine : public CIvrSubBaseSM
{
public:
	                    CIvrSubMuteNoisyLine(const CIVRMuteNoisyLineFeature* pMuteNoisyLine);
	                   ~CIvrSubMuteNoisyLine();
	virtual const char* NameOf() const { return "CIvrSubMuteNoisyLine";}

	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);
	void                OnDtmfNoisyLineUnmute(CSegment* pParam);
	void                OnDtmfNoisyLineMute(CSegment* pParam);
	void                OnDtmfNoisyLineAdjust(CSegment* pParam);
	void                OnDtmfNoisyLineDisable(CSegment* pParam);
	void                OnDtmfString(CSegment* pParam);
	WORD                DoSomething();
	BYTE                CreateSubConfDtmfTable();
	void                PrepareEndFeature();
	void                UpdateNoiseDetection(WORD noiseDetection, WORD noiseDetectionThreshold, WORD updateDsp);
	void                PlayMuteNoisyLinePreMenu(void);
	void                PlayMuteNoisyLineMenu(void);
	void                GetMuteNoisyLineMsgsParams(WORD event_op_code, char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum);

private:

	CIVRMuteNoisyLineFeature* m_pMuteNoisyLine;
	WORD                      m_general_type;
	WORD                      m_retry_opcode;
	BYTE                      m_error;

	CDTMFCodeList*            m_pSubConfDtmfTable;
	CDTMFCodeList*            m_pIvrDefConfTable;

	PDECLAR_MESSAGE_MAP
};


#define MAX_PLC_TABLE_TYPES 10
#define MAX_TYPE_ENTRIES    5

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubPLC
////////////////////////////////////////////////////////////////////////////
class CIvrSubPLC : public CIvrSubBaseSM
{
public:
	                    CIvrSubPLC();
	                   ~CIvrSubPLC();

	virtual const char* NameOf() const { return "CIvrSubPLC";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);
	virtual void        OnStopIvr();

public:
	void                RejectPLC();

protected:
	void                OnTimerDTMF(CSegment* pParam);
	void                OnDTMF(CSegment* pParam);

	void                DoPlcAction();
	void                DoStarAction();
	void                DoSulamitAction();
	void                DoZeroAction();
	void                PrepareTables();
	void                DoNumberAction(int num);
	void                SendChangeLayout();
	void                EndPlcFeature();

protected:
	WORD                m_messageType;
	CDTMFCodeList*      m_pIvrPrevTable;
	WORD                m_currentMainIndex;
	WORD                m_subMainIndex;
	WORD                m_typeNum[MAX_PLC_TABLE_TYPES];
	DWORD               m_plcTableOpcodes[MAX_PLC_TABLE_TYPES][MAX_TYPE_ENTRIES];

	PDECLAR_MESSAGE_MAP
};



////////////////////////////////////////////////////////////////////////////
//                        CIvrSubCascadeLink
////////////////////////////////////////////////////////////////////////////
class CIvrSubCascadeLink : public CIvrSubBaseSM
{
public:
	                    CIvrSubCascadeLink();
	                   ~CIvrSubCascadeLink();

	virtual const char* NameOf() const { return "CIvrSubCascadeLink";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);
	WORD                DoSomething();
	void                OnDTMF();
	void                MovePartyToItsConf(DWORD wConf_id, ETargetConfType targetConfType);

	// action functions
	void                OnTimerDTMF(CSegment* pParam);

protected:
	DWORD               FindOnGoingConfAccordingNid();
	DWORD               FindMRAccordingToNid(WORD& nidOfEqOrSipFactory);
	int                 IsPasswordOk(DWORD dwConf_id, ETargetConfType targetConfType);
	void                OnGetNID(WORD isFromDialString = 0);
	void                SetNIDAndPW();

protected:
	WORD                m_startWaitForDigit;
	WORD                m_DtmfTimer1;
	WORD                m_DtmfTimer2;
	char                m_confPW[MAX_DTMF_STRING_LENGTH];
	char                m_NID[MAX_DTMF_STRING_LENGTH];

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubOperatorAssist
////////////////////////////////////////////////////////////////////////////
class CIvrSubOperatorAssist : public CIvrSubBaseSM
{
public:
	                    CIvrSubOperatorAssist(const CIVROperAssistanceFeature* OperAssist, DWORD request_type);
	                   ~CIvrSubOperatorAssist();

	// more global functions
	virtual const char* NameOf() const;
	virtual void*       GetMessageMap();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        Start();
	virtual void        PlayRetryMessage(DWORD messageMode);

public:
	void                Disconnect();

protected:
	CIVROperAssistanceFeature* m_pOperAssistFeature;
	DWORD                      m_request_type; // what cause the operator feature

	PDECLAR_MESSAGE_MAP
};

//////////////////////////////////////////////////////////////
class CIvrSubAudioExternal : public CIvrSubBaseSM
{
public:

	CIvrSubAudioExternal(const char* fileName,DWORD duration, const std::string& dialogID);
	~CIvrSubAudioExternal();

	virtual const char* NameOf() const { return "CIvrSubAudioExternal";}
	virtual void* GetMessageMap();
	virtual void  Start();
	virtual int   IsLegalString( CSegment* pParam );
	virtual void   OnFirstDTMFReceivedACTIVE( CSegment* pSeg );
	virtual void  PlayRetryMessage( DWORD messageMode );
	virtual void  PlayMessage( DWORD messageMode, WORD loop_delay=30);


	PDECLAR_MESSAGE_MAP

};
//////////////////////////////////////////////////////////////

class CIvrSubVideoExternal : public CIvrSubBaseSM
{
public:

	CIvrSubVideoExternal(const char* fileName,DWORD duration, const std::string& dialogID);
	~CIvrSubVideoExternal();

	virtual const char* NameOf() const { return "CIvrSubVideoExternal";}
	virtual void* GetMessageMap();
	virtual void  Start();
	virtual int   IsLegalString( CSegment* pParam );
	virtual void   OnFirstDTMFReceivedACTIVE( CSegment* pParam );
	virtual void  PlayRetryMessage( DWORD messageMode );
	virtual void  PlayMessage( DWORD messageMode, WORD loop_delay=30);


	PDECLAR_MESSAGE_MAP

};

//////////////////////////////////////////////////////////////
class CIvrSubExternalInputCollect : public CIvrSubBaseSM
{
public:

	CIvrSubExternalInputCollect(const std::string& dialogIdStr, char strDelimiter, WORD dtmfTimeOut, WORD dtmfMaxDigits, BOOL isClearBuffer = TRUE/*, const char* strEscapeChar*/ );
	~CIvrSubExternalInputCollect();

	virtual const char* NameOf() const { return "CIvrSubExternalInputCollect";}
	virtual void* GetMessageMap();
	virtual void  Start();
	virtual int   IsLegalString( CSegment* pParam );
	virtual void  OnFirstDTMFReceivedACTIVE(CSegment* pSeg);
	virtual void  PlayRetryMessage( DWORD messageMode );
	virtual void  PlayMessage( DWORD messageMode, WORD loop_delay=30);
	virtual void  StopPlayMessage();
	WORD          EndCollectFeature(WORD status);
	virtual WORD  DoSomething();
	virtual void  EndFeatureError(WORD status);
	virtual void  EndFeature(WORD status);
	virtual int   GetDtmfResult(CSegment* pParam, WORD& opcode, WORD& rDigits);


	PDECLAR_MESSAGE_MAP

private:
	char m_escapeChar;
	BOOL m_bIsClearBuffer;
};

////////////////////////////////////////////////////////////////////////////
class CIvrSubInvite : public CIvrSubBaseSM
{
public:
	                    CIvrSubInvite();
	                   ~CIvrSubInvite();

	virtual const char* NameOf() const;
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayMessage(DWORD messageMode, WORD loop_delay = 30);
	virtual void        PlayRetryMessage(DWORD messageMode);
	WORD                DoSomething();
	int                 CheckIfIvrMsgsExistInFeature();

protected:
	WORD                CheckLeagalPW();

protected:
	WORD                m_general_type;
	WORD                m_retry_opcode;
	BYTE                m_error;
	BYTE                m_need_to_stop_previous_msg;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubNoVideoResources
////////////////////////////////////////////////////////////////////////////
class CIvrSubNoVideoResources : public CIvrSubBaseSM
{
public:
	                    CIvrSubNoVideoResources(WORD messageType);
	                   ~CIvrSubNoVideoResources();

	virtual const char* NameOf() const { return "CIvrSubNoVideoResources";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);

	// action functions
	void                ContinueNoVideoResources(CSegment* pParam);
	void                OnDTMFReceivedACTIVE(CSegment* pParam);

protected:
	WORD                m_messageType;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubVenus
////////////////////////////////////////////////////////////////////////////
class CIvrSubVenus : public CIvrSubBaseSM
{
public:
	                    CIvrSubVenus();
	                   ~CIvrSubVenus();

	virtual const char* NameOf() const { return "CIvrSubVenus";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);
	virtual void        OnStopIvr();

public:
	void                RejectVenus();

protected:
	void                OnTimerDTMF(CSegment* pParam);
	void                OnDTMF(CSegment* pParam);

	void                DoVenusAction();
	void                PrepareTables();
	void                DoChangeLayoutAction(const std::string& sLayoutCode);
	void                EndVenusFeature();

protected:
	WORD                              m_messageType;
	CDTMFCodeList*                    m_pIvrPrevTable;
	WORD                              m_currentMainIndex;
	WORD                              m_subMainIndex;
	WORD                              m_typeNum[MAX_PLC_TABLE_TYPES];
	DWORD                             m_plcTableOpcodes[MAX_PLC_TABLE_TYPES][MAX_TYPE_ENTRIES];

	std::map<std::string, LayoutType> m_mapLayouts;
	std::string                       m_sDTMF;


	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CIvrSubInviteParty
////////////////////////////////////////////////////////////////////////////
class CIvrSubInviteParty : public CIvrSubBaseSM
{
public:
	                    CIvrSubInviteParty(const CIVRInvitePartyFeature* pInvitePartyFeature, const char* pPartyDialingNum, BOOL bEnableExternalDB);
	                   ~CIvrSubInviteParty();

	virtual const char* NameOf() const { return "CIvrSubInviteParty";}
	virtual void*       GetMessageMap();
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual void        PlayRetryMessage(DWORD messageMode);

	WORD                DoSomething();
	void                OnTimeNoExtDBResponse(CSegment* pParam);
	int                 OnExtDBResponse(CSegment* pParam);
	void                OnExtDBFailure(CSegment* pParam);

protected:
	char                    m_pPartyDialingNum[MAX_PASSWORD_LEN+1];
	CIVRInvitePartyFeature* m_pInvitePartyFeature;
	int                     m_internalState;
	BYTE                    m_useExternalDB;
	BOOL                    m_skipPrompt;

	PDECLAR_MESSAGE_MAP
};

////////////////////////////////////////////////////////////////////////////
//                        CIvrSubWait
////////////////////////////////////////////////////////////////////////////
class CIvrSubWait : public CIvrSubBaseSM
{
public:

	                    CIvrSubWait();
	                   ~CIvrSubWait();

	virtual const char* NameOf() const { return "CIvrSubWait";}
	virtual void*       GetMessageMap();
	virtual void        PlayRetryMessage(DWORD messageMode);
	virtual void        Start();
	virtual int         IsLegalString(CSegment* pParam);
	virtual WORD        DoSomething();
	void                OnEndWait();


	PDECLAR_MESSAGE_MAP
};

#endif
