#include <iomanip>
using namespace std;

#include "ProcessPolicy.h"
#include "SystemFunctions.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "TraceStream.h"

extern const char* ProcessTypeToString(eProcessType processType);


//const DWORD MAX_TIME_BETWEEN_REBOOT = 2000;
const DWORD MAX_TIME_BETWEEN_REBOOT = 90;
const DWORD MAX_NUM_RETRY 			= 3;



static DWORD GetCurrentTime()
{
	return SystemGetTickCount().GetSeconds();
}





CProcessPolicy::CProcessPolicy()
:m_MaxNumRetry(MAX_NUM_RETRY)
{
	SetDefaults();
}

CProcessPolicy::~CProcessPolicy()
{
}

void CProcessPolicy::SetStatus(eMonitorProcessStatus status)
{
    m_Status = status;
    if(eMonitorProcessStatusAlive == m_Status)
	{
		m_KeepAliveCnt++;
    }
}


// void CProcessPolicy::SetIsAlive(bool val)
// {
// 	m_IsAlive = val;
// 	if(true == val)
// 	{
// 		m_KeepAliveCnt++;
// 		m_IsTerminated	= false;
// 	}
// }

void CProcessPolicy::SetDefaults(eProcessType processType)
{
    m_Status = eMonitorProcessStatusTerminated;
	m_PolicyType	= eNoRetryLoading;
	m_TimeUp 		= 0;
	m_TimeLastUp 	= 0;
	m_NumRetry 		= 0;
    m_AbsCrashCounter = 0;
	m_ProcessType	= processType;
	m_KeepAliveCnt	= 0;
    m_NumOfLaunch   = 0;
    m_NumWaitWDRetry= 0;
}

void CProcessPolicy::SetTimeFirstUp()
{
	DWORD now = GetCurrentTime();
	m_TimeUp = m_TimeLastUp = now;
}

void CProcessPolicy::SetTimeUp()
{
	DWORD now = GetCurrentTime();
	m_TimeUp = now;
}

void CProcessPolicy::UpdateNumOfRetry()
{
    m_AbsCrashCounter++;
    
	DWORD now = GetCurrentTime();
    
    DWORD maxTimeBetweenCrush = 30 * 60;
    CSysConfig *sysCfg = CProcessBase::GetProcess()->GetSysConfig();
    sysCfg->GetDWORDDataByKey("MAX_TIME_BETWEEN_PROCESS_CRUSH", maxTimeBetweenCrush);
    
    if(now - m_TimeLastUp > maxTimeBetweenCrush)
    {
        m_NumRetry = 1;
        
        TRACEINTO << "CProcessPolicy::UpdateNumOfRetry : become equal to 1\n"
                  << "now - m_TimeLastUp > maxTimeBetweenCrush\n"
                  << "now - m_TimeLastUp " << now - m_TimeLastUp << "; maxTime " << maxTimeBetweenCrush;
    }
    else
    {
        m_NumRetry++;
        
        TRACEINTO << "CProcessPolicy::UpdateNumOfRetry : increasing to " << m_NumRetry
                  << "\nelse\n"
                  << "now - m_TimeLastUp" << now - m_TimeLastUp << "; maxTime " << maxTimeBetweenCrush;
    }
    m_TimeLastUp = now;
}

eFaultedProcessAction CProcessPolicy::GetFaultedPolicy(eProcessType process)
{
	switch (process)
	{
	// policy 1
	case eProcessCards:
	case eProcessResource:
	case eProcessConfParty:
	case eProcessMcuMngr:
	case eProcessCSMngr:
	case eProcessCsModule:
		return eReset;

	case eProcessGideonSim: // for Call Generator
		return (CProcessBase::GetProcess()->GetProductType() == eProductTypeCallGenerator) ? eReset : eNoRetryLoading;

	// policy 2
	case eProcessMplApi:
	case eProcessCSApi:
	case eProcessRtmIsdnMngr:
	case eProcessSipProxy:
	case eProcessGatekeeper:
	case eProcessAuthentication:
	case eProcessDNSAgent:
	case eProcessIPMCInterface:
	case eProcessSystemMonitoring:
	case eProcessLicenseServer:
	case eProcessIce:
		return eResetAfterRetryLoading;

	case eProcessMediaMngr: // for Call Generator
		return (CProcessBase::GetProcess()->GetProductType() == eProductTypeCallGenerator) ? eResetAfterRetryLoading : eNoRetryLoading;

	// policy 3
	case eProcessFaults:
	case eProcessLogger:
	case eProcessCDR:
	case eProcessEncryptionKeyServer:
	case eProcessConfigurator:
	case eProcessCollector:
	case eProcessApacheModule:
	case eProcessInstaller:
	case eProcessSNMPProcess:
	case eProcessAuditor:
	case eProcessCertMngr:
	case eProcessFailover:
	case eProcessLdapModule:
	case eProcessBackupRestore:
	case eProcessUtility:
	case eProcessNotificationMngr:
	case eProcessMCCFMngr:
	case eProcessExchangeModule:
	case eProcessQAAPI:
		return eNoResetAfterRetryLoading;

	// policy 4
	case eProcessDemo:
	case eProcessClientLogger:
	case eProcessTestClient:
	case eProcessEndpointsSim:
	case eProcessMcuCmd:
	case eProcessMcmsDaemon:
	case eProcessDiagnostics:
	case eProcessMcmsNetwork:
		return eNoRetryLoading;

	default:
		FPASSERTMSG((int)process + 100, "GetFaultedAction invalid process + 100");
		return eNoRetryLoading;
	}
}

void CProcessPolicy::Dump(std::ostream &ostr) const
{
	const char *processName = ProcessTypeToString(m_ProcessType);
	ostr << processName << " Dump" 	<< std::endl
			<< "-------------------"<< std::endl
			<< std::setw(15) << "Policy " << GetFaultProcessesName(m_PolicyType) << std::endl
            << std::setw(15) << "Keep Alive " << m_KeepAliveCnt << std::endl
			<< std::setw(15) << "TimeUp " << std::setw(5) << m_TimeUp << std::endl
			<< std::setw(15) << "TimeLastUp " << std::setw(5) << m_TimeLastUp << std::endl
			<< std::setw(15) << "NumRetry " << std::setw(5) << m_NumRetry << std::endl
			<< std::setw(15) << "MaxNumRetry " << std::setw(5) << m_MaxNumRetry << std::endl
            << std::setw(15) << "Num of Launch " << std::setw(5) << m_NumOfLaunch << std::endl
            << std::setw(15) << "Num of Waiting WD Retry " << std::setw(5) << m_NumWaitWDRetry << std::endl
            << std::setw(15) << "Status " << std::setw(5) << GetProcessStatusName(m_Status) << std::endl;
}


DWORD CProcessPolicy::GetMaxWaitWDNumber(eProcessType process)
{
	if( eProcessConfParty == process )
		return 2;

	return 1;
}

bool CProcessPolicy::IsAdditionalWDRetry(eProcessType process)
{
	m_NumWaitWDRetry++;

	DWORD maxNumb = GetMaxWaitWDNumber(process);
	if( m_NumWaitWDRetry < maxNumb )
		return TRUE;

	if( m_NumWaitWDRetry == maxNumb )
		m_NumWaitWDRetry = 0;

	return FALSE;
}


