// FailoverMonitor.cpp: implementation of the FailoverMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "FailoverMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "FailoverConfiguration.h"
#include "FailoverConfigurationDBGet.h"


#include "Request.h"
////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CFailoverMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CFailoverMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CFailoverMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CFailoverMonitor)
	ON_TRANS("TRANS_HOTBACKUP", "GET", CFailoverConfigurationDBGet, CFailoverMonitor::OnServerGetFailoverConfiguration)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void FailoverMonitorEntryPoint(void* appParam)
{
	CFailoverMonitor *monitorTask = new CFailoverMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFailoverMonitor::CFailoverMonitor()
{
	PTRACE(eLevelInfoNormal,"CFailoverMonitor - CFailoverMonitor");
}

CFailoverMonitor::~CFailoverMonitor()
{

}

STATUS CFailoverMonitor::OnServerGetFailoverConfiguration(CRequest* pGetRequest)
{
	STATUS status = STATUS_OK;
	//PTRACE(eLevelInfoNormal,"CFailoverMonitor::OnServerGetFailoverConfiguration");

	//CFailoverProcess * process = (CFailoverProcess *) CProcessBase::GetProcess();
	//CFailoverConfiguration * pFailoverConfiguration=(process->GetFailoverConfiguration());
	CFailoverConfigurationDBGet* pFailoverConfigurationDBGet = new CFailoverConfigurationDBGet();
	*pFailoverConfigurationDBGet = *(CFailoverConfigurationDBGet*)pGetRequest->GetRequestObject() ;
	pGetRequest->SetConfirmObject(pFailoverConfigurationDBGet);
	//PDELETE(CFailoverConfigurationDBGet);
	//POBJDELETE(pFailoverConfigurationDBGet);


	return status;
}

