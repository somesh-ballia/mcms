#include "ExportIsdnVoicePartyCntl.h"
#include "TraceStream.h"
#include "IpPartyControl.h"
#include "OpcodesMcmsInternal.h"
#include "BridgePartyExportParams.h"
#include "ConfAppMngrInterface.h"
#include "AudioBridgeInterface.h"
#include "OpcodesMcmsCommon.h"
#include "AudioBridgePartyCntl.h"
#include "BridgeMoveParams.h"
#include "PartyApi.h"


PBEGIN_MESSAGE_MAP(CExportIsdnVoicePartyCntl)

	ONEVENT(PARTY_AUDIO_EXPORTED,      EXPORT_BRIDGES,      CExportIsdnVoicePartyCntl::OnAudioBridgeExported)
	ONEVENT(PARTYEXPORT,               EXPORT_PARTY,        CExportIsdnVoicePartyCntl::OnPartyExport)
	ONEVENT(START_PARTY_MOVE_RSRC_IND, EXPORT_RESOURCE,     CExportIsdnVoicePartyCntl::OnEndResourceAllocatorStartMove)
	ONEVENT(ACK_IND,                   EXPORT_MPL,          CExportIsdnVoicePartyCntl::OnMplApiMoveAck)
	ONEVENT(END_PARTY_MOVE_RSRC_IND,   EXPORT_RESOURSE_END, CExportIsdnVoicePartyCntl::OnResourceAllocatorEndMove)
	ONEVENT(EXPORT_PARTY_TOUT,         EXPORT_PARTY,        CExportIsdnVoicePartyCntl::OnTimerExport)
	ONEVENT(EXPORT_RSRC_TOUT,          EXPORT_RESOURCE,     CExportIsdnVoicePartyCntl::OnTimerRAStartMove)
	ONEVENT(EXPORT_MPL_TOUT,           EXPORT_MPL,          CExportIsdnVoicePartyCntl::OnTimerMPLExport)
	ONEVENT(END_EXPORT_RSRC_TOUT,      EXPORT_RESOURSE_END, CExportIsdnVoicePartyCntl::OnTimerRAEndMove)
	ONEVENT(EXPORT_BRIDGES_TOUT,       EXPORT_BRIDGES,      CExportIsdnVoicePartyCntl::OnTimerExportBridges)

PEND_MESSAGE_MAP(CExportIsdnVoicePartyCntl, CIsdnPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CExportIsdnVoicePartyCntl
////////////////////////////////////////////////////////////////////////////
CExportIsdnVoicePartyCntl::CExportIsdnVoicePartyCntl()
	: m_numOfActiveLogicalRsrc(0)
{
	for (WORD i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
	{
		m_ackTable[eLogicalResourceTypes(i)] = false;
		m_activeLogicalRsrc[i] = 0;
	}
}

//--------------------------------------------------------------------------
CExportIsdnVoicePartyCntl::~CExportIsdnVoicePartyCntl()
{
}

//--------------------------------------------------------------------------
void* CExportIsdnVoicePartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
const char* CExportIsdnVoicePartyCntl::NameOf()  const
{
	return "CExportIsdnVoicePartyCntl";
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::Transfer(COsQueue* pDestRcvMbx, void* pComConf, DWORD destConfId, DWORD destPartyId, EMoveType eCurMoveType)
{
	// 1) validation tests
	if (m_disconnectState == DISCONNECTED)       // disconnected party cannot be transfer !!!
	{
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return;
	}

	m_state = EXPORT_RESOURCE;

	// define move type
	m_moveType = eCurMoveType;
	// set destination conf mail box
	if (m_pDestConfMbx)
		POBJDELETE(m_pDestConfMbx);

	m_pDestConfMbx = new COsQueue(*pDestRcvMbx);

	m_pComConf           = pComConf;
	m_destMonitorConfId  = destConfId;
	m_destMonitorPartyId = destPartyId;

	// 3)
	// A. Start moving the resources from 1 conf to the other
	// B. Start a timer for the first "start move" resource stage.

	// Here we will move the resources in RA
	PARTY_MOVE_RSRC_REQ_PARAMS_S* pReq = new PARTY_MOVE_RSRC_REQ_PARAMS_S;
	pReq->source_monitor_conf_id  = m_monitorConfId;
	pReq->target_monitor_conf_id  = m_destMonitorConfId;
	pReq->source_monitor_party_id = m_monitorPartyId;
	pReq->target_monitor_party_id = 0XFFFFFFFF;  // m_destPartyId; // TBD 0xfffffff ?
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
void CExportIsdnVoicePartyCntl::OnResourceAllocatorEndMove(CSegment* pParam)
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
		return;
	}

	m_state = EXPORT_BRIDGES;
	// Here we start the disconnection from the A/V bridges
	if (m_pAudioInterface && IsAtLeastOneDirectionConnectedToAudioBridge())
	{
		CBridgePartyExportParams bridgePartyExportParams(m_pParty->GetPartyId());
		m_pConfAppMngrInterface->ExportPartyAudio(&bridgePartyExportParams);
	}

	ON(m_isPartyEndRAMoveOK);
	StartTimer(EXPORT_BRIDGES_TOUT, 12*SECOND);
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnTimerRAStartMove(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CExportIsdnVoicePartyCntl::OnTimerRAStartMove - Failed to start move in RA : Name - ", m_partyConfName);
	PASSERT(101);
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnTimerExportBridges(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Failed to start move in RA");

	// Disconnect the party
	m_isFaulty = 1;
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnEndResourceAllocatorStartMove(CSegment* pParam)
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
	MOVE_RESOURCES_REQ_S* pMoveRsrcReq = new MOVE_RESOURCES_REQ_S;

	// Set all resources as not active
	for (WORD i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
		pMoveRsrcReq->openLogicalResources[i] = eRsrcNonActive;

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

	// RTM Resource
	// m_activeLogicalRsrc[eLogical_net] = 1; //Wait for ACK
	// !!!!!- Just for the integration do not wait for RTM ack
	// m_numOfActiveLogicalRsrc++;
	// !!!!!!

	// Audio Enc/Dec represented as one message to the CM (MOVE_ART)
//	m_activeLogicalRsrc[eLogical_rtp] = 1; // Wait for ACK
//	m_numOfActiveLogicalRsrc++;
	TRACEINTO << "numOfActiveLogicalRsrc:" << m_numOfActiveLogicalRsrc;

	for (WORD i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
	{
		if (m_activeLogicalRsrc[i] == 1)
		{
			pMoveRsrcReq->openLogicalResources[i] = eRsrcActive; // Active
		}
	}

	// Filll the rest of the params
	pMoveRsrcReq->moveRsrcParams.confType  = 0XFFFFFFFF;
	pMoveRsrcReq->moveRsrcParams.newConfId = m_destResourceConfId;
	ESampleRate eSampleRate;
	if (FALSE == CAudioHardwareInterface::TranslateMcmsSampleRateToApiValues(GetConfAudioSampleRate(), eSampleRate))
		PASSERTMSG(GetConfAudioSampleRate()+1, "CExportIsdnVoicePartyCntl::OnEndResourceAllocatorStartMove - invalid sample rate!");

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
STATUS CExportIsdnVoicePartyCntl::TestReturnedParams(DWORD status, DWORD target_monitor_conf_id,
                                                     DWORD target_conf_id,
                                                     DWORD monitor_party_id,
                                                     DWORD party_id,
                                                     DWORD expected_monitor_party_id)
{
	if (status != STATUS_OK)
	{
		PASSERTSTREAM(1, m_partyConfName << ", status:" << status);

		// Disconnect the party
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return status;
	}

	if (monitor_party_id != expected_monitor_party_id)
	{
		PASSERTSTREAM(1, m_partyConfName << ", monitor_party_id:" << monitor_party_id << ", expected_monitor_party_id:" << expected_monitor_party_id);

		// Disconnect the party
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return STATUS_FAIL;
	}

	if (party_id != m_pPartyAllocatedRsrc->GetPartyRsrcId())
	{
		PASSERTSTREAM(1, m_partyConfName << ", party_id:" << party_id << ", expected_party_id:" << m_pPartyAllocatedRsrc->GetPartyRsrcId());

		// Disconnect the party
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return STATUS_FAIL;
	}

	if (target_monitor_conf_id != m_destMonitorConfId)
	{
		PASSERTSTREAM(1, m_partyConfName << ", target_monitor_conf_id:" << target_monitor_conf_id << ", expected_target_monitor_conf_id:" << m_destMonitorConfId);

		// Disconnect the party
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
		return STATUS_FAIL;
	}

	return STATUS_OK;
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnTimerMPLExport(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Failed to receive IND from MFA");

	// Disconnect the party
	m_isFaulty = 1;
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnMplApiMoveAck(CSegment* pParam)
{
	BYTE lrt;
	ACK_IND_S sAckIndStrcut;
	pParam->Get((BYTE*)&sAckIndStrcut, sizeof(ACK_IND_S));
	*pParam >> lrt;

	TRACEINTO << m_partyConfName << ", LogicalResourceType:"<< (WORD)lrt << ", Reason:" << sAckIndStrcut.ack_base.reason;

	if (sAckIndStrcut.ack_base.status != STATUS_OK)
	{
		PASSERTSTREAM(1, "status:" << sAckIndStrcut.ack_base.status);

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
			// TBD - Udi ??
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

	TRACEINTO << "numOfActiveLogicalRsrc:" << m_numOfActiveLogicalRsrc;
	if (m_numOfActiveLogicalRsrc == 0)
	{
		// This means we got all relavent ACKs from the MFA and we may continue the flow.
		MfaAcksCompleted();
	}
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::MfaAcksCompleted()
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
WORD CExportIsdnVoicePartyCntl::CheckIfLogicalRsrcAckAccepted(eLogicalResourceTypes lrt)
{
	// Checking if the correct ACK was received -
	// If so, decrease the number of active logical resource counter by one.
	TRACEINTO << m_partyConfName << ", LogicalResourceType:" << (WORD)lrt << " ,numOfActiveLogicalRsrc:" << m_numOfActiveLogicalRsrc;

	// If it is a party resource and we did not get it already
	if (m_activeLogicalRsrc[lrt] == 1 && false == m_ackTable[lrt])
	{
		m_ackTable[lrt] = true;
		m_numOfActiveLogicalRsrc--;
		TRACEINTO << "numOfActiveLogicalRsrc:" << m_numOfActiveLogicalRsrc;
		return STATUS_OK;
	}
	else
	{
		TRACESTRFUNC(eLevelError) << "LogicalResourceType:"<< (WORD)lrt << " - Failed, invalid type";
		return STATUS_FAIL;
	}
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnTimerRAEndMove(CSegment* pParam)
{
	PASSERTSTREAM(1, m_partyConfName << " - Failed to receive end move from RA");

	// Disconnect the party
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnAudioBridgeExported(CSegment* pParam)
{
	TRACEINTO << m_partyConfName;

	CAudioBridgePartyCntl* ptr = 0;
	WORD status;
	BYTE isParams;
	*pParam >> status >> isParams;
	if (status != STATUS_OK)
	{
		PASSERT(status);
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
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
void CExportIsdnVoicePartyCntl::BridgeExportCompleted()
{
	if (m_voice && !m_bIsMemberInAudBridge)
	{
		TRACEINTO << m_partyConfName << " - Export party";

		DeleteTimer(EXPORT_BRIDGES_TOUT);

		m_state = EXPORT_PARTY;

		// send export to party
		StartTimer(EXPORT_PARTY_TOUT, 30*SECOND);
		m_pBridgeMoveParams->UnregisterBridgePartyCntlsInTask();

		m_pPartyApi->Export(m_pDestConfMbx, this, m_pComConf, m_moveType);
	}
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnTimerExport(CSegment* pParam)
{
	TRACESTRFUNC(eLevelError) << m_partyConfName;

	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), statIllegal);
}

//--------------------------------------------------------------------------
void CExportIsdnVoicePartyCntl::OnPartyExport(CSegment* pParam)
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

	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(), status);
}
