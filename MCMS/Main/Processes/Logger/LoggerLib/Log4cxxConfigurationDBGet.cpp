#include "Log4cxxConfigurationDBGet.h"
#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ModuleContent.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "Log4cxxConfiguration.h"

CLog4cxxConfigurationDBGet::CLog4cxxConfigurationDBGet()
{
	m_loggerProcess = (CLoggerProcess*)CLoggerProcess::GetProcess();
}

CLog4cxxConfigurationDBGet::CLog4cxxConfigurationDBGet(const CLog4cxxConfigurationDBGet &other)
	 :CSerializeObject(other)
{
	
	
}

CLog4cxxConfigurationDBGet::~CLog4cxxConfigurationDBGet()
{
	
}

CLog4cxxConfigurationDBGet& CLog4cxxConfigurationDBGet::operator = (const CLog4cxxConfigurationDBGet& other)
{
	if (this == &other)
		return *this;

	return *this;
}

void   CLog4cxxConfigurationDBGet::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CLog4cxxConfiguration* pConfiguration = m_loggerProcess->GetLog4cxxConfiguration();
	pConfiguration->SerializeXml(pParentNode);		
}

 int  CLog4cxxConfigurationDBGet::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
 {
 	int nStatus = STATUS_OK;
	
	
	
	return nStatus;
 }



