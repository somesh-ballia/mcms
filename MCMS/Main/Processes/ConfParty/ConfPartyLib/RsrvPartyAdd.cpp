// RsrvPartyAdd.cpp: implementation of CRsrvPartyAdd class.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Add XML Party
//========   ==============   =====================================================================


#include "RsrvPartyAdd.h"
#include "psosxml.h"
#include "StatusesGeneral.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRsrvPartyAdd::CRsrvPartyAdd()
{
	m_ConfID = 0xFFFFFFFF;
	m_pRsrvParty= new CRsrvParty;
}
/////////////////////////////////////////////////////////////////////////////
CRsrvPartyAdd& CRsrvPartyAdd::operator = (const CRsrvPartyAdd &other)
{
	m_ConfID = other.m_ConfID;
	*m_pRsrvParty = *other.m_pRsrvParty;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRsrvPartyAdd::~CRsrvPartyAdd()
{

	POBJDELETE(m_pRsrvParty);
}


/////////////////////////////////////////////////////////////////////////////
int CRsrvPartyAdd::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	return DeSerializeXml(pResNode,pszError,numAction);
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CRsrvPartyAdd::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("ADD_PARTY",strAction,9))
		numAction=NEW_PARTY;
	
	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CRsrvPartyAdd::DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int nAction)
{
	
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_MANDATORY_CHILD(pPartyNode,"ID",&m_ConfID,_0_TO_DWORD);
	
	GET_MANDATORY_CHILD_NODE (pPartyNode, "PARTY", pPartyNode);
	
	return m_pRsrvParty->DeSerializeXml(pPartyNode, pszError, nAction);
}

///////////////////////////////////////////////////////////////////////////
void CRsrvPartyAdd::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement *pPartyNode;
//	pPartyNode = pFatherNode->AddChildNode("ID",m_ConfID);
//	pPartyNode=pFatherNode->AddChildNode("PARTY");
//	pPartyNode->AddChildNode("NAME",m_pRsrvParty->GetName());
	pPartyNode = pFatherNode->AddChildNode("ID",m_pRsrvParty->GetPartyId());
}

//////////////////////////////////////////////////////////////////////////
CRsrvParty*  CRsrvPartyAdd::GetRsrvParty()
{
	return m_pRsrvParty;
}
////////////////////////////////////////////////////////////////////////////
void  CRsrvPartyAdd::SetConfID(DWORD confId)
{
	m_ConfID = confId;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CRsrvPartyAdd::GetConfID()
{
	return m_ConfID;
}

