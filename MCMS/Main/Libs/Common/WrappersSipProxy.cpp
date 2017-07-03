#include <iomanip>

#include "WrappersSipProxy.h"
#include "OpcodesMcmsInternal.h"
#include "SystemFunctions.h"
#include "ProcessBase.h"
#include "StringsMaps.h"
#include "SystemFunctions.h"
#include "TraceStream.h"

#include <stdio.h>
extern char* IpTypeToString(APIU32 ipType, bool caps = false);


/*-----------------------------------------------------------------------------
	class CSipProxyIpParamsWrapper
-----------------------------------------------------------------------------*/
CSipProxyIpParamsWrapper::CSipProxyIpParamsWrapper(const SIP_PROXY_IP_PARAMS_S &data)
:m_Data(data)
{
}

CSipProxyIpParamsWrapper::~CSipProxyIpParamsWrapper()
{
}


void CSipProxyIpParamsWrapper::Dump(std::ostream &os) const
{
	TRACEINTO << __FUNCTION__ << " " << m_Data.OutboundProxyAddress.addr.v6.ip << " " << m_Data.OutboundProxyAddress.port;
	DumpHeader(os, "SIP_PROXY_IP_PARAMS_S::Dump");

	os << std::setw(20) << "Service Id: "   		<< m_Data.ServiceId << std::endl;
	os << std::setw(20) << "Service Name: " 		<< m_Data.ServiceName << std::endl;

//	os << std::setw(20) << "Ip type: " 				<< ::IpTypeToString( (eIpType)(m_Data.IpType) ) << std::endl;

	char strIp[128];
/*	SystemDWORDToIpString(m_Data.IpAddressIpV4.addr.v4.ip, strIp);
	os << std::setw(20) << "IpV4: " 				<< strIp << std::endl;

	ipV6ToString(m_Data.IpAddressIpV6.addr.v6.ip, strIp, TRUE);
	os << std::setw(20) << "IpV6: "   				<< strIp << std::endl;
	os << std::setw(20) << "IpV6 scopeId: "			<< m_Data.IpAddressIpV6.addr.v6.scopeId << std::endl;
*/
	for(int i=0; i<TOTAL_NUM_OF_IP_ADDRESSES; i++)
	{
		if(m_Data.pAddrList[i].ipVersion == eIpVersion6 )
		{
			ipV6ToString(m_Data.pAddrList[i].addr.v6.ip, strIp , TRUE);
			os << std::setw(20) << "IpV6: "   				<< strIp << std::endl;
			os << std::setw(20) << "IpV6 scopeId: "			<< m_Data.pAddrList[i].addr.v6.scopeId << std::endl;

		}
		else if(m_Data.pAddrList[i].ipVersion == eIpVersion4)
		{
			SystemDWORDToIpString(m_Data.pAddrList[i].addr.v4.ip, strIp);
			os << std::setw(20) << "IpV4: "   				<< strIp << std::endl;
		}
	}

	if(eIpVersion4 == m_Data.OutboundProxyAddress.ipVersion)
	{
		SystemDWORDToIpString(m_Data.OutboundProxyAddress.addr.v4.ip, strIp);
		os << std::setw(20) << "Outbound Proxy IpV4: "   	<< strIp << std::endl;
	}
	else
	{
		::ipV6ToString(m_Data.OutboundProxyAddress.addr.v6.ip, strIp, TRUE);
		os << std::setw(20) << "Outbound Proxy IpV6: "   	<< strIp << std::endl;
		os << std::setw(20) << "Outbound IpV6 scopeId: "	<< m_Data.OutboundProxyAddress.addr.v6.scopeId << std::endl;
	}

	os << std::setw(20) << "Outbound Proxy Port: " 	<< m_Data.OutboundProxyAddress.port << std::endl;

	if(eIpVersion4 == m_Data.ProxyAddress.ipVersion)
	{
		SystemDWORDToIpString(m_Data.ProxyAddress.addr.v4.ip, strIp);
		os << std::setw(20) << "Proxy IpV4: " 			<< strIp << std::endl;
	}
	else
	{
		ipV6ToString(m_Data.ProxyAddress.addr.v6.ip, strIp, TRUE);
		os << std::setw(20) << "Proxy IpV6: "   		<< strIp << std::endl;
		os << std::setw(20) << "Proxy IpV6 scopeId: "	<< m_Data.ProxyAddress.addr.v6.scopeId << std::endl;
	}

	//os << std::setw(20) << "Proxy Port: "   		<< m_Data.ProxyPort << std::endl;

	SystemDWORDToIpString(m_Data.AlternateProxyAddress.addr.v4.ip, strIp);
	os << std::setw(20) << "Alternate Proxy IpV4: " 	<< strIp << std::endl;

	ipV6ToString(m_Data.AlternateProxyAddress.addr.v6.ip, strIp, TRUE);
	os << std::setw(20) << "Alternate Proxy IpV6: "   	<< strIp << std::endl;
	os << std::setw(20) << "AlternateProxy IpV6 scopeId: "	<< m_Data.AlternateProxyAddress.addr.v6.scopeId << std::endl;

	os << std::setw(20) << "Alternate Proxy Port: " << m_Data.AlternateProxyAddress.port << std::endl;

	os << std::setw(20) << "Outbound Proxy Name: "  << m_Data.pOutboundProxyName << std::endl;
	os << std::setw(20) << "Proxy Name: " 			<< m_Data.pProxyName << std::endl;
	os << std::setw(20) << "Alt Proxy Name: " 		<< m_Data.pAltProxyName << std::endl;

	os << std::setw(20) << "Proxy Host Name: "   	<< m_Data.pProxyHostName << std::endl;
	os << std::setw(20) << "Alt Proxy Host Name: " 	<< m_Data.pAltProxyHostName << std::endl;
	os << std::setw(20) << "Refresh Tout: " 		<< m_Data.refreshTout << std::endl;

	os << std::setw(20) << "Transport Type: "   	<< (WORD)m_Data.transportType << std::endl;
	os << std::setw(20) << "Servers Config: " 		<< (WORD)m_Data.serversConfig << std::endl;
	os << std::setw(20) << "DNS Status: " 			<< (WORD)m_Data.DNSStatus << std::endl;

	os << std::setw(20) << "Dhcp: "   				<< (WORD)m_Data.Dhcp << std::endl;

	char flags[16];
	sprintf(flags, "%d", m_Data.RegistrationFlags);
	os << std::setw(20) << "RegistrationFlags: " 	<< flags << std::endl;
	
	os << std::setw(20) << "Sip advanced user name: " << m_Data.userName << std::endl;
	
	//TBD - Judith. Should I add enum string here??
	os << std::setw(20) << "ice environment: " << m_Data.IceType << std::endl;
	os << std::setw(20) << "ice standard params: " << std::endl;
	
	os << std::setw(20) << "STUN password server host name: " << m_Data.stunPassServerHostName << std::endl;

	if (eIpVersion4 == m_Data.stunPassServerAddress.ipVersion)
	{
		SystemDWORDToIpString(m_Data.stunPassServerAddress.addr.v4.ip, strIp);
		os << std::setw(20) << "STUN password server IpV4: " << strIp << std::endl;
	}
	else
	{
		ipV6ToString(m_Data.stunPassServerAddress.addr.v6.ip, strIp, TRUE);
		os << std::setw(20) << "STUN password server IpV6: " << strIp << std::endl;
	}
	os << std::setw(20) << "STUN password server port: " << m_Data.stunPassServerAddress.port << std::endl;
	os << std::setw(20) << "STUN password server user name: " << m_Data.stunPassServerUserName << std::endl;
	os << std::setw(20) << "STUN password server password: " << m_Data.stunPassServerPassword << std::endl;
	//os << std::setw(20) << "STUN password server realim: " << m_Data.stunPassServerRealm << std::endl;

	os << std::setw(20) << "STUN Host name: " << m_Data.stunServerHostName << std::endl;
	
	if (eIpVersion4 == m_Data.stunServerUDPAddress.ipVersion)
	{
		SystemDWORDToIpString(m_Data.stunServerUDPAddress.addr.v4.ip, strIp);
		os << std::setw(20) << "STUN server UDP IpV4: " << strIp << std::endl;
	}
	else
	{
		ipV6ToString(m_Data.stunServerUDPAddress.addr.v6.ip, strIp, TRUE);
		os << std::setw(20) << "STUN server UDP IpV6: " << strIp << std::endl;
	}
	os << std::setw(20) << "STUN server UDP port: " << m_Data.stunServerUDPAddress.port << std::endl;

	if (eIpVersion4 == m_Data.stunServerTCPAddress.ipVersion)
	{
		SystemDWORDToIpString(m_Data.stunServerTCPAddress.addr.v4.ip, strIp);
		os << std::setw(20) << "STUN server TCP IpV4: " << strIp << std::endl;
	}
	else
	{
		ipV6ToString(m_Data.stunServerTCPAddress.addr.v6.ip, strIp, TRUE);
		os << std::setw(20) << "STUN server TCP IpV6: " << strIp << std::endl;
	}
	os << std::setw(20) << "STUN server TCP port: " << m_Data.stunServerTCPAddress.port << std::endl;

	os << std::setw(20) << "RELAY server Host name: " << m_Data.RelayServerHostName << std::endl;

	if (eIpVersion4 == m_Data.RelayServerUDPAddress.ipVersion)
	{
		SystemDWORDToIpString(m_Data.RelayServerUDPAddress.addr.v4.ip, strIp);
		os << std::setw(20) << "RELAY server UDP IpV4: " << strIp << std::endl;
	}
	else
	{
		ipV6ToString(m_Data.RelayServerUDPAddress.addr.v6.ip, strIp, TRUE);
		os << std::setw(20) << "RELAY server UDP IpV6: " << strIp << std::endl;
	}
	os << std::setw(20) << "RELAY server UDP port: " << m_Data.RelayServerUDPAddress.port << std::endl;
	
	if (eIpVersion4 == m_Data.RelayServerTCPAddress.ipVersion)
	{
		SystemDWORDToIpString(m_Data.RelayServerTCPAddress.addr.v4.ip, strIp);
		os << std::setw(20) << "RELAY server TCP IpV4: " << strIp << std::endl;
	}
	else
	{
		ipV6ToString(m_Data.RelayServerTCPAddress.addr.v6.ip, strIp, TRUE);
		os << std::setw(20) << "RELAY server TCP IpV6: " << strIp << std::endl;
	}
	os << std::setw(20) << "RELAY server TCP port: " << m_Data.RelayServerTCPAddress.port << std::endl;
}







/*-----------------------------------------------------------------------------
	class CSipDynamicProxyParamWrapper
-----------------------------------------------------------------------------*/
CSipDynamicProxyParamWrapper::CSipDynamicProxyParamWrapper(const SIP_PROXY_DYNAMIC_PARAMS_S &data)
:m_Data(data)
{
}

CSipDynamicProxyParamWrapper::~CSipDynamicProxyParamWrapper()
{
}

void CSipDynamicProxyParamWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "SIP_PROXY_DYNAMIC_PARAMS_S::Dump");

    char strIp[32];
    SystemDWORDToIpString(m_Data.IpV4.v4.ip, strIp);

    char *strStatus = "";
    CStringsMaps::GetDescription(SIP_SERVICE_STATUS_ENUM, m_Data.Status, (const char **)&strStatus);

    char *strRole  = "";
    CStringsMaps::GetDescription(SERVICE_ROLE_ENUM, m_Data.Role, (const char **)&strRole);

	os << std::setw(25) << "Name: "     << m_Data.Name   		<< std::endl;
	os << std::setw(25) << "IpV4: "   	<< strIp         		<< std::endl;
	os << std::setw(25) << "IpV6: "   	<< m_Data.IpV6.v6.ip    << std::endl;
	os << std::setw(25) << "Status: " 	<< strStatus     		<< std::endl;
    os << std::setw(25) << "Role: " 	<< strRole       		<< std::endl;
    os << std::endl;
}








/*-----------------------------------------------------------------------------
	class CSipProxyStatusIndWrapper
-----------------------------------------------------------------------------*/
CSipProxyStatusIndWrapper::CSipProxyStatusIndWrapper(const SIP_PROXY_STATUS_PARAMS_S &data)
:m_Data(data)
{
}

CSipProxyStatusIndWrapper::~CSipProxyStatusIndWrapper()
{
}

void CSipProxyStatusIndWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "SIP_PROXY_STATUS_PARAMS_S::Dump");

    os << std::setw(25) << "Service Id: " << m_Data.ServiceId << std::endl;

    for(int i = 0 ; i < NUM_PROXY_SERVERS ; i++)
    {
        os << CSipDynamicProxyParamWrapper(m_Data.ProxyList[i]);
    }
}
