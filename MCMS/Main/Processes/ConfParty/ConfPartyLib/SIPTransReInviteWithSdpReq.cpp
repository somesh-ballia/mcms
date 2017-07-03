//+========================================================================+
//               SIPTransReInviteWithSdpReq.cpp    				       	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransReInviteWithSdpReq.cpp                          	   |
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
#include "SIPTransReInviteWithSdpReq.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransReInviteWithSdpReq)

ONEVENT(SIP_PARTY_SEND_REINVITE,			IDLE,								CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_REINVITEUPDATECHANNELS,		CSipTransReInviteWithSdpReq::OnPartyChannelsUpdatedInitReinvite)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_REINVITEOPENINCHANNELS,		CSipTransReInviteWithSdpReq::OnPartyChannelsConnectedOpenInChannels)
ONEVENT(SIP_PARTY_REINVITE_RESPONSE,		sTRANS_INITREINVITE,				CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite)
ONEVENT(PARTYCONNECTTOUT,					sTRANS_INITREINVITE,				CSipTransReInviteWithSdpReq::OnPartyConnectToutInitReinvite)

ONEVENT(SIP_CONF_BRIDGES_UPDATED,			sTRANS_INITREINVITEUPDATEBRIDGE,	CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges)

ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,		sTRANS_INITREINVITECLOSECHAN,		CSipTransReInviteWithSdpReq::OnPartyChannelsCloseReInviteCloseChannels)//the case when we close the video in channel because we can't recover video
ONEVENT(SIP_PARTY_STATISTIC_INFO,		sTRANS_SAVESTATISTICINFOBEFORECLOSECHANNEL, CSipTransReInviteWithSdpReq::OnPartyRecStatisticInfo)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED,sTRANS_DTLS_CLOSE_BEFORE_CLOSING_SIP_CHANNEL, CSipTransReInviteWithSdpReq::OnDtlsClosedChannelBeforeSipCloseChannels)


ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_INITREINVITEUPDATECHANN,		CSipTransReInviteWithSdpReq::OnPartyChannelsUpdatedChannels)

ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_INITEINVITEOPENCHANN,		CSipTransReInviteWithSdpReq::OnPartyChannelsConnected)// case of internal recovery recovery as well

// timeout
ONEVENT(UPDATEBRIDGESTOUT,					sTRANS_RECREINVITEUPDATEBRIDGE, 	CSipTransReInviteWithSdpReq::OnReInviteUpdateBridgesTout)

// Glare
ONEVENT(SIP_PARTY_RECEIVED_REINVITE,		ANYCASE,							CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteAnycase)
ONEVENT(SIP_PARTY_RECEIVED_ACK,				ANYCASE,							CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteAckAnycase)
//ICE
ONEVENT(ICE_PROCESS_ANS_IND,				sTRANS_WAITFORICECANDIDATES,    	CSipTransReInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack)
ONEVENT(ICE_MODIFY_OFFER_IND,				sTRANS_WAITFORICECANDIDATES,		CSipTransReInviteWithSdpReq::OnIceInviteModifyAnsArrivedFromIceStack)
ONEVENT(ICEGENERALTOUT,  					sTRANS_WAITFORICECANDIDATES,    	CSipTransReInviteWithSdpReq::OnICETimeout)
ONEVENT(ICEMODIFYTOUT,  					sTRANS_WAITFORICECANDIDATES,    	CSipTransReInviteWithSdpReq::OnICEModifyTimeout)
ONEVENT(CLOSE_ICE_SESSION_IND,              sTRANS_INITREINVITECLOSECHAN,		CSipTransReInviteWithSdpReq::OnICEReceiveCloseIceInd)
//DPA
ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION ,ANYCASE,				            CSipTransReInviteWithSdpReq::OnConfSetCapsAccordingToNewAllocation)
ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,   sTRANS_RMTCONNECTED,                CSipTransaction::OnRemoveAvcToSvcArtTranslatorAnycase)
ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,   sTRANS_INITREINVITEUPDATEBRIDGE,    CSipTransaction::OnRemoveAvcToSvcArtTranslatorAnycase)
ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED, sTRANS_RMTCONNECTED,                CSipTransaction::OnPartyTranslatorArtsDisconnected)
ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED, sTRANS_INITREINVITEUPDATEBRIDGE,    CSipTransaction::OnPartyTranslatorArtsDisconnected)

ONEVENT(SIP_PARTY_SLAVES_RECAP_FINISHED,	ANYCASE,							CSipTransReInviteWithSdpReq::OnPartySlavesRecapIsFinished)

PEND_MESSAGE_MAP(CSipTransReInviteWithSdpReq, CSipTransaction);

///////////////////////////////////////////////////////
CSipTransReInviteWithSdpReq::CSipTransReInviteWithSdpReq(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = TRUE;
	m_bIsReInvite = TRUE;
	m_bIsWaitForPendingAck = FALSE;
	m_oldContentProtocol = eUnknownAlgorithemCapCode;

	m_bIsAllChannelsAreConnected = FALSE;

	VALIDATEMESSAGEMAP
}

///////////////////////////////////////////////////////
CSipTransReInviteWithSdpReq::~CSipTransReInviteWithSdpReq()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle : Name", m_pPartyConfName);

	if(m_pTargetMode)
	{
		m_pTargetMode->Dump("CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle - m_pTargetMode START", eLevelInfoNormal);
		m_pCurrentMode->Dump("CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle - m_pCurrentMode START", eLevelInfoNormal);
	}
	DWORD reason= 0;

	if(pParam != NULL)
		*pParam >> reason;
	m_Transation_Reason = reason;

	// if the only audio cap is Rfc2833 we should reject the call.
	if (m_pTargetMode
				&& (((m_pTargetMode->IsMediaOn(cmCapAudio,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapAudio,cmCapTransmit)
				&& (m_pTargetMode->GetMediaType(cmCapAudio) != eRfc2833DtmfCapCode)))
				|| m_pParty->GetNonTipPartyOnHold() != eMediaOnHoldNon ))
	{

		// Don't declare content in re-invite_req in AS-SIP:
		if (m_pTargetMode->IsMediaOn(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation) && m_pSipCntl->IsASSIPContentDisabledinASSIPConf() )
		{
			PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle : Remove content from target mode. Name", m_pPartyConfName);
			m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
		}


		if (m_pSipCntl->GetIsEnableICE() && !IsNeedReInviteForIce() && IsNeedToUpdateIceStack())
		{	//If this is second Reinvite after reinvite of ice we need to check if there is any change of channels state(open/close)

			if(SendIceMgsReqAccordingToTargetModeAndDiffArr(ICE_MODIFY_SESSION_OFFER_REQ) == STATUS_OK)
			{
				m_state = sTRANS_WAITFORICECANDIDATES;
				StartTimer(ICEMODIFYTOUT, MAKE_ICE_CANDIDATES_TIMER);
			}
			else
			{
			   	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle Status Fail !!: Name ",m_pPartyConfName);
			   	m_bNeedCloseIceChannels = TRUE;
				ContinueToCloseChannelsIfNeeded();
			}
		}
		else
		{

			// The first stage is to update the channel because of the changes after the re-alloc
			m_state = sTRANS_REINVITEUPDATECHANNELS;
			/** AVA-1329 In case EP doesn't support LPR,
			 * Calling to Check changing will trigger RTP channel update which will close content in case it is active */
			{
				const CSipCaps* pCurRemoteCaps 		= m_pSipCntl->GetLastRemoteCaps();
				if (pCurRemoteCaps->GetIsLpr() == FALSE) {
					m_pCurrentMode->SetIsLpr(FALSE);
					m_pTargetMode->SetIsLpr(FALSE);
				}
			}
			CheckChangingInCurrentMode(TRUE);
		}
	}
	else
	{
		PTRACE(eLevelError,"CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle: No target mode found. must reject call");
		SetDialState(kCapsDontMatch);
		EndTransaction(kCapsDontMatch);
	}


	COstrStream msg3;
	m_pTargetMode->Dump(msg3);
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle, m_pTargetMode after: ", msg3.str().c_str());

	m_pCurrentMode->Dump("CSipTransReInviteWithSdpReq::OnPartySendReInviteIdle - m_pCurrentMode END", eLevelInfoNormal);

}
/////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipTransReInviteWithSdpReq::IsNeedToUpdateIceStack()
{
	BYTE res = FALSE;
	cmCapDataType mediaType;
	ERoleLabel eRole;
	for (int i = 0 ; i < MAX_SIP_MEDIA_TYPES; i++)
	{
		GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

		if (mediaType == cmCapBfcp)
			continue;

		BYTE removeChannelAccording2Scm = m_pCurrentMode->IsMediaOn(mediaType,cmCapTransmit,eRole) &&
										m_pTargetMode->IsMediaOff(mediaType,cmCapTransmit,eRole);

		BYTE AddChannelAccording2Scm = m_pCurrentMode->IsMediaOff(mediaType,cmCapTransmit,eRole) &&
										m_pTargetMode->IsMediaOn(mediaType,cmCapTransmit,eRole);

		if(removeChannelAccording2Scm || AddChannelAccording2Scm )
			res = TRUE;
	}

	return res;
}
/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnIceInviteModifyAnsArrivedFromIceStack(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnIceInviteModifyAnsArrivedFromIceStack : Name", m_pPartyConfName);
	DWORD status;
		*pParam >> status;

	if (IsValidTimer (ICEGENERALTOUT))
		DeleteTimer(ICEGENERALTOUT);

	if(status != STATUS_OK )
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnIceInviteModifyAnsArrivedFromIceStack Status Fail !!: Name ",m_pPartyConfName);
		m_bNeedCloseIceChannels = TRUE;
	}

	m_state = sTRANS_REINVITEUPDATECHANNELS;
	CheckChangingInCurrentMode(TRUE);
}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnICEModifyTimeout(CSegment* pParam)
{
	PASSERTMSG (TRUE, "CSipTransReInviteWithSdpReq::OnICEModifyTimeout");

	m_bNeedCloseIceChannels = TRUE;

	m_state = sTRANS_REINVITEUPDATECHANNELS;
	CheckChangingInCurrentMode(TRUE);


}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyChannelsUpdatedInitReinvite(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyChannelsUpdatedInitReinvite : Name", m_pPartyConfName);
	m_state = sTRANS_REINVITEOPENINCHANNELS;
	BYTE isAnsweringToNewCap = FALSE;
	OpenChannelsIfNeededForReInvite(isAnsweringToNewCap, cmCapReceive);
}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyChannelsConnectedOpenInChannels(CSegment* pParam)
{
	UpdateDbChannelsStatus(pParam, TRUE);

	RemoveRtvCapsIfNeeded(m_pTargetMode);

	if( m_pSipCntl->IsPendingTrns() == etransReinvite)
	{
		TRACEINTO << "Collision of ReInvites. State = " << (int)m_state;
		SendLastTargetModeToParty();
		m_pSipCntl->SipInviteResponseReq(SipCodesRequestPending);// response with 491. Even if we have already sent our ReInvite-Req, it was dropped by CS.
		m_bIsWaitForPendingAck = TRUE;
		SetDialState(kReInviteRejected);
		m_pSipCntl->RemovePendingReInviteTrns();
		return;
	}
	// Sending the re-invite request after all channels were updated
	m_state = sTRANS_INITREINVITE;
	m_pSipCntl->SipReInviteReq(m_Transation_Reason);
	SetDialState(kReInviteSent);
}
/////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyChannelsConnected(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyChannelsConnected, all needed channels are open: Name ",m_pPartyConfName);
//	PTRACE2(eLevelInfoNormal, "CSipTransReInviteNoSdpInd::OnPartyChannelsOpenRecReInviteOpenChann, all needed channels are open. Response to Re-Invite request - ",m_pPartyConfName);

	//very bad solution but because of time and not enough understanding on how all the function behave in all the state I'm writting all the actions here
	UpdateDbChannelsStatus(pParam, TRUE);
	m_pTargetMode->Dump("CSipTransReInviteWithSdpReq::OnPartyChannelsConnected m_pTargetMode BEFORE assignment to current", eLevelInfoNormal);
	*m_pTargetMode = *m_pCurrentMode;
    m_pTargetMode->Dump("CSipTransReInviteWithSdpReq::OnPartyChannelsConnected m_pTargetMode AFTER assignment to current", eLevelInfoNormal);
	SendMuteMediaToParty(cmCapReceiveAndTransmit);//instead of MuteMediaIfNeeded(cmCapReceiveAndTransmit);

	CSipChannel *pChannelIn = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapReceive, kRolePeople);
	if (eProductTypeSoftMCUMfw == CProcessBase::GetProcess()->GetProductType() ||
	    ((m_pTargetMode->GetConfMediaType()==eMixAvcSvc && !m_pSipCntl->GetIsMrcCall() && pChannelIn)) ||
		    m_pSipCntl->GetIsMrcCall())
	{
		TRACEINTO << "SendChannelHandleToParty";
		SendChannelHandleToParty();
	}

	m_bIsAllChannelsAreConnected = TRUE;

	SendInviteAckIfNeeded();
}
///////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::SendInviteAckIfNeeded()
{
	PTRACE(eLevelInfoNormal, "CSipTransReInviteWithSdpReq::SendInviteAckIfNeeded");

	if (!m_bIsAllChannelsAreConnected)
	{
		PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpReq::SendInviteAckIfNeeded, not all channels are connected - ",m_pPartyConfName);
		return;
	}

	if (m_pParty->GetIsTipCall() && m_needToWaitForSlavesEndChangeMode)
	{
		PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpReq::SendInviteAckIfNeeded, waiting for slaves to end change mode - ",m_pPartyConfName);
		return;
	}

	m_pSipCntl->SipInviteAckReq();
	EndTransaction();
}

///////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: Name ",m_pPartyConfName);

	PASSERT_AND_RETURN(!m_pSipCntl);

	DWORD status;
	status = 0;
	BYTE bRemovedAudio, bRemovedVideo;
	bRemovedAudio = bRemovedVideo = 0;
	*pParam >> status;
	*pParam >> bRemovedAudio >> bRemovedVideo;

	SetDialState(kNotInDialState);
	WORD callIndex = m_pSipCntl ? m_pSipCntl->GetCallIndex() : 0;

	if (status == STATUS_OK)
	{

		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: Response with SDP");
		// we need to close channels (if needed), update bridges and when receive the answer to update RTP and open new channels.

		CSipCaps*	pCurRemoteCaps = m_pSipCntl ? const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps()) : NULL;
		if (pCurRemoteCaps== NULL)
		{
			// Yossig Klocwork NPD issue - Noa?
			// No caps in remote SDP, if we got here there should have been..anyway return and notify
			PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: no remote caps. ignore the transaction");
			DBGPASSERT(callIndex);
			EndTransaction(statNoChange);
			return;
		}

		POBJDELETE(m_pChanDifArr);
		m_pChanDifArr	= new CSipChanDifArr;

		m_pChanDifArr->DeSerialize(NATIVE,*pParam);

		SendOriginalRemoteCapsToParty(pCurRemoteCaps);

		// TIP - Recognition if remote resume the media
		if (m_pParty->GetIsTipCall())
		{
			UpdateIfCallIsResumed();
			const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
			RemoteIdent remoteIdent = m_pSipCntl->GetRemoteIdent();

			if (!::CheckIfRemoteSdpIsTipCompatible(pCurRemoteCaps) && (m_bIsResumeMedia || (remoteIdent == PolycomEp)) )
            {
            	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected -fall back to none TIP");
            	FallbackFromTipToNoneTip();
            	m_pParty->SetIsTipCall(FALSE);
            }

			if( m_pParty->GetIsTipNegotiationActive() && m_pParty->GetTipPartyOnHold() )
			{
				m_pSipCntl->EndTipNegotiation(eTipNegError);
				m_pParty->SetIsTipNegotiationActive(FALSE);
			}
		}

		DWORD vidRateBefore = m_pTargetMode->GetMediaBitRate(cmCapVideo, cmCapReceive);
		CSipComMode* pBestMode = m_pSipCntl ? m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, TRUE) : NULL;

		if (pBestMode)
		{
			// to remove/update unnecessary sdes attributes of new media from local/max/full/chosen caps.  for example: content
			DWORD isEncrypted = pBestMode->GetIsEncrypted();

			if (isEncrypted == Encryp_On )
			{
				cmCapDataType	 mediaType;
				ERoleLabel		 eRole;

				for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
				{
					GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

					if (mediaType == cmCapBfcp)
						continue;

					if (pBestMode->IsMediaOn(mediaType,cmCapTransmit,eRole))
					{
						CSdesCap *pSdesCap = NULL;

						pSdesCap =  pBestMode->GetSipSdes(mediaType,cmCapReceive,eRole);

						if (!pSdesCap)
						{
							if(mediaType == cmCapData && pBestMode->GetSipSdes(cmCapAudio,cmCapReceive))
							{
								 TRACEINTO << "sdes available for audio but NOT for fecc - remove FECC ";
								 m_pSipCntl->RemoveFeccCaps();
								 pBestMode->RemoveData(cmCapReceiveAndTransmit);
							}
//							else if( (bIsUndefinedParty && bIsDisconnectOnEncFailure==NO && bIsWhenAvailableEncMode) ||
//									 ( isDtlsEncrypted == Encryp_On ) )
//							{
//								pBestMode->RemoveSipSdes(mediaType,cmCapTransmit,eRole);
//								pBestMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
//								isEncrypted = Encryp_Off;
//								m_pSipCntl->RemoveSdesCapFromLocalCaps(mediaType, eRole);
//							}
//							else
//								bCapsDontMatch = YES;
						}
						else
						{
							UpdateLocalCapsWithEncryptionParameters(pBestMode, mediaType , eRole);	//BRIDGE-10820
						}
					}
				}
			}
			
            if (pBestMode->GetMediaType(cmCapVideo, cmCapTransmit) == eSvcCapCode)
            {// copy streams information
                pBestMode->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople), cmCapVideo, cmCapTransmit, kRolePeople);
                pBestMode->Dump("CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite - @#@ after copy streams information", eLevelInfoNormal);
            }

			if (m_pSipCntl && m_pTargetMode->IsMediaOn(cmCapVideo,cmCapTransmit,kRolePeople))
			{
				BfcpDecisionCenter(pBestMode);

	            if (m_bTransactionSetContentOn)
					m_pTargetMode->RemoveContent(cmCapReceiveAndTransmit); // In order that we can compare the new best-mode with the original target mode (without the content set on by transaction)

				if (pBestMode->IsContent(cmCapReceiveAndTransmit) && !m_pTargetMode->IsContent(cmCapReceiveAndTransmit))
				{
					const CSipCaps *pLocalCaps = m_pSipCntl->GetLocalCaps();

					CheckIfNeedToFixContentAlgAndSendReInvite(pLocalCaps);//, pCurRemoteCaps);
				}
			}

			//send flow control with new video rate
			DWORD vidRateAfter = pBestMode->GetMediaBitRate(cmCapVideo, cmCapReceive);

			if( m_pTargetMode->GetConfType() == kCop )
				vidRateAfter = CalcualteNewRateAccordingToBestModeForCOP(vidRateAfter ,pBestMode);
			if ( (kCp == m_pTargetMode->GetConfType() || m_pTargetMode->GetConfType() == kCop) && vidRateBefore != vidRateAfter)
			{
				PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: sending flow control as video rate changed from", vidRateBefore);
				m_pSipCntl->SendFlowControlReq(mainType, cmCapReceive, vidRateAfter);
			}

			if (m_pTargetMode->GetConfType() == kCop && (m_pTargetMode->GetCopTxLevel() != pBestMode->GetCopTxLevel()))
			{
				CSipChanDif* pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapTransmit);
				if (pChanDif)
				    pChanDif->SetChangeAlg(YES);
				else
				    PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite - pChanDif is NULL");
			}


			if (pBestMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation) && m_pCurrentMode->IsMediaOn(cmCapVideo, cmCapReceive, kRoleContentOrPresentation)) {

				m_pCurrentMode->SetRtcpFeedbackMask(pBestMode->GetRtcpFeedbackMask(cmCapReceive, kRoleContentOrPresentation), cmCapReceive, kRoleContentOrPresentation);
				m_pTargetMode->SetRtcpFeedbackMask(pBestMode->GetRtcpFeedbackMask(cmCapReceive, kRoleContentOrPresentation), cmCapReceive, kRoleContentOrPresentation);
				m_pSipCntl->SetRtcpMaskIntoCall(pBestMode->GetRtcpFeedbackMask(cmCapReceive, kRoleContentOrPresentation));
			}

            m_oldContentProtocol = (CapEnum) m_pTargetMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePresentation);

			BYTE bChangeInMedia = YES;

			TRACEINTO << " m_bIsGlare " << (int)m_bIsGlare; //BRIDGE-12961

			if ((m_pTargetMode->IsMediaEquals(*pBestMode, cmCapAudio, cmCapReceiveAndTransmit))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapData, cmCapReceiveAndTransmit))
					&& (!(m_pTargetMode->GetConfType() == kCop && (m_pTargetMode->GetCopTxLevel() != pBestMode->GetCopTxLevel())))
					&& !m_isFallbackFromTipToSipFlow
					&& !m_bIsGlare)//BRIDGE-12961
				bChangeInMedia = NO;
			
			//LyncCCS
			if ((m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))			
				&&(m_pSipCntl->GetIsCCSPlugin())) 
				 bChangeInMedia = NO;
			
			//N.A.
			SetMediaSdesChangesIfNeeded(pBestMode);
			
			if (bChangeInMedia)
			{
				PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: update party control on new caps (update bridges)");

				if (m_pTargetMode->GetConfType() == kCop)
				{
					PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: best level=", pBestMode->GetCopTxLevel());
					PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: target level=", m_pTargetMode->GetCopTxLevel());
				}

				m_state = sTRANS_INITREINVITEUPDATEBRIDGE;
				StartTimer(UPDATEBRIDGESTOUT, BRIDGES_TIME * SECOND);
				SendReCapsReceivedToParty(pCurRemoteCaps, pBestMode, FALSE , m_bIsGlare); //BRIDGE-12961
                m_pSipCntl->CancelLpr();
			}
			else
			{
				if(m_pSipCntl->GetIsEnableICE())
				{
					if (STATUS_OK == SendIceMgsReqAccordingToTargetModeAndDiffArr(ICE_PROCESS_ANSWER_REQ,bChangeInMedia))
					{
						PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite - start timer  ");
						m_state = sTRANS_WAITFORICECANDIDATES;
						StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
					}
					else
					{
							PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack Status Fail !!: Name ",m_pPartyConfName);
							m_bNeedCloseIceChannels = TRUE;
							m_state = sTRANS_INITREINVITECLOSECHAN;
							CloseChannelsIfNeededForReceiveReInvite();
					}

				}
				else
				{
					PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: no change in bridges. continue to close channels stage");

					if (GetRtpStatisticsIfNeededForReinvite())
						PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite - GetRtpStatisticsIfNeededForReinvite=TRUE");
					else
						PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite - GetRtpStatisticsIfNeededForReinvite=FALSE");
				}
			}
		}
		else
		{
			// Best mode is null. No common codecs. Disconnect call.
			PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: no common mode. ignore the transaction");
			DBGPASSERT(callIndex);
			EndTransaction(statIllegal);
		}

		POBJDELETE(pBestMode);
	}
	else if (status == SipCodesRequestPending) // 491
	{
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: ReInvite Response with 491. Glare Status.");
		SendLastTargetModeToParty();
		if (m_pSipCntl)
			m_pSipCntl->SipInviteAckReq();
		EndTransaction(SIP_CLIENT_ERROR_491);
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteResponseInitReinvite: ReInvite Response with bad status - ",status);
		DBGPASSERT(callIndex);
		if (m_pSipCntl)
			m_pSipCntl->SipInviteAckReq();
		EndTransaction(status);
	}
}
///////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack: Name ",m_pPartyConfName);

	DWORD status;
	*pParam >> status;

	if (IsValidTimer (ICEGENERALTOUT))
		DeleteTimer(ICEGENERALTOUT);

	if(status != STATUS_OK )
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnIceInviteProcessAnsArrivedFromIceStack Status Fail !!: Name ",m_pPartyConfName);
		m_bNeedCloseIceChannels = TRUE;
		ContinueToCloseChannelsIfNeeded();
	}
	else
	{
		m_state = sTRANS_INITREINVITEUPDATECHANN;
		UpdateChannelsIfNeeded();
	}

}
/////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnICETimeout(CSegment* pParam)
{
    PASSERTMSG (TRUE, "CSipTransReInviteWithSdpReq::OnICETimeout");
    //todo real error handling here

    m_bNeedCloseIceChannels = TRUE;

    ContinueToCloseChannelsIfNeeded();
}
///////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::ContinueToCloseChannelsIfNeeded()
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::ContinueToCloseChannelsIfNeeded: no change in bridges. continue to close channels stage");
	m_state = sTRANS_RECREINVITECLOSECHANN;

	if (GetRtpStatisticsIfNeededForReinvite())
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::ContinueToCloseChannelsIfNeeded - GetRtpStatisticsIfNeededForReinvite=TRUE");
	else
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::ContinueToCloseChannelsIfNeeded - GetRtpStatisticsIfNeededForReinvite=FALSE");
}
/////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyChannelsCloseReInviteCloseChannels(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyChannelsCloseReInviteCloseChannels: connect call");

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
			if (STATUS_OK == SendIceMgsReqAccordingToTargetModeAndDiffArr(ICE_PROCESS_ANSWER_REQ))
            {
                PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyChannelsCloseReInviteCloseChannels - Wait for Candidates before continue transaction.  ");
                m_state = sTRANS_WAITFORICECANDIDATES;
                StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
            }
            else // remote answered with no ICE in its SDP or other failure - close Ice
            {
                m_pSipCntl->CloseIceSession();
                m_bNeedCloseIceChannels = FALSE;
            }
		}
		else
		{
			m_state = sTRANS_INITREINVITEUPDATECHANN;
			UpdateChannelsIfNeeded();
		}
	}
}
/////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnICEReceiveCloseIceInd(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpReq::OnICEReceiveCloseIceInd ", m_pPartyConfName);
	m_state = sTRANS_INITREINVITEUPDATECHANN;

	if (m_pSipCntl->IsRemoteMicrosoft())
	{
		//VNGR-25678 On ICE timeout and Microsoft don't fallback to non-ice just disconnect
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnICEReceiveCloseIceInd: Remote is MS don't fallback to non-ice, disconnect!!");
		SetDialState(kICETimeOut);
		EndTransaction(SIP_NO_ADDR_FOR_MEDIA);
	} else
	{
		UpdateChannelsIfNeeded();
	}
}

/////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyChannelsUpdatedChannels(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpReq::OnPartyChannelsUpdatedChannels, all needed channels are updated. Request to open channels if needed - ", m_pPartyConfName);
	m_state = sTRANS_INITEINVITEOPENCHANN;
	BYTE isAnsweringToNewCap = FALSE;
	OpenChannelsIfNeededForReInvite(isAnsweringToNewCap, cmCapReceiveAndTransmit);

}


/////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyConnectToutInitReinvite(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyConnectToutInitReinvite: Disconnect call");

	SetDialState(kConnectTimer);
	DBGPASSERT(GetDialState());

	EndTransaction(SIP_TIMER_POPPED_OUT);
}

/////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnReInviteUpdateBridgesTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnReInviteUpdateBridgesTout");
	WORD callIndex = m_pSipCntl->GetCallIndex();
	DBGPASSERT(callIndex);
	EndTransaction(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges(CSegment* pParam)
{
	DWORD status = STATUS_OK;
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges");
	//SIP re-invite to check if the change mode succeeded
	*pParam >> status;
	WORD callIndex = m_pSipCntl->GetCallIndex();
	if (IsValidTimer(UPDATEBRIDGESTOUT))
	{
		DeleteTimer(UPDATEBRIDGESTOUT);
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges: DeleteTimer(UPDATEBRIDGESTOUT) ");
	}
	//if change mode failed
	if (status != STATUS_OK)
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges, bridges failed to support new mode",m_pPartyConfName);
		DBGPASSERT(callIndex);
		EndTransaction(statIllegal); // Error while change mode.
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges, accept new transaction by bridges. Name ",m_pPartyConfName);

		m_pTargetMode->DeSerialize(NATIVE,*pParam);

		UdpAddresses sUdpAddressesParams;
		pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));
		m_pSipCntl->SetNewUdpPorts(sUdpAddressesParams);
		BYTE bIsContentSpeaker = FALSE;
		*pParam >> bIsContentSpeaker;
		CheckContentChangesInConfResponse(bIsContentSpeaker, m_oldContentProtocol);

		BYTE bIsSecondaryContent;

		*pParam >> bIsSecondaryContent;

        BYTE bUpdateMixModeResources = FALSE;
        *pParam >> bUpdateMixModeResources;
        if (bUpdateMixModeResources)
        {
            CRsrcParams* pMrmpRsrcParams = NULL;
            CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
            DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams);
            for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
            {
                DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i], "mix_mode: translator");
            }
            for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
            {
               POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
            }
            POBJDELETE(pMrmpRsrcParams);
        }

		PTRACE2INT(eLevelInfoNormal,"AN DEBUG CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges, 1.is secondary content on: ",bIsSecondaryContent);

		if (!m_bNeedReInviteForSecondaryContent && bIsSecondaryContent)
			m_bNeedReInviteForSecondaryContent = TRUE;

		if (m_bNeedReInviteForSecondaryContent)
			m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);

		if (GetRtpStatisticsIfNeededForReinvite())
			PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges - GetRtpStatisticsIfNeededForReinvite=TRUE");
		else
			PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnConfBridgesUpdatedUpdateBridges - GetRtpStatisticsIfNeededForReinvite=FALSE");
	}
}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteAnycase(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteAnycase - Collision of ReInvites. State = ",m_state);
	BYTE isReInviteWithSdp = YES;
	BYTE isRejectReInvite = YES;
	*pParam >> isReInviteWithSdp;
	*pParam >> isRejectReInvite;
	if(isRejectReInvite)
	{
		SendLastTargetModeToParty(); //BRIDGE-12961
		m_pSipCntl->SipInviteResponseReq(SipCodesRequestPending);// response with 491. Even if we have already sent our ReInvite-Req, it was dropped by CS.
		m_bIsWaitForPendingAck = TRUE;
		SetDialState(kReInviteRejected);
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteAnycase -  end outgoing reINVITE trans, start Glare");
		SetDialState(kNotInDialState);
		EndTransaction(SIP_CLIENT_ERROR_491); // In order to start Glare handling.
	}
}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteAckAnycase(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnPartyReceivedReInviteAck. State = ",m_state);
	if (m_bIsWaitForPendingAck)
	{
		// It is an Ack for our 491.
		SetDialState(kNotInDialState);
		m_bIsWaitForPendingAck = FALSE;
		EndTransaction(SIP_CLIENT_ERROR_491); // In order to start Glare handling.
	}
	else
	{
		WORD callIndex = m_pSipCntl->GetCallIndex();
		DBGPASSERT(callIndex);
		EndTransaction(statIllegal);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnConfSetCapsAccordingToNewAllocation(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnConfSetCapsAccordingToNewAllocation only need to be here in case of fall back from TIP to SIP!!!");
	SetCapsAccordingToNewAllocation(pParam);
	m_bNeedReInviteForReAlloc = TRUE;  //is this needed noa?
}

///////////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartySlavesRecapIsFinished(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransReInviteWithSdpReq::OnPartySlavesRecapIsFinished");

	m_needToWaitForSlavesEndChangeMode = FALSE;

	SendInviteAckIfNeeded();
}
////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnPartyRecStatisticInfo(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CDR_MCCF: CSipTransReInviteWithSdpReq::OnPartyRecStatisticInfo m_state:",m_state);

    CloseDtlsChannelsBeforeSipChannelsIfNeeded();
}
////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpReq::OnDtlsClosedChannelBeforeSipCloseChannels - m_state:", m_state);
	m_state = sTRANS_INITREINVITECLOSECHAN;

	CloseChannelsIfNeededForReceiveReInvite();
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpReq::RequiredChannelsActionDone(DWORD opcode)
{
    PTRACE2INT(eLevelInfoNormal,"CSipTransaction::RequiredChannelsActionDone, opcode:", opcode);
    PTRACE2INT(eLevelInfoNormal,"CSipTransaction::RequiredChannelsActionDone, m_state:", m_state);

    CSegment *pSeg = new CSegment;

    CSipComMode* pCurrentMode = new CSipComMode;
    pCurrentMode->Create(*m_pSipCntl->GetCallObj());
    pCurrentMode->CopyStaticAttributes(*m_pTargetMode);
    pCurrentMode->SetStreamsListForMediaMode(m_pTargetMode->GetStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople), cmCapVideo, cmCapTransmit, kRolePeople);
    pCurrentMode->Serialize(NATIVE, *pSeg);

    pCurrentMode->Dump("***CSipTransaction::RequiredChannelsActionDone @#@ pCurrentMode", eLevelInfoNormal);

    DispatchEvent(opcode,pSeg);
    POBJDELETE(pSeg);
    POBJDELETE(pCurrentMode);
}
