//+========================================================================+
//                SIPTransInviteWithSdpInd.cpp             	  			   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteWithSdpInd.cpp                            	   |
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
#include "SIPTransInviteWithSdpInd.h"

////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransInviteWithSdpInd)

ONEVENT(SIP_PARTY_ESTABLISH_CALL,			IDLE,					CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENCHANNELS,	CSipTransInviteWithSdpInd::OnPartyChannelsConnectedOpenChannels)
ONEVENT(SET_SITE_AND_VISUAL_NAME,			sTRANS_OPENCHANNELS,	CSipTransaction::OnPartySendSiteAndVisualNamePlusProductIdToPartyControl)

ONEVENT(AUDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveAudBridgeConnected)
ONEVENT(VIDBRDGCONNECT,						sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveVidBridgeConnected)
ONEVENT(FECCBRDGCONNECT,					sTRANS_OPENBRIDGES,		CSipTransaction::OnConfPartyReceiveFeccBridgeConnected)
ONEVENT(SIP_PARTY_CHANS_CONNECTED,			sTRANS_OPENBRIDGES,		CSipTransInviteWithSdpInd::OnPartyChannelsConnectedOpenBridges)

ONEVENT(SIP_PARTY_RECEIVED_ACK,				sTRANS_CONNECTING,		CSipTransInviteWithSdpInd::OnPartyReceivedAckConnecting)
// party events
ONEVENT(PARTYCONNECTTOUT,					sTRANS_CONNECTING,		CSipTransaction::OnPartyConnectToutConnecting)

ONEVENT(OPENBRIDGESTOUT,					sTRANS_OPENBRIDGES,		CSipTransInviteWithSdpInd::OnConfBridgesConnectionTout)

ONEVENT(SET_CAPS_ACCORDING_TO_NEW_ALLOCATION,ANYCASE,				CSipTransaction::SetCapsAccordingToNewAllocation)
ONEVENT(REMOVE_AVC_TO_SVC_ART_TRANSLATOR,  sTRANS_OPENBRIDGES,     CSipTransaction::OnRemoveAvcToSvcArtTranslatorAnycase)
ONEVENT(PARTY_TRANSLATOR_ARTS_DISCONNECTED, sTRANS_OPENBRIDGES, CSipTransaction::OnPartyTranslatorArtsDisconnected)

ONEVENT(MAKE_ANSWER_IND,					sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpInd::OnIceInviteMakeAnsArrivedFromIceStack)
ONEVENT(ICEGENERALTOUT,  					sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpInd::OnICEGeneralToutWaitForCandidates)
ONEVENT(ICEPORTSRETRYTOUT,    			sTRANS_WAITFORICECANDIDATES,    CSipTransInviteWithSdpInd::OnIcePortsRetryTout)
ONEVENT(CLOSE_ICE_SESSION_IND,          sTRANS_WAITFORICECANDIDATES,	CSipTransInviteWithSdpInd::OnICEReceiveCloseIceInd)
ONEVENT(ICECOMPLETETOUT,  					ANYCASE,						CSipTransInviteWithSdpInd::OnICECompleteTout)

ONEVENT(TRANS_ICE_CONN_CHECK_COMPLETE_IND,  ANYCASE,                        CSipTransInviteWithSdpInd::IceConnectivityCheckComplete)
ONEVENT(CLOSE_ICE_SESSION_IND,          	ANYCASE,				CSipTransInviteWithSdpInd::OnICEReceiveCloseIceInd)
//ONEVENT(ICE_CONN_CHECK_COMPLETE_IND,    ANYCASE,                        CSipTransInviteWithSdpInd::IceConnectivityCheckComplete)

ONEVENT(SIP_PARTY_SLAVES_RECAP_FINISHED,	ANYCASE,					CSipTransInviteWithSdpInd::OnPartySlavesRecapIsFinished)

ONEVENT(SIP_PARTY_DTLS_STATUS,				sTRANS_DTLS_STARTED,						CSipTransaction::OnPartyDtlsEndInd)
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			sTRANS_DTLS_STARTED,						CSipTransaction::DisconnectOnDtlsEncryptFail)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_UPDATED_CHAN, 					CSipTransInviteWithSdpInd::PartyConnectCall)
ONEVENT(PARTYCONNECTTOUT,					sTRANS_DTLS_UPDATED_CHAN, 					CSipTransaction::OnPartyConnectToutConnecting)
ONEVENT(DTLSTOUT,							sTRANS_DTLS_STARTED,						CSipTransaction::OnDtlsTout)
ONEVENT(SIP_PARTY_CHANS_UPDATED,			sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransInviteWithSdpInd::PartyConnectCall)
ONEVENT(SIP_PARTY_DTLS_CHANS_DISCONNECTED,	sTRANS_DTLS_CLOSED_CHAN_AFTER_DTLS_FAILURE, CSipTransInviteWithSdpInd::PartyConnectCall) //BRIDGE-6184
ONEVENT(SIP_PARTY_TIP_EARLY_PACKET,			ANYCASE,									CSipTransaction::TipEarlyPacketDtlsNotNeeded)
ONEVENT(SIP_PARTY_PENDING_TRANS,			sTRANS_DTLS_STARTED,						CSipTransaction::OnSipNotifyPendingTransactionDtlsStarted) //BRIDGE-15745


PEND_MESSAGE_MAP(CSipTransInviteWithSdpInd,CSipTransaction);

////////////////////////////////////////////////////////////////////////////
CSipTransInviteWithSdpInd::CSipTransInviteWithSdpInd(CTaskApp *pOwnerTask):CSipTransaction(pOwnerTask)
{
	m_bIsOfferer = FALSE;
	m_bIsReInvite = FALSE;
}

////////////////////////////////////////////////////////////////////////////
CSipTransInviteWithSdpInd::~CSipTransInviteWithSdpInd()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::DoMakeANewCallOnPartyEstablishCallIdle(CSipComMode * pBestMode)
{
	m_state = sTRANS_OPENCHANNELS;
	SetIsNeedToSendReInviteforFullAudioCapsAccordingToUserAgentAndVersion();

	if(m_pTargetMode->GetIsTipMode())
		m_pSipCntl->MakeANewCall(pBestMode, eTipMasterCenter);
	else
		m_pSipCntl->MakeANewCall(pBestMode, eTipNone);

	//inform conf (via party)
	SendRemoteCapsReceivedToParty();
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle: Name ");

	//const CSipCaps *pLocalCapsCheck = m_pSipCntl->GetLocalCaps();
	//CLargeString str;
	//pLocalCapsCheck->DumpToString(str);
	//PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle, Local Caps : ",str.GetString());

	BYTE bCapsDontMatch = FALSE;
	CSipCaps*	pRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());

	if (pRemoteCaps && pRemoteCaps->GetNumOfCapSets())
	{

		//update SDP in operator (connection info window)
		PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle: Get best mode");
		SendOriginalRemoteCapsToParty(pRemoteCaps);
		m_pSipCntl->CompleteRemoteCapDataFromLocalCap();

		if ( kCp == m_pTargetMode->GetConfType() || kCpQuad == m_pTargetMode->GetConfType() )
		{// for dial in we change the remote caps according to system setting
			BYTE isRmxInitiateTransaction = FALSE;
			BYTE bFixVideoRate = m_pSipCntl->CheckIsMobilePhoneByHeader(isRmxInitiateTransaction);
			pRemoteCaps->FixRemoteCapsBySystemSettings(m_pSipCntl->GetRmtHeaders(), bFixVideoRate);
		}
		m_pSipCntl->SetLastRemoteCaps(*pRemoteCaps);
		RemoveBfcpAccordingToRemoteIdent();
		RemoveBFCPIfTransportUnsupported();
		
		m_pSipCntl->RemoveUnsupportedSdesCapsForCiscoCallIfNeeded();
		m_pSipCntl->RemoveUnsupportedSdesCapsForRadVisionCallIfNeeded();

		// TIP
		if (m_pTargetMode->GetIsTipMode())
		{
            const CSipCaps* pCurRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
            CCommConf*      pCommConf      = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
            BYTE            isPreferTip    = FALSE;

            if (pCommConf && pCommConf->GetIsTipCompatible() == eTipCompatiblePreferTIP)
            {
                isPreferTip = TRUE;
                PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle PREFER TIP FOR POLYCOM EP");
            }
            else
            {
                PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle !DONOT! PREFER TIP FOR POLYCOM EP");
                if (pCommConf)
                    PTRACE2INT(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle !DONOT! PREFER TIP FOR POLYCOM EP TipCompatible:",pCommConf->GetIsTipCompatible());
                else
                    PTRACE(eLevelError, "IS_PREFER_TIP_MODE: CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle pCommConf is NULL");
            }

            if ( (!::CheckIfRemoteSdpIsTipCompatible(pCurRemoteCaps) && isPreferTip==FALSE) || !(pCurRemoteCaps->GetIsContainingCapCode(cmCapAudio,eAAC_LDCapCode)) ) //if (!CheckIfRemoteSdpIsTipCompatible())
			{
				PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle fall back from TIP to SIP: Name ",m_pPartyConfName);
		//		m_bNeedReInviteForSwitchToNoneTipCall = TRUE;
				FallbackFromTipToNoneTip();
		}
            else if (!CheckIfRemoteVideoRateIsTipCompatible() || IsNeedToRejectTheCallForPreferTIPmode(pCurRemoteCaps))
			{
				PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle drop the call: Name ",m_pPartyConfName);
//				m_bIsNeedToDropCall = TRUE;
				m_pTargetMode->SetTipMode(eTipModeNone);
				m_pTargetModeMaxAllocation->SetTipMode(eTipModeNone);
			}
		}

		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle - BFCP is:", m_pTargetMode->IsMediaOn(cmCapBfcp));
		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle - BFCP transport type is:", m_pTargetMode->GetBfcpTransportType());

		CSipComMode * pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, FALSE);

		if (kCop == m_pTargetMode->GetConfType())
		{
			const CSipCaps *pLocalCaps = m_pSipCntl->GetLocalCaps();
			if (pBestMode
				&& ((CapEnum)pBestMode->GetMediaType(cmCapVideo, cmCapReceive, kRolePeople) == eH264CapCode)
				&& ((CapEnum)pBestMode->GetMediaType(cmCapVideo, cmCapTransmit, kRolePeople) == eH263CapCode)
				&& pLocalCaps && pLocalCaps->IsCapSet(eH263CapCode))
			{
				// Prevent Rx 264 and Tx 263, because after Cop change mode, the remote may declare only 264 and we will not have video Tx.
				PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle: Change receive mode to h263");
				const capBuffer* newMediaMode = pLocalCaps->GetCapSetAsCapBuffer(eH263CapCode);
				if (newMediaMode)
				{
					m_pTargetMode->SetMediaMode(newMediaMode, cmCapVideo, cmCapReceive, kRolePeople);
					pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, FALSE);
				}
			}
		}

		//Check encryption
		DWORD encrypteStatus = Encryp_Off;
		if(pBestMode && !m_pSipCntl->IsWebRtcCntl()) {
			encrypteStatus = pBestMode->GetIsEncrypted();
			// Check if Undefind party and flag, that allow non enc to connect and when available
			// if so, remove SDES from RMX side
			// set enc to off
			BOOL bIsUndefinedParty = m_pSipCntl->IsUndefinedParty();
			BYTE bIsDisconnectOnEncFailure = pBestMode->GetIsDisconnectOnEncryptionFailure();
			BYTE bIsWhenAvailableEncMode = m_pSipCntl->IsWhenAvailableEncryptionMode();
			DWORD dtlsEncrypteStatus = pBestMode->GetIsDtlsEncrypted();

			if(encrypteStatus == Encryp_On ) {
				cmCapDataType mediaType;
				ERoleLabel eRole;

				for(int i = 0; i < MAX_SIP_MEDIA_TYPES; i++) {

					GetMediaDataTypeAndRole(globalMediaArr[i], mediaType, eRole);

					if (mediaType == cmCapBfcp)
						continue;

					if (pBestMode->IsMediaOn(mediaType,cmCapTransmit,eRole)) {
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
							else if( (bIsUndefinedParty && bIsDisconnectOnEncFailure==NO && bIsWhenAvailableEncMode) ||
								( dtlsEncrypteStatus == Encryp_On ) )
							{
								pBestMode->RemoveSipSdes(mediaType,cmCapTransmit,eRole);
								pBestMode->SetEncryption(Encryp_Off, bIsDisconnectOnEncFailure);
								encrypteStatus = Encryp_Off;
								m_pSipCntl->RemoveSdesCapFromLocalCaps(mediaType, eRole);
							}
							else
								bCapsDontMatch = YES;
						}
						else
						{
							UpdateLocalCapsWithEncryptionParameters(pBestMode, mediaType , eRole);	//BRIDGE-10820
						}
					}
				}
			}
			else if (dtlsEncrypteStatus == Encryp_On)
			{
				/* verify if we need to disconnect the call in case of DTLS encryption and not TIP call */
				if (RejectDTLSEncIfNeeded(pBestMode->GetIsDisconnectOnEncryptionFailure()))
				{
					POBJDELETE(pBestMode);
					return;
				}
			}
		}

		if(pBestMode && encrypteStatus == Encryp_On && bCapsDontMatch == NO)
			SendUpdateDbEncryptionStatusToParty(YES);

		if (bCapsDontMatch == NO && pBestMode && pBestMode->IsMediaOn(cmCapAudio,cmCapReceive) && pBestMode->IsMediaOn(cmCapAudio,cmCapTransmit))
		{
			PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle: mode found");
			pBestMode->SetConfType(m_pTargetMode->GetConfType());
			*m_pTargetMode = *pBestMode;

			if (m_pSipCntl->GetIsEnableICE())
			{
				if( SendIceMgsReqAccordingToTargetMode(ICE_MAKE_ANSWER_REQ) == STATUS_OK )
				{
					PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle - Wait seconds for Candidates before continue transaction.  ",
							MAKE_ICE_CANDIDATES_TIMER/SECOND);
					m_state = sTRANS_WAITFORICECANDIDATES;
					StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
				}
				else
				{
					PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle - Failed to start ice call: Name ",m_pPartyConfName);
					m_pSipCntl->SetIsEnableICE(FALSE);
					ContinueToOpenChannels();
				}
			}
			else
			{
				if (MakeANewCallOnPartyEstablishCallIdle(pBestMode))
					DoMakeANewCallOnPartyEstablishCallIdle(pBestMode);
			}
		}
		else
		{
			PTRACE(eLevelError,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle: No best mode found. must reject call");
			bCapsDontMatch = YES;
		}
		POBJDELETE(pBestMode);
	}
	else
	{
		PTRACE2(eLevelError,"CSipTransInviteWithSdpInd::OnPartyEstablishCallIdle: No Remote capabilities!!! Name ",m_pPartyConfName);
		bCapsDontMatch = YES;
	}

	if (bCapsDontMatch)
	{
		SetDialState(kCapsDontMatch);
		EndTransaction(SIP_CAPS_DONT_MATCH);
	}
}

////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnIceInviteMakeAnsArrivedFromIceStack(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnIceInviteMakeAnsArrivedFromIceStack: Name ",m_pPartyConfName);

	// we don't delete here the timer ICEGENERALTOUT - we wait for connectivity check.
	if (IsValidTimer (ICEGENERALTOUT))
	   DeleteTimer(ICEGENERALTOUT);

	WORD status = STATUS_OK;
	DWORD newRate = 0;

	*pParam >> status;
	*pParam >> newRate;

	if (status == STATUS_OK)
	{
		StartTimer(ICECOMPLETETOUT, MAKE_ICE_CANDIDATES_TIMER);
		if( newRate != 0 )
		{
			DWORD ms_cac_min_video_threshold_rate = (GetSystemCfgFlagInt<DWORD>(CFG_KEY_MS_CAC_VIDEO_MIN_BR))*10;

	        if(newRate < ms_cac_min_video_threshold_rate)
	        {
	        	PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnIceInviteMakeAnsArrivedFromIceStack: will be audio only call");
	        }
	        else
	        {
	        	if (m_pTargetMode->GetConfType() == kCp &&
	        	    (m_pTargetMode->GetConfMediaType()!=eMixAvcSvcVsw || m_pSipCntl->GetIsMrcCall()) &&
	        		(newRate < (DWORD)m_pTargetMode->GetVideoBitRate(cmCapReceiveAndTransmit, kRolePeople))) /* vngr-25846 - do not update the rate incase of vsw */
	        		m_pTargetMode->SetVideoBitRate(newRate,cmCapReceiveAndTransmit);
	        }
		}

		ContinueToOpenChannels();
	}
	else if (m_IceMakeOfferAnswerCounter < 3)
	{
		m_IceMakeOfferAnswerCounter++;
	    DWORD retryTimer = 0;
	    CSysConfig* pSysConfig = CProcessBase::GetProcess()->GetSysConfig();
	    pSysConfig->GetDWORDDataByKey("ICE_RETRY_TIMER_IN_SECONDS", retryTimer);
		m_pSipCntl->SetIsEnableICE(TRUE);
		m_pSipCntl->CloseIceSession();
		StartTimer(ICEPORTSRETRYTOUT, retryTimer*SECOND);
		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnIceInviteMakeAnsArrivedFromIceStack: counter ",m_IceMakeOfferAnswerCounter);
		return;
	}
	else
	{
		m_IceMakeOfferAnswerCounter=0;
		m_pSipCntl->SetIsEnableICE(FALSE);
		m_pSipCntl->CloseIceSession();
	}
}
////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnICEGeneralToutWaitForCandidates(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,
			"CSipTransInviteWithSdpInd::OnICEGeneralToutWaitForCandidates - Failed to start ice call: Name ",
			m_pPartyConfName);
	PTRACE2INT(eLevelInfoNormal, "after seconds", MAKE_ICE_CANDIDATES_TIMER/SECOND);

	m_pSipCntl->SetIsEnableICE(FALSE);
	m_pSipCntl->CloseIceSession();
}
////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnICECompleteTout(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnICECompleteTout - Failed to start ice call: Name ",m_pPartyConfName);

	EndTransaction(SIP_NO_ADDR_FOR_MEDIA);
}
////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::IceConnectivityCheckComplete(CSegment * pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpInd::IceConnectivityCheckComplete");

	CSegment* CopyOfseg = new CSegment(*pParam);

	ICE_CHECK_COMPLETE_IND_S* pConnCheckStruct = (ICE_CHECK_COMPLETE_IND_S*) pParam->GetPtr(1);
	int status = pConnCheckStruct->status;
	PTRACE2INT (eLevelInfoNormal, "CSipTransInviteWithSdpInd::IceConnectivityCheckComplete - ", status);

	if (IsValidTimer(ICECOMPLETETOUT))
		DeleteTimer(ICECOMPLETETOUT);

//	SendHandleIceConnectivityCheckCompleteToParty(CopyOfseg);

//	POBJDELETE(CopyOfseg);

}

////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnICEReceiveCloseIceInd(CSegment * pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpInd::OnICEReceiveCloseIceInd");

	ContinueToOpenChannels();
}

////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::ContinueToOpenChannels()
{
	SetIsNeedToSendReInviteforFullAudioCapsAccordingToUserAgentAndVersion();
	m_state = sTRANS_OPENCHANNELS;
	if(m_pTargetMode->GetIsTipMode())
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipMasterCenter);
	else
		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode, eTipNone);
	//inform conf (via party)
	SendRemoteCapsReceivedToParty();
}
////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnPartyChannelsConnectedOpenChannels(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyChannelsConnectedOpenChannels: Name ",m_pPartyConfName);

	if( m_pTargetMode->GetIsTipMode() )
		m_bIsTipMute = TRUE;

	m_state = sTRANS_OPENBRIDGES;
	StartTimer(OPENBRIDGESTOUT, 10*SECOND);
	InformChannelsConnectedOpenBridges(pParam);
}

////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnPartyChannelsConnectedOpenBridges(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyChannelsConnectedOpenBridges: Name ",m_pPartyConfName);
	HandleBridgeConnectedInd(STATUS_OK);
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::HandleBridgeConnectedInd(DWORD status)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::HandleBridgeConnectedInd: Name ",m_pPartyConfName);

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
				PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpInd::HandleBridgeConnectedInd: DeleteTimer(OPENBRIDGESTOUT) ");
			}
			if (GetDialState() == kBeforeOkInConf)
			{
				if (m_pSipCntl->ShouldUpdateMrmpPhysicalIdInfo())
				{
					TRACEINTO << "Update MRMP channel";
					if (m_pSipCntl->UpdateMrmpInternalChannelIfNeeded())
					{
						return;
					}
				}

				TRACEINTO << "Send 200 OK";

				m_state		 = sTRANS_CONNECTING;
				SendMuteMediaToParty(cmCapReceiveAndTransmit);// instead of MuteMediaIfNeeded(cmCapReceiveAndTransmit);
				SetDialState(kOkSent);
				m_pSipCntl->SendRemoteNumbering();
				m_pSipCntl->SipInviteResponseReq(OK_VAL);

				if(IsNeedToSendRtcpVideoPreference())
					SendStartVideoPreferenceToParty();
			}
			else
				PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpInd::HandleBridgeConnectedInd: Dial State is not kBeforeOkInConf, - ",GetDialState());
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

////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnPartyReceivedAckConnecting(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnPartyReceivedAckConnecting: Name ",m_pPartyConfName);
	DWORD status;
	*pParam >> status;

	// if we didn't get the checkcomplete by now, we can no longer wait for it, since we are
	// in dial-in and no action needed to be done. Re-invite should arrive and will trigger next steps
	if (IsValidTimer(ICECOMPLETETOUT))
		DeleteTimer(ICECOMPLETETOUT);

	if (status == STATUS_OK)
	{
		// After channels out were opened, start DTLS if needed.
		// If not, go to end transaction
		if (StartDtlsIfNeeded())
		{
			InformPartyRemoteConnect();
			return;
		}
		EndTransaction();
		//LyncCCS
		if(m_pSipCntl->GetIsCCSPlugin())
		{
			m_pSipCntl->PartyAuthAckReceiveFromCS();
		}
	}
	else
	{
		SetDialState(kBadStatusAckArrived);
		/* In case of timeout for 2xx CS will raise simulated ACK with status 408 - this is normal operation, call should disconnect */
		if (status != 408) {
			DBGPASSERT(status);
		}
		PTRACE2INT(eLevelError,"CSipTransInviteWithSdpInd::OnPartyReceivedAckConnecting: Ack with bad status - ",status);
		EndTransaction(status);
	}
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnConfBridgesConnectionTout(CSegment* pParam)
{
	PASSERTMSG((DWORD)statIllegal,"CSipTransInviteWithSdpInd::OnConfBridgesConnectionTout");
	EndTransaction(statIllegal);
}

/////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnIcePortsRetryTout(CSegment* pParam)
{
	m_pSipCntl->SetIsEnableICE(TRUE);
	if (SendIceMgsReqAccordingToTargetMode(ICE_MAKE_ANSWER_REQ) == STATUS_OK )
	{
		PTRACE2INT(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnIcePortsRetryTout - make answer seconds." ,
				MAKE_ICE_CANDIDATES_TIMER/SECOND);

		StartTimer(ICEGENERALTOUT, MAKE_ICE_CANDIDATES_TIMER);
	}
	else
	{
		PTRACE(eLevelInfoNormal,"CSipTransInviteWithSdpInd::OnIcePortsRetryTout - failed to make answer.");
		m_IceMakeOfferAnswerCounter = 0;

		m_pSipCntl->SetIsEnableICE(FALSE);
		m_pSipCntl->CloseIceSession();

		ContinueToOpenChannels();
	}
}

//////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::OnPartySlavesRecapIsFinished(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal, "CSipTransInviteWithSdpInd::OnPartySlavesRecapIsFinished");
}


//////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::PartyConnectCall()
{
	EndTransaction();
	//LyncCCS
	if(m_pSipCntl->GetIsCCSPlugin())
	{
		m_pSipCntl->PartyAuthAckReceiveFromCS();
	}
}

/////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWithSdpInd::RemoveBfcpForTIPDialInWithSDP()
{
		PTRACE(eLevelInfoNormal,"RemoveBfcpForTIPDialInWithSDP");

		m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
		m_pTargetMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);

		if (m_pTargetModeMaxAllocation)
		{
			m_pTargetMode->SetMediaOff(cmCapBfcp, cmCapReceiveAndTransmit);
			m_pTargetModeMaxAllocation->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePresentation);
		}
		//m_pSipCntl->RemoveBfcpCaps();
}
