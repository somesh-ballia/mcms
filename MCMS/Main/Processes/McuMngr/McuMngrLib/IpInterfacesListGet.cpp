

#include "IpInterfacesListGet.h"
#include "StatusesGeneral.h"
#include "McuMngrProcess.h"
#include "IpService.h"



//////////////////////////////////////////////////////////////////////////////
CIpInterfacesListGet::CIpInterfacesListGet()
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = 0;
}

/////////////////////////////////////////////////////////////////////////////
CIpInterfacesListGet::CIpInterfacesListGet( const CIpInterfacesListGet &other )
:CSerializeObject(other)
{
	m_pProcess = (CMcuMngrProcess*)CMcuMngrProcess::GetProcess();
	m_updateCounter = other.m_updateCounter;
}

/////////////////////////////////////////////////////////////////////////////
CIpInterfacesListGet& CIpInterfacesListGet::operator = (const CIpInterfacesListGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIpInterfacesListGet::~CIpInterfacesListGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CIpInterfacesListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CIPServiceList* pIpServiceList = m_pProcess->GetIpInterfaces_asIpServicesList();
	
	if (pIpServiceList)
	{
		pIpServiceList->SerializeXml(pActionsNode, m_updateCounter);
	}
}

/////////////////////////////////////////////////////////////////////////////
int CIpInterfacesListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
//	DeSerializeXml(pNode,pszError);
//	return STATUS_OK;

	int nStatus = STATUS_OK;
	GET_VALIDATE_CHILD(pNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	return nStatus;
	
}

////////////////////////////////////////////////////////////////////////////////////////////////



