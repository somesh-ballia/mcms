// ProcessBase.cpp

#include "ProcessBase.h"

#include <stdio.h>
#include <string>
#include <ostream>
#include <errno.h>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#include "Trace.h"
#include "TraceClass.h"
#include "SystemFunctions.h"
#include "ManagerApi.h"
#include "ManagerTask.h"
#include "MonitorTask.h"
#include "OsQueue.h"
#include "OsTask.h"
#include "StringsMaps.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "InitCommonStrings.h"
#include "FilterTrace.h"
#include "SysConfig.h"
#include "StatusesGeneral.h"
#include "OperatorDefines.h"
#include "IpCsOpcodes.h"
#include "OpcodeStringConverter.h"
#include "StatusStringConverter.h"
#include "FaultsContainer.h"
#include "ErrorHandlerTask.h"
#include "TraceStream.h"
#include "FilterTraceContainer.h"
#include "InternalProcessStatuses.h"
#include "FaultsDefines.h"
#include "ObjString.h"
#include "ProcessSettings.h"
#include "ServiceConfigList.h"
#include "TerminalCommand.h"
#include "FilterByLevel.h"
#include "psosxml.h"
#include "AlarmStrTable.h"
#include "AlarmStrDeclaration.h"
#include "OsFileIF.h"

CProcessBase* CProcessBase::m_pCurrentProcess = NULL;
void SignalHandler(int);
extern char* GetStringValidityStatus(eStringValidityStatus status);

CProcessBase* CProcessBase::GetProcess()
{
	return m_pCurrentProcess;
}

CProcessBase::CProcessBase() :
  m_pManagerApi(NULL),
  m_pTasks(new std::vector<CTaskApp*>),
  m_selfKill(false),
  m_FilterTraceContainer(new CFilterTraceContainer),
  m_ProcessState(eProcessIdle),
  m_pErrorHandlerApi(NULL)
{
	m_pListenSocketTask = NULL;

	m_pCurrentProcess = this;

	for (int i = 0; i < TASK_GROUPS_NUM; i++)
		m_taskGroupSemaphore[i] = -1;

  for (int i = 0; i < NUM_OF_PROCESS_TYPES; i++)
  {
    for (int j = 0; j < NUM_OF_HANDLES_IN_ENTRY; j++)
    {
      m_otherProcessQueues[i][j] = NULL;
    }
  }

  ResetMonitoring();

  m_OpcodeStringConverter = NULL;
  m_StatusStringConverter = NULL;

  //only for debug
  for (int i = 0; i < 5; i++)
    m_testFlag[i] = FALSE;
  m_testSignalFlag = TRUE;

  m_SysConfig = NULL; // should be initilized in Setup function
  m_ServiceConfigList = NULL;
  m_SystemState = NULL;// should be initilized in Setup function
  m_TasksSemaphoreId = -1;
  m_StartupTimeLimit = 0;
  m_traceIPC = FALSE;
  m_WDCnt = 0;
  m_IsMemoryExhausted = FALSE;

  pCProcessSettings = NULL;

  m_NumOfStartups = 0;
  m_Argv = NULL;
  m_Argc = 0;
  m_WorkMode = eProcessWorkModeNormal;
  m_SnmpTaskId = 0;

  m_IsTreatingOnAssert = 0;
  m_IsCompletedSetup = false;

  m_bIsFailoverFeatureEnabled	= false;
  m_bIsFailoverSlaveMode		= false;
  m_productTypeLastFound = eProductTypeUnknown;
  m_enableLocalTracer = FALSE;
  m_isUnderValgrid = FALSE;
  m_isFaultsSentMcuMngr = FALSE;
}

void CProcessBase::ResetMonitoring()
{
	m_numMessageSent   = 0;
	m_numMsgRcv        = 0;
	m_numSyncMsgSent   = 0;
	m_numToutSyncMsg   = 0;
	m_numLocalMsgSent  = 0;
	m_numLocalMsgRvc   = 0;
	m_numExpiredTimers = 0;
	m_numTrace	   	   = 0;
	m_numTraceNotSent  = 0;
	m_msgsSentConuterMap.clear();
}

// Virtual.
DWORD CProcessBase::GetMaxTimeForIdle() const
{
  return 4000;
}

CTaskApi* CProcessBase::GetErrorHandlerApi()
{
  FTRACEINTO << "Enter";
  return m_pErrorHandlerApi;
}

void CProcessBase::SetErrorHandlerApi(CTaskApi* pApi)
{
  m_pErrorHandlerApi = pApi;
}

CProcessBase::~CProcessBase()
{
  while (!m_pTasks->empty())
  {
    std::vector<CTaskApp*>::iterator itr = m_pTasks->begin();
    m_pTasks->erase(itr);
  }

  for (int i = 0; i < NUM_OF_PROCESS_TYPES; i++)
  {
    for (int j = 0; j < NUM_OF_HANDLES_IN_ENTRY; j++)
    {
      if (m_otherProcessQueues[i][j] != NULL)
      {
        PDELETE(m_otherProcessQueues[i][j]);
      }
    }
  }
  PDELETE(m_pTasks);
  m_pCurrentProcess = NULL;
  delete m_FilterTraceContainer;
}

int CProcessBase::Run()
{
  COsTask::SetTaskPrioirty(-10);

  SetUp();
  COsTask::Join(m_pManagerApi->GetTaskAppPtr()->GetOsTask()->m_id);

  TearDown();
  return TRUE;
}



int CProcessBase::SetUp()
{
  if (!IsTarget())
    std::cerr << GetProcessName(GetProcessType()) << " started." << std::endl;

  SetProcessAddressSpaceLimit();
  ParseArgv();

  std::string tasksSemName;
  tasksSemName += GetProcessName(GetProcessType());
  tasksSemName += "Tasks";
  if (STATUS_OK != CreateSemaphore(&m_TasksSemaphoreId, tasksSemName))
    m_selfKill = TRUE;

  if (UsingSockets())
    SystemInitSocket();

  COsQueue::DeleteAllProcessQueues(GetProcessType());
  CStringsMaps::Build();

  ::InitCommonStrings();

  AddExtraStringsToMap();

  InitAlarmTable();

  m_OpcodeStringConverter = new COpcodeStringConverter;
  AddExtraOpcodesStrings();

  m_StatusStringConverter = new CStatusStringConverter;
  AddExtraStatusesStrings();

  const char * rmx_status_dir = getenv("RMX_STATUS_DIR");
  if (rmx_status_dir)
  {
	GenerateStatusXmlFiles(rmx_status_dir);
	exit(0);
  }

  if (GivesAwayRootUser())
  {
    m_SystemState = new CSystemState;
  }
  else
  {
    m_SystemState = NULL;
  }

  if (IsHasSettings())
  {
    pCProcessSettings = new CProcessSettings();
  }

  SetUpProcess();
  CreateAllSemaphores();
  COsTask::InitTaskKey();
  PTRACE(eLevelInfoNormal, "CProcessBase::SetUp");
  if (GetManagerEntryPoint())
  {
    m_pManagerApi = new CManagerApi;
    COsQueue dummyMbx;
    m_pManagerApi->Create(GetManagerEntryPoint(), dummyMbx);
  }


  m_IsCompletedSetup = true;
  initLocalTracer();
  return 0;
}

// DONT USE PTRACE IN THIS FUNCTION
// LOGGER MIGHT NOT BE AVAILABLE AT THIS STAGE
// MIGHT LEAD TO ABNORMAL REMINATION OF THE PROCESS
int CProcessBase::TearDown()
{
	m_ProcessState = eProcessTearDown;

  COsQueue::DeleteAllProcessQueues(GetProcessType());
  if (!m_selfKill)
  {

    if (m_pManagerApi)
    {

      m_pManagerApi->SyncDestroy();
      //SystemSleep(200, FALSE);
    }
  }
  POBJDELETE(m_pManagerApi);
  //SystemSleep(100, FALSE);

  COsTask::DestroyTaskKey();
  CloseOtherProcessQueue();
  DeleteAllSemphores();
  TearDownProcess();

  PDELETE(m_SystemState);
  PDELETE(m_OpcodeStringConverter);
  PDELETE(m_SysConfig);
  PDELETE(m_ServiceConfigList);

  PDELETE(m_StatusStringConverter);
  PDELETE(pCProcessSettings);
  CStringsMaps::CleanUp();

  if (UsingSockets())
    SystemCleanupSocket();



  RemoveSemaphore(m_TasksSemaphoreId);
  m_TasksSemaphoreId = -1;

  return 0;
}

const COsQueue* CProcessBase::GetOtherProcessQueue(eProcessType processType,
                                                   eOtherProcessQueueEntry queueType) const
{
	//VNGFE-7067 Added < in "processType <= eProcessTypeInvalid" to prevent core dump
	if (processType <= eProcessTypeInvalid || processType >= NUM_OF_PROCESS_TYPES)
	{
		return NULL;
	}
	//this function should look in the other process handles table,
	// and return the handle for the other process dispatcher.
	const char * taskName = InfrastructuresTaskNames[queueType];

    BOOL non_blocking = TRUE; // by default write queues are non_blocked

    // when sending XML response to apache, the queues are blocked
    // since XML response are so big,
    // and non blocking queues can't support unlimited size.
    if (processType == eProcessApacheModule)
        non_blocking = FALSE;

    // all messages send from Failover process can be very large
    // all its write queue are blocked
    if (m_pCurrentProcess->GetProcessType() == eProcessFailover)
        non_blocking = FALSE;

    if (m_otherProcessQueues[processType][queueType] == NULL)
	{
		const char * taskName = InfrastructuresTaskNames[queueType];
		COsQueue *writeQueue = new COsQueue;
		writeQueue->CreateWrite(processType,
                                taskName,
                                non_blocking,
                                GetTaskMbxSndBufferSize());
		m_otherProcessQueues[processType][queueType] = writeQueue;
	}
	else
	{
		if (!(m_otherProcessQueues[processType][queueType]->IsValid()))
		{
			m_otherProcessQueues[processType][queueType]->CreateWrite(processType,
                                                                      taskName,
                                                                      non_blocking,
                                                                      GetTaskMbxSndBufferSize());
		}
	}

	return m_otherProcessQueues[processType][queueType];
}

void CProcessBase::CloseOtherProcessQueue(eProcessType otherProcessType)
{
	if (otherProcessType == eProcessTypeInvalid)
	{ // close all entries in the table
		for (int i=0;i<NUM_OF_PROCESS_TYPES; i++)
		{
			for (int j=0;j<NUM_OF_HANDLES_IN_ENTRY; j++)
			{
				if (m_otherProcessQueues[i][j] != NULL)
				{
					m_otherProcessQueues[i][j]->Delete();
					POBJDELETE(m_otherProcessQueues[i][j]);
				}
			}
		}
	}
	else
	{ // close only one specific entry in the table
		for (int j = 0; j < NUM_OF_HANDLES_IN_ENTRY; j++)
		{
			if (m_otherProcessQueues[otherProcessType][j] != NULL)
			{
				m_otherProcessQueues[otherProcessType][j]->Delete();
				POBJDELETE(m_otherProcessQueues[otherProcessType][j]);
			}
		}
	}
}

void CProcessBase::ParseArgv()
{
    if(2 > m_Argc)
        return;

    const char *strNumStartup = m_Argv[1];
    m_NumOfStartups = atoi(strNumStartup);
}

const char* CProcessBase::GetProcessName(eProcessType type)
{
  return ProcessTypeToStr(type);
}

CTaskApp* CProcessBase::GetCurrentTask() const
{
    return COsTask::GetTaskApp();
}

int CProcessBase::Add(CTaskApp* pTask)
{
  LockSemaphore(m_TasksSemaphoreId);
  {
    m_pTasks->push_back(pTask);
  }
  UnlockSemaphore(m_TasksSemaphoreId);
  return 0;
}

int CProcessBase::Cancel(CTaskApp* pTask)
{
  LockSemaphore(m_TasksSemaphoreId);
  {
    std::vector<CTaskApp*>::iterator itr =
        std::find(m_pTasks->begin(), m_pTasks->end(), pTask);
    m_pTasks->erase(itr);
    if(m_pManagerApi)
    {
    	if(m_pManagerApi->GetTaskAppPtr() == pTask)
    	{
    		POBJDELETE(m_pManagerApi);
    	}
    }
  }
  UnlockSemaphore(m_TasksSemaphoreId);
  return 0;
}

int CProcessBase::GetNumOfTasks() const
{
    int num = 0;
    LockSemaphore(m_TasksSemaphoreId);
    {
        num = m_pTasks->size();
    }

    UnlockSemaphore(m_TasksSemaphoreId);
    return num;
}

STATUS CProcessBase::ForEachTask(void (CTaskApp::*pfunc)(std::ostream&) const,
                                 std::ostream& ans) const
{
  LockSemaphore(m_TasksSemaphoreId);

  std::vector<CTaskApp*>::const_iterator it;
  for (it = m_pTasks->begin(); it != m_pTasks->end(); ++it)
  {
    // Obtains tasks in self killed mode
    if (!(*it)->GetSelfKill())
      ((*it)->*pfunc)(ans);
  }

  UnlockSemaphore(m_TasksSemaphoreId);
  return STATUS_OK;
}

STATUS CProcessBase::DumpTasks(std::ostream& ans)
{
  return ForEachTask(&CTaskApp::Dump, ans);
}

STATUS CProcessBase::DumpTasksQueues(std::ostream& ans)
{
  return ForEachTask(&CTaskApp::DumpQueues, ans);
}

STATUS CProcessBase::DumpTasksTimers(std::ostream& ans)
{
  return ForEachTask(&CTaskApp::DumpTimers, ans);
}

STATUS CProcessBase::DumpTasksStateMachines(std::ostream& ans)
{
  return ForEachTask(&CTaskApp::DumpStateMachines, ans);
}

STATUS CProcessBase::DumpTasksOpcodeTail(std::ostream& answer,
                                         DWORD numOfMessages)
{
  LockSemaphore(m_TasksSemaphoreId);
  {
    std::vector<CTaskApp*>::iterator iTer = m_pTasks->begin();
    std::vector<CTaskApp*>::iterator iEnd = m_pTasks->end();
    for (; iTer != iEnd; iTer++)
    {
      CTaskApp *pCurrentTask = *iTer;
      DWORD len = pCurrentTask->GetOpcodeTailLen();
      if (len == 0)
        continue;

      answer << pCurrentTask->NameOf() << "\n";
      pCurrentTask->DumpOpcodeTail(answer, numOfMessages);
      answer << "\n";
    }
  }
  UnlockSemaphore(m_TasksSemaphoreId);

  return STATUS_OK;
}

STATUS CProcessBase::SendMessageToAlarmTasks(OPCODE opcode, CSegment *pSeg)
{
  LockSemaphore(m_TasksSemaphoreId);

  std::vector<CTaskApp*>::iterator iTer = m_pTasks->begin();
  std::vector<CTaskApp*>::iterator iEnd = m_pTasks->end();
  for (; iTer != iEnd; iTer++)
  {
    CTaskApp *pCurrentTask = (*iTer);
    CAlarmableTask *alarmTask = dynamic_cast<CAlarmableTask*> (pCurrentTask);
    if (NULL == alarmTask)
      continue;

    CTaskApi api;
    api.CreateOnlyApi(pCurrentTask->GetRcvMbx());

    CSegment *pCurrentSeg = (NULL != pSeg ? new CSegment(*pSeg) : NULL);
    api.SendMsg(pCurrentSeg, opcode);
  }

  UnlockSemaphore(m_TasksSemaphoreId);
  PDELETE(pSeg);

  return STATUS_OK;
}

// This function is for debugging only
void CProcessBase::SendFileAsXML(char *fileName)
{
	const COsQueue * monitorQueue =
		GetOtherProcessQueue(GetProcessType(),eMonitor);

	((COsQueue *)monitorQueue)->m_process = (eProcessType) -1;

	const COsQueue * managerQueue =
		GetOtherProcessQueue(GetProcessType(),eManager);

	((COsQueue *)managerQueue)->m_process = (eProcessType) -1;

	CSegment *test = new CSegment;

	FILE * xmlFile = NULL;
	xmlFile  = fopen( fileName, "r" );
	if (xmlFile)
	{
		char * fileBuffer = new char[1024*16]; // 16k maximum size
		int numread = fread( fileBuffer, sizeof( char ), 1024*16, xmlFile );
		fileBuffer[numread] = 0;

		COsQueue stubDualMbx; // mailbox of the dual task (tx)

		stubDualMbx.Serialize(*test);
		*test << (DWORD) numread;
		*test << fileBuffer;
		*test << (WORD) GUEST; // authorization
		CTaskApi api;
		if (strstr(fileBuffer,"<GET"))
			api.CreateOnlyApi(*monitorQueue); // send GET request XML buffer to monitor task
		else
			api.CreateOnlyApi(*managerQueue); // send SET request XML buffer to manager task


		STATUS res =  api.SendMsg(test,XML_REQUEST);

		fclose(xmlFile);
		delete [] fileBuffer;
	}
	else
	{
		PASSERTMSG(100, "CProcessBase::SendFileAsXML failed open file");
		PDELETE (test);
	}
}

STATUS CProcessBase::DumpStatistics(std::ostream& answer, CTerminalCommand & command)
{
	answer.setf(std::ios::left,std::ios::adjustfield);

	answer << std::setw(25) << "Message sent" 			<< " : " << m_numMessageSent 	<< endl;
	answer << std::setw(25) << "Message received" 		<< " : " << m_numMsgRcv 		<< endl;
	answer << std::setw(25) << "Sync message timed out" << " : " << m_numToutSyncMsg 	<< endl;
	answer << std::setw(25) << "Local message sent" 	<< " : " << m_numLocalMsgSent 	<< endl;
	answer << std::setw(25) << "Local message received" << " : " << m_numLocalMsgRvc 	<< endl;
	answer << std::setw(25) << "Expired timers" 		<< " : " << m_numExpiredTimers 	<< endl;
	answer << std::setw(25) << "All Traces " 			<< " : " << m_numTrace 		<< endl;
	answer << std::setw(25) << "Traces NOT sent"		<< " : " << m_numTraceNotSent	<< endl;
	for (MsgsSentConuterMapItr itr = m_msgsSentConuterMap.begin(); itr != m_msgsSentConuterMap.end(); ++itr)
	{
		answer << std::setw(25) << "Message sent of Type OpCode "<< GetOpcodeAsString(itr->first) << " : # " << itr->second << endl;
	}
	std::string strProcName = GetProcessName(GetProcessType());
	COstrStream ostrProcMemory;
	DumpProccessMemoryState(ostrProcMemory);
    answer <<  strProcName << " Statistics Memory Report"  << ostrProcMemory.str()<< endl;

    COstrStream ostrProcCPUUsage;
    DumpProccessCPUUsagePercentage(ostrProcCPUUsage);
    answer << strProcName << " Statistics CPU Usage Report " << ostrProcCPUUsage.str() <<endl;

	COstrStream ostrTasksQueues;
	DumpTasksQueues(ostrTasksQueues);
	answer << ostrTasksQueues.str() << endl;

    char *argv [] = {"Bin/McuCmd", "stat", (char*)GetProcessName(GetProcessType()),(char*)GetProcessName(GetProcessType())};
    int argc = 4; //move all params so we will not get all Proccesses data from daemon
    CTerminalCommand command1("terminal_name", argv, argc);

	DumpProcessStatistics(answer, command1);

	return STATUS_OK;
}

void CProcessBase::DumpProccessCPUUsagePercentage(std::ostream& answer)
{
    TICKS selfUser, selfSystem, childUser, childSystem;
    TICKS total;
    pid_t pid = getpid();
    GetSelfCpuUsage(selfUser, selfSystem);
    GetChildrenCpuUsage(childUser, childSystem);
    total = selfUser + selfSystem + childUser + childSystem;

    answer << "PID: " << pid << " User: " <<  selfUser+childUser << " System: " << selfSystem+childSystem << " Total: " << total;
}

void CProcessBase::DumpProccessMemoryState(std::ostream& answer)
{
    int MaxAvailableMemory = GetProcessAddressSpace();
    int UsedMemory = GetUsedMemory(TRUE);
    if (MaxAvailableMemory != 0)
    {
      //Used memory is returned in Kilobytes , and MaxAvailableMemory is in bytes.
      //Multiplied by 100 to get percentage.
      int used_precent = (int) ((double) 100 * (UsedMemory * 1024)
          / MaxAvailableMemory);

      answer << "Limit: " << MaxAvailableMemory << " Usage: " << UsedMemory
          * 1024 << " used: " << used_precent << "%" << std::endl;

    }
    else
    {
      answer << "Limit: unlimited" << " Usage: " << UsedMemory * 1024
          << std::endl;

    }
}

void CProcessBase::SetListenSocketTask(CListenSocket* listenSocket)
{
	m_pListenSocketTask = listenSocket;
}

CListenSocket* CProcessBase::GetListenSocketTask()
{
	return m_pListenSocketTask;
}

const std::string& CProcessBase::GetOpcodeAsString(const OPCODE opcode)const
{
	return m_OpcodeStringConverter->GetStringByOpcode(opcode);
}

const std::string& CProcessBase::GetStatusAsString(const STATUS status)const
{
	return m_StatusStringConverter->GetStringByStatus(status);
}

void CProcessBase::SerializeApiStatuses(CXMLDOMElement *pLanguageNode)
{
	m_StatusStringConverter->SerializeApiStatuses(pLanguageNode);
}

CFilterTraceContainer* CProcessBase::GetTraceFilterContainer() const
{
  return m_FilterTraceContainer;
}


eLogLevel CProcessBase::GetMaxLogLevel() const
{
  // Reads current level from trace filter.
  const CFilterTraceContainer* container = GetTraceFilterContainer();
  PASSERT_AND_RETURN_VALUE(NULL == container, eLevelInfoNormal);

  const CFilterTrace* filter = container->GetFilterByName(CFilterByLevel::GetCompileType());
  PASSERT_AND_RETURN_VALUE(NULL == filter, eLevelInfoNormal);

  const CFilterByLevel& obj = reinterpret_cast<const CFilterByLevel&>(*filter);

  unsigned int level = obj.GetMaxLevel();
  PASSERT_AND_RETURN_VALUE(NUM_OF_TRACE_TYPES == level, eLevelInfoNormal);

  return static_cast<eLogLevel>(level);
}

// It's not case sensitive
eProcessType CProcessBase::GetProcessValueByString(const char *processName)
{
  string wantedProcessName = processName;
  CObjString::ToUpper((char*) (wantedProcessName.c_str()));
  std::string currentProcessName;

  for (int i = 0; i < NUM_OF_PROCESS_TYPES; i++)
  {
    currentProcessName = ProcessNames[i];
    CObjString::ToUpper((char*) (currentProcessName.c_str()));

    if (currentProcessName == wantedProcessName)
    {
      return static_cast<eProcessType> (i);
    }
  }
  return eProcessTypeInvalid;
}

BOOL CProcessBase::GetTestFlag(int boardId)const
{
	return m_testFlag[boardId];
}

void CProcessBase::SetTestFlag(BOOL flagVal,int boardId)
{
	m_testFlag[boardId-1] = flagVal;
}

BOOL CProcessBase::GetTestSignalFlag() const
{
	return m_testSignalFlag;
}

void CProcessBase::SetTestSignalFlag(BOOL flagVal)
{
	m_testSignalFlag = flagVal;
}

CSysConfig* CProcessBase::GetSysConfig()const
{
	return m_SysConfig;
}

void CProcessBase::SetSysConfig(CSysConfig *sysConfig)
{
	m_SysConfig = sysConfig;
}

CServiceConfigList* CProcessBase::GetServiceConfigList()const
{
	return m_ServiceConfigList;
}

void CProcessBase::SetServiceConfigList(CServiceConfigList *serviceConfigList)
{
	POBJDELETE(m_ServiceConfigList);
	m_ServiceConfigList = serviceConfigList;
}

STATUS CProcessBase::CreateAllSemaphores()
{
    std::string PrefixSemName= GetProcessName(GetProcessType());
    PrefixSemName+=  "_TaskGroup";
    const char * groupNames[TASK_GROUPS_NUM] =
        {"Low","Regular"/*,"High","Critical"*/};

    for (int i=0 ; i<TASK_GROUPS_NUM; i++)
    {
        string semName = PrefixSemName+groupNames[i];
        STATUS stat = CreateSemaphore(&m_taskGroupSemaphore[i], semName);
        if (STATUS_OK != stat)
            return STATUS_FAIL;
    }

    return STATUS_OK;
}

STATUS CProcessBase::DeleteAllSemphores()
{
    eProcessType CurrentProcess= GetProcessType();
    for (int i=0;i<TASK_GROUPS_NUM;i++)
    {
        STATUS stat = RemoveSemaphore(m_taskGroupSemaphore[i]);
        if (STATUS_OK != stat)
            return STATUS_FAIL;
        m_taskGroupSemaphore[i] = -1;
    }
    return STATUS_OK;

}

eProcessStatus CProcessBase::GetProcessStatus()
{
	return m_ProcessState;
}

void CProcessBase::SetProcessStatus(eProcessStatus state)
{
	CFaultList emptyFaultList;
	SetProcessStatus(state, &emptyFaultList);
}

void CProcessBase::SetProcessStatus(eProcessStatus newState, CFaultList *faultList)
{
	TRACEINTO << "\nProcess State Changed : " 	<< GetProcessStatusName(m_ProcessState)
												<< " -> "
												<< GetProcessStatusName(newState);

	STATUS status = STATUS_OK;
	if(eProcessIdle == m_ProcessState && eProcessIdle != newState)
	{
		status = SendStartupEventToManager();
		PASSERTMSG(STATUS_OK !=	status, "FAILED to send startup event to manager");
	}
	m_ProcessState = newState;

	status = SendStateChangeIndToMcuMngr(faultList);
}

eMcuState CProcessBase::GetSystemState()
{
	eMcuState state = eMcuState_Invalid;

	PASSERT(!m_SystemState);
	if(m_SystemState != NULL)
	   state = m_SystemState->Get();

	return state;
}

void CProcessBase::SetSystemState(const string &caller, eMcuState state)
{
    if(caller != "SetSecureMcuState")
    {
        string message = "CProcessBase::SetSystemState - Bad caller : ";
        message += caller;
        PASSERTMSG(TRUE, message.c_str());
        return;
    }

	eProcessType type = GetProcessType();
	if(eProcessMcuMngr != type)
	{
		PASSERTMSG(1, "CProcessBase::SetSystemState - Only McuMngr can change the System State");
		return;
	}

	m_SystemState->Set(state);

}

STATUS CProcessBase::SendProcessSetupToDeamon()
{
    CSegment *pSeg = new CSegment;
    *pSeg << 	(DWORD)GetProcessType();

	STATUS status = SendMessageToManager(eProcessMcmsDaemon, pSeg, PROCESS_UP);

	return status;
}

STATUS CProcessBase::SendStartupEventToManager()
{
	STATUS status = SendMessageToManager(GetProcessType(), NULL, STARTUP_EVENT);

	return status;
}

STATUS CProcessBase::SendStateChangeIndToMcuMngr(CFaultList *faultList)
{
	CProcessStateFaultList ProcessStateFaultList(GetProcessType(),
                                                 m_ProcessState,
                                                 faultList->Clone());
	CSegment *pSeg = new CSegment;
	ProcessStateFaultList.Serialize(*pSeg);

	STATUS status = SendMessageToManager(eProcessMcuMngr, pSeg, TO_MCUMNGR_STATE_CHANGED);
	if(status == STATUS_OK)
		m_isFaultsSentMcuMngr = TRUE;
	return status;
}

STATUS CProcessBase::SendMessageToManager(eProcessType processDest, CSegment *pSeg, OPCODE opcode)
{
	CManagerApi api(processDest);
    STATUS status = api.SendMsg(pSeg, opcode);
    api.DestroyOnlyApi();

	return status;
}

void CProcessBase::RemoveBrokenQueue(eProcessType processType)
{
	int i;
	for (i = 0; i < NUM_OF_HANDLES_IN_ENTRY; i++)
	{
		if (m_otherProcessQueues[processType][i] != NULL)
		{
			if (m_otherProcessQueues[processType][i]->IsValid())
				m_otherProcessQueues[processType][i]->Delete();
			POBJDELETE(m_otherProcessQueues[processType][i]);
		}
	}
}

eOtherProcessQueueEntry CProcessBase::FindQueueEntry(eProcessType processType,const COsQueue &queue) const
{
    int i;
    for ( i = 0 ; i<NUM_OF_HANDLES_IN_ENTRY; i++)
    {
        if ( m_otherProcessQueues[processType][i] != NULL)
        {
            const COsQueue & tmpQueue = *(m_otherProcessQueues[processType][i]);
            if (tmpQueue.m_id      == queue.m_id      &&
                tmpQueue.m_idType  == queue.m_idType  &&
                tmpQueue.m_process == queue.m_process &&
                tmpQueue.m_scope   == queue.m_scope   )
            {
                return (eOtherProcessQueueEntry)i;
            }

        }
    }

    return (eOtherProcessQueueEntry)-1;
}

void CProcessBase::AddOpcodeString(OPCODE opcode, const char *str)
{
	m_OpcodeStringConverter->AddOpcodeString(opcode, str);
}

void CProcessBase::AddStatusString(STATUS status, const char *str)
{
	m_StatusStringConverter->AddStatusString(status, str);
}

void CProcessBase::FindLiveProcesses(std::ostream& answer)
{
	for(int i = 0 ; i < NUM_OF_PROCESS_TYPES ; i++)
	{
		eProcessType processType = (eProcessType)i;
		if(YES == IsProcessAlive(processType))
		{
			const char *processName = ProcessNames[i];
			answer << processName << std::endl;
		}
	}
}

BOOL CProcessBase::IsProcessAlive(eProcessType processType)
{
  if (processType == eProcessTypeInvalid)
      return NO;
  CManagerApi api(processType);
  STATUS stat = api.SendOpcodeMsg(MANAGER_ALIVE);
  if (stat == STATUS_OK || stat == STATUS_SOCKET_WOULD_BLOCKED)
    return YES;

  return NO;
}

void CProcessBase::SetProcessAddressSpaceLimit()
{
  m_isUnderValgrid = IsUnderValgrind(GetProcessName(GetProcessType()));

  if (IsTarget() && m_isUnderValgrid == FALSE)
  {
    int size = (this->GetProcessAddressSpace());
    if (size > 0)
    {
      SetAddressSpaceLimit(size);
    }
  }
}

DWORD CProcessBase::AddActiveAlarmFromProcess( BYTE subject,
                                               DWORD errorCode,
                                               BYTE errorLevel,
                                               string description,
                                               bool isForEma,
                                               bool isForFaults,
                                               DWORD userId,
                                               DWORD boardId,
                                               DWORD unitId,
                                               WORD theType)
{
	DWORD faultId = 0;

	CTaskApp *theTask = GetCurrentTask();
	CAlarmableTask *alrmbleTask = dynamic_cast<CAlarmableTask*>(theTask);
	if (alrmbleTask)
	{
		faultId = alrmbleTask->AddActiveAlarm(subject,
                                              errorCode,
                                              errorLevel,
                                              description,
                                              isForEma,
                                              isForFaults,
                                              userId,
                                              boardId,
                                              unitId,
                                              theType);
	}
	else
	{
		PASSERT(1);
	}

	return faultId;
}

DWORD CProcessBase::AddActiveAlarmSingleToneFromProcess( BYTE subject,
                                               DWORD errorCode,
                                               BYTE errorLevel,
                                               string description,
                                               bool isForEma,
                                               bool isForFaults,
                                               DWORD userId,
                                               DWORD boardId,
                                               DWORD unitId,
                                               WORD theType)
{
	DWORD faultId = 0;

	CTaskApp *theTask = GetCurrentTask();
	CAlarmableTask *alrmbleTask = dynamic_cast<CAlarmableTask*>(theTask);
	if (alrmbleTask)
	{
		faultId = alrmbleTask->AddActiveAlarmSingleton(subject,
                                              errorCode,
                                              errorLevel,
                                              description,
                                              isForEma,
                                              isForFaults,
                                              userId,
                                              boardId,
                                              unitId,
                                              theType);
	}
	else
	{
		PASSERT(1);
	}

	return faultId;
}

void CProcessBase::RemoveActiveAlarmByErrorCodeUserIdFromProcess(DWORD errorCode,DWORD Id)
{
	CTaskApp *theTask = GetCurrentTask();
	CAlarmableTask *alrmbleTask = dynamic_cast<CAlarmableTask*>(theTask);
	if (alrmbleTask)
	{
		alrmbleTask->RemoveActiveAlarmByErrorCodeUserId(errorCode,Id);
	}
	else
	{
		PASSERT(1);
	}
}

void CProcessBase::RemoveActiveAlarmFromProcess(DWORD errorCode)
{
	CTaskApp *theTask = GetCurrentTask();
	CAlarmableTask *alrmbleTask = dynamic_cast<CAlarmableTask*>(theTask);
	if (alrmbleTask)
	{
		alrmbleTask->RemoveActiveAlarmByErrorCode(errorCode);
	}
	else
	{
		PASSERT(1);
	}
}

DWORD CProcessBase::AddActiveAlarmFaultOnlyToProcess(BYTE subject,
                                                       DWORD errorCode,
                                                       BYTE errorLevel,
                                                       std::string description,
                                                       DWORD userId ,
                                                       DWORD boardId,
                                                       DWORD unitId ,
                                                       WORD theType )
{
	DWORD faultId = 0;

	CTaskApp *theTask = GetCurrentTask();
	CAlarmableTask *alrmbleTask = dynamic_cast<CAlarmableTask*>(theTask);
	if (alrmbleTask)
	{
		faultId = alrmbleTask->AddActiveAlarmFaultOnly(subject,
														  errorCode,
														  errorLevel,
														  description,
														  userId,
														  boardId,
														  unitId,
														  theType);
	}
	else
	{
		PASSERT(1);
	}

	return faultId;
}

DWORD	CProcessBase::AddActiveAlarmFaultOnlySingleToneToProcess(BYTE subject,
                                                                 DWORD errorCode,
                                                                 BYTE errorLevel,
                                                                 std::string description,
                                                                 DWORD userId,
                                                                 DWORD boardId ,
                                                                 DWORD unitId,
                                                                 WORD theType)
{
	DWORD faultId = 0;

	CTaskApp *theTask = GetCurrentTask();
	CAlarmableTask *alrmbleTask = dynamic_cast<CAlarmableTask*>(theTask);
	if (alrmbleTask)
	{
		faultId = alrmbleTask->AddActiveAlarmFaultOnlySingleton(subject,
                                              errorCode,
                                              errorLevel,
                                              description,
                                              userId,
                                              boardId,
                                              unitId,
                                              theType);
	}
	else
	{
		PASSERT(1);
	}

	return faultId;

}

void CProcessBase::RemoveActiveAlarmFaultOnlyFromProcess(DWORD errorCode)
{
	CTaskApp *theTask = GetCurrentTask();
	CAlarmableTask *alrmbleTask = dynamic_cast<CAlarmableTask*>(theTask);
	if (alrmbleTask)
	{
		alrmbleTask->RemoveActiveAlarmFaultOnlyByErrorCode(errorCode);
	}
	else
	{
		PASSERT(1);
	}
}

void CProcessBase::RemoveActiveAlarmFaultOnlyByErrorCodeUserIdFromProcess(DWORD errorCode,
                                                                           DWORD Id)
{
	CTaskApp *theTask = GetCurrentTask();
	CAlarmableTask *alrmbleTask = dynamic_cast<CAlarmableTask*>(theTask);
	if (alrmbleTask)
	{
		alrmbleTask->RemoveActiveAlarmFaultOnlyByErrorCodeUserId(errorCode,Id);
	}
	else
	{
		PASSERT(1);
	}
}

// Virtual
int CProcessBase::GetProcessAddressSpace()
{
  return 48 * 1024 * 1024;
}

eStringValidityStatus CProcessBase::TestStringValidity(char* theString, const int maxLength, const string &sCaller, bool isAssert)
{
    // ===== 1. check characters' validity
    eStringValidityStatus retStat = CObjString::IsLegalAsciiString(theString, maxLength);
	if (eStringValid != retStat)
	{
		// ===== 2. replace string
		memset(theString, 0, maxLength);
		strncpy(theString, "Invalid string", maxLength - 1);
		theString[maxLength - 1] = '\0';

		// ===== 3. produce an ASSERT
        if(isAssert)
        {
            string errStr = sCaller;
            errStr += " - Invalid string. Problem: ";
            errStr += GetStringValidityStatus(retStat);

            FPASSERTMSG(1, errStr.c_str());
        }
	}

	return retStat;
}

eProductType CProcessBase::GetProductType() const
{
    eProductType prodType = eProductTypeRMX2000;
    if (m_productTypeLastFound == eProductTypeUnknown)
    {
    	std::string fname = MCU_MCMS_DIR+"/ProductType";
        FILE *pProductTypeFile = fopen(fname.c_str(), "r" );
        if (pProductTypeFile)
        {
            char * line = NULL;
            size_t len = 0;
            ssize_t read;
            read = getline(&line, &len, pProductTypeFile );
            if (read != -1)
            {
                prodType = ::StringToProductType(line);
                if (prodType == eProductTypeUnknown) //read from file and prodtType not analyzed yet so put defaults
                {
                	if (IsTarget())
                	    prodType = eProductTypeRMX2000;
                	else
                	    prodType = eProductTypeRMX4000;
                }
                else //the file was written with the correct ProdcuType
                {
                    m_productTypeLastFound = prodType;
                }

            }
            if (line)
            {
                free(line);
            }

            fclose(pProductTypeFile);
        }
        return prodType;
    }
    return m_productTypeLastFound;
}


///////////////////////////////////////////////////////////////////////////
bool CProcessBase::IsFlexeraLicenseInSysFlag() const
{


    CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
    if (pSysConfig)
    {
    	string licenseString = "";
    	BOOL isFlexeraLicense = NO;

    	isFlexeraLicense = pSysConfig->GetDataByKey("LICENSE_MODE", licenseString);

    	 if (isFlexeraLicense)
    	 {
    		 if ((licenseString.compare("flexera")==0) )
    			 return true;
    		 else
    			 return false;
    	 }
    }

    return false;
}




BOOL CProcessBase::GetLanRedundancy(const eIpType ipType) const
{
	BOOL bLAN_REDUNDANCY = CFG_VALUE_LANREDUNDANCY_DEFAULT;
	BOOL bIsFederal = NO;
	BOOL bIsMultipleServices = NO;
	CSysConfig* cfg = GetSysConfig();
	cfg->GetBOOLDataByKey("LAN_REDUNDANCY", bLAN_REDUNDANCY);
	cfg->GetBOOLDataByKey("ULTRA_SECURE_MODE", bIsFederal);
	cfg->GetBOOLDataByKey("MULTIPLE_SERVICES", bIsMultipleServices);
	if ( bIsFederal == YES || bIsMultipleServices == YES || eIpType_IpV4 != ipType)
	{
		bLAN_REDUNDANCY = NO;
	}

	return bLAN_REDUNDANCY;
}

eProductType CProcessBase::GetLastProductTypeFound() const
{
    return m_productTypeLastFound;
}

eProductFamily CProcessBase::GetProductFamily() const
{
  return ::ProductTypeToProductFamily(GetProductType());
}

/*eSystemCardsMode CProcessBase::GetRmxSystemCardsModeDefault() const
{
	if (eProductFamilySoftMcu == GetProductFamily() && eProductTypeCallGeneratorSoftMCU != GetProductType())
	  return eSystemCardsMode_breeze;

	return eSystemCardsMode_mpmrx;
}*/
//GetRmxSystemCardsModeDefault BRIDGE-6299 NGB the default should be mpmx (it should be change back to the above commented function in v8.3
eSystemCardsMode CProcessBase::GetRmxSystemCardsModeDefault() const
{
	//if (eProductFamilySoftMcu == GetProductFamily() && eProductTypeCallGeneratorSoftMCU != GetProductType())
	 // return eSystemCardsMode_breeze;

	return eSystemCardsMode_mpmrx;
}



void CProcessBase::SetFailoverParams(bool isFeatureEnabled, bool isSlaveMode)
{
	SetIsFailoverFeatureEnabled(isFeatureEnabled);
	SetIsFailoverSlaveMode(isSlaveMode);
}

void CProcessBase::SetIsFailoverFeatureEnabled(bool isEnabled)
{
	m_bIsFailoverFeatureEnabled = isEnabled;
}

bool CProcessBase::GetIsFailoverFeatureEnabled()
{
	return m_bIsFailoverFeatureEnabled;
}

void CProcessBase::SetIsFailoverSlaveMode(bool isSlave)
{
	m_bIsFailoverSlaveMode = isSlave;
}

bool CProcessBase::GetIsFailoverSlaveMode()
{
	return m_bIsFailoverSlaveMode;
}

bool CProcessBase::IsFailoverBlockTransaction_SlaveMode(std::string sAction)
{
	return false;
}

void CProcessBase::IncreaseMsgsSentConuter(OPCODE opcode)
{
	   MsgsSentConuterMap::iterator it=m_msgsSentConuterMap.find(opcode);
	   if (it == m_msgsSentConuterMap.end())
             m_msgsSentConuterMap.insert(std::make_pair(opcode, 1));
       else
             it->second++;
       //            DWORD dwVal = m_msgsSentConuterMap[opcode];
       //            m_msgsSentConuterMap[opcode] = ++dwVal;
}

bool CProcessBase::GetCSLogsStateFromSysCfg()
{
//if cs logs should be started then update member to notify LoggerMonitor
	 string data1;
	 string data2;
	 string data3;
	 CSysConfig* cfg = GetSysConfig();
	 BOOL b1 =  cfg->GetDataByKey("trace1level",data1);
	 BOOL b2 =  cfg->GetDataByKey("trace2level",data2);
	 BOOL b3 =  cfg->GetDataByKey("trace3level",data3);
	 if (b1 && b2 && b3)
	 {
		  if ("all.StackMsgs.logfileDebugLevel.5" == data1 && "all.XmlTrace.logfileDebugLevel.5" == data2 && "siptask.SipMsgsTrace.logfileDebugLevel.5" == data3)
		  {
			  return true;
		  }
		  else
		  {
			  return false;
		  }
	 }
	 return false;
}



void CProcessBase::GenerateStatusXmlFiles(const char* directory)
{
	{
		CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
		pXMLRootElement->set_nodeName("StringConfiguration");

		CXMLDOMElement* pNode = pXMLRootElement->AddChildNode("Translations");
		CXMLDOMElement* pLanguageNode = pNode->AddChildNode("Language");

		CXMLDOMAttribute* pLanguage_Attribute = new CXMLDOMAttribute();
		pLanguage_Attribute->set_nodeName("name");
		pLanguage_Attribute->SetValueForElement("English");
		pLanguageNode->AddAttribute(pLanguage_Attribute);

		SerializeApiStatuses(pLanguageNode);

		char* pszStrConfigListXml;
		DWORD nStrConfigListXmlLen;
		std::string file_name = directory;
		file_name += "/RmxStatusesEnglish.xml";
		FILE* pFileOperDB = fopen(file_name.c_str(),"w");

		if(!pFileOperDB)
		{
			std::cout << "Failed to create XML status file - fopen failed: " << file_name  << std::endl;
			return;
		}

		pXMLRootElement->DumpDataAsStringWithAttribute(&pszStrConfigListXml,&nStrConfigListXmlLen,0,TRUE);

		if(pszStrConfigListXml)
		{
			fprintf(pFileOperDB,"%s",pszStrConfigListXml);
			DEALLOCBUFFER(pszStrConfigListXml);
		}

		int fcloseReturn = fclose(pFileOperDB);
		if (FCLOSE_SUCCESS != fcloseReturn)
			std::cout << "Failed to create XML status file - fclose failed: "  << file_name  << std::endl;
		else
			std::cout  << "The file has been created successfully - "  << file_name  << std::endl;

    }

	{
		CXMLDOMElement* pXMLRootElement =  new CXMLDOMElement;
		pXMLRootElement->set_nodeName("StringConfiguration");

		CXMLDOMElement* pActiveAlarmsListNode = pXMLRootElement->AddChildNode("ActiveAlarmsList");

		CXMLDOMElement* pNode = pActiveAlarmsListNode->AddChildNode("Translations");
		CXMLDOMElement* pAALanguageNode = pNode->AddChildNode("Language");

		CXMLDOMAttribute* pAALanguage_Attribute = new CXMLDOMAttribute();
		pAALanguage_Attribute->set_nodeName("name");
		pAALanguage_Attribute->SetValueForElement("English");
		pAALanguageNode->AddAttribute(pAALanguage_Attribute);

		CXMLDOMElement* pFaultsListNode = pXMLRootElement->AddChildNode("FaultsList");

		pNode = pFaultsListNode->AddChildNode("Translations");
		CXMLDOMElement* pFaultsLanguageNode = pNode->AddChildNode("Language");

		CXMLDOMAttribute* pLanguage_Attribute = new CXMLDOMAttribute();
		pLanguage_Attribute->set_nodeName("name");
		pLanguage_Attribute->SetValueForElement("English");
		pFaultsLanguageNode->AddAttribute(pLanguage_Attribute);

		SerializeFaultsAndActiveAlarms(pAALanguageNode, pFaultsLanguageNode);

		char* pszStrConfigListXml;
		DWORD nStrConfigListXmlLen;
		std::string file_name = directory;
		file_name += "/RmxFaultsAndActiveAlarmsEnglish.xml";
		FILE* pFileOperDB = fopen(file_name.c_str(),"w");

		if(!pFileOperDB)
		{
			std::cout << "Failed to create XML faules and active alarms file - fopen failed: " << file_name << std::endl;
			return;
		}

		pXMLRootElement->DumpDataAsStringWithAttribute(&pszStrConfigListXml,&nStrConfigListXmlLen,0,TRUE);

		if(pszStrConfigListXml)
		{
			fprintf(pFileOperDB,"%s",pszStrConfigListXml);
			DEALLOCBUFFER(pszStrConfigListXml);
		}

		int fcloseReturn = fclose(pFileOperDB);
		if (FCLOSE_SUCCESS != fcloseReturn)
			std::cout << "Failed to create XML status file - fclose failed: " << file_name <<  std::endl;
		else
			std::cout << "The file has been created successfully - " << file_name  << std::endl;

	}
}

void   CProcessBase::initLocalTracer()
{
	 string processName = GetProcessName(GetProcessType());
	 string fullFilepath =OS_STARTUP_LOGS_PATH + processName +".Ind";

	 if(IsFileExists(fullFilepath))
	 {
		 m_enableLocalTracer=TRUE;
	 }

}

/*Begin:added by Richer for BRIDGE-15015, 11/13/2014*/
STATUS CProcessBase::SendLedAlarmEventToConfigMngr(CSegment *pSeg, OPCODE opcode)
{
	STATUS status = SendMessageToManager(eProcessConfigurator, pSeg, opcode);

	return status;
}
  

