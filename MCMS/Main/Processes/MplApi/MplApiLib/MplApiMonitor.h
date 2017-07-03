// MplApiMonitor.h: interface for the MplApiMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MPLAPIMONITOR__)
#define _MPLAPIMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CMplApiMonitor : public CMonitorTask
{
CLASS_TYPE_1(CMplApiMonitor,CMonitorTask )
public:
	CMplApiMonitor();
	virtual ~CMplApiMonitor();

	virtual const char* NameOf() const { return "CMplApiMonitor";}
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_MPLAPIMONITOR__)

