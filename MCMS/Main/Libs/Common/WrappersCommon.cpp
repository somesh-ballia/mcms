// WrappersCommon.cpp

#include "WrappersCommon.h"

#include <iomanip>
#include <netinet/in.h>

#include "ProcessBase.h"

extern const char* IPv6ScopeIdToString(enScopeId theScopeId);

CCommonHeaderWrapper::CCommonHeaderWrapper(const COMMON_HEADER_S &data)
:m_Data(data)
{}

CCommonHeaderWrapper::~CCommonHeaderWrapper()
{}

void CCommonHeaderWrapper::Dump(std::ostream &os) const
{
    CProcessBase *pProcess = CProcessBase::GetProcess();
    const std::string &strOpcode = pProcess->GetStatusAsString(m_Data.opcode);
    
	DumpHeader(os, "Common Header::Dump");
	
	os << std::setw(20) << "Protocol Ver: "   		<< (WORD)m_Data.protocol_version 	   << std::endl;
	os << std::setw(20) << "Option: " 				<< (WORD)m_Data.option 				   << std::endl;
	os << std::setw(20) << "Source Id: " 			<< (WORD)m_Data.src_id 				   << std::endl;
	os << std::setw(20) << "Dest Id: "   			<< (WORD)m_Data.dest_id 			   << std::endl;
	os << std::setw(20) << "Opcode: " 				<< m_Data.opcode << " : " << strOpcode.c_str() << std::endl;
	os << std::setw(20) << "Time Stamp: " 			<< m_Data.time_stamp 				   << std::endl;
	os << std::setw(20) << "Sequence Num: " 		<< m_Data.sequence_num 				   << std::endl;
	os << std::setw(20) << "Payload Len: "   		<< m_Data.payload_len 				   << std::endl;
	os << std::setw(20) << "Payload Offset: " 		<< m_Data.payload_offset 			   << std::endl;
	os << std::setw(20) << "Next Header Type: "   	<< m_Data.next_header_type 			   << std::endl;
	os << std::setw(20) << "Next Header Offset: " 	<< m_Data.next_header_offset		   << std::endl;		
}

CTraceHeaderWrapper::CTraceHeaderWrapper(const TRACE_HEADER_S &data)
:m_Data(data)
{}

CTraceHeaderWrapper::~CTraceHeaderWrapper()
{}

void CTraceHeaderWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "Trace Header::Dump");

	os << std::setw(20) << "Process Message Num: "  << m_Data.m_processMessageNumber 	<< std::endl;
	os << std::setw(20) << "System Tick: " 			<< m_Data.m_systemTick				<< std::endl;
	os << std::setw(20) << "System sec: " 			<< m_Data.m_tv_sec				    << std::endl;
	os << std::setw(20) << "System usec: " 			<< m_Data.m_tv_usec				    << std::endl;
	os << std::setw(20) << "Process Type: " 		<< m_Data.m_processType				<< std::endl;
	os << std::setw(20) << "Level: " 				<< m_Data.m_level 					<< std::endl;
	os << std::setw(20) << "Source Id: "			<< m_Data.m_sourceId 				<< std::endl;
	os << std::setw(20) << "Message Len: " 			<< m_Data.m_messageLen 				<< std::endl;
	os << std::setw(20) << "Task Name: "   			<< m_Data.m_taskName 				<< std::endl;
	os << std::setw(20) << "Object Name: " 			<< m_Data.m_objectName				<< std::endl;
	os << std::setw(20) << "Topic Id: " 			<< m_Data.m_topic_id				<< std::endl;
	os << std::setw(20) << "Unit Id: " 				<< m_Data.m_unit_id 				<< std::endl;
	os << std::setw(20) << "Conf Id: "				<< m_Data.m_conf_id 				<< std::endl;
	os << std::setw(20) << "Party Id: " 			<< m_Data.m_party_id 				<< std::endl;
	os << std::setw(20) << "Opcode: "   			<< m_Data.m_opcode 					<< std::endl;
	os << std::setw(20) << "Str Opcode: " 			<< m_Data.m_str_opcode				<< std::endl;
	os << std::setw(20) << "Terminal Name: " 		<< m_Data.m_terminalName			<< std::endl;
}







/*-----------------------------------------------------------------------------
	class CAuditHeaderWrapper
-----------------------------------------------------------------------------*/
CAuditHeaderWrapper::CAuditHeaderWrapper(const AUDIT_EVENT_HEADER_S &data)
:m_Data(data)
{}

CAuditHeaderWrapper::~CAuditHeaderWrapper()
{}

void CAuditHeaderWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "Audit Header::Dump");

	os << std::setw(20) << "User Name: "            << m_Data.userIdName 	            << std::endl;
	os << std::setw(20) << "Report Module: "        << m_Data.reportModule				<< std::endl;
	os << std::setw(20) << "Work Station: " 		<< m_Data.workStation				<< std::endl;
	os << std::setw(20) << "Client Ip Address: "    << m_Data.ipAddress				    << std::endl;
	os << std::setw(20) << "Event Type: "			<< m_Data.eventType			        << std::endl;
	os << std::setw(20) << "status: " 			    << m_Data.status 			 	    << std::endl;
	os << std::setw(20) << "action: "   			<< m_Data.action                    << std::endl;
	os << std::setw(20) << "description: " 			<< m_Data.description				<< std::endl;
	os << std::setw(20) << "descriptionEx: "        << m_Data.descriptionEx				<< std::endl;
}







/*-----------------------------------------------------------------------------
	class CTPKTHeaderWrapper
-----------------------------------------------------------------------------*/
CTPKTHeaderWrapper::CTPKTHeaderWrapper(const TPKT_HEADER_S &data)
:m_Data(data)
{}

CTPKTHeaderWrapper::~CTPKTHeaderWrapper()
{}

void CTPKTHeaderWrapper::Dump(std::ostream &os) const
{
	DumpHeader(os, "TPKT Header::Dump");

	os << std::setw(20) << "Version Num: "  	<< (WORD)m_Data.version_num		<< std::endl;
	os << std::setw(20) << "Reserved: " 		<< (WORD)m_Data.reserved		<< std::endl;
	os << std::setw(20) << "Payload Len(BE): " 	<< (WORD)m_Data.payload_len		<< std::endl;
	
	WORD num = ntohs(m_Data.payload_len);
	os << std::setw(20) << "Payload Len(LE): " 	<< num							<< std::endl;
}
 
/*-----------------------------------------------------------------------------
	class CIPV4Wrapper
-----------------------------------------------------------------------------*/
CIPV4Wrapper::CIPV4Wrapper(ipAddressV4If &data)
:m_Data(data)
{
}

CIPV4Wrapper::~CIPV4Wrapper()
{
}

void CIPV4Wrapper::Dump(std::ostream &os) const
{
	char ip[16];
	SystemDWORDToIpString(m_Data.ip, ip);

	os << "ip: "     << ip;
}

void CIPV4Wrapper::NullData()
{
	m_Data.ip = 0;
}

void CIPV4Wrapper::CopyData(ipAddressV4If &Data)
{
	m_Data.ip = Data.ip;
}



/*-----------------------------------------------------------------------------
	class CIPV6AraryWrapper
-----------------------------------------------------------------------------*/
CIPV6AraryWrapper::CIPV6AraryWrapper(ipv6AddressArray& data)
:m_Data(data)
{
}

CIPV6AraryWrapper::~CIPV6AraryWrapper()
{
}

void CIPV6AraryWrapper::Dump(std::ostream &os) const
{
	mcTransportAddress tempAddr;
	for(int i=0; i<NUM_OF_IPV6_ADDRESSES; i++)
	{
		memset(&tempAddr,0,sizeof(mcTransportAddress));
		tempAddr.ipVersion = (DWORD)eIpVersion6;
		tempAddr.addr.v6.scopeId = m_Data[i].scopeId;
		memcpy(tempAddr.addr.v6.ip,m_Data[i].ip,sizeof(m_Data[i].ip));
		char szIP[64];
		::ipToString(tempAddr,szIP,1);

		os << "\n     " << i << " ip: " << szIP  << " scopeId: "     << IPv6ScopeIdToString((enScopeId)(m_Data[i].scopeId));
	}
}


void CIPV6AraryWrapper::CopyData(const ipv6AddressArray& data)
{
  memcpy(m_Data, data, sizeof(m_Data));
}

