// EndpointsSimMonitor.cpp: implementation of the EndpointsSimMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "EndpointsSimMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CEndpointsSimMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    ,  CEndpointsSimMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CEndpointsSimMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CEndpointsSimMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CEndpointsSimMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY



////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void EndpointsSimMonitorEntryPoint(void* appParam)
{  
	CEndpointsSimMonitor *monitorTask = new CEndpointsSimMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CEndpointsSimMonitor::CEndpointsSimMonitor()
{

}

CEndpointsSimMonitor::~CEndpointsSimMonitor()
{

}

