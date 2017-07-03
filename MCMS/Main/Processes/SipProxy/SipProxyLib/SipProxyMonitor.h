// SipProxyMonitor.h: interface for the CSipProxyMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CSipProxyMONITOR__)
#define _CSipProxyMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CSipProxyMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CSipProxyMonitor();
	virtual ~CSipProxyMonitor();

	virtual const char* NameOf() const { return "CSipProxyMonitor";}
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CSipProxyMONITOR__)

