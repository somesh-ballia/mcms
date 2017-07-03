// RsrvPartyDel.cpp: implementation of the CRsrvPartyDel class.
//
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Party
//========   ==============   =====================================================================


#include "RsrvPartyDel.h"
#include "psosxml.h"
#include "StatusesGeneral.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRsrvPartyDel::CRsrvPartyDel()
{
	m_ConfID = 0xFFFFFFFF;
	m_PartyID = 0xFFFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////
CRsrvPartyDel& CRsrvPartyDel::operator = (const CRsrvPartyDel &other)
{
	m_ConfID = other.m_ConfID;
	m_PartyID = other.m_PartyID;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRsrvPartyDel::~CRsrvPartyDel()
{

}


/////////////////////////////////////////////////////////////////////////////
int CRsrvPartyDel::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	//TRACE UNKNOWN_ACTION==numAction
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CRsrvPartyDel::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("DELETE_PARTY",strAction,12))
		numAction=DEL_PARTY;
	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CRsrvPartyDel::DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int nAction)
{
	
	int nStatus = STATUS_OK;
	
	//GET_VALIDATE_MANDATORY_CHILD(pPartyNode,"ID",&m_ConfID,_0_TO_DWORD);

	GET_VALIDATE_CHILD(pPartyNode,"ID",&m_ConfID,_0_TO_DWORD);

	GET_VALIDATE_MANDATORY_CHILD(pPartyNode,"PARTY_ID",&m_PartyID,_0_TO_DWORD);

	return nStatus;

}

///////////////////////////////////////////////////////////////////////////
void CRsrvPartyDel::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}


//////////////////////////////////////////////////////////////////////////
DWORD  CRsrvPartyDel::GetConfID()
{
	return m_ConfID;
}


//////////////////////////////////////////////////////////////////////////
DWORD  CRsrvPartyDel::GetPartyID()
{
	return m_PartyID;
}

