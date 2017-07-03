// LectureModeParamsDrv.cpp: implementation for the CLectureModeParamsDrv class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//9/05			Talya			  Class for Change Lecture Mode Params
//========   ==============   =====================================================================



#include "LectureModeParamsDrv.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "CommConfDB.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CLectureModeParamsDrv::CLectureModeParamsDrv()
{
	m_ConfID				= 0xFFFFFFFF;
	m_pLectureModeParams	= new CLectureModeParams;
}
/////////////////////////////////////////////////////////////////////////////
CLectureModeParamsDrv& CLectureModeParamsDrv::operator = (const CLectureModeParamsDrv &other)
{
	if ( &other == this ) return *this;

	m_ConfID				= other.m_ConfID;
	*m_pLectureModeParams	= *other.m_pLectureModeParams;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CLectureModeParamsDrv::~CLectureModeParamsDrv()
{
	POBJDELETE(m_pLectureModeParams);
}

/////////////////////////////////////////////////////////////////////////////
int CLectureModeParamsDrv::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CLectureModeParamsDrv::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_LECTURE_MODE",strAction,16))
		numAction = CHANGE_LECTURE_MODE;
	
	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CLectureModeParamsDrv::DeSerializeXml(CXMLDOMElement *pForceNode, char *pszError, int nAction)
{
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_MANDATORY_CHILD(pForceNode,"ID",&m_ConfID,_0_TO_DWORD);
	
	GET_MANDATORY_CHILD_NODE(pForceNode, "LECTURE_MODE", pForceNode);
	
	CCommConf *pConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);
	
	if(pConf)
	{
		*m_pLectureModeParams = *(pConf->GetLectureMode());
	}
	
	nStatus = m_pLectureModeParams->DeSerializeXml(pForceNode,pszError);
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CLectureModeParamsDrv::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
}

//////////////////////////////////////////////////////////////////////////
CLectureModeParams*  CLectureModeParamsDrv::GetLectureModeParams()
{
	return m_pLectureModeParams;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CLectureModeParamsDrv::GetConfID()
{
	return m_ConfID;
}

