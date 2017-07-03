// IPServiceDynamicList.cpp: implementation of the CIPServiceDynamicList class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		does serialization of dynamic properties of ip services
//========   ==============   =====================================================================

#include "IPServiceDynamicList.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "IpService.h"
#include "DynIPSProperties.h"
#include "StatusesGeneral.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIPServiceFullList::CIPServiceFullList()
{
	m_IPServiceList = NULL;
}

//////////////////////////////////////////////////////////////////////
CIPServiceFullList::CIPServiceFullList(const CIPServiceFullList & other)
:CSerializeObject(other)
{
	m_IPServiceList = other.m_IPServiceList;	
}

//////////////////////////////////////////////////////////////////////
CIPServiceFullList::~CIPServiceFullList()
{

}

//////////////////////////////////////////////////////////////////////
CSerializeObject* CIPServiceFullList::Clone(void)
{
	CIPServiceFullList *clone = new CIPServiceFullList;
	clone->m_IPServiceList = m_IPServiceList;
	
	return clone;
}

//////////////////////////////////////////////////////////////////////
void CIPServiceFullList::SetIPServiceList(CIPServiceList *ipServiceList)
{
	m_IPServiceList = ipServiceList;
}

//////////////////////////////////////////////////////////////////////
void CIPServiceFullList::SerializeXml(CXMLDOMElement* thisNode,DWORD objToken)const
{
	CXMLDOMElement* childNode = thisNode->AddChildNode("FULL_IP_SERVICE_LIST");

	DWORD bChanged = InsertUpdateCntChanged(childNode, objToken);

	if(TRUE == bChanged)
	{		
		CIPService *service = m_IPServiceList->GetFirstService();
		while(NULL != service)
		{
			CXMLDOMElement* grandChildNode = childNode->AddChildNode("FULL_IP_SERVICE");

			service->SerializeXml(grandChildNode, UPDATE_CNT_BEGIN_END);
			service->GetDynamicProperties()->SerializeXml(grandChildNode);

			service = m_IPServiceList->GetNextService();
		}
	}
}

//////////////////////////////////////////////////////////////////////
void CIPServiceFullList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	SerializeXml(pFatherNode, m_updateCounter);
}

//////////////////////////////////////////////////////////////////////
int CIPServiceFullList::DeSerializeXml(class CXMLDOMElement *pNode, char* pszError, const char* strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pNode, "OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	return nStatus;
}
