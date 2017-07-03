#include <vector>
#include <ostream>
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "ProcessItemData.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"

/////////////////////////////////////////////////////////////////////////////
CProcessItemData::CProcessItemData()
{
		m_bEnabled = 1;
		m_iProcessame = 0;
}

//////////////////////////////////////////////////////////////////////////
CProcessItemData::~CProcessItemData()
{

}

//////////////////////////////////////////////////////////////////////////
CProcessItemData::CProcessItemData(const BYTE isEnabled,const DWORD processName)
{
	m_bEnabled = isEnabled;
	m_iProcessame = processName;
}

//////////////////////////////////////////////////////////////////////////
CProcessItemData::CProcessItemData(const CProcessItemData &other)
        : CSerializeObject(other)
{
    	*this = other;
}

//////////////////////////////////////////////////////////////////////////
CProcessItemData& CProcessItemData::operator= (const CProcessItemData& other)
{
	if (this == &other)
		return*this;
	
	m_bEnabled = other.m_bEnabled;
	m_iProcessame = other.m_iProcessame;
	return *this;
}

//////////////////////////////////////////////////////////////////////////
void CProcessItemData::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("ALL_MODULE_PROCESS_NAME",m_iProcessame,RMX_ALL_PROCESSES_ENUM);
	pFatherNode->AddChildNode("CHECKED",m_bEnabled,_BOOL);
}

/*void CProcessItemData::SerializeXml(CXMLDOMElement *pFatherNode, DWORD processName,BYTE bEnabled)
{
	m_bEnabled = bEnabled;
	m_iProcessame = 	processName;
	pFatherNode->AddChildNode("ALL_MODULE_PROCESS_NAME",processName,RMX_ALL_PROCESSES_ENUM);
	pFatherNode->AddChildNode("CHECKED",bEnabled,_BOOL);
}*/

/////////////////////////////////////////////////////////////////////////////
int CProcessItemData::DeSerializeXml(CXMLDOMElement* pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionNode, "ALL_MODULE_PROCESS_NAME", &m_iProcessame,RMX_ALL_PROCESSES_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "CHECKED", &m_bEnabled,_BOOL);
	
	
	return nStatus;
}

/////////////////////////////////////////////////////////////////////////////
void	CProcessItemData::SetEnabled(BYTE bEnabled)
{
	m_bEnabled = bEnabled;
}

/////////////////////////////////////////////////////////////////////////////
void	CProcessItemData::SetName(DWORD name)
{
	m_iProcessame = name;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CProcessItemData::IsEnabled() const
{
	return	m_bEnabled;
}

/////////////////////////////////////////////////////////////////////////////
DWORD CProcessItemData::GetName() const
{
	return m_iProcessame;
}


