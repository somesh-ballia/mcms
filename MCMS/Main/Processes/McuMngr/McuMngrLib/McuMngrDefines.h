#ifndef MCUMNGRDEFINES_H_
#define MCUMNGRDEFINES_H_


#include "ConfigManagerApi.h"


#define MANAGEMENT_NETWORK_CONFIG_PATH	"Cfg/NetworkCfg_Management.xml"
#define RESTORE_FACTORY_PATH            "States/McmsRestoreFactoryFileInd.flg"
#define FIRST_REBOOT_FILE 				((std::string)(MCU_CONFIG_DIR+"/common/etc/first-boot"))
#define PRODUCT_TYPE_FILE_PATH 			"Cfg/ProductType.xml"


// Licensing
//#define LICENSING_FILE_PATH  "Cfg/License.cfs"
#define BUFFER_SIZE          72

//#define FEATURES_DIGIT					7		// the 8th digit in Options bitmask indicates features (enable/disable)
//#define PARTNERS_DIGITS				    6       // the 7th digit in Options bitmask indicates partners(1-YES,0-NO) [x-x-x-x-x-x-1-x]
//#define CP_PORTS_NUM_DIGIT				4		// the 5th digit in Options bitmask indicates Total number of CP ports [x-x-x-x-1-x-x-x]

#define FEATURES_DIGIT2					1		// the 2nd digit in Options bitmask indicates features (enable/disable)	[x-1-x-x-x-x-x-x]
#define PARTNERS_DIGITS				    2       // the 3th digit in Options bitmask indicates partners(1-YES,0-NO)		[x-x-1-x-x-x-x-x]
#define FEATURES_DIGIT					3		// the 4th digit in Options bitmask indicates features (enable/disable)	[x-x-x-1-x-x-x-x]
#define COP_PORTS_NUM_FIRST_DIGIT		4		// the 5th+6th digits in Options bitmask indicate
#define COP_PORTS_NUM_SECOND_DIGIT		5		// 									Total number of COP ports			[x-x-x-x-1-1-x-x]
#define CP_PORTS_NUM_FIRST_DIGIT		6		// the 7th+8th digits in Options bitmask indicate
#define CP_PORTS_NUM_SECOND_DIGIT		7		// 									Total number of CP ports			[x-x-x-x-x-x-1-1]

#define ENCRYPTION_MASK					0x1		// the LSB in FEATURES_DIGIT indicates Encryption=YES/NO
#define PSTN_MASK						0x2		// the 2nd LSB in FEATURES_DIGIT indicates PSTN=YES/NO
#define TELEPRESENCE_MASK				0x4		// the 3nd LSB in FEATURES_DIGIT indicates Telepresence=YES/NO
// 02.02.09: it's decided that InternalScheduler is not a licensed feature
//#define INTERNAL_SCHEDULER_MASK			0x8		// the 4th LSB in FEATURES_DIGIT indicates InternalScheduler=YES/NO
// 21.4.10: MS in license is not in used. it is decided that this bit will indicate multiple services systems. 
//#define MS_MASK							0x8		// the 4th LSB in FEATURES_DIGIT indicates MS=YES/NO
#define MULTIPLE_SERVICES_MASK			0x8

#define AVAYA_MASK						0x1		// the LSB in PARTNERS_DIGIT indicates Avaya=YES/NO
#define IBM_MASK						0x2		// the 2nd LSB in PARTNERS_DIGIT indicates IBM=YES/NO

//#define ALCATEL_MASK					0x4		// the 3nd LSB in PARTNERS_DIGIT indicates Alcatel=YES/NO
#define HD_MASK                         0x4     // the 3nd LSB in PARTNERS_DIGIT indicates HD=YES/NO

//#define MICROSOFT_MASK					0x8		// the 4th LSB in PARTNERS_DIGIT indicates Microsoft=YES/NO
// Tsahi - MPMX_MASK is not in use anymore in v8.0 , use this bit in the future to support SVC
#define MPMX_MASK                       0x8		// the 4th LSB in PARTNERS_DIGIT indicates MPMX bit on/off,the license multiple by 5 or 10  [x-x-8-x-x-x-x-x]


#define MAX_NUM_OF_CP_PORTS_IN_KEYCODE_RMX4000	80
#define MAX_NUM_OF_CP_PORTS_IN_KEYCODE_RMX2000	40

	
#define CP_PORTS_NUM_INTERVAL			5		// '1' in the Options bitmask == 5 ports; '2' == 10 ports, etc. (according to PM definitions)
#define COP_PORTS_NUM_INTERVAL			5		// '1' in the Options bitmask == 5 ports; '2' == 10 ports, etc. (according to PM definitions)


// NTP
#define NTP_INTERNAL_SERVER_ADDRESS		"169.254.128.16"
#define MIN_LEGAL_YEAR					2000

#define FILE_IDENTIFIER_timeConfigFile  1

#define CORE_DUMPS_FOLDER				((std::string)(MCU_OUTPUT_DIR+"/core/"))

#define MAX_NUM_OF_DHCP_SAMPLING		10

#define MAC_ADDRESS_CONFIG_LEN			20


enum eMngmntIpUpdatePhase
{
	eMngmntIpInit = 0,
	eMngmntIpFromMpl,
	eMngmntIpFromEma,

	MAX_NUM_OF_MNGMNT_IP_UPDATE_PHASE
};
static const char *mngmntIpUpdatePhase[] =
{
	"eMngmntIpUpdatePhase_Init",
	"eMngmntIpUpdatePhase_FromMPL",
	"eMngmntIpUpdatePhase_FromEMA"
};

enum eLicensingValidationState
{
	eLicensingValidationSucceeded = 0,
	eLicensingValidationFailed,
    eLicensingValidationUnknown
};


enum eNtpServerStatus
{
	eNtpServerOk = 0,
	eNtpServerFail,
	eNtpServerConnecting,
	eNtpServerNotConfigured,
	NUM_OF_NTP_SERVER_STATUSES
};

static const char *ntpServerStatusStr[] = 
{
	"Ok",
	"Fail",
	"Connecting",
	"NotConfigured"
};

static const char *GetNtpServerStatusStr(eNtpServerStatus status)
{
	const char *name = (0 <= status && status < NUM_OF_NTP_SERVER_STATUSES
						?
						ntpServerStatusStr[status] : "Invalid NTP server status");
	return name;
}

/*enum eMcuRestoreType
{
        eMcuRestoreNone = 0,
        eMcuRestoreStandard,
        eMcuRestoreInhensive,
	eMcuRestoreBasic,

        NumOfMcuRestoreTypes
};
*/
enum eEthPortType
{
	eEthPortType_Illegal = 0,
	eEthPortType_Management1,
	eEthPortType_Management2,
	eEthPortType_ManagementShelfMngr,
	eEthPortType_Signaling1,
	eEthPortType_Signaling2,
	eEthPortType_Media,
	eEthPortType_Modem,
	eEthPortType_Media_Signaling_Managment,
	eEthPortType_Media_Signaling,
	eEthPortType_ManagementShelfMngr_Managment_Signaling_media,
	eEthPortType_ManagementShelfMngr_Managment_Signaling,
	eEthPortType_ManagementShelfMngr_Managment,
	eEthPortType_ManagementShelfMngr_Signaling,
	NUM_OF_ETH_PORT_TYPES
};

static const char *ethPortTypeStr[] =
{
	"Illegal",
	"Management_1",
	"Management_2",
	"ShM",
	"Signaling_1",
	"Signaling_2",
	"Media",
	"Modem",
	"Media_Signaling_Managment",
	"Media_Signaling",
	"ShM_Managment_Signaling_Media",  //RMX2000 no RTM_LAN port 2
	"ShM_Managment_Signaling",        //RMX2000 with RTM_LAN port 2
	"ShM_Managment" ,                  //RMX2000 MS YES port 2
	"ShM_Signaling"

};

enum eEthMediaPortType
{
	eEthMediaPortType_Illegal = 0,
	eEthMediaPortType_Rtm_Lan,
	eEthMediaPortType_Rtm_Isdn,

};

static const char *ethMediaPortTypeStr[] =
{
	"Illegal",
	"Rtm_Lan",
	"Rtm_Isdn"

};

static const char *GetEthPortTypeStr(eEthPortType type)
{
	const char *name = (0 <= type && type < NUM_OF_ETH_PORT_TYPES
						?
						ethPortTypeStr[type] : "Invalid ETH port type");
	return name;
}

enum eIpTypeConfigSuccess
{
	eIpTypeConfigSuccess_None = 0,
	eIpTypeConfigSuccess_IPv4,
	eIpTypeConfigSuccess_IPv6,
	eIpTypeConfigSuccess_Both,

	MAX_NUM_OF_IP_TYPE_CONFIG_SUCCESS
};
static const char *ipTypeConfigSuccess[] =
{
	"None",
	"IPv4",
	"IPv6",
	"Both"
};

enum eEth802_1xAuthenticationType
{
	eEth802_1xAuthenticationType_Off = 0,
	eEth802_1xAuthenticationType_PEAPv0_MSCHAPV2,
	eEth802_1xAuthenticationType_EAP_TLS,
	eEth802_1xAuthenticationType_EAP_MD5,


	NUM_OF_ETH_802_1X_AUTHENTICATION_TYPE
};

static const char *eth802_1xAuthenticationTypeStr[] =
{
	"Off",
	"PEAPv0_MSCHAPv2",
	"EAP_TLS",
	"EAP_MD5",
};

static const char *Get802_1xAuthenticationTypeStr(eEth802_1xAuthenticationType type)
{
	const char *name = (0 <= type && type < NUM_OF_ETH_802_1X_AUTHENTICATION_TYPE
						?
								eth802_1xAuthenticationTypeStr[type] : "Invalid ETH 802.1x authentication Type");
	return name;
}


#endif /*MCUMNGRDEFINES_H_*/
