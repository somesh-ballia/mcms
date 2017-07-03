// LdapModuleManager.cpp

#include "LdapModuleManager.h"

#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "LdapModuleCfg.h"
#include "DummyEntry.h"
#include "OpcodesMcmsInternal.h"
#include "ManagerApi.h"
#include "FaultsDefines.h"
#include "FipsMode.h"
#include "InternalProcessStatuses.h"
#include "ConfigManagerApi.h"
#include "StringsLen.h"

const DWORD CHECK_STARTUP_END_TIME = 1 * SECOND;
const WORD CHECK_STARTUP_END_TIMER = 2023;

PBEGIN_MESSAGE_MAP(CLdapModuleManager)
  ONEVENT(XML_REQUEST, IDLE, CLdapModuleManager::HandlePostRequest)
  ONEVENT(LDAP_MODULE_AD_SERVER_AVAILABE_IND,  	ANYCASE	,  CLdapModuleManager::OnLdapClientAdServerAvailableInd)
  ONEVENT(LDAP_MODULE_OCSP_SERVER_AVAILABE_IND, ANYCASE	,  CLdapModuleManager::OnLdapClientOCSPResponderAvailableInd)
  ONEVENT(LDAP_AUTHENTICATION_LOGIN_REQ,		ANYCASE ,  CLdapModuleManager::OnAuthenticationLoginReq )
  ONEVENT(LDAP_AUTHENTICATION_LOGIN_IND,		ANYCASE ,  CLdapModuleManager::OnLdapClientAuthenticationLoginInd )
  ONEVENT(CHECK_STARTUP_END_TIMER,          	ANYCASE,   CLdapModuleManager::OnCheckStartupEndTimerAnycase )
  ONEVENT(LDAP_ADD_ACTIVE_ALARM,				ANYCASE,   CLdapModuleManager::OnLdapAddActiveAlarm )
  ONEVENT(LDAP_MODULE_AD_SERVER_AVAILABE_REQ,	ANYCASE,   CLdapModuleManager::OnAuthenticationProcessLdapConfigurationReq)
  ONEVENT(CHECK_AD_SERVER_AVAILABILITY,			ANYCASE,   CLdapModuleManager::OnCheckAdServerAvailablilty)
  ONEVENT(SECURITY_PKI_CFG,					    ANYCASE,   CLdapModuleManager::OnLdapSecurityPKICfg)
PEND_MESSAGE_MAP(CLdapModuleManager,CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CLdapModuleManager)
	ON_TRANS("TRANS_MCU", "SET_ACTIVE_DIRECTORY_CONFIGURATION", CLdapModuleCfg, CLdapModuleManager::HandleSetActiveDirCfg)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CLdapModuleManager)
	ONCOMMAND("secure_connection",CLdapModuleManager::HandleSecureConnection,"test Secure connection TLS PKI ")
	ONCOMMAND("set_ldap_verbose_mode",CLdapModuleManager::HandleSetLdapDebugMode,"Set ldap to verbose(trace) mode")
	ONCOMMAND("set_normal_mode",CLdapModuleManager::HandleSetLdapNormalMode,"Set ldap to normal mode")
	ONCOMMAND("test_multiple_ous",CLdapModuleManager::HandleMultipleOUs,"Debug looks for multiple ou in ldap")
END_TERMINAL_COMMANDS

extern void LdapModuleMonitorEntryPoint(void* appParam);

void LdapModuleManagerEntryPoint(void* appParam)
{
	CLdapModuleManager * pLdapModuleManager = new CLdapModuleManager;
	pLdapModuleManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CLdapModuleManager::GetMonitorEntryPoint()
{
	return LdapModuleMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLdapModuleManager::CLdapModuleManager()
{
	m_pLdapModuleCfg = NULL;
	m_pLdapModuleClientControl = NULL;
	m_delNatCmd = "";
}

//////////////////////////////////////////////////////////////////////
CLdapModuleManager::~CLdapModuleManager()
{
	POBJDELETE(m_pLdapModuleCfg);
	POBJDELETE(m_pLdapModuleClientControl);
}

void CLdapModuleManager::ManagerPostInitActionsPoint()
{
	TestAndEnterFipsMode();

	POBJDELETE(m_pLdapModuleCfg);	// delete CFG if exists
	m_pLdapModuleCfg = new CLdapModuleCfg();

	// read from Configuration file. If failed - write default configuration
	int status = m_pLdapModuleCfg->ReadXmlFile();
	if( status != STATUS_OK )
		m_pLdapModuleCfg->WriteXmlFile();

	STATUS validationStatus = m_pLdapModuleCfg->ValidateConfiguration();

	// update the global Ldap module configuration
	::SetLdapModuleCfg(m_pLdapModuleCfg);

	CProcessBase::GetProcess()->m_NetSettings.LoadFromFile();
	// wait till startup ends to init m_pLdapModuleClientControltype filter text
	StartTimer(CHECK_STARTUP_END_TIMER, CHECK_STARTUP_END_TIME);
}

//////////////////////////////////////////////////////////////////////
void CLdapModuleManager::OnCheckStartupEndTimerAnycase()
{
	// wait till startup ends to init m_pLdapModuleClientControl
	if( IsStartupFinished() == FALSE )
	{
		PTRACE(eLevelInfoNormal,"CLdapModuleManager::OnCheckStartupEndTimerAnycase - System Still In startup");
		StartTimer(CHECK_STARTUP_END_TIMER, CHECK_STARTUP_END_TIME);
		return;
	}


	PTRACE(eLevelInfoNormal,"CLdapModuleManager::OnCheckStartupEndTimerAnycase - System finished startup");



	CManagerApi api(eProcessMcuMngr);
	CSegment*  pRetParam = new CSegment;
	api.SendMsg(pRetParam, SECURITY_PKI_CFG_REQ);

	if (::GetLdapModuleCfg()->GetEnableDirServices() == TRUE)
	{
		TRACESTR(eLevelInfoNormal) << "\nCLdapModuleManager::OnCheckStartupEndTimerAnycase - AD Enabled";
		CreateNatRuleIfNeeded();
		if (m_pLdapModuleClientControl)
			m_pLdapModuleClientControl->CheckAndReportAdServerAvailability();
	}

	InformAuthenticationProcessOnLdapConfiguration();
}
////////////////////////////////////////////////////////////////////
void CLdapModuleManager::CreateNatRuleIfNeeded()
{

    BOOL bLAN_REDUNDANCY = FALSE;
    CConfigManagerApi configApi;
    STATUS opcode = STATUS_OK;
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if ((eProductTypeGesher == curProductType) || (eProductTypeNinja == curProductType))
	{
		eIpType iptype = CProcessBase::GetProcess()->m_NetSettings.m_iptype;
		bLAN_REDUNDANCY = CProcessBase::GetProcess()->GetLanRedundancy(iptype);
	}
	else
		bLAN_REDUNDANCY = IsBondingEnabled();

	if(bLAN_REDUNDANCY)
	{
		if(!m_delNatCmd.empty())
		{
			opcode = configApi.DeleteAddNatRule(m_delNatCmd);
			if(STATUS_OK == opcode )
			{
				m_delNatCmd ="";
			}
			else
				TRACESTR(eLevelInfoNormal) << "failed to delete Nat rule error opcode " << opcode;
		}
		string targetAddress = m_pLdapModuleCfg->GetDirNameAddress();
		DWORD ipv4 =  CProcessBase::GetProcess()->m_NetSettings.m_ipv4;
		char ServerIpAddressStr[IP_ADDRESS_LEN];
		SystemDWORDToIpString(ipv4,ServerIpAddressStr);
		string port;

		if(e389 == m_pLdapModuleCfg->GetDirPort())
			port="389";
		else
			port = "636";

		string strIpv4 = ServerIpAddressStr;

		//TRACESTR(eLevelInfoNormal) << "send nat rule targetAddress : " <<targetAddress  << " ,strIpv4 : " << strIpv4
		//							<<" , port : "<< port ;
		opcode = configApi.AddNatRule(m_delNatCmd,targetAddress,strIpv4,port,YES,NO);

		if(STATUS_OK != opcode )
		{
				m_delNatCmd ="";
				TRACESTR(eLevelInfoNormal) << "failed to add Nat rule error opcode " << opcode;
		}


	}

}
//////////////////////////////////////////////////////////////////////
void CLdapModuleManager::OnAuthenticationProcessLdapConfigurationReq(CSegment* pMsg)
{
	if (IsStartupFinished() == FALSE)
	{
		PTRACE(eLevelInfoNormal,"CLdapModuleManager::OnAuthenticationProcessLdapConfigurationReq - System Still In startup");
		return; //will be handled after startup ends
	}

	PTRACE(eLevelInfoNormal,"CLdapModuleManager::OnAuthenticationProcessLdapConfigurationReq");
	InformAuthenticationProcessOnLdapConfiguration();
}

//////////////////////////////////////////////////////////////////////
void CLdapModuleManager::InformAuthenticationProcessOnLdapConfiguration()
{
	BOOL bIsLdapEnabled = m_pLdapModuleCfg->GetEnableDirServices();
	CSegment* pSeg = new CSegment();
	*pSeg << (WORD)bIsLdapEnabled;
	CManagerApi api(eProcessAuthentication);
	api.SendMsg(pSeg, LDAP_MODULE_AD_SERVER_AVAILABE_IND);
}
///////////////////////////////////////////////////////////////////////
void CLdapModuleManager::OnLdapClientOCSPResponderAvailableInd(CSegment* pMsg)
{
	string errorMsg;
 	WORD bIsOCSPResponderAvailable;
	*pMsg >> bIsOCSPResponderAvailable;
	*pMsg >> errorMsg;
	TRACESTR(eLevelInfoNormal) << "\nCLdapModuleManager::OnLdapClientOCSPResponderAvailableInd - bIsOCSPResponderAvailable = "
							  << bIsOCSPResponderAvailable
							  << " ,error= " << errorMsg;

	if(bIsOCSPResponderAvailable == FALSE)
	{
		if (!IsActiveAlarmExistByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER))
			{
				AddActiveAlarm(FAULT_GENERAL_SUBJECT,
							   AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER,
							   MAJOR_ERROR_LEVEL, errorMsg, true, true);
			}
	}
	else
	   if (IsActiveAlarmExistByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER))
			RemoveActiveAlarmByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER);
}

//////////////////////////////////////////////////////////////////////
void CLdapModuleManager::OnLdapClientAdServerAvailableInd(CSegment* pMsg)
{
	string errorMsg;
	//3. This section should be after m_pLdapModuleClientControl has finished to check the server
	WORD bIsAdServerAvailable;
	*pMsg >> bIsAdServerAvailable;
	*pMsg >> errorMsg;
	TRACESTR(eLevelInfoNormal) << "\nCLdapModuleManager::OnLdapModuleAdServerAvailableInd - bIsAdServerAvailable = " << bIsAdServerAvailable;
	HandleActiveAlarm(bIsAdServerAvailable,errorMsg);
}

/////////////////////////////////////////////////////////////////////
void CLdapModuleManager::HandleActiveAlarm(BOOL bIsAdServerAvailable,string &errorMsg)
{
	//Update AA:
	if (bIsAdServerAvailable == FALSE)
	{
		if (!IsActiveAlarmExistByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_ACTIVE_DIRECTORY_SERVER))
		{ //TODO: add specific reason in the str filed
			AddActiveAlarm(FAULT_GENERAL_SUBJECT,
						   AA_LDAP_FAILED_TO_CONNECT_TO_ACTIVE_DIRECTORY_SERVER,
						   MAJOR_ERROR_LEVEL, errorMsg, true, true);
		}
	}
	else if (IsActiveAlarmExistByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_ACTIVE_DIRECTORY_SERVER))
		RemoveActiveAlarmByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_ACTIVE_DIRECTORY_SERVER);
}

//////////////////////////////////////////////////////////////////////
void CLdapModuleManager::OnAuthenticationLoginReq(CSegment* pMsg)
{
	TRACESTR(eLevelInfoNormal) << "\nCLdapModuleManager - OnAuthenticationLoginReq";

	CSegment* pServerRspMsg = NULL;
	
	if (m_pLdapModuleClientControl)
		pServerRspMsg = m_pLdapModuleClientControl->AuthenticationLoginReq(pMsg);

	PASSERT_AND_RETURN(pServerRspMsg == NULL);
	
	DWORD requestId;
	*pServerRspMsg >> requestId;

	DWORD status;
	*pServerRspMsg >> status;

	DWORD authorizationLevel;
	*pServerRspMsg >> authorizationLevel;

	DWORD apiStatus = ConvertExtDBStatus(status);


	TRACESTR(eLevelInfoNormal) << "\nCLdapModuleManager::OnLdapClientAuthenticationLoginInd - requestId = " << requestId << ", status = " << status;

	//TODO: Translate AD groups to RMX authorization groups (using Authorization to group mapping configuration module),
	//and forward it to Authentication process

	CSegment*  pRspMsg = new CSegment;
	*pRspMsg << (DWORD)requestId;
	*pRspMsg << (DWORD)apiStatus;
	*pRspMsg << (DWORD)authorizationLevel;
	//TODO: add also failure message and RMX authorization group

	ResponedClientRequest(LDAP_AUTHENTICATION_LOGIN_IND, pRspMsg);

	POBJDELETE(pServerRspMsg);

}

//////////////////////////////////////////////////////////////////////
void CLdapModuleManager::OnLdapClientAuthenticationLoginInd(CSegment* pMsg)
{
	//3. This section should be after m_pLdapModuleClientControl has finished to check the server
	DWORD requestId;
	*pMsg >> requestId;

	DWORD status;
	*pMsg >> status;

	TRACESTR(eLevelInfoNormal) << "\nCLdapModuleManager::OnLdapClientAuthenticationLoginInd - requestId = " << requestId << ", status = " << status;

	//TODO: Translate AD groups to RMX authorization groups (using Authorization to group mapping configuration module),
	//and forward it to Authentication process

	CSegment* pSeg = new CSegment();
	*pSeg << (DWORD)requestId;
	*pSeg << (DWORD)status;
	//TODO: add also failure message and RMX authorization group
//	CManagerApi api(eProcessAuthentication);
//	api.SendMsg(pSeg, LDAP_AUTHENTICATION_LOGIN_IND);


//	char pContent[1024];
//	sprintf(pContent, "<CONFIRM_USER_DETAILS><RETURN_STATUS><ID>%d</ID><DESCRIPTION>OK</DESCRIPTION></RETURN_STATUS><TOKEN>%d</TOKEN><ACTION><AUTHENTICATE><AUTHORIZATION_GROUP>administrator</AUTHORIZATION_GROUP></AUTHENTICATE></ACTION></CONFIRM_USER_DETAILS>",status, requestId);

//	TRACESTR(eLevelInfoNormal) << "\nCLdapModuleManager::OnLdapClientAuthenticationLoginInd Response : " << pContent;

//	CSegment* pRespMsg = new CSegment; // pMsg will delete inside
//									*pRespMsg  << (DWORD)strlen(pContent);
//									*pRespMsg  << pContent;

	CTaskApi api;
	const COsQueue* pAuthenticationManager = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessAuthentication,eManager);
	api.CreateOnlyApi(*pAuthenticationManager);
	api.SendMsg(pSeg, LDAP_AUTHENTICATION_LOGIN_IND);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CLdapModuleManager::HandleSetActiveDirCfg(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,"CLdapModuleManager::HandleSetActiveDirCfg");

	if (pRequest->GetAuthorization() == SUPER)
	{
		STATUS status = STATUS_OK;

		// 1. Check if system finished Startup, if not - status
		if( IsStartupFinished() == FALSE )
		{
			PTRACE(eLevelInfoNormal,"CLdapModuleManager::HandleSetActiveDirCfg - System In startup");
			status = STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP;
		}

		// 2. Validate new configuration + Jitc restrictions
		CLdapModuleCfg* pNewLdapCfg = NULL;
		if (STATUS_OK == status )
		{
			pNewLdapCfg = (CLdapModuleCfg*)(pRequest->GetRequestObject());
			status = pNewLdapCfg->ValidateConfiguration();
		}

		if (STATUS_OK == status )
		{
			// in case of Jitc mode, only one user (administrator) is allowed - check with Authentication process
			if (pNewLdapCfg->IsJitcMode())
			{
				status = pNewLdapCfg->CheckOperatorList();

				if (status == STATUS_MORE_THAN_ONE_USER_IN_LIST)
				{
					status = STATUS_ONLY_ONE_ACCOUNT_IS_ALLOWED_IN_JITC_LDAP;
				}
			}
		}

		// 3. Self message - Check connectivity with the new parameters and update configuration + Authentication process
		if ((STATUS_OK == status ) &&
			(pNewLdapCfg->GetEnableDirServices()))
		{
/*			if (m_pLdapModuleClientControl)
			{
				BOOL bIsAvailable = (m_pLdapModuleClientControl->CheckAdServerAvailability(pNewLdapCfg));
				if (!bIsAvailable)
					status = STATUS_ACTIVE_DIRECTORY_SERVER_IS_NOT_AVAILABLE | WARNING_MASK;
				HandleActiveAlarm(bIsAvailable);
			}
*/

			CSegment* pMsg = NULL;
			CManagerApi ldapModuleManagerApi(eProcessLdapModule);
			ldapModuleManagerApi.SendMsg(pMsg,CHECK_AD_SERVER_AVAILABILITY);

			status = STATUS_IN_PROGRESS;
		}

		// 4. Update configuration and Authentication process (regardless the AD server availability
		if ((STATUS_OK == status ) || (STATUS_IN_PROGRESS == status))
		{
			UpdateLdapCfgParams(pNewLdapCfg);
			InformAuthenticationProcessOnLdapConfiguration();
		}
		else
		{
			PTRACE2INT(eLevelInfoNormal,"CLdapModuleManager::HandleSetActiveDirCfg: Configuration has not been changed; status: ",status);
		}

		// 5. send response to client
		pRequest->SetConfirmObject(new CDummyEntry);
		pRequest->SetStatus(status);

		if ((STATUS_OK != status) && (STATUS_IN_PROGRESS != status))
		{
			PTRACE2INT(eLevelInfoNormal,"CLdapModuleManager::HandleSetActiveDirCfg - status BAD=",status);
			///return status;
		}

    }
    else
	{
		PTRACE(eLevelInfoNormal,"CLdapModuleManager::HandleSetActiveDirCfg: No permission to update Active Directory configuration");
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}

	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
void CLdapModuleManager::OnCheckAdServerAvailablilty(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CLdapModuleManager::OnCheckAdServerAvailablilty");

	STATUS status = STATUS_OK;

	if (m_pLdapModuleClientControl)
	{
		// Check connectivity with the new parameters
		BOOL bIsAvailable = (m_pLdapModuleClientControl->CheckAdServerAvailability(m_pLdapModuleCfg));
		if (!bIsAvailable)
			status = STATUS_ACTIVE_DIRECTORY_SERVER_IS_NOT_AVAILABLE | WARNING_MASK;
		string err = m_pLdapModuleClientControl->get_last_error();
		HandleActiveAlarm(bIsAvailable,err);

		if(bIsAvailable)
			CreateNatRuleIfNeeded();
		// Update Monitor task for GET_AD_SERVER_AVAILABILITY_STATUS response
		CProcessBase * process = CProcessBase::GetProcess();
		CSegment *pSeg = new CSegment;
		*pSeg << (DWORD)status;
		CTaskApi LdapModuleMonitorApi(eProcessLdapModule,eMonitor);
		STATUS returnStatus = STATUS_OK;
		CSegment rspMsg;
		OPCODE resOpcode;
		STATUS statusFromLdapModuleProcess = STATUS_OK;

		STATUS responseStatus = LdapModuleMonitorApi.SendMessageSync(pSeg,LDAP_MONITOR_UPDATE_AVAILIBILITY_STATUS,100,resOpcode,rspMsg);

		if (STATUS_OK != responseStatus)
		{
			PTRACE2INT(eLevelInfoNormal,"CLdapModuleManager::OnCheckAdServerAvailablilty: Failed to send Status to LdapModule monitor; status: ",responseStatus);
		}
	}

	TRACEINTO << "CLdapModuleManager::OnCheckAdServerAvailablilty - status: " << status;

}
//////////////////////////////////////////////////////////////////////////////
void   CLdapModuleManager::OnLdapSecurityPKICfg(CSegment* pMsg)
{
//printf("OnLdapSecurityPKICfg  arrived\n");

	CLdapModuleCfg* pCfg = GetLdapModuleCfg();
	pCfg->DeSerializePki(pMsg);
	COstrStream msg;
	pCfg->DumpPKI(msg);
	if (IsActiveAlarmExistByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER)&& (pCfg->getRevocationMethodType() != eOcsp) )
				RemoveActiveAlarmByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER);
//	printf("***  OnLdapSecurityPKICfg **\n %s ",msg.str().c_str());
	PTRACE2(eLevelInfoNormal,"CLdapModuleManager::OnLdapSecurityPKICfg - ",msg.str().c_str());
	if(!m_pLdapModuleClientControl)
		m_pLdapModuleClientControl = new CLdapModuleClientControl(this);
}

/////////////////////////////////////////////////////////////////////////////
void CLdapModuleManager::UpdateLdapCfgParams(CLdapModuleCfg* pNewLdapCfg)
{
	std::string strCfg = pNewLdapCfg->Dump();
	PTRACE2(eLevelInfoNormal,"CLdapModuleManager::UpdateLdapCfgParams - ",strCfg.c_str());

	if(m_pLdapModuleCfg)
	{
		CSegment* pSeg = new CSegment();
		m_pLdapModuleCfg->SerializePki(pSeg);
		pNewLdapCfg->DeSerializePki(pSeg);
		delete pSeg;


		*m_pLdapModuleCfg = *pNewLdapCfg;
		m_pLdapModuleCfg->WriteXmlFile();

		// update the global Ldap module configuration
		::SetLdapModuleCfg(m_pLdapModuleCfg);

		// remove active alarm
		if (IsActiveAlarmExistByErrorCode(AA_INVALID_LDAP_MODULE_CONFIGURATION))
			RemoveActiveAlarmByErrorCode(AA_INVALID_LDAP_MODULE_CONFIGURATION);

		if (IsActiveAlarmExistByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER))
			RemoveActiveAlarmByErrorCode(AA_LDAP_FAILED_TO_CONNECT_TO_OCSP_SERVER);
		// start timer for periodical AD server availability check
		if (m_pLdapModuleCfg->GetEnableDirServices())
			if (m_pLdapModuleClientControl)
				m_pLdapModuleClientControl->StartCheckServerAvailablityTimer();
	}
}


/////////////////////////////////////////////////////////////////////////////
BOOL CLdapModuleManager::IsStartupFinished() const
{
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
	if( eMcuState_Invalid == systemState || eMcuState_Startup == systemState )
		return FALSE;
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
void CLdapModuleManager::OnLdapAddActiveAlarm(CSegment* pMsg)
{
	DWORD errorCode;
	string errDiscription;
	*pMsg >> errorCode
		  >> errDiscription;

	if (!IsActiveAlarmExistByErrorCode(errorCode))
		AddActiveAlarm(FAULT_GENERAL_SUBJECT,
						errorCode,
						MAJOR_ERROR_LEVEL,
						errDiscription,
						true,		//isForEma
						true);		//inForFaults
	else//update
		UpdateActiveAlarmDescriptionByErrorCode(errorCode, errDiscription);
}

/////////////////////////////////////////////////////////////////////////////
DWORD CLdapModuleManager::ConvertExtDBStatus(DWORD status)
{
	DWORD apiStatus = 0;
	switch(status)
	{
		case(EXT_APP_STATUS_OK):
		{
			apiStatus = STATUS_EXT_APP_STATUS_OK;
			break;
		}
		case(EXT_APP_ILLEGAL_MCU_USER_NAME_OR_PASSWORD):
		{
			apiStatus = STATUS_EXT_APP_ILLEGAL_MCU_USER_NAME_OR_PASSWORD;
			break;
		}
		case(EXT_APP_REQUEST_TIMEOUT):
		{
			apiStatus = STATUS_EXT_APP_REQUEST_TIMEOUT;
			break;
		}
		case(EXT_APP_ILLEGAL_NUMERIC_ID):
		{
			apiStatus = STATUS_EXT_APP_ILLEGAL_NUMERIC_ID;
			break;
		}
		case(EXT_APP_ILLEGAL_CLI):
		{
			apiStatus = STATUS_EXT_APP_ILLEGAL_CLI;
			break;
		}
		case(EXT_APP_ILLEGAL_CLI_OR_NUMERIC_ID):
		{
			apiStatus = STATUS_EXT_APP_ILLEGAL_CLI_OR_NUMERIC_ID;
			break;
		}
		case(EXT_APP_INTERNAL_ERROR):
		{
			apiStatus = STATUS_EXT_APP_INTERNAL_ERROR;
			break;
		}
		case(EXT_APP_ILLEGAL_USER_ID):
		{
			apiStatus = STATUS_EXT_APP_ILLEGAL_USER_ID;
			break;
		}
		case(EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD):
		{
			apiStatus = STATUS_EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD;
			break;
		}
		default:
			apiStatus = status;
	}
	return apiStatus;
}
/////////////////////////////////////////////////////////////////////
// test function
STATUS CLdapModuleManager::HandleSecureConnection(CSegment * seg,std::ostream& answer)
{
	if (m_pLdapModuleClientControl)
	{
		answer << "call ldap client control";
		m_pLdapModuleClientControl->SecureConnectionTest(answer);
	}
	else
		answer << "m_pLdapModuleClientControl is null";

	return STATUS_OK;
}

STATUS CLdapModuleManager::HandleSetLdapNormalMode(CSegment * seg,std::ostream& answer)
{
	if (m_pLdapModuleClientControl)
		{
			m_pLdapModuleClientControl->SetLdapNormalMode(answer);
		}
		else
			answer << "m_pLdapModuleClientControl is null";
	return STATUS_OK;
}
STATUS CLdapModuleManager::HandleSetLdapDebugMode(CSegment * seg,std::ostream& answer)
{
	if (m_pLdapModuleClientControl)
		{
			m_pLdapModuleClientControl->SetLdapDebugMode(answer);
		}
		else
			answer << "m_pLdapModuleClientControl is null";
	return STATUS_OK;
}
STATUS CLdapModuleManager::HandleMultipleOUs(CSegment * seg,std::ostream& answer)
{
	if (m_pLdapModuleClientControl)
			{
				m_pLdapModuleClientControl->TestMultipleOUs(answer);
			}
			else
				answer << "m_pLdapModuleClientControl is null";
		return STATUS_OK;
}
//////////////////////////////////////////////////////////////////////
//STATUS CLdapModuleManager::HandleTerminalPing(CSegment * seg,std::ostream& answer)
//{
//	PTRACE(eLevelError,"pong to logger");
//	answer << "pong to console";
//	return STATUS_OK;
//}


