//+========================================================================+
//                            SIPPartyInSimulation.h                                 |
//            Copyright 1995 POLYCOM Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPPartyInSimulation.h                                                |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTYINSIMULATION__
#define __SIPPARTYINSIMULATION__

#include "Party.h"
#include "IpChannelParams.h"
#include "SIPInternals.h"

class CSegment;

extern "C" void SipPartyInSimulationEntryPoint(void* appParam);

class CSipPartyInSimulation: public CParty
{
CLASS_TYPE_1(CSipPartyInSimulation, CParty)
public:

	CSipPartyInSimulation();
	virtual ~CSipPartyInSimulation();
	virtual const char* NameOf() const { return "CSipPartyInSimulation";}
		  // Initializations
	void Create(CSegment& appParam);

		  // Operations

		  // Action functions
	virtual void OnLobbyIdent(CSegment* pParam);
	virtual void OnLobbyReject(CSegment* pParam);
	virtual void OnLobbyTransfer(CSegment* pParam);
	virtual void OnConfEstablishCall(CSegment* pParam);
	
	virtual void* GetMessageMap() { return (void*)m_msgEntries;}
	virtual void   CleanUp();
//
//	virtual void OnConfConnectCall(CSegment* pParam);
//	virtual void OnConfChangeMode(CSegment* pParam);
	virtual void OnConfCloseConnectingCall(CSegment* pParam);
//	virtual void OnConfStreamOnMediaConnecting(CSegment* pParam);
//	virtual void OnConfDisconnectChannelsConnecting(CSegment* pParam);
//	
//	virtual void OnPartyCallFailed(CSegment* pParam);
//	virtual void OnPartyRemoteCloseConnectingCall(CSegment* pParam);
	virtual void OnPartyReceivedAck(DWORD status);
//	virtual void OnPartyMediaStreamOn(cmCapDataType eType);
//	virtual void OnPartyBadStatusConnecting(CSegment* pParam);
	virtual void LogicalChannelDisconnect(DWORD eChannelType);
//
	void LogicalChannelDisconnectIpPartyFunction(DWORD eChannelType);
	void PartyOriginalRemoteCaps(CSipCaps* pRemoteCaps);
	void TellConfOnDisconnecting(int reason,const char* alternativeAddrStr=NULL);
	void LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor, DWORD channelType,DWORD vendorType = 0);
	void SetPartyMonitorBaseParamsAndConnectChannel(DWORD channelType,DWORD rate=0xFFFFFFFF,
								mcTransportAddress* partyAdd=NULL,mcTransportAddress* mcuAdd=NULL,
								DWORD protocol = (DWORD)eUnknownAlgorithemCapCode,
								DWORD pmIndex=0,DWORD vendorType = 0);
	void LogicalChannelUpdate(DWORD channelType, DWORD vendorType = 0);
	void OnConfPartyReceiveAudBridgeConnected();
	void OnConfPartyReceiveVidBridgeConnected();
//
//	virtual void OnPartyConnectToutConnecting(CSegment* pParam);
	virtual void OnPartyChannelsConnected();
//	void UpdateDbOnSipPrivateExtension();
	void UpdateDbOnChannelsConnected();
	void UpdateDbScm();


protected:
	PDECLAR_MESSAGE_MAP;

	EResponsibility m_eResponsibility;	
	CLobbyApi* m_pLobbyApi;
	CSipNetSetup* m_pNetSetup;
	CIpComMode* m_pInitialModeSIP;
	CSipCaps* m_pLocalCapSIP;
	enSipCodes m_eLobbyRejectReason;//only if lobby rejects call in
	BYTE m_bIsAdvancedVideoFeatures;
	WORD m_isAudioBridgeConnected;
	WORD m_isVideoBridgeConnected;
	
	virtual void DestroyPartyTask();
};

#endif
