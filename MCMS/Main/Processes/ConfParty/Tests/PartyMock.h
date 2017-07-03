//+========================================================================+
//                     PartyMock.h                                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PartyMock.h	                                               |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Matvey                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | June-2005  |                                                      |
//+========================================================================+


#ifndef _PARTYMOCK_H__
#define _PARTYMOCK_H__

#include "Party.h"

class CPartyMock : public CParty
{
public:
	void CreateMock() {
		eProcessType processType = eProcessConfParty;
		
		char uniqueName[MAX_QUEUE_NAME_LEN];
		const char * taskName = NULL;
		
		COsQueue::CreateUniqueQueueName(uniqueName);
		taskName = uniqueName;
		
		m_pRcvMbxRead = new COsQueue;
		m_pRcvMbx     = new COsQueue;	
	}
};

#endif /* _PARTYMOCK_H__ */

