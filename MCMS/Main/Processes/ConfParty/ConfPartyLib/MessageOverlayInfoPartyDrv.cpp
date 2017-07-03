// CMessageOverlayInfoPartyDrv.cpp: implementation for the CMessageOverlayInfoPartyDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date                 Created By        Description
//
//13 April 2011			Marina			 Class for Change Message Overlay Params per specific Party
//===============   ==============   =====================================================================



#include "MessageOverlayInfoPartyDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommConfDB.h"
#include "Trace.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMessageOverlayInfoPartyDrv::CMessageOverlayInfoPartyDrv()
{
	m_ConfID				= 0xFFFFFFFF;
	m_PartyID 				= 0xFFFFFFFF;
	m_IsPrivate 			= FALSE;
	m_pMessageOverlayInfo	= new CMessageOverlayInfo;
}
/////////////////////////////////////////////////////////////////////////////
CMessageOverlayInfoPartyDrv& CMessageOverlayInfoPartyDrv::operator = (const CMessageOverlayInfoPartyDrv &other)
{
	if ( &other == this ) return *this;

	m_ConfID				= other.m_ConfID;
	m_PartyID				= other.m_PartyID;
	m_IsPrivate				= other.m_IsPrivate;
	*m_pMessageOverlayInfo	= *other.m_pMessageOverlayInfo;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMessageOverlayInfoPartyDrv::~CMessageOverlayInfoPartyDrv()
{
	POBJDELETE(m_pMessageOverlayInfo);
}

/////////////////////////////////////////////////////////////////////////////
int CMessageOverlayInfoPartyDrv::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CMessageOverlayInfoPartyDrv::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_PARTY_MESSAGE_OVERLAY",strAction,16))
		numAction = CHANGE_PARTY_MESSAGE_OVERLAY;

	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CMessageOverlayInfoPartyDrv::DeSerializeXml(CXMLDOMElement *pForceNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"ID",&m_ConfID,_0_TO_DWORD);
	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"PARTY_ID",&m_PartyID,_0_TO_DWORD);

	GET_MANDATORY_CHILD_NODE(pForceNode, "MESSAGE_OVERLAY", pForceNode);



	CCommConf *pConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);

	if(pConf)
	{
		CConfParty *pParty = pConf->GetCurrentParty(m_PartyID);
		if(pParty)
		{
			*m_pMessageOverlayInfo = *(pConf->GetMessageOverlay());

		}
	}

	nStatus = m_pMessageOverlayInfo->DeSerializeXml(pForceNode,pszError);
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CMessageOverlayInfoPartyDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
CMessageOverlayInfo*  CMessageOverlayInfoPartyDrv::GetMessageOverlayInfo()
{
	return m_pMessageOverlayInfo;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CMessageOverlayInfoPartyDrv::GetConfID()
{
	return m_ConfID;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CMessageOverlayInfoPartyDrv::GetPartyID()
{
	return m_PartyID;
}
//////////////////////////////////////////////////////////////////////////
WORD  CMessageOverlayInfoPartyDrv::GetIsPrivate()
{
	return m_IsPrivate;
}

