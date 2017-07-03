// NetworkParameters.cpp: implementation of the CNetworkParameters class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "NetworkParameters.h"
#include "SystemFunctions.h"
#include "IpRouter.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "StringsLen.h"
#include "ApiStatuses.h"

// ------------------------------------------------------------
CNetworkParameters::CNetworkParameters ()
{
}


// ------------------------------------------------------------
CNetworkParameters::CNetworkParameters (const NETWORK_PARAMS_S netParams)
{
	memcpy(&m_networkParametersStruct, &netParams, sizeof(NETWORK_PARAMS_S));
}


// ------------------------------------------------------------
CNetworkParameters::~CNetworkParameters ()
{
}


// ------------------------------------------------------------
void CNetworkParameters::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement *pNetParamsNode=NULL, *pIpRoutersListNode=NULL;
	
	pNetParamsNode = pFatherNode->AddChildNode("NETWORK_PARAMETERS");

	pNetParamsNode->AddChildNode("IS_DHCP_IN_USE", m_networkParametersStruct.isDhcpInUse, _BOOL);
	pNetParamsNode->AddChildNode("DHCP_SERVER_ADDRESS", m_networkParametersStruct.dhcpServerIpAddress, IP_ADDRESS);
	pNetParamsNode->AddChildNode("DHCP_STATE", m_networkParametersStruct.dhcpState, DHCP_STATE_ENUM);
	pNetParamsNode->AddChildNode("SUBNET_MASK", m_networkParametersStruct.subnetMask, IP_ADDRESS);
	pNetParamsNode->AddChildNode("DEFAULT_GATEWAY", m_networkParametersStruct.defaultGateway, IP_ADDRESS);

	pIpRoutersListNode = pNetParamsNode->AddChildNode("IP_ROUTERS_LIST");
	for (int i=0; i<MAX_ROUTERS_IN_H323_SERVICE; i++)
	{
		CIpRouter ipRouter(m_networkParametersStruct.ipRouter[i]);
		ipRouter.SerializeXml(pIpRoutersListNode);
	}

	pNetParamsNode->AddChildNode("VLAN_MODE", m_networkParametersStruct.vLanMode);
	pNetParamsNode->AddChildNode("VLAN_ID", m_networkParametersStruct.vLanId);

    pNetParamsNode->AddChildNode("DEFAULT_GATEWAY_IP_V6", (char*)(m_networkParametersStruct.defaultGatewayIPv6));
}


// ------------------------------------------------------------
int	 CNetworkParameters::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;
	CXMLDOMElement *pIpRoutersListNode = NULL, *pCurIpRouterNode=NULL;

	GET_VALIDATE_CHILD(pActionNode, "IS_DHCP_IN_USE", &m_networkParametersStruct.isDhcpInUse, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "DHCP_SERVER_ADDRESS", &m_networkParametersStruct.dhcpServerIpAddress, IP_ADDRESS);
	GET_VALIDATE_CHILD(pActionNode, "DHCP_STATE", &m_networkParametersStruct.dhcpState, DHCP_STATE_ENUM);
	GET_VALIDATE_CHILD(pActionNode, "SUBNET_MASK", &m_networkParametersStruct.subnetMask, IP_ADDRESS);
	GET_VALIDATE_CHILD(pActionNode, "DEFAULT_GATEWAY", &m_networkParametersStruct.defaultGateway, IP_ADDRESS);


	GET_CHILD_NODE(pActionNode, "IP_ROUTERS_LIST", pIpRoutersListNode);
	if (pIpRoutersListNode)
	{
		GET_FIRST_CHILD_NODE(pIpRoutersListNode, "IP_ROUTER", pCurIpRouterNode);

		int routerIndex = 0;
		while( pCurIpRouterNode  &&  (routerIndex < MAX_ROUTERS_IN_H323_SERVICE) )
		{
			CIpRouter ipRouter(m_networkParametersStruct.ipRouter[routerIndex]);
			nStatus = ipRouter.DeSerializeXml(pCurIpRouterNode,pszError);

			if( STATUS_OK != nStatus )
				return nStatus;

			routerIndex++;
			GET_NEXT_CHILD_NODE(pIpRoutersListNode, "IP_ROUTER", pCurIpRouterNode);
		}
	}


	GET_VALIDATE_CHILD(pActionNode, "VLAN_MODE", &m_networkParametersStruct.vLanMode, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode, "VLAN_ID", &m_networkParametersStruct.vLanId, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pActionNode, "DEFAULT_ROUTER_IP_V6", m_networkParametersStruct.defaultGatewayIPv6, _0_TO_IPV6_ADDRESS_LENGTH);

	return STATUS_OK;
}


// ------------------------------------------------------------
void  CNetworkParameters::Dump(ostream& msg) const
{
	char dhcpServerIpAddressStr[IP_ADDRESS_LEN],
	     subnetMaskStr[IP_ADDRESS_LEN],
	     defaultGatewayIPv4Str[IP_ADDRESS_LEN],
	     defaultGatewayIPv6Str[IPV6_ADDRESS_LEN];
	
	SystemDWORDToIpString(m_networkParametersStruct.dhcpServerIpAddress,   dhcpServerIpAddressStr);
	SystemDWORDToIpString(m_networkParametersStruct.subnetMask,            subnetMaskStr);
	SystemDWORDToIpString(m_networkParametersStruct.defaultGateway, defaultGatewayIPv4Str);
	strncpy(defaultGatewayIPv6Str, (char*)(m_networkParametersStruct.defaultGatewayIPv6), sizeof(defaultGatewayIPv6Str) - 1);
	defaultGatewayIPv6Str[sizeof(defaultGatewayIPv6Str) - 1] = '\0';
	
	msg.setf(ios::left,ios::adjustfield);
	msg.setf(ios::showbase);
	
	msg << "\n===== NetworkParameters::Dump =====\n"
	    << "DHCP In Use: "            << m_networkParametersStruct.isDhcpInUse << ", "
		<< "Dhcp Server Ip Address: " << dhcpServerIpAddressStr                << ", "
		<< "Dhcp State: "             << m_networkParametersStruct.dhcpState   << "\n"
		<< "Subnet Mask: "            << subnetMaskStr                         << ", "
		<< "Default Gateway IPv4: "   << defaultGatewayIPv4Str                 << ", "
		<< "Default Gateway IPv6: "   << defaultGatewayIPv6Str                 << ", "
		<< "VLan Mode: "              << m_networkParametersStruct.vLanMode    << ", "
		<< "VLan Id: "                << m_networkParametersStruct.vLanId      << "\n";

	for (int i=0; i<MAX_ROUTERS_IN_H323_SERVICE; i++)
	{
		msg << "== IpRouter " << i << ": " << CIpRouter(m_networkParametersStruct.ipRouter[i]);
	}
}


// ------------------------------------------------------------
CNetworkParameters& CNetworkParameters::operator = (const CNetworkParameters &rOther)
{
	memcpy( &m_networkParametersStruct,
		    &rOther.m_networkParametersStruct,
			sizeof(NETWORK_PARAMS_S) );

	return *this;
}


// ------------------------------------------------------------
NETWORK_PARAMS_S CNetworkParameters::GetNetworkParametersStruct()
{
	return m_networkParametersStruct;
}


// ------------------------------------------------------------
BOOL CNetworkParameters::GetIsDhcpInUse () // TRUE/FALSE
{
	return m_networkParametersStruct.isDhcpInUse;
}


// ------------------------------------------------------------
void CNetworkParameters::SetIsDhcpInUse (const BOOL isInUse)
{
	m_networkParametersStruct.isDhcpInUse = isInUse;
}

// ------------------------------------------------------------
DWORD CNetworkParameters::GetDhcpServerIpAddress ()
{
	return m_networkParametersStruct.dhcpServerIpAddress;
}


// ------------------------------------------------------------
void CNetworkParameters::SetDhcpServerIpAddress (const DWORD ipAdd)
{
	m_networkParametersStruct.dhcpServerIpAddress = ipAdd;
}


// ------------------------------------------------------------
eDHCPState CNetworkParameters::GetDhcpState ()
{
	return (eDHCPState)(m_networkParametersStruct.dhcpState);
}


// ------------------------------------------------------------
void CNetworkParameters::SetDhcpState (const eDHCPState state)
{
	m_networkParametersStruct.dhcpState = state;
}


// ------------------------------------------------------------
DWORD CNetworkParameters::GetSubnetMask ()
{
	return m_networkParametersStruct.subnetMask;
}


// ------------------------------------------------------------
void CNetworkParameters::SetSubnetMask (const DWORD subMask)
{
	m_networkParametersStruct.subnetMask = subMask;
}

	
// ------------------------------------------------------------
void   CNetworkParameters::SetDefaultGatewayIPv4(const DWORD defaultGateway)
{
	m_networkParametersStruct.defaultGateway = defaultGateway;

}

// ------------------------------------------------------------
DWORD  CNetworkParameters::GetDefaultGatewayIPv4()
{
	return m_networkParametersStruct.defaultGateway;
}


// ------------------------------------------------------------
void CNetworkParameters::SetDefaultGatewayIPv6(const char* defaultGateway)
{
	strncpy( (char*)(m_networkParametersStruct.defaultGatewayIPv6),
			 defaultGateway,
			 IPV6_ADDRESS_LEN );
}


// ------------------------------------------------------------
char* CNetworkParameters::GetDefaultGatewayIPv6() const
{
	return (char*)(m_networkParametersStruct.defaultGatewayIPv6);
}


// ------------------------------------------------------------
DWORD CNetworkParameters::GetVLanMode ()
{
	return m_networkParametersStruct.vLanMode;
}


// ------------------------------------------------------------
void CNetworkParameters::SetVLanMode (const DWORD mode)
{
	m_networkParametersStruct.vLanMode = mode;
}


// ------------------------------------------------------------
DWORD CNetworkParameters::GetVLanId ()
{
	return m_networkParametersStruct.vLanId;
}


// ------------------------------------------------------------
void CNetworkParameters::SetVLanId (const DWORD id)
{
	m_networkParametersStruct.vLanId = id;
}


// ------------------------------------------------------------
void CNetworkParameters::SetData(NETWORK_PARAMS_S netParams)
{
	memcpy(&m_networkParametersStruct, &netParams, sizeof(NETWORK_PARAMS_S));
}


// ------------------------------------------------------------
