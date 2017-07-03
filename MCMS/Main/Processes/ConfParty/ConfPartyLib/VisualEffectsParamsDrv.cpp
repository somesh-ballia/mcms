// VideoLayoutDrv.cpp: implementation for the CVideoLayoutDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//8/05		Talya			  Class for Change XML Conf Layout
//========   ==============   =====================================================================



#include "VisualEffectsParamsDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommConfDB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CVisualEffectsParamsDrv::CVisualEffectsParamsDrv()
{
	m_ConfID			= 0xFFFFFFFF;
	m_pVisualEffects	= new CVisualEffectsParams;
}
/////////////////////////////////////////////////////////////////////////////
CVisualEffectsParamsDrv& CVisualEffectsParamsDrv::operator = (const CVisualEffectsParamsDrv &other)
{
	if ( &other == this ) return *this;
	m_ConfID			= other.m_ConfID;
	*m_pVisualEffects	= *other.m_pVisualEffects;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CVisualEffectsParamsDrv::~CVisualEffectsParamsDrv()
{

	POBJDELETE(m_pVisualEffects);
}


/////////////////////////////////////////////////////////////////////////////
int CVisualEffectsParamsDrv::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CVisualEffectsParamsDrv::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_VISUAL_EFFECT",strAction,18))
		numAction = UPDATE_VISUAL_EFFECTS;
	
	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CVisualEffectsParamsDrv::DeSerializeXml(CXMLDOMElement *pForceNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"ID",&m_ConfID,_0_TO_DWORD);
	
	GET_MANDATORY_CHILD_NODE(pForceNode, "VISUAL_EFFECTS", pForceNode);
	
	CCommConf *pConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);
	
	if(pConf)
	{
		*m_pVisualEffects = pConf->GetVisualEffects();
	}
	
	nStatus = m_pVisualEffects->DeSerializeXml(pForceNode,pszError);
	return nStatus;
}
///////////////////////////////////////////////////////////////////////////
void CVisualEffectsParamsDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
CVisualEffectsParams*  CVisualEffectsParamsDrv::GetVisualEffectsParams()
{
	return m_pVisualEffects;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CVisualEffectsParamsDrv::GetConfID()
{
	return m_ConfID;
}

