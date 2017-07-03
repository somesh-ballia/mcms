// FailoverMonitor.h: interface for the CFailoverMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CFailoverMONITOR__)
#define _CFailoverMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CFailoverMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CFailoverMonitor();
	virtual ~CFailoverMonitor();
	STATUS OnServerGetFailoverConfiguration(CRequest* pGetRequest);
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CFailoverMONITOR__)
