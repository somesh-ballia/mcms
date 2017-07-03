// EndpointsSimMonitor.h: interface for the EndpointsSimMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ENDPOINTSSIMMONITOR__)
#define _ENDPOINTSSIMMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CEndpointsSimMonitor : public CMonitorTask
{
CLASS_TYPE_1(CEndpointsSimMonitor,CMonitorTask )
public:
	CEndpointsSimMonitor();
	virtual ~CEndpointsSimMonitor();

	virtual const char* NameOf() const { return "CEndpointsSimMonitor";}
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
};

#endif // !defined(_ENDPOINTSSIMMONITOR__)

