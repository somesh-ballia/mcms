// CSMngrMonitor.h: interface for the CCSMngrMonitor class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Manages monitoring 
//========   ==============   =====================================================================

#if !defined(_CCSMngrMONITOR__)
#define _CCSMngrMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CCSMngrMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CCSMngrMonitor();
	virtual ~CCSMngrMonitor();
	STATUS GetIpServiceList(CRequest* pGetRequest);	
	STATUS GetSystemInterfaceList(CRequest* pGetRequest);
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

private:
	STATUS OnFullServiceReq(CRequest*);
    STATUS GetPing (CRequest *pRequest);
};

#endif // !defined(_CCSMngrMONITOR__)

