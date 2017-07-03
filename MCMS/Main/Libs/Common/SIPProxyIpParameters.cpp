
// SIPProxyIpParameters.cpp: implementation of the CSipProxyIpParams class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "NStream.h"
#include "SIPProxyIpParameters.h"
#include "SystemFunctions.h"
#include "DefinesGeneral.h"
#include "DefinesIpService.h"
#include "TraceStream.h"
#include "SysConfig.h"
#include "ProcessBase.h"

// ------------------------------------------------------------
CSipProxyIpParams::CSipProxyIpParams () : m_RegistrarStatus(eSipServerStatusNotAvailable), m_AltRegistrarStatus(eSipServerStatusNotAvailable)

{
	memset(&m_RegistrarIp, 0, sizeof(mcTransportAddress));
	memset(&m_AltRegistrarIp, 0, sizeof(mcTransportAddress));
	m_sipProxyIpParamsStruct.ServiceId = -1;
}

// ------------------------------------------------------------
CSipProxyIpParams::CSipProxyIpParams (const SIP_PROXY_IP_PARAMS_S proxyIpParams) : m_RegistrarStatus(eSipServerStatusNotAvailable), m_AltRegistrarStatus(eSipServerStatusNotAvailable)

{
	memset(&m_RegistrarIp, 0, sizeof(mcTransportAddress));
	memset(&m_AltRegistrarIp, 0, sizeof(mcTransportAddress));

	memcpy(&m_sipProxyIpParamsStruct, &proxyIpParams, sizeof(SIP_PROXY_IP_PARAMS_S));
}

// ------------------------------------------------------------
CSipProxyIpParams::~CSipProxyIpParams ()
{
}

// ------------------------------------------------------------
void CSipProxyIpParams::Deserialize(CSegment* pSeg)
{
	//SIP_PROXY_IP_PARAMS_S *pNewServiceIpParams = (SIP_PROXY_IP_PARAMS_S*)pSeg->GetPtr();

	memcpy(&m_sipProxyIpParamsStruct, (SIP_PROXY_IP_PARAMS_S*)pSeg->GetPtr(), sizeof(SIP_PROXY_IP_PARAMS_S));

	char mo[128];
	memset (mo,0,128);
	TRACEINTO << __FUNCTION__ << " outbound ipVersion: " << m_sipProxyIpParamsStruct.OutboundProxyAddress.ipVersion
			  << ", ip: " << ipToString(m_sipProxyIpParamsStruct.OutboundProxyAddress, mo, 0);

}


// ------------------------------------------------------------
void  CSipProxyIpParams::Dump(std::ostream& msg) const
{
	char proxyIp[16];
	char outboundProxyIp[16];
	char altProxyIp[16];
	SystemDWORDToIpString(m_sipProxyIpParamsStruct.ProxyAddress.addr.v4.ip, proxyIp);
	SystemDWORDToIpString(m_sipProxyIpParamsStruct.OutboundProxyAddress.addr.v4.ip, outboundProxyIp);
	SystemDWORDToIpString(m_sipProxyIpParamsStruct.AlternateProxyAddress.addr.v4.ip, altProxyIp);

	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg << "\n\n"
		<< "SIPProxyIpParameters::Dump\n"
		<< "---------------------------\n";

//	CPObject::Dump(msg);
//
	if(eIpType_IpV4 == m_sipProxyIpParamsStruct.IpType)
		msg << std::setw(20) << "IpType: " << " eIpType_IpV4 \n";
	if(eIpType_IpV6 == m_sipProxyIpParamsStruct.IpType)
		msg << std::setw(20) << "IpType: " << " eIpType_IpV6 \n";
	if(eIpType_Both == m_sipProxyIpParamsStruct.IpType)
		msg << std::setw(20)  << "IpType: " << " eIpType_Both \n";
	if(eIpType_None == m_sipProxyIpParamsStruct.IpType)
		msg << std::setw(20)  << "IpType: " << " eIpType_None \n";


	char strIp[128];
	::ipV6ToString(m_sipProxyIpParamsStruct.ProxyAddress.addr.v6.ip, strIp, TRUE);

	msg	<< std::setw(20) << "Service Id: "   << m_sipProxyIpParamsStruct.ServiceId   << "\n"
		<< std::setw(20) << "Service Name: " << m_sipProxyIpParamsStruct.ServiceName << "\n";

	if(eIpVersion4 == m_sipProxyIpParamsStruct.ProxyAddress.ipVersion)
	{
		msg << std::setw(20) << "Proxy IpV4: " << proxyIp << "\n";
	}
	else
	{
		msg << std::setw(20) << "Proxy IpV6: " << strIp << "\n";
	}

	msg	<< std::setw(20) << "port: " << m_sipProxyIpParamsStruct.ProxyAddress.port << "\n"
		<< std::setw(20) << "Proxy Name: " << m_sipProxyIpParamsStruct.pProxyName << "\n"
		<< std::setw(20) << "Outbound Proxy Name: " << m_sipProxyIpParamsStruct.pOutboundProxyName << "\n"
		<< std::setw(20) << "Proxy Host Name: " << m_sipProxyIpParamsStruct.pProxyHostName << "\n";

	if(eIpVersion4 == m_sipProxyIpParamsStruct.OutboundProxyAddress.ipVersion)
	{
		msg << std::setw(20) << "Outbound Proxy IpV4: " << outboundProxyIp << " port: " << m_sipProxyIpParamsStruct.OutboundProxyAddress.port << "\n";
	}
	else
	{
		::ipV6ToString(m_sipProxyIpParamsStruct.OutboundProxyAddress.addr.v6.ip, strIp, TRUE);
		msg	<< std::setw(20) << "Outbound Proxy IpV6: " << strIp << "\n";
	}

	msg << std::setw(20) << "port: " << m_sipProxyIpParamsStruct.OutboundProxyAddress.port << "\n"
		<< std::setw(20) << "Alt Proxy IpV4: " << altProxyIp << " port: " << m_sipProxyIpParamsStruct.AlternateProxyAddress.port << "\n"
		<< std::setw(20) << "Alt Proxy IpV6: " <<  m_sipProxyIpParamsStruct.AlternateProxyAddress.addr.v6.ip << "\n"
		<< std::setw(20) << "port: " << m_sipProxyIpParamsStruct.AlternateProxyAddress.addr.v6.ip << "\n"
		<< std::setw(20) << "Alt Proxy Host Name: " << m_sipProxyIpParamsStruct.pAltProxyHostName << "\n";

	msg	<< std::setw(20) << "CS Address list: " << "\n";
	for (int j = 0; j < TOTAL_NUM_OF_IP_ADDRESSES; j++)
	{
		memset(strIp,'\0',128);
		::ipToString(m_sipProxyIpParamsStruct.pAddrList[j],strIp,1);
		msg	<< std::setw(20) << "CS Address [" << j << "] :" << strIp << "\n";
	}
	if(eTransportTypeUdp == m_sipProxyIpParamsStruct.transportType)
		msg << std::setw(20) << "Transport type: " << " UDP \n";
	else
		if(eTransportTypeTcp == m_sipProxyIpParamsStruct.transportType)
			msg << std::setw(20) << "Transport type: " << " TCP \n";

	if(eConfSipServerAuto == m_sipProxyIpParamsStruct.serversConfig)
		msg << std::setw(20) << "Servers Config: " << " Auto \n";
	else
		if(eConfSipServerManually == m_sipProxyIpParamsStruct.serversConfig)
			msg << std::setw(20) << "Servers Config: " << " Manually \n";

	if(eServerStatusOff == m_sipProxyIpParamsStruct.DNSStatus)
		msg << std::setw(20) << "DNS: " << " OFF \n";
	else
	{
		if(eServerStatusSpecify == m_sipProxyIpParamsStruct.DNSStatus)
			msg << std::setw(20) << "DNS: " << " Specify \n";
		else
			if(eServerStatusAuto == m_sipProxyIpParamsStruct.DNSStatus)
				msg << std::setw(20)  << "DNS: " << " Auto \n";
	}

	if(YES == m_sipProxyIpParamsStruct.Dhcp)
		msg << std::setw(20) << "DHCP: " << "Yes \n";
	else
		msg << std::setw(20) << "DHCP: " << "No \n";

/*	if(m_sipProxyIpParamsStruct.RegistrationFlags & REG_ON_GOING)
		msg << std::setw(20) << "Register On-Going: " << "Yes \n";
	else
		msg << std::setw(20) << "Register On-Going: " << "No \n";
	if(m_sipProxyIpParamsStruct.RegistrationFlags & REG_MEETING_ROOMS)
		msg << std::setw(20) << "Register MRs: " << "Yes \n";
	else
		msg << std::setw(20) << "Register MRs: " << "No \n";
	if(m_sipProxyIpParamsStruct.RegistrationFlags & REG_ENTRY_QUEUES)
		msg << std::setw(20) << "Register EQs: " << "Yes \n";
	else
		msg << std::setw(20) << "Register EQs: " << "No \n";
	if(m_sipProxyIpParamsStruct.RegistrationFlags & REG_FACTORIES)
		msg << std::setw(20) << "Register Factories: " << "Yes \n";
	else
		msg << std::setw(20) << "Register Factories: " << "No \n";
	if(m_sipProxyIpParamsStruct.RegistrationFlags & REG_GW_PROFILES)
		msg << std::setw(20) << "Register GW Profiles: " << "Yes \n";
	else
		msg << std::setw(20) << "Register GW Profiles: " << "No \n";*/

	msg << std::setw(20) << "Register refresh: " << m_sipProxyIpParamsStruct.refreshTout << "\n";
	
	switch (m_sipProxyIpParamsStruct.IceType)
	{
		case eIceEnvironment_ms:
		{
			msg << std::setw(40) << "ICE Environment: Microsoft " << std::endl;
			msg << std::setw(20) << "MS-ICE user name: " << m_sipProxyIpParamsStruct.userName << std::endl;
		}
		break;

		case eIceEnvironment_Standard:
		{
			msg << std::setw(40) << "ICE Environment: Standard " <<  std::endl;

			msg << std::setw(20) << "Standard ICE Params: " << std::endl;

			msg << std::setw(20) << "STUN password server host name: " << m_sipProxyIpParamsStruct.stunPassServerHostName << std::endl;
			if (eIpVersion4 == m_sipProxyIpParamsStruct.stunPassServerAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.stunPassServerAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "STUN password server IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.stunPassServerAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "STUN password server IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "STUN password server port: " << m_sipProxyIpParamsStruct.stunPassServerAddress.port << std::endl;
			msg << std::setw(20) << "STUN password server user name: " << m_sipProxyIpParamsStruct.stunPassServerUserName << std::endl;
			msg << std::setw(20) << "STUN password server password: " << m_sipProxyIpParamsStruct.stunPassServerPassword << std::endl;
			//msg << std::setw(20) << "STUN password server realim: " << m_sipProxyIpParamsStruct.stunPassServerRealm << std::endl;

			msg << std::setw(20) << "STUN server host name: " << m_sipProxyIpParamsStruct.stunServerHostName << std::endl;
			if (eIpVersion4 == m_sipProxyIpParamsStruct.stunServerUDPAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.stunServerUDPAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "STUN server UDP IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.stunServerUDPAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "STUN server UDP IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "STUN server UDP port: " << m_sipProxyIpParamsStruct.stunServerUDPAddress.port << std::endl;

			if (eIpVersion4 == m_sipProxyIpParamsStruct.stunServerTCPAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.stunServerTCPAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "STUN server TCP IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.stunServerTCPAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "STUN server TCP IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "STUN server TCP port: " << m_sipProxyIpParamsStruct.stunServerTCPAddress.port << std::endl;

			msg << std::setw(20) << "RELAY server host name: " << m_sipProxyIpParamsStruct.RelayServerHostName << std::endl;
			if (eIpVersion4 == m_sipProxyIpParamsStruct.RelayServerUDPAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.RelayServerUDPAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "RELAY server UDP IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.RelayServerUDPAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "RELAY server UDP IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "RELAY server UDP port: " << m_sipProxyIpParamsStruct.RelayServerUDPAddress.port << std::endl;

			if (eIpVersion4 == m_sipProxyIpParamsStruct.RelayServerTCPAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.RelayServerTCPAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "RELAY server TCP IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.RelayServerTCPAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "RELAY server TCP IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "RELAY server TCP port: " << m_sipProxyIpParamsStruct.RelayServerTCPAddress.port << std::endl;
		}
		break;
		case eIceEnvironment_WebRtc:
		{
			msg << std::setw(40) << "ICE Environment: WebRTC " <<  std::endl;

			msg << std::setw(20) << "WebRTC ICE Params: " << std::endl;

			msg << std::setw(20) << "STUN password server host name: " << m_sipProxyIpParamsStruct.stunPassServerHostName << std::endl;
			if (eIpVersion4 == m_sipProxyIpParamsStruct.stunPassServerAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.stunPassServerAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "STUN password server IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.stunPassServerAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "STUN password server IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "STUN password server port: " << m_sipProxyIpParamsStruct.stunPassServerAddress.port << std::endl;
			msg << std::setw(20) << "STUN password server user name: " << m_sipProxyIpParamsStruct.stunPassServerUserName << std::endl;
			msg << std::setw(20) << "STUN password server password: " << m_sipProxyIpParamsStruct.stunPassServerPassword << std::endl;

			msg << std::setw(20) << "STUN server host name: " << m_sipProxyIpParamsStruct.stunServerHostName << std::endl;
			if (eIpVersion4 == m_sipProxyIpParamsStruct.stunServerUDPAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.stunServerUDPAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "STUN server UDP IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.stunServerUDPAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "STUN server UDP IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "STUN server UDP port: " << m_sipProxyIpParamsStruct.stunServerUDPAddress.port << std::endl;

			if (eIpVersion4 == m_sipProxyIpParamsStruct.stunServerTCPAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.stunServerTCPAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "STUN server TCP IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.stunServerTCPAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "STUN server TCP IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "STUN server TCP port: " << m_sipProxyIpParamsStruct.stunServerTCPAddress.port << std::endl;
	
			msg << std::setw(20) << "RELAY server host name: " << m_sipProxyIpParamsStruct.RelayServerHostName << std::endl;
			if (eIpVersion4 == m_sipProxyIpParamsStruct.RelayServerUDPAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.RelayServerUDPAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "RELAY server UDP IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.RelayServerUDPAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "RELAY server UDP IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "RELAY server UDP port: " << m_sipProxyIpParamsStruct.RelayServerUDPAddress.port << std::endl;
	
			if (eIpVersion4 == m_sipProxyIpParamsStruct.RelayServerTCPAddress.ipVersion)
			{
				SystemDWORDToIpString(m_sipProxyIpParamsStruct.RelayServerTCPAddress.addr.v4.ip, strIp);
				msg << std::setw(20) << "RELAY server TCP IpV4: " << strIp << std::endl;
			}
			else
			{
				ipV6ToString(m_sipProxyIpParamsStruct.RelayServerTCPAddress.addr.v6.ip, strIp, TRUE);
				msg << std::setw(20) << "RELAY server TCP IpV6: " << strIp << std::endl;
			}
			msg << std::setw(20) << "RELAY server TCP port: " << m_sipProxyIpParamsStruct.RelayServerTCPAddress.port << std::endl;
		}
		break;
		case eIceEnvironment_None:
		default:
			break;
	}

	msg << "\n\n";
}

// ------------------------------------------------------------
CSipProxyIpParams& CSipProxyIpParams::operator = (const CSipProxyIpParams &rOther)
{
	memset(&m_sipProxyIpParamsStruct, 0, sizeof(SIP_PROXY_IP_PARAMS_S));
	memcpy(&m_sipProxyIpParamsStruct, (SIP_PROXY_IP_PARAMS_S*)&rOther.m_sipProxyIpParamsStruct, sizeof(SIP_PROXY_IP_PARAMS_S));

	return *this;
}

// ------------------------------------------------------------
WORD operator==(const CSipProxyIpParams& lhs,const CSipProxyIpParams& rhs)
{
	return (lhs.m_sipProxyIpParamsStruct.ServiceId == rhs.m_sipProxyIpParamsStruct.ServiceId);
}

// ------------------------------------------------------------
bool operator<(const CSipProxyIpParams& lhs,const CSipProxyIpParams& rhs)
{
	return lhs.m_sipProxyIpParamsStruct.ServiceId < rhs.m_sipProxyIpParamsStruct.ServiceId;
}

// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetServiceId()
{
	return m_sipProxyIpParamsStruct.ServiceId;
}

// ------------------------------------------------------------
void CSipProxyIpParams::SetServiceId(DWORD serviceId)
{
	m_sipProxyIpParamsStruct.ServiceId = serviceId;
}

// ------------------------------------------------------------
/*void CSipProxyIpParams::SetServiceId(DWORD id)
{
	m_sipProxyIpParamsStruct.ServiceId = id;
}*/

// ------------------------------------------------------------
char* CSipProxyIpParams::GetServiceName ()
{
	return m_sipProxyIpParamsStruct.ServiceName;
}

// ------------------------------------------------------------
/*void CSipProxyIpParams::SetServiceName (const char* name)
{
	memcpy(&m_sipProxyIpParamsStruct.ServiceName, name, NET_SERVICE_PROVIDER_NAME_LEN);
}*/

// ------------------------------------------------------------
eIpType	CSipProxyIpParams::GetIpType()
{
	return m_sipProxyIpParamsStruct.IpType;
}

/////////////////////////////////////////////////////////////////
void  CSipProxyIpParams::SetIpType(eIpType eType)
{
	m_sipProxyIpParamsStruct.IpType = eType;
}
/*
// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetIpV4()
{
	return m_sipProxyIpParamsStruct.IpAddressIpV4.addr.v4.ip;
}

// ------------------------------------------------------------
char* CSipProxyIpParams::GetIpV6()
{
	return (char*)( m_sipProxyIpParamsStruct.IpAddressIpV6.addr.v6.ip );
}

// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetIpV6scopeId()
{
	return m_sipProxyIpParamsStruct.IpAddressIpV6.addr.v6.scopeId;
}
// ------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetIpAddressIpV4()
{
   return m_sipProxyIpParamsStruct.IpAddressIpV4;
}
// ------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetIpAddressIpV6()
{
   return m_sipProxyIpParamsStruct.IpAddressIpV6;
}
*/
// ------------------------------------------------------------
/*
DWORD CSipProxyIpParams::GetIpVersion()
{
	return m_sipProxyIpParamsStruct.IpAddress.ipVersion;
}
*/
// ------------------------------------------------------------

char* CSipProxyIpParams::GetUserName ()
{

/*	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	std::string sKey;
	std::string sUserName;

	sKey = "ICE_USER_NAME";
	sysConfig->GetDataByKey(sKey, sUserName);
	if (sUserName.size() > 0 )
	{
		strncpy(m_sipProxyIpParamsStruct.userName,sUserName.c_str(),sUserName.size());

		if(strcmp("NONE", sUserName.c_str()) == 0)
		{
			PTRACE(eLevelInfoNormal,"CSipProxyIpParams::GetUserName - ICE user name is NONE!!! - Need to change user");
			DBGPASSERT(1);
		}
	}
*/
	return m_sipProxyIpParamsStruct.userName;

//	return "rmx187155";
}
// ------------------------------------------------------------
WORD CSipProxyIpParams::GetIsMsICE() ///Need to update
{
	if(m_sipProxyIpParamsStruct.IceType == eIceEnvironment_ms )
		return TRUE;
	else
		return FALSE;
}
// ------------------------------------------------------------
WORD CSipProxyIpParams::GetIsStandartICE()
{
	if(m_sipProxyIpParamsStruct.IceType == eIceEnvironment_Standard )
		return TRUE;
	else
		return FALSE;
}
// ------------------------------------------------------------
WORD CSipProxyIpParams::GetIsWebRtcICE()
{
	if(m_sipProxyIpParamsStruct.IceType == eIceEnvironment_WebRtc )
		return TRUE;
	else
		return FALSE;
}
// ------------------------------------------------------------
eIceEnvironmentType CSipProxyIpParams::GetICEType()
{
	return m_sipProxyIpParamsStruct.IceType;
}
// ------------------------------------------------------------
ipAddressStruct CSipProxyIpParams::GetServiceIpAddress(WORD index)
{
   return m_sipProxyIpParamsStruct.pAddrList[index];
}

void CSipProxyIpParams::SetServiceIpAddressToNull(WORD index,BYTE ipType)
{
	memset(&(m_sipProxyIpParamsStruct.pAddrList[index]),0,sizeof(ipAddressStruct));
	m_sipProxyIpParamsStruct.pAddrList[index].ipVersion = ipType;
	
}
// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetOutboundProxyIpV4()
{
	return m_sipProxyIpParamsStruct.OutboundProxyAddress.addr.v4.ip;
}
// ------------------------------------------------------------
char* CSipProxyIpParams::GetOutboundProxyIpV6()
{
	return (char*)( m_sipProxyIpParamsStruct.OutboundProxyAddress.addr.v6.ip );
}
// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetOutboundProxyV6scopeId()
{
	return m_sipProxyIpParamsStruct.OutboundProxyAddress.addr.v6.scopeId;
}
// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetOutboundProxyIpVersion()
{
	return m_sipProxyIpParamsStruct.OutboundProxyAddress.ipVersion;
}

// ------------------------------------------------------------
WORD	CSipProxyIpParams::GetOutboundProxyPort()
{
	return m_sipProxyIpParamsStruct.OutboundProxyAddress.port;
}
// ------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetOutboundProxyAddress()
{
	return m_sipProxyIpParamsStruct.OutboundProxyAddress;
}

// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetProxyIpV4()
{
	return m_sipProxyIpParamsStruct.ProxyAddress.addr.v4.ip;
}
// ------------------------------------------------------------
char* CSipProxyIpParams::GetProxyIpV6()
{
	return (char*)( m_sipProxyIpParamsStruct.ProxyAddress.addr.v6.ip );
}

// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetProxyV6scopeId()
{
	return m_sipProxyIpParamsStruct.ProxyAddress.addr.v6.scopeId;
}
// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetProxyIpVersion()
{
	return m_sipProxyIpParamsStruct.ProxyAddress.ipVersion;
}

// ------------------------------------------------------------
WORD CSipProxyIpParams::GetProxyPort()
{
	return m_sipProxyIpParamsStruct.ProxyAddress.port;
}
// ------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetProxyAddress()
{
	return m_sipProxyIpParamsStruct.ProxyAddress;
}

// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetAltProxyIpV4()
{
	return m_sipProxyIpParamsStruct.AlternateProxyAddress.addr.v4.ip;
}

// ------------------------------------------------------------
char* CSipProxyIpParams::GetAltProxyIpV6()
{
	return (char*)( m_sipProxyIpParamsStruct.AlternateProxyAddress.addr.v6.ip );
}

// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetAltProxyV6scopeId()
{
	return m_sipProxyIpParamsStruct.AlternateProxyAddress.addr.v6.scopeId;
}
// ------------------------------------------------------------
DWORD CSipProxyIpParams::GetAltProxyIpVersion()
{
	return m_sipProxyIpParamsStruct.AlternateProxyAddress.ipVersion;
}

// ------------------------------------------------------------
WORD CSipProxyIpParams::GetAltProxyPort()
{
	return m_sipProxyIpParamsStruct.AlternateProxyAddress.port;
}
// ------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetAltProxyAddress()
{
	return m_sipProxyIpParamsStruct.AlternateProxyAddress;
}

// ------------------------------------------------------------
char* CSipProxyIpParams::GetOutboundProxyName()
{
	return m_sipProxyIpParamsStruct.pOutboundProxyName;
}

// ------------------------------------------------------------
char* CSipProxyIpParams::GetProxyName()
{
	return m_sipProxyIpParamsStruct.pProxyName;
}

// ------------------------------------------------------------
char* CSipProxyIpParams::GetAltProxyName()
{
	return m_sipProxyIpParamsStruct.pAltProxyName;
}

// ------------------------------------------------------------
char* CSipProxyIpParams::GetProxyHostName()
{
	return m_sipProxyIpParamsStruct.pProxyHostName;
}

// ------------------------------------------------------------
char* CSipProxyIpParams::GetAltProxyHostName()
{
	return m_sipProxyIpParamsStruct.pAltProxyHostName;
}

// ------------------------------------------------------------
enTransportType CSipProxyIpParams::GetTransportType()
{
	return (enTransportType)m_sipProxyIpParamsStruct.transportType;
}

// ------------------------------------------------------------
BYTE  CSipProxyIpParams::GetServersConfig()
{
	return m_sipProxyIpParamsStruct.serversConfig;
}

// ------------------------------------------------------------
BYTE  CSipProxyIpParams::GetDNSStatus()
{
	return m_sipProxyIpParamsStruct.DNSStatus;
}

// ------------------------------------------------------------
BYTE  CSipProxyIpParams::IsDHCP()
{
	return m_sipProxyIpParamsStruct.Dhcp;
}

// ------------------------------------------------------------
BYTE  CSipProxyIpParams::IsRegOnGoing()
{
	return m_sipProxyIpParamsStruct.RegistrationFlags & REG_ON_GOING;
}

// ------------------------------------------------------------
BYTE  CSipProxyIpParams::IsRegMeetingRooms()
{
	return m_sipProxyIpParamsStruct.RegistrationFlags & REG_MEETING_ROOMS;
}

// ------------------------------------------------------------
BYTE  CSipProxyIpParams::IsRegEQs()
{
	return m_sipProxyIpParamsStruct.RegistrationFlags & REG_ENTRY_QUEUES;
}

// ------------------------------------------------------------
BYTE  CSipProxyIpParams::IsRegFactories()
{
	return m_sipProxyIpParamsStruct.RegistrationFlags & REG_FACTORIES;
}

// ------------------------------------------------------------
BYTE  CSipProxyIpParams::IsRegGWProfiles()
{
	return m_sipProxyIpParamsStruct.RegistrationFlags & REG_GW_PROFILES;
}
// ------------------------------------------------------------
WORD  CSipProxyIpParams::GetRefreshTout()
{
	return m_sipProxyIpParamsStruct.refreshTout;
}

// ------------------------------------------------------------
void  CSipProxyIpParams::SetOnGoingRegistrarStatus(DWORD status)
{
	m_RegistrarStatus = status;
}

// ------------------------------------------------------------
DWORD  CSipProxyIpParams::GetOnGoingRegistrarStatus()
{
	return m_RegistrarStatus;
}

// ------------------------------------------------------------
void 	CSipProxyIpParams::SetOnGoingRegistrarIp(mcTransportAddress proxyIp)
{
	memcpy(&m_RegistrarIp, &proxyIp, sizeof(mcTransportAddress));
}

// ------------------------------------------------------------
mcTransportAddress	CSipProxyIpParams::GetOnGoingRegistrarIp()
{
	return m_RegistrarIp;
}
// ------------------------------------------------------------
DWORD	CSipProxyIpParams::GetOnGoingRegistrarV4Ip()
{
	if(eIpVersion4 == m_RegistrarIp.ipVersion)
		return m_RegistrarIp.addr.v4.ip;
	else
		return 0;
}
char G_szEmpty[2]="";
// ------------------------------------------------------------
char*	CSipProxyIpParams::GetOnGoingRegistrarV6Ip()
{
	if(eIpVersion6 == m_RegistrarIp.ipVersion)
		return (char*)m_RegistrarIp.addr.v6.ip;
	else
		return G_szEmpty;

}

// ------------------------------------------------------------
void  CSipProxyIpParams::SetOnGoingAltRegistrarStatus(DWORD status)
{
	m_AltRegistrarStatus = status;
}

// ------------------------------------------------------------
DWORD  CSipProxyIpParams::GetOnGoingAltRegistrarStatus()
{
	return m_AltRegistrarStatus;
}

// ------------------------------------------------------------
void 	CSipProxyIpParams::SetOnGoingAltRegistrarIp(mcTransportAddress proxyIp)
{
	memcpy(&m_AltRegistrarIp, &proxyIp,sizeof(mcTransportAddress));
}

// ------------------------------------------------------------
mcTransportAddress	CSipProxyIpParams::GetOnGoingAltRegistrarIp()
{
	return m_AltRegistrarIp;
}
// ------------------------------------------------------------
DWORD	CSipProxyIpParams::GetOnGoingAltRegistrarV4Ip()
{
	if(eIpVersion4 == m_AltRegistrarIp.ipVersion)
		   return m_AltRegistrarIp.addr.v4.ip;
	else
		return 0;
}
// ------------------------------------------------------------
char*	CSipProxyIpParams::GetOnGoingAltRegistrarV6Ip()
{
	if(eIpVersion6 == m_AltRegistrarIp.ipVersion)
		return (char*)m_AltRegistrarIp.addr.v6.ip;
	else
		return G_szEmpty;

}

// ------------------------------------------------------------
WORD  CSipProxyIpParams::GetSipServerType()
{
	return m_sipProxyIpParamsStruct.SipServerType;
}
//------------------------------------------------------------
char* CSipProxyIpParams::GetSTUNpassHostName()
{
	return m_sipProxyIpParamsStruct.stunPassServerHostName;
}
//------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetSTUNpassIpAddress()
{
	return m_sipProxyIpParamsStruct.stunPassServerAddress;
}
//------------------------------------------------------------
char* CSipProxyIpParams::GetStunServerHostName()
{
	return m_sipProxyIpParamsStruct.stunServerHostName;
}
//------------------------------------------------------------
DWORD CSipProxyIpParams::GetSTUNIpVersion()
{
	return m_sipProxyIpParamsStruct.stunPassServerAddress.ipVersion;
}
// ------------------------------------------------------------
DWORD	CSipProxyIpParams::GetStunPassV4Ip()
{
	return m_sipProxyIpParamsStruct.stunPassServerAddress.addr.v4.ip;
}
// ------------------------------------------------------------
char*	CSipProxyIpParams::GetStunPassV6Ip()
{
	return (char*)(m_sipProxyIpParamsStruct.stunPassServerAddress.addr.v6.ip);
}
// ------------------------------------------------------------
DWORD	CSipProxyIpParams::GetStunPassPort()
{
	return m_sipProxyIpParamsStruct.stunPassServerAddress.port;
}
// ------------------------------------------------------------
char*	CSipProxyIpParams::GetStunPassUserName()
{
	return m_sipProxyIpParamsStruct.stunPassServerUserName;
}
// ------------------------------------------------------------
char*	CSipProxyIpParams::GetStunPassPassword()
{
	return m_sipProxyIpParamsStruct.stunPassServerPassword;
}
// ------------------------------------------------------------
char*	CSipProxyIpParams::GetStunPassRealm()
{
	return m_sipProxyIpParamsStruct.stunPassServerRealm;
}
//------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetStunUdpIpAddress()
{
	return m_sipProxyIpParamsStruct.stunServerUDPAddress;
}

//------------------------------------------------------------
DWORD CSipProxyIpParams::GetStunUDPIpVersion()
{
	return m_sipProxyIpParamsStruct.stunServerUDPAddress.ipVersion;
}
//------------------------------------------------------------
DWORD CSipProxyIpParams::GetStunUDPV4Ip()
{
	return m_sipProxyIpParamsStruct.stunServerUDPAddress.addr.v4.ip;
}
//------------------------------------------------------------
char*  CSipProxyIpParams::GetStunUDPV6Ip()
{
	return (char*)(m_sipProxyIpParamsStruct.stunServerUDPAddress.addr.v6.ip);
}
//------------------------------------------------------------
DWORD CSipProxyIpParams::GetStunUDPPort()
{
	return m_sipProxyIpParamsStruct.stunServerUDPAddress.port;
}
//------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetStunTcpIpAddress()
{
	return m_sipProxyIpParamsStruct.stunServerTCPAddress;
}
//------------------------------------------------------------
DWORD CSipProxyIpParams::GetStunTCPIpVersion()
{
	return m_sipProxyIpParamsStruct.stunServerTCPAddress.ipVersion;
}
//------------------------------------------------------------
DWORD CSipProxyIpParams::GetStunTCPV4Ip()
{
	return m_sipProxyIpParamsStruct.stunServerTCPAddress.addr.v4.ip;
}
//------------------------------------------------------------
char*  CSipProxyIpParams::GetStunTCPV6Ip()
{
	return (char*)(m_sipProxyIpParamsStruct.stunServerTCPAddress.addr.v6.ip);
}
//-----------------------------------------------------------
DWORD CSipProxyIpParams::GetStunTCPPort()
{
	return m_sipProxyIpParamsStruct.stunServerTCPAddress.port;
}
//------------------------------------------------------------
char * CSipProxyIpParams::GetRelayServerHostName()
{
	return m_sipProxyIpParamsStruct.RelayServerHostName;
}
//------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetRelayUdpIpAddress()
{
	return m_sipProxyIpParamsStruct.RelayServerUDPAddress;
}

//-----------------------------------------------------------
DWORD CSipProxyIpParams::GetRelayUdpIpVersion()
{
	return m_sipProxyIpParamsStruct.RelayServerUDPAddress.ipVersion;
}
//-----------------------------------------------------------
DWORD CSipProxyIpParams::GetRelayUdpV4Ip()
{
	return m_sipProxyIpParamsStruct.RelayServerUDPAddress.addr.v4.ip;
}
//-----------------------------------------------------------
char*  CSipProxyIpParams::GetRelayUdpV6Ip()
{
	return (char*)(m_sipProxyIpParamsStruct.RelayServerUDPAddress.addr.v6.ip);
}
//-----------------------------------------------------------
DWORD CSipProxyIpParams::GetRelayUdpPort()
{
	return m_sipProxyIpParamsStruct.RelayServerUDPAddress.port;
}
//------------------------------------------------------------
mcTransportAddress CSipProxyIpParams::GetRelayTcpIpAddress()
{
	return m_sipProxyIpParamsStruct.RelayServerTCPAddress;
}
//-----------------------------------------------------------
DWORD CSipProxyIpParams::GetRelayTcpIpVersion()
{
	return m_sipProxyIpParamsStruct.RelayServerTCPAddress.ipVersion;
}
//-----------------------------------------------------------
DWORD CSipProxyIpParams::GetRelayTcpV4Ip()
{
	return m_sipProxyIpParamsStruct.RelayServerTCPAddress.addr.v4.ip;
}
//-----------------------------------------------------------
char*  CSipProxyIpParams::GetRelayTcpV6Ip()
{
	return (char*)(m_sipProxyIpParamsStruct.RelayServerTCPAddress.addr.v6.ip);
}
//-----------------------------------------------------------
DWORD CSipProxyIpParams::GetRelayTcpPort()
{
	return m_sipProxyIpParamsStruct.RelayServerTCPAddress.port;
}

