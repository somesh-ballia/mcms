// FaultsMonitor.h: interface for the CFaultsMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CFaultsMONITOR__)
#define _CFaultsMONITOR__

#include "Macros.h"
#include "MonitorTask.h"


class CRequest;


class CFaultsMonitor : public CMonitorTask
{
CLASS_TYPE_1(CFaultsMonitor,CMonitorTask )
public:
	CFaultsMonitor();
	virtual ~CFaultsMonitor();

protected:
	STATUS OnServerFaultsList(CRequest* pGetRequest);
	STATUS OnServerFaultsListShort(CRequest* pGetRequest);

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CFaultsMONITOR__)

