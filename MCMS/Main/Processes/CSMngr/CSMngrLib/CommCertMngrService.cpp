// CommRsrcService.cpp: implementation for the CCommRsrcService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with Resource Allocator
//========   ==============   =====================================================================

#include "CommCertMngrService.h"
#include "IpService.h"
#include "Segment.h"
#include "OpcodesMcmsInternal.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "WrappersResource.h"
#include "CSMngrStatuses.h"
#include "CSMngrProcess.h"

//////////////////////////////////////////////////////////////////////
CCommCertMngrService::CCommCertMngrService()
{
}

//////////////////////////////////////////////////////////////////////
CCommCertMngrService::~CCommCertMngrService()
{
}

//////////////////////////////////////////////////////////////////////
STATUS CCommCertMngrService::SendIpServiceParamInd(CIPService *service)
{
	IP_SERVICE_CERTMNGR_S * param = new IP_SERVICE_CERTMNGR_S;
	memset(param, 0, sizeof(IP_SERVICE_CERTMNGR_S));
	FillParams(*param, service);
	
//TBD	TRACEINTO << CUDPResourceWrapper(*param);
	
	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)param, sizeof(IP_SERVICE_CERTMNGR_S));

	STATUS res = SendToMcmsProcess(eProcessCertMngr, CS_CERTMNGR_IP_SERVICE_PARAM_IND, pSeg);

    delete param;
    
	return res;	
}
//////////////////////////////////////////////////////////////////////
STATUS CCommCertMngrService::SendIpServiceParamEndInd()
{
	STATUS res = SendToMcmsProcess(eProcessCertMngr, CS_CERTMNGR_IP_SERVICE_PARAM_END_IND, NULL);

	return res;	
}

//////////////////////////////////////////////////////////////////////
STATUS CCommCertMngrService::SendDelIpService(CIPService *service)
{
	//TODO - add delete service, when needed
	STATUS res = SendDelIpServiceToMcmsProcess(eProcessCertMngr, CS_CERTMNGR_DELETE_IP_SERVICE_IND, service);

	return res;
}

// Virtual
STATUS CCommCertMngrService::SendServiceCfgList(CSegment *pSeg)
{
	/*TODO: maybe we need to send message to utility here.*/
	/*implement the virtual function avoid the memory leak:  David Liang -- 2012-05-08*/
	POBJDELETE(pSeg);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
void CCommCertMngrService::FillParams(IP_SERVICE_CERTMNGR_S &param, CIPService *service)
{
  param.ServId = service->GetId();
  strncpy(param.ServName, service->GetName(), sizeof(param.ServName)-1);
  param.ServName[sizeof(param.ServName)-1] = '\0';
  param.IsSecured = (service->GetSip()->GetTransportType() == eTransportTypeTls);
  strncpy(param.HostName, GetHostNameFromService(service).c_str(), sizeof(param.HostName)-1);
  param.HostName[sizeof(param.HostName)-1] = '\0';
  param.IsRequestPeerCertificate = service->GetManagementSecurity()->IsRequestPeerCertificate();
  param.Revocation_method = service->GetManagementSecurity()->GetRevocationMethodType();
  std::string url = service->GetManagementSecurity()->GetOCSPGlobalResponderURI();
  if(!url.empty() && (url.size() < _0_TO_IP_LIMIT_ADDRESS_CHAR_LENGTH))
  {
	  strncpy(param.OcspURL,url.c_str(),sizeof(param.OcspURL)-1);
	  param.OcspURL[sizeof(param.OcspURL)-1]='\0';
  }

  if((service->GetSip()->GetConfigurationOfSIPServers())&&
		  (eSipServer_ms == service->GetSip()->GetSipServerType()))
  {
	  param.IsMsService = TRUE;
  }
  else
	  param.IsMsService = FALSE;


}
//////////////////////////////////////////////////////////////////////

