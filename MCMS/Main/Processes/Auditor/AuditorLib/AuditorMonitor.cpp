// AuditorMonitor.cpp: implementation of the AuditorMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "AuditorMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "AuditFileListGet.h"
#include "Request.h"
#include "DummyEntry.h"
#include "ApiStatuses.h"
#include "AuditEventListGet.h"
#include "AuditorProcess.h"
#include "ManagerApi.h"
#include "OpcodesMcmsInternal.h"




////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CAuditorMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CAuditorMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CAuditorMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CAuditorMonitor)
  ON_TRANS("TRANS_AUDIT_FILE_SUMMARY_LIST", "GET", CAuditFileListGet, CAuditorMonitor::HandleGetAuditFileList )
  ON_TRANS("TRANS_AUDIT_EVENT_LIST",        "GET", CAuditEventListGet, CAuditorMonitor::HandleGetAuditEventList )     
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CAuditorMonitor::HandleOperLogin)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void AuditorMonitorEntryPoint(void* appParam)
{  
	CAuditorMonitor *monitorTask = new CAuditorMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAuditorMonitor::CAuditorMonitor()
{

}

CAuditorMonitor::~CAuditorMonitor()
{

}

//////////////////////////////////////////////////////////////////////
STATUS CAuditorMonitor::HandleGetAuditFileList(CRequest* pGetRequest)
{
    if(pGetRequest->GetAuthorization() != SUPER &&
       pGetRequest->GetAuthorization() != AUDITOR &&
       pGetRequest->GetAuthorization() != ADMINISTRATOR_READONLY)
    {
        pGetRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelInfoNormal,"CAuditorMonitor::HandleGetAuditFileList: No permission");
		pGetRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }
    
    CManagerApi api(eProcessAuditor);
    OPCODE respOpcode = FLUSH_BUFFER_IND_FAIL;
    STATUS status = api.SendSyncOpcodeMsg(FLUSH_BUFFER_REQ, 100, respOpcode);
    if(STATUS_OK != status || FLUSH_BUFFER_IND_OK != respOpcode)
    {
        PASSERTMSG(TRUE, "Failed to send FLUSH_BUFFER request");
    }
    
    CAuditFileListGet *pFileListGet = new CAuditFileListGet;
    *pFileListGet = *(CAuditFileListGet*)pGetRequest->GetRequestObject();
    pGetRequest->SetConfirmObject(pFileListGet);

    CAuditorProcess *pProcess = (CAuditorProcess*)CProcessBase::GetProcess();

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CAuditorMonitor::HandleGetAuditEventList(CRequest* pGetRequest)
{
    if(pGetRequest->GetAuthorization() != SUPER &&
       pGetRequest->GetAuthorization() != AUDITOR &&
       pGetRequest->GetAuthorization() != ADMINISTRATOR_READONLY)
    {
        pGetRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelInfoNormal,"CAuditorMonitor::HandleGetAuditEventList: No permission");
		pGetRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }

    // is changed
    
    CAuditEventListGet *auditEventList = new CAuditEventListGet;
    *auditEventList = *(CAuditEventListGet*)pGetRequest->GetRequestObject();
    
    pGetRequest->SetConfirmObject(auditEventList);

    return STATUS_OK;
}
