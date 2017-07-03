// CIVRServiceListGet.cpp: implementation of the CIVRServiceListGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//				Anatg			  Class for Get XML IVR List
//========   ==============   =====================================================================

#include "NStream.h"
#include "IVRServiceListGet.h"
#include "StatusesGeneral.h"
#include "IVRServiceList.h"


extern CAVmsgServiceList* GetpAVmsgServList();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CIVRServiceListGet::CIVRServiceListGet()
{
	m_updateCounter = 0;
}
/////////////////////////////////////////////////////////////////////////////
CIVRServiceListGet& CIVRServiceListGet::operator = (const CIVRServiceListGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CIVRServiceListGet::~CIVRServiceListGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CIVRServiceListGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	::GetpAVmsgServList()->SerializeXml(pActionsNode, m_updateCounter, YES, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
int CIVRServiceListGet::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{
	DeSerializeXml(pResNode,pszError);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CIVRServiceListGet::DeSerializeXml(CXMLDOMElement* pActionsNode, char* pszError)
{
		
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionsNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	//This code can`t be implement on Serialize
	//DWORD summaryCounter = ::GetpConfDB()->GetSummaryUpdateCounter();
	//if(summaryCounter != GetUpdateCounter())
	//{
	   //SetUpdateCounter(summaryCounter);
	  //SetUpdateFlag(YES);
	//}
	//	else
	//	{
		   //SetStatus(STATUS_OK);
	   //SetUpdateFlag(NO);
	//	}
	
	return nStatus;
}



