#include "NStream.h"
#include "ConfApi.h"
#include "Party.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsInternal.h"
#include "ObjString.h"
#include "Trace.h"
#include "Segment.h"
#include "CommConfDB.h"
#include "IVRCntlLocal.h"
#include "IVRCntlExternal.h"
#include "IVRAvMsgStruct.h"
#include "IVRService.h"
#include "IVRServiceList.h"
#include "ConfPartyGlobals.h"
#include "ConfPartyOpcodes.h"
#include "SysConfig.h"
#include "ProcessBase.h"
#include "SysConfigKeys.h"


const WORD TIMER_DELAYED_AUDIO_IVR = 114;

extern std::map<DWORD, PartyMonitorID>* GetMapPartiesTasksIds();

PBEGIN_MESSAGE_MAP(CParty)

	ONEVENT(EXPORT,                              PARTYCONNECTED,     CParty::OnConfExport)
	ONEVENT(SETMOVEPARAMS,                       ANYCASE,            CParty::OnConfSetMoveParams)
	ONEVENT(UPDATE_NO_RESOURCES_FOR_VIDEO_PARTY, ANYCASE,            CParty::OnConfUpdateNoResourcesForVideoParty)

	ONEVENT(DISCONNECTENC,                       PARTYSETUP,         CParty::OnEncDisconnect)
	ONEVENT(DISCONNECTENC,                       PARTYCHANGEMODE,    CParty::OnEncDisconnect)
	ONEVENT(DISCONNECTENC,                       PARTYCONNECTED,     CParty::OnEncDisconnect)

	ONEVENT(PARTY_ENCRYPTION_STATE,              ANYCASE,            CParty::OnUpdateEncryptionState)

	ONEVENT(VCUTOUT,                             PARTYCHANGEMODE,    CParty::OnTimerVcu)
	ONEVENT(VCUTOUT,                             PARTYCONNECTED,     CParty::OnTimerVcu)
	ONEVENT(VCUTOUT,                             ANYCASE,            CParty::NullActionFunction)

	ONEVENT(TIMER_START_IVR,                     PARTYDISCONNECTING, CParty::NullActionFunction)
	ONEVENT(TIMER_START_IVR,                     PARTYCONNECTED,     CParty::OnTimerStartIvr)     // anat - temporary
	ONEVENT(TIMER_START_IVR,                     PARTYCHANGEMODE,    CParty::OnTimerStartIvr)     // anat - temporary
	ONEVENT(TIMER_START_IVR,                     IDLE,               CParty::OnTimerStartIvr)     // anat - temporary
	ONEVENT(TIMER_START_IVR,                     PARTYSETUP,         CParty::OnTimerStartIvr)     // anat - temporary

	ONEVENT(IVR_PARTY_STOP_IVR,                  ANYCASE,            CParty::OnCamStopIvr)
	ONEVENT(TIMER_DELAYED_AUDIO_IVR,             ANYCASE,            CParty::OnTimerAudioDelayed) // anat - temporary

	ONEVENT(SET_PARTY_AS_LEADER,                 PARTYCONNECTED,     CParty::OnConfLeaderChangeStatusConnect)
	ONEVENT(SET_PARTY_AS_LEADER,                 ANYCASE,            CParty::NullActionFunction)

	ONEVENT(LEADER_CHANGED,                      ANYCASE,            CParty::NullActionFunction)
	ONEVENT(FORCE_KILL,                          ANYCASE,            CParty::OnMcuForceKillAnyCase)

	ONEVENT(SEND_H239_VIDEO_CAPS,                ANYCASE,            CParty::NullActionFunction)

	ONEVENT(PARTY_FLOWCONTROL,                   PARTYCONNECTED,     CParty::SendFlowControlToCs)
	ONEVENT(PARTY_FLOWCONTROL,                   ANYCASE,            CParty::NullActionFunction)

	ONEVENT(UPDATELPRACTIVATION,                 ANYCASE,            CParty::OnUpdateLprLocalRmtActivation)
	ONEVENT(UPDATE_LPR_CHANNEL_HEADER,           ANYCASE,            CParty::OnUpdateLprChannelHeader)
	ONEVENT(CONFCONTENTTOKENMSG,                 ANYCASE,            CParty::NullActionFunction)

	ONEVENT(DTMF_IND_PCM,                        ANYCASE,            CParty::ForwardDtmfToPCM)
	ONEVENT(SET_FECC_PARTY_TYPE,                 ANYCASE,            CParty::InformArtOnFeccPartyType)

	//AT&T
	ONEVENT(MCCF_IVR_START_DIALOG,               ANYCASE,            CParty::OnStartDialogParty)
	ONEVENT(ACK_PLAY_MESSAGE,                    ANYCASE,            CParty::OnAckPlayMessage)
	ONEVENT(ACK_SHOW_SLIDE,                      ANYCASE,            CParty::OnAckShowSlide)
	ONEVENT(ACK_RECORD_PLAY_MESSAGE,             ANYCASE,            CParty::OnAckRecordPlayMessage)

	ONEVENT(CAM_TO_IVR_PARTY_MEDIA_CONNECTED,    ANYCASE,            CParty::OnCAMMediaConnection)
	ONEVENT(CAM_TO_IVR_PARTY_MEDIA_DISCONNECTED, ANYCASE,            CParty::OnCAMMediaDisconnection)
// Lync 2013
	ONEVENT(VIDEO_IN_SYNCED,                     ANYCASE,            CParty::NullActionFunction)
PEND_MESSAGE_MAP(CParty, CStateMachine);

////////////////////////////////////////////////////////////////////////////
//                        CParty
////////////////////////////////////////////////////////////////////////////
CParty::CParty()
{
	m_monitorPartyId           = INVALID;
	m_PartyRsrcID              = INVALID;
	m_ConfRsrcId               = INVALID;
	m_numNetChnlCntl           = 0;
	m_pConfApi                 = NULL;
	m_isCleanup                = 0;
	m_disconnectCause          = 0;
	m_name[0]                  = '\0';
	m_numericConfId[0]         = '\0';
	m_pConfRcvMbx              = new COsQueue;
	m_cascadeMode              = CASCADE_MODE_NONE;
	m_rcvVcuCounter            = 0;
	m_lastVcuTime              = 0;
	m_lastContentRefreshTime   = 0;
	m_lastBridgeRefreshTime    = 0;
	m_rcvVcuCounterThreshHold  = 3;
	m_rcvVcuCounterWindowTout  = 5;
	m_cascadeParty             = 0;
	m_ivrCtrl                  = 0;
	m_isLeader                 = NO;
	m_ivrStarted               = NO;
	m_isDelayedIvr             = NO;
	m_isRecordingPort          = NO;
	m_bFromEntryQ              = 0;
	m_AudioOnly                = NO;
	m_bExtDone                 = 0;
	m_mcuNum                   = 0;
	m_termNum                  = 0;
	m_IsGateWay                = 0;
	m_serviceId                = 0;
	m_cntIVRWaitForConnected   = 0;
	m_bCAMReadyForIVR          = 0;
	m_bIvrAfterCallResume      = 0;
	m_isAutoVidBitRate         = 1;
	m_isPreSignalingFlowProb   = 0;
	m_bNoVideRsrcForVideoParty = FALSE;
	m_feccPartyType            = (BYTE)-1;
	m_RoomId                   = 0xFFFF;
	m_pDelayedExternalIVRDialog    = NULL;
	m_delayedExternalIVRDialogTime = 0;
	for (int i = 0; i < MAX_CONF_LEADER; i++)
		m_confsLeader[i] = (WORD)(-1);
	m_IsCallFromGateWay 	= false;

	m_pParty 			= NULL;
	m_pDestConfApi		= NULL;
	m_voice				= NO;
	m_status			= statOK;
	m_interfaceType		= NONE_INTERFACE_TYPE;
	m_nodeType			= CASCADE_MODE_NONE;
	m_isChairEnabled	= YES;  // matches PartyCntl and partyApi->create() defaults
	m_monitorConfId 	= static_cast<ConfMonitorID>(-1);
}

//--------------------------------------------------------------------------
CParty::~CParty()
{
	// BRIDGE-3326
	// The only place in our system that is not protected by a semaphore is a thread SELFKILL flow.
	// Here we access to the global lookup table, so we should lock the access with semaphore
	LockRelevantSemaphore();
	GetLookupTableParty()->Del(m_PartyRsrcID);
	GetLookupIdParty()->Clear(m_PartyRsrcID);
	UnlockRelevantSemaphore();

	POBJDELETE(m_pConfRcvMbx);
	POBJDELETE(m_pConfApi);
	POBJDELETE(m_ivrCtrl);
}

//--------------------------------------------------------------------------
void CParty::OnMcuForceKillAnyCase(CSegment* pParam)
{
	TRACEINTO << "PartyName:" << m_name;

	m_selfKill = TRUE;
}

//--------------------------------------------------------------------------
void CParty::Create(CSegment& appParam)
{
	char confName[2*H243_NAME_LEN+50];

	CTaskApp::Create(appParam); // get default param  & create task

	m_pConfRcvMbx->DeSerialize(appParam);

	appParam >> m_monitorPartyId
	         >> m_monitorConfId
	         >> m_name
	         >> confName
	         >> m_numericConfId
	         >> m_termNum
	         >> m_mcuNum
	         >> m_voice
	         >> m_isChairEnabled
	         >> m_IsGateWay
	         >> m_isRecordingPort
	         >> m_serviceId
	         >> m_isAutoVidBitRate
	         >> m_bNoVideRsrcForVideoParty;

	SetFullName(m_name, confName);

	m_pParty          = this; // needed for PARTYNAME macro
	//m_isRecordingPort = NO;   // it was used for past reasons, we don't need it any more.

	CCommConf* pTmpCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	if (pTmpCommConf)
	{
		CConfParty* pConfParty = pTmpCommConf->GetCurrentParty(m_monitorPartyId);
		if (pConfParty)
			m_AudioOnly = pConfParty->GetVoice();
	}

	UpdateTipParamForIvr(); //IVR for TIP
	// Create IVR Service
	CreateIVRService();

	std::map<DWORD, PartyMonitorID>* taskIdToPartyId = GetMapPartiesTasksIds();
	(*taskIdToPartyId)[GetTaskId()] = m_monitorPartyId;
}

//--------------------------------------------------------------------------
void CParty::PartySelfKill()
{
	TRACEINTO << "PartyName:" << m_name;

	DeleteAllTimers();

	ON(m_selfKill);
}

//--------------------------------------------------------------------------
void CParty::SetFullName(const char* partyName, const char* confName)
{
	if (partyName && confName)
		snprintf(m_partyConfName, sizeof(m_partyConfName), "ConfName:%s, PartyName:%s", confName, partyName);
	else if (partyName)
		snprintf(m_partyConfName, sizeof(m_partyConfName), "PartyName:%s", partyName);
	else if (confName)
		snprintf(m_partyConfName, sizeof(m_partyConfName), "ConfName:%s", confName);
}

//--------------------------------------------------------------------------
BOOL CParty::TaskHandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
#ifdef WINNT
	__try
	{
#endif
	switch (opCode)
	{
		case AUD_DTMF_IND_VAL:
		case FIRST_DTMF_WAS_DIALED:
		case DTMF_STRING_IDENT:
		case DTMF_OPCODE_IDENT:
		case PARTY_INVITE_RESULT_IND:
		case IVR_END_OF_FEATURE:
		case IVR_RESET:
		case IVR_EROR_FEATURE:
		{
			TRACEINTO << PARTYNAME << ", Party Opcode:" << opCode;
			if (m_ivrCtrl)
			{
				if (!m_ivrCtrl->IsExternalIVR())
				{
					if (IVR_END_OF_FEATURE != opCode && DTMF_STRING_IDENT != opCode && DTMF_OPCODE_IDENT != opCode && AUD_DTMF_IND_VAL != opCode)
						ForwardEventToTipSlaves(pMsg, opCode);
				}
				m_ivrCtrl->HandleEvent(pMsg, msgLen, opCode);
			}
			break;
		}

		case IVR_MESSG:
		case IVR_GENERAL_EVENT:
		{
			if (m_ivrCtrl)
			{
				DWORD subOpCode = 0;
				*pMsg >> subOpCode;
				m_ivrCtrl->HandleEvent(pMsg, msgLen, subOpCode);
			}
			break;
		}

		case CAM_READY_FOR_IVR:
		{
			TRACEINTO << PARTYNAME << ", Opcode:" << opCode;
			SetIsCamReadyForIVR(TRUE);



		break;
		}

//				case CAM_READY_FOR_EXTERNAL_IVR:
//				{
//					PTRACE2(eLevelInfoNormal,"CParty::HandleEvent : CAM_READY_FOR_EXTERNAL_IVR, Name - ",PARTYNAME);
//					if (m_ivrCtrl)
//						m_ivrCtrl->HandleEvent(pMsg, msgLen, opCode);
//					break;
//				}
		case XML_EVENT:
		{
			TRACEINTO << PARTYNAME << ", Opcode:" << opCode;
			HandleXMLEvent(pMsg, msgLen, opCode);
			break;
		}

		case SET_GW_DTMF_FORWARD:
		{
			BYTE bSetDTMFForward = FALSE;
			*pMsg >> bSetDTMFForward;
			TRACEINTO << PARTYNAME << ", Opcode:" << opCode << ", SetDTMFForward:" << (int)bSetDTMFForward;
			m_ivrCtrl->SetGwDTMFForwarding(bSetDTMFForward);
			break;
		}

		case SET_INVITED_DTMF_FORWARD:
		{
			BYTE bSetDTMFForward = FALSE;
			*pMsg >> bSetDTMFForward;
			TRACEINTO << PARTYNAME << ", Opcode:" << opCode << ", SetDTMFForward:" << (int)bSetDTMFForward;
			m_ivrCtrl->SetInvitePartyDTMFForwarding(bSetDTMFForward);
			break;
		}

		case GW_DTMF_FORWARD:
		case INVITED_DTMF_FORWARD:
		case IVR_SERVER_DTMF_FORWARD:
		{
			m_ivrCtrl->HandleDtmfForwarding(pMsg);
			break;
		}

		case PCM_CONNECTED:
		{
			m_ivrCtrl->PcmConnected();
			break;
		}

		case PCM_DISCONNECTED:
		{
			m_ivrCtrl->PcmDisconnected();
			break;
		}

		default:
		{                            // all other party api messages
			return FALSE;
			break;
		}
	} // switch

#ifdef WINNT
}
__except(pExptPointer = _exception_info(), iResult = GetHandleExptMode(), iResult)
{
	if (::GetAssertMode() == NO)
		ExceptionHandle(pExptPointer, 1);
	else
		throw;
}
#endif

	return TRUE;
}
//--------------------------------------------------------------------------
void CParty::OnConfEstablishCall(CSegment* pParam)
{
	m_rcvVcuCounterThreshHold = 2;
	m_rcvVcuCounterWindowTout = 15;
}

//--------------------------------------------------------------------------
void CParty::OnConfExport(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	DeleteTimer(VCUTOUT);

	WORD      boardId        = 0;
	COsQueue* pDestConfRcv   = new COsQueue;
	void*     pConfPartyCntl = NULL; // transfer as void pointer to avoid include dependency.
	void*     pConfPartyDesc = NULL;
	EMoveType eCurMoveType   = eMoveDefault;
	WORD      wMoveType      = 0xFF;

	*pParam >> pConfPartyCntl >> pConfPartyDesc >> wMoveType;
	eCurMoveType = (EMoveType)wMoveType;

	pDestConfRcv->DeSerialize(*pParam);
	m_pDestConfApi = new CConfApi;
	m_pDestConfApi->CreateOnlyApi(*pDestConfRcv, NULL, NULL, 1);
	WORD wOldMonitorPartyId = m_monitorPartyId;

	if (pConfPartyDesc)
	{
		m_monitorPartyId = ((CConfParty*)pConfPartyDesc)->GetPartyId();
	}
	else
	{
		PASSERT(1);
		m_monitorPartyId = DUMMY_PARTY_ID;
	}

	char* destConfName = new char[H243_NAME_LEN];
	memset(destConfName, '\0', H243_NAME_LEN);

	if (m_ivrCtrl)
	{
		if (m_cascadeParty)
			m_ivrCtrl->SetCascadeLinkInConf();
		m_ivrCtrl->ResetIvr();
	}

	if (pConfPartyDesc)
		((CConfParty*)pConfPartyDesc)->SetMoveType(eCurMoveType);

	m_bCAMReadyForIVR = 0;      // This member is set to 1 when CAM gets the "Audio Connected" notification

	// fixed bug: When moving IP party conf with IVR, party hear IVR. (Ron 10/03)
	DeleteTimer(TIMER_START_IVR);

	OPCODE   rspOpcode;
	CSegment rspMsg;

	//BRIDGE-14279: if we come from EQ so we not in cascade mode.

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	PASSERT_AND_RETURN(!pCommConf);
	if (YES == pCommConf->GetEntryQ())
	{
		if (pConfPartyDesc)
		{
			((CConfParty*)pConfPartyDesc)->SetSrcTypeIsEQ(TRUE);
			TRACEINTO << "doing move from EQ";
		}
	}

	WORD rc = m_pDestConfApi->ImportParty(pConfPartyCntl, pConfPartyDesc, destConfName, rspMsg, 15*SECOND, rspOpcode);
	if (rc)
	{
		TRACESTRFUNC(eLevelError) << PARTYNAME << ", Status:" << rc << " - Failed to export party";

		m_pConfApi->PartyExport(GetPartyId(), statTout);
		m_monitorPartyId = wOldMonitorPartyId;
	}
	else
	{
		if (rspOpcode != STATUS_OK)
		{
			TRACESTRFUNC(eLevelError) << PARTYNAME << ", rspOpcode:" << rspOpcode << " - Failed to export party";

			m_pConfApi->PartyExport(GetPartyId(), statIllegal);
			m_monitorPartyId = wOldMonitorPartyId;
		}
		else  // okay
		{
			TRACEINTO << PARTYNAME << " - Party exported successfully";

			DWORD OriginalConfId = m_monitorConfId;
			SetFullName(m_name, destConfName);
			m_monitorConfId = ::GetpConfDB()->GetConfId(destConfName);

			m_nodeType = CASCADE_MODE_MCU;

			m_pConfApi->PartyExport(GetPartyId(), statOK);       // ACK previous conference

			m_pConfApi->DestroyOnlyApi();
			POBJDELETE(m_pConfApi);

			m_pDestConfApi->DestroyOnlyApi();
			POBJDELETE(m_pDestConfApi);
			m_pDestConfApi = NULL;

			m_pConfApi = new CConfApi(m_monitorConfId);
			m_pConfApi->CreateOnlyApi(*pDestConfRcv, NULL, NULL, 1);
			if (m_pConfRcvMbx != NULL)
				(*m_pConfRcvMbx) = m_pConfApi->GetRcvMbx();

			// In case of move from 4x4 conference to a regular one (or the opposite)
			// we have to update the the repeated set about its sym mode (mms/cancel_mms)
			const CCommConf* pCommConf     = ::GetpConfDB()->GetCurrentConf(destConfName);
			const CCommConf* pCommCurrConf = ::GetpConfDB()->GetCurrentConf(OriginalConfId);

			if (pCommCurrConf)
			{
				if (pCommCurrConf->GetCascadeEQ())
					if (pCommConf)
					{
						CConfParty* pTmpCConfParty = pCommConf->GetCurrentParty(GetName());
						if (pTmpCConfParty)
							pTmpCConfParty->SetCascadeMode(pCommCurrConf->GetCascadeMode());
					}
			}

			POBJDELETE(m_ivrCtrl);      // delete old IVR
			bool turnOffIvrAfterMove = false;
			if (eCurMoveType != eMoveIntoIvr)
				turnOffIvrAfterMove = true;

			CreateIVRService(turnOffIvrAfterMove);
			if (m_ivrCtrl)
			{
				if (GetIsLeader())
				{
					m_ivrCtrl->SetIsLeader(0, 1);             // update DB only
					CDtmfCollector* pDtmfCollector = m_ivrCtrl->GetDtmfCollector();
					if (pDtmfCollector)
						pDtmfCollector->SetDtmpOpcodePermission(DTMF_LEADER_ACTION);
				}
				m_ivrStarted = NO;
				m_ivrCtrl->SetPartyRsrcID(m_PartyRsrcID);
				if (eCurMoveType != eMoveIntoIvr)
					StartTimer(TIMER_START_IVR, 1);
				else
					StartTimer(TIMER_START_IVR, 2*SECOND);
			}
		}
	}

	POBJDELETE(pDestConfRcv);
	PDELETEA(destConfName);
}

/////////////////////////////////////////////////////////////////////////////
void  CParty::OnTimerStartIvr(CSegment* pParam)
{
	//PASSERT(555777); - should be here
	//TODO remove the if and change to inheritance
	if (1)//strcmp(m_ivrCtrl->GetRTType(),CIvrCntlLocal::GetCompileType()) ==0)
	{
		PTRACE(eLevelInfoNormal,"CParty::OnTimerStartIvr");

		m_cntIVRWaitForConnected++;
		if (m_cntIVRWaitForConnected <= MAX_START_IVR_TIMER_ITER)
		{
			if ((m_ivrCtrl) && (m_bCAMReadyForIVR))
			{
				if(m_isDelayedIvr)
					StartIvrFinallyAudioDelayed();
				else
					StartIvrFinally();
			}
			else
			{
				if(m_isDelayedIvr)
					StartTimer(TIMER_START_IVR, 10);
				else
					StartTimer(TIMER_START_IVR,2*SECOND);
			}
		}
		else
		{
			TRACESTRFUNC(eLevelError) << "MAX_START_IVR_TIMER_ITER exceeded";
			StartIvrFinally();
		}
	}
}

//--------------------------------------------------------------------------
void CParty::OnCamStopIvr(CSegment* pParam)
{
	PASSERT_AND_RETURN(!m_ivrCtrl);

	DWORD restartIVR = 0;
	*pParam >> restartIVR;

	BOOL held_during_ivr = m_ivrCtrl->IsIvrOnHold();
	TRACEINTO << "is restart IVR:" << restartIVR << ",CAM stop during held IVR:" << (int)held_during_ivr;
	if (restartIVR && ! held_during_ivr)
	{
		// return to initial IVR state (cancel leader, delete IVR and start later on from beginning)

		TRACEINTO << "Restart IVR";

		m_ivrCtrl->StopIVR();

		POBJDELETE(m_ivrCtrl);
		CreateIVRService();
		m_ivrStarted = NO;
	}
	else
	{
		TRACEINTO << "Reset IVR parameters";

        m_ivrCtrl->ResetIvr();
	}
}

//--------------------------------------------------------------------------
void CParty::OnConfUpdateNoResourcesForVideoParty(CSegment* pParam)
{
	*pParam >> m_bNoVideRsrcForVideoParty;

	TRACEINTO << PARTYNAME << ", NoVideRsrcForVideoParty:" << (int)m_bNoVideRsrcForVideoParty;

	if (m_ivrCtrl)
		m_ivrCtrl->setNoVideRsrcForVideoParty(m_bNoVideRsrcForVideoParty);
}

//--------------------------------------------------------------------------
void CParty::OnEncDisconnect(CSegment* pParam)
{
	WORD cause;
	*pParam >> cause;

	TRACEINTO << PARTYNAME << ", DisconnectCause:" << cause;

	m_pConfApi->PartyDisConnect(cause, this);
}

//--------------------------------------------------------------------------
void CParty::OnUpdateEncryptionState(CSegment* pParam)
{
	WORD state;
	*pParam >> state;

	TRACEINTO << PARTYNAME << ", EncryptionState:" << state;

	m_pConfApi->UpdateDB(this, PARTY_ENCRYPTION_STATE, state, 1);
}

//--------------------------------------------------------------------------
void CParty::OnTimerVcu(CSegment* pParam)
{
	StartTimer(VCUTOUT, m_rcvVcuCounterWindowTout*SECOND);

	DWORD par = REMOTEVIDEO;
	par <<= 16;

	if (m_rcvVcuCounter < m_rcvVcuCounterThreshHold)
		par |= 1;
	else
	{
		TRACEINTO << PARTYNAME
			<< ", VcuCounter:"           << m_rcvVcuCounter
			<< ", VcuCounterThreshHold:" << m_rcvVcuCounterThreshHold
			<< ", VcuCounterWindowTout:" << m_rcvVcuCounterWindowTout
			<< " - Remote video sync loss";
	}

	m_pConfApi->UpdateDB(this, H221, par, 1);
	m_rcvVcuCounter = 0;
}

//--------------------------------------------------------------------------
void CParty::OnUpdateLprLocalRmtActivation(CSegment* pParam)
{
	DWORD param;
	WORD  updType;
	WORD  externalFlag;

	*pParam >> updType >> param >> externalFlag;

	TRACEINTO << PARTYNAME << ", updType:" << updType << ", param:" << param << ", externalFlag:" << externalFlag;

	m_pConfApi->UpdateDB(this, updType, param, externalFlag);
}

//--------------------------------------------------------------------------
void CParty::OnUpdateLprChannelHeader(CSegment* pParam)
{
	WORD externalFlag;
	BYTE isLprChannelHeader = 0;
	*pParam >>isLprChannelHeader >> externalFlag;

	TRACEINTO << PARTYNAME << ", isLprChannelHeader:" << (int)isLprChannelHeader << ", externalFlag:" << externalFlag;

	m_pConfApi->UpdateChannelsLprHeader(this, isLprChannelHeader);
}

//--------------------------------------------------------------------------
void CParty::CancelIVR()
{
	if (IsValidTimer(TIMER_START_IVR))
		DeleteTimer(TIMER_START_IVR);
}
//--------------------------------------------------------------------------
void CParty::StartIvrIfNeeded()
{
	if (!IsValidTimer(TIMER_START_IVR) && !m_ivrStarted)
		StartIvr();
}

//--------------------------------------------------------------------------
void CParty::StartIvr(BOOL bIsResume /*= FALSE*/)
{
	TRACECOND_AND_RETURN(!m_ivrCtrl, "PartyId:" << m_PartyRsrcID << " - Failed, IVR control doesn't exist");

	TRACEINTO << "PartyId:" << m_PartyRsrcID << ", is delayed: " << (int)m_isDelayedIvr;

	m_ivrCtrl->SetPartyRsrcID(m_PartyRsrcID);

	m_bIvrAfterCallResume = bIsResume;
	if (bIsResume)
		m_ivrStarted = FALSE;
	if (m_isDelayedIvr)
	{
		DWORD delayedIvrSeconds = 0;
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("DELAYED_IVR_FOR_SPECIFIC_EP", delayedIvrSeconds);
		StartTimer(TIMER_START_IVR, delayedIvrSeconds*SECOND); // no delay
	}
	else
	{
		PartySpecfiedIvrDelay();
	}
}
///////////////////////////////////////////////////////////////////////////////
void CParty::StartIvrExternal()
{
	PTRACE( eLevelInfoNormal, "CParty::StartIvrExternal()");

	if (NULL == m_ivrCtrl)
	{
		PTRACE( eLevelInfoNormal, "CParty::StartIvrExternal() - IVR CNTL doesn't exist!");
		return;
	}

	else
		m_ivrCtrl->SetPartyRsrcID(m_PartyRsrcID);

}

//--------------------------------------------------------------------------//
void CParty::StartIvrFinally()
{
	if (m_ivrCtrl && (NO == m_ivrStarted))
	{
		m_ivrStarted = YES;
		m_ivrCtrl->SetPartyRsrcID(m_PartyRsrcID);
		m_ivrCtrl->SetMonitorPartyID(m_monitorPartyId);

		TRACEINTO << PARTYNAME;

		// inform CAM that Party is ready for "ShowSlide"
		if (m_pConfApi)
			m_pConfApi->SendCAMGeneralNotifyCommand(m_PartyRsrcID, IVR_PARTY_READY_FOR_SLIDE, (DWORD)0);

		// VNGR-22650 cascading cascading between conference using cascade EQ (dial EQ_ID##Conf_Id) failed
		BYTE IsPartyInCascadeEQ = 0;
		CCommConf* pTmpCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
		if (pTmpCommConf && pTmpCommConf->GetCascadeEQ())
			IsPartyInCascadeEQ = 1; // this parameter is only in this current EQ Cascade

		// we have some history here:
		// 1) VNGR-19165 - 4.7.10- Slave see IVR slide upon link connection:
		// set to remark /* IsGateway() */ and add m_ivrCtrl->ResetFeaturesList()
		// this was merged to V7.6
		// 2) VNGR-22650 cascading cascading between conference using cascade EQ (dial EQ_ID##Conf_Id) failed
		// added IsPartyInCascadeEQ==0
		// 3) VNGR-22773 - no IVR when calling from PSTN ep to DMA GW and then to v7.6 conference.
		// set to un-remark IsGateway()
		// although it contradict VNGR-19165 fix - there is no other fix
		// VNGR-19165 will work when configured defined master-slave.

		// for GW we set IVR link mode on the fly
        // VNGFE-7683 - For Undefined dial-in cascade link party we set IVR link mode and set party as leader on the fly

        // VNGFE-7934 - H.323 GW dial-in participant - 1)            If chairperson password is required the participant is required to dial chairperson password to become chairperson.

		// BRIDGE-15016 - No IVR and no music for h323 call coming from GW - will be treated as not cascade link for IVR matters (except "SET_CHAIR_PERMISSIONS_TO_H323_GW_WITHOUT_PASSWORD" - see Jira ticket)

		//TRACEINTO << "IsCallFromGateway: " << (DWORD)IsCallFromGateway() << "IsConfOrLeaderPasswordRequired: "<< (DWORD)m_ivrCtrl->IsConfOrLeaderPasswordRequired();

		//BRIDGE-14279
		BYTE isCascadeLinkParty = IsCascadeToPolycomBridge();
		if (IsIsdnGWCallCamesFromEQ() == TRUE)
		{
			isCascadeLinkParty = FALSE;
			m_ivrCtrl->SetCascadeLinkInConf(FALSE);
			(m_ivrCtrl->GetDtmfCollector())->SetDtmpOpcodePermission(DTMF_USER_ACTION);
		}

		TRACEINTO << "isCascadeLinkParty: " << isCascadeLinkParty;

        if(IsCallFromGateway())
        {
			TRACEINTO << " h323 call from Gateway";
			BOOL bSetDtmfLeaderPermissions = 0;
			CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey("SET_CHAIR_PERMISSIONS_TO_H323_GW_WITHOUT_PASSWORD", bSetDtmfLeaderPermissions);
			if ((FALSE == m_ivrCtrl->IsConfOrLeaderPasswordRequired()) && (bSetDtmfLeaderPermissions))
			{
				TRACEINTO << " Set DTMF leader permissions to participant that came from GW";
				m_ivrCtrl->SetIsLeader(0, 0, 1);
			}
		}

        else if ( isCascadeLinkParty && IsPartyInCascadeEQ == 0 && (IsGateway() || IsUndefinedParty()))
		{
			m_ivrCtrl->SetCascadeLinkInConf();
			m_ivrCtrl->ResetFeaturesList();
			m_ivrCtrl->SetIsLeader(0, 0, 1);
			TRACEINTO << PARTYNAME << " - Reset features list for cascade link party";
		}

		if (m_bIvrAfterCallResume)
		{
			TRACEINTO << PARTYNAME << " - Resuming IVR after 'hold'";
			m_bIvrAfterCallResume = FALSE;
			m_ivrCtrl->Resume();
		}
		else
		m_ivrCtrl->Start();

		if (m_pDelayedExternalIVRDialog != NULL)
			StartDelayedExternalIvr();


		/////////////////////// TEST ///////////////////////////
//		TRACEINTO << " Starting IVR test TODO REMOVE";

//			const char* xml_dialog_start_prompt =
//				"<mscivr version=\"1.0\" xmlns=\"urn:ietf:params:xml:ns:msc-ivr\">"
//				  "<dialogstart connectionid=\"abc:xyz\">"
//					"<dialog>"
//					  "<prompt>"
//					   "<media type=\"audio/x-wav\" loc=\"https://dma.ip:8443/media/PolycomWelcomeLog.jpg\"/>"
//					   "</prompt>"
//					"</dialog>"
//				  "</dialogstart>"
//				"</mscivr>";
//
//			const char* xml_dialog_start_collect =
//				"<mscivr version=\"1.0\" xmlns=\"urn:ietf:params:xml:ns:msc-ivr\">"
//					"<dialogstart dialogid=\"1:1864798348\" connectionid=\"abc:xyz\">"
//						"<dialog>"
//						"<prompt>"
//						   "<media type=\"image/x-wav\" loc=\"https://dma.ip:8443/media/PolycomWelcomeLog.jpg\"/>"
//						"</prompt>"
//						  "<collect termchar=\"0\" timeout=\"20s\"/>"
//						"</dialog>"
//					"</dialogstart>"
//				"</mscivr>";
//
//			MscIvr* msg = new MscIvr;
//			msg->ReadFromXmlStream(xml_dialog_start_collect, strlen(xml_dialog_start_collect));
//
//			CSegment* pSeg = new CSegment;
//			void* response = NULL;
//			std::string dialogID = "dialogID";
//			*pSeg << response << dialogID <<msg;
//			OnStartDialogParty(pSeg);
//		//PASSERT(555777);
//		/////////////////////////////////////////////////////////


	}
	else
	{
		TRACEINTO << PARTYNAME << " - Already started started for this party";

		// inform CAM that Party is ready for "ShowSlide" (even if there is no need for start IVR - maybe the slide is needed for "Wait for Chair")
		if (m_pConfApi)
			m_pConfApi->SendCAMGeneralNotifyCommand(m_PartyRsrcID, IVR_PARTY_READY_FOR_SLIDE, (DWORD)0);
	}

}
void CParty::StartDelayedExternalIvr()
{

	TICKS ivr_delay_time = (m_delayedExternalIVRDialogTime != 0)? SystemGetTickCount() - m_delayedExternalIVRDialogTime : 0;
	DWORD delay_msecs = ivr_delay_time.GetMiliseconds();
	TRACEINTO << "party \"" << GetName() << "\" can now handle the delayed external IVR dialog. Passing to IVR cntl after a delay of " << delay_msecs << " msecs";
	StartDialogParty(*m_pDelayedExternalIVRDialog, delay_msecs);
	POBJDELETE(m_pDelayedExternalIVRDialog->baseObject);
	delete m_pDelayedExternalIVRDialog;
	m_pDelayedExternalIVRDialog = NULL;
	m_delayedExternalIVRDialogTime = CSystemTick(0);
}
/*
///////////////////////////////////////////////////////////////////////////////
void CParty::StartIvrFinalyExternal()
{
	if (m_ivrCtrl && (NO == m_ivrStarted))
	{
	    PTRACE2( eLevelInfoNormal, "CParty::StartIvrFinalyExternal - party: ", PARTYNAME);
	    m_ivrStarted = YES;
	    m_ivrCtrl->SetMonitorPartyID( m_monitorPartyId );

	    // vngr-22650 cascading cascading between coference using cascade EQ (dial EQ_ID##Conf_Id) failed
	    BYTE IsPartyInCascadeEQ = 0;
	    CCommConf*  pTmpCommConf = NULL;
	    pTmpCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	    if (pTmpCommConf && pTmpCommConf->GetCascadeEQ())
	    {
	    	PTRACE2( eLevelInfoNormal, "CParty::StartIvrFinally - IsPartyInCascadeEQ = 1 : ", PARTYNAME);
	    	IsPartyInCascadeEQ = 1;	// this parameter is only in this current EQ Cascade
	    }

	    // we have some history here:
	    // 1) VNGR-19165 - 4.7.10- Slave see IVR slide upon link connection:
	    //    set to remark  IsGateway()  and add m_ivrCtrl->ResetFeaturesList()
	    //    this was merged to V7.6
	    // 2) vngr-22650 cascading cascading between coference using cascade EQ (dial EQ_ID##Conf_Id) failed
	    //    added IsPartyInCascadeEQ==0
	    // 3) VNGR-22773 - no ivr when calling from pstn ep to DMA GW and then to v7.6 conference.
	    //    set to un-remark IsGateway()
	    //    although it contradict VNGR-19165 fix - there is no other fix
	    //    VNGR-19165 will work when configured defined master-slave.

	    //only for GW we set ivr link mode on the fly
	    if ( IsGateway() &&  IsCascadeToPolycomBridge() && IsPartyInCascadeEQ==0 )
	    {
	    	m_ivrCtrl->SetLinkParty();
			m_ivrCtrl->ResetFeaturesList();
			PTRACE2( eLevelInfoNormal, "CParty::StartIvrFinally - ResetFeaturesList for cacade link : ", PARTYNAME);
	    }

	    m_ivrCtrl->Start();
	    if (m_pConfApi)
	      m_pConfApi->SendCAMGeneralNotifyCommand( m_PartyRsrcID, IVR_PARTY_READY_FOR_SLIDE, (DWORD)0 );	// inform CAM that Party is ready for "ShowSlide"
	  }

	  else
	  {
	      PTRACE2( eLevelInfoNormal, "CParty::StartIvrFinally - already started, party: ", PARTYNAME);
	      // inform CAM that Party is ready for "ShowSlide" (even if there is no need for start IVR - maybe the slide is needed for "Wait for Chair")
	      if (m_pConfApi)
	    	  m_pConfApi->SendCAMGeneralNotifyCommand( m_PartyRsrcID, IVR_PARTY_READY_FOR_SLIDE, (DWORD)0 );
	  }
}

*/
void CParty::StartIvrFinallyAudioDelayed()
{
	if (m_ivrCtrl && (NO == m_ivrStarted))
	{
		TRACEINTO << PARTYNAME;

		m_ivrStarted = YES;
		m_ivrCtrl->SetMonitorPartyID(m_monitorPartyId);

		// inform CAM that Party is ready for "ShowSlide"
		if (m_pConfApi)
			m_pConfApi->SendCAMGeneralNotifyCommand(m_PartyRsrcID, IVR_PARTY_READY_FOR_SLIDE, (DWORD)0);

		DWORD delayedIvrSeconds = 5;
		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey(CFG_KEY_DELAYED_AUDIO_IVR_FOR_SPECIFIC_EP, delayedIvrSeconds);
		StartTimer(TIMER_DELAYED_AUDIO_IVR, delayedIvrSeconds*SECOND);
	}
	else
	{
		TRACEINTO << PARTYNAME << " - Already started started for this party";

		// inform CAM that Party is ready for "ShowSlide" (even if there is no need for start IVR - maybe the slide is needed for "Wait for Chair")
		if (m_pConfApi)
			m_pConfApi->SendCAMGeneralNotifyCommand(m_PartyRsrcID, IVR_PARTY_READY_FOR_SLIDE, (DWORD)0);
	}
}

//--------------------------------------------------------------------------
void CParty::OnTimerAudioDelayed(CSegment* pParam)
{
	TRACEINTO << PARTYNAME;

	// only for GW we set IVR link mode on the fly
	if (IsGateway() && IsCascadeToPolycomBridge())
		m_ivrCtrl->SetCascadeLinkInConf();

	if (m_ivrCtrl)
	{
		if (m_bIvrAfterCallResume)
		{
			TRACEINTO << PARTYNAME << " - Resuming IVR after 'hold'";
			m_bIvrAfterCallResume = FALSE;
			m_ivrCtrl->Resume();
		}
		else
			m_ivrCtrl->Start();
	}
	if (m_pDelayedExternalIVRDialog != NULL)
		StartDelayedExternalIvr();
}

//--------------------------------------------------------------------------
void CParty::SetMonitorPartyId(CSegment* pMsg)
{
	DWORD Id;
	*pMsg >> Id;
	m_monitorPartyId = Id;
	if (m_ivrCtrl)
		m_ivrCtrl->SetMonitorPartyID(m_monitorPartyId);
}

//--------------------------------------------------------------------------
void CParty::CreateIVRService(bool turnOffIvrAfterMove)
{
	TRACEINTO;
	PASSERTSTREAM_AND_RETURN(m_ivrCtrl != NULL, PARTYNAME << " - Failed, IVR already exists");

	CCommConf* pTmpCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	if (pTmpCommConf)
	{
		// conference and leader password
		WORD        IsPartyInCascadeEQ  = 0; // party in a cascade EQ (IsCascadeMcuParty in MGC)
		const char* pLeaderConfPassword = pTmpCommConf->GetH243Password();
		const char* pConfPassword       = pTmpCommConf->GetEntryPassword();

		if (pTmpCommConf->GetCascadeEQ())
		{
			IsPartyInCascadeEQ = 1; // this parameter is only in this current EQ Cascade
			m_cascadeParty     = 1; // this parameter will live for ever
		}

		eGatewayPartyType GatewayPartyType = eRegularPartyNoGateway;
		CConfParty* pConfParty = pTmpCommConf->GetCurrentParty(m_monitorPartyId);
		if (CPObject::IsValidPObjectPtr(pConfParty))
			GatewayPartyType = pConfParty->GetGatewayPartyType();

		// flag: start conference required leader
		WORD  startConfReqLeader = (WORD)pTmpCommConf->GetStartConfRequiresLeaderOnOff();
		DWORD dwMonitorConfId    = pTmpCommConf->GetMonitorConfId();

		// IVR mode
		//BOOL isExternalIVR = TRUE;
		BOOL isExternalIVR = pTmpCommConf->isExternalIvrControl(); //AT&T
		if(FALSE == isExternalIVR)
		{
			CAvMsgStruct* pMsgStruct = pTmpCommConf->GetpAvMsgStruct();
			if (pMsgStruct)
			{
				const char* serviceName = pMsgStruct->GetAvMsgServiceName();
				if (serviceName)
				{
					CAVmsgService* pAvService = ::GetpAVmsgServList()->GetCurrentAVmsgService( serviceName );
					if (pAvService)
					{
						CIVRService* pIVRService = (CIVRService*) pAvService->GetIVRService();
						if (pIVRService)
						{
							pIVRService->SetName(pAvService->GetName());

							TRACEINTO << PARTYNAME << ", IsPartyInCascadeEQ:" << IsPartyInCascadeEQ;

							m_ivrCtrl = new CIvrCntlLocal(this, *pIVRService, pConfPassword, pLeaderConfPassword,
						                         startConfReqLeader, dwMonitorConfId, (const char*)(NULL), IsPartyInCascadeEQ, GatewayPartyType, m_bNoVideRsrcForVideoParty);
							m_ivrCtrl->SetTurnOffIvrAfterMove(turnOffIvrAfterMove);
							m_ivrCtrl->SetMonitorPartyID(m_monitorPartyId);

							if ((0 == IsPartyInCascadeEQ) && (1 == m_cascadeParty)) // cascade party in regular conference (after move)
								m_ivrCtrl->SetCascadeLinkInConf();

						    m_ivrCtrl->Create(this, m_pConfRcvMbx);
						}
						else
							TRACESTRFUNC(eLevelError) << PARTYNAME << " - Failed, NULL pointer";
					}
					else
						TRACESTRFUNC(eLevelError) << PARTYNAME << " - Failed, NULL pointer";
				}
				else
					TRACESTRFUNC(eLevelError) << PARTYNAME << " - Failed, NULL pointer";
			}
			else
				TRACESTRFUNC(eLevelError) << PARTYNAME << " - Failed, NULL pointer";
		}

		else // External IVR
		{
			PTRACE2INT( eLevelInfoNormal, "CParty::CreateIVRService For External control IsExternalIVR = ", isExternalIVR );
			m_ivrCtrl = new CIvrCntlExternal(this, dwMonitorConfId);
			m_ivrCtrl->Create( this,m_pConfRcvMbx );
			m_ivrCtrl->SetMonitorPartyID(m_monitorPartyId); //fixing the assert
		}
	}
	else
		TRACESTRFUNC(eLevelError) << PARTYNAME << " - Failed, NULL pointer";
}

//--------------------------------------------------------------------------
void CParty::OnConfLeaderChangeStatusConnect(CSegment* pMsg)
{
	BYTE isLeader;
	*pMsg >> isLeader;

	TRACEINTO << PARTYNAME << ", isLeader:" << (int)isLeader;

	if (isLeader == GetIsLeader())
		return;

	if (isLeader)
	{
		SetIsLeader(YES);
		CConfApi* pConfApi = m_pConfApi;
		CSegment  seg;
		seg << (WORD)1;
		pConfApi->UpdateDB(m_pParty, SETISLEADER, (DWORD)1, 1, &seg);
	}
	else
	{
		SetIsLeader(NO);
		CSegment seg;
		seg << (WORD)1;
		m_pConfApi->UpdateDB(m_pParty, SETISLEADER, (DWORD)0, 1, &seg);
	}

	PASSERTSTREAM_AND_RETURN(!m_ivrCtrl, PARTYNAME << " - Failed, IVR is not exists");

	if (isLeader)
		m_ivrCtrl->SetIsLeader(1, 0, 1);
	else
		m_ivrCtrl->CancelSetLeader();
}

//--------------------------------------------------------------------------
BYTE CParty::IsUndefinedParty() const
{
	const CCommConf* pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pCommCurrConf), FALSE);

	CConfParty* pConfParty = pCommCurrConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!CPObject::IsValidPObjectPtr(pConfParty), FALSE);

	return pConfParty->IsUndefinedParty();
}

//--------------------------------------------------------------------------
void CParty::HandleXMLEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	if (m_ivrCtrl)
		m_ivrCtrl->HandleEvent(pMsg, msgLen, opCode);
}

//--------------------------------------------------------------------------
void CParty::SetLastConfIDPartyIsLeader(DWORD confID)
{
	TRACEINTO << PARTYNAME << ", ConfId:" << confID;

	// checking if exists
	int i = 0;
	for (i = 0; i < MAX_CONF_LEADER; i++)
		if (m_confsLeader[i] == confID)
			return;

	// already exists

	// searching for a free place
	for (i = 0; i < MAX_CONF_LEADER; i++)
		if (m_confsLeader[i] == (WORD)(-1))
		{
			m_confsLeader[i] = confID;
			return;
		}

	// if no free space we replace the oldest one
	for (i = 0; i < (MAX_CONF_LEADER-1); i++)
		m_confsLeader[i] = m_confsLeader[i+1];

	m_confsLeader[MAX_CONF_LEADER-1] = confID;
}

//--------------------------------------------------------------------------
void CParty::PartySpecfiedIvrDelay()
{
	TICKS delay = 2*SECOND;
	// This behavior was changed in the ISDN parties
	if (m_ivrCtrl->IsRecordingLinkParty())
		delay = 10;
	else
	{
		if (GetIsTipCall() && (GetTipPartyType() == eTipMasterCenter))
		{
			delay = delay + TIP_MASTER_WAIT_EXTRA_FOR_SLAVE_IVR_START;
		}
	}
	StartTimer(TIMER_START_IVR, delay);
	TRACEINTO << "starting IVR timer for for " << delay.GetMiliseconds() << "ms";
}

//--------------------------------------------------------------------------
void CParty::GetIpCallIdentifiers(IP_EXT_DB_STRINGS* ipStringsStruct)
{
	PASSERT(1);
}

//--------------------------------------------------------------------------
void CParty::SendFlowControlToCs(CSegment* pParam)
{
	TRACEINTO << PARTYNAME << " - Failed, Should call to derived class's function";
}

//--------------------------------------------------------------------------
BYTE CParty::IsCascadeToPolycomBridge()
{
	if (GetCascadeMode() == CASCADE_MODE_MASTER || GetCascadeMode() == CASCADE_MODE_SLAVE)
		return TRUE;
	return FALSE;
}

//--------------------------------------------------------------------------
void CParty::ForwardDtmfToPCM(CSegment* pParam)
{
	if (CPObject::IsValidPObjectPtr(m_pConfApi))
	{
		CSegment* pSeg = new CSegment;
		pSeg->CopySegmentFromReadPosition(*pParam);
		m_pConfApi->ForwardDtmfToPCM(GetPartyRsrcID(), pSeg);

		POBJDELETE(pSeg);
	}
}

//--------------------------------------------------------------------------
void CParty::InformArtOnFeccPartyType(CSegment* pParam)
{
	if (m_feccPartyType == (BYTE)-1) // first message to ART
	{
		BYTE isLeader = NO;
		CSegment* pSeg = new CSegment;
		*pSeg << isLeader;

		DispatchEvent(LEADER_CHANGED, pSeg);

		POBJDELETE(pSeg);
	}
}

//--------------------------------------------------------------------------
BYTE CParty::IsTipCompatibleContent() const
{
	const CCommConf* pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommCurrConf, FALSE);

	BYTE bIsTipCompatibleContent = pCommCurrConf->GetIsTipCompatibleContent();
	TRACEINTO << PARTYNAME << ", bIsTipCompatibleContent:" << bIsTipCompatibleContent;

	return bIsTipCompatibleContent;
}

//--------------------------------------------------------------------------//BRIDGE-13949
BYTE CParty::IsPreferTip() const
{
	const CCommConf* pCommCurrConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommCurrConf, FALSE);

	BYTE bIsPreferTip = pCommCurrConf->GetIsPreferTIP();
	TRACEINTO << PARTYNAME << ", bIsPreferTip:" << (int)bIsPreferTip;

	return bIsPreferTip;
}

//--------------------------------------------------------------------------
void CParty::InformConfOnPacketLossState(const eRtcpPacketLossStatus cellInd, const eRtcpPacketLossStatus layoutInd)
{
	TRACEINTO << PARTYNAME;

	DBGPASSERT_AND_RETURN(m_pConfApi == NULL);
	m_pConfApi->NotifyVbOnNetworkQualityChange(this, cellInd, layoutInd);
}

//--------------------------------------------------------------------------
void CParty::UpdateMuteIconState(EOnOff onOff)
{
	TRACEINTO << " unIsMuteOn " << (int)onOff;
	m_pConfApi->UpdateDB(this,MUTE_STATE,onOff |0xF0000000,1); // only update the icon.
}

//--------------------------------------------------------------------------
void CParty::SendArtUpdateWithSsrcAck(const DWORD aStatus)
{
    TRACEINTO << PARTYNAME << " ACK status= " << aStatus;

    DBGPASSERT_AND_RETURN(m_pConfApi == NULL);
    m_pConfApi->UpdateArtOnTranslateVideoSsrcAck(m_PartyRsrcID, aStatus);
}

//--------------------------------------------------------------------------
void CParty::OnCAMMediaDisconnection(CSegment* pSeg)
{
	if(m_state == PARTYDISCONNECTING)
	{
		PTRACE(eLevelError,"CParty::OnCAMMediaDisconnection: Party is disconnecting ");
		return ;
	}
	if (m_ivrCtrl != NULL && m_ivrCtrl->IsExternalIVR())
	{
		m_ivrCtrl->HandleEvent(pSeg, pSeg->GetLen(), CAM_TO_IVR_PARTY_MEDIA_DISCONNECTED);
	}
	else
		TRACEINTO << " no external IVR control. nothing to do.";

}

//--------------------------------------------------------------------------
void CParty::OnCAMMediaConnection(CSegment* pSeg)
{
	if(m_state == PARTYDISCONNECTING)
	{
		PTRACE(eLevelError,"CParty::OnCAMMediaCconnection: Party is disconnecting ");
		return ;
	}
	if (m_ivrCtrl != NULL && m_ivrCtrl->IsExternalIVR())
	{
		m_ivrCtrl->HandleEvent(pSeg, pSeg->GetLen(), CAM_TO_IVR_PARTY_MEDIA_CONNECTED);
	}
	else
		TRACEINTO << " no external IVR control. nothing to do.";

}

//--------------------------------------------------------------------------
void CParty::OnStartDialogParty(CSegment* pSeg)
{
	DialogState state;
	*pSeg >> state;
	StartDialogParty(state, 0);
}
//--------------------------------------------------------------------------
void CParty::StartDialogParty(DialogState& state, DWORD mcmsDelayInMsecs)
{

	if (!m_ivrStarted)
	{
		BOOL new_dialog = TRUE;
		if (m_pDelayedExternalIVRDialog != NULL)
		{
			// sanity check
			new_dialog = state.dialogID != m_pDelayedExternalIVRDialog->dialogID;
			if (new_dialog)
			{
				PASSERTSTREAM(TRUE, "party \"" << GetName() << "\" received a dialog while an existing delayed dialog exists. overriding previous dialog.");
				POBJDELETE(m_pDelayedExternalIVRDialog->baseObject);
				delete m_pDelayedExternalIVRDialog;
				m_pDelayedExternalIVRDialog = NULL;
			}
			else
			{
				TRACEINTO << "party \"" << GetName() << "\" received a dialog retry of a delayed dialog . resetting delay counter.";
				m_delayedExternalIVRDialogTime = SystemGetTickCount();
			}
		}
		TRACEINTO << "party \"" << GetName() << "\" received a dialog while not in IVR-ready state. dialog will be held until IVR is started.";
		if (new_dialog)
		{
			m_delayedExternalIVRDialogTime = SystemGetTickCount();
			m_pDelayedExternalIVRDialog = new DialogState(state);
			m_pDelayedExternalIVRDialog->baseObject = state.baseObject->NewCopy();
		}
		return;
	}
	PASSERT_AND_RETURN(/*!state.hMccfMsg ||*/ !state.baseObject);

	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogStart* dialog = (DialogStart*)(mscIvr.m_pResponseType);
	TRACEINTO << "state:" << state << ", dialog:" << dialog << '\n' << mscIvr;

	ExternalIvrStartDialog(state, mcmsDelayInMsecs);
}

void CParty::ExternalIvrStartDialog(DialogState& state, DWORD mcmsDelayInMsecs)
{
	PASSERTSTREAM(TRUE, "unexpected opcode:" << EXTERNAL_IVR_DIALOG_START << " should not be received by non-SIP party");
}
//-------------------- remarked for insertion of multiple IVR media feature
/*
//--------------------------------------------------------------------------
void CParty::OnStartDialogParty(CSegment* pSeg)
{
	DialogState state;
	*pSeg >> state;

	PASSERT_AND_RETURN( !state.baseObject);

	MscIvr& mscIvr = *(MscIvr*)(state.baseObject);
	DialogStart* dialog = (DialogStart*)(mscIvr.m_pResponseType);
	TRACEINTO << "state:" << state << ", dialog:" << dialog << '\n' << mscIvr;
	if (dialog->m_dialog.m_collect.IsAssigned())
	{
			BOOL clear_ivr_dtmf_buffer = dialog->m_dialog.m_collect.m_clearDigitBuffer;
			if (clear_ivr_dtmf_buffer)
			{
				TRACEINTO << "clearing previous DTMF buffer.";
				m_ivrCtrl->HandleEvent(NULL, 0, DTMF_IVR_CLEAR_BUFFER);
				clear_ivr_dtmf_buffer = FALSE;
				dialog->m_dialog.m_collect.m_clearDigitBuffer = false;
			}
	}

	if (dialog->m_dialog.m_collect.IsAssigned())
	{
		BOOL clear_ivr_dtmf_buffer = dialog->m_dialog.m_collect.m_clearDigitBuffer;
		if (clear_ivr_dtmf_buffer)
		{
			TRACEINTO << "clearing previous DTMF buffer.";
			m_ivrCtrl->HandleEvent(NULL, 0, DTMF_IVR_CLEAR_BUFFER);
			clear_ivr_dtmf_buffer = FALSE;
			dialog->m_dialog.m_collect.m_clearDigitBuffer = false;
		}
	}

	if (dialog->m_dialog.m_prompt.IsAssigned())
	{
		std::list<MediaElementType>::const_iterator it = dialog->m_dialog.m_prompt.m_media.begin();
		std::list<MediaElementType>::const_iterator it_end = dialog->m_dialog.m_prompt.m_media.end();

		for ( ; it != it_end; ++it)
			PlayFileExternalIvr(state, *it);
	}

	if (dialog->m_dialog.m_collect.IsAssigned())
		CollectDigitsExternalIvr(state, dialog->m_dialog.m_collect);
}
*/
void CParty::CollectDigitsExternalIvr(DialogState& state, const CollectElementType& collect)
{
	TRACEINTO;
}

void CParty::PlayFileExternalIvr(DialogState& state, const MediaElementType& media)
{
	TRACEINTO;
}

void CParty::OnAckPlayMessage(CSegment* pSeg)
{
	m_ivrCtrl->RecivedPlayMessageAck();
}

void CParty::OnAckShowSlide(CSegment* pSeg)
{
	TRACEINTO;
	m_ivrCtrl->RecivedShowSlideAck();
}

void CParty::OnAckRecordPlayMessage(CSegment* pSeg)
{
	TRACEINTO;
}

//BRIDGE-14279
BYTE CParty::IsIsdnGWCallCamesFromEQ()
{
	BYTE res = FALSE;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_monitorConfId);
	PASSERT_AND_RETURN_VALUE(!pCommConf, res);
	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_monitorPartyId);
	PASSERT_AND_RETURN_VALUE(!pConfParty, res);
	if (pConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE && pConfParty->IsSrcTypeIsEQ() == TRUE) //we are NOT cascade
		res = TRUE;
	else
		res = FALSE;


	TRACEINTO << "res: " << (DWORD)res << ", pConfParty->IsSrcTypeIsEQ(): " << pConfParty->IsSrcTypeIsEQ();

	return res;
}

const CConfParty* CParty::GetConfParty() const
{
	CCommConfDB*	pConfDB		= ::GetpConfDB();
	CCommConf*		pCommConf	= NULL;
	CConfParty*		pConfParty	= NULL;

	if (pConfDB)	pCommConf	= pConfDB->GetCurrentConf(GetMonitorConfId());
	if (pCommConf)	pConfParty	= pCommConf->GetCurrentParty(GetMonitorPartyId());

	return pConfParty;
}

CConfParty* CParty::GetConfPartyNonConst()
{   //May be implemented also by const-casting GetConfParty above, but such pointer conversion may cause an exception in some compilers
	CCommConfDB*	pConfDB		= ::GetpConfDB();
	CCommConf*		pCommConf	= NULL;
	CConfParty*		pConfParty	= NULL;

	if (pConfDB)	pCommConf	= pConfDB->GetCurrentConf(GetMonitorConfId());
	if (pCommConf)	pConfParty	= pCommConf->GetCurrentParty(GetMonitorPartyId());

	return pConfParty;
}

void CParty::PendRecapOnToken(const CSegment* recapInfo)
{
	//=================
	// General sanity
	//=================
	PTRACE(eLevelInfoNormal,"CParty::PendRecapOnToken - pending a recap/reinvite due to current token handling.");
	CConfParty* pConfParty = GetConfPartyNonConst();
	PASSERT_AND_RETURN(!pConfParty || !recapInfo);

	//====================
	// Saving recap info
	//====================
	pConfParty -> PendTokenRecapDueToCollisionDetection();
	m_recapPendedOnToken = *recapInfo;
}
