// CardsMonitor.cpp: implementation of the CardsMonitor class.
//
//////////////////////////////////////////////////////////////////////


#include "CardsMonitor.h"
#include "CommCardDBGet.h"
#include "StatusesGeneral.h"
#include "Request.h"
#include "OpcodesMcmsCommon.h"



////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CCardsMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CCardsMonitor::HandlePostRequest)
PEND_MESSAGE_MAP(CCardsMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CCardsMonitor)
	ON_TRANS("TRANS_CARDS_LIST","GET", CCommCardDBGet, CCardsMonitor::OnServerCardsList)
//	ON_TRANS("TRANS_CARD","GET", CCommCardGet, CCardsMonitor::OnServerCardStatuses)
END_TRANSACTION_FACTORY




////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CardsMonitorEntryPoint(void* appParam)
{  
	CCardsMonitor *monitorTask = new CCardsMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCardsMonitor::CCardsMonitor()
{

}

/////////////////////////////////////////////////////////////////////////////
CCardsMonitor::~CCardsMonitor()
{

}

/////////////////////////////////////////////////////////////////////////////
STATUS CCardsMonitor::OnServerCardsList(CRequest* pGetRequest)
{
	CCommCardDBGet* pCommCardDBGet = new CCommCardDBGet;
	*pCommCardDBGet = *(CCommCardDBGet*)pGetRequest->GetRequestObject() ; //To set the updateCounter
	pGetRequest->SetStatus(STATUS_OK);
	pGetRequest->SetConfirmObject(pCommCardDBGet);

 	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CCardsMonitor::OnServerCardStatuses(CRequest* pGetRequest)
{
/*	CCommCardGet* pCardStatusesGet = new CCommCardGet;
	*pCardStatusesGet = (CCommCardGet*)pGetRequest->GetRequestObject() ; //To set the updateCounter
	
	const WORD boardId = pCardStatusesGet->GetBoardId();
	


	pGetRequest->SetStatus(STATUS_OK);
	pGetRequest->SetConfirmObject(pCommCardDBGet);
*/
 	return STATUS_OK;
}
