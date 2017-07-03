#include "ResourceMonitor.h"
#include "TaskApi.h"
#include "TraceStream.h"
#include "Trace.h"
#include "Macros.h"
#include "OpcodesMcmsCommon.h"
#include "Macros.h"
#include "DummyEntry.h"
#include "ApacheDefines.h"
#include "FaultsDefines.h"
#include "ApiStatuses.h"
#include "McuMngrInternalStructs.h"
#include "MplMcmsProtocolTracer.h"
#include "InternalProcessStatuses.h"
#include "OpcodesMcmsInternal.h"
#include "SystemResources.h"
#include "HelperFuncs.h"
#include "EnhancedConfigResponse.h"
#include "EnhancedConfig.h"
#include "CommResRsrvDBGet.h"
#include "GetResSpecific.h"
#include "CRsrvDB.h"
#include "CommResApi.h"
#include "AllocationModeDetails.h"
#include "SharedRsrcList.h"
#include "AutoModeResources.h"
#include "ResourceManager.h"
#include "dsp_monitor_getter.h"

class CRsrcDetailElement;

PBEGIN_MESSAGE_MAP(CResourceMonitor)
  ONEVENT(XML_REQUEST, IDLE, CResourceMonitor::HandlePostRequest)
  ONEVENT(GET_RESOURCE_INFO_TIMER    ,IDLE  ,    CResourceMonitor::OnTimerGetResourceInformationForSNMP)
PEND_MESSAGE_MAP(CResourceMonitor, CAlarmableTask);

BEGIN_GET_TRANSACTION_FACTORY(CResourceMonitor)
  ON_TRANS("TRANS_CARD"                 , "GET"                                   , CRsrcDetailGet    , CResourceMonitor::OnServerCardDetail)
  ON_TRANS("TRANS_CARD"                 , "GET_UNIT"                              , CUnitRsrcDetailGet, CResourceMonitor::OnServerCardUnitDetail)
  ON_TRANS("TRANS_RSRC_REPORT"          , "GET_CARMEL_REPORT"                     , CDummyEntry       , CResourceMonitor::OnServerResourceReport)
  ON_TRANS("TRANS_CONFERENCES_RESOURCES", "GET_LS"                                , CSharedRsrcList   , CResourceMonitor::OnServerConfResourceReport)
  ON_TRANS("TRANS_MCU"                  , "GET_RECORDING_JUNCTION_LIST"           , CDummyEntry       , CResourceMonitor::OnGetRecordingJunctionsList)
  ON_TRANS("TRANS_MCU"                  , "GET_PORT_CONFIGURATION"                , CDummyEntry       , CResourceMonitor::OnGetConfigurationList)
  ON_TRANS("TRANS_MCU"                  , "GET_ENHANCED_PORT_CONFIGURATION"       , CDummyEntry       , CResourceMonitor::OnGetEnhancedConfiguration)
  ON_TRANS("TRANS_MCU"                  , "GET_CHECK_ENHANCED_PORT_CONFIGURATION" , CEnhancedConfig   , CResourceMonitor::OnCheckEnhancedPortConfiguration)
  ON_TRANS("TRANS_RES_LIST"             , "GET_RES_LIST"                          , CCommResRsrvDBGet , CResourceMonitor::OnGetReservationList)
  ON_TRANS("TRANS_RES_2"                , "GET_RES"                               , CGetResSpecific   , CResourceMonitor::OnServerGetResReq)
  ON_TRANS("TRANS_MCU"                  , "GET_ALLOCATION_MODE"                   , CDummyEntry       , CResourceMonitor::OnGetAllocationMode)
  ON_TRANS("TRANS_RSRC_REPORT"          , "GET_CARMEL_SERVICES_REPORT"            , CDummyEntry       , CResourceMonitor::OnServerServicesResourceReport)
END_TRANSACTION_FACTORY

////////////////////////////////////////////////////////////////////////////
void ResourceMonitorEntryPoint(void* appParam)
{
  CResourceMonitor* monitorTask = new CResourceMonitor;
  monitorTask->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////
//                        CResourceMonitor
////////////////////////////////////////////////////////////////////////////
CResourceMonitor::CResourceMonitor()
{
  m_pJunctionsList = new CJunctionsList();
  m_pJunctionsList->InitJunctionsList();
}

////////////////////////////////////////////////////////////////////////////
CResourceMonitor::~CResourceMonitor()
{
  PDELETE(m_pJunctionsList);
}

void  CResourceMonitor::InitTask()
{
    CMonitorTask::InitTask();

	TRACEINTO << "CConfPartyMonitor::Start a timer, yuhui \n";
	StartTimer(GET_RESOURCE_INFO_TIMER, GET_RESOURCE_INFO_TIMER_TIME_OUT_VALUE);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnServerCardDetail(CRequest* pGetRequest)
{
  if (pGetRequest->GetAuthorization() == SUPER)
  {
   // if product type is Ninjia
   CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
  if( pSyst )
  {
	 eProductType prodType = pSyst->GetProductType();
         if(eProductTypeNinja == prodType)
         {
            return OnServerCardDetailNinja(pGetRequest);
         }
   }
    CRsrcDetailGet*   pRsrcDetailGet = (CRsrcDetailGet*)pGetRequest->GetRequestObject(); // To set the updateCounter
    CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
    CRsrcDetailGet*   pRsrcDetailSnd = new CRsrcDetailGet;

    const WORD displayBoardId = pRsrcDetailGet->GetBoardId();
    PTRACE2INT(eLevelInfoNormal, " CResourceMonitor::OnServerCardDetail - displayBoardId:", displayBoardId);

    eProductType prodType = eProductTypeRMX4000;
    CReservator* pReservator = CHelperFuncs::GetReservator();
    if (pReservator)
      prodType = pReservator->GetProductType();

    WORD boardid = NO_DISPLAY_BOARD_ID, subBoardId = NO_DISPLAY_BOARD_ID;
    if (prodType == eProductTypeRMX2000)
      subBoardId = pRsrcDetailGet->GetSubBoardId();

    if (pSystemResources && pSystemResources->GetIDsFromDisplayBoardId(displayBoardId, prodType, boardid, subBoardId) == FALSE)
    {
      pGetRequest->SetConfirmObject(pRsrcDetailSnd);
      PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnServerCardDetail - Board not found, displayBoardId:", displayBoardId);
      return STATUS_OK;
    }

    WORD bIsRtm = FALSE;  // Need to be updated with input parameter value (parameter == -1 => MFA, parameter != -1 => RTM)
    if (subBoardId == RTM_SUB_BOARD_NUM)
      bIsRtm = TRUE;

    CCommDynCard* pDynCard = (CCommDynCard*)pRsrcDetailSnd->GetDynCard();
    pDynCard->SetBoardId(displayBoardId);

    CBoard* pBoard = (pSystemResources) ? pSystemResources->GetBoard(boardid) : NULL;
    if (pBoard == NULL)
    {
      pGetRequest->SetConfirmObject(pRsrcDetailSnd);
      PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnServerCardDetail - Board not found, boardId:", boardid);
      return STATUS_OK;
    }

    eCardType cardType = pBoard->GetCardType();

    for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
    {
      if (bIsRtm)
      {
        CSpanRTM* pRTM = (CSpanRTM*)(pBoard->GetRTM(i+1));
        if (!pRTM)
          continue;

        CCardRsrc* pRsrc = new CCardRsrc;
        pRsrc->SetUnitId(i+1);
        if (pRTM->GetIsAllocated())
          pRsrc->SetActive(i+1);

        // Need to update type or config (in EMA config should be displayed !, check with Kobi)
        BYTE spanType = pRTM->GetSpanType();
        if (pRTM->GetSpanType() == SPAN_GENERIC) // Not configured
        {
          pRTM->SetEnabled(FALSE);
          pRsrc->SetDisabledByError(FALSE); // faulty unit
          pRsrc->SetDisabledManually(FALSE); // disabled unit
        }
        else
        {
          if (spanType == TYPE_SPAN_T1)
            pRsrc->SetUnitCfg(T1_PRI);
          else if (spanType == TYPE_SPAN_E1)
            pRsrc->SetUnitCfg(E1_PRI);

          if (TRUE != pRTM->GetIsEnabled())
          {
            pRsrc->SetDisabledByError(TRUE); // faulty unit
            pRsrc->SetDisabledManually(TRUE); // disabled unit
          }
        }
        pRsrc->SetServiceName(pRTM->GetSpanServiceName());
        pDynCard->AddRsrc(*pRsrc);
        PDELETE(pRsrc);
      }
      else
      {
        CUnitMFA* pMFA = (CUnitMFA*)(pBoard->GetMFA(i+1));
        if (!pMFA)
          continue;

        CCardRsrc* pRsrc = new CCardRsrc;
        pRsrc->SetUnitId(i+1);
        if (pMFA->GetIsAllocated())
        {
          pRsrc->SetActive(i+1);
          DWORD promil = 0;
          // check if it's Netra unit with 3 accelerators
          if (CHelperFuncs::IsMpmRxOrNinja(cardType) && pMFA->GetUnitType() == eUnitType_Video)
          {
        	  for (int i = 0; i < ACCELERATORS_PER_UNIT_NETRA; i++)
        	  {
        		  promil += (DWORD)ceil(1000 - pMFA->GetFreeCapacity(i));
        	  }

        	  promil = (DWORD)ceil(promil / ACCELERATORS_PER_UNIT_NETRA);
          }
          else
          {
        	  promil = (DWORD)ceil(1000 - pMFA->GetFreeCapacity());
          }
          pRsrc->SetUtilization(promil);
        }

        if (pMFA->GetUnitType() == eUnitType_Art || pMFA->GetUnitType() == eUnitType_Art_Control)
          pRsrc->SetUnitType(UNIT_TYPE_SMART);
        else
          pRsrc->SetUnitType(UNIT_TYPE_VIDEO);

        if (TRUE != pMFA->GetIsEnabled())
        {
          pRsrc->SetDisabledByError(TRUE); // faulty unit
          pRsrc->SetDisabledManually(TRUE); // disabled unit
        }
        pDynCard->AddRsrc(*pRsrc);
        PDELETE(pRsrc);
      }
    }
    pGetRequest->SetConfirmObject(pRsrcDetailSnd);
  }
  else
  {
    pGetRequest->SetConfirmObject(new CDummyEntry());
    FPTRACE(eLevelInfoNormal, "CResourceMonitor::OnServerCardDetail - There is no permission to view unit details");
    pGetRequest->SetStatus(STATUS_NO_PERMISSION);
  }

  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnServerCardUnitDetail(CRequest* pGetRequest)
{
	if (pGetRequest->GetAuthorization() == SUPER)
	{
		STATUS              status = STATUS_OK;
		CSystemResources*   pSystemResources = CHelperFuncs::GetSystemResources();
                if( pSystemResources )
                {
	             eProductType prodType = pSystemResources->GetProductType();
                    if(eProductTypeNinja == prodType)
                    {
                         return OnServerCardUnitDetailNinja(pGetRequest);
                     }
                }
		CUnitRsrcDetailGet* pUnitRsrcDetailGet = (CUnitRsrcDetailGet*)pGetRequest->GetRequestObject();  // To set the updateCounter
		const WORD displayBoardId = pUnitRsrcDetailGet->GetBoardId();
		const WORD unitdId = pUnitRsrcDetailGet->GetUnitId();

		TRACEINTO << "DisplayBoardId:" << displayBoardId << ", UnitdId:" << unitdId;

		eProductType prodType = eProductTypeRMX4000;
		CReservator* pReservator = CHelperFuncs::GetReservator();
		if (pReservator)
			prodType = pReservator->GetProductType();

		WORD boardId = NO_DISPLAY_BOARD_ID, subBoardId = NO_DISPLAY_BOARD_ID;
		if (prodType == eProductTypeRMX2000)
			subBoardId = pUnitRsrcDetailGet->GetSubBoardId();

		if (pSystemResources && pSystemResources->GetIDsFromDisplayBoardId(displayBoardId, prodType, boardId, subBoardId) == FALSE)
		{
			pGetRequest->SetConfirmObject(new CUnitRsrcDetailGet(*pUnitRsrcDetailGet));
			PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnServerCardUnitDetail - Board not found, displayBoardId:", displayBoardId);
			return STATUS_OK;
		}

		WORD bIsRtm = FALSE;  // Need to be updated with input parameter value (parameter == 1 => MFA, parameter == 2 => RTM)

		if (subBoardId == RTM_SUB_BOARD_NUM)
			bIsRtm = TRUE;

		CUnitRsrcDetailGet* pUnitRsrcDetailSnd = new CUnitRsrcDetailGet;
		CBoard*             pBoard = (pSystemResources) ? pSystemResources->GetBoard(boardId) : NULL;

		if (pBoard == NULL)
		{
			pGetRequest->SetConfirmObject(pUnitRsrcDetailSnd);
			PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnServerCardUnitDetail - Board not found, boardId:", boardId);
			return STATUS_OK;
		}

		CRsrcDetailElement* pDetElmnt = (CRsrcDetailElement*)pUnitRsrcDetailSnd->GetDetElmnt();
		// to set here all needed parameters.....
		if (bIsRtm)
		{
			CSpanRTM* pSpanRtm = (CSpanRTM*)(pBoard->GetRTM(unitdId));
			if (pSpanRtm)
				status = pSpanRtm->GetRsrcDetail(pDetElmnt);
		}
		else
		{
			CUnitMFA* pMFA = (CUnitMFA*)(pBoard->GetMFA(unitdId));
			if (pMFA)
				status = pMFA->GetRsrcDetail(pDetElmnt);
		}
		pGetRequest->SetStatus(status);
		pGetRequest->SetConfirmObject(pUnitRsrcDetailSnd);
	}
	else
	{
		pGetRequest->SetConfirmObject(new CDummyEntry());
		FPTRACE(eLevelInfoNormal, "CResourceMonitor::OnServerCardUnitDetail - There is no permission to get card unit detail");
		pGetRequest->SetStatus(STATUS_NO_PERMISSION);
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnServerResourceReport(CRequest* pGetRequest)
{
  eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
  PTRACE2(eLevelInfoNormal, "CResourceMonitor::OnServerResourceReport, process_status:", GetProcessStatusName(curStatus));

  if (eProcessStartup == curStatus)
  {
    PTRACE(eLevelInfoNormal, "CResourceMonitor::OnServerResourceReport - Can't send resource report while Resource process in Startup");
    return STATUS_FAIL;
  }

  CRsrcReport* pReport = new CRsrcReport();
  CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

  STATUS status = STATUS_OK;
  if (pSystemResources)
    status = pSystemResources->CalculateResourceReport(pReport);

  CRsrcReport* pReportSend = new CRsrcReport(*pReport);
  pGetRequest->SetConfirmObject(pReportSend);
  POBJDELETE(pReport);

  return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnServerConfResourceReport(CRequest* pGetRequest)
{
  CSharedRsrcList* pSharedRsrcList   = (CSharedRsrcList*)pGetRequest->GetRequestObject();
  CSharedRsrcConfReport* pReportSend = new CSharedRsrcConfReport(pSharedRsrcList);
  CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

  STATUS status = STATUS_OK;
  if (pSystemResources)
    status = pSystemResources->CalculateConfResourceReport(pReportSend);
  pGetRequest->SetConfirmObject(pReportSend);
  return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnServerServicesResourceReport(CRequest* pGetRequest)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	CServicesRsrcReport* pReport = new CServicesRsrcReport();

	STATUS status = pSystemResources->CalculateServicesResourceReport(pReport);
	pGetRequest->SetConfirmObject(pReport);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnGetRecordingJunctionsList(CRequest* pGetRequest)
{
  PASSERTMSG_AND_RETURN_VALUE(!m_pJunctionsList, "Failed, 'm_pJunctionsList' is NULL", STATUS_FAIL);

  // junctions list already initialized at startup...
  CJunctionsList* pJunctioListSend = new CJunctionsList(*m_pJunctionsList);
  pGetRequest->SetConfirmObject(pJunctioListSend);
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnGetConfigurationList(CRequest* pGetRequest)
{
  STATUS status = STATUS_OK;

  if (CHelperFuncs::IsMode2C())
    status = OPERATION_BLOCKED;
  else
  {
    CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

    if (pSystemResources)
    {
      if (CHelperFuncs::IsFixedModeAllocationType())
      {
        status = STATUS_ILLEGAL_IN_FIXED_MPM_PLUS_MODE;
      }
      else
      {
    	CAutoModeResources* pAutoModeResources = dynamic_cast<CAutoModeResources*>(pSystemResources->GetCurrentResourcesInterface());
        CPortsConfig* pPortsConfig = pAutoModeResources ? pAutoModeResources->GetPortsConfig() : NULL;

        if (pPortsConfig)
        {
          status = pPortsConfig->GetStatus();
          CPortsConfig* pPortsCfgSend = new CPortsConfig(*pPortsConfig);
          pGetRequest->SetConfirmObject(pPortsCfgSend);
        }
        else
          status = STATUS_FAIL;
      }
    }
    else
      status = STATUS_FAIL;
  }
  PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnGetConfigurationList - status:", status);
  pGetRequest->SetStatus(status);
  if (status != STATUS_OK &&
      status != (STATUS_CURRENT_SLIDER_SETTINGS_REQUIRES_MORE_RESOURCES_THAN_AVAILBLE | WARNING_MASK) &&
      status != (STATUS_PORTS_CONFIGURATION_DOES_NOT_USE_FULL_SYSTEM_CAPACITY | WARNING_MASK))
  {
    pGetRequest->SetConfirmObject(new CDummyEntry());
  }
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnGetEnhancedConfiguration(CRequest* pGetRequest)
{
  STATUS status = STATUS_OK;

  if (CHelperFuncs::IsMode2C())
  {
    status = OPERATION_BLOCKED;
    PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnGetEnhancedConfiguration - status:", status);
    pGetRequest->SetStatus(status);
    pGetRequest->SetConfirmObject(new CDummyEntry());
    return STATUS_OK;
  }
  else
  {
    CEnhancedConfigResponse* pEnhancedConfigResponse = new CEnhancedConfigResponse;
    CSystemResources*        pSyst = CHelperFuncs::GetSystemResources();

    if (pSyst != NULL)
      status = pSyst->GetOrCheckEnhancedConfiguration(NULL, pEnhancedConfigResponse);
    else
      status = STATUS_FAIL;

    // response to EMA
    PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnGetEnhancedConfiguration - status:", status);
    pGetRequest->SetStatus(status);
    pGetRequest->SetConfirmObject(pEnhancedConfigResponse);
  }
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnCheckEnhancedPortConfiguration(CRequest* pSetRequest)
{
  STATUS status = STATUS_OK;
  if (CHelperFuncs::IsMode2C())
  {
    status = OPERATION_BLOCKED;
    PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnCheckEnhancedPortConfiguration - status:", status);
    pSetRequest->SetStatus(status);
    pSetRequest->SetConfirmObject(new CDummyEntry());
    return STATUS_OK;
  }
  else
  {
    CEnhancedConfig* pEnhancedCfg = new CEnhancedConfig();
    *pEnhancedCfg = *(CEnhancedConfig*)pSetRequest->GetRequestObject();

    CSystemResources*        pSyst = CHelperFuncs::GetSystemResources();
    CEnhancedConfigResponse* pEnhancedConfigResponse = new CEnhancedConfigResponse;

    if (pEnhancedCfg != NULL && pSyst != NULL)
    {
      pEnhancedCfg->DumpToTrace();
      status = pSyst->GetOrCheckEnhancedConfiguration(pEnhancedCfg, pEnhancedConfigResponse);
    }
    else
    {
      status = STATUS_FAIL;
    }

    POBJDELETE(pEnhancedCfg);
    // response to EMA
    PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnCheckEnhancedPortConfiguration - status:", status);
    pSetRequest->SetStatus(status);
    pSetRequest->SetConfirmObject(pEnhancedConfigResponse);
  }
  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnGetAllocationMode(CRequest* pSetRequest)
{
  STATUS status = STATUS_OK;

  CSystemResources*       pSyst = CHelperFuncs::GetSystemResources();
  CAllocationModeDetails* pAllocationModeResponse = new CAllocationModeDetails;
  if (pSyst != NULL)
    status = pSyst->GetAllocationMode(pAllocationModeResponse);
  else
    status = STATUS_FAIL;

  TRACEINTO << "CResourceMonitor::OnGetAllocationMode - mode:" << (int)pAllocationModeResponse->GetMode() << ", status:" << status;

  // response to EMA
  pSetRequest->SetStatus(status);
  pSetRequest->SetConfirmObject(pAllocationModeResponse);

  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnGetReservationList(CRequest* pGetRequest)
{
  STATUS status = STATUS_OK;
  BOOL   is_internal_reservator = FALSE;

  CReservator* pReservator = CHelperFuncs::GetReservator();
  if (NULL != pReservator)
    is_internal_reservator = pReservator->IsInternalReservator();

  TRACEINTO << "CResourceMonitor::OnGetReservationList - is_internal_reservator:" << (int)is_internal_reservator;

  CCommResRsrvDBGet* pCommResRsrvDBGet = new CCommResRsrvDBGet;
  if (TRUE == is_internal_reservator)
  {
    *pCommResRsrvDBGet = *(CCommResRsrvDBGet*)pGetRequest->GetRequestObject();
    pGetRequest->SetConfirmObject(pCommResRsrvDBGet);
  }
  else
  {
    status = STATUS_ILLEGAL;
    pGetRequest->SetStatus(status);
    pGetRequest->SetConfirmObject(pCommResRsrvDBGet);
  }
  return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnServerGetResReq(CRequest* pGetRequest)
{
  STATUS status = STATUS_OK;

  CGetResSpecific* pResSpecific = new CGetResSpecific;

  *pResSpecific = *(CGetResSpecific*)pGetRequest->GetRequestObject();

  const DWORD resId = pResSpecific->GetResID();

  CRsrvDB* pRsrvDb = CHelperFuncs::GetRsrvDB();

  if (pRsrvDb)
  {
    CCommResApi* pRes = pRsrvDb->GetRsrv(resId);
    if (!pRes)
      status = STATUS_CONF_NOT_EXISTS;
    else
      POBJDELETE(pRes);
  }
  else
    status = STATUS_FAIL;

  PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnServerGetResReq - status:", status);
  std::string responseTrancsName("TRANS_RES"); // Instead of TRANS_RES_2
  pGetRequest->SetTransName(responseTrancsName);
  pGetRequest->SetConfirmObject(pResSpecific);
  pGetRequest->SetStatus(status);

  return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CResourceMonitor::OnTimerGetResourceInformationForSNMP()
{
  eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
  if (eProcessStartup == curStatus)
  {
    PTRACE(eLevelInfoNormal, "CResourceMonitor::OnTimerGetResourceInformationForSNMP - Can't send resource report while Resource process in Startup");
    StartTimer(GET_RESOURCE_INFO_TIMER, GET_RESOURCE_INFO_TIMER_TIME_OUT_VALUE);
    return;
  }

  CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
  FPASSERT_AND_RETURN(!pProcess);
  if(pProcess->GetIsSNMPEnabled() == FALSE)
  {
	  StartTimer(GET_RESOURCE_INFO_TIMER, GET_RESOURCE_INFO_TIMER_TIME_OUT_VALUE);
	  return;
  }

  DWORD numParties = 0;

  DWORD numAPartiesTotal = 0;
  DWORD numVPartiesTotal = 0;
  DWORD numAPartiesUsed = 0;
  DWORD numVPartiesUsed = 0;

  DWORD nMode = 0;

  CRsrcReport* pReport = new CRsrcReport;
  CAllocationModeDetails* pAllocationModeResponse = new CAllocationModeDetails;

  CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

  STATUS status = STATUS_OK;
  if (pSystemResources)
  {
    status = pSystemResources->CalculateResourceReport(pReport);
    status = pSystemResources->GetAllocationMode(pAllocationModeResponse);

    nMode = pAllocationModeResponse->GetMode();

    numAPartiesTotal = pReport->GetNumParties(e_Audio,TYPE_TOTAL);
    numAPartiesUsed = pReport->GetNumParties(e_Audio, TYPE_OCCUPIED);
    numVPartiesTotal = pReport->GetNumParties(e_HD720,TYPE_TOTAL);
    numVPartiesUsed = pReport->GetNumParties(e_HD720,TYPE_OCCUPIED);

    std::ostringstream str;
    str << "\n	eRPRTtype = TYPE_TOTAL " << ", Audio = " << numAPartiesTotal << ", HD = " << numVPartiesTotal;
    str << "\n	eRPRTtype = TYPE_OCCUPIED " << ", Audio = " << numAPartiesUsed << ", HD = " << numVPartiesUsed;

    TRACEINTO << str.str().c_str();

	TRACEINTO << "mode:" << pAllocationModeResponse->GetMode() << ", status:" << status;
  }

  POBJDELETE(pReport);
  POBJDELETE(pAllocationModeResponse);

  CReservator* pReservator = CHelperFuncs::GetReservator();
  if (pReservator)
  {
    numParties = pReservator->GetDongleRestriction();
    TRACEINTO << "number parties:" << numParties;
  }

  CSegment* seg = new CSegment;
  *seg << 6u
       << static_cast<DWORD>(eTT_NumPorts)          << numParties
       << static_cast<DWORD>(eTT_NumVideoPorts)     << numVPartiesTotal
       << static_cast<DWORD>(eTT_NumVoicePorts)     << numAPartiesTotal
       << static_cast<DWORD>(eTT_NumVideoPortsUsed) << numVPartiesUsed
       << static_cast<DWORD>(eTT_NumVoicePortsUsed) << numAPartiesUsed
       << static_cast<DWORD>(eTT_RsrcAllocMode)     << nMode;

  CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_MULTIPLE_TELEMETRY_DATA_IND);

  StartTimer(GET_RESOURCE_INFO_TIMER, GET_RESOURCE_INFO_TIMER_TIME_OUT_VALUE);
}

////////////////////////////////////////////////////////////////////////////
void CResourceMonitor::SendMessageToSNMP(eTelemetryType type, DWORD value)
{
  TRACEINTO << "Type:" << type << ", value:" << value;

  CSegment* seg = new CSegment;
  *seg << static_cast<unsigned int>(type) << value;

  CManagerApi(eProcessSNMPProcess).SendMsg(seg, SNMP_UPDATE_TELEMETRY_DATA_IND);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnServerCardDetailNinja(CRequest* pGetRequest)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	CRsrcDetailGet* pRsrcDetailGet = (CRsrcDetailGet*)pGetRequest->GetRequestObject();
	CRsrcDetailGet* pRsrcDetailSnd = new CRsrcDetailGet;

	const BoardID displayBoardId = pRsrcDetailGet->GetBoardId();

	CCommDynCard* pDynCard = (CCommDynCard*)pRsrcDetailSnd->GetDynCard();
	pDynCard->SetBoardId(displayBoardId);

	// check board id if valid.
	if (DSP_CARD_SLOT_ID_0 <= displayBoardId && DSP_CARD_SLOT_ID_2 >= displayBoardId)
	{
		TRACEINTO << "DisplayBoardId:" << displayBoardId;

		DSPMonitorDspList dsp_unit_list;
		GetDspMonitorStatus(dsp_unit_list, displayBoardId - DSP_CARD_SLOT_ID_0);
		int portNumber = 0; //reorder the portNumber.   EMA display continuous portNumber.
		for (int i = 0; i < dsp_unit_list.len; ++i)
		{
			if (DSP_STAT_DEAD == dsp_unit_list.status[i].isFaulty)
				continue;  // no display if dsp is dead. (can not recovery)

			CCardRsrc cardRsrc;
			cardRsrc.SetUnitId(dsp_unit_list.status[i].dspId);
			cardRsrc.SetPortsNumber(portNumber);
			portNumber++;
			if (dsp_unit_list.status[i].isOccupied)
			{
				cardRsrc.SetActive(i + 1);
				cardRsrc.SetUtilization(dsp_unit_list.status[i].percentOccupied * 10);
			}
			cardRsrc.SetUnitType(UNIT_TYPE_VIDEO);
			cardRsrc.SetDisabledByError(dsp_unit_list.status[i].isFaulty != 0 ? TRUE : FALSE);

			CUnitMFA* pMFA = pSystemResources->GetNinjaUnit(displayBoardId, i);
			if (!pMFA)
				continue;

			if (TRUE != pMFA->GetIsEnabled())
				cardRsrc.SetDisabledManually(TRUE); // disabled unit

			pDynCard->AddRsrc(cardRsrc);
		}
		pGetRequest->SetConfirmObject(pRsrcDetailSnd);
	}
	else if (ISDN_CARD_SLOT_ID == displayBoardId)
	{
		//On Ninja, RTM DSP boardId is default 1 or FIXED_BOARD_ID_RTM_1?
		BoardID boardId = 1, subBoardId = RTM_SUB_BOARD_NUM;

		CBoard* pBoard = pSystemResources->GetBoard(boardId);
		if (!pBoard)
		{
			pGetRequest->SetConfirmObject(pRsrcDetailSnd);
			TRACEINTOLVLERR << "BoardId:" << boardId << " - Failed, Board not found";
			return STATUS_FAIL;
		}

		TRACEINTO << "DisplayBoardId:" << displayBoardId << ", BoardId:" << boardId << ", SubBoardId:" << subBoardId;

		eCardType cardType = pBoard->GetCardType();

		for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
		{
			CSpanRTM* pRTM = (CSpanRTM*)(pBoard->GetRTM(i + 1));
			if (!pRTM)
				continue;

			CCardRsrc cardRsrc;
			cardRsrc.SetUnitId(i + 1);
			cardRsrc.SetPortsNumber(i + 1);
			if (pRTM->GetIsAllocated())
				cardRsrc.SetActive(i + 1);

			// Need to update type or config (in EMA config should be displayed !, check with Kobi)
			BYTE spanType = pRTM->GetSpanType();
			if (pRTM->GetSpanType() == SPAN_GENERIC) // Not configured
			{
				pRTM->SetEnabled(FALSE);
				cardRsrc.SetDisabledByError(FALSE); // faulty unit
				cardRsrc.SetDisabledManually(FALSE); // disabled unit
			}
			else
			{
				if (spanType == TYPE_SPAN_T1)
					cardRsrc.SetUnitCfg(T1_PRI);
				else if (spanType == TYPE_SPAN_E1)
					cardRsrc.SetUnitCfg(E1_PRI);

				if (TRUE != pRTM->GetIsEnabled())
				{
					cardRsrc.SetDisabledByError(TRUE); // faulty unit
					cardRsrc.SetDisabledManually(TRUE); // disabled unit
				}
			}
			cardRsrc.SetServiceName(pRTM->GetSpanServiceName());
			pDynCard->AddRsrc(cardRsrc);
		}
		pGetRequest->SetConfirmObject(pRsrcDetailSnd);
	}
	else
	{
		PDELETE(pRsrcDetailSnd);
		TRACEINTOLVLERR << "DisplayBoardId:" << displayBoardId << " - Failed, invalid DisplayBoardId";
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceMonitor::OnServerCardUnitDetailNinja(CRequest* pGetRequest)
{
	STATUS status = STATUS_OK;
	CUnitRsrcDetailGet* pUnitRsrcDetailGet = (CUnitRsrcDetailGet*)pGetRequest->GetRequestObject();
	const WORD displayBoardId = pUnitRsrcDetailGet->GetBoardId();
	const WORD unitdId = pUnitRsrcDetailGet->GetUnitId();

	if (DSP_CARD_SLOT_ID_0 <= displayBoardId && DSP_CARD_SLOT_ID_2 >= displayBoardId)
	{
		if (5 < unitdId)
		{
			PTRACE2INT(eLevelError, " CResourceMonitor::OnServerCardDetailNinja - unit id out of range:", unitdId);
			return STATUS_FAIL;
		}
		CUnitRsrcDetailGet* pUnitRsrcDetailSnd = new CUnitRsrcDetailGet;
		CRsrcDetailElement* pDetElmnt = (CRsrcDetailElement*)pUnitRsrcDetailSnd->GetDetElmnt();
		CUnitMFA* pMFA = new CUnitMFA(displayBoardId - DSP_CARD_SLOT_ID_0, unitdId, eUnitType_Video);
		if (pMFA)
		{
			status = pMFA->GetRsrcDetailNinja(pDetElmnt);
			PDELETE(pMFA);
		}
		pGetRequest->SetStatus(status);
		pGetRequest->SetConfirmObject(pUnitRsrcDetailSnd);
	}
	else if (ISDN_CARD_SLOT_ID == displayBoardId)
	{
		TRACEINTO << "CResourceMonitor::OnServerCardUnitDetail, displayBoardId:" << displayBoardId << ", unitdId:" << unitdId;

		//On Ninja, RTM DSP boardId is default 1 or FIXED_BOARD_ID_RTM_1?
		WORD boardId = 1, subBoardId = RTM_SUB_BOARD_NUM;

		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();

		TRACEINTO << "CResourceMonitor::OnServerCardUnitDetailNinja -  displayBoardId:" << displayBoardId << " -> boardId:" << boardId << " subBoardId:" << subBoardId;

		WORD bIsRtm = FALSE;  // Need to be updated with input parameter value (parameter == 1 => MFA, parameter == 2 => RTM)

		if (subBoardId == RTM_SUB_BOARD_NUM)
			bIsRtm = TRUE;
		else
		{
			PTRACE2INT(eLevelError, " CResourceMonitor::OnServerCardUnitDetailNinja - subBoardId != RTM_SUB_BOARD_NUM :", subBoardId);
		}

		CUnitRsrcDetailGet* pUnitRsrcDetailSnd = new CUnitRsrcDetailGet;
		CBoard* pBoard = (pSystemResources) ? pSystemResources->GetBoard(boardId) : NULL;

		if (pBoard == NULL)
		{
			pGetRequest->SetConfirmObject(pUnitRsrcDetailSnd);
			PTRACE2INT(eLevelInfoNormal, "CResourceMonitor::OnServerCardUnitDetailNinja - Board not found, boardId:", boardId);
			return STATUS_OK;
		}

		CRsrcDetailElement* pDetElmnt = (CRsrcDetailElement*)pUnitRsrcDetailSnd->GetDetElmnt();
		// to set here all needed parameters.....
		if (bIsRtm)
		{
			CSpanRTM* pSpanRtm = (CSpanRTM*)(pBoard->GetRTM(unitdId));
			if (pSpanRtm)
				status = pSpanRtm->GetRsrcDetail(pDetElmnt);
		}
		pGetRequest->SetStatus(status);
		pGetRequest->SetConfirmObject(pUnitRsrcDetailSnd);
	}
	else
	{
		PTRACE2INT(eLevelError, " CResourceMonitor::OnServerCardDetailNinja - displayBoardId out of range:", displayBoardId);
		return STATUS_FAIL;
	}

	return status;
}
