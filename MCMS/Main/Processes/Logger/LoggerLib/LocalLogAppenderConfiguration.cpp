#include "LocalLogAppenderConfiguration.h"
#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ModuleContent.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"


CLocalLogAppenderConfiguration::CLocalLogAppenderConfiguration()
{
	SetEnabled(1);
}

CLocalLogAppenderConfiguration::CLocalLogAppenderConfiguration(const CLocalLogAppenderConfiguration &other)
	 :CAppenderConfiguration(other)
{
	
	
	
}

CLocalLogAppenderConfiguration::~CLocalLogAppenderConfiguration()
{
	
}

CLocalLogAppenderConfiguration& CLocalLogAppenderConfiguration::operator=(const CLocalLogAppenderConfiguration& other)
{
	if (this == &other)
		return *this;

	CAppenderConfiguration::operator=(other);
	return *this;
}

void   CLocalLogAppenderConfiguration::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pLocalLogAppenderNode = pParentNode->AddChildNode("LOCAL_APPENDER");
	pLocalLogAppenderNode->AddChildNode("CHECKED",m_isEnabled,_BOOL);

	CAppenderConfiguration::SerializeXml(pLocalLogAppenderNode);
}

int  CLocalLogAppenderConfiguration::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
{
 	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pLocalLogAppenderNode=NULL ;
	GET_CHILD_NODE(pActionNode, "LOCAL_APPENDER", pLocalLogAppenderNode);
	GET_VALIDATE_CHILD(pLocalLogAppenderNode, "CHECKED", &m_isEnabled,_BOOL);
	
	CAppenderConfiguration::DeSerializeXml(pLocalLogAppenderNode,pszError,action);
	
	return nStatus;
}



