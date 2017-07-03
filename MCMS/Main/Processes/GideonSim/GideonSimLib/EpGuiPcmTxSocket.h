//+========================================================================+
//                     SimGuiTxSocket.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpGuiPcmTxSocket.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __EPGUIPCMTXSOCKET_H__
#define   __EPGUIPCMTXSOCKET_H__


// base class definition
#include "SocketTxTask.h"


// task entry point
extern "C" void EpGuiPcmTxEntryPoint(void* appParam);


class CEpGuiPcmTxSocket : public CSocketTxTask
{
CLASS_TYPE_1(CEpGuiPcmTxSocket,CSocketTxTask )
public:

				// Constructors
	CEpGuiPcmTxSocket();
	virtual ~CEpGuiPcmTxSocket();

				// Initializations
//	virtual void  Create(CSegment& appParam);
	void* GetMessageMap(); 

				// Operations
	const char* NameOf() const {return "CEpGuiPcmTxSocket";}
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


#endif /* __EPGUIPCMTXSOCKET_H__ */
