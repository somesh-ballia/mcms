// MediaMngrMonitor.cpp: implementation of the MediaMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "MediaMngrMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CMediaMngrMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CMediaMngrMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CMediaMngrMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CMediaMngrMonitor)

END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void MediaMngrMonitorEntryPoint(void* appParam)
{  
	CMediaMngrMonitor *monitorTask = new CMediaMngrMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMediaMngrMonitor::CMediaMngrMonitor()
{

}

CMediaMngrMonitor::~CMediaMngrMonitor()
{

}
