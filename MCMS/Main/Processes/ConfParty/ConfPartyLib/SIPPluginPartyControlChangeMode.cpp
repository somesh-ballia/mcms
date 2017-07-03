//+========================================================================+
//                            SipPartyControlChangeMode.cpp                |
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
#include "SIPPluginPartyControlChangeMode.h"
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

#include "ContentBridge.h"

////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipPluginChangeModePartyCntl)
	ONEVENT(PRESENTATION_OUT_STREAM_UPDATED,	ANYCASE,						CSipChangeModePartyCntl::OnPartyPresentationOutStreamUpdate)
	ONEVENT(PARTY_CONTENT_CONNECTED,			ANYCASE,							CSipPluginChangeModePartyCntl::OnContentBrdgConnectedAnycase)
	ONEVENT(SIP_PARTY_CONF_PWD_STAUTS_ACK,	ANYCASE,							CSipPluginChangeModePartyCntl::OnPartyAuthCompleate)
	ONEVENT(CCS_PLUGIN_AUTH_TIMEOUT,ANYCASE,									CSipPluginChangeModePartyCntl::OnPartyAuthTOUT)
PEND_MESSAGE_MAP(CSipPluginChangeModePartyCntl,CSipChangeModePartyCntl);


///////////////////////////////////////////////////
CSipPluginChangeModePartyCntl::CSipPluginChangeModePartyCntl()
{
	
	VALIDATEMESSAGEMAP
}
///////////////////////////////////////////////////
CSipPluginChangeModePartyCntl::~CSipPluginChangeModePartyCntl()
{
}
////////////////////////////////////////////////
const char*   CSipPluginChangeModePartyCntl::NameOf() const
{
  	return "CSipPluginChangeModePartyCntl";
}
////////////////////////////////////////////////
int  CSipPluginChangeModePartyCntl::OnContentBrdgConnectedAnycase(CSegment* pParam)
{
	PTRACE(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::OnContentBrdgConnectedAnycase");
	CSipChangeModePartyCntl::OnContentBrdgConnectedAnycase(pParam);

	BOOL bPartially = FALSE;
	
	if (!AreTwoDirectionsConnectedToAudioBridge()) // (no audio) => partialy connected
	{
		bPartially = TRUE;
	}

	if (bPartially)
	{
		PTRACE(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::OnContentBrdgConnectedAnycase PARTY_CONNECTED_PARTIALY");
		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED_PARTIALY);
	}
	else if (m_isContentConn)
	{
		PTRACE(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::OnContentBrdgConnectedAnycase PARTY_CONNECTED");
		m_pTaskApi->UpdateDB(m_pParty,PARTYSTATE,PARTY_CONNECTED);
		UpdatePartyStateInCdr();
	}

	//UpdatePartyStateInCdr();

	return 0;
		
}
/////////////////////////////////////////////////////////////////////////////////
DWORD CSipPluginChangeModePartyCntl::GetPossibleContentRate() const
{
	
	DWORD ContentRate = m_pSipRemoteCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation);
	PTRACE2INT(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::GetPossibleContentRate",ContentRate);
	return ContentRate;
}
////////////////////////////////////////////////////////////////////////////////////
void  CSipPluginChangeModePartyCntl::OnPartyAuthCompleate(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::OnPartyAuthCompleate: ", GetPartyRsrcId());
	if (IsValidTimer(CCS_PLUGIN_AUTH_TIMEOUT))
		DeleteTimer(CCS_PLUGIN_AUTH_TIMEOUT);
	
	CSipPartyCntl::OnPartyAuthCompleate(pParam);
	
	BYTE bRes = FALSE;
	bRes = CheckIfNeedToSendIntra();

	if (!m_isPartyInConf)
		return;
	
	if(bRes)
	{
		SendIntraToParty();
		SendVideoPreferenceToParty();
	}
	
	ConnectToContentBridgeIfPossible();
}
//////////////////////////////////////////////////////////////////////////////////////////
void  CSipPluginChangeModePartyCntl::OnPartyAuthTOUT(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::OnPartyAuthTOUT: ", GetPartyRsrcId());

	m_pTaskApi->PartyDisConnect(PASSWORD_FAILURE,m_pParty);
}
//////////////////////////////////////////////////////////////////////////////////////////
BYTE CSipPluginChangeModePartyCntl::ChangeVideoBrdgRsrcIfNeeded()
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::ChangeVideoBrdgRsrcIfNeeded ", GetPartyRsrcId());
	return FALSE;
}
//////////////////////////////////////////////////////////////////////////////////////////
void CSipPluginChangeModePartyCntl::SetPartyStateUpdateDbAndCdrAfterEndConnected(BYTE reason)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::SetPartyStateUpdateDbAndCdrAfterEndConnected ", GetPartyRsrcId());
}
//////////////////////////////////////////////////////////////////////////////////////////
BYTE  CSipPluginChangeModePartyCntl::IsChangeContentNeeded(BYTE bSetChangeModeStateIfNeeded)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::IsChangeContentNeeded ", GetPartyRsrcId());
	BYTE bChange = CSipChangeModePartyCntl::IsChangeContentNeeded(bSetChangeModeStateIfNeeded);
	return bChange;
}
//////////////////////////////////////////////////////////////////////////////////////////
void CSipPluginChangeModePartyCntl::ChangeScm(CIpComMode* pScm)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::ChangeScm ", GetPartyRsrcId());
	if (!m_isPartyInConf)
	{
		//Note: the Conf password and the chairperson password are limited to 16 on EMA!
		char strPwd[64]={0};	
		memset(strPwd, '\0', 64);
		
		strcpy(strPwd, "ConfPwd:");
		strncat(strPwd, m_pConf->GetCommConf()->GetEntryPassword(), 16);
		strcat(strPwd, "H243Pwd:");
		strncat(strPwd, m_pConf->GetCommConf()->GetH243Password(),  16);
		
		m_pPartyApi->SendConfEntryPassword(strPwd);
		if (IsValidTimer(CCS_PLUGIN_AUTH_TIMEOUT))
			DeleteTimer(CCS_PLUGIN_AUTH_TIMEOUT);
		StartTimer(CCS_PLUGIN_AUTH_TIMEOUT, 2*60*SECOND);

		PTRACEPARTYID(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::ChangeScm starting plug auth ", GetPartyRsrcId());

	}
	return CSipChangeModePartyCntl::ChangeScm(pScm) ;
}
//////////////////////////////////////////////////////////////////////////////////////////
 void CSipPluginChangeModePartyCntl::ChangeModeIdle(CSegment* pParam)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginChangeModePartyCntl::ChangeModeIdle ", GetPartyRsrcId());
	return CSipChangeModePartyCntl::ChangeModeIdle(pParam);
}
//////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPluginChangeModePartyCntl::GetMinContentPartyRate(DWORD currContentRate)
{
	DWORD MinContentRate = 0;

	// tmp - Eitan:
	DWORD remoteRate = GetPossibleContentRate() * 100;//return remote content rate
	CCommConf* pCommConf = (CCommConf*)m_pConf->GetCommConf();
	BYTE xConfRate = pCommConf->GetConfTransferRate();//GetConfRate();
	CUnifiedComMode unifiedComMode(pCommConf->GetEnterpriseModeFixedRate(),xConfRate,pCommConf->GetIsHighProfileContent());
	

	CCapSetInfo lCapInfo = eUnknownAlgorithemCapCode;
	DWORD confRate    = lCapInfo.TranslateReservationRateToIpRate(xConfRate) * 1000;
	//BYTE lConfRate = CUnifiedComMode::TranslateXferRateToAmcRate(xConfRate);
	//DWORD confRate = m_pConf->TranslateAMCRateIPRate(lConfRate) * 100;	
	eEnterpriseMode ContRatelevel = (eEnterpriseMode)pCommConf->GetEnterpriseMode();
	DWORD possibleConfContentRate = unifiedComMode.FindIpContentRateByLevel(confRate,ContRatelevel, (ePresentationProtocol)pCommConf->GetPresentationProtocol(),
																			pCommConf->GetCascadeOptimizeResolution(), pCommConf->GetConfMediaType());

	MinContentRate = min(possibleConfContentRate,remoteRate);
	if (m_lastMinContentRate != MinContentRate)
	{
		CLargeString cstr;
		cstr << "CSipPluginChangeModePartyCntl::GetMinContentPartyRate\n";
		cstr << "remote Content Rate: " << remoteRate << ", conf Content Rate: " << currContentRate  << "\n";
		cstr << ", confRate: " << confRate << "\n";
		cstr << "possible content rate in conf (according to party's call rate) is: " << possibleConfContentRate << "\n";
		cstr << "chosen rate is: " << MinContentRate;
		PTRACE(eLevelInfoNormal,cstr.GetString());
		m_lastMinContentRate = MinContentRate;
	}

	return MinContentRate;
}
//////////////////////////////////////////////////////////////////////////////////////////
