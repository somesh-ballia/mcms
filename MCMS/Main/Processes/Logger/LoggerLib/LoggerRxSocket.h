// LoggerRxSocket.h

#ifndef LOGGERRXSOCKET_H_
#define LOGGERRXSOCKET_H_

#include "SocketRxTask.h"
#include "MplMcmsStructs.h"

const DWORD TRACE_BUFFER_LEN = 1024 * 100;

extern "C" void LoggerSocketRxEntryPoint(void* appParam);

class CLoggerProcess;
class CMplMcmsProtocol;

class CLoggerRxSocket : public CSocketRxTask
{
CLASS_TYPE_1(CLoggerRxSocket, CSocketRxTask)
public:
	CLoggerRxSocket();
	virtual const char* GetTaskName()const;
	virtual const char* NameOf() const { return "CLoggerRxSocket";}
  
	void InitTask();
	virtual BOOL IsSingleton() const {return NO;}

	void ReceiveFromSocket();
    
private:
    void PrintCorruptedTraceOncePerStatus(CMplMcmsProtocol &prot, STATUS errorStatus, const char *errorMsg);
    void PrintCorruptedTrace(CMplMcmsProtocol &prot, STATUS errorStatus);
    void PrintTpktHeader(const TPKT_HEADER_S &tpkt);
    
    void FixTaskName(const CMplMcmsProtocol & prot);
    STATUS ValidateHeaders(const CMplMcmsProtocol &prot);
    STATUS ValidateCommonHeader(const COMMON_HEADER_S & commonHeader)const;
    STATUS ValidateTraceHeader(const TRACE_HEADER_S & traceHeader, DWORD dataLen)const;

	BYTE m_TraceBuffer[TRACE_BUFFER_LEN + 1];
	std::string m_TaskName;
	bool m_IsNameFixed;
	CLoggerProcess* m_proc;
};

#endif  // LOGGERRXSOCKET_H_
