/*
 * CCommonSoftMcuMfwEdgeAxis.cpp
 *
 *  Created on: Aug 1, 2013
 *      Author: stanny
 *      Cases of SMCU family, not including Gesher/Ninja
 */

#include "CCommonSoftMcuMfwEdgeAxis.h"
#include "Trace.h"
#include "TraceStream.h"

namespace McmsNetworkPackage {

CCommonSoftMcuMfwEdgeAxis::CCommonSoftMcuMfwEdgeAxis() {
	// TODO Auto-generated constructor stub
	m_Ipv6_status = STATUS_OK;
}

CCommonSoftMcuMfwEdgeAxis::~CCommonSoftMcuMfwEdgeAxis() {
	// TODO Auto-generated destructor stub
}

STATUS CCommonSoftMcuMfwEdgeAxis::SetIPv4SpanParams()
{

	STATUS stat = STATUS_OK;
	TRACESTR(eLevelInfoNormal) << "CCommonSoftMcuMfwEdgeAxis::SetIPv4SpanParams";
	//read ip from physical address
	string ip_address;
	SystemPipedCommand("echo -n `/sbin/ifconfig | grep -A1 'eth[0-9]' | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1 | awk 'NR==1'`", ip_address);

	bool isIpValid = false;

	if (strstr(ip_address.c_str(),INVALID_SERVER_ADDR_STR) || ip_address == "")
		return MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV4;

	DWORD dword_ipAddress = SystemIpStringToDWORD(ip_address.c_str());
	IP_ADDR_S ipAddrS;
	memset(&ipAddrS,0,sizeof(ipAddrS));
	RetrieveIpAddresses(m_pMngmntService, ipAddrS);


	//Read default gw
	string ipv4_defGw;
	SystemPipedCommand("echo -n `/sbin/ip route show | grep default | cut -d' ' -f3`", ipv4_defGw);
	DWORD dword_defaultGateway = SystemIpStringToDWORD(ipv4_defGw.c_str());
	m_pMngmntService->SetDefaultGatewayIPv4(dword_defaultGateway);

	//Read net mask from first interface
	string net_mask;
	SystemPipedCommand("echo -n `/sbin/ifconfig | grep -A1 'eth[0-9]' | grep Mask | cut -d':' -f4`", net_mask);
	DWORD dword_netMask = SystemIpStringToDWORD(net_mask.c_str());
	m_pMngmntService->SetNetMask(dword_netMask);

	CIPSpan *pSpan0 = m_pMngmntService->GetSpanByIdx(0);
	if(pSpan0)
	{
		pSpan0->SetIPv4Address(dword_ipAddress);
		string interface;
		SystemPipedCommand("echo -n `/sbin/ifconfig | grep -A1 'eth[0-9]' | grep 'Link encap' | cut -d' ' -f1 | awk 'NR==1'`", interface);
		pSpan0->SetInterface(interface);
	}


	TRACESTR(eLevelInfoNormal) << "CCommonSoftMcuMfwEdgeAxis::SetIPv4SpanParams update mngmt service - ip address: " << ip_address
			<< " default gw: " << ipv4_defGw
			<< " net mask: " << net_mask;


	return stat;
}

STATUS CCommonSoftMcuMfwEdgeAxis::SetIPv6SpanParams()
{
	TRACESTR(eLevelInfoNormal) << "CCommonSoftMcuMfwEdgeAxis::SetIPv6SpanParams in.";

		STATUS stat = STATUS_OK;
		eV6ConfigurationType curIpV6configType	= m_pMngmntService->GetIpV6ConfigurationType();
		if(eV6Configuration_Manual == curIpV6configType )
		{
			 m_pMngmntService->SetIpV6ConfigurationType(eV6Configuration_Auto);
		}

		string retStr_ipV6,nic_name;
		MngmentIpv6Array autoIpV6;
		GetMngmntIpv6(autoIpV6, retStr_ipV6, nic_name, m_pMngmntService->GetIpType());
		TRACEINTO << "CCommonSoftMcuMfwEdgeAxis::SetIPv6SpanParams IpV6 for " << nic_name << " : " << retStr_ipV6;

		if(autoIpV6[0].address[0] == '\0')
		{
			stat = MANGMENT_STATUS_INTERFACE_WAITING_FOR_IPV6;
	        TRACESTR(eLevelInfoNormal) << "CCommonSoftMcuMfwEdgeAxis::SetIPv6SpanParams not found ipv6 global address.";
		}
		else
		{
			// for sofmcu edge and mfw we need to update the span since we don't configure ipv6 we take it from OS and update the service .
			// so it is equeal to auto configuration.
			UpdateMngntServiceIPv6Auto(autoIpV6);
	        if (eIpType_IpV6 == m_pMngmntService->GetIpType())
		    {
				CIPSpan* pCntrlSpan = m_pMngmntService->GetSpanByIdx(0);
				if(pCntrlSpan)
				{
					pCntrlSpan->SetInterface(nic_name);
				}
		    }
	        TRACESTR(eLevelInfoNormal) << "CCommonSoftMcuMfwEdgeAxis::SetIPv6SpanParams initialize mngmt IPV6 service"
	        								   << " IpV6 Full: " << autoIpV6[0].address
	        								   << " IpV6 mask: " << autoIpV6[0].mask
	        								   << " nic_name: "  << nic_name;
		}



		m_Ipv6_status = stat;
		return stat;

}

//in case of SMCU family, not including Gesher/Ninja, the first interface will be management and the second
//media, unless there is only one interface. We should read the interface and IP from file, compare
//with the physical values and update the file if they are different.
//Read net address from first interface
STATUS    CCommonSoftMcuMfwEdgeAxis::OnInitInterfaces()
{
	STATUS stat = STATUS_OK;

	eIpType curIpType	= m_pMngmntService->GetIpType();
	SetDhcpClientV6();

	if (eIpType_IpV4 == curIpType || eIpType_Both == curIpType)
		   stat =SetIPv4SpanParams();

	if(MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV4 == stat)
		return stat;

	if (eIpType_IpV6 == curIpType || eIpType_Both == curIpType)
			stat = SetIPv6SpanParams();

	return stat;
}

eConfigInterfaceNum CCommonSoftMcuMfwEdgeAxis::GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType)
{
	PASSERTSTREAM(1,"Invalid Flow GetInterfaceNum should not becalled in soft products SoftMcu,Mfw,EdgeAxis");
	return  eEth0;
}

eConfigDeviceName   CCommonSoftMcuMfwEdgeAxis::GetInterfaceDeviceNum(const eConfigInterfaceType ifType)
{
	PASSERTSTREAM(1,"Invalid Flow GetInterfaceDeviceNum should not becalled in soft products SoftMcu,Mfw,EdgeAxis");
	return eEth_0;
}

//in case of SMCU family, not including Gesher/Ninja, the first interface will be management and the second
//media, unless there is only one interface.
std::string CCommonSoftMcuMfwEdgeAxis::GetLogicalInterfaceName(const eConfigInterfaceType ifType,const eIpType ipType)
{
	std::string interface;
	SystemPipedCommand("echo -n `/sbin/ifconfig | grep -A1 'eth[0-9]' | grep 'Link encap' | cut -d' ' -f1 | awk 'NR==1'`", interface);
	return interface;
}

STATUS   CCommonSoftMcuMfwEdgeAxis::ValidateInterfaceIpv4(IP_ADDR_S& ipAddrS)
{
	// no need to to validate since we set the service from the interface

	return MANGMENT_STATUS_INTERFACE_READY;
}



STATUS CCommonSoftMcuMfwEdgeAxis::SetDhcpClientV6()
{

	STATUS status = STATUS_OK;
	eIpType ipType = m_pMngmntService->GetIpType();
	if ((eIpType_IpV6 == ipType) || (eIpType_Both == ipType))
		{
			TRACESTR(eLevelInfoNormal) << "SetDhcpClientV6 for ipv6 (lync)";
			//DHCP for IPv6 set by flags
			CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
			if (pSysConfig)
			{
				BOOL isDHCP_IPV6;
				pSysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_DHCPV6,isDHCP_IPV6 );
				if(isDHCP_IPV6)
				{
					if(TestMcmsUser())
					{
						std::string ans;
						std::string cmd = MCU_UTILS_DIR+"/Scripts/ManageIPv6.sh Enable";

						STATUS stat = SystemPipedCommand(cmd.c_str(), ans);
						TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "EnableIPv6 DHCP:" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;
					}
					else
						TRACESTR(eLevelInfoNormal) << "On Soft simulation we don't run the dhcp client command";

				}
				else
				{
					if(TestMcmsUser())
					{
						std::string ans;
						std::string cmd = MCU_UTILS_DIR+"/Scripts/ManageIPv6.sh Disable";

						STATUS stat = SystemPipedCommand(cmd.c_str(), ans);

						TRACESTR(stat ? eLevelError:eLevelInfoNormal) << "Disable IPv6 DHCP:" << cmd << std::endl << "answer:" << ans << ", stat:" << stat;
					}
					else
						TRACESTR(eLevelInfoNormal) << "On Soft simulation we don't run the dhcp client command";

				}
			}
		}
	return status;
}

} /* namespace McmsNetworkPackage */
