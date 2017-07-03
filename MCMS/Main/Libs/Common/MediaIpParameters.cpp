//////////////////////////////////////////////////////////////////////
//
// MediaIpParameters.cpp: implementation of the CMediaIpParameters class.
//
//////////////////////////////////////////////////////////////////////


#include <iomanip>
#include "NStream.h"
#include "MediaIpParameters.h"
#include "ProductType.h"
#include "StringsLen.h"
#include "SystemFunctions.h"


// ------------------------------------------------------------
CMediaIpParameters::CMediaIpParameters ()
{
    memset(&m_mediaIpParamsStruct, 0, sizeof(MEDIA_IP_PARAMS_S) );
}

// ------------------------------------------------------------
CMediaIpParameters::CMediaIpParameters (const MEDIA_IP_PARAMS_S mediaIpParams)
{
	memcpy( &m_mediaIpParamsStruct,
		    &mediaIpParams,
			sizeof(MEDIA_IP_PARAMS_S) );
}

// ------------------------------------------------------------
CMediaIpParameters::CMediaIpParameters (const CMediaIpParameters& rOther)
:CPObject(rOther)
{
	memcpy( &m_mediaIpParamsStruct,
		    &(rOther.m_mediaIpParamsStruct),
			sizeof(MEDIA_IP_PARAMS_S) );

	*this = rOther;
}

// ------------------------------------------------------------
CMediaIpParameters::~CMediaIpParameters ()
{
}

// ------------------------------------------------------------
void  CMediaIpParameters::Dump(std::ostream& msg) const
{
	char ipAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(m_mediaIpParamsStruct.csIp, ipAddressStr);	
	
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "MediaIpParameters::Dump\n"
		<< "-----------------------\n";

//	CPObject::Dump(msg);

	msg	<< std::setw(20) << "Service Id: "   << m_mediaIpParamsStruct.serviceId   << "\n"
		<< std::setw(20) << "Service Name: " << m_mediaIpParamsStruct.serviceName << "\n"
		<< std::setw(20) << "CS Ip: " << ipAddressStr << "\n";
	
	msg << CIpParameters(m_mediaIpParamsStruct.ipParams);

	msg << "\n\n";
}

// ------------------------------------------------------------
CMediaIpParameters& CMediaIpParameters::operator = (const CMediaIpParameters &rOther)
{
	memcpy( &m_mediaIpParamsStruct,
		    &(rOther.m_mediaIpParamsStruct),
			sizeof(MEDIA_IP_PARAMS_S) );

	return *this;
}

// ------------------------------------------------------------
WORD operator==(const CMediaIpParameters& lhs,const CMediaIpParameters& rhs)
{
	return (lhs.m_mediaIpParamsStruct.serviceId == rhs.m_mediaIpParamsStruct.serviceId);
}

// ------------------------------------------------------------
bool operator<(const CMediaIpParameters& lhs,const CMediaIpParameters& rhs)
{
	return lhs.m_mediaIpParamsStruct.serviceId < rhs.m_mediaIpParamsStruct.serviceId;
}

// ------------------------------------------------------------
MEDIA_IP_PARAMS_S* CMediaIpParameters::GetMediaIpParamsStruct()
{
	return &m_mediaIpParamsStruct;
}

// ------------------------------------------------------------
IP_PARAMS_S& CMediaIpParameters::GetIpParams ()
{
	return m_mediaIpParamsStruct.ipParams;
}

// ------------------------------------------------------------
void CMediaIpParameters::SetIpParams (const IP_PARAMS_S *ipParams)
{
	memcpy( &m_mediaIpParamsStruct.ipParams, ipParams, sizeof(IP_PARAMS_S) );
}

// ------------------------------------------------------------
DWORD CMediaIpParameters::GetServiceId()
{
	return m_mediaIpParamsStruct.serviceId;
}

// ------------------------------------------------------------
void CMediaIpParameters::SetServiceId(DWORD id)
{
	m_mediaIpParamsStruct.serviceId = id;
}

// ------------------------------------------------------------
BYTE* CMediaIpParameters::GetServiceName ()
{
	return m_mediaIpParamsStruct.serviceName;
}

// ------------------------------------------------------------
void CMediaIpParameters::SetServiceName (const BYTE* name)
{
	memcpy(&m_mediaIpParamsStruct.serviceName, name, NET_SERVICE_PROVIDER_NAME_LEN);
}

// ------------------------------------------------------------
void CMediaIpParameters::SetBoardId(DWORD id)
{
	m_boardId = id;
}

// ------------------------------------------------------------
DWORD CMediaIpParameters::GetBoardId()
{
	return m_boardId;
}

// ------------------------------------------------------------
void CMediaIpParameters::SetSubBoardId(DWORD id)
{
	m_subBoardId = id;
}

// ------------------------------------------------------------
DWORD CMediaIpParameters::GetSubBoardId()
{
	return m_subBoardId;
}

// ------------------------------------------------------------
void CMediaIpParameters::SetMediaIpParamsPlatformType(ePlatformType theType)
{
	m_mediaIpParamsStruct.platformType = (APIU32)theType;
}

// ------------------------------------------------------------
ePlatformType CMediaIpParameters::GetMediaIpParamsPlatformType()
{
	return (ePlatformType)(m_mediaIpParamsStruct.platformType);
}

// ------------------------------------------------------------
void CMediaIpParameters::SetIpTypeInAllInterfaces(eIpType theType)
{
	for (int i=0; i<MAX_NUM_OF_PQS; i++)
	{
		m_mediaIpParamsStruct.ipParams.interfacesList[i].ipType = (APIU32)theType;
	}
}

// ------------------------------------------------------------
void CMediaIpParameters::SetIpV6ConfigurationTypeInAllInterfaces(eV6ConfigurationType theType)
{
	for (int i=0; i<MAX_NUM_OF_PQS; i++)
	{
		for (int j=0; j<NUM_OF_IPV6_ADDRESSES; j++)
		{
			m_mediaIpParamsStruct.ipParams.interfacesList[i].iPv6s[j].configurationType = (APIU32)theType;
		}
	}
}

// ------------------------------------------------------------
void CMediaIpParameters::SetData(const char *data)
{
	memcpy(&m_mediaIpParamsStruct, data, sizeof(MEDIA_IP_PARAMS_S));
	
	m_mediaIpParamsStruct.serviceName[NET_SERVICE_PROVIDER_NAME_LEN-2] = '\0';
}

// ------------------------------------------------------------
void CMediaIpParameters::SetData(MEDIA_IP_PARAMS_S &mediaIpParams)
{
	memcpy(&m_mediaIpParamsStruct, &mediaIpParams, sizeof(MEDIA_IP_PARAMS_S));

	m_mediaIpParamsStruct.serviceName[NET_SERVICE_PROVIDER_NAME_LEN-2] = '\0';
}

// ------------------------------------------------------------
void CMediaIpParameters::SetData(CS_MEDIA_IP_PARAMS_S &csMediaIpParams)
{
	// ===== 1. clear intefacesList
	memset( &m_mediaIpParamsStruct.ipParams.interfacesList,
	        0,
	        sizeof(m_mediaIpParamsStruct.ipParams.interfacesList) );

	// ===== 2. copy networkParams
	memcpy( &m_mediaIpParamsStruct.ipParams.networkParams,
		    &csMediaIpParams.ipParams.networkParams,
			sizeof(NETWORK_PARAMS_S) );

	// ===== 3. copy dnsConfiguration
	memcpy( &m_mediaIpParamsStruct.ipParams.dnsConfig,
    	    &csMediaIpParams.ipParams.dnsConfig,
			sizeof(DNS_CONFIGURATION_S) );

	// ===== 4. copy serviceId
	m_mediaIpParamsStruct.serviceId = csMediaIpParams.serviceId;

	// ===== 5. copy serviceName
	memcpy( &m_mediaIpParamsStruct.serviceName,
			&csMediaIpParams.serviceName,
			NET_SERVICE_PROVIDER_NAME_LEN );

	m_mediaIpParamsStruct.serviceName[NET_SERVICE_PROVIDER_NAME_LEN-2] = '\0';
	
	m_mediaIpParamsStruct.csIp = csMediaIpParams.csIp;

	m_mediaIpParamsStruct.ipParams.v35GwIpv4Address = csMediaIpParams.ipParams.v35GwIpv4Address;
}

// ------------------------------------------------------------
void  CMediaIpParameters::SetVlan(DWORD vlan)
{
	m_mediaIpParamsStruct.ipParams.networkParams.vLanId = vlan;
	m_mediaIpParamsStruct.ipParams.networkParams.vLanMode = YES;
}

// ------------------------------------------------------------
void  CMediaIpParameters::SetMask(DWORD mask)
{
	m_mediaIpParamsStruct.ipParams.networkParams.subnetMask = mask;
}

// ------------------------------------------------------------
void  CMediaIpParameters::SetDefGW(DWORD defgw)
{
	m_mediaIpParamsStruct.ipParams.networkParams.defaultGateway = defgw;
}





//////////////////////////////////////////////////////////////////////
//
// CsMediaIpParameters.cpp: implementation of the CCsMediaIpParameters class.
//
//////////////////////////////////////////////////////////////////////

CCsMediaIpParameters::CCsMediaIpParameters ()
{
}

// ------------------------------------------------------------
CCsMediaIpParameters::CCsMediaIpParameters (const CS_MEDIA_IP_PARAMS_S *mediaIpParams)
{
	memcpy( &m_mediaIpParamsStruct,
		    mediaIpParams,
			sizeof(CS_MEDIA_IP_PARAMS_S) );
}

// ------------------------------------------------------------
CCsMediaIpParameters::CCsMediaIpParameters (const CCsMediaIpParameters &other)
:CPObject()
{
	memcpy( &m_mediaIpParamsStruct,
		    &other.m_mediaIpParamsStruct,
			sizeof(CS_MEDIA_IP_PARAMS_S) );

	*this=other;
}

// ------------------------------------------------------------
CCsMediaIpParameters::~CCsMediaIpParameters ()
{
}

// ------------------------------------------------------------
void  CCsMediaIpParameters::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n\n"
		<< "CsMediaIpParameters::Dump\n"
		<< "-------------------------\n";

//	CPObject::Dump(msg);
	char ipAddressStr[IP_ADDRESS_LEN];
	SystemDWORDToIpString(m_mediaIpParamsStruct.csIp, ipAddressStr);	
	

	msg	<< std::setw(20) << "Service Id: "   << m_mediaIpParamsStruct.serviceId   << "\n"
		<< std::setw(20) << "Service Name: " << m_mediaIpParamsStruct.serviceName << "\n"
		<< std::setw(20) << "CS Ip: " << ipAddressStr << "\n";
	
//	msg << CIpParameters(m_mediaIpParamsStruct.ipParams);
	msg << CCsIpParameters(m_mediaIpParamsStruct.ipParams);
	

	msg << "\n\n";
}

// ------------------------------------------------------------
CCsMediaIpParameters& CCsMediaIpParameters::operator = (const CCsMediaIpParameters &rOther)
{
	memcpy(&m_mediaIpParamsStruct, &rOther.m_mediaIpParamsStruct, sizeof(CS_MEDIA_IP_PARAMS_S));

	return *this;
}

// ------------------------------------------------------------
WORD operator==(const CCsMediaIpParameters& lhs,const CCsMediaIpParameters& rhs)
{
	return (lhs.m_mediaIpParamsStruct.serviceId == rhs.m_mediaIpParamsStruct.serviceId);
}

// ------------------------------------------------------------
bool operator<(const CCsMediaIpParameters& lhs,const CCsMediaIpParameters& rhs)
{
	return lhs.m_mediaIpParamsStruct.serviceId < rhs.m_mediaIpParamsStruct.serviceId;
}

// ------------------------------------------------------------
CS_MEDIA_IP_PARAMS_S* CCsMediaIpParameters::GetMediaIpParamsStruct()
{
	return &m_mediaIpParamsStruct;
}

// ------------------------------------------------------------
CS_IP_PARAMS_S& CCsMediaIpParameters::GetIpParams ()
{
	return m_mediaIpParamsStruct.ipParams;
}

// ------------------------------------------------------------
void CCsMediaIpParameters::SetIpParams (const CS_IP_PARAMS_S *ipParams)
{
	memcpy( &m_mediaIpParamsStruct.ipParams, ipParams, sizeof(CS_IP_PARAMS_S) );
}

// ------------------------------------------------------------
DWORD CCsMediaIpParameters::GetServiceId()
{
	return m_mediaIpParamsStruct.serviceId;
}

// ------------------------------------------------------------
void CCsMediaIpParameters::SetServiceId(DWORD id)
{
	m_mediaIpParamsStruct.serviceId = id;
}

// ------------------------------------------------------------
BYTE* CCsMediaIpParameters::GetServiceName ()
{
	return m_mediaIpParamsStruct.serviceName;
}

// ------------------------------------------------------------
void CCsMediaIpParameters::SetServiceName (const BYTE* name)
{
	memcpy(&m_mediaIpParamsStruct.serviceName, name, NET_SERVICE_PROVIDER_NAME_LEN);
}

// ------------------------------------------------------------
void CCsMediaIpParameters::SetBoardId(DWORD id)
{
	m_boardId = id;
}

// ------------------------------------------------------------
DWORD CCsMediaIpParameters::GetBoardId()
{
	return m_boardId;
}

// ------------------------------------------------------------
void CCsMediaIpParameters::SetSubBoardId(DWORD id)
{
	m_subBoardId = id;
}

// ------------------------------------------------------------
DWORD CCsMediaIpParameters::GetSubBoardId()
{
	return m_subBoardId;
}

// ------------------------------------------------------------
void CCsMediaIpParameters::SetData(const char *data)
{
	memcpy(&m_mediaIpParamsStruct, data, sizeof(CS_MEDIA_IP_PARAMS_S));
	
	m_mediaIpParamsStruct.serviceName[NET_SERVICE_PROVIDER_NAME_LEN-2] = '0';
}

// ------------------------------------------------------------
void CCsMediaIpParameters::SetData(CS_MEDIA_IP_PARAMS_S *mediaIpParams)
{
	memcpy(&m_mediaIpParamsStruct, mediaIpParams, sizeof(CS_MEDIA_IP_PARAMS_S));
	
	m_mediaIpParamsStruct.serviceName[NET_SERVICE_PROVIDER_NAME_LEN-2] = '0';
}
