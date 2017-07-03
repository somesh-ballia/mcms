//+========================================================================+
//                            PairOfSockets.H                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       PairOfSockets.H                                             |
// SUBSYSTEM:  MCMSOPER                                                    |
// PROGRAMMER: Shlomit                                                     |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        

# include "OsQueue.h"
# include "PObject.h"
#include "OsSocketConnected.h"
#include "SocketApi.h"

#if !defined(_PAIR_OF_SOCKETS__)
#define _PAIR_OF_SOCKETS__



class CPairOfSockets  :public CPObject
{
CLASS_TYPE_1(CPairOfSockets,CPObject )		
public:             
	
	CPairOfSockets(const COsQueue& reciveSocketMailSlot,
					const COsQueue& transmitSocketMailSlot,
					int conId,
					const COsSocketConnected & connected);

	CPairOfSockets(const CPairOfSockets& other);
	CPairOfSockets();
	virtual const char* NameOf() const { return "CPairOfSockets";}
	void KillBoth();
	virtual ~CPairOfSockets();  
	void Serialize(CSegment& seg);
	void DeSerialize(CSegment& seg);
	void operator=(const CPairOfSockets& psocketTable);

	COsQueue* m_ReciveSocketMailSlot;
	COsQueue* m_TransmitSocketMailSlot;

	WORD m_conId;
	COsSocketConnected m_connected;

	DWORD rx_pid;
	DWORD tx_pid;
	

};
#endif

