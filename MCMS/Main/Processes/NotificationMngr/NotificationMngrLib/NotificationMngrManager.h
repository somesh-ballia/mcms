// NotificationMngrManager.h: interface for the CNotificationMngrManager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_DEMOMANAGER_H__)
#define _DEMOMANAGER_H__

#include "ManagerTask.h"
#include "Macros.h"
#include "NotificationMngrProcess.h"


void NotificationMngrManagerEntryPoint(void* appParam);

class CNotificationMngrManager : public CManagerTask
{
CLASS_TYPE_1(CNotificationMngrManager,CManagerTask )
public:
	CNotificationMngrManager();
	virtual ~CNotificationMngrManager();

	virtual void ManagerPostInitActionsPoint();

	TaskEntryPoint GetMonitorEntryPoint();

	PDECLAR_MESSAGE_MAP
	PDECLAR_TRANSACTION_FACTORY
	PDECLAR_TERMINAL_COMMANDS

protected:
	SubscriptionsManager* GetSubscriptionsManager();
	BOOL IsStartupFinished() const;

	// Action functions
	void OnCheckSubscriptionExpirationTout(CSegment* pSegment);
	void OnHeartBeatTout(CSegment* pSegment);
	void OnMcuMngrMcuStateIndAnycase(CSegment* pSegment);
	void OnMcuMngrAAListIndAnycase(CSegment* pSegment);

	// terminal commands
	STATUS HandleTerminalSubscriptions(CSegment* seg, std::ostream& answer);

	// XML API events
	STATUS HandleSetAddSubscription(CRequest *pRequest);
	STATUS HandleSetDeleteSubscription(CRequest *pRequest);
	STATUS HandleSetSendUpdateNotification(CRequest *pRequest);

};

#endif // !defined(_DEMOMANAGER_H__)
