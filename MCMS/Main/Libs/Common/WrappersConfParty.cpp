#include <iomanip>

#include "WrappersConfParty.h"
#include "WrappersGK.h"
#include "SystemFunctions.h"

extern const char* IPv6ScopeIdToString(enScopeId theScopeId);


/*-----------------------------------------------------------------------------
	class CBaseSipServerWrapper
-----------------------------------------------------------------------------*/
CBaseSipServerWrapper::CBaseSipServerWrapper(const BASE_SIP_SERVER_S &data)
:m_Data(data)
{}

CBaseSipServerWrapper::~CBaseSipServerWrapper()
{}
	
void CBaseSipServerWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "BASE_SIP_SERVER_S::Dump");
	
	os << std::setw(20) << "Status: "   << m_Data.status << std::endl;
	os << std::setw(20) << "Host Name: "<< m_Data.hostName << std::endl;
	os << std::setw(20) << "Port: " 	<< m_Data.port << std::endl;
}


/*-----------------------------------------------------------------------------
	class CSipServerWrapper
-----------------------------------------------------------------------------*/
CSipServerWrapper::CSipServerWrapper(const SIP_SERVER_S &data)
:m_Data(data)
{}


CSipServerWrapper::~CSipServerWrapper()
{}


void CSipServerWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "SIP_SERVER_S::Dump");
	
	os << CBaseSipServerWrapper(m_Data.baseSipServer);
	os << std::setw(20) << "Domain Name: "   	<< m_Data.domainName << std::endl;
}


/*-----------------------------------------------------------------------------
	class CQosWrapper
-----------------------------------------------------------------------------*/
CQosWrapper::CQosWrapper(const QOS_S &data)
:m_Data(data)
{}

CQosWrapper::~CQosWrapper()
{}
	
void CQosWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "QOS_S::Dump");
	
	os << std::setw(20) << "Is Default: "   	<< (WORD)m_Data.m_IsDefault << std::endl;
	os << std::setw(20) << "Status: " 			<< (WORD)m_Data.m_eIpStatus << std::endl;
	os << std::setw(20) << "IpIsDiffServ: " 	<< (WORD)m_Data.m_bIpIsDiffServ << std::endl;
	os << std::setw(20) << "Tos: "   			<< (WORD)m_Data.m_bIpValueTOS << std::endl;
	os << std::setw(20) << "Precedence Audio: " << (WORD)m_Data.m_bIpPrecedenceAudio << std::endl;
	os << std::setw(20) << "Precedence Video: " << (WORD)m_Data.m_bIpPrecedenceVideo << std::endl;
	
	os << std::setw(20) << "Atm Status: "   		<< (WORD)m_Data.m_eAtmStatus << std::endl;
	os << std::setw(20) << "Atm Precedence Audio: " << (WORD)m_Data.m_bAtmPrecedenceAudio << std::endl;
	os << std::setw(20) << "Atm Precedence Video: " << (WORD)m_Data.m_bAtmPrecedenceVideo << std::endl;
}	

/*-----------------------------------------------------------------------------
	class CSipWrapper
-----------------------------------------------------------------------------*/
CSipWrapper::CSipWrapper(const SIP_S &data)
:m_Data(data)
{}

CSipWrapper::~CSipWrapper()
{}
	
void CSipWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "SIP_S::Dump");
		
	os << std::setw(20) << "Transport Type: "   				<< (WORD)m_Data.transportType << std::endl;
	os << std::setw(20) << "Configure SIP Servers Mode: " 		<< m_Data.configureSIPServersMode << std::endl;
	os << std::setw(20) << "Registration Ongoing Confrences: " 	<< m_Data.registrationOngoingConfrences << std::endl;
	
	os << std::setw(20) << "Registration Meeting Room: "   	<< m_Data.registrationMeetingRoom << std::endl;
	os << std::setw(20) << "Registration Entry Queue: " 	<< m_Data.registrationEntryQueue << std::endl;
	os << std::setw(20) << "Accept Meet Me: " 				<< m_Data.acceptMeetMe << std::endl;
	
	os << std::setw(20) << "Accept AdHoc: "   	<< m_Data.acceptAdHoc << std::endl;
	os << std::setw(20) << "Accept Factory: " 	<< m_Data.acceptFactory << std::endl;
	os << std::setw(20) << "Refresh Status: " 	<< m_Data.refreshStatus << std::endl;
	
	os << std::setw(20) << "Accept AdHoc: "   	<< m_Data.acceptAdHoc << std::endl;
	os << std::setw(20) << "Accept Factory: " 	<< m_Data.acceptFactory << std::endl;
	os << std::setw(20) << "Refresh Status: " 	<< m_Data.refreshStatus << std::endl;
	
	os << std::setw(20) << "Registration Mode: "   		 << (WORD)m_Data.registrationMode << std::endl;
	os << std::setw(20) << "Refresh Registration Tout: " << m_Data.refreshRegistrationTout << std::endl;
	
	os << std::setw(20) << "IceType: " << m_Data.IceType << std::endl;

	os << CBaseSipServerWrapper(m_Data.proxy);
	os << CBaseSipServerWrapper(m_Data.altProxy);

	os << CSipServerWrapper(m_Data.registrar);
	os << CSipServerWrapper(m_Data.altRegistrar);	
}	





/*-----------------------------------------------------------------------------
	class CConfIpParamWrapper
-----------------------------------------------------------------------------*/
CConfIpParamWrapper::CConfIpParamWrapper(const CONF_IP_PARAMS_S &data)
:m_Data(data)
{
}

CConfIpParamWrapper::~CConfIpParamWrapper()
{
}

void CConfIpParamWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "CONF_IP_PARAMS_S::Dump");
		
	os << std::setw(20) << "Service Id: "   	<< m_Data.service_id << std::endl;
	os << std::setw(20) << "Service Name: " 	<< m_Data.service_name << std::endl;
	os << std::setw(20) << "Service Default Type: " << (WORD)m_Data.service_default_type << std::endl;
	os << std::setw(20) << "Service Protocol Type: "<< m_Data.service_protocol_type << std::endl;
	os << std::setw(20) << "Domain Name: " 		<< m_Data.domain_name << std::endl;
	
	os << std::setw(20) << "Ip addres type: ";
	if (m_Data.service_ip_protocol_types == eIpType_None)
		os << "eIpType_None" << std::endl;
	else if (m_Data.service_ip_protocol_types == eIpType_IpV4)
		os << "eIpType_IpV4" << std::endl;
	else if (m_Data.service_ip_protocol_types == eIpType_IpV6)
		os << "eIpType_IpV6" << std::endl;
	else if (m_Data.service_ip_protocol_types == eIpType_Both)
		os << "eIpType_Both" << std::endl;
	char strIp[128];
	SystemDWORDToIpString(m_Data.cs_ipV4.v4.ip, strIp);
	os << std::setw(20) << "CS IpV4: "   		<< strIp << std::endl;

	mcTransportAddress tempAddr;
	char serAddr[IPV6_ADDRESS_LEN];
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(&tempAddr,0,sizeof(mcTransportAddress));
		memset(serAddr,'\0',IPV6_ADDRESS_LEN);
		memcpy(&(tempAddr.addr),&(m_Data.cs_ipV6Array[i]),sizeof(ipAddressIf));
		tempAddr.ipVersion = eIpVersion6;
		::ipToString(tempAddr,serAddr,1);
		
		os << "CS IpV6 "   << i << ":" << std::endl;
		os << std::setw(20) << "Address: "  << serAddr << std::endl;
		os << std::setw(20) << "ScopeId: "	<< IPv6ScopeIdToString( (enScopeId)(m_Data.cs_ipV6Array[i].v6.scopeId) ) << std::endl;
	}

	os << std::setw(20) << "Is GK External: " 	<< BOOL_TO_STRING(m_Data.is_gk_external) << std::endl;
    os <<CIpAddrStructWrapper(m_Data.gk_ip, "GK IP");
    
    os << std::setw(20) << "Is Avf On: "        << BOOL_TO_STRING(m_Data.isAvfOn) << std::endl;
		
	os << CQosWrapper(m_Data.qos);
	os << CSipWrapper(m_Data.sip);
	
	for(int i = 0 ; i < MAX_ALIAS_NAMES_NUM ; i++)
	{
		os << CAliasWrapper(m_Data.aliases[i]);
	}
	
	os << "Dialin Dump" << std::endl;
	os << CAliasWrapper(m_Data.dialIn);
	
	os << std::setw(20) << "Future Use 1: " 	<< m_Data.future_use1 << std::endl;
	os << std::setw(20) << "Future Use 2: " 	<< m_Data.future_use2 << std::endl;	
}
