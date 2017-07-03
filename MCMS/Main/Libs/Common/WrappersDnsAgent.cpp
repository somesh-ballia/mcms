#include <iomanip>


#include "WrappersDnsAgent.h"
#include "SystemFunctions.h"


CDnsParamsIpWrapper::CDnsParamsIpWrapper(const DNS_PARAMS_IP_S &data)
:m_Data(data)
{
}

CDnsParamsIpWrapper::~CDnsParamsIpWrapper()
{
}

void CDnsParamsIpWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "DNS_PARAMS_IP_S::Dump");
		
	os << std::setw(20) << "Service Id: "   << m_Data.ServiceId << std::endl;	
	os << std::setw(20) << "domainName: "   << m_Data.domainName << std::endl;
	
	char strIp[128];
//	SystemDWORDToIpString(m_Data.Ip, strIp);
//	os << std::setw(20) << "Ip: " 			<< strIp << std::endl;
	
//	SystemDWORDToIpString(m_Data.Ip.addr.v4.ip, strIp);
//	os << std::setw(20) << "IpV4: " 				<< strIp << std::endl;
	
//	os << std::setw(20) << "IpV6: "   				<< strIp << std::endl;
//	os << std::setw(20) << "IpV6 scopeId: "			<< m_Data.Ip.addr.v6.scopeId << std::endl;
	
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
	
}
