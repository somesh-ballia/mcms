/*
 * SIPTransInviteMrcSlaveWithSdpReq.cpp
 *
 *  Created on: Sep 10, 2013
 *      Author: shmuell
 */
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
#include "SIPTransInviteMrcSlaveWithSdpReq.h"
////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransInviteMrcSlaveWithSdpReq)
 ONEVENT(SIP_PARTY_ESTABLISH_CALL,			IDLE,						CSipTransInviteMrcSlaveWithSdpReq::OnPartyEstablishCallIdle)
 ONEVENT(SIP_PARTY_RECEIVED_200OK,			sTRANS_CONNECTING,			CSipTransInviteMrcSlaveWithSdpReq::OnPartyReceived200OkConnecting)
 ONEVENT(SIP_PARTY_RECEIVED_REINVITE,		sTRANS_WAIT_FOR_REINVITE,	CSipTransInviteMrcSlaveWithSdpReq::OnPartyReceivedReInviteWaitForReInvite)
 ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENCHANNELS,		CSipTransInviteMrcSlaveWithSdpReq::OnPartyChannelsConnectedOpenChannels)
 ONEVENT(AUDBRDGCONNECT,					sTRANS_OPENBRIDGES,			CSipTransaction::OnConfPartyReceiveAudBridgeConnected)
 ONEVENT(VIDBRDGCONNECT,					sTRANS_OPENBRIDGES,			CSipTransaction::OnConfPartyReceiveVidBridgeConnected)
 ONEVENT(FECCBRDGCONNECT,					sTRANS_OPENBRIDGES,			CSipTransaction::OnConfPartyReceiveFeccBridgeConnected)

 ONEVENT(SIP_PARTY_RECEIVED_ACK,			sTRANS_CONNECTING,			CSipTransInviteMrcSlaveWithSdpReq::OnPartyReceivedReInviteAck)
PEND_MESSAGE_MAP(CSipTransInviteMrcSlaveWithSdpReq, CSipTransaction);

/////////////////////////////////////////////////////////////////////////////////////////
CSipTransInviteMrcSlaveWithSdpReq::CSipTransInviteMrcSlaveWithSdpReq(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = FALSE;
	m_bIsReInvite = FALSE;
	m_bIsToCloseVideoChannels 	= FALSE;
	m_bIsToCloseDataChannels 	= FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
CSipTransInviteMrcSlaveWithSdpReq::~CSipTransInviteMrcSlaveWithSdpReq()
{

}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcSlaveWithSdpReq::OnPartyEstablishCallIdle(CSegment* pParam)
{
	m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone, FALSE, TRUE);
	m_pSipCntl->SipInviteReq(m_pAlternativeAddrStr);
	m_state = sTRANS_CONNECTING;
	SetDialState(kInviteSent);
}

///////////////////////////////////////////////////////////////////
void CSipTransInviteMrcSlaveWithSdpReq::OnPartyReceived200OkConnecting(CSegment* pParam)
{
	m_pSipCntl->SipInviteAckReq();
	m_state = sTRANS_WAIT_FOR_REINVITE;
	m_bIsNeedToPendOtherTransaction = FALSE;
}

///////////////////////////////////////////////////////////////////
void CSipTransInviteMrcSlaveWithSdpReq::OnPartyReceivedReInviteWaitForReInvite(CSegment* pParam)
{
	m_bIsNeedToPendOtherTransaction = TRUE;
	m_state = sTRANS_OPENCHANNELS;

	BYTE	bCapsDontMatch	= NO;
	DWORD	isDtlsEncrypted	= Encryp_Off;
	DWORD	isEncrypted		= Encryp_Off;

	CSipComMode* pBestMode	= m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, FALSE);
	if (pBestMode)
	{
		pBestMode->CopyStreamListToStreamGroup(cmCapVideo, kRolePeople, cmCapReceive, cmCapReceive);
		pBestMode->CopyStreamListToStreamGroup(cmCapVideo, kRolePeople, cmCapTransmit, cmCapReceive);
		pBestMode->CopyStreamListToStreamGroup(cmCapAudio, kRolePeople, cmCapReceive, cmCapReceive);
		//pBestMode->CopyStreamListToStreamGroup(cmCapAudio, kRolePeople, cmCapTransmit, cmCapReceive);

		//Check encryption
		isEncrypted = pBestMode->GetIsEncrypted();
		if(Encryp_On == isEncrypted)
		{
			// Check if Undefind party and flag, that allow non enc to connect and when available
			// if so, remove SDES from RMX side
			// set enc to off
			BOOL	bIsUndefinedParty = m_pSipCntl->IsUndefinedParty();
			BYTE	bIsDisconnectOnEncFailure = pBestMode->GetIsDisconnectOnEncryptionFailure();
			BYTE	bIsWhenAvailableEncMode = m_pSipCntl->IsWhenAvailableEncryptionMode();
			DWORD	isDtlsEncrypted = pBestMode->GetIsDtlsEncrypted();

			cmCapDataType	mediaType;
			ERoleLabel		eRole;

			for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++)
			{
				GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

				if (mediaType == cmCapBfcp)
					continue;

				if ( pBestMode->IsMediaOn(mediaType, cmCapTransmit, eRole) )
				{
					CSdesCap *pSdesCap =  pBestMode->GetSipSdes(mediaType, cmCapReceive, eRole);
					if (!pSdesCap)
					{
						if( (mediaType == cmCapData) && (pBestMode->GetSipSdes(cmCapAudio, cmCapReceive)) )
						{
				             TRACEINTO << "sdes available for audio but NOT for fecc - remove FECC ";
				             m_pSipCntl->RemoveFeccCaps();
				             pBestMode->RemoveData(cmCapReceiveAndTransmit);
						}
						else if ( ( bIsUndefinedParty && (bIsDisconnectOnEncFailure == NO) && bIsWhenAvailableEncMode ) ||
							      ( isDtlsEncrypted == Encryp_On ) )
						{
							pBestMode->RemoveSipSdes(mediaType, cmCapTransmit, eRole);
							pBestMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
							isEncrypted = Encryp_Off;
							m_pSipCntl->RemoveSdesCapFromLocalCaps(mediaType, eRole);
						}
						else
						{
							bCapsDontMatch = YES;
						}
					} // end if (!pSdesCap)

					else // pSdesCap
					{
						//remove unchosen SDES from local caps
						m_pSipCntl->RemoveUnsupportedSdesCapFromLocalCaps(pSdesCap->GetSdesCryptoSuite(), pSdesCap->GetIsSdesMkiInUse(0) ,mediaType, eRole);

						//upate  SDES tag from bestMode to local caps
						m_pSipCntl->UpdateSdesTagFromBestModeToLocalCaps(pSdesCap, mediaType, eRole);
						m_pSipCntl->UpdateLocalCapsSdesUnencryptedSrtcp(pBestMode, mediaType, eRole);
					}
				} // end if (pBestMode->IsMediaOn)
			} // loop on MAX_SIP_MEDIA_TYPES
		} // end if (isEncrypted==Encryp_On)

		else if (Encryp_On == isDtlsEncrypted)
		{
			/* verify if we need to disconnect the call in case of DTLS encryption and not TIP call */
			if ( RejectDTLSEncIfNeeded(pBestMode->GetIsDisconnectOnEncryptionFailure()) )
			{
				TRACEINTO << "RejectDTLSEncIfNeeded";
				POBJDELETE(pBestMode);
				return;
			}
		}
	} // end if (pBestMode)

	if ( (NO == bCapsDontMatch)		&&
		 (Encryp_On == isEncrypted)	&&
		 pBestMode )
	{
		SendUpdateDbEncryptionStatusToParty(YES);
	}


	if ( (NO == bCapsDontMatch)							&&
		 pBestMode										&&
		 pBestMode->IsMediaOn(cmCapAudio,cmCapReceive)	&&
		 pBestMode->IsMediaOn(cmCapAudio,cmCapTransmit) )
	{
		pBestMode->Dump("CSipTransInviteMrcSlaveWithSdpReq::OnPartyReceivedReInviteWaitForReInvite : Best Mode",eLevelInfoNormal);

		// update media from best mode to target mode
		*((CIpComMode*)m_pTargetMode) = *((CIpComMode*)pBestMode);

		// added to synchronize SSRC streams, real solution is to implement update flow
		*((CIpComMode*)m_pCurrentMode) = *((CIpComMode*)pBestMode);

//		m_state = sTRANS_OPENCHANNELS;

		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone);

		//inform conf (via party)
		SendRemoteCapsReceivedToParty();
	}

	else
	{
		TRACEINTO << "Reject call";
		bCapsDontMatch = YES;
	}

	POBJDELETE(pBestMode);


	if (bCapsDontMatch)
	{
		SetDialState(kCapsDontMatch);
		EndTransaction(SIP_CAPS_DONT_MATCH);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcSlaveWithSdpReq::OnPartyChannelsConnectedOpenChannels(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteMrcSlaveWithSdpReq::OnPartyChannelsConnectedOpenChannels: Name ",m_pPartyConfName);

	m_state = sTRANS_OPENBRIDGES;
	StartTimer(OPENBRIDGESTOUT, 10*SECOND);
	InformChannelsConnectedOpenBridges(pParam);

}
/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteMrcSlaveWithSdpReq::HandleBridgeConnectedInd(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::HandleBridgeConnectedInd: Name ",m_pPartyConfName);

	if (status == STATUS_OK)
	{
		// its OK if audio, video and FECC or audio and video without FECC bridges connected or its audio only call or there is no much in the video capability
		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected /*&& m_isFeccBridgeConnected*/) || // hg and Shmuel - currently MRC does not support FECC
			(m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_pTargetMode->IsMediaOff(cmCapData,cmCapReceiveAndTransmit)) ||
			(m_isAudioBridgeConnected && GetIsVoice()) ||
			(m_isAudioBridgeConnected && m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit)))
		{
			if (IsValidTimer(OPENBRIDGESTOUT))
			{
				DeleteTimer(OPENBRIDGESTOUT);
				PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpInd::HandleBridgeConnectedInd: DeleteTimer(OPENBRIDGESTOUT) ");
			}

			m_state		 = sTRANS_CONNECTING;

			SendMuteMediaToParty(cmCapReceiveAndTransmit);// instead of MuteMediaIfNeeded(cmCapReceiveAndTransmit);
			SetDialState(kOkSent);
			m_pSipCntl->SendRemoteNumbering();
			m_pSipCntl->SipInviteResponseReq(OK_VAL);

			if(IsNeedToSendRtcpVideoPreference())
				SendStartVideoPreferenceToParty();

		}
		else if(m_isAudioBridgeConnected)
			PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpInd::HandleBridgeConnectedInd: Is Voice Call? - ",GetIsVoice());
	}
	else
	{
//		m_eDialState = kBadStatusAckArrived;
		DBGPASSERT(status);
		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpInd::HandleBridgeConnectedInd: Ack with bad status - ",status);
		EndTransaction(status);
	}
}

//////////////////////////////////////////////////////
void CSipTransInviteMrcSlaveWithSdpReq::OnPartyReceivedReInviteAck(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteMrcSlaveWithSdpReq::OnPartyReceivedReInviteAck");
	EndTransaction();
}
