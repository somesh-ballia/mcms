#if !defined(AFX_IPSERVICERESOURCES_H__92ED09F5_2F72_45AC_B16E_126A7723D270__INCLUDED_)
#define AFX_IPSERVICERESOURCES_H__92ED09F5_2F72_45AC_B16E_126A7723D270__INCLUDED_

#include <set>
#include <valarray>
#include <vector>
#include <map>

#include "PObject.h"
#include "DataTypes.h"
#include "Trace.h"
#include "Segment.h"
#include "AllocateStructs.h"
#include "CommonStructs.h"
#include "SystemFunctions.h"
#include "EnumsAndDefines.h"


enum DefaultServiceEnum
{
	DEFAULT_SERVICE_NONE = 0,
	DEFAULT_SERVICE_H323,
	DEFAULT_SERVICE_SIP,
	DEFAULT_SERVICE_BOTH
};

enum ePortAllocTypes
{
	PORT_OCCUPIED = 0,
	PORT_FREE,
	PORT_FREE_DISABLED,
	PORT_RESERVED,

	// it MUST be always LAST
	PORT_ALLOC_STATUSES_COUNT__
};

typedef std::valarray<ePortAllocTypes> CPortAllocTypeVector;

typedef DWORD ResourceID;

////////////////////////////////////////////////////////////////////////////
//                        CPQperSrvResource
////////////////////////////////////////////////////////////////////////////
class CPQperSrvResource : public CPObject
{
	CLASS_TYPE_1(CPQperSrvResource, CPObject)

	friend bool operator==(const CPQperSrvResource&, const CPQperSrvResource&);
	friend bool operator <(const CPQperSrvResource&, const CPQperSrvResource&);

public:
	                        CPQperSrvResource(BoxID boxId, BoardID boardId, SubBoardID subBoardId, WORD PQid, SubServiceID subServiceId = 0);

	virtual const char*     NameOf() const                                     { return "CPQperSrvResource"; }

	void                    Dump(std::ostream& msg) const;

	void                    SetServiceId(ServiceID serviceId)                  { m_serviceId = serviceId; }
	ServiceID               GetServiceId() const                               { return m_serviceId; }

	void                    SetBoxId(BoxID boxId)                              { m_boxId = boxId; }
	BoxID                   GetBoxId() const                                   { return m_boxId; }

	void                    SetBoardId(BoardID boardId)                        { m_boardId = boardId; }
	BoardID                 GetBoardId() const                                 { return m_boardId; }

	void                    SetSubBoardId(SubBoardID subBoardId)               { m_subBoardId = subBoardId; }
	SubBoardID              GetSubBoardId() const                              { return m_subBoardId; }

	void                    SetSubServiceID(SubServiceID subServiceId)         { m_subServiceId = subServiceId; }
	SubServiceID            GetSubServiceId() const                            { return m_subServiceId; }

	void                    SetPQMId(WORD PQid)                                { m_PQid = PQid; }
	WORD                    GetPQMId() const                                   { return m_PQid; }

	void                    SetType(WORD type)                                 { m_type = type; }
	WORD                    GetType() const                                    { return m_type; }

	void                    SetUDPallocType(eUdpAllocMethod alloctype)         { m_UDPallocType = alloctype; }
	eUdpAllocMethod         GetUDPallocType() const                            { return m_UDPallocType; }

	void                    SetIpType(eIpType type)                            { m_ipType = type; }
	eIpType                 GetIpType()                                        { return m_ipType; }

	void                    SetIpV4Addr(ipAddressV4If IpV4Addr)                { memcpy(&m_IpV4Addr, &IpV4Addr, sizeof(ipAddressV4If)); }
	ipAddressV4If           GetIpV4Addr() const                                { return m_IpV4Addr; }


	void                    SetIpV6Addr(const ipv6AddressArray& IpV6AddrArray) { memcpy(&m_IpV6AddrArray, &IpV6AddrArray, sizeof(ipv6AddressArray)); }
	const ipv6AddressArray& GetIpV6Addr() const                                { return m_IpV6AddrArray; }

	void                    SetUDPChannels(WORD first, WORD last);

	size_t                  GetFreePortsCount() const                          { return m_freePortsCount; }
	size_t                  GetDisabledPortsCount() const                      { return m_disabledPortsCount; }

	STATUS                  CanAllocateUDP(eVideoPartyType type, bool isBFCPUDP, bool isIceParty) const;

	STATUS                  AllocateUDP(
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
	                            bool isIceParty = false, bool isBFCPUDP = false); //ICE 4 ports

	STATUS                  AllocateAdditionalUDP(
	                            ResourceID partyID,
	                            WORD& portVid,
	                            WORD& portFecc,
	                            WORD& portContent,
	                            WORD& portBFCPUDP,
	                            WORD& portVidAdditionalPorts,
	                            WORD& portFeccAdditionalPorts,
	                            WORD& portContentAdditionalPorts,
	                            WORD& portBFCPUDPAdditionalPorts,
	                            bool isIceParty, bool isBFCPUDP); //ICE 4 ports

	STATUS                  DeAllocateUDP(
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
	                            bool disable = false); //ICE 4 ports

	STATUS                  DeAllocateUDP(ResourceID partyID, bool disable = false);

	bool                    IsIpV4Null() const             { return ::IsIpNull(m_IpV4Addr); }
	bool                    IsIpV6Null() const;
	bool                    IsIpConfigured() const         { return !(IsIpV4Null() && IsIpV6Null()); }

private:

	typedef WORD PortIndex;

	bool                    CanAllocate(size_t count, bool isIceParty) const;

	bool                    AllocateUdpPort(ResourceID partyID, WORD& port, WORD& additionalPort, bool isIceParty, void* pTbl);
	size_t                  FreeUdpPort(ResourceID partyID, WORD* port = NULL, bool disable = false);

	size_t                  PortsAllocationMapSize() const { return std::min(m_UDPChannelLast - m_UDPChannelFirst + 1, MAX_UDP_PORTS) / 2; }
	void                    ReinitFreePortsMap();

	WORD                    Index2Port(PortIndex i) const  { return 2 * i + m_UDPChannelFirst; }
	bool                    Port2Index(WORD port, PortIndex& i) const;

	void                    LinkCellToParty(ResourceID partyID, PortIndex i);
	bool                    UnlinkCellFromParty(ResourceID partyID, PortIndex& i, bool any = false);


private:
	typedef std::map<ResourceID, PortIndex> PartyPortsMap; // maps Party's Resource ID into one of its allocated UDP Ports
	typedef std::vector<PortIndex> PortsAllocationMap;     // linked list in array: every cell "points" to another one through the 1-based index, or is 0 == NULL == LAST in the list

	eUdpAllocMethod         m_UDPallocType;     //0 - static, 1 - dynamic
	ServiceID               m_serviceId;
	BoxID                   m_boxId;
	BoardID                 m_boardId;
	SubBoardID              m_subBoardId;
	WORD                    m_PQid;             //1,2,3,4
	WORD                    m_type;
	eIpType                 m_ipType;
	ipAddressV4If           m_IpV4Addr;
	ipv6AddressArray        m_IpV6AddrArray;
	WORD                    m_UDPChannelFirst;
	WORD                    m_UDPChannelLast;
	WORD                    m_subServiceId;
	PartyPortsMap           m_assignedPortsMap; // index to the 1st port assigned to the party
	PortsAllocationMap      m_allocationTable;
	size_t                  m_freePortsCount;
	size_t                  m_disabledPortsCount;
	PortIndex               m_headFree;     // 1-based; 0 means NULL
	PortIndex               m_headDisabled; // 1-base; 0 means NULL
	PortIndex               m_tailFree;
	PortIndex               m_tailDisabled;
};


////////////////////////////////////////////////////////////////////////////
//                        CIPServiceResources
////////////////////////////////////////////////////////////////////////////
class CIPServiceResources : public CPObject
{
	CLASS_TYPE_1(CIPServiceResources, CPObject)

	friend bool              operator==(const CIPServiceResources&, const CIPServiceResources&);
	friend bool              operator<(const CIPServiceResources&, const CIPServiceResources&);

public:
	                         CIPServiceResources(const CIPServiceResources& other);
	                         CIPServiceResources(WORD id, const char* name = NULL, BYTE default_H323_SIP_service = DEFAULT_SERVICE_NONE);
	                        ~CIPServiceResources();

	virtual const char*      NameOf() const                                    { return "CIPServiceResources"; }

	WORD                     GetServiceId() const                              { return m_serviceId; }
	void                     SetServiceId(WORD id)                             { m_serviceId = id; }

	const char*              GetName() const                                   { return m_serviceName; }

	STATUS                   AddPQM(CPQperSrvResource* pPQM);
	STATUS                   RemovePQM(CPQperSrvResource* pPQM);
	STATUS                   RemoveAllPQM();

	const CPQperSrvResource* GetPQM(const CPQperSrvResource& PQM) const;

	// network separation
	bool                     IsConfiguredToBoard(WORD boxId, WORD boardId, WORD subBoardId) const;
	bool                     IsConfiguredToBoard(WORD boardId) const;

	DefaultServiceEnum       GetDefaultH323SipService() const                  { return m_default_H323_SIP_service; }
	void                     SetDefaultH323SipService(BYTE type)               { m_default_H323_SIP_service = static_cast<DefaultServiceEnum>(type); }
	void                     SetDefaultH323SipService(DefaultServiceEnum type) { m_default_H323_SIP_service = type; }

private:

	WORD                        m_serviceId;
	char                        m_serviceName[NET_SERVICE_PROVIDER_NAME_LEN];

	DefaultServiceEnum          m_default_H323_SIP_service;

	std::set<CPQperSrvResource> m_PQperSrvResourceList;
};

#endif // !defined(AFX_IPSERVICERESOURCES_H__92ED09F5_2F72_45AC_B16E_126A7723D270__INCLUDED_)
