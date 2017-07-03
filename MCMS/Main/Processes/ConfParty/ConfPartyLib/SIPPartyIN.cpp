////+========================================================================+
////                            SIPPartyIN.cpp                               |
////            Copyright 1995 POLYCOM Technologies Ltd.                     |
////                   All Rights Reserved.                                  |
////-------------------------------------------------------------------------|
//// NOTE: This software contains valuable trade secrets and proprietary     |
//// information of POLYCOM Technologies Ltd. and is protected by law.       |
//// It may not be copied or distributed in any form or medium, disclosed    |
//// to third parties, reverse engineered or used in any manner without      |
//// prior written authorization from POLYCOM Technologies Ltd.              |
////-------------------------------------------------------------------------|
//// FILE:       SIPPartyIN.cpp                                              |
//// SUBSYSTEM:  MCMS                                                        |
//// PROGRAMMER:															   |
////-------------------------------------------------------------------------|
//// Who | Date       | Description                                          |
////-------------------------------------------------------------------------|
////     | 15/11/05   | This file contains								   |
////     |            |                                                      |
////+========================================================================+
//
//
//
//#include "Segment.h"
//#include "StateMachine.h"
//#include "SysConfigKeys.h"
//#include "SysConfig.h"
//#include "Trace.h"
//#include "Macros.h"
//#include "NStream.h"
//#include "NetSetup.h"
//#include "StatusesGeneral.h"
//#include "DataTypes.h"
//#include "Conf.h"
//#include "ConfPartyOpcodes.h"
//#include "ConfPartyDefines.h"
////#include "IpCommonTypes.h"
//#include "IpAddressDefinitions.h"
//#include "IpCommonDefinitions.h"
//#include "IpCsOpcodes.h"
//#include "TaskApi.h"
//#include "Party.h"
//#include "PartyApi.h"
//#include "SipDefinitions.h"
//#include "SIPCommon.h"
//#include "SIPInternals.h"
//#include "SipUtils.h"
//#include "IpNetSetup.h"
//#include "SipNetSetup.h"
//#include "CsInterface.h"
//#include "SipScm.h"
//#include "SipCall.h"
//#include "ConfApi.h"
//#include "IPParty.h"
//#include "SIPControl.h"
//#include "Lobby.h"
//#include "SIPParty.h"
//#include "SIPPartyIN.h"
//#include "ConfPartyGlobals.h"
//#include "IpServiceListManager.h"
//#include "IPParty.h"
//
//extern CIpServiceListManager* GetIpServiceListMngr();
//
//////////////////////////////////////////////////////////////////////////////
//
////               Entry point
//
//////////////////////////////////////////////////////////////////////////////
//void SipPartyInEntryPoint(void* appParam)
//{
//  CSipPartyIn* pPartyTaskApp = new CSipPartyIn;
//  pPartyTaskApp->Create(*(CSegment*) appParam);
//}
//
//////////////////////////////////////////////////////////////////////////////
//// Sip Party In State machine comparing to MGC
////	MGC State									Carmel State
//// IP_DISCONNECTED as IDLE  state				PARTYIDLE, sPARTY_WAITFORCONFINITCALL
//// IP_CONNECTING as SETUP state  				sPARTY_OPENCHANNELS, sPARTY_OPENBRIDGES, sPARTY_CONNECTING
//// IP_CHANGEMODE as CHANGEMODE state			this is party of the SipParty State machine only!
//// IP_CONNECTED	as CONNECT state  				this is party of the SipParty State machine only!
//// IP_DISCONNECTING	as DISCONNECTING state		this is party of the SipParty State machine only!
//
//PBEGIN_MESSAGE_MAP(CSipPartyIn)
//// conf/lobby events
//ONEVENT(REJECTCALL,								PARTYIDLE,					CSipPartyIn::OnLobbyRejectIdle)
//ONEVENT(LOBBYNETIDENT,							PARTYIDLE,					CSipPartyIn::OnLobbyIdentIdle)
//ONEVENT(LOBBYTRANS,								PARTYIDLE,					CSipPartyIn::OnLobbyTransferIdle)
//ONEVENT(LOBBYDESTROY,							PARTYIDLE,					CSipPartyIn::OnConfCloseCall)
//ONEVENT(SET_SITE_AND_VISUAL_NAME,				sPARTY_WAITFORCONFINITCALL,	CSipParty::SendSiteAndVisualNamePlusProductIdToPartyControl)
//ONEVENT(ALLOCATE_PARTY_RSRC_IND,				sPARTY_WAITFORCONFINITCALL,	CSipPartyIn::OnConfAllocateResourcesIdle)
//ONEVENT(SIP_CONF_ESTABLISH_CALL,				sPARTY_WAITFORCONFINITCALL,	CSipPartyIn::OnConfEstablishCallIdle)
//ONEVENT(SIP_PARTY_BAD_STATUS,					sPARTY_WAITFORCONFINITCALL,	CSipPartyIn::OnPartyBadStatusConnecting)
//ONEVENT(SET_SITE_AND_VISUAL_NAME,				sPARTY_WAITFORCONFINITCALL,	CSipParty::SendSiteAndVisualNamePlusProductIdToPartyControl)
//
//ONEVENT(SIP_PARTY_CHANS_CONNECTED,				sPARTY_OPENCHANNELS,		CSipPartyIn::OnPartyChannelsConnectedOpenChannels)
//ONEVENT(SET_SITE_AND_VISUAL_NAME,				sPARTY_OPENCHANNELS,		CSipParty::SendSiteAndVisualNamePlusProductIdToPartyControl)
//ONEVENT(SIP_PARTY_BAD_STATUS,					sPARTY_OPENCHANNELS,		CSipPartyIn::OnPartyBadStatusConnecting)
//
//ONEVENT(AUDBRDGCONNECT,							sPARTY_OPENBRIDGES,			CSipParty::OnConfPartyReceiveAudBridgeConnected)
//ONEVENT(VIDBRDGCONNECT,							sPARTY_OPENBRIDGES,			CSipParty::OnConfPartyReceiveVidBridgeConnected)
//ONEVENT(FECCBRDGCONNECT,						sPARTY_OPENBRIDGES,			CSipParty::OnConfPartyReceiveFeccBridgeConnected)
//ONEVENT(SIP_PARTY_BAD_STATUS,					sPARTY_OPENBRIDGES,			CSipPartyIn::OnPartyBadStatusConnecting)
//
//ONEVENT(SIP_PARTY_RECEIVED_ACK,					sPARTY_CONNECTING,			CSipPartyIn::OnPartyReceivedAckConnecting)
//ONEVENT(SIP_PARTY_BAD_STATUS,					sPARTY_CONNECTING,			CSipPartyIn::OnPartyBadStatusConnecting)
//
//// party events
//ONEVENT(PARTYCONNECTTOUT,						sPARTY_CONNECTING,			CSipPartyIn::OnPartyConnectToutConnecting)
//
//ONEVENT(CONFDISCONNECT,							PARTYDISCONNECTING,			CSipParty::OnConfReadyToCloseCall)
//
//ONEVENT(SET_SITE_AND_VISUAL_NAME,				ANYCASE,					CSipPartyIn::NullActionFunction)
//ONEVENT(CONFDISCONNECT,							ANYCASE,					CSipPartyIn::OnConfCloseCall)
//
//  // Self timers
//ONEVENT(PARTYCONTOUT  	 						,PARTYIDLE   				,CSipPartyIn::OnSipDisconnectSetupIdle)
//ONEVENT(PARTYCONTOUT  	 						,sPARTY_WAITFORCONFINITCALL	,CSipPartyIn::OnSipDisconnectSetupIdle)
//ONEVENT(PARTYCONTOUT  	 						,ANYCASE 					,CSipPartyIn::NullActionFunction)
//
//ONEVENT(OPENBRIDGESTOUT  						,sPARTY_OPENBRIDGES   		,CSipParty::OnConfBridgesConnectionTout)
//ONEVENT(OPENBRIDGESTOUT   						,ANYCASE 					,CSipPartyIn::NullActionFunction)
//
//
//PEND_MESSAGE_MAP(CSipPartyIn,CSipParty);
//
//
//////////////////////////////////////////////////////////////////////////////
//CSipPartyIn::CSipPartyIn()
//{
//	m_pLobbyApi = new CLobbyApi;
//	m_pConfApi = new CConfApi;
//	m_eResponsibility	= kNoResponsibility;
//	m_eLobbyRejectReason = SipCodesSipUnknownStatus;
//	m_DialInRejectConnectionId = 0xFFFFFFFF;
//	m_bSetRsrcParam = FALSE;
//	VALIDATEMESSAGEMAP
//}
//
//
//////////////////////////////////////////////////////////////////////////////
//CSipPartyIn::~CSipPartyIn()
//{
//	m_pLobbyApi->DestroyOnlyApi();
//	POBJDELETE(m_pLobbyApi);
//	m_pConfApi->DestroyOnlyApi();
//	POBJDELETE(m_pConfApi);
//}
//
//
//////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::Create(CSegment& appParam)
//{
//	CIpParty::Create(appParam);
//	m_pLobbyApi->CreateOnlyApi(*m_pCreatorRcvMbx);
//	m_eResponsibility = kLobby;
//}
//
//
//////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::CleanUp()
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::CleanUp: Name ", m_partyConfName);
//
//	switch(m_eDialState)
//	{
//	case kRejectedByLobby:
//		if (m_alternativeAddrStr)
//		{
//			m_pSipCntl->RejectCallAndProvideAlternativeAddrToCall(SipCodesMovedTemp, m_alternativeAddrStr);
//		}
//		else
//		{
//			m_pSipCntl->RejectCall(m_eLobbyRejectReason);
//		}
//		break;
//
//	case kFailedInLobby:
//	case kFailedInConf:
//		m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
//		break;
//
//	case kTerminateByConf:
//		m_pSipCntl->RejectCall(SipCodesDecline);
//		break;
//
//	case kChannelsFailedToOpen:
//	case kTransferFailed:
//	case kBeforeOkInConf:
//		m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
//		break;
//
//	case kCapsDontMatch:
//		m_pSipCntl->RejectCall(SipCodesNotAcceptedInHere,SipWarningMediaTypeNotAvail); //section 14.2 in standard
//		break;
//
//	case kCancelArrived:
//		m_pSipCntl->CloseCall(NO,SipCodesRequestTerminated);
//		break;
//
//	case kOkSent: //wait for an ack to arrive (or timer to popped)
//	case kReInviteAccepted:
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
//		break;
//
//	case kReInviteRejected://wait for an ack to arrive (or timer to popped)
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
//		m_eDialState = kTerminateByConf;
//		break;
//
//	case kTerminateByRemote:
//		m_pSipCntl->CloseCall(NO);
//		break;
//
//	case kBadStatusAckArrived:
//	case kConnectTimer:
//		m_pSipCntl->CloseCall(YES);
//		break;
//
//	case kBadStatusArrived:
//	case kTransportErrorArrived:
//	case kDisconnectTimer:
//		m_pSipCntl->ViolentCloseCall();
//		break;
//
//	case kNoRecovery: // after re-invite
//		m_pSipCntl->SipInviteAckReq();	// for the 200 ok
//		m_pSipCntl->CloseCall(YES);		// send bye
//		break;
//
//	case kBadStatusArrivedAfterOk: // wait for connect timer.
//	case kReInviteSent:
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::CleanUp: Wait for answer or timer in dial state %d ",m_eDialState);
//		break;
//
//	case kReInviteArrived:
//		m_pSipCntl->SipInviteResponseReq(SipCodesBadRequest); // wait for ack
//		break;
//
//	case kNotInDialState:
//		m_pSipCntl->CloseCall(YES);
//		break;
//
//	case kByeArrived:
//		m_pSipCntl->CloseCall(NO);
//		break;
//	case kInternalProblem:
//		PTRACE(eLevelInfoNormal,"CSipPartyIn::CleanUp: kInternalProblem ");
//		m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
//		break;
//
//	default:
//		DBGPASSERT(m_eDialState);
//	}
//}
//
//
////////////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnLobbyIdentIdle(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnLobbyIdent: Name ",m_partyConfName);
//	CSipNetSetup* pNetSetup		= new CSipNetSetup;
//
//	DWORD				  len			= 0;
//	sipSdpAndHeaders* pSdpAndHeaders    = NULL;
//	CSipCaps* pRemoteCaps	= new CSipCaps;
//
//	pNetSetup->DeSerialize(NATIVE,*pParam);
////	m_pTargetMode->DeSerialize(NATIVE,*pParam);
//	*pParam >> len;
//	pSdpAndHeaders = (sipSdpAndHeaders *) new BYTE[len];
//	memset(pSdpAndHeaders, 0, len);
//	pParam->Get((BYTE*)pSdpAndHeaders,len);
//	pRemoteCaps->Create(*pSdpAndHeaders);
//
//	// code for testing different EP caps
////	CCapSetInfo capInfo = eH264CapCode;
////	pRemoteCaps->RemoveCapSet(capInfo);
//
//	m_pSipCntl->Create(this,pNetSetup,/*pRsrcDesc,*/NO, m_serviceId);
//	m_pSipCntl->SetRemoteSdp(*pSdpAndHeaders);
//	m_pSipCntl->SetRemoteHeaders(pSdpAndHeaders);
//	m_pSipCntl->SetCallLegAndCdrHeaders(*pSdpAndHeaders);
////	m_pSipCntl->SetRemoteCaps(*pRemoteCaps);
//	m_pSipCntl->SetLastRemoteCaps(*pRemoteCaps);
//	m_pSipCntl->StoreRemoteVendorInfo();
//
//	if(m_voice == NO)// if its a video call, then check if the remote has video, if not change it to audio only call.
//		m_voice = (pRemoteCaps->IsMedia(cmCapVideo) == NO);
//
//	m_pLobbyApi->PartyIdent(this, INITIAL, statOK);  // always initial channel
//
//	m_eResponsibility = kLobby;
//	m_eDialState = kBeforeOk;
//	POBJDELETE(pNetSetup);
//	PDELETEA(pSdpAndHeaders);
//	POBJDELETE(pRemoteCaps);
//}
//
//
//////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnLobbyRejectIdle(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnLobbyRejectIdle: Name ",m_partyConfName);
//	CSipNetSetup *		pNetSetup		= new CSipNetSetup;
//	DWORD	reason = 0xFFFFFFFF;
//	WORD	sdpLen=0, addressLen=0;
//	sipSdpAndHeaders*	pSdpAndHeaders	= NULL;
//
//	if (IsValidTimer(PARTYCONTOUT))
//		DeleteTimer(PARTYCONTOUT);
//
//	pNetSetup->DeSerialize(NATIVE,*pParam);
//	*pParam >> reason;
//	*pParam >> sdpLen;
//	pSdpAndHeaders = (sipSdpAndHeaders *) new BYTE[sdpLen];
//	pParam->Get((BYTE*)pSdpAndHeaders,sdpLen);
//	*pParam >> addressLen;
//	if(addressLen)
//	{
//		if(m_alternativeAddrStr)
//			PDELETEA(m_alternativeAddrStr);
//		m_alternativeAddrStr = new char[addressLen+1];
//		*pParam >> m_alternativeAddrStr;
//	}
//	PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::OnLobbyRejectIdle: Reject reason ",reason);
//	m_pSipCntl->Create(this, pNetSetup, NO, m_serviceId);
//	m_pSipCntl->SetCallLegAndCdrHeaders(*pSdpAndHeaders);
////	::AllocateRejectID(m_DialInRejectConnectionId);
////	m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
//	m_pSipCntl->SetRemoteSdp(*pSdpAndHeaders);
//
//	m_eDialState = kRejectedByLobby;
//	switch(reason)
//	{
//	case cmReasonTypeCallForwarded:
//		m_eLobbyRejectReason = SipCodesMovedTemp;
//		break;
//
//	case cmReasonTypeNoPermision:
//		m_eLobbyRejectReason = SipCodesForbidden;
//		break;
//
//	case cmReasonTypeNoBandwidth:
//		m_eLobbyRejectReason = SipCodesBusyHere;
//		break;
//
//	default:
//		DBGPASSERT(reason);
//		m_eLobbyRejectReason = SipCodesInternalSrvErr;
//		break;
//	}
//
//	POBJDELETE(pNetSetup);
//	PDELETEA(pSdpAndHeaders);
//}
//
//
/////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::UpdateRemoteSupportHighCapabilities(CSipNetSetup* pNetSetup)
//{
//  	const CSipCaps* pRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
//
//	eVideoPartyType RemoteVideoPartyType = eVideo_party_type_none;
//
//	if (pRemoteCaps->IsMedia(cmCapVideo) == FALSE)
//	{
//		RemoteVideoPartyType = eVideo_party_type_none;
//	}
//	else
//	{
//		RemoteVideoPartyType = eCP_H261_H263_H264_upto_CIF_video_party_type;
//
//		BYTE IsCIFForce = IsSetCIFRsrcForUser(m_pSipCntl->GetUserAgent());
//
//		if(IsCIFForce)
//		{
//			PTRACE(eLevelInfoNormal,"CSipPartyIn::UpdateRemoteSupportHighCapabilities FORCE CIF Rsrc");
//		}
//		else
//		{
//			CBaseCap* pCap = NULL;
//
//		int numOfVideoCapSets = pRemoteCaps->GetNumOfMediaCapSets(cmCapVideo);
//
//		for (int index=0; index < numOfVideoCapSets; index++)
//		{
//			pCap = pRemoteCaps->GetCapSet(eH264CapCode,index);
//			if (pCap)
//			{
//				if (((CH264VideoCap*)pCap)->GetProfile() == H264_Profile_BaseLine)
//				{
//						// the static MB should be calculated here according to the rules of the feature, but since for V4.1 it was made mainly for H323, for fast development I put here zero
//						DWORD staticMB = 0;
//						eVideoPartyType tempVideoPartyType = ((CH264VideoCap*)pCap)->GetVideoPartyTypeMBPSandFS(staticMB);
//					RemoteVideoPartyType = max(RemoteVideoPartyType, tempVideoPartyType);
//				}
//				POBJDELETE (pCap);
//			}
//			}
//		}
//		}
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::UpdateRemoteSupportHighCapabilities Video party type is: ",eVideoPartyTypeNames[RemoteVideoPartyType]);
//
//	pNetSetup->SetRemoteVideoPartyType(RemoteVideoPartyType);
//	pNetSetup->SetRemoteUserAgent(m_pSipCntl->GetUserAgent());
//}
//
//////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnLobbyTransferIdle(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal, "CSipPartyIn::OnLobbyTransferIdle: Name ", m_partyConfName);
//	CSegment rspMsg;
//	WORD mode = 0xFFFF;
//	WORD  returnSyncStatus = 0;
//	*pParam >> mode;
//	m_pConfApi->CreateOnlyApi(*m_pConfRcvMbx,NULL,NULL,1);
//
//	if (mode == PARTYTRANSFER)
//	{
//		CSipNetSetup* pNetSetup = new CSipNetSetup;
//		*pNetSetup = *m_pSipCntl->GetNetSetup();
//
//		UpdateRemoteSupportHighCapabilities(pNetSetup);
//
//		 CSipCaps* pRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
//		// sync call to conf
//		WORD res = m_pConfApi->AddInSipParty(pNetSetup, pRemoteCaps, this, *m_pRcvMbx, m_name,
//												PARTY_TRANSFER_TOUT,rspMsg);
//		POBJDELETE(pNetSetup);
//
//		if (res == 0)
//		{
//            rspMsg >> returnSyncStatus;
//		}
//
//		if (res)
//		{
//			PTRACE2(eLevelError, "CSipPartyIn::OnLobbyTransferIdle: Timer expired on transfer party to conf", m_partyConfName);
//			m_pLobbyApi->PartyTransfer(this, statTout);
//			m_eDialState = kTransferFailed;
//			m_state = PARTYDISCONNECTING;
//			if (IsValidTimer(PARTYCONTOUT))
//				DeleteTimer(PARTYCONTOUT);
//		}
//		else if (returnSyncStatus)
//		{
//			PTRACE(eLevelError,"CSipPartyIn::OnLobbyTransferIdle : \'EXPORT PARTY FAILED (Conf Reject) !!!\'");
//			m_pLobbyApi->PartyTransfer(this, returnSyncStatus);
//			m_eDialState = kTransferFailed;
//			m_state = PARTYDISCONNECTING;
//			if (IsValidTimer(PARTYCONTOUT))
//				DeleteTimer(PARTYCONTOUT);
//		}
//		else
//		{
//			m_state = sPARTY_WAITFORCONFINITCALL;
//			PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnLobbyTransferIdle: Transfer party to conf ok - ", m_partyConfName);
//			m_pLobbyApi->PartyTransfer(this, statOK);
//			m_eDialState = kBeforeOkInConf;
//			m_eResponsibility = kConf;
//			// site names current only for dial in
//			m_pSipCntl->FindSiteAndVisualNamePlusProductIdAndSendToConfLevel();
//		}
//	}
//	else
//	{
//		DBGPASSERT(mode ? mode : YES);
//	}
//}
//
///////////////////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnConfAllocateResourcesIdle(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnConfAllocateResourcesIdle: Name ",m_partyConfName);
//	CRsrcParams* pCsRsrcParams = new CRsrcParams;
//	pCsRsrcParams->DeSerialize(NATIVE,*pParam);
//	m_pSipCntl->SetRsrcParams(pCsRsrcParams);
//	m_bSetRsrcParam = TRUE;
//
//	// send Ringing
//	m_pSipCntl->SipRingingReq();
//	POBJDELETE(pCsRsrcParams);
//}
//
//
///////////////////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnConfEstablishCallIdle(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnConfEstablishCall: Name ",m_partyConfName);
//	BYTE bCapsDontMatch	= NO;
//
//	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
//	CRsrcParams* pCsRsrcParams = new CRsrcParams;
//	UdpAddresses sUdpAddressesParams;
//	CSipNetSetup*	pNetSetup = new CSipNetSetup;;
//	CSipCaps* pLocalCaps = new CSipCaps;
//	CQoS* pQos = new CQoS;
//	DWORD confParamLen = 0;
//	char* strConfInfo	= NULL;
//	BYTE eTransportType = 0;
//
//	pNetSetup->DeSerialize(NATIVE,*pParam);
//	pLocalCaps->DeSerialize(NATIVE,*pParam);
//
//	m_pTargetMode->DeSerialize(NATIVE,*pParam);
//
//	if (m_pSipCntl->GetRemoteIdent() == PolycomRMX)
//	{
//		PTRACE(eLevelInfoNormal,"CSipPartyIn::OnConfEstablishCallIdle: Remote ident is Polycom RMX. Remove Fecc cap");
//		pLocalCaps->CleanMedia(cmCapData);
//		m_pTargetMode->SetMediaOff(cmCapData,cmCapReceiveAndTransmit);
//	}
//
//	PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::OnConfEstablishCall: type - ",m_pTargetMode->GetConfType());
//	if ( kCp == m_pTargetMode->GetConfType() || kCpQuad == m_pTargetMode->GetConfType() )
//	{// for dial in we change the remote caps according to system setting
//		CSipCaps*	pLastRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());
//		BYTE isRmxInitiateTransaction = FALSE;
//		BYTE bFixVideoRate = m_pSipCntl->CheckIsMobilePhoneByHeader(isRmxInitiateTransaction);
//		pLastRemoteCaps->FixRemoteCapsBySystemSettings(m_pSipCntl->GetRmtHeaders(), bFixVideoRate);
//		m_pSipCntl->SetLastRemoteCaps(*pLastRemoteCaps);
//	}
//
//	pQos->DeSerialize(NATIVE,*pParam);
//	*pParam >> m_bIsAdvancedVideoFeatures;
//
//	*pParam >> confParamLen;
//	if ( confParamLen )
//	{
//		strConfInfo = new char[confParamLen];
//		pParam->Get((BYTE*)strConfInfo, confParamLen);
//		strConfInfo[confParamLen-1] = 0;
//	}
//
//	*pParam >> eTransportType; // not used in dial in
//	m_pSipCntl->SetLocalCaps(*pLocalCaps);
//	m_pSipCntl->SetQos(*pQos);
//	m_pSipCntl->SetConfParamInfo(strConfInfo);
//
//	pRtpRsrcParams->DeSerialize(NATIVE,*pParam);
//	pCsRsrcParams->DeSerialize(NATIVE,*pParam);
//	pParam->Get((BYTE *)&sUdpAddressesParams,sizeof(UdpAddresses));
//
//	*pParam >> m_mcuNum;
//	*pParam >> m_termNum;
//
//	m_PartyRsrcID = pCsRsrcParams->GetPartyRsrcId();
//	m_ConfRsrcId = pCsRsrcParams->GetConfRsrcId();
//    m_pSipCntl->SetControllerResource(pRtpRsrcParams, pCsRsrcParams, sUdpAddressesParams);
//    m_pSipCntl->AddToRoutingTable();
//
//    POBJDELETE(pRtpRsrcParams);
//	POBJDELETE(pCsRsrcParams);
//
//
//	if (IsValidTimer(PARTYCONTOUT))
//	{
//		PTRACE(eLevelInfoNormal,"CSipPartyIn::OnConfEstablishCallIdle - Delete PARTYCONTOUT");
//		DeleteTimer(PARTYCONTOUT);
//	}
//
//	const CSipCaps*	pRemoteCaps = m_pSipCntl->GetLastRemoteCaps();
//	if (pRemoteCaps && pRemoteCaps->GetNumOfCapSets())
//	{
//		//update SDP in operator (connection info window)
//		PTRACE(eLevelInfoNormal,"CSipPartyIn::OnConfEstablishCallIdle: Get best mode");
//		PartyOriginalRemoteCaps(const_cast<CSipCaps*>(pRemoteCaps));
//		m_pSipCntl->CompleteRemoteCapDataFromLocalCap();
//
//		CSipComMode * pBestMode = m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode);
//		if (pBestMode && pBestMode->IsMediaOn(cmCapAudio,cmCapReceive) && pBestMode->IsMediaOn(cmCapAudio,cmCapTransmit))
//		{
//			PTRACE(eLevelInfoNormal,"CSipPartyIn::OnConfEstablishCallIdle: mode found");
//			// check if one video direction is muted
//			EIpChannelType eMuteChannel	 = (EIpChannelType)0;
//
//			// if both direction are off there is no mute but audio only!
//			if (pBestMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit) == NO)
//			{
//				cmCapDirection directionArr[] = {cmCapReceive,cmCapTransmit};
//				for(int i=0 ; i<2 && !eMuteChannel/*only one direction could be muted*/; i++)
//				{
//					cmCapDirection eOpposite = (directionArr[i] == cmCapReceive)? cmCapTransmit: cmCapReceive;
//					if (pBestMode->IsMediaOff(cmCapVideo,directionArr[i]))
//					{
//						if (pRemoteCaps->IsMediaMuted(cmCapVideo,eOpposite))
//						{
//							pBestMode->CopyMediaModeToOppositeDirection(cmCapVideo,eOpposite);
//							eMuteChannel = ::CalcChannelType(cmCapVideo,directionArr[i]==cmCapTransmit,kRolePeople);
//						}
//						else // the mode is not muted. we just didn't find best mode to open. we turn to secondary
//							pBestMode->SetMediaOff(cmCapVideo,eOpposite);
//					}
//				}
//			}
//
//			GetProductIdAndSendToConfLevel();
//			m_state = sPARTY_OPENCHANNELS;
//			pBestMode->SetConfType(m_pTargetMode->GetConfType());
//			*m_pTargetMode = *pBestMode;
//			m_pSipCntl->MakeANewCall(pBestMode);
//			if (eMuteChannel)
//				m_pSipCntl->MuteChannels(YES,1,&eMuteChannel);
//		}
//		else
//		{
//			PTRACE(eLevelError,"CSipPartyIn::OnConfEstablishCallIdle: No best mode found. must reject call");
//			bCapsDontMatch = YES;
//		}
//		POBJDELETE(pBestMode);
//	}
//	else
//	{
//		PTRACE2(eLevelError,"CSipPartyIn::OnConfEstablishCallIdle: No Remote capabilities!!! Name ",m_partyConfName);
//		bCapsDontMatch = YES;
//	}
//
//	if (bCapsDontMatch)
//	{
//		DBGPASSERT(YES);
//		m_eDialState = kCapsDontMatch;
//		TellConfOnDisconnecting(SIP_CAPS_DONT_MATCH);
//	}
//	else
//	{
//		m_pConfApi->SipPartyRemoteCapsRecieved(this, const_cast<CSipCaps*>(pRemoteCaps), m_pCurrentMode);
//
//		mcTransportAddress localIp;
//		memset(&localIp,0,sizeof(mcTransportAddress));
//		mcTransportAddress remoteIp;
//		memset(&remoteIp,0,sizeof(mcTransportAddress));
//		m_pSipCntl->GetLocalMediaIpAsTrAddr(localIp);
//		m_pSipCntl->GetRemoteMediaIpAsTrAddr(cmCapAudio,remoteIp); // take the audio ip as default
//		localIp.port = 5060;
//		remoteIp.port = 5060;
//		DWORD actualRate=0xFFFFFFFF;
//		SetPartyMonitorBaseParamsAndConnectChannel(SDP,actualRate,&remoteIp,&localIp);
//	}
//
//	POBJDELETE(pNetSetup);
//	POBJDELETE(pLocalCaps);
//	POBJDELETE(pQos);
//	PDELETEA(strConfInfo);
//}
//
//////////////////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnPartyChannelsConnectedOpenChannels(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipParty::OnPartyChannelsConnectedOpenChannels: Name ",m_partyConfName);
//	m_state = sPARTY_OPENBRIDGES;
//	StartTimer(OPENBRIDGESTOUT, 10*SECOND);
//	ChannelsConnected(pParam);
//}
//
//
///////////////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::HandleBridgeConnectedInd(DWORD status)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::HandleBridgeConnectedInd: Name ",m_partyConfName);
//
//	if (status == STATUS_OK)
//	{
//		// its OK if both the bridges connected or its audio only call or there is no much in the video capability
//		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_isFeccBridgeConnected) ||
//			(m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_pTargetMode->IsMediaOff(cmCapData,cmCapReceiveAndTransmit)) ||
//			(m_isAudioBridgeConnected && GetIsVoice()) ||
//			(m_isAudioBridgeConnected && m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit)))
//		{
//			if (m_eDialState == kBeforeOkInConf)
//			{
//				if (IsValidTimer(OPENBRIDGESTOUT))
//				{
//					DeleteTimer(OPENBRIDGESTOUT);
//					PTRACE(eLevelInfoNormal,"CSipPartyIn::HandleBridgeConnectedInd: DeleteTimer(OPENBRIDGESTOUT) ");
//				}
//				m_state		 = sPARTY_CONNECTING;
//				m_eDialState = kOkSent;
//				m_pSipCntl->SendRemoteNumbering();
//				m_pSipCntl->SipInviteResponseReq(OK_VAL);
//			}
//			else
//				PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::HandleBridgeConnectedInd: Dial State is not kBeforeOkInConf, - ",m_eDialState);
//		}
//		else if(m_isAudioBridgeConnected)
//			PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::HandleBridgeConnectedInd: Is Voice Call? - ",GetIsVoice());
//	}
//	else
//	{
////		m_eDialState = kBadStatusAckArrived;
//		DBGPASSERT(status);
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::HandleBridgeConnectedInd: Ack with bad status - ",status);
//		TellConfOnDisconnecting(status);
//	}
//}
//
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnPartyReceivedAckConnecting(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnPartyReceivedAckConnecting: Name ",m_partyConfName);
//	DWORD status;
//	*pParam >> status;
//
//	if (status == STATUS_OK)
//	{
//		m_eDialState = kNotInDialState;
//		m_state		 = PARTYCONNECTED;
//		m_pConfApi->SipPartyRemoteConnected(this, m_pCurrentMode);
//		//DWORD localIp  = m_pSipCntl->GetLocalMediaIp();
//		//DWORD remoteIp = m_pSipCntl->GetRemoteMediaIp(cmCapAudio); // take the audio ip as default
//
//		CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
//		CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
//		ipAddressIf localAddress;
//		mcTransportAddress localIp;
//		memset(&localIp,0,sizeof(mcTransportAddress));
//		mcTransportAddress remoteIp;
//		memset(&remoteIp,0,sizeof(mcTransportAddress));
//
//		if (pServiceParams == NULL)
//		{
//			PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::OnPartyReceivedAckConnecting: IP Service does not exist!!! ServiceID = ",m_serviceId);//m_pSipCntl->GetServiceId());
//			return;
//		}
//		CSipNetSetup* pNetSetup = const_cast<CSipNetSetup*>(m_pSipCntl->GetNetSetup());
//		mcTransportAddress* pDestTaAddr = const_cast<mcTransportAddress*>(pNetSetup->GetTaDestPartyAddr());
//		if (pNetSetup->GetIpVersion() == eIpVersion4)
//		{
//			localAddress = pServiceParams->GetIpV4Address();
//			localIp.addr.v4.ip = localAddress.v4.ip;
//		}
//		else
//		{
//			BYTE place = ::FindIpVersionScopeIdMatchBetweenPartyAndService(pDestTaAddr, pServiceParams);
//			if (place == 0xFF)
//			{
//				PASSERTMSG(4,"CSipPartyIn::OnPartyReceivedAckConnecting - No IpV6 in Service");
//				return;
//			}
//			localAddress = pServiceParams->GetIpV6Address((int)place);
//			memcpy(&localIp.addr,&localAddress,sizeof(ipAddressIf));
//		}
//
//
//		m_pSipCntl->GetRemoteMediaIpAsTrAddr(cmCapAudio,remoteIp);
//		localIp.port = 5060;
//		remoteIp.port = 5060;
//		DWORD actualRate = 0xFFFFFFFF;
//		SetPartyMonitorBaseParamsAndConnectChannel(SIGNALING,actualRate,&remoteIp,&localIp);
//
//		UpdateDbOnSipPrivateExtension();
//		StartIvr();
//	}
//	else
//	{
//		m_eDialState = kBadStatusAckArrived;
//		DBGPASSERT(status);
//		PTRACE2INT(eLevelError,"CSipPartyIn::OnPartyReceivedAckConnecting: Ack with bad status - ",status);
//		TellConfOnDisconnecting(status);
//	}
//}
//
//
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnPartyCallFailed(CSegment * pParam)
//{
//	DWORD reason = 0xFFFFFFFF;
//	DWORD MipErrorNumber = 0;
//	*pParam >> reason;
//	PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::OnPartyCallFailed: Reason - ",reason);
//	if (reason == SIP_CARD_REJECTED_CHANNELS)
//	{
//		m_eDialState = kChannelsFailedToOpen;
//		reason = MCU_INTERNAL_PROBLEM; //change for disconnect cause in GUI
//		*pParam >> MipErrorNumber;
//	}
//	TellConfOnDisconnecting(reason,NULL,MipErrorNumber);
//}
//
//
//
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnPartyBadStatusConnecting(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnPartyBadStatus: Name ",m_partyConfName);
//	DWORD opcode;
//	DWORD len;
//	char* strDescription;
//	*pParam >> opcode;
//	*pParam >> len;
//	strDescription = new char[len+1];
//	pParam->Get((BYTE*)strDescription,len);
//	strDescription[len] = 0;
//	CSmallString str;
//	str << "Bad status opcode " << opcode << ", description: " <<strDescription;
//	PTRACE2(eLevelError,"CSipPartyIn::OnPartyBadStatus ",str.GetString());
//
//	if (opcode == SIP_CS_SIG_INVITE_RESPONSE_REQ)
//	{
//		if (m_eDialState != kOkSent)
//			DBGPASSERT(m_eDialState);
//		m_eDialState = kBadStatusArrivedAfterOk;
//	}
//
//	if (m_eDialState != kBeforeOkInConf)
//		m_eDialState = kBadStatusArrived;
//
//	TellConfOnDisconnecting(SIP_BAD_STATUS);
//}
//
//
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::DestroyPartyTask()
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::DestroyPartyTask: Name ",m_partyConfName);
//	//FilterMessage(m_pSipCntl);
//	m_pSipCntl->Suspend(); //ingore all events
//	if (m_eResponsibility == kConf)
//		m_pConfApi->PartyEndDisConnect(this,statOK);
//	else if (m_eResponsibility == kLobby)
//	{// in the new flow any reason that makes the dial in party to response with reject initiate the reject
//		// process from the lobby. And the party is not send End disconnect to the Lobby.
//		int i = 0; // do nothing!
////		m_pLobbyApi->PartyEndDisConnect(this,statOK);
//	}
//	else
//		DBGPASSERT(YES);
//	PartySelfKill();
//}
//
//
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnConfCloseCall(CSegment * pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnConfCloseCall: Name ",m_partyConfName);
//	m_state = PARTYDISCONNECTING;
//	LogicalChannelDisconnect(SDP);
//
//	if (IsValidTimer(OPENBRIDGESTOUT))
//		DeleteTimer(OPENBRIDGESTOUT);
//
//	if (IsValidTimer(PARTYCONTOUT))
//		DeleteTimer(PARTYCONTOUT);
///*
//	if ((m_eResponsibility == kLobby) && (m_DialInRejectConnectionId == 0xFFFFFFFF))
//	{// for reject dial in calls from the lobby due to lobby internal process error.
//		::AllocateRejectID(m_DialInRejectConnectionId);
//		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
//	}
//*/
//	if ((m_eDialState == kBeforeOk) && (m_eResponsibility == kLobby))
//		m_eDialState = kFailedInLobby;
//	else if (m_eDialState == kBeforeOk)
//	{
//		// for reject dial in calls from the conf due to conf internal problem with creating the party.
//		m_eDialState = kFailedInConf;
//	}
//	else if (m_eDialState == kBeforeOkInConf)
//		m_eDialState = kTerminateByConf;
//
//	// before cleanup connection id must be set
//	if (m_DialInRejectConnectionId == 0xFFFFFFFF && m_bSetRsrcParam == FALSE)
//	{
//		::AllocateRejectID(m_DialInRejectConnectionId);
//		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
//	}
//
//	CleanUp();
//}
//
//
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::LogicalChannelDisconnect(DWORD eChannelType)
//{
//	if(m_eResponsibility == kConf)
//		CIpParty::LogicalChannelDisconnect(eChannelType);
//}
//
//
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnConfDisconnectChannelsConnecting(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnConfDisconnectChannelsConnecting: To Do !!! Name ",m_partyConfName);
//}
//
//
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnPartyConnectToutConnecting(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyIn::OnPartyConnectToutConnecting: Name ",m_partyConfName);
//	m_eDialState = kConnectTimer;
//	TellConfOnDisconnecting(SIP_TIMER_POPPED_OUT);
//}
//////////////////////////////////////////////////////////////////////////
//void CSipPartyIn::OnSipDisconnectSetupIdle(CSegment* pParam)
//{
//	PASSERTMSG(m_PartyRsrcID,"CSipPartyIn::OnSipDisconnectSetupIdle: Disconnect call");
//	m_isPreSignalingFlowProb = 1;
//
//	if(m_eDialState == kNotInDialState)// under lobby responsibility
//	{
//		PTRACE(eLevelInfoNormal,"CSipPartyIn::OnSipDisconnectSetupIdle: Not in Dial stateName ");
//		DeleteAllTimers();
//		m_pSipCntl->Suspend(); //ingore all events
//		PartySelfKill();
//	}
//	else if(m_eDialState == kBeforeOk && m_eResponsibility == kLobby)// under lobby responsibility
//	{// under Lobby responsibility, reject the call.
//		PTRACE(eLevelInfoNormal,"CSipPartyIn::OnSipDisconnectSetupIdle: Before OK ");
//		m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
//		::AllocateRejectID(m_DialInRejectConnectionId);
//		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
//		m_eDialState = kFailedInLobby;
//		m_state = PARTYDISCONNECTING;
//		CleanUp();
//	}
//	else if(m_eDialState == kBeforeOkInConf && m_eResponsibility == kConf)
//	{//failed to response with establish call before timeout
//		// Activate "Kill Port"
//		PTRACE(eLevelInfoNormal,"CSipPartyIn::OnSipDisconnectSetupIdle: conf responsibility ");
//		OnPartySendFaultyMfaToPartyCntlAnycase(pParam);
//		// In case we already allocated and registered the CS and MFA we need to remove them from the
//		// routing table to avoid duplicate party entrence in the routing table
//		m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
//		::AllocateRejectID(m_DialInRejectConnectionId);
//		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
//		m_eDialState = kInternalProblem;
//
//		BYTE 	mipHwConn = (BYTE)eMipConnectionProcess;
//		BYTE	mipMedia = (BYTE)eMipNoneMedia;
//		BYTE	mipDirect = (BYTE)eMipIn;
//		BYTE	mipTimerStat = (BYTE)eMipTimer;
//		BYTE	mipAction = (BYTE)eMipConnect;
//		DWORD MpiErrorNumber = ::CalculateMcuInternalProblemErrorNumber(mipHwConn,mipMedia,mipDirect,mipTimerStat,mipAction);
//		TellConfOnDisconnecting(MCU_INTERNAL_PROBLEM,NULL,MpiErrorNumber);
//	}
//	else
//	{//unknown scenario
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyIn::OnSipDisconnectSetupIdle: Unknown state - ", m_eDialState);
//		DeleteAllTimers();
//		m_pSipCntl->Suspend(); //ingore all events
//		PartySelfKill();
//	}
//}
