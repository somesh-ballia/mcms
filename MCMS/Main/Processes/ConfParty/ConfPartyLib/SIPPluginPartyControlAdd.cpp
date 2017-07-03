//+========================================================================+
//                            SIPPartyControlAdd.CPP                       |
//            Copyright 2005 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyControlAdd.CPP                                      |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+


#include "StatusesGeneral.h"
#include "DataTypes.h"
#include "StateMachine.h"
#include "Segment.h"
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
#include "SIPPluginPartyControlAdd.h"
#include "PartyApi.h"
#include "PartyRsrcDesc.h"
#include "BridgeMoveParams.h"
#include "ApiStatuses.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include  "ConfPartyGlobals.h"
#include  "H263VideoMode.h"
#include "WrappersResource.h"  //to be removed noa
#include "ServiceConfigList.h"
#include "IpServiceListManager.h"
#include "SIPInternals.h"
#include "AudioBridgeInterface.h"
#include "ScpHandler.h"
#include "CDRUtils.h"

extern "C" void SipPartyOutCreateEntryPoint(void* appParam);
extern "C" void SipPartyOutEntryPointSimulation(void* appParam);
extern const char* GetStatusAsString(int status);
extern CIpServiceListManager* GetIpServiceListMngr();



extern const char* MOC_PRODUCT_NAME; // This product name is compatible with "Microsoft Office Communicator" and "Microsoft Lync"

extern const char* MOC_PRODUCT_NAME; // This product name is compatible with "Microsoft Office Communicator" and "Microsoft Lync"


#define MFA_CONNECT_TIME	20
#define CONNECT_TIME		100 // all internal MCU units receive 5 seconds to reply, and the network has 50 second -> 5+5+50+5+5 <= 100
#define REALLOCATE_TO_LOWER_VIDEO 2

////////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CSipPluginAddPartyCntl)
PEND_MESSAGE_MAP(CSipPluginAddPartyCntl,CSipAddPartyCntl);


////////////////////////////////////////////////////////////////////////////////
CSipPluginAddPartyCntl::CSipPluginAddPartyCntl()
{
	//m_pConfEntryPwd = NULL;
}
////////////////////////////////////////////////////////////////////////////////
CSipPluginAddPartyCntl::~CSipPluginAddPartyCntl()
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CSipPluginAddPartyCntl::Create(PartyControlInitParameters&	 partyControInitParam,PartyControlDataParameters &partyControlDataParams)
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginAddPartyCntl::Create:: ", GetPartyRsrcId());
	
	CSipAddPartyCntl::Create(partyControInitParam,partyControlDataParams);
	
	m_pIpInitialMode->SetMediaOff(cmCapVideo, cmCapReceiveAndTransmit, kRolePeople);
	m_pIpInitialMode->SetMediaOff(cmCapData, cmCapReceiveAndTransmit, kRolePeople);
	m_pIpInitialMode->SetBfcp(eTransportTypeUdp);
	//m_pIpInitialMode->SetIsLpr(FALSE);
}
/////////////////////////////////////////////////////////////////////////////
//Send CSipPluginAddPartyCntl event to resource allocator process and changes state to ALLOCATE.
void CSipPluginAddPartyCntl::AllocatePartyResources()
{
	//PTRACEPARTYID(eLevelInfoNormal, "CSipAddPartyCntl::AllocatePartyResources", GetPartyRsrcId());
	TRACEINTO << " CSipPluginAddPartyCntl::AllocatePartyResources::" << "monitor_conf_id: " << m_monitorConfId << ", monitor_party_id: " << m_monitorPartyId << ", party name: " << m_partyConfName ;

	BOOL bEnableFreeVideoResources = GetSystemCfgFlagInt<BOOL>(m_serviceId, CFG_KEY_SIP_FREE_VIDEO_RESOURCES);
	EAllocationPolicy bIsFreeVideoResources = eAllocateAllRequestedResources;
	eVideoPartyType localVideoPartyType = eVSW_Content_for_CCS_Plugin_party_type;
	ETipPartyTypeAndPosition tipPartyType = eTipNone;
	BOOL portGaugeThresholdExceeded = FALSE;
	DWORD artCapacity = 0;
	
	CreateAndSendAllocatePartyResources(eIP_network_party_type, localVideoPartyType, bIsFreeVideoResources, portGaugeThresholdExceeded,m_pSIPNetSetup->GetEnableSipICE(),artCapacity,tipPartyType, 0xFFFF, TRUE);
	m_state = ALLOCATE_RSC;
}
////////////////////////////////////////////////////////////////////////////
//skip the reallocate, and jump to connect bridges
void CSipPluginAddPartyCntl::HandleReAllocationForNotOfferer()
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginAddPartyCntl::HandleReAllocationForNotOfferer: ", GetPartyRsrcId());
	CSipAddPartyCntl::ConnectBridgesSetup();
}
////////////////////////////////////////////////////////////////////////////
void CSipPluginAddPartyCntl::EstablishCall(BYTE eTransportType)
{
	CSipAddPartyCntl::EstablishCall(eTransportType);
	
	//StartTimer(CCS_PLUGIN_AUTH_TIMEOUT, 2*60*SECOND);
	//StartTimer
}
//////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPluginAddPartyCntl::GetPossibleContentRate() const
{
	DWORD ContentRate = m_pSipRemoteCaps->GetMaxVideoBitRate(cmCapReceiveAndTransmit, kRolePresentation);
	PTRACE2INT(eLevelInfoNormal,"CSipPluginAddPartyCntl::GetPossibleContentRate: ", ContentRate);	
	return ContentRate;
}
//////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPluginAddPartyCntl::SendCreateParty(ENetworkType networkType, BYTE bIsMrcCall) const
{
	PTRACEPARTYID(eLevelInfoNormal,"CSipPluginAddPartyCntl::SendCreateParty: ", GetPartyRsrcId());
	return CSipAddPartyCntl::SendCreateParty(networkType,TRUE);//TRUE for plug in, which does not have video cap but content only
}
//////////////////////////////////////////////////////////////////////////////////////////
DWORD CSipPluginAddPartyCntl::GetMinContentPartyRate(DWORD currContentRate)
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
		cstr << "CSipPluginAddPartyCntl::GetMinContentPartyRate\n";
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
