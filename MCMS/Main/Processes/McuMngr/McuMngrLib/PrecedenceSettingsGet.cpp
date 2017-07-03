

#include "PrecedenceSettingsGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "PrecedenceSettings.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CPrecedenceSettingsGet::CPrecedenceSettingsGet()
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CPrecedenceSettingsGet& CPrecedenceSettingsGet::operator = (const CPrecedenceSettingsGet &other)
{
	if (this == &other)
	{
		//avoid assigning to oneself (might lead to data loss in cases of heap allocations)
		return *this;
	}
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CPrecedenceSettingsGet::~CPrecedenceSettingsGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CPrecedenceSettingsGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
//	CLicensing* pLicense = m_pProcess->GetLicensing();
//
//	if (pLicense)
//	{
//		pLicense->SerializeXml(pActionsNode);
//	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CPrecedenceSettingsGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	//GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD); Not implemented yet
	return nStatus;
}
