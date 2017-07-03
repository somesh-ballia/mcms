// CommProxyService.h: interface for the CCommProxyService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with SipProxy
//========   ==============   =====================================================================

#ifndef __COMMPROXYSERVICE_H__
#define __COMMPROXYSERVICE_H__

#include "CommIPSListService.h"
#include "IpAddressDefinitions.h"

class CSipServer;
class CBaseSipServer;
class CSipServer;
class CSip;
class CSipAdvanced;
struct SIP_PROXY_IP_PARAMS_S;
struct SIP_PROXY_STATUS_PARAMS_S;

class CCommProxyService : public CCommIPSListService
{
public:
	CCommProxyService();
	virtual ~CCommProxyService();

	const char * NameOf(void) const {return "CCommProxyService";}
	virtual STATUS SendIpServiceParamInd(CIPService *service);
	virtual STATUS SendIpServiceParamEndInd();
	virtual STATUS SendDelIpService(CIPService *service);
	virtual STATUS SendServiceCfgList(CSegment *pSeg);

	STATUS SendCsNewIndication(WORD csID);
	STATUS ReceiveSipProxyStatusInd(const SIP_PROXY_STATUS_PARAMS_S &param);

private:
	bool FillParams(SIP_PROXY_IP_PARAMS_S &param, CIPService *service)const ;
	void SetSipAdvancedFields(SIP_PROXY_IP_PARAMS_S &param, const CSipAdvanced* pSipAdvanced)const;
	void SetAltRegistrar(SIP_PROXY_IP_PARAMS_S &param, const CSipServer *altRegistrar)const ;
	void SetIpV6(mcTransportAddress & outTransAddress, char *name)const;
	void SetProxy(SIP_PROXY_IP_PARAMS_S &param, const CBaseSipServer *proxy)const ;
	void SetRegistrar(SIP_PROXY_IP_PARAMS_S &param, const CSipServer *registrar)const ;
	void SetSipFields(SIP_PROXY_IP_PARAMS_S &param, const CSip *pSip)const ;
	bool CheckSipProxyApplicable(CIPService* service) const;
};

#endif // __COMMPROXYSERVICE_H__
