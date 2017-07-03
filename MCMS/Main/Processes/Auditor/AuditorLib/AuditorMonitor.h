// AuditorMonitor.h: interface for the CAuditorMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(__AUDITOR_MONITOR_H__)
#define __AUDITOR_MONITOR_H__

#include "MonitorTask.h"
#include "Macros.h"


class CRequest;




class CAuditorMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CAuditorMonitor();
	virtual ~CAuditorMonitor();

private:
    STATUS HandleGetAuditFileList(CRequest* pGetRequest);
    STATUS HandleGetAuditEventList(CRequest* pGetRequest);
    
protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

};

#endif // !defined(__AUDITOR_MONITOR_H__)
