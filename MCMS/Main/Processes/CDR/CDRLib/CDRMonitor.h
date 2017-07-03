// DemoMonitor.h: interface for the CDemoMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CCDRMONITOR__)
#define _CCDRMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CRequest;
class CCDRProcess;

class CCDRMonitor : public CMonitorTask
{
CLASS_TYPE_1(CCDRMonitor,CMonitorTask )
public:
	CCDRMonitor();
	virtual ~CCDRMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

private:
	STATUS HandleCdrGet(CRequest*);
	STATUS HandleCdrFullGet(CRequest*);
	STATUS HandleCdrSettingsGet(CRequest* pGetRequest);
	
	CCDRProcess *m_CDRProcess;
};

#endif // !defined(_CCDRMONITOR__)
