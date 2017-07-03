// AllocateStructs.h

#ifndef ALLOCATE_STRUCTS_H_
#define ALLOCATE_STRUCTS_H_

#include <iostream>
#include <iomanip>

#include "MplMcmsStructs.h"
#include "SharedDefines.h"
#include "DefinesIpService.h"
#include "StringsLen.h"
#include "StructTm.h"
#include "CommonStructs.h"
#include "Macros.h"
#include "../../McmIncld/MPL/Card/CardMngrIpMedia/IpCmReq.h"
#include "../../McmIncld/MPL/Card/CardMngrTIP/TipStructs.h"

#include "ObjString.h"
#include "ConfPartyApiDefines.h"
#include "ConfPartySharedDefines.h"

#define SHARED_MEMORY_DEBUG_PRINTS                               0

#define  MAX(a, b) ((a) < (b) ? (b) : (a))

#include "../../McmIncld/Common/IpAddressDefinitions.h"
#include "../../McmIncld/MPL/Card/PhysicalPortVideo/VideoApiDefinitions.h"

extern char* LogicalResourceTypeToString(APIU32 logicalResourceType);
extern char* ResourceTypeToString(APIU32 resourceType);
extern char* RsrcCntlTypeToString(APIU32 rsrcCntlType);
extern const char* ConfMediaTypeToString(eConfMediaType confMediaType);
extern const char* AllocationPolicyToString(APIU32 allocationPolicy);

// rsrcAllocator structures
#define MAX_NUM_ALLOCATED_RSRCS_NET                              30
#define MAX_NUM_ALLOCATED_RSRCS_VID                              3
#define MAX_NUM_ALLOCATED_RSRCS_AUD                              2
#define MAX_NUM_ALLOCATED_RSRCS_RTP_OR_MUX                       1
#ifndef MS_LYNC_AVMCU_LINK
#define MAX_NUM_ALLOCATED_RSRCS_CS                               1
#else
#define MAX_NUM_ALLOCATED_RSRCS_CS                               4	// was 1 (4 for MS Lync)
#endif
#define MAX_NUM_ALLOCATED_RSRCS                                  (MAX_NUM_ALLOCATED_RSRCS_NET + MAX_NUM_ALLOCATED_RSRCS_VID + MAX_NUM_ALLOCATED_RSRCS_AUD + MAX_NUM_ALLOCATED_RSRCS_RTP_OR_MUX + MAX_NUM_ALLOCATED_RSRCS_CS)
#define MAX_NUM_COP_DYNAMIC_RSRCS                                16
#define MAX_NUM_RECV_STREAMS_FOR_VIDEO                           3
#define MAX_NUM_RECV_STREAMS_FOR_CONTENT                         2

#define GENERAL_SERVICE_PROVIDER_NAME_LEN                        MAX(RTM_ISDN_SERVICE_PROVIDER_NAME_LEN, NET_SERVICE_PROVIDER_NAME_LEN)

// UDP
#define IP_NET_SERV_NAME_LEN                                     80

#define MAX_UDP_PORTS                                            20000 // ICE 4 ports

// Max Party Resource/Connection ID in Resource allocator
#define RSRC_ALLOCATOR_MAX_RSRC_PARTY_ID                         (0xEE11FFFF)

#define RSRC_ALLOCATOR_MAX_CONNECTION_ID                         (RSRC_ALLOCATOR_MAX_RSRC_PARTY_ID)

// ***temp - urn on for pstn fixed port simulation path
#define IS_UDP_FIXED_PORT_SIM                                    0

const DWORD RSRC_TIME_SPREADING = 5 * SECOND;

// RMX Capacity constants
#define MAX_NUMBER_OF_MEETING_ROOM_RMX2000                       1000
#define MAX_NUMBER_OF_EQ_RMX2000                                 40
#define MAX_NUMBER_OF_SIP_FACTORY_RMX2000                        40
#define MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_BASED_RMX2000      200
#define MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_RMX2000 400

#define MAX_NUMBER_OF_ONGOING_CONFERENCES_NxM_MPM_PLUS_BASED     60
#define MAX_NUMBER_OF_ONGOING_CONFERENCES_NxM_MPMX_BASED         45

#define MAX_RSRV_IN_LIST_RMX2000                                 2000

// Gesher
#define MAX_RSRV_IN_LIST_GESHER                                  2000
#define MAX_NUMBER_OF_MEETING_ROOM_GESHER                        1000
#define MAX_NUMBER_OF_EQ_GESHER                                  40
#define MAX_NUMBER_OF_SIP_FACTORY_GESHER                         40

// Ninja
#define MAX_RSRV_IN_LIST_NINJA                                   2000
#define MAX_NUMBER_OF_MEETING_ROOM_NINJA                         1000
#define MAX_NUMBER_OF_EQ_NINJA                                   40
#define MAX_NUMBER_OF_SIP_FACTORY_NINJA                          40

// Soft MCU
#define MAX_NUMBER_OF_ONGOING_CONFERENCES_SOFT_BASED             200
#define MAX_NUMBER_OF_ONGOING_CONFERENCES_SOFT_MFW               667//1000 -  because of Embedded issue
#define MAX_RSRC_CONF_ID_NUMBER                                  200
#define MAX_RSRC_CONF_ID_NUMBER_SOFT_MFW                         667//1000

// AmosCapacity constants
#define MAX_NUMBER_OF_MEETING_ROOM_AMOS                          2000
#define MAX_NUMBER_OF_EQ_AMOS                                    80
#define MAX_NUMBER_OF_SIP_FACTORY_AMOS                           80
// no mpm in Amos: #define MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_BASED		 200
#define MAX_NUMBER_OF_ONGOING_CONFERENCES_MPM_PLUS_BASED_AMOS    800
#define MAX_RSRV_IN_LIST_AMOS                                    4000

#define  MAX_REPEATED_RESERVATIONS                               2000
#define MAX_NUMBER_OF_ONGOING_CONFERENCES_COP_VSW_BASED          8
// UDP
const WORD FIRST_UDP_PORT = 49152;
#define DUMMY_BOARD_ID                                           (0xFFFF)
#define DUMMY_SPAN_ID                                            (0xFFFF)
#define DUMMY_CONNECTION_ID                                      (0xFFFFFFFF)

// multiple services
#define MAX_NUM_OF_IP_SERVICES                                   8

// ISDN
#define NUM_E1_PORTS                                             30
#define MAX_NUM_SPANS_ORDER                                      12

#define MAX_NUMBER_OF_SERVICES_IN_RMX_4000                       8
#define MAX_NUMBER_OF_SERVICES_IN_RMX_2000_AND_1500              4


#define IVR_CSRC                                          0x01010101

typedef ipAddressV6If ipv6AddressArray[NUM_OF_IPV6_ADDRESSES];


typedef struct
{
	eIpType          IpType;       /* eIpType */
	ipAddressV4If    IpV4Addr;
	ipv6AddressArray IpV6AddrArray;

	APIU32           type;         /* cmTransportType */

	WORD             AudioChannelPort;
	WORD             AudioChannelAdditionalPorts;
	WORD             VideoChannelPort;
	WORD             VideoChannelAdditionalPorts;
	WORD             FeccChannelPort;
	WORD             FeccChannelAdditionalPorts;
	WORD             ContentChannelPort;
	WORD             ContentChannelAdditionalPorts;
	WORD             BfcpChannelPort;
	WORD             BfcpChannelAdditionalPorts;
} UdpAddresses;

enum eSessionType
{
	esession_type_none       = 0,
	eCP_session              = 1,
	eVS_session              = 2,
	eVOICE_session           = 3,
	eSTANDALONE_session      = 4,
	eCOP_HD1080_session      = 5,
	eCOP_HD720_50_session    = 6,
	eVSW_28_session          = 7,
	eVSW_56_session          = 8,
	eVSW_Auto_session        = 9,
	eCP_ContentXcode_session = 10,
	NUM_OF_SESSION_TYPES     = 11 // DONT FORGET TO UPDATE THIS
};


enum eXcodeRsrcType
{
	eXcodeH264Encoder       = 0,
	eXcodeH263Encoder       = 1,
	eXcodeH264LinksEncoder  = 2,     // H264
	eXcodeContentDecoder    = 3,
	MAX_CONTENT_XCODE_RSRCS = 4,     // DONT FORGET TO UPDATE THIS
	eXcodeEncoderDummy      = 5
};

static const char* eXcodeRsrcTypeNames[] =
{
	"eXcodeH264Encoder",
	"eXcodeH263Encoder",
	"eXcodeH264LinksEncoder",
	"eXcodeContentDecoder",
	"MAX_CONTENT_XCODE_RSRCS",
	"eXcodeEncoderDummy"
};

enum eRsrcConfType
{
	eConf_type_none = 0,
	eMR_type        = 1,
	eEQ_type        = 2,
	eSipFact_type   = 3,

	NUM_OF_RSRC_CONF_TYPES = 4   // DONT FORGET TO UPDATE THIS
};

inline const char* to_string(eRsrcConfType val)
{
	static const char* enumNames[] =
	{
		"eConfType_None",
		"eConfType_MeetingRoom",
		"eConfType_EntryQueue",
		"eConfType_SipFactory",
	};
	return (eConf_type_none <= val && val < NUM_OF_RSRC_CONF_TYPES) ? enumNames[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eRsrcConfType val) { return os << to_string(val); }


static const char* eSessionTypeNames[] =
{
	"esession_type_none",
	"eCP_session",
	"eVS_session",
	"eVOICE_session",
	"eSTANDALONE_session",
	"eCOP_HD1080_session",
	"eCOP_HD720_50_session",
	"eVSW_28_session",
	"eVSW_56_session",
	"eVSW_Auto_session",
	"eCP_ContentXcode_session"
};

enum eNetworkPartyType
{
	eNetwork_party_type_none,
	eIP_network_party_type,
	eISDN_network_party_type,
	NUM_OF_NETWORK_PARTY_TYPES
};

enum eVideoPartyType // Remember to change char * eVideoPartyTypeNames[]
{ //do not change order - ordered by size
	eVideo_party_type_dummy,
	eVideo_party_type_none,                                // None is for audio only calls
	eVSW_video_party_type,                                 // VSW party - all resolutions
	eCP_VP8_upto_CIF_video_party_type,
	eCP_H264_upto_CIF_video_party_type,
	eCP_H261_H263_upto_CIF_video_party_type,
	eCP_VP8_upto_SD30_video_party_type,
	eCP_H264_upto_SD30_video_party_type,
	eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type,    // in MPM-Rx - H263 4CIF equals to H264 SD resource
	eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type,
	eCP_H264_upto_HD720_30FS_Symmetric_video_party_type,
	eCP_Content_for_Legacy_Decoder_video_party_type,
	eCP_Content_for_Legacy_Decoder_HD1080_video_party_type,
	eCOP_party_type,
	eCP_H264_upto_HD720_60FS_Symmetric_video_party_type,
	eCP_H261_CIF_equals_H264_HD1080_video_party_type,      //NGB: because in MPM-Rx mode H261 CIF takes same resource as H264 HD1080p30
	eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type,
	eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type,
	eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type, // MPMx only
	eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type,  // MPM-Rx only
	eVideo_relay_CIF_party_type,
	eVideo_relay_SD_party_type,
	eVideo_relay_HD720_party_type,
	eVideo_relay_HD1080_party_type,
	eVoice_relay_party_type,
	eVSW_relay_CIF_party_type,
	eVSW_relay_SD_party_type,
	eVSW_relay_HD720_party_type,
	eVSW_relay_HD1080_party_type,
	eVSW_Content_for_CCS_Plugin_party_type,
	NUM_OF_VIDEO_PARTY_TYPES
};


typedef enum
{
	E_NORMAL,
	E_AC_MASTER,
	E_AC_SLAVE,
	E_AC_RESERVED,
	E_VIDEO_MASTER_LB_ONLY,
	E_VIDEO_SLAVE_FULL_ENCODER,
	E_VIDEO_MASTER_SPLIT_ENCODER,
	E_VIDEO_SLAVE_SPLIT_ENCODER,
	E_VIDEO_MASTER_SPLIT_ENCODER_HALF_DSP,
	E_VIDEO_SLAVE_SPLIT_ENCODER_HALF_DSP,
	E_VIDEO_CONTENT_DECODER,
	E_VIDEO_CONTENT_ENCODER,
	E_ENCODER_PCM
} ECntrlType;

static const char* eNetworkPartyTypeNames[] =
{
	"eNetwork_party_type_none",
	"eIP_network_party_type",
	"eISDN_network_party_type",
};

static const char * eVideoPartyTypeNames[] =
{
	"eVideo_party_type_dummy",
	"eVideo_party_type_none",
	"eVSW_video_party_type",
	"eCP_VP8_upto_CIF_video_party_type",
	"eCP_H264_upto_CIF_video_party_type",
	"eCP_H261_H263_upto_CIF_video_party_type",
	"eCP_VP8_upto_SD30_video_party_type",
	"eCP_H264_upto_SD30_video_party_type",
	"eCP_H263_upto_4CIF_H264_upto_SD30_video_party_type",
	"eCP_VP8_upto_HD720_30FS_Symmetric_video_party_type",
	"eCP_H264_upto_HD720_30FS_Symmetric_video_party_type",
	"eCP_Content_for_Legacy_Decoder_video_party_type",
	"eCP_Content_for_Legacy_Decoder_HD1080_video_party_type",
	"eCOP_party_type",
	"eCP_H264_upto_HD720_60FS_Symmetric_video_party_type",
	"eCP_H261_CIF_equals_H264_HD1080_video_party_type",
	"eCP_Content_for_Legacy_Decoder_HD1080_60FS_video_party_type",
	"eCP_H264_upto_HD1080_30FS_Symmetric_video_party_type",
	"eCP_H264_upto_HD1080_60FS_Asymmetric_video_party_type",
	"eCP_H264_upto_HD1080_60FS_Symmetric_video_party_type",
	"eVideo_relay_CIF_party_type",
	"eVideo_relay_SD_party_type",
	"eVideo_relay_HD720_party_type",
	"eVideo_relay_HD1080_party_type",
	"eVoice_relay_party_type",
	"eVSW_relay_CIF_party_type",
	"eVSW_relay_SD_party_type",
	"eVSW_relay_HD720_party_type",
	"eVSW_relay_HD1080_party_type",
	"eVSW_Content_for_CCS_Plugin_party_type",
	"NUM_OF_VIDEO_PARTY_TYPES"
};

static const char* ePartyRoleNames[] =
{
	"eParty_Role_regular_party",
	"eParty_Role_content_decoder",
	"eParty_Role_content_encoder",
	"eParty_Role_AvMcuLink_SlaveIn",
	"eParty_Role_AvMcuLink_SlaveOut"
};

typedef enum
{
	eNoAllocationPolicy = 0,
	eAllocateAllRequestedResources,
	eAllowDowngradingToAudioOnly,
	eLastAllocationPolicy
} EAllocationPolicy;


static const char* eTIPPartyTypeNames[] =
{
	"eTip_None",
	"eTip_Master_Center",
	"eTip_Slave_Left",
	"eTip_Slave_Rigth",
	"eTip_Slave_Aux",
	"eTip_Video_Aux",
	"eTip_Last"
};


// general struct for ISDN/PSTN
// direction: conf party ==> resource allocator
// description: board_id and span_id will be 0xFFFF for dial out calls and filled in dial in call, RA use it to detect dial out / dial in call
// num_of_isdn_ports can be used by conf rate (or by conf-party policy) or 1 for initial channel
typedef struct
{
	WORD board_id;
	WORD span_id;
	WORD num_of_isdn_ports;
	BYTE serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	BYTE isBondingTemporaryPhoneNumberNeeded;            // YES/NO
	char conferencePhoneNumber[PHONE_NUMBER_DIGITS_LEN]; // if length more than 7 digits - prefix should be the same
} ISDN_SPAN_PARAMS_S;


enum ePartyRole
{
	eParty_Role_regular_party,
	eParty_Role_content_decoder,    // allocate decoder only
	eParty_Role_content_encoder,    // allocate encoder only
	eParty_Role_AvMcuLink_SlaveIn,  // allocate decoder only
	eParty_Role_AvMcuLink_SlaveOut, // allocate encoder only

	NUM_OF_PARTY_ROLES
};

// uses opcode:
// direction: conf party ==> resource allocator
// description: party allocation request
typedef struct
{
	ConfMonitorID            monitor_conf_id;
	PartyMonitorID           monitor_party_id;
	PartyRsrcID              party_id;                       // Party resource Id
	DWORD                    artCapacity;                    // Calculated in Kbytes by formula artCapacity=(X[KB incoming] + Y[KB outgoing]) * (1 + 0.15 * LPR + 0.1 * STEREO + 0.25 * ENCRYPTION)
	eNetworkPartyType        networkPartyType;
	eVideoPartyType          videoPartyType;
	eSessionType             sessionType;
	WORD                     serviceId;                      // for UDP allocation
	WORD                     subServiceId;
	DWORD                    optionsMask;                    // 000...000 = roll-back if any allocation failed
	EAllocationPolicy        allocationPolicy;
	BYTE                     isWaitForRsrcAndAskAgain;
	BYTE                     bRmxPortGaugeThresholdExceeded; // PortGauge
	ISDN_SPAN_PARAMS_S       isdn_span_params;
	BOOL                     isIceParty;
	ETipPartyTypeAndPosition tipPartyType;                   // for TIP allocation
	WORD                     tipNumOfScreens;
	DWORD                    room_id;
	BOOL                     isBFCP;
	eConfMediaType           confMediaType;
	ePartyRole               partyRole;
	eAvMcuLinkType           avMcuLinkType; // MS Lync
	BOOL                     reqMsSsrc;
	PartyRsrcID              mainPartyRsrcID;
	eHdVswTypesInMixAvcSvc   HdVswTypeInMixAvcSvcMode;

} ALLOC_PARTY_REQ_PARAMS_S;

inline std::ostream& operator<<(std::ostream& os, const ALLOC_PARTY_REQ_PARAMS_S& in)
{
	os
	<< "\nALLOC_PARTY_REQ_PARAMS_S:"
	<< "\n  monitor_conf_id            :" << in.monitor_conf_id
	<< "\n  monitor_party_id           :" << in.monitor_party_id
	<< "\n  rsrc_party_id              :" << in.party_id
	<< "\n  artCapacity (Kbytes)       :" << in.artCapacity
	<< "\n  videoPartyType             :" << eVideoPartyTypeNames[in.videoPartyType] << " (" << in.videoPartyType << ")"
	<< "\n  networkPartyType           :" << eNetworkPartyTypeNames[in.networkPartyType] << " (" << in.networkPartyType << ")"
	<< "\n  sessionType                :" << eSessionTypeNames[in.sessionType] << " (" << in.sessionType << ")"
	<< "\n  serviceId                  :" << in.serviceId
	<< "\n  subServiceId               :" << in.subServiceId
	<< "\n  optionsMask                :" << in.optionsMask
	<< "\n  allocationPolicy           :" << AllocationPolicyToString(in.allocationPolicy) << " (" << in.allocationPolicy << ")"
	<< "\n  rmxPortGaugeThreshold      :" << (WORD)in.bRmxPortGaugeThresholdExceeded
	<< "\n  isWaitForRsrcAndAskAgain   :" << (WORD)in.isWaitForRsrcAndAskAgain
	<< "\n  isIceParty                 :" << (WORD)in.isIceParty
	<< "\n  isBFCP                     :" << (WORD)in.isBFCP
	<< "\n  tipPartyType               :" << eTIPPartyTypeNames[in.tipPartyType] << " (" << in.tipPartyType << ")"
	<< "\n  tipNumOfScreens            :" << in.tipNumOfScreens
	<< "\n  room_id                    :" << in.room_id
	<< "\n  confMediaType              :" << ConfMediaTypeToString(in.confMediaType) << " (" << in.confMediaType << ")"
	<< "\n  partyRole                  :" << ePartyRoleNames[in.partyRole] << " (" << in.partyRole << ")"
	<< "\n  reqMsSsrc                  :" << (WORD)in.reqMsSsrc
	<< "\n  avMcuLinkType              :" << eAvMcuLinkTypeNames[in.avMcuLinkType] << " (" << in.avMcuLinkType << ")"
	<< "\n  mainPartyRsrcID            :" << in.mainPartyRsrcID
	<< "\n  hdVswEnabledInMixAvcSvcMode:" << in.HdVswTypeInMixAvcSvcMode;

	if (in.networkPartyType == eISDN_network_party_type)
	{
		os
		<< "\nISDN_SPAN_PARAMS_S:"
		<< "\n  board_id                   :" << in.isdn_span_params.board_id
		<< "\n  span_id                    :" << in.isdn_span_params.span_id
		<< "\n  num_of_isdn_ports          :" << in.isdn_span_params.num_of_isdn_ports
		<< "\n  serviceName                :" << (char*)(in.isdn_span_params.serviceName)
		<< "\n  isBondingPhoneNumberNeeded :" << (in.isdn_span_params.isBondingTemporaryPhoneNumberNeeded ? "TRUE" : "FALSE")
		<< "\n  conferencePhoneNumber      :" << in.isdn_span_params.conferencePhoneNumber;
	}

	return os;
}



// uses opcode: REALLOCATE_RTM_ON_BOARD_FULL_REQ
// direction: conf party ==> resource allocator
// description: RTM (H.320 only) board full reallocate request
typedef struct
{
	ALLOC_PARTY_REQ_PARAMS_S allocPartyReqParams;
	DWORD                    connectionIdList[NUM_E1_PORTS];
} BOARD_FULL_REQ_PARAMS_S;

typedef struct
{
	DWORD                 connectionId;
	eLogicalResourceTypes logicalRsrcType;

	// DWORD           optionsMask; //000...000 = rollback if any alloc fail.
} ALLOCATED_RSRC_PARAM_S;

typedef struct
{
	DWORD                 connectionId;
	eLogicalResourceTypes logicalRsrcType;
	DWORD                 rsrcEntityId;
} ALLOCATED_COP_RSRC_PARAM_S;

// uses opcode:
// direction: resource allocator ==> conf party
// description: spans order that will set the priority of RTM card to chose the "next" span for dial out
typedef struct
{
	WORD  board_id;
	DWORD conn_id;
	WORD  spans_list[MAX_NUM_SPANS_ORDER];
} SPANS_ORDER_LIST_PER_PORT;

// uses opcode:
// direction: resource allocator ==> conf party
// description: spans order that will set the priority of RTM card to chose the "next" span for dial out
typedef struct
{
	SPANS_ORDER_LIST_PER_PORT port_spans_list[NUM_E1_PORTS];
} SPANS_ORDER_LIST;

// uses opcode:
// direction: resource allocator ==> conf party
// description: common indication from resource allocator
typedef struct
{
	DWORD                  status;
	DWORD                  rsrc_conf_id;
	DWORD                  rsrc_party_id;
	DWORD                  room_id;
	DWORD                  numRsrcs;
	eNetworkPartyType      networkPartyType;
	eVideoPartyType        videoPartyType;
	ALLOCATED_RSRC_PARAM_S allocatedRrcs[MAX_NUM_ALLOCATED_RSRCS];
	BOOL                   isIceParty;
	eConfMediaType         confMediaType;
	ePartyRole             partyRole;
	BOOL                   isAvcVswInMixedMode;
} ALLOC_PARTY_IND_PARAMS_S_BASE;

// uses opcode:
// direction: resource allocator ==> conf party
// description: common indication from resource allocator
typedef struct
{
	SPANS_ORDER_LIST spans_order;
	char             BondingTemporaryPhoneNumber[PHONE_NUMBER_DIGITS_LEN];
	WORD             num_of_isdn_ports; // for downgrade
} ISDN_PARTY_IND_PARAMS_S;


typedef struct
{
	DWORD m_ssrcAudio;
	DWORD m_ssrcContent[MAX_NUM_RECV_STREAMS_FOR_CONTENT];
	DWORD m_ssrcVideo[MAX_NUM_RECV_STREAMS_FOR_VIDEO];
} SVC_PARTY_IND_PARAMS_S;

typedef struct
{
	DWORD m_msSsrcFirst;
	DWORD m_msSsrcLast;
} MS_SSRC_PARTY_IND_PARAMS_S;

typedef struct
{
	ALLOC_PARTY_IND_PARAMS_S_BASE allocIndBase;
	UdpAddresses                  udpAdresses;
	ISDN_PARTY_IND_PARAMS_S       isdnParams;
	SVC_PARTY_IND_PARAMS_S        svcParams;
	MS_SSRC_PARTY_IND_PARAMS_S    msSsrcParams;
} ALLOC_PARTY_IND_PARAMS_S;

typedef struct
{
	ALLOC_PARTY_IND_PARAMS_S allocInd;
	DWORD                   partyId;
} ALLOC_PARTY_PARAMS_S;

typedef struct
{
	ConfMonitorID          monitor_conf_id;
	PartyMonitorID         monitor_party_id;
	PartyRsrcID            party_id;
	DWORD                  numOfRsrcsWithProblems;
	ALLOCATED_RSRC_PARAM_S rsrcsWithProblems[MAX_NUM_ALLOCATED_RSRCS];
	BYTE                   is_problem_with_UDP_ports; // TRUE|FALSE
	BYTE                   serviceName[GENERAL_SERVICE_PROVIDER_NAME_LEN];
	BYTE                   resetArtUnitOnKillPort;    // YES = 1 , NO = 0
	BYTE                   force_kill_all_ports;
	APIU32                 rtp_ice_channels[MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS];
	APIU32                 rtcp_ice_channels[MAX_NUM_OF_ALLOCATED_PARTY_UDP_PORTS];
} DEALLOC_PARTY_REQ_PARAMS_S;

inline std::ostream& operator<<(std::ostream& os, const DEALLOC_PARTY_REQ_PARAMS_S& in)
{
	os
	<< "\nDEALLOC_PARTY_REQ_PARAMS_S:"
	<< "\n  monitor_conf_id            :" << in.monitor_conf_id
	<< "\n  monitor_party_id           :" << in.monitor_party_id
	<< "\n  rsrc_party_id              :" << in.party_id
	<< "\n  serviceName                :" << DUMPSTR((char*)in.serviceName)
	<< "\n  is_problem_with_UDP_ports  :" << (WORD)in.is_problem_with_UDP_ports
	<< "\n  resetArtUnitOnKillPort     :" << (WORD)in.resetArtUnitOnKillPort
	<< "\n  force_kill_all_ports       :" << (WORD)in.force_kill_all_ports;

	for (WORD i = 0; i < ARRAYSIZE(in.rtp_ice_channels); ++i)
		os << "\n  rtp/rtcp ice_channels[" << i << "]   :" << in.rtp_ice_channels[i] << ", " << in.rtcp_ice_channels[i];

	for (WORD i = 0; i < ARRAYSIZE(in.rsrcsWithProblems) && i < in.numOfRsrcsWithProblems; ++i)
		os << "\n  connectionId/resourceType  :" << in.rsrcsWithProblems[i].connectionId << ", " << ::LogicalResourceTypeToString(in.rsrcsWithProblems[i].logicalRsrcType);

	return os;
}

typedef struct
{
	DWORD                 monitor_conf_id;
	eSessionType          sessionType;
	DWORD                 status;
	eLogicalResourceTypes logicalTypeList[4];  // could be define(eLogical_COP_HD1080_Master_encoder,eLogical_COP_HD720_encoder
	                                           // eLogical_COP_CIF_encoder, eLogical_COP_CIF_encoder    )
	eConfMediaType        confMediaType;
	WORD                  mrcMcuId;

} CONF_RSRC_REQ_PARAMS_S;

typedef struct
{
	DWORD                      status;
	DWORD                      rsrc_conf_id;
	DWORD                      num_predefinedRsrcs;

	ALLOCATED_COP_RSRC_PARAM_S allocatedRrcs[MAX_NUM_ALLOCATED_RSRCS];  // 9 ??define
	ALLOCATED_COP_RSRC_PARAM_S dec_conn_id_list[MAX_NUM_COP_DYNAMIC_RSRCS];

	DWORD                      pcmMenuId;                               // Eitan-PCM??
} CONF_RSRC_IND_PARAMS_S;

typedef struct
{
	DWORD                  status;
	DWORD                  target_monitor_conf_id;
	DWORD                  target_rsrc_conf_id;
	DWORD                  monitor_party_id;
	DWORD                  rsrc_party_id;

	DWORD                  numRsrcs;
	ALLOCATED_RSRC_PARAM_S allocatedRrcs[MAX_NUM_ALLOCATED_RSRCS];
} PARTY_MOVE_RSRC_IND_PARAMS_S;

typedef struct
{
	DWORD source_monitor_conf_id;
	DWORD target_monitor_conf_id;
	DWORD source_monitor_party_id;
	DWORD target_monitor_party_id;
	BYTE  serviceName[GENERAL_SERVICE_PROVIDER_NAME_LEN];
} PARTY_MOVE_RSRC_REQ_PARAMS_S;

typedef struct
{
	DWORD status;         // OK / FAIL
} DEALLOC_PARTY_IND_PARAMS_S;

typedef struct
{
	WORD board_id;
} BOARD_ID_IND_PARAMS_S;

// conn-to-card
#define CONN_TO_CARD_TABLE_NAME "ConnToCardTable"
#define CONN_TO_CARD_TABLE_SIZE  12600

class ConnToCardTableEntry
{
public:
	ConnToCardTableEntry()
	{
		m_id             = 0;
		rsrc_conf_id     = 0;
		rsrc_party_id    = 0;
		boxId            = 0;
		boardId          = 0;
		subBoardId       = 0;
		unitId           = 0;
		acceleratorId    = 0;
		portId           = 0;
		physicalRsrcType = ePhysical_res_none;
		rsrcType         = eLogical_res_none;
		rsrcCntlType     = E_NORMAL;
		channelId        = 0;
		ipAdress         = 0;
		channelId        = 0;
		room_id          = 0;
	}

	ConnToCardTableEntry(const ConnToCardTableEntry& rHnd)
	{
		m_id             = rHnd.m_id;
		rsrc_conf_id     = rHnd.rsrc_conf_id;
		rsrc_party_id    = rHnd.rsrc_party_id;
		boxId            = rHnd.boxId;
		boardId          = rHnd.boardId;
		subBoardId       = rHnd.subBoardId;
		unitId           = rHnd.unitId;
		acceleratorId    = rHnd.acceleratorId;
		portId           = rHnd.portId;
		physicalRsrcType = rHnd.physicalRsrcType;
		rsrcType         = rHnd.rsrcType;
		rsrcCntlType     = rHnd.rsrcCntlType;
		channelId        = rHnd.channelId;
		ipAdress         = rHnd.ipAdress;
		channelId        = rHnd.channelId;
		room_id          = rHnd.room_id;
	}

	void Dump(std::ostream& msg) const
	{
		msg << "ConnToCardTableEntry::Dump:"
		    << "\n  Id                  :" <<  m_id
		    << "\n  Rsrc Conf Id        :" <<  rsrc_conf_id
		    << "\n  Rsrc Party Id       :" <<  rsrc_party_id
		    << "\n  Box Id              :" <<  boxId
		    << "\n  Board Id            :" <<  boardId
		    << "\n  Sub Board Id        :" <<  subBoardId
		    << "\n  Unit Id             :" <<  unitId
		    << "\n  Accelerator Id      :" <<  acceleratorId
		    << "\n  Port Id             :" <<  portId
		    << "\n  Physical Rsrc Type  :" << ::ResourceTypeToString(physicalRsrcType)
		    << "\n  Rsrc Type           :" << ::LogicalResourceTypeToString(rsrcType)
		    << "\n  Rsrc Cntl Type      :" << ::RsrcCntlTypeToString(rsrcCntlType)
		    << "\n  Channel Id          :" <<  channelId
		    << "\n  IP Address          :" <<  ipAdress
		    << "\n  Room Id             :" <<  room_id << "\n";
	}

	void DumpRaw(CLargeString& mstr)
	{
		mstr << "ConnectionId:"<<m_id<<",ConfId:"<<rsrc_conf_id<<",PartyId:"<<rsrc_party_id<<",Box:"<<boxId<<",Board:"<< boardId <<",SubBoard:"<<subBoardId<<",Unit:"<<unitId<<",Accelerator:"<<acceleratorId<<",Port:"<<portId;
		mstr << ",PhysicalRsrcType:"<<::ResourceTypeToString(physicalRsrcType)<<",RsrcType:"<<::LogicalResourceTypeToString(rsrcType)<<",RsrcCntlType:"<<::RsrcCntlTypeToString(rsrcCntlType);
		mstr << ",Channel Id:"<<channelId<<",IPAddress:"<<ipAdress;
	}

	void DumpRaw(const char* calledFrom)
	{
		if (!SHARED_MEMORY_DEBUG_PRINTS)
			return;

		CMedString mstr;
		mstr << "ConnToCardTableEntry::DumpRaw (called from "<< calledFrom <<"):\n";
		mstr << "ConnectionId:"<<m_id<<",ConfId:"<<rsrc_conf_id<<",PartyId:"<<rsrc_party_id<<",Box:"<<boxId<<",Board:"<< boardId <<",SubBoard:"<<subBoardId<<",Unit:"<<unitId<<",Accelerator:"<<acceleratorId<<",Port:"<<portId;
		mstr << ",PhysicalRsrcType:"<<::ResourceTypeToString(physicalRsrcType)<<",RsrcType:"<<::LogicalResourceTypeToString(rsrcType)<<",RsrcCntlType:"<<::RsrcCntlTypeToString(rsrcCntlType);
		mstr << ",Channel Id:"<<channelId<<",IPAddress:"<<ipAdress;
		PTRACE1(eLevelInfoNormal, mstr.GetString());
	}

	DWORD                 m_id;
	DWORD                 rsrc_conf_id;
	DWORD                 rsrc_party_id;
	DWORD                 room_id;
	WORD                  boxId;
	WORD                  boardId;
	WORD                  subBoardId;
	WORD                  unitId;
	WORD                  acceleratorId;
	WORD                  portId;

	eResourceTypes        physicalRsrcType;
	eLogicalResourceTypes rsrcType;
	ECntrlType            rsrcCntlType;      // indicates whenever video is slave / master / stand alone

	// fields added for PSTN use
	WORD  channelId;                         // will be inserted for PSTN case into RTM, Audio Enc/Dec entry
	DWORD ipAdress;                          // Media address in Audio Enc/Dec entry, RTM adress in RTM entry.
};

#define MAX_NUM_PQS    8//4

enum eUDPMediaType
{
	eUDPaudio           = 0,
	eUDPvideo           = 1,
	eUDPfecc            = 2,
	eUDPcontent         = 3,
	NUM_UDP_MEDIA_TYPES = 4
};

inline const char* to_string(eUDPMediaType val)
{
	static const char* enumNames[] =
	{
		"eUDPaudio",
		"eUDPvideo",
		"eUDPfecc",
		"eUDPcontent",
	};
	return (eUDPaudio <= val && val < NUM_UDP_MEDIA_TYPES) ? enumNames[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eUDPMediaType val) { return os << to_string(val); }

enum eUdpAllocMethod
{
	eMethodStatic  = 0,
	eMethodDynamic = 1,

	NUM_OF_UDP_ALLOC_METHODS = 2     // DONT FORGET TO UPDATE THIS
};

inline const char* to_string(eUdpAllocMethod val)
{
	static const char* enumNames[] =
	{
		"eMethodStatic",
		"eMethodDynamic",
	};
	return (eMethodStatic <= val && val < NUM_OF_UDP_ALLOC_METHODS) ? enumNames[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eUdpAllocMethod val) { return os << to_string(val); }

typedef struct
{
	eUdpAllocMethod portsAlloctype;                        // static/ dynamic?
	WORD            boxId;
	WORD            boardId;
	WORD            subBoardId;
	WORD            PQid;                                  // 1,2,3,4
	WORD            type;
	eIpType         IpType;                                /* eIpType */
	ipAddressV4If   IpV4Addr;
	ipAddressV6If   IpV6Addr[NUM_OF_IPV6_ADDRESSES];
	WORD            UdpFirstPort;
	WORD            UdpLastPort;
	WORD            subServiceId;
}IP_SERVICE_UDP_RESOURCE_PER_PQ_S;

typedef struct
{
	WORD                             ServId;                                      // the only meaningful field in this req.
	char                             ServName[NET_SERVICE_PROVIDER_NAME_LEN];
	WORD                             numPQSactual;
	IP_SERVICE_UDP_RESOURCE_PER_PQ_S IPServUDPperPQList[MAX_NUM_PQS];             // 4
	eIceEnvironmentType              iceEnvironment;
	BYTE                             service_default_type;
} IP_SERVICE_UDP_RESOURCES_S;

typedef struct
{
	WORD          boxId;
	WORD          boardId;
	WORD          subBoardId;
	WORD          PQid;                          // 1,2,3,4
	ipAddressV6If IpV6Addr[NUM_OF_IPV6_ADDRESSES];
} IP_SERVICE_UDP_RESOURCE_PER_PQ_FOR_IPV6_UPDATE_S;

typedef struct
{
	WORD                                             ServId;
	IP_SERVICE_UDP_RESOURCE_PER_PQ_FOR_IPV6_UPDATE_S IPServUDPperPQList[MAX_NUM_PQS];   // 4
} IPV6_ADDRESS_UPDATE_RESOURCES_S;

typedef struct
{
	char defaultH323ServiceName[NET_SERVICE_PROVIDER_NAME_LEN];
	char defaultSIPServiceName[NET_SERVICE_PROVIDER_NAME_LEN];
}DEFAULT_IP_SERVICE_S;

typedef struct
{
    WORD ServId;
    char ServName[NET_SERVICE_PROVIDER_NAME_LEN];
    char HostName[NET_SERVICE_PROVIDER_NAME_LEN];
    BOOL IsSecured;
    BOOL IsRequestPeerCertificate;
    BYTE Revocation_method;
    char OcspURL[GENERAL_MES_LEN];
    BYTE IsMsService;
} IP_SERVICE_CERTMNGR_S;
///////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	eUdpAllocMethod portsAlloctype;                        // static/ dynamic?

	WORD            boxId;
	WORD            boardId;
	WORD            subBoardId;

	WORD            PQid;                        // 1,2,3,4

	WORD            type;

// ipAddressIf                  IpAddr;
	eIpType       IpType;                    /* eIpType */
	ipAddressV4If IpV4Addr;
	ipAddressV6If IpV6Addr[NUM_OF_IPV6_ADDRESSES];

	//WORD          UdpFirstPort;
	//WORD          UdpLastPort;
	//WORD          subServiceId;
}IP_SERVICE_UDP_MCUMNGR_PER_PQ_S;

typedef struct
{
	WORD                             ServId;
	eServerStatus         			 dnsStatus;
	char                             DefaultGatewayIPv6[IPV6_ADDRESS_LEN];
	DWORD                            DefaultGatewayMaskIPv6;
	WORD                             numPQSactual;                      //added for 802.1x
	IP_SERVICE_UDP_MCUMNGR_PER_PQ_S  IPServUDPperPQList[MAX_NUM_PQS];   //added for 802.1x

} IP_SERVICE_MCUMNGR_S;

typedef struct
{
	DWORD   id;
	DWORD   type;
	char    name[NET_SERVICE_PROVIDER_NAME_LEN];
	DWORD   span_ips[MAX_SPAN_NUMBER_IN_SERVICE];
	eIpType ipType;
	char    span_ipv6s[IPV6_ADDRESS_LEN];

} IP_SERVICE_TCPDUMP_S;

#define AUD_CNTLER_UNIT_ID                     11
#define AUD_CNTLER_UNIT_ID_BARAK_OPTION_1      13
#define AUD_CNTLER_UNIT_ID_BARAK_OPTION_2      14

#define AUD_CNTLER_UNIT_ID_BREEZE_PRIORITY_1   20
#define AUD_CNTLER_UNIT_ID_BREEZE_PRIORITY_2   15

// numericId / monitorId alloc
#define MAX_NUMERIC_ID                         1000

#define NUMERIC_CONF_ID_MIN_LEN                1  // should be taken from SYSTEM.CFG
#define NUMERIC_CONF_ID_MAX_LEN                16 // should be taken from SYSTEM.CFG
#define NUMERIC_CONF_ID_LEN                    5  // should be taken from SYSTEM.CFG,
// for non specific allocation

#define NUM_NON_SPECIFIC_NUM_CONF_ID_ALLOC_TRY 10

typedef struct                                  // temp, probably CCommRes should be transfered instead...
{
	DWORD status;
	DWORD monitor_Id;                             // monitor_Id = 0xFFFFFFFF non-specific
	char  numeric_Id[NUMERIC_CONF_ID_MAX_LEN+1];  // numeric_Id[0] = '\0' non-specific
	BYTE  is_numericId_needed;
}CONF_MONITOR_NUMERIC_ID;

#define MAX_SYSTEM_NUM_MR                      200

typedef struct
{
	DWORD         meeting_room_monitor_Id;                 // monitor_Id = 0xFFFFFFFF non-specific
	char          numeric_Id[NUMERIC_CONF_ID_MAX_LEN+1];   // numeric_Id[0] = '\0' non-specific
	eRsrcConfType conf_type;
	char          serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	char          firstPhoneNumber[PHONE_NUMBER_DIGITS_LEN];
	char          secondPhoneNumber[PHONE_NUMBER_DIGITS_LEN];

	// WORD              numServicePhoneStr ;
	// CServicePhoneStr* servicePhoneStr;
}MR_MONITOR_NUMERIC_ID_S;

typedef struct
{
	DWORD         monitor_Id;                              // monitor_Id = 0xFFFFFFFF non-specific
	char          numeric_Id[NUMERIC_CONF_ID_MAX_LEN+1];   // numeric_Id[0] = '\0' non-specific
	eRsrcConfType conf_type;
	char          conf_name[H243_NAME_LEN+1];
	DWORD         status;
}PREFERRED_NUMERIC_ID_S;

typedef struct
{
	DWORD                    list_size;
	MR_MONITOR_NUMERIC_ID_S* monitor_numeric_list;
}MR_MONITOR_NUMERIC_ID_LIST_S;

typedef struct
{
	DWORD status;
	DWORD meeting_room_monitor_Id;
}MR_IND_S;

typedef struct
{
	DWORD     list_size;
	MR_IND_S* mr_list;
}MR_IND_LIST_S;

typedef struct
{
	eVideoPartyType maxVideoPartyType;
	DWORD           profile_Id;
}PROFILE_IND_S;   // will be used for PROFILE_UPDATE_RSRC_IND and PROFILE_ADD_RSRC_IND and PROFILE_DELETE_RSRC_IND

typedef struct
{
	DWORD          list_size;
	PROFILE_IND_S* profile_list;
}PROFILE_IND_LIST_S;

typedef struct
{
	MR_MONITOR_NUMERIC_ID_LIST_S mr_list;
	PROFILE_IND_LIST_S           profile_list;
}MR_AND_PROFILE_IND_LISTS;  // will be used in STARTUP_READ_MR_AND_PROFILE_DB_IND

enum eRPRTtypes
{
	TYPE_TOTAL = 0,
	// TYPE_BAD    ,
	TYPE_OCCUPIED,
	TYPE_RESERVED,
	TYPE_FREE,
	NUM_RPRT_TYPES
};

inline const char* to_string(eRPRTtypes val)
{
	static const char* enumNames[] =
	{
		"TYPE_TOTAL",
		"TYPE_OCCUPIED",
		"TYPE_RESERVED",
		"TYPE_FREE"
	};
	return (TYPE_TOTAL <= val && val < NUM_RPRT_TYPES) ? enumNames[val] : "Invalid value";
}

inline std::ostream& operator <<(std::ostream& os, eRPRTtypes val) { return os << to_string(val); }

// For AVAYA
typedef struct
{
	WORD m_numVideoParties[NUM_RPRT_TYPES-1];
	WORD m_numAudioParties[NUM_RPRT_TYPES-1];
	WORD max_num_ongoing_conf;
	WORD current_num_ongoing_conf;
} ALLOC_REPORT_PARAMS_S;

typedef struct
{
	DWORD last_conf_id;
}LAST_CONF_ID_S;

typedef struct
{
	// Udp Range
	WORD UdpFirstPort;
	WORD UdpLastPort;

	// Id number of Ip Service
	DWORD ServiceId;
}UDP_PORT_RANGE_S;

typedef struct
{
	// Max Parties
	DWORD MaxPartiesNum;

	// Licensing
	DWORD Licensing;
}RSRC_CFS_S;

// general struct for ISDN/PSTN
// direction: conf party ==> resource allocator
// description: board_id and span_id will be the physical port uses by RTM
// connection_id will be 0xFFFFFFFF in case of dial in (and will be fill by RA in the Ack)
typedef struct
{
	DWORD monitor_conf_id;
	DWORD monitor_party_id;
	WORD  board_id;
	WORD  span_id;
	DWORD connection_id;
	DWORD channel_index;
} UPDATE_ISDN_PORT_S;

// general struct for ISDN/PSTN
// direction: resource allocator ==> conf party
// description: connection_id will be fill by RA.
// status is not ok in case of inconsistency
typedef struct
{
	DWORD monitor_conf_id;
	DWORD monitor_party_id;
	DWORD connection_id;
	DWORD status;
	DWORD channel_index;
} ACK_UPDATE_ISDN_PORT_S;

// general struct for ISDN/PSTN
// direction: resource allocator ==> conf party
// description: connection_id will be fill by RA.
// status is not ok in case of inconsistency
typedef struct
{
	DWORD monitor_conf_id;
	DWORD monitor_party_id;
	BYTE  serviceName[RTM_ISDN_SERVICE_PROVIDER_NAME_LEN];
	char  BondingTemporaryPhoneNumber[PHONE_NUMBER_DIGITS_LEN];
} DEALLOCATE_BONDING_TEMP_PHONE_S;

typedef struct
{
	DWORD monitor_conf_id;
	DWORD monitor_party_id;
} CONF_PARTY_ID_S;

typedef struct
{
	DWORD            list_size;
	CONF_PARTY_ID_S* conf_party_list;
} HW_REMOVED_PARTY_LIST_S;

typedef struct
{
	DWORD     monitorConfId;
	CStructTm newEndTime;
} SET_CONFERENCE_ENDTIME_REQ_PARAMS_S;

typedef struct
{
	DWORD     status;
	DWORD     monitorConfId;
	CStructTm newEndTime;
	bool      isSetByOperator;
} SET_CONFERENCE_ENDTIME_IND_PARAMS_S;

typedef struct
{
	DWORD status;
	DWORD monitor_conf_id;
	DWORD monitor_party_id;
	WORD  m_Direction;
	DWORD m_RemoteIPAddress;
	WORD  m_VideoPort;
	WORD  m_AudioPort;
} START_PREVIEW_IND_PARAMS_S;

typedef struct
{
	DWORD status;
	DWORD monitor_conf_id;
	DWORD monitor_party_id;
	WORD  m_Direction;
} STOP_PREVIEW_IND_PARAMS_S;

typedef struct
{
	DWORD rsrc_conf_id;
	DWORD rsrc_party_id;
} ALLOC_PCM_RSRC_REQ_PARAMS_S;

typedef struct
{
	DWORD                  status;
	DWORD                  rsrc_conf_id;
	DWORD                  rsrc_party_id;
	ALLOCATED_RSRC_PARAM_S allocatedPcmRsrc;
	DWORD                  pcmMenuId;
} ALLOC_PCM_RSRC_IND_PARAMS_S;

// Content Transcoding
typedef struct
{
	DWORD                 monitor_conf_id;
	DWORD                 rsrc_conf_id;
	DWORD                 rsrc_party_id;
	DWORD                 port_id;
	eLogicalResourceTypes logicalRsrcType;
	WORD                  acceleratorId;
} CONF_PARTY_ELEMENTS_S;


typedef struct
{
	DWORD                  boardId;
	DWORD                  unitId;
	DWORD                  list_size;
	CONF_PARTY_ELEMENTS_S* conf_party_list;
} CONF_PARTY_LIST_S;


typedef struct
{
	ALLOC_PARTY_REQ_PARAMS_S rsrc_params_list[MAX_CONTENT_XCODE_RSRCS];
} CONFERENCE_RSRC_REQ_PARAMS_S;

typedef struct
{
	DWORD                    status;
	ALLOC_PARTY_IND_PARAMS_S allocatedRsrcs[MAX_CONTENT_XCODE_RSRCS];
} CONFERENCE_RSRC_IND_PARAMS_S;

#endif  // ALLOCATE_STRUCTS_H_

