/*
 * CManagmentNetwork.cpp
 *
 *  Created on: Jul 28, 2013
 *      Author: stanny
 */

#include "CManagmentNetwork.h"
#include "IpServiceValidator.h"
#include "Trace.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include <algorithm>
#include "CNetworkFactory.h"
#include "SysConfigKeys.h"

extern const char* PortSpeedTypeToString(ePortSpeedType type);

namespace McmsNetworkPackage {

CManagmentNetwork::CManagmentNetwork() {
	// TODO Auto-generated constructor stub
	m_pMngmntService = NULL;
	m_netConfigStatus = eNetConfigurationSuccess;
}

CManagmentNetwork::~CManagmentNetwork() {

	if(m_pMngmntService)
		delete m_pMngmntService;
}


STATUS  CManagmentNetwork::ConfigNetworkMngmnt(const CIPService* pMngmntService)
{
	STATUS status = STATUS_OK;

	TRACESTR(eLevelInfoNormal) << "CManagmentNetwork::ConfigNetworkMngmnt";

	TRACESTR(eLevelDebug) << "check for NULL pointer of pMngmntService";
	if(NULL ==pMngmntService)
		return PLATFORM_STATUS_MNGMNT_IS_NULL;

	m_pMngmntService = new CIPService(*pMngmntService);

	status =OnInitInterfaces();
	if(ValidateInitInterfacesStatus(status) != STATUS_OK)
		return  status;
	if(STATUS_OK != (status = OnPreConfigNetworkMngmnt()))
	{
		TRACESTR(eLevelInfoNormal) << "Return from OnPreConfigNetworkMngmnt , reason status : " << GetStatusAsString(status);
		return status;
	}
	if(STATUS_OK != (status = OnConfigNetworkMngmnt()))
	{
		TRACESTR(eLevelInfoNormal) << "Return from OnConfigNetworkMngmnt , reason status : " << GetStatusAsString(status);
		return status;
	}
	if(STATUS_OK != (status = OnPostConfigNetworkMngmnt()))
	{
		TRACESTR(eLevelInfoNormal) << "Return from OnPostConfigNetworkMngmnt , reason status : " <<  GetStatusAsString(status);
		return status;
	}
	TRACESTR(eLevelDebug) << "ConfigNetworkMngmnt finish flow return to caller";
	return status;
}

STATUS    CManagmentNetwork::OnInitInterfaces()
{
	TRACESTR(eLevelInfoNormal) << "CManagmentNetwork::OnInitInterfaces";
	STATUS status = STATUS_OK;

	status = ConfigureEthernetSettingsCpu(eInternalNetwork, ePortSpeed_100_FullDuplex);

    TRACESTR(eLevelInfoNormal) << "\n CManagmentNetwork::ConfigureEthernetSettingsCpu"
    								<< "\nInterface num: " << GetDeviceName(eInternalNetwork)
    								<< "\nSpeed:         " << ::PortSpeedTypeToString(ePortSpeed_100_FullDuplex)
    								<< "Status: " 		   << GetStatusAsString(status) ;
	return status;
}
STATUS     CManagmentNetwork::OnPreConfigNetworkMngmnt()
{
	TRACESTR(eLevelInfoNormal) << "OnPreConfigNetworkMngmnt - validate service and check for dhcp server";
	STATUS status = STATUS_OK;

	if(STATUS_OK != (status = ValidateMngmntService()))
	{
		return status;
	}
	WORD isDhcpEnabled = m_pMngmntService->GetDHCPServer();
	if(isDhcpEnabled)
			return PLATFORM_STATUS_OK_DHCP_IS_ENABLED;

	TRACESTR(eLevelDebug) << "OnPreConfigNetworkMngmnt - DHCP is not configure kill dhcp client";

	eIpType ipType = m_pMngmntService->GetIpType();
	STATUS killDhcpStatus = KillDHCP(ipType);
	if(STATUS_OK !=  killDhcpStatus)
	{
		TRACESTR(eLevelWarn) << "Failed to kill dhcp client reason : " << GetStatusAsString(killDhcpStatus);
	}
	//Lync FSN-565  lync environment to work with ipv6 needs to be configured using dhcp
	eV6ConfigurationType ipv6Mode = m_pMngmntService->GetIpV6ConfigurationType();
	if(eV6Configuration_Auto == ipv6Mode)
	{
		OnConfigDHCP(ipType);
	}



	return status;
}




STATUS 	   CManagmentNetwork::OnConfigNetworkMngmnt()
{
	TRACESTR(eLevelInfoNormal) << "OnConfigNetworkMngmnt - start to config Mngnt service";

	STATUS status = STATUS_OK;
	eConfigInterfaceType ifType = GetMngrIfType();
	std::list<CRouter>  routerList;

	STATUS configStatusIPv4 = STATUS_OK,configStatusIPv6 = STATUS_OK;
	// ===== 2. get data
	char  iPv6defGWMaskStr[IPV6_ADDRESS_LEN];
	memset(iPv6defGWMaskStr, 0, IPV6_ADDRESS_LEN);
	// (Configurator uses addresses without brackets)
	m_pMngmntService->GetDefaultGatewayMaskIPv6Str(iPv6defGWMaskStr);

	char iPv4defGWStr[IP_ADDRESS_LEN],
         broadcastIPv4Str[IP_ADDRESS_LEN];

	SystemDWORDToIpString(m_pMngmntService->GetDefaultGatewayIPv4(), iPv4defGWStr);
	strcpy(broadcastIPv4Str, iPv4defGWStr);

    char * tmp = strrchr (broadcastIPv4Str, '.');
    strcpy (tmp+1 , "255");
	RetrieveRouterList(m_pMngmntService, routerList);
	// ===== 3. config new IpInterfaceConfig
	eIpType					ipType			= m_pMngmntService->GetIpType();
	eV6ConfigurationType	ipV6configType	= m_pMngmntService->GetIpV6ConfigurationType();

	TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - call any preActions that need to be done before start configuring interfaces";
	if(STATUS_OK == (status =OnPreConfigIpInterface()))
	{
		TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - check if bonding should be done";
		if(IsSystemFlagExist(flagLanRedunancy) &&( STATUS_OK ==(status =OnPreBondingInterface())))
		{
			TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - check for jitc mode";
			SystemStateOptions state =flagJitcMode; //Bonding is not allowed in Jitc mode
			if(!IsSystemFlagExist(state))
				SetBondingInterface();
		}

		IP_ADDR_S ipAddrS;
		memset(&ipAddrS,0,sizeof(ipAddrS));
		RetrieveIpAddresses(m_pMngmntService, ipAddrS);
		TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - check iptype for ipv4 or Both ";
		if ( (eIpType_IpV4 == ipType) || (eIpType_Both == ipType) )
			{

			    AddIpInterface( ifType,ipAddrS.ipType,ipAddrS.iPv4_str,ipAddrS.iPv4_netMask,iPv4defGWStr,broadcastIPv4Str,routerList );
			}

		TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - check iptype for ipv6 or Both ";

		if ( (eIpType_IpV6 == ipType) || (eIpType_Both == ipType) )
		{
			if(eV6Configuration_Manual == ipV6configType)
			{
				TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - add ipv6 Manual configuration  ";
				configStatusIPv6 = AddIpV6Interface( ifType,FALSE,ipAddrS.iPv6_NoBrackets,ipAddrS.iPv6_netMask,ipAddrS.iPv6_defGW,iPv6defGWMaskStr );
			}
			else if(eV6Configuration_Auto == ipV6configType)
			{
				TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - add ipv6 Auto configuration  ";

				AddIpV6Interface(ifType, TRUE);
			}
		}

	}
	TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - call any preActions that need to be done before start configuring interface Bonding slaves ";
	if(IsSystemFlagExist(flagLanRedunancy) &&( STATUS_OK ==(status =OnPreBondingInterfaceSlaves())))
	{
		SystemStateOptions state =flagJitcMode; //Bonding is not allowed in Jitc mode
		if(!IsSystemFlagExist(state))
			ConfigBondingInterfaceSlaves();
	}
	TRACESTR(eLevelDebug) << "OnConfigNetworkMngmnt - call any preActions that need to be done before start config network separations  ";
	STATUS statusPreNetSepConfig = STATUS_OK;
	if(STATUS_OK == (statusPreNetSepConfig= OnPreHandleNetworkSeparationConfigurations()))
	{
		HandleNetworkSeparationConfigurations();
	}
	else
		TRACESTR(eLevelInfoNormal) << "OnPreNetworkSeparationConfigurations failed Status: " 		   << GetStatusAsString(statusPreNetSepConfig) ;

	return status;
}

void  CManagmentNetwork::SetBondingInterface()
{
	TRACESTR(eLevelInfoNormal) << "SetBondingInterface - start bonding interfaces";
	std::string bondingMode;
	DWORD linkmonitoringfrequency;
	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
	{
		pSysConfig->GetDataByKey("CPU_BONDING_MODE", bondingMode);
		pSysConfig->GetDWORDDataByKey("CPU_BONDING_LINK_MONITORING_FREQUENCY",linkmonitoringfrequency);
		ConfigBondingInterface(bondingMode,linkmonitoringfrequency);
	}
}

STATUS  CManagmentNetwork::ValidateMngmntService()
{
	TRACESTR(eLevelInfoNormal) << "ValidateMngmntService - check validity of management service";
	STATUS status = STATUS_OK;
	// ===== 2. validate ip address with netMask
    CLargeString errorMsg;
	CIpServiceValidator serviceValidator(*m_pMngmntService);
	status = serviceValidator.ValidateFullMNGMNT(errorMsg);
	return status;
}
//
STATUS    CManagmentNetwork::OnPreConfigIpInterface()
{
	TRACESTR(eLevelInfoNormal) << "OnPreConfigIpvInterface - do any common preActions before config interfaces";
	return STATUS_OK;
}

STATUS 	   CManagmentNetwork::OnPreBondingInterface()
{
	TRACESTR(eLevelInfoNormal) << "OnPreBondingInterface - do any common preActions before do bonding";
	return STATUS_OK;
}

STATUS 	  CManagmentNetwork::OnPreBondingInterfaceSlaves()
{
	return STATUS_OK;
}
//
// Get All V4, V6 ip addresses and their relevant information from service,
// and store them in the struct
//
void  CManagmentNetwork::RetrieveIpAddresses(CIPService* pService, IP_ADDR_S & ipAddrS)
{
	if(!pService)
		{
			return;
		}
		CIPSpan* pTmpSpan0 = pService->GetSpanByIdx(0); // Ctrl
		CIPSpan* pTmpSpan1 = pService->GetSpanByIdx(1); // Shelf
		if(pTmpSpan0)
		{
			ipAddrS.iPv4 = pTmpSpan0->GetIPv4Address();
			pTmpSpan0->GetIPv6Address(0, ipAddrS.iPv6_str);
			pTmpSpan0->GetIPv6Address(0, ipAddrS.iPv6_NoBrackets, FALSE);
			pTmpSpan0->GetIPv6SubnetMaskStr(0, ipAddrS.iPv6_netMask);
		}
		SystemDWORDToIpString(ipAddrS.iPv4,	ipAddrS.iPv4_str);

		if(pTmpSpan1)
		{
			ipAddrS.iPv4_shelf = pTmpSpan1->GetIPv4Address();
			pTmpSpan1->GetIPv6Address(0, ipAddrS.iPv6_shelf);
		}

		ipAddrS.iPv4_defGW = pService->GetDefaultGatewayIPv4();
		SystemDWORDToIpString(pService->GetNetMask(), ipAddrS.iPv4_netMask);

		pService->GetDefaultGatewayIPv6(ipAddrS.iPv6_defGW, FALSE);
		ipAddrS.ipType = pService->GetIpType();

		// eV6Configuration_DhcpV6 is not supported!
		//   if it's retrieved, replace it with 'auto'
		if (eV6Configuration_DhcpV6 == pService->GetIpV6ConfigurationType())
		{
			PASSERTMSG(TRUE, "DhcpV6 is not supported as IPv6 configuration type");
			pService->SetIpV6ConfigurationType(eV6Configuration_Auto);
		}
		ipAddrS.ipV6configType = pService->GetIpV6ConfigurationType();
}

STATUS    CManagmentNetwork::HandleNetworkSeparationConfigurations()
{
	STATUS retStatus = STATUS_FAIL;
	eConfigInterfaceType ifType = GetMngrIfType();

	TRACESTR(eLevelInfoNormal) << "CManagmentNetwork::HandleNetworkSeparationConfigurations ifType = :" << ifType;
	CConfigManagerApi api;
	char ipStr[IP_ADDRESS_LEN]="";
	char defaultGWIPv4Str[IP_ADDRESS_LEN]="";
	char defaultGWIPv6Str[IPV6_ADDRESS_LEN] = "";

	CIPSpan* pTmpSpan = m_pMngmntService->GetSpanByIdx(0); // Mngmnt params are stored in the 1st span (idx==0)
	if(pTmpSpan)
	{
		char ipv6SubNetMask[IPV6_ADDRESS_LEN];
		pTmpSpan->GetIPv6SubnetMaskStr(0, ipv6SubNetMask);

		DWORD ipAddress   = pTmpSpan->GetIPv4Address();
		SystemDWORDToIpString(ipAddress,ipStr);
		DWORD dwDefaultGw = m_pMngmntService->GetDefaultGatewayIPv4();
		SystemDWORDToIpString(dwDefaultGw,defaultGWIPv4Str);
		DWORD mask = m_pMngmntService->GetNetMask();

		std::list<CRouter> routerList;
		RetrieveRouterList(m_pMngmntService, routerList);

		eIpType ipType			= m_pMngmntService->GetIpType();
		//Cleanup
		retStatus = DelRouteTableRule(ifType,ipType, ipStr, mask, defaultGWIPv4Str);
		//Add
		retStatus = AddRouteTableRule(ifType,ipType,ipStr, mask, defaultGWIPv4Str,ipv6SubNetMask, routerList);
	}
	else
		PASSERTMSG(1, "CManagmentNetwork::HandleNetworkSeparationConfigurations, span not found.");

	return retStatus;
}

// check if Management interface is up and correctly configured
STATUS  CManagmentNetwork::IsMngmntInterfaceReady(eIpType ipType)
{

	TRACESTR(eLevelDebug) << "IsMngmntInterfaceReady - check for NULL pointer of pMngmntService";
	if(NULL ==m_pMngmntService)
		return PLATFORM_STATUS_MNGMNT_IS_NULL;
	STATUS stat = MANGMENT_STATUS_INTERFACE_READY;
	IP_ADDR_S ipAddrS;
	memset(&ipAddrS,0,sizeof(ipAddrS));
	RetrieveIpAddresses(m_pMngmntService, ipAddrS);

	 if (eIpType_IpV4 == ipType || eIpType_Both == ipType)
		stat = ValidateInterfaceIpv4(ipAddrS);

	if(MANGMENT_STATUS_INTERFACE_READY != stat)
			return stat;
	 if (eIpType_IpV6 == ipType || eIpType_Both == ipType)
		 stat = ValidateInterfaceIpv6(ipAddrS);


	return stat;
}

void     CManagmentNetwork::GetMngmntIpv6(MngmentIpv6Array& autoIpV6,string& retStr_ipV6,string& nic_name,eIpType curIpType)
{
	for(int i = 0 ; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
	{
		memset(autoIpV6[i].address, '\0', IPV6_ADDRESS_LEN);
		autoIpV6[i].mask = DEFAULT_IPV6_SUBNET_MASK;
	}
	 eConfigInterfaceType ifType = GetMngrIfType();
	 nic_name = GetLogicalInterfaceName(ifType,curIpType);
	 RetrieveIpAddressConfigured_IpV6(autoIpV6, retStr_ipV6, nic_name);

}

STATUS    CManagmentNetwork::ValidateInterfaceIpv6(IP_ADDR_S& ipAddrS)
{

	if((eIpType_IpV6 == ipAddrS.ipType) || (eIpType_Both ==ipAddrS.ipType) )
	{

		string retStr_ipV6,nic_name,cmd,answer;
		MngmentIpv6Array autoIpV6;
		GetMngmntIpv6(autoIpV6,retStr_ipV6,nic_name,eIpType_IpV6);
		TRACEINTO << "\nCManagmentNetwork::ValidateInterfaceIpv6\n IpV6 for " << nic_name << " : " << retStr_ipV6;


		if(eV6Configuration_Auto != ipAddrS.ipV6configType) //manual or dhcp6
		{
			//ip -6 route show | grep '^default via ' | grep eth0 | awk '{print $3}'
			cmd = GetCmdLinePrefix() + "ip -6 route show | grep '^default via ' | grep " + nic_name + " | awk '{print $3}'";
			STATUS stat = SystemPipedCommand(cmd.c_str() ,answer);
			if (answer=="") //no default gateway for ipv6 please add it
			{
				cmd = GetCmdLinePrefix() + "/sbin/route -A inet6 add default gw " + ipAddrS.iPv6_defGW + " dev " + nic_name;
				stat = SystemPipedCommand(cmd.c_str(), answer);
			}
		}

		if(autoIpV6[0].address[0] == '\0')
		{
			if(eV6Configuration_Manual == ipAddrS.ipV6configType)
				return  MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV6;
			else
				return MANGMENT_STATUS_INTERFACE_WAITING_FOR_IPV6;
		}
		else
		{
			// for auto configuration update the service with the ipv6
			//for manual configuration check that ipv6 that config matches the service
			if(eV6Configuration_Auto == ipAddrS.ipV6configType)
			{
				TRACESTR(eLevelDebug) << "ValidateInterfaceIpv6 -  auto configuration update the service  with ipv6";
				UpdateMngntServiceIPv6Auto(autoIpV6);
			}
			else
			{
				TRACESTR(eLevelDebug) << "ValidateInterfaceIpv6 - manual configuration  -compare ipv6";
				if(strcmp(ipAddrS.iPv6_NoBrackets,autoIpV6[0].address)!=0)
					return MANGMENT_STATUS_INTERFACE_ERROR_CONFILCT_IPV6;

			}
		}
	}
	return MANGMENT_STATUS_INTERFACE_READY;
}
/////////////////////////////////////////////////////////////////////////////
std::string CManagmentNetwork::GetDefGwInAutoMode()
{
	string sDefGw;
	SystemPipedCommand("/sbin/route -n -A inet6 | grep G | grep U | awk '{ print $2 }'|sort -u", sDefGw);

	sDefGw.erase(std::remove_if(sDefGw.begin(), sDefGw.end(),
														&::isspace), sDefGw.end());
	TRACESTR(eLevelInfoNormal) << "\nCMcuMngrManager::GetDefGwInAutoMode"
						   << "\nDefGW: " << sDefGw.c_str();
	return sDefGw;
}



void CManagmentNetwork::UpdateMngntServiceIPv6Auto(IpV6AddressMaskS autoIpV6[])
{
	TRACEINTO << "\nCManagmentNetwork::UpdateMngntServiceIPv6Auto";

	// ===== update params
	std::string defGwIpv6 = GetDefGwInAutoMode();
	m_pMngmntService->SetDefaultGatewayIPv6(defGwIpv6.c_str());
	m_pMngmntService->SetDefaultGatewayMaskIPv6(DEFAULT_IPV6_SUBNET_MASK);

	CIPSpan* pCntrlSpan = m_pMngmntService->GetSpanByIdx(0);
	if(pCntrlSpan)
	{
		for(int i = 0 ; i < MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES ; ++i)
		{
			pCntrlSpan->SetIPv6Address(i, autoIpV6[i].address); // set the address as the 1st IPv6 address of the Control
			pCntrlSpan->SetIPv6SubnetMask(i, autoIpV6[i].mask); // set the mask
		}
	}

}

STATUS CManagmentNetwork::ValidateInterfaceIpv4(IP_ADDR_S& ipAddrS)
{
	if((eIpType_IpV4 == ipAddrS.ipType) || (eIpType_Both ==ipAddrS.ipType) )
	{
		 string ip_address,ans;
		 eConfigInterfaceType ifType = GetMngrIfType();
		 std::string nic_name = GetLogicalInterfaceName(ifType,ipAddrS.ipType);
		 //check if interface is up
		std::string cmd_intUp = "/sbin/ifconfig  |grep '''" +nic_name +" .*'''";
		STATUS stat = SystemPipedCommand(cmd_intUp.c_str(), ans);
		if (STATUS_OK == stat && (ans.size() == 0))
		{
				return MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV4;
		}
		 //check for conflict on Ipv4
		 std::string command_confilct = "echo -n `/sbin/ifconfig | grep -A1 " + nic_name +" | grep 'inet addr' | cut -d':' -f2 | cut -d' ' -f1 | awk 'NR==1'`";
		 SystemPipedCommand(command_confilct.c_str(), ip_address);
		 DWORD dword_ipAddress = SystemIpStringToDWORD(ip_address.c_str());

		 if(dword_ipAddress != ipAddrS.iPv4)
		 {
			 if (IsSystemFlagExist(flagIsTarget) ) // no configuration should be done on Pizzas
			 {
				 TRACESTR(eLevelWarn) << "Conflict expecting ipv4 -> " << ipAddrS.iPv4_str << "  interface ipv4 which is configured ->" << ip_address.c_str();
				 PASSERTMSG(1, " Conflict management configuration on ipv4");
				 return MANGMENT_STATUS_INTERFACE_ERROR_CONFILCT_IPV4;
			 }
			 else
				 TRACESTR(eLevelInfoNormal) << "Conflict expecting ipv4 -> " << ipAddrS.iPv4_str << "  interface ipv4 which is configured ->" << ip_address.c_str() << "status ok since this is a pizza";
		 }
	}
	return MANGMENT_STATUS_INTERFACE_READY;
}


STATUS CManagmentNetwork::OnPreHandleNetworkSeparationConfigurations()
{
	return STATUS_OK;
}

STATUS    CManagmentNetwork::WriteManagmentNetwork(CNetworkSettings& netSettings)
{
	if(NULL !=m_pMngmntService)
	{
		netSettings.m_iptype = m_pMngmntService->GetIpType();
		netSettings.m_ipv6ConfigType = m_pMngmntService->GetIpV6ConfigurationType();

		CIPSpan * pSpan	=m_pMngmntService->GetFirstSpan();
		if(pSpan)
		{
			netSettings.m_ipv4	 		= pSpan->GetIPv4Address();
			netSettings.m_ipv4_Mask	 	= m_pMngmntService->GetNetMask();
			netSettings.m_ipv6_0.addr 	= pSpan->m_IPv6AaddressArray[0];
			netSettings.m_ipv6_0.mask   = pSpan->GetIPv6SubnetMask(0);
			netSettings.m_ipv6_1.addr	= pSpan->m_IPv6AaddressArray[1];
			netSettings.m_ipv6_0.mask   = pSpan->GetIPv6SubnetMask(1);
			netSettings.m_ipv6_2.addr	= pSpan->m_IPv6AaddressArray[2];
			netSettings.m_ipv6_0.mask   = pSpan->GetIPv6SubnetMask(2);
			eConfigInterfaceType ifType = GetMngrIfType();
			netSettings.m_interface	  =   GetLogicalInterfaceName( ifType,m_pMngmntService->GetIpType());
			netSettings.m_MacAddress  =   GetMacAddress(netSettings.m_interface);
		}

		char curIpv6DefGW[IPV6_ADDRESS_LEN];
		memset(curIpv6DefGW, 0, IPV6_ADDRESS_LEN);
		m_pMngmntService->GetDefaultGatewayBytesIpv6(netSettings.m_ipv6_DefGw.addr);
		netSettings.m_ipv6_DefGw.mask =  m_pMngmntService->GetDefaultGatewayMaskIPv6();
		m_pMngmntService->GetDefaultGatewayIPv6(curIpv6DefGW);

		netSettings.m_ipv4_DefGw   = m_pMngmntService->GetDefaultGatewayIPv4();
		CIPSpan* shelfSpan = NULL;
		shelfSpan =m_pMngmntService->GetSpanByIdx(1);
		if(shelfSpan)
		{
			netSettings.m_ipv6_Shelf.addr = shelfSpan->m_IPv6AaddressArray[0];
			netSettings.m_ipv6_Shelf.mask = shelfSpan->GetIPv6SubnetMask(0);
			netSettings.m_ipv4Shelf       =  shelfSpan->GetIPv4Address();
		}

		CDnsManagment* pDnsMngt = CNetworkFactory::GetInstance().CreateMngmntDns(CProcessBase::GetProcess()->GetProductType());
		if(pDnsMngt)
		{
			pDnsMngt->WriteMngmtDnsNetwork(netSettings);
		}

		netSettings.m_netConfigStatus = m_netConfigStatus;
		if(eNetConfigurationFailureActionChangeMngntIp4Type == netSettings.m_netConfigStatus)
		{
			netSettings.m_iptype = eIpType_IpV4;
		}
		return STATUS_OK;
	}
	return PLATFORM_STATUS_MNGMNT_IS_NULL;
}

STATUS	  CManagmentNetwork::ConfigureEthernetSettingsCpu(eConfigInterfaceType ifType, ePortSpeedType portSpeed)
{
	if (!IsSystemFlagExist(flagIsTarget) )
	{
			TRACESTR(eLevelDebug) << "CManagmentNetwork - no configuration should be done on Pizzas";
			return STATUS_OK;
	}
	std::string stParams = ParsePortSpeed(portSpeed);

    std::string eth = GetDeviceName(ifType);
    std::string cmd;

    //only for inner NIC eth0, settings are hardcoded
     if("eth0" == eth)
            stParams = "speed 100 autoneg off duplex full";

       cmd = "/sbin/ethtool -s " + eth + " " + stParams;


	   std::string answer;

	    STATUS stat = SystemPipedCommand(cmd.c_str(),answer);
	    TRACESTR(stat ? eLevelError:eLevelInfoNormal) <<
	        "CManagmentNetwork::ConfigureEthernetSettingsCpu :" << cmd << std::endl << answer;

	    if(STATUS_OK != stat)
	    {
	        stat = STATUS_FAIL_TO_CONFIG_MNGMNT_ETHERNET;
	    }
	    return stat;
}

STATUS	 CManagmentNetwork::ConfigOtherNetComponents()
{
	STATUS retStatus = STATUS_OK;

	CDnsManagment* pDnsMngt = CNetworkFactory::GetInstance().CreateMngmntDns(CProcessBase::GetProcess()->GetProductType());
	if(pDnsMngt)
	{
		retStatus = pDnsMngt->StartMngmtDNSConfig(m_pMngmntService);
	}
	else
	{
		return DNS_STATUS_OBJ_IS_NULL;
	}
	return retStatus;
}

	void 	 CManagmentNetwork::SetNetworkMngmtConfigStatus(eNetConfigurationStatus status)
	{
		if(eNetConfigurationFailureActionChangeMngntIp4Type == status)
		{
			if(m_pMngmntService->GetIpType() == eIpType_Both)
					m_netConfigStatus = status;
			else
				m_netConfigStatus =eNetConfigurationFailureNoAction;
		}
		else
			m_netConfigStatus = status;
	}
} /* namespace McmsNetworkPackage */
