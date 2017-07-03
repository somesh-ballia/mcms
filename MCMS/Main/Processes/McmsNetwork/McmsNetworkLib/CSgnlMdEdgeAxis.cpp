/*
 * CSgnlMdEdgeAxis.cpp
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
#include "CSgnlMdEdgeAxis.h"

namespace McmsNetworkPackage
{

CSgnlMdEdgeAxis::CSgnlMdEdgeAxis()
{
	// TODO Auto-generated constructor stub

}

CSgnlMdEdgeAxis::~CSgnlMdEdgeAxis()
{
	// TODO Auto-generated destructor stub
}

STATUS CSgnlMdEdgeAxis::TryToRecover(STATUS eSelfLastStatus, STATUS eOtherLastStatus, CBaseNetworkPlatform *pOtherNetwork)
{
    TRACESTR(eLevelInfoNormal) << "CSgnlMdEdgeAxis::TryToRecover in.";
    return OnInitInterfaces();
}

STATUS CSgnlMdEdgeAxis::OnInitInterfaces()
{
	STATUS stat = STATUS_OK;
    STATUS default_file = 0;
	string ipServiceFileNameTmp;
	ipServiceFileNameTmp = IP_SERVICE_LIST_TMP_PATH;

	if (!IsFileExists(IP_SERVICE_LIST_TMP_PATH))
	{
        default_file = 1;
		CopyFile(IP_VERSION_CFG_SOFT_MCU_SERVICES_LIST_PATH, IP_SERVICE_LIST_TMP_PATH);
		TRACESTR(eLevelInfoNormal) << "CSgnlMdEdgeAxis::OnInitInterfaces IPServiceListTmp.XML not exsit, create.";
	}

	CIPServiceList stIpServListTmp;
	stat = stIpServListTmp.ReadXmlFile(ipServiceFileNameTmp.c_str(), eNoActiveAlarm, eRenameFile);
	if (STATUS_OK != stat)
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdEdgeAxis::OnInitInterfaces ReadXmlFile error.";
		return STATUS_FAIL;
	}

	CIPService* pIpService = stIpServListTmp.GetFirstService();
	if (NULL == pIpService)
	{
		TRACESTR(eLevelInfoNormal) << "CSgnlMdEdgeAxis::TryToRecover NULL==pIpService";
		return STATUS_FAIL;
	}

    //set cs ipv4&ipv6 address
    string  nic_name = "";
    eIpType	ipType   = pIpService->GetIpType();
    CIPSpan *pSpanCs = pIpService->GetFirstSpan();
    
    nic_name = pSpanCs->GetInterface();
    if ("" == nic_name)
    {
        nic_name = "eth0";
        TRACESTR(eLevelInfoNormal) << "CSgnlMdEdgeAxis::OnInitInterfaces, first startup, default use eth0.";
    }

    DWORD  d_IpV4Word  = 0;
	string strIpV6Full = "";
    string strIPV6     = "";
	DWORD d_IpV6Mask   = DEFAULT_IPV6_SUBNET_MASK;

    //get ipv4&ipv6 address
    string ip_address;
    string cmd;
    cmd = "echo -n `/sbin/ifconfig | grep -A1 " + nic_name + " | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1 | awk 'NR==1'`";
    SystemPipedCommand(cmd.c_str(), ip_address);
    d_IpV4Word = SystemIpStringToDWORD(ip_address.c_str());
	cmd = "echo -n `/sbin/ifconfig " + nic_name + " | grep 'Scope:Global' | cut -d':' -f2- | cut -d' ' -f2`";
	SystemPipedCommand(cmd.c_str(), strIpV6Full);

    //get ipv6 mask
	if ("" != strIpV6Full)
	{
		string::size_type position = 0;
		string iPv6_mask_str = "";
		position = strIpV6Full.find("/");
		if (position != strIpV6Full.npos)
		{
			iPv6_mask_str = strIpV6Full.substr(position+1, strIpV6Full.size()-position);
			sscanf(iPv6_mask_str.c_str(), "%d", &d_IpV6Mask);
            strIPV6 = strIpV6Full.substr(0, position);
		}
		TRACESTR(eLevelInfoNormal) << "CSgnlMdEdgeAxis::OnInitInterfaces iPv6_mask:" << d_IpV6Mask << " ipv6:" << strIPV6;
	}

	ipType = eIpType_Both;
	if(0 != d_IpV4Word && !strIPV6.empty())
		ipType = eIpType_Both;
	else if( 0 != d_IpV4Word )
		ipType = eIpType_IpV4;
	else if ( !strIpV6Full.empty() )
		ipType = eIpType_IpV6;
	pIpService->SetIpType(ipType);

    //cs span
	pSpanCs->SetInterface(nic_name);
	pSpanCs->SetIPv4Address(d_IpV4Word);
	pSpanCs->SetIPv6Address(0, strIPV6.c_str());
	pSpanCs->SetIPv6SubnetMask(0, d_IpV6Mask);

	//media span
	CIPSpan* pSpanMd = pIpService->GetNextSpan();
    PASSERT_AND_RETURN_VALUE(NULL == pSpanMd, STATUS_FAIL);
	pSpanMd->SetInterface(nic_name);
	pSpanMd->SetIPv4Address(d_IpV4Word);
	pSpanMd->SetIPv6Address(0, strIPV6.c_str());
	pSpanMd->SetIPv6SubnetMask(0, d_IpV6Mask);

    if (1 == default_file)
    {
        CCommH323PortRange* portRange0 = pSpanCs->GetPortRange();
        portRange0->SetDefaultTcpPorts(80);
        CCommH323PortRange* portRange1 = pSpanMd->GetPortRange();
        portRange1->SetDefaultTcpPorts(80);
    }

    stIpServListTmp.UpdateCounters();
	stIpServListTmp.WriteXmlFile(ipServiceFileNameTmp.c_str());
	CopyFile(IP_SERVICE_LIST_TMP_PATH, IP_SERVICE_LIST_PATH);
   
	TRACESTR(eLevelInfoNormal) << "CSgnlMdEdgeAxis::OnInitInterfaces set IPService interface"
							   << " nic_name: "   << nic_name
							   << " Ipv4Address: "<< d_IpV4Word
							   << " Ipv6: "       << strIPV6
							   << " IpV6 mask: "  << d_IpV6Mask;
    return stat;
}


} /* namespace McmsNetworkPackage */
