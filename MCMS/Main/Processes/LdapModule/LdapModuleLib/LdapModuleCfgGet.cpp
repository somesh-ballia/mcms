// CLdapModuleCfgGet.cpp: implementation of the CLdapModuleCfgGet class.
//////////////////////////////////////////////////////////////////////////


#include "LdapModuleCfgGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "LdapModuleCfg.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLdapModuleCfgGet::CLdapModuleCfgGet()
{
	m_pProcess = (CLdapModuleProcess*)CLdapModuleProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CLdapModuleCfgGet& CLdapModuleCfgGet::operator = (const CLdapModuleCfgGet &other)
{
	if(this == &other){
	    return *this;
	}

	m_pProcess = (CLdapModuleProcess*)CLdapModuleProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CLdapModuleCfgGet::~CLdapModuleCfgGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CLdapModuleCfgGet::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CLdapModuleCfg* ldapCfg = ::GetLdapModuleCfg();
	ldapCfg->SerializeXml(pFatherNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CLdapModuleCfgGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;	
	return nStatus;
}
