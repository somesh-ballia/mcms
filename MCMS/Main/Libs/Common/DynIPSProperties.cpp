// CDynIPSProperties.cpp: implementation of the CDynIPSProperties class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Keeping dynamic properties of ip service
//========   ==============   =====================================================================

#include "DynIPSProperties.h"
#include "psosxml.h"
#include "XmlApi.h"
#include "TraceStream.h"
#include "ProcessBase.h"


/*-----------------------------------------------------------------------------------
	CServiceInfo. 
	MCMS <-- CS Module (as a buffer)
-----------------------------------------------------------------------------------*/

//////////////////////////////////////////////////////////////////////
CServiceInfo::CServiceInfo()
{
	m_Id		= 0;
	m_Status	= eServiceStateFailed;
	m_Name		= THE_STRING;
}

//////////////////////////////////////////////////////////////////////
CServiceInfo::CServiceInfo(const CServiceInfo &other)
:CSerializeObject(other)
{
	*this = other;	
}

//////////////////////////////////////////////////////////////////////
CServiceInfo& CServiceInfo::operator=(const CServiceInfo &rHnd)
{
	if(&rHnd != this)
	{
		m_Name		= rHnd.m_Name;
		m_Id		= rHnd.m_Id;
		m_Status	= rHnd.m_Status;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CServiceInfo::operator==(const CServiceInfo &rHnd)const
{
	if(this == &rHnd)
		return true;
	
	if (m_Name != rHnd.m_Name ||
		m_Id   != rHnd.m_Id	  ||
		m_Status != rHnd.m_Status)
			return false;
		
	return true; 
}

////////////////////////////////////////////////////////////////////////////
bool CServiceInfo::operator!=(const CServiceInfo& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CServiceInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{
	thisNode->AddChildNode("NAME"	, m_Name.c_str());
	thisNode->AddChildNode("ID"		, m_Id);
	thisNode->AddChildNode("STATUS"	, m_Status);
// should be 	
//	thisNode->AddChildNode("STATUS", m_Status, IP_SERVICE_STATE_ENUM);
}








/*-----------------------------------------------------------------------------------
	CIPv4Address. 
	MCMS <-- CS Module (part of IpInfo, ...)
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CIPv4Address::CIPv4Address(DWORD ip)
{
	m_Ip = ip;
	m_ElementName = "IP";
}

//////////////////////////////////////////////////////////////////////
CIPv4Address::CIPv4Address(const CIPv4Address &other)
:CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CIPv4Address& CIPv4Address::operator=(const CIPv4Address &rHnd)
{
	if(&rHnd != this)
	{
		m_Ip = rHnd.m_Ip;
		m_ElementName = rHnd.m_ElementName;
	}
	return *this;
}
//////////////////////////////////////////////////////////////////////
bool CIPv4Address::operator==(const CIPv4Address &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (m_ElementName != rHnd.m_ElementName)
		return false;
	
	if (m_Ip != rHnd.m_Ip)
		return false;
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CIPv4Address::operator!=(const CIPv4Address& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CIPv4Address::SerializeXml(CXMLDOMElement*& thisNode) const
{
	PASSERTMSG(m_ElementName.empty(), "element name is empty");

	thisNode->AddChildNode(m_ElementName.c_str(), m_Ip, IP_ADDRESS);
}

//////////////////////////////////////////////////////////////////////
void CIPv4Address::SetElementName(const char *elementName)
{
	PASSERTMSG(0 == strlen(elementName), "element name is empty");
	
	m_ElementName = elementName;
}
	












/*-----------------------------------------------------------------------------------
	CDnsServerList. 
	MCMS <-- CS Module (part of IpInfo)
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CDnsServerList::CDnsServerList()
{
	m_Length = MAX_NUM_DNS;
	for(int i = 0 ; i < MAX_NUM_DNS ; i++)
	{
		m_DnsIPv4[i].SetElementName("DNS_SERVER_IP");

		memset(m_DnsIPv6[i].ip, 0, IPV6_ADDRESS_BYTES_LEN);
		m_DnsIPv6[i].scopeId = eScopeIdSite;
	}
}

//////////////////////////////////////////////////////////////////////
CDnsServerList::CDnsServerList(const CDnsServerList &other)
:CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CDnsServerList& CDnsServerList::operator=(const CDnsServerList &rHnd)
{
	if(&rHnd != this)
	{
		m_Length = rHnd.m_Length;
		for(int i = 0 ; i < MAX_NUM_DNS ; i++)
		{
			m_DnsIPv4[i] = rHnd.m_DnsIPv4[i];
	    	memcpy( &(m_DnsIPv6[i]), &(rHnd.m_DnsIPv6[i]), sizeof(ipAddressV6If) );
		}
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CDnsServerList::operator==(const CDnsServerList &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (m_Length != rHnd.m_Length)
		return false;
	
	for (int i=0; i<MAX_NUM_DNS; i++)
	{
		if (m_DnsIPv4[i] != rHnd.m_DnsIPv4[i])
			return false;
		
		if (memcmp(&m_DnsIPv6[i], &(rHnd.m_DnsIPv6[i]), sizeof(ipAddressV6If)) != 0)
			return false;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CDnsServerList::operator!=(const CDnsServerList& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CDnsServerList::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("DNS_SERVERS_LIST");

	for(DWORD i = 0 ; i < m_Length ; i++)
	{
		m_DnsIPv4[i].SerializeXml(childNode);
//		childNode->AddIPv6ChildNode("IP_V6", m_DnsIPv6[i].ip);
	}
}

//////////////////////////////////////////////////////////////////////
void CDnsServerList::SetDnsIPv4s(DWORD *dnsIps, DWORD length)
{
	if(length > MAX_NUM_DNS)
	{
		PASSERTMSG(1, "length > MAX_NUM_DNS");
		length = MAX_NUM_DNS;
	}
	
	for(DWORD i = 0 ; i < length ; i++)
	{
		m_DnsIPv4[i].SetIpAddress(dnsIps[i]);
	}
}

//////////////////////////////////////////////////////////////////////
void CDnsServerList::SetDnsIPv6s(char **dnsIps, DWORD length)
{
	if(length > MAX_NUM_DNS)
	{
		PASSERTMSG(1, "length > MAX_NUM_DNS");
		length = MAX_NUM_DNS;
	}
	
	mcTransportAddress tmpIPv6Addr;
	for(DWORD i = 0 ; i < length ; i++)
	{
		// ===== 1. address
		memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
		::stringToIpV6( &tmpIPv6Addr, dnsIps[i] );
		memcpy( &(m_DnsIPv6[i].ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );
	    
		// ===== 2. scopeId
		m_DnsIPv6[i].scopeId = ::getScopeId( dnsIps[i] );
	}
}











/*-----------------------------------------------------------------------------------
	CIpInfo. 
	MCMS <-- CS Module (as a buffer)
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CIpInfo::CIpInfo()
{
	m_IPv4.SetElementName("IP");

	memset(m_IPv6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
	m_IPv6.scopeId = eScopeIdSite;

	m_SubnetMask.SetElementName("SUBNET_MASK_ADDRESS");
	m_IPv4DefaultGateway.SetElementName("DEFAULT_GATEWAY_ADDRESS");
	m_DhcpStatusType = eDHCPStateDisable;
	m_DhcpServerIp.SetElementName("DHCP_SERVER_IP");
	m_DomainName = THE_STRING;
	m_NatIp.SetElementName("NAT_IP");
}

//////////////////////////////////////////////////////////////////////
CIpInfo::CIpInfo(const CIpInfo &other)
:CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CIpInfo& CIpInfo::operator=(const CIpInfo &rHnd)
{
	if(&rHnd == this)
	{
		return *this;
	}

	m_IPv4				= rHnd.m_IPv4;
	memcpy( &m_IPv6, &(rHnd.m_IPv6), sizeof(ipAddressV6If) );
	m_SubnetMask		= rHnd.m_SubnetMask;
	m_IPv4DefaultGateway= rHnd.m_IPv4DefaultGateway;
	m_IPv6DefaultGateway= rHnd.m_IPv6DefaultGateway;
	m_DhcpStatusType	= rHnd.m_DhcpStatusType;
	m_DhcpServerIp		= rHnd.m_DhcpServerIp;
	m_DnsServerList		= rHnd.m_DnsServerList;
	m_DomainName		= rHnd.m_DomainName;
	m_NatIp				= rHnd.m_NatIp;

	return *this;
}
//////////////////////////////////////////////////////////////////////
bool CIpInfo::operator==(const CIpInfo &rHnd)const
{	
	if(this == &rHnd)
		return true;
	
	if (m_IPv4 != rHnd.m_IPv4)
		return false;
	
	if ( memcmp(&m_IPv6, &(rHnd.m_IPv6), sizeof(ipAddressV6If)) != 0)
		return false;	
	
	if (m_SubnetMask != rHnd.m_SubnetMask)
		return false;
	
	if (m_IPv4DefaultGateway != rHnd.m_IPv4DefaultGateway)
		return false;

	if (m_IPv6DefaultGateway != rHnd.m_IPv6DefaultGateway)
		return false;
	
	if (m_DhcpStatusType != rHnd.m_DhcpStatusType)
		return false;
	
	if (m_NatIp != rHnd.m_NatIp)
		return false;
	
	if (m_DhcpServerIp != rHnd.m_DhcpServerIp)
		return false;
	
	if (m_DomainName != rHnd.m_DomainName)
		return false;
	
	if (m_DnsServerList != rHnd.m_DnsServerList)
		return false;
	
	return true; 
}

////////////////////////////////////////////////////////////////////////////
bool CIpInfo::operator!=(const CIpInfo& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CIpInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("IP_INFO");

	m_IPv4.SerializeXml(childNode);
	m_SubnetMask.SerializeXml(childNode);
	m_IPv4DefaultGateway.SerializeXml(childNode);
	childNode->AddChildNode("DHCP_STATUS_TYPE", m_DhcpStatusType, DHCP_STATE_ENUM);
	m_DhcpServerIp.SerializeXml(childNode);
	m_DnsServerList.SerializeXml(childNode);
	childNode->AddChildNode("DOMAIN_NAME", m_DomainName.c_str());
	m_NatIp.SerializeXml(childNode);
	
//	childNode->AddIPv6ChildNode("IP_V6", m_IPv6.ip);
}
	








/*-----------------------------------------------------------------------------------
	CProxyDataContent. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CProxyDataContent::CProxyDataContent()
{
	m_Status = eServerStatusAuto;
	m_Name = THE_STRING;

	m_IPv4.SetIpAddress(0);
	m_IPv4.SetElementName("IP");

	memset(m_IPv6.ip, 0, IPV6_ADDRESS_BYTES_LEN);
	m_IPv6.scopeId = eScopeIdSite;
}

//////////////////////////////////////////////////////////////////////
CProxyDataContent::CProxyDataContent(const CProxyDataContent &other)
:CSerializeObject(other)
{
	*this = other;	
}

//////////////////////////////////////////////////////////////////////
CProxyDataContent& CProxyDataContent::operator=(const CProxyDataContent &rHnd)
{
	if(&rHnd != this)
	{
		m_ElementName	= rHnd.m_ElementName;
		m_Status		= rHnd.m_Status;
		m_IPv4			= rHnd.m_IPv4;
		m_Name			= rHnd.m_Name;
    	memcpy( &m_IPv6, &(rHnd.m_IPv6), sizeof(ipAddressV6If) );
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CProxyDataContent::operator==(const CProxyDataContent &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (m_ElementName != rHnd.m_ElementName)
		return false;
	
	if (m_Status != rHnd.m_Status)
		return false;
	
	if (m_IPv4 != rHnd.m_IPv4)
		return false;

	if (m_Name != rHnd.m_Name)
		return false;

	if ( memcmp(&m_IPv6, &(rHnd.m_IPv6), sizeof(ipAddressV6If)) != 0)
		return false;	
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CProxyDataContent::operator!=(const CProxyDataContent& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CProxyDataContent::SerializeXml(CXMLDOMElement*& thisNode) const
{
	PASSERTMSG(m_ElementName.empty(), "element name is empty");

	CXMLDOMElement *childNode = thisNode->AddChildNode(m_ElementName.c_str());

	childNode->AddChildNode("SERVER_STATUS", m_Status, SERVER_STATUS_ENUM);
	m_IPv4.SerializeXml(childNode);
	childNode->AddChildNode("NAME"	, m_Name.c_str());
//	childNode->AddIPv6ChildNode("IP_V6", m_IPv6.ip);
}

/////////////////////////////////////////////////////////////////////////////
void CProxyDataContent::GetIPv6Address(char* retStr)
{
	::ipV6ToString(m_IPv6.ip, retStr, TRUE);
}

/////////////////////////////////////////////////////////////////////////////
void CProxyDataContent::SetIPv6Address(const char* ipV6Address)
{
	// ===== 1. address
	mcTransportAddress tmpIPv6Addr;
	memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
	::stringToIpV6( &tmpIPv6Addr, (char*)ipV6Address );
	memcpy( &(m_IPv6.ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );
    
	// ===== 2. scopeId
	m_IPv6.scopeId = ::getScopeId( (char*)ipV6Address );
}













/*-----------------------------------------------------------------------------------
	CSipInfo. 
	MCMS <-- CS Module (as a buffer)
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CSipInfo::CSipInfo()
{
	m_NumOfProxys = 0;
	m_PrimaryProxy.SetElementName("PRIMARY_PROXY");
	m_AlternateProxy.SetElementName("ALTERNATE_PROXY");
}

//////////////////////////////////////////////////////////////////////
CSipInfo::CSipInfo(const CSipInfo &other)
:CSerializeObject(other)
{
	*this = other;	
}

//////////////////////////////////////////////////////////////////////
CSipInfo& CSipInfo::operator=(const CSipInfo &rHnd)
{
	if(&rHnd == this)
	{
		m_NumOfProxys		= rHnd.m_NumOfProxys;
		m_PrimaryProxy		= rHnd.m_PrimaryProxy;
		m_AlternateProxy	= rHnd.m_AlternateProxy;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CSipInfo::operator==(const CSipInfo &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (m_NumOfProxys != rHnd.m_NumOfProxys)
		return false;
	
	if (m_PrimaryProxy != rHnd.m_PrimaryProxy)
		return false;
	
	if (m_AlternateProxy != rHnd.m_AlternateProxy)
		return false;	

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CSipInfo::operator!=(const CSipInfo& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////

void CSipInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("CENTRAL_SIGNALING_PROXY_INFO");
	
	m_PrimaryProxy.SerializeXml(childNode);
	m_AlternateProxy.SerializeXml(childNode);
}
	
	
	









/*-----------------------------------------------------------------------------------
	CH323Info. 
	MCMS <-- CS Module (as a buffer)
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CH323Info::CH323Info()
{
	m_IsUpdatedFromCsModule = false;
	m_NumOfGK = 0;
	m_PrimaryGK.SetElementName("PRIMARY_GK");
	m_AlternateGK.SetElementName("ALTERNATE_GK");
}

//////////////////////////////////////////////////////////////////////
CH323Info::CH323Info(const CH323Info &other)
:CSerializeObject(other)
{
	*this = other;	
}

//////////////////////////////////////////////////////////////////////
CH323Info& CH323Info::operator=(const CH323Info &rHnd)
{
	if(&rHnd != this)
	{
		m_IsUpdatedFromCsModule = rHnd.m_IsUpdatedFromCsModule;	
		m_NumOfGK		= rHnd.m_NumOfGK;
		m_PrimaryGK		= rHnd.m_PrimaryGK;
		m_AlternateGK	= rHnd.m_AlternateGK;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CH323Info::operator==(const CH323Info &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (m_IsUpdatedFromCsModule != rHnd.m_IsUpdatedFromCsModule)
		return false;

	if (m_NumOfGK != rHnd.m_NumOfGK)
		return false;

	if (m_PrimaryGK != rHnd.m_PrimaryGK)
		return false;

	if (m_AlternateGK != rHnd.m_AlternateGK)
		return false;

	return true;
}

//////////////////////////////////////////////////////////////////////
bool CH323Info::operator!=(const CH323Info& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CH323Info::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("CENTRAL_SIGNALING_GK_INFO");
	
	m_PrimaryGK.SerializeXml(childNode);
	m_AlternateGK.SerializeXml(childNode);
}













/*-----------------------------------------------------------------------------------
	CCardIpAddress. 
	MCMS <-- Cards
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CCardIpAddress::CCardIpAddress()
{}

//////////////////////////////////////////////////////////////////////
CCardIpAddress::CCardIpAddress(const CCardIpAddress &other)
:CSerializeObject(other)
{
	*this = other;	
}

CCardIpAddress::~CCardIpAddress()
{}

//////////////////////////////////////////////////////////////////////
CCardIpAddress& CCardIpAddress::operator=(const CCardIpAddress &rHnd)
{
	if(&rHnd != this)
	{
		for(int i = 0 ; i < NUM_OF_IP_ADDRESS ; i++)
		{
			m_IpInfos[i] = rHnd.m_IpInfos[i];
		}
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CCardIpAddress::operator==(const CCardIpAddress &rHnd)const
{
	if(this == &rHnd)
		return true;

	for (int i = 0 ; i < NUM_OF_IP_ADDRESS ; i++)
	{
		if (m_IpInfos[i] != rHnd.m_IpInfos[i])
			return false;
	}
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CCardIpAddress::operator!=(const CCardIpAddress& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CCardIpAddress::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("SPAN_IP_SERVICE_INFO");
	
	for(int i = 0 ; i < NUM_OF_IP_ADDRESS ; i++)
	{
		m_IpInfos[i].SerializeXml(childNode);
	}
}

//////////////////////////////////////////////////////////////////////
void CCardIpAddress::SetIpInfo(const MEDIA_IP_CONFIG_S &param)
{	
	if(param.pqNumber < NUM_OF_IP_ADDRESS)
	{
		m_IpInfos[param.pqNumber].SetIPv4Addr(param.iPv4.iPv4Address);
	}
	else
	{
		TRACEINTO << "\nBad Index" << param.pqNumber;
		PASSERT(TRUE);
	}
}
	












/*-----------------------------------------------------------------------------------
	CCSIpInfo. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CCSIpInfo::CCSIpInfo()
{}

//////////////////////////////////////////////////////////////////////
CCSIpInfo::CCSIpInfo(const CCSIpInfo &other)
:CSerializeObject(other)
{
	*this = other;	
}

//////////////////////////////////////////////////////////////////////
CCSIpInfo::~CCSIpInfo()
{}

//////////////////////////////////////////////////////////////////////
CCSIpInfo& CCSIpInfo::operator=(const CCSIpInfo &rHnd)
{
	if(&rHnd != this)
	{
		m_IpInfo = rHnd.m_IpInfo;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CCSIpInfo::operator==(const CCSIpInfo &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (m_IpInfo != rHnd.m_IpInfo)
		return false;
		
	return true; 
}

////////////////////////////////////////////////////////////////////////////
bool CCSIpInfo::operator!=(const CCSIpInfo& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CCSIpInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("CENTRAL_SIGNALING_IP_SERVICE_INFO");
	
	m_IpInfo.SerializeXml(childNode);
}











/*-----------------------------------------------------------------------------------
	CGKInfo. 
	MCMS <-- GK
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CGKInfo::CGKInfo()
{}

//////////////////////////////////////////////////////////////////////
CGKInfo::~CGKInfo()
{}

//////////////////////////////////////////////////////////////////////
CGKInfo::CGKInfo(const CGKInfo &other)
:CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CGKInfo& CGKInfo::operator=(const CGKInfo &rHnd)
{
	if(&rHnd != this)
	{
		m_GKFullInfo = rHnd.m_GKFullInfo;
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CGKInfo::operator==(const CGKInfo &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (m_GKFullInfo != rHnd.m_GKFullInfo)
		return false;
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CGKInfo::operator!=(const CGKInfo& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CGKInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("GK_INFO");
	
	childNode->AddChildNode("SERVICE_STATUS"		, m_GKFullInfo.m_ServiceStatus);
	childNode->AddChildNode("SERVICE_OPCODE"		, m_GKFullInfo.m_ServiceOpcode, GK_SERVICE_STATUS_ENUM);
	childNode->AddChildNode("REGISTRATION_INTERVAL"	, m_GKFullInfo.m_RrqPolingInterval);
	childNode->AddChildNode("CONNECTION_STATE"		, m_GKFullInfo.m_GkConnectionState, GK_CONNECTION_STATE_ENUM);
	
	CXMLDOMElement *gkListNode = childNode->AddChildNode("DYNAMIC_GATEKEEPER_LIST");
	
	eIPv6AddressScope curV6AddressScope = eIPv6AddressScope_other;
	for(int i = 0 ; i < MAX_NUM_GK ; i++)
	{
		CXMLDOMElement *gkInfoNode = gkListNode->AddChildNode("DYNAMIC_GATEKEEPER");
		
		const GKInfoStruct &currentInfo = m_GKFullInfo.m_GKInfoStruct[i];
		
		gkInfoNode->AddChildNode("GK_ID", currentInfo.m_GkId);
		gkInfoNode->AddChildNode("IP", currentInfo.m_GkIp.addr.v4.ip, IP_ADDRESS);
		gkInfoNode->AddChildNode("NAME", currentInfo.m_Name);
		gkInfoNode->AddChildNode("ROLE", currentInfo.m_eDynamicGkRole, GK_DYNAMIC_ROLE_ENUM);
		gkInfoNode->AddIPv6ChildNode("IP_V6", currentInfo.m_GkIp.addr.v6.ip);
		
//		curV6AddressScope = GetIPv6AddressScope( (char*)(currentInfo.m_GkIp.addr.v6.ip) );
//		const char* curV6AddressScopeStr = GetIPv6AddressScopeStr(curV6AddressScope);
//		gkInfoNode->AddChildNode("IP_V6_ADDRESS_SCOPE", curV6AddressScopeStr, IP_V6_ADDRESS_SCOPE_ENUM);
	}
}
	














/*-----------------------------------------------------------------------------------
	CProxyInfo. 
	MCMS <-- Proxy
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CProxyInfo::CProxyInfo()
{
    memset(&m_Data, 0, sizeof(SIP_PROXY_STATUS_PARAMS_S));
}

//////////////////////////////////////////////////////////////////////
CProxyInfo::CProxyInfo(const CProxyInfo &other)
:CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CProxyInfo::~CProxyInfo()
{}

//////////////////////////////////////////////////////////////////////
CProxyInfo& CProxyInfo::operator=(const CProxyInfo &rHnd)
{
	if(&rHnd != this)
	{
        memcpy(&m_Data, &rHnd.m_Data, sizeof(SIP_PROXY_STATUS_PARAMS_S));
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CProxyInfo::operator==(const CProxyInfo &rHnd)const
{
	if(this == &rHnd)
		return true;
	
	if (memcmp(&m_Data, &(rHnd.m_Data), sizeof(SIP_PROXY_STATUS_PARAMS_S) != 0))
		return false;
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CProxyInfo::operator!=(const CProxyInfo& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CProxyInfo::SetSipProxyParams(const SIP_PROXY_STATUS_PARAMS_S &param)
{
    memcpy(&m_Data, &param, sizeof(SIP_PROXY_STATUS_PARAMS_S));
}

//////////////////////////////////////////////////////////////////////
void CProxyInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("SIP_SERVER_LIST_INFO");

    for(int i = 0 ; i < NUM_PROXY_SERVERS ; i++)
    {
        SerializeXmlSingleSipProxy(childNode, m_Data.ProxyList[i]);
    }
}

//////////////////////////////////////////////////////////////////////	
void CProxyInfo::SerializeXmlSingleSipProxy(CXMLDOMElement*& thisNode,
                                            const SIP_PROXY_DYNAMIC_PARAMS_S &curProxy) const
{
	CXMLDOMElement *childNode = thisNode->AddChildNode("SIP_SERVER_INFO");
    
    childNode->AddChildNode("SERVER_ROLE", curProxy.Role, SERVICE_ROLE_ENUM);
    childNode->AddChildNode("NAME", curProxy.Name);
    childNode->AddChildNode("IP", curProxy.IpV4.v4.ip, IP_ADDRESS);
    childNode->AddChildNode("SIP_SERVER_STATUS", curProxy.Status, SIP_SERVICE_STATUS_ENUM);
    childNode->AddIPv6ChildNode("IP_V6", curProxy.IpV6.v6.ip);
}




/*-----------------------------------------------------------------------------------
	CDnsInfo. 
	MCMS <-- Proxy
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CDnsInfo::CDnsInfo()
{
    memset(&m_DnsParam, 0, sizeof m_DnsParam);

//	m_DnsParam.domainName[0] = '\0';
//	m_DnsParam.ServiceId 	 = 0;
//	m_DnsParam.Ip 			 = DEFAULT_IP_ADDRESS;
//	memset(&m_DnsParam.Ip, 0 , sizeof(mcTransportAddress));
//	for(int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES; i++)
//	{
//		memset(&m_DnsParam.pAddrList[i], 0 , sizeof(ipAddressStruct));
//
//}
}

//////////////////////////////////////////////////////////////////////
CDnsInfo::CDnsInfo(const CDnsInfo &other)
:CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CDnsInfo::~CDnsInfo()
{}

//////////////////////////////////////////////////////////////////////
CDnsInfo& CDnsInfo::operator=(const CDnsInfo &rHnd)
{
	if(&rHnd != this)
	{
		memcpy(&m_DnsParam, &(rHnd.m_DnsParam), sizeof(DNS_PARAMS_IP_S));
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
bool CDnsInfo::operator==(const CDnsInfo &rHnd)const
{
	if(this == &rHnd)
		return true;

	if (memcmp(&m_DnsParam, &(rHnd.m_DnsParam), sizeof(DNS_PARAMS_IP_S)) != 0)
		return false;
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CDnsInfo::operator!=(const CDnsInfo& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CDnsInfo::SetDnsResolutionParams(const DNS_PARAMS_IP_S &param)
{
	memcpy(&m_DnsParam, &param, sizeof(DNS_PARAMS_IP_S));
}

//////////////////////////////////////////////////////////////////////
void CDnsInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{	
	CXMLDOMElement *childNode = thisNode->AddChildNode("DNS_INFO");
	
	childNode->AddChildNode("DOMAIN_NAME"	, m_DnsParam.domainName				);
//	childNode->AddChildNode("IP"			, m_DnsParam.Ip			, IP_ADDRESS);
//    childNode->AddChildNode("IP", m_DnsParam.Ip.addr.v4.ip, IP_ADDRESS);
//    childNode->AddIPv6ChildNode("IP_V6", m_DnsParam.Ip.addr.v6.ip);


}





/*-----------------------------------------------------------------------------------
	CIceInfo. 
	MCMS <-- Cards
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CIceInfo::CIceInfo()
{
	for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
	{
		memset(&m_IceInitIndArray[i], 0, sizeof(ICE_INIT_IND_S));
		m_IceInitIndArray[i].STUN_Pass_status = eIceServerUnavailble;
		m_IceInitIndArray[i].STUN_udp_status = eIceServerUnavailble;
		m_IceInitIndArray[i].STUN_tcp_status = eIceServerUnavailble;
		m_IceInitIndArray[i].Relay_udp_status = eIceServerUnavailble;
		m_IceInitIndArray[i].Relay_tcp_status = eIceServerUnavailble;
		m_IceInitIndArray[i].fw_type = eFwTypeUnknown;
	}
	memset(&m_ice_servers_type, 0, sizeof(ICE_SERVER_TYPES_S));
}

//////////////////////////////////////////////////////////////////////
CIceInfo::CIceInfo(const CIceInfo &other)
:CSerializeObject(other)
{
	*this = other;
}

//////////////////////////////////////////////////////////////////////
CIceInfo::~CIceInfo()
{}

//////////////////////////////////////////////////////////////////////
CIceInfo& CIceInfo::operator=(const CIceInfo &rHnd)
{
	if(&rHnd != this)
	{
		memcpy(&m_ice_servers_type, &(rHnd.m_ice_servers_type), sizeof(ICE_SERVER_TYPES_S));
		for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
			memcpy(&m_IceInitIndArray[i], &(rHnd.m_IceInitIndArray[i]), sizeof(ICE_INIT_IND_S));
	}
	return *this;
}

//////////////////////////////////////////////////////////////////////
void CIceInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{	
	CXMLDOMElement *pIceServersListInfoNode = thisNode->AddChildNode("ICE_SERVERS_LIST_INFO");
	
	WORD fw_type = eFwTypeUnknown;
	
	//Judith - at the moment, only eIceServerRoleRelaySrvUdp will be printed
//	int role=eIceServerRoleRelaySrvUdp;
	for (int role=eIceServerRoleStunSrvUdp; role<=eIceServerRoleStunPwdServer; role++)
	{
		CXMLDOMElement *pIceServerNode = pIceServersListInfoNode->AddChildNode("ICE_SERVER");
		pIceServerNode->AddChildNode("ICE_SERVER_ROLE", role, ICE_SERVER_ROLE_ENUM);
		pIceServerNode->AddChildNode("ICE_SERVER_IP_ADDRESS", GetIceServerIp(role), IP_ADDRESS);
		pIceServerNode->AddChildNode("ICE_SERVER_STATUS_1", GetIceServerStatus(0, role), ICE_SERVER_STATUS_ENUM);
	}

	for (int i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
	{
		if (m_IceInitIndArray[i].fw_type != eFwTypeUnknown)
			fw_type = m_IceInitIndArray[i].fw_type;
	}
	
	thisNode->AddChildNode("ICE_FIREWALL_DETECTION", fw_type, ICE_FIREWALL_DETECTION_ENUM);
}

//////////////////////////////////////////////////////////////////////
void CIceInfo::SetIceServersType(const ICE_SERVER_TYPES_S &ice_servers_type)
{
	memcpy(&m_ice_servers_type, &ice_servers_type, sizeof(ICE_SERVER_TYPES_S));
}

//////////////////////////////////////////////////////////////////////
void CIceInfo::SetIceInitInd(const ICE_INIT_IND_S &ice_init_ind, const int board_id, const int sub_board_id)
{
	//TBD - Jud
	//for RMX 2000 eProductTypeRMX2000 check what the board id and sub board id is for the first and second media card, and fill the array in the right place
	//also for Rmx 1500 eProductTypeRMX1500
	// and for Rmx 4000 eProductTypeRMX4000

	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	eProductFamily curProductFamily = CProcessBase::GetProcess()->GetProductFamily();

	if(eProductTypeRMX4000 == curProductType)
	{
		if (board_id == 1 && sub_board_id == 1)
			memcpy (&m_IceInitIndArray[0], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 1 && sub_board_id == 2)
			memcpy (&m_IceInitIndArray[1], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 2 && sub_board_id == 1)
			memcpy (&m_IceInitIndArray[2], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 2 && sub_board_id == 2)
			memcpy (&m_IceInitIndArray[3], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 3 && sub_board_id == 1)
			memcpy (&m_IceInitIndArray[4], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 3 && sub_board_id == 2)
			memcpy (&m_IceInitIndArray[5], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 4 && sub_board_id == 1)
			memcpy (&m_IceInitIndArray[6], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 4 && sub_board_id == 2)
			memcpy (&m_IceInitIndArray[7], &ice_init_ind, sizeof(ICE_INIT_IND_S));
	}
	else if(eProductTypeRMX2000 == curProductType || eProductFamilySoftMcu == curProductFamily /* all SoftMcu products */)
	{
		if (board_id == 1 && sub_board_id == 1)
			memcpy (&m_IceInitIndArray[0], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 1 && sub_board_id == 2)
			memcpy (&m_IceInitIndArray[1], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 2 && sub_board_id == 1)
			memcpy (&m_IceInitIndArray[2], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 2 && sub_board_id == 2)
			memcpy (&m_IceInitIndArray[3], &ice_init_ind, sizeof(ICE_INIT_IND_S));
	}
	else if(eProductTypeRMX1500 == curProductType)
	{
		if (board_id == 1 && sub_board_id == 1)
			memcpy (&m_IceInitIndArray[0], &ice_init_ind, sizeof(ICE_INIT_IND_S));
		else if (board_id == 1 && sub_board_id == 2)
			memcpy (&m_IceInitIndArray[1], &ice_init_ind, sizeof(ICE_INIT_IND_S));
	}
	//memcpy(&m_IceInitIndArray[i], &ice_init_ind_array[i], sizeof(ICE_INIT_IND_S));
}

//////////////////////////////////////////////////////////////////////
const char* CIceInfo::GetIceServerIp(const int role) const
{
	switch (role)
	{
	case eIceServerRoleStunPwdServer:
		return m_ice_servers_type.stun_pass_server_params.sIpAddr;
	case eIceServerRoleStunSrvUdp:
		return m_ice_servers_type.stun_udp_server_params.sIpAddr;
	case eIceServerRoleStunSrvTcp:
		return m_ice_servers_type.stun_tcp_server_params.sIpAddr;
	case eIceServerRoleRelaySrvUdp:
		return m_ice_servers_type.relay_udp_server_params.sIpAddr;
	case eIceServerRoleRelaySrvTcp: 
		return m_ice_servers_type.relay_tcp_server_params.sIpAddr;
	default:
		return m_ice_servers_type.stun_pass_server_params.sIpAddr;
	}
	return m_ice_servers_type.stun_pass_server_params.sIpAddr;
}

int CIceInfo::GetIceServerStatus(const int IceInitArrayPos, const int role) const
{
	int i;
	int ret_val = eIceServerUnavailble;

	switch (role)
	{
	case eIceServerRoleStunPwdServer:
	{
		for(i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
		{
			if (m_IceInitIndArray[i].STUN_Pass_status != eIceServerUnavailble)
			{
				ret_val = m_IceInitIndArray[i].STUN_Pass_status;
				break;
			}
		}
	}
	break;

	case eIceServerRoleStunSrvUdp:
	{
		for(i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
		{
			if (m_IceInitIndArray[i].STUN_udp_status != eIceServerUnavailble)
			{
				ret_val = m_IceInitIndArray[i].STUN_udp_status;
				break;
			}
		}
	}
	break;

	case eIceServerRoleStunSrvTcp:
	{
		for(i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
		{
			if (m_IceInitIndArray[i].STUN_tcp_status != eIceServerUnavailble)
			{
				ret_val = m_IceInitIndArray[i].STUN_tcp_status;
				break;
			}
		}
	}
	break;

	case eIceServerRoleRelaySrvUdp:
	{
		for(i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
		{
			if (m_IceInitIndArray[i].Relay_udp_status != eIceServerUnavailble)
			{
				ret_val = m_IceInitIndArray[i].Relay_udp_status;
				break;
			}
		}
	}
	break;

	case eIceServerRoleRelaySrvTcp:
	{
		for(i=0; i<MAX_NUMBER_OF_SERVICES_IN_RMX_4000; i++)
		{
			if (m_IceInitIndArray[i].Relay_tcp_status != eIceServerUnavailble)
			{
				ret_val = m_IceInitIndArray[i].Relay_tcp_status;
				break;
			}
		}
	}
	break;
	default:
		break;
	}

	return ret_val;
}





/*-----------------------------------------------------------------------------------
	CDynIPSProperties. 
	the main class of dynamic data of IpService.
-----------------------------------------------------------------------------------*/
//////////////////////////////////////////////////////////////////////
CDynIPSProperties::CDynIPSProperties()
{

}

//////////////////////////////////////////////////////////////////////
CDynIPSProperties::~CDynIPSProperties()
{

}

//////////////////////////////////////////////////////////////////////
CDynIPSProperties::CDynIPSProperties(const CDynIPSProperties &other)
:CSerializeObject(other)
{
	*this = other;
}
//////////////////////////////////////////////////////////////////////
CDynIPSProperties& CDynIPSProperties::operator = (const CDynIPSProperties& other)
{
	if(&other != this)
	{
		m_ServiceInfo 		= other.m_ServiceInfo;
		m_CSIpInfo 			= other.m_CSIpInfo;
		m_SipInfo 			= other.m_SipInfo;
		m_H323Info 			= other.m_H323Info;
		m_CardIpAddresses 	= other.m_CardIpAddresses;
		m_GKInfo			= other.m_GKInfo;
		m_ProxyInfo			= other.m_ProxyInfo;
		m_DnsInfo			= other.m_DnsInfo;
		m_IceInfo			= other.m_IceInfo;
	}
	return *this;
}
//////////////////////////////////////////////////////////////////////
bool CDynIPSProperties::operator==(const CDynIPSProperties &rHnd)const
{
	if(this == &rHnd)
		return true;	
	
	if ( m_ServiceInfo != rHnd.m_ServiceInfo )
		return false;

	if ( m_CSIpInfo != rHnd.m_CSIpInfo )
		return false;
	
	if ( m_SipInfo != rHnd.m_SipInfo )
		return false;

	if ( m_H323Info != rHnd.m_H323Info )
		return false;

	if ( m_CardIpAddresses != rHnd.m_CardIpAddresses)
		return false;

	if ( m_GKInfo != rHnd.m_GKInfo )
		return false;

	if ( m_ProxyInfo != rHnd.m_ProxyInfo )
		return false;

	if ( m_DnsInfo != rHnd.m_DnsInfo )
		return false;
	
	return true;
}

//////////////////////////////////////////////////////////////////////
bool CDynIPSProperties::operator!=(const CDynIPSProperties& other)const
{
	return (!(*this == other));
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement *pChildNode = thisNode->AddChildNode("DYNAMIC_IP_SERVICE");
	
	m_ServiceInfo		.SerializeXml(pChildNode);
//	m_SipInfo			.SerializeXml(pChildNode);
	m_CardIpAddresses	.SerializeXml(pChildNode);
	m_GKInfo			.SerializeXml(pChildNode);
	m_CSIpInfo			.SerializeXml(pChildNode);
	m_H323Info			.SerializeXml(pChildNode);
	m_ProxyInfo			.SerializeXml(pChildNode);
	m_DnsInfo			.SerializeXml(pChildNode);
	m_IceInfo			.SerializeXml(pChildNode);
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SetInfo(const CServiceInfo &param)
{
	IncreaseUpdateCounter();

	m_ServiceInfo = param;
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SetInfo(const CCSIpInfo &param)
{
	IncreaseUpdateCounter();

	m_CSIpInfo = param;
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SetInfo(const CSipInfo &param)
{
	IncreaseUpdateCounter();

	m_SipInfo = param;
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SetInfo(CH323Info &param)
{
	IncreaseUpdateCounter();

	if(0 == param.GetNumGk())
	{
		m_H323Info.SetIsUpdatedFromCsModule(true);
		return;
	}
	
	m_H323Info = param;
	m_H323Info.SetIsUpdatedFromCsModule(true);
	
	m_GKInfo.SetPrimaryGkIp(param.GetPrimaryGk().GetIPv4Address());
	m_GKInfo.SetAltGkIp(param.GetAltGk().GetIPv4Address());
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SetInfo(const CCardIpAddress &param)
{
	IncreaseUpdateCounter();

	m_CardIpAddresses = param;
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SetInfo(const CProxyInfo &param)
{
	IncreaseUpdateCounter();
	
	m_ProxyInfo = param;
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SetInfo(const CDnsInfo &param)
{
	IncreaseUpdateCounter();
	
	m_DnsInfo = param;
}

//////////////////////////////////////////////////////////////////////
void CDynIPSProperties::SetInfo(const CIceInfo &param)
{
	IncreaseUpdateCounter();
	
	m_IceInfo = param;
}
