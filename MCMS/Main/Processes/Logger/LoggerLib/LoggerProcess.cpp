// LoggerProcess.cpp

#include "LoggerProcess.h"

#include <iomanip>

#include "Trace.h"
#include "SysConfig.h"
#include "TraceClass.h"
#include "ManagerApi.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "TraceStream.h"
#include "LoggerDefines.h"
#include "FaultsDefines.h"
#include "LoggerStatuses.h"
#include "OutsideEntities.h"
#include "TraceStatistics.h"
#include "OutsideEntityNames.h"
#include "OpcodesMcmsInternal.h"
#include "Log4cxxConfiguration.h"
#include "logdefs.h"

extern void LoggerManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess(void)
{
	return new CLoggerProcess;
}

// Static
const char* CLoggerProcess::GetProcessNameByMainEntity( const eMainEntities mainEntityType,
														const DWORD processType)
{
  switch (mainEntityType)
  {
  case eMcms:
    return CProcessBase::GetProcessName((eProcessType) processType);

  case eCentral_signaling:
    return GetCSProcessName((eCSProcesses) processType);

  case eEma:
    return GetEmaProcessName((eEmaProcesses) processType);

  case eCM_Switch:
  case eShelf:
  case eArtEntity:
  case eVideoEntity:
  case eCardManagerEntity:
  case eRTMEntity:
  case eMuxEntity:
  case eMpl:
  case eVmpEntity:
  case eAmpEntity:
  case eMpProxyEntity:
    return GetMplProcessName((eMplProcesses) processType);

  default:
    return "Invalid";
  }

  return "Invalid";
}

int CLoggerProcess::GetProcessIndexByMainEntity(const eMainEntities mainEntityType,
												const DWORD processType)
{
  switch (mainEntityType)
  {
  case eMcms:
    return (eProcessType) processType + 1;//eProcessMcmsDaemon startting from 0 not -1

  case eEma:
    return eEMA;//EMA

  case eCentral_signaling:
	return eCsModule;

  case eCM_Switch:
  case eShelf:
  case eArtEntity:
  case eVideoEntity:
  case eCardManagerEntity:
  case eRTMEntity:
  case eMuxEntity:
  case eMpl:
  case eVmpEntity:
  case eAmpEntity:
  case eMpProxyEntity:
    return eEMA + (eMplProcesses) processType + 1;//

  default:
    return -1;
  }

  return -1;
}

// Static
const char* CLoggerProcess::SenderToStr(CLoggerProcess::eSender sender)
{
  switch (sender)
  {
  case eSenderOut:
    return "External";

  case eSenderMCMS:
    return "MCMS";

  case eSenderEMA:
    return "EMA";

  default:
    FPASSERTSTREAM(true, "Illegal sender " << sender);
  }

  return "Unknown";
}

CLoggerProcess::CLoggerProcess(void) :
  m_IsDropMessage(false),
  m_CntNotSentTrace(0),
  m_SocketStatus(eSocketStatusNormal),
  m_pTraceStatistics(new CTraceStatistics),
  m_loggerFileList(new CLoggerFileList),
  m_CntDroppedMessages(eSenderQuantity, 0),
  m_pLicensing(NULL)
{
	m_pLog4cxxConfiguration = new CLog4cxxConfiguration;
}

// Virtual
CLoggerProcess::~CLoggerProcess(void)
{
  delete m_pTraceStatistics;
  delete m_loggerFileList;
  PDELETE(m_pLog4cxxConfiguration);
  if (m_pLicensing != NULL)
	  delete m_pLicensing;
}

// Virtual
TaskEntryPoint CLoggerProcess::GetManagerEntryPoint(void)
{
  return LoggerManagerEntryPoint;
}

// Virtual
const char* CLoggerProcess::NameOf(void) const
{
  return GetCompileType();
}

// Virtual
eProcessType CLoggerProcess::GetProcessType(void)
{
  return eProcessLogger;
}

// Virtual
BOOL CLoggerProcess::UsingSockets(void)
{
  return NO;
}

// Virtual
int CLoggerProcess::GetProcessAddressSpace(void)
{
  return 128 * 1024 * 1024;
}

// Virtual
DWORD CLoggerProcess::GetMaxTimeForIdle(void) const
{
  return 12000;
}

void CLoggerProcess::AddExtraStringsToMap(void)
{
	CStringsMaps::AddItem(TRACE_LEVEL_ENUM, eLevelDebug, CTrace::GetTraceLevelNameByValue(eLevelDebug));
	CStringsMaps::AddItem(TRACE_LEVEL_ENUM, eLevelTrace, CTrace::GetTraceLevelNameByValue(eLevelTrace));
	CStringsMaps::AddItem(TRACE_LEVEL_ENUM, eLevelInfoNormal, CTrace::GetTraceLevelNameByValue(eLevelInfoNormal));
	CStringsMaps::AddItem(TRACE_LEVEL_ENUM, eLevelInfoHigh, CTrace::GetTraceLevelNameByValue(eLevelInfoHigh));
	CStringsMaps::AddItem(TRACE_LEVEL_ENUM, eLevelWarn, CTrace::GetTraceLevelNameByValue(eLevelWarn));
	CStringsMaps::AddItem(TRACE_LEVEL_ENUM, eLevelError, CTrace::GetTraceLevelNameByValue(eLevelError));
	CStringsMaps::AddItem(TRACE_LEVEL_ENUM, eLevelFatal, CTrace::GetTraceLevelNameByValue(eLevelFatal));
	CStringsMaps::AddItem(TRACE_LEVEL_ENUM, eLevelOff, CTrace::GetTraceLevelNameByValue(eLevelOff));

	for (eEmaProcesses en = eTheOneTheOnlyEmaProcess;
	    en < NumOfEmaProcesses; en = (eEmaProcesses)(en + 1))
	{
		CStringsMaps::AddItem(EMA_PROCESSES_ENUM, en, EmaProcessNames[en]);
	}

	CStringsMaps::AddMinMaxItem(_0_TO_MAX_OPCODE_NAME_LENGTH, 0, STR_OPCODE_LEN);
	CStringsMaps::AddMinMaxItem(_0_TO_MAX_TASK_NAME_LENGTH,   0, MAX_TASK_NAME_LEN);
	CStringsMaps::AddMinMaxItem(_0_TO_MAX_CONTENT_LENGTH, 	  0, MAX_CONTENT_SIZE);
	CStringsMaps::AddMinMaxItem(_0_TO_MAX_OBJECT_NAME_LENGTH, 0, MAX_OBJECT_NAME_LEN);

	AddLog4cxxLogLevelStringsToMap();
	AddAllRMXProcessesToMap();
}

void CLoggerProcess::AddExtraStatusesStrings()
{
  AddStatusString(STATUS_BAD_MAIN_ENTITY, "Invalid main entity in trace");
  AddStatusString(STATUS_BAD_NEXT_HEADER_TYPE, "Trace contains invalid value in the next header type field ");
  AddStatusString(STATUS_BAD_MESSAGE_LEN, "Invalid message length");
  AddStatusString(STATUS_BAD_TRACE_LEVEL, "Invalid trace level");
  AddStatusString(STATUS_TRACE_DESERIALIZE_FAIL, "Deserialize failed");
}

void CLoggerProcess::AddLog4cxxLogLevelStringsToMap() const
{
	CStringsMaps::AddItem(LOG4CXX_LEVEL_ENUM , eTRACE, "TRACE");
	CStringsMaps::AddItem(LOG4CXX_LEVEL_ENUM , eDEBUG, "DEBUG");
	CStringsMaps::AddItem(LOG4CXX_LEVEL_ENUM , eINFO, 	"INFO");
	CStringsMaps::AddItem(LOG4CXX_LEVEL_ENUM , eWARN, "WARN");
	CStringsMaps::AddItem(LOG4CXX_LEVEL_ENUM , eERROR,"ERROR");
	CStringsMaps::AddItem(LOG4CXX_LEVEL_ENUM , eFATAL, "FATAL");

	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e1Hour,"1");
	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e2Hour,"2");
	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e4Hour,"4");
	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e6Hour,"6");
	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e8Hour,"8");
	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e12Hour,"12");
	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e16Hour,"16");
	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e24Hour,"24");
	CStringsMaps::AddItem(UDP_SEND_DURATION_ENUM,e48Hour,"48");

}

////////////////////////////////////////////////////////////////////////////////////////////
void CLoggerProcess::AddAllRMXProcessesToMap() const
{
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eInavlidProcess, "InvalidProcess");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMcmsDaemon, "McmsDaemon");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eConfigurator, "Configurator");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eLogger, "Logger");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eAuditor, "Auditor");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eFaults, "Faults");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eIPMCInterface, "IPMCInterface");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMcmsNetwork,"McmsNetwork");	
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eSNMP,"SNMPProcess");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eCSMngr, "CSMngr");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eCertMngr, "CertMngr");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMcuMngr,"McuMngr");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eLicenseServer,"LicenseServer");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eConfParty,"ConfParty");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eCards, "Cards");	
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eCDR, "CDR");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eResource, "Resource");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eSipProxy, "SipProxy");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eIce, "Ice");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eDNSAgent, "DNSAgent");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eGatekeeper, "Gatekeeper");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eQAAPI, "QAAPI");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eExchangeModule, "ExchangeModule");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eEncryptionKeyServer, "EncryptionKeyServer");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eAuthentication, "Authentication");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eInstaller, "Installer");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eRtmIsdnMngr, "RtmIsdnMngr");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eBackupRestore, "BackupRestore");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMplApi, "MplApi");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eCSApi, "CSApi");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eCollector, "Collector");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eSystemMonitoring, "SystemMonitoring");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMediaMngr, "MediaMngr");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eFailover, "Failover");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eLdapModule,"LdapModule");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMCCFMngr, "MCCFMngr");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eNotificationMngr, "NotificationMngr");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eApacheModule , "ApacheModule");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eGideonSim, "GideonSim");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eEndpointsSim, "EndpointsSim");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eDemo, "Demo");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eTestClient, "TestClient");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMcuCmd, "McuCmd");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eClientLogger, "ClientLogger");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eDiagnostics, "Diagnostics");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eUtility, "Utility");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eEMA, "EMA");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eOnlyMplProcess, "MplProcess");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMfaCardManager, "MfaCardManager");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eSwitchCardManager, "SwitchCardManager");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eVideoDsp, "VideoDsp");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eArtDsp, "ArtDsp");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eEmbeddedApacheModule, "EmbeddedApacheModule");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eIceManager, "IceManagerProcess");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eAMP, "AMP");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eVMP, "VMP");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMPProxy, "MpProxy");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eCsModule, "CS");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMfaLauncher, "MfaLauncher");
	CStringsMaps::AddItem(RMX_ALL_PROCESSES_ENUM , eMediaCardNA, "UndefinedProcess");
}

WORD CLoggerProcess::GetMaxNumberOfFiles() const
{
	// it's supposed the file size is 1024*1024 B (1 MB), the folder max size in GB
	WORD maxFileSizeInMb = DEFAULT_MAX_FILE_SIZE / (1024*1024);
	if (!maxFileSizeInMb)
	{
		DBGPASSERT(100+maxFileSizeInMb);
		return -1;
	}

	return (m_pLog4cxxConfiguration->GetMaxLogSize() * 1024) / maxFileSizeInMb;
}

void CLoggerProcess::DumpProcessStatistics(std::ostream& answer,
                                           CTerminalCommand& command) const
{
    m_pTraceStatistics->Dump(answer);
}

void CLoggerProcess::AddTraceToStatistics(const eMainEntities source,
										  const DWORD processType,
										  const DWORD size)
{
    m_pTraceStatistics->AddTrace(source, processType, size);
}

void CLoggerProcess::ResetStatistics()
{
    m_pTraceStatistics->Reset();
}

bool CLoggerProcess::GetDropMessageFlagAndIncrease(const eSender sender)
{
  bool ret = GetDropMessageFlag();
  if (!ret)
    return ret;

  // Updates statistics of dropped messages
  bool res = static_cast<unsigned int> (sender) >= m_CntDroppedMessages.size();
  PASSERTSTREAM_AND_RETURN_VALUE(res,
      "Size array " << m_CntDroppedMessages.size()
      << " is not suitable to the sender " << SenderToStr(sender),
      false);

  m_CntDroppedMessages[sender]++;

  return ret;
}

bool CLoggerProcess::GetDropMessageFlag(void) const
{
  bool ret;

  ret = m_IsDropMessage;

  return ret;
}

std::vector<unsigned long long> CLoggerProcess::GetDropMessageStat(void) const
{
  std::vector<unsigned long long> ret;

  ret = m_CntDroppedMessages;
  return ret;
}

void CLoggerProcess::SetDropMessageFlag(const bool val)
{

  m_IsDropMessage = val;

  // Starts new drop messages statistics
  if (val)
    std::fill(m_CntDroppedMessages.begin(), m_CntDroppedMessages.end(), 0);

}

CLog4cxxConfiguration* CLoggerProcess::GetLog4cxxConfiguration() const
{
	return m_pLog4cxxConfiguration;
}

ULONGLONG	CLoggerProcess::GetNumOfAllTraces() const
{
	return m_pTraceStatistics->GetNumOfAllTraces();
}
const LOGGER_LICENSING_S* CLoggerProcess::GetLicensingData()
{
	return m_pLicensing;
}
void CLoggerProcess::SetLicensingData(LOGGER_LICENSING_S& licensingData)
{
	if (m_pLicensing != NULL)
		delete m_pLicensing;
	m_pLicensing = new LOGGER_LICENSING_S;
	m_pLicensing->num_cop_parties = licensingData.num_cop_parties;
	m_pLicensing->num_cp_parties = licensingData.num_cp_parties;
	m_pLicensing->sys_card_mode = licensingData.sys_card_mode;
}
