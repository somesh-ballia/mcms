// FaultsMonitor.cpp: implementation of the FaultsMonitor class.
//
//////////////////////////////////////////////////////////////////////


#include "Macros.h"
#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "TaskApi.h"
#include "Request.h"
#include "CommFaultsDBGet.h"
#include "FaultsMonitor.h"
#include "ApacheDefines.h"
#include "ApiStatuses.h"
#include "DummyEntry.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CFaultsMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CFaultsMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CFaultsMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CFaultsMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CFaultsMonitor::HandleOperLogin)
	ON_TRANS("TRANS_FAULTS_LIST","GET", CCommFaultsDBGet, CFaultsMonitor::OnServerFaultsList) //What is th meening CCommConfDBGet
	ON_TRANS("TRANS_FAULTS_LIST_SHORT","GET", CCommFaultsShortDBGet, CFaultsMonitor::OnServerFaultsListShort) //What is th meening CCommConfDBGet
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void FaultsMonitorEntryPoint(void* appParam)
{  
	CFaultsMonitor *monitorTask = new CFaultsMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CFaultsMonitor::CFaultsMonitor()
{

}

//////////////////////////////////////////////////////////////////////
CFaultsMonitor::~CFaultsMonitor()
{

}

//////////////////////////////////////////////////////////////////////
STATUS CFaultsMonitor::OnServerFaultsList(CRequest* pGetRequest)
{
	if (pGetRequest->GetAuthorization() == SUPER || pGetRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		//PTRACE(eLevelInfoHigh, "CFaultsMonitor::OnServerFaultsList  ");
	
		CCommFaultsDBGet* pCommFaultsDBGet = new CCommFaultsDBGet;
		*pCommFaultsDBGet = *(CCommFaultsDBGet*)pGetRequest->GetRequestObject() ;
		pGetRequest->SetConfirmObject(pCommFaultsDBGet);
	}
	else
	{
		pGetRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelInfoNormal,"CFaultsMonitor::OnServerFaultsList: No permission to get second level faults list");
		pGetRequest->SetStatus(STATUS_NO_PERMISSION);
	}
 	return STATUS_OK;	
}

//////////////////////////////////////////////////////////////////////
STATUS CFaultsMonitor::OnServerFaultsListShort(CRequest* pGetRequest)
{
	//PTRACE(eLevelInfoHigh, "CFaultsMonitor::OnServerFaultsListShort  ");

	CCommFaultsShortDBGet* pCommFaultsShortDBGet = new CCommFaultsShortDBGet;
	*pCommFaultsShortDBGet = *(CCommFaultsShortDBGet*)pGetRequest->GetRequestObject() ;
	pGetRequest->SetConfirmObject(pCommFaultsShortDBGet);

 	return STATUS_OK;	
}






