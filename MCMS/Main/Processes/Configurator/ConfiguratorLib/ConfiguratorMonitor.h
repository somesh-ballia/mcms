// ConfiguratorMonitor.h: interface for the CConfiguratorMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CConfiguratorMONITOR__)
#define _CConfiguratorMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CConfiguratorMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CConfiguratorMonitor();
	virtual ~CConfiguratorMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CConfiguratorMONITOR__)
