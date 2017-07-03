// MyTxTask.h: interface for the CMyTxTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MYTXTASK_H__)
#define _MYTXTASK_H__

#include "SocketTxTask.h"

extern "C" void MyTxTaskEntryPoint(void* appParam);

class CMyTxTask : public CSocketTxTask  
{
CLASS_TYPE_1(CMyTxTask,CSocketTxTask )
public:
	CMyTxTask();
	virtual ~CMyTxTask();
	const char * GetTaskName() const {return "MyTxTask";}
	void InitTask();
	BOOL  IsSingleton() const {return NO;}

	void HandleDisconnect();


	void OnTxTimer(CSegment* pMsg);
	BOOL TaskHandleEvent(CSegment *pMsg,DWORD msgLen,OPCODE opCode);

	void*  GetMessageMap();
	
	PDECLAR_MESSAGE_MAP	

};

#endif // !defined(_MYTXTASK_H__)
