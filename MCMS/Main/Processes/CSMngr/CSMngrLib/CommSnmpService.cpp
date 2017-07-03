// CommSnmpService.cpp

#include "CommSnmpService.h"
#include "SNMPStructs.h"
#include "Segment.h"
#include "IpService.h"
#include "OpcodesMcmsInternal.h"
#include "WrappersSnmp.h"
#include "TraceStream.h"

CCommSnmpService::CCommSnmpService()
{ }

CCommSnmpService::~CCommSnmpService()
{ }

STATUS CCommSnmpService::SendIpServiceParamInd(CIPService* service)
{
  SNMP_CS_INFO_S param;

  FillParam(param, service);

  CSegment* pSeg = new CSegment;
  pSeg->Put((BYTE*)&param, sizeof(SNMP_CS_INFO_S));

  STATUS res = SendToMcmsProcess(eProcessSNMPProcess,
                                 SNMP_CS_INTERFACE_IP_IND,
                                 pSeg);

  return res;
}

void CCommSnmpService::FillParam(SNMP_CS_INFO_S& outParam,
                                 CIPService* service) const
{
  // CS ip
  CIPSpan* pSpan = service->GetSpanByIdx(0);
  PASSERTMSG_AND_RETURN(NULL == pSpan, "No CS span in the service");

  outParam.type = service->GetIPProtocolType();
  outParam.csIp = pSpan->GetIPv4Address();

  // GK ip
  if (FALSE == service->IsContainGK())
    return;

  CDynIPSProperties* dynService = service->GetDynamicProperties();
  PASSERTMSG_AND_RETURN(NULL == dynService, "No Dynamic properties in service");

  CH323Info&         gkInfo    = dynService->GetCSGKInfo();
  CProxyDataContent& primaryGK = gkInfo.GetPrimaryGk();
  outParam.gkIp                = primaryGK.GetIPv4Address();
}

