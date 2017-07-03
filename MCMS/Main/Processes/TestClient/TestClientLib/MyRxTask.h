// MyRxTask.h: interface for the CMyRxTask class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_MYRXTASK_H__)
#define _MYRXTASK_H__

#include "SocketRxTask.h"

extern "C" void MyRxTaskEntryPoint(void* appParam);

class CMyRxTask : public CSocketRxTask  
{
CLASS_TYPE_1(CMyRxTask,CSocketRxTask )
public:
	CMyRxTask();
	virtual ~CMyRxTask();
	const char * GetTaskName() const {return "MyRxTask";}
	void InitTask();
	BOOL  IsSingleton() const {return NO;}

	void HandleDisconnect();
	void ReceiveFromSocket();

};

#endif // !defined(_MYRXTASK_H__)
