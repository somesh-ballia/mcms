//+========================================================================+
//                     SimPcmRxSocket.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SimPcmRxSocket.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __SIMPCMRXSOCKET_H__
#define   __SIMPCMRXSOCKET_H__


// base class definition
#include "SocketRxTask.h"
#include "SocketApi.h"

//class CMplMcmsProtocol;


// task entry point
extern "C" void SimPcmRxEntryPoint(void* appParam);


class CSimPcmRxSocket : public CSocketRxTask
{
CLASS_TYPE_1(CSimPcmRxSocket,CSocketRxTask )
public:

				// Constructors
	CSimPcmRxSocket();
	virtual ~CSimPcmRxSocket();
	
				// Initializations
//	virtual void  Create(CSegment& appParam);
	void* GetMessageMap(); 
//	void  SetLoginConfirm();

				// Operations
	const char* NameOf() const {return "CSimPcmRxSocket";}
	const char * GetTaskName() const;

//	virtual void HandleDisconnect();
	virtual void ReceiveFromSocket();

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



#endif /* __SIMPCMRXSOCKET_H__ */
