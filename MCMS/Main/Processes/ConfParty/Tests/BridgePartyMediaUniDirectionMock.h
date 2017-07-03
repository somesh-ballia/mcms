//+========================================================================+
//              BridgePartyMediaUniDirectionMock.h                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       BridgePartyMediaUniDirectionMock.h	                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#ifndef _TEST_BRIDGEPARTY_MEDIA_UNIDERECTION_MOCK_H__
#define _TEST_BRIDGEPARTY_MEDIA_UNIDERECTION_MOCK_H__

#include "BridgePartyMediaUniDirection.h"

class CBridgePartyMediaUniDirectionMock : public CBridgePartyMediaUniDirection
{
public:

	virtual const char*	NameOf () const { return "CBridgePartyMediaUniDirectionMock"; }
	virtual void	HandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode) {}

	virtual void	Connect()	{}
	virtual void	DisConnect() {}

	virtual BOOL	IsConnected() { return  FALSE; }
	virtual BOOL    IsConnecting() {return FALSE; }
	virtual BOOL	IsDisConnected() { return  FALSE; }
	virtual BOOL    IsDisconnecting() {return  FALSE; }


	CBridgePartyCntl*	GetBridgePartyCntl() {return m_pBridgePartyCntl;}
	CHardwareInterface* GetHardwareInterface() {return m_pHardwareInterface;}
};


#endif /* _TEST_BRIDGEPARTY_MEDIA_UNIDERECTION_MOCK_H__ */

