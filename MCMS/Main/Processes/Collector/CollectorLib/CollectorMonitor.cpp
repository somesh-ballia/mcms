// CollectorMonitor.cpp: implementation of the CollectorMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "CollectorMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "DummyEntry.h"
#include "CollectorInfo.h"
#include "Request.h"
#include "CollectorInfoGet.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CCollectorMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CCollectorMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CCollectorMonitor,CStateMachine);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CCollectorMonitor)
ON_TRANS("TRANS_MCU","GET_COLLECT_INFO_SETTINGS", CCollectorInfoGet, CCollectorMonitor::OnGetCollectInfoSettings)
//ON_TRANS("TRANS_MCU","GET_COLLECT_INFO_ESTIMATED_SIZE", CDummyEntry, CCollectorMonitor::GetCollectInfoEstimatedSize)

END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CollectorMonitorEntryPoint(void* appParam)
{  
	CCollectorMonitor *monitorTask = new CCollectorMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCollectorMonitor::CCollectorMonitor()
{
}

//////////////////////////////////////////////////////////////////////
CCollectorMonitor::~CCollectorMonitor()
{

}

//////////////////////////////////////////////////////////////////////
STATUS CCollectorMonitor::OnGetCollectInfoSettings(CRequest *pGetRequest)
{
	STATUS status = STATUS_OK;

	CCollectorInfoGet* pCollectorInfoGet = new CCollectorInfoGet;
	*pCollectorInfoGet = *(CCollectorInfoGet*)pGetRequest->GetRequestObject() ;
	pGetRequest->SetConfirmObject(pCollectorInfoGet);

	return status;
}

//////////////////////////////////////////////////////////////////////
/*STATUS CCollectorMonitor::GetCollectInfoEstimatedSize(CRequest *pRequest)
{
	return STATUS_OK;
}*/

//////////////////////////////////////////////////////////////////////
