// LicenseServerMonitor.cpp

#include "LicenseServerMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

PBEGIN_MESSAGE_MAP(CLicenseServerMonitor)
  ONEVENT(XML_REQUEST, IDLE, CLicenseServerMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CLicenseServerMonitor, CAlarmableTask);

BEGIN_GET_TRANSACTION_FACTORY(CLicenseServerMonitor)
// ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CLicenseServerMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY

void LicenseServerMonitorEntryPoint(void* appParam)
{
  CLicenseServerMonitor* monitorTask = new CLicenseServerMonitor;
  monitorTask->Create(*(CSegment*)appParam);
}

CLicenseServerMonitor::CLicenseServerMonitor()
{}

CLicenseServerMonitor::~CLicenseServerMonitor()
{}


