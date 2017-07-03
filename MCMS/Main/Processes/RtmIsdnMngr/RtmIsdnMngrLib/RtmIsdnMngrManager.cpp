// RtmIsdnMngrManager.cpp

#include "RtmIsdnMngrManager.h"
#include "OpcodesMcmsCommon.h"
#include "Trace.h"
#include "StatusesGeneral.h"
#include "RtmIsdnMngrProcess.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "ApiStatuses.h"
#include "ApacheDefines.h"
#include "Request.h"
#include "DummyEntry.h"
#include "FaultsDefines.h"
#include "RtmIsdnServiceList.h"
#include "RtmIsdnServiceName.h"
#include "RtmIsdnSpanMap.h"
#include "RtmIsdnSpanMapList.h"
#include "RtmIsdnSpanMapDel.h"
#include "RtmIsdnPhoneNumberRangeSet.h"
#include "RtmIsdnMngrInternalDefines.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "Segment.h"
#include "ObjString.h"
#include "TaskApi.h"
#include "SlotsNumberingConversionTableWrapper.h"
#include "ManagerApi.h"
#include "SNMPDefines.h"

extern char* ProcessTypeToString(eProcessType processType);

extern const char* SystemSpanTypeToString(eSystemSpanType eType);
extern const char* SpanTypeToString(eSpanType theType);

extern const char* SpanAlarmToString(eSpanAlarmType theType);
extern const char* DChannelStateToString(eDChannelStateType theType);
extern const char* ClockingToString(eClockingType theType);

#define RTM_ISDN_UNITIALIZED_BOARD_ID	-1
#define RTM_ISDN_RESPONSE_TIMEOUT		5*SECOND
#define PSTN_UNLICENSED_DESCRIPTION		"No License for ISDN/PSTN. Please activate the RTM ISDN card through Polycom website"
#define RTM_ISDN_GET_SERVICE_INFO_TIMER_TIME_OUT_VALUE		2*SECOND

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CRtmIsdnMngrManager)
  ONEVENT(XML_REQUEST,									IDLE,		CRtmIsdnMngrManager::HandlePostRequest )
  ONEVENT( RTM_ISDN_SPAN_STATUS_MCMS_IND,				ANYCASE,	CRtmIsdnMngrManager::OnRtmIsdnSpanStatusInd )
  ONEVENT( RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_IND,		ANYCASE,	CRtmIsdnMngrManager::OnRtmIsdnSpanDisableInd )
  ONEVENT( RTM_ISDN_PHONE_RANGE_DELETE_IF_UPDATABLE_IND,ANYCASE,	CRtmIsdnMngrManager::OnRtmIsdnPhoneRangeDelInd )
  ONEVENT( RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_IND,	ANYCASE,	CRtmIsdnMngrManager::OnRtmServiceCancelInd )
  ONEVENT( MCUMNGR_TO_RTMISDN_LICENSING_IND,			ANYCASE,	CRtmIsdnMngrManager::OnMcuMngrLicensingInd )
  ONEVENT( RTM_ISDN_ENTITY_LOADED_IND,					ANYCASE,	CRtmIsdnMngrManager::OnRtmIsdnEntityLoadedInd )
  ONEVENT( MFA_STARTUP_TIMER,							ANYCASE,	CRtmIsdnMngrManager::OnCardTimerStartupTimeout )
  ONEVENT( CARDS_SLOTS_NUMBERING_CONVERSION_TABLE_IND,	ANYCASE,	CRtmIsdnMngrManager::OnSlotsNumberingConversionTableInd ) 
  //SNMP
  ONEVENT( RTM_ISDN_GET_SERVICE_INFO_TIMER,			    ANYCASE,    CRtmIsdnMngrManager::OnTimerGetServiceInfoInd)
  ONEVENT( SNMP_CONFIG_TO_OTHER_PROGRESS,				ANYCASE,    CRtmIsdnMngrManager::OnSNMPConfigInd)
  ONEVENT( RTM_ISDN_NUM_PRI_CARD,	                        ANYCASE,    CRtmIsdnMngrManager::DetachSpansFor_NumPriRtmIsdnCard)
PEND_MESSAGE_MAP(CRtmIsdnMngrManager,CManagerTask);


////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CRtmIsdnMngrManager)
	ON_TRANS("TRANS_ISDN_SERVICE",	"NEW_ISDN_SERVICE",		CRtmIsdnService,	CRtmIsdnMngrManager::HandleSetNewService)
	ON_TRANS("TRANS_ISDN_SERVICE",	"UPDATE_ISDN_SERVICE",	CRtmIsdnService,	CRtmIsdnMngrManager::HandleSetUpdateService)
	ON_TRANS("TRANS_ISDN_SERVICE",	"DEL_ISDN_SERVICE",		CRtmIsdnServiceName,	CRtmIsdnMngrManager::HandleSetDelService)
	ON_TRANS("TRANS_ISDN_SERVICE",	"SET_DEFAULT_ISDN_SERVICE",CRtmIsdnServiceName,	CRtmIsdnMngrManager::HandleSetSetDefaultService)

	ON_TRANS("TRANS_ISDN_PHONE",	"ADD_ISDN_PHONE",		CRtmIsdnPhoneNumberRangeSet,	CRtmIsdnMngrManager::HandleSetAddPhoneNumRange)
//	ON_TRANS("TRANS_ISDN_PHONE",	"UPDATE_ISDN_PHONE",	CRtmIsdnPhoneNumberRangeSet,	CRtmIsdnMngrManager::HandleSetUpdatePhoneNumRange)
	ON_TRANS("TRANS_ISDN_PHONE",	"DEL_ISDN_PHONE",		CRtmIsdnPhoneNumberRangeSet,	CRtmIsdnMngrManager::HandleSetDelPhoneNumRange)

	ON_TRANS("TRANS_RTM_ISDN_SPAN",	"UPDATE_RTM_ISDN_SPAN",	CRtmIsdnSpanMap,				CRtmIsdnMngrManager::HandleSetUpdateSpan)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
//               Terminal Commands
////////////////////////////////////////////////////////////////////////////
BEGIN_TERMINAL_COMMANDS(CRtmIsdnMngrManager)

END_TERMINAL_COMMANDS

extern void RtmIsdnMngrMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void RtmIsdnMngrManagerEntryPoint(void* appParam)
{
	CRtmIsdnMngrManager * pRtmIsdnMngrManager = new CRtmIsdnMngrManager;
	pRtmIsdnMngrManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CRtmIsdnMngrManager::GetMonitorEntryPoint()
{
	return RtmIsdnMngrMonitorEntryPoint;
}





//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRtmIsdnMngrManager::CRtmIsdnMngrManager()
{
	m_pProcess = (CRtmIsdnMngrProcess*)CRtmIsdnMngrProcess::GetProcess();
	m_isSystemNormalWithSingleClockSource	= NO;
	m_isPstnLicensed						= NO;
	m_systemSpanType						= eSystemSpanType_Undefined;
	m_maxAllowedConfiguredSpans				= 0;

	for (int i=0; i<RTM_CARDS_ARRAY_LEN; i++)
	{
		m_RtmIsdnLoaded_BoardId[i] = RTM_ISDN_UNITIALIZED_BOARD_ID;
	}	
	m_bSNMPEnabled = FALSE;
}

//////////////////////////////////////////////////////////////////////
CRtmIsdnMngrManager::~CRtmIsdnMngrManager()
{
}

////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::ManagerPostInitActionsPoint()
{
	if ( eProductFamilySoftMcu == CProcessBase::GetProcess()->GetProductFamily() && eProductTypeNinja != CProcessBase::GetProcess()->GetProductType())
		return; // soft mcu should not have ISDN services


	CSysConfig::SwitchCfgFiles(RTM_ISDN_SERVICE_LIST_PATH, RTM_ISDN_SERVICE_LIST_TMP_PATH);

	// ===== 1. read service configuration
//	CRtmIsdnServiceList *pServiceListUpdated = m_pProcess->GetServiceListUpdated();
//	pServiceListUpdated->ReadXmlFile(RTM_ISDN_SERVICE_LIST_PATH, eNoActiveAlarm, eRenameFile);
//	pServiceListUpdated->SetUpdateCounter(0);
//
//	CRtmIsdnServiceList *pServiceListOriginal = m_pProcess->GetServiceListOriginal();
//	*pServiceListOriginal = *pServiceListUpdated;

	CRtmIsdnServiceList *pServicesListTmp = new CRtmIsdnServiceList;
	pServicesListTmp->ReadXmlFile(RTM_ISDN_SERVICE_LIST_PATH, eNoActiveAlarm, eRenameFile);
	eSpanType theType = pServicesListTmp->DeleteServiceWithInconsistentSpanType();

	if ( FALSE == pServicesListTmp->IsEmpty() )
		SetParamsAccordingToSpanType(theType);

	CRtmIsdnServiceList *pServiceListUpdated = m_pProcess->GetServiceListUpdated();

	if(!pServicesListTmp->IsDefaultExists())
	{
		CRtmIsdnService* pServProv = pServicesListTmp->GetFirstService();
		if (pServProv!=NULL)
		{
            const char* nameServ = pServProv->GetName();
            pServicesListTmp->SetDefaultName(nameServ, RTM_ISDN_SERVICE_LIST_PATH);
            TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::ManagerPostInitActionsPoint new defualt service name is "<<nameServ;
			RemoveActiveAlarmByErrorCode(AA_NO_DEFAULT_ISDN_SERVICE);
		}
	}

	*pServiceListUpdated = *pServicesListTmp;

	pServiceListUpdated->SetUpdateCounter(0);

	CRtmIsdnServiceList *pServiceListOriginal = m_pProcess->GetServiceListOriginal();
	*pServiceListOriginal = *pServicesListTmp;
	pServiceListOriginal->SetUpdateCounter(0);

	POBJDELETE(pServicesListTmp);


	// ===== 2. read span mapping configuration
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	pSpanMapList->ReadXmlFile(RTM_ISDN_SPAN_MAP_LIST_PATH, eNoActiveAlarm, eRenameFile);
	pSpanMapList->SetUpdateCounter(0);

	// ===== 3. various checkings
	pSpanMapList->DetachSpanOfNonExistingService();
	pSpanMapList->DetachExceededNumOfConfiguredSpans(m_maxAllowedConfiguredSpans);

	CSysConfig *sysConfig = m_pProcess->GetSysConfig();
	sysConfig->GetBOOLDataByKey( CFG_KEY_SYSTEM_NORMAL_WITH_SINGLE_CLOCK_SOURCE, m_isSystemNormalWithSingleClockSource );

	//For SNMP
	CManagerApi api(eProcessSNMPProcess);
	CSegment *pSeg = new CSegment;
	DWORD type = eProcessRtmIsdnMngr;
	*pSeg << type;
	api.SendMsg(pSeg, SNMP_OTHER_PROCESS_READY);
	
	StartTimer(RTM_ISDN_GET_SERVICE_INFO_TIMER, RTM_ISDN_GET_SERVICE_INFO_TIMER_TIME_OUT_VALUE);
}

////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::ManagerStartupActionsPoint()
{
	SendServiceParamsStructToProcesses();
	SendDefaultServiceNameToProcesses();

//	Cards process will get the list to CRtmIsdnMngrManager::OnMcuMngrLicensingInd, since it needs the flag of PSTN in licensing
//	SendSpansMapListStructToCardsProcess();
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::IgnoreMessage()
{
	// (nothing to be done)
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::OnCardTimerStartupTimeout()
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnCardTimerStartupTimeout";

	for (int i=1; i<RTM_CARDS_ARRAY_LEN; i++)
	{
		////////**/////////CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();

		AddNoRtmCardActiveAlarmIfNeeded(i);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::OnSlotsNumberingConversionTableInd(CSegment* pMsg)
{
    SLOTS_NUMBERING_CONVERSION_TABLE_S* pSlotsNumTable = new SLOTS_NUMBERING_CONVERSION_TABLE_S; AUTO_DELETE(pSlotsNumTable);
	pMsg->Get((BYTE*)pSlotsNumTable, sizeof(SLOTS_NUMBERING_CONVERSION_TABLE_S));

	m_pProcess->SetSlotsNumberingConversionTable(pSlotsNumTable);
	CSlotsNumberingConversionTableWrapper* pCurTable = m_pProcess->GetSlotsNumberingConversionTable();
	pCurTable->PrintData("\nCRtmIsdnMngrManager::OnSlotsNumberingConversionTableInd");
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendServiceParamsStructToProcesses()
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendServiceParamsStructToProcesses";

	// ===== 1. prepare data - list of services - to send
	RTM_ISDN_PARAMS_MCMS_S isdnParamsStruct;

	CRtmIsdnServiceList *pServiceList = m_pProcess->GetServiceListOriginal();
	if (pServiceList)
	{
		CRtmIsdnService* pCurService = pServiceList->GetFirstService();
		while (pCurService)
		{
			pCurService->ConvertToRtmIsdnParamsMcmsStruct(isdnParamsStruct);

			// ===== 2. send
			SendServiceParamsStructToSpecProcess(isdnParamsStruct, eProcessConfParty);
			SendServiceParamsStructToSpecProcess(isdnParamsStruct, eProcessResource);
			SendServiceParamsStructToSpecProcess(isdnParamsStruct, eProcessCards);

			pCurService = pServiceList->GetNextService();
		}
	}

	// ===== 3. send End_List
	SendServiceParamsEndToProcesses();

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendServiceParamsStructToSpecProcess(RTM_ISDN_PARAMS_MCMS_S &theStruct, eProcessType theProcess)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendServiceParamsStructToSpecProcess"
	                       << "\nProcess: " << ::ProcessTypeToString(theProcess);

	m_rtmIsdnCommonMethods.PrintRtmIsdnParamsMcmsStruct(theStruct, "CRtmIsdnMngrManager::SendServiceParamsStructToSpecProcess");

	// ===== 1. fill the Segment with items from the list
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)&theStruct, sizeof(RTM_ISDN_PARAMS_MCMS_S) );

	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(theProcess, eManager);

	STATUS res = pMbx->Send(pRetParam,RTM_ISDN_PARAMS_IND,&GetRcvMbx());

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendServiceParamsEndToProcesses()
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendServiceParamsEndToProcesses";

	SendServiceParamsEndToSpecProcess(eProcessCards);
	SendServiceParamsEndToSpecProcess(eProcessConfParty);
	SendServiceParamsEndToSpecProcess(eProcessResource);

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendServiceParamsEndToSpecProcess(eProcessType theProcess)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendServiceParamsEndToSpecProcess"
	                       << "\nProcess: " << ::ProcessTypeToString(theProcess);

	CSegment*  pRetParam = new CSegment;
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(theProcess, eManager);

	STATUS res = pMbx->Send(pRetParam,RTM_ISDN_PARAMS_END_IND, &GetRcvMbx());

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendDefaultServiceNameToProcesses()
{
	CRtmIsdnServiceList *pServiceList = m_pProcess->GetServiceListOriginal();

	if ( true == pServiceList->IsDefaultExists() )
	{
		// ===== 1. prepare data to send
		RTM_ISDN_SERVICE_NAME_S defServNameStruct;
		memset(&defServNameStruct, 0, sizeof(RTM_ISDN_SERVICE_NAME_S));
		memcpy( &(defServNameStruct.serviceName), pServiceList->GetDefaultName(), RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );

		// ===== 2. send
		SendDefaultServiceNameToSpecProcess(defServNameStruct, eProcessConfParty);

		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendDefaultServiceNameToProcesses"
		                       << "\nDefault Service name: " << pServiceList->GetDefaultName();
		RemoveActiveAlarmByErrorCode(AA_NO_DEFAULT_ISDN_SERVICE);
	}
	else if (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())// defaultService does not exist
	{


		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendDefaultServiceNameToProcesses"
		                       << "\nNo Default Service exists";
		//AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_NO_DEFAULT_ISDN_SERVICE, MAJOR_ERROR_LEVEL, "No default ISDN/PSTN Network Service defined in ISDN/PSTN Network Services list", true, true);
	}

	return;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendDefaultServiceNameToSpecProcess(RTM_ISDN_SERVICE_NAME_S &theStruct, eProcessType theProcess)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendDefaultServiceNameToSpecProcess"
	                       << "\nProcess: " << ::ProcessTypeToString(theProcess);

	m_rtmIsdnCommonMethods.PrintRtmIsdnServiceNameStruct(theStruct, "CRtmIsdnMngrManager::SendDefaultServiceNameToSpecProcess");

	// ===== 1. fill the Segment
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)&theStruct, sizeof(RTM_ISDN_SERVICE_NAME_S) );

	// ===== 2. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(theProcess, eManager);

	STATUS res = pMbx->Send(pRetParam,RTM_ISDN_DEFAULT_SERVICE_NAME_IND,&GetRcvMbx());

	return;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::HandleSetNewService(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nRtmIsdnMngrManager::HandleSetNewRtmIsdnService";

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	// ===== 1. check & retrieve data
	if(pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "\nRtmIsdnMngrManager::HandleSetNewRtmIsdnService - No permission to add isdn service";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CRtmIsdnService *pNewService = (CRtmIsdnService*)pRequest->GetRequestObject();
	if(!pNewService)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetNewRtmIsdnService - service does not exist in request";
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	CRtmIsdnSpanDefinition *pSpanDef = pNewService->GetSpanDef();
	if (pSpanDef)
	{
		if ( false == IsAcceptedSpanType(pSpanDef->GetSpanType()) )
		{
			CLargeString errStr = "\nCRtmIsdnMngrManager::HandleSetNewRtmIsdnService";
			errStr << "\nUnacceptable Span Type -"
			       << "\nNew service's span type: "	<< ::SpanTypeToString( pSpanDef->GetSpanType() )
			       << "; system's span type: "		<< ::SystemSpanTypeToString(m_systemSpanType);
			TRACESTR(eLevelInfoNormal) << errStr.GetString();

			pRequest->SetExDescription( "Wrong span type (all span types in the system must be the same)" );
			pRequest->SetStatus(STATUS_INCONSISTENT_PARAMETERS);
			return STATUS_OK;
		}
	}


	// ===== 2. add the service
	CRtmIsdnServiceList *pServListOriginal = m_pProcess->GetServiceListOriginal(),
	                    *pServListUpdated  = m_pProcess->GetServiceListUpdated();

	STATUS addOriginalStat = pServListOriginal->Add(*pNewService, RTM_ISDN_SERVICE_LIST_PATH);
	if (STATUS_OK != addOriginalStat)
	{
		pRequest->SetStatus(addOriginalStat);
		return STATUS_OK;
	}

	STATUS addUpdatedStat  = pServListUpdated->Add(*pNewService, RTM_ISDN_SERVICE_LIST_TMP_PATH);
	if (STATUS_OK != addUpdatedStat)
	{
		pRequest->SetStatus(addUpdatedStat);
		return STATUS_OK;
	}
	else
	{
		if (pServListUpdated->IsDefaultExists())
			RemoveActiveAlarmByErrorCode(AA_NO_DEFAULT_ISDN_SERVICE);
		else
			TRACEINTO << " No deault service defined after adding service \"" << pNewService->GetName() << "\"";
	}
	// on STATUS_OK
	SendServiceToProcesses(*pNewService);


	pRequest->SetStatus(STATUS_OK);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendServiceToProcesses(CRtmIsdnService &other)
{
	RTM_ISDN_PARAMS_MCMS_S isdnParamsStruct;
	other.ConvertToRtmIsdnParamsMcmsStruct(isdnParamsStruct);

	SendServiceParamsStructToSpecProcess(isdnParamsStruct, eProcessCards);
	SendServiceParamsStructToSpecProcess(isdnParamsStruct, eProcessConfParty);
	SendServiceParamsStructToSpecProcess(isdnParamsStruct, eProcessResource);
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::HandleSetUpdateService(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetUpdateService";

	// ===== 1. check & retrieve data
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if(pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetUpdateService - No permission to update isdn service";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CRtmIsdnService* pServiceToUpdate = (CRtmIsdnService*)pRequest->GetRequestObject();
	if(!pServiceToUpdate)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetUpdateService - service does not exist in request";
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}

	CRtmIsdnServiceList *pServListUpdated = m_pProcess->GetServiceListUpdated();

	STATUS updateStat = pServListUpdated->Update(*pServiceToUpdate);
	if (STATUS_OK == updateStat)
	{
		AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
				SYSTEM_CONFIGURATION_CHANGED,
				 MAJOR_ERROR_LEVEL,
				 "System configuration changed. Please reset the MCU.",
				 true,
				 true
				);

	}

	pRequest->SetStatus(updateStat);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::HandleSetDelService(CRequest* pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetDelService";

	// ===== 1. check & retrieve data
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if(pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetDelService - No permission to delete isdn service";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CRtmIsdnServiceName* pServiceToDelete = (CRtmIsdnServiceName*)pRequest->GetRequestObject();
	if(!pServiceToDelete)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetDelService - service does not exist in request";
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}


	// ===== 2. check if deletion possible

	// --- 2a. check legality
	CRtmIsdnServiceList *pServListOriginal = m_pProcess->GetServiceListOriginal(),
	                    *pServListUpdated  = m_pProcess->GetServiceListUpdated();

	const char* serviceName = pServiceToDelete->GetName();

	STATUS delOriginalStat = pServListOriginal->GetCancelServiceStat(serviceName);
	if (STATUS_OK != delOriginalStat)
	{
		pRequest->SetStatus(delOriginalStat);
		return STATUS_OK;
	}

	STATUS delUpdatedStat  = pServListUpdated->GetCancelServiceStat(serviceName);
	if (STATUS_OK != delUpdatedStat)
	{
		pRequest->SetStatus(delUpdatedStat);
		return STATUS_OK;
	}


	// --- 2b. ask Resource process to delete
	RTM_ISDN_SERVICE_CANCEL_S serviceToCancel;
	memset(&serviceToCancel, 0, sizeof(RTM_ISDN_SERVICE_CANCEL_S));
	memcpy( &(serviceToCancel.serviceName), serviceName, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );

	STATUS retStat = SendMsgSyncCancelServiceToResourceProcess(serviceToCancel);
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetDelService"
	                       << "\nStatus from Resource process: " << m_pProcess->GetStatusAsString(retStat);

	// ===== 3. actually delete the service
	if (STATUS_OK == retStat)
	{
		bool wasDefaultServiceDeleted = pServListOriginal->IsDefault(serviceName);

		retStat = DeleteService(serviceName, false/*legality check was done at 2a*/);
		if (STATUS_OK == retStat)
		{
			SendDeleteServiceToSpecProcess(serviceName, eProcessCards);
			SendDeleteServiceToSpecProcess(serviceName, eProcessConfParty);

			for (int i=1; i<RTM_CARDS_ARRAY_LEN; i++)
			     AddNoRtmCardActiveAlarmIfNeeded(i);

			// DefaultService was changed - process should be updated
			if (true == wasDefaultServiceDeleted)
			{
				SendDefaultServiceNameToProcesses();
			}
		}
	} // end retStat (of Resource process answer) == ok


	if ( TRUE == pServListOriginal->IsEmpty() )
	{
		RemoveActiveAlarmByErrorCode(AA_NO_ISDN_PSTN_LICENSING);

		m_systemSpanType			= eSystemSpanType_Undefined;
		m_maxAllowedConfiguredSpans	= 0;
	}


	pRequest->SetStatus(retStat);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendDeleteServiceToSpecProcess(const char* serviceName, eProcessType theProcess)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendDeleteServiceToSpecProcess"
	                       << "\nProcess: " << ::ProcessTypeToString(theProcess);

	// ===== 1. prepare data to send
	RTM_ISDN_SERVICE_NAME_S servNameStruct;
	memset(&servNameStruct, 0, sizeof(RTM_ISDN_SERVICE_NAME_S));
	memcpy( &(servNameStruct.serviceName), serviceName, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );


	m_rtmIsdnCommonMethods.PrintRtmIsdnServiceNameStruct(servNameStruct, "CRtmIsdnMngrManager::SendDeleteServiceToSpecProcess");

	// ===== 2. send
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)&servNameStruct, sizeof(RTM_ISDN_SERVICE_NAME_S) );

	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(theProcess, eManager);

	STATUS res = pMbx->Send(pRetParam, RTM_ISDN_DELETE_SERVICE_IND, &GetRcvMbx());

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::SendMsgSyncCancelServiceToResourceProcess(const RTM_ISDN_SERVICE_CANCEL_S &serviceToCancel)
{
	STATUS retStat = STATUS_OK;

	// ===== 1. print
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncCancelServiceToResourceProcess";
	m_rtmIsdnCommonMethods.PrintRtmIsdnServiceCancelStruct(serviceToCancel, "CRtmIsdnMngrManager::SendMsgSyncCancelServiceToResourceProcess - send");

	// ===== 2. fill the Segment with items from the list
	CSegment*  pSegSent = new CSegment;
	pSegSent->Put( (BYTE*)&serviceToCancel, sizeof(RTM_ISDN_SERVICE_CANCEL_S) );

	// ===== 3. send
	CSegment rspMsg;
	OPCODE rspOpcode;
	CTaskApi resourceManagerApi(eProcessResource, eManager);
	STATUS responseStatus = resourceManagerApi.SendMessageSync(pSegSent, RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_REQ, RTM_ISDN_RESPONSE_TIMEOUT, rspOpcode, rspMsg);

	// ===== 2. get the response
	if (STATUS_OK == responseStatus)
	{
        if ( 0 < rspMsg.GetLen() )
        {
        	RTM_ISDN_SERVICE_CANCEL_S retServiceToCancel;
			rspMsg.Get( (BYTE*)(&retServiceToCancel), sizeof(RTM_ISDN_SERVICE_CANCEL_S) );
			m_rtmIsdnCommonMethods.PrintRtmIsdnServiceCancelStruct(retServiceToCancel, "CRtmIsdnMngrManager::SendMsgSyncCancelServiceToResourceProcess - response");

			retStat = retServiceToCancel.status;
			if (STATUS_OK == retStat)
			{
				TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncCancelServiceToResourceProcess"
				                       << "\nOpcode: RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_REQ"
				                       << "\nResource process approves to delete the service"
			                           << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";
			}
			else // retStat != OK
			{
				TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncCancelServiceToResourceProcess"
				                       << "\nOpcode: RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_REQ"
				                       << "\nResource process refuses to delete the service"
			                           << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";
	  		}
        }
        else // no content in response
        {
            retStat = STATUS_ILLEGAL;
			TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncCancelServiceToResourceProcess"
			                       << "\nOpcode: RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_REQ"
			                       << "\nNo content in response"
			                       << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";
        }

    }

	else // responseStatus != OK (i.e. timeout or dead process)
	{
		retStat = responseStatus;
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncCancelServiceToResourceProcess"
		                       << "\nOpcode: RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_REQ"
		                       << "\nResponse timeout"
		                       << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";

	}

	return retStat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::DeleteService(const char* serviceName, bool isToCheckStat)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DeleteService";

	// ===== 1. delete from lists
	CRtmIsdnServiceList *pServListOriginal = m_pProcess->GetServiceListOriginal();
	CRtmIsdnServiceList *pServListUpdated  = m_pProcess->GetServiceListUpdated();

	STATUS delOriginalStat = pServListOriginal->Cancel(serviceName, isToCheckStat, RTM_ISDN_SERVICE_LIST_PATH);
	if (STATUS_OK != delOriginalStat)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DeleteService"
		                              << "\nFailed to delete Service from original list. Status: "
		                              << m_pProcess->GetStatusAsString(delOriginalStat);
		return delOriginalStat;
	}

	STATUS delUpdatedStat = pServListUpdated->Cancel(serviceName, isToCheckStat, RTM_ISDN_SERVICE_LIST_TMP_PATH);
	if (STATUS_OK != delUpdatedStat)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DeleteService"
		                              << "\nFailed to delete Service from updated list. Status: "
		                              << m_pProcess->GetStatusAsString(delUpdatedStat);
		return delUpdatedStat;
	}


	//  ===== 2. on STATUS_OK - detach all spans that are attached to the deleted service
	m_pProcess->GetSpanMapList()->DetachAllSpanMapsOfService(serviceName);


	//  ===== 3. remove unnecessary Alerts
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	if (pSpanMapList)
	{
		for (int boardIdx=0; boardIdx<MAX_NUM_OF_BOARDS; boardIdx++)
		{
			if ( false == pSpanMapList->IsAnySpanAttachedOnBoard(boardIdx) )
			{
				RemoveActiveAlarmFaultOnlyByErrorCodeUserId(AA_SINGLE_CLOCK_SOURCE, boardIdx);
				RemoveActiveAlarmByErrorCodeUserId(AA_NO_CLOCK_SOURCE, boardIdx);
			}
		}
	}


	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::HandleSetSetDefaultService(CRequest* pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetSetDefaultService";

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if(pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetSetDefaultService - No permission to change default isdn service";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CRtmIsdnServiceName* pServiceToSetAsDefault = (CRtmIsdnServiceName*)pRequest->GetRequestObject();
	if(!pServiceToSetAsDefault)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetSetDefaultService - service does not exist in request";
		pRequest->SetStatus(STATUS_SERVICE_PROVIDER_NAME_NOT_EXISTS);
		return STATUS_OK;
	}


	CRtmIsdnServiceList *pServListOriginal = m_pProcess->GetServiceListOriginal(),
	                    *pServListUpdated  = m_pProcess->GetServiceListUpdated();

	pServListOriginal->SetDefaultName(pServiceToSetAsDefault->GetName(), RTM_ISDN_SERVICE_LIST_PATH);
	pServListUpdated->SetDefaultName(pServiceToSetAsDefault->GetName(), RTM_ISDN_SERVICE_LIST_TMP_PATH);

	// update processes
	SendDefaultServiceNameToProcesses();

	pRequest->SetStatus(STATUS_OK);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::HandleSetAddPhoneNumRange(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetAddPhoneNumRange";

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	// ===== 1. check & retrieve data
	if(pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetAddPhoneNumRange - No permission to add phone range";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CRtmIsdnPhoneNumberRangeSet* pPhoneRangeToAdd = (CRtmIsdnPhoneNumberRangeSet*)pRequest->GetRequestObject();
	if(!pPhoneRangeToAdd)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetAddPhoneNumRange - phone range does not exist in request";
		pRequest->SetStatus(STATUS_PHONE_RANGE_NOT_EXISTS);
		return STATUS_OK;
	}

	CRtmIsdnPhoneNumberRange* pPhoneRange = pPhoneRangeToAdd->GetPhoneNumRange();

	string firstNum		= pPhoneRange->GetFirstPhoneNumber(),
	       lastNum		= pPhoneRange->GetLastPhoneNumber();
	if ( firstNum.empty() || lastNum.empty() )
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetAddPhoneNumRange - empty phone number";
		pRequest->SetStatus(STATUS_ILLEGAL_PHONE_RANGE);
		return STATUS_OK;
	}


	// ===== 2. add
	CRtmIsdnServiceList *pServListOriginal = m_pProcess->GetServiceListOriginal();
	CRtmIsdnServiceList *pServListUpdated  = m_pProcess->GetServiceListUpdated();

	const char* serviceName = pPhoneRangeToAdd->GetServiceName();

	STATUS addOriginalStat = pServListOriginal->AddPhoneNumRange( serviceName,
	                                                              *pPhoneRange,
	                                                              RTM_ISDN_SERVICE_LIST_PATH);
	if (STATUS_OK != addOriginalStat)
	{
		pRequest->SetStatus(addOriginalStat);
		return STATUS_OK;
	}

	STATUS addUpdatedStat  = pServListUpdated->AddPhoneNumRange( serviceName,
	                                                             *pPhoneRange,
	                                                             RTM_ISDN_SERVICE_LIST_TMP_PATH);
	if (STATUS_OK != addUpdatedStat)
	{
		pRequest->SetStatus(addUpdatedStat);
		return STATUS_OK;
	}

	// update processes
	SendUpdatePhoneRangeReqToProcesses(RTM_ISDN_ADD_PHONE_RANGE_REQ, serviceName, *pPhoneRange);

	pRequest->SetStatus(STATUS_OK);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
/*
// update phone - not implemented
STATUS CRtmIsdnMngrManager::HandleSetUpdatePhoneNumRange(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetUpdatePhoneNumRange";

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	// ===== 1. check & retrieve data
	if(pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetUpdatePhoneNumRange - No permission to update phone range";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CRtmIsdnPhoneNumberRangeSet* pPhoneRangeToUpdate = (CRtmIsdnPhoneNumberRangeSet*)pRequest->GetRequestObject();
	if(!pPhoneRangeToUpdate)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetUpdatePhoneNumRange - phone range does not exist in request";
		pRequest->SetStatus(STATUS_PHONE_RANGE_NOT_EXISTS);
		return STATUS_OK;
	}

	// ===== 2. update
	CRtmIsdnServiceList *pServListOriginal = m_pProcess->GetServiceListOriginal();
	CRtmIsdnServiceList *pServListUpdated = m_pProcess->GetServiceListUpdated();

	const char* serviceName = pPhoneRangeToUpdate->GetServiceName();
	CRtmIsdnPhoneNumberRange* pPhoneRange = pPhoneRangeToUpdate->GetPhoneNumRange();

	STATUS updateOriginalStat = pServListOriginal->UpdatePhoneNumRange( serviceName,
	                                                                    *pPhoneRange,
	                                                                    RTM_ISDN_SERVICE_LIST_PATH);
	if (STATUS_OK != updateOriginalStat)
	{
		pRequest->SetStatus(updateOriginalStat);
		return STATUS_OK;
	}


	STATUS updateUpdatedStat = pServListUpdated->UpdatePhoneNumRange( serviceName,
	                                                                  *pPhoneRange,
	                                                                  RTM_ISDN_SERVICE_LIST_TMP_PATH);
	if (STATUS_OK != updateUpdatedStat)
	{
		pRequest->SetStatus(updateUpdatedStat);
		return STATUS_OK;
	}


	pRequest->SetStatus(STATUS_OK);
	return STATUS_OK;
}
*/

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::HandleSetDelPhoneNumRange(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetDelPhoneNumRange";

	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	// ===== 1. check & retrieve data
	if(pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetDelPhoneNumRange - No permission to delete phone range";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CRtmIsdnPhoneNumberRangeSet* pPhoneRangeToDelete = (CRtmIsdnPhoneNumberRangeSet*)pRequest->GetRequestObject();
	if(!pPhoneRangeToDelete)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetDelPhoneNumRange - phone range does not exist in request";
		pRequest->SetStatus(STATUS_PHONE_RANGE_NOT_EXISTS);
		return STATUS_OK;
	}

	// ===== 2. check if deletion possible

	// --- 2a. check legality
	CRtmIsdnServiceList *pServListOriginal = m_pProcess->GetServiceListOriginal();
	CRtmIsdnServiceList *pServListUpdated  = m_pProcess->GetServiceListUpdated();

	const char* serviceName = pPhoneRangeToDelete->GetServiceName();
	CRtmIsdnPhoneNumberRange* pPhoneRange = pPhoneRangeToDelete->GetPhoneNumRange();

	STATUS delOriginalStat = pServListOriginal->GetUpdatePhoneNumRangeStat(serviceName,*pPhoneRange);
	if (STATUS_OK != delOriginalStat)
	{
		pRequest->SetStatus(delOriginalStat);
		return STATUS_OK;
	}

	STATUS delUpdatedStat  = pServListUpdated->GetUpdatePhoneNumRangeStat(serviceName, *pPhoneRange);
	if (STATUS_OK != delUpdatedStat)
	{
		pRequest->SetStatus(delUpdatedStat);
		return STATUS_OK;
	}


	STATUS retStat = STATUS_OK;
	// --- 2b. ask Resource process to delete
	retStat = SendUpdatePhoneRangeReqToProcesses(RTM_ISDN_PHONE_RANGE_DELETE_IF_UPDATABLE_REQ, serviceName, *pPhoneRange);
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetDelPhoneNumRange"
	                       << "\nStatus from Resource process: " << m_pProcess->GetStatusAsString(retStat);

	// ===== 3. actually delete the phone range
	if (STATUS_OK == retStat)
	{
		retStat = DeletePhoneRange(serviceName, *pPhoneRange, false/*legality check was done at 2a*/);
	}


	pRequest->SetStatus(retStat);
	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::HandleSetUpdateSpan(CRequest *pRequest)
{
	TRACESTR(eLevelInfoNormal) << "\nRtmIsdnMngrManager::HandleSetUpdateSpan";

	// ===== 1. check & retrieve data
	pRequest->SetConfirmObject(new CDummyEntry);
	pRequest->SetStatus(STATUS_OK);

	if(pRequest->GetAuthorization() != SUPER)
	{
		TRACESTR(eLevelInfoNormal) << "\nRtmIsdnMngrManager::HandleSetUpdateSpan - No permission to add isdn span";
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	CRtmIsdnSpanMap *pSpanMapToUpdate = (CRtmIsdnSpanMap*)pRequest->GetRequestObject();
	if(!pSpanMapToUpdate)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetUpdateSpan - span does not exist in request";
		pRequest->SetStatus(STATUS_SPAN_MAP_NOT_EXISTS);
		return STATUS_OK;
	}

	WORD spansDisplayBoardId = pSpanMapToUpdate->GetBoardId();
	if ( (true  == pSpanMapToUpdate->GetIsAttachedToService()) &&				// is it attaching Span...
	     (false == IsAcceptedAdditionalConfiguredSpan(spansDisplayBoardId)) )	//   ...but max num of allowed spans is exceeded on that board
	{
		CLargeString errStr = "\nCRtmIsdnMngrManager::HandleSetUpdateSpan";
		errStr << "\nThe allowed number of configured spans (" << m_maxAllowedConfiguredSpans << ") has been reached";
		TRACESTR(eLevelInfoNormal) << errStr.GetString();

		pRequest->SetStatus(STATUS_NUMBER_OF_ISDN_SPAN_MAPS_EXCEEDED);
		return STATUS_OK;
	}


	// ===== 2. update (in list & other processes)

	// --- 2a. check legality
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	STATUS updateStat = pSpanMapList->GetUpdateSpanMapStat(*pSpanMapToUpdate);
	if (STATUS_OK != updateStat)
	{
		pRequest->SetStatus(updateStat);
		return STATUS_OK;
	}

	// --- 2b. attach Span
	if (true == pSpanMapToUpdate->GetIsAttachedToService())
	{
		// attach in list
		pSpanMapList->UpdateSpanMap(*pSpanMapToUpdate, false/*legality check was done at 2a*/);

		// ----- check if RTM board exists
		AddNoRtmCardActiveAlarmIfNeeded(spansDisplayBoardId);
		// ----- check if PSTN is licensed
		if (YES == m_isPstnLicensed)
		{
			// send to Cards process
			RTM_ISDN_SPAN_MAP_S spanMapStruct;
			spanMapStruct.boardId	= spansDisplayBoardId;
			spanMapStruct.spanId	= pSpanMapToUpdate->GetSpanId();
			memset(spanMapStruct.serviceName, 0, RTM_ISDN_SERVICE_PROVIDER_NAME_LEN);
			memcpy( &(spanMapStruct.serviceName), pSpanMapToUpdate->GetServiceName(), RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );

			SendAttachSpanMapToCardsProcess(spanMapStruct);
		}
		else
		{
			AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
					                 AA_NO_ISDN_PSTN_LICENSING,
			                         MAJOR_ERROR_LEVEL,
			                         PSTN_UNLICENSED_DESCRIPTION,
			                         true,
			                         true
			                        );
		}

		pRequest->SetStatus(STATUS_OK);

	} // end attachSpan

	// --- 2c. detach Span
	else
	{
		CSlotsNumberingConversionTableWrapper* pCurTable = m_pProcess->GetSlotsNumberingConversionTable();
		DWORD spansActualBoardId = pCurTable->GetBoardId(spansDisplayBoardId, RTM_ISDN_SUBBOARD_ID);

		if (CProcessBase::GetProcess()->GetProductType() == eProductTypeNinja)
			spansActualBoardId = 1;
		
		SPAN_DISABLE_S spanDisableStruct_ForRsrc;
		spanDisableStruct_ForRsrc.boardId	= spansActualBoardId;
		spanDisableStruct_ForRsrc.spanId	= pSpanMapToUpdate->GetSpanId();
		spanDisableStruct_ForRsrc.status	= STATUS_OK;

		STATUS retStat = SendMsgSyncDisableSpanMapToResourceProcess(spanDisableStruct_ForRsrc);
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::HandleSetUpdateSpan"
		                       << "\nStatus from Resource process: " << m_pProcess->GetStatusAsString(retStat);

		// ===== 3. actually detach the span
		if (STATUS_OK == retStat)
		{
			SPAN_DISABLE_S spanDisableStruct_ForCards;
			spanDisableStruct_ForCards.boardId	= spansDisplayBoardId;
			spanDisableStruct_ForCards.spanId	= pSpanMapToUpdate->GetSpanId();
			spanDisableStruct_ForCards.status	= STATUS_OK;

			retStat = DetachSpanMap(spanDisableStruct_ForCards);

			// ---- treat Alerts
			TreatClockSourceStateAlerts(spansDisplayBoardId);

			if ( NO == pSpanMapList->IsAnySpanAttachedToService() )
			{
				RemoveActiveAlarmByErrorCode(AA_NO_ISDN_PSTN_LICENSING);
			}
		}

		pRequest->SetStatus(retStat);

	} // end detachSpan


	return STATUS_OK;
}

void CRtmIsdnMngrManager::DetachSpansFor_NumPriRtmIsdnCard(CSegment* pSeg)
{
	WORD boardId = 0;
	WORD numPri = 0;
	*pSeg >> boardId;
	*pSeg >> numPri;

	TRACESTR(eLevelInfoNormal) << "\nRtmIsdnMngrManager::DetachSpansFor_NumPriRtmIsdnCard boardId " << boardId
			<<"numPri "<<numPri;


	CSlotsNumberingConversionTableWrapper* pCurTable = m_pProcess->GetSlotsNumberingConversionTable();
	DWORD displayBoardId = pCurTable->GetDisplayBoardId(boardId, 2);


if (numPri == RTM_ISDN_9PRI)
{
	SPAN_DISABLE_S spanDisableStruct_ForCards;
	spanDisableStruct_ForCards.boardId	= displayBoardId;
	spanDisableStruct_ForCards.spanId	= RTM_ISDN_PRI_10;
	spanDisableStruct_ForCards.status	= STATUS_OK;

	STATUS retStat = DetachSpanMap(spanDisableStruct_ForCards,false);

	spanDisableStruct_ForCards.spanId	= RTM_ISDN_PRI_11;
	retStat = DetachSpanMap(spanDisableStruct_ForCards,false);

	spanDisableStruct_ForCards.spanId	= RTM_ISDN_PRI_12;
	retStat = DetachSpanMap(spanDisableStruct_ForCards,false);
}
else
{
	SPAN_DISABLE_S spanDisableStruct_ForCards;
		spanDisableStruct_ForCards.boardId	= displayBoardId;
		spanDisableStruct_ForCards.spanId	= RTM_ISDN_PRI_10;
		spanDisableStruct_ForCards.status	= STATUS_OK;

		STATUS retStat = SetIsSpanValid(spanDisableStruct_ForCards,boardId);

		spanDisableStruct_ForCards.spanId	= RTM_ISDN_PRI_11;
		retStat = SetIsSpanValid(spanDisableStruct_ForCards,boardId);

		spanDisableStruct_ForCards.spanId	= RTM_ISDN_PRI_12;
		retStat = SetIsSpanValid(spanDisableStruct_ForCards,boardId);
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendAttachSpanMapToCardsProcess(const RTM_ISDN_SPAN_MAP_S &spanToAttach)
{
	// ===== 1. print
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendAttachSpanMapToCardsProcess";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpanMapStruct(spanToAttach, "CRtmIsdnMngrManager::SendAttachSpanMapToCardsProcess");

	// ===== 2. fill the Segment with items from the list
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)&spanToAttach, sizeof(RTM_ISDN_SPAN_MAP_S) );

	// ===== 3. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

	STATUS res = pMbx->Send(pRetParam,RTM_ISDN_ATTACH_SPAN_MAP_IND,&GetRcvMbx());

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::SendMsgSyncDisableSpanMapToResourceProcess(const SPAN_DISABLE_S &spanToDisable)
{
	STATUS retStat = STATUS_OK;

	// ===== 1. print
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncDisableSpanMapToResourceProcess";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansDisableStruct(spanToDisable, "CRtmIsdnMngrManager::SendSpansMapDisableIfUpdatableToResourceProcess - send");

	// ===== 2. fill the Segment with items from the list
	CSegment*  pSegSent = new CSegment;
	pSegSent->Put( (BYTE*)&spanToDisable, sizeof(SPAN_DISABLE_S) );

	// ===== 3. send
	CSegment rspMsg;
	OPCODE rspOpcode;
	CTaskApi resourceManagerApi(eProcessResource, eManager);
	STATUS responseStatus = resourceManagerApi.SendMessageSync(pSegSent, RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_REQ, RTM_ISDN_RESPONSE_TIMEOUT, rspOpcode, rspMsg);

	// ===== 2. get the response
	if (STATUS_OK == responseStatus)
	{
        if ( 0 < rspMsg.GetLen() )
        {
        	SPAN_DISABLE_S retSpanToDisable;
			rspMsg.Get( (BYTE*)(&retSpanToDisable), sizeof(SPAN_DISABLE_S) );
			m_rtmIsdnCommonMethods.PrintRtmIsdnSpansDisableStruct(retSpanToDisable, "CRtmIsdnMngrManager::SendMsgSyncDisableSpanMapToResourceProcess - response");

			retStat = retSpanToDisable.status;
			if ( (STATUS_OK == retStat) || (STATUS_SPAN_NOT_EXISTS == retStat) )
			{
				TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncDisableSpanMapToResourceProcess"
				                       << "\nOpcode: RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_REQ"
				                       << "\nSpan can be disabled"
			                           << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";

				retStat = STATUS_OK;
			}
			else // retStat != OK
			{
				TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncDisableSpanMapToResourceProcess"
				                       << "\nOpcode: RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_REQ"
				                       << "\nResource process refuses to disable the span"
			                           << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";
	  		}
        }
        else // no content in response
        {
            retStat = STATUS_ILLEGAL;
			TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncDisableSpanMapToResourceProcess"
			                       << "\nOpcode: RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_REQ"
			                       << "\nNo content in response"
			                       << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";
        }

    }

	else // responseStatus != OK (i.e. timeout or dead process)
	{
		retStat = responseStatus;
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncDisableSpanMapToResourceProcess"
		                       << "\nOpcode: RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_REQ"
		                       << "\nResponse timeout"
		                       << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";

	}

	return retStat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::DetachSpanMap(const SPAN_DISABLE_S &theStruct ,bool isSpanValid)
{
	STATUS retStat = STATUS_OK;

	// ===== 1. print
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DetachSpanMap";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansDisableStruct(theStruct, "CRtmIsdnMngrManager::DetachSpanMap");

	// ===== 2. update span in list
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	retStat = pSpanMapList->DetachSpanMap(theStruct,isSpanValid);

	// ===== 3. remove unnecessary Alerts
	if ( false == pSpanMapList->IsAnySpanAttachedOnBoard(theStruct.boardId) )
	{
		RemoveActiveAlarmFaultOnlyByErrorCodeUserId(AA_SINGLE_CLOCK_SOURCE, (DWORD)theStruct.boardId);
		RemoveActiveAlarmByErrorCodeUserId(AA_NO_CLOCK_SOURCE, (DWORD)theStruct.boardId);
	}
	RemoveNoRtmCardActiveAlarmIfNeeded(theStruct.boardId);

	if (STATUS_OK == retStat)
	{
		if (YES == m_isPstnLicensed)
		{
			SendDetachSpanMapToCardsProcess(theStruct);
		}
		else
		{
			TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DetachSpanMap"
			                              << "\nPSTN is unlicensed";
		}
	}

	else // disable failed
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DetachSpanMap"
		                              << "\nFailed to disable span. Status: "
		                              << m_pProcess->GetStatusAsString(retStat);
	}

	return retStat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::SetIsSpanValid(const SPAN_DISABLE_S &theStruct ,WORD boardId)
{
	STATUS retStat = STATUS_OK;

	// ===== 1. print
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SetIsSpanValid";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansDisableStruct(theStruct, "CRtmIsdnMngrManager::SetIsSpanValid");

	// ===== 2. update span in list
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	retStat = pSpanMapList->SetIsSpanMapValid(theStruct,boardId);

    if (retStat == STATUS_SPAN_MAP_UPDATED)
    	 SendSpansMapListStructToCardsProcess();

	return retStat;
}




///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::AddNoRtmCardActiveAlarmIfNeeded(const WORD boardId)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::AddNoRtmCardActiveAlarmIfNeeded (board " << boardId << ")";

	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();

	/*******************************************************************************************/
	/* 19.7.10 VNGR-16063 fixed by Rachel Cohen                                                */
	/* the boardid of rtmIsdn is for now 13 and not 1 as I agreed with Dotan from embedded     */
	/*******************************************************************************************/

	CSlotsNumberingConversionTableWrapper* pCurTable = m_pProcess->GetSlotsNumberingConversionTable();
	DWORD curDisplayBoardId = pCurTable->GetDisplayBoardId(boardId, 2);
    
	if (CProcessBase::GetProcess()->GetProductType() == eProductTypeNinja)
		curDisplayBoardId = m_pProcess->Get1stRtmIsdnBoardId(); // 13
    
	if (true == IsRtmIsdnLoaded(curDisplayBoardId))
	{

		CRtmIsdnServiceList *pServiceList = m_pProcess->GetServiceListOriginal();


		if (eProductFamilyRMX == CProcessBase::GetProcess()->GetProductFamily())
		{
			if (false == pServiceList->IsDefaultExists())
			{

				TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::AddNoRtmCardActiveAlarmIfNeeded"
						<< "\nNo Default Service exists";

				bool isAlertExists = IsActiveAlarmExistByErrorCode( AA_NO_DEFAULT_ISDN_SERVICE);
				if (false == isAlertExists)
					AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_NO_DEFAULT_ISDN_SERVICE, MAJOR_ERROR_LEVEL, "No default ISDN/PSTN Network Service defined in ISDN/PSTN Network Services list", true, true);
			}
			else
				RemoveActiveAlarmByErrorCode(AA_NO_DEFAULT_ISDN_SERVICE);

			if (true == pServiceList->IsEmpty())
			{
				bool isExist = IsActiveAlarmExistByErrorCode(AA_NO_ISDN_SERVICE_PARAMS);
				if(!isExist)
					AddActiveAlarm(FAULT_GENERAL_SUBJECT, AA_NO_ISDN_SERVICE_PARAMS, MAJOR_ERROR_LEVEL, "No ISDN/PSTN Network Services defined", true, true);

			}
			else
				RemoveActiveAlarmByErrorCode(AA_NO_ISDN_SERVICE_PARAMS);

		}



	}

	//rtm not loaded
	else if (pSpanMapList->IsAnySpanAttachedOnBoard(curDisplayBoardId))
	{
        bool isAlertExists = IsActiveAlarmExistByErrorCodeUserId( AA_NO_RTM_ISDN_CARD, (DWORD)curDisplayBoardId );
        if (false == isAlertExists)
        {
        	TRACESTR(eLevelInfoNormal) << "\nAlert does not exist (board " << curDisplayBoardId << ")";
        	CLargeString errStr = "Span is configured on a nonexisting RTM board";
        	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
        	if (curProductType !=eProductTypeRMX1500 && curProductType != eProductTypeNinja)
        	  errStr << " (board id " << curDisplayBoardId << ")";

    		AddActiveAlarm( FAULT_GENERAL_SUBJECT,
					        AA_NO_RTM_ISDN_CARD,
			                MAJOR_ERROR_LEVEL,
			                errStr.GetString(),
			                true,
			                true,
			                curDisplayBoardId // as userId
			               );
        }
       // else
       // {
        	//TRACESTR(eLevelInfoNormal) << "\nAlert already exists (board " << curDisplayBoardId << ")";
       // }
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::RemoveNoRtmCardActiveAlarmIfNeeded(const WORD boardId)
{
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	DWORD curDisplayBoardId = boardId;
      
	if (CProcessBase::GetProcess()->GetProductType() == eProductTypeNinja)
		curDisplayBoardId = m_pProcess->Get1stRtmIsdnBoardId(); // 13
    
	if ( (true  == IsRtmIsdnLoaded(curDisplayBoardId)) ||
		 (false == pSpanMapList->IsAnySpanAttachedOnBoard(curDisplayBoardId)) )
	{
		RemoveActiveAlarmByErrorCodeUserId(AA_NO_RTM_ISDN_CARD, curDisplayBoardId);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::OnRtmIsdnSpanStatusInd(CSegment* pSeg)
{
 	// ===== 1. extract data
    RTM_ISDN_SPAN_STATUS_MCMS_S* pSpanStatusStruct = new RTM_ISDN_SPAN_STATUS_MCMS_S;
	pSeg->Get( (BYTE*)pSpanStatusStruct, sizeof(RTM_ISDN_SPAN_STATUS_MCMS_S) );

	// ===== 2. print
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnRtmIsdnSpanStatusInd";
	PrintSpanStatusDataToTrace(*pSpanStatusStruct);

	// ===== 3. update status in spans' list
	CSlotsNumberingConversionTableWrapper* pCurTable = m_pProcess->GetSlotsNumberingConversionTable();
	DWORD displayBoardId = pCurTable->GetDisplayBoardId(pSpanStatusStruct->boardId, pSpanStatusStruct->subBoardId);
	eProductType curProductType = CProcessBase::GetProcess()->GetProductType();
	if (curProductType == eProductTypeNinja)
	displayBoardId = m_pProcess->Get1stRtmIsdnBoardId();
	pSpanStatusStruct->boardId = displayBoardId;
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnRtmIsdnSpanStatusInd" 
								<<" \nboard: "<<pSpanStatusStruct->boardId
								<<"\nspanId: "<<pSpanStatusStruct->spanId;
	
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	STATUS updateStat = pSpanMapList->UpdateSpanStatus(*pSpanStatusStruct);
	if (STATUS_OK == updateStat)
	{
		TreatClockSourceStateAlerts(pSpanStatusStruct->boardId, pSpanStatusStruct->subBoardId);
	}
	else
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnRtmIsdnSpanStatusInd - failed to update span status";
		PASSERT(1);
	}

	delete pSpanStatusStruct;
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::PrintSpanStatusDataToTrace(const RTM_ISDN_SPAN_STATUS_MCMS_S& theStruct)
{
	CLargeString retStr = "\nCRtmIsdnMngrManager::PrintSpanStatusDataToTrace: ";

	retStr << "\nBoardId:     "	<< theStruct.boardId
	       << "\nSubBoardId:  "	<< theStruct.subBoardId
	       << "\nSpanId:      "	<< theStruct.spanId
	       << "\nAlarmStatus: "	<< ::SpanAlarmToString( (eSpanAlarmType)(theStruct.alarm_status) )
	       << "\nDChannel:    "	<< ::DChannelStateToString( (eDChannelStateType)(theStruct.d_chnl_status) )
	       << "\nClocking:    "	<< ::ClockingToString( (eClockingType)(theStruct.clocking_status) );

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

///////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::TreatClockSourceStateAlerts(const WORD boardId, const WORD subBoardId)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::TreatClockSourceStateAlerts "
	                       << "(boardId " << boardId << ", subBoardId " << subBoardId << ")";


	// An Alert is produced only if at least one span is configured
	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	if ( true == pSpanMapList->IsThereAnySpanConfigured(boardId) )
	{
		eCardsClockSourceStateType clockState = pSpanMapList->GetCardsClockSourceState(boardId);

		// ===== 1. ClockSource ok
		if (eCardsClockSourceState_ok == clockState)
		{
			RemoveActiveAlarmFaultOnlyByErrorCodeUserId(AA_SINGLE_CLOCK_SOURCE, (DWORD)boardId);
			RemoveActiveAlarmByErrorCodeUserId(AA_NO_CLOCK_SOURCE, (DWORD)boardId);
		}

		// ===== 2. Single ClockSource
		else if (eCardsClockSourceState_singleClock == clockState)
		{
			RemoveActiveAlarmByErrorCodeUserId(AA_NO_CLOCK_SOURCE, (DWORD)boardId);

			if (NO == m_isSystemNormalWithSingleClockSource)
			{
	            CMedString errStr = "Single Clock Source ";
	        	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
	        	if (curProductType !=eProductTypeRMX1500)
	               errStr << "(board id " << boardId << ", subBoard id " << subBoardId << ")";

	            bool isAlertExists = IsActiveAlarmFaultOnlyExistByErrorCodeUserId( AA_SINGLE_CLOCK_SOURCE, (DWORD)boardId );
	            if (false == isAlertExists)
	            {
		            AddActiveAlarmFaultOnly( FAULT_GENERAL_SUBJECT,
							        AA_SINGLE_CLOCK_SOURCE,
									MAJOR_ERROR_LEVEL,
									errStr.GetString(),
									(DWORD)boardId // as userId
								  );

	            }
			} // end systemNormalWithSingleClockSource
		}

		// ===== 3. No ClockSource
		else // eCardsClockSourceState_noClock == clockState
		{
			RemoveActiveAlarmFaultOnlyByErrorCodeUserId(AA_SINGLE_CLOCK_SOURCE, (DWORD)boardId);

            CMedString errStr = "No Clock Source ";
        	eProductType  curProductType  = CProcessBase::GetProcess()->GetProductType();
        	if (curProductType !=eProductTypeRMX1500)
               errStr << "(board id " << boardId << ", subBoard id " << subBoardId << ")";

            bool isAlertExists = IsActiveAlarmExistByErrorCodeUserId( AA_NO_CLOCK_SOURCE, (DWORD)boardId );
            if (false == isAlertExists)
            {
	            AddActiveAlarm( FAULT_GENERAL_SUBJECT,
	            		AA_NO_CLOCK_SOURCE,
	            		SYSTEM_MESSAGE,
								errStr.GetString(),
								true,
								true,
								(DWORD)boardId // as userId
							  );
            }
		}

	} // end (IsThereAnySpanConfigured)

	else //(IsThereAnySpanConfigured != true)
	{
		RemoveActiveAlarmFaultOnlyByErrorCodeUserId(AA_SINGLE_CLOCK_SOURCE, (DWORD)boardId);
		RemoveActiveAlarmByErrorCodeUserId(AA_NO_CLOCK_SOURCE, (DWORD)boardId);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendDetachSpanMapToCardsProcess(const SPAN_DISABLE_S &spanToDetach)
{
	// ===== 1. print
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendDetachSpanMapToCardsProcess";
	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansDisableStruct(spanToDetach, "CRtmIsdnMngrManager::SendDetachSpanMapToCardsProcess");

	// ===== 2. fill the Segment with items from the list
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)&spanToDetach, sizeof(SPAN_DISABLE_S) );

	// ===== 3. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

	STATUS res = pMbx->Send(pRetParam,RTM_ISDN_DETACH_SPAN_MAP_IND,&GetRcvMbx());

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::SendUpdatePhoneRangeReqToProcesses( const OPCODE theOpcode,
                                                                const char* serviceName,
                                                                const CRtmIsdnPhoneNumberRange &phoneRange )
{
	STATUS retStat = STATUS_OK;

	// ===== 1. prepare the data to be sent
	RTM_ISDN_PHONE_RANGE_UPDATE_S phoneRangeUpdateStruct;
	memset(&phoneRangeUpdateStruct, 0, sizeof(RTM_ISDN_PHONE_RANGE_UPDATE_S));

	memcpy( &(phoneRangeUpdateStruct.serviceName),
	        serviceName,
	        RTM_ISDN_SERVICE_PROVIDER_NAME_LEN );

   	phoneRangeUpdateStruct.phoneRange.dialInGroupId	= phoneRange.GetDialInGroupId();
	phoneRangeUpdateStruct.phoneRange.category			= phoneRange.GetCategory();
	phoneRangeUpdateStruct.phoneRange.firstPortId		= phoneRange.GetFirstPortId();

	memcpy( &(phoneRangeUpdateStruct.phoneRange.firstPhoneNumber),
	        phoneRange.GetFirstPhoneNumber(),
	        ISDN_PHONE_NUMBER_DIGITS_LEN-1);

	memcpy( &(phoneRangeUpdateStruct.phoneRange.lastPhoneNumber),
	        phoneRange.GetLastPhoneNumber(),
	        ISDN_PHONE_NUMBER_DIGITS_LEN-1);


	// ===== 2. print
	m_rtmIsdnCommonMethods.PrintRtmIsdnPhoneRangeUpdateStruct(phoneRangeUpdateStruct, "CRtmIsdnMngrManager::SendUpdatePhoneRangeReqToProcesses");

	// ===== 3. send
	retStat = SendUpdatePhoneRangeReqToSpecProcess(theOpcode, phoneRangeUpdateStruct, eProcessResource);

	return retStat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::SendUpdatePhoneRangeReqToSpecProcess( const OPCODE theOpcode,
                                                                  RTM_ISDN_PHONE_RANGE_UPDATE_S &theStruct,
                                                                  eProcessType theProcess )
{
	STATUS retStat = STATUS_OK;

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendUpdatePhoneRangeReqToSpecProcess"
	                       << "\nOpcode: "  << m_pProcess->GetOpcodeAsString(theOpcode)
	                       << ", Process: " << ::ProcessTypeToString(theProcess);

	m_rtmIsdnCommonMethods.PrintRtmIsdnPhoneRangeUpdateStruct(theStruct, "CRtmIsdnMngrManager::SendUpdatePhoneRangeReqToSpecProcess");

	// ===== 1. fill the Segment with items from the list
	CSegment*  pSegSent = new CSegment;
	pSegSent->Put( (BYTE*)&theStruct, sizeof(RTM_ISDN_PHONE_RANGE_UPDATE_S) );


	// ===== 2. send

	// on Delete - a syncMessage is needed
	if (RTM_ISDN_PHONE_RANGE_DELETE_IF_UPDATABLE_REQ == theOpcode)
	{
		retStat = SendMsgSyncUpdatePhoneRangeToResourceProcess(pSegSent, theOpcode);
	}
	else // regular sending (non sync)
	{
		const COsQueue* pMbx = CProcessBase::GetProcess()->GetOtherProcessQueue(theProcess, eManager);
		retStat = pMbx->Send(pSegSent, theOpcode, &GetRcvMbx());
	}

	return retStat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::SendMsgSyncUpdatePhoneRangeToResourceProcess(CSegment* pSegSent, const OPCODE theOpcode)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncUpdatePhoneRangeToResourceProcess"
	                       << "\nOpcode: " << m_pProcess->GetOpcodeAsString(theOpcode);

	STATUS retStat = STATUS_OK;

	// ===== 1. send
	CSegment rspMsg;
	OPCODE rspOpcode;
	CTaskApi resourceManagerApi(eProcessResource, eManager);
	STATUS responseStatus = resourceManagerApi.SendMessageSync(pSegSent, theOpcode, RTM_ISDN_RESPONSE_TIMEOUT, rspOpcode, rspMsg);

	// ===== 2. get the response
	if (STATUS_OK == responseStatus)
	{
        if ( 0 < rspMsg.GetLen() )
        {
        	RTM_ISDN_PHONE_RANGE_UPDATE_S phoneUpdateStruct;
			rspMsg.Get( (BYTE*)(&phoneUpdateStruct), sizeof(RTM_ISDN_PHONE_RANGE_UPDATE_S) );
			m_rtmIsdnCommonMethods.PrintRtmIsdnPhoneRangeUpdateStruct(phoneUpdateStruct, "CRtmIsdnMngrManager::SendMsgSyncUpdatePhoneRangeToResourceProcess - response");

			retStat = phoneUpdateStruct.status;
			if (STATUS_OK == retStat)
			{
				TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncUpdatePhoneRangeToResourceProcess"
				                       << "\nOpcode: " << m_pProcess->GetOpcodeAsString(theOpcode)
				                       << "\nResource process approvee to delete the PhoneRange"
			                           << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";
			}
			else // retStat != OK
			{
				TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncUpdatePhoneRangeToResourceProcess"
				                       << "\nOpcode: " << m_pProcess->GetOpcodeAsString(theOpcode)
				                       << "\nResource process refuses to delete PhoneRange"
			                           << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";
	  		}
        }
        else // no content in response
        {
            retStat = STATUS_ILLEGAL;
			TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncUpdatePhoneRangeToResourceProcess"
			                       << "\nOpcode: " << m_pProcess->GetOpcodeAsString(theOpcode)
			                       << "\nNo content in response"
			                       << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";
        }

    }

	else // responseStatus != OK (i.e. timeout or dead process)
	{
		retStat = responseStatus;
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendMsgSyncUpdatePhoneRangeToResourceProcess"
		                       << "\nOpcode: " << m_pProcess->GetOpcodeAsString(theOpcode)
		                       << "\nResponse timeout"
		                       << "\n(status returned from Resource process: " << m_pProcess->GetStatusAsString(retStat) << ")";

	}

	return retStat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CRtmIsdnMngrManager::DeletePhoneRange(const char* serviceName, const CRtmIsdnPhoneNumberRange &phoneRange, bool isToCheckStat)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DeletePhoneRange";

	STATUS retStat = STATUS_OK;

	// delete from lists
	CRtmIsdnServiceList *pServListOriginal = m_pProcess->GetServiceListOriginal();
	CRtmIsdnServiceList *pServListUpdated  = m_pProcess->GetServiceListUpdated();

	STATUS delOriginalStat = pServListOriginal->DelPhoneNumRange( serviceName,
	                                                              phoneRange,
	                                                              RTM_ISDN_SERVICE_LIST_PATH,
	                                                              isToCheckStat );
	if (STATUS_OK != delOriginalStat)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DeletePhoneRange"
		                              << "\nFailed to delete Phone Range from original list. Status: "
		                              << m_pProcess->GetStatusAsString(delOriginalStat);
		return delOriginalStat;
	}

	STATUS delUpdatedStat  = pServListUpdated->DelPhoneNumRange( serviceName,
	                                                             phoneRange,
	                                                             RTM_ISDN_SERVICE_LIST_TMP_PATH,
	                                                             isToCheckStat );
	if (STATUS_OK != delUpdatedStat)
	{
		TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::DeletePhoneRange"
		                              << "\nFailed to delete Phone Range from updated list. Status: "
		                              << m_pProcess->GetStatusAsString(delUpdatedStat);
		return delUpdatedStat;
	}

	return retStat;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL	CRtmIsdnMngrManager::GetIsPstnLicensed() const
{
	return m_isPstnLicensed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SetIsPstnLicensed(const BOOL isLicensed)
{
	m_isPstnLicensed = isLicensed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnMngrManager::IsRtmIsdnLoaded(const int boardId) const
{
	bool retVal = false;

	for (int i=0; i<RTM_CARDS_ARRAY_LEN; i++)
	{
		if (boardId == m_RtmIsdnLoaded_BoardId[i])
		{
			retVal = true;
			break;
		}
	}

	return retVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SetRtmIsdnLoaded_BoardId(const int boardId)
{
	for (int i=0; i<RTM_CARDS_ARRAY_LEN; i++)
	{
		if (RTM_ISDN_UNITIALIZED_BOARD_ID == m_RtmIsdnLoaded_BoardId[i])
		{
			m_RtmIsdnLoaded_BoardId[i] = boardId;
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::OnRtmIsdnSpanDisableInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnRtmIsdnSpanDisableInd"
	                              << "\nThe response was received here instead of in the right mbx";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::OnRtmIsdnPhoneRangeDelInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnRtmIsdnPhoneRangeDelInd"
	                              << "\nThe response was received here instead of in the right mbx";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::OnRtmServiceCancelInd(CSegment* pSeg)
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnRtmServiceCancelInd"
	                              << "\nThe response was received here instead of in the right mbx";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::OnMcuMngrLicensingInd(CSegment* pSeg)
{
	BYTE flag = NO;
	*pSeg >> flag;

	m_isPstnLicensed = (BOOL)flag;

	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnMcuMngrLicensingInd"
	                              << "\nPSTN flag:" << (YES == flag ? "YES" : "NO");


	if (YES == m_isPstnLicensed)
	{
		SendSpansMapListStructToCardsProcess();
	}
	else
	{
		// if PSTN is unlicensed and there is a configured span - an Alert is produced
		CRtmIsdnSpanMapList *pSpanList = m_pProcess->GetSpanMapList();
		if ( pSpanList && (YES == pSpanList->IsAnySpanAttachedToService()) )
		{
			AddActiveAlarmSingleton( FAULT_GENERAL_SUBJECT,
					                 AA_NO_ISDN_PSTN_LICENSING,
			                         MAJOR_ERROR_LEVEL,
			                         PSTN_UNLICENSED_DESCRIPTION,
			                         true,
			                         true
			                        );
		}
	} // end PSTN unlicensed

	// start timer towards Card startup
	StartTimer(MFA_STARTUP_TIMER, CARD_STARTUP_TIME_LIMIT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::OnRtmIsdnEntityLoadedInd(CSegment* pSeg)
{
	// ===== 1. extract data
	RTM_ISDN_ENTITY_LOADED_S* pRtmBoardStruct = new RTM_ISDN_ENTITY_LOADED_S;
	pSeg->Get( (BYTE*)pRtmBoardStruct, sizeof(RTM_ISDN_ENTITY_LOADED_S) );

	// ===== 2. print
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::OnRtmIsdnEntityLoadedInd";
	PrintRtmIsdnEntityDataToTrace(*pRtmBoardStruct);

	// ===== 3. update array
	CSlotsNumberingConversionTableWrapper* pCurTable = m_pProcess->GetSlotsNumberingConversionTable();
	DWORD curDisplayBoardId = pCurTable->GetDisplayBoardId(pRtmBoardStruct->boardId, pRtmBoardStruct->subBoardId);

	if (CProcessBase::GetProcess()->GetProductType() == eProductTypeNinja)
	    	curDisplayBoardId = m_pProcess->Get1stRtmIsdnBoardId();
    
	SetRtmIsdnLoaded_BoardId(curDisplayBoardId);

	// ===== 4. remove irrelevant Alert
	RemoveNoRtmCardActiveAlarmIfNeeded(curDisplayBoardId);

	delete pRtmBoardStruct;
}

//////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::PrintRtmIsdnEntityDataToTrace(const RTM_ISDN_ENTITY_LOADED_S& theStruct)
{
	CLargeString retStr = "\nCRtmIsdnMngrManager::PrintRtmIsdnEntityDataToTrace: ";

	WORD	boardId;
	WORD	subBoardId;
	WORD	numOfSpans;
	retStr << "\nBoardId:      "	<< theStruct.boardId
	       << "\nSubBoardId:   "	<< theStruct.subBoardId
	       << "\nNum of Spans: "	<< theStruct.numOfSpans;

	TRACESTR(eLevelInfoNormal) << retStr.GetString();
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SendSpansMapListStructToCardsProcess()
{
	TRACESTR(eLevelInfoNormal) << "\nCRtmIsdnMngrManager::SendSpansMapListStructToCardsProcess";

	// ===== 1. prepare data to send
	RTM_ISDN_SPAN_MAPS_LIST_S spanMapListStruct;
	memset(&spanMapListStruct, 0, sizeof(RTM_ISDN_SPAN_MAPS_LIST_S));

	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	if (pSpanMapList)
	{
		int maxNumOfSpanMapsInList = pSpanMapList->GetMaxNumOfSpanMaps();
		CRtmIsdnSpanMap* pCurSpanMap = pSpanMapList->GetFirstSpanMap();
		for ( int i=0;
		      ( pCurSpanMap && (i<maxNumOfSpanMapsInList) );
		      i++)
		{
			RTM_ISDN_SPAN_MAP_S spanMapStruct;
			pCurSpanMap->ConvertToRtmIsdnSpanMapStruct(spanMapStruct);
			memcpy( &(spanMapListStruct.spanMap[i]), &spanMapStruct, sizeof(RTM_ISDN_SPAN_MAP_S) );

			pCurSpanMap = pSpanMapList->GetNextSpanMap();
		}
	}

	m_rtmIsdnCommonMethods.PrintRtmIsdnSpansMapsListStruct(spanMapListStruct, "CRtmIsdnMngrManager::SendSpansMapListStructToCardsProcess");


	// ===== 2. fill the Segment with items from the list
	CSegment*  pRetParam = new CSegment;
	pRetParam->Put( (BYTE*)&spanMapListStruct, sizeof(RTM_ISDN_SPAN_MAPS_LIST_S) );


	// ===== 3. send
	const COsQueue* pMbx =
			CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessCards, eManager);

	STATUS res = pMbx->Send(pRetParam,RTM_ISDN_SPAN_MAPS_IND,&GetRcvMbx());

	return;
}

/////////////////////////////////////////////////////////////////////////////
eSystemSpanType  CRtmIsdnMngrManager::GetSystemSpanType () const
{
    return m_systemSpanType;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SetSystemSpanType(const eSystemSpanType theType)
{
	m_systemSpanType = theType;
}

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnMngrManager::IsAcceptedSpanType(eSpanType theType)
{
	bool isAccepted = true;

	// ===== 1. system span type was not determined yet
	if (eSystemSpanType_Undefined == m_systemSpanType)
	{
		isAccepted = true;
		SetParamsAccordingToSpanType(theType);
	}

	// ===== 2. system span type was already determined
	else
	{
		if ( ((eSystemSpanType_T1 == m_systemSpanType) && (eSpanTypeE1 == theType)) ||
			 ((eSystemSpanType_E1 == m_systemSpanType) && (eSpanTypeT1 == theType)) )
		{
			isAccepted = false;
		}
	}


	return isAccepted;
}

/////////////////////////////////////////////////////////////////////////////
void CRtmIsdnMngrManager::SetParamsAccordingToSpanType(eSpanType theType)
{
	if (eSpanTypeT1 == theType)
	{
		m_systemSpanType			= eSystemSpanType_T1;
		m_maxAllowedConfiguredSpans	= MAX_CONFIGURED_SPANS_T1;
	}
	else
	{
		m_systemSpanType			= eSystemSpanType_E1;
		m_maxAllowedConfiguredSpans	= MAX_CONFIGURED_SPANS_E1;
	}
}

/////////////////////////////////////////////////////////////////////////////
bool CRtmIsdnMngrManager::IsAcceptedAdditionalConfiguredSpan(WORD boardId)
{
	int curNumOfAttachedSpans = 0;

	CRtmIsdnSpanMapList *pSpanMapList = m_pProcess->GetSpanMapList();
	if (pSpanMapList)
		curNumOfAttachedSpans = pSpanMapList->GetNumOfAttachedSpansOnBoard(boardId);

	bool isAccepted = ( (curNumOfAttachedSpans < m_maxAllowedConfiguredSpans)
	                    ?
	                    true : false );

	return isAccepted;
}


void CRtmIsdnMngrManager::OnTimerGetServiceInfoInd()
{
  // Updates data regardless SNMP enableness.
  CRtmIsdnServiceList* slist = m_pProcess->GetServiceListUpdated();
  if (slist->IsEmpty())
    return;

  CSegment* seg = new CSegment;
  *seg << static_cast<unsigned int>(eTT_ISDNStatus)
       << 3u;  // Failed.
  CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_TELEMETRY_DATA_IND);
}

void CRtmIsdnMngrManager::OnSNMPConfigInd(CSegment* pSeg)
{
	BOOL bSNMPEnabled = FALSE;
	*pSeg >> bSNMPEnabled;
	
	PTRACE2INT(eLevelInfoNormal,"CRtmIsdnMngrManager::OnSNMPConfigInd = ",(int)bSNMPEnabled);
	SetIsSNMPEnabled(bSNMPEnabled);
}
