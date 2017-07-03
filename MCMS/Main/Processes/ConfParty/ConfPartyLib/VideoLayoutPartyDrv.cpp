// VideoLayoutDrv.cpp: implementation for the CVideoLayoutDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//8/05		Talya			  Class for Change XML Conf Layout
//========   ==============   =====================================================================



#include "VideoLayoutPartyDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommConfDB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVideoLayoutPartyDrv::CVideoLayoutPartyDrv()
{
	m_ConfID = 0xFFFFFFFF;
	m_PartyID = 0xFFFFFFFF;
	m_IsPrivate = FALSE;
	m_pVideoLayout= new CVideoLayout;
}
/////////////////////////////////////////////////////////////////////////////
CVideoLayoutPartyDrv& CVideoLayoutPartyDrv::operator = (const CVideoLayoutPartyDrv &other)
{
	m_ConfID = other.m_ConfID;
	m_PartyID = other.m_PartyID;
	m_IsPrivate = other.m_IsPrivate;
	*m_pVideoLayout = *other.m_pVideoLayout;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CVideoLayoutPartyDrv::~CVideoLayoutPartyDrv()
{

	POBJDELETE(m_pVideoLayout);
}


/////////////////////////////////////////////////////////////////////////////
int CVideoLayoutPartyDrv::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CVideoLayoutPartyDrv::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_PARTY_VIDEO_LAYOUT_EX",strAction,16))
		numAction=SET_PARTY_VIDEO;
	
	return numAction;
 }


////////////////////////////////////////////////////////////////////////////////////////////////
int CVideoLayoutPartyDrv::DeSerializeXml(CXMLDOMElement *pForceNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;
	int nVal;
	BYTE nScreenLayout;
	
	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"ID",&m_ConfID,_0_TO_DWORD);
	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"PARTY_ID",&m_PartyID,_0_TO_DWORD);
	
	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"LAYOUT_TYPE",&nVal, LAYOUT_TYPE_ENUM);
		
	if(!nVal)
		m_IsPrivate = FALSE;
	else
		m_IsPrivate = TRUE;
	
	GET_MANDATORY_CHILD_NODE(pForceNode, "FORCE", pForceNode);
		
	CCommConf *pConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);
	
	if(pConf)
	{
		CConfParty *pParty = pConf->GetCurrentParty(m_PartyID);
		
		if(pParty)
		{
			GET_VALIDATE_CHILD(pForceNode,"LAYOUT",&nScreenLayout,LAYOUT_ENUM);				
			
			CVideoLayout *pFullLayoutData;
			
			if(m_IsPrivate)
				pFullLayoutData = pParty->GetPrivateVideoLayout(nScreenLayout);
			else
				pFullLayoutData = pParty->GetVideoLayout();
			
			if(pFullLayoutData)
				*m_pVideoLayout = *pFullLayoutData;
		}
	}

	nStatus = m_pVideoLayout->DeSerializeXml(pForceNode, pszError, 1);
	
	m_pVideoLayout->SetActive(YES);
	
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CVideoLayoutPartyDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
CVideoLayout*  CVideoLayoutPartyDrv::GetVideoLayout()
{
	return m_pVideoLayout;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CVideoLayoutPartyDrv::GetConfID()
{
	return m_ConfID;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CVideoLayoutPartyDrv::GetPartyID()
{
	return m_PartyID;
}
//////////////////////////////////////////////////////////////////////////
WORD  CVideoLayoutPartyDrv::GetIsPrivate()
{
	return m_IsPrivate;
}


