// CommCardService.cpp

#include "CommCardService.h"
#include "CSMngrProcess.h"
#include "IpService.h"
#include "Segment.h"
#include "DynIPSProperties.h"
#include "StatusesGeneral.h"
#include "MediaIpParameters.h"
#include "TraceStream.h"
#include "OpcodesMcmsInternal.h"
#include "WrappersCards.h"
#include "ManagerApi.h"

CCommCardService::CCommCardService()
{
	memset(m_shortInterfacesListForUtilityProcess, 0, sizeof(m_shortInterfacesListForUtilityProcess));
	m_numOfElem = 0;
}

CCommCardService::~CCommCardService()
{}

STATUS CCommCardService::SendIpServiceParamInd(CIPService *service)
{

	TRACEINTO << "\n SendIpServiceParamInd";
	CS_MEDIA_IP_PARAMS_S param;
	const DWORD sizeOfCS_MEDIA_IP_PARAMS_S = sizeof(CS_MEDIA_IP_PARAMS_S);
	
	memset(&param, 0, sizeOfCS_MEDIA_IP_PARAMS_S);
	param.future_use2 = 42;

	FillParams(param, service);
	FillParams(m_cs_media_ip_params, service);
	
	CCsMediaIpParameters wrapper;
	wrapper.SetData(&param);
	TRACEINTO << wrapper;

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&param, sizeOfCS_MEDIA_IP_PARAMS_S);

	STATUS res = SendToMcmsProcess(eProcessCards, CS_CARDS_MEDIA_IP_PARAMS_IND, pSeg);

	//save interface parameters for sending to Utility process



	for (int i =0;i<MAX_NUM_OF_BOARDS * MAX_NUM_OF_PQS;i++)
	{
		if (m_cs_media_ip_params.ipParams.interfacesList[i].boardId !=0 && m_cs_media_ip_params.ipParams.interfacesList[i].iPv4.iPv4Address != 0)
		{
			char tmpIP[IP_ADDRESS_LEN];
			::SystemDWORDToIpString(m_cs_media_ip_params.ipParams.interfacesList[i].iPv4.iPv4Address, tmpIP);

			TRACEINTO << "\nCCommCardService::SendIpServiceParamInd"
					  << "\nIndex: " << i
				          << "\nBoardId: " << m_cs_media_ip_params.ipParams.interfacesList[i].boardId
				          << "\niPv4Address: " << tmpIP
				          << "\nipType: " << m_cs_media_ip_params.ipParams.interfacesList[i].ipType
				          << "\npqId: " << m_cs_media_ip_params.ipParams.interfacesList[i].pqId
				          <<"\n";

			m_shortInterfacesListForUtilityProcess[m_numOfElem].boardId = m_cs_media_ip_params.ipParams.interfacesList[i].boardId;
			m_shortInterfacesListForUtilityProcess[m_numOfElem].iPv4Address = m_cs_media_ip_params.ipParams.interfacesList[i].iPv4.iPv4Address;
			m_shortInterfacesListForUtilityProcess[m_numOfElem].ipType = m_cs_media_ip_params.ipParams.interfacesList[i].ipType;
			m_shortInterfacesListForUtilityProcess[m_numOfElem].pqId   = m_cs_media_ip_params.ipParams.interfacesList[i].pqId;

			m_numOfElem++;
		}



	}


	TRACEINTO << "\nCCommCardService::SendIpServiceParamInd  m_numOfElem "<< m_numOfElem;

	return res;
}

STATUS CCommCardService::SendIpServiceParamUtility()
{
	const DWORD sizeOfArray = sizeof(m_shortInterfacesListForUtilityProcess);
	CSegment *pSeg = new CSegment;
	*pSeg << (DWORD)m_numOfElem;
	pSeg->Put((BYTE*)&m_shortInterfacesListForUtilityProcess, sizeOfArray);
	STATUS res = SendToMcmsProcess(eProcessUtility, CS_UTILITY_MEDIA_IP_PARAMS_IND, pSeg);

	return res;
}

STATUS CCommCardService::SendIpServiceParamEndInd()
{
	STATUS res = SendToMcmsProcess(eProcessCards, CS_CARDS_MEDIA_IP_PARAMS_END_IND, NULL);

	return res;
}

// Virtual
STATUS CCommCardService::SendServiceCfgList(CSegment *pSeg)
{
	/*TODO: maybe we need to send message to utility here.*/
	/*implement the virtual function avoid the memory leak:  David Liang -- 2012-05-08*/
	POBJDELETE(pSeg);
	return STATUS_OK;
}


#define REALLY_BAD_VALUE_OOOOOOOOOOOOOOOOOOOOOO 0x77777777

//////////////////////////////////////////////////////////////////////
STATUS CCommCardService::ReceiveMediaIpConfigInd(CSegment *pSeg, int& serviceId)
{
	const CS_MEDIA_IP_CONFIG_S *pParam = (const CS_MEDIA_IP_CONFIG_S*)pSeg->GetPtr();
	const MEDIA_IP_CONFIG_S mediaIpConfigStruct = pParam->mediaIpConfig;
	
	TRACEINTO << "\nCCommCardService::ReceiveMediaIpConfigInd"
	          << "\nBoardId: " << pParam->boardId << ", subBoardId: " << pParam->subBoardId << "\n"
	          << CMediaIpConfigWrapper(mediaIpConfigStruct);
	
/*	
	if(REALLY_BAD_VALUE_OOOOOOOOOOOOOOOOOOOOOO == param->status) 
	{
		// do something terrible
		TRACEINTO <<
		return param->status;
	}
*/
	serviceId = mediaIpConfigStruct.serviceId;

	CIPServiceList *pList = GetIpServiceList();
	
	CIPService *service = pList->GetService(serviceId);

	if(service == NULL)
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	
	service->GetDynamicProperties()->GetCardIpAddress().SetIpInfo(mediaIpConfigStruct);
	
	// update the services
	SetMpmIPv6AddressesIfNeeded(pParam, serviceId);
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
void CCommCardService::SetMpmIPv6AddressesIfNeeded(const CS_MEDIA_IP_CONFIG_S *pParam, const int serviceId)
{
	CIPServiceList *pList = GetIpServiceList();
	CIPService *pService = pList->GetService(serviceId);
	if (pService)
	{
		eIpType					ipType		= pService->GetIpType();
		eV6ConfigurationType	configType	= pService->GetIpV6ConfigurationType();
	
		if ( ((eIpType_IpV6 == ipType) || (eIpType_Both == ipType)) &&
			 (eV6Configuration_Auto == configType) )
		{
			SetMPMIPv6Addresses(pParam->boardId, pParam, serviceId);
		}
	}
}


//////////////////////////////////////////////////////////////////////
void CCommCardService::SetMPMIPv6Addresses(DWORD boardId, const CS_MEDIA_IP_CONFIG_S *pParam, const int serviceId)
{
	CLargeString retStr = "\nCommCardService::SetMPMIPv6Addresses";
	retStr << "\nBoardId" << boardId <<" ;ServiceId = "<<serviceId;

	CCSMngrProcess	*pProcess		= (CCSMngrProcess*)(CProcessBase::GetProcess()); 
	CIPServiceList	*pStaticList	= pProcess->GetIpServiceListStatic();
	CIPService		*pService		= pStaticList->GetService(serviceId);
	
	if (pService)
	{
		// params of Signaling are stored in span 0
		// params of MPM of boardId 1 are stored in spans 1, 2
		// params of MPM of boardId 2 are stored in spans 3, 4
		// params of MPM of boardId 3 are stored in spans 5, 6
		// params of MPM of boardId 4 are stored in spans 7, 8
		int firstRelevantSpanNumber = GetFirstSpanNumberForMPM(boardId);

		// ===== 1. update Static list
		CIPSpan *pFirstSpan		= pService->GetSpanByIdx(firstRelevantSpanNumber),
				*pSecondSpan	= pService->GetSpanByIdx(firstRelevantSpanNumber+1);

		//    ----- 1a. IPv6 addresses
		char curAddr[IPV6_ADDRESS_LEN];
		char curMask[IPV6_ADDRESS_LEN];

		for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
		{
			memset(curAddr,	0, IPV6_ADDRESS_LEN);

			// remove the slash
			SplitIPv6AddressAndMask((char*)(pParam->mediaIpConfig.iPv6[i].iPv6Address), curAddr, curMask);
			
			if (pFirstSpan)
			{

				pFirstSpan->SetIPv6Address(i, curAddr);
		   		pFirstSpan->SetIPv6SubnetMask(i, curMask);

			}
			
		   	if (pSecondSpan)
		   	{
		   		pSecondSpan->SetIPv6Address(i, curAddr);
		   		pFirstSpan->SetIPv6SubnetMask(i, curMask);
		   	}

		   	retStr << "\nAddress " << i << ": " << curAddr;
		   	if (0 != curAddr[0])
		   	{
		   		retStr << " (mask: " << curMask << ")";
		   	}
		}
	

	    std::string ipServiceName, ipServiceNameTmp;

	    pProcess->GetIpServiceFileNames(ipServiceName, ipServiceNameTmp);

		//    ----- 1b. IPv6 defGw
	    pService->GetDefaultGatewayFullIPv6( (char*)(pParam->mediaIpConfig.defaultGatewayIPv6) );

	   	retStr << "\nDefGw: " << (char*)(pParam->mediaIpConfig.defaultGatewayIPv6);

	   	pStaticList->UpdateCounters();
	   	pStaticList->WriteXmlFile(ipServiceNameTmp.c_str());
	
		// ===== 2. update Dynamic list
	   	CIPServiceList *pDynamicList = GetIpServiceList();
	   	*pDynamicList = *pStaticList;
	   	pDynamicList->UpdateCounters();
	   	pDynamicList->WriteXmlFile(ipServiceName.c_str());
   	
   	
		// ===== 3. print
	   	TRACESTR(eLevelInfoNormal) << retStr.GetString();
	}

	else // if (!pService)
	{
		TRACESTR(eLevelInfoNormal) << "\nCommCardService::SetMPMIPv6Addresses"
		                              << "\nBoardId: " << boardId
		                              << "\nFirst service in static list is NULL";
	}
}

//////////////////////////////////////////////////////////////////////
int CCommCardService::GetFirstSpanNumberForMPM(int boardId)
{
	// params of MPM of boardId 1 are stored in spans 1, 2
	// params of MPM of boardId 2 are stored in spans 3, 4
	// params of MPM of boardId 3 are stored in spans 5, 6
	// params of MPM of boardId 4 are stored in spans 7, 8

	int retSpanNum = 0;
	
	switch (boardId)
	{
		case FIXED_BOARD_ID_MEDIA_1:
		{
			retSpanNum = 1;
			break;
		}
		
		case FIXED_BOARD_ID_MEDIA_2:
		{
			retSpanNum = 3;
			break;
		}
		
		case FIXED_BOARD_ID_MEDIA_3:
		{
			retSpanNum = 5;
			break;
		}
		
		case FIXED_BOARD_ID_MEDIA_4:
		{
			retSpanNum = 7;
			break;
		}
		
		default:
		{
			break;
		}
	}
	
	return retSpanNum;
}

/*
//////////////////////////////////////////////////////////////////////
// The following methods retrive only the Global address for the Media (as was first required).
//    Now that it is required to use all types of addresses, those methods were changed.  
//////////////////////////////////////////////////////////////////////
void CCommCardService::GetIPv6AddressGlobalFromStruct(const CS_MEDIA_IP_CONFIG_S *pParam, char *pOutAddr)
{
	bool isGlobalAddressFound = false;
	char tmpAddr[IPV6_ADDRESS_LEN];
	char tmpMask[IPV6_ADDRESS_LEN];
	
	// ===== 1. find a Global address
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(tmpAddr,	0, IPV6_ADDRESS_LEN);

		// remove the slash
		SplitIPv6AddressAndMask((char*)(pParam->mediaIpConfig.iPv6[i].iPv6Address), tmpAddr, tmpMask);

		enScopeId curScopeIdType = ::getScopeId(tmpAddr);
		if (eScopeIdGlobal == curScopeIdType)
		{
			isGlobalAddressFound = true;
			strncpy(pOutAddr, tmpAddr, IPV6_ADDRESS_LEN);
			break;
		}
	}

	
	// ===== 2. print
	if (false == isGlobalAddressFound)
	{
		TRACESTR(eLevelInfoNormal) << "\nCCommCardService::GetIPv6AddressGlobalFromStruct"
									  << "\nGlobal address was not found";
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCCommCardService::GetIPv6AddressGlobalFromStruct"
									  << "\nGlobal address was found"
									  << "\nAddress: "	<< pOutAddr
									  << "\nMask: "		<< tmpMask;
	}
}

//////////////////////////////////////////////////////////////////////
void CCommCardService::SetMPMIPv6Addresses(DWORD boardId, char *pAddr, char *pIPv6DefGw)
{
	TRACESTR(eLevelInfoNormal) << "\nCommCardService::SetMPMIPv6Addresses"
	                              << "\nBoardId: " << boardId
	                              << "\nAddress: " << pAddr
	                              << "\nDefGw:   " << pIPv6DefGw;
	

	// params of Signaling are stored in span 0
	// params of MPM of boardId 1 are stored in spans 1, 2
	// params of MPM of boardId 2 are stored in spans 3, 4
	int firstRelevantSpanNumber = GetFirstSpanNumberForMPM(boardId);

	CCSMngrProcess	*pProcess		= (CCSMngrProcess*)(CProcessBase::GetProcess()); 
	CIPServiceList	*pStaticList	= pProcess->GetIpServiceListStatic();
	CIPService		*pService		= pStaticList->GetFirstService();
	if (pService)
	{
		// ===== 1. update Static list
		CIPSpan *pFirstSpan = pService->GetSpanByIdx(firstRelevantSpanNumber);
	   	if (pFirstSpan)
	   		pFirstSpan->SetIPv6Address(0, pAddr); // set the address as the 1st IPv6 address of the MPM

		CIPSpan *pSecondSpan = pService->GetSpanByIdx(firstRelevantSpanNumber+1);
	   	if (pSecondSpan)
	   		pSecondSpan->SetIPv6Address(0, pAddr);
	   	
	   	pService->SetDefaultGatewayIPv6(pIPv6DefGw);

	   	
	   	pStaticList->UpdateCounters();
	   	pStaticList->WriteXmlFile(IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_TMP_PATH);


		// ===== 2. update Dynamic list
	   	CIPServiceList *pDynamicList = GetIpServiceList();
	   	*pDynamicList = *pStaticList;
	   	pDynamicList->UpdateCounters();
	   	pDynamicList->WriteXmlFile(IP_MULTIPLE_SERVICES_TWO_SPANS_LIST_PATH);
	}
}
*/
//////////////////////////////////////////////////////////////////////
STATUS CCommCardService::SendDelIpService(CIPService *service)
{
	STATUS res = SendDelIpServiceToMcmsProcess(eProcessCards, CS_CARDS_DELETE_IP_SERVICE_IND, service);

	return res;
}

//////////////////////////////////////////////////////////////////////
void CCommCardService::FillParams(CS_MEDIA_IP_PARAMS_S &param, CIPService *service)
{
	memset(&param, 0, sizeof(CS_MEDIA_IP_PARAMS_S));
	
	param.serviceId = service->GetId();
	strcpy((char*)param.serviceName, service->GetName());

	SetMediaParams(param, service);
}

//////////////////////////////////////////////////////////////////////
void CCommCardService::SetMediaParams(CS_MEDIA_IP_PARAMS_S &param, CIPService *service)
{
	SetNetwork(param.ipParams.networkParams, service);
	SetSpans(param.ipParams, service, param.csIp);
	SetRouters(param.ipParams.networkParams, service);
	
	CIpDns *dns = service->GetpDns();
	if(NULL != dns)
	{
		SetDns(param.ipParams.dnsConfig, dns);
	}

	if(service->GetIsV35GwEnabled())
		param.ipParams.v35GwIpv4Address = service->GetV35GwIpAddress();
}

//////////////////////////////////////////////////////////////////////
void CCommCardService::SetNetwork(NETWORK_PARAMS_S &param, CIPService *service)
{
	param.isDhcpInUse	 = service->GetDHCPServer();
	param.subnetMask	 = service->GetNetMask();
	param.defaultGateway = service->GetDefaultGatewayIPv4();

	service->GetDefaultGatewayFullIPv6((char*)(param.defaultGatewayIPv6));

}
	
//////////////////////////////////////////////////////////////////////
void CCommCardService::SetDns(DNS_CONFIGURATION_S &param, CIpDns *dns)
{
	param.dnsServerStatus = (APIU32)dns->GetStatus();
	param.dnsConfiguredFromDHCPv4_or_DHCPv6 = eDnsDhcpV4;
	memcpy(
		param.hostName, 
		dns->GetHostServiceName().GetString(), 
		NAME_LEN
		);
	memcpy(
			param.domainName,
			dns->GetDomainName().GetString(),
			NAME_LEN
			);

	for(int i = 0 ; i < NUM_OF_DNS_SERVERS ; i++)
	{
		param.ipV4AddressList[i] = dns->GetIPv4Address(i);
	}
}

//////////////////////////////////////////////////////////////////////
void CCommCardService::SetSpans(CS_IP_PARAMS_S &param,  CIPService *service, APIU32 &csIp)
{
	APIU32 ipTypeFromService		= (APIU32)( service->GetIpType() );
	APIU32 ipv6CfgTypeFromService	= (APIU32)( service->GetIpV6ConfigurationType() );

	CIPSpan *span = service->GetFirstSpan(); // the first span is for CS Module
	 
	if(NULL != span)
	{
		csIp = span->GetIPv4Address();	//get the csIp address 
		span = service->GetNextSpan();		
	}

	int i = 0;
	int spanIndex = 0;
	int numOfInterfaces = MAX_NUM_OF_BOARDS * MAX_NUM_OF_PQS;

	while(NULL != span && i < numOfInterfaces)
	{
		//pass the span already if it is configured
		BOOL ipV6AddressExist = FALSE;
		BOOL invalidState =  FALSE;
		
		if (service->GetIsV35GwEnabled() == FALSE)
		{
			for (int j=0; j<NUM_OF_IPV6_ADDRESSES; j++)
			{
				if ( !span->GetIsIpV6Null(j) )
				{
					ipV6AddressExist = TRUE;
					break;
				}
			}
		}
		
		if((eIpType_Both == service->GetIpType()))
		{
			if((span->GetIPv4Address()==0) && ipV6AddressExist)
			{
				invalidState = TRUE;
				/*for (int j=0; j<NUM_OF_IPV6_ADDRESSES; j++)
				{
					span->SetIPv6Address(j,"::");
				}*/

			}
		}
		if (span->GetIPv4Address() != 0 || (ipV6AddressExist && !invalidState))
		{
			param.interfacesList[spanIndex].ipType 			 = ipTypeFromService;
			param.interfacesList[spanIndex].iPv4.iPv4Address = span->GetIPv4Address();
			
			string sCurIPv6Address;
			for (int j=0; j<NUM_OF_IPV6_ADDRESSES; j++)
			{
				// IPv6 configuration type
				param.interfacesList[spanIndex].iPv6s[j].configurationType = ipv6CfgTypeFromService;
	
				// IPv6 addressses
				sCurIPv6Address = span->GetFullIPv6Address(j);

				if ( "::" != sCurIPv6Address)
				{
					strncpy( (char*)(param.interfacesList[spanIndex].iPv6s[j].iPv6Address),
							 sCurIPv6Address.c_str(),
							 IPV6_ADDRESS_LEN );
				}
			}
	
			param.interfacesList[spanIndex].boardId = CalcBoardId(i, MAX_NUM_OF_PQS);
			param.interfacesList[spanIndex].pqId	= CalcPQId(i, MAX_NUM_OF_PQS);
			
			spanIndex++;
		}				
		i++;
		span = service->GetNextSpan();
	}
}

//////////////////////////////////////////////////////////////////////
void CCommCardService::SetRouters(NETWORK_PARAMS_S &param, CIPService *service)
{	
	CH323Router *router = service->GetFirstRouter();
	int i = 0;
	while(NULL != router && i < MAX_ROUTERS_IN_H323_SERVICE)
	{
		param.ipRouter[i].routerIp		= router->GetRouterIP();
		param.ipRouter[i].remoteIp		= router->GetRemoteIP();
		param.ipRouter[i].remoteFlag	= router->GetRemoteFlag();
		param.ipRouter[i].subnetMask	= router->GetSubnetMask();
		
		i++;
		router = service->GetNextRouter();
	}	
}
