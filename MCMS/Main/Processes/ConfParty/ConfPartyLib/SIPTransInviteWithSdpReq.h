//+========================================================================+
//                  SIPTransInviteWithSdpReq.h 				          	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteWithSdpReq.h                             	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSINVITEWITHSDPREQ_H_
#define SIPTRANSINVITEWITHSDPREQ_H_

class CSipTransInviteWithSdpReq : public CSipTransaction
{ 
CLASS_TYPE_1(CSipTransInviteWithSdpReq, CSipTransaction)

public:
	CSipTransInviteWithSdpReq(CTaskApp * pOwnerTask);
	virtual ~CSipTransInviteWithSdpReq();
	virtual const char* NameOf() const {return "CSipTransInviteWithSdpReq";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	void OnPartyEstablishCallIdle(CSegment* pParam);
	
	// Open in channels before invite state
	virtual void OnPartyChannelsConnectedOpenIn(CSegment * pParam);
		
	// party control response with bridges connected (channels connected state)
	virtual void HandleBridgeConnectedInd(DWORD status);
		
	// connecting state (send invite until 200 OK response)
	void OnPartyReceived200OkConnecting(CSegment * pParam);
	
	// re invite update in state after the party has receive the 200 OK for the re-invite (I think this will change nothing - need to check).
	void OnPartyChannelsDisconnectedChangeChannels(CSegment* pParam);
	void OnPartyChannelsUpdatedRecovery(CSegment* pParam);

	// open out channels state for dial out calls.
	void OnPartyChannelsConnectedOpenOut(CSegment * pParam);
	virtual void InternalRecoveryCompleted();
	
	// receive from the party control command to connect the call - send final Ack (remote connected state).
	virtual void OnConfConnectCallRmtConnected(CSegment * pParam);

	virtual void PartyConnectCall();
	
	virtual void OnPartyConnectToutConnecting(CSegment* pParam);
	virtual void OnConfBridgesConnectionTout(CSegment* pParam);
	virtual void OnUpdateBridgesTout(CSegment* pParam);
	
	void OnConfSetCapsAccordingToNewAllocation(CSegment* pParam);
    //Ice
    void ContinueToCloseChannels();
    void OnIceReinviteContentArrivedFromIceStack(CSegment* pParam);
    void OnIceInviteModifyAnsArrivedFromIceStack(CSegment* pParam);
    void OnICETimeout(CSegment* pParam);
    void OnICEOfferTimeout(CSegment* pParam);
    void OnIceInviteProcessAnsArrivedFromIceStack(CSegment* pParam);
    void OnIceInviteReceiveMakeOfferInd(CSegment* pParam);
    void IceConnectivityCheckComplete(CSegment * pParam);
    void HandleReinviteforICE(CSegment* pParam);

    void OnICEReceiveCloseIceInd (CSegment* pParam);
    void OnICEReceiveCloseIceIndWaitForCandidates(CSegment* pParam);

	void OnIcePortsRetryTout(CSegment* pParam);
    void OnPartySlavesRecapIsFinished(CSegment* pParam);
    void OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam);

    void OnPartyVideoArtDisconnected();
    virtual BOOL IsNeedToAddContentForFallbackFromTip() {return FALSE;}

protected:
	PDECLAR_MESSAGE_MAP
	BYTE m_bIsToCloseVideoChannels;
	BYTE m_bIsToCloseDataChannels;
	BYTE m_bIsToCloseBfcpChannels;
	BYTE m_bIsToUpdateAnatIpType; //added for ANAT

	DWORD m_vidRxRate;

    virtual void UserAgentAndVersionUpdated(const char* cUserAgent, const char* pVersionId);

    void HandleChannelsDisconnectedReopenChnlForAnat(CSegment* pParam); //added for ANAT
};

#endif /*SIPTRANSINVITEWITHSDPREQ_H_*/
