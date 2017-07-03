#include "ResourceManager.h"
#include "ResourceManagerHelper.h"
#include "Request.h"
#include "PartyPreviewDrv.h"
#include "CRsrcDetailGet.h"
#include "OpcodesMcmsInternal.h"
#include "HelperFuncs.h"
#include "InternalProcessStatuses.h"
#include "PartyDebugInfo.h"
#include "CommResDBAction.h"
#include "EnhancedConfig.h"
#include "AllocationModeDetails.h"
#include "GetResSpecific.h"
#include "CommResRsrvDBAction.h"
#include "PortGauge.h"
#include "ResRsrcCalculator.h"
#include "ProcessSettings.h"
#include "AutoModeResources.h"

////////////////////////////////////////////////////////////////////////////
BEGIN_SET_TRANSACTION_FACTORY(CResourceManager)
	ON_TRANS("TRANS_CONF_2"      ,"START_MEDIA_RECORDING"           ,CJunctionParamList     ,CResourceManager::OnStartMediaRecording)
	ON_TRANS("TRANS_CONF_2"      ,"GET_PARTY_PORTS_INFO"            ,CPartyPortsInfo        ,CResourceManager::OnGetPartyPortsInfo)
	ON_TRANS("TRANS_MCU"         ,"STOP_ALL_MEDIA_RECORDING"        ,CDummyEntry            ,CResourceManager::OnStopAllMediaRecording)
	ON_TRANS("TRANS_MCU"         ,"RESET_PORT_CONFIGURATION"        ,CDummyEntry            ,CResourceManager::OnResetPortConfiguration)
	ON_TRANS("TRANS_MCU"         ,"SET_PORT_CONFIGURATION"          ,CSetPortConfiguration  ,CResourceManager::OnSetPortConfiguration)
	ON_TRANS("TRANS_MCU"         ,"SET_ENHANCED_PORT_CONFIGURATION" ,CEnhancedConfig        ,CResourceManager::OnSetEnhancedPortConfiguration)
	ON_TRANS("TRANS_RSRC_REPORT" ,"SET_PORT_GAUGE"                  ,CPortGauge             ,CResourceManager::OnSetPortGauge)
	ON_TRANS("TRANS_MCU"         ,"SET_ALLOCATION_MODE"             ,CAllocationModeDetails ,CResourceManager::OnSetAllocationMode)
	ON_TRANS("TRANS_RES_2"       ,"TERMINATE_RES"                   ,CCommResRsrvDBAction   ,CResourceManager::OnDeleteResRequest)
	ON_TRANS("TRANS_RES_2"       ,"CANCEL_REPEATED"                 ,CCommResRsrvDBAction   ,CResourceManager::OnDeleteRepeatedResRequest)
	ON_TRANS("TRANS_RES_2"       ,"TERMINATE_PROFILE"               ,CCommResDBAction       ,CResourceManager::OnDelProfileRequest)
	ON_TRANS("TRANS_CONF_1"      ,"START_PREVIEW"                   ,CPartyPreviewDrv       ,CResourceManager::OnStartVideoPreviewRequest)
	ON_TRANS("TRANS_CONF_1"      ,"STOP_PREVIEW"                    ,CPartyPreviewDrv       ,CResourceManager::OnStopVideoPreviewRequest)
	ON_TRANS("TRANS_MCU"         ,"SHIFT_RESERVATIONS_TIME"         ,CShiftTime             ,CResourceManager::OnShiftFutureConferences)
	ON_TRANS("TRANS_CARD"        ,"RESET_UNITS"                     ,CUnitListRsrcAction    ,CResourceManager::OnResetUnitsRequest)
	ON_TRANS("TRANS_CARD"        ,"ENABLE_UNITS"                    ,CUnitListRsrcAction    ,CResourceManager::OnEnableUnitsRequest)
	ON_TRANS("TRANS_CARD"        ,"DISABLE_UNITS"                   ,CUnitListRsrcAction    ,CResourceManager::OnDisableUnitsRequest)
END_TRANSACTION_FACTORY

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnStartMediaRecording(CRequest* pSetRequest)
{
	PTRACE(eLevelInfoNormal, "CResourceManager::OnStartMediaRecording");
	STATUS status = STATUS_OK;

	pSetRequest->SetConfirmObject(new CDummyEntry());

	//only admin can start media recording
	if (pSetRequest->GetAuthorization() != SUPER || IsFederalOn())
	{
		if (IsFederalOn())
			FTRACESTR(eLevelInfoNormal) << "CResourceManager::OnStartMediaRecording - No permission to start media recording in federal mode";
		else
			FTRACESTR(eLevelInfoNormal) << "CResourceManager::OnStartMediaRecording - No permission to start media recording";

		pSetRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	std::string ans;
	std::string cmd = "du -s " + MCU_MCMS_DIR + "/MediaRecording/share/ | awk '{print $1}'";

	STATUS statusCheckSize = SystemPipedCommand(cmd.c_str(), ans, TRUE, FALSE);

	if (STATUS_OK != statusCheckSize)
	{
		TRACESTR(eLevelInfoNormal) << "\nFailed to execute command: " << cmd.c_str();
	}
	long folderSize = atol(ans.c_str());
	if (folderSize > MEDIA_RECORDING_MAX_FOLDER_SIZE) // Bigger then 1.5Giga
	{
		pSetRequest->SetStatus(STATUS_MEDIA_RECORDING_FOLDER_EXCEEDED_MAX_SIZE);
		return STATUS_OK;
	}

	CJunctionParamList* pJunctionParamList = new CJunctionParamList();
	*pJunctionParamList = *(CJunctionParamList*)pSetRequest->GetRequestObject();

	DWORD mntrConfId = pJunctionParamList->GetConfId();
	DWORD mntrPartyId = pJunctionParamList->GetPartyId();
	DWORD size_limit = pJunctionParamList->GetSizeLimit();

	CJunctionParam* pJunctionParam;

	WORD i = 0;
	for (; i < MAX_NUM_JUNCTIONS; i++)
	{
		pJunctionParam = pJunctionParamList->GetJunction(i);

		if (pJunctionParam)
			status = SendStartRecordingToCm(mntrConfId, mntrPartyId, size_limit, *pJunctionParam);
	}

	// response to EMA
	pSetRequest->SetStatus(status);

	POBJDELETE(pJunctionParamList);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnStopAllMediaRecording(CRequest* pSetRequest)
{
	PTRACE(eLevelInfoNormal, " CResourceMonitor::OnStopAllMediaRecording");
	pSetRequest->SetConfirmObject(new CDummyEntry());

	if (pSetRequest->GetAuthorization() != SUPER || IsFederalOn())
	{
		if (IsFederalOn())
			FTRACESTR(eLevelInfoNormal) << "CResourceManager::OnStopAllMediaRecording - No permission to start media recording in federal mode";
		else
			FTRACESTR(eLevelInfoNormal) << "CResourceManager::OnStopAllMediaRecording - No permission to start media recording";

		pSetRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_OK;
	}

	STATUS status = SendStopRecordingToCm();

	PASSERT(status);

	// response to EMA
	pSetRequest->SetStatus(STATUS_OK);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnGetPartyPortsInfo(CRequest* pRequest)
{
	if (!m_PartyDebugInfoList)
		m_PartyDebugInfoList = new std::set<CPartyDebugInfo*>;

	CPartyPortsInfo portGetPortsInfo(*(CPartyPortsInfo*)pRequest->GetRequestObject());

	ConfMonitorID  monitorConfId  = portGetPortsInfo.GetConfID();
	PartyMonitorID monitorPartyId = portGetPortsInfo.GetPartyID();
	BOOL           isSendToCM     = portGetPortsInfo.GetIsSendToCM();

	TRACEINTO << "MonitorConfId:" << monitorConfId << ", MonitorPartyId:" << monitorPartyId << ", IsSendToCM:" << (WORD)isSendToCM;

	// check if the info request has already been received
	CPartyDebugInfo* pPartyDebugInfo = NULL;
	std::set<CPartyDebugInfo*>::iterator _end = m_PartyDebugInfoList->end();
	for (std::set<CPartyDebugInfo*>::iterator _itr = m_PartyDebugInfoList->begin(); _itr != _end; ++_itr)
	{
		if ((*_itr)->GetMonitorConfId() == monitorConfId && (*_itr)->GetMonitorPartyId() == monitorPartyId)
		{
			pPartyDebugInfo = *_itr;
			break;
		}
	}

	if (!pPartyDebugInfo) //send the request to CM
	{
		CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
		if (!pConfRsrcDB) // while spreading it has to be done, excluded EQ
		{
			PASSERT(1);
			pRequest->SetConfirmObject(new CPartyPortsInfo(""));
			return STATUS_FAIL;
		}

		ConfRsrcID confId = pConfRsrcDB->MonitorToRsrcConfId(monitorConfId);
		const CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrcByRsrcConfId(confId);
		if (!pConfRsrc)
		{
			TRACEWARN << "Failed, conference doesn't exist";
			pRequest->SetConfirmObject(new CPartyPortsInfo(""));
			return STATUS_FAIL;
		}

		const CPartyRsrc* pPartyRsrc = pConfRsrc->GetParty(monitorPartyId);
		if (!pPartyRsrc)
		{
			TRACEWARN << "Failed, party doesn't exist";
			pRequest->SetConfirmObject(new CPartyPortsInfo(""));
			return STATUS_FAIL;
		}

		PartyRsrcID partyId = pPartyRsrc->GetRsrcPartyId();

		CPartyDebugInfo* partyDbgInfo = new CPartyDebugInfo(monitorConfId, monitorPartyId, confId, partyId);
		if (!isSendToCM)
		{
			ostringstream msg;
			AddPortsToPartyInfo(msg, partyDbgInfo);
			partyDbgInfo->GetInfoString(msg);
			pRequest->SetConfirmObject(new CPartyPortsInfo(msg.str().c_str()));

			TRACEINTO << "Send debug info string without CM info:\n" << msg.str().c_str();

			pRequest->SetStatus(STATUS_OK);
			partyDbgInfo->SetInfoRetrieved();
			return STATUS_OK;
		}

		CRsrcDesc** pRsrcDescArray = new CRsrcDesc*[MAX_NUM_ALLOCATED_RSRCS];
		for (int i = 0; i < MAX_NUM_ALLOCATED_RSRCS; i++)
			pRsrcDescArray[i] = NULL;

		// Get all resource descriptors per a given party
		pConfRsrc->GetDescArrayPerResourceTypeByRsrcId(partyId, eLogical_res_none, pRsrcDescArray, MAX_NUM_ALLOCATED_RSRCS);
		for (int j = 0; j < MAX_NUM_ALLOCATED_RSRCS; ++j)
		{
			if (pRsrcDescArray[j])
			{
				eLogicalResourceTypes logical_type = pRsrcDescArray[j]->GetType();
				if (logical_type == eLogical_video_encoder_content || logical_type == eLogical_audio_encoder || logical_type == eLogical_audio_decoder || logical_type == eLogical_net)
				{
					// since the information is for ART we insert only the rtp port
					// eLogical_net: not implemented for isdn net port
					continue;
				}
				CPortDebugInfo* portInfo = new CPortDebugInfo(*pRsrcDescArray[j]);
				partyDbgInfo->AddPortInfo(*portInfo);
				delete portInfo;
			}
		}
		delete[] pRsrcDescArray;

		// add audio controller
		DWORD acConnId = m_pConnToCardManager->GetConnIdByRsrcType(ePhysical_audio_controller, E_AC_MASTER);
		ConnToCardTableEntry acEntry;
		DWORD statusSharedMemory = m_pConnToCardManager->Get(acConnId, acEntry);
		if (statusSharedMemory == STATUS_OK)
		{
			WORD acBoxId         = acEntry.boxId;
			WORD acBoardId       = acEntry.boardId;
			WORD acSubBoardId    = acEntry.subBoardId;
			WORD acUnitId        = acEntry.unitId;
			WORD acAcceleratorId = acEntry.acceleratorId;
			WORD acPortId        = acEntry.portId;

			CRsrcDesc acRsrcDesc(acConnId, eLogical_audio_controller, confId, partyId, acBoxId, acBoardId, acSubBoardId, acUnitId, acAcceleratorId, acPortId, E_AC_MASTER);
			CPortDebugInfo* portInfo = new CPortDebugInfo(acRsrcDesc);
			partyDbgInfo->AddPortInfo(*portInfo);
			delete portInfo;
		}
		else
		{
			TRACEWARN << "Failed, get audio controller from shared memory";
		}

		//add the party to the list
		pair<std::set<CPartyDebugInfo*>::iterator, bool> retElem = m_PartyDebugInfoList->insert(partyDbgInfo);

		// timer in case EMA disconnected, and info does not retrieved: calculation GET_PORTS_INFO_TIMEOUT to get info for embedded + RETRIEVE_INFO_TIMEOUT for EMA + SECOND to symc with CPortDebugInfo
		TRACEINTO << "StartTimer RETRIEVE_INFO_TIMER for 13 seconds";
		StartTimer(RETRIEVE_INFO_TIMER, GET_PORTS_INFO_TIMEOUT + RETRIEVE_INFO_TIMEOUT + SECOND);

		//send a request to Card Manager
		partyDbgInfo->SendInfoReq();
	}

	//fill the response to EMA
	string responseTrancsName("TRANS_CONF");
	pRequest->SetTransName(responseTrancsName);

	if (!pPartyDebugInfo || pPartyDebugInfo->GetStatus() != STATUS_OK)
	{
		pRequest->SetConfirmObject(new CPartyPortsInfo(""));
		pRequest->SetStatus(STATUS_IN_PROGRESS);
	}
	else
	{
		ostringstream msg;
		AddPortsToPartyInfo(msg, pPartyDebugInfo);
		pPartyDebugInfo->GetInfoString(msg);
		pRequest->SetConfirmObject(new CPartyPortsInfo(msg.str().c_str()));

		TRACEINTO << "Send debug info string:\n" << msg.str().c_str();

		pRequest->SetStatus(STATUS_OK);
		pPartyDebugInfo->SetInfoRetrieved();

		// start timer to delete party
		DeleteTimer(RETRIEVE_INFO_TIMER);
		StartTimer(RETRIEVE_INFO_TIMER, 3 * SECOND);
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnResetPortConfiguration(CRequest* pSetRequest)
{
	TRACEINTO;

	STATUS status = STATUS_OK;

	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	CProcessSettings *pProcessSettings = CHelperFuncs::GetProcessSettings();
	if (pSystemResources && pProcessSettings)
	{
		if (CHelperFuncs::IsFixedModeAllocationType())
		{
			//tbd zoe - breeze - maybe add another status
			status = STATUS_ILLEGAL_IN_FIXED_MPM_PLUS_MODE;
		}
		else
		{
			CAutoModeResources *pAutoModeResources = dynamic_cast<CAutoModeResources*>(pSystemResources->GetCurrentResourcesInterface());

			status = pAutoModeResources ? pAutoModeResources->CanSetConfigurationNow() : STATUS_FAIL;
			if (status == STATUS_OK)
			{
				status = pAutoModeResources->ResetPortConfiguration();
				if (status == STATUS_OK)
				{
					CPortsConfig* pPortsConfig = pAutoModeResources->GetPortsConfig();
					if (pPortsConfig)
					{
						status = pPortsConfig->GetStatus();
						if (pSetRequest != NULL) //if it's not terminal command
						{
							CPortsConfig* pPortsCfgSend = new CPortsConfig(*pPortsConfig);
							pSetRequest->SetConfirmObject(pPortsCfgSend);
						}
					}
					else
						status = STATUS_FAIL;
				}
			}
		}
	}
	else
		status = STATUS_FAIL;

	if (pSetRequest != NULL) //if it's not terminal command
	{
		// response to EMA
		pSetRequest->SetStatus(status);
		if (status != STATUS_OK)
		{
			CDummyEntry* pDummyEntry = new CDummyEntry;
			pSetRequest->SetConfirmObject(pDummyEntry);
		}
	}

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnSetPortConfiguration(CRequest* pSetRequest)
{
	TRACEINTO;

	STATUS status = STATUS_OK;

	if (CHelperFuncs::IsMode2C())
		status = OPERATION_BLOCKED;
	else
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		CProcessSettings* pProcessSettings = CHelperFuncs::GetProcessSettings();
		if (pSystemResources && pProcessSettings)
		{
			if (CHelperFuncs::IsFixedModeAllocationType())
			{
				status = STATUS_ILLEGAL_IN_FIXED_MPM_PLUS_MODE;
			}
			else if (pSystemResources->GetResourceAllocationType() == eAutoMpmRxMode)
			{
				// Tsahi - this is temporary disabled. need to enable it only for MPMx cards.
				status = OPERATION_BLOCKED;
			}
			else
			{
				WORD l_selctedIndexFromEma = ((CSetPortConfiguration*)pSetRequest->GetRequestObject())->GetPortConfigurationIndex();

				CAutoModeResources* pAutoModeResources = dynamic_cast<CAutoModeResources*>(pSystemResources->GetCurrentResourcesInterface());

				status = pAutoModeResources ? pAutoModeResources->CanSetConfigurationNow() : STATUS_FAIL;
				if (status == STATUS_OK)
				{
					CPortsConfig* pPortsCfg = pAutoModeResources->GetPortsConfig();
					if (pPortsCfg)
					{
						CAudioVideoConfig* pAudVidCfg = pPortsCfg->FindPortsConfigurationConfigByIndex(l_selctedIndexFromEma);
						if (pAudVidCfg)
						{
							// Update selcted index of table
							pPortsCfg->SetSelectedIndex(l_selctedIndexFromEma);

							// Update File
							char audio[120];
							char video[120];

							snprintf(audio, sizeof(audio), "%d", pAudVidCfg->GetAudio());
							snprintf(video, sizeof(video), "%d", pAudVidCfg->GetVideo());

							pProcessSettings->SetSetting("audio", audio);
							pProcessSettings->SetSetting("video", video);

							status = pAutoModeResources->UserChangedPortConfiguration();
						}
						else
							status = STATUS_FAIL;
					}
					else
						status = STATUS_FAIL;

					if (pSystemResources->GetMultipleIpServices()) //VNGR-19830
					{
						pSystemResources->SetPortConfigurationPerIpServices(l_selctedIndexFromEma);
					}
				}
			}
		}
	}

	// response to EMA
	CDummyEntry* pDummyEntry = new CDummyEntry;
	pSetRequest->SetStatus(status);
	pSetRequest->SetConfirmObject(pDummyEntry);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnSetEnhancedPortConfiguration(CRequest* pSetRequest)
{
	TRACEINTO;

	STATUS status = STATUS_OK;
	BOOL bSetEnhancedConfiguration = TRUE;

	if (CHelperFuncs::IsMode2C())
		status = OPERATION_BLOCKED;
	else
	{
		CEnhancedConfig* pEnhancedCfg = new CEnhancedConfig();
		*pEnhancedCfg = *(CEnhancedConfig*)pSetRequest->GetRequestObject();

		if (pEnhancedCfg != NULL)
		{
			// For RMX 1500Q, if HD license is disabled then do not allow CP resolution over SD30.
			if (CResRsrcCalculator::IsRMX1500Q() && !CResRsrcCalculator::IsHDenabled())
			{
				if (pEnhancedCfg->GetConfiguration(e_HD720) > 0 || pEnhancedCfg->GetConfiguration(e_HD1080p30) > 0)
				{
					TRACEINTO << "Block settings due to HD license restrictions";
					status = STATUS_1500Q_NO_CP_HD_LICENSE;
					bSetEnhancedConfiguration = FALSE;
				}
			}
			if (bSetEnhancedConfiguration)
			{
				CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
				if (pSyst != NULL)
				{
					pEnhancedCfg->DumpToTrace();
					status = pSyst->SetEnhancedConfiguration(pEnhancedCfg);
				}
				else
				{
					status = STATUS_FAIL;
				}
			}
			POBJDELETE(pEnhancedCfg);
		}
		else
		{
			status = STATUS_FAIL;
		}
	}

	// response to EMA
	pSetRequest->SetStatus(status);
	pSetRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnSetPortGauge(CRequest* pSetRequest)
{
	STATUS status = STATUS_OK;

	if (pSetRequest->GetAuthorization() == SUPER)
	{
		CPortGauge* pPortGauge = (CPortGauge*)pSetRequest->GetRequestObject();

		if (pPortGauge)
		{
			DWORD portGauge = pPortGauge->GetPortGauge();
			TRACEINTO << "PortGauge:" << portGauge;

			CSystemResources* SystemResources = CHelperFuncs::GetSystemResources();
			if (SystemResources)
				SystemResources->SetPortGauge(portGauge);
		}
		else
		{
			TRACEINTO << "Failed, invalid object";
			status = STATUS_FAIL;
		}
	}
	else
	{
		TRACEINTO << "Failed, no permission to set Port Gauge";
		status = STATUS_NO_PERMISSION;
	}

	// response to EMA
	pSetRequest->SetStatus(status);
	pSetRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnSetAllocationMode(CRequest* pSetRequest)
{
	TRACEINTO;

	STATUS status = STATUS_OK;

	if (pSetRequest->GetAuthorization() != SUPER)
	{
		TRACEINTO << "No permission to set AllocationMode";
		status = STATUS_NO_PERMISSION;
	}
	else if (CHelperFuncs::IsMode2C())
		status = OPERATION_BLOCKED;
	else
	{
		CAllocationModeDetails* pAllocationModeRequest = new CAllocationModeDetails();

		CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
		if (pAllocationModeRequest != NULL && pSystemResources != NULL)
		{
			*pAllocationModeRequest = *(CAllocationModeDetails*)pSetRequest->GetRequestObject();
			status = pSystemResources->SetAllocationMode(pAllocationModeRequest);
		}
		else
		{
			status = STATUS_FAIL;
		}

		POBJDELETE(pAllocationModeRequest);
	}
	// response to EMA
	pSetRequest->SetStatus(status);
	pSetRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnDeleteResRequest(CRequest* pRequest)
{
	if (pRequest->GetAuthorization() == ADMINISTRATOR_READONLY)
	{
		TRACEINTO << "No permission to OnDeleteResRequest for administrator readonly";
		pRequest->SetConfirmObject(new CDummyEntry());
		pRequest->SetStatus(STATUS_NO_PERMISSION);
		return STATUS_NO_PERMISSION;
	}

	CCommResRsrvDBAction* pCommResRsrvDBAction = new CCommResRsrvDBAction;
	*pCommResRsrvDBAction = *(CCommResRsrvDBAction*)pRequest->GetRequestObject();

	ConfMonitorID confId = pCommResRsrvDBAction->GetConfID();

	TRACEINTO << "MonitorConfId:" << confId;

	CReservator *pReservator = CHelperFuncs::GetReservator();
	PASSERT(!pReservator);

	STATUS status = STATUS_FAIL;
	if (pReservator)
		status = pReservator->DeleteRes(confId);

	//(***) answer to EMA !!!!
	CDummyEntry* pDummyEntry = new CDummyEntry;
	pRequest->SetTransName("TRANS_RES");
	pRequest->SetConfirmObject(pDummyEntry);
	pRequest->SetStatus(status);

	POBJDELETE(pCommResRsrvDBAction);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnDeleteRepeatedResRequest(CRequest* pRequest)
{
	CCommResRsrvDBAction* pCommResRsrvDBAction = new CCommResRsrvDBAction;
	*pCommResRsrvDBAction = *(CCommResRsrvDBAction*)pRequest->GetRequestObject();

	ConfMonitorID confId = pCommResRsrvDBAction->GetConfID();

	TRACEINTO << "MonitorConfId:" << confId;

	CReservator *pReservator = CHelperFuncs::GetReservator();
	PASSERT(!pReservator);

	STATUS status = STATUS_FAIL;
	if (pReservator)
		status = pReservator->DeleteRepeatedRes(confId);

	//(***) answer to EMA !!!!
	CDummyEntry* pDummyEntry = new CDummyEntry;
	pRequest->SetTransName("TRANS_RES");
	pRequest->SetConfirmObject(pDummyEntry);
	pRequest->SetStatus(status);

	POBJDELETE(pCommResRsrvDBAction);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnDelProfileRequest(CRequest* pRequest)
{
	CCommResDBAction* pCommResDBAction = new CCommResDBAction;
	*pCommResDBAction = *(CCommResDBAction*)pRequest->GetRequestObject();

	ConfMonitorID confId = pCommResDBAction->GetConfID();

	TRACEINTO << "MonitorConfId:" << confId;

	CReservator *pReservator = CHelperFuncs::GetReservator();
	PASSERT(!pReservator);

	STATUS status = STATUS_FAIL;
	if (pReservator)
		status = pReservator->ProfileInRsrv(confId);

	if (STATUS_OK != status) // could be STATUS_PROFILE_IN_USE_CANNOT_BE_DELETED
	{ //(***) answer to EMA !!!!
		CDummyEntry* pDummyEntry = new CDummyEntry;
		pRequest->SetTransName("TRANS_RES");
		pRequest->SetConfirmObject(pDummyEntry);
		pRequest->SetStatus(status);
	}
	else
	{
		status = STATUS_FW_REQUEST_TO_CONFPARTY;
	}

	POBJDELETE(pCommResDBAction);
	return (status);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnStartVideoPreviewRequest(CRequest* pRequest)
{
	STATUS status = STATUS_FAIL;

	CPartyPreviewDrv* pPartyPreviewDrv = new CPartyPreviewDrv;
	*pPartyPreviewDrv = *(CPartyPreviewDrv*)pRequest->GetRequestObject();

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	ConfMonitorID  monitorConfId  = pPartyPreviewDrv->GetConfID();
	PartyMonitorID monitorPartyId = pPartyPreviewDrv->GetPartyID();
	WORD           direction      = pPartyPreviewDrv->GetDirection();
	ConfRsrcID     confId         = INVALID;
	PartyRsrcID    partyId        = INVALID;

	CConfRsrc* pConfRsrc = (pConfRsrcDB) ? (CConfRsrc*)pConfRsrcDB->GetConfRsrc(monitorConfId) : NULL;
	if (pConfRsrc)
	{
		confId = pConfRsrc->GetRsrcConfId();
		const CPartyRsrc* pPartyRsrc = pConfRsrc->GetParty(monitorPartyId);
		if (pPartyRsrc)
		{
			partyId = pPartyRsrc->GetRsrcPartyId();
			const CRsrcDesc* pDesc = pConfRsrc->GetDesc(partyId, eLogical_audio_encoder);
			if (pDesc)
			{
				CBoard* pBoard = pSystemResources ? pSystemResources->GetBoard(pDesc->GetBoardId()) : NULL;
				if (pBoard)
					status = pBoard->AddVideoPreviewOnThisBoard(monitorConfId, monitorPartyId, direction);
			}
		}
	}

	TRACEINTO
		<< "\n  MonitorConfId    :" << monitorConfId
		<< "\n  MonitorPartyId   :" << monitorPartyId
		<< "\n  ConfId           :" << confId
		<< "\n  PartyId          :" << partyId
		<< "\n  Direction        :" << direction
		<< "\n  Action           :" << ((STATUS_OK == status) ? "Succeeded, Send indication to ConfParty" : "Failed, Suppress send indication to ConfParty");

	// response to EMA
	pRequest->SetTransName("TRANS_CONF");
	pRequest->SetStatus(status);
	pRequest->SetConfirmObject(pPartyPreviewDrv);
	if (status == STATUS_FAIL)
		pRequest->SetExDescription("The maximum number of previews per MCU has been reached");

	if (STATUS_OK == status)
	{
		// Response to ConfParty
		START_PREVIEW_IND_PARAMS_S startPreview;

		startPreview.status            = status;
		startPreview.monitor_conf_id   = monitorConfId;
		startPreview.monitor_party_id  = monitorPartyId;
		startPreview.m_Direction       = direction;
		startPreview.m_RemoteIPAddress = pPartyPreviewDrv->GetRemoteIP();
		startPreview.m_VideoPort       = pPartyPreviewDrv->GetVideoPort();
		startPreview.m_AudioPort       = pPartyPreviewDrv->GetAudioPort();

		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)&startPreview, sizeof(startPreview));

		CManagerApi api(eProcessConfParty);
		api.SendMsg(pSeg, RSRC_START_PREVIEW_IND);
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnStopVideoPreviewRequest(CRequest* pRequest)
{
	STATUS status = STATUS_FAIL;

	CPartyPreviewDrv* pPartyPreviewDrv = new CPartyPreviewDrv;
	*pPartyPreviewDrv = *(CPartyPreviewDrv*)pRequest->GetRequestObject();

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	ConfMonitorID  monitorConfId  = pPartyPreviewDrv->GetConfID();
	PartyMonitorID monitorPartyId = pPartyPreviewDrv->GetPartyID();
	WORD           direction      = pPartyPreviewDrv->GetDirection();
	ConfRsrcID     confId         = INVALID;
	PartyRsrcID    partyId        = INVALID;

	CConfRsrc* pConfRsrc = (pConfRsrcDB) ? (CConfRsrc*)pConfRsrcDB->GetConfRsrc(monitorConfId) : NULL;
	if (pConfRsrc)
	{
		confId = pConfRsrc->GetRsrcConfId();
		const CPartyRsrc* pPartyRsrc = pConfRsrc->GetParty(monitorPartyId);
		if (pPartyRsrc)
		{
			partyId = pPartyRsrc->GetRsrcPartyId();
			const CRsrcDesc* pDesc = pConfRsrc->GetDesc(partyId, eLogical_audio_encoder);
			if (pDesc)
			{
				CBoard* pBoard = pSystemResources ? pSystemResources->GetBoard(pDesc->GetBoardId()) : NULL;
				if (pBoard)
				{
					pBoard->RemoveVideoPreviewOnThisBoard(monitorConfId, monitorPartyId, direction);
					status = STATUS_OK;
				}
			}
		}
	}

	TRACEINTO
		<< "\n  MonitorConfId    :" << monitorConfId
		<< "\n  MonitorPartyId   :" << monitorPartyId
		<< "\n  ConfId           :" << confId
		<< "\n  PartyId          :" << partyId
		<< "\n  Direction        :" << direction
		<< "\n  Action           :" << ((STATUS_OK == status) ? "Preview resource released, Send indication to ConfParty" : "Preview not found, Suppress send indication to ConfParty");

	// response to EMA
	pRequest->SetTransName("TRANS_CONF");
	pRequest->SetStatus(STATUS_OK); // Always send OK to EMA
	pRequest->SetConfirmObject(pPartyPreviewDrv);

	if (STATUS_OK == status)
	{
		// Response to ConfParty
		STOP_PREVIEW_IND_PARAMS_S stopPreview;

		stopPreview.status           = status;
		stopPreview.monitor_conf_id  = monitorConfId;
		stopPreview.monitor_party_id = monitorPartyId;
		stopPreview.m_Direction      = direction;

		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)&stopPreview, sizeof(stopPreview));

		CManagerApi api(eProcessConfParty);
		api.SendMsg(pSeg, RSRC_STOP_PREVIEW_IND);
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnShiftFutureConferences(CRequest* pRequest)
{
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		TRACEWARN << "ProcessStatus:" << GetProcessStatusName(curStatus) << " - Invalid process status";
		return STATUS_FAIL;
	}

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN_VALUE(!pReservator, STATUS_FAIL);

	CShiftTime* pShiftTime = (CShiftTime*)pRequest->GetRequestObject();
	WORD hour_shift = pShiftTime->GetHour();
	WORD min_shift  = pShiftTime->GetMin();
	int sign        = pShiftTime->GetSign();
	WORD signValue  = (-1 == sign) ? 0 : 1;

	pReservator->ShiftStartTimeAndRereserve(hour_shift, min_shift, signValue);

	// response to EMA
	pRequest->SetStatus(STATUS_OK);
	pRequest->SetConfirmObject(new CDummyEntry);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnResetUnitsRequest(CRequest* pRequest)
{
	return UnitOperation(pRequest, RESET_UNIT_ACTION);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnEnableUnitsRequest(CRequest* pRequest)
{
	return UnitOperation(pRequest, ENABLE_UNIT_ACTION);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::OnDisableUnitsRequest(CRequest* pRequest)
{
	return UnitOperation(pRequest, DISABLE_UNIT_ACTION);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::UnitOperation(CRequest* pRequest, OPCODE opCode)
{
	STATUS status = STATUS_OK;

	CUnitListRsrcAction* pUnitList = (CUnitListRsrcAction*)pRequest->GetRequestObject();
	WORD numb_of_units = pUnitList->GetNumbOfUnits();
	for (int i = 0; i < numb_of_units; i++)
	{
		CUnitRsrcDetailGet* pUnit = pUnitList->GetUnitRsrcDetailGet(i);
		if (pUnit == NULL)
			continue;
		switch (opCode)
		{
			case RESET_UNIT_ACTION:
				status |= ResetUnitById(pUnit->GetBoardId(), pUnit->GetUnitId(), pUnit->GetSubBoardId());
				break;
			case ENABLE_UNIT_ACTION:
				status |= EnableUnitById(pUnit->GetBoardId(), pUnit->GetUnitId(), TRUE);
				break;
			case DISABLE_UNIT_ACTION:
				status |= EnableUnitById(pUnit->GetBoardId(), pUnit->GetUnitId(), FALSE);
				break;
			default:
				PTRACE(eLevelInfoNormal, "UnitOperation failed");
				break;
		}
	}

	if (status != STATUS_OK)
		status = ConverFailureStatus(status, opCode);

	pRequest->SetStatus(status);
	pRequest->SetConfirmObject(new CDummyEntry);

	return status;
}

