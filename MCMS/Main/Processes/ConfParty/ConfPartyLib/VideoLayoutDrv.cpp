// VideoLayoutDrv.cpp: implementation for the CVideoLayoutDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//8/05		Talya			  Class for Change XML Conf Layout
//========   ==============   =====================================================================



#include "VideoLayoutDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommConfDB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVideoLayoutDrv::CVideoLayoutDrv()
{
	m_ConfID = 0xFFFFFFFF;
	m_pVideoLayout= new CVideoLayout;
}
/////////////////////////////////////////////////////////////////////////////
CVideoLayoutDrv& CVideoLayoutDrv::operator = (const CVideoLayoutDrv &other)
{
	m_ConfID = other.m_ConfID;
	*m_pVideoLayout = *other.m_pVideoLayout;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CVideoLayoutDrv::~CVideoLayoutDrv()
{

	POBJDELETE(m_pVideoLayout);
}


/////////////////////////////////////////////////////////////////////////////
int CVideoLayoutDrv::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CVideoLayoutDrv::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_VIDEO_LAYOUT",strAction,16))
		numAction=SET_CONF_VIDEO;
	
	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CVideoLayoutDrv::DeSerializeXml(CXMLDOMElement *pForceNode, char *pszError, int nAction)
{
	BYTE nScreenLayout;

	int nStatus = STATUS_OK;
	
	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"ID",&m_ConfID,_0_TO_DWORD);
	
	GET_MANDATORY_CHILD_NODE (pForceNode, "FORCE", pForceNode);
			
	CCommConf *pConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);

	if(pConf)
	{
		GET_VALIDATE_MANDATORY_CHILD(pForceNode,"LAYOUT",&nScreenLayout,LAYOUT_ENUM);
		
		CVideoLayout *pFullLayoutData = pConf->GetVideoLayout(nScreenLayout);

		if(pFullLayoutData)
			*m_pVideoLayout = *pFullLayoutData;
	}
	
	nStatus = m_pVideoLayout->DeSerializeXml(pForceNode, pszError, 0);
	
	m_pVideoLayout->SetActive(YES);
	
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CVideoLayoutDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
CVideoLayout*  CVideoLayoutDrv::GetVideoLayout()
{
	return m_pVideoLayout;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CVideoLayoutDrv::GetConfID()
{
	return m_ConfID;
}

