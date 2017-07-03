#ifndef LOGGERDEFINES_H_
#define LOGGERDEFINES_H_

#include "TraceHeader.h"


const DWORD MAX_TRACE_SIZE 				= 1024 * 1024;
const DWORD MAX_CONTENT_SIZE 			= (MAX_TRACE_SIZE - sizeof(TRACE_HEADER_S) - 1);
const WORD LOGGER_LISTEN_SOCKET_PORT 	= 10009;
const DWORD NUM_TRACE_IN_BUFFER 		= 100;

#define TITLE_Date 				"D"
#define TITLE_Time 				"T"
#define TITLE_SysTick 			"TK"
#define TITLE_Level 			"L"
#define TITLE_MainEntity 		"E"
#define TITLE_ProcessName 		"P"
#define TITLE_SourceId 			"SID"
#define TITLE_TaskName 			"TN"
#define TITLE_ObjectName 		"O"
#define TITLE_TopicID	 		"TopicID"
#define TITLE_UnitID 			"UnitID"
#define TITLE_ConfID 			"ConfID"
#define TITLE_PartyID 			"PartyID"
#define TITLE_Opcode 			"Op"
#define TITLE_StrOpcode 		"StrOp"
#define TITLE_TerminalName		"TrmlN"
#define TITLE_FileLine   		"Lctn"
#define TITLE_BoardId           "BrdID"
#define TITLE_PortId            "PrtID"
#define TITLE_AcceleratorId     "AccID"

#define COMPRESSION_CODE_NONE	0
#define COMPRESSION_CODE_ZLIB	1

#define LOGGER_FILE_MAX_NAME_LEN       255
#define COMPRESSION_FORMAT_MAX_NAME_LEN 10
#define MAX_LOG_SIZE	          		16

#endif /*LOGGERDEFINES_H_*/
