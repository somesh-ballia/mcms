// CSApiMonitor.h: interface for the CCSApiMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CCSApiMONITOR__)
#define _CCSApiMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CCSApiMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CCSApiMonitor();
	virtual ~CCSApiMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CCSApiMONITOR__)

