//+========================================================================+
//                            OPRTR.H                                      |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       OPRTR.H                                                     |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Itai                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 9/5/95     |                                                      |
//+========================================================================+

#ifndef _MPLAPITXSOCKET__H_
#define _MPLAPITXSOCKET__H_


#include "SocketTxTask.h"

extern "C" void MplApiSocketTxEntryPoint(void* appParam);

class CMplApiTxSocket : public CSocketTxTask  
{
CLASS_TYPE_1(CMplApiTxSocket,CSocketTxTask )
public:
	CMplApiTxSocket();
	virtual ~CMplApiTxSocket();
	const char * GetTaskName() const {return "MplApiTxSocket";}
	virtual const char* NameOf() const { return "CMplApiTxSocket";}
	void InitTask();
	BOOL  IsSingleton() const {return NO;}

	void HandleDisconnect();

	BOOL TaskHandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	void SendToSocket(CSegment& paramSegment);
	
	void*  GetMessageMap();
	
	virtual void AddFilterOpcodePoint();

  virtual int GetTaskMbxBufferSize() const {return 4096 * 1024 - 1;}
	
	PDECLAR_MESSAGE_MAP	

};

#endif /* _MPLAPITXSOCKET__H_ */
