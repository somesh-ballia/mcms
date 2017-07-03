// UtilityMonitor.h: interface for the CUtilityMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CUtilityMONITOR__)
#define _CUtilityMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CUtilityProcess;

class CUtilityMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CUtilityMonitor();
	virtual ~CUtilityMonitor();
	STATUS OnGetTcpDumpCfg(CRequest* pGetRequest);
	STATUS OnGetTcpDumpStatus(CRequest* pGetRequest);

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
private:
	CUtilityProcess* m_proc;

};

#endif // !defined(_CUtilityMONITOR__)
