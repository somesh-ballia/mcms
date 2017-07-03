// RsrvPartyAdd.cpp: implementation of CRsrvRecordLinkPartyAdd class.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Add XML Party
//========   ==============   =====================================================================


#include "RsrvRecordLinkPartyAdd.h"
#include "psosxml.h"
#include "StatusesGeneral.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRsrvRecordLinkPartyAdd::CRsrvRecordLinkPartyAdd()
{
	m_ConfID = 0xFFFFFFFF;
	m_pRsrvParty= new CRsrvParty;
}

CRsrvRecordLinkPartyAdd::CRsrvRecordLinkPartyAdd(CRsrvParty* pRsrvParty)
{
	m_ConfID = 0xFFFFFFFF;
	m_pRsrvParty= pRsrvParty;
}
/////////////////////////////////////////////////////////////////////////////
CRsrvRecordLinkPartyAdd& CRsrvRecordLinkPartyAdd::operator = (const CRsrvRecordLinkPartyAdd &other)
{
	m_ConfID = other.m_ConfID;
	*m_pRsrvParty = *other.m_pRsrvParty;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CRsrvRecordLinkPartyAdd::~CRsrvRecordLinkPartyAdd()
{

	POBJDELETE(m_pRsrvParty);
}


/////////////////////////////////////////////////////////////////////////////
int CRsrvRecordLinkPartyAdd::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	return DeSerializeXml(pResNode,pszError,numAction);
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CRsrvRecordLinkPartyAdd::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	
	if((strAction == NULL) || (!strncmp("RESERV_RL",strAction,9)))
		numAction=RESERV_RL;
	else if(!strncmp("ADD",strAction,3))
		numAction=ADD;
	else if(!strncmp("UPDATE",strAction,6))
		numAction=UPDATE;
	
		
	
	return numAction;
}



////////////////////////////////////////////////////////////////////////////////////////////////
int CRsrvRecordLinkPartyAdd::DeSerializeXml(CXMLDOMElement *pPartyNode, char *pszError, int nAction)
{
	
	int nStatus = STATUS_OK;
	
	GET_MANDATORY_CHILD_NODE (pPartyNode, "PARTY", pPartyNode);
	
	return m_pRsrvParty->DeSerializeXml(pPartyNode, pszError, nAction);
}

///////////////////////////////////////////////////////////////////////////
void CRsrvRecordLinkPartyAdd::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
 	
 	if(!pFatherNode)
	{
	    //!!!! in order to cancel the const on the method
	    CRsrvParty * pNode=m_pRsrvParty;
		pFatherNode =  new CXMLDOMElement;
		pFatherNode->set_nodeName("RESERV_RL");
		pNode->SerializeXml(pFatherNode,FULL_DATA);
	}

}

//////////////////////////////////////////////////////////////////////////
CRsrvParty*  CRsrvRecordLinkPartyAdd::GetRsrvParty()
{
	return m_pRsrvParty;
}


//////////////////////////////////////////////////////////////////////////
void  CRsrvRecordLinkPartyAdd::SetRsrvParty(CRsrvParty* pRsrvParty)
{
	m_pRsrvParty = pRsrvParty;
}
////////////////////////////////////////////////////////////////////////////
void  CRsrvRecordLinkPartyAdd::SetConfID(DWORD confId)
{
	m_ConfID = confId;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CRsrvRecordLinkPartyAdd::GetConfID()
{
	return m_ConfID;
}

