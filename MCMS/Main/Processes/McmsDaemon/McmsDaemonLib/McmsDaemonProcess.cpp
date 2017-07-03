#include "McmsDaemonProcess.h"

#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ConfigHelper.h"

#include "OsFileIF.h"
#include "SysConfigEma.h"
#include "TerminalCommand.h"
#include "InternalProcessStatuses.h"
#include "NStream.h"
#include "UnicodeDefines.h"
#include "EncodingConvertor.h"
#include "psosxml.h"
#include "XmlApi.h"
#include "SystemFunctions.h"

#include "TraceStream.h"

#include <unistd.h>

//////////////////////////////////////////////////////////////////////
extern void McmsDaemonManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CMcmsDaemonProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CMcmsDaemonProcess::GetManagerEntryPoint()
{
	return McmsDaemonManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
CMcmsDaemonProcess::CMcmsDaemonProcess()
	: m_isSafeMode(false)
	, m_systemCfgActiveAlarm(false)
{
	CreateProcessMap();
}

//////////////////////////////////////////////////////////////////////
CMcmsDaemonProcess::~CMcmsDaemonProcess()
{
}

//////////////////////////////////////////////////////////////////////

// static
CMcmsDaemonProcess::ProcessBitMap CMcmsDaemonProcess::m_processWatchMap;

void CMcmsDaemonProcess::CreateProcessMap()
{
	static bool created = false;

	if (created)
		return;

	m_processWatchMap.flip(); // turn ON all the bits

	m_processWatchMap[eProcessMcmsDaemon] = false;
	m_processWatchMap[eProcessDiagnostics] = false;
	m_processWatchMap[eProcessCsModule] = false;
	m_processWatchMap[eProcessEndpointsSim] = false;
	m_processWatchMap[eProcessDemo] = false;
	m_processWatchMap[eProcessTestClient] = false;
	m_processWatchMap[eProcessMcuCmd] = false;
	m_processWatchMap[eProcessClientLogger] = false;
	m_processWatchMap[eProcessMcmsNetwork] = false;

	const eProductType prodType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
	const bool isFlexeraLicense = CProcessBase::GetProcess()->IsFlexeraLicenseInSysFlag();

	if (prodType == eProductTypeCallGenerator)
	{
		m_processWatchMap[eProcessCDR] = false;
		m_processWatchMap[eProcessIPMCInterface] = false;
	}
	else
	{
		m_processWatchMap[eProcessMediaMngr] = false;
		m_processWatchMap[eProcessGideonSim] = false;
	}

	// for all SoftMCU products
	if (eProductFamilySoftMcu == prodFamily)
	{
		m_processWatchMap[eProcessDiagnostics] = false;
		m_processWatchMap[eProcessIPMCInterface] = false;
		//m_processWatchMap[eProcessLogger] = false; // soft mcu should use MRM logger in the future

		if (prodType != eProductTypeGesher && prodType != eProductTypeNinja && prodType != eProductTypeEdgeAxis)
			m_processWatchMap[eProcessInstaller] = false; // soft mcu installation is based on MRM installer

		if ((prodType != eProductTypeEdgeAxis) ||  (isFlexeraLicense == false))
			m_processWatchMap[eProcessLicenseServer] = false; 

		//m_processWatchMap[eProcessRtmIsdnMngr] = false; // no Rtm ISDN in soft mcu
		m_processWatchMap[eProcessMediaMngr] = false;
		m_processWatchMap[eProcessEndpointsSim] = false;
		m_processWatchMap[eProcessDemo] = false;
		m_processWatchMap[eProcessTestClient] = false;


	}

	created = true;
}


void CMcmsDaemonProcess::printMap(std::ostream& answer)
{
	CreateProcessMap();

	for (eProcessType pType = eProcessMcmsDaemon; pType < NUM_OF_PROCESS_TYPES; ++pType)
	{
		const bool state = m_processWatchMap[pType];
		const char* name = ProcessTypeToStr(pType);
		answer <<  name << " = " << state << "\n";
	}
}

eProcessType CMcmsDaemonProcess::ConvertStrToProcessType(const std::string& processName)
{
	for (int i = 0; i < NUM_OF_PROCESS_TYPES; ++i)
	{
		if (processName.compare(ProcessNames[i]) == 0)
			return static_cast<eProcessType>(i);
	}

	return eProcessTypeInvalid;
}

void CMcmsDaemonProcess::UpdateProcessWatchState(eProcessType processType, bool state)
{
	m_processWatchMap[processType] = state;
}

//////////////////////////////////////////////////////////////////////
int CMcmsDaemonProcess::SetUp()
{
	STATUS status1 = VerifyXmlFile(GetEmaCfgPath(eCfgParamUser));
	if (STATUS_OK != status1 && STATUS_FILE_NOT_EXIST != status1)
		DeleteFile(GetEmaCfgPath(eCfgParamUser));

	CSysConfig::SwitchCfgFiles(GetCfgPath(eCfgParamUser), GetEmaCfgPath(eCfgParamUser));

	STATUS status2 = VerifyXmlFile(GetEmaCfgPath(eCfgParamDebug));
	if (STATUS_OK != status2 && STATUS_FILE_NOT_EXIST != status2)
		DeleteFile(GetEmaCfgPath(eCfgParamDebug));

	CSysConfig::SwitchCfgFiles(GetCfgPath(eCfgParamDebug), GetEmaCfgPath(eCfgParamDebug));

	if ((STATUS_OK != status1 && STATUS_FILE_NOT_EXIST != status1) ||
		(STATUS_OK != status2 && STATUS_FILE_NOT_EXIST != status2))
	{
		m_systemCfgActiveAlarm = TRUE;
	}

	return CProcessBase::SetUp();
}

//////////////////////////////////////////////////////////////////////
STATUS CMcmsDaemonProcess::VerifyXmlFile(const char* fileName)
{
	if (!fileName)
		return STATUS_FAIL;

	STATUS status = STATUS_OK;
	int nStatus = STATUS_OK; // for the xml parsing

	FILE* infile = fopen(fileName, "r");
	if (!infile)
	{
		switch (errno)
		{
		case ENOENT:
			status = STATUS_FILE_NOT_EXIST;
			break;

		case EACCES:
			status = STATUS_OPEN_FILE_FAILED;
			break;

		default:
			status = STATUS_UNKNOWN_FILE_FAILURE;
			break;
		}

		return status;
	}

	fclose(infile);

	const int fileSize = GetFileSize(fileName);
	if (-1 == fileSize)
		return STATUS_FAIL;

	if ((DWORD)fileSize > CSerializeObject::GetMaxXMLFileSize())
		return STATUS_FAIL;

	COstrStream validateErrorString;
	STATUS statusUtf8Validate = CEncodingConvertor::ValidateFile(MCMS_INTERNAL_STRING_ENCODE_TYPE, fileName, validateErrorString);

	if (STATUS_OK != statusUtf8Validate)
		return STATUS_PARSING_XML_FILE_FAILED;

	CXMLDOMDocument* pCfgRoot = new CXMLDOMDocument;
	if (!ParseXMLFile(fileName, pCfgRoot))
		status = STATUS_PARSING_XML_FILE_FAILED;

	PDELETE(pCfgRoot);
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CMcmsDaemonProcess::DumpProcessStatistics(std::ostream& answer, CTerminalCommand & command)const
{
	answer << "Work Mode : " << (m_isSafeMode ? "Safe mode\n" : "Normal\n");

	// extract wanted process names
	vector<string> processNames;
	DWORD numOfProcesses = command.GetNumOfParams();
	if (numOfProcesses)
	{
		for (int i = (int)eCmdParam1; i < (int)(eCmdParam1 + numOfProcesses); ++i)
		{
			const string& currentProcess = command.GetToken((eCommandParamsIndex)i);
			processNames.push_back(currentProcess);
		}
	}

	if (processNames.size() > 0) //print only wanted processes
	{
		for (vector<string>::iterator it = processNames.begin(); it != processNames.end(); ++it)
		{
			const string &currentProcess = *it;
			eProcessType processType = CProcessBase::GetProcessValueByString(currentProcess.c_str());

			if (eProcessTypeInvalid == processType)
			{
				answer << currentProcess.c_str() << " : invalid process\n";
				continue;
			}

			if (!CMcmsDaemonProcess::IsLaunched(processType))
			{
				answer << currentProcess.c_str() << " : Is not launched\n";
				continue;
			}

			const CProcessPolicy &processPolicy = m_ProcessMonitoringInfo[processType];
			answer << std::endl;
			processPolicy.Dump(answer);
			answer << std::endl;
		}
	}
	else // print all processes
	{
		for (int i = (int)eProcessTypeInvalid + 1; i < (int)NUM_OF_PROCESS_TYPES; ++i)
		{
			eProcessType processType = static_cast<eProcessType>(i);
			if (!CMcmsDaemonProcess::IsLaunched(processType))
				continue;

			const CProcessPolicy &processPolicy = m_ProcessMonitoringInfo[i];
			answer << std::endl;
			processPolicy.Dump(answer);
			answer << std::endl;
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CMcmsDaemonProcess::ResetProcessMonitoringInfo()
{
	for (int i = (int)eProcessTypeInvalid + 1; i < (int)NUM_OF_PROCESS_TYPES; ++i)
	{
		eFaultedProcessAction policy = CProcessPolicy::GetFaultedPolicy(static_cast<eProcessType>(i));

		m_ProcessMonitoringInfo[i].SetDefaults(static_cast<eProcessType>(i));
		m_ProcessMonitoringInfo[i].SetPolicy(policy);
	}
}

//////////////////////////////////////////////////////////////////////
void CMcmsDaemonProcess::SetAllProcessesStatus(eMonitorProcessStatus from, eMonitorProcessStatus to)
{
	for (int i = (int)eProcessTypeInvalid + 1; i < (int)NUM_OF_PROCESS_TYPES; ++i)
	{
		if (from == m_ProcessMonitoringInfo[i].GetStatus())
			m_ProcessMonitoringInfo[i].SetStatus(to);
	}
}

//////////////////////////////////////////////////////////////////////
bool CMcmsDaemonProcess::IsWatched(eProcessType process)
{
	// skip processes  Exchange and Failover in JITC MODE
	switch (process)
	{
	case eProcessExchangeModule:
	case eProcessFailover:
		if (GetSystemCfgFlagInt<bool>(CFG_KEY_ULTRA_SECURE_MODE))
			return false;

		break;
	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
	}

	CreateProcessMap();

	return m_processWatchMap[process];
}

//////////////////////////////////////////////////////////////////////
bool CMcmsDaemonProcess::IsLaunched(eProcessType process)
{
	// skip processes Exchange and Failover in JITC MODE
    /*Begin:modified by Richer for BRIDGE-12409, 3/31/2014*/
    const eProductType prodType = CProcessBase::GetProcess()->GetProductType();
    const eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
    const bool isFlexeraLicense = CProcessBase::GetProcess()->IsFlexeraLicenseInSysFlag();
    
	switch (process)
	{
	case eProcessExchangeModule:
        return !GetSystemCfgFlagInt<bool>(CFG_KEY_ULTRA_SECURE_MODE);
        
	case eProcessFailover:
    {
        if (eProductTypeSoftMCUMfw == prodType)
            return false;
        
        return !GetSystemCfgFlagInt<bool>(CFG_KEY_ULTRA_SECURE_MODE);
    }

	case eProcessMCCFMngr:
		return true;

	default:
		// Note: some enumeration value are not handled in switch. Add default to suppress warning.
		break;
	}

	//const eProductType prodType = CProcessBase::GetProcess()->GetProductType();
	//const eProductFamily prodFamily = CProcessBase::GetProcess()->GetProductFamily();
    /*End:modified by Richer for BRIDGE-12409, 3/31/2014*/
    
	if (eProductFamilySoftMcu == prodFamily)
	{
		switch (process)
		{
		case eProcessMplApi:
		case eProcessConfigurator:
		case eProcessFaults:
		case eProcessLogger:
		case eProcessConfParty:
		case eProcessMcuMngr:
		case eProcessCards:
		case eProcessResource:
		case eProcessAuthentication:
		case eProcessCSApi:
		case eProcessCSMngr:
		case eProcessSipProxy:
		case eProcessIce:
		case eProcessDNSAgent:
		case eProcessCDR:
		case eProcessQAAPI:
		case eProcessEncryptionKeyServer:
		case eProcessGatekeeper:
		case eProcessSNMPProcess:
		case eProcessApacheModule:
		case eProcessBackupRestore:
		case eProcessCollector:
		case eProcessSystemMonitoring:
		case eProcessAuditor:
		case eProcessCertMngr:
		case eProcessUtility:
		case eProcessRtmIsdnMngr:
		case eProcessNotificationMngr:
		case eProcessMcmsNetwork:
			return true;

		case eProcessInstaller:
			return (prodType == eProductTypeGesher || prodType == eProductTypeNinja || prodType == eProductTypeEdgeAxis);

		case eProcessLdapModule:
			return (prodType != eProductTypeSoftMCUMfw);

		case eProcessLicenseServer:
			return ((prodType == eProductTypeEdgeAxis ) && (isFlexeraLicense == true));
		default:
			return false;
		}
	}

	switch (process)
	{
	case eProcessMplApi:
	case eProcessConfigurator:
	case eProcessFaults:
	case eProcessLogger:
	case eProcessConfParty:
	case eProcessMcuMngr:
	case eProcessCards:
	case eProcessResource:
	case eProcessAuthentication:
	case eProcessCSApi:
	case eProcessCSMngr:
	case eProcessRtmIsdnMngr:
	case eProcessSipProxy:
	case eProcessDNSAgent:
	case eProcessQAAPI:
	case eProcessEncryptionKeyServer:
	case eProcessGatekeeper:
	case eProcessSNMPProcess:
	case eProcessApacheModule:
	case eProcessInstaller:
	case eProcessBackupRestore:
	case eProcessCollector:
	case eProcessSystemMonitoring:
	case eProcessAuditor:
	case eProcessCertMngr:
	case eProcessUtility:
	case eProcessLdapModule:
	case eProcessMcmsNetwork:
		return true;

	case eProcessIPMCInterface:
	case eProcessCDR:
		return (CProcessBase::GetProcess()->GetProductType() != eProductTypeCallGenerator);

	case eProcessMediaMngr: // for Call Generator
	case eProcessGideonSim:
		return (CProcessBase::GetProcess()->GetProductType() == eProductTypeCallGenerator);

	default:
		return false;
	}
}

//////////////////////////////////////////////////////
bool CMcmsDaemonProcess::IsProcessExists(eProcessType ProcType)
{
	std::string procname;
	const size_t processNameArraySize = sizeof(ProcessNames)/sizeof(ProcessNames[0]);

	const size_t processTypeIndex = static_cast<size_t>(ProcType);

	if (processTypeIndex < processNameArraySize)
		procname = ProcessNames[processTypeIndex];

	return IsFileExists("Bin/" + procname);
}
