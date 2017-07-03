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
#include "IpWebRtcReq.h"
#include "IpWebRtcInd.h"
#include "SIPControlWebRtc.h"
#include "Lobby.h"
#include "SIPParty.h"
#include "ConfPartyGlobals.h"
#include "IpServiceListManager.h"
#include "IPParty.h"
#include "SIPTransaction.h"
#include "SIPTransInviteWithSdpInd.h"
#include "SIPTransInviteWebRtcWithSdpInd.h"

////////////////////////////////////////////////////////////////////////////

PBEGIN_MESSAGE_MAP(CSipTransInviteWebRtcWithSdpInd)

ONEVENT(SIP_WEBRTC_PARTY_ESTABLISH_CALL,	IDLE,					CSipTransInviteWebRtcWithSdpInd::OnWebRtcPartyEstablishCallIdle)
ONEVENT(WEBRTC_CONNECT_FAILURE,				IDLE,					CSipTransInviteWebRtcWithSdpInd::OnWebRtcConnectFailure)
ONEVENT(WEBRTC_CONNECT_TOUT,				IDLE,					CSipTransInviteWebRtcWithSdpInd::OnWebRtcConnectTout)


PEND_MESSAGE_MAP(CSipTransInviteWebRtcWithSdpInd,CSipTransInviteWithSdpInd);

////////////////////////////////////////////////////////////////////////////
CSipTransInviteWebRtcWithSdpInd::CSipTransInviteWebRtcWithSdpInd(CTaskApp *pOwnerTask):CSipTransInviteWithSdpInd(pOwnerTask)
{
	m_pBestMode = NULL;
}

////////////////////////////////////////////////////////////////////////////
CSipTransInviteWebRtcWithSdpInd::~CSipTransInviteWebRtcWithSdpInd()
{
	POBJDELETE(m_pBestMode);
}

/////////////////////////////////////////////////////////////////////////////////////////
BOOL CSipTransInviteWebRtcWithSdpInd::MakeANewCallOnPartyEstablishCallIdle(CSipComMode * pBestMode)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWebRtcWithSdpInd::MakeANewCallOnPartyEstablishCallIdle: Name ");

	CSipWebRtcCntl *pSipWebRtcCntl = m_pSipCntl->GetWebRtcCntl();
	if (!pSipWebRtcCntl) {
		PTRACE(eLevelError,"CSipTransInviteWebRtcWithSdpInd::MakeANewCallOnPartyEstablishCallIdle: Not WebRtc control ");
		return TRUE;
	}

	m_pBestMode = new CSipComMode(*pBestMode);
	pSipWebRtcCntl->SendWebRtcConnectReq(m_pBestMode);

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWebRtcWithSdpInd::OnWebRtcPartyEstablishCallIdle(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWebRtcWithSdpInd::OnWebRtcPartyEstablishCallIdle: Name ");

	CSipWebRtcCntl *pSipWebRtcCntl = m_pSipCntl->GetWebRtcCntl();
	if (!pSipWebRtcCntl) {
		PTRACE(eLevelError,"CSipTransInviteWebRtcWithSdpInd::OnWebRtcPartyEstablishCallIdle: Not WebRtc control ");
		return;
	}

	DoMakeANewCallOnPartyEstablishCallIdle(m_pBestMode);
	POBJDELETE(m_pBestMode);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWebRtcWithSdpInd::OnWebRtcConnectFailure(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWebRtcWithSdpInd::OnWebRtcConnectFailure: Name ");
	POBJDELETE(m_pBestMode);
	SetDialState(kWebRtcConnectFailure);
	EndTransaction(WEBRTC_CONNECT_FAILURE);
}

/////////////////////////////////////////////////////////////////////////////////////////
void CSipTransInviteWebRtcWithSdpInd::OnWebRtcConnectTout(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipTransInviteWebRtcWithSdpInd::OnWebRtcConnectTout: Name ");
	POBJDELETE(m_pBestMode);
	SetDialState(kWebRtcConnectTimeOut);
	EndTransaction(WEBRTC_CONNECT_TOUT);
}
