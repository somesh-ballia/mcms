#include "AppenderConfiguration.h"
#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ModuleContent.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "SystemFunctions.h"

CAppenderConfiguration::CAppenderConfiguration()
{
	m_isEnabled = 0;
	m_cliIP = 0;
	m_cliPort = 0;
}

CAppenderConfiguration::CAppenderConfiguration(const CAppenderConfiguration &other)
	 :CSerializeObject(other)
{

	for (unsigned int j = 0 ; j < m_moduleList.size() ;j++)
	{
		if (m_moduleList[j] != NULL)
			delete m_moduleList[j] ;
		m_moduleList[j] = NULL;
	}
	m_moduleList.clear();
	
	for (unsigned int i = 0 ; i < other.m_moduleList.size() ; i++)
	{
		CModuleContent* pModule = new CModuleContent();
		*pModule =  *(other.m_moduleList[i]);
		AddModule(pModule);
	}
	
	m_isEnabled = other.m_isEnabled;
	m_cliIP = other.m_cliIP;
	m_cliPort = other.m_cliPort;
}

CAppenderConfiguration::~CAppenderConfiguration()
{
	//printf("destructor of CAppenderConfiguration \n");
	for (unsigned int i = 0 ; i < m_moduleList.size() ; i++)
	{
		if (m_moduleList[i] != NULL)
			delete m_moduleList[i];
		m_moduleList[i] = NULL;
	}

	m_moduleList.clear();
}

void	CAppenderConfiguration::CopyValue(CAppenderConfiguration* pOther)
{
	vector<CModuleContent*>& pOtherModuleVec = pOther->GetModuleList();
	
	for (unsigned int i = 0 ; i < m_moduleList.size() ; i++)
	{
		if (pOtherModuleVec.size() <= i)
			break;
		
		m_moduleList[i]->CopyValue(pOtherModuleVec[i]);
		
	}
	
	m_isEnabled = pOther->IsEnabled();
	m_cliIP = pOther->GetIPIntValue();
	m_cliPort = pOther->GetPort();
}

CAppenderConfiguration& CAppenderConfiguration::operator= (const CAppenderConfiguration& other)
{
	if (this == &other)
		return *this;

	//printf("appenderConfiguratio operatot =\n");
	//m_moduleList = other.m_moduleList;
	for (unsigned int j = 0 ; j < m_moduleList.size() ;j++)
	{
		if (m_moduleList[j] != NULL)
			delete m_moduleList[j] ;
		m_moduleList[j] = NULL;
	}
	m_moduleList.clear();
	
	for (unsigned int i = 0 ; i < other.m_moduleList.size() ; i++)
	{
		CModuleContent* pModule = new CModuleContent();
		*pModule =  *(other.m_moduleList[i]);
		AddModule(pModule);
	}
	m_isEnabled = other.m_isEnabled;
	m_cliIP = other.m_cliIP;
	m_cliPort = other.m_cliPort;

	return *this;
}

void   CAppenderConfiguration::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pModuleListAppenderNode = pParentNode->AddChildNode("MODULE_LIST");

	for (unsigned int i = 0 ; i < m_moduleList.size() ; i++)
		m_moduleList[i]->SerializeXml(pModuleListAppenderNode);
}

 int  CAppenderConfiguration::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
 {
 	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pTempSonNode=NULL;
	CXMLDOMElement *pModuleInfoNode=NULL;
	
	GET_CHILD_NODE(pActionNode,"MODULE_LIST",pTempSonNode);
	//GET_CHILD_NODE(pActionNode, "MODULE_INFO", pModuleNode);
	if (pTempSonNode) {
		GET_FIRST_CHILD_NODE(pTempSonNode,"MODULE_INFO",pModuleInfoNode);
	}

	unsigned int i = 0;
	while(pModuleInfoNode)
	{
		if (i >= m_moduleList.size())
			return -1;
		
		m_moduleList[i]->DeSerializeXml(pModuleInfoNode, pszError, action);
		i++;
		GET_NEXT_CHILD_NODE(pTempSonNode, "MODULE_INFO", pModuleInfoNode);
	}
	
	return nStatus;
 }

void CAppenderConfiguration::SetEnabled(BYTE bEnabled)
{
	m_isEnabled = bEnabled;
}

BYTE CAppenderConfiguration::IsEnabled()
{
	return m_isEnabled;
}

void CAppenderConfiguration::SetIP(string	ip_addr)
{
	mcTransportAddress ipAddress;
	memset(&ipAddress, 0, sizeof(mcTransportAddress));
	stringToIp(&ipAddress, (char*)ip_addr.c_str(), eHost);
	
	m_cliIP = (DWORD)ipAddress.addr.v4.ip;
}

string CAppenderConfiguration::GetIP()
{
	mcTransportAddress trAddr;
	memset(&trAddr,0,sizeof(mcTransportAddress));
	trAddr.addr.v4.ip = m_cliIP;	
	trAddr.ipVersion = eIpVersion4;
	char tempName[64];
	memset (&tempName,'\0',64);

	ipToString(trAddr,tempName,0);
		
	return string(tempName);
}

DWORD	CAppenderConfiguration::GetIPIntValue()
{
	return m_cliIP;
}

void	CAppenderConfiguration::SetPort(DWORD port)
{
	m_cliPort = port;
}

DWORD CAppenderConfiguration::GetPort()
{
	return m_cliPort;
}

vector<CModuleContent*>&	CAppenderConfiguration::GetModuleList()
{
	return m_moduleList;
}

void	CAppenderConfiguration::SetMoudleList(vector<CModuleContent*> moduleList)
{
	m_moduleList = moduleList;
}

void	CAppenderConfiguration::AddModule(CModuleContent* module)
{
	m_moduleList.push_back(module);
}

CModuleContent*	CAppenderConfiguration::GetModuleByName(const char* moduleName)
{
	for (unsigned int i = 0 ; i < m_moduleList.size() ; i++)
	{
		if (strcmp(m_moduleList[i]->GetModuleName(),moduleName) == 0)
			return m_moduleList[i];
	}

	return NULL;
}
