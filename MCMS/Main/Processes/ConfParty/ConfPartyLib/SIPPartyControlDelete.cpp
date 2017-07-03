//+========================================================================+
//                            SIPPartyControlDelete.cpp                    |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControlDelete.cpp                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "StateMachine.h"
#include "Segment.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "NStream.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpCommonDefinitions.h"
#include "IpAddressDefinitions.h"
#include "IpScm.h"
#include "ConfPartyOpcodes.h"
#include "ConfDef.h"
#include "Conf.h"
#include "ConfApi.h"
#include "CommModeInfo.h"
#include "Capabilities.h"
#include "TaskApi.h"
#include "TaskApp.h"
#include "Party.h"
#include "PartyApi.h"
	
#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipCall.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "IpPartyControl.h"
#include "SipNetSetup.h"
#include "SIPCommon.h"
#include "SIPPartyControl.h"
#include "SIPPartyControlDelete.h"
#include "PartyRsrcDesc.h"
#include "BridgePartyDisconnectParams.h"
#include "BridgePartyVideoParams.h"
#include "BridgePartyAudioParams.h"
#include "AudioBridgeInterface.h"
#include "OsTask.h"
#include <sys/signal.h>

PBEGIN_MESSAGE_MAP(CSipDelPartyCntl)

	ONEVENT(PARTYENDDISCONNECT,				DESTROY_PARTY,		CSipDelPartyCntl::OnPartyDisconneted)
	ONEVENT(PARTYENDDISCONNECT,				IDLE,				CSipDelPartyCntl::OnPartyDisconnetedIdle)
	ONEVENT(PARTY_AUDIO_DISCONNECTED,		IDLE,				CSipDelPartyCntl::OnAudioBrdgDisconnected)
    //ONEVENT(PARTY_XCODE_DISCONNECTED,       ANYCASE /* TBD*/,   CSipDelPartyCntl::NullActionFunction)
    ONEVENT(PARTY_XCODE_DISCONNECTED,       IDLE,               CSipDelPartyCntl::OnXCodeBrdgDisconnected)
	ONEVENT(PARTY_VIDEO_DISCONNECTED,		IDLE,				CSipDelPartyCntl::OnVideoBrdgDisconnected)
	ONEVENT(FECC_PARTY_BRIDGE_DISCONNECTED,	IDLE,				CSipDelPartyCntl::OnFeccBridgeDisConnect)
	ONEVENT(PCM_PARTY_STATE_CHANGED,		IDLE,				CSipDelPartyCntl::OnPartyPcmStateChangedIdle)
	ONEVENT(PARTY_CONTENT_DISCONNECTED,		IDLE,				CSipDelPartyCntl::OnContentBrdgDisconnected)

	//mpl events:
	ONEVENT(ACK_IND,						DELETE_FROM_HW,		CSipDelPartyCntl::OnMplAckDeleteFromHw)
	ONEVENT(DEALLOCATE_PARTY_RSRC_IND,		DEALLOCATE_RSC,		CSipDelPartyCntl::OnRsrcDeallocatePartyRspDeallocate)
	
	//timers
	ONEVENT(BRIDGEDISCONNECT,				IDLE,				CSipDelPartyCntl::OnTimerBridgesDisconnect)
	ONEVENT(PCM_DISCONNECTPARTY_TOUT,		IDLE,				CSipDelPartyCntl::OnTimerPcmDisconnect)
	ONEVENT(PARTYDISCONNECTTOUT,			DESTROY_PARTY,		CSipDelPartyCntl::OnTimerPartyDisconnect)
	ONEVENT(PARTYRMTCAP,					DESTROY_PARTY,		CSipDelPartyCntl::NullActionFunction)
	ONEVENT(MPLDISCONNECTTOUT,				DELETE_FROM_HW,		CSipDelPartyCntl::OnTimerMplDisconnectDeleteFromHw)
	ONEVENT(RADISCONNECTTOUT,				DEALLOCATE_RSC,		CSipDelPartyCntl::OnTimerRsrcAllocatorDisconnect)
	ONEVENT(IPPARTYCONNECTED,               ANYCASE,            CSipDelPartyCntl::NullActionFunction)
    ONEVENT(PARTY_AUDIO_CONNECTED,				ANYCASE,		CSipDelPartyCntl::NullActionFunction)
	ONEVENT(PARTY_VIDEO_CONNECTED,			ANYCASE,			CSipDelPartyCntl::NullActionFunction)
	ONEVENT(PARTY_CONTENT_CONNECTED,		ANYCASE,			CSipDelPartyCntl::NullActionFunction)
	ONEVENT(FECC_PARTY_BRIDGE_CONNECTED,	ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(DISCONNECTDELAY,				IDLE,				CSipDelPartyCntl::OnTimerDelayDisconnectIdle)
	
	ONEVENT(IPPARTYUPDATEBRIDGES,         	ANYCASE,    		CSipDelPartyCntl::NullActionFunction)
    ONEVENT(UPDATE_VIDEO_RATE,  			ANYCASE,	        CSipDelPartyCntl::NullActionFunction)
 	ONEVENT(UPDATEVISUALNAME,        	 	ANYCASE, CSipDelPartyCntl::NullActionFunction)
// LPR
  ONEVENT(LPR_CHANGE_RATE,					ANYCASE,		    CSipDelPartyCntl::NullActionFunction)
 	ONEVENT(CHANGE_CONTENT_BIT_RATE_BY_LPR, ANYCASE, 			CSipDelPartyCntl::NullActionFunction)// VNGFE-8204
    //Cop
    ONEVENT(COP_VIDEO_IN_CHANGE_MODE,		ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(COP_VIDEO_OUT_CHANGE_MODE,		ANYCASE,			CSipDelPartyCntl::NullActionFunction)

//Slaves
    ONEVENT(PARTY_CONTROL_SLAVE_TO_MASTER,				    ANYCASE,			CSipDelPartyCntl::OnSlaveToMasterMessage)
    ONEVENT(DISCONNECT_SLAVE_ACK_TOUT,						ANYCASE,			CSipDelPartyCntl::OnTimerDisconnectSlavesAck)
    ONEVENT(PARTYCONTROL_PARTY_MASTER_TO_SLAVE,			    ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(REMOTE_SENT_RE_CAPS,							ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(PRESENTATION_OUT_STREAM_UPDATED,				ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(SCP_REQUEST_BY_EP,								ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(SCP_NOTIFICATION_BY_EP,							ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(RELAY_ENDPOINT_ASK_FOR_INTRA,					ANYCASE,			CSipDelPartyCntl::NullActionFunction)
	ONEVENT(REMOTE_SENT_DISCONNECT_BRIDGES,					ANYCASE,			CSipDelPartyCntl::NullActionFunction)
	ONEVENT(REMOTE_SENT_CONNECT_BRIDGES,					ANYCASE,			CSipDelPartyCntl::NullActionFunction)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,					    ANYCASE,			CSipDelPartyCntl::NullActionFunction)
	ONEVENT(PARTY_CONTROL_SLAVE_TO_MASTER_ACK,			    ANYCASE,			CSipDelPartyCntl::OnSlaveToMasterAckMessage)
//	ONEVENT(PARTY_CONTROL_SLAVE_TO_MASTER_ACK,			    ANYCASE,			CSipDelPartyCntl::NullActionFunction)
	ONEVENT(DISCONNECT_ALL_SLAVES_FROM_MASTER_PARTY_CONTROL,ANYCASE,		CSipDelPartyCntl::SendDisconnectMessageFromMasterToSlaves)
    ONEVENT(PARTY_CONTROL_MS_SLAVE_TO_MAIN_ACK,				ANYCASE,			CSipPartyCntl::OnMsSlaveToMainAckMessage)		///TBD - check if specific func is needed here
    ONEVENT(PARTY_CONTROL_ALL_MS_SLAVES_DELETED,	ANYCASE,			CSipDelPartyCntl::OnAllMsSlavesDeleted)
    ONEVENT(DISCONNECT_ALL_MS_SLAVES_TOUT,			ANYCASE,			CSipDelPartyCntl::OnAllMsSlavesDeletedTOUT)


    //MS
    ONEVENT(MSFOCUSENDDISCONNECT,				            ANYCASE,			       CSipDelPartyCntl::OnMSFocusEndDisConnection)
    ONEVENT(MSSUBSCRIBERENDDISCONNECT,				        DELETE_FROM_MNGR,			CSipDelPartyCntl::OnMSSubscriberEndDisConnection)

    ONEVENT(CONF_API_SCP_NOTIFICATION_ACK_FROM_EP,			ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(SCP_NOTIFICATION_REQ_TOUT,						ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(SCP_IVR_STATE_NOTIFICATION_REQ_TOUT,			ANYCASE,			CSipDelPartyCntl::NullActionFunction)
    ONEVENT(SCP_PIPES_MAPPING_NOTIFICATION_REQ_TOUT,		ANYCASE,			CSipDelPartyCntl::NullActionFunction)

PEND_MESSAGE_MAP(CSipDelPartyCntl,CSipPartyCntl);



/////////////////////////////////////////////////////////////////////////////
CSipDelPartyCntl::CSipDelPartyCntl()
{
	VALIDATEMESSAGEMAP
	m_DisConnectionModeForMasterInTip = 0;
	m_isViolentDestroy = FALSE;
	m_partyTaskId = 0;
	m_IsPartyDisconnectEnded = FALSE;

}


/////////////////////////////////////////////////////////////////////////////
CSipDelPartyCntl::~CSipDelPartyCntl()
{
 	POBJDELETE(m_pMsOrganizerMngr);
	POBJDELETE(m_pMsFocusMngr);
	POBJDELETE(m_pMsEventPackageMngr);
}


/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::Disconnect(WORD mode,WORD cause,const char* alternativeAddrStr,DWORD disconnectionDelay)
{
	CMedString str;
	str << "Cause " << cause << " mode " << mode << " name - " << m_partyConfName;
	PTRACE2PARTYID(eLevelInfoNormal,"CSipDelPartyCntl::Disconnect: ",str.GetString(), GetPartyRsrcId());

	m_disconnectionCause = cause;

	if(m_type == DIALOUT){
		if(m_lyncRedialOutAttempt==0){
			if(cause==SipPseudoCodesNotAcceptedInHereLyncDialOut || cause== SipPseudoCodesUnsuppMediaTypeDialOut){
				m_lyncRedialOutAttempt++;
				if(cause==SipPseudoCodesNotAcceptedInHereLyncDialOut)
				{
					m_disconnectionCause=SipCodesNotAcceptedInHere;
				}
				else if(cause==SipPseudoCodesUnsuppMediaTypeDialOut)
				{
					m_disconnectionCause=SipCodesUnsuppMediaType;
				}
				TRACEINTO << "LYNC_REDIAL setting m_lyncRedialOutAttempt=1 , m_disconnectionCause = " << m_disconnectionCause << " , " << ::GetRejectReasonStr((enSipCodes)m_disconnectionCause);
			}
		}else if(m_lyncRedialOutAttempt==1){
			m_lyncRedialOutAttempt++;
			TRACEINTO << "LYNC_REDIAL setting m_lyncRedialOutAttempt=2";
		}else if(m_lyncRedialOutAttempt==2){
			//BRIDGE-10389: clear  the flag after the remote connected
			if((cause != SipPseudoCodesNotAcceptedInHereLyncDialOut  && cause != SipCodesNotAcceptedInHere && cause != SipPseudoCodesUnsuppMediaTypeDialOut && cause != SipCodesUnsuppMediaType)){
				m_lyncRedialOutAttempt = 0;
				TRACEINTO << "LYNC_REDIAL resetting m_lyncRedialOutAttempt after remote was connected";
			}
		}else{
			TRACEINTO << "LYNC_REDIAL wrong m_lyncRedialOutAttempt = " << m_lyncRedialOutAttempt;
		}
	}

	if (alternativeAddrStr) 
	{
		m_strAlternativeAddr = alternativeAddrStr;

		if (m_disconnectionCause==SIP_MOVED_PERMANENTLY || m_disconnectionCause == SIP_REDIRECTION_300)
		{
			((CSipNetSetup*)m_pSIPNetSetup)->SetRemoteSipAddress(alternativeAddrStr);
			if (m_type == DIALOUT)
				m_pSIPNetSetup->SetDestPartyAddress(alternativeAddrStr);
		}
	}
	else if // disconnection cause is not tcp transport error or busy with redial we delete the alternative address 
		// and return to the original one.
		(m_disconnectionCause != SIP_BUSY_HERE || m_pConf->GetCommConf()->GetIsAutoRedial()==NO)
			m_strAlternativeAddr = "";

	if( eTipMasterCenter == m_TipPartyType  && (m_SlaveAuxRsrcId || m_SlaveLeftRsrcId || m_SlaveRightRsrcId))
	{
		CSegment* pSeg = new CSegment;
		*pSeg << mode;
	//	SendDisconnectMessageFromMasterToSlaves();
		m_DisConnectionModeForMasterInTip = mode;
		TRACEINTO << " CSipDelPartyCntl::Disconnect : TIP master party " << m_TipMasterName << " - start timer for slaves disconnect ";

		StartTimer(DISCONNECT_SLAVE_ACK_TOUT, SECOND*6, pSeg);
	//	return;
	}
	BYTE bContinueDisconnect = CheckDisconnectCases(mode, disconnectionDelay);
	if (bContinueDisconnect)
		DisconnectParty(mode);	
}

void  CSipDelPartyCntl::SendDisconnectMessageFromMasterToSlaves(CSegment* pParam)
{
	CSegment* pParamLocal = NULL;
	if( m_SlaveAuxRsrcId )
		m_pTaskApi->PartyCntlToPartyMsgFromMasterToSlave(m_SlaveAuxRsrcId, PARTYDISCONNECT, pParamLocal);
	if( m_SlaveLeftRsrcId )
		m_pTaskApi->PartyCntlToPartyMsgFromMasterToSlave(m_SlaveLeftRsrcId, PARTYDISCONNECT, pParamLocal);
	if( m_SlaveRightRsrcId )
		m_pTaskApi->PartyCntlToPartyMsgFromMasterToSlave(m_SlaveRightRsrcId, PARTYDISCONNECT, pParamLocal);
}
/////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::OnTimerDelayDisconnectIdle(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipDelPartyCntl::OnTimerDelayDisconnectIdle : Name - ",m_partyConfName, GetPartyRsrcId());
	WORD mode = 0;
	
	*pParam >> mode;
	
	DisconnectParty(mode);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::DisconnectParty(WORD mode)
{
	PTRACE2PARTYID(eLevelError,"CSipDelPartyCntl::DisconnectParty : Name - ",m_partyConfName, GetPartyRsrcId());
	
	CPartyCntl::Disconnect(mode); 

	if (m_disconnectState == DISCONNECTED) 
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::DisconnectParty: m_disconnectState == DISCONNECTED.", GetPartyRsrcId());
		UpdateConfEndDisconnect(statOK);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipDelPartyCntl::DisconnectParty: m_connectingState == ", m_connectingState);
		m_connectingState = IP_DISCONNECTING;
		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_DISCONNECTING);
		 // remove from terminal list only if it was numerated, in case status is not OK - We rejected the call
    	// and there is no need to remove from numbering list.
    	// If party was disocnnected during Move Process and after 
        if(m_isPartyEndRAMoveOK)
            SetDataForExportPartyFail();
	    if (m_pTerminalNumberingManager && m_isTerminalNumberingConn)
   		{
   			STATUS removeStatus = STATUS_OK;
			removeStatus = m_pTerminalNumberingManager->Remove(m_pParty);
			if (removeStatus != STATUS_OK)
				PASSERTMSG(removeStatus,"CSipDelPartyCntl::DisconnectParty - Party is not in the m_pTerminalNumberingManager list");
			OFF(m_isTerminalNumberingConn);
   		}
	    if(DisconnectMsSlavesIfNeeded() == FALSE)
	    	DisconnectBridges();
	    else
	    	PTRACE(eLevelInfoNormal,"CSipDelPartyCntl::DisconnectParty: av-mcu delte slaves ");
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::DisconnectBridges()
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::DisconnectBridges", GetPartyRsrcId());

	if ((m_eAudBridgeConnState != eBridgeDisconnected) || (m_eVidBridgeConnState != eBridgeDisconnected) || m_isFeccConn || m_isContentConn)
	{
		DWORD ticks = BRIDGES_DISCONNECT_TIME;
		if (m_pConf->GetCommConf()->IsRollCall())
			ticks +=15; //32 delay for the video bridge (because: max 8 seconds delay for a party + max delay for 4 parties).
		StartTimer(BRIDGEDISCONNECT, ticks * SECOND);
		
		if (m_eVidBridgeConnState != eBridgeDisconnected)
			DisconnectPartyFromVideoBridge();

		if (m_isFeccConn)
			DisconnectPartyFromFECCBridge();
		
		if (m_isContentConn)
			DisconnectPartyFromContentBridge();

		if (m_eAudBridgeConnState != eBridgeDisconnected)
			DisconnectPartyFromAudioBridge();
		
		
	}
	
	else
		BridgeDisconnetCompleted();
}


/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnAudioBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnAudioBrdgDisconnected", GetPartyRsrcId());
	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam); // the status is not relevant here - we just turn on the m_isFaulty flag
	BridgeDisconnetCompleted();   
}


/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnVideoBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnVideoBrdgDisconnected", GetPartyRsrcId());
	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam); // the status is not relevant here - we just turn on the m_isFaulty flag
	BridgeDisconnetCompleted();   
}
/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnXCodeBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnXCodeBrdgDisconnected", GetPartyRsrcId());
	CPartyCntl::OnXCodeBrdgDisconnected(pParam);
	BridgeDisconnetCompleted();
}
/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnFeccBridgeDisConnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnFeccBridgeDisConnect", GetPartyRsrcId());
	CIpPartyCntl::OnFeccBridgeDisConnect(pParam);
	BridgeDisconnetCompleted();   
}
/////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::OnPartyPcmStateChangedIdle(CSegment* pParam)
{
	CPartyCntl::OnPartyPcmStateChangedAnycase(pParam);

	PcmDisconnectionCompleted();
}
/////////////////////////////////////////////////////////////////////////////
int CSipDelPartyCntl::OnContentBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnContentBrdgDisconnected", GetPartyRsrcId());
	CPartyCntl::OnContentBrdgDisconnected(pParam);
	BridgeDisconnetCompleted();

	return 0;

}
/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::BridgeDisconnetCompleted()
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::BridgeDisconnetCompleted", GetPartyRsrcId());

	TRACEINTO << "AudBridgeConn "   << (WORD)m_bIsMemberInAudBridge
			<< "  VidBridgeConn "     << (WORD)m_bIsMemberInVidBridge
			<< "  FeccBridgeConn "    << (WORD)m_isFeccConn
			<< "  ContentBridgeConn " << (WORD)m_isContentConn
			<< "  XCodeBridgeConn "   << (WORD)m_isXCodeConn;
	
	if (!m_bIsMemberInAudBridge && !m_bIsMemberInVidBridge && !m_isFeccConn && !m_isContentConn && !m_isXCodeConn)
	{
		if (IsValidTimer(BRIDGEDISCONNECT)) 
			DeleteTimer(BRIDGEDISCONNECT);
		
		m_pConfAppMngrInterface->RemovePartyFromCAM(GetPartyRsrcId(), GetName());

		if (m_isPcmConnected)
		{
			PTRACE(eLevelInfoNormal,"CH323DelPartyCntl::BridgeDisconnetCompleted wait 5 more seconds for pcm disconnection");
			StartTimer(PCM_DISCONNECTPARTY_TOUT,5*SECOND);
		}

		// wait for pcm disconnection
		PcmDisconnectionCompleted();
	}
}
/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::PcmDisconnectionCompleted()
{
	if (!m_bIsMemberInAudBridge && !m_bIsMemberInVidBridge && !m_isFeccConn && !m_isPcmConnected)
	{
		if (IsValidTimer(PCM_DISCONNECTPARTY_TOUT))
		{
			PTRACE(eLevelInfoNormal,"CSipDelPartyCntl::PcmDisconnectionCompleted");
			DeleteTimer(PCM_DISCONNECTPARTY_TOUT);
		}

		//party cleanup on exception
		if(GetIsViolentDestroy())
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
}
/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::DestroyParty()
{
	m_state = DESTROY_PARTY;

	if (m_pPartyApi && CPObject::IsValidPObjectPtr(m_pPartyApi))
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::DestroyParty: (Disconnect to party)", GetPartyRsrcId());
		int timer = GetSystemCfgFlagInt<DWORD>(CFG_KEY_SIP_MSG_TIMEOUT);
		StartTimer(PARTYDISCONNECTTOUT, (timer+MINUTE)*SECOND);
		if( FALSE == m_IsPartyDisconnectEnded )
			m_pPartyApi->Destroy();
		else
		{
			CSegment *pParam = new CSegment;
			*pParam << m_disconnectionCause;
			DispatchEvent(PARTYENDDISCONNECT, pParam);
			POBJDELETE(pParam);
		}
		POBJDELETE(m_pPartyApi);
	} 
	else if ( IsValidTimer(PARTYDISCONNECTTOUT) == NO)
	{
		if (m_pPartyHWInterface)
		{
			PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::DestroyParty: (Disconnect Hardware)", GetPartyRsrcId());
			DeletePartyFromHW();
		}
		else
		{
			PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::DestroyParty: (Disconnect Resources)", GetPartyRsrcId());
			DeallocatePartyResources();
		}
	}
	else
		PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::DestroyParty: (No Disconnect Process)", GetPartyRsrcId());
}


/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnPartyDisconnetedIdle(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::OnPartyDisconnetedIdle", GetPartyRsrcId());
	WORD  status = 0;
	*pParam >> status;
	m_disconnectionCause = status;
	m_IsPartyDisconnectEnded = TRUE;
}

/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnPartyDisconneted(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::OnPartyDisconneted", GetPartyRsrcId());	
	WORD  status = 0;
	*pParam >> status;
	m_disconnectionCause = status;

	if (IsValidTimer(PARTYDISCONNECTTOUT))
		DeleteTimer(PARTYDISCONNECTTOUT);

	if( IsValidTimer(DISCONNECT_SLAVE_ACK_TOUT) && eTipMasterCenter == m_TipPartyType)
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::OnPartyDisconneted - timer for slave still valid -need to handle! -TBD", GetPartyRsrcId());
		DisconnectSlavesIfNeeded();
	}

	CConfParty* pConfParty = m_pConf->GetCommConf()->GetCurrentParty(m_name);

	if(pConfParty && eMsftAvmcuNone != pConfParty->GetMsftAvmcuState() && (m_pMsFocusMngr || m_pMsEventPackageMngr)) // Incase this is AVMCU invation we will stop the flow here...
	{
		TRACEINTO<<" AVMCU party " ;

		PTRACE2INT(eLevelInfoNormal,"CSipDelPartyCntl::OnPartyDisconneted  - m_EndFocus: ", m_EndFocus);
		PTRACE2INT(eLevelInfoNormal,"CSipDelPartyCntl::OnPartyDisconneted  - m_EndSubscriber: ", m_EndSubscriber);

		m_state = DELETE_FROM_MNGR;
		if (m_pMsFocusMngr && !m_EndFocus)
		{
			TRACEINTO<<" Terminate Focus";
			m_pMsFocusMngr->TerminateFocusConnection();
		}

		if (m_pMsEventPackageMngr && !m_EndSubscriber)
		{
			TRACEINTO<<" Terminate EventPackage";
			m_pMsEventPackageMngr->TerminateEventPackageConnection(m_pSIPNetSetup);
		}
	}
	else
	{
		DeletePartyFromHW();
	}
}

////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::DeletePartyFromHW()
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::DeletePartyFromHW", GetPartyRsrcId());


	if (m_pPartyHWInterface && CPObject::IsValidPObjectPtr(m_pPartyHWInterface))
	{
		m_state = DELETE_FROM_HW;
		StartTimer(MPLDISCONNECTTOUT, MPL_DISCONNECT_TIME * SECOND);

		DWORD isNeedToCollectInfoFromArt = (IsMsSlaveIn() && m_pIpCurrentMode->GetIsEncrypted() ) ? YES : NO ;
		m_pPartyHWInterface->SendDeleteParty(isNeedToCollectInfoFromArt);
		PTRACE2INT(eLevelInfoNormal,"CSipDelPartyCntl::DeletePartyFromHW  - isNeedToCollectInfoFromArt: ", isNeedToCollectInfoFromArt);
	}
	else
	{
		PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::DeletePartyFromHW - don't send message", GetPartyRsrcId());
		DeallocatePartyResources();
	}
}


/////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::OnMplAckDeleteFromHw(CSegment* pParam)
{
	OPCODE	AckOpcode;
	DWORD  ack_seq_num;
	STATUS  status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	
	if (AckOpcode==CONF_MPL_DELETE_PARTY_REQ)
	{
		PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnMplAckDeleteFromHw : Delete Ack Msg Received", GetPartyRsrcId());    

		if (IsValidTimer(MPLDISCONNECTTOUT))
			DeleteTimer(MPLDISCONNECTTOUT);

		if (status != STATUS_OK)
		{
			PTRACEPARTYID(eLevelError, "CH323DelPartyCntl::OnMplAckDeleteFromHw : Delete failed", GetPartyRsrcId());
			m_isFaulty = 1; //Invoking KillPort process in RA.
		}
		
		POBJDELETE(m_pPartyHWInterface);
		DeallocatePartyResources();
	}
	else
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipDelPartyCntl::OnMplAckDeleteFromHw : invalid opcode", GetPartyRsrcId());
		if (AckOpcode)	
			DBGPASSERT(AckOpcode);
		else
			DBGPASSERT(101);
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::OnTimerMplDisconnectDeleteFromHw(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipDelPartyCntl::OnTimerMplDisconnectDeleteFromHw", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	m_isFaulty = 1;
	DeallocatePartyResources();
}

/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::DeallocatePartyResources()
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::DeallocatePartyResources", GetPartyRsrcId());           

	DWORD	rtp_IceChannels[MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS];
	DWORD 	rtcp_IceChannels[MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS];

	for(int i=0;i<MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS;i++)
	{
		rtp_IceChannels[i] = 0;
		rtcp_IceChannels[i] = 0;
	}

//	m_IceParams->Dump("SetIceParams");

	if(m_pPartyAllocatedRsrc && (m_pPartyAllocatedRsrc->GetStatus()==STATUS_OK))
	{
		PTRACE2INT(eLevelInfoNormal,"CSipDelPartyCntl::DeallocatePartyResources  - Is ICE: ", m_IsIceParty);
		PTRACE2INT(eLevelInfoNormal,"CSipDelPartyCntl::DeallocatePartyResources  - m_isFaulty: ", m_isFaulty );

		m_state = DEALLOCATE_RSC;
		StartTimer(RADISCONNECTTOUT, RA_DISCONNECT_TIME * SECOND);

		if(m_IsIceParty && m_isFaulty && m_IceParams)
		{
			PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::DeallocatePartyResources ICE Party and Faulty", GetPartyRsrcId());

			rtp_IceChannels[0] = m_IceParams->GetAudioRtpId();
			rtcp_IceChannels[0] = m_IceParams->GetAudioRtcpId();

			rtp_IceChannels[1] = m_IceParams->GetVideoRtpId();
			rtcp_IceChannels[1] = m_IceParams->GetVideoRtcpId();

			rtp_IceChannels[2] = m_IceParams->GetDataRtpId();
			rtcp_IceChannels[2] = m_IceParams->GetDataRtcpId();

			rtp_IceChannels[3] = m_IceParams->GetContentRtpId();
			rtcp_IceChannels[3] = m_IceParams->GetContentRtcpId();

		}
		CreateAndSendDeallocatePartyResources(m_pSIPNetSetup->GetEnableSipICE(),rtp_IceChannels,rtcp_IceChannels);
	}
	else
	{
		m_connectingState = DISCONNECTED;	
		PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::DeallocatePartyResources - don't send message", GetPartyRsrcId());		
		EndIpPartyDisconnect();
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnRsrcDeallocatePartyRspDeallocate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnRsrcDeallocatePartyRspDeallocate", GetPartyRsrcId());           

	if (IsValidTimer(RADISCONNECTTOUT))
		DeleteTimer(RADISCONNECTTOUT);

	DWORD  structLen = sizeof(DEALLOC_PARTY_IND_PARAMS_S);
	DEALLOC_PARTY_IND_PARAMS_S  tDeallocatePartyIndParams;
	memset(&tDeallocatePartyIndParams,0,structLen);
	pParam->Get((BYTE*)(&tDeallocatePartyIndParams),structLen);


	CMedString *pStr = new CMedString;
	*pStr <<"DEALLOC_PARTY_IND_PARAMS_S:\n"
		  << "status	   =   "<< CProcessBase::GetProcess()->GetStatusAsString(tDeallocatePartyIndParams.status).c_str();	
	PTRACEPARTYID(eLevelInfoNormal, pStr->GetString(), GetPartyRsrcId());
	POBJDELETE(pStr);

	if (tDeallocatePartyIndParams.status != STATUS_OK)
	{
		PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnRsrcDeallocatePartyRspDeallocate : DEALLOCATION FAILED!!! continue process", GetPartyRsrcId());           
		DBGPASSERT(tDeallocatePartyIndParams.status);
	}

	/* MSSlave Flora comment: for MSSlave, we should nitofy the slavescontroller of the END of DELETE slave PARTY with msg: PARTY_CONTROL_SLAVE_TO_MASTER_ACK here */
	if (IsMsSlaveOut() || IsMsSlaveIn())
	{
		m_pTaskApi->SendMsSlaveToMainAck(m_MasterRsrcId, DELMSSLAVEPARTY, statOK, GetPartyRsrcId(), m_AvMcuLinkType, m_MSSlaveIndex);
	}
	
	//remove party resources from global resource/routing table
	if (::CPObject::IsValidPObjectPtr(m_pPartyAllocatedRsrc))
	{
//		m_pConf->UpdateConfPartyRsrcDeAllocated(m_pPartyAllocatedRsrc->GetPartyRsrcId());
		m_pPartyAllocatedRsrc->DeleteFromGlobalRsrcRoutingTbl();
		POBJDELETE(m_pPartyAllocatedRsrc);
	}
	else 
	{
		PASSERT(101);
	}

	m_connectingState = DISCONNECTED;
	
	EndIpPartyDisconnect();
	
}

/////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::OnTimerBridgesDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnTimerBridgesDisconnect", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	m_isFaulty = 1;
	
	m_pConfAppMngrInterface->RemovePartyFromCAM(GetPartyRsrcId(), GetName());
	
	DestroyParty();
}
/////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::OnTimerPcmDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnTimerPcmDisconnect", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());

	DestroyParty();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipDelPartyCntl::OnTimerPartyDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnTimerPartyDisconnect", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	DeletePartyFromHW();
}

/////////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnTimerRsrcAllocatorDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CSipDelPartyCntl::OnTimerRsrcAllocatorDisconnect", GetPartyRsrcId());           
	DBGPASSERT(GetPartyRsrcId());
	m_connectingState = DISCONNECTED;	
	EndIpPartyDisconnect();
}
/////////////////////////////////////////////////////////////////////////////

/* MSSlave Flora Question: Delete: It is for TipSlave, for MSSlave, maybe MSSlave will not send :DISCONNECT_SLAVE_ACK to Main PartyCntl or SlavesConroller ? */
void CSipDelPartyCntl::OnSlaveToMasterMessage(CSegment* pParam)
{
	DWORD tipPartyType, opcode;
	*pParam >> tipPartyType >> opcode;
	if( opcode != DISCONNECT_SLAVE_ACK )
	{
		TRACEINTO << "CSipDelPartyCntl::OnSlaveToMasterMessage : m_connectingState= " << m_connectingState << ", tipPartyType=" << tipPartyType << ", opcode=" << opcode;
		return;
	}
	switch(tipPartyType)
	{
	case eTipSlaveAux:
		m_SlaveAuxRsrcId = 0;
		break;
	case eTipSlaveLeft:
		m_SlaveLeftRsrcId = 0;
		break;
	case eTipSlaveRigth :
		m_SlaveRightRsrcId = 0;
		break;
	}
	if( 0==m_SlaveAuxRsrcId && 0==m_SlaveLeftRsrcId && 0==m_SlaveRightRsrcId )
	{
		TRACEINTO << "CSipDelPartyCntl::OnSlaveToMasterMessage : TIP master received all ACKs from slaves and continue disconnecting";

		if( IsValidTimer(DISCONNECT_SLAVE_ACK_TOUT) )
			DeleteTimer(DISCONNECT_SLAVE_ACK_TOUT);

	//	DisconnectParty(m_DisConnectionModeForMasterInTip);
	}
}
/////////////////////////////////////////////////////////////////////////////

void CSipDelPartyCntl::OnTimerDisconnectSlavesAck(CSegment* pParam)
{
	PTRACE(eLevelError, "CSipDelPartyCntl::OnTimerDisconnectSlavesAck - DISCONNECT_SLAVE_ACK_TOUT timer time-out, This is not a problem in case Conference was deleted");
	DBGPASSERT(DISCONNECT_SLAVE_ACK_TOUT);
	WORD mode;
	*pParam >> mode;
	TRACEINTO << " CSipDelPartyCntl::OnTimerDisconnectSlavesAck : mode=" << mode;
//	DisconnectParty(mode);
}
void CSipDelPartyCntl::OnSlaveToMasterAckMessage(CSegment* pParam)
{
	DWORD status;
	DWORD peerRsrcId;
	WORD tipPartyType;
	WORD allocated_resolution;
	DWORD m_monitorSlavePartyId;
	WORD reason;

	*pParam >> status
		    >> peerRsrcId
		    >> tipPartyType
		    >> allocated_resolution;

	PTRACE2INT(eLevelInfoNormal,"CSipDelPartyCntl::OnSlaveToMasterAckMessage peerRsrcId ", peerRsrcId);
	PTRACE2INT(eLevelInfoNormal,"CSipDelPartyCntl::OnSlaveToMasterAckMessage tipPartyType ", tipPartyType);
	SendDisconnectSlaveToConf(tipPartyType);
}

void CSipDelPartyCntl::SendDisconnectSlaveToConf(WORD tipPartyType)
{
	char *p_name = new char[H243_NAME_LEN];
	memset(p_name, '\0', H243_NAME_LEN);
	strncpy(p_name, m_TipMasterName, H243_NAME_LEN - 1);

	switch(	tipPartyType)
	{
		case eTipSlaveAux:
			strcat(p_name, "_aux");
			m_SlaveAuxRsrcId = 0;
			m_TipSlaveAuxAddSent = FALSE;

			break;
		case eTipSlaveLeft:
			strcat(p_name, "_2");
			m_SlaveLeftRsrcId = 0;
			m_TipSlaveLeftAddSent = FALSE;
			break;
		case eTipSlaveRigth:
			strcat(p_name, "_3");
			m_SlaveRightRsrcId = 0;
			m_TipSlaveRightAddSent = FALSE;

			break;
	}

	m_pTaskApi->DropParty(p_name, 0, 0);
	PDELETEA(p_name);
}

BOOL CSipDelPartyCntl::GetIsViolentDestroy()
{
	return m_isViolentDestroy;
}
//////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::SetIsViolentDestroy(BOOL isViolent)
{
	m_isViolentDestroy = isViolent;
}
DWORD CSipDelPartyCntl::GetPartyTaskId()
{
	return m_partyTaskId;
}
void  CSipDelPartyCntl::SetPartyTaskId(DWORD taskId)
{
	m_partyTaskId = taskId;
}

//////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::DisconnectSlavesIfNeeded()
{
	if( m_SlaveRightRsrcId || m_TipSlaveRightAddSent)
		SendDisconnectSlaveToConf(eTipSlaveRigth);
	if( m_SlaveLeftRsrcId || m_TipSlaveLeftAddSent)
		SendDisconnectSlaveToConf(eTipSlaveLeft);
	if( m_SlaveAuxRsrcId || m_TipSlaveAuxAddSent)
		SendDisconnectSlaveToConf(eTipSlaveAux);
}


//////////////////////////////////////////////////////////////////////////
BOOL CSipDelPartyCntl::DisconnectMsSlavesIfNeeded()
{
	TRACEINTO;
	if(m_AvMcuLinkType == eAvMcuLinkMain && m_pMsSlavesController)
	{
		PTRACE2PARTYID(eLevelInfoNormal,"CSipDelPartyCntl::DisconnectMsSlavesIfNeeded -sending : Name - ",m_partyConfName, GetPartyRsrcId());
		// message to slaves controller: delete all slaves
		m_state = WAITING_FOR_MSFT_ALL_SLAVES_DELETED;
		m_pMsSlavesController->DeleteAllSlaves();
		StartTimer(DISCONNECT_ALL_MS_SLAVES_TOUT, SECOND*6);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnAllMsSlavesDeleted(CSegment* pParam)
{
    PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnAllMsSlavesDeleted : Name - ",m_partyConfName, GetPartyRsrcId());
    PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnAllMsSlavesDeleted : m_state - ",m_state);
    PASSERT_AND_RETURN(!pParam);
    DeleteTimer(DISCONNECT_ALL_MS_SLAVES_TOUT);

    DWORD status = statOK;
    *pParam >> status;
    m_state = IDLE;

    DisconnectBridges();


}
/////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnAllMsSlavesDeletedTOUT(CSegment* pParam)
{
    PTRACE2PARTYID(eLevelInfoNormal,"CSipPartyCntl::OnAllMsSlavesDeletedTOUT : Name - ",m_partyConfName, GetPartyRsrcId());
    PTRACE2INT(eLevelInfoNormal,"CSipPartyCntl::OnAllMsSlavesDeletedTOUT : m_state - ", m_state);
    PASSERTMSG(m_state,"CSipDelPartyCntl::OnAllMsSlavesDeletedTOUT - no all slaves disconnected");
    m_state = IDLE;
    DisconnectBridges();


}



//////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnMSFocusEndDisConnection(CSegment* pParam)
{
	PTRACE(eLevelError, "CSipDelPartyCntl::OnMSFocusEndDisConnection");
	m_EndFocus = TRUE;

	if( NULL == m_pMsEventPackageMngr || (m_pMsEventPackageMngr && m_EndSubscriber))
	{
		DeletePartyFromHW();
	}
	else
	{
		TRACEINTO << "not delete party";
	}
}

//////////////////////////////////////////////////////////////////////////
void CSipDelPartyCntl::OnMSSubscriberEndDisConnection(CSegment* pParam)
{
	PTRACE(eLevelError, "CSipDelPartyCntl::OnMSSubscriberEndDisConnection");
	m_EndSubscriber = TRUE;

	if(m_EndFocus)
		DeletePartyFromHW();

}
