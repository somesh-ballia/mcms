// LoggerManager.cpp

#include "LoggerManager.h"

#include <time.h>
#include <ostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>

#include "Trace.h"
#include "HlogApi.h"
#include "Segment.h"
#include "Request.h"
#include "NStream.h"
#include "EMATrace.h"
#include "StructTm.h"
#include "SysConfig.h"
#include "ObjString.h"
#include "ManagerApi.h"
#include "DummyEntry.h"
#include "TraceClass.h"
#include "TraceHeader.h"
#include "TraceTailer.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "SysConfigKeys.h"
#include "LoggerProcess.h"
#include "OpcodesLogger.h"
#include "FaultsDefines.h"
#include "AlarmStrTable.h"
#include "LoggerStatuses.h"
#include "LoggerRxSocket.h"
#include "LoggerTxSocket.h"
#include "LogFileManager.h"
#include "TraceStatistics.h"
#include "TerminalCommand.h"
#include "OutsideEntities.h"
#include "SystemFunctions.h"
#include "StatusesGeneral.h"
#include "ListenSocketApi.h"
#include "MplMcmsProtocol.h"
#include "OpcodesMcmsCommon.h"
#include "OutsideEntityNames.h"
#include "LoggerSocketStatus.h"
#include "LoggerProcess.h"
#include "InternalProcessStatuses.h"
#include "XMLFileBuilder.h"
#include "ModuleContent.h"

#include "UDPAppenderConfiguration.h"
#include "Log4cxxConfiguration.h"
#include "log4cxx/logger.h"
#include "log4cxx/basicconfigurator.h"
#include "log4cxx/xml/domconfigurator.h"
#include "log4cxx/helpers/exception.h"
#include "SysConfigEma.h"
#include "log4cxx/net/syslogappender.h"
#include "log4cxx/patternlayout.h"
#include "XMLFileBuilder.h"

using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::net;
using namespace log4cxx::helpers;

#include "OpcodesMcmsInternal.h"
#include "FilterTraceContainer.h"
#include "ContainerClientLogger.h"

#define LOGGER_FILE_WARNING_TIMEOUT      1000           // 10 seconds
#define LOGGER_FILE_WARNING_IDLE_TIMEOUT 15 * 60 * 100  // 15 minutes
#define LOGGER_UDPAPPENDER_TIMEOUT       1000           // 10 seconds
#define LOGGER_TIMER_COUNT					120
#define LOGGER_MSG_AVG_COUNT_TIMEOUT     100 * LOGGER_TIMER_COUNT
#define LOGGER_CFG_FILE                  "Cfg/loggerConfig.xml"
#define LOG4CXX_APPENDER_FILE                  "Cfg/log4cxxAppender.xml"
#define UDP_APPENDER_NUMBER               4
#define UDP_LOGGER_NAME  				"UDPLogger"
#define SYS_LOG_LOGGER_NAME  			"SysLogLogger"
#define UPDATE_WRITE_CFG_TIME			1 * SECOND

extern const char* MainEntityToString(APIU32 entityType);
extern void        LoggerMonitorEntryPoint(void* appParam);
extern void        pipeToLoggerEntryPoint(void* appParam);

static LoggerPtr log4cxxRVAppenderPtr = Logger::getLogger(UDP_LOGGER_NAME);
static LoggerPtr log4cxxSysLogAppenderPtr = Logger::getLogger(SYS_LOG_LOGGER_NAME);

PBEGIN_MESSAGE_MAP(CLoggerManager)
  ONEVENT(TRACE_MESSAGE, ANYCASE, CLoggerManager::HandleTraceMessage)
  ONEVENT(OUT_TRACE_MESSAGE, ANYCASE, CLoggerManager::HandleOutTraceMessage)
  ONEVENT(SUBSCRIBE, ANYCASE, CLoggerManager::HandleSubscribeRequest)
  ONEVENT(UNSUBSCRIBE, ANYCASE, CLoggerManager::HandleUnSubscribeRequest)
  ONEVENT(FLUSH_TO_LOGGER, ANYCASE, CLoggerManager::HandleFlush)
  ONEVENT(CLOSE_SOCKET_CONNECTION, ANYCASE, CLoggerManager::HandleLoggerDisconnect)
  ONEVENT(OPEN_SOCKET_CONNECTION, ANYCASE, CLoggerManager::HandleLoggerConnection)
  ONEVENT(LOGGER_FILE_SYSTEM_WARNING_TIMER, ANYCASE, CLoggerManager::OnTimerFileSystemWarningTest)
  ONEVENT(LOGGER_SYSTEM_LOAD_AVERAGE, ANYCASE, CLoggerManager::OnSystemLoadAverage)
  ONEVENT(LOGGER_DROP_MESSAGE_FLAG_ON_TIMER, ANYCASE, CLoggerManager::OnDropMessageFlagOnTimer)
  ONEVENT(LOGGER_UDPAPPENDER_TIMER, ANYCASE, CLoggerManager::OnUDPAppenderTimer)
  ONEVENT(LOGGER_MSG_AVG_COUNT_TIMER, ANYCASE, CLoggerManager::OnMsgAvgCount)
  ONEVENT(LOGGER_LICENSING_IND, ANYCASE, CLoggerManager::OnLicensingInd)
  ONEVENT(LOGGER_SET_LEVEL_EXPLICIT, ANYCASE, CLoggerManager::OnSetLoggerLevelExplicit)
  ONEVENT(LOGGER_UPDATE_WRITE_CFG_TIMER, ANYCASE, CLoggerManager::OnUpdateCfgWriteAlternativeTout)

PEND_MESSAGE_MAP(CLoggerManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CLoggerManager)
  ON_TRANS("TRANS_TRACE", "ADD_TRACE", CEMATrace, CLoggerManager::OnEmaTrace)
  ON_TRANS("TRANS_MCU", "FLUSH", CDummyEntry, CLoggerManager::OnEmaFlush)
  ON_TRANS("TRANS_LOGGER", "UPDATE", CLog4cxxConfiguration, CLoggerManager::OnServerSetLoggerConfiguration)
  ON_TRANS("TRANS_LOGGER", "CS_LOG_START", CDummyEntry, CLoggerManager::OnServerStartCSLogs)
  ON_TRANS("TRANS_LOGGER", "CS_LOG_STOP", CDummyEntry, CLoggerManager::OnServerStopCSLogs)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CLoggerManager)
  ONCOMMAND("tail_t", CLoggerManager::HandleTerminalTail, "display logger tail (last entries)")
  ONCOMMAND("settail", CLoggerManager::HandleTerminalSetTail, "set logger tail size - set 1000")
  ONCOMMAND("flush", CLoggerManager::HandleTerminalFlush, "flush buffer into ZIP file")
  ONCOMMAND("socket", CLoggerManager::HandleTerminalSocket, "disable/enable traces from outside(cs, mpl), normal,drop,dead")
  ONCOMMAND("clear_stat", CLoggerManager::HandleTerminalClearStat, "clears all statistics about traces")
  ONCOMMAND("stop_file_sys", CLoggerManager::HandleTerminalStopFileSystem, "stops file system")
  ONCOMMAND("log4cxx_add_udp_appender", CLoggerManager::HandleTerminalAddAppender, "add a udp appender")
  ONCOMMAND("log4cxx_show_udp_appenders", CLoggerManager::HandleTerminalShowAllUDPAppenders, "show all udp appenders")
  ONCOMMAND("log4cxx_show_switch", CLoggerManager::HandleTerminalShowLog4cxxSwitch, "show log4cxx switch status")
  ONCOMMAND("log4cxx_enable", CLoggerManager::HandleTerminalEnableLog4cxxSwitch, "enable log4cxx  and start sending log items")
  ONCOMMAND("log4cxx_disable", CLoggerManager::HandleTerminalDisableLog4cxxSwitch, "disable log4cxx and clear all udp appenders")
  ONCOMMAND("stop_logging", CLoggerManager::HandleTerminalStopLogging, "stop recording log into files")
  ONCOMMAND("start_logging", CLoggerManager::HandleTerminalStartLogging, "start recording log into files")
  ONCOMMAND("logging_status", CLoggerManager::HandleTerminalLoggingStatus, "display logging status")
  ONCOMMAND("set_level", CLoggerManager::HandleTerminalSetLevel, "explicitly set trace level")
  ONCOMMAND("get_log_static",CLoggerManager::HandleTerminalGetLogStatic, "Get Logger static")
  ONCOMMAND("test_udp_appender", CLoggerManager::HandleTerminalTestUDPConfiguration, "test udp appender configuration")
END_TERMINAL_COMMANDS

void LoggerManagerEntryPoint(void* appParam)
{
  CLoggerManager* pLoggerManager = new CLoggerManager;
  pLoggerManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CLoggerManager::GetMonitorEntryPoint()
{
  return LoggerMonitorEntryPoint;
}

CLoggerManager::CLoggerManager(void) :
  m_ContainerClientLogger(NULL),
  m_FileService(new CLogFileManager),
  m_TraceTailer(NULL),
  m_pListenSocketApi(NULL),
  m_fileSystemWarning(FALSE),
  m_proc((CLoggerProcess*)CProcessBase::GetProcess()),
  m_enable_logging(true),
  m_enable_update_level(true),
  m_bIsMfaUpdated(STATUS_FAIL),
  m_cfgWriteToAlternativeFileEnabled(false)
{
  PASSERT_AND_RETURN(NULL == m_proc);

  memset(&m_prev_commonHeader, 0, sizeof m_prev_commonHeader);
  memset(&m_prev_physicalHeader, 0, sizeof m_prev_physicalHeader);
  memset(&m_prev_traceHeader, 0, sizeof m_prev_traceHeader);
  memset(&m_prev_mcmsHeader, 0, sizeof m_prev_mcmsHeader);

  m_xml_builder.Init(UDP_LOGGER_NAME,SYS_LOG_LOGGER_NAME);
  InitLog4cxxConfiguration();
}

// Virtual
CLoggerManager::~CLoggerManager(void)
{
  delete m_TraceTailer;
  delete m_FileService;
  delete m_ContainerClientLogger;
  delete m_pListenSocketApi;
}

// Virtual
const char* CLoggerManager::NameOf(void) const
{
  return GetCompileType();
}

void CLoggerManager::AddFilterOpcodePoint(void)
{
  AddFilterOpcodeToQueue(TRACE_MESSAGE);
  AddFilterOpcodeToQueue(OUT_TRACE_MESSAGE);
}

void CLoggerManager::SelfKill(void)
{
  m_ContainerClientLogger->RemoveAll();

  m_proc->SetSocketStatus(eSocketStatusDead);
  m_pListenSocketApi->SyncDestroy();

  m_PipeToLoggerTaskApi.SyncDestroy();

  CManagerTask::SelfKill();
}

// Virtual
unsigned int CLoggerManager::GetMaxLegitimateUsagePrecents() const
{
  return 50;
}

// Virtual
int CLoggerManager::GetTaskMbxBufferSize() const
{
  return 4096 * 1024 - 1;
}

// Virtual
int CLoggerManager::GetTaskMbxThreshold() const
{
  return 3500 * 1024 - 1;
}

void CLoggerManager::DispatchTrace(const char* message)
{
  if (!m_enable_logging)
    return;

  m_ContainerClientLogger->DispatchTrace(message);
  m_TraceTailer->SetTrace(message);
  TraceToFile(message);
  if (m_cfgWriteToAlternativeFileEnabled)
  {
	  std::ostringstream cmd;
	  cmd << message;
	  PrintErrorToLocalFile(cmd, false);
  }
}

void CLoggerManager::TraceToFile(const char* message)
{
  // The file system was disabled, see StopFileSystem()
  if (NULL == m_FileService)
    return;

  eLogFileManagerState state = m_FileService->GetState();
  if (eLogFileManagerState_Ready == state)
  {
    m_FileService->MessageToFile(message);
  }
  else
  {
    if (false == IsActiveAlarmExistByErrorCode(BAD_FILE_SYSTEM) &&
        false == IsActiveAlarmExistByErrorCode(FAILED_TO_INIT_FILE_SYSTEM))
    {
      StopFileSystem();
    }
  }
}

void CLoggerManager::HandleOutTraceMessage(CSegment* pMsg, DWORD msgLen)
{
  if (m_bStartLogStaticTimer == FALSE)
  {
  	StartTimer(LOGGER_MSG_AVG_COUNT_TIMER, LOGGER_MSG_AVG_COUNT_TIMEOUT);
  	m_bStartLogStaticTimer = TRUE;
    if (m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->IsEnabled())
        	StartTimer(LOGGER_UDPAPPENDER_TIMER, LOGGER_UDPAPPENDER_TIMEOUT);
  }
  // Should be skipped on higher level
  PASSERT_AND_RETURN(m_proc->GetDropMessageFlag());

  std::ostringstream buf;
  if (BuildOutMessage(pMsg, buf))
	  DispatchTrace(buf.str().c_str());
}

void CLoggerManager::HandleTraceMessage(CSegment* pMsg, DWORD msgLen)
{
//if (isn)
  // Skips the message on high system load average
  if (m_bStartLogStaticTimer == FALSE)
  {
  	StartTimer(LOGGER_MSG_AVG_COUNT_TIMER, LOGGER_MSG_AVG_COUNT_TIMEOUT);
  	m_bStartLogStaticTimer = TRUE;
    if (m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->IsEnabled())
    	StartTimer(LOGGER_UDPAPPENDER_TIMER, LOGGER_UDPAPPENDER_TIMEOUT);
  }

  if (m_proc->GetDropMessageFlagAndIncrease(CLoggerProcess::eSenderMCMS))
    return;

  std::ostringstream buf;
  if (BuildMcmsMessage(pMsg, buf))
	  DispatchTrace(buf.str().c_str());
}

STATUS CLoggerManager::OnEmaTrace(CRequest* pRequest)
{
  // Skips the message on high system load average
  if (m_proc->GetDropMessageFlagAndIncrease(CLoggerProcess::eSenderEMA))
  {
    pRequest->SetConfirmObject(new CDummyEntry);
    pRequest->SetStatus(STATUS_OK);
    return STATUS_OK;
  }

  std::ostringstream buf;
  CEMATrace*         pEMATrace = (CEMATrace*)pRequest->GetRequestObject();
  BuildEmaMessage(pEMATrace, buf);
  DispatchTrace(buf.str().c_str());

  // Must be here for infrastructure, else it sends general response.
  pRequest->SetConfirmObject(new CDummyEntry);
  pRequest->SetStatus(STATUS_OK);

  return STATUS_OK;
}

bool CLoggerManager::BuildMcmsMessage(CSegment* pSeg,
                                      std::ostringstream& buf)
{
  // init once
  static COMMON_HEADER_S commonHeader;
  static PHYSICAL_INFO_HEADER_S physicalHeader;
  if (commonHeader.src_id != eMcms)
  {
    memset(&commonHeader, 0, sizeof commonHeader);
    commonHeader.src_id = eMcms;
    commonHeader.dest_id = eMcms;
    memset(&physicalHeader, 0, sizeof physicalHeader);
  }

  const BYTE*     mainBuf = pSeg->GetPtr();
  WORD            offset = 0;
  TRACE_HEADER_S* pTraceHeader = (TRACE_HEADER_S*)(mainBuf + offset);

  offset += sizeof(TRACE_HEADER_S);
  const MCMS_TRACE_HEADER_S* pMcmsHeader =
    (const MCMS_TRACE_HEADER_S*)(mainBuf + offset);

  offset += sizeof(MCMS_TRACE_HEADER_S);
  const char* ptrContent = (const char*)(mainBuf + offset);

  BuildGenericMessage(buf,
                      commonHeader,
                      physicalHeader,
                      *pTraceHeader,
                      *pMcmsHeader,
                      ptrContent);

  unsigned int process_type = pTraceHeader->m_processType;
  unsigned int msg_log_level = pTraceHeader->m_level;
  int log4cxx_levevl = IsThisMsgNeedToBeSentToLoggerType(msg_log_level, eMcms,
                                                             process_type,m_proc->GetLog4cxxConfiguration()->GetLocalLogAppender());

  SendToAppenders(eMcms, msg_log_level, process_type, buf.str().c_str(),log4cxxSysLogAppenderPtr,m_proc->GetLog4cxxConfiguration()->GetSysLogAppender());
  SendToAppenders(eMcms, msg_log_level, process_type, buf.str().c_str(),log4cxxRVAppenderPtr,m_proc->GetLog4cxxConfiguration()->GetUDPAppender());

  if (log4cxx_levevl != -1)
      return true;
  else
	  return false;
}

bool CLoggerManager::BuildOutMessage(CSegment* pSeg,
                                     std::ostringstream& buf)
{
  CMplMcmsProtocol prot;
  bool res = prot.DeSerialize(*pSeg);
  PASSERTMSG_AND_RETURN_VALUE(!res, "Bad DeSerialize",res);

  const COMMON_HEADER_S&        commonHeader = prot.GetCommonHeaderConst();
  const PHYSICAL_INFO_HEADER_S& physicalHeader = prot.GetPhysicalHeaderConst();
  TRACE_HEADER_S&               traceHeader = prot.GetTraceHeader();
  const char*                   ptrContent = (const char*)(prot.GetData());

  MCMS_TRACE_HEADER_S mcmsHeader;
  memset(&mcmsHeader, 0, sizeof mcmsHeader);

  timeval tv;
  gettimeofday(&tv, NULL);
  traceHeader.m_tv_sec  = tv.tv_sec;
  traceHeader.m_tv_usec = tv.tv_usec;

  BuildGenericMessage(buf,
                      commonHeader,
                      physicalHeader,
                      traceHeader,
                      mcmsHeader,
                      ptrContent);

  int log4cxx_levevl = IsThisMsgNeedToBeSentToLoggerType(traceHeader.m_level, commonHeader.src_id,
		  traceHeader.m_processType,m_proc->GetLog4cxxConfiguration()->GetLocalLogAppender());

  SendToAppenders(commonHeader.src_id, traceHeader.m_level, traceHeader.m_processType,buf.str().c_str(),log4cxxSysLogAppenderPtr,m_proc->GetLog4cxxConfiguration()->GetSysLogAppender());
  SendToAppenders(commonHeader.src_id, traceHeader.m_level,traceHeader.m_processType, buf.str().c_str(),log4cxxRVAppenderPtr,m_proc->GetLog4cxxConfiguration()->GetUDPAppender());

  if (log4cxx_levevl != -1)
        return true;
    else
  	    return false;
}

void CLoggerManager::BuildEmaMessage(CEMATrace* trace,
                                     std::ostringstream& buf)
{


  TRACE_HEADER_S* traceHeader = trace->GetTraceHeader();
  const char*     content = trace->GetContent();
  CTrace::BuildEmaMessage(traceHeader,content,buf);

  if(m_proc->m_enableLocalTracer)
  {
	  CTrace::TraceEMAOutMessage(buf,*traceHeader);
  }

  int log4cxx_levevl = IsThisMsgNeedToBeSentToLoggerType(traceHeader->m_level, eEma,
  		  traceHeader->m_processType,m_proc->GetLog4cxxConfiguration()->GetLocalLogAppender());

  SendToAppenders(eEma, traceHeader->m_level,
                     traceHeader->m_processType,
                     buf.str().c_str(),log4cxxRVAppenderPtr,m_proc->GetLog4cxxConfiguration()->GetUDPAppender());
  SendToAppenders(eEma, traceHeader->m_level,
                       traceHeader->m_processType,
                       buf.str().c_str(),log4cxxSysLogAppenderPtr,m_proc->GetLog4cxxConfiguration()->GetSysLogAppender());
}

void CLoggerManager::BuildGenericMessage(std::ostringstream& buf,
                                         const COMMON_HEADER_S& commonHeader,
                                         const PHYSICAL_INFO_HEADER_S& physicalHeader,
                                         TRACE_HEADER_S& traceHeader,
                                         const MCMS_TRACE_HEADER_S& mcmsHeader,
                                         const char* content)
{
  if (eLevelInfoNormal == traceHeader.m_level && '0' != traceHeader.m_terminalName[0])
    PrintCommandAnswerToTerminal(traceHeader.m_terminalName, content);

  if ('\0' == traceHeader.m_taskName[0])
    strcpy(traceHeader.m_taskName, "NULL");

  if ('\0' == traceHeader.m_objectName[0])
    strcpy(traceHeader.m_objectName, "NULL");

  traceHeader.m_taskName[MAX_TASK_NAME_LEN - 1] = '\0';
  CObjString::ReplaceChar(traceHeader.m_taskName, ' ', '_');
  traceHeader.m_objectName[MAX_OBJECT_NAME_LEN - 1] = '\0';
  CObjString::ReplaceChar(traceHeader.m_objectName, ' ', '_');
  traceHeader.m_str_opcode[STR_OPCODE_LEN - 1] = '\0';
  CObjString::ReplaceChar(traceHeader.m_str_opcode, ' ', '_');
  traceHeader.m_terminalName[MAX_TERMINAL_NAME_LEN - 1] = '\0';
  CObjString::ReplaceChar(traceHeader.m_terminalName, ' ', '_');

  FillBuffer(buf, commonHeader, physicalHeader, traceHeader, mcmsHeader, content);
}

// Static
void CLoggerManager::GenerateTitle(const COMMON_HEADER_S& commonHeader,
                                   const TRACE_HEADER_S& traceHeader,
                                   std::string& out)
{
  const char* proc_name =
    CLoggerProcess::GetProcessNameByMainEntity((eMainEntities)commonHeader.src_id,
                                               traceHeader.m_processType);
  size_t proc_name_len = strlen(proc_name);

#if 0
  CStructTm localTime;
  SystemGetTime(localTime);
#endif

  const char* str_entity = MainEntityToString(commonHeader.src_id);
  size_t      str_entity_len = strlen(str_entity);

  const char* str_level = CTrace::GetTraceLevelShortNameByValue(traceHeader.m_level);
  size_t str_level_len = strlen(str_level);

  char   title[256];
  size_t len = 0;

  // Counts the title length
  size_t title_len = 29 + str_entity_len + 3 + proc_name_len
                     + 1 + ARRAYSIZE(TITLE_SourceId) + 10
                     + ARRAYSIZE(TITLE_TaskName)
                     + 1 + strlen(traceHeader.m_taskName)
                     + 3 + strlen(traceHeader.m_objectName)
                     + 3 + str_level_len;

  if (ARRAYSIZE(title) < title_len)
  {
    std::stringstream buf;
    buf << "buffer overflow: " << ARRAYSIZE(title) << " < " << title_len;
    out = buf.str();
    return;
  }

  title[len++] = TITLE_Date[0];
  title[len++] = ':';
  time_t deliveryTime = traceHeader.m_tv_sec;  
  len+=strftime(title+len,30,"%d/%m/%y-%T",gmtime(&deliveryTime));
#if 0
  title[len++] = (localTime.m_day / 10) % 10 + '0';
  title[len++] = localTime.m_day % 10 + '0';
  title[len++] = '/';
  title[len++] = (localTime.m_mon / 10) % 10 + '0';
  title[len++] = localTime.m_mon % 10 + '0';
  title[len++] = '/';
  title[len++] = ((localTime.m_year - 2000) / 10) % 10 + '0';
  title[len++] = (localTime.m_year - 2000) % 10 + '0';
  title[len++] = '-';
  title[len++] = (localTime.m_hour / 10) % 10 + '0';
  title[len++] = localTime.m_hour % 10 + '0';
  title[len++] = ':';
  title[len++] = (localTime.m_min / 10) % 10 + '0';
  title[len++] = localTime.m_min % 10 + '0';
  title[len++] = ':';
  title[len++] = (localTime.m_sec / 10) % 10 + '0';
  title[len++] = localTime.m_sec % 10 + '0';
#endif
  title[len++] = '.';
#if 0  
  title[len++] = (traceHeader.m_systemTick / 1000000) % 10 + '0';
  title[len++] = (traceHeader.m_systemTick / 100000) % 10 + '0';
  title[len++] = (traceHeader.m_systemTick / 10000) % 10 + '0';
  title[len++] = (traceHeader.m_systemTick / 1000) % 10 + '0';
  title[len++] = (traceHeader.m_systemTick / 100) % 10 + '0';
  title[len++] = (traceHeader.m_systemTick / 10) % 10 + '0';
  title[len++] = traceHeader.m_systemTick % 10 + '0';
#endif  
  //title[len++] = (traceHeader.m_tv_usec / 1000000) % 10 + '0';
  title[len++] = (traceHeader.m_tv_usec / 100000) % 10 + '0';
  title[len++] = (traceHeader.m_tv_usec / 10000) % 10 + '0';
  title[len++] = (traceHeader.m_tv_usec / 1000) % 10 + '0';
  title[len++] = (traceHeader.m_tv_usec / 100) % 10 + '0';
  title[len++] = (traceHeader.m_tv_usec / 10) % 10 + '0';
  title[len++] = traceHeader.m_tv_usec % 10 + '0';

  title[len++] = ' ';
  title[len++] = TITLE_MainEntity[0];
  title[len++] = ':';
  std::copy(str_entity, str_entity + str_entity_len, title + len);
  len += str_entity_len;

  title[len++] = ' ';
  title[len++] = TITLE_ProcessName[0];
  title[len++] = ':';
  std::copy(proc_name, proc_name + proc_name_len, title + len);
  len += proc_name_len;

  title[len++] = ' ';
  title[len++] = TITLE_SourceId[0];
  title[len++] = TITLE_SourceId[1];
  title[len++] = TITLE_SourceId[2];
  title[len++] = ':';
  title[len++] = (traceHeader.m_sourceId / 10000000) % 10 + '0';
  title[len++] = (traceHeader.m_sourceId / 1000000) % 10 + '0';
  title[len++] = (traceHeader.m_sourceId / 100000) % 10 + '0';
  title[len++] = (traceHeader.m_sourceId / 10000) % 10 + '0';
  title[len++] = (traceHeader.m_sourceId / 1000) % 10 + '0';
  title[len++] = (traceHeader.m_sourceId / 100) % 10 + '0';
  title[len++] = (traceHeader.m_sourceId / 10) % 10 + '0';
  title[len++] = traceHeader.m_sourceId % 10 + '0';

  title[len++] = ' ';
  title[len++] = TITLE_TaskName[0];
  title[len++] = TITLE_TaskName[1];
  title[len++] = ':';
  std::copy(traceHeader.m_taskName,
            traceHeader.m_taskName + strlen(traceHeader.m_taskName),
            title + len);
  len += strlen(traceHeader.m_taskName);

  title[len++] = ' ';
  title[len++] = TITLE_ObjectName[0];
  title[len++] = ':';
  std::copy(traceHeader.m_objectName,
            traceHeader.m_objectName + strlen(traceHeader.m_objectName),
            title + len);
  len += strlen(traceHeader.m_objectName);

  title[len++] = ' ';
  title[len++] = TITLE_Level[0];
  title[len++] = ':';
  std::copy(str_level, str_level + str_level_len, title + len);
  len += str_level_len;
  title[len] = '\0';

  out = title;
}

void CLoggerManager::FillBuffer(std::ostringstream& buf,
                                const COMMON_HEADER_S& commonHeader,
                                const PHYSICAL_INFO_HEADER_S& physicalHeader,
                                const TRACE_HEADER_S& traceHeader,
                                const MCMS_TRACE_HEADER_S& mcmsHeader,
                                const char* content)
{
  DWORD messageLen = (NULL == content) ? 0 : strlen(content);
  m_proc->AddTraceToStatistics((eMainEntities)commonHeader.src_id,
                               traceHeader.m_processType,
                               messageLen);

  BOOL isNewHeaderNeeded = CheckIfNewHeaderIsNeeded(commonHeader,
                                                    physicalHeader,
                                                    traceHeader,
                                                    mcmsHeader);
  if (isNewHeaderNeeded)
  {
    // Keeps the title for possible future usage
    GenerateTitle(commonHeader, traceHeader, m_title);

    // Keeps the headers for possible future usage
    memcpy(&m_prev_commonHeader, &commonHeader, sizeof m_prev_commonHeader);
    memcpy(&m_prev_physicalHeader, &physicalHeader, sizeof m_prev_physicalHeader);
    memcpy(&m_prev_traceHeader, &traceHeader, sizeof m_prev_traceHeader);
    memcpy(&m_prev_mcmsHeader, &mcmsHeader, sizeof m_prev_mcmsHeader);
  }

  buf << m_title;

  // Additional logger header
  if (DEFAULT_TOPIC_ID != traceHeader.m_topic_id)
    buf << " " << TITLE_TopicID << ":" << traceHeader.m_topic_id;

  if (DEFAULT_UNIT_ID != traceHeader.m_unit_id)
    buf << " " << TITLE_UnitID << ":" << traceHeader.m_unit_id;

  if (DEFAULT_CONF_ID != traceHeader.m_conf_id)
    buf << " " << TITLE_ConfID << ":" << traceHeader.m_conf_id;

  if (DEFAULT_PARTY_ID != traceHeader.m_party_id)
    buf << " " << TITLE_PartyID << ":" << traceHeader.m_party_id;

  if ('\0' != mcmsHeader.m_file_line_number[0])
    buf << " " << TITLE_FileLine << ":" << mcmsHeader.m_file_line_number;

  if (0 != physicalHeader.board_id)
    buf << " " << TITLE_BoardId << ":"
        << static_cast<short>(physicalHeader.board_id);

  if (0 != physicalHeader.port_id)
    buf << " " << TITLE_PortId << ":"
        << static_cast<short>(physicalHeader.port_id);

  if (0 != physicalHeader.accelerator_id)
    buf << " " << TITLE_AcceleratorId << ":"
        << static_cast<short>(physicalHeader.accelerator_id);

  buf << std::endl
      << ((NULL == content) ? "empty" : content)
      << "\n\n";
}

STATUS CLoggerManager::HandleTerminalTail(CTerminalCommand& command,
                                          std::ostream& answer)
{
  m_TraceTailer->GetTail(answer);
  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalSetTail(CTerminalCommand& command,
                                             std::ostream& answer)
{
  DWORD numOfParams = command.GetNumOfParams();
  if (1 != numOfParams)
  {
    answer << "error: size must be specified\n";
    answer << "usage: Bin/McuCmd settail Logger [size]\n";
    return STATUS_OK;
  }

  const string& strSize = command.GetToken(eCmdParam1);
  DWORD         size = atoi(strSize.c_str());
  m_TraceTailer->SetTailLen(size);

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalClearStat(CTerminalCommand& command,
                                               std::ostream& answer)
{
  m_proc->ResetStatistics();
  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalStopFileSystem(CTerminalCommand& command,
                                                    std::ostream& answer)
{
  StopFileSystem();

  return STATUS_OK;
}

// Static
STATUS CLoggerManager::ManSetLevel(std::ostream& ans)
{
  ans << "NAME\n"
      << "\tset_level - explicitly set trace level\n"
      << std::endl
      << "SYNOPSIS\n"
      << "\tBin/McuCmd set_level Logger LEVEL\n"
      << "DESCRIPTION\n"
      << "\tLEVEL\ttrace level integer:\n"
      << "\t\tOFF           = 0\n"
      << "\t\tFATAL         = 1\n"
      << "\t\tERROR         = 10\n"
      << "\t\tWARN          = 20\n"
      << "\t\tINFO_HIGH     = 30\n"
      << "\t\tINFO_NORMAL   = 50\n"
      << "\t\tDEBUG         = 70\n"
      << "\t\tTRACE         = 100\n"
      << std::endl;

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalSetLevel(CTerminalCommand& cmd,
                                              std::ostream& ans)
{
  if (1 != cmd.GetNumOfParams())
    return ManSetLevel(ans);

  int new_level = atoi(cmd.GetToken(eCmdParam1).c_str());
  if (!CTrace::IsTraceLevelValid(new_level))
  {
    ans << "set level: invalid level " << new_level << std::endl;
    return ManSetLevel(ans);
  }

  UpdateExplicitLogLevel(static_cast<eLogLevel>(new_level));

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalGetLogStatic(CTerminalCommand& cmd,std::ostream& ans)
{
	ans <<  m_proc->GetLog4cxxConfiguration()->GetMsgAvgRate() <<" msgs/sec";
	return STATUS_OK;
}

void CLoggerManager::OnTimerFileSystemWarningTest()
{
  if (m_FileService)
  {
    BOOL make_alarm = FALSE;
    CProcessBase::GetProcess()->GetSysConfig()->
    GetBOOLDataByKey(CFG_KEY_ENABLE_CYCLIC_FILE_SYSTEM_ALARMS, make_alarm);


    BOOL warning_flag = m_FileService->TestForFileSystemWarning();
    if (warning_flag && !m_fileSystemWarning)
    {
      if (make_alarm)
      {
        AddActiveAlarm(FAULT_GENERAL_SUBJECT,
                       BACKUP_OF_LOG_FILES_IS_REQUIRED,
                       MAJOR_ERROR_LEVEL,
                       "Administrator has to backup old log files before system will delete them",
                       true,
                       true);

        m_fileSystemWarning = TRUE;
      }
    }

    if (m_fileSystemWarning && !warning_flag)
    {
      RemoveActiveAlarmByErrorCode(BACKUP_OF_LOG_FILES_IS_REQUIRED);
      m_fileSystemWarning = FALSE;
    }
  }

  StartTimer(LOGGER_FILE_SYSTEM_WARNING_TIMER, LOGGER_FILE_WARNING_IDLE_TIMEOUT);
}

STATUS CLoggerManager::OnEmaFlush(CRequest* pRequest)
{
  STATUS status = GenericFlush();

  pRequest->SetConfirmObject(new CDummyEntry);
  pRequest->SetStatus(status);

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalFlush(CTerminalCommand& command,
                                           std::ostream& answer)
{
  STATUS             status = GenericFlush();
  const std::string& statusString = m_proc->GetStatusAsString(status);
  answer << "STATUS : " << statusString;

  return status;
}

STATUS CLoggerManager::HandleTerminalSocket(CTerminalCommand& command,
                                            std::ostream& answer)
{
  if (1 != command.GetNumOfParams())
  {
    answer << "Bad usage.\n";
    answer << "usage: >Bin/McuCmd socket {";
    SocketStatusUtils::DumpSocketStatus(answer);
    answer << "}";
    return STATUS_FAIL;
  }

  const std::string& decision = command.GetToken(eCmdParam1);
  eSocketStatus newStatus = SocketStatusUtils::GetSocketStatusByName(decision.c_str());
  if (eSocketStatusInvalid == newStatus)
  {
    answer << "bad param:" << decision << "\n" << "should be one of {";
    SocketStatusUtils::DumpSocketStatus(answer);
    answer << "}";
    return STATUS_FAIL;
  }

  m_proc->SetSocketStatus(newStatus);

  answer << "Socket Status : " << SocketStatusUtils::GetSocketStatusName(newStatus);

  return STATUS_OK;
}

void CLoggerManager::StopFileSystem()
{
  POBJDELETE(m_FileService);
  AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                          BAD_FILE_SYSTEM,
                          MAJOR_ERROR_LEVEL,
                          "The Log File System is disabled. Log files not found.",
                          true,
                          true);
}

STATUS CLoggerManager::GenericFlush()
{
  // The file system was disabled, see StopFileSystem
  if (NULL == m_FileService)
    return STATUS_FILE_SYSTEM_DISABLED;

  STATUS status = STATUS_OK;
  BOOL   res = m_FileService->Flush();
  if (FALSE == res)
  {
    StopFileSystem();
    status = STATUS_FAILED_TO_FLUSH;
  }

  return status;
}

STATUS CLoggerManager::CheckDiskSpaceForMaxLogSize(CLog4cxxConfiguration &logconfig,bool updateMaxLogLevel)
{
	std::string ans;
	std::string cmd;
	eProductFamily prodFamily = m_proc->GetProductFamily();
	eProductType prodType = m_proc->GetProductType();
	if( (eProductFamilyRMX == prodFamily && IsTarget()) || (eProductTypeGesher == prodType || eProductTypeNinja == prodType))
	{
		cmd = "df -m | grep -E ' /output$' | tr -s ' ' | cut -d' ' -f4";
	}
	else
	{
		cmd = "df -m "+(std::string)GET_MCU_HOME_DIR+" | tail -n -1 | tr -s ' ' | cut -d' ' -f4";
	}

	STATUS stat = SystemPipedCommand(cmd.c_str(), ans, TRUE, TRUE);


	if (STATUS_OK != stat)
	{
		TRACEINTO << "Failed to execute command: [" << cmd << "]";
		return STATUS_FAIL;
	}

	int free_space = atoi(ans.c_str());

	std::stringstream s;
	// get space used by logger now
	// desired: du -m LogFiles/ | cut -f1
	//
	s << "du -m " << LOGGER_DIR << " | cut -f1";
	stat = SystemPipedCommand(s.str().c_str(), ans, TRUE, TRUE);

	if (STATUS_OK != stat)
	{
		TRACEINTO << "Failed to execute command: [" << s.str() << "]";
		return STATUS_FAIL;
	}

	int used_space = atoi(ans.c_str());

	int total_space = (free_space + used_space) / 1024;

	TRACEINTO << "Disk space -> Used: "<< used_space << "M, Free: " << free_space << "M, Total: " << total_space << "G, Config: " << (int)logconfig.GetMaxLogSize() << "G.";
	if (logconfig.GetMaxLogSize() > total_space)
	{
		if (updateMaxLogLevel)
		{
			if (total_space > 2 && total_space < MAX_LOG_SIZE)
				logconfig.SetMaxLogSize(total_space);
		}
		TRACEINTO << "Status = " << (int)STATUS_INSUFFICIENT_DISK_SPACE << " (STATUS_INSUFFICIENT_DISK_SPACE)";
		return STATUS_INSUFFICIENT_DISK_SPACE;
	}

	return STATUS_OK;

}

STATUS CLoggerManager::HandleTerminalStopLogging(CTerminalCommand& command,
                                                 std::ostream& answer)
{

  Stoplogging();
  answer << "Logging has been disabled"<<"\n";

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalStartLogging(CTerminalCommand& command,
                                                  std::ostream& answer)
{
  Startlogging();
  answer << "Logging has been enabled"<<"\n";

  return STATUS_OK;
}

void   CLoggerManager::Stoplogging()
{
	m_enable_logging = false;

	AddActiveAlarmFaultOnlySingleton(FAULT_GENERAL_SUBJECT,
							BAD_FILE_SYSTEM,
							MAJOR_ERROR_LEVEL,
							"The Log File System is disabled by the admin.",
							true,
							true);


}

void   CLoggerManager::Startlogging()
{
	 m_enable_logging = true;
	 RemoveActiveAlarmFaultOnlyByErrorCode(BAD_FILE_SYSTEM);
}

STATUS CLoggerManager::HandleTerminalLoggingStatus(CTerminalCommand& command,
                                                   std::ostream& answer)
{
  if (m_enable_logging)
    answer << "Logging is on."<<"\n";
  else
    answer << "Logging is off."<<"\n";

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalAddAppender(CTerminalCommand& command,
                                                 std::ostream& answer)
{
  const string& ip = command.GetToken(eCmdParam1);
  const string& port = command.GetToken(eCmdParam2);
  string        filter = command.GetToken(eCmdParam3);
  if (filter == "Invalide Token")
    filter = "";

  std::vector<udp_appender>& vec_udp_tmp = m_xml_builder.GetUDPAppendersVec();
  if (vec_udp_tmp.size() == UDP_APPENDER_NUMBER)
  {
    answer<<"the amount of udp appenders could not exceed "<<
    UDP_APPENDER_NUMBER<<"\n";
    return STATUS_OK;
  }

  char buff[32];
  memset(buff, 0, sizeof(buff));
  sprintf(buff, "udp_appender_%d",
          m_xml_builder.GetUDPAppendersVec().size() + 1);

  m_xml_builder.AddUDPAppender(buff, ip, port, filter);
  std::vector<udp_appender>& vec_udp = m_xml_builder.GetUDPAppendersVec();

  for (unsigned int i = 0; i < vec_udp.size(); i++)
  {
    answer << "name:" << vec_udp[i].name << " ip:" << vec_udp[i].ip << " port:"
           << vec_udp[i].port << " filter:" << vec_udp[i].filter_str << "\n";
  }

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalShowLog4cxxSwitch(CTerminalCommand& command,
                                                       std::ostream& answer)
{
  if (m_xml_builder.GetSwitchState())
    answer<<"logger log4cxx on \n";
  else
    answer<<"logger log4cxx off \n";

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalClearAllUDPAppenders(CTerminalCommand& command,
                                                          std::ostream& answer)
{
  m_xml_builder.ClearAllUDPAppenders();

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalShowAllUDPAppenders(CTerminalCommand& command,
                                                         std::ostream& answer)
{
  std::vector<udp_appender>& vec_udp = m_xml_builder.GetUDPAppendersVec();
  for (unsigned int i = 0; i < vec_udp.size(); i++)
  {
    answer << "name:" << vec_udp[i].name << " ip:" << vec_udp[i].ip << " port:"
           << vec_udp[i].port << " filter:" << vec_udp[i].filter_str << "\n";
  }

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalEnableLog4cxxSwitch(CTerminalCommand& command,
                                                         std::ostream& answer)
{
  m_xml_builder.SetSwitchState(true);
  m_xml_builder.GenerateFile(LOG4CXX_APPENDER_FILE);
  DOMConfigurator::configure(LOG4CXX_APPENDER_FILE);

  return STATUS_OK;
}

STATUS CLoggerManager::HandleTerminalDisableLog4cxxSwitch(CTerminalCommand& command,
                                                          std::ostream& answer)
{
  m_xml_builder.SetSwitchState(false);
  m_xml_builder.ClearAllUDPAppenders();
  m_xml_builder.GenerateFile(LOG4CXX_APPENDER_FILE);
  DOMConfigurator::configure(LOG4CXX_APPENDER_FILE);

  return STATUS_OK;
}

void CLoggerManager::OnLicensingInd(CSegment* pSeg)
{
	LOGGER_LICENSING_S params;
	pSeg->Get((BYTE*)&params, sizeof(LOGGER_LICENSING_S));
	TRACEINTO
		<< "\nnum_cop_parties: " << params.num_cop_parties
		<< "\nnum_cp_parties : " << params.num_cp_parties
		<< "\nMedia card mode: " << params.sys_card_mode;
	m_proc->SetLicensingData(params);
}

void CLoggerManager::HandleSubscribeRequest(CSegment* pSeg)
{
  PTRACE(eLevelInfoNormal, "CRequestHandler::HandleSubscribeRequest");

  COsQueue* clientQueue = new COsQueue;
  char      uniqueName[MAX_QUEUE_NAME_LEN];
  *pSeg >> uniqueName;
  clientQueue->CreateWrite(eProcessLogger, uniqueName, TRUE);
  clientQueue->m_process = (eProcessType) -1;

  m_ContainerClientLogger->push_back(clientQueue);
}

void CLoggerManager::HandleUnSubscribeRequest(CSegment* pSeg)
{
  PTRACE(eLevelInfoNormal, "CRequestHandler::HandleUnSubscribeRequest");

  char uniqueName[MAX_QUEUE_NAME_LEN];
  *pSeg >> uniqueName;

  m_ContainerClientLogger->RemoveClient(uniqueName);
}

void CLoggerManager::HandleFlush(CSegment* pSeg)
{
  STATUS status = GenericFlush();
}

void CLoggerManager::HandleLoggerDisconnect(CSegment* pSeg)
{
  PTRACE(eLevelInfoNormal, "CLoggerManager::HandleLoggerDisconnect");
}

void CLoggerManager::HandleLoggerConnection(CSegment* pSeg)
{
  PTRACE(eLevelInfoNormal, "CLoggerManager::HandleLoggerConnection");
}

void CLoggerManager::PrintCommandAnswerToTerminal(const char* terminalName,
                                                  const char* answer) const
{
  ofstream file;
  file.open(terminalName);
  if (file.is_open())
  {
    file << answer << flush;
    file.close();
  }
}

void CLoggerManager::ManagerPostInitActionsPoint()
{
  if (!IsHardDiskOk())
  {
    AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            BAD_HARD_DISK,
                            MAJOR_ERROR_LEVEL,
                            "Hard disk not responding",
                            true,
                            true);
    m_FileService->SetState(eLogFileManagerState_Major);
  }

  if ( eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily() )//nati: Soft mcu dynamic logger level is disabled
      m_enable_update_level = false;
  m_FileService->InitLogger();


  if (eLogFileManagerState_Ready != m_FileService->GetState())
  {
    AddActiveAlarmSingleton(FAULT_GENERAL_SUBJECT,
                            FAILED_TO_INIT_FILE_SYSTEM,
                            MAJOR_ERROR_LEVEL,
                            "Failed to initialize the file system",
                            true,
                            true);
  }

  CreateTask(&m_PipeToLoggerTaskApi, pipeToLoggerEntryPoint, m_pRcvMbx);

  m_TraceTailer = new CTraceTailer;
  m_ContainerClientLogger = new CContainerClientLogger;

  const char* MplApiIp = "0.0.0.0";
  if (IsTarget())
  {
    switch (m_proc->GetProductFamily())
    {
      case eProductFamilyRMX:

        MplApiIp = "169.254.128.10";
        break;

      case eProductFamilyCallGenerator:
        MplApiIp = "127.0.0.1";
        break;

      default:
        PASSERTSTREAM(true, "Illegal product family " << m_proc->GetProductFamily());
    } // switch
  }
  else
	  if(eProductFamilySoftMcu == m_proc->GetProductFamily())
		  MplApiIp = "127.0.0.1";

  m_proc->GetLog4cxxConfiguration()->SetIsCSLogStarted(m_proc->GetCSLogsStateFromSysCfg());

  m_pListenSocketApi = new CListenSocketApi(LoggerSocketRxEntryPoint,
                                            LoggerSocketTxEntryPoint,
                                            LOGGER_LISTEN_SOCKET_PORT,
                                            MplApiIp);

  m_pListenSocketApi->SetCreateConnectionMode(eRxConnection);
  m_pListenSocketApi->SetMaxNumConnections(40);
  m_pListenSocketApi->Create(*m_pRcvMbx);
  StartTimer(LOGGER_FILE_SYSTEM_WARNING_TIMER, LOGGER_FILE_WARNING_TIMEOUT);

  OnUpdateCfgWriteAlternativeTout(NULL);
}

void CLoggerManager::InformHttpGetFile(const std::string& file_name)
{
  if (m_FileService)
    m_FileService->InformLogFileRetrieved(file_name);

  StartTimer(LOGGER_FILE_SYSTEM_WARNING_TIMER, LOGGER_FILE_WARNING_TIMEOUT);
}

eLogLevel CLoggerManager::GetTraceLevelByLoad(unsigned int load,
                                                unsigned int threshold) const
{
  eLogLevel level;

  if ( eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily() )
  {
	  if (load < (threshold*0.9))
	    level = eLevelTrace;
	  else if (load < threshold)
	    level = eLevelError;
	  else
    	level = eLevelOff;
  }
  else
  {
	  //    n    api          crt             no      disabled
	  // 0----1/8----1/4----------------3/4--------1----------

	  if (load < threshold / 8)
	    level = eLevelTrace;
	  else if (load < threshold / 4)
	    level = eLevelInfoHigh;
	  else if (load < threshold / 4 * 3)
	    level = eLevelError;
	  else
	    level = eLevelOff;
  }

  // Verifies if the level meets maximum level
  CSysConfig* cfg = m_proc->GetSysConfig();
  PASSERT_AND_RETURN_VALUE(NULL == cfg, level);

  std::string slevel;
  BOOL res = cfg->GetDataByKey(CFG_KEY_LOGGER_MAX_TRACE_LEVEL, slevel);
  PASSERTSTREAM_AND_RETURN_VALUE(!res,
    "GetDataByKey: " << CFG_KEY_LOGGER_MAX_TRACE_LEVEL,
    level);

  DWORD dlevel = CTrace::GetTraceLevelByName(slevel.c_str());
  PASSERTSTREAM_AND_RETURN_VALUE(NUM_OF_TRACE_TYPES == dlevel,
      "GetTraceLevelByName: " << slevel,
      level);

  if ((eLogLevel) dlevel < level)
    level = (eLogLevel) dlevel;

  return level;
}

STATUS CLoggerManager::UpdateLogLevel(eLogLevel new_level,
                                      eLogLevel& old_level)
{
  old_level = m_proc->GetMaxLogLevel();

  // Checks that the level is changed.
  if (old_level == new_level)
    return STATUS_OK;

  // Updates the log level to all MCMS processes.
  for (eProcessType i = eProcessMcmsDaemon; i < NUM_OF_PROCESS_TYPES; ++i)
  {
    if (!CProcessBase::IsProcessAlive(i))
      continue;

    CSegment* msg = new CSegment;
    *msg << static_cast<unsigned int>(new_level);
    CTaskApi::SendMsgWithTrace(i, eManager, msg, LOGGER_ALL_MAX_TRACE_LEVEL);
  }

  m_bIsMfaUpdated = SendMinimumConfiguredLevelToMFA(m_proc->GetLog4cxxConfiguration(),new_level);

  TRACESTRFUNC(eLevelError) << "Trace Max Level is changed from "
                            << CTrace::GetTraceLevelNameByValue(old_level)
                            << " to "
                            << CTrace::GetTraceLevelNameByValue(new_level);

  return STATUS_OK;
}

void CLoggerManager::OnSystemLoadAverage(CSegment* msg)
{
  if(m_bIsMfaUpdated == STATUS_FAIL)
  {
	  m_bIsMfaUpdated = SendMinimumConfiguredLevelToMFA(m_proc->GetLog4cxxConfiguration(),m_proc->GetMaxLogLevel());
  }

  TRACECOND_AND_RETURN(!m_enable_update_level,
    "Dynamic update trace level is disabled");

  static unsigned int upper;
  static unsigned int lower;

  // Calculates the threshold values only first time
  if (0 == upper)
  {
    unsigned int threshold;
    const char   key[] = CFG_KEY_LOGGER_SYSTEM_LOAD_AVERAGE_THRESHOLD;

    CSysConfig* cfg = m_proc->GetSysConfig();
    PASSERT_AND_RETURN(NULL == cfg);

    BOOL res = cfg->GetDWORDDataByKey(key, threshold);
    PASSERTSTREAM_AND_RETURN(!res, "GetDWORDDataByKey: " << key);

    STATUS stat = GetCPUUsageThreshold(threshold, upper, lower);
    if (STATUS_OK != stat)
      return;
  }

  // Extracts current load
  unsigned int load;
  *msg >> load;

  // Sends update to clients of all modules if needed
  eLogLevel dummy;
  eLogLevel level = GetTraceLevelByLoad(load, upper);
  if (STATUS_OK != UpdateLogLevel(level, dummy))
    return;

  std::ostringstream sysmsg;
  sysmsg << "Load average " << load
         << ", threshold upper " << upper
         << ", threshold lower " << lower;

  bool flag;
  if (load > upper)
  {
    flag = true;
  }
  else if (load < lower)
  {
    flag = false;
  }
  else
  {
    // Does nothing on hysteresis gap.
    TRACESTRFUNC(eLevelError) << sysmsg.str() << "...";
    return;
  }

  // Does nothing if status is not changed.
  if (flag == m_proc->GetDropMessageFlag())
  {
    TRACESTRFUNC(eLevelError) << sysmsg.str() << "...";
    return;
  }

  // Blocks/Unblocks processing of messages at server.
  std::ostringstream logmsg;

  if (flag)
  {
    logmsg << GetAlarmDescription(LOG_IS_DISABLED_BY_SYSTEM_LOAD)
           << ". New messages will be dropped.";
  }
  else
  {
    logmsg << "Logger is enabled. There were dropped ";

    unsigned long long              total = 0;
    std::vector<unsigned long long> stat = m_proc->GetDropMessageStat();
    for (unsigned int i = 0; i < stat.size(); i++)
    {
      total += stat[i];

      logmsg << CLoggerProcess::SenderToStr(static_cast<CLoggerProcess::eSender>(i))
             << " " << stat[i] << ((i != stat.size() - 1) ? ", " : " messages");
    }

    logmsg << ", total " << total << ".";
  }

  // Writes as debug assert.
  PAssert(__FILE__,
          __LINE__,
          this,
          LOG_IS_DISABLED_BY_SYSTEM_LOAD,
          logmsg.str().c_str(),
          YES);

  // Writes probably last message before changing the flag.
  TRACESTRFUNC(eLevelError) << logmsg.str() << " " << sysmsg.str() << ".";

  // Gives an opportunity to log last message before switch off.
  if (flag)
    StartTimer(LOGGER_DROP_MESSAGE_FLAG_ON_TIMER, 1);
  else
    m_proc->SetDropMessageFlag(false);
}

void CLoggerManager::OnDropMessageFlagOnTimer(CSegment*)
{
  m_proc->SetDropMessageFlag(true);
}

BOOL CLoggerManager::CheckIfNewHeaderIsNeeded(const COMMON_HEADER_S& commonHeader,
                                              const PHYSICAL_INFO_HEADER_S&
                                              physicalHeader,
                                              const TRACE_HEADER_S& traceHeader,
                                              const MCMS_TRACE_HEADER_S&
                                              mcmsHeader)
{
	m_prev_traceHeader.m_processMessageNumber = traceHeader.m_processMessageNumber;

	return (memcmp(&commonHeader,  &m_prev_commonHeader,  sizeof m_prev_commonHeader) ||
			memcmp(&physicalHeader,&m_prev_physicalHeader,sizeof m_prev_physicalHeader) ||
			memcmp(&traceHeader,   &m_prev_traceHeader,   sizeof m_prev_traceHeader) ||
			memcmp(&mcmsHeader,    &m_prev_mcmsHeader,    sizeof m_prev_mcmsHeader));
}

void CLoggerManager::InitLog4cxxConfiguration()
{
  TRACESTR(eLevelInfoNormal) << "\nCLoggerManager::InitLog4cxxConfiguration";
  STATUS retStatus = STATUS_OK;

  CLog4cxxConfiguration log4cxxConfiguration;
  retStatus =
    log4cxxConfiguration.ReadXmlFile(LOGGER_CFG_FILE,
                                     eNoActiveAlarm,
                                     eRenameFile);

  if (retStatus == STATUS_OK)
	 m_proc->GetLog4cxxConfiguration()->CopyValue(&log4cxxConfiguration);


  TRACESTR(eLevelInfoNormal) << "\nCLoggerManager:- InitLog4cxxConfiguration: read file status = " << retStatus;

  if ((STATUS_FILE_NOT_EXIST == retStatus) ||
      (STATUS_OPEN_FILE_FAILED == retStatus))  // no file exists (yet) - create a default, hard-coded MngmntNetworkInterface
  {
    if (STATUS_FILE_NOT_EXIST == retStatus)
    {
      TRACESTR(eLevelInfoNormal) << "\nCLoggerManager- InitLog4cxxConfiguration: STATUS_FILE_NOT_EXIST";
    }

    if (STATUS_OPEN_FILE_FAILED == retStatus)
    {
      TRACESTR(eLevelInfoNormal) << "\nCLoggerManager- InitLog4cxxConfiguration: STATUS_OPEN_FILE_FAILED";
    }

    m_proc->GetLog4cxxConfiguration()->WriteXmlFile(LOGGER_CFG_FILE,
                                                    "LOG_CONFIG_DATA");
  }

  ApplyAppendersConfiguration();

  for (unsigned int i = 0; i < CLoggerProcess::eLevelNum; i++)
  {
    for (unsigned int j = 0; j < 4; j++)
    {
      m_logLevelMapping[i][j] = 0;
    }
  }

  m_logLevelMapping[CLoggerProcess::eTRACE][0] = eLevelTrace;
  m_logLevelMapping[CLoggerProcess::eDEBUG][0] = eLevelDebug;
  m_logLevelMapping[CLoggerProcess::eINFO][0]  = eLevelInfoNormal;
  m_logLevelMapping[CLoggerProcess::eINFO][1]  = eLevelInfoHigh;
  m_logLevelMapping[CLoggerProcess::eWARN][0]  = eLevelWarn;
  m_logLevelMapping[CLoggerProcess::eERROR][0] = eLevelError;
  m_logLevelMapping[CLoggerProcess::eFATAL][0] = eLevelFatal;
  m_logLevelMapping[CLoggerProcess::eOFF][0] = eLevelOff;

  m_iMsgAvgTimerCount = 0;
  m_bStartLogStaticTimer = FALSE;

  CheckDiskSpaceForMaxLogSize(*m_proc->GetLog4cxxConfiguration(),true);
}

bool CLoggerManager::GetLog4cxxLogLevelAccordingRMXLogLevel(int rmx_log_level,
                                                            int& log4cxx_log_level)
{
  bool bFound = true;
  for (unsigned int i = 0; i < CLoggerProcess::eLevelNum; i++)
  {
    for (unsigned int j = 0; j < 4; j++)
    {
      if (m_logLevelMapping[i][j] == rmx_log_level)
      {
        log4cxx_log_level = i;
        return bFound;
      }
    }
  }

  log4cxx_log_level = -1;
  bFound = false;
  return bFound;
}

void CLoggerManager::CheckFullValidityForUDPAppender(CLog4cxxConfiguration* logCondig,STATUS &status)
{
	//This extra validity is to prevent a coredump from log4cxx library (that was ported in the past) in case 0.0.0.0:0 was sent by client
	if (!(logCondig->GetUDPAppender()->IsEnabled()))
		return;
	std::string str = logCondig->GetUDPAppender()->GetIP();
	char *cpy = new char[str.size()+1] ;
	strcpy(cpy, str.c_str());
	char * pch = strtok (cpy,".");
	if (pch != NULL &&  (TRUE == CObjString::IsNumeric(pch)))
	{
		if (atoi(pch) < 1 || atoi(pch) > 255)
			status = STATUS_IP_ADDRESS_NOT_VALID;
	}

	DWORD port = logCondig->GetUDPAppender()->GetPort();
	if (port < 1 || port > 4095)
			status = STATUS_IP_ADDRESS_NOT_VALID;

	delete []cpy;
}

STATUS CLoggerManager::OnServerSetLoggerConfiguration(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() != SUPER)
	{
		TRACEINTO << "No permission to set logger configuration";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	TRACEINTO << ".";

	STATUS status = STATUS_OK;

	CLog4cxxConfiguration* pLogcxxConfiguration = new CLog4cxxConfiguration();
	pLogcxxConfiguration->CopyValue((CLog4cxxConfiguration*)pRequest->GetRequestObject());

	//Ugly pacth to disable the SysLogAppender till the core dump will be fixed
	//VNGFE-6838 - System had 9 core dumps due to lack of external syslog support
	pLogcxxConfiguration->GetSysLogAppender()->SetEnabled(0);

	CheckFullValidityForUDPAppender(pLogcxxConfiguration,status);
	if (status != STATUS_OK)
	{
		delete pLogcxxConfiguration;
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(status);
		return status;
	}
	status = CheckDiskSpaceForMaxLogSize(*pLogcxxConfiguration);
	if (status != STATUS_OK)
	{
		delete pLogcxxConfiguration;
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(status);
		return status;
	}

	m_bIsMfaUpdated = SendMinimumConfiguredLevelToMFA(pLogcxxConfiguration,m_proc->GetMaxLogLevel());

	m_proc->GetLog4cxxConfiguration()->CopyValue(pLogcxxConfiguration);
	ApplyAppendersConfiguration();

	m_FileService->RemoveFilesOnLogSizeChanged();
	pRequest->SetConfirmObject(pLogcxxConfiguration);
	pRequest->SetStatus(status);
	return status;
}

DWORD CLoggerManager::GetMinimumLogLevel (CAppenderConfiguration *appender_config,DWORD currentMinimumLevel)
{
	if (appender_config->IsEnabled())
	{
		DWORD minimumConfiguredLevel = 0;
		if (!appender_config->GetModuleByName(MEDIACARD_MODULE_NAME)->IsEnabled())
		{
			minimumConfiguredLevel = CLoggerProcess::eOFF;
		}
		else
		{
			if (currentMinimumLevel <= appender_config->GetModuleByName(MEDIACARD_MODULE_NAME)->GetLogLevel())
				minimumConfiguredLevel = appender_config->GetModuleByName(MEDIACARD_MODULE_NAME)->GetLogLevel();
		}
		return minimumConfiguredLevel;
	}
	return currentMinimumLevel;
}

STATUS CLoggerManager::SendMinimumConfiguredLevelToMFA(CLog4cxxConfiguration* logCondig, eLogLevel system_level)
{

	DWORD minimumConfiguredLevel = 0;
	minimumConfiguredLevel = GetMinimumLogLevel(logCondig->GetLocalLogAppender(),minimumConfiguredLevel);
	minimumConfiguredLevel = GetMinimumLogLevel(logCondig->GetUDPAppender(),minimumConfiguredLevel);
	minimumConfiguredLevel = GetMinimumLogLevel(logCondig->GetSysLogAppender(),minimumConfiguredLevel);

    if (minimumConfiguredLevel >= CLoggerProcess::eLevelNum)
      return STATUS_OK;

    minimumConfiguredLevel = m_logLevelMapping[minimumConfiguredLevel][0];

    eLogLevel newLevel = (system_level > ((eLogLevel)minimumConfiguredLevel)) ? ((eLogLevel)minimumConfiguredLevel) : system_level;

    CSegment* msg = new CSegment;
    *msg << static_cast<unsigned int>(newLevel);

    return CTaskApi::SendMsgWithTrace(eProcessCards,
	                             eManager,
	                             msg,
	                             LOGGER_CARDS_MAX_TRACE_LEVEL);

}

STATUS CLoggerManager::OnServerStartCSLogs(CRequest* pRequest)
{
  PTRACE(eLevelInfoNormal, "CLoggerManager::OnServerStartCSLogs");

  std::ostringstream cmd;
      cmd << MCU_MCMS_DIR+"/Bin/McuCmd cslog CS start all";
      std::string ans;
      STATUS stat;

      if (IsTarget() || eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
      {
    	  stat = SystemPipedCommand(cmd.str().c_str(), ans);
    	  //update status member (checked by  LoggerMonitor) and file to save state after reset
		  if (stat == STATUS_OK)
		  {
			 //update member
			 m_proc->GetLog4cxxConfiguration()->SetIsCSLogStarted(true);

			 //update file

			UpdateCSLogTraceFlagsInSystemCfg("5");

			PTRACE(eLevelInfoNormal, "CLoggerManager::OnServerStartCSLogs. SetIsCSLogStarted(true)");
		  }
		  else
		  {
			stat = STATUS_FAIL_TO_CHANGE_CS_LOG_MODE;
			ans.insert(0,"SystemPipedCommand failed - ");
			PTRACE(eLevelInfoNormal, ans.c_str());
		  }
      }
      //when simulation then doesn't really stop CS log but update status member anyway in order to show indication in EMA
      else
      {
			stat = STATUS_OK;
			//update member
			m_proc->GetLog4cxxConfiguration()->SetIsCSLogStarted(true);

			//update file to save state after reset
			UpdateCSLogTraceFlagsInSystemCfg("5");

			PTRACE(eLevelInfoNormal, "CLoggerManager::OnServerStartCSLogs. SetIsCSLogStarted(true)");
      }

	  pRequest->SetConfirmObject(new CDummyEntry);
	  pRequest->SetStatus(stat);

	  return STATUS_OK;
}


STATUS CLoggerManager::OnServerStopCSLogs(CRequest* pRequest)
{
  PTRACE(eLevelInfoNormal, "CLoggerManager::OnServerStopCSLogs");

  STATUS stat;
  std::ostringstream cmd;
  cmd << MCU_MCMS_DIR+"/Bin/McuCmd cslog CS stop all";
  std::string ans;
  if (IsTarget() || eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily())
  {
	  stat = SystemPipedCommand(cmd.str().c_str(), ans);
	  //update status member (checked by  LoggerMonitor) and file to save state after reset
	  if (stat == STATUS_OK)
	  {
		  //update member
		m_proc->GetLog4cxxConfiguration()->SetIsCSLogStarted(false);

		//update file
		UpdateCSLogTraceFlagsInSystemCfg("0");

		PTRACE(eLevelInfoNormal, "CLoggerManager::OnServerStopCSLogs. SetIsCSLogStarted(false)");
	  }
	  else
	  {
		stat = STATUS_FAIL_TO_CHANGE_CS_LOG_MODE;
		ans.insert(0,"SystemPipedCommand failed - ");
		PTRACE(eLevelInfoNormal, ans.c_str());
	  }
  }
  //when simulation then doesn't really stop CS log but update status member anyway in order to show indication in EMA
  else
  {
	  stat = STATUS_OK;
	  //update member
	  m_proc->GetLog4cxxConfiguration()->SetIsCSLogStarted(false);

	  //update file to save state after reset
	  UpdateCSLogTraceFlagsInSystemCfg("0");


	  PTRACE(eLevelInfoNormal, "CLoggerManager::OnServerStopCSLogs. SetIsCSLogStarted(false)");
  }

  pRequest->SetConfirmObject(new CDummyEntry);
  pRequest->SetStatus(stat);

  return STATUS_OK;
}


string CLoggerManager::GetProcessName(unsigned int processType)
{
	return (processType < sizeof(ProcessNames)/sizeof(ProcessNames[0]) ? ProcessNames[processType] : "");
}

// according to the module level and process check tag
// if yes, the return value should be the log4cxx level for output
// if not, it shoud return -1
int CLoggerManager::IsThisMsgNeedToBeSentToLoggerType(int log_level,
                                                        int src_id,
                                                        int processType,
                                                        CAppenderConfiguration *appenderConfig)
{
  int processIndOfAllInRmx =
    CLoggerProcess::GetProcessIndexByMainEntity((eMainEntities)src_id,
                                                processType);

  if ((CLoggerProcess::eUtility < processIndOfAllInRmx) && (src_id == eMcms))
    return -1;

  int  log4cxx_level_of_the_trace;
  bool bRet = GetLog4cxxLogLevelAccordingRMXLogLevel(log_level,
                                                     log4cxx_level_of_the_trace);

  if (log4cxx_level_of_the_trace == -1 || (bRet == false))
  {
    return -1;
  }

  const char* processName =
    CLoggerProcess::GetProcessNameByMainEntity((eMainEntities)src_id,
                                               processType);

  // module its level
  CModuleContent* pModule =
      GetModulePtrByProcessByMainEntity(appenderConfig,
                                        (eMainEntities) src_id);

  if (pModule)
  {
    if (!pModule->IsEnabled())
      return -1;

    if (pModule->ValidateLogMsg(processIndOfAllInRmx,
                                log4cxx_level_of_the_trace,src_id))
    {
      return log4cxx_level_of_the_trace;
    }

    if ((CLoggerProcess::eEmbeddedApacheModule < processIndOfAllInRmx) &&
        (strcmp(pModule->GetModuleName(), MEDIACARD_MODULE_NAME) == 0))
    {
      if (pModule->GetProcessList().IsSpecifiedProcessChecked(CLoggerProcess::eMediaCardNA,
                                                              MEDIACARD_MODULE_NAME,src_id))
        return log4cxx_level_of_the_trace;
    }
  }

  return -1;
}

CModuleContent* CLoggerManager::GetModulePtrByProcessByMainEntity(CAppenderConfiguration* pAppender,
                                                                  eMainEntities mainEntityType)
{
  switch (mainEntityType)
  {
    case eMcms:
      return pAppender->GetModuleByName(MCMS_MODULE_NAME);

    case eEma:
      return pAppender->GetModuleByName(EMA_MODULE_NAME);

    case eCM_Switch:
    case eShelf:
    case eArtEntity:
    case eVideoEntity:
    case eCardManagerEntity:
    case eRTMEntity:
    case eMuxEntity:
    case eMpl:
    case eVmpEntity:
	case eAmpEntity:
	case eMpProxyEntity:
	case eCentral_signaling:
      return pAppender->GetModuleByName(MEDIACARD_MODULE_NAME);
    default:
      return NULL;
  } // switch

  return NULL;
}

void CLoggerManager::ApplySysLogAppenderConfiguration()
{
  if (m_proc->GetLog4cxxConfiguration()->GetSysLogAppender()->IsEnabled())
  {
    PTRACE(eLevelInfoNormal,
           "CLoggerManager::ApplySysLogAppenderConfiguration: SysLog Appender is enabled");
    m_xml_builder.ClearAllSysLogAppenders();

    char buff[32];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "%s", "appSYSLOG");

    char port[32];
    memset(port, 0, sizeof(port));
    sprintf(port, "%d",
            m_proc->GetLog4cxxConfiguration()->GetSysLogAppender()->GetPort());

    m_xml_builder.AddSysLogAppender(buff,
                                 m_proc->GetLog4cxxConfiguration()->GetSysLogAppender()->GetIP(),
                                 port,
                                 "");

  }
  else
  {
    PTRACE(eLevelInfoNormal,
           "CLoggerManager::ApplySysLogAppenderConfiguration: SysLog Appender is disabled");
    DisableSysLogAppender();
  }

  m_proc->GetLog4cxxConfiguration()->WriteXmlFile(LOGGER_CFG_FILE,
                                                  "LOG_CONFIG_DATA");
}

void CLoggerManager::ApplyUDPAppenderConfiguration()
{
  if (m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->IsEnabled())
  {
    PTRACE(eLevelInfoNormal,
           "CLoggerManager::ApplyUDPAppenderConfiguration: UDP Appender is enabled");
    m_xml_builder.ClearAllUDPAppenders();

    char buff[32];
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "udp_appender_%d",
            m_xml_builder.GetUDPAppendersVec().size() + 1);

    char port[32];
    memset(port, 0, sizeof(port));
    sprintf(port, "%d",
            m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetPort());

    m_xml_builder.AddUDPAppender(buff,
                                 m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetIP(),
                                 port,
                                 "");

    //m_xml_builder.GenerateFile(LOG4CXX_APPENDER_FILE);
    //DOMConfigurator::configure("Cfg/log4config.ref.xml");

    ULONGLONG now = SystemGetTickCount().GetSeconds();
    m_udpAppenderStopTime = now + m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetDuration() * 3600;

    //StartTimer(LOGGER_UDPAPPENDER_TIMER, LOGGER_UDPAPPENDER_TIMEOUT);
  }
  else
  {
    PTRACE(eLevelInfoNormal,
           "CLoggerManager::ApplyUDPAppenderConfiguration: UDP Appender is disabled");
    DisableUDPAppender();
  }

  m_proc->GetLog4cxxConfiguration()->WriteXmlFile(LOGGER_CFG_FILE,
                                                  "LOG_CONFIG_DATA");
}

STATUS CLoggerManager::HandleTerminalTestUDPConfiguration(CTerminalCommand& cmd,
                                                          std::ostream& ans)
{
  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->SetEnabled(1);
  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->SetDuration(1);
  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->SetIP("172.21.116.23");
  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->SetPort(4321);
  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetModuleByName(MCMS_MODULE_NAME)->SetEnabled(0);
  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetModuleByName(MCMS_MODULE_NAME)->SetLogLevel(CLoggerProcess::eFATAL);

  CProcessList& _process_list =
    m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetModuleByName(MCMS_MODULE_NAME)->GetProcessList();

  for (unsigned int ii = eProcessMcmsDaemon; ii < NUM_OF_PROCESS_TYPES; ii++)
  {
    int indexx = CLoggerProcess::GetProcessIndexByMainEntity(eMcms, ii);
    CProcessItemData* ppItem = _process_list.GetProcessItemByProcessIndex(indexx);
    if (ppItem == NULL)
      continue;

    ppItem->SetEnabled(0);
    if (ii == eProcessConfParty || eProcessMcuMngr == ii || eProcessCSApi == ii)
      ppItem->SetEnabled(1);
  }

  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetModuleByName(EMA_MODULE_NAME)->SetEnabled(0);
  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetModuleByName(MEDIACARD_MODULE_NAME)->SetEnabled(1);
  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetModuleByName(MEDIACARD_MODULE_NAME)->SetLogLevel(CLoggerProcess::eFATAL);
  CProcessList& _media_card_process_list =
    m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->GetModuleByName(MEDIACARD_MODULE_NAME)->GetProcessList();
  for (unsigned int j = eTheOneTheOnlyMplProcess; j < NumOfMplProcesses; j++)
  {
    int indexx = CLoggerProcess::GetProcessIndexByMainEntity(eMpl, j);
    CProcessItemData* ppItem = _media_card_process_list.GetProcessItemByProcessIndex(indexx);
    if (ppItem == NULL)
      continue;

    ppItem->SetEnabled(1);
  }

  ApplyUDPAppenderConfiguration();
  ans<<"msg avg:"<< m_proc->GetLog4cxxConfiguration()->GetMsgAvgRate()<<"\n";

  return STATUS_OK;
}

void CLoggerManager::OnUDPAppenderTimer()
{
  DWORD now = SystemGetTickCount().GetSeconds();
  if (now < m_udpAppenderStopTime)
    StartTimer(LOGGER_UDPAPPENDER_TIMER, LOGGER_UDPAPPENDER_TIMEOUT);
  else
    DisableUDPAppender();
}

void CLoggerManager::DisableUDPAppender()
{
  DeleteTimer(LOGGER_UDPAPPENDER_TIMER);

  m_xml_builder.ClearAllUDPAppenders();
  m_xml_builder.GenerateFile(LOG4CXX_APPENDER_FILE);
  DOMConfigurator::configure(LOG4CXX_APPENDER_FILE);

  m_proc->GetLog4cxxConfiguration()->GetUDPAppender()->SetEnabled(0);
}

void CLoggerManager::DisableSysLogAppender()
{
  //DeleteTimer(LOGGER_UDPAPPENDER_TIMER);

  m_xml_builder.ClearAllSysLogAppenders();
  m_xml_builder.GenerateFile(LOG4CXX_APPENDER_FILE);
  DOMConfigurator::configure(LOG4CXX_APPENDER_FILE);

  m_proc->GetLog4cxxConfiguration()->GetSysLogAppender()->SetEnabled(0);
}

void CLoggerManager::ApplyAppendersConfiguration()
{
  ApplyUDPAppenderConfiguration();
  ApplySysLogAppenderConfiguration();


  m_xml_builder.GenerateFile(LOG4CXX_APPENDER_FILE);
  DOMConfigurator::configure(LOG4CXX_APPENDER_FILE);

  if (m_proc->GetLog4cxxConfiguration()->GetLocalLogAppender()->IsEnabled())
  {
	  Startlogging();;
  }
  else
  {
	  Stoplogging();
  }
}

void  CLoggerManager::SendToAppenders(unsigned int src_id,
                                     unsigned int log_level,
                                     unsigned int process_type,
                                     const char* buf,
                                     LoggerPtr logger,
                                     CAppenderConfiguration *appenderConfig)
{
	if (!appenderConfig->IsEnabled())
		return;

	eProcessType processType = (eProcessType)process_type;
	int log4cxx_levevl = IsThisMsgNeedToBeSentToLoggerType(log_level, src_id,
                                                           process_type,appenderConfig);
	switch (log4cxx_levevl)
	{
		case -1:
			break;
		case CLoggerProcess::eTRACE:
			LOG4CXX_TRACE(logger, buf);
			break;
		case CLoggerProcess::eDEBUG:
			LOG4CXX_DEBUG(logger, buf);
			break;
		case CLoggerProcess::eINFO:
			LOG4CXX_INFO(logger, buf);
			break;
		case CLoggerProcess::eWARN:
			LOG4CXX_WARN(logger, buf);
			break;
		case CLoggerProcess::eERROR:
			LOG4CXX_ERROR(logger, buf);
			break;
		case CLoggerProcess::eFATAL:
			LOG4CXX_FATAL(logger, buf);
			break;
		default:
			LOG4CXX_TRACE(logger, buf);
			break;
	}

	return;
}

void CLoggerManager::OnMsgAvgCount()
{
  m_iMsgAvgTimerCount++;
  m_proc->GetLog4cxxConfiguration()->SetMsgAvgRate(m_proc->GetNumOfAllTraces()/(m_iMsgAvgTimerCount*LOGGER_TIMER_COUNT));
  StartTimer(LOGGER_MSG_AVG_COUNT_TIMER, LOGGER_MSG_AVG_COUNT_TIMEOUT);
}

void CLoggerManager::UpdateCSLogTraceFlagsInSystemCfg(std::string level)
{
	CSysConfigEma *cfg = new CSysConfigEma();
	cfg->LoadFromFile(eCfgParamUser);
	CCfgData *cfgData = NULL;
	if (!cfg->IsParamExist("trace1level"))
		cfg->AddParamInFileNonVisible("CS_MODULE_PARAMETERS","trace1level","all.StackMsgs.logfileDebugLevel."+ level,eCfgParamUser,ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	else
	{
		cfgData = cfg->GetCfgEntryByKey("trace1level");
		cfgData->SetData("all.StackMsgs.logfileDebugLevel."+level);
	}
	if (!cfg->IsParamExist("trace2level"))
		cfg->AddParamInFileNonVisible("CS_MODULE_PARAMETERS","trace2level","all.XmlTrace.logfileDebugLevel."+level,eCfgParamUser,ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	else
	{
		cfgData = cfg->GetCfgEntryByKey("trace2level");
		cfgData->SetData("all.XmlTrace.logfileDebugLevel."+level);
	}
	if (!cfg->IsParamExist("trace3level"))
		cfg->AddParamInFileNonVisible("CS_MODULE_PARAMETERS","trace3level","siptask.SipMsgsTrace.logfileDebugLevel."+level,eCfgParamUser,ONE_LINE_BUFFER_LENGTH,eCfgParamResponsibilityGroup_SwSysInfraApp,eCfgParamDataString);
	else
	{
		cfgData = cfg->GetCfgEntryByKey("trace3level");
		cfgData->SetData("siptask.SipMsgsTrace.logfileDebugLevel."+level);
	}
	cfg->SaveToFile("Cfg/SystemCfgUserTmp.xml");

	// ===== 1. fill the Segment
	CSegment*  pRetParam = new CSegment;

	if (level == "0") //cs loggs are of
	{
	    *pRetParam << (BOOL)FALSE;
	}
	else
	{
		*pRetParam << (BOOL)TRUE;
	}
	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcuMngr, eManager);
	STATUS res = pMbx->Send(pRetParam, LOGGER_TO_MCUMNGR_CS_LOG_STATE, &GetRcvMbx());
}

void CLoggerManager::OnSetLoggerLevelExplicit(CSegment* pSeg)
{
	PASSERT_AND_RETURN(NULL == pSeg);

	DWORD req_level = 0;
	*pSeg >> req_level;
	 if (!CTrace::IsTraceLevelValid(req_level)){
		 PASSERT_AND_RETURN(req_level+1);
	}

	UpdateExplicitLogLevel(static_cast<eLogLevel>(req_level));
}

void CLoggerManager::UpdateExplicitLogLevel(eLogLevel new_level)
{

  eLogLevel old_level;
  STATUS      stat = UpdateLogLevel(new_level, old_level);

  switch (new_level)
  {
  case eLevelInfoNormal:
    m_enable_update_level = true;
    break;

  default:
    m_enable_update_level = false;
    break;
  }

  TRACESTRFUNC(eLevelError) << "set_level: " << m_proc->GetStatusAsString(stat)
      << ": from " << CTrace::GetTraceLevelNameByValue(old_level)
      << " (" << old_level << ")"
      << " to " << CTrace::GetTraceLevelNameByValue(new_level)
      << " (" << new_level << ")"
      << " setting m_enable_update_level = " << m_enable_update_level;

}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CLoggerManager::OnUpdateCfgWriteAlternativeTout(CSegment* msg)
{
	StartTimer(LOGGER_UPDATE_WRITE_CFG_TIMER, UPDATE_WRITE_CFG_TIME);

	BOOL writeToFile;
	bool res = m_proc->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_WRITE_LOG_TO_FILE, writeToFile);
	bool newValue = (writeToFile)? true : false;
	if (newValue != m_cfgWriteToAlternativeFileEnabled)
	{
		TRACEINTO << "Configuration has changed, " << CFG_KEY_ENABLE_WRITE_LOG_TO_FILE << "=" << (newValue?"YES":"NO");
		m_cfgWriteToAlternativeFileEnabled = newValue;
	}
}

