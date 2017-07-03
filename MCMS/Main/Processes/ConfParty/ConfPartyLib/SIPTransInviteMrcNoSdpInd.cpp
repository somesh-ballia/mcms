//+========================================================================+
//               SIPTransInviteMrcNoSdpInd.h 				          	   |
//            Copyright 2012 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteMrcNoSdpInd.cpp                         	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include "Segment.h"
#include "StateMachine.h"
#include "Trace.h"
#include "TaskApi.h"
#include "PartyApi.h"
#include "SIPCommon.h"
#include "SipScm.h"
#include "SipCall.h"
#include "ConfApi.h"
#include "SIPControl.h"
#include "SIPParty.h"
#include "IPParty.h"
#include "SIPTransaction.h"
#include "CommConfDB.h"
#include "SIPTransInviteMrcNoSdpInd.h"

////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransInviteMrcNoSdpInd)

	ONEVENT(SIP_PARTY_ESTABLISH_CALL,			IDLE,					CSipTransInviteMrcNoSdpInd::OnPartyEstablishCallIdle)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENINCHANNELS,	CSipTransInviteMrcInd::OnPartyChannelsConnectedOpenIn)

	ONEVENT(AUDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveAudBridgeConnected)
	ONEVENT(VIDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveVidBridgeConnected)
	ONEVENT(FECCBRDGCONNECT,					sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveFeccBridgeConnected)

	ONEVENT(SIP_PARTY_ORIGINAL_RMOTCAP,			sTRANS_CONNECTING,		CSipTransaction::OnPartyOriginalRemoteCaps)
	ONEVENT(SIP_PARTY_RECEIVED_ACK,				sTRANS_CONNECTING,		CSipTransInviteMrcNoSdpInd::OnPartyReceivedAckConnecting)
	ONEVENT(PARTYCONNECTTOUT,					sTRANS_CONNECTING,		CSipTransInviteMrcNoSdpInd::OnPartyConnectToutConnecting)
	ONEVENT(SET_SITE_AND_VISUAL_NAME,			sTRANS_CONNECTING,		CSipTransaction::OnPartySendSiteAndVisualNamePlusProductIdToPartyControl)

	ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,		sTRANS_CHANGECHANNELS,	CSipTransInviteMrcInd::OnPartyChannelsDisconnectedChangeChannels)

	ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_RECOVERY,		CSipTransInviteMrcInd::OnPartyChannelsUpdatedRecovery)

	ONEVENT(SIP_CONF_CONNECT_CALL,				sTRANS_RMTCONNECTED,	CSipTransInviteMrcInd::OnConfConnectCallRmtConnected)
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENOUTCHANNELS,	CSipTransInviteMrcInd::OnPartyChannelsConnectedOpenOut)
	ONEVENT(OPENBRIDGESTOUT,                    sTRANS_OPENBRIDGES,     CSipTransInviteMrcInd::OnConfBridgesConnectionTout)
	// Ice
	ONEVENT(MAKE_OFFER_IND,						sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcNoSdpInd::OnIceInviteReceiveMakeOfferInd)
	ONEVENT(ICE_PROCESS_ANS_IND,				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnIceInviteProcessAnsArrivedFromIceStack)
	ONEVENT(ICE_REINVITE_IND,					sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnIceReinviteContentArrivedFromIceStack)
	ONEVENT(ICE_MODIFY_OFFER_IND,				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnIceInviteModifyAnsArrivedFromIceStack)
	ONEVENT(ICECOMPLETETOUT,	  				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnICETimeout)
	ONEVENT(ICEGENERALTOUT,		  				sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnICETimeout)
	ONEVENT(ICEOFFERTOUT,	  				    sTRANS_WAITFORICECANDIDATES,	CSipTransInviteMrcInd::OnICEOfferTimeout)
	ONEVENT(CLOSE_ICE_SESSION_IND,				sTRANS_WAITFORICECANDIDATES,    CSipTransInviteMrcInd::OnICEReceiveCloseIceIndWaitForCandidates)
	ONEVENT(ICEPORTSRETRYTOUT,					sTRANS_WAITFORICECANDIDATES,    CSipTransInviteMrcInd::OnIcePortsRetryTout)
	ONEVENT(TRANS_ICE_CONN_CHECK_COMPLETE_IND,	ANYCASE,                        CSipTransInviteMrcInd::IceConnectivityCheckComplete)
	ONEVENT(CLOSE_ICE_SESSION_IND,				sTRANS_CHANGECHANNELS,			CSipTransInviteMrcNoSdpInd::OnICEReceiveCloseIceInd)

PEND_MESSAGE_MAP(CSipTransInviteMrcNoSdpInd, CSipTransInviteMrcInd);

////////////////////////////////////////////////////////////////////////////
CSipTransInviteMrcNoSdpInd::CSipTransInviteMrcNoSdpInd(CTaskApp *pOwnerTask):CSipTransInviteMrcInd(pOwnerTask)
{
}

////////////////////////////////////////////////////////////////////////////
CSipTransInviteMrcNoSdpInd::~CSipTransInviteMrcNoSdpInd()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcNoSdpInd::OnPartyEstablishCallIdle(CSegment* pParam)
{
	TRACEINTO << "Name - " << m_pPartyConfName;

	InitPartyEstablishCallIdle();


	BYTE bIsIceFlow = FALSE;
	if(m_pSipCntl->GetIsEnableICE())
	{
		if( SendIceMgsReqAccordingToTargetMode(ICE_MAKE_OFFER_REQ) == STATUS_OK )
		{
			PTRACE(eLevelInfoNormal,"CSipTransInviteMrcNoSdpInd::OnPartyEstablishCallIdle - ICE is enabled   ");
			m_state = sTRANS_WAITFORICECANDIDATES;
			StartTimer(ICEOFFERTOUT, MAKE_ICE_CANDIDATES_TIMER);
			bIsIceFlow = TRUE;
		}
		else
			PTRACE(eLevelInfoNormal,"CSipTransInviteMrcNoSdpInd::OnPartyEstablishCallIdle - Failed to start ice call ");
	}

	if (!bIsIceFlow)
	{
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone, TRUE);
		m_state = sTRANS_OPENINCHANNELS;
	}

	SetDialState(kBeforeInvite);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcNoSdpInd::ContinueHandleBridgeConnectedInd()
{
	TRACEINTO << "Name - " << m_pPartyConfName;

//	m_pSipCntl->RemoveUnsupportedSdesCapsForMrcCall();
	SetDialState(kOkSent);
	m_pSipCntl->SipInviteResponseReq(OK_VAL, STATUS_OK, NULL, TRUE);
}


//////////////////////////////////////////////////////////////////
void CSipTransInviteMrcNoSdpInd::OnPartyReceivedAckConnecting(CSegment * pParam)
{
	DWORD status = 0,
		  isSdp = 0;

	BYTE bRemovedAudio = 0,
		 bRemovedVideo = 0;

	*pParam >> status
			>> isSdp
			>> bRemovedAudio
			>> bRemovedVideo;

	TRACEINTO << "Name - " << m_pPartyConfName
			  << ", status: " << status << ", isSdp: " << isSdp << ", bRemovedAudio: " << bRemovedAudio << ", bRemovedVideo: " << bRemovedVideo;

	if (status == STATUS_OK)
	{
		if(isSdp != 0)
		{
			ContinueOnPartyReceivedReinviteResponseOrAckConnecting();
		}

		else // isSdp == 0
		{
			PTRACE2INT(eLevelError,"CSipTransInviteMrcNoSdpInd::OnPartyReceivedAckConnecting: Ack without SDP - ",status);
			SetDialState(kNotInDialState);
		}

	}

	else // status != STATUS_OK
	{
		SetDialState(kBadStatusAckArrived);
		DBGPASSERT(status);
		PTRACE2INT(eLevelError,"CSipTransInviteMrcNoSdpInd::OnPartyReceivedAckConnecting: Ack with bad status - ",status);
	}
}
