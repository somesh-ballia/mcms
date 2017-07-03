// CommConfService.h: interface for the CCommConfService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with Conf-Party
//========   ==============   =====================================================================

#ifndef __COMMCONFSERVICE_H__
#define __COMMCONFSERVICE_H__


#include "CommIPSListService.h"
#include "CommonStructs.h"

class CIPService;
class CBaseSipServer;
class CIPSpan;
class CSipServer;
class CQualityOfService;
class CSip;
class CIpDns;
class CH323Alias;
struct CONF_IP_PARAMS_S;
struct SIP_S;
struct QOS_S;
struct BASE_SIP_SERVER_S;
struct SIP_SERVER_S;
class CSipAdvanced;


class CCommConfService : public CCommIPSListService  
{
public:
	CCommConfService();
	virtual ~CCommConfService();

	const char * NameOf(void) const {return "CCommConfService";}
	virtual STATUS SendIpServiceParamInd(CIPService *service);
	virtual STATUS SendIpServiceParamEndInd();
	virtual STATUS SendDelIpService(CIPService *service);
	STATUS SendDefaultService();
	virtual STATUS SendServiceCfgList(CSegment *pSeg);

	void  		   SendReleaseMsgReq() const;
	void FillParam(CONF_IP_PARAMS_S &param, CIPService *service)const ;
private:

	void SetGeneralParams(CONF_IP_PARAMS_S &param, CIPService *service)const ;
	void SetSpan(CONF_IP_PARAMS_S &param, CIPSpan *span, eIpType theType,eV6ConfigurationType v6configType)const ;
    void SetQos(QOS_S &qos, const CQualityOfService *pQos)const ;
	void SetSip(SIP_S &param, const CSip *sip, const CSipAdvanced* pSipAdvanced)const ;
	void SetBaseSipServer(BASE_SIP_SERVER_S &param, const CBaseSipServer *baseSipServer)const ;
	void SetSipServer(SIP_SERVER_S &param, const CSipServer *sipServer)const ;
	void SetDns(CONF_IP_PARAMS_S &param, const CIpDns *pDns)const ;
	void SetAlias(ALIAS_S &param, const CH323Alias *alias)const;
};

#endif // __COMMCONFSERVICE_H__
