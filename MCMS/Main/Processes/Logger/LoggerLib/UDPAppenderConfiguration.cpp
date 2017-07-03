#include "UDPAppenderConfiguration.h"
#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ModuleContent.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "SystemFunctions.h"
#include "LoggerProcess.h"

CUDPAppenderConfiguration::CUDPAppenderConfiguration()
{
	m_duration = CLoggerProcess::e48Hour;
}

CUDPAppenderConfiguration::CUDPAppenderConfiguration(const CUDPAppenderConfiguration &other)
	 :CAppenderConfiguration(other)
{
	
	m_duration = other.m_duration;
}

CUDPAppenderConfiguration::~CUDPAppenderConfiguration()
{
	//printf("destructor of CUDPAppenderConfiguration( \n");
}

CUDPAppenderConfiguration& CUDPAppenderConfiguration::operator = (const CUDPAppenderConfiguration& other)
{
	if (this == &other)
		return *this;

	//printf("udp operatot =\n");
	CAppenderConfiguration::operator =(other);
	m_duration = other.m_duration;
	
	return *this;
}

void   CUDPAppenderConfiguration::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pUDPAppenderNode = pParentNode->AddChildNode("UDP_APPENDER");
	pUDPAppenderNode->AddChildNode("CHECKED",m_isEnabled,_BOOL);
	pUDPAppenderNode->AddChildNode("IP",m_cliIP,IP_ADDRESS);
	pUDPAppenderNode->AddChildNode("PORT",m_cliPort);
	pUDPAppenderNode->AddChildNode("REMOTE_VIEWER_DURATION",m_duration,UDP_SEND_DURATION_ENUM);
	CAppenderConfiguration::SerializeXml(pUDPAppenderNode);
}

 int  CUDPAppenderConfiguration::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
 {
 	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pUDPAppenderNode=NULL, *pTempNode=NULL, *pTempSonNode=NULL;
	GET_CHILD_NODE(pActionNode, "UDP_APPENDER", pUDPAppenderNode);

	GET_VALIDATE_CHILD(pUDPAppenderNode, "CHECKED", &m_isEnabled,_BOOL);
	GET_VALIDATE_CHILD(pUDPAppenderNode, "IP", &m_cliIP,IP_ADDRESS);
	GET_VALIDATE_CHILD(pUDPAppenderNode, "PORT", &m_cliPort,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pUDPAppenderNode, "REMOTE_VIEWER_DURATION", &m_duration,UDP_SEND_DURATION_ENUM);

	CAppenderConfiguration::DeSerializeXml(pUDPAppenderNode,pszError,action);
	
	return nStatus;
 }


int	CUDPAppenderConfiguration::GetDuration()
{
	switch(m_duration)
	{
		case CLoggerProcess::e1Hour:
			return 1;
		case CLoggerProcess::e2Hour:
			return 2;
		case CLoggerProcess::e4Hour:
			return 4;
		case CLoggerProcess::e6Hour:
			return 6;
		case CLoggerProcess::e8Hour:
			return 8;
		case CLoggerProcess::e12Hour:
			return 12;
		case CLoggerProcess::e16Hour:
			return 16;
		case CLoggerProcess::e24Hour:
			return 24;
		case CLoggerProcess::e48Hour:
			return 48;

		default:
			return 1;
	}

	return 1;
}

void	CUDPAppenderConfiguration::SetDuration(DWORD duration)
{
	switch(duration)
	{
		case 1:
			m_duration = CLoggerProcess::e1Hour;
		break;
		
		case 2:
			m_duration = CLoggerProcess::e2Hour;
		break;
		
		case 4:
			m_duration = CLoggerProcess::e4Hour;
		break;
		
		case 6:
			m_duration = CLoggerProcess::e6Hour;
		break;
		
		case 8:
			m_duration = CLoggerProcess::e8Hour;
		break;
		
		case 12:
			m_duration = CLoggerProcess::e12Hour;
		break;

		case 16:
			m_duration = CLoggerProcess::e16Hour;
		break;
		
		case 24:
			m_duration = CLoggerProcess::e24Hour;
		break;
		
		case 48:
			m_duration = CLoggerProcess::e48Hour;
		break;
		
		default:
			m_duration = CLoggerProcess::e1Hour;
		break;
		
	}

}

