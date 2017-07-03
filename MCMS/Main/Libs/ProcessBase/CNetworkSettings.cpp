/*
 * CNetworkSettings.cpp
 *
 *  Created on: Aug 5, 2013
 *      Author: stanny
 */

#include "CNetworkSettings.h"
#include "StatusesGeneral.h"
#include "InternalProcessStatuses.h"
#include "OsFileIF.h"
#include "psosxml.h"
#include "InitCommonStrings.h"
#include "ApiStatuses.h"
#include "Trace.h"
#include "TraceStream.h"
#include "StringsMaps.h"
#include "StringsLen.h"



//xml nodes defines
#define XML_MAIN_NODE_NAME				"NETWORK_SETTINGS"
#define XML_MANAGEMENT_NODE_NAME		"MANAGEMENT_INTERFACE"
#define XML_IP_TYPE_NODE_NAME			"IP_TYPE"
#define XML_IPV6_CONFIGE_NODE_NAME		"IPV6_CONFIGURATION_TYPE"
#define XML_IPV4_NODE_NAME				"IPV4"
#define XML_IPV4_MASK_NODE_NAME			"IPV4_MASK"
#define XML_IP_V6_LIST_NODE_NAME		"IP_V6_LIST"
#define XML_IPV6_NODE_NAME				"IPV6"
#define XML_IPV6_ADDRESS_NODE_NAME		"IPV6_ADDRESS"
#define XML_IPV6_MASK_NODE_NAME			"IPV6_MASK"
#define XML_DEFAULT_GATEWAY_NODE_NAME	"DEFAULT_GATEWAY"
#define XML_SHELF_NODE_NAME				"SHELF_INTERFACE"
#define XML_SHELF_IP_NODE_NAME			"SHELF_IP"
#define XML_NETWORK_INTERFACE_NODE_NAME "NETWORK_INTERFACE"
#define XML_MNGMT_MAC_ADDRESS			"MAC_ADDRESS"
#define XML_MANGMT_DNS					"MANAGMENT_DNS"
#define XML_DNS_CONFIGURATION_STATUS	"DNS_CONFIGURATION_STATUS"
#define XML_DNS_SERVER_STATUS			"DNS_SERVER_STATUS"
#define XML_DOMAIN_NAME					"DOMAIN_NAME"
#define XML_NETWORK_CONFIG_STATUS		"NETWORK_CONFIG_STATUS"

#define XML_LIST_IPV6_SIZE				3
#define IPV6_MASK_DEFAULT			    64
CNetworkSettings::CNetworkSettings() {

	m_iptype 			 = eIpType_IpV4;
	m_ipv6ConfigType 	 = eV6Configuration_Auto;

	m_ipv4         		 =0;
	m_ipv4_Mask          =0;
	m_ipv4_DefGw		 =0;
	m_ipv4Shelf			 =0;
	memset(&m_ipv6_0.addr, 0, sizeof(ipAddressV6If));
	m_ipv6_0.mask =    IPV6_MASK_DEFAULT;
	memset(&m_ipv6_1.addr, 0, sizeof(ipAddressV6If));
	m_ipv6_1.mask =    IPV6_MASK_DEFAULT;
	memset(&m_ipv6_2.addr, 0, sizeof(ipAddressV6If));
	m_ipv6_2.mask =    IPV6_MASK_DEFAULT;
	memset(&m_ipv6_DefGw.addr, 0, sizeof(ipAddressV6If));
	m_ipv6_DefGw.mask =    IPV6_MASK_DEFAULT;
	memset(&m_ipv6_Shelf.addr, 0, sizeof(ipAddressV6If));
	m_ipv6_Shelf.mask =    IPV6_MASK_DEFAULT;

	m_ipv4DnsServer = 0;
	m_ipv4DnsServer_1 = 0;
	m_ipv4DnsServer_2 = 0;
	m_DnsConfigStatus = eDnsConfigurationSuccess;
	m_ServerDnsStatus = eServerStatusOff;
	memset(&m_ipv6_DnsServer.addr, 0, sizeof(ipAddressV6If));
	memset(&m_ipv6_DnsServer_1.addr, 0, sizeof(ipAddressV6If));
	memset(&m_ipv6_DnsServer_2.addr, 0, sizeof(ipAddressV6If));
	m_ipv6_DnsServer.mask =    IPV6_MASK_DEFAULT;
	m_ipv6_DnsServer_1.mask = IPV6_MASK_DEFAULT;
	m_ipv6_DnsServer_2.mask = IPV6_MASK_DEFAULT;
	m_netConfigStatus    = eNetConfigurationSuccess;
	m_interface = "eth0";
	m_domain_Name[0]       = '\0';
}

CNetworkSettings::~CNetworkSettings() {

}

CSerializeObject* CNetworkSettings::Clone()
{
	return new CNetworkSettings(*this);
}
bool CNetworkSettings::operator == (const CNetworkSettings &rHnd)
{
    if(this->m_iptype != rHnd.m_iptype)
    	return false;
    if(this->m_ipv6ConfigType != rHnd.m_ipv6ConfigType)
       	return false;

    if(this->m_ipv4 != rHnd.m_ipv4)
          	return false;
    if(this->m_ipv4_DefGw != rHnd.m_ipv4_DefGw)
          	return false;
    if(memcmp(&m_ipv6_0, &(rHnd.m_ipv6_0), sizeof(ipv6AddressWithMaskStruct)) != 0)
    {
    	FTRACEDEBUG << " CNetworkSettings::operator == compare ipv6 0";
    	TraceBetweenIpv6(m_ipv6_0,rHnd.m_ipv6_0);
        return false;
    }
    if(memcmp(&m_ipv6_1, &(rHnd.m_ipv6_1), sizeof(ipv6AddressWithMaskStruct)) != 0)
    {
    	FTRACEDEBUG << " CNetworkSettings::operator == compare ipv6 1";
    	TraceBetweenIpv6(m_ipv6_1,rHnd.m_ipv6_1);
        return false;
    }

    if(memcmp(&m_ipv6_2, &(rHnd.m_ipv6_2), sizeof(ipv6AddressWithMaskStruct)) != 0)
    {
    	FTRACEDEBUG << " CNetworkSettings::operator == compare ipv6 2";
    	TraceBetweenIpv6(m_ipv6_2,rHnd.m_ipv6_2);
         return false;
    }
    if(memcmp(&m_ipv6_DefGw, &(rHnd.m_ipv6_DefGw), sizeof(ipv6AddressWithMaskStruct)) != 0)
    {
    	FTRACEDEBUG << " CNetworkSettings::operator == compare default gateway ipv6";
    	TraceBetweenIpv6(m_ipv6_DefGw,rHnd.m_ipv6_DefGw);
        return false;
    }

    if(this->m_interface != rHnd.m_interface)
        	return false;

    if(this->m_MacAddress != rHnd.m_MacAddress)
           	return false;
    //check shelf data

    if(this->m_ipv4Shelf != rHnd.m_ipv4Shelf)
                 	return false;
    if(memcmp(&m_ipv6_Shelf, &(rHnd.m_ipv6_Shelf), sizeof(ipv6AddressWithMaskStruct)) != 0)
    {
    	FTRACEDEBUG << " CNetworkSettings::operator == compare shelf ip6s";
    	TraceBetweenIpv6(m_ipv6_Shelf,rHnd.m_ipv6_Shelf);
        return false;
    }

    // check Dns data
    FTRACEDEBUG << "compare dns server ipv4 ";
    if(this->m_ipv4DnsServer != rHnd.m_ipv4DnsServer)
    {
    	char  leftIp[20],RightIp[20];

    	SystemDWORDToIpString(this->m_ipv4DnsServer,leftIp);
    	SystemDWORDToIpString(rHnd.m_ipv4DnsServer,RightIp);
    	FTRACEDEBUG << " TraceBetweenIpv4  leftIpv3 is = " << leftIp << "\n" << "rightIpv4 is =" << RightIp;
                     	return false;
    }

    if(this->m_ipv4DnsServer_1 != rHnd.m_ipv4DnsServer_1)
    {
    	char  leftIp[20],RightIp[20];

    	SystemDWORDToIpString(this->m_ipv4DnsServer_1,leftIp);
    	SystemDWORDToIpString(rHnd.m_ipv4DnsServer_1,RightIp);
    	FTRACEDEBUG << " TraceBetweenIpv4  leftIpv3 is = " << leftIp << "\n" << "rightIpv4 is =" << RightIp;
                     	return false;
    }

    if(this->m_ipv4DnsServer_2 != rHnd.m_ipv4DnsServer_2)
    {
    	char  leftIp[20],RightIp[20];

    	SystemDWORDToIpString(this->m_ipv4DnsServer_2,leftIp);
    	SystemDWORDToIpString(rHnd.m_ipv4DnsServer_2,RightIp);
    	FTRACEDEBUG << " TraceBetweenIpv4  leftIpv3 is = " << leftIp << "\n" << "rightIpv4 is =" << RightIp;
                     	return false;
    }
    FTRACEDEBUG << "compare dns server ipv6 ";
    if(memcmp(&m_ipv6_DnsServer, &(rHnd.m_ipv6_DnsServer), sizeof(ipv6AddressWithMaskStruct)) != 0)
    {
    	FTRACEDEBUG << " CNetworkSettings::operator == compare Dns ip6s";
        	TraceBetweenIpv6(m_ipv6_DnsServer,rHnd.m_ipv6_DnsServer);
            return false;
    }
    FTRACEDEBUG << "compare dns server ipv6 1 ";
    if(memcmp(&m_ipv6_DnsServer_1, &(rHnd.m_ipv6_DnsServer_1), sizeof(ipv6AddressWithMaskStruct)) != 0)
    {
    	FTRACEDEBUG << " CNetworkSettings::operator == compare Dns ip6s";
    	TraceBetweenIpv6(m_ipv6_DnsServer_1,rHnd.m_ipv6_DnsServer_1);
    	return false;
    }

    FTRACEDEBUG << "compare dns server ipv6 2 ";
    if(memcmp(&m_ipv6_DnsServer_2, &(rHnd.m_ipv6_DnsServer_2), sizeof(ipv6AddressWithMaskStruct)) != 0)
    {
    	FTRACEDEBUG << " CNetworkSettings::operator == compare Dns ip6s";
    	TraceBetweenIpv6(m_ipv6_DnsServer_2,rHnd.m_ipv6_DnsServer_2);
    	return false;
    }

    if(this->m_ServerDnsStatus != rHnd.m_ServerDnsStatus)
    	return false;
    FTRACEDEBUG << "compare dns server configuration status ";
    if(this->m_DnsConfigStatus != rHnd.m_DnsConfigStatus )
    	return false;

    FTRACEDEBUG << "compare network  configuration status ";
    if(this->m_netConfigStatus != rHnd.m_netConfigStatus)
    	return false;

    FTRACEDEBUG << " CNetworkSettings::operator == is equel";
	return true;
}
void  CNetworkSettings::TraceBetweenIpv6(ipv6AddressWithMaskStruct& left,const ipv6AddressWithMaskStruct& right)
{
	char szIPLeft[IPV6_ADDRESS_LEN],szIPRight[IPV6_ADDRESS_LEN] ;
	memset(szIPLeft, 0, IPV6_ADDRESS_LEN);
	memset(szIPRight, 0, IPV6_ADDRESS_LEN);
	::ipV6ToString(left.addr.ip, szIPLeft, FALSE);
	::ipV6ToString(right.addr.ip, szIPRight, FALSE);
	FTRACEDEBUG << " CNetworkSettings::TraceBetweenIpv6  leftIpv6 is =" << szIPLeft << "\n" << "rightIpv6 is =" << szIPRight
	  << "\n CNetworkSettings::TraceBetweenIpv6  left mask is =" << left.mask << "\n" << "right mask is =" << right.mask;
}

void   CNetworkSettings::ConvertIpv6AddressToString(ipv6AddressWithMaskStruct& ipv6Address,std::string& ipv6,std::string& mask)
{
	  char  maskbuffer[IPV6_ADDRESS_LEN];
	  char szIPv6[IPV6_ADDRESS_LEN];
	  memset(maskbuffer, 0, IPV6_ADDRESS_LEN);
	  memset(szIPv6, 0, IPV6_ADDRESS_LEN);
	  ::ipV6ToString(ipv6Address.addr.ip, szIPv6, FALSE);
	  sprintf(maskbuffer, "%d", ipv6Address.mask);
	  ipv6 = szIPv6;
	  mask = maskbuffer;
}

void CNetworkSettings::SerializeXml(CXMLDOMElement*& pXMLRootElement)const
{
	ipv6AddressWithMaskStruct listIpv6[]		  ={m_ipv6_0,m_ipv6_1,m_ipv6_2};

    if(NULL == pXMLRootElement)
	{
		pXMLRootElement =  new CXMLDOMElement();
		pXMLRootElement->set_nodeName(XML_MAIN_NODE_NAME);
	}
	else
	{
		pXMLRootElement = pXMLRootElement->AddChildNode(XML_MAIN_NODE_NAME);
	}

    CXMLDOMElement *pMngtIntElement = pXMLRootElement->AddChildNode(XML_MANAGEMENT_NODE_NAME);

    pMngtIntElement->AddChildNode(XML_IP_TYPE_NODE_NAME, m_iptype, IP_TYPE_ENUM);
    pMngtIntElement->AddChildNode(XML_IPV6_CONFIGE_NODE_NAME,	m_ipv6ConfigType, IP_V6_CONFIG_TYPE_ENUM);
    pMngtIntElement->AddChildNode(XML_IPV4_NODE_NAME,	m_ipv4, IP_ADDRESS);

    pMngtIntElement->AddChildNode(XML_IPV4_MASK_NODE_NAME,	m_ipv4_Mask, IP_ADDRESS);

    pMngtIntElement->AddChildNode(XML_NETWORK_INTERFACE_NODE_NAME,	m_interface);

    pMngtIntElement->AddChildNode(XML_MNGMT_MAC_ADDRESS,	m_MacAddress);
    //add ipv6 list
    char  maskbuffer[IPV6_ADDRESS_LEN];
    CXMLDOMElement *pIpv6ListtElement = pMngtIntElement->AddChildNode(XML_IP_V6_LIST_NODE_NAME);
    for(int i=0;i<XML_LIST_IPV6_SIZE;i++)
    {
    	 CXMLDOMElement *pIpv6Element = pIpv6ListtElement->AddChildNode(XML_IPV6_ADDRESS_NODE_NAME);
    	 memset(maskbuffer, 0, IPV6_ADDRESS_LEN);
    	 sprintf(maskbuffer, "%d", listIpv6[i].mask);
    	 pIpv6Element->AddIPv6ChildNode(XML_IPV6_NODE_NAME, listIpv6[i].addr.ip,maskbuffer);

    }
    //add default gateway
    CXMLDOMElement *pGatwListtElement = pMngtIntElement->AddChildNode(XML_DEFAULT_GATEWAY_NODE_NAME);
    pGatwListtElement->AddChildNode(XML_IPV4_NODE_NAME,	m_ipv4_DefGw, IP_ADDRESS);
    memset(maskbuffer, 0, IPV6_ADDRESS_LEN);
    sprintf(maskbuffer, "%d", m_ipv6_DefGw.mask);
    pGatwListtElement->AddIPv6ChildNode(XML_IPV6_NODE_NAME, m_ipv6_DefGw.addr.ip,maskbuffer);

    // add shelf elements
    CXMLDOMElement *pShelfElement = pXMLRootElement->AddChildNode(XML_SHELF_NODE_NAME);
    pShelfElement->AddChildNode(XML_IPV4_NODE_NAME,	m_ipv4Shelf, IP_ADDRESS);
    memset(maskbuffer, 0, IPV6_ADDRESS_LEN);
    sprintf(maskbuffer, "%d", m_ipv6_Shelf.mask);
    pShelfElement->AddIPv6ChildNode(XML_IPV6_NODE_NAME,m_ipv6_Shelf.addr.ip,maskbuffer);

    // add Dns data
    CXMLDOMElement *pDnsfElement = pXMLRootElement->AddChildNode(XML_MANGMT_DNS);
    pDnsfElement->AddChildNode(XML_IPV4_NODE_NAME,	m_ipv4DnsServer, IP_ADDRESS);
    pDnsfElement->AddChildNode(XML_IPV4_NODE_NAME,	m_ipv4DnsServer_1, IP_ADDRESS);
    pDnsfElement->AddChildNode(XML_IPV4_NODE_NAME,	m_ipv4DnsServer_2, IP_ADDRESS);
    FTRACESTR(eLevelDebug) << "serialize m_ipv4DnsServer = " << m_ipv4DnsServer;


    memset(maskbuffer, 0, IPV6_ADDRESS_LEN);
    sprintf(maskbuffer, "%d", m_ipv6_DnsServer.mask);
    pDnsfElement->AddIPv6ChildNode(XML_IPV6_NODE_NAME,m_ipv6_DnsServer.addr.ip,maskbuffer);
    FTRACESTR(eLevelDebug) << "serialize m_ipv6_DnsServer = " << m_ipv6_DnsServer.addr.ip;
    FTRACESTR(eLevelDebug) << "serialize maskbuffer = " << maskbuffer;

    memset(maskbuffer, 0, IPV6_ADDRESS_LEN);
    sprintf(maskbuffer, "%d", m_ipv6_DnsServer_1.mask);
    pDnsfElement->AddIPv6ChildNode(XML_IPV6_NODE_NAME,m_ipv6_DnsServer_1.addr.ip,maskbuffer);

    memset(maskbuffer, 0, IPV6_ADDRESS_LEN);
    sprintf(maskbuffer, "%d", m_ipv6_DnsServer_2.mask);
    pDnsfElement->AddIPv6ChildNode(XML_IPV6_NODE_NAME,m_ipv6_DnsServer_2.addr.ip,maskbuffer);

    pDnsfElement->AddChildNode(XML_DNS_SERVER_STATUS,	m_ServerDnsStatus, SERVER_STATUS_ENUM);
    pDnsfElement->AddChildNode(XML_DNS_CONFIGURATION_STATUS,	m_DnsConfigStatus,_0_TO_DWORD);
    pDnsfElement->AddChildNode(XML_DOMAIN_NAME,	m_domain_Name.c_str(), ONE_LINE_BUFFER_LENGTH);

    //add net config status
    pXMLRootElement->AddChildNode(XML_NETWORK_CONFIG_STATUS,	m_netConfigStatus,_0_TO_DWORD);
}

int CNetworkSettings::DeSerializeXmlIpv6Helper(CXMLDOMElement *pXMLElement,ipv6AddressWithMaskStruct& ipv6,char *pszError)
{
	int nStatus = STATUS_OK;
	char curFullAddress[IPV6_ADDRESS_LEN];
	char tmpAddr[IPV6_ADDRESS_LEN];
	char tmpMask[IPV6_ADDRESS_LEN];
	memset(curFullAddress,	0, IPV6_ADDRESS_LEN);
    memset(tmpAddr,			0, IPV6_ADDRESS_LEN);
    memset(tmpMask,			0, IPV6_ADDRESS_LEN);
	GET_VALIDATE_ASCII_CHILD(pXMLElement,XML_IPV6_NODE_NAME, curFullAddress, _0_TO_IPV6_ADDRESS_LENGTH);
	SplitIPv6AddressAndMask(curFullAddress, tmpAddr, tmpMask);
	StrIpv6ToTransportAddress(tmpAddr,ipv6.addr);
	if(sscanf(tmpMask, "%d", &ipv6.mask) != 1)
	{
			FTRACESTR(eLevelInfoNormal) << "DeSerializeXmlIpv6Helper - failed  setting default value 64";
			ipv6.mask = IPV6_MASK_DEFAULT;
	}
	return STATUS_OK;
}

int  CNetworkSettings::DeSerializeXml(CXMLDOMElement *pXMLRootElement,char *pszError,const char* action)
{
	ipv6AddressWithMaskStruct *listIpv6[XML_LIST_IPV6_SIZE]		  ={&m_ipv6_0,&m_ipv6_1,&m_ipv6_2};

	int nStatus = STATUS_OK;
	char interfaceName[IPV6_ADDRESS_LEN]={0};
	CXMLDOMElement *pMngtIntElement = NULL;
	GET_CHILD_NODE(pXMLRootElement, XML_MANAGEMENT_NODE_NAME, pMngtIntElement);
	if(pMngtIntElement)
	{
		GET_VALIDATE_CHILD(pMngtIntElement,XML_IP_TYPE_NODE_NAME,(BYTE*)( &m_iptype), IP_TYPE_ENUM);
		GET_VALIDATE_CHILD(pMngtIntElement,XML_IPV6_CONFIGE_NODE_NAME,	(BYTE*)(&m_ipv6ConfigType), IP_V6_CONFIG_TYPE_ENUM);
		GET_VALIDATE_CHILD(pMngtIntElement,XML_IPV4_NODE_NAME, &m_ipv4, IP_ADDRESS);

		GET_VALIDATE_CHILD(pMngtIntElement,XML_IPV4_MASK_NODE_NAME, &m_ipv4_Mask, IP_ADDRESS);

		GET_VALIDATE_CHILD(pMngtIntElement,XML_NETWORK_INTERFACE_NODE_NAME,m_interface,_0_TO_H243_NAME_LENGTH);
		GET_VALIDATE_CHILD(pMngtIntElement,XML_MNGMT_MAC_ADDRESS,m_MacAddress,_0_TO_H243_NAME_LENGTH);
		//m_interface = interfaceName;



		CXMLDOMElement *pIpv6ListtElement = NULL;
		GET_CHILD_NODE(pMngtIntElement, XML_IP_V6_LIST_NODE_NAME, pIpv6ListtElement);
		if(pIpv6ListtElement)
		{
			CXMLDOMElement *pNode = NULL;
			GET_FIRST_CHILD_NODE(pIpv6ListtElement,XML_IPV6_ADDRESS_NODE_NAME,pNode);
			int i = 0;
			while (pNode && (i < XML_LIST_IPV6_SIZE))
			{
				DeSerializeXmlIpv6Helper(pNode,*listIpv6[i],pszError);
				 FTRACEINTO << " CNetworkSettings:: after DeSerializeXmlIpv6Helper for ipv6";
				TraceBetweenIpv6(*listIpv6[i],*listIpv6[i]);
				i++;
				GET_NEXT_CHILD_NODE(pIpv6ListtElement,XML_IPV6_ADDRESS_NODE_NAME,pNode);

			}
		}
		CXMLDOMElement *pDefGatwayElement = NULL;
		GET_CHILD_NODE(pMngtIntElement, XML_DEFAULT_GATEWAY_NODE_NAME, pDefGatwayElement);
		if(pDefGatwayElement)
		{
			GET_VALIDATE_CHILD(pDefGatwayElement,XML_IPV4_NODE_NAME, &m_ipv4_DefGw, IP_ADDRESS);
			DeSerializeXmlIpv6Helper(pDefGatwayElement,m_ipv6_DefGw,pszError);
		}
		// get shelf params
		  // add shelf elements
		CXMLDOMElement *pShelfElement = NULL;
		GET_CHILD_NODE(pXMLRootElement, XML_SHELF_NODE_NAME, pShelfElement);
		if(pShelfElement)
		{
		  	GET_VALIDATE_CHILD(pShelfElement,XML_IPV4_NODE_NAME, &m_ipv4Shelf, IP_ADDRESS);
		  	DeSerializeXmlIpv6Helper(pShelfElement,m_ipv6_Shelf,pszError);
		}
		// get Dns Params
		CXMLDOMElement *pDnsElement = NULL;
		CXMLDOMElement *pTempNode;
		DWORD tmpVal =0;
		GET_CHILD_NODE(pXMLRootElement,XML_MANGMT_DNS,pDnsElement);
		if(pDnsElement)
		{
			GET_FIRST_CHILD_NODE(pDnsElement,XML_IPV4_NODE_NAME,pTempNode)
			GET_VALIDATE(pTempNode, &m_ipv4DnsServer, IP_ADDRESS);
			GET_NEXT_CHILD_NODE(pDnsElement,XML_IPV4_NODE_NAME,pTempNode);
			GET_VALIDATE(pTempNode, &m_ipv4DnsServer_1, IP_ADDRESS);
			GET_NEXT_CHILD_NODE(pDnsElement,XML_IPV4_NODE_NAME,pTempNode);
			GET_VALIDATE(pTempNode, &m_ipv4DnsServer_2, IP_ADDRESS);

			CXMLDOMElement *pDnsElementIpv6 = NULL;
			GET_CHILD_NODE(pXMLRootElement,XML_MANGMT_DNS,pDnsElementIpv6);
			DeSerializeXmlIpv6Helper(pDnsElementIpv6,m_ipv6_DnsServer,pszError);

			FTRACEDEBUG << "  CNetworkSettings::DeSerializeXml m_ipv6_DnsServer = " << m_ipv6_DnsServer.addr.ip;
			GET_VALIDATE_CHILD(pDnsElement,XML_DNS_SERVER_STATUS,(BYTE*)( &m_ServerDnsStatus), SERVER_STATUS_ENUM);

			GET_VALIDATE_CHILD(pDnsElement,XML_DNS_CONFIGURATION_STATUS,&tmpVal,_0_TO_DWORD);
			m_DnsConfigStatus = (eDnsConfigurationStatus)tmpVal;

			GET_VALIDATE_CHILD(pDnsElement,XML_DOMAIN_NAME, m_domain_Name, ONE_LINE_BUFFER_LENGTH);
		}


		tmpVal =0;
		//get status network
		GET_VALIDATE_CHILD(pXMLRootElement,XML_NETWORK_CONFIG_STATUS,&tmpVal,_0_TO_DWORD);
		 m_netConfigStatus = (eNetConfigurationStatus)tmpVal;
	}


	return STATUS_OK;
}

void 	CNetworkSettings::StrIpv6ToTransportAddress(const char* sInIpv6,ipAddressV6If& OutIpv6)
{
	mcTransportAddress tmpIPv6Addr;
	memset(&tmpIPv6Addr, 0, sizeof(mcTransportAddress));
	::stringToIpV6( &tmpIPv6Addr, (char*)sInIpv6 );
	memcpy( &(OutIpv6.ip), &(tmpIPv6Addr.addr.v6.ip), IPV6_ADDRESS_BYTES_LEN );
	// ===== 2. scopeId
	OutIpv6.scopeId = ::getScopeId( (char*)sInIpv6 );
}

STATUS CNetworkSettings::WriteToFile()
{
	if(IsFileExists(FULL_FILE_NAME_NET_SETTINGS))
	{
		DeleteFile(FULL_FILE_NAME_NET_SETTINGS,TRUE);
	}
	STATUS status = WriteXmlFile((FULL_FILE_NAME_NET_SETTINGS).c_str());
	return status;
}
STATUS CNetworkSettings::LoadFromFile()
{
	  STATUS status = ReadXmlFile((FULL_FILE_NAME_NET_SETTINGS).c_str());
	 return status;
}

BOOL	CNetworkSettings::GetIpv6Mngnt(int index,ipv6AddressWithMaskStruct& ipv6Struct)
{
	if((index >= 0) && (index < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES))
	{
		switch(index)
		{
			case 0: ipv6Struct = m_ipv6_0;
				break;
			case 1: ipv6Struct = m_ipv6_1;
				break;
			case 2: ipv6Struct = m_ipv6_2;
				break;
		}
	}
	else
	{
		//invalid request
		PASSERT(1);
		return FALSE;
	}
	return TRUE;
}

void CNetworkSettings::SetDomainName(const std::string& domain_Name)
{
	m_domain_Name = domain_Name;
}
