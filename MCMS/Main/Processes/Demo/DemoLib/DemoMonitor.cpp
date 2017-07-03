// DemoMonitor.cpp

#include "DemoMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

PBEGIN_MESSAGE_MAP(CDemoMonitor)
  ONEVENT(XML_REQUEST, IDLE, CDemoMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CDemoMonitor, CAlarmableTask);

BEGIN_GET_TRANSACTION_FACTORY(CDemoMonitor)
// ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CDemoMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY

void DemoMonitorEntryPoint(void* appParam)
{
  CDemoMonitor* monitorTask = new CDemoMonitor;
  monitorTask->Create(*(CSegment*)appParam);
}

CDemoMonitor::CDemoMonitor()
{}

CDemoMonitor::~CDemoMonitor()
{}


