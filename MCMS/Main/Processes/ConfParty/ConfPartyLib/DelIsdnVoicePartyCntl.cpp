#include "NStream.h"
#include "DelIsdnVoicePartyCntl.h"
#include "Conf.h"
#include "PartyApi.h"
#include "AllocateStructs.h"
#include "ObjString.h"
#include "AudioBridgeInterface.h"
#include "BridgePartyDisconnectParams.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "TraceStream.h"
#include "OsTask.h"
#include <sys/signal.h>

#define DISCONNECT_TIME 90                 // (*seconds) timer for error handling
#define	DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT	     17892
#define DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT_VALUE  3*SECOND

const WORD PARTY_RSR_DEFICIENCY = 6;       // Incase there was resource diffisience in DIALIN

// CDelIsdnVoicePartyCntl states
const WORD DISCONNECTNET        = 1;       // waiting for net chnl disconnect ind from party
const WORD DISCONNECTAUDIO      = 2;       // waiting for disconnect ack from audio bridge
const WORD DESTROYPARTY         = 3;       // waiting for delete ack from mpl and Party End Disconnect ind from party
const WORD DEALLOCATE           = 4;       // waiting for resource allocator response


// IDLE -> DISCONNECTNET -> DISCONNECTAUDIO -> DESTROYPARTY -> DEALLOCATE -> IDLE

PBEGIN_MESSAGE_MAP(CDelIsdnVoicePartyCntl)
	ONEVENT(PARTYENDDISCONNECT,        DESTROYPARTY,    CDelIsdnVoicePartyCntl::OnPartyDisconnectDestroyParty)

	ONEVENT(ENDNETCHNLDISCONNECT,      DISCONNECTNET,   CDelIsdnVoicePartyCntl::OnPartyNetChnlDisconnetDisconnectNet)
	ONEVENT(ENDNETCHNLDISCONNECT,      DISCONNECTAUDIO, CDelIsdnVoicePartyCntl::NullActionFunction)
	ONEVENT(ENDNETCHNLDISCONNECT,      DESTROYPARTY,    CDelIsdnVoicePartyCntl::NullActionFunction)
	ONEVENT(ENDNETCHNLDISCONNECT,      DEALLOCATE,      CDelIsdnVoicePartyCntl::NullActionFunction)

	ONEVENT(PARTY_AUDIO_CONNECTED,     IDLE,            CDelIsdnVoicePartyCntl::NullActionFunction)
	ONEVENT(PARTY_AUDIO_DISCONNECTED,  DISCONNECTAUDIO, CDelIsdnVoicePartyCntl::OnAudDisConnectDisconnectAudio)
	ONEVENT(ACK_IND,                   DESTROYPARTY,    CDelIsdnVoicePartyCntl::OnMplAckDestroyParty)
	ONEVENT(DEALLOCATE_PARTY_RSRC_IND, DEALLOCATE,      CDelIsdnVoicePartyCntl::OnRsrcDeallocatePartyRspDeallocate)

	ONEVENT(PARTYDISCONNECTTOUT,       DISCONNECTNET,   CDelIsdnVoicePartyCntl::OnTimerDisconnectNet)
	ONEVENT(PARTYDISCONNECTTOUT,       DESTROYPARTY,    CDelIsdnVoicePartyCntl::OnTimerDestroyParty)
	ONEVENT(PARTYDISCONNECTTOUT,       DISCONNECTAUDIO, CDelIsdnVoicePartyCntl::OnTimerDisconnectAudio)
	ONEVENT(PARTYDISCONNECTTOUT,       DEALLOCATE,      CDelIsdnVoicePartyCntl::OnTimerDeallocate)
	ONEVENT(PARTYDISCONNECTTOUT,       IDLE,            CDelIsdnVoicePartyCntl::OnTimerIdle)
	ONEVENT(DISCONNECTDELAY,           IDLE,            CDelIsdnVoicePartyCntl::OnPartyDelayDisconnectIdle)

	ONEVENT(END_AVC_TO_SVC_ART_TRANSLATOR_DISCONNECTED,       DISCONNECTAUDIO,    CDelIsdnVoicePartyCntl::OnEndAvcToSvcArtTranslatorDisconnectedDisconnectAudio)

	ONEVENT(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT,        DISCONNECTAUDIO,    CDelIsdnVoicePartyCntl::OnDisconnectAvcToSvcArtTranslatorCloseToutDISCONNECTAUDIO)


PEND_MESSAGE_MAP(CDelIsdnVoicePartyCntl, CIsdnPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CDelIsdnVoicePartyCntl
////////////////////////////////////////////////////////////////////////////
CDelIsdnVoicePartyCntl::CDelIsdnVoicePartyCntl()
{
	m_isWaitForPartyDisconnection = FALSE;
	m_isWaitForDeleteInd          = FALSE;
	m_isViolentDestroy            = FALSE;
	m_partyTaskId                 = 0;
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CDelIsdnVoicePartyCntl::~CDelIsdnVoicePartyCntl()
{
}

//--------------------------------------------------------------------------
CDelIsdnVoicePartyCntl& CDelIsdnVoicePartyCntl::operator=(const CDelIsdnVoicePartyCntl& other)
{
	(CIsdnPartyCntl&)*this = (CIsdnPartyCntl&)other;
	return *this;
}

//--------------------------------------------------------------------------
void* CDelIsdnVoicePartyCntl::GetMessageMap()
{
	return (void*)m_msgEntries;
}

//--------------------------------------------------------------------------
// The function is an entry to disconnect process on conf control level
// First we send update algorithm and mute to audio, then we send disconnect to net
// channels. We start timer in case we do not get indication for net channel disconnection.
// mode == 0 - delete party
// mode == 1 - disconnect party
// mode == 2 - disconnect & recover
void CDelIsdnVoicePartyCntl::Disconnect(WORD mode)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name << ", Mode:" << mode << ", IsViolentDestroy:" << (int)GetIsViolentDestroy();

	CPartyCntl::Disconnect(mode);

	if (m_disconnectState == DISCONNECTED)
	{
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_DISCONNECTED);
		m_pTaskApi->EndDelParty(m_pParty, statOK);
		return;
	}

	if (mode == 2)    // disconnect & recover
		ON(m_isRecover);

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_DISCONNECTING);

	// Start Timer for entire disconnection process - carmel
	if (GetIsViolentDestroy())
	{
		StartTimer(PARTYDISCONNECTTOUT, 1*SECOND);
	}
	else
	{
		StartTimer(PARTYDISCONNECTTOUT, DISCONNECT_TIME*SECOND);
	}

	if (!m_pPartyApi) // We do not have party
	{
		TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name << " - Disconnecting without a party, no resources";
		EndPartyDisconnect();
		return;
	}

	// In case we do not have Audio connection finish party disconnection
	if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		TRACESTRFUNC(eLevelError) << "Failed, disconnecting party with resources, but no audio";
		DBGPASSERT(1);
		m_state = DISCONNECTNET;
		// VNGR-6166
		// m_pPartyApi->LogicalDelNetChannel(PARTY_RSR_DEFICIENCY);
		BridgeDisconnetCompleted();
		return;
	}

	// Mute Party Audio Enc/Dec
	m_pAudioInterface->UpdateMute(GetPartyId(), eMediaInAndOut, eOn, MCMS);

	// If party was disconnected during Move Process and after
	if (m_isPartyEndRAMoveOK)
		SetDataForExportPartyFail();

	m_state = DISCONNECTNET;

	m_pPartyApi->DelNetChannel(0);
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::DisconnectAudioBridge()
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	if (m_eAudBridgeConnState != eBridgeDisconnected)
	{
		m_state = DISCONNECTAUDIO;
		CPartyCntl::DisconnectPartyFromAudioBridge();
		m_welcome_msg_time &= 0x7fff;  // for welcome_no_wait Timer !
	}
	else
	{
		BridgeDisconnetCompleted();
	}
}
//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::BridgeDisconnetCompleted()
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name << ", IsViolentDestroy:" << (int)GetIsViolentDestroy();

	if (!m_bIsMemberInAudBridge)
	{
		m_pConfAppMngrInterface->RemovePartyFromCAM(GetPartyId(), GetName());


		if (m_pPartyApi || m_pPartyHWInterface)
		{
			// party cleanup on exception
			PTRACE(eLevelInfoNormal, "party cleanup on exception");
			if (GetIsViolentDestroy())
			{
				PTRACE2INT(eLevelInfoNormal, "m_pParty->GetTaskId = ", m_pParty->GetTaskId());
				COsTask::SendSignal(GetPartyTaskId(), SIGHUP);

				PTRACE(eLevelInfoNormal, "before m_pPartyHWInterface->SendDeleteParty");
				if (CPObject::IsValidPObjectPtr(m_pPartyHWInterface))
				{
					PTRACE(eLevelInfoNormal, "before m_pPartyHWInterface->SendDeleteParty");
					m_pPartyHWInterface->SendDeleteParty();
					m_isWaitForDeleteInd = TRUE; // sent DELETE PARTY to MPL and waiting for Ack
				}

				DeallocatePartyResources();
			}
			else
			{
				PTRACE(eLevelInfoNormal, "before DestroyParty");
				DestroyParty();
			}
		}
		else
		{
			DeallocatePartyResources();
		}
	}
}
//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::DestroyParty()
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	m_state = DESTROYPARTY;
	if (CPObject::IsValidPObjectPtr(m_pPartyApi))
	{
		m_pPartyApi->Destroy();
		POBJDELETE(m_pPartyApi);
		m_isWaitForPartyDisconnection = TRUE; // party API is null, but party is still alive
	}

	if (CPObject::IsValidPObjectPtr(m_pPartyHWInterface))
	{
		m_pPartyHWInterface->SendDeleteParty();
		m_isWaitForDeleteInd = TRUE; // sent DELETE PARTY to MPL and waiting for Ack
	}
}

//--------------------------------------------------------------------------
// Send DEALLOCATE_PARTY_RESOURCES_REQ event to resource allocator process
// and changes state to DEALLOCATE.
void CDelIsdnVoicePartyCntl::DeallocatePartyResources()
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	if (m_pPartyAllocatedRsrc && m_pPartyAllocatedRsrc->GetStatus() == STATUS_OK)
	{
		m_state = DEALLOCATE;
		CreateAndSendDeallocatePartyResources();
	}
	else
	{
		EndPartyDisconnect();
	}
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::EndPartyDisconnect()
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	m_state = IDLE;
	DeleteTimer(PARTYDISCONNECTTOUT); // One timer for entire process will be deleted after deallocation

	if (GetDisconnectMode())
		m_disconnectState = DISCONNECTED;

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
	m_pParty = (CTaskApp*)(m_monitorPartyId + 100);          // We use a trick which allowed to find party in spite of invalid pointer

	m_pConf->UpdateDB(m_pParty, PARTYSTATE, PARTY_DISCONNECTED);

	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0x00000000);     // indicate party is not audio muted by operator
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0x0000000E);     // indicate party is not video muted by operator
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0xF0000000);     // indicate party is not audio self muted
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0xF000000E);     // indicate party is not video self muted
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0x0F000000);     // indicate party is not audio muted by MCU
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0x0F00000E);     // indicate party is not video muted by MCU
	m_pConf->UpdateDB(m_pParty, BLOCK_STATE, 0x00000000);    // indicate party video is not blocked

	m_pTaskApi->UpdatePartyStateInCdr(m_pParty);

	WORD status = statOK;
	if (m_isRecover)
	{
		status = m_disconnectMode;
		OFF(m_isRecover);
	}

	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name << " - Party disconnection finished";

	m_pTaskApi->EndDelParty(m_pParty, status);
	RedialIfNeeded();
}

//--------------------------------------------------------------------------
// The function is a reaction to end of net all channels disconnection
// Starts disconnect audio process
void CDelIsdnVoicePartyCntl::OnPartyNetChnlDisconnetDisconnectNet(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	WORD status;
	*pParam >> status;
	DBGPASSERT(status);
	m_disconnectionCause = status;
	DisconnectAudioBridge();
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnAudDisConnectDisconnectAudio(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam); // the status is not relevant here - we just turn on the m_isFaulty flag
	DisconnectAvcToSvcArtTranslator();
	StartTimer(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT, DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT_VALUE);
//	BridgeDisconnetCompleted();
}

//--------------------------------------------------------------------------
// The function is a reaction to party disconnect and destroy
void CDelIsdnVoicePartyCntl::OnPartyDisconnectDestroyParty(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	WORD status;
	*pParam >> status;

	m_isWaitForPartyDisconnection = FALSE;

	if (m_isWaitForDeleteInd == FALSE)
		DeallocatePartyResources();
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnMplAckDestroyParty(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name << ", AckOpcode:" << AckOpcode << ", status:" << status;

	if (AckOpcode == CONF_MPL_DELETE_PARTY_REQ)
	{
		m_isWaitForDeleteInd = FALSE;
		if (status != STATUS_OK)
			m_isFaulty = 1;   // Invoking KillPort process in RA.

		if (m_isWaitForPartyDisconnection == FALSE)
			DeallocatePartyResources();
	}
	else
	{
		DBGPASSERT(101); // Invalid AckOpcode
	}
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnRsrcDeallocatePartyRspDeallocate(CSegment* pParam)
{
	DWORD structLen = sizeof(DEALLOC_PARTY_IND_PARAMS_S);
	DEALLOC_PARTY_IND_PARAMS_S tDeallocatePartyIndParams;
	memset(&tDeallocatePartyIndParams, 0, structLen);
	pParam->Get((BYTE*)(&tDeallocatePartyIndParams), structLen);

	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << ", Status:" << CProcessBase::GetProcess()->GetStatusAsString(tDeallocatePartyIndParams.status).c_str();
	PASSERTSTREAM(tDeallocatePartyIndParams.status != STATUS_OK, "Deallocation Failed, continue process");

	// remove party resources from global resource/routing table
	if (::CPObject::IsValidPObjectPtr(m_pPartyAllocatedRsrc))
	{
		m_pPartyAllocatedRsrc->DeleteFromGlobalRsrcRoutingTbl();
		POBJDELETE(m_pPartyAllocatedRsrc);
	}
	else
	{
		PASSERTSTREAM(1, "Invalid pointer");
	}

	EndPartyDisconnect();
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnTimerDisconnectNet(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;
	PASSERT(101);

	m_isFaulty = 1; // Invoking KillPort process in RA.
	StartTimer(PARTYDISCONNECTTOUT, DISCONNECT_TIME*SECOND);
	DisconnectAudioBridge();
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnTimerDisconnectAudio(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;
	DBGPASSERT(101);

	m_pTaskApi->UpdateDB(m_pParty, AUDCON, FALSE);
	m_isFaulty = 1; // Invoking KillPort process in RA.
	StartTimer(PARTYDISCONNECTTOUT, DISCONNECT_TIME*SECOND);

	m_pConfAppMngrInterface->RemovePartyFromCAM(GetPartyId(), GetName());

	OFF(m_bIsMemberInAudBridge);

//	BridgeDisconnetCompleted();
	DisconnectAvcToSvcArtTranslator();
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnTimerDestroyParty(CSegment* pParam)
{
	if (m_isWaitForPartyDisconnection)
	{
		TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;
		DBGPASSERT(101);
	}

	if (m_isWaitForDeleteInd)
	{
		TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;
		DBGPASSERT(101);
	}

	m_isFaulty                    = 1; // Invoking KillPort process in RA.
	m_isWaitForPartyDisconnection = FALSE;
	m_isWaitForDeleteInd          = FALSE;

	StartTimer(PARTYDISCONNECTTOUT, DISCONNECT_TIME*SECOND);
	DeallocatePartyResources();
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnTimerDeallocate(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;
	DBGPASSERT(101);
	m_isFaulty = 1; // Invoking KillPort process in RA.
	if (::CPObject::IsValidPObjectPtr(m_pPartyAllocatedRsrc))
	{
		m_pPartyAllocatedRsrc->DeleteFromGlobalRsrcRoutingTbl();
		POBJDELETE(m_pPartyAllocatedRsrc);
	}
	else
	{
		DBGPASSERT(101);
	}

	StartTimer(PARTYDISCONNECTTOUT, DISCONNECT_TIME*SECOND);
	EndPartyDisconnect();
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnTimerIdle(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;
	PASSERT(101);
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::UpdateConfEndDisconnect(WORD status)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	// Delete all the valid timer in case the party is only disconnected (dial out) and not deleted.
	if (IsValidTimer(BRIDGEDISCONNECT))
		DeleteTimer(BRIDGEDISCONNECT);

	if (IsValidTimer(PARTYDISCONNECTTOUT))
		DeleteTimer(PARTYDISCONNECTTOUT);

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_DISCONNECTED);
	m_pTaskApi->EndDelParty(m_pParty, status);
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::DisconnectPSTN(WORD mode, DWORD disconnectionDelay)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	BYTE bContinueDisconnect = CheckDisconnectCases(mode, disconnectionDelay);
	if (bContinueDisconnect)
		Disconnect(mode);
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnPartyDelayDisconnectIdle(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyId() << ", PartyName:" << m_name;

	WORD mode = 0;
	*pParam >> mode;

	Disconnect(mode);
}

//--------------------------------------------------------------------------
BOOL CDelIsdnVoicePartyCntl::GetIsViolentDestroy()
{
	return m_isViolentDestroy;
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::SetIsViolentDestroy(BOOL isViolent)
{
	m_isViolentDestroy = isViolent;
}

//--------------------------------------------------------------------------
DWORD CDelIsdnVoicePartyCntl::GetPartyTaskId()
{
	return m_partyTaskId;
}

//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::SetPartyTaskId(DWORD taskId)
{
	m_partyTaskId = taskId;
}
//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnEndAvcToSvcArtTranslatorDisconnectedDisconnectAudio(CSegment* pParam)
{
	STATUS status;
	*pParam >> status;

	DeleteTimer(DISCONNECT_AVC_TO_SVC_ART_TRANSLATOR_TOUT);

	InitAllAvcToSvcArtTranslatorFlags();

	TRACEINTO
			<< "PartyId:" << GetPartyRsrcId()
			<< ", PartyName:" << m_partyConfName
			<< ", ConfId:" << GetConfRsrcId()
			<< ", status:" << ((status == STATUS_OK) ? "OK" : "FAILED") << " (" << status << ")";

	if (status == STATUS_FAIL)
	{
		m_isFaulty = 1; //To Kill all ports;
	}
	BridgeDisconnetCompleted();
}
//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::OnDisconnectAvcToSvcArtTranslatorCloseToutDISCONNECTAUDIO(CSegment* pParam)
{
	m_isFaulty = 1;
	BridgeDisconnetCompleted();
}
//--------------------------------------------------------------------------
void CDelIsdnVoicePartyCntl::InitAllAvcToSvcArtTranslatorFlags()
{
	m_ssrcAudio = 0;
	m_bIsAlreadyUpgradeToMixAvcToSvc = false;
	m_bIsAckOnCreatePartyReceived = false;
	m_bIsAckOnAvcToSvcArtTransalatorReceived = false;
}

