// RtmIsdnMngrMonitor.cpp: implementation of the RtmIsdnMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "RtmIsdnMngrMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "Request.h"
#include "RtmIsdnServiceListGet.h"
#include "RtmIsdnSpanMapListGet.h"


////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CRtmIsdnMngrMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CRtmIsdnMngrMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CRtmIsdnMngrMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CRtmIsdnMngrMonitor)
	ON_TRANS("TRANS_ISDN_SERVICE_LIST",		"GET",	CRtmIsdnServiceListGet,	CRtmIsdnMngrMonitor::OnGetServiceList)
	ON_TRANS("TRANS_RTM_ISDN_SPAN_LIST",	"GET",	CRtmIsdnSpanMapListGet,	CRtmIsdnMngrMonitor::OnGetSpansList)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void RtmIsdnMngrMonitorEntryPoint(void* appParam)
{  
	CRtmIsdnMngrMonitor *monitorTask = new CRtmIsdnMngrMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRtmIsdnMngrMonitor::CRtmIsdnMngrMonitor()
{

}

CRtmIsdnMngrMonitor::~CRtmIsdnMngrMonitor()
{

}


///////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrMonitor::OnGetServiceList(CRequest* pGetRequest)
{	
	CRtmIsdnServiceListGet* pServiceListGet = new CRtmIsdnServiceListGet;
	*pServiceListGet = *(CRtmIsdnServiceListGet*)pGetRequest->GetRequestObject();

	pGetRequest->SetStatus(STATUS_OK);
	pGetRequest->SetConfirmObject(pServiceListGet);
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrMonitor::OnGetSpansList(CRequest* pGetRequest)
{
	CRtmIsdnSpanMapListGet* pIsdnSpanMapListGet = new CRtmIsdnSpanMapListGet;
	*pIsdnSpanMapListGet = *(CRtmIsdnSpanMapListGet*)pGetRequest->GetRequestObject();

	pGetRequest->SetStatus(STATUS_OK);
	pGetRequest->SetConfirmObject(pIsdnSpanMapListGet);

 	return STATUS_OK;	
}
