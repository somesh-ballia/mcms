// CommConfDBGet.cpp: implementation of the CCommConfDBGet class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Get XML Conf List
//========   ==============   =====================================================================

#include "NStream.h"
#include "CommConfDBGet.h"
#include "StatusesGeneral.h"


extern CCommConfDB* GetpConfDB();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommConfDBGet::CCommConfDBGet()
{
	m_updateCounter = 0;
}
/////////////////////////////////////////////////////////////////////////////
CCommConfDBGet& CCommConfDBGet::operator = (const CCommConfDBGet &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommConfDBGet::~CCommConfDBGet()
{

}

///////////////////////////////////////////////////////////////////////////
void CCommConfDBGet::SerializeXml(CXMLDOMElement*& pActionsNode) const
{

  if(::GetpConfDB() != NULL)
  	::GetpConfDB()->SerializeXml(pActionsNode,m_updateCounter);

}

/////////////////////////////////////////////////////////////////////////////
int CCommConfDBGet::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{

	DeSerializeXml(pResNode,pszError);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CCommConfDBGet::DeSerializeXml(CXMLDOMElement* pActionsNode, char* pszError)
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
	//	   //SetStatus(STATUS_OK);
	//	   //SetUpdateFlag(NO);
	//	}
	
	return nStatus;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommConfDBGetFull::CCommConfDBGetFull()
{
	m_updateCounter = 0;
}
/////////////////////////////////////////////////////////////////////////////
CCommConfDBGetFull& CCommConfDBGetFull::operator = (const CCommConfDBGetFull &other)
{
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommConfDBGetFull::~CCommConfDBGetFull()
{

}

///////////////////////////////////////////////////////////////////////////
void CCommConfDBGetFull::SerializeXml(CXMLDOMElement*& pActionsNode) const
{

  if(::GetpConfDB() != NULL)
  	::GetpConfDB()->SerializeFullXml(pActionsNode,m_updateCounter);

}

/////////////////////////////////////////////////////////////////////////////
int CCommConfDBGetFull::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{

	DeSerializeXml(pResNode,pszError);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CCommConfDBGetFull::DeSerializeXml(CXMLDOMElement* pActionsNode, char* pszError)
{
		
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_CHILD(pActionsNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
		
	return nStatus;
}

