// TraceClass.h

#ifndef TRACE_CLASS_H_
#define TRACE_CLASS_H_

#include <string>
#include <map>
#include "Macros.h"
#include "TraceHeader.h" 
#include "MplMcmsProtocol.h"
class CPObject;
#define MAX__FILTER_FILE_SIZE 10485760 //10 MB =1024*1024*10

static const DWORD NUM_OF_TRACE_TYPES = 256;
static const DWORD STR_LEN_OF_TRACE_TYPE = 256;
const DWORD MAX_FILE_LINE_NUMBER_LEN = 256;
#define LOCAL_TRACER_PATH	((std::string)(MCU_MCMS_DIR+"/LocalTracer/"))
struct MCMS_TRACE_HEADER_S
{
    char m_file_line_number[MAX_FILE_LINE_NUMBER_LEN];
};

class CTrace
{
public:
    static void OutMessage(const TRACE_HEADER_S& traceHeader,
                           const MCMS_TRACE_HEADER_S& mcmsHeader,
                           const std::string& content);

	static const char* GetTraceLevelNameByValue(int index);
	static const char* GetTraceLevelShortNameByValue(int index);
	static DWORD GetTraceLevelByName(const char *traceLevelName);
	static void FillTraceLevelNames(std::ostream& answer);
	static bool IsTraceLevelValid(DWORD level);
	static void BuildOutMessage(std::ostringstream& buf,CMplMcmsProtocol& prot);
	static void  BuildEmaMessage(TRACE_HEADER_S* pTraceHeader,const char* content, std::ostringstream& buf);
    static void TraceOutMessage(std::ostringstream& buf,CMplMcmsProtocol& prot);
    static void TraceEMAOutMessage(std::ostringstream& buf,const TRACE_HEADER_S& traceHeader);
private:

	static int InitTrace_Level_Names(void);

	// used for init the arrays
	static int Init;
    static long m_localFileSize;
    static void logToFile(std::string& processName,const char *log);
    static void logToFileEntity(std::string& entityName,const char *log);
    //keep copy to save time on regenerating titles
    static COMMON_HEADER_S			m_prev_commonHeader;
    static PHYSICAL_INFO_HEADER_S	m_prev_physicalHeader;
    static TRACE_HEADER_S			m_prev_traceHeader;
    static MCMS_TRACE_HEADER_S		m_prev_mcmsHeader;
    static std::string				m_title;
    static std::map<std::string,long>      m_filesEntitiesSize;
	// sparse arrays of trace level names and short names
	static char TRACE_LEVEL_NAMES [NUM_OF_TRACE_TYPES][STR_LEN_OF_TRACE_TYPE];
	static char TRACE_LEVEL_SHORT_NAMES [NUM_OF_TRACE_TYPES][STR_LEN_OF_TRACE_TYPE];
    static void TraceOutGenericMessage(std::ostringstream& buf,
    								  const COMMON_HEADER_S& commonHeader
    								  ,const TRACE_HEADER_S& traceHeader);
	static void BuildMcmsMessage(std::ostringstream& buf,
			    const TRACE_HEADER_S& traceHeader,
	            const MCMS_TRACE_HEADER_S& mcmsHeader,
	            const char* content);
    static void BuildGenericMessage(std::ostringstream& buf,
            const COMMON_HEADER_S& commonHeader,
            const PHYSICAL_INFO_HEADER_S& physicalHeader,
            TRACE_HEADER_S& traceHeader,
            const MCMS_TRACE_HEADER_S& mcmsHeader,
            const char* content);
    static void  FillBuffer(std::ostringstream& buf,
                            const COMMON_HEADER_S& commonHeader,
                            const PHYSICAL_INFO_HEADER_S& physicalHeader,
                            const TRACE_HEADER_S& traceHeader,
                            const MCMS_TRACE_HEADER_S& mcmsHeader,
                            const char* content);

    static void   GenerateTitle(const COMMON_HEADER_S& commonHeader,
                                const TRACE_HEADER_S& traceHeader,
                                std::string& out);
	static const char* GetProcessNameByMainEntity(eMainEntities mainEntityType,
                                              DWORD processType);
	static  BOOL  CheckIfNewHeaderIsNeeded(const COMMON_HEADER_S& commonHeader,
	                                                  const PHYSICAL_INFO_HEADER_S&
	                                                  physicalHeader,
	                                                  const TRACE_HEADER_S&
	                                                  traceHeader,
	                                                  const MCMS_TRACE_HEADER_S&
	                                                  mcmsHeader);
	CTrace(void);
	~CTrace(void);
	DISALLOW_COPY_AND_ASSIGN(CTrace);

};

#endif  // TRACE_CLASS_H_
