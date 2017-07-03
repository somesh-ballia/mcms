#include <ostream>
#include <iostream>
#include "ResourceManagerHelper.h"
#include "ProcessBase.h"
#include "SystemFunctions.h"
#include "PrettyTable.h"
#include "WrappersCommon.h"
#include "HelperFuncs.h"

extern char* CardTypeToString(APIU32 cardType);
extern char* CardUnitPhysicalTypeToString(APIU32 unitPhysicalType);
extern char* CardUnitConfiguredTypeToString(APIU32 unitConfigType);
extern char* CardUnitLoadedStatusToString(APIU32 unitLoadedStatus);

extern const char* SpanTypeToString(eSpanType theType);
extern const char* ServiceTypeToString(eServiceType theType);
extern const char* DfltNumToString(eDfltNumType theType);
extern const char* NumPlanToString(eNumPlanType theType);
extern const char* VoiceTypeToString(eVoiceType theType);
extern const char* FramingTypeToString(eFramingType theType);
extern const char* SideTypeToString(eSideType theType);
extern const char* LineCodingTypeToString(eLineCodingType theType);
extern const char* SwitchTypeToString(eSwitchType theType);
extern const char* UnitReconfigStatusToString(APIU32 unitReconfigStatus);


////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, ALLOC_PARTY_IND_PARAMS_S& params)
{
	os
		<< "\nALLOC_PARTY_IND_PARAMS_S:"
		<< "\n  status                         :" << CProcessBase::GetProcess()->GetStatusAsString(params.allocIndBase.status).c_str()
		<< "\n  rsrc_conf_id                   :" << params.allocIndBase.rsrc_conf_id
		<< "\n  rsrc_party_id                  :" << params.allocIndBase.rsrc_party_id
		<< "\n  room_id                        :" << params.allocIndBase.room_id
		<< "\n  numRsrcs                       :" << params.allocIndBase.numRsrcs
		<< "\n  videoPartyType                 :" << eVideoPartyTypeNames[params.allocIndBase.videoPartyType] << " (" << params.allocIndBase.videoPartyType << ")"
		<< "\n  networkPartyType               :" << eNetworkPartyTypeNames[params.allocIndBase.networkPartyType] << " (" << params.allocIndBase.networkPartyType << ")"
		<< "\n  isIceParty                     :" << (WORD)params.allocIndBase.isIceParty
		<< "\n  confMediaType                  :" << ConfMediaTypeToString(params.allocIndBase.confMediaType) << " (" << params.allocIndBase.confMediaType << ")";

	CPrettyTable<WORD, DWORD, const char*> tbl("Idx", "ConnId", "ResourceType");
	for (WORD i = 0; i < params.allocIndBase.numRsrcs; i++)
		tbl.Add(i+1, (params.allocIndBase.allocatedRrcs[i]).connectionId, to_string((params.allocIndBase.allocatedRrcs[i]).logicalRsrcType));

	os << tbl.Get();

	os
		<< "\nUDP_ADDRESSES:"
		<< "\n  AudioChannelPort               :" << params.udpAdresses.AudioChannelPort
		<< "\n  AudioChannelAdditionalPorts    :" << params.udpAdresses.AudioChannelAdditionalPorts
		<< "\n  VideoChannelPort               :" << params.udpAdresses.VideoChannelPort
		<< "\n  VideoChannelAdditionalPorts    :" << params.udpAdresses.VideoChannelAdditionalPorts
		<< "\n  ContentChannelPort             :" << params.udpAdresses.ContentChannelPort
		<< "\n  ContentChannelAdditionalPorts  :" << params.udpAdresses.ContentChannelAdditionalPorts
		<< "\n  FeccChannelPort                :" << params.udpAdresses.FeccChannelPort
		<< "\n  FeccChannelAdditionalPorts     :" << params.udpAdresses.FeccChannelAdditionalPorts
		<< "\n  BfcpChannelPort                :" << params.udpAdresses.BfcpChannelPort
		<< "\n  BfcpChannelAdditionalPorts     :" << params.udpAdresses.BfcpChannelAdditionalPorts
		<< "\n  UdpAddressType                 :" << params.udpAdresses.IpType
		<< "\n  IP v4                          :" << CIPV4Wrapper(params.udpAdresses.IpV4Addr)
		<< "\n  IP v6                          :" << CIPV6AraryWrapper(params.udpAdresses.IpV6AddrArray);

	os
		<< "\nSVC_PARTY_IND_PARAMS_S:"
		<< "\n  ssrcAudio                      :" << params.svcParams.m_ssrcAudio;

	for (int i = 0; i < MAX_NUM_RECV_STREAMS_FOR_VIDEO; i++)
		os << "\n  ssrcVideo[" << i << "]                   :" << params.svcParams.m_ssrcVideo[i];

	for (int i = 0; i < MAX_NUM_RECV_STREAMS_FOR_CONTENT; i++)
		os << "\n  ssrcContent[" << i << "]                 :" << params.svcParams.m_ssrcContent[i];

	if (CHelperFuncs::IsISDNParty(params.allocIndBase.networkPartyType))
	{
		os << "\nISDN_PARTY_IND_PARAMS_S:"
		    << "\n  BondingTemporaryPhoneNumber    :" << params.isdnParams.BondingTemporaryPhoneNumber;

		for (int i = 0; i < NUM_E1_PORTS; i++)
		{
			if (params.isdnParams.spans_order.port_spans_list[i].conn_id != 0)
			{
				os << "\n  port_spans_list:" << i << ", board_id:" << params.isdnParams.spans_order.port_spans_list[i].board_id << ", conn_id:" << params.isdnParams.spans_order.port_spans_list[i].conn_id;
				for (int k = 0; k < MAX_NUM_SPANS_ORDER; k++)
				{
					if (params.isdnParams.spans_order.port_spans_list[i].spans_list[k] != 0)
						os << "\n    spans_list[" << k << "] :" << params.isdnParams.spans_order.port_spans_list[i].spans_list[k];
				}
			}
		}
		os << "\n  NumAllocatedChannels           :" << params.isdnParams.num_of_isdn_ports;
	}

	os
		<< "\nMS_SSRC_PARTY_IND_PARAMS_S:"
		<< "\n  msSsrcFirst                    :" << params.msSsrcParams.m_msSsrcFirst
		<< "\n  msSsrcLast                     :" << params.msSsrcParams.m_msSsrcLast;

	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, DEALLOC_PARTY_IND_PARAMS_S& params)
{
	os
		<< "\nDEALLOC_PARTY_IND_PARAMS_S:"
		<< "\n  Status            :" << params.status;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, BOARD_FULL_REQ_PARAMS_S& params)
{
	os << params.allocPartyReqParams;
	for (int i = 0; i < NUM_E1_PORTS; i++)
		os << "\n  ConnectionId[" << i << "]   :" << params.connectionIdList[i];
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, PARTY_MOVE_RSRC_REQ_PARAMS_S& params)
{
	os
		<< "\nPARTY_MOVE_RSRC_REQ_PARAMS_S:"
		<< "\n  SourceMonitorConfId  :" << params.source_monitor_conf_id
		<< "\n  SourceMonitorPartyId :" << params.source_monitor_party_id
		<< "\n  TargetMonitorConfId  :" << params.target_monitor_conf_id
		<< "\n  TargetMonitorPartyId :" << params.target_monitor_party_id;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, PARTY_MOVE_RSRC_IND_PARAMS_S& params)
{
	os
		<< "\nPARTY_MOVE_RSRC_IND_PARAMS_S:"
		<< "\n  TargetMonitorConfId  :" << params.target_monitor_conf_id
		<< "\n  TargetMonitorPartyId :" << params.monitor_party_id
		<< "\n  TargetConfId         :" << params.target_rsrc_conf_id
		<< "\n  TargetPartyId        :" << params.rsrc_party_id
		<< "\n  Status               :" << params.status;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, IP_SERVICE_UDP_RESOURCES_S& params)
{
	os
		<< "\nIP_SERVICE_UDP_RESOURCES_S:"
		<< "\n  ServiceId          :" << params.ServId
		<< "\n  ServiceName        :" << params.ServName
		<< "\n  ServiceDefaultType :" << (int)params.service_default_type
		<< "\n  IceEnvironment     :" << params.iceEnvironment
		<< "\n  NumberOfPQ         :" << params.numPQSactual;

	if (params.numPQSactual)
	{
		CPrettyTable<eUdpAllocMethod, WORD, WORD, WORD, eIpType, const char*, const char*, WORD, WORD, WORD> tbl("AllocMethod", "BoxId", "BoardId", "PqId", "IpType", "IpV4Addr", "IpV6Addr", "UdpFirstPort", "UdpLastPort", "SubServiceId");
		for (int i = 0 ; i < params.numPQSactual; ++i)
		{
			char ipV4Addr[16];
			SystemDWORDToIpString(params.IPServUDPperPQList[i].IpV4Addr.ip, ipV4Addr);

			char ipV6Addr[64];
			mcTransportAddress transportAddress;
			memset(&transportAddress, 0, sizeof(transportAddress));
			transportAddress.ipVersion = (DWORD)eIpVersion6;
			transportAddress.addr.v6.scopeId = params.IPServUDPperPQList[i].IpV6Addr[0].scopeId;
			memcpy(transportAddress.addr.v6.ip, params.IPServUDPperPQList[i].IpV6Addr[0].ip, sizeof(params.IPServUDPperPQList[i].IpV6Addr[0].ip));
			::ipToString(transportAddress, ipV6Addr, 0);

			tbl.Add(
				params.IPServUDPperPQList[i].portsAlloctype,
				params.IPServUDPperPQList[i].boxId,
				params.IPServUDPperPQList[i].boardId,
				params.IPServUDPperPQList[i].PQid,
				params.IPServUDPperPQList[i].IpType,
				ipV4Addr,
				ipV6Addr,
				params.IPServUDPperPQList[i].UdpFirstPort,
				params.IPServUDPperPQList[i].UdpLastPort,
				params.IPServUDPperPQList[i].subServiceId);
		}
		os << tbl.Get();
	}
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, RSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S& params)
{
	CPrettyTable<int, int, const char*, APIU32, const char*> tbl("Unit", "BoardId", "Type", "PqId", "Status");

	for (int i = 0; i < MAX_NUM_OF_UNITS; ++i)
	{
		char* cardUnitPhysicalTypeStr = ::CardUnitPhysicalTypeToString(params.unitsConfigParamsList[i].unitType);
		char* cardUnitLoadedStatusStr = ::CardUnitLoadedStatusToString(params.unitsConfigParamsList[i].status);

		char cardUnitPhysicalType[128];
		snprintf(cardUnitPhysicalType, ARRAYSIZE(cardUnitPhysicalType), "%s (%d)", DUMPSTR(cardUnitPhysicalTypeStr), params.unitsConfigParamsList[i].unitType);

		char cardUnitLoadedStatus[128];
		snprintf(cardUnitLoadedStatus, ARRAYSIZE(cardUnitLoadedStatus), "%s (%d)", DUMPSTR(cardUnitLoadedStatusStr), params.unitsConfigParamsList[i].status);

		tbl.Add(
			params.unitsConfigParamsList[i].physicalHeader.unit_id,
			params.unitsConfigParamsList[i].physicalHeader.board_id,
			cardUnitPhysicalType,
			params.unitsConfigParamsList[i].pqNumber,
			cardUnitLoadedStatus);
	}
	os << "\nRSRCALLOC_UNITS_LIST_CONFIG_PARAMS_S:" << "\nCardType:" << DUMPSTR(::CardTypeToString(params.cardType)) << tbl.Get();
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CM_UNITS_CONFIG_S& params)
{
	CPrettyTable<int, const char*, APIU32> tbl("Unit", "Type", "PqId");

	for (int i = 0; i < MAX_NUM_OF_UNITS; i++)
	{
		char* cardType = ::CardUnitConfiguredTypeToString(params.unitsParamsList[i].type);

		tbl.Add(
			i,
			cardType ? cardType : "Invalid",
			params.unitsParamsList[i].pqNumber);
	}
	os << "\nCM_UNITS_CONFIG_S:" << tbl.Get();
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, ACK_IND_S& params)
{
	os
		<< "\nACK_IND_S:"
		<< "\n  ack_opcode        :" << params.ack_base.ack_opcode
		<< "\n  ack_seq_num       :" << params.ack_base.ack_seq_num
		<< "\n  status            :" << params.ack_base.status
		<< "\n  reason            :" << params.ack_base.reason
		<< "\n  media_type        :" << params.media_type
		<< "\n  media_direction   :" << params.media_direction
		<< "\n  channel_handle    :" << params.channelHandle;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, SPAN_ENABLED_S& params)
{
	os
		<< "\nSPAN_ENABLED_S:"
		<< "\n  BoardId           :" << params.boardId
		<< "\n  ServiceName       :" << (char*)(params.serviceName)
		<< "\n  SpanEnabledStatus :" << params.spanEnabledStatus
		<< "\n  SpanId            :" << params.spanId;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, SPAN_DISABLE_S& params)
{
	os
		<< "\nSPAN_DISABLE_S:"
		<< "\n  BoardId           :" << params.boardId
		<< "\n  SpanId            :" << params.spanId
		<< "\n  Status            :" << params.status;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, MR_MONITOR_NUMERIC_ID_LIST_S& params)
{
	os.precision(0);
	os
		<< "\nMEETING ROOMS LIST:"
		<< "\n ----+---------+------------+------+--------------------------+--------------------------+--------------------------+"
		<< "\n MR  | monitor | numeric    | conf | service name             | first phone number       | second phone number      |"
		<< "\n num | id      | id         | type |                          |                          |                          |"
		<< "\n ----+---------+------------+------+--------------------------+--------------------------+--------------------------+";

	for (DWORD i = 0; i < params.list_size; ++i)
	{
		os
			<< "\n"
			<< " " << setw( 3) << right << i << " |"
			<< " " << setw( 7) << right << (long)(params.monitor_numeric_list[i]).meeting_room_monitor_Id << " |"
			<< " " << setw(10) << right << (params.monitor_numeric_list[i]).numeric_Id << " |"
			<< " " << setw( 4) << right << (WORD)(params.monitor_numeric_list[i]).conf_type << " |"
			<< " " << setw(24) << left  << (params.monitor_numeric_list[i]).serviceName << " |"
			<< " " << setw(24) << left  << (params.monitor_numeric_list[i]).firstPhoneNumber << " |"
			<< " " << setw(24) << left  << (params.monitor_numeric_list[i]).secondPhoneNumber << " |";
	}
	os << "\n ----+---------+------------+------+--------------------------+--------------------------+--------------------------+";
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, MR_IND_LIST_S& params)
{
	CPrettyTable<DWORD, DWORD> tbl("MR Id", "Status");
	for (DWORD i = 0; i < params.list_size; ++i)
		tbl.Add(params.mr_list[i].meeting_room_monitor_Id, params.mr_list[i].status);
	os << "\nMR_IND_LIST_S:" << tbl.Get();
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, PROFILE_IND_S& params)
{
	os
		<< "\nPROFILE_IND_S:"
		<< "\n  ProfileId         :" << params.profile_Id
		<< "\n  MaxVideoPartyType :" << eVideoPartyTypeNames[params.maxVideoPartyType];
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, PROFILE_IND_LIST_S& params)
{
	CPrettyTable<DWORD, const char*> tbl("ProfileId", "ProfileMaxVideoType");
	for (DWORD i = 0; i < params.list_size; ++i)
	{
		eVideoPartyType maxPartyVideo = params.profile_list[i].maxVideoPartyType;
		if (maxPartyVideo < NUM_OF_VIDEO_PARTY_TYPES)
			tbl.Add(params.profile_list[i].profile_Id, eVideoPartyTypeNames[maxPartyVideo]);
		else
			tbl.Add(params.profile_list[i].profile_Id, "illegal");
	}
	os << "\nPROFILE_IND_LIST_S:" << tbl.Get();
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, RTM_ISDN_PARAMS_MCMS_S& params)
{
	// ===== 1. basic service params
	os
		<< "\nRTM_ISDN_PARAMS_MCMS_S:"
		<< "\n  ServiceName       :" << (char*)(params.serviceName)
		<< "\n  DfltNumType       :" << ::DfltNumToString((eDfltNumType)(params.dfltNumType))
		<< "\n  NumPlanType       :" << ::NumPlanToString((eNumPlanType)(params.numPlan))
		<< "\n  VoiceType         :" << ::VoiceTypeToString((eVoiceType)(params.voice));

	// ===== 2. span definition
	os
		<< "\n  SpanType          :" << ::SpanTypeToString((eSpanType)(params.spanDef.spanType))
		<< "\n  ServiceType       :" << ::ServiceTypeToString((eServiceType)(params.spanDef.serviceType))
		<< "\n  Framing           :" << ::FramingTypeToString((eFramingType)(params.spanDef.framing))
		<< "\n  Side              :" << ::SideTypeToString((eSideType)(params.spanDef.side))
		<< "\n  LineCoding        :" << ::LineCodingTypeToString((eLineCodingType)(params.spanDef.lineCoding))
		<< "\n  SwitchType        :" << ::SwitchTypeToString((eSwitchType)(params.spanDef.switchType));

	// ===== 2. phone ranges
	os
		<< "\n"
		<< "\n  Phone Ranges      :"
		<< "\n  ~~~~~~~~~~~~~~~~~~~";

	for (int i = 0; i < MAX_ISDN_PHONE_NUMBER_IN_SERVICE; i++)
	{
		if (0 != params.phoneRangesList[i].firstPhoneNumber[0])
		{
			os
				<< "\n  Range " << i << "           :" << (char*)(params.phoneRangesList[i].firstPhoneNumber) << "-" << (char*)(params.phoneRangesList[i].lastPhoneNumber)
				<< "\n    DialInGroupId   :" << params.phoneRangesList[i].dialInGroupId
				<< "\n    Category        :" << params.phoneRangesList[i].category
				<< "\n    FirstPortId     :" << (int)params.phoneRangesList[i].firstPortId;
		}
	}

	// ===== 3. ip addresses
	os
		<< "\n"
		<< "\n  IP Addresses      :"
		<< "\n  ~~~~~~~~~~~~~~~~~~~";

	char ipAddress1Str[IP_ADDRESS_LEN], ipAddress2Str[IP_ADDRESS_LEN];

	for (int i = 0; i < MAX_NUM_OF_BOARDS; i++)
	{
		DWORD ipAddress1 = params.ipAddressesList[i].ipAddress_Rtm, ipAddress2 = params.ipAddressesList[i].ipAddress_RtmMedia;

		if (((0 != ipAddress1) && (0xFFFF != ipAddress1)) || ((0 != ipAddress2) && (0xFFFF != ipAddress2)))
		{
			SystemDWORDToIpString(ipAddress1, ipAddress1Str);
			SystemDWORDToIpString(ipAddress2, ipAddress2Str);

			os
				<< "\n  BoardId           :" << params.ipAddressesList[i].boardId
				<< "\n    IP Address 1    :" << ipAddress1Str
				<< "\n    IP Address 2    :" << ipAddress2Str;
		}
	}
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, RTM_ISDN_SERVICE_CANCEL_S& params)
{
	os
		<< "\nRTM_ISDN_SERVICE_CANCEL_S:"
		<< "\n  ServiceName       :" << (char*)params.serviceName
		<< "\n  Status            :" << params.status;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, RTM_ISDN_PHONE_RANGE_UPDATE_S& params)
{
	os
		<< "\nRTM_ISDN_PHONE_RANGE_UPDATE_S:"
		<< "\n  ServiceName       :" << (char*)(params.serviceName)
		<< "\n  Category          :" << params.phoneRange.category
		<< "\n  DialInGroupId     :" << params.phoneRange.dialInGroupId
		<< "\n  FirstPhoneNumber  :" << (char*)(params.phoneRange.firstPhoneNumber)
		<< "\n  FirstPortId       :" << (int)params.phoneRange.firstPortId
		<< "\n  LastPhoneNumber   :" << (char*)(params.phoneRange.lastPhoneNumber)
		<< "\n  Status            :" << params.status;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, DEALLOCATE_BONDING_TEMP_PHONE_S& params)
{
	os
		<<"\nDEALLOCATE_BONDING_TEMP_PHONE_S:"
		<< "\n  BondingPhoneNumber:" << params.BondingTemporaryPhoneNumber
		<< "\n  monitor_conf_id   :" << params.monitor_conf_id
		<< "\n  monitor_party_id  :" << params.monitor_party_id;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, UPDATE_ISDN_PORT_S& params)
{
	os
		<<"\nUPDATE_ISDN_PORT_S:"
		<< "\n  monitor_conf_id   :" << params.monitor_conf_id
		<< "\n  monitor_party_id  :" << params.monitor_party_id
		<< "\n  board_id          :" << params.board_id
		<< "\n  span_id           :" << params.span_id
		<< "\n  connection_id     :" << params.connection_id
		<< "\n  channel_index     :" << params.channel_index;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, ACK_UPDATE_ISDN_PORT_S& params)
{
	os
		<< "\nACK_UPDATE_ISDN_PORT_S:"
		<< "\n  monitor_conf_id   :" << params.monitor_conf_id
		<< "\n  monitor_party_id  :" << params.monitor_party_id
		<< "\n  connection_id     :" << params.connection_id
		<< "\n  channel_index     :" << params.channel_index
		<< "\n  status            :" << params.status;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CONF_RSRC_REQ_PARAMS_S& params)
{
	os
		<< "\nCONF_RSRC_REQ_PARAMS_S:"
		<< "\n  Status            :" << params.status
		<< "\n  monitor_conf_id   :" << params.monitor_conf_id
		<< "\n  SessionType       :" << eSessionTypeNames[params.sessionType]
		<< "\n  ConfMediaType     :" << ConfMediaTypeToString(params.confMediaType)
		<< "\n  MrcMcuId          :" << params.mrcMcuId;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CONF_RSRC_IND_PARAMS_S& params)
{
	os
		<< "\nCONF_RSRC_IND_PARAMS_S:"
		<< "\n  ConfId            :" << params.rsrc_conf_id
		<< "\n  Status            :" << params.status;
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, UNIT_RECONFIG_S& params)
{
	os
		<<"\nUNIT_RECONFIG_S:"
		<< "\n  BoardId           :" << params.boardId
		<< "\n  UnitId            :" << params.unitId
		<< "\n  UnitType          :" << ::CardUnitConfiguredTypeToString(params.unitType)
		<< "\n  UnitStatus        :" << ::UnitReconfigStatusToString(params.unitStatus);
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, SLOTS_NUMBERING_CONVERSION_TABLE_S& params)
{
	CPrettyTable<BoardID, SubBoardID, DisplayBoardID> tbl("BoardId", "SubBoardId" ,"DisplayBoardId");

	for(APIU32 i = 0; i < params.numOfBoardsInTable && i < MAX_NUM_OF_BOARDS; ++i)
	{
		tbl.Add(
			params.conversionTable[i].boardId,
			params.conversionTable[i].subBoardId,
			params.conversionTable[i].displayBoardId);
	}
	os << tbl.Get();
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CARD_REMOVED_IND_S& params)
{
	os
		<< "\nCARD_REMOVED_IND_S:"
		<< "\n  BoardId           :" << params.BoardID
		<< "\n  SubBoardId        :" << params.SubBoardID
		<< "\n  CardType          :" << DUMPSTR(::CardTypeToString(params.cardType));
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, HW_REMOVED_PARTY_LIST_S& params)
{
	CPrettyTable<ConfMonitorID, PartyMonitorID> tbl("ConfMonitorId", "PartyMonitorId");
	for(WORD i = 0; i < params.list_size; ++i)
		tbl.Add(params.conf_party_list[i].monitor_conf_id, params.conf_party_list[i].monitor_party_id);

	os << "\nHW_REMOVED_PARTY_LIST_S:" << tbl.Get();
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, CONF_PARTY_LIST_S& params)
{
	os
		<< "\nCONF_PARTY_LIST_S:"
		<< "\n  BoardId           :" << params.boardId
		<< "\n  UnitId            :" << params.unitId
		<< "\n  ListSize          :" << params.list_size;

	CPrettyTable<ConfMonitorID, ConfRsrcID, PartyRsrcID, DWORD, const char*, WORD> tbl("ConfMonitorId", "ConfId" "PartyId", "PortId", "ResourceType", "AccelId");
	for (WORD i = 0; i < params.list_size; ++i)
		tbl.Add(
				params.conf_party_list[i].monitor_conf_id,
				params.conf_party_list[i].rsrc_conf_id,
				params.conf_party_list[i].rsrc_party_id,
				params.conf_party_list[i].port_id,
				to_string(params.conf_party_list[i].logicalRsrcType),
				params.conf_party_list[i].acceleratorId);

	os << tbl.Get();
	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, UNIT_RECOVERY_S& param)
{
	os
		<< "\nUNIT_RECOVERY_S:"
		<< "\n  BoxId             :" << (WORD)param.unit_recover.box_id
		<< "\n  BoardId           :" << (WORD)param.unit_recover.board_id
		<< "\n  SubBoardId        :" << (WORD)param.unit_recover.sub_board_id
		<< "\n  UnitId            :" << (WORD)param.unit_recover.unit_id;

	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, RECOVERY_REPLACEMENT_UNIT_S& param)
{
	os
		<< "\nRECOVERY_REPLACEMENT_UNIT_S:"
		<< "\n  BoxId             :" << (WORD)param.unit_replacement.box_id
		<< "\n  BoardId           :" << (WORD)param.unit_replacement.board_id
		<< "\n  SubBoardId        :" << (WORD)param.unit_replacement.sub_board_id
		<< "\n  UnitId            :" << (WORD)param.unit_replacement.unit_id
		<< "\n  Status            :" << (WORD)param.status;

	return os;
}

////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& os, PREFERRED_NUMERIC_ID_S& param)
{
	os
		<< "\nPREFERRED_NUMERIC_ID_S:"
		<< "\n  ConfName          :" << param.conf_name
		<< "\n  PreferedNumericId :" << param.numeric_Id
		<< "\n  ConfType          :" << param.conf_type
		<< "\n  Status            :" << param.status;

	return os;
}
