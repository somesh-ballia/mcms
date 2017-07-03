// SIPProxyStructs.h
//
//////////////////////////////////////////////////////////////////////
#if !defined(_DNSAgentMANAGERStructs_H__)
#define _DNSAgentMANAGERStructs_H__

#include "IpAddressDefinitions.h"
#include "IpCsSizeDefinitions.h"
#include "AllocateStructs.h"

struct DNS_PARAMS_IP_S
{
	char	domainName[DnsQueryNameSize];
	int		ServiceId;
//	mcTransportAddress	Ip;
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];

};

#define DNS_AGENT_DOMAIN_NAME_LEN 128

typedef struct
{
	WORD   dnsStatus; // eServerStatus
	DWORD  serversIpv4List[NUM_OF_DNS_SERVERS];
	char   serversIpv6List[NUM_OF_DNS_SERVERS][IPV6_ADDRESS_LEN];
	char   szDomainName[DNS_AGENT_DOMAIN_NAME_LEN];
} DNS_IP_SERVICE_S;

typedef struct
{
	WORD              ipType; // eIpType
	WORD              servicesNum;
	DNS_IP_SERVICE_S  services[MAX_NUM_OF_IP_SERVICES];
} DNS_IP_CONFIG_PARAM_S;

#endif // !defined(_SIPProxyStructs_H__)
