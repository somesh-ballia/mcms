// ManagerTask.cpp

#include "ManagerTask.h"

#include <ostream>
#include <fstream>
#include <iomanip>
#include <stdlib.h>
#include <algorithm>

#include "Trace.h"
#include "Macros.h"
#include "NStream.h"
#include "ManagerApi.h"
#include "MonitorTask.h"
#include "ProcessBase.h"
#include "WatchDogTask.h"
#include "DispatcherTask.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "TraceClass.h"
#include "FilterTrace.h"
#include "FilterByLevel.h"
#include "SysConfig.h"
#include "StatusesGeneral.h"
#include "FaultsContainer.h"
#include "HlogElement.h"
#include "DefinesGeneral.h"
#include "FaultsDefines.h"
#include "ErrorHandlerTask.h"
#include "TraceStream.h"
#include "TerminalCommand.h"
#include "FilterTraceContainer.h"
#include "ObjString.h"
#include "HlogApi.h"
#include "AlarmStrTable.h"
#include "IncludePaths.h"
#include "OsFileIF.h"
#include "ValgrindSequences.h"
#include "SysConfigKeys.h"
#include "TraceClass.h"
#include "McmsDaemonApi.h"
#include "ConfigManagerApi.h"

#define TIMER_BUF_LEN 3
#define MAX_BUF_SIZE_FROM_TERMINAL_INPUT 100000000

extern char* ProcessTypeToString(eProcessType processType);

PBEGIN_MESSAGE_MAP(CManagerTask)
  ONEVENT(TERMINAL_COMMAND, ANYCASE, CManagerTask::HandleTerminalCommand )
  ONEVENT(SYNC_TERMINAL_COMMAND, ANYCASE, CManagerTask::HandleTerminalCommandSync)
  ONEVENT(XML_REQUEST, ANYCASE , CManagerTask::HandlePostRequest )
  ONEVENT(TASK_CHANGE_STATE_FAULT_IND, ANYCASE, CManagerTask::OnTaskChangeStateInd)
  ONEVENT(PROCESS_IDLE_TIMER, ANYCASE, CManagerTask::HandleIdleTimerEnd )
  ONEVENT(MANAGER_STARTUP_TIMER, ANYCASE, CManagerTask::HandleStartupTimerEnd )
  ONEVENT(STARTUP_EVENT, ANYCASE, CManagerTask::OnStartupEvent )
  ONEVENT(MANAGER_ALIVE, ANYCASE, CManagerTask::OnManagerAlive )
  ONEVENT(MCUMNGR_TO_ALL_PROCESSES_STATE_REQ, ANYCASE, CManagerTask::CalculateSetProcessState )
  ONEVENT(COLLECTOR_PROCESS_INFO_REQ, ANYCASE, CManagerTask::OnProcessInfo )
  ONEVENT(PROCESS_MEMORY_USAGE_TIMER, ANYCASE, CManagerTask::OnTimerCheckMemoryExhaustion)
  ONEVENT(PROCESS_CPU_USAGE_TIMER, ANYCASE, CManagerTask::OnTimerCheckCpuUsage)
  ONEVENT(SEND_RESPONSE_MSG_TIMER, ANYCASE, CManagerTask::OnTimerSendResponseMessage)
  ONEVENT(ON_ASSERT_IND, ANYCASE, CManagerTask::OnAssertInd)
  ONEVENT(INFORM_HTTP_GET_FILE, ANYCASE, CManagerTask::OnInformHttpGetFile)
  ONEVENT(RESTART_STARTUP_TIMER, ANYCASE, CManagerTask::OnRestartStartupTimer)
  ONEVENT(FAILOVER_SET_PARAMS_IND, ANYCASE,	CManagerTask::OnFailoverSetParamsInd)
  ONEVENT(LOGGER_ALL_MAX_TRACE_LEVEL, ANYCASE, CManagerTask::OnLoggerAllMaxTraceLevel)
  ONEVENT(LOGGER_BUSY_TIMER, ANYCASE, CManagerTask::OnTimerLoggerBusyTimeout)
  ONEVENT(IS_STARTUP_FINISHED_TIMER, ANYCASE, CManagerTask::OnTimerIsStartupFinished)
  ONEVENT(PROCESSBASE_DUMP_STATISTICS_TIMER,  ANYCASE,  CManagerTask::OnTimerDumpStatistics )
PEND_MESSAGE_MAP(CManagerTask, CAlarmableTask);

BEGIN_BASE_TERMINAL_COMMANDS(CManagerTask)
  ONCOMMAND("help",CManagerTask::HandleTerminalHelp,"display this manual")
  ONCOMMAND("kill",CManagerTask::HandleTerminalKill,"force kill of destination process")
  ONCOMMAND("dd",CManagerTask::HandleTerminalDisplayMem,"display memory - dd 007a9d89")
  ONCOMMAND("tasks",CManagerTask::HandleTerminalDisplayTasks,"display all process tasks")
  ONCOMMAND("dump",CManagerTask::HandleTerminalDump,"dump CPObject in memory - dump 007a9d89")
  ONCOMMAND("dump_states",CManagerTask::HandleTerminalDumpState,"dump StateMachine in memory - dump 007a9d89")
  ONCOMMAND("stat",CManagerTask::HandleTerminalStat,"display process statistics")
  ONCOMMAND("ping",CManagerTask::HandleTerminalPing,"ping pong test")
  ONCOMMAND("f_level",CManagerTask::HandleTerminalFilterByLevel,"filter traces by level")
  ONCOMMAND("live_process",CManagerTask::HandleTerminalLiveProcess,"display all live processes")
  ONCOMMAND("trace_level",CManagerTask::HandleTerminalTraceLevel,"display all Trace level")
  ONCOMMAND("cfg_cnt",CManagerTask::HandleTerminalCfgCnt,"display counter of CFG params")
  ONCOMMAND("config",CManagerTask::HandleTerminalConfig,"display all system configuration params")
  ONCOMMAND("set",CManagerTask::HandleTerminalSetParam,"set [key] [value] - live set system cfg param")
  ONCOMMAND("get",CManagerTask::HandleTerminalGetParam,"get [key] - display system cfg param value")
  ONCOMMAND("core",CManagerTask::HandleTerminalCoreDump,"dump core file")
  ONCOMMAND("test_expt",CManagerTask::HandleTerminalTestException,"divide by zero - create an exception")
  ONCOMMAND("segment_fault",CManagerTask::HandleTerminalSegFault,"segment_fault - generate segmentation fault")
  ONCOMMAND("bad_state_machine",CManagerTask::HandleTerminalBadStateMachinePointer,"bad_state_machine - send message with invalid pointer to state machine")
  ONCOMMAND("state",CManagerTask::HandleTerminalState,"state - get the state of the process")
  ONCOMMAND("uninit_mem",CManagerTask::HandleTerminalUninitMemory,"uninit_mem - generate uninit memroy usage")
  ONCOMMAND("simulatelog",CManagerTask::HandleTerminalSimLog,"simulatelog - [size] [number]")
  ONCOMMAND("simulateleak",CManagerTask::HandleTerminalSimLeak,"simulateleak - [size] [number]")
  ONCOMMAND("ps",CManagerTask::HandleTerminalProcessStatistics,"ps - process statistics")
  ONCOMMAND("show_aa",CManagerTask::HandleTerminalGetAA,"show_aa - show active alarms of a process")
  ONCOMMAND("add_asserts", CManagerTask::HandleTerminalAddAsserts,  "add_asserts - create ASSERTs")
  ONCOMMAND("add_comment", CManagerTask::HandleTerminalAddComment,  "add_comment - create faults only in full list ")
  ONCOMMAND("add_big_faults", CManagerTask::HandleTerminalAddBigFaults,  "add_big_faults - create big faults")
  ONCOMMAND("add_fault", CManagerTask::HandleTerminalFaults_to_full_and_short,  "add_faults - create faults in short and full lists ")
  ONCOMMAND("add_fault_full", CManagerTask::HandleTerminalFaults_to_full_only,  "add_faults - create faults only in full list ")
  ONCOMMAND("add_aa", CManagerTask::HandleTerminalAddAA,  "add_aa - create ActiveAlarms for tests only")
  ONCOMMAND("add_aa_no_flush", CManagerTask::HandleTerminalAddAAnoFlush,  "add_aa - create ActiveAlarms without flush only")
  ONCOMMAND("aa_flush", CManagerTask::HandleTerminalAAFlush,  "sends all local AA to McuMngr flush only")
  ONCOMMAND("rm_aa", CManagerTask::HandleTerminalRemoveAA,  "rm_aa - remove ActiveAlarms for tests only")
  ONCOMMAND("set_aa", CManagerTask::HandleTerminalSetAA,  "set_aa - Enable/Disable All Active Alarms/Startup Conditions")
  ONCOMMAND("rm_all_aa", CManagerTask::HandleTerminalRmAA,  "rm_all_aa - Remove All Active Alarms")
  ONCOMMAND("enable_aa", CManagerTask::HandleTerminalEnableDisable,  "enable_aa - Enable/ Disable ActiveAlarms")
  ONCOMMAND("add_aa_fo", CManagerTask::HandleTerminalAddAAFaultOnly, "add_aa_fo - Enable/ Disable Fault Only ActiveAlarms. Usage: add_aa_fo [description number] [userId (Optional)]")
  ONCOMMAND("remove_aa_fo", CManagerTask::HandleTerminalRemoveAAFaultOnly, "remove_aa_fo - Remove Active Alarm Fault Only. Usage remove_aa_fo [userId (optional)]")
  ONCOMMAND("update_aa_fo", CManagerTask::HandleTerminalUpdateAAFaultOnly, "update_aa_fo - Update ActiveAlarm Fault Only. Usage update_aa_fo [description number]")
  ONCOMMAND("dis_sc", CManagerTask::HandleTerminalDisableSCTree,"dis_sc - disable startup condition tree")
  ONCOMMAND("ipc_trace", CManagerTask::HandleTerminalIpcTrace,  "ipc_trace [start|stop] trace all ipc messages")
  ONCOMMAND("wd", CManagerTask::HandleTerminalWatchDogCnt,"wd - display watch")
  ONCOMMAND("queue", CManagerTask::HandleTerminalDisplayQueue,"queue - show all queue info")
  ONCOMMAND("timer", CManagerTask::HandleTerminalDisplayTimer,"timer - show all timer info")
  ONCOMMAND("tail", CManagerTask::HandleTerminalTail,"tail - show last received opcodes")
  ONCOMMAND("deadlock", CManagerTask::HandleTerminalDeadlock,"deadlock - infinite loop in manager")
  ONCOMMAND("log_bookmark", CManagerTask::HandleTerminalLogBookmark,"log_bookmark - trace to logger")
  ONCOMMAND("info", CManagerTask::HandleTerminalAllProcessInfo,"info - all process info")
  ONCOMMAND("general_info", CManagerTask::HandleTerminalGenericProcessInfo,"general_info - general info")
  ONCOMMAND("specific_info", CManagerTask::HandleTerminalSpecificProcessInfo,"specific_info - specific information from process")
  ONCOMMAND("memory_stat", CManagerTask::HandleTerminalProcessMemoryUtilization , "Get the Process Memory Utilization percent")
  ONCOMMAND("all_state_machines", CManagerTask::HandleTerminalDumpAllStateMachines,"all_state_machines - list all tasks state machines")
  ONCOMMAND("send_sync_msg", CManagerTask::HandleTerminalSendSyncMsg,	"Sends a sync message")
  ONCOMMAND("send_response_msg", CManagerTask::HandleTerminalResponseSyncMsg,	"Sends a response message")
  ONCOMMAND("leak_check", CManagerTask::HandleTerminalLeakCheck,	"report leaks (only in valgrind)")
  ONCOMMAND("sleep_test",  CManagerTask::HandleTerminalSleepTest,	"activates 'SystemSleep' method")
  ONCOMMAND("product_type", CManagerTask::HandleTerminalGetProductType,	"print the product type and family")
  ONCOMMAND("log_stress_start", CManagerTask::HandleTerminalLogStressStart,"start producing a fault every 0.25 sec")
  ONCOMMAND("log_stress_stop", CManagerTask::HandleTerminalLogStressStop, "stop producing a fault every 0.25 sec")
  ONCOMMAND("periodic_stat",CManagerTask::HandleTerminalPeriodicStat,"display process statistics periodically in seconds")
  ONCOMMAND("reset_stat",CManagerTask::HandleTerminalResetMonitorStat,"reset process monitoring statistics")
  ONCOMMAND("set_local_tracer",CManagerTask::HandleTerminalTraceLocal,"set on/off the local tracer file")

  ONCOMMAND("simulate_add_aa", CManagerTask::HandleTerminalSimulateAddAA,  "simulate_add_aa - simulate adding specific aa")
  ONCOMMAND("simulate_rm_aa", CManagerTask::HandleTerminalSimulateRemoveAA,  "simulate_rm_aa - simulate removing specific aa")
  ONCOMMAND("dump_aa_db", CManagerTask::HandleTerminalDumpAADB, "dump_aa_db - print AA DB")


END_TERMINAL_COMMANDS;

// Static
const unsigned int CManagerTask::kMaxTraceLevelTimeout = 10 * SECOND;

// Static
unsigned long long CManagerTask::s_stress_log_counter;

CManagerTask::CManagerTask(void) :
  m_pManagerApi(NULL),
  m_pWatchDogApi(NULL),
  m_pDispatcherApi(NULL),
  m_pSyncDispatcherApi(NULL),
  m_pMonitorApi(NULL)    ,
  m_pErrorHandlerApi(NULL),
  m_toPrint(FALSE),
  m_IsMemoryExhausted(FALSE),
  m_isCpuAlarm(FALSE),
  m_iDumpStatisticsInterval(0),
  m_lastCpuUsageRead(0),
  m_TaskFaultContainer(NULL),
  m_lastReportedMemoryPercent(0)
{
  m_type = "CManagerTask";
  m_IdleStartTime.InitDefaults();
}

// Virtual
CManagerTask::~CManagerTask()
{
  POBJDELETE(m_pManagerApi);
  POBJDELETE(m_pErrorHandlerApi);
  POBJDELETE(m_pWatchDogApi);
  POBJDELETE(m_pDispatcherApi);
  POBJDELETE(m_pSyncDispatcherApi);
  POBJDELETE(m_pMonitorApi);
  POBJDELETE(m_TaskFaultContainer);
}

// Virtual
BOOL CManagerTask::IsManager(void) const
{
  return YES;
}

// Virtual
const char* CManagerTask::NameOf(void) const
{
  return GetCompileType();
}

// Virtual
BOOL CManagerTask::IsSingleton(void) const
{
  return YES;
}

// Virtual
const char* CManagerTask::GetTaskName(void) const
{
  return InfrastructuresTaskNames[eManager];
}

void CManagerTask::InitTask(void)
{
  // 0. start timer for idle state.
  StartIdleTimer();

  SetTaskState(eTaskStateIdle);
  TRACEINTOFUNC << "TaskState was set to IDLE";

  CProcessBase* process = CProcessBase::GetProcess();

  // Starts CPU usage checks after startup
  // It starts CPU usage checks after startup
  StartTimer(IS_STARTUP_FINISHED_TIMER, 30 * SECOND);

  m_TaskFaultContainer = new CTaskStateFaultListMap;

  RegisterAlarmableTask(this);

  DeclareStartupConditions();


  InitTerminalCommands();
  CRequestHandler::InitTask();

  //  1. create ErrorHandler Task
  if (process->HasErrorHandlerTask())
  {
    m_pErrorHandlerApi = new CTaskApi;
    CreateTask(m_pErrorHandlerApi, errorHandlerEntryPoint, m_pRcvMbx);
    process->SetErrorHandlerApi(m_pErrorHandlerApi);
  }

  //  2. create WD Task
  if (process->HasWatchDogTask())
  {
    // put here code to check system.cfg watch dog
    m_pWatchDogApi = new CTaskApi;
    CreateTask(m_pWatchDogApi, watchDogEntryPoint, m_pRcvMbx);
  }

  //  3. create Dispatcher Tasks
  if (process->HasDispatcherTask())
    CreateDispatcher();

  if (process->HasSyncDispathcerTask())
  {
    m_pSyncDispatcherApi = new CTaskApi;
    CreateTask(m_pSyncDispatcherApi, syncDispatcherEntryPoint, m_pRcvMbx);
  }

  //  4. create Monitor Task
  if (process->HasMonitorTask())
  {
    m_pMonitorApi = new CTaskApi;
    CreateRegisterAlarmableTask(m_pMonitorApi,
                                GetMonitorEntryPoint(),
                                m_pRcvMbx);
  }

  // 5. idle actions per manager(read system cfg by default)
  ManagerInitActionsPointBase();

  // 6. Send McmsDaemon to keep on loading processes,
  //    so all of the next process' work, will be done in backround
  process->SendProcessSetupToDeamon();

  // 7. All the actions the manager has to do to complete its configuration
  ManagerPostInitActionsPoint();

  // 8. set the state(not status) of manager to startup.
  SetTaskState(eTaskStateStartup);
  TRACEINTOFUNC << "TaskState was set to STARTUP";

  StartStatisticsTimer();
}

void CManagerTask::CreateDispatcher()
{
  m_pDispatcherApi = new CTaskApi;
  CreateTask(m_pDispatcherApi, dispatcherEntryPoint, m_pRcvMbx);
}

void CManagerTask::SelfKill()
{
  if (m_pMonitorApi)
    m_pMonitorApi->SyncDestroy();

  if (m_pSyncDispatcherApi)
    m_pSyncDispatcherApi->SyncDestroy();

  if (m_pDispatcherApi)
    m_pDispatcherApi->SyncDestroy();

  if (m_pWatchDogApi)
    m_pWatchDogApi->SyncDestroy();

  if (m_pErrorHandlerApi)
    m_pErrorHandlerApi->SyncDestroy();

  CTaskApp::SelfKill();
}

// Virtual
unsigned int CManagerTask::GetMaxLegitimateUsagePrecents(void) const
{
  return 35;
}

void CManagerTask::OnTaskChangeStateInd(CSegment *pSeg)
{

  CTaskStateFaultList *taskStateFault = new CTaskStateFaultList;
  taskStateFault->DeSerialize(*pSeg);

  m_TaskFaultContainer->UpdateTask(taskStateFault->GetTaskName(),
                                   taskStateFault);

  CalculateSetProcessState();
}

void CManagerTask::CalculateSetProcessState()
{
  static CProcessBase *process = CProcessBase::GetProcess();
  eProcessStatus newProcessState =
      m_TaskFaultContainer->CalculateProcessState();

  CFaultList faultList;
  m_TaskFaultContainer->GetCommonFaultList(&faultList);
  process->SetProcessStatus(newProcessState, &faultList);
}

void CManagerTask::HandleTerminalCommandSync(CSegment* pSeg)
{
  char uniqueName[MAX_QUEUE_NAME_LEN];
  *pSeg >> uniqueName;

  CTerminalCommand command;
  command.DeSerialize(*pSeg);
  const string & commandName = command.GetCommandName();

  CProcessBase* proc = CProcessBase::GetProcess();

  COstrStream answer;
  answer << "***** " << proc->GetProcessName(proc->GetProcessType())
         << " *****\n";

  bool isCommandFound = false;
  std::vector<STerminalCommand>::iterator iTer = m_terminalCommands.begin();
  std::vector<STerminalCommand>::iterator iEnd = m_terminalCommands.end();
  while (iTer != iEnd)
  {
    if (commandName == iTer->m_command)
    {
      HANDLE_COMMAND function = iTer->m_function;
      STATUS stat = (this->*(function))(command, answer);

      isCommandFound = true;
      break;
    }
    iTer++;
  }
  if (false == isCommandFound)
  {
    answer << commandName.c_str() << " : command not found" << "\n";
  }

  string buff = answer.str();
  CSegment *pRspSeg = new CSegment;
  *pRspSeg << buff;

  COsQueue rcvMbx;
  rcvMbx.CreateWrite(eProcessMcuCmd, uniqueName);

  CTaskApi api;
  api.CreateOnlyApi(rcvMbx);
  api.SendMsg(pRspSeg, 0);
  api.DestroyOnlyApi();
  rcvMbx.Delete();
}

void CManagerTask::HandleTerminalCommand(CSegment * pSeg)
{
  CTerminalCommand command;
  command.DeSerialize(*pSeg);

  const string &terminalName = command.GetTerminalName();
  const string &commandName = command.GetCommandName();

  CProcessBase * proc = CProcessBase::GetProcess();
  COstrStream answer;
  answer << "***** " << proc->GetProcessName(proc->GetProcessType())
      << " *****\n";

  std::vector<STerminalCommand>::iterator iTer = m_terminalCommands.begin();
  std::vector<STerminalCommand>::iterator iEnd = m_terminalCommands.end();
  while (iTer != iEnd)
  {
    if (commandName == iTer->m_command)
    {
      HANDLE_COMMAND function = iTer->m_function;
      STATUS stat = (this->*(function))(command, answer);

      PTRACECOMMAND(terminalName.c_str(), commandName.c_str(), answer.str().c_str());
      return;
    }
    iTer++;
  }

  answer << commandName.c_str() << " : command not found" << "\n";
  PTRACECOMMAND(terminalName.c_str(), commandName.c_str(), answer.str().c_str());
}

STATUS CManagerTask::HandleTerminalKill(CTerminalCommand& command,
                                        std::ostream& answer)
{
  ManagerPreTerminalDeathPoint();

  m_selfKill = TRUE;
  m_resetSource = eResetSourceExternalTerminal;
  m_ResetDescription = "Terminal command [kill] was received";
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalDisplayMem(CTerminalCommand& command,
                                              std::ostream& answer)
{
  const std::string& param1 = command.GetToken(eCmdParam1);

  DWORD x;
  sscanf(param1.c_str(), "%x", &x);

  void * mem_address = (void*) x;
  for (int i = 0; i < 32; i++)
  {
    DWORD mem_content = *((DWORD*) mem_address);
    answer << mem_address << "\t" << (void*) mem_content << "\n";
    x += 4;
    mem_address = (void*) x;
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalDump(CTerminalCommand& cmd,
                                        std::ostream& ans)
{
  if (1 != cmd.GetNumOfParams())
  {
    ans << "dump [mem address]";
    return STATUS_OK;
  }

  DWORD val;
  std::string sval = cmd.GetToken(eCmdParam1);
  sscanf(sval.c_str(), "%x", &val);

  CPObject* mem = (CPObject*) val;
  if (!CPObject::IsValidPObjectPtr(mem))
  {
    ans << "Illegal memory address " << sval;
    return STATUS_OK;
  }

  mem->Dump(ans);
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalDumpState(CTerminalCommand& cmd,
                                             std::ostream& ans)
{
  if (1 != cmd.GetNumOfParams())
  {
    ans << "dump_states [mem address]";
    return STATUS_OK;
  }

  DWORD val;
  std::string sval = cmd.GetToken(eCmdParam1);
  sscanf(sval.c_str(), "%x", &val);

  CStateMachine* mem = (CStateMachine*) val;
  if (!CPObject::IsValidPObjectPtr(mem))
  {
    ans << "Illegal memory address " << sval;
    return STATUS_OK;
  }

  mem->CStateMachine::Dump(ans);
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalDumpAllStateMachines(CTerminalCommand& cmd,
                                                        std::ostream& ans)
{
  CProcessBase::GetProcess()->DumpTasksStateMachines(ans);
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalDisplayTasks(CTerminalCommand& command,
                                                std::ostream& answer)
{
  answer << "\n";
  return CProcessBase::GetProcess()->DumpTasks(answer);
}

STATUS CManagerTask::HandleTerminalDisplayQueue(CTerminalCommand & command,
                                                std::ostream& answer)
{
  answer << "\n";
  return CProcessBase::GetProcess()->DumpTasksQueues(answer);
}

STATUS CManagerTask::HandleTerminalDisplayTimer(CTerminalCommand& command,
                                                std::ostream& answer)
{
  answer << "\n";
  return CProcessBase::GetProcess()->DumpTasksTimers(answer);
}


STATUS CManagerTask::HandleTerminalSendSyncMsg(CTerminalCommand & command,
                                               std::ostream& answer)
{
  PTRACE2(eLevelInfoNormal,"CManagerTask::HandleTerminalSendSyncMsg: Name - ",GetTaskName());

  DWORD numOfParams = command.GetNumOfParams();
  if (3 != numOfParams)
  {
    answer << "error: Illegal number of parameters\n";
    answer << "usage: Bin/McuCmd turn_ssh_on McuMngr [ON/*]\n";
    return STATUS_FAIL;
  }

  const string &SenderTimer = command.GetToken(eCmdParam1);
  const string &ReceiverName = command.GetToken(eCmdParam2);
  const string &ReceiverTimer = command.GetToken(eCmdParam3);

  //The timeout of sender for waiting to the response
  DWORD iSenderTimer = atoi(SenderTimer.c_str());

  eProcessType processType = CProcessBase::GetProcessValueByString(
      ReceiverName.c_str());

  if (eProcessTypeInvalid == processType)
  {
    cout << "Illegal destination during sync command: " << ReceiverName << endl
        << endl;
    return STATUS_FAIL;
  }

  const char *terminal_file_name = "/dev/null";
  const int argc = 4;

  char cReceiverTimer[TIMER_BUF_LEN + 1];
  strncpy(cReceiverTimer, ReceiverTimer.c_str(), TIMER_BUF_LEN);

  char *argv[] = { "dummy1", "send_response_msg", "dummy2", cReceiverTimer };
  CTerminalCommand command2(terminal_file_name, argv, argc);

  CSegment *pSeg = new CSegment;
  command2.Serialize(*pSeg);

  CSegment rspMsg;
  OPCODE rspOpcode;

  CManagerApi eReceiverManagerApi(processType);
  STATUS responseStatus = eReceiverManagerApi.SendMessageSync(pSeg,
                                                              TERMINAL_COMMAND,
                                                              iSenderTimer * SECOND,
                                                              rspOpcode,
                                                              rspMsg);
  return responseStatus;
}

STATUS CManagerTask::HandleTerminalResponseSyncMsg(CTerminalCommand& command,
                                                   std::ostream& answer)
{
  PTRACE2(eLevelInfoNormal,"CManagerTask::HandleTerminalResponseSyncMsg: Name - ",GetTaskName());

  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: Illegal number of parameters\n"
           << "usage: Bin/McuCmd turn_ssh_on McuMngr [ON/*]\n";
    return STATUS_FAIL;
  }

  const std::string ReceiverTimer = command.GetToken(eCmdParam1);

  //The timeout of sender for waiting to the response
  DWORD iSenderTimer = atoi(ReceiverTimer.c_str());

  CSegment *pClientRspInfo = new CSegment;

  m_pClientRspMbx->Serialize(*pClientRspInfo);

  *pClientRspInfo << (DWORD&) m_ClientStateMachineDesc.m_pStateMachine
                  << m_ClientStateMachineDesc.m_StateMachineHandle
                  << m_ClientRspMsgSeqNum
      << m_ClientRspMsgType;

  StartTimer(SEND_RESPONSE_MSG_TIMER, iSenderTimer * SECOND, pClientRspInfo);

  return 0;
}

void CManagerTask::OnTimerSendResponseMessage(CSegment* pMsg)
{
  PTRACE2(eLevelInfoNormal,"CManagerTask::OnTimerSendResponseMessage: Name - ",GetTaskName());

  COsQueue pClientRspMbx;
  pClientRspMbx.DeSerialize(*pMsg);

  StateMachineDescriptor ClientStateMachineDesc;
  StateMachineDescriptor* p;

  *pMsg >> (DWORD&) ClientStateMachineDesc.m_pStateMachine;
  *pMsg >> ClientStateMachineDesc.m_StateMachineHandle;

  DWORD ClientRspMsgSeqNum = 0;
  *pMsg >> ClientRspMsgSeqNum;

  DWORD ClientRspMsgType = 0;
  *pMsg >> ClientRspMsgType;

  ResponedClientRequest(pClientRspMbx,
                        ClientRspMsgSeqNum,
                        ClientRspMsgType,
                        &ClientStateMachineDesc,
                        TEST_RESPONSE_MSG_IND, NULL);
}

STATUS CManagerTask::HandleTerminalHelp(CTerminalCommand& command,
                                        std::ostream& answer)
{
  const std::string &param1 =
      (0 < command.GetNumOfParams() ? command.GetToken(eCmdParam1) : "");

  std::vector<STerminalCommand>::iterator itr = m_terminalCommands.begin();

  while (itr != m_terminalCommands.end())
  {
    if (param1.empty() || param1 == itr->m_command)
    {
      answer << itr->m_command << " - " << itr->m_command_help << "\n";
    }
    itr++;
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalStat(CTerminalCommand& command,
                                        std::ostream& answer)
{
   return CProcessBase::GetProcess()->DumpStatistics(answer, command);
}

STATUS CManagerTask::HandleTerminalPing(CTerminalCommand& command,
                                        std::ostream& answer)
{
  answer << "pong to console : cucu-lulu\n";

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalFilterByLevel(CTerminalCommand& command,
                                                 std::ostream& answer)
{
  CFilterByLevel flt(command);
  if (flt.IsValid())
  {
    CFilterTraceContainer *fltContainer =
        CProcessBase::GetProcess()->GetTraceFilterContainer();
    fltContainer->AddFilter(flt);
  }
  else
  {
    answer << "Invalid filter" << endl;
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalLiveProcess(CTerminalCommand& command,
                                               std::ostream& answer)
{
  CProcessBase::FindLiveProcesses(answer);

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalProcessStatistics(
    CTerminalCommand & command, std::ostream& answer)
{
  return DumpSystemStatistics(answer);
}

STATUS CManagerTask::HandleTerminalTraceLevel(CTerminalCommand& command,
                                              std::ostream& answer)
{
  CTrace::FillTraceLevelNames(answer);

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalConfig(CTerminalCommand& command,
                                          std::ostream& answer)
{
  if (1 != command.GetNumOfParams())
  {
    answer << "Bad usage\n"
           << "ca config DEST [mcms/cs/debug/all]\n";
    return STATUS_OK;
  }

  eCfgParamType cfgType = eCfgParamUser;
  eCfgSections section = NumOfCfgSections;
  const string &param = command.GetToken(eCmdParam1);
  if (param == "mcms")
  {
    section = eCfgSectionMcmsUser;
    cfgType = eCfgParamUser;
  }
  else if (param == "cs")
  {
    section = eCfgSectionCSModule;
    cfgType = eCfgParamUser;
  }
  else if (param == "debug")
  {
    section = eCfgSectionMcmsDebug;
    cfgType = eCfgParamDebug;
  }
  else if (param == "all")
  {
    section = NumOfCfgSections;
  }
  else
  {
    answer << "Bad usage\n" << param.c_str() << " - unknown option\n";
    return STATUS_OK;
  }

  CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();

  if (NumOfCfgSections != section)
  {
    pSysConfig->DumpByCfgType(answer, cfgType, section);
  }
  else if (NumOfCfgSections == section)
  {
    pSysConfig->Dump(answer);
  }
  else
  {
    PASSERTMSG(section + 100, "Bad Flow(code : section + 100)");
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalSetParam(CTerminalCommand& command,
                                            std::ostream& answer)
{
  const DWORD numOfParams = command.GetNumOfParams();
  if (numOfParams != 2)
  {
    answer << "Error\n"
           << "Usage: ca set [Destination] [Param1] [Param2]";
    return STATUS_OK;
  }

  const string &key = command.GetToken(eCmdParam1);
  const string &data = command.GetToken(eCmdParam2);

  BOOL res = CProcessBase::GetProcess()->GetSysConfig()->OverWriteParam(key,
      data);
  if (TRUE == res)
  {
    OnSysConfigTableChanged(key, data);
  }
  else
  {
    answer << "FAILED to set cfg param : " << key.c_str();
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalGetParam(CTerminalCommand& command,
                                            std::ostream& answer)
{
  const DWORD numOfParams = command.GetNumOfParams();
  if (numOfParams != 1)
  {
    answer << "Error\n"
           << "Usage: ca get [Destination] [cfg_key]";
    return STATUS_OK;
  }

  const string &key = command.GetToken(eCmdParam1);
  CProcessBase::GetProcess()->GetSysConfig()->DumpByKey(answer, key);

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalUninitMemory(CTerminalCommand& command,
                                                std::ostream& answer)
{
  char * temp = new char[10];
  if (temp[1] == 0)
  {
    int k = 0;
    k++;
  }

  delete[] temp;

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalCfgCnt(CTerminalCommand & command,
                                          std::ostream& answer)
{
  const string &param1 = command.GetToken(eCmdParam1);
  int limit = atoi(param1.c_str());

  CProcessBase *process = CProcessBase::GetProcess();
  process->GetSysConfig()->FillCnt(answer, limit);

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalCoreDump(CTerminalCommand& command,
                                            std::ostream& answer)
{
  SystemCoreDump(TRUE);
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalTestException(CTerminalCommand& command,
                                                 std::ostream& answer)
{
  int a = 1, b = 0, c;

  c = a / b;

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalSimLog(CTerminalCommand& command,
                                          std::ostream& answer)
{
	if (1 > command.GetNumOfParams())
	{
		answer << "Bad usage\n"
	           << "ca simulatelog [destination] [size] [number]\n";
	    return STATUS_FAIL;
	}

	const std::string &size = command.GetToken(eCmdParam1);
	const std::string &num = command.GetToken(eCmdParam2);

    bool isNumeric = CObjString::IsNumeric(num.c_str());
    if (false == isNumeric)
    {
      answer << "error: Parameter must be numeric, not " << num.c_str()
             << '\n'
             << "usage: Bin/McuCmd simulatelog [destination] [size] [number]\n";
      return STATUS_FAIL;
    }

    int isize = atoi(size.c_str());
    int inum = atoi(num.c_str());

  TICKS before = SystemGetTickCount();

  //===================================
  // Normalizing isize for allocation
  //===================================
  if (isize < 1) isize = 1;
  else if (isize > MAX_BUF_SIZE_FROM_TERMINAL_INPUT) isize = MAX_BUF_SIZE_FROM_TERMINAL_INPUT;

  char* buffer = new char[isize+1];

  memset(buffer, 'X', isize);
  buffer[isize] = 0;

  if (inum > 0)
  {
	  for (int i = 0; i < inum; i++)
	  {
		  TRACESTR(eLevelInfoNormal) << buffer;
		  usleep(100);
	  }
  }
  TICKS after = SystemGetTickCount();

  delete[] buffer;

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalLogBookmark(CTerminalCommand& command,
                                               std::ostream& answer)
{
  const DWORD BufferLen = 128;
  char buffer[BufferLen];

  const DWORD numOfParams = command.GetNumOfParams();
  if (numOfParams == 0)
  {
    memset(buffer, 'X', BufferLen - 1);
    buffer[BufferLen-1] = '\0';
  }
  else
  {
    const string &bookMarkText = command.GetToken(eCmdParam1);
    strncpy(buffer, bookMarkText.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
  }

  TRACESTR(eLevelInfoNormal) << '\n' << buffer;

  answer << "Bookmark : " << buffer << '\n';

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalSimLeak(CTerminalCommand& command,
                                           std::ostream& answer)
{
	if (1 > command.GetNumOfParams())
	{
		answer << "Bad usage\n"
	           << "ca simulateleak - [size] [number]\n";
	    return STATUS_FAIL;
	}

	const std::string &size = command.GetToken(eCmdParam1);
	const std::string &num = command.GetToken(eCmdParam2);

    bool isNumeric = CObjString::IsNumeric(num.c_str());
    if (false == isNumeric)
    {
      answer << "error: Parameter must be numeric, not " << num.c_str()
             << '\n'
             << "usage: Bin/McuCmd simulateleak [destination] [size] [number]\n";
      return STATUS_FAIL;
    }

    int isize = atoi(size.c_str());
    int inum = atoi(num.c_str());

  //===================================
  // Normalizing isize for allocation
  //===================================
  if (isize < 1) isize = 1;
  else if (isize > MAX_BUF_SIZE_FROM_TERMINAL_INPUT) isize = MAX_BUF_SIZE_FROM_TERMINAL_INPUT;

  if (inum > 0)
  {
	  for (int i = 0; i < inum; i++)
		  char* buffer = new char[isize];
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalSegFault(CTerminalCommand& command,
                                            std::ostream& answer)
{
  COsQueue* pqueue = (COsQueue*) 0xffffffff;
  delete pqueue;
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalBadStateMachinePointer(CTerminalCommand& command,
                                                          std::ostream& answer)
{
  //TODO
  /*    CTaskApi api;
   StateMachineDescriptor desc((CStateMachine*) 0xffffffff,
   1234);

   api.CreateOnlyApi(*m_pRspMbx,desc);
   api.SendMsg(NULL,0);*/

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalState(CTerminalCommand& command,
                                         std::ostream& answer)
{
  eProcessStatus processStatus = CProcessBase::GetProcess()->GetProcessStatus();
  answer << "Status : " << GetProcessStatusName(processStatus);

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalIpcTrace(CTerminalCommand& command,
                                            std::ostream& answer)
{
  const string &state = command.GetToken(eCmdParam1);

  if (state == "start")
  {
    CProcessBase::GetProcess()->SetTraceIPC(TRUE);
  }
  else if (state == "stop")
  {
    CProcessBase::GetProcess()->SetTraceIPC(FALSE);
  }
  else
  {
    answer << "Usage: ca start/stop [Destination Process]";
  }
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalGetAA(CTerminalCommand& command,
                                         std::ostream& answer)
{
  CTaskStateFaultList *iTer =
      m_TaskFaultContainer->GetFirstTaskStateFaultList();
  while (NULL != iTer)
  {
    answer << iTer->GetTaskName() << " task : " << GetTaskStateName(
        iTer->GetTaskState()) << endl;
    iTer->DumpActiveAlarmList(answer);
    answer << endl;

    iTer = m_TaskFaultContainer->GetNextTaskStateFaultList();
  }
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalActiveAlarm(CTerminalCommand& command,
                                               std::ostream& answer)
{
  CTaskStateFaultList *iTer =
      m_TaskFaultContainer->GetFirstTaskStateFaultList();
  while (NULL != iTer)
  {
    answer << iTer->GetTaskName() << " task : " << GetTaskStateName(
        iTer->GetTaskState()) << endl;
    iTer->DumpActiveAlarmList(answer);
    answer << endl;

    iTer = m_TaskFaultContainer->GetNextTaskStateFaultList();
  }
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalAddAsserts(CTerminalCommand& command,
                                              std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (1 > numOfParams)
  {
    answer << "error: Number must be specified, description is optional\n"
           << "usage: Bin/McuCmd add_asserts [destination] [Number][description]\n";
    return STATUS_FAIL;
  }

  const string &strNum = command.GetToken(eCmdParam1);

  bool isNumeric = CObjString::IsNumeric(strNum.c_str());
  if (false == isNumeric)
  {
    answer << "error: Parameter must be numeric, not " << strNum.c_str()
           << '\n'
           << "usage: Bin/McuCmd add_asserts [destination] [Number][description]\n";
    return STATUS_FAIL;
  }

  int numOfAsserts = atoi(strNum.c_str());

  const char* description =
      (2 == numOfParams ? command.GetToken(eCmdParam2).c_str() : "Test Assert");

  if (numOfAsserts>0)
	  for (int i = 0; i < numOfAsserts; i++)
	  {
		  PASSERTMSG(TRUE, description);
		  SystemSleep(20);
	  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalAddComment(CTerminalCommand& command, std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if (1 > numOfParams)
	{
		answer << "error: Number must be specified, description is optional\n"
				<< "usage: Bin/McuCmd add_comment [comment]\n";
		return STATUS_FAIL;
	}

	const string &sComment = command.GetToken(eCmdParam1);
	TRACEINTO << sComment;

	return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalAddBigFaults(CTerminalCommand& command,
                                                std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: Number must be specified\n"
           << "usage: Bin/McuCmd add_big_faults [destination] [Number]\n";
    return STATUS_FAIL;
  }

  const string &strNum = command.GetToken(eCmdParam1);

  bool isNumeric = CObjString::IsNumeric(strNum.c_str());
  if (false == isNumeric)
  {
    answer << "error: Parameter must be numeric, not " << strNum.c_str()
           << '\n'
           << "usage: Bin/McuCmd add_big_faults [destination] [Number]\n";
    return STATUS_FAIL;
  }

  int numOfAAs = atoi(strNum.c_str());

  if (numOfAAs>0)
  {
	  for (int i = 0, idxPlusOne = 0; i < numOfAAs; i++, idxPlusOne++)
	  {

		string desc;
		desc = "- Test " + idxPlusOne;
		desc += "123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-123456789-";

		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
							TEST_ERROR,
							MAJOR_ERROR_LEVEL,
							desc.c_str(),
							FALSE);

		SystemSleep(50);
	  }
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalFaults_to_full_and_short(
    CTerminalCommand& command, std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: Number must be specified\n"
           << "usage: Bin/McuCmd add_fault [destination] [Number]\n";
    return STATUS_FAIL;
  }

  const string &strNum = command.GetToken(eCmdParam1);

  bool isNumeric = CObjString::IsNumeric(strNum.c_str());
  if (false == isNumeric)
  {
    answer << "error: Parameter must be numeric, not " << strNum.c_str()
           << '\n'
           << "usage: Bin/McuCmd add_fault [destination] [Number]\n";
    return STATUS_FAIL;
  }

  int numOfAAs = atoi(strNum.c_str());

  if (numOfAAs > 0)
  {
	  for (int i = 0, idxPlusOne = 0; i < numOfAAs; i++, idxPlusOne++)
	  {

		char desc[ONE_LINE_BUFFER_LEN];
		sprintf(desc, "- - - Test %d - - -", i + 1);

		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
							TEST_ERROR, MAJOR_ERROR_LEVEL,
							desc, FALSE);

		SystemSleep(50);
	  }
  }

  return STATUS_OK;
}
STATUS CManagerTask::HandleTerminalFaults_to_full_only(CTerminalCommand& command,
                                                       std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: Number must be specified\n"
           << "usage: Bin/McuCmd add_fault_full [destination] [Number]\n";
    return STATUS_FAIL;
  }

  const string &strNum = command.GetToken(eCmdParam1);

  bool isNumeric = CObjString::IsNumeric(strNum.c_str());
  if (false == isNumeric)
  {
    answer << "error: Parameter must be numeric, not " << strNum.c_str()
           << '\n'
           << "usage: Bin/McuCmd add_fault_full [destination] [Number]\n";
    return STATUS_FAIL;
  }

  int numOfAAs = atoi(strNum.c_str());

  if (numOfAAs > 0)
  {
	  for (int i = 0, idxPlusOne = 0; i < numOfAAs; i++, idxPlusOne++)
	  {

		char desc[ONE_LINE_BUFFER_LEN];
		sprintf(desc, "- - - Test %d - - -", i + 1);

		CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, TEST_ERROR, MAJOR_ERROR_LEVEL,
			desc, TRUE
		);

		SystemSleep(50);
	  }
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalTail(CTerminalCommand& command,
                                        std::ostream& answer)
{
  int numOfMessages = 20;
  DWORD numOfParams = command.GetNumOfParams();
  if (1 == numOfParams)
  {
    const string &strNum = command.GetToken(eCmdParam1);
    bool isNumeric = CObjString::IsNumeric(strNum.c_str());
    if (false == isNumeric)
    {
      answer << "error: Parameter must be numeric, not " << strNum.c_str()
             << '\n'
             << "usage: Bin/McuCmd tail {destination} [Number]\n";
      return STATUS_FAIL;
    }
    numOfMessages = atoi(strNum.c_str());
  }
  CProcessBase::GetProcess()->DumpTasksOpcodeTail(answer, numOfMessages);
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalAddAA(CTerminalCommand& command,
                                         std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (2 != numOfParams)
  {
    answer << "error: Number and isExternal must be specified\n"
           << "usage: Bin/McuCmd add_aa [destination] [Number of AA] [isExternal: YES/NO]\n";
    return STATUS_FAIL;
  }

  const string &strNum = command.GetToken(eCmdParam1), &isExternal =
      command.GetToken(eCmdParam2);

  bool isNumeric = CObjString::IsNumeric(strNum.c_str());
  if (false == isNumeric)
  {
    answer << "error: Parameter must be numeric, not " << strNum.c_str()
           << '\n'
           << "usage: Bin/McuCmd add_aa [destination] [Number of AA] [isExternal: YES/NO]\n";
    return STATUS_FAIL;
  }

  int numOfAAs = atoi(strNum.c_str());

  if (numOfAAs>0)
  {
	  for (int i = 0; i < numOfAAs; i++)
	  {
		char errStr[ONE_LINE_BUFFER_LEN];
		sprintf(errStr, "- - -Error in Test %d - - -", i + 1);

		static DWORD userId = 0;
		userId++;

		AddActiveAlarm(FAULT_GENERAL_SUBJECT, TEST_ERROR, MAJOR_ERROR_LEVEL,
			errStr, (isExternal == "YES"), // true/false
			(isExternal == "YES"), // true/false
			userId);

		SystemSleep(50);
	  }
  }

  return STATUS_OK;
}

// simulate specific active alaram - was added  for SNMP testing
STATUS CManagerTask::HandleTerminalSimulateAddAA(CTerminalCommand& command,
                                         std::ostream& answer)
{
	// ca simulate_add_aa McuMngr 6 5013 0xFFFFFFFF MAJOR NO
	// ca simulate_add_aa McuMngr 6 5013 0xFFFFFFFF MAJOR NO YES NO
  DWORD numOfParams = command.GetNumOfParams();
  if (2 > numOfParams)
  {
    answer
           << "usage: Bin/McuCmd simulate_add_aa [AA Category] [AA Error Code] [optional: AA User ID] [optional: level STARTUP/MAJOR/SYSTEM. default: major] [optional: Is Singleton AA (YES/NO. default:NO)] \n\n"
    	<< "Categories\n"
    	<<	"FAULT_STARTUP_SUBJECT = 0, \t	FAULT_ASSERT_SUBJECT = 1, \t FAULT_FILE_SUBJECT = 2, \n FAULT_CARD_SUBJECT = 3, \t "
    	<< " FAULT_RESERVATION_SUBJECT = 4, \t  FAULT_CONFERENCE_SUBJECT = 5, \t FAULT_GENERAL_SUBJECT = 6, \t "
    	<< " FAULT_EXCEPTION_SUBJECT = 7, \t FAULT_MEETING_ROOM_SUBJECT = 8, \t	FAULT_DONGLE_SUBJECT = 9, \t FAULT_UNIT_SUBJECT = 10, \t "
    	<< " FAULT_MPL_SUBJECT = 11, \t"
    	<< "\n";
    return STATUS_FAIL;
  }



  const string &aaCategoryStr = command.GetToken(eCmdParam1);
  BYTE aaCategory  = atoi(aaCategoryStr.c_str());

  const string &aaErrorCodeStr = command.GetToken(eCmdParam2);
  DWORD aaErrorCode  = atoi(aaErrorCodeStr.c_str());

  string description = "Adding AA error code " + aaErrorCodeStr;
  DWORD aaUserId = 0xFFFFFFFF;
  if (numOfParams  > 2)
  {
	  const string &aaUserIdStr = command.GetToken(eCmdParam3);
	  if ("0xFFFFFFFF" !=  aaUserIdStr)
	  {
		  aaUserId = atoi(aaUserIdStr.c_str());
		  description = description + " User id " + aaUserIdStr;
	  }
  }

  description += ", ";
  description += GetAlarmDescription(aaErrorCode);

  BYTE errorLevel = MAJOR_ERROR_LEVEL;
  if (numOfParams  > 3)
  {
	  const string &errorLevelStr = command.GetToken(eCmdParam4);
	  if (errorLevelStr == "STARTUP")
	  {
		  errorLevel =STARTUP_ERROR_LEVEL ;
	  } else if (errorLevelStr == "MAJOR")
	  {
		  errorLevel = MAJOR_ERROR_LEVEL;
	  } else if (errorLevelStr == "SYSTEM")
	  {
		  errorLevel = SYSTEM_MESSAGE;
	  }
	  else
	  {
		 answer <<  errorLevelStr << ": Invalid error level\n";
		 return STATUS_FAIL;
	  }

  }

  bool isSingleton = false;
  if (numOfParams  > 4)
  {
	  const string &isSingletonStr = command.GetToken(eCmdParam5);
	  bool isSingleton = (isSingletonStr == "YES") ? true :false;
  }

//  DWORD boardId 	= 0;
//  DWORD unitId 	= 0;
 // WORD  theType 	= 0;

  bool isForFaults = true;
  bool isForEma = true;

  // These paramters are not in usage command line.

  if (numOfParams  > 5)
   {
 	  const string &isForEmaStr = command.GetToken(eCmdParam6);
 	 isForEma = (isForEmaStr == "YES") ? true :false;
   }

  if (numOfParams  > 6)
  {
	  const string &isForFaultsStr = command.GetToken(eCmdParam7);
	  isForFaults = (isForFaultsStr == "YES") ? true :false;
  }

	TRACESTR(eLevelInfoNormal) << "Simulate Active Alarm aaCategory " <<  (int)aaCategory << " aaErrorCode " << aaErrorCode
			<<  " errorLevel " << (int)errorLevel <<  " description " << description << " isForEma " << (int)isForEma
			<< " isForFaults " << (int)isForFaults << " aaUserId " << aaUserId;


  if (isSingleton)
  {
	  AddActiveAlarmSingleton(aaCategory, aaErrorCode, errorLevel,
			  description, isForEma, // true/false
			  isForFaults, // true/false
			  aaUserId);
  }
  else {
	  AddActiveAlarm(aaCategory, aaErrorCode, errorLevel,
				  description, isForEma, // true/false
				  isForFaults, // true/false
				  aaUserId);
  }
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalSimulateRemoveAA(CTerminalCommand& command,
                                         std::ostream& answer)
{
	  DWORD numOfParams = command.GetNumOfParams();
	  if (1 > numOfParams)
	  {
		answer  << "usage: Bin/McuCmd simulate_rm_aa [AA Error Code] [optional: AA User ID]\n";
		return STATUS_FAIL;
	  }
	  const string &aaErrorCodeStr = command.GetToken(eCmdParam1);
	  DWORD aaErrorCode  = atoi(aaErrorCodeStr.c_str());
	  if (numOfParams  == 1)
	  {
		  TRACESTR(eLevelInfoNormal) << "remove aa by error code " << aaErrorCode << "\n";
		  RemoveActiveAlarmByErrorCode(aaErrorCode);
	  }
	  else
	  {

		  const string &aaUserIdStr = command.GetToken(eCmdParam2);
		  DWORD aaUserId = atoi(aaUserIdStr.c_str());
		  TRACESTR(eLevelInfoNormal) << "remove aa by error code " << aaErrorCode << " user " << aaUserId << "\n";
		  RemoveActiveAlarmByErrorCodeUserId(aaErrorCode, aaUserId);
	  }
	  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalAddAAnoFlush(CTerminalCommand& command,
                                                std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (2 != numOfParams)
  {
    answer << "error: Number and isExternal must be specified\n"
           << "usage: Bin/McuCmd add_aa [destination] [Number of AA] [isExternal: YES/NO]\n";
    return STATUS_FAIL;
  }

  const string& strNum = command.GetToken(eCmdParam1);
  const string& isExternal = command.GetToken(eCmdParam2);

  bool isNumeric = CObjString::IsNumeric(strNum.c_str());
  if (false == isNumeric)
  {
    answer << "error: Parameter must be numeric, not " << strNum.c_str()
           << '\n'
           << "usage: Bin/McuCmd add_aa [destination] [Number of AA] [isExternal: YES/NO]\n";
    return STATUS_FAIL;
  }

  int numOfAAs = atoi(strNum.c_str());

  if (numOfAAs > 0)
  {
	  for (int i = 0; i < numOfAAs; i++)
	  {
		char errStr[ONE_LINE_BUFFER_LEN];
		sprintf(errStr, "- - -Error in Test %d - - -", i + 1);

		static DWORD userId = 0;
		userId++;

		AddActiveAlarmNoFlush(FAULT_GENERAL_SUBJECT,
							  TEST_ERROR,
							  MAJOR_ERROR_LEVEL,
							  errStr,
							  (isExternal == "YES"), // true/false
							  (isExternal == "YES"), // true/false
							  userId);

		SystemSleep(50);
	  }
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalAAFlush(CTerminalCommand& command,
                                           std::ostream& answer)
{
  FlushActiveAlarm();
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalRemoveAA(CTerminalCommand& command,
                                            std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: Number and isExternal must be specified\n";
    answer << "usage: Bin/McuCmd rm_aa [destination] [Number of AA]\n";
    return STATUS_FAIL;
  }

  const string &strNum = command.GetToken(eCmdParam1);

  bool isNumeric = CObjString::IsNumeric(strNum.c_str());
  if (false == isNumeric)
  {
    answer << "error: Parameter must be numeric, not " << strNum.c_str()
           << '\n'
           << "usage: Bin/McuCmd rm_aa [destination] [Number of AA]\n";
    return STATUS_FAIL;
  }

  int numOfAAs = atoi(strNum.c_str());

  if (numOfAAs > 0)
  {
	  for (int i = 0; i < numOfAAs; i++)
	  {
		RemoveActiveAlarmByErrorCode(TEST_ERROR);

		SystemSleep(50);
	  }
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalSetAA(CTerminalCommand& command,
                                         std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: enable/disable must be specified\n";
    answer << "usage: Bin/McuCmd set_aa [YES/NO]\n";
    return STATUS_FAIL;
  }

  BOOL isEnable = FALSE;
  const string &strNum = command.GetToken(eCmdParam1);
  if (strNum == "YES")
  {
    isEnable = TRUE;
  }
  else if (strNum == "NO")
  {
  }
  else
  {
    answer << "error: enable/disable must be specified\n";
    answer << "usage: Bin/McuCmd enable_aa [YES/NO]\n";
    return STATUS_FAIL;
  }

  CSegment *pSeg = new CSegment;
  *pSeg << isEnable;

  CProcessBase *process = CProcessBase::GetProcess();

  // 1) disable/enable startup conditions.
  SetIsStartupConditionTreeEnabled(isEnable == TRUE);

  // 2) disable/enable addition of AA
  STATUS status = process->SendMessageToAlarmTasks(SET_ENABLE_DISABLE_AA, pSeg);

  if (FALSE == isEnable)
  {
    // 3) remove existing AA
    STATUS status = process->SendMessageToAlarmTasks(REMOVE_ALL_AA, NULL);
  }

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalRmAA(CTerminalCommand& command,
                                        std::ostream& answer)
{
  CProcessBase *process = CProcessBase::GetProcess();
  STATUS status = process->SendMessageToAlarmTasks(REMOVE_ALL_AA, NULL);

  const string &strStatus = process->GetStatusAsString(status);
  answer << "Sent Status : " << strStatus.c_str();

  return status;
}
STATUS CManagerTask::HandleTerminalAddAAFaultOnly(CTerminalCommand& command,
		std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if (1 > numOfParams)
	{
		answer
				<< "usage: Bin/McuCmd TBD [Description number] [userId (optional)] \n";
		return STATUS_FAIL;
	}
	DWORD userId = 0xFFFFFFFF;

	const string &descriptionNumStr = command.GetToken(eCmdParam1);
	int descriptionNum = atoi(descriptionNumStr.c_str());
	char errStr[ONE_LINE_BUFFER_LEN];
	sprintf(errStr, "- - -Error in Test %d - - -", descriptionNum);

	if (1 < numOfParams)
	{
		const string &userIdStr = command.GetToken(eCmdParam2);
		userId = (DWORD) atoi(userIdStr.c_str());
	}
	TRACESTR(eLevelInfoNormal) << "HandleTerminalAddAAFaultOnly errStr  " << errStr
			<< " user Id " << userId;

	DWORD faultID = AddActiveAlarmFaultOnly(FAULT_GENERAL_SUBJECT, TEST_ERROR,
			MAJOR_ERROR_LEVEL, errStr, userId);
	SystemSleep(50);

	return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalUpdateAAFaultOnly(CTerminalCommand& command,
		std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	if (1 > numOfParams)
	{
		answer << "usage: Bin/McuCmd update_aa_fo [Description number]\n";
		return STATUS_FAIL;
	}

	const string &descriptionNumStr = command.GetToken(eCmdParam1);
	int descriptionNum = atoi(descriptionNumStr.c_str());
	char errStr[ONE_LINE_BUFFER_LEN];
	sprintf(errStr, "- - -Error in Test %d - - -", descriptionNum);

	TRACESTR(eLevelInfoNormal) << "HandleTerminalUpdateAAFaultOnly errStr  "
			<< errStr;

	UpdateActiveAlarmFaultOnlyDescriptionByErrorCode(TEST_ERROR, errStr);

	SystemSleep(50);

	return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalRemoveAAFaultOnly(CTerminalCommand& command,
		std::ostream& answer)
{
	DWORD numOfParams = command.GetNumOfParams();
	DWORD userId = 0xFFFFFFFF;
	if (0 < numOfParams)
	{
		const string &userIdStr = command.GetToken(eCmdParam1);
		userId = (DWORD) atoi(userIdStr.c_str());
	}

	if (userId == 0xFFFFFFFF)
	{
		TRACESTR(eLevelInfoNormal)
				<< "HandleTerminalRemoveAAFaultOnly RemoveActiveAlarmFaultOnlyByErrorCode ";
		RemoveActiveAlarmFaultOnlyByErrorCode(TEST_ERROR);
	} else
	{
		TRACESTR(eLevelInfoNormal)
				<< "HandleTerminalRemoveAAFaultOnly RemoveActiveAlarmFaultOnlyByErrorCodeUserId "
				<< userId;
		RemoveActiveAlarmFaultOnlyByErrorCodeUserId(TEST_ERROR, userId);
	}

	SystemSleep(50);

	return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalEnableDisable(CTerminalCommand& command,
                                                 std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: enable/disable must be specified\n";
    answer << "usage: Bin/McuCmd enable_aa [YES/NO]\n";
    return STATUS_FAIL;
  }

  BOOL isEnable = FALSE;
  const string &strNum = command.GetToken(eCmdParam1);
  if (strNum == "YES")
  {
    isEnable = TRUE;
  }
  else if (strNum == "NO")
  {
  }
  else
  {
    answer << "error: enable/disable must be specified\n";
    answer << "usage: Bin/McuCmd enable_aa [YES/NO]\n";
    return STATUS_FAIL;
  }

  CSegment *pSeg = new CSegment;
  *pSeg << isEnable;

  CProcessBase *process = CProcessBase::GetProcess();
  STATUS status = process->SendMessageToAlarmTasks(SET_ENABLE_DISABLE_AA, pSeg);

  const string &strStatus = process->GetStatusAsString(status);
  answer << "Sent Status : " << strStatus.c_str();

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalDisableSCTree(CTerminalCommand& command,
                                                 std::ostream& answer)
{
  SetIsStartupConditionTreeEnabled(false);
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalWatchDogCnt(CTerminalCommand& command,
                                               std::ostream& answer)
{
  CProcessBase *process = CProcessBase::GetProcess();
  DWORD cnt = process->GetWatchDogCnt();
  answer << "Watch Dog Cnt : " << cnt;
  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalDeadlock(CTerminalCommand& command,
                                            std::ostream& answer)
{
  PASSERTMSG(1,"TerminalDeadlock - this process will be locked for debugging deadlocks");
  while (1)
    ;

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalAllProcessInfo(CTerminalCommand& command,
                                                  std::ostream& answer)
{
  STATUS res1 = HandleTerminalGenericProcessInfo(command, answer);
  STATUS res2 = HandleTerminalSpecificProcessInfo(command, answer);
  if (res1 == STATUS_OK && res2 == STATUS_OK)
    return STATUS_OK;

  return STATUS_FAIL;
}

STATUS CManagerTask::HandleTerminalGenericProcessInfo(CTerminalCommand& command,
                                                      std::ostream& answer)
{
  HandleTerminalDisplayTasks(command, answer);
  HandleTerminalConfig(command, answer);
  HandleTerminalState(command, answer);
  HandleTerminalProcessStatistics(command, answer);
  HandleTerminalActiveAlarm(command, answer);
  HandleTerminalDisplayQueue(command, answer);
  HandleTerminalDisplayTimer(command, answer);
  HandleTerminalTail(command, answer);
  HandleTerminalGetProductType(command, answer);
  HandleTerminalGetAA(command, answer);

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalSpecificProcessInfo(CTerminalCommand& command,
                                                       std::ostream& answer)
{
  return SpecificProcessInfo(answer);
}

STATUS CManagerTask::HandleTerminalProcessMemoryUtilization(CTerminalCommand& command,
                                                            std::ostream& answer)
{
  CProcessBase *pProcess = CProcessBase::GetProcess();
  int MaxAvailableMemory = (pProcess->GetProcessAddressSpace());
  int UsedMemory = GetUsedMemory(TRUE);
  if (MaxAvailableMemory != 0)
  {
    //Used memory is returned in Kilobytes , and MaxAvailableMemory is in bytes.
    //Multiplied by 100 to get percentage.
    int used_precent =
        (int) ((double) 100 * (UsedMemory * 1024) / MaxAvailableMemory);

    answer << "Limit: " << MaxAvailableMemory
           << " Usage: " << UsedMemory * 1024
           << " used: " << used_precent << "%" << std::endl;

  }
  else
  {
    answer << "Limit: unlimited"
           << " Usage: " << UsedMemory * 1024 << std::endl;

  }

  return STATUS_OK;
}

STATUS CManagerTask::SpecificProcessInfo(std::ostream& answer)
{
  return STATUS_OK;
}

BYTE operator <(const STerminalCommand& l, const STerminalCommand& r)
{
  return (strcmp(l.m_command, r.m_command));
}

BYTE operator ==(const STerminalCommand& l, const STerminalCommand& r)
{
  return (strcmp(l.m_command, r.m_command) == 0);
}

void CManagerTask::CreateRegisterAlarmableTask(CTaskApi *taskApi,
                                               void(*entryPoint)(void*),
                                               const COsQueue *creatorRcvMbx)
{
  CreateTask(taskApi, entryPoint, creatorRcvMbx);
  CTaskApp *taskApp = taskApi->GetTaskAppPtr();

  const CAlarmableTask *alarmTask =
      dynamic_cast<const CAlarmableTask*> (taskApp);
  RegisterAlarmableTask(alarmTask);
}

void CManagerTask::CreateTask(CTaskApi *taskApi,
                              void(*entryPoint)(void*),
                              const COsQueue *creatorRcvMbx)
{
  taskApi->Create(entryPoint, *creatorRcvMbx);
}

void CManagerTask::RegisterAlarmableTask(const CAlarmableTask *taskApp)
{
  m_TaskFaultContainer->AddNewTask(taskApp->GetTaskName());
}

void CManagerTask::ManagerInitActionsPointBase()
{
  CSysConfig *sysConfig = new CSysConfig;
  CProcessBase *process = CProcessBase::GetProcess();

	if ((!sysConfig->IsReady() || sysConfig->IsActiveAlarmErrorExist())
			&& eProcessMcuMngr == process->GetProcessType())
  {
    TRACESTR(eLevelInfoNormal) << "Error exists, alarm will be sent\n";
    AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            CFG_INVALID,
                            MAJOR_ERROR_LEVEL,
                            sysConfig->GetActiveAlarmErrorMsg(),
                            true, true);
  }

  if (sysConfig->IsFaultErrorExist() &&
      eProcessMcuMngr == process->GetProcessType())
  {
    TRACESTR(eLevelInfoNormal) << "Error exists, fault will be sent\n";
    BOOL isFullOnly = FALSE;
    CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                        CFG_INVALID,
                        SYSTEM_MESSAGE,
                        sysConfig->GetFaultErrorMsg(),
                        isFullOnly);
  }

  process->SetSysConfig(sysConfig);

  // Updates log level by system flag
  std::string level;
  BOOL res = sysConfig->GetDataByKey(CFG_KEY_LOGGER_MAX_TRACE_LEVEL, level);
  PASSERTSTREAM_AND_RETURN(FALSE == res,
      "GetDataByKey: Unable to find " << CFG_KEY_LOGGER_MAX_TRACE_LEVEL);
  CSegment msg;
  msg << CTrace::GetTraceLevelByName(level.c_str());

  // Direct call to update the level
  OnLoggerAllMaxTraceLevel(&msg);

  // ===== set max startup time
  DWORD startupTime;
  sysConfig->GetDWORDDataByKey("MAX_STARTUP_TIME", startupTime);
  process->SetMaxTimeForStartup(startupTime);



  ManagerInitActionsPoint();
}

void CManagerTask::ManagerStartupActionsPointBase()
{
  ManagerStartupActionsPoint();

  bool result = IsThereStartupCondition();
  if (false == result)
  {
    SetTaskState(eTaskStateReady);
    ComputeSetTaskStatus();
  }
}

void CManagerTask::OnStartupEvent(CSegment * seg)
{
  PTRACE(eLevelInfoNormal, "Start startup");

  StartStartupTimer();

  ManagerStartupActionsPointBase();
}

void CManagerTask::OnProcessInfo(CSegment * seg)
{
  CProcessBase * proc = CProcessBase::GetProcess();
  std::string FileName = MCU_TMP_DIR+"/Collect/";
  FileName += proc->GetProcessName(proc->GetProcessType());
  std::ofstream out(FileName.c_str());
  CTerminalCommand tmp_cmd;
  HandleTerminalAllProcessInfo(tmp_cmd, out);
  out.close();
}

void CManagerTask::AlarmableTaskCreateEndPoint()
{
  // base: set status to normal.
}

void CManagerTask::StartIdleTimer()
{
  SystemGetTime(m_IdleStartTime);

  DWORD timeToEndIdle = CProcessBase::GetProcess()->GetMaxTimeForIdle();
  StartTimer(PROCESS_IDLE_TIMER, timeToEndIdle);
}

void CManagerTask::StartStartupTimer()
{
  DWORD timeToEndStartup = CProcessBase::GetProcess()->GetMaxTimeForStartup();
  StartTimer(MANAGER_STARTUP_TIMER, timeToEndStartup);
}

void CManagerTask::HandleIdleTimerEnd(CSegment* seg)
{
  eProcessStatus processState = CProcessBase::GetProcess()->GetProcessStatus();
  if (eProcessIdle == processState)
  {
    CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                        IDLE_DEADLINE_REACHED,
                        MAJOR_ERROR_LEVEL,
                        "Process startup exceeded allowed time.",
                         false);

    PrintIdleTimes();
  }
}

void CManagerTask::PrintIdleTimes()
{
  CStructTm curTime;
  SystemGetTime(curTime);

  COstrStream idleStartTimeStr, idleTimeoutStr;
  m_IdleStartTime.Serialize(idleStartTimeStr);
  curTime.Serialize(idleTimeoutStr);

  string idleStr = "\nCManagerTask::PrintIdleTimes";
  idleStr += "\nIdle started at ";
  idleStr += idleStartTimeStr.str();
  idleStr += "\nIdle timeout at ";
  idleStr += idleTimeoutStr.str();

  TRACESTR(eLevelInfoNormal) << idleStr.c_str();
}

void CManagerTask::HandleStartupTimerEnd(CSegment * seg)
{
  SetTaskState(eTaskStateReady);
  ConvertStartupConditionToActiveAlarm();
}

void CManagerTask::OnManagerAlive(CSegment * seg)
{}

void CManagerTask::OnTimerCheckMemoryExhaustion(CSegment* pMsg)
{
  CProcessBase* proc = CProcessBase::GetProcess();

  unsigned long long max = proc->GetProcessAddressSpace();
  unsigned long long used = GetUsedMemory(m_toPrint);
  unsigned long long percent = used * 1024 * 100 / max;

  std::stringstream msg;
  msg << "Process is approaching memory utilization limit: " << percent << " %";

  // More than 80 percent of allocated memory is used
  if (percent > 80)
  {
    m_IsMemoryExhausted = TRUE;

		bool isAAExist = IsActiveAlarmFaultOnlyExistByErrorCode(LOW_PROCESS_MEMORY_ALERT);
		if (isAAExist)
		{
			if (abs((int) (m_lastReportedMemoryPercent - percent)) > 5)
			{
				UpdateActiveAlarmFaultOnlyDescriptionByErrorCode(LOW_PROCESS_MEMORY_ALERT, msg.str());
				m_lastReportedMemoryPercent = percent;
			}
			else
			{
				TRACESTR(eLevelInfoNormal) << " Memory percent changed to "
                               << percent;
			}

		} else
    {
      if (selfKillOnMemoryExhaustion())
      {
        CMcmsDaemonApi api;
        api.SendResetProcessReq(proc->GetProcessType());
      }
      else
      {
				AddActiveAlarmFaultOnly(FAULT_GENERAL_SUBJECT,
						LOW_PROCESS_MEMORY_ALERT, MAJOR_ERROR_LEVEL, msg.str());
				m_lastReportedMemoryPercent = percent;
				static bool dumpCoreOnce = false;
				if (!dumpCoreOnce)
				{
					dumpCoreOnce = true;
					SystemCoreDump(TRUE);
				}
			}
		}
	} else if (TRUE == m_IsMemoryExhausted && percent < 70) //remove active alarm
	{
		proc->RemoveActiveAlarmFaultOnlyFromProcess(LOW_PROCESS_MEMORY_ALERT);
		m_lastReportedMemoryPercent = 0;
    m_IsMemoryExhausted = FALSE;
  }

  StartTimer(PROCESS_MEMORY_USAGE_TIMER, 60 * SECOND);

	TRACEINTOFUNC << msg.str() << ", " << max << "/" << used << "=" << percent
                << ", " << proc->GetOpcodeAsString(PROCESS_MEMORY_USAGE_TIMER)
                << " will fire in 60 seconds";

}

void CManagerTask::OnTimerCheckCpuUsage(CSegment*)
{
  CProcessBase *pProcess = CProcessBase::GetProcess();
  TICKS selfUser, selfSystem, childUser, childSystem;
  TICKS total;
  GetSelfCpuUsage(selfUser, selfSystem);
  GetChildrenCpuUsage(childUser, childSystem);
  total = selfUser + selfSystem + childUser + childSystem;
  TICKS delta = total - m_lastCpuUsageRead;

  int usage_precents = delta.GetIntegerPartForTrace() / 300;

  if (m_isCpuAlarm)
  {
    if (usage_precents < 33)
    {
			pProcess->RemoveActiveAlarmFaultOnlyFromProcess(
					HIGH_CPU_USAGE_PROCESS_ALERT);
      m_isCpuAlarm = FALSE;
    }
    else
    {
      COstrStream message;
      message << "Process CPU usage is high: " << usage_precents << "%";

			UpdateActiveAlarmFaultOnlyDescriptionByErrorCode(
					HIGH_CPU_USAGE_PROCESS_ALERT, message.str());
		}
  }
  else
  {
    int maxLegitimateUsagePrecents = GetMaxLegitimateUsagePrecents(); // 35 for most processes (50 for Logger) - fix for VNGR-16803
    if ((maxLegitimateUsagePrecents > 35) && (usage_precents > 35)) // trace to notify usage_precents > 35, in case we don't add AA (Logger)
    {
      TRACEINTOFUNC << "Process CPU usage is high: "
                    << " - Process CPU usage is high: " << usage_precents << "%"
                    << ", maxLegitimateUsagePrecents for this process is "
                    << maxLegitimateUsagePrecents << "%";
    }

    if (usage_precents > maxLegitimateUsagePrecents)
    {
      COstrStream message;
      message << "Process CPU usage is high: " << usage_precents << "%";

			AddActiveAlarmFaultOnly(FAULT_GENERAL_SUBJECT,
					HIGH_CPU_USAGE_PROCESS_ALERT, MAJOR_ERROR_LEVEL,
					message.str());

			m_isCpuAlarm = TRUE;
    }
  }

  m_lastCpuUsageRead = total;
  StartTimer(PROCESS_CPU_USAGE_TIMER, 5 * 60 * SECOND);
}

void CManagerTask::OnTimerIsStartupFinished(CSegment*)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);
  OPCODE opcode;
  eMcuState state;
  unsigned int timeout;

  // Can not define finish of startup for root processes
  if (proc->GivesAwayRootUser())
  {
    // Runs Process CPU usage verification after startup for non root processes
    state = proc->GetSystemState();
    if (state != eMcuState_Invalid && state != eMcuState_Startup)
    {
      // Startup is finished
      timeout = 2 * SECOND;
      opcode = PROCESS_CPU_USAGE_TIMER;
    }
    else
    {
      // Continues polling
      timeout = 30 * SECOND;
      opcode = IS_STARTUP_FINISHED_TIMER;
    }
  }
  else
  {
    // Gives 7 minutes for a startup for root processes
    state = eMcuState_Invalid;
    timeout = 7 * 60 * SECOND;
    opcode = PROCESS_CPU_USAGE_TIMER;
  }

  StartTimer(opcode, timeout);
  std::stringstream msg;

  msg << "MCU state " << GetMcuStateName(state)
      << ", " << proc->GetOpcodeAsString(opcode);

  // Also runs memory usage together with CPU usage
  if (PROCESS_CPU_USAGE_TIMER == opcode &&
      proc->GetProcessAddressSpace() > 0 &&
      !proc->IsUnderValgrid())
  {
    StartTimer(PROCESS_MEMORY_USAGE_TIMER, timeout);
    msg << " and "
        << proc->GetOpcodeAsString(PROCESS_MEMORY_USAGE_TIMER);
  }

  msg << " will fire in " << timeout / SECOND << " seconds";

  TRACEINTOFUNC << msg.str();
}

// Static
STATUS CManagerTask::GetCPUUsageThreshold(unsigned int threshold,
                                          unsigned int& upper,
                                          unsigned int& lower)
{
  // Assumes threshold gap 5% of the threshold
  const unsigned int gap = threshold / 20;

  FPASSERTSTREAM_AND_RETURN_VALUE(threshold <= gap,
      "Threshold " << threshold << " is smaller then gap " << gap,
      STATUS_FAIL);

  CProcessBase* proc = CProcessBase::GetProcess();
  FPASSERT_AND_RETURN_VALUE(NULL == proc, STATUS_FAIL);

  size_t cpu_num;
  // Load average at multi-core of RMX1500 doesn't work the same
  // at dual-core of RMX4000. The calculation should be reconsidered.
  if (eProductTypeRMX1500 == proc->GetProductType())
  {
    cpu_num = 1;
    FTRACEWARN << "Assumes that RMX1500 has " << cpu_num << " CPU";
  }
  else
  {
    std::string ans;
    STATUS stat = CConfigManagerApi().GetCPUType(ans);
    if (STATUS_OK == stat)
    {
      cpu_num = std::count(ans.begin(), ans.end(), '\n');
      if (cpu_num < 1)
      {
        cpu_num = 1;
        FTRACEWARN << "Wrong CPU number in " << ans << ", assume " << cpu_num;
      }
    }
    else
    {
      cpu_num = 1;
      FTRACEWARN << "Unable to get CPU number, assume " << cpu_num;
    }
  }

  upper = threshold * cpu_num;
  lower = (threshold - gap) * cpu_num;

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalLeakCheck(CTerminalCommand& command,
                                             std::ostream& answer)
{
  VALGRIND_DO_LEAK_CHECK

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalSleepTest(CTerminalCommand& command,
                                             std::ostream& answer)
{
  const string &seconds = command.GetToken(eCmdParam1);

  int numOfSeconds = atoi(seconds.c_str());
  SystemSleep(numOfSeconds * SECOND, false);

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalGetProductType(CTerminalCommand & command,
                                                  std::ostream& answer)
{
  CProcessBase *pProcess = CProcessBase::GetProcess();
  eProductType pt = pProcess->GetProductType();
  eProductFamily pf = pProcess->GetProductFamily();
  answer << "Product type : " << ProductTypeToString(pt) << " (" << (DWORD) pt
         << ") ";

  answer << "Product family : " << (DWORD) pf << std::endl;

  return STATUS_OK;
}

void CManagerTask::OnAssertInd(CSegment* pMsg)
{
  OnAssertTreatment();

  CProcessBase *pProcess = CProcessBase::GetProcess();

  pProcess->m_IsTreatingOnAssert--;
}

void CManagerTask::OnInformHttpGetFile(CSegment* pMsg)
{
  std::string file_name;
  *pMsg >> file_name;

  InformHttpGetFile(file_name);
}

void CManagerTask::InformHttpGetFile(const std::string & file_name)
{
  std::string error =
      "This function must be implemented in derived class. file name:"
          + file_name;
  PASSERTMSG(100, error.c_str());
}

void CManagerTask::OnRestartStartupTimer(CSegment* pMsg)
{
  DWORD newStartupTimeout;
  *pMsg >> newStartupTimeout;

  if (eTaskStateStartup == GetTaskState())
  {
    eProcessType curProcess = CProcessBase::GetProcess()->GetProcessType();

    // ===== restart the timer
    CProcessBase::GetProcess()->SetMaxTimeForStartup(newStartupTimeout);

    DeleteTimer(MANAGER_STARTUP_TIMER);
    StartTimer(MANAGER_STARTUP_TIMER, newStartupTimeout);

    int newStartupTimeoutInSeconds = newStartupTimeout / SECOND; // 'newStartupTimeout' is in ticks
    TRACESTR(eLevelInfoNormal) << "\nCManagerTask::OnRestartStartupTimer - "
        << ::ProcessTypeToString(curProcess)
        << "\nStartup timeout was restarted to " << newStartupTimeout
        << " ticks (" << newStartupTimeoutInSeconds << " seconds)";
  }
}

void CManagerTask::OnFailoverSetParamsInd(CSegment* pMsg)
{
  WORD isFeatureEnabled = false, isSlave = false;

  *pMsg >> isFeatureEnabled >> isSlave;

  bool bIsEnabled = (bool) isFeatureEnabled, bIsSlave = (bool) isSlave;

  TRACESTR(eLevelInfoNormal) << "\nCManagerTask::OnFailoverSetParamsInd"
      << "\nIsFeatureEnabled: " << (bIsEnabled ? "yes" : "no")
      << "\nIsSlave:          " << (bIsSlave ? "yes" : "no");

  CProcessBase *curProcess = CProcessBase::GetProcess();
  curProcess->SetFailoverParams(bIsEnabled, bIsSlave);
}

void CManagerTask::OnLoggerAllMaxTraceLevel(CSegment* msg)
{
  unsigned int level;
  *msg >> level;

  CFilterByLevel flt(CTrace::GetTraceLevelShortNameByValue(level));
  PASSERTSTREAM_AND_RETURN(!flt.IsValid(),
      "Illegal trace level " << CTrace::GetTraceLevelNameByValue(level)
      << " (" << level << ")");

  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CFilterTraceContainer* container = proc->GetTraceFilterContainer();
  PASSERT_AND_RETURN(NULL == container);

  const CFilterTrace* flt_old =
  container->GetFilterByName(CFilterByLevel::GetCompileType());

  // Filter exists
  if (NULL != flt_old)
  {
    // The function is called only if the level is changed
    PASSERT_AND_RETURN(*flt_old == flt);
  }

  // Updates the filter with the new value
  container->AddFilter(flt);
}

STATUS CManagerTask::HandleTerminalLogStressStart(CTerminalCommand& cmd,
                                                  std::ostream& ans)
{
  if (IsValidTimer(LOGGER_BUSY_TIMER))
  {
    ans << "The process is started already";
    return STATUS_OK;
  }

  s_stress_log_counter = 0l;

  StartTimer(LOGGER_BUSY_TIMER, 1); // 0.01 sec

  ans << "Start producing 30 messages every 0.01 sec";

  return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalLogStressStop(CTerminalCommand& cmd,
                                                 std::ostream& ans)
{
  if (!IsValidTimer(LOGGER_BUSY_TIMER))
  {
    ans << "The process is not started yet";
    return STATUS_OK;
  }

  DeleteTimer(LOGGER_BUSY_TIMER);

  ans << "Stop producing 30 messages every 0.01 sec, produced: "
      << s_stress_log_counter << " messages";

  return STATUS_OK;
}

void CManagerTask::OnTimerLoggerBusyTimeout(void)
{
  static CProcessBase* proc = CProcessBase::GetProcess();
  static const char* pname = proc->GetProcessName(proc->GetProcessType());

  if (s_stress_log_counter >= 99999999)
    s_stress_log_counter = 0;

  for (int i = 0; i < 30; i++)
  {
    TRACEINTOFUNC << pname << " log #" << std::setw(8)
                  << ++s_stress_log_counter;
  }

  StartTimer(LOGGER_BUSY_TIMER, 1);  // 0.01 sec
}

void CManagerTask::OnTimerDumpStatistics()
{
	CTerminalCommand command;
	std::stringstream answer;
	CProcessBase::GetProcess()->DumpStatistics(answer, command);
	TRACEINTOFUNC << answer.str();

	StartTimer(PROCESSBASE_DUMP_STATISTICS_TIMER, m_iDumpStatisticsInterval * SECOND);
}

void CManagerTask::StartStatisticsTimer()
{
	  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CSysConfig* cfg = proc->GetSysConfig();
  PASSERT_AND_RETURN(NULL == cfg);

  BOOL res = cfg->GetDWORDDataByKey(PROCESSBASE_DUMP_STATISTICS_TIMER_INTERVAL,
                                    m_iDumpStatisticsInterval);
  PASSERTSTREAM_AND_RETURN(!res,
    "Unable to read " << PROCESSBASE_DUMP_STATISTICS_TIMER_INTERVAL);

  //Zero value means this feature is not to be working
  if (0 == m_iDumpStatisticsInterval)
    return;

	StartTimer(PROCESSBASE_DUMP_STATISTICS_TIMER, m_iDumpStatisticsInterval * SECOND);
}

STATUS CManagerTask::HandleTerminalPeriodicStat(CTerminalCommand& command,
                                         std::ostream& answer)
{
    const string &strNum = command.GetToken(eCmdParam1);
    int iDumpStatisticsInterval = atoi(strNum.c_str());
    if (""==strNum || iDumpStatisticsInterval <0)
    {
        answer << "error: Please enter a valid Periodic Timeout greater than Zero\n";
        answer << "usage: Bin/McuCmd periodic_stat [Interval]\n";
        answer << "usage (Disable Periodic Statistics): Bin/McuCmd periodic_stat 0\n";
        return STATUS_FAIL;
    }

    m_iDumpStatisticsInterval = iDumpStatisticsInterval;
    if (0 == m_iDumpStatisticsInterval) //Zero value means this feature is not to be working
        return STATUS_FAIL;

    StartTimer(PROCESSBASE_DUMP_STATISTICS_TIMER, m_iDumpStatisticsInterval * SECOND);

    return STATUS_OK;
}
STATUS CManagerTask::HandleTerminalTraceLocal(CTerminalCommand& cmd, std::ostream& ans)
{
	 CProcessBase* proc = CProcessBase::GetProcess();
	 FPASSERT_AND_RETURN_VALUE(NULL == proc,STATUS_FAIL);
	 const string &strNum = cmd.GetToken(eCmdParam1);
	 if((strNum != "on")&&(strNum != "off"))
	 {
		 ans << "No valid parameters were found. please specify on or off";
		 return STATUS_FAIL;
	 }
	 string processName = proc->GetProcessName(proc->GetProcessType());
	 string fullFilepath =OS_STARTUP_LOGS_PATH + processName +".Ind";
	 if(strNum == "on")
	 {
		 proc->m_enableLocalTracer = TRUE;
		 CreateFile(fullFilepath);
		 ans << "Local tracer file for process " << processName << " was turn on" ;
	 }
	 if(strNum == "off")
	 {
	 	 proc->m_enableLocalTracer = FALSE;
	 	 DeleteFile(fullFilepath,TRUE);
	 	ans << "Local tracer file for process " << processName << " was turn off" ;
	 }

	 return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalResetMonitorStat(CTerminalCommand& cmd, std::ostream& ans)
{
    CProcessBase* proc = CProcessBase::GetProcess();
    FPASSERT_AND_RETURN_VALUE(NULL == proc,STATUS_FAIL);
    proc->ResetMonitoring();
    return STATUS_OK;
}

STATUS CManagerTask::HandleTerminalDumpAADB(CTerminalCommand& cmd, std::ostream& ans)
{
  CAlarmStringConverter::Instance().Dump(ans);
  return STATUS_OK;
}
