// CIpRouter.cpp: implementation of the CIpRouter class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "IpRouter.h"
#include "SystemFunctions.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "StringsLen.h"
#include "ApiStatuses.h"

// ------------------------------------------------------------
CIpRouter::CIpRouter ()
{
	m_ipRouterStruct.routerIp   = 0;
	m_ipRouterStruct.remoteIp   = 0;
	m_ipRouterStruct.subnetMask = 0;
	m_ipRouterStruct.remoteFlag = H323_REMOTE_NETWORK;
}


// ------------------------------------------------------------
CIpRouter::CIpRouter (const IP_ROUTER_S ipRouter)
{
	memcpy(&m_ipRouterStruct, &ipRouter, sizeof(IP_ROUTER_S));
}


// ------------------------------------------------------------
CIpRouter::~CIpRouter ()
{
}


// ------------------------------------------------------------
void CIpRouter::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pIpRouterNode = pFatherNode->AddChildNode("IP_ROUTER");

	pIpRouterNode->AddChildNode("ROUTER_IP",   m_ipRouterStruct.routerIp,   IP_ADDRESS);
	pIpRouterNode->AddChildNode("REMOTE_IP",   m_ipRouterStruct.remoteIp,   IP_ADDRESS);
	pIpRouterNode->AddChildNode("REMOTE_FLAG", m_ipRouterStruct.remoteFlag);
	pIpRouterNode->AddChildNode("SUBNET_MASK", m_ipRouterStruct.subnetMask, IP_ADDRESS);
}


// ------------------------------------------------------------
int	 CIpRouter::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"ROUTER_IP",   &m_ipRouterStruct.routerIp,   IP_ADDRESS);
	GET_VALIDATE_CHILD(pActionNode,"REMOTE_IP",   &m_ipRouterStruct.remoteIp,   IP_ADDRESS);
	GET_VALIDATE_CHILD(pActionNode,"REMOTE_FLAG", &m_ipRouterStruct.remoteFlag, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode,"SUBNET_MASK", &m_ipRouterStruct.subnetMask, IP_ADDRESS);

	return STATUS_OK;
}


// ------------------------------------------------------------
void  CIpRouter::Dump(ostream& msg) const
{
	char routerIpAddressStr[IP_ADDRESS_LEN],
	     remoteIpAddressStr[IP_ADDRESS_LEN],
	     subnetMaskStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(m_ipRouterStruct.routerIp,   routerIpAddressStr);
	SystemDWORDToIpString(m_ipRouterStruct.remoteIp,   remoteIpAddressStr);
	SystemDWORDToIpString(m_ipRouterStruct.subnetMask, subnetMaskStr);

	msg.setf(ios::left,ios::adjustfield);
	msg.setf(ios::showbase);
	
	msg << "Router Ip: "   << routerIpAddressStr          << ", "
		<< "Remote Ip: "   << remoteIpAddressStr          << ", "
		<< "Subnet Mask: " << subnetMaskStr               << ", "
		<< "Remote Flag: " << m_ipRouterStruct.remoteFlag << "\n";
}


// ------------------------------------------------------------
CIpRouter& CIpRouter::operator = (const CIpRouter &rOther)
{
	memcpy(&m_ipRouterStruct, &rOther.m_ipRouterStruct, sizeof(IP_ROUTER_S));

	return *this;
}


// ------------------------------------------------------------
IP_ROUTER_S CIpRouter::GetIpRouterStruct()
{
	return m_ipRouterStruct;
}


// ------------------------------------------------------------
void CIpRouter::SetIpRouterStruct(CIpRouter ipRouter)
{
	memcpy(&m_ipRouterStruct, &ipRouter.m_ipRouterStruct, sizeof(IP_ROUTER_S));
}


// ------------------------------------------------------------
void CIpRouter::SetRouterIp (const DWORD router_Ip)
{
	m_ipRouterStruct.routerIp = router_Ip;
}

// ------------------------------------------------------------
DWORD CIpRouter::GetRouterIp ()
{
	return m_ipRouterStruct.routerIp;
}

// ------------------------------------------------------------
void CIpRouter::SetRemoteIp (const DWORD remote_Ip)
{
	m_ipRouterStruct.remoteIp = remote_Ip;
}


// ------------------------------------------------------------
DWORD CIpRouter::GetRemoteIp ()
{
	return m_ipRouterStruct.remoteIp;
}


// ------------------------------------------------------------
void CIpRouter::SetRemoteFlag (const BYTE remote_Flag)
{
	m_ipRouterStruct.remoteFlag = (DWORD)remote_Flag;
}


// ------------------------------------------------------------
BYTE CIpRouter::GetRemoteFlag ()
{
	return (BYTE)(m_ipRouterStruct.remoteFlag);
}


// ------------------------------------------------------------
void CIpRouter::SetSubnetMask (const DWORD netMask)
{
	m_ipRouterStruct.subnetMask = netMask;
}


// ------------------------------------------------------------
DWORD CIpRouter::GetSubnetMask ()
{
	return m_ipRouterStruct.subnetMask;
}


// ------------------------------------------------------------
void CIpRouter::SetData(IP_ROUTER_S ipRouter)
{
	memcpy(&m_ipRouterStruct, &ipRouter, sizeof(IP_ROUTER_S));
}


// ------------------------------------------------------------
