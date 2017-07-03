// IPMCInterfaceMonitor.h: interface for the CIPMCInterfaceMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CIPMCInterfaceMONITOR__)
#define _CIPMCInterfaceMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CIPMCInterfaceMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CIPMCInterfaceMonitor();
	virtual ~CIPMCInterfaceMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CIPMCInterfaceMONITOR__)
