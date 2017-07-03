// DNSAgentMonitor.h: interface for the CDNSAgentMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CDNSAgentMONITOR__)
#define _CDNSAgentMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CDNSAgentMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CDNSAgentMonitor();
	virtual ~CDNSAgentMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CDNSAgentMONITOR__)

