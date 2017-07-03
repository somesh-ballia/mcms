#include "ConfAppMngr.h"
#include "StatusesGeneral.h"
#include "ConfApi.h"
#include "TaskApp.h"
#include "ConfAppMngrInitParams.h"
#include "BridgePartyInitParams.h"
#include "ConfAppBridgeParams.h"
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "ConfPartyGlobals.h"
#include "IVRService.h"
#include "IVRServiceList.h"
#include "PartyApi.h"		///TEMP
#include "TraceStream.h"
#include "ConfPartyDefines.h"
#include "AudHostApiDefinitions.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsVideo.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "ConfAppFeatureObject.h"
#include "BridgePartyDisconnectParams.h"
#include "ObjString.h"
#include "SysConfig.h"
#include "SysConfigKeys.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "ConfPartyManagerLocalApi.h"
#include "Party.h"
#include "PrettyTable.h"

/////////////////////////////////////////////////////////////////
#define APP_IDLE              1
#define APP_CONNECT           2
#define APP_TERMINATING       3

#define TIMER_START_RECORDING 1
#define TIMER_DISCONNECT_RL   2
#define TIMER_SEND_START_DTMF 3
#define TIMER_PRINT_LOGGER_NUMBER		4
#define TIMER_PRINT_LOGGER_NUMBER_TIME 	120*SECOND

//eFeatureRssDialin
#define  TIMER_RECORDING_CONTROL    		5
#define  TIMER_RECORDING_CONTROL_LEN		10*SECOND

#define CONFREQ               1
#define PARTYREQ              2

/////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CConfAppMngr)
	ONEVENT(EVENT_NOTIFY             , ANYCASE         , CConfAppMngr::HandleNotifyEvents)
	ONEVENT(EVENT_ACTION             , APP_CONNECT     , CConfAppMngr::OnActionActionEvents)
	ONEVENT(EVENT_ACTION             , APP_TERMINATING , CConfAppMngr::CAMNullActionFunction)
	ONEVENT(TIMER_START_RECORDING    , ANYCASE         , CConfAppMngr::OnTimerStartRecording)
	ONEVENT(TIMER_DISCONNECT_RL      , ANYCASE         , CConfAppMngr::OnTimerDisconnectRL)
	ONEVENT(TIMER_SEND_START_DTMF    , ANYCASE         , CConfAppMngr::OnTimerSendStartDtmf)
	ONEVENT(TIMER_PRINT_LOGGER_NUMBER, ANYCASE         , CConfAppMngr::OnTimerPrintLoggerNumber)
	ONEVENT(TIMER_RECORDING_CONTROL  , ANYCASE         , CConfAppMngr::OnTimerRecordingControlTOUT)
PEND_MESSAGE_MAP(CConfAppMngr, CStateMachine);

/////////////////////////////////////////////////////////////////
CConfAppMngr::CConfAppMngr()
{
	m_confAppInfo          = new CConfAppInfo;
	m_partiesList          = new CConfAppPartiesList(m_confAppInfo);
	m_waitList             = new CConfAppWaitEventsList;
	m_eventsList           = new CConfAppActiveEventsList(m_confAppInfo, m_partiesList);
	m_pcmAutorizationLevel = ePcmNone;
}

/////////////////////////////////////////////////////////////////
CConfAppMngr::~CConfAppMngr()
{
	POBJDELETE(m_partiesList);
	POBJDELETE(m_waitList);
	POBJDELETE(m_eventsList);
	POBJDELETE(m_confAppInfo);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::Create(const CConfAppMngrInitParams* pConfAppMngrInitParams)
{
	m_confAppInfo->Create(pConfAppMngrInitParams);
	m_state = APP_CONNECT;
	SetPcmAutorizationLevel();
	StartTimer(TIMER_PRINT_LOGGER_NUMBER, 10);
}

/////////////////////////////////////////////////////////////////
void* CConfAppMngr::GetMessageMap()
{
	return m_msgEntries;
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::HandleNotifyEvents(CSegment* pParam)
{
	DWORD opcode;
	*pParam >> opcode;

	if (ACK_IND == opcode)
		opcode = e_ACK_IND;

	std::ostringstream msgOpcode;
	std::ostringstream msgParams;
	msgOpcode << "Opcode:" << CAM_NotifyOpcodeToString(opcode) << "(#" << opcode << ")";

	// Get PartyRsrcID for some events
	PartyRsrcID PartyId = 0;
	CTaskApp* pParty = NULL;

	char partyName[H243_NAME_LEN];
	CArrayWrapper partyNameWrapper(partyName);

	WORD mediaDirection = eNoDirection;
	bool partyWithIVR = true;
	bool isExternalIVRRequest = false;
	// hold/resume IVR params
	BOOL isIvrResumeMode = FALSE;
	WORD isIvrOnHold = FALSE;
	std::string externalIVRFileName;

	if (GetState() == APP_TERMINATING)
		TRACEINTO << "Opcode:" << (int)opcode << ", State:" << GetStateAsString();

	CSegment *pTmpSeg = NULL;
	switch (opcode)
	{
		case eCAM_EVENT_PARTY_END_LAST_FEATURE:
		case IVR_PARTY_READY_FOR_SLIDE:
		{
			const DWORD offset = pParam->GetRdOffset();

			*pParam >> PartyId;
			msgOpcode << ", PartyId:" << PartyId;

			pParam->ResetRead(offset);
			break;
		}
		case PARTY_ON_HOLD_IND:
		case PARTY_IVR_MODE_ON_RESUME:
		case PARTY_CONNECT_AUDIO_ON_RESUME:
		{
			const DWORD offset = pParam->GetRdOffset();

			*pParam
			>> PartyId
			>> (void*&)pParty
			>> partyNameWrapper
			>> mediaDirection;

			msgOpcode << ", PartyId:" << PartyId;
			msgParams << ", PartyName:" << partyName << ", MediaDirection:" << mediaDirection;

			isIvrResumeMode = TRUE;
			if (opcode == PARTY_IVR_MODE_ON_RESUME)	// it is be necessary to initiate a 'PARTY_CONNECT_AUDIO_ON_RESUME' after handling 'PARTY_IVR_MODE_ON_RESUME'
			{
				COsQueue partyQueue;
				partyQueue.DeSerialize(*pParam);
				*pParam >> isIvrOnHold;
				// construct the segment for PARTY_CONNECT_AUDIO_ON_RESUME
				pTmpSeg = new CSegment();
				*pTmpSeg << (DWORD)PARTY_CONNECT_AUDIO_ON_RESUME;;
				*pTmpSeg << (DWORD) PartyId;
				*pTmpSeg << (void*)pParty;
				*pTmpSeg <<  partyNameWrapper.get();
				*pTmpSeg << (WORD)mediaDirection;
				partyQueue.Serialize(*pTmpSeg);
			}

			partyWithIVR = IsPartyWithIVR(partyName, isIvrResumeMode);

			pParam->ResetRead(offset);
			break;
		}
		case PARTY_AUDIO_CONNECTED:
		case PARTY_VIDEO_CONNECTED:
		case PARTY_VIDEO_IVR_MODE_CONNECTED:
		{
			const DWORD offset = pParam->GetRdOffset();

			*pParam
				>> PartyId
				>> (void*&)pParty
				>> partyNameWrapper
				>> mediaDirection;

			msgOpcode << ", PartyId:" << PartyId;
			msgParams << ", PartyName:" << partyName
				<< ", MediaDirection:" << MediaDirectionToString((EMediaDirection)mediaDirection);

			partyWithIVR = IsPartyWithIVR(partyName, isIvrResumeMode);
			if (opcode == PARTY_VIDEO_IVR_MODE_CONNECTED)
			{
				*pParam >> externalIVRFileName;
				if (externalIVRFileName.length() == 0)
					externalIVRFileName.clear();
				isExternalIVRRequest = !externalIVRFileName.empty();
				msgParams << ", ExternalIVRFileName:" << externalIVRFileName;
			}

			pParam->ResetRead(offset);
			break;
		}

		case eCAM_EVENT_PARTY_END_IVR:
		case PARTY_AUDIO_DISCONNECTED:
		case PARTY_VIDEO_DISCONNECTED:
		case eCAM_EVENT_PARTY_DELETED:
		case PARTY_AUDIO_EXPORTED:
		case PARTY_VIDEO_EXPORTED:
		case REJECT_PLC:
		case eCAM_EVENT_PARTY_AUDIO_DISCONNECTING:
		case eCAM_EVENT_PARTY_VIDEO_DISCONNECTING:
		case eCAM_EVENT_PARTY_IS_CASCADE_LINK:
		case eCAM_EVENT_PARTY_IS_TIP_SLAVE:
		case eCAM_EVENT_PARTY_UPDATE_GW_TYPE:
		{
			*pParam >> PartyId;
			msgOpcode << ", PartyId:" << PartyId;
			break;
		}

		case IVR_PARTY_START_MOVE_FROM_EQ:
		{
			*pParam >> PartyId >> partyNameWrapper;
			msgOpcode << ", PartyId:" << PartyId;
			msgParams << ", PartyName:" << partyName;
			break;
		}

		case IVR_PARTY_START_MOVE_FROM_CONF:
		{
			*pParam >> PartyId >> partyNameWrapper;
			msgOpcode << ", PartyId:" << PartyId;
			msgParams << ", PartyName:" << partyName;
			break;
		}

		case eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE:
		case SEQUENCE_NUM_IND:
		case e_ACK_IND:
		{
			*pParam >> PartyId;          // get partyRsrcID

			if (INVALID == PartyId)      // indication on conf request
			{
				msgOpcode << ", PartyId:INVALID";
				msgParams <<  " - Indication on CONF request";
			}
			else
			{
				*pParam >> (void*&)pParty;
				*pParam >> partyNameWrapper;
				*pParam >> mediaDirection;
				msgOpcode << ", PartyId:" << PartyId;
				msgParams << ", PartyName:" << partyName << ", MediaDirection:" << mediaDirection << " - Indication on PARTY request";
			}
			break;
		}
	}

	WORD confWithLocalIVR    = m_confAppInfo->GetConfWithIVR();
	WORD confWithExternalIVR = m_confAppInfo->GetIsExternalIVRInConf();

	TRACEINTO
		<< msgOpcode.str()
		<< "LocalIVR:" << confWithLocalIVR << ", ExternalIVR:" << confWithExternalIVR
		<< msgParams.str();

	switch (opcode)
	{
		case TERMINATECONF:
		{
			// eCAM_EVENT_CONF_TERMINATING
			m_state = APP_TERMINATING;
			m_confAppInfo->SetConfState(eAPP_CONF_STATE_TERMINATING);
			m_waitList->DelAllEvents();
			m_eventsList->RemoveAllIVREventsUponConfTerminating();
			break;
		}

		case eCAM_EVENT_CONF_TERMINATED:
		{
			m_confAppInfo->SetConfState(eAPP_CONF_STATE_TERMINATED);
			break;
		}

		case PARTY_CONNECT_AUDIO_ON_RESUME:
		case PARTY_AUDIO_CONNECTED:
		{
			if (partyWithIVR && eMediaInAndOut == mediaDirection)
			{
				m_partiesList->AddOrUpdateParty(eCAM_EVENT_PARTY_AUDIO_CONNECTED,
					confWithLocalIVR, confWithExternalIVR, pParam); // the party (video) may already exists

				int partyIndex = m_partiesList->FindPartyIndex(PartyId);

				if (-1 != partyIndex)
				{
					TRACEINTO << msgOpcode.str() << ", PartyIndex:" << partyIndex
						<< ", AudioState:" << AppPartyStateToString((TAppPartyState)m_partiesList->m_partyList[partyIndex]->GetPartyAudioState());

					if (eAPP_PARTY_STATE_IVR_ENTRY == m_partiesList->m_partyList[partyIndex]->GetPartyAudioState())
					{
						// Start IVR entrance (in case of a new party or party's state IDLE - after update changed to IVR_ENTRY)
						DoCAMRequest(PARTYREQ, 0, eCAM_EVENT_PARTY_IVR_ENTRANCE, PartyId);

						CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
						BYTE isConfTIP = pCommConf ? pCommConf->GetIsTipCompatibleVideo() : 0;
						bool isAuxParty = false;

						if (isConfTIP)
						{
							CConfParty* pConfParty = pCommConf->GetCurrentPartyAccordingToVisualName(partyName);

							if (pConfParty)
								isAuxParty =  pConfParty->IsTIPAuxParty();
						}

						//BRIDGE-10054, for local IVR, tip aux party could enter IVR stage with event eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER
						//So do not start ivr in this stage to avoid no audio
						if (isAuxParty && confWithLocalIVR)
							TRACEINTO<<"For local IVR, Do not start IVR for TIP Aux party name:" << partyName;
						else
							m_eventsList->DoPartyAction(eCAM_EVENT_PARTY_START_IVR, PartyId, CAM_START_FEATURE);
					}
				}

				if (confWithLocalIVR)
				{
					int partyIndex = m_partiesList->FindPartyIndex(PartyId);
					if (-1 != partyIndex)
					{
						TRACEINTO << msgOpcode.str().c_str() << ", LocalPartyIndex:" << partyIndex << ", LocalAudioState:" << m_partiesList->m_partyList[partyIndex]->GetPartyAudioState();

						if (eAPP_PARTY_STATE_DISCONNECTED == m_partiesList->m_partyList[partyIndex]->GetPartyAudioState())
						{
							// Change audio state to IVR_ENTRY
							m_partiesList->SetPartyAudioState(PartyId, eAPP_PARTY_STATE_IVR_ENTRY);

							// Start IVR entrance
							DoCAMRequest(PARTYREQ, 0, eCAM_EVENT_PARTY_IVR_ENTRANCE, PartyId);
							m_eventsList->DoPartyAction(eCAM_EVENT_PARTY_START_IVR, PartyId, CAM_START_FEATURE);

							// Start eCAM_EVENT_PARTY_END_IVR
							bool isLeader = m_partiesList->m_partyList[partyIndex]->GetIsPartyLeader();
							CSegment seg;
							seg << (DWORD)eCAM_EVENT_PARTY_END_IVR << PartyId << (BYTE)isLeader;
							HandleNotifyEvents(&seg);
						}

						// In case of a Recording Link
						if (IsRecordingLinkParty(PartyId))
						{
							if (YES == m_confAppInfo->GetRecordingFailedFlag()) // failed_flag == YES, i.e. Recording Link has managed to connect after it was considered to be failed
							{
								DeleteTimer(TIMER_START_RECORDING);                // just in case
								m_confAppInfo->SetRecordingFailedFlag(NO);
								// Update recording status in DB
								UpdateRecordingControl(eStartRecording);
								// Update CDR
								SendControlRecordingCDR(eStartRecording, PartyId); // CDR
							}
							else                                                 // failed_flag == NO, i.e. Ordinary connection
							{
								DeleteTimer(TIMER_START_RECORDING);                // Recording Link was connected on time :)
								// starting timer for sending DTMF to start the recording is in eCAM_EVENT_PARTY_END_IVR
								// Update CDR
								SendControlRecordingCDR(eStartRecording, PartyId); // CDR
							}
						}
					}
				}
			}
			break;
		}

		case PARTY_VIDEO_CONNECTED:
		{
			if ((partyWithIVR && (eMediaInAndOut == mediaDirection)) || (eMediaOut == mediaDirection))
			{
				m_partiesList->AddOrUpdateParty(eCAM_EVENT_PARTY_VIDEO_CONNECTED, confWithLocalIVR,confWithExternalIVR, pParam);             // the party (audio) may already exists
			}
			break;
		}
		case PARTY_ON_HOLD_IND:
		{
			//BOOL on_hold_during_ivr_slide = isIvrOnHold || m_confAppInfo->IsInWaitForChairNow();
			WORD partyStillNeedsToWaitForChair = (WORD)(m_confAppInfo->IsInWaitForChairNow() || (m_confAppInfo->IsWaitForChair() && !m_confAppInfo->IsConfPastWaitForChairStage()));
			*pParam << partyStillNeedsToWaitForChair;
			if ((/*partyWithIVR &&*/ (eMediaInAndOut == mediaDirection)) || (eMediaOut == mediaDirection))
			{
				m_partiesList->AddOrUpdateParty(eCAM_EVENT_PARTY_ON_HOLD, confWithLocalIVR,confWithExternalIVR, pParam);             // the party (audio) may already exists
			}
			break;
		}
		case PARTY_IVR_MODE_ON_RESUME:	// for ivr update
		case PARTY_VIDEO_IVR_MODE_CONNECTED:                                                                    // video bridge is ready to ShowSlide
		{
			if ((partyWithIVR && (eMediaInAndOut == mediaDirection)) || (eMediaOut == mediaDirection))
			{

				// BRIDGE-14692 recap may have caused v.bridge disconnect-reconnect during IVR.
				// check if party is reconnecting during IVR (as part of recaps), to re-enable "ready for slide".
				//if it's new the audio state value will be eAPP_PARTY_STATE_MAX
				CConfAppPartyParams* party_params =  m_partiesList->GetParty(PartyId);
				if (party_params != NULL && !isIvrResumeMode)
				{
					TRACEINTO << "reconnection to videos bridge scenario requires setting IVR \"ready for slide\" flag, since \"OnNotifyPartyReadyForSlide\" will not be called";
					TAppPartyState audio_state = (TAppPartyState)party_params->GetPartyAudioState();
					if ((eAPP_PARTY_STATE_IVR_ENTRY == audio_state) || (m_confAppInfo->IsWaitForChair()))
						party_params->SetPartyReadyForSlide(true);
					//					if (m_confAppInfo->IsWaitForChair())
					//						m_eventsList->DoPartyAction(eCAM_EVENT_PARTY_START_IVR, PartyId, CAM_START_FEATURE);
				}
				// BRIDGE-8051 - resuming from hold- check if slide is necessary immediately or the hold scenario was after video initiated.
				BOOL begin_with_slide = !isIvrResumeMode /* this is the regular case */ ||
										(isIvrOnHold || m_confAppInfo->IsInWaitForChairNow()) /* this is the 'resume call into IVR slide' case */;
				if (begin_with_slide)
					m_partiesList->AddOrUpdateParty(eCAM_EVENT_PARTY_VIDEO_IVR_MODE_CONNECTED, confWithLocalIVR, confWithExternalIVR, pParam);    // the party (video) may already exists

				// BRIDGE-8051 - when executing resume into IVR, the video bridge needs to be informed not to move the party into the mix
				// (the party image will be moved into the mix when IVR/wait for chair is completed)
				BOOL isResumeIntoIVR = isIvrResumeMode && begin_with_slide;
				if (isIvrResumeMode)
					TRACEINTO << "scenario requires resuming IVR slide: " << (int)begin_with_slide;
				if (isResumeIntoIVR)
				{
					CSegment cseg;
					cseg << isResumeIntoIVR;
					m_confAppInfo->m_pVideoBridgeInterface->IvrPartyCommand(PartyId, PARTY_RESUMING_FROM_HOLD, &cseg);
				}
				if (confWithLocalIVR || isExternalIVRRequest)
				{
					int ind = m_partiesList->FindPartyIndex(PartyId);
					TRACECOND_AND_RETURN(-1 == ind, msgOpcode.str() << " - Party doesn't exist in list");

					// in cascade EQ slide is not shown
					TRACECOND_AND_RETURN(m_confAppInfo->GetIsEntryQueue() && m_confAppInfo->IsCascadeEQ(), msgOpcode.str() << " - Cascade EQ: Doesn't show slide");

					if (!isIvrResumeMode)
						TRACECOND_AND_RETURN((1 == m_partiesList->IsVideoBridgeReadyForSlide(ind)) && externalIVRFileName.empty(), "PARTY_VIDEO_IVR_MODE_CONNECTED has already been received and handled");

					bool isSlideNoLongerPermitted = isIvrResumeMode ? false : m_partiesList->m_partyList[ind]->GetSlideIsNoLongerPermitted();
					if (m_confAppInfo->IsNeedToBlockIVR())
						isSlideNoLongerPermitted = true;

					DWORD isSlideNeeded = IsSlideNeeded(PartyId);
					std::ostringstream msg;
					msg << msgOpcode.str().c_str() << ", IsSlideNeeded:" << isSlideNeeded
						<< ", IsSlideBlocked:" << (int)isSlideNoLongerPermitted;

					if ((isSlideNeeded || (isExternalIVRRequest)) && (0 == isSlideNoLongerPermitted))
					{
						m_partiesList->SetVideoBridgeReadyForSlide(ind, 1);
						// Check if party is ready for slide; if yes - show slide if needed
						bool isPartyReadyForSlide = m_partiesList->IsPartyReadyForSlide(ind);
						msg << ", IsPartyReadyForSlide:" << (WORD)isPartyReadyForSlide;
						if (1 == isPartyReadyForSlide) // party ready for slide
						{
							int partyVideoState = m_partiesList->GetPartyVideoState(PartyId);
							msg << ", PartyVideoState:" << partyVideoState;
							if ((eAPP_PARTY_STATE_DISCONNECTED != partyVideoState) // party in DELETED state is in the partyDeletedList
							    && (eAPP_PARTY_STATE_MOVING != partyVideoState))
							{
								if (isExternalIVRRequest)
								{
									msg << ", ExternalIVRFileName:" << externalIVRFileName;
									CSegment* pSeg = new CSegment();
									*pSeg << externalIVRFileName;
									m_eventsList->DoPartyAction(eCAM_EVENT_PARTY_SHOW_SLIDE, PartyId, CAM_START_FEATURE, pSeg);
								}
								else
									m_eventsList->DoPartyAction(eCAM_EVENT_PARTY_SHOW_SLIDE, PartyId, CAM_START_FEATURE);   // show slide

								TRACEINTO << msg.str().c_str() << " - VNGR-21248, Sending eCAM_EVENT_PARTY_SHOW_SLIDE to party";
							}
							else
							{
								TRACEINTO << msg.str().c_str() << " - VNGR-21248, Do not send eCAM_EVENT_PARTY_SHOW_SLIDE to party";
							}
						}
						else
						{
							TRACEINTO << msg.str().c_str() << " - VNGR-21248, Party is not ready for slide";
						}
					}
					else
					{
						TRACEINTO << msg.str();
						m_eventsList->DoPartyAction(eCAM_EVENT_PARTY_JOIN_CONF_VIDEO, PartyId, CAM_START_FEATURE);  // join conf video
						InformPcmPartyAdded(PartyId);
					}
				}
				if (isResumeIntoIVR)	// BRIDGE-8051 IVR resume requires an additional event
				{
					TRACEINTO << "resuming IVR for partyId:" << PartyId << ". initiating PARTY_AUDIO_CONNECTED event.";
					m_partiesList->SetPartyAudioState(PartyId, eAPP_PARTY_STATE_IDLE);
					HandleNotifyEvents(pTmpSeg);
					POBJDELETE(pTmpSeg);
				}
			}
			break;
		}

		case IVR_PARTY_READY_FOR_SLIDE: // party task is ready for slide
		{
			OnNotifyPartyReadyForSlide(PartyId);
			break;
		}

		case eCAM_EVENT_PARTY_END_IVR:
		{
			// In case the conference should be recorded (Start Recording Immediately)
			if (TRUE == ShouldStartRecordingImm(PartyId) && (!IsRecordingLinkParty(PartyId)))
			{
				// In case this is the first party in the conference and there is no RL in the conf yet
				if ((!m_confAppInfo->IsFirstPartyMessagePlayed()) && (0 == m_confAppInfo->GetRecordingLinkInConf()))
				{
					// Add and connect Recording Link (muteVideo = YES)
					m_confAppInfo->m_pConfApi->AddAndConnectRecordingLink(YES);
					// Update recording status in DB
					UpdateRecordingControl(eStartRecording);
					// Update CDR - will be updated only when RL is connected!!
					UpdateFirstPartyMessagePlayedIfNeeded();      // in case the IVR message will be blocked for a certain feature
					// Start timer
					StartTimer(TIMER_START_RECORDING, 20*SECOND); // /TBD ???
				}
			}

			// In case of a Recording Link - send DTMF to RSS for Start Recording (*2)
			if (YES == IsRecordingLinkParty(PartyId) && (NO == IsPartyDialin(PartyId)))
			{
				// Set party as Recording Link in parties List + save RL ID in ConfAppInfo
				m_partiesList->SetIsRecordingLinkParty(PartyId, true);
				m_confAppInfo->SetRecordingLinkInConf(1);
				m_confAppInfo->SetRecordingLinkRsrcId(PartyId);

				// Set "failed" and "initiator" flags to their default values
				m_confAppInfo->SetRecordingFailedFlag(NO);
				m_confAppInfo->SetRecordingInitiatorId((DWORD)(-1));
				m_confAppInfo->SetRlDisconnectionWasReceived(NO);

				WORD lastReqRecAction = m_confAppInfo->GetLastReqRecAction();

				if ((eCAM_EVENT_CONF_START_RESUME_RECORDING == lastReqRecAction) ||
					(SET_START_RECORDING == lastReqRecAction) ||
					(SET_RESUME_RECORDING == lastReqRecAction))
				{
					// Start timer - a delay between connecting the RL and sending DTMF to RSS for starting the recording
					CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
					DWORD rlStartRecordingTimerDuration = 0;
					sysConfig->GetDWORDDataByKey(CFG_KEY_RECORDING_LINK_START_RECORDING_TIMER_DURATION, rlStartRecordingTimerDuration);

					StartTimer(TIMER_SEND_START_DTMF, rlStartRecordingTimerDuration*SECOND);

				}

				if (eCAM_EVENT_CONF_PAUSE_RECORDING == lastReqRecAction || SET_PAUSE_RECORDING == lastReqRecAction)
				{
					TRACEINTO << msgOpcode.str() << " - Pause recording during started";
					PauseRecordingDuringStarted();
				}

				if (eCAM_EVENT_CONF_STOP_RECORDING == lastReqRecAction || SET_STOP_RECORDING == lastReqRecAction)
				{
					TRACEINTO << msgOpcode.str() << " - Stop recording during started";
					StopRecordingDuringStarted();
				}
			}

			//eFeatureRssDialin -- for Dial in case
			if((YES == IsRecordingLinkParty(PartyId))  && (YES == IsPartyDialin(PartyId)))
			{
				m_confAppInfo->SetRecordingLinkInConf(1);
				m_confAppInfo->SetRecordingLinkRsrcId(PartyId);
				m_partiesList->SetIsRecordingLinkParty(PartyId, true);

				// Set "failed" and "initiator" flags to their default values
				m_confAppInfo->SetRecordingFailedFlag(NO);
				m_confAppInfo->SetRecordingInitiatorId((DWORD)(-1));
				m_confAppInfo->SetRlDisconnectionWasReceived(NO);
				TRACEINTO << msgOpcode.str() << " - incoming recording link connected";
			}


			// the party finished his IVR entrance session and ready to enter the conference
			if (m_partiesList->IsPartyAudioStateConnected(PartyId))
			{
				if (confWithLocalIVR)
					UpdateConfPartyDB(PartyId, FALSE); // update IVR status: Not in IVR

				TRACEINTO << msgOpcode.str().c_str() << " - End IVR Session, start CAM IVR";
				m_eventsList->StopPartyIVRNewFeature(0, eCAM_EVENT_PARTY_IVR_ENTRANCE, PartyId);    // stop this
				DoPartyEndIVRSession(opcode, PartyId, 0, pParam);
			}

			break;
		}

		case IVR_PARTY_START_MOVE_FROM_EQ:
		{
			int ind = m_partiesList->FindPartyIndex(partyName);
			TRACECOND_AND_RETURN(-1 == ind, msgOpcode.str() << " - Party doesn't exist in list");

			PartyId = m_partiesList->m_partyList[ind]->GetPartyRsrcId();

			CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
			PASSERTSTREAM_AND_RETURN(!pCommConf,
				msgOpcode.str() << " - Conference not found:"
					<< "\n ConfDB    = " << m_confAppInfo->GetCamString()
					<< "\n ConfName  = " << m_confAppInfo->GetConfName()
					<< "\n PartyId   = " << PartyId
					<< "\n PartyName = " << partyName);

			CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName);
			PASSERTSTREAM_AND_RETURN(!pConfParty,
				msgOpcode.str() << " - Party not found:"
					<< "\n ConfDB    = " << m_confAppInfo->GetCamString()
					<< "\n ConfName  = " << m_confAppInfo->GetConfName()
					<< "\n PartyId   = " << PartyId
					<< "\n PartyName = " << partyName);

			BYTE isVoiceParty = pConfParty->GetVoice();
			if (isVoiceParty)
				m_eventsList->StopPartyIVRNewFeature(0, eCAM_EVENT_PARTY_IVR_ENTRANCE, PartyId);
			else
			{
				if (m_partiesList->m_partyList[ind]->GetPartyVideoState() != eAPP_PARTY_STATE_IDLE)
					m_eventsList->StopPartyIVRNewFeature(0, eCAM_EVENT_PARTY_IVR_ENTRANCE, PartyId);  // stop slide to this party
			}

			m_partiesList->m_partyList[ind]->SetPartyVideoState(eAPP_PARTY_STATE_MOVING);
			break;
		}

		case eCAM_EVENT_PARTY_IS_TIP_SLAVE:
			break;

		case IVR_PARTY_START_MOVE_FROM_CONF:
		{
			int ind = m_partiesList->FindPartyIndex(partyName);
			TRACECOND_AND_RETURN(-1 == ind, msgOpcode.str() << " - Party doesn't exist in list: " << partyName);

			PartyId = m_partiesList->m_partyList[ind]->GetPartyRsrcId();

			if ((m_pcmAutorizationLevel == ePcmForChairpersonOnly && m_partiesList->GetIsPartyLeader(PartyId)) || m_pcmAutorizationLevel == ePcmForEveryone)
			{
				// send indication to PCM in order to clear the menu (deallocate rsrc in current conf) before move starts
				m_confAppInfo->m_pConfApi->ChairpersonStartMoveFromConf(PartyId);
			}

			CConfAppFeaturePartyIVR* pConfAppFeaturePartyIVR = m_eventsList->GetPartyIVR();
			if (pConfAppFeaturePartyIVR)
			{
				CFeatureObjectList* pPartyFeatureObjectList = pConfAppFeaturePartyIVR->GetFeaturesList();
				if (pPartyFeatureObjectList)
				{
					pPartyFeatureObjectList->ActionsUponPartyMoving(PartyId);
				}
			}

			if (m_partiesList->m_partyList[ind]->GetPartyVideoState() != eAPP_PARTY_STATE_IDLE)
				m_partiesList->m_partyList[ind]->SetPartyVideoState(eAPP_PARTY_STATE_MOVING);

			break;
		}

		case eCAM_EVENT_PARTY_DELETED:
		{
			*pParam >> partyNameWrapper;

			// In case of a Recording Link that failed to connect
			if (IsRecordingLinkParty(PartyId, partyName))
				OnRecordConfFailure();

			DoPartyOutOfConf(opcode, PartyId, 0, pParam, 1);
			break;
		}

		case PARTY_AUDIO_EXPORTED: // currently it is like "party Disconnected"
			DoPartyOutOfConf(eCAM_EVENT_PARTY_AUDIO_MOVE_OUT, PartyId, 0, pParam, 1);
			break;

		case PARTY_VIDEO_EXPORTED: // currently it is like "party Disconnected"
			DoPartyOutOfConf(eCAM_EVENT_PARTY_VIDEO_MOVE_OUT, PartyId, 0, pParam, 1);
			break;

		case e_ACK_IND:
		{
			OPCODE AckOpcode;
			*pParam >> AckOpcode;
			TRACEINTO << msgOpcode.str().c_str() << ", AckOpcode:" << AckOpcode;

			if (IVR_PLAY_MESSAGE_REQ == AckOpcode)
			{
				m_eventsList->HandlePlayMsgAckInd(pParam);

				int partyInd = m_partiesList->FindPartyIndex(PartyId);
				if (-1 != partyInd)
				{
					CConfAppPartyParams* pConfAppParty = m_partiesList->m_partyList[partyInd];
					CPartyApi* pPartyApi = pConfAppParty->GetPartyApi();
					pPartyApi->ReceivedAckOnIvrPlayMessageReq();
				}
			}

			if (IVR_SHOW_SLIDE_REQ == AckOpcode) //AT&T
			{
				int partyInd = m_partiesList->FindPartyIndex(PartyId);
				if (-1 != partyInd)
				{
					CConfAppPartyParams* pConfAppParty = m_partiesList->m_partyList[partyInd];
					CPartyApi* pPartyApi = pConfAppParty->GetPartyApi();
					pPartyApi->ReceivedAckOnIvrShowSlideReq();
				}
			}

			if (IVR_STOP_RECORD_ROLL_CALL_REQ == AckOpcode) //vngfe-8707
			{
				DWORD  ack_seq_num = 0;
				STATUS  status = 0;
				*pParam >> ack_seq_num >> status;
				TRACEINTO << " ROLL_CALL_STOP_RECORDING_ACK (01) PartyId = " << PartyId << " , status = " << status << " Ack received";
				OnPartyStopRollCallRecordingAck(PartyId,status);
			}

			break;
		}

		case REJECT_PLC:
			OnRejectPLC(PartyId, pParam);
			break;

		case AUD_DTMF_IND_VAL:
		case SIGNALLING_DTMF_INPUT_IND:
		case RTP_DTMF_INPUT_IND:
			m_eventsList->DtmfReceived(opcode, pParam);
			break;

		case IVR_RECORD_ROLL_CALL_IND:
			m_eventsList->RollCallRecorded(pParam);
			break;

		case SEQUENCE_NUM_IND:
			m_eventsList->ReplaceTokenWithSequenceNum(pParam);
			break;

		case eCAM_EVENT_PARTY_END_FEATURE:
		{
			DWORD uniqueEventNumber;
			*pParam >> PartyId >> uniqueEventNumber;                                  // get partyRsrcID
			DWORD originalEvent = m_waitList->GetEventType(uniqueEventNumber);
			if (originalEvent == eCAM_EVENT_MIN)
				break;                                                                      // not found

			if (eCAM_EVENT_PARTY_END_IVR == originalEvent)
				DoPartyEndIVRSession(opcode, PartyId, uniqueEventNumber, pParam);

			if (eCAM_EVENT_PARTY_DELETED == originalEvent)
				DoPartyOutOfConf(opcode, PartyId, uniqueEventNumber, pParam, 0);

			break;
		}

		case eCAM_EVENT_PARTY_END_LAST_FEATURE:
		{
			*pParam >> PartyId;                                                       // get partyRsrcID

			if (m_partiesList->IsPartyAudioStateConnected(PartyId))
			{
				int stat = WAIT_TO_EVENT;
				if (m_confAppInfo->GetIsGateWay())
					stat = m_eventsList->DoSomethingWithEndIVREvent(0, eCAM_EVENT_PARTY_PLAY_RINGING_TONE, PartyId);
				else
				{
					// Start Single Party Hears Music event if needed
					if ( IsNeedToDoSomethingWithEndIVROrPartyEvent(PartyId) == true ) //true = not TIP call or when we in aux slave (in TIP call)
						stat = m_eventsList->DoSomethingWithEndIVREvent(0, eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC, PartyId);
				}

				if (stat == WAIT_TO_EVENT)
					break;

				if ((!m_confAppInfo->GetIsGateWay() && !m_confAppInfo->GetSinglePartyNow())
					|| (m_confAppInfo->GetIsGateWay() && !m_confAppInfo->GetIsRingToneOn())
					|| m_confAppInfo->IsNeedToBlockIVR())
				{
					// If the party is not single in conference - it is entered to mix
					TRACEINTO << msgOpcode.str().c_str() << " - Enter party to Mix";
					m_eventsList->EnterPartyToMix(PartyId);
				}
			}

			break;
		}

		case eCAM_EVENT_TIMER_END_FEATURE:
		{
			DWORD confOrParty;
			DWORD uniqueEventNumber;
			DWORD originalOpcode;
			*pParam >> confOrParty >> PartyId >> uniqueEventNumber >> originalOpcode; // get partyRsrcID
			m_eventsList->StopFeature(uniqueEventNumber, (TConfAppEvents) originalOpcode, PartyId, confOrParty);
			break;
		}

		case PARTY_NOISE_DETECTION_IND:
			OnNoiseDetectionAlarm(pParam);
			break;

		case eCAM_EVENT_PARTY_AUDIO_DISCONNECTING:
		{
			// Remove events in Active Events List that related to the party + Stop party IVR
			CConfAppFeaturePartyIVR* pConfAppFeaturePartyIVR = m_eventsList->GetPartyIVR();
			if (pConfAppFeaturePartyIVR)
			{
				CFeatureObjectList* pPartyFeatureObjectList = pConfAppFeaturePartyIVR->GetFeaturesList();
				if (pPartyFeatureObjectList)
				{
					pPartyFeatureObjectList->ActionsUponPartyDisconnecting(PartyId, CAM_AUDIO);
				}
			}

			// Remove events in Wait Events List that related to the party (eCAM_EVENT_PARTY_END_IVR)
			m_waitList->DelEvent(PartyId);

			// Send STOP_IVR to CParty
			int partyInd = m_partiesList->FindPartyIndex(PartyId);
			TRACECOND_AND_RETURN(-1 == partyInd, msgOpcode.str() << " - Party doesn't exist in list");

			CConfAppPartyParams* pConfAppParty = m_partiesList->m_partyList[partyInd];

			DWORD restartIVR = 0;
			if (eAPP_PARTY_STATE_IVR_ENTRY == pConfAppParty->GetPartyAudioState())
				restartIVR = 1;
			// if audio disconnection is due to a 'hold call' event, we'll need to keep the ivr control to resume into IVR.
			if (m_partiesList->IsPartyOnHoldDuringSlide(PartyId))
				restartIVR = 0;

			CPartyApi* pPartyApi = pConfAppParty->GetPartyApi();
			pPartyApi->StopPartyIVR(restartIVR);

			// Update party audio state according to the current party state
			m_partiesList->UpdateStateUponDisconnecting(PartyId, CAM_AUDIO);

			break;
		}

		case eCAM_EVENT_LAST_PARTY_DISCONNECTING:
		{
			// Remove all conf's events + Stop conf IVR + Reset Active Events Counter
			CConfAppFeatureConfIVR* pConfAppFeatureConfIVR = m_eventsList->GetConfIVR();
			if (pConfAppFeatureConfIVR)
			{
				CFeatureObjectList* pConfFeatureObjectList = pConfAppFeatureConfIVR->GetFeaturesList();
				if (pConfFeatureObjectList)
					pConfFeatureObjectList->ActionsUponLastPartyDisconnecting();
			}
			break;
		}

		case eCAM_EVENT_PARTY_VIDEO_DISCONNECTING:
		{
			// Stop show slide
			CConfAppFeaturePartyIVR* pConfAppFeaturePartyIVR = m_eventsList->GetPartyIVR();
			if (pConfAppFeaturePartyIVR)
			{
				CFeatureObjectList* pPartyFeatureObjectList = pConfAppFeaturePartyIVR->GetFeaturesList();
				if (pPartyFeatureObjectList)
				{
					pPartyFeatureObjectList->ActionsUponPartyDisconnecting(PartyId, CAM_VIDEO);
				}
			}

			// Update party video state according to the current party state
			m_partiesList->UpdateStateUponDisconnecting(PartyId, CAM_VIDEO);
			break;
		}

		case eCAM_EVENT_PARTY_IS_CASCADE_LINK:
			m_partiesList->SetIsCascadeLinkParty(PartyId, true);
			break;

		case eCAM_EVENT_PARTY_UPDATE_GW_TYPE:
		{
			DWORD type = 0;
			*pParam >> type;
			m_partiesList->SetGatewayPartyType(PartyId, (eGatewayPartyType)type);
			if (type < eInviter)
				m_partiesList->SetSlideIsNoLongerPermitted(PartyId, true);
			break;
		}

		case eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE:
			m_eventsList->DoSomethingWithEndIVREvent(0, eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE, PartyId);
			break;

		case RECORD_CONF_FAILED:
			OnRecordConfFailure();
			break;

		case UPDATE_IVR_CNTR_IND:
		{
			WORD confState = IsValidPObjectPtr(m_confAppInfo)? m_confAppInfo->GetConfState() : (WORD)eAPP_CONF_STATE_TERMINATED;
			if ((confState == eAPP_CONF_STATE_TERMINATING) || (confState == eAPP_CONF_STATE_TERMINATED))
				TRACEINTO << " received event UPDATE_IVR_CNTR_IND(" << UPDATE_IVR_CNTR_IND << ") while in conf is in terminal state. skipping action.";
			else
				m_eventsList->HandleChangeIC();
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoPartyEndIVRSession(DWORD opcode, PartyRsrcID partyId, DWORD origUniqueEventNumber, CSegment* pParam)
{
	// if EQ conference only to stop the VideoSlide
	if (m_confAppInfo->GetIsEntryQueue())
		return;

	DWORD uniqueEventNumber = origUniqueEventNumber;
	if (eCAM_EVENT_PARTY_END_IVR == opcode) // the original event - starting the event in wait list
	{
		BYTE isLeader;
		*pParam >> isLeader; // 1=yes, 0=no
		if (isLeader)
		{ // this party entered as leader
			SetOldChairAsRegularPartyIfNeeded(partyId);
			m_partiesList->SetPartyAsLeader(partyId, isLeader);
			m_confAppInfo->SetLeaderInConfNow(1);
		}
		InformPcmPartyAdded(partyId);

		if (eAPP_CONF_STATE_TERMINATING == m_confAppInfo->GetConfState())
			return; // the conference started terminating

		uniqueEventNumber = m_waitList->AddEvent(eCAM_EVENT_PARTY_END_IVR, partyId, m_confAppInfo->GetMCUproductType());
		m_partiesList->ChangePartyState(partyId, opcode);
	}

	CPrettyTable<const char*, TConfAppEvents, const char*> tbl("Feature", "Opcode", "Description");
	while (TRUE)
	{
		TConfAppEvents featureOpcode = eCAM_EVENT_MIN;
		int status = m_waitList->GetNextFeature(uniqueEventNumber, featureOpcode);

		if (status == STATUS_FAIL)
		{
			tbl.Add(m_confAppInfo->GetStringFromOpcode(featureOpcode), featureOpcode, "Failed, so notify CAM");
			m_confAppInfo->m_pConfApi->NotifyCAM(partyId, eCAM_EVENT_PARTY_END_LAST_FEATURE, 0);
			break;
		}
		if (eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE == featureOpcode)
		{
			tbl.Add(m_confAppInfo->GetStringFromOpcode(featureOpcode), featureOpcode, "DUMMY_FEATURE, so notify CAM");
			m_confAppInfo->m_pConfApi->NotifyCAM(partyId, eCAM_EVENT_PARTY_END_LAST_FEATURE, 0);
			break;
		}
		tbl.Add(m_confAppInfo->GetStringFromOpcode(featureOpcode), featureOpcode, "");

		bool isNeed = false;
		if (featureOpcode == eCAM_EVENT_PARTY_FIRST_TO_JOIN)
		{
			isNeed = IsNeedToDoSomethingWithEndIVROrPartyEvent(partyId); //true = not TIP call or when we in aux slave (in TIP call)
		}
		else
		{
			isNeed = true;
		}

		if (isNeed == true)
			status = m_eventsList->DoSomethingWithEndIVREvent(uniqueEventNumber, featureOpcode, partyId);

		if (status == WAIT_TO_EVENT) // otherwise keep with "GetNextFeature"
			break; // end of while
	}
	TRACEINTO << "PartyId:" << partyId << tbl.Get();
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoPartyOutOfConf(DWORD opcode, PartyRsrcID partyId, DWORD origUniqueEventNumber, CSegment* pParam, WORD firstYesNo)
{
	// Check if the party exists in the parties lists
	if (-1 == m_partiesList->FindPartyIndex(partyId) && -1 == m_partiesList->FindPartyDelIndex(partyId))
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, party doesn't exist in list, maybe double deleted";
		return;
	}
	InformPcmPartyLeft(partyId);

	if (m_partiesList->GetIsPartyLeader(partyId))
	{
		// inform PCM on party left conference
		SetOldChairAsRegularPartyIfNeeded(partyId, FALSE);
		// roll back the changes in function
		m_partiesList->SetPartyAsLeader(partyId, TRUE);
		m_partiesList->SetChairPartyRsrcId(DUMMY_PARTY_ID);
	}

	// In case of an EQ
	if (m_confAppInfo->GetIsEntryQueue())
	{
		m_partiesList->ChangePartyState(partyId, opcode);
		return;
	}

	// In case of a regular conference (not an EQ)
	DWORD uniqueEventNumber = 0;

	if (firstYesNo) // the original event
	{
		if ((opcode == eCAM_EVENT_PARTY_DELETED || opcode == eCAM_EVENT_PARTY_AUDIO_MOVE_OUT) && // In case of party deleted
				(eAPP_CONF_STATE_TERMINATING != (m_confAppInfo->GetConfState()))) // In case the conference is not in terminating state
		{
			// Add new event to Wait list
			uniqueEventNumber = m_waitList->AddEvent((TConfAppEvents)opcode, partyId, m_confAppInfo->GetMCUproductType());
		}

		// Change party state and remove from Main Parties List in case of party deleted
		m_partiesList->ChangePartyState(partyId, opcode);
	}
	else // after feature ended
	{
		uniqueEventNumber = origUniqueEventNumber;
	}

	// update parameters on party lives conference - update "first to join"
	m_eventsList->DoSomethingUponPartyLeavesConf();

	// loop on all possible actions
	TConfAppEvents featureOpcode = eCAM_EVENT_MIN;
	CPrettyTable<const char*, TConfAppEvents, const char*> tbl("Feature", "Opcode", "Description");
	while (TRUE)
	{
		int status = m_waitList->GetNextFeature(uniqueEventNumber, featureOpcode);
		if (status == STATUS_FAIL)
		{
			tbl.Add(m_confAppInfo->GetStringFromOpcode(featureOpcode), featureOpcode, "Failed, so stop processing");
			break;
		}
		if (eCAM_EVENT_PARTY_LAST_DUMMY_FEATURE == featureOpcode)
		{
			tbl.Add(m_confAppInfo->GetStringFromOpcode(featureOpcode), featureOpcode, "DUMMY_FEATURE, so stop processing");

			// Remove party from Deleted Parties List and delete the related Roll Call recording file
			m_partiesList->OnPartyExitToneEnded(partyId);
			break;
		}

		tbl.Add(m_confAppInfo->GetStringFromOpcode(featureOpcode), featureOpcode, "");

		bool isNeed = false;
		if ( featureOpcode == eCAM_EVENT_PARTY_SINGLE_PARTY_MUSIC )
		{
			if ( IsNeedToDoSomethingWithEndIVROrPartyEvent(m_eventsList->FindSingleParty()) == true ) //true = not TIP call or when we in aux slave (in TIP call)
			{
				isNeed = true;
			}
		}
		else
		{
			isNeed = true;
		}

		if (isNeed == true)
			status = m_eventsList->DoSomethingWithEndPartyEvent(uniqueEventNumber, featureOpcode, partyId);

		if (status == WAIT_TO_EVENT) // otherwise keep with "GetNextFeature"
			break; // end of while
	}
	TRACEINTO << "PartyId:" << partyId << tbl.Get();
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnNoiseDetectionAlarm(CSegment* pParam)
{
	PartyRsrcID PartyId;
	CTaskApp* pParty = NULL;
	DWORD noiseDetectionAlarm; // 1 - noisy, 0 - not noisy
	DWORD noiseEnergyLevel;

	*pParam >> PartyId >> (void*&)pParty >> noiseDetectionAlarm >> noiseEnergyLevel;

	std::ostringstream msg;
	msg << "PartyId:" << PartyId << ", NoiseDetectionAlarm:" << noiseDetectionAlarm << ", NoiseEnergyLevel:" << noiseEnergyLevel;

	// find the party index
	int index = m_partiesList->FindPartyIndex(PartyId);
	if (-1 == index)
	{
		TRACEINTO << msg.str().c_str() << " - Failed, party doesn't exist in list";
		return;
	}

	string partyName = m_partiesList->m_partyList[index]->GetPartyName();

	if (noiseDetectionAlarm) // The party is noisy
	{
		// update the conference - change the party state to partially connected
		(m_confAppInfo->m_pConfApi)->UpdateDB((CTaskApp*)pParty, NOISE_DETECTION, PARTY_CONNECTED_WITH_PROBLEM);

		TNoiseEnergyLevel NoiseEnergyLevel = (TNoiseEnergyLevel)noiseEnergyLevel;

		switch (NoiseEnergyLevel)
		{
			case NOISE_ENERGY_LOW   : break;
			case NOISE_ENERGY_MIDDLE: break;
			case NOISE_ENERGY_HIGH  : break;
			default:
			{
				DBGPASSERT(1);
				break;
			}
		} // switch

		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
		if (!m_partiesList->m_partyList[index]->GetIsNoisyLine())
		{
			TRACEINTO << msg.str().c_str() << " - NOISE ON, Start Mute noisy line IVR";
			m_partiesList->m_partyList[index]->SetIsNoisyLine(true);
			CPartyApi* pPartyApi = m_partiesList->m_partyList[index]->GetPartyApi();
			pPartyApi->UpdateIVRGeneralOpcode(START_IVR_MUTE_NOISY_LINE);
		}
		else
		{
			TRACEINTO << msg.str().c_str() << " - NOISE ON, Party is already in noisy line IVR session";
		}
	}
	else // The party is not noisy anymore
	{
		// update the conference - change the party state to its previous state
		(m_confAppInfo->m_pConfApi)->UpdateDB((CTaskApp*)pParty, NOISE_DETECTION, PARTY_CONNECTED);
		TRACEINTO << msg.str().c_str() << " - NOISE OFF";
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnRejectPLC(PartyRsrcID partyId, CSegment* pParam)
{
	PartyRsrcID partyID;

	char partyName[H243_NAME_LEN];
	CArrayWrapper partyNameWrapper(partyName);

	WORD mediaDirection;
	DWORD opcode;

	*pParam >> partyID >> partyNameWrapper >> mediaDirection >> opcode;

	DWORD needToStopPlc = 0;
	if (opcode != VIDEO_GRAPHIC_OVERLAY_START_REQ)	// not reject on START_PLC
		needToStopPlc = 1;

	m_eventsList->DoPartyAction(eCAM_EVENT_PARTY_CANCEL_PLC, partyId, needToStopPlc);	// STOP_PLC
	m_eventsList->StopPartyIVRNewFeature(0, eCAM_EVENT_PARTY_CANCEL_PLC, partyId);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnActionActionEvents(CSegment* pParam)
{
	WORD confState = m_confAppInfo->GetConfState();
	TRACECOND_AND_RETURN(confState != eAPP_CONF_STATE_CONNECT, "action while conf terminating");

	// distinguish between party actions and conf actions
	DWORD opcode;
	*pParam >> opcode;

	TRACEINTO
		<< "Opcode:" << CAM_ActionOpcodeToString(opcode) << " (#" << opcode << ") " << m_confAppInfo->GetCamString()
		<< ", ConfName:" << m_confAppInfo->GetConfName();

	switch (opcode)
	{
	case EVENT_PARTY_REQUEST:
		DoPartyRequest(pParam);
		break;

	case EVENT_CONF_REQUEST:
		DoConfRequest(pParam);
		break;

	case EVENT_PLAY_MSG:
		DoPlayMessage(pParam);
		break;

	case EVENT_RECORD_ROLLCALL:
		DoRecordRollCall(pParam);
		break;

	case EVENT_STOP_MSG:
		DoStopMessage(pParam);
		/// TBD
		break;

	case EVENT_STOP_ROLLCALL_RECORDING:
		DoStopRollCallRecording(pParam);
		/// TBD
		break;

	case EVENT_STOP_ROLLCALL_RECORDING_ACK_TIMER:
		DoStopRollCallRecordingAckTimer(pParam);
		/// TBD
		break;

	case UPDATE_PARTY_NOISE_DETECTION:
		DoUpdatePartyNoiseDetection(pParam);
		break;

	default:
		break;
	};
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoConfRequest(CSegment* pParam)
{
	DWORD tOpcode;
	DWORD onOff;

	*pParam >> tOpcode;
	TConfAppEvents opcode = (TConfAppEvents)tOpcode;

	TRACEINTO
		<< "Opcode:" << m_confAppInfo->GetStringFromOpcode(opcode) << " (#" << opcode << ") " << m_confAppInfo->GetCamString()
		<< ", ConfName: " << m_confAppInfo->GetConfName();

	if (!m_eventsList->IsEventAllowed(opcode))
		return;

	int stat = m_eventsList->AskForObjection(opcode, 0, 0);
	if (APP_F_OBJECTION == stat)
	{	// there is an objection
		PTRACE(eLevelInfoNormal, "CConfAppMngr::DoConfRequest - Objection ");
		return;
	}

	switch (opcode)
	{
		case eCAM_EVENT_CONF_MUTE_ALL:
		{
			PartyRsrcID partyId;
			DWORD isIVR;
			DWORD isForMuteAllButLecture = FALSE;
			BYTE isMuteAllButLeader = FALSE;
			BYTE isMuteIncludeExistingUsers = FALSE;
			*pParam
				>> partyId
				>> isIVR
				>> isForMuteAllButLecture
				>> isMuteAllButLeader
				>> isMuteIncludeExistingUsers;

			TRACEINTO
				<< "PartyId:" << partyId
				<< ", IsIVR:" << (int)isIVR
				<< ", IsForMuteAllButLecture:" << (int)isForMuteAllButLecture
				<< ", IsMuteIncludeExistingUsers:" << (int)isMuteIncludeExistingUsers;

			if (isIVR)
			{
				m_confAppInfo->SetIsInMuteIncomingParties(YES);
				m_eventsList->StartConfIVRNewFeature(0, opcode, partyId);
			}
			else
			{
				*pParam << isIVR;
				*pParam << isForMuteAllButLecture;
				*pParam << isMuteAllButLeader;
				if (isMuteAllButLeader)
				{
					m_confAppInfo->SetIsMuteAllAudioButLeader(eOn);
					m_confAppInfo->m_pConfApi->UpdateDB((CTaskApp*)0xffff, UPDATE_MUTE_ALL_AUDIO_EXCEPT_LEADER, (DWORD)eOn);
					if (isMuteIncludeExistingUsers == FALSE)
					{
						break;
					}
				}
				m_eventsList->DoConfAction(opcode, partyId, pParam);
			}
			break;
		}

		case eCAM_EVENT_CONF_UNMUTE_ALL:
		{
			PartyRsrcID partyId;
			DWORD isIVR;
			DWORD isForMuteAllButLecture = FALSE;
			BYTE isMuteAllButLeader = FALSE;
			BYTE isMuteIncludeExistingUsers = FALSE;
			*pParam
				>> partyId
				>> isIVR
				>> isForMuteAllButLecture
				>> isMuteAllButLeader
				>> isMuteIncludeExistingUsers;

			TRACEINTO
				<< "PartyId:" << partyId
				<< ", IsIVR:" << (int)isIVR
				<< ", IsForMuteAllButLecture:" << (int)isForMuteAllButLecture
				<< ", isMuteAllButLeader:" << (int)isMuteAllButLeader
				<< ", isMuteIncludeExistingUsers:" << (int)isMuteIncludeExistingUsers;

			if (isIVR)
			{
				m_confAppInfo->SetIsInMuteIncomingParties(NO);
				m_eventsList->StartConfIVRNewFeature(0, opcode, partyId);
			}
			else
			{
				*pParam << isIVR;
				*pParam << isForMuteAllButLecture;
				*pParam << isMuteAllButLeader;
				if (isMuteAllButLeader)
				{
					m_confAppInfo->SetIsMuteAllAudioButLeader(eOff);
					m_confAppInfo->m_pConfApi->UpdateDB((CTaskApp*)0xffff, UPDATE_MUTE_ALL_AUDIO_EXCEPT_LEADER, (DWORD)eOff);
					if (isMuteIncludeExistingUsers == FALSE)
					{
						break;
					}
				}
				m_eventsList->DoConfAction(opcode, partyId, pParam);
			}
			break;
		}

		case eCAM_EVENT_CONF_ALERT_TONE:
		case eCAM_EVENT_CONF_ROLL_CALL_REVIEW:
		{
			m_eventsList->StartConfIVRNewFeature(0, opcode, 0);
			break;
		}

		case eCAM_EVENT_CONF_END_ROLL_CALL_REVIEW:
		{
			m_eventsList->StopConfIVRNewFeature(0, eCAM_EVENT_CONF_ROLL_CALL_REVIEW, 0);
			break;
		}

		case eCAM_EVENT_CONF_MUTE_INCOMING_PARTIES:
		{
			PartyRsrcID partyId;
			*pParam >> partyId;// the sender, it is not important
			*pParam >> onOff;  // 0:Off, 1:On
			if (onOff)
				m_confAppInfo->SetIsInMuteIncomingParties(YES);
			else
				m_confAppInfo->SetIsInMuteIncomingParties(NO);
			break;
		}

		case eCAM_EVENT_CONF_SECURE:
		{
			PartyRsrcID partyId;
			*pParam >> partyId;
			m_eventsList->StartConfIVRNewFeature(0, opcode, partyId);
			break;
		}

		case eCAM_EVENT_CONF_UNSECURE:
		{
			PartyRsrcID partyId;
			*pParam >> partyId;
			m_eventsList->StartConfIVRNewFeature(0, opcode, partyId);
			break;
		}

		case eIVR_SHOW_PARTICIPANTS:
		{
			PartyRsrcID partyId;
			*pParam >> partyId;
			m_eventsList->DoConfAction(opcode, partyId, 0);
			break;
		}

		case eIVR_SHOW_GATHERING:
		{
			PartyRsrcID partyId;
			*pParam >> partyId;
			m_eventsList->DoConfAction(opcode, partyId, 0);
			break;
		}

		case SET_START_RECORDING:
		case SET_STOP_RECORDING:
		case SET_PAUSE_RECORDING:
		case SET_RESUME_RECORDING:
		case eCAM_EVENT_CONF_STOP_RECORDING:
		case eCAM_EVENT_CONF_PAUSE_RECORDING:
		{
			// Set "initiator" flag to its default value (action is set from EMA - no initiator among the parties)
			if (SET_START_RECORDING == opcode)
				m_confAppInfo->SetRecordingInitiatorId((DWORD)(-1));

			if (YES == m_confAppInfo->GetEnableRecording())		// recording is enabled in conf
				DoRecordingAction(opcode);
			break;
		}

		case eCAM_EVENT_CONF_START_RESUME_RECORDING:
		{
			if (YES == m_confAppInfo->GetEnableRecording())		// recording is enabled in conf
			{
				if (eStopRecording == m_confAppInfo->GetRecordingStatus())		// only in case of start recording via DTMF
				{	// Save start recording initiator RsrcId
					PartyRsrcID partyId;
					*pParam >> partyId;
					m_confAppInfo->SetRecordingInitiatorId(partyId);
				}
				DoRecordingAction(opcode);
			}
			break;
		}

		case eCAM_EVENT_CONF_DTMF_FORWARDING:
		{
			PartyRsrcID partyId;
			*pParam >> partyId;
			m_eventsList->DoConfAction(opcode, partyId, pParam); // need to implement
			break;
		}

		case eCAM_EVENT_CONF_BLIP_ON_CASCADE_LINK:
		{
			PartyRsrcID partyId;
			*pParam >> partyId;
			m_eventsList->StartConfIVRNewFeature(0, opcode, partyId);
			break;
		}

		case eCAM_EVENT_CONF_DISCONNECT_INVITED_PARTICIPANT:
		{
			PartyRsrcID partyId;
			*pParam >> partyId;
			m_eventsList->DoConfAction(opcode, partyId, 0);
			break;
		}

		default:
		{
			PASSERT(1000 + opcode);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoPartyRequest(CSegment* pParam)
{
	DWORD tOpcode = 0;
	PartyRsrcID partyId = 0;
	DWORD onOff = 0;

	*pParam >> tOpcode;
	TConfAppEvents opcode = (TConfAppEvents)tOpcode;

	char partyName[256];
	memset(partyName, 0, sizeof(partyName));

	// bridge-1811
	if (eCAM_EVENT_SET_AS_LEADER_FROM_API == opcode || ((eCAM_EVENT_PARTY_CLIENT_SEND_DTMF == opcode) && ((CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator) || (CProcessBase::GetProcess()->GetProductType() == eProductTypeCallGeneratorSoftMCU))))
	{
		CArrayWrapper partyNameWrapper(partyName);

		*pParam >> partyNameWrapper;

		int index = m_partiesList->FindPartyIndex(partyName);
		if (-1 != index)
			partyId = m_partiesList->m_partyList[index]->GetPartyRsrcId();
	}
	else
		*pParam >> partyId;

	TRACEINTO
		<< "PartyId:" << partyId
		<< ", Opcode:" << m_confAppInfo->GetStringFromOpcode(opcode) << " (#" << opcode << ") " << m_confAppInfo->GetCamString()
		<< ", ConfName:" << m_confAppInfo->GetConfName();

	if (!m_eventsList->IsEventAllowed(opcode))
		return;

	switch (opcode)
	{
		case eCAM_EVENT_PARTY_PLAY_MENU:         // send the API to AB
		case eCAM_EVENT_PARTY_MUTE:              // self mute
		case eCAM_EVENT_PARTY_UNMUTE:            // self unmute
		case eCAM_EVENT_PARTY_OVERRIDE_MUTEALL:  // self override muteall
		{
			int stat = m_eventsList->AskForObjection(opcode, partyId, 1);
			if (APP_F_OBJECTION == stat)           // there is an objection
				return;

			m_eventsList->StartPartyIVRNewFeature(0, opcode, partyId);
			break;
		}

		case eCAM_EVENT_PARTY_CHANGE_PW:         // handle in IVR-Control
		case eCAM_EVENT_PARTY_SILENCE_IT:        // handle in IVR-Control
		case eCAM_EVENT_PARTY_PLC:               // Start / Stop PLC
		case eCAM_EVENT_PARTY_VENUS:             // Start / Stop PLC
		case eCAM_EVENT_PARTY_CHANGE_TO_LEADER:  // request from user via DTMF
		case eCAM_EVENT_INVITE_PARTY:
		case eCAM_EVENT_PARTY_PLAY_BUSY_MSG:
		case eCAM_EVENT_PARTY_PLAY_NOANSWER_MSG:
		case eCAM_EVENT_PARTY_PLAY_WRONG_NUMBER_MSG:
		case eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER:
		{
			*pParam >> onOff; // 0:Off, 1:On
			if (CAM_START_FEATURE == onOff) // on
			{
				int status = m_eventsList->AskForObjection(opcode, partyId, 1);
				if (APP_F_OBJECTION == status)	// there is an objection
					return;
				m_eventsList->StartPartyIVRNewFeature(0, opcode, partyId);
			}
			else // off
			{
				m_eventsList->StopPartyIVRNewFeature(0, opcode, partyId);									// stop feature
				if (opcode == eCAM_EVENT_PARTY_CHANGE_PW || opcode == eCAM_EVENT_PARTY_CHANGE_TO_LEADER || opcode == eCAM_EVENT_PARTY_SILENCE_IT || opcode == eCAM_EVENT_INVITE_PARTY || opcode == eCAM_EVENT_PARTY_PLAY_MESSAGE_FROM_TIP_MASTER)		// end of IVR feature
				{
					UpdateConfPartyDB(partyId, FALSE);	// update IVR status: Not in IVR
				}
			}
			break;
		}

		case eCAM_EVENT_PARTY_CHANGE_LAYOUT_PLC:       // Change Party Layout
		case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_PLC:  // Change from Conf to Private or oposite
		case eCAM_EVENT_PARTY_FORCE_PLC:               // Change from Conf to Private or oposite
		case eCAM_EVENT_PARTY_CHANGE_LAYOUT_VENUS:     // Change Party Layout
		case eCAM_EVENT_PARTY_CHANGE_LAYOUT_TYPE_VENUS:// Change from Conf to Private or oposite
		case eCAM_EVENT_PARTY_FORCE_VENUS:             // Change from Conf to Private or oposite
		{
			DWORD action;
			*pParam >> action;	// 0:Off, 1:On
			int status = m_eventsList->DoPartyAction(opcode, partyId, action, pParam);
			break;
		}

		case eCAM_EVENT_PARTY_INC_VOLUME:              // Change from Conf to Private or oposite
		case eCAM_EVENT_PARTY_DEC_VOLUME:              // Change from Conf to Private or oposite
		{
			DWORD param;
			*pParam >> param;	// Decode / Encoder
			int status = m_eventsList->DoPartyAction(opcode, partyId, param);
			break;
		}

		case eCAM_EVENT_PARTY_ROLL_CALL_YESNO:
		{
			DWORD action;
			*pParam >> action;	// 0:disable RollCall announcement, 1:enable RollCall announcement
			m_confAppInfo->SetRollCallAnnounceYesNo(action);
			break;
		}

			// bridge-1811
		case eCAM_EVENT_SET_AS_LEADER_FROM_API:
			DoSetPartyAsLeader(partyId, partyName, pParam);
			break;

		case eCAM_EVENT_ADD_FEATURE_TO_WAIT_LIST:
		{
			// Add party feature that was interrupted by another party feature to its event in the waiting list
			DWORD uniqueNumber;
			DWORD opcode;
			*pParam >> uniqueNumber >> opcode;
			m_waitList->AddFeature(uniqueNumber, opcode);
			break;
		}

		case eCAM_EVENT_PARTY_SEND_DTMF:
			DoSendDtmf(partyId, pParam);
			break;

		case eCAM_EVENT_PARTY_OPERATOR_ASSISTANCE:
		{
			DWORD action;
			*pParam >> action;	// 0:Off, 1:On

			TRACEINTO << "operator assist[op_assist_ivr], action:" << action;

			if (action)
			{
				int stat = m_eventsList->AskForObjection(opcode, partyId, 1);
				if (APP_F_OBJECTION == stat) 	// there is an objection
					return;

				m_eventsList->StartPartyIVRNewFeature(0, opcode, partyId);
				break;
			}
			else
				m_eventsList->StopPartyIVRNewFeature(0, opcode, partyId);

			//DoActionForParty( opcode, partyId, 1 );	// do mute / unmute and keep with message (new feature)
			break;
		}

		case eIVR_REQUEST_TO_SPEAK:
		{
			int stat = m_eventsList->AskForObjection(opcode, partyId, 1);
			if (APP_F_OBJECTION == stat) 	// there is an objection
				return;
			m_eventsList->StartPartyIVRNewFeature(0, opcode, partyId);
			break;
		}

		case eCAM_EVENT_PARTY_CLIENT_SEND_DTMF:
		{
			if ((CProcessBase::GetProcess()->GetProductFamily() != eProductFamilyCallGenerator) && (CProcessBase::GetProcess()->GetProductType() != eProductTypeCallGeneratorSoftMCU))
			{
				PTRACE(eLevelInfoNormal, "CConfAppMngr::DoPartyRequest - Client send DTMF - ERROR - system is not CG!!");
				return;
			}
			PTRACE(eLevelInfoNormal, "CConfAppMngr::DoPartyRequest - Client send DTMF ");
			DWORD direction = 0;
			*pParam >> direction;
			direction = 1; // temporary!!!!!
			if (direction == 1)
				m_eventsList->DoPartyAction(opcode, partyId, 0, pParam);	// send DTMF
			break;
		}

		case eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL: //AT&T
			m_eventsList->StartPartyIVRNewFeature(0, eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL, partyId);
			break;

		case eCAM_EVENT_PARTY_REMOVE_FEATURE: //AT&T
			m_eventsList->RemoveFeatureByOpcode(eCAM_EVENT_PARTY_PLAY_MESSAGE_EXTERNAL);
			break;

		default:
			PASSERTSTREAM_AND_RETURN(true, "Unexpected opcode:" << opcode);
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoCAMRequest(DWORD confOrParty, DWORD eventUniqueNumber, TConfAppEvents feature, PartyRsrcID partyId)
{
	if (!m_eventsList->IsOpcodeAllowed(feature))
		return;

	if (CONFREQ == confOrParty)
		m_eventsList->StartConfIVRNewFeature(eventUniqueNumber, feature, partyId);
	else
		m_eventsList->StartPartyIVRNewFeature(eventUniqueNumber, feature, partyId);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoPlayMessage(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CConfAppMngr::DoPlayMessage");
	m_eventsList->PlayMessage(pParam);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoRecordRollCall(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CConfAppMngr::DoRecordRollCall");
	m_eventsList->RecordRollCall(pParam);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoStopMessage(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CConfAppMngr::DoRecordRollCall");
	m_eventsList->StopMessage(pParam);
}
/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoStopRollCallRecording(CSegment* pParam)
{
	m_eventsList->StopRollCallRecording( pParam );
}
/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnPartyStopRollCallRecordingAck(DWORD PartyId,DWORD status)
{
	m_eventsList->StopRollCallRecordingAck(PartyId,status);
	ContinueDisconnectPartyAudio(PartyId);

}
/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoStopRollCallRecordingAckTimer(CSegment* pParam)
{
	TRACEINTO << " ROLL_CALL_STOP_RECORDING_ACK";
	DWORD partyId = 0;
	*pParam >> partyId;
	ContinueDisconnectPartyAudio(partyId);
}
/////////////////////////////////////////////////////////////////
void CConfAppMngr::ContinueDisconnectPartyAudio(DWORD PartyId)
{
	int  partyInd   = m_partiesList->FindPartyIndex(PartyId);
	BOOL isFound    = 1;;
	if (-1 == partyInd)
	{
		TRACEINTO << "PartyId:" << PartyId << " - Party doesn't exist in list";
	}
	else
	{
		TRACEINTO << " ROLL_CALL_STOP_RECORDING_ACK (03) partyId = " << PartyId << " , disconnect party audio";
		CConfAppPartyParams* pConfAppParty = m_partiesList->m_partyList[partyInd];
		pConfAppParty->SetInStopRecording(false);
		pConfAppParty->SetRollCallInRecording(false);

		EMediaDirection disconnectMediaDirection = eMediaInAndOut;
//		CBridgePartyDisconnectParams* pOriginalBridgePartyDisconnectParams = pConfAppParty->GetBridgePartyDisconnectParams();
//		if(NULL != pOriginalBridgePartyDisconnectParams){
//			disconnectMediaDirection = pOriginalBridgePartyDisconnectParams->GetMediaDirection();
//		}
		CBridgePartyDisconnectParams* pBridgePartyDisconnectParams = new CBridgePartyDisconnectParams(PartyId,disconnectMediaDirection);
		DisconnectPartyAudio(pBridgePartyDisconnectParams);
		POBJDELETE(pBridgePartyDisconnectParams);
	}
}
/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoUpdatePartyNoiseDetection(CSegment* pParam)
{
	PartyRsrcID partyId;
	WORD noiseDetection = 0;
	WORD noiseDetectionThreshold = 0;
	WORD updateAudioDsp = 0;

	*pParam >> partyId >> noiseDetection >> noiseDetectionThreshold >> updateAudioDsp;

	TRACEINTO
		<< "PartyId:" << partyId
		<< ", NoiseDetection:" << noiseDetection
		<< ", NoiseDetectionThreshold:" << noiseDetectionThreshold
		<< ", UpdateAudioDsp:" << updateAudioDsp;

	int index = m_partiesList->FindPartyIndex(partyId);
	if (-1 == index)
	{
		TRACEINTO << "PartyId:" << partyId << " - Failed, Party not found";
		return;
	}

	if (updateAudioDsp)
	{
		int isError = 0;

		BYTE noisyLineThresholdLevel = m_partiesList->m_partyList[index]->GetNoisyLineThresholdLevel();

		if (noiseDetection)
		{
			PTRACE(eLevelInfoNormal, "CConfAppMngr::DoUpdatePartyNoiseDetection - Update noise detection");
			if (noiseDetectionThreshold)
			{
				switch ((ENoiseDetectionThreshold)noisyLineThresholdLevel)
				{
					case E_NOISE_DETECTION_THRESHOLD_1: // Highest sensitivity
					case E_NOISE_DETECTION_THRESHOLD_3:
					case E_NOISE_DETECTION_THRESHOLD_5:
					case E_NOISE_DETECTION_THRESHOLD_7:
					{
						noisyLineThresholdLevel = noisyLineThresholdLevel + 2; 	// Increase the noise threshold level
						break;
					}
					case E_NOISE_DETECTION_THRESHOLD_9: // Lowest sensitivity
					{
						noisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_9;
						break;
					}
					default:
					{
						isError = 1;
						PTRACE(eLevelError, "CConfAppMngr::DoUpdatePartyNoiseDetection - Failed, Illegal noise threshold");
						break;
					}
				}
			}

			if (!isError)
				noiseDetection = eOn;

		}
		else
		{
			PTRACE(eLevelInfoNormal, "CConfAppMngr::DoUpdatePartyNoiseDetection - Disable noise detection");
			noisyLineThresholdLevel = E_NOISE_DETECTION_THRESHOLD_5;	// To be on the safe side
			noiseDetection = eOff;
		}

		// Update the party threshold level in the CAM parties list
		m_partiesList->m_partyList[index]->SetIsNoisyLine((noiseDetection == eOn));
		m_partiesList->m_partyList[index]->SetNoisyLineThresholdLevel(noisyLineThresholdLevel);

		// Send the update request to audio bridge
		if (GetConfAppInfo())
		{
			CAudioBridgeInterface* pAudBrdgInterface = GetConfAppInfo()->m_pAudBrdgInterface;
			if (pAudBrdgInterface)
			{
				pAudBrdgInterface->UpdateNoiseDetection(partyId, (EOnOff)noiseDetection, (BYTE)noisyLineThresholdLevel);
			}
		}
	}

	// Updating the party about ending the mute noisy line ivr session
	m_partiesList->m_partyList[index]->SetIsNoisyLine(false);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoSetPartyAsLeader(PartyRsrcID partyId, char* partyName, CSegment* pParam)
{
	DWORD isLeader;
	*pParam >> isLeader;

	int index = m_partiesList->FindPartyIndex(partyId);
	if (-1 != index)
	{
		TRACEINTO << "PartyId:" << partyId << ", IsLeader:" << isLeader;

		SetOldChairAsRegularPartyIfNeeded(partyId, isLeader);

		// Update DB
		CPartyApi* pPartyApi = m_partiesList->m_partyList[index]->GetPartyApi();
		pPartyApi->SendLeaderStatus((BYTE)isLeader);

		// Update CAM (party in parties list and info)
		m_partiesList->SetPartyAsLeader(partyId, isLeader);

		m_confAppInfo->SetLeaderInConfNow(isLeader);

		if (m_confAppInfo->IsInWaitForChairNow())
		{
			m_eventsList->DoSomethingWithSetAsChairEvent(partyId);
		}
		if (m_confAppInfo->GetIsMuteAllAudioButLeader())
		{
			if (isLeader)
			{
				m_confAppInfo->m_pAudBrdgInterface->UpdateMute(partyId, eMediaIn, eOff, OPERATOR);
			}
			else
			{
				m_confAppInfo->m_pAudBrdgInterface->UpdateMute(partyId, eMediaIn, eOn, OPERATOR);
			}
		}
	}
	else
	{
		TRACEINTO << "PartyId:" << partyId << "PartyName:" << partyName << ", IsLeader:" << isLeader;

		CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
		PASSERT_AND_RETURN(!pCommConf);

		CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName);
		PASSERT_AND_RETURN(!pConfParty);

		pConfParty->SetIsLeader((WORD)isLeader);
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoSendDtmf(PartyRsrcID partyId, CSegment* pParam)
{
	TRACEINTO << "PartyId:" << partyId;

	m_eventsList->DoPartyAction( eCAM_EVENT_PARTY_SEND_DTMF, partyId, CAM_START_FEATURE, pParam);	// send DTMF
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::ConnectPartyAudio(CBridgePartyInitParams* pBrdgPartyInitParams, bool isIVROn)
{
	PartyRsrcID partyId   = pBrdgPartyInitParams->GetPartyRsrcID();
	const char* partyName = pBrdgPartyInitParams->GetPartyName();

	TRACEINTO << "PartyId:" << partyId << ", PartyName:" << partyName;

	BOOL partyWithIVR = IsPartyWithIVR(partyName);

	// Set conf with IVR
	CConfAppBridgeParams confAppBridgeParams;
	if (partyWithIVR && 0 != m_confAppInfo->GetConfWithIVR()/*&& isIVROn*/)
		confAppBridgeParams.SetIvrInConf(TRUE);

	confAppBridgeParams.SetMuteIncoming((BOOL)m_confAppInfo->GetIsInMuteIncomingParties());   // Set mute incoming parties
	confAppBridgeParams.SetNoiseDetection(m_confAppInfo->GetIsEnableNoisyLineDetection());    // Set noisy line detection values
	confAppBridgeParams.SetNoiseDetectionThreshold(m_confAppInfo->GetDefualtNoisyLineThresholdLevel());
	confAppBridgeParams.SetMuteIncomingByOperator(IsPartyNeedToBeMuted(partyId));
	TRACEINTO << "IsPartyNeedToBeMuted:" << (WORD)IsPartyNeedToBeMuted(partyId) << ", IsInMuteIncomingParties:" << m_confAppInfo->GetIsInMuteIncomingParties();

	pBrdgPartyInitParams->SetConfAppParams(confAppBridgeParams);

	if (GetConfAppInfo())
	{
		CAudioBridgeInterface* pAudBrdgInterface = GetConfAppInfo()->m_pAudBrdgInterface;
		if (pAudBrdgInterface)
			pAudBrdgInterface->ConnectParty(pBrdgPartyInitParams);
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::ConnectPartyVideo(CBridgePartyInitParams* pBrdgPartyInitParams)
{
	PartyRsrcID partyId   = pBrdgPartyInitParams->GetPartyRsrcID();
	const char* partyName = pBrdgPartyInitParams->GetPartyName();
	bool bIsVideoRelay = pBrdgPartyInitParams->GetIsVideoRelay();
	BOOL partyWithIVR = IsPartyWithIVR(partyName);

	TRACEINTO
		<< "PartyId:" << partyId
		<< ", PartyName:" << partyName
		<< ", IsVideoRelay:" << (DWORD)bIsVideoRelay
		<< ", PartyWithIVR:" << (WORD)partyWithIVR;

	CConfAppBridgeParams confAppBridgeParams;

	if (partyWithIVR && ((m_partiesList->GetSlideIsOn(partyId) || IsSlideNeeded(partyName)) || m_partiesList->IsTipParty(partyName)) )
  {
		confAppBridgeParams.SetIvrInConf(TRUE);
	}
	//BRIDGE-8051 hold/resume in IVR/witing for chair
	WORD isResumingFromHoldDuringSlide = m_partiesList->IsPartyOnHoldDuringSlide(partyId);
	if (isResumingFromHoldDuringSlide)
	{
		// resetting the value once it's updated in confAppBridgeParams
		m_partiesList->SetPartyOnHoldDuringSlide(partyId, (WORD)FALSE);
		confAppBridgeParams.SetIvrInConf(TRUE);
	}

	pBrdgPartyInitParams->SetConfAppParams(confAppBridgeParams);

	if (GetConfAppInfo())
	{
		CVideoBridgeInterface* pVideoBrdgInterface = GetConfAppInfo()->m_pVideoBridgeInterface;

		if (pVideoBrdgInterface)
			pVideoBrdgInterface->ConnectParty(pBrdgPartyInitParams);
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DisconnectPartyAudio(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams)
{
	PartyRsrcID partyRsrcID = pBridgePartyDisconnectParams->GetPartyId();
	EMediaDirection eMediaDirection = pBridgePartyDisconnectParams->GetMediaDirection();

	TRACEINTO << "PartyId:" << partyRsrcID << ", MediaDirection:" << eMediaDirection;

	// vngfe-8707
	int  partyIndex   = m_partiesList->FindPartyIndex(partyRsrcID);
	if (-1 == partyIndex)
	{
		TRACEINTO << "PartyId:" << partyRsrcID << " - Party doesn't exist in list";
	}
	else
	{
		CConfAppPartyParams* pConfAppParty = m_partiesList->m_partyList[partyIndex];
		bool isInRecording = pConfAppParty->IsRollCallInRecording();
		if(isInRecording){
			if(false == pConfAppParty->IsInStopRecording()){
				TRACEINTO << " ROLL_CALL_STOP_RECORDING (01), PartyId: " << partyRsrcID  <<  " in roll call recording - stop recording before disconnect audio";
				pConfAppParty->StopRollCallRecording(pBridgePartyDisconnectParams);
				pConfAppParty->SetInStopRecording(true);
			}else{
				 TRACEINTO << " ROLL_CALL_STOP_RECORDING, PartyId: " << partyRsrcID  <<  " in roll call recording - already in stop recording";
			}
			 return;
		}

	}

	// Unexpected Recording Link disconnection
	if (IsRecordingLinkParty(partyRsrcID))                              // a Recording Link party
	{
		if (eStopRecording != m_confAppInfo->GetRecordingStatus())    // recording status is NOT "Stopped"
		{
			if (!m_confAppInfo->GetRlDisconnectionWasReceived())
			{
				// Update RlDisconnectionWasReceived flag (so the next channel disconnection won't invoke the following code)
				m_confAppInfo->SetRlDisconnectionWasReceived(YES);

				// Stop RL timers if necessary
				DeleteTimer(TIMER_START_RECORDING);
				DeleteTimer(TIMER_DISCONNECT_RL);
				DeleteTimer(TIMER_SEND_START_DTMF);
				DeleteTimer(TIMER_RECORDING_CONTROL);

				// Update DB, CDR and CAM Info when RL is disconnecting
				WORD recFailedFlag = NO;
				WORD recUnexpectedDiscon = NO;
				if (m_confAppInfo->GetConfState() != eAPP_CONF_STATE_TERMINATING)
				{
					recFailedFlag = YES;
					recUnexpectedDiscon = YES;
				}

				UpdatesUponRlDisconnection(partyRsrcID, recFailedFlag, recUnexpectedDiscon);
				m_confAppInfo->SetRecordingLinkInConf(0);

				// Play public message to all of the participants in the conf
				if (YES == recUnexpectedDiscon)
	        		DoCAMRequest( CONFREQ, 0, eCAM_EVENT_CONF_RECORDING_FAILED, 0);
			}
		}
	}

	// Party disconnecting procedure (Stop IVR, Remove all party events, Set party state)
	CSegment* partySeg = new CSegment;
	*partySeg << (DWORD)eCAM_EVENT_PARTY_AUDIO_DISCONNECTING
	          << partyRsrcID;
	HandleEvent(partySeg, 0, EVENT_NOTIFY);
	POBJDELETE(partySeg);

	// added partyState judgement for VNGFE-5702
	int  partyInd   = m_partiesList->FindPartyIndex(partyRsrcID);
	int  partyState = eAPP_PARTY_STATE_MIN;
	if (-1 != partyInd)
	{
		CConfAppPartyParams* pConfAppParty = m_partiesList->m_partyList[partyInd];
		partyState = pConfAppParty->GetPartyAudioState();
	}

	TRACEINTO << "PartyId:" << partyRsrcID << ", PartyAudioState:" << partyState << ", is party found in list: " << (-1 != partyInd);

	CSegment confSeg;
	if (m_partiesList->GetNumOfInMixParticipants() == 1)
	{
		WORD partyInMix = m_partiesList->GetPartyInMix();
		TRACEINTO << "partyInMix:" << partyInMix;
		if (-1 == partyInd) // Last party disconnecting procedure (Stop conf IVR, Remove all conf events, Reset counter)
		{
			if (partyInMix == partyRsrcID)
			{
				// Last party disconnecting procedure (Stop conf IVR, Remove all conf events, Reset counter)
				confSeg << (DWORD)eCAM_EVENT_LAST_PARTY_DISCONNECTING;
				HandleEvent(&confSeg, 0, EVENT_NOTIFY);
			}
		}
		else if ((partyState != eAPP_PARTY_STATE_IDLE) && !(m_partiesList->IsTipParty(partyRsrcID) || m_partiesList->IsTipMaster(partyRsrcID) == true)) //in TIP call we need to do this only for the master and not for the slaves
		{
			// Last party disconnecting procedure (Stop conf IVR, Remove all conf events, Reset counter)
			confSeg << (DWORD)eCAM_EVENT_LAST_PARTY_DISCONNECTING;
			HandleEvent(&confSeg, 0, EVENT_NOTIFY);
		}
	}

	if (GetConfAppInfo())
	{
		CAudioBridgeInterface* pAudBrdgInterface = GetConfAppInfo()->m_pAudBrdgInterface;
		if (pAudBrdgInterface)
		{
			pAudBrdgInterface->DisconnectParty(pBridgePartyDisconnectParams);
		}
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DisconnectPartyVideo(CBridgePartyDisconnectParams* pBridgePartyDisconnectParams)
{
	PartyRsrcID PartyId = pBridgePartyDisconnectParams->GetPartyId();
	EMediaDirection eMediaDirection = pBridgePartyDisconnectParams->GetMediaDirection();

	TRACEINTO << "PartyId:" << PartyId << ", MediaDirection:" << eMediaDirection;

	// Check if the video OUT channel is disconnecting
	if (eMediaOut == eMediaDirection || eMediaInAndOut == eMediaDirection)
	{
		// Party disconnecting procedure (Stop show slide, Set party state)
		std::auto_ptr<CSegment> seg(new CSegment);
		*seg << (DWORD)eCAM_EVENT_PARTY_VIDEO_DISCONNECTING << PartyId;
		HandleEvent(seg.get(), 0, EVENT_NOTIFY);
	}

	// Release preview resources if exist
	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(PartyId);
	if (pParty)
	{
		DWORD monitor_conf_id  = pParty->GetMonitorConfId();
		DWORD monitor_party_id = pParty->GetMonitorPartyId();

		WORD  direction;
		switch (eMediaDirection)
		{
			case eMediaIn : direction = 0; break;
			case eMediaOut: direction = 1; break;
			default       : direction = 2; break;
		}
		CSegment* pSeg = new CSegment;
		*pSeg << monitor_conf_id << monitor_party_id << direction;
		CManagerApi api(eProcessResource);
		api.SendMsg(pSeg, STOP_VIDEO_PREVIEW_REQ);
	}

	if (GetConfAppInfo())
	{
		CVideoBridgeInterface* pVideoBrdgInterface = GetConfAppInfo()->m_pVideoBridgeInterface;
		if (pVideoBrdgInterface)
			pVideoBrdgInterface->DisconnectParty(pBridgePartyDisconnectParams);
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::ExportPartyAudio(CBridgePartyExportParams* pBridgePartyExportParams)
{
	if (GetConfAppInfo())
	{
		CAudioBridgeInterface* pAudBrdgInterface = GetConfAppInfo()->m_pAudBrdgInterface;
		if (pAudBrdgInterface)
			pAudBrdgInterface->ExportParty(pBridgePartyExportParams);
	}
}


/////////////////////////////////////////////////////////////////
void CConfAppMngr::ExportPartyVideo(CBridgePartyExportParams* pBridgePartyExportParams)
{
	if (GetConfAppInfo())
	{
		CVideoBridgeInterface* pVideoBrdgInterface = GetConfAppInfo()->m_pVideoBridgeInterface;
		if (pVideoBrdgInterface)
			pVideoBrdgInterface->ExportParty(pBridgePartyExportParams);
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::RemovePartyFromCAM(PartyRsrcID partyId, const char* partyName)
{
	TRACEINTO << "PartyId:" << partyId << ", PartyName:" << partyName;

	CSegment seg;
	seg
		<< (DWORD)eCAM_EVENT_PARTY_DELETED
		<< partyId
		<< partyName;

	HandleEvent(&seg, 0, EVENT_NOTIFY);
}

/////////////////////////////////////////////////////////////////
DWORD CConfAppMngr::IsSlideNeeded(PartyRsrcID partyId)
{
	int partyIndex = m_partiesList->FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == partyIndex, "PartyId:" << partyId << " - Failed, Party not found", false);

	DWORD isSlideNeeded = IsSlideNeeded(m_partiesList->m_partyList[partyIndex]->GetPartyName().c_str());

	TRACEINTO << "PartyId:" << partyId << ", IsSlideNeeded:" << isSlideNeeded;

	return isSlideNeeded;
}

/////////////////////////////////////////////////////////////////
DWORD CConfAppMngr::IsSlideNeeded(const char* partyName)
{
	#undef LOGINFO
	#define LOGINFO "PartyName:" << DUMPSTR(partyName)

	if (m_confAppInfo->GetIsExternalIVRInConf() == true)
		return 1;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	TRACECOND_AND_RETURN_VALUE(!pCommConf, LOGINFO << " - Conf not found", 0);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName);
	TRACECOND_AND_RETURN_VALUE(!pConfParty, LOGINFO << " - Party not found", 0);

	if (pCommConf->GetIsGateway())
	{
		if (pConfParty->GetGatewayPartyType() < eInviter)
		{
			TRACEINTO << LOGINFO << " - No slide needed for GW party";
			return 0;
		}
		TRACEINTO << LOGINFO << ", IsSlideNeeded:1";
		return 1;
	}

	if ((!pCommConf->GetEntryQ()) && (pConfParty->GetCascadeMode() != CASCADE_MODE_NONE) && (0 == pConfParty->GetIsCallFromGW()) ) // cascade link Master or Slave
	{
		if(pConfParty->GetNetInterfaceType() == ISDN_INTERFACE_TYPE && pConfParty->IsSrcTypeIsEQ() == TRUE) //BRIDGE-14279
		{
			TRACEINTO << "IsIsdnGWCallCamesFromEQ";
		}
		else
		{
			TRACEINTO << LOGINFO << " - No slide needed for cascade party";
			return 0;
		}
	}

	if (!IsIvrForSVCEnabled() && pConfParty->GetPartyMediaType() == eSvcPartyType)
	{
		TRACEINTO << LOGINFO << ", IsIvrForSVCEnabled:0, PartyMediaType:eSvcPartyType - No slide needed";
		return 0;
	}

	const char* ivrServiceName = m_confAppInfo->GetIvrServiceName();
	TRACECOND_AND_RETURN_VALUE(!ivrServiceName, LOGINFO << " - Invalid service name", 0);

	CIVRService* pIVRService = ::GetpAVmsgServList()->GetIVRServiceByName(ivrServiceName);
	TRACECOND_AND_RETURN_VALUE(!pIVRService, LOGINFO << ", ServiceName:" << ivrServiceName << " - Service not found", 0);

	WORD isConfPwSet = 0;
	WORD isChairPwSet = 0;

	if (pCommConf)
	{
		isConfPwSet = strcmp(pCommConf->GetEntryPassword(), "");
		isChairPwSet = strcmp(pCommConf->GetH243Password(), "");
	}

	DWORD isIVREntranceSessionSet = 0;

	if (pIVRService->GetEntryQueueService())
	{
		isIVREntranceSessionSet = 1;
	}
	else if (pIVRService->GetWelcomeFeature()->GetEnableDisable() || pIVRService->GetRollCallFeature()->GetEnableDisable())
	{
		isIVREntranceSessionSet = 1;
	}
	else if ((pIVRService->GetConfPasswordFeature()->GetEnableDisable() && isConfPwSet) ||
		(pIVRService->GetConfLeaderFeature()->GetEnableDisable() && isChairPwSet))
	{
		isIVREntranceSessionSet = 1;
	}

	// Check if the party has finished the entrance session
	WORD finishedEntrance = 0;
	int partyIndex = m_partiesList->FindPartyIndex(partyName);

	if (-1 != partyIndex)
	{
		if ((eAPP_PARTY_STATE_IVR_ENTRY != m_partiesList->m_partyList[partyIndex]->GetPartyAudioState()) &&
				(eAPP_PARTY_STATE_IDLE != m_partiesList->m_partyList[partyIndex]->GetPartyAudioState()))
			finishedEntrance = 1;
	}

	if (((m_confAppInfo->IsWaitForChair()) && !(m_confAppInfo->GetIsLeaderInConfNow())) ||
			(isIVREntranceSessionSet && !finishedEntrance))
	{
		TRACEINTO << LOGINFO << ", IsSlideNeeded:1";
		return 1;
	}

	TRACEINTO << LOGINFO << ", IsSlideNeeded:0";
	return 0;
}

/////////////////////////////////////////////////////////////////
#define CASE_RETURN_STRING_FOR_ENUM(e) case (e): return #e " ";

/////////////////////////////////////////////////////////////////
const char* CConfAppMngr::CAM_NotifyOpcodeToString(DWORD opcode)
{
	switch (opcode)
	{
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_CONF_TERMINATING);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_CONF_TERMINATED);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_AUDIO_CONNECTED);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_VIDEO_CONNECTED);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_VIDEO_IVR_MODE_CONNECTED);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_ON_HOLD_IND);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_IVR_MODE_ON_RESUME);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_CONNECT_AUDIO_ON_RESUME);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_PARTY_END_IVR);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_AUDIO_DISCONNECTED);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_VIDEO_DISCONNECTED);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_PARTY_DELETED);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_AUDIO_EXPORTED);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_VIDEO_EXPORTED);
		CASE_RETURN_STRING_FOR_ENUM(e_ACK_IND);
		CASE_RETURN_STRING_FOR_ENUM(AUD_DTMF_IND_VAL);
		CASE_RETURN_STRING_FOR_ENUM(SIGNALLING_DTMF_INPUT_IND);
		CASE_RETURN_STRING_FOR_ENUM(RTP_DTMF_INPUT_IND);
		CASE_RETURN_STRING_FOR_ENUM(IVR_RECORD_ROLL_CALL_IND);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_PARTY_END_FEATURE);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_PARTY_END_LAST_FEATURE);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_TIMER_END_FEATURE);
		CASE_RETURN_STRING_FOR_ENUM(PARTY_NOISE_DETECTION_IND);
		CASE_RETURN_STRING_FOR_ENUM(REJECT_PLC);
		CASE_RETURN_STRING_FOR_ENUM(IVR_PARTY_START_MOVE_FROM_EQ);
		CASE_RETURN_STRING_FOR_ENUM(IVR_PARTY_START_MOVE_FROM_CONF);
		CASE_RETURN_STRING_FOR_ENUM(TERMINATECONF);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_PARTY_AUDIO_DISCONNECTING);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_LAST_PARTY_DISCONNECTING);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_PARTY_VIDEO_DISCONNECTING);
		CASE_RETURN_STRING_FOR_ENUM(FECC_PARTY_BRIDGE_CONNECTED);
		CASE_RETURN_STRING_FOR_ENUM(FECC_PARTY_BRIDGE_DISCONNECTED);
		CASE_RETURN_STRING_FOR_ENUM(SET_PARTY_AS_LEADER);
		CASE_RETURN_STRING_FOR_ENUM(SEQUENCE_NUM_IND);
		CASE_RETURN_STRING_FOR_ENUM(IVR_PARTY_READY_FOR_SLIDE);
		CASE_RETURN_STRING_FOR_ENUM(UPDATE_IVR_CNTR_IND);
		CASE_RETURN_STRING_FOR_ENUM(RECORD_CONF_FAILED);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_PARTY_STOP_PLAY_RINGING_TONE);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_PARTY_UPDATE_GW_TYPE);
		CASE_RETURN_STRING_FOR_ENUM(eCAM_EVENT_SET_AS_LEADER_FROM_API);
	}

	return "Notify Opcode not in print list! ";
}

/////////////////////////////////////////////////////////////////
const char* CConfAppMngr::CAM_ActionOpcodeToString(DWORD opcode)
{
	switch (opcode)
	{
		CASE_RETURN_STRING_FOR_ENUM(EVENT_PARTY_REQUEST);
		CASE_RETURN_STRING_FOR_ENUM(EVENT_CONF_REQUEST);
		CASE_RETURN_STRING_FOR_ENUM(EVENT_PLAY_MSG);
		CASE_RETURN_STRING_FOR_ENUM(EVENT_RECORD_ROLLCALL);
		CASE_RETURN_STRING_FOR_ENUM(EVENT_STOP_MSG);
		CASE_RETURN_STRING_FOR_ENUM(UPDATE_PARTY_NOISE_DETECTION);
		CASE_RETURN_STRING_FOR_ENUM(EVENT_STOP_ROLLCALL_RECORDING);
		CASE_RETURN_STRING_FOR_ENUM(EVENT_STOP_ROLLCALL_RECORDING_ACK_TIMER);
	}

	return "Action Opcode not in print list! ";
}

#undef CASE_RETURN_STRING_FOR_ENUM

/////////////////////////////////////////////////////////////////
void CConfAppMngr::UpdateConfPartyDB(PartyRsrcID partyId, BOOL isIvr)
{
	const WORD status = isIvr ? STATUS_PARTY_IVR : STATUS_PARTY_INCONF;

	m_confAppInfo->m_pConfApi->UpdatePartyIvrStatus(partyId, status);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnNotifyPartyReadyForSlide(PartyRsrcID partyId)
{
	// in cascade EQ slide is not shown
	TRACECOND_AND_RETURN(m_confAppInfo->GetIsEntryQueue() && m_confAppInfo->IsCascadeEQ(), "PartyId:" << partyId << " - Cascade EQ, doesn't show slide");

	const int ind = m_partiesList->FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == ind, "PartyId:" << partyId << " - Failed, Party not found");

	bool isVideoBridgeReadyForSlide = m_partiesList->IsVideoBridgeReadyForSlide(ind);
	TRACEINTO << "PartyId:" << partyId << ", IsVideoBridgeReadyForSlide:" << isVideoBridgeReadyForSlide << ", IsExternalIVRInConf:" << (int)m_confAppInfo->GetIsExternalIVRInConf();

	if (!isVideoBridgeReadyForSlide)
		m_partiesList->SetPartyReadyForSlide(ind, true);

	else if (!m_confAppInfo->GetIsExternalIVRInConf())
	{
		const bool isSlideNoLongerPermitted = m_partiesList->m_partyList[ind]->GetSlideIsNoLongerPermitted();
		const TAppPartyState partyVideoState = m_partiesList->GetPartyVideoState(partyId);

		if (!isSlideNoLongerPermitted
			&& eAPP_PARTY_STATE_DISCONNECTED != partyVideoState // party in DELETED state is in the partyDeletedList
			&& eAPP_PARTY_STATE_MOVING != partyVideoState)
		{
			m_eventsList->DoPartyAction(eCAM_EVENT_PARTY_SHOW_SLIDE, partyId, CAM_START_FEATURE); // show slide
			TRACEINTO << "PartyId:" << partyId << " - Send eCAM_EVENT_PARTY_SHOW_SLIDE to party"; // vngr-21248
		}
		else
		{
			TRACEINTO << "PartyId:" << partyId << " - Do not send eCAM_EVENT_PARTY_SHOW_SLIDE to party"; // vngr-21248
		}
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::CAMNullActionFunction(CSegment* pParam)
{
	TRACEINTO << "State:" << GetStateAsString();
}

/////////////////////////////////////////////////////////////////
BOOL CConfAppMngr::ShouldStartRecordingImm(PartyRsrcID partyId)
{
	if ((((m_confAppInfo->IsWaitForChair()) && (m_partiesList->GetIsPartyLeader(partyId)))	// chairperson in a "Wait for Chair" conference
		|| (m_partiesList->GetNumOfParticipants() >= 1))	// OR any first party in a NONE-"Wait for Chair" conference
		&&
		((YES == m_confAppInfo->GetEnableRecording()) &&	// recording is enabled in conf
		(START_RECORDING_IMMEDIATELY == m_confAppInfo->GetStartRecordingPolicy()) &&	// recording policy is start imm.
		(eStopRecording == m_confAppInfo->GetRecordingStatus())))			// recording status is "Stop"
		return TRUE;
	else
		return FALSE;
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnTimerStartRecording(CSegment* pParam)
{
	// Update DB, CDR and CAM Info when RL is disconnecting
	PartyRsrcID recordingLinkId = (DWORD)(-1);
	if (1 == m_confAppInfo->GetRecordingLinkInConf())
		recordingLinkId = m_confAppInfo->GetRecordingLinkRsrcId();

	TRACEINTO << "RecordingLinkId:" << recordingLinkId;

	WORD recFailedFlag = YES;	// if timer is up, Recording Link failed to be connected
	UpdatesUponRlDisconnection(recordingLinkId, recFailedFlag);

	// Play message to all the chairpersons that are currently in conf + initiator (for cases when the initiator is a regular party)
	int numOfParties = m_partiesList->GetNumOfParticipants();
	for (int i = 0; i < numOfParties; i++)
	{
		if ((m_partiesList->m_partyList[i]->GetIsPartyLeader()) ||		// check if chairperson
		    (m_partiesList->m_partyList[i]->GetIsCascadeLinkParty()) ||	// check if cascaded link party
		    (m_confAppInfo->GetRecordingInitiatorId() == m_partiesList->m_partyList[i]->GetPartyRsrcId()))	// check if initiator
		{
			PartyRsrcID partyId = m_partiesList->m_partyList[i]->GetPartyRsrcId();
			int stat = m_eventsList->AskForObjection(eCAM_EVENT_PARTY_RECORDING_FAILED, partyId, 1);
			if (APP_F_OBJECTION == stat) 	// there is an objection
				return;
			TRACEINTO << "PartyId:" << partyId << " - Play failed message";
			DoCAMRequest(PARTYREQ, 0, eCAM_EVENT_PARTY_RECORDING_FAILED, partyId);
		}
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnTimerDisconnectRL(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CConfAppMngr::OnTimerDisconnectRL");

	// Disconnect Recording Link
	m_confAppInfo->m_pConfApi->DisconnectRecordingLink();
	// Set Recording Link flag in CAM info to 0
	m_confAppInfo->SetRecordingLinkInConf(0);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnTimerSendStartDtmf(CSegment* pParam)
{
	PTRACE( eLevelInfoNormal, "CConfAppMngr::OnTimerSendStartDtmf");

	WORD lastReqRecAction = m_confAppInfo->GetLastReqRecAction();

	if ((DTMF_START_RESUME_RECORDING == lastReqRecAction) ||
		(SET_START_RECORDING == lastReqRecAction) ||
		(SET_RESUME_RECORDING == lastReqRecAction))
	{
		// Send DTMF to RSS in order to start recording
		if (1 == m_confAppInfo->GetRecordingLinkInConf())
		{
			DWORD recordingLinkRsrcId = m_confAppInfo->GetRecordingLinkRsrcId();

			//Dial In party needs the feedback
			if(false == IsPartyDialin( recordingLinkRsrcId))
			{
				//only for Dialout. the Dialin will set this flag when receives the feedback!
				m_confAppInfo->SetStartRecDtmfWasSent(1);
			}
			SendRecordingControlToRL(recordingLinkRsrcId, eStartRecording);
		}
	}

	if ((DTMF_PAUSE_RECORDING == lastReqRecAction) ||
		(SET_PAUSE_RECORDING == lastReqRecAction))
	{
		PTRACE( eLevelInfoNormal, "CConfAppMngr::OnTimerSendStartDtmf - Call PauseRecordingDuringStarted");
		PauseRecordingDuringStarted();
	}

	if ((eCAM_EVENT_CONF_STOP_RECORDING == lastReqRecAction) ||
		(SET_STOP_RECORDING == lastReqRecAction))
	{
		PTRACE( eLevelInfoNormal, "CConfAppMngr::OnTimerSendStartDtmf - Call StopRecordingDuringStarted");
		StopRecordingDuringStarted();
	}
}

/////////////////////////////////////////////////////////////////
bool CConfAppMngr::IsRecordingLinkParty(PartyRsrcID partyId, char* partyName)
{
	std::ostringstream msg;

	string strPartyName = "";
	if (NULL == partyName)
	{
		int ind = m_partiesList->FindPartyIndex(partyId);
		TRACECOND_AND_RETURN_VALUE(-1 == ind, "PartyId:" << partyId << " - Failed, Party not found", false);

		strPartyName = m_partiesList->m_partyList[ind]->GetPartyName();
		msg << "PartyId:" << partyId << ", PartyName:" << strPartyName.c_str();
	}
	else  // In case this function is called from eCAM_EVENT_PARTY_DELETED
	{
		strPartyName = partyName;
		msg << "PartyName:" << strPartyName.c_str();
	}

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERTMSG_AND_RETURN_VALUE(!pCommConf, "Failed, 'pCommConf' is NULL", false);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(strPartyName.c_str());
	PASSERTMSG_AND_RETURN_VALUE(!pConfParty, "Failed, 'pConfParty' is NULL", false);

	bool RC = (pConfParty->GetRecordingLinkParty() == YES) ? true : false;
	msg << ", RC:" << ((RC) ? "YES" : "NO");
	TRACEINTO << msg.str().c_str();
	return RC;
}

/////////////////////////////////////////////////////////////////
bool CConfAppMngr::IsPartyDialin(PartyRsrcID partyId, char* partyName)
{
	std::ostringstream msg;

	string strPartyName = "";
	if (NULL == partyName)
	{
		int ind = m_partiesList->FindPartyIndex(partyId);
		TRACECOND_AND_RETURN_VALUE(-1 == ind, "PartyId:" << partyId << " - Failed, Party not found", false);

		strPartyName = m_partiesList->m_partyList[ind]->GetPartyName();
		msg << "PartyId:" << partyId << ", PartyName:" << strPartyName.c_str();
	}
	else  // In case this function is called from eCAM_EVENT_PARTY_DELETED
	{
		strPartyName = partyName;
		msg << "PartyName:" << strPartyName.c_str();
	}

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERTMSG_AND_RETURN_VALUE(!pCommConf, "Failed, 'pCommConf' is NULL", false);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(strPartyName.c_str());
	PASSERTMSG_AND_RETURN_VALUE(!pConfParty, "Failed, 'pConfParty' is NULL", false);

	bool RC = (pConfParty->GetConnectionTypeOper() == DIAL_IN) ? true: false;
	msg << ", RC:" << ((RC) ? "YES" : "NO");
	TRACEINTO << msg.str().c_str();
	return RC ;
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::DoRecordingAction(TConfAppEvents opcode)
{
	PartyRsrcID recordingLinkRsrcId = (1 == m_confAppInfo->GetRecordingLinkInConf()) ? m_confAppInfo->GetRecordingLinkRsrcId() : -1;

	#undef LOGINFO
	#define LOGINFO "RecordingLinkPartyId:" << recordingLinkRsrcId << ", Opcode:" << m_confAppInfo->GetStringFromOpcode(opcode)

	switch (opcode)
	{
		case eCAM_EVENT_CONF_START_RESUME_RECORDING:
		case SET_START_RECORDING:
		case SET_RESUME_RECORDING:
		{
			//eFeatureRssDialin
			if (YES == IsPartyDialin(recordingLinkRsrcId))
			{
				// For Dial in case, we just wait for the RL AUDIO_CONNECTED
				WORD recordingLinkConnectionStageIN = CheckStageOfRecordingLinkConnectionForDialin();

				//for dial in case, RMX will send the startDTMF in StageII
				if (2 == recordingLinkConnectionStageIN)
				{
					TRACEINTO << LOGINFO << " - Start recording, dial-in";
					SendRecordingControlToRL(recordingLinkRsrcId, eStartRecording);
					break;
				}

				if ((3 == recordingLinkConnectionStageIN) && (((eCAM_EVENT_CONF_START_RESUME_RECORDING == opcode) && (ePauseRecording == m_confAppInfo->GetRecordingStatus())) || (SET_RESUME_RECORDING == opcode)))
				{
					TRACEINTO << LOGINFO << " - Resume recording, dial-in";
					SendRecordingControlToRL(recordingLinkRsrcId, eResumeRecording);
					break;
				}

				TRACEINTO << LOGINFO << " - Recording action doesn't fit the state, dial-in";
				break;
			}

			// Check if the Recording has started and in what stage the RL is
			WORD recordingLinkConnectionStage = CheckStageOfRecordingLinkConnection();

			if ((1 == recordingLinkConnectionStage) || (2 == recordingLinkConnectionStage))
			{ // Save the the action to "last requested action" variable in CAM info
				m_confAppInfo->SetLastReqRecAction(opcode);
				TRACEINTO << LOGINFO << " - Recording action was saved for further treatment";
				break;
			}

			// In case the recording action does not fit the state (state is already started, or stopped but RL has not disconnected yet)
			if ((3 == recordingLinkConnectionStage) || ((eStopRecording == m_confAppInfo->GetRecordingStatus()) && (0 != m_confAppInfo->GetRecordingLinkInConf())))
			{
				TRACEINTO << LOGINFO << " - Recording action doesn't fit the state";
				break;
			}

			TRACEINTO << LOGINFO;

			// Update recording status in DB
			UpdateRecordingControl(eStartRecording);

			// In case of START Recording (not Resume)
			if ((SET_START_RECORDING == opcode) || ((eCAM_EVENT_CONF_START_RESUME_RECORDING == opcode) && (eStopRecording == m_confAppInfo->GetRecordingStatus())))
			{
				// In case this is the first party in the conference and there is no RL in the conference yet
				if (0 == m_confAppInfo->GetRecordingLinkInConf())
				{
					// Add and connect Recording Link (muteVideo = YES)
					m_confAppInfo->m_pConfApi->AddAndConnectRecordingLink(YES);
					// Start timer
					StartTimer(TIMER_START_RECORDING, 20 * SECOND); ///TBD - check in MGC ???
				}
			}

			// In case of RESUME Recording (not Start)
			if ((SET_RESUME_RECORDING == opcode) || ((eCAM_EVENT_CONF_START_RESUME_RECORDING == opcode) && (ePauseRecording == m_confAppInfo->GetRecordingStatus())))
			{
				// Update CDR
				SendControlRecordingCDR(eResumeRecording, recordingLinkRsrcId); // CDR
				// Update flag in CAM info
				m_confAppInfo->SetStartRecDtmfWasSent(1);

				SendRecordingControlToRL(recordingLinkRsrcId, eStartRecording);

			}
			break;
		}
		case eCAM_EVENT_CONF_STOP_RECORDING:
		case SET_STOP_RECORDING:
		{
			//eFeatureRssDialin
			if (YES == IsPartyDialin(recordingLinkRsrcId))
			{
				// For Dial in case, we just wait for the RL AUDIO_CONNECTED
				WORD recordingLinkConnectionStageIN = CheckStageOfRecordingLinkConnectionForDialin();
				//for dial in case, RMX will send the startDTMF in StageII
				if ((2 == recordingLinkConnectionStageIN) || (1 == recordingLinkConnectionStageIN))
				{
					TRACEINTO << LOGINFO << ", State:" << recordingLinkConnectionStageIN << " - Recording action doesn't fit the state";
					break;
				}

				TRACEINTO << LOGINFO;

				SendRecordingControlToRL(recordingLinkRsrcId, eStopRecording);
				break;
			}

			// Check if the Recording has started and in what stage the RL is
			WORD recordingLinkConnectionStage = CheckStageOfRecordingLinkConnection();

			if ((1 == recordingLinkConnectionStage) || (2 == recordingLinkConnectionStage))
			{ // Save the the action to "last requested action" variable in CAM info
				m_confAppInfo->SetLastReqRecAction(opcode);
				TRACEINTO << LOGINFO << " - Recording action was saved for further treatment";
				break;
			}

			// In case the recording action does not fit the state (state is already stopped)
			if (eStopRecording == m_confAppInfo->GetRecordingStatus())
			{
				TRACEINTO << LOGINFO << " - Recording action doesn't fit the state";
				break;
			}

			TRACEINTO << LOGINFO;

			// Update DB, CDR and CAM Info when RL is disconnecting
			WORD recFailedFlag = NO;
			UpdatesUponRlDisconnection(recordingLinkRsrcId, recFailedFlag);

			// Send DTMF to RSS in order to stop recording (*3)
			SendRecordingControlToRL(recordingLinkRsrcId, eStopRecording);

			// Start timer - the delay between sending the RSS DTMF for stopping the recording and disconnecting the RL
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			std::string key = CFG_KEY_RECORDING_LINK_DISCONNECTION_TIMER_DURATION;
			DWORD recordingLinkDisconnectionTimerDuration;
			sysConfig->GetDWORDDataByKey(key, recordingLinkDisconnectionTimerDuration);

			StartTimer(TIMER_DISCONNECT_RL, recordingLinkDisconnectionTimerDuration * SECOND);
			break;
		}

		case eCAM_EVENT_CONF_PAUSE_RECORDING:
		case SET_PAUSE_RECORDING:
		{
			// In case the recording action does not fit the state (state is already paused or not started yet)
			if (ePauseRecording == m_confAppInfo->GetRecordingStatus())
			{
				TRACEINTO << LOGINFO << " - Recording action doesn't fit the state! Already paused";
				break;
			}
			else if (eStopRecording == m_confAppInfo->GetRecordingStatus())
			{
				TRACEINTO << LOGINFO << " - Recording action doesn't fit the state! Not started yet";
				break;
			}

			//eFeatureRssDialin
			if (YES == IsPartyDialin(recordingLinkRsrcId))
			{
				// For Dial in case, we just wait for the RL AUDIO_CONNECTED
				WORD recordingLinkConnectionStageIN = CheckStageOfRecordingLinkConnectionForDialin();
				//for dial in case, RMX will send the startDTMF in StageII
				if ((2 == recordingLinkConnectionStageIN) || (1 == recordingLinkConnectionStageIN))
				{
					TRACEINTO << LOGINFO << ", State:" << recordingLinkConnectionStageIN << " - Recording action doesn't fit the state";
					break;
				}

				if (eStartRecording != m_confAppInfo->GetRecordingStatus())
				{
					TRACEINTO << LOGINFO << ", State:" << m_confAppInfo->GetRecordingStatus() << " - Recording action doesn't fit the state";
					break;
				}
				TRACEINTO << LOGINFO;

				// Send DTMF to RSS in order to pause recording (*1)
				SendRecordingControlToRL(recordingLinkRsrcId, ePauseRecording);
				break;
			}

			// Check if the Recording has started and in what stage the RL is
			WORD recordingLinkConnectionStage = CheckStageOfRecordingLinkConnection();

			if ((1 == recordingLinkConnectionStage) || (2 == recordingLinkConnectionStage))
			{ // Save the the action to "last requested action" variable in CAM info
				m_confAppInfo->SetLastReqRecAction(opcode);
				TRACEINTO << LOGINFO << " - Recording action was saved for further treatment";
				break;
			}
			TRACEINTO << LOGINFO;

			// Update recording status in DB
			UpdateRecordingControl(ePauseRecording);
			// Update CDR
			SendControlRecordingCDR(ePauseRecording, recordingLinkRsrcId); // CDR

			// Send DTMF to RSS in order to pause recording (*1)
			SendRecordingControlToRL(recordingLinkRsrcId, ePauseRecording);
			break;
		}

		default:
		{
			TRACEINTO << LOGINFO << " - Illegal recording control opcode";
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::SendControlRecordingCDR(WORD recodingCommand, PartyRsrcID partyId)
{
	#undef LOGINFO
	#define LOGINFO "PartyId:" << partyId << ", RecodingCommand:" << recodingCommand

	int nPartyIdx = m_partiesList->FindPartyIndex(partyId);
	TRACECOND_AND_RETURN(-1 == nPartyIdx, LOGINFO << " - Failed, Party not found");

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	TRACECOND_AND_RETURN(!pCommConf, LOGINFO << " - Failed, Conf not found");

	CConfParty* pConfParty = pCommConf->GetCurrentParty(m_partiesList->m_partyList[nPartyIdx]->GetPartyName().c_str());
	TRACECOND_AND_RETURN(!pConfParty, LOGINFO << " - Failed, Party not found");

	pCommConf->ControlRecordingCDR(recodingCommand, pConfParty); // CDR
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::UpdateRecordingControl(eRecordingControl RecodingCommand)
{
	// Checking the conference state
	WORD confState = m_confAppInfo->GetConfState();
	if (confState != eAPP_CONF_STATE_CONNECT)
	{
		PTRACE(eLevelError, "CConfAppMngr::UpdateRecordingControl - Action while conference terminating");
		return;
	}

	m_confAppInfo->m_pConfApi->UpdateRecordingControl(RecodingCommand);

	EIconType eRecordingIcon = E_ICON_REC_OFF;

	if (YES == m_confAppInfo->GetEnableRecordingIcon())
	{
		switch (RecodingCommand)
		{
			case eStartRecording:
			case eResumeRecording:
				eRecordingIcon = E_ICON_REC_ON;
				PTRACE(eLevelInfoNormal, "CConfAppMngr::UpdateRecordingControl - RecordingIcon:E_ICON_ON");
				break;

			case ePauseRecording:
				eRecordingIcon = E_ICON_REC_PAUSE;
				PTRACE(eLevelInfoNormal, "CConfAppMngr::UpdateRecordingControl - RecordingIcon:E_ICON_PAUSE");
				break;

			case eStopRecording:
				eRecordingIcon = E_ICON_REC_OFF;
				PTRACE(eLevelInfoNormal, "CConfAppMngr::UpdateRecordingControl - RecordingIcon:E_ICON_OFF");
				break;
		}
	}
	else
	{
		PTRACE(eLevelInfoNormal, "CConfAppMngr::UpdateRecordingControl - RecordingIcon:Disabled");
	}

	// Notify video bridge to show/hide recording icon
	CSegment* seg = new CSegment;
	*seg << (OPCODE)INDICATION_ICONS_CHANGE << (WORD)eIconRecording << (WORD)eRecordingIcon;
	m_confAppInfo->m_pConfApi->SendMsg(seg, VIDEO_BRIDGE_MSG);
}

/////////////////////////////////////////////////////////////////
WORD CConfAppMngr::CheckStageOfRecordingLinkConnection()
{
	// Stage #1 - before RL AUDIO_CONNECTED indication
	// Stage #2	- after RL AUDIO_CONNECTED indication and before sending *2 to RSS for starting recording
	// Stage #3 - after sending *2 to RSS for starting recording

	WORD rcConnectingStage = (WORD)(-1);

	if (eStartRecording == m_confAppInfo->GetRecordingStatus())
	{
		if (!m_confAppInfo->GetRecordingLinkInConf() && !m_confAppInfo->GetStartRecDtmfWasSent())
			rcConnectingStage = 1;

		if (m_confAppInfo->GetRecordingLinkInConf())
		{
			if (!m_confAppInfo->GetStartRecDtmfWasSent())
				rcConnectingStage = 2;
			else	//	StartRecDtmfWasSent == 1
				rcConnectingStage = 3;
		}
	}

	return rcConnectingStage;
}

/////////////////////////////////////////////////////////////////
WORD CConfAppMngr::CheckStageOfRecordingLinkConnectionForDialin()
{
	// Stage #1 - before the incoming RL AUDIO_CONNECTED indication, or no incoming RL at all
	// Stage #2	- after the incoming RL AUDIO_CONNECTED indication and before sending *2/INFO to RSS for starting recording
	// Stage #3 - after the incoming sending *2/INFO to RSS for starting recording

	WORD rcConnectingStage = (WORD)(-1);
	if (!m_confAppInfo->GetRecordingLinkInConf() && !m_confAppInfo->GetStartRecDtmfWasSent())
		rcConnectingStage = 1;

	if (m_confAppInfo->GetRecordingLinkInConf())
	{
		if (!m_confAppInfo->GetStartRecDtmfWasSent())
			rcConnectingStage = 2;
		else	//	StartRecDtmfWasSent == 1
			rcConnectingStage = 3;
	}

	return rcConnectingStage;
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::UpdatesUponRlDisconnection(PartyRsrcID partyId, WORD recFailedFlag, WORD recUnexpectedDiscon)
{
	TRACEINTO << "PartyId:" << partyId << ", RecordingFailedFlag:" << recFailedFlag;

	// Set Recording Failed Flag in CAM info
	m_confAppInfo->SetRecordingFailedFlag(recFailedFlag);

	// Send fault in case of Recording failure
	if (YES == recFailedFlag)
	{
		if (YES == recUnexpectedDiscon)
		{	// in case of failure in the middle of recording (unexpected disconnection)
			BOOL isFullOnly = FALSE;
			CHlogApi::TaskFault(
					FAULT_GENERAL_SUBJECT,
					AA_RECORDING_LINK_DISCONNECED_UNEXPECTEDLY,
					SYSTEM_MESSAGE,
					"Recording device has disconnected unexpectedly",
					isFullOnly);
		}
		else	// in case of failure before connecting the Recording Link
		{
			BOOL isFullOnly = FALSE;
			CHlogApi::TaskFault(
					FAULT_GENERAL_SUBJECT,
					AA_FAILED_TO_CONNECT_RECORDING_LINK,
					SYSTEM_MESSAGE,
					"Failed to connect to recording device",
					isFullOnly);
		}
	}

	// re-initiate CAM info params
	m_confAppInfo->SetStartRecDtmfWasSent(0);
	m_confAppInfo->SetLastReqRecAction(SET_START_RECORDING);

	// Update status in DB
	UpdateRecordingControl(eStopRecording);
	// Update CDR
	SendControlRecordingCDR(eStopRecording, partyId); // CDR
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::PauseRecordingDuringStarted()
{
	// In case PAUSE action was received during connection of the RL ("last requested recording action" is pause):

	// Update recording status in DB
	UpdateRecordingControl(ePauseRecording);
	// Update CDR
	PartyRsrcID recordingLinkRsrcId =(DWORD)(-1);
	if (1 == m_confAppInfo->GetRecordingLinkInConf())
		recordingLinkRsrcId = m_confAppInfo->GetRecordingLinkRsrcId();
	SendControlRecordingCDR( ePauseRecording, recordingLinkRsrcId ); // CDR
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::StopRecordingDuringStarted()
{
	// In case STOP action was received during connection of the RL ("last requested recording action" is stop):

	// Update DB, CDR and CAM Info when RL is disconnecting
	PartyRsrcID recordingLinkRsrcId =(DWORD)(-1);
	if (1 == m_confAppInfo->GetRecordingLinkInConf())
		recordingLinkRsrcId = m_confAppInfo->GetRecordingLinkRsrcId();
	WORD recFailedFlag = NO;
	UpdatesUponRlDisconnection( recordingLinkRsrcId, recFailedFlag );
	// Update CDR
	SendControlRecordingCDR( eStopRecording, recordingLinkRsrcId ); // CDR

	// Disconnect Recording Link
	m_confAppInfo->m_pConfApi->DisconnectRecordingLink();
	// Set Recording Link flag in CAM info to 0
	m_confAppInfo->SetRecordingLinkInConf(0);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnRecordConfFailure()
{
	DeleteTimer(TIMER_START_RECORDING);
	OnTimerStartRecording(NULL);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::SetOldChairAsRegularPartyIfNeeded(PartyRsrcID newChairPartyId, BYTE isLeader)
{
	if (m_pcmAutorizationLevel != ePcmForChairpersonOnly)
		return;

	PartyRsrcID oldChairPartyId = m_partiesList->GetChairPartyRsrcId();
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERTMSG_AND_RETURN(!pCommConf, "Failed, Conf not found");

	BYTE isCop = pCommConf->GetIsCOP();
	if (isCop)
	{
		if (isLeader == FALSE || (oldChairPartyId != DUMMY_PARTY_ID && oldChairPartyId != newChairPartyId))
		{
			DWORD oldChairIndex = m_partiesList->FindPartyIndex(oldChairPartyId);
			// inform PCM that chair left conf
			if ((DWORD)-1 != oldChairIndex)
			{
				m_partiesList->SetPartyAsLeader(oldChairPartyId,0);
				m_confAppInfo->m_pConfApi->ChairpersonLeftConf(oldChairPartyId);
			}
		}
		if (isLeader)
		{
			// Inform PCM on new chair
			int index = m_partiesList->FindPartyIndex(newChairPartyId);
			if (-1 != index)
			{
				CPartyApi* pPartyApi = m_partiesList->m_partyList[index]->GetPartyApi();
				m_confAppInfo->m_pConfApi->ChairpersonEnteredConf(newChairPartyId, pPartyApi->GetRcvMbx());
			}
		}
	}
	else // CP
	{
		if (isLeader)
		{
			// Inform PCM on new chair
			int index = m_partiesList->FindPartyIndex(newChairPartyId);
			if (-1 != index)
			{
				CPartyApi* pPartyApi = m_partiesList->m_partyList[index]->GetPartyApi();
				m_confAppInfo->m_pConfApi->ChairpersonEnteredConf(newChairPartyId, pPartyApi->GetRcvMbx());
			}
		}
		else
		{
			m_confAppInfo->m_pConfApi->ChairpersonLeftConf(newChairPartyId);
		}
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::InformPcmPartyAdded(PartyRsrcID partyId)
{
	if (m_pcmAutorizationLevel != ePcmForEveryone)
		return;
	int index = m_partiesList->FindPartyIndex(partyId);
	if (-1 != index)
	{
		CPartyApi* pPartyApi = m_partiesList->m_partyList[index]->GetPartyApi();
		m_confAppInfo->m_pConfApi->ChairpersonEnteredConf(partyId, pPartyApi->GetRcvMbx());
	}
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::InformPcmPartyLeft(PartyRsrcID partyId)
{
	if (m_pcmAutorizationLevel != ePcmForEveryone)
		return;

	int index = m_partiesList->FindPartyIndex(partyId);
	if (-1 != index)
		m_confAppInfo->m_pConfApi->ChairpersonLeftConf(partyId);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::SetPcmAutorizationLevel()
{
	m_pcmAutorizationLevel = ePcmNone;

	const char* ivrServiceName = m_confAppInfo->GetIvrServiceName();
	TRACECOND_AND_RETURN(!ivrServiceName, "Failed, Invalid service name");

	CIVRService* pIVRService = ::GetpAVmsgServList()->GetIVRServiceByName(ivrServiceName);
	TRACECOND_AND_RETURN(!pIVRService, "ServiceName:" << DUMPSTR(ivrServiceName) << " - Failed, Service not found");

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	PASSERTMSG_AND_RETURN(!pCommConf, "Failed, Conf not found");

	if (pCommConf->GetIsCOP())
	{
		TRACEINTO << "Video session is COP currently supports only pcmForChairperson setting PCM for chairperson only";
		m_pcmAutorizationLevel = ePcmForChairpersonOnly;
		return;
	}

	WORD permissionLevelFromIvrService = pIVRService->GetDTMFCodePermission(DTMF_START_PCM);
	switch (permissionLevelFromIvrService)
	{
		case DTMF_USER_ACTION:
		{
			TRACEINTO << "Setting PCM for everyone";
			m_pcmAutorizationLevel = ePcmForEveryone;
			break;
		}
		case DTMF_LEADER_ACTION:
		{
			TRACEINTO << "Setting PCM for chairperson only";
			m_pcmAutorizationLevel = ePcmForChairpersonOnly;
			break;
		}
		default:
		{
			TRACEINTO << "PermissionLevel:" << permissionLevelFromIvrService << " - Invalid permission level";
			m_pcmAutorizationLevel = ePcmNone;
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////
DWORD CConfAppMngr::GetPartyToForce(PartyRsrcID partyId)
{
	return m_partiesList->GetPartyToForce(partyId);
}

/////////////////////////////////////////////////////////////////
bool CConfAppMngr::GetIsPartyCascadeLink(PartyRsrcID partyId)
{
	int index = m_partiesList->FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == index, "PartyId:" << partyId << " - Failed, Party not found", false);

	return m_partiesList->m_partyList[index]->GetIsCascadeLinkParty();
}

/////////////////////////////////////////////////////////////////
bool CConfAppMngr::IsPartyWithIVR(const char* partyName, BOOL isTipResumeCall /*= FALSE*/)
{
	CLargeString str;
	bool partyWithIVR = true;
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	BYTE isConfTIP = pCommConf ? pCommConf->GetIsTipCompatibleVideo() : 0;

	if (pCommConf && isConfTIP) //we need to disable IVR for all slaves participants in TIP call
	{
		CConfParty* pConfParty = pCommConf->GetCurrentPartyAccordingToVisualName(partyName);
		if (!pConfParty)
			pConfParty = pCommConf->GetCurrentParty(partyName);

		str
			<< " party_name = " << partyName << " is " << ( pConfParty ? "found" : "not found")
			<< " IsAudioMutedByParty " << (pConfParty ? pConfParty->IsAudioMutedByParty() : -1);

		const bool isTipWithMutedAudio =
			isConfTIP &&
			pConfParty &&
			(pConfParty->IsTIPMasterParty() || pConfParty->IsTIPSlaveParty()) &&
			pConfParty->IsAudioMutedByParty();

		if (isTipWithMutedAudio && !isTipResumeCall && !pCommConf->GetEntryQ())
		{
			str << " => TIP Party with muted media (Not EQ conf) - don't use IVR";
			partyWithIVR = false;
		}
	}

	//BRIDGE12419: disable IVR for CSS plugin in non-EQ
	if(pCommConf&&(!pCommConf->GetEntryQ()))
	{
		CConfParty* pConfParty = pCommConf->GetCurrentPartyAccordingToVisualName(partyName);
		if (!pConfParty)
			pConfParty = pCommConf->GetCurrentParty(partyName);

		str<< " party_name = " << partyName << " is " << ( pConfParty ? "found" : "not found");
		if(pConfParty && pConfParty->GetIsLyncPlugin())
		{
			str << " => CSS Plugin - don't use IVR";
			partyWithIVR = false;
		}
	}

	TRACEINTO << str.GetString();
	return partyWithIVR;
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::MuteAllButLecturer(PartyRsrcID partyId, EOnOff onOff)
{
	DWORD isIVR = FALSE;
	DWORD isForMuteAllButLecture = TRUE;
	CSegment seg;
	seg << (DWORD)((onOff == eOn)? eCAM_EVENT_CONF_MUTE_ALL : eCAM_EVENT_CONF_UNMUTE_ALL)
			<< partyId
			<< isIVR
			<< isForMuteAllButLecture;
	DoConfRequest(&seg);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::UpdateFirstPartyMessagePlayedIfNeeded()
{
	if (m_confAppInfo->IsNeedToBlockIVR())
		m_confAppInfo->SetFirstPartyMessagePlayed(1);
}

/////////////////////////////////////////////////////////////////
BOOL CConfAppMngr::IsPartyNeedToBeMuted(PartyRsrcID partyId)
{
	bool isMuteAllAudioButLeader = m_confAppInfo->GetIsMuteAllAudioButLeader();
	bool isPartyLeader = m_partiesList->GetIsPartyLeader(partyId);

	TRACEINTO << "PartyId:" << partyId << ", IsMuteAllAudioButLeader:" << (int)isMuteAllAudioButLeader << ", IsPartyLeader:" << (int)isPartyLeader;

	return (isMuteAllAudioButLeader && !isPartyLeader);
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::OnTimerPrintLoggerNumber(CSegment* pParam)
{
	if (!m_confAppInfo)
		return;
	StartTimer(TIMER_PRINT_LOGGER_NUMBER, TIMER_PRINT_LOGGER_NUMBER_TIME);

	DWORD currentLogger = ::GetCurrentLoggerNumber();
	if (currentLogger != m_confAppInfo->GetLastPrintedLoggerNumber())
	{
		DWORD dwConfStartedInLog = m_confAppInfo->GetloggerConfStartedNumber();
		if (dwConfStartedInLog == 0xFFFFFFFF)
			{ TRACEINTO << "unknown logger number in which the conf started";}
		else
			{ TRACEINTO << "\nConference  (RsrcId: " << (DWORD) m_confAppInfo->GetConfRsrcID() << ") Started On Log#: " << dwConfStartedInLog << "\n";}
		m_confAppInfo->SetLastPrintedLoggerNumber(currentLogger);
	}
	if (m_partiesList)
		m_partiesList->OnTimerPrintPartiesLogDetailsTbl(currentLogger);
}

/////////////////////////////////////////////////////////////////
bool CConfAppMngr::IsNeedToDoSomethingWithEndIVROrPartyEvent(PartyRsrcID partyId)
{
	int nPartyIdx = m_partiesList->FindPartyIndex(partyId);
	TRACECOND_AND_RETURN_VALUE(-1 == nPartyIdx, "PartyId:" << partyId << " - Failed, Party not found", false);

	string partyName = m_partiesList->m_partyList[nPartyIdx]->GetPartyName();

	if (!(m_partiesList->IsTipParty(partyName.c_str()))) //not TIP
		return true;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());
	TRACECOND_AND_RETURN_VALUE(!pCommConf, "PartyId:" << partyId << " - Failed, Conference not found", false);

	CConfParty* pConfParty = pCommConf->GetCurrentParty(partyName.c_str());
	TRACECOND_AND_RETURN_VALUE(!pConfParty, "PartyId:" << partyId << " - Failed, Party not found", false);

	bool isTipAUX = pConfParty->IsTIPAuxParty();

	TRACEINTO "PartyId:" << partyId << ", IsTipAUX:" << (int)isTipAUX;

	return isTipAUX;
}

/////////////////////////////////////////////////////////////////
void CConfAppMngr::SendRecordingControlToRL(PartyRsrcID partyId, eRecordingControl recodingCommand)
{
	if (partyId == static_cast<DWORD>(-1))
	{
		TRACEINTO << "Invalid PartyRsrcId!";
		return;
	}

	CSegment seg;
	BOOL viaInfo = ::GetFlagEnableRssControlViaInfo();

	switch (recodingCommand)
	{
		case eStartRecording:
		case eResumeRecording:
		{
			seg << SRS_CONTROL_START_RESUME;
			break;
		}
		case ePauseRecording:
		{
			seg << SRS_CONTROL_PAUSE;
			break;
		}
		case eStopRecording:
		{
			seg << SRS_CONTROL_STOP;
			break;
		}
		default:
		{
			TRACEINTO << "RecordingCommand:" << recodingCommand << " - Failed, Invalid recording command";
			return;
		}
	}
	//save the action for both DTMF and Signaling
	m_confAppInfo->SetRecordingControlActionSaved(recodingCommand);

	//BRIDGE-12715
	CConfParty* pConfParty = NULL;
	int ind = m_partiesList->FindPartyIndex(partyId);

    //added by Richer for VNGFE-8060
    if (-1 == ind)
    {
        TRACEINTO << "Party doesn't exist in list.";
        return;
    }
	string strPartyName = m_partiesList->m_partyList[ind]->GetPartyName();
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_confAppInfo->GetConfName());

	if (pCommConf)
		pConfParty = pCommConf->GetCurrentParty(strPartyName.c_str());

	if (pConfParty && (SIP_INTERFACE_TYPE == pConfParty->GetNetInterfaceType()) && viaInfo)
	{
		//Send Dtmf outband
		DoSendDtmfViaSignaling(partyId, &seg);
	}
	else
	{
		// Send DTMF to RSS
		DoSendDtmf(partyId, &seg);
	}
	return;
}

/////////////////////////////////////////////////////////////////////
void CConfAppMngr::DoSendDtmfViaSignaling(PartyRsrcID partyId, CSegment* pParam)
{
	if (partyId == static_cast<DWORD>(-1))
	{
		TRACEINTO << "Failed, Invalid party id";
		return;
	}
	if (m_confAppInfo->GetIsRecordingControlInProgress())
	{
		TRACEINTO << "Block current operation, the Recording-Control is in progress!";
		BYTE inProgressStatus = static_cast<BYTE>(eSrsRecordingControlInProgress);
		HandleRecordingControlStatus(inProgressStatus);
		return;
	}
	int index = m_partiesList->FindPartyIndex(partyId);
	if (-1 != index)
	{
		CPartyApi* pPartyApi = m_partiesList->m_partyList[index]->GetPartyApi();
		pPartyApi->SendRecordingControlInfoToParty(pParam);
		m_confAppInfo->SetIsRecordingControlInProgress(1);
		StartTimer(TIMER_RECORDING_CONTROL, TIMER_RECORDING_CONTROL_LEN);
	}
}

/////////////////////////////////////////////////////////////////////
void CConfAppMngr::HandleRecordingControlStatus(BYTE status)
{
	TRACEINTO << "Status:" << static_cast<int>(status);

	//Clear flag and Timer (not in 'In-progress' status)
	if (m_confAppInfo->GetIsRecordingControlInProgress() && (status != static_cast<BYTE>(eSrsRecordingControlInProgress)))
	{
		m_confAppInfo->SetIsRecordingControlInProgress(0);
		//Delete timer
		DeleteTimer(TIMER_RECORDING_CONTROL);
	}
	PartyRsrcID recordingLinkRsrcId = (1 == m_confAppInfo->GetRecordingLinkInConf()) ? m_confAppInfo->GetRecordingLinkRsrcId() : -1;
	if (recordingLinkRsrcId == static_cast<DWORD>(-1))
	{
		TRACEINTO << "Failed, Invalid party id";
		return;
	}

	switch (status)
	{
		case eSrsRecordingControlStarted:
		{
			// Update recording status in DB
			UpdateRecordingControl(eStartRecording);
			// Update flag in CAM info - in case we received 'Started' before we send any command
			m_confAppInfo->SetStartRecDtmfWasSent(1);
			//m_confAppInfo->SetLastReqRecAction( 0xFFFF);
			DeleteTimer(TIMER_SEND_START_DTMF);

			if (eStartRecording == m_confAppInfo->GetRecordingControlActionSaved())
			{
				// Update CDR
				SendControlRecordingCDR(eStartRecording, recordingLinkRsrcId);
			}
			else if (eResumeRecording == m_confAppInfo->GetRecordingControlActionSaved())
			{
				// Update CDR
				SendControlRecordingCDR(eResumeRecording, recordingLinkRsrcId);
			}
			else
			{
				TRACEINTO << "LastReqRecAction:" << m_confAppInfo->GetLastReqRecAction() << " - Faield, invalid lastAction when receive status 'STARTED'";
			}
			break;
		}
		case eSrsRecordingControlPaused:
		{
			// Update recording status in DB
			UpdateRecordingControl(ePauseRecording);
			// Update CDR
			SendControlRecordingCDR(ePauseRecording, recordingLinkRsrcId);
			break;
		}
		case eSrsRecordingControlStopped:
		{
			//Do not disconnect the recording link if it's reported by CS
			if(eStopRecording != m_confAppInfo->GetRecordingControlActionSaved())
			{
				// Update recording status in DB
				UpdateRecordingControl(eStopRecording);
				// Update CDR
				SendControlRecordingCDR(eStopRecording, recordingLinkRsrcId);
			}
			else
			{
			// Update DB, CDR and CAM Info when RL is disconnecting
			WORD recFailedFlag = NO;

			UpdatesUponRlDisconnection(recordingLinkRsrcId, recFailedFlag);
			// Start timer - the delay between sending the RSS DTMF for stopping the recording and disconnecting the RL
			CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			std::string key = CFG_KEY_RECORDING_LINK_DISCONNECTION_TIMER_DURATION;
			DWORD recordingLinkDisconnectionTimerDuration;
			sysConfig->GetDWORDDataByKey(key, recordingLinkDisconnectionTimerDuration);

			StartTimer(TIMER_DISCONNECT_RL, recordingLinkDisconnectionTimerDuration * SECOND);
			}			
			break;
		}
		case eSrsRecordingControlFailed:
		{
			// nothing to do currently
			break;
		}
		case eSrsRecordingControlInProgress:
		{
			// nothing to do currently
			break;
		}
		case eSrsRecordingControlTOUT:
		{
			// nothing to do currently
			break;
		}
		default:
			break;
	}
	return;
}

/////////////////////////////////////////////////////////////////////
void CConfAppMngr::OnTimerRecordingControlTOUT(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CConfAppMngr::OnTimerRecordingControlTOUT");
	BYTE tOutStatus = static_cast<BYTE>(eSrsRecordingControlTOUT);
	HandleRecordingControlStatus(tOutStatus);
	return;
}
