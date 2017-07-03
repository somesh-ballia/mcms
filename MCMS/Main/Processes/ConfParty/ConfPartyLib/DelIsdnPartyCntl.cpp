#include "NStream.h"
#include "DelIsdnPartyCntl.h"
#include "Conf.h"
#include "ConfPartyOpcodes.h"
#include "PartyApi.h"
#include "AllocateStructs.h"
#include "ObjString.h"
#include "AudioBridgeInterface.h"
#include "BridgePartyDisconnectParams.h"
#include "StatusesGeneral.h"
#include "OpcodesMcmsCommon.h"
#include "TraceStream.h"
#include "MuxHardwareInterface.h"
#include "NonStandardCaps.h"
#include "OpcodesMcmsMux.h"
#include "OsTask.h"
#include <sys/signal.h>

#define DISCONNECT_TIME 90       // (*seconds) timer for error handling

const WORD DESTROYPARTY = 1;     // waiting for disconnect ACK from audio bridge
const WORD DELETEFROMHW = 2;     // waiting for delete ACK from MPL and Party End Disconnect IND from party
const WORD DEALLOCATE   = 3;     // waiting for resource allocator response

PBEGIN_MESSAGE_MAP(CDelIsdnPartyCntl)
	ONEVENT(PARTYENDDISCONNECT,              DESTROYPARTY,       CDelIsdnPartyCntl::OnPartyDisconnectDestroyParty)
	ONEVENT(PARTYENDDISCONNECT,              DELETEFROMHW,       CDelIsdnPartyCntl::NullActionFunction)
	ONEVENT(PARTYCONNECT,                    ANYCASE,            CDelIsdnPartyCntl::NullActionFunction)
	ONEVENT(ACK_IND,                         DELETEFROMHW,       CDelIsdnPartyCntl::OnMplAckDeleteFromHw)
	ONEVENT(DEALLOCATE_PARTY_RSRC_IND,       DEALLOCATE,         CDelIsdnPartyCntl::OnRsrcDeallocatePartyRspDeallocate)
	ONEVENT(PARTY_AUDIO_DISCONNECTED,        IDLE,               CDelIsdnPartyCntl::OnAudBrdgDisconnect)
	ONEVENT(PARTY_VIDEO_DISCONNECTED,        IDLE,               CDelIsdnPartyCntl::OnVidBrdgDisconnect)
	ONEVENT(FECC_PARTY_BRIDGE_DISCONNECTED,  IDLE,               CDelIsdnPartyCntl::OnFeccBridgeDisConnect)
	ONEVENT(PARTY_CONTENT_DISCONNECTED,      IDLE,               CDelIsdnPartyCntl::OnContentBrdgDisconnected)
    ONEVENT(PARTY_XCODE_DISCONNECTED,        IDLE,               CDelIsdnPartyCntl::OnXCodeBrdgDisconnected)
	ONEVENT(PCM_PARTY_STATE_CHANGED,         IDLE,               CDelIsdnPartyCntl::OnPartyPcmStateChangedIdle)
	ONEVENT(BRIDGEDISCONNECT,                IDLE,               CDelIsdnPartyCntl::OnTimerBridgesDisconnect)
	ONEVENT(PCM_DISCONNECTPARTY_TOUT,        IDLE,               CDelIsdnPartyCntl::OnTimerPcmDisconnect)
	ONEVENT(PARTYDISCONNECTTOUT,             DESTROYPARTY,       CDelIsdnPartyCntl::OnTimerPartyDisconnectDestroyParty)
	ONEVENT(MPLDISCONNECTTOUT,               DELETEFROMHW,       CDelIsdnPartyCntl::OnTimerMplDisconnectDeleteFromHw)
	ONEVENT(RADISCONNECTTOUT,                DEALLOCATE,         CDelIsdnPartyCntl::OnTimerRsrcAllocatorDisconnect)
	ONEVENT(DISCONNECTDELAY,                 IDLE,               CDelIsdnPartyCntl::OnPartyDelayDisconnectIdle)
	ONEVENT(ENDNETCHNLDISCONNECT,            ANYCASE,            CDelIsdnPartyCntl::NullActionFunction)
	ONEVENT(PRESENTATION_OUT_STREAM_UPDATED, ANYCASE,            CDelIsdnPartyCntl::NullActionFunction)
	ONEVENT(RMTCAP,                          ANYCASE,            CDelIsdnPartyCntl::NullActionFunction)
	ONEVENT(PARTY_VIDEO_CONNECTED,           ANYCASE,            CDelIsdnPartyCntl::NullActionFunction)

PEND_MESSAGE_MAP(CDelIsdnPartyCntl, CIsdnPartyCntl);

////////////////////////////////////////////////////////////////////////////
//                        CDelIsdnPartyCntl
////////////////////////////////////////////////////////////////////////////
CDelIsdnPartyCntl::CDelIsdnPartyCntl()
{
	m_isViolentDestroy = FALSE;
	m_partyTaskId      = 0;
	VALIDATEMESSAGEMAP;
}

//--------------------------------------------------------------------------
CDelIsdnPartyCntl::~CDelIsdnPartyCntl()
{
}

//--------------------------------------------------------------------------
CDelIsdnPartyCntl& CDelIsdnPartyCntl::operator=(const CDelIsdnPartyCntl& other)
{
	(CIsdnPartyCntl&)*this = (CIsdnPartyCntl&)other;
	return *this;
}

// Process:	The function is an entry to disconnect process on conf control level
// First we send update algorithm and mute to audio, then we send disconnect to net
// channels. We start timer in case we do not get indication for net channel disconnection.
// mode == 0 - delete party
// mode == 1 - disconnect party
// mode == 2 - disconnect & recover
//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::Disconnect(WORD mode)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << ", Mode:" << mode;

	CPartyCntl::Disconnect(mode);

	if (m_disconnectState == DISCONNECTED)
	{
		m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_DISCONNECTED);
		m_pTaskApi->EndDelParty(m_pParty, statOK);
		return;
	}

	if (mode == 2)    // disconnect & recover
		ON(m_isRecover);

	// If party was disconnected during Move Process and after
	if (m_isPartyEndRAMoveOK)
		SetDataForExportPartyFail();

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_DISCONNECTING);

	if (!m_pPartyApi) // We do not have party
	{
		TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << " - Disconnecting without a party, no resources";
		EndPartyDisconnect();
		return;
	}

	if (m_eAudBridgeConnState != eBridgeDisconnected)
	{
		// Mute Party Audio Enc/Dec
		m_pAudioInterface->UpdateMute(GetPartyRsrcId(), eMediaInAndOut, eOn, MCMS);

		// ++++++++++++++++++++++++++++++++++
		// audio-off patch for mux, this will send audio off to the party
		// in order to disable audio noise while disconnecting

		CRsrcParams muxRsrcParams;
		WORD        mux_desc_found = 0;
		if (m_pPartyAllocatedRsrc)
			mux_desc_found = m_pPartyAllocatedRsrc->GetRsrcParams(muxRsrcParams, eLogical_mux);

		if (mux_desc_found)
		{
			CMuxHardwareInterface* pMux = new CMuxHardwareInterface(muxRsrcParams);

			CAudMode aud_mode;
			aud_mode.SetAudMode(Au_Off_F);
			m_pCurrentTransmitScm->SetAudMode(aud_mode);

			CVidMode vid_mode;
			vid_mode.SetVidMode(Video_Off);
			m_pCurrentTransmitScm->SetVidMode(vid_mode);

			CContentMode contentMode = m_pCurrentTransmitScm->m_contentMode;
			contentMode.SetContentRate(AMSC_0k);
			m_pCurrentTransmitScm->SetContentMode(contentMode);

			pMux->SetXmitRcvMode(*m_pCurrentTransmitScm, SET_XMIT_MODE, 0, IsPartyH239());

			POBJDELETE(pMux);
		}
		else
		{
			TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << " - Mux descriptor not valid";
		}
	}

	// remove from terminal list only if it was numerated, in case status is not OK - We rejected the call
	// and there is no need to remove from numbering list.
	if (m_pTerminalNumberingManager && m_isTerminalNumberingConn)
	{
		STATUS removeStatus = STATUS_OK;
		removeStatus = m_pTerminalNumberingManager->Remove(m_pParty);

		PASSERTSTREAM(removeStatus != STATUS_OK, "PartyId:" << GetPartyRsrcId() << ", Status:" << removeStatus << " - Party is not in the m_pTerminalNumberingManager list");

		OFF(m_isTerminalNumberingConn);
	}

	// Send Disconnect to bridges:
	if ((m_eAudBridgeConnState != eBridgeDisconnected) || (m_eVidBridgeConnState != eBridgeDisconnected) || m_isFeccConn || m_isContentConn)
	{
		DWORD ticks = BRIDGES_DISCONNECT_TIME;
		if (m_pConf->GetCommConf()->IsRollCall())
			ticks += 15; // 32 delay for the video bridge (because: max 8 seconds delay for a party + max delay for 4 parties). So I add a little bit.

		StartTimer(BRIDGEDISCONNECT, ticks * SECOND);

		if (m_eVidBridgeConnState != eBridgeDisconnected)
			DisconnectPartyFromVideoBridge();

		if (m_isFeccConn)
			DisconnectPartyFromFECCBridge();

		// "disconnect audio" will be the last msg sent, in order to
		// avoid noises at disconnection.
		if (m_eAudBridgeConnState != eBridgeDisconnected)
			DisconnectPartyFromAudioBridge();

		if (m_isContentConn)
			DisconnectPartyFromContentBridge();
	}
	else
		BridgeDisconnetCompleted();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::BridgeDisconnetCompleted()
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	if (!m_bIsMemberInAudBridge && !m_bIsMemberInVidBridge && !m_isFeccConn && !m_isContentConn && !m_isChairConn && !m_isXCodeConn)
	{
		if (IsValidTimer(BRIDGEDISCONNECT))
			DeleteTimer(BRIDGEDISCONNECT);

		m_pConfAppMngrInterface->RemovePartyFromCAM(GetPartyRsrcId(), GetName());

		if (m_isPcmConnected)
		{
			TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << " - Wait 5 more seconds for pcm disconnection";
			StartTimer(PCM_DISCONNECTPARTY_TOUT, 5*SECOND);
		}

		// wait for pcm disconnection
		PcmDisconnectionCompleted();
	}
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::PcmDisconnectionCompleted()
{
	if (!m_bIsMemberInAudBridge && !m_bIsMemberInVidBridge && !m_isFeccConn && !m_isContentConn && !m_isChairConn && !m_isPcmConnected)
	{
		if (IsValidTimer(PCM_DISCONNECTPARTY_TOUT))
		{
			DeleteTimer(PCM_DISCONNECTPARTY_TOUT);
			TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;
		}

		if (m_pPartyApi)
		{
			// party cleanup on exception
			if (GetIsViolentDestroy())
			{
				COsTask::SendSignal(GetPartyTaskId(), SIGHUP);
				if (m_pPartyHWInterface)
					DeletePartyFromHW();
				else
					DeallocatePartyResources();
			}
			else
			{
				DestroyParty();
			}
		}
		else if (m_pPartyHWInterface)
			DeletePartyFromHW();
		else
			DeallocatePartyResources();
	}
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::EndPartyDisconnect()
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	m_state = IDLE;
	DeleteTimer(PARTYDISCONNECTTOUT); // One timer for entire process will be deleted after deallocation

	if (GetDisconnectMode())
		m_disconnectState = DISCONNECTED;

#ifdef LOOKUP_TABLE_DEBUG_TRACE
	TRACEINTO << "D.M. PartyId:" << GetPartyId() << ", monitorPartyId:" << m_monitorPartyId << ", pParty:" << std::hex << m_pParty;
#endif
	m_pParty = (CTaskApp*)(m_monitorPartyId + 100);          // We use a trick which allowed to find party in spite of invalid pointer

	m_pConf->UpdateDB(m_pParty, PARTYSTATE, PARTY_DISCONNECTED);
	m_pConf->UpdateDB(m_pParty, PARTY_ENCRYPTION_STATE, NO);

	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0x00000000);     // indicate party is not audio muted by operator
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0x0000000E);     // indicate party is not video muted by operator
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0xF0000000);     // indicate party is not audio self muted
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0xF000000E);     // indicate party is not video self muted
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0x0F000000);     // indicate party is not audio muted by MCU
	m_pConf->UpdateDB(m_pParty, MUTE_STATE, 0x0F00000E);     // indicate party is not video muted by MCU
	m_pConf->UpdateDB(m_pParty, BLOCK_STATE, 0x00000000);    // indicate party video is not blocked

	m_pTaskApi->UpdatePartyStateInCdr(m_pParty);

	CSegment* pSeg  = new CSegment;
	BYTE onOff = 0;
	*pSeg << m_name << onOff;
	m_pTaskApi->UpdateDB(NULL, PARTY_REQUEST_TO_SPEAK, (DWORD)0, 1, pSeg);
	POBJDELETE(pSeg);

	WORD status = statOK;
	if (m_isRecover)
	{
		status = m_disconnectMode;
		OFF(m_isRecover);
	}

	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << " - Party disconnection finished";

	m_pTaskApi->EndDelParty(m_pParty, status);
	RedialIfNeeded();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnAudBrdgDisconnect(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam); // the status is not relevant here - we just turn on the m_isFaulty flag
	BridgeDisconnetCompleted();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnVidBrdgDisconnect(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam); // the status is not relevant here - we just turn on the m_isFaulty flag
	BridgeDisconnetCompleted();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnFeccBridgeDisConnect(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	WORD status;
	*pParam >> status;
	DBGPASSERT(status);
	OFF(m_isFeccConn);

	BridgeDisconnetCompleted();
}

//--------------------------------------------------------------------------
int CDelIsdnPartyCntl::OnContentBrdgDisconnected(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	WORD status;
	*pParam >> status;
	DBGPASSERT(status);
	OFF(m_isContentConn);
	m_pTaskApi->UpdateDB(m_pParty, CONTENTCON, FALSE);

	BridgeDisconnetCompleted();

	return 0;
}
//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnXCodeBrdgDisconnected(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	WORD status;
	*pParam >> status;
	DBGPASSERT(status);
	OFF(m_isXCodeConn);
	BridgeDisconnetCompleted();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnPartyPcmStateChangedIdle(CSegment* pParam)
{
	CPartyCntl::OnPartyPcmStateChangedAnycase(pParam);

	PcmDisconnectionCompleted();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::DestroyParty()
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	m_state = DESTROYPARTY;
	if (m_pPartyApi && CPObject::IsValidPObjectPtr(m_pPartyApi))
	{
		StartTimer(PARTYDISCONNECTTOUT, DISCONNECT_TIME*SECOND);
		m_pPartyApi->Destroy();
		POBJDELETE(m_pPartyApi);
	}
	else
		DeletePartyFromHW();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnPartyDisconnectDestroyParty(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	WORD status;
	*pParam >> status;
	m_disconnectionCause = status;

	if (IsValidTimer(PARTYDISCONNECTTOUT))
		DeleteTimer(PARTYDISCONNECTTOUT);

	DeletePartyFromHW();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::DeletePartyFromHW()
{
	if (m_pPartyHWInterface && CPObject::IsValidPObjectPtr(m_pPartyHWInterface) && m_isSmartRecovery == 0)
	{
		TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << " - Send message to Party Hardware Interface";
		m_state = DELETEFROMHW;
		StartTimer(MPLDISCONNECTTOUT, MPL_DISCONNECT_TIME * SECOND);
		m_pPartyHWInterface->SendDeleteParty();
	}
	else
	{
		TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << " - Don't send message to Party Hardware Interface";
		OFF(m_isSmartRecovery);
		DeallocatePartyResources();
	}
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnMplAckDeleteFromHw(CSegment* pParam)
{
	OPCODE AckOpcode;
	DWORD  ack_seq_num;
	STATUS status;
	*pParam >> AckOpcode >> ack_seq_num >> status;

	if (AckOpcode == CONF_MPL_DELETE_PARTY_REQ)
	{
		if (IsValidTimer(MPLDISCONNECTTOUT))
			DeleteTimer(MPLDISCONNECTTOUT);

		if (status != STATUS_OK)
		{
			TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << ", Status:" << status << " - Delete failed";
			m_isFaulty = 1; // Invoking KillPort process in RA.
		}
		else
		{
			TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << ", Status:" << status;
		}

		POBJDELETE(m_pPartyHWInterface);
		DeallocatePartyResources();
	}
	else
	{
		PASSERTSTREAM(1, "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << ", Opcode:" << AckOpcode << " - Invalid opcode");
	}
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnTimerPartyDisconnectDestroyParty(CSegment* pParam)
{
	PASSERTSTREAM(1, "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name);

	DeletePartyFromHW();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnTimerMplDisconnectDeleteFromHw(CSegment* pParam)
{
	PASSERTSTREAM(1, "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name);

	m_isFaulty = 1;
	DeallocatePartyResources();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnTimerBridgesDisconnect(CSegment* pParam)
{
	PASSERTSTREAM(1, "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name);

	m_isFaulty = 1;

	m_pConfAppMngrInterface->RemovePartyFromCAM(GetPartyRsrcId(), GetName());

	if (m_pPartyApi)
		DestroyParty();
	else if (m_pPartyHWInterface)
		DeletePartyFromHW();
	else
		DeallocatePartyResources();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnTimerPcmDisconnect(CSegment* pParam)
{
	PASSERTSTREAM(1, "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name);

	if (m_pPartyApi)
		DestroyParty();
	else if (m_pPartyHWInterface)
		DeletePartyFromHW();
	else
		DeallocatePartyResources();
}

// Send DEALLOCATE_PARTY_RESOURCES_REQ event to resource allocator process
//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::DeallocatePartyResources()
{
	if (m_pPartyAllocatedRsrc && (m_pPartyAllocatedRsrc->GetStatus() == STATUS_OK))
	{
		TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << " - Send message to Resource Allocator";
		m_state = DEALLOCATE;
		StartTimer(RADISCONNECTTOUT, RA_DISCONNECT_TIME * SECOND);
		CreateAndSendDeallocatePartyResources();
	}
	else
	{
		TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name << " - Don't send message to Resource Allocator";
		EndPartyDisconnect();
	}
}

//--------------------------------------------------------------------------
// Remove party resources from global resource/routing table
// Call function EndPartyDisconnect
void CDelIsdnPartyCntl::OnRsrcDeallocatePartyRspDeallocate(CSegment* pParam)
{
	if (IsValidTimer(RADISCONNECTTOUT))
		DeleteTimer(RADISCONNECTTOUT);

	DEALLOC_PARTY_IND_PARAMS_S  tDeallocatePartyIndParams;
	memset(&tDeallocatePartyIndParams, 0, sizeof(tDeallocatePartyIndParams));
	pParam->Get((BYTE*)(&tDeallocatePartyIndParams), sizeof(tDeallocatePartyIndParams));

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
void CDelIsdnPartyCntl::OnTimerRsrcAllocatorDisconnect(CSegment* pParam)
{
	PASSERTSTREAM(1, "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name);

	EndPartyDisconnect();
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::UpdateConfEndDisconnect(WORD status)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	// Delete all the valid timer in case the party is only disconnected (dial out) and not deleted.
	if (IsValidTimer(BRIDGEDISCONNECT))
		DeleteTimer(BRIDGEDISCONNECT);

	if (IsValidTimer(PARTYDISCONNECTTOUT))
		DeleteTimer(PARTYDISCONNECTTOUT);

	m_pTaskApi->UpdateDB(m_pParty, PARTYSTATE, PARTY_DISCONNECTED);
	m_pTaskApi->EndDelParty(m_pParty, status);
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::DisconnectISDN(WORD mode, DWORD disconnectionDelay)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	BYTE bContinueDisconnect = CheckDisconnectCases(mode, disconnectionDelay);
	if (bContinueDisconnect)
		Disconnect(mode);
}

//--------------------------------------------------------------------------
void CDelIsdnPartyCntl::OnPartyDelayDisconnectIdle(CSegment* pParam)
{
	TRACEINTO << "PartyId:" << GetPartyRsrcId() << ", PartyName:" << m_name;

	WORD mode = 0;
	*pParam >> mode;

	Disconnect(mode);
}
