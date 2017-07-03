//////////////////////////////////////////////////////////////////////
//
// IpParameters.cpp: implementation of the CIpParameters class.
//
//////////////////////////////////////////////////////////////////////


#include "NStream.h"
#include "IpParameters.h"
#include "IpService.h"
#include "StatusesGeneral.h"
#include "ProcessBase.h"
#include "TraceStream.h"


extern const char * PortSpeedTypeToString(ePortSpeedType type);


// ------------------------------------------------------------
CIpParameters::CIpParameters ()
{
}


// ------------------------------------------------------------
CIpParameters::CIpParameters (const IP_PARAMS_S ipParams)
{
	memcpy(&m_ipParamsStruct, &ipParams, sizeof(IP_PARAMS_S));
}

// ------------------------------------------------------------
CIpParameters::~CIpParameters ()
{
}


// ------------------------------------------------------------
void  CIpParameters::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n===== IpParameters::Dump =====\n";

	for (int i=0; i<MAX_NUM_OF_PQS; i++)
	{
		CIpInterface ipInterface(m_ipParamsStruct.interfacesList[i]);
		msg << ipInterface;
	}
	
	msg << CNetworkParameters(m_ipParamsStruct.networkParams)
		<< CDnsConfiguration(m_ipParamsStruct.dnsConfig);

    msg << "\n===== Port Speed Vector Dum::Dump =====\n";
    for (int i=0; i<MAX_NUM_OF_PORTS_SPEED; i++)
	{
        const char *speedName = PortSpeedTypeToString((ePortSpeedType)m_ipParamsStruct.portSpeedList[i].portSpeed);
        
		msg << "Number : "   << m_ipParamsStruct.portSpeedList[i].portNum
            << " , Speed : " << speedName << "\n";
	}

    msg << "\n===== V35 GW =====\n";
    msg << "V35 ipv4:" << m_ipParamsStruct.v35GwIpv4Address;
}


// ------------------------------------------------------------
CIpParameters& CIpParameters::operator = (const CIpParameters &rOther)
{
	memcpy(&m_ipParamsStruct, &rOther.m_ipParamsStruct, sizeof(IP_PARAMS_S));

	return *this;
}


// ------------------------------------------------------------
IP_PARAMS_S* CIpParameters::GetIpParamsStruct ()
{
	return &m_ipParamsStruct;
}


// ------------------------------------------------------------
IP_INTERFACE_S* CIpParameters::GetIpInterfacesList ()
{
	return m_ipParamsStruct.interfacesList;
}


// ------------------------------------------------------------
IP_INTERFACE_S	CIpParameters::GetIpInterfaceByIdx (const int idx)
{
    int index = idx;
	if (idx >= MAX_NUM_OF_PQS)
    {
        index = 0;
        PASSERTMSG(idx, "Bad index");
    }
    return m_ipParamsStruct.interfacesList[index];
}

// ------------------------------------------------------------
void CIpParameters::ClearInterfacesList ()
{
	memset(&m_ipParamsStruct.interfacesList, 0, sizeof(m_ipParamsStruct.interfacesList));
}

// ------------------------------------------------------------
void CIpParameters::ClearPortSpeedList()
{
	memset(&m_ipParamsStruct.portSpeedList, 0, sizeof(m_ipParamsStruct.portSpeedList));
}

// ------------------------------------------------------------
void CIpParameters::InsertToInterfacesList (IP_INTERFACE_S &ipInterface)
{
	int i=0;
	for (i = 0; i < MAX_NUM_OF_PQS; i++)
	{
		if (0 == m_ipParamsStruct.interfacesList[i].boardId) // empty interface
		{
			memcpy( &m_ipParamsStruct.interfacesList[i],
			        &ipInterface,
					sizeof(IP_INTERFACE_S) );
		
			break;
		}
	}
}


// ------------------------------------------------------------
NETWORK_PARAMS_S CIpParameters::GetNetworkParams ()
{
	return m_ipParamsStruct.networkParams;
}


// ------------------------------------------------------------
void CIpParameters::SetNetworkParams (const NETWORK_PARAMS_S netParams)
{
	memcpy( &m_ipParamsStruct.networkParams, &netParams, sizeof(NETWORK_PARAMS_S) );
}


// ------------------------------------------------------------
DNS_CONFIGURATION_S CIpParameters::GetDnsConfiguration ()
{
	return m_ipParamsStruct.dnsConfig;
}


// ------------------------------------------------------------
void CIpParameters::SetDnsConfiguration (const DNS_CONFIGURATION_S dnsConfig)
{
	memcpy( &m_ipParamsStruct.dnsConfig, &dnsConfig, sizeof(DNS_CONFIGURATION_S) );
}


// ------------------------------------------------------------
void CIpParameters::SetData(const char *data)
{
	memcpy(&m_ipParamsStruct, data, sizeof(IP_PARAMS_S));
}


// ------------------------------------------------------------
void CIpParameters::SetData(const IP_PARAMS_S ipParams)
{
	memcpy(&m_ipParamsStruct, &ipParams, sizeof(IP_PARAMS_S));
}


// ------------------------------------------------------------
STATUS CIpParameters::ConvertToIpService(CIPService &ipService)
{
	TRACESTR(eLevelInfoNormal) << "\nCIpParameters::ConvertToIpService";

	int i=0, j=0;
	char tmpIPv6Addr[IPV6_ADDRESS_LEN];
	char tmpIPv6Mask[IPV6_ADDRESS_LEN];
        
	// ===== 1. NetworkParameters attributes
	const NETWORK_PARAMS_S &curNetParamStruct = m_ipParamsStruct.networkParams;
	
	ipService.SetNetMask( curNetParamStruct.subnetMask );
	ipService.SetDefaultGatewayIPv4( curNetParamStruct.defaultGateway );

	memset(tmpIPv6Addr,	0, IPV6_ADDRESS_LEN);
	memset(tmpIPv6Mask,	0, IPV6_ADDRESS_LEN);

	SplitIPv6AddressAndMask((char*)(curNetParamStruct.defaultGatewayIPv6), tmpIPv6Addr, tmpIPv6Mask);

	ipService.SetDefaultGatewayIPv6( (char*)tmpIPv6Addr);
	ipService.SetDefaultGatewayMaskIPv6( (char*)tmpIPv6Mask);

	ipService.SetDHCPServer( (WORD)(curNetParamStruct.isDhcpInUse) );
	
	CProcessBase *pProcess = CProcessBase::GetProcess();
	
	if ( eProductFamilySoftMcu != pProcess->GetProductFamily() )
	{
		ipService.RemoveAllRouters();
		for (i=0; i < MAX_ROUTERS_IN_H323_SERVICE; i++)
		{
			CH323Router routerToAdd;
	//		if ( 0 != curNetParamStruct.ipRouter[i].routerIp ) // ipRouter exists
	//		{
				routerToAdd.SetRouterIP( curNetParamStruct.ipRouter[i].routerIp );
				routerToAdd.SetRemoteIP( curNetParamStruct.ipRouter[i].remoteIp );
				routerToAdd.SetSubnetMask( curNetParamStruct.ipRouter[i].subnetMask );
				routerToAdd.SetRemoteFlag( (BYTE)(curNetParamStruct.ipRouter[i].remoteFlag) );
	//		}
			STATUS retStatus = ipService.AddRouter(routerToAdd, false, false);
			TRACEINTO << (eLevelInfoNormal) << "\nAdd Router, Status : " << pProcess->GetStatusAsString(retStatus);
		}
	}

	// ===== 2. Spans attributes
    for (i=0; i < MAX_NUM_OF_PQS; i++)
    {
    	const IP_INTERFACE_S &curInterfaceStruct = m_ipParamsStruct.interfacesList[i];
    	
    	if (0 == i) // the following params are taken from the first interface only (Management interface)
    	{
	    	ipService.SetIpType( (eIpType)(curInterfaceStruct.ipType) );
	    	ipService.SetIpV6ConfigurationType( (eV6ConfigurationType)(curInterfaceStruct.iPv6s[0].configurationType) );
     		ipService.SetIsSecured( curInterfaceStruct.isSecured );
    		ipService.SetIsPermanentNetworkOpen( curInterfaceStruct.isPermanentOpen );
    		if(ipService.GetIsPermanentNetworkOpen())
    			TRACESTR(eLevelInfoNormal) <<"CIpParameters::ConvertToIpService- curInterfaceStruct.isPermanentOpen= YES" ;
    		else
    			TRACESTR(eLevelInfoNormal) <<"CIpParameters::ConvertToIpService- curInterfaceStruct.isPermanentOpen= NO" ;
    	}

    	
    	CIPSpan spanToAdd;
    	spanToAdd.SetLineNumber(i);

    	spanToAdd.SetIPv4Address( curInterfaceStruct.iPv4.iPv4Address );

    	for(int addressIdx=0; addressIdx<NUM_OF_IPV6_ADDRESSES; addressIdx++)
    	{
    		// remove the slash
    		memset(tmpIPv6Addr,	0, IPV6_ADDRESS_LEN);
    		memset(tmpIPv6Mask,	0, IPV6_ADDRESS_LEN);

    		SplitIPv6AddressAndMask((char*)(curInterfaceStruct.iPv6s[addressIdx].iPv6Address), tmpIPv6Addr, tmpIPv6Mask);

    		spanToAdd.SetIPv6Address(addressIdx, tmpIPv6Addr);
    		spanToAdd.SetIPv6SubnetMask(addressIdx, tmpIPv6Mask);

//    		spanToAdd.SetIPv6Address( (char*)(curInterfaceStruct.iPv6s[addressIdx].iPv6Address), addressIdx );	
//    		spanToAdd.SetIpV6SubnetMask( curInterfaceStruct.iPv6s[addressIdx].ipV6SubnetMask );
    	}


    	
    	for (j=0; j < MAX_ALIAS_NAMES_NUM; j++)
    	{
    		CH323Alias aliasToAdd;
			aliasToAdd.SetAliasName( (char*)(curInterfaceStruct.aliasesList[j].aliasContent) );
			
			spanToAdd.AddAlias(aliasToAdd);
    	}
    	
    	ipService.ReplaceSpan_NoCheck(i, spanToAdd);
    }


	// ===== 3. DNS attributes
	const DNS_CONFIGURATION_S &curDnsConfigStruct = m_ipParamsStruct.dnsConfig;

	CIPSpan *firstSpan = ipService.GetFirstSpan();
	if(NULL != firstSpan)
	{
		firstSpan->SetSpanHostName((char*)(curDnsConfigStruct.hostName));	
	}
	else
	{
		CIPSpan span;
		span.SetSpanHostName((char*)(curDnsConfigStruct.hostName));
		ipService.AddSpan(span);
	}
	
	CIpDns dnsToSet;
	dnsToSet.SetDomainName( (char*)(curDnsConfigStruct.domainName) );
	dnsToSet.SetStatus( (eServerStatus)(curDnsConfigStruct.dnsServerStatus) );
	dnsToSet.SetRegisterDNSAutomatically( (WORD)(curDnsConfigStruct.isRegister) );

    for (i=0; i < NUM_OF_DNS_SERVERS; i++)
    {
        dnsToSet.SetIPv4Address( i, curDnsConfigStruct.ipV4AddressList[i] );
        
		// remove the slash
		memset(tmpIPv6Addr,	0, IPV6_ADDRESS_LEN);
		SplitIPv6AddressAndMask((char*)(curDnsConfigStruct.ipV6AddressList[i]), tmpIPv6Addr, tmpIPv6Mask);
		dnsToSet.SetIPv6Address(i, tmpIPv6Addr);
//		dnsToSet.SetIPv6Address( (char*)(curDnsConfigStruct.ipV6AddressList[i]), i );	
    }
    
    ipService.SetDns(dnsToSet);

    const PORT_SPEED *pPortSpeedFrom = m_ipParamsStruct.portSpeedList;
    CPortSpeed *pPortSpeedTo = ipService.GetPortSpeedVector();
    for(int i = 0 ; i < MAX_NUM_OF_PORTS_SPEED ; i++)
    {
        pPortSpeedTo[i].SetNum(pPortSpeedFrom[i].portNum);
        pPortSpeedTo[i].SetSpeed((ePortSpeedType)pPortSpeedFrom[i].portSpeed);
    }
    
	return STATUS_OK;
}

// ------------------------------------------------------------
void CIpParameters::ValidateStrings()
{
    CProcessBase *pProcess = CProcessBase::GetProcess();

    // 1. validate IP_INTERFACE_S
    for(int i = 0 ; i < MAX_NUM_OF_PQS ; i++)
    {
        IP_INTERFACE_S &curIpInterface = m_ipParamsStruct.interfacesList[i];
        for(int j = 0 ; j < MAX_ALIAS_NAMES_NUM ; j++)
        {
            ALIAS_S &curAlias = curIpInterface.aliasesList[j];
            pProcess->TestStringValidity((char *)curAlias.aliasContent,
                                         MAX_ALIAS_NAMES_NUM,
                                         __PRETTY_FUNCTION__);
        }
    }

    // 2. validate DNS_CONFIGURATION_S
    std::string hostname = (char *)m_ipParamsStruct.dnsConfig.hostName;
    if(hostname.empty())
    {
    	char *ptr = (char *)m_ipParamsStruct.dnsConfig.hostName;
		if(ptr !=NULL)
		{
			// ===== 2. replace string
			memset(ptr, 0, NAME_LEN);
			strncpy(ptr, "PolycomMCU", NAME_LEN - 1);
			ptr[NAME_LEN - 1] = '\0';
		}
    }
    pProcess->TestStringValidity((char *)m_ipParamsStruct.dnsConfig.hostName,
                                 NAME_LEN,
                                 __PRETTY_FUNCTION__);
    pProcess->TestStringValidity((char *)m_ipParamsStruct.dnsConfig.domainName,
                                 NAME_LEN,
                                 __PRETTY_FUNCTION__);


}



//////////////////////////////////////////////////////////////////////
//
// CsIpParameters.cpp: implementation of the CCsIpParameters class.
//
//////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------
CCsIpParameters::CCsIpParameters ()
{
}


// ------------------------------------------------------------
CCsIpParameters::CCsIpParameters (const CS_IP_PARAMS_S ipParams)
{
	memcpy(&m_ipParamsStruct, &ipParams, sizeof(CS_IP_PARAMS_S));
}

// ------------------------------------------------------------
CCsIpParameters::~CCsIpParameters ()
{
}


// ------------------------------------------------------------
void  CCsIpParameters::Dump(std::ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);
	
	msg << "\n===== CsIpParameters::Dump =====\n";

	for (int i=0; i<(MAX_NUM_OF_BOARDS * MAX_NUM_OF_PQS); i++)
	{
		CIpInterface ipInterface(m_ipParamsStruct.interfacesList[i]);
		msg << ipInterface;
	}
	
	msg << CNetworkParameters(m_ipParamsStruct.networkParams)
		<< CDnsConfiguration(m_ipParamsStruct.dnsConfig);

	msg << "\n===== V35 GW =====\n";
	msg << "V35 ipv4:" << m_ipParamsStruct.v35GwIpv4Address;
}


// ------------------------------------------------------------
CCsIpParameters& CCsIpParameters::operator = (const CCsIpParameters &rOther)
{
	memcpy(&m_ipParamsStruct, &rOther.m_ipParamsStruct, sizeof(CS_IP_PARAMS_S));

	return *this;
}

// ------------------------------------------------------------
CS_IP_PARAMS_S* CCsIpParameters::GetIpParamsStruct ()
{
	return &m_ipParamsStruct;
}

// ------------------------------------------------------------
IP_INTERFACE_S* CCsIpParameters::GetIpInterfacesList ()
{
	return m_ipParamsStruct.interfacesList;
}


// ------------------------------------------------------------
void CCsIpParameters::InsertToInterfacesList (IP_INTERFACE_S &ipInterface)
{
	int i=0;
	for (i = 0; i < MAX_NUM_OF_PQS; i++)
	{
		if (0 == m_ipParamsStruct.interfacesList[i].boardId) // empty interface
		{
			memcpy( &m_ipParamsStruct.interfacesList[i],
			        &ipInterface,
					sizeof(IP_INTERFACE_S) );
		
			break;
		}
	}
}


// ------------------------------------------------------------
void CCsIpParameters::ClearInterfacesList ()
{
	memset(&m_ipParamsStruct.interfacesList, 0, sizeof(m_ipParamsStruct.interfacesList));
}


// ------------------------------------------------------------
NETWORK_PARAMS_S CCsIpParameters::GetNetworkParams ()
{
	return m_ipParamsStruct.networkParams;
}


// ------------------------------------------------------------
void CCsIpParameters::SetNetworkParams (const NETWORK_PARAMS_S netParams)
{
	memcpy( &m_ipParamsStruct.networkParams, &netParams, sizeof(NETWORK_PARAMS_S) );
}


// ------------------------------------------------------------
DNS_CONFIGURATION_S CCsIpParameters::GetDnsConfiguration ()
{
	return m_ipParamsStruct.dnsConfig;
}


// ------------------------------------------------------------
void CCsIpParameters::SetDnsConfiguration (const DNS_CONFIGURATION_S dnsConfig)
{
	memcpy( &m_ipParamsStruct.dnsConfig, &dnsConfig, sizeof(DNS_CONFIGURATION_S) );
}

// ------------------------------------------------------------
void CCsIpParameters::SetData(const char *data)
{
	memcpy(&m_ipParamsStruct, data, sizeof(CS_IP_PARAMS_S));
}


