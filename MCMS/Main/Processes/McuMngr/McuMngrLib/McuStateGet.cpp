// CMcuStateGet.cpp: implementation of the CMcuStateGet class.
//////////////////////////////////////////////////////////////////////////


#include "McuStateGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "McuState.h"



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMcuStateGet::CMcuStateGet()
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CMcuStateGet& CMcuStateGet::operator = (const CMcuStateGet &other)
{
	if(this == &other){
	    return *this;
	}

	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuStateGet::~CMcuStateGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CMcuStateGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CMcuState* pMcuState = m_pProcess->GetMcuStateObject();

	if (pMcuState)
	{
		pMcuState->SerializeXml(pActionsNode);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuStateGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
//	GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN", &m_updateCounter, _0_TO_DWORD);
	return nStatus;
}
