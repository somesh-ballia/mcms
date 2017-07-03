// UtilityMonitor.cpp: implementation of the UtilityMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "UtilityMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "TcpDumpEntityListGet.h"
#include "TcpDumpStatusGet.h"
#include "Macros.h"
#include "Request.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CUtilityMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CUtilityMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CUtilityMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CUtilityMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CUtilityMonitor::HandleOperLogin)
ON_TRANS("TRANS_TCP_DUMP"		, "GET_TCP_DUMP_CFG"	, CTcpDumpEntityListGet	, CUtilityMonitor::OnGetTcpDumpCfg)
ON_TRANS("TRANS_TCP_DUMP"		, "GET_TCP_DUMP_STATUS"	, CTcpDumpStatusGet	, CUtilityMonitor::OnGetTcpDumpStatus)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void UtilityMonitorEntryPoint(void* appParam)
{  
	CUtilityMonitor *monitorTask = new CUtilityMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUtilityMonitor::CUtilityMonitor()
{
	 m_proc = static_cast<CUtilityProcess*>(CProcessBase::GetProcess());
	   PASSERTMSG(NULL == m_proc, "Unable to continue");
}

CUtilityMonitor::~CUtilityMonitor()
{

}


/////////////////////////////////////////////////////////////////////////////
STATUS CUtilityMonitor::OnGetTcpDumpCfg(CRequest* pGetRequest)
{
	CTcpDumpEntityListGet* pTcpDumpEntityListGet = new CTcpDumpEntityListGet();
	*pTcpDumpEntityListGet  = *(CTcpDumpEntityListGet*)pGetRequest->GetRequestObject();
	pGetRequest->SetConfirmObject(pTcpDumpEntityListGet);
	TRACEINTO << "CUtilityMonitor::OnGetTcpDumpCfg";

	m_proc->SetIsUiUpdateNeeded(true);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CUtilityMonitor::OnGetTcpDumpStatus(CRequest* pGetRequest)
{
	CTcpDumpStatusGet* pTcpDumpStatusGet = new CTcpDumpStatusGet();
	*pTcpDumpStatusGet  = *(CTcpDumpStatusGet*)pGetRequest->GetRequestObject();
	pGetRequest->SetConfirmObject(pTcpDumpStatusGet);
	TRACEINTO << "CUtilityMonitor::OnGetTcpDumpStatus";


	return STATUS_OK;
}

