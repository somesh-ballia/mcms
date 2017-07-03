//+========================================================================+
//                            CSApiTxSocket.H                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CSApiTxSocket.H                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Shlomit                                                       |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 9/5/95     |                                                      |
//+========================================================================+

#ifndef _CSAPITXSOCKET__H_
#define _CSAPITXSOCKET__H_


#include "SocketTxTask.h"

extern "C" void CSApiSocketTxEntryPoint(void* appParam);

class CCSApiTxSocket : public CSocketTxTask  
{
CLASS_TYPE_1(CCSApiTxSocket,CSocketTxTask )
public:
	CCSApiTxSocket();
	virtual ~CCSApiTxSocket();
	const char * GetTaskName() const {return "CSApiTxSocket";}
	const char * NameOf(void) const {return "CCSApiTxSocket";}
	void InitTask();
	BOOL  IsSingleton() const {return NO;}

	void HandleDisconnect();

	BOOL TaskHandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);
	void SendToSocket(CSegment& paramSegment);

	void*  GetMessageMap();
	
	PDECLAR_MESSAGE_MAP	

private:
	STATUS FillBinarySegment(CSegment &outSeg, TPKT_HEADER_S &outTPKT, CSegment &inSeg);
	STATUS FillXMLSegment(CSegment &outSeg, TPKT_HEADER_S &outTPKT, CSegment &inSeg);
	virtual void AddFilterOpcodePoint();
};

#endif /* _CSAPITXSOCKET__H_ */
