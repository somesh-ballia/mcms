// CIPv6.cpp: implementation of the CIPv4 class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "IpV6.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "ObjString.h"
#include "ApiStatuses.h"

extern char* IpV6ConfigurationTypeToString(APIU32 v6Type, bool caps = false);


// ------------------------------------------------------------
CIPv6::CIPv6 ()
{
}


// ------------------------------------------------------------
CIPv6::CIPv6 (const IPV6_S iPv6)
{
	memcpy(&m_ipV6Struct, &iPv6, sizeof(IPV6_S));
}


// ------------------------------------------------------------
CIPv6::~CIPv6 ()
{
}


// ------------------------------------------------------------
void CIPv6::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pIpV6Node = pFatherNode->AddChildNode("IP_V6_INTERFACE");

	pIpV6Node->AddChildNode("IP_V6_CONFIGURATION_TYPE",	m_ipV6Struct.configurationType, IP_V6_CONFIG_TYPE_ENUM);
	pIpV6Node->AddChildNode("IP_V6",					(char*)(m_ipV6Struct.iPv6Address));
	pIpV6Node->AddChildNode("_6To4_RELAY_ADDRESS",		m_ipV6Struct._6To4RelayAddress);

	/*to be defined later - when MPL is ready for IpV6*/
//	pIpV6Node->AddChildNode("IP_V6_SUBNET_MASK",		m_ipV6Struct.ipV6SubnetMask);
}


// ------------------------------------------------------------
int	 CIPv6::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode,"IP_V6_CONFIGURATION_TYPE",	&m_ipV6Struct.configurationType, IP_V6_CONFIG_TYPE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"IP_V6",						(char*)(&m_ipV6Struct.iPv6Address), _0_TO_IPV6_ADDRESS_LENGTH);
	GET_VALIDATE_CHILD(pActionNode,"_6To4_RELAY_ADDRESS",		&m_ipV6Struct._6To4RelayAddress, _0_TO_DWORD);

	/*to be defined later - when MPL is ready for IpV6*/
//	GET_VALIDATE_CHILD(pActionNode,"IP_V6_SUBNET_MASK",			&m_ipV6Struct.ipV6SubnetMask, _0_TO_DWORD);

	return STATUS_OK;
}


// ------------------------------------------------------------
void  CIPv6::Dump(ostream& msg) const
{
	msg.setf(ios::left,ios::adjustfield);
	msg.setf(ios::showbase);
	
	msg << "\n===== IPv6::Dump =====   "
	    << "Configuration Type: ";
	
	char* v6ConfigurationTypeStr = ::IpV6ConfigurationTypeToString(m_ipV6Struct.configurationType);
	if (v6ConfigurationTypeStr)
	{
		msg << v6ConfigurationTypeStr;
	}
	else
	{
		msg << "(invalid: " << m_ipV6Struct.configurationType << ")";
	}
	
	msg	<< ", Ip Address: "         << m_ipV6Struct.iPv6Address
		<< ", 6To4 Relay Address: " << m_ipV6Struct._6To4RelayAddress;

	/*to be defined later - when MPL is ready for IpV6*/
//		<< ", subnet Mask: "		<< m_ipV6Struct.ipV6SubnetMask;

}

// ------------------------------------------------------------
CIPv6& CIPv6::operator = (const CIPv6 &rOther)
{
	memcpy(&m_ipV6Struct, &rOther.m_ipV6Struct, sizeof(IPV6_S));

	return *this;
}


// ------------------------------------------------------------
IPV6_S CIPv6::GetIpV6Struct()
{
	return m_ipV6Struct;
}


// ------------------------------------------------------------
void CIPv6::SetIpV6Struct(CIPv6 iPv6)
{
	memcpy(&m_ipV6Struct, &iPv6.m_ipV6Struct, sizeof(IPV6_S));
}


// ------------------------------------------------------------
eV6ConfigurationType CIPv6::GetConfigurationType ()
{
	return (eV6ConfigurationType)(m_ipV6Struct.configurationType);
}


// ------------------------------------------------------------
void CIPv6::SetConfigurationType (const eV6ConfigurationType type)
{
	m_ipV6Struct.configurationType = type;
}


// ------------------------------------------------------------
BYTE* CIPv6::GetiPv6Address ()
{
	return m_ipV6Struct.iPv6Address;
}


// ------------------------------------------------------------
void CIPv6::SetiPv6Address (const BYTE *address)
{
	memcpy(&m_ipV6Struct.iPv6Address, address, IPV6_ADDRESS_LEN);
}


// ------------------------------------------------------------
DWORD CIPv6::Get6To4RelayAddress ()
{
	return m_ipV6Struct._6To4RelayAddress;
}


// ------------------------------------------------------------
void CIPv6::Set6To4RelayAddress (const DWORD address)
{
	m_ipV6Struct._6To4RelayAddress = address;
}


// ------------------------------------------------------------
DWORD CIPv6::GetIpV6SubnetMask ()
{
	/*to be defined later - when MPL is ready for IpV6*/
//	return m_ipV6Struct.ipV6SubnetMask;
	return 0;
}


// ------------------------------------------------------------
void CIPv6::SetIpV6SubnetMask (const DWORD subnetMask)
{
	/*to be defined later - when MPL is ready for IpV6*/
//	m_ipV6Struct.ipV6SubnetMask = subnetMask;
}


// ------------------------------------------------------------
void CIPv6::SetData(IPV6_S iPv6)
{
	memcpy(&m_ipV6Struct, &iPv6, sizeof(IPV6_S));
}


// ------------------------------------------------------------
