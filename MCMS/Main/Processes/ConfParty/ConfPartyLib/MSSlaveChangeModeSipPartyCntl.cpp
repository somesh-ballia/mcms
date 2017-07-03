//+========================================================================+
//                            MSSlaveChangeModeSipPartyCntl.cpp                |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       MSSlaveChangeModeSipPartyCntl.cpp                               |
// SUBSYSTEM:  ConfParty                                                   |
// PROGRAMMER: Flora Yao                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
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
#include "MSSlaveChangeModeSipPartyCntl.h"
#include "PartyApi.h"
#include "PartyRsrcDesc.h"
#include "H263VideoMode.h"
#include "WrappersResource.h"
#include "IpChannelParams.h"
#include "ConfPartyOpcodes.h"
#include "ConfPartyDefines.h"
#include "ConfPartyGlobals.h"
#include "H263VideoMode.h"
#include "StatusesGeneral.h"
#include "SIPSlavePartyControl.h"

#include "ContentBridge.h"
#include "SipVsrControl.h"


////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CMSSlaveChangeModeSipPartyCntl)

/* Flora comment: TBD */
#if 0
	ONEVENT(SIP_PARTY_CHANS_CONNECTED,			IDLE,							CSipSlaveChangeModePartyCntl::NullActionFunction)
//	ONEVENT(CHANGEMODETOUT,						CHANGE_BRIDGES,					CSipChangeModePartyCntl::OnChangeModeToutChangeBridges)
//	ONEVENT(IPPARTYCONNECTED,					CONF_RESPONSE_PARTY_RE_CAPS,	CSipChangeModePartyCntl::OnPartyRemoteConnectedResponseReCaps)
//	ONEVENT(CHANGEMODETOUT,						CONF_RESPONSE_PARTY_RE_CAPS,	CSipChangeModePartyCntl::OnChangeModeToutResponseParty)
	ONEVENT(PARTY_AUDIO_CONNECTED,                  ANYCASE,                CSipSlavePartyCntl::OnAudConnectPartyConnectAudio)
	ONEVENT(PARTY_VIDEO_CONNECTED,			ANYCASE,		CSipSlavePartyCntl::OnVideoBrdgConnected)

	ONEVENT(IPPARTYCONNECTED,               CHANGE_BRIDGES,             CSipSlaveChangeModePartyCntl::OnPartyRemoteConnectedSecondTime)
	ONEVENT(IPPARTYCONNECTED,               PARTY_RE_CAPS,             CSipSlaveChangeModePartyCntl::OnPartyRemoteConnectedSecondTime)
#endif
	ONEVENT(SCMCHANGEMODE,						IDLE,							CMSSlaveChangeModeSipPartyCntl::OnConfChangeModeIdle)
    ONEVENT(SIP_PARTY_SINGLE_VSR_MSG_IND,		ANYCASE,			CMSSlaveChangeModeSipPartyCntl::OnSlavesControllerSingleVsrMassageInd)

PEND_MESSAGE_MAP(CMSSlaveChangeModeSipPartyCntl,CSipChangeModePartyCntl);


///////////////////////////////////////////////////
CMSSlaveChangeModeSipPartyCntl::CMSSlaveChangeModeSipPartyCntl()
{
	m_VideoPartyType = eVideo_party_type_none;

	VALIDATEMESSAGEMAP;
}

///////////////////////////////////////////////////
CMSSlaveChangeModeSipPartyCntl::~CMSSlaveChangeModeSipPartyCntl()
{
}

////////////////////////////////////////////////
const char*   CMSSlaveChangeModeSipPartyCntl::NameOf() const
{
	return "CMSSlaveChangeModeSipPartyCntl";
}

///////////////////////////////////////////////////
CMSSlaveChangeModeSipPartyCntl& CMSSlaveChangeModeSipPartyCntl::operator= (const CMSSlaveChangeModeSipPartyCntl& other)
{
	if ( &other == this )
		return *this;

	m_VideoPartyType				= other.m_VideoPartyType;

	(CMSSlaveChangeModeSipPartyCntl&)*this = (CMSSlaveChangeModeSipPartyCntl&)other;
	return *this;
}

void CMSSlaveChangeModeSipPartyCntl::OnConfChangeModeIdle(CSegment* pParam)
{
	PTRACE2PARTYID(eLevelInfoNormal,"CMSSlaveChangeModeSipPartyCntl::OnConfChangeModeIdle: Name - ",m_partyConfName, GetPartyRsrcId());
	ChangeModeIdle(pParam);

	if(m_connectingState == IP_CONNECTING) // when first invite and offering (invite req or invite ind no sdp) we are still connecting.
	{
		//Set the m_connectingState to IP_CONNECTED
		m_connectingState = IP_CONNECTED;
	}

	
}
//////////////////////////////////////////////////////////////////////////////
void CMSSlaveChangeModeSipPartyCntl::OnSlavesControllerSingleVsrMassageInd(CSegment* pMsg)
{
	TRACEINTO;
	if(!m_pPartyApi)
		PASSERT_AND_RETURN(1);
	m_pPartyApi->SendSingleVsrMsgIndToSipParty(pMsg);
}
//////////////////////////////////////////////////////////////////////////////
void CMSSlaveChangeModeSipPartyCntl::SendSingleUpdatePacsiInfoToSlavesController(const MsSvcParamsStruct& pacsiInfo,  BYTE isReasonFecOrRed)
{
	if(IsMsSlaveOut())
	{
		TRACEINTO << " ConfName:" << m_partyConfName << ", PartyName:" << GetPartyRsrcId()
				  << ", isReasonFecOrRed:" << (DWORD)isReasonFecOrRed << " - slave out";
		CSegment* pSeg = new CSegment;
		*pSeg << GetPartyRsrcId() << isReasonFecOrRed;
		pSeg->Put((BYTE*)(& pacsiInfo),sizeof(MsSvcParamsStruct));

		m_pTaskApi->SendMsgToSlavesController(m_MasterRsrcId, SINGLE_PACSI_INFO_IND, pSeg);
	}
	else
	{
		TRACEINTO << " ConfName:" << m_partyConfName << ", PartyName:" << GetPartyRsrcId()
				  << ", isReasonFecOrRed:" << (DWORD)isReasonFecOrRed << " - not slave out, pacsi will not be sent";
	}
}

#if 0
///////////////////////////////////////////////////////////////
void CMSSlaveChangeModeSipPartyCntl::StartSingleVsr(const TCmRtcpVsrInfo& vsrInfo)
{
	//=============================================
	// Converting embedded values to party domain
	//=============================================
	ST_VSR_SINGLE_STREAM sipCntlVsr;

	sipCntlVsr.key_frame		= vsrInfo.keyFrame;
	sipCntlVsr.msi				= vsrInfo.MSI;
	sipCntlVsr.partyId			= GetPartyId(vsrInfo.senderSSRC);
	sipCntlVsr.sender_ssrc		= vsrInfo.senderSSRC;
	sipCntlVsr.num_vsrs_params	= vsrInfo.numberOfEntries;

	for(DWORD entryNdx = 0; entryNdx < sipCntlVsr.num_vsrs_params; ++entryNdx)
	{
		const TCmRtcpVsrEntry&	cmRtcpVsrEntry	= vsrInfo.VSREntry[entryNdx];
		ST_VSR_PARAMS&			sipCntlVsrEntry	= sipCntlVsr.st_vsrs_params[entryNdx];

		sipCntlVsrEntry.aspect_ratio	= VsrAspectToSipAspect(cmRtcpVsrEntry.aspectRatioBitMask);
		sipCntlVsrEntry.min_bitrate		= cmRtcpVsrEntry.minBitRate;
		sipCntlVsrEntry.frame_rate		= VsrFRToSipFR(cmRtcpVsrEntry.frameRateBitmask);
		sipCntlVsrEntry.max_height		= cmRtcpVsrEntry.maxHeight;
		sipCntlVsrEntry.max_width		= cmRtcpVsrEntry.maxWidth;
		sipCntlVsrEntry.payload_type	= cmRtcpVsrEntry.payloadType;
	}

	//==================
	// Sending message
	//==================
	CMedString log;
	log << 	"CSipVsrCtrl::OnIpCmVsrMessageInd - Received VSR for SSRC[" << sipCntlVsr.sender_ssrc << "], MSI[" << sipCntlVsr.msi << "], keyFrame[" << sipCntlVsr.key_frame <<
						"], partyId[" << sipCntlVsr.partyId << "], number of entries received:" << sipCntlVsr.num_vsrs_params;
	PTRACE(eLevelInfoNormal, log.GetString());
	if (m_pPartyApi) m_pPartyApi->SendVsrMsgIndToSipParty(&sipCntlVsr);
}


/////////////////////////////////////////////////////////////////////////////
void CSipSlaveChangeModePartyCntl::OnPartyRemoteConnectedSecondTime(CSegment* pParam)
{
    PTRACEPARTYID(eLevelInfoNormal,"CSipSlavePartyCntl::OnPartyRemoteConnectedSecondTime", GetPartyRsrcId());
    m_state = IDLE;
    SendMessageFromSlaveToMaster(m_TipPartyType, PARTY_SLAVE_TO_MASTER_RECAP_ACK, NULL);

}

#endif


