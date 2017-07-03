// CommGKService.h: interface for the CCommGKService class.
//
//
//Date         Updated By         Description
//
//07/02/11	  Judith			Communication with McuMngr
//========   ==============   =====================================================================


#ifndef COMM_MCU_MNGR_SERVICE_H_
#define COMM_MCU_MNGR_SERVICE_H_

#include "CommIPSListService.h"
#include "AllocateStructs.h"

class CIPService;
class CIPSpan;

class CCommMcuMngrService : public CCommIPSListService
{
public:
	CCommMcuMngrService();
	virtual ~CCommMcuMngrService();
	
	const char * NameOf(void) const {return "CCommMcuMngrService";}
	virtual STATUS SendIpServiceParamInd(CIPService *service);
	virtual STATUS SendIpServiceParamEndInd();
	virtual STATUS SendDelIpService(CIPService *service);
	virtual STATUS SendServiceCfgList(CSegment *pSeg);

private:
	void FillParams(IP_SERVICE_MCUMNGR_S &param, CIPService *service);
	void SetIpAddr( eIpType &ipType, ipAddressV4If &paramV4, ipAddressV6If *paramV6, CIPSpan *pSpan, eIpType theType);
};

#endif /*COMM_MCU_MNGR_SERVICE_H_*/
