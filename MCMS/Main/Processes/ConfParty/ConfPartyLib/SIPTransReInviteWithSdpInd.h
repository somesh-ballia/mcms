//+========================================================================+
//               SIPTransReInviteWithSdpInd.h             	   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransReInviteWithSdpInd.h                           	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSREINVITEWITHSDPIND_H_
#define SIPTRANSREINVITEWITHSDPIND_H_

class CSipTransReInviteWithSdpInd : public CSipTransaction
{ 
CLASS_TYPE_1(CSipTransReInviteWithSdpInd, CSipTransaction)

public:
	CSipTransReInviteWithSdpInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransReInviteWithSdpInd();	
	virtual const char* NameOf() const {return "CSipTransReInviteWithSdpInd";}
	// State Machine functions
	void OnPartyReceivedReInviteConnected(CSegment* pParam);
	// re-invite bridges response
	void OnConfBridgesUpdatedUpdateBridges(CSegment* pParam);
	//Receive re-invite close channels state (sTRANS_RECREINVITECLOSECHANN)
	void OnPartyChannelsDisconnectedRecReInviteCloseChann(CSegment* pParam);
	// Receive re-invite update channels state (sTRANS_RECREINVITEUPDATECHANN)
	void OnPartyChannelsUpdatedRecReInviteUpdateChann(CSegment* pParam);
	// Receive re-invite open channels state (sTRANS_RECREINVITEOPENCHANN)
	void OnPartyChannelsOpenRecReInviteOpenChann(CSegment* pParam);
	// Receive Ack
	void OnPartyReceivedReInviteAck(CSegment * pParam);
	// Receive Ack for reject
	void OnPartyReceivedReInviteAckReInviteRejected(CSegment* pParam);
	// Send ReInvite Response
	void SendReInviteResponse();
	// Rollback transaction is case of transaction reject
	virtual void RollbackTransaction();
	// timeout
	void OnReInviteUpdateBridgesTout(CSegment* pParam);
	void OnConfSetCapsAccordingToNewAllocation(CSegment* pParam);
	
	virtual BYTE IsNeedToUpdateFlowControlInVideoBridge() {return TRUE;}; //Return true only if it is reinvite indication because then it is the remote who initiating the flow control

	void ContinueToCloseChannelsIfNeeded();
	void OnICEReceiveCloseIceInd(CSegment* pParam);
	//	void ContinueReinviteConnect();

	void OnICEReinviteGeneralTimeout(CSegment* pParam);
	void OnSipReceiveReinviteModifyAnswerInd(CSegment* pParam);
	void OnPartySlavesRecapIsFinished(CSegment* pParam);

	void OnPartyRecStatisticInfo(CSegment* pParam);
	void OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam);
	
    void OnPartyVideoArtDisconnected();
    void OnPartyTranslatorArtsConnected();
    void OnSipEndVideoUpgradeToMix(CSegment* pParam);
    void OnDtlsChannelsUpdated();
protected:

	DWORD 	m_retStatusForReject;  // return status after reject.
	BYTE	m_bRollbackNeeded;  // Is need to rollback transaction.
	BYTE	m_bIsAllChannelsAreOpenend;
	WORD    m_isFallBackFromTip;

	PDECLAR_MESSAGE_MAP
};

#endif /*SIPTRANSREINVITEWITHSDPIND_H_*/
