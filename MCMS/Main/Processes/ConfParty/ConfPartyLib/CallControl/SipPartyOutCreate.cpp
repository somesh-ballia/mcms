//+========================================================================+
//                            SIPPartyOutCreate.cpp                        |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyOutCreate.cpp                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 19/03/08   | This file contains								   |
//     |            |                                                      |
//+========================================================================+
#include "Segment.h"
#include "StateMachine.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "Trace.h"
#include "Macros.h"
#include "NStream.h"
#include "NetSetup.h"
#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "Conf.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "IpCsOpcodes.h"
#include "TaskApi.h"
#include "Party.h"
#include "PartyApi.h"
#include "SipDefinitions.h"
#include "SIPCommon.h"
#include "IpNetSetup.h"
#include "SipNetSetup.h"
#include "CsInterface.h"
#include "SipScm.h"
#include "SipCall.h"
#include "ConfApi.h"
#include "IPParty.h"
#include "SIPControl.h"
#include "SIPParty.h"
#include "SipPartyOutCreate.h"
#include "IpServiceListManager.h"
#include "IPParty.h"

void SipPartyOutCreateEntryPoint(void* appParam)
{
	CSipPartyOutCreate* pPartyTaskApp = new CSipPartyOutCreate;
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipPartyOutCreate)

ONEVENT(ALLOCATE_PARTY_RSRC_IND,		PARTYIDLE,				CSipPartyOutCreate::OnConfInformResourceAllocatedIdle)//Idle Case
ONEVENT(SIP_CONF_ESTABLISH_CALL,		PARTYIDLE,				CSipPartyOutCreate::OnConfEstablishCallIdle)
ONEVENT(CONFDISCONNECT,					PARTYIDLE,				CSipPartyOutCreate::OnConfCloseCallIdle)
ONEVENT(PARTY_TRANSLATOR_ARTS_CONNECTED,	sPARTY_ALLOCATE_TRANSLATOR_ARTS,	CSipPartyOutCreate::OnPartyTranslatorArtsConnected)
// DNS Resolution
//ONEVENT(SIP_PARTY_DNS_RES,			PARTYIDLE,				CSipPartyOutCreate::OnDnsResolutionCallIdle)
//ONEVENT(DNSQUARYTOUT,					PARTYIDLE,				CSipPartyOutCreate::OnSipDisconnectIdle)

ONEVENT(SET_SITE_AND_VISUAL_NAME,		ANYCASE,				CSipParty::SendSiteAndVisualNamePlusProductIdToPartyControl)

ONEVENT(CONFDISCONNECT,					PARTYDISCONNECTING,		CSipParty::OnConfReadyToCloseCall)
ONEVENT(CONFDISCONNECT,					ANYCASE,				CSipPartyOutCreate::OnConfCloseCall)

// Self timers
ONEVENT(PARTYCONTOUT  	 				,PARTYIDLE,				CSipPartyOutCreate::OnSipDisconnectIdle)
ONEVENT(PARTYCONTOUT  	 				,ANYCASE,				CSipPartyOutCreate::NullActionFunction)

//ppc
ONEVENT(SIP_PARTY_BFCP_MSG_IND,			sPARTY_CONNECTING,		CSipParty::OnPartyBfcpMsgInd)
ONEVENT(SIP_PARTY_BFCP_TRANSPORT_IND,	sPARTY_CONNECTING,		CSipParty::OnPartyBfcpTransportInd)

ONEVENT(SIP_CONF_BRIDGES_UPDATED,       PARTYDISCONNECTING,  	CSipPartyOutCreate::NullActionFunction)
PEND_MESSAGE_MAP(CSipPartyOutCreate,CSipParty);

///////////////////////////////////////////////////////
CSipPartyOutCreate::CSipPartyOutCreate()
{
	m_minValForGlareTimer = 210; // ticks //BRIDGE-8393 - changed to 210-400 as per rfc was 0-200
	m_maxValForGlareTimer = 400; // ticks
	VALIDATEMESSAGEMAP
}

///////////////////////////////////////////////////////
CSipPartyOutCreate::~CSipPartyOutCreate()
{
}

///////////////////////////////////////////////////////
void CSipPartyOutCreate::Create(CSegment& appParam)
{
	CSipParty::Create(appParam);

#ifdef MESSAGE_QUEUE_BLOCKING_RESERCH
	m_pConfApi = new CConfApi(GetMonitorConfId());
#else
	m_pConfApi = new CConfApi;
#endif
	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx, NULL, NULL, 1);

	// update initial number of retries
	DWORD redialnum = GetSystemCfgFlagInt<DWORD> (CFG_KEY_NUMBER_OF_REDIAL);
	m_pConfApi->UpdateDB(this, NUMRETRY, (DWORD) redialnum, 1);
}

///////////////////////////////////////////////////////
void CSipPartyOutCreate::CleanUp()
{
	CSmallString str;
	str << "Name " << m_partyConfName << " Dial state " << m_eDialState;
	PTRACE2(eLevelInfoNormal,"CSipPartyOutCreate::CleanUp: ", str.GetString());

	EndTransactionByPartyIfNeeded();

	switch(m_eDialState)
	{
		// before invite sent to Ep (could be internal failure)
	case kCapsDontMatch:
	case kChannelsFailedToOpen:
	case kTerminateByConf:
	case kBeforeInvite:
	case kNotInDialState:
		if (m_isPreSignalingFlowProb == 0)
			m_pSipCntl->CloseCall(YES);
		break;

		// after invite sent to Ep before answer arrived
	case kCancelInvite:
	case kInviteSent:
		m_pSipCntl->CancelCall(kCancelAfterInvite);
		break;

		// after answer arrived from Ep
	case kRejectArrived:
	case kTerminateByRemote:
		m_pSipCntl->CloseCall(NO); // send ack for the reject
		break;

	case kNoRecovery:
	case kNoRecoveryForVideo:
	case kGuessSucceeded:
		m_pSipCntl->SipInviteAckReq();	// for the 200 ok
		m_pSipCntl->CloseCall(YES);		// send bye
		break;

	case kBadStatusArrived:
	case kTransportErrorArrived:
	case kDisconnectTimer:
		m_pSipCntl->ViolentCloseCall();
		break;

	case kReInviteSent:			// wait for answer
	case kReInviteAccepted:
		PTRACE2INT(eLevelInfoNormal,"CSipPartyOutCreate::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
		break;

	case kReInviteRejected://wait for an ack to arrive (or timer to popped)
		PTRACE2INT(eLevelInfoNormal,"CSipPartyOutCreate::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
		m_eDialState = kTerminateByConf;
		break;

	case kConnectTimer:
		m_pSipCntl->CloseCall(YES);
		break;

	case kReInviteArrived:
		m_pSipCntl->SipInviteResponseReq(SipCodesDecline); // wait for ack
		break;

	case kByeArrived:
		m_pSipCntl->CloseCall(NO);
		break;

	default:
		if(m_eDialState == 0)
			DBGPASSERT(kLastDialState+1);
		else
			DBGPASSERT(m_eDialState);
		break;
	}
}


///////////////////////////////////////////////////////
void CSipPartyOutCreate::OnConfInformResourceAllocatedIdle(CSegment* pParam)
{// this message is used to initiate ringing in dial out
	PTRACE2(eLevelInfoNormal,"CSipPartyOutCreate::OnConfInformResourceAllocated. Do nothing wait for conf establish call command: Name ",m_partyConfName);
}

///////////////////////////////////////////////////////
void CSipPartyOutCreate::OnConfCloseCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutCreate::OnConfCloseIdleCall: Name ",m_partyConfName);
	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);
//	if (IsValidTimer(DNSQUARYTOUT))
//		DeleteTimer(DNSQUARYTOUT);

	if(m_eDialState == kNotInDialState)
		DestroyPartyTask();
	else
		DBGPASSERT(m_eDialState);
}

///////////////////////////////////////////////////////
void CSipPartyOutCreate::OnConfCloseCall(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutCreate::OnConfCloseCall: Name ",m_partyConfName);
	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);
//	if (IsValidTimer(DNSQUARYTOUT))
//		DeleteTimer(DNSQUARYTOUT);

	m_state = PARTYDISCONNECTING;
	LogicalChannelDisconnect(SDP);

	switch(m_eDialState)
	{
	case kBeforeInvite:
		m_eDialState = kTerminateByConf;
		break;
	case kInviteSent:
		m_eDialState = kCancelInvite;
		break;
	default:; // ok arrived (or any other dial state) - don't change dial state
	}

	CleanUp();
}


///////////////////////////////////////////////////////
void CSipPartyOutCreate::OnConfEstablishCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutCreate::OnConfEstablishCallIdle: Name ",m_partyConfName);
	BYTE bCapsDontMatch	= NO;

	CCommConf* pCommConf 	= ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
	PASSERTMSG_AND_RETURN(!pCommConf, "pCommConf = NULL");
	CConfParty* pConfParty 	= pCommConf->GetCurrentParty(GetMonitorPartyId());
	PASSERTMSG_AND_RETURN(!pConfParty, "pConfParty = NULL");

	CRsrcParams* pMrmpRsrcParams = NULL;
    CRsrcParams* pMfaRsrcParams = new CRsrcParams;
	CRsrcParams* pCsRsrcParams = new CRsrcParams;

	CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		avcToSvcTranslatorRsrcParams[i]=NULL;
	}


	UdpAddresses sUdpAddressesParams;
	CSipNetSetup*	pNetSetup		= new CSipNetSetup;
	CSipCaps*		pLocalCaps		= new CSipCaps;
	CSipCaps*       pMaxLocalCaps   = new CSipCaps;
	CQoS*			pQos			= new CQoS;
	DWORD			confParamLen	= 0;
	char*			strConfInfo		= NULL;
	BYTE			eTransportType	= 0;
	DWORD			addrLen			= 0;

	pNetSetup->DeSerialize(NATIVE,*pParam);
	pLocalCaps->DeSerialize(NATIVE,*pParam);
	m_pTargetMode->DeSerialize(NATIVE,*pParam);
	pQos->DeSerialize(NATIVE,*pParam);
	*pParam >> m_bIsAdvancedVideoFeatures;

	 if( (eMsftAvmcu2013 == pConfParty->GetMsftAvmcuState()
		  || (eMsftAvmcu2010 == pConfParty->GetMsftAvmcuState())))
	 {
	   PTRACE(eLevelInfoNormal, "CSipPartyOutCreate::OnConfEstablishCallIdle  AV-MCU updating Header");
		UpdateNetSetupToStrForSDP(pNetSetup);
		pNetSetup->SetCallIndex(0);
	 }
	 if( (eMsftAvmcu2013 == pConfParty->GetMsftAvmcuState()))
	 {
		  PTRACE2(eLevelInfoNormal, "CSipPartyOutCreate::OnConfEstablishCallIdle  AV-MCU  Lync2013 to:",pNetSetup->GetToPartyAddress());
		  m_pSipCntl->SetIsMs2013(eMsft2013AvMCU);
		   // pConfParty->SetCascadeMode(CASCADE_MODE_MASTER);
	 }
	 else
	 {
			m_pSipCntl->SetIsMs2013(eMsft2013None);
	 }

    *pParam >> confParamLen;
	if(confParamLen)
	{
		strConfInfo = new char[confParamLen];
		pParam->Get((BYTE*)strConfInfo,confParamLen);
		strConfInfo[confParamLen-1]=0;
	}

	*pParam >> eTransportType;
    pMfaRsrcParams->DeSerialize(NATIVE, *pParam);
	pCsRsrcParams->DeSerialize(NATIVE, *pParam);

	DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams);
	int cnt=0;
	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i], "!@#  translator" );
		if (avcToSvcTranslatorRsrcParams[i])
		     cnt++;
	}

	pParam->Get((BYTE *) &sUdpAddressesParams, sizeof(UdpAddresses));

	*pParam >> m_mcuNum;
	*pParam >> m_termNum;

    BYTE bIsCopVideoTxModes = FALSE;
	*pParam >> bIsCopVideoTxModes;
	if (bIsCopVideoTxModes)
	{
		CCopVideoTxModes* pTempCopVideoTxModes = new CCopVideoTxModes;
		pTempCopVideoTxModes->DeSerialize(NATIVE,*pParam);
		m_pSipCntl->SetCopVideoTxModes(pTempCopVideoTxModes);
		POBJDELETE(pTempCopVideoTxModes);
	}


	*pParam >> addrLen;//only in dial out
	if (addrLen)
	{
		PDELETEA(m_alternativeAddrStr);
		m_alternativeAddrStr = new char[addrLen];
		memset( m_alternativeAddrStr, 0, addrLen);
		pParam->Get((BYTE*)m_alternativeAddrStr,addrLen);
		m_alternativeAddrStr[addrLen-1]=0;
	}
	BYTE bNoVideRsrcForVideoParty = 0;
	*pParam >> bNoVideRsrcForVideoParty;
	m_pTargetModeMaxAllocation->DeSerialize(NATIVE,*pParam);  //dpa
	pMaxLocalCaps->DeSerialize(NATIVE,*pParam);
	*pParam >> m_RoomId;

    CLargeString cstr;
    if(m_pTargetMode->GetConfMediaType() == eMixAvcSvc || m_pTargetMode->GetConfMediaType()==eMixAvcSvcVsw)
    {
		m_SsrcIdsForAvcParty.DeSerialize(*pParam);
		m_SsrcIdsForAvcParty.Print("CSipPartyOutCreate::OnConfEstablishCallIdle");
    }

    BYTE bIsMrcCall = FALSE;
    *pParam >> bIsMrcCall;

    BYTE bIsASSIPContentEnable;
    *pParam >> bIsASSIPContentEnable;

    DWORD partyContentRate;
    *pParam >> partyContentRate;
     m_PartyContentRate = partyContentRate;



	m_PartyRsrcID = pCsRsrcParams->GetPartyRsrcId();
	m_ConfRsrcId = pCsRsrcParams->GetConfRsrcId();
	m_pSipCntl->Create(this, pNetSetup, /*pRsrcDesc,*/ YES, m_serviceId, m_RoomId);//only for dial out
	m_pSipCntl->SetLocalCaps(*pLocalCaps);
	m_pSipCntl->SetMaxLocalCaps(*pMaxLocalCaps);//DPA
	m_pSipCntl->SetFullContentRate(pLocalCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation));//***ppc can be removed when dpa implemented.
	m_pSipCntl->SetLocalSdesKeysAndTag(m_pTargetMode,m_pTargetModeMaxAllocation);
	//========================
	// Retrieving Qos Params
	//========================
	BYTE 		precedRPrio;
	const char*	precedDomain;
	if (m_pSipCntl-> GetQosParams(precedDomain, precedRPrio))
	{
		pQos -> AssembleValFromRPrio(precedDomain, precedRPrio);
	}

	m_pSipCntl->SetQos(*pQos);
	m_pSipCntl->SetConfParamInfo(strConfInfo);
	m_pSipCntl->SetTransportType((enTransportType) eTransportType);	//only for dial out
	TRACEINTO << "mix_mode: setting conf media type to " << m_pTargetMode->GetConfMediaType();
    m_pSipCntl->SetConfMediaType(m_pTargetMode->GetConfMediaType());


	m_pSipCntl->SetControllerResource(pMfaRsrcParams, pCsRsrcParams,sUdpAddressesParams);
	m_pSipCntl->SetInternalControllerResource(avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);

	m_pSipCntl->AddToRoutingTable();
	m_pSipCntl->AddToInternalRoutingTable();
	m_pSipCntl->SetIsEnableICE(pNetSetup->GetEnableSipICE());
	m_pSipCntl->SetITPRtcpMask();
	
	m_pSipCntl->DecideAnatSelectedIpVersion();  //added for ANAT
	
	if(pConfParty &&  pConfParty->GetMsftAvmcuState() == eMsftAvmcuNone)
	{
		m_pSipCntl->CreateSipBfcpCtrl();
		m_pSipCntl->SetBFCPcapConfIDfield(m_mcuNum);
		m_pSipCntl->SetBFCPcapUserIDfield(m_termNum);
	}

	m_pSipCntl->SetIsMrcCall(bIsMrcCall);

    m_pSipCntl->SetASSIPContent(bIsASSIPContentEnable);

	POBJDELETE(pMrmpRsrcParams);
    POBJDELETE(pMfaRsrcParams);
	POBJDELETE(pCsRsrcParams);




	POBJDELETE(pNetSetup);
	POBJDELETE(pLocalCaps);
	POBJDELETE(pQos);
	PDELETEA(strConfInfo);
	POBJDELETE(pMaxLocalCaps);


	 if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);
#if 0
	m_state = sPARTY_CONNECTING;

	StartTransaction(kSipTransInviteWithSdpReq, SIP_PARTY_ESTABLISH_CALL);
#endif
	  if(m_pTargetMode->GetConfMediaType() == eMixAvcSvc && cnt>0)
	{
		m_state = sPARTY_ALLOCATE_TRANSLATOR_ARTS;
		  m_pSipCntl->OpenInternalArts(E_NETWORK_TYPE_IP,cnt);

		for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
		{
		 POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
		}
		return;
	}

	for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
	}

	// IF YOU NEED TO ADD MORE LOGIC, ADD IT INSIDE THIS FUNCTION!!!
	ContinueEstablishCall();
}

void CSipPartyOutCreate::OnPartyTranslatorArtsConnected()
{
	ContinueEstablishCall();
}

void CSipPartyOutCreate::ContinueEstablishCall()
{
	m_state = sPARTY_CONNECTING;
	ESipTransactionType eSipTransType = kSipTransInviteWithSdpReq;
	if (m_pSipCntl->GetIsMrcCall() && (m_pSipCntl->GetCascadeMode() == CASCADE_MODE_SLAVE))
			eSipTransType = kSipTransInviteMrcSlaveWithSdpReq;
	StartTransaction(eSipTransType, SIP_PARTY_ESTABLISH_CALL);
}


/////////////////////////////////////////////////////////////////////
void CSipPartyOutCreate::OnSipDisconnectIdle(CSegment * pParam)
{
	PASSERTMSG(m_PartyRsrcID,"CSipPartyOutCreate::OnSipDisconnectIdle: Disconnect call");
	// The m_isPreSignalingFlowProb indicates that we are disconnecting the call before the signaling
	m_isPreSignalingFlowProb = 1;
	// Activate "Kill Port"
	OnPartySendFaultyMfaToPartyCntlAnycase(pParam);
	BYTE 	mipHwConn = (BYTE)eMipConnectionProcess;
	BYTE	mipMedia = (BYTE)eMipNoneMedia;
	BYTE	mipDirect = (BYTE)eMipOut;
	BYTE	mipTimerStat = (BYTE)eMipTimer;
	BYTE	mipAction = (BYTE)eMipConnect;
	DWORD MpiErrorNumber = ::CalculateMcuInternalProblemErrorNumber(mipHwConn,mipMedia,mipDirect,mipTimerStat,mipAction);

	TellConfOnDisconnecting(MCU_INTERNAL_PROBLEM,NULL,MpiErrorNumber);
	m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
	DispatchPartyCallClosed();// disconnect the call.
}



/////////////////////////////////////////////////////////////////////
void CSipPartyOutCreate::OnPartyCallFailed(CSegment* pParam)
{
	DWORD reason = 0xFFFFFFFF;
	DWORD MipErrorNumber = 0;
	*pParam >> reason;
	PTRACE2INT(eLevelInfoNormal,"CSipPartyOutCreate::OnPartyCallFailed: Reason - ",reason);
	const char* alternativeAddrStr = NULL;

//HIGH_REJECT_VAL == SipPseudoCodesNotAcceptedInHereLyncDialOut, to avoid the ASSERT after receiving 488 for lync!! 
	if ((reason >= LOW_REJECT_VAL && reason < HIGH_REJECT_VAL) || (reason == SipPseudoCodesNotAcceptedInHereLyncDialOut) || (reason == SipPseudoCodesUnsuppMediaTypeDialOut))
	{
		SetDialState(kRejectArrived);

		if (reason == SipCodesMovedPerm || reason == SipCodesMovedTemp || reason == SipCodesMultiChoice) // call forwards
			alternativeAddrStr = m_pSipCntl->GetForwardAddr();
	}
	else
	{
		DBGPASSERT(reason);

		if (reason == SIP_CARD_REJECTED_CHANNELS)
		{
			reason = MCU_INTERNAL_PROBLEM; //change for disconnect cause in GUI
			*pParam >> MipErrorNumber;
			if (GetDialState() == kBeforeInvite)
				SetDialState(kChannelsFailedToOpen);
			else if (GetDialState() == kInviteSent)
				SetDialState(kCancelInvite);
			// else ok arrived (or any other dial state) - don't change dial state
		}
		else if (GetDialState() == kInviteSent && (APIS32)reason < 0)
		{
			// ok arrived with parsing error - act like no recovery. send ack and bye
			SetDialState(kNoRecovery);
		}
	}

	if ( (m_eActiveTransactionType == kSipTransUpgradeAvcOnlyToMixReq) ||(m_eActiveTransactionType == kSipTransUpgradeSvcOnlyToMixReq)  )
	{
          if(m_pSipCntl->GetIsMrcCall())
	  {
	    TRACEINTO<<"!@# dynMixedErr failed while updating channels (svc)";
	  }
	  else
	  {
	    TRACEINTO<<"!@#  dynMixedErr failed while opening either internal channels or internal arts (avc)";
	  }
	  //	  EndTransactionByPartyIfNeeded();
	m_pSipCntl->ResetReqCounter();
	  return;
	}
	TellConfOnDisconnecting(reason,alternativeAddrStr,MipErrorNumber);
}

////////////////////////////////////////////////////////////////////////
void CSipPartyOutCreate::OnPartyBadStatusConnecting(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyOutCreate::OnPartyBadStatusConnecting: Name ",m_partyConfName);
	CSipParty::OnPartyBadStatus(pParam);
}

///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnDnsResolutionCallIdle(CSegment * pParam)
//{
//	PTRACE(eLevelInfoNormal,"CSipPartyOut::OnDnsResolutionCallIdle");
//	DWORD status = 0;
//
//	*pParam >> status;
//
//	if (IsValidTimer(DNSQUARYTOUT))
//		DeleteTimer(DNSQUARYTOUT);
//
//	if (status == STATUS_OK)
//	{
//		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode);
//		m_state = sPARTY_OPENINCHANNELS;
//		m_eDialState = kBeforeInvite;
//
//	}
//	else
//	{
//		DBGPASSERT(1);
//		PTRACE(eLevelError,"CSipPartyOut::OnDnsResolutionCallIdle: Problems with DNS quary");
//		m_eDialState = kBadStatusArrived;
//		TellConfOnDisconnecting(SIP_BAD_STATUS);
//
//	}
//
//
//}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyOutCreate::UpdateNetSetupToStrForSDP(CSipNetSetup* SipNetSetup)
{

	 PTRACE2(eLevelInfoNormal, "CSipPartyOutCreate::UpdateNetSetupToStrForSDP ", SipNetSetup->GetToPartyAddress());

	char buffSip[1024];
	memset(buffSip, '\0',sizeof(buffSip));
	strncpy(buffSip,SipNetSetup->GetToPartyAddress() ,sizeof(buffSip)-1);//strlen(SipNetSetup->GetToPartyAddress())); //WK


	 PTRACE2(eLevelInfoNormal, "CSipPartyOutCreate::UpdateNetSetupToStrForSDP ", buffSip);

    char *p1 = NULL;
    char *p2 = NULL;

    char bufId[256]={0};

    //char buffSip[1024] = "sip:Ori01@ilw14.polycom.eng;gruu;opaque=app:conf:focus:id:HMGL1789";

    p1 = (char*) strstr(buffSip, "conf:");

    p2 = (char*) strstr(buffSip, "id:");
    if (p2) {
          strncpy(bufId,p2,sizeof(bufId) - 1);
          bufId[sizeof(bufId) - 1] = '\0';
    }

    if(p1 && p2)
    {
           strncpy(p1+5, "audio-video:",12);

           strncpy(p1+5+12, bufId,12);
    }



    PTRACE2(eLevelInfoNormal, "CSipPartyOutCreate::UpdateNetSetupToStrForSDP ", buffSip);

    SipNetSetup->SetToPartyAddress(buffSip);
    SipNetSetup->SetRemoteSipAddress(buffSip);

    PTRACE2(eLevelInfoNormal, "CSipPartyOutCreate::UpdateNetSetupToStrForSDP ", SipNetSetup->GetToPartyAddress());



}
