//+========================================================================+
//                SIPTransInviteWithSdpReq.cpp 			            	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteWithSdpReq.cpp                            	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
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
#include "SIPTransaction.h"
#include "SIPTransInviteWithSdpReq.h"
#include "SIPControl.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransInviteWithSdpReq)

ONEVENT(SIP_PARTY_ESTABLISH_CALL,			IDLE,					CSipTransInviteWithSdpReq::OnPartyEstablishCallIdle)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENINCHANNELS,	CSipTransInviteWithSdpReq::OnPartyChannelsConnectedOpenIn)

ONEVENT(AUDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveAudBridgeConnected)
ONEVENT(VIDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveVidBridgeConnected)
ONEVENT(FECCBRDGCONNECT,					sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveFeccBridgeConnected)

ONEVENT(SIP_PARTY_ORIGINAL_RMOTCAP,			sTRANS_CONNECTING,		CSipTransaction::OnPartyOriginalRemoteCaps)
ONEVENT(SIP_PARTY_RECEIVED_200OK,			sTRANS_CONNECTING,		CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED, 	sTRANS_DTLS_CLOSE_BEFORE_CLOSING_SIP_CHANNEL, CSipTransInviteWithSdpReq::OnDtlsClosedChannelBeforeSipCloseChannels)
ONEVENT(PARTYCONNECTTOUT,					sTRANS_CONNECTING,		CSipTransInviteWithSdpReq::OnPartyConnectToutConnecting)
ONEVENT(SET_SITE_AND_VISUAL_NAME,			sTRANS_CONNECTING,		CSipTransaction::OnPartySendSiteAndVisualNamePlusProductIdToPartyControl)

ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,		sTRANS_CHANGECHANNELS,	CSipTransInviteWithSdpReq::OnPartyChannelsDisconnectedChangeChannels)//the case when we close the video in channel because we can't recover video

ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_RECOVERY,		CSipTransInviteWithSdpReq::OnPartyChannelsUpdatedRecovery)
ONEVENT(PARTYCONNECTTOUT,					sTRANS_RECOVERY,		CSipTransInviteWithSdpReq::OnPartyConnectToutConnecting)
// for Internal and guess succeed states.
ONEVENT(SIP_CONF_CONNECT_CALL,				sTRANS_RMTCONNECTED,	CSipTransInviteWithSdpReq::OnConfConnectCallRmtConnected)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENOUTCHANNELS,	CSipTransInviteWithSdpReq::OnPartyChannelsConnectedOpenOut)// case of internal recovery recovery as well

ONEVENT(SIP_PARTY_DTLS_STATUS,				sTRANS_DTLS_STARTED,	CSipTransaction::OnPartyDtlsEndInd)
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			sTRANS_DTLS_STARTED,	CSipTransaction::DisconnectOnDtlsEncryptFail)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_UPDATED_CHAN, CSipTransInviteWithSdpReq::PartyConnectCall)
ONEVENT(PARTYCONNECTTOUT,					sTRANS_DTLS_UPDATED_CHAN, CSipTransInviteWithSdpReq::OnPartyConnectToutConnecting)
ONEVENT(DTLSTOUT,							sTRANS_DTLS_STARTED,	CSipTransaction::OnDtlsTout)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransInviteWithSdpReq::PartyConnectCall)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED,	sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransInviteWithSdpReq::PartyConnectCall) //BRIDGE-6184
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			ANYCASE,						CSipTransaction::TipEarlyPacketDtlsNotNeeded)

// timeouts:
ONEVENT(OPENBRIDGESTOUT,					sTRANS_OPENBRIDGES,		CSipTransInviteWithSdpReq::OnConfBridgesConnectionTout)
ONEVENT(UPDATEBRIDGESTOUT,  				sTRANS_RMTCONNECTED,	CSipTransInviteWithSdpReq::OnUpdateBridgesTout)

ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION,ANYCASE,				CSipTransInviteWithSdpReq::OnConfSetCapsAccordingToNewAllocation)
ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,  sTRANS_RMTCONNECTED,     CSipTransaction::OnRemoveAvcToSvcArtTranslatorAnycase)
ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED, sTRANS_RMTCONNECTED, CSipTransaction::OnPartyTranslatorArtsDisconnected)

// Ice
ONEVENT(MAKE_OFFER_IND,					sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpReq::OnIceInviteReceiveMakeOfferInd)
ONEVENT(ICE_PROCESS_ANS_IND,			sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack)
ONEVENT(ICE_REINVITE_IND,				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpReq::OnIceReinviteContentArrivedFromIceStack)
ONEVENT(ICE_MODIFY_OFFER_IND,			sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpReq::OnIceInviteModifyAnsArrivedFromIceStack)
ONEVENT(ICECOMPLETETOUT,  				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpReq::OnICETimeout)
ONEVENT(ICEGENERALTOUT,  				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpReq::OnICETimeout)
ONEVENT(ICEOFFERTOUT,  				    sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpReq::OnICEOfferTimeout)
ONEVENT(CLOSE_ICE_SESSION_IND,          sTRANS_WAITFORICECANDIDATES,    CSipTransInviteWithSdpReq::OnICEReceiveCloseIceIndWaitForCandidates)
ONEVENT(ICEPORTSRETRYTOUT,    			sTRANS_WAITFORICECANDIDATES,    CSipTransInviteWithSdpReq::OnIcePortsRetryTout)

ONEVENT(TRANS_ICE_CONN_CHECK_COMPLETE_IND,    ANYCASE,                        CSipTransInviteWithSdpReq::IceConnectivityCheckComplete)
//ONEVENT(ICE_CONN_CHECK_COMPLETE_IND,    ANYCASE,                        CSipTransInviteWithSdpReq::IceConnectivityCheckComplete)

ONEVENT(CLOSE_ICE_SESSION_IND,  sTRANS_CHANGECHANNELS,       CSipTransInviteWithSdpReq::OnICEReceiveCloseIceInd)

ONEVENT(SIP_PARTY_SLAVES_RECAP_FINISHED,ANYCASE,						CSipTransInviteWithSdpReq::OnPartySlavesRecapIsFinished)

ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_REOPENINCHANNELS,	CSipTransInviteWithSdpReq::OnPartyChannelsUpdatedRecovery) //added for ANAT

PEND_MESSAGE_MAP(CSipTransInviteWithSdpReq, CSipTransaction);

///////////////////////////////////////////////////////
CSipTransInviteWithSdpReq::CSipTransInviteWithSdpReq(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = TRUE;
	m_bIsReInvite = FALSE;
    m_bIsToCloseVideoChannels 	= FALSE;
	m_bIsToCloseDataChannels 	= FALSE;
	m_bIsToCloseBfcpChannels 	= FALSE;
	m_bIsToUpdateAnatIpType 	= FALSE;  //added for ANAT

	m_vidRxRate = 0;

	VALIDATEMESSAGEMAP
}

///////////////////////////////////////////////////////
CSipTransInviteWithSdpReq::~CSipTransInviteWithSdpReq()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnPartyEstablishCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyEstablishCallIdle : Name", m_pPartyConfName);

	// if the only audio cap is Rfc2833 we should reject the call.
	if (m_pTargetMode
			&& m_pTargetMode->IsMediaOn(cmCapAudio,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapAudio,cmCapTransmit)
		 	&& (m_pTargetMode->GetMediaType(cmCapAudio) != eRfc2833DtmfCapCode))
	{
		//to support BFCP as initiator

		// Don't declare content in first invite_req:
		if (m_pTargetMode->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
		{
			PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyEstablishCallIdle : Remove content from target mode. Name", m_pPartyConfName);
			m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
		}

		if(m_pSipCntl->GetIsEnableICE())
		{
			if( SendIceMgsReqAccordingToTargetMode(ICE_MAKE_OFFER_REQ) == STATUS_OK )
			{
				PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyEstablishCallIdle - ICE is enabled   ");
				m_state = sTRANS_WAITFORICECANDIDATES;
				StartTimer(ICEOFFERTOUT, MAKE_ICE_CANDIDATES_TIMER);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyEstablishCallIdle - Failed to start ice call ");
				if(m_pTargetMode->GetIsTipMode())
					m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipMasterCenter);
				else
					m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone);

				m_state = sTRANS_OPENINCHANNELS;
				SetDialState(kBeforeInvite);

			}
		}
		else
		{
			if(m_pTargetMode->GetIsTipMode())
				m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipMasterCenter);
			else
				m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone);
			m_state = sTRANS_OPENINCHANNELS;
			SetDialState(kBeforeInvite);
		}
	}
	else
	{
		PTRACE(eLevelError,"CSipTransInviteWithSdpReq::OnPartyEstablishCallIdle: No target mode found. must reject call");
		SetDialState(kCapsDontMatch);
		EndTransaction(SIP_CAPS_DONT_MATCH);
	}
}
/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnICEOfferTimeout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnICEOfferTimeout: Name ",m_pPartyConfName);
	m_pSipCntl->SetIsEnableICE(FALSE);
	m_pSipCntl->CloseIceSession();


}
/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnIceInviteReceiveMakeOfferInd(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnIceInviteReceiveMakeOfferInd: Name ",m_pPartyConfName);

	if (IsValidTimer (ICEOFFERTOUT))
	    DeleteTimer(ICEOFFERTOUT);

	WORD status = STATUS_OK;
	*pParam >> status;
	if (STATUS_OK != status)
	{
		if (m_IceMakeOfferAnswerCounter < 3)
		{
			m_IceMakeOfferAnswerCounter++;
		    DWORD retryTimer = 0;
		    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
		    pSysConfig->GetDWORDDataByKey("ICE_RETRY_TIMER_IN_SECONDS", retryTimer);
			m_pSipCntl->SetIsEnableICE(TRUE);
			m_pSipCntl->CloseIceSession();
			StartTimer(ICEPORTSRETRYTOUT, retryTimer*SECOND);
			PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnIceInviteReceiveMakeOfferInd: counter ",m_IceMakeOfferAnswerCounter);
		}
		else
		{
			m_IceMakeOfferAnswerCounter = 0;
			PASSERTMSG((DWORD)statIllegal, "CSipTransInviteWithSdpReq::OnIceInviteReceiveMakeOfferInd - making ICE offer failed ");
			m_pSipCntl->SetIsEnableICE(FALSE);
			m_pSipCntl->CloseIceSession();
		}
	}
	else
	{
		if(m_pTargetMode->GetIsTipMode())
			m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipMasterCenter);
		else
			m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone);
		m_state = sTRANS_OPENINCHANNELS;
		SetDialState(kBeforeInvite);
	}

}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::IceConnectivityCheckComplete(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpReq::IceConnectivityCheckComplete");

//	CSegment* CopyOfseg = new CSegment(*pParam);

	ICE_CHECK_COMPLETE_IND_S* pConnCheckStruct = (ICE_CHECK_COMPLETE_IND_S*) pParam->GetPtr(1);
	int status = pConnCheckStruct->status;
	PTRACE2INT (eLevelInfoNormal, "CSipTransInviteWithSdpReq::IceConnectivityCheckComplete - ", status);

	//In case of secondary - we first send Process answer to CM with A+V
	// and only after connectivity check we send Modify_req with A only - We don't need to wait to Re invite ind.
	if (STATUS_OK == status && m_bIsChangeInICEChannels)
//	if (m_bIsChangeInICEChannels)
	{
		m_bIsChangeInICEChannels = FALSE;
		SendIceMgsReqAccordingToTargetMode(ICE_MODIFY_SESSION_OFFER_REQ);
	}

//	SendHandleIceConnectivityCheckCompleteToParty(CopyOfseg);
//	POBJDELETE(CopyOfseg);
}
/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnPartyChannelsConnectedOpenIn(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyChannelsConnectedOpenIn: Name ",m_pPartyConfName);
	if (GetDialState() == kBeforeInvite) // channels in connected
	{
		InformChannelsConnectedOpenBridges(pParam);
		m_state = sTRANS_OPENBRIDGES;
		StartTimer(OPENBRIDGESTOUT, 10*SECOND);
	}
	else
		PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyChannelsConnectedOpenIn: wrong dial state, Name ",m_pPartyConfName);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::HandleBridgeConnectedInd(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::HandleBridgeConnectedInd: Name ",m_pPartyConfName);

	if (status == STATUS_OK)
	{
		// its OK if both the bridges connected or its audio only call or there is no much in the video capability
		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_isFeccBridgeConnected) ||
			(m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_pTargetMode->IsMediaOff(cmCapData,cmCapReceiveAndTransmit)) ||
			(m_isAudioBridgeConnected && GetIsVoice()) ||
			(m_isAudioBridgeConnected && m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit)))
		{
			if (GetDialState() == kBeforeInvite)
			{
				if (IsValidTimer(OPENBRIDGESTOUT))
				{
					DeleteTimer(OPENBRIDGESTOUT);
					PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::HandleBridgeConnectedInd: DeleteTimer(OPENBRIDGESTOUT) ");
				}
				m_state		 = sTRANS_CONNECTING;
				SetDialState(kInviteSent);

				m_pSipCntl->SipInviteReq(m_pAlternativeAddrStr);
			}
		}
	}
	else
	{
//		m_eDialState = kBadStatusAckArrived;
		DBGPASSERT(status);
		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpReq::HandleBridgeConnectedInd: Ack with bad status %d",status);
		EndTransaction(status);
	}
}

///////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: Name ",m_pPartyConfName);

	// pass the product ID to party control
	GetProductIdAndSendToConfLevel();

	BYTE bRemovedAudio, bRemovedVideo;
	bRemovedAudio = bRemovedVideo = NO;
	*pParam >> bRemovedAudio >> bRemovedVideo >> m_bIsToUpdateAnatIpType;  //add param for ANAT

	SendRemoteCapsReceivedToParty();
	RemoveBfcpAccordingToRemoteIdent();
	//RemoveBfcpIfNecessaryForANAT();//Added for ANAT 
	if(m_pSipCntl)
	{
		m_pSipCntl->RemoveUnsupportedSdesCapsForCiscoCallIfNeeded();
		m_pSipCntl->RemoveUnsupportedSdesCapsForRadVisionCallIfNeeded();
	}
	// TIP
	if (m_pTargetMode->GetIsTipMode() && m_pSipCntl)
	{
        const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
        CCommConf*      pCommConf      = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
        BYTE            isPreferTip    = FALSE;

        if (pCommConf && pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP)
        {
            isPreferTip = TRUE;
            PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting PREFER TIP FOR POLYCOM EP");
        }
        else
        {
            PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting !DONOT! PREFER TIP FOR POLYCOM EP");
            if (pCommConf)
                PTRACE2INT(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting !DONOT! PREFER TIP FOR POLYCOM EP TipCompatible:",pCommConf->GetIsTipCompatible());
            else
                PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting pCommConf is NULL");
        }

        if ( (!::CheckIfRemoteSdpIsTipCompatible(pCurRemoteCaps) && isPreferTip==FALSE) || !(pCurRemoteCaps->GetIsContainingCapCode(cmCapAudio,eAAC_LDCapCode)) )
		{
			PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting fall back from TIP to SIP: Name ",m_pPartyConfName);

			m_bNeedReInviteForSwitchToNoneTipCall = TRUE;

			FallbackFromTipToNoneTip();
		}
        else if (!CheckIfRemoteVideoRateIsTipCompatible() || IsNeedToRejectTheCallForPreferTIPmode(pCurRemoteCaps))
		{
			PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting drop the call: Name ",m_pPartyConfName);
			m_bIsNeedToDropCall = TRUE;
			m_pTargetMode->SetTipMode(eTipModeNone);
			m_pTargetModeMaxAllocation->SetTipMode(eTipModeNone);
		}
	}

	if (!m_pSipCntl)
	{
		PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting - m_pSipCntl is NULL, Name: ",m_pPartyConfName);
		DBGPASSERT(1);
		SetDialState(kNoRecovery);
		EndTransaction(SIP_BAD_STATUS);
		return;
	}

	// Check that the incoming open channels are correct. That means the remote can transmit this mode to the MCU.
	// Therefore we have 3 options: GuessSucceeded (no change needed), Internal recovery (need to change only the RTP and codec/bridge setting)
	// and no recovery (this media should be close)
	CSipComMode* pBestMode	= NULL;

	if(m_pSipCntl)
		pBestMode	= m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, FALSE);

	//Check encryption
	DWORD isEncrypted = Encryp_Off;

	if(pBestMode) {
		isEncrypted = pBestMode->GetIsEncrypted();
		
		// Check if Undefind party and flag, that allow non enc to connect and when available
		// if so, remove SDES from RMX side
		// set enc to off
		BOOL  bIsUndefinedParty = m_pSipCntl->IsUndefinedParty();
		BYTE  bIsDisconnectOnEncFailure = pBestMode->GetIsDisconnectOnEncryptionFailure();
		BYTE  bIsWhenAvailableEncMode = m_pSipCntl->IsWhenAvailableEncryptionMode();
		DWORD isDtlsEncrypted = pBestMode->GetIsDtlsEncrypted();

		if(isEncrypted == Encryp_On) {
			cmCapDataType mediaType;
			ERoleLabel eRole;

			for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++) {

				GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

				if (mediaType == cmCapBfcp)
					continue;

				if (pBestMode->IsMediaOn(mediaType,cmCapTransmit, eRole)) {
					CSdesCap *pSdesCap = NULL;
					pSdesCap =  pBestMode->GetSipSdes(mediaType,cmCapReceive, eRole);
					if (!pSdesCap) 
					{
						if(mediaType == cmCapData && pBestMode->GetSipSdes(cmCapAudio,cmCapReceive))
						{
				             TRACEINTO << "sdes available for audio but NOT for fecc - remove FECC ";
				             m_pSipCntl->RemoveFeccCaps();
				             pBestMode->RemoveData(cmCapReceiveAndTransmit);
						}
						else if((bIsUndefinedParty && bIsDisconnectOnEncFailure==NO && bIsWhenAvailableEncMode) ||
							(!bIsUndefinedParty && bIsWhenAvailableEncMode) ||
							isDtlsEncrypted == Encryp_On)
						{
							PTRACE2(eLevelError,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: set encryption to OFF. Name ",m_pPartyConfName);
							pBestMode->RemoveSipSdes(mediaType,cmCapTransmit,eRole);
							pBestMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
							m_pTargetMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
							isEncrypted = Encryp_Off;
							m_bNeedCloseSrtpChannels = true;
							m_pSipCntl->RemoveSdesCapFromLocalCaps(mediaType, eRole);
						}
						else
						{
							POBJDELETE(pBestMode);
							
							DBGPASSERT(YES);
							PTRACE2(eLevelError,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: SDES cap is incorrect or not matching. Name ",m_pPartyConfName);
							SetDialState(kNoRecovery);
							EndTransaction(SIP_CAPS_DONT_MATCH);
							return;
						}
					}
					else
					{
						UpdateLocalCapsWithEncryptionParameters(pBestMode, mediaType , eRole); 	//BRIDGE-10820
					}
				}
			}
			if(pBestMode && isEncrypted == Encryp_On)
			{
				SendUpdateDbEncryptionStatusToParty(YES);
			}
		}
		else if (isDtlsEncrypted == Encryp_On)
		{
			/* verify if we need to disconnect the call in case of DTLS encryption and not TIP call */
			if (RejectDTLSEncIfNeeded(pBestMode->GetIsDisconnectOnEncryptionFailure()))
			{
				POBJDELETE(pBestMode);
				return;
			}
		}

		m_vidRxRate = pBestMode->GetMediaBitRate(cmCapVideo, cmCapReceive);
	}

	if(IsNoRecovery(pBestMode) )
	{
		PTRACE2(eLevelError,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: Audio remote receive is not matching. Name ",m_pPartyConfName);
		SetDialState(kNotInDialState);
	}
	else
	{
		// Need to check if we should close channels (audio only media)
		// If we need to update incoming channels (we check UDP and RTP but for incoming channels - the only channels that are open in this stage - the only change possible is for RTP).
		// If we update RTP we should also update bridge.
		// Open outgoing bridge.
		// Open outgoing channel.
		// Set call as connected (or secondary).
		PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: Guess succeeded. Name ",m_pPartyConfName);
		SetDialState(kGuessSucceeded);	// this state indicates now that media can be establish, maybe after internal change.
	}

	m_bIsToCloseVideoChannels = FALSE;
	if(IsNoRecoveryForVideo(pBestMode))
	{
	  TRACEINTO<<"!@# there's no recovery for video";
		if(GetDialState() == kGuessSucceeded)
			SetDialState(kNoRecoveryForVideo);
		m_bIsToCloseVideoChannels = TRUE;
		const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
		//Secondary
		if (pCurRemoteCaps->IsMedia(cmCapVideo))
			m_bNeedReInviteForSecondary = TRUE;
	}

	m_bIsToCloseDataChannels = FALSE;
	if(IsNoRecoveryForData(pBestMode))
	{
		m_bIsToCloseDataChannels = TRUE;
	}

	m_bIsToCloseBfcpChannels = FALSE;

	//added for ANAT begin --- close bfcp chnl if remote EP is not bfcp/udp
	/*BOOL isANATContained = FALSE;
	sipSdpAndHeadersSt* pRemoteSdp= m_pSipCntl->GetRemoteSdp();
	
	if (pRemoteSdp)
		isANATContained = IsANATPresentInSDP(pRemoteSdp);*/

	if (m_bIsToUpdateAnatIpType && (pBestMode->IsMediaOn(cmCapBfcp,cmCapReceive)) && (eMediaLineSubTypeUdpBfcp != m_pSipCntl->GetBfcpType()))
		pBestMode->SetMediaOff(cmCapBfcp, cmCapReceive);
	//added for ANAT end
		
	if(IsNoRecoveryForBfcp(pBestMode))
	{
		m_bIsToCloseBfcpChannels = TRUE;
		PTRACE2(eLevelError,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: need to close bfcp channels Name:",m_pPartyConfName);
	}

	if(isEncrypted == Encryp_On )
		m_bNeedCloseSrtpChannels = TRUE;

	if (pBestMode)
	{
		//If BestMode is not RTV and local caps contain RTV - we need to send recap without
		// RTV do the EP won't send us RTV

		if(RemoveRtvCapsIfNeeded(pBestMode))
		{
			PTRACE2(eLevelError,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: Need to remove RTV caps. Name ",m_pPartyConfName);
			m_bNeedReinviteForRemoveRtv = TRUE;
		}
	}

	// update media from best mode to target mode
	if (pBestMode)
	{
//	if (pBestMode && pBestMode->IsMediaOn(cmCapAudio,cmCapReceive) && pBestMode->IsMediaOn(cmCapAudio,cmCapTransmit))
		m_pTargetMode->CopyMediaMode(*pBestMode,cmCapAudio,cmCapReceiveAndTransmit);
//	if (pBestMode && pBestMode->IsMediaOn(cmCapVideo,cmCapReceive) && pBestMode->IsMediaOn(cmCapVideo,cmCapTransmit))
		m_pTargetMode->CopyMediaMode(*pBestMode,cmCapVideo,cmCapReceiveAndTransmit);
		m_pTargetMode->CopyMediaMode(*pBestMode,cmCapData,cmCapReceiveAndTransmit);
		m_pTargetMode->CopyMediaMode(*pBestMode,cmCapBfcp,cmCapReceiveAndTransmit);

		if(m_pTargetMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))
		{
			PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: audio only, removing bfcp -no content. Name ",m_pPartyConfName);
			m_pTargetMode->SetMediaOff(cmCapBfcp,cmCapReceiveAndTransmit);

			m_bNeedReInviteForBfcp = FALSE;
		}
		if (m_pTargetMode->GetConfType() == kCop)
			m_pTargetMode->SetCopTxLevel(pBestMode->GetCopTxLevel());
	}


	// end the function in one of two ways: disconnect the call if no recovery, or continue recovery flow (disconnect channels if needed)
	if(GetDialState() != kNotInDialState)
	{
		//We always Invite first with UDP, if we get TCP here then reInvite must be invoked and another reInvite with content after
		if (m_pSipCntl && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRolePeople))
		{

			BfcpDecisionCenter(pBestMode);
		}
        if (m_pSipCntl && m_pSipCntl->GetIsEnableICE())
        {
            if (STATUS_OK == SendIceMgsReqAccordingToTargetModeAndCurrentMode(ICE_PROCESS_ANSWER_REQ))
            {
                m_state = sTRANS_WAITFORICECANDIDATES;
                StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
            }
            else
            {
                m_bNeedReInviteForIce = TRUE;
                m_pSipCntl->SetIsEnableICE(FALSE);
                m_bNeedCloseIceChannels = TRUE;
                ContinueToCloseChannels();
                m_pSipCntl->RemoveIceParamsObject();
            }
        }
        else
        {
	  TRACEINTO<<"!@# continue to close channels";
            ContinueToCloseChannels();  
        }
	}
	else  // no recovery
	{
		DBGPASSERT(1);
		PTRACE2(eLevelError,"CSipTransInviteWithSdpReq::OnPartyReceived200OkConnecting: Audio remote receive is not matching. Name ",m_pPartyConfName);
		SetDialState(kNoRecovery);
		EndTransaction(SIP_CAPS_DONT_MATCH);
	}

	POBJDELETE(pBestMode);
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnPartyChannelsDisconnectedChangeChannels(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyChannelsDisconnectedChangeChannels");
    if(m_bNeedCloseIceChannels)
	{
        UpdateDbChannelsStatus(pParam, FALSE);
		m_pSipCntl->CloseIceSession();
	//	m_bNeedCloseIceChannels = FALSE;
	}
    else if (!m_bIsToUpdateAnatIpType)  //added for ANAT
    {
        m_state = sTRANS_RECOVERY;
        HandleChannelsDisconnectedStartRecovery(pParam);
    }
    else   //added for ANAT, reopen in channels
    	HandleChannelsDisconnectedReopenChnlForAnat(pParam);

}
/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnICEReceiveCloseIceInd (CSegment* pParam)
{
    PTRACE (eLevelInfoNormal, "CSipTransInviteWithSdpReq::OnICEReceiveCloseIceInd");
    m_state = sTRANS_RECOVERY;
    CheckChangingInCurrentMode();
}
/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnICEReceiveCloseIceIndWaitForCandidates(CSegment* pParam)
{
	PTRACE (eLevelInfoNormal, "CSipTransInviteWithSdpReq::OnICEReceiveCloseIceIndWaitForCandidates");

	if(m_pTargetMode->GetIsTipMode())
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipMasterCenter);
	else
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone);
	m_state = sTRANS_OPENINCHANNELS;
	SetDialState(kBeforeInvite);
}
/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnPartyChannelsUpdatedRecovery(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyChannelsUpdatedRecovery");

  if( m_pTargetMode->GetIsTipMode() )
  {
    m_bIsTipMute = TRUE;
    SendMuteMediaToParty(cmCapTransmit);
  }

	HandleChannelsUpdatedDuringRecovery(pParam);
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::InternalRecoveryCompleted()
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::InternalRecoveryCompleted");
	//open out channels
	switch (GetDialState())
	{
	case kGuessSucceeded:
	case kNoRecoveryForVideo:
		{
			m_state = sTRANS_RMTCONNECTED;
			StartTimer(UPDATEBRIDGESTOUT, BRIDGES_TIME * SECOND);
			InformPartyRemoteConnect();
			break;
		}
	default:
			DBGPASSERT(GetDialState());
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnConfConnectCallRmtConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnConfConnectCallRmtConnected: Name ",m_pPartyConfName);
	if (IsValidTimer(UPDATEBRIDGESTOUT))
		DeleteTimer(UPDATEBRIDGESTOUT);
	m_state = sTRANS_OPENOUTCHANNELS;
	BYTE isAnsweringToNewCap = FALSE;

	if(m_bNeedCloseIceChannels || m_bNeedCloseSrtpChannels)
	{
		OpenInAndOutChannelsIfNeeded (isAnsweringToNewCap); //if closed
		m_bNeedCloseIceChannels = FALSE;
		m_bNeedCloseSrtpChannels = FALSE;
	}

	else
		OpenOutChannels(isAnsweringToNewCap);
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnPartyChannelsConnectedOpenOut(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyChannelsConnectedOpenOut: Name ",m_pPartyConfName);
	HandleOutChannelsConnected(pParam);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::PartyConnectCall()
{
	PTRACE2(eLevelInfoNormal, "CSipTransInviteWithSdpReq::PartyConnectCall: Name ", m_pPartyConfName);
	switch(GetDialState())
	{
		case kGuessSucceeded:
		case kNoRecoveryForVideo:
			m_pSipCntl->SipInviteAckReq();
			if(IsNeedToSendRtcpVideoPreference())
				SendStartVideoPreferenceToParty();
			//send flow control with new video rate
			if ( kCp == m_pTargetMode->GetConfType() && m_vidRxRate != 0)
			{
				m_pSipCntl->SendFlowControlReq(mainType, cmCapReceive, m_vidRxRate);
			}
			EndTransaction();
			break;
		case kReInviteSent:
		case kNotInDialState:
			PTRACE2INT(eLevelInfoNormal, "CSipTransInviteWithSdpReq::PartyConnectCall: Do nothing on dial state Name ", GetDialState());  // there is nothing to do
			break;

		default:
			DBGPASSERT(GetDialState());
	}
}


/////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnPartyConnectToutConnecting(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnPartyConnectToutConnecting: Disconnect call");
	if (GetDialState() != kInviteSent)
	{
		SetDialState(kConnectTimer);
		DBGPASSERT(GetDialState());
	}
	EndTransaction(SIP_REMOTE_NO_ANSWER);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnConfBridgesConnectionTout(CSegment* pParam)
{
	PASSERTMSG((DWORD)statIllegal,"CSipTransInviteWithSdpReq::OnConfBridgesConnectionTout");
	EndTransaction(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnUpdateBridgesTout(CSegment* pParam)
{
	PASSERTMSG((DWORD)statIllegal,"CSipTransInviteWithSdpReq::OnUpdateBridgesTout");
	EndTransaction(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnConfSetCapsAccordingToNewAllocation(CSegment* pParam)
{
	SetCapsAccordingToNewAllocation(pParam);
	m_bNeedReInviteForReAlloc = TRUE;
}
/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::ContinueToCloseChannels()
{
     m_state = sTRANS_CHANGECHANNELS;
     CloseChannelsIfNeeded(m_bIsToCloseVideoChannels, m_bIsToCloseDataChannels, m_bIsToCloseBfcpChannels, FALSE, FALSE, m_bIsToUpdateAnatIpType);  //add param for ANAT
}
/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnIceInviteModifyAnsArrivedFromIceStack(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpReq::OnIceInviteModifyAnsArrivedFromIceStack");
	HandleReinviteforICE(pParam);
}
/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnIceReinviteContentArrivedFromIceStack(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpReq::OnIceReinviteContentArrivedFromIceStack");
	HandleReinviteforICE(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::HandleReinviteforICE(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpReq::HandleReinviteforICE");
    if (IsValidTimer (ICECOMPLETETOUT))
        DeleteTimer(ICECOMPLETETOUT);
    //todo - Get the reinvite content

    DWORD status;
   	*pParam >> status;

	CMedString str;
	str << "CSipTransInviteWithSdpReq::HandleReinviteforICE - status: " << status << " connectivity status: " << m_pSipCntl->GetIceConnectivityStatus() << " is MOC: " << m_pSipCntl->IsRemoteMicrosoft();
	FPTRACE (eLevelInfoNormal, str.GetString());

	BOOL preserve_ice_in_local_call_flag = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_PRESERVE_ICE_CHANNEL_IN_CASE_OF_LOCAL_MODE);

	if (!preserve_ice_in_local_call_flag)
	{
	    if (status != STATUS_OK || (m_pSipCntl->GetIceConnectivityStatus() != eIceConnectedRemote && !m_pSipCntl->IsRemoteMicrosoft()))
	    {
	    	m_pSipCntl->SetIsEnableICE(FALSE);
	    	m_bNeedCloseIceChannels = TRUE;

	    	FPTRACE (eLevelInfoNormal, "The call is local, ICE channel closed");

	    	//IPV6_Shira
	    	UdpAddresses udpAddressesParams = m_pSipCntl->GetUdpAddressParams();
	    	CIceParams*	 pIceParams          = m_pSipCntl->GetICEParams();
	    	CConfIpParameters* pServiceParams = m_pSipCntl->GetServiceParams();

	    	if (pIceParams)
	    	{
				mcTransportAddress *pIpFromIceStack = pIceParams->GetIceMediaIp(eAudioSession);

				TRACEINTO << "udpAddressesParams.IpType:" << (DWORD)udpAddressesParams.IpType;
				if (pIpFromIceStack)
					TRACEINTO << "iceParams->GetIceMediaIp(eAudioSession)->ipVersion:" << (DWORD)pIceParams->GetIceMediaIp(eAudioSession)->ipVersion;
				else
					TRACEINTO << "ipFromIceStack is NULl";

				if  (udpAddressesParams.IpType == eIpType_Both && pIpFromIceStack &&
						pIceParams->GetIceMediaIp(eAudioSession)->ipVersion == eIpVersion6)
				{
					FPTRACE (eLevelInfoNormal, "CSipTransInviteWithSdpReq::HandleReinviteforICE - RMX is both, candidates are ipv6 -> set ipVersion=IPV6");
					CSipNetSetup* pNetSetup = m_pSipCntl->GetNetSetup();
					if (pNetSetup)
					{
						pNetSetup->SetIpVersion(eIpVersion6);
						//m_eLocalMediaIpType = eIpVersion6;

						CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
						pNetSetup->SetSipLocalMediaType(eIpVersion6);

						if (pCommConf && pServiceParams)
						{
							const char* strConfName  = pCommConf->GetName();
							int nameLen = strlen(strConfName);

							char strLocalIp[IPV6_ADDRESS_LEN];
							memset (&strLocalIp,'\0',IPV6_ADDRESS_LEN);

							mcTransportAddress localSipAddr;
							memset(&localSipAddr, 0, sizeof(mcTransportAddress));
							localSipAddr.ipVersion = eIpVersion6;

							ipAddressIf localAddressService;

							/*
							for  (int i=0; i<NUM_OF_IPV6_ADDRESSES; ++i )
							{
								localAddressService = pServiceParams->GetIpV6Address(i);
								memcpy(&localSipAddr.addr.v6.ip, &localAddressService, sizeof(ipAddressIf));
								::ipToString(localSipAddr, strLocalIp, 1);
								TRACEINTO << "i:" << (DWORD)i << ", IP: "<< strLocalIp;
							}
							*/

							BYTE place = ::FindPlaceAccordingtoScopeType(eScopeIdGlobal, pServiceParams);

							if (place == 0xFF)
								place = ::FindPlaceAccordingtoScopeType(eScopeIdSite, pServiceParams);

							if (place != 0xFF)
							{
								localAddressService = pServiceParams->GetIpV6Address((int)place);
								memcpy(&localSipAddr.addr, &localAddressService, sizeof(ipAddressIf));
								::ipToString(localSipAddr, strLocalIp, 1);

								TRACEINTO << "Local IP: "<< strLocalIp;

								pNetSetup->SetLocalSipAddress(strConfName,nameLen,strLocalIp);
							}
							else
								TRACEINTO << "There is no Global/Site address of IPV6. localSipAddress will not be updated. Need to check contact feild (IPV4/6)";

						}
						else
							TRACEINTO << "pCommConf is NULL so localSipAddress will not be updated. Need to check contact feild (IPV4/6)";
					}
					else
						TRACEINTO << "pNetSetup is NULL so IpVersion will not be updated. Need to check IPs (IPV4/6)";
				}
	    	}
	    }

	} else
	{
		if (status != STATUS_OK || (m_pSipCntl->GetIceConnectivityStatus() != eIceConnectedRemote && !m_pSipCntl->IsRemoteMicrosoft()))
			FPTRACE (eLevelInfoNormal, "The call is local, ICE channel preserved due to configuration");
	}

    m_bNeedReInviteForIce = TRUE;

    ContinueToCloseChannels();
}
/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnICETimeout(CSegment* pParam)
{
    PASSERTMSG (TRUE, "CSipTransInviteWithSdpReq::OnICETimeout");
    if (eIceNotConnected != m_pSipCntl->GetIceConnectivityStatus())
    {
        m_pSipCntl->SetIsEnableICE(FALSE);
        m_bNeedCloseIceChannels = TRUE;
        ContinueToCloseChannels();
    }
    else
        EndTransaction(SIP_NO_ADDR_FOR_MEDIA);

}
/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack");
	if (IsValidTimer (ICEGENERALTOUT))
	   DeleteTimer(ICEGENERALTOUT);

	DWORD status;
	*pParam >> status;

	if (status == STATUS_OK)
		StartTimer(ICECOMPLETETOUT, MAKE_ICE_CANDIDATES_TIMER);
	else
	{
		PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack - Status FAIL!! - No need to wait for ReInvite_ind from Ice stuck");
		m_bNeedCloseIceChannels = TRUE;
		ContinueToCloseChannels();
	}

}

////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::UserAgentAndVersionUpdated(const char* cUserAgent, const char* pVersionId)
{
	BOOL change_audio_list_flag = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_CHANGE_AUDIO_CAPS_LIST_ACCORDING_TO_VENDOR);
	if(change_audio_list_flag)
	{
		m_bNeedToSendReInviteWithFullAudioCaps = isVendorSupportFullAudioCaps(cUserAgent, pVersionId);
		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpReq::GetProductIdAndSendToConfLevel NeedToSendReInviteWithFullAudioCaps=",m_bNeedToSendReInviteWithFullAudioCaps);
	}
}

//////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnIcePortsRetryTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnIcePortsRetryTout: make offer");
	m_pSipCntl->SetIsEnableICE(TRUE);

	SendIceMgsReqAccordingToTargetMode(ICE_MAKE_OFFER_REQ);

	StartTimer(ICEOFFERTOUT, MAKE_ICE_CANDIDATES_TIMER);
}

////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnPartySlavesRecapIsFinished(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpReq::OnPartySlavesRecapIsFinished");
}

////////////////////////////////////////////////////////////////////////////////////
//added for ANAT
void CSipTransInviteWithSdpReq::HandleChannelsDisconnectedReopenChnlForAnat(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpReq::HandleChannelsDisconnectedReopenChnlForAnat");
	UpdateDbChannelsStatus(pParam, FALSE);

	if(m_pTargetMode->GetIsTipMode())
	{
		m_pSipCntl->SipNewCallReq((CSipComMode*)m_pTargetMode, eTipMasterCenter);
	}
	else
	{
		m_pSipCntl->SipNewCallReq((CSipComMode*)m_pTargetMode, eTipNone);
	}
	
	m_state = sTRANS_REOPENINCHANNELS;
}
////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpReq::OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpReq::OnDtlsClosedChannelBeforeSipCloseChannels - error - shouldn't happen in this state - m_state:", m_state);

//	CloseChannelsIfNeeded(m_bIsToCloseVideoChannels, m_bIsToCloseDataChannels, m_bIsToCloseBfcpChannels, FALSE, FALSE, m_bIsToUpdateAnatIpType);  //add param for ANAT
}

void CSipTransInviteWithSdpReq::OnPartyVideoArtDisconnected()
{
        m_state = sTRANS_RECOVERY;
	UpdateChannelsIfNeeded();	
}
