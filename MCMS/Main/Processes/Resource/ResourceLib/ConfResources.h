#if !defined(AFX_CONFRSRC_H__BB14263F_471F_4E98_ADA4_06545537C069__INCLUDED_)
#define AFX_CONFRSRC_H__BB14263F_471F_4E98_ADA4_06545537C069__INCLUDED_

#include "SelfConsistency.h"
#include <string.h>
#include <set>
#include <bitset>
#include <functional>
#include "PObject.h"
#include "DataTypes.h"
#include "AllocateStructs.h"
#include "SystemResources.h"
#include "ObjString.h"

#define RsrcPartyIdType DWORD

////////////////////////////////////////////////////////////////////////////
//                        CUdpRsrcDesc
////////////////////////////////////////////////////////////////////////////
class CUdpRsrcDesc : public CPObject
{
	CLASS_TYPE_1(CUdpRsrcDesc, CPObject)

public:
	             CUdpRsrcDesc(WORD rsrcPartyId, WORD servId = 0, WORD subServId = 0, WORD PQMid = 0);
	virtual     ~CUdpRsrcDesc();

	const char*  NameOf() const                   { return "CUdpRsrcDesc"; }

	friend bool  operator ==(const CUdpRsrcDesc&, const CUdpRsrcDesc&);
	friend bool  operator <(const CUdpRsrcDesc&, const CUdpRsrcDesc&);

	void         SetRsrcPartyId(WORD rsrcPartyId) { m_rsrcPartyId = rsrcPartyId; };
	WORD         GetRsrcPartyId()                 { return m_rsrcPartyId;  };

	void         SetServId(WORD servId)           { m_servId = servId; };
	WORD         GetServId() const                { return m_servId;  };

	void         SetsubServId(WORD subServId)     { m_subServId = subServId; };
	WORD         GetsubServId() const             { return m_subServId;  };

	void         SetPQMId(WORD PQMid)             { m_PQMid = PQMid; };
	WORD         GetPQMId()                       { return m_PQMid;  };

	void         SetIpAddresses(ipAddressV4If ipAddressV4, const ipv6AddressArray& ipAddressV6);

private:
	WORD         m_rsrcPartyId;
	WORD         m_servId;
	WORD         m_subServId;
	WORD         m_PQMid;

public:
	UdpAddresses m_udp;
	APIU32       m_rtp_channels[MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS];  //ICE 4 ports-for pass channels
	APIU32       m_rtcp_channels[MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS]; //ICE 4 ports-for pass channels


};


////////////////////////////////////////////////////////////////////////////
//                        CRsrcDesc
////////////////////////////////////////////////////////////////////////////
class CRsrcDesc : public CPObject
{
	CLASS_TYPE_1(CRsrcDesc, CPObject)

public:
	                      CRsrcDesc(DWORD connectionId = 0, eLogicalResourceTypes type = eLogical_res_none, DWORD rsrcConfId = 0, WORD rsrcPartyId = 0,
	                                WORD boxId = 0, WORD bId = 0, WORD subBid = 0, WORD uid = 0, WORD acceleratorId = 0, WORD firstPortId = 0, ECntrlType cntrl = E_NORMAL,
	                                WORD channelId = 0, BOOL bIsUpdated = FALSE);

	virtual              ~CRsrcDesc();
	virtual const char*   NameOf() const                       { return "CRsrcDesc"; }

	friend bool operator  ==(const CRsrcDesc&, const CRsrcDesc&);
	friend bool operator  <(const CRsrcDesc&, const CRsrcDesc&);

	WORD                  GetBoxId() const                     { return m_boxId; };
	WORD                  GetBoardId() const                   { return m_boardId; };
	void                  SetBoardId( WORD bId )               { m_boardId = bId; };
	WORD                  GetSubBoardId() const                { return m_subBoardId; };
	WORD                  GetUnitId() const                    { return m_unitId; };
	void                  SetUnitId(WORD unitId)               { m_unitId = unitId; };
	WORD                  GetAcceleratorId() const             { return m_acceleratorId; };
	void                  SetAcceleratorId(WORD acceleratorId) { m_acceleratorId = acceleratorId; };
	WORD                  GetFirstPortId() const               { return m_firstPortId; };
	void                  SetFirstPortId(WORD portId)          { m_firstPortId = portId; };
	ipAddressV4If         GetIpAddrV4()                        { return m_IpV4Addr; };
	void                  SetIpAddresses(ipAddressV4If ipAddressV4);
	eLogicalResourceTypes GetType() const                      { return m_type; };
	eResourceTypes        GetPhysicalType() const;
	void                  SetNumPorts(WORD numPorts)           { m_numPorts = numPorts; };
	void                  SetRsrcConfId(DWORD rsrcConfId)      { m_rsrcConfId = rsrcConfId; };
	DWORD                 GetRsrcConfId() const                { return m_rsrcConfId;  };
	void                  SetRsrcPartyId(WORD rsrcPartyId)     { m_rsrcPartyId = rsrcPartyId; };
	WORD                  GetRsrcPartyId() const               { return m_rsrcPartyId;  };
	void                  SetConnId(WORD connectionId)         { m_connectionId = connectionId; };
	WORD                  GetConnId() const                    { return m_connectionId;  };
	void                  SetCntrlType(ECntrlType cntrl )      { m_cntrl = cntrl; };
	ECntrlType            GetCntrlType() const                 { return m_cntrl;  };
	void                  SetChannelId ( WORD channelId )      { m_channelId = channelId; }
	WORD                  GetChannelId() const                 { return m_channelId; }
	BOOL                  GetIsUpdated() const                 { return m_bIsUpdated; };
	void                  SetIsUpdated(BOOL bIsUpdated)        { m_bIsUpdated = bIsUpdated; };

	friend std::ostream&  operator<<(std::ostream& os, CRsrcDesc& obj);

private:
	DWORD                 m_connectionId;
	eLogicalResourceTypes m_type;
	DWORD                 m_rsrcConfId;
	WORD                  m_rsrcPartyId;
	WORD                  m_boxId;
	WORD                  m_boardId;
	WORD                  m_subBoardId;
	WORD                  m_unitId; //unitid manes spanid for Net ports
	WORD                  m_acceleratorId;
	WORD                  m_firstPortId;
	ipAddressV4If         m_IpV4Addr;
	WORD                  m_numPorts;
	ECntrlType            m_cntrl;
	WORD                  m_channelId;
	//This field will be used for Net descriptors only.
	//Updated means that this is the actual place of the port.
	//Updated will be true in two cases:
	//1. In dial-in, when we receive the first port in the allocate request
	//2. In dial-out and dial-in, after receiving an UPDATE_RTM_PORT_REQ transaction from ConfParty
	BOOL                  m_bIsUpdated;
};


////////////////////////////////////////////////////////////////////////////
//                        CPartyRsrc
////////////////////////////////////////////////////////////////////////////
class CPartyRsrc : public CPObject
{
	CLASS_TYPE_1(CPartyRsrc, CPObject)

public:
	                         CPartyRsrc(PartyMonitorID monitorPartyId, eNetworkPartyType networkPartyType = eNetwork_party_type_none,
	                                    eVideoPartyType videoPartyType = eVideo_party_type_none, WORD ARTChannels = 0, ePartyRole partyRole = eParty_Role_regular_party, eHdVswTypesInMixAvcSvc hdVswTypeInMixMode = eHdVswNon);

	                         CPartyRsrc(const CPartyRsrc& other);
	virtual                 ~CPartyRsrc();
	virtual const char*      NameOf() const                                    { return "CPartyRsrc"; }
	friend WORD operator ==  (const CPartyRsrc&, const CPartyRsrc&);
	friend bool operator <   (const CPartyRsrc&, const CPartyRsrc&);

	PartyMonitorID           GetMonitorPartyId() const                         { return m_monitorPartyId; };
	void                     SetMonitorPartyId(PartyMonitorID val)             { m_monitorPartyId = val; }

	PartyRsrcID              GetRsrcPartyId() const                            { return m_rsrcPartyId; };
	void                     SetRsrcPartyId(PartyRsrcID val)                   { m_rsrcPartyId = val; };

	eNetworkPartyType        GetNetworkPartyType() const                       { return m_networkPartyType; }
	void                     SetNetworkPartyType(eNetworkPartyType type)       { m_networkPartyType = type; }

	eVideoPartyType          GetVideoPartyType() const                         { return m_videoPartyType; }
	void                     SetVideoPartyType(eVideoPartyType type)           { m_videoPartyType = type; }

	ePartyResourceTypes      GetPartyResourceType() const                      { return m_resourcePartyType; }
	void                     SetPartyResourceType(ePartyResourceTypes type)    { m_resourcePartyType = type; }

	ePartyRole               GetPartyRole() const                              { return m_partyRole; }
	void                     SetPartyRole(ePartyRole partyRole)                { m_partyRole = partyRole; }

	WORD                     GetARTChannels() const                            { return m_ARTChannels; }
	void                     SetARTChannels(WORD ARTChannels);

	DWORD                    GetCSconnId() const                               { return m_CSconnId; }
	void                     SetCSconnId(DWORD CSconnId)                       { m_CSconnId = CSconnId; }

#ifdef MS_LYNC_AVMCU_LINK // MS Lync
	DWORD                    GetSigOrganizerConnId() const                     { return m_partySigOrganizerConnId; }
	void                     SetSigOrganizerConnId(DWORD val)                  { m_partySigOrganizerConnId = val; }
	DWORD                    GetSigFocusConnId() const                         { return m_partySigFocusConnId; }
	void                     SetSigFocusConnId(DWORD val)                      { m_partySigFocusConnId = val; }
	DWORD                    GetSigEventPackConnId() const                     { return m_partySigEventPackConnId; }
	void                     SetSigEventPackConnId(DWORD val)                  { m_partySigEventPackConnId = val; }
#endif

	DWORD                    GetRoomPartyId() const                            { return m_roomPartyId; }
	void                     SetRoomPartyId(DWORD val)                         { m_roomPartyId = val; }

	ETipPartyTypeAndPosition GetTIPPartyType() const                           { return m_tipPartyType; }
	void                     SetTIPPartyType(ETipPartyTypeAndPosition val)     { m_tipPartyType = val; }

	WORD                     GetDialInReservedPorts() const                    { return m_DialInReservedPorts; }
	void                     SetDialInReservedPorts(WORD val)                  { m_DialInReservedPorts = val; }

	WORD                     GetTipNumOfScreens() const                        { return m_tipNumOfScreens; }
	void                     SetTipNumOfScreens(WORD val)                      { m_tipNumOfScreens = val; }

	void                     SetCountPartyAsICEinMFW(BOOL val)                 { m_countPartyAsICEinMFW = val; }
	BOOL                     GetCountPartyAsICEinMFW() const                   { return m_countPartyAsICEinMFW; }

	eHdVswTypesInMixAvcSvc   GetHdVswTypeInMixAvcSvcMode() const               { return m_HdVswTypeInMixAvcSvcMode; }
	void                     SetHdVswTypeInMixMode(eHdVswTypesInMixAvcSvc val) { m_HdVswTypeInMixAvcSvcMode = val; }

private:
	PartyMonitorID           m_monitorPartyId;
	PartyRsrcID              m_rsrcPartyId;
	DWORD                    m_roomPartyId; //unique id for TIP Cisco
	ETipPartyTypeAndPosition m_tipPartyType;
	WORD                     m_tipNumOfScreens;
	eNetworkPartyType        m_networkPartyType;
	eVideoPartyType          m_videoPartyType;
	ePartyResourceTypes      m_resourcePartyType;
	ePartyRole               m_partyRole;
	WORD                     m_ARTChannels;
	DWORD                    m_CSconnId; // // CS connection id for IP parties

#ifdef MS_LYNC_AVMCU_LINK // MS Lync
	DWORD                    m_partySigOrganizerConnId;
	DWORD                    m_partySigFocusConnId;
	DWORD                    m_partySigEventPackConnId;
#endif
	WORD                     m_DialInReservedPorts;

	BOOL                     m_countPartyAsICEinMFW;
	eHdVswTypesInMixAvcSvc   m_HdVswTypeInMixAvcSvcMode;

public:
	DWORD                    m_ssrcAudio;
	DWORD                    m_ssrcContent[MAX_NUM_RECV_STREAMS_FOR_CONTENT];
	DWORD                    m_ssrcVideo[MAX_NUM_RECV_STREAMS_FOR_VIDEO];
};
/////////////////////////////////////////////////////////////////////////////

typedef std::map<PartyRsrcID, Phone*> BONDING_PHONES; // Party Bonding Temporary Phone Number

typedef std::multiset<CRsrcDesc> RSRC_DESC_MULTISET;
typedef std::multiset<CRsrcDesc>::iterator RSRC_DESC_MULTISET_ITR;

typedef std::set<CPartyRsrc> PARTIES;

////////////////////////////////////////////////////////////////////////////
//                        CConfRsrc
////////////////////////////////////////////////////////////////////////////
class CConfRsrc : public CPObject
{
	CLASS_TYPE_1(CConfRsrc, CPObject)

public:
	                            CConfRsrc(const CConfRsrc& other);
	                            CConfRsrc(ConfMonitorID monitorConfId, eSessionType sessionType = eCP_session, eLogicalResourceTypes = eLogical_res_none, eConfMediaType confMediaType = eConfMediaType_dummy);
	virtual                    ~CConfRsrc();
	virtual const char*         NameOf() const                   { return "CConfRsrc"; }


	friend WORD                 operator==(const CConfRsrc&, const CConfRsrc&);
	friend bool                 operator<(const CConfRsrc&, const CConfRsrc&);

	ConfMonitorID               GetMonitorConfId() const         { return m_monitorConfId; };
	eSessionType                GetSessionType() const           { return m_sessionType; };
	eLogicalResourceTypes       GetLogicalEncoderTypeCOP()       { return m_encoderTypeCOP; }

	ConfRsrcID                  GetRsrcConfId() const            { return m_rsrcConfId; };
	void                        SetRsrcConfId(ConfRsrcID confId) { m_rsrcConfId = confId; };

	WORD                        GetNumParties()                  { return m_num_Parties; };
	BOOL                        CheckIfOneMorePartyCanBeAddedToConf2C(eVideoPartyType videoPartyType);

	STATUS                      AddDesc(CRsrcDesc* pRsrcDesc);
	STATUS                      RemoveDesc(WORD rsrcpartyId, eLogicalResourceTypes type, ECntrlType cntrl = E_NORMAL, WORD portId = 0, WORD unitId = 0, WORD acceleratorId = 0, WORD boardId = 0);
	const CRsrcDesc*            GetDesc(WORD rsrcpartyId, eLogicalResourceTypes type, ECntrlType cntrl = E_NORMAL, WORD portId = 0, WORD unitId = 0, WORD acceleratorId = 0, WORD boardId = 0);
	WORD                        GetDescArrayPerResourceTypeByRsrcId(WORD rsrcpartyId, eLogicalResourceTypes type, CRsrcDesc** pRsrcDescArray, BYTE arrSize = MAX_NUM_ALLOCATED_RSRCS) const;
	CRsrcDesc*                  GetDescFromConnectionId(DWORD connectionId );
	int                         GetDescArray( CRsrcDesc**& pRsrcDescArray);
	const CRsrcDesc*            GetDescByType( eLogicalResourceTypes type, ECntrlType cntrl = E_NORMAL);

	WORD                        RemoveAllParties(BYTE force_kill_all_ports);
	STATUS                      AddParty(CPartyRsrc& party);
	STATUS                      RemoveParty(PartyMonitorID monitorPartyId);
	const CPartyRsrc*           GetParty(PartyMonitorID monitorPartyId) const;
	const CPartyRsrc*           GetPartyRsrcByRsrcPartyId(PartyRsrcID partyId) const;
	const PARTIES*              GetPartiesList() const { return &m_parties; }

	WORD                        GetNumVideoPartiesPerBoard( WORD boardId );
	// Temporary bonding phone numbers
	STATUS                      AddTempBondingPhoneNumber(PartyRsrcID partyId, Phone* pPhone);
	STATUS                      RemoveTempBondingPhoneNumber(PartyRsrcID partyId);
	Phone*                      GetTempBondingPhoneNumberByRsrcPartyId(PartyRsrcID partyId) const;
	BONDING_PHONES*             GetTempBondingPhoneNumbersMap() { return &m_bondingPhones; }

	//***udp
	STATUS                      AddUdpDesc(CUdpRsrcDesc* pUdpRsrcDesc);
	STATUS                      UpdateUdpDesc(WORD rsrcpartyId, CUdpRsrcDesc* pUdpRsrcDesc);
	STATUS                      RemoveUdpDesc(WORD rsrcpartyId);
	const CUdpRsrcDesc*         GetUdpDesc(WORD rsrcpartyId) const;

	WORD                        IsBoardIdAllocatedToConf(WORD boardId);

	void                        AddListOfPartiesDescriptorsOnBoard(WORD boardId, WORD subBoardId, std::map<std::string, CONF_PARTY_ID_S*>* listOfConfIdPartyIdPair) const;
	std::string                 GetPartyKey(DWORD rsrcConfId, DWORD rsrcPartyId) const;

	STATUS                      GetFreeVideoRsrcVSW(CRsrcDesc*& pEncDesc, CRsrcDesc*& pDecDesc);

	STATUS                      GetBoardUnitIdByRoomIdTIP(DWORD room_id, WORD& board, WORD& unit);
	STATUS                      GetBoardUnitIdByAvMcuLinkMain(PartyRsrcID mainPartyRsrcID, WORD& board, WORD& unit);

	WORD                        GetNumAllowedRsrcSameLRT(eLogicalResourceTypes lrt_type); //OLGA - SoftMCU
	WORD                        GetBoardIdForRelayParty();
	bool                        CheckIfThereAreRelayParty() const;
	bool                        CheckIfThereAreNotRelayParty() const;
	void                        SetConfMediaType(eConfMediaType type) { m_confMediaType = type; }
	eConfMediaType              GetConfMediaType() const              { return m_confMediaType; }
	void                        UpdateConfMediaStateByPartiesList();

	void                        SetConfMediaState(eConfMediaState newMediaState) { m_confMediaState = newMediaState; }
	eConfMediaState             GetConfMediaState() const                        { return m_confMediaState; }

	WORD                        GetMrcMcuId() const                              { return m_mrcMcuId; }
	void                        SetMrcMcuId(WORD value)                          { m_mrcMcuId = value; }

	void                        CountAvcSvcParties(WORD& numAvc, WORD& numSvc) const;
	WORD                        GetPartyRcrsDescArrayByRsrcId(DWORD rsrcpartyId, CRsrcDesc** pRsrcDescArray, WORD arrSize = MAX_NUM_ALLOCATED_RSRCS);
	WORD                        GetPartyRTPDescArrayByRsrcId(DWORD rsrcpartyId, CRsrcDesc** pRsrcDescArray, WORD arrSize = MAX_NUM_ALLOCATED_RSRCS);

private:

	struct compareVideoPartyFunc : public std::unary_function<CPartyRsrc, bool>
	{
		bool operator ()(const CPartyRsrc& rPartyRsrc) const
		{
			eVideoPartyType vpType = rPartyRsrc.GetVideoPartyType();
			return (eVideo_relay_CIF_party_type    == vpType ||
					eVideo_relay_SD_party_type     == vpType ||
					eVideo_relay_HD720_party_type  == vpType ||
					eVideo_relay_HD1080_party_type == vpType ||
					eVoice_relay_party_type == vpType);
		}
	};

	ConfMonitorID               m_monitorConfId;
	ConfRsrcID                  m_rsrcConfId;
	eSessionType                m_sessionType;
	WORD                        m_num_Parties;
	PARTIES                     m_parties;
	BONDING_PHONES              m_bondingPhones;
	RSRC_DESC_MULTISET          m_pRsrcDescList;
	std::set<CUdpRsrcDesc>*     m_pUdpRsrcDescList;
	eLogicalResourceTypes       m_encoderTypeCOP;
	eConfMediaState             m_confMediaState;
	eConfMediaType              m_confMediaType;
	WORD                        m_mrcMcuId;              // SIP Cascade

	friend class                CSelfConsistency;
};


typedef std::set<CConfRsrc> RSRC_CONF_LIST;


////////////////////////////////////////////////////////////////////////////
//                        CConfRsrcDB
////////////////////////////////////////////////////////////////////////////
class CConfRsrcDB : public CPObject
{
	CLASS_TYPE_1(CConfRsrcDB, CPObject)

public:
	CConfRsrcDB();
	virtual ~CConfRsrcDB();

	const char* NameOf() const { return "CConfRsrcDB"; }

	WORD                       GetNumConfRsrcs()        { return m_numConfRsrcs; };
	const RSRC_CONF_LIST* GetConfRsrcsList() const { return &m_confList; };

	STATUS     AddConfRsrc(CConfRsrc* pConfRsrc);
	STATUS     UpdateConfRsrc(CConfRsrc* pConfRsrc);
	STATUS     RemoveConfRsrc(ConfMonitorID confId);

	CConfRsrc* GetConfRsrc(ConfMonitorID confId);
	CConfRsrc* GetConfRsrcByRsrcConfId(ConfRsrcID confId);

	bool       IsExitingConf(ConfMonitorID confId);
	bool       IsEmptyConf(ConfMonitorID confId);

	ConfRsrcID MonitorToRsrcConfId(ConfMonitorID confId);

	PartyRsrcID MonitorToRsrcPartyId(ConfMonitorID confId, PartyMonitorID partyId);
	bool        GetPartyType(ConfMonitorID confId, PartyMonitorID partyId,
	                         eNetworkPartyType& networkPartyType, eVideoPartyType& videoPartyType, ePartyRole& partyRole,
	                         WORD& artChannels, eSessionType& sessionType);

	STATUS GetMonitorIdsRsrcIds(ConfRsrcID confId, PartyRsrcID partyId, ConfMonitorID& monitorConfId, PartyMonitorID& monitorPartyId);

	void GetAllPartiesOnBoard(WORD boardId, WORD subBoardId, std::map<std::string, CONF_PARTY_ID_S*>* listOfConfIdPartyIdPair);

	void FillISDNServiceName(ConfMonitorID confId, PartyMonitorID partyId, char serviceName[GENERAL_SERVICE_PROVIDER_NAME_LEN]);

private:
	WORD            m_numConfRsrcs;
	RSRC_CONF_LIST m_confList;

	friend class CSelfConsistency;
};

////////////////////////////////////////////////////////////////////////////////////////////////
#endif // !defined(AFX_CONFRSRC_H__BB14263F_471F_4E98_ADA4_06545537C069__INCLUDED_)
