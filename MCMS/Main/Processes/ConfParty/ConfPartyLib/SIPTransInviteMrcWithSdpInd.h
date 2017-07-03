//+========================================================================+
//               SIPTransInviteMrcWithSdpInd.h 				          	   |
//            Copyright 2012 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteMrcWithSdpInd.h                           	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef SIPTRANSINVITEMRCWITHSDPIND_H_
#define SIPTRANSINVITEMRCWITHSDPIND_H_

#include "SIPTransInviteMrcInd.h"


class CSipTransInviteMrcWithSdpInd : public CSipTransInviteMrcInd
{
public:
	CSipTransInviteMrcWithSdpInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransInviteMrcWithSdpInd();
	virtual const char* NameOf() const {return "CSipTransInviteMrcWithSdpInd";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }

	// Events Handles:

	// Establish Call
	void OnPartyEstablishCallIdle(CSegment* pParam);
	void OnPartyReceivedAck200OkNoMediaSent(CSegment* pParam);

	void OnPartyReceivedReinviteResponseConnecting(CSegment* pParam);
	// party control response with bridges connected (channels connected state)

	void SipInviteAckReqIfNeeded();
	void ContinueHandleBridgeConnectedInd();

    void OnPartyChannelsDisconnectedBeforeProcessAnswer(CSegment* pParam);

protected:
	PDECLAR_MESSAGE_MAP
};

#endif /* SIPTRANSINVITEMRCWITHSDPIND_H_ */
