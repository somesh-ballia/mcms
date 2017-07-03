// CommIPSListService.cpp

#include "CommIPSListService.h"

#include "IpService.h"
#include "StatusesGeneral.h"
#include "DefinesGeneral.h"
#include "CSMngrStatuses.h"
#include "TraceStream.h"
#include "CommMcmsService.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"

static CServiceConfigList *pServiceConfigList = new CServiceConfigList;

CCommIPSListService::CCommIPSListService()
{}

CCommIPSListService::~CCommIPSListService()
{}

STATUS CCommIPSListService::SendIpServiceList()
{
	STATUS res = STATUS_NO_SERVICE;
	CIPServiceList *pList = GetIpServiceList();
	CIPService *service = pList->GetFirstService();

	while (NULL != service)
	{
		CheckForV35Services(service);
		res = SendIpServiceParamInd(service);

		// Only the first request will fill the map
		if (pServiceConfigList->IsServiceExists(service->GetId()) == FALSE)
		{
		    TRACEINTOFUNC << "Service does not exist, service ID "<< service->GetId();

			CServiceConfig *pCServiceConfig = service->GetServiceConfig();
			pCServiceConfig->SetId(service->GetId());			
			pServiceConfigList->Add(pCServiceConfig);
		}

		service = pList->GetNextService();
		SystemSleep(5);  //VNGR-18309,VNGR-18884
	}

	CSegment *pSegment = new CSegment;
	pServiceConfigList->Serialize(NATIVE, pSegment);
	SendServiceCfgList(pSegment);

	return res;
}
////////////////////////////////////////////////
void  CCommIPSListService::CheckForV35Services(CIPService *service)
{
	if(service->GetIsV35GwEnabled())
	{
		service->SetIpType(eIpType_IpV4);
	}
}


//////////////////////////////////////////////////////////////////////
DWORD CCommIPSListService::CalcBoardId(int index, int maxNumOfSpans)
{
	DWORD res = (int)(index / maxNumOfSpans) + 1;
	return res;
}

//////////////////////////////////////////////////////////////////////
DWORD CCommIPSListService::CalcPQId(int index, int maxNumOfSpans)
{
	DWORD res = (index % maxNumOfSpans) + 1;
	return res;
}
