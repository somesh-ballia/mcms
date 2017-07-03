// AuthenticationManager.cpp

#include "AuthenticationManager.h"

#include <fstream>
#include <errno.h>
#include <algorithm>

#include "OsFileIF.h"
#include "ConfigManagerOpcodes.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesRanges.h"
#include "GlobalDataAccess.h"
#include "LogInRequest.h"
#include "LogInConfirm.h"
#include "Request.h"
#include "DummyEntry.h"
#include "ChangePassword.h"
#include "NewOperator.h"
#include "OperatorList.h"
#include "TraceStream.h"
#include "ApiStatuses.h"
#include "SysConfig.h"
#include "TaskApi.h"
#include "psosxml.h"
#include "SysConfigKeys.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "InternalProcessStatuses.h"
#include "MplMcmsProtocolTracer.h"
#include "FaultsDefines.h"
#include "IfConfig.h"
#include "ConfigManagerApi.h"
#include "OpcodesMcmsInternal.h"
#include "PostXmlHeader.h"
#include "ObjString.h"
#include "DefinesGeneral.h"
#include "RenameOperator.h"
#include "Operator.h"
#include "FipsMode.h"
#include "AuditorApi.h"
#include "AudibleAlarm.h"
#include "TerminalCommand.h"

extern char* UpdateOperatorTypeToString(int updateType);
extern char* AuthorizationGroupTypeToString(int authorizationGroup);

extern void  AuthenticationMonitorEntryPoint(void* appParam);

// the units are ticks
const WORD TIME_EXT_DB_TIMEOUT					= (REQUEST_TIMEOUT_SHORT / 2);
const WORD AD_AUTHENTICATION_RESPONSE_TIMEOUT	=  REQUEST_TIMEOUT_SHORT - 50; //4.5 sec
const DWORD DAILY_TIMEOUT			= SECOND * 1 * 60 * 60;

#define PASSWORD_FAILURE_LIMIT		3



const std::string CAuthenticationManager::JITC_MODE_FIRST_RUN_FILENAME = MCU_MCMS_DIR+"/JITC_MODE_FIRST_RUN.txt";

#define MAX_TRIES_UPDATE_JITCINDFILE	3
#define DELAY_BETWEEN_TRY_UPDATE_JITECINDFILE	5


BEGIN_TERMINAL_COMMANDS(CAuthenticationManager)
  ONCOMMAND("user_ls", CAuthenticationManager::HandleTerminalUserList, "Type the users list")
  ONCOMMAND("new_user", CAuthenticationManager::HandleTerminalNewOperator, "adding new user to users list")
  ONCOMMAND("enable_user", CAuthenticationManager::HandleTerminalEnableOperator, "enabled a user that was disabled")
  ONCOMMAND("unlock_user", CAuthenticationManager::HandleTerminalUnlockOperator, "enabled a user that was Locked")
END_TERMINAL_COMMANDS

PBEGIN_MESSAGE_MAP(CAuthenticationManager)
  ONEVENT(XML_REQUEST    ,IDLE    ,  CAuthenticationManager::HandlePostRequest)
  ONEVENT(MCUMNGR_AUTHENTICATION_STRUCT_REQ		, ANYCASE, CAuthenticationManager::OnMcuMngrAuthenticationStruct )
  ONEVENT(MCUMNGR_AUTHENTICATION_SUCCESS_REQ	, ANYCASE, CAuthenticationManager::OnMcuMngrAuthenticationSuccess )
  ONEVENT(MCUMNGR_TIME_SET						, ANYCASE, CAuthenticationManager::OnMcuMngrTimeSet)
  ONEVENT(EXT_DB_USER_LOGIN_CONFIRM    			, ANYCASE, CAuthenticationManager::OnExtDBUserLoginConfirm)
  ONEVENT(MCUMNGR_AUT_SET_DEFAULT_USER_LIST		, ANYCASE, CAuthenticationManager::OnSetDefaultUserList)
  ONEVENT(EXT_DB_RESPONSE_TOUT					, ANYCASE  ,CAuthenticationManager::OnTimerNoExtDBResponse)
  ONEVENT(EXT_DB_REQUEST_FAILED					, ANYCASE  ,CAuthenticationManager::OnExtDBFailure)
  ONEVENT(DAILY_TIMER_TOUT						, ANYCASE,  CAuthenticationManager::OnDailyTimerTests)
  ONEVENT(LOCK_TIMER_TOUT						, ANYCASE,  CAuthenticationManager::OnLockTimerTests)
  ONEVENT(MCUMNGR_TO_AUTHENTICATION_MULTIPLE_SERVICES_IND, ANYCASE, CAuthenticationManager::OnMultipleServicesInd)
  ONEVENT(FAILOVER_AUTHENTICATION_CONFIG_REQ,  ANYCASE,	CAuthenticationManager::OnFailoverReq)
  ONEVENT(LDAP_MODULE_AD_SERVER_AVAILABE_IND,  ANYCASE,	CAuthenticationManager::OnLdapModuleAdServerAvailableInd)
  ///ONEVENT(LDAP_AUTHENTICATION_LOGIN_RESPONSE_TIMER,  ANYCASE,	CAuthenticationManager::OnLdapLoginResponseTimeout)
  ///ONEVENT(LDAP_AUTHENTICATION_LOGIN_IND,  ANYCASE,	CAuthenticationManager::OnLdapLoginInd)
  ONEVENT(CHECK_USER_LIST_FOR_LDAP_REQ,	ANYCASE,	CAuthenticationManager::OnCheckUserListForLdapReq)
  // McuMngr sent security mode (when got it from Switch or service was updated)
  ONEVENT(MCUMNGR_SECURITY_MODE_IND             , ANYCASE, CAuthenticationManager::OnMcuMngrSecurityModeInd) 
  ONEVENT(AUTHETICATION_FIRST_JITC_TIMER , ANYCASE, CAuthenticationManager::ChangeFirstJITCModeIndictionToNo)
  ONEVENT(INSTALLER_AUTHETICATION_REMOVE_ENC_OPERATOR_FILE		, ANYCASE, CAuthenticationManager::OnInstallerAuthenticationRemovePasswordFile )  
  ONEVENT(SWITCH_LDAP_LOGIN_REQ , ANYCASE, CAuthenticationManager::OnSwitchLdapLoginRequest)
PEND_MESSAGE_MAP(CAuthenticationManager,CManagerTask);

BEGIN_SET_TRANSACTION_FACTORY(CAuthenticationManager)
  ON_TRANS("TRANS_MCU","LOGIN",CLogInRequest,CAuthenticationManager::HandleOperLogin)
  ON_TRANS("TRANS_OPERATOR","CHANGE_PASSWORD",CChangePassword,CAuthenticationManager::HandleChangePassword)
  ON_TRANS("TRANS_OPERATOR","DELETE_OPERATOR",COperator,CAuthenticationManager::HandleDeleteOperator)
  ON_TRANS("TRANS_OPERATOR","DISABLE_OPERATOR",COperator,CAuthenticationManager::HandleDisableOperator)
  ON_TRANS("TRANS_OPERATOR","NEW_OPERATOR",CNewOperator,CAuthenticationManager::HandleNewOperator)
  ON_TRANS("TRANS_OPERATOR","RENAME_OPERATOR",CRenameOperator,CAuthenticationManager::HandleRenameOperator)
  ON_TRANS("TRANS_OPERATOR","UNLOCK_OPERATOR",COperator,CAuthenticationManager::HandleUnlockOperator)
  ON_TRANS("TRANS_OPERATOR","SET_OPERATOR_AUDIBLE_ALARM",CAudibleAlarm,CAuthenticationManager::SetOperatorAudibleALarm)
END_TRANSACTION_FACTORY

void AuthenticationManagerEntryPoint(void* appParam)
{
	CAuthenticationManager * pAuthenticationManager = new CAuthenticationManager;
	pAuthenticationManager->Create(*(CSegment*)appParam);
}

TaskEntryPoint CAuthenticationManager::GetMonitorEntryPoint()
{
	return AuthenticationMonitorEntryPoint;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAuthenticationManager::CAuthenticationManager()
{
	m_pProcess = (CAuthenticationProcess*)CAuthenticationProcess::GetProcess();

	m_isAuthenticationStructAlreadyReceived = NO;
	m_pRequestQueueList = NULL;
	m_operName = NULL;
	m_bIsAdAvailable = FALSE;

	m_bSystemMultipleServices = TRUE;
	m_bV35JITCSupport = FALSE;
	m_pszClientCertificateCN = NULL;
	m_bSystemSecurityMode = NO;
	m_bRequestPeerCertificate = YES;
}

CAuthenticationManager::~CAuthenticationManager()
{
	POBJDELETE(m_pRequestQueueList);
	if(m_operName != NULL)
	{
		delete [] m_operName;
		m_operName = NULL;
	}
	PDELETEA(m_pszClientCertificateCN);
}

// If system startup is first time in JITC, mark all accounts as new to force pwd change
// In case of error returns TRUE
BOOL CAuthenticationManager::CheckFirstJITCModeStartup()
{	
  if (!IsFederalOn())
  {
    return FALSE;
  }

  // static std::string fname = MCU_MCMS_DIR+"/JITC_MODE_FIRST_RUN.txt";
  TRACECOND_AND_RETURN_VALUE(!IsFileExists(JITC_MODE_FIRST_RUN_FILENAME),
      "IsFileExists: " << JITC_MODE_FIRST_RUN_FILENAME << ": " << strerror(errno) << " (" << errno << ")",
      TRUE);

  std::string ans;
  STATUS status = ReadFileToString(JITC_MODE_FIRST_RUN_FILENAME.c_str(), 5, ans);
  if (STATUS_OK != status)
    return TRUE;

  TRACEINTOFUNC << JITC_MODE_FIRST_RUN_FILENAME << ": " << ans;

  if (ans.find("YES") == std::string::npos)
  {	  
	  return FALSE;
  }
	  
  StartTimer(AUTHETICATION_FIRST_JITC_TIMER, SECOND*DELAY_BETWEEN_TRY_UPDATE_JITECINDFILE);  
  return TRUE;
 }


void CAuthenticationManager::ChangeFirstJITCModeIndictionToNo()
{	
	static DWORD triesNumber = 0;	
	++triesNumber;
				
	// Writes configuration file with root permissions
	CSegment* seg = new CSegment;
	*seg << JITC_MODE_FIRST_RUN_FILENAME << "NO";
	
	OPCODE opcode;
	CSegment ret_seg;
	CManagerApi api(eProcessConfigurator);
	STATUS stat = api.SendMessageSync(seg,
									CONFIGURATOR_WRITE_FILE,
									5 * SECOND,
									opcode,
									ret_seg);
	
	if (stat == STATUS_OK && opcode == STATUS_OK)
	{
		triesNumber = 0;
		return;
	}
	
	if (triesNumber < MAX_TRIES_UPDATE_JITCINDFILE)
	{
		TRACEINTOFUNC << "Start timer to update first JITC indication file " << triesNumber;		
		StartTimer(AUTHETICATION_FIRST_JITC_TIMER, SECOND*DELAY_BETWEEN_TRY_UPDATE_JITECINDFILE);
	}
	else 
	{
		triesNumber = 0;		
		CProcessBase* proc = CProcessBase::GetProcess();
		PASSERT_AND_RETURN(NULL == proc);
		
		PASSERTSTREAM(stat != STATUS_OK,
		  "Unable to send " << proc->GetOpcodeAsString(CONFIGURATOR_WRITE_FILE)
		  << " (" << CONFIGURATOR_WRITE_FILE
		  << ") to " << ProcessNames[eProcessConfigurator]
		  << ": " << proc->GetStatusAsString(stat));
		
		PASSERTSTREAM(opcode != STATUS_OK,
		  "Unable to write file " << JITC_MODE_FIRST_RUN_FILENAME
		  << ": " << proc->GetStatusAsString(opcode));		
	}	
}

void CAuthenticationManager::OnDailyTimerTests(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CAuthenticationManager::OnDailyTimerTests");

	BOOL bJitcMode = IsFederalOn();
	BOOL bForcePwdPolicy = CMcmsAuthentication::IsForceStrongPassword();
	if(bJitcMode || bForcePwdPolicy)
	{
		COperatorList* pOperList = GetOperatorList();

		pOperList->CheckAndCancelInactiveAccounts();
		pOperList->CheckExpiredPasswords();
		StartTimer(DAILY_TIMER_TOUT, DAILY_TIMEOUT);
	}
}

void CAuthenticationManager::OnLockTimerTests(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CAuthenticationManager::OnLockTimerTests");
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string data;
	BYTE bEnableUserLockoutFeature;
	sysConfig->GetBOOLDataByKey("USER_LOCKOUT", bEnableUserLockoutFeature);

	if(bEnableUserLockoutFeature==FALSE)
		return;

	COperatorList* pOperList = GetOperatorList();
	pOperList->CheckLockedAccounts();

	StartTimer(LOCK_TIMER_TOUT, SECOND * 60);
}

void CAuthenticationManager::OnMcuMngrAuthenticationStruct(CSegment* pSeg)
{
	m_isAuthenticationStructAlreadyReceived = YES;

	// ===== 1. get the parameters from the structure received into process's attribute
	pSeg->Get( (BYTE*)(m_pProcess->GetAuthenticationStruct()), sizeof(MCMS_AUTHENTICATION_S) );

	// ===== 2. print the data received
	TRACESTR(eLevelInfoNormal) << "\n CCardsManager::OnMcuMngrAuthenticationSuccess "
                                << *(m_pProcess->GetAuthenticationStruct());
}

void CAuthenticationManager::OnMcuMngrAuthenticationSuccess()
{
	
	//yaela TOFIX - these messages are sent to many times to switch on loading 
	
	FTRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::OnMcuMngrAuthenticationSuccess";

	// send SuperUsers list to MPL
	SendSuperUsersListToMplApi();
	
		
	InformSwitchOnLdapConfiguration();
	
}

void CAuthenticationManager::UpdateHotBackupCurrentType(CLogInRequest* pLoginRequest, CLogInConfirm* pLoginConfirm)
{
	eMasterSlaveState ePairCurrentState = (eMasterSlaveState)pLoginRequest->GetMasterSlaveCurrentState();
	FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::UpdateHotBackupCurrentType: " << ePairCurrentState;
	if (ePairCurrentState != eMasterSlaveNone)
	{
		CManagerApi failoverMngrApi(eProcessFailover);
		CSegment rspMsg;
		OPCODE resOpcode;
		WORD eCurrentState;
		CSegment *pSeg =new CSegment();
		*pSeg << pLoginRequest->GetMasterSlaveCurrentState();
		*pSeg << pLoginRequest->GetClientIP();
		STATUS responseStatus  = failoverMngrApi.SendMessageSync(pSeg, FAILOVER_LOGIN_ARRIVED, 1 * SECOND,resOpcode, rspMsg);
		if (responseStatus == STATUS_OK)
		{
			rspMsg >> eCurrentState;
			pLoginConfirm->SetMasterSlaveState(eCurrentState);
		}
	}
}

void CAuthenticationManager::OnMcuMngrTimeSet()
{
	FTRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::OnMcuMngrTimeSet";

	//Force DAILY_TIMER_TOUT to pop immediately
	DeleteTimer(DAILY_TIMER_TOUT);
	//10 seconds for the ntp timeout + 1 second slack
	StartTimer(DAILY_TIMER_TOUT, SECOND*11);
}

STATUS CAuthenticationManager::HandleOperLogin(CRequest *pRequest)
{
	CLogInRequest* pLoginRequest = (CLogInRequest*) pRequest->GetRequestObject();
	STATUS status;

	BOOL bJitcMode = IsFederalOn();

	if (pLoginRequest->GetExternalDbAuthorized())	//if already authorized
	{
		if (bJitcMode)
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin - the user is already authorize";
		else
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin - the user "<<pLoginRequest->GetLoginName()<<" is already authorize";

		CLogInConfirm* pLoginConfirm = new CLogInConfirm(m_bSystemMultipleServices, m_bV35JITCSupport);
		UpdateHotBackupCurrentType(pLoginRequest, pLoginConfirm);
		pRequest->SetConfirmObject(pLoginConfirm);

		pRequest->SetStatus(STATUS_OK);
		pLoginConfirm->SetAuthorization(pRequest->GetAuthorization());
		pLoginConfirm->SetIsAudibleAlarmEnable(CheckIfAudibleAlarmEnable(pLoginRequest->GetLoginName()));
		return STATUS_OK;
	}

    // validate user name and password. it should be in US-ASCII
    string sLoginName = pLoginRequest->GetLoginName();
	string sPassword  = pLoginRequest->GetPassword();
	eStringValidityStatus isLegalAsciiLogin = CObjString::IsLegalAsciiString((char*)(sLoginName.c_str()),
                                                                             sLoginName.length(),
                                                                             false,true);
	eStringValidityStatus isLegalAsciiPwd   = CObjString::IsLegalAsciiString((char*)(sPassword.c_str()),
                                                                             sPassword.length(),
                                                                             false,true);
    int asciiStatus = STATUS_OK;
	if (eStringValid != isLegalAsciiLogin)
    {
		asciiStatus = STATUS_ILLEGAL_CHARACTERS_IN_LOGIN_NAME;
		pLoginRequest->SetLoginName(sLoginName);   	//VNGR-26890
    }
	else if (eStringValid != isLegalAsciiPwd)
    {
		asciiStatus = STATUS_ILLEGAL_CHARACTERS_IN_PASSWORD;
		pLoginRequest->SetPassword(sPassword);		//VNGR-26890
    }

    if (STATUS_OK != asciiStatus)
    {
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin: Illegal ASCII characters:\n"
                                << "Status : " << m_pProcess->GetStatusAsString(asciiStatus).c_str()
                                << "\nUser Details : login: " << sLoginName;

        if (bJitcMode)
        	asciiStatus = STATUS_LOGIN_INVALID;
        pRequest->SetStatus(asciiStatus);
        pRequest->SetConfirmObject(new CDummyEntry());

        return STATUS_OK;
    }

    COperator loginOperator;
    loginOperator.SetLogin(pLoginRequest->GetLoginName());
    loginOperator.SetNewPassword(pLoginRequest->GetNewPassword());

    eMasterSlaveState ePairCurrentState = (eMasterSlaveState)pLoginRequest->GetMasterSlaveCurrentState();
	bool bIsHotBackupLogin = (ePairCurrentState != eMasterSlaveNone);
	if (bIsHotBackupLogin)
	{
		loginOperator.SetEncPassword(pLoginRequest->GetPassword());
	}
	else
	{
		loginOperator.SetPassword(pLoginRequest->GetPassword());
	}

	COperatorList* pOperList = GetOperatorList();
	COperator* pOperInDB =  pOperList->GetOperatorByLogin(sLoginName);



	if (bJitcMode && sLoginName=="SUPPORT")
	{
        SetAuditMoreInfo("Failed - SUPPORT user name is not allowed in JITC mode.");
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin: the new operator name "<< sLoginName <<" is not allowed";
		pRequest->SetStatus(STATUS_SUPPORT_USER_NAME_IS_NOT_ALLOWED_JITC);
        return STATUS_OK;
	}



	bool isSHA1Password = FALSE;

  if (pOperInDB)
  {
    TRACEINTOFUNC << "Looks for the user in the internal DB";
    CLogInConfirm* pLoginConfirm = new CLogInConfirm(m_bSystemMultipleServices,
                                                     m_bV35JITCSupport);
    UpdateHotBackupCurrentType(pLoginRequest, pLoginConfirm);
    pLoginConfirm->SetIsAudibleAlarmEnable(CheckIfAudibleAlarmEnable(pLoginRequest->GetLoginName()));
    pRequest->SetConfirmObject(pLoginConfirm);

    // if certificate CN empty in Machine Account user
    if (m_bRequestPeerCertificate && pOperInDB->IsMachineAccount() && NULL == m_pszClientCertificateCN)
    {
      status = STATUS_LOGIN_INVALID;
      TRACEINTOFUNC << "Client Certificate CN is empty, FQDN is "
                    << pOperInDB->GetFqdnName();
    }
    // if certificate CN not equal FQDN name in Machine Account user
    else if (m_bRequestPeerCertificate && pOperInDB->IsMachineAccount() &&
             0 != strcasecmp(pOperInDB->GetFqdnName().c_str(),
                             m_pszClientCertificateCN))
    {
      status = STATUS_LOGIN_INVALID;
      TRACEINTOFUNC << "FQDN name is not equal to CN: "
                    << pOperInDB->GetFqdnName() << " != " << m_pszClientCertificateCN;

      // audit event
      SendEventToAuditor(pLoginRequest->GetLoginName(),
                         pLoginRequest->GetStationName(),
                         "Certificate of Machine Account user doesn't match to FQDN name",
                         "Login");
    }
		else if (pOperList->CheckUserDisabled(sLoginName))
		{
			status = STATUS_ACCOUNT_DISABLED;
		}
		else if (pOperList->CheckUserLocked(sLoginName))
		{
			status = STATUS_ACCOUNT_LOCKED;
		}
		else
		{
			if (bIsHotBackupLogin == false)
			{
				loginOperator.EncryptPassword();
				status = pOperList->CheckPassword(&loginOperator);
				if (STATUS_OK != status)
				{
					//we will try to check if the password has been enc in SHA1
					loginOperator.SetPassword(pLoginRequest->GetPassword());
					loginOperator.EncryptPasswordSHA1();
					status = pOperList->CheckPassword(&loginOperator);
					if (STATUS_OK == status)
					{
						//the password has been encrypted in SHA1 in that case we should encrypt it in sha256 BRIDGE-3727 .
						isSHA1Password = TRUE;
						//pOperInDB->SetForceChangePwd(TRUE);
					}

				}
			}
			else
			 status = pOperList->CheckPassword(&loginOperator);

			if (STATUS_OK == status)
			{
				FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin - user name and password are ok";
				// ******* New Account - must replace password on first login ***********

                if ((bJitcMode && pOperInDB->IsForceChangePwd()) || (isSHA1Password == TRUE) )
				{
					std::string sNewPassword  = loginOperator.GetNewPassword();
					FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin new Password.";
					if (("" == sNewPassword ) && (isSHA1Password == FALSE) )
					{
						DWORD cgfPwdFailureLimit = PASSWORD_FAILURE_LIMIT;
						CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
						sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_FAILURE_LIMIT, cgfPwdFailureLimit);

						if(pOperList->GetCountForceChangePwdNotifications(sLoginName) < cgfPwdFailureLimit)
						{
							pOperList->IncrementCountForceChangePwdNotifications(sLoginName);
							status = STATUS_FORCE_CHANGE_PASSWORD;
						}
						else
						{
                            BYTE bEnableUserLockoutFeature;
                            sysConfig->GetBOOLDataByKey("USER_LOCKOUT", bEnableUserLockoutFeature);

							//Disable account
							std::string stDescription = "User account: " + pOperInDB->GetLogin() + "  was disabled (locked) due to 3 change pwd password failures.";
							if( TRUE == bEnableUserLockoutFeature  &&  TRUE == DisableAccount(pOperInDB->GetLogin(), stDescription) )
							{
								pOperList->ResetCountForceChangePwdNotifications(sLoginName);
								status = STATUS_ACCOUNT_DISABLED;
							}
							else // can't disable last admin account
							{
								pOperList->IncrementCountForceChangePwdNotifications(sLoginName);
								status = STATUS_FORCE_CHANGE_PASSWORD;
							}
						}
					}
					//Like in change password / new operator
					else
					{

						// ===== check characters
						CLargeString description;
						CChangePassword ChangePassword;
						if (isSHA1Password == FALSE)

						  ChangePassword.SetNewPassword(sNewPassword);
						else
						  ChangePassword.SetNewPassword(sPassword);

						ChangePassword.SetOldPassword(sPassword);
						ChangePassword.SetLogin(loginOperator.GetLogin());


						if (bJitcMode == TRUE && isSHA1Password == FALSE)
						{
							status = VerifyStrongPassword(ChangePassword, description);
							if(STATUS_OK == status)
								status = CheckChangeFreq(ChangePassword.GetLogin());
						}

						if(STATUS_OK != status)
						{
							pRequest->SetExDescription(description.GetString());
						}
						else
						{
							status = UpdateOperatorPasswordInDB(&ChangePassword);

							// VNGR-10699
							if (STATUS_OK == status)
								SendEventToAuditor(pLoginRequest->GetLoginName(),
									pLoginRequest->GetStationName(),
									"The user password was changed",
									"Login");

							//***********************************************************************************
							// 6.11.12 Migration from SHA1 to SHA256 added by Rachel Cohen
							// for old passwords that have been encrypted in SHA1 and we re encrypt then in SHA256
							// we need to add then to os
							//**********************************************************************************/
							if((bJitcMode == FALSE) && (isSHA1Password == TRUE) && (loginOperator.GetAuthorization() == SUPER))
							{
								loginOperator.SetPasswordOnly(pLoginRequest->GetPassword());
								status = DeleteAdminFromOS(&loginOperator);
								if(STATUS_OK != status)
								{
									FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin: Can't delete operator from OS - status = "
											<< CProcessBase::GetProcess()->GetStatusAsString(status);
								}
								status = AddOperatorToOs(&loginOperator);
								if(STATUS_OK != status)
								{
									pRequest->SetStatus(status);
									FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin: Can't add operator in OS\n"
											<< "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status);
								}
								loginOperator.ClearPassword();
							}


						}
					}
				}
			}
			else
			{
				FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin, CheckPassword failed.";
				CountLoginPasswordFailures(pOperInDB);
			}
		}

		if (STATUS_OK == status)
		{
		    if(!pOperInDB->IsFirstLogin())
			{
				pLoginConfirm->SetLastLogin(pOperInDB->GetLastLogin());
				pLoginConfirm->SetLastLoginIPaddress(pOperInDB->GetLastLoginIPaddress());
				pLoginConfirm->SetLoginHistory(pOperInDB->GetLoginHistory());
			}
			CStructTm curTime;
			SystemGetTime(curTime);
			pOperInDB->SetLastLogin(curTime);
			pOperInDB->SetLastLoginIPaddress(GetCurrentClientAddress());

			pRequest->SetStatus(status);
			pLoginConfirm->SetAuthorization(pOperInDB->GetAuthorization());
			pLoginConfirm->SetMachineAccount(pOperInDB->IsMachineAccount());

			DWORD cgfPwdExpirationPeriod = 0;
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			if( pOperInDB->IsMachineAccount() == FALSE ) // different for Machine Account users
				sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_EXPIRATION_DAYS, cgfPwdExpirationPeriod);
			else
				sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_EXPIRATION_DAYS_MACHINE, cgfPwdExpirationPeriod);

			DWORD daysSinceLastPwdChange = pOperInDB->GetDaysSinceLastPwdChange();
			DWORD daysUntilPwdExpires = 0;
			if(cgfPwdExpirationPeriod > daysSinceLastPwdChange)
				daysUntilPwdExpires = cgfPwdExpirationPeriod - daysSinceLastPwdChange;

			pLoginConfirm->SetDaysUntilPwdExpires(daysUntilPwdExpires);

			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin: Operator login succeeded: "
                                    << loginOperator.GetLogin();

			//password is right, reset 'failure counters'
			if(bJitcMode || CMcmsAuthentication::IsForceStrongPassword())
			{
				pOperList->ResetLoginPwdFailures(pOperInDB->GetLogin());
				pOperList->ResetCountForceChangePwdNotifications(pOperInDB->GetLogin());
			}

			if(bJitcMode)
			    pOperList->ResetLogInHistory(pOperInDB->GetLogin());
		}
		else
		{
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin: Operator login failed: "
                                    << loginOperator.GetLogin()
                                    << " \nStatus=" << m_pProcess->GetStatusAsString(status);

	        if (bJitcMode)
	        	status = CheckIfStatusIsValidInJitc(status);

			CStructTm curTime;
			SystemGetTime(curTime);

			if(status != STATUS_FORCE_CHANGE_PASSWORD)
				pOperList->SetFailedLoginInfo(pOperInDB, curTime, GetCurrentClientAddress());


			pRequest->SetStatus(status);
		}
	}

	else if (m_bIsAdAvailable)//check in the AD
	{
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin - looking for the user in the AD";
		status = HandleLoginReqWithLdapModule(pRequest, loginOperator);
		if (STATUS_OK == status)
		{
			SendEventToAuditor(pLoginRequest->GetLoginName(),
							   pLoginRequest->GetStationName(),
							   "This is an Active Directory user",
							   "Login");
		}
		else
		{
			if (bJitcMode)
				status = CheckIfStatusIsValidInJitc(status);
		}
		pRequest->SetStatus(status);

///		return STATUS_WAIT_FOR_AN_ANSWER_FROM_EXTERNAL_DB;
	}

	else	//check in the external db
	{
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin - looking for the user in the external db";
		BYTE bEnableExternalDBAccess;
		CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		std::string data;
		sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_EXTERNAL_DB_ACCESS, bEnableExternalDBAccess);

		if (bEnableExternalDBAccess)
		{
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin - external db exist";
			char* pStrDBReq = NULL;
			CXMLDOMElement *pRootNode = new CXMLDOMElement("REQUEST_USER_DETAILS");

			int request_id = m_pRequestQueueList->AddRequest(pRequest);

			CreateReqToExternalDB(pRootNode, &loginOperator, request_id);

			pRootNode->DumpDataAsLongStringEx(&pStrDBReq);

			if (pStrDBReq)
			{
				const COsQueue* pQAApiManager = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessQAAPI,eManager);
				DWORD len = strlen (pStrDBReq);
				if (len > 0)
				{
					CSegment *pSeg = new CSegment;

					*pSeg 	<< (WORD)EXT_DB_USER_LOGIN_CONFIRM
							<< len
							<< pStrDBReq;

					const COsQueue* pAuthenticateQueue = CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessAuthentication,eManager);
					pAuthenticateQueue->Serialize(*pSeg);

					CManagerApi api(eProcessQAAPI);
					api.SendMsg(pSeg, SEND_REQ_TO_EXT_DB);

					FTRACESTR(eLevelError) << "CAuthenticationManager::HandleOperLogin - Send command to eProcessQAAPI - user:" <<
                        sLoginName;

					delete []pStrDBReq;

					PDELETE(pRootNode);

					CSegment *pTimerSegment = new CSegment;

					*pTimerSegment << (WORD)request_id;

					StartTimer( EXT_DB_RESPONSE_TOUT, TIME_EXT_DB_TIMEOUT, pTimerSegment );

					return STATUS_WAIT_FOR_AN_ANSWER_FROM_EXTERNAL_DB;
				}
				else
					FTRACESTR(eLevelError) << "CAuthenticationManager::HandleOperLogin - pStrDBReq length = 0 - user:" << sLoginName;
			}
			else
			{
				FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin: Operator login failed: " << sLoginName;
				pRequest->SetStatus(STATUS_LOGIN_INVALID);
			}
			delete []pStrDBReq;
			PDELETE(pRootNode);
		}
		else
		{
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleOperLogin: Operator login failed: " << sLoginName;
			CLogInConfirm* pLoginConfirm = new CLogInConfirm(m_bSystemMultipleServices, m_bV35JITCSupport);
			UpdateHotBackupCurrentType(pLoginRequest, pLoginConfirm);
			pLoginConfirm->SetIsAudibleAlarmEnable(CheckIfAudibleAlarmEnable(pLoginRequest->GetLoginName()));

			pRequest->SetConfirmObject(pLoginConfirm);
			pRequest->SetStatus(STATUS_LOGIN_INVALID);
		}
	}
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::CheckIfStatusIsValidInJitc(STATUS status)
{
	switch (status)
	{
		case STATUS_FORCE_CHANGE_PASSWORD:
		case STATUS_NEW_PASSWORD_WAS_USED_RECENTLY:
		case STATUS_INVALID_STRONG_PASSWORD:
		case STATUS_PASSWORD_LENGTH_IS_TOO_SHORT:
		case STATUS_INVALID_JITC_STRONG_PASSWORD:
		case STATUS_AT_LEAST_4_CHARACTERS_IN_PASSWORD_MUST_BE_CHANGED:
		case STATUS_PASSWORD_CANNOT_BE_CHANGED_YET:
		case STATUS_ACCOUNT_DISABLED:
		case STATUS_ACCOUNT_LOCKED:
			return status;
	}

	return STATUS_LOGIN_INVALID;
}

//////////////////////////////////////////////////////////////////////
//If password does not match - LOCK USER AFTER 3 PASSWORD FAILURES
void CAuthenticationManager::CountLoginPasswordFailures(COperator* pOperator)
{
	BYTE bEnableUserLockoutFeature;
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string data;
	sysConfig->GetBOOLDataByKey("USER_LOCKOUT", bEnableUserLockoutFeature);

	if(bEnableUserLockoutFeature)
	{
		COperatorList* pOperList = GetOperatorList();

		WORD failures = pOperator->GetLoginPwdFailures();

		DWORD cgfPwdFailureLimit = PASSWORD_FAILURE_LIMIT;
		sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_FAILURE_LIMIT, cgfPwdFailureLimit);

		if( failures < cgfPwdFailureLimit)
		{
			CStructTm curTime;
			SystemGetTime(curTime);

			//If first failure, mark time
			if( 0 == failures )
				pOperList->SetFirstFailureTime(pOperator->GetLogin());
			//If not first failure, check how many hours passed from first failure
			else
			{
				DWORD timeframe = 0;
				sysConfig->GetDWORDDataByKey(CFG_KEY_USER_LOCKOUT_WINDOW_IN_MINUTES, timeframe);
				if(timeframe)
				{
					CStructTm firstFailureTm = pOperator->GetFirstFailureTime();

					int theDenominator = 60/*minutes*/ ;

					DWORD diffTime	= curTime - firstFailureTm ;
					DWORD passedHours = diffTime / theDenominator;

					//If passed more hours than required, and user is not locked yet, reset count
					if(passedHours >= timeframe && timeframe != 45000 )
					{
						pOperList->ResetLoginPwdFailures(pOperator->GetLogin());
						pOperList->SetFirstFailureTime(pOperator->GetLogin());
					}
				}
			}
			pOperList->IncrementLoginPwdFailures(pOperator->GetLogin());
		}

		if(cgfPwdFailureLimit == pOperList->GetLoginPwdFailures(pOperator->GetLogin()))
		{
			//Lock account
			std::string stDescription = "User account: " + pOperator->GetLogin() + "  was locked due to 3 login password failures.";
			LockAccount(pOperator->GetLogin(), stDescription);
		}
	}
}

//////////////////////////////////////////////////////////////////////
//If password does not match - DISABLE USER AFTER 3 PASSWORD FAILURES
void CAuthenticationManager::CountChangePasswordFailures(COperator* pOperator)
{
	BYTE bEnableUserLockoutFeature;
	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string data;
	sysConfig->GetBOOLDataByKey("USER_LOCKOUT", bEnableUserLockoutFeature);

	if(bEnableUserLockoutFeature)
	{
		COperatorList* pOperList = GetOperatorList();

		DWORD cgfPwdFailureLimit = PASSWORD_FAILURE_LIMIT;
		sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_FAILURE_LIMIT, cgfPwdFailureLimit);

		WORD failures = pOperator->GetChangePwdFailures();
		if( failures < cgfPwdFailureLimit)
			pOperList->IncrementChangePwdFailures(pOperator->GetLogin());

		if(cgfPwdFailureLimit == pOperList->GetChangePwdFailures(pOperator->GetLogin()))
		{
			//Disable account
			std::string stDescription = "User account: " + pOperator->GetLogin() + "  was locked due to 3 change pwd password failures.";
			LockAccount(pOperator->GetLogin(), stDescription);
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CAuthenticationManager::CreateReqToExternalDB(CXMLDOMElement *pRootNode, COperator* pOperator, int request_id)
{
	CXMLDOMElement *pTempNode = pRootNode->AddChildNode("LOGIN");

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string data;

    int status;
	pTempNode->AddChildNode("MCU_IP", "");

	// get login string
	char login[100] = "";
	sysConfig->GetDataByKey(CFG_KEY_EXTERNAL_DB_LOGIN, data);
	strncpy( login, data.c_str(), 99);
	if (0 < strlen( login ))
		pTempNode->AddChildNode("USER_NAME", login);
	else
		pTempNode->AddChildNode("USER_NAME", "POLYCOM");
	data = "";

	// get pw string
	char pw[100] = "";
	sysConfig->GetDataByKey(CFG_KEY_EXTERNAL_DB_PASSWORD, data);
	strncpy( pw, data.c_str(), 99);
	if (0 < strlen( pw ))
		pTempNode->AddChildNode("PASSWORD", pw);
	else
		pTempNode->AddChildNode("PASSWORD", "POLYCOM");

	// requestID
	pRootNode->AddChildNode("TOKEN", request_id);	// be sent to the external-DB and return as is for internal usage

	pTempNode = pRootNode->AddChildNode("ACTION");
	pTempNode = pTempNode->AddChildNode("AUTHENTICATE");

	if(pOperator->GetLogin().empty())
		pTempNode->AddChildNode("USER_NAME", "");
	else
		pTempNode->AddChildNode("USER_NAME", pOperator->GetLogin());

	if(pOperator->GetPassword().empty())
		pTempNode->AddChildNode("PASSWORD", "");
	else
		pTempNode->AddChildNode("PASSWORD", pOperator->GetPassword());
}

//////////////////////////////////////////////////////////////////////
void CAuthenticationManager::OnTimerNoExtDBResponse(CSegment* pParam )
{
	FTRACESTR(eLevelError) << "CAuthenticationManager::OnTimerNoExtDBResponse";

	WORD token =0 ;
	*pParam >> token;

	CRequestQueue* pRequestQueue = m_pRequestQueueList->GetRequestQueue(token);
	if(NULL == pRequestQueue)
	{
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnTimerNoExtDBResponse - NULL == pRequestQueue ";
		return;
	}

	SendResponse(EXT_APP_REQUEST_TIMEOUT, pRequestQueue->GetRequest(), pRequestQueue->GetQueue());

	m_pRequestQueueList->RemoveRequest(token);
}

//////////////////////////////////////////////////////////////////////
void CAuthenticationManager::OnExtDBFailure(CSegment* pParam)
{
	FTRACESTR(eLevelError) << "CAuthenticationManager::OnExtDBFailure, EXT_DB_REQUEST_FAILED";

    DeleteTimer(EXT_DB_RESPONSE_TOUT);

	DWORD XMLStringLen = 0;
	WORD   RetOpCode;
	*pParam >> RetOpCode;
	*pParam >> XMLStringLen;

	ALLOCBUFFER(pXMLString, XMLStringLen+1);
	*pParam >> pXMLString;
	pXMLString[XMLStringLen]='\0';

	int token=0;

	CXMLDOMDocument *pDom=new CXMLDOMDocument;
	if (pDom->Parse((const char **)&pXMLString)==SEC_OK)
	{
		DEALLOCBUFFER(pXMLString);
		CXMLDOMElement *pRoot=pDom->GetRootElement();

		pRoot->getChildNodeDecValueByName("TOKEN",&token);

		CRequestQueue* pRequestQueue = m_pRequestQueueList->GetRequestQueue(token);
		if(NULL == pRequestQueue)
		{
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnExtDBFailure - NULL == pRequestQueue ";
			POBJDELETE(pDom);
			return;
		}

		SendResponse(EXT_APP_INTERNAL_ERROR, pRequestQueue->GetRequest(), pRequestQueue->GetQueue());

		m_pRequestQueueList->RemoveRequest(token);
	}
	else
	{
//		DEALLOCBUFFER(pXMLString); // Deallocate the buffer in one of macros fails
		PASSERT(100);
	}
	POBJDELETE(pDom);
}

//////////////////////////////////////////////////////////////////////
void CAuthenticationManager::SendResponse(int status, CRequest* req, COsQueue* q)
{
	CLogInConfirm* pLoginConfirm = new CLogInConfirm(m_bSystemMultipleServices, m_bV35JITCSupport);
	req->SetConfirmObject(pLoginConfirm);
	req->SetStatus(status);
	CXMLDOMElement *pXmlStream = req->SetConfirmSerializeXml();

	char *respBuff   = NULL;
	DWORD respBuffLen= 0;

	pXmlStream->DumpDataAsLongStringEx(&respBuff);

	PostXml(respBuff, strlen(respBuff), *q);

	PDELETEA(respBuff);
	POBJDELETE(pXmlStream);
}

//////////////////////////////////////////////////////////////////////
int  CAuthenticationManager::OnExtDBUserLoginConfirm(CSegment* pParam)
{
	FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnExtDBUserLoginConfirm";

    DeleteTimer(EXT_DB_RESPONSE_TOUT);

	DWORD XMLStringLen = 0;
	CXMLDOMElement *pNode=NULL, *pAction =NULL;
	DWORD id = 0xFFFFFFFF;
	int status = STATUS_OK, nStatus = SEC_OK;

	char* pszError = new char[ERROR_MESSAGE_LEN];
	pszError[0] = '\0';
	WORD group=GUEST; // Need to get the authorization from the response
	int token;

	*pParam >> XMLStringLen;

	ALLOCBUFFER(pXMLString, XMLStringLen+1);
	*pParam >> pXMLString;
	pXMLString[XMLStringLen]='\0';

	CRequestQueue* pRequestQueue = NULL;

	CXMLDOMDocument *pDom=new CXMLDOMDocument;
	if (pDom->Parse((const char **)&pXMLString)==SEC_OK)
	{
		CXMLDOMElement *pRoot=pDom->GetRootElement();

		// Parse the DB response and get the user details
		GET_CHILD_NODE(pRoot, "RETURN_STATUS", pNode);

		if(pNode) 
		{
			nStatus=pNode->GetAndVerifyChildNodeValue("ID",&id,pszError,_0_TO_DWORD);

			if(nStatus!=STATUS_OK&&nStatus!=STATUS_ENUM_VALUE_INVALID && nStatus!=STATUS_IP_ADDRESS_INVALID)
			{
				DEALLOCBUFFER(pXMLString); // Deallocate the buffer in one of macros fails		
				PDELETE(pDom);
				delete[] pszError;

				return nStatus;
			}
		}
		else
		{
			DEALLOCBUFFER(pXMLString); // Deallocate the buffer in one of macros fails		
			PDELETE(pDom);
			delete[] pszError;
			
			return STATUS_NODE_MISSING;
		}

		pRoot->getChildNodeDecValueByName("TOKEN",&token);

		pRequestQueue = m_pRequestQueueList->GetRequestQueue(token);
		if(NULL == pRequestQueue)
		{
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnExtDBUserLoginConfirm - NULL == pRequestQueue ";
			DEALLOCBUFFER(pXMLString); // Deallocate the buffer in one of macros fails
			PDELETE(pDom);
			delete[] pszError;
			return FALSE;
		}

		if(id==EXT_APP_STATUS_OK)
		{
			pNode = NULL;
			nStatus = STATUS_NODE_MISSING;

			GET_CHILD_NODE(pRoot, "ACTION", pAction);

			if(pAction)
			{
				GET_CHILD_NODE(pAction, "AUTHENTICATE", pNode);
			}
			
			if(pNode)
			{
				nStatus=pNode->GetAndVerifyChildNodeValue("AUTHORIZATION_GROUP",&group,pszError,AUTHORIZATION_GROUP_ENUM);
			}

			if (!pNode || (nStatus != STATUS_OK &&
			       nStatus != STATUS_NODE_MISSING &&
			       nStatus != STATUS_ENUM_VALUE_INVALID &&
			       nStatus != STATUS_NODE_LENGTH_TOO_SHORT &&
			       nStatus != STATUS_NODE_LENGTH_TOO_LONG &&
			       nStatus != STATUS_VALUE_OUT_OF_RANGE &&
			       nStatus != STATUS_IP_ADDRESS_INVALID))
			{
				DEALLOCBUFFER(pXMLString); // Deallocate the buffer in one of macros fails		
				PDELETE(pDom);
				delete[] pszError;
				
				return nStatus;
			}
		}
	}
	else
	{
		DEALLOCBUFFER(pXMLString); // Deallocate the buffer in one of macros fails
		PDELETE(pDom);
		PASSERT(100);
		delete[] pszError;
		return FALSE;
	}

	DEALLOCBUFFER(pXMLString);
	PDELETE(pDom);

	//If the client is not authorized
	switch(id)
	{
	case(EXT_APP_STATUS_OK):
		{
			status = STATUS_OK;
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnExtDBUserLoginConfirm - DB replied - OK. ";
			break;
		}
	case(EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD):
		{
			status = STATUS_LOGIN_INVALID;
			FTRACESTR(eLevelError) << "CAuthenticationManager::OnExtDBUserLoginConfirm - Illegal user name or password";
			break;
		}
	case(EXT_APP_INTERNAL_ERROR):
		{
			status = EXT_APP_INTERNAL_ERROR;
			FTRACESTR(eLevelError) << "CAuthenticationManager::OnExtDBUserLoginConfirm - Internal Error in DB";
			break;
		}
	default:
		{
			status = STATUS_LOGIN_INVALID;
			FTRACESTR(eLevelError) << "CAuthenticationManager::OnExtDBUserLoginConfirm - Failed to login, External DB status: " << id;
			break;
		}
	}

	char* buffer = NULL;
	DWORD len = 0;

	CXMLDOMElement* pXmlStream = NULL;
	if (status==STATUS_OK)
	{
		CSegment *pSeg = new CSegment;

		CLogInRequest* pLoginRequest = (CLogInRequest*)(pRequestQueue->GetRequest()->GetRequestObject());
		pLoginRequest->SetExternalDbAuthorized(TRUE);

		pXmlStream = (pRequestQueue->GetRequest())->SerializeXml();

		pXmlStream->DumpDataAsLongStringEx(&buffer);

		len = strlen(buffer);

		(pRequestQueue->GetQueue())->Serialize(*pSeg);
		const CPostXmlHeader & currentMsgHdr = GetCurrentMsgHdr();
        CPostXmlHeader postXmlHeader(len,
                                     (WORD)group,
                                     (BYTE)false,
                                     "",
                                     "Authentication WorkStation",
                                     "Authentication UserName",
                                     "Authentication Client Ip",
                                     false, "", "", "", 0,currentMsgHdr.GetConnId());
        postXmlHeader.Serialize(*pSeg);
        pSeg->Put((BYTE*)buffer,len);


		*pSeg << (DWORD) pLoginRequest->GetLoginName().length();
		*pSeg << (char*) pLoginRequest->GetLoginName().c_str();

		m_pRequestQueueList->RemoveRequest(token);

		HandlePostRequest(pSeg);
		DEALLOCBUFFER(buffer);
		POBJDELETE(pSeg);
	}
	else
	{
		CLogInConfirm* pLoginConfirm = new CLogInConfirm(m_bSystemMultipleServices, m_bV35JITCSupport);
		(pRequestQueue->GetRequest())->SetConfirmObject(pLoginConfirm);
		(pRequestQueue->GetRequest())->SetStatus(status);
		pXmlStream = (pRequestQueue->GetRequest())->SetConfirmSerializeXml();

		char *buffer   = NULL;
		pXmlStream->DumpDataAsLongStringEx(&buffer);

		PostXml(buffer,strlen(buffer),*(pRequestQueue->GetQueue()));

		PDELETEA(buffer);
	}
	m_pRequestQueueList->RemoveRequest(token);
	POBJDELETE(pXmlStream);
	delete[] pszError;

    return status;
}
//this function will  add an admin user to database for Rescure purposes when accounts are locked etc..
STATUS CAuthenticationManager::HandleTerminalNewOperator(CTerminalCommand& command, std::ostream& answer)
{
	STATUS status  = STATUS_OK;
    DWORD numOfParams = command.GetNumOfParams();
	if(2 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd new_user Authetication [User Name] [Password]\n";
		return STATUS_FAIL;
	}
	const string &userName = command.GetToken(eCmdParam1);
	const string &userPwd = command.GetToken(eCmdParam2);
	CNewOperator newOper(userName,userPwd);
	COperator* pOperator = newOper.GetOperator();
	if (pOperator!=NULL)
	{
		pOperator->SetAuthorization(SUPER);
		status = AddOperatorToOs(pOperator);
		pOperator->EncryptPassword();
		status =  AddOperatorToDB(pOperator);
	}
	return status;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleNewOperator(CRequest *pRequest)
{
    CNewOperator* pNewOperator = (CNewOperator*) pRequest->GetRequestObject();
	COperator* pOperator = pNewOperator->GetOperator();
	COperatorList* pOperList = GetOperatorList();

    FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleNewOperator: New operator: "
                            << pOperator->GetLogin().c_str()
                            << ", authorzation: "
                            <<::AuthorizationGroupTypeToString( pOperator->GetAuthorization() );

	pRequest->SetConfirmObject(new CDummyEntry());
    pRequest->SetStatus(STATUS_OK);

    if(pRequest->GetAuthorization() != SUPER)
    {
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleNewOperator: No permission to add operator";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }

    int status = pOperList->IsValidToAddUser(*pOperator, m_bIsAdAvailable);
    if(STATUS_OK != status)
    {
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleNewOperator: Validation Failed:\n"
                                << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << "\n"
                                << "User Details : " << pOperator->GetLogin().c_str()
                                << ", authorzation: "<< ::AuthorizationGroupTypeToString( pOperator->GetAuthorization());
        pRequest->SetStatus(status);
        return STATUS_OK;
    }

    BOOL bJitcMode = IsFederalOn();

	// ===== check Username/Password validity
    CLargeString description;
	string sLoginName = pOperator->GetLogin();

	status = IsValidUserName(sLoginName, description, bJitcMode);
    if(STATUS_OK != status)
    {
        SetAuditMoreInfo("Invalid user name.");
        pRequest->SetExDescription(description.GetString());
        pRequest->SetStatus(status);
        return STATUS_OK;
    }

    string sPassword  = pOperator->GetPassword();

    eStringValidityStatus isLegalAsciiPwd = CObjString::IsLegalAsciiString((char*)(sPassword.c_str()),
																			sPassword.length(),
                                                                             false,true);

    if (eStringValid != isLegalAsciiPwd)
    {
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleNewOperator: Illegal ASCII characters:\n"
                                << "Status : " << m_pProcess->GetStatusAsString(isLegalAsciiPwd).c_str()
                                << "\nUser Details : login: " << sLoginName;
    	pRequest->SetStatus(STATUS_ILLEGAL_CHARACTERS_IN_PASSWORD);
    	return STATUS_OK;
    }


    BOOL bForcePwdPolicy = CMcmsAuthentication::IsForceStrongPassword();

    if(bJitcMode || bForcePwdPolicy)
    	status = CMcmsAuthentication::IsLegalStrongPassword(pOperator->GetLogin(), sPassword, description);

    if(STATUS_OK != status)
    {
        SetAuditMoreInfo("Strong password policy failure.");
        pRequest->SetExDescription(description.GetString());
        pRequest->SetStatus(status);
        return STATUS_OK;
    }

    bool isAddedToOS = false;
	if(pOperator->GetAuthorization() == SUPER)
    {
        status = AddOperatorToOs(pOperator);
        if(STATUS_OK != status)
        {
            pRequest->SetStatus(status);
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleNewOperator: Can't add operator in OS\n"
                                    << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status);
        }
        else
        {
            isAddedToOS = true;
        }
    }

    pOperator->EncryptPassword();
	pOperator->SetLastLoginIPaddress(GetCurrentClientAddress());

    if(bJitcMode && !pOperator->IsMachineAccount())
    	pOperator->SetForceChangePwd(TRUE);

	bool isAddedToDb = false;
    status = AddOperatorToDB(pOperator);
    if(STATUS_OK != status)
    {
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleNewOperator: Can't add operator to Internal DB\n"
                                << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status) << "\n"
                                << "User Details : "
                                << pOperator->GetLogin().c_str()
                                << " (authorzation: "
                                << ::AuthorizationGroupTypeToString( pOperator->GetAuthorization() )
                                << ")";
        pRequest->SetStatus(status);
        return STATUS_OK;
    }
    else
    {
        isAddedToDb = true;
    }

    if (status == STATUS_OK)
    {

        std::ostringstream msg;

        msg << "New user was created, user name:"
            << pOperator->GetLogin()
            << " authorization level: " << ::AuthorizationGroupTypeToString( pOperator->GetAuthorization() );

        SetAuditMoreInfo(msg.str());
    }

    const char *strIsUpdatedInDB = (isAddedToDb ? "" : "NOT");
    const char *strIsDeleteFromOS = (isAddedToOS ? "" : "NOT");

    FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleNewOperator: Operator was added: "
                            << pOperator->GetLogin().c_str()
                            << " (authorzation: "
                            << ::AuthorizationGroupTypeToString( pOperator->GetAuthorization() )
                            << ")\n"
                            << "Was "
                            << strIsUpdatedInDB
                            << " added to Internal DB\n"
                            << "Was "
                            << strIsDeleteFromOS
                            << " added to OS";

    // if operator is Machine Account and RMX not in security mode - return warning
    if (pOperator->IsMachineAccount() && m_bSystemSecurityMode == NO)
        pRequest->SetStatus(STATUS_OK_WARNING);

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleRenameOperator(CRequest *pRequest)
{
	CRenameOperator* pRenameOperator = (CRenameOperator*) pRequest->GetRequestObject();
	COperator* pOperator = pRenameOperator->GetOperator();
	COperatorList* pOperList = GetOperatorList();

    FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleRenameOperator: Rename operator: "
                            << pOperator->GetOldLogin().c_str();

	pRequest->SetConfirmObject(new CDummyEntry());
	pRequest->SetStatus(STATUS_OK);

	//only admin can rename operator name
    if(pRequest->GetAuthorization() != SUPER)
    {
        SetAuditMoreInfo("Failed - Operation is not authorized");
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleRenameOperator: No permission to rename operator";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }

    BOOL bJitcMode = IsFederalOn();

    COperator* pOldOper = pOperList->GetOperatorByLogin(pOperator->GetOldLogin());

	if(pOldOper == NULL)
	{
        SetAuditMoreInfo("Failed - Login not exists");
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleRenameOperator: old login name "<<pOperator->GetOldLogin()<<" doesn't exist";
		pRequest->SetStatus(STATUS_LOGIN_NOT_EXISTS);
        return STATUS_OK;
	}

    // ===== check Username/Password validity
    CLargeString description;
	string sLoginName = pOperator->GetNewLogin();

	int status = IsValidUserName(sLoginName, description, bJitcMode);
    if(STATUS_OK != status)
    {
        SetAuditMoreInfo("Invalid user name.");
        pRequest->SetExDescription(description.GetString());
        pRequest->SetStatus(status);
        return STATUS_OK;
    }

    eStringValidityStatus isLegalAsciiLogin = CObjString::IsLegalAsciiString((char*)(pOperator->GetNewLogin().c_str()),
																			 pOperator->GetNewLogin().length(),
                                                                             false,true);

	if (eStringValid != isLegalAsciiLogin)
    {
        SetAuditMoreInfo("Failed - Illegal charecters");
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleRenameOperator: Illegal ASCII characters:\n"
                                << "Status : " << m_pProcess->GetStatusAsString(STATUS_ILLEGAL_CHARACTERS_IN_LOGIN_NAME).c_str()
                                << "\nUser Details : login: " << pOperator->GetNewLogin().c_str()
                                << ", authorzation: "<< ::AuthorizationGroupTypeToString( pOperator->GetAuthorization());
		pRequest->SetStatus(STATUS_ILLEGAL_CHARACTERS_IN_LOGIN_NAME);
		return STATUS_OK;
    }

	pOperator->SetLogin(pOperator->GetOldLogin());

	// ===== check Username validity
	string sNewLoginName = pOperator->GetNewLogin();

	status = IsValidUserName(sNewLoginName, description, bJitcMode);
	if(STATUS_OK != status)
	{
		SetAuditMoreInfo("Invalid user name.");
		pRequest->SetExDescription(description.GetString());
		pRequest->SetStatus(status);
		return STATUS_OK;
	}

	if (bJitcMode && sNewLoginName=="SUPPORT")
	{
        SetAuditMoreInfo("Failed - SUPPORT user name is not allowed in JITC mode.");
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleRenameOperator: the new operator name "<< sNewLoginName <<" is not allowed";
		pRequest->SetStatus(STATUS_SUPPORT_USER_NAME_IS_NOT_ALLOWED_JITC);
        return STATUS_OK;
	}

	if (pOperList->FindLogin(sNewLoginName.c_str())!=NOT_FOUND)
	{
        SetAuditMoreInfo("Failed - Login name not found.");
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleRenameOperator: the new operator name "<< sNewLoginName <<" already exist";
		pRequest->SetStatus(STATUS_LOGIN_EXISTS);
		return STATUS_OK;
	}

	pOperator->SetAuthorization(pOldOper->GetAuthorization());
	pOperator->SetForceChangePwd(true);
	pOperator->CopyPWDLog(*pOldOper);
	pOperator->SetNewPassword("");
	pOperator->SetLogin(sNewLoginName);

	if (pOldOper->IsEncryptedPassword())
	    pOperator->SetEncPassword(pOldOper->GetPassword());
	else
		pOperator->SetPassword(pOldOper->GetPassword());

	pOperator->SetMachineAccount(pOldOper->IsMachineAccount());
	pOperator->SetFqdnName(pOldOper->GetFqdnName());

	// VNGR-17789: Polycom default User exists reported after
	// default user has been renamed
    if (pOldOper->GetLogin() == COperatorList::GetCustomUserLogin()) {


        //modified for BRIDGE-13484
        BOOL bJitcMode = IsFederalOn();
        if (TRUE == bJitcMode)
        {
            RemoveActiveAlarmByErrorCode(AA_DEFAULT_USER_EXISTS);
        }
        else
        {
            RemoveActiveAlarmFaultOnlyByErrorCode(AA_DEFAULT_USER_EXISTS);
        }
    }

   	if (bJitcMode && pOldOper->GetLogin() == "SUPPORT")
   		RemoveActiveAlarmByErrorCode(AA_SUPPORT_OPERATOR_ILLEGAL_IN_FEDERAL_MODE);

    std::ostringstream msg;
    msg << "Old name: " << pOldOper->GetLogin()
        << " New name: " << sNewLoginName;
    SetAuditMoreInfo(msg.str());
	//remove old operator
	pOperList->Cancel(pOldOper->GetLogin().c_str());

	//add renamed operator

	// VNGR-10627
	// IT 08/06/09
	// Add using AddOperatorToDB to ensure proper Active Alarm is set if renamed to DEFAULT_USER
	// pOperList->Add(*pOperator);
	AddOperatorToDB(pOperator);

	FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleRenameOperator: Rename operator completed ";

	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::IsValidUserName(const string & userName, CObjString & description, BOOL bJitcMode)
{
    // 1) empty name
    if(0 == userName.length())
    {
        description = "Empty user name is illegal";

        return STATUS_ILLEGAL_CHARACTERS_IN_LOGIN_NAME;
    }

    // 2) illegal characters
    const char *pUserName = userName.c_str();
    for(DWORD i = 0 ; i < userName.length() ; i++)
    {
        const char test = pUserName[i];
        if('a' <= test && test <= 'z')
        {
            continue;
        }
        if('A' <= test && test <= 'Z')
        {
            continue;
        }
        if('0' <= test && test <= '9')
        {
            continue;
        }
        if('.' == test ||
           '-' == test ||
           '_' == test)
        {
            continue;
        }

        description = "Illegal character found at index ";
        description << i;
        description << ", the character is ";
        description << test;

        return STATUS_ILLEGAL_CHARACTERS_IN_LOGIN_NAME;
    }

    // 3) reserved names
    static const char * reservedUserNames [] =
        {
            "root"
        };
    for(DWORD i = 0 ; i < sizeof(reservedUserNames) / sizeof(reservedUserNames[0]) ; i++)
    {
        if(userName == reservedUserNames[i])
        {
            description = "The ";
            description << userName;
            description << " is reserved";

            return STATUS_ILLEGAL_CHARACTERS_IN_LOGIN_NAME;
        }
    }

    if (bJitcMode)
    {
    	if (userName == "SUPPORT")
    	{
    		description = "User name SUPPORT is not allowed in JITC mode";
    		return STATUS_SUPPORT_USER_NAME_IS_NOT_ALLOWED_JITC;
    	}
    }

    return STATUS_OK;
}

///////////////////////////////////////////////////////////
// strong password checking
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::VerifyStrongPassword(CChangePassword ChangePassword, CLargeString &StrDescription, BOOL bChangeBySuper)
{
	STATUS status = STATUS_OK;
	BOOL bJitcMode = IsFederalOn();
	BOOL bForcePwdPolicy = CMcmsAuthentication::IsForceStrongPassword();
	if(bJitcMode || bForcePwdPolicy)
	{
		if(!bChangeBySuper)
		status = WasPasswordUsed(ChangePassword.GetLogin(), ChangePassword.GetNewPassword());
		if(STATUS_OK == status)
		{
			if (ChangePassword.GetLogin() == ChangePassword.GetNewPassword())
			{
				TRACEWARN << "User name " << ChangePassword.GetLogin()
				          << " is equal to password";

				status = STATUS_INVALID_STRONG_PASSWORD;
			}
		}

		if (STATUS_OK == status)
		{
			std::string reversedPwd = ChangePassword.GetNewPassword();
			std::reverse(reversedPwd.begin(), reversedPwd.end());
			if (ChangePassword.GetLogin() == reversedPwd)
			{
				TRACEWARN << "Password " << ChangePassword.GetNewPassword()
				          << " is reversed of user name " << ChangePassword.GetLogin();

				status = STATUS_INVALID_STRONG_PASSWORD;
			}
		}

		if (STATUS_OK == status)
		{
			std::string strOldPwd = ChangePassword.GetOldPassword();
			std::string strNewPwd = ChangePassword.GetNewPassword();
			char oldPwd[OPERATOR_NAME_LEN] = "";
			char newPwd[OPERATOR_NAME_LEN] = "";
			memset(oldPwd, '\0', OPERATOR_NAME_LEN);
			memset(newPwd, '\0', OPERATOR_NAME_LEN);

			strncpy(oldPwd, strOldPwd.c_str(), OPERATOR_NAME_LEN - 1);
			strncpy(newPwd, strNewPwd.c_str(), OPERATOR_NAME_LEN - 1);

			status = CMcmsAuthentication::IsLegalStrongPassword(ChangePassword.GetLogin(), newPwd, StrDescription);

			if(STATUS_OK == status)
				if(!bChangeBySuper)
					status = COperatorList::CheckIfAtLeast4CharChanged(oldPwd, newPwd);
		}
	}
	else
	{
		//Verify password is not empty
		if(ChangePassword.GetNewPassword() == "")
		{
			status = STATUS_EMPTY_PASSWORD;
		}
	}

	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::CheckChangeFreq(std::string strLogin)
{
	COperatorList* pOperList = GetOperatorList();
	return pOperList->CheckChangeFreq(strLogin);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::WasPasswordUsed(std::string loginName, std::string newPwd)
{
	COperatorList* pOperList = GetOperatorList();
	if(pOperList->WasPasswordUsed(loginName, newPwd))
		return STATUS_NEW_PASSWORD_WAS_USED_RECENTLY;
	return STATUS_OK;
}


/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleChangePassword(CRequest *pRequest)
{
	CChangePassword* pChangePassword = (CChangePassword*) pRequest->GetRequestObject();
	COperatorList* pOperList = GetOperatorList();

    FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: "
                            << pChangePassword->GetLogin().c_str();

	pRequest->SetConfirmObject(new CDummyEntry());
    pRequest->SetStatus(STATUS_OK);

    //Non-admin users can only change their own passwords while JITC_MODE=NO
    if(pRequest->GetAuthorization() != SUPER &&
       strcmp(pChangePassword->GetLogin().c_str(), m_operName) != 0)
	{
        SetAuditMoreInfo("Failed - Operation is not authorized");
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: No permission to update operator password";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }

    //check if password is changed by SUPER for other user.
	BOOL bSuperUserChangesForOper = FALSE;
	if((pRequest->GetAuthorization() == SUPER) && (0 != strncmp(pChangePassword->GetLogin().c_str(), m_operName, OPERATOR_NAME_LEN)))
			bSuperUserChangesForOper = TRUE;


    int authorization = GUEST;
    int status = STATUS_OK;

    if(!bSuperUserChangesForOper)
    {
    	//Check if account is disabled
		if(pOperList->CheckUserDisabled(pChangePassword->GetLogin()))
		{
			SetAuditMoreInfo("Failed - Account is disabled.");
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: account is disabled.";
			pRequest->SetStatus(STATUS_ACCOUNT_DISABLED);
			return STATUS_OK;
		}

    	//Check if account is locked
    	if(pOperList->CheckUserLocked(pChangePassword->GetLogin()))
		{
			SetAuditMoreInfo("Failed - Account is locked.");
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: account is locked.";
			pRequest->SetStatus(STATUS_ACCOUNT_LOCKED);
			return STATUS_OK;
		}

    	//Check if given password matches old password
		status = pOperList->IsValidToChangePassword(pChangePassword->GetLogin(),pChangePassword->GetOldPassword(),authorization);
		if(STATUS_OK != status)
		{
			SetAuditMoreInfo("Password validation failed");
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: Validation Failed:\n"
                                << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << "\n"
                                << "User Details : " << pChangePassword->GetLogin().c_str();
        pRequest->SetStatus(status);

			//If password does not match - DISABLE USER AFTER 3 PASSWORD CHANGE FAILURES
				COperator* pOperInDB =  pOperList->GetOperatorByLogin(pChangePassword->GetLogin());

				if (pOperInDB != NULL) {
					CountChangePasswordFailures(pOperInDB);
				}
			return STATUS_OK;
		}
			else
				//old password is right, reset 'failure counter'
				if(IsFederalOn() || CMcmsAuthentication::IsForceStrongPassword())
					pOperList->ResetChangePwdFailures(pChangePassword->GetLogin());
	}
	//In JITC, when admin changes password for another user,
	//then 'login name' is the user account, 'old password' is the admin password, 'new password' is the new password for the user
    else
    {
    	if(IsFederalOn())
    	{
    		//Check if given password matches admin password
    		//status = pOperList->IsValidToChangePassword(pOperList->GetLastAdministrator(),pChangePassword->GetOldPassword(),authorization);
    		status = pOperList->IsValidToChangePassword(m_operName,pChangePassword->GetOldPassword(),authorization);
    		if(STATUS_OK != status)
			{
    			status = STATUS_ADMIN_PASSWORD_IS_INCORRECT;
				SetAuditMoreInfo("Admin Password validation failed");
				FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: Validation Failed (Admin pwd does not match):\n"
										<< "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status).c_str() << "\n"
										<< "User Details : " << pChangePassword->GetLogin().c_str();
				pRequest->SetStatus(status);

				return STATUS_OK;
			}
    	}
    }

    // ===== check characters
    CLargeString description;

    status = VerifyStrongPassword(*pChangePassword, description, bSuperUserChangesForOper);
    //Pwd change frequency is not checked if administrator changes his own password
    if(STATUS_OK == status && bSuperUserChangesForOper)
    	status = CheckChangeFreq(pChangePassword->GetLogin());

    if(STATUS_OK != status)
    {
        SetAuditMoreInfo("Strong password policy failure.");
        pRequest->SetExDescription(description.GetString());
        pRequest->SetStatus(status);
        return STATUS_OK;
    }

	eStringValidityStatus isLegalAsciiPwd = CObjString::IsLegalAsciiString((char*)(pChangePassword->GetNewPassword().c_str()),
																			pChangePassword->GetNewPassword().length(),
                                                                               false,true);
    if (eStringValid != isLegalAsciiPwd)
    {
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleNewOperator: Illegal ASCII characters:\n"
								<< "Status : " << m_pProcess->GetStatusAsString(isLegalAsciiPwd).c_str()
                                << "\nUser Details : login: " << pChangePassword->GetLogin().c_str();
      	pRequest->SetStatus(STATUS_ILLEGAL_CHARACTERS_IN_PASSWORD);
      	return STATUS_OK;
     }

    BOOL bForceChangePwd = FALSE;
    if(IsFederalOn() && bSuperUserChangesForOper)
    	bForceChangePwd = TRUE;

    // no force password change for MachineAccount users
    COperator* pOperInDB =  pOperList->GetOperatorByLogin(pChangePassword->GetLogin());
    if( bForceChangePwd )
    {
        if( pOperInDB != NULL && pOperInDB->IsMachineAccount() )
            bForceChangePwd = FALSE;
    }
    bool isUpdateDB = false;
    status = UpdateOperatorPasswordInDB(pChangePassword, bForceChangePwd);
    if(STATUS_OK != status)
    {
        SetAuditMoreInfo("Failed - Internal error.");
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: Can't update password in Internal DB\n"
                                << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status) << "\n"
                                << "User Details : "
                                << pChangePassword->GetLogin().c_str();
        pRequest->SetStatus(status);
        return STATUS_OK;
    }
    else
    {
        std::ostringstream msg;
        msg << "User: " << pChangePassword->GetLogin() << " updated password";
        SetAuditMoreInfo(msg.str());
        isUpdateDB = true;
    }

    bool isUpdateOs = false;
    if(authorization == SUPER)
    {
        status = UpdateOperatorPasswordInOS(pChangePassword);
        if (STATUS_OK != status)
        {
            SetAuditMoreInfo("Failed - Internal error.");
            pRequest->SetStatus(status);
            FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: Can't update operator password in OS\n"
                                    << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status);
        }
        else
        {
            isUpdateOs = true;
        }
    }

    const char *strIsUpdatedInDB = (isUpdateDB ? "" : "NOT");
    const char *strIsDeleteFromOS = (isUpdateOs ? "" : "NOT");

    FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::ChangePassword: Password updated: "
                            << pChangePassword->GetLogin().c_str()
                            << "\n"
                            << "Was "
                            << strIsUpdatedInDB
                            << " updated in Internal DB\n"
                            << "Was "
                            << strIsDeleteFromOS
                            << " deleted from OS";
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleDeleteOperator(CRequest *pRequest)
{
    pRequest->SetConfirmObject(new CDummyEntry());
    pRequest->SetStatus(STATUS_OK);

    if(pRequest->GetAuthorization() != SUPER)
	{
        SetAuditMoreInfo("Failed - Operation is not authorized");
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::DeleteOperator: No permission to delete operator";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }

	COperator* pOperator = (COperator*) pRequest->GetRequestObject();
    STATUS status = DeleteOperator(pOperator, true);
    pRequest->SetStatus(status);
    std::ostringstream msg;
    if (status == STATUS_OK)
    {
        msg << "User: " << pOperator->GetLogin() << " was deleted.";
    }
    else
    {
        msg << "Failed to delete user: " << pOperator->GetLogin();
    }
    SetAuditMoreInfo(msg.str());

	return STATUS_OK;
}
//this function will  enable a sidabled user for Rescue purposes when accounts are disabled etc..
STATUS CAuthenticationManager::HandleTerminalEnableOperator(CTerminalCommand& command, std::ostream& answer)
{
	STATUS status  = STATUS_OK;
    DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd enable_user Authetication [User Name]\n";
		return STATUS_FAIL;
	}
	const string &userName = command.GetToken(eCmdParam1);
	CNewOperator newOper(userName,"");
	COperator* pOperator = newOper.GetOperator();
	if (pOperator!=NULL)
	{
		pOperator->SetDisabled(FALSE);
		status = DisableEnableOperator(pOperator);
	}
	return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleDisableOperator(CRequest *pRequest)
{
    pRequest->SetConfirmObject(new CDummyEntry());
    pRequest->SetStatus(STATUS_OK);

    if(pRequest->GetAuthorization() != SUPER)
	{
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::DisableOperator: No permission to disable operator";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }

	STATUS status = STATUS_OK;
	COperator* pOperator = (COperator*) pRequest->GetRequestObject();
    status = DisableEnableOperator(pOperator);
// ===== send SuperUsers list to MPL
	if (status == STATUS_OK)
	    SendSuperUsersListToMplApi();
    pRequest->SetStatus(status);


	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleTerminalUnlockOperator(CTerminalCommand& command, std::ostream& answer)
{
    DWORD numOfParams = command.GetNumOfParams();
	if(1 != numOfParams)
	{
		answer << "error: Illegal number of parameters\n";
		answer << "usage: Bin/McuCmd unlock_user Authentication [User Name]\n";
		return STATUS_FAIL;
	}
	const string &userName = command.GetToken(eCmdParam1);
	CNewOperator newOper(userName,"");
	COperator* pOperator = newOper.GetOperator();
	if (pOperator!=NULL)
	{
		pOperator->SetLock(FALSE);
		if (UnlockOperator(pOperator)==TRUE) // the user is already locked bail out with STATUS_OK
			return STATUS_OK;                // stil i leave this return in case extra code will be added below
	}
	return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleUnlockOperator(CRequest *pRequest)
{
    pRequest->SetConfirmObject(new CDummyEntry());
    pRequest->SetStatus(STATUS_OK);

    if(pRequest->GetAuthorization() != SUPER)
	{
        FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleUnlockOperator: No permission to unlock operator";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_OK;
    }

	STATUS status = STATUS_OK;
	COperator* pOperator = (COperator*) pRequest->GetRequestObject();


	if (UnlockOperator(pOperator)==TRUE) // the user is already locked bail out with STATUS_OK
		return STATUS_OK;
// ===== send SuperUsers list to MPL
	if (status == STATUS_OK)
	    SendSuperUsersListToMplApi();
    pRequest->SetStatus(status);

	return STATUS_OK;
}

//this function will return TRUE - if the user is already Locked -> we can return back with a reply
//FALSE - if the user in not locked and we need to continoue with the procedure
BOOL CAuthenticationManager::UnlockOperator(COperator *pOperator)
{
	COperatorList* pOperList = GetOperatorList();
	//the same operation on current status will return status OK
	if ( pOperator->IsLocked())
	     return STATUS_OK;
	pOperList->UnlockUser(pOperator->GetLogin());

	// VNGR-11323
	pOperList->ResetLoginPwdFailures(pOperator->GetLogin());
	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::SetOperatorAudibleALarm(CRequest *pRequest)
{
    if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY )
    {
        FPTRACE(eLevelInfoNormal,"CAuthenticationManager::SetOperatorAudibleALarm: No permission to set Operator Audible ALarm for administrator readony");
        pRequest->SetConfirmObject(new CDummyEntry());
        pRequest->SetStatus(STATUS_NO_PERMISSION);
        return STATUS_NO_PERMISSION;
    }
	STATUS status = STATUS_OK;
	COperatorList* pOperList = GetOperatorList();

	CAudibleAlarm* pAudibleAlarm = new CAudibleAlarm();
	*pAudibleAlarm = *(CAudibleAlarm*) pRequest->GetRequestObject();

	string user_name = pAudibleAlarm->GetUserName();
	if (user_name == "")
	{
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_FAIL);
		return STATUS_FAIL;
	}
	COperator* pOperInDB =  pOperList->GetOperatorByLogin(pAudibleAlarm->GetUserName());
	if (pOperInDB != NULL)
	{
		pOperInDB->SetAudibleAlarm(pAudibleAlarm);
	}
	pOperList->SaveToFileDB();

	pRequest->SetConfirmObject(pAudibleAlarm);
	pRequest->SetStatus(status);

	return STATUS_OK;

}

/////////////////////////////////////////////////////////////////////////////
WORD CAuthenticationManager::CheckIfAudibleAlarmEnable(const std::string login_name)
{
	COperatorList* pOperList = GetOperatorList();
	COperator* pOperInDB =  pOperList->GetOperatorByLogin(login_name);
	if (pOperInDB != NULL)
	{
		CAudibleAlarm* pAudibleAlarm = new CAudibleAlarm();
		*pAudibleAlarm = *(pOperInDB->GetAudibleAlarm());
		WORD bIsAudibleAlarmEnable = pAudibleAlarm->GetIsEnabled();
		PDELETE(pAudibleAlarm);
		return bIsAudibleAlarmEnable;
	}
	return FALSE;


}



/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::DeleteOperator(COperator* pOperator, bool isDeleteFromInternalDB)
{
    COperatorList* pOperList = GetOperatorList();

    int authorization = GUEST;
    int status = pOperList->IsValidToDeleteUser(pOperator->GetLogin(), authorization);
	if (STATUS_OK != status)
    {
        FTRACESTR(eLevelInfoNormal)<< "CAuthenticationManager::DeleteOperator: Validation Failed: \n"
                               << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status) << "\n"
                               << "User Details : "
                               << pOperator->GetLogin().c_str()
                               << " (authorzation: "
                               << ::AuthorizationGroupTypeToString( pOperator->GetAuthorization() )
                               << ")";
         return status;
    }

    bool isDeleteFromDB = false;
    if(true == isDeleteFromInternalDB)
    {
        status = DeleteOperatorFromDB(pOperator);
        if(STATUS_OK != status)
        {
            FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::DeleteOperator: Can't delete operator from Internal DB\n"
                                    << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status) << "\n"
                                    << "User Details : "
                                    << pOperator->GetLogin().c_str()
                                    << " (authorzation: "
                                    << ::AuthorizationGroupTypeToString( pOperator->GetAuthorization() )
                                    << ")";
            return status;
        }
        else
        {
            isDeleteFromDB = true;
        }
    }

    bool isDeleteFromOs = false;
    if(SUPER == authorization)
    {
        status = DeleteAdminFromOS(pOperator);
        if(STATUS_OK != status)
        {
            FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::DeleteOperator: Can't delete operator from OS - status = "
                                    << CProcessBase::GetProcess()->GetStatusAsString(status);
        }
        else
        {
            isDeleteFromOs = true;
        }
    }

    // for Ldap, Jitc mode: remove AA AA_INVALID_LOCAL_USER_ACCOUNT_LIST_FOR_LDAP
    if ((isDeleteFromDB) &&	(STATUS_OK == ValidateUserListForLdapJitc()))
    	if (IsActiveAlarmExistByErrorCode(AA_INVALID_LOCAL_USER_ACCOUNT_LIST_FOR_LDAP))
    		RemoveActiveAlarmByErrorCode(AA_INVALID_LOCAL_USER_ACCOUNT_LIST_FOR_LDAP);


    const char *strIsDeleteFromDB = (isDeleteFromDB ? "" : "NOT");
    const char *strIsDeleteFromOS = (isDeleteFromOs ? "" : "NOT");

    FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::DeleteOperator: Operator deleted: "
                            << pOperator->GetLogin().c_str()
                            << " (authorzation: "
                            << ::AuthorizationGroupTypeToString( pOperator->GetAuthorization() )
                            << ")\n"
                            << "Was "
                            << strIsDeleteFromDB
                            << " deleted from Internal DB\n"
                            << "Was "
                            << strIsDeleteFromOS
                            << " deleted from OS";

    std::ostringstream msg;
    msg << "User: " << pOperator->GetLogin() << " is deleted.";
    SetAuditMoreInfo(msg.str());
    return status;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::DisableEnableOperator(COperator* pOperator)
{
	COperatorList* pOperList = GetOperatorList();
	//the same operation on current status will return status OK
	if ( (pOperator->IsDisabled() && pOperList->CheckUserDisabled(pOperator->GetLogin().c_str())) ||
	     (!pOperator->IsDisabled() && !pOperList->CheckUserDisabled(pOperator->GetLogin().c_str())) )
	     return STATUS_OK;

    int authorization = GUEST;
    //the same check for delete user and disable - note: we will not disable the last Enabled admin
    int status = STATUS_OK;
    if (pOperator->IsDisabled() && pOperList->IsValidToDisableUser(pOperator->GetLogin().c_str())!=STATUS_OK )
    {
        FTRACESTR(eLevelInfoNormal)<< "CAuthenticationManager::DisableEnableOperator: Validation Failed: \n"
                               << "Status : " << CProcessBase::GetProcess()->GetStatusAsString(status) << "\n"
                               << "User Details : "
                               << pOperator->GetLogin().c_str()
                               << " (authorzation: "
                               << ::AuthorizationGroupTypeToString( pOperator->GetAuthorization() )
                               << ")";
			 return STATUS_LAST_ENABLED_SUPERVISOR_OPERATOR_IN_MCU;
    }

    std::ostringstream msg;
    msg << "User: " << pOperator->GetLogin();

    if (pOperator->IsDisabled())
    {
        msg << " is disabled.";
        pOperList->DisableUser(pOperator->GetLogin().c_str());
    }
    else
    {
        msg << " is enabled.";
        pOperList->EnableUser(pOperator->GetLogin().c_str());

        if(IsFederalOn() || CMcmsAuthentication::IsForceStrongPassword())
        {
        	pOperList->ResetChangePwdFailures(pOperator->GetLogin());
        	pOperList->ResetLoginPwdFailures(pOperator->GetLogin());
        }
    }

    SetAuditMoreInfo(msg.str());

    FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::DisableEnableOperator: Operator disabled: "
                            << pOperator->GetLogin().c_str()
                            << " (authorzation: "
                            << ::AuthorizationGroupTypeToString( pOperator->GetAuthorization() )
                            << ")\n"
                            << "Was disabled from Internal DB\n";
    return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::DeleteAdminFromOS(COperator* pOperator)
{
    if(TRUE != IsTarget())
    {
        return STATUS_OK;
    }

    CConfigManagerApi api;
	int osStatus = STATUS_OK;
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) // for Call Generator - prevent creating OS users
	{
		osStatus = api.DelAdminUser(pOperator->GetLogin().c_str());
	}
    return osStatus;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::AddOperatorToDB(COperator* pOperator)
{
    COperatorList* pOperList = GetOperatorList();

       // ===== update list
    pOperList->Add(*pOperator);

    //modified for BRIDGE-13484
    if (NULL != pOperator && pOperator->GetLogin() == DEFAULT_USER_LOGIN)
    {
        AddDefaultUserAlertIfNeeded();
    }

    // ===== send SuperUsers list to MPL
    SendSuperUsersListToMplApi();

    return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::AddOperatorToOs(COperator* pOperator)
{
	//JITC Version does not allow adding accounts to ssh
    if(TRUE != IsTarget())
    {
        return STATUS_OK;
    }

    //In JITC Mode, operators are not added as system users
    if(IsFederalOn())
    	return STATUS_OK;

    CConfigManagerApi api;
	int retStatus = STATUS_OK;
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) // for Call Generator - prevent creating OS users
	{
    	retStatus = api.AddAdminUser(pOperator->GetLogin(),pOperator->GetPassword());
    }
    return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::UpdateOperatorPasswordInDB(CChangePassword* pChangePassword, BOOL bForceChangePwd)
{
    COperatorList* pOperList = GetOperatorList();

       // ===== update list
    STATUS status = pOperList->UpdatePassword(pChangePassword->GetLogin(), pChangePassword->GetNewPassword(), bForceChangePwd);

    // ===== send SuperUsers list to MPL
    if(STATUS_OK == status)
    	SendSuperUsersListToMplApi();

    return status;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::UpdateOperatorPasswordInOS(CChangePassword* pChangePassword)
{
    if(TRUE != IsTarget())
    {
        return STATUS_OK;
    }

    CConfigManagerApi api;
	int retStatus = STATUS_OK;
    if (CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) // for Call Generator - prevent creating OS users
	{
    	retStatus = api.ChangePassword(pChangePassword->GetLogin(),pChangePassword->GetNewPassword());
	}
    return retStatus;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::DeleteOperatorFromDB(COperator* pOperator)
{
    COperatorList* pOperList = GetOperatorList();

    // ===== update list
    pOperList->Cancel(pOperator->GetLogin().c_str());
   	if ( false == pOperList->GetIsDefaultUserExists() ) {

   		RemoveActiveAlarmFaultOnlyByErrorCode(AA_DEFAULT_USER_EXISTS);
   	}

    //modified for BRIDGE-13484
    BOOL bJitcMode = IsFederalOn();
    if (TRUE == bJitcMode)
    {
        if (pOperator->GetLogin() == DEFAULT_USER_LOGIN)
        {
            RemoveActiveAlarmByErrorCode(AA_DEFAULT_USER_EXISTS);
        }
    }

	FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::DeleteOperatorFromDB";

   	if (IsFederalOn() && pOperator->GetLogin() == "SUPPORT")
   		RemoveActiveAlarmByErrorCode(AA_SUPPORT_OPERATOR_ILLEGAL_IN_FEDERAL_MODE);

    // ===== send SuperUsers list to MPL
	SendSuperUsersListToMplApi();

    return STATUS_OK;
}
/////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::OnSetDefaultUserList(CSegment* pParam)
{
/* 13.03.07: not touching current list; it will be deleted during startup.
 *           just send 'default list' to ShelfMngr */
	TRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::OnSetDefaultUserList - "
	                              << "\nA list with default user only is sent to ShelManager";

    SendDefaultUserListToMplApi();
}

void CAuthenticationManager::ManagerPostInitActionsPoint()
{
  TestAndEnterFipsMode();

  BYTE bIsFirstJITC = CheckFirstJITCModeStartup();

	CProcessBase *processBase = CProcessBase::GetProcess();
	CAuthenticationProcess *processAut = dynamic_cast<CAuthenticationProcess*>(processBase);

	COperatorList *operatorList = new COperatorList(bIsFirstJITC, FALSE);
	processAut->SetOperatorList(operatorList);

	COperatorList *ldapOperatorList = new COperatorList(bIsFirstJITC, TRUE);
	processAut->SetLdapOperatorList(ldapOperatorList);

	m_pRequestQueueList = new CRequestQueueList();

	// Checks if this startup is after change into JITC_MODE
	COperatorList* pOperList = operatorList;

	if (IsFederalOn())
	{
		if (pOperList && pOperList->IsSupportNameExistInTheOperatorList())
		{
			CAlarmableTask::AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
								AA_SUPPORT_OPERATOR_ILLEGAL_IN_FEDERAL_MODE,
								MAJOR_ERROR_LEVEL,
								"SUPPORT operator is illegal in federal",
								true,
								true
							  );
		}
	}

	StartTimer(DAILY_TIMER_TOUT, SECOND * 5);

	StartTimer(LOCK_TIMER_TOUT, SECOND * 5);

}

/////////////////////////////////////////////////////////////////////
void CAuthenticationManager::DeclareStartupConditions()
{
	CActiveAlarm aa(FAULT_GENERAL_SUBJECT,
					FAILED_CONFIG_USER_LIST_IN_LINUX,
					MAJOR_ERROR_LEVEL,
					"FAILED to configure user list in Linux",
					true,
					true);
 	AddStartupCondition(aa);
}

/////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::ManagerStartupActionsPoint()
{
	// configure user list in linux
	RemoveActiveAlarmByErrorCode(FAILED_CONFIG_USER_LIST_IN_LINUX);

	COperatorList* pList = GetOperatorList();
	if (pList)
	{
		// ===== 1. produce Alert 'USERS_LIST_CORRUPTED' if needed
	  	if ( true == pList->GetIsListCorrupted() && pList->GetIsListDefault() == false )
			  //      if ( true == pList->GetIsListCorrupted() )
		{
			AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
							AA_USERS_LIST_CORRUPTED,
							MAJOR_ERROR_LEVEL,
							"Users list is corrupted; a default user was added, to enable access",
							true,
							true
						  );
		}

		// ===== 2. produce Alert 'DEFAULT_USER_ALERT' if needed
		AddDefaultUserAlertIfNeeded();

	} // end if (pList)

	//LDAP process:
	CManagerApi api(eProcessLdapModule);
	api.SendOpcodeMsg(LDAP_MODULE_AD_SERVER_AVAILABE_REQ);
}

/////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::SendSuperUsersListToMplApi()
{
	if (NO == m_isAuthenticationStructAlreadyReceived)
	{
		FTRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::SendSuperUsersListToMplApi -"
		                               << " MCUMNGR_AUTHENTICATION_STRUCT_REQ has not been received yet!";

		return;
	}


	CLargeString usersListToMplApiStr =
	                        "\nCAuthenticationManager::SendSuperUsersListToMplApi - Users being sent:";
	usersListToMplApiStr << "\n======================================================================\n";


	// ===== 1. prepare the list: convert COperatorList to USERS_LIST_S (for sending to Switch)
	USERS_LIST_S  *pUsersListStruct = new USERS_LIST_S;
	USER_S        *pUserStruct = new USER_S;
	memset( pUsersListStruct, 0, sizeof(USERS_LIST_S) );

	COperatorList *pUsersList  = GetOperatorList();
	COperator     *pUser       = NULL;
	int           idxInMplList = 0;

	// loop over usersList
	// SuperUsers will be inserted to the list (that will be sent to Switch)
	for(int i=0; i<MAX_OPERATORS_IN_MCU; i++)
	{
		// get current user
		pUser = pUsersList->GetOperatorById(i);

		if ( pUser && (SUPER == pUser->GetAuthorization()|| ADMINISTRATOR_READONLY == pUser->GetAuthorization()))
		{
			// clean userStruct
			memset( pUserStruct, 0, sizeof(USER_S) );

			// init current user
			STATUS stat = InitUserStruct(pUserStruct, pUser->GetLogin(), pUser->GetPassword(), pUser->IsEncryptedPassword(), pUser->GetAuthorization());
			if (STATUS_OK == stat)
			{
				// add user to usersList
				memcpy( &(pUsersListStruct->usersList[idxInMplList]),  pUserStruct, sizeof(USER_S) );
			}

			AddUserToUsersListString(usersListToMplApiStr, pUserStruct, idxInMplList);
			idxInMplList++;
		}
	} // end loop over usersList
	FTRACESTR(eLevelInfoNormal) << usersListToMplApiStr.GetString();

	// ===== 2. send
	SendMplApiToSwitch(AUTHENTICATION_USERS_LIST_REQ, (char*) pUsersListStruct, sizeof(USERS_LIST_S), "AUTHENTICATION_USERS_LIST_REQ");
	
	PDELETE(pUserStruct)
	PDELETE(pUsersListStruct)
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::SendDefaultUserListToMplApi()
{
	if (NO == m_isAuthenticationStructAlreadyReceived)
	{
		FTRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::SendDefaultUserListToMplApi -"
		                               << " MCUMNGR_AUTHENTICATION_STRUCT_REQ has not been received yet!";
		return;
	}

	TRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::SendDefaultUserListToMplApi -"
	                              << "\nA list with single, default user is sent (login: " << COperatorList::GetCustomUserLogin() << ")";

	// ===== 1. prepare the 'list' (of a single, default user)
	USERS_LIST_S  *pUsersListStruct = new USERS_LIST_S;
	USER_S        *pUserStruct = new USER_S;
	memset( pUsersListStruct, 0, sizeof(USERS_LIST_S) );
	memset( pUserStruct,      0, sizeof(USER_S) );

	// init user
	STATUS stat = InitUserStruct(pUserStruct, COperatorList::GetCustomUserLogin(), COperatorList::GetCustomUserPasswd(), FALSE, DEFAULT_USER_AUTHORIZATION);
	if (STATUS_OK == stat)
	{
		// add user to usersList
		memcpy( &(pUsersListStruct->usersList[0]), pUserStruct, sizeof(USER_S) );
	}


	// ===== 2. send
	SendMplApiToSwitch(AUTHENTICATION_USERS_LIST_REQ, (char*)pUsersListStruct, sizeof(USERS_LIST_S), "AUTHENTICATION_USERS_LIST_REQ");

	PDELETE(pUserStruct)
	PDELETE(pUsersListStruct)
}


////////////////////////////////////////////////////////////////////////////////////////////////

void CAuthenticationManager::SendMplApiToSwitch(DWORD msgId, char* msg, DWORD msgSize, const string& msgStr)
{
	CMplMcmsProtocol *mplPrtcl= new CMplMcmsProtocol();

	BYTE switchBoardId    = m_pProcess->GetAuthenticationStruct()->switchBoardId,
	     switchSubBoardId = m_pProcess->GetAuthenticationStruct()->switchSubBoardId;

	mplPrtcl->AddCommonHeader(msgId);
	mplPrtcl->AddMessageDescriptionHeader();
	mplPrtcl->AddPhysicalHeader(1, switchBoardId, switchSubBoardId);
	mplPrtcl->AddData( msgSize, (char*)msg );
	CMplMcmsProtocolTracer(*mplPrtcl).TraceMplMcmsProtocol(msgStr.c_str());
	mplPrtcl->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(mplPrtcl);
}


/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::InitUserStruct(USER_S *pUserStruct, string newLogin, string newPwd, BOOL isPwdEncrypted, int newAuthorizationGroup)
{
	int loginLength = newLogin.length(),
	    pwdLength   = newPwd.length();

	// OPERATOR_PWD_LEN api changed to 65 like SHA256_PWD_LEN
	//int pwdLengthLimit = OPERATOR_PWD_LEN-1;
	//if(isPwdEncrypted)
	//	pwdLengthLimit = SHA256_PWD_LEN-1;

	if ( (loginLength > OPERATOR_NAME_LEN-1) || (pwdLength > OPERATOR_PWD_LEN-1) )
	{
		PASSERT(1);
		return STATUS_ILLEGAL;
	}

	pUserStruct->authorizationGroup = newAuthorizationGroup;

	strncpy( ((char*)(pUserStruct->login)),    newLogin.c_str(), loginLength );

	 // 1. not need all call already memset struct 2. strncpy will put 0 after finish copying
	//memset(pUserStruct->password,0,sizeof(pUserStruct->password));

	strncpy( ((char*)(pUserStruct->password)), newPwd.c_str(),   sizeof(pUserStruct->password)-1 );

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::AddUserToUsersListString(CLargeString &usersListToMplApiStr, USER_S *pUserStruct, int i)
{
	char user_name[OPERATOR_NAME_LEN] = "";
	memcpy(user_name, pUserStruct->login, OPERATOR_NAME_LEN-1);
	user_name[OPERATOR_NAME_LEN-1] = '\0';
	usersListToMplApiStr
	      << "User " << i << " - "
	      << "Login Name: " << user_name;

	char* authorizationGroupTypeStr = ::AuthorizationGroupTypeToString(pUserStruct->authorizationGroup);
	if (authorizationGroupTypeStr)
	{
		usersListToMplApiStr << ", Authorization Group: " << authorizationGroupTypeStr << "\n";
	}
	else
	{
		usersListToMplApiStr << ", Authorization Group: (invalid: " << pUserStruct->authorizationGroup << ")\n";
	}
}

/////////////////////////////////////////////////////////////////////////////
////CAUTION: if bAllowDisableLastAdmin == TRUE then one enables to disable EVEN THE LAST ADMIN!!!!
BOOL CAuthenticationManager::DisableAccount(std::string operatorName, std::string description, BOOL bAllowDisableLastAdmin)
{
	//Disable account
	COperatorList* pOperList = GetOperatorList();
	BOOL disabled = pOperList->DisableUser(operatorName, bAllowDisableLastAdmin);
	if( TRUE == disabled )
	{
		pOperList->AuditDisableUser(description);
		return TRUE;
	}
	return FALSE;
}

void CAuthenticationManager::LockAccount(const std::string& operatorName,
                                         const std::string& description)
{
	COperatorList* pOperList = GetOperatorList();
	if (pOperList->LockUser(operatorName))
	{
	  TRACEINTOFUNC << "Description " << description;
		pOperList->AuditLockUser(description);
	}
}

void CAuthenticationManager::ReceiveAdditionalParams(CSegment* pSeg)
{
	DWORD dwLen;

	if(!pSeg)
		return;

	if(m_operName != NULL)
	{
		delete [] m_operName;
		m_operName = NULL;
	}

	if (pSeg->EndOfSegment())
		return;

	*pSeg >> dwLen;
	if(dwLen > 0)
	{
		m_operName = new char[dwLen+1];
		*pSeg >> m_operName;
	}

	if (pSeg->EndOfSegment())
		return;

	// get client certificate subject
	*pSeg >> dwLen;
	if( dwLen )
	{
		bool withBS = true;

		const char CN_PREFIX_WITH_BS[] = "/CN=";
		const char CN_PREFIX_WITHOUT_BS[] = "CN=";

		PDELETEA(m_pszClientCertificateCN);

		char* pszCert = new char[dwLen+1];
		*pSeg >> pszCert;

		char* pStartCN = strstr(pszCert,CN_PREFIX_WITH_BS);
		if( NULL != pStartCN )
		{
			withBS = true;
		}
		else
		{

			pStartCN = strstr(pszCert,CN_PREFIX_WITHOUT_BS);
			if( NULL != pStartCN )
			{
				withBS = false;
			}
		}

		if( NULL != pStartCN )
		{
			char charEnd;
			if (withBS)
			{
				pStartCN += strlen(CN_PREFIX_WITH_BS);
				charEnd = '/';
			}
			else
			{
				pStartCN += strlen(CN_PREFIX_WITHOUT_BS);
				charEnd = ',';
			}
			char* pEndCN = pStartCN;

			while( '\0' != *pEndCN  &&  charEnd != *pEndCN )
			{
				pEndCN++;
			}

			int len = pEndCN - pStartCN;

			m_pszClientCertificateCN = new char[len+1];
			strncpy(m_pszClientCertificateCN,pStartCN,len);
			m_pszClientCertificateCN[len] = '\0';

			TRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::ReceiveAdditionalParams : '/CN=' found in client certificate " << m_pszClientCertificateCN << " withBS : " << (int)withBS;
		}
		else
			TRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::ReceiveAdditionalParams : no '/CN=' found in client certificate;";


		delete [] pszCert;
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::ReceiveAdditionalParams : empty";
	}
}
/////////////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleTerminalUserList(CTerminalCommand & command, std::ostream& answer)
{
	COperatorList* pOperList = GetOperatorList();
	answer << pOperList->WriteUserList().c_str();

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::AddDefaultUserAlertIfNeeded()
{
	COperatorList* pOperList = GetOperatorList();

	if ( pOperList && (true == pOperList->GetIsDefaultUserExists()) )
	{

		BOOL isToProduceAlert = YES;
		CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		sysConfig->GetBOOLDataByKey("DEFAULT_USER_ALERT", isToProduceAlert);


		if (YES == isToProduceAlert)
		{
			BOOL bJitcMode = IsFederalOn();

			if (bJitcMode == TRUE)

				AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
						AA_DEFAULT_USER_EXISTS,
						MAJOR_ERROR_LEVEL,
						"Polycom default User exists. For security reasons, it is recommended to delete this User and create your own User.",
						true,
						true
				);
			else

				AddActiveAlarmFaultOnlySingleton( FAULT_GENERAL_SUBJECT,
						AA_DEFAULT_USER_EXISTS,
						MAJOR_ERROR_LEVEL,
						"Polycom default User exists. For security reasons, it is recommended to delete this User and create your own User." );
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::OnFailoverReq(CSegment* pSeg)
{
	CSegment *pMsg = NULL;

	COperatorList *pUsersList  = GetOperatorList();
	COperator     *pUser       = NULL;
	BYTE bIsFound = FALSE;

	for (int i=0; i<MAX_OPERATORS_IN_MCU; i++)
	{
		pUser = pUsersList->GetOperatorById(i);

		if ( pUser && (SUPER == pUser->GetAuthorization()) )
		{
			TRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnFailoverReq";

			pMsg = new CSegment;
			*pMsg << (DWORD) pUser->GetLogin().length();
			*pMsg << (char*) pUser->GetLogin().c_str();
			*pMsg << (DWORD) pUser->GetPassword().length();
			*pMsg << (char*) pUser->GetPassword().c_str();
			bIsFound = TRUE;
			break;
		}
	}

	if (bIsFound)
	{
		CManagerApi api(eProcessFailover);
		STATUS status = api.SendMsg(pMsg, FAILOVER_AUTHENTICATION_CONFIG_IND);
		if (status != STATUS_OK)
			FPASSERT(FAILOVER_AUTHENTICATION_CONFIG_IND);
	}
}

BOOL CAuthenticationManager::IsFederalOn(void)
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN_VALUE(NULL == proc, FALSE);

	CSysConfig* cfg = proc->GetSysConfig();
	PASSERT_AND_RETURN_VALUE(NULL == cfg, FALSE);

	BOOL val;
	BOOL res = cfg->GetBOOLDataByKey(CFG_KEY_JITC_MODE, val);
	PASSERTSTREAM_AND_RETURN_VALUE(!res,
	    "GetBOOLDataByKey: " << CFG_KEY_JITC_MODE,
	    FALSE);

	return val;
}

void CAuthenticationManager::SendEventToAuditor(const string& userName,
                                                const string& station,
                                                const string& message,
                                                const string& action)
{
	eAuditEventStatus status = eAuditEventStatusOk;

    AUDIT_EVENT_HEADER_S outAuditHdr;
    CAuditorApi::PrepareAuditHeader(outAuditHdr,
                                   userName,
                                   eMcms,
                                   station,
                                   GetCurrentClientAddress(),
                                   eAuditEventTypeInternal,
                                   eAuditEventStatusOk,
                                   action,
                                   message,
                                   "",
                                   "");
    CFreeData freeData;
    CAuditorApi::PrepareFreeData(freeData,
                                 "",
                                 eFreeDataTypeXml,
                                 "",
                                 "",
                                 eFreeDataTypeXml,
                                 "");
    CAuditorApi api;
    api.SendEventMcms(outAuditHdr, freeData);
}

void CAuthenticationManager::OnLdapModuleAdServerAvailableInd(CSegment* pSeg)
{
	WORD bIsAdAvailable;
	*pSeg >> bIsAdAvailable;
	m_bIsAdAvailable = bIsAdAvailable;
	TRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnLdapModuleAdServerAvailableInd - m_bIsAdAvailable = " << (WORD)m_bIsAdAvailable;
	
	InformSwitchOnLdapConfiguration();
}


////////////////////////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::InformSwitchOnLdapConfiguration()
{
	
	TRACEINTO << "\nCAuthenticationManager::InformSwitchOnLdapConfiguration " << (int)m_bIsAdAvailable;

	ACTIVE_DIRECTORY_STATE_S	ldapConfigurationStruct;
	ldapConfigurationStruct.isActiveDirectoryExist = (BYTE)m_bIsAdAvailable;

	SendMplApiToSwitch(ACTIVE_DIRECTORY_STATUS_REQ,(char*)&ldapConfigurationStruct, sizeof(ACTIVE_DIRECTORY_STATE_S), "ACTIVE_DIRECTORY_STATUS_REQ");
}

////////////////////////////////////////////////////
void CAuthenticationManager::OnSwitchLdapLoginRequest(CSegment* pSeg)
{
	const string STATION_NAME = "_INTERNAL_STATTION_NAME";
	TRACEINTO << "CAuthenticationManager::OnSwitchLdapLoginRequest! " ;

	// TODO moran's struct USER_LDAP_REQ_STRUCT
	AD_USER_IND_S* pUserLdapReqStruct = (AD_USER_IND_S*)pSeg->GetPtr();


	CSegment* pLdapReqSeg = new CSegment();
	*pLdapReqSeg << (DWORD)pUserLdapReqStruct->MsgId;		//maybe redundant

	*pLdapReqSeg << (DWORD)strlen((char *)pUserLdapReqStruct->login);
	*pLdapReqSeg << (char*)pUserLdapReqStruct->login;
	*pLdapReqSeg << (DWORD)strlen((char *)pUserLdapReqStruct->password);
	*pLdapReqSeg << (char*)pUserLdapReqStruct->password;

	CManagerApi ldapModuleMngrApi(eProcessLdapModule);
	CSegment rspMsg;
	OPCODE resOpcode = LDAP_AUTHENTICATION_LOGIN_IND;
	STATUS responseStatus  = ldapModuleMngrApi.SendMessageSync(pLdapReqSeg, LDAP_AUTHENTICATION_LOGIN_REQ, AD_AUTHENTICATION_RESPONSE_TIMEOUT, resOpcode, rspMsg);

	if (responseStatus == STATUS_OK)
	{
		DWORD requestId;		//maybe redundant
		rspMsg >> requestId;

		DWORD status;
		rspMsg >> status;

		DWORD authorizationLevel;
		rspMsg >> authorizationLevel;

		status = HandleUpdateLdapOperList (status, (char*)pUserLdapReqStruct->login, authorizationLevel, NULL);
		if ( STATUS_OK == status)
		{
			TRACEINTO << "user Ldap authorized. requestId  " << requestId <<  " authorizationLevel " << authorizationLevel;

			SendUserLdapLoginRespToSwitch( (DWORD)pUserLdapReqStruct->MsgId, (BYTE)TRUE, authorizationLevel);
			SendEventToAuditor((char*)pUserLdapReqStruct->login,
							STATION_NAME,
						   "This is an Active Directory user",
						   "Login");
		}
		else  {
			TRACEINTO << "user not Ldap authorized. requestId  " << requestId ;
			SendUserLdapLoginRespToSwitch( (DWORD)pUserLdapReqStruct->MsgId, (BYTE)FALSE, authorizationLevel);
		}
	}
	else {
		TRACEINTO << "Failed sending LDAP_AUTHENTICATION_LOGIN_REQ ";
		SendUserLdapLoginRespToSwitch((DWORD)pUserLdapReqStruct->MsgId, (BYTE)FALSE, 0);
	}
}


////////////////////////////////////////////////////
void CAuthenticationManager::SendUserLdapLoginRespToSwitch(DWORD requestId, BYTE allowLogin, DWORD authorizationGroup)
{
	TRACEINTO <<  "CAuthenticationManager::SendUserLdapLoginRespToSwitch " << requestId << " allowLogin " << (int)allowLogin;

	AD_USER_RESP_S userLdapLoginResp ;
	memset( &userLdapLoginResp, 0, sizeof(AD_USER_RESP_S) );

	userLdapLoginResp.authorizationGroup = authorizationGroup;
	userLdapLoginResp.IsExistOnAD = allowLogin;
	userLdapLoginResp.MsgId = requestId;

	SendMplApiToSwitch(IS_USER_EXIST_ON_ACTIVE_DIRECTORY_REQ,(char*)&userLdapLoginResp, sizeof(AD_USER_RESP_S), "IS_USER_EXIST_ON_ACTIVE_DIRECTORY_REQ");
}

////////////////////////////////////////////////////
STATUS CAuthenticationManager::HandleUpdateLdapOperList (STATUS status, const string& 	sLoginName, DWORD authorizationLevel, CLogInConfirm* pLoginConfirm)
{
	BOOL bJitcMode = IsFederalOn();

	COperatorList* pLdapOperList = GetLdapOperatorList();
	COperator* pOperInLdapDB = pLdapOperList->GetOperatorByLogin(sLoginName);

	if (!pOperInLdapDB)
	{
	//	TRACEINTO << "add user to Ldap operator list: " << sLoginName << " status: " <<  status;
		// add user to Ldap operator list
		COperator* pOperator = new COperator;
		pOperator->SetLogin(sLoginName);
		pOperator->SetAuthorization(authorizationLevel);

		pLdapOperList->Add(*pOperator);
		pOperInLdapDB = pLdapOperList->GetOperatorByLogin(sLoginName);
		delete pOperator;
	}
	else
	{
	//	TRACEINTO << "user in Ldap operator list: " << sLoginName << " status: " <<  status;
		pOperInLdapDB->SetAuthorization(authorizationLevel);
		if (pLoginConfirm != NULL)
		{
			pLoginConfirm->SetLastLogin(pOperInLdapDB->GetLastLogin());
			pLoginConfirm->SetLastLoginIPaddress(pOperInLdapDB->GetLastLoginIPaddress());
			pLoginConfirm->SetLoginHistory(pOperInLdapDB->GetLoginHistory());
		}
	}

	if (status == STATUS_EXT_APP_STATUS_OK) { // VNGR-19965
		status = STATUS_OK;
	}
	if (status == STATUS_OK)
	{
		CStructTm curTime;
		SystemGetTime(curTime);
		pOperInLdapDB->SetLastLogin(curTime);
		pOperInLdapDB->SetLastLoginIPaddress(GetCurrentClientAddress());
		if(bJitcMode) {
				pLdapOperList->ResetLogInHistory(pOperInLdapDB->GetLogin());
		}
	}
	else
	{
		if (bJitcMode) {
			status = CheckIfStatusIsValidInJitc(status);
		}

		CStructTm curTime;
		SystemGetTime(curTime);
		pLdapOperList->SetFailedLoginInfo(pOperInLdapDB, curTime, GetCurrentClientAddress());

	}
	return status;
}

STATUS CAuthenticationManager::HandleLoginReqWithLdapModule(CRequest *pRequest, COperator& loginOperator)
{
	DWORD request_id = m_pRequestQueueList->AddRequest(pRequest);

	CSegment* pSeg = new CSegment();
	*pSeg << (DWORD)request_id;		//maybe redundant
	*pSeg << (DWORD)loginOperator.GetLogin().length();
	*pSeg << (char*)loginOperator.GetLogin().c_str();
	*pSeg << (DWORD)loginOperator.GetPassword().length();
	*pSeg << (char*)loginOperator.GetPassword().c_str();

	CManagerApi ldapModuleMngrApi(eProcessLdapModule);
	CSegment rspMsg;
	OPCODE resOpcode = LDAP_AUTHENTICATION_LOGIN_IND;
	STATUS responseStatus  = ldapModuleMngrApi.SendMessageSync(pSeg, LDAP_AUTHENTICATION_LOGIN_REQ, AD_AUTHENTICATION_RESPONSE_TIMEOUT, resOpcode, rspMsg);

	if (responseStatus == STATUS_OK)
	{
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleLoginReqWithLdapModule - Ldap server indication was received!";

		DWORD requestId;		//maybe redundant
		rspMsg >> requestId;

		DWORD status;
		rspMsg >> status;

		DWORD authorizationLevel;
		rspMsg >> authorizationLevel;

		CLogInRequest* pLoginRequest = (CLogInRequest*) pRequest->GetRequestObject();

	    string sLoginName = pLoginRequest->GetLoginName();

		CLogInConfirm* pLoginConfirm = new CLogInConfirm(m_bSystemMultipleServices, m_bV35JITCSupport);
		pLoginConfirm->SetDaysUntilPwdExpires(90);

		
		pRequest->SetConfirmObject(pLoginConfirm);

		status = HandleUpdateLdapOperList (status, sLoginName, authorizationLevel, pLoginConfirm);
				
		if (status == STATUS_OK)
		{
			pLoginConfirm->SetAuthorization(authorizationLevel);
			pRequest->SetStatus(status);

			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleLoginReqWithLdapModule: Operator login succeeded: "
									<< pLoginRequest->GetLoginName();

		}
		else
		{
			FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::HandleLoginReqWithLdapModule: Operator login failed: "
									<< pLoginRequest->GetLoginName()
									<< " \nStatus=" << m_pProcess->GetStatusAsString(status);

			pRequest->SetStatus(status);			

		}
		responseStatus = status;
	}
	return responseStatus;
}

//////////////////////////////////////////////////////////////////////
/*void CAuthenticationManager::OnLdapLoginResponseTimeout(CSegment* pParam)
{
	FTRACESTR(eLevelError) << "CAuthenticationManager::OnLdapLoginResponseTimeout";

	WORD requestId =0 ;
	*pParam >> requestId;

	CRequestQueue* pRequestQueue = m_pRequestQueueList->GetRequestQueue(requestId);
	if(NULL == pRequestQueue)
	{
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnLdapLoginResponseTimeout - NULL == pRequestQueue ";
		return;
	}

	SendResponse(EXT_APP_REQUEST_TIMEOUT, pRequestQueue->GetRequest(), pRequestQueue->GetQueue());
	m_pRequestQueueList->RemoveRequest(requestId);
}
*/

//////////////////////////////////////////////////////////////////////
/*int CAuthenticationManager::OnLdapLoginInd(CSegment* pParam)
{
	FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnLdapLoginInd";
	DeleteTimer(LDAP_AUTHENTICATION_LOGIN_RESPONSE_TIMER);

	////////////////////

	DWORD requestId;
	*pParam >> requestId;

	DWORD status;
	*pParam >> status;

	BOOL bJitcMode = IsFederalOn();

	CRequestQueue* pRequestQueue = m_pRequestQueueList->GetRequestQueue(requestId);
	if(NULL == pRequestQueue)
	{
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnLdapLoginInd - NULL == pRequestQueue ";
		return STATUS_FAIL;
	}
	CRequest* pRequest = pRequestQueue->GetRequest();

	CLogInRequest* pLoginRequest = (CLogInRequest*) pRequest->GetRequestObject();

    string sLoginName = pLoginRequest->GetLoginName();
	///string sPassword  = pLoginRequest->GetPassword();

	CLogInConfirm* pLoginConfirm = new CLogInConfirm();
	pLoginConfirm->SetDaysUntilPwdExpires(90);

	COperatorList* pLdapOperList = GetLdapOperatorList();
	COperator* pOperInLdapDB = pLdapOperList->GetOperatorByLogin(sLoginName);
	if (!pOperInLdapDB)
	{
		// add user to Ldap operator list
		COperator* pOperator = new COperator;
		pOperator->SetLogin(sLoginName);
		///pOperator->SetPassword(pLoginRequest->GetPassword());
		///pOperator->SetAuthorization(SUPER);	///TODO: IS NEEDED????  replace with the authorization level as was received from the AD

	    pLdapOperList->Add(*pOperator);
	}
	else
	{
		pLoginConfirm->SetLastLogin(pOperInLdapDB->GetLastLogin());
		pLoginConfirm->SetLastLoginIPaddress(pOperInLdapDB->GetLastLoginIPaddress());
		pLoginConfirm->SetLoginHistory(pOperInLdapDB->GetLoginHistory());
	}

	pRequest->SetConfirmObject(pLoginConfirm);

	if (status == STATUS_OK)
	{
		CStructTm curTime;
		SystemGetTime(curTime);
		pOperInLdapDB->SetLastLogin(curTime);
		pOperInLdapDB->SetLastLoginIPaddress(GetCurrentClientAddress());

		pRequest->SetStatus(status);
		///pLoginConfirm->SetAuthorization(pOperInLdapDB->GetAuthorization());		///????

		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnLdapLoginInd: Operator login succeeded: "
								<< pLoginRequest->GetLoginName();

		if(bJitcMode)
			pLdapOperList->ResetLogInHistory(pOperInLdapDB->GetLogin());
	}
	else
	{
		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnLdapLoginInd: Operator login failed: "
								<< pLoginRequest->GetLoginName()
								<< " \nStatus=" << m_pProcess->GetStatusAsString(status);

		if (bJitcMode)
			status = CheckIfStatusIsValidInJitc(status);

		CStructTm curTime;
		SystemGetTime(curTime);
		pLdapOperList->SetFailedLoginInfo(pOperInLdapDB, curTime, GetCurrentClientAddress());

		pRequest->SetStatus(status);

	}


///	OnExtDBUserLoginConfirm(pParam); //temp func, will be handled separately.

	char* buffer = NULL;
	DWORD len = 0;

	CXMLDOMElement* pXmlStream = new CXMLDOMElement();
	if (status==STATUS_OK)
	{
		CSegment *pSeg = new CSegment;

///		CLogInRequest* pLoginRequest = (CLogInRequest*)(pRequestQueue->GetRequest()->GetRequestObject());
		pLoginRequest->SetExternalDbAuthorized(TRUE);

		pXmlStream = pRequest->SerializeXml();

		pXmlStream->DumpDataAsLongStringEx(&buffer);

		FTRACESTR(eLevelInfoNormal) << "CAuthenticationManager::OnLdapLoginInd - buffer content:\n"
								<< buffer;

		len = strlen(buffer);

		(pRequestQueue->GetQueue())->Serialize(*pSeg);

        CPostXmlHeader postXmlHeader(len,
                                     (WORD)SUPER,
                                     (BYTE)false,
                                     "",
                                     "Authentication WorkStation",
                                     "Authentication UserName",
                                     "Authentication Client Ip",
                                     false, "", "", "");
        postXmlHeader.Serialize(*pSeg);
        pSeg->Put((BYTE*)buffer,len);


		*pSeg << (DWORD) pLoginRequest->GetLoginName().length();
		*pSeg << (char*) pLoginRequest->GetLoginName().c_str();

		POBJDELETE(pXmlStream);
		m_pRequestQueueList->RemoveRequest(requestId);

		HandlePostRequest(pSeg);
		DEALLOCBUFFER(buffer);
	}
	else
	{
///		CLogInConfirm* pLoginConfirm = new CLogInConfirm;
///		pRequest->SetConfirmObject(pLoginConfirm);
///		pRequest->SetStatus(status);
		pXmlStream = pRequest->SetConfirmSerializeXml();

		char *buffer   = NULL;
		pXmlStream->DumpDataAsLongStringEx(&buffer);

		PostXml(buffer,strlen(buffer),*(pRequestQueue->GetQueue()));

		PDELETEA(buffer);
		POBJDELETE(pXmlStream);
	}
	m_pRequestQueueList->RemoveRequest(requestId);

	return status;
}
*/

//////////////////////////////////////////////////////////////////////
void CAuthenticationManager::OnCheckUserListForLdapReq(CSegment* pSeg)
{
	// check that there is only one user in the operator list and confirm that it is an administrator
	STATUS rspStatus = ValidateUserListForLdapJitc();

	CSegment*  pRspMsg = new CSegment;
	*pRspMsg << (DWORD)rspStatus;
	ResponedClientRequest(CHECK_USER_LIST_FOR_LDAP_IND, pRspMsg);
}


//////////////////////////////////////////////////////////////////////
STATUS CAuthenticationManager::ValidateUserListForLdapJitc()
{
	// check that there is only one user in the internal operator list and confirm that it is an administrator
	// (multiple Machine Account users are allowed)

	STATUS status = STATUS_OK;
	COperatorList *pOperatorList  = GetOperatorList();
	COperator     *pUser       = NULL;

	int notMaUsers = 0;
	bool isAdmin = false;

	for (int i=0; i < MAX_OPERATORS_IN_MCU; i++)
	{
		pUser = pOperatorList->GetOperatorById(i);

		if ( pUser && (TRUE != pUser->IsMachineAccount()) )
		{
			notMaUsers++;
			isAdmin = (SUPER == pUser->GetAuthorization() ) ? true : false;
			if( notMaUsers > 1 )
				return STATUS_MORE_THAN_ONE_USER_IN_LIST;
		}
	}

	if( isAdmin == false )
		status = STATUS_USER_IS_NOT_ADMIN;

	return status;
}




//////////////////////////////////////////////////////////////////////
// CRequestQueue class
//////////////////////////////////////////////////////////////////////
CRequestQueue::CRequestQueue()
{
	m_Id = 0;
	m_pRequest = NULL;
}
/////////////////////////////////////////////////////////////////////////////
CRequestQueue::CRequestQueue(WORD id, CRequest* pRequest)
{
	m_Id = id;
	m_pRequest = new CRequest();
	CLogInRequest* req = new CLogInRequest();
	*req = *((CLogInRequest*) pRequest->GetRequestObject());
	m_pRequest->SetRequestObject(req);
	m_pRequest->SetTransName(pRequest->GetTransName());
	m_pRequest->SetActionName(pRequest->GetActionName());
	m_pRequest->SetHdr(pRequest->GetHdr());
	m_pRequest->SetQueue(*(pRequest->GetQueue()));
}
/////////////////////////////////////////////////////////////////////////////
CRequestQueue::~CRequestQueue()
{
	POBJDELETE(m_pRequest)
}
/////////////////////////////////////////////////////////////////////////////
CRequest* CRequestQueue::GetRequest()
{
	return m_pRequest;
}
/////////////////////////////////////////////////////////////////////////////
COsQueue* CRequestQueue::GetQueue()
{
	return m_pRequest->GetQueue();
}
//////////////////////////////////////////////////////////////////////
// CRequestQueueList class
//////////////////////////////////////////////////////////////////////
CRequestQueueList::CRequestQueueList()
{
	m_nCount = 0;
	m_pRequestQueueArray = new std::valarray<CRequestQueue*>((CRequestQueue*)NULL,MAX_CONNECTIONS+1);
}
/////////////////////////////////////////////////////////////////////////////
CRequestQueueList::~CRequestQueueList()
{
	for(DWORD i=1; i < m_pRequestQueueArray->size(); i++)
	{
		CRequestQueue* pReqQueue = (*m_pRequestQueueArray)[i];

		if(pReqQueue)
			delete pReqQueue;
	}

	delete m_pRequestQueueArray;
}
/////////////////////////////////////////////////////////////////////////////
int CRequestQueueList::AddRequest(CRequest* request)
{
	int nRetIndex = 0;

	if(m_nCount < MAX_CONNECTIONS)
	{
		for(DWORD nIndex=1; nIndex < m_pRequestQueueArray->size(); nIndex++)
		{
			if(!(*m_pRequestQueueArray)[nIndex])
			{
				(*m_pRequestQueueArray)[nIndex] = new CRequestQueue(nIndex, request);
				m_nCount++;
				nRetIndex = nIndex;
				break;
			}
		}
	}
	return nRetIndex;
}
/////////////////////////////////////////////////////////////////////////////
void CRequestQueueList::RemoveRequest(DWORD dwRequestId)
{
	if((dwRequestId >= 1) && (dwRequestId < m_pRequestQueueArray->size()))
	{
		CRequestQueue* pRequestToRemove = (*m_pRequestQueueArray)[dwRequestId];

		if(pRequestToRemove)
		{
			delete pRequestToRemove;
			m_nCount--;
		}

		(*m_pRequestQueueArray)[dwRequestId] = NULL;
	}
}
/////////////////////////////////////////////////////////////////////////////
CRequestQueue* CRequestQueueList::GetRequestQueue(int index)
{
	//first, check that we are not unbound the list...
	if ((unsigned int)index <= (m_pRequestQueueArray)->size()) {
		if ((*m_pRequestQueueArray)[index])
			return (*m_pRequestQueueArray)[index];
		else
			return NULL;
	}
	else
		return NULL;
}



/////////////////////////////////////////////////////////////////////////////

void CAuthenticationManager::OnMultipleServicesInd(CSegment* pParam)
{
	*pParam >> m_bSystemMultipleServices;


	m_bV35JITCSupport = GetV35JITCSupport();

	TRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::OnLicensingInd"
						   << "\nMultiple services: : " << (YES ==  m_bSystemMultipleServices? "YES" : "NO")
						   << "\nV35_ULTRA_SECURED_SUPPORT: : " << (YES ==  m_bV35JITCSupport? "YES" : "NO");
}

/////////////////////////////////////////////////////////////////////////////

void CAuthenticationManager::OnMcuMngrSecurityModeInd(CSegment* pParam)
{
	*pParam >> m_bSystemSecurityMode;

	*pParam >> m_bRequestPeerCertificate;
	TRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::OnMcuMngrSecurityModeInd"
						   << "\nSecurity mode: " << (YES ==  m_bSystemSecurityMode? "YES" : "NO")
						   << " m_bRequestPeerCertificate " << (int)m_bRequestPeerCertificate;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CAuthenticationManager::GetV35JITCSupport()
{
	BOOL isV35JITCSupport = NO;
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	sysConfig->GetBOOLDataByKey(CFG_KEY_V35_ULTRA_SECURED_SUPPORT, isV35JITCSupport);

	return isV35JITCSupport;
}


/////////////////////////////////////////////////////////////////////////////
void CAuthenticationManager::OnInstallerAuthenticationRemovePasswordFile()
{
	TRACESTR(eLevelInfoNormal) << "\nCAuthenticationManager::OnInstallerAuthenticationRemovePasswordFile";

	remove(ENC_OPERATOR_DB_FILE);

	CAuthenticationProcess* pProcess = (CAuthenticationProcess*) CAuthenticationProcess::GetProcess();
	pProcess->SetPasswordFileFlg(true);
}







