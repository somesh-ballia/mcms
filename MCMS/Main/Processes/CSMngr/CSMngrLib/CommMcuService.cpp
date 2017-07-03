// CommMcuService.cpp: implementation of the CCommMcuService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with McuMngr
//========   ==============   =====================================================================

#include "CommMcuService.h"
#include "CsCommonStructs.h"
#include "TraceStream.h"
#include "WrappersMcuMngr.h"
#include "McuMngrInternalStructs.h"
#include "OpcodesMcmsInternal.h"
#include "CSMngrProcess.h"
#include "IpService.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCommMcuService::CCommMcuService()
{

}

CCommMcuService::~CCommMcuService()
{

}


STATUS CCommMcuService::SendNumPortReq()
{
	STATUS res = SendToMcmsProcess(eProcessMcuMngr, CS_MCU_NUM_OF_PORTS_REQ, NULL);

	return res;
}

//////////////////////////////////////////////////////////////////////
STATUS CCommMcuService::SendDelIpService(CIPService *service)
{
	STATUS res = SendDelIpServiceToMcmsProcess(eProcessMcuMngr, CS_MCU_DELETE_IP_SERVICE_IND, service);

	return res;
}

//////////////////////////////////////////////////////////////////////
/*STATUS CCommMcuService::SendDnsHostRegictrationRequest()
{
    STATUS res = SendToMcmsProcess(eProcessMcuMngr, DNS_HOST_REGISTRATION_REQ, NULL);
    
    return res;
}*/

//////////////////////////////////////////////////////////////////////
STATUS CCommMcuService::SendIpTypeRequest()
{
    STATUS res = SendToMcmsProcess(eProcessMcuMngr, CSMNGR_TO_MCUMNGR_IP_TYPE_REQ, NULL);
    
    return res;
}

//////////////////////////////////////////////////////////////////////
void CCommMcuService::ReceiveNumOfPortsInd(const CSMNGR_LICENSING_S *pParam)const
{
	TRACEINTO << CNumOfPortsIndWrapper(*pParam);
//	PASSERTMSG(pParam->numOfPorts == 0, "McuMngr sent 0 num of cp ports. Probably the service is not valid");
}

//////////////////////////////////////////////////////////////////////
void CCommMcuService::ReceiveDnsHostRegistration(const DNS_HOST_REGISTRATION_S *pParam)const
{
	CIPServiceList *serviceList = m_pCSProcess->GetIpServiceListDynamic();
	CIPService *service = serviceList->GetFirstService();
    while(NULL != service)
    {
    	CIpDns *pDns = service->GetpDns();
		if(NULL != pDns)
		{
			pDns->SetStatus((eServerStatus)(pParam->serverStatus)), 
	    	pDns->SetRegisterDNSAutomatically(pParam->isRegistrationAuto);
	    	pDns->SetDomainName(pParam->domainName);
	    	pDns->SetHostServiceName(pParam->hostName);
		}
    	service = serviceList->GetNextService();	
    }
}






