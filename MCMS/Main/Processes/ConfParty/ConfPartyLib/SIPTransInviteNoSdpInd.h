//+========================================================================+
//                  SIPTransInviteNoSdpInd.h	                      	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteNoSdpInd.h                                	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSINVITENOSDPIND_H_
#define SIPTRANSINVITENOSDPIND_H_

class CSipTransInviteNoSdpInd : public CSipTransaction
{ 
CLASS_TYPE_1(CSipTransInviteNoSdpInd, CSipTransaction)

public:
	CSipTransInviteNoSdpInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransInviteNoSdpInd();
	virtual const char* NameOf() const {return "CSipTransInviteNoSdpInd";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries;}
	//state wait for conf init call
	virtual void OnPartyEstablishCallIdle(CSegment* pParam);
	// open In channels state
	void OnPartyChannelsConnectedOpenInChannels(CSegment * pParam);
	// open bridge state
	virtual void HandleBridgeConnectedInd(DWORD status);
	// party connecting state
	void OnPartyReceivedAckConnecting(CSegment* pParam);
	// close incoming channel in the recovery process
	void OnPartyChannelsDisconnectedChangeChannels(CSegment* pParam);
	void OnPartyChannelsUpdatedRecovery(CSegment* pParam);
	
	// open out channels
	void OnConfConnectCallRmtConnected(CSegment* pParam);
	void OnPartyChannelsConnectedOpenOut(CSegment* pParam);
	virtual void InternalRecoveryCompleted();	
	virtual void PartyConnectCall();

	void ContinueToCloseChannels();
	// timeouts
	virtual void OnConfBridgesConnectionTout(CSegment* pParam);
	virtual void OnUpdateBridgesTout(CSegment* pParam);
	
	void OnConfSetCapsAccordingToNewAllocation(CSegment* pParam);
	void OnPartySlavesRecapIsFinished(CSegment* pParam);
	void OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam);
	
	//Ice
	void OnIceInviteReceiveMakeOfferInd(CSegment* pParam);
	void OnICEOfferTimeout(CSegment* pParam);
	void OnIceInviteProcessAnsArrivedFromIceStack(CSegment* pParam);
	void OnICETimeout(CSegment* pParam);
	void IceConnectivityCheckComplete(CSegment* pParam);
	void OnIceInviteModifyAnsArrivedFromIceStack(CSegment* pParam);
	void HandleReinviteforICE(CSegment* pParam);
	void OnIceReinviteContentArrivedFromIceStack(CSegment* pParam);
	void OnICEReceiveCloseIceIndWaitForCandidates(CSegment* pParam);
	void OnIcePortsRetryTout(CSegment* pParam);
	void OnICEReceiveCloseIceInd(CSegment* pParam);


protected:
	PDECLAR_MESSAGE_MAP

	BYTE m_bIsToCloseVideoChannels;
	BYTE m_bIsToCloseDataChannels;
	BYTE m_bIsToCloseBfcpChannels;
};

#endif /*SIPTRANSINVITENOSDPIND_H_*/
