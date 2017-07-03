// LdapModuleMonitor.cpp: implementation of the LdapModuleMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "LdapModuleMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "LdapModuleCfgGet.h"
#include "Request.h"
#include "OpcodesMcmsInternal.h"
#include "DummyEntry.h"
#include "ApiStatuses.h"


////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CLdapModuleMonitor)
	ONEVENT(XML_REQUEST    ,IDLE    , CLdapModuleMonitor::HandlePostRequest )
	ONEVENT(LDAP_MONITOR_UPDATE_AVAILIBILITY_STATUS    ,IDLE    , CLdapModuleMonitor::OnUpdateAdServerAvailabilityStatus )
PEND_MESSAGE_MAP(CLdapModuleMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CLdapModuleMonitor)
	ON_TRANS("TRANS_MCU", "GET_ACTIVE_DIRECTORY_CONFIGURATION", CLdapModuleCfgGet, CLdapModuleMonitor::OnGetActiveDirCfg)
	ON_TRANS("TRANS_MCU", "GET_AD_SERVER_AVAILABILITY_STATUS", 	CLdapModuleCfgGet, CLdapModuleMonitor::OnGetAdServerAvailabilityStatus)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void LdapModuleMonitorEntryPoint(void* appParam)
{  
	CLdapModuleMonitor *monitorTask = new CLdapModuleMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CLdapModuleMonitor::CLdapModuleMonitor()
{
	m_adServerAvailabilityStatus = STATUS_IN_PROGRESS;
}

/////////////////////////////////////////////////////////////////////////////
CLdapModuleMonitor::~CLdapModuleMonitor()
{

}


/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
STATUS CLdapModuleMonitor::OnGetActiveDirCfg(CRequest* pGetRequest)
{
	CLdapModuleCfgGet* pLdapCfgGet = new CLdapModuleCfgGet();
	*pLdapCfgGet  = *(CLdapModuleCfgGet*)pGetRequest->GetRequestObject();
	pGetRequest->SetConfirmObject(pLdapCfgGet);

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CLdapModuleMonitor::OnUpdateAdServerAvailabilityStatus(CSegment* pParam)
{
	PTRACE(eLevelTrace,"CLdapModuleMonitor::OnUpdateAvailabilityStatus");
	*pParam >> m_adServerAvailabilityStatus;
 	ResponedClientRequest(0,NULL);
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CLdapModuleMonitor::OnGetAdServerAvailabilityStatus(CRequest* pGetRequest)
{
	PTRACE(eLevelTrace,"CLdapModuleMonitor::OnGetAdServerAvailabilityStatus");
	pGetRequest->SetConfirmObject(new CDummyEntry());
	pGetRequest->SetStatus(m_adServerAvailabilityStatus);
	m_adServerAvailabilityStatus = STATUS_IN_PROGRESS; //after retrieving the status set it back to IN_PROGRESS
	return STATUS_OK;
}


