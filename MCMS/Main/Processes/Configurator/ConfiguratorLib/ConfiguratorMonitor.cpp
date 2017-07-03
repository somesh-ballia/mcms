// ConfiguratorMonitor.cpp: implementation of the ConfiguratorMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "ConfiguratorMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CConfiguratorMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CConfiguratorMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CConfiguratorMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CConfiguratorMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CConfiguratorMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void ConfiguratorMonitorEntryPoint(void* appParam)
{  
	CConfiguratorMonitor *monitorTask = new CConfiguratorMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CConfiguratorMonitor::CConfiguratorMonitor()
{

}

CConfiguratorMonitor::~CConfiguratorMonitor()
{

}

