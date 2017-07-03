
#include "RtmIsdnServiceListGet.h"
#include "RtmIsdnMngrProcess.h"
#include "RtmIsdnServiceList.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceListGet::CRtmIsdnServiceListGet()
{
 	m_pProcess = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceListGet::CRtmIsdnServiceListGet( const CRtmIsdnServiceListGet &other )
:CSerializeObject(other)
{
 	m_pProcess = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceListGet& CRtmIsdnServiceListGet::operator = (const CRtmIsdnServiceListGet &other)
{
	if(this == &other){
		return *this;
	}

	m_pProcess = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnServiceListGet::~CRtmIsdnServiceListGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CRtmIsdnServiceListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CRtmIsdnServiceList *pServiceListUpdated = m_pProcess->GetServiceListUpdated();
	
	if (pServiceListUpdated)
	{
		pServiceListUpdated->SerializeXml(pActionsNode, m_updateCounter);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnServiceListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	return nStatus;	
}


////////////////////////////////////////////////////////////////////////////////////////////////




