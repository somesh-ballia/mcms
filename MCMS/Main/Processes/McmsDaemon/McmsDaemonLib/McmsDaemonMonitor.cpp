// McmsDaemonMonitor.cpp: implementation of the McmsDaemonMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "McmsDaemonMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CMcmsDaemonMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CMcmsDaemonMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CMcmsDaemonMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CMcmsDaemonMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CMcmsDaemonMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void McmsDaemonMonitorEntryPoint(void* appParam)
{  
	CMcmsDaemonMonitor *monitorTask = new CMcmsDaemonMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMcmsDaemonMonitor::CMcmsDaemonMonitor()
{

}

CMcmsDaemonMonitor::~CMcmsDaemonMonitor()
{

}

