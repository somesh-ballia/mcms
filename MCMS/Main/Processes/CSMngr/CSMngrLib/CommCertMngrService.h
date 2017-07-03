// CommRsrcService.h: interface for the CCommRsrcService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with Resource Allocator
//========   ==============   =====================================================================


#ifndef _COMM_CERTMNGR_SERVICE_H_
#define _COMM_CERTMNGR_SERVICE_H_

#include "CommIPSListService.h"
#include "AllocateStructs.h"


class CCommCertMngrService : public CCommIPSListService
{
public:
	CCommCertMngrService();
	virtual ~CCommCertMngrService();
	
	const char * NameOf(void) const {return "CCommCertMngrService";}
	virtual STATUS SendIpServiceParamInd(CIPService *service);
	virtual STATUS SendIpServiceParamEndInd();
	virtual STATUS SendDelIpService(CIPService *service);
  	virtual STATUS SendServiceCfgList(CSegment *pSeg);
  
private:
	void FillParams(IP_SERVICE_CERTMNGR_S &param, CIPService *service);
};

#endif /*_COMMRSRCSERVICE_H_*/
