// CardsStructs.h
//
//////////////////////////////////////////////////////////////////////
#if !defined(_CardsStructs_H__)
#define _CardsStructs_H__


#include "DataTypes.h"
#include "CommonStructs.h"
#include "PhysicalResource.h"
#include "MplMcmsStructs.h"
#include "IpChannelParams.h"


//////////////////////////////
//   CONSTANTS DEFINITIONS
//////////////////////////////
#define MAX_NUM_OF_SW_VERSIONS			20
#define MAX_NUM_OF_SW_VERSION_DIGITS	16
#define MAX_NUM_OF_SLOTS            	20
#define SM_COMP_NAME_LEN				20
#define SYSCFG_FAILURE_DESCRIPTION_LEN	120
#define SYSCFG_PARAM_VAL_ERROR_READ          0xffffffff
#define SYSCFG_PARAM_VAL_ERROR_WRITE         0xfffffffe
#define MOUNT_NOT_CONNECTED 			0xDEAD
#define MOUNT_CONNECTED 				0
#define MAX_IP_LENGTH					64 //IPV6 is the largest
#define MAX_NUM_DNS_SERVICES			3
#define MAX_DNS_DOMAIN_NAME				64
//////////////////////////////
//       ENUMERATORS
//////////////////////////////

// =================================
// =========== eCardType ===========
// =================================
enum eCardType
{
	eEmpty				= 0,
	eCpuBoard			= 1,
	eSwitch				= 2,
	eMfa_26				= 3,
	eMfa_13				= 4,
	eRtmIsdn			= 5,
	eControl			= 6,
	eMpmPlus_20			= 7,	// Barak - half carrier (9 DSPs)
	eMpmPlus_40			= 8,	// Barak - carrier only (16 DSPs)
	eMpmPlus_80			= 9,	// Barak - with two mezzanines (32 DSPs)
	eMpmPlus_MezzanineA	= 10,	// asked by Emb (for development only!!): Barak with one Mezzanine (24 DSPs)
	eMpmPlus_MezzanineB	= 11,	// asked by Emb (for development only!!): Barak with one Mezzanine (24 DSPs)
	eMpmx_40			= 12,
	eMpmx_80			= 13,
	eMpmx_20			= 14,	// MPMX 10 DSPs
	eMpmx_Soft_Half		= 15,	// For soft MCU (1 Video DSP + 1 Audio DSP) - for weak core
	eMpmx_Soft_Full		= 16,	// For soft MCU (1 Video DSP + 1 Audio DSP) - for strong core
	eMpmRx_Half			= 17,	// MPMRX - half card - 5 Netra DSPs, 15 DaVinci DSPs
	eMpmRx_Full			= 18,	// MPMRX - full card - 17 Netra DSPs, 15 DaVinci DSPs
	eMpmRx_Ninja		= 19,
	eRtmIsdn_9PRI		= 20,	// RTM_ISDN new card keren
	eRtmIsdn_9PRI_10G	= 21,	// RTM_ISDN new card keren
    eRtmLan             = 22,   // RTM_ISDN new card keren
    eRtmLan4            = 23,   // RTM_ISDN new card keren
    eRtmLan4_10G        = 24,   // RTM_ISDN new card keren

	NUM_OF_CARD_TYPES	= 25	//DONT FORGET TO UPDATE THIS
};

// =================================
// ========== eUnitTypes: ==========
// ====== before configuration =====
// ==== and after configuration ====
// =================================
enum eCardUnitTypePhysical
{
	ePQ             = 0, //PowerQuick cpu
	eDsp            = 1,

	eUndefined      = 0xff,

	NUM_OF_CARD_UNIT_TYPES_NOT_CONFIGURED = 3
};


enum eCardUnitTypeConfigured
{
	eArt			= 0,
	eVideo			= 1,
	eArtCntlr		= 2,
	ePost			= 3, // for Emb internal use!
	eRtm			= 4, // for Emb internal use!

	eNotConfigured	= 0xff,

	NUM_OF_CARD_UNIT_TYPES_CONFIGURED = 5
};

enum eUnitReconfigStatus
{
	eUnitReconfigOk               = 0,  // type should be checked in unitType field
	eUnitReconfigFail              = 1,  // this status means that unit will be in disable mode, type should be checked in unitType field

	NUM_OF_UNIT_RECONFIGURATION_STATUSES = 2
};



// =================================
// ===== eCardUnitLoadedStatus =====
// =================================
enum eCardUnitLoadedStatus
{
	eOk          = 0,
	eUnitStartup = 1,
	eUnknown     = 2,
	eFatal       = 3,
	eReady       = 4,
	eNotExist    = 5,
	eNotLoaded   = 6,

	NUM_OF_CARD_UNIT_STATUSES = 7
};

// =================================
// ===== eMediaIpConfigStatus ======
// =================================
enum eMediaIpConfigStatus
{
	eMediaIpConfig_Ok           = 0,
	eMediaIpConfig_NotSupported = 1,
	eMediaIpConfig_NotExist     = 2,
	eMediaIpConfig_IpFail       = 3,
	eMediaIpConfig_IpDuplicate  = 4,
	eMediaIpConfig_DhcpFail     = 5,
	eMediaIpConfig_VLanFail     = 6,

	NUM_OF_MEDIA_IP_CONFIG_STATUSES = 7
};


// =================================
// ========== eCardState ===========
// =================================
enum eCardState
{
	eNormal            = 0,
	eMajorError        = 1,
	eMinorError        = 2,
	eSimulation        = 3,
	eCardStartup       = 4,
	eNoConnection      = 5,
	eDisabled          = 6,

	NUM_OF_CARD_STATES = 7
};

// =====================================================
// ========== eChecksumSimulationErrorStatus ===========
// =====================================================
enum eChecksumSimulationErrorStatus
{
    eStatusInactiveSimulation = 0,
    eStatusFailChecksumTest = 1
};

///////////////////////////////////////////////
// new ShelfManager struct
///////////////////////////////////////////////
// =================================
// ========= eSmUnitStatus =========
// =================================
enum eSmComponentStatus
{
	eSmComponentOk			= 0,
	eSmComponentMajor		= 1,
	eSmComponentNotExist	= 2,
	eSmComponentResetting	= 3,
	eSmComponentDiagnostic	= 4,
	eSmComponentPowerOff	= 5,

	NUM_OF_SM_UNIT_STATUSES	= 6
};

// =================================
// ========= eHotSwap===============
// =================================

enum eHotSwapRemoveType
{
	e_Remain            = 0,
	e_Removed           = 1,

	NUM_OF_HOT_SWAP_REMOVE_TYPES  = 2
};


// =================================
// ======= eSystemCardsMode ========
// ===== [MPM/MPM+/MPMX/MPMRX] =====
// =================================
enum eSystemCardsMode
{
	eSystemCardsMode_illegal = 0,
	eSystemCardsMode_mpm,
	eSystemCardsMode_mpm_plus,
	eSystemCardsMode_breeze,
	eSystemCardsMode_mpmrx,
	eSystemCardsMode_mixed_mode,
		
	NUM_OF_SYSTEM_CARDS_MODES
};
#ifndef CARD_TYPE_MPM_PLUS
static const char *sSystemCardsModes[] =
{
    "illegal_mode",	// eSystemCards_illegal
    "mpm",			// eSystemCards_mpm
    "mpm_plus",		// eSystemCards_mpm_plus
    "mpmx",         // eSystemCardsMode_breeze
    "mpmrx",        // eSystemCardsMode_mpmrx
    "mixed_mode"    //eSystemCardsMode_mixed_mode
};
#endif
typedef enum
{
	ePcmLangEnglish = 0,
	ePcmLangChineseSimplified,
	ePcmLangChineseTraditional,
	ePcmLangJapanese,
	ePcmLangGerman,
	ePcmLangFrench,
	ePcmLangSpanish,
	ePcmLangKorean,
	ePcmLangPortuguese,
	ePcmLangItalian,
	ePcmLangRussian,
	ePcmLangNorwegian,
	ePcmLangLast  //  must be last

} pcmLanguageEnum;
#ifndef CARD_TYPE_MPM_PLUS
static const char *sPcmLanguages[] =
{
	"English"
	"Chinese Simplified",
	"Chinese Traditional",
	"Japanese",
	"German",
	"French",
	"Spanish",
	"Korean",
	"Portuguese",
	"Italian",
	"Russian",
	"Norwegian"

};
#endif
//////////////////////////////
//    API STRUCTURES
//////////////////////////////

typedef struct
{
	APIU8  versionDescriptor[ONE_LINE_BUFFER_LEN];
	APIU8  versionNumber[MAX_NUM_OF_SW_VERSION_DIGITS];
} CM_SW_VERSION_S;


typedef struct
{
	APIU8            serialNum[MPL_SERIAL_NUM_LEN];
	APIU32           status;
	APIU32           cardType;
	APIU8            postResultsList[MAX_NUM_OF_UNITS];
	APIU8            unitsTypesList[MAX_NUM_OF_UNITS];
	VERSION_S        hardwareVersion;
	CM_SW_VERSION_S  swVersionsList[MAX_NUM_OF_SW_VERSIONS];
} CM_CARD_MNGR_LOADED_S;


typedef struct
{
	APIU32  type;
	APIU32  pqNumber;   // for ART units - their corresponding PowerQuick CPU
} CM_UNIT_CONFIG_PARAMS_S;


typedef struct
{
	CM_UNIT_CONFIG_PARAMS_S  unitsParamsList[MAX_NUM_OF_UNITS];
} CM_UNITS_CONFIG_S;

typedef struct
{
	APIU32  status;
} CM_CS_IP_MEDIA_S;

typedef  struct 
{
	APIU32 vlanId;
	APIU32 port;
	IPV4_S internalIpV4;
	IPV4_S externalIpV4;
}CM_CONFIG_DNAT_S;

typedef  struct 
{
	APIU8 IpServer[MAX_NUM_DNS_SERVICES][MAX_IP_LENGTH];
	APIU8  domainName[MAX_DNS_DOMAIN_NAME];
}CM_DNS_MEDIA_CONFIG_S;


typedef struct
{
	APIU32  status;
	APIU32  statusOfUnitsList[MAX_NUM_OF_UNITS];
} CM_UNIT_LOADED_S;


typedef struct
{
	IP_PARAMS_S  ipParams;
	APIU32       serviceId;
	APIU8        serviceName[NET_SERVICE_PROVIDER_NAME_LEN];
	APIU32       platformType;	// former 'future_use1'
	APIU32       csIp;	// former 'future_use2'
	
} MEDIA_IP_PARAMS_S;


typedef struct
{
	APIU32  status;
	APIU32  serviceId;

	IPV4_S	iPv4;
	IPV6_S	iPv6[NUM_OF_IPV6_ADDRESSES];
	APIU32	ipType;	 // eIpType - IpV4/IpV6/both/none

	APIU32  pqNumber;

	APIU8   defaultGatewayIPv6[IPV6_ADDRESS_LEN];

	APIU32  future_use1;
	APIU32  future_use2;
} MEDIA_IP_CONFIG_S;


typedef struct
{
	APIU32 service_id;
	APIU32 status;
} CM_DELETE_IP_SERVICE_S;


typedef struct
{
	PHYSICAL_INFO_HEADER_S  physicalHeader;
	APIU8                   errorDescriptionStr[ERROR_MESSAGE_LEN];
} CM_CARD_RESET_S;

typedef struct
{
	APIU32  boardId;
	APIU32  unitId;
    APIU32  unitType; //eCardUnitTypeConfigured
    APIU32  unitStatus;

} UNIT_RECONFIG_S;

//it was decided (17/1/06) that CM_FATAL_FAILURE_IND will send SWITCH_SM_KEEP_ALIVE_S
/*
typedef struct
{
	APIU32  errorLevel;
	APIU8   errorDescriptionStr[ERROR_MESSAGE_LEN];
} CM_FATAL_FAILURE_S;
*/


typedef struct
{
	APIU8 	serialNum[MPL_SERIAL_NUM_LEN]; // for authentication
} CM_CARD_MANAGER_RECONNECT_S;


typedef struct
{
	APIU32  ulMounted;					// STATUS_OK or STATUS_FAIL
	APIU32  ulReason;					// value of the mount system call
	APIU32  ulServerIpAddrss;			// server's ip address
	APIU8	path[MAX_FULL_PATHNAME];	// files path
} CM_FOLDER_MOUNT_S;


typedef struct
{
	APIU32  status;
	APIU32  statusOfUnitsList[MAX_NUM_OF_UNITS];
} KEEP_ALIVE_S;


typedef struct
{
	APIUBOOL resetMfa[MAX_NUM_OF_BOARDS];
} MFA_RESET_S;

typedef struct
{
	APIU32  status;
} UPGRADE_NEW_VERSION_READY_IND_S;

/*
//using TStartupDebugRecordingParamReq instead
typedef struct
{
	APIU8 mediaRecordingFilePath[MAX_FULL_PATHNAME];
} MEDIA_RECORDING_S;
*/


typedef struct
{
	APIU32  boardId;
	APIU32  subBoardId;
	APIU32	displayBoardId;
} SLOT_NUMBERING_CONVERTER_S;


typedef struct
{
	APIU32 						numOfBoardsInTable;
	SLOT_NUMBERING_CONVERTER_S	conversionTable[MAX_NUM_OF_BOARDS];
} SLOTS_NUMBERING_CONVERSION_TABLE_S;


///////////////////////////////////////////////
// new ShelfManager struct
///////////////////////////////////////////////
typedef struct
{
	APIU32 unSlotId;
	APIU32 unSubBoardId;
	APIU32 unStatus;	// eSmComponentStatus
	APIU32 unStatusDescriptionBitmask;		// 0-------00000
											//			   |> Other
											//			  |>  Voltage
											//			 |>   Temperature major
											//          |>    Temperature critical
											//         |>     Failed (gone through a reset)
											//        |>      Power_OFF

	APIU8  sSmCompName[SM_COMP_NAME_LEN];
} SM_COMPONENT_STATUS_S;


typedef struct
{
	SM_COMPONENT_STATUS_S unSmComp1;
	SM_COMPONENT_STATUS_S unSmComp2;
	SM_COMPONENT_STATUS_S unSmComp3;
	SM_COMPONENT_STATUS_S unSmComp4;
	SM_COMPONENT_STATUS_S unSmComp5;
	SM_COMPONENT_STATUS_S unSmComp6;
	SM_COMPONENT_STATUS_S unSmComp7;
	SM_COMPONENT_STATUS_S unSmComp8;
	SM_COMPONENT_STATUS_S unSmComp9;
	SM_COMPONENT_STATUS_S unSmComp10;
	SM_COMPONENT_STATUS_S unSmComp11;
	SM_COMPONENT_STATUS_S unSmComp12;
	SM_COMPONENT_STATUS_S unSmComp13;
	SM_COMPONENT_STATUS_S unSmComp14;
	SM_COMPONENT_STATUS_S unSmComp15;
	SM_COMPONENT_STATUS_S unSmComp16;
	SM_COMPONENT_STATUS_S unSmComp17;
	SM_COMPONENT_STATUS_S unSmComp18;
	SM_COMPONENT_STATUS_S unSmComp19;
	SM_COMPONENT_STATUS_S unSmComp20;
} SWITCH_SM_KEEP_ALIVE_S;
typedef struct
{
	APIU32 unSubBoardId;
	APIU32 unStatus;	// eSmComponentStatus
	APIU32 unStatusDescriptionBitmask;		// 0-------00000
												//			   |> Other
												//			  |>  Voltage
												//			 |>   Temperature major
												//          |>    Temperature critical
												//         |>     Failed (gone through a reset)
												//        |>      Power_OFF

	//APIU8  sSmCompName[SM_COMP_NAME_LEN];
	APIU8 RtmExist;
	APIU8 IsdnExist;
} SLOTS_CNF_S;


typedef struct
{
	APIU32       unDebugMode;
	APIU32       unJITCMode;
	APIU32       unSeparatedNetworks;
	APIU32		 unMultipleServices;
	APIU32       unV35Mode;  //for Rad vision GW
	APIU32		 unRtmLan;
	APIU8        sDescription[SYSCFG_FAILURE_DESCRIPTION_LEN];
	APIU32       unPcmLanguage;       //values from pcmLanguageEnum

	APIU32	     unReportDspCrash;
	APIU32       unLanRedundancy; // Added for LanRedundancy option -16/03/2011 (Liron.B)
	APIU32		 unEnableTcPackage;
	APIU32		 unTcLatencySize;
	APIU32		 unTcBurstSize;
	APIU32       unMonitoringPacket; // Added for handle packets -08/09/2011 (Liron.B)
	APIU32       unNtpLocalServerStratum; //takes values 1-16 for the STRATUM of the local NTP server
	APIU32 		 unIsIpV4ResposeEcho;
	APIU32 		 unCheckArping;
    APIU32       unPacketLossMajorValue;
    APIU32       unPacketLossCriticalValue;
	APIU8		 fEnablePacketLossIndication;
    APIU32       unRtmLanMtuSize;
	APIU32 		 unMaxNumOfParticipantsToChangeLogLevel;
	APIU32 		 unEnableAcceptingIcmpRedirect;
	APIU32 		 unEnableSendingIcmpDestinationUnreachable;
    APIU32       unMfaThreshold;
    APIU32 		 unNumOfPcmInMpmx;
    APIU32		 unIsPreferSvcOverLyncCapacity; // PREFER_SVC_OVER_LYNC_CAPACITY flag
} SYSCFG_PARAMS_S;


//2 modes cop/cp
typedef enum eSystemConfMode
{
      eSystemConfMode_Cp = 0,
      eSystemConfMode_Cop,
      eSystemConfMode_Mixed, // for future use

      NUM_OF_SYSTEM_CONF_MODES
} E_SYSTEM_CONF_TYPE;
#ifndef CARD_TYPE_MPM_PLUS
static const char *sSystemConfModes[] =
{
    "SystemConfMode_Cp",
    "SystemConfMode_Cop",
    "SystemConfMode_Mixed"
};
#endif
typedef struct
{
  APIU32  unSystemConfMode; // eSystemConfMode
  APIU32  unFutureUse; // 0 for now
} CARDS_CONFIG_PARAMS_S;

//2 modes cop/cp
// ====================================
//      UPGRADE_PROGRESS_IND_S
// ====================================
typedef struct
{
    APIU32        progress_precents; // 0-100
} UPGRADE_PROGRESS_IND_S;


// ====================================
//      UPGRADE_IPMC_IND_S
// ====================================
typedef struct
{
    APIU32        require_ipmc_upgrade; // 0-false 1-true
} UPGRADE_IPMC_IND_S;

// ====================================
//      TCP DUMP
// ====================================

typedef enum
{
    eTcpDumpOk = 0,           
    eTcpDumpInternalError = 1,
	eTcpDumpSystemError = 2

}  eTcpDumpStatus;

// internal error reason, see if got eTcpDumpInternalError
// check system errors, see if got eTcpDumpSystemError

typedef enum
{
	eTcpDumpNotRunning = 0,
	eTcpDumpAlreadyRunning = 1,
	eTcpDumpBadTcpDumpString = 2,
	eTcpDumpTimeoutError = 3

}  eTcpDumpInternalErrorReason;


typedef struct
{
    APIS8 sTcpDumpString[TCP_DUMP_PARAM_LEN];

} TCP_DUMP_CONFIG_REQ_S;

typedef struct
{
    eTcpDumpStatus eDumpStatus;
	APIU32 uErrorReason;

} TCP_DUMP_CONFIG_IND_S;

typedef enum
{
    ePacketLossNormal = 0,           
    ePacketLossMajor = 1,
	ePacketLossCritical = 2,
	ePacketLossFecInd = 3

}  eRtcpPacketLossStatus;

typedef struct
{
	APIU32 mediaDirection; 
	UINT32 fractionLossInPercent;
	eRtcpPacketLossStatus ePacketLossStatus;
	kChanneltype	eMediaType;	// Media type: audio or video
	UINT32 		    unSSRC;	    // In case of AV_MCU will identify the correct stream

} RTCP_PACKET_LOSS_STATUS_IND_S;


typedef struct  
{  
	APIU16 listenPortIpV4;
	APIU16 listenPortIpV6;  
} CM_MEDIA_OVER_TCP_LISTEN_PORTS_S; 


#endif // !defined(_CardsStructs_H__)
