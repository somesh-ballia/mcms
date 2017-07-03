#include "AudioBridge.h"
#include "AudioBridgePartyCntl.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgePartyExportParams.h"
#include "AudioHardwareInterface.h"
#include "AudioBridgeInitParams.h"
#include "OpcodesMcmsAudio.h"
#include "OpcodesMcmsAudioCntl.h"
#include "OpcodesMcmsCardMngrIvrCntl.h"
#include "OpcodesMcmsInternal.h"
#include "TraceStream.h"
#include "HostCommonDefinitions.h"
#include "AcIndicationStructs.h"
#include "Party.h"

PBEGIN_MESSAGE_MAP(CAudioBridge)

	ONEVENT(UPDATE_CONF_PARAMS,             CONNECTED,          CAudioBridge::OnConfUpdateConfParamsCONNECTED)
	ONEVENT(UPDATE_CONF_PARAMS,             DISCONNECTING,      CAudioBridge::OnConfUpdateConfParamsDISCONNECTING)

	ONEVENT(START_LOOK_FOR_ACTIVE_SPEAKER,  CONNECTED,          CAudioBridge::OnConfUpdateACLayoutChangeCompleteCONNECTED)
	ONEVENT(START_LOOK_FOR_ACTIVE_SPEAKER,  DISCONNECTING,      CAudioBridge::NullActionFunction)
	ONEVENT(DISCONNECTCONF,                 CONNECTED,          CAudioBridge::OnConfDisConnectConfCONNECTED)
	ONEVENT(DISCONNECTCONF,                 DISCONNECTING,      CAudioBridge::OnConfDisConnectConfDISCONNECTING)

	ONEVENT(TERMINATE,                      CONNECTED,          CAudioBridge::OnConfTerminateCONNECTED)
	ONEVENT(TERMINATE,                      DISCONNECTING,      CAudioBridge::OnConfTerminateDISCONNECTING)

	ONEVENT(CONNECTPARTY,                   CONNECTED,          CAudioBridge::OnConfConnectPartyCONNECTED)
	ONEVENT(CONNECTPARTY,                   DISCONNECTING,      CAudioBridge::OnConfConnectPartyDISCONNECTING)

	ONEVENT(ENDCONNECTPARTY,                CONNECTED,          CAudioBridge::OnEndPartyConnectCONNECTED)
	ONEVENT(ENDCONNECTPARTY,                DISCONNECTING,      CAudioBridge::OnEndPartyConnectDISCONNECTING)

	ONEVENT(DISCONNECTPARTY,                CONNECTED,          CAudioBridge::OnConfDisconnectParty)
	ONEVENT(DISCONNECTPARTY,                DISCONNECTING,      CAudioBridge::OnConfDisconnectParty)

	ONEVENT(ENDDISCONNECTPARTY,             CONNECTED,          CAudioBridge::OnEndPartyDisConnect)
	ONEVENT(ENDDISCONNECTPARTY,             DISCONNECTING,      CAudioBridge::OnEndPartyDisConnect)

	ONEVENT(EXPORTPARTY,                    CONNECTED,          CAudioBridge::OnConfExportPartyCONNECTED)
	ONEVENT(EXPORTPARTY,                    DISCONNECTING,      CAudioBridge::OnConfExportPartyDISCONNECTING)

	ONEVENT(ENDEXPORTPARTY,                 CONNECTED,          CAudioBridge::OnEndPartyExportCONNECTED)
	ONEVENT(ENDEXPORTPARTY,                 DISCONNECTING,      CAudioBridge::OnEndPartyExportDISCONNECTING)

	ONEVENT(AC_ACTIVE_SPEAKER_IND,          CONNECTED,          CAudioBridge::OnActiveSpeakerChangeCONNECTED)
	ONEVENT(AC_ACTIVE_SPEAKER_IND,          DISCONNECTING,      CAudioBridge::NullActionFunction)

	ONEVENT(IVR_PLAY_MESSAGE_REQ,           CONNECTED,          CAudioBridge::OnConfPlayMsgCONNECT)
	ONEVENT(IVR_STOP_PLAY_MESSAGE_REQ,      CONNECTED,          CAudioBridge::OnConfStopPlayMsgCONNECT)
	ONEVENT(IVR_START_IVR_REQ,              CONNECTED,          CAudioBridge::OnConfStartIvrSeqCONNECT)
	ONEVENT(IVR_STOP_IVR_REQ,               CONNECTED,          CAudioBridge::OnConfStopIvrSeqCONNECT)
	ONEVENT(IVR_PLAY_MUSIC_REQ,             CONNECTED,          CAudioBridge::OnConfPlayMusicCONNECT)
	ONEVENT(IVR_STOP_PLAY_MUSIC_REQ,        CONNECTED,          CAudioBridge::OnConfStopPlayMusicCONNECT)

	ONEVENT(IVR_STOP_PLAY_MESSAGE_REQ,      DISCONNECTING,      CAudioBridge::OnConfStopPlayMsgDISCONNECTING)
	ONEVENT(IVR_STOP_IVR_REQ,               DISCONNECTING,      CAudioBridge::OnConfStopIvrSeqDISCONNECTING)
	ONEVENT(IVR_STOP_PLAY_MUSIC_REQ,        DISCONNECTING,      CAudioBridge::OnConfStopPlayMusicDISCONNECTING)

	// Api Requests
	ONEVENT(UPDATE_AUDIO_VOLUME,            CONNECTED,          CAudioBridge::OnConfUpdateAudioVolumeCONNECTED)
	ONEVENT(UPDATE_AUDIO_VOLUME,            DISCONNECTING,      CAudioBridge::NullActionFunction)

	ONEVENT(AUDIOMUTE,                      CONNECTED,          CAudioBridge::OnConfUpdateAudioMuteCONNECTED)
	ONEVENT(AUDIOMUTE,                      DISCONNECTING,      CAudioBridge::NullActionFunction)

	ONEVENT(UPDATE_AGC_EXEC,                CONNECTED,          CAudioBridge::OnConfUpdateAgcCONNECTED)
	ONEVENT(UPDATE_AGC_EXEC,                DISCONNECTING,      CAudioBridge::NullActionFunction)

	ONEVENT(AC_OPEN_CONF_RESEND,            CONNECTED,          CAudioBridge::OnConfOpenConfReSendCONNECTED)
	ONEVENT(AC_OPEN_CONF_RESEND,            DISCONNECTING,      CAudioBridge::NullActionFunction)

	ONEVENT(UPDATE_AUDIO_ON_SEEN_IMAGE,     CONNECTED,          CAudioBridge::OnVideoUpdateSeenImageCONNECTED)
	ONEVENT(UPDATE_AUDIO_ON_SEEN_IMAGE,     DISCONNECTING,      CAudioBridge::NullActionFunction)
PEND_MESSAGE_MAP(CAudioBridge, CBridge);

////////////////////////////////////////////////////////////////////////////
//                        CAudioBridge
////////////////////////////////////////////////////////////////////////////
CAudioBridge::CAudioBridge() : CBridge(),
	m_pCurrentSpeakerParty(NULL),
	m_wTalkHoldTime(0),
	m_byAudioMixDepth(0),
	m_isAutoMuteNoisyParties(FALSE),
	m_pAudioHardwareInterface(NULL)
{
	m_ePT = CProcessBase::GetProcess()->GetProductType();
}

//--------------------------------------------------------------------------
CAudioBridge::~CAudioBridge()
{
	if (m_pAudioHardwareInterface)
		POBJDELETE(m_pAudioHardwareInterface);
}

//--------------------------------------------------------------------------
void CAudioBridge::Create(const CAudioBridgeInitParams* pAudioBridgeInitParams)
{
	if (!CPObject::IsValidPObjectPtr(pAudioBridgeInitParams))
	{
		PASSERTSTREAM(1, "Failed, invalid AudioBridgeInitParams");
		m_pConfApi->EndAudBrdgConnect(statInconsistent);
		return;
	}

	CBridge::Create((CBridgeInitParams*)pAudioBridgeInitParams);

	m_wTalkHoldTime   = pAudioBridgeInitParams->GetTalkHoldTime();
	m_byAudioMixDepth = pAudioBridgeInitParams->GetAudioMixDepth();
	m_isAutoMuteNoisyParties = pAudioBridgeInitParams->GetAutoMuteNoisyParties();

	if (m_pAudioHardwareInterface)
		POBJDELETE(m_pAudioHardwareInterface);

	if (DUMMY_CONF_ID == m_confRsrcID)
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, ConfId is not set yet";
	}
	else
	{
		CRsrcParams rsrcParams(DUMMY_CONNECTION_ID, DUMMY_PARTY_ID, m_confRsrcID, eLogical_res_none);
		m_pAudioHardwareInterface = new CAudioHardwareInterface();
		m_pAudioHardwareInterface->Create(&rsrcParams);
	}

	m_state = CONNECTED;

	m_pConfApi->EndAudBrdgConnect(statOK);
}

//--------------------------------------------------------------------------
void CAudioBridge::Destroy()
{
	CBridge::Destroy();

	if (m_pAudioHardwareInterface)
		POBJDELETE(m_pAudioHardwareInterface);
}

//--------------------------------------------------------------------------
void CAudioBridge::HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode)
{
	DispatchEvent(opCode, pMsg);
}

//--------------------------------------------------------------------------
void* CAudioBridge::GetMessageMap()
{
	return m_msgEntries;
}

//--------------------------------------------------------------------------
void CAudioBridge::UpdateAudioAlgorithm(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection,
                                        DWORD newAudioAlgorithm, DWORD maxAverageBitrate)
{
	CAudioBridgePartyCntl* pAudBridgePartyCtl = ((CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID));

	if (!CPObject::IsValidPObjectPtr(pAudBridgePartyCtl))
	{
		PASSERTSTREAM_AND_RETURN(1, "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << " - Failed, invalid party");
	}

	pAudBridgePartyCtl->UpdateAudioAlgorithm(eMediaDirection, newAudioAlgorithm, maxAverageBitrate);
}


//--------------------------------------------------------------------------
void CAudioBridge::UpdateMediaType(PartyRsrcID partyRsrcID, EMediaTypeUpdate eMediaTypeUpdate)
{
	CAudioBridgePartyCntl* pAudBridgePartyCtl = ((CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID));

	if (!CPObject::IsValidPObjectPtr(pAudBridgePartyCtl))
	{
		PASSERTSTREAM_AND_RETURN(1, "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << " - Failed, invalid party");
	}

	pAudBridgePartyCtl->UpdateMediaType(eMediaTypeUpdate);
}

//--------------------------------------------------------------------------
// speakerIndication - update
void CAudioBridge::UpdateUseSpeakerSsrcForTx(PartyRsrcID partyRsrcID, BOOL updatedUseSpeakerSsrcForTx)
{
	CAudioBridgePartyCntl* pAudBridgePartyCtl = ((CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID));

	if (!CPObject::IsValidPObjectPtr(pAudBridgePartyCtl))
	{
		PASSERTSTREAM_AND_RETURN(1, "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << " - Failed, invalid party");
	}

	pAudBridgePartyCtl->UpdateUseSpeakerSsrcForTx(updatedUseSpeakerSsrcForTx);
}

//--------------------------------------------------------------------------
void CAudioBridge::UpdateMute(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, EOnOff eOnOff, WORD srcRequest)
{
	CSegment* pSeg = new CSegment;

	if (srcRequest == OPERATOR)
	{
		CAudioBridgePartyCntl* pAudBridgePartyCtl = ((CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID));
		if (!CPObject::IsValidPObjectPtr(pAudBridgePartyCtl))
		{
			POBJDELETE(pSeg);
			PASSERTSTREAM_AND_RETURN(1, "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << " - Failed, invalid party");
		}

		*pSeg << srcRequest
		      << pAudBridgePartyCtl->GetName()
		      << (BYTE)eOnOff
		      << (BYTE)eMediaDirection;
	}
	else
	{
		*pSeg << srcRequest
		      << (DWORD)partyRsrcID
		      << (BYTE)eOnOff
		      << (BYTE)eMediaDirection;
	}

	DispatchEvent(AUDIOMUTE, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::UpdateAudioVolume(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, DWORD newAudioVolume)
{
	CAudioBridgePartyCntl* pAudBridgePartyCtl = ((CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID));

	if (!CPObject::IsValidPObjectPtr(pAudBridgePartyCtl))
	{
		PASSERTSTREAM_AND_RETURN(1, "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << " - Failed, invalid party");
	}

	pAudBridgePartyCtl->UpdateAudioVolume(eMediaDirection, newAudioVolume);
}

//--------------------------------------------------------------------------
void CAudioBridge::IncreaseAudioVolume(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, BYTE increaseRate)
{
	CAudioBridgePartyCntl* pAudBridgePartyCtl = ((CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID));

	if (!CPObject::IsValidPObjectPtr(pAudBridgePartyCtl))
	{
		PASSERTSTREAM_AND_RETURN(1, "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << " - Failed, invalid party");
	}

	pAudBridgePartyCtl->IncreaseAudioVolume(eMediaDirection, increaseRate);
}

//--------------------------------------------------------------------------
void CAudioBridge::DecreaseAudioVolume(PartyRsrcID partyRsrcID, EMediaDirection eMediaDirection, BYTE decreaseRate)
{
	CAudioBridgePartyCntl* pAudBridgePartyCtl = ((CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID));

	if (!CPObject::IsValidPObjectPtr(pAudBridgePartyCtl))
	{
		PASSERTSTREAM_AND_RETURN(1, "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << " - Failed, invalid party");
	}

	pAudBridgePartyCtl->DecreaseAudioVolume(eMediaDirection, decreaseRate);
}

//--------------------------------------------------------------------------
void CAudioBridge::UpdateNoiseDetection(PartyRsrcID partyRsrcID, EOnOff eOnOff, BYTE newNoiseDetectionThreshold)
{
	CAudioBridgePartyCntl* pAudBridgePartyCtl = ((CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID));

	if (!CPObject::IsValidPObjectPtr(pAudBridgePartyCtl))
	{
		PASSERTSTREAM_AND_RETURN(1, "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << " - Failed, invalid party");
	}

	pAudBridgePartyCtl->UpdateNoiseDetection(eOnOff, newNoiseDetectionThreshold);
}

//--------------------------------------------------------------------------
void CAudioBridge::UpdateConfParams(WORD newTalkHoldTime, BYTE newAudioMixDepth)
{
	if (IsStandaloneConf()) // UpdateConfParams never sent in stand-alone conferences
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Request ignored in stand-alone conference";
		return;
	}

	m_wTalkHoldTime   = newTalkHoldTime;
	m_byAudioMixDepth = newAudioMixDepth;

	DispatchEvent(UPDATE_CONF_PARAMS, NULL);
}

//--------------------------------------------------------------------------
void CAudioBridge::OpenConf(WORD bReOpenConf, WORD new_card_board_id)
{
	if (IsStandaloneConf()) // OpenConf/CloseConf never sent in stand-alone conferences
		return;
	CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	// BRIDGE-9818 active_speaker_preference - 0 to 6, denotes speaker change sensitivity
	DWORD active_speaker_preference  = 0;
	if (pSysConfig)
		pSysConfig->GetDWORDDataByKey("ACTIVE_SPEAKER_PREFERENCE", active_speaker_preference);
	if (m_pAudioHardwareInterface)
		((CAudioHardwareInterface*)m_pAudioHardwareInterface)->OpenConf(m_wTalkHoldTime, m_byAudioMixDepth, m_isAutoMuteNoisyParties, bReOpenConf, new_card_board_id, (WORD)active_speaker_preference);
	else
		DBGPASSERT_AND_RETURN(1);
}

//--------------------------------------------------------------------------
void CAudioBridge::CloseConf()
{
	if (IsStandaloneConf()) // OpenConf/CloseConf never sent in stand-alone conferences
		return;

	if (m_pAudioHardwareInterface)
		((CAudioHardwareInterface*)m_pAudioHardwareInterface)->CloseConf();
	else
		DBGPASSERT_AND_RETURN(1);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfUpdateConfParamsCONNECTED(CSegment* pParam)
{
	if (m_pAudioHardwareInterface)
		((CAudioHardwareInterface*)m_pAudioHardwareInterface)->UpdateConf(m_wTalkHoldTime, m_byAudioMixDepth);
	else
		DBGPASSERT_AND_RETURN(1);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfUpdateConfParamsDISCONNECTING(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << "ConfName:" << m_pConfName << " - Ignored (Bridge is disconnecting)";
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfUpdateACLayoutChangeCompleteCONNECTED(CSegment* pParam)
{
	if (m_pAudioHardwareInterface)
		((CAudioHardwareInterface*)m_pAudioHardwareInterface)->ACLayoutChangeComplete();
	else
		DBGPASSERT_AND_RETURN(1);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfConnectPartyCONNECTED(CSegment* pParam)
{
	CBridgePartyInitParams partyInitParams;
	partyInitParams.DeSerialize(NATIVE, *pParam);

	if (!partyInitParams.IsValidParams())
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, invalid audio params";
		CBridge::RejectPartyConnect(partyInitParams.GetParty(), PARTY_AUDIO_DISCONNECTED, statInvalidPartyInitParams);
		return;
	}

	// Send OpenConf to Audio Controller before first party connection
	// NOTE: This is the temp solution for Carmel V1. In final solution MPL-API will initiate
	// openConf & closeConf requests to Audio Controller
	if (0 == GetNumParties())
		OpenConf();

	const CParty* pParty = (CParty*)partyInitParams.GetParty();
	PartyRsrcID partyId = pParty->GetPartyRsrcID();

	CAudioBridgePartyCntl* pAudBrdgPartyCntl = (CAudioBridgePartyCntl*)GetPartyCntl(partyId);

	// If this party exist in this conf...
	// We don't need to create new AudioBridgePartyCntl - only add or update one of the directions -audioIn or audioOut
	if (pAudBrdgPartyCntl)
	{
		TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyId << " - Connecting existing party";

		pAudBrdgPartyCntl->Update(&partyInitParams);
		pAudBrdgPartyCntl->Connect(partyInitParams.IsIvrInConf());
	}
	else
	{
		// If move from other conf....
		pAudBrdgPartyCntl = (CAudioBridgePartyCntl*)partyInitParams.GetPartyCntl();
		if (pAudBrdgPartyCntl)
		{
			TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << pAudBrdgPartyCntl->GetPartyRsrcID() << " - Importing existing party";

			pAudBrdgPartyCntl->Import(&partyInitParams);
		}
		else
		{
			TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyInitParams.GetPartyRsrcID() << " - Creating new AudioBridgePartyCntl";

			pAudBrdgPartyCntl = new CAudioBridgePartyCntl();
			pAudBrdgPartyCntl->Create(&partyInitParams);
		}

		// Insert the party to the PartyCtl List and activate Connect on it
		ConnectParty(pAudBrdgPartyCntl, partyInitParams.IsIvrInConf());
	}
}

//--------------------------------------------------------------------------
void CAudioBridge::ConnectParty(CAudioBridgePartyCntl* pBridgePartyCntl, BYTE isIVR)
{
	m_pPartyList->Insert(pBridgePartyCntl);

	pBridgePartyCntl->Connect(isIVR);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfConnectPartyDISCONNECTING(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << "ConfName:" << m_pConfName << " - Party connection rejected";
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfDisconnectParty(CSegment* pParam)
{
	CBridgePartyDisconnectParams partyDisconnectParams;
	partyDisconnectParams.DeSerialize(NATIVE, *pParam);

	EMediaDirection eMediaDirection = partyDisconnectParams.GetMediaDirection();
	PartyRsrcID PartyId = partyDisconnectParams.GetPartyId();

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId << ", MediaDirection:" << eMediaDirection;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(PartyId);
	PASSERT_AND_RETURN(!pParty);

	CAudioBridgePartyCntl* pPartyCntl = (CAudioBridgePartyCntl*)GetPartyCntl(PartyId);
	if (!pPartyCntl)
	{
		// already disconnected
		m_pConfApi->PartyBridgeResponseMsgById(PartyId, AUDIO_BRIDGE_MSG, PARTY_AUDIO_DISCONNECTED, statOK, 1);
		return;
	}

	pPartyCntl->SetDisConnectingDirectionsReq(eMediaDirection);

	// VNGR-25046 - Since we are disconnecting existing party (DMA transfer?) party, lets unmute audio
	if (pPartyCntl->IsDirectionMuteSrcSet(eMediaDirection, PARTY))
		pPartyCntl->UpdateMute(eMediaDirection, eOff, ALL);

	CBridge::DisconnectParty(PartyId);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfExportPartyCONNECTED(CSegment* pParam)
{
	CBridgePartyExportParams bridgePartyExportParams;
	bridgePartyExportParams.DeSerialize(NATIVE, *pParam);

	PartyRsrcID PartyId = bridgePartyExportParams.GetPartyId();

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId;

	const CTaskApp* pParty = (CTaskApp*)GetLookupTableParty()->Get(PartyId);
	PASSERT_AND_RETURN(!pParty);

	CAudioBridgePartyCntl* pPartyCntl = (CAudioBridgePartyCntl*)GetPartyCntl(PartyId);
	if (!pPartyCntl)
	{
		TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId << " - Party audio not connected (cannot export)";
		m_pConfApi->SendResponseMsg(pParty, AUDIO_BRIDGE_MSG, PARTY_AUDIO_EXPORTED, statIllegal, 1);
		return;
	}

	if (FALSE == pPartyCntl->IsConnected())
	{
		TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << PartyId << " - Party audio not in connected state (cannot export)";
		m_pConfApi->SendResponseMsg(pParty, AUDIO_BRIDGE_MSG, PARTY_AUDIO_EXPORTED, statIllegal, 1);
		return;
	}

	CBridge::ExportParty(PartyId);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfExportPartyDISCONNECTING(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << "ConfName:" << m_pConfName << " - Party export rejected";
}

//--------------------------------------------------------------------------
void CAudioBridge::OnEndPartyExportCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	CBridge::EndPartyExport(pParam, PARTY_AUDIO_EXPORTED);

	// Send CloseConf to Audio Controller after last party disconnection
	// NOTE: This is the temp solution for Carmel V1. In final solution MPL-API will initiate
	// openConf & closeConf requests to Audio Controller
	if (0 == GetNumParties())
		CloseConf();
}

//--------------------------------------------------------------------------
void CAudioBridge::OnEndPartyExportDISCONNECTING(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << "ConfName:" << m_pConfName << " - Do nothing...";
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfDisConnectConfCONNECTED(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << "ConfName:" << m_pConfName << " - Changing state to DISCONNECTING...";

	// Change to disconnecting state - for preventing connections of new parties
	m_state = DISCONNECTING;
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfDisConnectConfDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName << " - Ignored (Bridge is already disconnecting)";
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfTerminateDISCONNECTING(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	EStat eStatus = statOK;

	m_state = IDLE;

	if (0 != GetNumParties())
	{
		// Upon receiving TERMINATE event, Bridge should be empty
		eStatus = statBridgeIsNotEmpty;
	}

	m_pConfApi->EndAudBrdgDisConnect(eStatus);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfTerminateCONNECTED(CSegment* pParam)
{
	PASSERTSTREAM(1, "ConfName:" << m_pConfName << " - The audio bridge should be in disconnecting state");
}

//--------------------------------------------------------------------------
void CAudioBridge::OnEndPartyConnectCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	CBridge::EndPartyConnect(pParam, PARTY_AUDIO_CONNECTED);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnEndPartyConnectDISCONNECTING(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << "ConfName:" << m_pConfName << " - Do nothing...";
}

//--------------------------------------------------------------------------
void CAudioBridge::OnEndPartyDisConnect(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	CBridge::EndPartyDisConnect(pParam, PARTY_AUDIO_DISCONNECTED);

	// Send CloseConf to Audio Controller after last party disconnection
	// NOTE: This is the temp solution for Carmel V1. In final solution MPL-API will initiate
	// openConf & closeConf requests to Audio Controller
	if (0 == GetNumParties())
		CloseConf();
}

//--------------------------------------------------------------------------
// Indicates the active speaker from amongst only the Video Parties in the conference and also indicate
// the Audio Speaker from amongst ALL the Parties in the conference.
//
// Will affect: 1. Video Layout
// 2. Speaker Notation in layout
// 3. Update EMA Icon indication
// 4. Presentation Mode -> Stop / Start Current Lecture Mode
// (explanation: we want the 'real ' ACTIVE_SPEAKER in the conference to become lecturer. If he is not a Video party ??? not connected to video bridge ??? we rather there be no lecturer in the conference )

void CAudioBridge::OnActiveSpeakerChangeCONNECTED(CSegment* pParam)
{
	TRACEINTO << "ConfName:" << m_pConfName;

	SpeakersChanged(pParam);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfOpenConfReSendCONNECTED(CSegment* pParam)
{
	WORD new_card_board_id = (WORD)-1;
	*pParam >> new_card_board_id;

	TRACEINTO << "ConfName:" << m_pConfName << ", BoardId:" << new_card_board_id;

	if (0 < GetNumParties())
		OpenConf(TRUE, new_card_board_id);
}

//--------------------------------------------------------------------------
void CAudioBridge::SpeakersChanged(CSegment* pParam)
{
	PartyRsrcID partyVideoRsrcID = DUMMY_PARTY_ID;
	PartyRsrcID partyAudioRsrcID = DUMMY_PARTY_ID;
	DWORD       partyDominantSpeakerMSI = DUMMY_DOMINANT_SPEAKER_MSI;

	*pParam >> (DWORD&)partyVideoRsrcID;
	*pParam >> (DWORD&)partyAudioRsrcID;

	if (eProductTypeSoftMCUMfw == m_ePT)
		UpdateSpeakerList( pParam );	// inside we get the Active Speaker List params
	else
	{
		DWORD tmp = 0;
		for(int i=0; i< MAX_ACTIVE_SPEAKER_LIST;i++)
			*pParam >> tmp;
	}

	*pParam >> partyDominantSpeakerMSI;

	CTaskApp* pPartyVideoTaskApp = NULL;
	CTaskApp* pPartyAudioTaskApp = NULL;
	// Get the Video Speaker from the Audio Bridge
	CAudioBridgePartyCntl* pVideoPartyCntl = (CAudioBridgePartyCntl*)(GetPartyCntl(partyVideoRsrcID));
	// Get the NEW Audio Speaker from the Audio Bridge
	CAudioBridgePartyCntl* pAudioPartyCntl = (CAudioBridgePartyCntl*)(GetPartyCntl(partyAudioRsrcID));

	if (pAudioPartyCntl)
	{
		pPartyAudioTaskApp = pAudioPartyCntl->GetPartyTaskApp();
		DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyAudioTaskApp));
		if (pVideoPartyCntl)
		{
			pPartyVideoTaskApp = pVideoPartyCntl->GetPartyTaskApp();
			DBGPASSERT_AND_RETURN(!CPObject::IsValidPObjectPtr(pPartyVideoTaskApp));
		}
	}

	else
	{
		if (!pAudioPartyCntl && !pVideoPartyCntl) // Case of single party in IVR
		{
			TRACEINTO << "No speaker in conference";
		}
		else
		{
			PASSERTMSG_AND_RETURN(1, "Wrong ACTIVE_SPEAKER params");
		}
	}
	TRACEINTO << " Dominant Speakse MSI is: " << partyDominantSpeakerMSI;
	m_pConfApi->SpeakersChanged(pPartyVideoTaskApp, pPartyAudioTaskApp,partyDominantSpeakerMSI);
}
void CAudioBridge::UpdateSpeakerList(CSegment* pParam)
{
	CMedString str = "CAudioBridge::UpdateSpeakerList - ";
	m_seg.Clear();
	PartyRsrcID dwPartySpeakerId;
	for (int i = 0; i < MAX_ACTIVE_SPEAKER_LIST; i++)
	{
		*pParam >> dwPartySpeakerId;
		m_seg << (DWORD)dwPartySpeakerId;	// gets PartyRsrcId of current speakers
		str   << (DWORD)dwPartySpeakerId << "  ";
	}
	PTRACE(eLevelInfoNormal, str.GetString());
	m_pConfApi->UpdateDB((CTaskApp*)0xffff, UPDATE_ACTIVE_SPEAKERS_LIST, (DWORD)0, 0, &m_seg);
}

//--------------------------------------------------------------------------
void CAudioBridge::SendSequenceNumIndicationToCAM(OPCODE opcode, DWORD seqNumToken, DWORD sequenceNum)
{
	CSegment* pSeg = new CSegment;
	*pSeg << opcode << seqNumToken << sequenceNum;
	m_pConfApi->IvrConfNotification(SEQUENCE_NUM_IND, pSeg);
	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
// Function for API request
void CAudioBridge::OnConfUpdateAudioVolumeCONNECTED(CSegment* pParam)
{
	char targetPartyName[H243_NAME_LEN];

	*pParam >> targetPartyName;
	targetPartyName[H243_NAME_LEN-1] = '\0';

	EMediaDirection eMediaDirection = eNoDirection;
	DWORD newAudioVolume;

	*pParam >> (BYTE&)eMediaDirection >> newAudioVolume;

	TRACEINTO
		<< "ConfName:"         << m_pConfName
		<< ", PartyName:"      << targetPartyName
		<< ", MediaDirection:" << eMediaDirection
		<< ", AudioVolume:"    << newAudioVolume;

	CAudioBridgePartyCntl* pCurrParty = (CAudioBridgePartyCntl*)GetPartyCntl(targetPartyName);
	PASSERT_AND_RETURN(!pCurrParty);

	pCurrParty->UpdateAudioVolume(eMediaDirection, newAudioVolume);
}

//--------------------------------------------------------------------------
// Function for API OR Internal request
void CAudioBridge::OnConfUpdateAudioMuteCONNECTED(CSegment* pParam)
{
	WORD srcReq = 0;
	EOnOff eOnOff = eOff;
	EMediaDirection eMediaDirection = eNoDirection;
	CAudioBridgePartyCntl* pPartyCntl = NULL;

	*pParam >> srcReq;

	if (srcReq == OPERATOR)
	{
		char name[H243_NAME_LEN];
		*pParam >> name >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
		name[H243_NAME_LEN-1] = '\0';
		pPartyCntl = (CAudioBridgePartyCntl*)GetPartyCntl(name);

		TRACEINTO << "ConfName:" << m_pConfName << ", PartyName:" << name << ", MediaDirection:" << eMediaDirection << ", OnOff:" << (int)eOnOff;
	}
	else
	{
		PartyRsrcID partyRsrcID = INVALID;
		*pParam >> partyRsrcID >> (BYTE&)eOnOff >> (BYTE&)eMediaDirection;
		pPartyCntl = (CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID);
		if (srcReq == OPERATOR_REQ_BY_ID)
			srcReq = OPERATOR;

		TRACEINTO << "ConfName:" << m_pConfName << ", PartyId:" << partyRsrcID << ", MediaDirection:" << eMediaDirection << ", OnOff:" << (int)eOnOff;
}

	if (!CPObject::IsValidPObjectPtr(pPartyCntl))
	{
		TRACEINTO << "ConfName:" << m_pConfName << " - Failed, party not connected to AudioBridge";
		return;
	}

	pPartyCntl->UpdateMute(eMediaDirection, eOnOff, srcReq);
}

//--------------------------------------------------------------------------
// Function for API request
void CAudioBridge::OnConfUpdateAgcCONNECTED(CSegment* pParam)
{
	char targetPartyName[H243_NAME_LEN];
	EOnOff eOnOff = eOff;

	*pParam >> targetPartyName;
	targetPartyName[H243_NAME_LEN-1] = '\0';

	*pParam >> (BYTE&)eOnOff;

	TRACEINTO << "ConfName:" << m_pConfName << ", PartyName:" << targetPartyName << ", OnOff:" << (int)eOnOff;

	CAudioBridgePartyCntl* pCurrParty = (CAudioBridgePartyCntl*)GetPartyCntl(targetPartyName);
	PASSERT_AND_RETURN(!pCurrParty);

	pCurrParty->UpdateAGC(eOnOff);
}

//--------------------------------------------------------------------------
void CAudioBridge::SpeakerChangeIndication(CSegment* pSpeakerChangeParams, DWORD dwSpeakerChangeOpcode)
{
	DispatchEvent(dwSpeakerChangeOpcode, pSpeakerChangeParams);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfPlayMsgCONNECT(CSegment* pDataSeg)
{
	if (DUMMY_CONF_ID == m_confRsrcID)
	{
		PASSERTSTREAM_AND_RETURN(101, "ConfName:" << m_pConfName << " - confRsrcID is not set yet");
	}

	DWORD seqNumToken;
	*pDataSeg >> seqNumToken;

	DWORD sequenceNum = m_pAudioHardwareInterface->PlayMessage(pDataSeg);

	SendSequenceNumIndicationToCAM(IVR_PLAY_MESSAGE_REQ, seqNumToken, sequenceNum);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopPlayMsgCONNECT(CSegment* pDataSeg)
{
	OnConfStopPlayMsg(pDataSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopPlayMsgDISCONNECTING(CSegment* pDataSeg)
{
	OnConfStopPlayMsg(pDataSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopPlayMsg(CSegment* pDataSeg)
{
	if (DUMMY_CONF_ID == m_confRsrcID)
	{
		PASSERTSTREAM_AND_RETURN(101, "ConfName:" << m_pConfName << " - confRsrcID is not set yet");
	}

	DWORD seqNumToken;
	*pDataSeg >> seqNumToken;

	DWORD sequenceNum = m_pAudioHardwareInterface->StopPlayMessage(pDataSeg);

	SendSequenceNumIndicationToCAM(IVR_STOP_PLAY_MESSAGE_REQ, seqNumToken, sequenceNum);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStartIvrSeqCONNECT(CSegment* pDataSeg)
{
	if (DUMMY_CONF_ID == m_confRsrcID)
	{
		PASSERTSTREAM_AND_RETURN(101, "ConfName:" << m_pConfName << " - confRsrcID is not set yet");
	}

	DWORD seqNumToken;
	*pDataSeg >> seqNumToken;

	DWORD sequenceNum = m_pAudioHardwareInterface->StartIVR(pDataSeg);

	SendSequenceNumIndicationToCAM(IVR_START_IVR_REQ, seqNumToken, sequenceNum);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopIvrSeqCONNECT(CSegment* pDataSeg)
{
	OnConfStopIvrSeq(pDataSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopIvrSeqDISCONNECTING(CSegment* pDataSeg)
{
	OnConfStopIvrSeq(pDataSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopIvrSeq(CSegment* pDataSeg)
{
	if (DUMMY_CONF_ID == m_confRsrcID)
	{
		PASSERTSTREAM_AND_RETURN(101, "ConfName:" << m_pConfName << " - confRsrcID is not set yet");
	}

	DWORD seqNumToken;
	*pDataSeg >> seqNumToken;

	DWORD sequenceNum = m_pAudioHardwareInterface->StopIVR(pDataSeg);

	SendSequenceNumIndicationToCAM(IVR_STOP_IVR_REQ, seqNumToken, sequenceNum);
}
//--------------------------------------------------------------------------
void CAudioBridge::OnConfPlayMusicCONNECT(CSegment* pDataSeg)
{
	if (DUMMY_CONF_ID == m_confRsrcID)
	{
		PASSERTSTREAM_AND_RETURN(101, "ConfName:" << m_pConfName << " - confRsrcID is not set yet");
	}

	DWORD seqNumToken;
	*pDataSeg >> seqNumToken;

	DWORD sequenceNum = m_pAudioHardwareInterface->PlayMusic(pDataSeg);

	SendSequenceNumIndicationToCAM(IVR_PLAY_MUSIC_REQ, seqNumToken, sequenceNum);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopPlayMusicCONNECT(CSegment* pDataSeg)
{
	OnConfStopPlayMusic(pDataSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopPlayMusicDISCONNECTING(CSegment* pDataSeg)
{
	OnConfStopPlayMusic(pDataSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::OnConfStopPlayMusic(CSegment* pDataSeg)
{
	if (DUMMY_CONF_ID == m_confRsrcID)
	{
		PASSERTSTREAM_AND_RETURN(101, "ConfName:" << m_pConfName << " - confRsrcID is not set yet");
	}

	DWORD seqNumToken;
	*pDataSeg >> seqNumToken;

	DWORD sequenceNum = m_pAudioHardwareInterface->StopPlayMusic(pDataSeg);

	SendSequenceNumIndicationToCAM(IVR_STOP_PLAY_MUSIC_REQ, seqNumToken, sequenceNum);
}

//--------------------------------------------------------------------------
void CAudioBridge::IvrPartyCommand(PartyRsrcID partyID, OPCODE opcode, DWORD seqNumToken, CSegment* pDataSeg)
{
	CAudioBridgePartyCntl* pAudioPartyCntl = (CAudioBridgePartyCntl*)(GetPartyCntl(partyID));

	if (!CPObject::IsValidPObjectPtr(pAudioPartyCntl))
	{
		PASSERTSTREAM_AND_RETURN(101, "ConfName:" << m_pConfName << ", PartyId:" << partyID << " - Invalid party id");
	}

	// temp patch to block ivr for parties
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	DBGPASSERT_AND_RETURN(!pCommConf);

	CConfParty* pConfParty   = pCommConf->GetCurrentParty(pAudioPartyCntl->GetName());
	DBGPASSERT_AND_RETURN(!pConfParty);

	if (!IsIvrForSVCEnabled() && pConfParty->GetPartyMediaType() == eSvcPartyType)
	{
		TRACEINTOFUNC << "TEMP TEMP TEMP IVR is blocked for svc parties!!! (To Do: move the block to CAM!)";

		return;
	}

	pAudioPartyCntl->IvrCommand(opcode, seqNumToken, pDataSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::IvrConfCommand(OPCODE opcode, DWORD seqNumToken, CSegment* pDataSeg)
{
	// temp patch to block ivr for parties
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	DBGPASSERT_AND_RETURN(!pCommConf);

	eConfMediaType confMeadiaType = (eConfMediaType)pCommConf->GetConfMediaType();
	if (!IsIvrForSVCEnabled() )
	{
		switch (confMeadiaType)
		{
		case eSvcOnly:
		case eMixAvcSvc:
		case eMixAvcSvcVsw:
		{
			TRACEINTOFUNC << "TEMP TEMP TEMP IVR is blocked for conf with svc parties!!! confMeadiaType = " << confMeadiaType << " (To Do: move the block to CAM!)";
			return;
		}
		default:
			break;
		}

	}
	CSegment* pSeg = new CSegment;
	*pSeg << (DWORD)seqNumToken
	      << *pDataSeg;

	DispatchEvent(opcode, pSeg);

	POBJDELETE(pSeg);
}

//--------------------------------------------------------------------------
void CAudioBridge::IvrUpdatePartyStandalone(PartyRsrcID partyID, BOOL isStandalone)
{
	CAudioBridgePartyCntl* pAudioPartyCntl = (CAudioBridgePartyCntl*)(GetPartyCntl(partyID));

	if (!CPObject::IsValidPObjectPtr(pAudioPartyCntl))
	{
		PASSERTSTREAM_AND_RETURN(101, "ConfName:" << m_pConfName << ", PartyId:" << partyID << " - Failed, invalid party");
	}

	pAudioPartyCntl->IvrUpdateStandalone(isStandalone);
}

//--------------------------------------------------------------------------
bool CAudioBridge::UpdateAudioDelay(PartyRsrcID partyID, TAudioUpdateCompressedAudioDelayReq* pstAudioDelay)
{
	CAudioBridgePartyCntl* pAudioBridgePartyCntl = (CAudioBridgePartyCntl*)(GetPartyCntl(partyID));
	if (pAudioBridgePartyCntl)
		return pAudioBridgePartyCntl->UpdateAudioDelay(pstAudioDelay);
	else
		return false;
}

//--------------------------------------------------------------------------
bool CAudioBridge::UpdateAudioRelayParamsIn(PartyRsrcID partyRsrcID, DWORD ssrc)
{
	CAudioBridgePartyCntl* pAudioBridgePartyCntl = (CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	if (pAudioBridgePartyCntl)
		return pAudioBridgePartyCntl->UpdateAudioRelayParamsIn(ssrc);

	return false;
}

//--------------------------------------------------------------------------
bool CAudioBridge::UpdateAudioRelayParamsOut(PartyRsrcID partyRsrcID, DWORD numOfSsrcIds, DWORD* ssrc_array, DWORD ivrSsrc)
{
	CAudioBridgePartyCntl* pAudioBridgePartyCntl = (CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	if (pAudioBridgePartyCntl)
		return pAudioBridgePartyCntl->UpdateAudioRelayParamsOut(numOfSsrcIds, ssrc_array ,ivrSsrc);

	return false;
}
//--------------------------------------------------------------------------
void CAudioBridge::UpgradeToMixAvcSvc(PartyRsrcID partyRsrcID)
{

	CAudioBridgePartyCntl* pAudioBridgePartyCntl = (CAudioBridgePartyCntl*)GetPartyCntl(partyRsrcID);
	if (pAudioBridgePartyCntl)
			return pAudioBridgePartyCntl->UpgradeToMixAvcSvc();
	else
	{

		ReplyUpgradeToMixAvcSvcUponError(partyRsrcID);
	}
}

//-----------------------------------------------------------------------------
void CAudioBridge::ReplyUpgradeToMixAvcSvcUponError(PartyRsrcID partyRsrcID)
{
    TRACEINTO << " Party not exists, PartyRsrcID:  " << (DWORD)partyRsrcID;
    DBGPASSERT(101);

   m_pConfApi->ReplayUpgradeSvcAvcTranslate( partyRsrcID, AUDIO, statInconsistent );
}
//-----------------------------------------------------------------------------
void CAudioBridge::OnVideoUpdateSeenImageCONNECTED(CSegment* pParam)
{
	 PartyRsrcID idOfPartyToUpdate = INVALID;
	 PartyRsrcID idOfSeenParty = INVALID;
	 DWORD audioUplinkSSRCOfSeenImage = INVALID;

	 CAudioBridgePartyCntl* pAudioBridgePartyCntlToUpdate = NULL;
	 CAudioBridgePartyCntl* pAudioBridgePartyCntlOfSeenImage = NULL;


	 *pParam >> idOfPartyToUpdate;
	 *pParam >> idOfSeenParty;
	 TRACEINTO <<" ID of party to update: " << idOfPartyToUpdate << ", Id of seen image: " << idOfSeenParty;
	 pAudioBridgePartyCntlToUpdate = (CAudioBridgePartyCntl*)GetPartyCntl(idOfPartyToUpdate);
	 if(idOfSeenParty!=INVALID)
	 {
		 pAudioBridgePartyCntlOfSeenImage = (CAudioBridgePartyCntl*)GetPartyCntl(idOfSeenParty);
		 if(pAudioBridgePartyCntlOfSeenImage)
		 {
			 audioUplinkSSRCOfSeenImage = pAudioBridgePartyCntlOfSeenImage->GetSSRCOfIn();
		 }
		 else
		 {
			 TRACEINTO << "can't find party with ID: " << idOfSeenParty;
		 }
	 }

	 if (pAudioBridgePartyCntlToUpdate)
	 {
		 pAudioBridgePartyCntlToUpdate->SendUpdateEncoderOnSeenImageSSRC(audioUplinkSSRCOfSeenImage);

	 }
	 else
	 {
		 if(!pAudioBridgePartyCntlToUpdate)
			 TRACEINTO <<"Didn't found party in AB idOfPartyToUpdate: " << idOfPartyToUpdate;

	 }

}
