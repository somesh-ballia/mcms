// IVRServiceAdd.cpp: implementation of the CIVRServiceAdd class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Add XML IVR Service
//========   ==============   =====================================================================

#include "NStream.h"
#include "IVRServiceAdd.h"
#include "psosxml.h"
#include "StatusesGeneral.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIVRServiceAdd::CIVRServiceAdd()
{
	m_pAVmsgService= new CAVmsgService;
}
/////////////////////////////////////////////////////////////////////////////
CIVRServiceAdd& CIVRServiceAdd::operator = (const CIVRServiceAdd &other)
{
	*m_pAVmsgService = *other.m_pAVmsgService;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIVRServiceAdd::~CIVRServiceAdd()
{

	POBJDELETE(m_pAVmsgService);
}


///////////////////////////////////////////////////////////////////////////
void CIVRServiceAdd::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	if(m_pAVmsgService)
		m_pAVmsgService->SerializeXml(pFatherNode); //since in setConfirm for AddRsrv this is the value on update it may be the m_dwChange...
}

/////////////////////////////////////////////////////////////////////////////
int CIVRServiceAdd::DeSerializeXml(CXMLDOMElement* pIVRServiceNode,char* pszError,const char* strAction)
{
	int numAction = convertStrActionToNumber(strAction);
	//TRACE UNKNOWN_ACTION==numAction
	DeSerializeXml(pIVRServiceNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CIVRServiceAdd::convertStrActionToNumber(const char* strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("ADD_IVR",strAction,7))
		numAction = NEW_AV_MSG;
	return numAction;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CIVRServiceAdd::DeSerializeXml(CXMLDOMElement* pIVRServiceNode, char* pszError, int nAction)
{
		
	int nStatus = STATUS_OK;
	
/// ????? 2 x pIVRServiceNode
	GET_MANDATORY_CHILD_NODE(pIVRServiceNode, "IVR_SERVICE", pIVRServiceNode);
	m_pAVmsgService->DeSerializeXml(pIVRServiceNode, pszError);

	return nStatus;

}

//////////////////////////////////////////////////////////////////////////
const CAVmsgService*  CIVRServiceAdd::GetAVmsgService()
{
	return m_pAVmsgService;
}

//////////////////////////////////////////////////////////////////////////
void  CIVRServiceAdd::SetAVmsgService(CAVmsgService* pAVmsgService)
{
	*m_pAVmsgService = *pAVmsgService;
}




