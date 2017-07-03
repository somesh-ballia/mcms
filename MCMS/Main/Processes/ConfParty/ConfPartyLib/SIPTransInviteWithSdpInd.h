//+========================================================================+
//                 SIPTransInviteWithSdpInd.h               	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteWithSdpInd.h                             	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSINVITEWITHSDPIND_H_
#define SIPTRANSINVITEWITHSDPIND_H_

class CSipTransInviteWithSdpInd : public CSipTransaction
{
CLASS_TYPE_1(CSipTransInviteWithSdpInd, CSipTransaction)

public:
	CSipTransInviteWithSdpInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransInviteWithSdpInd();
	virtual void* GetMessageMap() { return (void*)m_msgEntries;}
	virtual const char* NameOf() const {return "CSipTransInviteWithSdpInd";}
	//state wait for conf init call
	virtual void OnPartyEstablishCallIdle(CSegment* pParam);
	// open channels state
	void OnPartyChannelsConnectedOpenChannels(CSegment * pParam);
	void OnPartyChannelsConnectedOpenBridges(CSegment * pParam);
	// open bridge state
	virtual void HandleBridgeConnectedInd(DWORD status);
	// party connecting state
	void OnPartyReceivedAckConnecting(CSegment* pParam);
	virtual void OnConfBridgesConnectionTout(CSegment* pParam);

	void OnIceInviteMakeAnsArrivedFromIceStack(CSegment * pParam);
	void OnICECompleteTout(CSegment * pParam);
	void OnICEGeneralToutWaitForCandidates(CSegment * pParam);
	void IceConnectivityCheckComplete(CSegment * pParam);
	void OnICEReceiveCloseIceInd(CSegment * pParam);
	void OnIcePortsRetryTout(CSegment* pParam);
	void RemoveBfcpForTIPDialInWithSDP();
	void PartyConnectCall();

	void ContinueToOpenChannels();

	void OnPartySlavesRecapIsFinished(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP

	virtual BOOL MakeANewCallOnPartyEstablishCallIdle(CSipComMode * pBestMode) {return TRUE;}
	void DoMakeANewCallOnPartyEstablishCallIdle(CSipComMode * pBestMode);
};

#endif /*SIPTRANSINVITEWITHSDPIND_H_*/
