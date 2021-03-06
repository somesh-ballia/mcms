// MCCFMngrMonitor.cpp: implementation of the MCCFMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "MCCFMngrMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CMCCFMngrMonitor)
	ONEVENT(XML_REQUEST    ,IDLE    , CMCCFMngrMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CMCCFMngrMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CMCCFMngrMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CMCCFMngrMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void MCCFMngrMonitorEntryPoint(void* appParam)
{
	CMCCFMngrMonitor *monitorTask = new CMCCFMngrMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMCCFMngrMonitor::CMCCFMngrMonitor()
{

}

CMCCFMngrMonitor::~CMCCFMngrMonitor()
{

}

