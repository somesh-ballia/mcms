//+========================================================================+
//                SIPTransReInviteNoSdpInd.h               	   			   |
//            Copyright 2008 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransReInviteNoSdpInd.h                             	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
#ifndef SIPTRANSREINVITENOSDPIND_H_
#define SIPTRANSREINVITENOSDPIND_H_

class CSipTransReInviteNoSdpInd : public CSipTransaction
{ 
CLASS_TYPE_1(CSipTransReInviteNoSdpInd, CSipTransaction)

public:
	CSipTransReInviteNoSdpInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransReInviteNoSdpInd();
	virtual const char* NameOf() const {return "CSipTransReInviteNoSdpInd";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }
	//state machine functions
	// receive re-invite
	void OnPartyReceivedReInviteConnected(CSegment* pParam);
	// receive remote ack
	void OnPartyReceivedReInviteAck(CSegment* pParam);
	// close not supported channels
	void OnConfBridgesUpdatedUpdateBridges(CSegment* pParam);
	void OnPartyChannelsDisconnectedRecReInviteCloseChann(CSegment* pParam);
	void OnPartyChannelsUpdatedRecReInviteUpdateChann(CSegment* pParam);
	void OnPartyChannelsOpenRecReInviteOpenChann(CSegment* pParam);
			
	// for new transaction during handle of the previous one
	void OnPartyReceivedReInviteAnycase(CSegment* pParam);
	void OnPartyReceivedReInviteAckAnycase(CSegment* pParam);
	// timeout
	void OnReInviteUpdateBridgesTout(CSegment* pParam);
	void OnConfSetCapsAccordingToNewAllocation(CSegment* pParam);
	void OnPartySlavesRecapIsFinished(CSegment* pParam);
	void OnDtlsChannelsUpdated();

	void OnPartyRecStatisticInfo(CSegment* pParam);
	void OnDtlsClosedChannelBeforeSipCloseChannels(CSegment* pParam);
			
protected:
	PDECLAR_MESSAGE_MAP
};

#endif /*SIPTRANSREINVITENOSDPIND_H_*/
