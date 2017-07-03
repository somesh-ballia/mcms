////+========================================================================+
////                            SIPPartyIN.h                                 |
////            Copyright 1995 POLYCOM Technologies Ltd.                     |
////                   All Rights Reserved.                                  |
////-------------------------------------------------------------------------|
//// NOTE: This software contains valuable trade secrets and proprietary     |
//// information of POLYCOM Technologies Ltd. and is protected by law.       |
//// It may not be copied or distributed in any form or medium, disclosed    |
//// to third parties, reverse engineered or used in any manner without      |
//// prior written authorization from POLYCOM Technologies Ltd.              |
////-------------------------------------------------------------------------|
//// FILE:       SIPPartyIN.h                                                |
//// SUBSYSTEM:  MCMS                                                        |
//// PROGRAMMER:															   |
////-------------------------------------------------------------------------|
//// Who | Date       | Description                                          |
////-------------------------------------------------------------------------|
////     | 15/11/05   | This file contains								   |
////     |            |                                                      |
////+========================================================================+
//#ifndef __SIPPARTYIN__
//#define __SIPPARTYIN__
//
//#include "SIPParty.h"
//
//
//extern "C" void SipPartyInEntryPoint(void* appParam);
//
//
//class CSipPartyIn: public CSipParty
//{
//CLASS_TYPE_1(CSipPartyIn, CSipParty)
//public:
//
//	CSipPartyIn();
//	virtual ~CSipPartyIn();
//	virtual const char* NameOf() const { return "CSipPartyIn";}
//
//	void Create(CSegment& appParam);
//	virtual void* GetMessageMap() { return (void*)m_msgEntries;}
//	virtual void   CleanUp();
//
//	// state idle
//	void OnLobbyRejectIdle(CSegment* pParam);
//	void OnLobbyIdentIdle(CSegment* pParam);
//	void OnLobbyTransferIdle(CSegment* pParam);
//	void OnConfAllocateResourcesIdle(CSegment * pParam);
//	void OnConfEstablishCallIdle(CSegment* pParam);
//	// open channels state
//	void OnPartyChannelsConnectedOpenChannels(CSegment * pParam);
//	// open bridge state
//	virtual void HandleBridgeConnectedInd(DWORD status);
//	// party connecting state
//	void OnPartyReceivedAckConnecting(CSegment* pParam);
//
//	virtual void OnConfDisconnectChannelsConnecting(CSegment* pParam);
//	virtual void OnConfCloseCall(CSegment* pParam);
//	virtual void OnPartyCallFailed(CSegment* pParam);
//	virtual void OnPartyBadStatusConnecting(CSegment* pParam);
//	virtual void LogicalChannelDisconnect(DWORD eChannelType);
//	virtual void OnPartyConnectToutConnecting(CSegment* pParam);
//
//	void OnSipDisconnectSetupIdle(CSegment* pParam);
//
//protected:
//	void UpdateRemoteSupportHighCapabilities(CSipNetSetup* pNetSetup);
//
//	PDECLAR_MESSAGE_MAP;
//	EResponsibility m_eResponsibility;
//	CLobbyApi* m_pLobbyApi;
//	enSipCodes m_eLobbyRejectReason;//only if lobby rejects call in
//	DWORD	   m_DialInRejectConnectionId;
//	BYTE	   m_bSetRsrcParam;
//
//	virtual void DestroyPartyTask();
//};
//
//
//#endif

