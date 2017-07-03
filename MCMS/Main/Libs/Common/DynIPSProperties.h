// CDynIPSProperties.h: interface for the CDynIPSProperties class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Keeping dynamic properties of ip service
//========   ==============   =====================================================================


#ifndef __DYNAMICIPSSERVICE_H__
#define __DYNAMICIPSSERVICE_H__

#include <string>
#include "DataTypes.h"
#include "psosxml.h"
#include "StringsMaps.h"
#include "InitCommonStrings.h"
#include "Trace.h"
#include "CsStructs.h"
#include "SharedMcmsCardsStructs.h"
#include "DefinesIpService.h"
#include "SerializeObject.h"
#include "GKManagerStructs.h"
#include "DNSAgentManagerStructs.h"
#include "SIPProxyStructs.h"
#include "IceCmInd.h"
#include "IceCmReq.h"
#include "AllocateStructs.h"


#define DEFAULT_IP_ADDRESS	0
#define NUM_OF_IP_ADDRESS	4
#define MAX_NUM_DNS 		3
//#define DEFAULT_STATUS 		0xFFFFFFFF

//#define THE_ANSWER 42
#define THE_STRING ""

class CXMLDOMElement;


typedef enum {
	// if you change this enum, please make needed changes in void CIceInfo::SerializeXml(CXMLDOMElement*& thisNode)
	eIceServerRoleStunSrvUdp,
	eIceServerRoleStunSrvTcp,
	eIceServerRoleRelaySrvUdp,
	eIceServerRoleRelaySrvTcp,
	eIceServerRoleStunPwdServer,
	
	eIceServerRoleNotAvailable

} iceServers;

/*-----------------------------------------------------------------------------------
	CServiceInfo. 
	MCMS <-- CS Module (as a buffer)
-----------------------------------------------------------------------------------*/
class CServiceInfo : public CSerializeObject
{
CLASS_TYPE_1(CServiceInfo, CSerializeObject)
public:
	CServiceInfo();
	CServiceInfo(const CServiceInfo &other);
	CServiceInfo& operator=(const CServiceInfo &rHnd);
	bool operator==(const CServiceInfo &rHnd)const;
	bool operator!=(const CServiceInfo& other)const;

	virtual const char* NameOf() const { return "CServiceInfo";}
	
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}
	virtual CSerializeObject* Clone(){return new CServiceInfo(*this);}

	eIpServiceState GetStatus()const{return m_Status;}

public:
	std::string			m_Name;
	DWORD				m_Id;
	eIpServiceState		m_Status;
}; 

/*-----------------------------------------------------------------------------------
	CIPv4Address. 
	MCMS <-- CS Module (part of IpInfo, ...)
-----------------------------------------------------------------------------------*/
class CIPv4Address : public CSerializeObject
{
CLASS_TYPE_1(CIPv4Address, CSerializeObject)

public:
	CIPv4Address(DWORD ip = DEFAULT_IP_ADDRESS);
	CIPv4Address(const CIPv4Address &other);
	
	CIPv4Address& operator=(const CIPv4Address &rHnd);
	bool operator==(const CIPv4Address &rHnd)const;
	bool operator!=(const CIPv4Address& other)const;
	
	virtual const char* NameOf() const { return "CIPv4Address";}
	
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}
	virtual CSerializeObject* Clone(){return new CIPv4Address(*this);}
	
	void SetElementName(const char *elementName);
	void SetIpAddress(DWORD ipAddress){m_Ip = ipAddress;}
	DWORD GetIpAddress()const {return m_Ip;}
	 
private:
	std::string	m_ElementName;
	DWORD 		m_Ip;
};



/*-----------------------------------------------------------------------------------
	CDnsServerList. 
	MCMS <-- CS Module (part of IpInfo)
-----------------------------------------------------------------------------------*/
class CDnsServerList : public CSerializeObject
{
CLASS_TYPE_1(CDnsServerList, CSerializeObject)

public:
	CDnsServerList();
	CDnsServerList(const CDnsServerList &other);
	virtual const char* NameOf() const { return "CDnsServerList";}
	
	CDnsServerList& operator=(const CDnsServerList &rHnd);
	bool operator==(const CDnsServerList &rHnd)const;
	bool operator!=(const CDnsServerList& other)const;
	
	virtual CSerializeObject* Clone(){return new CDnsServerList(*this);}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}

	void SetLength(DWORD length){m_Length = length;}
	void SetDnsIPv4s(DWORD *dnsIps, DWORD length);
	void SetDnsIPv6s(char **dnsIps, DWORD length);
	
private:
	DWORD			m_Length;
	CIPv4Address	m_DnsIPv4[MAX_NUM_DNS];
	ipAddressV6If	m_DnsIPv6[MAX_NUM_DNS];
};




/*-----------------------------------------------------------------------------------
	CIpInfo. 
	MCMS <-- CS Module (as a buffer)
-----------------------------------------------------------------------------------*/
class CIpInfo : public CSerializeObject
{
CLASS_TYPE_1(CIpInfo, CSerializeObject)
public:
	CIpInfo();
	CIpInfo(const CIpInfo &other);
	virtual const char* NameOf() const { return "CIpInfo";}
	
	CIpInfo& operator=(const CIpInfo &rHnd);
	bool operator==(const CIpInfo &rHnd)const;
	bool operator!=(const CIpInfo& other)const;
	
	virtual CSerializeObject* Clone(){return new CIpInfo(*this);}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}

	void SetIPv4Addr(DWORD ip) 		{m_IPv4.SetIpAddress(ip);}
	void SetIPv6Address(const char* ipV6Address);
	void SetSubneMask(DWORD ip) 	{m_SubnetMask.SetIpAddress(ip);}
	void SetDefaultGW(DWORD ip) 	{m_IPv4DefaultGateway.SetIpAddress(ip);}
	void SetNatIp(DWORD ip) 		{m_NatIp.SetIpAddress(ip);}
	void SetDhcpServerIp(DWORD ip) 	{m_DhcpServerIp.SetIpAddress(ip);}
	void SetDhcpStatusType(eDHCPState state) 		{m_DhcpStatusType = state;}
	void SetDomainName(const string &domainName) 	{m_DomainName = domainName;}
	
	CDnsServerList& GetDnsServerList(){return m_DnsServerList;}
	
private:
	CIPv4Address	m_IPv4;
	ipAddressV6If	m_IPv6;
	CIPv4Address	m_SubnetMask;
	CIPv4Address	m_IPv4DefaultGateway;
	std::string		m_IPv6DefaultGateway;
	eDHCPState		m_DhcpStatusType;
	CIPv4Address	m_NatIp;
	CIPv4Address	m_DhcpServerIp;
	std::string		m_DomainName;
	CDnsServerList	m_DnsServerList;
};



/*-----------------------------------------------------------------------------------
	CProxyDataContent. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
class CProxyDataContent : public CSerializeObject
{
CLASS_TYPE_1(CProxyDataContent, CSerializeObject)
public:
	CProxyDataContent();
	CProxyDataContent(const CProxyDataContent &other);
	virtual const char* NameOf() const { return "CProxyDataContent";}
	
	CProxyDataContent& operator=(const CProxyDataContent &rHnd);
	bool operator==(const CProxyDataContent &rHnd)const;
	bool operator!=(const CProxyDataContent& other)const;
	
	virtual CSerializeObject* Clone(){return new CProxyDataContent(*this);}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}
	
	void SetElementName(const char *elementName){m_ElementName = elementName;}
	void SetName(const string &name){m_Name = name;}
	void SetIPv4Address(DWORD ipAddr){m_IPv4.SetIpAddress(ipAddr);}
	void SetIPv6Address(const char* ipV6Address);
	void SetStatus(eServerStatus status){m_Status = status;}
    
	DWORD GetIPv4Address()const {return m_IPv4.GetIpAddress();}
	void  GetIPv6Address(char* retStr);
	const string& GetName()const {return m_Name;}
    eServerStatus GetStatus()const{return m_Status;}
    
    
private:
	std::string		m_ElementName;
	eServerStatus	m_Status;
	CIPv4Address	m_IPv4;
	ipAddressV6If	m_IPv6;
	std::string		m_Name;
};




/*-----------------------------------------------------------------------------------
	CSipInfo. 
	MCMS <-- CS Module (as a buffer)
-----------------------------------------------------------------------------------*/
class CSipInfo : public CSerializeObject
{
CLASS_TYPE_1(CSipInfo, CSerializeObject)
public:	
	CSipInfo();
	CSipInfo(const CSipInfo &other);
	virtual const char* NameOf() const { return "CSipInfo";}
	
	CSipInfo& operator=(const CSipInfo &rHnd);
	bool operator==(const CSipInfo &rHnd)const;
	bool operator!=(const CSipInfo& other)const;
	
	virtual CSerializeObject* Clone(){return new CSipInfo(*this);}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}

	CProxyDataContent& GetPrimaryProxy(){return m_PrimaryProxy;}
	CProxyDataContent& GetAltProxy(){return m_AlternateProxy;}
	
	void SetNumProxy(DWORD numProxy){m_NumOfProxys = numProxy;}
	
private:
	DWORD				m_NumOfProxys;
	CProxyDataContent	m_PrimaryProxy;
	CProxyDataContent	m_AlternateProxy;
};




/*-----------------------------------------------------------------------------------
	CH323Info. 
	MCMS <-- CS Module (as a buffer)
-----------------------------------------------------------------------------------*/
class CH323Info : public CSerializeObject
{
CLASS_TYPE_1(CH323Info, CSerializeObject)

public:
	CH323Info();
	CH323Info(const CH323Info &other);
	virtual const char* NameOf() const { return "CH323Info";}
	
	CH323Info& operator=(const CH323Info &rHnd);
	bool operator==(const CH323Info &rHnd)const;
	bool operator!=(const CH323Info& other)const;
	
	virtual CSerializeObject* Clone(){return new CH323Info(*this);}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}

	bool GetIsUpdatedFromCsModule(){return m_IsUpdatedFromCsModule;}
	void SetIsUpdatedFromCsModule(bool val){m_IsUpdatedFromCsModule = val;}
	
	DWORD GetNumGk()const{return m_NumOfGK;}
	void SetNumGk(DWORD num){m_NumOfGK = num;}
	
	CProxyDataContent& GetPrimaryGk(){return m_PrimaryGK;}
	CProxyDataContent& GetAltGk(){return m_AlternateGK;}
	
	
private:
	bool				m_IsUpdatedFromCsModule;
	DWORD				m_NumOfGK;
	CProxyDataContent	m_PrimaryGK;
	CProxyDataContent	m_AlternateGK;
}; 




/*-----------------------------------------------------------------------------------
	CCardIpAddress. 
	MCMS <-- Cards
-----------------------------------------------------------------------------------*/
class CCardIpAddress : public CSerializeObject
{
CLASS_TYPE_1(CCardIpAddress, CSerializeObject)

public:
	CCardIpAddress();
	CCardIpAddress(const CCardIpAddress &other);
	virtual ~CCardIpAddress();
	virtual const char* NameOf() const { return "CCardIpAddress";}
	
	CCardIpAddress& operator=(const CCardIpAddress &rHnd);
	bool operator==(const CCardIpAddress &rHnd)const;
	bool operator!=(const CCardIpAddress& other)const;
	
	virtual CSerializeObject* Clone(){return new CCardIpAddress(*this);}
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}

	void SetIpInfo(const MEDIA_IP_CONFIG_S &param);
	
private:
	CIpInfo m_IpInfos[NUM_OF_IP_ADDRESS];
};





/*-----------------------------------------------------------------------------------
	CCSIpInfo. 
	MCMS <-- CS Module
-----------------------------------------------------------------------------------*/
class CCSIpInfo : public CSerializeObject
{
CLASS_TYPE_1(CCSIpInfo, CSerializeObject)

public:
	CCSIpInfo();
	CCSIpInfo(const CCSIpInfo &other);
	virtual ~CCSIpInfo();
	virtual const char* NameOf() const { return "CCSIpInfo";}
	
	CCSIpInfo& operator=(const CCSIpInfo &rHnd);
	bool operator==(const CCSIpInfo &rHnd)const;
	bool operator!=(const CCSIpInfo& other)const;
	
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}
	virtual CSerializeObject* Clone(){return new CCSIpInfo(*this);}

	CIpInfo& GetIpInfo(){return m_IpInfo;}
	
	
private:
	CIpInfo	m_IpInfo;
};





/*-----------------------------------------------------------------------------------
	CGKInfo. 
	MCMS <-- GK
-----------------------------------------------------------------------------------*/
class CGKInfo : public CSerializeObject
{
CLASS_TYPE_1(CGKInfo, CSerializeObject)
public:
	CGKInfo();
	virtual ~CGKInfo();
	
	CGKInfo& operator=(const CGKInfo &rHnd);
	bool operator==(const CGKInfo &rHnd)const;
	bool operator!=(const CGKInfo& other)const;
	
	CGKInfo(const CGKInfo &other);
	virtual const char* NameOf() const { return "CGKInfo";}
	
	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}
	virtual CSerializeObject* Clone(){return new CGKInfo(*this);}
	
	void SetPrimaryGkIp(DWORD ip)		{m_GKFullInfo.SetPrimaryGkIp(ip);}
	void SetAltGkIp(DWORD ip)			{m_GKFullInfo.SetAltGkIp(ip);}
	
	void SetGKInfo(const SetGkNameInPropertiesReqStruct &info)			{m_GKFullInfo.SetGKInfo(info);}
	void SetGKInfo(const SetGkIdInPropertiesReqStruct &info)				{m_GKFullInfo.SetGKInfo(info);}
	void SetGKInfo(const ClearGkParamsFromPropertiesReqStruct &info)		{m_GKFullInfo.SetGKInfo(info);}
	void SetGKInfo(const SetGkIPInPropertiesReqStruct &info)				{m_GKFullInfo.SetGKInfo(info);}
	void SetGKInfo(const GkManagerUpdateServicePropertiesReqStruct &info)	{m_GKFullInfo.SetGKInfo(info);}
	

private:
	GKFullInfoStruct	m_GKFullInfo;
};











/*-----------------------------------------------------------------------------------
	CProxyInfo. 
	MCMS <-- Proxy
-----------------------------------------------------------------------------------*/
class CProxyInfo : public CSerializeObject
{
CLASS_TYPE_1(CProxyInfo, CSerializeObject)
public:
	CProxyInfo();
	CProxyInfo(const CProxyInfo &other);
	virtual ~CProxyInfo();
	
	CProxyInfo& operator=(const CProxyInfo &rHnd);
	bool operator==(const CProxyInfo &rHnd)const;
	bool operator!=(const CProxyInfo& other)const;
	
	virtual const char* NameOf() const { return "CProxyInfo";}

	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}
	virtual CSerializeObject* Clone(){return new CProxyInfo(*this);}
	
 	const SIP_PROXY_STATUS_PARAMS_S &GetSipProxyParams()const{return m_Data;}
 	void SetSipProxyParams(const SIP_PROXY_STATUS_PARAMS_S &);
 	SIP_PROXY_STATUS_PARAMS_S GetSipProxyParams() { return m_Data;	}
	
private:
    void SerializeXmlSingleSipProxy(CXMLDOMElement*& thisNode,
                                                const SIP_PROXY_DYNAMIC_PARAMS_S &curProxy) const;
    
    
    SIP_PROXY_STATUS_PARAMS_S m_Data;
};











/*-----------------------------------------------------------------------------------
	CDnsInfo. 
	MCMS <-- Proxy
-----------------------------------------------------------------------------------*/
class CDnsInfo : public CSerializeObject
{
CLASS_TYPE_1(CDnsInfo, CSerializeObject)
public:
	CDnsInfo();
	CDnsInfo(const CDnsInfo &other);
	virtual ~CDnsInfo();
	
	CDnsInfo& operator=(const CDnsInfo &rHnd);
	bool operator==(const CDnsInfo &rHnd)const;
	bool operator!=(const CDnsInfo& other)const;

	virtual const char* NameOf() const { return "CDnsInfo";}

	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}
	virtual CSerializeObject* Clone(){return new CDnsInfo(*this);}
	
	const DNS_PARAMS_IP_S& GetDnsResolutionParam()const{return m_DnsParam;}
	void SetDnsResolutionParams(const DNS_PARAMS_IP_S&);
	
private:
	DNS_PARAMS_IP_S m_DnsParam;
};












/*-----------------------------------------------------------------------------------
	CIceInfo. 
	MCMS <-- Cards
-----------------------------------------------------------------------------------*/
class CIceInfo : public CSerializeObject
{
CLASS_TYPE_1(CIceInfo, CSerializeObject)
public:
	CIceInfo();
	CIceInfo(const CIceInfo &other);
	virtual ~CIceInfo();
	CIceInfo& operator=(const CIceInfo &rHnd);
	virtual const char* NameOf() const { return "CIceInfo";}

	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}
	virtual CSerializeObject* Clone(){return new CIceInfo(*this);}
	
	int GetIceServerStatus(const int IceInitArrayPos, const int role) const;
	const char* GetIceServerIp(const int role) const;
	
	void SetIceServersType(const ICE_SERVER_TYPES_S &ice_servers_type);
	void SetIceInitInd(const ICE_INIT_IND_S &ice_init_ind, const int board_id, const int sub_board_id);

private:
	ICE_INIT_IND_S m_IceInitIndArray[MAX_NUMBER_OF_SERVICES_IN_RMX_4000];
	ICE_SERVER_TYPES_S m_ice_servers_type;
};











/*-----------------------------------------------------------------------------------
	CDynIPSProperties. 
	the main class of dynamic data of IpService.
-----------------------------------------------------------------------------------*/
class CDynIPSProperties : public CSerializeObject
{
CLASS_TYPE_1(CDynIPSProperties, CSerializeObject)

public:
	CDynIPSProperties();
	CDynIPSProperties(const CDynIPSProperties &other);
	CDynIPSProperties& operator = (const CDynIPSProperties& other);
	bool operator==(const CDynIPSProperties &rHnd)const;
	bool operator!=(const CDynIPSProperties& other)const;
	
	virtual ~CDynIPSProperties();
	virtual const char* NameOf() const { return "CDynIPSProperties";}

	virtual void SerializeXml(CXMLDOMElement*& thisNode) const;
	virtual CSerializeObject* Clone(){return new CDynIPSProperties(*this);}
	virtual int  DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action){return 0;}

	CH323Info&  GetCSGKInfo() 	{return m_H323Info;} 	// CSModule monitoring
	CGKInfo&    GetGKInfo() 	{return m_GKInfo;}		// GK monitoring
	CH323Info&  GetH323Info() 	{return m_H323Info;}	// Init of h323
	CProxyInfo& GetProxyInfo()	{return m_ProxyInfo;}	// Proxy monitoring
	CDnsInfo&	GetDnsInfo()	{return m_DnsInfo;}		// Dns resolution monitoring
	CIceInfo&   GetIceInfo()	{return m_IceInfo;}		// Ice monitoring
	
	void SetInfo(const CServiceInfo &);
	void SetInfo(const CCSIpInfo &);
	void SetInfo(const CSipInfo &);
	void SetInfo(	   CH323Info &);
	void SetInfo(const CCardIpAddress &);
	void SetInfo(const CProxyInfo &);
	void SetInfo(const CDnsInfo &);
	void SetInfo(const CIceInfo &);

	CCardIpAddress& GetCardIpAddress() { return m_CardIpAddresses;	}
	CCSIpInfo&		GetCSIpInfo		() { return m_CSIpInfo;			}
	
	CServiceInfo& GetServiceInfo(){return m_ServiceInfo;}

private:
// CS Module
	CServiceInfo	m_ServiceInfo;
	CCSIpInfo		m_CSIpInfo;
	CSipInfo		m_SipInfo;
	CH323Info		m_H323Info;

// 	Card
	CCardIpAddress	m_CardIpAddresses;

// GK	
	CGKInfo			m_GKInfo;
	
// Sip Proxy	
	CProxyInfo 		m_ProxyInfo;

// DNS Agent
	CDnsInfo 		m_DnsInfo;
	
// Ice
	CIceInfo		m_IceInfo;
};





#endif // __DYNAMICIPSSERVICE_H__
