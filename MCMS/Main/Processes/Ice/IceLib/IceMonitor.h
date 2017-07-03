// IceMonitor.h: interface for the CIceMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CIceMONITOR__)
#define _CIceMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CIceMonitor : public CMonitorTask
{
CLASS_TYPE_1(CIceMonitor,CMonitorTask )
public:
	CIceMonitor();
	virtual ~CIceMonitor();

	virtual const char* NameOf() const { return "CIceMonitor";}
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CIceMONITOR__)

