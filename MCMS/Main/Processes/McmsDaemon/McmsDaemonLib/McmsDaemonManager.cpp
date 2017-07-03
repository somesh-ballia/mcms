// McmsDaemonManager.cpp

#include "McmsDaemonManager.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ostream>
#include <iomanip>

#include "TraceStream.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "Segment.h"
#include "SystemFunctions.h"
#include "TaskApi.h"
#include "SysConfig.h"
#include "WatchdogIF.h"
#include "HlogApi.h"
#include "FaultsDefines.h"
#include "ManagerApi.h"
#include "IPMCInterfaceApi.h"
#include "ProcessPolicy.h"
#include "InternalProcessStatuses.h"
#include "McmsDaemonProcess.h"
#include "SysConfigKeys.h"
#include "OsFileIF.h"
#include "ResetHistory.h"
#include "StructTm.h"
#include "AuditorApi.h"
#include "StructTm.h"
#include "ConfigManagerOpcodes.h"
#include "CyclicFileList.h"
#include "TerminalCommand.h"
extern void McmsDaemonMonitorEntryPoint(void* appParam);

const char* NEATLY_SHOUTDOWN_PATH = "States/McmsDaemonNeatlyFileInd";

const std::string WATCH_DOG_LOG_DIR = MCU_OUTPUT_DIR+"/core/";
const char* WATCH_DOG_CONF_PARTY_DUMP_PREFIX = "ConfPartyDump";
const char* ENTER_DIAGNOSTICS_IND = "States/EnterDiagnosticsind";

#define MCMS_DAEMON_HW_TIMER      1009
#define MCMS_DAEMON_SW_TIMER      1010
#define MCMS_DAEMON_CG_LICENCE_TIMER  1011  // for Call Generator - expiration time
#define MCMS_DAEMON_CHECK_ZOMBIE_TOUT 1012

#define WD_SW_INTERVAL       3000 // Merged from Amdocs version by demand (VNGFE-3001???)
#define WD_CG_INTERVAL       6000 // for Call Generator - expiration time

// Time interval to start IPMC watchdog. The value is increased from 10 to
// 30 seconds to prevent endless restarts on Restore Factory Default.
#define WD_HW_START 30 * SECOND

// Time interval to continue IPMC watchdog. Used by timer to trigger the
// watchdog.
#define WD_HW_INTERVAL 10 * SECOND
#define ZOMBIE_TOUT 60 * SECOND

#define RESET_LIST_DELETE_TIMER 1020

// Increased from 20 to 40 seconds because of prolonged time of FIPS initialization
#define MAX_TIME_FOR_PROCESS_STARTUP 4000
#define DAEMON_LOADING_LOG "DaemonProcessesLoad.log"

const WORD STARTUP = 1;
const WORD READY = 2;

static CMcmsDaemonProcess* pDaemonProcess = NULL;

PBEGIN_MESSAGE_MAP(CMcmsDaemonManager)
  ONEVENT(WD_KEEP_ALIVE, ANYCASE, CMcmsDaemonManager::OnKeepAlive)
  ONEVENT(MCMS_DAEMON_SW_TIMER, ANYCASE, CMcmsDaemonManager::OnSW_WD_Timer)
  ONEVENT(MCMS_DAEMON_HW_TIMER, ANYCASE, CMcmsDaemonManager::OnHW_WD_Timer)
  ONEVENT(PROCESS_UP, STARTUP, CMcmsDaemonManager::OnProccessUpStartup)
  ONEVENT(PROCESS_UP, READY, CMcmsDaemonManager::OnProccessUpReady)
  ONEVENT(TO_MCMS_DAEMON_RESET_MCMS_REQ, ANYCASE, CMcmsDaemonManager::OnResetMcmsReq)
  ONEVENT(RESET_LIST_DELETE_TIMER, ANYCASE, CMcmsDaemonManager::OnResetHistoryTimer)
  ONEVENT(TO_MCMS_DAEMON_RESET_PROCESS_REQ, ANYCASE, CMcmsDaemonManager::OnResetProcessReq)
  ONEVENT(CONFIG_APACHE_FINISHED_IND, ANYCASE, CMcmsDaemonManager::OnConfigApacheInd)
  ONEVENT(MCMS_DAEMON_CG_LICENCE_TIMER, ANYCASE, CMcmsDaemonManager::OnCG_WD_Timer)
  ONEVENT(INSTALLER_STOP_IPMC_WD, ANYCASE, CMcmsDaemonManager::OnInstallerStopIpmc)
  ONEVENT(MCMS_DAEMON_CHECK_ZOMBIE_TOUT, ANYCASE, CMcmsDaemonManager::OnCheckChildZombieProcessesTimer)
  ONEVENT(CSMNGR_TO_MCMSDAEMON_START_SIGNALING, ANYCASE, CMcmsDaemonManager::OnStartSignaling)
  ONEVENT(MCMSNETWORK_TO_MCMSDAEMON_FINISH, ANYCASE, CMcmsDaemonManager::OnMcmsNetworkFinish)
PEND_MESSAGE_MAP(CMcmsDaemonManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CMcmsDaemonManager)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CMcmsDaemonManager)
  ONCOMMAND("reset", CMcmsDaemonManager::HandleTerminalReset, "reset all box")
  ONCOMMAND("Enter_Diagnostics", CMcmsDaemonManager::HandleEnterDiagnostics, "Enter Diagnostics mode")
  ONCOMMAND("rm_watch_process", CMcmsDaemonManager::HandleRemoveProcess, "Remove watch Process from daemon list")
  ONCOMMAND("add_watch_process", CMcmsDaemonManager::HandleAddProcess, "Add watch Process to daemon list")
  ONCOMMAND("print_watch_process", CMcmsDaemonManager::HandlePrintProcessWatchList, "print watch Process list")
END_TERMINAL_COMMANDS

void McmsDaemonManagerEntryPoint(void* appParam)
{
	CMcmsDaemonManager* pMcmsDaemonManager = new CMcmsDaemonManager;
	pMcmsDaemonManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CMcmsDaemonManager::GetMonitorEntryPoint()
{
	return McmsDaemonMonitorEntryPoint;
}

CMcmsDaemonManager::CMcmsDaemonManager()
{
  m_prevToRun = eProcessMcmsDaemon;
  m_PowerButtonFD = -1;

  pDaemonProcess = dynamic_cast<CMcmsDaemonProcess*>(CProcessBase::GetProcess());
  pDaemonProcess->ResetProcessMonitoringInfo();

  m_state = STARTUP;
  m_apacheMode = eNotInit;
  m_isDuringInstalltion = FALSE;
  m_disable_IPMC_usage = -1;

  for (int i = 0; i < NUM_OF_PROCESS_TYPES; i++)
    m_isProcessRecievedCoreDump[i] = FALSE;


  m_isMcmsNetworkFinishInd = FALSE;
  m_LastCheckedDebugMode = FALSE;
  m_isDebug_Mode = FALSE;
  m_isEnteringInDiagnosticsMode = FALSE;
}

CMcmsDaemonManager::~CMcmsDaemonManager()
{}

void CMcmsDaemonManager::ManagerInitActionsPoint()
{
  std::string cmd, info, answer;

  cmd = "mkdir -p ";
  cmd += OS_STARTUP_LOGS_PATH;
  SystemPipedCommand(cmd.c_str(), answer);

  info = "TICKS: ";
  char ticks[10];
  snprintf(ticks, ARRAYSIZE(ticks), "%d",
           SystemGetTickCount().GetIntegerPartForTrace());
  info += ticks;

  info += "\nCMcmsDaemonManager::InitTask";

  std::string dump = "echo \"" + info + "\" > "+ OS_STARTUP_LOGS_PATH + DAEMON_LOADING_LOG;
  SystemPipedCommand(dump.c_str(), answer);

  dump = "cat /proc/uptime >> ";
  dump += OS_STARTUP_LOGS_PATH;
  dump += DAEMON_LOADING_LOG;
  SystemPipedCommand(dump.c_str(), answer);
}

void CMcmsDaemonManager::ManagerPostInitActionsPoint()
{
  int resetNumber = ResetHistory_GetResetNumber();
  if (MAX_RESET_NUM_BEFORE_SAFE_MODE <= resetNumber)
  {
    EnterSafeMode();
  }
  else
  {
    ResetHistory_AddStartup();
    StartHistoryTimer();
  }

  m_PowerButtonFD =  open("/proc/acpi/event", O_RDONLY);
  m_isDebug_Mode = FALSE;
  m_LastCheckedDebugMode = FALSE;
  CheckDebugMode();
  m_LastCheckedDebugMode = m_isDebug_Mode;
  m_isEnteringInDiagnosticsMode = FALSE;

  CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
  sysConfig->GetBOOLDataByKey("DISABLE_IPMC_USAGE", m_disable_IPMC_usage);


  // for Call Generator - expiration time
  if (CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator)
    StartTimer(MCMS_DAEMON_CG_LICENCE_TIMER, WD_CG_INTERVAL);
}

void CMcmsDaemonManager::StartHistoryTimer()
{
  DWORD time = 0;
  pDaemonProcess->GetSysConfig()->GetDWORDDataByKey("RESET_HISTORY_TIME_INTERVAL", time);
  StartTimer(RESET_LIST_DELETE_TIMER, time);
}

void CMcmsDaemonManager::ManagerPreTerminalDeathPoint()
{
  ResetHistory_Remove();
}

void CMcmsDaemonManager::EnterSafeMode()
{
  pDaemonProcess->SetIsSafeMode(TRUE);
  ResetHistory_Remove();
}

void CMcmsDaemonManager::OnKeepAlive(CSegment* pParam)
{
  DWORD process;
  *pParam >> process;
  if (process < NUM_OF_PROCESS_TYPES)
  {
    CProcessPolicy* processPolicies = pDaemonProcess->GetProcessMonitoringArray();
    processPolicies[process].SetStatus(eMonitorProcessStatusAlive);
  }
  else
  {
    PASSERTSTREAM(true,
        "Illegal process type " << process
        << ", not in a range(process + 100)");
  }
}

void CMcmsDaemonManager::OnResetProcessReq(CSegment* pParam)
{
  PASSERT_AND_RETURN(NULL == pParam);

  DWORD tmp;
  *pParam >> tmp;
  eProcessType type = (eProcessType)tmp;

  PASSERTSTREAM_AND_RETURN(!CMcmsDaemonProcess::IsProcessExists(type),
      "File " << ProcessTypeToStr(type)
      << " (" << type << ") doesn't exist");

  CProcessPolicy* policies = pDaemonProcess->GetProcessMonitoringArray();
  if (policies[type].GetStatus() == eMonitorProcessStatusTerminated)
  {
    TRACEINTOFUNC << "Process " << ProcessTypeToStr(type)
                  << " (" << type << ") hasn't been started yet";
    return;
  }

  switch (type)
  {
    case eProcessApacheModule:
    {
      // Sends asynchronous request to restart Apache as root
      CTaskApi::SendMsgWithTrace(eProcessConfigurator,
                                 eManager,
                                 NULL,
                                 CONFIGURATOR_RESTART_APACHE);
      break;
    }
    case eProcessQAAPI:
    case eProcessLogger:   //kobi - added option to reset Logger Proc Also
    {
      std::string killCmd = "Bin/McuCmd kill "  ; //former "Bin/McuCmd kill QAAPI"
      killCmd += ProcessTypeToStr(type);
      string      output;
      SystemPipedCommand(killCmd.c_str(), output);

      BOOL giveUpRoot = IsGiveRoot(type);
      int  processNice = GetProcessNiceLevel(type); // for Call Generator - change MM process priority

      char   arg1[32] = {0};
      STATUS status = SystemRunProcess(type, arg1, giveUpRoot, processNice);

      TRACEINTO << "Restart of " << ProcessTypeToStr(type) << " was "
                << (STATUS_OK == status ? "Successfully " : "NOT ")
                << "done";
      break;
    }
    default:
    {
      // For the moment, it effects only Apache process
      PASSERTSTREAM_AND_RETURN(true,
        "Reset of process " << ProcessTypeToStr(type)
        << " (" << type << ") is not supported");
    }
  } // switch
}

void CMcmsDaemonManager::OnInstallerStopIpmc(CSegment* pParam)
{
  m_isDuringInstalltion = TRUE;
}

void CMcmsDaemonManager::OnCheckChildZombieProcessesTimer(CSegment* pParam)
{
	TRACEINTOFUNC << "Enter";

	pid_t childId = CollectZombieChildProcesses();
	while (childId > 0)
	{
		TRACEINTOFUNC << "Collected zombie process: " << childId;
		childId = CollectZombieChildProcesses();
	}
	StartTimer(MCMS_DAEMON_CHECK_ZOMBIE_TOUT, ZOMBIE_TOUT);
}

void CMcmsDaemonManager::OnConfigApacheInd(CSegment* pParam)
{
	TRACEINTOFUNC << "apache mode = " << m_apacheMode;

	// if ApacheModule turn has been already arrived, run it from here
	if(eWaitForStartup == m_apacheMode)
	{
		TRACEINTOFUNC << "Launching Apache";
	  STATUS status = LaunchProcess(eProcessApacheModule);
	  CProcessPolicy *processPolicies = pDaemonProcess->GetProcessMonitoringArray();
	  processPolicies[eProcessApacheModule].SetTimeFirstUp();
	}
	else
	{
		m_apacheMode = eMcuMngrInd;
	}
}

void CMcmsDaemonManager::OnMcmsNetworkFinish(CSegment* pParam)
{
	m_isMcmsNetworkFinishInd = TRUE;
	CSegment  seg;
	seg << (DWORD)eProcessMcmsNetwork;
	OnProccessUpStartup(&seg);
}

void CMcmsDaemonManager::OnProccessUpStartup(CSegment* pParam)
{
  DWORD           processType;
  CProcessPolicy* processPolicies = pDaemonProcess->GetProcessMonitoringArray();

  if (pParam)
  {
    *pParam >> processType;
    if (processType >= NUM_OF_PROCESS_TYPES)
    {
      // this should never happen
      PASSERT(processType + 100);
      return;
    }

    if (eProcessMcmsDaemon != (eProcessType)processType &&
        !CMcmsDaemonProcess::IsLaunched((eProcessType)processType))
      return;

    if (processType != m_prevToRun)
    {
      TRACEINTOFUNC << "PROCESS_UP from: "
                    << CProcessBase::GetProcessName((eProcessType)processType)
                    << " But Daemon expects for "
                    << CProcessBase::GetProcessName((eProcessType)m_prevToRun);

      std::map<eProcessType, DWORD>::iterator itFind;
      itFind = m_mapProcessToFaultID.find((eProcessType)processType);
      if (itFind != m_mapProcessToFaultID.end())
      {
        RemoveActiveAlarmById(itFind->second);
        m_mapProcessToFaultID.erase(itFind);
      }

      return;
    }

    TRACEINTOFUNC << "Got PROCESS_UP from "
                  << CProcessBase::GetProcessName((eProcessType)processType);

    processPolicies[processType].SetStatus(eMonitorProcessStatusAlive);
    DeleteTimer(PROCESS_UP);
  }
  else
  {
    if (eProcessType(m_prevToRun) != eProcessDiagnostics)
      HandleProcessFailedToStart((eProcessType)m_prevToRun, "Startup");

    processType = m_prevToRun;
  }

  if (processType >= NUM_OF_PROCESS_TYPES)
  {
    // this should never happen
    PASSERT(processType + 100);
    return;
  }

  if (!CMcmsDaemonProcess::IsLaunched((eProcessType)processType) &&
      processType != eProcessMcmsDaemon)
    // non launched processes are not playing any part of this loop
    // and their indications are ignored
    return;

  if (eProcessIPMCInterface == (eProcessType)processType)
  {
    StartTimer(MCMS_DAEMON_HW_TIMER, WD_HW_START);
    TRACEINTOFUNC << "Start HW WD in " << WD_HW_START / SECOND
                  << " seconds...";
  }

  //if using mcmsnetwork check if phase1 of configuration is completed
  if((eProcessMcmsNetwork ==processType) && !m_isMcmsNetworkFinishInd)
  {
	  TRACEINTOFUNC << "McmsNetwork process is up waiting for event MCMSNETWORK_TO_MCMSDAEMON_FINISH to be received.";
	  return;
  }
  TRACEINTOFUNC << "processType = " << processType;
  DWORD nextToRun = processType + 1;


  if (CMcmsDaemonProcess::IsLaunched((eProcessType)nextToRun) &&
      !CMcmsDaemonProcess::IsProcessExists((eProcessType)nextToRun))
  {
    CSmallString FaultMsg;
    FaultMsg << CProcessBase::GetProcessName((eProcessType)nextToRun)
             << " does not exist!";
    cout <<FaultMsg.GetString() << endl;
    cout.flush();

    AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                   MODULE_DOES_NOT_EXIST,
                   MAJOR_ERROR_LEVEL,
                   FaultMsg.GetString(),
                   true,
                   true);
    nextToRun++;
  }

  TRACEINTOFUNC << "before loop nextToRun = " << nextToRun;

  while (!CMcmsDaemonProcess::IsLaunched((eProcessType)nextToRun) &&
         (eProcessType)nextToRun != NUM_OF_PROCESS_TYPES)
  {
    // Finds the next launched process
    nextToRun++;
    TRACEINTOFUNC << "in loop nextToRun = " << nextToRun;
  }

  if (nextToRun == NUM_OF_PROCESS_TYPES)
  {
    OnEndLaunchMcmsProcesses();
    return;
  }

	eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
	if (eProductFamilySoftMcu != prodFamily)
	{
  		usleep(200000);
	}
	else
	{
		usleep(50000);	
	}
  PASSERTSTREAM(processPolicies[nextToRun].IsAlive(),
      "Process " << nextToRun << ":"
      << CProcessBase::GetProcessName((eProcessType)nextToRun)
      << " is alive");

  // ApacheModule should wait for McuMngr indication
  if ((eProcessApacheModule == nextToRun) && (eNotInit == m_apacheMode))
  {
    m_apacheMode = eWaitForStartup;
    TRACEINTOFUNC << "An indication from McuMngr has not been received yet";

    // Increments for the next process
    return;
  }

  STATUS status = LaunchProcess((eProcessType)nextToRun);
  processPolicies[processType].SetTimeFirstUp();

  OnStartTimerByProcessAndProductType((eProcessType)nextToRun);
}

void CMcmsDaemonManager::OnStartTimerByProcessAndProductType(eProcessType type)
{
  StartTimer(PROCESS_UP, MAX_TIME_FOR_PROCESS_STARTUP);
  TRACEINTOFUNC << "Startup timeout of "
                << CProcessBase::GetProcessName(type)
                << " is " << MAX_TIME_FOR_PROCESS_STARTUP / SECOND << " seconds";
}

void CMcmsDaemonManager::OnEndLaunchMcmsProcesses()
{
  TRACEINTOFUNC << "Enter";

  SendEventToAudit("Power up",
                   "The system was powered up.",
                   "",
                   "");

  // this is end of startup, Software WD starts here
  StartTimer(MCMS_DAEMON_SW_TIMER, WD_SW_INTERVAL);

  m_state = READY;

  std::string file = OS_STARTUP_LOGS_PATH;
  file += DAEMON_LOADING_LOG;
  ::DumpFile(file);

  if (pDaemonProcess->GetIsSafeMode())
    AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                   SYSTEM_IN_SAFE_MODE,
                   MAJOR_ERROR_LEVEL,
                   "System entered to safe mode due to endless resets",
                   true,
                   true);

  if (pDaemonProcess->GetSystemCfgActiveAlarm())
    AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                   GUI_SYSTEM_CONFIG_IS_ILLEGAL,
                   MAJOR_ERROR_LEVEL,
                   "System did not use system cfg file sent through GUI",
                   true,
                   true);

  if (IsFileExists(NEATLY_SHOUTDOWN_PATH))
  {
    // BAD, The last shutdown was not done neatly
    CLargeString description;
    BOOL         isFullOnly = FALSE;
    CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                        WILD_RESET_IND,
                        MAJOR_ERROR_LEVEL,
                        "System did not shut down properly",
                        isFullOnly);

    SendEventToAudit("Improper shut down",
                     "The system did not shut down properly.",
                     "System did not shut down properly",
                     "");
  }
  else
  {
    // GOOD, The last shutdown was done neatly
    BOOL res = CreateFile(NEATLY_SHOUTDOWN_PATH);
    if (FALSE == res)
      TRACEINTOFUNC << "Failed to create file";
  }

  StartTimer(MCMS_DAEMON_CHECK_ZOMBIE_TOUT, ZOMBIE_TOUT);
}

void CMcmsDaemonManager::SendEventToAudit(const std::string& action,
                                          const std::string& description,
                                          const std::string& descriptionEx,
                                          const std::string& data)
{
  AUDIT_EVENT_HEADER_S auditHdr;
  CAuditorApi::PrepareAuditHeader(auditHdr,
                                  "",
                                  eMcms,
                                  "",
                                  "",
                                  eAuditEventTypeInternal,
                                  eAuditEventStatusOk,
                                  action,
                                  description,
                                  "",
                                  descriptionEx);
  CFreeData freeData;
  CAuditorApi::PrepareFreeData(freeData,
                               "Event Details",
                               eFreeDataTypeText,
                               data,
                               "",
                               eFreeDataTypeText,
                               "");
  CAuditorApi api;
  api.SendEventMcms(auditHdr, freeData);
}

void CMcmsDaemonManager::HandleProcessFailedToStart(eProcessType processsType,
                                                    const char* stage)
{
  if (eProcessApacheModule == processsType)
  {
    TRACEINTOFUNC << "ApacheModule FAILED to start during " << stage
                  << " (yet nothing is done, since it's ApacheModule process)";
  }
  else // other processes (except ApacheModule)
  {
    CSmallString msg;
    msg << CProcessBase::GetProcessName(processsType)
        << " FAILED to start during " << stage << " !!!";

    cout << msg.GetString() << endl;
    cout.flush();

    PASSERTMSG(1, msg.GetString());


   DWORD faultID = AddActiveAlarmFaultOnly(FAULT_GENERAL_SUBJECT,
                                           PROCESS_STARTUP_FAILED,
                                           MAJOR_ERROR_LEVEL,
                                           msg.GetString());

    if (faultID != 0xFFFFFFFF )
    {
    	m_mapProcessToFaultID[processsType] = faultID;
    }
    else
    {
        TRACEINTOFUNC << "Failed to add PROCESS_STARTUP_FAILED fault. message:\n" << msg ;
    }

    CProcessPolicy* processPolicies = pDaemonProcess->GetProcessMonitoringArray();
    processPolicies[m_prevToRun].Terminate();
  }
}

void CMcmsDaemonManager::OnProccessUpReady(CSegment* pParam)
{
  DWORD processType = NUM_OF_PROCESS_TYPES;
  if (pParam)
  {
    *pParam >> processType;
    TRACEINTOFUNC << "PROCESS_UP recovery from: "
                  << CProcessBase::GetProcessName((eProcessType)processType);

    DeleteTimer(PROCESS_UP);
    std::map<eProcessType, DWORD>::iterator itFind;
    itFind = m_mapProcessToFaultID.find((eProcessType)processType);
    if (itFind != m_mapProcessToFaultID.end())
    {
      RemoveActiveAlarmFaultOnlyById(itFind->second);
      m_mapProcessToFaultID.erase(itFind);
    }
  }
  else
  {
    HandleProcessFailedToStart((eProcessType)m_prevToRun, "Recovery");
    return;
  }

  // it's a recovery of some process
  if (processType < NUM_OF_PROCESS_TYPES)
  {
    CProcessPolicy* processPolicies = pDaemonProcess->GetProcessMonitoringArray();
    processPolicies[processType].UpdateNumOfRetry();
  }
  else
  {
    PASSERTMSG(processType, "Bad process type(err code = process type)");
    return;
  }
}

void CMcmsDaemonManager::DumpProcessLoadInfo(eProcessType processType)
{
  std::string info, answer;

  SystemPipedCommand("uptime", answer);
  info = "\nUPTIME: ";
  info += answer;

  info += "DATE: ";
  SystemPipedCommand("date", answer);
  info += answer;

  info += "TICKS: ";
  char ticks[10];
  memset(ticks, '\0', sizeof(ticks));
  snprintf(ticks, sizeof(ticks), "%d",
           SystemGetTickCount().GetIntegerPartForTrace());
  info += ticks;

  info += "\nProcessName: ";
  info += CProcessBase::GetProcessName(processType);

  std::string dump = "echo \"" + info + "\" >> " + OS_STARTUP_LOGS_PATH +
                DAEMON_LOADING_LOG;
  SystemPipedCommand(dump.c_str(), answer);
}

STATUS CMcmsDaemonManager::LaunchProcess(eProcessType processType)
{
  DumpProcessLoadInfo(processType);

  PASSERTSTREAM_AND_RETURN_VALUE((DWORD)processType >= NUM_OF_PROCESS_TYPES,
      "Bad process type " << processType,
      STATUS_FAIL);

  CProcessPolicy* processPolicies = pDaemonProcess->GetProcessMonitoringArray();
  DWORD           numLaunch = processPolicies[processType].GetNumLaunch();
  char            arg1[32];
  sprintf(arg1, "%d", numLaunch);

  BOOL   giveUpRoot = IsGiveRoot(processType);
  int    processNice = GetProcessNiceLevel(processType); // for Call Generator - change MM process priority
  STATUS status = SystemRunProcess(processType, arg1, giveUpRoot, processNice);

  m_prevToRun = processType;

  processPolicies[processType].IncNumLaunch();
  processPolicies[processType].SetStatus(eMonitorProcessStatusStartup);

  DWORD maxNumLaunch = processPolicies[processType].GetMaxNumLaunch();

  const std::string& statusName = pDaemonProcess->GetStatusAsString(status);
  const char* launchProcessName = CProcessBase::GetProcessName(processType);

  // Enables sending signal killall -6 to this process
  m_isProcessRecievedCoreDump[processType] = FALSE;

  std::ostringstream msg;
  msg << "Launch " << numLaunch << " of " << maxNumLaunch
      << " " << statusName << " " << launchProcessName << "...";

  TRACEINTOFUNC << msg.str();

  if (!IsTarget())
  {
    std::ostringstream buf;
    time_t tt = time(NULL);
    struct tm* now = localtime(&tt);
    if (now)
      buf << std::setfill('0')
          << std::setw(4) << now->tm_year + 1900
          << std::setw(2) << now->tm_mon + 1
          << std::setw(2) << now->tm_mday
          << std::setw(2) << now->tm_hour
          << std::setw(2) << now->tm_min
          << std::setw(2) << now->tm_sec;
    else
      buf << "undefined time";

    std::cerr << buf.str() << " " << msg.str();
  }

  return status;
}

void CMcmsDaemonManager::OnSW_WD_Timer(CSegment* pMsg)
{
  StartTimer(MCMS_DAEMON_SW_TIMER, WD_SW_INTERVAL);

  CheckDebugMode();
  OnWDTimerMcms();
}

// for Call Generator - expiration time
void CMcmsDaemonManager::OnCG_WD_Timer(CSegment* pMsg)
{
  TRACECOND_AND_RETURN(
      CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator,
      "The system is not CG");

  TRACEINTOFUNC << "Enter";

  StartTimer(MCMS_DAEMON_CG_LICENCE_TIMER, WD_CG_INTERVAL);
  if (IsActiveAlarmExistByErrorCode(AA_NO_LICENSING))
    RemoveActiveAlarmByErrorCode(AA_NO_LICENSING);

  if (Check_CG_expiration_time())
  {
    std::string description = "Abort due to CallGenearator time expiration";
    TryToAbort(description, eResetSourceInternalMcmsDaemon);
  }
}

// for Call Generator - expiration time
bool CMcmsDaemonManager::Check_CG_expiration_time()
{
  TRACECOND_AND_RETURN_VALUE(
    CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator,
    "The system is not CG",
    false);

  CStructTm currentTime;
  SystemGetTime(currentTime);
  CStructTm expirationTime(1, 6, 111, 0, 0, 0);
  currentTime.m_day -= 1;
  currentTime.m_mon -= 1;
  currentTime.m_year -= 1900;

  // CStructTm dif=expirationTime.GetTimeDelta(currentTime);
  tm expirationTimeTmp;
  expirationTime.GetAsTm(expirationTimeTmp);
  time_t absExpirationTime = mktime(&expirationTimeTmp);

  tm currentTimeTmp;
  currentTime.GetAsTm(currentTimeTmp);
  time_t absCurrentTime = mktime(&currentTimeTmp);
  int    diff = absExpirationTime - absCurrentTime;
  if (diff < 0)
    return true;

  DWORD daysDiff = (((diff / 60) / 60) / 24);
  CSmallString description;
  description << "CallGenrator license will expire in " << daysDiff << " days";
  if (daysDiff <= 7)
    AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                   AA_NO_LICENSING,
                   MAJOR_ERROR_LEVEL,
                   description.GetString(),
                   true, true);

  return false;
}

void CMcmsDaemonManager::OnHW_WD_Timer(CSegment* pMsg)
{
  if (FALSE == m_disable_IPMC_usage)
  {
    StartTimer(MCMS_DAEMON_HW_TIMER, WD_HW_INTERVAL);
    TRACEINTOFUNC << "Continue HW WD in " << WD_HW_INTERVAL / SECOND
                  << " seconds...";

    CheckDebugMode();
    OnWDTimerHW();
  }
  else
  {
    TRACEINTOFUNC << "DISABLE_IPMC_USAGE == YES";
  }
}

void CMcmsDaemonManager::OnWDTimerHW()
{
  CIPMCInterfaceApi api;
  BOOL              isSafeMode = pDaemonProcess->GetIsSafeMode();
  if (TRUE == m_isDebug_Mode ||
      TRUE == isSafeMode ||
      TRUE == m_isEnteringInDiagnosticsMode ||
      TRUE == m_isDuringInstalltion)
  {
    m_disable_IPMC_usage = TRUE;
    api.TurnOffWatchdog();
    return;
  }

  // IPMC - can not be set to a value greater than 25 seconds!
  DWORD wdTimeout = 25;
  pDaemonProcess->GetSysConfig()->GetDWORDDataByKey("HD_WATCHDOG_TIMEOUT",
                                                    wdTimeout);
  api.SetWatchdog(wdTimeout);
  api.TriggerWatchdog();
}

void CMcmsDaemonManager::OnWDTimerMcms()
{
  if (!m_isEnteringInDiagnosticsMode)
  {
    CProcessPolicy* processPolicies = pDaemonProcess->GetProcessMonitoringArray();
    for (int iter = (int)NUM_OF_PROCESS_TYPES - 1;
         iter > (int)eProcessTypeInvalid; iter--)
    {
      eProcessType process = (eProcessType) iter;

      if (false == CMcmsDaemonProcess::IsWatched(process))
        continue;

      if (true == processPolicies[iter].IsAlive() ||
          true == processPolicies[iter].IsInStartup() ||
          true == processPolicies[iter].IsTerminated())
        continue;

      if (processPolicies[iter].IsAdditionalWDRetry(process))
      {
        TRACEINTOFUNC << "Wait the process more " << ProcessNames[process];
        continue;
      }

      HandleProcessFailed(process);
    }

    pDaemonProcess->SetAllProcessesStatus(eMonitorProcessStatusAlive,
                                          eMonitorProcessWaitForWD);
  }
}

void CMcmsDaemonManager::OnResetHistoryTimer()
{
  ResetHistory_Remove();
}

void CMcmsDaemonManager::HandleProcessFailed(eProcessType process)
{
  if (!CMcmsDaemonProcess::IsProcessExists(process))
  {
    // Stop trying to load the process
    TerminateProcess(process);
    return;
  }

  // send check CPU usage to SystemMonitoring
  CManagerApi api(eProcessSystemMonitoring);
  api.SendOpcodeMsg(SYSTEM_CHECK_SYSTEM_CPU_USAGE_REQ);

  CProcessPolicy* processPolicies = pDaemonProcess->GetProcessMonitoringArray();

  CLargeString description;
  description << "Process failed "
              << "[" << processPolicies[process].GetAbsCrashCounter()
              << ":" << processPolicies[process].GetCrashCounter()<< "] : "
              << CProcessBase::GetProcessName(process);

  BOOL isFullOnly = TRUE;
  CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                      PROCESS_FAILED,
                      MAJOR_ERROR_LEVEL,
                      description.GetString(),
                      isFullOnly);

  eFaultedProcessAction policyType = processPolicies[process].GetPolicy();

  if (eProcessConfParty == process)
    DumpConfPartyProcessInfo();

  switch (policyType)
  {
    case eReset:
    {
      TryToForceCoreDump(process);
      TerminateProcess(process);
      TryToAbort(description.GetString(), eResetSourceInternalMcmsDaemon);
      break;
    }

    case eNoResetAfterRetryLoading:
    case eResetAfterRetryLoading:
    {
      if (processPolicies[process].IsAnotherRetry())
      {
        // on DEBUG_MODE=YES, Faults and Alerts are produced,
        // however the actual Watchdog mechanism is disabled
        if (FALSE == m_isDebug_Mode)
        {
          TryToForceCoreDump(process);
          usleep(500000);
          m_prevToRun = process;
          STATUS status = LaunchProcess(process);
          usleep(200000);
          OnStartTimerByProcessAndProductType(process);
        }
        else
          TerminateProcess(process);
      }
      else
      {
        TryToForceCoreDump(process);
        TerminateProcess(process);

        // In case the process is eNoResetAfterRetryLoading
        // We will not try to load it again after number resets
        if (eResetAfterRetryLoading == policyType)
        {
          TryToForceCoreDump(process);
          TryToAbort(description.GetString(), eResetSourceInternalMcmsDaemon);
        }
      }

      break;
    }
    case eNoRetryLoading:
    {
      TryToForceCoreDump(process);
      TerminateProcess(process);
      break;
    }

	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  } // switch
}

void CMcmsDaemonManager::TerminateProcess(eProcessType process)
{
  CProcessPolicy* processPolicies = pDaemonProcess->GetProcessMonitoringArray();
  processPolicies[process].Terminate();

  std::string description = CProcessBase::GetProcessName(process);
  description += " terminated";

  CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                      PROCESS_TERMINATED,
                      MAJOR_ERROR_LEVEL,
                      description.c_str(),
                      FALSE);
}

void CMcmsDaemonManager::TryToForceCoreDump(eProcessType process)
{
  TRACECOND_AND_RETURN(m_isProcessRecievedCoreDump[process],
      "Process " << CProcessBase::GetProcessName(process)
      << " received killall -6 the second time - ignore");

  std::ostringstream cmd;
  cmd <<"McmsDaemonManager::TryToForceCoreDump killall -6  process =" << CProcessBase::GetProcessName(process);
  PrintErrorToLocalFile(cmd);
  // Prevents the process from receiving killall -6 for next time
  m_isProcessRecievedCoreDump[process] = TRUE;

  if (!m_isDebug_Mode)
  {
    std::string command = "killall -6 ";

    // Judith - VNGR-16093. In case it is httpd process, we need to create the
    // core for the httpd file and not for the ApacheModule bash.
    if (process == eProcessApacheModule)
      command += "httpd";
    else
      command += CProcessBase::GetProcessName(process);

    std::string output;
    SystemPipedCommand(command.c_str(), output);
    PASSERTSTREAM(true, command.c_str() << ": " << output.c_str());

    SystemSleep(100);
  }
}

void CMcmsDaemonManager::SelfKill()
{
  DeleteFile(NEATLY_SHOUTDOWN_PATH);

  close(m_PowerButtonFD);

  ProduceResetFaultAndLog();
  SendEventToAudit("Shut down",
                   "The system was shut down.",
                   m_ResetDescription,
                   "");

  //send to apache to stop receiving requests
  CManagerApi apache_api(eProcessApacheModule);
  apache_api.SendOpcodeMsg(APACHE_BLOCK_REQUESTS);
  SystemSleep(3);

  for (int iter = (int)NUM_OF_PROCESS_TYPES - 1;
       iter > (int)eProcessTypeInvalid;
       iter--)
  {
    eProcessType process = (eProcessType)iter;
    if (CMcmsDaemonProcess::IsLaunched(process))
    {
      switch (process)
      {
        case eProcessLogger:
        case eProcessFaults:
        case eProcessApacheModule:
        case eProcessIPMCInterface:
        // Will be killed later
        break;

        case eProcessCsModule:
        KillCSModule();
        break;

        default:
        CManagerApi api(process);
        api.Destroy("Destroyed by McmsDaemon");
        break;
      } // switch
    }
  }

  SystemSleep(20);

  //kills also EndpointSim and GideonSim in Simulation
  if (!IsTarget())
  {
	  CManagerApi epSim_api(eProcessEndpointsSim);
	  epSim_api.Destroy("Destroyed by McmsDaemon");

	  CManagerApi gidSim_api(eProcessGideonSim);
	  gidSim_api.Destroy("Destroyed by McmsDaemon");

	  SystemSleep(20);
  }

	  //BRIDGE-15073 - kill apache before the logger (Shimon + Judith)
	  KillApacheModule();
	  SystemSleep(20);  
	  
  // the idea of this nested block is to put CManagerApi creation into block  
  // so the ManagerApi will be desroyed after this block
  // otherwise the destruction of the CManagerApi never happens and causes leak,
  // probably because of the "CManagerTask::SelfKill()".
  {
	  // close logger and fault and IPMCInterface last
	  CManagerApi logger_api(eProcessLogger);
	  logger_api.Destroy("Destroyed by McmsDaemon");

	  CManagerApi faults_api(eProcessFaults);
	  faults_api.Destroy("Destroyed by McmsDaemon");


  }

  //BRIDGE-11355 ******************************************
  CIPMCInterfaceApi api;
  if (!m_isEnteringInDiagnosticsMode)
  {
	  api.SetWatchdog(10,true);
	  api.TriggerWatchdog(true);
  }

  CManagerApi IPMC_api(eProcessIPMCInterface);
  IPMC_api.Destroy("Destroyed by McmsDaemon");

  //*********************************************************
  //fixing this patch by adding an indication file for entering Diagnostics mode , start.sh prevented from reboot
  //if (!m_isEnteringInDiagnosticsMode)
	  CManagerTask::SelfKill();
}

void CMcmsDaemonManager::KillCSModule()
{
  std::string output;
  SystemPipedCommand("killall acloader", output);
}

void CMcmsDaemonManager::KillApacheModule()
{
  std::string output;
  SystemPipedCommand("Scripts/KillApache.sh", output);
}

int CMcmsDaemonManager:: GetPrivateFileDescriptor()
{
  return m_PowerButtonFD;
}

BOOL CMcmsDaemonManager::IsGiveRoot(eProcessType processType) const
{
  BOOL giveUpRoot = (processType == eProcessConfigurator ||
                     processType == eProcessApacheModule ||
                     processType == eProcessCsModule ||
                     processType == eProcessIPMCInterface ||
                     processType == eProcessDiagnostics   ||
                     processType == eProcessMcmsNetwork
                     ?
                     FALSE : TRUE);
  return giveUpRoot;
}

int CMcmsDaemonManager::GetProcessNiceLevel(eProcessType type) const
{
  switch (type)
  {
    case eProcessMediaMngr:
    case eProcessIPMCInterface:
      return -15;

    case eProcessCollector:
    case eProcessEncryptionKeyServer:
      return 15;

	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
  }

  return 0xffff;
}

void CMcmsDaemonManager::AddFilterOpcodePoint()
{
  AddFilterOpcodeToQueue(WD_KEEP_ALIVE);
}

void CMcmsDaemonManager::CheckDebugMode()
{
  m_LastCheckedDebugMode = m_isDebug_Mode;
  CSysConfig* sysConfig = pDaemonProcess->GetSysConfig();
  sysConfig->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE, m_isDebug_Mode);
}

// 1) Daemon did not received watch dog from some of the MCMS process.
// 2) Daemon received Reset request from one of MCMS components
void CMcmsDaemonManager::TryToAbort(const std::string& description,
                                    eResetSource resetSource)
{
  m_resetSource = resetSource;
  if (eResetSourceInternalMcmsDaemon == resetSource)
  {
    m_ResetDescription = "McmsDaemon reset due to WD policy decision: ";
  }
  else if (eResetSourceInternalMcmsOther == resetSource)
  {
    m_ResetDescription = "McmsDaemon reset due to Mcms request: ";
  }
  else
  {
    m_ResetDescription = "McmsDaemon reset due to UNKNOWN reason: ";
    PASSERTMSG(resetSource + 100,
               "McmsDaemon reset due to UNKNOWN reason(resetSource + 100)");
  }

  m_ResetDescription += description;

  BOOL isDebugMode = FALSE;
  pDaemonProcess->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_DEBUG_MODE,
                                                   isDebugMode);
  BOOL isSafeMode = pDaemonProcess->GetIsSafeMode();

  if (FALSE == isDebugMode && FALSE == isSafeMode)
  {
    m_selfKill = TRUE;
  }
  else
  {
    m_ResetDescription += " ; Cannot reset while system is in ";
    m_ResetDescription += (TRUE == isDebugMode ? "DEBUG mode" : "SAFE mode");
    ProduceResetFaultAndLog();
    m_ResetDescription = "";
  }
}

// Daemon received reset request from MCMS process(CSMngr, Cards, McuMngr)
void CMcmsDaemonManager::OnResetMcmsReq(CSegment* pParam)
{
  std::string description;
  *pParam >> description;
  TryToAbort(description, eResetSourceInternalMcmsOther);
}

// Daemon received request to start new signaling process
void CMcmsDaemonManager::OnStartSignaling(CSegment* pParam)
{
  WORD  num_of_signalings;
  DWORD csId;
  WORD  numOfCalls;

  *pParam >> num_of_signalings;

  BOOL isXMLMode = FALSE;
  pDaemonProcess->GetSysConfig()->GetBOOLDataByKey("CS_API_XML_MODE", isXMLMode);
  std::ostringstream arg4;
  arg4 << "-X" << (int)isXMLMode;

  TRACEINTO << ">>>>+++++<<<<CMcmsDaemonManager::OnStartSignaling - arg4 " << arg4.str().c_str() << "\n";

  for (int i = 0; i < num_of_signalings; i++)
  {
    *pParam >> csId;
    *pParam >> numOfCalls;

    TRACEINTOFUNC << "Start Signaling number = " << csId
                  << " with "<< numOfCalls << " calls";

    // run loader according to these details
    std::ostringstream arg2, arg3;
    arg2 << "-S" << csId;
    arg3 << "-N" << numOfCalls;

    eProductType prodType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
	if (eProductFamilySoftMcu != prodFamily)
	{
  		SystemRunCommand((MCU_CS_DIR+"/bin/loader").c_str(), "-R", arg2.str().c_str(),arg3.str().c_str(), arg4.str().c_str());
	}
	else if (prodType == eProductTypeGesher || prodType == eProductTypeNinja)
	{
		//std::string runCsCmd = "./bin/acloader -c -C$CS_DIR/cfg/cfg_soft/cs_private_cfg_dev.xml -P0 -S" + i+" -N400";
		//SystemRunCommand("./bin/acloader -c -C$CS_DIR/cfg/cfg_soft/cs_private_cfg_dev.xml -P0 -S1 -N400");
		SystemRunCommand((MCU_CS_DIR+"/bin/loader").c_str(), "-R", arg2.str().c_str(),arg3.str().c_str(), arg4.str().c_str());
	}
	else
	{
		SystemRunCommand("./bin/acloader -c -C$CS_DIR/cfg/cfg_soft/cs_private_cfg_dev.xml -P0 -S1 -N400");
	}
  }
}

// Daemon received reset request from Shelf.
void CMcmsDaemonManager::HandlePrivateFileDescriptor()
{

    int revent = m_pRcvMbxRead->GetICPAEvent();

    FPTRACE2INT(eLevelInfoNormal, "\nCMcmsDaemonManager::HandlePrivateFileDescriptor got : revents ", revent );


    /* we get value 1 on soft reset and on WD
     * we get value -1 when m_internals is NULL
     * value different from 1 and -1 should be one of the following errors
     *
     *  Event types always implicitly polled for.  These bits need not be set in
   `events', but they will appear in `revents' to indicate the status of
   the file descriptor.
#define POLLERR     0x008       // Error condition.
#define POLLHUP     0x010       // Hung up.
#define POLLNVAL    0x020       // Invalid polling request.
     *
     */
    if (revent != 1  && revent != -1)
    {
        char desc[ONE_LINE_BUFFER_LEN];
        sprintf(desc, "Power down signal was detected with icpa event  %d - - -", revent);

        CHlogApi::TaskFault( FAULT_GENERAL_SUBJECT,
                INTERNAL_ICPA_EVENT,
                MAJOR_ERROR_LEVEL,
                desc,
                TRUE
        );
    }


    m_ResetDescription = "Power down signal was detected";
    m_resetSource = eResetSourceExternalShelf;
    m_selfKill = TRUE;

    ResetHistory_Remove();

}

// Daemon received reset request from terminal command
void CMcmsDaemonManager::HandleTerminalReset(CTerminalCommand& command,
                                             std::ostream& answer)
{
  m_ResetDescription = "MCU reset was initiated by Terminal command [reset]";
  m_resetSource = eResetSourceExternalTerminal;
  m_selfKill = TRUE;

  ResetHistory_Remove();
}

// Daemon received Enter_Diagnostics request from terminal command
void CMcmsDaemonManager::HandleEnterDiagnostics(CTerminalCommand& command,
                                                std::ostream& answer)
{
  m_isEnteringInDiagnosticsMode = TRUE;
  m_resetSource = eResetSourceDiagnostics;
  m_ResetDescription = "Enter diagnostics mode";

  CIPMCInterfaceApi api;
  if (FALSE == m_isDebug_Mode)
    api.TurnOffWatchdog();

  api.SetPostCommand(128);

  string output;
  string touchCmd = "touch ";
  touchCmd += ENTER_DIAGNOSTICS_IND;
  SystemPipedCommand(touchCmd.c_str(), output);

  TRACEINTOFUNC << "Removed IPMC WatchDog";
  m_selfKill = TRUE;
}

void CMcmsDaemonManager::HandleTerminalKillCS(CTerminalCommand& command,
                                              std::ostream& answer)
{
  KillCSModule();
}

void CMcmsDaemonManager::ProduceResetFaultAndLog()
{
  CLargeString fullDesc = m_ResetDescription.c_str();
  DWORD        errorCode = RESET_MCU;

  switch (m_resetSource)
  {
    case eResetSourceInternalUnknown:
      fullDesc << "\n" << "Reset cause: " << m_resetSource;
      PASSERTMSG(TRUE, fullDesc.GetString());
      break;

    case eResetSourceInternalMcmsDaemon:
      errorCode = RESET_MCU_INTERNAL;
      break;

    case eResetSourceInternalMcmsOther:
      errorCode = RESET_MCU_INTERNAL;
      break;

    case eResetSourceExternalEma:
      errorCode = RESET_MCU_BY_OPERATOR;
      break;

    case eResetSourceExternalShelf:
      errorCode = RESET_MCU_INTERNAL;
      break;

    case eResetSourceExternalTerminal:
      errorCode = RESET_MCU_BY_TERMINAL;
      break;

    case eResetSourceDiagnostics:
      errorCode = RESET_MCU_DIAGNOSTICS;
      break;

    default:
      fullDesc << "\n" << "Reset cause unknown: " << m_resetSource;
      PASSERTMSG(TRUE, fullDesc.GetString());
      break;
  }

  std::ostringstream cmd;
  cmd <<"CMcmsDaemonManager::ProduceResetFaultAndLog errorCode=" << errorCode;
  PrintErrorToLocalFile(cmd);
  CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                      errorCode,
                      MAJOR_ERROR_LEVEL,
                      fullDesc.GetString(),
                      FALSE);


}

void CMcmsDaemonManager::DumpConfPartyProcessInfo()
{
  CStructTm curTime;
  PASSERT(SystemGetTime(curTime));

  FileNameHeaders_S fileNameHeaders;

  char logfileName[CYCLIC_FILE_MAX_NAME_LEN] = {0};
  snprintf(logfileName, sizeof(logfileName),
           "%s%s_%02d-%02d-%04d_%02d-%02d-%02d.%s",
           WATCH_DOG_LOG_DIR.c_str(),
           WATCH_DOG_CONF_PARTY_DUMP_PREFIX,
           curTime.m_day,
           curTime.m_mon,
           curTime.m_year,
           curTime.m_hour,
           curTime.m_min,
           curTime.m_sec,
           fileNameHeaders.hdrExtension);

  DumpProcessLoadInfo(eProcessConfParty);

  std::string info, rm_cmd, ps_cmd, pstack_cmd, pmap_cmd, awk_result, answer;
  rm_cmd = "rm " + std::string(logfileName);
  SystemPipedCommand(rm_cmd.c_str(), answer, TRUE, FALSE);

  info += "DATE: ";
  SystemPipedCommand("date", answer);
  info += answer;

  ps_cmd = "ps -eLo pid,tid,comm,stat,pcpu,size,vsize,stackp,start_time,etime | grep ConfParty";

  if(YES==IsTarget() && (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily()) )
	  ps_cmd = "ps -eo pid,ppid,pgid,comm,nice,vsz,etime | grep ConfParty";

  SystemPipedCommand(ps_cmd.c_str(), answer);
  PASSERTMSG(true, answer.c_str());
  info +=
    "\n=======================\nCONFPARTY PROCESS INFO:\n=======================\n";
  info += answer.c_str();

  ps_cmd += " | awk '{print $1}' | awk 'NR<2' ";
  SystemPipedCommand(ps_cmd.c_str(), awk_result, TRUE, FALSE);

  pstack_cmd = "pstack " + awk_result;
  SystemPipedCommand(pstack_cmd.c_str(), answer, TRUE, FALSE);
  info += "\nPSTACK:\n=======================\n";
  info += answer;

  pmap_cmd = "pmap -x " + awk_result;
  SystemPipedCommand(pmap_cmd.c_str(), answer, TRUE, FALSE);
  info += "\nPMAP:\n=======================\n";
  info += answer;

  string dump = "echo \"" + info + "\" >> " + logfileName;
  SystemPipedCommand(dump.c_str(), answer, TRUE, FALSE);
}

void CMcmsDaemonManager::ManageProcessWatchList(CTerminalCommand& command,
                                                std::ostream& answer,
                                                BOOL State)
{
  DWORD numOfParams  = command.GetNumOfParams();
  if (numOfParams)
  {
    if (pDaemonProcess == NULL)
      pDaemonProcess = dynamic_cast<CMcmsDaemonProcess*>(CProcessBase::GetProcess());

    const std::string& processname = command.GetToken(eCmdParam1);
    answer << "convert " << processname << " to process Type \n";
    eProcessType pType = pDaemonProcess->ConvertStrToProcessType(processname);
    if (pType != eProcessTypeInvalid)
    {
      BOOL needUpdate = State ? pDaemonProcess->IsWatched(pType) :
          !(pDaemonProcess->IsWatched(pType));

      if (needUpdate)
      {
        answer << " process is already in current state no change is needed\n";
        return;
      }

      answer << "Update process process state\n";
      pDaemonProcess->UpdateProcessWatchState(pType, State);
    }
    else
      answer << "Error invalid process name\n";
  }
  else
    answer << "Error no process name was specify\n";
}

void CMcmsDaemonManager::HandleAddProcess(CTerminalCommand& command,
                                          std::ostream& answer)
{
  answer << "Add watch process to daemon \n";
  ManageProcessWatchList(command, answer, TRUE);
}

void CMcmsDaemonManager::HandleRemoveProcess(CTerminalCommand& command,
                                             std::ostream& answer)
{
  answer << "Remove watch process to daemon \n";
  ManageProcessWatchList(command, answer, FALSE);
}

void CMcmsDaemonManager::HandlePrintProcessWatchList(CTerminalCommand& command,
                                                     std::ostream& answer)
{
  answer << "Print watch process .. \n";
  if (pDaemonProcess == NULL)
    pDaemonProcess = dynamic_cast<CMcmsDaemonProcess*>(CProcessBase::GetProcess());

  pDaemonProcess->printMap(answer);
}
