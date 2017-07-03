// DemoMonitor.cpp

#include "McmsNetworkMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

PBEGIN_MESSAGE_MAP(CMcmsNetworkMonitor)
  ONEVENT(XML_REQUEST, IDLE, CMcmsNetworkMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CMcmsNetworkMonitor, CAlarmableTask);

BEGIN_GET_TRANSACTION_FACTORY(CMcmsNetworkMonitor)
// ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CDemoMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY

void McmsNetworkMonitorEntryPoint(void* appParam)
{
  CMcmsNetworkMonitor* monitorTask = new CMcmsNetworkMonitor;
  monitorTask->Create(*(CSegment*)appParam);
}

CMcmsNetworkMonitor::CMcmsNetworkMonitor()
{}

CMcmsNetworkMonitor::~CMcmsNetworkMonitor()
{}


