//+========================================================================+
//                     EpGuiTxSocket.h                                     |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpGuiTxSocket.h                                             |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __EPGUITXSOCKET_H__
#define   __EPGUITXSOCKET_H__


// base class definition
#include "SocketTxTask.h"


// task entry point
extern "C" void EpGuiTxEntryPoint(void* appParam);


class CEpGuiTxSocket : public CSocketTxTask
{
CLASS_TYPE_1(CEpGuiTxSocket,CSocketTxTask )
public:

				// Constructors
	CEpGuiTxSocket();
	virtual const char* NameOf() const { return "CEpGuiTxSocket";}
	virtual ~CEpGuiTxSocket();

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

				// Attributes

	PDECLAR_MESSAGE_MAP
};


#endif /* __EPGUITXSOCKET_H__ */
