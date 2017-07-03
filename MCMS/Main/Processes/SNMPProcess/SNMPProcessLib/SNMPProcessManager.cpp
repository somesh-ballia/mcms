// SNMPProcessManager.cpp

#include "SNMPProcessManager.h"

#include <iomanip>
#include <iostream>

#include <errno.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <fstream>
#include <list>

// All net-snmp includes are needful for netsnmp_dateandtime_set_buf_from_vars.
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "PrettyTable.h"
#include "ConfigManagerOpcodes.h"
#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "TaskApi.h"
#include "ApiStatuses.h"
#include "SystemFunctions.h"
#include "InitCommonStrings.h"
#include "Versions.h"
#include "ConfigManagerApi.h"
#include "SnmpData.h"
#include "SNMPProcessProcess.h"
#include "Request.h"
#include "DummyEntry.h"
#include "TerminalCommand.h"
#include "SnmpFileFormater.h"
#include "ifTable.h"
#include "ipAddrTable.h"
#include "ipAddressTable.h"
#include "h323McConfigTable.h"
#include "OpcodesMcmsInternal.h"
#include "HlogApi.h"
#include "FaultsDefines.h"
#include "ObjString.h"
#include "SNMPStructs.h"
#include "Segment.h"
#include "WrappersSnmp.h"
#include "TraceStream.h"
#include "OsTask.h"
#include "FipsMode.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "OsFileIF.h"
#include "Telemetry.h"
#include "NetSnmpIncludes.h"
#include "CommonStructs.h"


extern void snmpAgentMngrEntryPoint(void* appParam);
extern void SNMPProcessMonitorEntryPoint(void* appParam);

#define SNMP_CFG_FILE "Cfg/Snmp.xml"

#define SNMP_PERSISTENT_DIR	(MCU_MCMS_DIR+"/Cfg").c_str()


const WORD READY = 1;

PBEGIN_MESSAGE_MAP(CSNMPProcessManager)
  ONEVENT(XML_REQUEST                            , IDLE   , CSNMPProcessManager::HandlePostRequest)
  ONEVENT(SNMPD_WD                               , ANYCASE, CSNMPProcessManager::HandleSNMPD_WD)

  ONEVENT(SNMPD_FIX_SNMPD_TIMER      , ANYCASE, CSNMPProcessManager::HandleFixSNMPD)


  ONEVENT(SNMP_CS_INTERFACE_IP_IND               , READY  , CSNMPProcessManager::HandleCsIpInd)
  ONEVENT(SNMP_MFA_INTERFACE_IP_IND              , READY  , CSNMPProcessManager::HandleMfaIpInd)
  ONEVENT(SNMP_AGENT_READY_IND                   , IDLE   , CSNMPProcessManager::HandleSnmpAgentReadyInd)
  ONEVENT(SNMP_UPDATE_TELEMETRY_DATA_IND         , ANYCASE, CSNMPProcessManager::HandleSnmpUpdateTelemetryInd)
  ONEVENT(SNMP_UPDATE_MULTIPLE_TELEMETRY_DATA_IND, ANYCASE, CSNMPProcessManager::HandleSnmpUpdateMultipleTelemetryInd)
  ONEVENT(SNMP_GET_TELEMETRY_DATA_REQ            , ANYCASE, CSNMPProcessManager::HandleSnmpGetTelemetryReq)
  ONEVENT(SNMP_OTHER_PROCESS_READY               , ANYCASE, CSNMPProcessManager::TrigerSNMPConfigToOtherProcess)
  ONEVENT(SNMP_MFA_LINK_STATUS_IND , ANYCASE, CSNMPProcessManager::HandleMfaLinkStatusInd)
  ONEVENT(SNMP_CS_LINK_STATUS_IND , ANYCASE, CSNMPProcessManager::HandleCsLinkStatusInd)

PEND_MESSAGE_MAP(CSNMPProcessManager, CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CSNMPProcessManager)
  ON_TRANS("TRANS_SNMP", "UPDATE", CSnmpData, CSNMPProcessManager::HandleUpdateSnmpData)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CSNMPProcessManager)
  ONCOMMAND("mkMib"    , CSNMPProcessManager::HandleTerminalMakeMibFile , "insert All Alerts to MIB file")
  ONCOMMAND("update_gk", CSNMPProcessManager::HandleTerminalUpdateGk    , "Updates GK info in tables")
  ONCOMMAND("add_trap" , CSNMPProcessManager::HandleTerminalAddTrap     , "Add system alert to trap")
  ONCOMMAND("add_tele" , CSNMPProcessManager::HandleTerminalAddTelemetry, "Add telemetry")
  ONCOMMAND("get_tele" , CSNMPProcessManager::HandleTerminalGetTelemetry, "Get telemetry")
END_TERMINAL_COMMANDS

void SNMPProcessManagerEntryPoint(void* appParam)
{
  CSNMPProcessManager* pSNMPProcessManager = new CSNMPProcessManager;
  pSNMPProcessManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CSNMPProcessManager::GetMonitorEntryPoint()
{
  return SNMPProcessMonitorEntryPoint;
}

CSNMPProcessManager::CSNMPProcessManager():
    m_MngmntIp(0),
    m_telemetry(NULL)
{
	m_fixSNMPDState = FIX_SNMPD_NONE;
	m_snmpReady = FALSE;
	m_triesFix = 0;
	m_isFipsMode = false;

}

CSNMPProcessManager::~CSNMPProcessManager()
{
  delete m_telemetry;
}

void CSNMPProcessManager::SelfKill()
{
  DWORD snmpTaskId = CProcessBase::GetProcess()->GetSNMPTaskId();
  COsTask::SendSignal(snmpTaskId, SIGALRM);
  CManagerTask::SelfKill();
}

// Private
void CSNMPProcessManager::UpdateSysFlagBool(const char* flag,
                                            eTelemetryType type)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CSysConfig* cfg = proc->GetSysConfig();
  PASSERT_AND_RETURN(NULL == cfg);

  BOOL val;
  BOOL res = cfg->GetBOOLDataByKey(flag, val);
  PASSERTSTREAM_AND_RETURN(!res, "GetBOOLDataByKey:" << flag);

  m_telemetry->Update(CTelemetryValue(type, val));
}

void CSNMPProcessManager::ConvertSysFlagStringToIntAndUpdate(const char* flag,
                                                             eTelemetryType type)
{
  if (0 != strcmp(CFG_KEY_PAL_NTSC_VIDEO_OUTPUT, flag))
    return;

  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);
  CSysConfig* cfg = proc->GetSysConfig();
  PASSERT_AND_RETURN(NULL == cfg);

  std::string szVal;
  BOOL res = cfg->GetDataByKey(flag, szVal);
  PASSERTSTREAM_AND_RETURN(!res, "GetDataByKey:" << flag);

  unsigned int nVal = 0;
  if ("PAL" == szVal)
    nVal = 2;
  else if ("NTSC" == szVal)
    nVal = 1;
  else if ("AUTO" == szVal)
    nVal = 3;

  m_telemetry->Update(CTelemetryValue(type, nVal));
}


// Static, private.
std::vector<unsigned char> CSNMPProcessManager::SNMPDateAndTime(const char* date)
{
  FPASSERT_AND_RETURN_VALUE(NULL == date, std::vector<unsigned char>());

  struct tm tm;
  char* res = strptime(date, "%Y%m%d%H%M%S", &tm);
  FPASSERTSTREAM_AND_RETURN_VALUE(NULL == res,
    "Illegal date format: " << date,
     std::vector<unsigned char>());

  // Omits daylight saving time.
  tm.tm_isdst = 0;

  // Seconds since the Epoch
  time_t t = mktime(&tm);
  FPASSERTSTREAM_AND_RETURN_VALUE(-1 == t,
    "Illegal date format: " << date,
    std::vector<unsigned char>());

  size_t len;
  u_char* sd = date_n_time(&t, &len);

  return std::vector<unsigned char>(sd, sd+len);
}

void CSNMPProcessManager::ManagerStartupActionsPoint()
{

  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  CSysConfig* cfg = proc->GetSysConfig();
  PASSERT_AND_RETURN(NULL == cfg);

  BOOL jitc;
  BOOL res = cfg->GetBOOLDataByKey(CFG_KEY_JITC_MODE, jitc);
  PASSERTSTREAM_AND_RETURN(!res, "Unable to read " << CFG_KEY_JITC_MODE);

  ((CSNMPProcessProcess*)proc)->SetJitcMode(jitc);

  m_isFipsMode = GetSNMPFipsMode();

  TestAndEnterFipsMode(false);

  COsQueue dummyMbx;
  CTaskApi snmpTaskApi;
  CreateTask(&snmpTaskApi, snmpAgentMngrEntryPoint, &dummyMbx);


  CSnmpData snmpData;
  snmpData.UnSetIsFromEma();

  if (STATUS_OK != snmpData.ReadXmlFile(SNMP_CFG_FILE))
  {
	  TRACEINTO << "WriteXmlFile  Failed in ReadXmlFile ";

	  snmpData.WriteXmlFile(SNMP_CFG_FILE, "SNMP_DATA");
  }
  else
  {
	  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	  std::string errorMessage;
	  bool snmpDataValid = true;
	  bool trapsValid = true;
     if(true == jitc)
	 {
	       snmpData.SetVersion(eSnmpVer3);
	 }
	  
	  if (eProductTypeSoftMCUMfw==curProductType)
	  {
		  if (snmpData.Is_enable_snmp())
		  {
			  snmpDataValid  = CheckDataForMfw(snmpData, errorMessage);
			  if(!snmpDataValid)
			  {
				  TRACEINTOLVLERR << "SNMP.xml contains wrong Data for MFW.Return configuration back to default. error: " << errorMessage ;
				  snmpData.InitDefaults();
			  }
			  if (snmpDataValid)
			  {
				  trapsValid = CheckSnmpMFWTraps(snmpData, errorMessage, true);
				  if (!trapsValid)
				  {
					  TRACEINTOLVLERR << "Some invalid traps were deleted. last error: " << errorMessage;
				  }
			  }
			  if (!snmpDataValid || !trapsValid)
			  {
				  snmpData.WriteXmlFile(SNMP_CFG_FILE, "SNMP_DATA");
			  }

		  }
	  }
	  else
	  {
		  if (snmpData.Is_enable_snmp())
		  {
			  snmpDataValid = CheckSNMPData(snmpData, errorMessage);
			  if(!snmpDataValid)
			  {
				  TRACEINTOLVLERR << "SNMP.xml contains wrong Data. Return configuration back to default. error: " << errorMessage;
				  snmpData.InitDefaults();
			  }
			  trapsValid = CheckSNMPTraps(snmpData, errorMessage, true);
			  if (!trapsValid)
			  {
				  TRACEINTOLVLERR << "Some invalid traps were deleted: last error: " << errorMessage;
			  }
			  if (!snmpDataValid || !trapsValid)
			  {
				  snmpData.WriteXmlFile(SNMP_CFG_FILE, "SNMP_DATA");
			  }
		  }
	  }
  }
  //Begin Startup Feature
  proc->m_NetSettings.LoadFromFile();
  m_MngmntIp = proc->m_NetSettings.m_ipv4;

  if (proc->m_NetSettings.m_iptype == eIpType_IpV6 || proc->m_NetSettings.m_iptype == eIpType_Both)
  {
	  m_configForIpv6 = true;
  }
  else
  {
	  m_configForIpv6 = false;
  }

  //End Startup Feature


  ((CSNMPProcessProcess*)proc)->SetSnmpData(snmpData);

  // Creates state machine, couldn't be in ctor.
  m_telemetry = new CTelemetry;

  UpdateSysFlagBool(CFG_KEY_DEBUG_MODE                , eTT_MCUDebug);
  UpdateSysFlagBool(CFG_KEY_SEPARATE_NETWORK          , eTT_NetworkSeparation);
  UpdateSysFlagBool(CFG_KEY_GK_MANDATORY_FOR_CALLS_IN , eTT_IncomingCallsReqrGK);
  UpdateSysFlagBool(CFG_KEY_GK_MANDATORY_FOR_CALLS_OUT, eTT_OutgoingCallsReqrGK);
  UpdateSysFlagBool(CFG_KEY_RRQ_WITHOUT_GRQ           , eTT_RRQFirst);

  ConvertSysFlagStringToIntAndUpdate(CFG_KEY_PAL_NTSC_VIDEO_OUTPUT, eTT_PalNtsc);

  m_telemetry->Update(CTelemetryValue(eTT_UltraSecureMode,
                                      jitc ? "UltraSecure" : "Standard"));
  m_telemetry->Update(CTelemetryValue(eTT_IdentityDeviceType,
                                      ProductTypeToString(proc->GetProductType())));

  CVersions ver;
  std::string versionFilePath = VERSIONS_FILE_PATH;
  STATUS status = ver.ReadXmlFile(versionFilePath.c_str());
  PASSERTSTREAM_AND_RETURN(STATUS_OK != status,
    "ReadXmlFile: " << VERSIONS_FILE_PATH << ": Failed to parse: "
    << proc->GetStatusAsString(status));

  VERSION_S vver = ver.GetMcuVersion();
  std::stringstream buf;
  buf << vver.ver_major   << "."
      << vver.ver_minor   << "."
      << vver.ver_release << "."
      << vver.ver_internal;

  m_telemetry->Update(CTelemetryValue(eTT_IdentitySoftwareInfo, buf.str().c_str()));

  std::vector<unsigned char> date = SNMPDateAndTime(ver.GetMcuBuildDate());
  m_telemetry->Update(CTelemetryValue(eTT_IdentityBuildDate, date));

  bool enabled = snmpData.Is_enable_snmp();
  if (enabled)
    m_telemetry->EnableSlider(true);
}

void CSNMPProcessManager::DeclareStartupConditions()
{
  CActiveAlarm aa(FAULT_GENERAL_SUBJECT,
                  SNMP_AGENT_INIT_FAILED,
                  MAJOR_ERROR_LEVEL,
                  "SNMP agent failed to start",
                  true,
                  true);
  AddStartupCondition(aa);
}

STATUS CSNMPProcessManager::HandleSnmpAgentReadyInd(CSegment* pMsg)
{
  TRACEINTO << "State:" << m_state << " -> " << READY;

  m_state = READY;

  RemoveActiveAlarmByErrorCode(SNMP_AGENT_INIT_FAILED);

  CManagerApi apiCSMngr(eProcessCSMngr);
  apiCSMngr.SendOpcodeMsg(SNMP_CS_INTERFACE_IP_REQ);

  CManagerApi apiCards(eProcessCards);
  apiCards.SendOpcodeMsg(SNMP_MFA_INTERFACE_IP_REQ);
  //Begin Startup Feature
	  CProcessBase* proc = CProcessBase::GetProcess();
	  char buffer [128];
	  memset(buffer,0,sizeof(buffer));
	  SystemDWORDToIpString(proc->m_NetSettings.m_ipv4, buffer);
	  const APIU8*  	mngmtIpv6Address = proc->m_NetSettings.m_ipv6_0.addr.ip;
	  if(IsIpNull(proc->m_NetSettings.m_ipv6_0.addr))
		  UpdateSnmp_MNGMNT(buffer, (byte *)proc->m_NetSettings.m_ipv6_0.addr.ip , FALSE );
	  else
		  UpdateSnmp_MNGMNT(buffer, (byte *)proc->m_NetSettings.m_ipv6_0.addr.ip , TRUE );
	  memset(buffer,0,sizeof(buffer));
	  SystemDWORDToIpString(proc->m_NetSettings.m_ipv4Shelf, buffer);
	  if(IsIpNull(proc->m_NetSettings.m_ipv6_Shelf.addr))
		  UpdateSnmp_Switch(buffer, (byte *)proc->m_NetSettings.m_ipv6_Shelf.addr.ip , FALSE );
	  else
		  UpdateSnmp_Switch(buffer, (byte *)proc->m_NetSettings.m_ipv6_Shelf.addr.ip , TRUE );
	  ConfigSNMPState();
  //End Startup Feature
  StartTimer(SNMPD_WD, SNMPD_WD_TIME_INTERVAL);

  return STATUS_OK;
}

STATUS CSNMPProcessManager::HandleSnmpUpdateTelemetryInd(CSegment* seg)
{
  PASSERT_AND_RETURN_VALUE(NULL == seg, STATUS_FAIL);

  m_telemetry->Update(CTelemetryValue(*seg));
  return STATUS_OK;
}

STATUS CSNMPProcessManager::HandleSnmpUpdateMultipleTelemetryInd(CSegment* seg)
{
  PASSERT_AND_RETURN_VALUE(NULL == seg, STATUS_FAIL);
  unsigned int nCount;
  *seg >> nCount;

  for (unsigned int i = 0; i < nCount; i++)
    m_telemetry->Update(CTelemetryValue(*seg));

  return STATUS_OK;
}

void CSNMPProcessManager::HandleSnmpGetTelemetryReq(CSegment* seg)
{
  if (NULL == seg)
  {
    PASSERTMSG (true, "Illegal input parameter");
    STATUS status = ResponedClientRequest(STATUS_FAIL);
    PASSERT(STATUS_OK != status);
    return;
  }

  unsigned int t;
  *seg >> t;

  eTelemetryType type = static_cast<eTelemetryType>(t);
  CSegment* out = new CSegment(m_telemetry->Get(type).GetSegment());

  STATUS status = ResponedClientRequest(STATUS_OK, out);
  PASSERT(STATUS_OK != status);
}


void CSNMPProcessManager::TrigerSNMPConfigToOtherProcess(CSegment* seg)
{
  PASSERT_AND_RETURN(NULL == seg);


  unsigned int ptype;
  *seg >> ptype;

  TRACEINTO << " TrigerSNMPConfigToOtherProcess: " << (int)ptype <<  " m_snmpReady " << (int)m_snmpReady;

  if (m_snmpReady)
  {

	  SendSNMPConfigToOtherProcess(static_cast<eProcessType>(ptype),
			  IsSNMPEnabled());
  }
  else {
	  m_readyProcesses.insert(ptype);
  }
}

bool   CSNMPProcessManager::DoesNeedHardReset(const CSnmpData& snmpDataPrev, const CSnmpData& snmpDataNew)

{
  // prev  curr   action
  // ----------------------
  // 0     0      RestartSnmpd
  // 0     1      HardRestartSnmpd
  // 1     0      HardRestartSnmpd
  // 1     1      RestartSnmpd

    return (snmpDataPrev.Is_enable_snmp() != snmpDataNew.Is_enable_snmp() ||
	    snmpDataPrev.GetEngineID() != snmpDataNew.GetEngineID()); 

}

STATUS CSNMPProcessManager::HandleUpdateSnmpData(CRequest* pGetRequest)
{
  STATUS retStatus = STATUS_OK;
  pGetRequest->SetConfirmObject(new CDummyEntry());

  if (pGetRequest->GetAuthorization() != SUPER)
  {
    pGetRequest->SetStatus(STATUS_NO_PERMISSION);
    return STATUS_OK;
  }

  CSNMPProcessProcess* process =
    (CSNMPProcessProcess*) CProcessBase::GetProcess();
  const CSnmpData& snmpDataPrev = process->GetSnmpData();
  CSnmpData* pSnmpDataNew =
    (CSnmpData*)pGetRequest->GetRequestObject();

  eProductType curProductType = CProcessBase::GetProcess()->GetProductType();

  std::string errorMessage;

  bool dataIsOK = true;
  if (eProductTypeSoftMCUMfw==curProductType)
  {
	  if (!CheckDataForMfw(*pSnmpDataNew, errorMessage))
	  {
		  TRACEINTO << "Recieved wrong SNMP configuration for SNMP: " << errorMessage;

		  dataIsOK = false;
	  }
	  TRACEINTO << "After CheckDataForMfw " << (int)dataIsOK;

	  if (dataIsOK)
	  {
		  dataIsOK = CheckSnmpMFWTraps(*pSnmpDataNew, errorMessage, true);
		  if (!dataIsOK)
		  {
			  TRACEINTOLVLERR << "Some invalid traps were deleted. last error: " << errorMessage;
		  }
	  }
  }
  else
  {
	  if( !CheckSNMPData(*pSnmpDataNew, errorMessage))
	  {
		  TRACEINTO << "Recieved wrong SNMP configuration : " << errorMessage;
		  dataIsOK = false;
	  }

  }

  if (!dataIsOK)
  {
	  pGetRequest->SetStatus(STATUS_INVALID_MFW_SNMP_CONFIGURATION);
	  pGetRequest->SetExDescription(errorMessage.c_str());
	  return STATUS_OK;
  }

  bool isNeedReset = DoesNeedHardReset(snmpDataPrev, *pSnmpDataNew);

  if (STATUS_INCONSISTENT_PARAMETERS == retStatus)
  {
    pGetRequest->SetStatus(retStatus);
    return retStatus;
  }

  pSnmpDataNew->SetMngmntIp(m_MngmntIp);

  pSnmpDataNew->UnSetIsFromEma();
  process->SetSnmpData(*pSnmpDataNew);

  pSnmpDataNew->WriteXmlFile(SNMP_CFG_FILE, "SNMP_DATA");

  if (m_fixSNMPDState != FIX_SNMPD_NONE)
  {
 	  DeleteTimer(SNMPD_FIX_SNMPD_TIMER);
 	 m_triesFix = 0;
  }
   m_fixSNMPDState = FIX_SNMPD_NONE;


  STATUS Status = ConfigureSnmpDaemon(isNeedReset);

  pGetRequest->SetStatus(retStatus);

  if (isNeedReset)
  {
    bool enabled = pSnmpDataNew->Is_enable_snmp();
    m_telemetry->EnableSlider(enabled);
    SendSNMPConfigToOtherProcess(enabled);
  }

  return STATUS_OK;
}

bool CSNMPProcessManager::CheckAuthAgentIsSHA(const CSnmpData& snmpData, std::string& errorMessage) const
{
	if (snmpData.GetSecurityInfo().GetCommunityPermission().GetSnmpV3Param().GetAuthProtocol() != eSapSHA)
	{
		errorMessage = "Authentication Protocol must be SHA";
		return false;
	}
	return true;
}

bool CSNMPProcessManager::CheckPrivAgentIsAES(const CSnmpData& snmpData, std::string& errorMessage) const
{
	if (snmpData.GetSecurityInfo().GetCommunityPermission().GetSnmpV3Param().GetPrivProtocol() != eSppAES)
	{
		errorMessage = "Privacy Protocol must be AES";
		return false;
	}
	return true;
}

bool CSNMPProcessManager::CheckAuthTrapIsSHA(const CSnmpTrapCommunity& trapCommunity, std::string& errorMessage) const
{
	if (trapCommunity.GetSnmpV3Param().GetAuthProtocol() != eSapSHA)
	{
		errorMessage = "Each trap's Authentication Protocol must be SHA";
		return false;
	}

	return true;
}

bool CSNMPProcessManager::CheckPrivTrapIsAES(const CSnmpTrapCommunity& trapCommunity, std::string& errorMessage) const
{
	if (trapCommunity.GetSnmpV3Param().GetPrivProtocol() != eSppAES)
	{
		errorMessage = "Each trap's Privacy Protocol must be AES";
		return false;
	}

	return true;
}


bool CSNMPProcessManager::CheckGeneralTrapData(const CSnmpTrapCommunity& trapCommunity, std::string& errorMessage) const
{
	if (trapCommunity.GetGeneralTrapDestination().length() == 0 || trapCommunity.GetGeneralTrapDestination() == "0.0.0.0")
	{
		errorMessage = "Trap IP address is required";
		return false;
	}

	if (!ValidatePassword(trapCommunity.GetSnmpV3Param(), errorMessage))
	{
		errorMessage = "Invalid Trap: " + errorMessage;
		return false;
	}

	return true;
}

bool CSNMPProcessManager::ValidatePassword(const CSnmpV3SecurityParams& snmpV3SecurityParams, std::string& errorMessage) const
{
	const int MIN_AUTH_LEN = 8;
	const int MAX_AUTH_LEN =48;

	const int MIN_PRIV_LEN = 8;
	const int MAX_PRIV_LEN =64;



	if (snmpV3SecurityParams.GetAuthProtocol() != eSapNone)
	{
		int length = snmpV3SecurityParams.GetAuthPassword().length();

		if (length < MIN_AUTH_LEN || length > MAX_AUTH_LEN)
		{
			errorMessage = "The Authentication Protocol password cannot be less than 8 or longer than 48 characters.";
			return false;

		}
	}
	if (snmpV3SecurityParams.GetPrivProtocol() != eSppNone)
	{
		int length = snmpV3SecurityParams.GetPrivPassword().length();

		if (length < MIN_AUTH_LEN || length > MAX_PRIV_LEN)
		{
			errorMessage = "The Private Protocol password cannot be less than 8 or longer than 64 characters.";
			return false;

		}
	}
	return true;
}

bool CSNMPProcessManager::CheckSNMPData(const CSnmpData& snmpData, std::string& errorMessage) const
{

	if ( m_isFipsMode && snmpData.GetVersion() == eSnmpVer3)
	{
		if (!CheckAuthAgentIsSHA(snmpData, errorMessage))
		{
			return false;
		}
		if (!CheckPrivAgentIsAES(snmpData, errorMessage))
		{
			return false;
		}
	}

	if (!ValidatePassword(snmpData.GetSecurityInfo().GetCommunityPermission().GetSnmpV3Param(), errorMessage))
	{
		return false;
	}

	return true;

}

bool CSNMPProcessManager::CheckDataForMfw(const CSnmpData& snmpData, std::string& errorMessage) const
{
	if (!ValidatePassword(snmpData.GetSecurityInfo().GetCommunityPermission().GetSnmpV3Param(), errorMessage))
	{
		return false;
	}

	if (snmpData.GetVersion() != eSnmpVer3)
	{
		errorMessage = "Only SNMP V3 is allowed";
		return false;
	}
	if (!snmpData.GetSecurityInfo().AcceptAllRequests())
	{
		errorMessage = "Accept SNMP packets from all hosts must be TRUE";
		return false;
	}
	if (snmpData.GetSecurityInfo().GetSnmpTrapVersion() != eSnmpVer3Trap )
	{
		errorMessage = "Traps version must be V3";
		return false;
	}

	if (snmpData.GetSecurityInfo().GetCommunityPermission().GetCommunityName() != "public")
	{
		errorMessage = "Accepted host community name must be public";
		return false;
	}

	if (snmpData.GetSecurityInfo().GetCommunityPermission().GetSnmpV3Param().GetSecLevel() != eSslPriv)
	{
		errorMessage = "Security Level must be Authorized, Private";
		return false;
	}

	if (!CheckAuthAgentIsSHA(snmpData, errorMessage))
	{
		return false;
	}
	if (!CheckPrivAgentIsAES(snmpData, errorMessage))
	{
		return false;
	}

	return true;

}

bool CSNMPProcessManager::CheckSNMPTraps(CSnmpData& snmpData, std::string& errorMessage, bool deleteInvalid ) const
{
	const vector<CSnmpTrapCommunity>&  trapDestinations =  snmpData.GetSecurityInfo().GetTrapDestinations();
	vector<CSnmpTrapCommunity>::const_iterator itTraps =  trapDestinations.begin();

	list<CSnmpTrapCommunity > trapsList;

	bool retVal = true;
	for (; itTraps != trapDestinations.end(); ++itTraps)
	{
		bool trapValid = true;
		if (!CheckGeneralTrapData(*itTraps, errorMessage))
		{
			trapValid = false;
			retVal =  false;
		}
		if (trapValid && m_isFipsMode && itTraps->GetSnmpTrapVersionForConfig() == eSnmpVer3Trap)
		{
			if (itTraps->GetSnmpV3Param().GetSecLevel() == eSslPriv || itTraps->GetSnmpV3Param().GetSecLevel() == eSslAuth)
			{
				if (!CheckAuthTrapIsSHA(*itTraps, errorMessage))
				{
					trapValid = false;
					retVal =  false;
				}
				if (trapValid && itTraps->GetSnmpV3Param().GetSecLevel() == eSslPriv)
				{
					if (!CheckPrivTrapIsAES(*itTraps, errorMessage))
					{
						trapValid = false;
						retVal =  false;
					}
				}
			}
		}
		if (!trapValid)
		{
			if (deleteInvalid)
			{
				trapsList.push_back(*itTraps);
			}
			else
			{
				return retVal;
			}
		}
	}
	if (deleteInvalid && trapsList.size() > 0)
	{
		TRACEINTOLVLERR << "Delete Invalid traps. number: " << trapsList.size();
		snmpData.DeleteTraps(trapsList);
	}

	return retVal;

}

bool CSNMPProcessManager::CheckSnmpMFWTraps(CSnmpData& snmpData, std::string& errorMessage, bool deleteInvalid) const
{
	bool retval= true;

	const vector<CSnmpTrapCommunity>&  trapDestinations =  snmpData.GetSecurityInfo().GetTrapDestinations();
	vector<CSnmpTrapCommunity>::const_iterator itTraps =  trapDestinations.begin();

	list<CSnmpTrapCommunity > trapsList;

	for (; itTraps != trapDestinations.end(); ++itTraps)
	{

		bool trapValid = true;
		if (!CheckGeneralTrapData(*itTraps, errorMessage))
		{
			trapValid = false;
			retval =  false;
		}

		if (trapValid && itTraps->GetSnmpTrapVersionForConfig() != eSnmpVer3Trap)
		{
			errorMessage = "Each trap's version must be V3";
			trapValid = false;
			retval =  false;
		}

		if (trapValid && itTraps->GetSnmpV3Param().GetSecLevel() != eSslPriv)
		{
			errorMessage = "Each trap’s Security Level must be Authorized, Private";
			trapValid = false;
			retval =  false;
		}
		if (trapValid && !CheckAuthTrapIsSHA(*itTraps, errorMessage))
		{
			trapValid = false;
			retval =  false;
		}
		if (trapValid && !CheckPrivTrapIsAES(*itTraps, errorMessage))
		{
			trapValid = false;
			retval =  false;
		}
		if (!trapValid)
		{
			if (deleteInvalid)
			{
				trapsList.push_back(*itTraps);
			}
			else
			{
				return retval;
			}
		}
	}

	if (deleteInvalid && trapsList.size() > 0)
	{
		TRACEINTOLVLERR << "Delete Invalid traps for MFW. number: " << trapsList.size();
		snmpData.DeleteTraps(trapsList);
	}

	return retval;
}

bool CSNMPProcessManager::IsSNMPEnabled() const
{
	CProcessBase* proc = CProcessBase::GetProcess();
	PASSERT_AND_RETURN_VALUE(NULL == proc, STATUS_FAIL);

	return (((CSNMPProcessProcess*)proc)->GetSnmpData()).Is_enable_snmp();
}

// again, the configurator not always manage to write file so we try fixing it from time to time
void CSNMPProcessManager::FixSNMPIpInConfFIleIfNeed()
{
	CProcessBase* proc = CProcessBase::GetProcess();
	PASSERT_AND_RETURN(NULL == proc);

	CSnmpData& snmpData = ((CSNMPProcessProcess*)proc)->GetSnmpData();
	if (snmpData.GetMngmntIp() != m_MngmntIp)
	{
		TRACEINTO << "FixSNMPIpInConfFIleIfNeed " << m_MngmntIp;

		snmpData.SetMngmntIp(m_MngmntIp);
		snmpData.WriteXmlFile(SNMP_CFG_FILE, "SNMP_DATA");
	}
}

void   CSNMPProcessManager::ConfigSNMPState()
{
	 // trying to fix snmpd
		  bool enabled = IsSNMPEnabled();
		  if (ConfigureSnmpDaemon(true) != STATUS_OK)
		  {
			  if(enabled)
			  {
				  // we will try to reread configuration later
				  TRACEINTO << "Failed starting snmpd - retry";

				  m_fixSNMPDState = FIX_SNMPD_RECONFIGURE;
			  }
		  }
		  else
		  {
			  if (enabled && !IsFileExists(AGENTX_PIPE_NAME))
			  {
				  TRACEINTO << "agentX wasn't started properly";

				  m_fixSNMPDState = FIX_SNMPD_RERUN;
			  }
		  }
		  if (m_fixSNMPDState != FIX_SNMPD_NONE)
		  {
			  TRACEINTO << "Executing fix snmpd timer";
			  StartTimer(SNMPD_FIX_SNMPD_TIMER, 6 * SECOND);
		  }
		  else
		  {
			  TRACEINTO << "Done reconfigured SMNP ";
		  }

		  if (!m_snmpReady)
		  {
			  m_snmpReady = TRUE;
			  NotifyAllReadyProcessesOnReady(enabled);
		  }
		  FixSNMPIpInConfFIleIfNeed();

}



STATUS CSNMPProcessManager::HandleCsIpInd(CSegment* pMsg)
{
  const SNMP_CS_INFO_S* pParam = (SNMP_CS_INFO_S*)pMsg->GetPtr();

  TRACEINTO << CSnmpCSInfoWrapper(*pParam);

  char buffer [128];
  SystemDWORDToIpString(pParam->csIp, buffer);
  UpdateSnmp_CS(buffer);

  SystemDWORDToIpString(pParam->gkIp, buffer);
  UpdateSnmp_GK(buffer);

  eTelemetryType card;
  switch (pParam->type)
  {
    case eIPProtocolType_SIP:
      m_telemetry->Update(CTelemetryValue(eTT_SIPStatus, 2u));  // OK.
      break;

    case eIPProtocolType_H323:
      m_telemetry->Update(CTelemetryValue(eTT_H323Status, 2u));
      break;

    case eIPProtocolType_SIP_H323:
      m_telemetry->Update(CTelemetryValue(eTT_SIPStatus, 2u));
      m_telemetry->Update(CTelemetryValue(eTT_H323Status, 2u));
      break;

    default: PASSERTSTREAM(true, "Illegal protocol type " << pParam->type);
  }

  return STATUS_OK;
}

STATUS CSNMPProcessManager::HandleMfaIpInd(CSegment* pMsg)
{
  const SNMP_CARDS_INFO_S* pParam = (const SNMP_CARDS_INFO_S*)pMsg->GetPtr();

  TRACEINTO << CSnmpCardsInfoWrapper(*pParam);

  char buffer [128];

  if (0 != pParam->mfaLinks[0].ipAddress)
  {
    SystemDWORDToIpString(pParam->mfaLinks[0].ipAddress, buffer);
    UpdateSnmp_Mfa1(buffer);
   
    AddInterfaceIndex(pParam->mfaLinks[0].boardId, eMedia_1If);

  }

  if (0 != pParam->mfaLinks[1].ipAddress)
  {
    SystemDWORDToIpString(pParam->mfaLinks[1].ipAddress, buffer);
    UpdateSnmp_Mfa2(buffer);
    AddInterfaceIndex(pParam->mfaLinks[1].boardId, eMedia_2If);

  }

  if (0 != pParam->mfaLinks[2].ipAddress)
  {
    SystemDWORDToIpString(pParam->mfaLinks[2].ipAddress, buffer);
    UpdateSnmp_Mfa3(buffer);
    AddInterfaceIndex(pParam->mfaLinks[2].boardId, eMedia_3If);
  }

  if (0 != pParam->mfaLinks[3].ipAddress)
  {
    SystemDWORDToIpString(pParam->mfaLinks[3].ipAddress, buffer);
    UpdateSnmp_Mfa4(buffer);
    AddInterfaceIndex(pParam->mfaLinks[3].boardId,  eMedia_4If);
  }

  return STATUS_OK;
}

STATUS CSNMPProcessManager::HandleSNMPD_WD(CSegment* pMsg)
{
  StartTimer(SNMPD_WD, SNMPD_WD_TIME_INTERVAL);

  CSNMPProcessProcess* process =
    (CSNMPProcessProcess*) CProcessBase::GetProcess();
  const CSnmpData& snmpData = process->GetSnmpData();
  if (NO == snmpData.Is_enable_snmp())
  {
	  return STATUS_OK;
  }

  static DWORD failureCnt = 0;

  if (!IsProcessUp())
  {
    //In MFW we don't control the snmpd, it is administrator's responsibility,
    //Therefore we don't won't to send more then 3 errors in case of snmpd
    //down, but we still continue trying to connect to it.
    eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
    if(eProductTypeSoftMCUMfw==curProductType && failureCnt>2)
        return STATUS_OK;

    failureCnt++;
    CMedString message = "snmpd should be up, but is not running failure number ";
    message << failureCnt;

    BOOL isFullOnly = FALSE;
    CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT,
                        SNMPD_FAILURE,
                        MAJOR_ERROR_LEVEL,
                        message.GetString(),
                        isFullOnly);

    HardRestartSnmpd(true);
  }
  else
  {
	  failureCnt = 0;
  }

  return STATUS_OK;
}
// trying to fix snmpd
STATUS CSNMPProcessManager::HandleFixSNMPD(CSegment* pMsg)
{
	const int MAX_NUM_TRIES = 3;
	++m_triesFix;

	bool notifyToMcuMngr = FALSE;
	if(!IsSNMPEnabled())
	{
		TRACEINTO << " snmpd is not enabled exiting...";
		m_triesFix = 0;
		return STATUS_OK;
	}

	FixSNMPDEnum 		newfixSNMPDState =  m_fixSNMPDState;
	bool fileExist = FALSE;
	if (newfixSNMPDState == FIX_SNMPD_RECONFIGURE)  // we previously failed on writing configuration file
	{
		STATUS stat = ConfigureSnmpDaemon(true);

		if (stat != STATUS_OK)
		{
			TRACEINTO << " Failed configuring snmpd again";
		}
		else
		{
			notifyToMcuMngr = TRUE;
			fileExist = IsFileExists(AGENTX_PIPE_NAME);
			if (!fileExist)
			{
				TRACEINTO << " Managed configuring snmp  but agent id not exist";
				newfixSNMPDState = FIX_SNMPD_RERUN;
				--m_triesFix;
			}
		}
		if (fileExist || m_triesFix >= MAX_NUM_TRIES)
		{
			m_triesFix = 0;
			newfixSNMPDState = FIX_SNMPD_NONE;
			notifyToMcuMngr = TRUE;
		}
	}
	else if (newfixSNMPDState == FIX_SNMPD_RERUN)
	{
		fileExist = IsFileExists(AGENTX_PIPE_NAME);
		if (!fileExist)
		{
			//tried
			TRACEINTO << "Restarted snmpd because agentX still not exist";
			HardRestartSnmpd(true);
			notifyToMcuMngr = TRUE;
			fileExist = IsFileExists(AGENTX_PIPE_NAME);
			if (!fileExist)
			{
				TRACEINTO << "Failed run agentX again "<< (int)m_triesFix;
			}
		}
		 // else no need to notify - it just took some time to get the agent up
		if (fileExist || (m_triesFix >=MAX_NUM_TRIES))
		{
			m_triesFix = 0;
			newfixSNMPDState = FIX_SNMPD_NONE;
		}
	}

	TRACEINTO << " old m_fixSNMPDState " << (int)m_fixSNMPDState << " new state " << newfixSNMPDState << " m_triesFix " << m_triesFix << " notifyToMcuMngr " << (int)notifyToMcuMngr;
	m_fixSNMPDState = newfixSNMPDState;

	if (notifyToMcuMngr)
	{
	    SendSNMPConfigToOtherProcess(static_cast<eProcessType>(eProcessMcuMngr), true);
	}
	if (m_fixSNMPDState != FIX_SNMPD_NONE)
	{
			// retry fixing things
		  StartTimer(SNMPD_FIX_SNMPD_TIMER, 7 * SECOND);
	}

	FixSNMPIpInConfFIleIfNeed();
	return STATUS_OK;

}


// Static
bool CSNMPProcessManager::IsProcessUp()
{
  bool   is_alive;
  STATUS status = IsProcAlive("snmpd", is_alive);
  //STATUS status = IsProcAlive(snmpdStr.c_str(), is_alive);

  FTRACECOND_AND_RETURN_VALUE(STATUS_OK != status,
   "Assumed that snmpd is alive",
   true);

  return is_alive;
}

STATUS CSNMPProcessManager::ConfigureSnmpDaemon(bool isNeedReset) const
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN_VALUE(NULL == proc, STATUS_FAIL);

  STATUS Status = STATUS_OK;
  const CSnmpData& snmpData = ((CSNMPProcessProcess*)proc)->GetSnmpData();


  CVersions pSystemVersions;
  std::string versionFilePath = VERSIONS_FILE_PATH;
  pSystemVersions.ReadXmlFile(versionFilePath.c_str());
  VERSION_S mcuVersion;
  mcuVersion = pSystemVersions.GetMcuVersion();


  std::ostringstream out;
  out << "master agentx\n"
	  << "AgentXSocket " << AGENTX_PIPE_NAME << "\n"
	  << "agentXPerms 777\n";
 	  if (snmpData.GetVersion() != eSnmpVer3)
  	  {
  		out <<  "group notConfigGroup v1  notConfigUser\n"
  			<< "group notConfigGroup v2c notConfigUser\n";
  	  }
  	  else 
  	  {
  		out <<  "group notConfigGroup usm notConfigUser\n";
  	  }
  	  out <<  "view systemview included .1.3.6.1.2.1.1\n"
      << "view systemview included .1.3.6.1.2.1.2\n"
      << "view systemview included .1.3.6.1.2.1.4\n"
      << "view systemview included .1.3.6.1.2.1.118.1.2\n"
      << "view systemview included .1.3.6.1.4.1.13885.9.1.1\n"
      << "view systemview included .0.0.8.341.1.1.4.2.1\n"
      << "access notConfigGroup \"\" any noauth exact systemview none none"
      << std::endl;

  eProductType curProductType = proc->GetProductType();
  eProductFamily curProductFamily = proc->GetProductFamily();

  if (snmpData.Is_enable_snmp())
  {

    if (IsTarget()|| eProductFamilySoftMcu==curProductFamily)
    {
      if (0 != m_MngmntIp)
      {
          out << "agentaddress udp:161" << std::endl;  // enable udp on port 161
          if (m_configForIpv6)
          {
        	  out << "agentaddress udp6:161" << std::endl;  // enable udp6 on port 161
          }

      }
      else
      {
        // McuMngr did not send Mngmnt Ind
        out << "agentaddress localhost:161" << std::endl;  // disable
      }
    }
    else
    {
        out << "agentaddress udp:8090" << std::endl;  // enable udp on port 8090
        if (m_configForIpv6)
        {
        	out << "agentaddress udp6:8090" << std::endl;  // enable udp6 on port 8090
        }
    }
    // todo take care of ipv6.

    if (m_MngmntIp != 0 && IsTarget())
    {
  	  char buffer [128];
  	  SystemDWORDToIpString(m_MngmntIp, buffer);
  	  out << "[snmp] clientAddr " << buffer << endl;
    }
  }
  else
  {

    if (IsTarget()|| eProductFamilySoftMcu==curProductFamily)
      out << "agentaddress localhost:161" << std::endl;  // disable
    else
      out << "agentaddress localhost:8099"<< std::endl;  // disable
  }

  if (snmpData.GetVersion() == eSnmpVer3)
  {
	  if (snmpData.GetEngineID() != "")
	  {
		  out << "engineID " << snmpData.GetEngineID() << endl;
	  }
 	  out << "[snmp] persistentDir " << SNMP_PERSISTENT_DIR << endl;
  }
  
  out << "sysdescr ";
  switch (curProductFamily)
  {
  case eProductFamilyRMX:
  case eProductFamilySoftMcu:
    out << "RMX.";
    break;
  case eProductFamilyCallGenerator:
    out << "CallGenerator.";
    break;
  default:
	// Note: some enumeration value are not handled in switch. Add default to suppress warning.
	break;
  }

  out << mcuVersion.ver_major << "."
      << mcuVersion.ver_minor << "."
      << mcuVersion.ver_release << "."
      << mcuVersion.ver_internal << endl;

  std::string contact = snmpData.GetContactName();
  if (contact.empty())
    contact = "\"\"";

  std::string location = snmpData.GetLocation();
  if (location.empty())
    location = "\"\"";

  std::string name = snmpData.GetSystemName();
  if (name.empty())
    name = "\"\"";

  out << "syscontact  " << contact << endl;
  out << "syslocation " << location << endl;
  out << "sysname     " << name << endl;
  
  //eProductType curProductType = proc->GetProductType();
  if (eProductTypeRMX4000 == curProductType)
    out << "sysobjectid 1.3.6.1.4.1.13885.9.1.10.2" << std::endl;  // RMX4000
  else if(eProductTypeRMX2000 == curProductType || eProductFamilySoftMcu == curProductFamily /*all SoftMcu products*/)
    out << "sysobjectid 1.3.6.1.4.1.13885.9.1.10.1" << std::endl;  // RMX2000
  else if (eProductTypeRMX1500 == curProductType)
    out << "sysobjectid 1.3.6.1.4.1.13885.9.1.10.3" << std::endl;  // RMX1500
  else if (eProductTypeCallGenerator == curProductType)
    out << "sysobjectid 1.3.6.1.4.1.13885.9.1.10.77" << std::endl;  // Call Generator

  const CSnmpSecurity& snmpSecurity = snmpData.GetSecurityInfo();

  switch (snmpData.GetVersion())
  {
  case eSnmpVer1:
  case eSnmpVer2:
    snmpSecurity.CommunityCfg(out);
    break;

  case eSnmpVer3:
    // Writes SNMPv3 user configuration
    snmpSecurity.GetSnmpV3Param().UserConfig(out);
    break;

  default:
    PASSERTSTREAM_AND_RETURN_VALUE(true,
        "Unknown SNMP version:" << snmpData.GetVersion(),
        STATUS_FAIL);
  }
  

  WORD isSendAuthenticationTraps = snmpSecurity.SendAuthenticationTraps();
  if (isSendAuthenticationTraps)
    out << "authtrapenable 1" << std::endl;
  else
    out << "authtrapenable 2" << std::endl;


  if (snmpData.Is_enable_snmp())
    snmpSecurity.TrapCfg(out);

  const string fname = MCU_TMP_DIR+"/snmpd.conf";
  
  // Writes configuration file with root permissions
  OPCODE opcode = WriteSNMPFile(fname, out.str(), 0640);

  PASSERTSTREAM_AND_RETURN_VALUE(opcode != STATUS_OK,
        "Unable to write file " << fname << ": " << proc->GetStatusAsString(opcode),
        opcode);
 if (!isNeedReset)
  {
	  RestartSnmpd();
  }
  else
  {
	  //TRACEINTO << "HardRestartSnmpd after configuring ";
	  HardRestartSnmpd(snmpData.Is_enable_snmp());
  }


  return Status;
}

STATUS CSNMPProcessManager::HandleTerminalMakeMibFile(CTerminalCommand& command,
                                                      std::ostream& answer)
{
  if (1 != command.GetNumOfParams() && 2 != command.GetNumOfParams())
  {
    answer << "bad usage no file name\n";
    answer << "Usage: >mkbib [source file name] [destination file name]";
    return STATUS_OK;
  }

  const std::string& fileNameSource = command.GetToken(eCmdParam1);
  const std::string& fileNameDest = (2 == command.GetNumOfParams() ?
      command.GetToken(eCmdParam2) : fileNameSource);

  bool ret = CSnmpFileFormater::UpdateNsmpMibFile(fileNameSource, fileNameDest, answer);
  if (ret)
    answer << "\nUpdate Mib file completed successfully\n"
           << fileNameSource.c_str() << " -> " << fileNameDest.c_str() << '\n';
  else
    answer << "\n\nFAILED to update Mib file\n";

  return STATUS_OK;
}

STATUS CSNMPProcessManager::HandleTerminalUpdateGk(CTerminalCommand& command,
                                                   std::ostream& answer)
{
  IpTable_AddRow("0.0.0.1", eCSIf);
  IpTable_AddRow("0.0.0.2", eCSIf);
  IpTable_AddRow("0.0.0.3", eMcuMgmtIf);
  IpTable_AddRow("0.0.0.4", eMcuMgmtIf);

  IfTable_AddRow(eCSIf, "Signaling Host", eethernetCsmacd);
  IfTable_AddRow(eCSIf, "Signaling Host", eethernetCsmacd);
  IfTable_AddRow(eMcuMgmtIf, "Control Unit", eethernetCsmacd);
  IfTable_AddRow(eMcuMgmtIf, "Control Unit", eethernetCsmacd);

  H323Table_SetGK("1.2.3.4");
  H323Table_SetGK("1.2.3.4");
  H323Table_SetGK("1.2.3.4");
  H323Table_SetGK("1.2.3.4");

  return STATUS_OK;
}

STATUS CSNMPProcessManager::HandleTerminalAddTrap(CTerminalCommand& command,
                                                  std::ostream& answer)
{
  AddActiveAlarm(FAULT_CARD_SUBJECT,
                 AA_RTM_ISDN_STARTUP_PROBLEM,
                 MAJOR_ERROR_LEVEL,
                 "RtmIsdn startup was received too many times",
                 true,
                 true,
                 1,  // displayBoardId as 'userId' (token)
                 2,
                 0,  // unitId
                 FAULT_TYPE_MPM);
  return STATUS_OK;
}

STATUS CSNMPProcessManager::HandleTerminalAddTelemetry(CTerminalCommand& command,
                                                       std::ostream& answer)
{
  // 1
  CSegment datavalue;
  datavalue << (DWORD)eTT_NumPorts << 80u;
  m_telemetry->Update(CTelemetryValue(datavalue));

  // 2
  CSegment datavalue1;
  datavalue1 << (DWORD)eTT_NumVideoPorts << 160u;
  m_telemetry->Update(CTelemetryValue(datavalue1));

  // 3
  CSegment datavalue2;
  datavalue2 << (DWORD)eTT_SuccessfulNewCalls << 100u;
  m_telemetry->Update(CTelemetryValue(datavalue2));

  eTelemetryType types[] =  {
    eTT_NumPorts,
    eTT_NumVideoPorts,
    eTT_SuccessfulNewCalls
  };

  CPrettyTable<const char*, const char*> tb("name", "value");
  for (eTelemetryType* ii = types; ii != ARRAYEND(types); ++ii)
  {
    std::ostringstream buf;
    buf << m_telemetry->Get(*ii);
    tb.Add(TelemetaryTypeToStr(*ii), buf.str().c_str());
  }
  
  tb.Dump(answer);

  return STATUS_OK;
}

STATUS CSNMPProcessManager::HandleTerminalGetTelemetry(CTerminalCommand& command,
                                                       std::ostream& answer)
{
  if (0 == command.GetNumOfParams())
  {
    answer << *m_telemetry;
    return STATUS_OK;
  }

  std::string szPara = command.GetToken(eCmdParam1);
  DWORD type = atoi(szPara.c_str());
  answer << m_telemetry->Get((eTelemetryType)type);

  return STATUS_OK;
}

void CSNMPProcessManager::HardRestartSnmpd(bool enable) const
{
	 CConfigManagerApi().StopSnmpd();
	 if (enable || (CProcessBase::GetProcess()->GetProductType()==eProductTypeNinja) ) 
	 // TRACEINTO << "HardRestartSnmpd " << (int)snmpData.Is_enable_snmp();
	 {
		  CConfigManagerApi().StartSnmpd();
	 }
}

void CSNMPProcessManager::RestartSnmpd() const
{
  CConfigManagerApi().RestartSnmpd();
}

void CSNMPProcessManager::UpdateSnmp_MNGMNT(const string& mngmntAddress, const byte* ipv6Address, bool isMngmtIpv6Address)
{   
  TRACEINTO << mngmntAddress.c_str(); 
   
  IpTable_AddRow(mngmntAddress.c_str(), eMcuMgmtIf);
  CIpTableAddress::instance().IpTableAddress_AddIp4Address(mngmntAddress.c_str(), eMcuMgmtIf);
    
  if (isMngmtIpv6Address)
  {
	  CIpTableAddress::instance().IpTableAddress_AddIp6Address(ipv6Address, eMcuMgmtIf);
  }  
  if (!mngmntAddress.empty())
  {
    IfTable_AddRow(eMcuMgmtIf, "Control Unit", eethernetCsmacd);
  }  
}

void CSNMPProcessManager::UpdateSnmp_CS(const string& csAddress)
{
  TRACEINTO << csAddress.c_str();
  
  IpTable_AddRow(csAddress.c_str(), eCSIf);
    
  if (!csAddress.empty())
  {
    IfTable_AddRow(eCSIf, "Signaling Host", eethernetCsmacd);
  }
}

void CSNMPProcessManager::UpdateSnmp_GK(const string& gkAddress)
{
  TRACEINTO << gkAddress.c_str();

  H323Table_SetGK(gkAddress.c_str());
}

void CSNMPProcessManager::UpdateSnmp_Switch(const string& shelfAddress, const byte* ipv6Address, bool isShelfIpv6Address)
{
  TRACEINTO << shelfAddress.c_str();
  
  IpTable_AddRow(shelfAddress.c_str(), eShelfMgmtIf);  
  
  //currently shelf ipv6 address is not displayed  
  /*CIpTableAddress::instance().IpTableAddress_AddIp4Address(shelfAddress, eShelfMgmtIf); 
  if (isShelfIpv6Address)
  {
 	  CIpTableAddress::instance().IpTableAddress_AddIp6Address(ipv6Address, eShelfMgmtIf);
  }*/
  
  if (!shelfAddress.empty())
    IfTable_AddRow(eShelfMgmtIf, "Shelf Management", eethernetCsmacd);
}

void CSNMPProcessManager::UpdateSnmp_Mfa1(const string& mfa1Address)
{
  TRACEINTO << mfa1Address.c_str();
 
  IpTable_AddRow(mfa1Address.c_str(), eMedia_1If);
    
  if (!mfa1Address.empty())
  {
    IfTable_AddRow(eMedia_1If, "Slot1 Media Processing Module", eethernetCsmacd);
  }

}

void CSNMPProcessManager::UpdateSnmp_Mfa2(const string& mfa2Address)
{
  TRACEINTO << mfa2Address.c_str();
  IpTable_AddRow(mfa2Address.c_str(), eMedia_2If);
  
  if (!mfa2Address.empty())
  {
    IfTable_AddRow(eMedia_2If, "Slot2 Media Processing Module", eethernetCsmacd);
  }
}

void CSNMPProcessManager::UpdateSnmp_Mfa3(const string& mfa3Address)
{
  TRACEINTO << mfa3Address.c_str();

  IpTable_AddRow(mfa3Address.c_str(), eMedia_3If);
  if (!mfa3Address.empty())
    IfTable_AddRow(eMedia_3If, "Slot3 Media Processing Module", eethernetCsmacd);
}

void CSNMPProcessManager::UpdateSnmp_Mfa4(const string& mfa4Address)
{
  TRACEINTO << mfa4Address.c_str();

  IpTable_AddRow(mfa4Address.c_str(), eMedia_4If);
  if (!mfa4Address.empty())
    IfTable_AddRow(eMedia_4If, "Slot4 Media Processing Module", eethernetCsmacd);
}

void CSNMPProcessManager::SendSNMPConfigToOtherProcess(bool enabled)
{
  static const eProcessType procs[] = {
      eProcessConfParty,
      eProcessResource,
      eProcessMcuMngr,
      eProcessCSMngr,      
      eProcessRtmIsdnMngr,
      eProcessCards
  };

  for (const eProcessType* ptype = procs; ptype != ARRAYEND(procs); ++ptype)
    SendSNMPConfigToOtherProcess(*ptype, enabled);
}

void CSNMPProcessManager::SendSNMPConfigToOtherProcess(eProcessType ptype,
                                                       bool enabled)
{
  CSegment* seg = new CSegment;
  *seg << static_cast<BOOL>(enabled);
  CManagerApi(ptype).SendMsg(seg, SNMP_CONFIG_TO_OTHER_PROGRESS);
}


void CSNMPProcessManager::NotifyAllReadyProcessesOnReady(bool enabled)
{
	  //TRACEINTO << "Notify procs: " << (int)m_readyProcesses.size();

	  std::set<unsigned int>::iterator itSet =  m_readyProcesses.begin();
	  for(; itSet != m_readyProcesses.end(); ++itSet)
	  {
		    SendSNMPConfigToOtherProcess(static_cast<eProcessType>(*itSet), enabled);
	  }
	  m_readyProcesses.clear();
}


STATUS CSNMPProcessManager::HandleMfaLinkStatusInd(CSegment* pMsg)
{

	const LINK_STATUS_S* pParam = (const LINK_STATUS_S*)pMsg->GetPtr();

	//	TRACEINTO << CSnmpLinkStatusWrapper(*pParam);


	eInterfaceIndex interfaceIndex =  GetInterfaceIndex(pParam->boardId);
	if (interfaceIndex == eUnknownIf)
	{
		TRACESTR(eLevelError) << "HandleLinkStatusInd on unknown interface : boardId " <<  pParam->boardId << " portId " << pParam->portId << " isUp " << (int)pParam->isUp;
		return STATUS_OK;
	}

	unsigned int ifIndex = (unsigned int) interfaceIndex;

	SendLinkUpDownTrap(pParam->isUp, ifIndex);

	return STATUS_OK;
}


void CSNMPProcessManager::AddInterfaceIndex(WORD 	boardId, eInterfaceIndex interfaceIndex)
{ 
	TRACEINTO << " AddInterfaceIndex " << boardId << " interfaceIndex " << (int)interfaceIndex;
	
	m_boardToInterfaceIndex[boardId] = interfaceIndex;
}

eInterfaceIndex CSNMPProcessManager::GetInterfaceIndex(DWORD 	boardId) const
{
	std::map< WORD, eInterfaceIndex >::const_iterator it=
			m_boardToInterfaceIndex.find(boardId);

	if (it == m_boardToInterfaceIndex.end())
	{
		return eUnknownIf;
	}
	return it->second;
}


void CSNMPProcessManager::SendLinkUpDownTrap(BOOL isUp, unsigned int ifIndex) const
{

	TRACEINTO << "SendLinkUpDownTrap   isUp "  << (int)isUp<<  " ifIndex " << (int)ifIndex;

	oid             objid_snmptrap[] = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
	size_t          objid_snmptrap_len = OID_LENGTH(objid_snmptrap);


	netsnmp_variable_list *notification_vars = NULL;
	oid             notification_oid[] =   { 1, 3 , 6 , 1 ,6, 3, 1, 1, 5, 3 }; //link down

	if (isUp)
	{
		notification_oid[9] = 4;  //link up
	}

	size_t          notification_oid_len = OID_LENGTH(notification_oid);

	// add in the trap definition object
	snmp_varlist_add_variable(&notification_vars,
							  objid_snmptrap, objid_snmptrap_len,
							  ASN_OBJECT_ID,
							  (u_char *) notification_oid,
							  notification_oid_len * sizeof(oid));

	oid    ifIndex_oid[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 1};
	size_t   ifIndex_oid_len = OID_LENGTH(ifIndex_oid);

	// add date and time
	snmp_varlist_add_variable(&notification_vars,
		ifIndex_oid, ifIndex_oid_len ,
		ASN_UNSIGNED,
		(const unsigned char*)&ifIndex,sizeof(ifIndex));

	oid    ifAdminStatus_oid[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 7 };
	size_t   ifAdminStatus_oid_len = OID_LENGTH(ifAdminStatus_oid);


	// 1.3.6.1.2.1.2.2.1.7   1 : up  2 : down  3 : testing
	unsigned int adminStatus = 1;

	// add date and time
	snmp_varlist_add_variable(&notification_vars,
		ifAdminStatus_oid, ifAdminStatus_oid_len ,
		ASN_UNSIGNED,
		(const unsigned char*)&adminStatus,sizeof(adminStatus));

	// 1.3.6.1.2.1.2.2.1.81 1: up  2 : down  3 : testing  4 : unknown......

	oid    ifOperStatus_oid[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 8 };
	size_t   ifOperStatus_oid_len = OID_LENGTH(ifAdminStatus_oid);

	unsigned int operStatus = isUp ? 1 : 2;

	// add date and time
	snmp_varlist_add_variable(&notification_vars,
		   ifOperStatus_oid, ifOperStatus_oid_len ,
		   ASN_UNSIGNED,
		   (const unsigned char*)&operStatus,sizeof(operStatus));

	send_v2trap( notification_vars);

  snmp_free_varbind(notification_vars);
}

STATUS CSNMPProcessManager::HandleCsLinkStatusInd(CSegment* pMsg)
{
	BOOL isUp;
	*pMsg >> isUp;
	
	SendLinkUpDownTrap(isUp, (unsigned int)eCSIf);
	
	return STATUS_OK;
}

OPCODE CSNMPProcessManager::WriteSNMPFile(const std::string& fname, const std::string& content, mode_t mode) const
{
  // Removes old content
	OPCODE stat;
	std::ofstream out(fname.c_str(), std::ios_base::trunc);
	if (out)
	{
		out << content;
		out.flush();

		if (out)
		{
			if (mode != 0)
			{
				chmod(fname.c_str(), mode);
			}

			stat = STATUS_OK;
		}
		else
		{
			TRACEINTO << "Unable to write to file " << fname << ": " << strerror(errno);
			return STATUS_FILE_WRITE_ERROR;
		}
	  }
	  else
	  {
		TRACEINTO <<  "Unable to open file " << fname << ": " << strerror(errno);


		return STATUS_FILE_OPEN_ERROR;
	  }

	return stat;
}



