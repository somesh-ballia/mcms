//+========================================================================+
//                 GideonSimLoggerRxSocket.h                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimLoggerRxSocket.h                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __GIDEONSIMLOGGERRXSOCKET_H__
#define   __GIDEONSIMLOGGERRXSOCKET_H__


// base class definition
#include "SocketRxTask.h"
#include "SocketApi.h"

//class CMplMcmsProtocol;


// task entry point
extern "C" void GideonSimLoggerRxEntryPoint(void* appParam);


class CGideonSimLoggerRxSocket : public CSocketRxTask
{
CLASS_TYPE_1(CGideonSimLoggerRxSocket,CSocketRxTask )
public:

				// Constructors
	CGideonSimLoggerRxSocket();
	virtual const char* NameOf() const { return "CGideonSimLoggerRxSocket";}
	virtual ~CGideonSimLoggerRxSocket();
	
				// Initializations
//	virtual void  Create(CSegment& appParam);
	void* GetMessageMap(); 
//	void  SetLoginConfirm();

				// Operations
	const char * GetTaskName() const;

//	virtual void HandleDisconnect();
	virtual void ReceiveFromSocket();

	BOOL IsValidTimer(OPCODE type);


protected:
				// base overriding
	BOOL         IsSingleton() const { return NO; }
	virtual void InitTask();
//	virtual void SelfKill();
//	void HandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode);
//	virtual void  WaitForEvent();

				// Operations
//	virtual void DispatchMplMsg(CMplMcmsProtocol& mplPrtcl);

				// Action functions
//	void OnLoginTimeOut(CSegment* pParam);

				// Attributes

	PDECLAR_MESSAGE_MAP
};



#endif /* __GIDEONSIMLOGGERRXSOCKET_H__ */
