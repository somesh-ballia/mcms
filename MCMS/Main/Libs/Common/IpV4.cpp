// CIPv4.cpp: implementation of the CIPv4 class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "IpV4.h"
#include "SystemFunctions.h"
#include "InitCommonStrings.h"
#include "StatusesGeneral.h"
#include "StringsLen.h"
#include "ApiStatuses.h"

// ------------------------------------------------------------
CIPv4::CIPv4 ()
{
}


// ------------------------------------------------------------
CIPv4::CIPv4 (const IPV4_S iPv4)
{
	memcpy(&m_ipV4Struct, &iPv4, sizeof(IPV4_S));
}


// ------------------------------------------------------------
CIPv4::~CIPv4 ()
{
}


// ------------------------------------------------------------
void CIPv4::SerializeXml(CXMLDOMElement* pFatherNode)
{
	CXMLDOMElement* pIpV4Node = pFatherNode->AddChildNode("IP_V4_INTERFACE");

	pIpV4Node->AddChildNode("IS_DHCP_IN_USE", m_ipV4Struct.isDHCPv4InUse, _BOOL);
	pIpV4Node->AddChildNode("IP_ADDRESS",     m_ipV4Struct.iPv4Address,   IP_ADDRESS);
}


// ------------------------------------------------------------
int	 CIPv4::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError)
{
	int nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "IS_DHCP_IN_USE", &m_ipV4Struct.isDHCPv4InUse, _BOOL);
	GET_VALIDATE_CHILD(pActionNode, "IP_ADDRESS",     &m_ipV4Struct.iPv4Address,   IP_ADDRESS);

	return STATUS_OK;
}


// ------------------------------------------------------------
void  CIPv4::Dump(ostream& msg) const
{
	char ipAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(m_ipV4Struct.iPv4Address, ipAddressStr);
	
	msg.setf(ios::left,ios::adjustfield);
	msg.setf(ios::showbase);
	
	msg << "\n===== IPv4::Dump =====   "
	    << "DHCP In Use: " << m_ipV4Struct.isDHCPv4InUse << ",   "
		<< "Ip Address: "  << ipAddressStr;
}


// ------------------------------------------------------------
CIPv4& CIPv4::operator = (const CIPv4 &rOther)
{
	memcpy(&m_ipV4Struct, &rOther.m_ipV4Struct, sizeof(IPV4_S));

	return *this;
}


// ------------------------------------------------------------
IPV4_S CIPv4::GetIpV4Struct()
{
	return m_ipV4Struct;
}


// ------------------------------------------------------------
void CIPv4::SetIpV4Struct(CIPv4 iPv4)
{
	memcpy(&m_ipV4Struct, &iPv4.m_ipV4Struct, sizeof(IPV4_S));
}


// ------------------------------------------------------------
BOOL CIPv4::GetIsDHCPv4InUse ()
{
	return m_ipV4Struct.isDHCPv4InUse;
}


// ------------------------------------------------------------
void CIPv4::SetIsDHCPv4InUse (const BOOL isInUse)
{
	m_ipV4Struct.isDHCPv4InUse = isInUse;
}


// ------------------------------------------------------------
DWORD CIPv4::GetiPv4Address ()
{
	return m_ipV4Struct.iPv4Address;
}


// ------------------------------------------------------------
void CIPv4::SetiPv4Address (const DWORD address)
{
	m_ipV4Struct.iPv4Address = address;
}


// ------------------------------------------------------------
void CIPv4::SetData(IPV4_S iPv4)
{
	memcpy(&m_ipV4Struct, &iPv4, sizeof(IPV4_S));
}


// ------------------------------------------------------------
