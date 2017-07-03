#include "LoggerTxSocket.h"
#include "Segment.h"
#include "TraceStream.h"


extern "C" void LoggerSocketTxEntryPoint(void* appParam)
{  	
	CLoggerTxSocket *pTaskApp = new CLoggerTxSocket;
	pTaskApp->Create(*(CSegment*)appParam);
	*(CSegment*)appParam << (void*)pTaskApp;
}


CLoggerTxSocket::CLoggerTxSocket()
{
}

CLoggerTxSocket::~CLoggerTxSocket()
{
}

void CLoggerTxSocket::InitTask()
{
	TRACEINTO_SOCKET << ".";
}
