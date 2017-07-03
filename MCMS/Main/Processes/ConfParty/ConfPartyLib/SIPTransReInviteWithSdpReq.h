//+========================================================================+
//                SIPTransReInviteWithSdpReq.h                      	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransReInviteWithSdpReq.h                            	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: GuyD                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSREINVITEWITHSDPREQ_H_
#define SIPTRANSREINVITEWITHSDPREQ_H_

class CSipTransReInviteWithSdpReq : public CSipTransaction
{
CLASS_TYPE_1(CSipTransReInviteWithSdpReq, CSipTransaction)

public:
	CSipTransReInviteWithSdpReq(CTaskApp * pOwnerTask);
	virtual ~CSipTransReInviteWithSdpReq();
	virtual const char* NameOf() const {return "CSipTransReInviteWithSdpReq";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	void OnPartySendReInviteIdle(CSegment* pParam);

	// connecting state (send invite until 200 OK response)
	void OnPartyReceivedReInviteResponseInitReinvite(CSegment * pParam);

	// re invite update in state after the party has receive the 200 OK for the re-invite (I think this will change nothing - need to check).
	void OnPartyChannelsCloseReInviteCloseChannels(CSegment* pParam);
	void OnPartyChannelsUpdatedChannels(CSegment* pParam);

	void OnConfBridgesUpdatedUpdateBridges(CSegment* pParam);

	// open out channels state for dial out calls.
	void OnPartyChannelsConnected(CSegment * pParam);

	void OnPartyChannelsUpdatedInitReinvite(CSegment * pParam);
	void OnPartyChannelsConnectedOpenInChannels(CSegment* pParam);
	// timeout
	void OnReInviteUpdateBridgesTout(CSegment* pParam);
	void OnPartyConnectToutInitReinvite(CSegment* pParam);
	// Glare
	void OnPartyReceivedReInviteAnycase(CSegment* pParam);
	void OnPartyReceivedReInviteAckAnycase(CSegment* pParam);

	//ICE
	void OnIceInviteProcessAnsArrivedFromIceStack(CSegment* pParam);
	void OnIceInviteModifyAnsArrivedFromIceStack(CSegment* pParam);
	void ContinueToCloseChannelsIfNeeded();
	void ContinueReinviteConnect();
	void OnICETimeout(CSegment* pParam);
	BYTE IsNeedToUpdateIceStack();
	void OnICEModifyTimeout(CSegment* pParam);
	void OnICEReceiveCloseIceInd(CSegment* pParam);

	// ppc
//	void HandleContentScmAndCaps();

	void OnConfSetCapsAccordingToNewAllocation(CSegment* pParam);

	void OnPartySlavesRecapIsFinished(CSegment* pParam);

	void SendInviteAckIfNeeded();

	void OnPartyRecStatisticInfo(CSegment* pParam);
	void OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam);

    virtual void RequiredChannelsActionDone(DWORD opcode);

protected:
    virtual bool ShouldKeepTargetTxStreams() {return true;}

	BYTE	m_bIsWaitForPendingAck;
	//BYTE	m_bTransactionSetContentOn; // Used at the first reinvite-req in dial-out calls, that content is set to on by transaction if local and remote support bfcp
	CapEnum m_oldContentProtocol;  // save in this parameter the content protocol, in order to check if conference changed protocol during the transaction.

	BYTE	m_bIsAllChannelsAreConnected;

	PDECLAR_MESSAGE_MAP
};

#endif /*SIPTRANSREINVITEWITHSDPREQ_H_*/
