// CommRsrcService.cpp: implementation for the CCommRsrcService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with Resource Allocator
//========   ==============   =====================================================================

#include "CommRsrcService.h"
#include "IpService.h"
#include "Segment.h"
#include "OpcodesMcmsInternal.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "WrappersResource.h"
#include "CSMngrStatuses.h"
#include "CSMngrProcess.h"

//////////////////////////////////////////////////////////////////////
CCommRsrcService::CCommRsrcService()
{
}

//////////////////////////////////////////////////////////////////////
CCommRsrcService::~CCommRsrcService()
{
}

//////////////////////////////////////////////////////////////////////
STATUS CCommRsrcService::SendIpServiceParamInd(CIPService *service)
{
	IP_SERVICE_UDP_RESOURCES_S * param = new IP_SERVICE_UDP_RESOURCES_S;
	memset(param, 0, sizeof(IP_SERVICE_UDP_RESOURCES_S));
	FillParams(*param, service);
	
	TRACEINTO << CUDPResourceWrapper(*param);
	
	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)param, sizeof(IP_SERVICE_UDP_RESOURCES_S));

	STATUS res = SendToMcmsProcess(eProcessResource, CS_RSRC_IP_SERVICE_PARAM_IND, pSeg);

    delete param;
    
	return res;	
}
//////////////////////////////////////////////////////////////////////
void CCommRsrcService::SendUpdateIpV6ParamReq(const int serviceId)
{
	CIPServiceList *pList = GetIpServiceList();
	IPV6_ADDRESS_UPDATE_RESOURCES_S param;
	memset(&param, 0, sizeof(IPV6_ADDRESS_UPDATE_RESOURCES_S));
	
	CIPService* pService = pList->GetService(serviceId);
	if(pService)
	{
		FillIpV6Params(param, pService);

		TRACEINTO << CIpV6AddressUpdateResourcesWrapper(param);

		CSegment *pSeg = new CSegment;
		pSeg->Put((BYTE*)&param, sizeof(IPV6_ADDRESS_UPDATE_RESOURCES_S));

		SendToMcmsProcess(eProcessResource, CS_RSRC_UPDATE_IPV6_SERVICE_PARAM_REQ, pSeg);
	}
	else
		PASSERT(1);
}
//////////////////////////////////////////////////////////////////////
STATUS CCommRsrcService::SendIpServiceParamEndInd()
{
	STATUS res = SendToMcmsProcess(eProcessResource, CS_RSRC_IP_SERVICE_PARAM_END_IND, NULL);

	return res;	
}

//////////////////////////////////////////////////////////////////////
STATUS CCommRsrcService::SendDelIpService(CIPService *service)
{
	STATUS res = SendDelIpServiceToMcmsProcess(eProcessResource, CS_RSRC_DELETE_IP_SERVICE_IND, service);

	return res;
}

// Virtual
STATUS CCommRsrcService::SendServiceCfgList(CSegment *pSeg)
{
	/*TODO: maybe we need to send message to utility here.*/
	/*implement the virtual function avoid the memory leak:  David Liang -- 2012-05-08*/
	POBJDELETE(pSeg);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
STATUS CCommRsrcService::SendDefaultService()
{
	CIPServiceList *list = m_pCSProcess->GetIpServiceListStatic();

	DEFAULT_IP_SERVICE_S param;

  strncpy(param.defaultSIPServiceName, list->GetSIPDefaultName(), sizeof(param.defaultSIPServiceName)-1);
  param.defaultSIPServiceName[sizeof(param.defaultSIPServiceName)-1] = '\0';

  strncpy(param.defaultH323ServiceName, list->GetH323DefaultName(), sizeof(param.defaultH323ServiceName)-1);
  param.defaultH323ServiceName[sizeof(param.defaultH323ServiceName)-1] = '\0';

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&param, sizeof(DEFAULT_IP_SERVICE_S));

	STATUS res = SendToMcmsProcess(eProcessResource, CS_RSRC_DEFAULT_SERVICE_IND, pSeg);

	return res;
}
//////////////////////////////////////////////////////////////////////
void CCommRsrcService::FillParams(IP_SERVICE_UDP_RESOURCES_S &param, CIPService *service)
{
	param.ServId = service->GetId();
	strncpy(param.ServName, service->GetName(), sizeof(param.ServName)-1);
	param.ServName[sizeof(param.ServName)-1] = '\0';

	WORD num = service->GetSpansNumber();
	if(0 == num)
	{
		PASSERTMSG(1, "No PQ for CS Module");
	}
	else
	{
		// PowerQuick == PQ == Span
		param.numPQSactual = num - 1; // check if zero
	}
	
	CCommH323PortRange *portRange = NULL;
	
	// the first span == power quick is for cs module
	CIPSpan* pSpan = service->GetFirstSpan();
	if(NULL != pSpan)
	{
		portRange = pSpan->GetPortRange();
		pSpan = service->GetNextSpan();		
	}
	
	int i = 0;
	while(NULL != pSpan)
	{
	    if (i >= MAX_NUM_PQS)
	    {
		  PASSERTMSG(i, "PQS overflow");
		  param.numPQSactual = i;		// to send the right number
		  return; 
		}
        
		param.IPServUDPperPQList[i].boardId    = CalcBoardId(i, MAX_NUM_OF_PQS);
		eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
		if (eProductTypeGesher == curProductType || eProductTypeNinja == curProductType)
		{
			//for gesher/ninja, the maximum ip service should be 3 and the board id should always be 1.
			param.IPServUDPperPQList[i].boardId    = CalcBoardId(i, 3);
		}
		param.IPServUDPperPQList[i].PQid 	   = CalcPQId(i, MAX_NUM_OF_PQS);

		// subBoardId is hard coded
		param.IPServUDPperPQList[i].subBoardId = 1;
		
/* -- IpV6 With Rsrc -- */
		SetIpAddr( param.IPServUDPperPQList[i].IpType,
				   param.IPServUDPperPQList[i].IpV4Addr,
				   param.IPServUDPperPQList[i].IpV6Addr,
				   pSpan,
				   service->GetIpType(),service->GetIpV6ConfigurationType() );
	
		SetPortRange(param.IPServUDPperPQList[i], *portRange);
		
		i++;
		pSpan = service->GetNextSpan();
	}
	param.iceEnvironment = service->GetpSipAdvanced()->GetIceEnvironment();
	
	CIPServiceList *list = m_pCSProcess->GetIpServiceListStatic();
	param.service_default_type = service->GetServiceDefaultType(list->GetH323DefaultName(), list->GetSIPDefaultName());
}
//////////////////////////////////////////////////////////////////////
void CCommRsrcService::FillIpV6Params(IPV6_ADDRESS_UPDATE_RESOURCES_S &param, CIPService *service)
{
	param.ServId = service->GetId();

	// the first span == power quick is for cs module
	CIPSpan* pSpan = service->GetFirstSpan();
	if(NULL != pSpan)
		pSpan = service->GetNextSpan();		
	
	int span_index = 0;
	while(NULL != pSpan)
	{
        if (span_index >= MAX_NUM_PQS)
        {
        	PASSERTMSG(span_index, "PQS overflow");
        	return; 
		}
        
        BOOL ipv6Empty = FALSE;
        // 		VNGR-26361
        if(eIpType_Both == service->GetIpType())
        {
        	if(0==pSpan->GetIPv4Address())
        	{
        		ipv6Empty = TRUE;
        	}
        }
		param.IPServUDPperPQList[span_index].boardId    = CalcBoardId(span_index, MAX_NUM_OF_PQS);
		param.IPServUDPperPQList[span_index].PQid 	   = CalcPQId(span_index, MAX_NUM_OF_PQS);

		// subBoardId is hard coded
		param.IPServUDPperPQList[span_index].subBoardId = 1;

		char curIPv6Addr[IPV6_ADDRESS_LEN];
		mcTransportAddress tempTransAdd;
		
		for (int j=0; j<NUM_OF_IPV6_ADDRESSES; j++)
		{
			memset(curIPv6Addr,		0, IPV6_ADDRESS_LEN);
			memset(&tempTransAdd,	0, sizeof(mcTransportAddress));
			
			if(ipv6Empty)
			{
					strncpy(curIPv6Addr ,"::",IPV6_ADDRESS_LEN );

			}
			else
			{
				pSpan->GetIPv6Address(j, curIPv6Addr);
			}
			::stringToIp(&tempTransAdd, curIPv6Addr);
			memcpy(param.IPServUDPperPQList[span_index].IpV6Addr[j].ip, &tempTransAdd.addr, IPV6_ADDRESS_BYTES_LEN);
			
			param.IPServUDPperPQList[span_index].IpV6Addr[j].scopeId = ::getScopeId(curIPv6Addr);
		}
		
		span_index++;
		pSpan = service->GetNextSpan();
	}	
}
//////////////////////////////////////////////////////////////////////
void CCommRsrcService::SetIpAddr( eIpType &ipType,
		                          ipAddressV4If &paramV4, 
		                          ipAddressV6If *paramV6,
		                          CIPSpan *pSpan,
		                          eIpType theType,
		                          eV6ConfigurationType v6configType)
{
	ipType = theType;
	
	// ===== 1. IPv4
	paramV4.ip = pSpan->GetIPv4Address();
	BOOL ipv6Empty = FALSE;

	if((eIpType_Both == theType))
	{
		if(0==pSpan->GetIPv4Address())
		{
			ipv6Empty = TRUE;
		}
	}
	// ===== 2. IPv6
	char curIPv6Addr[IPV6_ADDRESS_LEN];
	mcTransportAddress tempTransAdd;
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(curIPv6Addr,		0, IPV6_ADDRESS_LEN);
		memset(&tempTransAdd,	0, sizeof(mcTransportAddress));


		if(ipv6Empty)
		{
			strncpy(curIPv6Addr ,"::",IPV6_ADDRESS_LEN );

		}
		else
		{
			pSpan->GetIPv6Address(i, curIPv6Addr);
		}
		::stringToIp(&tempTransAdd, curIPv6Addr);
		memcpy(&paramV6[i].ip, &tempTransAdd.addr, IPV6_ADDRESS_BYTES_LEN);

		paramV6[i].scopeId = ::getScopeId(curIPv6Addr);
	}
}

//////////////////////////////////////////////////////////////////////
void CCommRsrcService::SetPortRange(IP_SERVICE_UDP_RESOURCE_PER_PQ_S &param, const CCommH323PortRange &portRange)
{
  WORD first = portRange.GetUdpFirstPort();
  WORD last = first + portRange.GetUdpNumberOfPorts();

  param.UdpFirstPort = first;
  param.UdpLastPort = last;

  param.portsAlloctype = (TRUE == portRange.IsEnabledPortRange() ? eMethodStatic : eMethodDynamic);
}

