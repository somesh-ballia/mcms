// CommRsrcService.h: interface for the CCommRsrcService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with Resource Allocator
//========   ==============   =====================================================================


#ifndef _COMMRSRCSERVICE_H_
#define _COMMRSRCSERVICE_H_

#include "CommIPSListService.h"
#include "AllocateStructs.h"


class CCommH323PortRange;
class CIPSpan;

class CCommRsrcService : public CCommIPSListService
{
public:
	CCommRsrcService();
	virtual ~CCommRsrcService();
	
	const char * NameOf(void) const {return "CCommRsrcService";}
	virtual STATUS SendIpServiceParamInd(CIPService *service);
	virtual STATUS SendIpServiceParamEndInd();
	virtual STATUS SendServiceCfgList(CSegment *pSeg);
	virtual STATUS SendDelIpService(CIPService *service);
	STATUS SendDefaultService();
	void SendUpdateIpV6ParamReq(const int serviceId);
  
private:
	void FillParams(IP_SERVICE_UDP_RESOURCES_S &param, CIPService *service);
	void FillIpV6Params(IPV6_ADDRESS_UPDATE_RESOURCES_S &param, CIPService *service);
//	void SetIpAddr(ipAddressIf &param, const CIPSpan &span);
	void SetIpAddr( eIpType &ipType, ipAddressV4If &paramV4, ipAddressV6If *paramV6, CIPSpan *pSpan, eIpType theType,eV6ConfigurationType v6configType);
	void SetPortRange(IP_SERVICE_UDP_RESOURCE_PER_PQ_S &param, const CCommH323PortRange &portRange);

};

#endif /*_COMMRSRCSERVICE_H_*/
