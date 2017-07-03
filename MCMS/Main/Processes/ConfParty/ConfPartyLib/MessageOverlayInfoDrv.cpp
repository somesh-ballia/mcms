// MessageOverlayInfoDrv.cpp: implementation for the CMessageOverlayInfoDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//9/05			Talya			  Class for Change Message Overlay Params
//========   ==============   =====================================================================



#include "MessageOverlayInfoDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommConfDB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CMessageOverlayInfoDrv::CMessageOverlayInfoDrv()
{
	m_ConfID				= 0xFFFFFFFF;
	m_pMessageOverlayInfo	= new CMessageOverlayInfo;
}
/////////////////////////////////////////////////////////////////////////////
CMessageOverlayInfoDrv& CMessageOverlayInfoDrv::operator = (const CMessageOverlayInfoDrv &other)
{
	if ( &other == this ) return *this;

	m_ConfID				= other.m_ConfID;
	*m_pMessageOverlayInfo	= *other.m_pMessageOverlayInfo;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMessageOverlayInfoDrv::~CMessageOverlayInfoDrv()
{
	POBJDELETE(m_pMessageOverlayInfo);
}

/////////////////////////////////////////////////////////////////////////////
int CMessageOverlayInfoDrv::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CMessageOverlayInfoDrv::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_MESSAGE_OVERLAY",strAction,16))
		numAction = CHANGE_MESSAGE_OVERLAY;

	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CMessageOverlayInfoDrv::DeSerializeXml(CXMLDOMElement *pForceNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"ID",&m_ConfID,_0_TO_DWORD);

	GET_MANDATORY_CHILD_NODE(pForceNode, "MESSAGE_OVERLAY", pForceNode);

	CCommConf *pConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);

	if(pConf)
	{
		*m_pMessageOverlayInfo = *(pConf->GetMessageOverlay());
	}

	nStatus = m_pMessageOverlayInfo->DeSerializeXml(pForceNode,pszError);
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CMessageOverlayInfoDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
CMessageOverlayInfo*  CMessageOverlayInfoDrv::GetMessageOverlayInfo()
{
	return m_pMessageOverlayInfo;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CMessageOverlayInfoDrv::GetConfID()
{
	return m_ConfID;
}

