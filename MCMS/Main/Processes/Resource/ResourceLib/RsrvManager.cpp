#include "TaskApi.h"
#include "InterProcessStruct.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "Macros.h"
#include "Trace.h"
#include "CommResApi.h"
#include "RsrvManager.h"
#include "StatusesGeneral.h"
#include "psosxml.h"
#include "Request.h"
#include "CommResAdd.h"
#include "InternalProcessStatuses.h"
#include "TraceStream.h"
#include "HelperFuncs.h"
#include <fcntl.h>
#include "RsrvResources.h"
#include "Reservator.h"
#include "CommResRsrvShort.h"
#include "CommResRsrvDBAction.h"
#include "SysConfig.h"
#include "DummyEntry.h"
#include "CommResRecurrenceResponse.h"
#include "SetEndTime.h"


PBEGIN_MESSAGE_MAP(CRsrvManager)

	ONEVENT(XML_REQUEST,                           IDLE,    CRsrvManager::HandlePostRequest)
	ONEVENT(ACTIVATE_MR_REQ,                       ANYCASE, CRsrvManager::OnCreateRsrvFromMRRequest)
	ONEVENT(START_AD_HOC_CONF_REQ,                 ANYCASE, CRsrvManager::OnCreateRsrvAdHocRequest)
	ONEVENT(DEL_RSRV_CONF_REQ,                     ANYCASE, CRsrvManager::OnDeleteRsrvFromApiRequest)
	ONEVENT(SYNC_DEL_RSRV_CONF_REQ,                ANYCASE, CRsrvManager::OnSyncDeleteRsrvFromApiRequest)
	ONEVENT(DEACTIVATE_MR_REQ,                     ANYCASE, CRsrvManager::OnDeleteRsrvFromMrRequest)
	ONEVENT(SYNC_DEACTIVATE_MR_REQ,                ANYCASE, CRsrvManager::OnSyncDeleteRsrvFromMrRequest)
	ONEVENT(DEL_MR_REQ,                            ANYCASE, CRsrvManager::OnDeleteMrRequest)
	ONEVENT(IP_RESOURCE_INFO_REQ,                  ANYCASE, CRsrvManager::OnIPResourceRequest)
	ONEVENT(RESOURCE_READ_RES_DB,                  ANYCASE, CRsrvManager::OnReadResDB)
	ONEVENT(MCUMNGR_TO_RSRCALLOC_TIME_CHANGED_IND, ANYCASE, CRsrvManager::OnTimeChangedInd)
	ONEVENT(RESOURCE_DELETE_REPEATED_IND,          ANYCASE, CRsrvManager::OnDeleteRepeatedInd)
	ONEVENT(RESOURCE_DELETE_ALL_RES_IND,           ANYCASE, CRsrvManager::OnDeleteRepeatedInd)
	ONEVENT(RESOURCE_WRITE_REPEATED_IND,           ANYCASE, CRsrvManager::OnWriteRepeatedInd)
	ONEVENT(RESOURCE_START_CONFERENCE_TIMER,       ANYCASE, CRsrvManager::OnStartConferenceTimerInd)
	ONEVENT(SLAVE_ADD_OR_UPDATE_RSRV_REQ,          ANYCASE, CRsrvManager::OnCreateOrUpdateSlaveRsrvRequest)
	ONEVENT(SLAVE_ADD_CONF_REQ,                    ANYCASE, CRsrvManager::OnStartSlaveOngRequest)
	ONEVENT(SLAVE_DELETE_RSRV_REQ,                 ANYCASE, CRsrvManager::OnDeleteSlaveRsrvRequest)
	ONEVENT(SLAVE_ADD_MEETING_ROOM_REQ,            ANYCASE, CRsrvManager::OnCreateOrUpdateSlaveRsrvRequest)
	ONEVENT(SLAVE_DELETE_MEETING_ROOM_REQ,         ANYCASE, CRsrvManager::OnDeleteMrRequest)
	ONEVENT(SLAVE_ACTIVATE_MR_REQ,                 ANYCASE, CRsrvManager::OnCreateRsrvFromMRRequest)
	ONEVENT(RESOURCE_DELETE_CONFERENCE_TIMER,      ANYCASE, CRsrvManager::OnDeleteConferenceTimerInd)

PEND_MESSAGE_MAP(CRsrvManager, CRequestHandler);

BEGIN_SET_TRANSACTION_FACTORY(CRsrvManager)
	ON_TRANS("TRANS_RES_1",          "START",             CCommResAdd, CRsrvManager::OnCreateRsrvRequest)
	ON_TRANS("TRANS_RES_CONTINUE_1", "START",             CCommResAdd, CRsrvManager::OnCreateRsrvRequestContinue1)
	ON_TRANS("TRANS_RES_1",          "START_REPEATED_EX", CCommResAdd, CRsrvManager::OnCreateRepeatedRsrvRequest)
	ON_TRANS("TRANS_RES_1",          "UPDATE",            CCommResAdd, CRsrvManager::OnUpdateRequest)
	ON_TRANS("TRANS_CONF_2",         "SET_END_TIME",      CSetEndTime, CRsrvManager::OnUpdateEndTimeFromApirequest)
END_TRANSACTION_FACTORY


void rsrvMngrEntryPoint(void* appParam)
{
	CRsrvManager* rsrvMngrTask = new CRsrvManager;

	CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	if (pProcess)
	{
		pProcess->SetRsrvManager(rsrvMngrTask);
	}
	else
	{
		FPASSERTMSG(1, "pProcess is NULL, can't set RsrvManager");
	}

	rsrvMngrTask->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////
//                        CRsrvManager
////////////////////////////////////////////////////////////////////////////
CRsrvManager::CRsrvManager()
{
	m_pReservator = NULL;
	m_dwInternalConfStatus = 0;
}

////////////////////////////////////////////////////////////////////////////
CRsrvManager::~CRsrvManager()
{
	POBJDELETE(m_pReservator);
}

////////////////////////////////////////////////////////////////////////////
void* CRsrvManager::GetMessageMap()
{
	return m_msgEntries;
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager:: InitTask()
{
	// MR DB reading on Startup done in Resource Manager
	CRequestHandler::InitTask();

	// read reservation mode / create reservation folder if needed
	m_pReservator = new CReservator();
	// reset a flag
	m_dwInternalConfStatus = 0;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrvManager::OnCreateRsrvRequest(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		FTRACEINTO << "No permission to Create conference  for administrator read-only";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	CCommResAdd* pCommResAdd = new CCommResAdd;
	CCommResApi* pCommResApi = new CCommResApi;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pCommResApi = *(pCommResAdd->GetCommResApi());

	DumpToTrace(pCommResApi, "CRsrvManager::OnCreateRsrvRequest");

	DWORD problemConfId = 0;
	bool bForwardTransaction = false;
	STATUS status = m_pReservator->CreateRsrvFromAPIRequest(pCommResApi, bForwardTransaction, &problemConfId);

	if (bForwardTransaction)
	{
		pRequest->SetTransName("TRANS_RES_CONTINUE_1"); // instead of TRANS_RES_1
		pCommResAdd->SetCommResApi(pCommResApi);
		pRequest->SetRequestObject(pCommResAdd);

		if (STATUS_OK == status && STATUS_OK != m_dwInternalConfStatus)
		{
			pRequest->SetStatus(m_dwInternalConfStatus | WARNING_MASK);   // olga (approved by Sergey M)
		}
		else
		{
			pRequest->SetStatus(status);
			if (CHelperFuncs::IsMode2C() && pCommResApi->IsPermanent())
			{
				ostringstream strStatus;
				strStatus << "Can't add the permanent conference due a conflict with the following conference/reservation " << problemConfId;
				pRequest->SetExDescription(strStatus.str().c_str());
			}
		}
		POBJDELETE(pCommResApi);
		status = STATUS_FW_REQUEST_TO_CONFPARTY;
	}
	else
	{
		// (***) answer to EMA !!!!
		std::string responseTrancsName("TRANS_RES"); // instead of TRANS_RES_1
		pRequest->SetTransName(responseTrancsName);
		pRequest->SetConfirmObject(pCommResApi);

		if (STATUS_OK == status && STATUS_OK != m_dwInternalConfStatus)
		{
			pRequest->SetStatus(m_dwInternalConfStatus | WARNING_MASK);   // olga (approved by Sergey M)
		}
		else
		{
			pRequest->SetStatus(status);
			if (CHelperFuncs::IsMode2C() && pCommResApi->IsPermanent())
			{
				ostringstream strStatus;
				strStatus << "Can't add the permanent conference due a conflict with the following conference/reservation " << problemConfId;
				pRequest->SetExDescription(strStatus.str().c_str());
			}
		}
		POBJDELETE(pCommResAdd);
	}
	m_dwInternalConfStatus = 0;
	TRACEINTO << "Status:" << status << (status == STATUS_FW_REQUEST_TO_CONFPARTY ? " (STATUS_FW_REQUEST_TO_CONFPARTY)" : "");
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrvManager::OnCreateRsrvRequestContinue1(CRequest* pRequest)
{
	STATUS status = STATUS_OK;
	CCommResAdd* pCommResAdd = new CCommResAdd;
	CCommResApi* pCommResApi = new CCommResApi;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pCommResApi = *(pCommResAdd->GetCommResApi());

	DumpToTrace(pCommResApi, "CRsrvManager::OnCreateRsrvRequestContinue1");

	DWORD monitorConfId = pCommResApi->GetMonitorConfId();
	m_pReservator->DecActiveForwardTransaction();

	CCommResApi* pCommResApiTmp = m_pReservator->StartConference(true, monitorConfId);

	if (pCommResApiTmp)
	{
		std::string responseTrancsName("TRANS_RES_CONTINUE_2"); // instead of TRANS_RES_1
		pRequest->SetTransName(responseTrancsName);

		pCommResAdd->SetCommResApi(pCommResApiTmp);
		pRequest->SetRequestObject(pCommResAdd);
		POBJDELETE(pCommResApiTmp);
		POBJDELETE(pCommResApi);
		status = STATUS_FW_REQUEST_TO_CONFPARTY;
	}
	else
	{
		PASSERTMSG(true, "cannot find the conference !!!");

		std::string responseTrancsName("TRANS_RES"); // instead of TRANS_RES_1
		pRequest->SetTransName(responseTrancsName);
		pRequest->SetConfirmObject(pCommResApi);
		status = STATUS_OK;
		POBJDELETE(pCommResAdd);
	}
	TRACEINTO << "Status:" << status << (status == STATUS_FW_REQUEST_TO_CONFPARTY ? " (STATUS_FW_REQUEST_TO_CONFPARTY)" : "");
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnCreateOrUpdateSlaveRsrvRequest(CSegment* pSeg)
{
	CCommResApi* pCommResApi = new CCommResApi();
	pCommResApi->DeSerialize(NATIVE, *pSeg);

	DumpToTrace(pCommResApi, "CRsrvManager::OnCreateOrUpdateSlaveRsrvRequest");

	STATUS status = m_pReservator->CreateOrUpdateSlaveRsrv(pCommResApi);
	TRACECOND(status, "Status:" << status);

	POBJDELETE(pCommResApi);
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnStartSlaveOngRequest(CSegment* pMsg)
{
	CCommResApi* pCommResApi = new CCommResApi();
	pCommResApi->DeSerialize(NATIVE, *pMsg);

	DumpToTrace(pCommResApi, "CRsrvManager::OnStartSlaveOngRequest");

	STATUS status = m_pReservator->StartSlaveOngReq(pCommResApi);
	TRACECOND(status, "Status:" << status);

	POBJDELETE(pCommResApi);
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrvManager::OnCreateRepeatedRsrvRequest(CRequest* pRequest)
{
	STATUS status = STATUS_OK;

	CCommResAdd*          pCommResAdd      = new CCommResAdd;
	CCommResApi*          pCommResApi      = new CCommResApi;
	CComResRepeatDetails* pCommResRepeated = new CComResRepeatDetails;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pCommResApi = *(pCommResAdd->GetCommResApi());

	DumpToTrace(pCommResApi, "CRsrvManager::OnCreateRepeatedRsrvRequest");

	CComResRepeatDetails* pCommResRepeatedFromRequest = (CComResRepeatDetails*)(pCommResAdd->GetComResRepeatDetails());
	CCommResRecurrenceResponse* pResRecurrenceResponse = new CCommResRecurrenceResponse();
	if (pCommResRepeatedFromRequest != NULL)
	{
		*pCommResRepeated = *pCommResRepeatedFromRequest;
		DumpToTrace(pCommResRepeatedFromRequest, "OnCreateRepeatedRsrvRequest");
		status = m_pReservator->CreateRepeatedRsrvFromAPIRequest(pCommResApi, pCommResRepeated, pResRecurrenceResponse);
	}
	else
	{
		PASSERT(1);
		status = STATUS_FAIL;
	}

	// (***) answer to EMA !!!!
	std::string responseTrancsName("TRANS_RES"); // instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pResRecurrenceResponse);
	pRequest->SetStatus(status);

	POBJDELETE(pCommResAdd);
	POBJDELETE(pCommResRepeated);
	POBJDELETE(pCommResApi);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrvManager::OnUpdateEndTimeFromApirequest(CRequest* pRequest)
{
	PTRACE(eLevelInfoHigh, "CRsrvManager::OnUpdateEndTimeFromApirequest");

	CSetEndTime* pStructTm = new CSetEndTime;

	*pStructTm = *(CSetEndTime*)pRequest->GetRequestObject();
	const DWORD confId = pStructTm->GetConfID();
	const CStructTm* pTime = pStructTm->GetTime();

	DumpToTrace(pStructTm, "OnUpdateEndTimeFromApirequest");

	STATUS status = m_pReservator->SetConferenceEndTimeRequest(confId, pTime);
	TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);

	// Answer to confparty
	SET_CONFERENCE_ENDTIME_IND_PARAMS_S* pResult = new SET_CONFERENCE_ENDTIME_IND_PARAMS_S;
	pResult->monitorConfId = confId;
	pResult->newEndTime    = *pTime;
	pResult->status        = status;
	pResult->isSetByOperator = true;

	CSegment* pRetParam = new CSegment();
	pRetParam->Put((BYTE*)pResult, sizeof(SET_CONFERENCE_ENDTIME_IND_PARAMS_S));

	CManagerApi api(eProcessConfParty);
	api.SendMsg(pRetParam, SET_CONFERENCE_ENDTIME_IND);

	PDELETE(pResult);

	// (***) answer to EMA !!!!
	std::string responseTrancsName("TRANS_CONF"); // instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pStructTm);
	pRequest->SetStatus(status);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnCreateRsrvFromMRRequest(CSegment* pSeg)
{
	CCommResApi* pCommResApi = new CCommResApi();
	pCommResApi->DeSerialize(NATIVE, *pSeg);
	DumpToTrace(pCommResApi, "CRsrvManager::OnCreateRsrvFromMrRequest");

	STATUS status = m_pReservator->CreateRsrvFromMRRequest(pCommResApi);
	TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);

	if (eRPSlaveMode != m_pReservator->GetRPMode())
	{
		CSegment*   pRetParam = new CSegment();
		COstrStream ostr;
		pCommResApi->Serialize(NATIVE, ostr);
		ostr << status;
		(*pRetParam) << ostr.str().c_str();

		// (**) send back to  ConfMngr
		CManagerApi api(eProcessConfParty);
		api.SendMsg(pRetParam, RSRC_ACTIVATE_MR_IND);
	}

	POBJDELETE(pCommResApi);
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnCreateRsrvAdHocRequest(CSegment* pMsg)
{
	PTRACE(eLevelInfoHigh, "CRsrvManager::OnCreateRsrvAdHocRequest");

	CCommResApi* pCommResApi = new CCommResApi();

	// Eitan (GW) - add some additional info at the end of stream, and return them to ConfParty
	CIstrStream istr(*pMsg);

	pCommResApi->DeSerialize(NATIVE, istr);

	// VNGR-22659: get the sender position identifier in order to identify ongoing
	// conference restore after reboot
	WORD senderPositionId;
	bool bUseOldConfId;
	istr >> senderPositionId;

	TRACEINTO << "senderPositionId:" << senderPositionId;

	if (senderPositionId == 3)
		bUseOldConfId = true;
	else
		bUseOldConfId = false;

	// Eitan (GW) - read additional info from the end of stream (and store it at ostrTmp)
	COstrStream ostrTmp;
	ALLOCBUFFER(tempBuffer, H243_NAME_LEN);
	while (istr.good())
	{
		istr.getline(tempBuffer, H243_NAME_LEN);
		ostrTmp << tempBuffer << "\n";
	}

	DEALLOCBUFFER(tempBuffer);

	STATUS status = m_pReservator->CreateAdHocRequest(pCommResApi, bUseOldConfId);
	TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);

	CSegment* pRetParam = new CSegment();

	COstrStream ostr;

	pCommResApi->Serialize(NATIVE, ostr);
	ostr << status << "\n";
	// Eitan (GW) - return the additional info that stored in ostrTmp to ConfParty
	ostr << ostrTmp.str().c_str();

	(*pRetParam) << ostr.str().c_str();

	// (**) send back to  ConfMngr
	CManagerApi api(eProcessConfParty);
	api.SendMsg(pRetParam, RSRC_START_CONF_IND);

	POBJDELETE(pCommResApi);
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnDeleteRsrvFromApiRequest(CSegment* pSeg)
{
	// (**) deallocate numericId specific ( in segment)
	// (**) deallocate monitorId specific ( in segment)
	// (**) send back to ConfMngr
	STATUS status = STATUS_OK;
	ConfMonitorID monitor_Id = 0xFFFFFFFF;
	*pSeg >> monitor_Id;
	TRACESTRFUNC(eLevelInfoHigh) << "MonitorConfId:" << monitor_Id;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (!pSystemResources)
	{
		PASSERT(1);
		status = STATUS_FAIL;
	}
	else
	{
		if (!m_pReservator->IsInternalReservator()) // delete phones for regular conferences in case of InternalReservator==false
		{
			CConfRsrvRsrc* pRsrvConf = (CConfRsrvRsrc*)m_pReservator->GetRegularConference(monitor_Id);
			if (pRsrvConf)
			{
				CServicePhoneStr* pPhoneStr = pRsrvConf->GetFirstServicePhone();
				while (pPhoneStr != NULL)
				{
					status = pSystemResources->DeAllocateServicePhones(*pPhoneStr);
					PASSERTSTREAM(STATUS_OK != status, "Status:" << status); // olga - temp

					pPhoneStr = pRsrvConf->GetNextServicePhone();
				}
			}
		}

		eRsrcConfType conf_type = eConf_type_none;
		status = m_pReservator->DeleteResAllTypes(monitor_Id, &conf_type);
		PASSERTSTREAM(STATUS_OK != status, "Status:" << status);

		// check status
		if (eConf_type_none != conf_type)
			TRACEINTO << "ConfType:" << (DWORD)conf_type;
	}

	CSegment* pRetParam = new CSegment();
	pRetParam->Put((BYTE*)(&status), sizeof(DWORD));

	// (**) send back to ConfMngr
	TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);
	CManagerApi api(eProcessConfParty);
	api.SendMsg(pRetParam, RSRC_DEL_RSRV_CONF_IND);
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnDeleteSlaveRsrvRequest(CSegment* pSeg)
{
	ConfMonitorID monitor_Id = 0xFFFFFFFF;
	*pSeg >> monitor_Id;
	TRACESTR(eLevelInfoHigh) << "MonitorConfId:" << monitor_Id;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	PASSERT_AND_RETURN(eRPSlaveMode != m_pReservator->GetRPMode());

	STATUS status = m_pReservator->DeleteRes(monitor_Id);
	if (status != STATUS_OK)
		TRACEINTO << "Status:" << status;
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnSyncDeleteRsrvFromApiRequest(CSegment* pSeg)
{
	// (**) deallocate numericId specific ( in segment)
	// (**) deallocate monitorId specific ( in segment)
	// (**) send back to Conf

	STATUS status = STATUS_OK;
	ConfMonitorID monitor_Id = 0xFFFFFFFF;
	*pSeg >> monitor_Id;

	TRACESTR(eLevelInfoHigh) << "MonitorConfId:" << monitor_Id;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (!pSystemResources)
	{
		PASSERT(1);
		status = STATUS_FAIL;
	}
	else
	{
		if (!m_pReservator->IsInternalReservator()) // delete phones for regular conferences in case of InternalReservator==false
		{
			CConfRsrvRsrc* pRsrvConf = (CConfRsrvRsrc*)m_pReservator->GetRegularConference(monitor_Id);
			if (pRsrvConf)
			{
				CServicePhoneStr* pPhoneStr = pRsrvConf->GetFirstServicePhone();
				while (pPhoneStr != NULL)
				{
					status = pSystemResources->DeAllocateServicePhones(*pPhoneStr);
					PASSERTSTREAM(STATUS_OK != status, "Status:" << status);   // olga - temp

					pPhoneStr = pRsrvConf->GetNextServicePhone();
				}
			}
		}

		eRsrcConfType conf_type = eConf_type_none;
		status = m_pReservator->DeleteResAllTypes(monitor_Id, &conf_type);
		if (status != STATUS_OK)
			TRACEINTO << "Status:" << status;

		if (eConf_type_none != conf_type)
			TRACEINTO << "ConfType:" << (DWORD)conf_type;
	}

	// (**) send back to Conf
	CSegment* pRetParam = new CSegment();
	pRetParam->Put((BYTE*)(&status), sizeof(DWORD));
	ResponedClientRequest(RSRC_DEL_RSRV_CONF_IND, pRetParam);
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnDeleteRsrvFromMrRequest(CSegment* pSeg)
{
	// (**) deallocate monitorId specific ( in segment)
	// (**) send back to ConfMngr

	STATUS status = STATUS_OK;
	ConfMonitorID monitor_Id = 0xFFFFFFFF;
	*pSeg >> monitor_Id;
	TRACESTR(eLevelInfoHigh) << "MonitorConfId:" << monitor_Id;

	// seems obsolete
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB)
	{
		const CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(monitor_Id);
		if (pConfRsrc)
		{
			if (STANDALONE_CONF_ID == pConfRsrc->GetRsrcConfId())
				pConfRsrcDB->RemoveConfRsrc(monitor_Id);
		}
		else
			TRACESTR(eLevelError) << "MonitorConfId:" << monitor_Id << " - Conference not found";
	}

	eRsrcConfType conf_type = eConf_type_none;
	status = m_pReservator->DeleteResAllTypes(monitor_Id, &conf_type);
	PASSERTSTREAM(status != STATUS_OK, "Status:" << status);

	// (**) send back to ConfMngr
	CSegment* pRetParam = new CSegment();
	pRetParam->Put((BYTE*)(&status), sizeof(DWORD));

	TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);
	CManagerApi api(eProcessConfParty);
	api.SendMsg(pRetParam, RSRC_DEACTIVATE_MR_IND);
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnSyncDeleteRsrvFromMrRequest(CSegment* pSeg)
{
	// (**) deallocate monitorId specific ( in segment)
	// (**) send back to ConfMngr

	STATUS status = STATUS_OK;
	ConfMonitorID monitor_Id = 0xFFFFFFFF;
	*pSeg >> monitor_Id;
	TRACESTR(eLevelInfoHigh) << "MonitorConfId:" << monitor_Id;

	// seems obsolete
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB)
	{
		const CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(monitor_Id);
		if (pConfRsrc)
		{
			if (STANDALONE_CONF_ID == pConfRsrc->GetRsrcConfId())
				pConfRsrcDB->RemoveConfRsrc(monitor_Id);
		}
		else
			TRACESTR(eLevelError) << "MonitorConfId:" << monitor_Id << " - Conference not found";
	}

	if (m_pReservator->IsInternalReservator())
	{
		eRsrcConfType conf_type = eConf_type_none;
		status = m_pReservator->DeleteResAllTypes(monitor_Id, &conf_type);
		if (status != STATUS_OK)
			PASSERT(status);
	}

	// /(**) send back to Conf
	CSegment* pRetParam = new CSegment();
	pRetParam->Put((BYTE*)(&status), sizeof(DWORD));
	ResponedClientRequest(RSRC_DEACTIVATE_MR_IND, pRetParam);
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnDeleteMrRequest(CSegment* pSeg)
{
	// (**) deallocate numericId specific ( in segment)
	// (**) deallocate monitorId specifdelete pCommResApi; //POBJDELETE?ic ( in segment)
	// (**) send back to ConfMngr

	STATUS status = STATUS_OK;
	ConfMonitorID monitor_Id = 0xFFFFFFFF;
	*pSeg >> monitor_Id;
	TRACESTR(eLevelInfoHigh) << "MonitorConfId:" << monitor_Id;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (!pSystemResources)
	{
		PASSERT(1);
		status = STATUS_FAIL;
	}
	else
	{
		// *** PSTN phones Deallocation
		CSleepingConference* pConf = (CSleepingConference*)(m_pReservator->GetSleepingConference(monitor_Id));
		if (!pConf)
		{
			PASSERT(1);
		}
		else
		{
			CServicePhoneStr* pPhoneStr = pConf->GetFirstServicePhone();
			while (pPhoneStr != NULL)
			{
				status = pSystemResources->DeAllocateServicePhones(*pPhoneStr);
				pPhoneStr = pConf->GetNextServicePhone();
			}
		}

		// *** PSTN phones Deallocation
		eRsrcConfType conf_type = eConf_type_none;
		status = m_pReservator->DeleteResAllTypes(monitor_Id, &conf_type);
		if (status != STATUS_OK)
			PASSERT(status);

		if (eConf_type_none != conf_type)
			m_pReservator->MinusNumConfPerType(conf_type);
	}

	// (**) send back to ConfMngr
	if (eRPSlaveMode != m_pReservator->GetRPMode())
	{
		CSegment* pRetParam = new CSegment();
		pRetParam->Put((BYTE*)(&status), sizeof(DWORD));

		TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);
		CManagerApi api(eProcessConfParty);
		api.SendMsg(pRetParam, RSRC_DEL_MR_IND);
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrvManager::OnUpdateRequest(CRequest* pRequest)
{
	STATUS status = STATUS_OK;

	CCommResAdd* pCommResAdd = new CCommResAdd;
	CCommResApi* pCommResApi = new CCommResApi;

	*pCommResAdd = *(CCommResAdd*)pRequest->GetRequestObject();
	*pCommResApi = *(pCommResAdd->GetCommResApi());

	DumpToTrace(pCommResApi, "CRsrvManager::OnUpdateRequest");

	if (pCommResApi->IsNormalRes())
	{
		status = OnUpdateResRequest(pCommResApi);  // it could be only in case of IsInternalReservator == true
	}
	else
	{
		status = OnUpdateMRRequest(pCommResApi);
	}

	// (***) answer to EMA !!!!
	std::string responseTrancsName("TRANS_RES"); // instead of TRANS_RES_1
	pRequest->SetTransName(responseTrancsName);
	pRequest->SetConfirmObject(pCommResApi);

	if (STATUS_OK == status && STATUS_OK != m_dwInternalConfStatus)
	{
		pRequest->SetStatus(m_dwInternalConfStatus | WARNING_MASK);   // olga (approved by Sergey M)
	}
	else
		pRequest->SetStatus(status);

	m_dwInternalConfStatus = 0;

	POBJDELETE(pCommResAdd);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrvManager::OnUpdateResRequest(CCommResApi* pCommResApi)
{
	return m_pReservator->UpdateRsrvFromAPIRequest(pCommResApi);
}

////////////////////////////////////////////////////////////////////////////
STATUS CRsrvManager::OnUpdateMRRequest(CCommResApi* pCommResApi)
{
	// (**) numeric id - update.
	// (**) return to ConfMngr if status OK - CCommResApi.
	// (**) return to EMA (status, CCommResApi)

	STATUS status = STATUS_OK;

	if (!(pCommResApi->IsMeetingRoom()) && (pCommResApi->GetEntryQ() != YES))
	{
		PASSERTSTREAM(1, "Not found");
		status = STATUS_FAIL;
	}

	eRsrcConfType c_type = eConf_type_none;
	if (pCommResApi->IsSIPFactory())
		c_type = eSipFact_type;
	else if (pCommResApi->GetEntryQ())
		c_type = eEQ_type;
	else if (pCommResApi->IsMeetingRoom())
		c_type = eMR_type;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (!pSystemResources)
	{
		PASSERT(1);
		status = STATUS_FAIL;
	}

	char*       numericId_old   = NULL;
	const char* numericId       = NULL;
	const char* numericIdFromDB = NULL;

	if (status == STATUS_OK)
	{
		// (**) numeric id treatment
		ConfMonitorID monitor_Id = pCommResApi->GetMonitorConfId();
		numericId = pCommResApi->GetNumericConfId();
		numericIdFromDB = m_pReservator->GetNumericIdbyMonitorId(monitor_Id);

		if (numericIdFromDB != NULL)
		{
			WORD numericId_old_len = strlen(numericIdFromDB);
			numericId_old = new char[numericId_old_len+1];
			strncpy(numericId_old, numericIdFromDB, numericId_old_len);
			numericId_old[numericId_old_len] = '\0';
		}

		if (numericId_old == NULL)
		{
			PASSERTSTREAM(1, "Not Found");
			status = STATUS_FAIL;
		}

		// ** update only in case numericId has really changed.
		else if (strcmp(numericId_old, numericId) != 0)
		{
			status = m_pReservator->AllocateNumericConfId((char*)numericId);

			if (status == STATUS_OK)
			{
				status = m_pReservator->UpdateSleepingConference(numericId, monitor_Id, c_type);

				if (status != STATUS_OK)
					m_pReservator->UpdateSleepingConference(numericId_old, monitor_Id, c_type);
				else
					m_pReservator->UpdateSleepingConference(numericId, monitor_Id, c_type);
			}
		}
	}

	// Phone Numbers Update
	// IMPORTANT : for now we assume that we can receive only one CServicePhoneStr to update (although in general the API supports multiple)
	if (status == STATUS_OK && (eEQ_type == c_type || eMR_type == c_type))
	{
		WORD l_bIsPstnEnabledInNew = false;
		if (pCommResApi->IsEnableIsdnPstnAccess())  // NETWORK_H320_H323 == pCommResApi->GetNetwork() )
			l_bIsPstnEnabledInNew = true;

		WORD l_bWasPstnEnabledInOld = false;
		ConfMonitorID monitor_Id = pCommResApi->GetMonitorConfId();
		CSleepingConference* pConfInDB = (CSleepingConference*)(m_pReservator->GetSleepingConference(monitor_Id));
		CServicePhoneStr* pPhoneStrFromDb = NULL;
		CServicePhoneStr* pPhoneStrOld = new CServicePhoneStr;

		if (pConfInDB)
		{
			pPhoneStrFromDb = pConfInDB->GetFirstServicePhone();
			if (pPhoneStrFromDb != NULL)
			{
				*pPhoneStrOld = *pPhoneStrFromDb;   // Need to save copy of the phone from DB for roll-back
				if (pPhoneStrOld->m_netServiceName[0] != '\0')
					l_bWasPstnEnabledInOld = true;
			}
		}

		CServicePhoneStr* pPhoneStrNew = pCommResApi->GetFirstServicePhone();

		// Case update from PSTN_ENABLED to PSTN_DISABLED
		// Need to deallocate Phone Numbers
		if (true == l_bWasPstnEnabledInOld && false == l_bIsPstnEnabledInNew)
		{
			status = pSystemResources->DeAllocateServicePhones(*pPhoneStrOld);
			if (status == STATUS_OK && pConfInDB)
			{
				status = pConfInDB->DeleteServicePhone(*pPhoneStrOld);
				if (status != STATUS_OK)      // roll-back
				{
					TRACESTRFUNC(eLevelError) << "Update failed due to phone number (case 1)";
					pSystemResources->AllocateServicePhones(*pPhoneStrOld);
				}
			}
		}
		// Case update from PSTN_DISABLED to PSTN_ENABLED
		else if (false == l_bWasPstnEnabledInOld && true == l_bIsPstnEnabledInNew)
		{
			if (pPhoneStrNew != NULL && pPhoneStrNew->m_netServiceName[0] != '\0') // SPECIFIC AND NON SPECIFIC CASE
			{
				if (m_pReservator->IsInternalReservator())
				{
					status = m_pReservator->AllocationPhoneMR_WithSchedule(pCommResApi, true);
				}
				else
				{
					status = pSystemResources->AllocateServicePhones(*pPhoneStrNew);
					if (status == STATUS_OK && pConfInDB)
					{
						status = pConfInDB->AddServicePhone(*pPhoneStrNew);
						if (status != STATUS_OK)    // roll-back
						{
							TRACESTRFUNC(eLevelError) << "Update failed due to phone number (case 2)";
							pSystemResources->DeAllocateServicePhones(*pPhoneStrNew);
						}
					}
				}
			}
		}

		// Case PSTN stayed enabled
		else if (true == l_bWasPstnEnabledInOld && true == l_bIsPstnEnabledInNew)
		{
			// Check if service name changed OR phone(s) changed - the treatment is same in both cases :
			// deallocate old phone(s) and allocate new

			// Check if one of the phone numbers has changed
			WORD l_bNumChanged = false;
			for (WORD i = 0;  i < MAX_PHONE_NUMBERS_IN_CONFERENCE; i++)
			{
				if (pPhoneStrNew != NULL && pPhoneStrOld != NULL)
				{
					if ((pPhoneStrNew->m_pPhoneNumberList[i] == NULL && pPhoneStrOld->m_pPhoneNumberList[i] != NULL) ||
					    (pPhoneStrNew->m_pPhoneNumberList[i] != NULL && pPhoneStrOld->m_pPhoneNumberList[i] == NULL) ||
					    (pPhoneStrNew->m_pPhoneNumberList[i] != NULL && pPhoneStrOld->m_pPhoneNumberList[i] != NULL &&
					     strcmp(pPhoneStrNew->m_pPhoneNumberList[i]->phone_number, pPhoneStrOld->m_pPhoneNumberList[i]->phone_number)))
					{
						l_bNumChanged = true;
						break;
					}
				}
				else
					PASSERT(1);
			}

			if (pPhoneStrNew && pPhoneStrOld && (strcmp(pPhoneStrNew->GetNetServiceName(), pPhoneStrOld->GetNetServiceName()) || l_bNumChanged))
			{
				if (pPhoneStrOld)
				{
					status = pSystemResources->DeAllocateServicePhones(*pPhoneStrOld);
					if (STATUS_OK == status)
					{
						status = pConfInDB->DeleteServicePhone(*pPhoneStrOld);
						if (status != STATUS_OK)          // roll-back
						{
							TRACESTRFUNC(eLevelError) << "Update failed due to phone number (case 3)";
							pSystemResources->AllocateServicePhones(*pPhoneStrOld);
						}
					}
				}

				if (STATUS_OK == status)
				{
					if (m_pReservator->IsInternalReservator())
					{
						status = m_pReservator->AllocationPhoneMR_WithSchedule(pCommResApi, true);
						if (STATUS_OK != status)
						{
							TRACESTRFUNC(eLevelError) << "Update failed due to phone number (case 4)";
							pSystemResources->AllocateServicePhones(*pPhoneStrOld);
							pConfInDB->AddServicePhone(*pPhoneStrOld);
						}
					}
					else
					{
						status = pSystemResources->AllocateServicePhones(*pPhoneStrNew);
						if (STATUS_OK == status)
						{
							status = pConfInDB->AddServicePhone(*pPhoneStrNew);
							if (status != STATUS_OK)        // roll-back
							{
								TRACESTRFUNC(eLevelError) << "Update failed due to phone number (case 5)";
								pSystemResources->DeAllocateServicePhones(*pPhoneStrNew);
								pSystemResources->AllocateServicePhones(*pPhoneStrOld);
								pConfInDB->AddServicePhone(*pPhoneStrOld);
							}
						}
						else            // roll-back
						{
							TRACESTRFUNC(eLevelError) << "Update failed due to phone number (case 6)";
							pSystemResources->AllocateServicePhones(*pPhoneStrOld);
							pConfInDB->AddServicePhone(*pPhoneStrOld);
						}
					}
				}
			}
		}

		// roll-back nid
		if (status != STATUS_OK)
		{
			if (numericId_old)
				m_pReservator->UpdateSleepingConference(numericId_old, monitor_Id, c_type);
		}

		POBJDELETE(pPhoneStrOld);
	}

	if (numericId_old)
		delete [] numericId_old;

	// End Phone Numbers Update

	if (STATUS_OK == status)  // //(**) send back to  to ConfMngr
	{
		CSegment* pRetParam = new CSegment();
		pCommResApi->Serialize(NATIVE, *pRetParam);
		pRetParam->Put((BYTE*)(&status), sizeof(DWORD));

		TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);
		CManagerApi api(eProcessConfParty);
		api.SendMsg(pRetParam, RSRC_MR_UPDATE_IND);
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnIPResourceRequest(CSegment* pSeg)
{
	STATUS status = STATUS_OK;
	DWORD serviceId;
	*pSeg >> serviceId;
	TRACESTR(eLevelInfoHigh) << "ServiceId:" << serviceId;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	ALLOC_REPORT_PARAMS_S* pResult = new ALLOC_REPORT_PARAMS_S;
	memset(pResult, 0, sizeof(ALLOC_REPORT_PARAMS_S));

	if (!pSystemResources || !pConfRsrcDB)
	{
		PASSERT(1);
		status = STATUS_FAIL;
	}
	else
	{
		CRsrcReport* pReport = new CRsrcReport();
		status = pSystemResources->CalculateResourceReport(pReport);
		if (status != STATUS_OK)
			PASSERT(status);
		else
		{
			pResult->max_num_ongoing_conf     = pSystemResources->GetMaxNumberOfOngoingConferences();
			pResult->current_num_ongoing_conf = pConfRsrcDB->GetNumConfRsrcs();

			pResult->m_numAudioParties[0] = pReport->GetNumParties(e_Audio, TYPE_TOTAL);
			pResult->m_numAudioParties[1] = pReport->GetNumParties(e_Audio, TYPE_OCCUPIED);
			pResult->m_numAudioParties[2] = pReport->GetNumParties(e_Audio, TYPE_FREE);

			// sum of all video types
			pResult->m_numVideoParties[0] = 0;
			pResult->m_numVideoParties[1] = 0;
			pResult->m_numVideoParties[2] = 0;

			for (int i = e_Cif; i < NUM_OF_PARTY_RESOURCE_TYPES; i++)
			{
				pResult->m_numVideoParties[0] += pReport->GetNumParties((ePartyResourceTypes)i, TYPE_TOTAL);
				pResult->m_numVideoParties[1] += pReport->GetNumParties((ePartyResourceTypes)i, TYPE_OCCUPIED);
				pResult->m_numVideoParties[2] += pReport->GetNumParties((ePartyResourceTypes)i, TYPE_FREE);
			}
		}

		POBJDELETE(pReport);
	}

	// pResult->status = status; ???
	CSegment*   pRetParam = new CSegment;
	pRetParam->Put(serviceId);
	pRetParam->Put((BYTE*)pResult, sizeof(ALLOC_REPORT_PARAMS_S));

	TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);
	CManagerApi api(eProcessConfParty);
	api.SendMsg(pRetParam, IP_RESOURCE_INFO_IND);

	delete pResult;
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnReadResDB(CSegment* pSeg)
{
	if (m_pReservator != NULL)
	{
		STATUS status = m_pReservator->InitInternalReservator();
		DWORD ret_status = (STATUS_OK == status) ? STATUS_OK : STATUS_FAIL;
		CSegment* pRetParam  = new CSegment;
		*pRetParam << ret_status;

		TRACEINTO << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);
		CManagerApi api(eProcessConfParty);
		api.SendMsg(pRetParam, RESOURCE_READY_IND);
	}
	else
		PASSERT(1);
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnTimeChangedInd(CSegment* pSeg)
{
	PASSERT_AND_RETURN(!m_pReservator);
	m_pReservator->TimeChanged();
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnStartConferenceTimerInd(CSegment* pSeg)
{
	PASSERT_AND_RETURN(!m_pReservator);
	m_pReservator->StartConferenceTimer();
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnDeleteConferenceTimerInd(CSegment* pSeg)
{
	PASSERT_AND_RETURN(!m_pReservator);
	m_pReservator->DeleteConferenceTimer();
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnDeleteRepeatedInd(CSegment* pSeg)
{
	PASSERT_AND_RETURN(!m_pReservator);
	m_pReservator->ActuallyDeleteRepeatedRes();
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::OnWriteRepeatedInd(CSegment* pSeg)
{
	PASSERT_AND_RETURN(!m_pReservator);
	m_pReservator->ActuallyWriteRepeatedRes();
	TRACEINTO << "Succeeded writing repeated reservations";
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::DumpToTrace(CCommResApi* pCommResApi, char* strNameOfFunction)
{
	std::ostringstream msg;
	msg << strNameOfFunction
	    << "\n  ConfName        :" << pCommResApi->GetName()
	    << "\n  MonitorConfId   :" << pCommResApi->GetMonitorConfId()
	    << "\n  IsSIPFactory    :" << (int)pCommResApi->IsSIPFactory()
	    << "\n  IsEntryQ        :" << (int)pCommResApi->GetEntryQ()
	    << "\n  IsMeetingRoom   :" << (int)pCommResApi->IsMeetingRoom()
	    << "\n  IsPermanent     :" << (int)pCommResApi->IsPermanent()
	    << "\n  NumericConfId   :" << pCommResApi->GetNumericConfId()
	    << "\n  Network         :" << (int)pCommResApi->GetNetwork()
	    << "\n  MinAudioParties :" << pCommResApi->GetNumOfMinAudioParties()
	    << "\n  MinVideoParties :" << pCommResApi->GetNumOfMinVideoParties()
	    << "\n  ServiceName     :" << pCommResApi->GetServiceNameForMinParties();

	CServicePhoneStr* pPhoneStr = pCommResApi->GetFirstServicePhone();
	if (pPhoneStr != NULL && pPhoneStr->m_netServiceName[0] != '\0')
	{
		Phone* phone = pPhoneStr->GetFirstPhoneNumber();
		msg << "\n  PhoneNumber     :" << (phone ? phone->phone_number : "Default");
	}
	PTRACE(eLevelInfoNormal, msg.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::DumpToTrace(CCommResRsrvDBAction* pCommResRsrvDBAction, char* strNameOfFunction)
{
	TRACESTRFUNC(eLevelInfoHigh) << "Called from function " << strNameOfFunction
			<< "\n  MonitorConfId:" << pCommResRsrvDBAction->GetConfID();
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::DumpToTrace(CComResRepeatDetails* pCommResRepeated, char* strNameOfFunction)
{
	std::ostringstream msg;
	msg << "Called from function " << strNameOfFunction
	    << "\n  LimitTime        :" << pCommResRepeated->GetLimitTime()
	    << "\n  RecurrenceType   :" << pCommResRepeated->GetRecurrenceType()
	    << "\n  OccurNumber      :" << pCommResRepeated->GetOccurNumber()
	    << "\n  EndBy            :" << pCommResRepeated->GetEndBy()
	    << "\n  Interval         :" << pCommResRepeated->GetInterval()
	    << "\n  DayOfMonth       :" << pCommResRepeated->GetDayOfMonth()
	    << "\n  Instance         :" << pCommResRepeated->GetInstance()
	    << "\n  DaysIndex        :" << pCommResRepeated->GetDaysIndex()
	    << "\n  ByMonth          :" << pCommResRepeated->GetByMonth()
	    << "\n  GMTOffsetMinutes :" << pCommResRepeated->GetGMTOffsetMinutes();

	for (int i = 0; i < 7; i++)
	{
		msg << "\n  WeekDay" << i << "         :" << pCommResRepeated->GetWeekDay(i);
	}

	TRACESTRFUNC(eLevelInfoHigh) << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::DumpToTrace(CSetEndTime* pStructTmDrv, char* strNameOfFunction)
{
	TRACESTRFUNC(eLevelInfoHigh) << "Called from function " << strNameOfFunction
			<< "\n  MonitorConfId:" << pStructTmDrv->GetConfID()
			<< "\n  Time         :" << *(pStructTmDrv->GetTime());
}

////////////////////////////////////////////////////////////////////////////
void CRsrvManager::ReceiveAdditionalParams(CSegment* pSeg) // olga (approved by Judith S.)
{
	m_dwInternalConfStatus = 0;
	if (pSeg)
		*pSeg >> m_dwInternalConfStatus;
}
