// ClientLoggerProcess.h: interface for the CClientLoggerProcess class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_ClientLoggerPROCESS_H__)
#define _ClientLoggerPROCESS_H__

#include "ProcessBase.h"
#include "OsQueue.h"
#include "LoggerDefines.h"
#include <map>

class CSegment;


#define EXCEPTION_FILE_NAME		"exceptions_client_logger.txt"
class CClientLoggerProcess : public CProcessBase  
{
CLASS_TYPE_1(CClientLoggerProcess,CProcessBase )
public:
	friend class CTestClientLoggerProcess;

	CClientLoggerProcess();
	virtual ~CClientLoggerProcess();
	virtual eProcessType GetProcessType() {return eProcessClientLogger;}
	virtual BOOL UsingSockets() {return NO;}

	virtual TaskEntryPoint GetManagerEntryPoint(){return 0;}

	virtual int Run();
	virtual int SetUp();

private:	
	void Subscribe();
	void UnSubcribe();
	void HandleLoggerDispatch();
	STATUS SendCommand(CSegment *seg, eProcessType destinationProcess, OPCODE opcode);
	int parseFiltersFromCmdLine();
	int parseProcess(char del,std::string strProcess);
	int parseTraceLevels(char del,std::string strLevels);
	void buildFilesPaths();
	void removeOldLogFiles();
	void logToFile(int iProcessIndex,const char *log);
	void logExceptionToFile(const char *log);
	eProcessType getProcessType(const char* processName );
	STATUS UpdateLogLevel(eLogLevel new_level,eProcessType iProcess);
	STATUS UpdateAllProcessLogLevel(eLogLevel new_level);
	int nextFilter(std::string& filterInput,int index,std::string& token);
	bool isHelp();
	COsQueue m_TerminalMbx;  // mailbox of the dual task (tx)
	char m_UniqueName[MAX_QUEUE_NAME_LEN];
	char m_Buffer[MAX_TRACE_SIZE];
	eProcessType m_processFilters[NUM_OF_PROCESS_TYPES];
	eLogLevel    m_FiltersTraceLevels[NUM_OF_PROCESS_TYPES];
	int m_numProcess;
	int m_validFilter;
	bool m_isTarget;
	bool m_isAllProcess;
	bool m_isTraceLevel;
	bool m_printToScreen[NUM_OF_PROCESS_TYPES];
	std::string  m_processFiles[NUM_OF_PROCESS_TYPES];
	std::string  m_exceptionFile;
	int m_exceptionFileSize;
	long m_processFilesSize[NUM_OF_PROCESS_TYPES];
	std::map<std::string,eLogLevel> m_traceMap;
};


#endif // !defined(_ClientLoggerPROCESS_H__)
