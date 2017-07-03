//+========================================================================+
//                            IvrCntlLocal.cpp                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       IvrCntlLocal.cpp                                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 25.09.2000 |                                                      |
//+========================================================================+

#include "IVRCntlLocal.h"
#include "IVRService.h"
#include "Trace.h"
#include "DataTypes.h"
#include "CommConf.h"
#include "ConfPartyOpcodes.h"
#include "StateMachine.h"
#include "PObject.h"
#include "ConfPartyGlobals.h"
#include "StatusesGeneral.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "SysConfigKeys.h"
#include "IVRServiceList.h"
#include "ConfAppMngr.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "ApiStatuses.h"
#include "IpRtpFeccRoleToken.h"
#include "IVRAvMsgStruct.cpp"
#include "IpCommon.h"

#define STR_OPCODE_SIZE 64

PBEGIN_MESSAGE_MAP(CIvrCntlLocal)

  ONEVENT(IVR_END_OF_FEATURE,      ACTIVE,    CIvrCntlLocal::OnEndFeature)
  ONEVENT(IVR_END_OF_FEATURE,      NOTACTIVE, CIvrCntlLocal::OnEndFeatureNotActive)
  ONEVENT(DTMF_OPCODE_IDENT,       ACTIVE,    CIvrCntlLocal::OnDtmfOpcode)
  ONEVENT(DTMF_OPCODE_IDENT,       NOTACTIVE, CIvrCntlLocal::OnDtmfOpcodeNotActive)
  ONEVENT(IVR_EROR_FEATURE,        ACTIVE,    CIvrCntlLocal::OnErrorFeature)
  ONEVENT(IVR_RESET,               ACTIVE,    CIvrCntlLocal::OnIvrReset)
  ONEVENT(START_IVR,               ACTIVE,    CIvrCntlLocal::Start)
  ONEVENT(TIP_MASTER_STARTED_IVR,               ACTIVE,    CIvrCntlLocal::OnTIPMasterStartedIVR)
  ONEVENT(EXTENSION_DTMF_TIMER,    ACTIVE,    CIvrCntlLocal::OnTimerSendfExt)                 // Dtmf on connect party
  ONEVENT(DTMF_FWD_INVITOR_TOUT,   ACTIVE,    CIvrCntlLocal::onInvitePartyDTMFForwardingTout) // Dtmf on connect party
  ONEVENT(DTMF_IVR_DELAY_TOUT,     ACTIVE,    CIvrCntlLocal::OnDtmfOpcodeImp)                 // Dtmf on connect party
  ONEVENT(PARTY_INVITE_RESULT_IND, ACTIVE,    CIvrCntlLocal::OnPartyInviteResultInd)
  ONEVENT(PARTY_ON_HOLD_IND,       ANYCASE,    CIvrCntlLocal::OnPartyOnHoldInd)

PEND_MESSAGE_MAP(CIvrCntlLocal, CIvrCntl);

////////////////////////////////////////////////////////////////////////////
//                        CIvrCntlLocal
////////////////////////////////////////////////////////////////////////////
CIvrCntlLocal::CIvrCntlLocal(
	CTaskApp* pOwnerTask,
	const CIVRService& pIvrService,
	const char* pConfPassword,
	const char* pConfLeaderPassword,
	WORD startConfReqLeader,
	const DWORD dwMonitorConfId,
	const char* pNumericId,
	WORD IsPartyInCascadeEQ,
	eGatewayPartyType GatewayPartyType,
	BYTE bNoVideRsrcForVideoParty)
		: CIvrCntl(pOwnerTask, dwMonitorConfId, pNumericId)
{
	m_pConfPassword        = pConfPassword;
	m_leaderPassword       = 0;
	m_startConfReqLeader   = startConfReqLeader;

	if (NULL != pConfLeaderPassword)
	{
		int len = strlen(pConfLeaderPassword);
		m_leaderPassword = new char[len+1];
		strncpy(m_leaderPassword, pConfLeaderPassword, len+1);
	}

	m_pIvrService                = new CIVRService(pIvrService);
	m_language                   = 0;
	m_bIsLeaderFound             = 0;
	m_ChangePWDType              = 0;
	m_new_password[0]            = '\0';
	m_bIsToggleSelfMute          = 0;
	m_bIsToggleSecuredConference = 0;
	m_bIsToggleMuteAllButMe      = 0;
	m_dtmfForExt                 = 0;
	m_initExtensionTimer         = 0;
	m_pauseAtTheEnd              = 0;
	m_updateDuringConf           = 0;
	m_featureActionDuringConf    = 0;
	m_masterEndFeatures          = false;

	std::ostringstream msg;

	if (IsPartyInCascadeEQ)
	{
		msg << "IsPartyInCascadeEQ:CASCADE_LINK_IN_EQ";
		m_cascadeLink = CASCADE_LINK_IN_EQ;
	}
	else
	{
		msg << "IsPartyInCascadeEQ:CASCADE_LINK_DUMMY";
		m_cascadeLink = CASCADE_LINK_DUMMY;
	}

	m_bEnableExternalDB = 0;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_ENABLE_EXTERNAL_DB_ACCESS, m_bEnableExternalDB);
	msg << ", IsEnableExternalDB:" << (int)m_bEnableExternalDB;

	m_GatewayPartyType = GatewayPartyType;

	ResetIvrHoldParams();

	switch (m_GatewayPartyType)
	{
		case (eRegularPartyNoGateway): // regular party do nothing
			break;

		case (eNormalGWPartyType):
			msg << ", Conf is GW and party is regular dial out / dial in";
			break;


		case (eInitiatorNotInviter):
			msg<< ", Conf is GW and party is the Initiator (dial in in dial thorough)";
			break;

		case (eInviter):
			msg << ", Conf is GW and party is Video Inviter";
			break;

		default:
			break;
	}

	m_bNoVideRsrcForVideoParty = bNoVideRsrcForVideoParty;

	TRACEINTO << msg.str().c_str();
	m_turnOffIvrAfterMove = 0;
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CIvrCntlLocal::~CIvrCntlLocal() {

	POBJDELETE(m_pIvrService); // the DTMF has a pointer to this class
	RemoveLeaderPassword();

	if (m_initExtensionTimer)
	{
		DeleteAllTimers();

		if (m_dtmfForExt)
			delete[] m_dtmfForExt;
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode) {
	switch (opCode) {
	case TIMER: {
		TRACEINTO << "TIMER";
		break;
	}

	case AUD_DTMF_IND_VAL: { // go to DTMF Collector StateMachine
		if (m_pDtmfCollector)
			m_pDtmfCollector->HandleEvent(pMsg, msgLen, opCode);

		if (CASCADE_LINK_IN_EQ == m_cascadeLink)
			InformFeatureDTMF();
		break;
	}

	case FIRST_DTMF_WAS_DIALED:
	case DTMF_STRING_IDENT: { // go to Sub IVR Feature StateMachine
		if (m_pIvrSubGenSM)
			m_pIvrSubGenSM->HandleEvent(pMsg, msgLen, opCode);
		break;
	}

	case IVR_SUB_FEATURE_MESSG: {
		if (m_pIvrSubGenSM) {
			DWORD msgLen = 0;
			DWORD opCode = 0;
			*pMsg >> opCode;
			m_pIvrSubGenSM->HandleEvent(pMsg, msgLen, opCode);
		}
		break;
	}

	case XML_EVENT: {
		TRACEINTO << "XML_EVENT";
		if (m_pIvrSubGenSM) {
			DWORD msgLen = 0;
			DWORD opCode = 0;
			if (pMsg)
				*pMsg >> opCode;
			else
				opCode = EXT_DB_PWD_CONFIRM;

			m_pIvrSubGenSM->HandleEvent(pMsg, msgLen, opCode);
		}
		break;
	}

	case REJECT_PLC: {
		TRACEINTO <<  "REJECT_PLC";
		if (m_pIvrSubGenSM) {
			if (0 == strcmp("CIvrSubPLC", m_pIvrSubGenSM->NameOf()))
				((CIvrSubPLC*) m_pIvrSubGenSM)->RejectPLC();
			else
				TRACEINTO << "Failed, Not in CIvrSubPLC feature";
		}
		break;
	}

	case MASTER_END_FEATURES:
	{
		TRACEINTO <<"MASTER_END_FEATURES send END_WAIT to slaves. m_pIvrSubGenSM: " << m_pIvrSubGenSM;

		if (m_pIvrSubGenSM)
		{
			m_pIvrSubGenSM->HandleEvent(pMsg, msgLen, END_WAIT);
		}
		else
		{
			TRACEINTO << "set m_masterEndFeatures to true because master already finished the features";
			m_masterEndFeatures = true;
		}
		break;
	}

	default: { // all other messages
		DispatchEvent(opCode, pMsg);
		break;
	}
	} // switch
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::Create(CParty* pParty, COsQueue* pConfRcvMbx) {

	//CIvrCntl::Create(pParty, pConfRcvMbx);

	PASSERT_AND_RETURN(!pParty);
	PASSERT_AND_RETURN(!pConfRcvMbx);
	CIvrCntl::Create(pParty, pConfRcvMbx);

	CIVRLanguage* pIVRLanguage = m_pIvrService->GetFirstLanguage();
	if (pIVRLanguage)
		m_language = pIVRLanguage->GetLanguagePos();

	if (IsCascadeLinkPartyInConf()) // IsLeaderLinkParty() in MGC
		m_cascadeLink = CASCADE_LINK_IN_CONF;

	if (CASCADE_LINK_IN_CONF == m_cascadeLink) // if the party is a cascade link give it a leader permissions
		SetIsLeader(0, 0, 1); // set only in DTMF collector (not in DB)

	// set IVR DTMF table
	CDTMFCodeList* pCDTMFCodeList = NULL;
	if ((NULL == m_pIvrService->GetDTMFCodeList()) || 	// no DTMF table!
		(YES == IsRecordingLinkParty()) ||				// or - the party is a Recording Lin
		(m_MCUproductType==eProductTypeSoftMCUMfw))		// eProductTypeSoftMCUMfw
		pCDTMFCodeList = new CDTMFCodeList; // creates an empty dtmf table
	else
		pCDTMFCodeList = new CDTMFCodeList(*(m_pIvrService->GetDTMFCodeList()));

	if ((CASCADE_LINK_IN_CONF == m_cascadeLink) && (m_MCUproductType!=eProductTypeSoftMCUMfw))
		PrepareForwardDtmfOpcodesTable(pCDTMFCodeList); // update DTNF table for "Forward DTMF"

	m_pDtmfCollector->SetDtmfTable(pCDTMFCodeList);		// empty DTMF table for MFW product type

	if (m_pIvrService->m_bIsEntryQueueService)
		m_pParty->SetFromEntryQ(1);

	// Toggle Dtmf Self Mute
	CDTMFCode* DtmfToggleOn = NULL;
	CDTMFCode* DtmfToggleOff = NULL;
	DtmfToggleOn = pCDTMFCodeList->GetCurrentDTMFCodeByOpcode(DTMF_SELF_MUTE);
	DtmfToggleOff = pCDTMFCodeList->GetCurrentDTMFCodeByOpcode(DTMF_SELF_UNMUTE);
	if (DtmfToggleOn != NULL && DtmfToggleOff != NULL)
		if (!strncmp(DtmfToggleOn->GetDTMFStr(), DtmfToggleOff->GetDTMFStr(), DTMF_STRING_LEN))
			SetToggleSelfMute(1);

	// Toggle Secure DTMF
	DtmfToggleOn = pCDTMFCodeList->GetCurrentDTMFCodeByOpcode(DTMF_SECURE_CONF);
	DtmfToggleOff = pCDTMFCodeList->GetCurrentDTMFCodeByOpcode(
			DTMF_UNSECURE_CONF);
	if (DtmfToggleOn != NULL && DtmfToggleOff != NULL)
		if (!strncmp(DtmfToggleOn->GetDTMFStr(), DtmfToggleOff->GetDTMFStr(),
				DTMF_STRING_LEN))
			SetToggleSecuredConference(1);

	// Toggle Mute All DTMF
	DtmfToggleOn = pCDTMFCodeList->GetCurrentDTMFCodeByOpcode(
			DTMF_MUTE_ALL_BUT_X);
	DtmfToggleOff = pCDTMFCodeList->GetCurrentDTMFCodeByOpcode(
			DTMF_UNMUTE_ALL_BUT_X);
	if (DtmfToggleOn != NULL && DtmfToggleOff != NULL)
		if (!strncmp(DtmfToggleOn->GetDTMFStr(), DtmfToggleOff->GetDTMFStr(),
				DTMF_STRING_LEN))
			SetToggleMuteAllButMe(1);

	POBJDELETE(pCDTMFCodeList);

	// If this party is a cascade link
	if (CASCADE_LINK_IN_CONF == m_cascadeLink)
	{
		if (GetSystemCfgFlagInt<BOOL> (CFG_KEY_CASCADE_LINK_PLAY_TONE_ON_CONNECTION))
			BlipOnCascadeLink();

		//BRIDGE-14279
		if (m_pParty->IsIsdnGWCallCamesFromEQ() == FALSE)
			return;
	}

	if (YES == IsRecordingLinkParty())
		return; // all the IVR sub-features should be disabled - skip GetFeaturesToDo()

	if (YES == IsOperatorParty())
		return; // all the IVR sub-features should be disabled - skip GetFeaturesToDo()


	if (!IsIvrForSVCEnabled() && YES == IsRelayParty())
		return;                     // all the IVR sub-features should be disabled - skip GetFeaturesToDo()

	// determines the 'Features' to do
	GetFeaturesToDo();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::Start(CSegment* pParam)
{

	m_state = ACTIVE;

	PASSERT_AND_RETURN(!m_pParty);
	PASSERT_AND_RETURN(!m_pConfApi);

	if (NeedExtension())
		return;

	/*if (m_pParty->GetRecordingPort())
		return;*/


	if (!IsIvrForSVCEnabled() && YES == IsRelayParty())
	{
		TRACEINTOFUNC << "Skip IVR for relay party -> ResetFeaturesList";
		ResetFeaturesList();
	}

	if (m_bIsResumeAfterHold)
	{
		if (m_stageForHoldCallScenario == (DWORD)-1)
		{
			if (m_pParty->GetIsTipCall() && ( m_pParty->GetTipPartyType() > eTipMasterCenter))
			{
				TRACEINTO << "Resume IVR for TIP slave party -> will initiate stage IVR_STAGE_WAIT";
				SetIVRWaitStateForTIPSlave();
			}
			else
			{
				TRACEINTO << "Resume IVR for party -> No IVR feature list to resume - reset IVR list";
				memset(m_featuresList, 0, sizeof(m_featuresList));
			}
		}
		else
		{
			TRACEINTO << "Resume IVR for party -> ResumeFeaturesList";
			ResumeFeaturesList();
		}
		// resetting the "resume" flag
		m_bIsResumeAfterHold = FALSE;
	}
	CConfApi* pConfApi = m_pConfApi;

	if (CASCADE_LINK_IN_CONF == m_cascadeLink)
	pConfApi->SendCAMGeneralNotifyCommand(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_IS_CASCADE_LINK, (DWORD) 0);

	if (m_GatewayPartyType > eRegularPartyNoGateway)
	pConfApi->SendCAMGeneralNotifyCommand(m_pParty->GetPartyRsrcID(), eCAM_EVENT_PARTY_UPDATE_GW_TYPE, (DWORD) m_GatewayPartyType);

	// Finds the Next Feature
	m_pIvrSubGenSM = StartNextFeature();

	// end feature in case of no more features to do
	if (NULL == m_pIvrSubGenSM) // end of features
	{
		TRACEINTO << "PartyId:" << m_pParty->GetPartyId() << ", End of features for party";
		m_stage = 0;
		if ((DWORD)(-1) == m_monitorPartyId)
			TRACEINTO << "MonitorPartyId:" << m_monitorPartyId << ", Invalid monitor PartyId";
		else
		{
			// vngr-23410 - uograde leader sooner.
			// in GW calls - set the initiator as leader
			if (m_GatewayPartyType > eNormalGWPartyType)
				SetIsLeader(1, 1, 1);

			if (!m_pIvrService->m_bIsEntryQueueService)
			{
				// if not back from 'Modify passwords'
				if (!m_updateDuringConf) // IVR Entrance
					pConfApi->EndOfIVRSession(m_rsrcPartyId, m_pParty->GetIsLeader());
				else
				// IVR During Conf (Change-PW, Silence-It, Change-To-Leader, etc.)
					m_pConfApi->SendStopFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, m_featureActionDuringConf);
			}
			pConfApi->PartyPassedIVR_EntranceProcedures(m_pParty->GetName()); // olga VNGFE-1552 (approved by Anat G.)

			//IVR for TIP
			ETipPartyTypeAndPosition tipType = m_pParty->GetTipPartyType();
			TRACEINTO << "is tip call: " << m_pParty->GetIsTipCall() << " partyType: " << tipType;
			if ( m_pParty->GetIsTipCall() && tipType == eTipMasterCenter )
			{
				m_pParty->ForwardEventToTipSlaves(NULL, MASTER_END_FEATURES);
			}
		}

		// reset DTMF buffer
		if (m_pDtmfCollector)
		{
			m_pDtmfCollector->ResetDtmfBuffer();
			m_pDtmfCollector->EndIvrPartSession();
		}

		// remove leader password
		RemoveLeaderPassword();

		// reset saved IVR params
		ResetIvrHoldParams();
		return;
	}

	//////////////////////
	// BRIDGE-7575 - need to update confparty object manually, otherwise "Connected (IVR)" state will be updated too late-
	// only after the ivr feature is already in operation. it is needed for a state check for a fix for this issue

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);
	pConfParty->SetOrdinaryParty(STATUS_PARTY_IVR);
	//////////////////////

	OnStartIVR();

	m_featuresList[m_stage].bStarted = 1; // feature started

	if (m_stage == IVR_STAGE_WAIT)
	{
		m_featureActionDuringConf = eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER;
		m_pConfApi->SendStartFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, (DWORD) eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER);
	}

	// set feature params
	StartNewFeature();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::StartNewFeature()
{

	m_pIvrSubGenSM->SetLanguage(m_language);
	m_pIvrSubGenSM->SetIvrService(m_pIvrService);
	CIvrCntl::StartNewFeature();

}

//--------------------------------------------------------------------------
void CIvrCntlLocal::Restart()
{
	POBJDELETE(m_pIvrSubGenSM);
	m_state = NOTACTIVE; // state
	m_stage = 0; // current stage in the progress IVR process

	// features status
	for (int i = 0; i < NUM_OF_FEATURES; i++)
	{
		m_featuresList[i].bStarted = 0;
		m_featuresList[i].bFinished = 0;
	}

	Start();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnEndFeature(CSegment* pParam) {
	// Delete current Feature
	POBJDELETE(m_pIvrSubGenSM);

	// sets end status
	WORD status;
	*pParam >> status;

	if (m_stage >= NUM_OF_FEATURES) {
		int stage = m_stage;
		m_stage = 0; // into legal range
		PASSERT_AND_RETURN(stage)
;	}

	if (DISCONNECT_IVR_PROVIDER_STS == status) // for ivrProviderEQ timer
	{
		TRACEINTO << "Stage:" << IvrStageToString(m_stage) << "(" << m_stage << "), NextFeature:" << IvrStageToString(m_startNextFeature) << "(" << m_startNextFeature << ")";
		m_featuresList[m_stage++].bFinished = 1; // feature end (1: OK, 2: ERROR)
		m_pConfApi->PartyDisConnect(PASSWORD_FAILURE, m_pParty);
	}
	else if (0 == status) // feature ends OK
	{
		TRACEINTO << "Stage:" << IvrStageToString(m_stage) << "(" << m_stage << "), Failed with error code:" << status;
		m_featuresList[m_stage++].bFinished = 1; // feature end (1: OK, 2: ERROR)
		if (m_startNextFeature)
		{
			m_startNextFeature = FALSE; // for the next feature
			Start();
		}
	}
	else // ends with an error or 'welcome wait'
	{
		TRACEINTO << "CIvrCntlLocal::OnEndFeature - " << IvrStageToString(m_stage) << "(" << m_stage << "), Failed with error code:" << status;
		m_featuresList[m_stage++].bFinished = 2; // feature end (1: OK, 2: ERROR)
		m_pConfApi->PartyDisConnect(PASSWORD_FAILURE, m_pParty);
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnEndFeatureNotActive(CSegment* pParam)
{
	// NULL function - nothing to do
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnErrorFeature(CSegment* pParam)
{
	WORD errorCode;
	*pParam >> (WORD &) errorCode;

	// set previous feature finished status
	m_featuresList[m_stage].bFinished = 2; // feature end (1: OK, 2: ERROR)

	CConfApi* pConfApi = m_pConfApi;

	int result = 0;

	TRACEINTO << IvrStageToString(m_stage) << "(" << m_stage << ")";

	if (m_stage == IVR_STAGE_CONF_PASSWORD)
	{
		const CIVRConfPasswordFeature* pConfPasswordFeature = m_pIvrService->GetConfPasswordFeature();
		PASSERT_AND_RETURN(!pConfPasswordFeature);
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
		PASSERT_AND_RETURN(!pCommConf);

		CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
		PASSERT_AND_RETURN(!pConfParty);

		BYTE bIvrEntryType = 0;

		// If the entry type is REQUEST_DIGIT and the party didn't press any dtmf, we disable
		// the operator assistance for him (even if exists) since it answering machine.
		if (pConfParty->GetConnectionTypeOper() == DIAL_OUT || pConfParty->GetConnectionTypeOper() == DIRECT)
		bIvrEntryType = pConfPasswordFeature->GetDialOutEntryPassword();
		else
		bIvrEntryType = pConfPasswordFeature->GetDialInEntryPassword();

		if (bIvrEntryType == REQUEST_DIGIT)
		{
			TRACEINTO << "REQUEST_DIGIT do not activate Operator assistance (answering machine) [operator_assistance_trace]";
			result = 1; // means we won't call OnOperatorHelp
		}
	}

	WORD errorNum = 1;
	BOOL bOperatorAssistanceThroughRequestOnly = 0;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_OPERATOR_ASSISTANCE_THROUGH_REQUEST_ONLY, bOperatorAssistanceThroughRequestOnly);

	if (result == 0)
	{
		switch (m_stage)
		{
			case IVR_STAGE_CONF_PASSWORD:
			{
				result = (bOperatorAssistanceThroughRequestOnly) ? 1 : OperatorAssistance(pConfApi, IVR_EVENT_WAIT_FOR_OPER_ON_CONF_PWD_FAIL);
				break;
			}

			case IVR_STAGE_CONF_LEADER:
			{
				result = (bOperatorAssistanceThroughRequestOnly) ? 1 : OperatorAssistance(pConfApi, IVR_EVENT_WAIT_FOR_OPER_ON_CHAIR_PWD_FAIL);
				break;
			}

			case IVR_STAGE_NUMERIC_CONF_ID:
			{
				result = (bOperatorAssistanceThroughRequestOnly) ? 1 : OperatorAssistance(pConfApi, IVR_EVENT_WAIT_FOR_OPER_ON_NID_FAIL);
				break;
			}

			case IVR_STAGE_LOCK_SECURE:
			{
				result = (m_pParty->GetMonitorConfId() != (DWORD)-1) ? 1 : 0;
				break;
			}

			// case IVR_STAGE_CHANGE_TO_LEADER - After several failures of
			// "Change to Leader" we simply want to return to the conf without any changes
			// and without an operator assistance.
			case IVR_STAGE_CHANGE_TO_LEADER:
			case IVR_STAGE_CHANGE_PWDS_MENU:
			case IVR_STAGE_CHANGE_CONF_PWD:
			case IVR_STAGE_CHANGE_LEADER_PWD:
			{
				POBJDELETE(m_pIvrSubGenSM); // Delete currect Feature
				m_startNextFeature = FALSE;
				for (int i = 1; i < NUM_OF_FEATURES; i++)
				m_featuresList[i].bToDo = 0; // removes all other features

				Start();
				result = 0;
				break;
			}

			default:
			result = 1;
			break;
		}
	}

	// on feature error (operator assistance)
	if (0 != result)
	{
		CSegment seg;
		seg << (WORD) errorNum;
		OnEndFeature(&seg);
	}
}

//--------------------------------------------------------------------------
WORD CIvrCntlLocal::IsIvrFeatureUseDtmFforwarding(WORD opcode)
{
	if (opcode == DTMF_CONF_TERMINATE ||
		opcode == DTMF_MUTE_ALL_BUT_X ||
		opcode == DTMF_UNMUTE_ALL_BUT_X ||
		opcode == DTMF_SECURE_CONF ||
		opcode == DTMF_UNSECURE_CONF ||
		opcode == DTMF_MUTE_INCOMING_PARTIES ||
		opcode == DTMF_UNMUTE_INCOMING_PARTIES)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnDtmfOpcode(CSegment* pParam) {
	WORD opcode;
	CSegment* pParamCopyTmp = new CSegment;

	*pParamCopyTmp = *pParam;

	*pParamCopyTmp >> (WORD&) opcode;

	TRACEINTO << "Opcode:" << DtmfCodeToString(opcode)<< "(" << opcode << ")";

	if (m_cascadeLink != CASCADE_LINK_DUMMY)
	{
		if (!IsIvrFeatureUseDtmFforwarding(opcode))
		{
			TRACEINTO << "This opcode is ignored for cascade link.";
			POBJDELETE(pParamCopyTmp);

			return;
		}
	}

	switch (opcode) {
	case DTMF_INVITE_PARTY:
	case DTMF_SELF_MUTE:
	case DTMF_SELF_UNMUTE: {
		// VNGR-23452 - the first time the IVR is played, it is cuted because EP is in mute for about half second
		// so we delayed the beginning of playing the IVR according to CFG_KEY_DTMF_IVR_OPERATION_DELAY (default 1 sec)
		// we do it for the invite & mute/un-mute
		DWORD dDelayTicks = 2;
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(
				CFG_KEY_DTMF_IVR_OPERATION_DELAY, dDelayTicks);

		CSegment* pParamCopy = new CSegment;
		*pParamCopy << (WORD) opcode;
		StartTimer(DTMF_IVR_DELAY_TOUT, dDelayTicks, pParamCopy);
		break;
	}

	default:
		OnDtmfOpcodeImp(pParam);
		break;
	}

	POBJDELETE(pParamCopyTmp);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnPartyInviteResultInd(CSegment* pParam) {
	eGeneralDisconnectionCause disconnectionCause;
	BOOL bRedial, bIsGwInvite;

	*pParam >> (BYTE &) disconnectionCause;
	*pParam >> bRedial;
	*pParam >> bIsGwInvite;

	if (disconnectionCause == eRemoteWrongNumber)
	{
		if (bRedial)
		  ReInviteParty(m_pConfApi, bIsGwInvite);
		else
		  PlayWrongNumberMsg(m_pConfApi);
	}
	else if (disconnectionCause == eRemoteBusy)
		PlayBusyMsg(m_pConfApi);
	else if (disconnectionCause == eRemoteNoAnswer)
		PlayNoAnswerMsg(m_pConfApi);
	else
		PlayNoAnswerMsg(m_pConfApi);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnDtmfOpcodeImp(CSegment* pParam) {
	WORD opcode;
	*pParam >> (WORD &) opcode;

	CConfApi* pConfApi = m_pConfApi;

	switch (opcode)
	{
		case DTMF_CONF_TERMINATE:
		{
			SendDtmfForwarding(DTMF_CONF_TERMINATE);
			OnConfTerminate(pConfApi, IsEnableDtmfForwarding());
			break;
		}

		case DTMF_SELF_MUTE:
		{
			MuteParty(pConfApi, m_pParty);
			break;
		}

		case DTMF_SELF_UNMUTE:
		{
			UnMuteParty(pConfApi, m_pParty);
			break;
		}

		case DTMF_INC_SELF_VOLUME:
		{
			IncreaseVolume(pConfApi);
			break;
		}

		case DTMF_DEC_SELF_VOLUME:
		{
			DecreaseVolume(pConfApi);
			break;
		}

		case DTMF_MUTE_ALL_BUT_X:
		{
			MuteAllButX(pConfApi, 1);
			SendDtmfForwarding(DTMF_MUTE_ALL_BUT_X);
			break;
		}

		case DTMF_UNMUTE_ALL_BUT_X:
		{
			MuteAllButX(pConfApi, 0);
			SendDtmfForwarding(DTMF_UNMUTE_ALL_BUT_X);
			break;
		}

		case DTMF_OVERRIDE_MUTE_ALL:
		{
			OverrideMuteAll(pConfApi);
			break;
		}

		case DTMF_MUTE_INCOMING_PARTIES:
		{
			SendDtmfForwarding(DTMF_MUTE_INCOMING_PARTIES);
			pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId,
				EVENT_CONF_REQUEST, eCAM_EVENT_CONF_MUTE_INCOMING_PARTIES, 1);
			break;
		}

		case DTMF_UNMUTE_INCOMING_PARTIES:
		{
			SendDtmfForwarding(DTMF_UNMUTE_INCOMING_PARTIES);
			pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId,
				EVENT_CONF_REQUEST, eCAM_EVENT_CONF_MUTE_INCOMING_PARTIES, 0);
			break;
		}

		case DTMF_START_DIALOUT:
		{
			if (!::GetpConfDB()->IsConfSecured(m_dwMonitorConfId))
				StartDialOut();
			break;
		}

		case DTMF_PLAY_MENU:
		{
			SendGeneralMessageMENU();
			break;
		}

		case DTMF_CHANGE_PASSWORD:
		{
			ChangePwds();
			break;
		}

		case DTMF_ROLL_CALL_REVIEW_NAMES:
		{
			ReviewNames(pConfApi);
			break;
		}

		case DTMF_ROLL_CALL_STOP_REVIEW_NAMES:
		{
			StopNamesReviewing(pConfApi);
			break;
		}

		case DTMF_ENABLE_ROLL_CALL: // enables Roll-Call announce
		{
			pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId,EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_ROLL_CALL_YESNO, 1);
			break;
		}

		case DTMF_DISABLE_ROLL_CALL: // disables Roll-Call announce
		{
			pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId,EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_ROLL_CALL_YESNO, 0);
			break;
		}

		case DTMF_INC_LISTEN_VOLUME:
		{
			IncreaseVolume(pConfApi, 1);
			break;
		}

		case DTMF_DEC_LISTEN_VOLUME:
		{
			DecreaseVolume(pConfApi, 1);
			break;
		}

		case DTMF_DUMP_TABLE:
		{
			if (m_pDtmfCollector)
				m_pDtmfCollector->DumpDtmfTable();
			break;
		}

		case DTMF_CHANGE_TO_LEADER:
		{
			ChangeToLeader();
			break;
		}

		case DTMF_START_VC:
		{
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
			PASSERT_AND_RETURN(!pCommConf);
			if (pCommConf->GetVideoSession() == VIDEO_SESSION_COP)
			{
				if (m_pParty->GetIsLeader())
					pConfApi->PCMClickAndView(m_rsrcPartyId);
			}
			else // not COP
			{
				if (!(pCommConf->GetManageTelepresenceLayoutInternaly()) )
				{
					Start_PLC();
				}
				else
				{
					TRACEINTO << "Can't start feature because layout is managed internally in telepresence room switch mode";
				}
			}
			break;
		}

		case DTMF_START_RESUME_RECORDING:
		{
			DoRecordingAction(pConfApi, eCAM_EVENT_CONF_START_RESUME_RECORDING);
			break;
		}

		case DTMF_PAUSE_RECORDING:
		{
			DoRecordingAction(pConfApi, eCAM_EVENT_CONF_PAUSE_RECORDING);
			break;
		}

		case DTMF_STOP_RECORDING:
		{
			DoRecordingAction(pConfApi, eCAM_EVENT_CONF_STOP_RECORDING);
			break;
		}

		case DTMF_SECURE_CONF:
		{
			SecureConf(pConfApi, 1);
			SendDtmfForwarding(DTMF_SECURE_CONF);
			break;
		}

		case DTMF_UNSECURE_CONF:
		{
			SecureConf(pConfApi, 0);
			SendDtmfForwarding(DTMF_UNSECURE_CONF);
			break;
		}

		case DTMF_SHOW_PARTICIPANTS:
		{
			ShowParticipants(pConfApi);
			break;
		}

		case DTMF_REQUEST_TO_SPEAK:
		{
			RequestToSpeak(pConfApi);
			break;
		}

		case DTMF_OPER_ASSISTANCE_PRIVATE:
		{
			OperatorAssistance(pConfApi, IVR_EVENT_WAIT_FOR_OPER_ON_PARTY_PRIVATE_REQ);
			break;
		}

		case DTMF_OPER_ASSISTANCE_PUBLIC:
		{
			OperatorAssistance(pConfApi, IVR_EVENT_WAIT_FOR_OPER_ON_PARTY_PUBLIC_REQ);
			break;
		}

		case DTMF_SHOW_GATHERING:
		{
			ShowGathering();
			break;
		}

		case DTMF_START_PCM:
		{
			if (m_pParty->GetIsTipCall())
				TRACEINTO << "CIvrCntl::OnDtmfOpcodeImp - PCM is not supported in a TIP call!\n";
			else
			   StartPcm();
			break;
		}

		case DTMF_INVITE_PARTY:
		{
			InviteParty(pConfApi /*, TRUE*/);
			break;
		}

		case DTMF_DISCONNECT_INVITED_PARTICIPANT:
		{
			pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_CONF_REQUEST,eCAM_EVENT_CONF_DISCONNECT_INVITED_PARTICIPANT, 0);
			break;
		}

		// not implemented in first version
		case DTMF_LOCK_CONF:
		case DTMF_UNLOCK_CONF:
		case DTMF_START_ONHOLD:
		case DTMF_STOP_ONHOLD:
		{
			break;
		}

		case DTMF_START_VENUS:
		{
			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
			PASSERT_AND_RETURN(!pCommConf);

			StartVenus();
			break;
		}

	}//Switch
}

//--------------------------------------------------------------------------

void CIvrCntlLocal::OnDtmfOpcodeNotActive(CSegment* pParam)
{
	TRACESTR(eLevelError) <<  "Failed, DTMF opcode before IVR started";
}

//--------------------------------------------------------------------------
CIvrSubBaseSM* CIvrCntlLocal::StartNextFeature()
{
	while (TRUE) {
		if (m_stage >= NUM_OF_FEATURES) {
			m_stage = 0;
			return NULL;
		}

		if (m_featuresList[m_stage].bToDo)
		{
			TRACEINTO << "Stage:" << IvrStageToString(m_stage) << "(" << m_stage << ")";

			switch (m_stage) {
			case IVR_STAGE_GENERAL_WELCOME: // welcome
				return (CIvrSubBaseSM*) new CIvrSubWelcome(MSG_WELCOME_TYPE);

			case IVR_STAGE_NO_VIDEO_RSRC: // No video resources
				return (CIvrSubBaseSM*) new CIvrSubNoVideoResources(
						IVR_FEATURE_GENERAL);

			case IVR_STAGE_NUMERIC_CONF_ID: // Numeric confernce id
				return (CIvrSubBaseSM*) new CIvrSubNumericConferenceId(
						m_pIvrService->GetNumericConferenceIdFeature(),
						m_pNumericConferenceId, m_bEnableExternalDB);

			case IVR_STAGE_CONF_PASSWORD: // conference password
				return (CIvrSubBaseSM*) new CIvrSubConfPassword(
						m_pIvrService->GetConfPasswordFeature(),
						m_pConfPassword, m_bEnableExternalDB);

			case IVR_STAGE_CONF_LEADER: // Leader
			{
				if (m_bIsLeaderFound) // already found (at password stage), skip to the next feature!
					break;

				return (CIvrSubBaseSM*) new CIvrSubLeader(
						m_pIvrService->GetConfLeaderFeature(),
						m_bEnableExternalDB);
			}

			case IVR_STAGE_BILLING_CODE: // Billing
			{
				if (m_bIsLeaderFound)
					if (m_pIvrService->GetConfLeaderFeature()->GetIsBillingCode())
						return (CIvrSubBaseSM*) new CIvrSubBillingCode(
								m_pIvrService->GetBillingCodeFeature());
				break;
			}

			case IVR_STAGE_CONF_WELCOME: // user ID
				return (CIvrSubBaseSM*) new CIvrSubWelcome(MSG_ENTRANCE_TYPE);

			case IVR_STAGE_LOCK_SECURE:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(GEN_TP_LOCK_SECURE);

			case IVR_STAGE_CHANGE_PWDS_MENU:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_CHANGE_PWDS_MENU);

			case IVR_STAGE_CHANGE_CONF_PWD:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_CHANGE_CONF_PWD);

			case IVR_STAGE_CHANGE_LEADER_PWD:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_CHANGE_LEADER_PWD);

			case IVR_STAGE_CHANGE_PWDS_CONFIRM:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_CHANGE_PWDS_CONFIRM);

			case IVR_STAGE_CHANGE_PWDS_OK:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_CHANGE_PWDS_OK);

			case IVR_STAGE_CHANGE_PWDS_INVALID:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_CHANGE_PWDS_INVALID);

			case IVR_STAGE_ROLL_CALL:
				return (CIvrSubBaseSM*) new CIvrSubRollCall(
						m_pIvrService->GetRollCallFeature());

			case IVR_STAGE_MAX_PARTICIPANTS:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_MAX_PARTICIPANTS);

			case IVR_STAGE_RECORDING_IN_PROGRESS:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_RECORDING_IN_PROGRESS);

			case IVR_STAGE_RECORDING_FAILED:
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_RECORDING_FAILED);

			case IVR_STAGE_CASCADE_MCU_PARTY:
				return (CIvrSubBaseSM*) new CIvrSubCascadeLink();

			case IVR_STAGE_VIDEO_INVITE:
				return (CIvrSubBaseSM*) new CIvrSubInvite();

			case IVR_STAGE_INVITE_PARTY: // user ID
				return (CIvrSubBaseSM*) new CIvrSubGeneral(GEN_TP_INVITE_PARTY);

			case IVR_STAGE_GW_REINVITE_PARTY: // user ID
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_GW_REINVITE_PARTY);

			case IVR_STAGE_DTMF_REINVITE_PARTY: // user ID
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_DTMF_REINVITE_PARTY);

			case IVR_STAGE_PLAY_BUSY_MSG: // user ID
				return (CIvrSubBaseSM*) new CIvrSubGeneral(GEN_TP_PLAY_BUSY_MSG);

			case IVR_STAGE_PLAY_NOANSWER_MSG: // user ID
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_PLAY_NOANSWER_MSG);

			case IVR_STAGE_PLAY_WRONG_NUMBER_MSG: // user ID
				return (CIvrSubBaseSM*) new CIvrSubGeneral(
						GEN_TP_PLAY_WRONG_NUMBER_MSG);

			case IVR_STAGE_WAIT:   // IVR for TIP
				return (CIvrSubBaseSM*) new CIvrSubWait();

			default: // error
				return NULL;
			} // switch
		}
		// step to next feature
		m_stage++;
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetStartNextFeature()
{
	m_startNextFeature = TRUE; // after welcome, starts the next feature without waiting to a trigger from the party
}

void CIvrCntlLocal::GetFeaturesToDoMfwRestrictions()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	bool skip_ivr = true;

	ResetFeaturesList();

	if (!pCommConf->GetEntryQ())
	{
		TRACEINTO << "MFW product type, no features to do in MR";
		return;
	}
//	if (!pCommConf->isIvrProviderEQ())
//	{
//		TRACEINTO << "MFW product type, only 'IVR provider EQ' allowed. no features to do.";
//		return;
//	}

	// max participants
	DWORD confMonitorId = m_pParty->GetMonitorConfId();
	if (confMonitorId != (DWORD) -1)
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
		if (pConfParty)
		{
			WORD confMaxParts = pCommConf->GetMaxParties(); // max participants in conf
			if ((confMaxParts < pCommConf->GetNumParties())
					&& (confMaxParts != 0x255) && (DIAL_IN == pConfParty->GetConnectionType()))
			{
				for (int i = 1; i < NUM_OF_FEATURES; i++)
					m_featuresList[i].bToDo = 0; // removes all other features

				TRACEINTO << "numParties:" << pCommConf->GetNumParties() << ", maxParties:" << confMaxParts << " - Conference has reached max participants";
				PASSERT(pCommConf->GetNumParties());
				m_featuresList[IVR_STAGE_MAX_PARTICIPANTS].bToDo = 1; // conf has reached max participants
				return;
			}
		}
	}
	// welcome
	if (NULL != m_pIvrService->GetWelcomeFeature())
	{
		if (YES == (m_pIvrService->GetWelcomeFeature())->GetEnableDisable())
			m_featuresList[IVR_STAGE_GENERAL_WELCOME].bToDo = 1;
	}

	// Numeric-ID (NID)
	int bCurrentEQ = m_pIvrService->m_bIsEntryQueueService; // the current service is EQ
	if (bCurrentEQ)
	{
		if (NULL != m_pIvrService->GetNumericConferenceIdFeature())
		{
			m_featuresList[IVR_STAGE_NUMERIC_CONF_ID].bToDo = 1; // Numeric conference id
			m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 0; // Disable the conference password feature in EQ from V6
		}
	}

}
//--------------------------------------------------------------------------
void CIvrCntlLocal::GetFeaturesToDo()
{
	PASSERT_AND_RETURN(!m_pParty);	// pointer should be ok

	std::ostringstream msg;
	TRACEINTOFUNC << "PartyId:" << m_pParty->GetPartyId() << " Name:" << m_pParty->GetName();
	// EE-462 enable IVR for MFW
	if (m_MCUproductType==eProductTypeSoftMCUMfw)
	{
		GetFeaturesToDoMfwRestrictions();
		return;
	}
	//BRIDGE-14279
	if (CASCADE_LINK_IN_CONF == m_cascadeLink && m_pParty->IsIsdnGWCallCamesFromEQ() == FALSE)
	{
		TRACEINTO << "PartyId:" << m_pParty->GetPartyId() << " - Party is a Cascade Link in Conference, so no features to do";
		return;
	}

	if (CASCADE_LINK_IN_EQ == m_cascadeLink) // Cascade EQ
	{
		m_featuresList[IVR_STAGE_CASCADE_MCU_PARTY].bToDo = 1;
		TRACEINTO << "PartyId:" << m_pParty->GetPartyId() << " - Party is a Cascade Link in EQ, so no features to do";
		return;
	}

	//IVR for TIP
	ETipPartyTypeAndPosition tipType = m_pParty->GetTipPartyType();
	TRACEINTO << "is tip call: " << m_pParty->GetIsTipCall() << " partyType: " << tipType;
	if ( m_pParty->GetIsTipCall() && ( tipType > eTipMasterCenter ) )
	{
		m_featuresList[IVR_STAGE_WAIT].bToDo = 1;
		TRACEINTO << "PartyId:" << m_pParty->GetPartyId() << " - Party is a slave in TIP call, so no features to do";
		return;
	}

	// password (eanable it also after move)
	BYTE bIvrEntryType = 0;
	int isConfPassword = 0;
	BYTE bIsExtDbPW = 0;
	if (m_bEnableExternalDB)
	{
		bIsExtDbPW = (BYTE) m_pIvrService->GetIvrExternalDB();
		if (bIsExtDbPW == IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD)
		{
			TRACEINTO << "External DB participant password can be used";
		}
	}
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	if (NULL != m_pConfPassword || (bIsExtDbPW == IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD))
	{
		if ((m_pConfPassword && 0 != m_pConfPassword[0]) || (bIsExtDbPW == IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD))
		{
			const CIVRConfPasswordFeature* ConfPasswordFeature = m_pIvrService->GetConfPasswordFeature();

			if (NULL != ConfPasswordFeature)
			{
				if (YES == (ConfPasswordFeature->GetEnableDisable()))
				{

					if (pCommConf)
					{
						CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
						TRACECOND_AND_RETURN(!pConfParty, "NULL pointer");

						if (pConfParty->GetConnectionTypeOper() == DIAL_OUT || pConfParty->GetConnectionTypeOper() == DIRECT)
						{
							bIvrEntryType = ConfPasswordFeature->GetDialOutEntryPassword();
							switch (bIvrEntryType)
							{
								case (REQUEST_PASSWORD):
								{
									m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 1; // conference password entry
									isConfPassword = 1;
									break;
								}

								case (REQUEST_DIGIT):
								{
									m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 1; // Any digit entry
									break;
								}

								case (NO_REQUEST):
								{
									m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 0; // Direct entry
									break;
								}
							} // switch
						}
						else if (pConfParty->GetConnectionTypeOper() == DIAL_IN)
						{
							bIvrEntryType = ConfPasswordFeature->GetDialInEntryPassword();
							switch (bIvrEntryType)
							{
								case (REQUEST_PASSWORD):
								{
									m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 1; // conference password entry
									isConfPassword = 1;
									break;
								}

								case (REQUEST_DIGIT):
								{
									m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 1; // Any digit entry
									break;
								}

								case (NO_REQUEST):
								{
									m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 0; // Direct entry
									break;
								}
							} // switch
						}

						//VNGFE-5706/VNGR-25885: Change behavior back (No pwd after move!)
						//VNGFE-5900: No pwd after move party from operator conf back to home conference.
						if((pConfParty->GetMoveType() >= eMoveDefault) && (pConfParty->GetMoveType() <= eMoveBack))
						{
							TRACEINTO << "MoveType=" << pConfParty->GetMoveType() << ", Turn OFF conference password.";
							m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 0;
						}
					}
				}
			}
		}
	}

	if (m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo)
	{
		TRACEINTO << "Conference password is ON";
	}

	if (m_turnOffIvrAfterMove)
	{
		TRACEINTO << "Party is after move to Operator Conference or between regular CP conferences, so no features to do";
		OFF(m_turnOffIvrAfterMove);
		return;
	}

	// Secure state
	// if conf is secured or (locked and  pa no need in IVR features.
	if (pCommConf)
	{
		if (pCommConf->IsConfSecured())
		{
			for (int i = 1; i < NUM_OF_FEATURES; i++)
			{
				m_featuresList[i].bToDo = 0;
			}

			m_featuresList[IVR_STAGE_LOCK_SECURE].bToDo = 1; // Conf is Locked.
			return;
		}
	}

	if (IsRecordingLinkParty()) // Cascade EQ
	{
		m_featuresList[IVR_STAGE_CASCADE_MCU_PARTY].bToDo = 1;
		return;
	}

	// GW
	if (m_GatewayPartyType > eNormalGWPartyType)
	{
		// for the session initiator play the roll call message first (if needed)
		if (NULL != m_pIvrService->GetRollCallFeature())
		{
			if (m_pIvrService->GetRollCallFeature()->GetEnableDisable())
			m_featuresList[IVR_STAGE_ROLL_CALL].bToDo = 1; // Roll Call
		}

		// in case the party is inviter start the video invite feature
		if (m_GatewayPartyType == eInviter)
		{
			TRACEINTO << "Party is video inviter in GW session";
			m_featuresList[IVR_STAGE_VIDEO_INVITE].bToDo = 1;
		}

		// don't start any other faeture for initiator
		return;
	}

	// max participants
	DWORD confMonitorId = m_pParty->GetMonitorConfId();
	if (confMonitorId != (DWORD) -1)
	{
		const CCommConf* pCurCommConf = ::GetpConfDB()->GetCurrentConf(confMonitorId);
		if (NULL != pCurCommConf)
		{
			CConfParty* pConfParty = pCurCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
			if (pConfParty)
			{
				WORD confMaxParts = pCurCommConf->GetMaxParties(); // max participants in conf
				if ((confMaxParts < pCurCommConf->GetNumParties())
						&& (confMaxParts != 0x255) && (DIAL_IN == pConfParty->GetConnectionType()))
				{
					for (int i = 1; i < NUM_OF_FEATURES; i++)
					m_featuresList[i].bToDo = 0; // removes all other features

					TRACEINTO << "numParties:" << pCurCommConf->GetNumParties() << ", maxParties:" << confMaxParts << " - Conference has reached max participants";
					PASSERT(pCurCommConf->GetNumParties());
					m_featuresList[IVR_STAGE_MAX_PARTICIPANTS].bToDo = 1; // conf has reached max participants
					return;
				}
			}
		}
	}

	// invite party
	if (NULL != m_pIvrService->GetInvitePartyFeature())
	{
		if (YES == (m_pIvrService->GetInvitePartyFeature())->GetEnableDisable())
			m_featuresList[IVR_STAGE_INVITE_PARTY_ADDRESS].bToDo = 1;
	}

	// welcome
	if (NULL != m_pIvrService->GetWelcomeFeature())
	{
		if (YES == (m_pIvrService->GetWelcomeFeature())->GetEnableDisable())
			m_featuresList[IVR_STAGE_GENERAL_WELCOME].bToDo = 1;
	}

	// Numeric-ID (NID)
	int bCurrentEQ = m_pIvrService->m_bIsEntryQueueService; // the current service is EQ
	if (bCurrentEQ)
	{
		if (NULL != m_pIvrService->GetNumericConferenceIdFeature())
		{
			m_featuresList[IVR_STAGE_NUMERIC_CONF_ID].bToDo = 1; // Numeric conference id
			m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 0; // Disable the conference password feature in EQ from V6
		}
	}

	// leader
	if (!bCurrentEQ) // only if this is not an E.Q.
	{
		if (!m_bIsLeaderFound) // can be leader from EQ
		{
			const CIVRConfLeaderFeature* leaderF = m_pIvrService->GetConfLeaderFeature();
			if (NULL != leaderF)
			{
				if ((NULL != m_leaderPassword) && (0 != m_leaderPassword[0]) // leader pwd is set in the conf
						&& (YES == leaderF->GetEnableDisable())) // leader pwd feature is enabled in the service
				{
					// Check the conditions for skipping leader pwd feature in "Same Time" environment (IBM)
					BOOL bSkipPrompt;
					CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_SKIP_PROMPT, bSkipPrompt);

					CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
					if (pCommConf)
					{
						CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
						TRACECOND_AND_RETURN(!pConfParty, "NULL pointer");

						if (!bSkipPrompt || (DIAL_OUT != pConfParty->GetConnectionTypeOper()))
						{
							// In case the conference password is set to REQUEST_DIGIT or NO_REQUEST and use chairperson password as conf password
							// is also set, we enable the participant to connect as chairperson.
							int EnableToConnectAsChair = 0;
							BYTE confPwAsLeaderPw =	leaderF->GetConfPwAsLeaderPw();
							if (confPwAsLeaderPw && bIvrEntryType != REQUEST_PASSWORD)
								EnableToConnectAsChair = 1;

							if ((!confPwAsLeaderPw || EnableToConnectAsChair || !isConfPassword))
							m_featuresList[IVR_STAGE_CONF_LEADER].bToDo = 1; // leader

						}
					}
				}
			}
		}
	}

	// if leader (from Ad-hoc) then not pass Conf-PW or Chair-PW
	if (m_pParty->GetIsLeader() && (pCommConf->GetAppointmentId() == NULL))
	{
		m_featuresList[IVR_STAGE_CONF_LEADER].bToDo = 0; // Ad hoc leader
		m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo = 0;
	}

	// billing code
	if (NULL != m_leaderPassword)
	{
		if (m_pIvrService->GetConfLeaderFeature() != NULL)
		if (m_pIvrService->GetConfLeaderFeature()->GetIsBillingCode())
		if (NULL != m_pIvrService->GetBillingCodeFeature())
		m_featuresList[IVR_STAGE_BILLING_CODE].bToDo = 1;
	}

	// Change Passwords Feature set
	m_featuresList[IVR_STAGE_CHANGE_PWDS_MENU].bToDo = 0;
	m_featuresList[IVR_STAGE_CHANGE_CONF_PWD].bToDo = 0;
	m_featuresList[IVR_STAGE_CHANGE_LEADER_PWD].bToDo = 0;
	m_featuresList[IVR_STAGE_CHANGE_PWDS_CONFIRM].bToDo = 0;
	m_featuresList[IVR_STAGE_CHANGE_PWDS_OK].bToDo = 0;
	m_featuresList[IVR_STAGE_CHANGE_PWDS_INVALID].bToDo = 0;

	// Roll call Feature
	m_featuresList[IVR_STAGE_ROLL_CALL].bToDo = 0;

	BOOL bIvrIsRollCallTone = FALSE;
	const CIVRRollCallFeature* pRollCallFeature = m_pIvrService->GetRollCallFeature();
	if (pRollCallFeature != NULL)
	{
		bIvrIsRollCallTone = pRollCallFeature->GetUseTones();
	}

	if (bIvrIsRollCallTone)
		TRACEINTO << "PartyId:" << m_pParty->GetPartyId() << " - Use_Entry_Tones is set to YES in IVR service, so no roll call recording";

	if (NULL != m_pIvrService->GetRollCallFeature() && !bIvrIsRollCallTone)
	{
		if (m_pIvrService->GetRollCallFeature()->GetEnableDisable())
		m_featuresList[IVR_STAGE_ROLL_CALL].bToDo = 1; // Roll Call	
	}

	if (m_bNoVideRsrcForVideoParty)
	{
		if (m_pParty->IsFromEntryQ() && !m_pIvrService->m_bIsEntryQueueService)
			TRACEINTO << "Party move from EQ, so do not play message for No Video Resources";
		else
		{
			m_featuresList[IVR_STAGE_NO_VIDEO_RSRC].bToDo = 1;
			TRACEINTO << "Play message for No Video Resources";
		}
	}
}

//--------------------------------------------------------------------------
BYTE CIvrCntlLocal::IsEmptyConf() {
	if (m_pParty) {
		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(
				m_pParty->GetMonitorConfId());
		PASSERT_AND_RETURN_VALUE(!pCommConf, YES)
;
		int numParties = pCommConf->GetNumParties();

		CConfParty* pConfParty = pCommConf->GetFirstParty();
		for (int i = 0; i < numParties; i++)
		{
			if (NULL != pConfParty)
			{
				DWORD partyState = pConfParty->GetPartyState();
				if (partyState == PARTY_CONNECTED || partyState == PARTY_CONNECTED_WITH_PROBLEM || partyState == PARTY_SECONDARY)
				return NO;
			}
			pConfParty = pCommConf->GetNextParty();
		}
	}
	return YES;
}

/*
 03/03/03 :
 When we implemented the ability to update Party's leader status,
 we added a new third parameter bUpdateDTMFCollector. By default it is set to 0.
 However, if we wish to use CIvrCntlLocal::SetIsLeader to set an existing party, it should be 1.
 See function CParty::OnConfLeaderChangeStatusConnect
 */
//--------------------------------------------------------------------------
void CIvrCntlLocal::SetIsLeader(WORD bUpdateParty, WORD bUpdateDB,BYTE bUpdateDTMFCollector)
{
	if( IsTIPSlaveParty() )
	{
		TRACEINTO << "SetIsLeader - slave party: " << m_rsrcPartyId;
		return;
	}
	TRACEINTO << "PartyId:" << m_rsrcPartyId;

	if (bUpdateParty)
	{
		if (m_pParty)
		{
			m_pParty->SetIsLeader(YES);

			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
			if (pCommConf)
			{
				if (pCommConf->GetMuteAllButX() != (DWORD) -1)
				{
					TRACEINTO << "PartyId:" << m_rsrcPartyId << " - 'Mute All but me' is ON, unmute new chairperson";
					m_pConfApi->AudioActive(m_pParty->GetPartyRsrcID(), 0, MCMS); // unmute
				}
				TRACEINTO << "MuteAllPartiesAudioExceptLeader: " << (WORD)pCommConf->GetMuteAllPartiesAudioExceptLeader();
				if (pCommConf->GetMuteAllPartiesAudioExceptLeader())
				{
					m_pConfApi->AudioActive(m_pParty->GetPartyRsrcID(), 0, OPERATOR_REQ_BY_ID);
				}
			}
			m_pParty->SetLastConfIDPartyIsLeader(m_dwMonitorConfId);
		}
	}

	if (bUpdateDB) {
		CSegment seg;
		seg << (WORD) 1;
		m_pConfApi->UpdateDB(m_pParty, SETISLEADER, (DWORD) 1, 1, &seg);
	}

	m_bIsLeaderFound = 1;

	if (bUpdateDTMFCollector)
	{
		PASSERT_AND_RETURN(!m_pDtmfCollector);
		m_pDtmfCollector->SetDtmpOpcodePermission(DTMF_LEADER_ACTION);
	}

}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetCascadeLinkInConf(BOOL isCascadeLinkInConf)
{
	if (TRUE == isCascadeLinkInConf)
	{
		  m_cascadeLink = CASCADE_LINK_IN_CONF;
	}
	else
	{
		m_cascadeLink = CASCADE_LINK_DUMMY;
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::MuteParty(CConfApi* pConfApi, CTaskApp* pParty,BYTE bPlayMessage)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	if (pCommConf && IsToggleSelfMute())
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
		if (pConfParty)
		{
			if (pConfParty->IsAudioMutedByParty())
			{
				UnMuteParty(pConfApi, pParty);
				return;
			}
		}
	}

	WORD action = NO; // audio_non_valid => mute
	DWORD mediaMask = 0x00000001;

	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_PARTY_MUTE);
	}
	
	pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST,eCAM_EVENT_PARTY_MUTE, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::UnMuteParty(CConfApi* pConfApi, CTaskApp* pParty)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	if (pCommConf && IsToggleSelfMute())
	{
		CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
		if (pConfParty)
		{
			if (!pConfParty->IsAudioMutedByParty())
			{
				MuteParty(pConfApi, pParty);
				return;
			}
		}
	}

	WORD action = YES; // audio_valid => unmute
	DWORD mediaMask = 0x00000001;

	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_PARTY_UNMUTE);
	}
	
	pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST,eCAM_EVENT_PARTY_UNMUTE, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OverrideMuteAll(CConfApi* pConfApi)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	if (pCommConf->GetMuteAllButX() == ((DWORD)(-1)))
	{
		TRACESTR(eLevelError) << "CIvrCntlLocal::OverrideMuteAll, Conference is not in MuteAllButMe state" ;
		return;
	}

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_OVERRIDE_MUTEALL, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::IncreaseVolume(CConfApi* pConfApi, BYTE bVolumeInOut) 
{
	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		CSegment* pParam = new CSegment();
		*pParam << bVolumeInOut;
		
		m_pParty->ForwardEventToTipSlaves(pParam, TIP_MASTER_PARTY_INC_VOLUME);

		POBJDELETE(pParam);

		// 0 means AudioIn, TIP has separate AudioIns?
		if (bVolumeInOut == 0)
		{
			pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST,
					eCAM_EVENT_PARTY_INC_VOLUME, bVolumeInOut);		
		}
	}
	else
	{
		pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST,
				eCAM_EVENT_PARTY_INC_VOLUME, bVolumeInOut);
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::DecreaseVolume(CConfApi* pConfApi, BYTE bVolumeInOut) 
{
	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		CSegment* pParam = new CSegment();
		*pParam << bVolumeInOut;
		
		m_pParty->ForwardEventToTipSlaves(pParam, TIP_MASTER_PARTY_DEC_VOLUME);

		POBJDELETE(pParam);

		// 0 means AudioIn, TIP has separate AudioIns?
		if (bVolumeInOut == 0)
		{
			pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST,
					eCAM_EVENT_PARTY_DEC_VOLUME, bVolumeInOut);		
		}
	}
	else
	{
		pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST,
				eCAM_EVENT_PARTY_DEC_VOLUME, bVolumeInOut);
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::MuteAllButX(CConfApi* pConfApi, BYTE yesNo)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);

	if (pCommConf && IsToggleMuteAllButMe())
	{
		if (pCommConf->IsMuteAllButX(m_rsrcPartyId))
		{
			TRACEINTO << "PartyId:" << m_rsrcPartyId << " - Toggle DTMF, Cancel 'Mute All but me' state";
			yesNo = 0;
		}

		else
		{
			TRACEINTO << "PartyId:" << m_rsrcPartyId << " - Toggle DTMF, Activate 'Mute All but me' state";
			yesNo = 1;
		}
	}

	// mute / unmute and play message
	CSegment muteControlSegment;
	DWORD isIVR = TRUE; //VNGR-26819 was FALSE;
	DWORD isForMuteAllButLecture = FALSE;
	muteControlSegment << isIVR;
	muteControlSegment << isForMuteAllButLecture;

	if (yesNo)
		pConfApi->SendCAMGeneralActionCommandSeg(m_rsrcPartyId,
				EVENT_CONF_REQUEST, eCAM_EVENT_CONF_MUTE_ALL,
				&muteControlSegment);
	else
		pConfApi->SendCAMGeneralActionCommandSeg(m_rsrcPartyId,
				EVENT_CONF_REQUEST, eCAM_EVENT_CONF_UNMUTE_ALL,
				&muteControlSegment);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ChangePwds()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	TRACECOND_AND_RETURN(pCommConf->GetEntryQ(), "Failed, Can't start feature in EQ");

	TRACECOND_AND_RETURN(!m_pIvrService, "Failed, Can't start, IVR Service is missing");

	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	TRACECOND_AND_RETURN(!generalMsgs, "Failed, General tab in IVR Service is missing");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	// change PW action in "general" sub-feature
	m_pIvrSubGenSM = new CIvrSubGeneral(GEN_TP_CHANGE_PWDS_MENU);

	memset(m_featuresList, 0, sizeof(m_featuresList));

	m_featuresList[IVR_STAGE_CHANGE_PWDS_MENU].bToDo = 1; // password change failed
	m_stage = IVR_STAGE_CHANGE_PWDS_MENU;

	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_STARTED_IVR);
	}
	
	// move party to IVR
	m_pConfApi->SendStartFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, (DWORD) eCAM_EVENT_PARTY_CHANGE_PW);

	// update state
	m_updateDuringConf = 1;
	m_featureActionDuringConf = (DWORD) eCAM_EVENT_PARTY_CHANGE_PW;
	m_state = ACTIVE;

	// set feature params
	StartNewFeature();
	OnStartIVR();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetChangeConfPasswordFeature()
{
	for (int i = 1; i < NUM_OF_FEATURES; i++)
		m_featuresList[i].bToDo = 0;

	m_featuresList[IVR_STAGE_CHANGE_CONF_PWD].bToDo = 1; // enter new password
	m_featuresList[IVR_STAGE_CHANGE_PWDS_CONFIRM].bToDo = 1; // re-enter password
	m_featuresList[IVR_STAGE_CHANGE_PWDS_OK].bToDo = 1; // password change done
	m_stage = IVR_STAGE_LOCK_SECURE;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetChangeLeaderPasswordFeature() {
	for (int i = 1; i < NUM_OF_FEATURES; i++)
		m_featuresList[i].bToDo = 0;

	m_featuresList[IVR_STAGE_CHANGE_LEADER_PWD].bToDo = 1; // enter new password
	m_featuresList[IVR_STAGE_CHANGE_PWDS_CONFIRM].bToDo = 1; // re-enter password
	m_featuresList[IVR_STAGE_CHANGE_PWDS_OK].bToDo = 1; // password change done
	m_stage = IVR_STAGE_CHANGE_PWDS_MENU;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetChangePasswordFailed() {
	for (int i = 1; i < NUM_OF_FEATURES; i++)
		m_featuresList[i].bToDo = 0;

	m_featuresList[IVR_STAGE_CHANGE_PWDS_INVALID].bToDo = 1; // password change failed
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetChangePasswordFeature() {
	for (int i = 1; i < NUM_OF_FEATURES; i++)
		m_featuresList[i].bToDo = 0;

	m_featuresList[IVR_STAGE_CHANGE_PWDS_MENU].bToDo = 1; // password change failed
	m_stage = IVR_STAGE_LOCK_SECURE;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetIVRWaitStateForTIPSlave()
{
	BOOL isTIPSlave = m_pParty->GetIsTipCall() && ( m_pParty->GetTipPartyType() > eTipMasterCenter);
	PASSERTMSG_AND_RETURN(!isTIPSlave, "Party is not TIP slave- should not enter IVR_STAGE_WAIT!");
	TRACEINTO << "TIP slave party entering IVR";
	for (int i = 1; i < NUM_OF_FEATURES; i++)
		m_featuresList[i].bToDo = 0;

	m_featuresList[IVR_STAGE_WAIT].bToDo = 1; // password change failed
	m_masterEndFeatures = FALSE;
}
//--------------------------------------------------------------------------
void CIvrCntlLocal::ReviewNames(CConfApi* pConfApi) {
	// Call the begin roll call names review with the id of the party that asked for it as a parameter.
	// pConfApi->BeginRollCallNamesReview(m_pParty->GetMonitorPartyId());
	pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_CONF_REQUEST,
			eCAM_EVENT_CONF_ROLL_CALL_REVIEW, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::StopNamesReviewing(CConfApi* pConfApi) {
	pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_CONF_REQUEST,
			eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::EnableRollCall(CConfApi* pConfApi, BYTE byteEnable) {
	// enable / disable RollCall announcement (entry / exit)
	pConfApi->EnableRollCall(byteEnable);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::InviteParty(CConfApi* pConfApi /*, BYTE byteEnable*/)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	TRACECOND_AND_RETURN(pCommConf->GetEntryQ(), "Failed, Can't start feature in EQ");

	TRACECOND_AND_RETURN(!m_pIvrService, "Failed, Can't start, IVR Service is missing");

	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	TRACECOND_AND_RETURN(!generalMsgs, "Failed, General tab in IVR Service is missing");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	// change PW action in "general" sub-feature
	m_pIvrSubGenSM = new CIvrSubGeneral(GEN_TP_INVITE_PARTY);

	memset(m_featuresList, 0, sizeof(m_featuresList));

	m_featuresList[IVR_STAGE_INVITE_PARTY].bToDo = 1;
	m_stage = IVR_STAGE_INVITE_PARTY;

	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_STARTED_IVR);
	}
	
	// move party to IVR
	m_pConfApi->SendStartFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, (DWORD) eCAM_EVENT_INVITE_PARTY);

	// update state
	m_updateDuringConf = 1;
	m_featureActionDuringConf = (DWORD) eCAM_EVENT_INVITE_PARTY;
	m_state = ACTIVE;

	// set feature params
	StartNewFeature();
	OnStartIVR();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ReInviteParty(CConfApi* pConfApi, BOOL bIsGwInvite)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	TRACECOND_AND_RETURN(pCommConf->GetEntryQ(), "Failed, Can't start feature in EQ");

	TRACECOND_AND_RETURN(!m_pIvrService, "Failed, Can't start, IVR Service is missing");

	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	TRACECOND_AND_RETURN(!generalMsgs, "Failed, General tab in IVR Service is missing");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	ResetFeaturesList();

	if (bIsGwInvite)
	{
		m_pIvrSubGenSM = new CIvrSubGeneral(GEN_TP_GW_REINVITE_PARTY);
		m_featuresList[IVR_STAGE_GW_REINVITE_PARTY].bToDo = 1;
		m_stage = IVR_STAGE_GW_REINVITE_PARTY;
	}
	else
	{
		m_pIvrSubGenSM = new CIvrSubGeneral(GEN_TP_DTMF_REINVITE_PARTY);
		m_featuresList[IVR_STAGE_DTMF_REINVITE_PARTY].bToDo = 1;
		m_stage = IVR_STAGE_DTMF_REINVITE_PARTY;
	}

	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_STARTED_IVR);
	}
	
	//move party to IVR
	m_pConfApi->SendStartFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, (DWORD)eCAM_EVENT_INVITE_PARTY);

	// update state
	m_updateDuringConf = 1;
	m_featureActionDuringConf = (DWORD)eCAM_EVENT_INVITE_PARTY;
	m_state = ACTIVE;

	// set feature params
	StartNewFeature();
	OnStartIVR();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::PlayBusyMsg(CConfApi* pConfApi)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	TRACECOND_AND_RETURN(pCommConf->GetEntryQ(), "Failed, Can't start feature in EQ");

	TRACECOND_AND_RETURN(!m_pIvrService, "Failed, Can't start, IVR Service is missing");

	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	TRACECOND_AND_RETURN(!generalMsgs, "Failed, General tab in IVR Service is missing");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	// change PW action in "general" sub-feature
	m_pIvrSubGenSM = new CIvrSubGeneral(GEN_TP_PLAY_BUSY_MSG);

	ResetFeaturesList();

	m_featuresList[IVR_STAGE_PLAY_BUSY_MSG].bToDo = 1;
	m_stage = IVR_STAGE_PLAY_BUSY_MSG;

	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_STARTED_IVR);
	}
	
	//move party to IVR
	m_pConfApi->SendStartFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, (DWORD)eCAM_EVENT_PARTY_PLAY_BUSY_MSG);

	// update state
	m_updateDuringConf = 1;
	m_featureActionDuringConf = (DWORD)eCAM_EVENT_PARTY_PLAY_BUSY_MSG;
	m_state = ACTIVE;

	// set feature params
	StartNewFeature();
	OnStartIVR();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::PlayNoAnswerMsg(CConfApi* pConfApi)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	TRACECOND_AND_RETURN(pCommConf->GetEntryQ(), "Failed, Can't start feature in EQ");

	TRACECOND_AND_RETURN(!m_pIvrService, "Failed, Can't start, IVR Service is missing");

	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	TRACECOND_AND_RETURN(!generalMsgs, "Failed, General tab in IVR Service is missing");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	// change PW action in "general" sub-feature
	m_pIvrSubGenSM = new CIvrSubGeneral(GEN_TP_PLAY_NOANSWER_MSG);

	ResetFeaturesList();

	m_featuresList[IVR_STAGE_PLAY_NOANSWER_MSG].bToDo = 1;
	m_stage = IVR_STAGE_PLAY_NOANSWER_MSG;

	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_STARTED_IVR);
	}
	
	//move party to IVR
	m_pConfApi->SendStartFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, (DWORD)eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG);

	// update state
	m_updateDuringConf = 1;
	m_featureActionDuringConf = (DWORD)eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG;
	m_state = ACTIVE;

	// set feature params
	StartNewFeature();
	OnStartIVR();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::PlayWrongNumberMsg(CConfApi* pConfApi)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	TRACECOND_AND_RETURN(pCommConf->GetEntryQ(), "Failed, Can't start feature in EQ");

	TRACECOND_AND_RETURN(!m_pIvrService, "Failed, Can't start, IVR Service is missing");

	const CIVRGeneralMsgsFeature* generalMsgs = m_pIvrService->GetGeneralMsgsFeature();
	TRACECOND_AND_RETURN(!generalMsgs, "Failed, General tab in IVR Service is missing");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start feature while other exists");

	// change PW action in "general" sub-feature
	m_pIvrSubGenSM = new CIvrSubGeneral(GEN_TP_PLAY_WRONG_NUMBER_MSG);

	ResetFeaturesList();

	m_featuresList[IVR_STAGE_PLAY_WRONG_NUMBER_MSG].bToDo = 1;
	m_stage = IVR_STAGE_PLAY_WRONG_NUMBER_MSG;

	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_STARTED_IVR);
	}
	
	//move party to IVR
	m_pConfApi->SendStartFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, (DWORD)eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG);

	// update state
	m_updateDuringConf = 1;
	m_featureActionDuringConf = (DWORD)eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG;
	m_state = ACTIVE;

	// set feature params
	StartNewFeature();
	OnStartIVR();
}

// The function is responsible for the process of changing a regular party to
// leader during an ongoing conf
//--------------------------------------------------------------------------
void CIvrCntlLocal::ChangeToLeader()
{
	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start Feature while another is exists");
	// In case the party is already a leader, we simply ignore the request
	TRACECOND_AND_RETURN(YES == m_pParty->GetIsLeader(), "Failed, Party is already a leader");

	// We must check if we can deal the leader (depending the IVR configuration)
	const CIVRConfLeaderFeature* leaderF = m_pIvrService->GetConfLeaderFeature();
	TRACECOND_AND_RETURN(YES != leaderF->GetEnableDisable(), "Failed, Feature is disable");

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);

	// We need to retrieve the currect conference Leader Password
	const char* pLeaderConfPassword = pCommConf->GetH243Password();
	TRACECOND_AND_RETURN(!pLeaderConfPassword, "Failed, Leader password isn't available");

	int len = strlen(pLeaderConfPassword);
	TRACECOND_AND_RETURN(0 == len, "Leader password in conf is empty");

	if (m_leaderPassword)
	delete[] m_leaderPassword;

	m_leaderPassword = new char[len + 1];
	strncpy(m_leaderPassword, pLeaderConfPassword, len+1);

	// Creating the subClass CIvrSubLeader to deal the request
	m_pIvrSubGenSM = new CIvrSubLeader(m_pIvrService->GetConfLeaderFeature(), m_bEnableExternalDB, TRUE);

	memset(m_featuresList, 0, sizeof(m_featuresList));

	// update during conf
	m_updateDuringConf = 1;
	m_featureActionDuringConf = eCAM_EVENT_PARTY_CHANGE_TO_LEADER;
	if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
	{
		m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_STARTED_IVR);
	}
	// update state
	m_state = ACTIVE;
	m_stage = IVR_STAGE_CHANGE_TO_LEADER;
	// move party to IVR
	m_pConfApi->SendStartFeature(m_rsrcPartyId, EVENT_PARTY_REQUEST, (DWORD) eCAM_EVENT_PARTY_CHANGE_TO_LEADER);
	// set feature params
	StartNewFeature();

	OnStartIVR();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::Start_PLC()
{
	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start Feature while another is exists");
	// checks if Click&View enabled in IVR Service
	const CIVRVideoFeature* pVideoFeature = m_pIvrService->GetVideoFeature();
	if (pVideoFeature)
		TRACECOND_AND_RETURN(!pVideoFeature->GetEnableVC(), "Failed, Click&View disabled in IVR Service (Video Tab)");

	// Creating the subClass CIvrSubLeader to deal the request
	// The dynamic allocation will be deleted later on, through CIvrCntlLocal::OnEndFeature()
	m_pIvrSubGenSM = new CIvrSubPLC;

	memset(m_featuresList, 0, sizeof(m_featuresList));

	m_updateDuringConf = 1;

	m_state = ACTIVE;
	m_stage = 0;

	// set feature params
	StartNewFeature();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::StartVenus()
{
	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start Feature while another is exists");
	// Creating the subClass CIvrSubVenus to deal the request
	// The dynamic allocation will be deleted later on, through CIvrCntlLocal::OnEndFeature()
	m_pIvrSubGenSM = new CIvrSubVenus;

	memset(m_featuresList, 0, sizeof(m_featuresList));

	m_updateDuringConf = 1;

	m_state = ACTIVE;
	m_stage = 0;

	// set feature params
	StartNewFeature();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SecureConf(CConfApi* pConfApi, BYTE SecureFlag)
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	if (pCommConf && IsToggleSecuredConference())
		SecureFlag = pCommConf->IsConfSecured() ? 0 : 1;

	DWORD partyRsrcID = m_pParty->GetPartyRsrcID();

	TRACEINTO << "CIvrCntlLocal::SecureConf - PartyId:" << partyRsrcID
			<< ", SecureFlag:" << (int) SecureFlag;

	if (SecureFlag)
		m_pConfApi->SendCAMGeneralActionCommand(partyRsrcID,EVENT_CONF_REQUEST, eCAM_EVENT_CONF_SECURE, 0);
	else
		m_pConfApi->SendCAMGeneralActionCommand(partyRsrcID,EVENT_CONF_REQUEST, eCAM_EVENT_CONF_UNSECURE, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ShowParticipants(CConfApi* pConfApi)
{
	DWORD partyRsrcID = m_pParty->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyRsrcID;
	m_pConfApi->SendCAMGeneralActionCommand(partyRsrcID, EVENT_CONF_REQUEST,eIVR_SHOW_PARTICIPANTS, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::RequestToSpeak(CConfApi* pConfApi)
{
	DWORD partyRsrcID = m_pParty->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyRsrcID;
	m_pConfApi->SendCAMGeneralActionCommand(partyRsrcID, EVENT_PARTY_REQUEST,eIVR_REQUEST_TO_SPEAK, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ShowGathering()
{
	DWORD partyRsrcID = m_pParty->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyRsrcID;
	m_pConfApi->SendCAMGeneralActionCommand(partyRsrcID, EVENT_CONF_REQUEST,eIVR_SHOW_GATHERING, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::StartPcm()
{
	TRACEINTO << "PartyId:" << m_rsrcPartyId;
	m_pConfApi->FeccKeyMsg(m_rsrcPartyId, eFeccKeyZoomIn);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::MovePartyToInConf()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	TRACEINTO << "PartyId:" << m_pParty->GetPartyRsrcID();

	// Moving the Party back to Default Group
	m_pConfApi->UpdateDB(m_pParty, PARTYAVSTATUS, STATUS_PARTY_INCONF, 1);
	m_updateDuringConf = 0;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::CancelSetLeader()
{
	m_bIsLeaderFound = 0;

	if (m_pDtmfCollector)
		m_pDtmfCollector->SetDtmpOpcodePermission(DTMF_USER_ACTION);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::StartDialOut()
{
	TRACECOND_AND_RETURN(m_pIvrSubGenSM, "Failed, Can't start Feature while another is exists");
	m_pIvrSubGenSM = new CIvrSubDialOutToRemote;
	// set feature params
	StartNewFeature();

	// update state
	m_state = ACTIVE;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnIvrReset(CSegment* pParam)
{
	ResetIvr();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ResetIvr()
{
	if (m_pDtmfCollector)
	{
		m_pDtmfCollector->ResetDtmfParams();
		m_pDtmfCollector->ResetDtmfBuffer();
		m_pDtmfCollector->EndIvrPartSession();
		m_pDtmfCollector->ResetOtherCollectorParams();
	}

	// Do some actions in feature (if needed) upon Stop IVR (in V2.0 - only for PLC)
	if (m_pIvrSubGenSM)
		m_pIvrSubGenSM->OnStopIvr();

	// reset features status (to start later from the beginning)
	for (int i = 0; i < NUM_OF_FEATURES; i++)
	{
		m_featuresList[i].bStarted = 0;
		m_featuresList[i].bFinished = 0;
	}

	POBJDELETE(m_pIvrSubGenSM); // delete current feature
	m_state = ACTIVE; // state
	m_stage = 0; // current stage in the progress IVR process
	m_startNextFeature = FALSE;
	m_bIsLeaderFound = 0;
	m_ChangePWDType = 0;
	m_new_password[0] = 0;
	m_dtmfForExt = 0;
	m_initExtensionTimer = 0;
	m_pauseAtTheEnd = 0;
	m_updateDuringConf = 0;
	m_featureActionDuringConf = 0;
	m_bIsResumeAfterHold = FALSE;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::StopIVR()
{
	// update leader (no leader) in Conf
	// if stage is after leader...
	if (m_bIsLeaderFound)
		m_pConfApi->UpdateDB(m_pParty, SETISLEADER, (DWORD) 0, 1);

	// update leader (no leader) in Conf
	if (m_pParty)
		m_pParty->SetIsLeader(NO);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnTIPMasterStartedIVR(CSegment* pParam)
{
	TRACEINTO;
	SetIVRWaitStateForTIPSlave();
	Start();
}
//--------------------------------------------------------------------------
void CIvrCntlLocal::OnPartyOnHoldInd(CSegment* pParam)
{
	CSmallString str;
	BOOL isIVRActive = (GetState() == ACTIVE) && m_stage != 0;
	if (isIVRActive)
		str << ", IVR active, saving IVR feature list.";
	else
		str << ", IVR not currently active, nothing to save.";
	TRACEINTO << " current m_stage is " << IvrStageToString(m_stage) << str.GetString();
	if (isIVRActive)
	{
		for (int i = 0; i < NUM_OF_FEATURES; i++)
		{
			m_featuresListForHoldCallScenario[i] = m_featuresList[i];
			if (m_featuresListForHoldCallScenario[i].bToDo != 0)
			{
				BOOL isFeatureCompleted = FALSE;
				if (m_featuresListForHoldCallScenario[i].bStarted != 0)
					isFeatureCompleted = m_featuresListForHoldCallScenario[i].bFinished != 0;
				// if feature is complete, there's no need to handle it when resuming
				if (isFeatureCompleted)
					m_featuresListForHoldCallScenario[i].bToDo = 0;
				// if hold occured in mid-feature , we restart it when resuming
				else
					m_featuresListForHoldCallScenario[i].bStarted = 0;
			}
		}
		m_stageForHoldCallScenario = m_stage;
		TRACEINTO << "holding at stage " << IvrStageToString(m_stage);
	}
}
//--------------------------------------------------------------------------
const char* CIvrCntlLocal::GetLeaderPassword()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	if (pCommConf)
	{
		const char* pLeaderPassword = pCommConf->GetH243Password();
		if (pLeaderPassword)
		{
			TRACEINTO << "Old Leader PWD :" <<  m_leaderPassword << " , New Leader PWD :" << pLeaderPassword << "Party Name: " << m_pParty->GetName();
			RemoveLeaderPassword();
			int len = strlen(pLeaderPassword);
			m_leaderPassword = new char[len+1];
			strncpy(m_leaderPassword, pLeaderPassword, len+1);
		}
	}
	return m_leaderPassword;

}
//--------------------------------------------------------------------------
void CIvrCntlLocal::RemoveLeaderPassword() {
	PDELETEA(m_leaderPassword);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnConfTerminate(CConfApi* pConfApi, WORD IsDtmfForwarding) {
	CSegment* seg = new CSegment;
	*seg << (WORD) IsDtmfForwarding;

	pConfApi->SendMsg(seg, IVR_CONFTERMINATE);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::BillingCodeToCDR(const char* billingCode) {
	DWORD confID = m_pParty->GetMonitorConfId();
	const char* PartyName = m_pParty->GetName();
	DWORD partyMonitorID = m_pParty->GetMonitorPartyId();

	CCommConf* pCommConf = new CCommConf;
	pCommConf->BillingCodeToCDR(confID, PartyName, partyMonitorID, billingCode);

	POBJDELETE(pCommConf);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::GetGeneralSystemMsgsParams(WORD event_op_code,
		char* msgFullPath, WORD* msgDuration, WORD* msgCheckSum) {
	msgFullPath[0] = '\0';
	(*msgDuration) = 0;
	(*msgCheckSum) = (WORD) (-1);
	if (m_pIvrService) {
		const char* ivrServiceName = m_pIvrService->GetName();

		int status = ::GetpAVmsgServList()->GetIVRMsgParams(ivrServiceName,
				IVR_FEATURE_GENERAL, event_op_code, msgFullPath, msgDuration,
				msgCheckSum);
		if (STATUS_OK != status)
		{
			msgFullPath[0] = '\0';
			(*msgDuration) = 0;
			(*msgCheckSum) = (WORD) (-1);
			TRACEINTO << "status:" << status << " - Failed in GetIVRMsgParams";
		}
	}
}

//--------------------------------------------------------------------------
WORD CIvrCntlLocal::GetMsgDuration(const CIVRFeature* feature,
		const WORD event_opcode) {
	// time delay to let the message be herd
	WORD duration = 12; // some default

	CIVREvent* pIVREvent = feature->GetCurrentIVREvent(event_opcode);
	TRACECOND_AND_RETURN_VALUE(!pIVREvent, "Failed, return default = 12 seconds", duration);
	CIVRMessage* pIVRMessage = pIVREvent->GetCurrentIVRMessage(m_language);
	TRACECOND_AND_RETURN_VALUE(!pIVRMessage, "Failed, return default = 12 seconds", duration);

	return pIVRMessage->GetMsgDuration();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetNewPassword(const char* pass) {
	strncpy(m_new_password, pass, sizeof(m_new_password) - 1);
	m_new_password[sizeof(m_new_password) - 1] = '\0';
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetInvitePartyAddress(const char* sPartyAddress) {
	char invitePartyAddress[PARTY_ADDRESS_LEN];
	strncpy(invitePartyAddress, sPartyAddress, PARTY_ADDRESS_LEN - 1);
	invitePartyAddress[PARTY_ADDRESS_LEN - 1] = 0;

	for (WORD i = 1; i < strlen(invitePartyAddress); i++) {
		if (invitePartyAddress[i] == '*')
			invitePartyAddress[i] = '.';
	}

	strncpy(m_invitePartyAddress, invitePartyAddress, PARTY_ADDRESS_LEN);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetInvitePartyFeature() {
	for (int i = 1; i < NUM_OF_FEATURES; i++)
		m_featuresList[i].bToDo = 0;

	m_featuresList[IVR_STAGE_INVITE_PARTY_ADDRESS].bToDo = 1; // enter new password
	m_stage = IVR_STAGE_INVITE_PARTY_ADDRESS;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SendGeneralMessageMENU() {
	if (m_pParty) 
	{
		if ( m_pParty->GetIsTipCall() && m_pParty->GetTipPartyType() == eTipMasterCenter )
		{
			m_pParty->ForwardEventToTipSlaves(NULL, TIP_MASTER_PARTY_PLAY_MENU);
		}
		else
		{
			DWORD partyRsrcID = m_pParty->GetPartyRsrcID();
			m_pConfApi->SendCAMGeneralActionCommand(partyRsrcID,
					EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_PLAY_MENU, 0);
		}
	}
}

//--------------------------------------------------------------------------
int CIvrCntlLocal::NeedExtension() // Dtmf on connect party
{
	TRACECOND_AND_RETURN_VALUE(1 == m_pParty->GetExtDone(), "Failed, Extension was already done", 0);
	m_pParty->SetExtDone(1); // done only once per party

	char name[USER_IDENTIFIER_STRING_LEN];

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, 0);

	char* pUserIdetifyString = (char*) pConfParty->GetUserIdentifierString();
	TRACECOND_AND_RETURN_VALUE(!pUserIdetifyString, "Failed, User identifier string is NULL", 0);

	TRACECOND_AND_RETURN_VALUE(!strlen(pUserIdetifyString), "Failed, User identifier string is empty", 0);

	strncpy(name, pUserIdetifyString, sizeof(name)-1);
	name[sizeof(name)-1] = '\0';

	TRACEINTO << "CIvrCntlLocal::NeedExtension - Using user identify value for sending DTMF, Extension:" << name;

	char* ext = name;
	int len = strlen(ext);

	// skip not legal characters:
	int s = 0;
	int i = 0;
	for (i = 0; i < len; i++)
	{
		if (((ext[i] != ' ') && (ext[i] != '#') && (ext[i] != '*') && (ext[i] != 'p') && (ext[i] != 'P')) && (ext[i] < '0' || ext[i] > '9'))
		continue; // ignore illegal characters

		ext[i] = tolower(ext[i]);
		ext[s++] = ext[i];
	}

	TRACECOND_AND_RETURN_VALUE(0 == s, "Failed, User identifier string has only illegal characters", 0);

	ext[s] = 0;
	len = s;

	// finds number of 'p' (for pause) and remove from string
	for (i = 0; i < len; i++) // count p
	if (ext[i] != 'p')
	break;

	int pauseAtStart = i;
	if (i != 0)
	{
		ext = &ext[i];
		len -= i;
	}

	TRACECOND_AND_RETURN_VALUE(0 == len, "Failed, Failed, User identifier string has only 'p' characters without tones", 0);

	// finds number of 'p' (for pause) at the end (after extension)
	m_pauseAtTheEnd = 0;
	const char* pEnd = &ext[len - 1];
	while (*pEnd == 'p')
	{
		m_pauseAtTheEnd++;
		pEnd--;
	}

	len = len - m_pauseAtTheEnd;
	ext[len] = 0;

	PDELETEA(m_dtmfForExt);
	m_dtmfForExt = new char[len + 1];
	strncpy(m_dtmfForExt, ext, len + 1);
	m_dtmfForExt[len] = '\0';

	TRACEINTO << "Extension:" << m_dtmfForExt;

	// start timer
	StartTimer(EXTENSION_DTMF_TIMER, (pauseAtStart * SECOND));
	m_initExtensionTimer = 1;

	return 1;
}

//--------------------------------------------------------------------------
BOOL CIvrCntlLocal::IsIvrProviderEQPartyInConf() {
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, FALSE);
	BOOL isIvrProviderEQ = pCommConf->isIvrProviderEQ();
	TRACEINTO << "isIvrProviderEQ:" << (WORD)isIvrProviderEQ;
	return isIvrProviderEQ;
}

//--------------------------------------------------------------------------
WORD CIvrCntlLocal::IsCascadeLinkPartyInConf() {
	WORD result = 0;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, 0);


	if (YES == pCommConf->GetEntryQ())
		TRACEINTO << "PartyId:" << pConfParty->GetPartyId() << ", RC:" << result << " - Party in EQ";
	else
	{
		if (pConfParty->GetCascadeMode() != CASCADE_MODE_NONE)
		{
			result = 1;
			TRACEINTO << "PartyId:" << pConfParty->GetPartyId() << ", RC:" << result << " - Cascade party";
		}
		else
			TRACEINTO << "PartyId:" << pConfParty->GetPartyId() << ", RC:" << result << " - Not cascade party";
	}

	return result;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetLinkParty() {
	m_cascadeLink = CASCADE_LINK_IN_CONF;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::OnTimerSendfExt(CSegment* pParam) // Dtmf on connect party
{

	m_initExtensionTimer = 0;
	int timerTime = 0;

	// send DTMF string
	if (m_dtmfForExt) {
		int len = strlen(m_dtmfForExt);
		if (len > 0) {
			int numLen, pLen; // pLen: number of p after the current sub-string to send
			GetNumbersLen(len, &numLen, &pLen); // input: len is max string len.
			// output: numLen - what to send now.
			// pLen - how many p's we encouter.
			// len-pLen-numLen is rest of str to
			// send next timer activation.
			if (numLen > 0) // error if not
			{
				char* tempStr = new char[numLen + 1];
				strncpy(tempStr, m_dtmfForExt, numLen);
				tempStr[numLen] = 0;

				DWORD partyRsrcID = m_pParty->GetPartyRsrcID();

				TRACEINTO << "PartyId:" << partyRsrcID << ", DtmfString:" << tempStr;
				m_pConfApi->SendDtmfFromParty(partyRsrcID, tempStr);

				delete[] tempStr;

				int timerTime = numLen * (SECOND / 2); // 1/2 second for every DTMF char (0.2 for the tone+0.2 for the silence)

				if (numLen != len) // not all string was send, need to send more in 1 second
				{
					int j = 0;
					for (j = 0; j < (len - (numLen + pLen)); j++) // update buffer to send
						m_dtmfForExt[j] = m_dtmfForExt[numLen + pLen + j];

					m_dtmfForExt[j] = 0;
					timerTime += (pLen * SECOND); // pLen can be 0
				} else // all the string was sent
				{
					delete[] m_dtmfForExt;
					m_dtmfForExt = 0;
					timerTime += (m_pauseAtTheEnd * SECOND);
				}

				// wait for the current tones to play (1 second)
				m_initExtensionTimer = 1;
				StartTimer(EXTENSION_DTMF_TIMER, timerTime);
			}
		}
	}

	if (0 == m_initExtensionTimer) {
		DeleteAllTimers();

		if (m_dtmfForExt) {
			delete[] m_dtmfForExt;
			m_dtmfForExt = 0;
		}

		Start(NULL); // start the IVR entrance session after DTMF was handled.
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::GetNumbersLen(int len, int* numLen, int* pLen) {
	// find the first p char (if exists)
	// at this point 'p' is not at the beginning of the string.
	int i = 0;
	for (i = 0; i < len; i++) {
		if (m_dtmfForExt[i] == 'p')
			break;
	}

	if ((i == len) || (i > MAX_DTMF_CHUNK)) // no p in the string or in the first 9 characters
	{
		if (i > MAX_DTMF_CHUNK) // MAX_DTMF_CHUNK=31 since struct size is 64. this includes SILENCES for every tone:
			// SILENCE TONE SILENCE TONE SILENCE... so 31*2+1 is max dtmfs to send to card. means
			// 31 to send as real DTMF tones.
			i = MAX_DTMF_CHUNK;

		(*numLen) = i;
		(*pLen) = 0;
	}

	else // there is at least one p
	{
		(*numLen) = i;
		int j = 0;
		for (j = i; j < len; j++) {
			if (m_dtmfForExt[j] != 'p')
				break;
		}

		(*pLen) = j - i;
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::InformFeatureDTMF() {
	if (m_pIvrSubGenSM)
		m_pIvrSubGenSM->OnDTMF();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ActivateIvrMuteNoisyLine()
{
	// check if home conf is not EQ
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN(!pConfParty);

	TRACECOND_AND_RETURN(pCommConf->GetEntryQ(), "Failed, Can't start feature in EQ");

	TRACECOND_AND_RETURN(!m_pIvrService, "Failed, Can't start, IVR Service is missing");


	if (m_pIvrSubGenSM == NULL)
	{
		m_pIvrSubGenSM = new CIvrSubMuteNoisyLine(m_pIvrService->GetMuteNoisyLineFeature());

		memset(m_featuresList, 0, sizeof(m_featuresList));
	}

	// set feature params
	StartNewFeature();
	OnStartIVR();

	m_state = ACTIVE;
}

//--------------------------------------------------------------------------
int CIvrCntlLocal::IsEnableDtmfForwarding()
{
	BOOL bIsEnableDtmfForwarding;
	CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(CFG_KEY_DTMF_FORWARDING, bIsEnableDtmfForwarding);
	return (bIsEnableDtmfForwarding) ? 1 : 0;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SendDtmfForwarding(WORD wEventOpCode)
{
	TRACECOND_AND_RETURN(!IsEnableDtmfForwarding(), "Failed, Disabled by configuration");
	CDTMFCodeList* pDTMFCodeList = m_pDtmfCollector->GetDtmfCodeList();
	PASSERT_AND_RETURN(!pDTMFCodeList);

	CDTMFCode* pDTMFCode = pDTMFCodeList->GetCurrentDTMFCodeByOpcode(wEventOpCode);
	PASSERT_AND_RETURN(!pDTMFCode);

	char sDtmfString[DTMF_STRING_LEN];
	sDtmfString[0] = '\0';

	if (pDTMFCode->GetDTMFStr())
	{
		strncpy(sDtmfString, pDTMFCode->GetDTMFStr(), sizeof(sDtmfString) - 1); // not ensure \0 termination
		sDtmfString[sizeof(sDtmfString) - 1] = '\0';
	}

	PASSERT_AND_RETURN(sDtmfString[0] == '\0'); // empty DTMF string

	TRACEINTO << "DtmfString:" << sDtmfString;

	// send message to CAM to forward the DTMF string to the Cascade link
	CSegment dtmfSegment;
	dtmfSegment << sDtmfString;
	m_pConfApi->SendCAMGeneralActionCommandSeg(m_rsrcPartyId, EVENT_CONF_REQUEST, eCAM_EVENT_CONF_DTMF_FORWARDING, &dtmfSegment);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::HandleDtmfForwarding(CSegment* pParam)
{
	char sDtmfString[DTMF_STRING_LEN];
	memset(sDtmfString, '\0', DTMF_STRING_LEN);

	DWORD dtmfStrLen;
	*pParam >> (DWORD &) dtmfStrLen;

	PASSERT_AND_RETURN(dtmfStrLen == 0);	// empty DTMF string

	pParam->Get((BYTE*) (&sDtmfString), dtmfStrLen);
	sDtmfString[dtmfStrLen] = '\0';
	CSegment dtmfSegment;
	dtmfSegment << sDtmfString;
	TRACEINTO << "DtmfString:" << sDtmfString;
	m_pConfApi->SendCAMGeneralActionCommandSeg(m_rsrcPartyId, EVENT_CONF_REQUEST, eCAM_EVENT_CONF_DTMF_FORWARDING, &dtmfSegment);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetGwDTMFForwarding(BYTE val) {
	m_pDtmfCollector->SetGwDTMFForwarding(val);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SetInvitePartyDTMFForwarding(BYTE val)
{
	m_pDtmfCollector->SetInvitedPartyDTMFForwarding(val);
	StartTimer(DTMF_FWD_INVITOR_TOUT, m_pIvrService->getDtmfForwardDuration()* SECOND);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::onInvitePartyDTMFForwardingTout() {
	m_pDtmfCollector->SetInvitedPartyDTMFForwarding(FALSE);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::setNoVideRsrcForVideoParty(BYTE bNoVideRsrcForVideoParty)
{
	TRACEINTO << " bNoVideRsrcForVideoParty=" << (DWORD)bNoVideRsrcForVideoParty;

	m_bNoVideRsrcForVideoParty = bNoVideRsrcForVideoParty;
	if (m_bNoVideRsrcForVideoParty)
		m_featuresList[IVR_STAGE_NO_VIDEO_RSRC].bToDo = 1;
	else
		m_featuresList[IVR_STAGE_NO_VIDEO_RSRC].bToDo = 0;
}

//--------------------------------------------------------------------------
int CIvrCntlLocal::IsPSTNCall() {
	return 1;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::PcmConnected() {
	m_pDtmfCollector->PcmConnected();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::PcmDisconnected() {
	m_pDtmfCollector->PcmDisconnected();
}

//--------------------------------------------------------------------------
WORD CIvrCntlLocal::IsRecordingLinkParty()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, 0);

	return pConfParty->GetRecordingLinkParty();
}

//--------------------------------------------------------------------------
WORD CIvrCntlLocal::IsOperatorParty()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, 0);

	return pConfParty->IsOperatorParty();
}

//--------------------------------------------------------------------------
BOOL CIvrCntlLocal::IsTIPSlaveParty()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, 0);

	return pConfParty->IsTIPSlaveParty();
}

//--------------------------------------------------------------------------
BOOL CIvrCntlLocal::IsRelayParty()
{
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, 0);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, 0);

	return (pConfParty->GetPartyMediaType() == eSvcPartyType);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::DoRecordingAction(CConfApi* pConfApi, WORD opcode) {
	pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_CONF_REQUEST,
			opcode, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::PrepareForwardDtmfOpcodesTable(	CDTMFCodeList* pCDTMFCodeList)
{
	CDTMFCodeList* pOriginCDTMFCodeList = new CDTMFCodeList(*(m_pIvrService->GetDTMFCodeList()));

	// all list of DTMF forwarding
	CDTMFCode* forwardingCode =
			pOriginCDTMFCodeList->GetCurrentDTMFCodeByOpcode(
					DTMF_CONF_TERMINATE);
	if (forwardingCode)
		pCDTMFCodeList->AddDTMFCode(*forwardingCode);

	forwardingCode = pOriginCDTMFCodeList->GetCurrentDTMFCodeByOpcode(
			DTMF_MUTE_ALL_BUT_X);
	if (forwardingCode)
		pCDTMFCodeList->AddDTMFCode(*forwardingCode);

	forwardingCode = pOriginCDTMFCodeList->GetCurrentDTMFCodeByOpcode(
			DTMF_UNMUTE_ALL_BUT_X);
	if (forwardingCode)
		pCDTMFCodeList->AddDTMFCode(*forwardingCode);

	forwardingCode = pOriginCDTMFCodeList->GetCurrentDTMFCodeByOpcode(
			DTMF_SECURE_CONF);
	if (forwardingCode)
		pCDTMFCodeList->AddDTMFCode(*forwardingCode);

	forwardingCode = pOriginCDTMFCodeList->GetCurrentDTMFCodeByOpcode(
			DTMF_UNSECURE_CONF);
	if (forwardingCode)
		pCDTMFCodeList->AddDTMFCode(*forwardingCode);

	// delete list
	POBJDELETE(pOriginCDTMFCodeList);
}

//--------------------------------------------------------------------------
WORD CIvrCntlLocal::OperatorAssistance(CConfApi* pConfApi, DWORD request_type)
{
	TRACEINTO << "requestType:"	<< request_type;

	// find feature in service
	const CIVROperAssistanceFeature* assist =
			m_pIvrService->GetOperAssistanceFeature();
	TRACECOND_AND_RETURN_VALUE(!assist, "Failed, Feature does not exist", 1);
	TRACECOND_AND_RETURN_VALUE(!assist->GetEnableDisable(), "Failed, Feature disabled", 1);

	// do nothing if party already in operator assistance
	if (m_pIvrSubGenSM)
	{
		if (0 == strcmp("CIvrSubOperatorAssist", m_pIvrSubGenSM->NameOf())) // operator assist
		{
			TRACEINTO << "Failed, Party already in operator assistance";
			return 0;
		}
	}

	// get conf and party from DB
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_dwMonitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, 1);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, 1);

	// move is blocked for telepresence, so we block the operator assistance
	WORD isTelePresence = pCommConf->GetIsTelePresenceMode();
	TRACECOND_AND_RETURN_VALUE(isTelePresence, "Failed, Operator assistance is blocked for TelePresence conf", 1);

	// move is blocked for VSW conf, so we block the operator assistance
	WORD isVSW = pCommConf->GetIsHDVSW();
	TRACECOND_AND_RETURN_VALUE(isVSW, "Failed, Operator assistance is blocked for VSW conf", 1);

	eConfMediaType confMediaType = pCommConf->GetConfMediaType();
	TRACECOND_AND_RETURN_VALUE(eAvcOnly != confMediaType, "Failed, Operator assistance only works for AVC only conference", 1);

	// vngfe-3720
	TRACECOND_AND_RETURN_VALUE(CASCADE_LINK_IN_CONF == m_cascadeLink, "Failed, Operator assistance is blocked for link party (CASCADE_LINK_IN_CONF)", 1);

	// vngfe-3720
	TRACECOND_AND_RETURN_VALUE(CASCADE_LINK_IN_EQ == m_cascadeLink, "Failed, Operator assistance is blocked for link party (CASCADE_LINK_IN_EQ)", 1);

	BYTE mode_val = WAIT_FOR_OPER_NONE;
	BYTE failed_on_password = 0;

	switch (request_type)
	{
		case IVR_EVENT_WAIT_FOR_OPER_ON_CONF_PWD_FAIL:
		case IVR_EVENT_WAIT_FOR_OPER_ON_CHAIR_PWD_FAIL:
		case IVR_EVENT_WAIT_FOR_OPER_ON_NID_FAIL:
		{
			failed_on_password = 1;
			mode_val = WAIT_FOR_OPER_ON_REQ_PRIVATE;
			break;
		}

		case IVR_EVENT_WAIT_FOR_OPER_ON_PARTY_PRIVATE_REQ:
		{
			mode_val = WAIT_FOR_OPER_ON_REQ_PRIVATE;
			break;
		}

		case IVR_EVENT_WAIT_FOR_OPER_ON_PARTY_PUBLIC_REQ:
		{
			mode_val = WAIT_FOR_OPER_ON_REQ_PUBLIC;

			// start feature - play once , set public
			break;
		}
	}

	// will update conf status
	pConfParty->SetWaitForOperAssistance(mode_val);
	pConfApi->OperatorAssistance(m_monitorPartyId, 1, mode_val, request_type);

	if (!failed_on_password)
	{
		// update CAM (list) for new feature
		TRACEINTO << "Start the feature";
		pConfApi->SendCAMGeneralActionCommand(m_rsrcPartyId, EVENT_PARTY_REQUEST, eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE, 1);
	}

	// start the OperatorAssistanceFeature
	m_pIvrSubGenSM = (CIvrSubBaseSM*) new CIvrSubOperatorAssist(m_pIvrService->GetOperAssistanceFeature(), request_type);

	// set feature params
	StartNewFeature();

	WORD messageDuration = GetMsgDuration(m_pIvrService->GetOperAssistanceFeature(), IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE);
	TRACEINTO << "messageDuration:" << messageDuration;

	// update state
	m_state = ACTIVE;

	return 0;
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::SipConfNIDConfirmationInd(DWORD sts) // will be called from party response
{
	CSegment* seg = new CSegment;
	*seg << (DWORD) sts;
	DWORD msgLen = 0;

	m_pIvrSubGenSM->HandleEvent(seg, msgLen, SIP_CONF_NID_CONFIRM_IND);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::BlipOnCascadeLink()
{
	DWORD partyRsrcID = m_pParty->GetPartyRsrcID();

	TRACEINTO << "PartyId:" << partyRsrcID;
	m_pConfApi->SendCAMGeneralActionCommand(partyRsrcID, EVENT_CONF_REQUEST,eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK, 0);
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ResetFeaturesList()
{
	for (int i = 0; i < NUM_OF_FEATURES; i++)
	{
		m_featuresList[i].bToDo = 0;
		m_featuresList[i].bStarted = 0;
		m_featuresList[i].bFinished = 0;
	}
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ResumeFeaturesList(DWORD lastStageBeforeResume /*= (DWORD)-1*/)
{
	for (int i = 0; i < NUM_OF_FEATURES; i++)
	{
		m_featuresList[i] = m_featuresListForHoldCallScenario[i];
	}
	if (lastStageBeforeResume == (DWORD)-1)
		m_stage = m_stageForHoldCallScenario;
	else
		m_stage = lastStageBeforeResume;
	TRACEINTO << "resuming IVR at stage " << IvrStageToString(m_stage);
	// resetting saved parameters
	ResetIvrHoldParams();
}

//--------------------------------------------------------------------------
void CIvrCntlLocal::ResetIvrHoldParams()
{
	memset(m_featuresListForHoldCallScenario, 0, sizeof(m_featuresListForHoldCallScenario));
	m_stageForHoldCallScenario = (DWORD)-1;
}

//--------------------------------------------------------------------------
BOOL CIvrCntlLocal::IsIvrOnHold()
{
	TRACEINTO << "held IVR stage:" << (int)m_stageForHoldCallScenario;
	return (BOOL)(m_stageForHoldCallScenario != (DWORD)-1);
}
//--------------------------------------------------------------------------
bool CIvrCntlLocal::IsConfOrLeaderPasswordRequired()const
{
	if(0 != m_featuresList[IVR_STAGE_CONF_PASSWORD].bToDo || 0 != m_featuresList[IVR_STAGE_CONF_LEADER].bToDo)
	{
		TRACEINTO << " leader or conf password required";
		return true;
	}
	return false;
}
//--------------------------------------------------------------------------
void CIvrCntlLocal::ResetFeaturesListForCallFromGW()
{
	for (int i = 0; i < NUM_OF_FEATURES; i++)
	{
		if(i == IVR_STAGE_CONF_PASSWORD || i == IVR_STAGE_CONF_LEADER){
			continue;
		}
		m_featuresList[i].bToDo = 0;
		m_featuresList[i].bStarted = 0;
		m_featuresList[i].bFinished = 0;
	}
}

//--------------------------------------------------------------------------
const char* CIvrCntlLocal::DtmfCodeToString(WORD opcode)
{
	switch (opcode) {
	case DTMF_CONF_TERMINATE:
		return "DTMF_CONF_TERMINATE";
	case DTMF_SELF_MUTE:
		return "DTMF_SELF_MUTE";
	case DTMF_SELF_UNMUTE:
		return "DTMF_SELF_UNMUTE";
	case DTMF_INC_SELF_VOLUME:
		return "DTMF_INC_SELF_VOLUME";
	case DTMF_DEC_SELF_VOLUME:
		return "DTMF_DEC_SELF_VOLUME";
	case DTMF_MUTE_ALL_BUT_X:
		return "DTMF_MUTE_ALL_BUT_X";
	case DTMF_UNMUTE_ALL_BUT_X:
		return "DTMF_UNMUTE_ALL_BUT_X";
	case DTMF_OVERRIDE_MUTE_ALL:
		return "DTMF_OVERRIDE_MUTE_ALL";
	case DTMF_MUTE_INCOMING_PARTIES:
		return "DTMF_MUTE_INCOMING_PARTIES";
	case DTMF_UNMUTE_INCOMING_PARTIES:
		return "DTMF_UNMUTE_INCOMING_PARTIES";
	case DTMF_START_DIALOUT:
		return "DTMF_START_DIALOUT";
	case DTMF_PLAY_MENU:
		return "DTMF_PLAY_MENU";
	case DTMF_CHANGE_PASSWORD:
		return "DTMF_CHANGE_PASSWORD";
	case DTMF_ROLL_CALL_REVIEW_NAMES:
		return "DTMF_ROLL_CALL_REVIEW_NAMES";
	case DTMF_ROLL_CALL_STOP_REVIEW_NAMES:
		return "DTMF_ROLL_CALL_STOP_REVIEW_NAMES";
	case DTMF_ENABLE_ROLL_CALL:
		return "DTMF_ENABLE_ROLL_CALL";
	case DTMF_DISABLE_ROLL_CALL:
		return "DTMF_DISABLE_ROLL_CALL";
	case DTMF_INC_LISTEN_VOLUME:
		return "DTMF_INC_LISTEN_VOLUME";
	case DTMF_DEC_LISTEN_VOLUME:
		return "DTMF_DEC_LISTEN_VOLUME";
	case DTMF_DUMP_TABLE:
		return "DTMF_DUMP_TABLE";
	case DTMF_CHANGE_TO_LEADER:
		return "DTMF_CHANGE_TO_LEADER";
	case DTMF_START_VC:
		return "DTMF_START_VC";
	case DTMF_START_RESUME_RECORDING:
		return "DTMF_START_RESUME_RECORDING";
	case DTMF_PAUSE_RECORDING:
		return "DTMF_PAUSE_RECORDING";
	case DTMF_STOP_RECORDING:
		return "DTMF_STOP_RECORDING";
	case DTMF_SECURE_CONF:
		return "DTMF_SECURE_CONF";
	case DTMF_UNSECURE_CONF:
		return "DTMF_UNSECURE_CONF";
	case DTMF_SHOW_PARTICIPANTS:
		return "DTMF_SHOW_PARTICIPANTS";
	case DTMF_REQUEST_TO_SPEAK:
		return "DTMF_REQUEST_TO_SPEAK";
	case DTMF_OPER_ASSISTANCE_PRIVATE:
		return "DTMF_OPER_ASSISTANCE_PRIVATE";
	case DTMF_OPER_ASSISTANCE_PUBLIC:
		return "DTMF_OPER_ASSISTANCE_PUBLIC";
	case DTMF_SHOW_GATHERING:
		return "DTMF_SHOW_GATHERING";
	case DTMF_START_PCM:
		return "DTMF_START_PCM";
	case DTMF_INVITE_PARTY:
		return "DTMF_INVITE_PARTY";
	case DTMF_DISCONNECT_INVITED_PARTICIPANT:
		return "DTMF_DISCONNECT_INVITED_PARTICIPANT";
	case DTMF_LOCK_CONF:
		return "DTMF_LOCK_CONF";
	case DTMF_UNLOCK_CONF:
		return "DTMF_UNLOCK_CONF";
	case DTMF_START_ONHOLD:
		return "DTMF_START_ONHOLD";
	case DTMF_STOP_ONHOLD:
		return "DTMF_STOP_ONHOLD";
	case DTMF_START_VENUS:
		return "DTMF_START_VENUS";
	}
	return "DTMF_UNDEFINED";
}

//--------------------------------------------------------------------------
const char* CIvrCntlLocal::IvrStageToString(WORD stage) {
	switch (stage) {
	case IVR_STAGE_LANGUAGE:
		return "IVR_STAGE_LANGUAGE";
	case IVR_STAGE_NO_VIDEO_RSRC:
		return "IVR_STAGE_NO_VIDEO_RSRC";
	case IVR_STAGE_GENERAL_WELCOME:
		return "IVR_STAGE_GENERAL_WELCOME";
	case IVR_STAGE_NUMERIC_CONF_ID:
		return "IVR_STAGE_NUMERIC_CONF_ID";
	case IVR_STAGE_CONF_PASSWORD:
		return "IVR_STAGE_CONF_PASSWORD";
	case IVR_STAGE_CONF_LEADER:
		return "IVR_STAGE_CONF_LEADER";
	case IVR_STAGE_BILLING_CODE:
		return "IVR_STAGE_BILLING_CODE";
	case IVR_STAGE_PIN_CODE:
		return "IVR_STAGE_PIN_CODE";
	case IVR_STAGE_CONF_WELCOME:
		return "IVR_STAGE_CONF_WELCOME";
	case IVR_STAGE_LOCK_SECURE:
		return "IVR_STAGE_LOCK_SECURE";
	case IVR_STAGE_CHANGE_PWDS_MENU:
		return "IVR_STAGE_CHANGE_PWDS_MENU";
	case IVR_STAGE_CHANGE_CONF_PWD:
		return "IVR_STAGE_CHANGE_CONF_PWD";
	case IVR_STAGE_CHANGE_LEADER_PWD:
		return "IVR_STAGE_CHANGE_LEADER_PWD";
	case IVR_STAGE_CHANGE_PWDS_CONFIRM:
		return "IVR_STAGE_CHANGE_PWDS_CONFIRM";
	case IVR_STAGE_CHANGE_PWDS_OK:
		return "IVR_STAGE_CHANGE_PWDS_OK";
	case IVR_STAGE_CHANGE_PWDS_INVALID:
		return "IVR_STAGE_CHANGE_PWDS_INVALID";
	case IVR_STAGE_ROLL_CALL:
		return "IVR_STAGE_ROLL_CALL";
	case IVR_STAGE_MAX_PARTICIPANTS:
		return "IVR_STAGE_MAX_PARTICIPANTS";
	case IVR_STAGE_CHANGE_TO_LEADER:
		return "IVR_STAGE_CHANGE_TO_LEADER";
	case IVR_STAGE_RECORDING_IN_PROGRESS:
		return "IVR_STAGE_RECORDING_IN_PROGRESS";
	case IVR_STAGE_RECORDING_FAILED:
		return "IVR_STAGE_RECORDING_FAILED";
	case IVR_STAGE_CASCADE_MCU_PARTY:
		return "IVR_STAGE_CASCADE_MCU_PARTY";
	case IVR_STAGE_VIDEO_INVITE:
		return "IVR_STAGE_VIDEO_INVITE";
	case IVR_STAGE_INVITE_PARTY:
		return "IVR_STAGE_INVITE_PARTY";
	case IVR_STAGE_INVITE_PARTY_ADDRESS:
		return "IVR_STAGE_INVITE_PARTY_ADDRESS";
	case IVR_STAGE_INVITE_PARTY_MENU:
		return "IVR_STAGE_INVITE_PARTY_MENU";
	case IVR_STAGE_WAIT:
		return "IVR_STAGE_WAIT";
	}
	return "IVR_STAGE_UNDEFINED";
}
