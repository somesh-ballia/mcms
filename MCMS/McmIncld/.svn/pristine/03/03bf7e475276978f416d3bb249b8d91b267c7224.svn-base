#ifndef COMMONSTRUCTS_H_
#define COMMONSTRUCTS_H_


#include "DataTypes.h"
#include "SharedDefines.h"
#include "DefinesIpService.h"
#include "VersionStruct.h"


//////////////////////////////
//   CONSTANTS DEFINITIONS
//////////////////////////////

#define IPV6_ADDRESS_LEN             64

#define MAX_NUM_OF_INTERFACES			16
#define MAX_ROUTERS_IN_H323_SERVICE		5
#define MAX_NUM_OF_PORTS_SPEED			3
#define MAX_NUM_OF_LAN_PORTS			30

#define PORT_INACTIVE			    0
#define PORT_ACTIVE			    1
#define PORT_STANDBY			    2

// Ninja specific
#define VIDEO_UNIT_START_NUMBER_NINJA              6
#define MAX_VIDEO_PORTS_PER_DSP_NINJA              48
#define NETRA_DSP_CHIP_COUNT_NINJA                 18


//////////////////////////////
//       ENUMERATORS
//////////////////////////////
// ================================
// ===== eV6ConfigurationType =====
// ================================
enum eV6ConfigurationType
{
	eV6Configuration_Auto			= 0,
	eV6Configuration_DhcpV6			= 1,
	eV6Configuration_Manual			= 2,

	NUM_OF_V6_CONFIGURATION_TYPES	= 3	 //DONT FORGET TO UPDATE THIS
};


// =================================
// ============ eIpType ============
// =================================
enum eIpType
{
	eIpType_None			= 0,
	eIpType_IpV4			= 1,
	eIpType_IpV6			= 2,
	eIpType_Both			= 3,
	
	NUM_OF_IP_TYPES		= 4      //DONT FORGET TO UPDATE THIS
};


// =================================
// === eDnsDhcpConfigurationType ===
// =================================
enum eDnsDhcpConfigurationType
{
	eDnsDhcpV4                           = 0,
	eDnsDhcpV6                           = 1,

	NUM_OF_DNS_DHCP_CONFIGURSTION_TYPES  = 2      //DONT FORGET TO UPDATE THIS
};


// =================================
// === ePortSpeedType ===
// =================================
enum ePortSpeedType
{
	ePortSpeed_Auto                      = 0,
    ePortSpeed_10_HalfDuplex,
    ePortSpeed_10_FullDuplex,
    ePortSpeed_100_HalfDuplex,
    ePortSpeed_100_FullDuplex,
    ePortSpeed_1000_HalfDuplex,
    ePortSpeed_1000_FullDuplex,

	NUM_OF_PORT_SPEED_TYPES
};


// =================================
// ======== eEthSettingsState ======
// =================================
enum eEthSettingsState
{
	eEthSettingsState_ok	= 0,
	eEthSettingsState_fail,

	NUM_OF_ETH_SETTINGS_STATES
};


// =================================
// ======== eConfSpeakerChangeMode ======
// =================================

typedef enum
{
	E_CONF_SPEAKER_CHANGE_MODE_DUMMY			= -1,
	E_CONF_DEFAULT_SPEAKER_CHANGE_MODE			= 0,
	E_CONF_FAST_SPEAKER_CHANGE_MODE				= 1,
	E_CONF_SPEAKER_CHANGE_MODE_LAST
} EConfSpeakerChangeMode;
 



//////////////////////////////
//    API STRUCTURES
//////////////////////////////


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  ipConfiguration structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct
{
	APIU32 aliasType;
	APIU8 aliasContent[ALIAS_NAME_LEN];
} ALIAS_S;


typedef struct
{
	APIU32  dynamicPortAllocation;
	APIU32  signallingFirstPort;
	APIU32  signallingNumPorts;
	APIU32  controlFirstPort;
	APIU32  controlNumPorts;
	APIU32  audioFirstPort;
	APIU32  audioNumPorts;
	APIU32  videoFirstPort;
	APIU32  videoNumPorts;
	APIU32  contentFirstPort;
	APIU32  contentNumPorts;
	APIU32  feccFirstPort;
	APIU32  feccNumPorts;
	APIU32  numIntendedCalls;
	APIU32  enablePortRange;

	APIU32  future_use1;
	APIU32  future_use2;
} IP_PORT_RANGE_S;


typedef struct
{
	APIUBOOL         isDHCPv4InUse;
	APIU32           iPv4Address;

	APIU32           future_use1;
	APIU32           future_use2;
} IPV4_S;


typedef struct
{
	APIU32    configurationType;	// eV6ConfigurationType (auto/dhcp/manual)
	APIU8     iPv6Address[IPV6_ADDRESS_LEN];
	APIU32    _6To4RelayAddress;

	APIU32    future_use1;
	APIU32    future_use2;
} IPV6_S;


typedef struct
{
	IPV4_S   iPv4;
	IPV6_S   iPv6s[NUM_OF_IPV6_ADDRESSES];
	APIU32   ipType;	 // eIpType - IpV4/IpV6/both/none
    ALIAS_S  aliasesList[MAX_ALIAS_NAMES_NUM];
	APIU32   boardId;    // id of card
	APIU32   pqId;       // id of PowerQuick, just index

	APIUBOOL isSecured;
	APIUBOOL isPermanentOpen;
	
	IPV4_S   iPv4Internal;

	APIU32  future_use1;
	APIU32  future_use2;
} IP_INTERFACE_S;


typedef struct
{
    APIU32  routerIp;
    APIU32  remoteIp;
    APIU32  remoteFlag;

	APIU32  subnetMask;
	APIU32  future_use2;
} IP_ROUTER_S;


typedef struct
{
	APIUBOOL    isDhcpInUse; //TRUE/FALSE
	APIU32      dhcpServerIpAddress;
	APIU32      dhcpState;

	APIU32      subnetMask;
	APIU32      defaultGateway;
	APIU8       defaultGatewayIPv6[IPV6_ADDRESS_LEN];
	IP_ROUTER_S ipRouter[MAX_ROUTERS_IN_H323_SERVICE];

	APIU32      vLanMode;
	APIU32      vLanId;

	APIU32      future_use1;
	APIU32      future_use2;
} NETWORK_PARAMS_S;


typedef struct
{
	IP_INTERFACE_S     interfacesList[MAX_NUM_OF_INTERFACES];  // what's the length?
	NETWORK_PARAMS_S   networkParams;

	APIU32             future_use1;
	APIU32             future_use2;
} IP_CONFIGURATION_S;


typedef struct
{
	APIU32    dnsServerStatus;
	APIU32    dnsConfiguredFromDHCPv4_or_DHCPv6;
	APIU8     hostName[NAME_LEN];
	APIU8     domainName[NAME_LEN];
	
	APIU32    ipV4AddressList[NUM_OF_DNS_SERVERS];
	APIU8     ipV6AddressList[NUM_OF_DNS_SERVERS][IPV6_ADDRESS_LEN];
	// max total items (addresses) in both lists: three (either v4 or v6)
	
	APIUBOOL  isRegister;

	APIU32    future_use1;
	APIU32    future_use2;
} DNS_CONFIGURATION_S;


typedef struct
{
    APIU32  portNum;
    APIU32  portSpeed;

    APIU32  future_use1;
	APIU32  future_use2;
}PORT_SPEED;


typedef struct
{
	IP_INTERFACE_S       interfacesList[MAX_NUM_OF_PQS];
	NETWORK_PARAMS_S     networkParams;
	DNS_CONFIGURATION_S  dnsConfig;
    	PORT_SPEED           portSpeedList[MAX_NUM_OF_PORTS_SPEED];
	APIU32               v35GwIpv4Address; 
    
	APIU32               future_use1;
	APIU32               future_use2;
} IP_PARAMS_S;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  end of ipConfiguration structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	Log Level structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct
{
    APIU32 log_level;  // eLogLevel
} LOG_LEVEL_S;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  end of Log Level structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Max num of connected parties structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct
{
    APIU32             muxNumOfParticipantsToChangeLogLevel;  
} MAX_NUM_OF_PARTICIPANTS_TO_CHANGE_LOG_LEVEL_S;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//  Max num of connected parties structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	EthernetSettings structures
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct
{
	APIU32	slotId;
	APIU32	portNum;
    
	APIU32	future_use1;
	APIU32	future_use2;
} ETH_SETTINGS_PORT_DESC_S;


typedef struct
{
	ETH_SETTINGS_PORT_DESC_S	portParams;
    APIU32						portSpeed;	// ePortSpeedType
    
	APIU32						future_use1;
	APIU32						future_use2;
} ETH_SETTINGS_CONFIG_S;


typedef struct
{
	ETH_SETTINGS_PORT_DESC_S	portParams;
	APIU32						configState; // eEthSettingsState
    
	APIU32						future_use1;
	APIU32						future_use2;
} ETH_SETTINGS_STATE_S;


typedef struct
{
    APIU32 ulRxPackets;
    APIU32 ulRxBadPackets;
    APIU32 ulRxCRC;
    APIU32 ulRxOctets;
    
    APIU32 ulMaxRxPackets;
    APIU32 ulMaxRxBadPackets;
    APIU32 ulMaxRxCRC;
    APIU32 ulMaxRxOctets;
    
    APIU32 ulTxPackets;
    APIU32 ulTxBadPackets;
    APIU32 ulTxFifoDrops;
    APIU32 ulTxOctets;
    
    APIU32 ulMaxTxPackets;
    APIU32 ulMaxTxBadPackets;
    APIU32 ulMaxTxFifoDrops;
    APIU32 ulMaxTxOctets;
    
    APIU32 ulActLinkStatus;
    APIU32 ulActLinkMode;
    APIU32 ulActLinkAutoNeg;
    
    APIU32 ulAdvLinkMode;
    APIU32 ulAdvLinkAutoNeg;                    

	//3 params for the 802.1x monitoring
	APIU32 e802_1xSuppPortStatus;
	APIU32 e802_1xMethod;
	APIU32 e802_1xFailReason;	        
} ETH_SETTINGS_S;


typedef struct
{
	ETH_SETTINGS_PORT_DESC_S	portParams;
	ETH_SETTINGS_S				monitoringParams;
    
	APIU32						future_use1;
	APIU32						future_use2;
} ETH_SETTINGS_SPEC_S;


typedef struct
{
	ETH_SETTINGS_PORT_DESC_S	portParams;
    APIU32						isActive;	// ulActLinkStatus (from ETH_SETTINGS_S)
    APIU32						isMounted;	// (is there a NIC for this port) 
	
	//3 params for the 802.1x monitoring
	APIU32                      e802_1xSuppPortStatus;
	APIU32                      e802_1xMethod;
	APIU32                      e802_1xFailReason;	
    
	APIU32						future_use1;
	APIU32						future_use2;
} ETH_SETTINGS_ACTIVE_PORT_S;


typedef struct
{
	ETH_SETTINGS_ACTIVE_PORT_S	activePortsList[MAX_NUM_OF_LAN_PORTS];
    
	APIU32						future_use1;
	APIU32						future_use2;    
} ETH_SETTINGS_ACTIVE_PORTS_LIST_S;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	end of EthernetSettings structurs
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

typedef struct
{
     APIU8   description[GENERAL_MES_LEN];
} FAULT_GENERAL_S;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	CM High CPU Usage struct
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct
{
	   APIU8	board_id;
       APIU8	is_exceed;
	   APIU8	future_use1;
	   APIU8	future_use2;    
} CM_HIGH_CPU_USAGE_S;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	end of CM High CPU Usage struct
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


typedef struct
{
	APIU32	isActiveDirectoryExist;
} ACTIVE_DIRECTORY_STATE_S;


#endif /*COMMONSTRUCTS_H_*/


