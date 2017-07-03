// AuthenticationProcess.cpp

#include "AuthenticationProcess.h"
#include "OperatorList.h"
#include "CardsStructs.h"

extern void AuthenticationManagerEntryPoint(void* appParam);


//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CAuthenticationProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CAuthenticationProcess::GetManagerEntryPoint()
{
	return AuthenticationManagerEntryPoint;
}

//////////////////////////////////////////////////////////////////////
CAuthenticationProcess::CAuthenticationProcess()
{
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	
	m_pAuthenticationStruct = new MCMS_AUTHENTICATION_S;
	m_pAuthenticationStruct->productType     = (DWORD)curProductType;
	m_pAuthenticationStruct->switchBoardId    = 0;
	m_pAuthenticationStruct->switchSubBoardId = 1;
	m_pAuthenticationStruct->rmxSystemCardsMode	= (DWORD)( GetRmxSystemCardsModeDefault() );

	m_isPasswordFileRemoved = false;
	m_pLdapOperatorList = NULL;
	m_pOperatorList = NULL;
}

//////////////////////////////////////////////////////////////////////
CAuthenticationProcess::~CAuthenticationProcess()
{
	delete m_pAuthenticationStruct;
}

// Virtual
const char* CAuthenticationProcess::NameOf(void) const
{
    return GetCompileType();
}

// Virtual
eProcessType CAuthenticationProcess::GetProcessType(void)
{
    return eProcessAuthentication;
}

// Virtual
BOOL CAuthenticationProcess::UsingSockets(void)
{
    return NO;
}

// Virtual
int CAuthenticationProcess::GetProcessAddressSpace(void)
{
    return 64 * 1024 * 1024;
}

////////////////////////////////////////////////////////////////////////////
void CAuthenticationProcess::SetOperatorList(COperatorList *operatorList)
{
	m_pOperatorList = operatorList;
}

////////////////////////////////////////////////////////////////////////////
void CAuthenticationProcess::SetLdapOperatorList(COperatorList *ldapOperatorList)
{
	m_pLdapOperatorList = ldapOperatorList;
}

////////////////////////////////////////////////////////////////////////////
void CAuthenticationProcess::SetAuthenticationStruct(MCMS_AUTHENTICATION_S* authentStruct)
{
	m_pAuthenticationStruct->productType = authentStruct->productType;
}

////////////////////////////////////////////////////////////////////////////
MCMS_AUTHENTICATION_S*  CAuthenticationProcess::GetAuthenticationStruct() const
{
	return m_pAuthenticationStruct;
}

////////////////////////////////////////////////////////////////////////////
void CAuthenticationProcess::SetUpProcess()
{
	m_pOperatorList = NULL;
	m_pLdapOperatorList = NULL;
}

////////////////////////////////////////////////////////////////////////////
void CAuthenticationProcess::TearDownProcess()
{
	POBJDELETE(m_pOperatorList);
	POBJDELETE(m_pLdapOperatorList);
}

void CAuthenticationProcess::SetPasswordFileFlg(BYTE isPasswordFileRemoved)
{
	m_isPasswordFileRemoved = isPasswordFileRemoved;
}

BYTE CAuthenticationProcess::GetPasswordFileFlg()
{
	return m_isPasswordFileRemoved ;
}
