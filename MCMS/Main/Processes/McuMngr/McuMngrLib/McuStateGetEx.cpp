/*
 * McuStateGetEx.cpp
 *
 *  Created on: Nov 24, 2009
 *      Author: yael
 */

//////////////////////////////////////////////////////////////////////////


#include "McuStateGetEx.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "psosxml.h"
#include "ApiStatuses.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction  for CMcuStateGetEx class
//////////////////////////////////////////////////////////////////////
CMcuStateGetEx::CMcuStateGetEx()
{
	strncpy(m_client_ip, "", IP_ADDRESS_STR_LEN);
}

/////////////////////////////////////////////////////////////////////////////
CMcuStateGetEx& CMcuStateGetEx::operator = (const CMcuStateGetEx &other)
{
	strncpy(m_client_ip, other.m_client_ip, IP_ADDRESS_STR_LEN);
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuStateGetEx::~CMcuStateGetEx()
{

}

///////////////////////////////////////////////////////////////////////////
void CMcuStateGetEx::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	PASSERTMSG(1, "this function should not be called");
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuStateGetEx::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pNode, "CLIENT_IP", m_client_ip, _1_TO_IP_ADDRESS_LENGTH);
	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////
char* CMcuStateGetEx::GetClientIp()
{
	return m_client_ip;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction --- CMcuStateGetExReply
//////////////////////////////////////////////////////////////////////
CMcuStateGetExReply::CMcuStateGetExReply()
{
	m_bFailoverTrigger = FALSE;
}

/////////////////////////////////////////////////////////////////////////////
CMcuStateGetExReply& CMcuStateGetExReply::operator = (const CMcuStateGetExReply &other)
{
	m_bFailoverTrigger = other.m_bFailoverTrigger;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CMcuStateGetExReply::~CMcuStateGetExReply()
{

}

///////////////////////////////////////////////////////////////////////////
void CMcuStateGetExReply::SerializeXml(CXMLDOMElement*& pActionsNode) const
{
	pActionsNode->AddChildNode("FAILOVER_TRIGGER",m_bFailoverTrigger,_BOOL);
}


////////////////////////////////////////////////////////////////////////////////////////////////
int CMcuStateGetExReply::DeSerializeXml(CXMLDOMElement* pNode,char* pszError,const char* strAction)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pNode, "FAILOVER_TRIGGER", &m_bFailoverTrigger, _BOOL);
	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////
