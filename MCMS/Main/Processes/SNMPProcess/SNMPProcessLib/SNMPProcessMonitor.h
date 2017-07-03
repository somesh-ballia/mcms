// SNMPProcessMonitor.h: interface for the CSNMPProcessMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CSNMPProcessMONITOR__)
#define _CSNMPProcessMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CSNMPProcessMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CSNMPProcessMonitor();
	virtual ~CSNMPProcessMonitor();
	
private:
	STATUS HandleGetSnmpData(CRequest* pGetRequest);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_CSNMPProcessMONITOR__)
