#include <algorithm>
#include "PartyDebugInfo.h"
#include "MplMcmsProtocol.h"
#include "OpcodesMcmsCardMngrTB.h"
#include "TBStructs.h"
#include "MplMcmsProtocolTracer.h"
#include "InitCommonStrings.h"
#include <string.h>
#include "psosxml.h"

////////////////////////////////////////////////////////////////////////////
//                        CPortDebugInfo
////////////////////////////////////////////////////////////////////////////
CPortDebugInfo::CPortDebugInfo(CRsrcDesc& rsrcDesc) : m_rsrcDesc(rsrcDesc)
{
	memset(&m_portInfo, 0, sizeof(m_portInfo));
}

////////////////////////////////////////////////////////////////////////////
CPortDebugInfo::CPortDebugInfo(const CPortDebugInfo& other)
{
	*this = other;
}

////////////////////////////////////////////////////////////////////////////
CPortDebugInfo& CPortDebugInfo::operator=(const CPortDebugInfo& other)
{
	if (&other == this)
		return *this;
	m_rsrcDesc = other.m_rsrcDesc;
	memcpy(&m_portInfo, &other.m_portInfo, sizeof(m_portInfo));
	return *this;
}

////////////////////////////////////////////////////////////////////////////
bool operator==(const CPortDebugInfo& port_1, const CPortDebugInfo& port_2)
{
	return (port_1.m_rsrcDesc == port_2.m_rsrcDesc);
}

////////////////////////////////////////////////////////////////////////////
bool operator==(const CPortDebugInfo& port_1, const CRsrcDesc& desc_2)
{
	return (port_1.m_rsrcDesc == desc_2);
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CPortDebugInfo& port_1, const CPortDebugInfo& port_2)
{
	if (port_1.m_rsrcDesc.GetBoardId() < port_2.m_rsrcDesc.GetBoardId())
		return true;
	if (port_1.m_rsrcDesc.GetSubBoardId() < port_2.m_rsrcDesc.GetSubBoardId())
		return true;
	if (port_1.m_rsrcDesc.GetUnitId() < port_2.m_rsrcDesc.GetUnitId())
		return true;
	if (port_1.m_rsrcDesc.GetFirstPortId() < port_2.m_rsrcDesc.GetFirstPortId())
		return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////
void CPortDebugInfo::SetPortDebugInfo(char* portDebugInfo, bool receivedFromMpl)
{
	strcpy_safe(m_portInfo[0].debugStr, portDebugInfo);
	if (receivedFromMpl)
		m_portInfo[0].isFromMpl = true;
}

////////////////////////////////////////////////////////////////////////////
void CPortDebugInfo::SetPortCmDebugInfo(char* portDebugInfo, bool receivedFromMpl)
{
	strcpy_safe(m_portInfo[1].debugStr, portDebugInfo);
	if (receivedFromMpl)
		m_portInfo[1].isFromMpl = true;
}

////////////////////////////////////////////////////////////////////////////
STATUS CPortDebugInfo::SendInfoReq(ConfRsrcID confId, PartyRsrcID partyId) const
{
	FTRACEINTO << "ConfId:" << confId << ", PartyId:" << partyId;

	bool isAudioController = (m_rsrcDesc.GetType() == eLogical_audio_controller) ? true : false;

	CMplMcmsProtocol mplMcmsProtocol;
	if (isAudioController)
		mplMcmsProtocol.AddCommonHeader(CONF_DEBUG_INFO_REQ);
	else
		mplMcmsProtocol.AddCommonHeader(PARTY_DEBUG_INFO_REQ);

	mplMcmsProtocol.AddMessageDescriptionHeader();
	mplMcmsProtocol.AddPhysicalHeader(m_rsrcDesc.GetBoxId(), m_rsrcDesc.GetBoardId(), m_rsrcDesc.GetSubBoardId(), m_rsrcDesc.GetUnitId(), m_rsrcDesc.GetFirstPortId(), m_rsrcDesc.GetAcceleratorId(), m_rsrcDesc.GetPhysicalType());
	mplMcmsProtocol.AddPortDescriptionHeader(partyId, confId, m_rsrcDesc.GetConnId(), m_rsrcDesc.GetType());

	if (isAudioController)
	{
		CONF_DEBUG_INFO_REQ_S confDebugInfo;
		confDebugInfo.confId = confId;
		mplMcmsProtocol.AddData(sizeof(CONF_DEBUG_INFO_REQ_S), (char*)&confDebugInfo);
	}

	CMplMcmsProtocolTracer(mplMcmsProtocol).TraceMplMcmsProtocol("RESOURCE_SENDS_TO_MPL_API");
	return mplMcmsProtocol.SendMsgToMplApiCommandDispatcher();
}

////////////////////////////////////////////////////////////////////////////
bool CPortDebugInfo::IsAllInfoReceived() const
{
	// Temporary for V8.3, we ignore opcode PARTY_CM_DEBUG_INFO_IND due to socket issues.
	return (m_portInfo[0].isFromMpl /*&& m_portInfo[1].isFromMpl*/);
}

////////////////////////////////////////////////////////////////////////////
void CPortDebugInfo::GetInfoString(std::ostream& msg) const
{
	msg
	<< "\n"            << m_rsrcDesc.GetType()
	<< ", ConnId:"     << m_rsrcDesc.GetConnId()
	<< ", BoardId:"    << m_rsrcDesc.GetBoardId()
	<< ", SubBoardId:" << m_rsrcDesc.GetSubBoardId()
	<< ", UnitId:"     << m_rsrcDesc.GetUnitId()
	<< ", PortId:"     << m_rsrcDesc.GetFirstPortId();

	msg << "\nport debug info:\n";
	if (m_portInfo[0].isFromMpl)
		msg << m_portInfo[0].debugStr;
	else
		msg << "empty";

	msg << "\ncard manager debug info:\n";
	if (m_portInfo[1].isFromMpl)
		msg << m_portInfo[1].debugStr;
	else
		msg << "empty";
}

////////////////////////////////////////////////////////////////////////////
bool CPortDebugInfo::IsEqual(DWORD req_board_id, DWORD req_sub_board_id, DWORD req_unit_id, DWORD req_accelerator_id, DWORD req_port_id, DWORD req_conn_id)
{
	bool isEquale = false;

	DWORD board_id       = m_rsrcDesc.GetBoardId();
	DWORD sub_board_id   = m_rsrcDesc.GetSubBoardId();
	DWORD unit_id        = m_rsrcDesc.GetUnitId();
	DWORD accelerator_id = m_rsrcDesc.GetAcceleratorId();
	DWORD port_id        = m_rsrcDesc.GetFirstPortId();
	DWORD conn_id        = m_rsrcDesc.GetConnId();

	// Currently ignore conn_id because dsp implemented per ART port and not per logical resource, we don't get right conn id in the IND.
	if (req_board_id == board_id && req_sub_board_id == sub_board_id && req_unit_id == unit_id && req_accelerator_id == accelerator_id && req_port_id == port_id /*&& req_conn_id == conn_id*/)
	{
		isEquale = true;
	}

	// this is a patch for audio controller. one day I will debug it and remove the patch. ha ha ha.
	if (req_board_id == board_id && req_sub_board_id == sub_board_id && req_unit_id == unit_id && req_accelerator_id == accelerator_id)
	{
		if ((port_id == 65535 && req_port_id == 255) || (port_id == 255 && req_port_id == 65535) || ((req_port_id == 0xFFFF || req_port_id == 0xFF) && (port_id == 0xFFFF || port_id == 0xFF)))
		{
			isEquale = true;
		}
	}
	return isEquale;
}

////////////////////////////////////////////////////////////////////////////
void CPortDebugInfo::WhyNotEqual(DWORD req_board_id, DWORD req_sub_board_id, DWORD req_unit_id, DWORD req_accelerator_id, DWORD req_port_id, DWORD req_conn_id) const
{
	DWORD board_id       = m_rsrcDesc.GetBoardId();
	DWORD sub_board_id   = m_rsrcDesc.GetSubBoardId();
	DWORD unit_id        = m_rsrcDesc.GetUnitId();
	DWORD accelerator_id = m_rsrcDesc.GetAcceleratorId();
	DWORD port_id        = m_rsrcDesc.GetFirstPortId();
	DWORD conn_id        = m_rsrcDesc.GetConnId();

	std::ostringstream msg;
	msg << "LogicalType:" << m_rsrcDesc.GetType();

	if (req_board_id != board_id)
	{
		msg << ", BoardId:" << board_id << ", ReqBoardId:" << req_board_id;
	}
	else if (req_sub_board_id != sub_board_id)
	{
		msg << ", SubBoardId:" << sub_board_id << ", ReqSubBoardId:" << req_sub_board_id;
	}
	else if (req_unit_id != unit_id)
	{
		msg << ", UnitId:" << unit_id << ", ReqUnitId:" << req_unit_id;
	}
	else if (req_accelerator_id != accelerator_id)
	{
		msg << ", AcceleratorId:" << accelerator_id << ", ReqAcceleratorId:" << req_accelerator_id;
	}
	else if (req_port_id != port_id)
	{
		msg << ", PortId:" << port_id << ", ReqPortId:" << req_port_id;
	}

	FTRACEINTO << msg.str().c_str();
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CPortDebugInfo& obj)
{
	os
		<< obj.m_rsrcDesc
		<< "\n  PortInfo      :" << DUMPSTR(obj.m_portInfo[0].debugStr)
		<< "\n  PortInfoDone  :" << (int)obj.m_portInfo[0].isFromMpl
		<< "\n  PortCmInfo    :" << DUMPSTR(obj.m_portInfo[1].debugStr)
		<< "\n  PortCmInfoDone:" << (int)obj.m_portInfo[1].isFromMpl;

	return os;
}

PBEGIN_MESSAGE_MAP(CPartyDebugInfo)
	ONEVENT(GET_PORTS_INFO_TIMER, ANYCASE, CPartyDebugInfo::OnTimerComplete)
	ONEVENT(RETRIEVE_INFO_TIMER,  ANYCASE, CPartyDebugInfo::OnRetriveInfoTimer)
PEND_MESSAGE_MAP(CPartyDebugInfo,CStateMachine);

////////////////////////////////////////////////////////////////////////////
void DelPortDebugInfoElem (CPortDebugInfo* pPortDebugInfo)
{
	POBJDELETE(pPortDebugInfo);
}

////////////////////////////////////////////////////////////////////////////
void CPartyDebugInfo::HandleEvent (CSegment *pMsg,DWORD msgLen,OPCODE opCode)
{
	DispatchEvent(opCode,pMsg);
}

////////////////////////////////////////////////////////////////////////////
void* CPartyDebugInfo::GetMessageMap()
{
	return (void*)m_msgEntries;
}

////////////////////////////////////////////////////////////////////////////
//                        CPartyDebugInfo
////////////////////////////////////////////////////////////////////////////
CPartyDebugInfo::CPartyDebugInfo(ConfMonitorID monitorConfId, PartyMonitorID monitorPartyId, ConfRsrcID confId, PartyRsrcID partyId)
  :m_monitorConfId(monitorConfId), m_monitorPartyId(monitorPartyId), m_confId(confId), m_partyId(partyId), m_isInfoRetrieved(false)
{
	m_state = IDLE;
}

////////////////////////////////////////////////////////////////////////////
CPartyDebugInfo::~CPartyDebugInfo()
{
	for_each(m_portsDebugInfoList.begin(), m_portsDebugInfoList.end(), DelPortDebugInfoElem);
}

////////////////////////////////////////////////////////////////////////////
CPartyDebugInfo::CPartyDebugInfo(const CPartyDebugInfo& other) : CStateMachine(other)
{
	*this = other;
}

////////////////////////////////////////////////////////////////////////////
CPartyDebugInfo& CPartyDebugInfo::operator=(const CPartyDebugInfo& other)
{
	if (&other == this)
		return *this;

	m_monitorConfId   = other.m_monitorConfId;
	m_monitorPartyId  = other.m_monitorPartyId;
	m_confId          = other.m_confId;
	m_partyId         = other.m_partyId;
	m_isInfoRetrieved = other.m_isInfoRetrieved;

	m_portsDebugInfoList.clear();
	PortDebugInfoList::iterator _itr, _end = other.m_portsDebugInfoList.end();
	for (_itr = other.m_portsDebugInfoList.begin(); _itr != _end; ++_itr)
		m_portsDebugInfoList.insert(new CPortDebugInfo(**_itr));

	return *this;
}

////////////////////////////////////////////////////////////////////////////
STATUS CPartyDebugInfo::AddPortInfo(const CPortDebugInfo& portDebugInfo)
{
	CPortDebugInfo* pPortDebugInfo = new CPortDebugInfo(portDebugInfo);
	if (m_portsDebugInfoList.find(pPortDebugInfo) != m_portsDebugInfoList.end())
	{
		// portDebugInfo already exist - replace it
		m_portsDebugInfoList.erase(pPortDebugInfo);
	}
	m_portsDebugInfoList.insert(pPortDebugInfo);
	TRACEINTO << *pPortDebugInfo;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
bool operator==(const CPartyDebugInfo& party_1, const CPartyDebugInfo& party_2)
{
	return (party_1.m_confId == party_2.m_confId && party_1.m_partyId == party_2.m_partyId);
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CPartyDebugInfo& party_1, const CPartyDebugInfo& party_2)
{
	return (party_1.m_partyId < party_2.m_partyId);
}

////////////////////////////////////////////////////////////////////////////
void CPartyDebugInfo::SendInfoReq()
{
	PTRACE(eLevelInfoNormal, "CPartyDebugInfo::SendInfoReq");

	m_state = IN_PROGRESS;
	StartTimer(GET_PORTS_INFO_TIMER, GET_PORTS_INFO_TIMEOUT);

	PortDebugInfoList::iterator _itr, _end = m_portsDebugInfoList.end();
	for (_itr = m_portsDebugInfoList.begin(); _itr != _end; ++_itr)
		(*_itr)->SendInfoReq(m_confId, m_partyId);
}

////////////////////////////////////////////////////////////////////////////
void CPartyDebugInfo::PortInfoInd(bool isCmInfo, BoardID boardId, SubBoardID subBoardId, UnitID unitId, AcceleratorID acceleratorId, PortID portId, ConnectionID connId, char* info)
{
	TRACEINTO << "BoardId:" << boardId << ", SubBoardId:" << subBoardId << ", UnitId:" << unitId << ", PortId:" << portId << ", ConnectionId:" << connId << ", isCmInfo:" << (int)isCmInfo;

	bool port_found = false;
	PortDebugInfoList::iterator _itr, _end = m_portsDebugInfoList.end();
	for (_itr = m_portsDebugInfoList.begin(); _itr != _end; ++_itr)
	{
		port_found = (*_itr)->IsEqual(boardId, subBoardId, unitId, acceleratorId, portId, connId);
		if (port_found)
			break;
	}

	if (!port_found)
	{
		PTRACE(eLevelInfoNormal, "CPartyDebugInfo::PortInfoInd - port not found");
		for (_itr = m_portsDebugInfoList.begin(); _itr != _end; ++_itr)
			(*_itr)->WhyNotEqual(boardId, subBoardId, unitId, acceleratorId, portId, connId);

		return;
	}

	if (isCmInfo)
		(*_itr)->SetPortCmDebugInfo(info);
	else
		(*_itr)->SetPortDebugInfo(info);

	if (IsAllInfoReceived())
		InfoCompleted();
}

////////////////////////////////////////////////////////////////////////////
bool CPartyDebugInfo::IsAllInfoReceived() const
{
	PortDebugInfoList::iterator _itr, _end = m_portsDebugInfoList.end();
	for (_itr = m_portsDebugInfoList.begin(); _itr != _end; ++_itr)
	{
		if (!(*_itr)->IsAllInfoReceived())
			return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////
void CPartyDebugInfo::GetInfoString(std::ostream& msg) const
{
	PortDebugInfoList::iterator _itr, _end = m_portsDebugInfoList.end();
	for (_itr = m_portsDebugInfoList.begin(); _itr != _end; ++_itr)
	{
		(*_itr)->GetInfoString(msg);
		msg << "\n=================================================";
	}
}

////////////////////////////////////////////////////////////////////////////
STATUS CPartyDebugInfo::GetStatus() const
{
	return (m_state == COMPLETED) ? STATUS_OK : STATUS_IN_PROGRESS;
}

////////////////////////////////////////////////////////////////////////////
void CPartyDebugInfo::OnTimerComplete(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CPartyDebugInfo::OnTimerComplete");
	InfoCompleted();
}

////////////////////////////////////////////////////////////////////////////
void CPartyDebugInfo::InfoCompleted()
{
	PTRACE(eLevelInfoNormal, "CPartyDebugInfo::InfoCompleted");
	DeleteTimer(GET_PORTS_INFO_TIMER);
	m_state = COMPLETED;
	StartTimer(RETRIEVE_INFO_TIMER, RETRIEVE_INFO_TIMEOUT);
}

////////////////////////////////////////////////////////////////////////////
bool CPartyDebugInfo::IsSameParty(ConfRsrcID confId, PartyRsrcID partyId)
{
	return (m_confId == confId && m_partyId == partyId);
}

////////////////////////////////////////////////////////////////////////////
void CPartyDebugInfo::SetInfoRetrieved()
{
	PTRACE(eLevelInfoNormal, "CPartyDebugInfo::SetInfoRetrieved");
	DeleteTimer(RETRIEVE_INFO_TIMER);
	m_isInfoRetrieved = true;
}

////////////////////////////////////////////////////////////////////////////
void CPartyDebugInfo::OnRetriveInfoTimer(CSegment* pSeg)
{
	PTRACE(eLevelInfoNormal, "CPartyDebugInfo::OnRetriveInfoTimer");
	m_isInfoRetrieved = true;
}


////////////////////////////////////////////////////////////////////////////
//                        CPartyPortsInfo
////////////////////////////////////////////////////////////////////////////
CPartyPortsInfo::CPartyPortsInfo()
{
	m_confId = 0;
	m_partyId = 0;
	m_bIsSendToCM = TRUE;
}

////////////////////////////////////////////////////////////////////////////
CPartyPortsInfo::CPartyPortsInfo(const char* infoStr)
{
	m_confId = 0;
	m_partyId = 0;
	m_bIsSendToCM = TRUE;
	m_debugInfo = infoStr;
}

////////////////////////////////////////////////////////////////////////////
void CPartyPortsInfo::SerializeXml(CXMLDOMElement*& thisNode) const
{
	CXMLDOMElement* pReportNode = thisNode->AddChildNode("PARTY_PORTS_INFO");
	pReportNode->AddChildNode("PORTS_INFO", m_debugInfo);
}

////////////////////////////////////////////////////////////////////////////
int CPartyPortsInfo::DeSerializeXml(CXMLDOMElement *pNode,char *pszError, const char* action)
{
	STATUS nStatus = STATUS_OK;

	GET_VALIDATE_CHILD(pNode, "ID", &m_confId, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pNode, "PARTY_ID", &m_partyId, _0_TO_DWORD);
	GET_VALIDATE_CHILD(pNode, "SEND_REQ_TO_CARD", &m_bIsSendToCM, _BOOL);

	return nStatus;
}

