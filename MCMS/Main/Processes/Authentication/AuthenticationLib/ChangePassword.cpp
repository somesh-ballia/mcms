// ChangePassword.cpp: implementation of the CChangePassword class.
//
//////////////////////////////////////////////////////////////////////

#include "ChangePassword.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CChangePassword::CChangePassword()
{
}

/////////////////////////////////////////////////////////////////////////////
CChangePassword::CChangePassword(const CChangePassword &other) : CSerializeObject(other)
{
	m_strLogin = other.m_strLogin;
	m_strNewPassword = other.m_strNewPassword;
	m_strOldPassword = other.m_strOldPassword;
}

/////////////////////////////////////////////////////////////////////////////
CChangePassword::~CChangePassword()
{
}

/////////////////////////////////////////////////////////////////////////////
int CChangePassword::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, const char* action)
{
	int nStatus;
	CXMLDOMElement *pMCUNode=NULL;

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"USER_NAME",m_strLogin,_1_TO_OPERATOR_NAME_LENGTH);

	GET_VALIDATE_MANDATORY_CHILD(pActionNode,"NEW_PASSWORD",m_strNewPassword,_0_TO_OPERATOR_NAME_LENGTH);

	GET_VALIDATE_CHILD(pActionNode,"OLD_PASSWORD",m_strOldPassword,_0_TO_OPERATOR_NAME_LENGTH);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CChangePassword::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
} 

void CChangePassword::SetLogin(const std::string strLogin)
{
	m_strLogin = strLogin;
}

const std::string& CChangePassword::GetLogin () const
{
	return m_strLogin;
}

void CChangePassword::SetNewPassword(const std::string strNewPassword)
{
	m_strNewPassword = strNewPassword;
}

const std::string& CChangePassword::GetNewPassword () const
{
	return m_strNewPassword;
}

void CChangePassword::SetOldPassword(const std::string strOldPassword)
{
	m_strOldPassword = strOldPassword;
}

const std::string& CChangePassword::GetOldPassword () const
{
	return m_strOldPassword;
}

