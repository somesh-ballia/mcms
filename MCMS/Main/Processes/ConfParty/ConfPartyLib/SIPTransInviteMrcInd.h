//+========================================================================+
//               SIPTransInviteMrcInd.h 				          	   |
//            Copyright 2012 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteMrcInd.h                           	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef SIPTRANSINVITEMRCIND_H_
#define SIPTRANSINVITEMRCIND_H_

class CSipTransInviteMrcInd : public CSipTransaction
{
public:
	CSipTransInviteMrcInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransInviteMrcInd();
	virtual const char* NameOf() const {return "CSipTransInviteMrcInd";}

	// Events Handles:


	void InitPartyEstablishCallIdle();
	void ContinueToCloseChannels();

	// party control response with bridges connected (channels connected state)
	void OnPartyChannelsDisconnectedChangeChannels(CSegment* pParam);
	void OnPartyChannelsUpdatedRecovery(CSegment* pParam);
	void InternalRecoveryCompleted();
	void OnConfConnectCallRmtConnected(CSegment* pParam);
	void OnPartyChannelsConnectedOpenOut(CSegment* pParam);
	void OnConfBridgesConnectionTout(CSegment* pParam);
	void OnUpdateBridgesTout(CSegment* pParam);

	void ContinueOnPartyReceivedReinviteResponseOrAckConnecting(); // in WithSdp receiving 'ReinviteResponse'; in NoSdp receiving 'Ack'

	void PartyConnectCall();
	virtual void SipInviteAckReqIfNeeded();

	void HandleBridgeConnectedInd(DWORD status);
	virtual void ContinueHandleBridgeConnectedInd() {};

	// Open in channels before invite state
	void OnPartyChannelsConnectedOpenIn(CSegment *pParam);

	// Ice
    void OnIceReinviteContentArrivedFromIceStack(CSegment  *pParam); // y
    void OnIceInviteModifyAnsArrivedFromIceStack(CSegment  *pParam);
    void OnIceInviteProcessAnsArrivedFromIceStack(CSegment *pParam);
	void OnIceInviteReceiveMakeOfferInd(CSegment *pParam);

    void OnICETimeout(CSegment *pParam);
    void OnICEOfferTimeout(CSegment *pParam);
    void OnICEReceiveCloseIceInd (CSegment *pParam);
    void OnICEReceiveCloseIceIndWaitForCandidates(CSegment *pParam);
    void OnIcePortsRetryTout(CSegment *pParam);
    void IceConnectivityCheckComplete(CSegment *pParam);
    void HandleReinviteforICE(CSegment *pParam);



protected:
    BYTE m_bIsCloseVideoChannels;
	BYTE m_bIsCloseDataChannels;
	BYTE m_bIsCloseBfcpChannels;  //FSN-613: Dynamic Content for SVC/Mix Conf
};

#endif /* SIPTRANSINVITEMRCIND_H_ */
