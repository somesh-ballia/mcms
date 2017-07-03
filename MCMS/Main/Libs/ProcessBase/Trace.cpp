// Trace.cpp

#include "Trace.h"

#include <execinfo.h>
#include <malloc.h>
#include <sys/time.h>
#include <time.h>

#include "TraceHeader.h"
#include "SystemFunctions.h"
#include "Segment.h"
#include "OsTask.h"
#include "ProcessBase.h"
#include "TaskApp.h"
#include "Macros.h"
#include "TraceClass.h"
#include "HlogApi.h"
#include "ObjString.h"
#include "OpcodesMcmsInternal.h"
#include "ManagerApi.h"
//Boris: please open this include only if you use this option in debugging
//#include "FileLogger.h"

const DWORD MAX_STACK_DEPTH = 20;

void UnitTestAssert(const char* text);
bool IsUnitTest();

// Cuts file name off a path, e.g:
// '/path/to/file/foobar.cpp' will be trimmed to 'foobar.cpp'
static const char* trim_file_name(const char* fname)
{
  char* pos = (char*)strrchr(fname, '/');
  return (NULL != pos) ? pos+1 : fname;
}

void FillTraceHeader(TRACE_HEADER_S& traceHeader,
                     const WORD level,
                     const CPObject* pObj,
                     const char* message1,
                     const char* message2,
                     const DWORD topic_id,
                     const char* terminalName,
                     const DWORD unit_id,
                     const DWORD conf_id,
                     const DWORD party_id,
                     const OPCODE opcode,
                     const char* str_opcode)
{
  static DWORD lMessageCounter = 0;
  lMessageCounter++;   // count each message

  CProcessBase* process = CProcessBase::GetProcess();

  DWORD messageLen = strlen(message1);
  DWORD tmp = 0;
  if (NULL != message2)
  {
    tmp = strlen(message2);
    messageLen += tmp;
  }

  traceHeader.m_level                 = level;
  traceHeader.m_topic_id              = topic_id;
  traceHeader.m_unit_id               = unit_id;
  traceHeader.m_conf_id               = conf_id;
  traceHeader.m_party_id              = party_id;
  traceHeader.m_opcode                = opcode;
  traceHeader.m_processType           =
    (NULL != process ? process->GetProcessType() : eProcessTypeInvalid);
  traceHeader.m_messageLen            = messageLen;
  traceHeader.m_sourceId              = (DWORD)pObj;

  timeval tv;
  gettimeofday(&tv, NULL);
  traceHeader.m_tv_sec            = tv.tv_sec;
  traceHeader.m_tv_usec           = tv.tv_usec;
  traceHeader.m_systemTick            =
    SystemGetTickCount().GetIntegerPartForTrace();
  traceHeader.m_terminalName[0]       = '\0';
  traceHeader.m_taskName[0]           = '\0';
  traceHeader.m_objectName[0]         = '\0';
  traceHeader.m_str_opcode[0]         = '\0';
  traceHeader.m_processMessageNumber  = lMessageCounter;

  const CTaskApp* currentTask =
    (NULL != process ? process->GetCurrentTask() : NULL);
  if (NULL != currentTask)
  {
    strncpy(traceHeader.m_taskName,
            currentTask->GetTaskName(), MAX_OBJECT_NAME_LEN - 1);
    size_t nLen = strlen(traceHeader.m_taskName);
    if (nLen < MAX_OBJECT_NAME_LEN - 11)
    {
    	DWORD dwTaskID = currentTask->GetTaskId();
    	snprintf(traceHeader.m_taskName + nLen, 10, " #%x", dwTaskID);
    }
    traceHeader.m_taskName[MAX_OBJECT_NAME_LEN - 1] = '\0';

    if (DEFAULT_CONF_ID == traceHeader.m_conf_id)
      traceHeader.m_conf_id = currentTask->GetConfId();

    if (DEFAULT_PARTY_ID == traceHeader.m_party_id)
      traceHeader.m_party_id = currentTask->GetPartyId();
  }

  if (NULL != str_opcode)
  {
    strncpy(traceHeader.m_str_opcode, str_opcode, STR_OPCODE_LEN-1);
    traceHeader.m_str_opcode[STR_OPCODE_LEN - 1] = '\0';
  }

  if (0 != traceHeader.m_sourceId)
    if (CPObject::IsValidPObjectPtr(pObj))
    {
      strncpy(traceHeader.m_objectName, pObj->NameOf(), MAX_OBJECT_NAME_LEN-1);
      traceHeader.m_objectName[MAX_OBJECT_NAME_LEN - 1] = '\0';
    }

  if (NULL != terminalName)
  {
    strncpy(traceHeader.m_terminalName, terminalName, MAX_TERMINAL_NAME_LEN-1);
    traceHeader.m_terminalName[MAX_TERMINAL_NAME_LEN - 1] = '\0';
  }
}

void FillMcmsHeader(MCMS_TRACE_HEADER_S& mcmsHeader, const char* file_name,
                    const WORD lineNum)
{
  snprintf(mcmsHeader.m_file_line_number,
           sizeof(mcmsHeader.m_file_line_number),
           "%s(%d)",
           file_name,
           lineNum);
}

void PAssertDebug(const char* fname,
                  const WORD lineNum,
                  const CPObject* pObj,
                  DWORD errCode)
{
  if (TRUE /*GetAssertMode()==DEBUG_MODE */)
    PAssert(fname, lineNum, pObj, errCode, NULL, YES);
  else
    PAssert(fname, lineNum, pObj, errCode, NULL, NO);
}

void PAssert(const char* fname,
             const WORD lineNum,
             const CPObject* pObj,
             DWORD errCode,
             const char* assertMessage,
             BYTE debug)
{

	if (IsUnitTest())
		return;

  fname = trim_file_name(fname);

  if (assertMessage == NULL)
    assertMessage = "";

  CProcessBase* pProcess    = CProcessBase::GetProcess();
  eProcessType  processType = pProcess->GetProcessType();
  const char*   processName = CProcessBase::GetProcessName(processType);

  DWORD taskID = COsTask::GetCurrentTaskId();
  const CTaskApp* currentTask =
    CProcessBase::GetProcess()->GetCurrentTask();

  const char* taskName = (currentTask != NULL) ?
    currentTask->GetTaskName() : "Unknown";

  const char* debugString = (debug == YES) ? "DEBUG-" : "";

  void*  array[MAX_STACK_DEPTH];
  DWORD  size = backtrace(array, MAX_STACK_DEPTH);
  char** strings = backtrace_symbols(array, size);

  // Fix for httpd crash
  if (NULL == strings) {
  	if (!IsTarget())
    		std::cerr << "In Trace.cpp: strings allocation failed" << std::endl;
	return;
  }
  // End of fix

  const char* bt0 = (size > 0 ? strings[0] : "--");
  const char* bt1 = (size > 1 ? strings[1] : "--");
  const char* bt2 = (size > 2 ? strings[2] : "--");
  const char* bt3 = (size > 3 ? strings[3] : "--");
  const char* bt4 = (size > 4 ? strings[4] : "--");
  const char* bt5 = (size > 5 ? strings[5] : "--");
  const char* bt6 = (size > 6 ? strings[6] : "--");
  const char* bt7 = (size > 7 ? strings[7] : "--");
  const char* bt8 = (size > 8 ? strings[8] : "--");
  const char* bt9 = (size > 9 ? strings[9] : "--");

  char buffer[8 * 1024];
  int realLength = snprintf(
    buffer,
    ARRAYSIZE(buffer),
"********************************** EXCEPTION **********************************\n\
%sASSERT Code:%u Process:%s Task:%s TaskID:%u\n\
%s:%1d %s\n\
--\n\
0:%s\n\
1:%s\n\
2:%s\n\
3:%s\n\
4:%s\n\
5:%s\n\
6:%s\n\
7:%s\n\
8:%s\n\
9:%s\n\
*******************************************************************************",
    debugString,
    errCode,
    processName,
    taskName,
    taskID,
    fname,
    lineNum,
    assertMessage,
    bt0,
    bt1,
    bt2,
    bt3,
    bt4,
    bt5,
    bt6,
    bt7,
    bt8,
    bt9
    );

  if (NULL != strings)
    free(strings);

  // Prints on terminal only in simulation
  if (!IsTarget())
    std::cerr << buffer << std::endl;

  OutTraceMessage(fname,
                  lineNum,
                  eLevelError,
                  pObj,
                  buffer,
                  NULL);

  // temp. disable debug asserts calling unittest asserts
  if (!debug)
    UnitTestAssert(buffer);

  // Sends to Faults process
  snprintf(buffer, ARRAYSIZE(buffer), "%sASSERT:%s", debugString, assertMessage);
  CObjString::ReplaceChar(buffer, ' ', '_');
  CHlogApi::SoftwareBug(fname, lineNum, errCode, buffer);

  if (pProcess->m_IsTreatingOnAssert == 0)
  {
    pProcess->m_IsTreatingOnAssert++;
    CManagerApi api(processType);
    api.SendMsg(NULL, ON_ASSERT_IND);
  }
}

void OutTraceOpcode(const char* file_name,
                    const WORD lineNum,
                    const WORD level,
                    const CPObject* pObj,
                    const char* message1,
                    const OPCODE opcode,
                    const char* strOpcode)
{
  OutTraceMessage(file_name,
                  lineNum,
                  level,
                  pObj,
                  message1,
                  NULL,
                  DEFAULT_TOPIC_ID,
                  NULL,
                  DEFAULT_UNIT_ID,
                  DEFAULT_CONF_ID,
                  DEFAULT_PARTY_ID,
                  opcode,
                  strOpcode);
}

void OutTraceMessage(const char* fname,
                     const WORD lineNum,
                     const WORD level,
                     const CPObject* pObj,
                     const char* message1,
                     const char* message2,
                     const DWORD topic_id,
                     const char* terminalName,
                     const DWORD unit_id,
                     const DWORD conf_id,
                     const DWORD party_id,
                     const OPCODE opcode,
                     const char* str_opcode)
{
  CProcessBase* process = CProcessBase::GetProcess();
  if (NULL == process)
    // it happens when a trace is sent in a TDD context.
    return;

  TRACE_HEADER_S traceHeader;
  memset(&traceHeader, 0, sizeof traceHeader);

  FillTraceHeader(traceHeader,
                  level,
                  pObj,
                  message1,
                  message2,
                  topic_id,
                  terminalName,
                  unit_id,
                  conf_id,
                  party_id,
                  opcode,
                  str_opcode);

  MCMS_TRACE_HEADER_S mcmsHeader;
  memset(&mcmsHeader, 0, sizeof mcmsHeader);

  FillMcmsHeader(mcmsHeader, trim_file_name(fname), lineNum);

  std::string data = message1;
  if (NULL != message2)
    data += message2;

//  if (traceHeader.m_processType == eProcessConfParty || traceHeader.m_processType == eProcessResource)
//	  FILE_LOGGER.OutMessage(traceHeader, mcmsHeader, data);

  CTrace::OutMessage(traceHeader, mcmsHeader, data);
}

void OutTraceMessageD(const char* file_name,
                      const WORD lineNum,
                      const WORD level,
                      const CPObject* pObj,
                      const char* message,
                      DWORD status)
{
  DWORD strLen1 = strlen(message);
  if (!strLen1) return;

  char* seg = (char*)malloc(strLen1+20);
  if (!seg) return;

  sprintf(seg, "%s%d", message, status);
  OutTraceMessage(file_name,
                  lineNum,
                  level,
                  pObj,
                  seg,
                  NULL);
  free(seg);
}

void OutTraceMessageH(const char* file_name,
                      const WORD lineNum,
                      const WORD level,
                      const CPObject* pObj,
                      const char* message,
                      DWORD status)
{
  DWORD strLen1 = strlen(message);
  if (!strLen1)
    return;

  char* seg = (char*)malloc(strLen1+20);
  if (!seg) return;

  sprintf(seg, "%s0x%x", message, status);
  OutTraceMessage(file_name,
                  lineNum,
                  level,
                  pObj,
                  seg,
                  NULL);
  free(seg);
}

void OutTraceCommandAnswer(const char* file_name,
                           const WORD lineNum,
                           const CPObject* pObj,
                           const char* terminalName,
                           const char* commandName,
                           const char* answer)
{
  std::string buff = "Command : ";
  buff += commandName;
  buff += "   Process: ";
  buff += answer;
  buff += '\n';

  OutTraceMessage(file_name,
                  lineNum,
                  eLevelInfoNormal,
                  pObj,
                  buff.c_str(),
                  NULL,
                  DEFAULT_TOPIC_ID,
                  terminalName);
}

/* colinzuo 2011/06/21: back stack trace, mainly for code study purpose */
void PBtTrace(const char* fileName,
              const WORD lineNum,
              const CPObject* pObj,
              const char* traceMessage)
{
  char* fname;
  if (traceMessage == NULL)
    traceMessage = "";

  fname = (char*)strrchr(fileName, '\\');
  if (fname == NULL)
    fname = (char*) fileName;
  else
    fname++;

  CProcessBase* pProcess    = CProcessBase::GetProcess();
  eProcessType  processType = pProcess->GetProcessType();
  const char*   processName = CProcessBase::GetProcessName(processType);

  DWORD taskID = COsTask::GetCurrentTaskId();

  const CTaskApp* currentTask =
    CProcessBase::GetProcess()->GetCurrentTask();

  const char* taskName = "Unknown";
  if (currentTask != NULL)
    taskName = currentTask->GetTaskName();

  void*  array[MAX_STACK_DEPTH];
  char** strings = NULL;
  DWORD  size = backtrace(array, MAX_STACK_DEPTH);
  strings = backtrace_symbols(array, size);

  const char* btsymbol0 = (size > 0 ? strings[0] : "--");
  const char* btsymbol1 = (size > 1 ? strings[1] : "--");
  const char* btsymbol2 = (size > 2 ? strings[2] : "--");
  const char* btsymbol3 = (size > 3 ? strings[3] : "--");
  const char* btsymbol4 = (size > 4 ? strings[4] : "--");
  const char* btsymbol5 = (size > 5 ? strings[5] : "--");
  const char* btsymbol6 = (size > 6 ? strings[6] : "--");
  const char* btsymbol7 = (size > 7 ? strings[7] : "--");
  const char* btsymbol8 = (size > 8 ? strings[8] : "--");
  const char* btsymbol9 = (size > 9 ? strings[9] : "--");
  const char* btsymbol10 = (size > 10 ? strings[10] : "--");
  const char* btsymbol11 = (size > 11 ? strings[11] : "--");
  const char* btsymbol12 = (size > 12 ? strings[12] : "--");
  const char* btsymbol13 = (size > 13 ? strings[13] : "--");
  const char* btsymbol14 = (size > 14 ? strings[14] : "--");

  ALLOCBUFFER(buffer, 7 * 1024);

  int realLength = sprintf(
    buffer,
    "\n**************************** Back Stack Trace ********************************\n\
Process Name: %s, Task ID: %u, Task Name: %s\n\
%s:%1d:  %s\n\
------- 15 frames are obtained-------\n\
0:%s\n\
1:%s\n\
2:%s\n\
3:%s\n\
4:%s\n\
5:%s\n\
6:%s\n\
7:%s\n\
8:%s\n\
9:%s\n\
10:%s\n\
11:%s\n\
12:%s\n\
13:%s\n\
14:%s\n\
******************************************************************************\n",
    processName,
    taskID,
    taskName,
    fname,
    lineNum,
    traceMessage,
    btsymbol0,
    btsymbol1,
    btsymbol2,
    btsymbol3,
    btsymbol4,
    btsymbol5,
    btsymbol6,
    btsymbol7,
    btsymbol8,
    btsymbol9,
    btsymbol10,
    btsymbol11,
    btsymbol12,
    btsymbol13,
    btsymbol14
    );

  OutTraceMessage(fileName,
                  lineNum,
                  eLevelInfoNormal,
                  pObj,
                  buffer,
                  NULL);

  if (NULL != strings)
    free(strings);

  DEALLOCBUFFER(buffer);
}

void ProxyTraceThroughUtility(const char* file_name,
                     const WORD lineNum,
                     const WORD level,
                     const char* message1)
{
	CSegment*  seg = new CSegment;

	*seg << file_name;
	*seg << lineNum;
	*seg << level;
	*seg << message1;

	CManagerApi api(eProcessUtility);
	STATUS res = api.SendMsg(seg, UTILITY_FORWARD_TRACE_TO_LOGGER);
}

std::string GetTimeOfDay()
{
	timeval tv;
	gettimeofday(&tv, NULL);
	return TimeOfDayToString(tv);
}
std::string TimeOfDayToString(timeval& tv)
{
	struct tm * timeinfo;
	char buffer	[80] = "";
	char buff2	[20] = "";

	timeinfo = localtime ( &(tv.tv_sec) );
	if(timeinfo)
	  strftime (buffer, 79, "%X.", timeinfo);
	else
	  FPASSERT(1);
	
	snprintf(buff2, 19, "%06d", static_cast<int>(tv.tv_usec));
	return std::string(buffer) + std::string(buff2);
}

void ExceptionTrace(const char* file_name,
             const WORD lineNum,
             const CPObject* pObj,
             DWORD errCode,
             const char* assertMessage)
{
  char* fname;
  if (assertMessage == NULL)
    assertMessage = "";

  fname = (char*)strrchr(file_name, '\\');
  if (fname == NULL)
    fname = (char*) file_name;
  else
    fname++;

  CProcessBase* pProcess    = CProcessBase::GetProcess();
  eProcessType  processType = pProcess->GetProcessType();
  const char*   processName = CProcessBase::GetProcessName(processType);

  DWORD taskID = COsTask::GetCurrentTaskId();
  const CTaskApp* currentTask =
    CProcessBase::GetProcess()->GetCurrentTask();

  const char* taskName = (currentTask != NULL) ?
    currentTask->GetTaskName() : "Unknown";

  const char* debugString = "";

  void*  array[MAX_STACK_DEPTH];
  DWORD  size = backtrace(array, MAX_STACK_DEPTH);
  char** strings = backtrace_symbols(array, size);

  const char* bt0 = (size > 0 ? strings[0] : "--");
  const char* bt1 = (size > 1 ? strings[1] : "--");
  const char* bt2 = (size > 2 ? strings[2] : "--");
  const char* bt3 = (size > 3 ? strings[3] : "--");
  const char* bt4 = (size > 4 ? strings[4] : "--");
  const char* bt5 = (size > 5 ? strings[5] : "--");
  const char* bt6 = (size > 6 ? strings[6] : "--");
  const char* bt7 = (size > 7 ? strings[7] : "--");
  const char* bt8 = (size > 8 ? strings[8] : "--");
  const char* bt9 = (size > 9 ? strings[9] : "--");

  char buffer[8 * 1024];
  int realLength = snprintf(
    buffer,
    ARRAYSIZE(buffer),
"********************************** EXCEPTION **********************************\n\
%sEXCEPTION_TRACE Code:%u Process:%s Task:%s TaskID:%u\n\
%s:%1d %s\n\
--\n\
0:%s\n\
1:%s\n\
2:%s\n\
3:%s\n\
4:%s\n\
5:%s\n\
6:%s\n\
7:%s\n\
8:%s\n\
9:%s\n\
*******************************************************************************",
    debugString,
    errCode,
    processName,
    taskName,
    taskID,
    fname,
    lineNum,
    assertMessage,
    bt0,
    bt1,
    bt2,
    bt3,
    bt4,
    bt5,
    bt6,
    bt7,
    bt8,
    bt9
    );

  if (NULL != strings)
    free(strings);

  // Prints on terminal only in simulation
  if (!IsTarget())
    std::cerr << buffer << std::endl;

  OutTraceMessage(file_name,
                  lineNum,
                  eLevelOff,
                  pObj,
                  buffer,
                  NULL);


}
