// CommServiceService.h: interface for the CCommServiceService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with CS Module. 
//								Service type(it sends/receives buffer of chars)
//========   ==============   =====================================================================



#ifndef __COMMSERVICESERVICE_H__
#define __COMMSERVICESERVICE_H__

#include <map>
#include <string>
using namespace std;


#include "CommCsModuleService.h"
#include "CfgApi.h"
#include "CsStructs.h"
#include "DefinesGeneral.h"

class CIPService;
class CIPServiceList;
class CCommServiceService;
class CSegment;

typedef STATUS (CCommServiceService::*UpdatedInfoMethodType)(cfgBufHndl &);
typedef std::map<std::string, UpdatedInfoMethodType> CUpdateMethodMap;

typedef STATUS (CCommServiceService::*SectionMethodType)();

typedef STATUS (CCommServiceService::*ReceiveMethodType)(BYTE*);
typedef std::map<OPCODE, ReceiveMethodType> CReceiveMethodMap;


enum eCommonParamSectionType
{
	eSectionBegin = -1,

	eSERVICE_NAME,
	ePORT_RANGE,
	eIP_INTERFACE,
	eSUPPORTED_PROTOCOLS,
	eDNS_CFG,
	eDNS_CALLS,
	eDNS_NAME,
	eAUTH_ENTRY,
	eSIP_SPAN_TYPE,
	eDHCP,
	eSIP_TRANSPORT_TYPE,

	eSectionEnd
};


enum eInfoType
{
	eInfoTypeInvalid = -1,
	eServiceInfo = 0,
	eIpInfo,
	eSipInfo,
	eH323Info,
	
	NumOfInfoTypes
};


struct LastErrorSection_S
{
	string SectionName;
	string Key;
	string Data;
	string Reason;
	
public:
	LastErrorSection_S()
	{
	}
	void Set(const string &sectionName, const string &key, const string &data, const string &reason)
	{
        SectionName = sectionName;
        Key = key;
        Data = data;
        Reason = reason;
	}
};


class CCommServiceService : public CCommCsModuleService  
{
public:
	CCommServiceService();
	virtual ~CCommServiceService();
		
	const char * NameOf(void) const {return "CCommServiceService";}
	STATUS StartNewServiceConfiguration();
	
	STATUS SendNewServiceInitReq();
	STATUS SendCommonParam(WORD isFirst);
	STATUS SendEndServiceInitReq();

	STATUS SendDelServiceReq(CIPService *service);
	STATUS SendPingToCs(const char* destination, const ePingIpType ipType);
	STATUS Receive(OPCODE,CSegment*);
	
	eInfoType GetLastInfoType();
	WORD GetLastUpdateServiceId();
	
	const LastErrorSection_S& GetLastErrorSection(){return m_LastErrorSection;}
	bool IsErrorSectionReceived(){return m_IsErrorSectionReceived;}

    STATUS SendIpInterface();
    
    void   SetCsId(WORD id);
	const char * GetServiceName(void); 
private:
    DWORD GetVlanInternalIpv4Address(const DWORD vlanId);

	virtual STATUS SendToCsApi(OPCODE opcode, const int dataLen, const char * data);

	STATUS SendGenericCommonParam(eCommonParamSectionType type);
	STATUS SendCommonParamReq(WORD isFirst);

	STATUS ReceiveNewServiceInitInd(BYTE*);
	STATUS ReceiveCommonParamInd(BYTE*);
	STATUS ReceiveEndServiceInitInd(BYTE*);
	STATUS ReceiveDeleteServiceInd(BYTE*);

	// new
	STATUS SendServiceName();
	STATUS SendPortRange();
	STATUS SendSIPTransportType();


	// old
	STATUS SendSupportedProtocols();
	STATUS SendDNSCfg();
	STATUS SendDNSCalls();
	STATUS SendDNSName();
	STATUS SendAuthenticationEntry();
	STATUS SendSIPSpanType();
	STATUS SendDHCP();


	
	STATUS SupportedNewService(const char *serviceName, DWORD serviceId);
	STATUS SupportedPortRange(WORD tcpFirstPort, WORD tcpNumOfPorts);
	STATUS SupportedTransportType(DWORD theType);
	// new
	STATUS SupportedServiceName(const char *serviceName, DWORD serviceId);
	STATUS SupportedIpInterface(
				const char *ifName, 
				BOOL  ifState, 
				BOOL  vlan_support, 	
				DWORD vlan_mode, 	
				DWORD vlan_id, 
				DWORD ipv4Address, 
				DWORD ipv4DefaultGateway, 
				DWORD ipv4SubnetMask, 
				DWORD ipv4Mtu, 
				DWORD ipv6_6to4_relay_ad,
				char *ipv6_address,		
				char *ipv6_subnet_mask,	
				char *ipv6_config_type,
				char *ip_interface_type,
				BOOL multiple_service,
				DWORD multi_services_ext_ipv4Address,
				char *multi_services_ext_ipv6_address,
				DWORD ipv4InternalCardAddress,
				char * ipDnsAddres);	//_M_S

	// old
	STATUS SupportedProtocolsRequest(WORD isH323, WORD isSip);
	STATUS SupportedDNSCfg(WORD isUSED, WORD isGetFromDHCP);
	STATUS SupportedDNServerEntry(char* Action, DWORD IPAddress);
	STATUS SupportedDNSCalls(WORD isAcceptDNSCalls,WORD isRegisterToDNS,const char* hostName);
	STATUS SupportedDNSName(WORD isGetFromDHCP,const char* domainName,const char* hostName);
	STATUS SupportedAuthenticationEntry(	const char* action,const char* protocolName,
										const char* userName,const char* password,
										const char* domainName,const char* serverName,
										DWORD ServerIpAddress);
	STATUS SupportedSIPSpanType(const char* spanType, const char* name);

	STATUS SupportedDHCP(WORD isDHCP, WORD isSipFromDhcp, WORD isGkFromDhcp);
	
	STATUS SupportedEndServiceInitReq(const char *serviceName);
	STATUS SupportedDelService(const char *serviceName);

	// indications
	STATUS SectionOKNewService(cfgBufHndl &);	
	STATUS SectionOKCommonParam(cfgBufHndl &);
	STATUS SectionOKEndService(cfgBufHndl &);
	STATUS SectionOKDelService(cfgBufHndl &request);
	STATUS SectionError(cfgBufHndl &);
	
	// monitoring. CS Module sends to CSMngr
	STATUS UpdatedServiceInfo(cfgBufHndl &requestHandle);
	STATUS UpdatedIpInfo(cfgBufHndl &requestHandle);
	STATUS UpdatedSipInfo(cfgBufHndl &requestHandle);
	STATUS UpdatedH323Info(cfgBufHndl &requestHandle);

	CIPService* GetService(const char * name);
	CIPService* GetService(int serviceId);


	CUpdateMethodMap		m_InfoMethodMap;
	SectionMethodType		m_SectionMethodArray[eSectionEnd];
	CReceiveMethodMap		m_ReceiveMethodMap;
	
	eInfoType 	m_LastInfoType;
	WORD 		m_LastInfoServiceId;
	
	bool m_IsErrorSectionReceived;
	LastErrorSection_S m_LastErrorSection;
	
	DWORD m_csId;
	
	eCommonParamSectionType m_type;
};

#endif // __COMMSERVICESERVICE_H__
