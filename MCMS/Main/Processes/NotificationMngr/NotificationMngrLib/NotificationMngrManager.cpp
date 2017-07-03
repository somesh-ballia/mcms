// NotificationMngrManager.cpp

#include "NotificationMngrManager.h"

#include "Trace.h"
#include "FipsMode.h"
#include "ApiStatuses.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Request.h"
#include "ManagerApi.h"
#include "Subscription.h"
#include "DummyEntry.h"
#include "CommApiUid.h"
#include "CommApiSubscription.h"


const DWORD SUBSCRIPTION_EXPIRATION_TIME       = 10 * SECOND;
const DWORD HEART_BEAT_TIME                    =  3 * SECOND;

const WORD CHECK_SUBSCRIPTION_EXPIRATION_TOUT  = 2031;
const WORD HEART_BEAT_TOUT                     = 2032;


PBEGIN_MESSAGE_MAP(CNotificationMngrManager)
  ONEVENT(XML_REQUEST, IDLE, CNotificationMngrManager::HandlePostRequest)
  ONEVENT(CHECK_SUBSCRIPTION_EXPIRATION_TOUT, ANYCASE, CNotificationMngrManager::OnCheckSubscriptionExpirationTout)
  ONEVENT(HEART_BEAT_TOUT,                    ANYCASE, CNotificationMngrManager::OnHeartBeatTout)
  ONEVENT(MCUMNGR_NOTIFMNGR_MCU_STATE_IND,    ANYCASE, CNotificationMngrManager::OnMcuMngrMcuStateIndAnycase)
  ONEVENT(MCUMNGR_NOTIFMNGR_AA_LIST_IND,      ANYCASE, CNotificationMngrManager::OnMcuMngrAAListIndAnycase)
PEND_MESSAGE_MAP(CNotificationMngrManager, CManagerTask);


BEGIN_SET_TRANSACTION_FACTORY(CNotificationMngrManager)
  ON_TRANS("TRANS_SUBSCRIBE", "ADD",                      Subscription, CNotificationMngrManager::HandleSetAddSubscription)
  ON_TRANS("TRANS_SUBSCRIBE", "DELETE",                   CommApiUid,   CNotificationMngrManager::HandleSetDeleteSubscription)
  ON_TRANS("TRANS_SUBSCRIBE", "SEND_UPDATE_NOTIFICATION", CommApiUid,   CNotificationMngrManager::HandleSetSendUpdateNotification)
END_TRANSACTION_FACTORY


BEGIN_TERMINAL_COMMANDS(CNotificationMngrManager)
  ONCOMMAND("subscriptions",CNotificationMngrManager::HandleTerminalSubscriptions,"print subscriptions list")
END_TERMINAL_COMMANDS


extern void NotificationMngrMonitorEntryPoint(void* appParam);


void NotificationMngrManagerEntryPoint(void* appParam)
{
	CNotificationMngrManager* mngr = new CNotificationMngrManager;
	mngr->Create(*(CSegment*)appParam);
}

TaskEntryPoint CNotificationMngrManager::GetMonitorEntryPoint()
{
	return NotificationMngrMonitorEntryPoint;
}

CNotificationMngrManager::CNotificationMngrManager()
{}

CNotificationMngrManager::~CNotificationMngrManager()
{}

void CNotificationMngrManager::ManagerPostInitActionsPoint()
{
	// put task init code here
	// this function is called just before WaitForEvent

	TestAndEnterFipsMode();

	SubscriptionsManager* pSubsMngr = GetSubscriptionsManager();
	if( pSubsMngr )
		pSubsMngr->ReadXmlFile();

	// send mcu_state request to McuMngr
	CManagerApi api(eProcessMcuMngr);
	STATUS res = api.SendOpcodeMsg(MCUMNGR_NOTIFMNGR_MCU_STATE_REQ);
	DBGPASSERT(res);
	res = api.SendOpcodeMsg(MCUMNGR_NOTIFMNGR_AA_LIST_REQ);
	DBGPASSERT(res);

	StartTimer(CHECK_SUBSCRIPTION_EXPIRATION_TOUT,1 * SECOND);

	StartTimer(HEART_BEAT_TOUT,HEART_BEAT_TIME);
}

/////////////////////////////////////////////////////////////////////////////
BOOL CNotificationMngrManager::IsStartupFinished() const
{
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
	//std::cout << "state=" << (int)systemState << std::endl;
	if( eMcuState_Invalid == systemState || eMcuState_Startup == systemState )
		return FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
SubscriptionsManager* CNotificationMngrManager::GetSubscriptionsManager()
{
	CNotificationMngrProcess* pProcess = (CNotificationMngrProcess*)CProcessBase::GetProcess();
	if(!CPObject::IsValidPObjectPtr(pProcess))
	{
		DBGPASSERT(1001);
		return NULL;
	}
	return pProcess->GetSubscriptionsManager();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CNotificationMngrManager::HandleTerminalSubscriptions(CSegment* seg, std::ostream& answer)
{
	SubscriptionsManager* pSubsMngr = GetSubscriptionsManager();
	if(pSubsMngr)
		pSubsMngr->Dump(answer);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CNotificationMngrManager::HandleSetAddSubscription(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,"CNotificationMngrManager::HandleSetAddSubscription");
	//std::cout << "HandleSetAddSubscription" << std::endl;

	if (pRequest->GetAuthorization() == SUPER)
	{
		STATUS status = STATUS_OK;
		std::string statusDesc = "";

		Subscription* pSubscriptionObj = NULL;
		pSubscriptionObj = (Subscription*)(pRequest->GetRequestObject());

		std::string sUserId = "POLYCOM";
		std::string sSubscriptionUid;

		SubscriptionsManager* pSubsMngr = GetSubscriptionsManager();
		if(pSubsMngr)
			status = pSubsMngr->AddSubscription(*pSubscriptionObj,sUserId,sSubscriptionUid);

		if( STATUS_OK == status )
		{
			CommApiSubscription* pResponseObj = new CommApiSubscription;
			pResponseObj->m_sUid = sSubscriptionUid;
			pResponseObj->m_subscription = *pSubscriptionObj;
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
		PTRACE(eLevelInfoNormal,"CNotificationMngrManager::HandleSetAddSubscription: No permission to add subscriptions");
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CNotificationMngrManager::HandleSetDeleteSubscription(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,"CNotificationMngrManager::HandleSetDeleteSubscription");
	//std::cout << "HandleSetDeleteSubscription" << std::endl;

	if (pRequest->GetAuthorization() == SUPER)
	{
		STATUS status = STATUS_OK;
		std::string statusDesc = "";

		CommApiUid* pUidObj = NULL;
		pUidObj = (CommApiUid*)(pRequest->GetRequestObject());

		SubscriptionsManager* pSubsMngr = GetSubscriptionsManager();
		if( pSubsMngr )
			status = pSubsMngr->DeleteSubscription(pUidObj->GetUid());

		pRequest->SetConfirmObject(new CDummyEntry);
		//pRequest->SetExDescription(statusDesc.c_str());
		pRequest->SetStatus(status);
    }
    else
	{
		PTRACE(eLevelInfoNormal,"CNotificationMngrManager::HandleSetDeleteSubscription: No permission to delete subscription");
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CNotificationMngrManager::HandleSetSendUpdateNotification(CRequest *pRequest)
{
	PTRACE(eLevelInfoNormal,"CNotificationMngrManager::HandleSetSendUpdateNotification");
	//std::cout << "HandleSetSendUpdateNotification" << std::endl;

	if (pRequest->GetAuthorization() == SUPER)
	{
		STATUS status = STATUS_OK;
		std::string statusDesc = "";

		CommApiUid* pUidObj = NULL;
		pUidObj = (CommApiUid*)(pRequest->GetRequestObject());

		SubscriptionsManager* pSubsMngr = GetSubscriptionsManager();
		if(pSubsMngr)
			status = pSubsMngr->SendUpdateNotification(pUidObj->GetUid());

		// if resource data not received yet
		if( STATUS_FAIL == status )
		{
			status = STATUS_OK;
		}

		pRequest->SetConfirmObject(new CDummyEntry);
		pRequest->SetStatus(status);
    }
    else
	{
		PTRACE(eLevelInfoNormal,"CNotificationMngrManager::HandleSetSendUpdateNotification: No permission to send notifications");
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CNotificationMngrManager::OnCheckSubscriptionExpirationTout(CSegment* pSegment)
{
	PTRACE(eLevelInfoNormal,"CNotificationMngrManager::OnCheckSubscriptionExpirationTout - restart timer");
	//std::cout << "CNotificationMngrManager::OnCheckSubscriptionExpirationTout - restart timer" << std::endl;
	StartTimer(CHECK_SUBSCRIPTION_EXPIRATION_TOUT,SUBSCRIPTION_EXPIRATION_TIME);

	SubscriptionsManager* pSubsMngr = GetSubscriptionsManager();
	DBGPASSERT_AND_RETURN(!pSubsMngr);

	pSubsMngr->CheckExpiration();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CNotificationMngrManager::OnHeartBeatTout(CSegment* pSegment)
{
	PTRACE(eLevelInfoNormal,"CNotificationMngrManager::OnHeartBeatTout - restart timer");
	//std::cout << "CNotificationMngrManager::OnHeartBeatTout - " << std::endl;
	StartTimer(HEART_BEAT_TOUT,HEART_BEAT_TIME);

//	if( FALSE == IsStartupFinished() )
//		return;

	SubscriptionsManager* pSubsMngr = GetSubscriptionsManager();
	DBGPASSERT_AND_RETURN(!pSubsMngr);

	pSubsMngr->HandleEvent(eResourceEventHeartbeat,"<HEARTBIT_INDICATION><DESCRIPTION>Status OK</DESCRIPTION></HEARTBIT_INDICATION>");
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CNotificationMngrManager::OnMcuMngrMcuStateIndAnycase(CSegment* pSegment)
{
	PTRACE(eLevelInfoNormal,"CNotificationMngrManager::OnMcuMngrMcuStateIndAnycase");

	std::string sMcuState;
	*pSegment >> sMcuState;

	//std::cout << "CNotificationMngrManager::OnMcuMngrMcuStateIndAnycase - " << sMcuState << std::endl;

	SubscriptionsManager* pSubscMngr = GetSubscriptionsManager();
	DBGPASSERT_AND_RETURN(!pSubscMngr);

	pSubscMngr->HandleEvent(eResourceEventStatus,sMcuState);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void CNotificationMngrManager::OnMcuMngrAAListIndAnycase(CSegment* pSegment)
{
	PTRACE(eLevelInfoNormal,"CNotificationMngrManager::OnMcuMngrAAListIndAnycase");

	std::string sAaList;
	*pSegment >> sAaList;

	//std::cout << "CNotificationMngrManager::OnMcuMngrMcuStateIndAnycase - " << sMcuState << std::endl;

	SubscriptionsManager* pSubscMngr = GetSubscriptionsManager();
	DBGPASSERT_AND_RETURN(!pSubscMngr);

	pSubscMngr->HandleEvent(eResourceEventAlarms,sAaList);
}







