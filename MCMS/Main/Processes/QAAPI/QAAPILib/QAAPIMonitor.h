// DemoMonitor.h: interface for the CDemoMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CQAMONITOR__)
#define _CQAMONITOR__

#include "MonitorTask.h"

class CQAAPIMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CQAAPIMonitor();
	virtual ~CQAAPIMonitor();

	virtual const char* NameOf() const { return "CQAAPIMonitor";}
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CQAAPIMonitor__)
