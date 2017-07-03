//+========================================================================+
//                            SipImportPartyCntl.cpp                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SipImportPartyCntl.cpp                                     |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 13/11/05   |  Create + Stage 1 - Move between same conferences    |
//+========================================================================+

#include "SIPPartyControlImport.h"
#include "ConfPartyOpcodes.h"
#include "StatusesGeneral.h"
#include "ConfApi.h"
#include "PartyApi.h"
#include "AudioBridgeInterface.h"
#include "ConfAppMngrInterface.h"
#include "VideoBridgeInterface.h"
#include "BridgeMoveParams.h"
 
PBEGIN_MESSAGE_MAP(CSipImportPartyCntl)

	ONEVENT(PARTY_AUDIO_CONNECTED,					IDLE,	CSipImportPartyCntl::OnAudConnect)
 	ONEVENT(PARTY_VIDEO_CONNECTED,					IDLE,	CSipImportPartyCntl::OnVideoBrdgConnected)
 	ONEVENT(PARTY_VIDEO_IVR_MODE_CONNECTED,			IDLE,	CSipImportPartyCntl::OnVideoBrdgConnected)
 	ONEVENT(FECC_PARTY_BRIDGE_CONNECTED,			IDLE,	CSipImportPartyCntl::OnFeccBrdgCon)
	ONEVENT(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE,	IDLE,	CSipImportPartyCntl::OnTimerConnectToAudioBrdg)
    ONEVENT(IMPORT_PARTY_CONNECT_TO_VIDEO_BRIDGE,	IDLE,	CSipImportPartyCntl::OnTimerConnectToVideoBrdg)
    ONEVENT(IMPORT_PARTY_CONNECT_TO_FECC_BRIDGE,	IDLE,	CSipImportPartyCntl::OnTimerConnectToFeccBrdg)
    ONEVENT(PARTY_VIDEO_IN_UPDATED,	                IDLE,	CSipImportPartyCntl::NullActionFunction)
    ONEVENT(PARTY_VIDEO_OUT_UPDATED,		        IDLE,	CSipImportPartyCntl::NullActionFunction)
PEND_MESSAGE_MAP(CSipImportPartyCntl,CSipPartyCntl);

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

CSipImportPartyCntl::CSipImportPartyCntl() //Ctor
{
	m_isImportDone = FALSE; //BRIDGE-13498
}

/////////////////////////////////////////////////////////////////////////////
CSipImportPartyCntl::~CSipImportPartyCntl() // Dtor
{
	
}

/////////////////////////////////////////////////////////////////////////////
CSipImportPartyCntl& CSipImportPartyCntl::operator =(const CSipImportPartyCntl& other)
{
	(CSipPartyCntl&)*this = (CSipPartyCntl&)other;
	
	m_isImportDone = other.m_isImportDone; //BRIDGE-13498

	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void*  CSipImportPartyCntl::GetMessageMap()                                        
{
  return (void*)m_msgEntries;    
}

/////////////////////////////////////////////////////////////////////////////
const char*   CSipImportPartyCntl::NameOf()  const
{
  return "CSipImportPartyCntl";
}

/////////////////////////////////////////////////////////////////////////////
// This function is the most importent function in the move party scenario.
// we create the CPartyCntl in the target coference for the new party as in CAddH323PartyCntl::Create
// but using existing allready opened channels, and resources (H323,audio).

void  CSipImportPartyCntl::Create(CMoveIPImportParams* pMoveImportParams)
{
	PASSERT_AND_RETURN(! CPObject::IsValidPObjectPtr(pMoveImportParams->m_pImpConfPartyCntl));
	PASSERT_AND_RETURN(! CPObject::IsValidPObjectPtr(pMoveImportParams->m_pConf));
		
	m_state = IDLE;
	WORD IsVoiceOnlyParty = (pMoveImportParams->m_pImpConfPartyCntl)->GetVoice();
	
	m_isImportDone = FALSE; //BRIDGE-13498

	// 1) copy data from source-party CPartyCntl
	//	  here we copy all the relevant party data that doesn't depend on the new conference. 
	SetDataForImportPartyCntl(pMoveImportParams->m_pImpConfPartyCntl);
	m_eLastAllocatedVideoPartyType = pMoveImportParams->m_eLastAllocatedVideoPartyType;

    // 1.a  register all party control state machines in the current task
    if (m_pBridgeMoveParams)
    {
        m_pBridgeMoveParams->RegisterBridgePartyCntlsInTask();
    }
    
    // 2) set data from new conference
	//	  here we set the new conference relevant data in the CPartyCntl
	m_pConf        = pMoveImportParams->m_pConf;
	SetFullName(pMoveImportParams->m_pImpConfPartyCntl->GetName(),pMoveImportParams->m_name);
	PTRACE2PARTYID(eLevelInfoNormal, "CSipImportPartyCntl::Create: Name - ", m_partyConfName, GetPartyRsrcId());
	
	m_pTaskApi = new CConfApi;
	m_pTaskApi->CreateOnlyApi((pMoveImportParams->m_pConf)->GetRcvMbx(),this);
	m_pTaskApi->SetLocalMbx((pMoveImportParams->m_pConf)->GetLocalQueue());

	UpdatePartyEntryInGlobalRsrcRoutingTblAfterMove();
/* 323 only parameters
	// reset flow control parameters
	m_bIsFlowControlParty = 0;
	m_flowControlRate = 0;
*/
	
	if (pMoveImportParams->m_pAudBridgeInterface != NULL)
		m_pAudioInterface = (pMoveImportParams->m_pAudBridgeInterface);
		
	if (pMoveImportParams->m_pVidBridgeInterface != NULL)
		m_pVideoBridgeInterface = (pMoveImportParams->m_pVidBridgeInterface);
		
	if (pMoveImportParams->m_pConfAppMngrInterface != NULL)
		m_pConfAppMngrInterface = (pMoveImportParams->m_pConfAppMngrInterface);
		
	if (pMoveImportParams->m_pContentBridge != NULL)
		m_pContentBridge = (pMoveImportParams->m_pContentBridge);
	
	if (pMoveImportParams->m_pFECCBridge != NULL)
		m_pFECCBridge = (pMoveImportParams->m_pFECCBridge);

		
	if (pMoveImportParams->m_pTerminalNumberingManager != NULL)
		m_pTerminalNumberingManager = (pMoveImportParams->m_pTerminalNumberingManager);				
	
	if (pMoveImportParams->m_pCopVideoTxModes != NULL)
	{
		POBJDELETE(m_pCopVideoTxModes);
		m_pCopVideoTxModes	= new CCopVideoTxModes(*(pMoveImportParams->m_pCopVideoTxModes));
	}

	// 3) set communication mode and local capabilities
	m_IsMovedParty = TRUE;

	//m_pIpInitialMode->SetVideoBitRate(m_videoRate, cmCapReceiveAndTransmit);
	//m_pPartyApi->SetRsrcConfIdForInterface(m_destResourceConfId);
	//PTRACE2INT(eLevelInfoNormal,"CSipImportPartyCntl::Create rate is  - ",m_videoRate);

	// BRIDGE-15567: update SCM if Conference Profile is changed
	UpdateScmAccordingtoRes(pMoveImportParams->m_pImpConfPartyCntl);

	m_pPartyHWInterface	= new CPartyInterface(*((pMoveImportParams->m_pImpConfPartyCntl)->GetPartyCntlResource()));
	m_pPartyHWInterface->SetConfRsrcId(m_destResourceConfId);
		// b) set resourses descriptors that allocated/dealocaeted at CConf level

	// set to new conf id only after using old conf id for dealocating resources
    m_monitorConfId    = pMoveImportParams->m_sysConfId;
    
	CBridgePartyInitParams* pBrdgPartyInitParams = new CBridgePartyInitParams(m_name, m_pParty, 
																			  GetPartyRsrcId(), m_RoomId,
																			  GetInterfaceType(),
																			  NULL, NULL,																			  
												                              m_pBridgeMoveParams->GetAndResetAudioBridgePartyCntlOnImport()); 
	bool isIvrAfterMove = false;
	if(m_moveType == eMoveIntoIvr)
			ON(isIvrAfterMove);
	m_pConfAppMngrInterface->ConnectPartyAudio(pBrdgPartyInitParams,isIvrAfterMove);

	m_eAudBridgeConnState = eSendOpenInAndOut;
	ON(m_bIsMemberInAudBridge);
	
	StartTimer(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE, 8*SECOND);

	// connect video bridge only on CP or equal mode in VSW
	if (m_pIpCurrentMode->IsMediaOn(cmCapVideo) && m_eConfTypeForAdvancedVideoFeatures == NO && (m_eVidBridgeConnState == eBridgeDisconnected))
		ConnectPartyToVideoBridge(m_pIpCurrentMode);
	StartTimer(IMPORT_PARTY_CONNECT_TO_VIDEO_BRIDGE, 8*SECOND);

	//FECC:
	if(m_pIpCurrentMode->IsMediaOn(cmCapData) && !m_isFeccConn && !m_bIsMrcCall)
	{
		ConnectPartyToFECCBridge(m_pIpCurrentMode);
		StartTimer(IMPORT_PARTY_CONNECT_TO_FECC_BRIDGE, 8*SECOND);
	}
	
	WORD refMcuNumber = 1;
	WORD refTerminalNumber = 1;
	if (m_pTerminalNumberingManager)
   	{
   		m_pTerminalNumberingManager->allocatePartyNumber(m_pParty, refMcuNumber, refTerminalNumber);
   		ON(m_isTerminalNumberingConn);
   	}
	else
		PASSERTMSG(GetPartyRsrcId(),"CSipImportPartyCntl::Create - m_pTerminalNumberingManager not valid ");
		
    if (m_pConf->GetVideoOperationPointsSet())
    {
        TRACEINTO << "@#@ ";
        m_pIpInitialMode->SetOperationPoints(m_pConf->GetVideoOperationPointsSet());
        m_pIpCurrentMode->SetOperationPoints(m_pConf->GetVideoOperationPointsSet());
    }
    else
    {
        TRACEINTO << "mix_mode: no operation points at conference level.";
    }
	EConfType confType = m_pIpInitialMode->GetConfType();
	m_pPartyApi->SetMoveDestConfParams( (WORD)confType, refMcuNumber, refTerminalNumber, m_pCopVideoTxModes, m_pConf->GetVideoOperationPointsSet());
	POBJDELETE(pBrdgPartyInitParams);	
}



/////////////////////////////////////////////////////////////////////////////
//              CImportIpPartyCntl Action Functions
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// This function ends the import side of the move party scenario successfuly.
// Audio connected end we continue to "SetCapCommonDenominator" and "ChangeMode" as any
// other new party in the conference.
void  CSipImportPartyCntl::OnAudConnect(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipImportPartyCntl::OnAudConnect", GetPartyRsrcId());
    DeleteTimer(IMPORT_PARTY_CONNECT_TO_AUDIO_BRIDGE);

    if (m_eAudBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CSipImportPartyCntl::OnAudConnect : Connect has received after disconnect.", GetPartyRsrcId());
	}    
	
	HandleAudioBridgeConnectedInd(pParam);
	   
    CheckBridgeConnection();
}

/////////////////////////////////////////////////////////////////////////////
// This function ends the import side of the move party scenario successfuly.
// Audio connected end we continue to "SetCapCommonDenominator" and "ChangeMode" as any
// other new party in the conference.
/////////////////////////////////////////////////////////////////////////////
void CSipImportPartyCntl::OnVideoBrdgConnected(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipImportPartyCntl::OnVideoBrdgConnected", GetPartyRsrcId());
    DeleteTimer(IMPORT_PARTY_CONNECT_TO_VIDEO_BRIDGE);
	if (m_eVidBridgeConnState == eBridgeDisconnected)
	{
		DBGPASSERT(GetPartyRsrcId());
		PTRACEPARTYID(eLevelInfoNormal,"CSipImportPartyCntl::OnVideoBrdgConnected : Connect has received after disconnect.", GetPartyRsrcId());
	}
	
	HandleVideoBridgeConnectedInd(pParam);	
	UpdatePartyStateAfterVideoBridgeConnected();
	
	if (IsOutDirectionConnectedToVideoBridge()) 
	    CheckBridgeConnection();	
}

/////////////////////////////////////////////////////////////////////////////
void CSipImportPartyCntl::OnFeccBrdgCon(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipImportPartyCntl::OnFeccBrdgCon", GetPartyRsrcId());
	DeleteTimer(IMPORT_PARTY_CONNECT_TO_VIDEO_BRIDGE);
	//m_pPartyApi->SendFeccBridgeConnected();
	CIpPartyCntl::OnFeccBrdgCon(pParam);
	CheckBridgeConnection();
}

/////////////////////////////////////////////////////////////////////////////
void  CSipImportPartyCntl::CheckBridgeConnection()
{
	BYTE bConnected = IsAtLeastOneDirectionConnectedToAudioBridge();
	TRACEINTO<<"!@# starting ";
	if (bConnected) //if audio is connected - cheack video connection
	{
	        TRACEINTO<<"!@# audio already connected ";
		if (m_pIpCurrentMode->IsMediaOn(cmCapVideo))
			bConnected = IsAtLeastOneDirectionConnectedToVideoBridge();
	}
	
	if (bConnected) //if audio + video is connected - cheack fecc connection
	{
	        TRACEINTO<<"!@# video already connected ";
		if (m_pIpCurrentMode->IsMediaOn(cmCapData) && !m_bIsMrcCall)
			bConnected = m_isFeccConn;
	}
	
	if (bConnected)
	  {
	    TRACEINTO<<"!@# fecc bridge already connected";
	        TRACEINTO<<"!@# all bridges are connected...sending EndImportParty to conference ";
	    m_isImportDone = TRUE; //BRIDGE-13498
	    m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statOK);
	  }
}


/////////////////////////////////////////////////////////////////////////////
void  CSipImportPartyCntl::OnTimerConnectToAudioBrdg(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipImportPartyCntl::OnTimerConnectToAudioBrdg", GetPartyRsrcId());	
	PASSERT(111);
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statIllegal);
} 


/////////////////////////////////////////////////////////////////////////////
void  CSipImportPartyCntl::OnTimerConnectToVideoBrdg(CSegment* pParam)
{

	PTRACEPARTYID(eLevelInfoNormal,"CSipImportPartyCntl::OnTimerConnectToVideoBrdg", GetPartyRsrcId());	
	PASSERT(112);
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statIllegal);
} 

////////////////////////////////////////////////////////////////////////////
void  CSipImportPartyCntl::OnTimerConnectToFeccBrdg(CSegment* pParam)
{

	PTRACEPARTYID(eLevelInfoNormal,"CSipImportPartyCntl::OnTimerConnectToFeccBrdg", GetPartyRsrcId());	
	PASSERT(113);
	// Disconnect the party
	// TBD - Is there any thing to do before ? Matvey.
	m_pTaskApi->EndImportParty(m_pParty->GetPartyId(), statIllegal);
} 

//////////////////////////////////////////////////////////////////////////////
void CSipImportPartyCntl::UpdateScmAccordingtoRes(CPartyCntl* pImpPartyCntl)
{
	const CCommConf* pCommConf = m_pConf->GetCommConf();
	if (pCommConf)
	{
		BYTE impTipContentCompatibility = eTipCompatibleNone;
		if (pImpPartyCntl->GetConf())
		{
			const CCommConf* pImpCommConf = pImpPartyCntl->GetConf()->GetCommConf();
			if (pImpCommConf)
				impTipContentCompatibility = pImpCommConf->GetIsTipCompatible();
			else
			{
				impTipContentCompatibility = ((CIpPartyCntl*)pImpPartyCntl)->GetInitialMode()->GetTipContentMode();
			}
		}

		if (pCommConf->GetIsTipCompatible() != impTipContentCompatibility)
		{
			m_pIpInitialMode->SetContentProtocolMode((ePresentationProtocol)(pCommConf->GetPresentationProtocol()));
			m_pIpInitialMode->SetTipContentMode(pCommConf->GetIsTipCompatible());			
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//    CSS plugin: Adhoc enhancement
PBEGIN_MESSAGE_MAP(CSipPluginImportPartyCntl)
PEND_MESSAGE_MAP(CSipPluginImportPartyCntl,CSipImportPartyCntl);
/////////////////////////////////////////////////////
CSipPluginImportPartyCntl::CSipPluginImportPartyCntl()
{

}
////////////////////////////////////////////////////
CSipPluginImportPartyCntl::~CSipPluginImportPartyCntl()
{

}
/////////////////////////////////////////////////////
const char*   CSipPluginImportPartyCntl::NameOf()  const
{
 	return "CSipPluginImportPartyCntl";
}
//////////////////////////////////////////////////////
void  CSipPluginImportPartyCntl::Create(CMoveIPImportParams* pMoveImportParams)
{
	TRACEINTO << " create for CSS Plugin .";
	CSipImportPartyCntl::Create(pMoveImportParams);

}

//////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPluginImportPartyCntl::GetPossibleContentRate() const
{
	DWORD ContentRate = m_pSipRemoteCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation);
	PTRACE2INT(eLevelInfoNormal,"CSipPluginImportPartyCntl::GetPossibleContentRate: ", ContentRate);	
	return ContentRate;
}
//////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPluginImportPartyCntl::GetMinContentPartyRate(DWORD currContentRate)
{
	DWORD MinContentRate = 0;

	DWORD remoteRate = GetPossibleContentRate() * 100;
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	BYTE xConfRate = pCommConf->GetConfTransferRate();
	CUnifiedComMode unifiedComMode(pCommConf->GetEnterpriseModeFixedRate(),xConfRate,pCommConf->GetIsHighProfileContent());
	

	CCapSetInfo lCapInfo = eUnknownAlgorithemCapCode;
	DWORD confRate    = lCapInfo.TranslateReservationRateToIpRate(xConfRate) * 1000;
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)pCommConf->GetEnterpriseMode();
	DWORD possibleConfContentRate = unifiedComMode.FindIpContentRateByLevel(confRate,ContRatelevel, (ePresentationProtocol)pCommConf->GetPresentationProtocol(),
																			pCommConf->GetCascadeOptimizeResolution(), pCommConf->GetConfMediaType());

	MinContentRate = min(possibleConfContentRate,remoteRate);
	if (m_lastMinContentRate != MinContentRate)
	{
		CLargeString cstr;
		cstr << "CSipPluginImportPartyCntl::GetMinContentPartyRate\n";
		cstr << "remote Content Rate: " << remoteRate << ", conf Content Rate: " << currContentRate  << "\n";
		cstr << ", confRate: " << confRate << "\n";
		cstr << "possible content rate in conf (according to party's call rate) is: " << possibleConfContentRate << "\n";
		cstr << "chosen rate is: " << MinContentRate;
		PTRACE(eLevelInfoNormal,cstr.GetString());
		m_lastMinContentRate = MinContentRate;
	}
	PTRACE2INT(eLevelInfoNormal,"CSipPluginImportPartyCntl::GetMinContentPartyRate: ", MinContentRate);	
	return MinContentRate;
}





