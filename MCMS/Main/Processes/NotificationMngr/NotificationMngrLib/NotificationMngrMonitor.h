// NotificationMngrMonitor.h: interface for the CNotificationMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_CNotificationMngrMONITOR__)
#define _CNotificationMngrMONITOR__

#include "MonitorTask.h"
#include "Macros.h"

class CNotificationMngrMonitor : public CMonitorTask
{
CLASS_TYPE_1(DCemoMonitor,CMonitorTask )
public:
	CNotificationMngrMonitor();
	virtual ~CNotificationMngrMonitor();

protected:
	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY

	// XML API events
	STATUS HandleGetSubscription(CRequest *pRequest);
};

#endif // !defined(_CNotificationMngrMONITOR__)
