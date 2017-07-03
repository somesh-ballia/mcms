// Trace.h

#ifndef TRACE_H_
#define TRACE_H_

#include <string>
#include <iostream>
#include <algorithm>

#include "Macros.h"
#include "DataTypes.h"
#include "TraceHeader.h"
#include "SharedDefines.h"

class CPObject;

void OutTraceMessage(const char* file_name,
                     const WORD lineNum,
                     const WORD level,
                     const CPObject* pObj,
                     const char* message1,
                     const char* message2,
                     const DWORD topic_id = DEFAULT_TOPIC_ID,
                     const char* terminalName = NULL,
                     const DWORD unit_id = DEFAULT_UNIT_ID,
                     const DWORD conf_id = DEFAULT_CONF_ID,
                     const DWORD party_id = DEFAULT_PARTY_ID,
                     const OPCODE opcode = DEFAULT_OPCODE,
                     const char* str_opcode = NULL);

void OutTraceMessageD(const char* file_name,
                      const WORD lineNum,
                      const WORD level,
                      const CPObject* pObj,
                      const char* message,
                      DWORD status);

void OutTraceMessageH(const char* file_name,
                      const WORD lineNum,
                      const WORD level,
                      const CPObject* pObj,
                      const char* message,
                      DWORD status);

void OutTraceCommandAnswer(const char* file_name,
                           const WORD lineNum,
                           const CPObject* pObj,
                           const char* terminalName,
                           const char* commandName,
                           const char* answer);

void OutTraceOpcode(const char* file_name,
                    const WORD lineNum,
                    const WORD level,
                    const CPObject* pObj,
                    const char* message1,
                    const OPCODE opcode,
                    const char* strOpcode);

void PAssert(const char* fname,
             const WORD lineNum,
             const CPObject* pObj,
             DWORD errCode,
             const char* message,
             BYTE debug = NO);

void PAssertDebug(const char* fname,
                  const WORD lineNum,
                  const CPObject* pObj,
                  DWORD errCode);

void PBtTrace(const char* fileName,
              const WORD lineNum,
              const CPObject * pObj,
              const char * traceMessage);

void ProxyTraceThroughUtility(const char* file_name,
                              const WORD lineNum,
                              const WORD level,
                              const char* message1);

void ExceptionTrace(const char* fname,
             const WORD lineNum,
             const CPObject* pObj,
             DWORD errCode,
             const char* message);

std::string GetTimeOfDay();
std::string TimeOfDayToString(timeval& tv);

#define PBTTRACESTREAM_HELPER(cond, msg, ptr) \
  if (cond) \
  { \
    COstrStream _buf_; \
    _buf_ << msg; \
    PBtTrace(__FILE__, __LINE__, ptr, _buf_.str().c_str()); \
  } \
  else \
    (void)0

#define PBTTRACE(cond, msg) ((cond) ? PBtTrace(__FILE__, __LINE__, this, msg) : (void)0)
#define FPBTTRACE(cond, msg) ((cond) ? PBtTrace(__FILE__, __LINE__, NULL, msg) : (void)0)

#define PBTTRACESTREAM(cond, msg) PBTTRACESTREAM_HELPER(cond, msg, this)
#define FPBTTRACESTREAM(cond, msg) PBTTRACESTREAM_HELPER(cond, msg, NULL)

// Asserts

#define PASSERTMSG_HELPER(cond, msg, ptr) \
  ((cond) ? PAssert(__FILE__, __LINE__, ptr, cond, msg) : (void)0)

#define PASSERTMSG(cond, msg) PASSERTMSG_HELPER(cond, msg, this)
#define FPASSERTMSG(cond, msg) PASSERTMSG_HELPER(cond, msg, NULL)
#define PASSERT(cond) PASSERTMSG(cond, NULL)
#define FPASSERT(cond) FPASSERTMSG(cond, NULL)

#define PASSERTMSG_AND_RETURN_HELPER(cond, msg, ptr)\
{\
	if (cond)\
	{\
		PAssert(__FILE__, __LINE__, ptr, cond, msg);\
		return;\
	}\
}

#define PASSERTMSG_AND_RETURN(cond, msg) \
  PASSERTMSG_AND_RETURN_HELPER(cond, msg, this)

#define FPASSERTMSG_AND_RETURN(cond, msg) \
  PASSERTMSG_AND_RETURN_HELPER(cond, msg, NULL)

#define PASSERT_AND_RETURN(cond) \
  PASSERTMSG_AND_RETURN(cond, NULL)

#define FPASSERT_AND_RETURN(cond) \
  FPASSERTMSG_AND_RETURN(cond, NULL)

#define PASSERTMSG_AND_RETURN_VALUE_HELPER(cond, msg, ret, ptr)\
{\
	if (cond)\
	{\
		PAssert(__FILE__, __LINE__, ptr, cond, msg);\
		return ret;\
	}\
}

#define PASSERTMSG_AND_RETURN_VALUE(cond, msg, ret) \
  PASSERTMSG_AND_RETURN_VALUE_HELPER(cond, msg, ret, this)

#define FPASSERTMSG_AND_RETURN_VALUE(cond, msg, ret) \
  PASSERTMSG_AND_RETURN_VALUE_HELPER(cond, msg, ret, NULL)

#define PASSERT_AND_RETURN_VALUE(cond, ret) \
  PASSERTMSG_AND_RETURN_VALUE(cond, NULL, ret)

#define FPASSERT_AND_RETURN_VALUE(cond, ret) \
  FPASSERTMSG_AND_RETURN_VALUE(cond, NULL, ret)

#define PASSERTSTREAM_HELPER(cond, msg, ptr) \
  if (cond) \
  { \
    COstrStream _buf_; \
    _buf_ << msg; \
    PAssert(__FILE__, __LINE__, ptr, cond, _buf_.str().c_str()); \
  } \
  else \
    (void)0

#define EXCEPTION_TRACE(cond, msg) \
  if (cond) \
  { \
    COstrStream _buf_; \
    _buf_ << msg; \
    ExceptionTrace(__FILE__, __LINE__, this, cond, _buf_.str().c_str()); \
  } \
  else \
    (void)0

#define FEXCEPTION_TRACE(cond, msg) \
  if (cond) \
  { \
    COstrStream _buf_; \
    _buf_ << msg; \
    ExceptionTrace(__FILE__, __LINE__, NULL, cond, _buf_.str().c_str()); \
  } \
  else \
    (void)0

#define EXCEPTION_TRACE1(cond)   EXCEPTION_TRACE(cond,"")
#define FEXCEPTION_TRACE1(cond)   FEXCEPTION_TRACE(cond,"")

#define PASSERTSTREAM(cond, msg) PASSERTSTREAM_HELPER(cond, msg, this)
#define FPASSERTSTREAM(cond, msg) PASSERTSTREAM_HELPER(cond, msg, NULL)

#define PASSERTSTREAM_AND_RETURN_HELPER(cond, msg, ptr) \
	{\
		if (cond)\
		{\
			COstrStream _buf_;\
			_buf_ << msg;\
			PAssert(__FILE__, __LINE__, ptr, cond, _buf_.str().c_str());\
			return;\
		}\
	}

#define PASSERTSTREAM_AND_RETURN(cond, msg) \
    PASSERTSTREAM_AND_RETURN_HELPER(cond, msg, this)

#define FPASSERTSTREAM_AND_RETURN(cond, msg) \
	PASSERTSTREAM_AND_RETURN_HELPER(cond, msg, NULL)

#define PASSERTSTREAM_AND_RETURN_VALUE_HELPER(cond, msg, ret, ptr)\
	{\
		if (cond)\
		{\
			COstrStream _buf_;\
			_buf_ << msg;\
			PAssert(__FILE__, __LINE__, ptr, cond, _buf_.str().c_str());\
			return ret;\
		}\
	}

#define PASSERTSTREAM_AND_RETURN_VALUE(cond, msg, ret) \
	PASSERTSTREAM_AND_RETURN_VALUE_HELPER(cond, msg, ret, this)

#define FPASSERTSTREAM_AND_RETURN_VALUE(cond, msg, ret) \
	PASSERTSTREAM_AND_RETURN_VALUE_HELPER(cond, msg, ret, NULL)

#define PASSERTMSGONCE_HELPER(cond, msg, ptr) \
	{\
		if (cond)\
		{\
			static bool once = true;\
			if (once)\
			{\
				once = false;\
				PAssert(__FILE__, __LINE__, ptr, cond, msg);\
			}\
		}\
	}

#define PASSERTMSGONCE(cond, msg)  PASSERTMSGONCE_HELPER(cond, msg, this)
#define FPASSERTMSGONCE(cond, msg) PASSERTMSGONCE_HELPER(cond, msg, NULL)
#define PASSERTONCE(cond)          PASSERTMSGONCE(cond, NULL)
#define FPASSERTONCE(cond)         FPASSERTMSGONCE(cond, NULL)
#define DBGPASSERT(a)              ((a) ? PAssertDebug(__FILE__,__LINE__,this,a) : (void)(0))
#define DBGFPASSERT(a)             ((a) ? PAssertDebug(__FILE__,__LINE__,NULL,a) : (void)(0))
#define DBGPASSERT_AND_RETURN(a)   if(a) { PAssertDebug(__FILE__,__LINE__,this,a); return;} else (void)(0)

#define TRACESTR_HELPER(level, ptr) \
	(CTraceStream(__FILE__, __LINE__, level, ptr).seekp(0, std::ios_base::cur))

// GCC treats __func__ as a variable and fills with a function name
// during compilation, which is part of the C99 standard
#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

// Cuts signature of a C++ function off return value and parameters, e.g.:
// 'std::vector Foo::foo(std::string) const' will be trimmed to 'Foo::foo'.
inline std::string trim_pretty_function(const char* first, const char* last)
{
	// Looks for open brace
	const char* brc = std::find(first, last, '(');
	if (last == brc)
		return std::string(first, last);

	// Looks for preceded space
	const char* spe = brc-1;
	for (; spe > first; --spe)
	{
		if (' ' == *spe)
		{
			++spe;
			break;
		}
	}

	// Trims from space till open brace
	return std::string(spe, brc);
}

#define PRETTY_FUNCTION (trim_pretty_function(__PRETTY_FUNCTION__, ARRAYEND(__PRETTY_FUNCTION__)))

#define TRACESTRFUNC_HELPER(level, ptr)\
	TRACESTR_HELPER(level, ptr) << trim_pretty_function(__PRETTY_FUNCTION__, ARRAYEND(__PRETTY_FUNCTION__)) << " - "

#define TRACESTRWARN_HELPER(ptr)\
	TRACESTRFUNC_HELPER(eLevelError, ptr) << "WARNING: "

#define TRACESTR(level)      TRACESTR_HELPER(level, this)
#define FTRACESTR(level)     TRACESTR_HELPER(level, NULL)
#define TRACESTRFUNC(level)  TRACESTRFUNC_HELPER(level, this)
#define FTRACESTRFUNC(level) TRACESTRFUNC_HELPER(level, NULL)
#define TRACEINTO            TRACESTRFUNC(eLevelInfoNormal)
#define TRACEINTOLVLERR      TRACESTRFUNC(eLevelError)
#define FTRACEINTO           FTRACESTRFUNC(eLevelInfoNormal)
#define FTRACEDEBUG          FTRACESTRFUNC(eLevelDebug)
#define TRACEINTOFUNC        TRACESTRFUNC(eLevelInfoNormal)   // Deprecated, use TRACEINTO
#define FTRACEINTOFUNC       FTRACESTRFUNC(eLevelInfoNormal)  // Deprecated, use FTRACEINTO
#define TRACEWARN            TRACESTRWARN_HELPER(this)
#define FTRACEWARN           TRACESTRWARN_HELPER(NULL)

#define TRACECOND_HELPER(cond, msg, level, ptr)\
	{\
		if (cond)\
			TRACESTRFUNC_HELPER(level, ptr) << msg;\
	}

#define TRACECOND_AND_RETURN_HELPER(cond, msg, level, ptr)\
	{\
		if (cond)\
		{ \
			TRACESTRFUNC_HELPER(level, ptr) << msg;\
			return;\
		}\
	}

#define TRACECOND_AND_RETURN_VALUE_HELPER(cond, msg, ret, level, ptr)\
	{\
		if (cond)\
		{ \
			TRACESTRFUNC_HELPER(level, ptr) << msg;\
			return ret;\
		}\
	}

#define TRACECOND(cond, msg)  TRACECOND_HELPER(cond, msg, eLevelInfoNormal, this)
#define FTRACECOND(cond, msg) TRACECOND_HELPER(cond, msg, eLevelInfoNormal, NULL)

#define TRACECOND_AND_RETURN(cond, msg) \
  TRACECOND_AND_RETURN_HELPER(cond, msg, eLevelInfoNormal, this)
#define FTRACECOND_AND_RETURN(cond, msg) \
  TRACECOND_AND_RETURN_HELPER(cond, msg, eLevelInfoNormal, NULL)
#define TRACECOND_AND_RETURN_VALUE(cond, msg, ret) \
  TRACECOND_AND_RETURN_VALUE_HELPER(cond, msg, ret, eLevelInfoNormal, this)
#define FTRACECOND_AND_RETURN_VALUE(cond, msg, ret) \
  TRACECOND_AND_RETURN_VALUE_HELPER(cond, msg, ret, eLevelInfoNormal, NULL)

#define PTRACE(level, message1)                 OutTraceMessage(__FILE__, __LINE__, level, this, message1, NULL)
#define PTRACE1(level, message1)                OutTraceMessage(__FILE__, __LINE__, level, NULL, message1, NULL)
#define PTRACE2(level, message1, message2)      OutTraceMessage(__FILE__, __LINE__, level, this, message1, message2)
#define PTRACE2INT(level, message1, message2)   OutTraceMessageD(__FILE__, __LINE__, level, this, message1, message2)
#define PTRACE2HINT(level, message1, message2)  OutTraceMessageH(__FILE__, __LINE__, level, this, message1, message2)
#define PTRACE2INT2(level, message1, message2)  OutTraceMessage(__FILE__, __LINE__, level, this, message1, message2)
#define FPTRACE(level, message1)                OutTraceMessage(__FILE__, __LINE__, level, NULL, message1, NULL)
#define FPTRACE2(level, message1, message2)     OutTraceMessage(__FILE__, __LINE__, level, NULL, message1, message2)
#define FPTRACE2INT(level, message1, message2)  OutTraceMessageD(__FILE__, __LINE__, level, NULL, message1, message2)
#define FPTRACE2HINT(level, message1, message2) OutTraceMessageH(__FILE__, __LINE__, level, NULL, message1, message2)

#define PTRACETOPIC(level, message1, message2)             OutTraceMessage(__FILE__, __LINE__, level, this, message1, NULL, message2)
#define PTRACECOMMAND(termName, commName, answer)          OutTraceCommandAnswer(__FILE__, __LINE__, this, termName, commName, answer)
#define PTRACEOPCODE(level, message1, opcode, strOpcode)   OutTraceOpcode(__FILE__, __LINE__, level, this, message1, opcode, strOpcode)
#define PTRACEPARTYID(level, message1, partyID)            OutTraceMessage(__FILE__, __LINE__, level, this, message1, NULL, DEFAULT_TOPIC_ID, NULL, DEFAULT_UNIT_ID, DEFAULT_CONF_ID, partyID)
#define PTRACE2PARTYID(level, message1, message2, partyID) OutTraceMessage(__FILE__, __LINE__, level, this, message1, message2, DEFAULT_TOPIC_ID, NULL, DEFAULT_UNIT_ID, DEFAULT_CONF_ID, partyID)
#define PTRACE2COND(cond, level, message1, message2)       if(cond) { OutTraceMessage(__FILE__, __LINE__, level, this, message1, message2); }
#define PTRACE2INTCOND(cond, level, message1, message2)    if(cond) { OutTraceMessageD(__FILE__, __LINE__, level, this, message1, message2); }

#define DUMPSTR(str) (((str != 0) && (str[0] != '\0')) ? str : "NA")

#define MESSAGE_QUEUE_BLOCKING_RESERCH
#define LOOKUP_TABLE_DEBUG_TRACE
#define MONITOR_PARTY_ID
#define MS_LYNC_AVMCU_LINK

#define _GLA_
#ifdef _GLA_
#define TRACEINTO_GLA TRACESTR_HELPER(eLevelInfoNormal, this) << "_GLA_ " << GetTimeOfDay() << " - " << trim_pretty_function(__PRETTY_FUNCTION__, ARRAYEND(__PRETTY_FUNCTION__)) << " - "
#define FTRACEINTO_GLA TRACESTR_HELPER(eLevelInfoNormal, NULL) << "_GLA_ " << GetTimeOfDay() << " - " << trim_pretty_function(__PRETTY_FUNCTION__, ARRAYEND(__PRETTY_FUNCTION__)) << " - "
#else
#define TRACEINTO_GLA if(false) TRACESTR_HELPER(eLevelInfoNormal, this)
#define FTRACEINTO_GLA if(false) TRACESTR_HELPER(eLevelInfoNormal, NULL)
#endif

#define PRFFORMANCE_ANALYSIS

#endif
