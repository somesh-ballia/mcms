
#include "RtmIsdnSpanMapListGet.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "RtmIsdnMngrProcess.h"
#include "RtmIsdnSpanMapList.h"
#include "ApiStatuses.h"


//////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapListGet::CRtmIsdnSpanMapListGet()
{
	m_pProcess = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapListGet::CRtmIsdnSpanMapListGet( const CRtmIsdnSpanMapListGet &other )
:CSerializeObject(other)
{
	m_pProcess = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapListGet& CRtmIsdnSpanMapListGet::operator = (const CRtmIsdnSpanMapListGet &other)
{
	if(this == &other){
		return *this;
	}

	m_pProcess = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRtmIsdnSpanMapListGet::~CRtmIsdnSpanMapListGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CRtmIsdnSpanMapListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	if (pSpanMapList)
	{
		pSpanMapList->SerializeXml(pActionsNode/*, m_updateCounter*/);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CRtmIsdnSpanMapListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////


