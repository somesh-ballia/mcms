#include <iomanip>
#include "WrappersCards.h"
#include "SystemFunctions.h"


extern char* CardMediaIpConfigStatusToString(APIU32 mediaIpConfigStatus);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);


/*-----------------------------------------------------------------------------
	class CBaseSipServerWrapper
-----------------------------------------------------------------------------*/
CMediaIpConfigWrapper::CMediaIpConfigWrapper(const MEDIA_IP_CONFIG_S &data)
:m_Data(data)
{}


CMediaIpConfigWrapper::~CMediaIpConfigWrapper()
{}


void CMediaIpConfigWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "MEDIA_IP_CONFIG_S::Dump");

	char* mediaIpConfigStatusStr =
	                  ::CardMediaIpConfigStatusToString( (eMediaIpConfigStatus)(m_Data.status) );

	if (mediaIpConfigStatusStr)
	{
		os << std::setw(20) << "Status: " << mediaIpConfigStatusStr    << std::endl;
	}
	else
	{
		os << std::setw(20) << "Status: (invalid: "<< mediaIpConfigStatusStr << std::endl;
	}

//	os << std::setw(20) << "Status: "   	<< m_Data.status 	<< std::endl;
	os << std::setw(20) << "Service Id: "   << m_Data.serviceId << std::endl;

	char buffer[128];
	SystemDWORDToIpString(m_Data.iPv4.iPv4Address, buffer);

	os	<< std::setw(20) << "IPv4 Address: "	<< buffer							<< std::endl;
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		os	<< "IPv6 Address " << i << ": "		<< m_Data.iPv6[i].iPv6Address	<< std::endl;
	}
	os	<< std::setw(20) << "IP Type: "			<< IpTypeToString(m_Data.ipType)	<< std::endl;
	os << std::setw(20) << "PQ Number: " 	<< m_Data.pqNumber 	<< std::endl;
	os << std::setw(20) << "IPv6 DefGw: " 	<< m_Data.defaultGatewayIPv6 << std::endl;

	os << std::setw(20) << "Future Use 1: "	<< m_Data.future_use1 << std::endl;
	os << std::setw(20) << "Future Use 2: "	<< m_Data.future_use2 << std::endl;
}
