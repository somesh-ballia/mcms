//+========================================================================+
//                            MplApiRxSocket.h                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       MplApiRxSocket.h                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Itai                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 9/5/95     |                                                      |
//+========================================================================+


#ifndef _MPLAPIRXSOCKET_H__
#define _MPLAPIRXSOCKET_H__

#include "SocketRxTask.h"
#include "MplMcmsProtocol.h"

extern "C" void MplApiSocketRxEntryPoint(void* appParam);

class CMplApiRxSocketMngr;



class CMplApiRxSocket : public CSocketRxTask
{
CLASS_TYPE_1(CMplApiRxSocket,CSocketRxTask )
public:
	CMplApiRxSocket();
	virtual ~CMplApiRxSocket();
	const char * GetTaskName() const {return "MplApiRxSocket";}
	virtual const char* NameOf() const { return "CMplApiRxSocket";}
	void InitTask();
	BOOL  IsSingleton() const {return NO;}

	void HandleDisconnect();
	void ReceiveFromSocket();

  virtual int GetTaskMbxBufferSize() const {return 4096 * 1024 - 1;}

private:
    BOOL  IsKnownBoardID();
	void  SetKnownBoardID(BOOL knownBoardID);
	void UpdateBoardId(CMplMcmsProtocol &mplPrtcl);
	virtual void AddFilterOpcodePoint();
	
	
	BOOL m_IsKnownBoardID;
	CMplApiRxSocketMngr *m_pMplApiRxSocketMngr;
};

#endif /* _MPLAPIRXSOCKET_H__ */
