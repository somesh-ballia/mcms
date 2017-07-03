/*
 * SipBWStatus.cpp
 *
 *  Created on: Jun 28, 2011
 *      Author: mvolovik
 */

#include "SipBWStatus.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"

CSIPBandwidthAllocationStatus::CSIPBandwidthAllocationStatus()
{
	m_reqBandwidth		  = -1;
	m_allocBandwidth	  = -1;
}

/////////////////////////////////////////////////////////////////////////////
CSIPBandwidthAllocationStatus::CSIPBandwidthAllocationStatus(const CSIPBandwidthAllocationStatus &other)
:CPObject(other)
{
	*this = other;
}

/////////////////////////////////////////////////////////////////////////////
CSIPBandwidthAllocationStatus& CSIPBandwidthAllocationStatus::operator = (const CSIPBandwidthAllocationStatus& other)
{
	m_reqBandwidth		  = other.m_reqBandwidth;
	m_allocBandwidth	  = other.m_allocBandwidth;

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CSIPBandwidthAllocationStatus::~CSIPBandwidthAllocationStatus()
{
}

/////////////////////////////////////////////////////////////////////////////
void CSIPBandwidthAllocationStatus::SerializeXml(CXMLDOMElement* pFatherNode)
{
	//PTRACE2INT(eLevelInfoNormal,"CSIPBandwidthAllocationStatus ALLOC_BANDWIDTH: ",m_allocBandwidth);
	//PTRACE2INT(eLevelInfoNormal,"CSIPBandwidthAllocationStatus REQ_BANDWIDTH: ",m_reqBandwidth);
	CXMLDOMElement* pGKStatusNode = pFatherNode->AddChildNode("CAC_STATUS");

	pGKStatusNode->AddChildNode("ALLOC_BANDWIDTH",m_allocBandwidth);
	pGKStatusNode->AddChildNode("REQ_BANDWIDTH",m_reqBandwidth);
}

/////////////////////////////////////////////////////////////////////////////
int  CSIPBandwidthAllocationStatus::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"ALLOC_BANDWIDTH",&m_allocBandwidth,_0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"REQ_BANDWIDTH",&m_reqBandwidth,_0_TO_DWORD);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
const char*  CSIPBandwidthAllocationStatus::NameOf() const
{
	return "CSIPBandwidthAllocationStatus";
}


/////////////////////////////////////////////////////////////////////////////
void  CSIPBandwidthAllocationStatus::SetReqBandwidth(const DWORD reqBandwidth)
{
	m_reqBandwidth = reqBandwidth;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CSIPBandwidthAllocationStatus::GetReqBandwidth() const
{
	return m_reqBandwidth;
}

/////////////////////////////////////////////////////////////////////////////
void  CSIPBandwidthAllocationStatus::SetAllocBandwidth(const DWORD allocBandwidth)
{
	m_allocBandwidth = allocBandwidth;
}

/////////////////////////////////////////////////////////////////////////////
DWORD  CSIPBandwidthAllocationStatus::GetAllocBandwidth() const
{
	return m_allocBandwidth;
}

