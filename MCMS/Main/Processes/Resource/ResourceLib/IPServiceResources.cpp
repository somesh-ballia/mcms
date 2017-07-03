// IPServiceResources.cpp: implementation of the CIPServiceResources class.
//
//////////////////////////////////////////////////////////////////////

#include "IPServiceResources.h"
#include "ResourceProcess.h"
#include "StatusesGeneral.h"
#include "TraceStream.h"
#include "WrappersResource.h"
#include "HelperFuncs.h"
#include "PrettyTable.h"

///////////////////////////////////////////////////////////////////////////
CPQperSrvResource::CPQperSrvResource(WORD boxId, WORD boardId, WORD subBoardId, WORD PQid, WORD subServiceId)
	: m_UDPallocType(eMethodStatic)
	, m_serviceId(0)
	, m_boxId(boxId)
	, m_boardId(boardId)
	, m_subBoardId(subBoardId)
	, m_PQid(PQid)
	, m_type(0)
	, m_ipType(eIpType_None)
	, m_UDPChannelFirst(0)
	, m_UDPChannelLast(0)
	, m_subServiceId(subServiceId)
	, m_freePortsCount(0)
	, m_disabledPortsCount(0)
	, m_headFree(0)
	, m_headDisabled(0)
	, m_tailFree(0)
	, m_tailDisabled(0)
{
	memset(&m_IpV4Addr, 0, sizeof(m_IpV4Addr));
	memset(&m_IpV6AddrArray, 0, sizeof(m_IpV6AddrArray));
}

///////////////////////////////////////////////////////////////////////////
bool CPQperSrvResource::IsIpV6Null() const
{
	for (size_t i = 0; i < NUM_OF_IPV6_ADDRESSES; ++i)
	{
		if (! ::IsIpNull(m_IpV6AddrArray[i]))
			return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////
void CPQperSrvResource::SetUDPChannels(WORD first, WORD last)
{
	m_UDPChannelFirst = first;
	m_UDPChannelLast = last;
	ReinitFreePortsMap();
}

///////////////////////////////////////////////////////////////////////////
void CPQperSrvResource::ReinitFreePortsMap()
{
	m_freePortsCount = PortsAllocationMapSize();
	m_disabledPortsCount = 0;

	m_allocationTable.resize(m_freePortsCount, 0);

	if (m_freePortsCount > 0)
	{
		const PortIndex end = static_cast<PortIndex>(m_freePortsCount - 1);

		m_headFree = 1;                  // 1-based, points to the 1-st cell
		m_tailFree = end + 1;            // 1-based, points to the last cell

		m_tailDisabled = m_headDisabled = 0; // initially, there are no disabled ports

		for (PortIndex i = 0; i < end; ++i)
		{
			m_allocationTable[i] = i + 2;      // building the linked list, each cell holds a 1-based index of the next free cell
		}
	}
	else // m_freePortsCount == 0
	{
		m_headFree = 0;
		m_tailFree = 0;
		m_tailDisabled = 0;
		m_headDisabled = 0;

		PASSERTSTREAM(1, "m_freePortsCount=" << m_freePortsCount << ", m_UDPChannelFirst=" << m_UDPChannelFirst << ", m_UDPChannelLast=" << m_UDPChannelLast);
	}

	TRACEINTO
		<< "BoxId:" << GetBoxId()
		<< ", BoardId:" << GetBoardId()
		<< ", SubBoardId:" << GetSubBoardId()
		<< ", SubServiceId:" << GetSubServiceId()
		<< ", PqId:" << GetPQMId()
		<< ", FirstPort:" << m_UDPChannelFirst
		<< ", LastPort:" << m_UDPChannelLast
		<< ", FreePorts:" << m_freePortsCount;
}

//////////////////////////////////////////////////////////////////////////////
void CPQperSrvResource::Dump(std::ostream& msg) const
{
	msg <<
		"\n\n"
		"CPQperSrvResource::Dump\n"
		"-----------------------\n\n"
		"Summary:";

	CPrettyTable<const char*, size_t> summary;
	summary.Add("Sub-Service ID", m_subServiceId);
	summary.Add("Box ID", m_boxId);
	summary.Add("Board ID", m_boardId);
	summary.Add("Sub-board ID", m_subBoardId);
	summary.Add("PQ ID", m_PQid);

	summary.Add("UDP Allocation", m_UDPallocType);
	summary.Add("type", m_type);

	summary.Add("First Channel", m_UDPChannelFirst);
	summary.Add("Last Channel", m_UDPChannelLast);
	summary.Add("Free Ports #", GetFreePortsCount() * 2);
	summary.Add("Disabled Ports #", GetDisabledPortsCount() * 2);
	summary.Dump(msg);

	msg
		<< "\nIPv4: " << CIPV4Wrapper(const_cast<ipAddressV4If&>(m_IpV4Addr))
		<< "\nIPv6: " << CIPV6AraryWrapper(const_cast<ipv6AddressArray&>(m_IpV6AddrArray))
		<< "\n";

	CPrettyTable<ResourceID, DWORD, PortIndex> allocated("Party ID", "Port", "Index");

	for (PartyPortsMap::const_iterator it = m_assignedPortsMap.begin(); it != m_assignedPortsMap.end(); ++it)
	{
		PortIndex i = it->second;

		while (i)
		{
			--i; // translate to 0-based
			allocated.Add(it->first, Index2Port(i), i);

			i = m_allocationTable[i];
		}
	}

	if (!allocated.IsEmpty())
	{
		msg << "Allocated ports:\n";
		allocated.Sort(1);
		allocated.Dump(msg);
		msg << "\n\n";
	}
}

//////////////////////////////////////////////////////////////////////////////
bool operator==(const CPQperSrvResource& lhs, const CPQperSrvResource& rhs)
{
	return
		(lhs.m_boxId == rhs.m_boxId) &&
		(lhs.m_boardId == rhs.m_boardId) &&
		(lhs.m_subBoardId == rhs.m_subBoardId) &&
		(lhs.m_subServiceId == rhs.m_subServiceId) &&
		(lhs.m_PQid == rhs.m_PQid);
}
////////////////////////////////////////////////////////////////////////////
bool operator<(const CPQperSrvResource& lhs, const CPQperSrvResource& rhs)
{
	if (lhs.m_boxId != rhs.m_boxId)
		return (lhs.m_boxId < rhs.m_boxId);

	if (lhs.m_boardId != rhs.m_boardId)
		return (lhs.m_boardId < rhs.m_boardId);

	if (lhs.m_subBoardId != rhs.m_subBoardId)
		return (lhs.m_subBoardId < rhs.m_subBoardId);

	if (lhs.m_subServiceId != rhs.m_subServiceId)
		return (lhs.m_subServiceId < rhs.m_subServiceId);

	return (lhs.m_PQid < rhs.m_PQid);
}
//////////////////////////////////////////////////////////////////////////////
bool CPQperSrvResource::CanAllocate(size_t count, bool isIceParty) const
{
	return count * (isIceParty ? 2 : 1) <= (m_freePortsCount + m_disabledPortsCount);
}

//////////////////////////////////////////////////////////////////////////////
void CPQperSrvResource::LinkCellToParty(ResourceID partyID, PortIndex i)
{
	PartyPortsMap::iterator it = m_assignedPortsMap.find(partyID);

	if (it != m_assignedPortsMap.end()) // are any open ports associated with the party?
	{
		m_allocationTable[i] = it->second; // "push_front"
		it->second = i + 1;                // update the "Head"
	}
	else
	{
		m_allocationTable[i] = 0;
		m_assignedPortsMap.insert(std::make_pair(partyID, i + 1));
	}

	TRACESTRFUNC(eLevelDebug) << "PartyID:" << partyID << ", index:" << i;
}

//////////////////////////////////////////////////////////////////////////////
bool CPQperSrvResource::AllocateUdpPort(ResourceID partyID, WORD& port, WORD& additionalPort, bool isIceParty, void* pTbl)
{
	PASSERT_AND_RETURN_VALUE(!partyID, false);

	const size_t PortsToAlloc = isIceParty ? 2 : 1;

	struct PortData {
		WORD& port;
	};

	size_t k = 0;
	PortData ports[] = { { port }, { additionalPort } };

	struct Head {
		PortIndex& head;
		PortIndex& tail;
		size_t&    freeCount;
	};

	CResourceProcess* pResourceProcess = (CResourceProcess*)CResourceProcess::GetProcess();
	PASSERT_AND_RETURN_VALUE(!pResourceProcess, false);
	AllocatedUdpPorts& allocatedUdpPorts = pResourceProcess->GetAllocatedUdpPorts();
	std::pair<std::string, PortPerProcess>& service = allocatedUdpPorts[m_serviceId];
	PortPerProcess& portPerProcess = service.second;
	CPrettyTable<long, const char*> tbl("UDP port", "Owner");
	PortPerProcess::iterator _itr, _end = portPerProcess.end();
	for (_itr = portPerProcess.begin(); _itr != _end; ++_itr)
		tbl.Add(_itr->first, _itr->second.c_str());
	Head lists[] = { { m_headFree, m_tailFree, m_freePortsCount }, { m_headDisabled, m_tailDisabled, m_disabledPortsCount } };

	//TRACEINTOLVLERR << "ServiceId:" << m_serviceId << ", Name:" << service.first << ", Size:"<< allocatedUdpPorts.size() << ", Size2:" << portPerProcess.size() << tbl.Get();

	for (size_t h = 0; h < sizeof(lists)/sizeof(lists[0]); )
	{
		if (!lists[h].head) // the current Head does NOT point anywhere, try the next one if available
		{
			++h;
			continue;
		}

		PASSERT(ports[k].port); // it should be ZERO before allocation

		PortID portCandidate = 0;
		PortIndex i = 0;
		while (1)
		{
			i = lists[h].head - 1; // convert 1-based index stored at the Head to 0-based
		lists[h].head = m_allocationTable[i]; // update the Head to point to the next unoccupied cell

			portCandidate = Index2Port(i);
			_itr = portPerProcess.find(portCandidate);
			if (_itr != portPerProcess.end())
			{
				TRACEINTOLVLERR << "Port:" << portCandidate << ", ServiceId:" << m_serviceId << "- An attempt to allocate already allocated UDP port" << tbl.Get();
				PASSERT(1);
				continue;
			}
			_itr = portPerProcess.find(portCandidate+1);
			if (_itr != portPerProcess.end())
			{
				TRACEINTOLVLERR << "Port:" << portCandidate+1 << ", ServiceId:" << m_serviceId << "- An attempt to allocate already allocated UDP port" << tbl.Get();
				PASSERT(1);
				continue;
			}
			break;
		}
		if (!lists[h].head)
			lists[h].tail = 0;

		LinkCellToParty(partyID, i);

		--lists[h].freeCount;

		ports[k].port = portCandidate;

		if (pTbl)
			static_cast<CPrettyTable<PortIndex, size_t, WORD, size_t>*>(pTbl)->Add(i, h, ports[k].port, lists[h].freeCount);

		++k;

		if (k == PortsToAlloc)
			return true;
	}

	std::ostringstream msg;
	msg << "";
	Dump(msg);

	TRACESTRFUNC(eLevelError) << msg.str().c_str();
	PASSERT(true);
	return false;
}

//////////////////////////////////////////////////////////////////////////////
bool CPQperSrvResource::UnlinkCellFromParty(ResourceID partyID, PortIndex& i, bool any/* = false*/)
{
	PartyPortsMap::iterator it = m_assignedPortsMap.find(partyID);

	if (it != m_assignedPortsMap.end()) // are any open ports associated with the party?
	{
		PortIndex next = it->second; // 1-based index of the 1st open port associated with the party
		PortIndex prev  = 0;          // 1-based index of the cell before the current

		while (next) // not at the end of the "linked list"
		{
			--next; // translate to 0-based
			PASSERT(next >= m_allocationTable.size());

			PortIndex current = next; // save the cell index (0-based)
			next = m_allocationTable[current]; // retrieve the new next (1-based)

			// TRACEINTO << "PartyID:" << partyID << ", any:" << any << ", prev:" << prev << ", current:" << current << ", next:" << next;

			if (any || current == i)
			{
				if (prev)
					m_allocationTable[prev - 1] = next;
				else // removing the very 1st port in the linked list
				{
					if (next) // the party still has opened ports associated with it?
						it->second = next;
					else
						m_assignedPortsMap.erase(it);
				}

				i = current;
				return true;
			}

			prev = current + 1; // translate to 1-based
		}
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
bool CPQperSrvResource::Port2Index(WORD port, PortIndex& i) const
{
	if (port >= m_UDPChannelLast) {
		PASSERT_AND_RETURN_VALUE(port, false);
	}

	i = (port - m_UDPChannelFirst) / 2;
	return true;
}

size_t CPQperSrvResource::FreeUdpPort(ResourceID partyID, WORD* port/* = NULL*/, bool disable/* = false*/)
{
	PASSERT_AND_RETURN_VALUE(!partyID, false);

	PortIndex i;
	if (port)
		if (!Port2Index(*port, i))
			return 0;

	struct Head {
		PortIndex& head;
		PortIndex& tail;
		size_t&    freeCount;
	};

	Head lists[] = { { m_headFree, m_tailFree, m_freePortsCount }, { m_headDisabled, m_tailDisabled, m_disabledPortsCount } };

	size_t count = 0;

	while (UnlinkCellFromParty(partyID, i, !port))
	{
		++count;
		++lists[disable].freeCount; // update the unoccupied ports count

		// *** insert a new cell at the end of the list of unoccupied cells
		if (lists[disable].tail)
			m_allocationTable[lists[disable].tail - 1] = i + 1;

		m_allocationTable[i] = 0; // mark the end of the list
		lists[disable].tail = i + 1; // tail points to this currently freed port

		if (!lists[disable].head) // let head point to the same as tail for one-element-list
			lists[disable].head = lists[disable].tail;

		TRACEINTO
			<< "PartyID:" << partyID << ", port:" << Index2Port(i) << ", index:" << i << ", disabled:" << disable
			<< ", " << lists[disable].freeCount << " ports available in the list";

		if (port)
			*port = 0;
	}

	PASSERT(!count);
	return count;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CPQperSrvResource::CanAllocateUDP(eVideoPartyType type, bool isBFCPUDP, bool isIceParty) const
{
	size_t count = CHelperFuncs::IsAudioParty(type) ? 1 : (MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS - !isBFCPUDP);
	return CanAllocate(count, isIceParty) ? STATUS_OK : STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CPQperSrvResource::AllocateUDP(
	ResourceID partyID,
	eVideoPartyType videoPartyType,
	WORD& portAud,
	WORD& portVid,
	WORD& portFecc,
	WORD& portContent,
	WORD& portBFCPUDP,
	WORD& portAudAdditionalPorts,
	WORD& portVidAdditionalPorts,
	WORD& portFeccAdditionalPorts,
	WORD& portContentAdditionalPorts,
	WORD& portBFCPUDPAdditionalPorts,
	bool isIceParty, bool isBFCPUDP)//ICE 4 ports
{
	struct PortData {
		WORD& port;
		WORD& additional;
	};

	PortData ports[] = {
		{ portAud, portAudAdditionalPorts },
		{ portVid, portVidAdditionalPorts },
		{ portFecc, portFeccAdditionalPorts },
		{ portContent, portContentAdditionalPorts },
		{ portBFCPUDP, portBFCPUDPAdditionalPorts },
	};

	const size_t count = CHelperFuncs::IsAudioParty(videoPartyType) ? 1 : (sizeof(ports)/sizeof(ports[0]) - !isBFCPUDP);

	if (CanAllocate(count, isIceParty))
	{
		CPrettyTable<PortIndex, size_t, WORD, size_t> tbl("PortIndex", "ListIndex", "PortId", "FreePorts");

		for (size_t i = 0; i < count; ++i)
			AllocateUdpPort(partyID, ports[i].port, ports[i].additional, isIceParty, &tbl);

		TRACEINTO << "PartyId:" << partyID << ", IsIceParty:" << isIceParty << ", IsBfcpUDP:" << isBFCPUDP << " - Going to allocate " << count << " ports:" << tbl.Get();

		return STATUS_OK;
	}

	STATUS status = STATUS_FAIL;

	std::ostringstream msg;
	msg << " allocation failed, status=" << status << "\n";
	Dump(msg);

	TRACESTRFUNC(eLevelWarn) << msg.str().c_str();
	return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// The functions allocates UDP ports for audio only party upgrading to video
STATUS CPQperSrvResource::AllocateAdditionalUDP(
	ResourceID partyID,
	WORD& portVid,
	WORD& portFecc,
	WORD& portContent,
	WORD& portBFCPUDP,
	WORD& portVidAdditionalPorts,
	WORD& portFeccAdditionalPorts,
	WORD& portContentAdditionalPorts,
	WORD& portBFCPUDPAdditionalPorts,
	bool isIceParty, bool isBFCPUDP)//ICE 4 ports
{
	struct PortData {
		WORD& port;
		WORD& additional;
	};

	PortData ports[] = {
		{ portVid, portVidAdditionalPorts },
		{ portFecc, portFeccAdditionalPorts },
		{ portContent, portContentAdditionalPorts },
		{ portBFCPUDP, portBFCPUDPAdditionalPorts },
	};

	const size_t count = sizeof(ports)/sizeof(ports[0]) - !isBFCPUDP;

	if (CanAllocate(count, isIceParty))
	{
		CPrettyTable<PortIndex, size_t, WORD, size_t> tbl("PortIndex", "ListIndex", "PortId", "FreePorts");

		for (size_t i = 0; i < count; ++i)
			AllocateUdpPort(partyID, ports[i].port, ports[i].additional, isIceParty, &tbl);

		TRACEINTO << "PartyId:" << partyID << ", IsIceParty:" << isIceParty << ", IsBfcpUDP:" << isBFCPUDP << " - Going to allocate " << count << " ports:" << tbl.Get();

		return STATUS_OK;
	}

	STATUS status = STATUS_FAIL;

	std::ostringstream msg;
	msg << " allocation failed, status=" << status << "\n";
	Dump(msg);

	TRACESTRFUNC(eLevelWarn) << msg.str().c_str();
	return status;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CPQperSrvResource::DeAllocateUDP(ResourceID partyID, bool disable/* = false*/)
{
	return FreeUdpPort(partyID, NULL, disable) ? STATUS_OK : STATUS_FAIL;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
STATUS CPQperSrvResource::DeAllocateUDP(
	ResourceID partyID,
	WORD& portAud,
	WORD& portVid,
	WORD& portFecc,
	WORD& portContent,
	WORD& portBFCPUDP,
	WORD& portAudAdditionalPorts,
	WORD& portVidAdditionalPorts,
	WORD& portFeccAdditionalPorts,
	WORD& portContentAdditionalPorts,
	WORD& portBFCPUDPAdditionalPorts,
	bool disable)
{
	struct PortData {
		WORD& port;
		WORD& additional;
	};

	PortData ports[] = {
		{ portAud, portAudAdditionalPorts },
		{ portVid, portVidAdditionalPorts },
		{ portFecc, portFeccAdditionalPorts },
		{ portContent, portContentAdditionalPorts },
		{ portBFCPUDP, portBFCPUDPAdditionalPorts },
	};

	bool ok = true;

	for (size_t i = 0; i < sizeof(ports)/sizeof(ports[0]); ++i)
	{
		if (ports[i].port)
			ok &= FreeUdpPort(partyID, &ports[i].port, disable);

		if (ports[i].additional)
			ok &= FreeUdpPort(partyID, &ports[i].additional, disable);
	}

	return ok ? STATUS_OK : STATUS_FAIL;
}

///////////////////////////////////////////////////////////////////////////
// class CIPServiceResources
///////////////////////////////////////////////////////////////////////////
CIPServiceResources::CIPServiceResources(WORD id, const char* name, BYTE default_H323_SIP_service)
	: m_serviceId(id)
	, m_default_H323_SIP_service(static_cast<DefaultServiceEnum>(default_H323_SIP_service))
{
	memset(m_serviceName, '\0', NET_SERVICE_PROVIDER_NAME_LEN);

	if (name)
	{
		strncpy(m_serviceName, name, sizeof(m_serviceName) - 1);
		m_serviceName[sizeof(m_serviceName) - 1] = '\0';
	}
}

//////////////////////////////////////////////////////////////////////////////
CIPServiceResources::CIPServiceResources(const CIPServiceResources& other)
	: CPObject(other)
	, m_serviceId(other.m_serviceId)
	, m_default_H323_SIP_service(other.m_default_H323_SIP_service)
	, m_PQperSrvResourceList(other.m_PQperSrvResourceList)
{
	strncpy(m_serviceName, other.m_serviceName, NET_SERVICE_PROVIDER_NAME_LEN);
}

//////////////////////////////////////////////////////////////////////////
CIPServiceResources::~CIPServiceResources()
{
}

//////////////////////////////////////////////////////////////////////////
bool operator==(const CIPServiceResources& lhs,const CIPServiceResources& rhs)
{
	return (lhs.m_serviceId == rhs.m_serviceId);
}

//////////////////////////////////////////////////////////////////////////
bool operator<(const CIPServiceResources& lhs, const CIPServiceResources& rhs)
{
	return (lhs.m_serviceId < rhs.m_serviceId);
}

//////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceResources::AddPQM(CPQperSrvResource* pPQM)
{
	CPrettyTable<WORD, WORD, WORD, WORD, WORD> tbl("BoxId", "BoardId", "SubBoardId", "SubServiceId", "PqId");

	for (std::set<CPQperSrvResource>::iterator i = m_PQperSrvResourceList.begin(); i != m_PQperSrvResourceList.end(); ++i)
		tbl.Add(i->GetBoxId(), i->GetBoardId(), i->GetSubBoardId(), i->GetSubServiceId(), i->GetPQMId());

	TRACEINTO
		<< "BoxId:" << pPQM->GetBoxId()
		<< ", BoardId:" << pPQM->GetBoardId()
		<< ", SubBoardId:" << pPQM->GetSubBoardId()
		<< ", SubServiceId:" << pPQM->GetSubServiceId()
		<< ", PqId:" << pPQM->GetPQMId()
		<< tbl.Get();

	if (m_PQperSrvResourceList.find(*pPQM) != m_PQperSrvResourceList.end())
	{
		PASSERT(1); // exists - trace?
		return STATUS_FAIL;
	}

	m_PQperSrvResourceList.insert(*pPQM);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceResources::RemovePQM(CPQperSrvResource* pPQM)
{
	TRACEINTO << "this:" << (std::hex) << this << (std::dec)
	          << ", boxId:" << pPQM->GetBoxId()
	          << ", boardId:" << pPQM->GetBoardId()
	          << ", subBoardId:" << pPQM->GetSubBoardId()
	          << ", subServiceId:" << pPQM->GetSubServiceId()
	          << ", PQid:" << pPQM->GetPQMId();

	if (m_PQperSrvResourceList.find(*pPQM) == m_PQperSrvResourceList.end())
	{
		PASSERT_AND_RETURN_VALUE(1, STATUS_FAIL); // not found - trace?
	}

	m_PQperSrvResourceList.erase(*pPQM);
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
STATUS CIPServiceResources::RemoveAllPQM()
{
	m_PQperSrvResourceList.clear();
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////////
const CPQperSrvResource* CIPServiceResources::GetPQM(const CPQperSrvResource& PQM) const
{
	std::set<CPQperSrvResource>::iterator i = m_PQperSrvResourceList.find(PQM);

	if (i == m_PQperSrvResourceList.end())//not exists
		return NULL;

	return (&(*i));
}

//////////////////////////////////////////////////////////////////////////////
bool CIPServiceResources::IsConfiguredToBoard(WORD boxId, WORD boardId, WORD subBoardId) const
{
	std::set<CPQperSrvResource>::iterator pq_itr;
	for (pq_itr = m_PQperSrvResourceList.begin(); pq_itr != m_PQperSrvResourceList.end(); ++pq_itr)
	{
		if (boxId == pq_itr->GetBoxId() && boardId == pq_itr->GetBoardId() && subBoardId == pq_itr->GetSubBoardId() && pq_itr->IsIpConfigured())
			return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////////
bool CIPServiceResources::IsConfiguredToBoard(WORD boardId) const
{
	std::set<CPQperSrvResource>::iterator pq_itr;
	for (pq_itr = m_PQperSrvResourceList.begin(); pq_itr != m_PQperSrvResourceList.end(); ++pq_itr)
	{
		if (boardId == pq_itr->GetBoardId() && pq_itr->IsIpConfigured())
			return true;
	}

	return false;
}

