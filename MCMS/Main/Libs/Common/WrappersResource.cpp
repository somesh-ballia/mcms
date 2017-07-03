#include <iomanip>
#include "WrappersResource.h"
#include "SystemFunctions.h"
#include "WrappersCommon.h"

extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern const char* IPv6ScopeIdToString(enScopeId theScopeId);


const char *GetAllocMethodName(eUdpAllocMethod eMethod)
{
    static const char * names [] =
        {
            "Static, allocated by user",
            "Dynamic, allocated by Resource"
        };
    const int namesLen = sizeof(names) / sizeof(names[0]);
    const char *name = (0 <= eMethod && eMethod < namesLen
                        ?
                        names[eMethod] : "Invalid");
    return name;
}






/*-----------------------------------------------------------------------------
	class CUDPResourcePerPQWrapper
-----------------------------------------------------------------------------*/
CUDPResourcePerPQWrapper::CUDPResourcePerPQWrapper(const IP_SERVICE_UDP_RESOURCE_PER_PQ_S &data)
:m_Data(data)
{

}

CUDPResourcePerPQWrapper::~CUDPResourcePerPQWrapper()
{}


void CUDPResourcePerPQWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "IP_SERVICE_UDP_RESOURCE_PER_PQ_S::Dump");

	os << std::setw(20) << "Port Alloc Type: "   << GetAllocMethodName(m_Data.portsAlloctype) << std::endl;
	os << std::setw(20) << "Box Id: " << m_Data.boxId << std::endl;
	os << std::setw(20) << "Board Id: " 	<< m_Data.boardId << std::endl;

	os << std::setw(20) << "Port Alloc Type: "   << m_Data.portsAlloctype << std::endl;
	os << std::setw(20) << "Box Id: " << m_Data.boxId << std::endl;
	os << std::setw(20) << "Board Id: " 	<< m_Data.boardId << std::endl;

	os << std::setw(20) << "Sub Board Id: "   << m_Data.subBoardId << std::endl;
	os << std::setw(20) << "PQ Id: " << m_Data.PQid << std::endl;
	os << std::setw(20) << "Type: " 	<< m_Data.type << std::endl;

	os << std::setw(20) << "IpType: "		<< ::IpTypeToString(m_Data.IpType) << std::endl;
	os << std::setw(20) << "IpV4Addr: "		<< CIPV4Wrapper((ipAddressV4If&)(m_Data.IpV4Addr) ) << std::endl;
	os << std::setw(20) << "IpV6Addr: " 	<< CIPV6AraryWrapper( (ipv6AddressArray&)(*(m_Data.IpV6Addr)) ) << std::endl;

	// ipAddress                    IpAddr;

	os << std::setw(20) << "Udp First Channel: "   << m_Data.UdpFirstPort << std::endl;
	os << std::setw(20) << "Udp Last Channel: " << m_Data.UdpLastPort << std::endl;

}







/*-----------------------------------------------------------------------------
	class CUDPResourceWrapper
-----------------------------------------------------------------------------*/
CUDPResourceWrapper::CUDPResourceWrapper(const IP_SERVICE_UDP_RESOURCES_S &data)
:m_Data(data)
{
}

CUDPResourceWrapper::~CUDPResourceWrapper()
{
}

void CUDPResourceWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "IP_SERVICE_UDP_RESOURCES_S::Dump");

	os << std::setw(20) << "Service Id: "   << m_Data.ServId << std::endl;
	os << std::setw(20) << "Service Name: " << m_Data.ServName << std::endl;
	os << std::setw(20) << "Num of PQ: " 	<< m_Data.numPQSactual << std::endl;

	for(int i = 0 ; i < MAX_NUM_PQS ; i++)
	{
		os << CUDPResourcePerPQWrapper(m_Data.IPServUDPperPQList[i]) << std::endl;
	}
}








/*-----------------------------------------------------------------------------
	class CIpV6AddressResourcesWrapper
-----------------------------------------------------------------------------*/
CIpV6AddressUpdateResourcesWrapper::CIpV6AddressUpdateResourcesWrapper(const IPV6_ADDRESS_UPDATE_RESOURCES_S &data)
:m_Data(data)
{
}

CIpV6AddressUpdateResourcesWrapper::~CIpV6AddressUpdateResourcesWrapper()
{
}

void CIpV6AddressUpdateResourcesWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "IPV6_ADDRESS_UPDATE_RESOURCES_S::Dump");
    
    char strIp[128];
    
	os << std::setw(20) << "Service Id: "   << m_Data.ServId << std::endl;
	
	for (int spanIndex=0; spanIndex<MAX_NUM_PQS; spanIndex++)
	{
		os << std::setw(20) << "Span - "<< spanIndex; 
		os << CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper(m_Data.IPServUDPperPQList[spanIndex]) << std::endl;
	}
}






/*-----------------------------------------------------------------------------
	class CIpServiceUdpResourcesPerPqForIpv6Update
-----------------------------------------------------------------------------*/

CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper::CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper(const IP_SERVICE_UDP_RESOURCE_PER_PQ_FOR_IPV6_UPDATE_S &data)
:m_Data(data)
{
}

CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper::~CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper()
{
}

void CIpServiceUdpResourcesPerPqForIpv6UpdateWrapper::Dump(std::ostream &os)const
{
	DumpHeader(os, "IP_SERVICE_UDP_RESOURCE_PER_PQ_FOR_IPV6_UPDATE_S::Dump");
	
	os << std::setw(20) << "; Box Id: "			<< m_Data.boxId;
	os << std::setw(20) << "; Board Id: "		<< m_Data.boardId;
	os << std::setw(20) << "; Sub Board Id: "	<< m_Data.subBoardId;
	os << std::setw(20) << "; PQ id: "			<< m_Data.PQid;
	os << std::setw(20) << "; IpV6Addr: " 		<< CIPV6AraryWrapper( (ipv6AddressArray&)(*(m_Data.IpV6Addr)) ) << std::endl;

}



/*-----------------------------------------------------------------------------
	class CLastConfIdWrapper
-----------------------------------------------------------------------------*/
CLastConfIdWrapper::CLastConfIdWrapper(const LAST_CONF_ID_S &data)
:m_Data(data)
{
}

CLastConfIdWrapper::~CLastConfIdWrapper()
{
}

void CLastConfIdWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "LAST_CONF_ID_S::Dump");

	os << std::setw(20) << "Last Conf Id: "   << m_Data.last_conf_id << std::endl;
}









/*-----------------------------------------------------------------------------
	class CCFSWrapper
-----------------------------------------------------------------------------*/
CCFSWrapper::CCFSWrapper(const RSRC_CFS_S &data)
:m_Data(data)
{
}

CCFSWrapper::~CCFSWrapper()
{
}

void CCFSWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "RSRC_CFS_S::Dump");

	os << std::setw(20) << "Max Parties Num: "   << m_Data.MaxPartiesNum << std::endl;
    os << std::setw(20) << "Licensing: "   << m_Data.Licensing << std::endl;
}








/*-----------------------------------------------------------------------------
	class CUDPPortRangeWrapper
-----------------------------------------------------------------------------*/
CUDPPortRangeWrapper::CUDPPortRangeWrapper(const UDP_PORT_RANGE_S &data)
:m_Data(data)
{
}

CUDPPortRangeWrapper::~CUDPPortRangeWrapper()
{
}

void CUDPPortRangeWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "UDP_PORT_RANGE_S::Dump");

	os << std::setw(20) << "Service Id: "     << m_Data.ServiceId    << std::endl;
    os << std::setw(20) << "Udp First Port: " << m_Data.UdpFirstPort << std::endl;
    os << std::setw(20) << "Udp Last Port: "  << m_Data.UdpLastPort  << std::endl;
}

