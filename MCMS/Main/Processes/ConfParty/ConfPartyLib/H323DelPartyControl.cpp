/////////////////////////////////////////////////////////////////////////////
//                        CH323DelPartyCntl
/////////////////////////////////////////////////////////////////////////////

// CH323DelPartyCntl class is used as a controller for the process of disconnecting
// a 323 party from the conference. In case the party is disconnected by the mcu itself
// the droping procedure take place at the CChangeModeCntl control that controls
// the zeroing process and upon completion CDelPartyCntl is invoked for audio control
// and party disconnection.
// In case of remote disconnection zeroing process cannot take place and CDelPartyCntl
// is responsible for bridge controllers disconnection ( video,mlp,data ) .
// CDelPartyCntl take no assumption to which bridge controler it is current connected
// because there can be a state that a request to bridge connection was send but 
// no ack received yet, in this case from the party point of view it is not connected
// but from the bridge point of view the party is in the process of being connected.
// The bridge controllers will be reade to receive disconnect requestes for party
// that are not connected to them and will ack those request with positive ack.
// In case of remote disconnection CDelPartyCntl can be invoked while party is
// process of connecting to conference or in a state of change mode , therefore
// CDelPartyCntl will ignore bridge control events that are relevants to  CAddPartyCntl
// and CChangeModeCntl. 
  
//+========================================================================+
#include "H323DelPartyControl.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyGlobals.h"
#include "Conf.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "H323Caps.h"
#include "H323Scm.h"
#include "ConfDef.h"
#include "StatusesGeneral.h"
#include "AudioBridgeInterface.h"
#include "BridgePartyDisconnectParams.h"
#include "FECCBridge.h"
#include "OpcodesMcmsCommon.h"
#include "TraceStream.h"
#include "OsTask.h"
#include <sys/signal.h>




PBEGIN_MESSAGE_MAP(CH323DelPartyCntl)
  //party events:
  ONEVENT(PARTYENDDISCONNECT,				DESTROY_PARTY,	CH323DelPartyCntl::OnPartyDisconnectDestroyParty)
  ONEVENT(INCREASE_DISCONNECT_TIMER, 		DESTROY_PARTY,	CH323DelPartyCntl::OnPartyIncreaseDisconnctingTimerDestroyParty)
  
  //mpl events:
  ONEVENT(ACK_IND,							DELETE_FROM_HW,	CH323DelPartyCntl::OnMplAckDeleteFromHw)
  ONEVENT(PARTY_FAULTY_RSRC     			,ANYCASE	 ,CH323DelPartyCntl::OnPartyReceivedFaultyRsrc)
  //resource allocator events:
  ONEVENT(DEALLOCATE_PARTY_RSRC_IND,		DEALLOCATE_RSC,	CH323DelPartyCntl::OnRsrcDeallocatePartyRspDeallocate)
  
  //bridges:
  ONEVENT(PARTY_AUDIO_DISCONNECTED,			IDLE,			CH323DelPartyCntl::OnAudBrdgDisconnect)
  ONEVENT(PARTY_VIDEO_DISCONNECTED,			IDLE,			CH323DelPartyCntl::OnVidBrdgDisconnect)
  ONEVENT(FECC_PARTY_BRIDGE_DISCONNECTED,	IDLE,			CH323DelPartyCntl::OnFeccBridgeDisConnect)
  ONEVENT(PARTY_CONTENT_DISCONNECTED,		IDLE,			CH323DelPartyCntl::OnContentBrdgDisconnected)
  //ONEVENT(PARTY_XCODE_DISCONNECTED,        ANYCASE,        CH323DelPartyCntl::NullActionFunction)
  ONEVENT(PARTY_XCODE_DISCONNECTED,         IDLE,           CH323DelPartyCntl::OnXCodeBrdgDisconnected)
  ONEVENT(DISCONNECTDELAY,					IDLE,			CH323DelPartyCntl::OnPartyDelayDisconnectIdle)
  ONEVENT(DISCONNECTDELAY,					ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(PARTY_AUDIO_CONNECTED,			IDLE,			CH323DelPartyCntl::NullActionFunction)
  ONEVENT(PARTY_VIDEO_CONNECTED,			IDLE,			CH323DelPartyCntl::NullActionFunction)
  ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,	IDLE,			CH323DelPartyCntl::NullActionFunction)
  ONEVENT(PARTY_CONTENT_CONNECTED,			IDLE,			CH323DelPartyCntl::NullActionFunction)
  ONEVENT(PCM_PARTY_STATE_CHANGED,			IDLE,			CH323DelPartyCntl::OnPartyPcmStateChangedIdle)
  //resource allocator events:
  ONEVENT(DEALLOCATE_PARTY_RSRC_IND,    IDLE, CH323DelPartyCntl::NullActionFunction)
// ONEVENT(DATCONNECT,						IDLE,			CH323DelPartyCntl::NullActionFunction) 
// ONEVENT(CHAIRDISCONNECT,					IDLE,			CH323DelPartyCntl::OnChairDisConnect)
  
  //timers
  ONEVENT(BRIDGEDISCONNECT,					IDLE,			CH323DelPartyCntl::OnTimerBridgesDisconnect)
  ONEVENT(PCM_DISCONNECTPARTY_TOUT,			IDLE,			CH323DelPartyCntl::OnTimerPcmDisconnect)
  ONEVENT(PARTYDISCONNECTTOUT,				DESTROY_PARTY,	CH323DelPartyCntl::OnTimerPartyDisconnectDestroyParty)
  ONEVENT(MPLDISCONNECTTOUT,       		    DELETE_FROM_HW,	CH323DelPartyCntl::OnTimerMplDisconnectDeleteFromHw)
  ONEVENT(RADISCONNECTTOUT,					DEALLOCATE_RSC,	CH323DelPartyCntl::OnTimerRsrcAllocatorDisconnect)
  
  //party:
  ONEVENT(PARTY_CLOSE_CHANNEL,				ANYCASE,		CH323DelPartyCntl::OnPartyChannelDisconnectAnycase)
  
  ONEVENT(PARTYENDCHANGEMODE, 				ANYCASE,   		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(UPDATE_CAPS,                      ANYCASE,   		CH323DelPartyCntl::NullActionFunction)

  ONEVENT(H323PARTYCONNECTALL,				ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(UPDATE_VIDEO_RATE,				ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(CLEAN_VIDEO_RATE_LIMIT, 			ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(VIDEOMUTE,			            ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(REMOTE_SENT_RE_CAPS,              ANYCASE,        CH323DelPartyCntl::NullActionFunction)
  // LPR
  ONEVENT(LPR_CHANGE_RATE,					ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(PARTY_RECEIVE_ECS,				ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(PARTY_AUDIO_CONNECTED,			ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(H323PARTYCONNECT,   				ANYCASE,		CH323DelPartyCntl::NullActionFunction)
  ONEVENT(SECONDARYCAUSEH323,		        ANYCASE,        CH323DelPartyCntl::NullActionFunction)
  ONEVENT(CHANGE_CONTENT_BIT_RATE_BY_LPR,   ANYCASE, 		CH323DelPartyCntl::NullActionFunction)// VNGFE-8204

  //Multiple links for ITP in cascaded conference feature:
  ONEVENT(ITPSPEAKERIND,                    ANYCASE,        CH323DelPartyCntl::OnMainPartyDoNothing) //error handling..
  ONEVENT(ITPSPEAKERACKIND,                 ANYCASE,        CH323DelPartyCntl::OnMainPartyDoNothing) //error handling..
  ONEVENT(NEW_ITP_SPEAKER_IND,              ANYCASE,        CH323DelPartyCntl::OnMainPartyDoNothing) //msg from VB - error handling...

    //Cop
    ONEVENT(COP_VIDEO_IN_CHANGE_MODE   ,ANYCASE			    ,CH323DelPartyCntl::NullActionFunction)
    ONEVENT(COP_VIDEO_OUT_CHANGE_MODE   ,ANYCASE			,CH323DelPartyCntl::NullActionFunction)  
    ONEVENT(COP_UPDATE_CASCADE_LINK_LECTURE_MODE   ,ANYCASE			,CH323DelPartyCntl::NullActionFunction)
    ONEVENT(PARTYEXPORT,						ANYCASE,			CH323DelPartyCntl::OnPartyExport)
PEND_MESSAGE_MAP(CH323DelPartyCntl,CH323PartyCntl);   


/////////////////////////////////////////////////////////////////////////////
CH323DelPartyCntl::CH323DelPartyCntl() // constructor
{
	 m_isViolentDestroy   				= FALSE;
	 m_partyTaskId						= 0;
	VALIDATEMESSAGEMAP;
}
  
/////////////////////////////////////////////////////////////////////////////
CH323DelPartyCntl::~CH323DelPartyCntl() // destructor
{
}
  
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void*  CH323DelPartyCntl::GetMessageMap()                                        
{
	return (void*)m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
CH323DelPartyCntl& CH323DelPartyCntl::operator=(const CH323PartyCntl& other)
{
	(CH323PartyCntl&)*this = (CH323PartyCntl&)other;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
CH323DelPartyCntl& CH323DelPartyCntl::operator=(const CH323DelPartyCntl& other)
{
	(CH323PartyCntl&)*this = (CH323PartyCntl&)other;
	return *this;
}

///////////////////////////////////////////////////////////////////////////////shiraITP - 126
void  CH323DelPartyCntl::DisconnectH323(WORD mode,DWORD disconnectionDelay)                                        
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323DelPartyCntl::DisconnectH323 : Name - ", m_partyConfName, GetPartyRsrcId()); 
	m_bSuspendVideoUpdates = FALSE;
	BYTE bContinueDisconnect = CheckDisconnectCases(mode, disconnectionDelay);
	if (bContinueDisconnect)
		DisconnectParty(mode); //shiraITP - 127
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::DisconnectParty(WORD mode)	
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323DelPartyCntl::DisconnectParty : Name - ",m_partyConfName, GetPartyRsrcId());
	if(m_linkType == eMainLinkParty)
	    POBJDELETE(m_roomControl);

	CPartyCntl::Disconnect(mode); 
    
	if (m_disconnectState == DISCONNECTED)
	{
		PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::m_disconnectState == DISCONNECTED", GetPartyRsrcId());
		UpdateConfEndDisconnect(statOK);
		return;
	}      
    // If party was disocnnected during Move Process and after 
    if(m_isPartyEndRAMoveOK)
       SetDataForExportPartyFail();
	if (mode == DISCONNECT_N_RECOVER_MODE)  // disconnect & recover
		ON(m_isRecover);
	
	m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_DISCONNECTING);      
	
	 // remove from terminal list only if it was numerated, in case status is not OK - We rejected the call
    // and there is no need to remove from numbering list.
    if (m_pTerminalNumberingManager && m_isTerminalNumberingConn)
   	{
   		STATUS removeStatus = STATUS_OK;
		removeStatus = m_pTerminalNumberingManager->Remove(m_pParty);
		if (removeStatus != STATUS_OK)
			PASSERTMSG(removeStatus,"CH323DelPartyCntl::DisconnectParty - Party is not in the m_pTerminalNumberingManager list");
		OFF(m_isTerminalNumberingConn);
   	}
	
	//Send Disconnect to bridges:	
 	if ((m_eAudBridgeConnState != eBridgeDisconnected) || (m_eVidBridgeConnState != eBridgeDisconnected) || m_isFeccConn || m_isContentConn)
	{
		DWORD ticks = BRIDGES_DISCONNECT_TIME;
		if (m_pConf->GetCommConf()->IsRollCall())
			ticks +=15; //32 delay for the video bridge (because: max 8 seconds delay for a party + max delay for 4 parties). So I add a little bit.
		StartTimer(BRIDGEDISCONNECT, ticks * SECOND);
	
		if (m_eVidBridgeConnState != eBridgeDisconnected)
			DisconnectPartyFromVideoBridge();
		
		if (m_isFeccConn)
			DisconnectPartyFromFECCBridge();
		
		// "disconnect audio" will be the last msg sent, in oreder to
		// avoid noises at disconnection.
		if (m_eAudBridgeConnState != eBridgeDisconnected)
			DisconnectPartyFromAudioBridge();
			
		if (m_isContentConn)
			DisconnectPartyFromContentBridge();
	}
	else
		BridgeDisconnetCompleted();

}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnPartyDelayDisconnectIdle(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnPartyDelayDisconnectIdle : Name - ",m_partyConfName, GetPartyRsrcId());
	WORD mode = 0;
	
	*pParam >> mode;

	DisconnectParty(mode);

}	
/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnAudBrdgDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnAudBrdgDisconnect", GetPartyRsrcId());
	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam); // the status is not relevant here - we just turn on the m_isFaulty flag
	BridgeDisconnetCompleted();  
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnVidBrdgDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnVidBrdgDisconnect", GetPartyRsrcId());
	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam); // the status is not relevant here - we just turn on the m_isFaulty flag
	BridgeDisconnetCompleted();   
}

/////////////////////////////////////////////////////////////////////////////
int  CH323DelPartyCntl::OnContentBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnContentBrdgDisconnected", GetPartyRsrcId());
	CH323PartyCntl::OnContentBrdgDisconnected(pParam);
	BridgeDisconnetCompleted();

    return 0;
}
/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnXCodeBrdgDisconnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnXCodeBrdgDisconnected", GetPartyRsrcId());
	CPartyCntl::OnXCodeBrdgDisconnected(pParam);
	BridgeDisconnetCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnFeccBridgeDisConnect(CSegment* pParam)
{
	CH323PartyCntl::OnFeccBridgeDisConnect(pParam);
	BridgeDisconnetCompleted();   
}
/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnPartyPcmStateChangedIdle(CSegment* pParam)
{
	CPartyCntl::OnPartyPcmStateChangedAnycase(pParam);

	PcmDisconnectionCompleted();
}
/////////////////////////////////////////////////////////////////////////////
/*void  CH323DelPartyCntl::OnChairDisConnect(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnChairDisConnect : Name - ",m_partyConfName);
	WORD  status;
	*pParam >> status;
	DBGPASSERT(status);
	OFF(m_isChairConn);
	BridgeDisconnetCompleted();
}*/

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::BridgeDisconnetCompleted()                                        
{
	if ( !m_bIsMemberInAudBridge && !m_bIsMemberInVidBridge && ! m_isFeccConn && ! m_isContentConn && !m_isChairConn && !m_isXCodeConn)
	{
	    PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::BridgeDisconnetCompleted - all bridges are disconnected", GetPartyRsrcId());

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
void  CH323DelPartyCntl::PcmDisconnectionCompleted()
{
	if ( !m_bIsMemberInAudBridge && !m_bIsMemberInVidBridge && ! m_isFeccConn && ! m_isContentConn && !m_isChairConn && !m_isPcmConnected)
	{
		if (IsValidTimer(PCM_DISCONNECTPARTY_TOUT))
		{
			PTRACE(eLevelInfoNormal,"CH323DelPartyCntl::PcmDisconnectionCompleted");
			DeleteTimer(PCM_DISCONNECTPARTY_TOUT);
		}


		if (m_pPartyApi)  
		{
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
		else if (m_pPartyHWInterface)
			DeletePartyFromHW();
		else
			DeallocatePartyResources();
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::DestroyParty()
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::DestroyParty", GetPartyRsrcId());           

	m_state = DESTROY_PARTY;
	if (m_pPartyApi && CPObject::IsValidPObjectPtr(m_pPartyApi))
	{
	    StartTimer(PARTYDISCONNECTTOUT, 115*SECOND);//timers in H323Cntl: 60+50
		m_pPartyApi->Destroy();
		POBJDELETE(m_pPartyApi);
	}
	else 
		DeletePartyFromHW();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnPartyDisconnectDestroyParty(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnPartyDisconnectDestroyParty", GetPartyRsrcId());
	
	WORD  status;
	*pParam >> status;
	m_disconnectionCause = status;

	if (IsValidTimer(PARTYDISCONNECTTOUT))
		DeleteTimer(PARTYDISCONNECTTOUT);

	DeletePartyFromHW();
}

////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::DeletePartyFromHW()
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::DeletePartyFromHW", GetPartyRsrcId());

	if (m_pPartyHWInterface && CPObject::IsValidPObjectPtr(m_pPartyHWInterface))
	{
		m_state = DELETE_FROM_HW;
		StartTimer(MPLDISCONNECTTOUT, MPL_DISCONNECT_TIME * SECOND);
		m_pPartyHWInterface->SendDeleteParty();
	}
	else
	{
		PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::DeletePartyFromHW - don't send message", GetPartyRsrcId());
		DeallocatePartyResources();
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnMplAckDeleteFromHw(CSegment* pParam)
{
	OPCODE	AckOpcode;
	DWORD  ack_seq_num;
	STATUS  status;
	*pParam >> AckOpcode >> ack_seq_num >> status;
	
	if (AckOpcode==CONF_MPL_DELETE_PARTY_REQ)
	{
		PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::OnMplAckDeleteFromHw : Delete Ack Msg Received", GetPartyRsrcId());

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
		PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnMplAckDeleteFromHw : invalid opcode", GetPartyRsrcId());
		if (AckOpcode)	
			DBGPASSERT(AckOpcode);
		else
			DBGPASSERT(GetPartyRsrcId());
	}
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnPartyIncreaseDisconnctingTimerDestroyParty(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnPartyIncreaseDisconnctingTimerDestroyParty", GetPartyRsrcId());
	if (IsValidTimer(PARTYDISCONNECTTOUT))
		DeleteTimer(PARTYDISCONNECTTOUT);
	StartTimer(PARTYDISCONNECTTOUT, 90*SECOND);//timers in H323Cntl: 30+60
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnTimerPartyDisconnectDestroyParty(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323DelPartyCntl::OnTimerPartyDisconnectDestroyParty", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	DeletePartyFromHW();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnTimerMplDisconnectDeleteFromHw(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323DelPartyCntl::OnTimerMplDisconnectDeleteFromHw", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	m_isFaulty = 1;
	POBJDELETE(m_pPartyHWInterface);
	DeallocatePartyResources();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnTimerBridgesDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323DelPartyCntl::OnTimerBridgesDisconnect", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());
	m_isFaulty = 1;
	
	m_pConfAppMngrInterface->RemovePartyFromCAM(GetPartyRsrcId(), GetName());
	
	if (m_pPartyApi)  
		DestroyParty();
	else if (m_pPartyHWInterface)
		DeletePartyFromHW();
	else
		DeallocatePartyResources();

	// VNGFE-6353 : Reset conditions
	OFF(m_bIsMemberInAudBridge);
	OFF(m_bIsMemberInVidBridge);
	OFF(m_isFeccConn);
	OFF(m_isContentConn);
	OFF(m_isChairConn);
}
/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnTimerPcmDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323DelPartyCntl::OnTimerPcmDisconnect", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());

	if (m_pPartyApi)
		DestroyParty();
	else if (m_pPartyHWInterface)
		DeletePartyFromHW();
	else
		DeallocatePartyResources();
}
/////////////////////////////////////////////////////////////////////////////
void CH323DelPartyCntl::DeallocatePartyResources()
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::DeallocatePartyResources", GetPartyRsrcId());           

	if(m_pPartyAllocatedRsrc && (m_pPartyAllocatedRsrc->GetStatus()==STATUS_OK))
	{
		m_state = DEALLOCATE_RSC;
		StartTimer(RADISCONNECTTOUT, RA_DISCONNECT_TIME * SECOND);		
		CreateAndSendDeallocatePartyResources();
	}
	else
	{
		PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::DeallocatePartyResources - party was not allocated - send message anyway", GetPartyRsrcId());
		CreateAndSendDeallocatePartyResources(FALSE, NULL,NULL, FALSE); //VNGR-23685
		EndIpPartyDisconnect();
	}
}

/////////////////////////////////////////////////////////////////////////////
//remove party resources from global resource/routing table
//call function EndPartyDisconnect
void CH323DelPartyCntl::OnRsrcDeallocatePartyRspDeallocate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::OnRsrcDeallocatePartyRspDeallocate", GetPartyRsrcId());

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
		PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::OnRsrcDeallocatePartyRspDeallocate : DEALLOCATION FAILED!!! continue process", GetPartyRsrcId());           
		DBGPASSERT(tDeallocatePartyIndParams.status);
	}
		
	//remove party resources from global resource/routing table
	if (::CPObject::IsValidPObjectPtr(m_pPartyAllocatedRsrc))
	{
		//m_pConf->UpdateConfPartyRsrcDeAllocated(m_pPartyAllocatedRsrc->GetPartyRsrcId());//not needed since spreading
		m_pPartyAllocatedRsrc->DeleteFromGlobalRsrcRoutingTbl();
		POBJDELETE(m_pPartyAllocatedRsrc);
	}
	else 
		PASSERTMSG(GetPartyRsrcId(),"CH323DelPartyCntl::OnRsrcDeallocatePartyRspDeallocate - m_pPartyAllocatedRsrc not valid");
	
	EndIpPartyDisconnect();
}

/////////////////////////////////////////////////////////////////////////////
void CH323DelPartyCntl::OnTimerRsrcAllocatorDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal, "CH323DelPartyCntl::OnTimerRsrcAllocatorDisconnect", GetPartyRsrcId());           
	DBGPASSERT(GetPartyRsrcId());
	EndIpPartyDisconnect();
}


/////////////////////////////////////////////////////////////////////////////
void  CH323DelPartyCntl::OnPartyChannelDisconnectAnycase(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CH323DelPartyCntl::OnPartyChannelDisconnectAnycase - do nothing : Name - ",m_partyConfName, GetPartyRsrcId());
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Multiple links for ITP in cascaded conference feature: CH323DelPartyCntl::OnMainPartyDoNothing
void  CH323DelPartyCntl::OnMainPartyDoNothing(CSegment* pParam)
{
    PTRACE(eLevelError,"CH323DelPartyCntl::OnMainPartyDoNothing -ERROR- party is deleted");
}
/////////////////////////////////////////////////////////////////////////

void CH323DelPartyCntl::OnPartyReceivedFaultyRsrc(CSegment* pSeg)
{
	PTRACE2(eLevelInfoNormal,"CH323DelPartyCntl::OnPartyReceivedFaultyRsrc: Name - ",m_partyConfName);
	m_isFaulty = 1;
	DWORD reason;
	*pSeg >> reason;
	if (reason == STATUS_FAIL)
	{
		DWORD MpiErrorNumber = GetMpiErrorNumber(pSeg);
		//m_pTaskApi->PartyDisConnect(MCU_INTERNAL_PROBLEM,m_pParty,NULL,MpiErrorNumber);
	    CSmallString str;
	    str << "CH323DelPartyCntl::OnPartyReceivedFaultyRsrc - in the process of disconnecting party:" << GetPartyRsrcId();
	    PASSERTMSG(MpiErrorNumber, str.GetString());
	}
	else
	{
		//m_pTaskApi->PartyDisConnect(reason,m_pParty);
		PTRACE2(eLevelInfoNormal,"CH323DelPartyCntl::OnPartyReceivedFaultyRsrc(no status fail! : Name - ",m_partyConfName);
	}
	
}
//////////////////////////////////////////////////////////////////////////
BOOL CH323DelPartyCntl::GetIsViolentDestroy()
{
	return m_isViolentDestroy;
}
//////////////////////////////////////////////////////////////////////////
void CH323DelPartyCntl::SetIsViolentDestroy(BOOL isViolent)
{
	m_isViolentDestroy = isViolent;
}
DWORD CH323DelPartyCntl::GetPartyTaskId()
{
	return m_partyTaskId;
}
void  CH323DelPartyCntl::SetPartyTaskId(DWORD taskId)
{
	m_partyTaskId = taskId;
}
void  CH323DelPartyCntl::OnPartyExport(CSegment* pParam)
{
	// if we arrived here this probably means that the party was in the middle of disconnection when it got PARTYEXPORT
	DBGPASSERT_AND_RETURN(1);
}

