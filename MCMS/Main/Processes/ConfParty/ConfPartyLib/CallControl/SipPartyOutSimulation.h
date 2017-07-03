//+========================================================================+
//                            SipPartyOutSimulation.h                                 |
//            Copyright 1995 POLYCOM Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SipPartyOutSimulation.h                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:															   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 15/11/05   | This file contains								   |
//     |            |                                                      |
//+========================================================================+

#ifndef __SIPPARTYOUTSIMULATION__
#define __SIPPARTYOUTSIMULATION__

#include "Party.h"
#include "IpChannelParams.h"


class CSegment;


extern "C" void SipPartyOutEntryPointSimulation(void* appParam);

class CSipPartyOutSimulation: public CParty
{
CLASS_TYPE_1(CSipPartyOutSimulation, CParty)
public:
	CSipPartyOutSimulation();
	virtual ~CSipPartyOutSimulation();
	virtual const char* NameOf() const { return "CSipPartyOutSimulation";}
 
	void  Create(CSegment& appParam);	

	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	virtual void  CleanUp();

	virtual void OnConfEstablishCall(CSegment * pParam);

	void OnPartyChannelsConnected(CIpComMode* pIpComMode);
	void UpdateDbOnChannelsConnected();
	void UpdateDbScm();
	void SetPartyMonitorBaseParamsAndConnectChannel(DWORD channelType,DWORD rate=0xFFFFFFFF,
								mcTransportAddress* partyAdd=NULL,mcTransportAddress* mcuAdd=NULL,
								DWORD protocol = (DWORD)eUnknownAlgorithemCapCode,
								DWORD pmIndex=0,DWORD vendorType = 0);
	void LogicalChannelConnect(CPrtMontrBaseParams *pPrtMonitor, DWORD channelType, DWORD vendorType);
	void LogicalChannelUpdate(DWORD channelType, DWORD vendorType);
	void OnConfPartyReceiveAudBridgeConnected();
	void OnConfPartyReceiveVidBridgeConnected();
	virtual void OnPartyReceivedAck(DWORD status);
	virtual void OnConfCloseConnectingCall(CSegment* pParam);
	virtual void LogicalChannelDisconnect(DWORD eChannelType);
	void TellConfOnDisconnecting(int reason,const char* alternativeAddrStr=NULL);


//	virtual void OnConfConnectCall(CSegment * pParam);
//	virtual void OnConfChangeMode(CSegment * pParam);
//	virtual void OnConfDisconnectChannelsConnecting(CSegment* pParam);
//	virtual void OnConfCloseConnectingCall(CSegment * pParam);
//
//	void OnPartyReceived200Ok(CSegment * pParam);
//	virtual void OnPartyCallFailed(CSegment * pParam);
//	virtual void OnPartyRemoteCloseConnectingCall(CSegment* pParam);
//	virtual void OnPartyMediaStreamOn(cmCapDataType eMediaType);
//	virtual void OnPartyChannelsConnected(CSegment * pParam);
//	virtual void OnPartyBadStatusConnecting(CSegment * pParam);
//	virtual void OnPartyReInviteResponseConnecting(CSegment* pParam);
//	virtual void OnPartyConnectToutConnecting(CSegment* pParam);
//	virtual void OnDnsResolvingTout(CSegment* pParam);
//	
//	void OnDnsResolveInd(CSegment* pParam);
//	void OnDnsServiceInd(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP;

	EResponsibility m_eResponsibility;	
	CSipNetSetup* m_pNetSetup;
	CIpComMode* m_pInitialModeSIP;
	CSipCaps* m_pLocalCapSIP;
	BYTE m_bIsAdvancedVideoFeatures;
	WORD m_isAudioBridgeConnected;
	WORD m_isVideoBridgeConnected;
	WORD m_eDialState;
	
	virtual void DestroyPartyTask();
//	void ConnectCall();
};

#endif
