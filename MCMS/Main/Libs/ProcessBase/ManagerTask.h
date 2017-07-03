// ManagerTask.h

#ifndef MANAGER_TASK_H_
#define MANAGER_TASK_H_

#include <vector>

#include "RequestHandler.h"
#include "StructTm.h"

// 30 minutes (increased from 20 minutes upon VNGR-12224)
#define STARTUP_TIME_LIMIT_AFTER_INSTALLATION 30 * 60 * SECOND
#define STARTUP_TIME_LIMIT_WHEN_NO_BOARDS 15 * SECOND
//#define CPU_USAGE_TIMER 300

extern "C" void managerEntryPoint(void* appParam);

class CManagerTask;
class CTaskStateFaultListMap;
class CTerminalCommand;

typedef STATUS (CManagerTask::*HANDLE_COMMAND)(CTerminalCommand & command, std::ostream&);

struct STerminalCommand
{
	STerminalCommand(const char* command,
					 HANDLE_COMMAND function,
					 const char* command_help)
					 :m_command(command),
					 m_function(function),
					 m_command_help(command_help)
	{}

	STerminalCommand(const STerminalCommand& other)
		:m_command(other.m_command),
		 m_function(other.m_function),
		 m_command_help(other.m_command_help)
	{}

	STerminalCommand()
		:m_command(NULL),
		 m_function(NULL),
		 m_command_help(NULL)
	{}

	friend BYTE operator<(const STerminalCommand&, const STerminalCommand&);
	friend BYTE operator==(const STerminalCommand&, const STerminalCommand&);
	const char* m_command;
	HANDLE_COMMAND m_function;
	const char* m_command_help;
};

#define	PDECLAR_TERMINAL_COMMANDS \
	virtual void InitTerminalCommands();

#define BEGIN_TERMINAL_COMMANDS(CLASS) \
	void CLASS::InitTerminalCommands(){ \
	CManagerTask::InitTerminalCommands();

#define BEGIN_BASE_TERMINAL_COMMANDS(CLASS) \
	void CLASS::InitTerminalCommands(){

#define ONCOMMAND(command,function,help) \
	m_terminalCommands.push_back(STerminalCommand(command, COMMAND_FUNC function,help));

#define END_TERMINAL_COMMANDS }

class CDispatcherApi;
class CManagerApi;

class CManagerTask: public CRequestHandler
{
CLASS_TYPE_1(CManagerTask, CRequestHandler)
public:
  static const unsigned int kMaxTraceLevelTimeout;
  static unsigned long long s_stress_log_counter;

	CManagerTask(void);
  virtual ~CManagerTask(void);
  virtual void SelfKill(void);
  virtual void CreateDispatcher(void);
  virtual BOOL IsManager(void) const;
  virtual BOOL IsSingleton(void) const;
  virtual const char* NameOf(void) const;
  virtual const char* GetTaskName(void) const;

  void HandleTerminalCommand(CSegment* seg);
  void HandleTerminalCommandSync(CSegment* pSeg);
  virtual int GetTaskMbxBufferSize(void) const
  {
    return 1024 * 1024 - 1;
  }

  virtual void OnAssertTreatment()
  {}

  CTaskApi* m_pManagerApi;
  CTaskApi* m_pWatchDogApi;
  CTaskApi* m_pDispatcherApi;
  CTaskApi* m_pSyncDispatcherApi;
  CTaskApi* m_pMonitorApi;
  CTaskApi* m_pErrorHandlerApi;

protected:

	static STATUS GetCPUUsageThreshold(unsigned int threshold,
                                     unsigned int& upper,
                                     unsigned int& lower);

    virtual TaskEntryPoint GetMonitorEntryPoint() = 0;

    STATUS HandleTerminalKill(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalDisplayMem(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalDisplayTasks(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalDisplayQueue(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalDisplayTimer(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalDump(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalDumpState(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalDumpAllStateMachines(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalHelp(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalStat(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalPing(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalFilterByLevel(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalLiveProcess(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalTraceLevel(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalCfgCnt(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalConfig(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalSetParam(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalGetParam(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalCoreDump(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalTestException(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalSegFault(CTerminalCommand& command, std::ostream& answer);
    STATUS HandleTerminalBadStateMachinePointer(CTerminalCommand& command,std::ostream& answer);
    STATUS HandleTerminalState(CTerminalCommand& command,std::ostream& answer);
    STATUS HandleTerminalUninitMemory(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalSimLog(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalSimLeak(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalProcessStatistics(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalGetAA(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalSetAA(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalAddAsserts(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalAddComment(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalAddBigFaults(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalAddAA(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalAddAAnoFlush(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalAAFlush(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalRemoveAA(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalRmAA(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalEnableDisable(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalDisableSCTree(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalAddAAFaultOnly(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalRemoveAAFaultOnly(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalUpdateAAFaultOnly(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalWatchDogCnt(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalIpcTrace(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalTail(CTerminalCommand &command, std::ostream& answer);
    STATUS HandleTerminalDeadlock(CTerminalCommand &command, std::ostream& answer);
    STATUS HandleTerminalLogBookmark(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalAllProcessInfo(CTerminalCommand &command, std::ostream& answer);
    STATUS HandleTerminalGenericProcessInfo(CTerminalCommand &command, std::ostream& answer);
    STATUS HandleTerminalSpecificProcessInfo(CTerminalCommand &command, std::ostream& answer);
    STATUS HandleTerminalActiveAlarm(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalProcessMemoryUtilization(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalSendSyncMsg(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalResponseSyncMsg(CTerminalCommand & command,std::ostream& answer);
    STATUS HandleTerminalFaults_to_full_and_short(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalFaults_to_full_only(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalLeakCheck(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalSleepTest(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalGetProductType(CTerminalCommand & command, std::ostream& answer);
    STATUS HandleTerminalLogStressStart(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalLogStressStop(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalPeriodicStat(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalResetMonitorStat(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalTraceLocal(CTerminalCommand& cmd, std::ostream& ans);
    STATUS HandleTerminalDumpAADB(CTerminalCommand& cmd, std::ostream& ans);

	virtual void ManagerInitActionsPoint()
	{}
	virtual void ManagerPostInitActionsPoint()
	{}
	virtual void DeclareStartupConditions()
	{}
	virtual void ManagerStartupActionsPoint()
	{}
	virtual void ManagerPreTerminalDeathPoint()
	{}
	virtual void OnSysConfigTableChanged(const string& key, const string &data)
	{}
  virtual unsigned int GetMaxLegitimateUsagePrecents(void) const;
  virtual void InformHttpGetFile(const std::string& file_name);
  virtual STATUS SpecificProcessInfo(std::ostream& answer);

  void OnInformHttpGetFile(CSegment* seg);
	void OnTaskChangeStateInd(CSegment* seg);
	void CreateRegisterAlarmableTask(CTaskApi* taskApi,
                                   void (*entryPoint)(void*),
                                   const COsQueue*creatorRcvMbx);
	void CreateTask(CTaskApi* taskApi,
                  void (*entryPoint)(void*),
                  const COsQueue* creatorRcvMbx);
	void RegisterAlarmableTask(const CAlarmableTask *taskApp);
	void CalculateSetProcessState();

	static WORD IsProcessAlive(eProcessType processType);

	PDECLAR_MESSAGE_MAP;
	PDECLAR_TERMINAL_COMMANDS;

	std::vector<STerminalCommand> m_terminalCommands;
	BOOL m_toPrint;

private:
	virtual void AlarmableTaskCreateEndPoint();
	virtual BOOL selfKillOnMemoryExhaustion()
	{
	  return FALSE;
	}

	void InitTask();
	void ManagerInitActionsPointBase();
	void ManagerStartupActionsPointBase();
	void StartStartupTimer();
	void StartIdleTimer();
	void HandleIdleTimerEnd(CSegment* seg);
	void HandleStartupTimerEnd(CSegment* seg);
	void PrintIdleTimes();
	void OnStartupEvent(CSegment* seg);
	void OnManagerAlive(CSegment* seg);
	void OnProcessInfo(CSegment* seg);
	void OnTimerSendResponseMessage(CSegment* pMsg);
	void OnAssertInd(CSegment* pMsg);
	void OnRestartStartupTimer(CSegment* pMsg);
	void OnTimerIsStartupFinished(CSegment*);
	void OnTimerCheckMemoryExhaustion(CSegment* pMsg);
	void OnTimerCheckCpuUsage(CSegment* pMsg);
	void OnFailoverSetParamsInd(CSegment* pMsg);
	void OnLoggerAllMaxTraceLevel(CSegment* msg);
	void OnTimerLoggerBusyTimeout(void);
	void OnTimerDumpStatistics(void);
	void StartStatisticsTimer();
	STATUS HandleTerminalSimulateAddAA(CTerminalCommand& command, std::ostream& answer);
	STATUS HandleTerminalSimulateRemoveAA(CTerminalCommand& command, std::ostream& answer);

	// In case more than 80% of the processes space is used , this variable will be TRUE
	BOOL m_IsMemoryExhausted;
	BOOL m_isCpuAlarm;
	DWORD m_iDumpStatisticsInterval;
	TICKS m_lastCpuUsageRead;
	CStructTm m_IdleStartTime;
	CTaskStateFaultListMap* m_TaskFaultContainer;
	unsigned long long m_lastReportedMemoryPercent;
};

#endif  // MANAGER_TASK_H_
