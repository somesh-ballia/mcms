//+========================================================================+
//                            SIPPartyInCreate.cpp                         |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyInCreate.cpp                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 18/12/06   | This file contains								   |
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
#include "SIPPartyInCreate.h"
#include "ConfPartyGlobals.h"
#include "IpServiceListManager.h"
#include "IPParty.h"
#include "IVRCntl.h"
#include "ServiceConfigList.h"
#include "OpcodesMcmsCardMngrTIP.h"
#include "CdrPersistHelper.h"
extern BOOL GetVendorDetection();
extern CIpServiceListManager* GetIpServiceListMngr();

void SipPartyInCreateEntryPoint(void* appParam)
{
	CSipPartyInCreate* pPartyTaskApp = new CSipPartyInCreate(eSipPartyCntlDefult);
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

void SipPartyInWebRtcCreateEntryPoint(void* appParam)
{
	CSipPartyInCreate* pPartyTaskApp = new CSipPartyInCreate(eSipPartyCntlWebRtc);
	pPartyTaskApp->Create(*(CSegment*)appParam);
}

////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipPartyInCreate)
// conf/lobby events
ONEVENT(REJECTCALL,								PARTYIDLE,					CSipPartyInCreate::OnLobbyRejectIdle)
ONEVENT(LOBBYNETIDENT,							PARTYIDLE,					CSipPartyInCreate::OnLobbyIdentIdle)
ONEVENT(LOBBYTRANS,								PARTYIDLE,					CSipPartyInCreate::OnLobbyTransferIdle)
ONEVENT(LOBBYDESTROY,							PARTYIDLE,					CSipPartyInCreate::OnConfCloseCall)

ONEVENT(SET_SITE_AND_VISUAL_NAME,				sPARTY_WAITFORCONFINITCALL,	CSipParty::SendSiteAndVisualNamePlusProductIdToPartyControl)
ONEVENT(ALLOCATE_PARTY_RSRC_IND,				sPARTY_WAITFORCONFINITCALL,	CSipPartyInCreate::OnConfAllocateResourcesIdle)
ONEVENT(SIP_CONF_ESTABLISH_CALL,				sPARTY_WAITFORCONFINITCALL,	CSipPartyInCreate::OnConfEstablishCallIdle)
ONEVENT(SIP_PARTY_BAD_STATUS,					sPARTY_WAITFORCONFINITCALL,	CSipPartyInCreate::OnPartyBadStatusConnecting)
ONEVENT(PARTY_TRANSLATOR_ARTS_CONNECTED,	    sPARTY_ALLOCATE_TRANSLATOR_ARTS,	CSipPartyInCreate::OnPartyTranslatorArtsConnected)
ONEVENT(TRANS_ICE_CONN_CHECK_COMPLETE_IND,		ANYCASE,					CSipPartyInCreate::OnTransCheckCompleteInd)

ONEVENT(CONFDISCONNECT,							PARTYDISCONNECTING,			CSipParty::OnConfReadyToCloseCall)
ONEVENT(CONFDISCONNECT,							ANYCASE,					CSipPartyInCreate::OnConfCloseCall)
//null action functions
ONEVENT(SET_SITE_AND_VISUAL_NAME,				ANYCASE,					CSipPartyInCreate::SendSiteAndVisualNamePlusProductIdToPartyControl)
// Self timers
ONEVENT(PARTYCONTOUT,							PARTYIDLE,					CSipPartyInCreate::OnSipDisconnectSetupIdle)
ONEVENT(PARTYCONTOUT,							sPARTY_WAITFORCONFINITCALL,	CSipPartyInCreate::OnSipDisconnectSetupIdle)
ONEVENT(PARTYCONTOUT,							ANYCASE,					CSipPartyInCreate::NullActionFunction) // Self timers

//ppc
ONEVENT(SIP_PARTY_BFCP_MSG_IND,					sPARTY_CONNECTING,			CSipParty::OnPartyBfcpMsgInd)
ONEVENT(SIP_PARTY_BFCP_TRANSPORT_IND,			sPARTY_CONNECTING,			CSipParty::OnPartyBfcpTransportInd)
ONEVENT(SIP_PARTY_BFCP_DELAY_ACK_MSG,           sPARTY_CONNECTING,          CSipParty::OnPartyBfcpMsgDelayAck)

ONEVENT(MAKE_ANSWER_IND,						PARTYDISCONNECTING,		CSipPartyInCreate::NullActionFunction)

ONEVENT(SIP_PARTY_RECEIVED_REINVITE,			PARTYDISCONNECTING,		CSipPartyInCreate::NullActionFunction)

//CDR_MCCF:
ONEVENT(SIP_PARTY_STATISTIC_INFO,   IP_CM_PARTY_WAITING_FOR_STATISTIC_INFO_IND,  CSipPartyInCreate::ContinueToCloseTIPcallReq)
PEND_MESSAGE_MAP(CSipPartyInCreate,CSipParty);


////////////////////////////////////////////////////////////////////////////
CSipPartyInCreate::CSipPartyInCreate(ESipPartyCntlType sipCntlType)
: CSipParty(sipCntlType)
{
	m_pLobbyApi = new CLobbyApi;
	m_pConfApi = new CConfApi;
	m_eResponsibility	= kNoResponsibility;
	m_eLobbyRejectReason = SipCodesSipUnknownStatus;
	m_DialInRejectConnectionId = 0xFFFFFFFF;
	m_bSetRsrcParam = FALSE;
	m_minValForGlareTimer = 0; // ticks //BRIDGE-8393 - changed to 0-200 as per rfc was 210-400
	m_maxValForGlareTimer = 200; // ticks
	VALIDATEMESSAGEMAP
}

////////////////////////////////////////////////////////////////////////////
CSipPartyInCreate::~CSipPartyInCreate()
{
	m_pLobbyApi->DestroyOnlyApi();
	POBJDELETE(m_pLobbyApi);
	m_pConfApi->DestroyOnlyApi();
	POBJDELETE(m_pConfApi);
}


////////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::Create(CSegment& appParam)
{
	CSipParty::Create(appParam);
	m_pLobbyApi->CreateOnlyApi(*m_pCreatorRcvMbx);
	m_eResponsibility = kLobby;
}

//////////////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnLobbyIdentIdle(CSegment * pParam)
{
	TRACEINTO << "Name " << m_partyConfName;

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	CConfParty* pConfParty = NULL;

	if (pCommConf)
	{
		pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
	}
	else
	{
		TRACEINTO << "pCommConf is NULL";
		DBGPASSERT(1127);
		return;
	}

	if(!m_pSipCntl)
	{
		TRACEINTO << "m_pSipCntl is NULL";
		DBGPASSERT(1127);
		return;
	}


	// if a call was received from 'Master' - reject (Master should not call others)
	if (m_pSipCntl && (m_pSipCntl->IsRemoteMaster()))
	{
		TRACEINTO << "Call received from cascade Master; illegal";

		m_pConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()),NULL,NULL,1);
		m_pConfApi->DropParty(pConfParty->GetName(), 0, SIP_CLIENT_ERROR_488);
		m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
		::AllocateRejectID(m_DialInRejectConnectionId);
		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
		m_eDialState = kRejectedByLobby;
		m_state = PARTYDISCONNECTING;
		m_eLobbyRejectReason = SipCodesNotAcceptedInHere;
		CleanUp();
	}

	CSipNetSetup* pNetSetup		= new CSipNetSetup;

	DWORD				  len			= 0;
	sipSdpAndHeaders* pSdpAndHeaders= NULL;
	CSipCaps* pRemoteCaps	= new CSipCaps;

	pNetSetup->DeSerialize(NATIVE,*pParam);
	*pParam >> len;
	pSdpAndHeaders = (sipSdpAndHeaders *) new BYTE[len];
	memset(pSdpAndHeaders, 0, len);
	pParam->Get((BYTE*)pSdpAndHeaders,len);

	BYTE isWebRtcCall = IsWebRtcCall(pSdpAndHeaders);

	if (pSdpAndHeaders->sipHeadersOffset)
	{
		PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnLobbyIdentIdle - SDP : Name  ",m_partyConfName);
		m_pSipCntl->SetMediaLinesInternalTypeForRmtSdp(*pSdpAndHeaders, pCommConf);
		m_pSipCntl->SetCapsRolesForRmtSdp(*pSdpAndHeaders, kMediaLineInternalTypeVideo, kRolePeople);
		m_pSipCntl->SetCapsRolesForRmtSdp(*pSdpAndHeaders, kMediaLineInternalTypeContent, kRolePresentation);
		RemoteIdent fromHdrIdent = m_pSipCntl->GetFromIdent(pSdpAndHeaders);
		pRemoteCaps->Create(*pSdpAndHeaders, pCommConf->GetConfMediaType(), m_pSipCntl->GetIsMrcCall(),TRUE, fromHdrIdent);
		//N.A. DEBUG VP8
		CSuperLargeString strCaps1;
		pRemoteCaps->DumpToString(strCaps1);
		PTRACE2(eLevelInfoNormal, "N.A. DEBUG CSipPartyInCreate::OnLobbyIdentIdle pRemoteCaps->Dump", strCaps1.GetString());

	}

	APIU16 plcmReqMask;
	eMediaLineInternalType plcmReqMaskMlineOrder[MaxMediaLinesPlcmReqTag];
	GetPlcmRequireHeaderMask(pSdpAndHeaders,&plcmReqMask, plcmReqMaskMlineOrder);
	m_pSipCntl->SetPlcmRequireMask(plcmReqMask);
	m_pSipCntl->SetPlcmReqMaskMlineOrder(plcmReqMaskMlineOrder);
	BOOL isCiscoTagExist = CheckXciscoInSupportedHeader(pSdpAndHeaders);
	m_pSipCntl->SetIsCiscoTagExist(isCiscoTagExist);

	if(m_voice == NO)// if its a video call, then check if the remote has video, if not change it to audio only call.
		m_voice = (pRemoteCaps->IsMedia(cmCapVideo) == NO);


	CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(m_serviceId);
	if(pService && pService->GetSipServerType() == eSipServer_ms   )
	{
		if(pService->GetIPAddressTypesInService() == eIpType_IpV6)
		{
			TRACEINTO << "ICE is enabled and MS env. and service IP type is ipv6 only";
			pNetSetup->SetIpVersion(eIpVersion6);
		}
		else if(pService->GetIPAddressTypesInService() == eIpType_IpV4)
		{
			TRACEINTO << "ICE is enabled and MS env. and service IP type is ipv4 only";
			pNetSetup->SetIpVersion(eIpVersion4);
		}
	}

	m_pSipCntl->Create(this,pNetSetup,/*pRsrcDesc,*/NO, m_serviceId, 0xFFFFFFFF);
	m_pSipCntl->SetRemoteSdp(*pSdpAndHeaders);
	m_pSipCntl->SetRemoteHeaders(pSdpAndHeaders);
	m_pSipCntl->SetCallLegAndCdrHeaders(*pSdpAndHeaders);

	// eFeatureRssDialin
	//Reject the RSS dialin if there's passowrd in the conf, make sure this is behind the m_pSipCntl->Create() to send 403 response to remote
	if(true ==  IsRssDialinRejected())
	{
		POBJDELETE(pNetSetup);
		PDELETEA(pSdpAndHeaders);
		POBJDELETE(pRemoteCaps);
		return;
	}

	bool isVendorPolycom = m_pSipCntl->StoreRemoteVendorInfo();
	
	m_pSipCntl->CheckAndStoreRemoteOriginVersionField();
#if 0 //added by Flora Yao
	if(MicrosoftEP_Lync_CCS == m_pSipCntl->GetRemoteIdent())
	{	
		PTRACE(eLevelInfoNormal,"CSipPartyInCreate::OnLobbyIdentIdle - CCS: remove LPR from remote Caps!!!");
		CCapSetInfo capInfoLpr (eLPRCapCode);
		pRemoteCaps->RemoveCapSet (capInfoLpr, kRolePresentation);
	}
#endif
	m_pSipCntl->SetLastRemoteCaps(*pRemoteCaps);
	
    //---TCP Keep-Alive -----//
    m_pSipCntl->vStoreTcpAliveParameters();
    //-----------------------//

	//LyncCCS
	BYTE	isCCSPlugin   = (MicrosoftEP_Lync_CCS == m_pSipCntl->GetRemoteIdent());
	m_pSipCntl->SetIsCCSPlugin(isCCSPlugin);
	
	if (!pConfParty)
	{
		PTRACE(eLevelError, "CSipPartyInCreate::OnLobbyIdentIdle - pConfParty is NULL");
		POBJDELETE(pNetSetup);
		PDELETEA(pSdpAndHeaders);
		POBJDELETE(pRemoteCaps);
		DBGPASSERT(1128);
		return;
	}
	if(Polycom_Lync_CCS_Gw == m_pSipCntl->GetRemoteIdent())
	{
		if(NO == pCommConf->IncludeRDPGw())
		{
			m_pSipCntl->SetRemoteIdent(MicrosoftEP_Lync_CCS); // reuse the CSS logic!
			pConfParty->SetRsrvPartyIsRdpGw(TRUE);
			m_pSipCntl->SetIsCCSPlugin(TRUE);
			
		}
		else
		{
			//reject the call
			m_pConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()),NULL,NULL,1);
			m_pConfApi->DropParty(pConfParty->GetName(), 0, SIP_FORBIDDEN);
			m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
			::AllocateRejectID(m_DialInRejectConnectionId);
			m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
			m_eDialState = kRejectedByLobby;
			m_state = PARTYDISCONNECTING;
			m_eLobbyRejectReason = SipCodesBusyHere;
			CleanUp();
			POBJDELETE(pNetSetup);
			PDELETEA(pSdpAndHeaders);
			POBJDELETE(pRemoteCaps);
			return;
		}
	}
	pConfParty->SetRemoteIdent(m_pSipCntl->GetRemoteIdent());
	pConfParty->SetPlcmRequireMask(m_pSipCntl->GetPlcmRequireMask());
	pConfParty->SetIsCiscoTagExist(isCiscoTagExist);

	eVideoPartyType RemoteVideoPartyType = eVideo_party_type_none;
	RemoteVideoPartyType = m_pSipCntl->GetMaxRemoteVideoPartyType(pRemoteCaps);
	pNetSetup->SetRemoteVideoPartyType(RemoteVideoPartyType);
	m_pSipCntl->SetRemoteTypeOnNetSetup(RemoteVideoPartyType);
	PTRACE2INT(eLevelInfoNormal, "CSipPartyInCreate::OnLobbyIdentIdle: the remote video type is  ", (WORD)RemoteVideoPartyType);

	// for Call Generator - Vendor detection
	if ((CProcessBase::GetProcess()->GetProductFamily() == eProductFamilyCallGenerator) &&
		::GetVendorDetection() && !isVendorPolycom)
	{
		PASSERTMSG(m_pSipCntl->GetCallObj()->GetCallIndex(),"CSipPartyIn::OnLobbyIdentIdle -  Call Generator - Not a Polycom manufacturer");
		POBJDELETE(pNetSetup);
		PDELETEA(pSdpAndHeaders);
		POBJDELETE(pRemoteCaps);
		return;// to avoid the rest of the actions still in the function
	}

        
	if(Microsoft_AV_MCU == m_pSipCntl->GetRemoteIdent() || pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone)
	{
		//need to check if there is already AVMCU in this conf -> if so reject the call
		CConfParty* pTempConfParty = pCommConf->GetFirstParty();
		bool found_avmcu = false;
	    PTRACE(eLevelInfoNormal, "CSipPartyInCreate::OnLobbyIdentIdle: Checking for AVMCU");
		do
		{
			if (NULL == pTempConfParty)
			{
				break;
			}
			if (pTempConfParty->GetPartyId() != pConfParty->GetPartyId())
			{
				if (Microsoft_AV_MCU == pTempConfParty->GetRemoteIdent() ||  pConfParty->GetMsftAvmcuState() != eMsftAvmcuNone )
				{
				    PTRACE(eLevelInfoNormal, "CSipPartyInCreate::OnLobbyIdentIdle: found another AVMCU party in this conf -> reject the call");
					found_avmcu = true;
				}
			}
			
			pTempConfParty = pCommConf->GetNextParty();
		} while (!found_avmcu);
		
		if (found_avmcu)
		{
			
		    m_pConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()),NULL,NULL,1);
			m_pConfApi->DropParty(pTempConfParty->GetName(), 0, NO_DISCONNECTION_CAUSE);
	    
		    m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
			::AllocateRejectID(m_DialInRejectConnectionId);
			m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
			m_eDialState = kRejectedByLobby;
			m_state = PARTYDISCONNECTING;
			m_eLobbyRejectReason = SipCodesNotAcceptedInHere;
			CleanUp();

			POBJDELETE(pNetSetup);
			PDELETEA(pSdpAndHeaders);
			POBJDELETE(pRemoteCaps);
			return;// to avoid the rest of the actions still in the function
		}
	}

	if((m_pSipCntl->GetRemoteIdent() == CiscoCucm) || pConfParty->GetIsTipCall() || isCiscoTagExist || (m_pSipCntl->GetRemoteIdent() == MicrosoftMediationServer) ||
	   (!isCCSPlugin && (plcmReqMask & (m_plcmRequireVideoSlides | m_plcmRequireBfcpTcp | m_plcmRequireBfcpUdp))))
	{
		pNetSetup->SetEnableSipICE(FALSE);
		m_pSipCntl->SetIceCallOnNetSetup(FALSE);

		if (m_pSipCntl->GetRemoteIdent() == MicrosoftMediationServer)
			//VNGFE-4207 if the SIP comes from Mediation server don't go through ICE.
			PTRACE(eLevelInfoNormal, "CSipPartyInCreate::OnLobbyIdentIdle: call is coming from MediationServer set no ICE");
	}

    m_pSipCntl->SetIsEnableICE(pNetSetup->GetEnableSipICE());
	m_pSipCntl->SetITPRtcpMask();
	PTRACE2INT(eLevelInfoNormal,"CSipPartyInCreate::OnLobbyIdentIdle: ICE - ",pNetSetup->GetEnableSipICE());
	m_pLobbyApi->PartyIdent(this, INITIAL, statOK);  // always initial channel

	m_eResponsibility = kLobby;
	m_eDialState = kBeforeOk;
	POBJDELETE(pNetSetup);
	PDELETEA(pSdpAndHeaders);
	POBJDELETE(pRemoteCaps);
}


////////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnLobbyRejectIdle(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnLobbyRejectIdle: Name ",m_partyConfName);
	CSipNetSetup *		pNetSetup		= new CSipNetSetup;
	DWORD	reason = 0xFFFFFFFF;
	WORD	sdpLen=0, addressLen=0;
	STATUS  status;
	sipSdpAndHeaders*	pSdpAndHeaders	= NULL;

	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);

	pNetSetup->DeSerialize(NATIVE,*pParam);
	*pParam >> reason;
	*pParam >> status;
	*pParam >> sdpLen;
	pSdpAndHeaders = (sipSdpAndHeaders *) new BYTE[sdpLen];
	pParam->Get((BYTE*)pSdpAndHeaders,sdpLen);
	*pParam >> addressLen;
	if(addressLen)
	{
		if(m_alternativeAddrStr)
			PDELETEA(m_alternativeAddrStr);
		m_alternativeAddrStr = new char[addressLen+1];
		*pParam >> m_alternativeAddrStr;
	}
	PTRACE2INT(eLevelInfoNormal,"CSipPartyInCreate::OnLobbyRejectIdle: Reject reason ",reason);
	m_pSipCntl->Create(this, pNetSetup, NO, m_serviceId, 0xFFFFFFFF);
	m_pSipCntl->SetCallLegAndCdrHeaders(*pSdpAndHeaders);
//	::AllocateRejectID(m_DialInRejectConnectionId);
//	m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
	m_pSipCntl->SetRemoteSdp(*pSdpAndHeaders);

    //BRIDGE-777: Need Remote Vendor Type to identify Lync EP
    m_pSipCntl->SetRemoteHeaders(pSdpAndHeaders);
    bool isVendorPolycom = m_pSipCntl->StoreRemoteVendorInfo();


	//BRIDGE-6681 - add a warning header if the encryption definitions are not compatible
	if (status ==  PARTICIPANT_ENCRYPTION_SETTINGS_DO_NOT_MATCH_THE_CONFERENCE_SETTINGS)
	{
		const char *pRemoteSipContact = (m_pSipCntl->GetNetSetup()) ? m_pSipCntl->GetNetSetup()->GetRemoteSipContact() : 0;

		if (pRemoteSipContact && strstr(pRemoteSipContact, ".DMA_VMR.")) //Send the customized warning headers just in cases when the SIP INVITE is coming from a DMA
		{
			const char* warningStr = "[10401] Un-encrypted endpoint cannot join or launch encrypted conference.";

			m_pSipCntl->GetCallObj()->SetWarning(SipWarningMiscellaneous);
			m_pSipCntl->GetCallObj()->SetWarningString(warningStr);
		}
	}


	m_eDialState = kRejectedByLobby;
	switch(reason)
	{
	case cmReasonTypeCallForwarded:
		m_eLobbyRejectReason = SipCodesMovedTemp;
		break;

	case cmReasonTypeNoPermision:
        if( status ==  PARTICIPANT_ENCRYPTION_SETTINGS_DO_NOT_MATCH_THE_CONFERENCE_SETTINGS && m_pSipCntl->IsRemoteMicrosoft () )
        {
            m_eLobbyRejectReason = SipCodesNotAcceptedInHere; //488 Added for BRIDGE-777. Microsoft wants 488 for encryption levels don't match
        }
        else
        {
			m_eLobbyRejectReason = SipCodesForbidden;
        }

		break;

	case cmReasonTypeNoBandwidth:
		m_eLobbyRejectReason = SipCodesBusyHere;
		break;

	case SipCodesNotAcceptedInHere:
		m_eLobbyRejectReason = SipCodesNotAcceptedInHere;
		break;

	default:
		DBGPASSERT(reason);
		m_eLobbyRejectReason = SipCodesInternalSrvErr;
		break;
	}

	POBJDELETE(pNetSetup);
	PDELETEA(pSdpAndHeaders);
}


////////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnLobbyTransferIdle(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal, "CSipPartyInCreate::OnLobbyTransferIdle: Name ", m_partyConfName);
	CSegment rspMsg;
	WORD mode = 0xFFFF;
	WORD  returnSyncStatus = 0;
	*pParam >> mode;
	if(m_pConfApi)
		m_pConfApi->DestroyOnlyApi();
	POBJDELETE(m_pConfApi);
	m_pConfApi = new CConfApi(m_monitorConfId);
	m_pConfApi->CreateOnlyApi(*m_pConfRcvMbx,NULL,NULL,1);

	if (mode == PARTYTRANSFER)
	{
		CSipNetSetup* pNetSetup = new CSipNetSetup;
		*pNetSetup = *m_pSipCntl->GetNetSetup();
		 eVideoPartyType RemoteVideoPartyType = eVideo_party_type_none;
		 CSipCaps* pRemoteCaps = const_cast<CSipCaps*>(m_pSipCntl->GetLastRemoteCaps());

		//N.A. DEBUG VP8
		CSuperLargeString strCaps1;
		pRemoteCaps->DumpToString(strCaps1);
		PTRACE2(eLevelInfoNormal, "N.A. DEBUG pRemoteCaps->Dump", strCaps1.GetString());

		BYTE IsOfferer = ((m_pSipCntl->GetRemoteSdp())->sipHeadersOffset) ? FALSE : TRUE;
		
        BYTE bIsMrcHeader = ::IsMrcHeader(m_pSipCntl->GetRemoteSdp());
		PTRACE2INT(eLevelInfoNormal, "CSipPartyInCreate::OnLobbyTransferIdle: bIsMrcHeader=", bIsMrcHeader);

        BYTE bIsWebRtcCall = m_pSipCntl->IsWebRtcCntl();
		PTRACE2INT(eLevelInfoNormal, "CSipPartyInCreate::OnLobbyTransferIdle: bIsWebRtcCall=", bIsWebRtcCall);

		//eFeatureRssDialin
		enSrsVideoLayoutType iniVideolayout  =	m_pSipCntl->GetInitVideoLayoutForRL(m_pSipCntl->GetRemoteSdp());

		RemoteIdent epType = Regular;
		epType = m_pSipCntl->GetRemoteIdent();
		


		PTRACE2INT(eLevelError, "CSipPartyInCreate::OnLobbyTransferIdle: epType=", (DWORD)epType);

		eIsUseOperationPointsPreset isUseOperationPointesPresets = IsUseOperationPointesPresets();
		m_pSipCntl->FillRemoteInformationHeader();
		TRACEINTO << "---cascade --- remoteInfoHeader: " <<  m_pSipCntl->GetSdpRemoteSessionInformation();

		BYTE bIsRemoteSlave = m_pSipCntl->IsRemoteSlave();

		TRACEINTO << "epType: " << epType << ", isUseOperationPointesPresets: "  << (isUseOperationPointesPresets ? "TRUE" : "FALSE")
				  << ", bIsRemoteSlave: " << (bIsRemoteSlave ? "TRUE" : "FALSE");


		LyncConnType lyncEpType = No_Lync;
		if (MicrosoftEP_Lync_CCS == m_pSipCntl->GetRemoteIdent())
			lyncEpType = Lync_Addon;
		else if (MicrosoftEP_Lync_R1  == m_pSipCntl->GetRemoteIdent() )
			lyncEpType = Lync;
		else if( m_pSipCntl->GetRemoteIdent() == MicrosoftEP_Lync_2013)
		{
			lyncEpType = Lync;
			BOOL ignoreIceForLync2013 = FALSE;

			BOOL isDmaCall = GetIsCallWithDma(m_pSipCntl->GetRemoteSdp());
			std::string key1 = "MS_DEBUG_CONNECT_WITHOUT_DMA_AS_WITH";
		    BOOL bTreatnonDmaAsDmaCall = NO;
		    CSysConfig* sysConfig = CProcessBase::GetProcess()->GetSysConfig();
			if (sysConfig)
			 {
				sysConfig->GetBOOLDataByKey(key1, bTreatnonDmaAsDmaCall);
				sysConfig->GetBOOLDataByKey(CFG_KEY_IGNORE_ICE_FOR_LYNC2013, ignoreIceForLync2013);
			  }

			if(isMsftSvc2013Supported() && ( isDmaCall || bTreatnonDmaAsDmaCall ) && (ignoreIceForLync2013 || m_pSipCntl->GetIsEnableICE()))
			{
				lyncEpType = Lync2013;
				PTRACE(eLevelInfoNormal, "CSipPartyInCreate::OnLobbyTransferIdle: MS 2013- this will work only for dial in! ");
				m_pSipCntl->SetIsMs2013(eMsft2013LyncClient);
			}
		}

		PTRACE2INT(eLevelError, "CSipPartyInCreate::OnLobbyTransferIdle: lyncEpType=", (DWORD)lyncEpType);
		 // sync call to conf
		WORD res =m_pConfApi->AddInSipParty( pNetSetup, pRemoteCaps, this, *m_pRcvMbx, m_name,
											  PARTY_TRANSFER_TOUT, rspMsg, IsOfferer,
											  bIsMrcHeader, bIsWebRtcCall, lyncEpType,epType, isUseOperationPointesPresets, bIsRemoteSlave, (BYTE)iniVideolayout);
		POBJDELETE(pNetSetup);

		if (res == 0)
		{
            rspMsg >> returnSyncStatus;
		}

		if (res)
		{
			PTRACE2(eLevelError, "CSipPartyInCreate::OnLobbyTransferIdle: Timer expired on transfer party to conf", m_partyConfName);
			m_pLobbyApi->PartyTransfer(this, statTout);
			m_eDialState = kTransferFailed;
			m_state = PARTYDISCONNECTING;
			if (IsValidTimer(PARTYCONTOUT))
				DeleteTimer(PARTYCONTOUT);
		}
		else if (returnSyncStatus)
		{
			PTRACE(eLevelError,"CSipPartyInCreate::OnLobbyTransferIdle : \'EXPORT PARTY FAILED (Conf Reject) !!!\'");
			m_pLobbyApi->PartyTransfer(this, returnSyncStatus);
			m_eDialState = kTransferFailed;
			m_state = PARTYDISCONNECTING;
			if (IsValidTimer(PARTYCONTOUT))
				DeleteTimer(PARTYCONTOUT);
		}
		else
		{
			m_state = sPARTY_WAITFORCONFINITCALL;
			PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnLobbyTransferIdle: Transfer party to conf ok - ", m_partyConfName);
			m_pLobbyApi->PartyTransfer(this, statOK);
			m_eDialState = kBeforeOkInConf;
			m_eResponsibility = kConf;
			// site names current only for dial in
			m_pSipCntl->FindSiteAndVisualNamePlusProductIdAndSendToConfLevel();
		}
	}
	else
	{
		DBGPASSERT(mode ? mode : YES);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
eIsUseOperationPointsPreset CSipPartyInCreate::IsUseOperationPointesPresets()
{
	eIsUseOperationPointsPreset retVal = eIsUseOPP_No;

	RemoteIdent	theRemoteIdent = m_pSipCntl->GetRemoteIdent();
	bool bIsWithSdp = ( (m_pSipCntl->GetRemoteSdp())->sipHeadersOffset ) ? true : false;

	if (m_pSipCntl->IsSameTimeEP())
	{
		retVal = eIsUseOPP_Yes_SameTime;
	}
	// BRIDGE-6844
	else if ( (Regular == theRemoteIdent) &&  (false == bIsWithSdp) ) // at this stage we still do not know if it's offerless (no sdp)
	{
		retVal = eIsUseOPP_Yes_Unrecognized;
	}

	TRACEINTO << "theRemoteIdent: " << theRemoteIdent
			  << ", bIsWithSdp: "  << (bIsWithSdp ? "true" : "false")
			  << "\nretVal (isUseOperationPointesPresets): "  << retVal
			  << " [No=0, Yes_SameTime=1, Yes_Unrecognized=3]";

	return retVal;
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnConfAllocateResourcesIdle(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnConfAllocateResourcesIdle: Name ",m_partyConfName);
	CRsrcParams* pCsRsrcParams = new CRsrcParams;
	pCsRsrcParams->DeSerialize(NATIVE,*pParam);
	PTRACE2INT(eLevelInfoNormal,"CSipPartyInCreate::OnConfAllocateResourcesIdle - BRIDGE-12931 - resource params initiated with connectionId - ", pCsRsrcParams->GetConnectionId());
	m_pSipCntl->SetRsrcParams(pCsRsrcParams);
	m_bSetRsrcParam = TRUE;

	// send Ringing
	m_pSipCntl->SipRingingReq();
	POBJDELETE(pCsRsrcParams);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnConfEstablishCallIdle(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnConfEstablishCallIdle - Name = ", m_partyConfName);

	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(GetMonitorConfId());
	CConfParty* pConfParty = NULL;

	if (pCommConf)
		pConfParty = pCommConf->GetCurrentParty(GetMonitorPartyId());

	if(!pConfParty)
	{
		DBGPASSERT_AND_RETURN(!pConfParty);
	}
	CSipNetSetup netSetup;
	CSipCaps localCaps;

	netSetup.DeSerialize(NATIVE, *pParam);
	TRACEINTO << "IP VERSION:" << netSetup.GetIpVersion();  // =1 -from m_pSIpNetSetup !!!!!!!!!
	localCaps.DeSerialize(NATIVE, *pParam);
	m_pTargetMode->DeSerialize(NATIVE, *pParam);

	//============================
	// Preparing qos information
	//============================
	BOOL requireHeaderFailure;
	CQoS qos;  
	qos.DeSerialize(NATIVE, *pParam);
	ParseIncomingPrecedenceInfo(m_pSipCntl->GetRemoteCallLegHeaders(), qos, &requireHeaderFailure);
	if (requireHeaderFailure)
	{
		//====================================================================================
		// Received unknown precedence however remote REQUIRED this feature.  Disconnecting.
		//====================================================================================
		PTRACE(eLevelError, "CSipPartyInCreate::OnConfEstablishCallIdle - could not match precedence level for a REQUIRED r-priority.  Disconnecting");
		m_pConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()),NULL,NULL,1);
		m_pConfApi->DropParty(pConfParty->GetName(), 0, SIP_CLIENT_ERROR_400);
		m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
		::AllocateRejectID(m_DialInRejectConnectionId);
		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
		m_eDialState = kRejectedByLobby;
		m_state = PARTYDISCONNECTING;
		m_eLobbyRejectReason = static_cast<enSipCodes>(417);
		CleanUp();
	}

	UdpAddresses sUdpAddressesParams;
	DWORD confParamLen = 0;
	char* strConfInfo = NULL;
	BYTE eTransportType = 0;
	BOOL IsIceEnable = FALSE;

	*pParam >> m_bIsAdvancedVideoFeatures;

	*pParam >> confParamLen;
	if (confParamLen)
	{
		strConfInfo = new char[confParamLen];
		pParam->Get((BYTE*)strConfInfo, confParamLen);
		strConfInfo[confParamLen - 1] = 0;
	}

	m_pSipCntl->SetMaxRate(netSetup.GetMaxRate());
	*pParam >> eTransportType; // not used in dial in
	APIU16 plcmRequireMask = pConfParty->GetPlcmRequireMask(); //N.A.BRIDGE-11697
	m_pSipCntl->SetLocalCaps(localCaps,plcmRequireMask);
	m_pSipCntl->SetFullContentRate(localCaps.GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation));//***ppc can be removed when dpa implemented.

	m_pSipCntl->SetQos(qos);
	m_pSipCntl->SetConfParamInfo(strConfInfo);
	PDELETEA(strConfInfo);
	m_pSipCntl->SetConfMediaType(m_pTargetMode->GetConfMediaType());

	BYTE bIsMrmpExists = FALSE;
	CRsrcParams* pMrmpRsrcParams = NULL;
	CRsrcParams mfaRsrcParams;
	CRsrcParams csRsrcParams;

	mfaRsrcParams.DeSerialize(NATIVE, *pParam);
	csRsrcParams.DeSerialize(NATIVE, *pParam);

	DeSerializeNonMandatoryRsrcParams(pParam, pMrmpRsrcParams);
	int avcToSvcTranslatorCnt = 0;
	BYTE bIsTranslatorExists;
	CRsrcParams* avcToSvcTranslatorRsrcParams[NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS];
	for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		DeSerializeNonMandatoryRsrcParams(pParam, avcToSvcTranslatorRsrcParams[i]);
		if (avcToSvcTranslatorRsrcParams[i])
			avcToSvcTranslatorCnt++;
	}

	pParam->Get((BYTE *)&sUdpAddressesParams, sizeof(UdpAddresses));

	*pParam >> m_mcuNum;
	*pParam >> m_termNum;

	BYTE bIsCopVideoTxModes = FALSE;
	*pParam >> bIsCopVideoTxModes;
	if (bIsCopVideoTxModes)
	{
		CCopVideoTxModes tempCopVideoTxModes;
		tempCopVideoTxModes.DeSerialize(NATIVE, *pParam);
		m_pSipCntl->SetCopVideoTxModes(&tempCopVideoTxModes);
	}

	DWORD addrLen = 0;
	*pParam >> addrLen;
	*pParam >> m_bNoVideRsrcForVideoParty;

	if (m_bNoVideRsrcForVideoParty && m_ivrCtrl)
	{
		m_ivrCtrl->setNoVideRsrcForVideoParty(m_bNoVideRsrcForVideoParty);
	}

	m_PartyRsrcID = csRsrcParams.GetPartyRsrcId();
	m_ConfRsrcId = csRsrcParams.GetConfRsrcId();

	if (m_pTargetMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople))
	{
		TRACEINTO<<"!@# m_pTargetMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople) - media is off";
	}
	else
	{
		TRACEINTO<<"!@# m_pTargetMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople) - media is on";
	}

	if (m_pTargetMode->IsMediaOff(cmCapVideo, cmCapReceive, kRolePeople))
	{
		TRACEINTO<<"!@# m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceive, kRolePeople) - media is off";
	}
	else
	{
		TRACEINTO<<"!@# m_pTargetMode->IsMediaOff(cmCapVideo,cmCapReceive, kRolePeople) - media is on";
	}

	if (m_pTargetMode->IsMediaOff(cmCapVideo, cmCapTransmit, kRolePeople) &&  
		m_pTargetMode->IsMediaOff(cmCapVideo, cmCapReceive, kRolePeople))
	{
		TRACEINTO<<"dynMixedPosAck - audio only call: deleting translator resource";	 
	}

	m_pSipCntl->SetControllerResource(&mfaRsrcParams, &csRsrcParams,sUdpAddressesParams);
	m_pSipCntl->SetInternalControllerResource(avcToSvcTranslatorRsrcParams, pMrmpRsrcParams);
	POBJDELETE(pMrmpRsrcParams);
	for (int i=0;i<NUMBER_OF_INTERNAL_AVC_TO_SVC_TRANSLATORS;++i)
	{
		POBJDELETE(avcToSvcTranslatorRsrcParams[i]);
	}

	m_pSipCntl->AddToRoutingTable();

	m_pTargetModeMaxAllocation->DeSerialize(NATIVE, *pParam); //dpa
	CSipCaps maxLocalCaps;
	maxLocalCaps.DeSerialize(NATIVE, *pParam);
	m_pSipCntl->SetMaxLocalCaps(maxLocalCaps);
	*pParam >> m_RoomId;

	if (m_pTargetMode->GetConfMediaType() == eMixAvcSvc || m_pTargetMode->GetConfMediaType()==eMixAvcSvcVsw)
	{
		m_SsrcIdsForAvcParty.DeSerialize(*pParam);
		m_SsrcIdsForAvcParty.Print("avc_vsw_relay: CSipPartyInCreate::OnConfEstablishCallIdle GetConfMediaType()==eMixAvcSvcVsw || GetConfMediaType() == eMixAvcSvc");
	}

	BYTE bIsMrcCall = FALSE;
	*pParam >> bIsMrcCall;

	BYTE bIsASSIPContentEnable;
	*pParam >> bIsASSIPContentEnable;

	m_pSipCntl->SetASSIPContent(bIsASSIPContentEnable);

  DWORD partyContentRate;
  *pParam >> partyContentRate;
   m_PartyContentRate = partyContentRate;
	// Yiping - for BRIDGE-892
	// The value IsMrcCall is set by the following procedure:
	// Step 1: CSipPartyInCreate::OnLobbyTransferIdle() : find the Mrd header, and tell Conf via SIPADDINPARTY
	// Step 2: Conf check the conference profiles and decide the actuall IsMrcCall. (CConf::OnSipLobbyAddPartyConnect())
	// Step 3: Conf tell party the actual type: CSipAddpartyCntl::EstablishCall(), via SIP_CONF_ESTABLISH_CALL

	m_pSipCntl->SetIsMrcCall(bIsMrcCall);
	TRACEINTOFUNC << "bIsMrcCall: " << (bIsMrcCall? "yes" : "no");

	BOOL bMsEnviroment = FALSE;
	CConfIpParameters* pService = ::GetIpServiceListMngr()->FindIpService(m_serviceId);
	if( pService != NULL && pService->GetConfigurationOfSipServers() )
	{
		if(pService->GetSipServerType() == eSipServer_ms)
			bMsEnviroment = TRUE;
	}

	if(bMsEnviroment && bIsMrcCall)
	{
		m_pSipCntl->SetIceCallOnNetSetup(FALSE);
		m_pSipCntl->SetIsEnableICE(FALSE);
	}

	if (bIsMrcCall)
	{// update MRE version
		m_pSipCntl->FillMrdVersion();
	}

	m_pSipCntl->SetTipRoomId(m_RoomId);
	m_pSipCntl->SetLocalSdesKeysAndTag(m_pTargetMode, m_pTargetModeMaxAllocation);

	m_pSipCntl->DecideAnatSelectedIpVersion(m_pSipCntl->GetRemoteSdp()); //Added for ANAT

	m_pSipCntl->CreateSipBfcpCtrl();
	m_pSipCntl->SetBFCPcapConfIDfield(m_mcuNum);
	m_pSipCntl->SetBFCPcapUserIDfield(m_termNum);

	const char* pDtmfForwardSource = m_pSipCntl->GetDtmfForwardSource();
	if (pDtmfForwardSource && (strstr(pDtmfForwardSource, "chairperson")))
	{
		if (m_pSipCntl->IsSameTimeEP())
			TRACEINTO << "Don't forward dtmf to sametime";
		else if (m_pSipCntl->GetIsMrcCall())
			TRACEINTO << "Don't forward dtmf to mrc call";
		else
		{
			TRACEINTO << "SetReceiveDtmfFromChairperson TRUE";
			if (pConfParty)
			{
				pConfParty->SetReceiveDtmfFromChairperson(TRUE);
			}
		}
	}

	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);

	if (m_pSipCntl->GetRemoteIdent() == PolycomRMX)
	{
		PTRACE(eLevelInfoNormal,"CSipPartyInCreate::OnConfEstablishCallIdle - Remote identified as Polycom RMX, so remove FECC cap");
		localCaps.CleanMedia(cmCapData);
		m_pTargetMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit);
	}

	if (m_pTargetMode->GetConfMediaType() == eMixAvcSvc && !bIsMrcCall && avcToSvcTranslatorCnt>0)
	{
		m_state = sPARTY_ALLOCATE_TRANSLATOR_ARTS;
		m_pSipCntl->OpenInternalArts(E_NETWORK_TYPE_IP,avcToSvcTranslatorCnt);

		return;
	}

	// IF YOU NEED TO ADD MORE LOGIC, ADD IT INSIDE THIS FUNCTION!!!
	ContinueEstablishCall(NULL);
	if(pConfParty)
	{
		//Send Info to CDR for SIP dial in
		pConfParty->SetCorrelationId(std::string(netSetup.GetCallId()));
		pCommConf->PartyCorrelationDataToCDR(pConfParty->GetName(), m_pParty->GetMonitorPartyId(), pConfParty->GetCorrelationId());
		PlcmCdrEventCallStartExtendedHelper cdrEventCallStartExtendedHelper;
		cdrEventCallStartExtendedHelper.SetNewIsdnUndefinedParty_BasicAndContinue(*pConfParty, SIP_INTERFACE_TYPE, *pCommConf);
		pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&cdrEventCallStartExtendedHelper.GetCdrObject(), false, cdrEventCallStartExtendedHelper.GetCdrObject().m_partyDetails.m_id);

		//send visual name
		pCommConf->OperatorSetVisualName(pConfParty->GetVisualPartyName(), pConfParty->GetPartyId(), pConfParty->GetName(), pConfParty->GetCorrelationId());

	  //send event to cdr only here because only here we get the correlationID
	  PlcmCdrEventConfUserDataUpdate ConfUserDataUpdateEvent;
	  ConfUserDataUpdateEvent.m_userDefinedInformation.m_contactInfoList.m_contactInfo = pConfParty->GetUserDefinedInfo(0);
	  ConfUserDataUpdateEvent.m_userDefinedInformation.m_contactInfoList.m_contactInfo2 = pConfParty->GetUserDefinedInfo(1);
	  ConfUserDataUpdateEvent.m_userDefinedInformation.m_contactInfoList.m_contactInfo3 = pConfParty->GetUserDefinedInfo(2);
	  ConfUserDataUpdateEvent.m_userDefinedInformation.m_contactInfoList.m_contactInfo4 = pConfParty->GetUserDefinedInfo(3);
	  ConfUserDataUpdateEvent.m_userDefinedInformation.m_vip = false;
	  pCommConf->SendCdrEvendToCdrManager((ApiBaseObjectPtr)&ConfUserDataUpdateEvent, false, pConfParty->GetPartyId());
	}
}

void CSipPartyInCreate::OnPartyTranslatorArtsConnected()
{
	ContinueEstablishCall(NULL);
}

void CSipPartyInCreate::ContinueEstablishCall(CSegment* pParam)
{
	m_state = sPARTY_CONNECTING;
	ESipTransactionType eSipTransType = kSipTransNone;
	bool bIsWithSdp = ( (m_pSipCntl->GetRemoteSdp())->sipHeadersOffset ) ? true : false;
	if ( !m_pSipCntl->GetIsMrcCall() )
	{
		if (m_pSipCntl->IsWebRtcCntl() && bIsWithSdp)
			eSipTransType = kSipTransInviteWebRtcWithSdpInd;
		else
			eSipTransType = ( bIsWithSdp ? kSipTransInviteWithSdpInd : kSipTransInviteNoSdpInd );
	}
	else // bIsMrcCall
	{
		eSipTransType = ( bIsWithSdp ? kSipTransInviteMrcWithSdpInd : kSipTransInviteMrcNoSdpInd );
	}

	StartTransaction(eSipTransType, SIP_PARTY_ESTABLISH_CALL,pParam);
        POBJDELETE(pParam);
}


////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::DestroyPartyTask()
{
	TRACEINTO << PARTYNAME << ", PartyId:" << GetPartyId();

	m_pSipCntl->Suspend(); // Ignore all events
	if (m_eResponsibility == kConf)
		m_pConfApi->PartyEndDisConnect(GetPartyId(), statOK);
	else if (m_eResponsibility == kLobby) // in the new flow any reason that makes the dial in party to response with reject initiate the reject
	{ // process from the lobby. And the party is not send End disconnect to the Lobby.
		int i = 0; // do nothing!
	}
	else
		DBGPASSERT(YES);

	PartySelfKill();
}

////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnPartyBadStatusConnecting(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnPartyBadStatus: Name ",m_partyConfName);
	DWORD opcode;
	DWORD len;
	char* strDescription;
	*pParam >> opcode;
	*pParam >> len;
	strDescription = new char[len+1];
	pParam->Get((BYTE*)strDescription,len);
	strDescription[len] = 0;
	CSmallString str;
	str << "Bad status opcode " << opcode << ", description: " <<strDescription;
	PTRACE2(eLevelError,"CSipPartyInCreate::OnPartyBadStatus ",str.GetString());

	PDELETEA(strDescription);

	if (opcode == SIP_CS_SIG_INVITE_RESPONSE_REQ)
	{
		if (m_eDialState != kOkSent)
			DBGPASSERT(m_eDialState);
		m_eDialState = kBadStatusArrivedAfterOk;
	}

	if (m_eDialState != kBeforeOkInConf)
		m_eDialState = kBadStatusArrived;

	TellConfOnDisconnecting(SIP_BAD_STATUS);
}

////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnTransCheckCompleteInd(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnTransCheckCompleteInd: Name ",m_partyConfName);
}

void CSipPartyInCreate::OnIceInviteReceiveMakeOfferInd(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnIceInviteReceiveMakeOfferInd: Name ",m_partyConfName);
}

////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnConfCloseCall(CSegment * pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnConfCloseCall: Name ",m_partyConfName);
	m_state = PARTYDISCONNECTING;
	LogicalChannelDisconnect(SDP);

	if (IsValidTimer(PARTYCONTOUT))
		DeleteTimer(PARTYCONTOUT);
/*
	if ((m_eResponsibility == kLobby) && (m_DialInRejectConnectionId == 0xFFFFFFFF))
	{// for reject dial in calls from the lobby due to lobby internal process error.
		::AllocateRejectID(m_DialInRejectConnectionId);
		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
	}
*/

	if ((m_eDialState == kBeforeOk) && (m_eResponsibility == kLobby))
		m_eDialState = kFailedInLobby;
	else if (m_eDialState == kBeforeOk)
	{// for reject dial in calls from the conf due to conf internal problem with creating the party.
		m_eDialState = kFailedInConf;
	}
	else if (m_eDialState == kBeforeOkInConf)
		m_eDialState = kTerminateByConf;

	// before cleanup connection id must be set
	if (m_DialInRejectConnectionId == 0xFFFFFFFF && m_bSetRsrcParam == FALSE)
	{
		::AllocateRejectID(m_DialInRejectConnectionId);
		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
	}

	if(m_tipPartyType == eTipMasterCenter)
	{
		/*
		SendMessageFromMasterToSlave(eTipSlaveAux, PARTYDISCONNECT, NULL);

	    WORD numOfSlaves = m_TipNumOfStreams -1 + m_bIsAudioAux;
		if(numOfSlaves > 1)
		{
			SendMessageFromMasterToSlave(eTipSlaveLeft, PARTYDISCONNECT, NULL);
			SendMessageFromMasterToSlave(eTipSlaveRigth, PARTYDISCONNECT, NULL);
		}

		//CDR_MCCF:
		BOOL bEnableCdrMCCF = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_CDR_FOR_MCCF);

		if (bEnableCdrMCCF == TRUE)
		{
			if (GetDisconnectInitiator() == eUnKnown)
				SetDisconnectInitiator(eConf);

			BYTE sendReq = m_pSipCntl->SendRtpVideoChannelStatisticsReq();
			if (sendReq == TRUE)
			{
				PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipPartyInCreate::OnConfCloseCall - send statistic req before close sending IP_MSG_CLOSE_TIP_CALL_REQ");
				m_oldstate = m_state;
				m_state = IP_CM_PARTY_WAITING_FOR_STATISTIC_INFO_IND;
				return;
			}
		}

		PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipPartyInCreate::OnConfCloseCall - flag bEnableCdrMCCF or sendReq is FLASE - do not send statistic");
		m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_CLOSE_TIP_CALL_REQ);
*/
	}

	CleanUp();
}

////////////////////////////////////////////////////////////////////////////
//CDR_MCCF:
void CSipPartyInCreate::ContinueToCloseTIPcallReq()
{
    if ( m_tipPartyType == eTipMasterCenter )
    {
    	PTRACE2INT(eLevelInfoNormal,"CDR_MCCF: CSipPartyInCreate::ContinueToCloseTIPcallReq - eTipMasterCenter GetDisconnectInitiator()=", GetDisconnectInitiator());

        m_state = m_oldstate;
        m_pSipCntl->SendTipCallMessageToMPL(m_PartyRsrcID, m_SlaveLeftRsrcId, m_SlaveRightRsrcId, m_SlaveAuxRsrcId, IP_MSG_CLOSE_TIP_CALL_REQ);

        if ( GetDisconnectInitiator() == eConf)
        	CleanUp();

    }
    else
        PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipPartyInCreate::ContinueToCloseTIPcallReq");
}

////////////////////////////////////////////////////////////////////////////
//CDR_MCCF:
void CSipPartyInCreate::forDailInSendStatisticsInfoOfThisEP()
{
	PTRACE(eLevelInfoNormal,"CDR_MCCF: CSipPartyInCreate::forDailInSendStatisticsInfoOfThisEP - Master or None - send the info");

	//send to MCMS Polycom mixer
	m_pSipCntl->SendStatisticsInfoOfThisEpToMCMSpolycomMixer();
}

////////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::CleanUp()
{
	BOOL isPortGaugeFlagOn = NO;
	DWORD sipBusyResponceValue = 0;
	std::string key = "SEND_SIP_BUSY_UPONRESOURCE_THRESHOLD";

	CSmallString str;
	str << "Name " << m_partyConfName << " Dial state " << m_eDialState;
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::CleanUp: ", str.GetString());

	EndTransactionByPartyIfNeeded();

	switch(m_eDialState)
	{
	case kRejectedByLobby:
		if (m_alternativeAddrStr)
		{
			m_pSipCntl->RejectCallAndProvideAlternativeAddrToCall(SipCodesMovedTemp, m_alternativeAddrStr);
		}
		else
		{
			m_pSipCntl->RejectCall(m_eLobbyRejectReason);
		}
		break;

	case kFailedInLobby:
	case kFailedInConf:
		m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
		break;

	case kTerminateByConf:
//		CProcessBase::GetProcess()->GetSysConfig()->GetBOOLDataByKey(key, isPortGaugeFlagOn);
	    if( CProcessBase::GetProcess()->GetServiceConfigList() )
		    CProcessBase::GetProcess()->GetServiceConfigList()->GetBOOLDataByKey(m_serviceId, key, isPortGaugeFlagOn);

		CProcessBase::GetProcess()->GetSysConfig()->GetDWORDDataByKey("SIP_BUSY_UPONRESOURCE_ERROR_ID", sipBusyResponceValue);

		if( isPortGaugeFlagOn )
			m_pSipCntl->RejectCall(sipBusyResponceValue);
		else
			m_pSipCntl->RejectCall(SipCodesDecline);
		break;

	case kChannelsFailedToOpen:
	case kTransferFailed:
	m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
		break;
	case kBeforeOkInConf:
	

       {
		if(m_disconnectCause == SIP_INSUFFICIENT_BANDWIDTH)
			m_pSipCntl->RejectCall(SipCodesNotAcceptedInHere,SipWarningInsufficientBandwidth);
		else
			m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
		break;
	}
	case kCapsDontMatch:
		m_pSipCntl->RejectCall(SipCodesNotAcceptedInHere,SipWarningMediaTypeNotAvail); //section 14.2 in standard
		break;

	case kICETimeOut:
		// VNGR-25678 If ICE timeout
		m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
		break;
	case kWebRtcConnectFailure:
	case kWebRtcConnectTimeOut:
		m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
		break;
	case kCancelArrived:
		m_pSipCntl->CloseCall(NO,SipCodesRequestTerminated);
		break;

	case kOkSent: //wait for an ack to arrive (or timer to popped)
	case kReInviteAccepted:
		PTRACE2INT(eLevelInfoNormal,"CSipPartyInCreate::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
		break;

	case kReInviteRejected://wait for an ack to arrive (or timer to popped)
		PTRACE2INT(eLevelInfoNormal,"CSipPartyInCreate::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
		m_eDialState = kTerminateByConf;
		break;

	case kTerminateByRemote:
		m_pSipCntl->CloseCall(NO);
		break;

	case kBadStatusAckArrived:
	case kConnectTimer:
		m_pSipCntl->CloseCall(YES);
		break;

	case kBadStatusArrived:
	case kTransportErrorArrived:
	case kDisconnectTimer:
	case kDisconnectTimerAfterRejectedByLobby:
		m_pSipCntl->ViolentCloseCall();
		break;

	case kNoRecovery:
		if (m_pSipCntl->GetIsMrcCall())
		{
			m_pSipCntl->SipInviteAckReq();	// in MRC, dial-in behaves similarly to dial-out
		}
		m_pSipCntl->CloseCall(YES);
		break;
	case kNoRecoveryForVideo:
	case kGuessSucceeded:
		m_pSipCntl->CloseCall(YES);	// The MCU send 200 OK and receive invite ack (this is dial in, and those states are when the dial in is invite with No SDP). send bye
		break;

	case kBadStatusArrivedAfterOk: // wait for connect timer.
		PTRACE2INT(eLevelInfoNormal,"CSipPartyInCreate::CleanUp: Wait for answer or timer in dial state - ",m_eDialState);
		break;
	case kReInviteSent:
		m_pSipCntl->SipCancelReq();	// send cancel for Re-Invite and proceed with call closure
		m_eDialState = kTerminateByConf;
		m_pSipCntl->CloseCall(YES);
		break;

	case kReInviteArrived:
		m_pSipCntl->SipInviteResponseReq(SipCodesBadRequest); // wait for ack
		break;

	case kNotInDialState:
		m_pSipCntl->CloseCall(YES);
		break;

	case kByeArrived:
		m_pSipCntl->CloseCall(NO);
		break;
	case kInternalProblem:
		PTRACE(eLevelInfoNormal,"CSipPartyInCreate::CleanUp: kInternalProblem ");
		m_pSipCntl->RejectCall(SipCodesInternalSrvErr);
		break;
	case kRejectArrived: // Can be in ReInvite
		m_pSipCntl->CloseCall(NO); // send ack for the reject
		break;

	case kBeforeInvite:
	    if (::IsMrcHeader(m_pSipCntl->GetRemoteSdp()))
	    {
	        PTRACE(eLevelInfoNormal,"CSipPartyInCreate::CleanUp: kInternalProblem - send BYE");
	        m_pSipCntl->CloseCall(YES);
	    }
        PTRACE(eLevelInfoNormal,"CSipPartyInCreate::CleanUp: kInternalProblem - do not send BYE");
		break;

	default:
		DBGPASSERT(m_eDialState);
	}
}

////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::LogicalChannelDisconnect(DWORD eChannelType)
{
	if(m_eResponsibility == kConf)
		CIpParty::LogicalChannelDisconnect(eChannelType);
}

////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnPartyCallFailed(CSegment * pParam)
{
	DWORD reason = 0xFFFFFFFF;
	DWORD MipErrorNumber = 0;
	const char* alternativeAddrStr = NULL;
	*pParam >> reason;
	PTRACE2INT(eLevelInfoNormal,"CSipPartyInCreate::OnPartyCallFailed: Reason - ",reason);

	if (reason >= LOW_REJECT_VAL && reason < HIGH_REJECT_VAL) // can be reject to reinvite req.
	{
		SetDialState(kRejectArrived);
		if (reason == SipCodesMovedPerm || reason == SipCodesMovedTemp) // call forwards
			alternativeAddrStr = m_pSipCntl->GetForwardAddr();
	}
	else if (reason == SIP_CARD_REJECTED_CHANNELS)
	{
		m_eDialState = kChannelsFailedToOpen;
		reason = MCU_INTERNAL_PROBLEM; //change for disconnect cause in GUI
		*pParam >> MipErrorNumber;
	}

	if ( (m_eActiveTransactionType == kSipTransUpgradeAvcOnlyToMixReq) ||(m_eActiveTransactionType == kSipTransUpgradeSvcOnlyToMixReq)  )
	{
          if(m_pSipCntl->GetIsMrcCall())
	  {
	    TRACEINTO<<"!@# dynMixedErr failed while updating channels (svc)";
	  }
	  else
	  {
	    TRACEINTO<<"!@#  dynMixedErr failed while opening either internal channels or internal arts (avc)";
	  }
	m_pSipCntl->ResetReqCounter();
	  //	  EndTransactionByPartyIfNeeded();
	  return;
	}


	TellConfOnDisconnecting(reason,alternativeAddrStr,MipErrorNumber);
}

////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnSipDisconnectSetupIdle(CSegment* pParam)
{
	PASSERTMSG(m_PartyRsrcID,"CSipPartyInCreate::OnSipDisconnectSetupIdle: Disconnect call");
	m_isPreSignalingFlowProb = 1;

	if(m_eDialState == kNotInDialState)// under lobby responsibility
	{
		PTRACE(eLevelInfoNormal,"CSipPartyInCreate::OnSipDisconnectSetupIdle: Not in Dial stateName ");
		DeleteAllTimers();
		m_pSipCntl->Suspend(); //ingore all events
		PartySelfKill();
	}
	else if(m_eDialState == kBeforeOk && m_eResponsibility == kLobby)// under lobby responsibility
	{// under Lobby responsibility, reject the call.
		PTRACE(eLevelInfoNormal,"CSipPartyInCreate::OnSipDisconnectSetupIdle: Before OK ");
		m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
		::AllocateRejectID(m_DialInRejectConnectionId);
		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
		m_eDialState = kFailedInLobby;
		m_state = PARTYDISCONNECTING;
		CleanUp();
	}
	else if(m_eDialState == kBeforeOkInConf && m_eResponsibility == kConf)
	{//failed to response with establish call before timeout
		// Activate "Kill Port"
		PTRACE(eLevelInfoNormal,"CSipPartyInCreate::OnSipDisconnectSetupIdle: conf responsibility ");
		OnPartySendFaultyMfaToPartyCntlAnycase(pParam);
		// In case we already allocated and registered the CS and MFA we need to remove them from the
		// routing table to avoid duplicate party entrence in the routing table
		m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
		::AllocateRejectID(m_DialInRejectConnectionId);
		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
		m_eDialState = kInternalProblem;

		BYTE 	mipHwConn = (BYTE)eMipConnectionProcess;
		BYTE	mipMedia = (BYTE)eMipNoneMedia;
		BYTE	mipDirect = (BYTE)eMipIn;
		BYTE	mipTimerStat = (BYTE)eMipTimer;
		BYTE	mipAction = (BYTE)eMipConnect;
		DWORD MpiErrorNumber = ::CalculateMcuInternalProblemErrorNumber(mipHwConn,mipMedia,mipDirect,mipTimerStat,mipAction);

		TellConfOnDisconnecting(MCU_INTERNAL_PROBLEM,NULL, MpiErrorNumber);
	}
	else
	{//unknown scenario
		PTRACE2INT(eLevelInfoNormal,"CSipPartyInCreate::OnSipDisconnectSetupIdle: Unknown state - ", m_eDialState);
		DeleteAllTimers();
		m_pSipCntl->Suspend(); //ingore all events
		PartySelfKill();
	}
}

////////////////////////////////////////////////////////////////////////
void CSipPartyInCreate::OnPartyCallClosed(CSegment* pParam)
{
	PTRACE2(eLevelInfoNormal,"CSipPartyInCreate::OnPartyCallClosed: Name ",m_partyConfName);

	//CDR_MCCF:
	BOOL bEnableCdrMCCF = GetSystemCfgFlagInt<BOOL>(CFG_KEY_ENABLE_CDR_FOR_MCCF);
	if (bEnableCdrMCCF == TRUE && (m_tipPartyType == eTipMasterCenter || m_tipPartyType == eTipNone) )
	{
		forDailInSendStatisticsInfoOfThisEP();
	}

	// update communication mode
	m_state = IP_DISCONNECTED;
    if (m_eDialState != kRejectedByLobby && 
        m_eDialState != kFailedInLobby &&
        m_eDialState != kTransferFailed &&
	m_eDialState != kDisconnectTimerAfterRejectedByLobby)
	{
	    m_pCurrentMode->DeSerialize(NATIVE,*pParam);	
            //otherwise we don't have a valid confApi
	    LogicalChannelDisconnect(SIGNALING);
            UpdateDbOnChannelsDisconnected();
            StopAllPreviews();
	}
	DestroyPartyTask();
}

////////////////////////////////////
//eFeatureRssDialin
BOOL CSipPartyInCreate::IsRssDialinRejected()
{
	BYTE	needToRejectCall   = false;
	CCommConf* pCommConf = ::GetpConfDB()->GetCurrentConf(m_pParty->GetMonitorConfId());
	CConfParty* pConfParty = NULL;
	if (pCommConf)
	{
		pConfParty = pCommConf->GetCurrentParty(m_pParty->GetMonitorPartyId());
	}
	if(!pCommConf || !pConfParty )
	{
		TRACEINTO << "pCommConf is NULL";
		DBGPASSERT(1127);
		return false;
	}

	//Only for Rss dialin
	if((NO == pConfParty->GetRecordingLinkParty())&&(NO == pConfParty->GetPlaybackLinkParty()))
	{
		return false;
	}

	if((YES == pConfParty->GetRecordingLinkParty())&&(false == pCommConf->GetEnableRecording()))
	{
		TRACEINTO << "reject recording server due to not enable the recording !";
		needToRejectCall = true;
	}
	else	if(strlen (pCommConf->GetEntryPassword()) > 0)
	{
		//if(m_ivrCtrl && (!m_ivrCtrl->GetConfPwdEnabled()))
		TRACEINTO << "reject Capture/Recording/Playback server due to password invalid !";
		needToRejectCall = true; //Reject RSS if there's pwd in Conf currently
	}
	else
	{
		needToRejectCall = false;
	}

	if(true == needToRejectCall)
	{
		m_pConfApi->CreateOnlyApi(*(pCommConf->GetRcvMbx()),NULL,NULL,1);
		m_pConfApi->DropParty(pConfParty->GetName(), 0, SIP_FORBIDDEN);
		m_pSipCntl->SetCallDisconnectedAndRemoveFromRsrcTbl();
		::AllocateRejectID(m_DialInRejectConnectionId);
		m_pSipCntl->SetConnectionId(m_DialInRejectConnectionId);
		m_eDialState = kRejectedByLobby;
		m_state = PARTYDISCONNECTING;
		m_eLobbyRejectReason = SipCodesForbidden;
		CleanUp();
		
		return true;

	}
	return false;
}



