// ConfPartySpecific.cpp: implementation of the CConfPartySpecific class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Reservation
//========   ==============   =====================================================================

#include "ConfPartySpecific.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "ConfPartyManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CConfPartySpecific::CConfPartySpecific()
{
	m_ConfID     = 0xFFFFFFFF;
	m_PartyID    = 0xFFFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////
CConfPartySpecific& CConfPartySpecific::operator = (const CConfPartySpecific &other)
{
	m_ConfID = other.m_ConfID;
	m_PartyID = other.m_PartyID;
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CConfPartySpecific::~CConfPartySpecific()
{

}
/////////////////////////////////////////////////////////////////////////////
int CConfPartySpecific::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{
	DeSerializeXml(pResNode,pszError,0);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CConfPartySpecific::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, int nAction)
{
	
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"CONF_ID",&m_ConfID,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"PARTY_ID",&m_PartyID,_0_TO_DWORD);
	
	return nStatus;

}

///////////////////////////////////////////////////////////////////////////
void CConfPartySpecific::SerializeXml(CXMLDOMElement*& pFatherNode) const
{

	STATUS status=STATUS_OK;

	CConfParty* pConfParty = const_cast<CConfParty*>(::GetpConfDB()->GetCurrentParty(m_ConfID,m_PartyID));
	if (pConfParty)
		pConfParty->SerializeSpecPartyXml(pFatherNode);

}

//////////////////////////////////////////////////////////////////////////


