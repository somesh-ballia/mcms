// CommGKService.h: interface for the CCommGKService class.
//
//
//Date         Updated By         Description
//
//3/10/05	  Yuri Ratner		Communication with Gate Keaper
//========   ==============   =====================================================================


#ifndef COMMGKSERVICE_H_
#define COMMGKSERVICE_H_

#include "CommIPSListService.h"
#include "GKManagerStructs.h"


class CSegment;
class CIPService;


class CCommGKService : public CCommIPSListService
{
public:
	CCommGKService();
	virtual ~CCommGKService();
	
	const char * NameOf(void) const {return "CCommGKService";}
	
	virtual STATUS SendIpServiceParamInd(CIPService *service);
	virtual STATUS SendUpdateIpServiceParamInd(CIPService *service);
	virtual STATUS SendIpServiceParamEndInd();
	virtual STATUS SendDelIpService(CIPService *service);
	virtual STATUS SendServiceCfgList(CSegment *pSeg);
	
	STATUS ReceiveUpdatePropertiesReq(const GkManagerUpdateServicePropertiesReqStruct &param);
	STATUS ReceiveClearPropertiesReq(const ClearGkParamsFromPropertiesReqStruct &param);
	STATUS ReceiveIpInPropertiesReq(const SetGkIPInPropertiesReqStruct &param);
	STATUS ReceiveIdInPropertiesReq(const SetGkIdInPropertiesReqStruct &param);
	STATUS ReceiveNameInPropertiesReq(const SetGkNameInPropertiesReqStruct &param);	
	
	
private:
	bool FillParams(GkManagerServiceParamsIndStruct &param, CIPService *service);
	bool IsServiceReady(CIPService *service);

    STATUS UpdateQOS(const GkManagerUpdateServicePropertiesReqStruct &param);
    
};

#endif /*COMMGKSERVICE_H_*/
