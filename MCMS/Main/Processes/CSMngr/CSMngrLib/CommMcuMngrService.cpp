// CommRsrcService.cpp: implementation for the CCommRsrcService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with Resource Allocator
//========   ==============   =====================================================================

#include "CommMcuMngrService.h"
#include "IpService.h"
#include "OpcodesMcmsInternal.h"
#include "StatusesGeneral.h"
#include "CSMngrProcess.h"
#include "TraceStream.h"
#include "WrappersMcuMngr.h"

/*#include "Segment.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "WrappersResource.h"
#include "CSMngrStatuses.h"
#include "CSMngrProcess.h"*/

//////////////////////////////////////////////////////////////////////
CCommMcuMngrService::CCommMcuMngrService()
{
}

//////////////////////////////////////////////////////////////////////
CCommMcuMngrService::~CCommMcuMngrService()
{
}

//////////////////////////////////////////////////////////////////////
STATUS CCommMcuMngrService::SendIpServiceParamInd(CIPService *service)
{
	IP_SERVICE_MCUMNGR_S * param = new IP_SERVICE_MCUMNGR_S;
	memset(param, 0, sizeof(IP_SERVICE_MCUMNGR_S));
	FillParams(*param, service);
	
	TRACEINTO << CUDPMcuMngrWrapper(*param);

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)param, sizeof(IP_SERVICE_MCUMNGR_S));

	STATUS res = SendToMcmsProcess(eProcessMcuMngr, CS_MCUMNGR_IP_SERVICE_PARAM_IND, pSeg);

    delete param;
    
	return res;	
}
//////////////////////////////////////////////////////////////////////
STATUS CCommMcuMngrService::SendIpServiceParamEndInd()
{
	STATUS res = SendToMcmsProcess(eProcessMcuMngr, CS_MCUMNGR_IP_SERVICE_PARAM_END_IND, NULL);

	return res;	
}

//////////////////////////////////////////////////////////////////////
STATUS CCommMcuMngrService::SendDelIpService(CIPService *service)
{
	//TODO - add delete service, when needed
	STATUS res = SendDelIpServiceToMcmsProcess(eProcessMcuMngr, CS_MCUMNGR_DELETE_IP_SERVICE_IND, service);

	return res;
}

// Virtual
STATUS CCommMcuMngrService::SendServiceCfgList(CSegment *pSeg)
{
	/*TODO: maybe we need to send message to utility here.*/
	/*implement the virtual function avoid the memory leak:  David Liang -- 2012-05-08*/
	POBJDELETE(pSeg);
	return STATUS_OK;
}



//////////////////////////////////////////////////////////////////////
void CCommMcuMngrService::FillParams(IP_SERVICE_MCUMNGR_S &param, CIPService *service)
{
	param.ServId = service->GetId();
	param.dnsStatus = service->GetpDns()->GetStatus();


	TRACESTR(eLevelInfoNormal)
				<< "CCommMcuMngrService::FillParams "
				 << " ServId " << param.ServId ;

    BOOL isGW = service->GetIsV35GwEnabled();






	//param.ServId = service->GetId();
		//strncpy(param.ServName, service->GetName(), sizeof(param.ServName)-1);
		//param.ServName[sizeof(param.ServName)-1] = '\0';

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

		//CCommH323PortRange *portRange = NULL;

		// the first span == power quick is for cs module
		CIPSpan* pSpan = service->GetFirstSpan();
		if(NULL != pSpan)
		{
			//portRange = pSpan->GetPortRange();
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

			if (isGW == YES)
			{
				param.numPQSactual = 0;		// we dont want to send v35 parameters
				break;

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
					service->GetIpType() );


			//SetPortRange(param.IPServUDPperPQList[i], *portRange);

			i++;
			pSpan = service->GetNextSpan();

		}


	char curDefGW[IPV6_ADDRESS_LEN];
	service->GetDefaultGatewayIPv6(curDefGW);
	strcpy(param.DefaultGatewayIPv6, curDefGW);

	param.DefaultGatewayMaskIPv6 = service->GetDefaultGatewayMaskIPv6();
}
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CCommMcuMngrService::SetIpAddr( eIpType &ipType,
		                          ipAddressV4If &paramV4,
		                          ipAddressV6If *paramV6,
		                          CIPSpan *pSpan,
		                          eIpType theType )
{
	ipType = theType;

	// ===== 1. IPv4
	paramV4.ip = pSpan->GetIPv4Address();

	// ===== 2. IPv6
	char curIPv6Addr[IPV6_ADDRESS_LEN];
	mcTransportAddress tempTransAdd;
	for (int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(curIPv6Addr,		0, IPV6_ADDRESS_LEN);
		memset(&tempTransAdd,	0, sizeof(mcTransportAddress));

		pSpan->GetIPv6Address(i, curIPv6Addr);
		::stringToIp(&tempTransAdd, curIPv6Addr);
		memcpy(&paramV6[i].ip, &tempTransAdd.addr, IPV6_ADDRESS_BYTES_LEN);

		paramV6[i].scopeId = ::getScopeId(curIPv6Addr);
	}
}
