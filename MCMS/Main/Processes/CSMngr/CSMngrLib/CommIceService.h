// CommIceService.h: interface for the CCommIceService class.
//
//
//Date         Updated By         Description
//
//24/04/14	  Boris Sirotin		Communication with Ice
//========   ==============   =====================================================================

#ifndef __COMMICESERVICE_H__
#define __COMMICESERVICE_H__

#include "CommIPSListService.h"
#include "IpAddressDefinitions.h"

class CSipServer;
class CBaseSipServer;
class CSipServer;
class CSip;
class CSipAdvanced;
struct SIP_PROXY_IP_PARAMS_S;
struct SIP_PROXY_STATUS_PARAMS_S;

class CCommIceService : public CCommIPSListService
{
public:
	CLASS_TYPE_1(CCommIceService, CCommIPSListService)
	CCommIceService();
	virtual ~CCommIceService();

	const char * NameOf(void) const {return "CCommIceService";}
	virtual STATUS SendIpServiceParamInd(CIPService *service);
	virtual STATUS SendIpServiceParamEndInd();
	virtual STATUS SendDelIpService(CIPService *service);
	virtual STATUS SendServiceCfgList(CSegment *pSeg);

	//STATUS SendCsNewIndication(WORD csID);
	//STATUS ReceiveIceStatusInd(const SIP_PROXY_STATUS_PARAMS_S &param);

private:
	bool FillParams(SIP_PROXY_IP_PARAMS_S &param, CIPService *service)const ;
	void SetSipAdvancedFields(SIP_PROXY_IP_PARAMS_S &param, const CSipAdvanced* pSipAdvanced)const;
	//void SetAltRegistrar(SIP_PROXY_IP_PARAMS_S &param, const CSipServer *altRegistrar)const ;
	void SetIpV6(mcTransportAddress & outTransAddress, char *name)const;
	void SetProxy(SIP_PROXY_IP_PARAMS_S &param, const CBaseSipServer *proxy)const ;
	void SetRegistrar(SIP_PROXY_IP_PARAMS_S &param, const CSipServer *registrar)const ;
	void SetSipFields(SIP_PROXY_IP_PARAMS_S &param, const CSip *pSip)const ;
	bool CheckSipProxyApplicable(CIPService* service) const;
	bool CheckWebRtcIceApplicable(CIPService* service) const;
};

#endif // __COMMICESERVICE_H__
