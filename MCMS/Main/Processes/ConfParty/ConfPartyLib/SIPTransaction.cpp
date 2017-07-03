//+========================================================================+
//                    SIPTransaction.cpp 		                     	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransaction.cpp                                     	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include "SipUtils.h"
#include "ConfPartyOpcodes.h"
#include "SipScm.h"
#include "SIPTransaction.h"
#include "SIPControl.h"
#include "PartyApi.h"
#include "SIPParty.h"
#include "IpCommon.h"
#include "CommConf.h"
#include "CommConfDB.h"
#include "H264Util.h"
#include "ConfPartyGlobals.h" //TIP call from Polycom EPs feature
#include "SIPInternals.h"


PBEGIN_MESSAGE_MAP(CSipTransaction)
  ONEVENT(SIP_PARTY_CHANS_CONNECTED, ANYCASE, CSipTransaction::OnPartyChannelsConnectedAnycase)
  ONEVENT(SIP_PARTY_CHANS_UPDATED, ANYCASE, CSipTransaction::OnPartyChannelsUpdatedAnycase)
  ONEVENT(OPENBRIDGESTOUT, ANYCASE,	CSipTransaction::NullActionFunction)


PEND_MESSAGE_MAP(CSipTransaction, CStateMachine);

CSipTransaction::CSipTransaction(CTaskApp *pOwnerTask):CStateMachine(pOwnerTask)
{
	m_pPartyApi					= new CPartyApi;
	m_pChanDifArr 				= new CSipChanDifArr;
	m_isVideoBridgeConnected	= 0;
	m_isAudioBridgeConnected	= 0;
	m_isFeccBridgeConnected  	= 0;
	m_voice						= NO;
	m_bNeedReInviteForReAlloc	= FALSE;
	m_bNeedReInviteForSecondary = FALSE;
	m_bNeedReInviteForSecondaryContent = FALSE;
	m_bChangeInVideoFormatOnly  = FALSE;
	m_bNeedReInviteForIce       = FALSE;
	m_bNeedCloseIceChannels 	= FALSE;
//	m_bNeedUpdateIceChannels	= FALSE;
	m_bNeedCloseSrtpChannels	= FALSE;
	m_bNeedUpdateSrtpChannels	= FALSE;
	m_bIsChangeInICEChannels	= FALSE;
	m_bNeedToSendReInviteWithFullAudioCaps = FALSE;
	m_bNeedReInviteForAddContent 	= FALSE;
	m_bNeedReInviteToFixContentAlg 	= FALSE;
	m_bContentRateWasChanged		= FALSE;
	m_bContentProtocolWasChanged 	= FALSE;
	m_bNeedReinviteForRemoveRtv    	= FALSE;

	// The following pointers are Party's pointers that used also by Transaction.
	// Transaction gets them from Party and initiate them in InitTransaction function.
	// They must be initiate by Party in each new Transaction.
	// They are allocated in Party and it is Party responsible to delete them.
	m_pPartyConfName 			= NULL;
	m_pCurrentMode 				= NULL;
	m_pTargetMode 				= NULL;
	m_pSipCntl 					= NULL;
	m_pEDialState 				= NULL;
	m_pAlternativeAddrStr		= NULL;
	m_pTargetModeMaxAllocation 	= NULL;
	m_IceMakeOfferAnswerCounter = 0;
	m_pParty			= NULL;
	// TIP
	m_bNeedReInviteForSwitchToNoneTipCall = FALSE;
	m_bIsNeedToDropCall			= FALSE;
	m_bIsTipMute				= FALSE;
	m_needToWaitForSlavesEndChangeMode 	= FALSE;
	m_bIsResumeMedia			= FALSE;
	m_Transation_Reason = 0;
	m_bNeedReInviteForBandwidth  = FALSE;
	m_bIsNeedToFallbackFromIceToSip = FALSE;
	m_isNeedToEndTransAfterTipSlavesAck = FALSE;
	// BFCP
	m_bNeedReInviteForBfcp		= FALSE;
	m_bNeedToCloseBfcpAndContentWithoutReinvite = FALSE;
	m_isContentSuggested 		= FALSE;
    m_bNeedToCloseInternalVideoArt= false;
	m_oldstate = IDLE;
    m_incomingVideoChannelHandle=INVALID_CHANNEL_HANDLE;
    m_outgoingVideoChannelHandle=INVALID_CHANNEL_HANDLE;
        m_bIsNeedToPendOtherTransaction = TRUE;
    m_bTipEarlyPacket = FALSE;
    m_bIsGlare = FALSE; //BRIDGE-12961
    m_bIsReInvite = FALSE;
    m_bIsOfferer = FALSE;
}

CSipTransaction::~CSipTransaction()
{
	POBJDELETE(m_pPartyApi);
	POBJDELETE(m_pChanDifArr);
}

BOOL CSipTransaction::DispatchEvent(OPCODE event,CSegment* pParam)
{
    if (IsEventExist(event))
    {
        return CStateMachine::DispatchEvent(event,pParam);
    }
    return FALSE;
}

BOOL CSipTransaction::HandleSipPartyEvent(CSegment *pMsg, DWORD msgLen, OPCODE opCode)
{
	m_pPartyApi->SendSipTransMsg(opCode, pMsg);
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// All messages from Sip-Transaction to Sip-Party must be sent by this function
void CSipTransaction::SendMessageToParty(OPCODE event, CSegment *pSeg)
{
	m_pPartyApi->SendSipTransMsg(event, pSeg);
	POBJDELETE(pSeg);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::InitTransaction(CSipParty* pParty, CSipCntl* pPartySipCntl, CIpComMode* pPartyCurrentMode, CIpComMode* pPartyTargetMode, ESipDialState* pPartyEDialState, char* pPartyConfName, WORD voice, char* alternativeAddrStr,CIpComMode* pTargetModeMaxAllocation, BYTE bTransactionSetContentOn, BYTE isContentSuggested, BYTE isFallbackFromTipToSipFlow, BYTE bIsGlare)
{
	PTRACE2(eLevelInfoNormal,"***CSipTransaction::InitTransaction : Name - ", pPartyConfName);
	m_pSipCntl 				= pPartySipCntl;
	m_pCurrentMode 			= pPartyCurrentMode;
	m_pTargetMode 			= pPartyTargetMode;
	m_pEDialState 			= pPartyEDialState;
	m_pPartyConfName 		= pPartyConfName;
	m_voice					= voice;
	m_pAlternativeAddrStr	= alternativeAddrStr;
	m_pPartyApi->CreateOnlyApi(pParty->GetRcvMbx(), this);
	m_pPartyApi->SetLocalMbx(pParty->GetLocalQueue());
	m_pTargetModeMaxAllocation = pTargetModeMaxAllocation;
	m_pParty				= pParty;
	m_isFallbackFromTipToSipFlow = isFallbackFromTipToSipFlow;

	m_bTransactionSetContentOn = bTransactionSetContentOn;

	m_isContentSuggested 	= isContentSuggested;
	m_bIsGlare = bIsGlare;

	CSipChannel *pChannelIn;
	CSipChannel *pChannelOut;

	pChannelIn = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapReceive, kRolePeople);
	pChannelOut = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapTransmit, kRolePeople);

	if(pChannelIn)
	{
            m_incomingVideoChannelHandle=pChannelIn->GetChannelHandle();
	}
	if(pChannelOut)
	{
            m_outgoingVideoChannelHandle=pChannelOut->GetChannelHandle();
	}

	PTRACE2INT(eLevelInfoNormal,"CSipTransaction::InitTransaction : TIP mode: ", m_pTargetMode->GetIsTipMode());
	PTRACE2INT(eLevelInfoNormal,"CSipTransaction::InitTransaction : m_bTransactionSetContentOn: ", m_bTransactionSetContentOn);
    m_pTargetMode->Dump("CSipTransaction::InitTransaction - m_pTargetMode", eLevelInfoNormal);
    m_pCurrentMode->Dump("CSipTransaction::InitTransaction - m_pCurrentMode", eLevelInfoNormal);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::EndTransaction(DWORD retStatus)
{
	//PTRACE2(eLevelInfoNormal,"CSipTransaction::EndTransaction : Name - ", m_pPartyConfName);
	CleanTransaction();

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	if (pCommConf)
	{
		CConfParty* pConfParty 	= pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());

		if (pConfParty)
		{
			eAvMcuLinkType avMcuLinkType = pConfParty->GetAvMcuLinkType();

			TRACEINTO << "Name:" << m_pPartyConfName << ", avMcuLinkType:" << (DWORD)avMcuLinkType
					  << ", isMs2013Active:" << (DWORD)m_pSipCntl->isMs2013Active();

			if ( m_pSipCntl->isMs2013Active() != eMsft2013AvMCU || avMcuLinkType == eAvMcuLinkMain )
			{
				StartFECIfNeeded();
				StartREDIfNeeded();
			}
		}
	}

	SendEndTransactionToParty(retStatus);
	m_state = sTRANS_END_WAS_SENT;
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::StartFECIfNeeded()
{
	if (m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive))
	{
		CapEnum videoAlg = (CapEnum)(m_pCurrentMode->GetMediaType(cmCapVideo, cmCapTransmit));

		//LYNC2013_FEC_RED:
		BOOL isMsSvcFecEnabled = TRUE;
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		pSysConfig->GetBOOLDataByKey("ENABLE_SIP_LYNC2013_FEC", isMsSvcFecEnabled);
		if(!m_pSipCntl->isMs2013Active() )
			isMsSvcFecEnabled = FALSE;

		const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
		bool lync2013ClientAndMCUsupportFec = FALSE;

		if ( (CapEnum)videoAlg==eMsSvcCapCode && isMsSvcFecEnabled && pCurRemoteCaps->GetIsFec() )
			lync2013ClientAndMCUsupportFec = TRUE;

		if( ((CapEnum)videoAlg==eRtvCapCode && IsMsFECEnabled()) ||  (lync2013ClientAndMCUsupportFec == TRUE) )
		{
			PTRACE2(eLevelInfoNormal,"LYNC2013_FEC_RED: CSipTransaction::StartFECIfNeeded - Both client and MCU support FEC -> SendStartFecToCM, Name:", m_pPartyConfName);
			m_pSipCntl->SendStartFecToCM(lync2013ClientAndMCUsupportFec);
		}
		else
			PTRACE2(eLevelInfoNormal,"LYNC2013_FEC_RED: CSipTransaction::StartFECIfNeeded - FEC is not supported by client/MCU/both, Name:", m_pPartyConfName);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CSipTransaction::StartFECIfNeeded - LYNC2013_FEC_RED: FEC is closed!! Name:", m_pPartyConfName);
		m_pSipCntl->SetIsFecOn(FALSE);
		m_pSipCntl->SetIsFecStarted(FALSE);
	}
}
///////////////////////////////////////////////////////////////////////////////////////
//LYNC2013_FEC_RED:
void CSipTransaction::StartREDIfNeeded()
{
	if (m_pCurrentMode->IsMediaOn(cmCapAudio,cmCapReceive) && m_pSipCntl->isMs2013Active())
	{
		RemoteIdent remoteIdent = m_pSipCntl->GetRemoteIdent();

		BOOL isSipRedEnabled = TRUE;
		CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		pSysConfig->GetBOOLDataByKey("ENABLE_SIP_LYNC2013_RED", isSipRedEnabled);
		const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();

		if ( pCurRemoteCaps->GetIsRed() && isSipRedEnabled )   //(remoteIdent==Microsoft_AV_MCU2013 || remoteIdent==MicrosoftEP_Lync_2013)
		{
			PTRACE2(eLevelInfoNormal,"LYNC2013_FEC_RED: CSipTransaction::StartREDIfNeeded - Both client and MCU support RED -> SendStartRedToCM, Name:", m_pPartyConfName);
			m_pSipCntl->SendStartRedToCM();
		}
		else
			PTRACE2(eLevelInfoNormal,"LYNC2013_FEC_RED: CSipTransaction::StartREDIfNeeded - RED is not supported by client/MCU/both, Name:", m_pPartyConfName);
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CSipTransaction::StartFECIfNeeded - LYNC2013_FEC_RED: RED is closed!! Name:", m_pPartyConfName);
		m_pSipCntl->SetIsRedOn(FALSE);
		m_pSipCntl->SetIsRedStarted(FALSE);
	}
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::EndTransactionByParty()
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::EndTransactionByParty : Name - ", m_pPartyConfName);
	CleanTransaction();
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::CleanTransaction()
{
	if (IsValidTimer(OPENBRIDGESTOUT))
		DeleteTimer(OPENBRIDGESTOUT);
	if (IsValidTimer(UPDATEBRIDGESTOUT))
		DeleteTimer(UPDATEBRIDGESTOUT);
	if (IsValidTimer(ICEPORTSRETRYTOUT))
		DeleteTimer(ICEPORTSRETRYTOUT);
	if (IsValidTimer(UPGRADETOMIXTOUT))
		DeleteTimer(UPGRADETOMIXTOUT);
}

//////////////////////////////////////////////////////
void CSipTransaction::RollbackTransaction()
{
	PTRACE(eLevelInfoNormal,"CSipTransaction::RollbackTransaction - Do nothing. Should be implement in inheritor class");
}

///////////////////////////////////////////////////////////////////////////////////////
ESipDialState CSipTransaction::GetDialState() const
{
	return *m_pEDialState;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SetDialState(ESipDialState eSipDialState)
{
	*m_pEDialState = eSipDialState;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::RequiredChannelsActionDone(DWORD opcode)
{
	PTRACE2INT(eLevelInfoNormal,"CSipTransaction::RequiredChannelsActionDone, opcode:", opcode);
	PTRACE2INT(eLevelInfoNormal,"CSipTransaction::RequiredChannelsActionDone, m_state:", m_state);

	CSegment *pSeg = new CSegment;

	CSipComMode* pCurrentMode = new CSipComMode;
	pCurrentMode->Create(*m_pSipCntl->GetCallObj());
	pCurrentMode->CopyStaticAttributes(*m_pTargetMode);
	if (ShouldKeepTargetTxStreams())
	{
	    pCurrentMode->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapAudio, cmCapTransmit, kRolePeople), cmCapAudio, cmCapTransmit, kRolePeople);
        pCurrentMode->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople), cmCapVideo, cmCapTransmit, kRolePeople);
	}
    pCurrentMode->SetStreamsListForMediaMode(m_pCurrentMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople), cmCapVideo, cmCapReceive, kRolePeople);

	pCurrentMode->Serialize(NATIVE, *pSeg);

	pCurrentMode->Dump("***CSipTransaction::RequiredChannelsActionDone pCurrentMode", eLevelInfoNormal);

    DispatchEvent(opcode,pSeg);
	POBJDELETE(pSeg);
	POBJDELETE(pCurrentMode);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::UpdateDbChannelsStatus(CSegment* pParam, BYTE bIsConnected /* TRUE or FALSE = update connected or disconnected channels*/)
{
	EConfType eConfType = m_pTargetMode->GetConfType();
	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pCurrentMode->SetConfType(eConfType);
	DWORD contentRateScm = m_pTargetMode->GetContentBitRate(cmCapReceive);
	m_pCurrentMode->SetContentBitRate(contentRateScm, cmCapReceiveAndTransmit);
	if (m_pSipCntl->GetIsMrcCall() || (m_pTargetMode->GetConfMediaType()==eMixAvcSvcVsw))
	{
		std::list <StreamDesc> streamsDescList = m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
		m_pCurrentMode->SetStreamsListForMediaMode(streamsDescList, cmCapVideo, cmCapReceive, kRolePeople);
		streamsDescList = m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);
		m_pCurrentMode->SetStreamsListForMediaMode(streamsDescList, cmCapVideo, cmCapTransmit, kRolePeople);
	}
	SendUpdateDbChannelsStatusToParty(bIsConnected);
}

//////////////////////////////////////////////////////
void CSipTransaction::CheckChangingInCurrentMode(BYTE bCheckUpgradeReceive)
{
	PASSERT_AND_RETURN(!m_pSipCntl);
	PTRACE2(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode : Name - ",m_pPartyConfName);
	DWORD details = 0;

	DWORD videoValuesToCompare 		= kCapCode|kFormat|kFrameRate|kH264Profile|kH264Level|kH264Additional|kBitRate|kPacketizationMode;
	DWORD audioValuesToCompare 		= kCapCode|kFrameRate;
	DWORD contentValuesToCompare		= kCapCode|kH264Profile;  //FSN-613: Dynamic Content for SVC/Mix Conf
	DWORD bfcpValuesToCompare		= kCapCode | kTransportType;

	POBJDELETE(m_pChanDifArr);
	m_pChanDifArr						= new CSipChanDifArr;
	CSipChanDif* pChanDif				= NULL;
	const CSipCaps* pCurRemoteCaps 		= m_pSipCntl->GetLastRemoteCaps();

	BYTE  bIsContainPayload 			= YES;
	int arrInd 							= 0;
	BYTE bIsSdesEnable					= NO;

	if(m_pTargetMode->GetIsEncrypted() == Encryp_On)
		bIsSdesEnable = YES;


	// start change mode
	// in the current implementation the call for the internal media recovery is only when the in channel are open and there is no out channel opened
	// there for the check of the out direction will always fail (Carmel - GL1).
	// receive direction target checking current. if bCheckUpgradeReceive is true, check current (video) contains target too.
	// transmit direction current checking target

	if (m_pCurrentMode->IsMediaOn(cmCapAudio,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapAudio,cmCapReceive))
	{
		if (m_pTargetMode->IsMediaContaining(*m_pCurrentMode, audioValuesToCompare, &details,cmCapAudio,cmCapReceive) == NO)
		{
			pChanDif = m_pChanDifArr->GetChanDif(cmCapAudio,cmCapReceive);
			if(pChanDif) {
				pChanDif->SetChangeAlg(YES);
			}
		}

		if( ((bIsSdesEnable) && (!bCheckUpgradeReceive)) || m_bNeedUpdateSrtpChannels == true )
		{
			pChanDif = m_pChanDifArr->GetChanDif(cmCapAudio,cmCapReceive);
			if(pChanDif) {
				pChanDif->SetChangeSdes(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeSdes(YES) Audio Rcv");
			}
		}
	}
	if (m_pCurrentMode->IsMediaOn(cmCapAudio,cmCapTransmit) && m_pTargetMode->IsMediaOn(cmCapAudio,cmCapTransmit))
	{
		pChanDif = m_pChanDifArr->GetChanDif(cmCapAudio,cmCapTransmit);
		if (m_pCurrentMode->IsMediaContaining(*m_pTargetMode, audioValuesToCompare, &details,cmCapAudio,cmCapTransmit) == NO && pChanDif)
		{
			pChanDif->SetChangeAlg(YES);
		}
		CCapSetInfo capInfo = (CapEnum)m_pTargetMode->GetMediaType(cmCapAudio,cmCapTransmit);
		APIU8 uiPayloadType = m_pSipCntl->GetChannelPayloadType(cmCapAudio,cmCapTransmit);
		bIsContainPayload 	= pCurRemoteCaps->IsContainingPayloadType(capInfo,uiPayloadType,&arrInd);
		if(bIsContainPayload == NO && pChanDif)
			pChanDif->SetChangePayload(YES);

		if( ((bIsSdesEnable) && (!bCheckUpgradeReceive)) || m_bNeedUpdateSrtpChannels == true )
		{
			if(pChanDif) {
				pChanDif->SetChangeSdes(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeSdes(YES) Audio Tx");
			}
		}
	}

	if (m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive))
	{

		if((m_pTargetMode->IsMediaContaining(*m_pCurrentMode, videoValuesToCompare, &details,cmCapVideo,cmCapReceive) == NO)
			|| (bCheckUpgradeReceive && (m_pCurrentMode->IsMediaContaining(*m_pTargetMode, videoValuesToCompare, &details,cmCapVideo,cmCapReceive) == NO)))
		{
		    pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapReceive);
		    if (pChanDif)
		        pChanDif->SetChangeAlg(YES);
		    else
		        PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode (no SetChangeAlg) - pChanDif is NULL");
		}

		if (m_pCurrentMode->GetIsLpr() && pCurRemoteCaps->GetIsLpr()==FALSE)
		{
		    pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapReceive);
		    if (pChanDif)
		        pChanDif->SetChangeLpr(YES);
		    else
		        PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode (no SetChangeLpr) - pChanDif is NULL");
		}
		if( ((bIsSdesEnable) && (!bCheckUpgradeReceive)) || m_bNeedUpdateSrtpChannels == true )
		{
			pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapReceive);
			if(pChanDif)
			{
				pChanDif->SetChangeSdes(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeSdes(YES) Video Rcv");
			}
		}

	}
	if (m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit) && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapTransmit))
	{
		pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapTransmit);
		if (pChanDif && (m_pCurrentMode->IsMediaContaining(*m_pTargetMode, videoValuesToCompare, &details,cmCapVideo,cmCapTransmit) == NO) )
		{
			pChanDif->SetChangeAlg(YES);
		}

		CCapSetInfo capInfo = (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo,cmCapTransmit);
		APIU8 uiPayloadType = m_pSipCntl->GetChannelPayloadType(cmCapVideo,cmCapTransmit);
		bIsContainPayload 	= pCurRemoteCaps->IsContainingPayloadType(capInfo,uiPayloadType,&arrInd);
		if(pChanDif && (bIsContainPayload == NO) )
		    pChanDif->SetChangePayload(YES);

		if( ((bIsSdesEnable) && (!bCheckUpgradeReceive)) || m_bNeedUpdateSrtpChannels == true )
		{
		    //	bVideoOutDifferent = YES;
		    if(pChanDif)
		    {
		        pChanDif->SetChangeSdes(YES);
		        PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeSdes(YES) Video Tx");
		    }
		}
	}

	// if the video in H263 channel payload type is dynamic and the remote only declared on static H263 payload, we need to change the payload to static one.
	if (m_pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive) && m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive) )
	{
		APIU8 currentPayloadType = m_pSipCntl->GetChannelPayloadType(cmCapVideo,cmCapReceive);
		if (IsDynamicPayloadType(currentPayloadType))
		{
			CCapSetInfo capInfo	= (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive);
			WORD   profile  = H264_Profile_None;
			APIS32 H264mode = H264_standard;
			APIU8  packetizationMode = 0;
			if ((CapEnum)capInfo == eH264CapCode)
			{
				profile  = m_pTargetMode->GetH264Profile(cmCapReceive);
				packetizationMode = m_pTargetMode->GetH264PacketizationMode(cmCapReceive);
				if (m_pTargetMode->IsTIPContentEnableInH264Scm() == TRUE)
					H264mode = H264_tipContent;
			}
			payload_en remotePayload = pCurRemoteCaps->GetPayloadTypeByDynamicPreference(capInfo, profile, kRolePeople, H264mode, packetizationMode);

			if (pChanDif && (!IsDynamicPayloadType(remotePayload)) )
			{// different payload type definitions, remote support only H263 static payload, change to what remote supported
				PTRACE(eLevelInfoNormal,"CSipParty::CheckChangingInCurrentMode, video payload type changed");
				pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapReceive);
				if( pChanDif )
					pChanDif->SetChangePayload(YES);
			}
		}
	}

	//FECC for SRTP only
	if (m_pCurrentMode->IsMediaOn(cmCapData,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapData,cmCapReceive))
	{
		if( ((bIsSdesEnable) && (!bCheckUpgradeReceive)) || m_bNeedUpdateSrtpChannels == true )
		{
			pChanDif = m_pChanDifArr->GetChanDif(cmCapData,cmCapReceive);
			if(pChanDif)
			{
				pChanDif->SetChangeSdes(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeSdes(YES) Data Rcv");
			}
		}
	}
	if (m_pCurrentMode->IsMediaOn(cmCapData,cmCapTransmit) && m_pTargetMode->IsMediaOn(cmCapData,cmCapTransmit))
	{
		if( ((bIsSdesEnable) && (!bCheckUpgradeReceive)) || m_bNeedUpdateSrtpChannels == true )
		{
			pChanDif = m_pChanDifArr->GetChanDif(cmCapData,cmCapTransmit);
			if(pChanDif)
			{
				pChanDif->SetChangeSdes(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeSdes(YES) Data Tx");
			}
		}

	}

	// Content:
	if (m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive, kRolePresentation) && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive, kRolePresentation))
	{
		if(m_pTargetMode->IsMediaContaining(*m_pCurrentMode, contentValuesToCompare, &details,cmCapVideo,cmCapReceive, kRolePresentation) == NO)
		{
			pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapReceive, kRolePresentation);
			if(pChanDif)
				pChanDif->SetChangeAlg(YES);
		}

		if (m_pCurrentMode->GetIsLpr() && pCurRemoteCaps->GetIsLpr()==FALSE)
		{
			pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapReceive, kRolePresentation);
			if(pChanDif)
				pChanDif->SetChangeLpr(YES);
		}
		if((bIsSdesEnable) && (!bCheckUpgradeReceive))
		{
			pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapReceive, kRolePresentation);
			if(pChanDif) {
				pChanDif->SetChangeSdes(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeSdes(YES) Content Rcv");
			}
		}

	}
	if (m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapTransmit, kRolePresentation) && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapTransmit, kRolePresentation))
	{
		pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo,cmCapTransmit, kRolePresentation);
		if (m_pCurrentMode->IsMediaContaining(*m_pTargetMode, contentValuesToCompare, &details,cmCapVideo,cmCapTransmit, kRolePresentation) == NO && pChanDif)
		{
			if(pChanDif)
				pChanDif->SetChangeAlg(YES);
			else
				PASSERT(1);			
		}
		CCapSetInfo capInfo = (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo,cmCapTransmit, kRolePresentation);
		APIU8 uiPayloadType = m_pSipCntl->GetChannelPayloadType(cmCapVideo,cmCapTransmit, kRolePresentation);
		if (m_pTargetMode->GetIsTipMode())
			bIsContainPayload = TRUE;
		else
			bIsContainPayload 	= pCurRemoteCaps->IsContainingPayloadType(capInfo,uiPayloadType,&arrInd, kRolePresentation);
		if(bIsContainPayload == NO)
		{
			if(pChanDif)
				pChanDif->SetChangePayload(YES);
			else
				PASSERT(1);	
		}

		if((bIsSdesEnable) && (!bCheckUpgradeReceive))
		{
			if(pChanDif) {
				pChanDif->SetChangeSdes(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeSdes(YES) Content Tx");
			}
		}
	}

	if (m_pCurrentMode->IsMediaOn(cmCapBfcp,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapBfcp,cmCapReceive))
	{
		pChanDif = m_pChanDifArr->GetChanDif(cmCapBfcp,cmCapReceive);

		if (m_pCurrentMode->IsMediaContaining(*m_pTargetMode, bfcpValuesToCompare, &details,cmCapBfcp,cmCapReceive) == NO && pChanDif)
		{
			if(pChanDif)
			{
				pChanDif->SetChangeBfcpTransportProtocol(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeBfcpTransportProtocol(YES) for receive bfcp");
			}
			else
				PASSERT(1);
		}
	}

	if (m_pCurrentMode->IsMediaOn(cmCapBfcp,cmCapTransmit) && m_pTargetMode->IsMediaOn(cmCapBfcp,cmCapTransmit))
	{
		pChanDif = m_pChanDifArr->GetChanDif(cmCapBfcp,cmCapTransmit);

		if (m_pCurrentMode->IsMediaContaining(*m_pTargetMode, bfcpValuesToCompare, &details,cmCapBfcp,cmCapTransmit) == NO && pChanDif)
		{
			if(pChanDif)
			{
				pChanDif->SetChangeBfcpTransportProtocol(YES);
				PTRACE(eLevelInfoNormal,"CSipTransaction::CheckChangingInCurrentMode. SetChangeBfcpTransportProtocol(YES) for receive bfcp");
			}
			else
				PASSERT(1);
		}
	}

	if(m_bIsOfferer && !m_bIsReInvite && m_pSipCntl && m_pSipCntl->isMs2013Active())
	{
		if (m_pCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapVideo, cmCapReceive))
		{
			CSipChanDif* pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapReceive, kRolePeople);
			TRACEINTO << " SetChangeMSSsrc(YES)";
			if(pChanDif)
				pChanDif->SetChangeMSSsrc(YES);
		}
	}

//	if (bVideoInDifferent)
//	{
//		//If local caps aren't contained in the new mode - that means that the new mode is lower than local caps.
//		//This means that the new mode is a subset of local caps => we need to send re-invite.
//		//But if the new mode is equal to local caps => re-invite is not needed.
//		const CSipCaps*	pLocalCaps  = m_pSipCntl->GetLocalCaps();
//		const CBaseCap* pNewReceive = m_pTargetMode->GetMediaAsCapClass(cmCapVideo,cmCapReceive);
//		BYTE bNewModeLower = pLocalCaps->IsContainedInCapSet(*pNewReceive, videoValuesToCompare, &details, &arrInd) ? FALSE : TRUE;
//		if (bNewModeLower)
//			DBGPASSERT(1);//instead of this line: m_eDialState = kExternalRecovery; // in order to send re-invite to the ep
//		POBJDELETE(pNewReceive);
//	}

	CMedString str;
	m_pChanDifArr->DumpToString(str);

	PTRACE2(eLevelInfoNormal, "CSipTransaction::CheckChangingInCurrentMode\n", str.GetString());
	UpdateChannelsIfNeeded();
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::UpdateChannelsIfNeeded()
{
	EIpChannelType chanArr;
	int numOfUpdateChannels = 0;
	PTRACE2(eLevelInfoNormal, "CSipTransaction::UpdateChannelsIfNeeded,- ", m_pPartyConfName); //TBD - we need to update with new scm if the video channels are not the same
	cmCapDataType mediaType;
	ERoleLabel eRole;
	BYTE bNeedUpdateIceChannels = m_pSipCntl->GetIsNeedUpdateIceToNonIce();
	
	for (int i = 0 ; i < CHANNEL_TYPES_COUNT; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

	//removed	( if(mediaType == cmCapBfcp) continue;  ) on purpose in order to handle port changes on BFCP channel !!!

		for (int j = 0; j < 2; j++)// 2 = number of direction (receive, transmit).
		{
			CSipChanDif* pChanDif		= NULL;
			pChanDif = m_pChanDifArr->GetChanDif(mediaType,globalDirectionArr[j],eRole);
			BYTE UpdateChannelAccording2Scm = NO;

			if(m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole) &&
								 m_pTargetMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole))
			{
				DWORD details				= 0x00000000;
				CMediaModeH323& currentmedia = m_pCurrentMode->GetMediaMode(mediaType,globalDirectionArr[j],eRole);
				CMediaModeH323& targetmedia = m_pTargetMode->GetMediaMode(mediaType,globalDirectionArr[j],eRole);
				if(!currentmedia.IsContaining(targetmedia,kCapCode|kBitRate|kFormat|kAnnexes|kH264Profile|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, &details))
					UpdateChannelAccording2Scm = TRUE;
				if(!targetmedia.IsContaining(currentmedia,kCapCode|kBitRate|kFormat|kAnnexes|kH264Profile|kH264Level|kH264Additional_FS|kH264Additional_MBPS| kMaxFR | kH264Mode, &details))
					UpdateChannelAccording2Scm = TRUE;
			}

			TRACEINTO << "media type:" << mediaType << ", direction:" << globalDirectionArr[j]
			    << ", current mode - is media on:" << (int)m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole)
			    << ", target mode - is media on: " << (int)m_pTargetMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole)
				<< ", is sdes changed:" << (pChanDif?((int)pChanDif->IsChangeSdes()):-1)// << ", is dtls changed:" << pChanDif->IsChangeDtls();
				<< ", is ms ssrc changed:" << (pChanDif?((int)pChanDif->IsChangeMSSsrc()):-1);

			if(pChanDif &&  m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole) &&
					 m_pTargetMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole))
			{
				TRACEINTO << "media type:" << mediaType << ", direction:" << globalDirectionArr[j] <<
					   ", UpdateChannelAccording2Scm:" << (int)UpdateChannelAccording2Scm << ", is sdes changed:" << (int)pChanDif->IsChangeSdes() << ", is ms ssrc changed:" << (int)pChanDif->IsChangeMSSsrc();// << ", is dtls changed:" << pChanDif->IsChangeDtls();

				if(UpdateChannelAccording2Scm || pChanDif->IsChangeAlg() || pChanDif->IsChangePayload() || pChanDif->IsChangeLpr() || pChanDif->IsChangeSdes() || pChanDif->IsChangeMSSsrc())// || (pChanDif->IsChangeLpr() && globalDirectionArr[j] == cmCapReceive && globalMediaArr[i] == cmCapVideo ))
				{
					BYTE bChangeParams = 0;

					if(pChanDif->IsChangeAlg())
						bChangeParams = kChannelParams;
					else if(pChanDif->IsChangePayload())
						bChangeParams |= kChangePayload;

					if(pChanDif->IsChangeLpr() && mediaType == cmCapVideo/* && eRole==kRolePeople*/)
					{
						bChangeParams |= kChangeLpr;
					}

					if (pChanDif->IsChangeSdes())
						bChangeParams |= kChangeSdes;

					if (UpdateChannelAccording2Scm)
						bChangeParams |= kChannelParams;

					if(pChanDif->IsChangeMSSsrc())
						bChangeParams |= kChangeMSSsrc;

					chanArr = ::CalcChannelType(mediaType,(globalDirectionArr[j] == cmCapTransmit), eRole);
					if(m_pSipCntl->SipUpdateChannelReq((CSipComMode*)m_pTargetMode, chanArr, bChangeParams))
						numOfUpdateChannels++;
				}
			}

			if(pChanDif)
			{
				if((globalDirectionArr[j] == cmCapTransmit) && (pChanDif->IsChangeIp() || pChanDif->IsChangePort() || pChanDif->IsChangeRtcpPort())
						&&  m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole))
				{
					chanArr = ::CalcChannelType(mediaType,YES, eRole);
					if(m_pSipCntl->SipUpdateChannelReq(NULL, chanArr, kIpAddress))
					    numOfUpdateChannels++;
				}

				// Adding update of change of IP/UDP in case of TIP.
				// When TIP EP hold, CUCM send it's IP:port and send media, so we need to receive media from CUCM.
				if (m_pParty->GetIsTipCall())
				{
					BYTE bChangeParams = 0;

					TRACEINTO << "tip call - media type:" << mediaType << ", direction:" << globalDirectionArr[j]
						<< ", is sdes changed:" << (int)pChanDif->IsChangeSdes()
						<< ", is change IP:" << (int)pChanDif->IsChangeIp()
						<< ", is change port: " << (int)pChanDif->IsChangePort()
						<< ", is change rtcp port:" << (int)pChanDif->IsChangeRtcpPort();// << ", is dtls changed:" << pChanDif->IsChangeDtls();

					if((globalDirectionArr[j] == cmCapReceive) &&
					   (pChanDif->IsChangeIp() || pChanDif->IsChangePort() || pChanDif->IsChangeRtcpPort()))// || pChanDif->IsChangeSdes()))
					{
						if (pChanDif->IsChangeIp() || pChanDif->IsChangePort() || pChanDif->IsChangeRtcpPort())
							bChangeParams |= kIpAddress;

						chanArr = ::CalcChannelType(mediaType, NO, eRole);

						if(m_pSipCntl->SipUpdateChannelReq(NULL, chanArr, bChangeParams))
						    numOfUpdateChannels++;
					}
				}
			}
		}
	}

	if(numOfUpdateChannels == 0)
	{//send message to party on all requested channels are closed.
		PTRACE2(eLevelInfoNormal, "CSipTransaction::UpdateChannelsIfNeeded, there are no channels to update. Send all channels are updated - ", m_pPartyConfName);
		RequiredChannelsActionDone(SIP_PARTY_CHANS_UPDATED);
	}
	else
	    PTRACE2INT(eLevelInfoNormal, "CSipTransaction::UpdateChannelsIfNeeded updated channels count=",numOfUpdateChannels);
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::OpenChannelsIfNeededForReInviteOpenChannels(BYTE isAnsweringToNewCap, cmCapDirection eDirection, WORD isToOpenAudioChannel, WORD isToOpenVideoChannel)
{
	TRACEINTO << "multi_line - isToOpenAudioChannel: " << (isToOpenAudioChannel ? "yes" : "no")
					  << ", isToOpenVideoChannel: " << (isToOpenVideoChannel ? "yes" : "no");

	// multi_line - temp temp temp - we need to look at this again

#if 0
	CSipComMode* pNewMediaMode = new CSipComMode;
	EIpChannelType chanArr;
	int numOfOpenChannels 		= 0;
	CSdesCap *pPrefferedSdesCap 	= NULL;
	CDtlsCap *pPrefferedDtlsCap 	= NULL;
	int bIsFirstEncAppearance 		= 1;
	int bIsFirstDtlsEncAppearance 	= 1;

	DWORD isEncrypted 		= ((CSipComMode*)m_pTargetMode)->GetIsEncrypted();
	DWORD isDtlsEncrypted 	= ((CSipComMode*)m_pTargetMode)->GetIsDtlsEncrypted();
	BYTE  isDtlsAvailable	= ((CSipComMode*)m_pTargetMode)->GetIsDtlsAvailable();
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		// if audio does not need to get opened
		if ( (FALSE == isToOpenAudioChannel) && (cmCapAudio == mediaType) )
			continue;

		for (int j = 0; j < 2; j++)// 2 = number of direction (receive, transmit).
		{
			CSipChanDif* pChanDif		= NULL;

			pChanDif = m_pChanDifArr->GetChanDif(mediaType,globalDirectionArr[j],eRole);

			if(m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole) == NO)
			{
				BYTE AddChannelAccording2Scm = m_pTargetMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole);

				if((pChanDif && pChanDif->IsAddChannel()) || AddChannelAccording2Scm)
				{
					CBaseCap* pMedia = m_pTargetMode->GetMediaAsCapClass(mediaType, globalDirectionArr[j],eRole);

					if (pMedia && pNewMediaMode)
					{
						pNewMediaMode->SetMediaMode(pMedia, mediaType, globalDirectionArr[j],eRole);

						//Copy also SRTP if enable
						if(isEncrypted == Encryp_On)
						{
							pPrefferedSdesCap =  m_pTargetMode->GetSipSdes(mediaType,globalDirectionArr[j],eRole);
							if(pPrefferedSdesCap != NULL)
							{
								pNewMediaMode->SetSipSdes(mediaType,globalDirectionArr[j],eRole,pPrefferedSdesCap);
								if(bIsFirstEncAppearance)
								{
									pNewMediaMode->SetEncryption(Encryp_On,((CSipComMode*)m_pTargetMode)->GetIsDisconnectOnEncryptionFailure());
									bIsFirstEncAppearance = 0;
								}
							}
						}

						//DTLS
						if(isDtlsEncrypted == Encryp_On)
						{
							pPrefferedDtlsCap =  m_pTargetMode->GetSipDtls(mediaType,globalDirectionArr[j],eRole);
							if(pPrefferedDtlsCap != NULL) {
								pNewMediaMode->SetSipDtls(mediaType,globalDirectionArr[j],eRole,pPrefferedDtlsCap);
								if(bIsFirstDtlsEncAppearance)
								{
									pNewMediaMode->SetDtlsEncryption(isDtlsEncrypted);
									pNewMediaMode->SetDtlsAvailable(isDtlsAvailable);
									bIsFirstDtlsEncAppearance = 0;
								}
							}
						}

						numOfOpenChannels++;
					}
					POBJDELETE(pMedia);
				}
			}
		}
	}


	if(numOfOpenChannels == 0)
	{//send message to party on all requested channels are closed.
		PTRACE2(eLevelInfoNormal, "CSipTransaction::OpenChannelsIfNeededForReInviteOpenChannels, there are no channels to open. Send all needed channels are opened - ", m_pPartyConfName);
		RequiredChannelsActionDone(SIP_PARTY_CHANS_CONNECTED);
	}
	else	//we need to send in the SCM only the new channels!!
	{
		if( m_pSipCntl )
		{
			//			*pNewMediaMode = *((CSipComMode*)m_pTargetMode);
			RestoreStreamGroups(m_pTargetModeMaxAllocation, pNewMediaMode, isToOpenAudioChannel);
			//			m_pTargetModeMaxAllocation->Dump("multi_line - unhold m_pTargetModeMaxAllocation open channels ", eLevelInfoNormal);
			//			pNewMediaMode->Dump("pNewMediaMode open channels", eLevelInfoNormal);
			if (m_pParty->GetIsTipCall()))
				m_pSipCntl->SipOpenChannelsReq(pNewMediaMode,eDirection,isAnsweringToNewCap, eTipMasterCenter);
			else
			m_pSipCntl->SipOpenChannelsReq(pNewMediaMode,eDirection,isAnsweringToNewCap);
		}
	}

	POBJDELETE(pNewMediaMode);
#else

	CSipComMode* pNewMediaMode  = new CSipComMode;
	*pNewMediaMode= *((CSipComMode*)m_pTargetMode);
	RestoreStreamGroups(m_pTargetModeMaxAllocation, pNewMediaMode, isToOpenAudioChannel, isToOpenVideoChannel);
	m_pTargetModeMaxAllocation->Dump("multi_line - m_pTargetModeMaxAllocation open channels ", eLevelInfoNormal);
	pNewMediaMode->Dump("pNewMediaMode open channels", eLevelInfoNormal);

	if (m_pParty->GetIsTipCall())
		m_pSipCntl->SipOpenChannelsReq(pNewMediaMode,eDirection,isAnsweringToNewCap, eTipMasterCenter);
	else
		m_pSipCntl->SipOpenChannelsReq(pNewMediaMode,eDirection,isAnsweringToNewCap);

	POBJDELETE(pNewMediaMode);

#endif
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::OpenChannelsIfNeededForReInvite(BYTE isAnsweringToNewCap, cmCapDirection eDirection, BYTE bIsToOpenAudio/*=FALSE*/, BYTE bIsToOpenVideo/*=FALSE*/)
{
	TRACEINTO << "partyConfName: " << m_pPartyConfName << "; isAnsweringToNewCap: " << (isAnsweringToNewCap? "yes" : "no") << ", direction: " << eDirection
			  << ", isToOpenAudio: " << (bIsToOpenAudio ? "yes" : "no") << ", isToOpenVideo: " << (bIsToOpenVideo ? "yes" : "no");

	CSipComMode* pNewMediaMode = new CSipComMode;
	EIpChannelType chanArr;
	int numOfOpenChannels = 0;
	CSdesCap *pPrefferedSdesCap = NULL;
	int bIsFirstEncAppearance = 1;
	int startPoint = 0;
	int endPoint   = 2;

	if (eDirection == cmCapReceive) //and not transmit
	{
		// startPoint is 0
		endPoint = 1;
	}
	else if (eDirection == cmCapTransmit) //and not receive
	{
	// endPoint is 2
		startPoint = 1;
	}
	DWORD isEncrypted = ((CSipComMode*)m_pTargetMode)->GetIsEncrypted();
	cmCapDataType mediaType;
	ERoleLabel eRole;

	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		for (int j = startPoint; j < endPoint; j++)// 2 = number of direction (receive, transmit).
		{
			CSipChanDif* pChanDif		= NULL;

			pChanDif = m_pChanDifArr->GetChanDif(mediaType,globalDirectionArr[j],eRole);

			TRACEINTO << "media type: " << mediaType << ", direction: " << globalDirectionArr[j]
				<< ", current mode is media on: " << (int)m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole)
				<< ", target mode is media on: " << (int)m_pTargetMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole)
				<< ", isEncrypted: " << isEncrypted;

			if (m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole) == NO)
			{
				// BRIDGE-4049 - don't reopen bfcp channels if need to close them
				if ( m_bNeedToCloseBfcpAndContentWithoutReinvite &&
				     ((mediaType == cmCapBfcp) || (mediaType == cmCapVideo && eRole == kRolePresentation)))
				     continue;

				BYTE AddChannelAccording2Scm = m_pTargetMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole);

				if((pChanDif && pChanDif->IsAddChannel()) || AddChannelAccording2Scm)
				{
					CBaseCap* pMedia = m_pTargetMode->GetMediaAsCapClass(mediaType, globalDirectionArr[j],eRole);
					if (pMedia && pNewMediaMode)
					{
						pNewMediaMode->SetMediaMode(pMedia, mediaType, globalDirectionArr[j],eRole);
#if 1
						if(mediaType == cmCapVideo && eRole == kRolePeople &&  globalDirectionArr[j]==cmCapReceive)
						{
                            std::list <StreamDesc> targetStreamsDescList = m_pTargetMode->GetStreamsListForMediaMode(mediaType, globalDirectionArr[j],eRole);
                            pNewMediaMode->SetStreamsListForMediaMode(targetStreamsDescList,mediaType, globalDirectionArr[j],eRole);
						}
#endif

						//Copy also SRTP if enable
						if(isEncrypted == Encryp_On)
						{
							pPrefferedSdesCap =  m_pTargetMode->GetSipSdes(mediaType,globalDirectionArr[j],eRole);
							if(pPrefferedSdesCap != NULL) {
								pNewMediaMode->SetSipSdes(mediaType,globalDirectionArr[j],eRole,pPrefferedSdesCap);
								if(bIsFirstEncAppearance)
								{
									pNewMediaMode->SetEncryption(Encryp_On,((CSipComMode*)m_pTargetMode)->GetIsDisconnectOnEncryptionFailure());
									bIsFirstEncAppearance = 0;
								}
							}
						}

						numOfOpenChannels++;
						TRACEINTO << "%#%# -4- numOfOpenChannels: " << numOfOpenChannels;
					}
					POBJDELETE(pMedia);
				}
			}
		}
	}

	// check for internal channels
    bool isChangeInternalChannels = false;
    if (m_pTargetMode->GetConfMediaType() == eMixAvcSvc && !m_pSipCntl->GetIsMrcCall())
    {
        APIU32* ssrcIds = NULL;
        int numOfSsrcIds = 0;
        m_pTargetMode->GetSsrcIds(cmCapVideo, cmCapReceive, ssrcIds, &numOfSsrcIds);

        APIU32* currentSsrcIds = NULL;
        int currentNumOfSsrcIds = 0;
        m_pCurrentMode->GetSsrcIds(cmCapVideo, cmCapReceive, currentSsrcIds, &currentNumOfSsrcIds);

        TRACEINTO << "ChangeInternalChannels: numOfSsrcIds=" << numOfSsrcIds << " currentNumOfSsrcIds=" << currentNumOfSsrcIds;
        if (numOfSsrcIds > currentNumOfSsrcIds)
            isChangeInternalChannels = true;

       	delete[] ssrcIds;
       	ssrcIds = NULL;

       	delete[] currentSsrcIds;
       	currentSsrcIds = NULL;
     }

	if (numOfOpenChannels == 0 && !isChangeInternalChannels)
	{//send message to party on all requested channels are closed.
		PTRACE2(eLevelInfoNormal, "CSipTransaction::OpenChannelsIfNeededForReInvite, there are no channels to open. Send all needed channels are opened - ", m_pPartyConfName);
		RequiredChannelsActionDone(SIP_PARTY_CHANS_CONNECTED);
	}
	else if (numOfOpenChannels > 0)	//we need to send in the SCM only the new channels!!
	{
		TRACEINTO << "numOfOpenChannels: " << numOfOpenChannels;

		pNewMediaMode->Dump("pNewMediaMode open channels - before", eLevelInfoNormal);

//		if (bIsToOpenAudio || bIsToOpenVideo)
//			RestoreStreamGroups(m_pTargetModeMaxAllocation, pNewMediaMode, (WORD)bIsToOpenAudio, (WORD)bIsToOpenVideo);
		RestoreStreamGroups(pNewMediaMode);

		pNewMediaMode->Dump("pNewMediaMode open channels - after", eLevelInfoNormal);

        if( m_pSipCntl )
            m_pSipCntl->SipOpenChannelsReq(pNewMediaMode,eDirection,isAnsweringToNewCap);
		if ( m_pParty->GetIsTipCall() )
		{
			CSegment * pSeg = new CSegment;

			DWORD dir = (DWORD) eDirection;
			*pSeg << dir;
			m_pTargetMode->Serialize(NATIVE,*pSeg);

			m_pParty->ForwardEventToTipSlaves(pSeg, SIP_TRANS_SLAVE_OPEN_CHANS);

			POBJDELETE(pSeg);
		}
	}
	else
	{
        TRACEINTO << "Open internal channels";
//        RestoreStreamGroups(pNewMediaMode);
//        pNewMediaMode->Dump("pNewMediaMode open channels - after restore streams", eLevelInfoNormal);
        if( m_pSipCntl )
            m_pSipCntl->OpenInternalChannels((CSipComMode *)m_pTargetMode, (CSipComMode *)m_pCurrentMode, kConnecting);
	}

	POBJDELETE(pNewMediaMode);
}
///////////////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::GetRtpStatisticsIfNeededForReinvite()
{
	//CDR_MCCF:
	BOOL bEnableCdrMCCF = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_CDR_FOR_MCCF);

	m_state = sTRANS_SAVESTATISTICINFOBEFORECLOSECHANNEL;

	if (bEnableCdrMCCF == TRUE)
	{
		PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipTransaction::GetRtpStatisticsIfNeededForReinvite bEnableCdrMCCF == TRUE");

		if (CheckTxChannelsVideoAndContentForCdrMCCF())
		{
			BYTE sendReq = m_pSipCntl->SendRtpVideoChannelStatisticsReq();
			if (sendReq == TRUE)
			{
				PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipTransaction::GetRtpStatisticsIfNeededForReinvite SendRtpVideoChannelStatisticsReq");
				return TRUE;
			}
		}
	}

	PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipTransaction::GetRtpStatisticsIfNeededForReinvite bEnableCdrMCCF == FALSE");
	RequiredChannelsActionDone(SIP_PARTY_STATISTIC_INFO);

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::CloseDtlsChannelsBeforeSipChannelsIfNeeded()
{
	BYTE bCloseDtlsMessageSent = FALSE;
	cmCapDataType 	mediaType;
	ERoleLabel 		eRole;

	BYTE removeDtlsChannelAccording2Scm;

	m_state = sTRANS_DTLS_CLOSE_BEFORE_CLOSING_SIP_CHANNEL;

	CSipCall* pCall = m_pSipCntl->GetCallObj();

	COstrStream msg1;
	m_pCurrentMode->Dump(msg1);
	PTRACE2(eLevelError,"CSipTransaction::CloseDtlsChannelsBeforeSipChannelsIfNeeded, m_pCurrentMode : ", msg1.str().c_str());

	COstrStream msg2;
	m_pTargetMode->Dump(msg2);
	PTRACE2(eLevelError,"CSipTransaction::CloseDtlsChannelsBeforeSipChannelsIfNeeded, m_pTargetMode : ", msg2.str().c_str());

	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if ((mediaType != cmCapAudio) && ((mediaType != cmCapVideo) || eRole != kRolePeople))
			continue;

		for (int j = 0; j < 2; j++)// 2 = number of direction (receive, transmit).
		{
			CSipChanDif* pChanDif		= NULL;
			pChanDif = m_pChanDifArr->GetChanDif(mediaType,globalDirectionArr[j],eRole);

			removeDtlsChannelAccording2Scm = ((m_pCurrentMode->IsDtlsChannelEnabled(mediaType,globalDirectionArr[j],eRole) == TRUE) &&
										 	 ((m_pTargetMode->IsDtlsChannelEnabled(mediaType,globalDirectionArr[j],eRole) == FALSE)));

			// if active DTLS, ignore SDES changes
			CSipChannel* pChannel = pCall->GetChannel(true, mediaType, globalDirectionArr[j],eRole);

			if (!pChannel)
			{
				PTRACE2INT(eLevelInfoNormal,"CSipTransaction::CloseDtlsChannelsBeforeSipChannelsIfNeeded pChannel is NULL, mediaType:", mediaType);
				continue;
			}

			BOOL bIsNeedToCloseDtlsForMutedChannel 		= FALSE;
			BOOL bIsNeedToCloseDtlsForRemovedChannel 	= FALSE;

			if (pChanDif)
			{
				bIsNeedToCloseDtlsForMutedChannel 	= (pChanDif->IsMute() && IsCallHold());
				bIsNeedToCloseDtlsForRemovedChannel = pChanDif->IsRemoveChannel();
			}

			if (removeDtlsChannelAccording2Scm || bIsNeedToCloseDtlsForRemovedChannel || bIsNeedToCloseDtlsForMutedChannel)
			{
				if (pChanDif)
				{
					TRACEINTO << "close channel reason - is remove channel: " << (int) bIsNeedToCloseDtlsForRemovedChannel
					<< "\n close dtls for muted channel: " << (int) bIsNeedToCloseDtlsForMutedChannel
					<< "\n remove dtls according to scm: " << (int) removeDtlsChannelAccording2Scm ;
				}
				else
				{
					TRACEINTO << "close dtls channel reason - according to scm: " << (int) removeDtlsChannelAccording2Scm;
				}

				if (m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole))
				{
					CLargeString str;

					str << ", mediaType: " << mediaType
						<< ", direction: " << pChannel->GetDirection()
						<< ", dtls state: " << pChannel->GetDtlsConnectionState()
						<< "\n, channel state: " << pChannel->GetConnectionState()
						<< ", is channel dtls enable: " << pChannel->IsChannelDtlsEnabled()
						<< ", removeDtlsChannelAccording2Scm:" << removeDtlsChannelAccording2Scm
						<< ", dtls state in target mode:" << m_pTargetMode->GetIsDtlsEncrypted();

					PTRACE2(eLevelError,"CSipTransaction::CloseDtlsChannelsBeforeSipChannelsIfNeeded :",str.GetString());

					if ((pChannel->IsChannelDtlsEnabled() && m_pTargetMode->GetIsDtlsEncrypted() == Encryp_On && pChannel->GetDtlsConnectionState() == kConnected) ||
						removeDtlsChannelAccording2Scm || bIsNeedToCloseDtlsForRemovedChannel || bIsNeedToCloseDtlsForMutedChannel)
					{
						TRACEINTO << "close dtls before closing sip channel - media type:" << mediaType << ", direction:" << globalDirectionArr[j];
						bCloseDtlsMessageSent |= m_pSipCntl->SipCloseDtlsChannelReq(pChannel);
					}
				}
			}
		}
	}

	if (bCloseDtlsMessageSent)
		return TRUE;
	else
		RequiredChannelsActionDone(SIP_PARTY_DTLS_CHANS_DISCONNECTED);

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::CloseChannelsIfNeededForReceiveReInvite()
{// best programing was to unified it with CloseChannelsIfNeeded function, but I don't have the time for that.

    PTRACE(eLevelError,"CSipTransaction::CloseChannelsIfNeededForReceiveReInvite");
	//const CSipCaps*	pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
	EIpChannelType eChanType;
	BYTE bIsTransmit = FALSE;
	int numOfCloseChannels = 0;
	DWORD channelsMask = 0;
	cmCapDataType mediaType;
	ERoleLabel eRole;

	BYTE removeChannelAccording2Scm;

	CSipCall* pCall = m_pSipCntl->GetCallObj();

	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		for (int j = 0; j < 2; j++)// 2 = number of direction (receive, transmit).
		{
			TRACEINTO << "media type: " << mediaType << ", direction: " << globalDirectionArr[j]
				<< ", current mode is media on: " << (int)m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole)
				<< ", target mode is media on: " << (int)m_pTargetMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole);
			CSipChanDif* pChanDif		= NULL;
			pChanDif = m_pChanDifArr->GetChanDif(mediaType,globalDirectionArr[j],eRole);
			removeChannelAccording2Scm = m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole) &&
										 m_pTargetMode->IsMediaOff(mediaType,globalDirectionArr[j],eRole);

			// if active DTLS, ignore SDES changes
			CSipChannel* pChannel = pCall->GetChannel(true, mediaType, globalDirectionArr[j],eRole);

			if (!pChannel)
			{
				PTRACE2INT(eLevelInfoNormal,"CSipTransaction::CloseChannelsIfNeededForReceiveReInvite pChannel is NULL, mediaType:", mediaType);
				continue;
			}

			if (pChanDif && (pChanDif->IsChangeSdes() && pChannel->IsChannelDtlsEnabled() && m_pTargetMode->GetIsDtlsEncrypted() == Encryp_On))
			{
				PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipTransaction::CloseChannelsIfNeededForReceiveReInvite set change sdes to no for tip in DTLS call");
				pChanDif->SetChangeSdes(FALSE);
			}


			BOOL removeChannelForEncReset = FALSE;
			if( !m_pParty->GetIsTipCall() && pChannel->IsChannelSdesEnabled() &&
					m_pTargetMode->GetIsEncrypted() && IsReInvite() && m_pSipCntl->CheckIfCallisResumedByMedia(m_pChanDifArr) )
			{
				BYTE ResetSdesForVendorFlag = GetResetSdesForVendorFlag();
				if( ResetSdesForVendorFlag == MASK_RESET_SDES_FOR_ALL )
					removeChannelForEncReset = TRUE;
				else if (ResetSdesForVendorFlag & MASK_RESET_SDES_FOR_CISCO)
				{
				   if( m_pSipCntl->GetRemoteIdent()==TandbergEp || m_pSipCntl->GetRemoteIdent()==CiscoCucm)
					   removeChannelForEncReset = TRUE;
				}
				else if(ResetSdesForVendorFlag & MASK_RESET_SDES_FOR_MICROSOFT)
				{
				   if( m_pSipCntl->IsRemoteMicrosoft() )
					   removeChannelForEncReset = TRUE;
				}
				else if(ResetSdesForVendorFlag & MASK_RESET_SDES_FOR_POLYCOM)
				{
				   if( m_pSipCntl->GetRemoteIdent() == PolycomEp )
					   removeChannelForEncReset = TRUE;
				}
			}

			if (  ( pChanDif && (pChanDif->IsRemoveChannel() || pChanDif->IsChangeSdes() || pChanDif -> IsChangeQoS())) ||
				  removeChannelAccording2Scm || m_bNeedCloseIceChannels || removeChannelForEncReset
			   )
			{
				if(pChanDif)
				{
					TRACEINTO << "close channel reason - is remove channel: " << (int)pChanDif->IsRemoveChannel() <<
					", is change SDES: " << (int)pChanDif->IsChangeSdes() <<", is change QoS: " << (int)pChanDif->IsChangeQoS() <<
					"\n remove according to scm: " << (int)removeChannelAccording2Scm <<
					", need to close ICE channel: " << (int)m_bNeedCloseIceChannels <<
					", need to close channel for Enc reset: " << (int)removeChannelForEncReset;
				}
				else
				{
					TRACEINTO << "close channel reason - according to scm: " << (int)removeChannelAccording2Scm <<
					", need to close ICE channel: " << (int)m_bNeedCloseIceChannels <<
					", need to close channel for Enc reset: " << (int)removeChannelForEncReset;
				}

				if(m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole))
				{
					PTRACE2INT(eLevelInfoNormal,"CSipTransaction::CloseChannelsIfNeededForReceiveReInvite: close channel mediaType: ", (int) mediaType);
					bIsTransmit = (globalDirectionArr[j] == cmCapTransmit);
					eChanType = ::CalcChannelType(mediaType,bIsTransmit,eRole);
					if(eRole==kRolePeople  && mediaType==cmCapVideo && globalDirectionArr[j]==cmCapReceive)
					{
						if(m_pTargetMode->GetConfMediaType() == eMixAvcSvc && !m_pSipCntl->GetIsMrcCall())
						{
							if (IsSoftMcu() && m_pSipCntl->IsAtLeastOneInternalTranslatorArtConnected())
							{
								m_bNeedToCloseInternalVideoArt=true;
								TRACEINTO<<"changing m_bNeedToCloseInternalVideoArt to true";
							}
							if(!IsSoftMcu() && m_pSipCntl->AreAllInternalArtsConnected())
							{
								m_bNeedToCloseInternalVideoArt=true;
								TRACEINTO<<"changing m_bNeedToCloseInternalVideoArt to true";
							}
						}
					}
					m_pSipCntl->SipCloseChannelReq(eChanType);

					AddToChannelsMask(channelsMask, eChanType);
					numOfCloseChannels++;
				}
			}
		}
	}

	if(numOfCloseChannels == 0)
	{//send message to party on all requested channels are closed.
		PTRACE2(eLevelError, "CSipTransaction::CloseChannelsIfNeededForReceiveReInvite, there are no channels to close. Send all needed channels are in disconnected state - ", m_pPartyConfName);
		RequiredChannelsActionDone(SIP_PARTY_CHANS_DISCONNECTED);
	}
	else
	{
		if (m_pParty->GetIsTipCall() && channelsMask)
		{
			CSegment * pSeg = new CSegment;

			*pSeg << channelsMask;

			m_pParty->ForwardEventToTipSlaves(pSeg, SIP_TRANS_SLAVE_CLOSE_CHANS);

			POBJDELETE(pSeg);

		}
	}
}




///////////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::AddToChannelsMask(DWORD &channelsMask, EIpChannelType eChanType)
{
	switch(eChanType)
	{
		case AUDIO_IN:
			channelsMask |= 0x0001;
			break;
		case AUDIO_OUT:
			channelsMask |= 0x0002;
			break;

		case VIDEO_IN:
			channelsMask |= 0x0004;
			break;
		case VIDEO_OUT:
			channelsMask |= 0x0008;
			break;

		case VIDEO_CONT_IN:
			channelsMask |= 0x0010;
			break;
		case VIDEO_CONT_OUT:
			channelsMask |= 0x0020;
			break;

		case FECC_IN:
			channelsMask |= 0x0040;
			break;
		case FECC_OUT:
			channelsMask |= 0x0080;
			break;

		case BFCP_IN:
			channelsMask |= 0x0100;
			break;
		case BFCP_OUT:
			channelsMask |= 0x0200;
			break;
		default:
			FPTRACE(eLevelInfoNormal,"Unexpected channel type");
	}
}
///////////////////////////////////////////////////////////////////////////////////////
//CDR_MCCF:
BYTE CSipTransaction::CheckTxChannelsVideoAndContentForCdrMCCF()
{
    PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipTransaction::CheckTxChannelsVideoAndContentForCdrMCCF");

    cmCapDataType mediaType;
    ERoleLabel eRole;
    BYTE removeChannelAccording2Scm;

    for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
    {
        GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
        for (int j = 0; j < 2; j++)// 2 = number of direction (receive, transmit).
        {
            CSipChanDif* pChanDif       = NULL;
            pChanDif = m_pChanDifArr->GetChanDif(mediaType,globalDirectionArr[j],eRole);
            removeChannelAccording2Scm = m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole) &&
                                         m_pTargetMode->IsMediaOff(mediaType,globalDirectionArr[j],eRole);

            if( ((mediaType == cmCapVideo) &&  (globalDirectionArr[j] == cmCapTransmit) && (eRole == kRolePeople || eRole == kRolePresentation) &&
                pChanDif && ((pChanDif && pChanDif->IsRemoveChannel()) || pChanDif->IsChangeSdes())) || removeChannelAccording2Scm ||m_bNeedCloseIceChannels)
            {
                 if(m_pCurrentMode->IsMediaOn(mediaType,globalDirectionArr[j],eRole))
                {
                    PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipTransaction::CheckTxChannelsVideoAndContentForCdrMCCF: found! close channel");
                    return TRUE;
                }
            }
        }
    }

    return FALSE;
}

///////////////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::StartDtlsIfNeeded()
{
	PTRACE2(eLevelInfoNormal, "CSipTransaction::StartDtlsIfNeeded - ", m_pPartyConfName);

	if (!m_pParty->GetIsTipCall())
	{
		PTRACE(eLevelInfoNormal, "CSipTransaction::StartDtlsIfNeeded - don't start, not a TIP call");
		return FALSE;
	}

	if (m_pCurrentMode->GetIsDtlsEncrypted() != Encryp_On)
	{
		PTRACE(eLevelInfoNormal, "CSipTransaction::StartDtlsIfNeeded - don't start, not a secured DTLS call");
		return FALSE;
	}

	if( m_pSipCntl->IsPendingTrns() != eNoPendTrans )
    {
		PTRACE(eLevelInfoNormal, "CSipTransaction::StartDtlsIfNeeded - don't start, there are pending transactions");
		return FALSE;
    }

	//Check if SDES negotiated with sha_32 suite
	if(m_pCurrentMode->GetIsEncrypted() == Encryp_On)
	{
		//Assuming audio crypto suite represents all media suites
		CMediaModeH323& audioModeTx = m_pCurrentMode->GetMediaMode(cmCapAudio, cmCapTransmit);
		CSdesCap* audioSdesCap = audioModeTx.GetSdesCap();
		if(audioSdesCap && audioSdesCap->GetSdesCryptoSuite() == eSha1_length_32)
		{
			PTRACE(eLevelInfoNormal, "CSipTransaction::StartDtlsIfNeeded - don't start, use SDES (SHA_32)");
			return FALSE;
		}
	}

	m_pSipCntl->CloseTipSessionIfNeeded();
	m_pSipCntl->SetIsOneDtlsArrived(FALSE);
	m_state = sTRANS_DTLS_STARTED;

	m_pSipCntl->SipStartDtlsChannelReq(cmCapReceive);
	m_pSipCntl->SipStartDtlsChannelReq(cmCapTransmit);

	CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();

	DWORD dtlsTimeout = 5;

	if (sysConfig)
		sysConfig->GetDWORDDataByKey("DTLS_TIMEOUT", dtlsTimeout);

	if (dtlsTimeout > 0)
		StartTimer(DTLSTOUT, dtlsTimeout*SECOND);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::OnPartyDtlsEndInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransaction::OnPartyDtlsEndInd");

	APIU32		status;				// status 0 - OK, 1 Violate.
	APIU32		channelType;		// kChanneltype
	APIU32		channelDirection;	// cmCapDirection.

	cmCapDirection eDirection;
	cmCapDataType mediaType;
	ERoleLabel 	  eRole;

	CSipChanDif	*pChanDif	= NULL;

	*pParam >> status;

	if (IsValidTimer(DTLSTOUT))
		DeleteTimer(DTLSTOUT);

	// All DTLS channels were opened successfully
	// need to update RTP
	if (status == STATUS_OK)
	{
		m_pCurrentMode->SetDtlsAvailable(TRUE);
		m_pTargetMode->SetDtlsAvailable(TRUE);

		m_state = sTRANS_DTLS_UPDATED_CHAN;

		UpdateDtlsChannels();
		SendUpdateDbEncryptionStatusToParty(YES);
	}
	else
	{
		DisconnectOnDtlsEncryptFail();
	}
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::UpdateDtlsChannels()
{
	BYTE bChangeParams = 0;

	EIpChannelType 	chanArr = H225;
	int 			numOfUpdateDtlsChannels = 0;
	cmCapDataType 	mediaType;
	ERoleLabel 		eRole;

	PTRACE2(eLevelInfoNormal, "CSipTransaction::UpdateDtlsChannels,- ", m_pPartyConfName); //TBD - we need to update with new scm if the video channels are not the same

	for (int i = 0 ; i < CHANNEL_TYPES_COUNT; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if(mediaType != cmCapAudio && mediaType != cmCapVideo)
			continue;

		for (int j = 0; j < 2; j++)// 2 = number of direction (receive, transmit).
		{
			bChangeParams |= kChangeDtls;

			if((globalDirectionArr[j] == cmCapReceive))
			{
				chanArr = ::CalcChannelType(mediaType, NO, eRole);

				PTRACE2INT(eLevelInfoNormal, "CSipTransaction::UpdateDtlsChannels, change receive dtls, media type:", mediaType);
			}
			else if (globalDirectionArr[j] == cmCapTransmit)
			{
				chanArr = ::CalcChannelType(mediaType,YES, eRole);
				PTRACE2INT(eLevelInfoNormal, "CSipTransaction::UpdateDtlsChannels, change transmit dtls, media type:", mediaType);
			}
			
			if(m_pSipCntl->SipUpdateChannelReq((CSipComMode*)m_pTargetMode, chanArr, bChangeParams))
				numOfUpdateDtlsChannels++;
		}
	}

	if(numOfUpdateDtlsChannels == 0)
	{//send message to party on all requested channels are closed.
		PTRACE2(eLevelInfoNormal, "CSipTransaction::UpdateDtlsChannels, there are no channels to update. Send all channels are updated - ", m_pPartyConfName);
		RequiredChannelsActionDone(SIP_PARTY_CHANS_UPDATED);
	}
	else
	    PTRACE2INT(eLevelInfoNormal, "CSipTransaction::UpdateDtlsChannels updated channels count=",numOfUpdateDtlsChannels);
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::OnDtlsTout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::OnDtlsTout: Name ",m_pPartyConfName);
	DisconnectOnDtlsEncryptFail();
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::CloseDtlsChannels()
{
	PTRACE(eLevelInfoNormal, "CSipTransaction::CloseDtlsChannels");

	m_state = sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE;

	m_pSipCntl->SipCloseAllDtlsChannelsAfterDtlsFailureReq();
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::TipEarlyPacketDtlsNotNeeded()
{
	TRACEINTO;
	if( !m_bTipEarlyPacket)
	{
		BOOL  bIsUndefinedParty 		= m_pSipCntl->IsUndefinedParty();
		BYTE  bIsDisconnectOnEncFailure = m_pTargetMode->GetIsDisconnectOnEncryptionFailure();
		BYTE  bIsWhenAvailableEncMode 	= m_pSipCntl->IsWhenAvailableEncryptionMode();
		DWORD isEncrypted 				= m_pTargetMode->GetIsEncrypted();
		m_pCurrentMode->SetDtlsEncryption(Encryp_Off);
		m_pTargetMode->SetDtlsEncryption(Encryp_Off);
		m_pTargetModeMaxAllocation->SetDtlsEncryption(Encryp_Off);
		m_bTipEarlyPacket = TRUE;
		// Check if Undefind party and flag, that allow non enc to connect and when available
		// if so, remove SDES from RMX side
		// set enc to off
		CheckEncryptionAndDisconnectIfNeeded( bIsUndefinedParty, bIsDisconnectOnEncFailure, bIsWhenAvailableEncMode, isEncrypted );
	}
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::DisconnectOnDtlsEncryptFail()
{
	CSipComMode* pComMode	= (CSipComMode*)m_pTargetMode;

	if(!pComMode || !m_pSipCntl)
	{
		DBGPASSERT(YES);
		PTRACE2(eLevelError,"CSipTransaction::DisconnectOnDtlsEncryptFail: No mode!. name: ",m_pPartyConfName);
		return;
	}

	// Check if Undefind party and flag, that allow non enc to connect and when available
	// if so, remove SDES from RMX side
	// set enc to off
	BOOL  bIsUndefinedParty 		= m_pSipCntl->IsUndefinedParty();
	BYTE  bIsDisconnectOnEncFailure = pComMode->GetIsDisconnectOnEncryptionFailure();
	BYTE  bIsWhenAvailableEncMode 	= m_pSipCntl->IsWhenAvailableEncryptionMode();
	DWORD bIsDtlsEncrypted 			= pComMode->GetIsDtlsEncrypted();
	BYTE  bIsDtlsAvailable 			= pComMode->GetIsDtlsAvailable();
	DWORD isEncrypted 				= pComMode->GetIsEncrypted();

	if(bIsDtlsEncrypted != Encryp_On)
	{
		DBGPASSERT(YES);
		PTRACE2(eLevelError,"CSipTransaction::DisconnectOnDtlsEncryptFail: No DTLS encryption. name: ",m_pPartyConfName);
		return;
	}

	if(!bIsDtlsAvailable)
	{
		PTRACE2(eLevelError,"CSipTransaction::DisconnectOnDtlsEncryptFail: DTLS encryption failed. name: ",m_pPartyConfName);
		CloseDtlsChannels();
		CheckEncryptionAndDisconnectIfNeeded( bIsUndefinedParty, bIsDisconnectOnEncFailure, bIsWhenAvailableEncMode, isEncrypted );
	}
	else
	{
		DBGPASSERT(YES);
		PTRACE2(eLevelError,"CSipTransaction::DisconnectOnDtlsEncryptFail: Close DTLS encryption but DTLS is available. name: ",m_pPartyConfName);
	}
}

void CSipTransaction::CheckEncryptionAndDisconnectIfNeeded(BOOL bIsUndefinedParty, BYTE bIsDisconnectOnEncFailure, BYTE bIsWhenAvailableEncMode, DWORD isEncrypted )
{
		//Check SDES status
		if(isEncrypted == Encryp_On)
		{
			//SDES Available
			PTRACE2(eLevelError,"CSipTransaction::DisconnectOnDtlsEncryptFail: SDES encryption is available, just close DTLS. Name ",m_pPartyConfName);
		}
		else //No SDES and NO DTLS Available
		{
		BYTE  confEncMode = m_pSipCntl->GetConfEncryptionMode();
		if( (confEncMode == eEncryptNone) || (bIsUndefinedParty && bIsDisconnectOnEncFailure==NO && bIsWhenAvailableEncMode) ||
				(!bIsUndefinedParty && bIsWhenAvailableEncMode))
			{
				//Keep party connected
				PTRACE2(eLevelError,"CSipTransaction::DisconnectOnDtlsEncryptFail: Close DTLS and keep party connected anyway. Name ",m_pPartyConfName);
			}
			else
			{
				//Disconnect party
				DBGPASSERT(YES);
				PTRACE2(eLevelError,"CSipTransaction::DisconnectOnDtlsEncryptFail: SDES cap is incorrect or not matching. Name ",m_pPartyConfName);
				SetDialState(kNoRecovery);
				EndTransaction(SIP_CAPS_DONT_MATCH);
		}
	}

}

///////////////////////////////////////////////////////////////////////////////////////
WORD CSipTransaction::SendIceMgsReqAccordingToTargetModeAndDiffArr(DWORD Opcode,BYTE bChangeInMedia)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndDiffArr: Name ",m_pPartyConfName);
    WORD Status = STATUS_OK;
    m_bIsChangeInICEChannels = FALSE;

	CSipChanDifArr*	pIceDifArr	= new CSipChanDifArr;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if (mediaType == cmCapBfcp)
			continue;

			CSipChanDif* pChanDif		= NULL;
			pChanDif = m_pChanDifArr->GetChanDif(mediaType,cmCapTransmit,eRole);

			CSipChanDif* pIceChanDif	= NULL;
			pIceChanDif = pIceDifArr->GetChanDif(mediaType,cmCapTransmit,eRole);
			if (!pIceChanDif)
			{
			    PTRACE(eLevelError, "CSipTransaction::SendIceMgsReqAccordingToTargetModeAndDiffArr - pIceChanDif is NULL");
			    DBGPASSERT(1129);
				POBJDELETE(pIceDifArr);
			    return STATUS_FAIL;
			}

			BYTE removeChannelAccording2Scm = m_pCurrentMode->IsMediaOn(mediaType,cmCapReceive,eRole) &&
												m_pTargetMode->IsMediaOff(mediaType,cmCapReceive,eRole);

			BYTE AddChannelAccording2Scm = m_pCurrentMode->IsMediaOff(mediaType,cmCapReceive,eRole) &&
											m_pTargetMode->IsMediaOn(mediaType,cmCapReceive,eRole);

			// Is there a change in best mode. For example incase of secondary ep when remote send reinvite with video caps but
			// the video is still lower and find best mode is still doesn't include video.
			if (!bChangeInMedia)
			{
				PTRACE(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndDiffArr - No change in media");

				if(m_pCurrentMode->IsMediaOn(mediaType,cmCapReceive,eRole)) //If there is no change but media is open - the
					pIceChanDif->SetAddChannel(YES);
				else
					pIceChanDif->SetAddChannel(NO);

				pIceChanDif->SetRemoveChannel(NO);
			}
			///// 2 . Check if need to close //////////////
			else if((pChanDif && pChanDif->IsRemoveChannel() ) //&& m_pCurrentMode->IsMediaOn(mediaType,cmCapReceive,eRole)) //BRIDGE-6293
						|| removeChannelAccording2Scm)
			{
				PTRACE(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndDiffArr - Need to modify/remove Ice channel");
				pIceChanDif->SetRemoveChannel(YES);
				pIceChanDif->SetAddChannel(NO);
				m_bIsChangeInICEChannels = TRUE;
			}
			///// 3.  Check if need to open //////////////
			else if(AddChannelAccording2Scm)
			{
				PTRACE(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndDiffArr - Need to modify/add Ice channel");
				pIceChanDif->SetRemoveChannel(NO);
				pIceChanDif->SetAddChannel(YES);
				m_bIsChangeInICEChannels = TRUE;
			}
			else if (m_pCurrentMode->IsMediaOn(mediaType,cmCapReceive,eRole)) //If there is no change but media is open - the
			{
				PTRACE(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndDiffArr - No change in channel - still open");
				pIceChanDif->SetRemoveChannel(NO);
				pIceChanDif->SetAddChannel(YES);

			}
			else
			{
				PTRACE(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndDiffArr - No change in channel - still close");
				pIceChanDif->SetRemoveChannel(NO);
				pIceChanDif->SetAddChannel(NO);

			}
	}


	if(Opcode == ICE_MODIFY_SESSION_OFFER_REQ)
	{
		m_pSipCntl->SipIceMakeOffer(Opcode,pIceDifArr,m_bIsChangeInICEChannels);
	}
	else
		Status = m_pSipCntl->SendRemoteICESdp(Opcode,pIceDifArr,m_bIsChangeInICEChannels,m_bNeedReInviteForSecondary, m_pTargetMode->IsEncrypted());

	POBJDELETE(pIceDifArr);
    return Status;
}
////////////////////////////////////////////////////////////////////////////////////////
WORD CSipTransaction::SendIceMgsReqAccordingToTargetMode(DWORD Opcode)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetMode: Name ",m_pPartyConfName);
    WORD Status = STATUS_OK;
 	CSipChanDifArr*	pIceDifArr	= new CSipChanDifArr;
    m_bIsChangeInICEChannels = FALSE;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
		BYTE removeChannelAccording2Scm = m_pCurrentMode->IsMediaOn(mediaType,cmCapReceive, eRole) &&
															m_pTargetMode->IsMediaOff(mediaType,cmCapReceive, eRole);

		BYTE AddChannelAccording2Scm = m_pCurrentMode->IsMediaOff(mediaType,cmCapReceive, eRole) &&
														m_pTargetMode->IsMediaOn(mediaType,cmCapReceive, eRole);

		CSipChanDif* pIceChanDif = NULL;
		pIceChanDif = pIceDifArr->GetChanDif(mediaType,cmCapTransmit, eRole);
		if (!pIceChanDif)
		{
		    PTRACE(eLevelError, "CSipTransaction::SendIceMgsReqAccordingToTargetMode - pIceChanDif is NULL");
			POBJDELETE(pIceDifArr);
		    DBGPASSERT(1130);
		    return STATUS_FAIL;
		}

		///// 2 . Check if need to open //////////////
		if(m_pTargetMode->IsMediaOn(mediaType,cmCapReceive, eRole))
		{
			pIceChanDif->SetRemoveChannel(NO);
			pIceChanDif->SetAddChannel(YES);
			if(AddChannelAccording2Scm)
				m_bIsChangeInICEChannels = TRUE;
		}
		///// 2 . Check if need to close //////////////
		else if(removeChannelAccording2Scm)
		{
			pIceChanDif->SetRemoveChannel(YES);
			pIceChanDif->SetAddChannel(NO);
			m_bIsChangeInICEChannels = TRUE;
		}
		else
		{
			pIceChanDif->SetRemoveChannel(NO);
			pIceChanDif->SetAddChannel(NO);
		}

	}
	if(Opcode == ICE_MAKE_OFFER_REQ)
	{
		m_pSipCntl->SipIceMakeOffer(Opcode,pIceDifArr,m_bIsChangeInICEChannels);
	}
	else
		Status = m_pSipCntl->SendRemoteICESdp(Opcode,pIceDifArr,m_bIsChangeInICEChannels,m_bNeedReInviteForSecondary,m_pTargetMode->IsEncrypted());

	POBJDELETE(pIceDifArr);
    return Status;
}
////////////////////////////////////////////////////////////////////////////////////////
WORD CSipTransaction::SendIceMgsReqAccordingToTargetModeAndCurrentMode(DWORD Opcode)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndCurrentMode: Name ",m_pPartyConfName);
    WORD Status = STATUS_OK;
 	CSipChanDifArr*	pIceDifArr	= new CSipChanDifArr;
    m_bIsChangeInICEChannels = FALSE;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		BYTE removeChannelAccording2Scm = m_pCurrentMode->IsMediaOn(mediaType,cmCapReceive,eRole) &&
															m_pTargetMode->IsMediaOff(mediaType,cmCapReceive,eRole);

		BYTE AddChannelAccording2Scm = m_pCurrentMode->IsMediaOff(mediaType,cmCapReceive,eRole) &&
														m_pTargetMode->IsMediaOn(mediaType,cmCapReceive,eRole);

		CSipChanDif* pIceChanDif = NULL;
		pIceChanDif = pIceDifArr->GetChanDif(mediaType,cmCapTransmit,eRole);
		if (!pIceChanDif)
		{
		    PTRACE(eLevelError, "CSipTransaction::SendIceMgsReqAccordingToTargetModeAndCurrentMode - pIceChanDif is NULL");
			POBJDELETE(pIceDifArr);
		    DBGPASSERT(1131);
		    return STATUS_FAIL;
		}


		///// 2 . Check if need to open //////////////
		if(AddChannelAccording2Scm)
		{
			pIceChanDif->SetRemoveChannel(NO);
			pIceChanDif->SetAddChannel(YES);
			m_bIsChangeInICEChannels = TRUE;
		}
		///// 2 . Check if need to close //////////////
		else if(removeChannelAccording2Scm)
		{
			//BRIDGE-10739 in-case of dial-out and first 200OK don't mind about removed channels
			//             1) Avoid sending modidy_session at this stage
			//             2) AFE would close them implicitly and
			if( m_bIsOfferer && !m_bIsReInvite)
			{
				m_bIsChangeInICEChannels = FALSE;
				pIceChanDif->SetRemoveChannel(NO);
				pIceChanDif->SetAddChannel(NO);
				PTRACE(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndCurrentMode - Dial-out 1st 200OK no change in channels");
			} else {
				pIceChanDif->SetRemoveChannel(YES);
				pIceChanDif->SetAddChannel(NO);
				m_bIsChangeInICEChannels = TRUE;
			}
		}
		else if (m_pCurrentMode->IsMediaOn(mediaType,cmCapReceive,eRole)) //If there is no change but media is open - the
		{
			PTRACE(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndCurrentMode - No change in channel - still open");
			pIceChanDif->SetRemoveChannel(NO);
			pIceChanDif->SetAddChannel(YES);

		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndCurrentMode - No change in channel - still close");
			pIceChanDif->SetRemoveChannel(NO);
			pIceChanDif->SetAddChannel(NO);
		}

	}
	PTRACE2INT(eLevelInfoNormal,"CSipTransaction::SendIceMgsReqAccordingToTargetModeAndCurrentMode - m_bIsChangeInICEChannels:",m_bIsChangeInICEChannels);
	Status = m_pSipCntl->SendRemoteICESdp(Opcode,pIceDifArr,m_bIsChangeInICEChannels,m_bNeedReInviteForSecondary,m_pTargetMode->IsEncrypted());

	POBJDELETE(pIceDifArr);
    return Status;
}
/////////////////////////////////////////////////////////////////////
//void CSipTransaction::MuteMediaIfNeeded(cmCapDirection eDirection, BYTE bIsTipForceMuteOutChannels)
//{
//	PTRACE(eLevelInfoNormal, "CSipTransaction::MuteMediaIfNeeded");
//	BYTE bAudioMuteIn		= AUTO;
//	BYTE bAudioMuteOut		= AUTO;
//	BYTE bVideoMuteIn		= AUTO;
//	BYTE bVideoMuteOut		= AUTO;
//	BYTE bFeccMuteOut		= AUTO;
//	BYTE bFeccMuteIn		= AUTO;
//	BYTE bContentMuteOut	= AUTO;
//	BYTE bContentMuteIn		= AUTO;
//	BYTE *MuteArray[MAX_SIP_MEDIA_TYPES][2] = {{&bAudioMuteIn, &bAudioMuteOut}, {&bVideoMuteIn, &bVideoMuteOut}, {&bFeccMuteIn, &bFeccMuteOut}, {&bContentMuteIn, &bContentMuteOut}};
//
//	BYTE bIsMute = NO;
//	BYTE bIsChanged = NO;
//
//	// check mute by party for in channels only. out channels will be checked after connected
//	CSipChannel* pChannel = NULL;
//	cmCapDataType mediaType;
//	ERoleLabel eRole;
//
//	// TIP
//	if (bIsTipForceMuteOutChannels)
//	{
//		PTRACE(eLevelInfoNormal, "CSipTransaction::MuteMediaIfNeeded, TIP force mute audio and video");
//		bAudioMuteOut = YES;
//		bVideoMuteOut = YES;
//	}
//
//	for(int i=0 ; i<MAX_SIP_MEDIA_TYPES; i++)
//	{
//		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);
//		for (int j=0; j < 2; j++)
//		{
//			if (eDirection & globalDirectionArr[j])
//			{
//				if (m_pSipCntl->SetMediaMuteState(mediaType, globalDirectionArr[j], MuteArray[i][j],eRole))
//					bIsChanged = YES;
//				if(*(MuteArray[i][j]) != AUTO)
//					MuteMediaByParty(*(MuteArray[i][j]), mediaType, globalDirectionArr[j],eRole);
//			}
//		}
//	}
//
//	if(bIsChanged)
//		SendMuteMediaToParty(bAudioMuteIn,bAudioMuteOut,bVideoMuteIn,bVideoMuteOut,bContentMuteIn,bContentMuteOut,bFeccMuteIn,bFeccMuteOut);
//}

//////////////////////////////////////////////////////
//void CSipTransaction::MuteMediaByParty(BYTE bIsMute,cmCapDataType eMedia,cmCapDirection eDirection, ERoleLabel eRole)
//{// we only mute the party DB, the mute of bridges is done by API.
//	PTRACE(eLevelInfoNormal,"CSipTransaction::MuteMediaByParty");
//	if (eDirection == cmCapReceiveAndTransmit)
//	{
//		EIpChannelType chanArr[2];
//		chanArr[0] = ::CalcChannelType(eMedia, NO, eRole);
//		chanArr[1] = ::CalcChannelType(eMedia, YES, eRole);
//		m_pSipCntl->MuteChannels(bIsMute,2,chanArr);
//	}
//	else
//	{
//		EIpChannelType chan = ::CalcChannelType(eMedia,eDirection==cmCapTransmit,eRole);
//		m_pSipCntl->MuteChannels(bIsMute,1,&chan);
//	}
//}

/////////////////////////////////////////////////////////////////////
// This function implement the flow after receiving channels disconnected message in change channels stage
void CSipTransaction::HandleChannelsDisconnectedStartRecovery(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransaction::HandleChannelsDisconnectedStartRecovery");
	UpdateDbChannelsStatus(pParam, FALSE);
	CheckChangingInCurrentMode();
}

/////////////////////////////////////////////////////////////////////
// This function implement the flow after receiving channels updated message in recovery stage
void CSipTransaction::HandleChannelsUpdatedDuringRecovery(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransaction::HandleChannelsUpdatedDuringRecovery");
	UpdateDbChannelsStatus(pParam, TRUE);

	// check mute by party for in channels only. out channels will be checked after connected
	SendMuteMediaToParty(cmCapReceive);// instead of MuteMediaIfNeeded(cmCapReceive);
	InternalRecoveryCompleted();
}

////////////////////////////////////////////////////////////////////////////////////////
// This function implement the flow after receiving channels connected message in connecting stage
void CSipTransaction::InformChannelsConnectedOpenBridges(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::InformChannelsConnectedOpenBridges: Name ",m_pPartyConfName);
	EConfType eConfType = m_pTargetMode->GetConfType();
	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pCurrentMode->SetConfType(eConfType);
	SendChannelsConnectedToParty();
}

/////////////////////////////////////////////////////////////////////
void CSipTransaction::HandleOutChannelsConnected(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransaction::HandleOutChannelsConnected");

	//channels out connected
	EConfType eConfType = m_pTargetMode->GetConfType();
	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
	m_pCurrentMode->SetConfType(eConfType);

	if( m_pTargetMode->GetIsTipMode() )
		m_bIsTipMute = TRUE;

	SendMuteMediaToParty(cmCapTransmit);// instead of MuteMediaIfNeeded(cmCapTransmit, m_pCurrentMode->GetIsTipMode());

	SendChannelsConnectedToParty();
	m_pSipCntl->SendRemoteNumbering();

	// After channels out were opened, start DTLS if needed.
	// If not, go to end transaction
	if (StartDtlsIfNeeded())
		return;

	PartyConnectCall();
}

//////////////////////////////////////////////////////
void CSipTransaction::OnPartyChannelsConnectedAnycase(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::OnPartyChannelsConnectedAnycase: Name ",m_pPartyConfName);
	UpdateDbChannelsStatus(pParam, TRUE);
}

/////////////////////////////////////////////////////////////////////
void CSipTransaction::OnPartyChannelsUpdatedAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransaction::OnPartyChannelsUpdatedAnycase");
	UpdateDbChannelsStatus(pParam, TRUE);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::OnConfPartyReceiveAudBridgeConnected()
{
	ON(m_isAudioBridgeConnected);
	HandleBridgeConnectedInd(STATUS_OK);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::OnConfPartyReceiveVidBridgeConnected()
{
	ON(m_isVideoBridgeConnected);
	HandleBridgeConnectedInd(STATUS_OK);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::OnConfPartyReceiveFeccBridgeConnected()
{
	ON(m_isFeccBridgeConnected);
	HandleBridgeConnectedInd(STATUS_OK);
}

//////////////////////////////////////////////////////
void CSipTransaction::OnPartyOriginalRemoteCaps(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::OnPartyOriginalRemoteCaps: Name ", m_pPartyConfName);

	CSipCaps* pRemoteCaps = new CSipCaps;
	

	if ( pRemoteCaps )
	{
		pRemoteCaps->DeSerialize(NATIVE, *pParam);
		SendOriginalRemoteCapsToParty(pRemoteCaps);
	}
	else
		PASSERT(1);
		

	POBJDELETE(pRemoteCaps);
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::IsNoRecovery(CSipComMode* pBestMode)
{
	BYTE rval = FALSE;
	PTRACE(eLevelInfoNormal,"CSipTransaction::IsNoRecovery");
	if(pBestMode == NULL)
		rval = TRUE;// if the best mode if null there is no recovery for the call
	else if(pBestMode->IsMediaOff(cmCapAudio,cmCapReceive) && pBestMode->IsMediaOff(cmCapAudio,cmCapTransmit))
		rval = TRUE;

	return rval;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::IsNoRecoveryForVideo(CSipComMode* pBestMode)
{
	// No Recovery for video - when traget has video and best mode hasn't got video.
	BYTE rval = FALSE;

	PTRACE(eLevelInfoNormal,"CSipTransaction::IsNoRecoveryForVideo");
	if(pBestMode && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive) && pBestMode->IsMediaOff(cmCapVideo,cmCapReceive))
		rval = TRUE;

	return rval;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::IsNoRecoveryForData(CSipComMode* pBestMode)
{
	// No Recovery for data - when target has data and best mode hasn't got data.
	BYTE rval = FALSE;

	PTRACE(eLevelInfoNormal,"CSipTransaction::IsNoRecoveryForData");
	if(pBestMode && m_pTargetMode->IsMediaOn(cmCapData,cmCapReceive) && pBestMode->IsMediaOff(cmCapData,cmCapReceive))
		rval = TRUE;

	return rval;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::IsNoRecoveryForBfcp(CSipComMode* pBestMode)
{
	// No Recovery for bfcp - when target has bfcp and best mode hasn't got bfcp.
	BYTE rval = FALSE;

	PTRACE(eLevelInfoNormal,"CSipTransaction::IsNoRecoveryForBfcp");

	if(pBestMode && m_pTargetMode->IsMediaOn(cmCapBfcp,cmCapReceive) && pBestMode->IsMediaOff(cmCapBfcp,cmCapReceive))
		rval = TRUE;

	return rval;
}

/////////////////////////////////////////////////////////////////////
void CSipTransaction::CloseChannelsIfNeeded( BYTE bIsCloseVideoChannels,
											 BYTE bIsCloseDataChannels/*=FALSE*/,
											 BYTE bIsCloseBfcpChannels/*=FALSE*/,
											 BYTE bIsCloseAudioChannels/*=FALSE*/,
											 BYTE bIsCloseContentChannels/*=FALSE*/,
											 BYTE bIsUpdateAnatIpType/*=FALSE*/ ) //add param for ANAT
{
	TRACEINTO << "multi_line - video: " << (bIsCloseVideoChannels ? "yes" : "no")
	          << ", data: " << (bIsCloseDataChannels ? "yes" : "no")
			  << ", bfcp: " << (bIsCloseBfcpChannels ? "yes" : "no")
			  << ", audio: " << (bIsCloseAudioChannels ? "yes" : "no")
			  << ", content: " << (bIsCloseContentChannels ? "yes" : "no")
			  << ", isUpdateAnatIpType: " << (bIsUpdateAnatIpType ? "yes" : "no");

	BYTE bMessageSent = NO;

    if (m_bNeedCloseIceChannels || m_bNeedCloseSrtpChannels)
    {
        bMessageSent |= m_pSipCntl->SipCloseChannelReq(AUDIO_IN);
		bMessageSent |= m_pSipCntl->SipCloseChannelReq(AUDIO_OUT);
        bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_IN);
		bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_OUT);
        bMessageSent |= m_pSipCntl->SipCloseChannelReq(FECC_IN);
		bMessageSent |= m_pSipCntl->SipCloseChannelReq(FECC_OUT);
        bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_CONT_IN);
		bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_CONT_OUT);

      if(bIsCloseBfcpChannels)
      {
          // no recovery only for BFCP
    	  TRACEINTO << "Remote has no BFCP/UDP support. Name " << m_pPartyConfName;

          bMessageSent |= m_pSipCntl->SipCloseChannelReq(BFCP_IN);
          bMessageSent |= m_pSipCntl->SipCloseChannelReq(BFCP_OUT);
      }

		// in case of ICE - for the future
//		if (m_bNeedCloseIceChannels)
//		{
//			// need to send goodbye before closing bfcp connection
//			m_pSipCntl->SendBfcpMessageReq(CONTENT_ROLE_BFCP_GOODBYE, m_pParty->GetMcuNum(), m_pParty->GetTerminalNum());
//
//			bMessageSent |= m_pSipCntl->SipCloseChannelReq(BFCP_IN);
//			bMessageSent |= m_pSipCntl->SipCloseChannelReq(BFCP_OUT);
//		}
    }
//    else if (m_bNeedUpdateSrtpChannels)
//    {
//    	// For SRTP, Content channels are currently AVC so we need to close and reopen them as in AVC flow, and not just update
//    	bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_CONT_IN);
//    	bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_CONT_OUT);
//    }
    else
    {
        if (bIsCloseVideoChannels)
        {
            // no recovery only for video
        	TRACEINTO << "Remote has no video. Name " << m_pPartyConfName;
			if(m_pTargetMode->GetConfMediaType() == eMixAvcSvc && !m_pSipCntl->GetIsMrcCall())
			{
				if (IsSoftMcu() && m_pSipCntl->IsAtLeastOneInternalTranslatorArtConnected())
				{
					m_bNeedToCloseInternalVideoArt=true;
					TRACEINTO<<"changing m_bNeedToCloseInternalVideoArt to true";
				}
				if(!IsSoftMcu() && m_pSipCntl->AreAllInternalArtsConnected())
				{
					m_bNeedToCloseInternalVideoArt=true;
					TRACEINTO<<"changing m_bNeedToCloseInternalVideoArt to true";
				}
			}
            m_pTargetMode->SetMediaOff(cmCapVideo,cmCapReceiveAndTransmit);
            bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_IN);
            bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_OUT);
        }

        if (bIsCloseDataChannels)
        {
            // no recovery only for data
        	TRACEINTO << "Remote has no data. Name " << m_pPartyConfName;
            m_pTargetMode->SetMediaOff(cmCapData,cmCapReceiveAndTransmit);
            bMessageSent |= m_pSipCntl->SipCloseChannelReq(FECC_IN);
            bMessageSent |= m_pSipCntl->SipCloseChannelReq(FECC_OUT);
        }

        if (bIsCloseBfcpChannels)
        {
            // no recovery only for BFCP
        	TRACEINTO << "Remote has no BFCP/UDP support. Name " << m_pPartyConfName;

            m_pTargetMode->SetMediaOff(cmCapBfcp,cmCapReceiveAndTransmit);

            bMessageSent |= m_pSipCntl->SipCloseChannelReq(BFCP_IN);
            bMessageSent |= m_pSipCntl->SipCloseChannelReq(BFCP_OUT);
        }

        if (bIsCloseAudioChannels)
        {
            // no recovery only for audio
            TRACEINTO << "Remote has no audio. Name " << m_pPartyConfName;
            m_pTargetMode->SetMediaOff(cmCapAudio,cmCapReceiveAndTransmit);

            bMessageSent |= m_pSipCntl->SipCloseChannelReq(AUDIO_IN);
            bMessageSent |= m_pSipCntl->SipCloseChannelReq(AUDIO_OUT);
        }

        if (bIsCloseContentChannels || m_bNeedUpdateSrtpChannels /* For SRTP, Content channels are currently AVC so we need to close and reopen them as in AVC flow, and not just update*/)
        {
            // no recovery only for audio
            TRACEINTO << "Remote has no content. Name " << m_pPartyConfName;

        	bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_CONT_IN);
        	bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_CONT_OUT);
        }


	//added for ANAT
	if (bIsUpdateAnatIpType)
	{
		 PTRACE2(eLevelInfoNormal,"CSipTransaction::CloseChannelsIfNeeded: ANAT close current channels. Name ",m_pPartyConfName);
		 if (!bIsCloseAudioChannels)
		 {
			bMessageSent |= m_pSipCntl->SipCloseChannelReq(AUDIO_IN);
			bMessageSent |= m_pSipCntl->SipCloseChannelReq(AUDIO_OUT);
		 }
		
		if (!bIsCloseVideoChannels)
		{
			bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_IN);
			bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_OUT);
		}

		if (!bIsCloseDataChannels)
		{
			bMessageSent |= m_pSipCntl->SipCloseChannelReq(FECC_IN);
			bMessageSent |= m_pSipCntl->SipCloseChannelReq(FECC_OUT);
		}

		if (!bIsCloseBfcpChannels)
		{
			bMessageSent |= m_pSipCntl->SipCloseChannelReq(BFCP_IN);
			bMessageSent |= m_pSipCntl->SipCloseChannelReq(BFCP_OUT);
		}

		  if (!bIsCloseContentChannels)
	        {
	        	bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_CONT_IN);
	        	bMessageSent |= m_pSipCntl->SipCloseChannelReq(VIDEO_CONT_OUT);
	        }
	}
    }

	if(bMessageSent == NO)
	{// no channel close, Send event to state mechine to continue the flow
		PTRACE2(eLevelInfoNormal, "CSipTransaction::CloseChannelsIfNeeded, there are no channels to close. Send all needed channels are in disconnected state - ", m_pPartyConfName);
		RequiredChannelsActionDone(SIP_PARTY_CHANS_DISCONNECTED);
	}

	m_pTargetModeMaxAllocation->Dump("multi_line - m_pTargetModeMaxAllocation close channels ", eLevelInfoNormal);
}

/////////////////////////////////////////////////////////////////////
void CSipTransaction::InformPartyRemoteConnect()
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::InformPartyRemoteConnect: Name ",m_pPartyConfName);
	SendRemoteConnectedToParty();
}

/////////////////////////////////////////////////////////////////////
void CSipTransaction::OpenOutChannels(BYTE isAnsweringToNewCap)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::OpenOutChannels: Name ",m_pPartyConfName);
	//open out channels
	if (m_pSipCntl->IsMedia(cmCapAudio,cmCapTransmit) == NO ||
		(m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive) && m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit) == NO))
	{
		if (m_pParty->GetIsTipCall())
			m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode,cmCapTransmit,isAnsweringToNewCap, eTipMasterCenter);
		else
		m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode,cmCapTransmit,isAnsweringToNewCap);
	}
	else
	{
		DBGPASSERT(1);// if we can't open out channels (at least audio) its a mistake.
	}
}
/////////////////////////////////////////////////////////////////////
void CSipTransaction::OpenInAndOutChannelsIfNeeded(BYTE isAnsweringToNewCap)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::OpenInAndOutChannelsIfNeeded: Name ",m_pPartyConfName);
    cmCapDirection eDirection;
	if (m_pSipCntl->IsMedia(cmCapAudio,cmCapTransmit) == NO ||
		(m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive) && m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit) == NO))
    {
        eDirection = cmCapTransmit;
        if (m_pSipCntl->IsMedia(cmCapAudio,cmCapReceive) == NO || m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive) == NO)
            eDirection = cmCapReceiveAndTransmit;

        if (m_pParty->GetIsTipCall())
        	m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode,eDirection,isAnsweringToNewCap, eTipMasterCenter);
        else
		m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode,eDirection,isAnsweringToNewCap);
    }
	else
		DBGPASSERT(1);// if we can't open out channels (at least audio) its a mistake.

}
/////////////////////////////////////////////////////////////////////
void CSipTransaction::OnPartySendSiteAndVisualNamePlusProductIdToPartyControl(CSegment* pParam)
{
	SendSiteAndVisualNamePlusProductIdToParty(pParam);
}

/////////////////////////////////////////////////////////////////////
void CSipTransaction::OnPartyConnectToutConnecting(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::OnPartyConnectToutConnecting: Name ",m_pPartyConfName);
	SetDialState(kConnectTimer);
	EndTransaction(SIP_TIMER_POPPED_OUT);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SetCapsAccordingToNewAllocation(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::SetCapsAccordingToNewAllocation: Name ",m_pPartyConfName);
	WORD len = 0;
	BYTE bIsAudioOnly = FALSE;
	BYTE bIsRtv = FALSE;
	DWORD videoRate = 0;
	BYTE cif4Mpi = FALSE;
	BYTE bIsMsSvc= FALSE;

	*pParam >> bIsAudioOnly;
	*pParam >> bIsRtv;
	*pParam >> bIsMsSvc;
	*pParam >> videoRate;
	*pParam >> cif4Mpi;

	if (bIsAudioOnly)
	{
		m_pSipCntl->SetLocalCapToAudioOnly();
	}
	else if (bIsMsSvc == FALSE)
	{
		PTRACE2INT(eLevelInfoNormal,"CSipTransaction::SetCapsAccordingToNewAllocation: videoRate=",videoRate);

		*pParam >> len;
		H264VideoModeDetails h264VidModeDetails;
		pParam->Get((BYTE*)&h264VidModeDetails,len);
		m_pSipCntl->SetVideoParamInCaps(h264VidModeDetails, cif4Mpi,bIsRtv,videoRate);

		if(bIsRtv)
			CheckChangesInVideo();
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipTransaction::SetCapsAccordingToNewAllocation:ms svc videoRate=",videoRate);
		*pParam >> len;
		MsSvcVideoModeDetails MsSvcVidModeDetails;
		pParam->Get((BYTE*)&MsSvcVidModeDetails,len);
		m_pSipCntl->SetMsSvcVideoParamInCaps(MsSvcVidModeDetails, cif4Mpi,bIsRtv,videoRate);
	}
}

//////////////////////////////////////////////////////
void CSipTransaction::CheckChangesInVideo()
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::CheckChangesInVideo : Name - ",m_pPartyConfName);


	if(IsNeedToSendRtcpVideoPreference())
			m_bChangeInVideoFormatOnly = TRUE;
}

////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::GetProductIdAndSendToConfLevel()
{
	BYTE bIsProductId = FALSE;
	BYTE bIsVersionId = FALSE;
	char* pVersionId = NULL;
	eTelePresencePartyType  eLocalTelePresencePartyType = eTelePresencePartyNone;
	ALLOCBUFFER(cUserAgent, MaxUserAgentSize);
	PASSERT_AND_RETURN(NULL == cUserAgent);	// just for KW as in "ALLOCBUFFER there is already copy to buffer

	sipMessageHeaders *pRemoteSipHeaders = m_pSipCntl->GetRmtHeaders();//GetRemoteCallLegHeaders();
	//if value received from header

	if (pRemoteSipHeaders)
	{
		::SipGetHeaderValue(pRemoteSipHeaders, kUserAgent, cUserAgent, MaxUserAgentSize);
		if ( (cUserAgent) && (strlen(cUserAgent) > 0) )
		{
			bIsProductId = TRUE;

			pVersionId = new char[strlen(cUserAgent)+1];
			memset(pVersionId,'\0',strlen(cUserAgent)+1);

			IdentifyVersionId(cUserAgent,&pVersionId,cUserAgent,strlen(cUserAgent));
			if (pVersionId[0] != '\0')
			{
				FPTRACE2(eLevelInfoNormal,"CSipTransaction::GetProductIdAndSendToConfLevel : The VersionId is ",pVersionId );
				bIsVersionId = TRUE;
				if(strstr((const char*)cUserAgent,"RPX"))
				{
					eLocalTelePresencePartyType = eTelePresencePartyRPX;
					PTRACE2(eLevelInfoNormal,"CSipTransaction::GetProductIdAndSendToConfLevel - Identify party as RPX: Name - ",m_pPartyConfName);
				}
				if(strstr((const char*)cUserAgent,"FLEX"))
				{
					eLocalTelePresencePartyType = eTelePresencePartyFlex;
					PTRACE2(eLevelInfoNormal,"CSipTransaction::GetProductIdAndSendToConfLevel - Identify party as FLEX: Name - ",m_pPartyConfName);
				}
			}

			UserAgentAndVersionUpdated(cUserAgent, pVersionId);
		}
	}


	BYTE bIsVisualName = FALSE;

	// we enter the segment only product ID without site name.
	CSegment*  pParam = new CSegment;
	*pParam << (BYTE)FALSE;//is remote cascade to cop mcu in SIP aleways false
	*pParam << bIsVisualName;
	DWORD len = 0;
	*pParam << len;
	*pParam << bIsProductId;
	DWORD len2 = 0;
	if (cUserAgent)
	{
		if (cUserAgent[0] != '\0')
			len2 = strlen(cUserAgent) + 1;//for string used.
	}
	else
	{
		PASSERT_AND_RETURN(1);
	}
	*pParam << len2;
	*pParam << bIsVersionId;
	DWORD len3 = 0;
	if (bIsVersionId)
		len3 = strlen(pVersionId) + 1;//for string used.
	*pParam << len3;
	if(len2)
		pParam->Put((unsigned char*)cUserAgent, len2);
	if(len3)
	{
		pParam->Put((unsigned char*)pVersionId, len3);

	}

	*pParam << (BYTE)eLocalTelePresencePartyType;

	// speakerIndication
	RemoteIdent eRemoteVendorIdent = m_pSipCntl->GetRemoteIdent();
	*pParam << (BYTE)eRemoteVendorIdent;


	APIU16 isGetInfoFromCname = m_pSipCntl->RetriveMaskAndNameFromEpIfPosible(m_pSipCntl->GetRemoteSdp()->cCname,bIsProductId,cUserAgent,bIsVersionId,pVersionId);
	if(!isGetInfoFromCname)
		DispatchEvent(SET_SITE_AND_VISUAL_NAME, pParam);
	else
	{
		PTRACE(eLevelInfoNormal,"CSipTransaction::GetProductIdAndSendToConfLevel - sent info as CName - no need to double send ");
	}
	DEALLOCBUFFER(cUserAgent);
	PDELETEA(pVersionId);
	POBJDELETE(pParam);
}
///////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SetIsNeedToSendReInviteforFullAudioCapsAccordingToUserAgentAndVersion()
{
	BYTE bIsProductId = FALSE;
	BYTE bIsVersionId = FALSE;
	char* pVersionId = NULL;
	ALLOCBUFFER(cUserAgent, MaxUserAgentSize);
	sipMessageHeaders *pRemoteSipHeaders = m_pSipCntl->GetRmtHeaders();//GetRemoteCallLegHeaders();
		//if value received from header

		if (pRemoteSipHeaders)
		{
			::SipGetHeaderValue(pRemoteSipHeaders, kUserAgent, cUserAgent, MaxUserAgentSize);
			if ( (cUserAgent) && (strlen(cUserAgent) > 0) )
			{
				bIsProductId = TRUE;
				pVersionId = new char[strlen(cUserAgent)+1];
				memset(pVersionId,'\0',strlen(cUserAgent)+1);
				IdentifyVersionId(cUserAgent,&pVersionId,cUserAgent,strlen(cUserAgent));
				UserAgentAndVersionUpdated(cUserAgent, pVersionId);
				PDELETEA(pVersionId);
			}
		}
		DEALLOCBUFFER(cUserAgent);



}
////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::CheckContentChangesInConfResponse(BYTE bIsContentSpeaker, CapEnum oldContentProtocol)
{
    if (m_pTargetMode->IsMediaOn(cmCapVideo, cmCapReceive, kRolePresentation))
	{
		DWORD currentContentRate = m_pCurrentMode->GetContentBitRate(cmCapReceive);
		DWORD newContentRate = m_pTargetMode->GetContentBitRate(cmCapReceive);
		CapEnum newContentProtocol = (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePresentation);
		CLargeString str;
		str << "old content protocol - " << oldContentProtocol << ", new content protocol - " << newContentProtocol << "\ncurrent content rate - " << currentContentRate << ", new rate - " << newContentRate << ", Content Speaker - " << bIsContentSpeaker;
		PTRACE2(eLevelInfoNormal,"CSipTransaction::CheckContentChangesInConfResponse : ", str.GetString());

		if (newContentRate != currentContentRate)
		{
			PTRACE2INT(eLevelInfoNormal,"CSipTransaction::CheckContentChangesInConfResponse : new content rate: ", newContentRate);
			m_bContentRateWasChanged = TRUE; // ***ppc for tandberg we can use the 200ok for change rate
		}
		else if (bIsContentSpeaker != m_pSipCntl->GetIsContentSpeaker())
		{
			PTRACE2INT(eLevelInfoNormal,"CSipTransaction::CheckContentChangesInConfResponse : new speaker status: ", bIsContentSpeaker);
			m_pSipCntl->SetIsContentSpeaker(bIsContentSpeaker);
			m_bContentRateWasChanged = TRUE; // ***ppc for tandberg we can use the 200ok for change rate
		}

		if ((oldContentProtocol == eH264CapCode && newContentProtocol == eH263CapCode)
			|| (oldContentProtocol == eH263CapCode && newContentProtocol == eH264CapCode))
		{
			PTRACE2INT(eLevelInfoNormal,"CSipTransaction::CheckContentChangesInConfResponse : new content protocol: ", newContentProtocol);
			m_bContentProtocolWasChanged = TRUE;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::RemoveBfcpAccordingToRemoteIdent()
{
	RemoteIdent remoteIdent = m_pSipCntl->GetRemoteIdent();
	BOOL bEnableSipPPC = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_SIP_PPC_FOR_ALL_USER_AGENT);

	if ((!bEnableSipPPC) && !(remoteIdent == PolycomEp) && !(remoteIdent == PolycomRMX) && !(remoteIdent == AvayaEP))
	{
		PTRACE2INT(eLevelInfoNormal,"CSipTransaction::RemoveBfcpAccordingToRemoteIdent : ENABLE_SIP_PPC_FOR_ALL_USER_AGENT is: ",bEnableSipPPC);

		m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);

		if (m_pTargetModeMaxAllocation)
		{
			m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
			m_pTargetModeMaxAllocation->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
		}
		m_pSipCntl->RemoveBfcpAndContentCaps();
	}
	else
		PTRACE2INT(eLevelInfoNormal,"CSipTransaction::RemoveBfcpAccordingToRemoteIdent : Not removed. Remote Ident = ", remoteIdent);
}

/////////////////////////////////////////////////////
void CSipTransaction::RemoveBFCPIfTransportUnsupported()
{
	sipMediaLineSt *pBFCPMediaLine = NULL;
	BOOL	isANATContained = FALSE;
	sipSdpAndHeadersSt* pRemoteSdp= m_pSipCntl->GetRemoteSdp();

	if (pRemoteSdp)
		isANATContained = IsANATPresentInSDP(pRemoteSdp);
	if(isANATContained)
	{
		//Remove TCP/BFCP for ANAT
		RemoveBfcpIfNecessaryForANAT();
		return;
	}

//NOTE: We can remove the following codes as soon as MediaCard supports IPv6/TCP/BFCP
//------IPv6/TCP/BFCP------	
	if(pRemoteSdp)
		pBFCPMediaLine = GetMediaLine(*pRemoteSdp, kMediaLineInternalTypeBfcp);
	
	if((pBFCPMediaLine)&&(eIpVersion6 == pBFCPMediaLine->mediaIp.transAddr.ipVersion)
		&&(eMediaLineSubTypeTcpBfcp == pBFCPMediaLine->subType))
	{
		if (eMediaLineSubTypeUdpBfcp != m_pSipCntl->GetBfcpType())
		{
			PTRACE(eLevelInfoNormal,"CSipTransaction::RemoveBFCPIfTransportUnsupported : remove.");

			m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
			m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);

			if (m_pTargetModeMaxAllocation)
			{
				m_pTargetModeMaxAllocation->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
				m_pTargetModeMaxAllocation->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
			}
			m_pSipCntl->RemoveBfcpAndContentCaps();
		}
		else
			PTRACE(eLevelInfoNormal,"CSipTransaction::RemoveBFCPIfTransportUnsupported : BFCP traport type is not identical in sipBfcpCntl and remoteSDP!!!!.");
	}
	//------IPv6/TCP/BFCP------ 
	return;
}

//added for ANAT
void CSipTransaction::RemoveBfcpIfNecessaryForANAT()
{
	BOOL isANATContained = FALSE;
	sipSdpAndHeadersSt* pRemoteSdp= m_pSipCntl->GetRemoteSdp();
	
	if (pRemoteSdp)
		isANATContained = IsANATPresentInSDP(pRemoteSdp);

	if (isANATContained && m_pTargetMode->GetBfcpTransportType() != eUnknownTransportType)
	{
		if (eMediaLineSubTypeUdpBfcp != m_pSipCntl->GetBfcpType())
		{
			PTRACE(eLevelInfoNormal,"CSipTransaction::RemoveBfcpIfNecessaryForANAT : remove.");

			m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
			m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);

			if (m_pTargetModeMaxAllocation)
			{
				m_pTargetModeMaxAllocation->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
				m_pTargetModeMaxAllocation->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
			}
			m_pSipCntl->RemoveBfcpAndContentCaps();
		}
		else
			PTRACE(eLevelInfoNormal,"CSipTransaction::RemoveBfcpIfNecessaryForANAT : Not removed.");
	}	
}
// TIP
/////////////////////////////////////////////////////////////////////////////////
/*
BYTE CSipTransaction::CheckIfRemoteSdpIsTipCompatible()
{
	BYTE bIsTipAudio	= FALSE;
	BYTE bIsTipVideo	= FALSE;
	BYTE bIsTipResolution = FALSE;
	BYTE bIsFecc		= FALSE;

	const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();

	if (pCurRemoteCaps->GetIsContainingCapCode(cmCapAudio, eAAC_LDCapCode))
		bIsTipAudio = TRUE;

//	if (pCurRemoteCaps->GetH264ProfileFromCapCode(cmCapVideo) == H264_Profile_Main)
//		bIsTipVideo = TRUE;

	if (pCurRemoteCaps->GetIsTipResolution())
		bIsTipResolution = TRUE;

	if ((pCurRemoteCaps->GetIsContainingCapCode(cmCapData, eAnnexQCapCode)) ||
		(pCurRemoteCaps->GetIsContainingCapCode(cmCapData, eAnnexQCapCode)))
		bIsFecc = TRUE;//noa temp

	CLargeString str;
	str << " bIsTipAudio: " << bIsTipAudio <<  ", bIsTipResolution: " << bIsTipResolution << ", bIsFecc: " << bIsFecc;
	PTRACE2(eLevelInfoNormal, "CSipTransaction::CheckIfRemoteSdpIsTipCompatible: ", str.GetString());

//	if (!bIsTipAudio || !bIsTipVideo || !bIsTipResolution || bIsFecc)
	if (!bIsTipAudio || !bIsTipResolution || bIsFecc)
		return FALSE;

	return TRUE;
}
*/

BYTE CSipTransaction::CheckIfRemoteVideoRateIsTipCompatible()
{
	const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
	long videoRate = pCurRemoteCaps->GetVideoRate();

	PTRACE2INT(eLevelInfoNormal,"CSipTransaction::CheckIfRemoteVideoRateIsTipCompatible Video Rate: ",videoRate);

	if (videoRate >= 9360)
		return TRUE;

	PTRACE2INT(eLevelInfoNormal, "CSipTransaction::CheckIfRemoteVideoRateIsTipCompatible - video rate: ", videoRate);

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CSipTransaction::IsNeedToSendRtcpVideoPreference()
{
	PTRACE2(eLevelInfoNormal,"CSipTransaction::IsNeedToSendRtcpVideoPreference : Name - ",m_pPartyConfName);

	COstrStream msg1;
	m_pCurrentMode->Dump(msg1);
	PTRACE2(eLevelInfoNormal,"CSipTransaction::IsNeedToSendRtcpVideoPreference, m_pCurrentMode : ", msg1.str().c_str());

	COstrStream msg2;
	m_pTargetMode->Dump(msg2);
	PTRACE2(eLevelInfoNormal,"CSipTransaction::IsNeedToSendRtcpVideoPreference, m_pTargetMode : ", msg2.str().c_str());

	if (m_pCurrentMode->IsMediaOn(cmCapVideo,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapReceive))
	{
		CCapSetInfo TargetCapInfo	= (CapEnum)m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive);
		CCapSetInfo CurrenttCapInfo	= (CapEnum)m_pCurrentMode->GetMediaType(cmCapVideo, cmCapReceive);

		if  (TargetCapInfo == CurrenttCapInfo && (CapEnum)TargetCapInfo == eRtvCapCode)
		{
			if ( m_pSipCntl->GetRemoteIdent() == MicrosoftEP_R1 ||
			     m_pSipCntl->GetRemoteIdent() == MicrosoftEP_R2 ||
		   	     m_pSipCntl->GetRemoteIdent() == MicrosoftEP_Lync_R1 || 
			     m_pSipCntl->GetRemoteIdent() == MicrosoftEP_Lync_2013 ||
			     m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU ||
			     m_pSipCntl->GetRemoteIdent() == Microsoft_AV_MCU2013)
				return TRUE;
		}
	}

	return FALSE;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::IsNeedReinviteForRemoveRtv()
{
	return m_bNeedReinviteForRemoveRtv;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::RemoveRtvCapsIfNeeded(CIpComMode* pScmMode)
{
	PTRACE(eLevelInfoNormal,"CSipTransaction::RemoveRtvCapsIfNeeded");

	BYTE res = FALSE;
	const CSipCaps* pLocalCaps = m_pSipCntl->GetLocalCaps();

	if ((CapEnum)pScmMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople) != eRtvCapCode
			&& pLocalCaps && pLocalCaps->IsCapSet(eRtvCapCode))
	{
		if(!(m_pSipCntl->IsRemoteMicrosoft()))
		{
			PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::RemoveRtvCapsIfNeeded - Need to remove");
			m_pSipCntl->RemoveCapSet(eRtvCapCode);
			res = TRUE;
		}

	}
	return res;

}

/////////////////////////////////////////////////////////////////////////////
//this function is for cop only
DWORD CSipTransaction::CalcualteNewRateAccordingToBestModeForCOP( DWORD vidRateBefore ,CIpComMode*  pBestMode)
{
	if(pBestMode->IsMediaOn(cmCapVideo,cmCapReceive) )
	{
		const CVidModeH323 targetmedia = ((const CVidModeH323 &)pBestMode->GetMediaMode(cmCapVideo,cmCapReceive,kRolePeople));
		if(targetmedia.GetType() == eH261CapCode && vidRateBefore > COP_MAX_RATE_FOR_CIF_RESOLUTION)
			return COP_MAX_RATE_FOR_CIF_RESOLUTION;
		else
		{
			if(targetmedia.GetType() == eH263CapCode)
			{
				if(targetmedia.GetFormatMpi(k4Cif) > 0 && vidRateBefore > COP_MAX_RATE_FOR_SD_RESOLUTION )
					return COP_MAX_RATE_FOR_SD_RESOLUTION;
				else if(vidRateBefore > COP_MAX_RATE_FOR_CIF_RESOLUTION)
					return COP_MAX_RATE_FOR_CIF_RESOLUTION;
			}
			if(targetmedia.GetType() == eH264CapCode)
			{
				APIU16 profile = 0;
				APIU8 level=0;
				long mbps=0, fs=0, dpb=0, brAndCpb=0, sar=0, staticMB=0;
				targetmedia.GetH264Scm(profile,level,mbps,fs,dpb,brAndCpb,sar,staticMB);
				if( fs == (long)INVALID )
			    {
			            CH264Details thisH264Details = level;
			            fs = thisH264Details.GetDefaultFsAsDevision();
			    }
				if(fs <= H264_CIF_FS_AS_DEVISION && vidRateBefore > COP_MAX_RATE_FOR_CIF_RESOLUTION)
					return COP_MAX_RATE_FOR_CIF_RESOLUTION;
				if(fs <= H264_WCIF60_FS_AS_DEVISION && vidRateBefore > COP_MAX_RATE_FOR_SD_RESOLUTION)
					return COP_MAX_RATE_FOR_SD_RESOLUTION;
			}

		}



	}
	return vidRateBefore;
}
/////////////////////////////////////////////////////////////////////////////////
//              Messages from SipTransaction to SipParty			 	       //
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SendSiteAndVisualNamePlusProductIdToParty(CSegment* pParam)
{
	CSegment*  pSeg = new CSegment(*pParam);
	SendMessageToParty(SIP_TRANS_SITE_VISUAL_NAME, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendOriginalRemoteCapsToParty(CSipCaps * pRemoteCaps)
{
	CSegment * pSeg = new CSegment;
	pRemoteCaps->Serialize(NATIVE,*pSeg);
	SendMessageToParty(SIP_TRANS_ORIGINAL_RMOTCAP, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendRemoteConnectedToParty()
{
	CSegment * pSeg = new CSegment;
	SendMessageToParty(SIP_TRANS_REMOTE_CONNECTED, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendChannelHandleToParty()
{
	SendMessageToParty(SIP_TRANS_SEND_CHANNEL_HANDLE, NULL);
}
/////////////////////////////////////////////////////////////////////////////
//void  CSipTransaction::SendMuteMediaToParty(BYTE audioInMute, BYTE audioOutMute, BYTE VideoInMute, BYTE VideoOutMute, BYTE ContentInMute, BYTE ContentOutMute, BYTE FeccInMute, BYTE FeccOutMute)
void  CSipTransaction::SendMuteMediaToParty(cmCapDirection eDirection)
{
	// NO - means unmute
	// YES - means mute.
	// AUTO - means unchanged
	CSegment * pSeg = new CSegment;
//	*pSeg 	<< audioInMute
//			<< audioOutMute
//			<< VideoInMute
//			<< VideoOutMute
//			<< ContentInMute
//			<< ContentOutMute
//			<< FeccInMute
//			<< FeccOutMute;
	DWORD dir = (DWORD) eDirection;
	*pSeg << dir;

	SendMessageToParty(SIP_TRANS_MUTE_MEDIA, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendReCapsReceivedToParty(CSipCaps* pCaps,CIpComMode * pBestMode,WORD isFallBckFromTip, WORD bIsGlare)//BRIDGE-12961 bIsGlare
{
	CSegment * pSeg = new CSegment;

	pCaps->Serialize(NATIVE,*pSeg);
	pBestMode->Serialize(NATIVE,*pSeg);
	*pSeg << (WORD)isFallBckFromTip;
	*pSeg << (WORD)bIsGlare; //BRIDGE-12961

	SendMessageToParty(SIP_TRANS_PARTY_RECEIVE_RECAP, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SendUpdateDbToParty(DWORD partyState)
{
	CSegment * pSeg = new CSegment;
	*pSeg << (DWORD)partyState;

	SendMessageToParty(SIP_TRANS_UPDATE_DB, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SendDisconnectBridgesToParty(WORD isDisconnectAudio, WORD isDisconnectVideo)
{
	CSegment * pSeg = new CSegment;
	*pSeg << (WORD)isDisconnectAudio
		  << (WORD)isDisconnectVideo;

	SendMessageToParty(SIP_TRANS_DISCONNECT_BRIDGES, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SendConnectBridgesToParty(WORD isConnectAudio, WORD isConnectVideo, unsigned int* channelHandle)
{
	CSegment * pSeg = new CSegment;
	*pSeg << (WORD)isConnectAudio
		  << (WORD)isConnectVideo;

	m_pTargetMode->Serialize(NATIVE, *pSeg);

	CSipCaps* pCurRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
	pCurRemoteCaps->Serialize(NATIVE, *pSeg);

	*pSeg << channelHandle[0];
	*pSeg << channelHandle[1];

	SendMessageToParty(SIP_TRANS_CONNECT_BRIDGES, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendVideoChangeResolutionToParty(DWORD Width,DWORD Height,DWORD FrameRate,DWORD BitRate)
{
	CSegment * pSeg = new CSegment;

	*pSeg 	<< Width
			<< Height
			<< FrameRate
			<< BitRate;

	SendMessageToParty(SIP_TRANS_PARTY_CHANGE_VIDEO_RES, pSeg);
}
/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendStartVideoPreferenceToParty()
{
	CSegment * pSeg = new CSegment;
	SendMessageToParty(START_VIDEO_PREFERENCE, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendUpdateDbChannelsStatusToParty(BYTE bIsConnected)
{
	CSegment * pSeg = new CSegment;
	*pSeg << bIsConnected;
	SendMessageToParty(SIP_TRANS_UPDATE_CHANS_STATUS, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendChannelsConnectedToParty()
{
	CSegment * pSeg = new CSegment;
	PTRACE(eLevelInfoNormal,"CSipTransaction::SendChannelsConnectedToParty");
	SendMessageToParty(SIP_TRANS_CHANS_CONNECTED, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendRemoteCapsReceivedToParty()
{
	CSegment * pSeg = new CSegment;
	SendMessageToParty(SIP_TRANS_REMOTE_CAPS_RECEIVED, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendEndTransactionToParty(DWORD retStatus)
{
	CSegment * pSeg = new CSegment;
	*pSeg << (DWORD)retStatus;
	SendMessageToParty(SIP_TRANS_END_TRANSACTION, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendUpdateDbEncryptionStatusToParty(WORD bIsEncrypted)
{
	CSegment * pSeg = new CSegment;
	*pSeg << (WORD)bIsEncrypted;
	SendMessageToParty(SIP_TRANS_UPDATE_ENC_STATUS, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendHandleIceConnectivityCheckCompleteToParty(CSegment* pParam)
{
	CSegment*  pSeg = new CSegment(*pParam);
	SendMessageToParty(SIP_TRANS_HANDLE_CONNECTIVITY_CHECK, pSeg);
}

/////////////////////////////////////////////////////////////////////////////
BYTE  CSipTransaction::GetIsMuteForTip()
{
	return m_bIsTipMute;
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SetIsMuteForTip(BYTE bIsMute)
{
	m_bIsTipMute = bIsMute;
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SetIsNeedToWaitForSlavesEndChangeMode(BYTE bIsNeedToWait)
{
	m_needToWaitForSlavesEndChangeMode = bIsNeedToWait;
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransaction::SetIsTipResumeMedia(BYTE bIsResume)
{
	m_bIsResumeMedia = bIsResume;
}

/////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::GetIsTipResumeMedia() const
{
	return m_bIsResumeMedia;
}
/////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::IsNeedToReInviteToFallbackFromIceToSip() const
{
	return m_bIsNeedToFallbackFromIceToSip;
}
/////////////////////////////////////////////////////////////////////////////
void CSipTransaction::UpdateIfCallIsResumed()
{
	m_bIsResumeMedia = m_pSipCntl->CheckIfCallIsResumed(m_pChanDifArr);

	PTRACE2INT(eLevelInfoNormal, "CSipTransaction::UpdateIfCallIsResumed, m_bIsResumeMedia: ", m_bIsResumeMedia);
}
/////////////////////////////////////////////////////////////////////////////
//N.A.
BYTE CSipTransaction::SetMediaSdesChangesIfNeeded(CSipComMode* pBestMode,BYTE bChangeInMediaIn )
{
	BYTE bChangeInMedia = bChangeInMediaIn;

	if (m_pTargetMode->IsSdesMediaEquals(*pBestMode, cmCapAudio, cmCapReceive) == FALSE)
	{
		CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapAudio,cmCapReceive, kRolePeople);
		m_pTargetMode->SetSipSdes(cmCapAudio,cmCapReceive,kRolePeople,pSdesCap);
		m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, cmCapAudio, kRolePeople);
		bChangeInMedia = YES;
	}

	if (m_pTargetMode->IsSdesMediaEquals(*pBestMode, cmCapVideo, cmCapReceive) == FALSE)
	{
		CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapVideo,cmCapReceive, kRolePeople);
		m_pTargetMode->SetSipSdes(cmCapVideo,cmCapReceive,kRolePeople,pSdesCap);
		m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, cmCapVideo, kRolePeople);
		bChangeInMedia = YES;
	}

	if (m_pTargetMode->IsSdesMediaEquals(*pBestMode, cmCapData, cmCapReceive) == FALSE)
	{
		CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapData,cmCapReceive, kRolePeople);
		m_pTargetMode->SetSipSdes(cmCapData,cmCapReceive,kRolePeople,pSdesCap);
		m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, cmCapAudio, kRolePeople);
		bChangeInMedia = YES;
	}

	if (m_pTargetMode->IsSdesMediaEquals(*pBestMode, cmCapVideo, cmCapReceive, kRolePresentation) == FALSE)
	{
		CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapVideo,cmCapReceive, kRolePresentation);
		m_pTargetMode->SetSipSdes(cmCapVideo,cmCapReceive,kRolePresentation,pSdesCap);
		m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, cmCapVideo, kRolePresentation);
		bChangeInMedia = YES;
	}

	return bChangeInMedia;

}

/////////////////////////////////////////////////////////////////////////////
BOOL CSipTransaction::IsCallResumed()
{
	BOOL IsResume = FALSE;

	CSipChanDif* pAudOutChanDif = m_pChanDifArr->GetChanDif(cmCapAudio, cmCapTransmit);
	CSipChanDif* pAudInChanDif  = m_pChanDifArr->GetChanDif(cmCapAudio, cmCapReceive);

	CSipChanDif* pVidOutChanDif = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapTransmit);
	CSipChanDif* pVidInChanDif  = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapReceive);

	if ( (pAudOutChanDif && (pAudOutChanDif->IsUnMute())) ||//&& pAudInChanDif->IsUnMute())
		 (pVidOutChanDif && (pVidOutChanDif->IsUnMute()) ) )//pVidInChanDif->IsUnMute()))
	{
		IsResume = TRUE;
	}

	TRACEINTO << "CSipTransaction::IsCallResumed, IsResumeMedia: " <<  (IsResume ? "YES" : "NO");
	return IsResume;
}
/////////////////////////////////////////////////////////////////////////////
BOOL CSipTransaction::IsCallHold()
{
	BOOL IsHold = FALSE;

	CSipChanDif* pAudOutChanDif = m_pChanDifArr->GetChanDif(cmCapAudio, cmCapTransmit);
	CSipChanDif* pAudInChanDif  = m_pChanDifArr->GetChanDif(cmCapAudio, cmCapReceive);

	CSipChanDif* pVidOutChanDif = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapTransmit);
	CSipChanDif* pVidInChanDif  = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapReceive);

	if ( (pAudOutChanDif && (pAudOutChanDif->IsMute())) ||//&& pAudInChanDif->IsUnMute())
		 (pVidOutChanDif && (pVidOutChanDif->IsMute()) ) )//pVidInChanDif->IsUnMute()))
	{
		IsHold = TRUE;
	}

	TRACEINTO << "CSipTransaction::IsCallHold, IsHoldMedia: " <<  (IsHold ? "YES" : "NO");
	return IsHold;
}
/////////////////////////////////////////////////////////////////////////////
//          End of Messages from SipTransaction to SipParty                //
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::CheckIfNeedToFixContentAlgAndSendReInvite(const CSipCaps *pLocalCaps)//, CSipCaps *pRemoteCaps)
{
	int sentContentCapsCount = m_pSipCntl->GetContentCapsCountInSentSdp();

    PTRACE2INT(eLevelInfoNormal, "CSipTransaction::CheckIfNeedToFixContentAlgAndSendReInvite - last sent SDP contained content caps count=", sentContentCapsCount);

	m_pSipCntl->DeclareOnContentFromScmOnly(FALSE);

	if (sentContentCapsCount > 1)
	{
		m_bNeedReInviteToFixContentAlg = TRUE;

		m_pSipCntl->DeclareOnContentFromScmOnly(TRUE);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::FallbackFromTipToNoneTip()
{
	PTRACE(eLevelInfoNormal, "CSipTransaction::FallbackFromTipToNoneTip");

	m_pTargetMode->SetTipMode(eTipModeNone);
	m_pTargetModeMaxAllocation->SetTipMode(eTipModeNone);

	m_pParty->SetTipPartyTypeAndPosition(eTipNone);

	m_pTargetMode->SetDtlsEncryption(Encryp_Off);
	m_pTargetModeMaxAllocation->SetDtlsEncryption(Encryp_Off);

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	if (pCommConf)
	{
	    CConfParty* pConfParty 	= pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
	    if (pConfParty)
	    {
	        pConfParty->SetIsTipCall(FALSE);
	        pConfParty->SetTIPPartyType(eTipPartyNone);
	    }
	    else
	        PTRACE(eLevelInfoNormal, "CSipTransaction::FallbackFromTipToNoneTip - pConfParty is NULL");
	}
	else{
	    PTRACE(eLevelInfoNormal, "CSipTransaction::FallbackFromTipToNoneTip - pCommConf is NULL");
	    PASSERT_AND_RETURN(1);
	}


	if( m_pParty->GetIsTipNegotiationActive() )
	{
		m_pSipCntl->EndTipNegotiation(eTipNegError);

		m_pParty->CloseTipSessionAndSendMuxDisconnect();

		m_pParty->SetIsTipNegotiationActive(FALSE);

		//m_pParty->StartIvr();
	}
	else
	{
		if(m_pTargetMode->IsTipNegotiated())
			m_pParty->CloseTipSessionAndSendMuxDisconnect();
	}

	if (m_pTargetMode->IsMediaOff(cmCapBfcp, cmCapReceiveAndTransmit))
		m_pSipCntl->AddBfcpOnFallback(m_pTargetMode, m_pTargetModeMaxAllocation);

	BOOL bIsNeedToAddContent = IsNeedToAddContentForFallbackFromTip();

	if (m_pTargetMode->IsMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation) && bIsNeedToAddContent)
	{
		PTRACE(eLevelInfoNormal, "CSipTransaction::FallbackFromTipToNoneTip add content");

		m_pTargetMode->SetTipAuxFPS(eTipAuxNone);
		m_pTargetModeMaxAllocation->SetTipAuxFPS(eTipAuxNone);

		if (pCommConf && pCommConf->GetIsPreferTIP())
		{
			m_pTargetMode->SetTIPContent(0,cmCapReceiveAndTransmit,FALSE);
			m_pTargetModeMaxAllocation->SetTIPContent(0,cmCapReceiveAndTransmit,FALSE);
		}
		else //eTipCompatibleVideoAndContent
		{
			m_pTargetMode->SetTIPContent(0,cmCapReceiveAndTransmit);
			m_pTargetModeMaxAllocation->SetTIPContent(0,cmCapReceiveAndTransmit);
		}

		m_pSipCntl->SetContentCap(m_pTargetMode);
		if( m_pSipCntl->GetFullContentRate() == 0 && pCommConf->GetIsTipCompatibleContent() )
		{
			m_pSipCntl->SetFullContentRate(5120);//tip is laways 512k
			m_pParty->SetPartyContentRate(5120);
			m_pTargetModeMaxAllocation->SetVideoBitRate(5120, cmCapReceive, kRoleContentOrPresentation);
			m_pTargetModeMaxAllocation->SetVideoBitRate(5120, cmCapTransmit, kRoleContentOrPresentation);
		    PTRACE2INT(eLevelInfoNormal, "CSipTransaction::FallbackFromTipToNoneTip - m_pParty->GetPartyContentRate()", m_pParty->GetPartyContentRate());
			m_pSipCntl->SetLocalSdesKeysAndTagByHost(m_pTargetMode, m_pTargetModeMaxAllocation, cmCapVideo, kRolePresentation);
			m_pSipCntl->AddContentCapIfNeeded(m_pTargetMode, eH264CapCode);
			m_pTargetMode->Dump("CSipTransaction::FallbackFromTipToNoneTip - m_pTargetMode", eLevelError);
			CSuperLargeString msgPM4;
			m_pSipCntl->GetLocalCaps()->DumpToString(msgPM4);
		}
		else
		{
		    PTRACE(eLevelInfoNormal, "CSipTransaction::FallbackFromTipToNoneTip -m_pSipCntl->GetFullContentRate() != 0 ");
		}
	}
	m_pParty->SetNeedToStartIVRAfterFallbackFromTip(TRUE);


}

////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::BfcpDecisionCenter(CSipComMode* pBestMode)
{
	eSipBfcpMode4DialOut 	localBfcpMode 	= m_pSipCntl->GetBfcpMode4DialOut();
	eMediaLineSubType 		remoteBfcpType 	= m_pSipCntl->GetBfcpType();
	BYTE					bIsContentOn	= pBestMode->IsContent(cmCapReceiveAndTransmit);

	const CSipCaps* 		pLocalCaps 		= m_pSipCntl->GetLocalCaps();

	CLargeString str;

	str << "\n local bfcp mode:			" 	<< localBfcpMode
		<< "\n remote bfcp Type:			" 	<< remoteBfcpType
		<< "\n bfcp transport type in target mode:	" << m_pTargetMode->GetBfcpTransportType()
		<< "\n is bfcp in best mode:		" 	<< pBestMode->IsMediaOn(cmCapBfcp, cmCapReceiveAndTransmit)
		<< "\n is content in best mode:	" 	<< pBestMode->IsContent(cmCapReceiveAndTransmit)
		<< "\n is bfcp in target mode:	" 		<< m_pTargetMode->IsMediaOn(cmCapBfcp, cmCapReceiveAndTransmit)
		<< "\n is content in target mode:" 	<< m_pTargetMode->IsContent(cmCapReceiveAndTransmit)
		<< "\n is bfcp in caps:			" 	<< pLocalCaps->IsMedia(cmCapBfcp)
		<< "\n is content in caps:		" 		<< pLocalCaps->IsMedia(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation);

	PTRACE2(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter :",str.GetString());

	if (m_pParty->GetIsTipCall())
	{
		PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - TIP call, return without do nothing");
		return;
	}

	if (!pLocalCaps->IsMedia(cmCapBfcp))
	{
		PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - no bfcp in local caps, return without do nothing");
		return;
	}

//	if (m_bIsCloseBfcpChannels)
//	{
//		PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - need to close BFCP channels, return without do nothing");
//		return;
//	}

	switch (remoteBfcpType)
	{
		case eMediaLineSubTypeUdpBfcp:

			if (localBfcpMode == BFCP_FULL_FLOW || localBfcpMode == BFCP_UDP_ONLY)
			{
				if ((!bIsContentOn)&& !(m_pSipCntl->IsASSIPContentDisabledinASSIPConf()))
				{

					m_bNeedReInviteForAddContent = TRUE;
					PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter -  udp/bfcp need reinvite for content");
				}

			}
			else //if (localBfcpMode == BFCP_TCP_ONLY)
			{
				if (pLocalCaps->IsMedia(cmCapBfcp) || pLocalCaps->IsMedia(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation))
				{
					m_bNeedReInviteForSecondaryContent = TRUE;
					PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - remote doesn't support tcp, need reinvite to close bfcp and content");
				}
			}
			break;

		case eMediaLineSubTypeTcpBfcp:

			if (localBfcpMode == BFCP_FULL_FLOW)
			{
				if (m_pTargetMode->GetBfcpTransportType() == eUnknownTransportType)
				{
					m_bNeedReInviteForBfcp = TRUE;
					PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - need reinvite for tcp bfcp");
				}
				else if (m_pTargetMode->GetBfcpTransportType() == eTransportTypeTcp)
				{

					if (!bIsContentOn)
					{
						m_bNeedReInviteForAddContent = TRUE;
						PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - 1. tcp/bfcp need reinvite for content");
					}
				}
			}
			else if (localBfcpMode == BFCP_UDP_ONLY)
			{
				if (pLocalCaps->IsMedia(cmCapBfcp) || pLocalCaps->IsMedia(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation))
				{
					m_bNeedReInviteForSecondaryContent = TRUE;
					PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - remote doesn't support udp, need reinvite to close bfcp and content");
				}
			}
			else //if (localBfcpMode == BFCP_TCP_ONLY)
			{
				if (!bIsContentOn)
				{
					m_bNeedReInviteForAddContent = TRUE;
					PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - 2. tcp/bfcp need reinvite for content");
				}
			}
			break;

		case eMediaLineSubTypeNull:

			if (localBfcpMode == BFCP_FULL_FLOW)
			{
				if (m_pTargetMode->GetBfcpTransportType() == eTransportTypeUdp)
				{
					m_bNeedReInviteForBfcp = TRUE;
					PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - remote doesn't support udp - need reinvite for tcp/bfcp");
				}
				else
				{
					if (pLocalCaps->IsMedia(cmCapBfcp) || pLocalCaps->IsMedia(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation))
					{
						m_bNeedToCloseBfcpAndContentWithoutReinvite = TRUE;
						PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - remote doesn't support tcp - need to close bfcp and content");
					}
				}
			}
			else if (localBfcpMode == BFCP_UDP_ONLY || (localBfcpMode == BFCP_TCP_ONLY))
			{
				if (pLocalCaps->IsMedia(cmCapBfcp) || pLocalCaps->IsMedia(cmCapVideo, cmCapReceiveAndTransmit, kRoleContentOrPresentation))
				{
					m_bNeedToCloseBfcpAndContentWithoutReinvite = TRUE;
					PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - remote doesn't support tcp or udp, need to close bfcp and content");
				}
			}
			break;

		default:
			PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - close bfcp, remote doesn't support bfcp");
			break;
	}

	//Added by Efi
	if (m_bNeedReInviteForAddContent && !m_isContentSuggested)
	{
		m_isContentSuggested 				= TRUE;
	}
	else if (m_bNeedReInviteForAddContent && m_isContentSuggested)
	{
		m_bNeedReInviteForAddContent 		= FALSE;
		m_bNeedReInviteForSecondaryContent 	= TRUE;
		PTRACE(eLevelInfoNormal,"CSipTransaction::BfcpDecisionCenter - Content has been suggested before, need reinvite to close bfcp and content");
		//m_isContentSuggested 				= FALSE; // BRIDGE-10590 To stop sending a new reinvite that wasn't needed after received port 0 from EP!!!
	}

}

void CSipTransaction::OnPartyVideoArtDisconnectedDoNothing()
{
  TRACEINTO<<"!@# got internal video art disconnection event while party is disconnecting";
}
////////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::IsNeedReInviteForAddContent()
{
	if(m_pSipCntl->IsASSIPContentDisabledinASSIPConf())
	{
		PTRACE(eLevelInfoNormal,"CSipTransaction::IsNeedReInviteForAddContent - FALSE");
		return FALSE;
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipTransaction::IsNeedReInviteForAddContent - m_bNeedReInviteForAddContent:",m_bNeedReInviteForAddContent);
		return m_bNeedReInviteForAddContent;
	}
}

////////////////////////////////////////////////////////////////////
void CSipTransaction::RestoreStreamGroups(CIpComMode* pComMode)
{
	RestoreSendStreamGroup(pComMode);
	RestoreRecvStreamGroup(pComMode);
}

////////////////////////////////////////////////////////////////////
void CSipTransaction::RestoreSendStreamGroup(CIpComMode* pComMode)
{
	int i=0;
	std::list <StreamDesc> streamsDescList;
	std::list <StreamDesc>::iterator itr_streams;
	STREAM_GROUP_S SendStreamGroup;

	// ===== video
	streamsDescList = pComMode->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);
	memset(&SendStreamGroup, 0, sizeof(STREAM_GROUP_S));
	SendStreamGroup.streamGroupId = 0;
	for (itr_streams = streamsDescList.begin(); itr_streams != streamsDescList.end(); itr_streams++)
	{
		SendStreamGroup.streams[i].streamSsrcId = itr_streams->m_pipeIdSsrc;
		SendStreamGroup.streams[i].frameWidth = itr_streams->m_width;
		SendStreamGroup.streams[i].frameHeight = itr_streams->m_height;
		SendStreamGroup.streams[i].maxFrameRate =itr_streams->m_frameRate;

		i++;
	}
	SendStreamGroup.numberOfStreams = streamsDescList.size();
	pComMode->SetStreamsGroup(SendStreamGroup, cmCapVideo, cmCapTransmit);


	// ===== audio
	i=0;
	streamsDescList = pComMode->GetStreamsListForMediaMode(cmCapAudio, cmCapTransmit, kRolePeople);
	memset(&SendStreamGroup, 0, sizeof(STREAM_GROUP_S));
	for (itr_streams = streamsDescList.begin(); itr_streams != streamsDescList.end(); itr_streams++)
	{
		SendStreamGroup.streams[i].streamSsrcId = itr_streams->m_pipeIdSsrc;
		i++;
	}
	SendStreamGroup.numberOfStreams = streamsDescList.size();
	pComMode->SetStreamsGroup(SendStreamGroup, cmCapAudio, cmCapTransmit);
}

////////////////////////////////////////////////////////////////////
void CSipTransaction::RestoreRecvStreamGroup(CIpComMode* pComMode)
{
	int i=0;
	std::list <StreamDesc> streamsDescList;
	std::list <StreamDesc>::iterator itr_streams;
	STREAM_GROUP_S RecvStreamGroup;


	// ===== video
	streamsDescList = pComMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople);
	memset(&RecvStreamGroup, 0, sizeof(STREAM_GROUP_S));
	RecvStreamGroup.streamGroupId = 0;
	for (itr_streams = streamsDescList.begin();itr_streams != streamsDescList.end();itr_streams++)
	{
		RecvStreamGroup.streams[i].streamSsrcId = itr_streams->m_pipeIdSsrc;
		RecvStreamGroup.streams[i].frameWidth = itr_streams->m_width;
		RecvStreamGroup.streams[i].frameHeight = itr_streams->m_height;
		RecvStreamGroup.streams[i].maxFrameRate =itr_streams->m_frameRate;

		i++;
	}
	RecvStreamGroup.numberOfStreams = streamsDescList.size();
	pComMode->SetStreamsGroup(RecvStreamGroup, cmCapVideo, cmCapReceive);


	// ===== audio
	i=0;
	streamsDescList = pComMode->GetStreamsListForMediaMode(cmCapAudio, cmCapReceive, kRolePeople);
	memset(&RecvStreamGroup, 0, sizeof(STREAM_GROUP_S));
	for (itr_streams = streamsDescList.begin();itr_streams != streamsDescList.end();itr_streams++)
	{
		RecvStreamGroup.streams[i].streamSsrcId = itr_streams->m_pipeIdSsrc;
		i++;
	}
	RecvStreamGroup.numberOfStreams = streamsDescList.size();
	pComMode->SetStreamsGroup(RecvStreamGroup, cmCapAudio, cmCapReceive);
}

////////////////////////////////////////////////////////////////////
BOOL CSipTransaction::GetIsRemoteIdentMicrosoft()
{
	if(!m_pSipCntl)
	{
		DBGPASSERT(YES);
		TRACEINTO << "Error: m_pSipCntl == NULL";
		return false;
	}

	RemoteIdent remoteIdent 	 =	m_pSipCntl->GetRemoteIdent();
	BOOL 		bIsMSremoteIdent =  FALSE;

	bIsMSremoteIdent 		=  		(remoteIdent == MicrosoftEP_R1 || remoteIdent == MicrosoftEP_R2 || remoteIdent == MicrosoftEP_Lync_R1 ||
									 remoteIdent == MicrosoftEP_MAC || remoteIdent == MicrosoftEP_MAC_Lync ||
									 remoteIdent == MicrosoftMediationServer || remoteIdent == Microsoft_AV_MCU || remoteIdent == MicrosoftEP_Lync_CCS);
	return bIsMSremoteIdent;
}
////////////////////////////////////////////////////////////////////////////////
BYTE CSipTransaction::RemoveEncryptionFromScmAndCapsWhenTipResumedIfNeeded(CSipComMode* pBestMode)
{
	BYTE bIsRemoved = FALSE;

	// AN debug
	if (pBestMode && m_pParty->GetIsTipCall() && m_bIsResumeMedia) {

		PTRACE(eLevelInfoNormal,"CSipTransaction::RemoveEncryptionFromScmAndCapsWhenTipResumedIfNeeded:resuming with encryption");
		cmCapDataType mediaType;
		ERoleLabel eRole;
		CSdesCap *pSdesCap = NULL;
		CSdesCap *pSdesCapaccordingtoMax = NULL;

		for (int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
		{
			GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

			if (mediaType == cmCapBfcp)
				continue;

			if (pBestMode->IsMediaOn(mediaType,cmCapTransmit, eRole))
			{
				pSdesCap = NULL;
				pSdesCap =  pBestMode->GetSipSdes(mediaType,cmCapTransmit, eRole);
				pSdesCapaccordingtoMax = m_pTargetModeMaxAllocation->GetSipSdes(mediaType,cmCapTransmit, eRole);

				if (!pSdesCap && pSdesCapaccordingtoMax != NULL)
				{
					//remove unchosen SDES from local caps
					PTRACE2INT(eLevelInfoNormal,"CSipTransaction::RemoveEncryptionFromScmAndCapsWhenTipResumedIfNeeded: remove sdes from local cap mediaType:",mediaType);

					m_pSipCntl->RemoveSdesCapFromLocalCaps(mediaType, eRole);
					m_pTargetModeMaxAllocation->RemoveSipSdes(mediaType,cmCapTransmit,kRolePeople);
					m_pTargetModeMaxAllocation->RemoveSipSdes(mediaType,cmCapReceive,kRolePeople);

					CSipChanDif* pChanDifForOut = m_pChanDifArr->GetChanDif(mediaType, cmCapTransmit);
					CSipChannel *pChannelOut = m_pSipCntl->GetChannel(mediaType, cmCapTransmit);

					if ( pChannelOut && pChannelOut->IsChannelSdesEnabled() && pChanDifForOut)
					{
						PTRACE(eLevelInfoNormal,"CSipTransaction::RemoveEncryptionFromScmAndCapsWhenTipResumedIfNeeded:resuming change sdes for tx also");
						pChanDifForOut->SetChangeSdes(YES);
					}
				}

				bIsRemoved = TRUE;

				pBestMode->RemoveSipDtls(mediaType,cmCapTransmit,kRolePeople);
				pBestMode->RemoveSipDtls(mediaType,cmCapReceive,kRolePeople);
			}
		}

		pSdesCap = NULL;
		pSdesCapaccordingtoMax = NULL;
		pSdesCap =  pBestMode->GetSipSdes(cmCapVideo,cmCapTransmit, kRolePeople);

		if(pSdesCap == NULL && pBestMode->IsMediaOn(cmCapVideo,cmCapTransmit, kRolePresentation))
		{
			PTRACE(eLevelInfoNormal,"CSipTransaction::RemoveEncryptionFromScmAndCapsWhenTipResumedIfNeeded:remove SDES content caps on TIP");

			m_pSipCntl->RemoveSdesCapFromLocalCaps(cmCapVideo, kRolePresentation);
			m_pTargetModeMaxAllocation->RemoveSipSdes(cmCapVideo,cmCapTransmit,kRolePresentation);
			m_pTargetModeMaxAllocation->RemoveSipSdes(cmCapVideo,cmCapReceive,kRolePresentation);

			pBestMode->RemoveSipSdes(cmCapVideo,cmCapReceive,kRolePresentation);
			pBestMode->RemoveSipSdes(cmCapVideo,cmCapTransmit,kRolePresentation);
			pBestMode->RemoveSipDtls(cmCapVideo,cmCapTransmit,kRolePresentation);
			pBestMode->RemoveSipDtls(cmCapVideo,cmCapReceive,kRolePresentation);

			bIsRemoved = TRUE;
		}

		return bIsRemoved;
	}

	return bIsRemoved;
}

/*
 * The function checks if DTLS encryption is set, if it does and call is not TIP disconnect the call.
 * */
BOOL CSipTransaction::RejectDTLSEncIfNeeded(BYTE bIsDisconnectOnEncFailure)
{
	BOOL retVal = FALSE;
	/* we only support DTLS in TIP call, disconnect otherwise */
	if (!(m_pParty->GetIsTipCall()) && bIsDisconnectOnEncFailure)
	{
		DBGPASSERT(YES);
		PTRACE(eLevelError,"IsDTLSEncryptionSupported:::  DTLS mismatch");
		SetDialState(kNoRecovery);
		EndTransaction(SIP_CAPS_DONT_MATCH);
		retVal = TRUE;
	}
	return retVal;
}

/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendVideoChangeAfterVsrMsgToParty()
{
	//SendMessageToParty(SIP_TRANS_PARTY_CHANGE_VIDEO_AFTER_VSR_MSG, pSeg);
	//POBJDELETE(pSeg);
	CSegment * pSeg2 = new CSegment;

	m_pTargetMode->Serialize(NATIVE, *pSeg2);

	SendMessageToParty(SIP_TRANS_PARTY_CHANGE_VIDEO_AFTER_VSR_MSG, pSeg2);
}
//--------------------------------------------------------------------------
void CSipTransaction::OnRemoveAvcToSvcArtTranslatorAnycase(CSegment* pParam)
{
    TRACEINTO << "PartyId: " << m_pParty->GetPartyId() << ", ConfId:" << m_pParty->GetConfId() << ", m_state:" << m_state;

    m_pTargetMode->DeSerialize(NATIVE, *pParam);
    m_pSipCntl->Deescalate();
}


//--------------------------------------------------------------------------
void CSipTransaction::OnPartyTranslatorArtsDisconnected(CSegment* pParam)
{
    TRACEINTO << "PartyId: " << m_pParty->GetPartyId() << ", ConfId:" << m_pParty->GetConfId() << ", m_state:" << m_state;

    // update streams in the current mode
    m_pCurrentMode->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapAudio, cmCapReceive, kRolePeople), cmCapAudio, cmCapReceive, kRolePeople);
    m_pCurrentMode->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople), cmCapVideo, cmCapReceive, kRolePeople);
    m_pTargetModeMaxAllocation->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapAudio, cmCapReceive, kRolePeople), cmCapAudio, cmCapReceive, kRolePeople);
    m_pTargetModeMaxAllocation->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople), cmCapVideo, cmCapReceive, kRolePeople);

    m_pCurrentMode->Dump("CSipTransaction::OnPartyTranslatorArtsDisconnected");

    // update party control
    m_pParty->SendAvcToSvcArtTranslatorDisconnectedToPartyControl(STATUS_OK);
}

/////////////////////////////////////////////////////////////////////////BRIDGE-10820
void CSipTransaction::UpdateLocalCapsWithEncryptionParameters(CSipComMode* pBestMode, cmCapDataType mediaType , ERoleLabel eRole)
{
	PASSERTMSG_AND_RETURN(!pBestMode || !m_pSipCntl, "!pBestMode || !m_pSipCntl");

	BOOL 	 bUseMki   = TRUE;
	CSdesCap *pSdesCap = NULL;

	pSdesCap = pBestMode->GetSipSdes(mediaType,cmCapReceive, eRole);
	bUseMki  = GetIsUseMkiForLocalCaps(pBestMode,mediaType, eRole);

	PASSERTMSG_AND_RETURN( !pSdesCap, "!pSdesCap");

	TRACEINTO << " mediaType " << (int)mediaType << " eRole " << (int)eRole << " bUseMki " << (int)bUseMki ;

	//remove unchosen SDES from local caps
    m_pSipCntl->RemoveUnsupportedSdesCapFromLocalCaps(pSdesCap->GetSdesCryptoSuite(), bUseMki ,mediaType, eRole);

     //upate  SDES tag from bestMode to local caps
    m_pSipCntl->UpdateSdesTagFromBestModeToLocalCaps(pSdesCap, mediaType, eRole);

	m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, mediaType, eRole);
}

/////////////////////////////////////////////////////////////////////////BRIDGE-10820
BOOL CSipTransaction::GetIsUseMkiForLocalCaps(CSipComMode* pBestMode, cmCapDataType mediaType , ERoleLabel eRole)
{
	PASSERTMSG_AND_RETURN_VALUE(!pBestMode, "!pBestMode" ,TRUE);

	CSdesCap *pTxSdesCap = 	NULL;
	BOOL 	 bUseMki	 =  TRUE;

	pTxSdesCap 	=  pBestMode->GetSipSdes(mediaType,cmCapTransmit, eRole);
	PASSERTMSG_AND_RETURN_VALUE(!pTxSdesCap, "!pTxSdesCap" ,bUseMki);
	bUseMki 	=  pTxSdesCap->GetIsSdesMkiInUse(0);

	TRACEINTO << " MKI to use from transmit mode - bUseMki " << (int)bUseMki ;

	return bUseMki;
}
/////////////////////////////////////////////////////////////////////////////
void  CSipTransaction::SendLastTargetModeToParty()
{
	CSegment * pSeg2 = new CSegment;

	m_pTargetMode->Serialize(NATIVE, *pSeg2);

	SendMessageToParty(SIP_TRANS_PARTY_LAST_TARGET_MODE_MSG, pSeg2);
}

//BRIDGE-15745
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransaction::OnSipNotifyPendingTransactionDtlsStarted(CSegment* pParam)
{
	EPendingTransType ePendTrans 	= eNoPendTrans;
	BYTE valFromParam				= 0;

	*pParam >> valFromParam;
	ePendTrans = (EPendingTransType)valFromParam;

	TRACEINTO << " ePendTrans = "<< (ePendTrans==etransReinvite)?("etransReinvite"):((ePendTrans==etransBye)?("etransBye"):("eNoPendTrans"));

	if(IsValidTimer(DTLSTOUT))
	{
		TRACEINTO << "Closing DTLS and ending the current transaction.";
		DeleteTimer(DTLSTOUT);
		CloseDtlsChannels();
	}


}

