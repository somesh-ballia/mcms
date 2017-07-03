
///////////////////////////////////////////////////////////////////////////////////////////////
//
//  Date         Created By         Description
//
//  11/9/2012	    Judith		 wrapper class for Get-SystemInterfaceList
//  =========   ==============   ========================================================
////////////////////////////////////////////////////////////////////////////////////////////////


#include "SystemInterfaceListGet.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "InitCommonStrings.h"
#include "CSMngrProcess.h"
#include "SystemInterface.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////////////
CSystemInterfaceListGet::CSystemInterfaceListGet()
{
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CSystemInterfaceListGet& CSystemInterfaceListGet::operator = (const CSystemInterfaceListGet &other)
{
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CSystemInterfaceListGet::~CSystemInterfaceListGet()
{

}

/////////////////////////////////////////////////////////////////////////////
CSystemInterfaceListGet::CSystemInterfaceListGet( const CSystemInterfaceListGet &other )
:CSerializeObject(other)
{

}

///////////////////////////////////////////////////////////////////////////
void CSystemInterfaceListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CCSMngrProcess *pProcess = (CCSMngrProcess*)CProcessBase::GetProcess();
	CSystemInterfaceList *pSystemInterfaceList = pProcess->GetSystemInterfaceList();
	pSystemInterfaceList->SerializeXml(pActionsNode);

}

/////////////////////////////////////////////////////////////////////////////
int CSystemInterfaceListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
//	GET_VALIDATE_CHILD(pNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	return nStatus;	
}


////////////////////////////////////////////////////////////////////////////////////////////////




