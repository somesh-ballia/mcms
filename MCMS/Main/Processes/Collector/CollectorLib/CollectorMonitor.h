// CollectorMonitor.h: interface for the CCollectorMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CCollectorMONITOR__)
#define _CCollectorMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CCollectorMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CCollectorMonitor();
	virtual ~CCollectorMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

private:
	STATUS OnGetCollectInfoSettings(CRequest *pGetRequest);
//	STATUS GetCollectInfoEstimatedSize(CRequest *pRequest);

};

#endif // !defined(_CCollectorMONITOR__)
