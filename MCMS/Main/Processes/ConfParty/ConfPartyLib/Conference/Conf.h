// +========================================================================+
// CONF.H                                                                   |
// Copyright 1995 Pictel Technologies Ltd.                                  |
// All Rights Reserved.                                                     |
// -------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of Pictel Technologies Ltd. and is protected by law.         |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from Pictel Technologies Ltd.                |
// -------------------------------------------------------------------------|
// FILE:       CONF.H                                                       |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Sami                                                         |
// -------------------------------------------------------------------------|
// Who | Date       | Description                                           |
// -------------------------------------------------------------------------|
// | 7/3/95     |                                                           |
// +========================================================================+

#ifndef _CONF_H__
#define _CONF_H__

#include <map>
#include "ProcessBase.h"
#include "Trace.h"
#include "ConfPartyDefines.h"
#include "ConfPartyRoutingTable.h"
#include "ConfApi.h"
#include "ConfPartyOpcodes.h"
#include "MplMcmsProtocol.h"
#include "PartyList.h"
#include "CommConfDB.h"
#include "CommConf.h"
#include "UnifiedComMode.h"
#include "ConfPartyTimeOut.h"
#include "LobbyApi.h"
#include "TerminalListManager.h"
// SIP simulation
#include "SipStructures.h"
#include "SipDefinitions.h"
#include "BridgeDefs.h"
#include "ArtDefinitions.h"
// IpV6
#include "ConfIpParameters.h"
#include "H320Caps.h"
#include "H320ComMode.h"
#include "ConfPartiesDialingSequence.h"
#include "VideoOperationPointsSet.h"
#include "OpcodesMcmsVideo.h"
#include "SNMPDefines.h"
#include "IpPartyControl.h"
#include "ConfPartyApiDefines.h"

class CPartyCntl;
class CPartyList;
class CPartyConnection;
class CAudioBridgeInterface;
class CVideoBridgeInterface;
class CFECCBridge;
class CSIPEventPackageDispatcher;
class CSvcEventPackageDispatcher;
class CConfAppMngrInterface;
class CConfApi;
class CAVmsgServiceList;
class CConfPartiesDialingSequence;
class CTextOnScreenMngrForGwSession;
class CTextOnScreenMngrForInvitedSession;
class CVideoBridgeInitParams;
class CMcmsPCMManager;
class CGatheringManager;
class CGathering;
class CContentToken;

// disconnect modes
#define DELETE_MODE                  0
#define DISCONNECT_MODE              1
#define DISCONNECT_N_RECOVER_MODE    2

#define UpdateSCM                    1
#define HCWithRemoteCap              2

#define GATHERING_UPDATE_TIMER_5_SEC 5*SECOND
#define GATHERING_UPDATE_TIMER_1_SEC 1*SECOND
#define GATHERING_CONNECTING_TIMER   2*SECOND

#define GW_SETUP                     0
#define GW_CONNECT                   1

// Added for Redial on Wrong Number
typedef struct SInviteResult{
	BOOL	bIsH323Used;
	BOOL	bIsSIPUsed;
	BOOL	bIsISDNUsed;
	BOOL	bIsPSTNUsed;
	eGeneralDisconnectionCause	eH323InviteResult;
	eGeneralDisconnectionCause	eSIPInviteResult;
	eGeneralDisconnectionCause	eISDNInviteResult;
	eGeneralDisconnectionCause	ePSTNInviteResult;
	eGeneralDisconnectionCause	eOverallInviteResult;
} SInviteResult;

typedef enum {
	eInviteTypeGw,
	eInviteTypeDtmf
} INVITE_TYPE;

typedef std::map <DWORD, SInviteResult> PARTIES_INVITE_RESULTS;
typedef std::map <PartyMonitorID, DWORD> PARTIES_REDIAL_NUM;
typedef std::map <PartyMonitorID, PartyMonitorID> PARTIES_INVITOR_ID;
typedef std::map <DWORD, DWORD> PARTIES_DEST_PARTY_NUM;
typedef std::map <PartyMonitorID, INVITE_TYPE> PARTIES_INVITE_TYPE;


typedef std::map <DWORD, BYTE> GW_PARTIES_STATUS;
typedef std::map <DWORD, WORD> GW_PARTIES_ID_LINE;
typedef std::map <PartyMonitorID, BYTE> INVITED_PARTIES_STATUS;

typedef std::map <DWORD, DWORD> PARTIES_TO_RELEASE_FROM_LOBBY;
typedef std::map <eXcodeRsrcType, ALLOC_PARTY_PARAMS_S*> XCODE_RSRC;

extern "C" void          ConfEntryPoint(void* appParam);
extern CCommConfDB*      GetpConfDB();


////////////////////////////////////////////////////////////////////////////
//                        CConf
////////////////////////////////////////////////////////////////////////////
class CConf : public CTaskApp
{
	CLASS_TYPE_1(CConf, CTaskApp)

	friend class CContentToken;

public:
	                       CConf();
	virtual               ~CConf();
	virtual const char*    NameOf() const               { return "CConf";}

	// Initializations
	void                   Create(CSegment& appParam);
	void                   Destroy(WORD discCause = 0);

	// Operations
	virtual BOOL           TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);
	virtual void*          GetMessageMap();
	const char*            GetTaskName() const;
	void                   InitTask()                   { }
	BOOL                   IsSingleton() const          {return NO;}
	const char*            GetName() const;

	virtual eTaskRecoveryPolicyAfterSeveralRetries GetTaskRecoveryPolicyAfterSeveralRetries() const {return eDoNothing;}

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	virtual int            GetTaskMbxBufferSize() const; // VNGFE-7734 - from system cfg flag
	virtual int            GetTaskMbxSndBufferSize() const; //  VNGFE-7734 - from system cfg flag
#else
	virtual int            GetTaskMbxBufferSize() const {return 512*1024-1;}
#endif

	std::ostream&          PrintName(std::ostream& ostr);

	CPartyCntl*            GetPartyCntl(PartyRsrcID partyId);
	CPartyCntl*            GetPartyCntl(const CTaskApp* pParty);
	CPartyCntl*            GetPartyCntl(const char* name);

	CPartyConnection*      GetPartyConnection(PartyRsrcID partyId);
	CPartyConnection*      GetPartyConnection(const CTaskApp* pParty);
	CPartyConnection*      GetPartyConnection(const char* name);

	CPartyConnection*      RemovePartyConnection(const CTaskApp* pParty);
	void                   InsertPartyConnection(CPartyConnection* pPartyCntl);
	void                   SetPartyAsLeader(const char* partyName, EOnOff onOff);

	WORD                   GetVoiceType(CConfParty* pConfParty);
	ENetworkType           GetNetoworkType(WORD spanType) const;

	const CCommConf*       GetCommConf() const;
	void                   AdjustVideoCapToHW(WORD action, CPartyConnection* pPartyConnection = NULL);

	WORD                   CheckIfIVRServiceIsOn();

	void                   ChangeScmOfIpPartyInCpOrCopConf(CIpComMode* pPartyScm, CConfParty* pConfParty, BYTE partyVidProtocol, DWORD setupRate, BYTE bCreateNewScm = FALSE, DWORD confRate = 0, WORD dialType = 1, DWORD serviceId = 0, CSipCaps* pRmtCaps = NULL,RemoteIdent epType = Regular, CSipNetSetup* pNetSetup = NULL);
	BYTE                   DecideOnProtocolForCpAuto(CConfParty* pConfParty);

	eSessionType           GetSessionTypeForResourceAllocator();

	STATUS                 GetPartyTerminalNumber(const CTaskApp* pParty, WORD& mcuNumber, WORD& terminalNumber);

	bool                   IsHDVSW() const;
	void                   SetHDVSWComMode();
	void                   SetCopComMode();

	bool                   IsLegacyShowContentAsVideo() const;

	// Content
	DWORD                  TranslateAMCRateIPRate(BYTE AMCRate);
	DWORD                  GetActualIpRateForContentSession();                                                                                                                                                                                                                                        // get the ip rate with/without the BCH factor
	void                   SetActualIpRateForContentSession(DWORD actualBitRate);
	BYTE                   TranslateConfIPRateToXferRate(DWORD H323Rate);
	void                   SetContentMode(BYTE XCallRate, eEnterpriseMode ContRatelevel, ePresentationProtocol ContProtocol, eCascadeOptimizeResolutionEnum resolutionLevel, cmCapDirection eDirection, BYTE bContentAsVideo, BOOL isContentHD1080 = FALSE, BYTE HDMpi = 0);                          // Set the caps with content rate
	BYTE                   GetAllowedContentRateAMC(BYTE minPartyConfRate);
	void                   SetNewContentBitRate(BYTE newAMCBitRate, cmCapDirection eDirection = cmCapReceiveAndTransmit);                                                                                                                                                                             // Set the current content rate with a new rate
	void                   SetNewContentProtocol(BYTE newProtocol, cmCapDirection eDirection = cmCapReceiveAndTransmit);
	BYTE                   GetCurrentContentBitRateAMC() const;
	BYTE                   GetCurrentContentProtocolInConfValues() const;
	CapEnum                GetCurrentContentProtocolInIpValues() const;
	//HP content
	BYTE                   GetCurrentContentIsHighProfile() const;
	void                   SetCurrentContentIsHighProfile(const BYTE isHPContent);

	void                   OnPartyLinkConnectCONNECT(CSegment* pParam);
	void                   OnPartyLinkDisconnectCONNECT(CSegment* pParam);
	void                   OnPartyLinkDisconnectTERMINATION(CSegment* pParam);
	void                   ChangeContentBridgeToSlave();
	void                   ChangeContentBridgeToMaster();
	void                   ChangeContentBridgeToBasicBridge();
	void                   SetTheNewBridgeForAllParties();
	void                   DetermineTelePresenceConfMode(BYTE onOff);
	CVideoBridgeInterface* GetVideoBridgeInterface()    {return m_pVideoBridgeInterface;}

	ConfRsrcID             GetConfId() const            { return m_ConfRsrcId; }
	ConfMonitorID          GetMonitorConfId() const     { return m_monitorConfId; }

	CGatheringManager*     GetGatheringManager()        { return m_pGatheringManager; }
	CGathering*            GetGathering(const char* pszPartyName) const;
	CVideoLayout*          GetGatheringLayout(CConfParty* pConfParty = NULL);
	CVisualEffectsParams*  GetGatheringVisualEffects();
	void                   StopGathering(const char* pszPartyName);
    void OnPartyDisConnected(CConfParty* pConfParty);//added for BRIDGE-13167
	
	void                   StartGatheringUpdateTimer(unsigned int iDelay = GATHERING_UPDATE_TIMER_5_SEC);
	void                   StartGatheringPartyConnectingTimer();
	int                    GetExceptionPartList(std::set<std::string>& setParts);

	// TIP
	BYTE                   CheckIfTipEnableByParams(CIpComMode* pPartyScm, CConfParty* pConfParty, DWORD confRate, DWORD partyRate);
	void                   SetPartyScmForTip(CIpComMode* pPartyScm, CConfParty* pConfParty, CSipNetSetup* pNetSetup, DWORD serviceId, BOOL remoteSDPAvailable);
	Eh264VideoModeType     GetTipResolutionTypeAccordingToVideoRate(DWORD videoBitRate);

	//BRIDGE-5753
	Eh264VideoModeType     GetTipResolutionTypeAccordingToMaxResolutionAndVidRate(EVideoResolutionType maxResolution, DWORD videoBitRate,CConfParty* pConfParty);
	void                   FixTipScmVideoBitRateIfNeeded(CConfParty* pConfParty , CIpComMode* pPartyScm , CSipNetSetup* pNetSetup, Eh264VideoModeType selectedVideoMode);
	BOOL                   IsNeedToChangeTipVidRate(Eh264VideoModeType selectedVideoMode, DWORD vidBitrate);
	DWORD                  GetMaxVidBitrateForHD720();
	void                   ChangeVideoBitrateForTipIfNeeded(CConfParty* pConfParty, CIpComMode* pPartyScm , DWORD& vidBitrate);
	BOOL                   GetFlagTipResAccordingToConfVidQuality();
	BOOL                   GetIsNeedToChangeTipResAccordingToConfVidQuality(CConfParty* pConfParty);

	// Content++
	BYTE                   DoesPartyMeetConfContentHDResolution(char* partyName);

	static const char*     ConfStateToString(WORD state);
	static const char*     LastQuitTypeToString(ELastQuitType lastQuitType);

	void                   RestoreLecturerVideoLayoutIfNeeded(CConfParty* pConfParty);
	STATUS                 CheckVideoLayout(CVideoLayout* pVideoLayoutOper, CVideoLayout* pVideoLayoutDB, BYTE isHDVSW, const WORD isPrivate);

	//VNGR-26449 - un-encrypted conference message
	void                   StopConfSecureMessageTimer();

	void                   LedSysAlarmInd();

	/*Begin:added by Richer for BRIDGE-15015, 11/13/2014*/
	STATUS SendLedSysAlarmIndToConfigurator (bool isDel); /*" true = del_ep","false = add_ep"*/
	/*End:added by Richer for BRIDGE-15015, 11/13/2014*/
	
	void                   UpdateDB(CTaskApp* pParty, WORD type, DWORD param, CSegment* pMsg = NULL, DWORD MipErrorNumber = 0);
	WORD                   GetNumXCodeRsrc() const {return m_mapXCodeRsrc.size();}
	XCODE_RSRC             GetXCodeRsrc() const {return m_mapXCodeRsrc;}

	//SVC AVC MIX mode
	void                   OnConfAvcSvcModeSet(CSegment* pParam);
	void                   OnPartyAvcSvcModeSet(CSegment* pParam);
	void                   InitConfMediaState();
	void                   UpdateConfMediaState(eConfMediaState state);
	bool                   IsMixAvcSvcDynamicAllocation() const;
	void                   SetMediaStateByMediaType();
	eConfMediaState        GetMediaState() const {return m_confMediaState;}
	void                   SetMediaState(eConfMediaState confMediaState);
	void                   SetConfAvcSvcMode(eConfMediaState confMediaState, ConfMonitorID confId);
	ePMediaType            ConvertConfMediaTypeToPartyType(eConfMediaState confMediaState);
	bool				   IsCPandAvcConf(){return ((GetVideoSession() == CONTINUOUS_PRESENCE) && (GetMediaState() == eMediaStateAvcOnly));}

	DWORD                  GetNumberOfParticpants(std::vector<DWORD>& partyStates);
	void                   SetMediaStateOnPartyDisconnectConnct(int partyState);

	void                   UpdateAudioParticipantsCount(CConfParty *pConfParty, BOOL isAddAudioOnly);
	void                   SendIndicationIconChange();

protected:
	STATUS                 SyncRsrcAllocConfSpreading(OPCODE opcode);
	STATUS                 SyncRsrcRsrvManagerConfTerminate(OPCODE opcode);

	void                   AddConfEntryToRsrcRoutingTbl();
	void                   RemoveConfEntryFromRsrcRoutingTbl();

	void                   HandlePartyEvent(CSegment* pMsg);
	void                   HandlePartyRsrcIdEvent(CSegment* pMsg);
	void                   HandlePartyExternalEvent(CSegment* pMsg);
	void                   HandleBrdgEvent(CSegment* pMsg);
	void                   HandleUpdateEvent(CSegment* pMsg);
	void                   HandleObserverUpdateEvent(CSegment* pMsg);
	void                   HandleConfMplEvent(CSegment* pMsg);
	void                   HandleMsLyncSlavesControllerEvent(CSegment* pMsg);
	void                   HandleMsLyncConfEvent(CSegment* pMsg);
	void                   HandleCdrUpdateEvent(CSegment* pMsg);

	void                   AllocPartyList();

	void                   DelParty(const char* name, WORD mode = 0, WORD discCause = 0,BOOL isVioloent = FALSE, DWORD taskId = 0);

	void                   DelPartyWithNoConnection(const char* name, WORD mode = 0, WORD discCause = 0);
	void                   CleanUp();
	void                   OnEndDelAllParties();

	STATUS                 CreateVideoBridge(EBridgeImplementationTypes vbImplementaionType = eVideoCP_Bridge_V1);
	STATUS                 CreateFECCBridge();
	STATUS                 CreateInActiveContentBridge();
	STATUS                 CreateContentBridge();

	void                   BridgeConnectionCompleted();
	void                   BridgeDisConnectionCompleted();

	void                   ConnectParty(CConfParty* pConfParty, DWORD connectDelay = 0);
	void                   ConnectPstnIsdnParty(CConfParty* pConfParty, char* avServiceName, WORD welcomeMsgTime, WORD connectDelay = 0);
	void                   ConnectIpParty(CConfParty* pConfParty, const char* avServiceNameStr, WORD welcomeMsgTime, DWORD connectDelay, DWORD roomID = 0, eTypeOfLinkParty partyType = eRegularParty);
	void                   ReconnectIpParty(CConfParty* pConfParty, CPartyConnection* pPartyConnection, DWORD redialInterval = 0, DWORD connectDelay = 0);
	void                   ReconnectPstnIsdnParty(CConfParty* pConfParty, CPartyConnection* pPartyConnection, WORD redialInterval = 0, DWORD connectDelay = 0);

	void                   UpdateConfStatus(DWORD status = 0xFFFFFFFF, BYTE bOnOff = 0xFF);
	void                   SetCapCommonDenominator(CTaskApp* partyId, BYTE bTakeCurrent);
	WORD                   GetNodeType(CConfParty* pConfParty);
	void                   UpdateActiveSpeakersList(CSegment *pSeg);
	PartyMonitorID         GetPartyMonitorIdFromPartyRsrcId(PartyRsrcID partyId);

	WORD                   GetVideoSession() const;

	void                   SetH320PartyCapsAndVideoParam(CComMode* pPartyScm, CComMode* pPartyTransmitScm, CCapH320* pCap, CConfParty* pConfParty, DWORD vidBitrate, WORD dialType, DWORD setupRate = 0, DWORD confRate = 0);

	BYTE                   GetConfTerminateCause() const;
	void                   SetConfTerminateCause(const BYTE conf_terminate_cause);

	void                   RemovePartyConnection(CTaskApp* pParty, BYTE isCancel);
	WORD                   AreAllPartiesDisconnected(WORD& numOfNonDisconnected, bool ignoreItpSlaves = false);
	void                   MuteVideoOfCascadeLinksIfNoVideoParties();

	// move change
	void                   EndImportParty(PartyRsrcID partyId, WORD status);
	void                   ImportPartyFailed(PartyRsrcID partyId, WORD status);
	void                   EndExportParty(PartyRsrcID partyId, WORD status);
	void                   ExportPartyFailed(PartyRsrcID partyId, WORD status);
	void                   ExportParty(PartyMonitorID monitorPartyId, ConfMonitorID monitorDestConfId, EMoveType eMoveType);
	void                   AnnounceNewConfToSipProxy();
	void                   AnnounceDelConfToSipProxy();

	void                   AnnounceLobbyConfOnAir(ConfMonitorID confMonitorID, char* targetConfName);
	void                   AnnounceLobbyConfOnAirIfNeeded();
	// VNGR-23989
	void                   ReleaseLobbySuspendPartiesIfNeeded();

	//VNGR-26449 - un-encrypted conference message
	void                   SendSecureMessageWhenAddParty(CPartyConnection*  pPartyConnection);
	void                   SendSecureMessageWhenDelParty(CPartyConnection*  pPartyConnection);
	void                   OnConfEndTimerSecureMessage(CSegment* pParam);

	void                   IvrConfTerminate();
	BYTE                   IsIvrAcknowledge(OPCODE ack_opcode);

	// action functions
	void                   OnMcuStartConfInd(CSegment* pParam);
	virtual void           OnMcuStartConfIdle(CSegment* pParam);

	void                   OnMcuAddPartyConnect(CSegment* pParam);
	void                   OnMcuAddPartyTermination(CSegment* pParam);
	void                   OnMcuAddPartyIdle(CSegment* pParam);
	void                   OnMcuAddPartySetup(CSegment* pParam);
	void                   OnMcuAddPartyAnycase(CSegment* pParam);
	virtual void           OnMcuDelPartyConnect(CSegment* pParam);
	virtual void           OnMcuDelPartyTermination(CSegment* pParam);
	void                   OnMcuDelPartyCViolent(CSegment* pParam);

	void                   OnMcuReconnectPartyConnect(CSegment* pParam);
	virtual void           OnMcuSetEndTimeConnect(CSegment* pParam);
	void                   SetEndTime(CSegment* pParam, BOOL bSetAlertTout = TRUE);
	void                   OnMcuSetAutoRedial(CSegment* pParam);
	void                   OnPartyEndDel(CSegment* pParam);

	void                   OnMcuTerminateConfConnect(CSegment* pParam);
	void                   OnMcuForceKillAnyCase(CSegment* pParam);

	void                   OnMcuExportParty(CSegment* pParam);

	void                   OnConfRemovePartyConnection(CSegment* pParam);
	void                   OnConfRemovePartyConnectionTerminate(CSegment* pParam);

	void                   OnH323LobbyAddPartyConnect(CSegment* pParam);
	void                   OnH323LobbyRejectParty(CSegment* pParam);
	void                   OnSipLobbyAddPartyConnect(CSegment* pParam);
	void                   OnSipLobbyRejectParty(CSegment* pParam);

	void                   OnLobbyRelaseAvMcuPartyConnect(CSegment* pParam);

	void                   OnPartyEndAddConnect(CSegment* pParam);

	void                   OnPartyEndExport(CSegment* pParam);
	void                   OnPartyEndImport(CSegment* pParam);
	void                   OnPartyEndDelConnect(CSegment* pParam);
	void                   OnPartyRedialConnect(CSegment* pParam);
	void                   OnPartyEndDelTerminate(CSegment* pParam);
	void                   OnPartyLayoutChanged(CSegment* pParam);
	void                   OnSourcePartyIsConfReadyForMoveConnect(CSegment* pParam);
	void                   OnSourcePartyIsConfReadyForMoveIdle(CSegment* pParam);

	virtual void           OnPartyDisConnectConnect(CSegment* pParam);
	void                   OnPartyImportParty(CSegment* pParam);

	void                   OnAudBrdgConnectSetup(CSegment* pParam);
	void                   OnAudBrdgDisConnectTerminate(CSegment* pParam);
	void                   OnVidBrdgConnectSetup(CSegment* pParam);
	void                   OnVidBrdgDisConnectTerminate(CSegment* pParam);
	void                   OnFeccBrdgConnectSetup(CSegment* pParam);
	void                   OnFeccBrdgDisConnectTerminate(CSegment* pParam);
	void                   OnContentBrdgDisConnectTerminate(CSegment* pParam);
	void                   OnContentBrdgDisConnectConnect(CSegment* pParam);
	void                   OnContentBrdgDisConnect(CSegment* pParam);
	void                   OnContentBrdgConnectSetup(CSegment* pParam);

	void                   OnContentBrdgStartContentCONNECT(CSegment* pParam);
  void                   OnContentBrdgStopContentCONNECT(CSegment* pParam);

  // XCode
  void                   OnXCodeBrdgConnectSetup(CSegment* pParam);
	void                   OnXCodeBrdgConnectConnect(CSegment* pParam);
	void                   OnXCodeBrdgDisConnectTerminate(CSegment* pParam);
	void                   OnXCodeBrdgDisConnectConnect(CSegment* pParam);
	void                   UpdateXCodeBrdgInterfaceForEachPartyCntl();
	BYTE                   SelectContentRate(bool isUpdateIpRate = false, eXcodeRsrcType eXCodeEncoderType = eXcodeEncoderDummy);
	BYTE                   SelectContentProtocol(BYTE bTakeCurrent);
	bool                   SelectContentHDResolution(BYTE newContentRateAMC, BYTE newContentProtocol, BYTE& HDMpi, BOOL isHighestCommonRequired = TRUE,eXcodeRsrcType eXCodeEncoderType = eXcodeEncoderDummy);
	BYTE                   SelectContentH264HighProfile();  //HP content
	BYTE                   GetContentHD1080SupportedByConfSettings(BYTE AMCContentRate = 0);
	void                   UpdateUnifiedAndContentBrdg(BYTE ContentProtocol, CPartyConnection* pPartyConnection = NULL, BOOL bUpdateContentBrdg = TRUE, BOOL bZeroContentRate = TRUE, BOOL isContentRateChanged = FALSE);
	void                   CorrectContentModeForParty(CPartyConnection* pPartyConnection, CIpComMode* pH323NewMode, bool isRateChanged = true, bool isProtocolChanged = false, bool isTriggeredParty = false);
	void                   CorrectContentModeForIsdnParty(CPartyConnection* pPartyConnection, CComMode* pNewMode) const;
	BYTE                   FindMinPartiesContentProtocol(BYTE bTakeCurrent);
	void                   UpdateContentProtocolOnDisconnectParty();

	DWORD                  TranslateVideoIpRateToCorrectReservationRate(DWORD reservationVideoRate, DWORD h323ConfRate);
	void                   ChangeContentMode(CPartyConnection* pTriggeredPartyConnection = NULL, bool isRateChanged = true, bool isProtocolChanged = false, bool isDowngradingfToContentVSW = false);

	void                   OnPartyUpdateContentProtocolCONNECT(CSegment* pParam);
	void                   OnDelayUpdateContentTimeout(CSegment* pParam);
	void                   OnTerminalHandleEventConnect(CSegment* pParam);
	void                   OnTerminalRecurrentIntraReqTout(CSegment* pParam);

	void                   OnSetAutoTerminateTimerCONNECT(CSegment* pParam);

	void                   OnSendIpMonitorReqToParty(CSegment* pParam);

	void                   OnIvrConfTerminate(CSegment* pParam);
	void                   OnChairDroppedTerminate(CSegment* pParam);
	void                   OnIvrTimerConfTerminate(CSegment* pParam);
	void                   OnIvrUpdatePartyIvrStatus(CSegment* pParam);

	void                   OnSipSubscribeIndCONNECT(CSegment* pParam);
	void                   OnSipReferIndCONNECT(CSegment* pParam);
	void                   OnSipNotifyResponseIndCONNECT(CSegment* pParam);
	void                   OnPartyChangeVidMode(CSegment* pParam);
	void                   OnPartyReceiveReCaps(CSegment* pParam);
	void                   OnAddAndConnectRecordingLink(CSegment* pParam);
	void                   OnDisconnectRecordingLink(CSegment* pParam);
	void                   OnCGStartContent(CSegment* pParam);                          // for Call Generator
	void                   OnCGStoptContent(CSegment* pParam);                          // for Call Generator

	void                   OnMcuVideoRefreshBeforeRecording(CSegment* pParam);

	// timer events
	void                   OnTimerConnect(CSegment* pParam);
	void                   OnTimerConnectAuto(CSegment* pParam);
	void                   OnTimerIdle(CSegment* pParam);
	virtual void           OnTimerRemindConnect(CSegment* pSeg);
	void                   OnTimerMessageQueue(CSegment* pParam);

	void                   OnTimerAudioNumberChanged(CSegment* pSeg);
	void                   OnTimerAudioNumberHidden(CSegment* pSeg);

	void                   SetMuteIncomingForLectureMode(PartyRsrcID lecturerId);
	void                   SetMuteIncomingForLectureMode(const char* lecturerName);
	DWORD                  GetMediaBitRate(cmCapDataType type, cmCapDirection direction, ERoleLabel eRole) const;
	DWORD                  GetContentBitRate() const;
	BOOL                   IsEnableH239();
	BOOL                   IsTIPContentEnable() const;
	BOOL                   IsConfRateValidForH239();
	void                   UpdateContentBrdgOnLectureMode();

	void                   OnUpdateVisualNameForParty(CSegment* pParam);
	WORD                   TranslateToConfPartyConnectionType(WORD dialType);

	DWORD                  ComputeConnectingConnectionDelay(BYTE isVoice, const char* partyName, BYTE netInterfaceType);

	BOOL                   isISDNPartyExistInConf();
	BOOL                   IsNeedRecalcH264ParamsAccordingToPartySettings(CConfParty* pConfParty);
	void                   GetH264VideoParamsAccordingToPartySettings(H264VideoModeDetails& returnH264VidModeDetails, CConfParty* pConfParty, DWORD decisionRate, BOOL isHighProfile = TRUE);

	void                   CheckGWConfEstablishedWith2Parties(CSegment* pParam);
	void                   TerminateConfWith2PartiesOrLess();

	void                   GenerateGeneralDisconnectionCause(CConfParty* pConfParty,eGeneralDisconnectionCause& discCauseForGW);
	void                   GenerateGeneralDisconnectionMessageFromCause(CConfParty* pConfParty,eGeneralDisconnectionCause discCauseForGW,CMedString& cstr);

	void                   GenerateGeneralConnectedMessage(CConfParty* pConfParty, CSmallString& cstr);
	void                   GenerateGeneralDisconnectedMessage(CConfParty* pConfParty, CSmallString& cstr);
	void                   GenerateGeneralDialingMessage(CRsrvParty* pParty, CSmallString& cstr);

	void                   AddPartyMsgOnScreenForGWConf(PartyMonitorID partyId, const char* msgStr, DWORD timeout = 0, BYTE allowOverride = TRUE, BYTE displayImmediately = FALSE);

	// new functions for invite by dtmf
	bool                   IsInvitedPartyInSetupStage(PartyMonitorID partyId, std::ostringstream& msg);
	bool                   AreTextMessagesNeededForThisInvitedParty(PartyMonitorID partyId, std::ostringstream& msg);
	void                   InvitedPartyEndSetup(PartyMonitorID partyId, BOOL bRedialProcessed = FALSE);
	void                   OnDtmfInviteParty(CSegment* pParam);
	void                   OnDisconnectInvitedParticipantReq(CSegment* pParam);
	void                   AddNextPartyForInvitePartySession(DWORD partyId, BYTE discCause, CMedString& discCauseStr, int i = 1);
	void                   StartInvitePartyDialOutLoop(DWORD inviterPartyId);
	void                   OnDtmfInvitedPartyConnected(CConfParty* pConfParty);
	void                   AddPartyMsgOnScreenForInvitedConf(PartyMonitorID partyId, const char* msgStr, DWORD timeout = 0, BYTE allowOverride = TRUE, BYTE displayImmediately = FALSE);
	DWORD                  TestDtmfInvitedPartyValidity(CRsrvParty* pRsrvParty);
	void                   OnDtmfInvitePartyConfEndSetup(CSegment* pParam);

	void                   StopTextOnScreenMessages(CSegment* pParam);
	void                   OnPartyEndInitComm(CSegment* pParam);

	void                   CalcGwPartiesStateFlag();

	DWORD                  TestGWPartyValidity(CRsrvParty* pParty);
	DWORD                  TestPartyNameValidity(CRsrvParty* pParty);

	void                   OnGateWayConfEndSetup(CSegment* pParam);
	void                   GateWayPartyEndSetup(DWORD partyId,BOOL bRedialProcessed = FALSE);

	void                   OnUpdateIsdnDialOutServiceNameForGW(CSegment* pParam);
	void                   OnUpdateDialString(CSegment* pParam);
	BYTE                   ParseString(char* partyNumOrIp, char*& dialString, BYTE isPstn = NO);
	BYTE                   IsContainingNumbers(char* stringToCheck);

	void                   ReplaceChar(char* str, char charToReplace, char newChar);

	bool                   IsGWPartyInSetupStage(PartyMonitorID partyId, std::ostringstream& msg);
	bool                   AreTextMessagesNeededForThisParty(DWORD partyId, std::ostringstream& msg);

	void                   OnVideoDecoderInitialSync(CSegment* pParam);
	void                   OnGateWayPartyConnected(CConfParty* pParty);
	void                   FailToStartGWSession();

	void                   InitPartiesFromString(CConfPartiesDialingSequence* pConfPartiesDialingSequence, CCommRes* pMR, char* dialString);
	DWORD                  AddPartyDialingSequence(CConfPartiesDialingSequence* pConfPartiesDialingSequence, CCommRes* pMR, char* dialString, map<WORD, WORD>& interfaceOrder);
	DWORD                  AddPartyDialingSequenceForGw(CConfPartiesDialingSequence* pConfPartiesDialingSequence, CCommRes* pMR, char* dialString, BYTE isPSTN, CLargeString& cstr);
	void                   StartDialOutLoopForGw(PartyMonitorID inviterPartyId);
	void                   AddNextPartyForGWSession(PartyMonitorID partyId, BYTE discCause, CMedString& discCauseStr, int i = 1);
	BOOL                   HandlePartyDialRetry(PartyMonitorID partyId);

	// LEGACY
	void                   InformVideoBridgeStartContentPresentation(DWORD activeProtocol, DWORD activeContentRate);
	void                   InformVideoBridgeStopContentPresentation();
	void                   OnVideoBridgeContentDecoderAllocFail();
	void                   OnVideoBridgeContentDecoderResetFailStatus();

	// COP_VIDEO_BRIDGE
	void                   InitCopEncodersVideoOutParams(CVideoBridgeInitParams* pVideoBridgeInitParams);
	void                   InitPartyXCodeParamsAccordingToEncoderType(eXcodeRsrcType eEncoderPartyType, CBridgePartyVideoParams* pBridgePartyVideoParams,BYTE isForUpdateEncoder = FALSE);
	eXcodeRsrcType         ConnectPartyToXCodeEncoder(CPartyConnection* pPartyConnection,CConfParty* pConfParty);

	void                   DowngradeConferenceFromXCodeToVSW();
	void                   ActionsOnStartingContentSession(BYTE selectedContentRateAMC, bool isDowngradingfToContentVSW = FALSE);
	BYTE                   DoesPartyMatchContentEncoder(CPartyCntl* pPartyCntl,eXcodeRsrcType ePartyContentEncoderType);
	void                   GetCopLevelVideoParams(WORD encoder_index, DWORD& videoAlg, DWORD& videoBitRate, eVideoResolution& videoResolution, eVideoFrameRate& videoFrameRate, DWORD& h264_MBPS, DWORD& h264_FS, DWORD& sampleAspectRatio);
	void                   GetCopEncoderResourceParams(WORD encoder_index, DWORD& copConnectionId, DWORD& copPartyId, eLogicalResourceTypes& copLrt);
	void                   CreateCopEncoderVideoOutParams(CBridgePartyVideoOutParams& bridgePartyVideoOutParams, DWORD videoAlg, DWORD videoBitRate, eVideoResolution videoResolution, eVideoFrameRate videoFrameRate, DWORD h264_MBPS, DWORD h264_FS, DWORD sampleAspectRatio, DWORD copConnectionId, DWORD copPartyId, WORD copResourceIndex, eLogicalResourceTypes copLrt);

	// PCM
	void                   OnPCMInviteParty(CSegment* pParam);
	void                   RespondPCMManagerInviteFailed(CSegment* pParam);
	void                   RespondPCMManagerInviteSucceed(CSegment* pParam = NULL);
	DWORD                  InsertInvitedPartyToSequence(char* dialString, WORD interfaceType);
	void                   AddNextInvitedParty(DWORD partyId, int i = 1);
	void                   OnInvitedPartyConnected(CConfParty* pConfParty);
	void                   SetVisualNameForInvitedParty(CConfParty* pConfParty);
	void                   FailToStartDtmfInvitedSession();

	BOOL                   IsAlwaysForwardDtmfInGWSessionToIsdn();

	void                   OnLobbyAddPartyConnect(CSegment* pParam);
	void                   OnLobbyAddPartyChannelDescConnect(CSegment* pParam);
	void                   LobbyAddPartyVoiceConnect(CSegment* pParam, CTaskApp* pParty, char* name, CConfParty* pConfParty);
	void                   LobbyAddPartyIsdnVideoConnect(CSegment* pParam, CTaskApp* pParty, char* name, CConfParty* pConfParty);
	void                   UpdatePartyState(CConfParty* pConfParty);
	void                   OnIvrShowParticipants(CSegment* pParam);
	void                   OnIvrShowGathering(CSegment* pParam);
	void                   OnIvrMuteAllButX(CSegment* pParam);
	void                   OnOperatorAssistance(CSegment* pParam);
	void                   OnIVRPartyPassedEntranceProcedCONNECT(CSegment* pParam);
	void                   AddISDNPartyForAllServicesToSequence(PARTY_SEQUENCE_LIST* pList, CCommRes* pMR, DWORD partyId, char* dialString, BYTE isPstn);
	void                   AddIPPartyForAllServicesToSequence(PARTY_SEQUENCE_LIST* pList, DWORD partyId, char* dialString, WORD interfaceType, DWORD callSignallingPort);

	// Recording conference
	void                   OnUpdateRecordingControlCONNECT(CSegment* pParam);

	// Gathering
	void                   GatheringUpdateTimerHandler(CSegment* pParam);
	void                   GatheringConnectingTimerHandler(CSegment* pParam);
	void                   OnGatheringPartyConnected(CSegment* pParam);
	void                   FallBackFromIceToRegularSip(CSegment* pParam);

	//Multiple links for ITP in cascaded conference feature:
	void                  OnAddSubLinksPartiesConnect(CSegment* pParam);
	void                   DisconnectAllRoomControlLinks(const char* mainLinkName , BYTE cascadedLinkNunber,WORD mode);
	void                   SendToMainLinkThatSubWasDisconnectedOrDeleted(CPartyConnection*  pPartyConnection);
	void                   SetOriginalName(CPartyConnection*  pPartyConnection, CConfParty* pConfParty);
	void                   SendToMainLinkThatSubWasAdded(CTaskApp*  pParty, CPartyConnection*  pPartyConnection, DWORD subMailBox);

	// TIP
	void                   OnAddSlavePartyConnect(CSegment* pParam);
	void                   ConnectSipSlaveParty(CConfParty* pConfParty, DWORD connectDelay, WORD tipPartyType, DWORD room_Id, DWORD pPeerPartyRsrcID, DWORD masterVideoPartyType, CIpComMode* pIpMasterInitialMode);
	void                   FallBackToRegularSip(CSegment* pParam);

	void                   OnExclusiveContentSet(CSegment* pParam);     // Restricted content
	void                   OnRemoveExclusiveContent(CSegment* pParam);  // Restricted content
	void                   OnExclusiveContentModeSet(CSegment* pParam); //Restricted content
	void                   LegacyOnDemandCheck(CPartyConnection* pPartyConnection, BYTE  isBlockContent=FALSE);
	void 				   SetIsEnableHdInAvcSvcMixMode();

	// MS Lync 2013 - AV MCU
	void                   ConnectAVMCUParty(CSipNetSetup * pNetSetup,CConfParty* pConfParty, DWORD connectDelay = 0,sipSdpAndHeaders* pSdpAndHeaders = NULL);
	void                   ConnectDMAAVMCUParty(CConfParty* pConfParty, DWORD connectDelay);
	void                   OnAddMsSlavePartyConnect(CSegment* pParam);
	void                   ConnectMsSlaveParty(CConfParty* pConfParty, DWORD connectDelay, DWORD mainPartyRsrcID, eAvMcuLinkType AvMcuLinkType, DWORD msSlaveIndex, DWORD msSsrcRangeStart, CSipCaps* remoteCaps, CVidModeH323	*pLocalSdesCap = NULL);

	BYTE                    IsOnlyRmxInAvMcuConf();
	void                    HandleEventPackageEvent(CSegment* pMsg);
	void                    OnActivateAutoTerminationTestForAvMcu();

public:
	BYTE                   GetIsGateWay()                           {return m_isGateWay;}
	void                   SendPartyMsgOnScreenForGWConf();
	void                   SetGwDtmfForwardIfNeeded();
	void                   SetDtmfForwardAllIfNeeded();
	void                   SetInvitorPartyDtmfForward();
	void                   OnTimerDtmfFwdInvitor(CSegment* pParam);

	void                   SendPartyMsgOnScreenForDtmfInvitePartyConf(DWORD partyId);

	void                   OnTimerDtmfFwdAll(CSegment* pParam);
	void                   OnTimerDelayIsdnLinkDisconnect(CSegment* pParam);
	void                   ActivateAutoTermenation(const char* callerFunc);
	void                   ActivateAutoTerminationForMsAvMcu(const char* callerFunc);

	void                   SetConfRsrcReqForCop(CONF_RSRC_REQ_PARAMS_S& confRequestParams);
	void                   SetConfRsrcReqForCopMPMPlus(CONF_RSRC_REQ_PARAMS_S& confRequestParams);
	void                   SetConfRsrcReqForCopMPMx(CONF_RSRC_REQ_PARAMS_S& confRequestParams);
	void                   HandleConfRsrcIndForCop(OPCODE opcode, CONF_RSRC_IND_PARAMS_S& confIndicationParams);
	void                   DumpCopRsrcs(CONF_RSRC_IND_PARAMS_S& confIndicationParams);
	void                   SetCopRsrcsInRoutingTable();
	void                   DeleteCopRsrcsFromRoutingTable();

	CopRsrcsParams*        GetCopResources() const                  {return m_pCopRsrcs;}
	void                   SetCopRsrcs(CONF_RSRC_IND_PARAMS_S& confIndicationParams);
	eSessionType           GetSessionTypeForCop() const;
	eLogicalResourceTypes  GetRsrcTypeForHighestCopLevel() const;
	void                   OnSendStartPreviewReqToParty(CSegment* pParam);
	void                   OnSendStopPreviewReqToParty(CSegment* pParam);
	void                   OnMcuIntraPreviewReq(CSegment* pParam);
	void                   DumpRsrcReqToTrace(CONF_RSRC_REQ_PARAMS_S& pParam);

	void                   SetCopCascadeLinkAsLecturer(CTaskApp* pParty);
	void                   SetLastRateFromMaster(const CTaskApp* pParty);
	void                   SetLastRateFromMaster(DWORD ContentRate) {m_lastContentRateFromMaster = ContentRate;}
	void                   ObtainDisplayNameFromAddressBook(CConfParty* pRsrvParty, CIpNetSetup* pIpNetSetup);
	BOOL                   SearchAddressBookByAlias(const char* AliasName, CConfParty* pRsrvParty);
	BOOL                   SearchAddressBookByIPAddress(CConfParty* pRsrvParty, const mcTransportAddress* currPartyIpAddr);
	BOOL                   SearchAddressBookBySipAddress(const char* SipAddress, CConfParty* pRsrvParty);
	BOOL                   IsTwoAliasNamesMatch(const char* alias_1, const char* alias_2);

	void                   OnMuteIncomingLectureModeSet(CSegment* pParam);

	// Added for Redial on Wrong Number
	void                   SaveInviteResult(DWORD partyId, WORD interfaceType, eGeneralDisconnectionCause disconnectionCause);
	void                   CheckInviteResult(DWORD partyId, eGeneralDisconnectionCause &disconnectionCause);
// ContentTranscoding
	void                   SetConfRsrcReqForXCode(CONFERENCE_RSRC_REQ_PARAMS_S& confRequestParams);
	void                   SetConfRsrcDeallocReqForXCode(CONFERENCE_RSRC_REQ_PARAMS_S& confRequestParams);
	void                   SetXcodeRsrcs(CONFERENCE_RSRC_IND_PARAMS_S& confIndicationParams);
	void                   DumpXcodeRsrcs(CONFERENCE_RSRC_IND_PARAMS_S& confIndicationParams);
	void                   DumpConfMapListOfXcodeRsrcs();
	void                   DumpXCodeRsrcReqToTrace(CONFERENCE_RSRC_REQ_PARAMS_S& pParam);
	void                   SetXcodeRsrcsInRoutingTable();
	void                   DeleteXcodeRsrcsFromRoutingTable();
	void                   HandleConfRsrcIndForXCode(OPCODE opcode, CONFERENCE_RSRC_IND_PARAMS_S& confIndicationParams);
	void                   InitAllXCodePortsParams(CVideoBridgeInitParams* pVideoBridgeInitParams);
	void                   GetH264ContentParams(ePresentationProtocol contentProtocolMode, APIU16& profile, APIU8& level, long& mbps, long& fs, long& dpb, long& brAndCpb, long& sar, long& staticMB) const;
	CPartyRsrcDesc*        GetXCodeContentDecoderPartyRsrc();
	DWORD                  GetContentDecoderXCodeMonitorPartyId();
	eXcodeRsrcType         GetPartyXCodeEncoderType(CPartyConnection* pPartyConnection,CConfParty* pConfParty);

	CVideoOperationPointsSet* GetVideoOperationPointsSet(){return m_pConfOperationPointsSet;}
	DWORD                  SelectMaxContentRateForXCodeDecoder();

	// Sends the statistics to SNMP
	void                   SendMessageToSNMP(eTelemetryType eType, DWORD value);
	//VNGR-26449 - un-encrypted conference message
	void                   SetNumOfUnencryptedParty(int NumOfUnencryptedParty) { m_numOfUnencryptedParty = NumOfUnencryptedParty; }
	int                    GetNumOfUnencryptedParty()                          { return m_numOfUnencryptedParty; }

	WORD                   GetAudioParticipantsNum()                           { return m_AudioParticipantsNumber; }

	//Move SVC Event Package Subscriber
	bool                   DisconnectSubscriberByRsrcID(DWORD dwPartyRsrcId, CSegment& seg);
	bool                   ConnectSubscriberByRsrcID(DWORD dwPartyRsrcId);
	bool                   DeleteSubscriberByRsrcID(DWORD dwPartyRsrcId);
	bool                   UnchainSubscriberByRsrcID(DWORD dwPartyRsrcId, CSegment& seg);
	bool                   UnchainSubscriberByMonitorID(DWORD dwPartyMonitorId, CSegment& seg);
	bool                   ChainSubscriber(CSegment& seg,DWORD dwNewPartyMonitorId);
	void                   ConnectPartyToXcodeBridge(CTaskApp *pParty );

	DWORD                  GetLocalContentModeAMCInIPRate(CConfParty* pConfParty, DWORD setupRate);
	BYTE                   GetContentModeAMC();
	void                  SetControlDataParams(PartyControlDataParameters& partyControlDataParams, CConfParty* pConfParty, CIpNetSetup* pIpNetSetup, CTaskApp* pParty, CSipCaps* pRmtSipCaps, BYTE bIsOffer, BYTE bIsMrcHeader = FALSE, BYTE bIsWebRtcCall = FALSE, RemoteIdent epType = Regular, LyncConnType lyncEpType = No_Lync, DWORD roomID = 0, eTypeOfLinkParty partyType = eRegularParty, DWORD setupRate = 0, eIsUseOperationPointsPreset isUseOperationPointesPresets = eIsUseOPP_No);
	void                   AjustPartyAndRoomIdByLinkType(PartyControlDataParameters& partyControlDataParams, CConfParty* pConfParty);
	void                   SetControlInitParams(PartyControlInitParameters& partyControInitParam, CConfParty* pConfParty, BYTE bIsRedial, const char* avServiceNameStr, WORD welcomeMsgTime, DWORD connectDelay, DWORD redialInterval, COsQueue* pPartyRcvMbx);
	void                   CreateConfPartyIfNeeded(char* name);
	void                   SetupConfPartyOnH323(CConfParty* pConfParty, CH323NetSetup* pIpNetSetup);
	EStat                  SetupConfPartyOnSIP(CConfParty* pConfParty, CSipNetSetup* pNetSetup, BYTE bIsMrcHeader, BYTE bIsRemoteSlave);
	BOOL                   GetIsMrcCall(BYTE bIsMrcHeader, CConfParty* pConfParty);
	std::string            GetProductTypeAsString() const;

	void                   AddAvMCUParty(const char* avMcuFocusUri);
	char*                  GetSipUriAddressType(const char* avMcuFocusUri);
	PartyMonitorID         FindMatchingConversationId(CConfParty* pAVMCUConfParty);
	void                   VideoRecoverySendByeToParty(CSegment* pParam);
	void  		DisconnectRdpGw(void);

	// TELEPRESENCE_LAYOUTS
	void                   OnSetTelepresenceLayoutModeSetup(CSegment* pParam);
	void                   OnSetTelepresenceLayoutModeConnected(CSegment* pParam);


protected:

	typedef struct TELEPRESENCE_ON_OFF_PARAMS_S
	{
		BYTE                isSameLayout;
		BYTE                isAutoLayout;
		BYTE                isAutoBrightness;
		CLectureModeParams* pLectureModeParams;

		TELEPRESENCE_ON_OFF_PARAMS_S()
		{
			isSameLayout       = 0;
			isAutoLayout       = 0;
			isAutoBrightness   = 0;
			pLectureModeParams = NULL;
		}
	} TELEPRESENCE_ON_OFF_PARAMS_S;


	ConfRsrcID                          m_ConfRsrcId;
	ConfMonitorID                       m_monitorConfId;
	char                                m_name[H243_NAME_LEN];

	CCommConf*                          m_pCommConf;
	CAudioBridgeInterface*              m_pAudBrdgInterface;
	CVideoBridgeInterface*              m_pVideoBridgeInterface;
	CFECCBridge*                        m_pFECCBridge;


	CVideoBridgeInterface*              m_pContentXcodeBridgeInterface;

//for china usages only
public:
	CContentBridge*                     m_pContentBridge;

protected:
	CConfAppMngrInterface*              m_pConfAppMngrInterface;
	CMcmsPCMManager*                    m_pMcmsPCMManager;

	CTerminalNumberingManager*          m_pTerminalNumberingManager;

	CPartyList*                         m_pPartyList;

	CUnifiedComMode*                    m_pUnifiedComMode;          // For CP

	CConfPartiesDialingSequence*        m_pInvitedDialingSequence;  // Invite from PCM
	// Added for Redial on Wrong Number, used by both GW and DTMF Invite
	PARTIES_INVITE_RESULTS              m_PartiesInviteResults;
	PARTIES_REDIAL_NUM                  m_PartiesRedialNum;
	PARTIES_INVITE_TYPE                 m_PartiesInviteType;
	PARTIES_INVITOR_ID                  m_PartiesInvitorId;
	BOOL                                m_bEnableRedialOnWrongNumber;
	////////////  GW /////////////////////////////////////
	CConfPartiesDialingSequence*        m_pDialingSequence;
	CConfPartiesDialingSequence*        m_pDtmfInvitedPartyDialingSequence;
	GW_PARTIES_STATUS*                  m_pGWPartiesState;
	CTextOnScreenMngrForGwSession*      m_pTextOnScreenMngrForGwSession;

	CTextOnScreenMngrForInvitedSession* m_pTextOnScreenMngrForInvitedSession;
	INVITED_PARTIES_STATUS*             m_pInvitedPartiesState;
	BYTE                                m_isDtmfInviteParty;
	BYTE                                m_invitedPartiesStateFlag;

	BYTE                                GwPartiesStateFlag;
	BYTE                                m_isGateWay;
	char                                m_IsdnDialOutServiceNameForGW[NET_SERVICE_PROVIDER_NAME_LEN];
	std::string                         serviceName;

	// VNGR-23989
	PARTIES_TO_RELEASE_FROM_LOBBY       m_PartiesToReleaseFromLobby;
	WORD                                m_isAudConnected;
	WORD                                m_isVidConnected;
	WORD                                m_isFeccConnected;
	WORD                                m_isContentConnected;
	WORD                                m_isContentXCodeConnected;
	WORD                                m_isUpdateContentPending;

	WORD                                m_inCallCounter;  // for names of undefined dial in parties in conf

	char                                m_aConfGUID[16];

	WORD                                m_master;
	BYTE                                m_StandByStart;

	BYTE                                m_confTerminateCause;
	WORD                                m_isBridgeError;

	DWORD                               m_AutoTerminateAfterLastQuit;
	DWORD                               m_AutoTerminateBeforeFirstJoin;
	WORD                                m_IsAnyPartyWasConnected;
	WORD                                m_AccumulatedExtensionTime;
	eConfMediaState                     m_confMediaState;

	// Relevant for GW conf
	WORD                                m_TcMode;         // VS only / Smart VS / TR only.//? talya

	BYTE                                m_initTimerDtmfForwarding;

	// Sip conference Package
	CSIPEventPackageDispatcher*         m_pSipEventPackage;
	CSvcEventPackageDispatcher*         m_pSvcEventPackage;

	// Blast Connection handling
	TICKS                               m_firstParticipantConnetingTicksTime;
	TICKS                               m_lastPartyConnectedInTicks;
	WORD                                m_numOfBlastParticipants;
	// Blast Disconnection handling
	TICKS                               m_firstParticipantDisconnectingTicksTime;
	TICKS                               m_lastPartyDisconnectedInTicks;
	WORD                                m_numOfBlastDisconnectingParticipants;
	BYTE                                m_isLastPartyConnectedAudioOnly;
	BYTE                                m_LastPartyInterfaceType;
	// Content H.C.
	DWORD                               m_CurrentIpBitRateForContentSession;

	CGatheringManager*                  m_pGatheringManager;
	CVisualEffectsParams*               m_pOriginalVisualEffects;
	BYTE                                m_isOriginalCropping;

	TELEPRESENCE_ON_OFF_PARAMS_S*       m_pTelepresenceOnOffParams;

	DWORD                               m_lastContentRateFromMaster;
	BYTE                                m_initialSync;
	BYTE                                m_startDialingOutGW;

	CPartyConnection*                   m_pGwDtmfForwarderConnection;

	void SetConfOperationPoints();

	// COP_VIDEO_BRIDGE
	// COP data members
	CopRsrcsParams*                     m_pCopRsrcs;
  XCODE_RSRC                          m_mapXCodeRsrc;
  BYTE                                m_contentXcodeSrcProtocol; // Content Speaker Protocol
	DWORD                               m_rsrcDeallocateStatus;
	BOOL                                m_isAllocateConfResourcesFailed;  // in case when we fail to allocate resources for the conference in the clean up we don't need to deallocate.

	WORD                                m_invitePartyDtmfForwardDec;
	map<DWORD, DWORD>                   m_invitePartyMap;                 // key - invitor, value - invited
	PartyMonitorID                      m_lastInvitedParticipantId;
	PDECLAR_MESSAGE_MAP

	//VNGR-26449 - unencrypted conference message
	int                                 m_numOfUnencryptedParty;

	CVideoOperationPointsSet*           m_pConfOperationPointsSet;// We may move it to commres...

	BYTE                                m_IsAsSipContentEnable;
	DWORD                               m_AvMcuPartyRsrcId;

	WORD                                m_AudioParticipantsNumber;   //indication for audio participants icon displayed on layout
	BOOL                                m_IsAudioParticipantsNumberNotSent; //whether the audio number changed but not update the indication icon
};


#endif /* _CONF_H__ */

