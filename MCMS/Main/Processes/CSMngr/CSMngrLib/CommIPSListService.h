// CommIPSListService.h: interface for the CCommIPSListService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Base class for all classes who need to send a list of IpServices
//========   ==============   =====================================================================


#ifndef _COMMIPSLISTSERVICE_H_
#define _COMMIPSLISTSERVICE_H_

#include "CommMcmsService.h"
#include "ServiceConfigList.h"


class CIPService;

class CCommIPSListService : public CCommMcmsService
{
public:
	CCommIPSListService();
	virtual ~CCommIPSListService();
	
	STATUS SendIpServiceList();
	
protected:	
	virtual STATUS SendIpServiceParamInd(CIPService *service) = 0;
	virtual STATUS SendIpServiceParamEndInd() = 0;
	virtual STATUS SendServiceCfgList(CSegment *pSeg){return 0;};
	void  CheckForV35Services(CIPService *service);
	DWORD CalcBoardId(int index, int maxNumOfSpans);
	DWORD CalcPQId(int index, int maxNumOfSpans);

};

#endif /*_COMMIPSLISTSERVICE_H_*/
