
///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Date         Created By         Description
//
//  6/7/2005	    ERAN		wrapper class for Get-IPService	   
//  ========   ==============   ========================================================
////////////////////////////////////////////////////////////////////////////////////////////////


#include "CIPServiceListGet.h"
#include "CSMngrProcess.h"
#include "StatusesGeneral.h"


//////////////////////////////////////////////////////////////////////////////
CIPServiceListGet::CIPServiceListGet()
{
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CIPServiceListGet& CIPServiceListGet::operator = (const CIPServiceListGet &other)
{
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIPServiceListGet::~CIPServiceListGet()
{

}

/////////////////////////////////////////////////////////////////////////////
CIPServiceListGet::CIPServiceListGet( const CIPServiceListGet &other )
:CSerializeObject(other)
{

}

///////////////////////////////////////////////////////////////////////////
void CIPServiceListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CCSMngrProcess *pProcess = (CCSMngrProcess*)CProcessBase::GetProcess();
	CIPServiceList *pIpServiceListStatic = pProcess->GetIpServiceListStatic();
	pIpServiceListStatic->SerializeXml(pActionsNode, m_updateCounter);
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	return nStatus;	
}


////////////////////////////////////////////////////////////////////////////////////////////////




