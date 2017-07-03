//+========================================================================+
//                            SipSlavePartyControlChangeMode.cpp                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SipPartyControlChangeMode.cpp                               |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: UriA                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// GuyD| 13/11/05   |  Create + Stage 1 - Move between same conferences    |
//+========================================================================+

#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "StateMachine.h"
#include "Segment.h"
#include "SysConfigKeys.h"
#include "SysConfig.h"
#include "NStream.h"
#include "ConfPartyDefines.h"
//#include "IpCommonTypes.h"
#include "IpCommonDefinitions.h"
#include "IpAddressDefinitions.h"
#include "IpScm.h"
#include "ConfPartyOpcodes.h"
#include "ConfDef.h"
#include "Conf.h"
#include "ConfApi.h"
#include "CommModeInfo.h"
#include "Capabilities.h"
#include "TaskApi.h"
#include "TaskApp.h"
#include "Party.h"
#include "PartyApi.h"

#include "SipDefinitions.h"
#include "SipStructures.h"
#include "SipHeadersList.h"
#include "SipCsReq.h"
#include "SipCsInd.h"
#include "SipUtils.h"
#include "SipCaps.h"
#include "SipScm.h"
#include "SipCall.h"
#include "NetSetup.h"
#include "IpNetSetup.h"
#include "IpPartyControl.h"
#include "SipNetSetup.h"
#include "SIPCommon.h"
#include "SIPPartyControl.h"
#include "SIPPartyControlChangeMode.h"
#include "SIPSlavePartyControlChangeMode.h"
#include "PartyApi.h"
#include "PartyRsrcDesc.h"
#include  "H263VideoMode.h"
#include "WrappersResource.h"
#include "IpChannelParams.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
#include  "ConfPartyGlobals.h"
#include  "H263VideoMode.h"
#include "StatusesGeneral.h"
#include "SIPSlavePartyControl.h"

#include "ContentBridge.h"

////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipSlaveChangeModePartyCntl)

	ONEVENT(SIP_PARTY_CHANS_CONNECTED,			IDLE,							CSipSlaveChangeModePartyCntl::NullActionFunction)
//	ONEVENT(CHANGEMODETOUT,						CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnChangeModeToutChangeBridges)
//	ONEVENT(IPPARTYCONNECTED,					CONF_RESPONSE_PARTY_RE_CAPS,	CSipChangeModePartyCntl::OnPartyRemoteConnectedResponseReCaps)
//	ONEVENT(CHANGEMODETOUT,						CONF_RESPONSE_PARTY_RE_CAPS,	CSipChangeModePartyCntl::OnChangeModeToutResponseParty)
	ONEVENT(PARTY_AUDIO_CONNECTED,                  ANYCASE,                CSipSlavePartyCntl::OnAudConnectPartyConnectAudio)
	ONEVENT(PARTY_VIDEO_CONNECTED,			ANYCASE,		CSipSlavePartyCntl::OnVideoBrdgConnected)

	ONEVENT(IPPARTYCONNECTED,               CHANGE_BRIDGES,             CSipSlaveChangeModePartyCntl::OnPartyRemoteConnectedSecondTime)
	ONEVENT(IPPARTYCONNECTED,               PARTY_RE_CAPS,             CSipSlaveChangeModePartyCntl::OnPartyRemoteConnectedSecondTime)

PEND_MESSAGE_MAP(CSipSlaveChangeModePartyCntl,CSipChangeModePartyCntl);


///////////////////////////////////////////////////
CSipSlaveChangeModePartyCntl::CSipSlaveChangeModePartyCntl()
{
	m_VideoPartyType = eVideo_party_type_none;

	VALIDATEMESSAGEMAP;
}

///////////////////////////////////////////////////
CSipSlaveChangeModePartyCntl::~CSipSlaveChangeModePartyCntl()
{
}

////////////////////////////////////////////////
const char*   CSipSlaveChangeModePartyCntl::NameOf() const
{
	return "CSipSlaveChangeModePartyCntl";
}

///////////////////////////////////////////////////
CSipSlaveChangeModePartyCntl& CSipSlaveChangeModePartyCntl::operator= (const CSipSlaveChangeModePartyCntl& other)
{
	if ( &other == this )
		return *this;

	m_VideoPartyType				= other.m_VideoPartyType;

	(CSipChangeModePartyCntl&)*this = (CSipChangeModePartyCntl&)other;
	return *this;
}

/////////////////////////////////////////////////////////////////////////////
void CSipSlaveChangeModePartyCntl::EndChangeMode()
{
	PTRACE2PARTYID(eLevelInfoNormal,"CSipSlaveChangeModePartyCntl::EndChangeMode: Name - ",m_partyConfName, GetPartyRsrcId());

	//send to master end change mode;
	m_bIsNewScm = FALSE;//noa temp
	if (!m_bIsNewScm)
		m_pPartyApi->SlaveEndChangeMode();

	CSipChangeModePartyCntl::EndChangeMode();
}
///////////////////////////////////////////////////////////////
void CSipSlaveChangeModePartyCntl::OnPartyRemoteConnectedSecondTime(CSegment* pParam)
{
    PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnPartyRemoteConnectedSecondTime", GetPartyRsrcId());
    m_state = IDLE;
    SendMessageFromSlaveToMaster(m_TipPartyType, PARTY_SLAVE_TO_MASTER_RECAP_ACK, NULL);

}


