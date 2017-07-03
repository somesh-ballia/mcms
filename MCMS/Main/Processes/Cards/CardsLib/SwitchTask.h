// SwitchTask.h

#ifndef SWITCH_TASK_H_
#define SWITCH_TASK_H_

#include "AlarmableTask.h"
#include "CardMngrLoaded.h"
#include "CardsInternalStructs.h"
#include "CardsDefines.h"
#include "ObjString.h"
#include "StructTm.h"
#include "RtmIsdnMaintenanceStructs.h"

#define MAX_RTM_LAN_OR_ISDN_CARDS 4

class CCardsProcess;

extern "C" void switchEntryPoint(void* appParam);

class CSwitchTask : public CAlarmableTask
{
public:
	CSwitchTask();
	virtual ~CSwitchTask();
	virtual const char* NameOf() const { return "CSwitchTask";}
	void InitTask();
	void AlarmableTaskCreateEndPoint();
	void SelfKill();

	BOOL        IsSingleton() const {return NO;}
	const char* GetTaskName() const {return m_TaskName.c_str();}

	void              SetCardMngrStructData(const char* data);
    void              SetCardMngrStructData(CCardMngrLoaded* other);
	CCardMngrLoaded   GetCardMngr();
	void              SetBoardId(DWORD id);
	DWORD             GetBoardId();
	void              SetSubBoardId(DWORD id);
	DWORD             GetSubBoardId();

	DWORD AddActiveAlarmSwitch( CSmallString callerStr,
				    BOOL isToChangeState,
				    BYTE subject,
				    DWORD errorCode,
				    BYTE errorLevel,
				    string description,
				    bool isForEma,
				    bool isForFaults,
				    DWORD userId,
				    DWORD boardId,
				    WORD theType );

	DWORD AddActiveAlarmFaultOnlySwitch( CSmallString callerStr,
				    BYTE subject,
				    DWORD errorCode,
				    BYTE errorLevel,
				    string description,
				    DWORD userId,
				    DWORD boardId,
				    WORD theType );


	void RemoveActiveAlarmSwitchByErrorCode(CSmallString callerStr,
						BOOL isToChangeState,
						BYTE subject,
						DWORD errorCode,
						BOOL isFaultOnly = FALSE);

	void RemoveActiveAlarmSwitchByErrorCodeUserId(CSmallString callerStr,
                                                  BOOL isToChangeState,
                                                  WORD subject,
                                                  WORD errorCode,
                                                  DWORD userId,
                                                  BOOL isFaultOnly = FALSE);

	BOOL IsActiveAlarmOrFaultOnlyExistByErrorCodeUserId(BYTE errorLevel, DWORD userId, BOOL isFaultOnly);


	void UpdateCardStateAccordingToTaskState(CSmallString callerStr);

	void SwapEndian(ETH_SETTINGS_SPEC_S *pSpecSettingsStruct);
	
	void OnLdapLoginRequest(CSegment* msg);

	void SendRtmLanIsdnParamsToProcesses(SLOTS_CONFIGURATION_S mfaBoardCnfArray[]);
	void SendRtmLanIsdnParamsToMcuMngr(SLOTS_CONFIGURATION_S mfaBoardCnfArray[]);
	void SendRtmLanIsdnParamsToUtility(SLOTS_CONFIGURATION_S mfaBoardCnfArray[]);

protected:
	void   IgnoreMessage();

	void   OnCmReconnectInd(CSegment* pSeg);
	void   OnCmUpgradeNewVersionReadyAckInd(CSegment* pSeg);
	void   OnCmUpgradeProcessInd(CSegment* pSeg);
	void   OnCmUpgradeIpmcInd(CSegment* pSeg);
	void   OnCmUpgradeIpmcProcessInd(CSegment* pSeg);
	void   OnCmUpgradeIpmcTimer(CSegment* pSeg);

	void   OnShmFatalFailureInd(CSegment* pSeg);
	void   OnShmLanFailureInd(CSegment* pSeg);
	void   OnShmKeepAliveInd(CSegment* pSeg);
	void   OnTimerKeepAliveReceiveTimeout();
	void   OnTimerKeepAliveSendTimeout();
	void   TreatSmKeepAliveFailure();
	void   OnSpecMfaActiveAlarmReq(CSegment* pSeg);
	void   OnNewSysCfgParamsInd(CSegment* pSeg);
	void   OnOldSysCfgParamsInd(CSegment* pSeg);
	void   OnBadSpontaneousInd(CSegment* msg);

	void   HandleRtmIsdnOrLanSlots(SWITCH_SM_KEEP_ALIVE_S keepAliveStruct,BOOL sendMsg=TRUE);
	void   SendRtmIsdnOrLanToMcuMngr(WORD RtmLanIndex, WORD RtmIsdnIndex, DWORD RtmLan[], DWORD RtmIsdn[]);
	//void   IsRtmShmComponent(SM_COMPONENT_STATUS_S statusStruct, DWORD RtmLan[], DWORD RtmIsdn[], WORD &RtmLanIndex, WORD &RtmIsdnIndex, BOOL &bExist);

	void   SendMessageToSwitch(const DWORD opCode, const size_t size, const char* pStruct);
	void   OnEthSettingInd(CSegment* pSeg);
	void   OnEthSettingClearMaxCountersInd(CSegment* pSeg);
	void   OnSlotsNumberingConversionTableInd(CSegment* pSeg);

	void   SetSwitchLanPortsMounted();
	void   SendSlotsNumberingConversionReqToMplApi();
	void   SendVLanIdReqToMplApi();
	void   SendSysCfgParamsToShelfMngr();

	void   PrintRtmIsdnVLanIdStruct(const RTM_ISDN_VLAN_ID_S &theStruct);

	void   HandleCpuBoardId(SWITCH_SM_KEEP_ALIVE_S &pStrcut);
	void   GetCpuBoardIdSubBoardId(SWITCH_SM_KEEP_ALIVE_S &pStrcut, int &cpuBoardId, int &cpuSubBoardId);
	void   GetSpecBoardIdSubBoardId(SWITCH_SM_KEEP_ALIVE_S *pStrcut, string sCompName, int &boardId, int &subBoardId);
	void   SendSpecSlotIdToSpecProcess(int boardId, int subBoardId, eShmComponentType compType, eProcessType processType);

	void   SendSmKeepAliveRequestToMplApi();
	void   RemoveActiveAlarmsByCompTypeUserId(eShmComponentType compType, DWORD slotId, BYTE subject, BOOL isTransferredToMfa = NO);
	void   PrintSmSpecificCompFailure(SM_COMPONENT_STATUS_S *compFailureStruct, char* resStr);
	void   PrintSmFatalFailureToTrace(SWITCH_SM_KEEP_ALIVE_S &fatalFailureStructPtr);
	void   PrintSmLanFailureToTrace(SM_COMPONENT_STATUS_S lanFailureStruct);

	void   SendSlotsNumberingConversionTableToProcesses();
	void   SendSlotsNumberingConversionTableToSpecProcess(const eProcessType processType);

	void   TreatAllComponentsOnSmFatalFailure(SWITCH_SM_KEEP_ALIVE_S &fatalFailureStruct,SLOTS_CONFIGURATION_S mfaBoardCnfArray[]);
	void   MountRtmShmComponent(SLOTS_CONFIGURATION_S mfaBoardCnfArray[]);

	void   TreatSpecificComponent(SM_COMPONENT_STATUS_S * statusStruct);
	void   TreatRtmOrIsdnFailureInSmUnit_mfa(int slotid,int subboardid,bool IsRTMorISDNmissing);
	void   TreatFailureInSmUnit_mfa(eShmComponentType compType, SM_COMPONENT_STATUS_S * compStatus);
	void   TreatFailureInSmUnit_nonMfa(eShmComponentType compType, SM_COMPONENT_STATUS_S *compStatus);

	void   ReportMpmExistsIfNeeded(SWITCH_SM_KEEP_ALIVE_S keepAliveStruct);
	void   RetrieveBasicInfoFromKA(SWITCH_SM_KEEP_ALIVE_S & fatalFailureStruct,bool isKeepAliveMsg=false);
	void   UpdateNumOfBoardsExpectedInProcess(SM_COMPONENT_STATUS_S * statusStruct, eCardType& curCardType);

	BOOL   AddMfaActiveAlarms(APIU32 problemBitmask, DWORD mfaBoardId, BOOL isResetting);
	BOOL   UpdateActiveAlarmsNonMfaIfNeeded(eShmComponentType compType, APIU32 problemBitmask, DWORD slotId, BOOL isResetting, char *compName);
	BOOL   UpdateSpecificActiveAlarmNonMfa( APIU32 specificBitMask, APIU32 problemBitMAsk, WORD errCode, BOOL isToChangeState,
	                                        DWORD slotId, eShmComponentType compType, string description, BOOL isFaultOnly = FALSE);
	BOOL   AddResettingActiveAlarmNonMfa(BOOL isToChangeState, DWORD slotId, eShmComponentType compType);
	void   FillActiveAlarmsSpecificCardStruct(ACTIVE_ALARMS_SPECIFIC_CARD_S* pAaStruct, DWORD mfaBoardId);

	void CreateTaskName();

	WORD				ConvertShelfMngrComponentTypeToFaultType(eShmComponentType compType);
	char*				ConvertShelfMngrComponentTypeToString(APIU32 compType);
	char*				ConvertShelfMngrComponentProblemTypeToString(APIU32 problemType);
	eShmComponentType	ConvertShelfMngrStringToCompType(char *compType);
	eCardType			ConvertShelfMngrStringToCardType(char *compType);
	eShmComponentType   ConvertShelfMngrStringToSpecificCompType(char *compType);

	BOOL				IsMpmOrMpmPlusSmComponent(eShmComponentType compType);
	bool				IsMpmSmComponent(SM_COMPONENT_STATUS_S statusStruct);

	virtual void AddFilterOpcodePoint();

	std::string     m_TaskName;
	CCardsProcess*  m_pProcess;
	DWORD           m_boardId;
	DWORD           m_subBoardId;
	CCardMngrLoaded m_cardMngr;
	BOOL			m_isFirstKeepAliveAlreayTreated;
	BOOL			m_isFirstCNRLalreadyTreated;
	BOOL            m_isModeDetectionAlreadyTreated;
	DWORD           m_keepAliveReceivePeriod;
	DWORD           m_keepAliveSendPeriod;
	BYTE            m_numOfkeepAliveTimeoutReached;
	BOOL            m_isKeepAliveFailureAlreadyTreated;
	BOOL            m_isMpmAlreadyReported;
	CStructTm		m_lastKeepAliveReqTime;
	CStructTm		m_lastKeepAliveIndTime;
	BOOL			m_isJitcMode;
	BOOL			m_resetOnOldJitcInd;
	BOOL			m_isSeparatedNetworks;
	BOOL			m_isMultipleServices;
	BOOL            m_isInstalling;
	BOOL            m_require_ipmc_upgrade;
	BOOL            m_isInstallingIpmc;

	PDECLAR_MESSAGE_MAP
};

#endif  // SWITCH_TASK_H_
