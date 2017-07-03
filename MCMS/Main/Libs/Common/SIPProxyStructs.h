// SIPProxyStructs.h
//
//////////////////////////////////////////////////////////////////////
#if !defined(_SIPProxyStructs_H__)
#define _SIPProxyStructs_H__

#include "DataTypes.h"


#define NUM_PROXY_SERVERS 2
//#define NUM_OF_IPS		  3

struct SIP_PROXY_IP_PARAMS_S
{
	char	ServiceName[NET_SERVICE_PROVIDER_NAME_LEN];
	int		ServiceId;
//	DWORD	Ip;

	eIpType	IpType;
/*
	ipAddressIf IpV4;
	ipAddressIf IpV6;

	ipAddressIf	OutboundProxyIpV4;
	ipAddressIf	OutboundProxyIpV6;
	WORD		OutboundProxyPort;

	ipAddressIf	ProxyIpV4;
	ipAddressIf	ProxyIpV6;
	WORD		ProxyPort;

	ipAddressIf  AlternateProxyIpV4;
	ipAddressIf  AlternateProxyIpV6;
	WORD		 AlternateProxyPort;
*/	
//	mcTransportAddress  IpAddressIpV4;
//	mcTransportAddress  IpAddressIpV6;
	ipAddressStruct pAddrList[TOTAL_NUM_OF_IP_ADDRESSES];
	mcTransportAddress	OutboundProxyAddress;
	mcTransportAddress	ProxyAddress;
	mcTransportAddress  AlternateProxyAddress;
	
	char	pOutboundProxyName[H243_NAME_LEN];
	char	pProxyName[H243_NAME_LEN];
	char	pAltProxyName[H243_NAME_LEN];
	char	pProxyHostName[H243_NAME_LEN];
	char	pAltProxyHostName[H243_NAME_LEN];
	WORD	refreshTout;
	BYTE	transportType;	//enTransportType
	BYTE	serversConfig;	// CONFIGURE_SIP_SERVERS_AUTOMATICALLY or CONFIGURE_SIP_SERVERS_MANUALLY                              0
	BYTE	DNSStatus;		// SERVER_OFF or SERVER_SPECIFY or SERVER_AUTO 
	BYTE	Dhcp;				// YES or NO
	//The next BYTE is bit-wise to hold  registration flags for on-going, MR, EQ
	//0x01 - ongoing
	//0x02 - MR
	//0x04 - EQ
	//0x08 - Sip factories
	//0x10 - GW profiles
	BYTE	RegistrationFlags;

	//ICE parameters
	char    userName[H243_NAME_LEN];
	
	eIceEnvironmentType	IceType;

	mcTransportAddress stunPassServerAddress;
	char    stunPassServerHostName[H243_NAME_LEN];
	char 	stunPassServerUserName[H243_NAME_LEN];
	char 	stunPassServerPassword[H243_NAME_LEN];
	char 	stunPassServerRealm[H243_NAME_LEN];

	char stunServerHostName[H243_NAME_LEN];
	char RelayServerHostName[H243_NAME_LEN]; //TURN

	mcTransportAddress stunServerUDPAddress;
	mcTransportAddress stunServerTCPAddress;
	mcTransportAddress RelayServerUDPAddress; //TURN
	mcTransportAddress RelayServerTCPAddress; //TURN

	WORD 			    SipServerType;

};


struct SIP_PROXY_DYNAMIC_PARAMS_S
{
    DWORD   Role; // eServerType
    char Name[H243_NAME_LEN];
//    DWORD Ip;
//	mcTransportAddress IpAddress;
	ipAddressIf IpV4;
	ipAddressIf IpV6;
	
    DWORD Status;
};


struct SIP_PROXY_STATUS_PARAMS_S
{
    int		ServiceId;
    SIP_PROXY_DYNAMIC_PARAMS_S ProxyList[NUM_PROXY_SERVERS];
};



#endif // !defined(_SIPProxyStructs_H__)
