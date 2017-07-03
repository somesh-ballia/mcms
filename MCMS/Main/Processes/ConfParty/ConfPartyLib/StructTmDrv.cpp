// CSetEndTime.cpp: implementation for the CSetEndTime class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//12/05			Ori			  Class for Change XML CStructTm
//========   ==============   =====================================================================



#include "SetEndTime.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "InternalProcessStatuses.h"
#include "ApiStatuses.h"
#include "ConfPartyApiDefines.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CSetEndTime::CSetEndTime()
{
	m_ConfID = 0xFFFFFFFF;
	m_pTime= new CStructTm;
}
/////////////////////////////////////////////////////////////////////////////
CSetEndTime& CSetEndTime::operator = (const CSetEndTime &other)
{
	m_ConfID = other.m_ConfID;
	*m_pTime = *other.m_pTime;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CSetEndTime::~CSetEndTime()
{

	POBJDELETE(m_pTime);
}


/////////////////////////////////////////////////////////////////////////////
int CSetEndTime::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CSetEndTime::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("SET_END_TIME",strAction,16))
		numAction=SET_END_TIME;
	
	return numAction;
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CSetEndTime::DeSerializeXml(CXMLDOMElement *pTimeNode, char *pszError, int nAction)
{
	BYTE nScreenLayout;
	int nStatus = STATUS_OK;
	
	GET_VALIDATE_MANDATORY_CHILD(pTimeNode,"ID", &m_ConfID, _0_TO_DWORD);
	GET_VALIDATE_MANDATORY_CHILD(pTimeNode,"END_TIME",m_pTime, DATE_TIME);
	
	return nStatus;
}

///////////////////////////////////////////////////////////////////////////
void CSetEndTime::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	pFatherNode->AddChildNode("ID",m_ConfID,_0_TO_DWORD);

    if (m_pTime)
    {    
        pFatherNode->AddChildNode("END_TIME",*m_pTime);
    }
    

}

//////////////////////////////////////////////////////////////////////////
CStructTm*  CSetEndTime::GetTime()
{
	return m_pTime;
}
//////////////////////////////////////////////////////////////////////////
DWORD  CSetEndTime::GetConfID()
{
	return m_ConfID;
}

