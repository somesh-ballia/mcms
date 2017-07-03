// AuthenticationMonitor.cpp: implementation of the AuthenticationMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "AuthenticationMonitor.h"
#include "OpcodesMcmsCommon.h"
#include "OperListGet.h"
#include "Request.h"
#include "StatusesGeneral.h"
#include "AudibleAlarmGet.h"

////////////////////////////////////////////////////////////////////////////
//               Message Map
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CAuthenticationMonitor)
  ONEVENT(XML_REQUEST    ,IDLE    , CAuthenticationMonitor::HandlePostRequest )
PEND_MESSAGE_MAP(CAuthenticationMonitor,CAlarmableTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_GET_TRANSACTION_FACTORY(CAuthenticationMonitor)
  ON_TRANS("TRANS_OPER_LIST","GET_OPER_LIST",COperListGet,CAuthenticationMonitor::HandleGetOperList)
  ON_TRANS("TRANS_OPERATOR","GET_OPERATOR_AUDIBLE_ALARM",CAudibleAlarmGet,CAuthenticationMonitor::GetOperatorAudibleALarm)
END_TRANSACTION_FACTORY


////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void AuthenticationMonitorEntryPoint(void* appParam)
{
	CAuthenticationMonitor *monitorTask = new CAuthenticationMonitor;
	monitorTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAuthenticationMonitor::CAuthenticationMonitor()
{

}

CAuthenticationMonitor::~CAuthenticationMonitor()
{

}

STATUS CAuthenticationMonitor::GetOperatorAudibleALarm(CRequest *pRequest)
{
	CAudibleAlarmGet* pRequestAudibleAlarm = (CAudibleAlarmGet*)pRequest->GetRequestObject();

	CAudibleAlarmGet* pConfirmAudibleAlarm = new CAudibleAlarmGet(*pRequestAudibleAlarm);

	pRequest->SetConfirmObject(pConfirmAudibleAlarm);

	return STATUS_OK;
}

STATUS CAuthenticationMonitor::HandleGetOperList(CRequest *pRequest)
{
	COperListGet* pRequestOperListGet = (COperListGet*)pRequest->GetRequestObject();

	COperListGet* pConfirmOperListGet = new COperListGet(*pRequestOperListGet,pRequest->GetAuthorization());

	pRequest->SetConfirmObject(pConfirmOperListGet);

	return STATUS_OK;
}


