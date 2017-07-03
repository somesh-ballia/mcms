#include "ExportIsdnPartyCntl.h"
#include "TraceStream.h"
#include "IpPartyControl.h"
#include "OpcodesMcmsInternal.h"
#include "BridgePartyExportParams.h"
#include "ConfAppMngrInterface.h"
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "OpcodesMcmsCommon.h"
#include "AudioBridgePartyCntl.h"
#include "BridgeMoveParams.h"
#include "PartyApi.h"
#include "ContentBridge.h"

const WORD REMOTE_BAS_CAPS = RMTCAP;

PBEGIN_MESSAGE_MAP(CExportIsdnPartyCntl)

	ONEVENT(PARTY_VIDEO_DISCONNECTED,   UPDATE_VIDEOBRDG_BEFORE_EXPORT, CExportIsdnPartyCntl::OnVidBrdgDisconnectUpdateState)
	ONEVENT(PCM_PARTY_STATE_CHANGED,    EXPORT_RESOURCE,                CExportIsdnPartyCntl::OnPartyPcmStateChangedExportRsrc)
	ONEVENT(START_PARTY_MOVE_RSRC_IND,  EXPORT_RESOURCE,                CExportIsdnPartyCntl::OnEndResourceAllocatorStartMove)
	ONEVENT(ACK_IND,                    EXPORT_MPL,                     CExportIsdnPartyCntl::OnMplApiMoveAck)
	ONEVENT(END_PARTY_MOVE_RSRC_IND,    EXPORT_RESOURSE_END,            CExportIsdnPartyCntl::OnResourceAllocatorEndMove)
	ONEVENT(PARTY_AUDIO_EXPORTED,       EXPORT_BRIDGES,                 CExportIsdnPartyCntl::OnAudioBridgeExported)
	ONEVENT(PARTY_VIDEO_EXPORTED,       EXPORT_BRIDGES,                 CExportIsdnPartyCntl::OnVideoBridgeExported)
	ONEVENT(PARTY_CONTENT_DISCONNECTED, EXPORT_BRIDGES,                 CExportIsdnPartyCntl::OnContentBridgeDisConnect)
	ONEVENT(PARTY_AUDIO_DISCONNECTED,   EXPORT_BRIDGES,                 CExportIsdnPartyCntl::OnAudBrdgDisconnect)
	ONEVENT(PARTY_VIDEO_DISCONNECTED,   EXPORT_BRIDGES,                 CExportIsdnPartyCntl::OnVidBrdgDisconnect)
        ONEVENT(PARTY_XCODE_DISCONNECTED,   EXPORT_BRIDGES,                 CExportIsdnPartyCntl::OnXCodeBrdgDisconnect)
	ONEVENT(PARTYEXPORT,                EXPORT_PARTY,                   CExportIsdnPartyCntl::OnPartyExport)
	ONEVENT(PARTY_VIDEO_OUT_UPDATED,    ANYCASE,                        CExportIsdnPartyCntl::NullActionFunction)
	ONEVENT(PCM_DISCONNECTPARTY_TOUT,   EXPORT_RESOURCE,                CExportIsdnPartyCntl::OnTimerPcmDisconnect)
	ONEVENT(EXPORT_RSRC_TOUT,           EXPORT_RESOURCE,                CExportIsdnPartyCntl::OnTimerRAStartMove)
	ONEVENT(EXPORT_MPL_TOUT,            EXPORT_MPL,                     CExportIsdnPartyCntl::OnTimerMPLExport)
	ONEVENT(END_EXPORT_RSRC_TOUT,       EXPORT_RESOURSE_END,            CExportIsdnPartyCntl::OnTimerRAEndMove)
	ONEVENT(EXPORT_BRIDGES_TOUT,        EXPORT_BRIDGES,                 CExportIsdnPartyCntl::OnTimerExportBridges)
	ONEVENT(EXPORT_FAILED_TOUT,         ANYCASE,                        CExportIsdnPartyCntl::OnTimerExportFailed)
	ONEVENT(EXPORT_PARTY_TOUT,          EXPORT_PARTY,                   CExportIsdnPartyCntl::OnTimerExport)
	ONEVENT(REMOTE_BAS_CAPS,            ANYCASE,                        CExportIsdnPartyCntl::OnRmtCapsAnycase)

PEND_MESSAGE_MAP(CExportIsdnPartyCntl, CIsdnPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CExportIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
CExportIsdnPartyCntl::CExportIsdnPartyCntl() : m_numOfActiveLogicalRsrc(0)
{
	for (WORD i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
	{
		m_activeLogicalRsrc[i] = 0;
		m_ackTable[i] = 0;
	}
}

//--------------------------------------------------------------------------
CExportIsdnPartyCntl::~CExportIsdnPartyCntl()
{
}

//--------------------------------------------------------------------------
CExportIsdnPartyCntl& CExportIsdnPartyCntl::operator=(const CExportIsdnPartyCntl& other)
{
	CIsdnPartyCntl::operator=(other);

	m_numOfActiveLogicalRsrc = other.m_numOfActiveLogicalRsrc;
	for (WORD i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
	{
		m_activeLogicalRsrc[i] = other.m_activeLogicalRsrc[i];
		m_ackTable[i] = other.m_ackTable[i];
	}

	return *this;
}

//--------------------------------------------------------------------------
void* CExportIsdnPartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
const char* CExportIsdnPartyCntl::NameOf() const
{
	return "CExportIsdnPartyCntl";
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::Transfer(COsQueue* pDestRcvMbx, void* pComConf, DWORD destConfId, DWORD destPartyId, EMoveType eCurMoveType)
{
	// 1) validation tests
	if (m_disconnectState == DISCONNECTED)       // disconnected party cannot be transfer !!!
	{
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return;
	}

	// define move type
	m_moveType = eCurMoveType;

	// set destination conf mail box
	if (m_pDestConfMbx)
		POBJDELETE(m_pDestConfMbx);

	m_pDestConfMbx       = new COsQueue(*pDestRcvMbx);
	m_pComConf           = pComConf;
	m_destMonitorConfId  = destConfId;
	m_destMonitorPartyId = destPartyId;

	// close content if needed
	BYTE bIsTokenHolder = m_pContentBridge->IsTokenHolder(m_pParty);
	if (bIsTokenHolder)
		m_pTaskApi->ContentTokenWithdraw();

	if (m_pContentBridge->IsTokenHeld() && (m_pTargetTransmitScm->IsContentOn()))
	{
		BYTE partyChanNum    = m_pTargetTransmitScm->GetNumB0Chnl();
		BYTE ISDNContentRate = CUnifiedComMode::TranslateAmcRateToAmscRate(AMC_0k);

		m_pTargetTransmitScm->SetContentModeContentRate(ISDNContentRate);
		m_pTargetTransmitScm->m_contentMode.CalculateSartEndDummyForH239(partyChanNum);

		// Reduce content rate to 0k without waiting
		ChangeContent();

		TRACEINTO << m_partyConfName << " - Update Bridge after change content rate to 0k";

		CBridgePartyVideoOutParams* outVideoParams = new CBridgePartyVideoOutParams;
		InitVideoParams(m_pTargetTransmitScm, outVideoParams);
		m_pVideoBridgeInterface->UpdateVideoOutParams(m_pParty, outVideoParams);
		POBJDELETE(outVideoParams);
	}

	if (UpdateVidBrdgStateBeforeExport() != TRUE)
	{
		Transfer();
	}
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::ChangeContent()
{
	TRACEINTO << m_partyConfName;

	CComMode TempTargetTransmitScm(*m_pCurrentTransmitScm);
	TempTargetTransmitScm.m_contentMode = m_pTargetTransmitScm->m_contentMode;

	TempTargetTransmitScm.UpdateH221string(1);
	TempTargetTransmitScm.Dump(1);

	m_pPartyApi->ChangeMode(&TempTargetTransmitScm);

	if (IsPartyH239() && m_pTargetTransmitScm->IsContentOn()) // H.239
	{
		if (m_pCurrentTransmitScm->GetContentMode() == 0)       // only on connection send AMC_C&I MCS
			m_pPartyApi->SendH239MCS(TempTargetTransmitScm.m_contentMode.GetControlID());

		if (m_pTargetTransmitScm->GetContentModeContentRate() != AMSC_0k) // send AMC CI Video Mode
		{
			m_pPartyApi->SendH239ContentVideoMode(&(TempTargetTransmitScm.m_contentMode));
		}
		else // send logical channel inactive
		{
			m_pPartyApi->SendContentTokenMediaProducerStatus(TempTargetTransmitScm.m_contentMode.GetControlID(), CHANNEL_INACTIVE);
		}
	}

	UpdateCurrComMode(&TempTargetTransmitScm);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnPartyPcmStateChangedExportRsrc(CSegment* pParam)
{
	CPartyCntl::OnPartyPcmStateChangedAnycase(pParam);
	if (!m_isPcmConnected)
	{
		if (IsValidTimer(PCM_DISCONNECTPARTY_TOUT))
		{
			TRACEINTO << m_partyConfName << " - PCM disconnection completed";
			DeleteTimer(PCM_DISCONNECTPARTY_TOUT);
		}
		Transfer();
	}
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnTimerPcmDisconnect(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << "PartyId:" << GetPartyRsrcId() << " - PCM did not completed its disconnection";

	DBGPASSERT(GetPartyRsrcId());
	OFF(m_isPcmConnected);

	Transfer();
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::Transfer()
{
	m_state = EXPORT_RESOURCE;
	if (m_isPcmConnected)
	{
		TRACEINTO << "Wait 5 more seconds for PCM disconnection";
		StartTimer(PCM_DISCONNECTPARTY_TOUT, 5*SECOND);
		return;
	}

	// 3)
	// A. Start moving the resources from 1 conf to the other
	// B. Start a timer for the first "start move" resource stage.

	// Here we will move the resources in RA
	PARTY_MOVE_RSRC_REQ_PARAMS_S* pReq = new PARTY_MOVE_RSRC_REQ_PARAMS_S;
	pReq->source_monitor_conf_id  = m_monitorConfId;
	pReq->target_monitor_conf_id  = m_destMonitorConfId;
	pReq->source_monitor_party_id = m_monitorPartyId;
	pReq->target_monitor_party_id = 0XFFFFFFFF;
	if (m_service_provider)
	{
		strncpy((char*)pReq->serviceName, m_service_provider, GENERAL_SERVICE_PROVIDER_NAME_LEN);
		pReq->serviceName[GENERAL_SERVICE_PROVIDER_NAME_LEN - 1] = '\0';
	}
	else
	{
		pReq->serviceName[0] = '\0';
	}

	TRACEINTO << m_partyConfName
		<< "\nSTART_PARTY_MOVE_RSRC_REQ:"
		<< "\n  source_monitor_conf_id  :" << pReq->source_monitor_conf_id
		<< "\n  target_monitor_conf_id  :" << pReq->target_monitor_conf_id
		<< "\n  source_monitor_party_id :" << pReq->source_monitor_party_id
		<< "\n  target_monitor_party_id :" << pReq->target_monitor_party_id
		<< "\n  service_name            :" << (char*)pReq->serviceName;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)(pReq), sizeof(PARTY_MOVE_RSRC_REQ_PARAMS_S));
	SendReqToResourceAllocator(pSeg, START_PARTY_MOVE_RSRC_REQ);
	PDELETE(pReq);

	StartTimer(EXPORT_RSRC_TOUT, 5*SECOND);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnEndResourceAllocatorStartMove(CSegment* pParam)
{
	// Here we check if status OK and if so we start the MPL_API part
	// Moving the physical resources (MFA).
	DeleteTimer(EXPORT_RSRC_TOUT);

	DWORD status, target_monitor_conf_id, target_conf_id, source_monitor_party_id, source_party_id;
	*pParam >> status >> target_monitor_conf_id >> target_conf_id >> source_monitor_party_id >> source_party_id;

	TRACEINTO << m_partyConfName
		<< "\n  target_conf_id          :" << target_conf_id
		<< "\n  target_monitor_conf_id  :" << target_monitor_conf_id
		<< "\n  source_monitor_party_id :" << source_monitor_party_id
		<< "\n  source_party_id         :" << source_party_id
		<< "\n  status                  :" << status;

	if (STATUS_OK != TestReturnedParams(status, target_monitor_conf_id, target_conf_id, source_monitor_party_id, source_party_id, m_monitorPartyId))
		return;

	m_destResourceConfId = target_conf_id;
	m_state = EXPORT_MPL;

	// Here we send the request to the MPL_API(MFA)
	// But first we will find which logical resources are open by asking the bridges and filling the array.
	BOOL isEncOpen = FALSE;
	BOOL isDecOpen = FALSE;
	m_numOfActiveLogicalRsrc = 0;

	// Audio Enc/Dec represented as one message to the CM (MOVE_ART)
	// check if audio encoder and decoder are open
	m_pAudioInterface->ArePortsOpened((const CParty*)m_pParty, isEncOpen, isDecOpen);
	if (isEncOpen != FALSE)
	{
		m_activeLogicalRsrc[eLogical_audio_encoder] = 1; // Wait for ACK
		m_numOfActiveLogicalRsrc++;
	}

	if (isDecOpen)
	{
		m_activeLogicalRsrc[eLogical_audio_decoder] = 1; // Wait for ACK
		m_numOfActiveLogicalRsrc++;
	}

	// If there is an audio logical rsrc open - we open the RTP too
	if (m_numOfActiveLogicalRsrc && !m_voice) // audio logical rsrc is open
	{
		m_activeLogicalRsrc[eLogical_mux] = 1;
		m_numOfActiveLogicalRsrc++;
	}

	// Video check
	isEncOpen = FALSE;
	isDecOpen = FALSE;
	m_pVideoBridgeInterface->ArePortsOpened((const CParty*)m_pParty, isEncOpen, isDecOpen);
	if (isEncOpen != FALSE)
	{
		m_activeLogicalRsrc[eLogical_video_encoder] = 1;
		m_numOfActiveLogicalRsrc++;
	}

	if (isDecOpen)
	{
		m_activeLogicalRsrc[eLogical_video_decoder] = 1;
		m_numOfActiveLogicalRsrc++;
	}

	MOVE_RESOURCES_REQ_S* pMoveRsrcReq = new MOVE_RESOURCES_REQ_S;

	// fill MOVE_RESOURCES_REQ_S struct with resources state
	for (WORD i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
	{
		// Set resource as not active
		pMoveRsrcReq->openLogicalResources[i] = eRsrcNonActive;
		if (m_activeLogicalRsrc[i] == 1)
		{
			pMoveRsrcReq->openLogicalResources[i] = eRsrcActive; // Active
		}
	}

	// Fill the rest of the params
	pMoveRsrcReq->moveRsrcParams.confType  = 0XFFFFFFFF;
	pMoveRsrcReq->moveRsrcParams.newConfId = m_destResourceConfId;
	ESampleRate eSampleRate;
	if (FALSE == CAudioHardwareInterface::TranslateMcmsSampleRateToApiValues(GetConfAudioSampleRate(), eSampleRate))
		PASSERTMSG(GetConfAudioSampleRate()+1, "CExportIsdnPartyCntl::OnEndResourceAllocatorStartMove - invalid sample rate!");

	pMoveRsrcReq->moveRsrcParams.confAudioSampleRate = eSampleRate;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_destMonitorConfId);
	if (pCommConf)
		pMoveRsrcReq->moveRsrcParams.enConfSpeakerChangeMode = pCommConf->GetConfSpeakerChangeMode();
	else
		pMoveRsrcReq->moveRsrcParams.enConfSpeakerChangeMode = E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;

	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(pMoveRsrcReq), sizeof(MOVE_RESOURCES_REQ_S));
	m_pPartyHWInterface->SendMsgToMPL(MOVE_PARTY_RESOURCE_REQ, pMsg);

	StartTimer(EXPORT_MPL_TOUT, 10*SECOND);

	POBJDELETE(pMoveRsrcReq);
	POBJDELETE(pMsg);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnMplApiMoveAck(CSegment* pParam)
{
	BYTE lrt;
	ACK_IND_S sAckIndStrcut;
	pParam->Get((BYTE*)&sAckIndStrcut, sizeof(ACK_IND_S));
	*pParam >> lrt;

	TRACEINTO << m_partyConfName << ", LogicalResourceType:"<< (WORD)lrt << ", Reason:" << sAckIndStrcut.ack_base.reason;

	if (sAckIndStrcut.ack_base.status != STATUS_OK)
	{
		PASSERTSTREAM(1, "status:" << sAckIndStrcut.ack_base.status);

		// TOOREN - is it needed
		m_isFaulty = 1;
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return;
	}

	WORD status = STATUS_OK;

	switch (sAckIndStrcut.ack_base.ack_opcode)
	{
		case MOVE_PARTY_RESOURCE_REQ:
		{
			status = CheckIfLogicalRsrcAckAccepted((eLogicalResourceTypes)lrt);
			break;
		}

		default:
		{
			DBGPASSERT(sAckIndStrcut.ack_base.ack_opcode);
			break;
		}
	} // switch

	if (status != STATUS_OK)
	{
		PASSERTSTREAM(1, "status:" << status);
		// TBD - What are we doing in that case - disconnect the party ?
		m_isFaulty = 1;
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return;
	}

	if (m_numOfActiveLogicalRsrc == 0)
	{
		// This means we got all relevant ACKs from the MFA and we may continue the flow.
		MfaAcksCompleted();
	}
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::MfaAcksCompleted()
{
	DeleteTimer(EXPORT_MPL_TOUT);

	m_state = EXPORT_RESOURSE_END;
	StartTimer(END_EXPORT_RSRC_TOUT, 8*SECOND);

	// Here we will move the resources in RA
	PARTY_MOVE_RSRC_REQ_PARAMS_S* pReq = new PARTY_MOVE_RSRC_REQ_PARAMS_S;
	pReq->source_monitor_conf_id  = m_monitorConfId;
	pReq->target_monitor_conf_id  = m_destMonitorConfId;
	pReq->source_monitor_party_id = m_monitorPartyId;
	pReq->target_monitor_party_id = m_destMonitorPartyId; // TBD 0xfffffff ?

	TRACEINTO << m_partyConfName
		<< "\nEND_PARTY_MOVE_RSRC_REQ:"
		<< "\n  source_monitor_conf_id  :" << pReq->source_monitor_conf_id
		<< "\n  target_monitor_conf_id  :" << pReq->target_monitor_conf_id
		<< "\n  source_monitor_party_id :" << pReq->source_monitor_party_id
		<< "\n  target_monitor_party_id :" << pReq->target_monitor_party_id;

	CSegment* pSeg = new CSegment;
	pSeg->Put((BYTE*)(pReq), sizeof(PARTY_MOVE_RSRC_REQ_PARAMS_S));
	SendReqToResourceAllocator(pSeg, END_PARTY_MOVE_RSRC_REQ);
	PDELETE(pReq);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnResourceAllocatorEndMove(CSegment* pParam)
{
	DeleteTimer(END_EXPORT_RSRC_TOUT);
	DeleteTimer(EXPORT_RSRC_TOUT);

	DWORD status, target_monitor_conf_id, target_conf_id, target_monitor_party_id, target_party_id;
	*pParam >> status >> target_monitor_conf_id >> target_conf_id >> target_monitor_party_id >> target_party_id;

	TRACEINTO << m_partyConfName
		<< "\n  target_conf_id          :" << target_conf_id
		<< "\n  target_monitor_conf_id  :" << target_monitor_conf_id
		<< "\n  target_party_id         :" << target_party_id
		<< "\n  target_monitor_party_id :" << target_monitor_party_id
		<< "\n  status                  :" << status;

	if (STATUS_OK != TestReturnedParams(status, target_monitor_conf_id, target_conf_id, target_monitor_party_id, target_party_id, m_destMonitorPartyId))
	{
		m_isFaulty = 1;
		// Disconnect the party
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return;
	}

	m_state = EXPORT_BRIDGES;
	// Here we start the disconnection from the A/V bridges
	if (m_pAudioInterface && IsAtLeastOneDirectionConnectedToAudioBridge())
	{
		CBridgePartyExportParams bridgePartyExportParams(m_pParty->GetPartyId());
		m_pConfAppMngrInterface->ExportPartyAudio(&bridgePartyExportParams);
	}

	// TOOREN -- check following lines until ON(m_isPartyEndRAMoveOK)...
	// for voice party
	if (!m_voice)
	{
		// disconnect video
		if (m_pVideoBridgeInterface && IsAtLeastOneDirectionConnectedToVideoBridge())
		{
			CBridgePartyExportParams bridgePartyExportParams(m_pParty->GetPartyId());
			m_pConfAppMngrInterface->ExportPartyVideo(&bridgePartyExportParams);
		}
	}

	if (m_isFeccConn)
		DisconnectPartyFromFECCBridge();

	if (m_isContentConn)
		DisconnectPartyFromContentBridge();

	// remove from terminal list
	if (m_pTerminalNumberingManager && m_isTerminalNumberingConn)
	{
		STATUS removeStatus = STATUS_OK;
		removeStatus = m_pTerminalNumberingManager->Remove(m_pParty);
		if (removeStatus != STATUS_OK)
			PASSERT(removeStatus);

		OFF(m_isTerminalNumberingConn);
	}

	ON(m_isPartyEndRAMoveOK);

	StartTimer(EXPORT_BRIDGES_TOUT, 12*SECOND);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnAudioBridgeExported(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CAudioBridgePartyCntl* ptr = 0;
	WORD status;
	BYTE isParams;
	*pParam >> status >> isParams;

	if (status != STATUS_OK)
	{
		PASSERT(status);
		// TOOREN - check if needed
		SetDataForExportPartyFail();
		StartTimer(EXPORT_FAILED_TOUT, SECOND);
		return;
	}

	*pParam >> (void*&)ptr;

	m_pBridgeMoveParams->SetAudioBridgePartyCntlOnExport(ptr);

	m_eAudBridgeConnState = eBridgeDisconnected;
	OFF(m_bIsMemberInAudBridge);

	m_pTaskApi->UpdateDB(m_pParty, AUDCON, FALSE);
	BridgeExportCompleted();
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnVideoBridgeExported(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CVideoBridgePartyCntl* ptr;
	WORD status;
	BYTE isParams;
	*pParam >> status >> isParams;
	if (status != STATUS_OK)
	{
		PASSERT(status);
		SetDataForExportPartyFail();
		StartTimer(EXPORT_FAILED_TOUT, SECOND);
		return;
	}

	*pParam >> (void*&)ptr;

	m_pBridgeMoveParams->SetVideoBridgePartyCntlOnExport(ptr);

	m_eVidBridgeConnState = eBridgeDisconnected;
	OFF(m_bIsMemberInVidBridge);

	m_pTaskApi->UpdateDB(m_pParty, VIDCON, FALSE);
	BridgeExportCompleted();
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnContentBridgeDisConnect(CSegment* pParam)
{
	CPartyCntl::OnContentBrdgDisconnected(pParam);
	BridgeExportCompleted();
}
/////////////////////////////////////////////////////////////
void  CExportIsdnPartyCntl::OnXCodeBrdgDisconnect(CSegment* pParam)
{
	CPartyCntl::OnXCodeBrdgDisconnected(pParam);
	BridgeExportCompleted();
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::BridgeExportCompleted()
{
          if ( !m_bIsMemberInAudBridge && (m_voice || (!m_bIsMemberInVidBridge && !m_isFeccConn && !m_isContentConn && !m_isXCodeConn) ) )
	{
		TRACEINTO << m_partyConfName << " - Export party";


		DeleteTimer(EXPORT_BRIDGES_TOUT);

		// validation test for party api
		if (!CPObject::IsValidPObjectPtr(m_pPartyApi))
		{
			DBGPASSERT(GetPartyRsrcId());
			SetDataForExportPartyFail();
			StartTimer(EXPORT_FAILED_TOUT, SECOND);
			return;
		}

		m_state = EXPORT_PARTY;

		// send export to party
		StartTimer(EXPORT_PARTY_TOUT, 30*SECOND);
		m_pBridgeMoveParams->UnregisterBridgePartyCntlsInTask();
		m_pPartyApi->Export(m_pDestConfMbx, this, m_pComConf, m_moveType);
	}
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnAudBrdgDisconnect(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CExportIsdnPartyCntl::OnAudBrdgDisconnect");
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnVidBrdgDisconnect(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CExportIsdnPartyCntl::OnVidBrdgDisconnect");
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnPartyExport(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	DeleteTimer(EXPORT_PARTY_TOUT);
	WORD status;
	*pParam >> status;
	if (!status) // export was o.k.
	{
		m_pPartyApi->DestroyOnlyApi();
		POBJDELETE(m_pPartyApi);
	}

	// TOOREN - check if right behavior (RA)
	if (status == statIllegal)
	{
		// In this case we will switch the conf id's since the party is already registered in the destination conf.
		SetDataForExportPartyFail();
		StartTimer(EXPORT_FAILED_TOUT, SECOND);
	}
	else
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), status);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnRmtCapsAnycase(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Not handled");
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnTimerRAStartMove(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Failed to start move in RA");

	// Disconnect the party
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnTimerMPLExport(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Failed to receive IND from MFA");

	// Disconnect the party
	m_isFaulty = 1;
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnTimerRAEndMove(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Failed to receive end move from RA");

	// Disconnect the party
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnTimerExportBridges(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Failed to receive IND from bridges");

	// Disconnect the party
	m_isFaulty = 1;
	SetDataForExportPartyFail();
	StartTimer(EXPORT_FAILED_TOUT, SECOND);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnTimerExport(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << m_partyConfName;

	// TOOREN - is it right ?
	SetDataForExportPartyFail();
	StartTimer(EXPORT_FAILED_TOUT, SECOND);
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnTimerExportFailed(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << m_partyConfName;

	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
WORD CExportIsdnPartyCntl::CheckIfLogicalRsrcAckAccepted(eLogicalResourceTypes lrt)
{
	// Checking if the correct ACK was received -
	// If so, decrease the number of active logical resource counter by one.
	TRACEINTO << m_partyConfName << ", LogicalResourceType:" << (WORD)lrt;

	// If it is a party resource and we did not get it already
	if (m_activeLogicalRsrc[lrt] == 1 && 0 == m_ackTable[lrt])
	{
		m_ackTable[lrt] = 1;
		m_numOfActiveLogicalRsrc--;
		return STATUS_OK;
	}
	else if (m_activeLogicalRsrc[lrt] == 1 && 1 == m_ackTable[lrt])
	{
		// for some reason we got double ack for same resource:
		// return ok but m_numOfActiveLogicalRsrc is not decreased.
		return STATUS_OK;
	}
	else
	{
		TRACESTRFUNC(eLevelError) << "LogicalResourceType:"<< (WORD)lrt << " - Failed, invalid type";
		return STATUS_FAIL;
	}
}

//--------------------------------------------------------------------------
STATUS CExportIsdnPartyCntl::TestReturnedParams(DWORD status, DWORD target_monitor_conf_id, DWORD target_conf_id,
                                                DWORD monitor_party_id, DWORD party_id, DWORD expected_monitor_party_id)
{
	if (status != STATUS_OK)
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, m_partyConfName << ", status:" << status, status);
	}

	if (monitor_party_id != expected_monitor_party_id)
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, m_partyConfName << ", monitor_party_id:" << monitor_party_id << ", expected_monitor_party_id:" << expected_monitor_party_id, STATUS_FAIL);
	}

	if (party_id != m_pPartyAllocatedRsrc->GetPartyRsrcId())
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, m_partyConfName << ", party_id:" << party_id << ", expected_party_id:" << m_pPartyAllocatedRsrc->GetPartyRsrcId(), STATUS_FAIL);
	}

	if (target_monitor_conf_id != m_destMonitorConfId)
	{
		PASSERTSTREAM_AND_RETURN_VALUE(1, m_partyConfName << ", target_monitor_conf_id:" << target_monitor_conf_id << ", expected_target_monitor_conf_id:" << m_destMonitorConfId, STATUS_FAIL);
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
BYTE CExportIsdnPartyCntl::UpdateVidBrdgStateBeforeExport()
{
	if (!m_pCurrentReceiveScm->VideoOn() && ((m_eVidBridgeConnState & eInConnected) == eInConnected))
	{
		TRACEINTO << "Remote video mode is Video_Off but m_eVidBridgeConnState is InConnected, disconnect party from video in";

		m_state = UPDATE_VIDEOBRDG_BEFORE_EXPORT;

		return DisconnectPartyFromVideoIn();
	}
	return FALSE;
}

//--------------------------------------------------------------------------
void CExportIsdnPartyCntl::OnVidBrdgDisconnectUpdateState(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	DeleteTimer(CHANGETOUT);

	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);

	if (resStat == statVideoInOutResourceProblem)
	{
		BYTE mipHwConn    = (BYTE)eMipBridge;
		BYTE mipMedia     = (BYTE)eMipVideo;
		BYTE mipDirect    = 0;
		BYTE mipTimerStat = 0;
		BYTE mipAction    = 0;

		*pParam >>  mipDirect >> mipTimerStat >> mipAction;

		CSegment* pSeg = new CSegment;
		*pSeg << mipHwConn << mipMedia << mipDirect << mipTimerStat << mipAction;
		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM, m_pParty, NULL, MpiErrorNumber);
		POBJDELETE(pSeg);
		return;
	}
	else if (resStat == statOK)
	{
		Transfer();
	}
}

