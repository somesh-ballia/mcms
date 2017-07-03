// McmsDaemonMonitor.h: interface for the CMcmsDaemonMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CMcmsDaemonMONITOR__)
#define _CMcmsDaemonMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CMcmsDaemonMonitor : public CMonitorTask
{
CLASS_TYPE_1(CMcmsDaemonMonitor,CMonitorTask )
public:
	CMcmsDaemonMonitor();
	virtual ~CMcmsDaemonMonitor();

	virtual const char* NameOf() const { return "CMcmsDaemonMonitor";}
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CMcmsDaemonMONITOR__)

