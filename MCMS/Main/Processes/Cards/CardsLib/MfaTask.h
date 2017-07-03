// MfaTask.h

#ifndef MFA_TASK_H_
#define MFA_TASK_H_

#include "AlarmableTask.h"
#include "CardsStructs.h"
#include "CardMngrLoaded.h"
#include "MediaIpConfig.h"
#include "MediaIpConfigVector.h"
#include "CardsInternalStructs.h"
#include "CardsDefines.h"
#include "ObjString.h"
#include "StructTm.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include <map>

class CCardsProcess;
class CMediaIpParameters;

extern "C" void mfaEntryPoint(void* appParam);

class CMfaTask : public CAlarmableTask
{
public:
	CMfaTask();
	virtual ~CMfaTask();
	const char* GetTaskName() const {return m_TaskName.c_str();}
	virtual const char* NameOf() const { return "CMfaTask";}

	void InitTask();
	void AlarmableTaskCreateEndPoint();
	void SelfKill();

	BOOL        IsSingleton() const {return NO;}

	void              SetCardMngrStructData(const char* data);
    void              SetCardMngrStructData(CCardMngrLoaded* other);
	CCardMngrLoaded   GetCardMngr();


	void              SetBoardId(DWORD id);
	DWORD             GetBoardId();

	void              SetSubBoardId(DWORD id);
	DWORD             GetSubBoardId();

	void              SetIsEnabled(bool isEnabled);
	bool              GetIsEnabled();

	void               SetUnitsTypesConfigured(CSegment* pSeg);
	void               SetUnitsTypesConfigured(int idx, APIU32 unitType);
	CM_UNITS_CONFIG_S* GetUnitsTypesConfigured();

	void              SetUnitsLoadedStatuses(char* data);
	CM_UNIT_LOADED_S  GetUnitsLoadedStatuses();

	CMediaIpConfigVector*  GetMediaIpConfigVector()const;

	DWORD AddActiveAlarmMfa( CSmallString callerStr, BYTE subject, DWORD errorCode, BYTE errorLevel,
                             string description, bool isForEma, bool isForFaults,
                             DWORD userId, DWORD boardId, DWORD unitId, WORD theType ,
                             BOOL doesFlush = TRUE);
	DWORD AddActiveAlarmFaultOnlyMfa( CSmallString callerStr, BYTE subject, DWORD errorCode, BYTE errorLevel,
										string description,	DWORD userId, DWORD boardId, DWORD unitId, WORD theType);


	void RemoveActiveAlarmMfaByErrorCode(CSmallString callerStr, WORD errorCode, BOOL isFaultOnly = FALSE);
	void RemoveActiveAlarmMfaByErrorCodeUserId(CSmallString callerStr, WORD errorCode, DWORD userId, BOOL isFaultOnly = FALSE);

	void UpdateCardStateAccordingToTaskState(CSmallString callerStr);
        void OnAudioCntrlrActiveReq(CSegment* pSeg);
        void OnCmUnitReconfigInd(CSegment* pSeg);

    void SwapEndian(ETH_SETTINGS_SPEC_S *pSpecSettingsStruct);

    void OnOldSysCfgParamsInd(CSegment* pSeg);
    void OnNewSysCfgParamsInd(CSegment* pSeg);
    void OnCardConfigReq(CSegment* pSeg);//2 modes cop/cp

protected:
	void   IgnoreMessage();

	void   OnMfaKeepAliveInd(CSegment* pSeg);
	void   CheckKeepAliveResults(KEEP_ALIVE_S *tmpKA, eMfaKeepAlivePhase kaPhase);
	void   OnBadSpontIndFromMFA(CSegment* msg);
	void   OnSpecMfaActiveAlarmInd(CSegment* pSeg);
	virtual void   OnTimerKeepAliveReceiveTimeout();
	virtual void   OnTimerKeepAliveSendTimeout();
	virtual void   OnTimerReceiveAllFips140IndsTimeout(CSegment* pSeg);
    virtual void  	OnTimerReceiveAllIPConfigMsgsTimeout(CSegment* pSeg);
	virtual void   TreatMfaKeepAliveFailure();

	void   TreatMpmExistsOnSystem(eCardType cardType);
	void   AskSwitchForActiveAlarms();
	BOOL   IsKeepAliveChangedFromPrevious(KEEP_ALIVE_S *tmpKA);
	void   PrintKeepAliveResults(KEEP_ALIVE_S *KaStruct, eMfaKeepAlivePhase kaPhase);
	void   SendKeepAliveToRsrcAlloc(KEEP_ALIVE_S *KaStruct);
	BOOL   IsNewFailure(int id, eCardUnitLoadedStatus unitStatus);
	// DWORD  ProduceFaultAndLogger(eFailureType failureType, int id=0, BOOL doesFlush=TRUE);
	void  ProduceFaultAndLogger(eFailureType failureType, int id=0, BOOL doesFlush=TRUE);
	void   SendKeepAliveFailureToRsrcAlloc( eCardState cardState=eNoConnection,
	                                        OPCODE theOpcode=CM_KEEP_ALIVE_IND);

	void   OnRsrcAllocUnitConfigInd(CSegment* pSeg);
	void   OnUnitLoadedInd(CSegment* pSeg);
	void   OnMediaIpConfigInd(CSegment* pSeg);
	void   OnCmReconnectInd(CSegment* pSeg);


	void   OnCmUpgradeNewVersionReadyAckInd(CSegment* pSeg);
	void   OnCmUpgradeProcessInd(CSegment* pSeg);
	void   OnCmUpgradeIpmcInd(CSegment* pSeg);

	void   OnCmUpgradeIpmcProcessInd(CSegment* pSeg);
	void   OnCmUpgradeIpmcTimer(CSegment* pSeg);
	void   OnShmMfaFailureInd(CSegment* pSeg);
	void   OnTimerStartupTimeout();
	void   OnCardsNewMediaIpInd(CSegment* pSeg);
	void   SendDnsMediaConfigReqToMplApi();
	void   OnIvrMusicAddSourceReqReceivedInd(CSegment* pSeg);
	void   OnEthSettingInd(CSegment* pSeg);
	void   OnEthSettingClearMaxCountersInd(CSegment* pSeg);

	void   OnIceInitInd(CSegment* pSeg);
	void   OnIceStatusInd(CSegment* pSeg);
	void   OnCSExternalConfigInd(CSegment* pSeg);
	void   OnCSInternalConfigInd(CSegment* pSeg);
    void   OnCSDnsConfigInd(CSegment* pSeg);
    void   OnMediaConfigurationCompletedInd(CSegment* pSeg);

    void   OnTimerCSExtIntIPConfigFirstTimeout(CSegment* pParam );
    void   OnTimerCSExtIntIPConfigSecondTimeout(CSegment* pParam );
    void   OnTimerCSExtIntWaitTimeout(CSegment* pParam );

	void   OnArtFips140Ind(CSegment* pSeg);
	void   OnMfaChecksumInd(CSegment* pSeg);
	void   OnLicenseInd(CSegment* pSeg);

	void   RemoveSmActiveAlarms(CSmallString callerStr, DWORD boardId);
	void   RemoveAllActiveAlarms(CSmallString callerStr);
	void   RemoveAllUserMsgEmbActiveAlarms(CSmallString callerStr);
	BOOL   UpdateActiveAlarmsIfNeeded(DWORD problemBitmask, BOOL isResetting);
	BOOL   UpdateSpecificActiveAlarm(APIU32 specificBitMask, APIU32 problemBitMAsk, WORD errCode, string description, BOOL isFaultOnly = FALSE);
	BOOL   AddResettingActiveAlarm();
	void   ProduceActiveAlarmsTransferredFromSwitch(ACTIVE_ALARMS_SPECIFIC_CARD_S* pAaStruct);
	void   TreatNoLanActiveAlarm(ETH_SETTINGS_SPEC_S* pEthSetting);

	void   PrintUnitLoadedDataToTrace();
	void   PrintUnitTypesConfiguredDataFromRsrcAlloc();
	void   PrintUnitTypesConfiguredDataToMfa(CM_UNITS_CONFIG_S* pConfigParamsList);
	void   PrintMfaStartupCompleteIndToRsrcAlloc(DWORD pq1status, DWORD pq2status);
	void   PrintMediaIpConfigDataToTrace(const MEDIA_IP_CONFIG_S *pNewMediaIpConfigStruct, const string theCaller);
	void   PrintMfaStartupCompleteIndToCsMngr();

	BOOL   AreAllFips140IndsReceived();
	void   SendFips140ActiveAlarmIfNeeded();
	void   SendAnActiveAlarmAboutAllFailedARTs();
	void   SendAnActiveAlarmAboutChecksumFailure();

	void   HandleStartupOverIfNeeded(CSmallString callerStr);

	void   HandleStartupOver(CSmallString callerStr);
	void   ProduceActiveAlarmIvrMountIfNeeded();
	void   SetIsMediaIpConfigIndReceived( DWORD ipAddressToCheck, DWORD rtmMediaIp1,
										  DWORD rtmMediaIp2, DWORD rtmMediaIp3,
										  DWORD rtmMediaIp4, DWORD status);
	void   CalculateTotalPQsState();

	void   SendMessageToMfa(DWORD boardId, DWORD subBoardId, DWORD opcode, size_t size, char* pStruct);
	void   SendResetMfaReqToMplApi();
	void   SendDebugModeToMfa();
	void   SendSysCfgParamsToMfa();
	void   SendSystemCardsModeToMfa();
	void   SendMpmKeepAliveRequestToMplApi(OPCODE theOpcode);
	void   SendUnitConfigReqToRsrcalloc();
	void   SendUnitConfigReqToMplApi();
	void   SendMediaIpConfigReqToMplApi();
	void   SendRtmMediaIpConfigReqToMplApi();
	void   SendMediaIpConfigEndReqToMplApi();
	void   SendIvrMusicAddSourceReqToMplApi();
	void   SendMediaRecordingReqToMplApi();
	void   SendEthSettingIndToCS(DWORD portStatus);
	void   SendMediaIpConfigIndToCS(CMediaIpConfig* pMediaIpConfig);
	void   SendMfaStartupCompleteIndToRsrcAlloc(DWORD pq1status, DWORD pq2status);
	void   SendMfaStartupCompleteToCsMngr();
	void   SendFIPS140RequestsToMplApi();
	void   SendCSInternalIpConfigReqToMplApi(CMediaIpParameters* mediaIpParamsFromVector);
	void   SendCSExternalIpConfigReqToMplApi(CMediaIpParameters* mediaIpParamsFromVector);
	void   HandleDNSConfiguration(CMediaIpParameters* mediaIpParamsFromVector);
	void   HandleV35GWConfiguration(CMediaIpParameters* mediaIpParamsFromVector);
	void   SendNATRuleToMfa(DWORD vlanId, DWORD port, DWORD internalAddress, DWORD externalAddress);
	STATUS ValidateIpAddressWithMask(CMediaIpParameters* mediaIpParams);
    void   SendHotSwapCardRemovedToRtmProc();
    void   SendHotSwapCardRemovedToRtmTask();
    void   SendHotSwapCardRemovedToCardMngr();
    void   SendReconfigStructToRsrcAlloc(UNIT_RECONFIG_S* pReconfigStruct);

    void   ProduceIPv6AddressAssertIfNeeded(const MEDIA_IP_CONFIG_S *pNewMediaIpConfigStruct);
	void   CheckSignalPortStatusForFailover(ETH_SETTINGS_SPEC_S* pEthSettings);	
	void   OnTimerFirstSignalPortInactTimeout();
	void   OnTimerSecondSignalPortInactTimeout();

	virtual void CreateTaskName();

	virtual void AddFilterOpcodePoint();
	virtual void   SendLanEthSettingToMcuMngr();

	//For multiple services
	void	SetVlanData(DWORD PQ_Id, MEDIA_IP_PARAMS_S* pMediaIpStruct, BOOL bNoVlan);
	void 	SetVlanDataForMultiServices(DWORD PQ_Id, MEDIA_IP_PARAMS_S* pMediaIpStruct);
	void 	SendCSIpConfigRequestsToMplApi();
	void    SetCSInternalIPv4(DWORD PQ_Id, MEDIA_IP_PARAMS_S* pMediaIpStruct);
	void 	SetCSExternalIPv4(CMediaIpParameters* mediaIpParamsFromVector, MEDIA_IP_PARAMS_S* pMediaIpStruct);
	void 	SetMediaCardInternalIPv4MultiServices(DWORD PQ_Id, MEDIA_IP_PARAMS_S* pMediaIpStruct);
	BOOL	IsMultipleServicesMode();
	BOOL	IsV35JITC();
	bool isMsIceConfigured();
	std::string	GetEmbBoardIPAddress(DWORD board_id);
	
	
	//set bAddAA, And return if any active alarm still exists and some port
	BOOL SetPortAAAndReturnIfAnyAAExist(BYTE portNumber, BOOL bAddAA);

	std::string m_TaskName;

	CCardsProcess*      m_pProcess;

	DWORD               m_boardId;
	DWORD               m_subBoardId;
	bool                m_isEnabled;	// an MPM card is disabled in MPM+ (PURE) mode (and is not reported to Resource process)
	CCardMngrLoaded     m_cardMngr;
	CM_UNITS_CONFIG_S*  m_pUnitsTypesConfiguredStruct;
	CM_UNIT_LOADED_S    m_unitsLoadedStatusesStruct;
	MEDIA_IP_CONFIG_S*  m_mediaIpConfigStruct;
	CMediaIpConfigVector*  m_pIpMediaConfigVector;
	
	std::map<BYTE, BOOL>	m_portAAs;  // port number -> isAAExist on port
	
	BOOL  m_isStartupOverActionsAlreadyPerformed;
	BOOL  m_isStartupCompleteIndAlreadySentToRsrcAlloc;
	BOOL  m_isFirstKaAfterCompleteAlreaySentToRsrcAlloc;
	BOOL  m_isUnitLoadedIndReceived;
	BOOL  m_isMediaIpConfigIndReceived;
	BOOL  m_isRtmMediaIpConfigIndReceived;
	BOOL  m_isMediaIpExistsForThisMfa;
	BOOL  m_isIvrMusicSourceReqAlreaySentToMpm;
	eMediaIpConfigStatus m_mediaIpConfigStatus;

	KEEP_ALIVE_S            m_keepAliveStructure;
	DWORD                   m_keepAliveReceivePeriod;
	DWORD                   m_keepAliveSendPeriod;
	BOOL                    m_isKeepAliveAlreadyReceived;
	BYTE                    m_numOfkeepAliveTimeoutReached;
	BOOL                    m_isKeepAliveFailureAlreadyTreated;
	BOOL					m_isAssertKeepAliveAfterNoConnectionAlreadyProduced;
	CStructTm				m_lastKeepAliveReqTime;
	CStructTm				m_lastKeepAliveIndTime;
	WORD                    m_unitsFips140StatusList[MAX_NUM_OF_UNITS];
	BOOL					m_isJitcMode;
	BOOL					m_resetOnOldJitcInd;
	BOOL                	m_isInstalling;
	BOOL                	m_require_ipmc_upgrade;
	BOOL                	m_isInstallingIpmc;
	BOOL					m_isLanActive;	//save the last result we got from ETHERNET_SETTINGS_IND regarding lan activity
	int                     m_ipConfigMsgACKCounter;
    BOOL                    m_isMediaConfigurationCompletedIndReceived;

	/*EXT-4321: to check the signaling port for Failover*/
	BOOL					m_isFirstPortUp;
	BOOL					m_isSecondPortUp;

	PDECLAR_MESSAGE_MAP

};

#endif  // MFA_TASK_H_
