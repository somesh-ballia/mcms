#include "EMATrace.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "InitCommonStrings.h"
#include "StringsMaps.h"
#include "ApiStatuses.h"

CEMATrace::CEMATrace()
{
	m_TraceHeader.m_processMessageNumber = INVALID;
	m_TraceHeader.m_systemTick = INVALID;
	m_TraceHeader.m_processType = INVALID;
	m_TraceHeader.m_level = eLevelInfoHigh;
	m_TraceHeader.m_sourceId = DEFAULT_SOURCE_ID;
	m_TraceHeader.m_messageLen = INVALID;	
	m_TraceHeader.m_unit_id = DEFAULT_UNIT_ID;
	m_TraceHeader.m_conf_id = DEFAULT_CONF_ID;
	m_TraceHeader.m_party_id = DEFAULT_PARTY_ID;
	m_TraceHeader.m_opcode = DEFAULT_OPCODE;
	m_TraceHeader.m_topic_id = INVALID;
	bzero(m_TraceHeader.m_taskName, sizeof(m_TraceHeader.m_taskName));
	bzero(m_TraceHeader.m_objectName, sizeof(m_TraceHeader.m_objectName));
	bzero(m_TraceHeader.m_str_opcode, sizeof(m_TraceHeader.m_str_opcode));
	bzero(m_TraceHeader.m_terminalName, sizeof(m_TraceHeader.m_terminalName));

	m_Content					[0] = '\0';
}

CEMATrace::~CEMATrace()
{}


void CEMATrace::SerializeXml(CXMLDOMElement*& thisNode) const
{
	PASSERTMSG(1, "this function should not be called");
}


int CEMATrace::DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action)
{
	int nStatus = STATUS_OK;
	
	CXMLDOMElement *pTraceNode;
	GET_MANDATORY_CHILD_NODE(pNode, "TRACE", pTraceNode);
	
	GET_VALIDATE_CHILD(pTraceNode, "OPCODE_NAME", 			m_TraceHeader.m_str_opcode, 			_0_TO_MAX_OPCODE_NAME_LENGTH);		
	GET_VALIDATE_CHILD(pTraceNode, "CONTENT", 				m_Content, 								_0_TO_MAX_CONTENT_LENGTH);
	GET_VALIDATE_CHILD(pTraceNode, "TASK_NAME", 			m_TraceHeader.m_taskName, 				_0_TO_MAX_TASK_NAME_LENGTH);		
	GET_VALIDATE_CHILD(pTraceNode, "OBJECT_NAME", 			m_TraceHeader.m_objectName, 			_0_TO_MAX_OBJECT_NAME_LENGTH);
	
	GET_VALIDATE_CHILD(pTraceNode, "PROCESS_TYPE", 			&m_TraceHeader.m_processType, 			EMA_PROCESSES_ENUM);
	GET_VALIDATE_CHILD(pTraceNode, "LEVEL", 				&m_TraceHeader.m_level, 				TRACE_LEVEL_ENUM);
	
	GET_VALIDATE_CHILD(pTraceNode, "PROCESS_MESSAGE_NUMBER",&m_TraceHeader.m_processMessageNumber, 	_0_TO_DWORD);
	GET_VALIDATE_CHILD(pTraceNode, "SYSTEM_TICK", 			&m_TraceHeader.m_systemTick, 			_0_TO_DWORD);
	GET_VALIDATE_CHILD(pTraceNode, "SOURCE_ID", 			&m_TraceHeader.m_sourceId, 				_0_TO_DWORD);		
	GET_VALIDATE_CHILD(pTraceNode, "TOPIC_ID", 				&m_TraceHeader.m_topic_id, 				_0_TO_DWORD);
	GET_VALIDATE_CHILD(pTraceNode, "UNIT_ID", 				&m_TraceHeader.m_unit_id, 				_0_TO_DWORD);
	GET_VALIDATE_CHILD(pTraceNode, "CONF_ID", 				&m_TraceHeader.m_conf_id, 				_0_TO_DWORD);		
	GET_VALIDATE_CHILD(pTraceNode, "PARTY_ID", 				&m_TraceHeader.m_party_id, 				_0_TO_DWORD);
	GET_VALIDATE_CHILD(pTraceNode, "OPCODE", 				&m_TraceHeader.m_opcode, 				_0_TO_DWORD);		

	m_TraceHeader.m_messageLen = strnlen(m_Content, MAX_CONTENT_SIZE);
	if (m_TraceHeader.m_messageLen == MAX_CONTENT_SIZE)
	    m_Content[MAX_CONTENT_SIZE - 1] = '\0';
	
	return nStatus;
}
