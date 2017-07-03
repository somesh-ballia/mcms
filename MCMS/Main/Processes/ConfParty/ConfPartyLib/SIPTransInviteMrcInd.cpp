//+========================================================================+
//               SIPTransInviteMrcInd.h 				          	   |
//            Copyright 2012 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteMrcInd.cpp                         	   |
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
#include "SIPTransInviteMrcInd.h"



////////////////////////////////////////////////////////////////////////////
CSipTransInviteMrcInd::CSipTransInviteMrcInd(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = TRUE;
	m_bIsReInvite = FALSE;
	m_bIsCloseVideoChannels = FALSE;
	m_bIsCloseDataChannels = FALSE;
	m_bIsCloseBfcpChannels = FALSE;
}

////////////////////////////////////////////////////////////////////////////
CSipTransInviteMrcInd::~CSipTransInviteMrcInd()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::InitPartyEstablishCallIdle()
{
	if (!m_pTargetMode)
	{
		TRACEINTO << "No target mode found. must reject call. Name: " << m_pPartyConfName;
		SetDialState(kCapsDontMatch);
		EndTransaction(SIP_CAPS_DONT_MATCH);
	}

	TRACEINTO << "Name - " << m_pPartyConfName;

	if (!m_pSipCntl->IsBfcpCtrlCreated())
	{
		TRACEINTO << "Content & BFCP is disabled";
		m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
	}

	//	m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
	if (m_pTargetMode->IsMediaOn(cmCapBfcp, cmCapReceiveAndTransmit))
	{
		m_pSipCntl->SetFloorParamsInLocalCaps();
		m_pTargetMode->SetVideoBitRate(0, cmCapReceiveAndTransmit, kRolePresentation);
	}

	CSipCaps*	pRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
	if (pRemoteCaps && pRemoteCaps->GetNumOfCapSets())
	{
		SendOriginalRemoteCapsToParty(pRemoteCaps);
	}

	m_pSipCntl->RemoveUnsupportedSdesCapsForMrcCall();
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::ContinueToCloseChannels()
{
     m_state = sTRANS_CHANGECHANNELS;

     //FSN-613: Dynamic Content for SVC/Mix Conf
     BYTE IsCloseContentChannels = FALSE;
     if (m_bIsCloseBfcpChannels)
	 	IsCloseContentChannels =TRUE;
	 
     CloseChannelsIfNeeded(m_bIsCloseVideoChannels, m_bIsCloseDataChannels, m_bIsCloseBfcpChannels, FALSE, IsCloseContentChannels);  
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnPartyChannelsDisconnectedChangeChannels(CSegment* pParam)
{
	TRACEINTO;
	m_state = sTRANS_RECOVERY;
    HandleChannelsDisconnectedStartRecovery(pParam);
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnPartyChannelsUpdatedRecovery(CSegment* pParam)
{
	TRACEINTO;

	if (m_pSipCntl->GetIsNeedUpdateIceToNonIce())
	{
		TRACEINTO <<"need to close ICE session";
		m_pSipCntl->CloseIceSession();
		m_pSipCntl->SetNeedUpdateIceToNonIce(FALSE);
	}

	HandleChannelsUpdatedDuringRecovery(pParam);
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::InternalRecoveryCompleted()
{
	TRACEINTO;
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
void CSipTransInviteMrcInd::OnConfConnectCallRmtConnected(CSegment* pParam)
{
	TRACEINTO << "Name - " << m_pPartyConfName;

	if (IsValidTimer(UPDATEBRIDGESTOUT))
		DeleteTimer(UPDATEBRIDGESTOUT);
	m_state = sTRANS_OPENOUTCHANNELS;
	BYTE isAnsweringToNewCap = FALSE;

	// if(m_bNeedCloseIceChannels)
	if (m_pSipCntl->GetIsNeedUpdateIceToNonIce())
	{
		OpenInAndOutChannelsIfNeeded (isAnsweringToNewCap); //if closed
		// m_bNeedCloseIceChannels = FALSE;
		m_pSipCntl->SetNeedUpdateIceToNonIce(FALSE);
	}
	else
	{
		OpenOutChannels(isAnsweringToNewCap);

		if (m_bNeedUpdateSrtpChannels/* For SRTP, Content channel is currently AVC so we need to close and reopen it as in AVC flow and not just update */)
		{
			CSipComMode* pComMode = new CSipComMode;
			pComMode->CopyMediaMode(*m_pTargetMode, cmCapVideo, cmCapReceive, kRolePresentation);
			pComMode->SetIsEncrypted(m_pTargetMode->GetIsEncrypted(), m_pTargetMode->GetIsDisconnectOnEncryptionFailure());
			m_pSipCntl->OpenContentRecvChannelForSrtp(pComMode);
			POBJDELETE(pComMode);

			m_bNeedUpdateSrtpChannels = FALSE;
		}
	}
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnPartyChannelsConnectedOpenOut(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteMrcInd::OnPartyChannelsConnectedOpenOut: Name ",m_pPartyConfName);
	HandleOutChannelsConnected(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnConfBridgesConnectionTout(CSegment* pParam)
{
	TRACEINTO;
	PASSERTMSG((DWORD)statIllegal,"CSipTransInviteMrcInd::OnConfBridgesConnectionTout");
	EndTransaction(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnUpdateBridgesTout(CSegment* pParam)
{
	TRACEINTO;
	PASSERTMSG((DWORD)statIllegal,"CSipTransInviteMrcInd::OnUpdateBridgesTout");
	EndTransaction(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::PartyConnectCall()
{
	TRACEINTO << "Name - " << m_pPartyConfName;

	switch(GetDialState())
	{
		case kGuessSucceeded:
		case kNoRecoveryForVideo:
		{
			SipInviteAckReqIfNeeded();

//			if(IsNeedToSendRtcpVideoPreference())
//				SendStartVideoPreferenceToParty();
			//send flow control with new video rate
//			if ( kCp == m_pTargetMode->GetConfType() && m_vidRxRate != 0)
//			{
//				m_pSipCntl->SendInfoFlowControlReq(mainType, cmCapReceive, m_vidRxRate);
//			}
			EndTransaction();
			break;
		}
		case kReInviteSent:
		case kNotInDialState:
		{
			PTRACE2INT(eLevelInfoNormal, "CSipTransInviteMrcWithSdpInd::PartyConnectCall: Do nothing on dial state Name ", GetDialState());  // there is nothing to do
			break;
		}

		default:
		{
			DBGPASSERT(GetDialState());
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
// empty function.
// implemented only in the derived class CSipTransInviteMrcWithSdpInd
void CSipTransInviteMrcInd::SipInviteAckReqIfNeeded()
{
	return;
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::HandleBridgeConnectedInd(DWORD status)
{
	TRACEINTO << "Name - " << m_pPartyConfName;

	if (status == STATUS_OK)
	{
		// its OK if both the bridges connected or its audio only call or there is no much in the video capability
		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected /*&& m_isFeccBridgeConnected*/) || // hg and Shmuel - currently MRC does not support FECC
			(m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_pTargetMode->IsMediaOff(cmCapData,cmCapReceiveAndTransmit)) ||
			(m_isAudioBridgeConnected && GetIsVoice()) ||
			(m_isAudioBridgeConnected && m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit)))
		{
			if (GetDialState() == kBeforeInvite)
			{
				if (IsValidTimer(OPENBRIDGESTOUT))
				{
					DeleteTimer(OPENBRIDGESTOUT);
					TRACEINTO << "DeleteTimer(OPENBRIDGESTOUT)";
				}

				m_state = sTRANS_CONNECTING;
				ContinueHandleBridgeConnectedInd();
			}
		}

		else if (m_isAudioBridgeConnected)
		{
			TRACEINTO << "Is Voice Call? - " << ( GetIsVoice() ? "yes" : "no" );
		}
	}

	else // (status != STATUS_OK)
	{
//		m_eDialState = kBadStatusAckArrived;
		DBGPASSERT(status);
		TRACEINTO << "Ack with bad status: " << status;
		EndTransaction(status);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnPartyChannelsConnectedOpenIn(CSegment* pParam)
{
	TRACEINTO << "Name - " << m_pPartyConfName;

	if (GetDialState() == kBeforeInvite) // channels in connected
	{
		InformChannelsConnectedOpenBridges(pParam);
		m_state = sTRANS_OPENBRIDGES;
		StartTimer(OPENBRIDGESTOUT, 10*SECOND);
	}
	else
	{
		TRACEINTO << "wrong dial state, Name: " << m_pPartyConfName;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::ContinueOnPartyReceivedReinviteResponseOrAckConnecting() // in WithSdp receiving 'ReinviteResponse'; in NoSdp receiving 'Ack'
{
	// pass the product ID to party control
	GetProductIdAndSendToConfLevel();

	SendRemoteCapsReceivedToParty();

	// Check that the incoming open channels are correct. That means the remote can transmit this mode to the MCU.
	// Therefore we have 3 options: GuessSucceeded (no change needed), Internal recovery (need to change only the RTP and codec/bridge setting)
	// and no recovery (this media should be close)
	CSipComMode* pBestMode	= m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, FALSE);

	//Check encryption
//	DWORD isEncrypted = Encryp_Off;


	if(pBestMode)
	{
		pBestMode->Dump("1 : Best Mode",eLevelInfoNormal);
		DWORD isEncrypted = pBestMode->GetIsEncrypted();
		BYTE bIsDisconnectOnEncFailure = pBestMode->GetIsDisconnectOnEncryptionFailure(); 	// BRIDGE-2510/2511
																							//-N.A-check IsDisconnectOnEncrypFail to see if nonEncrypted party can still connect
		// currently, Encryption is not supported in Cascade
		if ( (isEncrypted == Encryp_On) /*&& (CASCADE_MODE_NONE == m_pSipCntl->GetCascadeMode())*/ )	//--- patch for ignoring Encryption in Cascade
		{
			cmCapDataType mediaType;
			ERoleLabel eRole;

			for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
			{

				GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

				if (mediaType == cmCapBfcp)
					continue;

				if (pBestMode->IsMediaOn(mediaType,cmCapTransmit, eRole))
				{
					CSdesCap *pSdesCap = NULL;
					pSdesCap =  pBestMode->GetSipSdes(mediaType,cmCapReceive, eRole);
					if (!pSdesCap)
					{
						if( bIsDisconnectOnEncFailure == FALSE )
						{// BRIDGE-2510/2511 -N.A- if so, remove SDES from RMX side and Set Encryption to OFF connect as Non-Encrypted Party
							TRACESTRFUNC(eLevelError) << "CSipTransInviteMrcInd::ContinueOnPartyReceivedReinviteResponseOrAckConnecting: set encryption to OFF. Name: " << m_pPartyConfName;
							pBestMode->RemoveSipSdes(mediaType,cmCapTransmit,eRole);
							pBestMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
							m_pTargetMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
							isEncrypted = Encryp_Off;
							m_bNeedUpdateSrtpChannels = TRUE;
						}
						else
						{//Disconnect Party
							POBJDELETE(pBestMode);
							DBGPASSERT(YES);
							TRACESTRFUNC(eLevelError) << "CSipTransInviteMrcInd::ContinueOnPartyReceivedReinviteResponseOrAckConnecting: SDES cap is incorrect or not matching. Media: " <<  globalMediaArr[i] << ", Name " << m_pPartyConfName;
							SetDialState(kNoRecovery);
							EndTransaction(SIP_CAPS_DONT_MATCH);
							return;
						}
					}
					else // pSdesCap
					{
						UpdateLocalCapsWithEncryptionParameters(pBestMode, mediaType , eRole);	//BRIDGE-10820
					}
				}
			}
			if ( isEncrypted == Encryp_On )
			{
				SendUpdateDbEncryptionStatusToParty(YES);
			}
		}



		pBestMode->Dump("CSipTransInviteMrcInd::ContinueOnPartyReceivedReinviteResponseOrAckConnecting : Best Mode",eLevelInfoNormal);

		if(IsNoRecovery(pBestMode) )
		{
			PTRACE2(eLevelError,"CSipTransInviteMrcInd::ContinueOnPartyReceivedReinviteResponseOrAckConnecting: Audio remote receive is not matching. Name ",m_pPartyConfName);
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
			TRACEINTO << "Guess succeeded. Name - " << m_pPartyConfName;
			SetDialState(kGuessSucceeded);	// this state indicates now that media can be establish, maybe after internal change.
		}

		m_bIsCloseVideoChannels = FALSE;
		if(IsNoRecoveryForVideo(pBestMode))
		{
			if(GetDialState() == kGuessSucceeded)
				SetDialState(kNoRecoveryForVideo);
			m_bIsCloseVideoChannels = TRUE;
			const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
			//Secondary
			if (pCurRemoteCaps->IsMedia(cmCapVideo))
				m_bNeedReInviteForSecondary = TRUE;
		}

		m_bIsCloseDataChannels = FALSE;
		if(IsNoRecoveryForData(pBestMode))
		{
			m_bIsCloseDataChannels = TRUE;
		}

		//FSN-613: Dynamic Content for SVC/Mix Conf
		m_bIsCloseBfcpChannels = FALSE;
		if(IsNoRecoveryForBfcp(pBestMode))
		{
			m_bIsCloseBfcpChannels = TRUE;
			PTRACE2(eLevelError,"CSipTransInviteMrcInd::ContinueOnPartyReceivedReinviteResponseOrAckConnecting : need to close bfcp channels Name:",m_pPartyConfName);
		}
		
		if(isEncrypted == Encryp_On )
			m_bNeedUpdateSrtpChannels = TRUE;

		// update media from best mode to target mode
		*((CIpComMode*)m_pTargetMode) = *((CIpComMode*)pBestMode);
        // added to synchronize SSRC streams, real solution is to implement update flow
        *((CIpComMode*)m_pCurrentMode) = *((CIpComMode*)pBestMode);
        //update pCall video streams with best mode
        m_pSipCntl->UpdateVideoInStreamsList(pBestMode->GetStreamsListForMediaMode(cmCapVideo, cmCapReceive, kRolePeople)) ;

        m_pTargetMode->Dump("CSipTransInviteMrcInd::ContinueOnPartyReceivedReinviteResponseOrAckConnecting : Best Mode",eLevelInfoNormal);

		POBJDELETE(pBestMode);
	}

	// end the function in one of two ways: disconnect the call if no recovery, or continue recovery flow (disconnect channels if needed)
	if(GetDialState() != kNotInDialState)
	{
		if (m_pSipCntl && m_pSipCntl->GetIsEnableICE())
		{
			if (CProcessBase::GetProcess()->GetProductType() == eProductTypeSoftMCUMfw)
			{
				// VNGSWIBM-742
				TRACEINTO << "Close channels if needed, before ICE stack close them";
				m_state = sTRANS_CLOSE_CHANNELS_BEFORE_PROCESS_ANSWER;
				CloseChannelsIfNeeded(m_bIsCloseVideoChannels, m_bIsCloseDataChannels);
			}
			else
			{
				if (STATUS_OK == SendIceMgsReqAccordingToTargetModeAndCurrentMode(ICE_PROCESS_ANSWER_REQ))
				{
					m_state = sTRANS_WAITFORICECANDIDATES;
					StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
				}
				else
				{
					TRACEINTO << "Send ICE_PROCESS_ANSWER_REQ returned error !";
					m_bNeedReInviteForIce = TRUE;
					m_pSipCntl->SetIsEnableICE(FALSE);
					m_bNeedCloseIceChannels = TRUE;
					m_pSipCntl->SetNeedUpdateIceToNonIce(TRUE);
//					m_bNeedUpdateIceChannels = TRUE;
					ContinueToCloseChannels();
				}
			}
		}
		else // ! m_pSipCntl->GetIsEnableICE()
		{
			ContinueToCloseChannels();
		}
	}
	else  // no recovery  (DialState == kNotInDialState)
	{
		DBGPASSERT(1);
		PTRACE2(eLevelError,"CSipTransInviteMrcInd::ContinueOnPartyReceivedReinviteResponseOrAckConnecting: Audio remote receive is not matching. Name ",m_pPartyConfName);
		SetDialState(kNoRecovery);
		EndTransaction(SIP_CAPS_DONT_MATCH);
		return;
	}

}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnIceReinviteContentArrivedFromIceStack(CSegment* pParam)
{
	TRACEINTO;
	HandleReinviteforICE(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnIceInviteModifyAnsArrivedFromIceStack(CSegment* pParam)
{
	TRACEINTO;
	HandleReinviteforICE(pParam);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnIceInviteProcessAnsArrivedFromIceStack(CSegment* pParam)
{
	TRACEINTO;

	if (IsValidTimer (ICEGENERALTOUT))
	   DeleteTimer(ICEGENERALTOUT);

	DWORD status;
	*pParam >> status;

	if (status == STATUS_OK)
		StartTimer(ICECOMPLETETOUT, MAKE_ICE_CANDIDATES_TIMER);
	else
	{
		TRACEINTO << "Status FAIL!! - No need to wait for ReInvite_ind from Ice stuck";
		//m_bNeedCloseIceChannels = TRUE;
		m_pSipCntl->SetNeedUpdateIceToNonIce(TRUE);
		ContinueToCloseChannels();
	}

}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnIceInviteReceiveMakeOfferInd(CSegment* pParam)
{
	TRACEINTO << "Name: " << m_pPartyConfName;

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

			TRACEINTO << "counter: " << m_IceMakeOfferAnswerCounter;
		}
		else
		{
			m_IceMakeOfferAnswerCounter = 0;
			PASSERTMSG((DWORD)statIllegal, "CSipTransInviteMrcInd::OnIceInviteReceiveMakeOfferInd - making ICE offer failed ");
			m_pSipCntl->SetIsEnableICE(FALSE);
			m_pSipCntl->CloseIceSession();
		}
	}
	else
	{
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone, TRUE);
		m_state = sTRANS_OPENINCHANNELS;
		SetDialState(kBeforeInvite);
	}

}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnICETimeout(CSegment* pParam)
{
	TRACEINTO;
    PASSERTMSG (TRUE, "CSipTransInviteMrcInd::OnICETimeout");
    if (eIceNotConnected != m_pSipCntl->GetIceConnectivityStatus())
    {
        m_pSipCntl->SetIsEnableICE(FALSE);
        //m_bNeedCloseIceChannels = TRUE;
        m_pSipCntl->SetNeedUpdateIceToNonIce(TRUE);
        ContinueToCloseChannels();
    }
    else
        EndTransaction(SIP_NO_ADDR_FOR_MEDIA);

}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnICEOfferTimeout(CSegment* pParam)
{
	TRACEINTO << "Name " << m_pPartyConfName;

	m_pSipCntl->SetIsEnableICE(FALSE);
	m_pSipCntl->CloseIceSession();
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnICEReceiveCloseIceInd (CSegment* pParam)
{
	TRACEINTO << "Name " << m_pPartyConfName;

	m_state = sTRANS_RECOVERY;
    CheckChangingInCurrentMode();
}

/////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnICEReceiveCloseIceIndWaitForCandidates(CSegment* pParam)
{
	TRACEINTO << "Name " << m_pPartyConfName;

	m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone, TRUE);

	m_state = sTRANS_OPENINCHANNELS;
	SetDialState(kBeforeInvite);
}

//////////////////////////////////////////////////////
void CSipTransInviteMrcInd::OnIcePortsRetryTout(CSegment* pParam)
{
	TRACEINTO << " make offer. Name " << m_pPartyConfName;

	m_pSipCntl->SetIsEnableICE(TRUE);

	SendIceMgsReqAccordingToTargetMode(ICE_MAKE_OFFER_REQ);

	StartTimer(ICEOFFERTOUT, MAKE_ICE_CANDIDATES_TIMER);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::IceConnectivityCheckComplete(CSegment* pParam)
{
	TRACEINTO << "Name " << m_pPartyConfName;

	CSegment* CopyOfseg = new CSegment(*pParam);

	ICE_CHECK_COMPLETE_IND_S* pConnCheckStruct = (ICE_CHECK_COMPLETE_IND_S*) pParam->GetPtr(1);
	int status = pConnCheckStruct->status;
	TRACEINTO << "status: " << status;

	//In case of secondary - we first send Process answer to CM with A+V
	// and only after connectivity check we send Modify_req with A only - We don't need to wait to Re invite ind.
	if (STATUS_OK == status && m_bIsChangeInICEChannels)
//	if (m_bIsChangeInICEChannels)
	{
		m_bIsChangeInICEChannels = FALSE;
		SendIceMgsReqAccordingToTargetMode(ICE_MODIFY_SESSION_OFFER_REQ);
	}
//	else // Temp for ICE-SVC, currently we don't support ReInvite for SVC call so we continue with the flow.
//	{
//		PTRACE(eLevelInfoNormal, "ICE-SVC: CSipTransInviteMrcWithSdpInd::IceConnectivityCheckComplete : Currently we don't support ReInvite for SVC call. continue the flow without reinvite");
//		ContinueToCloseChannels();
//	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcInd::HandleReinviteforICE(CSegment* pParam)
{
	TRACEINTO;

    if (IsValidTimer (ICECOMPLETETOUT))
        DeleteTimer(ICECOMPLETETOUT);
    //todo - Get the reinvite content

    DWORD status;
   	*pParam >> status;

   	TRACEINTO << "status: " << status
   			  << ", connectivity status: " << m_pSipCntl->GetIceConnectivityStatus()
   			  << ", is MOC: " << ( (m_pSipCntl->IsRemoteMicrosoft()) ? "yes" : "no" );

//  Bridge-5263 - SVC call, We cannot drop ICE since we cannot send re-invite request to MFW - not imp. yet
//	Continue flow with ICE even if the call is local

//	BOOL preserve_ice_in_local_call_flag = GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_PRESERVE_ICE_CHANNEL_IN_CASE_OF_LOCAL_MODE);
//
//	if (!preserve_ice_in_local_call_flag)
//	{
//	    if (status != STATUS_OK || (m_pSipCntl->GetIceConnectivityStatus() != eIceConnectedRemote && !m_pSipCntl->IsRemoteMicrosoft()))
//	    {
//	    	m_pSipCntl->SetIsEnableICE(FALSE);
//	    	//m_bNeedCloseIceChannels = TRUE;
//	    	m_pSipCntl->SetNeedUpdateIceToNonIce(TRUE);
//
//	    	FPTRACE (eLevelInfoNormal, "The call is local, ICE channel closed");
//	    }
//
//	} else
//	{
//		if (status != STATUS_OK || (m_pSipCntl->GetIceConnectivityStatus() != eIceConnectedRemote && !m_pSipCntl->IsRemoteMicrosoft()))
//			FPTRACE (eLevelInfoNormal, "The call is local, ICE channel preserved due to configuration");
//	}

	//PTRACE(eLevelInfoNormal, "ICE-SVC: CSipTransInviteMrcWithSdpInd::HandleReinviteforICE : Currently we don't support ReInvite for SVC call. continue the flow without reinvite");
	//m_bNeedReInviteForIce = FALSE;

   	TRACEINTO << "requesting Re-Invite";

	m_bNeedReInviteForIce = TRUE;

    ContinueToCloseChannels();
}
