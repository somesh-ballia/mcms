// CommDnsAgentService.cpp: implementation of the CCommDnsAgentService class.
//
//////////////////////////////////////////////////////////////////////

#include "CommDnsAgentService.h"

#include "Segment.h"
#include "DNSAgentManagerStructs.h"
#include "StatusesGeneral.h"
#include "WrappersDnsAgent.h"
#include "TraceStream.h" 
#include "OpcodesMcmsInternal.h"
#include "ObjString.h"
#include "ApiStatuses.h"
#include "DynIPSProperties.h"
#include "IpService.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommDnsAgentService::CCommDnsAgentService()
{

}

//////////////////////////////////////////////////////////////////////
CCommDnsAgentService::~CCommDnsAgentService()
{

}

//////////////////////////////////////////////////////////////////////
STATUS CCommDnsAgentService::ReceiveDnsResolution(const DNS_PARAMS_IP_S &param)
{
	CIPService *service = GetDynamicIpServiceIncListCnt(param.ServiceId);
	if(NULL == service)
	{
		CSmallString message = "Service Id was not found : ";
		message << param.ServiceId;
		PASSERTMSG(TRUE, message.GetString());
		return STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS;
	}
	TRACEINTO << CDnsParamsIpWrapper(param);
	
	CDynIPSProperties *pDynIpService = service->GetDynamicProperties();
	CDnsInfo &refDnsInfo = pDynIpService->GetDnsInfo();
	refDnsInfo.SetDnsResolutionParams(param);
	
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommDnsAgentService::SendDelIpService(CIPService *service)
{
	STATUS res = SendDelIpServiceToMcmsProcess(
		eProcessDNSAgent, 
		CS_DNS_AGENT_DELETE_IP_SERVICE_IND, 
		service);

	return res;
}

//////////////////////////////////////////////////////////////////////
