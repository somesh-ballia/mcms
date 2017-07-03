// NotificationMngrProcess.cpp: implementation of the CNotificationMngrProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "NotificationMngrProcess.h"
#include "SystemFunctions.h"

extern void NotificationMngrManagerEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
CProcessBase* CreateNewProcess()
{
	return new CNotificationMngrProcess;
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CNotificationMngrProcess::GetManagerEntryPoint()
{
	return NotificationMngrManagerEntryPoint;
}


//////////////////////////////////////////////////////////////////////
CNotificationMngrProcess::CNotificationMngrProcess()
{
	m_pSubscriptionsManager = new SubscriptionsManager();
}

//////////////////////////////////////////////////////////////////////
CNotificationMngrProcess::~CNotificationMngrProcess()
{
	POBJDELETE(m_pSubscriptionsManager);
}







