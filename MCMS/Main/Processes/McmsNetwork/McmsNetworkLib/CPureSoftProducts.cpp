/*
 * CPureSoftProducts.cpp
 *
 *  Created on: Aug 14, 2013
 *      Author: stanny
 */

#include "CPureSoftProducts.h"
#include "Trace.h"
#include "TraceStream.h"
#include "OsFileIF.h"
#include "ActiveAlarmDefines.h"
#include "FaultsDefines.h"
#include "SystemFunctions.h"

namespace McmsNetworkPackage {

CMngntSoftMcu::CMngntSoftMcu() {
	// TODO Auto-generated constructor stub

}

CMngntSoftMcu::~CMngntSoftMcu() {
	// TODO Auto-generated destructor stub
}

CMngntMfw::CMngntMfw() {
	// TODO Auto-generated constructor stub
}

CMngntMfw::~CMngntMfw() {
	// TODO Auto-generated destructor stub
}

//////////////////////////////////////////////////////////////////////////////////////////////
//MFW:
//////////////////////////////////////////////////////////////////////////////////////////////
STATUS CMngntMfw::OnPreConfigNetworkMngmnt()
{
    TRACESTR(eLevelInfoNormal) << "CMngntMfw::OnPreConfigNetworkMngmnt";
    eIpType ipType = m_pMngmntService->GetIpType();
    CIPSpan *pIpSpan = m_pMngmntService->GetFirstSpan();
	if(NULL == pIpSpan)
	{
		TRACESTR(eLevelInfoNormal) << "CMngntMfw::CheckMngConfiguredInterface pIpSpan==NULL.";
		return STATUS_FAIL;
	}
	return CheckPerIPSpan(pIpSpan, ipType);
}

STATUS CMngntMfw::CheckStatusWithSystmSetting(CIPSpan* pIpSpan, STATUS status)
{
	TRACEINTO << "CMngntMfw::CheckStatusWithSystmSetting";
	STATUS nRet = status;
	CSystemInterface* pInterface = m_systemInterfaceList.GetSystemInterfaceByName(pIpSpan->GetInterface());
    PASSERT_AND_RETURN_VALUE(NULL == pInterface, nRet);
	switch(nRet)
	{
	case STATUS_IP_ADDRESS_MISMATCHES_SYSTEM:
		if(pInterface->GetSystemInterfaceIp() == 0)
		{
			TRACEINTO << "CMngntMfw::CheckStatusWithSystmSetting - system didn't retrieve IPv4";
			nRet = MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV4;
		}
		break;
	case STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM:
		if(pInterface->GetSystemInterfaceIpv6_global().empty())
		{
			TRACEINTO << "CMngntMfw::CheckStatusWithSystmSetting - system didn't retrieve IPv6 global";
			nRet = MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV6;
		}

		break;
	case STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM:
		if(pInterface->GetSystemInterfaceIpv6_site().empty())
		{
			TRACEINTO << "CMngntMfw::CheckStatusWithSystmSetting - system didn't retrieve IPv6 site";
			nRet = MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV6;
		}
		break;
	case STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM:
		if(pInterface->GetSystemInterfaceIpv6_link().empty())
		{
			TRACEINTO << "CMngntMfw::CheckStatusWithSystmSetting - system didn't retrieve IPv6 link";
			nRet = MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV6;
		}
		break;
	}
	return nRet;

}


STATUS CMngntMfw::CheckPerIPSpan(CIPSpan* pIpSpan, eIpType ipType)
{
	if(pIpSpan->GetInterface().empty())
	{
		return STATUS_MUST_ASSIGN_INTERFACE_IN_MFW;
	}
	STATUS nRet = m_systemInterfaceList.ValidateMFWSPANField(pIpSpan, ipType);

	string start_alarm = "The configuration of the Management service is invalid. Interface " + pIpSpan->GetInterface();
	switch (nRet)
	{
	case STATUS_IP_ADDRESS_MISMATCHES_SYSTEM:
	{
		start_alarm += " with address IpV4:" + m_systemInterfaceList.GetValidatedResult() + " does not exist or is invalid.";
        TRACESTR(eLevelInfoNormal) << "CMngntMfw::CheckPerIPSpan failed d_mngIpV4:" << m_systemInterfaceList.GetValidatedResult();
	}
		break;
	case STATUS_MUST_ASSIGN_INTERFACE_IN_MFW:
	{
		start_alarm += " does not exist or is invalid.";
		TRACESTR(eLevelInfoNormal) << "CMngntMfw::CheckPerIPSpan interface "<< pIpSpan->GetInterface()<< " does bit exist or is invalid.";
	}
		break;
	case STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM:
	{
		start_alarm += " with address IpV6:" + m_systemInterfaceList.GetValidatedResult() + " does not exist or is invalid.";
		TRACESTR(eLevelInfoNormal) << "CMngntMfw::CheckMngConfiguredInterface IpV6 check failed mngSpan_IpV6Full:" <<m_systemInterfaceList.GetValidatedResult();
	}
		break;
	default:
		start_alarm = "";
		break;
	}
	SetAlarm(start_alarm);
	nRet = CheckStatusWithSystmSetting(pIpSpan, nRet);
	return nRet;
}

STATUS CMngntMfw::TryToRecover(STATUS eSelfLastStatus, STATUS eOtherLastStatus, CBaseNetworkPlatform * pOtherNetwork)
{
	STATUS stat = STATUS_OK;
	m_pMngmntService->SetIpType(eIpType_IpV4);
	stat = SetIPv4SpanParams();
	stat = SetIPv6SpanParams();
	ConfigOtherNetComponents();
	m_pMngmntService->WriteXmlFile(MANAGEMENT_NETWORK_CONFIG_PATH, "MngmntNetwork");
	return stat;
}

STATUS CMngntMfw::SetIPv4SpanParams()
{
	TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv4SpanParams";
    
	STATUS stat = STATUS_OK;
	DWORD  dword_ipAddress = 0;
    string first_interface = "";
    string strIpV6_global = "";
    
    //1. get the first available interface, follow is the criteria: 
    //   a. The interface should be UP and RUNNING
    //   b. The interface should have a valid IP address (IPV4 preferred, IPV6 as second priority)
    //   c. The interface should be an Ethernet interface, unless no such interface exists 
    //   d. The interface should not be the loopback (lo) interface
    CIPService stCIPService;
    stCIPService.GetFirstAvailableIpAddressAndInterface(dword_ipAddress, first_interface, strIpV6_global);

	CIPSpan *pSpan0 = m_pMngmntService->GetSpanByIdx(0);
	if ((0 == dword_ipAddress) && (NULL != pSpan0))
	{
	    pSpan0->SetInterface(first_interface);
	    m_pMngmntService->SetIpType(eIpType_IpV6);
        TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv4SpanParams no ipv4 interface, use the ipv6 interface.";
        return stat;
	}
    
    //2. attached all network functionalities(management, signaling, media) to the same network interface and IP address.
	string ipv4_defGw;
	SystemPipedCommand("echo -n `/sbin/ip route show | grep default | cut -d' ' -f3`", ipv4_defGw);
	DWORD dword_defaultGateway = SystemIpStringToDWORD(ipv4_defGw.c_str());
	m_pMngmntService->SetDefaultGatewayIPv4(dword_defaultGateway);

	//Read net mask from first interface
	string net_mask;
    string cmd = "echo -n `/sbin/ifconfig " + first_interface + " | grep 'inet addr' | cut -d':' -f4 | cut -d' ' -f1`";
    SystemPipedCommand(cmd.c_str(), net_mask);
	DWORD dword_netMask = SystemIpStringToDWORD(net_mask.c_str());
	m_pMngmntService->SetNetMask(dword_netMask);

	if(pSpan0)
	{
        pSpan0->SetIPv4Address(dword_ipAddress);
	    pSpan0->SetInterface(first_interface);
        m_pMngmntService->SetIpType(eIpType_Both);
	}

	TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv4SpanParamsinitialize mngmt IPV4 service"
							   << " default gw: "     << ipv4_defGw
							   << " net mask: "       << net_mask
							   << " ip_ethname: "     << first_interface
							   << " dword_ipAddress:" << dword_ipAddress;

    return stat;
}

STATUS CMngntMfw::SetIPv6SpanParams()
{
    TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv6SpanParams in.";
    
	STATUS stat = STATUS_OK;
	eV6ConfigurationType curIpV6configType	= m_pMngmntService->GetIpV6ConfigurationType();
	if(eV6Configuration_Manual == curIpV6configType )
	{
		 m_pMngmntService->SetIpV6ConfigurationType(eV6Configuration_Auto);
	}

    #if 0
    CIPSpan *pSpan0 = m_pMngmntService->GetSpanByIdx(0);
    string nic_name = pSpan0->GetInterface();
    string retStr_ipV6;
    #endif
    
   	DWORD  dword_ipAddress;
    string nic_name;
    string strIpV6_global;
    int iPv6_mask = DEFAULT_IPV6_SUBNET_MASK;
    CIPService stCIPService;
    stCIPService.GetFirstAvailableIpAddressAndInterface(dword_ipAddress, nic_name, strIpV6_global);

    if ("" != strIpV6_global)
    {
        string::size_type position;
        string iPv6_mask_str;
        position = strIpV6_global.find("/");
        if (position != strIpV6_global.npos)
        {
            iPv6_mask_str = strIpV6_global.substr(position+1, strIpV6_global.size()-position);           
            sscanf(iPv6_mask_str.c_str(), "%d", &iPv6_mask);
        }
        TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv6SpanParams iPv6_mask:" << iPv6_mask;
    }
    
    //RetrieveIpAddressConfigured_IpV6(autoIpV6, retStr_ipV6, nic_name);
    TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv6SpanParams nic_name:" << nic_name << " str_ipV6:" << strIpV6_global;

    //now, just set the first ipv6 address.
    //for(int i = 1 ; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
	//{
	//	memset(autoIpV6[i].address, '\0', IPV6_ADDRESS_LEN);
	//	autoIpV6[i].mask = DEFAULT_IPV6_SUBNET_MASK;
	//}
    
    if(strIpV6_global.empty())
	{
		//stat = MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV6;
    	TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv6SpanParams  since IPv6 is none, set IP_TYPE to Ipv4";
        m_pMngmntService->SetIpType(eIpType_IpV4);
	}

	CIPSpan* pIpSpan = m_pMngmntService->GetFirstSpan();

	pIpSpan->SetIPv6Address(0, strIpV6_global.c_str());
	pIpSpan->SetIPv6SubnetMask(0, iPv6_mask);

	TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv6SpanParams initialize mngmt IPV6 service"
							   << " IpV6 Full: " << strIpV6_global
							   << " IpV6 mask: " << iPv6_mask
							   << "IP type: "<< m_pMngmntService->GetIpType()
							   << " nic_name: "  << nic_name;
    
	return STATUS_OK;
}


std::string CMngntMfw::GetLogicalInterfaceName(const eConfigInterfaceType ifType,const eIpType ipType)
{
	std::string interface_name;
    CIPSpan *pSpan0 = m_pMngmntService->GetSpanByIdx(0);
    interface_name = pSpan0->GetInterface();

    TRACESTR(eLevelInfoNormal) << "CMngntMfw::GetLogicalInterfaceName interface:" << interface_name;
    return interface_name;
}

STATUS CMngntMfw::ValidateInterfaceIpv4(IP_ADDR_S& ipAddrS)
{
	// no need to to validate since we set the service from the interface
	return MANGMENT_STATUS_INTERFACE_READY;
}

STATUS CMngntMfw::ValidateInterfaceIpv6(IP_ADDR_S& ipAddrS)
{
	// no need to to validate since we set the service from the interface
	return MANGMENT_STATUS_INTERFACE_READY;
}

//////////////////////////////////////////////////////////////////////////////////////////
CMngntEdgeAxis::CMngntEdgeAxis() {
	// TODO Auto-generated constructor stub

}

CMngntEdgeAxis::~CMngntEdgeAxis() {
	// TODO Auto-generated destructor stub
}


STATUS CMngntEdgeAxis::SetIPv4SpanParams()
{
	TRACESTR(eLevelInfoNormal) << "CMngntEdgeAxis::SetIPv4SpanParams in";
    
	STATUS stat = STATUS_OK;
	DWORD  dword_ipAddress = 0;
    string first_interface = "";
	string ip_address = "";

    //TODO::Allow to set management to any interface.
    //Edge, Management, first available interface
    SystemPipedCommand("echo -n `/sbin/ifconfig |grep -A0 'Link encap' | cut -d' ' -f1|grep -v lo | head -1`", first_interface);

    string cmd = "echo -n `/sbin/ifconfig | grep -A1 " + first_interface + " | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1 | awk 'NR==1'`";
    SystemPipedCommand(cmd.c_str(), ip_address);

	if (strstr(ip_address.c_str(),INVALID_SERVER_ADDR_STR) || ip_address == "")
	{
        TRACESTR(eLevelInfoNormal) << "CMngntEdgeAxis::SetIPv4SpanParams no ipv4 interface wait.";
		return MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV4;
	}
	dword_ipAddress = SystemIpStringToDWORD(ip_address.c_str()); 
    
	string ipv4_defGw;
	SystemPipedCommand("echo -n `/sbin/ip route show | grep default | cut -d' ' -f3`", ipv4_defGw);
	DWORD dword_defaultGateway = SystemIpStringToDWORD(ipv4_defGw.c_str());
	m_pMngmntService->SetDefaultGatewayIPv4(dword_defaultGateway);

	//Read net mask from first interface
	string net_mask;
    cmd = "echo -n `/sbin/ifconfig " + first_interface + " | grep 'inet addr' | cut -d':' -f4 | cut -d' ' -f1`";
    SystemPipedCommand(cmd.c_str(), net_mask);
	DWORD dword_netMask = SystemIpStringToDWORD(net_mask.c_str());
	m_pMngmntService->SetNetMask(dword_netMask);

	CIPSpan *pSpan0 = m_pMngmntService->GetSpanByIdx(0);
	if(pSpan0)
	{
        pSpan0->SetIPv4Address(dword_ipAddress);
	    pSpan0->SetInterface(first_interface);
        //m_pMngmntService->SetIpType(eIpType_Both);
	}

	TRACESTR(eLevelInfoNormal) << "CMngntEdgeAxis::SetIPv4SpanParams initialize mngmt IPV4 service"
							   << " default gw: "     << ipv4_defGw
							   << " net mask: "       << net_mask
							   << " ip_ethname: "     << first_interface
							   << " dword_ipAddress:" << dword_ipAddress;

    return stat;
}

/*STATUS CMngntEdgeAxis::SetIPv6SpanParams()
{
    TRACESTR(eLevelInfoNormal) << "CMngntEdgeAxis::SetIPv6SpanParams in.";
    
	STATUS stat = STATUS_OK;
	eV6ConfigurationType curIpV6configType	= m_pMngmntService->GetIpV6ConfigurationType();
	if(eV6Configuration_Manual == curIpV6configType )
	{
        m_pMngmntService->SetIpV6ConfigurationType(eV6Configuration_Auto);
	}

	string retStr_ipV6,nic_name;
	MngmentIpv6Array autoIpV6;
	GetMngmntIpv6(autoIpV6, retStr_ipV6, nic_name, m_pMngmntService->GetIpType());
	TRACEINTO << "CMngntEdgeAxis::SetIPv6SpanParams IpV6 for " << nic_name << " : " << retStr_ipV6;

	if(autoIpV6[0].address[0] == '\0')
	{
		stat = MANGMENT_STATUS_INTERFACE_WAITING_FOR_IPV6;
        TRACESTR(eLevelInfoNormal) << "CMngntEdgeAxis::SetIPv6SpanParams not found ipv6 global address.";
	}
	else
	{
		UpdateMngntServiceIPv6Auto(autoIpV6);
        if (eIpType_IpV6 == m_pMngmntService->GetIpType())
	    {
			CIPSpan* pCntrlSpan = m_pMngmntService->GetSpanByIdx(0);
			if(pCntrlSpan)
			{
				pCntrlSpan->SetInterface(nic_name);
			}
	    }
	}
    
	TRACESTR(eLevelInfoNormal) << "CMngntEdgeAxis::SetIPv6SpanParams initialize mngmt IPV6 service"
							   << " IpV6 Full: " << autoIpV6[0].address
							   << " IpV6 mask: " << autoIpV6[0].mask
							   << " nic_name: "  << nic_name;
	return stat;
}*/


std::string CMngntEdgeAxis::GetLogicalInterfaceName(const eConfigInterfaceType ifType,const eIpType ipType)
{
	std::string interface_name;
    CIPSpan *pSpan0 = m_pMngmntService->GetSpanByIdx(0);
    interface_name = pSpan0->GetInterface();

    TRACESTR(eLevelInfoNormal) << "CMngntEdgeAxis::GetLogicalInterfaceName interface:" << interface_name;
    return interface_name;
}

STATUS CMngntEdgeAxis::ValidateInterfaceIpv4(IP_ADDR_S& ipAddrS)
{
	// no need to to validate since we set the service from the interface
	return MANGMENT_STATUS_INTERFACE_READY;
}

STATUS    CMngntEdgeAxis::ValidateInterfaceIpv6(IP_ADDR_S& ipAddrS)
{
	if(m_Ipv6_status == MANGMENT_STATUS_INTERFACE_WAITING_FOR_IPV6)
	{
		return CManagmentNetwork::ValidateInterfaceIpv6(ipAddrS);
	}
	else
		return MANGMENT_STATUS_INTERFACE_READY;
}
//======================================================================================

CMngntSoftCallGenerator::CMngntSoftCallGenerator() {
	// TODO Auto-generated constructor stub

}

CMngntSoftCallGenerator::~CMngntSoftCallGenerator() {
	// TODO Auto-generated destructor stub
}

} /* namespace McmsNetworkPackage */
