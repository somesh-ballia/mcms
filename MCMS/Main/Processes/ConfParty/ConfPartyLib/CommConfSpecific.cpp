// CommConfSpecific.cpp: implementation of the CCommConfSpecific class.
//
//////////////////////////////////////////////////////////////////////
//
//Date         Created By         Description
//
//3/6/05		Yoella			  Class for Del XML Reservation
//========   ==============   =====================================================================

#include "CommConfSpecific.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "ConfPartyManager.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CCommConfSpecific::CCommConfSpecific()
{
	m_ConfID    = 0xFFFFFFFF;
}

/////////////////////////////////////////////////////////////////////////////
CCommConfSpecific& CCommConfSpecific::operator = (const CCommConfSpecific &other)
{
	m_ConfID = other.m_ConfID;
	m_updateCounter = other.m_updateCounter;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CCommConfSpecific::~CCommConfSpecific()
{
}

/////////////////////////////////////////////////////////////////////////////
int CCommConfSpecific::DeSerializeXml(CXMLDOMElement *pResNode,char *pszError,const char * strAction)
{

	int numAction = convertStrActionToNumber(strAction);
	DeSerializeXml(pResNode,pszError,numAction);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CCommConfSpecific::convertStrActionToNumber(const char * strAction)
{
	int numAction = UNKNOWN_ACTION;
	if(!strncmp("TERMINATE_CONF",strAction,14))
		numAction=TERMINATE_CONF;
	
	return numAction;
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CCommConfSpecific::DeSerializeXml(CXMLDOMElement *pActionNode, char *pszError, int nAction)
{
	
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"OBJ_TOKEN",&m_updateCounter,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"ID",&m_ConfID,_0_TO_DWORD);
	
	return nStatus;

}

///////////////////////////////////////////////////////////////////////////
void CCommConfSpecific::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	STATUS status=STATUS_OK;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_ConfID);

	if(pCommConf)
		pCommConf->SerializeXml(pFatherNode,m_updateCounter/*1*/); //1 for update counter see old code

}


