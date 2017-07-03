#include "TcpDumpEntity.h"

#include "TraceStream.h"

#include "StringsMaps.h"
#include "psosxml.h"
#include "StatusesGeneral.h"
#include "ApiStatuses.h"
#include "SystemFunctions.h"
#include "StringsLen.h"
#include "UtilityProcess.h"
#include "OsFileIF.h"

#include <iomanip>

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntityList::InitMembers()
{
	for (size_t i = 0; i < ARRAYSIZE(m_pTcpDumpEntities); ++i)
		m_pTcpDumpEntities[i] = new CTcpDumpEntity;

	m_cyclic_storage = true;
	m_tcpDump_State = e_TcpDumpState_Idle;
	m_max_capture_size = e_MaxCaptureSize_none;
	m_max_capture_duration = e_MaxCaptureDuration_none;
	m_time_elapsed.m_hour = 0;
	m_time_elapsed.m_min = 0;
	m_time_elapsed.m_sec = 0;
	m_storage_in_used = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntityList::ClearMembers()
{
	for (size_t i = 0; i < ARRAYSIZE(m_pTcpDumpEntities); ++i)
		m_pTcpDumpEntities[i]->ClearMembers();

	m_cyclic_storage = true;
	m_tcpDump_State = e_TcpDumpState_Idle;
	m_max_capture_size = e_MaxCaptureSize_none;
	m_max_capture_duration = e_MaxCaptureDuration_none;
	m_time_elapsed.m_hour = 0;
	m_time_elapsed.m_min = 0;
	m_time_elapsed.m_sec = 0;
	m_storage_in_used = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityList::CTcpDumpEntityList()
{
	InitMembers();
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityList::CTcpDumpEntityList(const CTcpDumpEntityList& other)
	: CSerializeObject(other)
{
	for (size_t i = 0 ; i < ARRAYSIZE(m_pTcpDumpEntities); ++i)
		m_pTcpDumpEntities[i] = other.m_pTcpDumpEntities[i] ? new CTcpDumpEntity(*other.m_pTcpDumpEntities[i]) : NULL;

	m_tcpDump_State        = other.m_tcpDump_State;
	m_updateCounter        = other.m_updateCounter;
	m_cyclic_storage       = other.m_cyclic_storage;
	m_time_elapsed         = other.m_time_elapsed;
	m_storage_in_used      = other.m_storage_in_used;
	m_max_capture_size     = other.m_max_capture_size;
	m_max_capture_duration = other.m_max_capture_duration;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void  CTcpDumpEntityList::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg
		<< "\n===== CTcpDumpEntityList::Dump ====="
		<< "\nCyclic Storage "        << m_cyclic_storage
		<< "\nTcp Dump State "        << m_tcpDump_State
		<< "\n m_max_capture_size "   << m_max_capture_size
		<< "\nm_max_capture_duration "<< m_max_capture_duration
		<< "\nm_time_elapsed.m_hour " << m_time_elapsed.m_hour
		<< "\nm_time_elapsed.m_min "  <<m_time_elapsed.m_min
		<< "\nm_time_elapsed.m_sec "  <<m_time_elapsed.m_sec
		<< "\nm_storage_in_used "     <<m_storage_in_used;

	for (size_t i = 0; i < ARRAYSIZE(m_pTcpDumpEntities); ++i)
		msg << "\nTcp Dump Entry " << i << "\nENTRY   "<< *m_pTcpDumpEntities[i];
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityList&  CTcpDumpEntityList::operator=( const CTcpDumpEntityList& other )
{
	if (this == &other)
		return *this;

	for (size_t i = 0; i < ARRAYSIZE(m_pTcpDumpEntities); ++i)
	{
		POBJDELETE(m_pTcpDumpEntities[i]);
		m_pTcpDumpEntities[i] = other.m_pTcpDumpEntities[i] ? new CTcpDumpEntity(*other.m_pTcpDumpEntities[i]) : NULL;
	}

	m_updateCounter        = other.m_updateCounter;
	m_tcpDump_State        = other.m_tcpDump_State;
	m_cyclic_storage       = other.m_cyclic_storage;
	m_time_elapsed         = other.m_time_elapsed;
	m_storage_in_used      = other.m_storage_in_used;
	m_max_capture_size     = other.m_max_capture_size;
	m_max_capture_duration = other.m_max_capture_duration;

	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpEntityList::~CTcpDumpEntityList()
{
	for(size_t i = 0; i < ARRAYSIZE(m_pTcpDumpEntities); ++i)
		PDELETE(m_pTcpDumpEntities[i]);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntityList::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* ptcpDumpCfgNode;

	if (!pFatherNode)
	{
		pFatherNode = new CXMLDOMElement();
		pFatherNode->set_nodeName("TCP_DUMP_CFG");
		ptcpDumpCfgNode = pFatherNode;
	}
	else
	{
		ptcpDumpCfgNode = pFatherNode->AddChildNode("TCP_DUMP_CFG");
	}

	ptcpDumpCfgNode->AddChildNode("TCP_DUMP_STATE", m_tcpDump_State, TCP_DUMP_STATE_ENUM);
	CXMLDOMElement* pTcpDumpEntitiesList = ptcpDumpCfgNode->AddChildNode("TCP_DUMP_ENTITIES");

	for (int i = 0; i < 10; ++i)
		m_pTcpDumpEntities[i]->SerializeXml(pTcpDumpEntitiesList);

	ptcpDumpCfgNode->AddChildNode("CYCLIC_STORAGE", m_cyclic_storage);
	CXMLDOMElement* pTempNode=ptcpDumpCfgNode->AddChildNode("TIME_ELAPSED");

	pTempNode->AddChildNode("HOUR", m_time_elapsed.m_hour);
	pTempNode->AddChildNode("MINUTE", m_time_elapsed.m_min);
	pTempNode->AddChildNode("SECOND", m_time_elapsed.m_sec);

	ptcpDumpCfgNode->AddChildNode("STORAGE_IN_USED", m_storage_in_used);
	ptcpDumpCfgNode->AddChildNode("MAX_CAPTURE_SIZE", m_max_capture_size, MAX_CAPTURE_SIZE_ENUM);
	ptcpDumpCfgNode->AddChildNode("MAX_CAPTURE_DURATION", m_max_capture_duration, MAX_CAPTURE_DURATION_ENUM);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CTcpDumpEntityList::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	// schema file name:  obj_ip_srv_list.xsd

	int nStatus = STATUS_OK;
	CXMLDOMElement *pTcpDumpEntitiesList;

	GET_CHILD_NODE(pActionNode, "TCP_DUMP_ENTITIES", pTcpDumpEntitiesList);
	if (pTcpDumpEntitiesList)
	{
		CXMLDOMElement* pTcpDumpEntity;
		GET_FIRST_CHILD_NODE(pTcpDumpEntitiesList, "ENTITY", pTcpDumpEntity);

		size_t i = 0;
		while (pTcpDumpEntity)
		{
			m_pTcpDumpEntities[i]->DeSerializeXml(pTcpDumpEntity, pszError, action);

			GET_NEXT_CHILD_NODE(pTcpDumpEntitiesList,"ENTITY",pTcpDumpEntity);
			++i;
		}

		TRACEINTO << "END CTcpDumpEntityList::DeSerializeXml  i=" << i;
	}

	GET_VALIDATE_CHILD(pActionNode,"CYCLIC_STORAGE", &m_cyclic_storage,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MAX_CAPTURE_SIZE", (WORD*)&m_max_capture_size,MAX_CAPTURE_SIZE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"MAX_CAPTURE_DURATION", (WORD*)&m_max_capture_duration,MAX_CAPTURE_DURATION_ENUM);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpEntity::CTcpDumpEntity()
{
	InitMembers();
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpEntity::CTcpDumpEntity(const CTcpDumpEntity& other)
	: CSerializeObject(other)
{
	for (size_t i = 0 ; i < ARRAYSIZE(m_pIPList); ++i)
		m_pIPList[i] = other.m_pIPList[i] ? new CIPEntity(*other.m_pIPList[i]) : NULL;

	m_boardId = other.m_boardId;
	strncpy(m_filter, other.m_filter, ONE_LINE_BUFFER_LEN);
	m_entityType = other.m_entityType;
	m_tcpDumpState = other.m_tcpDumpState;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpEntity::~CTcpDumpEntity()
{
	for (size_t i = 0 ; i < ARRAYSIZE(m_pIPList); ++i)
		delete m_pIPList[i];
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpEntity& CTcpDumpEntity::operator =(const CTcpDumpEntity& other)
{
	if (this == &other)
		return *this;

	for (size_t i = 0 ; i < ARRAYSIZE(m_pIPList); ++i)
	{
		POBJDELETE(m_pIPList[i]);
		m_pIPList[i] = other.m_pIPList[i] ? new CIPEntity(*other.m_pIPList[i]) : NULL;
	}

	strncpy(m_filter, other.m_filter, ONE_LINE_BUFFER_LEN);
	m_entityType = other.m_entityType;
	m_boardId = other.m_boardId;

	m_tcpDumpState = other.m_tcpDumpState;

	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntity::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* ptcpDumpEntityNode = pParentNode->AddChildNode("ENTITY");

	ptcpDumpEntityNode->AddChildNode("BOARD_ID", m_boardId);
	ptcpDumpEntityNode->AddChildNode("ENTITY_TYPE", m_entityType, ENTITY_TYPE_ENUM);
	ptcpDumpEntityNode->AddChildNode("FILTER",  m_filter);

	CXMLDOMElement* pIpListNode = ptcpDumpEntityNode->AddChildNode("IP_LIST");
	for (size_t i = 0; i < 5; ++i)
		m_pIPList[i]->SerializeXml(pIpListNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int  CTcpDumpEntity::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	int nStatus = STATUS_OK;

	if (pActionNode)
	{
		GET_VALIDATE_CHILD(pActionNode, "BOARD_ID", &m_boardId, _0_TO_DWORD);
		TRACEINTO << "m_boardId:" << m_boardId;

		DWORD entityType = 0;
		GET_VALIDATE_CHILD(pActionNode, "ENTITY_TYPE", &entityType, ENTITY_TYPE_ENUM);
		m_entityType = (eEntityType)entityType;

		GET_VALIDATE_CHILD(pActionNode, "FILTER", (char*)&m_filter, ONE_LINE_BUFFER_LENGTH);

		CXMLDOMElement* pTcpDumpIpList;
		GET_CHILD_NODE(pActionNode, "IP_LIST", pTcpDumpIpList);

		if (pTcpDumpIpList)
		{
			CXMLDOMElement* pTcpDumpIpEntity;
			GET_FIRST_CHILD_NODE(pTcpDumpIpList,"IP_ENTITY",pTcpDumpIpEntity);
			int i = 0;

			while (pTcpDumpIpEntity)
			{
				m_pIPList[i]->DeSerializeXml(pTcpDumpIpEntity, pszError, action);
				GET_NEXT_CHILD_NODE(pTcpDumpIpList, "IP_ENTITY", pTcpDumpIpEntity);
				++i;
			}

			TRACEINTO << "i:" << i;
		}
	}

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntity::AddIpEntity(DWORD ipAddress)
{
	for (size_t i = 0 ; i < ARRAYSIZE(m_pIPList); ++i)
	{
		if (m_pIPList[i]->GetIpAddress() == 0xFFFFFFFF || m_pIPList[i]->GetIpAddress() == ipAddress)
		{
			m_pIPList[i]->SetIpAddress(ipAddress);
			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntity::AddIpV6Entity(char* ipV6Address)
{
	m_pIPList[0]->SetIpV6Address(ipV6Address);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntity::InitMembers()
{
	m_boardId = 0;
	memset(m_filter, 0, sizeof(ONE_LINE_BUFFER_LEN));
	m_entityType   = e_EntityType_Media_Card;
	m_tcpDumpState = e_TcpDumpState_Idle;

	for (size_t i = 0 ; i < ARRAYSIZE(m_pIPList); ++i)
		m_pIPList[i] = new CIPEntity;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntity::ClearMembers()
{
	m_boardId = 0;
	memset(m_filter, 0, sizeof(ONE_LINE_BUFFER_LEN));
	m_entityType   = e_EntityType_Media_Card;
	m_tcpDumpState = e_TcpDumpState_Idle;

	for (size_t i = 0 ; i < ARRAYSIZE(m_pIPList); ++i)
		m_pIPList[i]->ClearMembers();
}

////////////////////////////////////////////////////////////////////////////////////////////////
void  CTcpDumpEntity::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg
		<< "\n===== CTcpDumpEntity::Dump =====\n"
		<< std::setw(20)  << "m_boardId:    " << m_boardId << "\n"
		<< std::setw(120) << "m_filter:    " << m_filter << "\n"
		<< std::setw(20)  << "m_entityType:  " << m_entityType << "\n";

	for (size_t i = 0; i < ARRAYSIZE(m_pIPList); ++i)
		msg << "\nIPList " << i << "\n" << *m_pIPList[i];
}

/////////////////////////////////////////////////////////////////////////////////////////////////
const std::string CTcpDumpEntity::GetTcpDumpEntityName() const
{
	switch (m_entityType)
	{
		case e_EntityType_Management:
			return "Management";

		case e_EntityType_Central_Signaling:
			return "Central_Signaling";

		case e_EntityType_Media_Card:
			return "Media Card " + m_boardId;

		default:
			return "";
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpEntity::SetTcpDumpState(eTcpDumpState state)
{
	m_tcpDumpState = state;

}

eTcpDumpState CTcpDumpEntity::GetTcpDumpState(void)
{
	return m_tcpDumpState;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
void  CIPEntity::Dump(ostream& msg) const
{
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	char ip_str[IP_ADDRESS_LEN];
	SystemDWORDToIpString(m_ipAddress.addr.v4.ip, ip_str);

	msg << "\n===== CIPEntity::Dump =====" << "\n"
		<< std::setw(20) << "m_selected:    " << m_selected << "\n"
		<< std::setw(20) << "m_ipAddress:    " << ip_str << "\n";
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CIPEntity::InitMembers()
{
	SetIpAddress(0xFFFFFFFF);
	m_selected = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CIPEntity::ClearMembers()
{
	SetIpAddress(0xFFFFFFFF);
	m_selected = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CIPEntity::CIPEntity()
{
	InitMembers();
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CIPEntity::SerializeXml(CXMLDOMElement*& pParentNode) const
{
	CXMLDOMElement* pIpEntityNode = pParentNode->AddChildNode("IP_ENTITY");

	if (m_ipAddress.ipVersion == eIpVersion4)
		pIpEntityNode->AddChildNode("IP_V4_V6", m_ipAddress.addr.v4.ip, IP_ADDRESS);

	else if (m_ipAddress.ipVersion == eIpVersion6)
		pIpEntityNode->AddIPv6ChildNode("IP_V4_V6", m_ipAddress.addr.v6.ip, NULL);

	pIpEntityNode->AddChildNode("SELECTED", m_selected);
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CIPEntity::SetIpV6Address(char* ipV6addr)
{
	TRACEINTO << "ipV6addr:" << ipV6addr;
	if (::isIpV6Str(ipV6addr))
		stringToIpV6(&m_ipAddress, ipV6addr);
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CIPEntity::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pActionNode, "IP_V4_V6", &m_ipAddress.addr.v4.ip, IP_ADDRESS);
	GET_VALIDATE_CHILD(pActionNode, "SELECTED", &m_selected, _BOOL);

	TRACEINTO << "m_ipAddress.addr.v4.ip:" << m_ipAddress.addr.v4.ip;

	return nStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpStatus::InitMembers()
{
	m_tcpDump_State = e_TcpDumpState_Idle;

	m_time_elapsed.m_hour = 0;
	m_time_elapsed.m_min = 0;
	m_time_elapsed.m_sec = 0;
	m_storage_in_used = 0;

	m_isStartOn = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpStatus::ClearMembers()
{
	m_tcpDump_State = e_TcpDumpState_Idle;

	m_time_elapsed.m_hour = 0;
	m_time_elapsed.m_min = 0;
	m_time_elapsed.m_sec = 0;
	m_storage_in_used = 0;

	m_isStartOn = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpStatus::CTcpDumpStatus()
{
	InitMembers();
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpStatus::CTcpDumpStatus(const CTcpDumpStatus& other)
	: CSerializeObject(other)
{
	m_tcpDump_State   = other.m_tcpDump_State;
	m_updateCounter   = other.m_updateCounter;
	m_time_elapsed    = other.m_time_elapsed;
	m_storage_in_used = other.m_storage_in_used;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void  CTcpDumpStatus::Dump(ostream& msg) const
{
#if 0
	msg.setf(std::ios::left,std::ios::adjustfield);
	msg.setf(std::ios::showbase);

	msg
		<< "\n===== CTcpDumpStatus::Dump ====="
		<< "\nCyclic Storage "        << m_cyclic_storage
		<< "\nTcp Dump State "        << m_tcpDump_State
		<< "\n m_max_capture_size "   << m_max_capture_size
		<< "\nm_max_capture_duration "<< m_max_capture_duration
		<< "\nm_time_elapsed.m_hour " << m_time_elapsed.m_hour
		<< "\nm_time_elapsed.m_min "  <<m_time_elapsed.m_min
		<< "\nm_time_elapsed.m_sec "  <<m_time_elapsed.m_sec
		<< "\nm_storage_in_used "     <<m_storage_in_used;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////
CTcpDumpStatus& CTcpDumpStatus::operator=( const CTcpDumpStatus& other )
{
	if(this == &other)
		return *this;

	m_updateCounter   = other.m_updateCounter;
	m_tcpDump_State   = other.m_tcpDump_State;
	m_time_elapsed    = other.m_time_elapsed;
	m_storage_in_used =other.m_storage_in_used;

	return *this;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpStatus::SerializeXml(CXMLDOMElement*& pFatherNode) const
{
	CXMLDOMElement* ptcpDumpCfgNode;

	if(!pFatherNode)
	{
		pFatherNode =  new CXMLDOMElement();
		pFatherNode->set_nodeName("TCP_DUMP_STATUS");
		ptcpDumpCfgNode = pFatherNode;
	}
	else
	{
		ptcpDumpCfgNode = pFatherNode->AddChildNode("TCP_DUMP_STATUS");
	}

	CUtilityProcess *pUtilityProcess = (CUtilityProcess*)CUtilityProcess::GetProcess();
	bool            isUiUpdateNeeded = pUtilityProcess->GetIsUiUpdateNeeded();

	if (isUiUpdateNeeded && m_tcpDump_State == e_TcpDumpState_Idle)
	{
		pUtilityProcess->SetIsUiUpdateNeeded(false);
		ptcpDumpCfgNode->AddChildNode("TCP_DUMP_STATE", e_TcpDumpState_Success, TCP_DUMP_STATE_ENUM);
	}
	else
	{
		ptcpDumpCfgNode->AddChildNode("TCP_DUMP_STATE", m_tcpDump_State, TCP_DUMP_STATE_ENUM);
		if(m_tcpDump_State == e_TcpDumpState_Failed)
		{
		    ptcpDumpCfgNode->AddChildNode("DESCRIPTION", m_description);
		}
	}

	CXMLDOMElement* pTempNode=ptcpDumpCfgNode->AddChildNode("TIME_ELAPSED");

	pTempNode->AddChildNode("HOUR", m_time_elapsed.m_hour);
	pTempNode->AddChildNode("MINUTE",m_time_elapsed.m_min);
	pTempNode->AddChildNode("SECOND",m_time_elapsed.m_sec);

	/***********************************************/
	BOOL isTcpdumpFolderEmpty = (GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/emb/").c_str()) ||
		GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/mcms/").c_str()) || GetDirFilesNum((MCU_OUTPUT_DIR+"/tcp_dump/media-signaling/").c_str())) ? FALSE : TRUE;
	string tcp_dump_size;

	if (!isTcpdumpFolderEmpty)
	{
		string command = "du -s "+MCU_OUTPUT_DIR+"/tcp_dump | awk '{print $1}'";

		SystemPipedCommand(command.c_str(), tcp_dump_size);
		PTRACE(eLevelInfoNormal,command.c_str());
	}
	else
	{
		tcp_dump_size = "0";
	}

	PTRACE(eLevelInfoNormal,tcp_dump_size.c_str());
	/***********************************************/
	static int storage = 1;
	storage++;
	ptcpDumpCfgNode->AddChildNode("STORAGE_IN_USED", tcp_dump_size.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////
int CTcpDumpStatus::DeSerializeXml(CXMLDOMElement *pActionNode,char *pszError,const char* action)
{
	// schema file name:  obj_ip_srv_list.xsd

#if 0
	int nStatus = STATUS_OK;
	CXMLDOMElement *pTcpDumpEntitiesList;

	GET_CHILD_NODE(pActionNode, "TCP_DUMP_ENTITIES", pTcpDumpEntitiesList);
	if (pTcpDumpEntitiesList)
	{
		CXMLDOMElement *pTcpDumpEntity;
		GET_FIRST_CHILD_NODE(pTcpDumpEntitiesList,"ENTITY",pTcpDumpEntity);

		int i=0;
		while (pTcpDumpEntity)
		{
			m_pTcpDumpEntities[i]->DeSerializeXml(pTcpDumpEntity, pszError, action);

			GET_NEXT_CHILD_NODE(pTcpDumpEntitiesList,"ENTITY",pTcpDumpEntity);
			i++;
		}

		TRACEINTO << "END CTcpDumpStatus::DeSerializeXml  i=" << i;

	}

	GET_VALIDATE_CHILD(pActionNode,"CYCLIC_STORAGE", &m_cyclic_storage,_BOOL);
	GET_VALIDATE_CHILD(pActionNode,"MAX_CAPTURE_SIZE", (WORD*)&m_max_capture_size,MAX_CAPTURE_SIZE_ENUM);
	GET_VALIDATE_CHILD(pActionNode,"MAX_CAPTURE_DURATION", (WORD*)&m_max_capture_duration,MAX_CAPTURE_DURATION_ENUM);
#endif

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpStatus::SetTcpDumpState(eTcpDumpState state, const char* descTemplate/* = NULL*/, const char* entityName/* = NULL*/)
{
	m_tcpDump_State = state;

	if (!descTemplate)
		m_description[0] = 0;
	else
	{
		if (entityName && *entityName)
			snprintf(m_description, ONE_LINE_BUFFER_LEN, descTemplate, entityName);
		else
			snprintf(m_description, ONE_LINE_BUFFER_LEN, descTemplate);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////
void CTcpDumpStatus::SetDescription(const char* desc)
{
	if (desc)
		strncpy(m_description, desc, ONE_LINE_BUFFER_LEN);
	else
		m_description[0] = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
