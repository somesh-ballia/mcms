#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ModuleContent.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"
#include "ProcessList.h"
#include "LoggerProcess.h"

CModuleContent::CModuleContent()
{
	m_isEnabled = 1;
	m_traceLevel = CLoggerProcess::eINFO;
	memset(m_moduleName,0, sizeof(m_moduleName));
}

CModuleContent::CModuleContent(const CModuleContent &other)
	 :CSerializeObject(other)
{
	
	m_processList = other.m_processList;
	m_isEnabled = other.m_isEnabled;
	m_traceLevel= other.m_traceLevel;
	strcpy(m_moduleName,other.m_moduleName);

	
}

/////////////////////////////////////////////////////////////////////////////
void	CModuleContent::CopyValue(CModuleContent* pOther)
{
	if (pOther == NULL || (this == pOther))
		return;

	m_processList.CopyValue(&(pOther->GetProcessList()));
	m_isEnabled = pOther->IsEnabled();
	m_traceLevel = pOther->GetLogLevel();
}

/////////////////////////////////////////////////////////////////////////////
CModuleContent& CModuleContent::operator = (const CModuleContent& other)
{
	if (this == &other)
		return *this;

	m_processList = other.m_processList;
	m_isEnabled = other.m_isEnabled;
	m_traceLevel = other.m_traceLevel;
	strcpy(m_moduleName,other.m_moduleName);
	
	

	return *this;
}

/////////////////////////////////////////////////////////////////////////////	
CModuleContent::~CModuleContent()
{
	//printf("destructor of CModuleContent\n");
}

/////////////////////////////////////////////////////////////////////////////	
void CModuleContent::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* pModuleNode = pFatherNode->AddChildNode("MODULE_INFO");

	pModuleNode->AddChildNode("MODULE_NAME",m_moduleName);
	pModuleNode->AddChildNode("LOG4CXX_LEVEL",m_traceLevel,LOG4CXX_LEVEL_ENUM);
	pModuleNode->AddChildNode("CHECKED",m_isEnabled,_BOOL);
	m_processList.SerializeXml(pModuleNode);

}

/////////////////////////////////////////////////////////////////////////////
int CModuleContent::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode, "MODULE_NAME", m_moduleName,_0_TO_31_STRING_LENGTH);
	GET_VALIDATE_CHILD(pActionNode, "LOG4CXX_LEVEL", &m_traceLevel ,LOG4CXX_LEVEL_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "CHECKED", &m_isEnabled,_BOOL);
	
	m_processList.DeSerializeXml(pActionNode, pszError, action);
	
	return nStatus;

}

/////////////////////////////////////////////////////////////////////////////
void	CModuleContent::SetEnabled(BYTE bEnabled)
{
	m_isEnabled= bEnabled;
}

/////////////////////////////////////////////////////////////////////////////
void	CModuleContent::SetLogLevel(DWORD level)
{
	m_traceLevel = level;
}

/////////////////////////////////////////////////////////////////////////////
void	CModuleContent::SetLModuleName(char* name)
{
  if (name)
  {
    strncpy(m_moduleName, name, sizeof(m_moduleName)-1);
    m_moduleName[sizeof(m_moduleName)-1] = '\0';
  }
  else
  {
    m_moduleName[0] = '\0';
  }
}

void	CModuleContent::SetProcessList(CProcessList processList)
{
	m_processList = processList;
	
}

/////////////////////////////////////////////////////////////////////////////
char* CModuleContent::GetModuleName()
{
	return m_moduleName;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CModuleContent::GetLogLevel()
{
	return m_traceLevel;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CModuleContent::IsEnabled()
{
	return m_isEnabled;
}

/////////////////////////////////////////////////////////////////////////////
CProcessList&	CModuleContent::GetProcessList()
{
	return m_processList;
}

/////////////////////////////////////////////////////////////////////////////
bool CModuleContent::ValidateLogMsg(int IndexInAllRmxProcesses,int log4cxx_level_of_the_trace,int src_id)
{
	if ((int)GetLogLevel() <= log4cxx_level_of_the_trace)
	{
		//int SpecifiedProcessInd = ConvertIndexInAllRmxProcessesToIndexOfProcessListInModule(IndexInAllRmxProcesses,m_moduleName);
		//printf("index %d is enabled? %s\n",SpecifiedProcessInd,m_processList.IsSpecifiedProcessChecked(SpecifiedProcessInd)?"yes":"no");
		return m_processList.IsSpecifiedProcessChecked(IndexInAllRmxProcesses,m_moduleName,src_id);
	}

	return false;
}


