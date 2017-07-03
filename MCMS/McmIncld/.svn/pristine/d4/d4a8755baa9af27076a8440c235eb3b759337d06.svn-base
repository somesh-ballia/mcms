// TraceHeader.h: Interfaces for CTraceHeader struct
//
//////////////////////////////////////////////////////////////////////

#if !defined(_TRACEHEADER_H__)
#define _TRACEHEADER_H__

#include "DataTypes.h"
#include "MplMcmsStructs.h"



#define MAX_TASK_NAME_LEN    32
#define MAX_OBJECT_NAME_LEN  32
#define MAX_TERMINAL_NAME_LEN 32

#define INVALID				 	0xFFFFFFFF
#define DEFAULT_TOPIC_ID		0xFFFFFFFF
#define DEFAULT_UNIT_ID		 	0xFFFFFFFF
#define DEFAULT_CONF_ID		 	0xFFFFFFFF
#define DEFAULT_PARTY_ID		0xFFFFFFFF
#define DEFAULT_OPCODE			0xFFFFFFFF
#define DEFAULT_SOURCE_ID		0xFFFFFFFF
#define DEFAULT_CONNECTION_ID	0xFFFFFFFF


enum eLogLevel
{
        eLevelOff          = 0,
        eLevelFatal        = 1,  //System crash, conference crash etc'. NULL pointer for example. (p0 jira issue)
        eLevelError        = 10, //Error which is not fatal but still a bug (p1,p2,p3,… jira issues)
        eLevelWarn         = 20, //Not a bug but has to be taken into consideration. High CPU usage for example.
        eLevelInfoHigh     = 30, //MPLAPI, Inter process and other high importance messages.
        eLevelInfoNormal   = 50, //Minimal debugging information
        eLevelDebug        = 70, //Debug messages (additional information on field)
        eLevelTrace        = 100 //Trace messages (mostly for lab)
};

typedef struct 	 
{
    APIU32 m_processMessageNumber;
	APIU32 m_systemTick;
        APIU32 m_tv_sec;
        APIU32 m_tv_usec;
	APIU32 m_processType; // enum eProcessType
	//DWORD taskId;
	APIU32 m_level;
	APIU32 m_sourceId;		// pointer to a sender object.
	APIU32 m_messageLen;	// length of a content
	char   m_taskName	[MAX_TASK_NAME_LEN];
    char   m_objectName	[MAX_OBJECT_NAME_LEN];
	APIU32 m_topic_id; 
	
	APIU32 m_unit_id;
	APIU32 m_conf_id;
	APIU32 m_party_id;
	APIU32 m_opcode;
	char m_str_opcode [STR_OPCODE_LEN];
	
	// for mcms internal use.
	char   m_terminalName[MAX_TERMINAL_NAME_LEN];
} TRACE_HEADER_S;

#endif //_TRACEHEADER_H__
