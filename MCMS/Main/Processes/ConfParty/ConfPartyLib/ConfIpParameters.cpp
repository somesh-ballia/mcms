// ConfIpParameters.cpp: implementation of the CConfIpParameters class.
//
//////////////////////////////////////////////////////////////////////



#include <iomanip>
#include "NStream.h"
#include "ConfIpParameters.h"
#include "ObjString.h"



// ------------------------------------------------------------
CConfIpParameters::CConfIpParameters()
{
    memset(&m_dnsParams, 0, sizeof(DnsParamsSt));
}

// ------------------------------------------------------------
CConfIpParameters::CConfIpParameters (const CONF_IP_PARAMS_S &confIpParams)
{
	m_confIpParamsStruct = confIpParams;
}

// ------------------------------------------------------------
CConfIpParameters::~CConfIpParameters ()
{
}

// ------------------------------------------------------------
void  CConfIpParameters::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "ConfIpParameters::Dump\n"
		<< "-----------------------\n";

//	CPObject::Dump(msg);

	msg	<< std::setw(20) << "Service Id: "   << m_confIpParamsStruct.service_id   << "\n"
		<< std::setw(20) << "Service Name: " << m_confIpParamsStruct.service_name << "\n"
		<< std::setw(20) << "Service Default Type: " << m_confIpParamsStruct.service_default_type << "\n";
	
	//msg << CIpParameters(m_confIpParamsStruct.ipParams);

	msg << "\n\n";
}

// ------------------------------------------------------------
CConfIpParameters& CConfIpParameters::operator = (const CConfIpParameters &rOther)
{
	if(this != &rOther)
	{
		m_confIpParamsStruct = rOther.m_confIpParamsStruct;
	}

	return *this;
}

// ------------------------------------------------------------
WORD operator==(const CConfIpParameters& lhs,const CConfIpParameters& rhs)
{
	return (lhs.m_confIpParamsStruct.service_id == rhs.m_confIpParamsStruct.service_id);
}

// ------------------------------------------------------------
bool operator<(const CConfIpParameters& lhs,const CConfIpParameters& rhs)
{
	return lhs.m_confIpParamsStruct.service_id < rhs.m_confIpParamsStruct.service_id;
}

// ------------------------------------------------------------
CONF_IP_PARAMS_S* CConfIpParameters::GetConfIpParamsStruct()
{
	return &m_confIpParamsStruct;
}

// ------------------------------------------------------------
BOOL CConfIpParameters::isGKExternal() 
{
	return m_confIpParamsStruct.is_gk_external;
}

// ------------------------------------------------------------
ipAddressStruct *  CConfIpParameters::GetGKIp() 
{
	return &(m_confIpParamsStruct.gk_ip);
}

// ------------------------------------------------------------
ALIAS_S* CConfIpParameters::GetAliasArray()
{
	return m_confIpParamsStruct.aliases;
}
// ------------------------------------------------------------
ALIAS_S  CConfIpParameters::GetDialIn()
{
	return m_confIpParamsStruct.dialIn;
}
// ------------------------------------------------------------
ipAddressIf CConfIpParameters::GetIpV4Address()
{
	return m_confIpParamsStruct.cs_ipV4;
}
// ------------------------------------------------------------
ipAddressIf* CConfIpParameters::GetIpV6AddressArr()
{
	return m_confIpParamsStruct.cs_ipV6Array;
}
// ------------------------------------------------------------
ipAddressIf CConfIpParameters::GetIpV6Address(int idx)
{
	if ( (0 <= idx) && (NUM_OF_IPV6_ADDRESSES > idx) )
		return m_confIpParamsStruct.cs_ipV6Array[idx];
	else
		return m_confIpParamsStruct.cs_ipV6Array[0];
}
// ------------------------------------------------------------
eIpType CConfIpParameters::GetIPAddressTypesInService()
{
	return m_confIpParamsStruct.service_ip_protocol_types; // eIpType
}

// ------------------------------------------------------------
void CConfIpParameters::SetIPAddressTypesInService(eIpType	ipVType)
{
	m_confIpParamsStruct.service_ip_protocol_types = ipVType;
}

// ------------------------------------------------------------
DWORD CConfIpParameters::GetServiceId()
{
	return m_confIpParamsStruct.service_id;
}

// ------------------------------------------------------------
void CConfIpParameters::SetServiceId(DWORD id)
{
	m_confIpParamsStruct.service_id = id;
}

// ------------------------------------------------------------
BYTE* CConfIpParameters::GetServiceName ()
{
	return (BYTE*)m_confIpParamsStruct.service_name;
}

// ------------------------------------------------------------
void CConfIpParameters::SetServiceName (const BYTE* name)
{
	memcpy(&m_confIpParamsStruct.default_service_name, name, NET_SERVICE_PROVIDER_NAME_LEN);
}
// ------------------------------------------------------------
void CConfIpParameters::SetData(const CONF_IP_PARAMS_S *data)
{
	m_confIpParamsStruct = *data;
}

// ------------------------------------------------------------
const CSmallString CConfIpParameters::GetRegistrarDomainName() const
{
	CSmallString str(m_confIpParamsStruct.sip.registrar.domainName);
	return str;
}

// ------------------------------------------------------------
const CSmallString CConfIpParameters::GetLocalDomainName() const
{
	CSmallString str(m_confIpParamsStruct.domain_name);
	return str;
}

// ------------------------------------------------------------
const CSmallString CConfIpParameters::GetSipProxyName() const
{
	CSmallString str(m_confIpParamsStruct.sip.proxy.hostName);
	return str;
}

// ------------------------------------------------------------
DWORD CConfIpParameters::GetSipProxyIpV4Address()
{
	const char * temp  = m_confIpParamsStruct.sip.proxy.hostName;
	DWORD  result      = 0;
	BYTE * b           = ((BYTE *) &result) + 3;
	BYTE   after_dot   = TRUE;  // are we just after a dot or in the first char
	int    digits      = 0;     // how many digits we already found in the current number
	int    dots        = 0;     // how many dots we already found

	while (*temp)
	{		
		if (*temp >= '0' && *temp <= '9')
		{
			if (*b == 0 && !after_dot)
				return 0; // leading zeros are not allowed

			after_dot = FALSE;
			DWORD current_num = (*b) * 10 + (*temp - '0');

			if (current_num > 255)
				return 0; // each number should be in the range of (0-255)

			*b = (BYTE)current_num;
			digits++;
		}
		else 
		{
			if (*temp == '.')
			{
				if (after_dot)
					return 0; // '.' can't start the string, or b just after another

				if (dots >= 3)
					return 0; // more than 3 dots are not allowed

				dots++;
				b--;
				after_dot = TRUE;
			}
			else
				return 0; // ip string should contain only ['0'..'9','.'] chars
		}

		temp++;
	} // while (*temp)

	if (dots != 3) // 3 dots are mandatory
		return 0;

	if (after_dot) // ip string can't end with dot
		return 0;

	return result; // parsing is OK	
}

// ------------------------------------------------------------
DWORD CConfIpParameters::GetSipProxyStatus()
{
	return m_confIpParamsStruct.sip.proxy.status;
}
// ------------------------------------------------------------
QOS_S CConfIpParameters::GetQOS()
{
	return m_confIpParamsStruct.qos;
}

// ------------------------------------------------------------
BYTE CConfIpParameters::GetSipTransportType()
{
	return m_confIpParamsStruct.sip.transportType;
}

// ------------------------------------------------------------
WORD CConfIpParameters::GetConfigurationOfSipServers()
{
	return m_confIpParamsStruct.sip.configureSIPServersMode;
}

// ------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
void CConfIpParameters::SetNumOfDnsWaiting(BYTE num)
{
	m_dnsParams.numOfWaiting = num;
}

//////////////////////////////////////////////////////////////////////////////
BYTE CConfIpParameters::GetNumOfDnsWaiting()
{
	return m_dnsParams.numOfWaiting;
}

//////////////////////////////////////////////////////////////////////////////
void CConfIpParameters::IncreaseNumOfDnsWaiting()
{
	m_dnsParams.numOfWaiting++;
}

//////////////////////////////////////////////////////////////////////////////
void CConfIpParameters::DecreaseNumOfDnsWaiting()
{
	m_dnsParams.numOfWaiting--;
}

//////////////////////////////////////////////////////////////////////////////
void CConfIpParameters::SetProxyIpFromDns(ipAddressStruct* pProxyAddr)
{
	memcpy(&(m_dnsParams.ProxyIpFromDns),pProxyAddr,sizeof(ipAddressStruct));
}
//////////////////////////////////////////////////////////////////////////////
WORD CConfIpParameters::GetICEType()
{
	return m_confIpParamsStruct.sip.IceType;
}
//////////////////////////////////////////////////////////////////////////////
void CConfIpParameters::SetICEType(WORD Type)
{
	 m_confIpParamsStruct.sip.IceType=Type;
}
//////////////////////////////////////////////////////////////////////////////
BYTE CConfIpParameters::GetIsDefaultH323SipService()const
{
	return m_confIpParamsStruct.service_default_type;
}
//////////////////////////////////////////////////////////////////////////////
void CConfIpParameters::SetIsDefaultH323SipService( BYTE val )
{
	m_confIpParamsStruct.service_default_type = val;
}

//////////////////////////////////////////////////////////////////////////////
WORD CConfIpParameters::GetSipServerType()
{
	return m_confIpParamsStruct.sip.SipServerType;
}
//////////////////////////////////////////////////////////////////////////////
void CConfIpParameters::SetSipServerType(WORD Type)
{
	 m_confIpParamsStruct.sip.SipServerType=Type;
}
