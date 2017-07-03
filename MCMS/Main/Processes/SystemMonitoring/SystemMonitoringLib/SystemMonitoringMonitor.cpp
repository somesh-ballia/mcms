// SystemMonitoringMonitor.cpp: implementation of the SystemMonitoringMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "SystemMonitoringMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSystemMonitoringMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CSystemMonitoringMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CSystemMonitoringMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CSystemMonitoringMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CSystemMonitoringMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void SystemMonitoringMonitorEntryPoint(void* appParam)
{  
	CSystemMonitoringMonitor *monitorTask = new CSystemMonitoringMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSystemMonitoringMonitor::CSystemMonitoringMonitor()
{

}

CSystemMonitoringMonitor::~CSystemMonitoringMonitor()
{

}

