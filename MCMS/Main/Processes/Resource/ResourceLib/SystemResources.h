#if !defined(AFX_SYSTEMRESOURCES_H__DA95D6C6_46D3_4708_ADDB_CB6AB72DC5A6__INCLUDED_)
  #define AFX_SYSTEMRESOURCES_H__DA95D6C6_46D3_4708_ADDB_CB6AB72DC5A6__INCLUDED_

#include <bitset>
#include <set>
#include <vector>
#include <map>

#include "SerializeObject.h"
#include "PObject.h"
#include "Trace.h"
#include "SharedMcmsCardsStructs.h"
#include "DataTypes.h"
#include "Macros.h"
#include "StatusesGeneral.h"
#include "CsCommonStructs.h"
#include "AllocateStructs.h"
#include "IPServiceResources.h"
#include "HostCommonDefinitions.h"
#include "RtmIsdnMngrInternalStructs.h"
#include "InnerStructs.h"
#include "CdrApiClasses.h"
#include "Board.h"
#include "DefinesGeneral.h"
#include "ProductType.h"
#include "ResourcesInterfaceArray.h"
#include "dsp_monitor_getter.h"
#include "LookupId.h"
#include "MsSsrc.h"
#include "Unit.h"

class CRsrcDetailElement;
class CRsrcReport;
class CNetServicesDB;
class CNetPortsPerService;
class CRsrcDesc;
class CAllocationDecider;
class CConnToCardManager;
class CInterval;
class CIntervalRsrvAmount;
class CConfRsrvRsrc;
class CEnhancedConfig;
class CEnhancedConfigResponse;
class CAllocationModeDetails;
class CServicesRsrcReport;

#ifndef min
  #define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define ID_ALL_IP_SERVICES   0xFFFF

#define AUDIO_PAYLOAD_TYPE   0     // 000000                        // OLGA - SoftMCU
#define VIDEO_PAYLOAD_TYPE   16    // 010000, 17=010001, 18=010010  // OLGA - SoftMCU
#define FECC_PAYLOAD_TYPE    47    // 101111                        // OLGA - SoftMCU
#define CONTENT_PAYLOAD_TYPE 48    // 110000, 110001                // OLGA - SoftMCU


// temp., global
eUnitType PhysicalToUnitType(eResourceTypes type);
ePortType    ResourceToPortType(eResourceTypes type);


class CActivePort; // defined below. separate file???***
class CConfRsrc;
class CAcivePort;

typedef std::set<CActivePort> CActivePortsList;
typedef std::map<ePartyResourceTypes, size_t> PartyResourceTypesCounter;

enum eChangedStateAC
{
	eAC_None        = 0,
	eAC_Failed      = 1,
	eAC_Back        = 2,
	NUM_OF_AC_STATE = 3 // DONT FORGET TO UPDATE THIS
};


////////////////////////////////////////////////////////////////////////////
struct PhysicalPortDesc
{
	WORD m_boxId;
	WORD m_boardId;
	WORD m_subBoardId;
	WORD m_unitId;
	WORD m_portId;
	WORD m_acceleratorId;
	ipAddressV4If m_IpAddrV4;
	ipAddressV6If m_IpAddrV6;
};

struct eSystemCPUDesc
{
	eSystemCPUSize m_CpuSize;
	DWORD m_CpuCapacity;
};


////////////////////////////////////////////////////////////////////////////
//                        CPhone
////////////////////////////////////////////////////////////////////////////
class CPhone : public CPObject
{
	CLASS_TYPE_1(CPhone, CPObject)

public:
	            CPhone();
	            CPhone(const CPhone&);
	            CPhone(const char*);
	           ~CPhone();
	void        Dump(std::ostream& msg) const;
	const char* NameOf() const              { return "CPhone"; }

	friend WORD operator ==(const CPhone&, const CPhone&);
	friend bool operator <(const CPhone&, const CPhone&);

	const char* GetNumber() const           { return &m_pNumber[0];                               }
	void        SetNumber(const char* numb) { strncpy(m_pNumber, numb, PHONE_NUMBER_DIGITS_LEN);  }
	WORD        IsEmpty() const             { return (m_pNumber[0] ? FALSE : TRUE);               }
	void        SetIsBusy(WORD isBusy)      { m_busy = isBusy;                                    }
	WORD        IsBusy() const              { return m_busy;                                      }

	BOOL        IsSimilarTo(const char* pSimilarToThisString) const;

private:
	char* m_pNumber;
	WORD  m_busy;
};


////////////////////////////////////////////////////////////////////////////
//                        PhoneHelper
////////////////////////////////////////////////////////////////////////////
class PhoneHelper
{
public:
	static void CutPhone(char* strPhone, char* strPrefix, DWORD& suffix, int& suffixlength);
	static void GluePhone(char* strPhone, char* strPrefix, DWORD suffix, int suffixlength);
};


////////////////////////////////////////////////////////////////////////////
//                        SBoardIpAdresses
////////////////////////////////////////////////////////////////////////////
struct SBoardIpAdresses
{
	WORD boardId;
	ipAddressV4If ipAdressRTMV4;
	ipAddressV4If ipAdressMediaV4;
};


////////////////////////////////////////////////////////////////////////////
//                        CNetServiceRsrcs
////////////////////////////////////////////////////////////////////////////
class CNetServiceRsrcs : public CPObject
{
	CLASS_TYPE_1(CNetServiceRsrcs, CPObject)

public:
	                     CNetServiceRsrcs(const char* name, WORD isNFAS = FALSE);
	                     CNetServiceRsrcs(const CNetServiceRsrcs& other);
	virtual             ~CNetServiceRsrcs();
	virtual const char*  NameOf() const                                     { return "CNetServiceRsrcs";  }
	const char*          GetName() const                                    { return m_pName;             }

	friend WORD operator ==(const CNetServiceRsrcs&, const CNetServiceRsrcs&);
	friend bool operator <(const CNetServiceRsrcs&, const CNetServiceRsrcs&);

	eRTMSpanType         GetSpanType()                                      { return m_spanType;          }
	void                 SetSpanType(eRTMSpanType spanType)                 { m_spanType = spanType;      }

	STATUS               EnablePhone(char* num);
	STATUS               DisablePhone(char* num);

	STATUS               CapturePhone(const char* num);
	STATUS               CapturePhone(ULONGLONG num);
	STATUS               AllocatePhone(char*& num, const char* pSimilarToThisString);
	STATUS               DeAlocatePhone(char* num);

	int                  AllocatePort(WORD& port_num, WORD& bId, WORD& uId) { return STATUS_OK;             }
	int                  DeAllocatePort(WORD port_num, WORD bId, WORD uId)  { return STATUS_OK;             }

	std::set<CPhone>*    GetPhonesList() const                              { return m_pPhoneslist;         }
	CNetPortsPerService* GetNetPortsPerService()                            { return m_pNetPortsPerService; }

private:
	char*                            m_pName;
	eRTMSpanType                     m_spanType;    // service either T1 or E1 for all its' spans
	WORD                             m_isNFAS;
	std::set<CPhone>*                m_pPhoneslist; // dial-in phones.
	std::map<ULONGLONG, std::string> m_pPhonesMap;  //VNGFE-7414

public:
	SBoardIpAdresses                 m_ipAddressesList[MAX_NUM_OF_BOARDS];
	CNetPortsPerService*             m_pNetPortsPerService;
};


////////////////////////////////////////////////////////////////////////////
//                        CSleepingConference
////////////////////////////////////////////////////////////////////////////
// class storage for numeric / monitor conf id / pstn phones alloc
class CSleepingConference : public CPObject
{
	CLASS_TYPE_1(CSleepingConference, CPObject)

public:
	                  CSleepingConference();
	                  CSleepingConference(DWORD monitorConfId);
	                  CSleepingConference(const CSleepingConference&);
	virtual          ~CSleepingConference();
	const char*       NameOf(void) const;

	friend WORD       operator==(const CSleepingConference& lhs, const CSleepingConference& rhs);
	friend bool       operator<(const CSleepingConference& lhs, const CSleepingConference& rhs);

	DWORD             GetMonitorConfId() const              { return m_monitorConfId;           }
	void              SetMonitorConfId(DWORD monitorConfId) { m_monitorConfId = monitorConfId;  }
	char*             GetNumConfId() const                  { return m_numConfId;               }
	void              SetNumConfId(char* numConfId);

	STATUS            AddServicePhone(const CServicePhoneStr& other);
	STATUS            DeleteServicePhone(const CServicePhoneStr& other);
	int               FindServicePhone(const CServicePhoneStr& other);
	CServicePhoneStr* GetFirstServicePhone();
	CServicePhoneStr* GetNextServicePhone();

	eRsrcConfType     GetConfType() const                   { return m_conf_type;       }
	void              SetConfType(eRsrcConfType conf_type)  { m_conf_type = conf_type;  }

	// reservation related functionality
	void              GetConferenceNeededAmount(CIntervalRsrvAmount& confNeededAmount) const;

protected:
	DWORD             m_monitorConfId;
	char*             m_numConfId;
	eRsrcConfType     m_conf_type;
	WORD              m_numServicePhoneStr;
	CServicePhoneStr* m_pServicePhoneStr[MAX_NET_SERV_PROVIDERS_IN_LIST];
	WORD              m_ind_service_phone;

private:
	const CSleepingConference& operator =(const CSleepingConference& other);

};

STATUS  CreateControllerRecord(WORD boardId, WORD unitId, DWORD connId, eResourceTypes r_type, ECntrlType rsrcCntlType = E_NORMAL);


////////////////////////////////////////////////////////////////////////////
//                        CAudioVideoConfig
////////////////////////////////////////////////////////////////////////////
class CAudioVideoConfig : public CSerializeObject
{
	CLASS_TYPE_1(CAudioVideoConfig, CSerializeObject)

public:
	                    CAudioVideoConfig(WORD aud = 0, WORD vid = 0) { m_audio = aud; m_video = vid; }
	                    CAudioVideoConfig(const CAudioVideoConfig& other);
	                   ~CAudioVideoConfig(){ }
	const char*         NameOf() const                                { return "CAudioVideoConfig"; }
	void                Dump(std::ostream& msg) const;

	CSerializeObject*   Clone()                                       { return new CAudioVideoConfig; }
	void                SerializeXml(CXMLDOMElement*& thisNode, WORD Id) const;
	void                SerializeXml(CXMLDOMElement*& thisNode) const { }
	int                 DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action);

	friend WORD         operator==(const CAudioVideoConfig& lhs, const CAudioVideoConfig& rhs);

	WORD                GetAudio() const                              { return m_audio;  }
	void                SetAudio(WORD audio)                          { m_audio = audio; }

	WORD                GetVideo() const                              { return m_video;  }
	void                SetVideo(WORD video)                          { m_video = video; }

	CAudioVideoConfig& operator=(const CAudioVideoConfig& audVidConfig);

private:
	WORD m_audio; // Audio Participants
	WORD m_video; // Video Participants
};


typedef std::vector< CAudioVideoConfig > PORTS_CONFIG_LIST;

////////////////////////////////////////////////////////////////////////////
//                        CPortsConfig
////////////////////////////////////////////////////////////////////////////
class CPortsConfig : public CSerializeObject
{
	CLASS_TYPE_1(CPortsConfig, CSerializeObject)

public:
	                   CPortsConfig();
	                   CPortsConfig(const CPortsConfig& other);
	                  ~CPortsConfig();
	const char*        NameOf() const                       { return "CPortsConfig"; }

	CSerializeObject*  Clone()                              { return new CPortsConfig; }
	void               SerializeXml(CXMLDOMElement*& thisNode) const;
	int                DeSerializeXml(CXMLDOMElement* pNode, char* pszError, const char* action);

	WORD               GetSelectedIndex() const             { return m_selectedIndex;          }
	void               SetSelectedIndex(WORD selectedIndex) { m_selectedIndex = selectedIndex; }

	STATUS             SetDongleRestriction(DWORD dongleRestriction);
	DWORD              GetDonglePortsRestriction() const    { return m_DongleRestriction;      }

	PORTS_CONFIG_LIST* GetpPortsConfigList() const          { return m_pPortsConfigList;       }

	STATUS             ResetConfigurationListAccordingToLicenseAndCards();
	STATUS             GenerateConfigurationList(DWORD max_hd720_ports);

	void               SetMaxHD720PortsAccordingToCards(DWORD hd720_ports);
	STATUS             GetStatus();
	bool               FindPortsConfigurationIndexByConfig(const CAudioVideoConfig& aud_vid, size_t& index) const;
	CAudioVideoConfig* FindPortsConfigurationConfigByIndex(size_t index) const;

private:
	WORD               m_selectedIndex;                    // Selected configuration from the list
	PORTS_CONFIG_LIST* m_pPortsConfigList;                 // Ports Configuration options list
	DWORD              m_DongleRestriction;
	DWORD              m_LastHD720PortsAccordingToCardsRestriction;
	DWORD              m_LastGeneratedConfigurationListMaxHD720Ports;
};


typedef std::bitset<MAX_CHANNEL_IDS>  CHANNELS_IDS;

////////////////////////////////////////////////////////////////////////////
//                        CRtmChannelIds
////////////////////////////////////////////////////////////////////////////
// Used as pool for RTM channel Ids (per RTM board)
class CRtmChannelIds : public CPObject
{
	CLASS_TYPE_1(CRtmChannelIds, CPObject)

public:
	            CRtmChannelIds();
	            CRtmChannelIds(WORD boxId, WORD bId, WORD subBoardId);
	           ~CRtmChannelIds();
	const char* NameOf() const                 { return "CRtmChannelIds"; }

	WORD        GetBoxId() const               { return m_boxId; }
	void        SetBoxId(WORD boxId)           { m_boxId = boxId; }

	WORD        GetBoardId() const             { return m_boardId; }
	void        SetBoardId(WORD boardId)       { m_boardId = boardId; }

	WORD        GetSubBoardId() const          { return m_subBoardId; }

	WORD        Allocate();
	STATUS      DeAllocate(WORD channelId);
	bool        GetIsAllocated(WORD channelId) { return ((*m_pChannelIds)[channelId - 1] ? TRUE : FALSE); }

	// back to original code (before old implementation of SlotsNumbering feature)...
	void        SetSubBoardId(WORD subBoardId)
	{
		if (RTM_ISDN_SUBBOARD_ID != subBoardId)
			PASSERT(subBoardId);
		m_subBoardId = RTM_ISDN_SUBBOARD_ID;
	}
private:
	WORD          m_boxId;
	WORD          m_boardId;
	WORD          m_subBoardId;
	CHANNELS_IDS* m_pChannelIds;              // range is 1-360
	WORD          m_nextChannelId;
};


struct CUnitKey
{
	WORD fpga_index;
	BYTE unit_turbo;
	BYTE unit_video;
};

////////////////////////////////////////////////////////////////////////////
//                        CUnitKey_Comparator
////////////////////////////////////////////////////////////////////////////
class CUnitKey_Comparator : binary_function<CUnitKey, CUnitKey, bool>
{
public:
	bool operator ()(const CUnitKey& lhs, const CUnitKey& rhs) const
	{
		if (lhs.fpga_index < rhs.fpga_index)
			return true;

		if (lhs.fpga_index == rhs.fpga_index && rhs.unit_turbo < lhs.unit_turbo)
			return true;

		if (lhs.fpga_index == rhs.fpga_index && rhs.unit_turbo == lhs.unit_turbo && rhs.unit_video < lhs.unit_video)
			return true;

		return false;
	}
};


typedef std::multimap<CUnitKey, CUnitMFA*, CUnitKey_Comparator> CFittedUnitsMap;
typedef std::multimap<CUnitKey, CUnitMFA*>::iterator CFittedUnitsItr;
typedef CFittedUnitsMap::value_type CFittedUnitsPair;

enum PriorityType
{
	PriorityType_TurboDSP    = 0,
	PriorityType_NonTurboDSP = 1,
	PriorityType_AnyEmptyDSP = 2
};


////////////////////////////////////////////////////////////////////////////
//                        CSystemResources
////////////////////////////////////////////////////////////////////////////
class CSystemResources : public CPObject
{
	CLASS_TYPE_1(CSystemResources, CPObject)

public:
	CSystemResources();
	virtual ~CSystemResources();
	virtual const char*                 NameOf() const                    { return "CSystemResources"; }

	int                                 EnableUnit(BoardID boardId, UnitID unitId, eUnitType unitType, bool& isController);
	STATUS                              SetUnitMfaStatus(WORD boardId, WORD unitId, BYTE enbl_st, BYTE isManually = FALSE, BYTE isFatal = FALSE);
	STATUS                              GetUnitMfaStatus(WORD boardId, WORD unitId, BYTE& enbl_st);
	STATUS                              SetSpanRTMStatus(WORD boardId, WORD unitId, BYTE enbl_st, BYTE isManually = FALSE, BYTE isFatal = FALSE);
	STATUS                              SetBoardMfaStatus(WORD boardId, BYTE enbl_st, BYTE isManually = FALSE);
	void                                SetSlotNumberingConversionTable(SLOTS_NUMBERING_CONVERSION_TABLE_S* pSlotNumberingConversionTable);
	SLOTS_NUMBERING_CONVERSION_TABLE_S* GetSlotNumberingConversionTable() { return &m_SlotNumberingConversionTable; }

	WORD                                GetDisplayBoardId(WORD bid, WORD subBid);
	BOOL                                GetIDsFromDisplayBoardId(WORD displayBID, eProductType prodType, WORD& bid, WORD& subBid);

	int                                 EnableNetService(char* name /* phones */);
	int                                 DisableNetService(char* name);

	STATUS                              EnableSpan(WORD boardId, WORD unitId, eRTMSpanType SpanType);
	STATUS                              DisableSpan(WORD boardId, WORD unitId);

	CBoard*                             GetBoard(WORD bId) const;                                                                                                         // 1-based index!!!!
	CBoard*                             GetBoardByDisplayId(WORD displayId);
	CUnitMFA*                           GetUnit(WORD bId, WORD uid);                                                                                                      // bid: 1-based index!!!!
	CUnitMFA*                           GetNinjaUnit(WORD displayBoardId, WORD displayUnitId);                                                                            //displayBoardId: 6-8, displayUnitId: 0-5
	const CNetServiceRsrcs*             findServiceByName(const char* name);
	CNetPortsPerService*                FindNetPortsPerServiceByName(const char* serviceName);

	STATUS                              ConfigureResources(WORD num_units, RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S* pConfigParamsList, CM_UNITS_CONFIG_S* pRetConfigParams); // system resource configuration at startup
	int                                 ConfigureResourcesNew(WORD num_units, RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S* pConfigParamsList, CM_UNITS_CONFIG_S* pRetConfigParams);
	void                                SetKeepAliveList(RSRCALLOC_KEEP_ALIVE_S* pKeepAliveList, eChangedStateAC& stateAC);
	STATUS                              SetFipsInd(RSRCALLOC_ART_FIPS_140_IND_S* pFipsInd);

	void                                SetIpMediaConfigFailure(DWORD boardId, DWORD PQnumber);

	STATUS                              ConfigureIPServicePQResources(IP_SERVICE_UDP_RESOURCES_S* pIPService, UDP_PORT_RANGE_S& udpPortRange);
	STATUS                              DeleteIPService(Del_Ip_Service_S* pIPService);
	void                                IPv6ServiceUpdate(IPV6_ADDRESS_UPDATE_RESOURCES_S* pIPv6AddressUpdate);

	ConfRsrcID                          AllocateRsrcConfId()                    { return m_confIds.Alloc(); }
	void                                DeAllocateRsrcConfId(ConfRsrcID confId) { m_confIds.Clear(confId); }

	ConnectionID                        AllocateConnId()                        { return m_connectionIds.Alloc(); }
	void                                DeAllocateConnId(ConnectionID connId)   { m_connectionIds.Clear(connId); }

	RoomID                              AllocateRoomId()                        { return m_roomIds.Alloc(); }
	void                                DeAllocateRoomId(RoomID roomId)         { m_roomIds.Clear(roomId); }

	DWORD                               AllocateSSRCId(unsigned short dmaId, unsigned short mrmId);
	bool                                CanAllocateSSRC(unsigned short dmaId, unsigned short mrmId, WORD ssrc);
	unsigned int                        GetDmaMrmMask(unsigned short dmaId, unsigned short mrmId, WORD ssrc, WORD mask);

	STATUS                              AllocateART(ConfRsrcID confId, PartyRsrcID partyId, DWORD partyCapacity, eResourceTypes physType, WORD serviceId, WORD subServiceId, PhysicalPortDesc* pPortDesc, PartyDataStruct& partyData, BOOL isIceParty, BOOL isBFCPUDP,  WORD reqBoardId); // ICE 4 ports
	STATUS                              DeAllocateART(CConfRsrc* pConf, WORD rsrcPartyId, WORD ARTChannels, eResourceTypes physType, eVideoPartyType videoPartyType, PhysicalPortDesc* pPortDesc, BYTE dsbl = FALSE, WORD numOfTipScreens = 0);
	STATUS                              ReAllocateART(DWORD rsrcConfId, WORD rsrcPartyId, ALLOC_PARTY_REQ_PARAMS_S* pParam);
	WORD                                FindARTunits(CBoard* pBoard, PartyDataStruct& partyData, eUnitType needed_type, CUnitMFA**& return_find_arr, BOOL isTIPSlave = FALSE);

	STATUS                              AllocateVideo(DWORD rsrcConfId, WORD rsrcPartyId, AllocData& videoAlloc, PartyDataStruct& partyData);
	STATUS                              DeAllocateVideo(CConfRsrc* pConf, WORD rsrcPartyId, AllocData& videoAlloc, eVideoPartyType videoPartyType, BYTE dsbl = FALSE);
	STATUS                              AllocateVideo2C(DWORD rsrcConfId, AllocData& videoAlloc);
	STATUS                              DeAllocateVideo2C(CConfRsrc* pConf, CRsrcDesc* pDesc, BYTE dsbl);

	static BYTE                         IsTheSamePCI(WORD unitId1, WORD unitId2);
	BYTE                                IsNeighbOnPCIEmpty(WORD boardId, WORD unitId);
	STATUS                              AllocateRTMOneChannel(DWORD rsrcConfId, WORD rsrcPartyId, BOOL bIsDialOut, WORD reqBoardId, WORD reqUnitId, PhysicalPortDesc* pPortDesc);
	STATUS                              DeAllocateRTMOneChannel(CConfRsrc* pConf, WORD rsrcPartyId, PhysicalPortDesc* pPortDesc, BOOL bIsUpdated = TRUE);
	bool                                IsBoardIdExists(BoardID boardId);
	STATUS                              UpdateActivePort(WORD Bid, WORD Uid, WORD portId, DWORD rsrcConfId, DWORD rsrcPartyId);
	STATUS                              UpdateActiveRtmPort(WORD Bid, WORD Uid, WORD portId, DWORD rsrcConfId, DWORD rsrcPartyId);
	STATUS                              GetBestARTAndCheckUDP(CUnitMFA** find_arr, WORD serviceId, WORD subServiceId, WORD arr_size, DWORD partyCapacity, float needed_capacity, eVideoPartyType videoPartyType, WORD& found_id, BOOL isIceParty = FALSE, BOOL isBFCPUDP = FALSE, WORD tipNumOfScreens = 0); // ICE 4 ports
	void                                GetBestARTSpreadUnits(CUnitMFA** find_arr, WORD arr_size, WORD reqTipNumOfScreens, WORD& best_id);
	void                                GetBestARTDontSpreadUnits(CUnitMFA** find_arr, WORD arr_size, WORD& best_id);
	void                                GetBestARTSmartSpreadUnits(CUnitMFA** find_arr, WORD arr_size, WORD tipNumOfScreens, float needed_promilles, WORD& best_id);
	void                                GetBestARTCapacitySpreadUnits(CUnitMFA** find_arr, WORD arr_size, DWORD partyCapacity, WORD& best_id);

	STATUS                              AddIPService(WORD id, char* name = NULL, BYTE default_H323_SIP_service = 0);
	STATUS                              RemoveIPService(WORD id);
	const CIPServiceResources*          GetIPService(WORD id) const;
	const CIPServiceResources*          GetFirstIPService() const;
	const CIPServiceResources*          GetIPServiceByBoardId(WORD board_id) const;

	STATUS                              EnablePQMonService(WORD servId, CPQperSrvResource* pPQM);
	STATUS                              DisablePQMonService(WORD servId, CPQperSrvResource& PQM);

	CPQperSrvResource*                  ARTtoPQM(WORD servId, WORD subServiceId, WORD boxId, WORD boardId, WORD subBoardId, WORD unitId) const;

	WORD                                SimulateAllocateUDP(UdpAddresses& udp);
	STATUS                              SimulateDeAllocateUDP(WORD id);

	BOOL                                IsBoardBlngToSubSevice(WORD boardId, PartyDataStruct& partyData);

	CNetServicesDB*                     GetNetServicesDB() const                              { return m_pNetServicesDB; }
	ResourcesInterface*                 GetCurrentResourcesInterface() const;

	STATUS                              CheckUDPports(WORD servId, WORD subServiceId, WORD boxId, WORD boardId, WORD subBoardId, WORD unitId,  eVideoPartyType videoPartyType, BOOL isIceParty = FALSE, BOOL isBFCPUDP = FALSE); // ICE 4 ports
	void                                SetDongleRestriction(DWORD num_parties);

	void                                SetFederalFlag(BOOL federal)                          { m_Federal = federal; }
	BOOL                                GetFederalFlag()                                      { return m_Federal; }

	void                                SetEventMode(BOOL eventMode)                          { m_eventMode = eventMode; }
	BOOL                                IsEventMode()                                         { return m_eventMode; }

	void                                SetSvcFlag(BOOL svc)                                  { m_SVC = svc; }
	BOOL                                GetSvcFlag()                                          { return m_SVC; }

	STATUS                              CalculateSetConfiguration();
	BYTE                                IsSameUnitOnPCI(CM_UNITS_CONFIG_S* pRetConfigParams, WORD i, eCardUnitTypeConfigured unitType);
	BYTE                                IsArtOnSamePCI(WORD boardId, WORD unitId);

	BOOL                                IsThereAnyParty();

	void                                SetStartupCond(eStartupCondType condType, BYTE value);
	STATUS                              IsStartupOk();
	STATUS                              IsRsrcEnough(bool isKeepAlive = FALSE);
	STATUS                              IsIVRmountEnough()                                    { return STATUS_OK; /*logic TBD*/ } // *** ivr mount
	STATUS                              IsCondOk(eStartupCondType type);
	STATUS                              CalculateResourceReport(CRsrcReport* pReport);
	STATUS                              CalculateConfResourceReport(CSharedRsrcConfReport* pReport);

	DWORD                               GetNumConfiguredArtPorts(WORD boardId);                                                   // boardId of ALL_BOARD_IDS (0xFF) returns total art ports on all boards
	DWORD                               GetNumConfiguredVideoPorts(WORD boardId);                                                 // boardId of ALL_BOARD_IDS (0xFF) returns total video ports on all boards
	void                                SetNumConfiguredArtPorts(DWORD art, WORD boardId)     { m_num_configured_ART_Ports[boardId - 1] = art; }
	void                                SetNumConfiguredVideoPorts(DWORD video, WORD boardId) { m_num_configured_VIDEO_Ports[boardId - 1] = video; }

	// ivr mount, mfa complete
	STATUS                              SetMfaStartupComplete(BoardID boardId, DWORD PQ1status, DWORD PQ2status);
	STATUS                              CompleteResourcesStartup();
	bool                                IsBoardReady(BoardID boardId);
	BYTE                                IsResourcesStartupOver()                              { return m_ResourcesStartupOver; }
	void                                SetResourcesStartupOver(BYTE isOver)                  { m_ResourcesStartupOver = isOver; }

	STATUS                              PartyParamsToPhysicalLocation(DWORD ConfId, DWORD PartyId, eResourceTypes typePhysical,
	                                                                  WORD& bId, WORD& uId, WORD& acceleratorId, WORD& portId);
	DWORD                               GetIsSystemRecording()                                { return m_isRecording; }
	void                                SetIsSystemRecording(DWORD isRecording)               { m_isRecording = isRecording; }

	// Ports Configuration and allocation mode
	STATUS                              GetOrCheckEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg, CEnhancedConfigResponse* pResponse) const;
	STATUS                              SetEnhancedConfiguration(CEnhancedConfig* pEnhancedCfg);
	void                                OnReconfigureUnitsTimer();
	STATUS                              GetAllocationMode(CAllocationModeDetails* pAllocationModeResponse);
	STATUS                              SetAllocationMode(CAllocationModeDetails* pAllocationModeRequest);

	// RTM
	STATUS                              CreateRTMBoard(RTM_ISDN_ENTITY_LOADED_S* pRTMBoard);
	STATUS                              ConfigureRTMService(RTM_ISDN_PARAMS_MCMS_S* pRTMService);
	STATUS                              DeleteRTMService(RTM_ISDN_SERVICE_CANCEL_S* pRTMServiceToDelete);
	STATUS                              AddRTMPhoneNumberRange(RTM_ISDN_PHONE_RANGE_UPDATE_S* pRTMPhoneNumberRange);
	STATUS                              DeleteRTMPhoneNumberRange(RTM_ISDN_PHONE_RANGE_UPDATE_S* pRTMPhoneNumberRange);
	STATUS                              ConfigureRTMSpan(SPAN_ENABLED_S* pRTMSpan);
	STATUS                              ChangeSpanToNullConfigure(SPAN_DISABLE_S* pSpan);
	STATUS                              DisableAllRtmSpanPerBoard(RTM_ISDN_BOARD_ID_S* pIsdnBId);

	STATUS                              AllocateServicePhones(CServicePhoneStr& phoneStr, const char* pSimilarToThisString = NULL);
	STATUS                              DeAllocateServicePhones(CServicePhoneStr& phoneStr);

	STATUS                              FindVideoUnits(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, BOOL& canVideoRequestBeSatisfied);
	STATUS                              FindOptUnits(PartyDataStruct& partyData, eVideoPartyType videoPartyType, VideoDataPerBoardStruct& videoAlloc, BoardID boardId);
	STATUS                              FindOptUnitsStartingFromSpecificUnitId(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int startUnitId, BOOL is_spread_video_ports);
	STATUS                              FindOptBreezeUnit(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex, BOOL is_spread_video_ports);
	STATUS                              FindOptBreezeUnit(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex, BOOL is_spread_video_ports, BOOL is_turbo_needed);
	STATUS                              FindOptBreezeUnits(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndexMaster, int unitIndexSlave);
	STATUS                              FindOptNetraUnit(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex);
	STATUS                              FindOptSoftUnit(PartyDataStruct& partyData, VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex, BOOL is_spread_video_ports);
	STATUS                              FindVideoUnitsCOP(eSessionType sessnType, eLogicalResourceTypes encType, VideoDataPerBoardStruct& videoAlloc, int reqBoardId);
	STATUS                              FindOptUnitsCOP(eSessionType sessnType, eLogicalResourceTypes encType, VideoDataPerBoardStruct& videoAlloc, int reqBoardId);
	STATUS                              FindOptBreezeUnitCOP(VideoDataPerBoardStruct& videoAlloc, int reqBoardId, int unitIndex); // Breeze-COP
	STATUS                              FindOptUnitsAdditonalVideoAvcSvcMix(PartyDataStruct& partyData, eVideoPartyType videoPartyType, VideoDataPerBoardStruct& videoAlloc, int reqBoardId);

	void                                DumpBoardUnits(VideoDataPerBoardStruct& videoAlloc, BoardID boardId);

	void                                CollectVideoAvailabilityForBoard2C(eSessionType sessnType, eLogicalResourceTypes encType,
	                                                                       VideoDataPerBoardStruct& videoAlloc, int boardId);
	STATUS                              GetBestBoardsCOP(eSessionType sessnType, eLogicalResourceTypes encType, BestAllocStruct& bestAllocStruct, WORD ip_service_id = ID_ALL_IP_SERVICES);
	STATUS                              GetBestARTBoardCOP(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, int videoBoardId, WORD ip_service_id = ID_ALL_IP_SERVICES);
	STATUS                              GetBestBoardsVSW(eSessionType sessnType, BestAllocStruct& bestAllocStruct1, BestAllocStruct& bestAllocStruct2, WORD ip_service_id = ID_ALL_IP_SERVICES);

	float                               GetFreeVideoCapacity(WORD boardId, float* freePortCapacity = NULL);
	WORD                                GetNumVideoParties(WORD boardId, DWORD monitor_conf_id);
	WORD                                GetHD720PortsAccordingToCards(WORD* numBoards = NULL);
	WORD                                GetSvcPortsPerCards(WORD* numBoards = NULL);

	BOOL                                IsBetterBandWidth(DWORD needed_bandwidth_in, DWORD needed_bandwidth_out, DWORD current_bandwidth_in, DWORD current_bandwidth_out, DWORD max_bandwidth_in, DWORD max_bandwidth_out);

	BOOL                                CanUseReconfigurationForAllocation(PartyDataStruct& partyData);
	STATUS                              GetRemainingArtOnBoard(WORD boardId, WORD neededArtChannels, int& num_parties, DWORD artCapacity, PartyDataStruct& rPartyRsrc, WORD tipNumOfScreens = 0, bool neededArtChannelsPerUnit = true);
	STATUS                              GetBestBoards(PartyDataStruct& partyData, BestAllocStruct& bestAllocStruct, WORD ip_service_id = ID_ALL_IP_SERVICES);
	STATUS                              GetBestAllocationForRTMOnly(PartyDataStruct& partyData, ISDN_PARTY_IND_PARAMS_S* pIsdn_Params_Response, int numOfRTMPorts, int notOnThisBoard, int preferablyOnThisBoard);
	void                                CollectDataForBoard(DataPerBoardStruct& allocStruct, PartyDataStruct& partyData, int boardId);
	void                                CollectArtAvailabilityForBoard(ARTDataPerBoardStruct& artAlloc, PartyDataStruct& partyData, int boardId);
	void                                CollectVideoAvailabilityForBoard(VideoDataPerBoardStruct& videoAlloc, PartyDataStruct& partyData, int boardId, BOOL& canVideoRequestBeSatisfied);
	void                                CollectBandWidthAvailabilityForBoard(VideoDataPerBoardStruct& videoAlloc, PartyDataStruct& partyData, int boardId);
	void                                CollectRTMAvailabilityForBoard(RTMDataPerBoardStruct& rtmAllocStruct, PartyDataStruct& partyData, int boardId);
	void                                CollectAdditonalAvcSvcMixVideoAvailabilityForBoard(VideoDataPerBoardStruct& videoAlloc, PartyDataStruct& partyData, BoardID boardId, bool& canVideoRequestBeSatisfied);

	WORD                                AllocateChannelId(BoardID boardId);
	STATUS                              DeAllocateChannelId(BoardID boardId, ChannelID channelId);
	bool                                GetIsChannelIdAllocated(BoardID boardId, ChannelID channelId);

	// Hot Swap
	void                                RemoveCard(BoardID boardId, SubBoardID subBoardId);
	BYTE                                DisableAllUnits(WORD boardId, WORD subBoardId);

	STATUS                              HotSwapIvrAndAudioControllersAdd(DWORD boardId, BYTE& wasAudioControllerMasterUpdated, BYTE& wasAudioControllerSlaveUpdated, CConnToCardManager* pConnToCardManager);
	STATUS                              HotSwapIvrAndAudioControllersUpdate();
	STATUS                              HotSwapIvrAndAudioControllersRemove(DWORD boardId, WORD subBoardId, BYTE& wasAudioControllerMasterUpdated, BYTE& wasIvrControllerUpdated, CConnToCardManager* pConnToCardManager);

	BYTE                                IsThereExistUtilizableUnitForAudioController();

	WORD                                GetAudioCntrlMasterBid() const                         { return m_AudioCntrlMasterBid;                      }
	void                                SetAudioCntrlMasterBid(WORD audioMasterCntrlMasterBid) { m_AudioCntrlMasterBid = audioMasterCntrlMasterBid; }

	WORD                                GetAudioCntrlSlaveBid() const                          { return m_AudioCntrlSlaveBid;                     }
	void                                SetAudioCntrlSlaveBid(WORD audioMasterCntrlSlaveBid)   { m_AudioCntrlSlaveBid = audioMasterCntrlSlaveBid; }

	WORD                                GetIvrCntrlBid() const                                 { return m_IVRCntrlBid;        }
	void                                SetIvrCntrlBid(WORD ivrCntrlBid)                       { m_IVRCntrlBid = ivrCntrlBid; }

	WORD                                GetUtilizablePQUnitIdPerBoardId(WORD boardId);

	BYTE                                DisableAllUnitsAndSpans(WORD boardId, WORD subBoardId);

	WORD                                GetCapacity(DWORD boardId, WORD subBoardId, WORD UnitId = 0xFFFF);

	eResourceAllocationTypes            GetResourceAllocationType() const                      { return m_ResourceAllocationType;         }
	WORD                                GetMaxNumberOfOngoingConferences()                     { return m_maxNumberOfOngoingConferences;  }
	WORD                                GetMaxNumOngoingConfPerCardsInEventMode();
	WORD                                GetMaxNumberVSWConferencesEventMode() const;

	int                                 GetPortsConfigurationStep()                            { return m_PortsConfigurationStep; }
	float                               GetAudioFactor();

	eProductType                        GetProductType()                                       { return m_ProductType; }
	void                                InitProductType(eProductType pProductType);

	void                                SetRamSize(eSystemRamSize ramSize);
	eSystemRamSize                      GetRamSize()                                           { return m_RamSize; }

	void                                SetCpuSize(WORD cpuSize);

	eSystemCardsMode                    GetSystemCardsMode()                                   { return m_SystemCardsMode;    }
	void                                SetSystemCardsMode(eSystemCardsMode curMode)           { m_SystemCardsMode = curMode; }

	BYTE                                GetMultipleIpServices() const;
	void                                SetMultipleIpServices(BYTE isMultipleIpServices)       { m_isMultipleIpServices = isMultipleIpServices; }

	DWORD                               GetPortGauge()                                         { return m_portGauge;        }
	void                                SetPortGauge(DWORD i_portGauge)                        { m_portGauge = i_portGauge; }

	void                                InitResourceAllocationMode(eSystemCardsMode curMode);

	void                                FineTuneUnitsConfiguration();
	STATUS                              CheckEnhancedConfigurationWithCurUnitsConfig();
	void                                CheckSetEnhancedConfiguration();

	STATUS                              ShowNumArtChannels(WORD b_id, std::ostream& answer);
	STATUS                              FindBestRsrvAC(WORD& BestRsrvACBId, WORD& BestAcUnitID);
	void                                SwapMasterAcAndReserveAc(WORD prevMasterBoardID, WORD prevReservedBoardID);
	void                                DumpBrdsState(DataPerBoardStruct* pAllocDataPerBoardArray);
	void                                DumpBrdsStateCmd(std::ostream& answer);

	void                                FreeAllOccupiedPorts();

	WORD                                GetMaxUnitsNeededForVideo() const;
	WORD                                GetMaxRequiredPortsPerMediaUnit() const;
	WORD                                GetTotalMaxVideoResources() const;

	STATUS                              ReserveRecoveryART(WORD boardId);
	BOOL                                IsPcmMenuIdExist(DWORD rsrcPartyId) const;
	void                                UnregisterTaskStateMachines();

	void                                SetAllFreePortsToFreeDisable(PhysicalPortDesc* pPortDesc);

	// network separation
	WORD                                GetNumOfIpServicesConfiguredToBoard(WORD boardId);
	WORD                                GetSystemNumOfMfaUnits(BOOL withIpServiceOnly = FALSE);
	float                               GetIpServiceNumOfMfaUnits(WORD service_id);
	const CIPServiceResources*          GetIpService(WORD service_id);
	BOOL                                IsIpServiceConfiguredToBoard(WORD service_id, WORD board_id);
	float                               GetAllIpServicesNumOfMfaUnits();
	float                               GetIpServicesWeight(WORD service_id);
	STATUS                              CreateIpServiceInterface(WORD service_id, const char* service_name);
	STATUS                              InitIpServicesInterfaces();
	STATUS                              UpdateIpServicesDongleRestriction();

	CIpServiceResourcesInterfaceArray*  GetIpServiceInterface(WORD service_id);
	BOOL                                CheckIfOneMorePartyCanBeAdded(eVideoPartyType& videoPartyType, ePartyRole& partyRole, EAllocationPolicy allocPolicy, ALLOC_PARTY_IND_PARAMS_S* pResult,
	                                                                  WORD ipServiceId, BYTE rmxPortGaugeThreshold = FALSE, BOOL* pbAddAudioAsVideo = NULL,
	                                                                  eConfModeTypes confModeType = eNonMix, BOOL countPartyAsICEinMFW = FALSE);
	void                                RemoveParty(BasePartyDataStruct& rPartyData);
	void                                AddParty(BasePartyDataStruct& rPartyData);
	STATUS                              CalculateServicesResourceReport(CServicesRsrcReport* pReport);
	WORD                                GetPartyIpServiceId(DWORD party_rsrc_id);
	const char*                         GetIpServiceName(DWORD service_id);
	DWORD                               GetIpServiceId(const char* service_name);

	const char*                         GetDefaultIpService(DWORD& service_id, BYTE default_type = DEFAULT_SERVICE_H323);
	void                                SetDefaultIpService(const char* defaultH323serv, const char* defaultSIPserv);
	void                                SetPortConfigurationPerIpServices(WORD selectedIndex);

	WORD                                CalculatePortsConfigurationStep() const;
	float                               CalculateAudioFactorStep() const;

	STATUS                              UpdatePortWeightsTo1500Q();
	DWORD                               GetDongleNumOfParties() const;
	void                                SetDongleNumOfParties(DWORD dongleNumParties);
	WORD                                GetDSPCount();
	ULONG                               GetDSPLocationBitmap();

	void                                DumpRtmSpans() const;
	STATUS                              CheckIfConfVideoCanBeUpgraded(size_t (&additionalPorts)[ePartyType_Max][NUM_OF_PARTY_RESOURCE_TYPES]);
	STATUS                              CheckIfConfVideoCanBeUpgraded(WORD cif_ports, WORD sd_ports, WORD hd_ports, WORD svc_ports, WORD num_of_svc_1080p, WORD num_of_svc_720p, WORD num_of_svc_SD);

	bool                                AllocateMsSsrc(PartyRsrcID partyId, DWORD& first, DWORD& last) { return m_MsSsrcAllocator.Alloc(partyId, first, last); }
	void                                DeAllocateMsSsrc(PartyRsrcID partyId)                          { m_MsSsrcAllocator.Clear(partyId); }

	bool                                IsSystemCpuProfileDefined()                                    { return (m_CpuSizeDesc.m_CpuCapacity > 0); }
	DWORD                               GetMcuCapacity() const                                         { return m_CpuSizeDesc.m_CpuCapacity; }
	void                                UpdateLogicalWeightsAndValues();
	bool                                isLicenseExpired()                                             { return m_isLicenseExpired; }
	bool                                SetLicenseExpired(bool isLicenseExpired);

private:
	STATUS                              TestIfSpanIsRemovable(WORD bId, WORD uId, CSpanRTM*& pSpanFound);
	STATUS                              TestIfSpanIsRemovable(CSpanRTM* pSpan);
	STATUS                              TestIfPhoneNumberRangeIsRemovable(char* strPrefix, DWORD first, DWORD last, int suffixLength);
	STATUS                              TestIfServiceIsRemovable(const char* serviceName, CNetServiceRsrcs*& pService);
	STATUS                              TestIfPhoneNumberIsRemovable(char* s_phone);

	STATUS                              findTwoSuitableUnitsForSplittedEncoder(CFittedUnitsMap & mapFittedUnits, CFittedUnitsItr(&_iiFound)[2], PriorityType priorityType);
	bool                                IsUnitAlreadySelectedForAllocation(VideoDataPerBoardStruct& videoAlloc, int mediaUnitIndex, WORD unitId);

	char*                               GetServiceName(ISDN_SPAN_PARAMS_S& isdn_params);
	void                                ChangeResourceAllocationModePerIpServices(eResourceAllocationTypes newResourceAllocationType);

	void                                DumpRtmSpans(WORD boardId) const;

	CNetServicesDB*                              m_pNetServicesDB;
	CAllocationDecider*                          m_pAllocationDecider;
	ResourcesInterface*                          m_pResourcesInterface[MAX_NUMBER_OF_RESOURCES_TYPE];
	CResourcesInterfaceArray                     m_ResourcesInterfaceArray;
	std::set<CIpServiceResourcesInterfaceArray>* m_IpServicesResourcesInterface;
	std::set<CIPServiceResources>*               m_pIPServices;

	CLookupId<MAX_RSRC_CONF_IDS>                 m_confIds;
	CLookupId<MAX_ROOM_IDS>                      m_roomIds;
	CLookupId<MAX_CONN_IDS>                      m_connectionIds;

	CBoard*                                      m_pBoards[BOARDS_NUM];
	SLOTS_NUMBERING_CONVERSION_TABLE_S           m_SlotNumberingConversionTable;
	WORD                                         m_AudioCntrlMasterBid;
	WORD                                         m_AudioCntrlSlaveBid;
	WORD                                         m_IVRCntrlBid;
	BOOL                                         m_Federal;
	BOOL                                         m_eventMode;
	BOOL                                         m_SVC;
	BOOL                                         m_TIP;
	DWORD                                        m_num_configured_ART_Ports[BOARDS_NUM];
	DWORD                                        m_num_configured_VIDEO_Ports[BOARDS_NUM];
	BOOL                                         m_bIsRsrcEnough;         // will be set each time we call IsRsrcEnough
	UdpAddresses                                 UdpAdressesArray[3];
	BYTE                                         FreeFixedUDP[3];
	BYTE                                         m_StartupEndCondArray[NumOfStartupCondTypes];
	BYTE                                         m_ResourcesStartupOver;  // TRUE, after alarms removed
	DWORD                                        m_isRecording;           // 0 = OFF, 1 = ON ; media recording in system
	std::set<CRecordingJunction>*                m_pRecordingJunction;
	eResourceAllocationTypes                     m_ResourceAllocationType;
	eAllocationModeType                          m_FutureMode;
	eSystemCardsMode                             m_SystemCardsMode;
	eProductType                                 m_ProductType;
	eSystemRamSize                               m_RamSize;
	eSystemCPUDesc                               m_CpuSizeDesc;
	WORD                                         m_maxNumberOfOngoingConferences;
	int                                          m_PortsConfigurationStep;
	float                                        m_AudioFactor;
	DWORD                                        m_portGauge;
	BYTE                                         m_isMultipleIpServices;

	WORD                                         m_nextSSRCId;        //OLGA - SoftMCU
	unsigned long                                m_DspAliveBitmap;    // Ninja
	unsigned long                                m_DspLocationBitmap; // Ninja

	CMsSsrcAllocator                             m_MsSsrcAllocator;
	bool                                         m_isLicenseExpired;

	friend class CSelfConsistency;
	friend class CCardResourceConfig;
};

#endif // !defined(AFX_SYSTEMRESOURCES_H__DA95D6C6_46D3_4708_ADDB_CB6AB72DC5A6__INCLUDED_)
