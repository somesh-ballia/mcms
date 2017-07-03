// NotificationMngrMonitor.cpp: implementation of the NotificationMngrMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "NotificationMngrMonitor.h"
#include "TaskApi.h"
#include "Trace.h"
#include "ApiStatuses.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "NotificationMngrProcess.h"
#include "Request.h"
#include "DummyEntry.h"
#include "CommApiUid.h"
#include "CommApiSubscription.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CNotificationMngrMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CNotificationMngrMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CNotificationMngrMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CNotificationMngrMonitor)
  ON_TRANS("TRANS_SUBSCRIBE", "GET",  CommApiUid,  CNotificationMngrMonitor::HandleGetSubscription)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void NotificationMngrMonitorEntryPoint(void* appParam)
{  
	CNotificationMngrMonitor *monitorTask = new CNotificationMngrMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNotificationMngrMonitor::CNotificationMngrMonitor()
{

}

CNotificationMngrMonitor::~CNotificationMngrMonitor()
{

}

/////////////////////////////////////////////////////////////////////////////
STATUS CNotificationMngrMonitor::HandleGetSubscription(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,"CNotificationMngrMonitor::HandleGetSubscription");

	if (pRequest->GetAuthorization() == SUPER)
	{
		STATUS status = STATUS_OK;
		std::string statusDesc = "";

		CommApiUid* pUidObj = NULL;
		pUidObj = (CommApiUid*)(pRequest->GetRequestObject());
		Subscription subscription;
		status = ((CNotificationMngrProcess*)CProcessBase::GetProcess())->GetSubscriptionsManager()->GetSubscription(pUidObj->GetUid(),subscription);
		if( STATUS_OK == status )
		{
			CommApiSubscription* pResponseObj = new CommApiSubscription;
			pResponseObj->m_sUid = pUidObj->GetUid();
			pResponseObj->m_subscription = subscription;
			pResponseObj->m_subscription.SetPassword("********");

			pRequest->SetConfirmObject(pResponseObj);
		}
		else
		{
			pRequest->SetConfirmObject(new CDummyEntry);
		}
		//pRequest->SetExDescription(statusDesc.c_str());
		pRequest->SetStatus(status);
    }
    else
	{
		PTRACE(eLevelInfoNormal,"CNotificationMngrMonitor::HandleGetSubscription: No permission to get subscription");
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}

	return STATUS_OK;
}

