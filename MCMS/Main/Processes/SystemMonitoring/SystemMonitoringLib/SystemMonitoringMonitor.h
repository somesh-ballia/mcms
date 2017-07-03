// SystemMonitoringMonitor.h: interface for the CSystemMonitoringMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CSystemMonitoringMONITOR__)
#define _CSystemMonitoringMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CSystemMonitoringMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CSystemMonitoringMonitor();
	virtual ~CSystemMonitoringMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CSystemMonitoringMONITOR__)
