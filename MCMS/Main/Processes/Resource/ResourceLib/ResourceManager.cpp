#include <fstream>
#include "ResourceManager.h"
#include "ResourceManagerHelper.h"
#include "OpcodesMcmsCardMngrMaintenance.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "OpcodesMcmsShelfMngr.h"
#include "OpcodesMcmsInternal.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "MplMcmsProtocolTracer.h"
#include "RsrvManager.h"
#include "ResRsrcCalculator.h"
#include "../../../McmIncld/MPL/Card/PhysicalPortAudioCntl/AcRequestStructs.h"
#include "../../../McmIncld/MPL/Card/PhysicalPortAudioCntl/AcDefinitions.h"
#include "../../../McmIncld/MPL/Card/PhysicalPortAudioCntl/OpcodesMcmsAudioCntl.h"
#include "HelperFuncs.h"
#include "NetServicesDB.h"
#include "AllocationModeDetails.h"
#include "ProfilesDB.h"
#include "PartyDebugInfo.h"
#include "AlarmStrTable.h"
#include "WrappersCommon.h"
#include "PrettyTable.h"
#include "ConfigManagerApi.h"
#include "ConfigHelper.h"

extern void ResourceMonitorEntryPoint(void* appParam);
extern const char* GetSystemRamSizeStr(eSystemRamSize theSize);

#undef TRACEINTO
#define TRACEINTO TRACESTRFUNC(eLevelInfoHigh)

PBEGIN_MESSAGE_MAP(CResourceManager)

	ONEVENT(XML_REQUEST                                  , IDLE   , CResourceManager::HandlePostRequest )

	ONEVENT(RSRCALLOC_UNIT_CONFIG_REQ                    , ANYCASE, CResourceManager::OnResourceConfigRequest)
	ONEVENT(CM_KEEP_ALIVE_IND                            , ANYCASE, CResourceManager::OnResourceKeepAliveInd)
	ONEVENT(RSRCALLOC_IP_CONFIG_FAIL_IND                 , ANYCASE, CResourceManager::OnResourceKeepAliveInd)//for DUP_IP
	ONEVENT(ART_FIPS_140_IND                             , ANYCASE, CResourceManager::OnResourceFipsInd)
	ONEVENT(CARDS_RSRC_MFA_STARTUP_COMPLETE_IND          , ANYCASE, CResourceManager::OnResourceMfaStartupCompleteInd)
	ONEVENT(ALLOCATE_PARTY_RSRC_REQ                      , ANYCASE, CResourceManager::OnResourceAllocRequest)
	ONEVENT(ALLOCATE_CONTENT_XCODE_REQ                   , ANYCASE, CResourceManager::OnResourceAllocContentXCodeRequest)
	ONEVENT(REALLOCATE_ART_PARTY_REQ                     , ANYCASE, CResourceManager::OnResourceReAllocArtRequest)
	ONEVENT(DEALLOCATE_PARTY_RSRC_REQ                    , ANYCASE, CResourceManager::OnResourceDeAllocRequest)
	ONEVENT(DEALLOCATE_CONTENT_XCODE_REQ                 , ANYCASE, CResourceManager::OnResourceDeAllocContentXCodeRequest)
	ONEVENT(START_CONF_RSRC_REQ                          , ANYCASE, CResourceManager::OnResourceStartConfRequest)
	ONEVENT(TERMINATE_CONF_RSRC_REQ                      , ANYCASE, CResourceManager::OnResourceTerminateConfRequest)
	ONEVENT(CS_RSRC_IP_SERVICE_PARAM_IND                 , ANYCASE, CResourceManager::OnIPServicePQConfigRequest)
	ONEVENT(CS_RSRC_DELETE_IP_SERVICE_IND                , ANYCASE, CResourceManager::OnIPServiceDeleteRequest)
	ONEVENT(CS_RSRC_IP_SERVICE_PARAM_END_IND             , ANYCASE, CResourceManager::OnIPServiceConfigEnd)
	ONEVENT(CS_RSRC_UPDATE_IPV6_SERVICE_PARAM_REQ        , ANYCASE, CResourceManager::OnIPv6ServiceUpdateRequest)
	ONEVENT(CS_RSRC_DEFAULT_SERVICE_IND                  , ANYCASE, CResourceManager::OnIPServiceDefaultUpdate)
	ONEVENT(MPLAPI_MSG                                   , ANYCASE, CResourceManager::OnMplApiInd)
	ONEVENT(ACK_IND                                      , ANYCASE, CResourceManager::OnSpreadingInd)
	ONEVENT(START_PARTY_MOVE_RSRC_REQ                    , ANYCASE, CResourceManager::OnStartPartyMoveReq)
	ONEVENT(END_PARTY_MOVE_RSRC_REQ                      , ANYCASE, CResourceManager::OnEndPartyMoveReq)
	ONEVENT(MCUMNGR_TO_RSRCALLOC_LICENSING_IND           , ANYCASE, CResourceManager::OnDongleRestrictInd)
	ONEVENT(MCUMNGR_TO_RSRCALLOC_TIME_CHANGED_IND        , ANYCASE, CResourceManager::OnTimeChangedInd)
	ONEVENT(STARTUP_READ_MR_AND_PROFILE_DB_IND           , ANYCASE, CResourceManager::OnStartupMRAndProfileRead)
	ONEVENT(PROFILE_UPDATE_RSRC_IND                      , ANYCASE, CResourceManager::OnProfileUpdate)
	ONEVENT(PROFILE_ADD_RSRC_IND                         , ANYCASE, CResourceManager::OnProfileAdd)
	ONEVENT(SLAVE_ADD_PROFILE_REQ                        , ANYCASE, CResourceManager::OnProfileAdd)
	ONEVENT(PROFILE_DELETE_RSRC_IND                      , ANYCASE, CResourceManager::OnProfileDelete)
	ONEVENT(SLAVE_DELETE_PROFILE_REQ                     , ANYCASE, CResourceManager::OnProfileDelete)
	ONEVENT(SLAVE_UPDATE_PROFILE_REQ                     , ANYCASE, CResourceManager::OnProfileUpdate)
	ONEVENT(CDR_RSRC_SET_LAST_CONF_ID_IND                , ANYCASE, CResourceManager::OnLastConfIdInd)
	ONEVENT(REALLOCATE_PARTY_RSRC_REQ                    , ANYCASE, CResourceManager::OnResourceReAllocRequest)
	ONEVENT(SET_AUTO_EXTEND_CONF_ENDTIME_REQ             , ANYCASE, CResourceManager::OnSetConferenceEndTimeRequest)
	ONEVENT(RTM_ISDN_ENTITY_LOADED_IND                   , ANYCASE, CResourceManager::OnRTMBoardLoadedRequest)
	ONEVENT(RTM_ISDN_PARAMS_IND                          , ANYCASE, CResourceManager::OnRTMServiceConfigRequest)
	ONEVENT(RTM_ISDN_PARAMS_END_IND                      , ANYCASE, CResourceManager::OnRTMServiceConfigEndRequest)
	ONEVENT(RTM_ISDN_SPAN_ENABLED_IND                    , ANYCASE, CResourceManager::OnRTMCreateSpanRequest)
	ONEVENT(RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_REQ       , ANYCASE, CResourceManager::OnRTMRemoveSpanRequest)
	ONEVENT(RTM_ISDN_DISABLE_ALL_SPANS_IND               , ANYCASE, CResourceManager::OnRTMDisableAllSpansRequest)
	ONEVENT(RTM_ISDN_ADD_PHONE_RANGE_REQ                 , ANYCASE, CResourceManager::OnRTMAddPhoneRangeRequest)
	ONEVENT(RTM_ISDN_PHONE_RANGE_DELETE_IF_UPDATABLE_REQ , ANYCASE, CResourceManager::OnRTMDelPhoneRangeRequest)
	ONEVENT(RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_REQ     , ANYCASE, CResourceManager::OnRTMDelServiceRequest)
	ONEVENT(DEALLOCATE_BONDING_TEMP_PHONE_REQ            , ANYCASE, CResourceManager::OnDeallocateBondingTemporaryNumber)

	ONEVENT(UPDATE_RTM_PORT_REQ                          , ANYCASE, CResourceManager::OnRTMUpdatePortRequest)
	ONEVENT(REALLOCATE_RTM_ON_BOARD_FULL_REQ             , ANYCASE, CResourceManager::OnRTMReallocateBoardFull)
	ONEVENT(MCMS_SYSTEM_CARDS_MODE_IND                   , ANYCASE, CResourceManager::OnSystemCardsModeInd)
	ONEVENT(CARD_REMOVED_IND                             , ANYCASE, CResourceManager::OnCardRemovedRequest)
	ONEVENT(RESOURCE_UNIT_RECONFIG_IND                   , ANYCASE, CResourceManager::OnUnitReconfiguredInd)
	ONEVENT(CARDS_SLOTS_NUMBERING_CONVERSION_TABLE_IND   , ANYCASE, CResourceManager::OnSlotNumberingConversionTableInd)
	ONEVENT(SYSTEM_RAM_SIZE_IND                          , ANYCASE, CResourceManager::OnSystemRamSizeInd)  // local event
	ONEVENT(SYSTEM_CPU_PROFILE_IND                       , ANYCASE, CResourceManager::OnSystemCPUProfileInd)  // local event
	ONEVENT(RECONFIGURE_UNITS_TIMER                      , ANYCASE, CResourceManager::OnReconfigureUnitsTimer)
	ONEVENT(RESOURCE_NUMERIC_ID_REQ                      , ANYCASE, CResourceManager::OnNumericIdRequest)
	ONEVENT(UPDATE_RESOLUTION_THRESHOLD                  , ANYCASE, CResourceManager::OnUpdateResolutionThreshold)

	ONEVENT(FAILOVER_START_SLAVE                         , ANYCASE, CResourceManager::OnFailoverStartSlaveInd)
	ONEVENT(FAILOVER_RESTART_SLAVE                       , ANYCASE, CResourceManager::OnFailoverReStartSlaveInd)
	ONEVENT(FAILOVER_START_MASTER                        , ANYCASE, CResourceManager::OnFailoverStartMasterInd)
	ONEVENT(FAILOVER_SLAVE_BECOME_MASTER                 , ANYCASE, CResourceManager::OnFailoverSlaveBcmMasterInd)
	ONEVENT(FAILOVER_START_MASTER_BECOME_SLAVE           , ANYCASE, CResourceManager::OnFailoverStartMasterBecomeSlaveInd)
	ONEVENT(SLAVE_UPDATE_CONF_TIME_REQ                   , ANYCASE, CResourceManager::OnFailoverSlaveUpdateConfTimeReq)
	ONEVENT(RSRC_CHANGE_SYS_MODE_REQ                     , ANYCASE, CResourceManager::OnHandleChangeSysMode)//2 modes cop/cp
	ONEVENT(ALLOCATE_PCM_RSRC_REQ                        , ANYCASE, CResourceManager::OnResourceAllocPcmRequest)
	ONEVENT(DEALLOCATE_PCM_RSRC_REQ                      , ANYCASE, CResourceManager::OnResourceDeAllocPcmRequest)
	ONEVENT(TASK_CHANGE_STATE_FAULT_IND                  , ANYCASE, CResourceManager::OnResourceSetProcessState )
	ONEVENT(UNIT_RECOVERY_IND                            , ANYCASE, CResourceManager::OnResourceUnitRecoveryInd ) //works in MPMX only
	ONEVENT(UNIT_RECOVERY_END_IND                        , ANYCASE, CResourceManager::OnResourceUnitRecoveryEndInd ) //works in MPMX only
	ONEVENT(PARTY_DEBUG_INFO_IND                         , ANYCASE, CResourceManager::OnMplPartyDebugInfoInd )
	ONEVENT(PARTY_CM_DEBUG_INFO_IND                      , ANYCASE, CResourceManager::OnMplPartyCmDebugInfoInd )
	ONEVENT(CONF_DEBUG_INFO_IND                          , ANYCASE, CResourceManager::OnMplConfDebugInfoInd )
	ONEVENT(RETRIEVE_INFO_TIMER                          , ANYCASE, CResourceManager::OnTimerRetreiveDebugInfo )
	ONEVENT(UNIT_FATAL_IND                               , ANYCASE, CResourceManager::OnMplUnitFatalInd )    //works in MPMX only
	ONEVENT(UNIT_UNFATAL_IND                             , ANYCASE, CResourceManager::OnMplUnitUnFatalInd )    //works in MPMX only
	ONEVENT(MCUMNGR_TO_RSRC_MULTIPLE_SERVICES_IND        , ANYCASE, CResourceManager::OnMultipleServicesInd)
	ONEVENT(SYSMNTR_TO_RSRC_STOP_ALL_MEDIA_RECORDING_REQ , ANYCASE, CResourceManager::OnSysMntrStopAllMediaRecordingReq)

	ONEVENT(FORCE_DEALLOCATE_ALL_PARTIES_IN_CONF_RSRC_REQ, ANYCASE, CResourceManager::OnResourceForceDeAllocAllPartiesInConfRequest)
	ONEVENT(GET_CONF_AND_PARTIES_RSRC_IDS_REQ            , ANYCASE, CResourceManager::OnResourceGetConfAndPartiesRsrcIdsRequest)
	ONEVENT(GET_CONF_COP_RSRC_IDS_REQ                    , ANYCASE, CResourceManager::OnResourceGetConfCopRsrcIdsRequest)
	ONEVENT(GET_CONFS_AND_PARTIES_LIST_REQ               , ANYCASE, CResourceManager::OnSendAllPartiesOnUnit)
	ONEVENT(GET_PARTY_RSRC_ID_REQ                        , ANYCASE, CResourceManager::OnResourceGetPartyRsrcIdRequest)
	ONEVENT(STOP_VIDEO_PREVIEW_REQ                       , ANYCASE, CResourceManager::OnStopVideoPreview)

	ONEVENT(SNMP_CONFIG_TO_OTHER_PROGRESS                ,ANYCASE,  CResourceManager::OnSNMPConfigInd)
	ONEVENT(CM_HIGH_CPU_USAGE_IND                        ,ANYCASE,  CResourceManager::OnHighUsageCPUInd)
	ONEVENT(AVC_SVC_ADDITIONAL_PARTY_RSRC_REQ            ,ANYCASE,  CResourceManager::OnResourceGetAvcSvcAdditionalPartyRsrcReq)
	ONEVENT(CSMNGR_TO_RESOURSE_INTERFACE_UPDATE          ,ANYCASE,  CResourceManager::OnUpdateInterfaceServices)
	ONEVENT(RETRIEVE_OCCUPIED_UDP_PORTS_TIMER            ,ANYCASE,  CResourceManager::OnRetrieveOccupiedUdpPortsTimer)

PEND_MESSAGE_MAP(CResourceManager,CManagerTask);

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void ResourceManagerEntryPoint(void* appParam)
{
	CResourceManager* pResourceManager = new CResourceManager;
	pResourceManager->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////
TaskEntryPoint CResourceManager::GetMonitorEntryPoint()
{
	return ResourceMonitorEntryPoint;
}

////////////////////////////////////////////////////////////////////////////
//                        CResourceManager
////////////////////////////////////////////////////////////////////////////
CResourceManager::CResourceManager()
{
	m_pConnToCardManager   = new CConnToCardManager;
	m_operName             = NULL;
	m_PartyDebugInfoList   = NULL;
	m_pRsrcAlloc           = NULL;
	m_pRsrvManagerApi      = NULL;
}

////////////////////////////////////////////////////////////////////////////
CResourceManager::~CResourceManager()
{
	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->UnregisterTaskStateMachines();

	POBJDELETE(m_pRsrcAlloc);
	delete m_pConnToCardManager;

	if (m_operName)
		delete[] m_operName;

	if (m_PartyDebugInfoList != NULL)
	{
		m_PartyDebugInfoList->clear();
		delete m_PartyDebugInfoList;
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SelfKill()
{
	m_pRsrvManagerApi->Destroy();
	POBJDELETE(m_pRsrvManagerApi);
	CManagerTask::SelfKill();
}

////////////////////////////////////////////////////////////////////////////
void* CResourceManager::GetMessageMap()
{
	return (void*)m_msgEntries;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::ManagerPostInitActionsPoint()
{
	PTRACE(eLevelInfoHigh, "CResourceManager::ManagerPostInitActionsPoint");

	m_pRsrcAlloc = new CRsrcAlloc;

	m_pRsrvManagerApi = new CTaskApi;
	COsQueue dummyMbx;
	CreateTask(m_pRsrvManagerApi, rsrvMngrEntryPoint, &dummyMbx);

	//RTM span allocation method from SYSTEM.CFG
	RTMConfigInit();

	// send req. to CarsProcess for obtaining mix/pure mode
	SendModeReqToCardMngr();

	CStructTm curTime;
	PASSERT(SystemGetTime(curTime));
	srand((unsigned)(curTime)); // for randomize the numeric conference id

	CProcessBase* proc = CProcessBase::GetProcess();
	proc->m_NetSettings.LoadFromFile();

	//Send Resource's ready to SNMPProcess
	CManagerApi api(eProcessSNMPProcess);
	CSegment *pSeg = new CSegment;
	DWORD type = eProcessResource;
	*pSeg << type;
	api.SendMsg(pSeg, SNMP_OTHER_PROCESS_READY);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::DeclareStartupConditions()
{
	CActiveAlarm aa1(FAULT_GENERAL_SUBJECT,
					AA_NO_IP_SERVICE_PARAMS,
					MAJOR_ERROR_LEVEL,
					"No IP service was received from CSMngr",
					false,
					false);
	AddStartupCondition(aa1);

	CActiveAlarm aa2(FAULT_GENERAL_SUBJECT,
					INSUFFICIENT_RESOURCES,
					MAJOR_ERROR_LEVEL,
					"Insufficient resources",
					true,
					true);
	AddStartupCondition(aa2);


	CActiveAlarm aa4(FAULT_GENERAL_SUBJECT,
					AA_NO_LICENSING,
					MAJOR_ERROR_LEVEL,
					"Licensing was not received from McuMngr",
					false,
					false);
	AddStartupCondition(aa4);

	CActiveAlarm aa6(FAULT_GENERAL_SUBJECT,
					 NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER,
					 MAJOR_ERROR_LEVEL,
					"No media resources are available",
					 true,
					 true);
	AddStartupCondition(aa6);

	// Fault only conditions
	CActiveAlarm aaFaultOnlyMeeting(FAULT_GENERAL_SUBJECT,
					NO_MEETING_ROOM,
					MAJOR_ERROR_LEVEL,
					"Not received Meeting Room from Conf-Party",
					false,
					false);
	AddStartupConditionFaultOnly(aaFaultOnlyMeeting);

	AddStartupCondDependency(AA_NO_LICENSING, NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER);
	AddStartupCondDependency(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER, AA_NO_IP_SERVICE_PARAMS);
	AddStartupCondDependency(AA_NO_IP_SERVICE_PARAMS, INSUFFICIENT_RESOURCES );
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::ReceiveAdditionalParams(CSegment* pSeg)
{
	DWORD dwLen = 0;

	if (!pSeg)
		return;

	if (m_operName != NULL)
	{
		delete[] m_operName;
		m_operName = NULL;
	}

	*pSeg >> dwLen;

	if (dwLen > 0)
	{
		m_operName = new char[dwLen + 1];
		*pSeg >> m_operName;
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendAdditionalParams(CSegment* pSeg)
{
	if (!pSeg)
		return;

	if (m_operName)
	{
		DWORD len = strlen(m_operName);
		*pSeg << len;
		*pSeg << m_operName;
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceConfigRequest(CSegment* pMsg)
{
	RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	BoardID boardId = (params.unitsConfigParamsList[0]).physicalHeader.board_id;

	CM_UNITS_CONFIG_S result;
	memset(&result, 0, sizeof(result));

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	eProductType productType = pSystemResources->GetProductType();

	TRACEINTO << "Opcode:RSRCALLOC_UNIT_CONFIG_REQ, ProductType:" << ProductTypeToString(productType) << ", BoardId:" << boardId;

	if (productType == eProductTypeNinja)
	{
		params.cardType = eMpmRx_Ninja;

		ULONG dspLocationBitmap = ((NULL == pSystemResources) ? 0 : pSystemResources->GetDSPLocationBitmap());
		BOOL isDspLocated = FALSE;

		RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams = params.unitsConfigParamsList;

		pConfigParams[1].status = eNotExist;
		pConfigParams[1].unitType = eUndefined;

		WORD dspIdx;

		for (dspIdx = 0; dspIdx < NETRA_DSP_CHIP_COUNT_NINJA; dspIdx++)
		{
			isDspLocated = (dspLocationBitmap >> dspIdx) % 2;

			if (isDspLocated)
			{
				pConfigParams[VIDEO_UNIT_START_NUMBER_NINJA + dspIdx].status = eOk;
				pConfigParams[VIDEO_UNIT_START_NUMBER_NINJA + dspIdx].unitType = eDsp;
				pConfigParams[VIDEO_UNIT_START_NUMBER_NINJA + dspIdx].physicalHeader.board_id = pConfigParams[1].physicalHeader.board_id;
				pConfigParams[VIDEO_UNIT_START_NUMBER_NINJA + dspIdx].physicalHeader.unit_id = VIDEO_UNIT_START_NUMBER_NINJA + dspIdx;
				pConfigParams[VIDEO_UNIT_START_NUMBER_NINJA + dspIdx].pqNumber = pConfigParams[1].pqNumber;
			}
			else
			{
				pConfigParams[VIDEO_UNIT_START_NUMBER_NINJA + dspIdx].status = eNotExist;
				pConfigParams[VIDEO_UNIT_START_NUMBER_NINJA + dspIdx].unitType = eUndefined;
			}
		}

		WORD invalidCount = 0;
		dspIdx = VIDEO_UNIT_START_NUMBER_NINJA + NETRA_DSP_CHIP_COUNT_NINJA;

		for (; dspIdx < MAX_NUM_OF_UNITS; dspIdx++)
		{
			if ((eDsp == pConfigParams[dspIdx].unitType) && (eOk == pConfigParams[dspIdx].status))
			{
				pConfigParams[dspIdx].status = eNotExist;
				pConfigParams[dspIdx].unitType = eUndefined;
				invalidCount++;
			}
		}

		if (invalidCount)
			TRACEINTO << "set DSP status to be \"eNotExist\" , num_dsp = " << invalidCount;
	}

	if (CHelperFuncs::IsSoftMCU(productType) &&  //OLGA - SoftMCU temp - NEED TO BE DELETED!!!
			(params.cardType != eMpmx_Soft_Full && params.cardType != eMpmx_Soft_Half && params.cardType != eMpmRx_Ninja))
	{
		params.cardType = eMpmx_Soft_Full;
		RSRCALLOC_UNIT_CONFIG_PARAMS_S* pConfigParams = params.unitsConfigParamsList;
		WORD numDSP = 0, count = 0;
		for (int i = 0; pConfigParams && i < MAX_NUM_OF_UNITS; i++)
		{
			if ((eDsp == pConfigParams[i].unitType) && (eOk == pConfigParams[i].status))
			{
				numDSP++;
				if (numDSP > 5)
				{
					pConfigParams[i].status = eNotExist;
					pConfigParams[i].unitType = eUndefined;
					count++;
				}
			}
		}
		if (count)
			TRACEINTO << "set DSP status to be \"eNotExist\" , num_dsp = " << count;
	}

	m_resourceStartupInfo.MfaCardConfigReq(boardId, params.cardType);

	TRACEINTO << params;

	// Tsahi - need to check this part...
	// vngr-16225 cards mode received after cards loaded
	if (eSystemCardsMode_illegal == pSystemResources->GetSystemCardsMode())
	{ // SystemCardsMode not initiated
		eSystemCardsMode detectedCardsMode = CHelperFuncs::GetSystemCardsModeFromCardType((eCardType)(params.cardType));
		if (detectedCardsMode != eSystemCardsMode_illegal)
		{
			CSegment seg;
			seg << (DWORD)detectedCardsMode;
			TRACEINTO << "CardsMode:" << ::GetSystemCardsModeStr(detectedCardsMode);
			OnSystemCardsModeInd(&seg);
		}
	}

	//*** performing enable units types
	pSystemResources->ConfigureResourcesNew(MAX_NUM_OF_UNITS, &params, &result);

	TRACEINTO << result;

	//*** sending info back to Cards process
	CSegment* pRetParam = new CSegment;
	pRetParam->Put((BYTE*)&result, sizeof(result));

	COsQueue q;
	q.DeSerialize(*pMsg);
	CTaskApi api;
	api.CreateOnlyApi(q);
	api.SendMsg(pRetParam, RSRCALLOC_UNIT_CONFIG_IND);

	SendToMFA_CardConfigReq(&api); //2 modes cop/cp

	if (m_resourceStartupInfo.IsLicenseReceived() && eSystemCardsMode_illegal != pSystemResources->GetSystemCardsMode())
	{
		OnResourceCardTypeInd(boardId);
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceCardTypeInd(WORD board_id)
{
	DWORD cardType = m_resourceStartupInfo.GetCardType(board_id);

	TRACEINTO << "BoardId:" << board_id << ", CardType:" << cardType << " - Send the card type to ConfParty";

	// send the card type to ConfParty process
	CSegment* cardTypeInd = new CSegment;
	*cardTypeInd << cardType;
	CManagerApi confpartyManagerApi(eProcessConfParty);
	confpartyManagerApi.SendMsg(cardTypeInd, RSRC_CARD_TYPE_IND);

	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	eProductType productType = pSystemResources->GetProductType();

	if (CHelperFuncs::IsSoftMCU(productType) &&
			eProductTypeNinja != productType &&
			eProductTypeGesher != productType &&             // Change by hzsun : HD1080p30 support in Gesher
			eProductTypeEdgeAxis != productType &&           // Change by hzsun : HD1080p30 support in Edge
			eProductTypeCallGeneratorSoftMCU != productType) // Change by Han: HD1080P60 support in Ninja
	{
		TRACEINTO << "ProductType:" << ProductTypeToString(productType) << ", SetHD1080enabled(FALSE)";
		CResRsrcCalculator::SetHD1080enabled(FALSE);       //AVC EP can connect with HD1080
	}

	if (eProductTypeRMX1500 == productType)
	{
		if (eMpmx_20 == cardType)
		{
			TRACEINTO << "ProductType:" << ProductTypeToString(productType) << ", SetRMX1500Q(TRUE)";
			CResRsrcCalculator::SetRMX1500Q(TRUE);

			if (pSystemResources->GetDongleNumOfParties() >= 7)
			{
				TRACEINTO << "ProductType:" << ProductTypeToString(productType) << ", SetRMX1500QRatios(TRUE)";
				CResRsrcCalculator::SetRMX1500QRatios(TRUE);
				//VNGFE-7517
				CManagerApi api(eProcessMcuMngr);
				api.SendOpcodeMsg(RSRCMNGR_UPDATE_MCUMNGR_ON_1500Q);
			}

			if (!m_resourceStartupInfo.Is1500qEnableHD()) // Disable HD in 1500q
			{
				TRACEINTO << "ProductType:" << ProductTypeToString(productType) << ", SetHDenabled(FALSE)";
				CResRsrcCalculator::SetHDenabled(FALSE);
			}
		}
		else //not RMX1500Q update HD back to TRUE even if Alcatel is not marked
		{
			CManagerApi api(eProcessMcuMngr);
			api.SendOpcodeMsg(RSRCMNGR_UPDATE_HD_TRUE_IN_NON_1500Q);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendToMFA_CardConfigReq(CTaskApi* api)
{
	PTRACE(eLevelInfoHigh, "CResourceManager::SendToMFA_CardConfigReq");
	CSegment* pSeg = new CSegment;

	CARDS_CONFIG_PARAMS_S CardConfigParams;

	if (CHelperFuncs::IsMode2C())
		CardConfigParams.unSystemConfMode = eSystemConfMode_Cop;
	else
		CardConfigParams.unSystemConfMode = eSystemConfMode_Cp;

	CardConfigParams.unFutureUse = 0;

	pSeg->Put((BYTE*)&CardConfigParams, sizeof(CARDS_CONFIG_PARAMS_S));

	CTaskApi Newapi;
	Newapi.CreateOnlyApi(api->GetRcvMbx());
	Newapi.SendMsg(pSeg, CARD_CONFIG_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceKeepAliveInd(CSegment* pMsg)
{
	STATUS status = STATUS_OK;
	RSRCALLOC_KEEP_ALIVE_S keepAliveList;
	pMsg->Get((BYTE*)&keepAliveList, sizeof(keepAliveList));

	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	if (pSystemResources->IsBoardReady(keepAliveList.physicalHeader.board_id) != TRUE)
	{
		TRACEINTO << "Opcode:CM_KEEP_ALIVE_IND" << ", BoardId:" << (int)keepAliveList.physicalHeader.board_id << " - Board isn't ready";
		// VNGFE-782 - it was decided that in case we received a status != OK in OnResourceMfaStartupCompletion
		// We will reject any consequent keepAlive indication.
		return;
	}

	TRACEINTO << "Opcode:CM_KEEP_ALIVE_IND" << ", BoardId:" << (int)keepAliveList.physicalHeader.board_id;

	eChangedStateAC stateAC = eAC_None;
	pSystemResources->SetKeepAliveList(&keepAliveList, stateAC);
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessStartup == curStatus) || (eProcessIdle == curStatus))
	{
		return;
	}

	//Move Master AC
	if (eAC_Failed == stateAC || eAC_Back == stateAC)
		MoveMasterAC(keepAliveList.physicalHeader.board_id, stateAC);

	CheckResourceEnoughAndAddOrRemoveAciveAlarm();

	if (FALSE == pSystemResources->IsThereExistUtilizableUnitForAudioController())
	{
		if (false == IsActiveAlarmExistByErrorCode(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER))
		{
			AddActiveAlarm(FAULT_GENERAL_SUBJECT,
										 NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER,
										 MAJOR_ERROR_LEVEL,
										 "No media resources are available",
										 true,
										 true);
		}
	}
	else
	{
		if (true == IsActiveAlarmExistByErrorCode(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER))
			RemoveActiveAlarm(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER);
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceFipsInd(CSegment* pMsg)
{
	RSRCALLOC_ART_FIPS_140_IND_S fipsInd;
	pMsg->Get((BYTE*)&fipsInd, sizeof(fipsInd));

	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	STATUS status = pSystemResources->SetFipsInd(&fipsInd);

	TRACEINTO << "Opcode:ART_FIPS_140_IND" << ", Status:" << status;
}

////////////////////////////////////////////////////////////////////////////
	void CResourceManager::CheckResourceEnoughAndAddOrRemoveAciveAlarm(BOOL bAddActiveAlarmIfNecessary, BOOL bForcebRemoveActiveAlarm, BOOL bRemoveCard)
	{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	STATUS status = pSystemResources->IsRsrcEnough(TRUE);
	std::ostringstream msg;
	msg << "CResourceManager::CheckResourceEnoughAndAddOrRemoveAciveAlarm:"
			<< "\n  AddActiveAlarmIfNecessary :" << (DWORD)bAddActiveAlarmIfNecessary
			<< "\n  ForceRemoveActiveAlarm    :" << (DWORD)bForcebRemoveActiveAlarm
			<< "\n  RemoveCard                :" << (DWORD)bRemoveCard
			<< "\n  ResourcesStatus           :" << (DWORD)status << (status == STATUS_OK ? " (enough resources)" : " (not enough resources)");

	if (bRemoveCard)
	{
		if (STATUS_OK == status)
			status = pSystemResources->CheckEnhancedConfigurationWithCurUnitsConfig();
		else
			pSystemResources->CheckSetEnhancedConfiguration();//olga
	}

	if (STATUS_OK != status)
	{
		if (bAddActiveAlarmIfNecessary && !IsActiveAlarmExistByErrorCode(INSUFFICIENT_RESOURCES))
		{
			msg << "\n  AlarmIsAdded              :TRUE";
			switch (status)
			{
				case STATUS_CURRENT_SLIDER_SETTINGS_REQUIRES_MORE_RESOURCES_THAN_AVAILBLE:
					AddActiveAlarm(FAULT_GENERAL_SUBJECT, INSUFFICIENT_RESOURCES, MAJOR_ERROR_LEVEL, "System has insufficient resources for max capacity stated in license", true, true);
					break;
				default:
					AddActiveAlarm(FAULT_GENERAL_SUBJECT, INSUFFICIENT_RESOURCES, MAJOR_ERROR_LEVEL, "Insufficient resources", true, true);
					break;
			}
		}
	}
	else
	{
		if (bForcebRemoveActiveAlarm || IsActiveAlarmExistByErrorCode(INSUFFICIENT_RESOURCES))
		{
			msg << "\n  AlarmIsRemoved            :TRUE";
			RemoveActiveAlarm(INSUFFICIENT_RESOURCES);
			pSystemResources->FineTuneUnitsConfiguration();
		}
	}
	PTRACE(eLevelInfoNormal, msg.str().c_str());
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceMfaStartupCompleteInd(CSegment* pMsg)
{
	DWORD boardId = 0, PQ1status = 0, PQ2status = 0;

	*pMsg >> boardId >> PQ1status >> PQ2status;

	PASSERT_AND_RETURN(boardId <= 0);

	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	m_resourceStartupInfo.MfaCardStartupComplete(boardId);

	eProductType productType = pSystemResources->GetProductType();

	TRACEINTO << "Opcode:CARDS_RSRC_MFA_STARTUP_COMPLETE_IND" << ", BoardId:" << boardId << ", PQ1status:" << PQ1status << ", PQ2status:" << PQ2status << ", ProductType:" << ProductTypeToString(productType);

	switch (productType)
	{
		case eProductTypeRMX1500:
		case eProductTypeRMX2000:
		case eProductTypeRMX4000:
		case eProductTypeCallGenerator:
		case eProductTypeSoftMCU:
		case eProductTypeSoftMCUMfw:
		case eProductTypeGesher:
		case eProductTypeNinja:
		case eProductTypeEdgeAxis:
		case eProductTypeCallGeneratorSoftMCU:
			break;

		default:
		{
			PASSERT(productType);
			return;
		}
	}

	pSystemResources->SetMfaStartupComplete(boardId, PQ1status, PQ2status);

	// *** set board's unit enabled if board's startup completed!!!
	// *** check alarm removal conditions !!!
	// *** remove alarms if conditions satisfied !!!

	if (pSystemResources->IsBoardReady(boardId) == TRUE)
	{
		pSystemResources->SetBoardMfaStatus(boardId, TRUE);

		BYTE l_bWasAudioControllerMasterUpdated = FALSE;
		BYTE l_bWasAudioControllerSlaveUpdated = FALSE;

		pSystemResources->HotSwapIvrAndAudioControllersAdd(boardId, l_bWasAudioControllerMasterUpdated, l_bWasAudioControllerSlaveUpdated, m_pConnToCardManager);

		if (l_bWasAudioControllerMasterUpdated)
		{
			WORD masterBoardId = pSystemResources->GetAudioCntrlMasterBid();
			CBoard* pBoard = pSystemResources->GetBoard(masterBoardId);

			// Sending request to Cards process
			TAcTypeReq data;
			data.unBoardId = masterBoardId;
			data.unSubBoardId = MFA_SUBBOARD_ID;
			data.unUnitId = pBoard ? pBoard->GetAudioControllerUnitId() : 0;
			data.eAudioContollerType = E_AUDIO_CONTROLLER_TYPE_MASTER;

			CSegment* pSeg = new CSegment();
			pSeg->Put((BYTE*)&data, sizeof(data));

			TRACEINTO << "MasterBoardId:" << masterBoardId << " - AC_TYPE_REQ send to CardManager";

			CManagerApi cardsManagerApi(eProcessCards);
			cardsManagerApi.SendMsg(pSeg, AC_TYPE_REQ);

			// Remove Active Alarm if possible
			if (TRUE == pSystemResources->IsThereExistUtilizableUnitForAudioController()) // We have a utilizable AC unit if AC master bId != 0xFFFF
				RemoveActiveAlarm(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER);
		}

		if (l_bWasAudioControllerSlaveUpdated)
		{
			WORD slaveBoardId = (eProductTypeRMX2000 == productType) ? pSystemResources->GetAudioCntrlSlaveBid() : boardId;
			CBoard* pBoard = pSystemResources->GetBoard(slaveBoardId);

			// Sending request to Cards process
			TAcTypeReq data;
			data.unBoardId = slaveBoardId;
			data.unSubBoardId = MFA_SUBBOARD_ID;
			data.unUnitId = pBoard ? pBoard->GetAudioControllerUnitId() : 0;
			data.eAudioContollerType = (eProductTypeRMX2000 == productType) ? E_AUDIO_CONTROLLER_TYPE_SHADOW : E_AUDIO_CONTROLLER_TYPE_RESERVED;

			CSegment* pSeg = new CSegment();
			pSeg->Put((BYTE*)&data, sizeof(data));

			TRACEINTO << "SlaveBoardId:" << slaveBoardId << " - AC_TYPE_REQ send to CardManager";

			CManagerApi cardsManagerApi(eProcessCards);
			cardsManagerApi.SendMsg(pSeg, AC_TYPE_REQ);
		}

		if (!CHelperFuncs::IsSoftMCU(productType))
		{
			// reserve ART for recovery
			STATUS status = pSystemResources->ReserveRecoveryART(boardId);
			CBoard* pBoard = pSystemResources->GetBoard(boardId);
			if (pBoard)
				TRACEINTO << "BoardId:" << boardId << ", ReserveRecoveryART:" << pBoard->GetRecoveryReservedArtUnitId() << ", Status:" << status;
			else
				TRACEINTO << "BoardId:" << boardId << " - Invalid Board Id";
		}
	}

	// Tsahi - update ports configuration if the product is RMX1500Q
	if (CResRsrcCalculator::IsRMX1500Q())
	{
		// License in RMX1500Q can be only 7HD or 5HD ports.
		// Since we are using a multiply of 5, we represents a license of 7HD by 10HD. In this case we need to adjust it to 7HD.
		DWORD num_parties = pSystemResources->GetDongleNumOfParties();
		if (num_parties >= 10)
			num_parties = 7;

		pSystemResources->SetDongleNumOfParties(num_parties);

		if (pSystemResources->UpdatePortWeightsTo1500Q() == STATUS_OK)
		{
			CReservator* pReservator = CHelperFuncs::GetReservator();
			if (pReservator)
				pReservator->SetDongleRestriction(num_parties);

			PTRACE(eLevelInfoNormal, "RMX1500Q ports configured successfully");
		}
		else
		{
			PTRACE(eLevelInfoNormal, "RMX1500Q update ports configuration failed");
		}
	}

	// Update logical ports weight if needed, according to hardware resources (ports) and license ports.
	pSystemResources->UpdateLogicalWeightsAndValues();

	CheckResourceEnoughAndAddOrRemoveAciveAlarm(FALSE, TRUE);

	// Sending HW_NEW_IND to ConfMngr (Insert card case)
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessNormal == curStatus) || (eProcessMinor == curStatus) || (eProcessMajor == curStatus))
	{
		BOARD_ID_IND_PARAMS_S data;
		data.board_id = (WORD)boardId;

		CSegment* pSeg = new CSegment();
		pSeg->Put((BYTE*)&data, sizeof(data));

		TRACEINTO << "BoardId:" << boardId << " - HW_NEW_IND send to ConfParty";

		CManagerApi confpartyManagerApi(eProcessConfParty);
		confpartyManagerApi.SendMsg(pSeg, HW_NEW_IND);
	}

	UpdateIpServicesDongleRestrictions();
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::CorrectServiceName(ALLOC_PARTY_REQ_PARAMS_S* params)
{
	BOOL bIsDialOut = CHelperFuncs::IsDialOutParty(&params->isdn_span_params);
	if (bIsDialOut)
		return;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	CBoard* pBoard = pSystemResources->GetBoard(params->isdn_span_params.board_id);
	PASSERT_AND_RETURN(!pBoard);

	CSpanRTM* pSpan = (CSpanRTM*)pBoard->GetRTM(params->isdn_span_params.span_id);
	PASSERT_AND_RETURN(!pSpan);

	// Bridge-14545: correct service name
	TRACEINTOLVLERR << "OldServiceName:" << params->isdn_span_params.serviceName << ", NewServiceName:" << pSpan->GetSpanServiceName();
	strcpy_safe((char*)params->isdn_span_params.serviceName, sizeof(params->isdn_span_params.serviceName), pSpan->GetSpanServiceName());
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceAllocRequest(CSegment* pMsg)
{
	ALLOC_PARTY_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:ALLOCATE_PARTY_RSRC_REQ" << params;

	ALLOC_PARTY_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	// Bridge-14545: correct service name
	if (CHelperFuncs::IsISDNParty(params.networkPartyType))
	{
		CorrectServiceName(&params);
	}

	if (CHelperFuncs::IsMode2C())
	{
		if (params.sessionType == eSTANDALONE_session)
		{
			m_pRsrcAlloc->AllocatePartyEQ(&params, &result);
		}
		else if (CHelperFuncs::IsSessionTypeCOP(params.sessionType) || params.sessionType == eVSW_Auto_session || CHelperFuncs::IsVideo2CParty(params.videoPartyType))
		{
			if (params.networkPartyType == eISDN_network_party_type)
			{
				TRACEINTO << "Failed, ISDN_network_party_type isn't supported in RMX2000C";
				result.allocIndBase.status = STATUS_FAIL;
			}
			else if (params.sessionType == eVSW_Auto_session)
			{
				m_pRsrcAlloc->Allocate(&params, &result);
			}
			else
				m_pRsrcAlloc->AllocateParty2C(&params, &result);
		}
	}
	else
		m_pRsrcAlloc->Allocate(&params, &result);

	CSegment* pRetParam = new CSegment;
	pRetParam->Put((BYTE*)&result, sizeof(result));

	TRACEINTO << result;

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	if (result.allocIndBase.status == STATUS_OK)
	{
		PASSERTSTREAM(params.party_id != result.allocIndBase.rsrc_party_id, "RecievedId:" << params.party_id << ", AllocatedId:" << result.allocIndBase.rsrc_party_id);
	}
#endif

	ResponedClientRequest(ALLOCATE_PARTY_RSRC_IND, pRetParam);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceAllocContentXCodeRequest(CSegment* pMsg)
{
	CONFERENCE_RSRC_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	std::ostringstream msg;
	msg.precision(0);
	msg << "CResourceManager::OnResourceAllocContentXCodeRequest:" << endl;
	msg << " -----+-------+-----------------------------+-----------------------------------------------------" << endl;
	msg << " conf | party | video party role            | video party type"                                     << endl;
	msg << " id   | id    |                             |"                                                      << endl;
	msg << " -----+-------+-----------------------------+-----------------------------------------------------" << endl;

	for (uint i = 0; i < ARRAYSIZE(params.rsrc_params_list); ++i)
	{
		msg << " " << setw( 4) << right << (WORD)params.rsrc_params_list[i].monitor_conf_id << " |"
		    << " " << setw( 5) << right << (WORD)params.rsrc_params_list[i].monitor_party_id << " |"
		    << " " << setw(27) << left  << ePartyRoleNames[params.rsrc_params_list[i].partyRole] << " |"
		    << " " << setw(27) << left  << eVideoPartyTypeNames[params.rsrc_params_list[i].videoPartyType]  << endl;
	}
	msg << " -----+-------+-----------------------------+-----------------------------------------------------";

	TRACEINTO << msg.str().c_str();

	CONFERENCE_RSRC_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	result.status = STATUS_OK;

	for (uint i = 0; i < ARRAYSIZE(params.rsrc_params_list); ++i)
	{
		if (params.rsrc_params_list[i].monitor_party_id != 0)
		{
			m_pRsrcAlloc->Allocate(&params.rsrc_params_list[i], &result.allocatedRsrcs[i]);
			TRACEINTO << result.allocatedRsrcs[i];

			if (STATUS_OK != result.allocatedRsrcs[i].allocIndBase.status)
			{
				DEALLOC_PARTY_IND_PARAMS_S* pResult = new DEALLOC_PARTY_IND_PARAMS_S;
				DEALLOC_PARTY_REQ_PARAMS_S* pParams = new DEALLOC_PARTY_REQ_PARAMS_S;

				// if we already allocated other XCode resources, deallocate these resources
				for (uint j = 0; j < i; j++)
				{
					memset(pParams, 0, sizeof(DEALLOC_PARTY_REQ_PARAMS_S));
					memset(pResult, 0, sizeof(DEALLOC_PARTY_IND_PARAMS_S));

					pParams->monitor_conf_id = params.rsrc_params_list[j].monitor_conf_id;
					pParams->monitor_party_id = params.rsrc_params_list[j].monitor_party_id;

					if (pParams->monitor_party_id != 0)
						DeAlloc(pParams, pResult);
				}

				delete pParams;
				delete pResult;

				result.status = result.allocatedRsrcs[i].allocIndBase.status;

				// Stop allocation of other XCode resources
				break;
			}
		}
	}

	CSegment* pRetParam = new CSegment;
	pRetParam->Put((BYTE*)&result, sizeof(result));

	ResponedClientRequest(ALLOCATE_PARTY_RSRC_IND, pRetParam);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceReAllocArtRequest(CSegment* pMsg)
{
	ALLOC_PARTY_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:REALLOCATE_ART_PARTY_REQ" << params;

	ALLOC_PARTY_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	m_pRsrcAlloc->ReAllocateART(&params, &result);
}

////////////////////////////////////////////////////////////////////////////
void  CResourceManager::OnResourceAllocPcmRequest(CSegment* pMsg)
{
	ALLOC_PCM_RSRC_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	ALLOC_PCM_RSRC_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	m_pRsrcAlloc->AllocatePcm(&params, &result);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));

	TRACEINTO
		<< "\nALLOC_PCM_RSRC_REQ_PARAMS_S:"
		<< "\n  ConfId          :" << params.rsrc_conf_id
		<< "\n  PartyId         :" << params.rsrc_party_id
		<<"\nALLOC_PCM_RSRC_IND_PARAMS_S:"
		<< "\n  Status          :" << CProcessBase::GetProcess()->GetStatusAsString(result.status).c_str()
		<< "\n  ConfId          :" << result.rsrc_conf_id
		<< "\n  PartyId         :" << result.rsrc_party_id
		<< "\n  ConnId          :" << result.allocatedPcmRsrc.connectionId
		<<" \n  LogicalRsrcType :" << result.allocatedPcmRsrc.logicalRsrcType
		<< "\n  PcmMenuId       :" << result.pcmMenuId;

	ResponedClientRequest(ALLOCATE_PCM_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void  CResourceManager::OnResourceDeAllocPcmRequest(CSegment* pMsg)
{
	DEALLOC_PARTY_IND_PARAMS_S result;
	ALLOC_PCM_RSRC_REQ_PARAMS_S params;

	pMsg->Get((BYTE*)&params, sizeof(params));

	m_pRsrcAlloc->DeAllocatePcm(&params, &result);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));
	ResponedClientRequest(DEALLOCATE_PCM_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnCardRemovedRequest(CSegment* pMsg)
{
	CARD_REMOVED_IND_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:CARD_REMOVED_IND" << params;

	CardRemovedRequest(&params);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::DeallocateAllPartiesOnThisBoardAndFillHWRemovedPartyList(WORD boardId, WORD subBoardId, HW_REMOVED_PARTY_LIST_S* pRemovedParties)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB);

	std::map<std::string, CONF_PARTY_ID_S*>* listOfConfIdPartyIdPair = new std::map<std::string, CONF_PARTY_ID_S*>;
	std::map<std::string, CONF_PARTY_ID_S*>::iterator pairsIterator;
	CONF_PARTY_ID_S* pConfIdPartyIdPair = NULL;

	//get all parties on this board
	pConfRsrcDB->GetAllPartiesOnBoard(boardId, subBoardId, listOfConfIdPartyIdPair);

	WORD numOfParties = listOfConfIdPartyIdPair->size();

	if (numOfParties != 0)
	{
		//prepare the list to send to confparty
		//Set the size and allocate the list array
		pRemovedParties->list_size = numOfParties;
		pRemovedParties->conf_party_list = new CONF_PARTY_ID_S[numOfParties];
		memset(pRemovedParties->conf_party_list, 0, numOfParties * sizeof(CONF_PARTY_ID_S));

		//deallocate them, and also fill the list to send to confparty
		DEALLOC_PARTY_REQ_PARAMS_S deallocateParams;
		DEALLOC_PARTY_IND_PARAMS_S deallocateResult;
		deallocateParams.numOfRsrcsWithProblems = 0;
		deallocateParams.force_kill_all_ports = FALSE;
		WORD i = 0;
		for (pairsIterator = listOfConfIdPartyIdPair->begin(); pairsIterator != listOfConfIdPartyIdPair->end(); pairsIterator++)
		{
			memset(&deallocateParams, 0, sizeof(DEALLOC_PARTY_REQ_PARAMS_S));

			pConfIdPartyIdPair = pairsIterator->second;
			deallocateParams.monitor_conf_id = pConfIdPartyIdPair->monitor_conf_id;
			deallocateParams.monitor_party_id = pConfIdPartyIdPair->monitor_party_id;
			ResourceDeAllocRequest(&deallocateParams, &deallocateResult);
			pRemovedParties->conf_party_list[i++] = *pConfIdPartyIdPair;
		}
		TRACEINTO << *pRemovedParties;
	}
	else
	{
		PTRACE(eLevelInfoHigh, "CResourceManager::DeallocateAllPartiesOnThisBoardAndFillHWRemovedPartyList: no parties found on this card");
	}

	//cleanup
	if (listOfConfIdPartyIdPair)
	{
		for (pairsIterator = listOfConfIdPartyIdPair->begin(); pairsIterator != listOfConfIdPartyIdPair->end(); pairsIterator++)
		{
			pConfIdPartyIdPair = pairsIterator->second;
			delete pConfIdPartyIdPair;
		}

		listOfConfIdPartyIdPair->clear();
		delete listOfConfIdPartyIdPair;
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::DeallocateAllPartiesOnUnitAndSendHWRemovedPartyList(CUnitMFA* unitMFA)
{
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB);

	TRACEINTO << "BoardId:" << unitMFA->GetBoardId() << ", SubBoardId:" << unitMFA->GetSubBoardId() << ", UnitId:" << unitMFA->GetUnitId();

	const CActivePortsList* pActivePorts = unitMFA->GetActivePorts();
	if (pActivePorts == NULL || pActivePorts->size() == 0)
	{
		TRACEINTO << "BoardId:" << unitMFA->GetBoardId() << "No active ports on dsp";
		return;
	}

	HW_REMOVED_PARTY_LIST_S* pRemovedParties = new HW_REMOVED_PARTY_LIST_S;
	pRemovedParties->list_size = 0;

	std::list<CONF_PARTY_ID_S> confIdPartyIdList;

	int actualValidActiveParties = 0;

	for (std::set<CActivePort>::iterator port = pActivePorts->begin(); port != pActivePorts->end(); ++port)
	{
		DWORD monitorConfId = 0xFFFFFFFF, monitorPartyId = 0xFFFFFFFF;

		if (pConfRsrcDB->GetMonitorIdsRsrcIds(port->GetConfId(), port->GetPartyId(), monitorConfId, monitorPartyId) != STATUS_OK)
		{
			std::ostringstream msg;
			msg << "Failed to get monitorConfId and monitorPartyId from confId" << port->GetConfId() << " and " << "partyId " << port->GetPartyId();

			PTRACE(eLevelInfoNormal, msg.str().c_str());
			continue;
		}
		++actualValidActiveParties;
		CONF_PARTY_ID_S confIdPartyId;
		confIdPartyId.monitor_conf_id = monitorConfId;
		confIdPartyId.monitor_party_id = monitorPartyId;
		confIdPartyIdList.push_back(confIdPartyId);
	}

	//prepare the list to send to confparty
	//Set the size and allocate the list array
	pRemovedParties->list_size = actualValidActiveParties;
	pRemovedParties->conf_party_list = new CONF_PARTY_ID_S[actualValidActiveParties];
	memset(pRemovedParties->conf_party_list, 0, actualValidActiveParties * sizeof(CONF_PARTY_ID_S));

	int currentconfIdPartyId = 0;
	for (std::list<CONF_PARTY_ID_S>::iterator confIdPartyId = confIdPartyIdList.begin(); confIdPartyId != confIdPartyIdList.end(); ++currentconfIdPartyId, ++confIdPartyId)
	{
		pRemovedParties->conf_party_list[currentconfIdPartyId] = *confIdPartyId;
	}
	TRACEINTO << *pRemovedParties;

	SendRemovedPartyListToConfPartyAndClean(pRemovedParties);
	delete pRemovedParties;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::ExecUnitFatalOrUnFatalInd(UNIT_RECOVERY_S* pParams, BYTE recievedIsEnable)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	TRACEINTO << *pParams;

	WORD board_id     = pParams->unit_recover.board_id;
	WORD sub_board_id = pParams->unit_recover.sub_board_id;
	WORD unit_id      = pParams->unit_recover.unit_id;

	eProductType productType = pSystemResources->GetProductType();

	switch (productType)
	{
		case eProductTypeRMX1500:
		case eProductTypeRMX2000:
		case eProductTypeRMX4000:
		case eProductTypeCallGenerator:
		case eProductTypeNinja:
			break;

		default:
		{
			PASSERT(productType);
			return;
		}
	}

	CUnitMFA* pUnitMFA = pSystemResources->GetUnit(board_id, unit_id);
	PASSERT_AND_RETURN(!pUnitMFA);

	pUnitMFA->SetFatal(!recievedIsEnable);

/*  removed by BRIDGE-15218: "insufficient resources" is only needed when system start-up. dsp recovery will not trigger this alarm.
	if (productType == eProductTypeNinja)
		CheckResourceEnoughAndAddOrRemoveAciveAlarm();
*/
	if (productType == eProductTypeNinja && !recievedIsEnable)
		return;

	if (recievedIsEnable) // UnFatal - no actions required
		return;

	//Update AC and IVR controller (if necessary) in shared memory
	BYTE l_bWasAudioControllerMasterUpdated = FALSE;
	BYTE l_bWasIvrControllerUpdated = FALSE;

	CBoard* pBoard = pSystemResources->GetBoard(sub_board_id);
	PASSERT_AND_RETURN(!pBoard);

	WORD acUnitID = pBoard->GetAudioControllerUnitId();
	if (acUnitID == unit_id)
	{
		pSystemResources->HotSwapIvrAndAudioControllersRemove(board_id, sub_board_id, l_bWasAudioControllerMasterUpdated, l_bWasIvrControllerUpdated, m_pConnToCardManager);
	}

	DeallocateAllPartiesOnUnitAndSendHWRemovedPartyList(pUnitMFA);

	if (l_bWasAudioControllerMasterUpdated)
	{
		UpdateAudioController(pSystemResources);
	}
	if (l_bWasIvrControllerUpdated)
	{ // Send IVR UPDATE to ConfParty process
		UpdateIvrController(pSystemResources);
	}

	//Add active alarm in case we don't have utilizable AC unit in the system (Ohad)
	if (FALSE == pSystemResources->IsThereExistUtilizableUnitForAudioController())
	{
		if (false == IsActiveAlarmExistByErrorCode(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER))
		{
			AddActiveAlarm(FAULT_GENERAL_SUBJECT, NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER, MAJOR_ERROR_LEVEL, "No media resources are available", true, true);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendRemovedPartyListToConfPartyAndClean(HW_REMOVED_PARTY_LIST_S* pRemovedParties)
{
	PASSERT_AND_RETURN(!pRemovedParties);

	if (pRemovedParties->list_size > 0)
	{
		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)&(pRemovedParties->list_size), sizeof(DWORD));
		if (pRemovedParties->list_size)
			pSeg->Put((BYTE *)(pRemovedParties->conf_party_list), sizeof(CONF_PARTY_ID_S) * pRemovedParties->list_size);
		//Send ASynch Msg to the confparty process
		CManagerApi confpartyManagerApi(eProcessConfParty);
		confpartyManagerApi.SendMsg(pSeg, HW_REMOVED_PARTY_LIST_IND);

		TRACEINTO << "Size:" << pRemovedParties->list_size;
		delete[] pRemovedParties->conf_party_list;
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendConfPartyListToConfPartyAssist(CONF_PARTY_LIST_S* party_list)
{
	PASSERT_AND_RETURN(!party_list);

	if (party_list->list_size > 0)
	{
		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)&(party_list->list_size), sizeof(DWORD));
		if (party_list->list_size)
			pSeg->Put((BYTE *)(party_list->conf_party_list), sizeof(CONF_PARTY_ID_S) * party_list->list_size);

		//Send ASynch Msg to the confparty process
		CManagerApi confpartyManagerApi(eProcessConfParty);
		confpartyManagerApi.SendMsg(pSeg, GET_CONFS_AND_PARTIES_LIST_IND);

		TRACEINTO << "Size:" << party_list->list_size;
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::UpdateAudioController(CSystemResources* pSystemResources)
{
	WORD masterBoardId = pSystemResources->GetAudioCntrlMasterBid();
	CBoard* pBoard = pSystemResources->GetBoard(masterBoardId);

	// Sending request to Cards process
	TAcTypeReq data;
	data.unBoardId = masterBoardId;
	data.unSubBoardId = MFA_SUBBOARD_ID;
	data.unUnitId = pBoard ? pBoard->GetAudioControllerUnitId() : 0;
	data.eAudioContollerType = E_AUDIO_CONTROLLER_TYPE_MASTER;

	CSegment* pSeg = new CSegment();
	pSeg->Put((BYTE*)&data, sizeof(data));

	CManagerApi cardsManagerApi(eProcessCards);
	cardsManagerApi.SendMsg(pSeg, AC_TYPE_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::UpdateIvrController(CSystemResources* pSystemResources)
{
	CManagerApi confpartyManagerApi(eProcessConfParty);
	confpartyManagerApi.SendOpcodeMsg(UPDATE_IVR_CNTR_IND);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::CardRemovedRequest(CARD_REMOVED_IND_S* pParams)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	BoardID boardId = pParams->BoardID;
	SubBoardID subBoardId = pParams->SubBoardID;

	//Disable all units on this card.
	//If there are no units, it means that there is no card, so return
	BYTE someUnits = pSystemResources->DisableAllUnitsAndSpans(boardId, subBoardId);
	if (someUnits == FALSE)
		return;

	//Remove all parties on this board from our list and also prepare list to send to confparty
	HW_REMOVED_PARTY_LIST_S* pRemovedParties = new HW_REMOVED_PARTY_LIST_S;
	pRemovedParties->list_size = 0;
	DeallocateAllPartiesOnThisBoardAndFillHWRemovedPartyList(boardId, subBoardId, pRemovedParties);

	//Update AC and IVR controller (if necessary) in shared memory
	BYTE l_bWasAudioControllerMasterUpdated = FALSE;
	BYTE l_bWasIvrControllerUpdated = FALSE;
	pSystemResources->HotSwapIvrAndAudioControllersRemove(boardId, subBoardId, l_bWasAudioControllerMasterUpdated, l_bWasIvrControllerUpdated, m_pConnToCardManager);

	//Remove the card from our list: remove all units and spans, and update the configuration
	pSystemResources->RemoveCard(boardId, subBoardId);

	//send the list of all parties to confparty
	SendRemovedPartyListToConfPartyAndClean(pRemovedParties);

	delete pRemovedParties;

	//Send Update IVR controller (if necessary)
	if (l_bWasAudioControllerMasterUpdated)
	{
		UpdateAudioController(pSystemResources);

	}
	if (l_bWasIvrControllerUpdated)
	{   // Send IVR UPDATE to ConfParty process
		UpdateIvrController(pSystemResources);
	}

	//Update logical ports weight if needed, according to hardware resources (ports) and license ports.
	pSystemResources->UpdateLogicalWeightsAndValues();

	//Check if the left resources are enough
	CheckResourceEnoughAndAddOrRemoveAciveAlarm(TRUE, FALSE, TRUE);

	//Add active alarm in case we don't have utilizable AC unit in the system (Ohad)
	if (FALSE == pSystemResources->IsThereExistUtilizableUnitForAudioController())
	{
		if (false == IsActiveAlarmExistByErrorCode(NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER))
		{
			AddActiveAlarm(FAULT_GENERAL_SUBJECT, NO_UTILIZABLE_UNIT_FOR_AUDIO_CONTROLLER, MAJOR_ERROR_LEVEL, "No media resources are available", true, true);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceReAllocRequest(CSegment* pMsg)
{
	ALLOC_PARTY_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:REALLOCATE_PARTY_RSRC_REQ" << params;

	ALLOC_PARTY_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	if (CHelperFuncs::IsMode2C() && (CHelperFuncs::IsSessionTypeCOP(params.sessionType) ||
	                                 params.sessionType == eVSW_Auto_session ||
	                                 CHelperFuncs::IsVideo2CParty(params.videoPartyType)))
	{
		TRACEINTO << "Reallocate for COP / NxM conference is restricted";
	}
	else
		m_pRsrcAlloc->ReAllocate(&params, &result);

	CSegment* pRetParam = new CSegment;
	pRetParam->Put((BYTE*)&result, sizeof(result));

	TRACEINTO << result;

	ResponedClientRequest(REALLOCATE_PARTY_RSRC_IND, pRetParam);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::ResourceDeAllocRequest(DEALLOC_PARTY_REQ_PARAMS_S* pParams, DEALLOC_PARTY_IND_PARAMS_S* pResult)
{
	TRACEINTO << *pParams;

	m_pRsrcAlloc->DeAllocate(pParams, pResult);

	TRACEINTO << *pResult;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceDeAllocRequest(CSegment* pMsg)
{
	DEALLOC_PARTY_IND_PARAMS_S result;
	DEALLOC_PARTY_REQ_PARAMS_S params;

	pMsg->Get((BYTE*)&params, sizeof(params));

	DeAlloc(&params, &result);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));
	ResponedClientRequest(DEALLOCATE_PARTY_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceDeAllocContentXCodeRequest(CSegment* pMsg)
{
	WORD numEncodersToDeallocate = 0;

	*pMsg >> numEncodersToDeallocate;

	TRACEINTO << "NumEncodersToDeallocate:" << numEncodersToDeallocate;

	DEALLOC_PARTY_IND_PARAMS_S result;
	DEALLOC_PARTY_REQ_PARAMS_S params;

	for (WORD i = 0; i < numEncodersToDeallocate; ++i)
	{
		pMsg->Get((BYTE*)&params, sizeof(params));

		DeAlloc(&params, &result);

		CSegment* pSeg = new CSegment;
		pSeg->Put((BYTE*)&result, sizeof(result));
		ResponedClientRequest(DEALLOCATE_CONTENT_XCODE_IND, pSeg);
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceForceDeAllocAllPartiesInConfRequest(CSegment* pMsg)
{
	DWORD status = STATUS_FAIL;
	ConfMonitorID confId = 0;

	*pMsg >> confId;

	TRACEINTO << "MonitorConfId:" << confId;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB)
	{
		CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(confId);
		if (pConfRsrc)
		{
			pConfRsrc->RemoveAllParties(TRUE);
			status = STATUS_OK;
		}
		else
		{
			TRACEINTOLVLERR << "MonitorConfId:" << confId << " - Failed, cannot find conference";
			status = STATUS_FAIL;
		}
	}

	CSegment* pSeg = new CSegment;
	*pSeg << status;
	ResponedClientRequest(FORCE_DEALLOCATE_ALL_PARTIES_IN_CONF_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceGetConfAndPartiesRsrcIdsRequest(CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;

	ConfMonitorID confId = 0;
	BOOL isPartiesListNeeded = TRUE;

	*pMsg >> confId;
	*pMsg >> isPartiesListNeeded;

	TRACEINTO << "MonitorConfId:" << confId << ", isPartiesListNeeded:" << (int)isPartiesListNeeded;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	if (pConfRsrcDB)
	{
		CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(confId);
		if (pConfRsrc)
		{
			*pSeg << pConfRsrc->GetRsrcConfId();

			WORD numOfParties = 0;
			if (isPartiesListNeeded)
			{
				numOfParties = pConfRsrc->GetNumParties();

				*pSeg << numOfParties;

				if (numOfParties > 0)
				{
					const std::set<CPartyRsrc>* pPartiesList = pConfRsrc->GetPartiesList();

					std::set<CPartyRsrc>::const_iterator _iiEnd = pPartiesList->end();
					for (std::set<CPartyRsrc>::const_iterator _ii = pPartiesList->begin(); _ii != _iiEnd; ++_ii)
						*pSeg << _ii->GetRsrcPartyId();
				}
			}
			else
			{
				*pSeg << numOfParties;
			}
		}
		else
		{
			TRACEINTOLVLERR << "MonitorConfId:" << confId << " - Failed, cannot find conference";
		}
	}

	ResponedClientRequest(GET_CONF_AND_PARTIES_RSRC_IDS_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceGetPartyRsrcIdRequest(CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;

	ConfMonitorID confId = 0;
	PartyMonitorID partyId = 0;

	*pMsg >> confId;
	*pMsg >> partyId;

	TRACEINTO << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB)
	{
		CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(confId);
		if (pConfRsrc)
		{
			const CPartyRsrc* pPartyRsrc = pConfRsrc->GetParty(partyId);
			if (pPartyRsrc)
			{
				*pSeg << pPartyRsrc->GetRsrcPartyId();
			}
			else
			{
				TRACEINTOLVLERR << "MonitorConfId:" << confId << ", MonitorPartyId:" << partyId << " - Failed, cannot find party";
			}
		}
		else
		{
			TRACEINTOLVLERR << "MonitorConfId:" << confId << " - Failed, cannot find conference";
		}
	}

	ResponedClientRequest(GET_PARTY_RSRC_ID_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceGetConfCopRsrcIdsRequest(CSegment* pMsg)
{
	CSegment* pSeg = new CSegment;

	ConfMonitorID confId = 0;

	*pMsg >> confId;

	TRACEINTO << "MonitorConfId:" << confId;

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB)
	{
		CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(confId);
		if (pConfRsrc)
		{
			CRsrcDesc** pRsrcDescArray = NULL;
			DWORD num_rsrc = pConfRsrc->GetDescArray(pRsrcDescArray);

			if (pRsrcDescArray)
			{
				*pSeg << pConfRsrc->GetRsrcConfId();
				*pSeg << num_rsrc;

				ALLOCATED_COP_RSRC_PARAM_S copRsrcParams;
				memset(&copRsrcParams, 0, sizeof(copRsrcParams));

				for (DWORD i = 0; i < num_rsrc; i++)
				{
					if (pRsrcDescArray[i])
					{
						copRsrcParams.rsrcEntityId    = pRsrcDescArray[i]->GetRsrcPartyId();
						copRsrcParams.logicalRsrcType = pRsrcDescArray[i]->GetType();
						copRsrcParams.connectionId    = pRsrcDescArray[i]->GetConnId();

						pSeg->Put((BYTE*)&copRsrcParams, sizeof(copRsrcParams));
					}
				}
				delete[] pRsrcDescArray;
			}
		}
		else
		{
			TRACEINTOLVLERR << "MonitorConfId:" << confId << " - Failed, cannot find conference";
		}
	}

	ResponedClientRequest(GET_CONF_COP_RSRC_IDS_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::DeAlloc(DEALLOC_PARTY_REQ_PARAMS_S* pParams, DEALLOC_PARTY_IND_PARAMS_S* pResult)
{
	m_pRsrcAlloc->DeAllocate(pParams, pResult); //parameters validity check. inside / outside?

	TRACEINTO << *pResult;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceStartConfRequest(CSegment* pMsg)
{
	CONF_RSRC_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "START_CONF_RSRC_REQ:" << params;

	CONF_RSRC_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (!pSystemResources)
	{
		result.status = STATUS_FAIL;
		result.rsrc_conf_id = 0xFFFFFFFF; //not allocated
	}
	else if (CHelperFuncs::IsMode2C() && pSystemResources->GetResourceAllocationType() != eAutoBreezeMode)
	{
		result.status = STATUS_FAIL;
		result.rsrc_conf_id = 0xFFFFFFFF; //not allocated
		TRACEWARN << "Failed because system is not in auto mode";
	}
	else
	{
		m_pRsrcAlloc->StartConf(&params, &result); //parameters validity check. inside / outside?
	}

	TRACEINTO << result;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));
	ResponedClientRequest(START_CONF_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceTerminateConfRequest(CSegment* pMsg)
{
	CONF_RSRC_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:TERMINATE_CONF_RSRC_REQ" << params;

	CONF_RSRC_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	m_pRsrcAlloc->TerminateConf(&params, &result);

	TRACEINTO << result;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));
	ResponedClientRequest(TERMINATE_CONF_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSetConferenceEndTimeRequest(CSegment* pMsg)
{
	SET_CONFERENCE_ENDTIME_REQ_PARAMS_S params;
	*pMsg >> params.monitorConfId;
	params.newEndTime.DeSerialize(NATIVE, *pMsg);
	TRACEINTO << "MonitorConfId:" << params.monitorConfId << ", EndTime:" << params.newEndTime;

	SET_CONFERENCE_ENDTIME_IND_PARAMS_S result;
	result.monitorConfId = params.monitorConfId;
	result.newEndTime = params.newEndTime;
	result.isSetByOperator = false;

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (!pReservator)
	{
		PASSERT(1);
		result.status = STATUS_FAIL;
	}
	else
	{
		result.status = pReservator->SetConferenceEndTimeRequest(params.monitorConfId, &(params.newEndTime));
	}

	if (result.status)
		TRACEWARN << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(result.status);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));

	CManagerApi api(eProcessConfParty);
	api.SendMsg(pSeg, SET_CONFERENCE_ENDTIME_IND);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnFailoverSlaveUpdateConfTimeReq(CSegment* pMsg)
{
	SET_CONFERENCE_ENDTIME_REQ_PARAMS_S params;
	*pMsg >> params.monitorConfId;
	params.newEndTime.DeSerialize(NATIVE, *pMsg);
	TRACEINTO << "MonitorConfId:" << params.monitorConfId << ", EndTime:" << params.newEndTime;

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN(!pReservator);

	STATUS status = pReservator->SetConferenceEndTimeRequest(params.monitorConfId, &(params.newEndTime));

	if (status)
		TRACEWARN << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnIPServicePQConfigRequest(CSegment* pMsg)
{
	IP_SERVICE_UDP_RESOURCES_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	m_resourceStartupInfo.IPServiceConfigReqInd(params.ServId, params.ServName);

	TRACEINTO << "Opcode:CS_RSRC_IP_SERVICE_PARAM_IND" << params;

	UDP_PORT_RANGE_S udpPortRange;
	memset(&udpPortRange, 0, sizeof(UDP_PORT_RANGE_S));
	udpPortRange.ServiceId = params.ServId;

	STATUS status = STATUS_FAIL;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		status = pSystemResources->ConfigureIPServicePQResources(&params, udpPortRange);
		if (STATUS_OK == status)
		{
			pSystemResources->SetStartupCond(eIpService, TRUE);
			RemoveActiveAlarm(AA_NO_IP_SERVICE_PARAMS);
		}
	}

	SendUdpPortRangeParamsToCS(udpPortRange);

	if (status)
		TRACEWARN << "Status:" << CProcessBase::GetProcess()->GetStatusAsString(status);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnIPv6ServiceUpdateRequest(CSegment* pMsg)
{
	IPV6_ADDRESS_UPDATE_RESOURCES_S ipV6AddressUpdate;
	memcpy(&ipV6AddressUpdate, pMsg->GetPtr(), sizeof(ipV6AddressUpdate));

	std::ostringstream msg;

	msg << "ServiceId:" << ipV6AddressUpdate.ServId;
	for (int spanIndex = 0; spanIndex < MAX_NUM_PQS; spanIndex++)
	{
		msg
			<< "\nSpan:"       << spanIndex
			<< ", BoxId:"      << ipV6AddressUpdate.IPServUDPperPQList[spanIndex].boxId
			<< ", BoardId:"    << ipV6AddressUpdate.IPServUDPperPQList[spanIndex].boardId
			<< ", SubBoardId:" << ipV6AddressUpdate.IPServUDPperPQList[spanIndex].subBoardId
			<< ", PqId:"       << ipV6AddressUpdate.IPServUDPperPQList[spanIndex].PQid
			<< ", IPV6:"       << CIPV6AraryWrapper(ipV6AddressUpdate.IPServUDPperPQList[spanIndex].IpV6Addr);
	}

	TRACEINTO << "Opcode:CS_RSRC_UPDATE_IPV6_SERVICE_PARAM_REQ" << msg.str().c_str();

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->IPv6ServiceUpdate(&ipV6AddressUpdate);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnIPServiceDeleteRequest(CSegment* pMsg)
{
	Del_Ip_Service_S ipService;
	memcpy(&ipService, pMsg->GetPtr(), sizeof(ipService));

	TRACEINTO << "ServiceName:" << ipService.service_name << ", ServiceId:" << ipService.service_id;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		STATUS status = pSystemResources->DeleteIPService(&ipService);
		PASSERT(status);
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendRsrcIpServiceParamsReqToCS()
{
	TRACEINTO << "Opcode:CS_RSRC_IP_SERVICE_PARAM_REQ, RESOURCE-->MANAGER";
	CManagerApi api(eProcessCSMngr);
	STATUS res = api.SendOpcodeMsg(CS_RSRC_IP_SERVICE_PARAM_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendRsrcRamSizeReqToMcuMngr()
{
	TRACEINTO << "Opcode:RESOURCE_SYSTEM_RAM_SIZE_REQ, RESOURCE-->MANAGER";
	CManagerApi api(eProcessMcuMngr);
	STATUS res = api.SendOpcodeMsg(RESOURCE_SYSTEM_RAM_SIZE_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendRsrcCPUProfileReqToMcuMngr()
{
	TRACEINTO << "Opcode:RESOURCE_SYSTEM_CPU_PROFILE_REQ, RESOURCE-->MANAGER";
	CManagerApi api(eProcessMcuMngr);
	STATUS res = api.SendOpcodeMsg(RESOURCE_SYSTEM_CPU_PROFILE_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendCfsParamsToCS()
{
	RSRC_CFS_S param;
	memset(&param, 0, sizeof(RSRC_CFS_S));

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN(!pReservator);

	ResourcesInterface* pResourcesInterface = pSystemResources->GetCurrentResourcesInterface();
	PASSERT_AND_RETURN(!pResourcesInterface);

	eProductType prodType = pSystemResources->GetProductType();

	param.Licensing = pReservator->GetDongleRestriction();
	param.MaxPartiesNum = pResourcesInterface->GetMaxNumberOfParties();

	if (param.MaxPartiesNum > MAX_PARTIES_FOR_RAM_HALF_SIZE && eSystemRamSize_half == pSystemResources->GetRamSize())
		param.MaxPartiesNum = MAX_PARTIES_FOR_RAM_HALF_SIZE;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)(&param), sizeof(param));

	CManagerApi api(eProcessCSMngr);
	STATUS status = api.SendMsg(pSeg, CS_RSRC_CFS_IND);
	TRACEINTO
		<< "Opcode:CS_RSRC_CFS_IND, RESOURCE-->MANAGER"
		<< "\n  MaxPartiesNum :" << param.MaxPartiesNum
		<< "\n  Licensing     :" << param.Licensing
		<< "\n  Status        :" << CProcessBase::GetProcess()->GetStatusAsString(status);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendUdpPortRangeParamsToCS(const UDP_PORT_RANGE_S& param)
{
	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)(&param), sizeof(param));

	CManagerApi api(eProcessCSMngr);
	STATUS status = api.SendMsg(pSeg, CS_RSRC_UDP_PORT_RANGE_IND);
	TRACEINTO
		<< "Opcode:CS_RSRC_UDP_PORT_RANGE_IND, RESOURCE-->MANAGER"
		<< "\n  ServiceId     :" << param.ServiceId
		<< "\n  UdpFirstPort  :" << param.UdpFirstPort
		<< "\n  UdpLastPort   :" << param.UdpLastPort
		<< "\n  Status        :" << CProcessBase::GetProcess()->GetStatusAsString(status);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnIPServiceConfigEnd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:CS_RSRC_IP_SERVICE_PARAM_END_IND";

	m_resourceStartupInfo.IPServicesParamsEndInd();
	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	pSystemResources->InitIpServicesInterfaces();

	if (STATUS_FAIL == pSystemResources->IsCondOk(eIpService))
		UpdateStartupConditionByErrorCode(AA_NO_IP_SERVICE_PARAMS, eStartupConditionFail);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnTimeChangedInd(CSegment* pMsg)
{
	TRACEINTO;

	CSingleToneApi rsrvManager(eProcessResource, "RsrvManager");
	rsrvManager.SendMsg(NULL, MCUMNGR_TO_RSRCALLOC_TIME_CHANGED_IND);
	rsrvManager.CleanUp();
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnDongleRestrictInd(CSegment* pMsg)
{
	RSRCALLOC_LICENSING_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	BOOL isMode2C = params.num_cop_parties ? TRUE : FALSE;

	DWORD num_parties = isMode2C ? params.num_cop_parties : params.num_cp_parties;
	eProductType theProductType = (eProductType)params.productType;

	if (params.isSvcEnabled && eProductTypeSoftMCU == theProductType) //VM of develop environment
	{
		num_parties = MAX_NUMBER_HD_AVC_PARTICIPANTS_SOFT_MCU;
	}

	m_resourceStartupInfo.LicensingInd(num_parties, theProductType, params.federal, params.isHD, params.isRPPModeEnabled);
	if (0 == num_parties)
		PASSERT(1);

	TRACEINTO << "Opcode:MCUMNGR_TO_RSRCALLOC_LICENSING_IND"
		<< "\n  Num_of_parties    :" << num_parties
		<< "\n  Num_cp_parties    :" << params.num_cp_parties
		<< "\n  IsHdPortsUnit     :" << (params.isHdPortsUnit ? "YES" : "NO")
		<< "\n  ProductType       :" << ::ProductTypeToString(theProductType)
		<< "\n  Is1500q_HD_enable :" << (params.isHD ? "YES" : "NO")
		<< "\n  IsFederal         :" << (params.federal ? "YES" : "NO")
		<< "\n  IsSvcEnabled      :" << (params.isSvcEnabled ? "YES" : "NO")
		<< "\n  NumSvcParties     :" << params.num_svc_parties
		<< "\n  IsRPPModeEnabled  :" << (int)params.isRPPModeEnabled;

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator)
	{
		pReservator->SetRMode2C(isMode2C);
		pReservator->SetDongleRestriction(num_parties);
		pReservator->InitProductType(theProductType);
	}

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		pSystemResources->InitProductType(theProductType);
		pSystemResources->SetFederalFlag(params.federal);
		pSystemResources->SetEventMode(isMode2C);
		pSystemResources->SetSvcFlag(params.isSvcEnabled);
		if (!pSystemResources->SetLicenseExpired(params.isLicenseExpired))
		{
			TRACEINTO << "Failed, License is expired shouldn't continue";
			RemoveActiveAlarmByErrorCode(AA_NO_LICENSING);
			return;
		}
		if (pSystemResources->GetSystemCardsMode() != eSystemCardsMode_illegal)
		{
			TRACEINTO << "Warning, MCMS_SYSTEM_CARDS_MODE_IND arrived before the MCUMNGR_TO_RSRCALLOC_LICENSING_IND";
			SetDongleRestriction();

			DWORD cardType = m_resourceStartupInfo.GetCardType(1);
			if (cardType != eEmpty)
			{
				TRACEINTO << "Warning, RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S arrived before the MCUMNGR_TO_RSRCALLOC_LICENSING_IND";
				OnResourceCardTypeInd(0);
			}
		}
	}
	else
		PASSERT(1);

	// vngr-16036 - startup with no cards
	// MCMS_SYSTEM_CARDS_MODE_IND does not received at all, SetDongleRestriction not called and AA_NO_LICENSING not removed
	// results empty "Active Alarms"
	RemoveActiveAlarmByErrorCode(AA_NO_LICENSING);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SetDongleRestriction()
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator && pSystemResources)
	{
		DWORD num_parties = pReservator->GetDongleRestriction();

		//first setting in pSyst, because it might be changed (see function CBaseModeResources::BaseInitDongleRestriction))
		pSystemResources->SetDongleRestriction(num_parties);
		pReservator->SetDongleRestriction(num_parties);
	}
	else
		PASSERT(1);

	RemoveActiveAlarm(AA_NO_LICENSING);

	//send req for IP/Udp
	SendRsrcIpServiceParamsReqToCS();

	//SendCfsParamsToCS(); inserted into OnSystemRamSizeInd() func.
	SendRsrcRamSizeReqToMcuMngr();
	SendLastConfIdReadReqToCDR();

	if (pSystemResources)
	{
		switch (pSystemResources->GetProductType())
		{
			case eProductTypeEdgeAxis:
			case eProductTypeSoftMCUMfw:
				SendRsrcCPUProfileReqToMcuMngr();
				break;
			default:
				break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS SendKillPortRequest(PhysicalPortDesc* pPortDesc, eResourceTypes res_type, BYTE isPortNotResponding = 0)
{
	CTaskApp* pTaskApp = CProcessBase::GetProcess()->GetCurrentTask();
	if (NULL == pTaskApp || strcmp((pTaskApp->GetTaskName()), "Manager") != 0)
	{
		FPASSERT(1); //have to be in context of manager task
		return STATUS_FAIL;
	}

	CResourceManager* pResourceManager = (CResourceManager*)(pTaskApp);
	STATUS status = pResourceManager->KillPortRequest(pPortDesc, res_type);

	CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
	BOOL isEnableRecoveryOnKillPort = YES;
	sysConfig->GetBOOLDataByKey(CFG_KEY_ENABLE_DSP_RECOVERY_ON_KILL_PORT, isEnableRecoveryOnKillPort);

	if (isEnableRecoveryOnKillPort && isPortNotResponding)
		pResourceManager->PortNotRespondingRequest(pPortDesc, res_type);

	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS SendUdpKillPortRequest(PhysicalPortDesc* pPortDesc, CUdpRsrcDesc* pUdpDesc)
{
	CTaskApp* pTaskApp = CProcessBase::GetProcess()->GetCurrentTask();
	if (NULL == pTaskApp || strcmp((pTaskApp->GetTaskName()), "Manager") != 0)
	{
		FPASSERT(1); //have to be in context of manager task
		return STATUS_FAIL;
	}

	CResourceManager* pResourceManager = (CResourceManager*)(pTaskApp);
	STATUS status = pResourceManager->KillUdpPortRequest(pPortDesc, pUdpDesc);
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS SendIsdnUdpKillPortRequest(PhysicalPortDesc* pPortDesc, WORD channelId, eResourceTypes resourceType)
{
	CTaskApp* pTaskApp = CProcessBase::GetProcess()->GetCurrentTask();
	if (NULL == pTaskApp || strcmp((pTaskApp->GetTaskName()), "Manager") != 0)
	{
		FPASSERT(1); //have to be in context of manager task
		return STATUS_FAIL;
	}

	CResourceManager* pResourceManager = (CResourceManager*)(pTaskApp);
	STATUS status = pResourceManager->KillIsdnUdpPortRequest(pPortDesc, channelId, resourceType);
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::SystemResourcesEmptyRequest()
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	for (int i = 0; i < BOARDS_NUM; ++i)
	{
		CBoard* pBoard = pSystemResources->GetBoard(i+1); // board num is 1-based
		if (pBoard && (CHelperFuncs::IsBreeze(pBoard->GetCardType()) || CHelperFuncs::IsMpmRxOrNinja(pBoard->GetCardType())))
		{
			CMplMcmsProtocol mplMcmsProtocol;
			mplMcmsProtocol.AddCommonHeader(CM_SYSTEM_RESOURCES_EMPTY);
			mplMcmsProtocol.AddMessageDescriptionHeader();
			mplMcmsProtocol.AddPhysicalHeader(1/*box_id*/, i+1/*board_id*/, 1/*sub_board_id*/, 0, 0, 0, 0);
			mplMcmsProtocol.AddPortDescriptionHeader(DUMMY_PARTY_ID, DUMMY_CONF_ID, DUMMY_CONNECTION_ID);
			CMplMcmsProtocolTracer(mplMcmsProtocol).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");

			mplMcmsProtocol.SendMsgToMplApiCommandDispatcher();
		}
	}
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::KillPortRequest(PhysicalPortDesc* pPortDesc, eResourceTypes resourceType)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.AddCommonHeader(KILL_PORT_REQ);
	mplMcmsProtocol.AddMessageDescriptionHeader();
	mplMcmsProtocol.AddPhysicalHeader(
		pPortDesc->m_boxId,
		pPortDesc->m_boardId,
		pPortDesc->m_subBoardId,
		pPortDesc->m_unitId,
		pPortDesc->m_portId,
		pPortDesc->m_acceleratorId,
		resourceType);

	mplMcmsProtocol.AddPortDescriptionHeader(DUMMY_PARTY_ID, DUMMY_CONF_ID, DUMMY_CONNECTION_ID);
	CMplMcmsProtocolTracer(mplMcmsProtocol).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");

	mplMcmsProtocol.SendMsgToMplApiCommandDispatcher();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::PortNotRespondingRequest(PhysicalPortDesc* pPortDesc, eResourceTypes resourceType)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.AddCommonHeader(PORT_NOT_RESPONDING_REQ);
	mplMcmsProtocol.AddMessageDescriptionHeader();
	mplMcmsProtocol.AddPhysicalHeader(
		pPortDesc->m_boxId,
		pPortDesc->m_boardId,
		pPortDesc->m_subBoardId,
		pPortDesc->m_unitId,
		pPortDesc->m_portId,
		pPortDesc->m_acceleratorId,
		resourceType);

	mplMcmsProtocol.AddPortDescriptionHeader(DUMMY_PARTY_ID, DUMMY_CONF_ID, DUMMY_CONNECTION_ID);
	CMplMcmsProtocolTracer(mplMcmsProtocol).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");

	mplMcmsProtocol.SendMsgToMplApiCommandDispatcher();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::KillUdpPortRequest(PhysicalPortDesc* pPortDesc, CUdpRsrcDesc* pUdpDesc)
{
	TKillUdpPortMessageStruct* pStruct = new TKillUdpPortMessageStruct;

	mcKillUdpPortRequestStruct* pUdpReq = &pStruct->tCmKillUdpPort;

	memset(pUdpReq, 0xFFFFFFFF, sizeof(mcKillUdpPortRequestStruct));
	memset(&(pStruct->physicalPort.physical_id), 0, sizeof(PHYSICAL_RESOURCE_INFO_S));

	for (int i = 0; i < MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS; i++)
	{
		pUdpReq->udp_ports[i] = 0;
		pUdpReq->rtcp_ports[i] = 0;
	}

	TRACEINTO << "pUdpDesc->m_rtcp_channels[3] = " << pUdpDesc->m_rtcp_channels[3];
	//Checking only the first port (of audio)
	if (pUdpDesc->m_rtcp_channels[0])
	{
		pUdpReq->ulIsIceConn = TRUE; //should be enum or which val???
		for (int index = 0; index < MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS; index++)
		{
			if (pUdpDesc->m_rtp_channels[index])
				pUdpReq->udp_ports[index] = pUdpDesc->m_rtp_channels[index];
			if (pUdpDesc->m_rtcp_channels[index])
				pUdpReq->rtcp_ports[index] = pUdpDesc->m_rtcp_channels[index];
		}
	}
	else
	{
		pUdpReq->ulIsIceConn = FALSE;

		if (pUdpDesc->m_udp.AudioChannelPort)
		{
			pUdpReq->udp_ports[0] = pUdpDesc->m_udp.AudioChannelPort;
			pUdpReq->rtcp_ports[0] = pUdpDesc->m_udp.AudioChannelPort + 1;
		}
		if (pUdpDesc->m_udp.VideoChannelPort)
		{
			pUdpReq->udp_ports[1] = pUdpDesc->m_udp.VideoChannelPort;
			pUdpReq->rtcp_ports[1] = pUdpDesc->m_udp.VideoChannelPort + 1;
		}
		if (pUdpDesc->m_udp.FeccChannelPort)
		{
			pUdpReq->udp_ports[2] = pUdpDesc->m_udp.FeccChannelPort;
			pUdpReq->rtcp_ports[2] = pUdpDesc->m_udp.FeccChannelPort + 1;
		}
		if (pUdpDesc->m_udp.ContentChannelPort)
		{
			pUdpReq->udp_ports[3] = pUdpDesc->m_udp.ContentChannelPort;
			pUdpReq->rtcp_ports[3] = pUdpDesc->m_udp.ContentChannelPort + 1;
		}
		if (pUdpDesc->m_udp.BfcpChannelPort)
		{
			pUdpReq->udp_ports[4] = pUdpDesc->m_udp.BfcpChannelPort;
			pUdpReq->rtcp_ports[4] = pUdpDesc->m_udp.BfcpChannelPort + 1;
		}
	}
	std::ostringstream ostr;
	for (int index = 0; index < MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS; index++)
	{
		ostr << "\n pUdpReq->udp_ports[" << index << "] = " << pUdpReq->udp_ports[index] << "   pUdpReq->rtcp_ports[" << index << "] = " << pUdpReq->rtcp_ports[index];

	}
	TRACEINTO << ostr.str().c_str();

	const BYTE box_id = 1; //(BYTE)(pPortDesc->m_boxId);
	const BYTE board_id = (BYTE)(pPortDesc->m_boardId);
	const BYTE sub_board_id = 1; //(BYTE)(pPortDesc->m_subBoardId);
	const BYTE unit_id = (BYTE)(pPortDesc->m_unitId);
	const BYTE accelerator_id = (BYTE)(pPortDesc->m_acceleratorId);
	const WORD port_id = pPortDesc->m_portId;
	const BYTE resource_type = (BYTE)ePhysical_art; // for a while(need to distinguish between art and art light)

	pStruct->physicalPort.connection_id = DUMMY_CONNECTION_ID; //???
	pStruct->physicalPort.party_id = DUMMY_PARTY_ID; //???
	pStruct->physicalPort.physical_id.physical_unit_params.box_id = box_id;
	pStruct->physicalPort.physical_id.physical_unit_params.board_id = board_id;
	pStruct->physicalPort.physical_id.physical_unit_params.sub_board_id = sub_board_id;
	pStruct->physicalPort.physical_id.physical_unit_params.unit_id = unit_id;
	pStruct->physicalPort.physical_id.accelerator_id = accelerator_id;
	pStruct->physicalPort.physical_id.port_id = port_id;
	pStruct->physicalPort.physical_id.resource_type = resource_type;

	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.AddCommonHeader(KILL_UDP_PORT_REQ);
	mplMcmsProtocol.AddMessageDescriptionHeader();
	mplMcmsProtocol.AddPhysicalHeader(box_id, board_id, sub_board_id); // SM according to MFA request
	mplMcmsProtocol.AddPortDescriptionHeader(DUMMY_PARTY_ID, DUMMY_CONF_ID, DUMMY_CONNECTION_ID);
	mplMcmsProtocol.AddData(sizeof(TKillUdpPortMessageStruct), (char*)pStruct);
	CMplMcmsProtocolTracer(mplMcmsProtocol).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");

	mplMcmsProtocol.SendMsgToMplApiCommandDispatcher();

	POBJDELETE(pStruct);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::KillIsdnUdpPortRequest(PhysicalPortDesc* pPortDesc, WORD channelId, eResourceTypes resourceType)
{
	TKillUdpPortMessageStruct* pStruct = new TKillUdpPortMessageStruct;

	mcKillUdpPortRequestStruct* pUdpReq = &pStruct->tCmKillUdpPort;

	memset(pUdpReq, 0xFFFFFFFF, sizeof(mcKillUdpPortRequestStruct));
	memset(&(pStruct->physicalPort.physical_id), 0, sizeof(PHYSICAL_RESOURCE_INFO_S));

	pUdpReq->udp_ports[0] = channelId;

	const BYTE box_id = 1; //(BYTE)(pPortDesc->m_boxId);
	const BYTE board_id = (BYTE)(pPortDesc->m_boardId);
	const BYTE sub_board_id = (BYTE)(pPortDesc->m_subBoardId);
	const BYTE unit_id = (BYTE)(pPortDesc->m_unitId);
	const BYTE accelerator_id = (BYTE)(pPortDesc->m_acceleratorId);
	const WORD port_id = pPortDesc->m_portId;
	const BYTE resource_type = (BYTE)resourceType;

	pStruct->physicalPort.connection_id = DUMMY_CONNECTION_ID;
	pStruct->physicalPort.party_id = DUMMY_PARTY_ID;
	pStruct->physicalPort.physical_id.physical_unit_params.box_id = box_id;
	pStruct->physicalPort.physical_id.physical_unit_params.board_id = board_id;
	pStruct->physicalPort.physical_id.physical_unit_params.sub_board_id = sub_board_id;
	pStruct->physicalPort.physical_id.physical_unit_params.unit_id = unit_id;
	pStruct->physicalPort.physical_id.accelerator_id = accelerator_id;
	pStruct->physicalPort.physical_id.port_id = port_id;
	pStruct->physicalPort.physical_id.resource_type = resource_type;

	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.AddCommonHeader(KILL_UDP_PORT_REQ);
	mplMcmsProtocol.AddMessageDescriptionHeader();
	mplMcmsProtocol.AddPhysicalHeader(box_id, board_id, sub_board_id); // SM according to MFA request
	mplMcmsProtocol.AddPortDescriptionHeader(DUMMY_PARTY_ID, DUMMY_CONF_ID, DUMMY_CONNECTION_ID);
	mplMcmsProtocol.AddData(sizeof(TKillUdpPortMessageStruct), (char*)pStruct);
	CMplMcmsProtocolTracer(mplMcmsProtocol).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");

	mplMcmsProtocol.SendMsgToMplApiCommandDispatcher();

	POBJDELETE(pStruct);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnMplApiInd(CSegment* pMsg)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pMsg);

	CMplMcmsProtocolTracer(mplMcmsProtocol).TraceMplMcmsProtocol("RESOURCE_RECEIVED_FROM_MPL");

	OPCODE opcode = mplMcmsProtocol.getOpcode(); // extract the internal opcode...

	pMsg->ResetRead();
	DispatchEvent(opcode, pMsg);                //  ... and send it to the stateMachine
	PushMessageToQueue(opcode, pMsg->GetLen(), eProcessMplApi);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSpreadingInd(CSegment* pMsg)
{
	PTRACE(eLevelInfoHigh, "CResourceManager::OnSpreadingInd");

	// ===== 1. fill CMcuMngrManager's attribute with data from segment received
	CMplMcmsProtocol* pMplMcmsProtocol = new CMplMcmsProtocol;
	pMplMcmsProtocol->DeSerialize(*pMsg);

	STATUS status = pMplMcmsProtocol->ValidateDataSize(sizeof(ACK_IND_S));
	if (STATUS_OK != status)
		return;

	CSegment* pSeg = new CSegment;
	if (pMplMcmsProtocol->getDataLen())
	{
		pSeg->Put((unsigned char*)pMplMcmsProtocol->GetData(), pMplMcmsProtocol->getDataLen());
	}

	ACK_IND_S *rAckStruct = (ACK_IND_S *)(pSeg->GetPtr());

	DWORD opcode_s = rAckStruct->ack_base.ack_opcode;                // = pMplMcmsProtocol->getCommonHeaderOpcode();

	POBJDELETE(pSeg);
	POBJDELETE(pMplMcmsProtocol);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnStartPartyMoveReq(CSegment* pMsg)
{
	PARTY_MOVE_RSRC_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:START_PARTY_MOVE_RSRC_REQ" << params;

	PartyRsrcID partyId = 0xFFFFFFFF;
	ConfRsrcID target_rsrc_conf_id = 0xFFFFFFFF;

	STATUS status = m_pRsrcAlloc->StartPartyMove(&params, partyId, target_rsrc_conf_id);

	PARTY_MOVE_RSRC_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	result.status                 = status;
	result.monitor_party_id       = params.source_monitor_party_id;
	result.rsrc_party_id          = partyId;
	result.target_monitor_conf_id = params.target_monitor_conf_id;
	result.target_rsrc_conf_id    = target_rsrc_conf_id;

	TRACEINTO << result;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));
	ResponedClientRequest(START_PARTY_MOVE_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnEndPartyMoveReq(CSegment* pMsg)
{
	PARTY_MOVE_RSRC_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:END_PARTY_MOVE_RSRC_REQ" << params;

	PartyRsrcID rsrc_partyId = 0xFFFFFFFF;
	ConfMonitorID target_monitor_conf_id = 0xFFFFFFFF;
	ConfRsrcID target_rsrc_conf_id = 0xFFFFFFFF;
	STATUS status;

	PARTY_MOVE_RSRC_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	eSessionType sessionType = esession_type_none;
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	if (pConfRsrcDB)
	{
		CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrc(params.target_monitor_conf_id);
		if (pConfRsrc)
			sessionType = pConfRsrc->GetSessionType();
	}

	if (CHelperFuncs::IsMode2C() && (CHelperFuncs::IsSessionTypeCOP(sessionType) || eVSW_Auto_session == sessionType))
	{
		status = m_pRsrcAlloc->EndPartyMove2C(&params, &result);
	}
	else
	{
		status = m_pRsrcAlloc->EndPartyMove(&params, rsrc_partyId, target_rsrc_conf_id);

		result.monitor_party_id       = params.target_monitor_party_id;
		result.rsrc_party_id          = rsrc_partyId;
		result.target_monitor_conf_id = params.target_monitor_conf_id;
		result.target_rsrc_conf_id    = target_rsrc_conf_id;
	}
	result.status = status;

	TRACEINTO << result;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));
	ResponedClientRequest(END_PARTY_MOVE_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
STATUS CreateControllerRecord(WORD boardId, WORD unitId, DWORD connId, eResourceTypes r_type, ECntrlType rsrcCntlType)
{
	CTaskApp* pTaskApp = CProcessBase::GetProcess()->GetCurrentTask();
	if (NULL == pTaskApp || strcmp((pTaskApp->GetTaskName()), "Manager") != 0)
	{
		FPASSERT(1); //trying to get table from other task
		return STATUS_FAIL;
	}

	return ((CResourceManager*)pTaskApp)->CreateController(boardId, unitId, connId, r_type, rsrcCntlType);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::CreateController(BoardID boardId, UnitID unitId, ConnectionID connId, eResourceTypes resourceType, ECntrlType controllerType)
{
	PASSERT_AND_RETURN_VALUE(!m_pConnToCardManager, STATUS_FAIL);
	PASSERT_AND_RETURN_VALUE(!m_pRsrcAlloc, STATUS_FAIL);

	ConnToCardTableEntry Entry;
	Entry.m_id             = connId;
	Entry.boardId          = boardId;
	Entry.unitId           = unitId;
	Entry.boxId            = 1;
	Entry.subBoardId       = 1;
	Entry.portId           = 0xFFFF; //??? may be 1, and parties on ART start from 2
	Entry.rsrcCntlType     = controllerType;
	Entry.physicalRsrcType = resourceType;

	if (ePhysical_audio_controller == resourceType)
	{
		Entry.rsrcType = eLogical_audio_controller;
	}
	else if (ePhysical_ivr_controller == resourceType)
	{
		Entry.rsrcType = eLogical_ivr_controller;
	}
	else
	{
		PASSERT_AND_RETURN_VALUE(resourceType, STATUS_FAIL);
	}

	STATUS status = m_pConnToCardManager->Add(Entry);
	if (status != STATUS_OK)
	{
		if (ePhysical_audio_controller == resourceType)
			PASSERTMSG(status, "Audio Controller enable failed");
		else
			PASSERTMSG(status, "IVR Controller enable failed");
	}

	return status;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::RTMConfigInit()
{
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	PASSERT_AND_RETURN(!pSysConfig);

	CNetServicesDB* pNetServicesDB = CHelperFuncs::GetNetServicesDB();
	PASSERT_AND_RETURN(!pNetServicesDB);

	eSpanAllocation spanAllocationOrder = eSpanAllocation_LoadBalancing;
	string spanAllocationOrderString = "";
	BOOL isSpanAllocationOrderConfigured = pSysConfig->GetDataByKey(CFG_KEY_ISDN_RESOURCE_POLICY, spanAllocationOrderString);
	if (isSpanAllocationOrderConfigured)
	{
		if (spanAllocationOrderString.compare("LOAD_BALANCE") == 0)
		{
			spanAllocationOrder = eSpanAllocation_LoadBalancing;
		}
		else if (spanAllocationOrderString.compare("FILL_FROM_FIRST_CONFIGURED_SPAN") == 0)
		{
			spanAllocationOrder = eSpanAllocation_FillFromStart;
		}
		else if (spanAllocationOrderString.compare("FILL_FROM_LAST_CONFIGURED_SPAN") == 0)
		{
			spanAllocationOrder = eSpanAllocation_FillFromEnd;
		}
		else
		{
			PASSERTSTREAM(1, "SpanAllocationOrder:" << spanAllocationOrderString << " - Invalid value in system.cfg");
		}
	}
	TRACESTRFUNC(eLevelInfoHigh) << "SpanAllocationMode:" << spanAllocationOrder << ", IsSpanAllocationOrderConfigured:" << (int)isSpanAllocationOrderConfigured;

	pNetServicesDB->SetSpanAllocationOrder(spanAllocationOrder);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendMeetingRoomsAndProfileDBreadReqToConfMngr()
{
	TRACEINTO << "Opcode:STARTUP_READ_MR_AND_PROFILE_DB_REQ, RESOURCE-->MANAGER";
	CSegment* pSeg = new CSegment;
	CManagerApi api(eProcessConfParty);
	api.SendMsg(pSeg, STARTUP_READ_MR_AND_PROFILE_DB_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendModeReqToCardMngr()
{
	TRACEINTO << "Opcode:RESOURCE_SYSTEM_CARDS_MODE_REQ, RESOURCE-->MANAGER";
	CSegment* pSeg = new CSegment;
	CManagerApi api(eProcessCards);
	api.SendMsg(pSeg, RESOURCE_SYSTEM_CARDS_MODE_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnProfileUpdate(CSegment* pMsg)
{
	PROFILE_IND_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:PROFILE_UPDATE_RSRC_IND" << params;

	CProfilesDB* pProfilesDB = CHelperFuncs::GetProfilesDB();
	PASSERT_AND_RETURN(!pProfilesDB);

	STATUS status = pProfilesDB->UpdateProfile(params.profile_Id, params.maxVideoPartyType);
	PASSERT_AND_RETURN(status);

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN(!pReservator);

	pReservator->ProfileChangedInd(params.profile_Id);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnProfileAdd(CSegment* pMsg)
{
	PROFILE_IND_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:PROFILE_ADD_RSRC_IND" << params;

	CProfilesDB* pProfilesDB = CHelperFuncs::GetProfilesDB();
	PASSERT_AND_RETURN(!pProfilesDB);

	pProfilesDB->AddProfile(params.profile_Id, params.maxVideoPartyType);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnProfileDelete(CSegment* pMsg)
{
	PROFILE_IND_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:PROFILE_DELETE_RSRC_IND" << params;

	CProfilesDB* pProfilesDB = CHelperFuncs::GetProfilesDB();
	PASSERT_AND_RETURN(!pProfilesDB);

	pProfilesDB->RemoveProfile(params.profile_Id);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnStartupMRAndProfileRead(CSegment* pMsg)
{
	// indication return to ConfMngr + status
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN(!pReservator);

	STATUS status = STATUS_FAIL;

	//////////////////////////////////////////////////////////////////////////
	//Meeting rooms treatment
	///////////////////////////////////////////////////////////////////////////

	//for return to ConfMngr
	CSegment* pSeg = new CSegment;
	MR_IND_LIST_S* pRetList = new MR_IND_LIST_S;

	// reading MR DB size
	MR_AND_PROFILE_IND_LISTS* pMRAndProfileList = new MR_AND_PROFILE_IND_LISTS;
	pMsg->Get((BYTE*)(&(pMRAndProfileList->mr_list.list_size)), sizeof(DWORD));

	if (pMRAndProfileList->mr_list.list_size != 0) //MR DB not empty
	{
		size_t size = pMRAndProfileList->mr_list.list_size;
		pMRAndProfileList->mr_list.monitor_numeric_list = new MR_MONITOR_NUMERIC_ID_S[size];

		CManagerApi api(eProcessConfParty);
		CSegment response;

		for (size_t i = 0; i < size; ++i)
		{
			CSegment* pRequest = new CSegment;
			*pRequest << i;

			OPCODE retOp;
			status = api.SendMessageSync(pRequest, SPECIFIC_MEETING_ROOM_AT_STARTUP_REQ, 10 * SECOND, retOp, response);
			if (status == STATUS_OK && retOp == STATUS_OK)
			{
				response.Get((BYTE*)(&(pMRAndProfileList->mr_list.monitor_numeric_list[i])), sizeof(MR_MONITOR_NUMERIC_ID_S));
			}
			else
			{
				PASSERT(100 + i);
			}
		}

		TRACEINTO << "Opcode:STARTUP_READ_MR_AND_PROFILE_DB_IND" << pMRAndProfileList->mr_list;

		pRetList->list_size = size;
		pRetList->mr_list = new MR_IND_S[size];

		// Allocating MRs numeric and monitoring conference ids
		for (size_t i = 0; i < size; ++i)
		{
			ConfMonitorID monitor_Id = (pMRAndProfileList->mr_list.monitor_numeric_list[i]).meeting_room_monitor_Id;
			char* numeric_Id         = (pMRAndProfileList->mr_list.monitor_numeric_list[i]).numeric_Id;
			eRsrcConfType conf_type  = (pMRAndProfileList->mr_list.monitor_numeric_list[i]).conf_type;

			if (eConf_type_none == conf_type)
				PASSERT(1); //couldn't occur
			if (numeric_Id == NULL || monitor_Id == 0xFFFFFFFF)
			{
				PTRACE(eLevelError, "CResourceManager::OnStartupMRAndProfileRead() invalid MR DB params");
				continue;
			}

			status = (pReservator->AllocateMntrConfId(monitor_Id)) ? STATUS_OK : STATUS_INSUFFICIENT_MNTR_CONF_ID;

			if (status == STATUS_OK)
			{
				status = pReservator->AllocateNumericConfId(numeric_Id, TRUE);
				if (status != STATUS_OK)
					pReservator->DeAllocateMntrConfId(monitor_Id);
			}

			if (status == STATUS_OK)
			{
				pReservator->AddSleepingConference(numeric_Id, monitor_Id, conf_type);
				pReservator->PlusNumConfPerType(conf_type);
			}
			else
			{
				PTRACE2INT(eLevelInfoNormal, "CResourceManager::OnStartupMRAndProfileRead() status - ", status);
				PASSERT(status);  //+trace?
			}

			//*** PSTN phones allocation
			if (status == STATUS_OK && (eEQ_type == conf_type || eMR_type == conf_type))
			{
				CSleepingConference* pConf = const_cast<CSleepingConference*>(pReservator->GetSleepingConference(monitor_Id));
				if (pConf)
				{
					if (((pMRAndProfileList->mr_list.monitor_numeric_list[i]).serviceName[0]) != '\0')  //EQ has no isdn service phones - ok.
					{
						CServicePhoneStr* pServicePhoneStr = new CServicePhoneStr();

						pServicePhoneStr->SetNetServiceName((pMRAndProfileList->mr_list.monitor_numeric_list[i]).serviceName);

						if ((pMRAndProfileList->mr_list.monitor_numeric_list[i]).firstPhoneNumber[0] != '\0')
							pServicePhoneStr->AddPhoneNumber((pMRAndProfileList->mr_list.monitor_numeric_list[i]).firstPhoneNumber);

						if ((pMRAndProfileList->mr_list.monitor_numeric_list[i]).secondPhoneNumber[0] != '\0')
							pServicePhoneStr->AddPhoneNumber((pMRAndProfileList->mr_list.monitor_numeric_list[i]).secondPhoneNumber);

						status = pSystemResources->AllocateServicePhones(*pServicePhoneStr);

						if (status == STATUS_OK)
							pConf->AddServicePhone(*pServicePhoneStr);

						POBJDELETE(pServicePhoneStr);
					}
				}

				//(**) rollback monitor id and numeric_Id
				if (status != STATUS_OK)
				{
					pReservator->DeleteResAllTypes(monitor_Id, &conf_type);

					//monitor_Id = 0xFFFFFFFF;
					PTRACE2INT(eLevelInfoNormal, "CResourceManager::OnStartupMRAndProfileRead, status = ", status);
				}
				//(**) rollback monitor id and numeric_Id END
			}
			// *** PSTN phones allocation

			//parameters to return to ConfMngr
			(pRetList->mr_list[i]).meeting_room_monitor_Id = monitor_Id;
			(pRetList->mr_list[i]).status = status;
		}

		TRACEINTO << *pRetList;

		pSeg->Put((BYTE*)pRetList, sizeof(DWORD));
		pSeg->Put((BYTE*)(pRetList->mr_list), (sizeof(MR_IND_S) * size));

		delete[] pRetList->mr_list;
		delete[] pMRAndProfileList->mr_list.monitor_numeric_list;
	}
	else //mo MR in DB, nothing has to be done
	{
		TRACEINTO << "Opcode:STARTUP_READ_MR_AND_PROFILE_DB_IND" << pMRAndProfileList->mr_list;

		pRetList->list_size = 0;
		pSeg->Put((BYTE*)pRetList, sizeof(DWORD));
	}

	//////////////////////////////////////////////////////////////////////////
	//Profiles treatment
	///////////////////////////////////////////////////////////////////////////
	pMsg->Get((BYTE*)(&(pMRAndProfileList->profile_list.list_size)), sizeof(DWORD));
	if (pMRAndProfileList->profile_list.list_size != 0) //MR DB not empty
	{
		size_t size = pMRAndProfileList->profile_list.list_size;
		pMRAndProfileList->profile_list.profile_list = new PROFILE_IND_S[size];

		// reading profile DB
		pMsg->Get((BYTE*)(pMRAndProfileList->profile_list.profile_list), sizeof(PROFILE_IND_S) * size);

		TRACEINTO << "Opcode:STARTUP_READ_MR_AND_PROFILE_DB_IND" << pMRAndProfileList->profile_list;

		CProfilesDB* pProfilesDB = CHelperFuncs::GetProfilesDB();
		if (pProfilesDB != NULL)
			pProfilesDB->InitProfileList(&(pMRAndProfileList->profile_list));

		delete[] pMRAndProfileList->profile_list.profile_list;
	}
	else //mo profile in DB, nothing has to be done
	{
		TRACEINTO << "Opcode:STARTUP_READ_MR_AND_PROFILE_DB_IND" << pMRAndProfileList->profile_list;
	}

	//now that we finished this process, also read the DB of the reservations
	//(this has to be done after reading the MRs because of possible collisions)
	CSingleToneApi rsrvManager(eProcessResource, "RsrvManager");
	rsrvManager.SendMsg(NULL, RESOURCE_READ_RES_DB);
	rsrvManager.CleanUp();

	delete pMRAndProfileList;
	delete pRetList;

	RemoveActiveAlarmFaultOnlyByErrorCode(NO_MEETING_ROOM);

	CManagerApi api(eProcessConfParty);
	api.SendMsg(pSeg, STARTUP_INIT_MR_DB_IND);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendLastConfIdReadReqToCDR()
{
	TRACEINTO << "Opcode:CDR_RSRC_GET_LAST_CONF_ID_REQ, RESOURCE-->MANAGER";

	CSegment* pSeg = new CSegment;
	CManagerApi api(eProcessCDR);
	api.SendMsg(pSeg, CDR_RSRC_GET_LAST_CONF_ID_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnLastConfIdInd(CSegment* pMsg)
{
	LAST_CONF_ID_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:CDR_RSRC_SET_LAST_CONF_ID_IND" << "LastConfId:" << params.last_conf_id;

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN(!pReservator);

	pReservator->SetStartConfId(params.last_conf_id);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSysMntrStopAllMediaRecordingReq(CSegment* pMsg)
{
	TRACEINTO << "Opcode:SYSMNTR_TO_RSRC_STOP_ALL_MEDIA_RECORDING_REQ";
	STATUS status = SendStopRecordingToCm();
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::AddDSPInfoToFileName(APIS8 ucFileName[MAX_RECORDING_FILE_NAME_LENGTH], APIU8 board_id, APIU8 unit_id)
{
	std::string newFileName = std::string(ucFileName);
	size_t pos = newFileName.find_last_of(".");
	if (pos != std::string::npos)
	{
		//Save file extension
		char fileExt[4] = { };

		for (size_t i = 0; i < 3; ++i)
		{
			fileExt[i] = newFileName.at(pos + i + 1);
		}

		std::ostringstream board_unit;
		board_unit << "_b" << (DWORD)board_id << "_u" << (DWORD)unit_id << '.' << fileExt;

		newFileName.replace(pos, (board_unit.str()).size(), (board_unit.str()));

		strncpy(ucFileName, newFileName.c_str(), MAX_RECORDING_FILE_NAME_LENGTH - 1);
		ucFileName[MAX_RECORDING_FILE_NAME_LENGTH - 1] = '\0';
	}
}

////////////////////////////////////////////////////////////////////////////
BOOL CResourceManager::IsFederalOn()
{
	BOOL bJitcMode = FALSE;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_JITC_MODE, bJitcMode);
	return bJitcMode;
}

////////////////////////////////////////////////////////////////////////////
eResourceTypes JunctionId2PhysicalType(WORD id)
{
	if (id < E_CM_JUNCTION_LAN_INCOMING_AUDIO || id > E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL30)
		return ePhysical_res_none;

	else if (id >= E_CM_JUNCTION_LAN_INCOMING_AUDIO && id <= E_CM_JUNCTION_OUTGOING_MRMP_AUDIO)
		return ePhysical_art; // in later versions: ePhysical_art_light;

	else if ((id >= E_CM_JUNCTION_LAN_INCOMING_VIDEO && id <= E_CM_JUNCTION_MRMP_OUTGOING_VIDEO) || (id >= E_CM_JUNCTION_OUTGOING_ISDN_CHANNEL01 && id <= E_CM_JUNCTION_ISDN_TO_FABRIC_CHANNEL30) /* MUX */)
		return ePhysical_art;

	else if (id >= E_CM_JUNCTION_INCOMING_COMPRESSED_VIDEO && id <= E_CM_JUNCTION_VIDEO_DECODER_OUTGOING_INTERNAL_RECORDING2)
		return ePhysical_video_decoder;

	else if (id >= E_CM_JUNCTION_INCOMING_UNCOMPRESSED_VIDEO_1 && id <= E_CM_JUNCTION_VIDEO_ENCODER_OUTGOING_INTERNAL_RECORDING2)
		return ePhysical_video_encoder;

	return NUM_OF_RESOURCE_TYPES;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::SendStartRecordingToCm(ConfMonitorID monitorConfId, PartyMonitorID monitorPartyId, DWORD sizeLimit, CJunctionParam& junc)
{
	WORD bId =0, uId = 0, acceleratorId = 0, portId = 0, length = 0;

	TStartDebugRecordingReq* pRecord = new TStartDebugRecordingReq;

	pRecord->tDebugRecordingJunction.unJunction      = junc.GetId();
	pRecord->tDebugRecordingJunction.unRate          = junc.GetRate();
	pRecord->tDebugRecordingJunction.unMaxRecFileLen = sizeLimit;

	if (junc.GetFileName())
	{
		// this is exactly what strncpy will check anyway
		//length = ( strlen(junc.GetFileName()) < MAX_RECORDING_FILE_NAME_LENGTH-1 ) ? strlen(junc.GetFileName()) : MAX_RECORDING_FILE_NAME_LENGTH-1;

		strncpy( pRecord->tDebugRecordingJunction.ucFileName , junc.GetFileName(), sizeof(pRecord->tDebugRecordingJunction.ucFileName) - 1);
		pRecord->tDebugRecordingJunction.ucFileName[sizeof(pRecord->tDebugRecordingJunction.ucFileName) - 1] = '\0';
	}
	else
	{
		PASSERT(1);
		pRecord->tDebugRecordingJunction.ucFileName[0] = '\0';
	}

	STATUS status = STATUS_NOT_FOUND;
	eResourceTypes  physicalRsrcType = JunctionId2PhysicalType(junc.GetId());

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (!pSystemResources || !pConfRsrcDB)
	{
		POBJDELETE(pRecord);
		PASSERT_AND_RETURN_VALUE (true, STATUS_FAIL);
	}

	// monitor Conf / Party ids are received from EMA, but rsrc conf / party ids should be sent to card.
	ConfRsrcID confId  = pConfRsrcDB->MonitorToRsrcConfId(monitorConfId);
	PartyRsrcID partyId = pConfRsrcDB->MonitorToRsrcPartyId(monitorConfId, monitorPartyId);

	if (confId == 0 || partyId == 0)
	{
		POBJDELETE(pRecord);
		PASSERT_AND_RETURN_VALUE (true, STATUS_FAIL);
	}

	pRecord->tDebugRecordingJunction.unStreamId = partyId; //need to get the PartyId when dealing with these MRMP junctions
	CConfRsrc* pConfRsrc = pConfRsrcDB ? pConfRsrcDB->GetConfRsrc(monitorConfId) : NULL;
	const CPartyRsrc* pParty = pConfRsrc ? pConfRsrc->GetParty(monitorPartyId) : NULL;
	bool isPartyAVC = pParty ? (!CHelperFuncs::IsVideoRelayParty(pParty->GetVideoPartyType())) : false;

	// When user ask to record a junction for AVC participant in mixed call, we should send
	// the request to EMB 3 times: one for each ART with the relevant ART number.
	if ((CHelperFuncs::GetConferenceMode(monitorConfId) == eMix) && (ePhysical_art==physicalRsrcType) && isPartyAVC)
	{
		CRsrcDesc** pRsrcDescArray = new CRsrcDesc*[MAX_NUM_ALLOCATED_RSRCS];
		WORD num_party_desc = pConfRsrc->GetPartyRTPDescArrayByRsrcId(partyId, pRsrcDescArray, MAX_NUM_ALLOCATED_RSRCS);
		for (WORD desc_index=0; (desc_index<num_party_desc) && (desc_index<MAX_NUM_ALLOCATED_RSRCS); desc_index++)
		{
			if (pRsrcDescArray[desc_index] && physicalRsrcType == pRsrcDescArray[desc_index]->GetPhysicalType())
			{
				pRecord->physicalPort.physical_unit_params.box_id       = pRsrcDescArray[desc_index]->GetBoxId();
				pRecord->physicalPort.physical_unit_params.board_id     = pRsrcDescArray[desc_index]->GetBoardId();
				pRecord->physicalPort.physical_unit_params.sub_board_id = pRsrcDescArray[desc_index]->GetSubBoardId();
				pRecord->physicalPort.physical_unit_params.unit_id      = pRsrcDescArray[desc_index]->GetUnitId();
				pRecord->physicalPort.accelerator_id                    = pRsrcDescArray[desc_index]->GetAcceleratorId();
				pRecord->physicalPort.port_id                           = pRsrcDescArray[desc_index]->GetFirstPortId();
				pRecord->physicalPort.resource_type                     = physicalRsrcType;

				AddDSPInfoToFileName(pRecord->tDebugRecordingJunction.ucFileName,
									 pRecord->physicalPort.physical_unit_params.board_id,
									 pRecord->physicalPort.physical_unit_params.unit_id);

				SendRecordingToCm(pRecord, confId, partyId);
				status = STATUS_OK;

				strncpy( pRecord->tDebugRecordingJunction.ucFileName , junc.GetFileName(), sizeof(pRecord->tDebugRecordingJunction.ucFileName) - 1);
				pRecord->tDebugRecordingJunction.ucFileName[sizeof(pRecord->tDebugRecordingJunction.ucFileName) - 1] = '\0';
			}
		}
		delete [] pRsrcDescArray;
	}
	else
	{
		status = pSystemResources->PartyParamsToPhysicalLocation(confId, partyId, physicalRsrcType, bId, uId, acceleratorId, portId);

		if (status == STATUS_OK)
		{
			//fills in all needed params
			pRecord->physicalPort.physical_unit_params.box_id       = 1;
			pRecord->physicalPort.physical_unit_params.board_id     = bId;
			pRecord->physicalPort.physical_unit_params.sub_board_id = 1;
			pRecord->physicalPort.physical_unit_params.unit_id      = uId;
			pRecord->physicalPort.accelerator_id                    = acceleratorId;
			pRecord->physicalPort.port_id                           = portId;
			pRecord->physicalPort.resource_type                     = physicalRsrcType;

			AddDSPInfoToFileName(pRecord->tDebugRecordingJunction.ucFileName,
								 pRecord->physicalPort.physical_unit_params.board_id,
								 pRecord->physicalPort.physical_unit_params.unit_id);

			SendRecordingToCm(pRecord, confId, partyId);
		}
	}

	POBJDELETE(pRecord);

	//set & send to McuMngr monitoring ind recording ON
	if (status == STATUS_OK)
	{
		if (pSystemResources->GetIsSystemRecording() == FALSE)
			SendSystemRecordingToMcuMngr(TRUE);

		pSystemResources->SetIsSystemRecording(TRUE);
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendRecordingToCm(TStartDebugRecordingReq* pRecord, ConfRsrcID confId, PartyRsrcID partyId)
{
	if (!pRecord)
		return;

	ConnectionID connId = 0xFFFFFFFF; //not needed
	BoardID boardId = pRecord->physicalPort.physical_unit_params.board_id;

	// building MPL protocol
	CMplMcmsProtocol mplPrtcl;
	mplPrtcl.AddCommonHeader(START_DEBUG_RECORDING_REQ);
	mplPrtcl.AddMessageDescriptionHeader();
	mplPrtcl.AddPhysicalHeader(1, boardId, 1); //boxId, boardId, subBoardId, unitId = 0 means sent to CM
	mplPrtcl.AddPortDescriptionHeader(partyId, confId, connId);
	mplPrtcl.AddData(sizeof(TStartDebugRecordingReq), (char*)pRecord);
	CMplMcmsProtocolTracer(mplPrtcl).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");

	TRACEINTO << "BoardId:" << boardId << ", ConfId:" << confId << ", PartyId:" << partyId;

	mplPrtcl.SendMsgToMplApiCommandDispatcher();
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::SendStopRecordingToCm()
{
	//NO parameters from EMA. Opcode sent to all CMs ( MFA boards ) in system
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN_VALUE(!pSystemResources, STATUS_FAIL);

	if (pSystemResources->GetIsSystemRecording() == FALSE)
		return STATUS_OK; // we do not want to send stop recording when recording is off

	for (int i = 1; i <= BOARDS_NUM; i++)
	{
		if (!pSystemResources->IsBoardIdExists(i))
			continue;

		//fills in all needed params
		CMplMcmsProtocol mplPrtcl;
		mplPrtcl.AddCommonHeader(STOP_ALL_DEBUG_RECORDING_REQ);
		mplPrtcl.AddMessageDescriptionHeader();
		mplPrtcl.AddPhysicalHeader(1, i, 1); //boxId, boardId, subBoardId, unitId = 0 means sent to CM

		CMplMcmsProtocolTracer(mplPrtcl).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");
		PTRACE2INT(eLevelInfoHigh, " CResourceManager::SendStopRecordingToCm , boardId = ", i);

		mplPrtcl.SendMsgToMplApiCommandDispatcher();
	}
	//set & send to McuMngr monitoring ind recording OFF
	pSystemResources->SetIsSystemRecording(FALSE);
	SendSystemRecordingToMcuMngr(FALSE);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SendSystemRecordingToMcuMngr(DWORD isRecording)
{
	RSRCALLOC_MEDIA_RECORDING_S params;
	params.isRecording = isRecording;

	TRACEINTO << "IsRecording:" << isRecording;

	CSegment* pMsg = new CSegment();
	pMsg->Put((BYTE*)&params, sizeof(params));

	CManagerApi api(eProcessMcuMngr);
	STATUS res = api.SendMsg(pMsg, RSRC_SYSTEM_MEDIA_RECORDING_REQ);

	PASSERT(res);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMBoardLoadedRequest(CSegment* pMsg)
{
	TRACEINTO << "Opcode:RTM_ISDN_ENTITY_LOADED_IND";

	RTM_ISDN_ENTITY_LOADED_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->CreateRTMBoard(&params);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMServiceConfigRequest(CSegment* pMsg)
{
	RTM_ISDN_PARAMS_MCMS_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	TRACEINTO << "Opcode:RTM_ISDN_PARAMS_IND" << params;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->ConfigureRTMService(&params);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMServiceConfigEndRequest(CSegment* pMsg)
{
	TRACEINTO << "Opcode:RTM_ISDN_PARAMS_END_IND";

	SendMeetingRoomsAndProfileDBreadReqToConfMngr();
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMCreateSpanRequest(CSegment* pMsg)
{
	SPAN_ENABLED_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	TRACEINTO << "Opcode:RTM_ISDN_SPAN_ENABLED_IND" << params;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->ConfigureRTMSpan(&params);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMRemoveSpanRequest(CSegment* pMsg)
{
	SPAN_DISABLE_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	params.status = STATUS_FAIL;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		params.status = pSystemResources->ChangeSpanToNullConfigure(&params);

	TRACEINTO << "opcode:RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_REQ" << params;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&params, sizeof(params));
	ResponedClientRequest(RTM_ISDN_SPAN_DISABLE_IF_UPDATABLE_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMDisableAllSpansRequest(CSegment* pMsg)
{
	RTM_ISDN_BOARD_ID_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	TRACESTRFUNC(eLevelInfoHigh) << "Opcode:RTM_ISDN_DISABLE_ALL_SPANS_IND"
		<< ", BoardId:"    << params.boardId
		<< ", SubBoardId:" << params.subBoardId;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->DisableAllRtmSpanPerBoard(&params);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMAddPhoneRangeRequest(CSegment* pMsg)
{
	RTM_ISDN_PHONE_RANGE_UPDATE_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	TRACEINTO << "Opcode:RTM_ISDN_ADD_PHONE_RANGE_REQ" << params;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->AddRTMPhoneNumberRange(&params);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMDelPhoneRangeRequest(CSegment* pMsg)
{
	RTM_ISDN_PHONE_RANGE_UPDATE_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	params.status = STATUS_FAIL;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		params.status = pSystemResources->DeleteRTMPhoneNumberRange(&params);

	TRACEINTO << "Opcode:RTM_ISDN_PHONE_RANGE_DELETE_IF_UPDATABLE_REQ" << params;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&params, sizeof(params));
	ResponedClientRequest(RTM_ISDN_PHONE_RANGE_DELETE_IF_UPDATABLE_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMDelServiceRequest(CSegment* pMsg)
{
	RTM_ISDN_SERVICE_CANCEL_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	params.status = STATUS_FAIL;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		params.status = pSystemResources->DeleteRTMService(&params);

	TRACEINTO << "Opcode:RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_REQ" << params;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&params, sizeof(params));
	ResponedClientRequest(RTM_ISDN_SERVICE_CANCEL_IF_UPDATABLE_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMUpdatePortRequest(CSegment* pMsg)
{
	CNetServicesDB* pNetServicesDB = CHelperFuncs::GetNetServicesDB();
	PASSERT_AND_RETURN(!pNetServicesDB);

	UPDATE_ISDN_PORT_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	TRACEINTO << "Opcode:UPDATE_RTM_PORT_REQ" << params;

	ACK_UPDATE_ISDN_PORT_S result;
	memset(&result, 0, sizeof(result));

	pNetServicesDB->RTMUpdatePortRequest(&params, &result);

	TRACEINTO << params;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));
	ResponedClientRequest(UPDATE_RTM_PORT_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRTMReallocateBoardFull(CSegment* pMsg)
{
	BOARD_FULL_REQ_PARAMS_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	TRACEINTO << "Opcode:REALLOCATE_RTM_ON_BOARD_FULL_REQ" << params;

	ALLOC_PARTY_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	m_pRsrcAlloc->ReAllocateRTMBoardFull(&params, &result);

	TRACEINTO << result;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));
	ResponedClientRequest(REALLOCATE_RTM_ON_BOARD_FULL_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnDeallocateBondingTemporaryNumber(CSegment* pMsg)
{
	DEALLOCATE_BONDING_TEMP_PHONE_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	TRACEINTO << "Opcode:DEALLOCATE_BONDING_TEMP_PHONE_REQ" << params;

	m_pRsrcAlloc->DeAllocateBondingTemporaryNumber((const char*)params.serviceName, params.monitor_conf_id, params.monitor_party_id, params.BondingTemporaryPhoneNumber);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnUpdateResolutionThreshold(CSegment* pMsg)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	TRACEINTO << "Opcode:UPDATE_RESOLUTION_THRESHOLD";

	BOOL wasChanged = false;
	CResRsrcCalculator resRsrcCalculator;
	resRsrcCalculator.ReadResolutionConfigurationFromFile(pSystemResources->GetSystemCardsMode(), &wasChanged);

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator && wasChanged)
	{
		bool bReReserveAll = false;
		PROFILE_IND_LIST_S profileList;
		memset(&profileList, 0, sizeof(profileList));

		pMsg->Get((BYTE*)&profileList.list_size, sizeof(DWORD));
		if (profileList.list_size != 0)
		{
			DWORD size = profileList.list_size;
			profileList.profile_list = new PROFILE_IND_S[size];

			pMsg->Get((BYTE*)profileList.profile_list, sizeof(PROFILE_IND_S)*size);

			CProfilesDB* pProfilesDB = CHelperFuncs::GetProfilesDB();
			if (pProfilesDB != NULL)
			{
				for (DWORD i = 0; i < profileList.list_size; ++i)
				{
					ProfileIdType profileId = profileList.profile_list[i].profile_Id;
					eVideoPartyType newVideoType = profileList.profile_list[i].maxVideoPartyType;
					eVideoPartyType oldVideoType = pProfilesDB->GetMaxVideoPartyTypeByProfileId(profileId);
					if (newVideoType != oldVideoType)
					{
						TRACEINTO << "ProfileId:" << profileId << ", OldVideoType:" << eVideoPartyTypeNames[oldVideoType] << ", NewVideoType:" << eVideoPartyTypeNames[newVideoType];
						pProfilesDB->UpdateProfile(profileId, newVideoType);
						pReservator->ProfileChangedInd(profileId, FALSE);
						bReReserveAll = true;
					}
				}
			}
			delete[] profileList.profile_list;
		}

		if (bReReserveAll)
		{
			TRACEINTO << "Resolution slider and related profiles were changed, so need to rereserve all";
			pReservator->Rereserve(eRereserveAll);
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSystemCardsModeInd(CSegment* pMsg)
{
	DWORD tempMode = (DWORD)eSystemCardsMode_illegal;
	*pMsg >> tempMode;

	eSystemCardsMode cardsMode = (eSystemCardsMode)tempMode;

	TRACEINTO << "Opcode:MCMS_SYSTEM_CARDS_MODE_IND" << ", CardsMode:" << ::GetSystemCardsModeStr(cardsMode);

	m_resourceStartupInfo.SystemCardsModeInd(cardsMode);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		if (pSystemResources->GetSystemCardsMode() == cardsMode)//VNGR-15244
		{
			TRACEINTO << "The same card indication arrived";
			return;
		}
		pSystemResources->InitResourceAllocationMode(cardsMode);
	}
	else
	{
		PASSERT(1);
	}

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator && pReservator->GetDongleRestriction())
	{
		SetDongleRestriction();
	}

	// Olga - read a resolution slider configuration
	CResRsrcCalculator resRsrcCalculator;
	resRsrcCalculator.ReadResolutionConfigurationFromFile(cardsMode);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSlotNumberingConversionTableInd(CSegment* pMsg)
{
	SLOTS_NUMBERING_CONVERSION_TABLE_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:CARDS_SLOTS_NUMBERING_CONVERSION_TABLE_IND" << params;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	pSystemResources->SetSlotNumberingConversionTable(&params);

	for (WORD i = 0; i < params.numOfBoardsInTable; i++)
	{
		BoardID boardId = params.conversionTable[i].boardId;
		SubBoardID subBoardId = params.conversionTable[i].subBoardId;

		if (boardId <= BOARDS_NUM && MEDIA_SUB_BOARD_NUM == subBoardId) // card and not Isdn
		{
			CBoard* pBoard = pSystemResources->GetBoard(boardId);
			if (pBoard)
			{
				eCardType card_type = pBoard->GetCardType();
				if (eEmpty != card_type) // card already arrived before the table.
				{
					TRACEINTO << "BoardId:" << boardId << " - Board arrived before the table, adding the DisplayBoardId";
					WORD displayBoardId = pSystemResources->GetDisplayBoardId(boardId, MEDIA_SUB_BOARD_NUM);
					if (displayBoardId == NO_DISPLAY_BOARD_ID)
						TRACEINTOLVLERR << "BoardId:" << boardId << " - Display Board Id not found in table";
					else
						pBoard->SetDisplayBoardId(displayBoardId);
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSystemRamSizeInd(CSegment* pMsg)
{
	DWORD tempSize = (DWORD)eSystemRamSize_illegal;
	*pMsg >> tempSize;

	eSystemRamSize ramSize = (eSystemRamSize)tempSize;

	TRACEINTO << "Opcode:SYSTEM_RAM_SIZE_IND" << ", RamSize:" << ::GetSystemRamSizeStr(ramSize);

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->SetRamSize(ramSize);
	else
		PASSERT(1);

	SendCfsParamsToCS();
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSystemCPUProfileInd(CSegment* pMsg)
{
	WORD cpuSize;
	*pMsg >> cpuSize;

	TRACEINTO << "Opcode:SYSTEM_CPU_PROFILE_IND" << ", CPU_Size:" << cpuSize;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->SetCpuSize(cpuSize);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnUnitReconfiguredInd(CSegment* pMsg)
{
	UNIT_RECONFIG_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:RESOURCE_UNIT_RECONFIG_IND" << params;

	CMoveManager* pMoveManager = CHelperFuncs::GetMoveManager();
	if (pMoveManager)
		pMoveManager->ReceivedReconfigureUnitInd(&params);
	else
		PASSERT(1);

	//VNGR-23401,restart the timer in order to 1)check the resource 2)add or remove alarm.
	if (!IsValidTimer(RECONFIGURE_UNITS_TIMER))
		StartTimer(RECONFIGURE_UNITS_TIMER, SECONDS_FOR_RECONFIGURE_UNITS_TIMER * SECOND);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnReconfigureUnitsTimer(CSegment* pMsg)
{
	TRACEINTO << "Opcode:RECONFIGURE_UNITS_TIMER";

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	pSystemResources->OnReconfigureUnitsTimer();
	CheckResourceEnoughAndAddOrRemoveAciveAlarm();
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnFailoverStartSlaveInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:FAILOVER_START_SLAVE";

	CResourceProcess* pProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	STATUS status = STATUS_FAIL;
	if (pProcess)
	{
		CRsrvDB* pRsrvDB = CHelperFuncs::GetRsrvDB();
		WORD num_res = 0;
		if (pRsrvDB)
		{
			num_res = pRsrvDB->GetResNumber();
			if (0 == num_res)
				status = STATUS_OK;
			else
				status = STATUS_ILLEGAL_WHILE_ONGOING_CONFERENCE_AND_RESERVATIONS_EXISTS;
		}
	}
	if (STATUS_OK == status)
	{
		CReservator* pReservator = CHelperFuncs::GetReservator();
		if (pReservator)
			pReservator->SetRPMode(eRPSlaveMode);
		else
		{
			PASSERT(1);
			status = STATUS_FAIL;
		}
	}

	CSegment* pRspMsg = new CSegment;
	*pRspMsg << status;
	ResponedClientRequest(0, pRspMsg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnFailoverStartMasterBecomeSlaveInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:FAILOVER_START_MASTER_BECOME_SLAVE";

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator)
	{
		// to delete all reservations
		CRsrvDB* pRsrvDB = CHelperFuncs::GetRsrvDB();
		if (pRsrvDB)
		{
			if (pRsrvDB->GetResNumber() > 0)
				pReservator->DeleteAllRes();
		}

		eRPMode rpMode = pReservator->GetRPMode();
		pReservator->SetRPMode(eRPSlaveMode);

		TRACEINTO << "PreviousMode:" << rpMode;
	}
	else
	{
		PASSERT(1);
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnFailoverStartMasterInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:FAILOVER_START_MASTER";

	STATUS status = STATUS_FAIL;

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator)
	{
		eRPMode rpMode = pReservator->GetRPMode();
		if (rpMode == eRPSlaveMode) // due to the fact that it was from GUI
		{
			// to delete all reservations
			CRsrvDB* pRsrvDB = CHelperFuncs::GetRsrvDB();
			if (pRsrvDB)
			{
				if (pRsrvDB->GetResNumber() > 0)
				{
					pReservator->DeleteAllRes();
					status = STATUS_ONGOING_CONFERENCE_AND_RESERVATIONS_EXISTS;
				}
				else
					status = STATUS_OK;
			}
		}
		else
			status = STATUS_OK;

		pReservator->SetRPMode(eRPMasterMode);
		TRACEINTO << "PreviousMode:" << rpMode;
	}
	else
	{
		PASSERT(1);
		status = STATUS_FAIL;
	}

	CSegment* pRspMsg = new CSegment;
	*pRspMsg << status;
	ResponedClientRequest(0, pRspMsg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnFailoverReStartSlaveInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:FAILOVER_RESTART_SLAVE";

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator)
	{
		eRPMode rpMode = pReservator->GetRPMode();
		if (rpMode == eRPSlaveMode)
		{
			// to delete all reservations
			CRsrvDB* pRsrvDB = CHelperFuncs::GetRsrvDB();
			if (pRsrvDB)
			{
				if (pRsrvDB->GetResNumber() > 0)
					pReservator->DeleteAllRes();
			}
		}
		else
		{
			PASSERT(1);
			TRACEINTO << "PreviousMode:" << rpMode;
		}

	}
	else
		PASSERT(1);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnFailoverSlaveBcmMasterInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:FAILOVER_SLAVE_BECOME_MASTER";

	CReservator* pReservator = CHelperFuncs::GetReservator();
	if (pReservator)
	{
		eRPMode rpMode = pReservator->GetRPMode();
		pReservator->SetRPMode(eRPMasterMode);
		TRACEINTO << "PreviousMode:" << rpMode;
	}
	else
	{
		PASSERT(1);
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnNumericIdRequest(CSegment* pMsg)
{
	PREFERRED_NUMERIC_ID_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:RESOURCE_NUMERIC_ID_REQ" << params;

	STATUS status = STATUS_FAIL;

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN(!pReservator);

	WORD num_conf = pReservator->GetNumConfPerType(params.conf_type);
	switch (params.conf_type)
	{
		case eMR_type:
			status = (num_conf >= pReservator->GetMaxNumbMR()) ? STATUS_MAX_RESERVATIONS_EXCEEDED : STATUS_OK;
			break;
		case eEQ_type:
			status = (num_conf >= pReservator->GetMaxNumbEQ()) ? STATUS_MAX_RESERVATIONS_EXCEEDED : STATUS_OK;
			break;
		default:
		{
			status = STATUS_FAIL;
			TRACEWARN << "ConfType:" << params.conf_type << " - Illegal conference type";
		}
	}

	std::string requested_numeric_Id(params.numeric_Id);
	if (status == STATUS_OK)
	{
		ConfMonitorID confId = pReservator->AllocateMntrConfId();
		status = (confId) ? STATUS_OK : STATUS_INSUFFICIENT_MNTR_CONF_ID;

		if (status == STATUS_OK)
		{
			status = pReservator->AllocateNumericConfId(params.numeric_Id); //ask specific numeric id
			if (status != STATUS_OK)
			{
				params.numeric_Id[0] = '\0';
				status = pReservator->AllocateNumericConfId(params.numeric_Id); //ask non-specific
				if (status != STATUS_OK)
					pReservator->DeAllocateMntrConfId(confId);
			}
		}
		if (status == STATUS_OK)
		{
			params.monitor_Id = confId;
			pReservator->AddSleepingConference(params.numeric_Id, params.monitor_Id, params.conf_type);
			pReservator->PlusNumConfPerType(params.conf_type);
		}
	}
	params.status = status;
	TRACEINTO << "MonitorConfId:" << params.monitor_Id << ", Status:" << params.status;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&params, sizeof(params));

	CManagerApi api(eProcessConfParty);
	api.SendMsg(pSeg, RESOURCE_NUMERIC_ID_IND);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::MoveMasterAC(DWORD boardID, eChangedStateAC stateAC)
{
	STATUS status = STATUS_FAIL;
	CSystemResources *pSyst = CHelperFuncs::GetSystemResources();
	eProcessStatus curStatus = CHelperFuncs::GetProcessStatus();
	if ((eProcessInvalid == curStatus) || (eProcessStartup == curStatus) || (eProcessIdle == curStatus) || (!pSyst))
		return STATUS_ACTION_ILLEGAL_AT_SYSTEM_STARTUP;

	eProductType prodType = pSyst->GetProductType();
	PASSERT_AND_RETURN_VALUE((prodType != eProductTypeRMX4000), STATUS_FAIL);

	WORD masterBId = pSyst->GetAudioCntrlMasterBid();

	if (eAC_Back == stateAC)
	{
		//no Master Audio Controller
		if (0xFFFF != masterBId)
			return status;

		CBoard* pBoard = pSyst->GetBoard(boardID);

		// Master AC unit came back
		if (pBoard && E_AC_MASTER == pBoard->GetACType())
		{
			TRACEINTO << " CResourceManager::MoveMasterAC :  Master AC unit came back, boardID = " << boardID;
			pSyst->SetAudioCntrlMasterBid(boardID);
			status = STATUS_OK;
		}

		// Reserved AC unit came back
		if (pBoard && E_AC_RESERVED == pBoard->GetACType())
		{
			TRACEINTO << " CResourceManager::MoveMasterAC :  Reserved AC unit came back, boardID = " << boardID;
			WORD foundMasterBId = 0;
			for (WORD i = 0; i < BOARDS_NUM; i++)
			{
				WORD oneBasedBoardId = i + 1;
				if (!pSyst->IsBoardIdExists(oneBasedBoardId))
					continue;
				CBoard* pBoardOneBased = pSyst->GetBoard(oneBasedBoardId);
				if (pBoardOneBased && E_AC_MASTER == pBoardOneBased->GetACType())
				{
					foundMasterBId = oneBasedBoardId; //master AC exists only as enum on board, not on system
					TRACEINTO << " CResourceManager::MoveMasterAC :  Master AC exists only as enum on board, not on system = " << foundMasterBId;
					break;
				}
			}
			if (foundMasterBId)
				MoveMasterToReserved(foundMasterBId);

			MoveReservedToMaster(boardID, pBoard->GetAudioControllerUnitId());

			// update shared memory
			pSyst->SwapMasterAcAndReserveAc(foundMasterBId, boardID);

			status = STATUS_OK;
		}
	}
	else
	{
		WORD bestRsrvACBId = 0xFFFF, bestAcUnitID = 0xFFFF;

		status = pSyst->FindBestRsrvAC(bestRsrvACBId, bestAcUnitID);

		TRACEINTO << " CResourceManager::MoveMasterAC : status = " << status << ", found bestRsrvACBId = " << bestRsrvACBId << ", bestAcUnitID = " << bestAcUnitID;
		if (STATUS_OK != status)
		{
			pSyst->SetAudioCntrlMasterBid(0xFFFF);
			//TODO: forbid allocation
			return status;
		}
		//inform a master about it is reserved AC from now
		MoveMasterToReserved(masterBId);

		//inform a prev reserved AC about it is a master from now
		MoveReservedToMaster(bestRsrvACBId, bestAcUnitID);

		// update shared memory
		pSyst->SwapMasterAcAndReserveAc(masterBId, bestRsrvACBId);
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::MoveMasterToReserved(BoardID boardId)
{
	TRACEINTO << "BoardId:" << boardId;

	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	CBoard* pBoard = pSystemResources ? pSystemResources->GetBoard(boardId) : NULL;

	TAcTypeReq params;
	params.unBoardId = boardId;
	params.unSubBoardId = MFA_SUBBOARD_ID;
	params.unUnitId = pBoard ? pBoard->GetAudioControllerUnitId() : 0;
	params.eAudioContollerType = E_AUDIO_CONTROLLER_TYPE_RESERVED;

	CSegment* pSeg = new CSegment();
	pSeg->Put((BYTE*)&params, sizeof(params));

	CManagerApi cardsManagerApi(eProcessCards);
	cardsManagerApi.SendMsg(pSeg, AC_TYPE_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::MoveReservedToMaster(BoardID boardId, UnitID unitId)
{
	TRACEINTO << "BoardId:" << boardId << ", UnitId:" << unitId;

	TAcTypeReq params;
	params.unBoardId = boardId;
	params.unSubBoardId = MFA_SUBBOARD_ID;
	params.unUnitId = unitId;
	params.eAudioContollerType = E_AUDIO_CONTROLLER_TYPE_MASTER;

	CSegment* pSeg = new CSegment();
	pSeg->Put((BYTE*)&params, sizeof(params));

	CManagerApi cardsManagerApi(eProcessCards);
	cardsManagerApi.SendMsg(pSeg, AC_TYPE_REQ);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnStopVideoPreview(CSegment* pMsg)
{
	STATUS status = STATUS_FAIL;

	ConfMonitorID  monitorConfId  = 0;
	PartyMonitorID monitorPartyId = 0;
	WORD           direction      = 0;
	ConfRsrcID     confId         = INVALID;
	PartyRsrcID    partyId        = INVALID;

	*pMsg
		>> monitorConfId
		>> monitorPartyId
		>> direction;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();

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
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnHandleChangeSysMode(CSegment* pMsg)
{
	BOOL isCOP;
	*pMsg >> isCOP;

	TRACEINTO << "Opcode:RSRC_CHANGE_SYS_MODE_REQ" << ", IsCopMode:" << (int)isCOP;

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	CReservator* pReservator = CHelperFuncs::GetReservator();
	PASSERT_AND_RETURN(!pReservator);

	pReservator->SetRMode2C(isCOP);
	CHelperFuncs::CheckAllReservationsSysMode();

	//curr sys mode
	CAllocationModeDetails allocationModeDetails_sys;
	pSystemResources->GetAllocationMode(&allocationModeDetails_sys); //the sys curr situation
	eAllocationModeType lastmode = allocationModeDetails_sys.GetMode();

	//req mode
	CAllocationModeDetails allocationMode;
	allocationMode.ReadFromProcessSetting();
	eAllocationModeType fileAllocMode = allocationMode.GetMode();

	//set situation in the struct.
	allocationMode.SetModes(lastmode, fileAllocMode);

	//save situation in file and in sys. reconfigure units
	pSystemResources->SetAllocationMode(&allocationMode);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceSetProcessState(CSegment* pMsg) //olga
{
	eProcessStatus oldState = CHelperFuncs::GetProcessStatus();
	CManagerTask::OnTaskChangeStateInd(pMsg);
	eProcessStatus newState = CHelperFuncs::GetProcessStatus();

	std::ostringstream msg;
	msg << "Opcode:TASK_CHANGE_STATE_FAULT_IND" << ", OldState:" << ::GetProcessStatusName(oldState) << ", NewState:" << ::GetProcessStatusName(newState);

	if ((oldState != newState) && (eProcessMajor == newState))
	{
		CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
		PASSERT_AND_RETURN(!pSystemResources);

		TRACEINTO << msg.str().c_str() << " - Check configuration";
		pSystemResources->CheckSetEnhancedConfiguration();
	}

	if (eProcessStartup == oldState && eProcessStartup != newState)
	{
		TRACEINTO << msg.str().c_str();
		OnStartupEnded();
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::AddPortsToPartyInfo(std::ostream& answer, CPartyDebugInfo* pPartyDebugInfo)
{
	PASSERT_AND_RETURN(!pPartyDebugInfo);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB);

	ConfMonitorID  monitorConfId  = pPartyDebugInfo->GetMonitorConfId();
	PartyMonitorID monitorPartyId = pPartyDebugInfo->GetMonitorPartyId();
	ConfRsrcID     confId         = pPartyDebugInfo->GetResourceConfId();
	PartyRsrcID    partyId        = pPartyDebugInfo->GetResourcePartyId();

	TRACEINTO << "ConfId:" << confId << ", PartyId:" << partyId;

	const CConfRsrc* pConfRsrc = pConfRsrcDB->GetConfRsrcByRsrcConfId(confId);
	if (!pConfRsrc)
	{
		TRACEWARN << "ConfId:" << confId << "Failed, conference doesn't exist";
		return;
	}

	const CPartyRsrc* pPartyRsrc = pConfRsrc->GetParty(monitorPartyId);
	if (!pPartyRsrc)
	{
		TRACEWARN << "PartyId:" << partyId << "Failed, conference doesn't exist";
		return;
	}

	RoomID roomID = pPartyRsrc->GetRoomPartyId();

	answer << "=================================================";
	answer
			<< "\nPartyMonitoringId:" << monitorPartyId
			<< "\nConfMonitoringId:" << monitorConfId
			<< "\nPartyResourceId:" << partyId
			<< "\nConfResourceId:" << confId
			<< "\nRoomId:" << roomID;

	if (pPartyRsrc->m_ssrcAudio != 0)
	{
		answer << "\n========== SVC PARTY PARAMS =====================";
		answer << "\nSsrcAudio:" << pPartyRsrc->m_ssrcAudio;
		answer << "\nSsrcVideo:";
		for (int i = 0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; i++)
			answer << pPartyRsrc->m_ssrcVideo[i] << ", ";

		answer << "\nContent:";
		for (int i = 0; i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; i++)
			answer << pPartyRsrc->m_ssrcContent[i] << ", ";
	}
	answer << "\n=================================================";

	CRsrcDesc** pRsrcDescArray = new CRsrcDesc*[MAX_NUM_ALLOCATED_RSRCS];
	for (int i = 0; i < MAX_NUM_ALLOCATED_RSRCS; i++)
		pRsrcDescArray[i] = NULL;

	// Get all resource descriptors per a given party
	pConfRsrc->GetDescArrayPerResourceTypeByRsrcId(partyId, eLogical_res_none, pRsrcDescArray, MAX_NUM_ALLOCATED_RSRCS);
	for (int j = 0; j < MAX_NUM_ALLOCATED_RSRCS; j++)
	{
		if (pRsrcDescArray[j] == NULL)
			break;

		answer
			<< "\n"         << pRsrcDescArray[j]->GetType()
			<< ": ConnId:"  << pRsrcDescArray[j]->GetConnId()
			<< ", BoardId:" << pRsrcDescArray[j]->GetBoardId()
			<< ", UnitId:"  << pRsrcDescArray[j]->GetUnitId()
			<< ", PortId:"  << pRsrcDescArray[j]->GetFirstPortId();
	}
	delete[] pRsrcDescArray;
	answer << "\n=================================================\n";
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceUnitRecoveryInd(CSegment* pMsg)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pMsg);

	STATUS status = mplMcmsProtocol.ValidateDataSize(sizeof(UNIT_RECOVERY_S));
	if (STATUS_OK != status)
	{
		TRACEWARN << "Failed, ValidateDataSize returns wrong status UNIT_RECOVERY_IND";
		return;
	}

	CSegment seg;
	if (mplMcmsProtocol.getDataLen())
		seg.Put((unsigned char*)mplMcmsProtocol.GetData(), mplMcmsProtocol.getDataLen());

	UNIT_RECOVERY_S* pParams = (UNIT_RECOVERY_S*)(seg.GetPtr());

	TRACEINTO << "Opcode:UNIT_RECOVERY_IND" << *pParams;

	RECOVERY_REPLACEMENT_UNIT_S result;
	memset(&result, 0, sizeof(result));

	m_pRsrcAlloc->UnitRecovery(pParams, &result);

	TRACEINTO << result;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));

	CMplMcmsProtocol *pProtocol = new CMplMcmsProtocol();
	pProtocol->AddCommonHeader(RECOVERY_REPLACEMENT_UNIT_REQ);
	pProtocol->AddMessageDescriptionHeader();
	pProtocol->AddPhysicalHeader(result.unit_replacement.box_id, result.unit_replacement.board_id, result.unit_replacement.sub_board_id);
	pProtocol->AddPortDescriptionHeader(DUMMY_PARTY_ID, DUMMY_CONF_ID, DUMMY_CONNECTION_ID);

	BYTE* pMessage = new BYTE[pSeg->GetWrtOffset()];
	pSeg->Get(pMessage, pSeg->GetWrtOffset());
	pProtocol->AddData(pSeg->GetWrtOffset(), (const char*)pMessage);
	PDELETEA(pMessage);

	CMplMcmsProtocolTracer(*pProtocol).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");
	status = pProtocol->SendMsgToMplApiCommandDispatcher();

	PASSERT(status);
	POBJDELETE(pProtocol);
	POBJDELETE(pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceUnitRecoveryEndInd(CSegment* pMsg)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pMsg);

	STATUS status = mplMcmsProtocol.ValidateDataSize(sizeof(UNIT_RECOVERY_S));
	if (STATUS_OK != status)
	{
		TRACEWARN << "Failed, ValidateDataSize returns wrong status";
		return;
	}

	CSegment seg;
	if (mplMcmsProtocol.getDataLen())
		seg.Put((unsigned char*)mplMcmsProtocol.GetData(), mplMcmsProtocol.getDataLen());

	UNIT_RECOVERY_S* pParams = (UNIT_RECOVERY_S*)(seg.GetPtr());

	TRACEINTO << "Opcode:UNIT_RECOVERY_END_IND" << *pParams;

	m_pRsrcAlloc->UnitRecoveryEnd(pParams);
}
void CResourceManager::OnMplPartyDebugInfoInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:PARTY_DEBUG_INFO_IND";
	PartyDebugInfoInd(pMsg, false);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnMplPartyCmDebugInfoInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:PARTY_CM_DEBUG_INFO_IND";
	PartyDebugInfoInd(pMsg, true);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnMplConfDebugInfoInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:CONF_DEBUG_INFO_IND";
	PartyDebugInfoInd(pMsg, false);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::PartyDebugInfoInd(CSegment* pSeg, bool isCM)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pSeg);

	PartyRsrcID partyId = mplMcmsProtocol.getPortDescriptionHeaderParty_id();
	ConfRsrcID  confId  = mplMcmsProtocol.getPortDescriptionHeaderConf_id();

	TRACEINTO << "ConfId:" << confId << ", PartyId:" << partyId;

	char portInfo[DEBUG_INFO_STRING_SIZE + 1];
	memset(portInfo, '\0', DEBUG_INFO_STRING_SIZE + 1);

	DEBUG_INFO_IND_S* pPortDebugInfoStruct = (DEBUG_INFO_IND_S*)mplMcmsProtocol.getpData();
	if (pPortDebugInfoStruct)
		strcpy_safe(portInfo, pPortDebugInfoStruct->debugInfo);

	CPartyDebugInfo* pPartyDebugInfo = NULL;
	std::set<CPartyDebugInfo*>::iterator _end = m_PartyDebugInfoList->end();
	for (std::set<CPartyDebugInfo*>::iterator _itr = m_PartyDebugInfoList->begin(); _itr != _end; ++_itr)
	{
		if ((*_itr)->IsSameParty(confId, partyId))
		{
			pPartyDebugInfo = *_itr;
			break;
		}
	}
	if (!pPartyDebugInfo)
	{
		TRACEWARN << "ConfId:" << confId << ", PartyId:" << partyId << "Failed, party not found";
		return;
	}

	DWORD mpl_board_id       = mplMcmsProtocol.getPhysicalInfoHeaderBoard_id();
	DWORD mpl_sub_board_id   = mplMcmsProtocol.getPhysicalInfoHeaderSub_board_id();
	DWORD mpl_unit_id        = mplMcmsProtocol.getPhysicalInfoHeaderUnit_id();
	DWORD mpl_accelerator_id = mplMcmsProtocol.getPhysicalInfoHeaderAccelerator_id();
	DWORD mpl_port_id        = mplMcmsProtocol.getPhysicalInfoHeaderPort_id();
	DWORD mpl_connId         = mplMcmsProtocol.getPortDescriptionHeaderConnection_id();

	pPartyDebugInfo->PortInfoInd(isCM, mpl_board_id, mpl_sub_board_id, mpl_unit_id, mpl_accelerator_id, mpl_port_id, mpl_connId, (char*)portInfo);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnTimerRetreiveDebugInfo(CSegment* pSeg, bool isCM)
{
	std::ostringstream msg;
	WORD num_of_active_parties = 0;
	std::set<CPartyDebugInfo*>::iterator _itr, _end = m_PartyDebugInfoList->end();
	for (_itr = m_PartyDebugInfoList->begin(); _itr != _end; ++_itr)
	{
		if ((*_itr)->IsInfoRetrieved())
		{
			msg << "\nPartyId:" << (*_itr)->GetResourcePartyId() << ", ConfId:" << (*_itr)->GetResourceConfId() << " - Party info deleting";
			CPartyDebugInfo* pPartyDebugInfo = *_itr;
			m_PartyDebugInfoList->erase(_itr);
			delete pPartyDebugInfo;
			break;
		}
		else
		{
			msg << "\nPartyId:" << (*_itr)->GetResourcePartyId() << ", ConfId:" << (*_itr)->GetResourceConfId() << " - Party info retrieving";
			num_of_active_parties++;
		}

	}
	TRACEINTO << msg.str().c_str();

	if (num_of_active_parties > 0)
		StartTimer(RETRIEVE_INFO_TIMER, RETRIEVE_INFO_TIMEOUT);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnMultipleServicesInd(CSegment* pParam)
{
	BYTE isMultipleIpServices;
	*pParam >> isMultipleIpServices;

	TRACEINTO << "Opcode:MCUMNGR_TO_RSRC_MULTIPLE_SERVICES_IND" << ", IsMultipleIpServices:" << (int)isMultipleIpServices;

	m_resourceStartupInfo.MultipleServicesInd(isMultipleIpServices);
	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->SetMultipleIpServices(isMultipleIpServices);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::RemoveActiveAlarm(WORD alarm, bool& isStartupEnded)
{
	isStartupEnded = false;
	eTaskState stateBefore = GetTaskState();
	RemoveActiveAlarmByErrorCode(alarm);
	eTaskState stateAfter = GetTaskState();

	TRACEINTO << "ActiveAlarm:" << GetAlarmName(alarm) << ", OldState:" << GetTaskStateName(stateBefore) << ", NewState:" << GetTaskStateName(stateAfter) << " - Removing active alarm";

	if (stateBefore == eTaskStateStartup && stateAfter != eTaskStateStartup)
	{
		isStartupEnded = true;
		TRACEINTO << "Startup ended with state " << GetTaskStateName(stateAfter);
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::RemoveActiveAlarm(WORD alarm)
{
	bool isStartupEnded = false;
	RemoveActiveAlarm(alarm, isStartupEnded);
	if (isStartupEnded)
		OnStartupEnded();
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnStartupEnded()
{
	TRACEINTO << m_resourceStartupInfo;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->DumpRtmSpans();
	UpdateIpServicesDongleRestrictions();
}

////////////////////////////////////////////////////////////////////////////
BOOL CResourceManager::IsStartupFinished() const
{
	eMcuState systemState = CProcessBase::GetProcess()->GetSystemState();
	if (eMcuState_Invalid == systemState || eMcuState_Startup == systemState)
		return FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::UpdateIpServicesDongleRestrictions()
{
	if (m_resourceStartupInfo.CanUpdateIpServicesDongleRestrictions())
	{
		CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
		PASSERT_AND_RETURN(!pSystemResources);

		TRACEINTO << "updating";

		pSystemResources->UpdateIpServicesDongleRestriction();
	}
	else
	{
		TRACEINTO << "not ready" << m_resourceStartupInfo;
	}
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnIPServiceDefaultUpdate(CSegment* pMsg)
{
	DEFAULT_IP_SERVICE_S params;
	memcpy(&params, pMsg->GetPtr(), sizeof(params));

	const char* defaultH323serv = params.defaultH323ServiceName;
	const char* defaultSIPserv  = params.defaultSIPServiceName;

	TRACEINTO << "Opcode:CS_RSRC_DEFAULT_SERVICE_IND" << ", DefaultH323ServiceName:" << defaultH323serv << ", DefaultSIPServiceName:" << defaultSIPserv;
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
		pSystemResources->SetDefaultIpService(defaultH323serv, defaultSIPserv);
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::ConverFailureStatus(STATUS status, OPCODE opCode)
{
	switch (opCode)
	{
		case RESET_UNIT_ACTION:
			status = STATUS_FALIED_TO_RESET_UNITS;
			break;
		case ENABLE_UNIT_ACTION:
			status = STATUS_FALIED_TO_ENABLE_UNITS;
			break;
		case DISABLE_UNIT_ACTION:
			status = STATUS_FALIED_TO_DISABLE_UNITS;
			break;
		default:
			PTRACE(eLevelInfoNormal, "ConverFailureStatus failed");
			break;
	}
	return status;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::ResetUnitById(int boardId, int unitId, int subBoardId)
{
	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		eProductType product_type = pSystemResources->GetProductType();
		if (eProductTypeNinja == product_type)
		{
			//reuse Rescue Card(reset all dsp card) Request  as reset unit message -> MFA -> MPProxy -> VMP  in Ninja
			return SendResetUnitToMFA(boardId, unitId);
		}
	}

	UNIT_RECOVERY_S* pParams = new UNIT_RECOVERY_S;
	pParams->unit_recover.box_id = 1;
	pParams->unit_recover.board_id = boardId;
	pParams->unit_recover.sub_board_id = subBoardId;
	pParams->unit_recover.unit_id = unitId;

	RECOVERY_REPLACEMENT_UNIT_S* pResultStruct = new RECOVERY_REPLACEMENT_UNIT_S;
	m_pRsrcAlloc->UnitRecovery(pParams, pResultStruct);

	BYTE box_id = pResultStruct->unit_replacement.box_id;
	BYTE board_id = pResultStruct->unit_replacement.board_id;
	BYTE sub_board_id = pResultStruct->unit_replacement.sub_board_id;
	BYTE unit_id = pResultStruct->unit_replacement.unit_id;
	STATUS unit_replacement_status = pResultStruct->status;

	CSegment* pReplaceParam = new CSegment;
	pReplaceParam->Put((BYTE*)pResultStruct, sizeof(RECOVERY_REPLACEMENT_UNIT_S));

	CMplMcmsProtocol *mplPrtclReplace = new CMplMcmsProtocol();
	mplPrtclReplace->AddCommonHeader(RECOVERY_REPLACEMENT_UNIT_REQ);
	mplPrtclReplace->AddMessageDescriptionHeader();
	mplPrtclReplace->AddPhysicalHeader(1, boardId, 1);
	mplPrtclReplace->AddPortDescriptionHeader(DUMMY_PARTY_ID, DUMMY_CONF_ID, DUMMY_CONNECTION_ID);

	BYTE* pMessageResult = new BYTE[pReplaceParam->GetWrtOffset()];
	pReplaceParam->Get(pMessageResult, pReplaceParam->GetWrtOffset());
	mplPrtclReplace->AddData(pReplaceParam->GetWrtOffset(), (const char*)pMessageResult);
	PDELETEA(pMessageResult);

	CMplMcmsProtocolTracer(*mplPrtclReplace).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");
	STATUS status = mplPrtclReplace->SendMsgToMplApiCommandDispatcher();

	POBJDELETE(pParams);
	POBJDELETE(pReplaceParam);
	POBJDELETE(pResultStruct);
	POBJDELETE(mplPrtclReplace);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::EnableUnitById(int boardId, int unitId, bool enable)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		eProductType product_type = pSystemResources->GetProductType();
		STATUS status = STATUS_OK;
		if (eProductTypeNinja == product_type && boardId == ISDN_CARD_SLOT_ID)
		{
			status = pSystemResources->SetSpanRTMStatus(boardId, unitId, enable, TRUE);
		}
		else
		{
			status = pSystemResources->SetUnitMfaStatus(boardId, unitId, enable, TRUE);
		}

		if (STATUS_OK != status)
		{
			TRACEINTO << " CResourceManager::OnResetUnitsRequest ";
		}
	}

	CheckResourceEnoughAndAddOrRemoveAciveAlarm();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnMplUnitFatalInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:UNIT_FATAL_IND";
	SetMplUnitFatalInd(pMsg, false);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnMplUnitUnFatalInd(CSegment* pMsg)
{
	TRACEINTO << "Opcode:UNIT_UNFATAL_IND";
	SetMplUnitFatalInd(pMsg, true);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::SetMplUnitFatalInd(CSegment* pMsg, bool recievedIsEnable)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pMsg);

	STATUS status = mplMcmsProtocol.ValidateDataSize(sizeof(UNIT_RECOVERY_S));
	if (STATUS_OK != status)
	{
		TRACEWARN << "ValidateDataSize returns wrong status";
		return;
	}

	CSegment seg;
	if (mplMcmsProtocol.getDataLen())
		seg.Put((unsigned char*)mplMcmsProtocol.GetData(), mplMcmsProtocol.getDataLen());

	UNIT_RECOVERY_S* pParams = (UNIT_RECOVERY_S*)(seg.GetPtr());

	ExecUnitFatalOrUnFatalInd(pParams, recievedIsEnable);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSNMPConfigInd(CSegment* pMsg)
{
	BOOL bSNMPEnabled = FALSE;
	*pMsg >> bSNMPEnabled;

	TRACEINTO << "Opcode:SNMP_CONFIG_TO_OTHER_PROGRESS" << ", IsSNMP_Enabled:" << (int)bSNMPEnabled;

	CResourceProcess* pProcess = (CResourceProcess*)CProcessBase::GetProcess();
	FPASSERT_AND_RETURN(!pProcess);

	pProcess->SetIsSNMPEnabled(bSNMPEnabled);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnSendAllPartiesOnUnit(CSegment* pMsg)
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	CConfRsrcDB* pConfRsrcDB = CHelperFuncs::GetConfRsrcDB();
	PASSERT_AND_RETURN(!pConfRsrcDB);

	DWORD boardId = 0;
	DWORD unitId = 0;

	*pMsg >> boardId;
	*pMsg >> unitId;

	COsQueue q;
	q.DeSerialize(*pMsg);

	TRACEINTO << "Opcode:GET_CONFS_AND_PARTIES_LIST_REQ" << ", BoardId:" << boardId << ", UnitId:" << unitId;

	CUnitMFA* pUnit = pSystemResources->GetUnit(boardId, unitId);
	PASSERT_AND_RETURN(!pUnit);

	const CActivePortsList* pActivePorts = pUnit->GetActivePorts();

	CONF_PARTY_LIST_S partyList;
	memset(&partyList, 0, sizeof(partyList));

	std::list<CONF_PARTY_ELEMENTS_S> tmpPartyList;

	if (pActivePorts == NULL || pActivePorts->size() == 0)
	{
		TRACEINTO << "No active ports on DSP";
	}
	else
	{
		std::set<CActivePort>::iterator _end = pActivePorts->end();
		for (std::set<CActivePort>::iterator _itr = pActivePorts->begin(); _itr != _end; ++_itr)
		{
			eResourceTypes physicalPortType = _itr->GetPortType();

			CONF_PARTY_ELEMENTS_S confIdPartyId;
			if (physicalPortType == ePhysical_video_encoder)
				confIdPartyId.logicalRsrcType = eLogical_video_encoder;
			else if (physicalPortType == ePhysical_video_decoder)
				confIdPartyId.logicalRsrcType = eLogical_video_decoder;
			else
				continue;

			ConfMonitorID monitorConfId = 0xFFFFFFFF;
			PartyMonitorID monitorPartyId = 0xFFFFFFFF;
			if (pConfRsrcDB->GetMonitorIdsRsrcIds(_itr->GetConfId(), _itr->GetPartyId(), monitorConfId, monitorPartyId) != STATUS_OK)
			{
				TRACEWARN << "ConfId:" << _itr->GetConfId() << ", PartyId:" << _itr->GetPartyId() << " - Failed to get monitor id pair";
				continue;
			}

			confIdPartyId.monitor_conf_id = monitorConfId;
			confIdPartyId.rsrc_conf_id    = _itr->GetConfId();
			confIdPartyId.rsrc_party_id   = _itr->GetPartyId();
			confIdPartyId.port_id         = _itr->GetPortId();
			confIdPartyId.acceleratorId   = _itr->GetAcceleratorId();

			tmpPartyList.push_back(confIdPartyId);
		}
	}

	partyList.boardId = boardId;
	partyList.unitId = unitId;
	partyList.list_size = tmpPartyList.size();
	partyList.conf_party_list = new CONF_PARTY_ELEMENTS_S[partyList.list_size];
	memset(partyList.conf_party_list, 0, partyList.list_size*sizeof(CONF_PARTY_ELEMENTS_S));

	int i = 0;
	std::list<CONF_PARTY_ELEMENTS_S>::iterator _end = tmpPartyList.end();
	for (std::list<CONF_PARTY_ELEMENTS_S>::iterator _itr = tmpPartyList.begin(); _itr != _end; ++i, ++_itr)
		partyList.conf_party_list[i] = *_itr;

	TRACEINTO << partyList;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&partyList.list_size, sizeof(partyList.list_size));
	pSeg->Put((BYTE*)&partyList.boardId, sizeof(partyList.boardId));
	pSeg->Put((BYTE*)&partyList.unitId, sizeof(partyList.unitId));
	if (partyList.list_size)
		pSeg->Put((BYTE*)partyList.conf_party_list, partyList.list_size*sizeof(CONF_PARTY_ELEMENTS_S));

	CTaskApi api;
	api.CreateOnlyApi(q);
	api.SendMsg(pSeg, GET_CONFS_AND_PARTIES_LIST_IND);

	if (partyList.conf_party_list)
		delete[] partyList.conf_party_list;
}

////////////////////////////////////////////////////////////////////////////
STATUS CResourceManager::SendResetUnitToMFA(DWORD boardId, DWORD dspUnitId)
{
	#ifndef RESCUE_CARD_REQ
		#define RESCUE_CARD_REQ 5010018
	#endif

	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		eProductType product_type = pSystemResources->GetProductType();
		if (eProductTypeNinja != product_type)
		{
			return STATUS_FAIL;
		}
	}
	WORD phyboardId = NO_DISPLAY_BOARD_ID, dspboardId = NO_DISPLAY_BOARD_ID, subBoardId = NO_DISPLAY_BOARD_ID;

	if (boardId >= DSP_CARD_SLOT_ID_0 && boardId <= DSP_CARD_SLOT_ID_2)
	{
		//reuse Rescue Card(reset all dsp card) Request  as reset unit message -> MFA -> MPProxy -> VMP  in Ninja
		phyboardId = FIXED_BOARD_ID_MAIN_SOFTMCU;
		subBoardId = FIXED_CM_SUBBOARD_ID;
		dspboardId = boardId - DSP_CARD_SLOT_ID_0;
	}
	else if (boardId == ISDN_CARD_SLOT_ID)
	{
		//reuse Rescue Card(reset RTM DSP card) Request  as reset unit message -> MFA -> VMP  in Ninja
		phyboardId = 1;
		subBoardId = RTM_SUB_BOARD_NUM;
		dspboardId = 1;
	}
	else
	{
		TRACEWARN << "BoardId:" << boardId << ", UnitId:" << dspUnitId << " - Invalid";
		return STATUS_FAIL;
	}

	CMplMcmsProtocol pMplMcmsProtocol;
	pMplMcmsProtocol.AddCommonHeader(RESCUE_CARD_REQ);
	pMplMcmsProtocol.AddMessageDescriptionHeader();
	pMplMcmsProtocol.AddPhysicalHeader(1, phyboardId, subBoardId);
	RESCUE_CARD_REQ_S rescueCardParam;
	rescueCardParam.boardID = dspboardId;
	rescueCardParam.unitID = dspUnitId;
	pMplMcmsProtocol.AddData(sizeof(RESCUE_CARD_REQ_S), (const char*)(&rescueCardParam));

	TRACEINTO << "PhyboardId:" << phyboardId << ", SubBoardId:" << subBoardId << ", boardId:" << dspboardId << ", UnitId:" << dspUnitId << " - Sending rescue request to mfa";
	return pMplMcmsProtocol.SendMsgToMplApiCommandDispatcher();
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnHighUsageCPUInd(CSegment* pMsg)
{
	CMplMcmsProtocol mplMcmsProtocol;
	mplMcmsProtocol.DeSerialize(*pMsg);

	STATUS status = mplMcmsProtocol.ValidateDataSize(sizeof(CM_HIGH_CPU_USAGE_S));
	if (STATUS_OK != status)
	{
		TRACEINTO << "Failed, ValidateDataSize returns wrong status";
		return;
	}

	CSegment seg;
	if (mplMcmsProtocol.getDataLen())
		seg.Put((unsigned char*)mplMcmsProtocol.GetData(), mplMcmsProtocol.getDataLen());

	CM_HIGH_CPU_USAGE_S* pParams = (CM_HIGH_CPU_USAGE_S*)(seg.GetPtr());

	TRACEINTO << "Opcode:CM_HIGH_CPU_USAGE_IND" << ", BoardId:" << (WORD)pParams->board_id << ", IsExceed:" << (WORD)pParams->is_exceed;

	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	if (pSystemResources)
	{
		for (int i = 0; i < BOARDS_NUM; ++i)
		{
			if (pParams->board_id == i+1 || pParams->board_id == 0xFF)
			{
				CBoard* pBoard = pSystemResources->GetBoard(i+1); // board num is 1-based
				if (pBoard)
					pBoard->SetHighUsageCPUstate(pParams->is_exceed);
			}
		}
	}
	if (pSystemResources->GetBoard(pParams->board_id))
		pSystemResources->GetBoard(pParams->board_id)->SetHighUsageCPUstate(pParams->is_exceed);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnResourceGetAvcSvcAdditionalPartyRsrcReq(CSegment* pMsg)
{
	PASSERT_AND_RETURN(!pMsg);

	ALLOC_PARTY_REQ_PARAMS_S params;
	pMsg->Get((BYTE*)&params, sizeof(params));

	TRACEINTO << "Opcode:AVC_SVC_ADDITIONAL_PARTY_RSRC_REQ" << params;

	ALLOC_PARTY_IND_PARAMS_S result;
	memset(&result, 0, sizeof(result));

	m_pRsrcAlloc->ResponseWithExistingResources(&params, &result);

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)&result, sizeof(result));

	TRACEINTO << result;

	ResponedClientRequest(AVC_SVC_ADDITIONAL_PARTY_RSRC_IND, pSeg);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnUpdateInterfaceServices(CSegment* pSeg)
{
	DWORD serviceId;
	std::string interfaceName;
	*pSeg >> serviceId >> interfaceName;

	CResourceProcess* pResourceProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	PASSERT_AND_RETURN(!pResourceProcess);

	AllocatedUdpPorts& allocatedUdpPorts = pResourceProcess->GetAllocatedUdpPorts();

	std::pair<std::string, PortPerProcess>& service = allocatedUdpPorts[serviceId];
	service.first = interfaceName;

	StartTimer(RETRIEVE_OCCUPIED_UDP_PORTS_TIMER, 5 * SECOND);
}

////////////////////////////////////////////////////////////////////////////
void CResourceManager::OnRetrieveOccupiedUdpPortsTimer(CSegment* pMsg)
{
	TRACEINTO << "Opcode:RETRIEVE_OCCUPIED_UDP_PORTS_TIMER";

	CResourceProcess* pResourceProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	PASSERT_AND_RETURN(!pResourceProcess);

	AllocatedUdpPorts& allocatedUdpPorts = pResourceProcess->GetAllocatedUdpPorts();

	AllocatedUdpPorts::iterator _itr, _end = allocatedUdpPorts.end();
	for (_itr = allocatedUdpPorts.begin(); _itr != _end; ++_itr)
	{
		// Create allocated UDP ports file
		CConfigManagerApi api;
		api.GetUdpOccupiedPorts(_itr->second.first);

		PortPerProcess& portPerProcess = _itr->second.second;
		portPerProcess.clear();

		// Read allocated UDP ports file
		std::string fileName = "/tmp/GetUdpOccupiedPorts."+_itr->second.first+".out";
		std::ifstream infile(fileName.c_str(), std::ifstream::in);
		if (infile)
		{
			std::string line;
			while (getline(infile, line, '\n'))
			{
				std::size_t found = line.find(',');
				if (found != std::string::npos)
					portPerProcess.insert(std::make_pair(std::atol(line.substr(0, found).c_str()), line.substr(found+1)));
			}
		}
/*
		// Dump
		CPrettyTable<long, const char*> tbl("UDP port", "Owner");

		PortPerProcess::iterator _itrp, _endp = portPerProcess.end();
		for (_itrp = portPerProcess.begin(); _itrp != _endp; ++_itrp)
			tbl.Add(_itrp->first, _itrp->second.c_str());

		TRACEINTO << "ServiceId:" << _itr->first << ", InterfaceName:" << _itr->second.first << tbl.Get();
*/
	}

	DWORD timerValue = GetSystemCfgFlag<DWORD>(CFG_KEY_RETRIEVE_OCCUPIED_UDP_PORTS_TIMER);
	StartTimer(RETRIEVE_OCCUPIED_UDP_PORTS_TIMER, timerValue * SECOND);
}

#undef TRACEINTO
#define TRACEINTO TRACESTRFUNC(eLevelInfoNormal)
