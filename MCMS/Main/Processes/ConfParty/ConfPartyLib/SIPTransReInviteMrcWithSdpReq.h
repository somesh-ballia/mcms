//+========================================================================+
//                SIPTransReInviteMrcWithSdpReq.h                      	   |
//            Copyright 2014 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: Add the new class for FSN-613: content support in SVC only  and SVC/AVC       | 
//           mixed conference to the level supported in AVC only conference.                       |                
//-------------------------------------------------------------------------|
// FILE:       SIPTransReInviteMrcWithSdpReq.h                            	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Jason Zhu                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSREINVITEMRCWITHSDPREQ_H_
#define SIPTRANSREINVITEMRCWITHSDPREQ_H_

class CSipTransReInviteMrcWithSdpReq : public CSipTransaction
{
CLASS_TYPE_1(CSipTransReInviteMrcWithSdpReq, CSipTransaction)

public:
	CSipTransReInviteMrcWithSdpReq(CTaskApp * pOwnerTask);
	virtual ~CSipTransReInviteMrcWithSdpReq();
	virtual const char* NameOf() const {return "CSipTransReInviteMrcWithSdpReq";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	void OnPartySendReInviteIdle(CSegment* pParam);
	
	BYTE IsNeedToUpdateIceStack();
	void ContinueToCloseChannelsIfNeeded();

	// open out channels state for dial out calls.
	void OnPartyChannelsConnected(CSegment * pParam);
	void OnPartyChannelsUpdatedInitReinvite(CSegment * pParam);
	void OnPartyChannelsConnectedOpenInChannels(CSegment* pParam);

	void OnPartyReceivedReInviteResponseInitReinvite(CSegment * pParam);

	void OnPartyRecStatisticInfo(CSegment* pParam);
	void OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam);
	// re invite update in state after the party has receive the 200 OK for the re-invite (I think this will change nothing - need to check).
	void OnPartyChannelsCloseReInviteCloseChannels(CSegment* pParam);
	void OnPartyChannelsUpdatedChannels(CSegment* pParam);

	void SendInviteAckIfNeeded();
	void OnConfBridgesUpdatedUpdateBridges(CSegment* pParam);
	
	//void OnConfChangeModeWaitForFixContent(CSegment* pParam);

	/*
	// connecting state (send invite until 200 OK response)
	

	

	void OnConfBridgesUpdatedUpdateBridges(CSegment* pParam);

	
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

    virtual void RequiredChannelsActionDone(DWORD opcode);

protected:
    virtual bool ShouldKeepTargetTxStreams() {return true;}*/

	BYTE	m_bIsWaitForPendingAck;
	CapEnum m_oldContentProtocol;  // save in this parameter the content protocol, in order to check if conference changed protocol during the transaction.

	BYTE	m_bIsAllChannelsAreConnected;

	PDECLAR_MESSAGE_MAP
};

#endif /*SIPTRANSREINVITEMRCWITHSDPREQ_H_*/
