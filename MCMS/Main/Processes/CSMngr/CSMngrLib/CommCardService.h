// CommCardService.h: interface for the CCommCardService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with Cards
//========   ==============   =====================================================================

#ifndef __COMMCARDSERVICE_H__
#define __COMMCARDSERVICE_H__


#include "CommIPSListService.h"
#include "SharedMcmsCardsStructs.h"

class CIPService;
class CIpDns;



class CCommCardService : public CCommIPSListService  
{
public:
	CCommCardService();
	virtual ~CCommCardService();	

	const char * NameOf(void) const {return "CCommCardService";}
	virtual STATUS SendIpServiceParamInd(CIPService *service);
	virtual STATUS SendIpServiceParamEndInd();	
	virtual STATUS SendDelIpService(CIPService *service);
	virtual STATUS SendServiceCfgList(CSegment *pSeg);
	
	STATUS ReceiveMediaIpConfigInd(CSegment*, int& serviceId);
	void   SetMpmIPv6AddressesIfNeeded(const CS_MEDIA_IP_CONFIG_S *pParam, const int serviceId);
//	void   GetIPv6AddressGlobalFromStruct(const CS_MEDIA_IP_CONFIG_S *pParam, char *pOutAddr);
	void   SetMPMIPv6Addresses(DWORD boardId, const CS_MEDIA_IP_CONFIG_S *pParam, const int serviceId);
	int    GetFirstSpanNumberForMPM(int boardId);
	CS_MEDIA_IP_PARAMS_S m_cs_media_ip_params;
	IP_INTERFACE_SHORT_S       m_shortInterfacesListForUtilityProcess[MAX_NUM_OF_BOARDS * MAX_NUM_OF_PQS];
	DWORD m_numOfElem ;
	STATUS SendIpServiceParamUtility();
	

private:
	void FillParams(CS_MEDIA_IP_PARAMS_S &param, CIPService *service);
	void SetNetwork(NETWORK_PARAMS_S &param, CIPService *service);
	void SetMediaParams(CS_MEDIA_IP_PARAMS_S &param, CIPService *service);
	void SetDns(DNS_CONFIGURATION_S &param, CIpDns *dns);
	void SetSpans(CS_IP_PARAMS_S &param,  CIPService *service, APIU32 &csIp);
	void SetSecondaryNetwork(CS_IP_PARAMS_S &param,  CIPService *service);
	void SetRouters(NETWORK_PARAMS_S &param, CIPService *service);

};

#endif // __COMMCARDSERVICE_H__
