// CardsManager.h

#ifndef CARDS_MANAGER_H_
#define CARDS_MANAGER_H_

#include "ManagerTask.h"
#include "CardMngrLoaded.h"
#include "CsCommonStructs.h"
#include "IvrApiStructures.h"
#include "MplMcmsProtocol.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "RtmIsdnMngrCommonMethods.h"
#include "ICEApiDefinitions.h"
#include "IceCmReq.h"
#include "IceCmInd.h"
#include "SharedMcmsCardsStructs.h"
#include "CardsDefines.h"

class CCardsProcess;
class CMediaIpParameters;
class CEthernetSettingsStructWrapper;
class CEthernetSettingsStructWrappersList;

class CCardsManager : public CManagerTask
{
CLASS_TYPE_1(CCardsManager, CManagerTask)
public:
	CCardsManager();
	virtual ~CCardsManager();

	virtual const char* NameOf() const { return "CCardsManager";}
	void SelfKill();
	//virtual void InitTask();
	void ManagerPostInitActionsPoint();
	virtual bool IsResetInStartupFail() const {return true;}

	TaskEntryPoint GetMonitorEntryPoint();
	void CreateDispatcher();
	void OnTimerMfaStartupTimeout();
	void OnTimerRmxSystemCardsModeStartupTimeout();
	void OnTimerMfaTaskAlreadyExistTimeout();
	void OnTimerMfaHotSwapTimeout(CSegment* pParam);

	void OnTimerIceInitTimeout();

// producing an Alert for SpanConfiguredOnNonExistingCard is moved to RtmIsdnMngr process
//	void ProduceAlertsIfRtmSpanConfiguredOnNonExistingCard();

	void  OnMplApiMsg(CSegment* pSeg);
	void  OnMplMsgWhileNotREADY(CSegment* pSeg);
	void  OnMcuMngrLicensingInd(CSegment* pSeg);
	void  OnCardMngrLoadedInd(CSegment* pSeg);
    void  OnMediaIpParamsFromCsInd(CSegment* pSeg);
    void  OnMcuMngrIpTypeInd(CSegment* pSeg);
    void  OnIvrMusicAddSourceReq(CSegment* pSeg);
    void  OnCardFolderMountInd(CSegment* pSeg);
    void  OnEmbUserMsgInd(CSegment* pSeg);
    void  OnCardsResetAllBoardsReq(CSegment* pSeg);
    void  OnMediaIpParamsEndFromCsInd();
	void  OnDeleteIpServiceReq(CSegment* pSeg);
	void  OnDeleteIpServiceInd(CSegment* pSeg);

	void  OnRtmIsdnParamsInd(CSegment* pSeg);
	void  OnRtmIsdnSpanMapsInd(CSegment* pSeg);
	void  OnRtmIsdnAttachSpanMapsInd(CSegment* pSeg);
	void  OnRtmIsdnDetachSpanMapsInd(CSegment* pSeg);
	void  OnRtmIsdnDeleteServiceInd(CSegment* pSeg);

	void  OnResourceSystemCardsModeReq(CSegment* pSeg);		// should be united to a single
	void  OnConfPartySystemCardsModeReq(CSegment* pSeg);	//    method: OnSystemCardsModeReq
	void  OnMcuMngrSystemCardsModeReq(CSegment* pSeg);	//    method: OnSystemCardsModeReq
	void  OnUtilitySystemCardsModeReq(CSegment* pSeg);	//    method: OnSystemCardsModeReq
	
    void  OnSNMPInterfaceReq(CSegment* pSeg);
    void  OnNewSysCfgParamsInd(CSegment* pSeg);
    void  OnResourceReconfigReq(CSegment* pSeg);
    void  OnVersionIsReady(CSegment* pSeg);
    void  OnLastCardInstallationFinished(CSegment* pSeg);
    void  OnResetAllCardsReqFromInstaller(CSegment* pSeg);
    void  OnCheckAll_IPMC_IND_received(CSegment* pSeg);
    void  OnIPMC_IND_receivedTOUT(CSegment* pSeg);

    void   OnEthSettingMonitoringFromSysMonitor(CSegment* pSeg);
	void   OnEthSettingActivePortsSwitchInd(CSegment* pSeg);
	void   OnEthSettingMonitoringSwitchInd(CSegment* pSeg);
	void   OnEthSettingClearMaxCountersSwitchInd(CSegment* pSeg);

	void   OnStartTcpDump(CSegment* pSeg);
	void   OnStopTcpDump(CSegment* pSeg);
	void   OnStartTcpDumpInd(CSegment* pSeg);
	void   SendStartTcpDumpStatusToUtilityProcess(CSegment* pSeg);
	void   OnStopTcpDumpInd(CSegment* pSeg);
	void   SendStopTcpDumpStatusToUtilityProcess(CSegment* pSeg);

	const char* GetNGBSystemCardsModeStr(eNGBSystemCardsMode theMode);

	void   OnSipProxyToCardsIceInitReq(CSegment* pSeg);
	void   OnIceInitInd(CSegment* pSeg);
	void   OnIceInitIndReceived(CSegment* pSeg);
	void   OnIceStatIndReceived(CSegment* pSeg); //N.A. DEBUG
	void   OnLoggerCardsMaxTraceLevel(CSegment* msg);
	BOOL   IsMultipleServicesMode();
	
	void   PrintSipProxyIceInitRequest(ICE_SERVER_TYPES_S* pIceServerTypesStruct);
	void   SendIceInitToAllMpmCM(ICE_SERVER_TYPES_S* pIceServerTypesStruct);
	void   SendIceInitToSpecificMpmCM(const DWORD boardId, const DWORD subBoardId, ICE_SERVER_TYPES_S* pIceServerTypesStruct);
	void   SendIceInitIndResultToCsMngr();
	void   SendIceStatIndResultToCsMngr(ICE_STATUS_IND_S* IceStatusInd);
	
	DWORD  GetLanPortOnSwitch(const DWORD portOnCpu);
    void   SendEthernetSettingsActivePortsReqToSwitch();
    void   SendEthernetSettingsMonitoringReqToSwitch(CEthernetSettingsStructWrapper *pCurEth);

	void  SendUpdateSpanMapToRtmIsdnTask(WORD spanBoardId, WORD spanId, OPCODE theOpcode);
    void  SendCurSystemCardsModeToSpecProcess(eProcessType processType);
	void  SendNewSysCfgParamsToAllCards(BOOL bDebugMode,
                                        BOOL bJITCMode,
                                        BOOL bSeparatedNetworks,
                                        BOOL bMultipleServices);

	void  CreateSwitchTask(CCardMngrLoaded* cardMngr, BYTE boardId, BYTE subBoardId);
	void  CreateMfaTask(CCardMngrLoaded* cardMngr, BYTE boardId, BYTE subBoardId, eCardType curCardType);
	void  CreateRtmIsdnTask(CCardMngrLoaded* cardMngr, BYTE boardId, BYTE subBoardId, eCardType curCardType);

	void  SplitMediaIpParamsAndInsertToVector(CS_MEDIA_IP_PARAMS_S *pMediaIpParamsStruct);
	BOOL  IsTaskAlreadyExists(BYTE boardId, BYTE subBoardId);
	BOOL  TreatTaskAlreadyExists_ResetMcms(BYTE boardId, BYTE subBoardId);
	void  TreatTaskAlreadyExists(BYTE boardId, BYTE subBoardId, eCardType curCardType);
	BOOL  TreatTaskAlreadyExists_NoReset(BYTE boardId, BYTE subBoardId, int numOfStartups);
	void  TaskAlreadyExists_AskStartupAgain(BYTE boardId, BYTE subBoardId, COsQueue* pMbx);
	void  TaskAlreadyExists_TooManyStartups(BYTE boardId, BYTE subBoardId, COsQueue* pMbx);

	void  SendDisableAllSpansToRsrcAlloc(WORD boardId, WORD subBoardId);
	void  AddToCardsMonitoringDB(CCardMngrLoaded* cardMngr, BYTE boardId, BYTE subBoardId);
	void RemoveFromCardsMonitoringDB(BYTE boardId, BYTE subBoardId);
	void  SendMediaIpParamsReqToCS();
	void  SendIvrMusicGetSourceReqToConfParty();
	void  SendDeleteIpServiceReqToMplApi(Del_Ip_Service_S *pDelSrv);
	void  SendNewMediaIpToMfaBoard(CMediaIpParameters *pNewMediaIpParams);
	void  SendReconfigUnitToMpm(UNIT_RECONFIG_S* pReconfigStruct);
	void  SendIvrMusicAddSourceReqReceivedToAllBoards();
	void  PrintEmbUsrMsg(USER_MSG_S *msgStruct, int boardId, int subBoardId);
	void  HandleEmbUserMsg(USER_MSG_S *msgStruct, int boardId, int subBoardId);
	void  AddEmbUserMsg(USER_MSG_S *msgStruct, int boardId, int subBoardId);
	void  RemoveEmbUserMsg(USER_MSG_S *msgStruct, int boardId, int subBoardId);
	std::string ConvertUserMsgToString(eUserMsgCode_Emb theCode);

	void  SendCardsNotReadyToMplApi(BYTE boardId, BYTE subBoardId);
	void  SendCardsNotSupportedToMplApi(BYTE boardId, BYTE subBoardId);
	void  OnCardRemovedInd(CSegment* pSeg);
	void  OnBoardsExpectedInd(CSegment* pSeg);
	void  OnModeDetectionInd(CSegment* pSeg);
	void  OnAudioCntrlrActiveReq(CSegment* pSeg);

	void  RecalculateBoardsExpectedVsActual();
	void  RestartStartupTimers(DWORD newStartupTimeout);
	void  OnConfigCardReq(CSegment* pSeg);//2 modes cop/cp

	void OnSNMPConfigInd(CSegment* pSeg);

protected:
	void   IgnoreMessage();

	// terminal commands
	STATUS HandleTerminalCardsState(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalResetReqToDaemon(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalSetDebugModeMpl(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalEnableDisableRtmIsdnKeepAlive(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalSysCardsMode(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalUsrMsgEmb(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTerminalKaRecievedTimeout(CTerminalCommand & command,std::ostream& answer);

	STATUS HandleTerminalEthTest(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTestUpgrade(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTestIpmcUpgrade(CTerminalCommand & command,std::ostream& answer);
	STATUS HandleTestResetAllCards(CTerminalCommand & command,std::ostream& answer);


    void	FillEthernetSettingsActivePortsList(ETH_SETTINGS_ACTIVE_PORTS_LIST_S *pInStruct);
    void	PrintEthernetSettingsActivePortsListStruct(ETH_SETTINGS_ACTIVE_PORTS_LIST_S *pStruct, const string theCaller);
    void	PrintSpecSettingsStruct(ETH_SETTINGS_SPEC_S *pStruct, const string theCaller);
    void	SendClearMaxCountersToSysMonitoring(ETH_SETTINGS_PORT_DESC_S &portDesc);
    void    SendEthSetSysMonitoringReq();
    void    SendUpgradeStartedIndToMplApi();
    void	SendClearMaxCountersReqToMplApi(ETH_SETTINGS_PORT_DESC_S &portDesc);
    void	SendLicenseIndToMfaBoard();

private:
	virtual void OnSysConfigTableChanged(const string &key, const string &data);
	void SendDebugModeChangedToAllCards(const string &data);
	void SendDebugModeChangedToSpecificCard(DWORD boardId, DWORD subBoardId, const string &data);
	void SendUpgradeReqToAllCards();
	void SendUpgradeReqToSpecificCard(DWORD boardId, DWORD subBoardId);
	void SendIpmcUpgradeReqToAllCards();
	void SendIpmcUpgradeReqToAllCardsRequireIpmc();
	void SendIpmcUpgradeReqToSpecificCard(DWORD boardId, DWORD subBoardId);
	void SendResetReqToAllCards();
	void SendResetReqToSpecificCard(DWORD boardId, DWORD subBoardId);

    void SendCardEventToAuditor(bool isInstaled, DWORD boardId, DWORD subBoardId, eCardType cardType)const;
    void InitSystemCardsMode();
    void RecalculateSystemCardsMode();
    void RecalculateSingleMediaBoardOnSecondSlot();
    bool IsSystemCardsModeChanged();
    void UpdateSystemCardsModeFile();
    bool HandleSystemCardsModeAlerts();
    bool HandleNGBSystemCardsModeAlerts(eCardType curCardType,BYTE boardId,BYTE subBoardId);
    std::string	GetEmbBoardIPAddress(DWORD board_id);

	virtual void DeclareStartupConditions();
	void SwapEndian(ETH_SETTINGS_SPEC_S *pSpecSettingsStruct);

	void RescueCard(CRequest* pRequest);

	STATUS SendRescueToCard(DWORD boardId);
	STATUS SendRescueToMFA(DWORD rescueBoardId, DWORD dspUnitId);
	DWORD TranslateBoardIdToRescueBoardId(DWORD boardId);


	// 03.05.07: temp, rush patch for two rtmIsdn boards
	int m_numOfRtmIsdnStartups_boardId_1;
	int m_numOfRtmIsdnStartups_boardId_2;
	int m_numOfRtmIsdnStartups_boardId_3;
	int m_numOfRtmIsdnStartups_boardId_4;

protected:
	CCardsProcess*  m_pProcess;
	BOOL            m_isMediaIpParamsEndReceived;
    bool            m_isStartupOver;
	BOOL            m_isModeDetectionAlreadyTreated;
	BOOL            m_isResourceSystemCardsModeReq;
	int             m_numOfBreezeCards_NGB;
	int             m_numOfMPMRXCards_NGB;
	CRtmIsdnMngrCommonMethods m_rtmIsdnCommonMethods;
	BOOL m_bSNMPEnabled;
	bool            m_isNewVersionBlockedForMPMRX;


// 01/01/2009: SRE requires that removing the last board will NOT affect cards' mode
//	bool m_isCardModeMpmPlusDueToNoCardsOnSystem;

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS
};

#endif  // CARDS_MANAGER_H_
