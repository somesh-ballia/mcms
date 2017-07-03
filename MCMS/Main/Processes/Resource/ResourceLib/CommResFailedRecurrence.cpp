#include "CommResFailedRecurrence.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "FaultsDefines.h"
#include "TraceStream.h"
#include "ProcessBase.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CCommResFailedRecurrence::CCommResFailedRecurrence()
{
    m_H243confName[0] = '\0';
    m_status = 0;
}
//////////////////////////////////////////////////////////////////////////////

CCommResFailedRecurrence::CCommResFailedRecurrence(const CCommResFailedRecurrence& other)
:CSerializeObject(other)
{
    *this = other;
}
//////////////////////////////////////////////////////////////////////////////
CCommResFailedRecurrence::CCommResFailedRecurrence(CStructTm& StartTime, char* pszName, STATUS status)
{
	SetStartTime(StartTime);
	SetName(pszName);
    m_status = status;
}
//////////////////////////////////////////////////////////////////////////////
CCommResFailedRecurrence& CCommResFailedRecurrence::operator = (const CCommResFailedRecurrence &other)
{
	SetStartTime(*(other.GetStartTime()));
    SetName(other.GetName());
    m_status    = other.GetStatus();
    return *this;
}
///////////////////////////////////////////////////////////////////////////////
const char*  CCommResFailedRecurrence::NameOf() const
{
    return "CCommResFailedRecurrence";
}
///////////////////////////////////////////////////////////////////////////////
void  CCommResFailedRecurrence::SetStartTime(const CStructTm &other)
{
    m_startTime = other;
}
//////////////////////////////////////////////////////////////////////////////
const CStructTm*  CCommResFailedRecurrence::GetStartTime() const
{
    return &m_startTime;
}
//////////////////////////////////////////////////////////////////////////////
void  CCommResFailedRecurrence::SetName(const char*  name)
{

	strncpy(m_H243confName, name, sizeof(m_H243confName) - 1);
	m_H243confName[sizeof(m_H243confName) - 1]='\0';
}
//////////////////////////////////////////////////////////////////////////////
const char*  CCommResFailedRecurrence::GetName () const
{
    return m_H243confName;
}
//////////////////////////////////////////////////////////////////////////////
void  CCommResFailedRecurrence::SetStatus(DWORD status)
{
    m_status = status;
}
//////////////////////////////////////////////////////////////////////////////
DWORD CCommResFailedRecurrence::GetStatus() const
{
    return m_status;
}
///////////////////////////////////////////////////////////////////////////////
void CCommResFailedRecurrence::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	if(!pFatherNode)
		return;
	
	CXMLDOMElement *pRecurNode = pFatherNode->AddChildNode("RECURRENCE");	
	pRecurNode->AddChildNode("NAME",m_H243confName);

	CXMLDOMElement *pStatusNode = pRecurNode->AddChildNode("RETURN_STATUS");
	pStatusNode->AddChildNode("ID",m_status);
	pStatusNode->AddChildNode("DESCRIPTION",CProcessBase::GetProcess()->GetStatusAsString(m_status).c_str());

	pRecurNode->AddChildNode("START_TIME",m_startTime);
}
///////////////////////////////////////////////////////////////////////////////
int	CCommResFailedRecurrence::DeSerializeXml(CXMLDOMElement *pRecurrenceNode, char *pszError, const char* action)
{
	TRACESTR(eLevelError) << "CCommResFailedRecurrence::DeSerializeXml This Code Should not be invoked !!!!! " ;
	PASSERT(1);
	return STATUS_FAIL;
}
