// TestClientMonitor.h: interface for the TestClientMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TESTCLIENTMONITOR__)
#define _TESTCLIENTMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CTestClientMonitor : public CMonitorTask
{
CLASS_TYPE_1(CTestClientMonitor,CMonitorTask )
public:
	CTestClientMonitor();
	virtual ~CTestClientMonitor();


protected:
	// action function for set requests
	STATUS HandleOperLogin(CRequest *p);

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(_TESTCLIENTMONITOR__)

