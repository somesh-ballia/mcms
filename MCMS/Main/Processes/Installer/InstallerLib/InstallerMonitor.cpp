// InstallerMonitor.cpp: implementation of the InstallerMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "InstallerMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Macros.h"
#include "InstallerProcess.h"
#include "DummyEntry.h"
#include "Request.h"
#include "ApiStatuses.h"
#include "TraceStream.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CInstallerMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CInstallerMonitor::HandlePostRequest )
  ONEVENT( INSTALLER_MONITOR_UPDATE_KEY_STATUS_PARAMS, IDLE , CInstallerMonitor::OnSetLastUpdateKeyStatus )
PEND_MESSAGE_MAP(CInstallerMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CInstallerMonitor)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CInstallerMonitor::HandleOperLogin)
  ON_TRANS("TRANS_MCU","GET_INSTALLATION_STATUS", CDummyEntry, CInstallerMonitor::HandleGetInstallationStatus)  
  ON_TRANS("TRANS_MCU", "GET_LAST_UPDATE_KEY_CODE_INDICATION", CDummyEntry, CInstallerMonitor::OnGetUpdateKeyStatus)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void InstallerMonitorEntryPoint(void* appParam)
{  
	CInstallerMonitor *monitorTask = new CInstallerMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CInstallerMonitor::CInstallerMonitor()
{
    m_pProcess  = dynamic_cast<const CInstallerProcess*>(CInstallerProcess::GetProcess());

    m_nLastSetUpdateKeyStatus = STATUS_IN_PROGRESS;
    m_LastSetUpdateKeyStatusDesc = "";
}

//////////////////////////////////////////////////////////////////////
CInstallerMonitor::~CInstallerMonitor()
{

}

//////////////////////////////////////////////////////////////////////
STATUS CInstallerMonitor::HandleGetInstallationStatus(CRequest *pRequest)
{
    STATUS installationStatus = m_pProcess->GetInstallationStatus();
    
    pRequest->SetStatus(installationStatus);
    pRequest->SetConfirmObject(new CDummyEntry);

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CInstallerMonitor::OnGetUpdateKeyStatus(CRequest* pGetRequest)
{
	PTRACE(eLevelInfoNormal,"CInstallerMonitor::OnGetUpdateKeyStatus");
	pGetRequest->SetConfirmObject(new CDummyEntry());
	pGetRequest->SetStatus(m_nLastSetUpdateKeyStatus);
	pGetRequest->SetExDescription(m_LastSetUpdateKeyStatusDesc.c_str());
	//m_nLastSetUpdateKeyStatus = STATUS_IN_PROGRESS; //after retriving the last status gets it back to IN_PROGRESS
	//m_LastSetUpdateKeyStatusDesc = "";
	return STATUS_OK;
}
////////////////////////////////////////////////////////////////////////////
STATUS CInstallerMonitor::OnSetLastUpdateKeyStatus(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CInstallerMonitor::OnSetLastUpdateKeyStatus");
	*pParam >> (DWORD&)m_nLastSetUpdateKeyStatus;
    *pParam >> m_LastSetUpdateKeyStatusDesc;
    TRACESTR(eLevelInfoNormal) << "\nCInstallerMonitor::OnSetLastUpdateKeyStatus"
        					   << "\nm_nLastSetUpdateKeyStatus:               " << m_nLastSetUpdateKeyStatus
        					   << "\nm_LastSetUpdateKeyStatusDesc: " << m_LastSetUpdateKeyStatusDesc;

	ResponedClientRequest(0,NULL);
	return STATUS_OK;
}

