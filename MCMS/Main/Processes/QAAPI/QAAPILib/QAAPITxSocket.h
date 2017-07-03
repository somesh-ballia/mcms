//+========================================================================+
//                    SimCardTxSocket.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimCardTxSocket.h                                           |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __QAAPITXSOCKET_H__
#define   __QAAPITXSOCKET_H__

#include <string>

// base class definition
#include "SocketTxTask.h"
#include "httpBldr.h"



// task entry point
extern "C" void QAAPITxEntryPoint(void* appParam);


class CQAAPITxSocket : public CSocketTxTask ,public CHTTPHeaderBuilder 
{
CLASS_TYPE_1(CQAAPITxSocket,CSocketTxTask )
public:

				// Constructors
	CQAAPITxSocket(COsSocketConnected * pSocketDesc = NULL);
	virtual const char* NameOf() const { return "CQAAPITxSocket";}
	virtual ~CQAAPITxSocket();

				// Initializations
//	virtual void  Create(CSegment& appParam);
	void* GetMessageMap(); 

				// Operations
	const char * GetTaskName() const;

//	virtual void HandleDisconnect();

protected:
	// base overriding
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();

	// Operations
	
	// Action functions
	void OnWriteSocketAnycase(CSegment* pParam);


	PDECLAR_MESSAGE_MAP

private:
    void AssertSendFailure(const char *str, STATUS status)const;
};


#endif /* __QAAPITXSOCKET_H__ */
