#ifndef _PARTY
#define _PARTY

#include "TaskApp.h"
#include "ConfPartyDefines.h"
#include "StructTm.h"
#include "RsrcDesc.h"
#include "IVRDtmfColl.h"
#include "OpcodesMcmsCommon.h"
#include "ConfPartyStructs.h"

#include "MscIvr.h"
#include "CollectElementType.h"

#include "MccfHelper.h"

#define PARTYNAME (m_pParty->GetFullName())

// general party state machine
// Have to Be the same in all parties!!!
// party states
const WORD PARTYIDLE                = 1;
const WORD PARTYSETUP               = 2;
const WORD PARTYCHANGEMODE          = 3;
const WORD PARTYCONNECTED           = 4;
const WORD PARTYDISCONNECTING       = 5;
const WORD PARTY_RSR_DEFICIENCY     = 6;

const WORD PartyStateNo             = 5;
const WORD CONFDISCONNECT           = DESTROY;
const WORD TIPADDSLAVEPARTY         = 10; // TIP
const WORD CONFDISCONNECTOUT        = 54;
const WORD PARTYCONTOUT             = 55;
const WORD MAX_START_IVR_TIMER_ITER = 100;
const TICKS TIP_MASTER_WAIT_EXTRA_FOR_SLAVE_IVR_START = 20;
const WORD TIMER_START_IVR          = 113;
const WORD DNSQUARYTOUT             = 120; // DNS timer
const WORD ICERESPONSETOUT          = 130; // ICE timers
const WORD MAX_CONF_LEADER          = 5;

class CConfApi;
class  CIvrCntlLocal;
class CIvrCntl;

// PARTY CONTROL STATE MACHINE DESCRIPTION
// --------------------------------------

// 1. INCOMING EVENTS :  UPPER INTERFACE - CONF , X-BRDG , LOWER INTERFACE - MUXCNTL , NETCNTL

// +==========================================================================+
// |  EVENT NAME     |  INTERFACE     |  DESCRIPTION                          |
// +==========================================================================+
// |  ESTABLISHCALL  |     CONF       |  ESTABLISH IN CALL                    |
// +--------------------------------------------------------------------------+
// |  CHANGEMODE     |     CONF       |* CHANGE TRANSMIT MODE                 |
// +--------------------------------------------------------------------------+
// |  EXNGCAP        |     CONF       |  EXCHANGE CAPBILITIES                 |
// +--------------------------------------------------------------------------+
// |                 |                |                                       |
// +--------------------------------------------------------------------------+
// |  FREEZEPIC      |   VIDBRDG      |  FREEEZE PICTURE                      |
// +--------------------------------------------------------------------------+
// |  FASTUPDATE     |   VIDBRDG      |  FAST PICTURE UPDATE                  |
// +--------------------------------------------------------------------------+
// |  SETVIDPARAM    |   VIDBRDG      |* SET VIDIO PARAMTERS ( SWITCH RCV TS )|
// +--------------------------------------------------------------------------+
// |                 |                |                                       |
// +--------------------------------------------------------------------------+
// |  SETAUDPARAM    |   AUDBRDG      |* SET AUDIO PARAMTERS ( SWITCH RCV TS )|
// +--------------------------------------------------------------------------+
// |                 |                |                                       |
// +--------------------------------------------------------------------------+
// |  SETDATPARAM    |   DATA         |* SET DATA PARAMTERS                   |
// +--------------------------------------------------------------------------+
// |                 |                |                                       |
// +--------------------------------------------------------------------------+
// |  ENDH221CON     |   MUXCNTL      | H221 CONNECTION ESTABLISHED           |
// +--------------------------------------------------------------------------+
// |  ENDIDENTCHNL   |   MUXCNTL      | CHANNEL IDENTIFICATION                |
// +--------------------------------------------------------------------------+
// |  ENDCHAGEMODE   |   MUXCNTL      | CHANGE MODE COMPLETED                 |
// +--------------------------------------------------------------------------+
// |  RMTCAP         |   MUXCNTL      | REMOTE CAPBAILTY                      |
// +--------------------------------------------------------------------------+
// |  SYNCLOSS       |   MUXCNTL      | SINCHRONIZATION LOST (LOCAL OR REMOTE)|
// +--------------------------------------------------------------------------+
// |  SYNCREGAIN     |   MUXCNTL      | SINCHRONIZATION REGAINED              |
// +--------------------------------------------------------------------------+
// |  H221CONTOUT    |   MUXCNTL      | H221 CONNECTION FAILED                |
// +--------------------------------------------------------------------------+
// |  CHANGEMODETOUT |   MUXCNTL      | CHANGE MODE FAILED                    |
// +--------------------------------------------------------------------------+
// |  IDENTCHNLTOUT  |   MUXCNTL      | IDETIFY CHANNEL FAILED                |
// +--------------------------------------------------------------------------+
// |  SYNCTOUT       |   MUXCNTL      | SINCHRONIZATION REGAIN TIME OUT       |
// +--------------------------------------------------------------------------+
// |  EXCHNGCAPTOUT  |   MUXCNTL      | EXCHANE CAP TIME OUT                  |
// +--------------------------------------------------------------------------+
// |                 |                |                                       |
// +--------------------------------------------------------------------------+
// |  NETCONNECT     |   NETCNTL      | CHANNEL IS CONNECTED                  |
// +--------------------------------------------------------------------------+
// |  CONTOUT        |   NETCNTL      | CONNECTION SETUP TIME OUT             |
// +--------------------------------------------------------------------------+
// |  DISCONNECT     |   NETCNTL      | REMOTE CHANNEL DISCONNECT             |
// +--------------------------------------------------------------------------+


// 2. STATES

// +==========================================================================+
// |  STATE NAME     | DESCRIPTION                                            |
// +==========================================================================+
// |   IDLE          |  CONSTRUCTION STATE                                    |
// +--------------------------------------------------------------------------+
// |   SETUP         |  ESTABLISHMENT INIAIL CHANNEL NET + H221 CONNECTION    |
// +--------------------------------------------------------------------------+
// |   CONNECTED     |  TRANSMIT MODE == RECIEVE MODE == TARGET MODE          |
// +--------------------------------------------------------------------------+
// |   CHANGEMODE    |  CHANGE MODE                                           |
// +--------------------------------------------------------------------------+

// 3. OUTGOING EVENTS -

// +==========================================================================+
// |  EVENT NAME     |  INTERFACE     |  DESCRIPTION                          |
// +==========================================================================+
// |  ENDINCALL      |  CONF          | IN CALL CONNECTED                     |
// +--------------------------------------------------------------------------+
// |  ENDOUTCALL     |  CONF          | OUT CALL CONNECTED                    |
// +--------------------------------------------------------------------------+
// |  RMTCAP         |  CONF          | REMOTE CAPBILITIES                    |
// +--------------------------------------------------------------------------+
// |  ZEROING        |  CONF          | ZERO MODE ENFORCING                   |
// +--------------------------------------------------------------------------+
// |  DISCONNECT     |  CONF          | INITIAL CHANNEL DISCONNECT            |
// +--------------------------------------------------------------------------+
// |  ENDIDENTCHNL   |  CONF          | END CHANNEL IDENTIFICATION            |
// +--------------------------------------------------------------------------+
// |                 |                |                                       |
// +--------------------------------------------------------------------------+
// |  CALLOUT        |  NETCNTL       | ESTABLISH OUT CALL                    |
// +--------------------------------------------------------------------------+
// |  CALLIN         |  NETCNTL       | ESTABLISH IN CALL                     |
// +--------------------------------------------------------------------------+
// |  DISCONNECT     |  NETCNTL       | LOACL DISCONNECT                      |
// +--------------------------------------------------------------------------+
// |                 |                |                                       |
// +--------------------------------------------------------------------------+
// |  H221CON        |  MUXCNTL       | ESTABLISH H221 CONNECTION             |
// +--------------------------------------------------------------------------+
// |  SETCOMMODE     |  MUXCNTL	      | SET TARGET COMMUNICTION MODE          |
// +--------------------------------------------------------------------------+
// |  IDENTCHNL      |  MUXCNTL	      | IDETIFY INCOME CHANNEL                |
// +--------------------------------------------------------------------------+
// |  EXNGCAP        |  MUXCNTL	      | EXCHANGE CAPBABILITIES                |
// +--------------------------------------------------------------------------+
// |  SETNETSTRM     |  MUXCNTL       | SET NET STREAM                        |
// +--------------------------------------------------------------------------+
// |  SETDATSTRM     |  MUXCNTL       | SET DATA STREAM                       |
// +--------------------------------------------------------------------------+
// |  SETAUDSTRM     |  MUXCNTL       | SET AUDIO STREAM                      |
// +--------------------------------------------------------------------------+
// |  SETVIDSTRM     |  MUXCNTL       | SET VIDEO STREAM                      |
// +--------------------------------------------------------------------------+
// |  FREEZEPIC      |  MUXCNTL       | FREEZE PICTURE REQUEST                |
// +--------------------------------------------------------------------------+
// |  FASTUPDATE     |  MUXCNTL       | FAST UPDATE REQUEST                   |
// +--------------------------------------------------------------------------+
// |                 |                |                                       |
// +--------------------------------------------------------------------------+

// 4. PREDICATES

// +==========================================================================+
// |  PREDICATES NAME | DESCRIPTION                                           |
// +==========================================================================+
// |                  |                                                       |
// +--------------------------------------------------------------------------+


// 5. STATE MACHINE TABLE -  __STATES
// |
// EVENTS
//
// ENTRY - ACTION FUNCTION(OUTGING EVENT),NEW STATE
// NV(NOT VALID) NA(NO ACTION - NO OUTGOING EVENT<NO NEW STATE)  ESC-#(ACTION REFERENC)

// +=====================================================================+
// |                  |      IDLE    |  SETUP  |  CHANGEMODE | STEADY    |
// +==================|==================================================+
// |  ESTABLISHCALL   |     ESC-0    |  ESC-20 |    NV       |   NV      |
// +------------------|--------------------------------------------------+
// |  CHANGEMODE      |     NV       |   NV    |    NV       |  ESC-2    |
// +------------------|--------------------------------------------------+
// |  EXNGCAP         |     NV       |   NV    |   ESC-3     |  ESC-3    |
// +------------------|--------------------------------------------------+
// |  FREEZEPIC       |     NV       |   NV    |    NV       |  ESC-4    |
// +------------------|--------------------------------------------------+
// |  FASTUPDATE      |     NV       |   NV    |    NV       |  ESC-5    |
// +------------------|--------------------------------------------------+
// |  SETVIDPARAM     |     NV       |   NV    |    NV       |  ESC-6    |
// +------------------|--------------------------------------------------+
// |  SETAUDPARAM     |     NV       |   NV    |    NV       |  ESC-7    |
// +------------------|--------------------------------------------------+
// |  SETDATPARAM     |     NV       |   NV    |    NV       |  ESC-8    |
// +------------------|--------------------------------------------------+
// |  ENDH221CON      |     NV       |  ESC-30 |    NV       |   NV      |
// +------------------|--------------------------------------------------+
// |  ENDIDENTCHNL    |     NV       |  ESC-31 |   ESC-31    |  ESC-31   |
// +------------------|--------------------------------------------------+
// |  ENDCHAGEMODE    |     NV       |   NV    |   ESC-9     |   NV      |
// +------------------|--------------------------------------------------+
// |  RMTCAP          |     NV       |   NV    |    NV       |  ESC-10   |
// +------------------|--------------------------------------------------+
// |  SYNCLOSS        |     NV       |  ESC-10 |  ESC-10     |  ESC-10   |
// +------------------|--------------------------------------------------+
// |  SYNCREGAIN      |     NV       |  ESC-11 |  ESC-11     |  ESC-11   |
// +------------------|--------------------------------------------------+
// |  H221CONTOUT     |     NV       |  ESC-12 |    NV       |   NV      |
// +------------------|--------------------------------------------------+
// |  CHANGEMODETOUT  |     NV       |   NV    |   ESC-13    |   NV      |
// +------------------|--------------------------------------------------+
// |  IDENTCHNLTOUT   |     NV       |  ESC-14 |   ESC-14    |  ESC-14   |
// +------------------|--------------------------------------------------+
// |  SYNCTOUT        |     NV       |  ESC-15 |   ESC-15    |  ESC-15   |
// +------------------|--------------------------------------------------+
// |  EXCHNGCAPTOUT   |     NV       |   NV    |   ESC-16    |  ESC-16   |
// +------------------|--------------------------------------------------+
// |  NETCONNECT      |     NV       |  ESC-17 |   NV        |   NV      |
// +------------------|--------------------------------------------------+
// |  CONTOUT         |     NV       |  ESC-18 |   NV        |   NV      |
// +------------------|--------------------------------------------------+
// |  DISCONNECT      |     NV       |  ESC-19 |   ESC-19    |  ESC-19   |
// +------------------|--------------------------------------------------+


// ENTRY REFERNCE -

class CConfParty;

extern "C" void PartyEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//                        CParty
////////////////////////////////////////////////////////////////////////////
class CParty : public CTaskApp
{
	CLASS_TYPE_1(CParty, CTaskApp)

public:
	                    CParty();
	virtual            ~CParty();

	virtual const char* NameOf() const                         { return "CParty";}

	virtual void        Create(CSegment& appParam);
	virtual void        SetFullName(const char* partyName, const char* confName);
	void                SetIsLeader(BYTE isLeader)             { m_isLeader = isLeader; }
	BYTE                GetIsLeader() const                    { return m_isLeader; }
	void                InformArtOnFeccPartyType(CSegment* pParam);

	const char*         GetName() const                        { return m_name; }
	virtual int         GetTaskMbxBufferSize() const           { return 192*1024-1; }

	virtual eTaskRecoveryPolicyAfterSeveralRetries
	                    GetTaskRecoveryPolicyAfterSeveralRetries() const { return eDoNothing; }

	// Operations
	BOOL                TaskHandleEvent(CSegment* pMsg, DWORD msgLength, OPCODE opcode);
	void                HandleXMLEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	const char*         GetTaskName() const                    { return "CParty"; }
	BOOL                IsSingleton() const                    { return NO; }
	virtual void        InitTask()                             { }
	virtual void*       GetMessageMap()                        { return (void*)m_msgEntries; }

	void                OnMcuForceKillAnyCase(CSegment* pParam);
	void                OnConfExport(CSegment* pParam);
	void                OnConfSetMoveParams(CSegment* pParam)  { }
	void                OnConfUpdateNoResourcesForVideoParty(CSegment* pParam);

	void                OnTimerVcu(CSegment* pParam);

	void                OnConfEstablishCall(CSegment* pParam);
	void                OnEncDisconnect(CSegment* pParam);
	void                OnUpdateEncryptionState(CSegment* pParam);
	void                OnTimerStartIvr(CSegment* pParam);
	void                OnCamStopIvr(CSegment* pParam);
	void                OnStartDialogParty(CSegment* pParam); //AT&T
	void                StartDialogParty(DialogState& state, DWORD mcmsDelayInMsecs); //AT&T
	virtual void        OnAckPlayMessage(CSegment* pParam); //AT&T
	void                OnAckShowSlide(CSegment* pParam); //AT&T
	virtual void        OnAckRecordPlayMessage(CSegment* pParam); //IVR for TIP
	void                OnCAMMediaConnection(CSegment* pParam); // Extrenal IVR - CAM notification of media connection
	void                OnCAMMediaDisconnection(CSegment* pParam); // Extrenal IVR handling of internal media disconnection

	char*               GetFullName()                          { return m_partyConfName; }

	CIvrCntl*           GetIvrCntl(void)                       { return m_ivrCtrl; }
	PartyRsrcID         GetPartyRsrcID() const                 { return m_PartyRsrcID; }

	void                StartIvr(BOOL bIsResume = FALSE);
	void                StartIvrExternal();
	virtual void        PartySpecfiedIvrDelay();
	void                CancelIVR();
	void                StartIvrIfNeeded();
	void                StartIvrFinally();
	void                StartDelayedExternalIvr();
	void                StartIvrFinallyAudioDelayed();
	void                OnTimerAudioDelayed(CSegment* pParam);

	BYTE                IsGateway() const                      { return m_IsGateWay; }

	BYTE                GetRecordingPort()                     { return m_isRecordingPort; }
	virtual void        CreateIVRService(bool turnOffIvrAfterMove = FALSE);
	void                SetFromEntryQ(const WORD bFromEntryQ)  { m_bFromEntryQ = bFromEntryQ; }
	WORD                IsFromEntryQ()                         { return m_bFromEntryQ; }
	void                SetLastConfIDPartyIsLeader(DWORD confID);
	virtual void        GetIpCallIdentifiers(IP_EXT_DB_STRINGS* ipStringsStruct);

	BYTE                GetMcuNum() const                      { return m_mcuNum; }
	BYTE                GetTerminalNum() const                 { return m_termNum; }
	void                SetExtDone(const WORD bExtDone)        { m_bExtDone = bExtDone; }
	WORD                GetExtDone()                           { return m_bExtDone; }
	void                OnConfLeaderChangeStatusConnect(CSegment* pMsg);
	virtual void        OnPhoneError()                         { }
	BYTE                IsAudioOnly() const                    { return m_AudioOnly; }

	WORD                GetCascadeMode()                       { return m_cascadeMode; }
	void                SetCascadeMode(WORD cascadeMode)       { m_cascadeMode = cascadeMode; }
	WORD                GetIsVoice() const                     { return m_voice; }

	BYTE                GetIsAutoVidBitRate()                  { return m_isAutoVidBitRate; }

	virtual const char* GetNumericConfId() const               { return m_numericConfId; }

	PartyMonitorID      GetMonitorPartyId() const              { return m_monitorPartyId; }
	PartyRsrcID         GetPartyId() const                     { return m_PartyRsrcID; }
	void                SetPartyId(PartyRsrcID partyId)        { m_PartyRsrcID = partyId; }

	ConfMonitorID       GetMonitorConfId() const               { return m_monitorConfId; }
	ConfRsrcID          GetConfId() const                      { return m_ConfRsrcId; }

	virtual BYTE        IsUndefinedParty() const;
	virtual void        SendFlowControlToCs(CSegment* pParam);

	void                OnUpdateLprLocalRmtActivation(CSegment* pParam);
	void                OnUpdateLprChannelHeader(CSegment* pParam);
	virtual BYTE        IsCascadeToPolycomBridge();
	virtual BYTE        IsRemoteIsSlaveMGCWithContent()        { return NO; }
	virtual BYTE        IsRemoteIsMGC()                        { return NO; }

	void                ForwardDtmfToPCM(CSegment* pParam);
	virtual BYTE        IsRemoteIsRVpresentation()             { return NO; }
	BYTE                IsTipCompatibleContent() const;
	BYTE 				IsPreferTip() const; //BRIDGE-13949
	void                InformConfOnPacketLossState(const eRtcpPacketLossStatus cellInd, const eRtcpPacketLossStatus layoutInd);
	void                UpdateMuteIconState(EOnOff onoff);
	void                SendArtUpdateWithSsrcAck(const DWORD aStatus);
	virtual void        ForwardEventToTipSlaves(CSegment* pParam, OPCODE opCode) {}
	void                SetIsCamReadyForIVR(BOOL newVal) { m_bCAMReadyForIVR = newVal; }
	BOOL                IsCamReadyForIVR() const { return m_bCAMReadyForIVR; }

	virtual bool        GetIsTipCall() const { return false; }
	virtual             ETipPartyTypeAndPosition GetTipPartyType() const { return eTipNone; } //AT&T
	virtual             void UpdateTipParamForIvr() {} //IVR for TIP
	CConfApi*           GetConfApi() { return  m_pConfApi; }

	bool                IsCallFromGateway() const { return m_IsCallFromGateWay; }
	void                SetIsCallFromGateway(bool isCallFromGateway) { m_IsCallFromGateWay = isCallFromGateway; }

	BYTE 				IsIsdnGWCallCamesFromEQ();

	CConfParty* 		GetConfPartyNonConst();  // Overload can technically work here but will cause mis-usage, please leave the name as is
	const CConfParty* 	GetConfParty() const;

	void 				PendRecapOnToken(const CSegment* recapInfo);

protected:
	void                PartySelfKill();
	void                SetMonitorPartyId(CSegment* pMsg);

private:

	virtual void        CollectDigitsExternalIvr(DialogState& state, const CollectElementType& collect);
	virtual void        PlayFileExternalIvr(DialogState& state, const MediaElementType& media);
	virtual void        ExternalIvrStartDialog(DialogState& state, DWORD mcmsDelayInMsecs = 0);

protected:

	// Attributes
	WORD                m_status;
	PartyMonitorID      m_monitorPartyId;
	PartyRsrcID         m_PartyRsrcID;
	ConfMonitorID       m_monitorConfId;
	ConfRsrcID          m_ConfRsrcId;
	char                m_numericConfId[NUMERIC_CONFERENCE_ID_LEN];
	char                m_name[H243_NAME_LEN];
	WORD                m_RoomId;
	CConfApi*           m_pConfApi;
	CConfApi*           m_pDestConfApi;
	COsQueue*           m_pConfRcvMbx;
	CParty*             m_pParty;
	WORD                m_numNetChnlCntl;
	char                m_partyConfName[2*H243_NAME_LEN+50];
	char                m_password[30];
	WORD                m_mcuNum;
	WORD                m_termNum;
	WORD                m_voice;
	WORD                m_cascadeMode;         // local type: master mcu or slave mcu
	WORD                m_nodeType;            // terminal , master mcu , slave mcu
	WORD                m_isCleanup;
	WORD                m_disconnectCause;
	WORD                m_rcvVcuCounter;
	WORD                m_rcvVcuCounterThreshHold;
	WORD                m_rcvVcuCounterWindowTout;
	TICKS               m_lastVcuTime;
	TICKS               m_lastContentRefreshTime;
	TICKS               m_lastBridgeRefreshTime;
	TICKS               m_lastPmTicks;
	WORD                m_interfaceType;
	WORD                m_isChairEnabled;
	BYTE                m_IsGateWay;
	CIvrCntl*           m_ivrCtrl;
	BYTE                m_isLeader;
	BYTE                m_ivrStarted;
	BYTE                m_isRecordingPort;
	BYTE                m_bFromEntryQ;
	BYTE                m_AudioOnly;
	WORD                m_bExtDone;
	DWORD               m_confsLeader[MAX_CONF_LEADER];
	WORD                m_cascadeParty;
	WORD                m_serviceId;
	WORD                m_cntIVRWaitForConnected;
	BOOL                m_bCAMReadyForIVR;
	BOOL                m_bIvrAfterCallResume;
	BYTE                m_isAutoVidBitRate;
	BYTE                m_isPreSignalingFlowProb;
	BYTE                m_isDelayedIvr;
	DialogState*        m_pDelayedExternalIVRDialog;
	TICKS               m_delayedExternalIVRDialogTime;
	BYTE                m_bNoVideRsrcForVideoParty;
	BYTE                m_feccPartyType;
	bool                m_IsCallFromGateWay;
	CSegment 			m_recapPendedOnToken;

	PDECLAR_MESSAGE_MAP
};

#endif /* _PARTY */
