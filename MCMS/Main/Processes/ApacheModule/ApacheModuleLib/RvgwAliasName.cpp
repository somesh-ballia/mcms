/*
 * RvgwAliasName.cpp
 *
 *  Created on: Mar 15, 2012
 *      Author: stanny
 */


#include "RvgwAliasName.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "ApacheModuleEngine.h"
CRvgwAliasName::CRvgwAliasName()
{
	m_strAliasName = "";
}

CRvgwAliasName::CRvgwAliasName(const CRvgwAliasName &other): CSerializeObject(other)
{
	m_strAliasName = other.m_strAliasName;
}

CRvgwAliasName& CRvgwAliasName::operator= (const CRvgwAliasName &other)
{
	m_strAliasName = other.m_strAliasName;
	return *this;
}

CRvgwAliasName::~CRvgwAliasName()
{

}

void CRvgwAliasName::SerializeXml(CXMLDOMElement*& pActionsNode) const
{

}
int CRvgwAliasName::DeSerializeXml(CXMLDOMElement* pActionNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	GET_VALIDATE_CHILD(pActionNode,"GATEWAY_ALIAS_NAME",m_strAliasName,_1_TO_NEW_FILE_NAME_LENGTH);

	return nStatus;
}


