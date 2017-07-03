//+========================================================================+
//                     SimGuiRxSocket.h                                    |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       EpGuiPcmRxSocket.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef   __EPGUIPCMRXSOCKET_H__
#define   __EPGUIPCMRXSOCKET_H__


// base class definition
#include "SocketRxTask.h"
#include "SocketApi.h"

//class CMplMcmsProtocol;


// task entry point
extern "C" void EpGuiPcmRxEntryPoint(void* appParam);


class CEpGuiPcmRxSocket : public CSocketRxTask
{
CLASS_TYPE_1(CEpGuiPcmRxSocket,CSocketRxTask )
public:

				// Constructors
	CEpGuiPcmRxSocket();
	virtual ~CEpGuiPcmRxSocket();
	
				// Initializations
//	virtual void  Create(CSegment& appParam);
	void* GetMessageMap(); 
//	void  SetLoginConfirm();

				// Operations
	const char* NameOf() const {return "CEpGuiPcmRxSocket";}
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



#endif /* __EPGUIPCMRXSOCKET_H__ */
