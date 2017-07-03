
//+========================================================================+
//                            H323ExportPartyControl.cpp                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       H323ExportPartyCntl.cpp                                     |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 11/11/05   |  Create + Stage 1 - Move between same conferences    |
//+========================================================================+

#include "H323ExportPartyCntl.h"
#include "ConfPartyOpcodes.h"
#include "StatusesGeneral.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "ConfPartyRoutingTable.h"
#include "ConfStructs.h"
#include "MplMcmsStructs.h"
//#include "ConfPartyDefines.h"
#include "AudioBridgePartyCntl.h"
#include "VideoBridgePartyCntl.h"
#include "BridgeMoveParams.h"
#include "BridgePartyExportParams.h"
#include "AudioBridgeInterface.h"
#include "VideoBridgeInterface.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsAudio.h"
#include "H323Caps.h"
#include "TraceStream.h"
#include "ContentBridge.h"

#define PARTYCNTL_CHANGERATE_TIME CHANGERATE_TIME - SECOND


 
PBEGIN_MESSAGE_MAP(CH323ExportPartyCntl)

  ONEVENT(PCM_PARTY_STATE_CHANGED,			IDLE,					CH323ExportPartyCntl::OnPartyPcmStateChangedIdle)
  ONEVENT(START_PARTY_MOVE_RSRC_IND,		EXPORT_RESOURCE,		CH323ExportPartyCntl::OnEndResourceAllocatorStartMove)
  ONEVENT(ACK_IND,							EXPORT_MPL,				CH323ExportPartyCntl::OnMplApiMoveAck)
  ONEVENT(END_PARTY_MOVE_RSRC_IND,			EXPORT_RESOURSE_END,	CH323ExportPartyCntl::OnResourceAllocatorEndMove)
  ONEVENT(PARTY_AUDIO_EXPORTED,				EXPORT_BRIDGES,			CH323ExportPartyCntl::OnAudioBridgeExported)
  ONEVENT(PARTY_VIDEO_EXPORTED,				EXPORT_BRIDGES,			CH323ExportPartyCntl::OnVideoBridgeExported)
  ONEVENT(FECC_PARTY_BRIDGE_DISCONNECTED,	EXPORT_BRIDGES, 		CH323ExportPartyCntl::OnFeccBridgeDisConnect)
  ONEVENT(PARTY_CONTENT_DISCONNECTED,        EXPORT_BRIDGES, 		CH323ExportPartyCntl::OnContentBridgeDisConnect)
  ONEVENT(PARTY_CONTENT_DISCONNECTED,    	ANYCASE,	            CH323ExportPartyCntl::OnContentBridgeDisConnectAnyCase)
  ONEVENT(PARTY_AUDIO_DISCONNECTED,			EXPORT_BRIDGES,   		CH323ExportPartyCntl::OnAudBrdgDisconnect)
  ONEVENT(PARTY_VIDEO_DISCONNECTED,			EXPORT_BRIDGES,   		CH323ExportPartyCntl::OnVidBrdgDisconnect)
  ONEVENT(PARTY_XCODE_DISCONNECTED,         IDLE,                   CH323ExportPartyCntl::OnXCodeBrdgDisconnectIdle)
  ONEVENT(PARTY_XCODE_DISCONNECTED,         EXPORT_BRIDGES,         CH323ExportPartyCntl::OnXCodeBrdgDisconnectExportBrdgs)
  ONEVENT(PARTY_XCODE_DISCONNECTED,         ANYCASE,                CH323ExportPartyCntl::OnXCodeBrdgDisconnectAnyCase)
  ONEVENT(PARTYEXPORT,						EXPORT_PARTY,			CH323ExportPartyCntl::OnPartyExport)
  ONEVENT(PARTY_CLOSE_CHANNEL,				EXPORT_RESOURCE,		CH323ExportPartyCntl::OnPartyChannelDisconnect)
  ONEVENT(PARTY_CLOSE_CHANNEL,				EXPORT_MPL,				CH323ExportPartyCntl::OnPartyChannelDisconnect)
  ONEVENT(PARTY_CLOSE_CHANNEL,				EXPORT_RESOURSE_END,	CH323ExportPartyCntl::OnPartyChannelDisconnect)
  ONEVENT(PARTY_CLOSE_CHANNEL,				EXPORT_PARTY,			CH323ExportPartyCntl::OnPartyChannelDisconnect)         
  // Timer events
  ONEVENT(PCM_DISCONNECTPARTY_TOUT,			IDLE,					CH323ExportPartyCntl::OnTimerPcmDisconnect)
  ONEVENT(XCODE_DISCONNECTPARTY_TOUT,		IDLE,					CH323ExportPartyCntl::OnTimerDisconnectXCodeBrdg)
  ONEVENT(EXPORT_PARTY_TOUT,				EXPORT_PARTY,   		CH323ExportPartyCntl::OnTimerExport)
  ONEVENT(EXPORT_RSRC_TOUT,					EXPORT_RESOURCE, 		CH323ExportPartyCntl::OnTimerRAStartMove)
  ONEVENT(EXPORT_MPL_TOUT,  				EXPORT_MPL,				CH323ExportPartyCntl::OnTimerMPLExport)
  ONEVENT(END_EXPORT_RSRC_TOUT,				EXPORT_RESOURSE_END,	CH323ExportPartyCntl::OnTimerRAEndMove)
  ONEVENT(EXPORT_BRIDGES_TOUT,				EXPORT_BRIDGES,			CH323ExportPartyCntl::OnTimerExportBridges)
  ONEVENT(EXPORT_FAILED_TOUT,				ANYCASE,				CH323ExportPartyCntl::OnTimerExportFailed)
  // ReCap
  ONEVENT(REMOTE_SENT_RE_CAPS,				EXPORT_BRIDGES,			CH323ExportPartyCntl::OnPartyReceivedReCaps)
  ONEVENT(REMOTE_SENT_RE_CAPS,   			EXPORT_PARTY, 			CH323ExportPartyCntl::OnPartyReceivedReCaps)
  ONEVENT(REMOTE_SENT_RE_CAPS,   			EXPORT_RESOURCE,		CH323ExportPartyCntl::OnPartyReceivedReCaps) 
  ONEVENT(REMOTE_SENT_RE_CAPS,   			EXPORT_MPL, 			CH323ExportPartyCntl::OnPartyReceivedReCaps)
  ONEVENT(REMOTE_SENT_RE_CAPS,   			EXPORT_RESOURSE_END,	CH323ExportPartyCntl::OnPartyReceivedReCaps)

  // Party connect event
  ONEVENT(H323PARTYCONNECTALL,				ANYCASE,				CH323ExportPartyCntl::OnPartyH323ConnectAll)
  // Reduce Content rate to zero
 ONEVENT(PARTY_VIDEO_OUT_UPDATED,          ANYCASE,                CH323ExportPartyCntl::NullActionFunction)
 //VNGR-23940
 ONEVENT(SECONDARYCAUSEH323,   			EXPORT_RESOURSE_END,	CH323ExportPartyCntl::NullActionFunction)

PEND_MESSAGE_MAP(CH323ExportPartyCntl,CH323PartyCntl);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

CH323ExportPartyCntl::CH323ExportPartyCntl() //Ctor
{
  m_numOfActiveLogicalRsrc = 0;
  WORD i = 0;
  for (i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES ; i++)
  	m_activeLogicalRsrc[i] = 0;
  		
}

/////////////////////////////////////////////////////////////////////////////
CH323ExportPartyCntl::~CH323ExportPartyCntl() // Dtor
{
	
}

/////////////////////////////////////////////////////////////////////////////
CH323ExportPartyCntl& CH323ExportPartyCntl::operator =(const CH323ExportPartyCntl& other)
{
	(CH323PartyCntl&)*this = (CH323PartyCntl&)other;
	m_numOfActiveLogicalRsrc = other.m_numOfActiveLogicalRsrc;

	WORD i = 0;
	for (i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES ; i++)
		m_activeLogicalRsrc[i] = other.m_activeLogicalRsrc[i];
		
		
	return *this;
}

///////////////////////////////////////////////////////////////////////////////
CH323ExportPartyCntl& CH323ExportPartyCntl::operator =(const CH323PartyCntl& other)
{
	(CH323PartyCntl&)*this = other;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void*  CH323ExportPartyCntl::GetMessageMap()                                        
{
  return (void*)m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
const char*   CH323ExportPartyCntl::NameOf()  const
{
  return "CH323ExportPartyCntl";
}

/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::OnAudioBridgeExported(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnAudioBridgeExported", GetPartyRsrcId());
	
	// Amir/Talya - Within the segment there will be a pointer to a struct/class that will hold all relevant 
	// params needed for the move Audio wise.
	// TBD
	CAudioBridgePartyCntl* ptr;
	WORD status;
	BYTE isParams;
	*pParam >> status >> isParams;
	if (status != STATUS_OK)
	{
		PASSERT(status);
		SetDataForExportPartyFail();
		StartTimer(EXPORT_FAILED_TOUT,SECOND);
		return;
	}		
		
	*pParam >> (void*&)ptr;
	
	m_pBridgeMoveParams->SetAudioBridgePartyCntlOnExport(ptr);
	
	m_eAudBridgeConnState = eBridgeDisconnected;
	OFF(m_bIsMemberInAudBridge);
	
	m_pTaskApi->UpdateDB(m_pParty,AUDCON,FALSE); 
	BridgeExportCompleted();
}


/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::OnVideoBridgeExported(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnVideoBridgeExported", GetPartyRsrcId());
	
	// Amir/Talya - Within the segment there will be a pointer to a struct/class that will hold all relevant 
	// params needed for the move Audio wise.
	// TBD
	CVideoBridgePartyCntl* ptr;
	WORD status;
	BYTE isParams;
	*pParam >> status >> isParams;
	if (status != STATUS_OK)
	{
		PASSERT(status);
		SetDataForExportPartyFail();
		StartTimer(EXPORT_FAILED_TOUT,SECOND);
		return;
	}		

	*pParam >> (void*&)ptr;

	m_pBridgeMoveParams->SetVideoBridgePartyCntlOnExport(ptr);
  
    m_eVidBridgeConnState = eBridgeDisconnected;
    OFF(m_bIsMemberInVidBridge);
    
    m_pTaskApi->UpdateDB(m_pParty,VIDCON,FALSE); 
    BridgeExportCompleted();   
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnFeccBridgeDisConnect(CSegment* pParam)
{
	CH323PartyCntl::OnFeccBridgeDisConnect(pParam);
	BridgeExportCompleted();   
}
/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnContentBridgeDisConnect(CSegment* pParam)
{
	CPartyCntl::OnContentBrdgDisconnected(pParam);
	BridgeExportCompleted();   
}
/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnContentBridgeDisConnectAnyCase(CSegment* pParam)
{
    CPartyCntl::OnContentBrdgDisconnected(pParam);
}
/////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnXCodeBrdgDisconnectIdle(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnXCodeBrdgDisconnect", GetPartyRsrcId());

	if (IsValidTimer(XCODE_DISCONNECTPARTY_TOUT))
		DeleteTimer(XCODE_DISCONNECTPARTY_TOUT);

	CPartyCntl::OnXCodeBrdgDisconnected(pParam);

	if( (!m_isPcmConnected) ||(!IsValidTimer(PCM_DISCONNECTPARTY_TOUT)))
	SendStartMoveToResourceProcess();


}
/////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnXCodeBrdgDisconnectAnyCase(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnXCodeBrdgDisconnectAnyCase", GetPartyRsrcId());
	CPartyCntl::OnXCodeBrdgDisconnected(pParam);
}
/////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnXCodeBrdgDisconnectExportBrdgs(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnXCodeBrdgDisconnectExportBrdgs", GetPartyRsrcId());

	if (IsValidTimer(XCODE_DISCONNECTPARTY_TOUT))
		DeleteTimer(XCODE_DISCONNECTPARTY_TOUT);

	CPartyCntl::OnXCodeBrdgDisconnected(pParam);

	BridgeExportCompleted();
}
/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::BridgeExportCompleted()
{
	if ( !m_bIsMemberInAudBridge && (m_voice || (!m_bIsMemberInVidBridge && !m_isFeccConn && !m_isContentConn && !m_isXCodeConn) ) )
	{
		PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::BridgeExportCompleted - export party", GetPartyRsrcId());
		DeleteTimer(EXPORT_BRIDGES_TOUT);
		// validation test for party api
		if(! CPObject::IsValidPObjectPtr(m_pPartyApi))
		{
			DBGPASSERT(GetPartyRsrcId());
			SetDataForExportPartyFail();
			StartTimer(EXPORT_FAILED_TOUT,SECOND);
			
			return;
		}
		m_state = EXPORT_PARTY;
		// send export to party
		StartTimer(EXPORT_PARTY_TOUT, 30*SECOND);

        m_pBridgeMoveParams->UnregisterBridgePartyCntlsInTask();
		
		m_pPartyApi->Export(m_pDestConfMbx,this,m_pComConf,m_moveType);
	}
} 	


/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnAudBrdgDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnAudBrdgDisconnect", GetPartyRsrcId());
	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam);
	if (bIsDisconnectOk == FALSE)
	{
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
	}
	
	BridgeDisconnetCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnPartyDisconnet(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnPartyDisconnet", GetPartyRsrcId());
	WORD  status;
	*pParam >> status;
	DeleteTimer(PARTYDISCONNECTTOUT);
//  EndPartyDisconnect();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnVidBrdgDisconnect(CSegment* pParam)
{
 	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnVidBrdgDisconnect", GetPartyRsrcId());

	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);
	if (resStat == statVideoInOutResourceProblem)
	{
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
	}
	if (m_pIpCurrentMode->GetConfType() == kCop) // RMX2000C: we disconnect the video bridge instead of just export it, because of the different bridge and resource type.
	{
		BridgeExportCompleted();
	}
	else
		BridgeDisconnetCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnPartyExport(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnPartyExport", GetPartyRsrcId());
	DeleteTimer(EXPORT_PARTY_TOUT);
 	WORD status; 
  	*pParam >> status;
  	if ( ! status )  
  	{  // export was o.k.
    	m_pPartyApi->DestroyOnlyApi();
    	POBJDELETE(m_pPartyApi);

  	}
	if (status == statIllegal)
	{
		// In this case we will switch the conf id's since the party is already registered in the destination conf.
		SetDataForExportPartyFail();
		StartTimer(EXPORT_FAILED_TOUT,SECOND);
	}
	else  	  
  		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),status);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnTimerExport(CSegment* pParam)
{
  PTRACEPARTYID(eLevelError,"CH323ExportPartyCntl::OnTimerExport", GetPartyRsrcId());
  SetDataForExportPartyFail();
  StartTimer(EXPORT_FAILED_TOUT,SECOND);
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnTimerExportFailed(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323ExportPartyCntl::OnTimerExportFailed", GetPartyRsrcId());
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
}
 	
/////////////////////////////////////////////////////////////////////////////
//	
void  CH323ExportPartyCntl::Transfer(COsQueue* pDestRcvMbx,void* pComConf,DWORD destConfId,DWORD destPartyId,EMoveType eCurMoveType)                                        
{ 
	PTRACE2PARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::Transfer : Name - ",m_partyConfName, GetPartyRsrcId());
	
	// 1) validation tests
	if ( m_disconnectState == DISCONNECTED )  {  // disconnected party cannot be transfer !!!
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
	}
	
	// 2) setings of party controll that required under the source conference
//	InitTimer(m_pConf->GetRcvMbx());
  	// define move type
	m_moveType = eCurMoveType;
	// set destination conf mail box
	if ( m_pDestConfMbx )
		POBJDELETE(m_pDestConfMbx);
	m_pDestConfMbx = new COsQueue(*pDestRcvMbx);
	//
	m_pComConf = pComConf;
	m_destMonitorConfId = destConfId;
	m_destMonitorPartyId = destPartyId;
	
	// close content if needed (not in case of a voice party)
	if(!m_voice)
	{
		BYTE bIsTokenHolder = m_pContentBridge->IsTokenHolder(m_pParty);
		if(bIsTokenHolder)
			m_pTaskApi->ContentTokenWithdraw();

		if(m_pContentBridge->IsTokenHeld())
		{
			if (!bIsTokenHolder)
				m_pPartyApi->SendContentTokenMediaProducerStatus(0,CHANNEL_INACTIVE);

			if (m_pIpCurrentMode->GetContentBitRate(cmCapTransmit))
			{
				DWORD H323ContentRate =  CUnifiedComMode ::TranslateAMCRateIPRate(AMC_0k);
				m_pIpCurrentMode->SetContentBitRate(H323ContentRate,cmCapReceiveAndTransmit);
				m_pIpInitialMode->SetContentBitRate(H323ContentRate,cmCapReceiveAndTransmit);
				m_pIpCurrentMode->SetVideoBitRate(m_pIpCurrentMode->GetTotalVideoRate());
				m_pIpInitialMode->SetVideoBitRate(m_pIpCurrentMode->GetTotalVideoRate());
			}
		}
	}
	if ((m_isPcmConnected)||(m_isXCodeConn))
	{
		if(m_isPcmConnected)
		{
			PTRACE(eLevelInfoNormal,"CH323ExportPartyCntl::Transfer wait 5 more seconds for pcm disconnection");
			PASSERT (m_state != IDLE);
			StartTimer(PCM_DISCONNECTPARTY_TOUT,5*SECOND);
		}
		if(m_isXCodeConn)
		{
			PTRACE(eLevelInfoNormal,"CH323ExportPartyCntl::Transfer wait 5 more seconds for Xcode disconnection");
			DisconnectPartyFromXCodeBridge();
			StartTimer(XCODE_DISCONNECTPARTY_TOUT,5*SECOND);
		}
	}
	else
	{
		SendStartMoveToResourceProcess();
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnPartyPcmStateChangedIdle(CSegment* pParam)
{
	CPartyCntl::OnPartyPcmStateChangedAnycase(pParam);
	if (!m_isPcmConnected)
	{
		if (IsValidTimer(PCM_DISCONNECTPARTY_TOUT))
		{
			PTRACE(eLevelInfoNormal,"CH323ExportPartyCntl::OnPartyPcmStateChangedIdle Pcm Disconnection Completed");
			DeleteTimer(PCM_DISCONNECTPARTY_TOUT);
		}
		if(!m_isXCodeConn)
			SendStartMoveToResourceProcess();
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnTimerPcmDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323ExportPartyCntl::OnTimerPcmDisconnect PCM did not completed its disconnection!", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());

	if(!m_isXCodeConn)
		SendStartMoveToResourceProcess();

}
/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnTimerDisconnectXCodeBrdg(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CH323ExportPartyCntl::OnTimerDisconnectXCodeBrdg Xcode brdg not completed its disconnection!", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());

	if( (!m_isPcmConnected) ||(!IsValidTimer(PCM_DISCONNECTPARTY_TOUT)))
		SendStartMoveToResourceProcess();

}

/////////////////////////////////////////////////////////////////////////////

void CH323ExportPartyCntl::OnTimerRAStartMove(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnTimerRAStartMove - Failed to start move in RA", GetPartyRsrcId());
	PASSERT(GetPartyRsrcId());
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
}
	
/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::OnEndResourceAllocatorStartMove(CSegment* pParam)
{			
    PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnEndResourceAllocatorStartMove", GetPartyRsrcId());
    // Here we check if status OK and if so we start the MPL_API part
    // Moving the phisical resources (MFA).
    DWORD  status,targetMonitorConfId, targetResourceConfId, monitorPartyId,rsrcPartyId;
    DeleteTimer(EXPORT_RSRC_TOUT);
    *pParam >> status >> targetMonitorConfId >> targetResourceConfId >> monitorPartyId >> rsrcPartyId;
    if (status != STATUS_OK) 
    {
		PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnEndResourceAllocatorStartMove - Failed to start move in RA - Bad status", GetPartyRsrcId());    	
    	PASSERT(status);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }
 	if (monitorPartyId != m_monitorPartyId)
 	{
 		PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnEndResourceAllocatorStartMove - wrong party monitor Id", GetPartyRsrcId());    	
    	PASSERT(monitorPartyId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }
    if (rsrcPartyId != m_pPartyAllocatedRsrc->GetPartyRsrcId())
    {
		PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnEndResourceAllocatorStartMove - wrong party rsrc Id", GetPartyRsrcId());    	
    	PASSERT(rsrcPartyId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    } 
    if(targetMonitorConfId != m_destMonitorConfId)
    {
    	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnEndResourceAllocatorStartMove - wrong target monitor conf Id", GetPartyRsrcId());    	
    	PASSERT(targetMonitorConfId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }
    
    CSmallString str;
    str << targetResourceConfId;
   	PTRACE2PARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnEndResourceAllocatorStartMove - targetResourceConfId value is: ",str.GetString(), GetPartyRsrcId());    	    
    m_destResourceConfId = targetResourceConfId; 		  
	m_state = EXPORT_MPL;
	
	// Here we send the request to the MPL_API(MFA)
	// But first we will find which logical resources are open by asking the bridges and filling the array.
	// TBD - Waiting for Talya/Matvey interface
//	BOOL isEncOpen = FALSE;
//	BOOL isDecOpen = FALSE;
//	m_pAudioInterface->ArePortsOpened(m_pParty,isEncOpen,isDecOpen);
//	if (isEncOpen != FALSE)
//		m_activeLogicalRsrc[eLogical_audio_encoder] = 1;
//	if (isDecOpen)
//		m_activeLogicalRsrc[eLogical_audio_decoder] = 1;

	std::map<eLogicalResourceTypes,bool> isOpenedAudioRsrcMap;
	m_pAudioInterface->ArePortsOpened(rsrcPartyId, isOpenedAudioRsrcMap);
	for (std::map<eLogicalResourceTypes,bool>::const_iterator audioRsrcIter = isOpenedAudioRsrcMap.begin(); audioRsrcIter != isOpenedAudioRsrcMap.end(); ++audioRsrcIter)
	{
		eLogicalResourceTypes lrt = audioRsrcIter->first;
		m_activeLogicalRsrc[lrt]  = audioRsrcIter->second;

		// Here we check - If there is an audio logical rsrc open - we open the RTP too
		if (m_activeLogicalRsrc[lrt] == 1)
			m_activeLogicalRsrc[eLogical_rtp] = 1;
	}
	
	if ( m_activeLogicalRsrc[eLogical_rtp] == 0 )
	{
		PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnEndResourceAllocatorStartMove - No active resources", GetPartyRsrcId());	
		PASSERT(GetPartyRsrcId());
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
	}
	// Video check
//	isEncOpen = FALSE;
//	isDecOpen = FALSE;
//	m_pVideoBridgeInterface->ArePortsOpened(m_pParty,isEncOpen,isDecOpen);
//	if (isEncOpen != FALSE)
//		m_activeLogicalRsrc[eLogical_video_encoder] = 1;
//	if (isDecOpen)
//		m_activeLogicalRsrc[eLogical_video_decoder] = 1;

	std::map<eLogicalResourceTypes,bool> isOpenedVideoRsrcMap;
	m_pVideoBridgeInterface->ArePortsOpened(rsrcPartyId, isOpenedVideoRsrcMap);

	for (std::map<eLogicalResourceTypes,bool>::const_iterator videoRsrcIter = isOpenedVideoRsrcMap.begin(); videoRsrcIter != isOpenedVideoRsrcMap.end(); ++videoRsrcIter)
	{
		eLogicalResourceTypes lrt = videoRsrcIter->first;
		m_activeLogicalRsrc[lrt]  = videoRsrcIter->second;
	}

	if (eMixAvcSvc == m_pIpInitialMode->GetConfMediaType())
	{
		m_activeLogicalRsrc[eLogical_relay_avc_to_svc_rtp] = IsPortConnected(eLogical_relay_avc_to_svc_rtp);
		m_activeLogicalRsrc[eLogical_relay_avc_to_svc_rtp_with_audio_encoder] = IsPortConnected(eLogical_relay_avc_to_svc_rtp_with_audio_encoder);
		m_activeLogicalRsrc[eLogical_relay_rtp] = IsPortConnected(eLogical_relay_rtp);
	}

	ostringstream ostr;
	m_numOfActiveLogicalRsrc = 0;

	MOVE_RESOURCES_REQ_S *pMoveRsrcReq = new MOVE_RESOURCES_REQ_S;
	WORD i = 0;
	for (i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES ; i++)
		pMoveRsrcReq->openLogicalResources[i] = eRsrcNonActive;


	for (i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
	{ 
		if ( m_activeLogicalRsrc[i] == 1 )
		{
			pMoveRsrcReq->openLogicalResources[i] = eRsrcActive; // Active
			m_numOfActiveLogicalRsrc++;
			ostr << "\n" << ::LogicalResourceTypeToString(i) << " = eRsrcActive";
		}
	}
	TRACEINTO << ostr.str().c_str();

	pMoveRsrcReq->moveRsrcParams.confType = 0XFFFFFFFF;
	pMoveRsrcReq->moveRsrcParams.newConfId = m_destResourceConfId;
	ESampleRate eSampleRate;
	if ( FALSE == CAudioHardwareInterface::TranslateMcmsSampleRateToApiValues(GetConfAudioSampleRate(), eSampleRate) ) {
		PASSERTMSG( GetConfAudioSampleRate()+1, "CH323ExportPartyCntl::OnEndResourceAllocatorStartMove - invalid sample rate!");
	}
	pMoveRsrcReq->moveRsrcParams.confAudioSampleRate = eSampleRate;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_destMonitorConfId);

	if(pCommConf)
		pMoveRsrcReq->moveRsrcParams.enConfSpeakerChangeMode = pCommConf->GetConfSpeakerChangeMode(); //xifwang
	else
	{
		TRACESTR (eLevelInfoNormal) << "CH323ExportPartyCntl::OnEndResourceAllocatorStartMove,couldn't find conf with monitor id: " <<m_destMonitorConfId; 
		pMoveRsrcReq->moveRsrcParams.enConfSpeakerChangeMode = E_CONF_DEFAULT_SPEAKER_CHANGE_MODE;
	}
	  
			
	CSegment* pMsg = new CSegment;
	pMsg->Put((BYTE*)(pMoveRsrcReq),sizeof(MOVE_RESOURCES_REQ_S));
	m_pPartyHWInterface->SendMsgToMPL(MOVE_PARTY_RESOURCE_REQ,pMsg);

    //update conf rsrc ID
    m_pPartyApi->SetRsrcConfIdForInterface(m_destResourceConfId);
    m_pPartyHWInterface->SetConfRsrcId(m_destResourceConfId);
    
	StartTimer(EXPORT_MPL_TOUT, 10*SECOND);

	POBJDELETE(pMoveRsrcReq);
	POBJDELETE(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::OnMplApiMoveAck(CSegment* pParam)
{	  	
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnMplApiMoveAck", GetPartyRsrcId());
	BYTE lrt;
	ACK_IND_S* pAckIndStrcut = new ACK_IND_S; AUTO_DELETE(pAckIndStrcut);
	pParam->Get((BYTE*)pAckIndStrcut,sizeof(ACK_IND_S));
	*pParam >> lrt;
	
	if (pAckIndStrcut->ack_base.status != STATUS_OK)
	{
		PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnMplApiMoveAck : Status Not OK", GetPartyRsrcId());
		PASSERT(pAckIndStrcut->ack_base.status);
		m_isFaulty = 1;
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;

	}
			
	WORD status = STATUS_OK;
				
	switch (pAckIndStrcut->ack_base.ack_opcode)
	{
		case MOVE_PARTY_RESOURCE_REQ:
		{
			status = CheckIfLogicalRsrcAckAccepted((eLogicalResourceTypes)lrt);
			break;
		}
		
		default:
		{
			// TBD - Guy + Yael + Uri - What are doing in case of wrong opcode? (Ignore/fail)
			DBGPASSERT(pAckIndStrcut->ack_base.ack_opcode);
			break;
		}
	}
	if (status != STATUS_OK)
	{
		CSmallString str;
		str << pAckIndStrcut->ack_base.reason;
		PTRACE2PARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnMplApiMoveAck : Move ack is wrong - ",str.GetString(), GetPartyRsrcId());
		// TBD - What are we doing in that case - disconnect the party ?
		PASSERT(lrt);
		m_isFaulty = 1;
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		PDELETE(pAckIndStrcut); 
		return;
		
	}
	if (m_numOfActiveLogicalRsrc == 0)
	{
		// This means we got all relavent ACKs from the MFA and we may continue the flow.
		MfaAcksCompleted();
	}
	PDELETE(pAckIndStrcut); 	
								
}

/////////////////////////////////////////////////////////////////////////////
WORD CH323ExportPartyCntl::CheckIfLogicalRsrcAckAccepted(eLogicalResourceTypes lrt)
{		
	// Checking if the correct ACK was received - 
	// If so, decrease the number of active logical resourse counter by one.
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::CheckIfLogicalRsrcAckAccepted", GetPartyRsrcId());
	
	if (m_activeLogicalRsrc[lrt] == 1)
	{
			m_numOfActiveLogicalRsrc--;
			return STATUS_OK;
	}
	else
	{
		CSmallString str;
		str << (WORD)lrt;
		if 	( lrt == eLogical_audio_encoder || lrt == eLogical_audio_decoder || lrt == eLogical_rtp )
		{
			PTRACE2PARTYID(eLevelError,	"CH323ExportPartyCntl::CheckIfLogicalRsrcAckAccepted: logical rsrc %d sent extra due to ART ind",str.GetString(), GetPartyRsrcId());
			return STATUS_OK;
		}
		else
		{
			PTRACE2PARTYID(eLevelError,	"CH323ExportPartyCntl::CheckIfLogicalRsrcAckAccepted: logical rsrc %d not valid",str.GetString(), GetPartyRsrcId());
			return STATUS_FAIL;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::MfaAcksCompleted()
{	
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::MfaAcksCompleted", GetPartyRsrcId());
	DeleteTimer(EXPORT_MPL_TOUT);
	
	m_state = EXPORT_RESOURSE_END;
	StartTimer(END_EXPORT_RSRC_TOUT, 8*SECOND);

  	// Here we will move the resources in RA
  	PARTY_MOVE_RSRC_REQ_PARAMS_S *pReq = new PARTY_MOVE_RSRC_REQ_PARAMS_S;
   	pReq->source_monitor_conf_id = m_monitorConfId;
   	pReq->target_monitor_conf_id = m_destMonitorConfId;
   	pReq->source_monitor_party_id = m_monitorPartyId;
   	pReq->target_monitor_party_id = m_destMonitorPartyId; // TBD 0xfffffff ?
    CMedString *pStrReq = new CMedString;
	*pStrReq <<"CH323ExportPartyCntl::MfaAcksCompleted - END_PARTY_MOVE_RSRC_REQ:\n"
		<< "source_monitor_conf_id	   =   "<< pReq->source_monitor_conf_id <<",  "
		<< "target_monitor_conf_id     =   "<< pReq->target_monitor_conf_id <<'\n'
		<< "source_monitor_party_id	   =   "<< pReq->source_monitor_party_id <<",  "
		<< "target_monitor_party_id    =   "<< pReq->target_monitor_party_id;
	PTRACEPARTYID(eLevelInfoNormal, pStrReq->GetString(), GetPartyRsrcId());
	POBJDELETE(pStrReq);   	
  	CSegment *pSeg = new CSegment;
	pSeg->Put((BYTE*)(pReq),sizeof(PARTY_MOVE_RSRC_REQ_PARAMS_S));  	
  	SendReqToResourceAllocator(pSeg,END_PARTY_MOVE_RSRC_REQ);
   	PDELETE(pReq);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::OnResourceAllocatorEndMove(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnResourceAllocatorEndMove", GetPartyRsrcId());
	
	DeleteTimer(END_EXPORT_RSRC_TOUT);
	
    DWORD  status, targetMonitorConfId, targetRsrcConfId, monitorPartyId, rsrcPartyId, numRsrcs;

    DeleteTimer(EXPORT_RSRC_TOUT); 
    *pParam >> status >> targetMonitorConfId >> targetRsrcConfId >> monitorPartyId >> rsrcPartyId >> numRsrcs;

    CMedString *pStr = new CMedString;
	*pStr <<"CH323ExportPartyCntl::OnResourceAllocatorEndMove - END_PARTY_MOVE_RSRC_IND:\n"
		<< "target_monitor_conf_id	   =   "<< targetMonitorConfId <<",  "
		<< "target_rsrc_conf_id     =   "<< targetRsrcConfId <<'\n'
		<< "monitor_party_id	   =   "<< monitorPartyId <<",  "
		<< "rsrc_party_id    =   "<< rsrcPartyId << "\n"
		<< "numRsrcs	=	"<< numRsrcs << "\n";
	for(WORD i=0;i<numRsrcs;i++)
	{
		CRsrcDesc* pTempRsrcDesc = new CRsrcDesc;
		pTempRsrcDesc->DeSerialize(NATIVE, *pParam);
		*pStr  << (i+1) << ". connId = "<< pTempRsrcDesc->GetConnectionId()
		   <<" : logicalRsrcType = "
		   << ::LogicalResourceTypeToString(pTempRsrcDesc->GetLogicalRsrcType()) << '\n';

		m_pPartyAllocatedRsrc->AddNewRsrcDesc(pTempRsrcDesc);
		POBJDELETE(pTempRsrcDesc);
	}

	PTRACEPARTYID(eLevelInfoNormal, pStr->GetString(), GetPartyRsrcId());
	POBJDELETE(pStr);

	if (status != STATUS_OK)
    {
		PTRACEPARTYID(eLevelError,"CH323ExportPartyCntl::OnResourceAllocatorEndMove - Failed to end move in RA - Bad status", GetPartyRsrcId());
    	PASSERT(status);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey.
		m_isFaulty = 1; 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }
 	if (monitorPartyId != m_destMonitorPartyId)
 	{
 		PTRACEPARTYID(eLevelError,"CH323ExportPartyCntl::OnResourceAllocatorEndMove - wrong party monitor Id", GetPartyRsrcId());
    	PASSERT(monitorPartyId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_isFaulty = 1;
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }
    if (rsrcPartyId != m_pPartyAllocatedRsrc->GetPartyRsrcId())
    {
		PTRACEPARTYID(eLevelError,"CH323ExportPartyCntl::OnResourceAllocatorEndMove - wrong party rsrc Id", GetPartyRsrcId());
    	PASSERT(rsrcPartyId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey.
		m_isFaulty = 1; 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }  
    if(targetMonitorConfId != m_destMonitorConfId)
    {
    	PTRACEPARTYID(eLevelError,"CH323ExportPartyCntl::OnResourceAllocatorEndMove - wrong target monitor conf Id", GetPartyRsrcId());
    	PASSERT(targetMonitorConfId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey.
		m_isFaulty = 1; 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }     	
	m_state = EXPORT_BRIDGES;
	// Here we start the disconnection from the A/V bridges	
	if (m_pAudioInterface && IsAtLeastOneDirectionConnectedToAudioBridge())
	{
		// TBD - Amir K./Matvey
		// Need to define + new + fill with data (Talya will tell what to fill)
		// CBridgePartyExportParams bridgePartyExportParams(...);
//		m_pConfAppMngrInterface->ExportPartyAudio(&bridgePartyExportParams);
		CBridgePartyExportParams bridgePartyExportParams(m_pParty->GetPartyId());
		m_pConfAppMngrInterface->ExportPartyAudio(&bridgePartyExportParams);
	}
		// for voice party
	if (!m_voice)
	{
	// disconnect video
		if (m_pVideoBridgeInterface && IsAtLeastOneDirectionConnectedToVideoBridge())
		{
			// TBD - Amir K.
			// Need to define + new + fill with data (Talya will tell what to fill)
			// CBridgePartyExportParams bridgePartyExportParams(...);		
	//			m_pConfAppMngrInterface->ExportPartyVideo(&bridgePartyExportParams);

			// RMX2000C:
			// when move from Cop EQ, we disconnect the video bridge because Cop EQ has different type of video bridge
			if (m_pIpCurrentMode->GetConfType() == kCop)
				DisconnectPartyFromVideoBridge();
			else
			{
				CBridgePartyExportParams bridgePartyExportParams(m_pParty->GetPartyId());
				m_pConfAppMngrInterface->ExportPartyVideo(&bridgePartyExportParams);
			}

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
		OFF(m_isTerminalNumberingConn);;
   	}
   	
   	ON(m_isPartyEndRAMoveOK);
	
	StartTimer(EXPORT_BRIDGES_TOUT, 12*SECOND);	
}	
		
/////////////////////////////////////////////////////////////////////////////
//	This function is used by CExportH323PartyCntl and CSipExportPartyCntl
// this function export party
// after it disconnected from all bridges
void  CH323ExportPartyCntl::BridgeDisconnetCompleted()                                        
{
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
	// TBD - Yael - Is regular disconnection process ? (Initiating the partyApi ?)
/*	if ( (m_voice && !m_isAudConn)||(!m_isAudConn && !m_isVidConn ) ) 
	{
		PTRACEPARTYID(eLevelInfoNormal,"CExportIpPartyCntl::BridgeDisconnetCompleted - export party");
		if (m_pPartyApi)  
			DestroyParty();
		else if (m_pPartyHWInterface)
			DeletePartyFromHW();
		else
			DeallocatePartyResources();
	}*/
}		


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnTimerMPLExport(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnTimerMPLExport - Failed to receive IND from MFA", GetPartyRsrcId());
	PASSERT(GetPartyRsrcId());
	m_isFaulty = 1;
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
}
	
/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnTimerRAEndMove(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnTimerRAEndMove - Failed to receive end move from RA", GetPartyRsrcId());
	PASSERT(GetPartyRsrcId());
	m_isFaulty = 1;
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::OnTimerExportBridges(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnTimerExportBridges - Failed to receive IND from bridges", GetPartyRsrcId());
	PASSERT(GetPartyRsrcId());
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	SetDataForExportPartyFail();
	StartTimer(EXPORT_FAILED_TOUT,SECOND);
	
}

/////////////////////////////////////////////////////////////////////////////
void  CH323ExportPartyCntl::OnPartyChannelDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnPartyChannelDisconnect", GetPartyRsrcId());
	WORD tempDataType, tempDirection, tempRoleLabel;
	*pParam >> tempDataType >> tempDirection >> tempRoleLabel;
	
	cmCapDataType  dataType	 = (cmCapDataType)tempDataType;
	cmCapDirection direction = (cmCapDirection)tempDirection;
	ERoleLabel     roleLabel = (ERoleLabel)tempRoleLabel;		
		
	UpdateCurrentModeMediaOff(dataType, roleLabel, direction);
	UpdateInitialModeMediaOff(dataType, roleLabel, direction);
}

/////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::OnPartyReceivedReCaps(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnPartyReceivedReCaps", GetPartyRsrcId());
	PartyReceivedReCaps(pParam);
}

//////////////////////////////////////////////////////////////////////////////
void CH323ExportPartyCntl::OnPartyH323ConnectAll(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CH323ExportPartyCntl::OnPartyH323ConnectAll", GetPartyRsrcId());
	PartyH323ConnectAllPartyConnectAudioOrChangeAll(pParam);
	m_bIsPartyConnectAllWhileMove = TRUE;
}

//////////////////////////////////////////////////////////////////////////////
	
	    	
