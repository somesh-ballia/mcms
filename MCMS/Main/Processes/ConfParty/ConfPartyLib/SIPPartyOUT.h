////+========================================================================+
////                            SIPPartyOut.h                                |
////            Copyright 1995 POLYCOM Technologies Ltd.                     |
////                   All Rights Reserved.                                  |
////-------------------------------------------------------------------------|
//// NOTE: This software contains valuable trade secrets and proprietary     |
//// information of POLYCOM Technologies Ltd. and is protected by law.       |
//// It may not be copied or distributed in any form or medium, disclosed    |
//// to third parties, reverse engineered or used in any manner without      |
//// prior written authorization from POLYCOM Technologies Ltd.              |
////-------------------------------------------------------------------------|
//// FILE:       SIPPartyOut.h                                               |
//// SUBSYSTEM:  MCMS                                                        |
//// PROGRAMMER:															   |
////-------------------------------------------------------------------------|
//// Who | Date       | Description                                          |
////-------------------------------------------------------------------------|
////     | 15/11/05   | This file contains								   |
////     |            |                                                      |
////+========================================================================+
//
//#ifndef __SIPPARTYOUT__
//#define __SIPPARTYOUT__
//
//extern "C" void SipPartyOutEntryPoint(void* appParam);
//
//
//class CSipPartyOut : public CSipParty
//{
//CLASS_TYPE_1(CSipPartyOut, CSipParty)
//public:
//	CSipPartyOut();
//	virtual ~CSipPartyOut();
//	void  Create(CSegment& appParam);
//
//	virtual const char* NameOf() const { return "CSipPartyOut";}
//	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
//	virtual void   CleanUp();
//
//	// idle state
//	void OnConfInformResourceAllocatedIdle(CSegment * pParam);
//	void OnConfEstablishCallIdle(CSegment * pParam);
//	virtual void OnConfCloseCallIdle(CSegment * pParam);
//
//	// Open in channels before invite state
//	virtual void OnPartyChannelsConnectedOpenIn(CSegment * pParam);
//
//	// party control response with bridges connected (channels connected state)
//	virtual void HandleBridgeConnectedInd(DWORD status);
//
//	// connecting state (send invite until 200 OK response)
//	void OnPartyReceived200OkConnecting(CSegment * pParam);
//
//	// update in state after the party has receive the 200 OK
//	void OnPartyCloseChannelConnecting(CSegment * pParam);
//	void OnPartyChannelsUpdatedRecovery(CSegment * pParam);
//
//	// receive response after the re-invite to remove additional algorithms in the SDP
//	void OnPartyReInviteResponseInitReInvite(CSegment* pParam);
//
//	// re invite update in state after the party has receive the 200 OK for the re-invite (I think this will change nothing - need to check).
//	void OnPartyChannelsCloseReInviteUpdateIn(CSegment * pParam);
//
//	// open out channels state for dial out calls.
//	void OnPartyChannelsCloseReInviteCloseChannels(CSegment* pParam);
//	void OnPartyChannelsConnectedOpenOut(CSegment * pParam);
//	void EndInternalRecovery();
//	void OpenOutChannels();
//
//	// receive from the party control command to connect the call - send final Ack (remote connected state).
//	virtual void OnConfConnectCallRmtConnected(CSegment * pParam);
//
//	virtual void OnConfDisconnectChannelsConnecting(CSegment* pParam);
//	virtual void OnConfCloseCall(CSegment * pParam);
//
//	virtual void OnPartyCallFailed(CSegment * pParam);
//	virtual void OnPartyBadStatusConnecting(CSegment * pParam);
//	virtual void OnPartyConnectToutConnecting(CSegment* pParam);
//
//	void OnSipDisconnectIdle(CSegment * pParam);
//
//	void OnDnsResolutionCallIdle(CSegment * pParam);
//
//
//protected:
//
//	PDECLAR_MESSAGE_MAP;
//	virtual void PartyChannelsUpdatedOk();
//	void InformConfRemoteConnect();
//	void PartyConnectCall();
//	void ConnectCall();
//};
//
//
//
//#endif

