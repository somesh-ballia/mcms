// ConfPartyManager.h

#if !defined(_ConfPartyMANAGER_H__)
#define _ConfPartyMANAGER_H__

#include <set>
#include <map>
#include "ManagerTask.h"
#include "ConfIpParameters.h"
#include "ConfPartyProcess.h"
#include "ConfPartyRoutingTable.h"
#include "ConfPartyDispatcherTask.h"
#include "Trace.h"
#include "Macros.h"
#include "IVRManager.h"
#include "CommRes.h"
#include "ConfApi.h"
#include "Request.h"
#include "RsrvPartyAdd.h"
#include "RsrvRecordLinkPartyAdd.h"
#include "RsrvPartyAction.h"
#include "CommResAdd.h"
#include "CommConfSpecific.h"
#include "CommConfDB.h"
#include "IVRServiceAdd.h"
#include "IVRServiceDel.h"
#include "IVRLanguageAdd.h"
#include "IVRServiceSetDefault.h"
#include "InternalProcessStatuses.h"
#include "CUpdateInfo.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "RtmIsdnMngrCommonMethods.h"
#include "MoveAction.h"
#include "CustomizeDisplaySettingForOngoingConfConfiguration.h"
#include "PrecedenceSettings.h"
#include "PlcmCdrEventConfOperatorMoveParty.h"
#include "IVRServiceConvertSlide.h"

// also defined in ...\vob\MCMS\McmIncld\MPL\Card\PhysicalPortVideo\VideoStructs.h
#define MAX_VIDEO_PORTS_PER_UNIT 8

enum eSlaveSyncElement
{
	eSlaveSyncIvrServices,
	eSlaveSyncProfiles,
	eSlaveSyncMeetingRooms,
	eSlaveSyncRecordingLinks,
	eNumOfSlaveSync
};

class CConfPartyManager: public CManagerTask
{
	CLASS_TYPE_1(CConfPartyManager, CManagerTask)
	virtual const char* NameOf() const { return GetCompileType(); }

public:
	CConfPartyManager();
	virtual ~CConfPartyManager();

	void CreateDispatcher();
	void* GetMessageMap();

	TaskEntryPoint GetMonitorEntryPoint();
	TaskEntryPoint GetLobbyEntryPoint();
	void SelfKill();
	virtual void ReceiveAdditionalParams(CSegment* pSeg);

	virtual int GetTaskMbxBufferSize() const { return 4096 * 1024 - 1; } //4MB
	virtual void SendAdditionalParams(CSegment* pSeg);

	typedef std::map<DWORD, DWORD> TaskIdToConfId;
	typedef std::map<DWORD, DWORD> TaskIdToPartyId;
	typedef std::vector< COsQueue > CLIENT_RSP_MBX_LIST;
	typedef std::map<std::string, DialogState> ProceedingMedia;

	void IVRDialogStartMsg(DialogState& state); //AT&T
	void IVRDialogPrepareMsg(DialogState& state); //AT&T

protected:

	virtual unsigned int GetMaxLegitimateUsagePrecents() const { return 50; }
	virtual void ManagerPostInitActionsPoint();
	virtual void DeclareStartupConditions();
	virtual void ManagerStartupActionsPoint();
	void CreateDefaultConfig();
	void UpdateGWDefaultProfile();
	void CreateDefaultsGWSessionAndSendToRsrc();
	BOOL IsNeedToAddGWDefaults(BYTE checkIfNeedToUpdate = FALSE);
	BOOL IsNeedToUpdateGWDefaults() { return IsNeedToAddGWDefaults(TRUE); }
	void UpdateSystemCapacityLimits();

	// Create default profiles
	STATUS CreateDefaultProfiles();
	STATUS CreateDefaultProfile_FactoryVideo();
	STATUS CreateDefaultProfile_FactorySvcVideo();
	STATUS CreateDefaultProfile_FactoryMixCpVideo();
	STATUS CreateDefaultProfile_FactoryGW();
	STATUS CreateDefaultProfile_Cop(int profileIndex);

	// COP Mode with MPM Plus mode
	void AddActiveAlarmForCOPWithMPMPlus();

	// Action functions

	//EMA
	STATUS OnServerAddRsrv(CRequest *pRequest);
	STATUS OnServerAddRsrvContinue(CRequest *pRequest);
	STATUS OnServerAddRsrvContinue1(CRequest *pRequest);
	STATUS OnServerAddRsrvContinue2(CRequest *pRequest);
	STATUS OnServerAddRepeatedRsrv(CRequest *pRequest);
	STATUS OnServerTerminateConf(CRequest *pRequest);
	STATUS OnServerDelMR(CRequest* pRequest);
	STATUS OnServerUpdateAction(CRequest* pRequest);
	STATUS OnServerDelProfile(CRequest* pRequest);
	STATUS OnServerDelConfTemplate(CRequest* pRequest);
	STATUS OnServerSetTransitEQ(CRequest* pRequest);
	STATUS OnServerCancelTransitEQ(CRequest* pRequest);
	STATUS OnServerMoveParty(CRequest* pRequest);
	STATUS OnServerAttendParty(CRequest* pRequest);
	STATUS OnServerMovePartyBackToHomeConf(CRequest* pRequest);
	STATUS OnServerSetSiteName(CRequest *pRequest);
	STATUS OnServerSetCustomizeDisplayForConf(CRequest *pRequest);
	//Resources
	void OnRsrcStartConfInd(CSegment* pMsg);
	void OnRsrcStartConf(CCommResApi* pCommResApi);
	void OnRsrcAddMeetingRoomInd(CSegment* pMsg);
	void OnRsrcAddMeetingRoom(CCommResApi* pCommResApi);
	void OnRsrcActivateMeetingRoomInd(CSegment* pMsg);
	void OnRsrcDeActivateMeetingRoomInd(CSegment* pMsg);
	void OnRsrcDelRsrvConfInd(CSegment* pMsg);
	void OnRsrcDelMRInd(CSegment* pMsg);
	void OnRsrcUpdateMeetingRoomInd(CSegment* pMsg);
	void OnRsrcSetConferenceEndTimeInd(CSegment* pMsg);
	void OnRsrcReadyInd(CSegment* pMsg);
	void OnRsrcUpdateNumericIdInd(CSegment* pMsg);

	//GKManager
	void OnGKManagerResourceQuery(CSegment * pSeg);
	void OnIpResourceReportInd(CSegment * pSeg);

	//LOBBY
	void AddUnreservedParty(CSegment* pParam);
	void RemoveUnreservedParty(CSegment* pParam);
	void OnLobbyStartMeetingRoom(CSegment* pSeg);
	void OnLobbyStartAdHocConf(CSegment* pSeg);
	void OnLobbyStartGateWayConf(CSegment* pSeg);
	//END LOBBY
	void OnStartAssistTask(CSegment* pSeg);

	//CONF
	void OnAddRecordingLinkParty(CSegment* pSeg);
	void OnDisconnectRecordingLinkParty(CSegment* pSeg);
	//END CONF

	STATUS CreateProfile(CCommRes* commRes);
	STATUS AddProfile(CCommRes* commRes);
	STATUS UpdateProfile(CCommRes* commRes);
	STATUS UpdateMR(CCommRes * commRes);
	STATUS AddConfTemplate(CCommRes* commRes);
	STATUS UpdateConfTemplate(CCommRes* commRes);
	STATUS StartOnGoingConf(CCommRes& rsrv, char* dialString = NULL, char* isdnServiceName = NULL);

	void RejectMeetingRoomOrAdHocActivation(
		DWORD mrId,
		STATUS status,
		BOOL sendDeactivationToRA = FALSE,
		BOOL isAdHoc = FALSE,
		char* adHocConfName = NULL);

	STATUS SyncExchangeProcessNotification(CCommResApi& pNewConf);
	STATUS OnServerAddPartyToConf(CRequest *pRequest);

	STATUS OnServerAddPartyToConfByType(
		CRequest *pRequest,
		DWORD undefId,
		eSipFactoryType factoryType = eNotSipFactory,
		eTypeOfLinkParty partyType = eRegularParty);

	STATUS OnDelParty(CRequest *pRequest, DWORD undefId = 0);
	STATUS OnServerDelParty(CRequest *pRequest, DWORD undefId = 0);
	STATUS OnServerUpdateParty(CRequest *pRequest);
	STATUS OnServerSetConnectOrDisconnect(CRequest *pRequest);
	STATUS OnServerSetConfVideoLayout(CRequest *pRequest);
	STATUS OnServerSetPartyVideoLayout(CRequest *pRequest);
	STATUS OnServerSetPartyPrivateLayoutButtonOnly(CRequest *pRequest);
	STATUS OnServerSetAudioVolume(CRequest *pRequest);
	STATUS OnServerSetVisualName(CRequest *pRequest);
	STATUS OnServerSetAGC(CRequest *pRequest);
	STATUS OnServerSetLeader(CRequest *pRequest);
	STATUS OnServerSetBillingData(CRequest *pRequest);
	STATUS OnServerConfContactInfo(CRequest *pRequest);
	STATUS OnServerPartyContactInfo(CRequest *pRequest);
	STATUS OnServerSetListeningAudioVolume(CRequest *pRequest);
	STATUS OnServerSetAudioVideoMute(CRequest *pRequest);
	STATUS OnServerSetAudioBlock(CRequest *pRequest);
	STATUS OnServerUpdateConfAutoLayout(CRequest *pRequest);
	STATUS OnServerUpdateVisualEffects(CRequest *pRequest);
	STATUS OnServerWithdrawContentToken(CRequest *pRequest);
	STATUS OnServerSendContentRequest(CRequest *pRequest); // for Call Generator
	STATUS OnServerStopContentRequest(CRequest *pRequest); // for Call Generator
	STATUS OnServerUpdateConfLectureModeParams(CRequest *pRequest);
	STATUS OnServerUpdateEndTime(CRequest *pRequest);
	STATUS OnConfIpServiceParamsInd(CSegment* pSeg);
	STATUS OnConfIpServiceParamsEndInd(CSegment* pSeg);
	STATUS OnConfDeleteIpeServiceParamsInd(CSegment* pSeg);
	STATUS OnTimerIPServiceFromCSMngrTout(CSegment* pSeg);

	void OnCSMngrServiceCfgUpdate(CSegment* pSeg);
	void OnCSMngrServiceDefaultUpdate(CSegment* pSeg);

	STATUS OnConfBlockInd(CSegment* pSeg);
	STATUS OnGetNumOfConferences(CSegment* pSeg);
	STATUS OnServerUpdateConfVideoClarity(CRequest *pRequest);
	STATUS OnServerIntraRequest(CRequest *pRequest);
	STATUS OnSetResolutionThreshold(CRequest* pSetRequest);

	STATUS OnConfPartyReportOnActiveAlarmInd(CSegment* pSeg);
	STATUS OnSpecificMeetingRoomInformationReq(CSegment* pSeg);
	STATUS OnServerStartPreview(CRequest *pRequest);
	STATUS OnServerStopPreview(CRequest *pRequest);
	STATUS OnServerRequestIntra(CRequest *pRequest);
	STATUS OnServerUpdateConfAutoRedial(CRequest *pRequest);
	STATUS OnServerSetMessageOverLay(CRequest *pRequest);
	STATUS OnServerSetPartyMessageOverLay(CRequest *pRequest);
	STATUS OnServerUpdateAutoScanInterval(CRequest *pRequest);
	STATUS OnServerUpdateAutoScanOrder(CRequest *pRequest);
	STATUS OnRsrcExtractPartyInfo(CSegment* pMsg);
	STATUS OnServerUpdateTelepresenceLayoutMode(CRequest* pRequest);// TELEPRESENCE_LAYOUTS

	void OnRsrcStartPreviewInd(CSegment* pMsg);
	void OnRsrcStopPreviewInd(CSegment* pMsg);

	// ISDN service
	STATUS OnRtmIsdnParamsInd(CSegment* pSeg);
	STATUS OnRtmIsdnParamsEndInd(CSegment* pSeg);
	STATUS OnRtmIsdnDefaultServiceNameInd(CSegment* pSeg);
	STATUS OnRtmDeleteIsdnServiceParamsInd(CSegment* pSeg);

	STATUS OnServerAddIVRService(CRequest* pRequest);
	STATUS OnServerUpdateIVRService(CRequest* pRequest);

	int CheckIVRLegalNames(const CAVmsgService* pAVmsgServ);
	int CheckIvrFile(char *fullPathMsgName);

	STATUS OnServerDeleteIVRService(CRequest* pRequest);
	ESTATUS IsAVServiceUsedBySomeConf(const char* ivrServiceName);
	STATUS OnServerAddIVRLanguage(CRequest* pRequest);
	STATUS OnServerSetDefaultIVRService(CRequest* pRequest);
	STATUS OnServerSetDefaultEQService(CRequest* pRequest);
	STATUS OnServerConfEntryPW(CRequest *pRequest);
	STATUS OnServerSetConfChairPass(CRequest *pRequest);
    STATUS OnServerConvertSlide(CRequest* pRequest);

	STATUS DisconnectParty(ConfMonitorID confId, PartyMonitorID partyId, const char* operName);
	STATUS ReconnectParty(ConfMonitorID confId, PartyMonitorID partyId);
	void SendConfIPParamsReqToCS();
	void OnAddDialOutPartyToConf(CSegment* pSeg);

	STATUS MovePartyToMROOM_OR_CONF(CSegment* pParam);
	STATUS CheckVideoLayout(CVideoLayout* pVideoLayoutOper, CVideoLayout* pVideoLayoutDB, BYTE isHDVSW, const WORD isPrivate = FALSE);
	STATUS OnIvrAddMusicSourceReq(CSegment* pSeg);
	STATUS OnStartupReadMRDB(CSegment* pSeg);
	STATUS OnRAMeetingRoomDBInd(CSegment* pSeg);
	STATUS OnRsrcRemoveCardInd(CSegment* pParam);
	STATUS OnRsrcAddCardInd(CSegment* pParam);
	STATUS OnRsrcUpdateIvrCntlInd(CSegment* pParam);
	STATUS OnRsrcCardTypeInd(CSegment* pParam);

	//RecordingLink List
	STATUS OnServerAddRecordingLink(CRequest* pRequest);
	STATUS OnServerUpdateRecordingLink(CRequest* pRequest);
	STATUS OnServerDeleteRecordingLink(CRequest* pRequest);
	STATUS OnServerSetDefaultRecordingLink(CRequest* pRequest);
	BYTE IsRecordingLinkInUse(const char* recordingLinkName);

	//Cards
	void OnCardsSystemBasedModeInd(CSegment* pMsg);
	void SendSystemBasedModeReqToCardMngr();

	//McuMngr
	STATUS OnMcuMngrLicensingInd(CSegment* pSeg);
	STATUS OnMcuMngrPrecedenceSettings(CSegment* pSeg);

	void OnSystemRamSizeInd(CSegment* pMsg);
	void OnMcuSetGMTOffsetInd(CSegment* pMsg);

	//Failover
	void OnFailoverStartSlaveInd(CSegment* pMsg);
	void OnFailoverRestartSlaveInd(CSegment* pMsg);
	void OnFailoverStartMasterInd(CSegment* pMsg);
	void OnFailoverSlaveBecomeMasterInd(CSegment* pMsg);
	void OnFailoverStartMasterBecomeSlaveInd(CSegment* pMsg);
	void OnFailoverAddOrUpdateConfInd(CSegment* pSeg);
	void OnFailoverTerminateConfInd(CSegment* pSeg);
	void OnFailoverAddOrUpdateProfileInd(CSegment* pSeg);
	void OnFailoverTerminateProfileInd(CSegment* pSeg);
	void OnFailoverAddOrUpdateMeetingRoomInd(CSegment* pSeg);
	void OnFailoverTerminateMeetingRoomInd(CSegment* pSeg);
	void OnFailoverIVRServiceListInd(CSegment* pSeg);
	void OnFailoverRecordingLinksListInd(CSegment* pSeg);
	void OnDeleteAllConfsTimeOut(CSegment* pSeg);
	void OnMaxTimeDeleteAllConfsTimeOut(CSegment* pSeg);
	void OnKillPortAck(CSegment* pSeg);
	void OnAllocStatusPerUnitAck(CSegment* pSeg);

	STATUS OnSipProxyDBReq(CSegment* pSeg);
	STATUS OnSipProxyEndIceInit(CSegment* pSeg);
	STATUS OnWebRTCIceSeriveEndIceInit(CSegment* pSeg); //End WEbRTC Ice_init
	STATUS OnSipProxyEndIceStatus(CSegment* pParam);
	void IceInitTimeout(CSegment* pParam);

	void OnTimerAddParty(CSegment* pMsg);
	void OnTimerDelParty(CSegment* pMsg);
	void OnTimerDelMovedParty(CSegment* pMsg);
	void OnTimerAddMovedParty(CSegment* pMsg);
	void DeleteUpdateInfo(WORD ind);
	WORD InsertUpdateInfo(CUpdateInfo* pUpdate_info);
	BYTE IsUpdateCompleted();
	BYTE IsUpdateCompleted(DWORD confId);
	void DeleteMovePartyInfo(WORD ind);

	STATUS HandleColdMove(CCommConf* pSourceConf, CCommConf* pDestConf, CConfParty* pConfParty);
	WORD InsertMovePartyInfo(CRsrvParty* pMoveParty_info);
	BYTE IsMoveCompleted();

	STATUS SetDefaultIVRService(CRequest* pRequest, WORD isEQSettings);
	STATUS ValidateUpdateMR(CCommResApi * pRsrv);

	void verifyExitFromStartUpAndPerformAfterStartUpActions();
	void OnTimerIsMcuStartupFinished(CSegment*);

	STATUS MoveActions(CCommConf* pSourceConf, CCommConf* pDestConf, DWORD sourcePartyId, EMoveType eMoveType = eMoveDefault);
	STATUS SendMoveCDREvents(CCommConf* pSourceConf, CCommConf* pDestConf, DWORD sourcePartyId, EMoveType eMoveType);

	void SendPartyAudioVideoMute(CConfParty* pConfParty, CConfApi* confApi, BOOL isAudioMuted, BOOL isVideoMuted, BOOL muteSlaveFromConf = FALSE);
	void SendPartyAudioBlock(CConfParty* pConfParty, CConfApi* confApi, BOOL isAudioBlocked, BOOL blockSlaveFromConf = FALSE);

	BOOL IsRMXInSlaveMode();

	//Echange Mngr
	void   OnExchangeConfigInd(CSegment* pMsg);

	void OnTimerCheckSlideProceeding(CSegment* pSeg);

    void SaveExternalIVRFilesInfo(const std::string & url, const std::string & appServerIp); // save the downloaded External IVR files info for deleting the useless files
	void OnTimerDeleteUselessIVRFiles(CSegment* pSeg);

private: // terminal commands

	STATUS HandleTerminalFeccToken(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalMove(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleTerminalSendAquireReleaseReq(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleRemoveAddMFACard(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleBlockConfIndication(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalLPRInd(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleUpdateRsrcPrefix(CTerminalCommand& command, std::ostream& answer);
	STATUS HandlePrintDecoderDB(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleLimitDecoderResolution(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleIntraRequestFromEP(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleRecurrentIntraRequestFromEP(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalKillConf(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalPartySlowFastChange(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleSet1080p60FR(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleSpeakerInd(CTerminalCommand & command, std::ostream& answer);
	STATUS HandleTerminalKillParty(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleSetDebugVal(CTerminalCommand & command,std::ostream& answer);

	STATUS HandleSetConfAvcSvcMode(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleSetPartyAvcSvcMode(CTerminalCommand & command,std::ostream& answer);

	// for Call Generator - Vendor detection
	STATUS HandleVendorDetectionForCG(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalPCMReq(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleSendResourcesMap(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleSendEmptyResourcesMap(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleOperatorAssistance(CTerminalCommand& command, std::ostream& answer);

	STATUS OnSetExclusiveContent(CRequest *pRequest); //Restricted content
	STATUS OnRemoveExclusiveContent(CRequest *pRequest);//Restricted content
	STATUS OnServerSetExclusiveContentMode(CRequest *pRequest); //Restricted content
	STATUS OnServerSetMuteIncomingLectureMode(CRequest *pRequest);
	STATUS OnServerSetMuteAllAudioVideoPartiesExceptLeader(CRequest *pRequest);

	//void   HandlePostRequest(CSegment* pSeg);
	STATUS HandleTerminalSendDtmf(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleSetExclusiveContent(CTerminalCommand& command, std::ostream& answer);//Restricted content
	STATUS HandleRemoveExclusiveContent(CTerminalCommand& command, std::ostream& answer);//Restricted content
	STATUS OnServerRecordingCommand(CRequest *pRequest);
	STATUS OnServerSetDtmf(CRequest *pReqCreateProfilesDBuest); // for Call Generator

	STATUS GetMoveConfsFromRequest(CMoveAction* pMoveAction, CCommConf*& pSourceRequestedConf, CCommConf*& pDestRequestedConf, DWORD& partyId);
	STATUS GetMoveConfsFromRequest(CMoveBaseAction* pMoveBaseAction, CCommConf*& pSourceRequestedConf, DWORD& partyId);
	STATUS GetUserOperatorConf(CCommConf*& pOperatorConf);
	STATUS GetHomeConfForParty(CCommConf* pSourceConf, DWORD partyId, CCommConf*& pDestRequestedConf);

	//  STATUS UpdateOperatorAssistanceInDB(CCommConf* pCommConf, DWORD partyId, BYTE action = 0,BYTE mode = 0 );
	STATUS ResetOperatorAssistanceInDB(CCommConf* pSourceConf, CCommConf* pDestConf, DWORD partyId);

	STATUS HandleFailoverStartSlave(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleFailoverStopSlaveAndStartRestore(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleFailoverStartMaster(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleChangeSysMode(CTerminalCommand& command, std::ostream& answer);//2 modes cop/cp
	STATUS HandleConfPartyProcessInfo(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleProcessMemInfoPrint(CTerminalCommand& command, std::ostream& answer);

	STATUS HandleNQIndicationIcon(CTerminalCommand& command, std::ostream& answer);

	STATUS HandleIvrFilesCache(CTerminalCommand& command, std::ostream& answer); // AT&T

	// TELEPRESENCE_LAYOUTS
	STATUS HandleSetTelepresenceLayoutMode(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleLoadReserveGridMapFromXML(CTerminalCommand& command, std::ostream& answer);
	void UpdateConfTelepresenceLayoutMode(CCommConf& rCommConf, ETelePresenceLayoutMode newLayoutMode);

	void CreateRoutingTable();
	void CreateConfDB();
	void CreateLobbyTask();
	void CreateAssistTask();
	STATUS CreateRecordingLinkDB();
	STATUS CreateProfilesDB();
	STATUS CreateMeetingRoomDB();
	STATUS CreateConfTemplateDB();
	void CreateOngoingConfStore();

	// Attributes
	STATUS AddRsrv(CRequest* pRequest);

	void SendAsyncMsgToRsrcProcess(CSegment* segment, OPCODE opcode);
	void SendMsgToRsrvMngrRsrcProcess(CSegment* segment, OPCODE opcode);
	void SendNumericIdListToRsrcProcess();

	eConfMediaState GetConfType(const string &confType);
	void   EmptyDbForFailoverSlave();
	void   StopSlaveAndStartRestoreInd();
	void   TerminateOngoingConferences();
	void   TerminateOngoingConf(DWORD confID);
	void   ResetProfiles();
	void   ResetMeetingRooms();
    STATUS HandleGetConnectedPartiesNumber(CTerminalCommand& command, std::ostream& answer);
	STATUS AddProfileToSlave(CCommRes *commRes);

	void SendSlaveAddOrUpdateProfileIndToRsrc(CCommRes* pRsrvApi, OPCODE opcode);
	void TerminateConferences();
	void NotifyFailoverOnEndPrepareMasterBecomeSlave(STATUS status);
	void SendCardConfigReq();//2 modes cop/cp
	bool IsRepeatedDigitsNextDigitOK(char* psz, char chNextDigit, int nRepeatedMax);
	void AutoGeneratePasswords(CCommResApi* pRsrvApi);
	int  AllocateStringOfRandomNumbers(int length, char* pszRandomString);

	STATUS OnCsProxyConfRegisterStatus(CSegment* pSeg);
	void OnProxyConfUpdateServerType(CSegment*);
	void OnRemoveConfByTaskId(CSegment* pSeg);
	void OnConfOrPartyTaskFailed(CSegment* pSeg);
	void ConfTaskCleanup(DWORD taskId);
	void PartyTaskCleanup(DWORD taskId);
	void OnConfUpdateTaskFailed(CSegment* pSeg);
	void OnSNMPConfigInd(CSegment* pSeg);

	//AT&T
	void OnMccfIvrMsg(CSegment* pSeg);
	void OnMccfIvrFileDownloadComplete(CSegment* pSeg);
	void OnMccfIvrSlideConvertComplete(CSegment* pSeg);
	

	void OnMccfSyncMsg(CSegment* pSeg);
	void OnMccfDropMsg(CSegment* pSeg);

	void IVRDialogTerminateMsg(DialogState& state);

	void ModifyITPLayout(CVideoLayout* pVideoLayout, DWORD confId, BOOL bIsPrivate);

	void OnResourcesSetConfAvcSvcMediaStateInd(CSegment* pSeg);
	void OnLoggerUpdateNumber(CSegment* pSeg);

	void OnSipLyncBeNotify(CSegment* pSeg);
       //added by Richer for Video Recovery project om 2013.12.26
       STATUS HandleVideoRecovery(CTerminalCommand & command, std::ostream& answer);

    // Convert slide from EMA done.
    void OnEMAIvrSlideConvertComplete(CSegment* pSeg);
protected:

	CIVRManager* m_pIVRManager;

	CConfPartyProcess* m_pProcess;
	BOOL m_isConfIpParamsEndReceived;
	BOOL m_isConfIsdnParamsEndReceived;
	BOOL m_isStartupReadMRDBReqRecieved;
	DWORD m_lockConfReqCounter;
	BYTE m_bLockConfForInvalidCertificate;

	BOOL m_bIsIBMLicense; //this variable should be set according to license received from CSMngr  - CFS
	BOOL m_isSystemCardsModeReceived;
	BOOL m_isNeedToAddGWDefaults;
	BOOL m_isRsrcProcessReady;
	BOOL m_isProfilesFolderEmpty;
	BOOL m_SipProxyDBReqReceived;
	BOOL m_isDefaultGWSessionAdded;
	BOOL m_isWebRtcGWStarted;
	BOOL m_isWebRtcIceServerFailure;

	char* m_operName;

	std::vector<CUpdateInfo*> m_UpdateInfo;
	std::vector<CRsrvParty*> m_MoveInfo;
	WORD m_del_timer_num;
	WORD m_add_timer_num;

	DWORD m_dwInternalConfStatus;
	BOOL m_bExchangeConfigured;
	MR_AND_PROFILE_IND_LISTS* m_pMrAndProfileListDuringStartup;

	BOOL m_slaveSyncElements[eNumOfSlaveSync];
	CTaskApi* m_pAssistMngApi;
	BYTE m_GMT_offset;
	BYTE m_GMT_offsetSign;

	TaskIdToConfId m_TaskIdToConfId;
	TaskIdToPartyId m_TaskIdToPartyId;
	DWORD m_ConfTaskCrashesCounter;
	DWORD m_PartyTaskCrashesCounter;
	ProceedingMedia m_ProceedingSlides;
	ProceedingMedia m_ProceedingAudio;

	PDECLAR_TERMINAL_COMMANDS;
	PDECLAR_MESSAGE_MAP;
	PDECLAR_TRANSACTION_FACTORY;

	typedef map<string, BYTE> EXTERNAL_IVR_MEDIA_FILE_INFO; // <media file name, useless BYTE>
	typedef map<string, EXTERNAL_IVR_MEDIA_FILE_INFO> EXTERNAL_IVR_PROMPT_SET; // <prompt set name, useless BYTE>
	typedef map<string, EXTERNAL_IVR_PROMPT_SET > APP_SERVER_IVR_INFO; // <Application server IP, prompt set>
	APP_SERVER_IVR_INFO	m_appServerIvrInfo;
};

#endif // !defined(_ConfPartyMANAGER_H__)
