#include "SysLogAppenderConfiguration.h"
#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ModuleContent.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "SystemFunctions.h"


CSysLogAppenderConfiguration::CSysLogAppenderConfiguration()
{

}

CSysLogAppenderConfiguration::CSysLogAppenderConfiguration(const CSysLogAppenderConfiguration &other)
	 :CAppenderConfiguration(other)
{
	
}

CSysLogAppenderConfiguration::~CSysLogAppenderConfiguration()
{
	
}

CSysLogAppenderConfiguration& CSysLogAppenderConfiguration::operator = (const CSysLogAppenderConfiguration& other)
{
	if (this == &other)
		return *this;

	CAppenderConfiguration::operator =(other);
	
	return *this;
}

void   CSysLogAppenderConfiguration::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pSysLogAppenderNode = pParentNode->AddChildNode("SYSLOG_APPENDER");
	pSysLogAppenderNode->AddChildNode("CHECKED",m_isEnabled,_BOOL);
	pSysLogAppenderNode->AddChildNode("IP",m_cliIP,IP_ADDRESS);
	pSysLogAppenderNode->AddChildNode("PORT",m_cliPort);

	CAppenderConfiguration::SerializeXml(pSysLogAppenderNode);
}

 int  CSysLogAppenderConfiguration::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
 {
 	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pSysLogAppenderNode=NULL, *pTempNode=NULL, *pTempSonNode=NULL;
	GET_CHILD_NODE(pActionNode, "SYSLOG_APPENDER", pSysLogAppenderNode);

	
	GET_VALIDATE_CHILD(pSysLogAppenderNode, "CHECKED", &m_isEnabled,_BOOL);
	GET_VALIDATE_CHILD(pSysLogAppenderNode, "IP", &m_cliIP,IP_ADDRESS);
	GET_VALIDATE_CHILD(pSysLogAppenderNode, "PORT", &m_cliPort,_0_TO_DWORD);
	
	CAppenderConfiguration::DeSerializeXml(pSysLogAppenderNode,pszError,action);
	
	return nStatus;
 }


