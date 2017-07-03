/*
 * CNetworkSettings.h
 *
 *  Created on: Aug 5, 2013
 *      Author: stanny
 */

#ifndef CNETWORKSETTINGS_H_
#define CNETWORKSETTINGS_H_

#include "SerializeObject.h"

typedef struct
{
	ipAddressV6If  addr;
	DWORD 		   mask;
} ipv6AddressWithMaskStruct;

#define FULL_FILE_NAME_NET_SETTINGS	((std::string)MCU_TMP_DIR+"/network_settings.xml")

class CNetworkSettings: public CSerializeObject {
public:
	CNetworkSettings();
	virtual ~CNetworkSettings();
	virtual const char* NameOf() const { return "CNetworkSettings";}
	STATUS WriteToFile();
	STATUS LoadFromFile();
// Management data
	eIpType 			 			 m_iptype;
	eV6ConfigurationType 		     m_ipv6ConfigType;
	DWORD				 			 m_ipv4;
	DWORD				 			 m_ipv4_Mask;
	DWORD				 			 m_ipv4_DefGw;
	ipv6AddressWithMaskStruct		 m_ipv6_0;
	ipv6AddressWithMaskStruct		 m_ipv6_1;
	ipv6AddressWithMaskStruct		 m_ipv6_2;
	ipv6AddressWithMaskStruct		 m_ipv6_DefGw;
	std::string						 m_interface; //in rmx 2000 there the ipv4 is separated from ipv6 interface in an alias interface
	std::string 					 m_MacAddress;
// Shelf data
	DWORD				 			 m_ipv4Shelf;
	ipv6AddressWithMaskStruct   	 m_ipv6_Shelf;
// DNS data
	eDnsConfigurationStatus			m_DnsConfigStatus; //Success,failure,not config
	eServerStatus					m_ServerDnsStatus;   // auto,specify,off
	ipv6AddressWithMaskStruct   	m_ipv6_DnsServer;
	ipv6AddressWithMaskStruct   	m_ipv6_DnsServer_1;
	ipv6AddressWithMaskStruct   	m_ipv6_DnsServer_2;
	DWORD							m_ipv4DnsServer;
	DWORD							m_ipv4DnsServer_1;
	DWORD							m_ipv4DnsServer_2;
	eNetConfigurationStatus         m_netConfigStatus;
	std::string 					m_domain_Name;
	bool operator == (const CNetworkSettings &rHnd);
	void 		 TraceBetweenIpv6(ipv6AddressWithMaskStruct& left,const ipv6AddressWithMaskStruct& right);
	void 		 ConvertIpv6AddressToString(ipv6AddressWithMaskStruct& ipv6Address,std::string& ipv6,std::string& mask);
	BOOL	GetIpv6Mngnt(int index,ipv6AddressWithMaskStruct& ipv6Struct);
	void 		 SetDomainName(const std::string& domain_Name);
	std::string GetDomainName(){return m_domain_Name;}

protected:
	virtual void SerializeXml(CXMLDOMElement*& pXMLRootElement) const;
    virtual int  DeSerializeXml(CXMLDOMElement *pXMLRootElement,char *pszError,const char* action);
    int 		 DeSerializeXmlIpv6Helper(CXMLDOMElement *pXMLElement,ipv6AddressWithMaskStruct& ipv6,char *pszError);
    void 		 StrIpv6ToTransportAddress(const char* sInIpv6,ipAddressV6If& OutIpv6);

    CSerializeObject* Clone();
};

#endif /* CNETWORKSETTINGS_H_ */
