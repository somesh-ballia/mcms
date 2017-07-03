#include "H323GkStatus.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

CH323GatekeeperStatus::CH323GatekeeperStatus()
{
	m_gkState			  = NO;
	m_reqBandwidth		  = 0;
	m_allocBandwidth	  = 0;
	m_requestIntoInterval = 0;
	m_gkRouted            = NO;
}

/////////////////////////////////////////////////////////////////////////////
CH323GatekeeperStatus::CH323GatekeeperStatus(const CH323GatekeeperStatus &other)
:CPObject(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
CH323GatekeeperStatus& CH323GatekeeperStatus::operator = (const CH323GatekeeperStatus& other)
{
	m_gkState			  = other.m_gkState;
	m_reqBandwidth		  = other.m_reqBandwidth;
	m_allocBandwidth	  = other.m_allocBandwidth;
	m_requestIntoInterval = other.m_requestIntoInterval;
	m_gkRouted			  = other.m_gkRouted;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CH323GatekeeperStatus::~CH323GatekeeperStatus()
{
}

/////////////////////////////////////////////////////////////////////////////
void CH323GatekeeperStatus::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pGKStatusNode = pFatherNode->AddChildNode("GK_STATUS");

	pGKStatusNode->AddChildNode("GK_STATE",m_gkState,GK_STATE_ENUM);			
	pGKStatusNode->AddChildNode("ALLOC_BANDWIDTH",m_allocBandwidth);
	pGKStatusNode->AddChildNode("REQ_BANDWIDTH",m_reqBandwidth);
	pGKStatusNode->AddChildNode("REQ_INTERVAL",m_requestIntoInterval);
	pGKStatusNode->AddChildNode("GK_ROUTED",m_gkRouted,_BOOL);
}

/////////////////////////////////////////////////////////////////////////////
int  CH323GatekeeperStatus::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"GK_STATE",&m_gkState,GK_STATE_ENUM);			
	GET_VALIDATE_CHILD(pActionNode,"ALLOC_BANDWIDTH",&m_allocBandwidth,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"REQ_BANDWIDTH",&m_reqBandwidth,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"REQ_INTERVAL",&m_requestIntoInterval,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"GK_ROUTED",&m_gkRouted,_BOOL);				

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CH323GatekeeperStatus::NameOf() const
{
	return "CH323GatekeeperStatus";
}

/////////////////////////////////////////////////////////////////////////////
void  CH323GatekeeperStatus::SetGkState(const BYTE gkState)
{
	 m_gkState = gkState;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CH323GatekeeperStatus::GetGkState() const
{
	return m_gkState;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323GatekeeperStatus::SetReqBandwidth(const DWORD reqBandwidth)
{
	m_reqBandwidth = reqBandwidth;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CH323GatekeeperStatus::GetReqBandwidth() const
{
	return m_reqBandwidth;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323GatekeeperStatus::SetAllocBandwidth(const DWORD allocBandwidth)
{
	m_allocBandwidth = allocBandwidth;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CH323GatekeeperStatus::GetAllocBandwidth() const
{
	return m_allocBandwidth;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323GatekeeperStatus::SetRequestIntoInterval(const WORD requestIntoInterval)
{
	m_requestIntoInterval = requestIntoInterval;
}

/////////////////////////////////////////////////////////////////////////////
WORD  CH323GatekeeperStatus::GetRequestIntoInterval() const
{
	return m_requestIntoInterval;
}

/////////////////////////////////////////////////////////////////////////////
void  CH323GatekeeperStatus::SetGkRouted(const BYTE gkRouted)
{
	m_gkRouted = gkRouted;
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CH323GatekeeperStatus::GetGkRouted() const
{
	return m_gkRouted;
}

