#include "SiteNameInfoDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommConfDB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSiteNameInfoDrv::CSiteNameInfoDrv()
{
	m_ConfID				= 0xFFFFFFFF;
	m_pSiteNameInfo	= new CSiteNameInfo;
}
/////////////////////////////////////////////////////////////////////////////
CSiteNameInfoDrv& CSiteNameInfoDrv::operator = (const CSiteNameInfoDrv &other)
{
	if ( &other == this ) return *this;

	m_ConfID				= other.m_ConfID;
	*m_pSiteNameInfo		= *other.m_pSiteNameInfo;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CSiteNameInfoDrv::~CSiteNameInfoDrv()
{
	POBJDELETE(m_pSiteNameInfo);
}

/////////////////////////////////////////////////////////////////////////////
int CSiteNameInfoDrv::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CSiteNameInfoDrv::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_SITE_NAME",strAction,13))
		numAction = CHANGE_SITE_NAME;

	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CSiteNameInfoDrv::DeSerializeXml(CXMLDOMElement *pForceNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"ID",&m_ConfID,_0_TO_DWORD);

	GET_MANDATORY_CHILD_NODE(pForceNode, "SITE_NAME", pForceNode);

	CCommConf *pConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);

	if(pConf)
	{
		*m_pSiteNameInfo = *(pConf->GetSiteNameInfo());
	}

	nStatus = m_pSiteNameInfo->DeSerializeXml(pForceNode,pszError);
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CSiteNameInfoDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
CSiteNameInfo*  CSiteNameInfoDrv::GetSiteNameInfo()
{
	return m_pSiteNameInfo;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CSiteNameInfoDrv::GetConfID()
{
	return m_ConfID;
}

