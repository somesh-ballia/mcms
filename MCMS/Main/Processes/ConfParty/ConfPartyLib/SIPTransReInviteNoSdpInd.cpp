//+========================================================================+
//	               SIPTransReInviteNoSdpInd.cpp            	  			   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransReInviteNoSdpInd.cpp                            	   |
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
#include "IpServiceListManager.h"
#include "IPParty.h"
#include "SIPTransaction.h"
#include "SIPTransReInviteNoSdpInd.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransReInviteNoSdpInd)

ONEVENT(SIP_PARTY_RECEIVED_REINVITE,	IDLE,							CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteConnected)
ONEVENT(SIP_PARTY_RECEIVED_ACK,			sTRANS_RESPONSEREINVITE,		CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck)
ONEVENT(SIP_CONF_BRIDGES_UPDATED,		sTRANS_RECREINVITEUPDATEBRIDGE,	CSipTransReInviteNoSdpInd::OnConfBridgesUpdatedUpdateBridges)
ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,	sTRANS_RECREINVITECLOSECHANN,	CSipTransReInviteNoSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann)
ONEVENT(SIP_PARTY_STATISTIC_INFO,		sTRANS_SAVESTATISTICINFOBEFORECLOSECHANNEL, CSipTransReInviteNoSdpInd::OnPartyRecStatisticInfo)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED,sTRANS_DTLS_CLOSE_BEFORE_CLOSING_SIP_CHANNEL, CSipTransReInviteNoSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels)
ONEVENT(SIP_PARTY_CHANS_UPDATED,		sTRANS_RECREINVITEUPDATECHANN,	CSipTransReInviteNoSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,		sTRANS_RECREINVITEOPENCHANN,	CSipTransReInviteNoSdpInd::OnPartyChannelsOpenRecReInviteOpenChann)
ONEVENT(SIP_PARTY_SLAVES_RECAP_FINISHED,ANYCASE,						CSipTransReInviteNoSdpInd::OnPartySlavesRecapIsFinished)
ONEVENT(SIP_PARTY_RECEIVED_REINVITE,	ANYCASE,						CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAnycase)// response with 491 (pending)
ONEVENT(SIP_PARTY_RECEIVED_ACK,			ANYCASE,						CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAckAnycase)

ONEVENT(SIP_PARTY_DTLS_STATUS,				sTRANS_DTLS_STARTED,		CSipTransaction::OnPartyDtlsEndInd)
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			sTRANS_DTLS_STARTED,	CSipTransaction::DisconnectOnDtlsEncryptFail)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_UPDATED_CHAN, 	CSipTransReInviteNoSdpInd::OnDtlsChannelsUpdated)
ONEVENT(DTLSTOUT,							sTRANS_DTLS_STARTED,		CSipTransaction::OnDtlsTout)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransReInviteNoSdpInd::OnDtlsChannelsUpdated)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED,	sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransReInviteNoSdpInd::OnDtlsChannelsUpdated) //BRIDGE-6184
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			ANYCASE,						CSipTransaction::TipEarlyPacketDtlsNotNeeded)
ONEVENT(SIP_PARTY_PENDING_TRANS,			sTRANS_DTLS_STARTED,		CSipTransaction::OnSipNotifyPendingTransactionDtlsStarted) //BRIDGE-15745


// timeout
ONEVENT(UPDATEBRIDGESTOUT,				sTRANS_RECREINVITEUPDATEBRIDGE, CSipTransReInviteNoSdpInd::OnReInviteUpdateBridgesTout)

//DPA
ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION,ANYCASE,				CSipTransReInviteNoSdpInd::OnConfSetCapsAccordingToNewAllocation)
ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,  sTRANS_RECREINVITEUPDATEBRIDGE,     CSipTransaction::OnRemoveAvcToSvcArtTranslatorAnycase)
ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED, sTRANS_RECREINVITEUPDATEBRIDGE, CSipTransaction::OnPartyTranslatorArtsDisconnected)

PEND_MESSAGE_MAP(CSipTransReInviteNoSdpInd,CSipTransaction);

///////////////////////////////////////////////////////
CSipTransReInviteNoSdpInd::CSipTransReInviteNoSdpInd(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = TRUE;
	m_bIsReInvite = TRUE;
	VALIDATEMESSAGEMAP
}

///////////////////////////////////////////////////////
CSipTransReInviteNoSdpInd::~CSipTransReInviteNoSdpInd()
{
}

////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteConnected(CSegment* pParam)
{// different then regular re-invite, when remote send re-invite without SDP, the RMX starting the transaction with its 200 OK and until
	// the invite Ack indication, the RMX doesn't change its working mode.
	PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteConnected, response to remote Re-Invite without SDP");
	m_state		 = sTRANS_RESPONSEREINVITE;
	SetDialState(kOkSent);

	if(m_pSipCntl)
		m_pSipCntl->SipInviteResponseReq(OK_VAL, STATUS_OK, NULL, YES);
}


//////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck");
	DWORD status, isSdp;
	status = isSdp = 0;
	BYTE bRemovedAudio, bRemovedVideo;
	bRemovedAudio = bRemovedVideo = 0;
	*pParam >> status >> isSdp;
	*pParam >> bRemovedAudio >> bRemovedVideo;

	SetDialState(kNotInDialState);
	WORD callIndex = m_pSipCntl ? m_pSipCntl->GetCallIndex() : 0;

	COstrStream msg2;
	m_pTargetMode->Dump(msg2);
	PTRACE2(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck, m_pTargetMode : ", msg2.str().c_str());


	if (status == STATUS_OK)
	{
		if(isSdp == NO)
		{
			PTRACE(eLevelError,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck, No SDP in remote Ack");
			// if we are in re-invite without SDP and we receive invite Ack without SDP we should do one of the two following:
			// 1. ignore the transaction and do nothing.
			// 2. disconnect the call.
			// currently I choose to ignore (more simple, and maybe better solution).

			DBGPASSERT(callIndex);
			EndTransaction(); // not need to rollback because nothing was done (even remote caps haven't been changed).
		}
		else
		{
			PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck: Ack with SDP");
			// we need to close channels (if needed), update bridges and when receive the answer to update RTP and open new channels.

			CSipCaps*	pCurRemoteCaps = m_pSipCntl ? const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps()) : NULL;
			if (pCurRemoteCaps ==  NULL)
			{
				// Yossig Klocwork NPD issue - Noa?
				// No caps in remote SDP, if we got here there should have been..anyway return and notify
				PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck: no remote caps. ignore the transaction");
				DBGPASSERT(callIndex);
				EndTransaction(statNoChange);
				return;
			}
			POBJDELETE(m_pChanDifArr);
			m_pChanDifArr	= new CSipChanDifArr;

			m_pChanDifArr->DeSerialize(NATIVE,*pParam);

			SendOriginalRemoteCapsToParty(pCurRemoteCaps);
			CSipComMode* pBestMode =NULL;
			if( m_pSipCntl )
			{
				if(m_pTargetMode->GetConfType() == kCp)
					 pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetModeMaxAllocation, TRUE,TRUE/*intersect with max caps*/);  //DPA
				else
					pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, TRUE);
			}

			if(pBestMode)
			{
				if ( m_pSipCntl && m_pTargetMode->IsMediaOn(cmCapAudio,cmCapTransmit) && (m_pSipCntl->GetLastRemoteCaps()->IsBfcpSupported()) && (!(pBestMode->IsContent(cmCapReceiveAndTransmit))))
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
				// TIP - Recognition if remote resume the media
				if (m_pParty->GetIsTipCall())
				{
					UpdateIfCallIsResumed();
					RemoteIdent remoteIdent = m_pSipCntl->GetRemoteIdent();

					if( (m_bIsResumeMedia || (remoteIdent == PolycomEp)) )
					{
						if (!CheckIfRemoteVideoRateIsTipCompatible())
						{
							PTRACE2(eLevelInfoNormal,"CSipTransInviteNoSdpInd::OnPartyReceivedAckConnecting drop the call: Name ",m_pPartyConfName);
							m_bIsNeedToDropCall = TRUE;
							m_pTargetMode->SetTipMode(eTipModeNone);
							m_pTargetModeMaxAllocation->SetTipMode(eTipModeNone);
						}
						else if (!::CheckIfRemoteSdpIsTipCompatible(pCurRemoteCaps) )
						{
							PTRACE(eLevelInfoNormal,"CSipTransReInviteWithSdpInd::OnPartyReceivedReInviteConnected -fall back to none TIP");
							FallbackFromTipToNoneTip();
							m_pParty->SetIsTipCall(FALSE);
						}
					}
					if( m_pParty->GetIsTipNegotiationActive() && m_pParty->GetTipPartyOnHold() )
					{
						m_pSipCntl->EndTipNegotiation(eTipNegError);
						m_pParty->SetIsTipNegotiationActive(FALSE);
					}
				}


				// Check if Undefind party and flag, that allow non enc to connect and when available
				// if so, remove SDES from RMX side
				// set enc to off
				//DWORD isDtlsEncrypted = pBestMode->GetIsDtlsEncrypted();

				if (RemoveEncryptionFromScmAndCapsWhenTipResumedIfNeeded(pBestMode))
					PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck: TIP resumed - Encryption removed");





				if (m_pTargetMode->GetConfType()==kCop && (m_pTargetMode->GetCopTxLevel()!=pBestMode->GetCopTxLevel()))
				{
					CSipChanDif* pChanDif = m_pChanDifArr->GetChanDif(cmCapVideo, cmCapTransmit);
					if (pChanDif)
					    pChanDif->SetChangeAlg(YES);
					else
					    PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck - pChanDif is NULL");
				}
				if (m_pTargetMode->IsSdesMediaEquals(*pBestMode, cmCapAudio, cmCapReceive) == FALSE)
				{
					CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapAudio,cmCapReceive, kRolePeople);
					m_pTargetMode->SetSipSdes(cmCapAudio,cmCapReceive,kRolePeople,pSdesCap);
					m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, cmCapAudio, kRolePeople);
				}
				if (m_pTargetMode->IsSdesMediaEquals(*pBestMode, cmCapVideo, cmCapReceive) == FALSE)
				{
					CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapVideo,cmCapReceive, kRolePeople);
					m_pTargetMode->SetSipSdes(cmCapVideo,cmCapReceive,kRolePeople,pSdesCap);
					m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, cmCapVideo, kRolePeople);
				}
				if (m_pTargetMode->IsSdesMediaEquals(*pBestMode, cmCapData, cmCapReceive) == FALSE)
				{
					CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapData,cmCapReceive, kRolePeople);
					m_pTargetMode->SetSipSdes(cmCapData,cmCapReceive,kRolePeople,pSdesCap);
					m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, cmCapData, kRolePeople);
				}
				if (m_pParty->GetIsTipCall() && m_pTargetMode->IsSdesMediaEquals(*pBestMode, cmCapVideo, cmCapReceive,kRoleContentOrPresentation) == FALSE )
				{
					PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck: tip call copy presentation sdes also");
					CSdesCap *pSdesCap = pBestMode->GetSipSdes(cmCapVideo,cmCapReceive, kRoleContentOrPresentation);
					m_pTargetMode->SetSipSdes(cmCapVideo,cmCapReceive,kRoleContentOrPresentation,pSdesCap);
				}

				BYTE bChangeInMedia = YES;
				if ((m_pTargetMode->IsMediaEquals(*pBestMode, cmCapAudio, cmCapReceiveAndTransmit))
						&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit))
						&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation))
						&& (m_pTargetMode->IsMediaEquals(*pBestMode, cmCapData, cmCapReceiveAndTransmit))						
						&& !(m_pTargetMode->GetConfType()==kCop && (m_pTargetMode->GetCopTxLevel()!=pBestMode->GetCopTxLevel()))
						&& (m_pTargetMode->GetConfMediaType()!=eMixAvcSvcVsw))
					bChangeInMedia = NO;

				if (bChangeInMedia)
				{
					PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck: update party control on new caps (update bridges)");
					m_state = sTRANS_RECREINVITEUPDATEBRIDGE;
					StartTimer(UPDATEBRIDGESTOUT, BRIDGES_TIME * SECOND);
					SendReCapsReceivedToParty(pCurRemoteCaps, pBestMode);
				}
				else
				{
					PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck: no change in bridges. continue to close channels stage");
					*m_pTargetMode = *pBestMode;

					if (GetRtpStatisticsIfNeededForReinvite())
						PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck - GetRtpStatisticsIfNeededForReinvite=TRUE");
					else
						PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck - GetRtpStatisticsIfNeededForReinvite=FALSE");

                    if (kCop == m_pTargetMode->GetConfType() && m_pSipCntl)
                        m_pSipCntl->CancelLpr();
				}
			}
			else
			{
				// Best mode is null. Disconnect call.
				PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck: no common mode. ignore the transaction");
				DBGPASSERT(callIndex);
				EndTransaction(statIllegal);
			}

			POBJDELETE(pBestMode);
		}
	}
	else
	{
		PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAck: Ack with bad status - ",status);
		DBGPASSERT(callIndex);
		EndTransaction();  // not need to rollback because nothing was done (even remote caps haven't been changed).
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnConfBridgesUpdatedUpdateBridges(CSegment* pParam)
{
	DWORD status = STATUS_OK;

	//SIP re-invite to check if the change mode succeeded
	*pParam >> status;
	WORD callIndex = m_pSipCntl ? m_pSipCntl->GetCallIndex() : 0;
	if (IsValidTimer(UPDATEBRIDGESTOUT))
	{
		DeleteTimer(UPDATEBRIDGESTOUT);
		PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnConfBridgesUpdatedUpdateBridges: DeleteTimer(UPDATEBRIDGESTOUT) ");
	}
	//if change mode failed
	if (status != STATUS_OK)
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnConfBridgesUpdatedUpdateBridges, bridges failed to support new mode",m_pPartyConfName);
		DBGPASSERT(callIndex);
		EndTransaction(); // Error while change mode. Call will be disconnected by conf.
	}
	else
	{
		PTRACE2(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnConfBridgesUpdatedUpdateBridges, accept new transaction by bridges. Name ",m_pPartyConfName);
		m_state = sTRANS_RECREINVITECLOSECHANN;

        m_pTargetMode->DeSerialize(NATIVE,*pParam);

		UdpAddresses sUdpAddressesParams;
		pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));
		if(m_pSipCntl)
			m_pSipCntl->SetNewUdpPorts(sUdpAddressesParams);
		BYTE bIsContentSpeaker = FALSE;
		*pParam >> bIsContentSpeaker;
		CheckContentChangesInConfResponse(bIsContentSpeaker);

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

		if (GetRtpStatisticsIfNeededForReinvite())
			PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnConfBridgesUpdatedUpdateBridges - GetRtpStatisticsIfNeededForReinvite=TRUE");
		else
			PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnConfBridgesUpdatedUpdateBridges - GetRtpStatisticsIfNeededForReinvite=FALSE");

	}
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteNoSdpInd::OnPartyChannelsDisconnectedRecReInviteCloseChann, all needed channels are closed. Request to update channels if needed - ", m_pPartyConfName);
	UpdateDbChannelsStatus(pParam, FALSE);
	m_state = sTRANS_RECREINVITEUPDATECHANN;
	UpdateChannelsIfNeeded();
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteNoSdpInd::OnPartyChannelsUpdatedRecReInviteUpdateChann, all needed channels are updated. Request to open channels if needed - ", m_pPartyConfName);
	m_state = sTRANS_RECREINVITEOPENCHANN;
	BYTE isAnsweringToNewCap = FALSE;
	OpenChannelsIfNeededForReInvite(isAnsweringToNewCap,cmCapReceiveAndTransmit);
}

///////////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartyChannelsOpenRecReInviteOpenChann(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipTransReInviteNoSdpInd::OnPartyChannelsOpenRecReInviteOpenChann, all needed channels are open. Response to Re-Invite request - ",m_pPartyConfName);

	//very bad solution but because of time and not enough understanding on how all the function behave in all the state I'm writting all the actions here
	UpdateDbChannelsStatus(pParam, TRUE);
	*m_pTargetMode = *m_pCurrentMode;

	if ( m_pParty->GetIsTipCall() )
	{
		if( m_needToWaitForSlavesEndChangeMode )
		{
			m_isNeedToEndTransAfterTipSlavesAck = TRUE;
			PTRACE2(eLevelInfoNormal, "CSipTransReInviteWithSdpInd::SendReInviteResponse, waiting for slaves to end change mode - ",m_pPartyConfName);
			return;
		}
		if (!(GetIsTipResumeMedia()))
			SendMuteMediaToParty(cmCapReceiveAndTransmit);// instead of MuteMediaIfNeeded(cmCapReceiveAndTransmit);
		else
		{
			//m_bIsTipMute =TRUE;
			// After channels out were opened, start DTLS if needed.
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

				//if (m_pCurrentMode->GetIsEncrypted() == Encryp_On && bAllowDtlsFlag && !m_bTipEarlyPacket)
				if (m_pCurrentMode->GetIsDtlsEncrypted() == Encryp_On && bAllowDtlsFlag && !m_bTipEarlyPacket) //BRIDGE-14495
				{
					m_pCurrentMode->SetDtlsEncryption(Encryp_On);
					m_pCurrentMode->CreateLocalSipComModeDtls(Encryp_On, TRUE /*is tip compatible*/);
					m_pTargetMode->SetDtlsEncryption(Encryp_On);
					m_pTargetMode->CreateLocalSipComModeDtls(Encryp_On, TRUE /*is tip compatible*/);
				}
			}

			if (StartDtlsIfNeeded())
				return;
			//else
			//	SendMuteMediaToParty(cmCapReceiveAndTransmit);

		}

	}
	else
		SendMuteMediaToParty(cmCapReceiveAndTransmit);


	EndTransaction();
}

/////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAnycase: connect call");
	if(m_pSipCntl)
		m_pSipCntl->SipInviteResponseReq(SipCodesRequestPending);// response with 491
}

/////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAckAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartyReceivedReInviteAckAnycase: connect call");
}

/////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnReInviteUpdateBridgesTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnReInviteUpdateBridgesTout");
	WORD callIndex = m_pSipCntl ? m_pSipCntl->GetCallIndex() : 0;
	DBGPASSERT(callIndex);
	EndTransaction(statIllegal);
}
///////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnConfSetCapsAccordingToNewAllocation(CSegment* pParam)
{
	SetCapsAccordingToNewAllocation(pParam);
	m_bNeedReInviteForReAlloc = TRUE;  //is this needed noa?
}
///////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartySlavesRecapIsFinished(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnPartySlavesRecapIsFinished");
	if(m_needToWaitForSlavesEndChangeMode && m_isNeedToEndTransAfterTipSlavesAck )
	{
		PTRACE(eLevelError,"CSipTransReInviteNoSdpInd::OnPartySlavesRecapIsFinished -finish trnas after ack from slaves");
		if (!(GetIsTipResumeMedia()))
			SendMuteMediaToParty(cmCapReceiveAndTransmit);// instead of MuteMediaIfNeeded(cmCapReceiveAndTransmit);
		else
		{
			if(m_bIsResumeMedia)
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
			}
			if (m_pParty->GetIsTipCall() && StartDtlsIfNeeded())
			{
				m_isNeedToEndTransAfterTipSlavesAck = FALSE;
				m_needToWaitForSlavesEndChangeMode = FALSE;
				return;
			}
		}

		m_isNeedToEndTransAfterTipSlavesAck = FALSE;
		EndTransaction();
	}

	m_needToWaitForSlavesEndChangeMode = FALSE;
}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnDtlsChannelsUpdated()
{
			EndTransaction();
}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnPartyRecStatisticInfo(CSegment* pParam)
{
//	m_state = m_oldstate;
    PTRACE2INT(eLevelInfoNormal,"CDR_MCCF: CSipTransReInviteNoSdpInd::OnPartyRecStatisticInfo m_state:",m_state);
    CloseDtlsChannelsBeforeSipChannelsIfNeeded();
}
////////////////////////////////////////////////////////////////////////////////
void CSipTransReInviteNoSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam)
{
	PTRACE2INT(eLevelInfoNormal,"CSipTransReInviteNoSdpInd::OnDtlsClosedChannelBeforeSipCloseChannels - m_state:", m_state);

	m_state = sTRANS_RECREINVITECLOSECHANN;
	CloseChannelsIfNeededForReceiveReInvite();
}
