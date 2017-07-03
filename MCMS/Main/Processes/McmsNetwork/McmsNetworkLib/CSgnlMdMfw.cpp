/*
 * CSgnlMdMfw.cpp
 *
 *  Created on: May 16, 2014
 *      Author: penrod
 */


#include "SystemInterface.h"
#include "IpService.h"
#include "NetCommonDefines.h"
#include "Trace.h"
#include "TraceStream.h"
#include "OsFileIF.h"
#include "CSgnlMdMfw.h"

namespace McmsNetworkPackage
{

CSgnlMdMfw::CSgnlMdMfw()
{
	// TODO Auto-generated constructor stub

}

CSgnlMdMfw::~CSgnlMdMfw()
{
	// TODO Auto-generated destructor stub
}

STATUS CSgnlMdMfw::CheckStatusWithSystmSetting(CIPSpan* pIpSpan, STATUS status)
{
	TRACEINTO << "CSgnlMdMfw::CheckStatusWithSystmSetting";
	STATUS nRet = status;
	CSystemInterface* pInterface = m_systemInterfaceList.GetSystemInterfaceByName(pIpSpan->GetInterface());
    PASSERT_AND_RETURN_VALUE(NULL == pInterface, nRet);
	switch(nRet)
	{
	case STATUS_IP_ADDRESS_MISMATCHES_SYSTEM:
		if(pInterface->GetSystemInterfaceIp() == 0)
		{
			TRACEINTO << "CSgnlMdMfw::CheckStatusWithSystmSetting - system didn't retrieve signal/media IPv4";
			nRet = SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV4;
		}
		break;
	case STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM:
		if(pInterface->GetSystemInterfaceIpv6_global().empty())
		{
			TRACEINTO << "CSgnlMdMfw::CheckStatusWithSystmSetting - system didn't retrieve signal/media IPv6 global";
			nRet = SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV6;
		}
		break;
	case STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM:
		if(pInterface->GetSystemInterfaceIpv6_site().empty())
		{
			TRACEINTO << "CSgnlMdMfw::CheckStatusWithSystmSetting - system didn't retrieve signal/media IPv6 site";
			nRet = SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV6;
		}
		break;
	case STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM:
		if(pInterface->GetSystemInterfaceIpv6_link().empty())
		{
			TRACEINTO << "CSgnlMdMfw::CheckStatusWithSystmSetting - system didn't retrieve signal/media IPv6 link";
			nRet = SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV6;
		}
		break;
	}
	return nRet;

}

STATUS CSgnlMdMfw::CheckPerIPSpan(string strServiceName, CIPSpan* pIpSpan, eIpType  ipType)
{
	STATUS nRet = m_systemInterfaceList.ValidateMFWSPANField(pIpSpan, ipType);


	string start_alarm = "The configuration of the " + strServiceName +" service is invalid. Interface " + pIpSpan->GetInterface();
	switch (nRet)
	{
	case STATUS_IP_ADDRESS_MISMATCHES_SYSTEM:
	{
		start_alarm += " with address IpV4:" + m_systemInterfaceList.GetValidatedResult() + " does not exist or is invalid.";
        TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::CheckPerIPSpan failed d_mngIpV4:" << m_systemInterfaceList.GetValidatedResult();
	}
		break;
	case STATUS_MUST_ASSIGN_INTERFACE_IN_MFW:
	{
		start_alarm += " does not exist or is invalid.";
		TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::CheckPerIPSpan interface "<< pIpSpan->GetInterface()<< " does bit exist or is invalid.";
	}
		break;
	case STATUS_IPV6_GLOBAL_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_SITE_ADDRESS_MISMATCHES_SYSTEM:
	case STATUS_IPV6_LINK_ADDRESS_MISMATCHES_SYSTEM:
	{
		start_alarm += " with address IpV6:" + m_systemInterfaceList.GetValidatedResult() + " does not exist or is invalid.";
		TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::CheckPerIPSpan IpV6 check failed mngSpan_IpV6Full:" <<m_systemInterfaceList.GetValidatedResult();
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

STATUS CSgnlMdMfw::OnPreConfigNetworkSignalMedia()
{
	STATUS stat = STATUS_OK;

	CIPService* pIpService = m_pSignalMediaServiceList->GetFirstService();
	if (NULL == pIpService)
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::OnPreConfigNetworkSignalMedia NULL == pIpService";
		return STATUS_FAIL;
	}
	eIpType ipType = pIpService->GetIpType();
	//cs IpSpan
	CIPSpan *pIpSpan = pIpService->GetFirstSpan();
	if(NULL == pIpSpan)
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::OnPreConfigNetworkSignalMedia pIpSpan==NULL.";
		return STATUS_FAIL;
	}
	stat = CheckPerIPSpan("Signal", pIpSpan, ipType);

	if(STATUS_OK != stat)
		return stat;

	//media IpSpan
	pIpSpan = pIpService->GetNextSpan();
	if(NULL == pIpSpan)
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::OnPreConfigNetworkSignalMedia pIpSpan==NULL.";
		return STATUS_FAIL;
	}
	stat = CheckPerIPSpan("Media", pIpSpan, ipType);

	return stat;
}

STATUS CSgnlMdMfw::TryToRecover(STATUS eSelfLastStatus, STATUS eOtherLastStatus, CBaseNetworkPlatform *pOtherNetwork)
{
	STATUS stat = STATUS_OK;
	TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::TryToRecover in.";

	string ipServiceFileNameTmp;
	ipServiceFileNameTmp = IP_SERVICE_LIST_TMP_PATH;

	if (!IsFileExists(IP_SERVICE_LIST_TMP_PATH))
	{
		CopyFile(IP_VERSION_CFG_SOFT_MCU_SERVICES_LIST_PATH, IP_SERVICE_LIST_TMP_PATH);
		TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::TryToRecover IPServiceListTmp.XML not exsit, create.";
	}

	CIPServiceList stIpServListTmp;
	stat = stIpServListTmp.ReadXmlFile(ipServiceFileNameTmp.c_str(), eNoActiveAlarm, eRenameFile);
	if (STATUS_OK != stat)
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::TryToRecover ReadXmlFile error.";
		return STATUS_FAIL;
	}

	CIPService* pIpService = stIpServListTmp.GetFirstService();
	if (NULL == pIpService)
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::TryToRecover NULL==pIpService";
		return STATUS_FAIL;
	}

	DWORD  d_IpV4Word = 0;
	string nic_name = "";
	string strIpV6Full = "";

	//1. get the first available interface, follow is the criteria:
	//   a. The interface should be UP and RUNNING
	//   b. The interface should have a valid IP address (IPV4 preffered, IPV6 as second priority)
	//   c. The interface should be an Ethernet interface, unless no such interface exists
	//   d. The interface should not be the loopback (lo) interface
	CIPService stCIPService;
	stCIPService.GetFirstAvailableIpAddressAndInterface(d_IpV4Word, nic_name, strIpV6Full);


	//write cs and media interface and ip to the first available interface.
	eIpType  ipType    = eIpType_Both;
	if(0 != d_IpV4Word && !strIpV6Full.empty())
		ipType = eIpType_Both;
	else if( 0 != d_IpV4Word )
		ipType = eIpType_IpV4;
	else if ( !strIpV6Full.empty() )
		ipType = eIpType_IpV6;



	DWORD d_IpV6Mask   = DEFAULT_IPV6_SUBNET_MASK;
	if ("" != strIpV6Full)
	{
		string::size_type position = 0;
		string iPv6_mask_str = "";
		position = strIpV6Full.find("/");
		if (position != strIpV6Full.npos)
		{
			iPv6_mask_str = strIpV6Full.substr(position+1, strIpV6Full.size()-position);
			sscanf(iPv6_mask_str.c_str(), "%d", &d_IpV6Mask);
		}
		TRACESTR(eLevelInfoNormal) << "CMngntMfw::SetIPv6SpanParams iPv6_mask:" << d_IpV6Mask;
	}


	pIpService->SetIpType(ipType);
	pIpService->SetIPProtocolType(eIPProtocolType_SIP);

	//cs span
	CIPSpan* pSpanCs = pIpService->GetFirstSpan();

	pSpanCs->SetInterface(nic_name);
	pSpanCs->SetIPv4Address(d_IpV4Word);
	pSpanCs->SetIPv6Address(0, strIpV6Full.c_str());
	pSpanCs->SetIPv6SubnetMask(0, d_IpV6Mask);

	CCommH323PortRange* portRange0 = pSpanCs->GetPortRange();
	portRange0->SetDefaultTcpPorts(1000);
	portRange0->SetUdpPortRange(40000,10000);

	//media span
	CIPSpan* pSpanMd = pIpService->GetNextSpan();
    PASSERT_AND_RETURN_VALUE(NULL == pSpanMd, STATUS_FAIL);
	pSpanMd->SetInterface(nic_name);
	pSpanMd->SetIPv4Address(d_IpV4Word);
	pSpanMd->SetIPv6Address(0, strIpV6Full.c_str());
	pSpanMd->SetIPv6SubnetMask(0, d_IpV6Mask);

	CCommH323PortRange* portRange1 = pSpanMd->GetPortRange();
	portRange1->SetDefaultTcpPorts(1000);
	portRange1->SetUdpPortRange(40000,10000);

	// change default to tcp VNGSWIBM-1665
	CSip *pSip = pIpService->GetpSip();
	pSip->SetTransportType(eTransportTypeTcp);

	stIpServListTmp.WriteXmlFile(ipServiceFileNameTmp.c_str());
	CopyFile(IP_SERVICE_LIST_TMP_PATH, IP_SERVICE_LIST_PATH);

	TRACESTR(eLevelInfoNormal) << "CSgnlMdMfw::TryToRecover initialize IPService interface"
							   << " nic_name: "   << nic_name
							   << " Ipv4Address: "<< d_IpV4Word
							   << " Ipv6Full: "   << strIpV6Full
							   << " IpV6 mask: "  << d_IpV6Mask;

	return stat;
}








} /* namespace McmsNetworkPackage */

