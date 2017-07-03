
#include "ActiveAlarmsListGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "McuMngrProcess.h"
#include "FaultsContainer.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////////////
CActiveAlarmsListGet::CActiveAlarmsListGet()
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_id = 0;
}

/////////////////////////////////////////////////////////////////////////////
CActiveAlarmsListGet& CActiveAlarmsListGet::operator = (const CActiveAlarmsListGet &other)
{
	if(&other != this)
	{
		CSerializeObject::operator =(other);
		m_pProcess = other.m_pProcess;
		m_id = other.m_id;
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CActiveAlarmsListGet::~CActiveAlarmsListGet()
{
}

/////////////////////////////////////////////////////////////////////////////
CActiveAlarmsListGet::CActiveAlarmsListGet( const CActiveAlarmsListGet &other )
:CSerializeObject(other)
{
	m_pProcess = other.m_pProcess;
}

///////////////////////////////////////////////////////////////////////////
void CActiveAlarmsListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CFaultList* pActiveAlarmsList = m_pProcess->GetActiveAlarmsList();
	if (pActiveAlarmsList)
	{
		pActiveAlarmsList->SerializeXml(pActionsNode, m_updateCounter, m_id);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CActiveAlarmsListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	GET_VALIDATE_CHILD(pNode,"ID",&m_id,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////


