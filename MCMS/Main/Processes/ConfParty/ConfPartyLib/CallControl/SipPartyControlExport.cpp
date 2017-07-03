
//+========================================================================+
//                            SipExportPartyControl.cpp                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SipExportPartyControl.cpp                                     |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 11/11/05   |  Create + Stage 1 - Move between same conferences    |
//+========================================================================+

#include "SIPPartyControlExport.h"
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

 
PBEGIN_MESSAGE_MAP(CSipExportPartyCntl)

  ONEVENT(PCM_PARTY_STATE_CHANGED,			IDLE,					CSipExportPartyCntl::OnPartyPcmStateChangedIdle)
  ONEVENT(START_PARTY_MOVE_RSRC_IND,		EXPORT_RESOURCE,		CSipExportPartyCntl::OnEndResourceAllocatorStartMove)
  ONEVENT(ACK_IND,							EXPORT_MPL,				CSipExportPartyCntl::OnMplApiMoveAck)
  ONEVENT(END_PARTY_MOVE_RSRC_IND,			EXPORT_RESOURSE_END,	CSipExportPartyCntl::OnResourceAllocatorEndMove)
  ONEVENT(PARTY_AUDIO_EXPORTED,				EXPORT_BRIDGES,			CSipExportPartyCntl::OnAudioBridgeExported)
  ONEVENT(PARTY_VIDEO_EXPORTED,				EXPORT_BRIDGES,			CSipExportPartyCntl::OnVideoBridgeExported)
  ONEVENT(FECC_PARTY_BRIDGE_DISCONNECTED,	EXPORT_BRIDGES, 		CSipExportPartyCntl::OnFeccBridgeDisConnect)
  ONEVENT(PARTY_AUDIO_DISCONNECTED,			EXPORT_BRIDGES,   		CSipExportPartyCntl::OnAudBrdgDisconnect)
  ONEVENT(PARTY_VIDEO_DISCONNECTED,			EXPORT_BRIDGES,   		CSipExportPartyCntl::OnVideoBrdgDisconnected)
  ONEVENT(PARTY_CONTENT_DISCONNECTED,		EXPORT_BRIDGES,   		CSipExportPartyCntl::OnContentBridgeDisconnect)
  ONEVENT(PARTY_CONTENT_DISCONNECTED,		EXPORT_BRIDGES,   		CSipExportPartyCntl::OnContentBridgeDisconnectAnycase)
  ONEVENT(PARTY_XCODE_DISCONNECTED,         IDLE,                   CSipExportPartyCntl::OnXCodeBrdgDisconnectIdle)
  ONEVENT(PARTY_XCODE_DISCONNECTED,         EXPORT_BRIDGES,         CSipExportPartyCntl::OnXCodeBrdgDisconnectExportBrdgs)
  ONEVENT(PARTY_XCODE_DISCONNECTED,         ANYCASE,                CSipExportPartyCntl::OnXCodeBrdgDisconnectAnyCase)
  //  ONEVENT(PARTY_XCODE_DISCONNECTED,		    EXPORT_BRIDGES,   		CSipExportPartyCntl::OnXCodeBridgeDisconnect)
  ONEVENT(PARTYEXPORT,       				EXPORT_PARTY,   		CSipExportPartyCntl::OnPartyExport)
  ONEVENT(EXPORT_PARTY_TOUT,   				EXPORT_PARTY,   		CSipExportPartyCntl::OnTimerExport) 
  // Timer events
  ONEVENT(PCM_DISCONNECTPARTY_TOUT,			IDLE,					CSipExportPartyCntl::OnTimerPcmDisconnect)
  ONEVENT(XCODE_DISCONNECTPARTY_TOUT,		IDLE,					CSipExportPartyCntl::OnTimerDisconnectXCodeBrdg)
  ONEVENT(EXPORT_RSRC_TOUT, 				EXPORT_RESOURCE, 		CSipExportPartyCntl::OnTimerRAStartMove)
  ONEVENT(EXPORT_MPL_TOUT,  				EXPORT_MPL,				CSipExportPartyCntl::OnTimerMPLExport)
  ONEVENT(END_EXPORT_RSRC_TOUT,				EXPORT_RESOURSE_END,	CSipExportPartyCntl::OnTimerRAEndMove)
  ONEVENT(EXPORT_BRIDGES_TOUT,				EXPORT_BRIDGES,			CSipExportPartyCntl::OnTimerExportBridges)
  ONEVENT(EXPORT_FAILED_TOUT,				ANYCASE,				CSipExportPartyCntl::OnTimerExportFailed)  
    	 
PEND_MESSAGE_MAP(CSipExportPartyCntl,CSipPartyCntl);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

CSipExportPartyCntl::CSipExportPartyCntl() //Ctor
{
  m_numOfActiveLogicalRsrc = 0;
  WORD i = 0;
  for (i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES ; i++)
  	m_activeLogicalRsrc[i] = 0;	
}

/////////////////////////////////////////////////////////////////////////////
CSipExportPartyCntl::~CSipExportPartyCntl() // Dtor
{
	
}

/////////////////////////////////////////////////////////////////////////////
CSipExportPartyCntl& CSipExportPartyCntl::operator =(const CSipExportPartyCntl& other)
{
	(CSipPartyCntl&)*this = (CSipPartyCntl&)other;
	m_numOfActiveLogicalRsrc = other.m_numOfActiveLogicalRsrc;

	WORD i = 0;
	for (i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES ; i++)
		m_activeLogicalRsrc[i] = other.m_activeLogicalRsrc[i];
		
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void*  CSipExportPartyCntl::GetMessageMap()                                        
{
  return (void*)m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
const char*   CSipExportPartyCntl::NameOf()  const
{
  return "CSipExportPartyCntl";
}

/////////////////////////////////////////////////////////////////////////////
void CSipExportPartyCntl::OnAudioBridgeExported(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnAudioBridgeExported", GetPartyRsrcId());
	
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
void CSipExportPartyCntl::OnVideoBridgeExported(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnVideoBridgeExported", GetPartyRsrcId());
	
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
void  CSipExportPartyCntl::OnFeccBridgeDisConnect(CSegment* pParam)
{
	CSipPartyCntl::OnFeccBridgeDisConnect(pParam);
	BridgeExportCompleted();   
}
/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnContentBridgeDisconnect(CSegment* pParam)
{
	CPartyCntl::OnContentBrdgDisconnected(pParam);
	BridgeExportCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnContentBridgeDisconnectAnycase(CSegment* pParam)
{
    CPartyCntl::OnContentBrdgDisconnected(pParam);
}
/////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnXCodeBrdgDisconnectIdle(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnXCodeBrdgDisconnect", GetPartyRsrcId());

	if (IsValidTimer(XCODE_DISCONNECTPARTY_TOUT))
		DeleteTimer(XCODE_DISCONNECTPARTY_TOUT);

	CPartyCntl::OnXCodeBrdgDisconnected(pParam);

	if( (!m_isPcmConnected) ||(!IsValidTimer(PCM_DISCONNECTPARTY_TOUT)))
	SendStartMoveToResourceProcess();


}
/////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnXCodeBrdgDisconnectAnyCase(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnXCodeBrdgDisconnectAnyCase", GetPartyRsrcId());
	CPartyCntl::OnXCodeBrdgDisconnected(pParam);
}
/////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnXCodeBrdgDisconnectExportBrdgs(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnXCodeBrdgDisconnectExportBrdgs", GetPartyRsrcId());

	if (IsValidTimer(XCODE_DISCONNECTPARTY_TOUT))
		DeleteTimer(XCODE_DISCONNECTPARTY_TOUT);

	CPartyCntl::OnXCodeBrdgDisconnected(pParam);

	BridgeExportCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void CSipExportPartyCntl::BridgeExportCompleted()
{
	if (!m_bIsMemberInAudBridge && !m_bIsMemberInVidBridge && !m_isFeccConn && !m_isContentConn && !m_isXCodeConn)
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::BridgeExportCompleted - export party", GetPartyRsrcId());
		DeleteTimer(EXPORT_BRIDGES_TOUT);

		if(! CPObject::IsValidPObjectPtr(m_pTaskApi))
		{
			DBGPASSERT(1);
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
void  CSipExportPartyCntl::OnAudBrdgDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnAudBrdgDisconnect", GetPartyRsrcId());
	BYTE bIsDisconnectOk = HandleAudioBridgeDisconnectedInd(pParam);
	if (bIsDisconnectOk == FALSE)
	{
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
	}
  	BridgeDisconnetCompleted();  
}

/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnPartyDisconnet(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnPartyDisconnet", GetPartyRsrcId());
	WORD  status;
	*pParam >> status;
	DeleteTimer(PARTYDISCONNECTTOUT);
//  EndPartyDisconnect();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnVideoBrdgDisconnected(CSegment* pParam)
{
 	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnVideoBrdgDisconnected", GetPartyRsrcId());

	EStat resStat = HandleVideoBridgeDisconnectedInd(pParam);
	if (resStat == statVideoInOutResourceProblem)
	{
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
	}
	if (m_pIpCurrentMode->GetConfType() == kCop || m_bIsMrcCall) // RMX2000C: we disconnect the video bridge instead of just export it, because of the different bridge and resource type.
	{
		BridgeExportCompleted();
	}
	else
		BridgeDisconnetCompleted();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnPartyExport(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnPartyExport", GetPartyRsrcId());
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
void  CSipExportPartyCntl::OnTimerExport(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipExportPartyCntl::OnTimerExport", GetPartyRsrcId());
  	SetDataForExportPartyFail();
  	StartTimer(EXPORT_FAILED_TOUT,SECOND);
 
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//	
void  CSipExportPartyCntl::Transfer(COsQueue* pDestRcvMbx,void* pComConf,DWORD destConfId,DWORD destPartyId,EMoveType eCurMoveType)                                        
{ 
	PTRACE2PARTYID(eLevelInfoNormal,"CSipExportPartyCntl::Transfer : Name - ",m_partyConfName, GetPartyRsrcId());
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

	if ((m_isPcmConnected)||(m_isXCodeConn))
	{
		if(m_isPcmConnected)
		{
			PTRACE(eLevelInfoNormal,"CSipExportPartyCntl::Transfer wait 5 more seconds for pcm disconnection");
			PASSERT (m_state != IDLE);
			StartTimer(PCM_DISCONNECTPARTY_TOUT,5*SECOND);
		}
		 if (m_isXCodeConn)
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
void  CSipExportPartyCntl::OnPartyPcmStateChangedIdle(CSegment* pParam)
{
	CPartyCntl::OnPartyPcmStateChangedAnycase(pParam);
	if (!m_isPcmConnected)
	{
		if (IsValidTimer(PCM_DISCONNECTPARTY_TOUT))
		{
			PTRACE(eLevelInfoNormal,"CSipExportPartyCntl::OnPartyPcmStateChangedIdle Pcm Disconnection Completed");
			DeleteTimer(PCM_DISCONNECTPARTY_TOUT);
		}
		if(!m_isXCodeConn)
			SendStartMoveToResourceProcess();
	}
}
/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnTimerPcmDisconnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipExportPartyCntl::OnTimerPcmDisconnect PCM did not completed its disconnection!", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());

	if(!m_isXCodeConn)
	 SendStartMoveToResourceProcess();

}

/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnTimerDisconnectXCodeBrdg(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipExportPartyCntl::OnTimerDisconnectXCodeBrdg Xcode brdg not completed its disconnection!", GetPartyRsrcId());
	DBGPASSERT(GetPartyRsrcId());

	if( (!m_isPcmConnected) ||(!IsValidTimer(PCM_DISCONNECTPARTY_TOUT)))
		SendStartMoveToResourceProcess();

}

/////////////////////////////////////////////////////////////////////////////

void CSipExportPartyCntl::OnTimerRAStartMove(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnTimerRAStartMove - Failed to start move in RA", GetPartyRsrcId());
	PASSERT(101);
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
}
	
/////////////////////////////////////////////////////////////////////////////
void CSipExportPartyCntl::OnEndResourceAllocatorStartMove(CSegment* pParam)
{			
    PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnEndResourceAllocatorStartMove", GetPartyRsrcId());
    // Here we check if status OK and if so we start the MPL_API part
    // Moving the phisical resources (MFA).    
    DWORD  status,targetMonitorConfId, targetResourceConfId, monitorPartyId,rsrcPartyId;
    DeleteTimer(EXPORT_RSRC_TOUT);
    *pParam >> status >> targetMonitorConfId >> targetResourceConfId >> monitorPartyId >> rsrcPartyId;
    ISDEBUGMODE_SET_STATUS("MOVE", 4, STATUS_START_RSRC_PARTY_MOVE_FAIL)
    if (status != STATUS_OK) 
    {
		PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnEndResourceAllocatorStartMove - Failed to start move in RA - Bad status", GetPartyRsrcId());    	
    	PASSERT(status);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }
 	if (monitorPartyId != m_monitorPartyId)
 	{
 		PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnEndResourceAllocatorStartMove - wrong party monitor Id", GetPartyRsrcId());    	
    	PASSERT(monitorPartyId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }
    if (rsrcPartyId != m_pPartyAllocatedRsrc->GetPartyRsrcId())
    {
		PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnEndResourceAllocatorStartMove - wrong party rsrc Id", GetPartyRsrcId());    	
    	PASSERT(rsrcPartyId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    } 
    if(targetMonitorConfId != m_destMonitorConfId)
    {
    	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnEndResourceAllocatorStartMove - wrong target monitor conf Id", GetPartyRsrcId());    	
    	PASSERT(targetMonitorConfId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }

    CSmallString str;
    str << targetResourceConfId;
   	PTRACE2PARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnEndResourceAllocatorStartMove - targetResourceConfId value is: ",str.GetString(), GetPartyRsrcId());    	    
    m_destResourceConfId = targetResourceConfId; 		  
	m_state = EXPORT_MPL;
	
	// Here we send the request to the MPL_API(MFA)
	// But first we will find which logical resources are open by asking the bridges and filling the array.
	// TBD - Waiting for Talya/Matvey interface

	std::map<eLogicalResourceTypes,bool> isOpenedAudioRsrcMap;
	m_pAudioInterface->ArePortsOpened(rsrcPartyId, isOpenedAudioRsrcMap);

	for (std::map<eLogicalResourceTypes,bool>::const_iterator audioRsrcIter = isOpenedAudioRsrcMap.begin(); audioRsrcIter != isOpenedAudioRsrcMap.end(); ++audioRsrcIter)
	{
		eLogicalResourceTypes lrt = audioRsrcIter->first;
		m_activeLogicalRsrc[lrt]  = audioRsrcIter->second;
		// Here we check - If there is an audio logical rsrc open - we open the RTP too
		if (m_activeLogicalRsrc[lrt])
			m_activeLogicalRsrc[eLogical_rtp] = 1;
	}
	
	if ( m_activeLogicalRsrc[eLogical_rtp] == 0 )
	{
		PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnEndResourceAllocatorStartMove - No active resources", GetPartyRsrcId());	
		PASSERT(102);
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return; 
	}

	// temp Eitan (IVR - MOVE!)
	if (!m_bIsMrcCall)
	{
		// Video check
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
			m_activeLogicalRsrc[eLogical_relay_rtp] = IsPortConnected(eLogical_relay_rtp); //mrmp
		}
	}
	else
	{
		m_activeLogicalRsrc[eLogical_relay_rtp] = 1;//IsPortConnected(eLogical_relay_rtp); //mrmp
	}


	ostringstream ostr;
	m_numOfActiveLogicalRsrc = 0;

	MOVE_RESOURCES_REQ_S *pMoveRsrcReq = new MOVE_RESOURCES_REQ_S;
	WORD i = 0;
	for (i = 0; i < NUM_OF_LOGICAL_RESOURCE_TYPES; i++)
		pMoveRsrcReq->openLogicalResources[i] = eRsrcNonActive;

	for(i= 0; i< NUM_OF_LOGICAL_RESOURCE_TYPES; i++ )
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
		PASSERTMSG( GetConfAudioSampleRate()+1, "CSipExportPartyCntl::OnEndResourceAllocatorStartMove - invalid sample rate!");
	}
	pMoveRsrcReq->moveRsrcParams.confAudioSampleRate = eSampleRate;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_destMonitorConfId);
	
	if(pCommConf)
		pMoveRsrcReq->moveRsrcParams.enConfSpeakerChangeMode = pCommConf->GetConfSpeakerChangeMode(); //xifwang
	else
	{
		TRACESTR (eLevelInfoNormal) << "CSipExportPartyCntl::OnEndResourceAllocatorStartMove,couldn't find conf with monitor id: " <<m_destMonitorConfId; 
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
void CSipExportPartyCntl::OnMplApiMoveAck(CSegment* pParam)
{	  	
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnMplApiMoveAck", GetPartyRsrcId());
	BYTE lrt;
	ACK_IND_S sAckIndStrcut;
	pParam->Get((BYTE*)&sAckIndStrcut,sizeof(ACK_IND_S));
	*pParam >> lrt;

	if(sAckIndStrcut.ack_base.status != STATUS_OK)
	{
	    CSmallString str;
	    str << sAckIndStrcut.ack_base.reason;
		PTRACE2PARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnMplApiMoveAck : Move ack is wrong - ",str.GetString(), GetPartyRsrcId());
		// TBD - What are we doing in that case - disconnect the party ?
		DBGPASSERT(sAckIndStrcut.ack_base.reason);
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
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
			// TBD - Guy + Yael + Uri - What are doing in case of wrong opcode? (Ignore/fail)
			DBGPASSERT(sAckIndStrcut.ack_base.ack_opcode);
			break;
		}
	}
    ISDEBUGMODE_SET_STATUS("MOVE", 5, 1001)	// simulate status error from MFA
	if (status != STATUS_OK)
	{
	    CSmallString str;
	    str << lrt;
		PTRACE2PARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnMplApiMoveAck : Move Logical Resource Error - ",str.GetString(), GetPartyRsrcId());
		// TBD - What are we doing in that case - disconnect the party ?
		DBGPASSERT(lrt);
		m_isFaulty = 1;
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
	}
	ISDEBUGMODE_SET_VAL("MOVE", 6, m_numOfActiveLogicalRsrc, 1)	// simulate not all MFA response
	if (m_numOfActiveLogicalRsrc == 0)
	{
		// This means we got all relavent ACKs from the MFA and we may continue the flow.
		MfaAcksCompleted();
	}								
}

/////////////////////////////////////////////////////////////////////////////
WORD CSipExportPartyCntl::CheckIfLogicalRsrcAckAccepted(eLogicalResourceTypes lrt)
{		
	// Checking if the correct ACK was received - 
	// If so, decrease the number of active logical resourse counter by one.
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::CheckIfLogicalRsrcAckAccepted", GetPartyRsrcId());
	
	if (m_activeLogicalRsrc[lrt] == 1)
	{
		m_numOfActiveLogicalRsrc--;
		return STATUS_OK;
	}
	else
	{
		if (lrt == eLogical_audio_encoder || lrt == eLogical_audio_decoder || lrt == eLogical_rtp ||
			lrt == eLogical_relay_audio_encoder || lrt == eLogical_relay_audio_decoder || lrt == eLogical_relay_video_encoder || lrt ==eLogical_relay_svc_to_avc_rtp  )
		{
			TRACESTRFUNC(eLevelError) << "logical rsrc sent extra due to ART/MRMP ind" << ::LogicalResourceTypeToString(lrt) << ", PartyRsrcID=" << GetPartyRsrcId();
			return STATUS_OK;
		}
		else
		{
			TRACESTRFUNC(eLevelError) << "logical rsrc not valid " << ::LogicalResourceTypeToString(lrt) << ", PartyRsrcID=" << GetPartyRsrcId();
			return STATUS_FAIL;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipExportPartyCntl::MfaAcksCompleted()
{	
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::MfaAcksCompleted", GetPartyRsrcId());
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
	*pStrReq <<"CSipExportPartyCntl::Transfer - END_PARTY_MOVE_RSRC_REQ:\n"
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
void CSipExportPartyCntl::OnResourceAllocatorEndMove(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnResourceAllocatorEndMove", GetPartyRsrcId());
	
	DeleteTimer(END_EXPORT_RSRC_TOUT);
    DWORD  status,targetMonitorConfId, targetRsrcConfId, monitorPartyId,rsrcPartyId, numRsrcs;
    DeleteTimer(EXPORT_RSRC_TOUT); 
    *pParam >> status >> targetMonitorConfId >> targetRsrcConfId >> monitorPartyId >> rsrcPartyId >> numRsrcs;

    CMedString *pStr = new CMedString;
 	*pStr <<"CSipExportPartyCntl::OnResourceAllocatorEndMove - END_PARTY_MOVE_RSRC_IND:\n"
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
    ISDEBUGMODE_SET_STATUS("MOVE", 7, 1001)

    if (status != STATUS_OK) 
    {
		PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnResourceAllocatorEndMove - Failed to end move in RA - Bad status", GetPartyRsrcId());    	
    	PASSERT(status);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return; 
    }
 	if (monitorPartyId != m_destMonitorPartyId)
 	{
 		PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnResourceAllocatorEndMove - wrong party monitor Id", GetPartyRsrcId());    	
    	PASSERT(monitorPartyId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }
    if (rsrcPartyId != m_pPartyAllocatedRsrc->GetPartyRsrcId())
    {
		PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnResourceAllocatorEndMove - wrong party rsrc Id", GetPartyRsrcId());    	
    	PASSERT(rsrcPartyId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
		m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
		return;
    }       	
    if(targetMonitorConfId != m_destMonitorConfId)
    {
    	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnResourceAllocatorEndMove - wrong target monitor conf Id", GetPartyRsrcId());    	
    	PASSERT(targetMonitorConfId);
    	// Disconnect the party
		// TBD - Is there any thing to do before ? Matvey. 
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
			if (m_pIpCurrentMode->GetConfType() == kCop || m_bIsMrcCall)
				DisconnectPartyFromVideoBridge();
			else
			{
				CBridgePartyExportParams bridgePartyExportParams(m_pParty->GetPartyId());
				m_pConfAppMngrInterface->ExportPartyVideo(&bridgePartyExportParams);
			}

		}
	}
	if (m_isContentConn)
		DisconnectPartyFromContentBridge();

	if (m_isFeccConn)
		DisconnectPartyFromFECCBridge();
	
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
void  CSipExportPartyCntl::BridgeDisconnetCompleted()                                        
{
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
	DBGPASSERT(1);//still TBD
	//m_pPartyApi->SipPartyDisConnect(IP_BRIDGE_DISCONNECT_WHILE_MOVE);
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
void  CSipExportPartyCntl::OnTimerMPLExport(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnTimerMPLExport - Failed to receive IND from MFA", GetPartyRsrcId());
	PASSERT(105);
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_isFaulty = 1;
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
}
	
/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnTimerRAEndMove(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnTimerRAEndMove - Failed to receive end move from RA", GetPartyRsrcId());
	PASSERT(106);
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
}

/////////////////////////////////////////////////////////////////////////////
void CSipExportPartyCntl::OnTimerExportBridges(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipExportPartyCntl::OnTimerExportBridges - Failed to receive IND from bridges", GetPartyRsrcId());
	PASSERT(107);
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.

 	SetDataForExportPartyFail();
 	StartTimer(EXPORT_FAILED_TOUT,SECOND);
	
}

/////////////////////////////////////////////////////////////////////////////
void  CSipExportPartyCntl::OnTimerExportFailed(CSegment* pParam)
{
	PTRACEPARTYID(eLevelError,"CSipExportPartyCntl::OnTimerExportFailed", GetPartyRsrcId());
	m_pTaskApi->EndExportParty(m_pParty->GetPartyId(),statIllegal);
}
 
 


	
	    	
