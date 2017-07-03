// ProcessBase.h

#ifndef PROCESS_BASE_H_
#define PROCESS_BASE_H_

#include <vector>
#include <string>

#include "McmsProcesses.h"
#include "DataTypes.h"
#include "PObject.h"
#include "TaskApp.h"
#include "SlotNumConvertor.h"
#include "Semaphore.h"
#include "SystemState.h"
#include "ProductType.h"
#include "CardsStructs.h"
#include "Macros.h"
#include "CNetworkSettings.h"

class COsQueue;
class CTaskApp;
class CManagerApi;
class CListenSocket;
class CFilterTraceContainer;
class CSysConfig;
class COpcodeStringConverter;
class CFaultList;
class CStatusStringConverter;
class CProcessSettings;
class CXMLDOMElement;
class CTerminalCommand;
class CServiceConfigList;

enum eProcessWorkMode
{
  eProcessWorkModeUnitTest,
  eProcessWorkModeNormal
};

typedef std::map<DWORD, DWORD> MsgsSentConuterMap;
typedef std::map<DWORD, DWORD>::iterator MsgsSentConuterMapItr;

static const char* GetProcessWorkModeName(eProcessWorkMode mode)
{
  static const char* names[] =
  {
    "eProcessWorkModeUnitTest",
    "eProcessWorkModeNormal"
  };

  const char* name = (unsigned int)mode < ARRAYSIZE(names) ?
      names[mode] : "Invalid Process Work Mode";

  return name;
}

enum eProcessStatus
{
  eProcessInvalid = -1,
  eProcessNormal  = 0,
  eProcessMinor,
  eProcessMajor,
  eProcessStartup,
  eProcessIdle,
  eProcessTearDown,

  NUM_OF_PROCESS_STATUSES
};

static const char* GetProcessStatusName(eProcessStatus status)
{
  static const char* names[] =
  {
    "Normal",           // eProcessNormal
    "Minor",            // eProcessMinor
    "Major",            // eProcessMajor
    "Startup",          // eProcessStartup
    "Idle",             // eProcessIdle
    "TearDown",         // eProcessTearDown
  };
  const char* name = eProcessInvalid < status && (unsigned int) status < ARRAYSIZE(names) ?
      names[status] : "Invalid Process Type";

  return name;
}

enum eOtherProcessQueueEntry
{
  eDispatcher       = 0,
  eSyncDispatcher   = 1,
  eManager          = 2,
  eMonitor          = 3,

  // DONT FORGET TO UPDATE THIS
  NUM_OF_HANDLES_IN_ENTRY = 4
};

static const char* InfrastructuresTaskNames[] =
{
  "Dispatcher",
  "SyncDispatcher",
  "Manager",
  "Monitor"
};

class CProcessBase : public CPObject
{
  CLASS_TYPE_1(CProcessBase, CPObject)
 public:
  virtual int                  Run();
  virtual int                  SetUp();
  virtual int                  TearDown();
  virtual const char*          NameOf() const {return "CProcessBase";}

                               CProcessBase();
  virtual                     ~CProcessBase();
  virtual eProcessType         GetProcessType() = 0;
  virtual BOOL                 UsingSockets() = 0;
  virtual BOOL                 HasWatchDogTask() {return TRUE;}
  virtual BOOL                 HasErrorHandlerTask() {return TRUE;}
  virtual BOOL                 HasDispatcherTask() {return TRUE;}
  virtual BOOL                 HasSyncDispathcerTask() {return TRUE;}
  virtual BOOL                 HasMonitorTask() {return TRUE;}
  virtual BOOL                 HasSNMPTask() {return FALSE;}
  virtual BOOL                 GivesAwayRootUser() {return TRUE;}
  virtual int                  GetProcessAddressSpace();
  virtual BOOL                 IsHasSettings() {return FALSE;}
  virtual void                 CreateTask(const char* taskName) {}
  virtual std::string          GetIPAddressByBoardId(DWORD board_id) {return "";}
  eProductType                 GetProductType() const;
  
  virtual void                 CloseConnection(const WORD conId){}

  BOOL 						   GetLanRedundancy(const eIpType ipType = eIpType_IpV4) const;
  bool                         IsFlexeraLicenseInSysFlag() const;
  eProductType                 GetLastProductTypeFound() const;
  eProductFamily               GetProductFamily() const;
  static eStringValidityStatus TestStringValidity(char* theString,
                                                  const int maxLength,
                                                  const std::string& sCaller,
                                                  bool isAssert = true);
  static CProcessBase*         GetProcess();
  static const char*           GetProcessName(eProcessType type);
  static void                  FindLiveProcesses(std::ostream& answer);
  static BOOL                  IsProcessAlive(eProcessType processType);
  const COsQueue*              GetOtherProcessQueue(eProcessType otherProcessType,
                                                    eOtherProcessQueueEntry
                                                    queueType = eDispatcher) const;
  void                         CloseOtherProcessQueue(eProcessType otherProcessType = eProcessTypeInvalid);
  eOtherProcessQueueEntry      FindQueueEntry(eProcessType processType,
                                              const COsQueue& queue) const;
  CTaskApp*                    GetCurrentTask() const;
  int                          Add(CTaskApp* pTask);
  int                          Cancel(CTaskApp* pTask);
  void                         SetSelfKill() { m_selfKill = TRUE; }
  void                         ResetMonitoring();
  STATUS                       DumpTasks(std::ostream& answer);
  STATUS                       DumpTasksQueues(std::ostream& answer);
  STATUS                       DumpTasksTimers(std::ostream& answer);
  STATUS                       DumpTasksStateMachines(std::ostream& answer);
  STATUS                       DumpTasksOpcodeTail(std::ostream& answer,
                                                   DWORD numOfMessages);
  STATUS                       SendMessageToAlarmTasks(OPCODE opcode,
                                                       CSegment* pSeg);
  STATUS                       DumpStatistics(std::ostream& answer,
                                              CTerminalCommand& command);
  virtual void                 DumpProcessStatistics(std::ostream& answer,
                                                     CTerminalCommand& command) const {}
  void                         SetProcessAddressSpaceLimit();
  const std::string&           GetOpcodeAsString(const OPCODE opcode) const;
  const std::string&           GetStatusAsString(const STATUS status) const;
  void                         SerializeApiStatuses(CXMLDOMElement* pLanguageNode);
  virtual void                 AddExtraStringsToMap() {}
  virtual void                 SetUpProcess() {}
  virtual void                 TearDownProcess() {}
  void                         SendFileAsXML(char* fileName);
  static eProcessType          GetProcessValueByString(const char* processName);
  virtual TaskEntryPoint       GetManagerEntryPoint() = 0;
  void                         SetListenSocketTask(CListenSocket* listenSocket);
  CListenSocket*               GetListenSocketTask();
  CSysConfig*                  GetSysConfig() const;
  void                         SetSysConfig(CSysConfig* sysConfig);
  BOOL                         GetTestFlag(int boardId) const;
  void                         SetTestFlag(BOOL flagVal, int boardId);
  BOOL                         GetTestSignalFlag() const;
  void                         SetTestSignalFlag(BOOL flagVal);
  const CManagerApi*           GetManagerApi() const  { return m_pManagerApi; }
  int                          GetGroupSID(int GroupId) { return m_taskGroupSemaphore[GroupId]; }
  CFilterTraceContainer*       GetTraceFilterContainer() const;
  eLogLevel                    GetMaxLogLevel() const;
  eProcessStatus               GetProcessStatus();
  void                         SetProcessStatus(eProcessStatus state);
  void                         SetProcessStatus(eProcessStatus state,
                                                CFaultList* faultList);
  eMcuState                    GetSystemState();
  void                         SetSystemState(const std::string& caller,
                                              eMcuState state);
  CTaskApi*                    GetErrorHandlerApi();
  void                         SetErrorHandlerApi(CTaskApi*);
  void                         RemoveBrokenQueue(eProcessType processType);
  int                          GetNumOfTasks() const;
  virtual DWORD                GetMaxTimeForIdle() const;
  virtual DWORD                GetMaxTimeForStartup() { return m_StartupTimeLimit; }
  void                         SetMaxTimeForStartup(DWORD time) { m_StartupTimeLimit = time; }
  virtual bool                 RequiresProcessInstanceForUnitTests() { return false; }
  DWORD                        AddActiveAlarmFromProcess(BYTE subject,
                                                         DWORD errorCode,
                                                         BYTE errorLevel,
                                                         std::string description,
                                                         bool isForEma,
                                                         bool isForFaults,
                                                         DWORD userId = 0xFFFFFFFF,
                                                         DWORD boardId = 0,
                                                         DWORD unitId = 0,
                                                         WORD theType = 0);
  DWORD                        AddActiveAlarmSingleToneFromProcess(BYTE subject,
                                                                   DWORD errorCode,
                                                                   BYTE errorLevel,
                                                                   std::string description,
                                                                   bool isForEma,
                                                                   bool isForFaults,
                                                                   DWORD userId = 0xFFFFFFFF,
                                                                   DWORD boardId = 0,
                                                                   DWORD unitId = 0,
                                                                   WORD theType = 0);
  void                         RemoveActiveAlarmFromProcess(DWORD errorCode);
  void                         RemoveActiveAlarmByErrorCodeUserIdFromProcess(DWORD errorCode,
                                                                             DWORD Id);

  // Active Alarms Fault only. Faults similarly to active alarms stored and can be Later be found.  also positive fault can be sent when they are removed
  DWORD                        AddActiveAlarmFaultOnlyToProcess(BYTE subject,
                                                         DWORD errorCode,
                                                         BYTE errorLevel,
                                                         std::string description,
                                                         DWORD userId = 0xFFFFFFFF,
                                                         DWORD boardId = 0,
                                                         DWORD unitId = 0,
                                                         WORD theType = 0);
  DWORD                        AddActiveAlarmFaultOnlySingleToneToProcess(BYTE subject,
                                                                   DWORD errorCode,
                                                                   BYTE errorLevel,
                                                                   std::string description,
                                                                   DWORD userId = 0xFFFFFFFF,
                                                                   DWORD boardId = 0,
                                                                   DWORD unitId = 0,
                                                                   WORD theType = 0);
  void                         RemoveActiveAlarmFaultOnlyFromProcess(DWORD errorCode);
  void                         RemoveActiveAlarmFaultOnlyByErrorCodeUserIdFromProcess(DWORD errorCode,
                                                                             DWORD Id);

  void                         IncrementWatchDogCnt() { m_WDCnt++; }
  DWORD                        GetWatchDogCnt() const { return m_WDCnt; }
  BOOL                         GetTraceIPC() const    { return m_traceIPC; }
  void                         SetTraceIPC(BOOL newV) { m_traceIPC = newV; }
  void                         AddTaskToMessagePool(const std::string& taskName);
  void                         RemoveTaskFromMessagePool(const std::string& taskName);
  void                         AddMessageToMessagePool(const std::string& taskName,
                                                       OPCODE opcode,
                                                       eProcessType processType);
  void                         AddFilterToMessagePool(const std::string& taskName,
                                                      OPCODE opcode);
  void                         DumpSingleTaskMessagePool(const std::string& taskName,
                                                         std::ostream& ostr);
  void                         DumpAllTasksMessagePool(std::ostream& ostr);
  CProcessSettings*            GetProcessSettings() const { return pCProcessSettings; }

  void                         ParseArgv();
  void                         SetArgv(char** const argv) { m_Argv = argv; }
  void                         SetArgc(int argc) { m_Argc = argc; }
  char * const *               GetArgv() const { return m_Argv; }
  int                          GetArgc() const { return m_Argc; }

  // 0 - indicates normal startup.
  // 1-3 - indicates recovery startup, number indicates number of recoveries
  int                          GetNumStartup() const { return m_NumOfStartups; }
  void                         SetWorkMode(eProcessWorkMode mode) { m_WorkMode = mode; }
  eProcessWorkMode             GetWorkMode() const { return m_WorkMode; }
  DWORD                        GetSNMPTaskId() const { return m_SnmpTaskId; }
  void                         SetSNMPTaskId(DWORD id) { m_SnmpTaskId = id; }
  CServiceConfigList*          GetServiceConfigList() const;
  void                         SetServiceConfigList(CServiceConfigList* serviceConfigList);
  bool                         IsCompletedSetup() const { return m_IsCompletedSetup; }
  eSystemCardsMode             GetRmxSystemCardsModeDefault() const;
  STATUS                       SendProcessSetupToDeamon();
  void                         SetFailoverParams(bool isFeatureEnabled,
                                                 bool isSlaveMode);
  void                         SetIsFailoverFeatureEnabled(bool isEnabled);
  bool                         GetIsFailoverFeatureEnabled();
  void                         SetIsFailoverSlaveMode(bool isSlave);
  bool                         GetIsFailoverSlaveMode();
  virtual bool                 IsFailoverBlockTransaction_SlaveMode(std::string sAction);
  void                         IncreaseMsgsSentConuter(DWORD opcode);
  virtual int                  GetTaskMbxSndBufferSize() const { return -1; }
  bool                         GetCSLogsStateFromSysCfg();
  std::vector<CTaskApp*>*      GetTasks() { return m_pTasks; }
  BOOL                         IsUnderValgrid()  { return m_isUnderValgrid; }

  int                m_IsTreatingOnAssert;
  DWORD              m_numMessageSent;
  DWORD              m_numMsgRcv;
  DWORD              m_numSyncMsgSent;
  DWORD              m_numToutSyncMsg;
  DWORD              m_numLocalMsgSent;
  DWORD              m_numLocalMsgRvc;
  DWORD              m_numExpiredTimers;
  DWORD              m_numTrace;
  DWORD              m_numTraceNotSent;
  MsgsSentConuterMap m_msgsSentConuterMap;
  CListenSocket*     m_pListenSocketTask;

  static CProcessBase* m_pCurrentProcess;
  BOOL m_enableLocalTracer;
  CNetworkSettings  m_NetSettings;
  BOOL				IsFaultsSentMcuMngr() {return m_isFaultsSentMcuMngr;};

  /*Begin:added by Richer for BRIDGE-15015, 11/13/2014*/
  STATUS SendLedAlarmEventToConfigMngr(CSegment *pSeg, OPCODE opcode);
  /*End:added by Richer for BRIDGE-15015, 11/13/2014*/
  
protected:
  void                         initLocalTracer();
  virtual void                 AddExtraOpcodesStrings() {}
  virtual void                 AddExtraStatusesStrings() {}
  void                         AddOpcodeString(OPCODE opcode, const char* str);
  void                         AddStatusString(STATUS status, const char* str);

  CManagerApi*            m_pManagerApi;
  mutable COsQueue*       m_otherProcessQueues[NUM_OF_PROCESS_TYPES][NUM_OF_HANDLES_IN_ENTRY];
  std::vector<CTaskApp*>* m_pTasks;
  int                     m_selfKill;
  int                     m_TasksSemaphoreId;

 private:
  STATUS ForEachTask(void (CTaskApp::*pfunc)(std::ostream&) const, std::ostream& ans) const;

  void GenerateStatusXmlFiles(const char* directory);

  CSysConfig*             m_SysConfig;
  COpcodeStringConverter* m_OpcodeStringConverter;
  CStatusStringConverter* m_StatusStringConverter;
  CFilterTraceContainer*  m_FilterTraceContainer;
  CSystemState*           m_SystemState;
  CServiceConfigList*     m_ServiceConfigList;
  eProcessStatus          m_ProcessState;
  int                     m_taskGroupSemaphore[TASK_GROUPS_NUM];
  DWORD                   m_StartupTimeLimit;
  DWORD                   m_WDCnt;

  // In case more than 80% of the processes space is used , this variable will be TRUE
  BOOL m_IsMemoryExhausted;
  BOOL m_traceIPC;

  BOOL m_testFlag[5];
  BOOL m_testSignalFlag;

  BOOL m_isFaultsSentMcuMngr;

  STATUS CreateAllSemaphores();
  STATUS DeleteAllSemphores();
  STATUS SendStartupEventToManager();
  STATUS SendMessageToManager(eProcessType processDest, CSegment* pSeg,
                              OPCODE opcode);

  STATUS SendStateChangeIndToMcuMngr(CFaultList* faultList);
  bool   IsProcessMustSendToMcuMngr();
  void   DumpProccessMemoryState(std::ostream& answer);
  void   DumpProccessCPUUsagePercentage(std::ostream& answer);

  CTaskApi*         m_pErrorHandlerApi;
  CProcessSettings* pCProcessSettings;

  char** m_Argv;
  int    m_Argc;
  int    m_NumOfStartups;

  eProcessWorkMode m_WorkMode;
  DWORD            m_SnmpTaskId;

  bool m_IsCompletedSetup;
  bool m_bIsFailoverFeatureEnabled;
  bool m_bIsFailoverSlaveMode;


  mutable eProductType m_productTypeLastFound;
  BOOL m_isUnderValgrid;
};

#endif  // PROCESS_BASE_H_
