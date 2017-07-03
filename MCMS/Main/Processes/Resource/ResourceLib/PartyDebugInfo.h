#ifndef _PARTY_DEBUG_INFO_H_
#define _PARTY_DEBUG_INFO_H_

#include "ConfResources.h"
#include "SerializeObject.h"

////////////////////////////////////////////////////////////////////////////
//                        CPortDebugInfo
////////////////////////////////////////////////////////////////////////////
class CPortDebugInfo
{
	struct PortInfo { char debugStr[DEBUG_INFO_STRING_SIZE+1]; bool isFromMpl; };

public:
	                CPortDebugInfo(CRsrcDesc& rsrcDesc);
	                CPortDebugInfo(const CPortDebugInfo& other);

	CPortDebugInfo& operator= (const CPortDebugInfo& other);
	friend bool     operator==(const CPortDebugInfo& port_1, const CPortDebugInfo& port_2);
	friend bool     operator==(const CPortDebugInfo& port_1, const CRsrcDesc& desc_2);
	friend bool     operator< (const CPortDebugInfo& port_1, const CPortDebugInfo& port_2);
	friend std::ostream&  operator<<(std::ostream& os, CPortDebugInfo& obj);

	STATUS          SendInfoReq(ConfRsrcID confId, PartyRsrcID partyId) const;

	void            SetPortDebugInfo(char* portDebugInfo, bool receivedFromMpl = true);
	void            SetPortCmDebugInfo(char* portDebugInfo, bool receivedFromMpl = true);

	void            GetInfoString(std::ostream& infoString) const;
	bool            IsAllInfoReceived() const;
	bool            IsEqual(DWORD board_id, DWORD sub_board_id, DWORD unit_id, DWORD req_accelerator_id, DWORD port_id, DWORD conn_id);
	void            WhyNotEqual(DWORD req_board_id, DWORD req_sub_board_id, DWORD req_unit_id, DWORD req_accelerator_id, DWORD req_port_id, DWORD req_conn_id) const;

protected:
	CRsrcDesc       m_rsrcDesc;
	PortInfo        m_portInfo[2];
};


typedef std::set<CPortDebugInfo*> PortDebugInfoList;

////////////////////////////////////////////////////////////////////////////
//                        CPartyDebugInfo
////////////////////////////////////////////////////////////////////////////
class CPartyDebugInfo : public CStateMachine
{
	CLASS_TYPE_1(CUnitRsrc, CPartyDebugInfo )

public:
	enum STATE { IN_PROGRESS = (IDLE + 1), COMPLETED };

	                  CPartyDebugInfo(ConfMonitorID monitor_conf_id, PartyMonitorID monitor_party_id, ConfRsrcID resource_conf_id, PartyRsrcID resource_party_id);
	                  CPartyDebugInfo(const CPartyDebugInfo& other);
	virtual          ~CPartyDebugInfo();
	const char*       NameOf() const { return "CPartyDebugInfo"; }

	void*             GetMessageMap();
	void              HandleEvent(CSegment* pMsg, DWORD msgLen, OPCODE opCode);

	CPartyDebugInfo&  operator= (const CPartyDebugInfo& other);
	friend bool       operator==(const CPartyDebugInfo& party_1, const CPartyDebugInfo& party_2);
	friend bool       operator< (const CPartyDebugInfo& party_1, const CPartyDebugInfo& party_2);

	STATUS            AddPortInfo(const CPortDebugInfo& portDebugInfo);
	void              SendInfoReq();

	void              PortInfoInd(bool isCmInfo, BoardID boardId, SubBoardID subBoardId, UnitID unitId, AcceleratorID acceleratorId, PortID portId, ConnectionID connId, char* info);

	void              PortCmInfoInd(CRsrcDesc& rsrcDesc, DEBUG_INFO_IND_S& debugInfo);

	void              GetInfoString(std::ostream& msg) const;
	STATUS            GetStatus() const;
	bool              IsAllInfoReceived() const;
	void              InfoCompleted();

	ConfMonitorID     GetMonitorConfId() const   { return m_monitorConfId; }
	PartyMonitorID    GetMonitorPartyId() const  { return m_monitorPartyId; }
	ConfRsrcID        GetResourceConfId() const  { return m_confId; }
	PartyRsrcID       GetResourcePartyId() const { return m_partyId; }

	bool              IsSameParty(ConfRsrcID confId, PartyRsrcID partyId);

	bool              IsInfoRetrieved() const    { return m_isInfoRetrieved; }
	void              SetInfoRetrieved();

	void              OnTimerComplete(CSegment* pSeg);
	void              OnRetriveInfoTimer(CSegment* pSeg);

protected:
	ConfMonitorID     m_monitorConfId;
	PartyMonitorID    m_monitorPartyId;
	ConfRsrcID        m_confId;
	PartyRsrcID       m_partyId;
	bool              m_isInfoRetrieved;
	PortDebugInfoList m_portsDebugInfoList;

	PDECLAR_MESSAGE_MAP
};


////////////////////////////////////////////////////////////////////////////
//                        CPartyPortsInfo
////////////////////////////////////////////////////////////////////////////
class CPartyPortsInfo : public CSerializeObject
{
public:
	                  CPartyPortsInfo();
	                  CPartyPortsInfo(const char* infoStr);
	const char*       NameOf() const        { return "CPartyPortsInfo"; }

	CSerializeObject* Clone()               { return new CPartyPortsInfo; }
	void              SerializeXml(CXMLDOMElement*& thisNode) const;
	int               DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action);

	ConfRsrcID        GetConfID() const     { return m_confId; }
	PartyRsrcID       GetPartyID() const    { return m_partyId; }
	BOOL              GetIsSendToCM() const { return m_bIsSendToCM; }

private:
	ConfRsrcID        m_confId;
	PartyRsrcID       m_partyId;
	BOOL              m_bIsSendToCM;
	string            m_debugInfo;
};

#endif // _PARTY_DEBUG_INFO_H_
