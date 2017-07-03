// NotificationMngrProcess.h: interface for the CNotificationMngrProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOPROCESS_H__)
#define _DEMOPROCESS_H__


#include "ProcessBase.h"
#include "SubscriptionsManager.h"


class CNotificationMngrProcess : public CProcessBase  
{
CLASS_TYPE_1(CNotificationMngrProcess,CProcessBase )
public:
	friend class CTestNotificationMngrProcess;

	CNotificationMngrProcess();
	virtual ~CNotificationMngrProcess();
	virtual eProcessType GetProcessType() {return eProcessNotificationMngr;}
	virtual BOOL UsingSockets() {return NO;}
	virtual TaskEntryPoint GetManagerEntryPoint();

	SubscriptionsManager* GetSubscriptionsManager() { return m_pSubscriptionsManager; }

protected:
	SubscriptionsManager* m_pSubscriptionsManager;
};

#endif // !defined(_DEMOPROCESS_H__)
