// SIPProxyIpParameters.h: interface for the CSipProxyIpParams class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SIPProxyIpParameters_H_
#define _SIPProxyIpParameters_H_


#include "IpParameters.h"
#include "SipDefinitions.h"
#include "SIPProxyStructs.h"
#include "Segment.h"



#define	MAX_CONF_REGISTRATIONS		1000 //Changed from 100 up to 1000. 30-May-2011. Alex.Sheinberg

#define BAD_CHARACTERS_FOR_SIP_URI " \"<>@:\\"





class CSipProxyIpParams : public CPObject
{

CLASS_TYPE_1(CSipProxyIpParams, CPObject)

public:
	CSipProxyIpParams ();
	virtual const char* NameOf() const { return "CSipProxyIpParams";}
	CSipProxyIpParams (const SIP_PROXY_IP_PARAMS_S proxyIpParams);
	virtual ~CSipProxyIpParams ();
	virtual void Dump(std::ostream& msg) const;

	CSipProxyIpParams& operator = (const CSipProxyIpParams &rOther);
	friend WORD operator==(const CSipProxyIpParams& lhs,const CSipProxyIpParams& rhs);    
	friend bool operator<(const CSipProxyIpParams& lhs,const CSipProxyIpParams& rhs);    
	
	void	Deserialize(CSegment* pSeg);
	//Configuration data
	char*	GetServiceName();
	DWORD	GetServiceId();
	void	SetServiceId(DWORD serviceId);
	eIpType	GetIpType();
	// For debug
	void    SetIpType(eIpType eType);
//	DWORD	GetIpV4();
//	char*	GetIpV6();
//	DWORD 	GetIpV6scopeId();
//	DWORD 	GetIpVersion();
//	mcTransportAddress GetIpAddressIpV4();
//	mcTransportAddress GetIpAddressIpV6();
	ipAddressStruct GetServiceIpAddress(WORD index);
	
	char* GetUserName();
	WORD  GetIsMsICE();
	WORD  GetIsStandartICE();
	WORD GetIsWebRtcICE();
	eIceEnvironmentType GetICEType();

	void 	SetServiceIpAddressToNull(WORD index,BYTE ipType);
	
	DWORD	GetOutboundProxyIpV4();
	char*	GetOutboundProxyIpV6();
	DWORD 	GetOutboundProxyV6scopeId();
	DWORD 	GetOutboundProxyIpVersion();	
	WORD	GetOutboundProxyPort();
	mcTransportAddress GetOutboundProxyAddress();
	
	DWORD	GetProxyIpV4();
	char*	GetProxyIpV6();
	DWORD 	GetProxyV6scopeId();
	DWORD 	GetProxyIpVersion();	
	WORD	GetProxyPort();
	mcTransportAddress GetProxyAddress();
	
	DWORD	GetAltProxyIpV4();
	char*	GetAltProxyIpV6();
	DWORD 	GetAltProxyV6scopeId();	
	DWORD 	GetAltProxyIpVersion();	
	WORD	GetAltProxyPort();
	mcTransportAddress GetAltProxyAddress();
	
	char*	GetOutboundProxyName();
	char*	GetProxyName();
	char*	GetAltProxyName();
	char*	GetProxyHostName();
	char*	GetAltProxyHostName();
	enTransportType	GetTransportType();
	WORD	GetRefreshTout();
	BYTE	GetServersConfig();
	BYTE	GetDNSStatus();
	BYTE	IsDHCP();
	BYTE	IsRegOnGoing();
	BYTE	IsRegMeetingRooms();
	BYTE	IsRegEQs();
	BYTE  	IsRegFactories();
	BYTE  	IsRegGWProfiles();

	//run-time data
	void	SetOnGoingRegistrarStatus(DWORD status);
	DWORD	GetOnGoingRegistrarStatus();
	void 	SetOnGoingRegistrarIp(mcTransportAddress proxyIp);
	mcTransportAddress	GetOnGoingRegistrarIp();
	DWORD	GetOnGoingRegistrarV4Ip();
	char*	GetOnGoingRegistrarV6Ip();
	void	SetOnGoingAltRegistrarStatus(DWORD status);
	DWORD	GetOnGoingAltRegistrarStatus();
	void 	SetOnGoingAltRegistrarIp(mcTransportAddress proxyIp);
	mcTransportAddress	GetOnGoingAltRegistrarIp();
	DWORD	GetOnGoingAltRegistrarV4Ip();
	char*	GetOnGoingAltRegistrarV6Ip();
	WORD  	GetSipServerType();

// Ice Standard params
	char* GetSTUNpassHostName();
	mcTransportAddress GetSTUNpassIpAddress();
	DWORD GetSTUNIpVersion();
	DWORD GetStunPassV4Ip();
	char* GetStunPassV6Ip();
	DWORD GetStunPassPort();
	char* GetStunPassUserName();
	char* GetStunPassPassword();
	char* GetStunPassRealm();
	char* GetStunServerHostName();
	mcTransportAddress GetStunUdpIpAddress();
	DWORD GetStunUDPIpVersion();
	DWORD GetStunUDPV4Ip();
	char* GetStunUDPV6Ip();
	DWORD GetStunUDPPort();
	mcTransportAddress GetStunTcpIpAddress();
	DWORD GetStunTCPIpVersion();
	DWORD GetStunTCPV4Ip();
	char* GetStunTCPV6Ip();
	DWORD GetStunTCPPort();
	char* GetRelayServerHostName();
	mcTransportAddress GetRelayUdpIpAddress();
	DWORD GetRelayUdpIpVersion();
	DWORD GetRelayUdpV4Ip();
	char* GetRelayUdpV6Ip();
	DWORD GetRelayUdpPort();
	mcTransportAddress GetRelayTcpIpAddress();
	DWORD GetRelayTcpIpVersion();
	DWORD GetRelayTcpV4Ip();
	char* GetRelayTcpV6Ip();
	DWORD GetRelayTcpPort();


protected:
	SIP_PROXY_IP_PARAMS_S   m_sipProxyIpParamsStruct;
	
	DWORD	m_RegistrarStatus;
	mcTransportAddress 	m_RegistrarIp;
	DWORD	m_AltRegistrarStatus;
	mcTransportAddress	m_AltRegistrarIp;
	WORD 			    m_SipServerType;
};

#endif // _SIPProxyIpParameters_H_
