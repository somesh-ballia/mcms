#if !defined(_ResourceMANAGER_H__)
#define _ResourceMANAGER_H__

#include "ConnToCardManager.h"
#include "RsrcAlloc.h"
#include "MplMcmsProtocol.h"
#include "ManagerTask.h"
#include "RvCommonDefs.h"
#include "CMediaRecordingGet.h"
#include "DummyEntry.h"
#include "OpcodesMcmsCardMngrRecording.h"
#include "RecordingRequestStructs.h"
#include "InnerStructs.h"
#include "SysConfig.h"
#include "StartupInfo.h"

const char* GetSystemCardsModeStr(eSystemCardsMode theMode);

class CPartyDebugInfo;

////////////////////////////////////////////////////////////////////////////
//                        CResourceManager
////////////////////////////////////////////////////////////////////////////
class CResourceManager : public CManagerTask
{
	CLASS_TYPE_1(CResourceManager, CManagerTask)

public:
	               CResourceManager();
	              ~CResourceManager();

	const char*    NameOf() const               { return "CResourceManager"; } //virtual
	bool           IsResetInStartupFail() const { return true; } //virtual
	TaskEntryPoint GetMonitorEntryPoint();
	void           SelfKill(); //virtual
	void           ManagerPostInitActionsPoint();
	int            GetTaskMbxBufferSize() const { return 1024 * 1024 - 1; } //virtual
	void*          GetMessageMap();
	void           OnAssertTreatment() { } //virtual

private:
	// StateMashine commands
	void OnResourceConfigRequest(CSegment* pMsg);
	void OnResourceKeepAliveInd(CSegment* pMsg);
	void OnResourceFipsInd(CSegment* pMsg);
	void OnResourceMfaStartupCompleteInd(CSegment* pMsg);
	void OnResourceAllocRequest(CSegment* pMsg);
	void OnResourceReAllocArtRequest(CSegment* pMsg);
	void OnResourceReAllocRequest(CSegment* pMsg);
	void OnResourceDeAllocRequest(CSegment* pMsg);
	void OnResourceForceDeAllocAllPartiesInConfRequest(CSegment* pMsg);
	void OnResourceGetConfAndPartiesRsrcIdsRequest(CSegment* pMsg);
	void OnResourceGetPartyRsrcIdRequest(CSegment* pMsg);
	void OnResourceGetConfCopRsrcIdsRequest(CSegment* pMsg);
	void OnResourceStartConfRequest(CSegment* pMsg);
	void OnResourceTerminateConfRequest(CSegment* pMsg);
	void OnSetConferenceEndTimeRequest(CSegment* pMsg);
	void OnFailoverSlaveUpdateConfTimeReq(CSegment* pMsg);
	void OnResourceGetAvcSvcAdditionalPartyRsrcReq(CSegment* pMsg);
	void OnDongleRestrictInd(CSegment* pMsg);
	void OnTimeChangedInd(CSegment* pMsg);
	void OnMplApiInd(CSegment* pMsg);
	void OnSpreadingInd(CSegment* pMsg);
	void OnStartPartyMoveReq(CSegment* pMsg);
	void OnEndPartyMoveReq(CSegment* pMsg);
	void OnResourceAllocPcmRequest(CSegment* pMsg);
	void OnResourceAllocContentXCodeRequest(CSegment* pMsg);
	void OnResourceDeAllocContentXCodeRequest(CSegment* pMsg);
	void OnResourceDeAllocPcmRequest(CSegment* pMsg);
	void OnLastConfIdInd(CSegment* pMsg);
	void OnIPServicePQConfigRequest(CSegment* pMsg);
	void OnIPv6ServiceUpdateRequest(CSegment* pMsg);
	void OnIPServiceDeleteRequest(CSegment* pMsg);
	void OnIPServiceConfigEnd(CSegment* pMsg);
	void OnIPServiceDefaultUpdate(CSegment* pMsg);
	void OnRTMBoardLoadedRequest(CSegment* pMsg);
	void OnRTMServiceConfigRequest(CSegment* pMsg);
	void OnRTMServiceConfigEndRequest(CSegment* pMsg);
	void OnRTMCreateSpanRequest(CSegment* pMsg);
	void OnRTMRemoveSpanRequest(CSegment* pMsg);
	void OnRTMDisableAllSpansRequest(CSegment* pMsg);
	void OnRTMAddPhoneRangeRequest(CSegment* pMsg);
	void OnRTMDelPhoneRangeRequest(CSegment* pMsg);
	void OnRTMDelServiceRequest(CSegment* pMsg);
	void OnDeallocateBondingTemporaryNumber(CSegment* pMsg);
	void OnRTMUpdatePortRequest(CSegment* pMsg);
	void OnRTMReallocateBoardFull(CSegment* pMsg);
	void OnUpdateResolutionThreshold(CSegment* pMsg);
	void OnSystemCardsModeInd(CSegment* pMsg);
	void OnSystemRamSizeInd(CSegment* pMsg);
	void OnSystemCPUProfileInd(CSegment* pMsg);
	void OnSlotNumberingConversionTableInd(CSegment* pMsg);
	void OnReconfigureUnitsTimer(CSegment* pMsg);
	void OnNumericIdRequest(CSegment* pMsg);
	void OnFailoverStartSlaveInd(CSegment* pMsg);
	void OnFailoverStartMasterInd(CSegment* pMsg);
	void OnFailoverSlaveBcmMasterInd(CSegment* pMsg);
	void OnFailoverReStartSlaveInd(CSegment* pMsg);
	void OnFailoverStartMasterBecomeSlaveInd(CSegment* pMsg);
	void OnResourceSetProcessState(CSegment* pMsg);
	void OnResourceUnitRecoveryInd(CSegment* pMsg);
	void OnResourceUnitRecoveryEndInd(CSegment* pMsg);
	void OnMplUnitFatalInd(CSegment* pMsg);
	void OnMplUnitUnFatalInd(CSegment* pMsg);
	void OnMplPartyDebugInfoInd(CSegment* pMsg);
	void OnMplPartyCmDebugInfoInd(CSegment* pMsg);
	void OnMplConfDebugInfoInd(CSegment* pMsg);
	void OnMultipleServicesInd(CSegment* pMsg);
	void OnSysMntrStopAllMediaRecordingReq(CSegment* pMsg);
	void OnStopVideoPreview(CSegment* pMsg);
	void OnStartupMRAndProfileRead(CSegment* pMsg);
	void OnProfileUpdate(CSegment* pMsg);
	void OnProfileAdd(CSegment* pMsh);
	void OnProfileDelete(CSegment* pMsg);
	void OnCardRemovedRequest(CSegment* pMsg);
	void OnUnitReconfiguredInd(CSegment* pMsg);
	void OnHandleChangeSysMode(CSegment* pMsg);
	void OnSNMPConfigInd(CSegment* pMsg);
	void OnHighUsageCPUInd(CSegment* pMsg);
	void OnSendAllPartiesOnUnit(CSegment* pMsg);
	void OnUpdateInterfaceServices(CSegment* pSeg);
	void OnRetrieveOccupiedUdpPortsTimer(CSegment* pSeg);

	// ResourceManagerTerminal.cpp
	STATUS HandleTerminalCheckConsistency(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalDisableUnit(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalUnitFatal(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalDisableBoard(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalRemoveBoard(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowMasterAC(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowSlaveAC(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowResevedAC(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalMoveMasterAC(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalRestoreMasterAC(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowIvrCntrlr(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowSharedMemory(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowPQ(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalNidCount(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalIsNidOccupied(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowCapacity(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalAllocTBN(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowcTBN(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalIsChannelIdAllocateded(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalReconfigureUnit(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalPrintRes(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalPrintProfiles(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalSetMaxArtChannelsPerArt(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowNumArtChannels(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalStartRecording(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalStopAllMediaRecording(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalShowSlotTable(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalSendNewHWInd(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalResetPortConfiguration(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalPrintBoardStreamStatus(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleDumpBrdsState(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalRecoveryUnit(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalExtractPartyInfo(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalExtractPartyPorts(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalSetFreeDisable(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleShiftFutureConferences(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalSetCPUSize(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalGetCPUSize(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleSetConfAvcSvcMode(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleSetPortGauge(CTerminalCommand& command, std::ostream& answer);
	void   DumpTerminalCommand(char* functionname, CTerminalCommand& command);

	// ResourceManagerTransactions.cpp
	STATUS OnStartMediaRecording(CRequest* pSetRequest);
	STATUS OnStopAllMediaRecording(CRequest* pSetRequest);
	STATUS OnGetPartyPortsInfo(CRequest* pRequest);
	STATUS OnResetPortConfiguration(CRequest* pSetRequest);
	STATUS OnSetPortConfiguration(CRequest* pSetRequest);
	STATUS OnSetEnhancedPortConfiguration(CRequest* pSetRequest);
	STATUS OnSetPortGauge(CRequest* pSetRequest);
	STATUS OnSetAllocationMode(CRequest* pSetRequest);
	STATUS OnDeleteResRequest(CRequest* pRequest);
	STATUS OnDeleteRepeatedResRequest(CRequest* pRequest);
	STATUS OnDelProfileRequest(CRequest* pRequest);
	STATUS OnStartVideoPreviewRequest(CRequest* pMsg);
	STATUS OnStopVideoPreviewRequest(CRequest* pMsg);
	STATUS OnShiftFutureConferences(CRequest* pMsg);
	STATUS OnResetUnitsRequest(CRequest* pRequest);
	STATUS OnEnableUnitsRequest(CRequest* pRequest);
	STATUS OnDisableUnitsRequest(CRequest* pRequest);
	STATUS UnitOperation(CRequest* pRequest, OPCODE opCode);

public:
	CConnToCardManager* GetConnToCardManager() const { return m_pConnToCardManager; };

	STATUS SystemResourcesEmptyRequest();
	void   DeAlloc(DEALLOC_PARTY_REQ_PARAMS_S* pParams, DEALLOC_PARTY_IND_PARAMS_S* pResult);
	void   CheckResourceEnoughAndAddOrRemoveAciveAlarm(BOOL bAddActiveAlarmIfNecessary = TRUE, BOOL bForcebRemoveActiveAlarm = FALSE, BOOL bRemoveCard = FALSE );
	STATUS KillPortRequest(PhysicalPortDesc* pPortDesc, eResourceTypes resourceType);
	STATUS KillUdpPortRequest(PhysicalPortDesc* pPortDesc, CUdpRsrcDesc* pUdpDesc);
	STATUS KillIsdnUdpPortRequest(PhysicalPortDesc* pPortDesc, WORD channelId, eResourceTypes resourceType);
	STATUS PortNotRespondingRequest(PhysicalPortDesc* pPortDesc, eResourceTypes resourceType);
	STATUS CreateController(BoardID boardId, UnitID unitId, ConnectionID connId, eResourceTypes resourceType, ECntrlType controllerType = E_NORMAL);
	void   RemoveActiveAlarm(WORD alarm);
	void   SendMeetingRoomsAndProfileDBreadReqToConfMngr();
	void   SendModeReqToCardMngr();
	STATUS ConverFailureStatus(STATUS status, OPCODE opCode);
	STATUS SendStartRecordingToCm(ConfMonitorID monitorConfId, PartyMonitorID monitorPartyId, DWORD sizeLimit, CJunctionParam& junc);
	STATUS SendStopRecordingToCm();
	STATUS ResetUnitById(int board_id, int unit_id, int subBoardId);
	STATUS EnableUnitById(int boardId, int unitId, bool enable);
	void   SendSystemRecordingToMcuMngr(DWORD isRecording);
	void   ReceiveAdditionalParams(CSegment* pSeg); //virtual
	void   SendAdditionalParams(CSegment* pSeg);    //virtual
	void   SendToMFA_CardConfigReq(CTaskApi* api);  //2 modes cop/cp
	void   OnTimerRetrieveDebugInfo(CSegment* pSeg, bool isCM);
	void   OnTimerRetreiveDebugInfo(CSegment* pSeg, bool isCM);
	void   AddPortsToPartyInfo(std::ostream& answer, CPartyDebugInfo* pPartyDebugInfo);
	STATUS SendResetUnitToMFA(DWORD boardId, DWORD dspUnitId);
	bool   isRPPLicense() { return m_resourceStartupInfo.isRPPLicense(); }

	PDECLAR_MESSAGE_MAP
	PDECLAR_TERMINAL_COMMANDS
	PDECLAR_TRANSACTION_FACTORY

	CConnToCardManager* m_pConnToCardManager;
	CRsrcAlloc* m_pRsrcAlloc;
	char* m_operName;
	std::set<CPartyDebugInfo*>* m_PartyDebugInfoList;
	CTaskApi* m_pRsrvManagerApi;

private:
	STATUS ExtractPartyInfo(std::ostream& answer, DWORD monitor_conf_id, DWORD monitor_party_id);
	BOOL   IsFederalOn();
	void   PartyDebugInfoInd(CSegment* pSeg, bool isCM);
	void   RemoveActiveAlarm(WORD alarm, bool& isStartupEnded);
	void   OnStartupEnded();
	BOOL   IsStartupFinished() const;
	void   AddDSPInfoToFileName(APIS8 ucFileName[MAX_RECORDING_FILE_NAME_LENGTH], APIU8 board_id, APIU8 unit_id);
	void   CorrectServiceName(ALLOC_PARTY_REQ_PARAMS_S* params);
	void   SendLastConfIdReadReqToCDR();
	void   SendRsrcRamSizeReqToMcuMngr();
	void   SendRsrcCPUProfileReqToMcuMngr();
	void   SendRsrcIpServiceParamsReqToCS();
	void   SendUdpPortRangeParamsToCS( const UDP_PORT_RANGE_S& param );
	void   SendCfsParamsToCS();
	void   OnResourceCardTypeInd(WORD board_id);
	STATUS MoveMasterAC(DWORD boardID = 0xFFFF, eChangedStateAC stateAC = eAC_None);
	void   MoveMasterToReserved(BoardID boardId);
	void   MoveReservedToMaster(BoardID boardId, UnitID unitId);
	void   UpdateIpServicesDongleRestrictions();
	void   RTMConfigInit();
	void   DeclareStartupConditions(); //virtual
	void   SetMplUnitFatalInd(CSegment* pMsg, bool recievedIsEnable);
	void   ResourceDeAllocRequest(DEALLOC_PARTY_REQ_PARAMS_S* pParams, DEALLOC_PARTY_IND_PARAMS_S* pResult);
	void   CardRemovedRequest(CARD_REMOVED_IND_S* pParams);
	void   DeallocateAllPartiesOnThisBoardAndFillHWRemovedPartyList(WORD boardId, WORD subBoardId, HW_REMOVED_PARTY_LIST_S* pHw_removed_party_list);
	void   ExecUnitFatalOrUnFatalInd(UNIT_RECOVERY_S* pParams, BYTE recievedIsEnable);
	void   SendRemovedPartyListToConfPartyAndClean(HW_REMOVED_PARTY_LIST_S* pHw_removed_party_list);
	void   SendConfPartyListToConfPartyAssist(CONF_PARTY_LIST_S* party_list);
	void   UpdateAudioController(CSystemResources* pSystemResources);
	void   UpdateIvrController(CSystemResources* pSystemResources);
	void   DeallocateAllPartiesOnUnitAndSendHWRemovedPartyList(CUnitMFA* unitMFA);
	void   Dump(UNIT_RECOVERY_S* pParams);
	void   SetDongleRestriction();
	void   SendRecordingToCm(TStartDebugRecordingReq* pRecord, ConfRsrcID confId, PartyRsrcID partyId);

	CResourceStartupInfo m_resourceStartupInfo;
};

////////////////////////////////////////////////////////////////////////////
//                        CSetPortConfiguration
////////////////////////////////////////////////////////////////////////////
class CSetPortConfiguration : public CSerializeObject
{
public:
	CSetPortConfiguration() { m_selectedIndex = 0; }
	~CSetPortConfiguration() { }

	CSerializeObject* Clone()        { return new CSetPortConfiguration; }
	const char*       NameOf() const { return "CSetPortConfiguration"; }
	void              SerializeXml(CXMLDOMElement*& thisNode) const { PASSERT(1); }
	int               DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action)
	{
		{
			STATUS nStatus = STATUS_OK;
			WORD tmp = m_selectedIndex;
			GET_VALIDATE_CHILD(pNode, "SELECTED_ID", &tmp, _0_TO_WORD);
			m_selectedIndex = tmp;
			return nStatus;
		}
	}

	WORD GetPortConfigurationIndex() const { return m_selectedIndex; }

	CSetPortConfiguration& operator =(const CSetPortConfiguration& other)
	{
		m_selectedIndex = other.m_selectedIndex;
		return *this;
	}

private:
	CSetPortConfiguration(const CSetPortConfiguration&);

	WORD m_selectedIndex;
};

#endif // !defined(_ResourceMANAGER_H__)
