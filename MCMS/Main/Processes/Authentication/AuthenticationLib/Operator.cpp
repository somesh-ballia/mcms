// Operator.cpp: implementation of the COperator class.
//
//////////////////////////////////////////////////////////////////////

#include "Operator.h"
#include "psosxml.h"
#include "Transactions.h"
#include "XmlApi.h"
#include "InitCommonStrings.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "SslFunc.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"
#include "StringsMaps.h"
#include "Trace.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COperator::COperator()
{
	m_authorizationGroup = ORDINARY;
    m_isForceChangePwd = FALSE;
	m_newPassword = "";
	m_encryptedPassword = "";
	m_isPwdEncrypted = FALSE;
	m_isDisabled = FALSE;
	m_isLocked = FALSE;

	CStructTm time;
	m_accountDisabledAtTime = time;
	m_accountLockedAtTime   = time;
	m_accountEnabledAtTime = time;
	m_isUnderTDD = FALSE;
	m_isUnderFederalTDD = FALSE;

	m_LastPwdChanged.m_day =0;
	m_LastPwdChanged.m_hour=0;
	m_LastPwdChanged.m_min =0;
	m_LastPwdChanged.m_mon =0;
	m_LastPwdChanged.m_sec =0;
	m_LastPwdChanged.m_year=0;

	for(int i=0; i < SIZE_OF_OLD_PWD_LOG; i++)
		m_pwdLog[i] = "";

	m_countChangePwdFailures = 0;
	m_countLoginPwdFailures = 0;
	m_countForceChangePwdNotifications = 0;

	m_firstLoginPwdFailureTime.m_day =0;
	m_firstLoginPwdFailureTime.m_hour=0;
	m_firstLoginPwdFailureTime.m_min =0;
	m_firstLoginPwdFailureTime.m_mon =0;
	m_firstLoginPwdFailureTime.m_sec =0;
	m_firstLoginPwdFailureTime.m_year=0;

	m_oldLogin = "";
	m_newLogin = "";
	m_AudibleAlarm = new CAudibleAlarm();

	m_bMachineAccount = FALSE;
	m_strFqdnName = "";
}

/////////////////////////////////////////////////////////////////////////////
COperator::COperator(const COperator &other) : CSerializeObject(other)
{
	m_authorizationGroup       = other.m_authorizationGroup;
	m_login                    = other.m_login;
	m_password                 = other.m_password;
	m_isForceChangePwd			   = other.m_isForceChangePwd;
	m_newPassword			   = other.m_newPassword;
	m_encryptedPassword 	   = other.m_encryptedPassword;
	m_isPwdEncrypted 		   = other.m_isPwdEncrypted;
	m_isDisabled			   = other.m_isDisabled;
	m_isLocked				   = other.m_isLocked;
	m_accountDisabledAtTime	   = other.m_accountDisabledAtTime;
	m_accountLockedAtTime      = other.m_accountLockedAtTime;
	m_accountEnabledAtTime     = other.m_accountEnabledAtTime;
	m_LastPwdChanged		   = other.m_LastPwdChanged;
// 	m_LastLogin				   = other.m_LastLogin;
	m_oldLogin				   = other.m_oldLogin;
	m_newLogin				   = other.m_newLogin;

	for(int i=0; i < SIZE_OF_OLD_PWD_LOG; i++)
		m_pwdLog[i] = other.m_pwdLog[i];

	m_countChangePwdFailures = other.m_countChangePwdFailures;
	m_countLoginPwdFailures  = other.m_countLoginPwdFailures;
	m_countForceChangePwdNotifications = other.m_countForceChangePwdNotifications;
	m_firstLoginPwdFailureTime = other.m_firstLoginPwdFailureTime;

	m_isUnderTDD		   = other.m_isUnderTDD;
	m_isUnderFederalTDD		   = other.m_isUnderFederalTDD;
	m_loginHistory 	           = other.m_loginHistory;
	m_AudibleAlarm 			   = new CAudibleAlarm(*other.m_AudibleAlarm);
	m_bMachineAccount          = other.m_bMachineAccount;
	m_strFqdnName              = other.m_strFqdnName;
}

/////////////////////////////////////////////////////////////////////////////
COperator::~COperator()
{
	POBJDELETE(m_AudibleAlarm);
}

/////////////////////////////////////////////////////////////////////////////
bool COperator::operator==(const COperator &rHnd)const
{
    if(this == &rHnd)
    {
        return true;
    }

    if(m_login != rHnd.m_login)
    {
        return false;
    }
    if(m_password != rHnd.m_password)
    {
        return false;
    }
    if(m_authorizationGroup != rHnd.m_authorizationGroup)
    {
        return false;
    }
    if(m_isForceChangePwd != rHnd.m_isForceChangePwd)
	{
		return false;
	}
    if(m_encryptedPassword != rHnd.m_encryptedPassword)
    {
    	return false;
    }
    if(m_isPwdEncrypted != rHnd.m_isPwdEncrypted)
	{
		return false;
	}
    if(m_LastPwdChanged != rHnd.m_LastPwdChanged)
    	return false;

//     if(m_LastLogin != rHnd.m_LastLogin)
//         return false;

    if(m_loginHistory.GetLastLogin() != rHnd.GetLastLogin())
        return false;

    if(m_AudibleAlarm!= rHnd.m_AudibleAlarm)
    	return false;

    if( m_bMachineAccount != rHnd.m_bMachineAccount )
    	return false;

    if( m_strFqdnName != rHnd.m_strFqdnName )
    	return false;

    return true;
}

/////////////////////////////////////////////////////////////////////////////
int COperator::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int nStatus;
	CXMLDOMElement *pMCUNode=NULL;

	if (action==NULL || (action!=NULL && strcmp(action,"RENAME_OPERATOR")!=0)) {
		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"USER_NAME",m_login,
									 _1_TO_OPERATOR_NAME_LENGTH);
	}

	GET_VALIDATE_CHILD(pActionNode,"PASSWORD",m_password,_0_TO_OPERATOR_NAME_LENGTH);

	GET_VALIDATE_CHILD(pActionNode,"AUTHORIZATION_GROUP",&m_authorizationGroup,
					   AUTHORIZATION_GROUP_ENUM);

	if (action!=NULL && strcmp(action,"RENAME_OPERATOR")==0)
	{
		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"OLD_USER_NAME",m_oldLogin,_0_TO_OPERATOR_NAME_LENGTH);
		GET_VALIDATE_MANDATORY_CHILD(pActionNode,"NEW_USER_NAME",m_newLogin,_0_TO_OPERATOR_NAME_LENGTH);
	}

//only should be read when reading from file. Not part of the schemas.
	GET_VALIDATE_CHILD(pActionNode,"NEW_PASSWORD",m_newPassword,_0_TO_OPERATOR_NAME_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"ENCRYPTED_PASSWORD",m_encryptedPassword,ONE_LINE_BUFFER_LENGTH);
	CXMLDOMElement *pChild = NULL, *pTempNode;
	char *szVal;
	int i = 0;

	GET_CHILD_NODE(pActionNode, "PWD_LOG", pChild);
	if (pChild)
	{
		GET_FIRST_CHILD_NODE(pChild, "LOG", pTempNode);
		while (pTempNode && i < SIZE_OF_OLD_PWD_LOG)
		{
			GET_VALIDATE(pTempNode, &szVal, ONE_LINE_BUFFER_LENGTH);
			if(nStatus == STATUS_OK)
			{
				m_pwdLog[i] = szVal;
				i++;
			}
			GET_NEXT_CHILD_NODE(pChild, "LOG", pTempNode);
		}
	}

	GET_VALIDATE_CHILD(pActionNode,"FORCE_CHANGE_PASSWORD",&m_isForceChangePwd,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"ENCRYPTED",&m_isPwdEncrypted,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"LAST_PWD_CHANGE",&m_LastPwdChanged,DATE_TIME);
// 	GET_VALIDATE_CHILD(pActionNode,"LAST_LOGIN_DATE",&m_LastLogin,DATE_TIME);
	GET_VALIDATE_CHILD(pActionNode,"CHANGE_PWD_FAILURES",&m_countChangePwdFailures,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"LOGIN_PWD_FAILURES",&m_countLoginPwdFailures,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"CHANGE_PWD_NOTIFICATIONS",&m_countForceChangePwdNotifications,_0_TO_WORD);
	GET_VALIDATE_CHILD(pActionNode,"FIRST_LOGIN_PWD_FAILURE",&m_firstLoginPwdFailureTime,DATE_TIME);
	GET_VALIDATE_CHILD(pActionNode,"DISABLED",&m_isDisabled,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"DISABLED_AT",&m_accountDisabledAtTime,DATE_TIME);
	GET_VALIDATE_CHILD(pActionNode,"LOCKED",&m_isLocked,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"LOCKED_AT",&m_accountLockedAtTime,DATE_TIME);
	GET_VALIDATE_CHILD(pActionNode,"ENABLED_AT",&m_accountEnabledAtTime,DATE_TIME);

	m_loginHistory.DeSerializeXml(pActionNode, pszError, action);
	m_AudibleAlarm->DeSerializeXml(pActionNode, pszError, action);

	GET_VALIDATE_CHILD(pActionNode,"MACHINE_ACCOUNT",&m_bMachineAccount,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"FQDN_NAME",m_strFqdnName,FIFTY_LINE_BUFFER_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::RawSerializeXml(CXMLDOMElement*& pFatherNode, bool bSerializePassword) const
{
	CXMLDOMElement *pMainNode, *pPwdLogNode;

	if(!pFatherNode)
		return;

	pMainNode = pFatherNode->AddChildNode("OPERATOR");

	pMainNode->AddChildNode("USER_NAME",m_login);

	if(bSerializePassword)
	{
		pMainNode->AddChildNode("PASSWORD",m_password);
		pMainNode->AddChildNode("NEW_PASSWORD",m_newPassword);
		pMainNode->AddChildNode("ENCRYPTED_PASSWORD",m_encryptedPassword);

		pPwdLogNode = pMainNode->AddChildNode("PWD_LOG");
		char nodeName[16] = "";
		for(int i=0; i < SIZE_OF_OLD_PWD_LOG; i++)
		{
			if("" == m_pwdLog[i])
				break;

			pPwdLogNode->AddChildNode("LOG", m_pwdLog[i]);
		}
	}

	pMainNode->AddChildNode("AUTHORIZATION_GROUP",m_authorizationGroup,AUTHORIZATION_GROUP_ENUM);

	pMainNode->AddChildNode("FORCE_CHANGE_PASSWORD", m_isForceChangePwd, _BOOL);
	pMainNode->AddChildNode("ENCRYPTED", m_isPwdEncrypted, _BOOL);
	pMainNode->AddChildNode("LAST_PWD_CHANGE",m_LastPwdChanged);
	pMainNode->AddChildNode("CHANGE_PWD_FAILURES",m_countChangePwdFailures);
	pMainNode->AddChildNode("LOGIN_PWD_FAILURES",m_countLoginPwdFailures);
	pMainNode->AddChildNode("CHANGE_PWD_NOTIFICATIONS",m_countForceChangePwdNotifications);
	pMainNode->AddChildNode("FIRST_LOGIN_PWD_FAILURE",m_firstLoginPwdFailureTime);
	pMainNode->AddChildNode("DISABLED", m_isDisabled, _BOOL);
	pMainNode->AddChildNode("DISABLED_AT",m_accountDisabledAtTime);
	pMainNode->AddChildNode("LOCKED", m_isLocked, _BOOL);
	pMainNode->AddChildNode("LOCKED_AT",m_accountLockedAtTime);
	pMainNode->AddChildNode("ENABLED_AT",m_accountEnabledAtTime);

	m_loginHistory.SerializeXml(pMainNode);
	m_AudibleAlarm->SerializeXml(pMainNode);

	pMainNode->AddChildNode("MACHINE_ACCOUNT",m_bMachineAccount,_BOOL);
	pMainNode->AddChildNode("FQDN_NAME",m_strFqdnName);
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pMainNode, *pPwdLogNode;

	if(!pFatherNode)
		return;

	pMainNode = pFatherNode->AddChildNode("OPERATOR");

	pMainNode->AddChildNode("USER_NAME",m_login);
	pMainNode->AddChildNode("PASSWORD",m_password);
	pMainNode->AddChildNode("AUTHORIZATION_GROUP",m_authorizationGroup,AUTHORIZATION_GROUP_ENUM);
	pMainNode->AddChildNode("DISABLED",m_isDisabled,_BOOL);
	pMainNode->AddChildNode("LOCKED",m_isLocked,_BOOL);
	pMainNode->AddChildNode("MACHINE_ACCOUNT",m_bMachineAccount,_BOOL);
	pMainNode->AddChildNode("FQDN_NAME",m_strFqdnName);
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetLogin(const std::string  login)
{
	m_login = login;
}

/////////////////////////////////////////////////////////////////////////////
const std::string& COperator::GetLogin () const
{
	return m_login;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetPassword(const std::string  password)
{
	m_password = password;
	m_isPwdEncrypted = FALSE;
	m_encryptedPassword = "";
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetPasswordOnly(const std::string  password)
{
	m_password = password;

}

/////////////////////////////////////////////////////////////////////////////
void COperator::ClearPassword()
{
	m_password = "";

}

/////////////////////////////////////////////////////////////////////////////
const std::string& COperator::GetPassword () const
{
	if(!m_isPwdEncrypted)
		return m_password;
	else
		return m_encryptedPassword;
}
/////////////////////////////////////////////////////////////////////////////
const std::string& COperator::GetNonEncryptedPassword () const
{
	return m_password;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetAuthorization(const WORD group)
{
	m_authorizationGroup = group;
}

/////////////////////////////////////////////////////////////////////////////
WORD COperator::GetAuthorization() const
{
	return m_authorizationGroup;
}

/////////////////////////////////////////////////////////////////////////////
BOOL COperator::IsForceChangePwd() const
{
	return m_isForceChangePwd;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetForceChangePwd(const BOOL bForce)
{
	m_isForceChangePwd = bForce;
}

/////////////////////////////////////////////////////////////////////////////
std::string COperator::GetNewPassword() const
{
	return m_newPassword;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetNewPassword(std::string newPwd)
{
	m_newPassword = newPwd;
}

/////////////////////////////////////////////////////////////////////////////
std::string COperator::GetNewLogin() const
{
	return m_newLogin;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetNewLogin(const std::string newLogin)
{
	m_newLogin = newLogin;
}

/////////////////////////////////////////////////////////////////////////////
std::string COperator::GetOldLogin() const
{
	return m_oldLogin;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetOldLogin(const std::string oldLogin)
{
	m_oldLogin = oldLogin;
}

/////////////////////////////////////////////////////////////////////////////

BOOL COperator::IsEncryptedPassword() const
{
	return m_isPwdEncrypted;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::EncryptPassword()
{
	   //FTRACESTR(eLogLevelFATAL) << "\n COperator::EncryptPassword : m_password =" << m_password.c_str() ;
	   //FTRACESTR(eLogLevelFATAL) << "\n COperator::EncryptPassword : m_password =" << m_encryptedPassword.c_str() ;

	if(!IsEncryptedPassword())
	{
		//Encrypt password
		CSslFunctions::SHA256_Encryption(m_password, m_encryptedPassword);
		m_isPwdEncrypted = TRUE;
		//Delete unencrypted password
		char p[OPERATOR_NAME_LEN] = "";
		for(int i=0; i<OPERATOR_NAME_LEN; i++)
			p[i]='\0';

		m_password = p;
		  // FTRACESTR(eLogLevelFATAL) << "\n COperator::EncryptPassword : m_password2 =" << m_encryptedPassword.c_str()<<
		  // "password len " << m_encryptedPassword.length();
	}
}

/////////////////////////////////////////////////////////////////////////////
void COperator::EncryptPasswordSHA1()
{
	   //FTRACESTR(eLogLevelFATAL) << "\n COperator::EncryptPassword : m_password =" << m_password.c_str() ;
	   //FTRACESTR(eLogLevelFATAL) << "\n COperator::EncryptPassword : m_password =" << m_encryptedPassword.c_str() ;

	    m_encryptedPassword = "";
		//Encrypt password
		CSslFunctions::SHA1_Encryption(m_password, m_encryptedPassword);
		m_isPwdEncrypted = TRUE;
		//Delete unencrypted password
		char p[OPERATOR_NAME_LEN] = "";
		for(int i=0; i<OPERATOR_NAME_LEN; i++)
			p[i]='\0';

		m_password = p;
		  // FTRACESTR(eLogLevelFATAL) << "\n COperator::EncryptPassword : m_password2 =" << m_encryptedPassword.c_str()<<
		  // "password len " << m_encryptedPassword.length();

}

////////////////////////////////////////////////////////////////////////////
void COperator::SetEncPassword(const std::string  encPassword)
{
	   //FTRACESTR(eLogLevelFATAL) << "\n COperator::SetEncPassword : m_password =" << m_password.c_str() ;

	if (m_password.length() != 0)
	{//Delete unencrypted password
		char p[OPERATOR_NAME_LEN] = "";
		for(int i=0; i<OPERATOR_NAME_LEN; i++)
			p[i]='\0';

		m_password = p;
	}
	m_isPwdEncrypted = TRUE;
	m_encryptedPassword = encPassword;
	   //FTRACESTR(eLogLevelFATAL) << "\n COperator::SetEncPassword : m_password =" << m_encryptedPassword.c_str() ;

}

/////////////////////////////////////////////////////////////////////////////
CStructTm COperator::GetLastPwdChanged() const
{
	return m_LastPwdChanged;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetLastPwdChanged(CStructTm newTime)
{
	m_LastPwdChanged = newTime;
}

/////////////////////////////////////////////////////////////////////////////
DWORD COperator::GetDaysSinceLastPwdChange() const
{
	CStructTm lastPwdChanged = GetLastPwdChanged();
	return GetNumOfPassedDays(lastPwdChanged);
}

/////////////////////////////////////////////////////////////////////////////
CStructTm COperator::GetLastLogin() const
{
    return m_loginHistory.GetLastLogin();//m_LastLogin;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetLastLogin(CStructTm newTime)
{
    m_loginHistory.SetLastLogin(newTime);//m_LastLogin = newTime;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetAudibleAlarm(CAudibleAlarm * audibleAlarm)
{
	*m_AudibleAlarm=* audibleAlarm;
}

/////////////////////////////////////////////////////////////////////////////
CAudibleAlarm* COperator::GetAudibleAlarm()
{
	return m_AudibleAlarm;
}

/////////////////////////////////////////////////////////////////////////////
DWORD COperator::GetDaysSinceLastLogin() const
{
	CStructTm lastLoginTm = GetLastLogin();
	return GetNumOfPassedDays(lastLoginTm);
}

/////////////////////////////////////////////////////////////////////////////
DWORD COperator::GetDaysSinceLastEnabledUser()
{
	CStructTm lastEnabledTm = GetEnabledTime();
	return GetNumOfPassedDays(lastEnabledTm);
}
/////////////////////////////////////////////////////////////////////////////
BOOL COperator::IsDisabled() const
{
	return m_isDisabled;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetDisabled(BOOL bDisabled)
{
	m_isDisabled = bDisabled;
}
/////////////////////////////////////////////////////////////////////////////
BOOL COperator::IsLocked() const
{
	return m_isLocked;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetLock(BOOL bLock)
{
	m_isLocked = bLock;
}
/////////////////////////////////////////////////////////////////////////////
void COperator::SetLockTime(CStructTm time)
{
	m_accountLockedAtTime = time;
}
/////////////////////////////////////////////////////////////////////////////
CStructTm COperator::GetLockTime(void) const
{
	return m_accountLockedAtTime;
}
/////////////////////////////////////////////////////////////////////////////
void COperator::SetDisabledTime(CStructTm time)
{
	m_accountDisabledAtTime = time;
}
/////////////////////////////////////////////////////////////////////////////
CStructTm COperator::GetDisabledTime()
{
	return m_accountDisabledAtTime;
}
/////////////////////////////////////////////////////////////////////////////
CStructTm COperator::GetEnabledTime()
{
	return m_accountEnabledAtTime;
}
/////////////////////////////////////////////////////////////////////////////
void COperator::SetEnabledTime(CStructTm time)
{
	m_accountEnabledAtTime = time;
}
/////////////////////////////////////////////////////////////////////////////
void COperator::UpdatePwdLog()
{
	for(int i=SIZE_OF_OLD_PWD_LOG-1; i > 0; i--)
	{
		m_pwdLog[i] = m_pwdLog[i-1];
	}
	m_pwdLog[0] = GetPassword();
}
/////////////////////////////////////////////////////////////////////////////
void COperator::CopyPWDLog(const COperator &other)
{
   for (int i=0; i<SIZE_OF_OLD_PWD_LOG; i++)
	   m_pwdLog[i] = other.m_pwdLog[i];
}
/////////////////////////////////////////////////////////////////////////////
BOOL COperator::WasPasswordUsed(std::string newPwd)
{
	BOOL result = FALSE;
	std::string encryptedNewPwd="";

	CSslFunctions::SHA256_Encryption(newPwd, encryptedNewPwd);

	//Check with current password
	if(encryptedNewPwd == GetPassword())
		result = TRUE;
	//Check in Log
	else
	{
		BOOL bJitcMode = FALSE;
		DWORD cgfPwdLogSize = 0;

		if(!m_isUnderTDD)
		{
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			if(sysConfig)
			{
				sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_HISTORY_SIZE, cgfPwdLogSize);
				sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);
			}
		}
		else
		{
			if(m_isUnderFederalTDD)
				bJitcMode = TRUE;
			cgfPwdLogSize = 10;
		}

		if(cgfPwdLogSize)
		{
			for(DWORD i=0; i < cgfPwdLogSize-1; i++)
			{
				if(encryptedNewPwd == m_pwdLog[i])
				{
					result = TRUE;
					break;
				}
			}
		}
	}
	return result;
}

/////////////////////////////////////////////////////////////////////////////
WORD COperator::GetChangePwdFailures() const
{
	return m_countChangePwdFailures;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::IncrementChangePwdFailures()
{
	m_countChangePwdFailures++;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::ResetChangePwdFailures()
{
	m_countChangePwdFailures = 0;
}

/////////////////////////////////////////////////////////////////////////////
WORD COperator::GetLoginPwdFailures() const
{
	return m_countLoginPwdFailures;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::IncrementLoginPwdFailures()
{
	m_countLoginPwdFailures++;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::ResetLoginPwdFailures()
{
	m_countLoginPwdFailures = 0;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::IncrementCountForceChangePwdNotifications()
{
	m_countForceChangePwdNotifications++;
}

/////////////////////////////////////////////////////////////////////////////
WORD COperator::GetCountForceChangePwdNotifications()
{
	return m_countForceChangePwdNotifications;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::ResetCountForceChangePwdNotifications()
{
	m_countForceChangePwdNotifications = 0;
}


/////////////////////////////////////////////////////////////////////////////
std::string COperator::GetLastLoginIPaddress() const
{
	return m_loginHistory.GetLastLoginIPaddress();
}
/////////////////////////////////////////////////////////////////////////////
void  COperator::SetLastLoginIPaddress(const std::string address)
{
    m_loginHistory.SetLastLoginIPaddress(address);
}
/////////////////////////////////////////////////////////////////////////////
void  COperator::SetFailedLoginInfo(const CStructTm login, const std::string address)
{
    //FTRACESTR(eLevelError) << "\n COperator::SetFailedLoginInfo : login =" << GetLogin().c_str() << " at " << login;
    m_loginHistory.SetFailedLoginInfo(login, address);
}
/////////////////////////////////////////////////////////////////////////////
void  COperator::ResetLogInHistory()
{
    m_loginHistory.ResetLogInHistory();
}
/////////////////////////////////////////////////////////////////////////////
BOOL  COperator::IsFirstLogin() const
{
    return m_loginHistory.IsFirstLogin();
}
/////////////////////////////////////////////////////////////////////////////
CStructTm COperator::GetFirstFailureTime()
{
	return m_firstLoginPwdFailureTime;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetFirstFailureTime(CStructTm time)
{
	m_firstLoginPwdFailureTime = time;
}

/////////////////////////////////////////////////////////////////////////////
DWORD COperator::GetNumOfPassedDays(CStructTm &otherTime) const
{
	DWORD passedDays = 0;

	CStructTm curTime;
	SystemGetTime(curTime);

	if (curTime > otherTime )
	{
		int theDenominator = 24/*hours*/ * 60/*minutes*/ * 60/*seconds*/;

		DWORD diffTime	= curTime - otherTime ;
		passedDays		= diffTime / theDenominator;
	}

	return passedDays;
}

/////////////////////////////////////////////////////////////////////////////
CStructTm COperator::GetLastFailedLoginInList()const
{
	return m_loginHistory.GetLastFailedLoginInList();
}


/////////////////////////////////////////////////////////////////////////////
void COperator::SetMachineAccount(const bool bMachineAccount)
{
	m_bMachineAccount = bMachineAccount;
}

/////////////////////////////////////////////////////////////////////////////
void COperator::SetFqdnName(const std::string sFqdnName)
{
	m_strFqdnName = sFqdnName;
}



//****** FOR TDD USE ONLY !!! *****
/////////////////////////////////////////////////////////////////////////////
void COperator::SetTDDState(BOOL isFederal)
{
	m_isUnderTDD = TRUE;
	if(isFederal)
		m_isUnderFederalTDD = TRUE;
}
//****** FOR TDD USE ONLY !!! *****
