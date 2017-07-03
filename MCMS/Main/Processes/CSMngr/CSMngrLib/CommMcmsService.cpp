// CommMcmsService.cpp

#include "CommMcmsService.h"

#include "Segment.h"
#include "IpService.h"
#include "ManagerApi.h"
#include "ProcessBase.h"
#include "TraceStream.h"
#include "CSMngrProcess.h"
#include "CsCommonStructs.h"
#include "IPServiceDynamicList.h"

CCommMcmsService::CCommMcmsService()
{}

// Virtual
const char* CCommMcmsService::NameOf(void) const
{
  return GetCompileType();
}

STATUS CCommMcmsService::SendToMcmsProcess(eProcessType dest,
                                           OPCODE opcode,
                                           CSegment* data) const
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN_VALUE(NULL == proc, STATUS_FAIL);

  STATUS stat = CManagerApi(dest).SendMsg(data, opcode);
  if (STATUS_OK == stat)
  {
    TRACEINTOFUNC << "Request " << proc->GetOpcodeAsString(opcode)
                  << " (" << opcode << ") to " << ProcessNames[dest]
                  << " was sent successfully";
  }
  else
  {
    TRACEINTOFUNC << "WARNING: Unable to send " << proc->GetOpcodeAsString(opcode)
                  << " (" << opcode << ") to " << ProcessNames[dest]
                  << ": " << proc->GetStatusAsString(stat);
  }

  return stat;
}

STATUS CCommMcmsService::SendDelIpServiceToMcmsProcess(eProcessType dest,
                                                       OPCODE opcode,
                                                       CIPService *service)
{
	Del_Ip_Service_S param;
	param.service_id = service->GetId();
	strncpy(param.service_name, service->GetName(), sizeof(param.service_name)-1);
	param.service_name[sizeof(param.service_name)-1] = '\0';

	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)&param,sizeof(Del_Ip_Service_S));

	STATUS res = SendToMcmsProcess(dest, opcode, pSeg);

	return res;
}

CIPService* CCommMcmsService::GetDynamicIpServiceIncListCnt(WORD serviceId)const
{
	CIPServiceFullList *dynPart =  m_pCSProcess->GetXMLWraper();
	dynPart->IncreaseUpdateCounter();

	CIPServiceList *list = m_pCSProcess->GetIpServiceListDynamic();
	CIPService *service = list->GetService(serviceId);

	return service;
}
