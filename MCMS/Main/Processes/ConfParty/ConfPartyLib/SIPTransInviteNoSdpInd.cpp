//+========================================================================+
//                SIPTransInviteNoSdpInd.cpp           			    	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteNoSdpInd.cpp                                     	   |
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
#include "SIPInternals.h"
#include "SipUtils.h"
#include "IpNetSetup.h"
#include "SipNetSetup.h"
#include "CsInterface.h"
#include "SipScm.h"
#include "SipCall.h"
#include "ConfApi.h"
#include "IPParty.h"
#include "SIPControl.h"
#include "Lobby.h"
#include "SIPParty.h"
#include "ConfPartyGlobals.h"
#include "IpServiceListManager.h"
#include "IPParty.h"
#include "SIPTransaction.h"
#include "SIPTransInviteNoSdpInd.h"

////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransInviteNoSdpInd)

ONEVENT(SIP_PARTY_ESTABLISH_CALL,			IDLE,					CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENINCHANNELS,	CSipTransInviteNoSdpInd::OnPartyChannelsConnectedOpenInChannels)
ONEVENT(SET_SITE_AND_VISUAL_NAME,			sTRANS_OPENINCHANNELS,	CSipTransaction::OnPartySendSiteAndVisualNamePlusProductIdToPartyControl)

ONEVENT(AUDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveAudBridgeConnected)
ONEVENT(VIDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveVidBridgeConnected)
ONEVENT(FECCBRDGCONNECT,					sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveFeccBridgeConnected)

ONEVENT(SIP_PARTY_ORIGINAL_RMOTCAP,			sTRANS_CONNECTING,		CSipTransaction::OnPartyOriginalRemoteCaps)
ONEVENT(SIP_PARTY_RECEIVED_ACK,				sTRANS_CONNECTING,		CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting)
ONEVENT(PARTYCONNECTTOUT,					sTRANS_CONNECTING,		CSipTransaction::OnPartyConnectToutConnecting)

ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED, 	sTRANS_DTLS_CLOSE_BEFORE_CLOSING_SIP_CHANNEL, CSipTransInviteNoSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels)
ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,		sTRANS_CHANGECHANNELS,	CSipTransInviteNoSdpInd::OnPartyChannelsDisconnectedChangeChannels)//the case when we close the video in channel because we can't recover video

ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_RECOVERY,		CSipTransInviteNoSdpInd::OnPartyChannelsUpdatedRecovery)

// for Internal and guess succeed states.
ONEVENT(SIP_CONF_CONNECT_CALL,				sTRANS_RMTCONNECTED,	CSipTransInviteNoSdpInd::OnConfConnectCallRmtConnected)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENOUTCHANNELS,	CSipTransInviteNoSdpInd::OnPartyChannelsConnectedOpenOut)// case of internal recovery recovery as well

ONEVENT(SIP_PARTY_DTLS_STATUS,				sTRANS_DTLS_STARTED,	CSipTransaction::OnPartyDtlsEndInd)
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			sTRANS_DTLS_STARTED,	CSipTransaction::DisconnectOnDtlsEncryptFail)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_UPDATED_CHAN, CSipTransInviteNoSdpInd::PartyConnectCall)
ONEVENT(PARTYCONNECTTOUT,					sTRANS_DTLS_UPDATED_CHAN, CSipTransaction::OnPartyConnectToutConnecting)
ONEVENT(DTLSTOUT,							sTRANS_DTLS_STARTED,	CSipTransaction::OnDtlsTout)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransInviteNoSdpInd::PartyConnectCall)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED,	sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransInviteNoSdpInd::PartyConnectCall) //BRIDGE-6184
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			ANYCASE,	CSipTransaction::TipEarlyPacketDtlsNotNeeded)

// timeouts:
ONEVENT(OPENBRIDGESTOUT,					sTRANS_OPENBRIDGES,		CSipTransInviteNoSdpInd::OnConfBridgesConnectionTout)
ONEVENT(UPDATEBRIDGESTOUT,  				sTRANS_RMTCONNECTED,	CSipTransInviteNoSdpInd::OnUpdateBridgesTout)

ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION,ANYCASE,				CSipTransInviteNoSdpInd::OnConfSetCapsAccordingToNewAllocation)
ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,  sTRANS_RMTCONNECTED,                 CSipTransaction::OnRemoveAvcToSvcArtTranslatorAnycase)
ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED, sTRANS_RMTCONNECTED, CSipTransaction::OnPartyTranslatorArtsDisconnected)

ONEVENT(SIP_PARTY_SLAVES_RECAP_FINISHED,ANYCASE,					CSipTransInviteNoSdpInd::OnPartySlavesRecapIsFinished)
// Ice
ONEVENT(MAKE_OFFER_IND,				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteNoSdpInd::OnIceInviteReceiveMakeOfferInd)
ONEVENT(ICEOFFERTOUT,  			   	sTRANS_WAITFORICECANDIDATES,	CSipTransInviteNoSdpInd::OnICEOfferTimeout)
ONEVENT(ICE_PROCESS_ANS_IND,		sTRANS_WAITFORICECANDIDATES,	CSipTransInviteNoSdpInd::OnIceInviteProcessAnsArrivedFromIceStack)
ONEVENT(ICEGENERALTOUT,  			sTRANS_WAITFORICECANDIDATES,	CSipTransInviteNoSdpInd::OnICETimeout)
ONEVENT(ICECOMPLETETOUT,  			sTRANS_WAITFORICECANDIDATES,	CSipTransInviteNoSdpInd::OnICETimeout)
ONEVENT(TRANS_ICE_CONN_CHECK_COMPLETE_IND, ANYCASE,					CSipTransInviteNoSdpInd::IceConnectivityCheckComplete)
ONEVENT(ICE_MODIFY_OFFER_IND,		sTRANS_WAITFORICECANDIDATES,	CSipTransInviteNoSdpInd::OnIceInviteModifyAnsArrivedFromIceStack)
ONEVENT(ICE_REINVITE_IND,			sTRANS_WAITFORICECANDIDATES,	CSipTransInviteNoSdpInd::OnIceReinviteContentArrivedFromIceStack)
ONEVENT(CLOSE_ICE_SESSION_IND,    	sTRANS_WAITFORICECANDIDATES,    CSipTransInviteNoSdpInd::OnICEReceiveCloseIceIndWaitForCandidates)
ONEVENT(ICEPORTSRETRYTOUT,    		sTRANS_WAITFORICECANDIDATES,    CSipTransInviteNoSdpInd::OnIcePortsRetryTout)
ONEVENT(CLOSE_ICE_SESSION_IND,  	sTRANS_CHANGECHANNELS,       	CSipTransInviteNoSdpInd::OnICEReceiveCloseIceInd)

PEND_MESSAGE_MAP(CSipTransInviteNoSdpInd, CSipTransaction);

////////////////////////////////////////////////////////////////////////////
CSipTransInviteNoSdpInd::CSipTransInviteNoSdpInd(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = TRUE;
	m_bIsReInvite = FALSE;

	m_bIsToCloseVideoChannels 	= FALSE;
	m_bIsToCloseDataChannels	= FALSE;
	m_bIsToCloseBfcpChannels 	= FALSE;
}

////////////////////////////////////////////////////////////////////////////
CSipTransInviteNoSdpInd::~CSipTransInviteNoSdpInd()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle: Name ", m_pPartyConfName);

	// if the only audio cap is Rfc2833 we should reject the call.
	if (m_pTargetMode
			&& m_pTargetMode->IsMediaOn(cmCapAudio,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapAudio,cmCapTransmit)
		 	&& (m_pTargetMode->GetMediaType(cmCapAudio) != eRfc2833DtmfCapCode))
	{
		RemoveBfcpAccordingToRemoteIdent();
		// Don't declare content in first invite:
		if (m_pTargetMode->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
		{
			PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle : Remove content from target mode. Name", m_pPartyConfName);
			m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
		}
		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle: TIP mode: ", m_pTargetMode->GetIsTipMode());

//		if(m_pSipCntl->GetIsCiscoTagExist())
			m_pSipCntl->RemoveUnsupportedSdesCapsForCiscoCallIfNeeded();
			m_pSipCntl->RemoveUnsupportedSdesCapsForRadVisionCallIfNeeded();
		if(m_pSipCntl->GetIsEnableICE())
		{
			if(GetIsRemoteIdentMicrosoft())
			{
				PTRACE(eLevelError,"CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle: MS-ICE invite no SDP , remote is Microsoft - must reject call!");
				SetDialState(kCapsDontMatch);
				EndTransaction(SIP_CAPS_DONT_MATCH);
			}
			else if( SendIceMgsReqAccordingToTargetMode(ICE_MAKE_OFFER_REQ) == STATUS_OK )
			{
				PTRACE(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle - ICE is enabled   ");
				m_state = sTRANS_WAITFORICECANDIDATES;
				StartTimer(ICEOFFERTOUT, MAKE_ICE_CANDIDATES_TIMER);
			}
			else
			{
				PTRACE(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle - Failed to start ice call, fallback to non-ice");
				if(m_pTargetMode->GetIsTipMode())
					m_pSipCntl->SipNewCallReq((CSipComMode*)m_pTargetMode, eTipMasterCenter);
				else
					m_pSipCntl->SipNewCallReq((CSipComMode*)m_pTargetMode, eTipNone);
				m_state = sTRANS_OPENINCHANNELS;
			}
		}
		else
		{
			if(m_pTargetMode->GetIsTipMode())
			{
				m_pSipCntl->SipNewCallReq((CSipComMode*)m_pTargetMode, eTipMasterCenter);
			}
			else {
				m_pSipCntl->SipNewCallReq((CSipComMode*)m_pTargetMode, eTipNone);
			}
			m_state = sTRANS_OPENINCHANNELS;
		}

	}
	else
	{
		PTRACE(eLevelError,"CSipTransInviteNoSdpInd::OnPartyEstablishCallIdle: No target mode found. must reject call");
		SetDialState(kCapsDontMatch);
		EndTransaction(SIP_CAPS_DONT_MATCH);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnIceInviteReceiveMakeOfferInd(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnIceInviteReceiveMakeOfferInd Name ",m_pPartyConfName);

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
			PTRACE2INT(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnIceInviteReceiveMakeOfferInd: counter ",m_IceMakeOfferAnswerCounter);
		}
		else
		{
			m_IceMakeOfferAnswerCounter = 0;
			PASSERTMSG((DWORD)statIllegal, "CSipTransInviteNoSdpInd::OnIceInviteReceiveMakeOfferInd - making ICE offer failed ");
			m_pSipCntl->SetIsEnableICE(FALSE);
			m_pSipCntl->CloseIceSession();
		}
	}
	else
	{
		if(m_pTargetMode->GetIsTipMode())
			m_pSipCntl->SipNewCallReq((CSipComMode*)m_pTargetMode, eTipMasterCenter);
		else
			m_pSipCntl->SipNewCallReq((CSipComMode*)m_pTargetMode, eTipNone);
		m_state = sTRANS_OPENINCHANNELS;
	}

}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnICEOfferTimeout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnICEOfferTimeout: Name ",m_pPartyConfName);
	m_pSipCntl->SetIsEnableICE(FALSE);
	m_pSipCntl->CloseIceSession();
}

////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnIceInviteProcessAnsArrivedFromIceStack(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteNoSdpInd::OnIceInviteProcessAnsArrivedFromIceStack");
	if (IsValidTimer (ICEGENERALTOUT))
	   DeleteTimer(ICEGENERALTOUT);

	DWORD status;
	*pParam >> status;

	if (status == STATUS_OK)
		StartTimer(ICECOMPLETETOUT, MAKE_ICE_CANDIDATES_TIMER);
	else
	{
		PTRACE(eLevelInfoNormal, "CSipTransInviteNoSdpInd::OnIceInviteProcessAnsArrivedFromIceStack - Status FAIL!! - No need to wait for ReInvite_ind from Ice stuck");
		m_bNeedCloseIceChannels = TRUE;
		ContinueToCloseChannels();
	}
}

///////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnICETimeout(CSegment* pParam)
{
    PASSERTMSG (TRUE, "CSipTransInviteNoSdpInd::OnICETimeout");
    if (eIceNotConnected != m_pSipCntl->GetIceConnectivityStatus())
    {
        m_pSipCntl->SetIsEnableICE(FALSE);
        m_bNeedCloseIceChannels = TRUE;
        ContinueToCloseChannels();
    }
    else
        EndTransaction(SIP_NO_ADDR_FOR_MEDIA);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::IceConnectivityCheckComplete(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteNoSdpInd::IceConnectivityCheckComplete");

//	CSegment* CopyOfseg = new CSegment(*pParam);

	ICE_CHECK_COMPLETE_IND_S* pConnCheckStruct = (ICE_CHECK_COMPLETE_IND_S*) pParam->GetPtr(1);
	int status = pConnCheckStruct->status;
	PTRACE2INT (eLevelInfoNormal, "CSipTransInviteNoSdpInd::IceConnectivityCheckComplete - ", status);

	//In case of secondary - we first send Process answer to CM with A+V
	// and only after connectivity check we send Modify_req with A only - We don't need to wait to Re invite ind.
	if (STATUS_OK == status && m_bIsChangeInICEChannels)
	{
		m_bIsChangeInICEChannels = FALSE;
		SendIceMgsReqAccordingToTargetMode(ICE_MODIFY_SESSION_OFFER_REQ);
	}

//	SendHandleIceConnectivityCheckCompleteToParty(CopyOfseg);
//	POBJDELETE(CopyOfseg);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnIceInviteModifyAnsArrivedFromIceStack(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteNoSdpInd::OnIceInviteModifyAnsArrivedFromIceStack");
	HandleReinviteforICE(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnIceReinviteContentArrivedFromIceStack(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteNoSdpInd::OnIceReinviteContentArrivedFromIceStack");
	HandleReinviteforICE(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::HandleReinviteforICE(CSegment* pParam)
{
    PTRACE(eLevelInfoNormal, "CSipTransInviteNoSdpInd::HandleReinviteforICE");
    if (IsValidTimer (ICECOMPLETETOUT))
        DeleteTimer(ICECOMPLETETOUT);
    //todo - Get the reinvite content ??

    DWORD status;
   	*pParam >> status;

	CMedString str;
	str << "CSipTransInviteNoSdpInd::HandleReinviteforICE - status: " << status << " connectivity status: " << m_pSipCntl->GetIceConnectivityStatus() << " is MOC: " << m_pSipCntl->IsRemoteMicrosoft();
	FPTRACE (eLevelInfoNormal, str.GetString());

	BOOL preserve_ice_in_local_call_flag = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_PRESERVE_ICE_CHANNEL_IN_CASE_OF_LOCAL_MODE);

	if (!preserve_ice_in_local_call_flag)
	{
	    if (status != STATUS_OK || (m_pSipCntl->GetIceConnectivityStatus() != eIceConnectedRemote && !m_pSipCntl->IsRemoteMicrosoft()))
	    {
	    	m_pSipCntl->SetIsEnableICE(FALSE);
	    	m_bNeedCloseIceChannels = TRUE;

	    	FPTRACE (eLevelInfoNormal, "The call is local, ICE channel closed");
	    }

	} else
	{
		if (status != STATUS_OK || (m_pSipCntl->GetIceConnectivityStatus() != eIceConnectedRemote && !m_pSipCntl->IsRemoteMicrosoft()))
			FPTRACE (eLevelInfoNormal, "The call is local, ICE channel preserved due to configuration");
	}

    m_bNeedReInviteForIce = TRUE;

    ContinueToCloseChannels();
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnICEReceiveCloseIceIndWaitForCandidates(CSegment* pParam)
{
	PTRACE (eLevelInfoNormal, "CSipTransInviteNoSdpInd::OnICEReceiveCloseIceIndWaitForCandidates");

	if(m_pTargetMode->GetIsTipMode())
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipMasterCenter);
	else
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone);
	m_state = sTRANS_OPENINCHANNELS;
	SetDialState(kBeforeInvite);
}

//////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnIcePortsRetryTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnIcePortsRetryTout: make offer");
	m_pSipCntl->SetIsEnableICE(TRUE);

	SendIceMgsReqAccordingToTargetMode(ICE_MAKE_OFFER_REQ);

	StartTimer(ICEOFFERTOUT, MAKE_ICE_CANDIDATES_TIMER);
}

void CSipTransInviteNoSdpInd::OnICEReceiveCloseIceInd (CSegment* pParam)
{
    PTRACE (eLevelInfoNormal, "CSipTransInviteNoSdpInd::OnICEReceiveCloseIceInd Set state sTRANS_RECOVERY");
    m_state = sTRANS_RECOVERY;
    CheckChangingInCurrentMode();
}

////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnPartyChannelsConnectedOpenInChannels(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyChannelsConnectedOpenInChannels: Name ",m_pPartyConfName);
	m_state = sTRANS_OPENBRIDGES;
	StartTimer(OPENBRIDGESTOUT, 10*SECOND);
	InformChannelsConnectedOpenBridges(pParam);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::HandleBridgeConnectedInd(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::HandleBridgeConnectedInd: Name ",m_pPartyConfName);

	if (status == STATUS_OK)
	{
		// its OK if audio, video and FECC or audio and video without FECC bridges connected or its audio only call or there is no much in the video capability
		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_isFeccBridgeConnected) ||
			(m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_pTargetMode->IsMediaOff(cmCapData,cmCapReceiveAndTransmit)) ||
			(m_isAudioBridgeConnected && GetIsVoice()) ||
			(m_isAudioBridgeConnected && m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit)))
		{
			if (IsValidTimer(OPENBRIDGESTOUT))
			{
				DeleteTimer(OPENBRIDGESTOUT);
				PTRACE(eLevelInfoNormal,"CSipTransInviteNoSdpInd::HandleBridgeConnectedInd: DeleteTimer(OPENBRIDGESTOUT) ");
			}

			if (GetDialState() == kBeforeOkInConf)
			{
				m_state		 = sTRANS_CONNECTING;
				SetDialState(kOkSent);

				m_pSipCntl->SipInviteResponseReq(OK_VAL, STATUS_OK, NULL, YES);
			}
			else
				PTRACE2INT(eLevelInfoNormal,"CSipTransInviteNoSdpInd::HandleBridgeConnectedInd: Dial State is not kBeforeOkInConf, - ",GetDialState());
		}
		else if(m_isAudioBridgeConnected)
			PTRACE2INT(eLevelInfoNormal,"CSipTransInviteNoSdpInd::HandleBridgeConnectedInd: Is Voice Call? - ",GetIsVoice());
	}
	else
	{
		DBGPASSERT(status);
		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteNoSdpInd::HandleBridgeConnectedInd: Ack with bad status - ",status);
		EndTransaction(status);
	}
}

////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: Name ",m_pPartyConfName);
	DWORD status, isSdp;
	status = isSdp = 0;
	BYTE bRemovedAudio, bRemovedVideo;
	bRemovedAudio = bRemovedVideo = 0;
	*pParam >> status >> isSdp;
	*pParam >> bRemovedAudio >> bRemovedVideo;

	if (status == STATUS_OK)
	{
		if(isSdp != 0)
		{
			// pass the product ID to party control
			GetProductIdAndSendToConfLevel();

			SendRemoteCapsReceivedToParty();

			// TIP
			if (m_pTargetMode->GetIsTipMode())
			{
			    const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
			    CCommConf*      pCommConf      = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
			    BYTE            isPreferTip    = FALSE;

			    if (pCommConf && pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP)
			    {
			        isPreferTip = TRUE;
			        PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting PREFER TIP FOR POLYCOM EP");
			    }
			    else
			    {
			        PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting !DONOT! PREFER TIP FOR POLYCOM EP");
			        if (pCommConf)
			            PTRACE2INT(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting !DONOT! PREFER TIP FOR POLYCOM EP TipCompatible:",pCommConf->GetIsTipCompatible());
			        else
			            PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting pCommConf is NULL");
			    }

			    if ( (!::CheckIfRemoteSdpIsTipCompatible(pCurRemoteCaps) && isPreferTip==FALSE) || !(pCurRemoteCaps->GetIsContainingCapCode(cmCapAudio,eAAC_LDCapCode)) ) //if (!CheckIfRemoteSdpIsTipCompatible())
				{
					PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting fall back from TIP to SIP: Name ",m_pPartyConfName);

					m_bNeedReInviteForSwitchToNoneTipCall = TRUE;

					FallbackFromTipToNoneTip();
				}
				else if (!CheckIfRemoteVideoRateIsTipCompatible() || IsNeedToRejectTheCallForPreferTIPmode(pCurRemoteCaps))
				{
					PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting drop the call: Name ",m_pPartyConfName);
					m_bIsNeedToDropCall = TRUE;
					m_pTargetMode->SetTipMode(eTipModeNone);

					m_pTargetModeMaxAllocation->SetTipMode(eTipModeNone);
				}
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
				BOOL bIsUndefinedParty = m_pSipCntl->IsUndefinedParty();
				BYTE bIsDisconnectOnEncFailure = pBestMode->GetIsDisconnectOnEncryptionFailure();
				BOOL bIsWhenAvailableEncMode = m_pSipCntl->IsWhenAvailableEncryptionMode();
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
									PTRACE2(eLevelError,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: set encryption to OFF. Name ",m_pPartyConfName);
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
									CSmallString log;
									log<<"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: SDES cap is incorrect or not matching. Name "<<m_pPartyConfName;
									log << ", bIsUndefinedParty:"<<bIsUndefinedParty<<", bIsDisconnectOnEncFailure:"<<(bIsDisconnectOnEncFailure==YES)<<", bIsWhenAvailableEncMode"<<(bIsWhenAvailableEncMode);
									PTRACE(eLevelError,log.GetString());
									SetDialState(kNoRecovery);
									EndTransaction(SIP_CAPS_DONT_MATCH);
									return;
								}
							}
							else if(GetIsRemoteIdentMicrosoft())
							{
								POBJDELETE(pBestMode);
								DBGPASSERT(YES);
								CSmallString log;
								log<<"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: MS-ICE remote is Microsoft - encryption not supported - must reject call! . Name "<<m_pPartyConfName;
								PTRACE(eLevelError,log.GetString());
								SetDialState(kNoRecovery);
								EndTransaction(SIP_CAPS_DONT_MATCH);
								return;
							}
							else
							{
								UpdateLocalCapsWithEncryptionParameters(pBestMode, mediaType , eRole); //BRIDGE-10820
							}
						}
						else
						{
							if (!pBestMode->GetSipSdes(cmCapAudio,cmCapReceive))
							{
								if ((mediaType == cmCapVideo) && (eRole & kRoleContentOrPresentation))
									m_pSipCntl->RemoveSdesCapFromLocalCaps(mediaType, eRole);
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
			}

			if(IsNoRecovery(pBestMode))
			{
				PTRACE2(eLevelError,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: Audio remote receive is not matching. Name ",m_pPartyConfName);
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
				PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: Guess succeeded. Name ",m_pPartyConfName);
				SetDialState(kGuessSucceeded);	// this state indicates now that media can be establish, maybe after internal change.
			}

			m_bIsToCloseVideoChannels = FALSE;
			if(IsNoRecoveryForVideo(pBestMode))
			{
				if(GetDialState() == kGuessSucceeded)
					SetDialState(kNoRecoveryForVideo);
				m_bIsToCloseVideoChannels = TRUE;
				const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
				if (pCurRemoteCaps->IsMedia(cmCapVideo))
					m_bNeedReInviteForSecondary = TRUE;
			}

			m_bIsToCloseDataChannels = FALSE;
			if(IsNoRecoveryForData(pBestMode))
			{
				m_bIsToCloseDataChannels = TRUE;
			}

			if (IsNoRecoveryForBfcp(pBestMode))
			{
				m_bIsToCloseBfcpChannels = TRUE;
			}

			if(isEncrypted == Encryp_On )
				m_bNeedCloseSrtpChannels = TRUE;

			// update media from best mode to target mode
			if (pBestMode)
			{

		//	if (pBestMode && pBestMode->IsMediaOn(cmCapAudio,cmCapReceive) && pBestMode->IsMediaOn(cmCapAudio,cmCapTransmit))
				m_pTargetMode->CopyMediaMode(*pBestMode,cmCapAudio,cmCapReceiveAndTransmit);
		//	if (pBestMode && pBestMode->IsMediaOn(cmCapVideo,cmCapReceive) && pBestMode->IsMediaOn(cmCapVideo,cmCapTransmit))
				m_pTargetMode->CopyMediaMode(*pBestMode,cmCapVideo,cmCapReceiveAndTransmit);
				m_pTargetMode->CopyMediaMode(*pBestMode,cmCapData,cmCapReceiveAndTransmit);
				m_pTargetMode->CopyMediaMode(*pBestMode,cmCapBfcp,cmCapReceiveAndTransmit);

				if (m_pTargetMode->IsMediaOff(cmCapVideo,cmCapTransmit,kRolePeople))
				{
					PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: audio only removing bfcp -no content. Name ",m_pPartyConfName);
					m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
					m_bNeedReInviteForBfcp = FALSE;
				}
			}

			if (GetDialState() != kNotInDialState)
			{
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
					m_state = sTRANS_CHANGECHANNELS;

					CloseChannelsIfNeeded(m_bIsToCloseVideoChannels, m_bIsToCloseDataChannels, m_bIsToCloseBfcpChannels);
				}

				POBJDELETE(pBestMode); //YG - do we need it if ICE?
				return;
			}
		}
		else
		{
			PTRACE2INT(eLevelError,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: Ack without SDP - ",status);
			SetDialState(kNotInDialState);
		}
	}
	else
	{
		SetDialState(kBadStatusAckArrived);
		DBGPASSERT(status);
		PTRACE2INT(eLevelError,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting: Ack with bad status - ",status);
	}

	EndTransaction(SIP_CAPS_DONT_MATCH);
}

///////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnPartyChannelsDisconnectedChangeChannels(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyChannelsDisconnectedChangeChannels");
    if(m_bNeedCloseIceChannels)
	{
        UpdateDbChannelsStatus(pParam, FALSE);
		m_pSipCntl->CloseIceSession();
	}
    else
    {
        m_state = sTRANS_RECOVERY;
        HandleChannelsDisconnectedStartRecovery(pParam);
    }
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnPartyChannelsUpdatedRecovery(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyChannelsUpdatedRecovery");
	HandleChannelsUpdatedDuringRecovery(pParam);
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::InternalRecoveryCompleted()
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteNoSdpInd::InternalRecoveryCompleted");
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
void CSipTransInviteNoSdpInd::OnConfConnectCallRmtConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnConfConnectCallRmtConnected: Name ",m_pPartyConfName);
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
void CSipTransInviteNoSdpInd::OnPartyChannelsConnectedOpenOut(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyChannelsConnectedOpenOut");
	HandleOutChannelsConnected(pParam);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::PartyConnectCall()
{
	switch(GetDialState())
	{
		case kGuessSucceeded:
		case kNoRecoveryForVideo:
			EndTransaction();
			break;
		case kNotInDialState:
			PTRACE2INT(eLevelInfoNormal, "CSipTransInviteNoSdpInd::PartyConnectCall: Do nothing on dial state Name ", GetDialState());  // there is nothing to do
			break;
		default:
			DBGPASSERT(GetDialState());
	}
}


/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnConfBridgesConnectionTout(CSegment* pParam)
{
	PASSERTMSG((DWORD)statIllegal,"CSipTransInviteNoSdpInd::OnConfBridgesConnectionTout");
	EndTransaction(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnUpdateBridgesTout(CSegment* pParam)
{
	PASSERTMSG((DWORD)statIllegal,"CSipTransInviteNoSdpInd::OnUpdateBridgesTout");
	EndTransaction(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnConfSetCapsAccordingToNewAllocation(CSegment* pParam)
{
	SetCapsAccordingToNewAllocation(pParam);
	m_bNeedReInviteForReAlloc = TRUE;
}

//////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnPartySlavesRecapIsFinished(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteNoSdpInd::OnPartySlavesRecapIsFinished");
}

////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam)
{
	PTRACE2INT(eLevelError,"CSipTransInviteNoSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels - error - shouldn't happen in this state - m_state:", m_state);
//	CloseChannelsIfNeeded(m_bIsToCloseVideoChannels, m_bIsToCloseDataChannels, m_bIsToCloseBfcpChannels);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteNoSdpInd::ContinueToCloseChannels()
{
     m_state = sTRANS_CHANGECHANNELS;
     CloseChannelsIfNeeded(m_bIsToCloseVideoChannels, m_bIsToCloseDataChannels, m_bIsToCloseBfcpChannels, FALSE, FALSE, FALSE);
}
