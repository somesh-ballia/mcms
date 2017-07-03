// LogOutRequest.cpp: implementation of the CLogOutRequest class.
//
//////////////////////////////////////////////////////////////////////
#include "string.h"
#include "LogOutRequest.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "Transactions.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "StringsMaps.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
CLogOutRequest::CLogOutRequest()
{
    m_reason =  logoutNormal;
}

/////////////////////////////////////////////////////////////////////////////
CLogOutRequest::~CLogOutRequest()
{
}

CLogOutRequest& CLogOutRequest::operator = (const CLogOutRequest &other)
{
    m_reason = other.m_reason;
    return *this;
}
 
/////////////////////////////////////////////////////////////////////////////
void CLogOutRequest::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
    
	pFatherNode->AddChildNode("REASON",m_reason,DISCONNECT_REASON_ENUM);
} 

/////////////////////////////////////////////////////////////////////////////

int CLogOutRequest::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError, const char* action)
{
	int nStatus;
    GET_VALIDATE_CHILD(pActionNode,"REASON",(int*)&m_reason,DISCONNECT_REASON_ENUM);
	return STATUS_OK;
}


