#include "CIPServiceFullListGet.h"
#include "CSMngrProcess.h"
#include "IPServiceDynamicList.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "ApiStatuses.h"


CIPServiceFullListGet::CIPServiceFullListGet()
{
}

CIPServiceFullListGet::CIPServiceFullListGet(const CIPServiceFullListGet &rHnd)
:CSerializeObject(rHnd)
{
	
}
	
CIPServiceFullListGet::~CIPServiceFullListGet()
{
}

///////////////////////////////////////////////////////////////////////////
void CIPServiceFullListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	CCSMngrProcess *pCSMngrProcess = (CCSMngrProcess*)CProcessBase::GetProcess();
	CIPServiceFullList *fullList = pCSMngrProcess->GetXMLWraper();
	CIPServiceList *ipServiceListDyn = pCSMngrProcess->GetIpServiceListDynamic();
	fullList->SetIPServiceList(ipServiceListDyn);
	fullList->SerializeXml(pActionsNode, m_updateCounter);
}

/////////////////////////////////////////////////////////////////////////////
int CIPServiceFullListGet::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	return nStatus;	
}
////////////////////////////////////////////////////////////////////////////////////////////////
