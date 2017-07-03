//+========================================================================+
//                            OsSocketListener.h                           |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       OsSocketListener.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:                                                             |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 3/8/95     |                                                      |
//+========================================================================+


#ifndef __OSSOCKETLISTNER_H
#define __OSSOCKETLISTNER_H

#include "DataTypes.h"
#include "OsSocketConnected.h"

class COsSocketListener
{

public:
	COsSocketListener(WORD port);
	~COsSocketListener();
	void Close();
	STATUS ConfigureListenSocket(DWORD remoteIp,
                                 const char* deviceName = NULL);// = INADDR_ANY);
	STATUS Listen(TICKS timeout, COsSocketConnected & connected);
	STATUS Accept(COsSocketConnected & connected);
    STATUS Reject(COsSocketConnected & connected);
    STATUS Close(COsSocketConnected & connected);
    STATUS   CloseSocket(int s);
    
	WORD m_portNum;
	int m_descriptor;

private:

public:		

	
};


#endif /* __OSSOCKETLISTNER_H */
