/*
 * CDnsManagment.cpp
 *
 *  Created on: Oct 7, 2013
 *      Author: stanny
 */

#include "CDnsManagment.h"
#include "Trace.h"
#include "TraceStream.h"
#include <fstream>
#include <sys/types.h>
#include <grp.h>

namespace McmsNetworkPackage {

CDnsManagment::CDnsManagment() {

	m_pMngmntService = NULL;
	m_pMngmntDns    = NULL;
	m_statusDnsConfig = eDnsNotConfigured;

}

CDnsManagment::~CDnsManagment() {
	//POBJDELETE(m_pMngmntService);

}


STATUS CDnsManagment::NameServerUpdate(const std::string& dns, const std::string& host,
									 const std::string& zone,
									 const std::string& ip,
									 const std::string& ipv6)
{

	STATUS retStatus = STATUS_OK;
    //IPv4
	std::string fname = MCU_TMP_DIR+"/nsupdate.txt";
    ofstream nsupdate_file(fname.c_str());
    if (nsupdate_file.is_open() == FALSE)
    {
        PASSERT(1);
        return DNS_STATUS_FAIL_TO_OPEN_FILE;
    }

    nsupdate_file << "server " << dns << std::endl;
    nsupdate_file << "zone " << zone << std::endl;

    if("" != ip)
    {
        nsupdate_file << "update add " << host << "." << zone << " 86400 A " << ip << std::endl;
    }
    if("" != ipv6)
    {
        nsupdate_file << "update add " << host << "." << zone << " 86400 AAAA " << ipv6 << std::endl;
    }

    nsupdate_file << "show" << std::endl;
    nsupdate_file << "send" << std::endl;


    nsupdate_file.close();

    std::string answer;
    STATUS stat;
    std::string command;

    command = GetCmdLinePrefix() + "nsupdate < "+MCU_TMP_DIR+"/nsupdate.txt";

    retStatus = SystemPipedCommand(command.c_str(), answer);

    TRACESTR(retStatus ? eLevelError:eLevelInfoNormal) <<"NameServerUpdate IPv4/6:"  << answer;

    return (retStatus == STATUS_OK)? STATUS_OK:DNS_STATUS_FAIL_TO_RUN_NSUPDATE;
}


STATUS CDnsManagment::ConfigDnsAuto()
{
	TRACESTR(eLevelInfoNormal) << "\n" << __FUNCTION__ << " - dns mode is 'Auto'";
	STATUS retStatus = STATUS_OK;
	BOOL isDhcp = NO;
	CSysConfig *pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	if (pSysConfig)
		pSysConfig->GetBOOLDataByKey("DHCP_ENABLED", isDhcp);

	if ( YES == isDhcp )	// dhcp enabled in sysCfg
	{
		TRACESTR(eLevelInfoNormal)	<< "configuration using dhcp for dns is disabled for management. ";
	}
	else					// dhcp disabled in sysCfg
	{
		TRACESTR(eLevelInfoNormal)	<< "\n" << __FUNCTION__ << " - "
								<< "DHCP is disabled (in SysConfig); therefore nothing is actually done";
	}
	m_statusDnsConfig = eDnsConfigurationSuccess;
	return retStatus;
}
STATUS CDnsManagment::ConfigureDnsServers(const std::string& search,const std::string& dns1,const std::string& dns2,const std::string& dns3)
{
	STATUS retStatus = STATUS_OK;
	TRACESTR(eLevelInfoNormal)<< "ConfigureDnsServers" << " search: " << search
	    		                                   << " nameserver: " << dns1 <<" nameserver: " << dns2 << " nameserver: " << dns3;

	ofstream resolv_conf("/etc/resolv.conf");
	if (resolv_conf.is_open() == FALSE)
	{
		TRACESTR(eLevelError) << "failed to open /etc/resolv.conf";
		return DNS_STATUS_FAIL_TO_OPEN_FILE;
	}

	resolv_conf << "search " << search << std::endl;
	resolv_conf << "nameserver " << dns1 << std::endl;
	resolv_conf << "nameserver " << dns2 << std::endl;
	resolv_conf << "nameserver " << dns3 << std::endl;
	resolv_conf << "options timeout:3" << std::endl;
	resolv_conf << "options attempts:1" << std::endl;
	resolv_conf.close();
	return retStatus;
}



STATUS CDnsManagment::ConfigDnsInOS()
{
	STATUS retStatus = STATUS_OK;

    int ip_server1 = m_pMngmntDns->GetIPv4Address(0),
        ip_server2 = m_pMngmntDns->GetIPv4Address(1),
        ip_server3 = m_pMngmntDns->GetIPv4Address(2);

    if(ip_server1 == -1) ip_server1 = 0;
    if(ip_server2 == -1) ip_server2 = 0;
    if(ip_server3 == -1) ip_server3 = 0;

	char domainName[NAME_LEN],
		 ip_server1Str[IPV6_ADDRESS_LEN],
         ip_server2Str[IPV6_ADDRESS_LEN],
         ip_server3Str[IPV6_ADDRESS_LEN];

    memset(ip_server1Str, 0, IPV6_ADDRESS_LEN);
    memset(ip_server2Str, 0, IPV6_ADDRESS_LEN);
    memset(ip_server3Str, 0, IPV6_ADDRESS_LEN);
	eIpType ipType = m_pMngmntService->GetIpType();

	if(eIpType_IpV4 != ipType)
	{

		FTRACESTR(eLevelInfoNormal) <<"\nConfigDnsInOS ip_server1 = "<<(DWORD)ip_server1;
		// try to get ipv6
		if(0 == ip_server1)
		{
			m_pMngmntDns->GetIPv6Address(0, ip_server1Str);
		}

		if(0 == ip_server2)
		{
			m_pMngmntDns->GetIPv6Address(1, ip_server2Str);
		}

		if(0 == ip_server3)
		{
			m_pMngmntDns->GetIPv6Address(2, ip_server3Str);
		}
	}

    memset(domainName, 0, NAME_LEN);
    memcpy(domainName, m_pMngmntDns->GetDomainName().GetString(), NAME_LEN-1);

    if ( 0 != ip_server1)
        SystemDWORDToIpString(ip_server1, ip_server1Str);

    FTRACESTR(eLevelInfoNormal) <<"\nConfigDnsInOS ip_server1Str = "<<ip_server1Str;

    if ( 0 != ip_server2)
        SystemDWORDToIpString(ip_server2, ip_server2Str);

    if ( 0 != ip_server3)
        SystemDWORDToIpString(ip_server3, ip_server3Str);

	// ===== 2. call DnsConfig method
	retStatus = ConfigureDnsServers(domainName, ip_server1Str, ip_server2Str, ip_server3Str);

	// ===== 3. print to log
	FTRACESTR(eLevelInfoNormal)
    	<<"\nConfigDnsInOS "
    	<< "\ndomain name - " << domainName
    	<< "\nip_server1  - "  << ip_server1Str
    	<< "\nip_server2  - "  << ip_server2Str
        << "\nip_server3  - "  << ip_server3Str
    	<< "\nConfiguration status: " << CProcessBase::GetProcess()->GetStatusAsString(retStatus);

	return retStatus;
}

STATUS CDnsManagment::RegisterDnsClient()
{
	STATUS retStatus = STATUS_OK;

	if ( FALSE == m_pMngmntDns->GetRegisterDNSAutomatically() )
	{
		TRACESTR(eLevelInfoNormal) << "RegisterDnsClient - " << "dns not AutoRegister; therefore nothing is actually done";
		return retStatus;
	}

	string myIpStr = "", myIpV6Str = "", myHostName, myZone = "", dnsServerIpStr = "";
	char myIpArr[IP_ADDRESS_LEN+1], dnsServerIpArr[IP_ADDRESS_LEN];
	memset(myIpArr, 0, IP_ADDRESS_LEN+1);

   	CIPSpan* pTmpSpan  = m_pMngmntService->GetSpanByIdx(0); // Mngmnt params are stored in the 1st span (idx==0)
   	eIpType ipType = m_pMngmntService->GetIpType();

	// ===== 1. retrieve myIp
	if( pTmpSpan )
	{
		if(eIpType_IpV6 != ipType)
		{
			DWORD myIp = pTmpSpan->GetIPv4Address();
			SystemDWORDToIpString(myIp, myIpArr);
			myIpStr = myIpArr;
		}

		if(eIpType_IpV4 != ipType)
		{
			myIpV6Str = pTmpSpan->GetIPv6Address(0);
		}
	}
	else
		PTRACE(eLevelError,"RegisterDnsClient - pTmpSpan is NULL!!");


	// ===== 2. retrieve hostName
	myHostName =::GetHostNameFromService(m_pMngmntService);

	// ===== 3. retrieve zone
	myZone = m_pMngmntDns->GetDomainName().GetString();

	// ===== 4. retrieve dnsServerIp

	DWORD dnsIp = m_pMngmntDns->GetIPv4Address(0);
	if(dnsIp != 0)
	{
	SystemDWORDToIpString(dnsIp, dnsServerIpArr);
	dnsServerIpStr = dnsServerIpArr;
	}
	else
	{
		char ipv6[IPV6_ADDRESS_LEN];
		ipv6[0] = '\0';
		m_pMngmntDns->GetIPv6Address(0, ipv6);
		dnsServerIpStr = ipv6;
	}


	TRACESTR(eLevelInfoNormal) << "\nRegisterDnsClient"
	                       << "\nDns Server: "	<< dnsServerIpStr
	                       << "; HostName: "	<< myHostName
	                       << "; Zone: "		<< myZone
	                       << "; Ip: "			<< myIpStr;

	// ===== 5. register as client
	retStatus = NameServerUpdate(dnsServerIpStr, myHostName, myZone, myIpStr, myIpV6Str);

	return retStatus;
}

STATUS CDnsManagment::ConfigDnsSpecify()
{
	STATUS retStatus = STATUS_OK;
	TRACESTR(eLevelInfoNormal) << "start ConfigDnsSpecify";

	// ===== 1. config Dns (resolving)
	retStatus = ConfigDnsInOS();

	if (STATUS_OK != retStatus) // config Dns failed
	{
			TRACESTR(eLevelInfoNormal) << "\nConfigDnsSpecify - " << "Config Dns failed; status: "
										<< CProcessBase::GetProcess()->GetStatusAsString(retStatus).c_str();
			m_statusDnsConfig = eDnsConfigurationFailure;
	}
	else // config Dns succeeded
	{
			TRACESTR(eLevelInfoNormal) << "Config Dns succeeded! Start RegisterDnsClient";

			// ===== 2. register as client
			retStatus = RegisterDnsClient();
			TRACESTR(eLevelInfoNormal) << "ConfigDnsSpecify - " << "RegisterDnsClient status: "
									   << CProcessBase::GetProcess()->GetStatusAsString(retStatus).c_str();
			if(STATUS_OK == retStatus)
			{
				m_statusDnsConfig = eDnsConfigurationSuccess;
			}
			else
			{
				m_statusDnsConfig = eDnsConfigurationFailure;
			}

			/*
			if (STATUS_OK == retStatus) // dns registartion succeeded
			{
				RemoveActiveAlarmByErrorCode(AA_DNS_REGISTRAION_FAILED);
			}
			else
			{
				UpdateStartupConditionByErrorCode(AA_DNS_REGISTRAION_FAILED, eStartupConditionFail);
			}*/
	}
	return retStatus;
}

STATUS CDnsManagment::ConfigDnsOff()
{
	STATUS retStatus = STATUS_OK;


	retStatus = ConfigureDnsServers("", "0.0.0.0", "0.0.0.0", "0.0.0.0");
	if(STATUS_OK ==retStatus)
	{
		m_statusDnsConfig = eDnsNotConfigured;
		TRACESTR(eLevelInfoNormal) << "\nConfigDnsOff - Dns server mode is Off";
	}
	else
		TRACESTR(eLevelInfoNormal) << "\nConfigDnsOff - failed to change Dns server mode to Off"
        							<< "\nConfiguration status: " << CProcessBase::GetProcess()->GetStatusAsString(retStatus);
	return retStatus;
}




STATUS CDnsManagment::StartMngmtDNSConfig(CIPService *pService)
{
	STATUS retStatus = STATUS_OK;

	if( pService == NULL )
			return DNS_STATUS_OBJ_IS_NULL;

	UpdateDnsService(pService);
	/*if(m_pMngmntService)
		POBJDELETE(m_pMngmntService);

	m_pMngmntService = new CIPService(*pService);*/
	m_pMngmntService = pService;
	m_pMngmntDns = m_pMngmntService->GetpDns();
	if( m_pMngmntDns== NULL )
		return DNS_STATUS_OBJ_IS_NULL;

	eServerStatus dnsRegistrationMode = m_pMngmntDns->GetStatus();

	switch(dnsRegistrationMode)
	{
		case eServerStatusAuto:
			ConfigDnsAuto();
			break;
		case eServerStatusSpecify:
			ConfigDnsSpecify();
			break;

		case eServerStatusOff:
			ConfigDnsOff();
			break;
		default:
			retStatus = DNS_STATUS_INVALID_REG_MODE;
	}

	return retStatus;
}

STATUS CDnsManagment::WriteMngmtDnsNetwork(CNetworkSettings& netSettings)
{
	STATUS retStatus = STATUS_OK;
	if(m_pMngmntDns != NULL)
	{
		netSettings.m_ServerDnsStatus = m_pMngmntDns->GetStatus();
		netSettings.m_ipv4DnsServer =   m_pMngmntDns->GetIPv4Address(0);
		netSettings.m_ipv4DnsServer_1 =   m_pMngmntDns->GetIPv4Address(1);
		netSettings.m_ipv4DnsServer_2 =   m_pMngmntDns->GetIPv4Address(2);
		netSettings.SetDomainName(m_pMngmntDns->GetDomainName().GetString());
		netSettings.m_ipv6_DnsServer.mask = m_pMngmntDns->GetIPv6SubnetMask(0);
		m_pMngmntDns->GetDnsServersIpv6(0,netSettings.m_ipv6_DnsServer.addr);
		TRACESTR(eLevelInfoNormal) << "\n CDnsManagment::WriteMngmtDnsNetwork m_ipv6_DnsServer.addr = " << netSettings.m_ipv6_DnsServer.addr.ip;
		m_pMngmntDns->GetDnsServersIpv6(1,netSettings.m_ipv6_DnsServer_1.addr);
		m_pMngmntDns->GetDnsServersIpv6(2,netSettings.m_ipv6_DnsServer_2.addr);

		netSettings.m_DnsConfigStatus = m_statusDnsConfig;
	}
	else
		retStatus = DNS_STATUS_OBJ_IS_NULL;
	return retStatus;
}

////////////////////////
CDnsSoftMcuMfwEdgeAxis::CDnsSoftMcuMfwEdgeAxis() {
	m_isResolvedConfig = false;
}

CDnsSoftMcuMfwEdgeAxis::~CDnsSoftMcuMfwEdgeAxis() {
	//POBJDELETE(m_pMngmntService);
}

bool CDnsSoftMcuMfwEdgeAxis::TestMcmsUser()
{
	gid_t id = getgid();
	struct group  *gr  = getgrgid(id);
	if(gr && gr->gr_name)
	{
		std::string grp_mcms="mcms";
		if(strcmp("mcms",gr->gr_name)==0)
			return true;
		else
		{
			TRACESTR(eLevelInfoNormal)<< "not a mcms user  group user : " << gr->gr_name;
		}

	}

	return false;
}

void  CDnsSoftMcuMfwEdgeAxis::UpdateLocalDomain(std::string& line,int Ind,int tokenLen,CIPService* pService)
{
	line.replace(Ind,tokenLen,"");
	pService->GetpDns()->SetDomainName(line.c_str());
}

void   CDnsSoftMcuMfwEdgeAxis::UpdateNameServerInstances(std::string& line,int Ind,CIPService* pService)
{
	std::stringstream ss(line);
	string strFromFile;
	while (!ss.eof() && !ss.fail())
	{
		ss >> strFromFile;
		if (isIpV4Str(strFromFile.c_str()))
		{
					mcTransportAddress HostIp;
					memset(&HostIp,0,sizeof(mcTransportAddress));
					::stringToIpV4 (&HostIp, (char *)strFromFile.c_str(), eHost);
					pService->GetpDns()->SetIPv4Address(Ind, HostIp.addr.v4.ip);
		}
		else if (isIpV6Str(strFromFile.c_str()))
		{
					FTRACESTR(eLevelInfoNormal) << "CDnsSoftMcuMfwEdgeAxis::UpdateDnsService strFromFile = " << strFromFile.c_str();
					char tmpAddr[IPV6_ADDRESS_LEN];
					char tmpMask[IPV6_ADDRESS_LEN];

					memset(tmpAddr,			0, IPV6_ADDRESS_LEN);
					memset(tmpMask,			0, IPV6_ADDRESS_LEN);
					SplitIPv6AddressAndMask((char*)strFromFile.c_str(), tmpAddr, tmpMask);
					pService->GetpDns()->SetIPv6Address(Ind,  tmpAddr);
					pService->GetpDns()->SetIPv6SubnetMask(Ind, tmpMask);
		}
	}
}

STATUS CDnsSoftMcuMfwEdgeAxis::UpdateDnsService(CIPService* pService)
{
	//todo Ofir: read resolv.conf
	std::ifstream resolv_conf("/etc/resolv.conf");
	//std::ifstream resolv_conf("/tmp/resolve.conf");

	std::string line_with_domain;
	std::string domainStr = "domain ";
	std::string nameserverStr ="nameserver";
	int idxIpAddress = 0;
	while(std::getline(resolv_conf, line_with_domain))
	{
		std::size_t found = line_with_domain.find(domainStr);
		if (found!=std::string::npos)
		{
			UpdateLocalDomain(line_with_domain,found,domainStr.length(),pService);
		}
		else
		{
			std::size_t nsfound = line_with_domain.find(nameserverStr);
			if (nsfound!=std::string::npos)
			{
				UpdateNameServerInstances(line_with_domain,idxIpAddress,pService);
				idxIpAddress++;
			}
		}
	}

	resolv_conf.close();

	if (idxIpAddress > 0)
		pService->GetpDns()->SetStatus(eServerStatusSpecify);

	return STATUS_OK;
}
} /* namespace McmsNetworkPackage */
