//+========================================================================+
//                     PartyConnectionMock.h.	                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PartyConnectionMock.h                                       |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Talya                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 2/2/06   |                                                        |
//+========================================================================+
#ifndef _PARTYCONNECTIONMOCK_H__
#define _PARTYCONNECTIONMOCK_H__

#include "PartyConnection.h"
#include "PartyCntlMock.h"

class CPartyConnectionMock : public CPartyConnection
{
public: 
	void CreateMock(DWORD partyRsrcId, CTaskApp* pParty, char* name ) 
	{
		m_pPartyCntl = new CPartyCntlMock; 
		((CPartyCntlMock*)m_pPartyCntl)->CreateMock(partyRsrcId, pParty, name);
	}

};


#endif //_PARTYCONNECTIONMOCK_H__
