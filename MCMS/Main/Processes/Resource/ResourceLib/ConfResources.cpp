#include "ConfResources.h"
#include "StatusesGeneral.h"
#include "ProcessBase.h"
#include "TraceStream.h"
#include "ResourceManager.h"
#include "WrappersResource.h"
#include "HelperFuncs.h"
#include "VideoApiDefinitionsStrings.h"
#include <algorithm>
#include <functional>

////////////////////////////////////////////////////////////////////////////
//                        CUdpRsrcDesc
////////////////////////////////////////////////////////////////////////////
CUdpRsrcDesc::CUdpRsrcDesc(WORD rsrcPartyId, WORD servId, WORD subServId, WORD PQMid)
{
	m_rsrcPartyId = rsrcPartyId;
	m_servId      = servId;
	m_PQMid       = PQMid;
	m_subServId   = subServId;
	memset(&m_udp, 0, sizeof(UdpAddresses));

	for (int i = 0; i < MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS; i++)
	{
		m_rtp_channels[i]  = 0;
		m_rtcp_channels[i] = 0;
	}
}

////////////////////////////////////////////////////////////////////////////
CUdpRsrcDesc::~CUdpRsrcDesc()
{
}

////////////////////////////////////////////////////////////////////////////
bool operator==(const CUdpRsrcDesc& lhs,const CUdpRsrcDesc& rhs)
{
	return ((lhs.m_rsrcPartyId == rhs.m_rsrcPartyId) && (lhs.m_type == rhs.m_type ));
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CUdpRsrcDesc& lhs,const CUdpRsrcDesc& rhs)
{
	if (lhs.m_rsrcPartyId != rhs.m_rsrcPartyId)
		return (lhs.m_rsrcPartyId < rhs.m_rsrcPartyId);

	return (lhs.m_type < rhs.m_type);
}

////////////////////////////////////////////////////////////////////////////
void CUdpRsrcDesc::SetIpAddresses(ipAddressV4If ipAddressV4, const ipv6AddressArray& ipAddressV6)
{
	CIPV4Wrapper v4wrapper(m_udp.IpV4Addr);
	v4wrapper.CopyData(ipAddressV4);

	CIPV6AraryWrapper v6wrapper(m_udp.IpV6AddrArray);
	v6wrapper.CopyData(ipAddressV6);
}


////////////////////////////////////////////////////////////////////////////
//                        CRsrcDesc
////////////////////////////////////////////////////////////////////////////
CRsrcDesc::CRsrcDesc(DWORD connectionId, eLogicalResourceTypes type, DWORD rsrcConfId, WORD rsrcPartyId,
					 WORD boxId ,  WORD bId, WORD subBid, WORD uid, WORD acceleratorId, WORD firstPortId, ECntrlType cntrl,
					 WORD channelId, BOOL bIsUpdated /*= FALSE*/ )
{
	m_connectionId  = connectionId;
	m_type          = type;
	m_rsrcConfId    = rsrcConfId;
	m_rsrcPartyId   = rsrcPartyId;
	m_boxId         = boxId;
	m_boardId       = bId;
	m_subBoardId    = subBid;
	m_unitId        = uid;
	m_acceleratorId = acceleratorId;
	m_firstPortId   = firstPortId;
	m_numPorts      = 1;
	m_cntrl         = cntrl;
	m_channelId     = channelId;
	m_bIsUpdated    = bIsUpdated;

	memset(&m_IpV4Addr, 0, sizeof(m_IpV4Addr));
}

////////////////////////////////////////////////////////////////////////////
CRsrcDesc::~CRsrcDesc()
{
}

////////////////////////////////////////////////////////////////////////////
bool operator==(const CRsrcDesc& lhs,const CRsrcDesc& rhs)
{
	//net ports can be the same except for the board, portid, unit_id (=span id) and accelerator_id, so check this
	if (lhs.m_type == eLogical_net && rhs.m_type == eLogical_net)
		if (lhs.m_firstPortId != rhs.m_firstPortId || lhs.m_unitId != rhs.m_unitId || lhs.m_acceleratorId != rhs.m_acceleratorId || lhs.m_boardId != rhs.m_boardId)
			return FALSE;

	return ((lhs.m_rsrcPartyId == rhs.m_rsrcPartyId) &&
			(lhs.m_type == rhs.m_type ) && (lhs.m_cntrl == rhs.m_cntrl));
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CRsrcDesc& lhs,const CRsrcDesc& rhs)
{
	if (lhs.m_rsrcPartyId != rhs.m_rsrcPartyId)
		return (lhs.m_rsrcPartyId < rhs.m_rsrcPartyId);
	else if (lhs.m_type != rhs.m_type)
		return (lhs.m_type < rhs.m_type);
	else if (lhs.m_type != eLogical_net)
		return (lhs.m_cntrl < rhs.m_cntrl);
	else //if it's net type
	{
		if (lhs.m_firstPortId != rhs.m_firstPortId)
			return (lhs.m_firstPortId < rhs.m_firstPortId);
		else if (lhs.m_acceleratorId != rhs.m_acceleratorId)
			return (lhs.m_acceleratorId < rhs.m_acceleratorId);
		else if (lhs.m_unitId != rhs.m_unitId)
			return (lhs.m_unitId < rhs.m_unitId);
		else
			return (lhs.m_boardId < rhs.m_boardId);
	}
}

////////////////////////////////////////////////////////////////////////////
void CRsrcDesc::SetIpAddresses(ipAddressV4If ipAddressV4)
{
	CIPV4Wrapper v4wrapper(m_IpV4Addr);
	v4wrapper.CopyData(ipAddressV4);
}

////////////////////////////////////////////////////////////////////////////
eResourceTypes CRsrcDesc::GetPhysicalType() const
{
	switch (m_type)
	{
		case eLogical_audio_encoder:
			return ePhysical_art;

		case eLogical_audio_decoder:
			return ePhysical_art;

		case eLogical_audio_controller:
			return ePhysical_audio_controller;

		case eLogical_video_encoder:
		case eLogical_video_encoder_content:
		case eLogical_COP_CIF_encoder:
		case eLogical_COP_VSW_encoder:
		case eLogical_COP_PCM_encoder:
		case eLogical_COP_HD720_encoder:
		case eLogical_COP_HD1080_encoder:
		case eLogical_COP_4CIF_encoder:
			return ePhysical_video_encoder;

		case eLogical_video_decoder:
		case eLogical_COP_Dynamic_decoder:
		case eLogical_COP_VSW_decoder:
		case eLogical_COP_LM_decoder:
			return ePhysical_video_decoder;

		case eLogical_rtp:
			return ePhysical_art;

		/*Not in use for now
		case eLogical_ip_signaling:
			return ePhysical_res_none;
		*/

		case eLogical_net:
			return ePhysical_rtm;

		case eLogical_ivr_controller:
			return ePhysical_ivr_controller;

		case eLogical_mux:
			return ePhysical_art;

		// OLGA - Soft MCU
		case eLogical_relay_rtp:
		case eLogical_relay_svc_to_avc_rtp:
		case eLogical_relay_video_encoder:
			return ePhysical_mrmp;

		case eLogical_relay_audio_encoder:
		case eLogical_relay_audio_decoder:
		case eLogical_legacy_to_SAC_audio_encoder:
		case eLogical_relay_avc_to_svc_rtp:
		case eLogical_relay_avc_to_svc_rtp_with_audio_encoder:
			return ePhysical_art;

		case eLogical_relay_avc_to_svc_video_encoder_1:
		case eLogical_relay_avc_to_svc_video_encoder_2:
			return ePhysical_video_encoder;

		default:
			return ePhysical_res_none;
	}
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CRsrcDesc& obj)
{
	os
		<< "\n  ConnectionId  :" << obj.m_connectionId
		<< "\n  ResourceType  :" << obj.m_type
		<< "\n  ConfId        :" << obj.m_rsrcConfId
		<< "\n  PartyId       :" << obj.m_rsrcPartyId
		<< "\n  BoxId         :" << obj.m_boxId
		<< "\n  BoardId       :" << obj.m_boardId
		<< "\n  SubBoardId    :" << obj.m_subBoardId
		<< "\n  UnitId        :" << obj.m_unitId
		<< "\n  AcceleratorId :" << obj.m_acceleratorId
		<< "\n  FirstPortId   :" << obj.m_firstPortId;
	return os;
}


////////////////////////////////////////////////////////////////////////////
//                        CConfRsrc
////////////////////////////////////////////////////////////////////////////
CConfRsrc::CConfRsrc(DWORD monitorConfId, eSessionType sessionType, eLogicalResourceTypes encoderLogicalType, eConfMediaType confMediaType)
{
	m_monitorConfId    = monitorConfId;
	m_sessionType      = sessionType;
	m_rsrcConfId       = 0; //0 - not allocated.
	m_num_Parties      = 0;
	m_pUdpRsrcDescList = new std::set<CUdpRsrcDesc>;
	m_encoderTypeCOP   = encoderLogicalType;
	m_confMediaState   = eMediaStateEmpty;
	m_confMediaType    = confMediaType;
	m_mrcMcuId         = 1;
}

////////////////////////////////////////////////////////////////////////////
CConfRsrc::CConfRsrc(const CConfRsrc& other) : CPObject(other), m_pUdpRsrcDescList(new std::set<CUdpRsrcDesc>(*(other.m_pUdpRsrcDescList)))
{
	m_monitorConfId  = other.m_monitorConfId;
	m_sessionType    = other.m_sessionType;
	m_bondingPhones  = other.m_bondingPhones;
	m_parties        = other.m_parties;
	m_rsrcConfId     = other.m_rsrcConfId; //0 - not allocated.
	m_num_Parties    = other.m_num_Parties;
	m_encoderTypeCOP = other.m_encoderTypeCOP;
	m_confMediaState = other.m_confMediaState;
	m_confMediaType  = other.m_confMediaType;
	m_mrcMcuId       = other.m_mrcMcuId;
	m_pRsrcDescList  = other.m_pRsrcDescList;
}

////////////////////////////////////////////////////////////////////////////
CConfRsrc::~CConfRsrc()
{
	m_pUdpRsrcDescList->clear();
	PDELETE( m_pUdpRsrcDescList);
	m_pUdpRsrcDescList = 0 ;
}

////////////////////////////////////////////////////////////////////////////
WORD operator==(const CConfRsrc& lhs,const CConfRsrc& rhs)
{
	return (lhs.m_monitorConfId == rhs.m_monitorConfId);
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CConfRsrc& lhs,const CConfRsrc& rhs)
{
	return (lhs.m_monitorConfId < rhs.m_monitorConfId);
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::AddUdpDesc(CUdpRsrcDesc* pUdpRsrcDesc)
{
	PASSERT_AND_RETURN_VALUE(!pUdpRsrcDesc, STATUS_FAIL);

	std::set<CUdpRsrcDesc>::iterator _itr = m_pUdpRsrcDescList->find(*pUdpRsrcDesc);
	PASSERT_AND_RETURN_VALUE(_itr != m_pUdpRsrcDescList->end(), STATUS_FAIL);

	m_pUdpRsrcDescList->insert(*pUdpRsrcDesc);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
// The function only updates existing desc, not adding new one, if not found
STATUS CConfRsrc::UpdateUdpDesc(WORD rsrcpartyId, CUdpRsrcDesc* pUdpRsrcDesc)
{
	PASSERT_AND_RETURN_VALUE(!pUdpRsrcDesc, STATUS_FAIL);

	CUdpRsrcDesc desc(rsrcpartyId);

	std::set<CUdpRsrcDesc>::iterator _itr = m_pUdpRsrcDescList->find(desc);
	PASSERT_AND_RETURN_VALUE(_itr == m_pUdpRsrcDescList->end(), STATUS_FAIL);

	m_pUdpRsrcDescList->erase(_itr);

	m_pUdpRsrcDescList->insert(*pUdpRsrcDesc);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::RemoveUdpDesc(WORD rsrcpartyId)
{
	std::set<CUdpRsrcDesc>::iterator i;
	CUdpRsrcDesc desc(rsrcpartyId);

	i = m_pUdpRsrcDescList->find(desc);

	if (i == m_pUdpRsrcDescList->end())//not exists
	{
		PASSERT(1);
		return STATUS_FAIL;
	}

	m_pUdpRsrcDescList->erase(i);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
const CUdpRsrcDesc* CConfRsrc::GetUdpDesc(WORD rsrcpartyId)const
{
	std::set<CUdpRsrcDesc>::iterator i;
	CUdpRsrcDesc desc(rsrcpartyId);

	i = m_pUdpRsrcDescList->find(desc);

	if (i == m_pUdpRsrcDescList->end())//not exists
	{
		PASSERT(1);
		return NULL;
	}

	return (&(*i));
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::AddDesc(CRsrcDesc* pRsrcDesc)
{
	if (!pRsrcDesc || pRsrcDesc->GetRsrcConfId() != m_rsrcConfId)
	{
		DBGPASSERT(1);
		if (pRsrcDesc)
			TRACEINTOLVLERR << "ERROR: m_rsrcConfId = " << m_rsrcConfId << ", GetRsrcConfId()=" << pRsrcDesc->GetRsrcConfId();
		return STATUS_FAIL;
	}

	if (m_pRsrcDescList.find(*pRsrcDesc) != m_pRsrcDescList.end())
	{//desc already exists

		TRACEINTOLVLERR << "Failed, descriptor already exists" << *pRsrcDesc;
		PASSERT_AND_RETURN_VALUE(1, STATUS_FAIL);
	}

	m_pRsrcDescList.insert(*pRsrcDesc);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::RemoveDesc(WORD rsrcpartyId, eLogicalResourceTypes type, ECntrlType cntrl, WORD portId, WORD unitId, WORD acceleratorId, WORD boardId)
{
	CRsrcDesc desc(0, type, 0, rsrcpartyId, 0, boardId, 0, unitId, acceleratorId, portId, cntrl);

	RSRC_DESC_MULTISET_ITR itr = m_pRsrcDescList.find(desc);

	if (itr == m_pRsrcDescList.end()) // not exists
	{
		TRACEINTOLVLERR << "Failed, descriptor not exist" << desc;
		PASSERT_AND_RETURN_VALUE(1, STATUS_FAIL);
	}

	m_pRsrcDescList.erase(itr);

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
CRsrcDesc* CConfRsrc::GetDescFromConnectionId(DWORD connectionId)
{
	RSRC_DESC_MULTISET_ITR itr;

	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		if ((itr->GetConnId() == connectionId))
		{
			return ((CRsrcDesc*)(&(*itr)));
		}
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////
const CRsrcDesc* CConfRsrc::GetDesc(WORD rsrcpartyId, eLogicalResourceTypes type, ECntrlType cntrl, WORD portId, WORD unitId, WORD acceleratorId, WORD boardId)
{
	CRsrcDesc desc(0, type, 0, rsrcpartyId, 0, boardId, 0, unitId, acceleratorId, portId, cntrl);

	RSRC_DESC_MULTISET_ITR i = m_pRsrcDescList.find(desc);

	if (i == m_pRsrcDescList.end()) // not exists
	{
		// PASSERT(1);
		return NULL;
	}

	return (&(*i));
}

////////////////////////////////////////////////////////////////////////////
const CRsrcDesc* CConfRsrc::GetDescByType(eLogicalResourceTypes type, ECntrlType cntrl)
{
	RSRC_DESC_MULTISET_ITR itr;

	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		if (itr->GetType() == type && itr->GetCntrlType() == cntrl)
		{
			return ((CRsrcDesc*)(&(*itr)));
		}
	}

	return NULL;
}

////////////////////////////////////////////////////////////////////////////
int CConfRsrc::GetDescArray( CRsrcDesc**& pRsrcDescArray)
{
	int arrSize = m_pRsrcDescList.size();
	if( 0 == arrSize )
		return 0;

	pRsrcDescArray = new CRsrcDesc*[arrSize];
	RSRC_DESC_MULTISET_ITR itr;
	int i = 0;

	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		pRsrcDescArray[i] = (CRsrcDesc*)&(*itr);
		i++;
	}
	return i;
}

////////////////////////////////////////////////////////////////////////////
// Need to receive in pRsrcDescList memory of MAX_NUM_ALLOCATED_RSRCS_NET pointers (CRsrcDesc*) allocated for the output (and initialized to all NULLs)
// Returns number of RsrcDescs found.
WORD CConfRsrc::GetDescArrayPerResourceTypeByRsrcId(WORD rsrcpartyId,eLogicalResourceTypes type, CRsrcDesc** pRsrcDescArray, BYTE arrSize) const
{
	RSRC_DESC_MULTISET_ITR itr;
	int i = 0;

	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		if ((itr->GetRsrcPartyId() == rsrcpartyId ) && ( (itr->GetType() == type) || (eLogical_res_none == type)))
		{
			if (i < arrSize) // For protection
			{
				pRsrcDescArray[i] = (CRsrcDesc*)&(*itr);
				i++;
			}
			else
				PASSERTMSG(1, "CConfRsrc::GetDescArrayPerResourceTypeByRsrcId - error in call to function");
		}
	}

	return i;
}

////////////////////////////////////////////////////////////////////////////
const CPartyRsrc* CConfRsrc::GetPartyRsrcByRsrcPartyId(PartyRsrcID partyId) const
{
	PARTIES::iterator _iiEnd = m_parties.end();
	for (PARTIES::iterator _ii = m_parties.begin(); _ii != _iiEnd; ++_ii)
	{
		if (_ii->GetRsrcPartyId() == partyId)
			return &(*_ii);
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
WORD CConfRsrc::GetNumVideoPartiesPerBoard(WORD boardId)
{
	WORD numVideoParties = 0;

	PARTIES::iterator _iiEnd = m_parties.end();
	for (PARTIES::iterator _ii = m_parties.begin(); _ii != _iiEnd; ++_ii)
	{
		PartyRsrcID partyId = _ii->GetRsrcPartyId();
		CPartyRsrc* pParty = const_cast<CPartyRsrc*>(GetPartyRsrcByRsrcPartyId(partyId));
		if (pParty)
		{
			eVideoPartyType videoPartyType = pParty->GetVideoPartyType();
			if (eVideo_party_type_none != videoPartyType)
			{
				CRsrcDesc* pDecDesc = const_cast<CRsrcDesc*>(GetDesc(partyId, eLogical_video_decoder));
				if (pDecDesc)
				{
					if (boardId == pDecDesc->GetBoardId())
						numVideoParties++;
				}
			}
		}
	}

	return numVideoParties;
}


////////////////////////////////////////////////////////////////////////////
//                        CPartyRsrc
////////////////////////////////////////////////////////////////////////////
CPartyRsrc::CPartyRsrc(PartyMonitorID monitorPartyId, eNetworkPartyType networkPartyType, eVideoPartyType videoPartyType, WORD ARTChannels, ePartyRole partyRole,eHdVswTypesInMixAvcSvc hdVswTypeInMixAvcSvcMode)
{
	m_monitorPartyId           = monitorPartyId;
	m_networkPartyType         = networkPartyType;
	m_videoPartyType           = videoPartyType;
	m_resourcePartyType        = e_Audio;
	m_partyRole                = partyRole;
	m_rsrcPartyId              = 0; //0 - not allocated.
	m_CSconnId                 = 0;
#ifdef MS_LYNC_AVMCU_LINK
	// MS Lync
	m_partySigOrganizerConnId  = 0;
	m_partySigFocusConnId      = 0;
	m_partySigEventPackConnId  = 0;
#endif

	m_DialInReservedPorts      = 0;
	m_ARTChannels              = ARTChannels;
	m_roomPartyId              = 0; //TIP Cisco
	m_tipPartyType             = eTipNone;
	m_countPartyAsICEinMFW     = FALSE;
	m_ssrcAudio                = 0;
	m_tipNumOfScreens          = 0;
	m_HdVswTypeInMixAvcSvcMode = hdVswTypeInMixAvcSvcMode;

	memset(m_ssrcContent, 0, sizeof(m_ssrcContent));
	memset(m_ssrcVideo, 0, sizeof(m_ssrcVideo));
}

////////////////////////////////////////////////////////////////////////////
CPartyRsrc::CPartyRsrc(const CPartyRsrc& other) : CPObject(other),
	m_monitorPartyId(other.m_monitorPartyId),
	m_rsrcPartyId(other.m_rsrcPartyId),
	m_roomPartyId(other.m_roomPartyId),
	m_tipPartyType(other.m_tipPartyType),
	m_tipNumOfScreens(other.m_tipNumOfScreens),
	m_networkPartyType(other.m_networkPartyType),
	m_videoPartyType(other.m_videoPartyType),
	m_resourcePartyType(other.m_resourcePartyType),
	m_partyRole(other.m_partyRole),
	m_ARTChannels(other.m_ARTChannels),
	m_CSconnId(other.m_CSconnId),
#ifdef MS_LYNC_AVMCU_LINK
	// MS Lync
	m_partySigOrganizerConnId(other.m_partySigOrganizerConnId),
	m_partySigFocusConnId(other.m_partySigFocusConnId),
	m_partySigEventPackConnId(other.m_partySigEventPackConnId),
#endif
	m_DialInReservedPorts(other.m_DialInReservedPorts),
	m_countPartyAsICEinMFW(other.m_countPartyAsICEinMFW),
	m_HdVswTypeInMixAvcSvcMode(other.m_HdVswTypeInMixAvcSvcMode)
{

	m_ssrcAudio = other.m_ssrcAudio;
	for(WORD i=0;i<MAX_NUM_RECV_STREAMS_FOR_CONTENT;i++)
		m_ssrcContent[i] = other.m_ssrcContent[i];

	for(WORD i=0;i<MAX_NUM_RECV_STREAMS_FOR_VIDEO;i++)
		m_ssrcVideo[i] = other.m_ssrcVideo[i];

}

////////////////////////////////////////////////////////////////////////////
CPartyRsrc::~CPartyRsrc()
{
}

////////////////////////////////////////////////////////////////////////////
void CPartyRsrc::SetARTChannels(WORD ARTChannels)
{
	m_ARTChannels = ARTChannels;

	// bridge-809: assert only not in soft MCU - in soft MCU ARTChannels set to 0, not to limit soft ART capacity
	CSystemResources* pSyst = CHelperFuncs::GetSystemResources();
	if (pSyst && !CHelperFuncs::IsSoftMCU(pSyst->GetProductType()))
	{
		DBGPASSERT(0 == ARTChannels);
	}
}

////////////////////////////////////////////////////////////////////////////
WORD operator==(const CPartyRsrc& lhs,const CPartyRsrc& rhs)
{
	return (lhs.m_monitorPartyId == rhs.m_monitorPartyId);
}

////////////////////////////////////////////////////////////////////////////
bool operator<(const CPartyRsrc& lhs,const CPartyRsrc& rhs)
{
	return (lhs.m_monitorPartyId < rhs.m_monitorPartyId);
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::AddParty(CPartyRsrc& party)
{
	PASSERTSTREAM_AND_RETURN_VALUE(m_parties.find(party) != m_parties.end(), "MonitorPartyId:" << party.GetMonitorPartyId() << " - Already exist", STATUS_FAIL);

	TRACEINTO << "MonitorPartyId:" << party.GetMonitorPartyId() << ", PartyLogicalType:" << party.GetPartyResourceType();
	m_parties.insert(party);
	m_num_Parties++;

	UpdateConfMediaStateByPartiesList();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
BOOL CConfRsrc::CheckIfOneMorePartyCanBeAddedToConf2C(eVideoPartyType videoPartyType)
{
	CSystemResources *pSystemResources = CHelperFuncs::GetSystemResources();
	eSystemCardsMode systemCardsMode = pSystemResources ? pSystemResources->GetSystemCardsMode() : eSystemCardsMode_illegal;

	if (eVSW_video_party_type == videoPartyType)
	{
		if((eVSW_28_session == m_sessionType && m_num_Parties > MAX_PARTIES_IN_CONF_2C_VSW28) ||
		   (eVSW_56_session == m_sessionType && m_num_Parties > MAX_PARTIES_IN_CONF_2C_VSW56))
		{
			return FALSE;
		}
		else if (eVSW_Auto_session  == m_sessionType)
		{
			if ((eSystemCardsMode_breeze == systemCardsMode && m_num_Parties > 180))
				return FALSE;
		}
	}
	else if (eCOP_party_type == videoPartyType)
	{
		DWORD max_cop_parties = 160; //on breeze
		CReservator* pReservator = CHelperFuncs::GetReservator();
		if (pReservator)
			max_cop_parties = pReservator->GetDongleRestriction();
		TRACEINTO << " CConfRsrc::CheckIfOneMorePartyCanBeAddedToConf2C : max_cop_parties = " << max_cop_parties;

		if ((eCOP_HD1080_session == m_sessionType || eCOP_HD720_50_session  == m_sessionType) && m_num_Parties >= max_cop_parties)//192
			return FALSE;
	}
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////
WORD CConfRsrc::RemoveAllParties(BYTE force_kill_all_ports)
{
	CResourceManager* pRsrcMngr = (CResourceManager*)(CProcessBase::GetProcess()->GetCurrentTask());
	if (!pRsrcMngr)
	{
		PASSERT(1); return 0;
	}

	DEALLOC_PARTY_REQ_PARAMS_S* pdeallocateParams = new DEALLOC_PARTY_REQ_PARAMS_S;
	memset(pdeallocateParams, 0, sizeof(DEALLOC_PARTY_REQ_PARAMS_S));

	DEALLOC_PARTY_IND_PARAMS_S* pResult = new DEALLOC_PARTY_IND_PARAMS_S;

	pdeallocateParams->numOfRsrcsWithProblems = 0;
	pdeallocateParams->monitor_conf_id = m_monitorConfId;
	pdeallocateParams->force_kill_all_ports = force_kill_all_ports;

	// make a copy of parties container
	PARTIES PartyRsrcList = m_parties;
	PARTIES::iterator itr = PartyRsrcList.begin();
	DWORD monitorPartyId = 0;
	WORD  numOfRemovedParties = 0;

	for (itr = PartyRsrcList.begin(); itr != PartyRsrcList.end(); itr++)
	{
		monitorPartyId = ((CPartyRsrc*)(&(*itr)))->GetMonitorPartyId();

		TRACEINTO << "monitorPartyId=" << monitorPartyId;

		memset(pResult, 0, sizeof(DEALLOC_PARTY_IND_PARAMS_S));

		pdeallocateParams->monitor_party_id = monitorPartyId;

		pRsrcMngr->DeAlloc(pdeallocateParams, pResult);

		if (pResult->status == STATUS_OK)
			++numOfRemovedParties;
	}

	delete pdeallocateParams;
	delete pResult;

	return numOfRemovedParties;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::RemoveParty(PartyMonitorID monitorPartyId)
{
	CPartyRsrc party(monitorPartyId);

	PARTIES::iterator itr = m_parties.find(party);
	PASSERTSTREAM_AND_RETURN_VALUE(itr == m_parties.end(), "MonitorPartyId:" << monitorPartyId << " - Party not exist", STATUS_FAIL);

	m_parties.erase(itr);
	m_num_Parties--;

	UpdateConfMediaStateByPartiesList();

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
const CPartyRsrc* CConfRsrc::GetParty(PartyMonitorID monitorPartyId) const
{
	CPartyRsrc party(monitorPartyId);

	PARTIES::iterator itr = m_parties.find(party);
	TRACECOND_AND_RETURN_VALUE(itr == m_parties.end(), "MonitorPartyId:" << monitorPartyId << " - Party not exist", NULL);

	return &(*itr);
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::AddTempBondingPhoneNumber(PartyRsrcID partyId, Phone* pPhone)
{
	PASSERT_AND_RETURN_VALUE(!pPhone, STATUS_FAIL);

	Phone* phone = new Phone;
	strcpy_safe(phone->phone_number, pPhone->phone_number);

	m_bondingPhones.insert(BONDING_PHONES::value_type(partyId, phone));

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::RemoveTempBondingPhoneNumber(PartyRsrcID partyId)
{
	BONDING_PHONES::iterator itr = m_bondingPhones.find(partyId);
	if (itr == m_bondingPhones.end())
		return STATUS_FAIL;

	delete itr->second;
	m_bondingPhones.erase(itr);
	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
Phone* CConfRsrc::GetTempBondingPhoneNumberByRsrcPartyId(PartyRsrcID partyId) const
{
	BONDING_PHONES::const_iterator itr = m_bondingPhones.find(partyId);
	if (itr == m_bondingPhones.end())
		return NULL;
	return itr->second;
}

////////////////////////////////////////////////////////////////////////////
WORD CConfRsrc::IsBoardIdAllocatedToConf(WORD boardId)
{
	//if board allocated o conf -
	// there is have to be at least on RsrcDesc of this board.

	//WORD bId = 2;

	RSRC_DESC_MULTISET_ITR itr;

	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		if (itr->GetBoardId() == boardId)
			return TRUE;
	}
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////
void CConfRsrc::AddListOfPartiesDescriptorsOnBoard(WORD boardId, WORD subBoardId, std::map<std::string,CONF_PARTY_ID_S*>* listOfConfIdPartyIdPair) const
{
	RSRC_DESC_MULTISET_ITR itr;
	CRsrcDesc rsrcdesc;
	std::string strKey;
	CONF_PARTY_ID_S* pConf_party_id_s = NULL;
	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		rsrcdesc = ((CRsrcDesc)(*itr));
		if (rsrcdesc.GetBoardId() == boardId)
		{
			//if subboard is 1 (MFA), this means that the whole card will be taken out, including the RTM
			//so it doesn't matter on which subboard it is
			if (subBoardId == MFA_SUBBOARD_ID || rsrcdesc.GetSubBoardId() == subBoardId)
			{
				strKey = GetPartyKey(rsrcdesc.GetRsrcConfId() ,rsrcdesc.GetRsrcPartyId());
				if (listOfConfIdPartyIdPair->find(strKey) == listOfConfIdPartyIdPair->end())
				{
					pConf_party_id_s = new CONF_PARTY_ID_S();
					pConf_party_id_s->monitor_conf_id = GetMonitorConfId();
					CPartyRsrc* pParty = (CPartyRsrc*)(GetPartyRsrcByRsrcPartyId(rsrcdesc.GetRsrcPartyId()));
					if (pParty == NULL)
					{
						POBJDELETE(pConf_party_id_s);
						PASSERT(1);
						continue;
					}
					else
					{
						pConf_party_id_s->monitor_party_id = pParty->GetMonitorPartyId();
					}

					(*listOfConfIdPartyIdPair)[strKey] = pConf_party_id_s;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////
std::string CConfRsrc::GetPartyKey(DWORD rsrcConfId, DWORD rsrcPartyId) const
{
	std::ostringstream o;
	o << rsrcConfId << "_" << rsrcPartyId;
	return o.str();
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::GetFreeVideoRsrcVSW(CRsrcDesc*& pEncDesc, CRsrcDesc*& pDecDesc)
{
	WORD rsrcId = 0, enc_found = 0, dec_found = 0;
	RSRC_DESC_MULTISET_ITR itr;
	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		CRsrcDesc* pRsrcDesc = (CRsrcDesc*)&(*itr);

		BOOL isDecType = CHelperFuncs::IsLogicalVideoDecoderType( pRsrcDesc->GetType());
		BOOL isEncType = CHelperFuncs::IsLogicalVideoEncoderType( pRsrcDesc->GetType());

		if (pRsrcDesc->GetIsUpdated() || (!isDecType && !isEncType) ||
			(isDecType && dec_found) || (isEncType && enc_found))
			continue;

		rsrcId = pRsrcDesc->GetRsrcPartyId();
		if (isDecType)
		{
			dec_found = rsrcId;
			pDecDesc = pRsrcDesc;
		}
		else
		{
			enc_found = rsrcId;
			pEncDesc = pRsrcDesc;
		}

		if (dec_found && enc_found && dec_found == enc_found)
			break;
	}

	if (!pEncDesc || !pDecDesc)
		return STATUS_FAIL;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::GetBoardUnitIdByRoomIdTIP(DWORD room_id, WORD& boardIdTIP, WORD& unitIdTIP) //TIP Cisco
{
	PARTIES::iterator _iiEnd = m_parties.end();
	for (PARTIES::iterator _ii = m_parties.begin(); _ii != _iiEnd; ++_ii)
	{
		if (_ii->GetRoomPartyId() == room_id)
		{
			const CRsrcDesc* pAudEncDesc = GetDesc(_ii->GetRsrcPartyId(), eLogical_audio_encoder);
			if (pAudEncDesc)
			{
				boardIdTIP = pAudEncDesc->GetBoardId();
				unitIdTIP = pAudEncDesc->GetUnitId();
				return STATUS_OK;
			}
		}
	}

	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
WORD CConfRsrc::GetBoardIdForRelayParty() //olga
{
	PARTIES::iterator _it = std::find_if(m_parties.begin(), m_parties.end(), compareVideoPartyFunc());
	if (_it != m_parties.end())
	{
		const CRsrcDesc* pRtpDesc = GetDesc(_it->GetRsrcPartyId(), eLogical_rtp);
		if (pRtpDesc)
			return pRtpDesc->GetBoardId();
	}
	TRACEINTO << "Relay party is not allocated on any board";
	return 0;
}

////////////////////////////////////////////////////////////////////////////
bool CConfRsrc::CheckIfThereAreRelayParty() const
{
	PARTIES::iterator _it = std::find_if(m_parties.begin(), m_parties.end(), compareVideoPartyFunc());
	return (_it != m_parties.end());
}

////////////////////////////////////////////////////////////////////////////
bool CConfRsrc::CheckIfThereAreNotRelayParty() const
{
	PARTIES::iterator _it = std::find_if(m_parties.begin(), m_parties.end(), not1(compareVideoPartyFunc()));
	return (_it != m_parties.end());
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrc::GetBoardUnitIdByAvMcuLinkMain(PartyRsrcID mainPartyRsrcID, WORD& boardId, WORD& unitId)
{
	const CRsrcDesc* pRtpDesc = GetDesc(mainPartyRsrcID, eLogical_rtp);
	if (pRtpDesc)
	{
		boardId = pRtpDesc->GetBoardId();
		unitId  = pRtpDesc->GetUnitId();
		return STATUS_OK;
	}
	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
void CConfRsrc::CountAvcSvcParties(WORD& numAvc, WORD& numSvc) const
{
	numAvc = 0;
	numSvc = 0;

	PARTIES::iterator _iiEnd = m_parties.end();
	for (PARTIES::iterator _ii = m_parties.begin(); _ii != _iiEnd; ++_ii)
	{
		if (CHelperFuncs::IsVideoRelayParty(_ii->GetVideoPartyType()))
			numSvc++;
		else
			numAvc++;
	}
}

////////////////////////////////////////////////////////////////////////////
// Need to receive in pRsrcDescList memory of MAX_NUM_ALLOCATED_RSRCS_NET pointers (CRsrcDesc*) allocated for the output (and initialized to all NULLs)
// Returns number of RsrcDescs found.
WORD CConfRsrc::GetPartyRcrsDescArrayByRsrcId(DWORD rsrcpartyId, CRsrcDesc** pRsrcDescArray, WORD arrSize)
{
	RSRC_DESC_MULTISET_ITR itr;
	int num_party_resources = 0;

	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		if (itr->GetRsrcPartyId() == rsrcpartyId)
		{
			if (num_party_resources < arrSize)    // For protection
			{
				pRsrcDescArray[num_party_resources] = (CRsrcDesc*)&(*itr);
				num_party_resources++;
			}
			else{
				PASSERTMSG(num_party_resources, "CConfRsrc::GetDescArrayPerResourceTypeByRsrcId - error in call to function");
			}
		}
	}

	return num_party_resources;
}

////////////////////////////////////////////////////////////////////////////
WORD CConfRsrc::GetPartyRTPDescArrayByRsrcId(DWORD rsrcpartyId, CRsrcDesc** pRsrcDescArray, WORD arrSize)
{
	RSRC_DESC_MULTISET_ITR itr;
	int num_party_resources = 0;

	for (itr = m_pRsrcDescList.begin(); itr != m_pRsrcDescList.end(); itr++)
	{
		if (itr->GetRsrcPartyId() == rsrcpartyId && CHelperFuncs::IsLogicalRTPtype(itr->GetType()))
		{
			if (num_party_resources < arrSize)    // For protection
			{
				pRsrcDescArray[num_party_resources] = (CRsrcDesc*)&(*itr);
				num_party_resources++;
			}
			else{
				PASSERTMSG(num_party_resources, "CConfRsrc::GetDescArrayPerResourceTypeByRsrcId - error in call to function");
			}
		}
	}
	return num_party_resources;
}

////////////////////////////////////////////////////////////////////////////
void CConfRsrc::UpdateConfMediaStateByPartiesList()
{
	bool hasRelayParties = CheckIfThereAreRelayParty();
	bool hasNotRelayParties = CheckIfThereAreNotRelayParty();

	if (eMediaStateMixAvcSvc == m_confMediaState)
	{
		// since we don't support downgrade - we don't change ConfMediaState after it already in mixed
		return;
	}

	if (!hasRelayParties && !hasNotRelayParties)
	{
		SetConfMediaState(eMediaStateEmpty);
	}
	else if (hasRelayParties && !hasNotRelayParties)
	{
		SetConfMediaState(eMediaStateSvcOnly);
	}
	else if (!hasRelayParties && hasNotRelayParties)
	{
		SetConfMediaState(eMediaStateAvcOnly);
	}
}


////////////////////////////////////////////////////////////////////////////
//                        CConfRsrcDB
////////////////////////////////////////////////////////////////////////////
CConfRsrcDB::CConfRsrcDB()
{
	m_numConfRsrcs = 0;
}

////////////////////////////////////////////////////////////////////////////
CConfRsrcDB::~CConfRsrcDB()
{
	m_numConfRsrcs = 0;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrcDB::AddConfRsrc(CConfRsrc* pConfRsrc)
{
	PASSERT_AND_RETURN_VALUE(!pConfRsrc, STATUS_FAIL);

	PASSERT_AND_RETURN_VALUE(m_confList.find(*pConfRsrc) != m_confList.end(), STATUS_FAIL);

	m_confList.insert(*pConfRsrc);

	ConfRsrcID id = pConfRsrc->GetRsrcConfId();
	if (STANDALONE_CONF_ID != id)
		m_numConfRsrcs++;

	TRACEINTO << "ConfId:" << id << ", NumOngoingConf:" << m_numConfRsrcs;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrcDB::RemoveConfRsrc(ConfMonitorID confId)
{
	CConfRsrc conf(confId);

	RSRC_CONF_LIST::iterator _itr = m_confList.find(conf);
	if (_itr == m_confList.end())
		return STATUS_FAIL;

	ConfRsrcID id = _itr->GetRsrcConfId();
	if (STANDALONE_CONF_ID != id)
	{
		if (m_numConfRsrcs > 0)
			m_numConfRsrcs--;
		else
			PASSERT(1);
	}

	m_confList.erase(_itr);

	TRACEINTO << "ConfId:" << id << ", NumOngoingConf:" << m_numConfRsrcs;

	return STATUS_OK;
}

////////////////////////////////////////////////////////////////////////////
CConfRsrc* CConfRsrcDB::GetConfRsrc(ConfMonitorID confId)
{
	CConfRsrc conf(confId);

	RSRC_CONF_LIST::iterator _itr = m_confList.find(conf);
	if (_itr != m_confList.end())
		return (CConfRsrc*)(&(*_itr));
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
CConfRsrc* CConfRsrcDB::GetConfRsrcByRsrcConfId(ConfRsrcID confId)
{
	RSRC_CONF_LIST::iterator _end = m_confList.end();
	for (RSRC_CONF_LIST::iterator _itr = m_confList.begin(); _itr != _end; ++_itr)
	{
		if (_itr->GetRsrcConfId() == confId)
			return (CConfRsrc*)(&(*_itr));
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////
STATUS CConfRsrcDB::GetMonitorIdsRsrcIds(ConfRsrcID confId, PartyRsrcID partyId, ConfMonitorID& monitorConfId, PartyMonitorID& monitorPartyId)
{
	monitorConfId = 0xFFFFFFFF;
	monitorPartyId = 0xFFFFFFFF;

	RSRC_CONF_LIST::iterator _end = m_confList.end();
	for (RSRC_CONF_LIST::iterator _itr = m_confList.begin(); _itr != _end; ++_itr)
	{
		if (_itr->GetRsrcConfId() == confId)
		{
			monitorConfId = _itr->GetMonitorConfId();
			const CPartyRsrc* pParty = _itr->GetPartyRsrcByRsrcPartyId(partyId);
			if (pParty)
			{
				monitorPartyId = pParty->GetMonitorPartyId();
				return STATUS_OK;
			}
		}
	}
	return STATUS_FAIL;
}

////////////////////////////////////////////////////////////////////////////
bool CConfRsrcDB::IsExitingConf(ConfMonitorID confId)
{
	const CConfRsrc* pConfRsrc = GetConfRsrc(confId);
	return (pConfRsrc == NULL)? false : true;
}

////////////////////////////////////////////////////////////////////////////
bool CConfRsrcDB::IsEmptyConf(ConfMonitorID confId)
{
	CConfRsrc* pConfRsrc = GetConfRsrc(confId);
	PASSERT_AND_RETURN_VALUE(!pConfRsrc, true);

	return pConfRsrc->GetNumParties() ? false : true;
}

////////////////////////////////////////////////////////////////////////////
ConfRsrcID CConfRsrcDB::MonitorToRsrcConfId(ConfMonitorID confId)
{
	CConfRsrc* pConfRsrc = GetConfRsrc(confId);
	return pConfRsrc ? pConfRsrc->GetRsrcConfId() : 0;
}

////////////////////////////////////////////////////////////////////////////
void CConfRsrcDB::GetAllPartiesOnBoard(WORD boardId, WORD subBoardId, std::map<std::string, CONF_PARTY_ID_S*>* listOfConfIdPartyIdPair)
{
	RSRC_CONF_LIST::iterator _end = m_confList.end();
	for (RSRC_CONF_LIST::iterator _itr = m_confList.begin(); _itr != _end; ++_itr)
		_itr->AddListOfPartiesDescriptorsOnBoard(boardId, subBoardId, listOfConfIdPartyIdPair);
}

////////////////////////////////////////////////////////////////////////////
void CConfRsrcDB::FillISDNServiceName(ConfMonitorID confId, PartyMonitorID partyId, char serviceName[GENERAL_SERVICE_PROVIDER_NAME_LEN])
{
	CSystemResources* pSystemResources = CHelperFuncs::GetSystemResources();
	PASSERT_AND_RETURN(!pSystemResources);

	const CConfRsrc* pConfRsrc = GetConfRsrc(confId);
	TRACECOND_AND_RETURN(!pConfRsrc, "MonitorConfId:" << confId << " - Failed, conference not found");

	const CPartyRsrc* pPartyRsrc = pConfRsrc->GetParty(partyId);
	TRACECOND_AND_RETURN(!pPartyRsrc, "MonitorPartyId:" << partyId << " - Failed, participant not found");

	CRsrcDesc** pRsrcDescArray = new CRsrcDesc*[MAX_NUM_ALLOCATED_RSRCS_NET];
	for (int i = 0 ; i < MAX_NUM_ALLOCATED_RSRCS_NET ; i++)
		pRsrcDescArray[i] = NULL;

	// Get all resource descriptors of RTM per a given party
	pConfRsrc->GetDescArrayPerResourceTypeByRsrcId(pPartyRsrc->GetRsrcPartyId(), eLogical_net, pRsrcDescArray);
	if (pRsrcDescArray[0] == NULL)
	{
		TRACEINTO << "MonitorPartyId:" << partyId << " - Failed, RTMs not found for party";
		delete []pRsrcDescArray;
		return;
	}

	WORD boardId = pRsrcDescArray[0]->GetBoardId();
	WORD unitId  = pRsrcDescArray[0]->GetUnitId();

	delete []pRsrcDescArray;

	CBoard* pBoard = pSystemResources->GetBoard(boardId);
	PASSERT_AND_RETURN(!pBoard);

	const CSpanRTM* pSpan = pBoard->GetRTM(unitId);
	PASSERT_AND_RETURN(!pSpan);

	const char* serviceNameFromSpan = pSpan->GetSpanServiceName();
	if (serviceNameFromSpan)
		strcpy_safe(serviceName, sizeof(serviceName), serviceNameFromSpan);
}

////////////////////////////////////////////////////////////////////////////
PartyRsrcID CConfRsrcDB::MonitorToRsrcPartyId(ConfMonitorID confId, PartyMonitorID partyId)
{
	const CConfRsrc* pConfRsrc = GetConfRsrc(confId);
	if (pConfRsrc)
	{
		const CPartyRsrc* pPartyRsrc = pConfRsrc->GetParty(partyId);
		if (pPartyRsrc)
			return pPartyRsrc->GetRsrcPartyId();
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
bool CConfRsrcDB::GetPartyType(ConfMonitorID confId, PartyMonitorID partyId,
								eNetworkPartyType& networkPartyType,
								eVideoPartyType& videoPartyType,
								ePartyRole& partyRole,
								WORD& artChannels,
								eSessionType& sessionType)
{
	const CConfRsrc* pConfRsrc = GetConfRsrc(confId);
	TRACECOND_AND_RETURN_VALUE(!pConfRsrc, "MonitorConfId:" << confId << " - Failed, conference not found", false);

	const CPartyRsrc* pPartyRsrc = pConfRsrc->GetParty(partyId);
	TRACECOND_AND_RETURN_VALUE(!pPartyRsrc, "MonitorPartyId:" << partyId << " - Failed, participant not found", false);

	networkPartyType = pPartyRsrc->GetNetworkPartyType();
	videoPartyType   = pPartyRsrc->GetVideoPartyType();
	partyRole        = pPartyRsrc->GetPartyRole();
	artChannels      = pPartyRsrc->GetARTChannels();
	sessionType      = pConfRsrc->GetSessionType();
	return true;
}

