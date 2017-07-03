#ifndef LOGGERTXSOCKET_H_
#define LOGGERTXSOCKET_H_

#include "SocketTxTask.h"


extern "C" void LoggerSocketTxEntryPoint(void* appParam);


class CLoggerTxSocket : public CSocketTxTask
{
CLASS_TYPE_1(CLoggerTxSocket,CSocketTxTask )	
public:
	CLoggerTxSocket();
	virtual ~CLoggerTxSocket();
	const char * GetTaskName() const {return "LoggerTxSocket";}
	virtual const char* NameOf() const { return "CLoggerTxSocket";}
	void InitTask();
	BOOL  IsSingleton() const {return NO;}
};

#endif /*LOGGERTXSOCKET_H_*/
