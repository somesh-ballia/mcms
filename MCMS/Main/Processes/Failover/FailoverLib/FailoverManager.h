// FailoverManager.h: interface for the CFailoverManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOMANAGER_H__)
#define _DEMOMANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"
#include "FailDetectionApi.h"
#include "FailoverSyncApi.h"
#include "FailoverDefines.h"
#include "McuMngrInternalStructs.h"
#include "LogInConfirm.h"


class CFailoverProcess;
class CFailoverCommunication;

class CFailoverManager : public CManagerTask
{
CLASS_TYPE_1(CFailoverManager,CManagerTask )
public:
	CFailoverManager();
	virtual ~CFailoverManager();

	const char* NameOf() const {return "CFailoverManager";}

	TaskEntryPoint GetMonitorEntryPoint();

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

	void OnMasterDownInd(CSegment* pSeg);
	void OnWholeSyncCompletedInd(CSegment* pSeg);

private:
	virtual void ManagerPostInitActionsPoint();

	void SelfKill();

	void GetInfoForCommunication();
	void InitFailoverConfiguration();
	void EndSetFailoverConfiguration();
	void InitCommunicationConfiguration();
	void ConnectToPairRMX();
	void StartMasterSlaveDetermination();
	void SendLoginMessage();
	void SendLogoutMessage();
	void StartSlave();
	void StartMaster();
	void StartMasterTimersIfNeeded();
	void StartFailDetection();
	void StartFailoverSync();
	void SocketDropped(CSegment* pMsg);

	void StopCommunicate(const string theCaller);
	void SendActiveAlarmDeterminationConflict(string strDescription);
	void HandleMasterSlaveDeterminationLoginResponse(eMasterSlaveState masterSlaveStateResponse, CLogInConfirm* pLogInConfirm);
	void SendUnreachableUrlAlarm(string strMsg);

	void SendFailoverParamsToProcesses(bool isFeatureEnabled, bool isSlave, string theCaller);
	void SendFailoverParamsToSpecProcess(eProcessType processType, bool isFeatureEnabled, bool isSlave, string theCaller);
	void SendFailoverRefreshRegToGatekeeper(bool isSetEndFlag);
	void SendFailoverParamsToMcuMngr();
	void SendFailoverTriggerToMcuMngr();
	eMasterSlaveState ConvertHotBackupTypeToMasterSlaveState(DWORD HotBackupType);
	void RemoveFailoverActiveAlarms();
	void RemoveMasterSlaveConfigurationActiveAlarms();
	void RemoveMasterNetworkIssueActiveAlarm();
	void SendStartMasterBecomeSlaveToConfPartyAndReservation();
	void StopMasterCheckOnGetStateFromSlave();

	BOOL IsNeedToReconnectSocket();
	BOOL IsNetworkOk();
    
    void LedHotStandbyStateInd(CSegment* pMsg = NULL);
	void UpdateMasterSlaveState(eFailoverStatusType eState);
	void MasterGetStateFromSlaveError();
	void MasterGetStateFromSlaveRemoveError();

	const char* GetMasterSlaveStateStr(eMasterSlaveState eState);

	//Action Functions

	void OnMcuMngrConfigIndInitConfiguration(CSegment* pMsg);
	void OnMcuMngrGetStateExIdle(CSegment* pSeg);
	void OnMcuMngrGetStateExAnycase(CSegment* pSeg);
	void OnAuthenticationConfigIndInitConfiguration(CSegment* pMsg);

	void OnSocketFailedConnectingSocket(CSegment* pMsg);
	void OnSocketConnectedConnectingSocket(CSegment* pMsg);

	void OnSocketRcvIndMasterSlaveDetermination(CSegment* pMsg);
	void OnSocketRcvIndIdle(CSegment* pMsg);
	void OnSocketRcvIndAnycase(CSegment* pMsg);

	void OnSocketDroppedConnectingSocket(CSegment* pMsg);
	void OnSocketDroppedMasterSlaveDetermination(CSegment* pMsg);
	void OnSocketDroppedSlaveReady(CSegment* pMsg);
	void OnSocketDroppedSlaveRestoreMaster(CSegment* pMsg);
	void OnSocketDroppedReloginAfterLogout(CSegment* pMsg);
	void OnSocketDroppedIdle(CSegment* pMsg);

	void OnSocketRcvIndlogout(CSegment* pMsg);
	void OnTimerReconnectSocketTimeout(CSegment* pParam);

	void OnTimerMasterPingTimeoutIdle(CSegment* pSeg);
	void OnTimerMasterPingTimeoutAnycase(CSegment* pSeg);

	void OnLoginArrived(CSegment* pSeg);
	void OnTimerReloginTimeout(CSegment* pParam);

	STATUS OnServerSetFailoverConfiguration(CRequest *pRequest);


	void OnTimerMasterGetStateFromSlaveTimeoutIdle(CSegment* pSeg);
	void OnTimerMasterGetStateFromSlaveTimeoutAnycase(CSegment* pSeg);

	void OnConfPartyEndPrepareMasterBecomeSlaveSlaveReady(CSegment* pMsg);
	void OnConfPartyEndPrepareMasterBecomeSlaveAnycase(CSegment* pMsg);
	void OnEventTriggerIndAnycase(CSegment* pMsg);	

	void OnFailoverRestartSlaveInd(CSegment* pMsg);

	STATUS HandleTerminalNetworkDown(CTerminalCommand& cmd, std::ostream& ans);
	void OnMcuMngrSecurityModeInd(CSegment* pMsg);

	/*Begin:added by Richer for BRIDGE-14263, 2014.7.16*/
	BOOL IsStartupFinished() const;
	void OnTimerInitFailoverTimeout(CSegment* pSeg);
	/*End:added by Richer for BRIDGE-14263, 2014.7.16*/
	
	CFailoverProcess		*m_pProcess;
	CFailDetectionApi		*m_pFailDetectionApi;
	CFailoverSyncApi		*m_pFailoverSyncApi;
	CFailoverCommunication	*m_pFailoverCommunicationFromProcess;
	eMasterSlaveState		m_eMasterSlaveCurrentState;
	MASTER_SLAVE_DATA_S 	*m_pMngmntInfoFromProcess;
	BOOL 					m_bIsNetworkProblemsInMaster;
	BOOL					m_bSimuNetworkDown;
	
};

#endif // !defined(_DEMOMANAGER_H__)
