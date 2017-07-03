// McmsDaemonManager.h

#ifndef MCMS_DAEMON_MANAGER_H_
#define MCMS_DAEMON_MANAGER_H_

#include "ManagerTask.h"
#include "Macros.h"
#include "ObjString.h"

class CTerminalCommand;

void McmsDaemonManagerEntryPoint(void* appParam);

class CMcmsDaemonManager : public CManagerTask
{
CLASS_TYPE_1(CMcmsDaemonManager, CManagerTask)
public:
	CMcmsDaemonManager();
	virtual ~CMcmsDaemonManager();

	virtual const char* NameOf() const { return "CMcmsDaemonManager";}

    void SelfKill();
	virtual void ManagerInitActionsPoint();
    virtual void ManagerPostInitActionsPoint();
    virtual void ManagerPreTerminalDeathPoint();

    void  OnKeepAlive(CSegment* pParam);
    void  OnSW_WD_Timer(CSegment* pParam);
    void  OnHW_WD_Timer(CSegment* pMsg);
    void  OnProccessUpStartup(CSegment* pParam);
    void  OnProccessUpReady(CSegment* pParam);
    void  OnResetMcmsReq(CSegment* pParam);
    void  OnConfigApacheInd(CSegment* pParam);
    void  OnInstallerStopIpmc(CSegment* pParam);
    void  OnCheckChildZombieProcessesTimer(CSegment* pParam);
    void  OnStartSignaling(CSegment* pParam);

	TaskEntryPoint GetMonitorEntryPoint();

    virtual int   GetPrivateFileDescriptor();
    virtual void  HandlePrivateFileDescriptor();

    void ProduceResetFaultAndLog();

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS
    void HandleTerminalReset(CTerminalCommand & command,std::ostream& answer);
    void HandleTerminalKillCS(CTerminalCommand & command,std::ostream& answer);

    void HandleEnterDiagnostics(CTerminalCommand & command,std::ostream& answer);
    void HandleAddProcess(CTerminalCommand & command,std::ostream& answer);
    void HandleRemoveProcess(CTerminalCommand & command,std::ostream& answer);
private:
    void TerminateProcess(eProcessType process);
    void HandleProcessFailed(eProcessType process);
    void TryToForceCoreDump(eProcessType process);
    void HandleProcessFailedToStart(eProcessType processsType, const char *stage);
    void ManageProcessWatchList(CTerminalCommand & command,std::ostream& answer,BOOL State);
    void HandlePrintProcessWatchList(CTerminalCommand & command,std::ostream& answer);
	void OnWDTimerHW();
	void OnWDTimerMcms();
	void OnResetHistoryTimer();
	void OnResetProcessReq(CSegment* pParam);
	void OnMcmsNetworkFinish(CSegment* pParam);

	BOOL IsGiveRoot(eProcessType processType)const;
	void DumpProcessLoadInfo(eProcessType processType);
	void DumpConfPartyProcessInfo();
    int  GetProcessNiceLevel(eProcessType processType)const;	// for Call Generator - change MM process priority

	STATUS LaunchProcess(eProcessType processType);
	void TryToAbort(const string &description, eResetSource resetSource);
	virtual void AddFilterOpcodePoint();
    void CheckDebugMode();
    void KillCSModule();
    void KillApacheModule();
    void EnterSafeMode();
    void StartHistoryTimer();
    void OnEndLaunchMcmsProcesses();
    void SendEventToAudit(const string & action,
                          const string & description,
                          const string & descriptionEx,
                          const string & data);

	// for Call Generator - expiration time
    bool Check_CG_expiration_time();
    void OnCG_WD_Timer(CSegment* pMsg);
    void OnStartTimerByProcessAndProductType(eProcessType curProcessType);

    int m_PowerButtonFD;
    BOOL m_LastCheckedDebugMode;
    BOOL m_isDebug_Mode;
    BOOL m_isEnteringInDiagnosticsMode;
    BOOL m_disable_IPMC_usage;
    BOOL m_isMcmsNetworkFinishInd;
    DWORD m_prevToRun;
    BOOL m_isDuringInstalltion;
	std::map<eProcessType, DWORD> m_mapProcessToFaultID;
	BOOL m_isProcessRecievedCoreDump[NUM_OF_PROCESS_TYPES];

    enum eApacheMode
    {
    	eNotInit,
    	eMcuMngrInd, // McuMngr indication has been arrived, that apacheModule can start up.
    	eWaitForStartup // Apache turn to start up has arrived.
    } m_apacheMode;
};

#endif  // MCMS_DAEMON_MANAGER_H_
