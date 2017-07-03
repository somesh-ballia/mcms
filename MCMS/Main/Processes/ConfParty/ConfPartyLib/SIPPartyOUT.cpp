////+========================================================================+
////                            SIPPartyOut.cpp                              |
////            Copyright 1995 POLYCOM Technologies Ltd.                     |
////                   All Rights Reserved.                                  |
////-------------------------------------------------------------------------|
//// NOTE: This software contains valuable trade secrets and proprietary     |
//// information of POLYCOM Technologies Ltd. and is protected by law.       |
//// It may not be copied or distributed in any form or medium, disclosed    |
//// to third parties, reverse engineered or used in any manner without      |
//// prior written authorization from POLYCOM Technologies Ltd.              |
////-------------------------------------------------------------------------|
//// FILE:       SIPPartyOut.cpp                                             |
//// SUBSYSTEM:  MCMS                                                        |
//// PROGRAMMER:															   |
////-------------------------------------------------------------------------|
//// Who | Date       | Description                                          |
////-------------------------------------------------------------------------|
////     | 15/11/05   | This file contains								   |
////     |            |                                                      |
////+========================================================================+
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
//#include "IpNetSetup.h"
//#include "SipNetSetup.h"
//#include "CsInterface.h"
//#include "SipScm.h"
//#include "SipCall.h"
//#include "ConfApi.h"
//#include "IPParty.h"
//#include "SIPControl.h"
//#include "SIPParty.h"
//#include "SIPPartyOUT.h"
//#include "IpServiceListManager.h"
//#include "IPParty.h"
//#include "H264Util.h"
//
//extern CIpServiceListManager* GetIpServiceListMngr();
//
//void SipPartyOutEntryPoint(void* appParam)
//{
//  CSipPartyOut* pPartyTaskApp = new CSipPartyOut;
//  pPartyTaskApp->Create(*(CSegment*)appParam);
//}
//
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//// Sip Party Out State machine comparing to MGC
////	MGC State									Carmel State
//// IP_DISCONNECTED as IDLE  state				PARTYIDLE
//// IP_CONNECTING as SETUP state  				before signaling start- sPARTY_OPENINCHANNELS, sPARTY_OPENBRIDGES,
////												After signaling start - sPARTY_CONNECTING, sPARTY_CHANGECHANNELS, sPARTY_RECOVERY
////												sPARTY_INITREINVITE, sPARTY_INITREINVITECLOSECHAN, sPARTY_OPENOUTCHANNELS
////												sPARTY_RMTCONNECTED
//// IP_CHANGEMODE as CHANGEMODE state			this is party of the SipParty State machine only!
//// IP_CONNECTED	as CONNECT state  				this is party of the SipParty State machine only!
//// IP_DISCONNECTING	as DISCONNECTING state		this is party of the SipParty State machine only!
//
//PBEGIN_MESSAGE_MAP(CSipPartyOut)
//
//ONEVENT(ALLOCATE_PARTY_RSRC_IND,		PARTYIDLE,				CSipPartyOut::OnConfInformResourceAllocatedIdle)//Idle Case
//ONEVENT(SIP_CONF_ESTABLISH_CALL,		PARTYIDLE,				CSipPartyOut::OnConfEstablishCallIdle)
//ONEVENT(CONFDISCONNECT,					PARTYIDLE,				CSipPartyOut::OnConfCloseCallIdle)
//// DNS Resolution
//ONEVENT(SIP_PARTY_DNS_RES,				PARTYIDLE,				CSipPartyOut::OnDnsResolutionCallIdle)
//ONEVENT(DNSQUARYTOUT,					PARTYIDLE,				CSipPartyOut::OnSipDisconnectIdle)
//
//ONEVENT(SIP_PARTY_CHANS_CONNECTED,		sPARTY_OPENINCHANNELS,	CSipPartyOut::OnPartyChannelsConnectedOpenIn)
//
//ONEVENT(AUDBRDGCONNECT,					sPARTY_OPENBRIDGES,		CSipParty::OnConfPartyReceiveAudBridgeConnected)
//ONEVENT(VIDBRDGCONNECT,					sPARTY_OPENBRIDGES,		CSipParty::OnConfPartyReceiveVidBridgeConnected)
//ONEVENT(FECCBRDGCONNECT,				sPARTY_OPENBRIDGES,		CSipParty::OnConfPartyReceiveFeccBridgeConnected)
//
//ONEVENT(SIP_PARTY_ORIGINAL_RMOTCAP,		sPARTY_CONNECTING,		CSipParty::OnPartyOriginalRemoteCaps)
//ONEVENT(SIP_PARTY_RECEIVED_200OK,		sPARTY_CONNECTING,		CSipPartyOut::OnPartyReceived200OkConnecting)
//ONEVENT(PARTYCONNECTTOUT,				sPARTY_CONNECTING,		CSipPartyOut::OnPartyConnectToutConnecting)
//
//ONEVENT(SET_SITE_AND_VISUAL_NAME,		sPARTY_CONNECTING,		CSipParty::SendSiteAndVisualNamePlusProductIdToPartyControl)
//
//ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,	sPARTY_CHANGECHANNELS,	CSipPartyOut::OnPartyCloseChannelConnecting)//the case when we close the video in channel because we can't recover video
//
//ONEVENT(SIP_PARTY_CHANS_UPDATED,		sPARTY_RECOVERY,		CSipPartyOut::OnPartyChannelsUpdatedRecovery)
//ONEVENT(PARTYCONNECTTOUT,				sPARTY_RECOVERY,		CSipPartyOut::OnPartyConnectToutConnecting)
//
//// for External recovery case only
//ONEVENT(SIP_PARTY_REINVITE_RESPONSE,	sPARTY_INITREINVITE,	CSipPartyOut::OnPartyReInviteResponseInitReInvite)// case of external recovery
//ONEVENT(PARTYCONNECTTOUT,				sPARTY_INITREINVITE,	CSipPartyOut::OnPartyConnectToutConnecting)
//ONEVENT(SIP_PARTY_CHANS_DISCONNECTED,	sPARTY_INITREINVITECLOSECHAN,CSipPartyOut::OnPartyChannelsCloseReInviteCloseChannels)
//// for External, Internal and geuss succeed states.
//ONEVENT(SIP_CONF_CONNECT_CALL,			sPARTY_RMTCONNECTED,	CSipPartyOut::OnConfConnectCallRmtConnected)
//ONEVENT(SIP_PARTY_CHANS_CONNECTED,		sPARTY_OPENOUTCHANNELS,	CSipPartyOut::OnPartyChannelsConnectedOpenOut)// case of internal recovery and external recovery as well
//
//ONEVENT(CONFDISCONNECT,					PARTYDISCONNECTING,		CSipParty::OnConfReadyToCloseCall)
//
//ONEVENT(CONFDISCONNECT,					ANYCASE,				CSipPartyOut::OnConfCloseCall)
//
//  // Self timers
//ONEVENT(PARTYCONTOUT  	 				,PARTYIDLE   				,CSipPartyOut::OnSipDisconnectIdle)
//ONEVENT(PARTYCONTOUT  	 				,ANYCASE 					,CSipPartyOut::NullActionFunction)
//
//ONEVENT(OPENBRIDGESTOUT  				,sPARTY_OPENBRIDGES   		,CSipParty::OnConfBridgesConnectionTout)
//ONEVENT(OPENBRIDGESTOUT   				,ANYCASE 					,CSipPartyOut::NullActionFunction)
//
//
//PEND_MESSAGE_MAP(CSipPartyOut,CSipParty);
//
/////////////////////////////////////////////////////////
//CSipPartyOut::CSipPartyOut()
//{
//	VALIDATEMESSAGEMAP
//}
//
//
/////////////////////////////////////////////////////////
//CSipPartyOut::~CSipPartyOut()
//{
//}
//
/////////////////////////////////////////////////////////
//void  CSipPartyOut::Create(CSegment& appParam)
//{
//	CIpParty::Create(appParam);
//	m_pConfApi = new CConfApi;
//	m_pConfApi->CreateOnlyApi(*m_pCreatorRcvMbx, NULL, NULL, 1);
//
//	// update initial number of retrials
//	DWORD redialnum = GetSystemCfgFlagInt<DWORD>(CFG_KEY_NUMBER_OF_REDIAL); //::GetpConfCfg()->GetNumRedials();
//	m_pConfApi->UpdateDB(this, NUMRETRY,(DWORD) redialnum,1);
//
//}
//
/////////////////////////////////////////////////////////
//void CSipPartyOut::CleanUp()
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::CleanUp: Name ",m_partyConfName);
//	switch(m_eDialState)
//	{
//		// before invite sent to Ep (could be internal failure)
//	case kCapsDontMatch:
//	case kChannelsFailedToOpen:
//	case kTerminateByConf:
//	case kBeforeInvite:
//	case kNotInDialState:
//		if (m_isPreSignalingFlowProb == 0)
//			m_pSipCntl->CloseCall(YES);
//		break;
//
//		// after invite sent to Ep before answer arrived
//	case kCancelInvite:
//	case kInviteSent:
//		m_pSipCntl->CancelCall(kCancelAfterInvite);
//		break;
//
//		// after answer arrived from Ep
//	case kRejectArrived:
//	case kTerminateByRemote:
//		m_pSipCntl->CloseCall(NO); // send ack for the reject
//		break;
//
//	case kNoRecovery:
//	case kNoRecoveryForVideo:
//	case kInternalRecovery:
//	case kExternalRecovery:
//	case kGuessSucceeded:
//		m_pSipCntl->SipInviteAckReq();	// for the 200 ok
//		m_pSipCntl->CloseCall(YES);		// send bye
//		break;
//
//	case kBadStatusArrived:
//	case kTransportErrorArrived:
//	case kDisconnectTimer:
//		m_pSipCntl->ViolentCloseCall();
//		break;
//
//	case kReInviteSent:			// wait for answer
//	case kReInviteAccepted:
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyOut::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
//		break;
//
//	case kReInviteRejected://wait for an ack to arrive (or timer to popped)
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyOut::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
//		m_eDialState = kTerminateByConf;
//		break;
//
//	case kConnectTimer:
//		m_pSipCntl->CloseCall(YES);
//		break;
//
//	case kReInviteArrived:
//		m_pSipCntl->SipInviteResponseReq(SipCodesDecline); // wait for ack
//		break;
//
//	case kByeArrived:
//		m_pSipCntl->CloseCall(NO);
//		break;
//
//	default:
//		if(m_eDialState == 0)
//			DBGPASSERT(kLastDialState+1);
//		else
//			DBGPASSERT(m_eDialState);
//		break;
//	}
//}
//
//
/////////////////////////////////////////////////////////
//void CSipPartyOut::OnConfInformResourceAllocatedIdle(CSegment* pParam)
//{// this message is used to initiate ringing in dial out
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnConfInformResourceAllocated. Do nothing wait for conf establish call command: Name ",m_partyConfName);
//}
//
/////////////////////////////////////////////////////////
//void CSipPartyOut::OnConfCloseCallIdle(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnConfCloseIdleCall: Name ",m_partyConfName);
//	if (IsValidTimer(PARTYCONTOUT))
//		DeleteTimer(PARTYCONTOUT);
//	if (IsValidTimer(DNSQUARYTOUT))
//		DeleteTimer(DNSQUARYTOUT);
//
//
//	if(m_eDialState == kNotInDialState)
//		DestroyPartyTask();
//	else
//		DBGPASSERT(m_eDialState);
//}
//
/////////////////////////////////////////////////////////
//void CSipPartyOut::OnConfCloseCall(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnConfCloseCall: Name ",m_partyConfName);
//	if (IsValidTimer(OPENBRIDGESTOUT))
//		DeleteTimer(OPENBRIDGESTOUT);
//	if (IsValidTimer(PARTYCONTOUT))
//		DeleteTimer(PARTYCONTOUT);
//	if (IsValidTimer(DNSQUARYTOUT))
//		DeleteTimer(DNSQUARYTOUT);
//
//	m_state = PARTYDISCONNECTING;
//	LogicalChannelDisconnect(SDP);
//
//	switch(m_eDialState)
//	{
//	case kBeforeInvite:
//		m_eDialState = kTerminateByConf;
//		break;
//	case kInviteSent:
//		m_eDialState = kCancelInvite;
//		break;
//	default:; // ok arrived (or any other dial state) - don't change dial state
//	}
//
//	CleanUp();
//}
//
//
/////////////////////////////////////////////////////////
//void CSipPartyOut::OnConfEstablishCallIdle(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnConfEstablishCallIdle: Name ",m_partyConfName);
////	InitTimer(GetRcvMbx());
//
//	CRsrcParams* pRtpRsrcParams = new CRsrcParams;
//	CRsrcParams* pCsRsrcParams = new CRsrcParams;
//	UdpAddresses sUdpAddressesParams;
//	CSipNetSetup*	pNetSetup		= new CSipNetSetup;
////	CIpRsrcDesc*	pRsrcDesc		= new CIpRsrcDesc;
//	CSipCaps*		pLocalCaps		= new CSipCaps;
//	CQoS*			pQos			= new CQoS;
//	DWORD			confParamLen	= 0;
//	char*			strConfInfo		= NULL;
//	BYTE			eTransportType	= 0;
//	DWORD			addrLen			= 0;
//
//	pNetSetup->DeSerialize(NATIVE,*pParam);
//	pLocalCaps->DeSerialize(NATIVE,*pParam);
//	m_pTargetMode->DeSerialize(NATIVE,*pParam);
//	pQos->DeSerialize(NATIVE,*pParam);
//	*pParam >> m_bIsAdvancedVideoFeatures;
//
//	*pParam >> confParamLen;
//	if(confParamLen)
//	{
//		strConfInfo = new char[confParamLen];
//		pParam->Get((BYTE*)strConfInfo,confParamLen);
//		strConfInfo[confParamLen-1]=0;
//	}
//
//	*pParam >> eTransportType;
//	pRtpRsrcParams->DeSerialize(NATIVE, *pParam);
//	pCsRsrcParams->DeSerialize(NATIVE, *pParam);
//	pParam->Get((BYTE *) &sUdpAddressesParams, sizeof(UdpAddresses));
//
//	*pParam >> m_mcuNum;
//	*pParam >> m_termNum;
//
//	*pParam >> addrLen;
//	if (addrLen)
//	{
//		PDELETEA(m_alternativeAddrStr);
//		m_alternativeAddrStr = new char[addrLen];
//		memset( m_alternativeAddrStr, 0, addrLen);
//		pParam->Get((BYTE*)m_alternativeAddrStr,addrLen);
//		m_alternativeAddrStr[addrLen-1]=0;
//	}
//
//
//	m_PartyRsrcID = pCsRsrcParams->GetPartyRsrcId();
//	m_ConfRsrcId = pCsRsrcParams->GetConfRsrcId();
//	m_pSipCntl->Create(this, pNetSetup, /*pRsrcDesc,*/ YES, m_serviceId);
//	m_pSipCntl->SetLocalCaps(*pLocalCaps);
//	m_pSipCntl->SetQos(*pQos);
//	m_pSipCntl->SetConfParamInfo(strConfInfo);
//	m_pSipCntl->SetTransportType((enTransportType) eTransportType);
//	m_pSipCntl->SetControllerResource(pRtpRsrcParams, pCsRsrcParams, sUdpAddressesParams);
//	m_pSipCntl->AddToRoutingTable();
//
//	POBJDELETE(pRtpRsrcParams);
//	POBJDELETE(pCsRsrcParams);
//	POBJDELETE(pNetSetup);
//	//POBJDELETE(pRsrcDesc);
//	POBJDELETE(pLocalCaps);
//	POBJDELETE(pQos);
//	PDELETEA(strConfInfo);
//
//	if (IsValidTimer(PARTYCONTOUT))
//	{
//		PTRACE(eLevelInfoNormal,"CSipPartyOut::OnConfEstablishCallIdle - Delete PARTYCONTOUT");
//		DeleteTimer(PARTYCONTOUT);
//	}
//
//	// if the only audio cap is Rfc2833 we should reject the call.
//	// if the only audio cap is Rfc2833 we should reject the call.
//	if (m_pTargetMode
//			&& m_pTargetMode->IsMediaOn(cmCapAudio,cmCapReceive) && m_pTargetMode->IsMediaOn(cmCapAudio,cmCapTransmit)
//		 	&& (m_pTargetMode->GetMediaType(cmCapAudio) != eRfc2833DtmfCapCode))
//	{
//		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode);
//		m_state = sPARTY_OPENINCHANNELS;
//		m_eDialState = kBeforeInvite;
//	}
//	else
//	{
//		DBGPASSERT(YES);
//		PTRACE(eLevelError,"CSipPartyOut::OnConfEstablishCall: No target mode found. must reject call");
//		m_eDialState = kCapsDontMatch;
//		TellConfOnDisconnecting(SIP_CAPS_DONT_MATCH);
//	}
//}
//
//
///////////////////////////////////////////////////////////////////////////////////
//void CSipPartyOut::HandleBridgeConnectedInd(DWORD status)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::HandleBridgeConnectedInd: Name ",m_partyConfName);
//
//	if (status == STATUS_OK)
//	{
//		// its OK if both the bridges connected or its audio only call or there is no much in the video capability
//		if((m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_isFeccBridgeConnected) ||
//			(m_isAudioBridgeConnected && m_isVideoBridgeConnected && m_pTargetMode->IsMediaOff(cmCapData,cmCapReceiveAndTransmit)) ||
//			(m_isAudioBridgeConnected && GetIsVoice()) ||
//			(m_isAudioBridgeConnected && m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceiveAndTransmit)))
//		{
//			if (m_eDialState == kBeforeInvite)
//			{
//				if (IsValidTimer(OPENBRIDGESTOUT))
//				{
//					DeleteTimer(OPENBRIDGESTOUT);
//					PTRACE(eLevelInfoNormal,"CSipPartyOut::HandleBridgeConnectedInd: DeleteTimer(OPENBRIDGESTOUT) ");
//				}
//				m_state		 = sPARTY_CONNECTING;
//				m_eDialState = kInviteSent;
//				m_pSipCntl->SipInviteReq(m_alternativeAddrStr);
//			}
//		}
//	}
//	else
//	{
////		m_eDialState = kBadStatusAckArrived;
//		DBGPASSERT(status);
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyOut::HandleBridgeConnectedInd: Ack with bad status %d",status);
//		TellConfOnDisconnecting(status);
//	}
//}
//
///////////////////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnConfConnectCallRmtConnected(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnConfConnectCallRmtConnected: Name ",m_partyConfName);
//	m_state = sPARTY_OPENOUTCHANNELS;
//	OpenOutChannels();
//}
//
///////////////////////////////////////////////////////////////////////////////////
//void CSipPartyOut::PartyConnectCall()
//{
//	switch(m_eDialState)
//	{
//		case kGuessSucceeded:
//		case kNoRecoveryForVideo:
//			ConnectCall();
//			break;
//
//		case kReInviteSent:
//		case kNotInDialState:
//			PTRACE2INT(eLevelInfoNormal, "CSipPartyOut::PartyConnectCall: Do nothing on dial state Name ", m_eDialState);  // there is nothing to do
//			break;
//
//		default:
//			DBGPASSERT(m_eDialState);
//	}
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::ConnectCall()
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::ConnectCall: Name ", m_partyConfName);
//	m_pSipCntl->SipInviteAckReq();
//
//	m_state = PARTYCONNECTED;
//
//	CIpServiceListManager* pIpServiceListManager = ::GetIpServiceListMngr();
//	CConfIpParameters* pServiceParams = pIpServiceListManager->FindIpService(m_serviceId);
//	// IpV6 - Monitoring
//	mcTransportAddress localIp;
//	memset(&localIp,0,sizeof(mcTransportAddress));
//	mcTransportAddress remoteIp;
//	memset(&remoteIp,0,sizeof(mcTransportAddress));
//
//	ipAddressIf localAddress;
//	if (pServiceParams == NULL)
//	{
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyOut::ConnectCall: IP Service does not exist!!! ServiceID = ",m_serviceId);//m_pSipCntl->GetServiceId());
//		return;
//	}
//	CSipNetSetup* pNetSetup = const_cast<CSipNetSetup*>(m_pSipCntl->GetNetSetup());
//	mcTransportAddress* pDestTaAddr = const_cast<mcTransportAddress*>(pNetSetup->GetTaDestPartyAddr());
//	if (pNetSetup->GetIpVersion() == eIpVersion4)
//	{
//		localAddress = pServiceParams->GetIpV4Address();
//		localIp.addr.v4.ip = localAddress.v4.ip;
//	}
//	else
//	{
//		BYTE place = ::FindIpVersionScopeIdMatchBetweenPartyAndService(pDestTaAddr, pServiceParams);
//		if (place == 0xFF)
//		{
//			PASSERTMSG(4,"CSipPartyOut::ConnectCall - No IpV6 in Service");
//			return;
//		}
//		localAddress = pServiceParams->GetIpV6Address((int)place);
//		memcpy(&localIp.addr,&localAddress,sizeof(ipAddressIf));
//	}
//
//
//	m_pSipCntl->GetRemoteMediaIpAsTrAddr(cmCapAudio,remoteIp);
//	localIp.port = 5060;
//	remoteIp.port = 5060;
//	DWORD actualRate = 0xFFFFFFFF;
//	SetPartyMonitorBaseParamsAndConnectChannel(SIGNALING,actualRate,&remoteIp,&localIp);
//
//	m_eDialState = kNotInDialState;
//
//	UpdateDbOnSipPrivateExtension();
//	StartIvr();
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnConfDisconnectChannelsConnecting(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnConfDisconnectChannelsConnecting: Name ",m_partyConfName);
//	if (m_eDialState != kNotInDialState)
//		m_pSipCntl->SipInviteAckReq(); // for the last response
//	CSipParty::OnConfDisconnectChannels(pParam);
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyChannelsConnectedOpenIn(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyChannelsConnectedOpenIn: Name ",m_partyConfName);
//	if (m_eDialState == kBeforeInvite) // channels in connected
//	{
//		ChannelsConnected(pParam);
//		m_state = sPARTY_OPENBRIDGES;
//		StartTimer(OPENBRIDGESTOUT, 10*SECOND);
//	}
//	else
//		PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyChannelsConnectedOpenIn: wrong dial state, Name ",m_partyConfName);
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::InformConfRemoteConnect()
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::InformConfRemoteConnect: Name ",m_partyConfName);
//	m_pConfApi->SipPartyRemoteConnected(this, m_pTargetMode);
//}
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyChannelsConnectedOpenOut(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyChannelsConnectedOpenOut: Name ",m_partyConfName);
////	m_state = sPARTY_CALLCONNECTED;
//
//	//channels out connected
//	EConfType eConfType = m_pTargetMode->GetConfType();
//	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
//	m_pCurrentMode->SetConfType(eConfType);
//	m_pSipCntl->SendRemoteNumbering();
//	MuteMediaIfNeeded(cmCapTransmit);
//	m_pConfApi->SipPartyChannelsConnected(this,m_pCurrentMode);
//	UpdateDbOnChannelsConnected();
//	PartyConnectCall();
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyCallFailed(CSegment* pParam)
//{
//	DWORD reason = 0xFFFFFFFF;
//	DWORD MipErrorNumber = 0;
//	*pParam >> reason;
//	PTRACE2INT(eLevelInfoNormal,"CSipPartyOut::OnPartyCallFailed: Reason - ",reason);
//	const char* alternativeAddrStr = NULL;
//
//	if (reason >= LOW_REJECT_VAL && reason < HIGH_REJECT_VAL)
//	{
//		m_eDialState = kRejectArrived;
//		if (reason == SipCodesMovedPerm || reason == SipCodesMovedTemp) // call forwards
//			alternativeAddrStr = m_pSipCntl->GetForwardAddr();
//	}
//	else
//	{
//		DBGPASSERT(reason);
//
//		if (reason == SIP_CARD_REJECTED_CHANNELS)
//		{
//			reason = MCU_INTERNAL_PROBLEM; //change for disconnect cause in GUI
//			*pParam >> MipErrorNumber;
//			if (m_eDialState == kBeforeInvite)
//				m_eDialState = kChannelsFailedToOpen;
//			else if (m_eDialState == kInviteSent)
//				m_eDialState = kCancelInvite;
//			// else ok arrived (or any other dial state) - don't change dial state
//		}
//		else if (m_eDialState == kInviteSent && (APIS32)reason < 0)
//		{
//			// ok arrived with parsing error - act like no recovery. send ack and bye
//			m_eDialState = kNoRecovery;
//		}
//	}
//	TellConfOnDisconnecting(reason,alternativeAddrStr,MipErrorNumber);
//}
//
//
/////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyReceived200OkConnecting(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: Name ",m_partyConfName);
//	// pass the product ID to party control
//	GetProductIdAndSendToConfLevel();
//
//	CSipCaps*	pRemoteCaps		= new CSipCaps;
//	BYTE		bRemovedCaps	= NO;
//	pRemoteCaps->DeSerialize(NATIVE,*pParam);
//	*pParam >> bRemovedCaps;
//	// IpV6 - Monitoring
//	mcTransportAddress localIp;
//	memset(&localIp,0,sizeof(mcTransportAddress));
//	mcTransportAddress remoteIp;
//	memset(&remoteIp,0,sizeof(mcTransportAddress));
//
//	m_pSipCntl->GetLocalMediaIpAsTrAddr(localIp);
//	m_pSipCntl->GetRemoteMediaIpAsTrAddr(cmCapAudio,remoteIp); // take the audio ip as default
//	localIp.port = 5060;
//	remoteIp.port = 5060;
//	DWORD actualRate=0xFFFFFFFF;
//	const CSipCaps*		pLocalCaps = m_pSipCntl->GetLocalCaps();
//
//	SetPartyMonitorBaseParamsAndConnectChannel(SDP,actualRate,&remoteIp,&localIp);
//	//update RemoteCaps member with remote side capabilities
//	m_pConfApi->SipPartyRemoteCapsRecieved(this,pRemoteCaps,m_pCurrentMode);
//
//	ESipDialState eAudioState 		= kGuessSucceeded;
//	ESipDialState eVideoState 		= kGuessSucceeded;
//	ESipDialState eDataState 		= kGuessSucceeded;
//	int		numOfAudioCaps			= pRemoteCaps->GetNumOfMediaCapSets(cmCapAudio);
//	int		numOfVideoCaps			= pRemoteCaps->GetNumOfMediaCapSets(cmCapVideo);
//	int		numOfDataCaps			= pRemoteCaps->GetNumOfMediaCapSets(cmCapData);
//	BYTE	bIsDtmf					= pRemoteCaps->IsCapSet((CCapSetInfo) eRfc2833DtmfCapCode);
//	BYTE	bIsCloseChannels		= FALSE;
//	BYTE	bIsCloseDataChannels	= FALSE;
//
//	BYTE bIsReInviteNotEnabled = (GetSystemCfgFlagInt<BOOL>(CFG_KEY_SIP_REINVITE) == NO);
//
//	// Identify the media states.
//	if (bIsDtmf) // dtmf is sent on audio channel but it is not an audio algorithm.
//		numOfAudioCaps--;
//
//	// ----check audio----
//	if (numOfAudioCaps == 0)// no audio
//	{
//		eAudioState = kNoRecovery;
//		PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: No remote audio cap. Name ",m_partyConfName);
//	}
//	else if (numOfAudioCaps == 1 && bRemovedCaps == NO)
//	{
//		eAudioState = kNoRecovery;
//
//		CBaseCap* pRmtAudio = pRemoteCaps->GetCapSet(cmCapAudio,0);
//		if (bIsDtmf && pRmtAudio->GetCapCode() == eRfc2833DtmfCapCode) // if the first one is dtmf take the second
//		{
//			POBJDELETE(pRmtAudio);
//			pRmtAudio = pRemoteCaps->GetCapSet(cmCapAudio,1);
//		}
//		if (pRmtAudio)
//		{
//			CBaseCap* pTargetAudio 	= m_pTargetMode->GetMediaAsCapClass(cmCapAudio,cmCapReceive);
//			WORD details			= 0;
//			BYTE bIsContaining 		= NO;
//			if(pTargetAudio)
//			{
//				WORD audioValuesToCompare = kCapCode;//|kFrameRate; for incoming channels we only check that its the same alg since the EP declare in its SDP on its capabilities (what it can receive)
//				bIsContaining = pRmtAudio->IsContaining(*pTargetAudio,audioValuesToCompare,&details);
//			}
//			eAudioState = bIsContaining? kGuessSucceeded: kInternalRecovery;
//			POBJDELETE(pTargetAudio);
//		}
//		else
//			DBGPASSERT(YES);
//		POBJDELETE(pRmtAudio);
//	}
//	else if(bIsReInviteNotEnabled)
//	{// if we declared only on one algorithm per media, we need to check that the first cap from each media contained in the remote caps
//		// Temporary code!! the code here is very bad need to add "IsMediaContaining" function
//		eAudioState = kNoRecovery;
//		CBaseCap* pLocalAudio = pLocalCaps->GetCapSet(cmCapAudio,0);
//		if (pLocalAudio->GetCapCode() == eRfc2833DtmfCapCode) // if the first one is dtmf take the second
//		{
//			POBJDELETE(pLocalAudio);
//			pLocalAudio = pLocalCaps->GetCapSet(cmCapAudio,1);
//		}
//
//		// check if the remote contained the local caps
//		if (pLocalAudio)
//		{// we don't need to check payload since its receive channel and the remote has to transmit is streams with the Carmel payload numbers.
//			WORD details	= 0;
//			BYTE values		= 0;
//			int  arrIndex	= 0;
//			WORD audioValuesToCompare = kCapCode;//|kFrameRate;for incoming channels we only check that its the same alg since the EP declare in its SDP on its capabilities (what it can receive)
//			BYTE bIsContainingAudioAlg = pRemoteCaps->IsContainingCapSet(cmCapReceive, *pLocalAudio, audioValuesToCompare, &details, &arrIndex);
//			eAudioState = bIsContainingAudioAlg? kGuessSucceeded: kNoRecovery;
//		}
//		POBJDELETE(pLocalAudio);
//	}
//	else  // (numOfAudioCaps > 1 || bRemovedCaps)
//	{
//		eAudioState = kNoRecovery;
//		// if we have only two audio algorith when one is DTMF continue.
//		if((pLocalCaps->GetNumOfMediaCapSets(cmCapAudio) == 2) && pLocalCaps->IsCapSet(eRfc2833DtmfCapCode))
//		{
//			CBaseCap* pLocalAudio = pLocalCaps->GetCapSet(cmCapAudio,0);
//			if (pLocalAudio->GetCapCode() == eRfc2833DtmfCapCode) // if the first one is dtmf take the second
//			{
//				POBJDELETE(pLocalAudio);
//				pLocalAudio = pLocalCaps->GetCapSet(cmCapAudio,1);
//			}
//
//			// check if the remote contained the local caps
//			if (pLocalAudio)
//			{// we don't need to check payload since its receive channel and the remote has to transmit is streams with the Carmel payload numbers.
//				WORD details	= 0;
//				BYTE values		= 0;
//				int  arrIndex	= 0;
//				WORD audioValuesToCompare = kCapCode;//|kFrameRate;for incoming channels we only check that its the same alg since the EP declare in its SDP on its capabilities (what it can receive)
//				BYTE bIsContainingAudioAlg = pRemoteCaps->IsContainingCapSet(cmCapReceive, *pLocalAudio, audioValuesToCompare, &details, &arrIndex);
//				eAudioState = bIsContainingAudioAlg? kGuessSucceeded: kNoRecovery;
//			}
//			POBJDELETE(pLocalAudio);
//		}
//		else
//			eAudioState = kExternalRecovery;
//		PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: More than one remote audio cap. Name ",m_partyConfName);
//	}
//
//	// ----check video----
//	if (numOfVideoCaps == 0) // no video
//	{
//		m_voice = YES;
//		eVideoState = kNoRecovery;
//		if (m_pCurrentMode->IsMediaOn(cmCapVideo))
//		{
//			eVideoState = kNoRecovery;
//			PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: No remote video cap. Name ",m_partyConfName);
//		}
//		else // local didn't want video either
//			eVideoState = kGuessSucceeded;
//	}
//
//	else if (numOfVideoCaps == 1 && bRemovedCaps == NO)
//	{
//		eVideoState = kInternalRecovery;
//		/*eVideoState = kNoRecovery;
//		CBaseCap* pRmtVideo = pRemoteCaps->GetCapSet(cmCapVideo,0);
//		if (pRmtVideo)
//		{// we don't need to check payload since its receive channel and the remote has to transmit is streams with the Carmel payload numbers.
//			WORD details	= 0;
//			int arrInd = 0;
//			CBaseCap* pTargetVideo 	= m_pTargetMode->GetMediaAsCapClass(cmCapVideo,cmCapReceive);
//			BYTE bIsContaining 		= NO;
//			if(pTargetVideo)
//			{
//				BYTE videoValuesToCompare = kCapCode;//|kFormat|kFrameRate|kH264Level|kH264Additional|kBitRate;for incoming channels we only check that its the same alg since the EP declare in its SDP on its capabilities (what it can receive)
//				bIsContaining = pRmtVideo->IsContaining(*pTargetVideo,videoValuesToCompare,&details);
//			}
//			eVideoState = bIsContaining? kGuessSucceeded: kInternalRecovery;
//			POBJDELETE(pTargetVideo);
//		}
//		else
//			DBGPASSERT(YES);
//		POBJDELETE(pRmtVideo);*/
//	}
//	else if(bIsReInviteNotEnabled)
//	{// if we declared only on one algorithm per media, we need to check that the first cap from each media contained in the remote caps
//		// Temporary code!! the code here is very bad need to add "IsMediaContaining" function
//		// get the local cap set of audio and video
//		eVideoState = kNoRecovery;
//		CBaseCap* pLocalVideo = NULL;
//		for(int index = 0; index < pLocalCaps->GetNumOfCapSets(); index++)
//		{
//			pLocalVideo = pLocalCaps->GetCapSet(cmCapVideo,index);
//			if(pLocalVideo)
//				break;
//		}
//
//		// check if the remote contained the local caps
//		if (pLocalVideo)
//		{
//			WORD details	= 0;
//			BYTE values		= 0;
//			int  arrIndex	= 0;
//			WORD videoValuesToCompare = kCapCode;//|kFormat|kFrameRate|kH264Level|kH264Additional|kBitRate;for incoming channels we only check that its the same alg since the EP declare in its SDP on its capabilities (what it can receive)
//			BYTE bIsContainingVideoAlg = pRemoteCaps->IsContainingCapSet(cmCapReceive, *pLocalVideo, videoValuesToCompare, &details, &arrIndex);
//			eVideoState = bIsContainingVideoAlg? kGuessSucceeded: kNoRecovery;
//		}
//		POBJDELETE(pLocalVideo);
//	}
//	else // (numOfVideoCaps > 1 || bRemovedCaps)
//	{
//	/*	eVideoState = kNoRecovery;
//		if(pLocalCaps->GetNumOfMediaCapSets(cmCapVideo) == 1)
//		{
//			CBaseCap* pLocalVideo = NULL;
//			pLocalVideo = pLocalCaps->GetCapSet(cmCapVideo,0);
//			// check if the remote contained the local caps
//			if (pLocalVideo)
//			{
//				WORD details	= 0;
//				BYTE values		= 0;
//				int  arrIndex	= 0;
//				BYTE videoValuesToCompare = kCapCode;//|kFormat|kFrameRate|kH264Level|kH264Additional|kBitRate;for incoming channels we only check that its the same alg since the EP declare in its SDP on its capabilities (what it can receive)
//				BYTE bIsContainingVideoAlg = pRemoteCaps->IsContainingCapSet(cmCapReceive, *pLocalVideo, videoValuesToCompare, &details, &arrIndex);
//				eVideoState = bIsContainingVideoAlg? kGuessSucceeded: kNoRecovery;
//			}
//			POBJDELETE(pLocalVideo);
//		}
//		else // re-invite
//			eVideoState = kExternalRecovery;*/
//		eVideoState = kExternalRecovery;
//		PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: More than one remote video cap. Name ",m_partyConfName);
//	}
//
//	// ----check fecc (data)----
//	// As for now, we support and declare in our Invite only one data capability - AnnexQ.
//	// If remote have it, there is nothing to do. (no internal recovery and not external recovery)
//	// If remote doesn't have it - only close the channels. (no external recovery needed).
//	if (numOfDataCaps == 0) // no data
//	{
//		eDataState = kNoRecovery;
//		if (m_pCurrentMode->IsMediaOn(cmCapData))
//		{
//			eDataState = kNoRecovery;
//			PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: No remote data cap. Name ",m_partyConfName);
//		}
//		else // local didn't want data either
//			eDataState = kGuessSucceeded;
//	}
//
//	else // numOfDataCaps >= 1
//	{
//		eDataState = kNoRecovery;
//		CBaseCap* pLocalData = NULL;
//		pLocalData = pLocalCaps->GetCapSet(cmCapData, 0);
//
//		// check if the remote contained the local caps
//		if (pLocalData)
//		{
//			WORD details	= 0;
//			BYTE values		= 0;
//			int  arrIndex	= 0;
//			WORD dataValuesToCompare = kCapCode;
//			BYTE bIsContainingDataAlg = pRemoteCaps->IsContainingCapSet(cmCapReceive, *pLocalData, dataValuesToCompare, &details, &arrIndex);
//			eDataState = bIsContainingDataAlg? kGuessSucceeded: kNoRecovery;
//		}
//		POBJDELETE(pLocalData);
//	}
//
//	// according to media states set the action to occur.
//	if (eAudioState == kGuessSucceeded && eVideoState == kGuessSucceeded/* && eDataState == kGuessSucceeded*/)
//	{
//		PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: Guess succeeded. Name ",m_partyConfName);
//		m_eDialState = kInternalRecovery;
//	}
//
//	// we base on the target mode and change it according to remote caps
//	BYTE bWithinProtocolLimitation = FALSE;
//	CSipComMode* pBestMode	= m_pSipCntl->FindBestModeToOpen((const CSipComMode&)*m_pTargetMode, bWithinProtocolLimitation);
//
//	//update audio mode:
//	if (pBestMode && pBestMode->IsMediaOn(cmCapAudio,cmCapReceive) && pBestMode->IsMediaOn(cmCapAudio,cmCapTransmit))
//		m_pTargetMode->CopyMediaMode(*pBestMode,cmCapAudio,cmCapReceiveAndTransmit);
//	else if (eAudioState != kGuessSucceeded)//in case we needed to choose a new audio mode, and didn't find
//		eAudioState = kNoRecovery;
//
//	//update video mode & recovery state:
//	if (pBestMode && pBestMode->IsMediaOn(cmCapVideo,cmCapReceive) && pBestMode->IsMediaOn(cmCapVideo,cmCapTransmit))
//	{
//		if (eVideoState == kGuessSucceeded) //the bitrate wasn't checked before, so we need to check it now
//		{
//			DWORD prevVideoRate = m_pTargetMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);
//			DWORD newVideoRate  = pBestMode->GetMediaBitRate(cmCapVideo, cmCapTransmit);
//			if (prevVideoRate != newVideoRate)
//				eVideoState = kInternalRecovery;
//		}
//		m_pTargetMode->CopyMediaMode(*pBestMode,cmCapVideo,cmCapReceiveAndTransmit);
//	}
//	else if (eVideoState != kGuessSucceeded)//in case we needed to choose a new video mode, and didn't find
//		eVideoState = kNoRecovery;
//
//	//update data mode:
//	if (pBestMode && pBestMode->IsMediaOn(cmCapData,cmCapReceive) && pBestMode->IsMediaOn(cmCapData,cmCapTransmit))
//		m_pTargetMode->CopyMediaMode(*pBestMode,cmCapData,cmCapReceiveAndTransmit);
//	else if (eDataState != kGuessSucceeded)//in case we needed to choose a new data mode, and didn't find
//		eDataState = kNoRecovery;
//
//	if ((eAudioState == kInternalRecovery && eVideoState == kGuessSucceeded  ) ||
//		(eAudioState == kGuessSucceeded   && eVideoState == kInternalRecovery) ||
//		(eAudioState == kInternalRecovery && eVideoState == kInternalRecovery))
//	{
//		PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: Guess failed. Internal recovery. Name ",m_partyConfName);
//		m_eDialState = kInternalRecovery;
//	}
//	else if (eAudioState == kNoRecovery) // no recovery
//	{
//		DBGPASSERT(YES);
//		PTRACE2(eLevelError,"CSipPartyOut::OnPartyReceived200OkConnecting: Audio remote receive is not matching. Name ",m_partyConfName);
//		m_eDialState = kNoRecovery;
//		TellConfOnDisconnecting(SIP_CAPS_DONT_MATCH);
//		POBJDELETE(pRemoteCaps);
//		return;
//	}
//
//	// video off or could not be recover and audio ok or could be recover
//	else if ((numOfVideoCaps == 0) || (eVideoState == kNoRecovery))
//	{
//		if (m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive)) // only if video channel is open (we only have channel in for now)
//			bIsCloseChannels = TRUE;
//
//		if ((eAudioState == kGuessSucceeded) && (numOfVideoCaps == 0) )
//		{
//			PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: Remote has no video caps, but audio guess succeeded. Name ",m_partyConfName);
//			m_eDialState = kNoRecoveryForVideo;
//		}
//		else if ((numOfVideoCaps > 0) && bWithinProtocolLimitation) // tell the EP to close the video
//		{
//			PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: Remote has no matching video params within protocol. External Recovery. Name ",m_partyConfName);
//			m_eDialState = kExternalRecovery; //need to send re-invite
//		}
//		else
//		{
//			PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: Remote has no video caps, and audio guess failed. Name ",m_partyConfName);
//			m_eDialState = eAudioState; // internal or external
//		}
//	}
//
//	else
//	{
//		PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReceived200OkConnecting: Media remote receive is not matching. External recovery. Name ",m_partyConfName);
//		m_eDialState = kExternalRecovery;
//	}
//	POBJDELETE(pBestMode);
//
//	bIsCloseDataChannels = (eDataState == kNoRecovery);
//
//	m_state = sPARTY_CHANGECHANNELS;
//
//	CloseChannelsIfNeeded(bIsCloseChannels, bIsCloseDataChannels);
//
//	POBJDELETE(pRemoteCaps);
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyCloseChannelConnecting(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyCloseChannelConnecting: Name ",m_partyConfName);
//	m_state = sPARTY_RECOVERY;
//	EConfType eConfType = m_pTargetMode->GetConfType();
//	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
//	m_pCurrentMode->SetConfType(eConfType);
//	UpdateDbOnChannelsDisconnected();
//
//	// since the response to the action taking care at the internal recovery activate external media recovery or open out channels and connecting
//	// the call - there is no need to ask about switch case here if its external or internal recovery
//	// (actually the function should have been called MediaRecovery())
//	m_eChangeModeInitiator = ePartyInitiator;
//	InternalMediaRecovery();
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyBadStatusConnecting(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyBadStatusConnecting: Name ",m_partyConfName);
//	CSipParty::OnPartyBadStatus(pParam);
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::PartyChannelsUpdatedOk()
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::PartyChannelsUpdatedOk: Name ",m_partyConfName);
//	if (m_eDialState == kNotInDialState || m_eDialState == kReInviteAccepted || m_eDialState == kReInviteArrived)
//		CSipParty::PartyChannelsUpdatedOk();
//	else
//	{
//		if (m_eDialState == kInternalRecovery || m_eDialState == kExternalRecovery)
//		{
//			//m_pConfApi->SipPartyChannelsConnected(this,m_pCurrentMode); //will cause vid bridge to connect
//			UpdateDbOnChannelsConnected();
//
////			m_pConfApi->IpMuteMedia(this,cmCapAudio,NO);//Unmute + Update of cur audio algorithm
////			m_pSipCntl->StreamOnMediaIfNeeded(cmCapAudio,cmCapReceive);
//			if (m_eDialState == kInternalRecovery)
//				m_eDialState = kGuessSucceeded;
//		}
//
//		MuteMediaIfNeeded(cmCapReceive);
//
//		//open out channels
//		switch (m_eDialState)
//		{
//		case kGuessSucceeded:
//		case kNoRecoveryForVideo:
//			//open out channels if they are not open yet
//			if (m_pSipCntl->IsMedia(cmCapAudio,cmCapTransmit) == NO ||
//				(m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive) && m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit) == NO))
//				m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode,cmCapTransmit);
//			else
//				ConnectCall();
//			break;
//		case kExternalRecovery:
//				m_pSipCntl->SipInviteAckReq(NO); // don't close session cause we want to send re-invite right away
//				m_pSipCntl->SipReInviteReq(0);
//				m_eDialState = kReInviteSent;
//			break;
//		default:
//			DBGPASSERT(m_eDialState);
//		}
//	}
//}
//
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyChannelsUpdatedRecovery(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyChannelsUpdatedRecovery: Name ",m_partyConfName);
//	EConfType eConfType = m_pTargetMode->GetConfType();
//	m_pCurrentMode->DeSerialize(NATIVE,*pParam);
//	m_pCurrentMode->SetConfType(eConfType);
//
//	UpdateDbOnChannelsConnected();
//
//	if (m_eDialState == kInternalRecovery)
//		m_eDialState = kGuessSucceeded;
//
//	// check mute by party for in channels only. out channels will be checked after connected
//	MuteMediaIfNeeded(cmCapReceive);
//
//	EndInternalRecovery();
//}
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::EndInternalRecovery()
//{
//	//open out channels or External Recovery (Ack and Re-invite)
//	switch (m_eDialState)
//	{
//	case kGuessSucceeded:
//	case kNoRecoveryForVideo:
//		{
//			m_state = sPARTY_RMTCONNECTED;
//			InformConfRemoteConnect();
//			break;
//		}
//	case kExternalRecovery:
//		{
//			m_state = sPARTY_INITREINVITE;
//			m_pSipCntl->SipInviteAckReq(NO); // don't close session cause we want to send re-invite right away
//			m_pSipCntl->SipReInviteReq(0);
//			m_eDialState = kReInviteSent;
//			break;
//		}
//	default:
//			DBGPASSERT(m_eDialState);
//	}
//}
//
//
//////////////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyReInviteResponseInitReInvite(CSegment* pParam)
//{
//	PTRACE2(eLevelInfoNormal,"CSipPartyOut::OnPartyReInviteResponseInitReInvite: Name ",m_partyConfName);
//	m_state = sPARTY_INITREINVITECLOSECHAN;
//
//	DWORD status					= STATUS_OK;
//	CSipCaps*		pCurRemoteCaps	= new CSipCaps;
//	CSipChanDifArr* pChanDifArr		= new CSipChanDifArr;
//	BYTE			bRemovedCaps	= NO;
//	BYTE 			bIsCloseDataChannels = FALSE;
//
//	*pParam >> status;
//	pCurRemoteCaps->DeSerialize(NATIVE,*pParam);
//	pChanDifArr->DeSerialize(NATIVE,*pParam);
//	*pParam >> bRemovedCaps;
//
//	m_eDialState = kNoRecovery; // start from failure
//	CBaseCap* pTargetAudio 	= m_pTargetMode->GetMediaAsCapClass(cmCapAudio,cmCapReceive);
//	if(pTargetAudio)
//	{
//		WORD details	= 0;
//		BYTE values		= 0;
//		int  arrIndex	= 0;
//		WORD audioValuesToCompare = kCapCode;//|kFrameRate;for incoming channels we only check that its the same alg since the EP declare in its SDP on its capabilities (what it can receive)
//		BYTE bIsContainingAudioAlg = pCurRemoteCaps->IsContainingCapSet(cmCapReceive, *pTargetAudio, audioValuesToCompare, &details, &arrIndex);
//		m_eDialState = bIsContainingAudioAlg? kGuessSucceeded: kNoRecovery;
//	}
//	POBJDELETE(pTargetAudio);
//
//	CBaseCap* pTargetVideo 	= m_pTargetMode->GetMediaAsCapClass(cmCapVideo,cmCapReceive);
//	if(pTargetVideo)
//	{
//		WORD details	= 0;
//		BYTE values		= 0;
//		int  arrIndex	= 0;
//		WORD videoValuesToCompare = kCapCode;//|kFrameRate;for incoming channels we only check that its the same alg since the EP declare in its SDP on its capabilities (what it can receive)
//		BYTE bIsContainingVideoAlg = pCurRemoteCaps->IsContainingCapSet(cmCapReceive, *pTargetVideo, videoValuesToCompare, &details, &arrIndex);
//		if (m_eDialState == kGuessSucceeded) // if audio is ok we have 2 choices: secondary (no video) or success
//		{
//			m_eDialState = kNoRecoveryForVideo; // start from video failure
//			m_eDialState = bIsContainingVideoAlg? kGuessSucceeded: kNoRecoveryForVideo;
//		}
//	}
//	POBJDELETE(pTargetVideo);
//
//	CBaseCap* pTargetData 	= m_pTargetMode->GetMediaAsCapClass(cmCapData,cmCapReceive);
//	if(pTargetData)
//	{
//		WORD details	= 0;
//		BYTE values		= 0;
//		int  arrIndex	= 0;
//		WORD dataValuesToCompare = kCapCode;
//		BYTE bIsContainingDataAlg = pCurRemoteCaps->IsContainingCapSet(cmCapReceive, *pTargetData, dataValuesToCompare, &details, &arrIndex);
//		if (!bIsContainingDataAlg)
//			bIsCloseDataChannels = TRUE;
//	}
//	POBJDELETE(pTargetData);
//
//	if (status != STATUS_OK)
//	{
//		PTRACE2INT(eLevelInfoNormal,"CSipPartyOut::OnPartyReInviteResponseInitReInvite: Reject ",status);
//		if (status < LOW_REJECT_VAL || status >= HIGH_REJECT_VAL)
//		{
//			m_eDialState = kNoRecovery; // bad status!!!
//			DBGPASSERT(status);
//		}
//	}
//
//	BYTE bIsCloseChannels = FALSE;
//	switch (m_eDialState)
//	{
//	case kGuessSucceeded:
//		{
//			// send null cause we don't want to update any cap struct only the remote ip.
//			PTRACE(eLevelInfoNormal,"CSipPartyOut::OnPartyReInviteResponseInitReInvite: Guess succeeded");
//		}
//		break;
//	case kNoRecoveryForVideo:
//		{
//			PTRACE(eLevelInfoNormal,"CSipPartyOut::OnPartyReInviteResponseInitReInvite: No Video");
//			if (m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive) ||
//				m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit)) // only if video channel is open
//				bIsCloseChannels = TRUE;
//		}
//		break;
//	case kNoRecovery:
//		PTRACE(eLevelInfoNormal,"CSipPartyOut::OnPartyReInviteResponseInitReInvite: No recovery! Call will be disconnected");
//		TellConfOnDisconnecting(SIP_CAPS_DONT_MATCH);
//		POBJDELETE(pCurRemoteCaps);
//		POBJDELETE(pChanDifArr);
//		return;
//		break;
//	default:
//		DBGPASSERT(m_eDialState);
//	}
//
//	CloseChannelsIfNeeded(bIsCloseChannels, bIsCloseDataChannels);
//
//	POBJDELETE(pCurRemoteCaps);
//	POBJDELETE(pChanDifArr);
//}
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyChannelsCloseReInviteCloseChannels(CSegment* pParam)
//{
//	PTRACE(eLevelInfoNormal,"CSipPartyOut::OnPartyChannelsCloseReInviteCloseChannels: connect call");
//	UpdateDbOnChannelsConnected();
//
//	// check mute by party for in channels only. out channels will be checked after connected
//	// the re-invite can also cause for 200 OK with mute values.
//	MuteMediaIfNeeded(cmCapReceive);
//
//	m_state = sPARTY_RMTCONNECTED;
//	InformConfRemoteConnect();
//}
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OpenOutChannels()
//{
//	//open out channels
//	if (m_pSipCntl->IsMedia(cmCapAudio,cmCapTransmit) == NO ||
//		(m_pSipCntl->IsMedia(cmCapVideo,cmCapReceive) && m_pSipCntl->IsMedia(cmCapVideo,cmCapTransmit) == NO) ||
//		(m_pSipCntl->IsMedia(cmCapData,cmCapReceive) && m_pSipCntl->IsMedia(cmCapData,cmCapTransmit) == NO))
//		m_pSipCntl->SipOpenChannelsReq((CSipComMode *)m_pTargetMode,cmCapTransmit);
//	else
//		DBGPASSERT(1);// if we can't open out channels (at least audio) its a mistake.
//}
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnPartyConnectToutConnecting(CSegment* pParam)
//{
//	PTRACE(eLevelInfoNormal,"CSipPartyOut::OnPartyConnectToutConnecting: Disconnect call");
//	if (m_eDialState != kInviteSent)
//	{
//		m_eDialState = kConnectTimer;
//		DBGPASSERT(m_eDialState);
//	}
//	TellConfOnDisconnecting(SIP_TIMER_POPPED_OUT);
//}
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnSipDisconnectIdle(CSegment * pParam)
//{
//	PASSERTMSG(m_PartyRsrcID,"CSipPartyOut::OnSipDisconnectIdle: Disconnect call");
//	// The m_isPreSignalingFlowProb indicates that we are disconnecting the call before the signaling
//	m_isPreSignalingFlowProb = 1;
//	// Activate "Kill Port"
//	OnPartySendFaultyMfaToPartyCntlAnycase(pParam);
//	BYTE 	mipHwConn = (BYTE)eMipConnectionProcess;
//	BYTE	mipMedia = (BYTE)eMipNoneMedia;
//	BYTE	mipDirect = (BYTE)eMipOut;
//	BYTE	mipTimerStat = (BYTE)eMipTimer;
//	BYTE	mipAction = (BYTE)eMipConnect;
//	DWORD MpiErrorNumber = ::CalculateMcuInternalProblemErrorNumber(mipHwConn,mipMedia,mipDirect,mipTimerStat,mipAction);
//
//	TellConfOnDisconnecting(MCU_INTERNAL_PROBLEM,NULL,MpiErrorNumber);
//	m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
//	DispatchEvebtSipParty(SIP_PARTY_CALL_CLOSED);// disconnect the call.
//}
//
///////////////////////////////////////////////////////////////////////
//void CSipPartyOut::OnDnsResolutionCallIdle(CSegment * pParam)
//{
//	PTRACE(eLevelInfoNormal,"CSipPartyOut::OnDnsResolutionCallIdle");
//	DWORD status = 0;
//
//	*pParam >> status;
//
//	if (IsValidTimer(DNSQUARYTOUT))
//		DeleteTimer(DNSQUARYTOUT);
//
//	if (status == STATUS_OK)
//	{
//		m_pSipCntl->MakeANewCall((CSipComMode*)m_pTargetMode);
//		m_state = sPARTY_OPENINCHANNELS;
//		m_eDialState = kBeforeInvite;
//
//	}
//	else
//	{
//		DBGPASSERT(1);
//		PTRACE(eLevelError,"CSipPartyOut::OnDnsResolutionCallIdle: Problems with DNS quary");
//		m_eDialState = kBadStatusArrived;
//		TellConfOnDisconnecting(SIP_BAD_STATUS);
//
//	}
//
//
//}
//
//

