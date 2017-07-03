// CollectorManager.cpp: implementation of the CCollectorManager class.
//
//////////////////////////////////////////////////////////////////////

#include "CollectorManager.h"
#include "CollectorInfo.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "SingleToneApi.h"
#include "McuMngrInternalStructs.h"
#include "Request.h"
#include "DummyEntry.h"
#include "ApiStatuses.h"

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CCollectorManager)
  ONEVENT(XML_REQUEST          , IDLE   , CCollectorManager::HandlePostRequest )
  ONEVENT(COLLECT_INFO_END_IND , ANYCASE, CCollectorManager::OnCollectInfoEndInd )
  ONEVENT(COLLECT_INFO_FAILED_IND , ANYCASE, CCollectorManager::OnCollectInfoFailedInd )
  ONEVENT(COLLECT_INFO_TEST    , ANYCASE, CCollectorManager::SimulateCollectInfo )
PEND_MESSAGE_MAP(CCollectorManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CCollectorManager)
  ON_TRANS("TRANS_MCU","COLLECT_INFO",    CInfoTimeInterval,  CCollectorManager::OnCollectInfo )
  ON_TRANS("TRANS_MCU","START_COLLECT_INFO_ESTIMATED_SIZE", CDummyEntry, CCollectorManager::OnStartCollectInfoEstimatedSize )
  ON_TRANS("TRANS_MCU","ABORT_COLLECT_INFO", CDummyEntry, CCollectorManager::OnAbortCollectInfo )
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CCollectorManager)

  ONCOMMAND("DisableUnit",CCollectorManager::HandleTerminalCollectInfo,"Collect Info")

END_TERMINAL_COMMANDS
////////////////////////////////////////////////////////////////////////////

extern void CollectorMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void CollectorManagerEntryPoint(void* appParam)
{
	CCollectorManager * pCollectorManager = new CCollectorManager;
	pCollectorManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CCollectorManager::GetMonitorEntryPoint()
{
	return CollectorMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCollectorManager::CCollectorManager()
{

}

//////////////////////////////////////////////////////////////////////
CCollectorManager::~CCollectorManager()
{

}

//////////////////////////////////////////////////////////////////////
void CCollectorManager::ManagerPostInitActionsPoint()
{
	m_pCollectorProcess = dynamic_cast<CCollectorProcess*>(CProcessBase::GetProcess());

	m_pCollectorUnitApi = new CTaskApi;
	COsQueue dummyMbx;
	CreateTask(m_pCollectorUnitApi, collectorUnitEntryPoint, &dummyMbx);

	m_pCollectorProcess->SetCollectingStatus(eCollectingStatus_ready);

    //*** simulate collect ***//
    //SimulateCollectInfo();
}
/////////////////////////////////////////////////////////////////////////
void CCollectorManager::SelfKill()
{
	m_pCollectorUnitApi->Destroy();
	POBJDELETE(m_pCollectorUnitApi);
	CManagerTask::SelfKill();
}

//////////////////////////////////////////////////////////////////////
//STATUS CCollectorManager::HandleTerminalPing(CSegment * seg,std::ostream& answer)
//{
//	PTRACE(eLevelError,"pong to logger");
//	answer << "pong to console";
//	return STATUS_OK;
//}
///////////////////////////////////////////////////////////////////////
STATUS CCollectorManager::OnCollectInfo(CRequest* pRequest)
{
   PTRACE(eLevelInfoNormal, "CCollectorManager::OnCollectInfo");
   STATUS status = STATUS_OK;

   if (pRequest->GetAuthorization() == SUPER )
   {
	   if (m_pCollectorProcess->GetCollectingStatus() != eCollectingStatus_ready)
	   {
		   status = STATUS_COLLECTING_ALREADY_IN_PROGRESS;
	   }
	   else
	{
	   //1. deserialize SetReq ( start and end time from EMA )
		   CInfoTimeInterval* pInfoTimeInterval = NULL;

		   pInfoTimeInterval = (CInfoTimeInterval*)pRequest->GetRequestObject() ;

		   m_pCollectorProcess->SetCollectingInfo(pInfoTimeInterval);

		   //2. Forward Req to CollectorUnit for all work to be done.
		   CSegment *pSeg = new CSegment;
		   pSeg->Put((BYTE*)pInfoTimeInterval, sizeof(CInfoTimeInterval));


		   m_pCollectorProcess->SetCollectingStatus(eCollectingStatus_collecting);

		   CSingleToneApi collectorUnitApi(eProcessCollector,"CollectorUnit");
		   collectorUnitApi.SendMsg(pSeg,COLLECT_INFO_REQ);

	               //3. Set flag for request proceed and pass to McuManager (notifying all EMAs about - in mcu state)

		   m_pCollectorProcess->SendCollectingToMcuMngr(TRUE);
         }
   }
   else
   {
       status = STATUS_NO_PERMISSION ;
       FPTRACE(eLevelInfoNormal,"CCollectorManager::OnCollectInfot: No permission to collect info");
   }
   //4. send confirm to EMA
   CDummyEntry* pDummyEntry = new CDummyEntry;
   pRequest->SetStatus(status);
   pRequest->SetConfirmObject(pDummyEntry);
   return status;

}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollectorManager::OnCollectInfoEndInd(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CCollectorManager::OnCollectInfoEndInd  ");
	// 1. Set flag for request proceed OFF and pass to McuManager (notifying all EMAs about - in mcu state)
	m_pCollectorProcess->SetCollectingStatus(eCollectingStatus_ready);
	m_pCollectorProcess->SendCollectingToMcuMngr(FALSE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
void CCollectorManager::OnCollectInfoFailedInd(CSegment* pSeg)
{
 // 1. Set flag for request proceed OFF and pass to McuManager (notifying all EMAs about - in mcu state)
	PTRACE(eLevelInfoNormal, "CCollectorManager::OnCollectInfoFailedInd  ");
	m_pCollectorProcess->SetCollectingStatus(eCollectingStatus_ready);
	m_pCollectorProcess->SendCollectingToMcuMngr(COLLECT_INFO_FAILED);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
//*** simulation
void CCollectorManager::SimulateCollectInfo(CSegment* pSeg)
{
   PTRACE(eLevelInfoNormal, "CCollectorManager::SimulateCollectInfo  ");
   STATUS status = STATUS_OK;

   CInfoTimeInterval infoTimeInterval;

   CStructTm start(0,0,0,0,0,0); //01/01/2006
   CStructTm   end(1,0,2007,0,0,0); //01/01/2007

   infoTimeInterval.SetStartTime(start);
   infoTimeInterval.SetEndTime(end);

   //2. Forward Req to CollectorUnit for all work to be done.
   CSegment *pMsgSeg = new CSegment;
   pMsgSeg->Put((BYTE*)(&infoTimeInterval), sizeof(CInfoTimeInterval));

   CSingleToneApi collectorUnitApi(eProcessCollector,"CollectorUnit");
   collectorUnitApi.SendMsg(pMsgSeg,COLLECT_INFO_REQ);


}
/////////////////////////////////////////////////////////////////////////////////////////////
STATUS CCollectorManager::HandleTerminalCollectInfo(CTerminalCommand & command, std::ostream& answer)
{
	PTRACE(eLevelInfoNormal, "  CCollectorManager::HandleTerminalDisableUnit ");

	//SimulateCollectInfo();

    return STATUS_OK;
}
///////////////////////////////////////////////////////////////////////////////////////
void CCollectorManager::OnStartCollectInfoEstimatedSize(CRequest* pRequest)
{
/*	 PTRACE(eLevelInfoNormal, "CCollectorManager::OnStartCollectInfoEstimatedSize");
	 STATUS status = STATUS_OK;

	 //1. deserialize SetReq ( start and end time from EMA )
	 CInfoTimeInterval infoTimeInterval;
	 infoTimeInterval = *(CInfoTimeInterval*)pRequest->GetRequestObject() ;

	 //2. Forward Req to CollectorUnit for all work to be done.
	 CSegment *pSeg = new CSegment;
	 pSeg->Put((BYTE*)(&infoTimeInterval), sizeof(CInfoTimeInterval));

	 CSingleToneApi collectorUnitApi(eProcessCollector,"CollectorUnit");
	 collectorUnitApi.SendMsg(pSeg,COLLECT_INFO_ESTIMATE_SIZE_REQ);

	 //3. Set flag for request proceed and pass to McuManager (notifying all EMAs about - in mcu state)
	 m_collecting = TRUE;
	 m_pCollectorProcess->SendCollectingToMcuMngr(TRUE);

	 //4. send confirm to EMA
	 CDummyEntry* pDummyEntry = new CDummyEntry;
	 pRequest->SetStatus(STATUS_OK);
	 pRequest->SetConfirmObject(pDummyEntry);*/
}

///////////////////////////////////////////////////////////////////////////////////////
void CCollectorManager::OnAbortCollectInfo(CRequest* pRequest)
{
	TRACESTR(eLevelInfoNormal) << "CCollectorManager::OnAbortCollectInfo";
	STATUS status = STATUS_OK;

	if (pRequest->GetAuthorization() == SUPER )
	{

	        if (m_pCollectorProcess->GetCollectingStatus() == eCollectingStatus_ready)
	           status = STATUS_COLLECTOR_IS_NOT_ACTIVE_AT_THE_MOMENT;
	        else if (m_pCollectorProcess->GetCollectingStatus() == eCollectingStatus_aborting)
	           status = STATUS_ABORT_COLLECTOR_ALREADY_STARTED;
		else
	        {
		   m_pCollectorProcess->SetCollectingStatus(eCollectingStatus_aborting);


		   if (m_pCollectorProcess->GetCollectFilesStarted() == FALSE)
	               m_pCollectorProcess->HandleAbort();
        	}
	}
	else
	{
	       status = STATUS_NO_PERMISSION ;
	       FPTRACE(eLevelInfoNormal,"CCollectorManager::OnAbortCollectInfo: No permission to abort collect info");
	}

	CDummyEntry* pDummyEntry = new CDummyEntry;
	pRequest->SetStatus(status);
	pRequest->SetConfirmObject(pDummyEntry);
}
