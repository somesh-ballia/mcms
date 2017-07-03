//+========================================================================+
//                     BridgePartyCntlMock.h                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyCntlMock.h	                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#ifndef _BRIDGEPARTYCNTLMOCK_H__
#define _BRIDGEPARTYCNTLMOCK_H__

#include "BridgePartyCntl.h"

class CBridgePartyCntlMock : public CBridgePartyCntl
{
public:
	virtual ~CBridgePartyCntlMock () {}
	CBridgePartyCntlMock () : CBridgePartyCntl() {}
	CBridgePartyCntlMock(const void* pParty, const char* partyName, PartyRsrcID partyId): CBridgePartyCntl(pParty, partyName, partyId) {}
	CBridgePartyCntlMock (const CBridgePartyCntlMock& rBridgePartyCntl): CBridgePartyCntl(rBridgePartyCntl) {}
	CBridgePartyCntlMock&	operator= (const CBridgePartyCntlMock& rOther) { (CBridgePartyCntl&)(*this) = (CBridgePartyCntl&)rOther; return *this; }

	virtual const char*	NameOf () const { return "CBridgePartyCntlMock"; }

	void SetPartyRsrcID(PartyRsrcID partyRsrcID) {m_partyRsrcID = partyRsrcID;}
	CBridge* GetBridgePtr() {return m_pBridge;}

	CBridgePartyMediaUniDirection*	GetBridgePartyMediaIn() { return m_pBridgePartyIn; }
	CBridgePartyMediaUniDirection*	GetBridgePartyMediaOut() { return m_pBridgePartyOut; }
	CPartyApi*	GetPartyApi() { return m_pPartyApi; }
	CConfApi*	GetConfApi() { return m_pConfApi; }

};

#endif /* _BRIDGEPARTYCNTLMOCK_H__ */
