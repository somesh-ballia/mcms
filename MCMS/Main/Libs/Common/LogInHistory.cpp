// LogInConfirm.cpp: implementation of the CLogInHistory class.
//
//////////////////////////////////////////////////////////////////////
#include "string.h"
#include "LogInHistory.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"



void LOGIN_RECORD_S::Init() {
  login_time.m_day =0;
  login_time.m_hour=0;
  login_time.m_min =0;
  login_time.m_mon =0;
  login_time.m_sec =0;
  login_time.m_year=0;

  address = "";
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogInHistory::CLogInHistory()
{
    m_NumberOfFailedLogin = 0;
	m_lastLogin.Init();
	for(DWORD i = 0; i < MAX_FAILED_LOGINS; i++)
	    m_failedLogins[i].Init();
}
/////////////////////////////////////////////////////////////////////////////
CLogInHistory::CLogInHistory(const CLogInHistory &other) : CSerializeObject(other)
{
    m_NumberOfFailedLogin = other.m_NumberOfFailedLogin;

    m_lastLogin.login_time = other.m_lastLogin.login_time;
	m_lastLogin.address = other.m_lastLogin.address;

	for (int i=0; i < MAX_FAILED_LOGINS; i++)
	{
	    m_failedLogins[i].login_time = other.m_failedLogins[i].login_time;
		m_failedLogins[i].address = other.m_failedLogins[i].address;
	}
}
/////////////////////////////////////////////////////////////////////////////
CLogInHistory::~CLogInHistory()
{}


/////////////////////////////////////////////////////////////////////////////
/*const CLogInHistory& CLogInHistory::operator=(const CLogInHistory& other)
{
    if(this == &other)
    {
        return *this;
    }

    m_NumberOfFailedLogin = other.m_NumberOfFailedLogin;

    m_lastLogin.login_time = other.m_lastLogin.login_time;
	m_lastLogin.address = other.m_lastLogin.address;

	for (int i=0; i < MAX_FAILED_LOGINS; i++)
	{
	    m_failedLogins[i].login_time = other.m_failedLogins[i].login_time;
		m_failedLogins[i].address = other.m_failedLogins[i].address;
	}

    return *this;
}
*/
/////////////////////////////////////////////////////////////////////////////
void CLogInHistory::SerializeXml(CXMLDOMElement*& pXMLResponse) const
{
//     FTRACESTR(eLevelInfoNormal) << "\CLogInHistory::SerializeXml : " << m_lastLogin.login_time;
    CXMLDOMElement *pLoginRecords, *pLastLogin,*pLoginRecord, *pFailedLogins;
	pLoginRecords = pXMLResponse->AddChildNode("LOGIN_RECORDS");
	if(pLoginRecords) {
	    pLastLogin = pLoginRecords->AddChildNode("LAST_LOGIN");
		if(pLastLogin) {
		    pLoginRecord = pLastLogin->AddChildNode("LOGIN_RECORD");
			if(pLoginRecord) {
			    pLoginRecord->AddChildNode("DATE", m_lastLogin.login_time);
				pLoginRecord->AddChildNode("IP_ADDRESS", m_lastLogin.address);
			}
		}
		
		pFailedLogins = pLoginRecords->AddChildNode("FAILED_LOGINS");
		if(pFailedLogins) {
		    for(DWORD i = 0; i < m_NumberOfFailedLogin && i < MAX_FAILED_LOGINS; i++) {
			    pLoginRecord = pFailedLogins->AddChildNode("LOGIN_RECORD");
				if(pLoginRecord) {
				    pLoginRecord->AddChildNode("DATE", m_failedLogins[i].login_time);
					pLoginRecord->AddChildNode("IP_ADDRESS", m_failedLogins[i].address);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
int CLogInHistory::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
    CXMLDOMElement *pLoginRecords = NULL, *pLastLogin = NULL, *pLoginRecord = NULL, *pFailedLogins = NULL;

	GET_CHILD_NODE(pActionNode, "LOGIN_RECORDS", pLoginRecords);
	if(pLoginRecords) {

	    GET_CHILD_NODE(pLoginRecords, "LAST_LOGIN", pLastLogin);
		if(pLastLogin) {

		    GET_CHILD_NODE(pLastLogin, "LOGIN_RECORD", pLoginRecord);
			if(pLoginRecord) {
			    GET_VALIDATE_CHILD(pLoginRecord, "DATE", &m_lastLogin.login_time, DATE_TIME);
				GET_VALIDATE_CHILD(pLoginRecord, "IP_ADDRESS", m_lastLogin.address, ONE_LINE_BUFFER_LENGTH);
			}
		}
		
		GET_CHILD_NODE(pLoginRecords, "FAILED_LOGINS", pFailedLogins);

		if(pFailedLogins) {

		    GET_FIRST_CHILD_NODE(pFailedLogins, "LOGIN_RECORD", pLoginRecord);

			m_NumberOfFailedLogin = 0;

			while(pLoginRecord && m_NumberOfFailedLogin < MAX_FAILED_LOGINS)
			{
			    GET_VALIDATE_CHILD(pLoginRecord, "DATE", &m_failedLogins[m_NumberOfFailedLogin].login_time, DATE_TIME);
				GET_VALIDATE_CHILD(pLoginRecord, "IP_ADDRESS", m_failedLogins[m_NumberOfFailedLogin].address, ONE_LINE_BUFFER_LENGTH);

				m_NumberOfFailedLogin++;

				GET_NEXT_CHILD_NODE(pFailedLogins, "LOGIN_RECORD", pLoginRecord);
			}
		}
	}
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
CStructTm  CLogInHistory::GetLastLogin() const   
{
    return m_lastLogin.login_time;
}
/////////////////////////////////////////////////////////////////////////////
void  CLogInHistory::SetLastLogin(const CStructTm last_login)
{
	m_lastLogin.login_time = last_login;
}
/////////////////////////////////////////////////////////////////////////////
std::string CLogInHistory::GetLastLoginIPaddress() const
{
	return m_lastLogin.address;
}
/////////////////////////////////////////////////////////////////////////////
void  CLogInHistory::SetLastLoginIPaddress(const std::string address)
{
  //memcpy(&m_lastLogin.address, address, IPV6_ADDRESS_LEN);
    m_lastLogin.address = address;
}
/////////////////////////////////////////////////////////////////////////////
void  CLogInHistory::SetFailedLoginInfo(const CStructTm login, const std::string address)
{
    if(m_NumberOfFailedLogin < MAX_FAILED_LOGINS) {
	    m_failedLogins[m_NumberOfFailedLogin].login_time = login;
		m_failedLogins[m_NumberOfFailedLogin].address = address;

	} else {
	    for (int i=1; i < MAX_FAILED_LOGINS; i++)
		{
		    m_failedLogins[i-1].login_time = m_failedLogins[i].login_time;
			m_failedLogins[i-1].address = m_failedLogins[i].address;
		}
	    m_failedLogins[MAX_FAILED_LOGINS-1].login_time = login;
		m_failedLogins[MAX_FAILED_LOGINS-1].address = address;
	}
	m_NumberOfFailedLogin++;
}

/////////////////////////////////////////////////////////////////////////////
void  CLogInHistory::ResetLogInHistory()
{
    //FTRACESTR(eLevelInfoNormal) << "\nCLogInHistory::ResetLogInHistory : lastLogin before reset =" << m_lastLogin.login_time;
    m_NumberOfFailedLogin = 0;
	for(DWORD i = 0; i < MAX_FAILED_LOGINS; i++)
	    m_failedLogins[i].Init();
}

/////////////////////////////////////////////////////////////////////////////
BOOL  CLogInHistory::IsFirstLogin() const
{
    if( "" == m_lastLogin.address &&
		0 == m_lastLogin.login_time.m_day && 0 == m_lastLogin.login_time.m_year)
	    return TRUE;

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CStructTm	 CLogInHistory::GetLastFailedLoginInList()const
{
	if (0 == m_NumberOfFailedLogin)
	{
		CStructTm defaultTime;
		defaultTime.InitDefaults();
		return defaultTime;
	}
	else
		return m_failedLogins[m_NumberOfFailedLogin-1].login_time;
}

/////////////////////////////////////////////////////////////////////////////

