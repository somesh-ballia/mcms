// SNMPProcessMonitor.cpp: implementation of the SNMPProcessMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "SNMPProcessMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "SnmpData.h"
#include "DummyEntry.h"
#include "SNMPProcessProcess.h"
#include "Request.h"
#include "ApiStatuses.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSNMPProcessMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CSNMPProcessMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CSNMPProcessMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CSNMPProcessMonitor)
  ON_TRANS("TRANS_SNMP","GET",CDummyEntry,CSNMPProcessMonitor::HandleGetSnmpData)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void SNMPProcessMonitorEntryPoint(void* appParam)
{  
	CSNMPProcessMonitor *monitorTask = new CSNMPProcessMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSNMPProcessMonitor::CSNMPProcessMonitor()
{

}

CSNMPProcessMonitor::~CSNMPProcessMonitor()
{

}

//////////////////////////////////////////////////////////////////////
STATUS CSNMPProcessMonitor::HandleGetSnmpData(CRequest* pGetRequest)
{
    if(pGetRequest->GetAuthorization() != SUPER && pGetRequest->GetAuthorization() != ADMINISTRATOR_READONLY)
    {
	pGetRequest->SetStatus(STATUS_NO_PERMISSION);
        pGetRequest->SetConfirmObject(new CDummyEntry());
        return STATUS_OK;
    }
      
	CSNMPProcessProcess * process = (CSNMPProcessProcess *) CProcessBase::GetProcess();
	CSnmpData * pData = new CSnmpData(process->GetSnmpData());
	pData->SetIsForEMA(true);
	pGetRequest->SetConfirmObject(pData);
	return STATUS_OK;
}

