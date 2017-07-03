// GatekeeperMonitor.h: interface for the CGatekeeperMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CGKMONITOR__)
#define _CGKMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CGKMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CGKMonitor();
	virtual ~CGKMonitor();

	virtual const char* NameOf() const { return "CGKMonitor";}
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CGKMONITOR__)

