// RtmIsdnMngrMonitor.h: interface for the CRtmIsdnMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CRtmIsdnMngrMONITOR__)
#define _CRtmIsdnMngrMONITOR__

#include "MonitorTask.h"
//#include "Macros.h"



class CRtmIsdnMngrMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CRtmIsdnMngrMonitor();
	virtual ~CRtmIsdnMngrMonitor();

	STATUS OnGetServiceList(CRequest* pGetRequest);	
	STATUS OnGetSpansList(CRequest* pGetRequest);


protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CRtmIsdnMngrMONITOR__)
