//+========================================================================+
//              SIPTransReInviteWithSdpInd.cpp           	 			   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransReInviteWithSdpInd.cpp                          	   |
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
#include "SIPTransReInviteWithSdpInd.h"
#include "OpcodesMcmsCardMngrICE.h"
#include "SIPParty.h"

extern CConfPartyRoutingTable* GetpConfPartyRoutingTable( void );

PBEGIN_MESSAGE_MAP(CSipTransReInviteWithSdpInd)

// handle re-caps (Remote Re-Invite)
ONEVENT(SIP_PARTY_RECEIVED_REINVITE,	IDLE,										CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected)
ONEVENT(SIP_CONF_BRIDGES_UPDATED,		sTRANS_RECREINVITEUPDATEBRIDGE,				CSipTransReInviteWithSdpInd::OnConfBridgesUpdatedUpdateBridges)
ONEVENT(PARTY_TRANSLATOR_ARTS_CONNECTED,sPARTY_ALLOCATE_TRANSLATOR_ARTS,            CSipTransReInviteWithSdpInd::OnPartyTranslatorArtsConnected)
ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,	sTRANS_RECREINVITECLOSECHANN,				CSipTransReInviteWithSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann)
ONEVENT(SIP_PARTY_STATISTIC_INFO,		sTRANS_SAVESTATISTICINFOBEFORECLOSECHANNEL, CSipTransReInviteWithSdpInd::OnPartyRecStatisticInfo)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED,sTRANS_DTLS_CLOSE_BEFORE_CLOSING_SIP_CHANNEL, CSipTransReInviteWithSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels)
ONEVENT(SIP_PARTY_CHANS_UPDATED,		sTRANS_RECREINVITEUPDATECHANN,				CSipTransReInviteWithSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,		sTRANS_RECREINVITEOPENCHANN,				CSipTransReInviteWithSdpInd::OnPartyChannelsOpenRecReInviteOpenChann)

ONEVENT(SIP_PARTY_SLAVES_RECAP_FINISHED,ANYCASE,									CSipTransReInviteWithSdpInd::OnPartySlavesRecapIsFinished)

ONEVENT(SIP_PARTY_RECEIVED_ACK,			sTRANS_RECREINVITE,							CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteAck)
//ICE
ONEVENT(ICE_MODIFY_ANSWER_IND,		    sTRANS_WAITFORICECANDIDATES,				CSipTransReInviteWithSdpInd::OnSipReceiveReinviteModifyAnswerInd)
ONEVENT(ICEGENERALTOUT,				    sTRANS_WAITFORICECANDIDATES,				CSipTransReInviteWithSdpInd::OnICEReinviteGeneralTimeout)
ONEVENT(CLOSE_ICE_SESSION_IND,          sTRANS_RECREINVITECLOSECHANN,		     	CSipTransReInviteWithSdpInd::OnICEReceiveCloseIceInd)

//DTLS
ONEVENT(SIP_PARTY_DTLS_STATUS,				sTRANS_DTLS_STARTED,		CSipTransaction::OnPartyDtlsEndInd)
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			sTRANS_DTLS_STARTED,	CSipTransaction::DisconnectOnDtlsEncryptFail)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_UPDATED_CHAN, 	CSipTransReInviteWithSdpInd::OnDtlsChannelsUpdated)
ONEVENT(DTLSTOUT,							sTRANS_DTLS_STARTED,		CSipTransaction::OnDtlsTout)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransReInviteWithSdpInd::OnDtlsChannelsUpdated)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED,	sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransReInviteWithSdpInd::OnDtlsChannelsUpdated) //BRIDGE-6184
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			ANYCASE,						CSipTransaction::TipEarlyPacketDtlsNotNeeded)

// Reject
ONEVENT(SIP_PARTY_RECEIVED_ACK,			sTRANS_RECREINVITEREJECTED,					CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteAckReInviteRejected)

// timeout
ONEVENT(UPDATEBRIDGESTOUT,				sTRANS_RECREINVITEUPDATEBRIDGE, 			CSipTransReInviteWithSdpInd::OnReInviteUpdateBridgesTout)
//DPA
ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION,ANYCASE,				CSipTransReInviteWithSdpInd::OnConfSetCapsAccordingToNewAllocation)
ONEVENT(END_VIDEO_UPGRADE_TO_MIX_AVC_SVC,	sTRANS_CHANNSUPDATEDWAITFORBRIDGESUPGRADE/*sTRANS_WAITFORUPDATECHANN*/, CSipTransReInviteWithSdpInd::OnSipEndVideoUpgradeToMix)

ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,  sTRANS_RECREINVITEUPDATEBRIDGE,     CSipTransaction::OnRemoveAvcToSvcArtTranslatorAnycase)
ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED, sTRANS_RECREINVITEUPDATEBRIDGE, CSipTransaction::OnPartyTranslatorArtsDisconnected)

PEND_MESSAGE_MAP(CSipTransReInviteWithSdpInd, CSipTransaction);



///////////////////////////////////////////////////////
CSipTransReInviteWithSdpInd::CSipTransReInviteWithSdpInd(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_retStatusForReject 		= STATUS_OK;
	m_bRollbackNeeded 			= FALSE;
	m_bIsOfferer 				= FALSE;
	m_bIsReInvite 				= TRUE;

	m_bNeedCloseIceChannels 	= FALSE;
	m_bIsAllChannelsAreOpenend 	= FALSE;
	m_isFallBackFromTip         = FALSE;

	VALIDATEMESSAGEMAP
}

///////////////////////////////////////////////////////
CSipTransReInviteWithSdpInd::~CSipTransReInviteWithSdpInd()
{
}

////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected(CSegment* pParam)
{
	// We reject the re-invite only IF the remote SDP contians valid caps for media to be transmit, but there is no match with our local caps.
	// more specific: if no match in A+V, or no match in A, if we have no match in V but we have match in A we will make the call secondary.
	// E.g. remote has H261 video only and the local is H263.
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected");

	BYTE	isRejectReInvite;
	*pParam >> isRejectReInvite;

	if (isRejectReInvite)
	{
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected - Reject remote ReInvite");
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
		if( false == m_pSipCntl->CheckAndStoreRemoteOriginVersionField() )
		{
			PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected - session timer");
			m_state = sTRANS_RECREINVITE;
			SetDialState(kReInviteAccepted);
			m_pSipCntl->SipInviteResponseReq(OK_VAL);
			return;
		}
		SetDialState(kReInviteArrived);

		SendOriginalRemoteCapsToParty(pCurRemoteCaps);

		// TIP - Recognition if remote resume the media
		if (m_pParty->GetIsTipCall())
		{
			UpdateIfCallIsResumed();
			const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
			RemoteIdent remoteIdent = m_pSipCntl->GetRemoteIdent();

			if (!::CheckIfRemoteSdpIsTipCompatible(pCurRemoteCaps))
            {
            	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected -fall back to none TIP");
            	FallbackFromTipToNoneTip();
            	m_pParty->SetIsTipCall(FALSE);
            	m_isFallBackFromTip = TRUE;
            }

			if( m_pParty->GetIsTipNegotiationActive() && m_pParty->GetTipPartyOnHold() )
			{
				m_pSipCntl->EndTipNegotiation(eTipNegError);
				m_pParty->SetIsTipNegotiationActive(FALSE);
			}
		}

		CSipComMode* pBestMode =NULL;

		if(m_pTargetMode->GetConfType() == kCp || m_pTargetMode->GetConfType() == kVSW_Fixed || m_pTargetMode->GetConfType() == kCop) //jason zhu for VNGR-24428
		{
			TRACEINTO << "using m_pTargetModeMaxAllocation";
			m_pTargetModeMaxAllocation->Dump("m_pTargetModeMaxAllocation");
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
			{
				//m_pSipCntl->UpdateLocalCapsSdesTag(pBestMode);

				//BRIDGE-13713
				BOOL  bIsUndefinedParty 		= m_pSipCntl->IsUndefinedParty();
				BYTE  bIsDisconnectOnEncFailure = pBestMode->GetIsDisconnectOnEncryptionFailure();
				BYTE  bIsWhenAvailableEncMode 	= m_pSipCntl->IsWhenAvailableEncryptionMode();
				DWORD isDtlsEncrypted 			= pBestMode->GetIsDtlsEncrypted();

				TRACEINTO << " bIsUndefinedParty " <<(int)bIsUndefinedParty << " bIsDisconnectOnEncFailure " <<(int)bIsDisconnectOnEncFailure << " bIsWhenAvailableEncMode " <<(int)bIsWhenAvailableEncMode ;

				//Check encryption
				DWORD isEncrypted = Encryp_Off;
				isEncrypted = pBestMode->GetIsEncrypted();

				if(isEncrypted == Encryp_On)
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
							if ((mediaType == cmCapVideo) && (eRole & kRoleContentOrPresentation))
							{
								if (!pBestMode->GetSipSdes(cmCapAudio,cmCapReceive))
								{
									pBestMode->RemoveSipSdes(mediaType, cmCapTransmit, eRole);
									m_pSipCntl->RemoveSdesCapFromLocalCaps(mediaType, eRole);
								}
							}
							CSdesCap *pSdesCap = NULL;
							pSdesCap =  pBestMode->GetSipSdes(mediaType,cmCapReceive, eRole);
							//BRIDGE-13713
							if (!pSdesCap)
							{
								//BRIDGE-13713
								if((bIsUndefinedParty && bIsDisconnectOnEncFailure==NO && bIsWhenAvailableEncMode) ||
									(!bIsUndefinedParty && bIsWhenAvailableEncMode) ||
									(isDtlsEncrypted == Encryp_On))
								{
									PTRACE2(eLevelError,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected: set encryption to OFF. Name ",m_pPartyConfName);

									pBestMode->RemoveSipSdes(mediaType,cmCapTransmit,eRole);
									m_pTargetModeMaxAllocation->RemoveSipSdes(mediaType,cmCapTransmit,kRolePeople);
									m_pTargetModeMaxAllocation->RemoveSipSdes(mediaType,cmCapReceive,kRolePeople);
									m_pTargetMode->RemoveSipSdes(mediaType,cmCapTransmit,kRolePeople);
									m_pTargetMode->RemoveSipSdes(mediaType,cmCapReceive,kRolePeople);

									pBestMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
									m_pTargetMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
									m_pTargetModeMaxAllocation->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);

									m_pSipCntl->RemoveSdesCapFromLocalCaps(mediaType, eRole);

									isEncrypted 			 = Encryp_Off;

									CSipChanDif* pChanDifForOut = m_pChanDifArr->GetChanDif(mediaType, cmCapTransmit);
									CSipChannel *pChannelOut = m_pSipCntl->GetChannel(mediaType, cmCapTransmit);

									if ( pChannelOut && pChannelOut->IsChannelSdesEnabled() && pChanDifForOut)
									{
										PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected: change sdes for tx also");
										pChanDifForOut->SetChangeSdes(YES);
									}

								}
								else
								{
									POBJDELETE(pBestMode);

									DBGPASSERT(YES);
									PTRACE2(eLevelError,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected: SDES cap is incorrect or not matching. Name ",m_pPartyConfName);
									SetDialState(kCapsDontMatch);
									EndTransaction(SIP_CAPS_DONT_MATCH);
									return;
								}
							}
							//if (pSdesCap) //BRIDGE-13713
							else //BRIDGE-13713
							{
								TRACEINTO << "updating LocalCaps with SDES params for mediaType " << mediaType <<" eRole " << eRole;

								UpdateLocalCapsWithEncryptionParameters(pBestMode, mediaType , eRole); 	//BRIDGE-10820
							}
						}
					}
				}
			}

			if (RemoveEncryptionFromScmAndCapsWhenTipResumedIfNeeded(pBestMode))
				PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected: TIP resumed - Encryption removed");

			if (m_pTargetModeMaxAllocation->GetConfType()==kCop && (m_pTargetModeMaxAllocation->GetCopTxLevel()!=pBestMode->GetCopTxLevel()))
			{
				CSipChanDif* pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapTransmit);

				if (pChanDif)
				    pChanDif->SetChangeAlg(YES);
				else
				    PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected - pChanDif is NULL");
			}

			if (m_pSipCntl->GetIsMrcCall() || (m_pTargetMode->GetConfMediaType()==eMixAvcSvcVsw))
			{
				CSipChanDif* pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapTransmit);
				PASSERT(NULL == pChanDif);
				if (pChanDif && pChanDif->IsMute())
				{
					pBestMode->RemoveStreamsListForMediaMode(cmCapVideo, cmCapTransmit, kRolePeople);
					pBestMode->Dump("Best mode after remove streams", eLevelInfoNormal);
				}
			}
			DWORD details = 0;
			BYTE bChangeInMedia = YES;

			if ((m_pTargetMode->IsMediaEquals(*pBestMode, cmCapAudio, cmCapReceiveAndTransmit))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
					&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapData, cmCapReceiveAndTransmit))
					&& (m_pTargetMode->IsMediaContaining(*pBestMode, kNumOfStreamDesc, &details, cmCapVideo, cmCapTransmit))
					&& !(m_pTargetMode->GetConfType()==kCop && (m_pTargetMode->GetCopTxLevel()!=pBestMode->GetCopTxLevel()))
					&& !(m_pTargetMode->GetConfMediaType()==eMixAvcSvcVsw && !m_pSipCntl->GetIsMrcCall()) )
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
			PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected -  before set changes in Sdes:", bChangeInMedia);
// N.A.
// A.N - BRIDGE-9853
			bChangeInMedia = SetMediaSdesChangesIfNeeded(pBestMode, bChangeInMedia);

			if (bChangeInMedia)
			{
				PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected - bChangeInSdes:", bChangeInMedia);
				PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected: update party control on new caps (update bridges)");

				m_state = sTRANS_RECREINVITEUPDATEBRIDGE;
				StartTimer(UPDATEBRIDGESTOUT, BRIDGES_TIME * SECOND);

				// this brings to UpdateBridge (in CSipChangeModePartyCntl::StartReCaps)
				SendReCapsReceivedToParty(pCurRemoteCaps, pBestMode,m_isFallBackFromTip);
			}

			if(!bChangeInMedia)
			{
				if(m_pSipCntl->GetIsEnableICE()&& !(m_pSipCntl->GetIsST852Resume()))
				{

					if (STATUS_OK == SendIceMgsReqAccordingToTargetModeAndDiffArr(ICE_MODIFY_SESSION_ANSWER_REQ,bChangeInMedia))
					{
					PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected - Wait for Candidates before continue transaction.  ");
                            PTRACE(eLevelInfoNormal,"CSipTransaction::IsIceActionsNeeded .  ");
                            m_state = sTRANS_WAITFORICECANDIDATES;
                            StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
                        }
                        else
                        {
                            PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected - Modify Reinvite answer failed : Name ",m_pPartyConfName);
                            m_bNeedCloseIceChannels = TRUE;
                            ContinueToCloseChannelsIfNeeded();
                        }

				} // if(m_pSipCntl->GetIsEnableICE())
				else
				{
					PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected: no change in bridges. continue to close channels stage");

					*m_pTargetMode = *pBestMode;
					if (GetRtpStatisticsIfNeededForReinvite())
						PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected - GetRtpStatisticsIfNeededForReinvite=TRUE");
					else
						PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected - GetRtpStatisticsIfNeededForReinvite=FALSE");
				}
			} // if(!bChangeInMedia)

			// update bfcp in case remote declares on it only in reinvite
		} // if(pBestMode)
		else
		{// if the best mode is null, no common codec (the case of port=0 is with pBestMode without and media set in to it).
			// reject the transaction
			PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected: no common mode. reject re-invite");
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
void CSipTransReInviteWithSdpInd::OnSipReceiveReinviteModifyAnswerInd(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnSipReceiveReinviteModifyAnswerInd");

	WORD Status = STATUS_FAIL;
	DWORD newRate = 0;

	*pParam >> Status;
	*pParam >> newRate;

	if (IsValidTimer(ICEGENERALTOUT))
		DeleteTimer(ICEGENERALTOUT);

	/* vngr-25846 do not update the rate in case of vsw*/
	if( newRate != 0 && m_pTargetMode->GetConfType() == kCp  &&
	        !(m_pTargetMode->GetConfMediaType()==eMixAvcSvcVsw && !m_pSipCntl->GetIsMrcCall())
	        && (newRate < m_pTargetMode->GetVideoBitRate(cmCapReceiveAndTransmit, kRolePeople)))
		m_pTargetMode->SetVideoBitRate(newRate,cmCapReceiveAndTransmit);

	if(Status != STATUS_OK )
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnSipReceiveReinviteModifyAnswerInd - Modify Reinvite answer failed : Name ",m_pPartyConfName);
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
void CSipTransReInviteWithSdpInd::OnICEReinviteGeneralTimeout(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnICEReinviteGeneralTimeout: Name ",m_pPartyConfName);
	m_pSipCntl->SetIsEnableICE(FALSE);
	m_bNeedCloseIceChannels = TRUE;

	ContinueToCloseChannelsIfNeeded();

}
///////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::ContinueToCloseChannelsIfNeeded()
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::ContinueToCloseChannelsIfNeeded: no change in bridges. continue to close channels stage");
    m_pCurrentMode->Dump("CSipTransReInviteWithSdpInd::ContinueToCloseChannelsIfNeeded -this is current mode", eLevelInfoNormal);

	if (GetRtpStatisticsIfNeededForReinvite())
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::ContinueToCloseChannelsIfNeeded - GetRtpStatisticsIfNeededForReinvite=TRUE");
	else
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::ContinueToCloseChannelsIfNeeded - GetRtpStatisticsIfNeededForReinvite=FALSE");
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnConfBridgesUpdatedUpdateBridges(CSegment* pParam)
{
	DWORD status = STATUS_OK;
	int IsOpenInternalArts=0;

	if (IsValidTimer(UPDATEBRIDGESTOUT))
	{
		DeleteTimer(UPDATEBRIDGESTOUT);
		PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnConfBridgesUpdatedUpdateBridges: DeleteTimer(UPDATEBRIDGESTOUT) ");
	}

	//SIP re-invite to check if the change mode succeeded
	*pParam >> status;

	//if change mode failed
	if (status != STATUS_OK)
	{
		// Error while change mode. Reject reinvite. Call will be disconnected by conf
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnConfBridgesUpdatedUpdateBridges: bridges failed. Reject Re-Invite. Name ",m_pPartyConfName);
		if   ( kReInviteArrived == GetDialState() )
		{
			SetDialState(kReInviteRejected);
			m_state = sTRANS_RECREINVITEREJECTED;
			m_retStatusForReject = STATUS_OK; // Party should not initiate disconnecting. The conf will disconnect party.
			m_pSipCntl->SipInviteResponseReq(SipCodesNotAcceptedInHere, SipWarningMediaTypeNotAvail);
		}
		return;
	}
    PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnConfBridgesUpdatedUpdateBridges, accept Re-Invite. Name ",m_pPartyConfName);

    m_pTargetMode->DeSerialize(NATIVE,*pParam);
    m_pTargetMode->Dump("CSipTransReInviteWithSdpInd::OnConfBridgesUpdatedUpdateBridges -this is new target mode", eLevelInfoNormal);
    m_pCurrentMode->Dump("CSipTransReInviteWithSdpInd::OnConfBridgesUpdatedUpdateBridges -this is current mode", eLevelInfoNormal);
    UdpAddresses sUdpAddressesParams;
    pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));
    m_pSipCntl->SetNewUdpPorts(sUdpAddressesParams);
    BYTE bIsContentSpeaker = FALSE;
    *pParam >> bIsContentSpeaker;
    CheckContentChangesInConfResponse(bIsContentSpeaker);
    *pParam >> m_bNeedReInviteForSecondaryContent;

    m_bNeedReInviteForSecondaryContent = FALSE;

    BYTE bUpdateMixModeResources = FALSE;
    *pParam >> bUpdateMixModeResources;
    CRsrcParams* pMrmpRsrcParams = NULL;
    CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
    for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    {
        avcToSvcTranslatorRsrcParams[i] = NULL;
    }
    int cnt = 0;

    if (bUpdateMixModeResources)
    {
        DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams);
        for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
        {
            DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i], "mix_mode: translator");
            if (avcToSvcTranslatorRsrcParams[i])
                cnt++;
        }
    }

    if (m_pTargetMode->GetConfMediaType() == eMixAvcSvc && bUpdateMixModeResources)
    {
        m_pSipCntl->PrintChangeModeWithinTransactionValue();

        m_pSipCntl->SetInternalControllerResource(avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);
        m_pSipCntl->AddToInternalRoutingTable();

        int cntPrev = m_pSipCntl->GetNumberOfActiveInternalArts();
        if (cnt > cntPrev)
        {
           TRACEINTO << "!@# cnt=" << cnt << " cntPrev=" << cntPrev << "up: need to update internal controller resource";
           m_pCurrentMode->Dump("CSipTransReInviteWithSdpInd::OnConfBridgesUpdatedUpdateBridges -this is current mode", eLevelInfoNormal);
           m_state = sPARTY_ALLOCATE_TRANSLATOR_ARTS;
           IsOpenInternalArts = m_pSipCntl->OpenInternalArts(E_NETWORK_TYPE_IP, cnt);

           for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
           {
              POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
           }
           POBJDELETE(pMrmpRsrcParams);

           return;
         }
         else if(cnt<cntPrev)
         {
             TRACEINTO<<"!@# cnt="<<cnt<<" cntPrev="<<cntPrev<<"down: need to update internal controller resource";
         }
         else
         {
             TRACEINTO<<"!@# cnt="<<cnt<<" cntPrev="<<cntPrev<<"no need to update internal controller resource";
         }

        CConfPartyRoutingTable* pRoutingTbl = ::GetpConfPartyRoutingTable();
        if( NULL == pRoutingTbl )
        {
            TRACEINTO<<"!@# ptr to routing table is NULL";
            PASSERT(103);
        }
        else
        {
            TRACEINTO<<"!@# printing routing table";
            pRoutingTbl->DumpTable();
        }
    }

    for(int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
    {
       POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
    }
    POBJDELETE(pMrmpRsrcParams);

    // IF YOU NEED TO ADD MORE LOGIC, ADD IT INSIDE THIS FUNCTION!!!
    ContinueToCloseChannelsIfNeeded();
}

void CSipTransReInviteWithSdpInd::OnPartyTranslatorArtsConnected()
{
    ContinueToCloseChannelsIfNeeded();
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann(CSegment* pParam)
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
	
		if(m_pSipCntl->GetIsEnableICE() && !(m_pSipCntl->GetIsST852Resume()))
		{

			if (STATUS_OK == SendIceMgsReqAccordingToTargetModeAndDiffArr(ICE_MODIFY_SESSION_ANSWER_REQ, TRUE)) //TRUE=ChangeInMedia
            {
                PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann - Wait for Candidates before continue transaction.  ");
                m_state = sTRANS_WAITFORICECANDIDATES;
                StartTimer(ICEGENERALTOUT,MAKE_ICE_CANDIDATES_TIMER);
            }
            else
            {
                PTRACE2(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnSipReceiveReinviteModifyAnswerInd - Modify Reinvite answer failed : Name ",m_pPartyConfName);
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
void CSipTransReInviteWithSdpInd::OnICEReceiveCloseIceInd(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpInd::OnICEReceiveCloseIceInd ", m_pPartyConfName);

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
void CSipTransReInviteWithSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann, all needed channels are updated. Request to open channels if needed - ", m_pPartyConfName);

	m_state = sTRANS_RECREINVITEOPENCHANN;
	BYTE isAnsweringToNewCap = TRUE;
	OpenChannelsIfNeededForReInvite(isAnsweringToNewCap,cmCapReceiveAndTransmit);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnPartyChannelsOpenRecReInviteOpenChann(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpInd::OnPartyChannelsOpenRecReInviteOpenChann, all needed channels are open. Response to Re-Invite request - ",m_pPartyConfName);

	//very bad solution but because of time and not enough understanding on how all the function behave in all the state I'm writting all the actions here
	UpdateDbChannelsStatus(pParam, TRUE);
	CSipChannel *pChannelIn;


	pChannelIn = m_pSipCntl->GetChannelEx(cmCapVideo, cmCapReceive, kRolePeople);

    if (eProductTypeSoftMCUMfw == CProcessBase::GetProcess()->GetProductType() ||
	    ((m_pTargetMode->GetConfMediaType()==eMixAvcSvc && !m_pSipCntl->GetIsMrcCall() && pChannelIn)) ||
	    m_pSipCntl->GetIsMrcCall())
	{
		SendChannelHandleToParty();
	}
	m_bIsAllChannelsAreOpenend = TRUE;
	    *m_pTargetMode = *m_pCurrentMode;
	    SendReInviteResponse();
}

//////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnPartySlavesRecapIsFinished(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransReInviteWithSdpInd::OnPartySlavesRecapIsFinished");

	m_needToWaitForSlavesEndChangeMode = FALSE;

	SendReInviteResponse();
}
//////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::SendReInviteResponse()
{
	if (!m_bIsAllChannelsAreOpenend)
	{
		PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpInd::SendReInviteResponse, not all channels are opened - ",m_pPartyConfName);
		return;
	}

	if (m_pParty->GetIsTipCall() && m_needToWaitForSlavesEndChangeMode)
	{
		PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpInd::SendReInviteResponse, waiting for slaves to end change mode - ",m_pPartyConfName);
		return;
	}

	PTRACE(eLevelInfoNormal, "CSipTransReInviteWithSdpInd::SendReInviteResponse");

	m_bIsAllChannelsAreOpenend = FALSE;

	m_state = sTRANS_RECREINVITE;
	if(!m_pParty->GetIsTipCall())
	SendMuteMediaToParty(cmCapReceiveAndTransmit);// instead of MuteMediaIfNeeded(cmCapReceiveAndTransmit);
	SetDialState(kReInviteAccepted);
	if (m_pParty->GetIsTipCall() )
		m_pSipCntl->TipPrepareRatesInLocalCaps( m_pParty->TipGetNumOfStreams(), m_pParty->TipGetIsVideoAux() );
	m_pSipCntl->SipInviteResponseReq(OK_VAL);

	if(IsNeedToSendRtcpVideoPreference())
		SendStartVideoPreferenceToParty();
}

//////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteAck(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteAck");
	if ( m_pParty->GetIsTipCall() )
	{
		if (m_pParty->GetTipPartyOnHold())
			SendMuteMediaToParty(cmCapReceiveAndTransmit);// instead of MuteMediaIfNeeded(cmCapReceiveAndTransmit);
		else
		{
			// start DTLS if needed.
			// If not, go to end transaction
			//DTLS
			if (m_bIsResumeMedia)
			{
				BYTE bAllowDtlsFlag = FALSE;
				std::string strEncryptionMode;
				CSysConfig *sysConfig = CProcessBase::GetProcess()->GetSysConfig();
				sysConfig->GetDataByKey(CFG_KEY_SIP_ENCRYPTION_KEY_EXCHANGE_MODE, strEncryptionMode);

				if(strEncryptionMode.compare("AUTO")== 0 || strEncryptionMode.compare("DTLS")== 0)
					bAllowDtlsFlag = TRUE;

				if (m_pCurrentMode->GetIsEncrypted() == Encryp_On && bAllowDtlsFlag && !m_bTipEarlyPacket)
				{
					m_pCurrentMode->SetDtlsEncryption(Encryp_On);
					m_pCurrentMode->CreateLocalSipComModeDtls(Encryp_On, TRUE /*is tip compatible*/);
					m_pTargetMode->SetDtlsEncryption(Encryp_On);
					m_pTargetMode->CreateLocalSipComModeDtls(Encryp_On, TRUE /*is tip compatible*/);
				}

				if (StartDtlsIfNeeded())
					return;
			}
		}

	}
	EndTransaction();
}

//////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteAckReInviteRejected(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteAckReInviteRejected");
	SetDialState(kNotInDialState);
	if (m_bRollbackNeeded == TRUE)
		RollbackTransaction();
	EndTransaction(m_retStatusForReject);
}

//////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::RollbackTransaction()
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::RollbackTransaction");
	m_pSipCntl->ReturnRemoteCapsToThePreviousCaps();  // retrun remote caps to the previous remote caps
}

//////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnReInviteUpdateBridgesTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnReInviteUpdateBridgesTout");
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
void CSipTransReInviteWithSdpInd::OnConfSetCapsAccordingToNewAllocation(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnConfSetCapsAccordingToNewAllocation");
	SetCapsAccordingToNewAllocation(pParam);
}

////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnPartyRecStatisticInfo(CSegment* pParam)
{
    PTRACE2INT(eLevelInfoNormal,"CDR_MCCF: CSipTransReInviteWithSdpInd::OnPartyRecStatisticInfo m_state:",m_state);

    CloseDtlsChannelsBeforeSipChannelsIfNeeded();
}
////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteWithSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels - m_state:", m_state);
	m_state = sTRANS_RECREINVITECLOSECHANN;

	CloseChannelsIfNeededForReceiveReInvite();
}


void CSipTransReInviteWithSdpInd::OnPartyVideoArtDisconnected()
{
	TRACEINTO<<"will update soon m_state: "<<(int)m_state;
      m_state = sTRANS_RECREINVITEUPDATECHANN;
      UpdateChannelsIfNeeded();
}

void CSipTransReInviteWithSdpInd::OnSipEndVideoUpgradeToMix(CSegment* pParam)
{
     *m_pTargetMode = *m_pCurrentMode;
     SendReInviteResponse();

  //	EndTransaction();
	TRACEINTO<<"!@# ";
}
void CSipTransReInviteWithSdpInd::OnDtlsChannelsUpdated()
{
			EndTransaction();
}
