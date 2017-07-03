#if 0
//+========================================================================+
//              SIPTransReInviteMrcWithSdpInd.cpp           	 			   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransReInviteMrcWithSdpInd.cpp                          	   |
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
#include "Macros.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "NStream.h"
#include "NetSetup.h"
#include "StatusesGeneral.h"
#include "Conf.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"
#include "TaskApi.h"
#include "Party.h"
#include "PartyApi.h"
#include "CsInterface.h"
#include "SipDefinitions.h"
#include "SIPCommon.h"
#include "IpNetSetup.h"
#include "SipNetSetup.h"
#include "SipScm.h"
#include "SipCall.h"
#include "SIPControl.h"
#include "SystemFunctions.h"
#include "SIPTransaction.h"
#include "SIPTransReInviteMrcWithSdpInd.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "SIPParty.h"


PBEGIN_MESSAGE_MAP(CSipTransReInviteMrcWithSdpInd)

// handle re-caps (Remote Re-Invite)
ONEVENT(SIP_PARTY_RECEIVED_REINVITE,	IDLE,										CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected)
ONEVENT(SIP_CONF_BRIDGES_UPDATED,		sTRANS_RECREINVITEUPDATEBRIDGE,				CSipTransReInviteMrcWithSdpInd::OnConfBridgesUpdatedUpdateBridges)
ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,	sTRANS_RECREINVITECLOSECHANN,				CSipTransReInviteMrcWithSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann)
ONEVENT(SIP_PARTY_CHANS_UPDATED,		sTRANS_RECREINVITEUPDATECHANN,				CSipTransReInviteMrcWithSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,		sTRANS_RECREINVITEOPENCHANN,				CSipTransReInviteMrcWithSdpInd::OnPartyChannelsOpenRecReInviteOpenChann)

ONEVENT(SIP_PARTY_SLAVES_RECAP_FINISHED,ANYCASE,									CSipTransReInviteMrcWithSdpInd::OnPartySlavesRecapIsFinished)

ONEVENT(SIP_PARTY_RECEIVED_ACK,			sTRANS_RECREINVITE,							CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteAck)
//ICE
ONEVENT(ICE_MODIFY_ANSWER_IND,		    sTRANS_WAITFORICECANDIDATES,				CSipTransReInviteMrcWithSdpInd::OnSipReceiveReinviteModifyAnswerInd)
ONEVENT(ICEGENERALTOUT,				    sTRANS_WAITFORICECANDIDATES,				CSipTransReInviteMrcWithSdpInd::OnICEReinviteGeneralTimeout)
ONEVENT(CLOSE_ICE_SESSION_IND,          sTRANS_RECREINVITECLOSECHANN,		     	CSipTransReInviteMrcWithSdpInd::OnICEReceiveCloseIceInd)

// Reject
ONEVENT(SIP_PARTY_RECEIVED_ACK,			sTRANS_RECREINVITEREJECTED,					CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteAckReInviteRejected)

// timeout
ONEVENT(UPDATEBRIDGESTOUT,				sTRANS_RECREINVITEUPDATEBRIDGE, 			CSipTransReInviteMrcWithSdpInd::OnReInviteUpdateBridgesTout)
//DPA
ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION,ANYCASE,				CSipTransReInviteMrcWithSdpInd::OnConfSetCapsAccordingToNewAllocation)

PEND_MESSAGE_MAP(CSipTransReInviteMrcWithSdpInd, CSipTransaction);


///////////////////////////////////////////////////////
CSipTransReInviteMrcWithSdpInd::CSipTransReInviteMrcWithSdpInd(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_retStatusForReject 		= STATUS_OK;
	m_bRollbackNeeded 			= FALSE;
	m_bIsOfferer 				= FALSE;
	m_bIsReInvite 				= TRUE;

	m_bNeedCloseIceChannels 	= FALSE;
	m_bIsAllChannelsAreOpenend 	= FALSE;

	VALIDATEMESSAGEMAP
}


///////////////////////////////////////////////////////
CSipTransReInviteMrcWithSdpInd::~CSipTransReInviteMrcWithSdpInd()
{
}

////////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected(CSegment* pParam)
{
	// We reject the re-invite only IF the remote SDP contians valid caps for media to be transmit, but there is no match with our local caps.
	// more specific: if no match in A+V, or no match in A, if we have no match in V but we have match in A we will make the call secondary.
	// E.g. remote has H261 video only and the local is H263.
	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected");

	BYTE	isRejectReInvite;
	*pParam >> isRejectReInvite;

	if (isRejectReInvite)
	{
		PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected - Reject remote ReInvite");
		m_state = sTRANS_RECREINVITEREJECTED;
		SetDialState(kReInviteRejected);
		m_retStatusForReject = STATUS_OK;
		m_pSipCntl->SipInviteResponseReq(SipCodesNotAcceptedInHere, SipWarningMediaTypeNotAvail);
	}

	// no RejectReInvite
	else
	{
		WORD callIndex = m_pSipCntl->GetCallIndex();

		CSipCaps*	pCurRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());

		POBJDELETE(m_pChanDifArr);
		m_pChanDifArr	= new CSipChanDifArr;

		m_pChanDifArr->DeSerialize(NATIVE,*pParam);
		SetDialState(kReInviteArrived);

		SendOriginalRemoteCapsToParty(pCurRemoteCaps);

		// TIP - Recognition if remote resume the media
		if (m_pParty->GetIsTipCall())
		{
			UpdateIfCallIsResumed();
			const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();

			if (!::CheckIfRemoteSdpIsTipCompatible(pCurRemoteCaps)  && m_bIsResumeMedia)
            {
            	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected -fall back to none TIP");
            	FallbackFromTipToNoneTip();
            	m_pParty->SetIsTipCall(FALSE);
            }
		}

		CSipComMode* pBestMode =NULL;

		if(m_pTargetMode->GetConfType() == kCp || m_pTargetMode->GetConfType() == kVSW_Fixed || m_pTargetMode->GetConfType() == kCop
					|| m_pTargetMode->GetConfType() == kVswRelayAvc) //jason zhu for VNGR-24428
		{
			TRACEINTO << "using m_pTargetModeMaxAllocation";
			pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetModeMaxAllocation, TRUE,TRUE/*intersect with max caps*/);  //DPA
		}
		else
		{
			TRACEINTO << "using m_pTargetMode";
			pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, TRUE);
		}


		if(pBestMode)
		{
			if(m_pTargetModeMaxAllocation->GetIsEncrypted() == Encryp_On )
				m_pSipCntl->UpdateLocalCapsSdesTag(pBestMode);

			if (m_pTargetModeMaxAllocation->GetConfType()==kCop && (m_pTargetModeMaxAllocation->GetCopTxLevel()!=pBestMode->GetCopTxLevel()))
			{
				CSipChanDif* pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapTransmit);

				if (pChanDif)
				    pChanDif->SetChangeAlg(YES);
				else
				    PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected - pChanDif is NULL");
			}

			BYTE bChangeInMedia = YES;

			if ((m_pTargetMode->IsMediaEquals(*pBestMode, cmCapAudio, cmCapReceiveAndTransmit))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapData, cmCapReceiveAndTransmit))
					&& !(m_pTargetMode->GetConfType()==kCop && (m_pTargetMode->GetCopTxLevel()!=pBestMode->GetCopTxLevel()))
					&& (m_pTargetMode->GetConfType()!=kVswRelayAvc) )
				bChangeInMedia = NO;

			if (bChangeInMedia)
			{
				bool bEqual = m_pTargetMode->IsMediaEquals(*pBestMode, cmCapAudio, cmCapReceiveAndTransmit);
				TRACEINTO << "m_pTargetMode->IsMediaEquals(*pBestMode, cmCapAudio, cmCapReceiveAndTransmit): " << (bEqual? "true" : "false");

				bEqual = m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit);
				TRACEINTO << "m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit): " << (bEqual? "true" : "false");

				bEqual = m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
				TRACEINTO << "m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation): " << (bEqual? "true" : "false");

				bEqual = m_pTargetMode->IsMediaEquals(*pBestMode, cmCapData, cmCapReceiveAndTransmit);
				TRACEINTO << "m_pTargetMode->IsMediaEquals(*pBestMode, cmCapData, cmCapReceiveAndTransmit): " << (bEqual? "true" : "false");
			}



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

			if (bChangeInMedia)
			{
				PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected: update party control on new caps (update bridges)");
				m_state = sTRANS_RECREINVITEUPDATEBRIDGE;
				StartTimer(UPDATEBRIDGESTOUT, BRIDGES_TIME * SECOND);

				// this brings to UpdateBridge (in CSipChangeModePartyCntl::StartReCaps)
				SendReCapsReceivedToParty(pCurRemoteCaps, pBestMode);
			}

			if(!bChangeInMedia)
			{
				if(m_pSipCntl->GetIsEnableICE())
				{
					PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected - Wait for Candidates before continue transaction.  ");
                    if(m_pSipCntl->GetIsEnableICE())
                    {
                        if (STATUS_OK == SendIceMgsReqAccordingToTargetModeAndDiffArr(ICE_MODIFY_SESSION_ANSWER_REQ,bChangeInMedia))
                        {

                            PTRACE(eLevelInfoNormal,"CSipTransaction::IsIceActionsNeeded .  ");
                            m_state = sTRANS_WAITFORICECANDIDATES;
                            StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
                        }
                        else
                        {
                            PTRACE2(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnSipReceiveReinviteModifyAnswerInd - Modify Reinvite answer failed : Name ",m_pPartyConfName);
                            m_bNeedCloseIceChannels = TRUE;
                            ContinueToCloseChannelsIfNeeded();
                        }

                    }
				}
				else
				{
					PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected: no change in bridges. continue to close channels stage");
					m_state = sTRANS_RECREINVITECLOSECHANN;
					CloseChannelsIfNeededForReceiveReInvite();
				}
			}

			// update bfcp in case remote decalres on it only in reinvite
		}
		else
		{// if the best mode is null, no common codec (the case of port=0 is with pBestMode without and media set in to it).
			// reject the transaction
			PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteConnected: no common mode. reject re-invite");
			m_state = sTRANS_RECREINVITEREJECTED;
			SetDialState(kReInviteRejected);
			m_bRollbackNeeded = TRUE; // return remote caps to the previous remote caps
			m_retStatusForReject = STATUS_OK;
			m_pSipCntl->SipInviteResponseReq(SipCodesNotAcceptedInHere, SipWarningMediaTypeNotAvail);
		}

		POBJDELETE(pBestMode);

	} // end no RejectReInvite
}

///////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnSipReceiveReinviteModifyAnswerInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnSipReceiveReinviteModifyAnswerInd");

	WORD Status = STATUS_FAIL;
	DWORD newRate = 0;

	*pParam >> Status;
	*pParam >> newRate;

	if (IsValidTimer(ICEGENERALTOUT))
		DeleteTimer(ICEGENERALTOUT);

	/* vngr-25846 do not update the rate in case of vsw*/
	if( newRate != 0 && m_pTargetMode->GetConfType() == kCp && (newRate < m_pTargetMode->GetVideoBitRate(cmCapReceiveAndTransmit, kRolePeople)))
		m_pTargetMode->SetVideoBitRate(newRate,cmCapReceiveAndTransmit);

	if(Status != STATUS_OK )
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnSipReceiveReinviteModifyAnswerInd - Modify Reinvite answer failed : Name ",m_pPartyConfName);
		m_bNeedCloseIceChannels = TRUE;
		ContinueToCloseChannelsIfNeeded();
	}
	else
	{
		m_state = sTRANS_RECREINVITEUPDATECHANN;
		UpdateChannelsIfNeeded();
	}
}
///////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnICEReinviteGeneralTimeout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnICEReinviteGeneralTimeout: Name ",m_pPartyConfName);
	m_pSipCntl->SetIsEnableICE(FALSE);
	m_bNeedCloseIceChannels = TRUE;

	ContinueToCloseChannelsIfNeeded();

}
///////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::ContinueToCloseChannelsIfNeeded()
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::ContinueToCloseChannelsIfNeeded: no change in bridges. continue to close channels stage");
	m_state = sTRANS_RECREINVITECLOSECHANN;
	CloseChannelsIfNeededForReceiveReInvite();
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnConfBridgesUpdatedUpdateBridges(CSegment* pParam)
{
	DWORD status = STATUS_OK;

	if (IsValidTimer(UPDATEBRIDGESTOUT))
	{
		DeleteTimer(UPDATEBRIDGESTOUT);
		PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnConfBridgesUpdatedUpdateBridges: DeleteTimer(UPDATEBRIDGESTOUT) ");
	}

	//SIP re-invite to check if the change mode succeeded
	*pParam >> status;

	//if change mode failed
	if (status != STATUS_OK)
	{
		// Error while change mode. Reject reinvite. Call will be disconnected by conf
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnConfBridgesUpdatedUpdateBridges: bridges failed. Reject Re-Invite. Name ",m_pPartyConfName);
		if   ( kReInviteArrived == GetDialState() )
		{
			SetDialState(kReInviteRejected);
			m_state = sTRANS_RECREINVITEREJECTED;
			m_retStatusForReject = STATUS_OK; // Party should not initiate disconnecting. The conf will disconnect party.
			m_pSipCntl->SipInviteResponseReq(SipCodesNotAcceptedInHere, SipWarningMediaTypeNotAvail);
		}
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnConfBridgesUpdatedUpdateBridges, accept Re-Invite. Name ",m_pPartyConfName);

		m_pTargetMode->DeSerialize(NATIVE,*pParam);
		m_pTargetMode->Dump("CSipTransReInviteMrcWithSdpInd::OnConfBridgesUpdatedUpdateBridges -this is new target mode", eLevelInfoNormal);
		UdpAddresses sUdpAddressesParams;
		pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));
		m_pSipCntl->SetNewUdpPorts(sUdpAddressesParams);
		BYTE bIsContentSpeaker = FALSE;
		*pParam >> bIsContentSpeaker;
		CheckContentChangesInConfResponse(bIsContentSpeaker);
		*pParam >> m_bNeedReInviteForSecondaryContent;

		m_bNeedReInviteForSecondaryContent = FALSE;

		m_state = sTRANS_RECREINVITECLOSECHANN;

		CloseChannelsIfNeededForReceiveReInvite();
	}
}


///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann(CSegment* pParam)
{
	TRACEINTO << "multi_line - all needed channels are closed. Request to update channels if needed - " << m_pPartyConfName;

	UpdateDbChannelsStatus(pParam, FALSE);

	if(m_bNeedCloseIceChannels)
	{
		m_pSipCntl->CloseIceSession();
		m_bNeedCloseIceChannels = FALSE;
	}
	else
	{
		if(m_pSipCntl->GetIsEnableICE())
		{

			if (STATUS_OK == SendIceMgsReqAccordingToTargetModeAndDiffArr(ICE_MODIFY_SESSION_ANSWER_REQ))
            {
                PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann - Wait for Candidates before continue transaction.  ");
                m_state = sTRANS_WAITFORICECANDIDATES;
                StartTimer(ICEGENERALTOUT,MAKE_ICE_CANDIDATES_TIMER);
            }
            else
            {
                PTRACE2(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnSipReceiveReinviteModifyAnswerInd - Modify Reinvite answer failed : Name ",m_pPartyConfName);
                m_bNeedCloseIceChannels = TRUE;
                ContinueToCloseChannelsIfNeeded();
                return;
            }


		}
		else
		{
			m_state = sTRANS_RECREINVITEUPDATECHANN;
			UpdateChannelsIfNeeded();
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnICEReceiveCloseIceInd(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteMrcWithSdpInd::OnICEReceiveCloseIceInd ", m_pPartyConfName);

	if (m_pSipCntl->IsRemoteMicrosoft())
	{
		//VNGR-25678 On ICE timeout and Microsoft don't fallback to non-ice just disconnect
		PTRACE(eLevelInfoNormal,"OnICEReceiveCloseIceInd::OnICEReceiveCloseIceInd: Remote is MS don't fallback to non-ice, disconnect!!");
		SetDialState(kICETimeOut);
		EndTransaction(SIP_NO_ADDR_FOR_MEDIA);
	} else
	{
	    m_state = sTRANS_RECREINVITEUPDATECHANN;
		UpdateChannelsIfNeeded();
	}
}


///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteMrcWithSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann, all needed channels are updated. Request to open channels if needed - ", m_pPartyConfName);

	m_state = sTRANS_RECREINVITEOPENCHANN;
	BYTE isAnsweringToNewCap = TRUE;
	OpenChannelsIfNeededForReInvite(isAnsweringToNewCap,cmCapReceiveAndTransmit);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnPartyChannelsOpenRecReInviteOpenChann(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteMrcWithSdpInd::OnPartyChannelsOpenRecReInviteOpenChann, all needed channels are open. Response to Re-Invite request - ",m_pPartyConfName);

	//very bad solution but because of time and not enough understanding on how all the function behave in all the state I'm writting all the actions here
	UpdateDbChannelsStatus(pParam, TRUE);
	if (m_pTargetMode->GetConfType() == kVswRelayAvc)
		SendChannelHandleToParty();
	*m_pTargetMode = *m_pCurrentMode;

	m_bIsAllChannelsAreOpenend = TRUE;

	SendReInviteResponse();
}

//////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnPartySlavesRecapIsFinished(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransReInviteMrcWithSdpInd::OnPartySlavesRecapIsFinished");

	m_needToWaitForSlavesEndChangeMode = FALSE;

	SendReInviteResponse();
}
//////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::SendReInviteResponse()
{
	if (!m_bIsAllChannelsAreOpenend)
	{
		PTRACE2(eLevelInfoNormal, "CSipTransReInviteMrcWithSdpInd::SendReInviteResponse, not all channels are opened - ",m_pPartyConfName);
		return;
	}

	if (m_pParty->GetIsTipCall() && m_needToWaitForSlavesEndChangeMode)
	{
		PTRACE2(eLevelInfoNormal, "CSipTransReInviteMrcWithSdpInd::SendReInviteResponse, waiting for slaves to end change mode - ",m_pPartyConfName);
		return;
	}

	PTRACE(eLevelInfoNormal, "CSipTransReInviteMrcWithSdpInd::SendReInviteResponse");

	m_bIsAllChannelsAreOpenend = FALSE;

	m_state = sTRANS_RECREINVITE;
	SendMuteMediaToParty(cmCapReceiveAndTransmit);// instead of MuteMediaIfNeeded(cmCapReceiveAndTransmit);
	SetDialState(kReInviteAccepted);
	if (m_pParty->GetIsTipCall() )
		m_pSipCntl->TipPrepareRatesInLocalCaps( m_pParty->TipGetNumOfStreams(), m_pParty->TipGetIsVideoAux() );
	m_pSipCntl->SipInviteResponseReq(OK_VAL);

	if(IsNeedToSendRtcpVideoPreference())
		SendStartVideoPreferenceToParty();
}

//////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteAck(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteAck");
	EndTransaction();
}

//////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteAckReInviteRejected(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnPartyReceivedReInviteAckReInviteRejected");
	SetDialState(kNotInDialState);
	if (m_bRollbackNeeded == TRUE)
		RollbackTransaction();
	EndTransaction(m_retStatusForReject);
}

//////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::RollbackTransaction()
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::RollbackTransaction");
	m_pSipCntl->ReturnRemoteCapsToThePreviousCaps();  // retrun remote caps to the previous remote caps
}

//////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnReInviteUpdateBridgesTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnReInviteUpdateBridgesTout");
	WORD callIndex = m_pSipCntl->GetCallIndex();
	DBGPASSERT(callIndex);
	if   ( kReInviteArrived == GetDialState() )
	{
		SetDialState(kReInviteRejected);
		m_state = sTRANS_RECREINVITEREJECTED;
		m_retStatusForReject = statIllegal; // Party need to initiate disconnecting.
		m_pSipCntl->SipInviteResponseReq(SipCodesNotAcceptedInHere, SipWarningMediaTypeNotAvail);
	}
}
//////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteMrcWithSdpInd::OnConfSetCapsAccordingToNewAllocation(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteMrcWithSdpInd::OnConfSetCapsAccordingToNewAllocation");
	SetCapsAccordingToNewAllocation(pParam);
}

#endif // 0
