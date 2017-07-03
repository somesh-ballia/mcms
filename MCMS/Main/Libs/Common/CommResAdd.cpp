// CommResAdd.cpp: implementation of the CCommResAdd class.
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Add XML Reservation
//========   ==============   =====================================================================

#include "NStream.h"
#include "CommResAdd.h"
#include "psosxml.h"
#include "StatusesGeneral.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommResAdd::CCommResAdd()
{
	m_pCommResApi= new CCommResApi;
	m_pComResRepeatDetails = NULL;
}
/////////////////////////////////////////////////////////////////////////////
CCommResAdd& CCommResAdd::operator = (const CCommResAdd &other)
{
	*m_pCommResApi = *other.m_pCommResApi;
	if(other.m_pComResRepeatDetails == NULL)
	{
		POBJDELETE(m_pComResRepeatDetails);
		m_pComResRepeatDetails = NULL;
	}
	else
	{
		if(m_pComResRepeatDetails == NULL)
			m_pComResRepeatDetails = new CComResRepeatDetails;
		*m_pComResRepeatDetails = *other.m_pComResRepeatDetails;
	}
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommResAdd::~CCommResAdd()
{
	POBJDELETE(m_pCommResApi);
	POBJDELETE(m_pComResRepeatDetails);
}


///////////////////////////////////////////////////////////////////////////
void CCommResAdd::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	if(m_pCommResApi)
		m_pCommResApi->SerializeXml(pFatherNode, 0xFFFFFFFF); //since in setConfirm for AddRsrv this is the value on update it may be the m_dwChange...
		
	if(m_pComResRepeatDetails)	
		m_pComResRepeatDetails->SerializeXml(pFatherNode); 

}

/////////////////////////////////////////////////////////////////////////////
int CCommResAdd::DeSerializeXml(CXMLDOMElement* pResNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;
	int numAction = convertStrActionToNumber(strAction);
	//TRACE UNKNOWN_ACTION==numAction
	nStatus = DeSerializeXml(pResNode,pszError,numAction);
	return nStatus;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CCommResAdd::convertStrActionToNumber(const char* strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("START_REPEATED_EX", strAction,17))
		numAction=ADD_REPEATED_EX;		
	else if(!strncmp("START",strAction,5))
		numAction=ADD_RESERVE;

	return numAction;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CCommResAdd::DeSerializeXml(CXMLDOMElement* pFather, char* pszError, int nAction)
{	
	int nStatus = STATUS_OK;
	CXMLDOMElement* pResNode;
	
	GET_CHILD_NODE(pFather, "RESERVATION", pResNode);
	nStatus = m_pCommResApi->DeSerializeXml(pResNode, pszError, nAction);

	if(nAction == ADD_REPEATED_EX && nStatus==STATUS_OK )
	{
		if(m_pComResRepeatDetails == NULL)
			m_pComResRepeatDetails = new CComResRepeatDetails;

		nStatus = m_pComResRepeatDetails->DeSerializeXml(pFather, pszError, "START_REPEATED_EX");		
	}
	
	return nStatus;
}
//////////////////////////////////////////////////////////////////////////
const CCommResApi*  CCommResAdd::GetCommResApi()
{
	return m_pCommResApi;
}
//////////////////////////////////////////////////////////////////////////
void  CCommResAdd::SetCommResApi(CCommResApi* pCommResApi)
{
	*m_pCommResApi = *pCommResApi;
}
//////////////////////////////////////////////////////////////////////////
const CComResRepeatDetails*  CCommResAdd::GetComResRepeatDetails()
{
	return m_pComResRepeatDetails;
}
//////////////////////////////////////////////////////////////////////////
void  CCommResAdd::SetComResRepeatDetails(CComResRepeatDetails* pComResRepeatDetails)
{
	*m_pComResRepeatDetails = *pComResRepeatDetails;
}

