//+========================================================================+
//               SIPTransInviteMrcNoSdpInd.h 				          	   |
//            Copyright 2012 Polycom Israel Ltd.		                   |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       SIPTransInviteMrcNoSdpInd.h                           	   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef SIPTRANSINVITEMRCNOSDPIND_H_
#define SIPTRANSINVITEMRCNOSDPIND_H_

#include "SIPTransInviteMrcInd.h"


class CSipTransInviteMrcNoSdpInd : public CSipTransInviteMrcInd
{
public:
	CSipTransInviteMrcNoSdpInd(CTaskApp * pOwnerTask);
	virtual ~CSipTransInviteMrcNoSdpInd();
	virtual const char* NameOf() const {return "CSipTransInviteMrcNoSdpInd";}
	virtual void* GetMessageMap() { return (void*)m_msgEntries; }

	// Events Handles:

	// Establish Call
	void OnPartyEstablishCallIdle(CSegment* pParam);

	void OnPartyReceivedAckConnecting(CSegment* pParam);
	// party control response with bridges connected (channels connected state)

	void ContinueHandleBridgeConnectedInd();


protected:
	PDECLAR_MESSAGE_MAP
};

#endif /* SIPTRANSINVITEMRCNOSDPIND_H_ */
