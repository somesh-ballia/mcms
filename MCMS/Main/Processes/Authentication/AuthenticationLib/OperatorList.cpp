// OperatorList.cpp

#include "OperatorList.h"

#include <string>
#include <algorithm>

#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ConfigManagerApi.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ProcessBase.h"
#include "OsFileIF.h"
#include "SslFunc.h"
#include "ObjString.h"
#include "GlobalDataAccess.h"
#include "AuditorApi.h"
#include "OpcodesMcmsInternal.h"
#include "FaultsDefines.h"
#include "SysConfig.h"

#define ACCOUNT_IS_CLOSE_TO_EXPERATION_DATE   	7	// user is not allowed to be inactive (not perform login) more than 30 days

struct SDescendingOperatorSort
{
  bool operator()(COperator* const & rpStart, COperator* const & rpEnd)
  {
    return rpStart->GetDaysSinceLastLogin() > rpEnd->GetDaysSinceLastLogin();
  }
};

COperatorList::COperatorList(BYTE bIsFirstJitc, BOOL isLdap)
{
  m_nCount = 0;

  for (int i=0;i<MAX_OPERATORS_IN_MCU;i++)
  {
    m_pOperators[i] = NULL;
  }

  m_nIterIndex=0;
  m_updateCounter = 0;
  m_isListCorrupted		= false;
  m_isListDefault		= false;
  m_isDefaultUserExists	= false;

  m_isUnderTDD = FALSE;
  m_isUnderFederalTDD = FALSE;

  if ((bIsFirstJitc) && (!isLdap))
  {
	  remove(OLD_OPERATOR_DB_FILE);
	  remove(ENC_OPERATOR_DB_FILE);
  }

  m_isLdapOperatorList = isLdap;

  LoadFromFileDB(bIsFirstJitc);

  if (TRUE == m_isLdapOperatorList)
    return;

  if (!IsFederalOn() || CConfigManagerApi::IsSSHOnForDebug())
  {
    CConfigManagerApi api;
    api.AddAdminUser("rt3p1aa", "1z56tcp");  //TODO - here ??????
  }

}

COperatorList::~COperatorList()
{
  for (int i = 0; i < MAX_OPERATORS_IN_MCU; i++)
    PDELETE(m_pOperators[i]);
}

void COperatorList::SerializeXml(CXMLDOMElement*& pActionNode, DWORD dwObjectToken,WORD nReqAuthorization/*=ANONYMOUS*/) const
{
	BYTE bChanged = FALSE;
	BYTE bJitcMode = IsFederalOn();
	COperator* pReqOperator = NULL;
	std::string requestingUserName = "";

	COperatorList* pOperList;

	if(bJitcMode)
	{
		char pszError[64]="";
		int nStatus = pActionNode->GetAndVerifyChildNodeValue("REQ_USER", requestingUserName, pszError, _1_TO_OPERATOR_NAME_LENGTH);

		if(nStatus == STATUS_OK)
		{
			pOperList = GetOperatorList();
			pReqOperator = pOperList->GetOperatorByLogin(requestingUserName);

			if (pReqOperator == NULL)
			{
				CAuthenticationProcess* pProcess = (CAuthenticationProcess*) CAuthenticationProcess::GetProcess();
				pOperList = pProcess->GetLdapOperatorList();
				if (pOperList)
					pReqOperator = pOperList->GetOperatorByLogin(requestingUserName);
			}
		}
	}

    CXMLDOMElement *pSumListNode=pActionNode->AddChildNode("OPER_LIST");
	pSumListNode->AddChildNode("OBJ_TOKEN",m_updateCounter);

	if (dwObjectToken == 0xFFFFFFFF)
		bChanged = TRUE;
	else
	{
		if(m_updateCounter > dwObjectToken)
			bChanged=TRUE;
	}

	pSumListNode->AddChildNode("CHANGED",bChanged,_BOOL);

	if(bChanged)
	{
		for (int i=0;i<(int)m_nCount;i++)
		{
			if(bJitcMode)
			{
				if((pReqOperator && SUPER ==pReqOperator->GetAuthorization()) ||
					requestingUserName == m_pOperators[i]->GetLogin() ||
					( SUPER == nReqAuthorization) )
				{
					m_pOperators[i]->SerializeXml(pSumListNode);
				}
			}
			else
				m_pOperators[i]->SerializeXml(pSumListNode);
		}
	}
}

int COperatorList::DeSerializeXml(CXMLDOMElement *pOperListNode,char* pszError, const char* action)
{
	return STATUS_OK;
}

std::string COperatorList::GetCustomUserLogin()
{
 	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeSoftMCUMfw == curProductType)
	{
		std::string sdata;
		char user[100]="";
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		if(sysConfig->GetDataByKey("CUSTOM_USER_LOGIN", sdata) && sdata.length() > 0 && sdata.length() < 100)
		{
			strncpy( user, sdata.c_str(), 99);
			user[sdata.length()] = '\0';
			return user;
		}
	}

	return DEFAULT_USER_LOGIN;
}

std::string COperatorList::GetCustomUserPasswd()
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (eProductTypeSoftMCUMfw == curProductType)
	{
		std::string sdata;
		char pwd[100]="";

		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
		if(sysConfig->GetDataByKey("CUSTOM_USER_PASSWD", sdata) && sdata.length() > 0 && sdata.length() < 100)
		{
			strncpy( pwd, sdata.c_str(), 99);
			pwd[sdata.length()] = '\0';
			return pwd;
		}
	}

	return DEFAULT_USER_PASSWORD;
}

STATUS COperatorList::IsValidToAddUser(COperator&  other, BOOL bIsAdAvailable)
{
	STATUS result = STATUS_OK;

	if (m_nCount >= MAX_OPERATORS_IN_MCU)
		result =  STATUS_NUMBER_OF_OPERATORS_IN_MCU_EXCEEDED;

	if(STATUS_OK == result)
		if (FindLogin(other.GetLogin().c_str()) != NOT_FOUND)
			result = STATUS_LOGIN_EXISTS;


	// SINGLE ROLE ACCOUNT
    BOOL bJitcMode = IsFederalOn();
	if(STATUS_OK == result && bJitcMode)
	{
		WORD authorization = other.GetAuthorization();
		//No 'chairperson' or 'auditor' operator types can be added in JITC mode
		if(AUTH_OPERATOR == authorization || AUDITOR == authorization)
		    result = STATUS_ACCOUNT_AUTHORIZATION_LEVEL_NOT_ALLOWED;
		else if ((bIsAdAvailable) && (!other.IsMachineAccount())) //multiple Machine Account users are allowed even if Ldap is enabled
		{
			//for LDAP - in case Ldap is enabled, only one user (administrator) is allowed
			if (ORDINARY == authorization)
			    result = STATUS_ACCOUNT_AUTHORIZATION_LEVEL_NOT_ALLOWED_LDAP;
			else if (NumberOfAdministrator() >= 1)
				result = STATUS_ONLY_ONE_ACCOUNT_IS_ALLOWED_IN_JITC_LDAP;
		}
	}

	return result;
}

void COperatorList::Add(COperator&  other)
{
	if ((TRUE == IsLdapOperatorList()) && (m_nCount >= MAX_OPERATORS_IN_MCU))
	{
		DeleteUserWithOldestLoginRecord();
	}

	m_pOperators[m_nCount] = new COperator(other);

	if (FALSE == IsLdapOperatorList())
	{
		CStructTm curTime;
		SystemGetTime(curTime);
		m_pOperators[m_nCount]->SetLastPwdChanged(curTime);
		m_pOperators[m_nCount]->SetLastLogin(curTime);
	}

	m_nCount++;

	IncreaseUpdateCounter();
	UpdateIsDefaultUserExists();

	SaveToFileDB();
}

void COperatorList::DeleteUserWithOldestLoginRecord()
{
	CStructTm earliestTime;
	SystemGetTime(earliestTime);	// initialize variable with current time
	CStructTm defaultTime;
	defaultTime.InitDefaults();

	int oldestLdapUserIndex = 0;

	for (int ind=0; ind < MAX_OPERATORS_IN_MCU; ind++)
	{
		if (m_pOperators[ind]->GetLastFailedLoginInList() == defaultTime)
		{
			if (m_pOperators[ind]->GetLastLogin() < earliestTime)
			{
				oldestLdapUserIndex = ind;
				earliestTime = m_pOperators[ind]->GetLastLogin();
			}
		}
		else
		{
			if (m_pOperators[ind]->GetLastFailedLoginInList() < earliestTime)
			{
				oldestLdapUserIndex = ind;
				earliestTime = m_pOperators[ind]->GetLastFailedLoginInList();
			}
		}
	}

	// delete ldap user with the oldest login records
	PDELETE(m_pOperators[oldestLdapUserIndex]);

	for (int j = oldestLdapUserIndex; j < (int)m_nCount-1 && j < MAX_OPERATORS_IN_MCU-1; j++)
  	{
    	m_pOperators[j] = m_pOperators[j+1] ;
  	}
  	m_pOperators[m_nCount-1] = NULL;

  	m_nCount--;

}

int COperatorList::IsValidToChangePassword(std::string strLogin, std::string strOldPassword, int &authorization)
{
	int ind;
	ind=FindLogin(strLogin.c_str());

	if (ind == NOT_FOUND)
		return STATUS_LOGIN_NOT_EXISTS;

	std::string strEncryptedOldPwd = "";
	CSslFunctions::SHA256_Encryption(strOldPassword, strEncryptedOldPwd);

	if (m_pOperators[ind]->GetPassword() != strEncryptedOldPwd)
		return STATUS_OLD_PASSWORD_IS_INCORRECT;

	authorization = m_pOperators[ind]->GetAuthorization();

	return STATUS_OK;
}

STATUS COperatorList::CheckChangeFreq(std::string strLogin) const
{
	STATUS status = STATUS_FAIL;
	int ind;
	ind=FindLogin(strLogin.c_str());

	if (ind != NOT_FOUND)
	{
		//Test password change freq.

		// if operator is Machine Account -> skip test
		if( m_pOperators[ind]->IsMachineAccount() )
			return STATUS_OK;

		//If operator is marked as 'force change new password', skip this test.
		if((IsFederalOn() || CMcmsAuthentication::IsForceStrongPassword()) && !m_pOperators[ind]->IsForceChangePwd())
		{
			DWORD daysFromLastPwdChange = m_pOperators[ind]->GetDaysSinceLastPwdChange();

			DWORD minPwdChangeFreq = 0;
			//BOOL bJitcMode = FALSE;
			if(!m_isUnderTDD)
			{
				CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				sysConfig->GetDWORDDataByKey(CFG_KEY_MIN_PWD_CHANGE_FREQUENCY_IN_DAYS, minPwdChangeFreq);
				//sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);
			}


			FTRACESTR(eLevelInfoNormal) << "COperatorList::CheckChangeFreq - daysFromLastPwdChange=" << daysFromLastPwdChange;

			//minPwdChangeFreq == 0 means this feature is OFF.
			if ( minPwdChangeFreq > daysFromLastPwdChange && 0 != minPwdChangeFreq)
			{
				status = STATUS_PASSWORD_CANNOT_BE_CHANGED_YET;
				FTRACESTR(eLevelInfoNormal) << "COperatorList::CheckChangeFreq - can not change password before " << minPwdChangeFreq << " days have passed.";
			}
			else
				status = STATUS_OK;
		}
		if(m_pOperators[ind]->IsForceChangePwd() || (!IsFederalOn() && !CMcmsAuthentication::IsForceStrongPassword()))
			status = STATUS_OK;
	}

	return status;
}

STATUS COperatorList::UpdatePassword(std::string strLogin, std::string strNewPassword, BOOL bForceChangePwd)
{
	STATUS status = STATUS_FAIL;
	int ind;
	ind=FindLogin(strLogin.c_str());

	if (ind != NOT_FOUND)
	{
		m_pOperators[ind]->UpdatePwdLog();

		m_pOperators[ind]->SetPassword(strNewPassword);
		m_pOperators[ind]->EncryptPassword();

		CStructTm	curTime;
		SystemGetTime(curTime);
		m_pOperators[ind]->SetLastPwdChanged(curTime);
		m_pOperators[ind]->SetForceChangePwd(bForceChangePwd);

		IncreaseUpdateCounter();
		SaveToFileDB();

		status = STATUS_OK;
	}

	return status;
}

STATUS COperatorList::IsValidToDeleteUser(std::string login, int& authorization)
{
	int ind;

	ind=FindLogin(login.c_str());

	if (ind==NOT_FOUND)
		return STATUS_LOGIN_NOT_EXISTS;

	authorization = m_pOperators[ind]->GetAuthorization();
	if(authorization == SUPER)
	{
		if(IsLastEnabledAdministrator(login))
			return STATUS_LAST_SUPERVISOR_OPERATOR_IN_MCU;
	}

	return STATUS_OK;
}

STATUS COperatorList::IsValidToDisableUser(const char* login)
{
	int ind;

	ind=FindLogin(login);

	if (ind==NOT_FOUND)
		return STATUS_LOGIN_NOT_EXISTS;

	int authorization = m_pOperators[ind]->GetAuthorization();
	if(authorization == SUPER)
	{
		if(IsLastEnabledAdministrator(login))
			return STATUS_LAST_ENABLED_SUPERVISOR_OPERATOR_IN_MCU;
	}
	return STATUS_OK;
}

void COperatorList::Cancel(const char* login)
{
	int ind;

	ind=FindLogin(login);
	if(ind != NOT_FOUND && ind < MAX_OPERATORS_IN_MCU)
	{
		PDELETE(m_pOperators[ind]);

		int i;

		for (i=0; i < (int)m_nCount; i++)
		{
			if (m_pOperators[i]==NULL)
				break;
		}

		for (int j=i; j < (int)m_nCount-1; j++)
		{
			m_pOperators[j]=m_pOperators[j+1] ;
		}

		m_pOperators[m_nCount-1] = NULL;
		m_nCount--;

		IncreaseUpdateCounter();
		UpdateIsDefaultUserExists();

		SaveToFileDB();
	}
	else
		FPTRACE2(eLevelInfoNormal, "\nCOperatorList::Cancel, not found:", login);
}

void COperatorList::Clear(COperator *defaultOperator)
{
  for (int i = 0; i < m_nCount; i++)
  {
    if (*defaultOperator == *m_pOperators[i])
    {
      m_pOperators[i] = NULL;
    }
    else
    {
      PDELETE( m_pOperators[i]);
    }
  }

  m_pOperators[0] = defaultOperator;
  m_nCount = 1;

  IncreaseUpdateCounter();
  SaveToFileDB();
}

int COperatorList::FindOperator(COperator &oper)
{
  for (int i=0; i < MAX_OPERATORS_IN_MCU; i++)
  {
     if (m_pOperators[i]!=NULL) {
         if ((m_pOperators[i]->GetLogin() == oper.GetLogin()) &&
			 (m_pOperators[i]->GetPassword() == oper.GetPassword()))
		 {
			oper = *(m_pOperators[i]);
            return i;
		 }
     }
  }

  return NOT_FOUND;
}

int COperatorList::FindLogin(const char* login) const
{
  for (int i=0; i < MAX_OPERATORS_IN_MCU; i++)
  {
     if (m_pOperators[i]!=NULL) {
         if (m_pOperators[i]->GetLogin() == login)
              return i;
     }
  }

  return NOT_FOUND;
}

int COperatorList::FindCaseSensitiveLogin(const char* login) const
{
  string LoginParam (login);
  string LoginInListParam;
  for (int i=0; i < MAX_OPERATORS_IN_MCU; i++)
  {
     if (m_pOperators[i]!=NULL) {
    	 LoginInListParam = m_pOperators[i]->GetLogin();
    	  CObjString::ToUpper((char*)(LoginInListParam.c_str()));
    	  CObjString::ToUpper((char*)(LoginParam.c_str()));
         if (LoginInListParam == LoginParam)
              return i;
     }
  }

  return NOT_FOUND;
}

STATUS COperatorList::CheckPassword(COperator* const pOper)
{
	for (int i=0; i < MAX_OPERATORS_IN_MCU; i++)
	{
		if (m_pOperators[i]!=NULL)
		{
			if ((m_pOperators[i]->GetLogin() == pOper->GetLogin()) &&
				(m_pOperators[i]->GetPassword() == pOper->GetPassword()))
			{
				return STATUS_OK;
		 	}
     	}
  	}
	return STATUS_LOGIN_INVALID;
}

int COperatorList::NumberOfAdministrator()
{
	int num_of_admin = 0;
	for (int i=0; i < MAX_OPERATORS_IN_MCU; i++)
	{
		if (m_pOperators[i]!=NULL)
	   	{
			if(m_pOperators[i]->GetAuthorization() == SUPER)
				num_of_admin++;
	   	}
	}

	return num_of_admin;
}

COperator* COperatorList::GetOperatorById(const int idx)
{
	if ( (idx < 0) || (idx >= MAX_OPERATORS_IN_MCU) )
		return NULL;

	return m_pOperators[idx];
}

COperator* COperatorList::GetOperatorByLogin(const std::string login)
{
	int ind = FindLogin(login.c_str());
	if (ind==NOT_FOUND)
		return NULL;

	return m_pOperators[ind];
}

/////////////////////////////////////////////////////////////////////////////
DWORD  COperatorList::GetUpdateCounter() const
{
	return m_updateCounter;
}

void COperatorList::IncreaseUpdateCounter()
{
    m_updateCounter++;

    if (m_updateCounter == 0xFFFFFFFF)
        m_updateCounter = 0;
}

WORD COperatorList::GetCount()
{
	return m_nCount;
}

void COperatorList::SaveToFileDB()
{
	char* pszOperListXml;
	char* operatorDbFile;

	//12.11.12 BRIDGE-2074 Rachel cohen
	//for the migration of SHA1 to SHA256 we should remove the ENC_OPERATOR_DB_FILE on downgrade to old version
	CAuthenticationProcess* pProcess = (CAuthenticationProcess*) CAuthenticationProcess::GetProcess();
	BYTE isFileHasBeenRemoved = pProcess->GetPasswordFileFlg();
	if (isFileHasBeenRemoved == true)
	{
		FTRACESTR(eLevelInfoNormal) << "COperatorList::SaveToFileDB isFileHasBeenRemoved is true";
		remove(ENC_OPERATOR_DB_FILE);
		if (IsFileExists(ENC_OPERATOR_DB_FILE))
		{
			FTRACESTR(eLevelInfoNormal) << "COperatorList::SaveToFileDB after remove file still exist";
		    //SystemPipedCommand("sudo /bin/chmod 777 MCU_MCMS_DIR+/Cfg/EncOperatorDB.xml", answer);
		    remove(ENC_OPERATOR_DB_FILE);
		}
		return;
	}

	if (TRUE == IsLdapOperatorList())
		operatorDbFile = LDAP_OPERATOR_DB_FILE;
	else
		operatorDbFile = ENC_OPERATOR_DB_FILE;


	FILE* pFileOperDB = fopen(operatorDbFile,"w");

	if(!pFileOperDB)
		return;

	CXMLDOMElement *OperListDOMElement = new CXMLDOMElement;

	OperListDOMElement->set_nodeName("OPER_LIST");

	for (int i=0;i<(int)m_nCount;i++)
		m_pOperators[i]->RawSerializeXml(OperListDOMElement,true);

	OperListDOMElement->DumpDataAsLongStringEx(&pszOperListXml);

	if(pszOperListXml)
	{
		fprintf(pFileOperDB,"%s",pszOperListXml);
		DEALLOCBUFFER(pszOperListXml);
	}

	int fcloseReturn = fclose(pFileOperDB);
	if (FCLOSE_SUCCESS != fcloseReturn)
	{
		perror("\nCOperatorList::SaveToFileDB - fclose failed. "); // for printing the errno
		FPTRACE2(eLevelInfoNormal, "\nCOperatorList::SaveToFileDB - failed to close file ", operatorDbFile);
	}

	PDELETE(OperListDOMElement);
}

void COperatorList::LoadFromFileDB(BYTE bIsFirstJitc)
{
	char szFileLine[100], *pRet, *pNewLine, szErrorMsg[ERROR_MESSAGE_LEN];
	char* buffer = NULL;
	BYTE bFirstTime = FALSE, bOldFile = FALSE;
	std::string strOperListXml, strFileName;
	FILE* pFileOperDB = NULL;
	STATUS status = STATUS_FAIL;

	if (TRUE == IsLdapOperatorList())
	{
		pFileOperDB = fopen(LDAP_OPERATOR_DB_FILE, "r");
		strFileName = LDAP_OPERATOR_DB_FILE;
	}
	else
	{
		pFileOperDB = fopen(OLD_OPERATOR_DB_FILE, "r");
		if(!pFileOperDB)
		{
			pFileOperDB = fopen(ENC_OPERATOR_DB_FILE, "r");
			strFileName = ENC_OPERATOR_DB_FILE;
		}
		else
		{
			bOldFile = TRUE;
			strFileName = OLD_OPERATOR_DB_FILE;

			if(IsFederalOn())
			{
				fclose(pFileOperDB);
				DeleteFile(OLD_OPERATOR_DB_FILE);
				pFileOperDB = NULL;
			}
		}
	}

	const char *pszOperListXml;

	if(!pFileOperDB)
	{
		status = CreateDefaultDBFile();
		if(STATUS_OK == status)
		{
			if (FALSE == IsLdapOperatorList())
				AddDefaultOperatorOnFailure();

			if(!bIsFirstJitc)
				m_isListCorrupted = true;

			return;
		}
		else
		{
			bFirstTime = TRUE;
		}
	}

	if (bFirstTime==FALSE && pFileOperDB)
	{
		pRet = fgets(szFileLine,99,pFileOperDB);

		while(pRet)
		{
			pNewLine = strstr(szFileLine,"\n");

			if(pNewLine)
				*pNewLine = 0;

			strOperListXml += szFileLine;
			pRet = fgets(szFileLine,99,pFileOperDB);
		}

		int fcloseReturn = fclose(pFileOperDB);
		if (FCLOSE_SUCCESS != fcloseReturn)
		{
			perror("\nCOperatorList::LoadFromFileDB - fclose failed. "); // for printing the errno
			FPTRACE2(eLevelInfoNormal, "\nCOperatorList::LoadFromFileDB - failed to close file ", strFileName.c_str());
		}

		buffer = new char[strOperListXml.size()+1];
		strcpy(buffer, strOperListXml.c_str());
	}

	pszOperListXml = buffer;

	CXMLDOMElement OperListDOMElement, *pOperatorNode;

	if(OperListDOMElement.Parse(&pszOperListXml) != SEC_OK)
	{
		FPASSERTMSG(1,"COperatorList::LoadFromFileDB: Failed parsing Operator List XML");
		DEALLOCBUFFER(buffer);

		// 07.03.07: add default user to memory & OS, so system can be accessed
		if (FALSE == IsLdapOperatorList())
			AddDefaultOperatorOnFailure();
		m_isListCorrupted = true;
		return;
	}

	// Prevents compiles's warning:
  // warning: the address of 'OperListDOMElement' will never be NULL
	CXMLDOMElement* pp = &OperListDOMElement;
  GET_FIRST_CHILD_NODE(pp, "OPERATOR", pOperatorNode);

	while(pOperatorNode && (m_nCount < MAX_OPERATORS_IN_MCU))
	{
		COperator* pOperator = new COperator;
		STATUS status = pOperator->DeSerializeXml(pOperatorNode,szErrorMsg);

		if(status != STATUS_OK)
		{
			FPTRACE(eLevelInfoNormal, "COperatorList::LoadFromFileDB: Failed Deserializing one of the Operators from file");
			POBJDELETE(pOperator);
		}
		else
		{
			//if data is taken from old operators file, then encrypt the password
			if(bOldFile)
			{
				if(!IsFederalOn())
				{
					if(pOperator->GetLogin()==DEFAULT_USER_LOGIN &&
							COperatorList::GetCustomUserLogin()!=DEFAULT_USER_LOGIN)
						{
						   pOperator->SetLogin(COperatorList::GetCustomUserLogin());
						   pOperator->SetPassword(COperatorList::GetCustomUserPasswd());
						}
					CLargeString description;
					status = CMcmsAuthentication::IsLegalStrongPassword(pOperator->GetLogin(), pOperator->GetPassword(), description);

					if(STATUS_OK != status)
						pOperator->SetForceChangePwd(TRUE);

					CStructTm curTime;
					SystemGetTime(curTime);
					pOperator->SetLastPwdChanged(curTime);
					pOperator->SetLastLogin(curTime);
				}
				pOperator->EncryptPassword();
			}

			POBJDELETE(m_pOperators[m_nCount]);
			m_pOperators[m_nCount] = pOperator;

			m_nCount++;
		}

		GET_NEXT_CHILD_NODE(pp, "OPERATOR", pOperatorNode);
	}

	//Load all operator list to linux
	//JITC Version does not allow adding accounts to ssh
	if(!IsFederalOn())
	{
		CConfigManagerApi api;
		for (int i=0; i<m_nCount; i++)
		{
			if (m_pOperators[i]->GetAuthorization()==SUPER)
				api.AddAdminUser(m_pOperators[i]->GetLogin(), m_pOperators[i]->GetPassword());
		}
	}

	DEALLOCBUFFER(buffer);

	UpdateIsDefaultUserExists();

	if(bOldFile)
	{
		DeleteFile(OLD_OPERATOR_DB_FILE);
		SaveToFileDB();
	}
}

BOOL COperatorList::IsSupportNameExistInTheOperatorList()
{
	for (int i=0; i<m_nCount; i++)
	{
		if (m_pOperators[i]->GetLogin()=="SUPPORT")
			return TRUE;
	}
	return FALSE;
}

STATUS COperatorList::CreateDefaultDBFile(void)
{
	FILE* pFileOperDB = NULL;
	STATUS result = STATUS_FAIL;
	CXMLDOMElement* pRootNode =  new CXMLDOMElement;

	if (TRUE == IsLdapOperatorList())
	{
		pRootNode->set_nodeName("OPER_LIST");
	}
	else
	{
		COperator* pDefaultOperator = new COperator;
		pDefaultOperator->SetLogin(GetCustomUserLogin());
		pDefaultOperator->SetAuthorization(DEFAULT_USER_AUTHORIZATION);
		pDefaultOperator->SetPassword(GetCustomUserPasswd());
		pDefaultOperator->SetForceChangePwd(TRUE);

		CStructTm curTime;
		SystemGetTime(curTime);

		pDefaultOperator->SetLastPwdChanged(curTime);
		pDefaultOperator->SetLastLogin(curTime);
		pDefaultOperator->SetLastLoginIPaddress("");

		if(IsFederalOn())
			pDefaultOperator->SetForceChangePwd(TRUE);

		pDefaultOperator->EncryptPassword();

		pRootNode->set_nodeName("OPER_LIST");

		pDefaultOperator->RawSerializeXml(pRootNode, true);

		POBJDELETE(pDefaultOperator);
	}

    char* buffer = NULL;
	pRootNode->DumpDataAsLongStringEx(&buffer);

	POBJDELETE(pRootNode);

	if (TRUE == IsLdapOperatorList())
		pFileOperDB = fopen(LDAP_OPERATOR_DB_FILE, "w");
	else
		pFileOperDB = fopen(ENC_OPERATOR_DB_FILE, "w");

	if (!pFileOperDB)
	{
		if (TRUE == IsLdapOperatorList())
			FPTRACE2(eLevelInfoNormal, "COperatorList::CreateDefaultDBFile: failed to open file ", LDAP_OPERATOR_DB_FILE);
		else
			FPTRACE2(eLevelInfoNormal, "COperatorList::CreateDefaultDBFile: failed to open file ", ENC_OPERATOR_DB_FILE);
		DEALLOCBUFFER(buffer);
		return result;
	}

	fputs(buffer, pFileOperDB);
	fclose(pFileOperDB);
	DEALLOCBUFFER(buffer);

	result = STATUS_OK;

	if (TRUE == IsLdapOperatorList())
		FPTRACE2(eLevelInfoNormal, "COperatorList::CreateDefaultDBFile: New LDAP Operators file has been created",buffer);
	else
		FPTRACE(eLevelInfoNormal, "COperatorList::CreateDefaultDBFile: New Operators file has been created");

	m_isListDefault = true;
	return result;
}

BOOL COperatorList::IsFederalOn() const
{
	BOOL result = FALSE;
	BOOL bJitcMode = FALSE;

	if(!m_isUnderFederalTDD && !m_isUnderTDD)
	{
		CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();		
		if (sysConfig)
			sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);
		if (sysConfig==NULL)
		    return FALSE;
	}
	else
		if(m_isUnderFederalTDD)
			bJitcMode = TRUE;

	if(bJitcMode)
	{
		result = TRUE;
		FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::IsFederalOn - System is in FEDERAL MODE!";
	}

	return result;
}

void COperatorList::AddDefaultOperatorOnFailure()
{
	// ==== 1. add in memory
	COperator* pOperator = new COperator;
	pOperator->SetLogin(GetCustomUserLogin());
	pOperator->SetPassword(GetCustomUserPasswd());
	pOperator->SetAuthorization(DEFAULT_USER_AUTHORIZATION);
	pOperator->EncryptPassword();

	if(IsFederalOn())
		pOperator->SetForceChangePwd(TRUE);

	int idx = ( (MAX_OPERATORS_IN_MCU > m_nCount)
	            ?
	            m_nCount : 0 );

	POBJDELETE(m_pOperators[idx]);
	m_pOperators[idx] = pOperator;

	m_nCount++;
	IncreaseUpdateCounter();
	m_isDefaultUserExists = true;
}

std::string COperatorList::WriteUserList()
{
	std::string str_users = "";
	char msg[200];
	char authorization_group[20];

	if (m_nCount==0)
		str_users = "The users list is empty\n";
	else
	{
		for (int i=0; i<m_nCount; i++)
		{
			switch (m_pOperators[i]->GetAuthorization())
			{
				case SUPER: strcpy(authorization_group, "administrator");break;
				case ORDINARY: strcpy(authorization_group, "operator");break;
				case GUEST: strcpy(authorization_group, "moderator");break;
				case AUTH_OPERATOR: strcpy(authorization_group, "attendant");break;
			}
			snprintf(msg, sizeof(msg), "User name = %s Authorization = %s\n", m_pOperators[i]->GetLogin().c_str(), authorization_group);

			str_users += msg;
		}
	}

	return str_users;
}

bool  COperatorList::GetIsListCorrupted() const
{
	return m_isListCorrupted;
}

bool  COperatorList::GetIsDefaultUserExists() const
{
	return m_isDefaultUserExists;
}

void COperatorList::UpdateIsDefaultUserExists()
{
	int ind = FindLogin(GetCustomUserLogin().c_str());

	m_isDefaultUserExists = ( (NOT_FOUND != ind)
	                          ?
	                          true : false );
}

DWORD COperatorList::GetNumOfPassedHours(CStructTm &otherTime) const
{
	DWORD passedHours = 0;

	CStructTm curTime;
	SystemGetTime(curTime);

	if (curTime > otherTime )
	{
		int theDenominator = 60/*minutes*/ * 60/*seconds*/;

		DWORD diffTime	= curTime - otherTime ;
		passedHours		= diffTime / theDenominator;
	}

	return passedHours;
}

void COperatorList::CheckAndCancelInactiveAccounts()
{
	FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckAndCancelInactiveAccounts ,list length:" << m_nCount;

	COperator* pOperator = NULL;

	// VNGR-10886
	// sort the operators according to DaysSinceLastLogin (larger values comes first)
	// -----------------------------------------------------------------------------
	std::vector< COperator *> vect;

	for (int i=0; i<m_nCount; i++)
	{
		pOperator = m_pOperators[i];
		if (NULL != pOperator)
		{
			vect.push_back(pOperator);
		}
	}

	std::sort(vect.begin(), vect.end(), SDescendingOperatorSort());
	// -----------------------------------------------------------------------------

	for (int i=0; i<(int)vect.size(); i++)
	{
		pOperator = vect[i];

		if (NULL != pOperator)
		{
			FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckAndCancelInactiveAccounts ,name:" << pOperator->GetLogin() << " index:" << i;
			if(!pOperator->IsDisabled())
			{
				DWORD daysFromLastLogin = pOperator->GetDaysSinceLastLogin();
				DWORD daysFromlastEnabled = pOperator->GetDaysSinceLastEnabledUser();

				CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				BOOL bJitcMode;
				sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

				DWORD cgfMaxInactiveDays = 0;
				sysConfig->GetDWORDDataByKey(CFG_KEY_DISABLE_INACTIVE_USER, cgfMaxInactiveDays);

				FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckAndCancelInactiveAccounts , " << pOperator->GetLogin() << "Last login was " << daysFromLastLogin << " days ago. Max Inactive is: " << cgfMaxInactiveDays
				<< "Last Enabled was " << daysFromlastEnabled;

				// cancel if needed
				if ( (0 != cgfMaxInactiveDays) && ( daysFromLastLogin >= cgfMaxInactiveDays)
					&& (daysFromlastEnabled >= cgfMaxInactiveDays))
				{
					char errMsg[256];
					memset(errMsg, 0, 256);

					std::string strLoginName = pOperator->GetLogin();
					//Only non-admin users can be disabled
					if (!IsLastEnabledAdministrator(strLoginName))
					{
						FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckAndCancelInactiveAccounts , " << pOperator->GetLogin() << " marked as an inactive account.";
						pOperator->SetDisabled(TRUE);

						// VNGR-10503
						// JITC: Audit Log does not record that an account was disabled due to inactivity
						// ------------------------------------------------------------------------------
						AUDIT_EVENT_HEADER_S outAuditHdr;
						CAuditorApi::PrepareAuditHeader(outAuditHdr,
														"",
														eMcms,
														"",
														"",
														eAuditEventTypeInternal,
														eAuditEventStatusOk,
														"A user account was locked",
														"User account: " + pOperator->GetLogin() + "  was disabled (locked) due to inactivity",
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
						// ------------------------------------------------------------------------------

						AuditDisableUser( pOperator->GetLogin() + " marked as an inactive account.");

					}
				}
			}
			else
				FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckAndCancelInactiveAccounts , " << pOperator->GetLogin() << " is disabled";
		}
		else
			FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckAndCancelInactiveAccounts , operator[" << i << "] == NULL !!! ";
	}
}

//Marks expired accounts as 'new account' - this forces the user to change password on his next login.
void COperatorList::CheckExpiredPasswords()
{
	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL bJitcMode;
	sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

	DWORD cgfPwdExpirationPeriod = 0;
	sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_EXPIRATION_DAYS, cgfPwdExpirationPeriod);

	DWORD cgfPwdExpirationPeriodMachine = 0; // psw expiration period for Machine Account users
	sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_EXPIRATION_DAYS_MACHINE, cgfPwdExpirationPeriodMachine);

	DWORD cgfPwdExpirationWarningPeriod = 0;
	sysConfig->GetDWORDDataByKey(CFG_KEY_PASSWORD_EXPIRATION_WARNING_DAYS, cgfPwdExpirationWarningPeriod);

	int nNumOfMachineAccountsToBeExpired = 0;

	COperatorListIterator itr(m_pOperators);
	COperator* pOperator = itr.FirstOperator();
	while (pOperator)
	{
		// expiration period different for MAchine Account users
		DWORD expirationPeriod = (pOperator->IsMachineAccount() == FALSE) ? cgfPwdExpirationPeriod : cgfPwdExpirationPeriodMachine;

		if(/*!pOperator->IsDisabled() &&*/ 0 != expirationPeriod)
		{
			DWORD passedDays = pOperator->GetDaysSinceLastPwdChange();

			if (passedDays >= expirationPeriod)
			{
				pOperator->SetForceChangePwd(TRUE);
				FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckExpiredPasswords , " << pOperator->GetLogin() << ": Password expiration date reached -  account was marked as a new account.";
			}

			else
			{
				DWORD Experation_date = ( expirationPeriod - passedDays);

				//If account is getting close to it's expiration date
				if (Experation_date <= ACCOUNT_IS_CLOSE_TO_EXPERATION_DATE)
				{
					FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckExpiredPasswords , password will expire in " << Experation_date << " days; Login name: " << pOperator->GetLogin() ;
				}

				if( pOperator->IsMachineAccount() == TRUE &&  Experation_date <= cgfPwdExpirationWarningPeriod )
					nNumOfMachineAccountsToBeExpired++;
			}
		}

		pOperator = itr.NextOperator();
	}

	// Start active alarm
	CProcessBase* pProcess = CProcessBase::GetProcess();
	pProcess->RemoveActiveAlarmFromProcess(AA_MACHINE_ACCOUNT_EXPIRATION_ALERT);

	if( nNumOfMachineAccountsToBeExpired )
	{
		char szTmp[16];
		sprintf(szTmp,"%d",nNumOfMachineAccountsToBeExpired);

		std::string sMessageAA = "Password of ";
		sMessageAA.append( (nNumOfMachineAccountsToBeExpired > 1) ? szTmp : "one" );
		sMessageAA.append(" of machine accounts will expire soon.");

		pProcess->AddActiveAlarmSingleToneFromProcess( FAULT_GENERAL_SUBJECT, AA_MACHINE_ACCOUNT_EXPIRATION_ALERT,
					MAJOR_ERROR_LEVEL, sMessageAA, true, true );
		FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckExpiredPasswords , nNumOfMachineAccountsToBeExpired=" << nNumOfMachineAccountsToBeExpired	<< ".";
	}

	SaveToFileDB();
}

// Marks expired accounts as 'new account' - this forces the user to change
// password on his next login
void COperatorList::CheckLockedAccounts(void)
{
	CSysConfig* cfg = CProcessBase::GetProcess()->GetSysConfig();
	FPASSERT_AND_RETURN(NULL == cfg);

	BOOL lockedout;
	BOOL res = cfg->GetBOOLDataByKey("USER_LOCKOUT", lockedout);
	FPASSERT_AND_RETURN(!res);

	if (!lockedout)
		return;

	DWORD timeframe;
	res = cfg->GetDWORDDataByKey(CFG_KEY_USER_LOCKOUT_DURATION_IN_MINUTES, timeframe);
	FPASSERT_AND_RETURN(!res);

	COperatorListIterator itr(m_pOperators);
	for (COperator* user = itr.FirstOperator(); user; user = itr.NextOperator())
	{
		if (!user->IsLocked())
		  continue;

    CStructTm curTime;
    SystemGetTime(curTime);

    DWORD minutes = (curTime - user->GetLockTime()) / 60;
    if (minutes >= 1 && IsLastAdministrator(*user))
    {
      UnlockUser(user->GetLogin());
      ResetLoginPwdFailures(user->GetLogin());
      FTRACEINTOFUNC << "Last administrator has been unlocked " << user->GetLogin();
    }

    if (timeframe)
    {
      // If passed more hours than required, and user is not locked yet, reset count
      if (minutes >= timeframe && timeframe != 45000)
      {
        UnlockUser(user->GetLogin());
        ResetLoginPwdFailures(user->GetLogin());
        FTRACEINTOFUNC << "User has been unlocked " << user->GetLogin();
      }
    }
	}
}

bool COperatorList::IsLastAdministrator(const COperator& admin) const
{
	COperatorListIterator itr(const_cast<COperator**>(m_pOperators));
	for (const COperator* user = itr.FirstOperator(); user; user = itr.NextOperator())
	{
	  if (user->GetLogin() == admin.GetLogin())
	    continue;

	  if (user->IsMachineAccount())
	    continue;

	  if (SUPER != user->GetAuthorization())
	    continue;

    if (user->IsDisabled())
      continue;

	  if (user->IsLocked())
	  {
	    // If there are couple of locked administrators takes only last locked
	    if (user->GetLockTime() < admin.GetLockTime())
	      continue;

	    FTRACEINTOFUNC << "User " << admin.GetLogin()
	                   << " with time " << admin.GetLockTime()
	                   << " is not last administrator, user "
	                   << user->GetLogin()
	                   << " with time " << user->GetLockTime()
	                   << " exists";
	  }
	  else
	  {
	    FTRACEINTOFUNC << "User " << admin.GetLogin()
	                   << " is not last administrator, user "
	                   << user->GetLogin() << " exists";
	  }

    return false;
	}

	FTRACEINTOFUNC << "User " << admin.GetLogin() << " is last administrator";

	return true;
}

BOOL COperatorList::IsLastEnabledAdministrator(std::string loginName)
{
	BOOL result = TRUE;

	COperatorListIterator itr(m_pOperators);
	COperator* pOperator = itr.FirstOperator();
	while(pOperator)
	{
		if(pOperator->GetLogin() != loginName)
		{
			if(pOperator->GetAuthorization() == SUPER && pOperator->IsDisabled() == FALSE && pOperator->IsMachineAccount() == FALSE)
			{
				result = FALSE;
				break;
			}
		}
		pOperator = itr.NextOperator();
	}

	return result;
}

std::string COperatorList::GetLastAdministrator()
{
	std::string result = "";

	COperatorListIterator itr(m_pOperators);
	COperator* pOperator = itr.FirstOperator();
	while(pOperator)
	{
		if(pOperator->GetAuthorization() == SUPER)
		{
			result = pOperator->GetLogin();
			break;
		}
		pOperator = itr.NextOperator();
	}

	return result;
}

void COperatorList::MarkAllOperatorsAsNewAccounts()
{
	COperatorListIterator itr(m_pOperators);
	COperator* pOperator = itr.FirstOperator();
	while(pOperator)
	{
		pOperator->SetForceChangePwd(TRUE);
		pOperator = itr.NextOperator();
	}

	SaveToFileDB();
}

BOOL COperatorList::WasPasswordUsed(std::string loginName, std::string newPwd)
{
	BOOL result = FALSE;

	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			result = m_pOperators[ind]->WasPasswordUsed(newPwd);
		}
	}
	return result;
}

STATUS COperatorList::CheckIfAtLeast4CharChanged(const char* old_password, const char* new_password)
{
	int diffChar = 0;

	for (int i=0; i<OPERATOR_NAME_LEN; i++)
	{
		if(old_password[i] == '\0' || new_password[i] == '\0')
			break;

		if (old_password[i]!=new_password[i])
			diffChar++;
	}

	if (diffChar<4)
		return STATUS_AT_LEAST_4_CHARACTERS_IN_PASSWORD_MUST_BE_CHANGED;

	return STATUS_OK;
}

void COperatorList::IncrementChangePwdFailures(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->IncrementChangePwdFailures();
			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
}

WORD COperatorList::GetChangePwdFailures(const std::string loginName)
{
	WORD result = 0;

	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
			result = m_pOperators[ind]->GetChangePwdFailures();
	}

	return result;
}

void COperatorList::ResetChangePwdFailures(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->ResetChangePwdFailures();
			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
}

void COperatorList::IncrementLoginPwdFailures(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->IncrementLoginPwdFailures();
			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
}

WORD COperatorList::GetLoginPwdFailures(const std::string loginName)
{
	WORD result = 0;

	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
			result = m_pOperators[ind]->GetLoginPwdFailures();
	}

	return result;
}

void COperatorList::ResetLoginPwdFailures(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->ResetLoginPwdFailures();
			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
}

void COperatorList::IncrementCountForceChangePwdNotifications(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->IncrementCountForceChangePwdNotifications();
			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
}

WORD COperatorList::GetCountForceChangePwdNotifications(const std::string loginName)
{
	WORD result = 0;

	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
			result = m_pOperators[ind]->GetCountForceChangePwdNotifications();
	}

	return result;
}

void COperatorList::ResetCountForceChangePwdNotifications(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->ResetCountForceChangePwdNotifications();
			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
}

void COperatorList::ResetLogInHistory(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->ResetLogInHistory();
			SaveToFileDB();//olga TODO?
		}
	}
}

void COperatorList::SetFailedLoginInfo(COperator* pOperInDB, const CStructTm loginTime, const std::string address)
{
    if(pOperInDB)
	{
		pOperInDB->SetFailedLoginInfo(loginTime, address);
		SaveToFileDB();
	}
}

void COperatorList::SetFirstFailureTime(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			CStructTm curTime;
			SystemGetTime(curTime);

			m_pOperators[ind]->SetFirstFailureTime(curTime);
			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
}

//Check if user is disabled.
//If it is disabled then:
//1. If account is disabled due to inactivity - it is left disabled
//2. If account is disabled due to other reason - check if lockout period is over.
//	 If lockout period is over - release the lockout
BOOL COperatorList::CheckUserDisabled(const std::string loginName)
{
	BOOL result = FALSE;
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
			result = m_pOperators[ind]->IsDisabled();

		//If operator is disabled, check that lockout duration did not pass
		if(result)
		{
			//First, check if account is disabled due to inactivity
			DWORD daysFromLastLogin = m_pOperators[ind]->GetDaysSinceLastLogin();

			BOOL bLastEnabledAdmin = IsLastEnabledAdministrator(loginName);

			//what is that for?
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			BOOL bJitcMode;
			sysConfig->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);

			DWORD cgfMaxInactiveDays = 0;
			sysConfig->GetDWORDDataByKey(CFG_KEY_DISABLE_INACTIVE_USER, cgfMaxInactiveDays);


			if(!bLastEnabledAdmin)
				FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckUserDisabled , " << m_pOperators[ind]->GetLogin() << " Last login was " << daysFromLastLogin << " days ago. Max Inactive is: " << cgfMaxInactiveDays;
			else
				FTRACESTR(eLevelInfoNormal) << "\nCOperatorList::CheckUserDisabled , " << m_pOperators[ind]->GetLogin() << " is last enabled admin.";

			//Account is not inactive - check if disable time is over
			/*	colinzuo VNGR-18480: confirmed with Assaf, if a user is disabled because
			 *	of inactivity, it will not be automatically enabled because flag
			 *	CFG_KEY_DISABLE_INACTIVE_USER is later changed to 0
			 */
			if ( bLastEnabledAdmin )
			  // ||	 0 == cgfMaxInactiveDays 
			  // || //if cgfMaxInactiveDays=0 than the feature is disabled so we release any disabled user
			  // cgfMaxInactiveDays > daysFromLastLogin )
			{
				EnableUser(loginName);
				result = FALSE;
			}

			//Account is not inactive - check if disable time is over
			if ( !bLastEnabledAdmin &&
				 0 != cgfMaxInactiveDays && //if cgfMaxInactiveDays=0 than the feature is disabled so we release any disabled user
				 cgfMaxInactiveDays <= daysFromLastLogin )
			{
				DisableUser(loginName, FALSE);
			}
		}
	}

	return result;
}

// Checks if user is locked
BOOL COperatorList::CheckUserLocked(const std::string loginName)
{
	CheckLockedAccounts();//VNGR-11089

	BOOL result = FALSE;
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
			result = m_pOperators[ind]->IsLocked();
	}

	return result;
}

//CAUTION: if bAllowDisableLastAdmin == TRUE then one enables to disable EVEN THE LAST ADMIN!!!!
BOOL COperatorList::DisableUser(const std::string loginName, BOOL bAllowDisableLastAdmin)
{
	BOOL result = FALSE;
	int ind = FindLogin(loginName.c_str());

	if(ind != NOT_FOUND && ind < MAX_OPERATORS_IN_MCU)
	{
		if (bAllowDisableLastAdmin || !IsLastEnabledAdministrator(loginName))
		{
			if(m_pOperators[ind])
			{
				m_pOperators[ind]->SetDisabled(TRUE);

				CStructTm curTime;
				SystemGetTime(curTime);
				m_pOperators[ind]->SetDisabledTime(curTime);
				result = TRUE;

				IncreaseUpdateCounter();
				SaveToFileDB();
			}
		}
	}
	else
		FPTRACE2(eLevelInfoNormal, "COperatorList::DisableUser: user not found", loginName.c_str());

	return result;
}

BOOL COperatorList::LockUser(const std::string& loginName)
{
	BYTE bJitcMode = IsFederalOn();
	BOOL result = FALSE;
	int ind = FindLogin(loginName.c_str());
	if(ind != NOT_FOUND && ind < MAX_OPERATORS_IN_MCU)
	{
		if (m_pOperators[ind])
		{
			if ((bJitcMode == FALSE ) && (IsLastAdministrator(*(m_pOperators[ind])) == true))
					return FALSE;  //BRIDGE-3743 do not lock the last admin

			m_pOperators[ind]->SetLock(TRUE);

			CStructTm curTime;
			SystemGetTime(curTime);
			m_pOperators[ind]->SetLockTime(curTime);
			result = TRUE;

			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
	else
		FPTRACE2(eLevelInfoNormal, "COperatorList::LockUser: user not found", loginName.c_str());

	return result;
}

void COperatorList::AuditDisableUser(const std::string description)
{
	//Audit
	AUDIT_EVENT_HEADER_S outAuditHdr;
	CAuditorApi::PrepareAuditHeader(outAuditHdr,
									"",
									eMcms,
									"",
									"",
									eAuditEventTypeInternal,
									eAuditEventStatusOk,
									"A user account was disabled",
									description,
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

void COperatorList::AuditLockUser(const std::string description)
{
	//Audit
	AUDIT_EVENT_HEADER_S outAuditHdr;
	CAuditorApi::PrepareAuditHeader(outAuditHdr,
									"",
									eMcms,
									"",
									"",
									eAuditEventTypeInternal,
									eAuditEventStatusOk,
									"A user account was locked",
									description,
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

void COperatorList::EnableUser(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->SetDisabled(FALSE);

			CStructTm curTime;
			m_pOperators[ind]->SetDisabledTime(curTime); //sets empty time for last disabled time
			SystemGetTime(curTime); //sets the last Enabled time for that user
			m_pOperators[ind]->SetEnabledTime(curTime);

			IncreaseUpdateCounter();
			SaveToFileDB();

			// IT 09/06/09
			// VNGR-10692 : Acct. disabled due to inactivity / Enable the account / Account does not become disabled again after 3 notifications of password
			// After enabling an account, make sure to force the account to change password is needed
			CheckExpiredPasswords();
		}
	}
}

void COperatorList::UnlockUser(const std::string loginName)
{
	int ind = FindLogin(loginName.c_str());
	if (NOT_FOUND != ind)
	{
		if(m_pOperators[ind])
		{
			m_pOperators[ind]->SetLock(FALSE);

			CStructTm curTime;
			m_pOperators[ind]->SetLockTime(curTime);

			IncreaseUpdateCounter();
			SaveToFileDB();
		}
	}
}

void COperatorList::SetTDDState(BOOL isFederal)
{
	m_isUnderTDD = TRUE;
	if(isFederal)
		m_isUnderFederalTDD = TRUE;
}
