// CSMngrMonitor.cpp: implementation of the CSMngrMonitor class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Manages monitoring 
//========   ==============   =====================================================================

#include "CSMngrMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "IpService.h"
#include "CSMngrProcess.h"
#include "Request.h"
#include "CIPServiceListGet.h"
#include "CIPServiceFullListGet.h"
#include "SystemInterfaceListGet.h"
#include "PingData.h"
#include "StatusesGeneral.h"
#include "DummyEntry.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CCSMngrMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CCSMngrMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CCSMngrMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CCSMngrMonitor)
ON_TRANS("TRANS_IP_SERVICE_LIST", "GET", CIPServiceListGet, CCSMngrMonitor::GetIpServiceList)
ON_TRANS("TRANS_IP_SERVICE_LIST", "GET_FULL_IP_SERVICE_LIST", CIPServiceFullListGet, CCSMngrMonitor::OnFullServiceReq)
ON_TRANS("TRANS_MCU"           ,"GET_PING"                 , CPingGet			   , CCSMngrMonitor::GetPing)
ON_TRANS("TRANS_SYSTEM_INTERFACE_LIST", "GET", CSystemInterfaceListGet, CCSMngrMonitor::GetSystemInterfaceList)

//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CCSMngrMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CSMngrMonitorEntryPoint(void* appParam)
{  
	CCSMngrMonitor *monitorTask = new CCSMngrMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCSMngrMonitor::CCSMngrMonitor()
{
	
}

CCSMngrMonitor::~CCSMngrMonitor()
{

}

///////////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrMonitor::GetIpServiceList(CRequest* pGetRequest)
{	
	CIPServiceListGet* pCIPServiceListGet = (CIPServiceListGet*)(pGetRequest->GetRequestObject());
	CIPServiceListGet* pIPServiceListGet2 = new CIPServiceListGet(*pCIPServiceListGet);
	pGetRequest->SetConfirmObject(pIPServiceListGet2);
	pGetRequest->SetStatus(STATUS_OK);
	
	return STATUS_OK;
	
}

///////////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrMonitor::GetSystemInterfaceList(CRequest* pGetRequest)
{
	STATUS nRetStatus = STATUS_OK;

	CSystemInterfaceListGet *pListResponse = NULL;
	pListResponse = new CSystemInterfaceListGet();

	pGetRequest->SetConfirmObject(pListResponse);
	pGetRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrMonitor::OnFullServiceReq(CRequest *pRequest)
{	
	CIPServiceFullListGet *fullListReq = (CIPServiceFullListGet*)(pRequest->GetRequestObject());
	CIPServiceFullListGet *fullListRes = new CIPServiceFullListGet(*fullListReq);	
	pRequest->SetConfirmObject(fullListRes);
	pRequest->SetStatus(STATUS_OK);
	
	return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCSMngrMonitor::GetPing (CRequest *pRequest)
{
    CPingGet * pPingReqData = (CPingGet *)(pRequest->GetRequestObject());
    CPingGet * pPingData = new CPingGet (*pPingReqData);
    DWORD pingId = pPingData->GetPingId();
    CCSMngrProcess * pCSMngrProcess = dynamic_cast<CCSMngrProcess*>(CProcessBase::GetProcess());
    CPingData * currPingData = pCSMngrProcess->GetPing();
    STATUS pingStatus = STATUS_OK;
    if (!CPObject::IsValidPObjectPtr(currPingData))
    {
        PTRACE2INT (eLevelError, "CCSMngrMonitor::GetPing - m_currPingData not valid = ", (DWORD)currPingData);
        pPingData->SetPingStatus(STATUS_FAIL);
    }
    
    if (currPingData->GetPingId() == pingId)
    {
        currPingData->Dump ("CCSMngrMonitor::GetPing - same pingId as current", eLevelInfoNormal);
        pingStatus = currPingData->GetPingStatus();
        pPingData->SetPingStatus(pingStatus);

        /*************************************************************/
        /* 8.7.10 VNGR-15744 VNGR-15743 added by Rachel Cohen        */
        /* when we are in progress status which means we didn't get  */
        /* CS_PING_IND from CS we need to send EMA status = 1        */
        /* STATUS_IN_PROGRESS   .                                    */
        /*************************************************************/
        if (STATUS_IN_PROGRESS == pingStatus)
        {
          	 pRequest->SetConfirmObject(new CPingGet(*pPingData));
           	 pRequest->SetStatus(STATUS_IN_PROGRESS);
			 delete pPingData;
			 
           	 return STATUS_OK;
        }



        //if (IsValidTimer(PING_EMA_QUERY_TIMER))
        //    DeleteTimer (PING_EMA_QUERY_TIMER);
        m_state = IDLE;
        pCSMngrProcess->DeletePing();

    }
    else
    {
        PTRACE2INT (eLevelError, "CCSMngrMonitor::GetPing - different ping ID in query = ", pingId);
        currPingData->Dump ("CCSMngrMonitor::GetPing current ping is:", eLevelInfoNormal);
        pPingData->SetPingStatus(STATUS_FAIL);
    }
    pRequest->SetConfirmObject(new CPingGet(*pPingData));
    pRequest->SetStatus(STATUS_OK);

	delete pPingData;
	
    return STATUS_OK;
}
