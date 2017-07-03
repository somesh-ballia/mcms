//+========================================================================+
//               SIPTransInviteMrcWithSdpInd.h 				          	   |
//            Copyright 2012 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteMrcWithSdpInd.cpp                         	   |
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
#include "SIPTransInviteMrcWithSdpInd.h"
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransInviteMrcWithSdpInd)

	ONEVENT(SIP_PARTY_ESTABLISH_CALL,			IDLE,						CSipTransInviteMrcWithSdpInd::OnPartyEstablishCallIdle)
	ONEVENT(SIP_PARTY_RECEIVED_ACK,				sTRANS_200OK_NO_MEDIA_SENT, CSipTransInviteMrcWithSdpInd::OnPartyReceivedAck200OkNoMediaSent)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENINCHANNELS,		CSipTransInviteMrcInd::OnPartyChannelsConnectedOpenIn)

	ONEVENT(AUDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveAudBridgeConnected)
	ONEVENT(VIDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveVidBridgeConnected)
	ONEVENT(FECCBRDGCONNECT,					sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveFeccBridgeConnected)

	ONEVENT(SIP_PARTY_ORIGINAL_RMOTCAP,			sTRANS_CONNECTING,		CSipTransaction::OnPartyOriginalRemoteCaps)
	ONEVENT(SIP_PARTY_REINVITE_RESPONSE,		sTRANS_CONNECTING,		CSipTransInviteMrcWithSdpInd::OnPartyReceivedReinviteResponseConnecting)
	ONEVENT(PARTYCONNECTTOUT,					sTRANS_CONNECTING,		CSipTransInviteMrcWithSdpInd::OnPartyConnectToutConnecting)
	ONEVENT(SET_SITE_AND_VISUAL_NAME,			sTRANS_CONNECTING,		CSipTransaction::OnPartySendSiteAndVisualNamePlusProductIdToPartyControl)

	ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,		sTRANS_CHANGECHANNELS,	CSipTransInviteMrcInd::OnPartyChannelsDisconnectedChangeChannels)

	ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_RECOVERY,		CSipTransInviteMrcInd::OnPartyChannelsUpdatedRecovery)

	ONEVENT(SIP_CONF_CONNECT_CALL,				sTRANS_RMTCONNECTED,	CSipTransInviteMrcInd::OnConfConnectCallRmtConnected)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENOUTCHANNELS,	CSipTransInviteMrcInd::OnPartyChannelsConnectedOpenOut)
	ONEVENT(OPENBRIDGESTOUT,                    sTRANS_OPENBRIDGES,     CSipTransInviteMrcInd::OnConfBridgesConnectionTout)
	// Ice
	ONEVENT(MAKE_OFFER_IND,						sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcWithSdpInd::OnIceInviteReceiveMakeOfferInd)
	ONEVENT(ICE_PROCESS_ANS_IND,				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnIceInviteProcessAnsArrivedFromIceStack)
	ONEVENT(ICE_REINVITE_IND,					sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnIceReinviteContentArrivedFromIceStack)
	ONEVENT(ICE_MODIFY_OFFER_IND,				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnIceInviteModifyAnsArrivedFromIceStack)
	ONEVENT(ICECOMPLETETOUT,	  				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnICETimeout)
	ONEVENT(ICEGENERALTOUT,		  				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnICETimeout)
	ONEVENT(ICEOFFERTOUT,	  				    sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnICEOfferTimeout)
	ONEVENT(CLOSE_ICE_SESSION_IND,				sTRANS_WAITFORICECANDIDATES,    CSipTransInviteMrcInd::OnICEReceiveCloseIceIndWaitForCandidates)
	ONEVENT(ICEPORTSRETRYTOUT,					sTRANS_WAITFORICECANDIDATES,    CSipTransInviteMrcInd::OnIcePortsRetryTout)
	ONEVENT(TRANS_ICE_CONN_CHECK_COMPLETE_IND,	ANYCASE,                        CSipTransInviteMrcInd::IceConnectivityCheckComplete)
	ONEVENT(CLOSE_ICE_SESSION_IND,				sTRANS_CHANGECHANNELS,			CSipTransInviteMrcWithSdpInd::OnICEReceiveCloseIceInd)

	ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,		sTRANS_CLOSE_CHANNELS_BEFORE_PROCESS_ANSWER, CSipTransInviteMrcWithSdpInd::OnPartyChannelsDisconnectedBeforeProcessAnswer)

PEND_MESSAGE_MAP(CSipTransInviteMrcWithSdpInd, CSipTransaction);

/////////////////////////////////////////////////////////////////////////////////////////
CSipTransInviteMrcWithSdpInd::CSipTransInviteMrcWithSdpInd(CTaskApp *pOwnerTask):CSipTransInviteMrcInd(pOwnerTask)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
CSipTransInviteMrcWithSdpInd::~CSipTransInviteMrcWithSdpInd()
{

}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcWithSdpInd::OnPartyEstablishCallIdle(CSegment* pParam)
{
	TRACEINTO << "Name - " << m_pPartyConfName;

	InitPartyEstablishCallIdle();


	//BRIDGE-4244 if remote does not support ICE mark it now for the re-invite to be with no ICE,
	sipSdpAndHeadersSt* pRemoteSdp = m_pSipCntl->GetRemoteSdp();
	CSipCaps* pRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
	if (pRemoteSdp && pRemoteCaps && !pRemoteCaps->IsRemoteSdpContainsICE(pRemoteSdp))
	{
		PTRACE(eLevelInfoNormal,"CSipTransInviteMrcWithSdpInd::OnPartyEstablishCallIdle: Remote SDP does not contain ICE");
		m_pSipCntl->SetIceCallOnNetSetup(FALSE);
		m_pSipCntl->SetIsEnableICE(FALSE);
	}
	else
		PTRACE(eLevelInfoNormal,"CSipTransInviteMrcWithSdpInd::OnPartyEstablishCallIdle: Remote SDP contains ICE");

	WORD  bIsMrcHeader	 = ::IsMrcHeader(m_pSipCntl->GetRemoteSdp());

	if (!bIsMrcHeader)
	{
		const char *pRemoteSipContact = (m_pSipCntl->GetNetSetup()) ? m_pSipCntl->GetNetSetup()->GetRemoteSipContact() : 0;

		if (pRemoteSipContact && strstr(pRemoteSipContact, ".DMA_VMR.")) //Send the customized warning headers just in cases when the SIP INVITE is coming from a DMA
		{
			const char* warningStr = "[10400] Call rejected: Non-SVC endpoint is not permitted in SVC-only conference.";
			m_pSipCntl->GetCallObj()->SetWarning(SipWarningMiscellaneous);
			m_pSipCntl->GetCallObj()->SetWarningString(warningStr);
		}

		SetDialState(kCapsDontMatch);
		EndTransaction(SIP_CAPS_DONT_MATCH);
		return;
	}

	BYTE isBwmForIBM = (CProcessBase::GetProcess()->GetProductType() == eProductTypeSoftMCUMfw) ? YES : NO;


	m_pSipCntl->SipInviteResponseReq(OK_VAL, STATUS_OK, NULL, FALSE, isBwmForIBM);

	m_state = sTRANS_200OK_NO_MEDIA_SENT;
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcWithSdpInd::OnPartyReceivedAck200OkNoMediaSent(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteMrcWithSdpInd::OnPartyReceivedAck200OkNoMediaSent: Name-", m_pPartyConfName);
	BYTE bIsIceFlow = FALSE;

	if (m_pSipCntl->GetIsEnableICE())
	{
		if (SendIceMgsReqAccordingToTargetMode(ICE_MAKE_OFFER_REQ) == STATUS_OK)
		{
			PTRACE(eLevelInfoNormal,"CSipTransInviteMrcWithSdpInd::OnPartyReceivedAck200OkNoMediaSent - ICE is enabled   ");
			m_state = sTRANS_WAITFORICECANDIDATES;
			StartTimer(ICEOFFERTOUT, MAKE_ICE_CANDIDATES_TIMER);
			bIsIceFlow = TRUE;
		}
		else
			PTRACE(eLevelInfoNormal,"CSipTransInviteMrcWithSdpInd::OnPartyReceivedAck200OkNoMediaSent - Failed to start ice call ");
	}

	if (!bIsIceFlow)
	{
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone, TRUE);
		m_state = sTRANS_OPENINCHANNELS;
	}
	
	SetDialState(kBeforeInvite);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcWithSdpInd::ContinueHandleBridgeConnectedInd()
{
	TRACEINTO << "Name - " << m_pPartyConfName;

	SetDialState(kInviteSent);
	m_pSipCntl->SipReInviteReq(0, TRUE);
}


//////////////////////////////////////////////////////////////////
void CSipTransInviteMrcWithSdpInd::OnPartyReceivedReinviteResponseConnecting(CSegment* pParam)
{
	BYTE bRemovedAudio = 0,
		 bRemovedVideo = NO;

	*pParam >> bRemovedAudio
			>> bRemovedVideo;


	TRACEINTO << "Name - " << m_pPartyConfName
			  << ", bRemovedAudio: " << bRemovedAudio << ", bRemovedVideo: " << bRemovedVideo;

	ContinueOnPartyReceivedReinviteResponseOrAckConnecting();
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcWithSdpInd::OnPartyChannelsDisconnectedBeforeProcessAnswer(CSegment* pParam)
{
	TRACEINTO << m_pPartyConfName;

	WORD Status = SendIceMgsReqAccordingToTargetModeAndCurrentMode(ICE_PROCESS_ANSWER_REQ);

	UpdateDbChannelsStatus(pParam, FALSE); // Important  - call this only after the process answer request, because this function need the previous current mode (before the close channels).

	if (Status == STATUS_OK)
	{
		m_state = sTRANS_WAITFORICECANDIDATES;
		StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
	}
	else
	{
		PTRACE(eLevelInfoNormal, "ICE-SVC: CSipTransInviteMrcWithSdpInd::OnPartyChannelsDisconnectedBeforeProcessAnswer : Currently we don't support ReInvite for SVC call. continue the flow without reinvite");
		m_bNeedReInviteForIce = TRUE;
		m_pSipCntl->SetIsEnableICE(FALSE);
		m_pSipCntl->SetNeedUpdateIceToNonIce(TRUE);
		ContinueToCloseChannels();
	}

}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcWithSdpInd::SipInviteAckReqIfNeeded()
{
	m_pSipCntl->SipInviteAckReq();
}
