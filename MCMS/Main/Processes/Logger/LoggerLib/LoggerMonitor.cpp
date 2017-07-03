// LoggerMonitor.cpp

#include "LoggerMonitor.h"

#include "Macros.h"
#include "TaskApi.h"
#include "Request.h"
#include "TraceStream.h"
#include "LogFileListGet.h"
#include "OpcodesMcmsCommon.h"
#include "InitCommonStrings.h"
#include "OpcodesMcmsInternal.h"
#include "LoggerProcess.h"
#include "Log4cxxConfigurationDBGet.h"
#include "Log4cxxConfiguration.h"

PBEGIN_MESSAGE_MAP(CLoggerMonitor)
    ONEVENT(XML_REQUEST, IDLE, CLoggerMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CLoggerMonitor, CAlarmableTask);

BEGIN_GET_TRANSACTION_FACTORY(CLoggerMonitor)
  ON_TRANS("TRANS_LOG_FILE_LIST", "GET", CLogFileListGet, CLoggerMonitor::OnServerLoggerFileList)
  ON_TRANS("TRANS_LOGGER", "GET", CLog4cxxConfigurationDBGet, CLoggerMonitor::OnServerGetLoggerConfiguration)
END_TRANSACTION_FACTORY

void LoggerMonitorEntryPoint(void* appParam)
{
  CLoggerMonitor* monitorTask = new CLoggerMonitor;
  monitorTask->Create(*(CSegment*) appParam);
}

CLoggerMonitor::CLoggerMonitor(void)
{}

// Virtual
const char* CLoggerMonitor::NameOf(void) const
{
  return GetCompileType();
}

STATUS CLoggerMonitor::OnServerLoggerFileList(CRequest* pGetRequest)
{
  CLogFileListGet* getter = new CLogFileListGet;

  // To set the updateCounter
  *getter = *(CLogFileListGet*) pGetRequest->GetRequestObject();
  pGetRequest->SetConfirmObject(getter);

  return STATUS_OK;
}

STATUS CLoggerMonitor::OnServerGetLoggerConfiguration(CRequest* pGetRequest)
{
  CLoggerProcess* process = (CLoggerProcess*) CProcessBase::GetProcess();
  CLog4cxxConfigurationDBGet* pData = new CLog4cxxConfigurationDBGet;
  *pData = *(CLog4cxxConfigurationDBGet*) pGetRequest->GetRequestObject();
  pGetRequest->SetConfirmObject(pData);
  return STATUS_OK;
}
