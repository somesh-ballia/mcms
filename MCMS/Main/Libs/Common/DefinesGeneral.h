
#ifndef      __DefinesGeneral_h__
#define      __DefinesGeneral_h__

#include "ProductType.h"

// ----- Mcu states
enum eMcuState
{
	eMcuState_Invalid = -1,
	eMcuState_Startup,
	eMcuState_Normal,
	eMcuState_Disconnected,
	eMcuState_Major,
	eMcuState_Minor,
	eMcuState_Resetting,
	eMcuState_LowMemory,
	eMcuState_Diagnostics,
	eMcuState_Critical,
    //Any Change in this enumeration should also be reflected
    //In the RMX eDnsConfigurationSuccessMIB FILE
    //Dan Porat
	NUM_OF_MCU_STATES
};



static const char *GetMcuStateName(eMcuState state)
{
    static const char *mcuStateStr[] =
        {
//    "Invalid",  	// eMcuStateInvalid = -1
            "Startup",  	// eMcuState_Startup
            "Normal",   	// eMcuState_Normal
            "Disconnected", // eMcuState_Disconnected
            "Major",    	// eMcuState_Major
            "Minor",    	// eMcuState_Minor
            "Resetting", 	// eMcuState_Resetting
            "Low memory",	// eMcuState_LowMemory
            "Diagnostics", 	// eMcuState_Diagnostics
            "Critical"  	// eMcuState_Critical
        };
    const char *name = (0 <= state && (unsigned int)state < sizeof(mcuStateStr) / sizeof(mcuStateStr[0])
                  ?
                  mcuStateStr[state] : "Invalid");
    return name;
}


enum eDnsConfigurationStatus
{
	eDnsConfigurationSuccess = 0,
	eDnsConfigurationFailure,
	eDnsNotConfigured,

	MAX_NUM_OF_DNS_CONFIG_STATUS
};


// ----- string validity
enum eStringValidityStatus
{
	eStringValid = 0,
    eStringNotNullTerminated,
	eStringInvalidChar,
	eStringIllegalLength,

	NUM_OF_STRING_VALIDITY_STATUSES
};

static const char *StringValidityStatuses[] =
{
    "Valid string",			// eStringValid
    "No NULL termination",  // eStringNotNullTerminated
    "Invalid character",	// eStringInvalid
    "Illegal length",		// eStringIllegalLength
};

#define RAM_SIZE_HALF 512*1024         // 512MB = 0.5GB
#define RAM_SIZE_01GB RAM_SIZE_HALF*2
#define RAM_SIZE_02GB RAM_SIZE_01GB*2
#define RAM_SIZE_03GB RAM_SIZE_01GB*3
#define RAM_SIZE_04GB RAM_SIZE_01GB*4
#define RAM_SIZE_05GB RAM_SIZE_01GB*5
#define RAM_SIZE_06GB RAM_SIZE_01GB*6
#define RAM_SIZE_07GB RAM_SIZE_01GB*7
#define RAM_SIZE_08GB RAM_SIZE_01GB*8
#define RAM_SIZE_09GB RAM_SIZE_01GB*9
#define RAM_SIZE_10GB RAM_SIZE_01GB*10
#define RAM_SIZE_11GB RAM_SIZE_01GB*11
#define RAM_SIZE_12GB RAM_SIZE_01GB*12
#define RAM_SIZE_13GB RAM_SIZE_01GB*13
#define RAM_SIZE_14GB RAM_SIZE_01GB*14
#define RAM_SIZE_15GB RAM_SIZE_01GB*15
#define RAM_SIZE_16GB RAM_SIZE_01GB*16
#define RAM_SIZE_17GB RAM_SIZE_01GB*17
#define RAM_SIZE_18GB RAM_SIZE_01GB*18
#define RAM_SIZE_19GB RAM_SIZE_01GB*19
#define RAM_SIZE_20GB RAM_SIZE_01GB*20
#define RAM_SIZE_21GB RAM_SIZE_01GB*21
#define RAM_SIZE_22GB RAM_SIZE_01GB*22
#define RAM_SIZE_23GB RAM_SIZE_01GB*23
#define RAM_SIZE_24GB RAM_SIZE_01GB*24
#define RAM_SIZE_25GB RAM_SIZE_01GB*25
#define RAM_SIZE_26GB RAM_SIZE_01GB*26
#define RAM_SIZE_27GB RAM_SIZE_01GB*27
#define RAM_SIZE_28GB RAM_SIZE_01GB*28
#define RAM_SIZE_29GB RAM_SIZE_01GB*29
#define RAM_SIZE_30GB RAM_SIZE_01GB*30
#define RAM_SIZE_31GB RAM_SIZE_01GB*31
#define RAM_SIZE_32GB RAM_SIZE_01GB*32

#define MINIMUM_CORES_SIZE 2

#define MAX_CORES_LIMIT 32
#define MAX_CORES_LIMIT_EDGE 30
#define MAX_CORE_SIZE_FACTOR 2
#define MAX_MEMORY_LIMIT 12
#define MAX_MEMORY_FACTOR 1
#define MAX_BOGO_MIPS_LIMIT 5800
#define MAX_BOGO_MIPS_FACTOR 2

enum eSystemRamSize
{
	eSystemRamSize_illegal = 0,
	eSystemRamSize_half,	    // 0.5GB
	eSystemRamSize_full_1,	    // 1GB
	eSystemRamSize_full_2,	    // 2GB
	eSystemRamSize_full_3,	    // 3GB
	eSystemRamSize_full_4,	    // 4GB
	eSystemRamSize_full_5,	    // 5GB
	eSystemRamSize_full_6,	    // 6GB
	eSystemRamSize_full_7,	    // 7GB
	eSystemRamSize_full_8,	    // 8GB
	eSystemRamSize_full_9,	    // 9GB
	eSystemRamSize_full_10,	    // 10GB
	eSystemRamSize_full_11,	    // 11GB
	eSystemRamSize_full_12,	    // 12GB
	eSystemRamSize_full_13,	    // 13GB
	eSystemRamSize_full_14,	    // 14GB
	eSystemRamSize_full_15,	    // 15GB
	eSystemRamSize_full_16,	    // 16GB
	eSystemRamSize_full_17,	    // 17GB
	eSystemRamSize_full_18,	    // 18GB
	eSystemRamSize_full_19,	    // 19GB
	eSystemRamSize_full_20,	    // 20GB
	eSystemRamSize_full_21,	    // 21GB
	eSystemRamSize_full_22,	    // 22GB
	eSystemRamSize_full_23,	    // 23GB
	eSystemRamSize_full_24,	    // 24GB
	eSystemRamSize_full_25,	    // 25GB
	eSystemRamSize_full_26,	    // 26GB
	eSystemRamSize_full_27,	    // 27GB
	eSystemRamSize_full_28,	    // 28GB
	eSystemRamSize_full_29,	    // 29GB
	eSystemRamSize_full_30,	    // 30GB
	eSystemRamSize_full_31,	    // 31GB
	eSystemRamSize_full_32,	    // 32GB
	NUM_OF_SYSTEM_RAM_SIZES
};

enum eSystemCPUSize
{
	eSystemCPUSize_illegal = 0,
	eSystemCPUSize_2 = 2,
	eSystemCPUSize_4 = 4,
	eSystemCPUSize_8 = 8,
	eSystemCPUSize_12 = 12,
	eSystemCPUSize_16 = 16,
	eSystemCPUSize_24 = 24,
	eSystemCPUSize_32 = 32,

	NUM_OF_SYSTEM_CPU_SIZES=6
};

static const char *sSystemRamSizes[] =
{
		"illegal size",		// eSystemRamSize_illegal = 0,
		"512 MB",			//	eSystemRamSize_half,	    // 0.5GB
		"1024 MB",			//	eSystemRamSize_full_1,	    // 1GB
		"2048 MB",			//	eSystemRamSize_full_2,	    // 2GB
		"3072 MB",			//	eSystemRamSize_full_3,	    // 3GB
		"4096 MB",			//	eSystemRamSize_full_4,	    // 4GB
		"5120 MB",			//	eSystemRamSize_full_5,	    // 5GB
		"6144 MB", 			//  eSystemRamSize_full_6
		"7168 MB", 			//  eSystemRamSize_full_7,	    // 7GB
		"8192 MB", 			//	eSystemRamSize_full_8,	    // 8GB
		"9216 MB", 			//	eSystemRamSize_full_9,	    // 9GB
		"10240 MB", 		//	eSystemRamSize_full_10,	    // 10GB
		"11264 MB", 		//	eSystemRamSize_full_11,	    // 11GB
		"12288 MB", 		//	eSystemRamSize_full_12,	    // 12GB
		"13312 MB", 		//	eSystemRamSize_full_13,	    // 13GB
		"14336 MB", 		//	eSystemRamSize_full_14,	    // 14GB
		"15360 MB", 		//	eSystemRamSize_full_15,	    // 15GB
		"16348 MB", 		//	eSystemRamSize_full_16,	    // 16GB
		"17408 MB", 		//	eSystemRamSize_full_17,	    // 17GB
		"18432 MB", 		//	eSystemRamSize_full_18,	    // 18GB
		"19456 MB", 		//	eSystemRamSize_full_19,	    // 19GB
		"20480 MB", 		//	eSystemRamSize_full_20,	    // 20GB
		"21504 MB", 		//	eSystemRamSize_full_21,	    // 21GB
		"22528 MB", 		//	eSystemRamSize_full_22,	    // 22GB
		"23552 MB", 		//	eSystemRamSize_full_23,	    // 23GB
		"24576 MB", 		//	eSystemRamSize_full_24,	    // 24GB
		"25600 MB", 		//	eSystemRamSize_full_25,	    // 25GB
		"26624 MB", 		//	eSystemRamSize_full_26,	    // 26GB
		"27648 MB", 		//	eSystemRamSize_full_27,	    // 27GB
		"28672 MB", 		//	eSystemRamSize_full_28,	    // 28GB
		"29696 MB", 		//	eSystemRamSize_full_29,	    // 29GB
		"30720 MB", 		//	eSystemRamSize_full_30,	    // 30GB
		"31744 MB", 		//	eSystemRamSize_full_31,	    // 31GB
		"32768 MB", 		//	eSystemRamSize_full_32,	    // 32GB
};

enum eLicenseMode
{
	eLicenseMode_none = 0,
	eLicenseMode_flexera,
	eLicenseMode_cfs
};

static const char *sLicenseMode[] =
{
		"NONE",
		"FLEXERA",
		"CFS"
};



enum eBackupProgressType
{
	eBackup_Success = 0,
	eBackup_InProgress,
	eBackup_Failure,
	eBackup_FailureTimeout,
	eBackup_FailureTar,
	eBackup_FailureEncrypt,
	eBackup_Idle,

	eBackup_EnumNum,
};

enum eRestoreProgressType
{
	eRestore_Success = 0,
	eRestore_InProgress,
	eRestore_Failure,
	eRestore_FailureTimeout,
	eRestore_FailureUntar,
	eRestore_FailureDecrypt,

	eRestore_EnumNum,
};

enum eBackupRestoreAction
{
	eBRIdle =	0,
	eBackup,
	eRestore
};

static const char * strBackupRestoreAction[] =
{
	"Idle",
	"Backup",
	"Restore"
};

// to FPGA Upgrade
enum eFPGAUpgradeAction
{
	eFPGAErase =	0,
	eFPGAUpgrade,
	eFPGAReadBack,
	eFPGAActionMaxNum
};

enum eConfigInterfaceType
{
	eManagmentNetwork			 = 0,	// in Rmx2000: eth0:1		in Rmx4000: eth1
	eSeparatedManagmentNetwork 	 = 1,	// in Rmx2000: eth0.2197
	eInternalNetwork 		   	 = 2,	// in Rmx2000: eth0.2093	in Rmx4000: eth0.2093
	eSignalingNetwork 		   	 = 3,	// in Rmx2000: eth0:2		in Rmx4000: eth2
	eSeparatedSignalingNetwork 	 = 4, 	// in Rmx2000: eth0.2198

	eSeparatedSignalingNetwork_1_1 = 5, 	//These interfaces use vlans eth0.2012 - eth0.2043,
	eSeparatedSignalingNetwork_1_2 = 6,	//depending on how many services run,
	eSeparatedSignalingNetwork_2_1 = 7,	//and which media cards they are assigned to.
	eSeparatedSignalingNetwork_2_2 = 8,
	eSeparatedSignalingNetwork_3_1 = 9,
	eSeparatedSignalingNetwork_3_2 = 10,
	eSeparatedSignalingNetwork_4_1 = 11,
	eSeparatedSignalingNetwork_4_2 = 12,

	ePermanentNetwork		       = 13, 	// in Rmx2000: eth0.2097	in Rmx4000: eth0.2097

	eSoftMcuEth0Network          = 14,
	eSoftMcuEth1Network          = 15,
	eSoftMcuEth2Network          = 16,
	eSoftMcuEth3Network          = 17,

	eMultipleSignalingNetwork_1	   = 18,    // Only For Gesher
	eMultipleSignalingNetwork_2    = 19,
	eMultipleSignalingNetwork_3    = 20,
	eLanRedundancyManagementNetwork = 21,
	eLanRedundancySignalingNetwork = 22,

	NumOfConfigInterfaceTypes
};

static char* sConfigInterfaceTypeName[] =
{
	"Managment",
	"Managment",
	"Internal",
	"Signaling",
	"Signaling",
	"Signaling",
	"Signaling",
	"Signaling",
	"Signaling",
	"Signaling",
	"Signaling",
	"Signaling",
	"Signaling",
	"Permanent",
	"SoftMcuEth0",
	"SoftMcuEth1",
	"SoftMcuEth2",
	"SoftMcuEth3"
	"Permanent",
	"Signaling",
	"Signaling",
	"Signaling",
	"Managment",
	"Signaling"
};

static const char* GetConfigInterfaceTypeName(eConfigInterfaceType ifType)
{
	const char *name = (0 <= ifType && ifType < NumOfConfigInterfaceTypes
						?
						sConfigInterfaceTypeName[ifType] : "Invalid");
	return name;
}

enum eConfigDeviceName
{
	eEth_0 = 0,
	eEth_1,
	eEth_2,
	eEth_3, //added for Gesher
	bond_0,
	bond_0_1,
	bond_0_2,

	NumOfConfigDeviceName
};

static char* sConfigDeviceName[] =
{
	"eth0",
	"eth1",
	"eth2",
	"eth3",
	"bond0",
	"bond0:1",
	"bond0:2"

};

static const char* GetConfigDeviceName(eConfigDeviceName ifName)
{
	const char *name = (0 <= ifName && ifName < NumOfConfigDeviceName
						?
								sConfigDeviceName[ifName] : "Invalid");
	return name;
}

enum eConfigInterfaceNum
{
	eEth0 = 0,
	eEth1,
	eEth2,
	eEth0_alias_1,
	eEth0_alias_2,
	eEth0_2093,
	eEth0_2097,
	eEth0_2197,
	eEth0_2198,
	eEth0_2012,
	eEth0_2013,
	eEth0_2022,
	eEth0_2023,
	eEth0_2032,
	eEth0_2033,
	eEth0_2042,
	eEth0_2043,
	eEth3,
	eBond0_alias_1,
	eBond0_alias_2,
	NumOfConfigInterfaceNums
};

static char* sConfigInterfaceNum[] =
{
	"eth0",
	"eth1",
	"eth2",
	"eth0:1",
	"eth0:2",
	"eth0.2093",
	"eth0.2097",
	"eth0.2197",
	"eth0.2198",
	"eth0.2012",
	"eth0.2013",
	"eth0.2022",
	"eth0.2023",
	"eth0.2032",
	"eth0.2033",
	"eth0.2042",
	"eth0.2043",
	"eth3",
	"bond0:1",
	"bond0:2"
};

#ifndef COMPILE_TIME_ASSERT
#define COMPILE_TIME_ASSERT(expn) typedef char __C_ASSERT__[(expn)?1:-1]
#endif

//Compile time verification - Number of elements defined for eConfigInterfaceNum == Number of elements in sConfigInterfaceNum
COMPILE_TIME_ASSERT(NumOfConfigInterfaceNums == sizeof(sConfigInterfaceNum)/sizeof(char*));

//Compile time verification - Number of elements defined for eConfigDeviceName == Number of elements in sConfigDeviceName
COMPILE_TIME_ASSERT(NumOfConfigDeviceName == sizeof(sConfigDeviceName)/sizeof(char*));

//Compile time verification - Number of elements defined for eConfigInterfaceType == Number of elements in sConfigInterfaceTypeName
COMPILE_TIME_ASSERT(NumOfConfigInterfaceTypes == sizeof(sConfigInterfaceTypeName)/sizeof(char*));




static const char* GetConfigInterfaceNumName(eConfigInterfaceNum ifNum)
{
	const char *name = (0 <= ifNum && ifNum < (NumOfConfigInterfaceNums )
						?
						sConfigInterfaceNum[ifNum] : "Invalid");
	return name;
}

// factory defaults flag for system reset
#define DEFAULT_RESTORE_FILE 			"States/restore_factory_defaults.flg"
#define DEFAULT_RESTORE_USING_AdministrationTools_FILE 			\
	"States/restore_factory_defaults_using_administration_tools.flg"

enum eShmComponentType // ShelfManager's components
{
	eShmComp_Illegal			= -1,
	eShmComp_SwitchBoard		=  0,
	eShmComp_MfaBoard1			=  1,
	eShmComp_MfaBoard2			=  2,
	eShmComp_MfaBoard3			=  3,
	eShmComp_MfaBoard4			=  4,
	eShmComp_MfaBoard5			=  5,
	eShmComp_MfaBoard6			=  6,
	eShmComp_MfaBoard7			=  7,
	eShmComp_MfaBoard8			=  8,
	eShmComp_MfaMpmx			=  9,
	eShmComp_MfaMpmRx			=  10,
	eShmComp_RtmIsdn			=  11,
	eShmComp_RtmIsdn9			=  12,
	eShmComp_RtmIsdn9_10G			=  13,
	eShmComp_McmsCpu            =  14,
	eShmComp_Fan                =  15,
	eShmComp_PowerSupply        =  16,
	eShmComp_Lan                =  17,
	eShmComp_Backplane          =  18,
	eShmComp_Iam                =  19,	// (2nd CPU board)
	eShmComp_Fsm4000            =  20,	// Shoval
	eShmComp_RtmLan				    =  21,
	eShmComp_RtmLan4				=  22,
	eShmComp_RtmLan4_10G		    =  23,

	eMaxNumOfShmComponents
};

enum eMasterSlaveState
{
	eMasterSlaveNone			= 0,
	eMasterConfigurationState,
	eSlaveState,
	eMasterActualState,

	NUM_OF_MASTER_SLAVE_STATES
};

static const char *sMasterSlaveState[] =
{
	"none",					// eMasterSlaveNone
	"master configuration",  // eMasterConfigurationState
	"slave",				// eSlaveState
	"master actual",		// eMasterActualState,
};

enum eFailoverEventTriggerType
{
	eFailoverMgtPortFailure =1,
	eFailoverMpmCardFailure,
	eFailoverSignalPortFailure,
	eFailoverIsdnCardFailure,
	eFailoverSwitchCardFailure
};


/*
enum eConfigInterfaceType
{
	eManagmentNetwork	= 0,				// in Rmx2000:  eth0:1		in Rmx2000: eth1
	eSeparatedManagmentNetwork, 			// in Rmx2000c: eth0.2198:1
	eInternalNetwork,   					// in Rmx2000:  eth0.2093	in Rmx4000: eth0.2093
	eSignalingNetwork,						// in Rmx2000:  eth0:2		in Rmx4000: eth2
	eSeparatedPrimarySignalingNetwork,		// in Rmx2000c: eth0.2198:2
	eSeparatedSecondarySignalingNetwork,	// in Rmx2000c: eth0.2199
	ePermanentNetwork,						// in Rmx2000:  eth0.2097	in Rmx4000: eth0.2097

	NumOfConfigInterfaceTypes
};

static char* sConfigInterfaceType[] =
{
	"Managment",
	"Managment",
	"Internal",
	"Signaling",
	"Signaling",
	"Signaling",
	"Permanent"
};

static const char* GetConfigInterfaceTypeName(eConfigInterfaceType ifType)
{
	const char *name = (0 <= ifType && ifType < NumOfConfigInterfaceTypes
						?
						sConfigInterfaceType[ifType] : "Invalid");
	return name;
}
*/

#define GET_MCU_HOME_DIR (getenv("MCU_HOME_DIR") == NULL ? "" : getenv("MCU_HOME_DIR"))

#define MCU_TMP_DIR ((std::string)GET_MCU_HOME_DIR+"/tmp")

#define MCU_MCMS_DIR ((std::string)GET_MCU_HOME_DIR+"/mcms")

#define MCU_OUTPUT_DIR ((std::string)GET_MCU_HOME_DIR+"/output")

#define MCU_OUTPUT_TMP_DIR ((std::string)GET_MCU_HOME_DIR+"/output/tmp")

#define MCU_CONFIG_DIR ((std::string)GET_MCU_HOME_DIR+"/config")

#define MCU_CONFIG_MCMS_DIR ((std::string)GET_MCU_HOME_DIR+"/config/mcms")

#define MCU_CONFIG_EMA_DIR ((std::string)GET_MCU_HOME_DIR+"/config/ema")

#define MCU_CONFIG_LINKS_DIR ((std::string)GET_MCU_HOME_DIR+"/config/Links")

#define MCU_DATA_DIR ((std::string)GET_MCU_HOME_DIR+"/data")

#define MCU_VAR_DIR ((std::string)GET_MCU_HOME_DIR+"/var")

#define MCU_MRMX_DIR ((std::string)GET_MCU_HOME_DIR+"/usr/rmx1000")

#define MCU_MRMX_ETC_DIR ((std::string)GET_MCU_HOME_DIR+"/etc/rmx1000")

#define MCU_CS_DIR ((std::string)GET_MCU_HOME_DIR+"/cs")

#define MCU_MFA_DIR ((std::string)GET_MCU_HOME_DIR+"/usr/share/MFA")

#define MCU_MRM_DIR ((std::string)GET_MCU_HOME_DIR+"/usr/share/EngineMRM")

#define MCU_UTILS_DIR ((std::string)GET_MCU_HOME_DIR+"/usr/Plcm-Utils/")

#define MCU_APACHE_DIR ((std::string)GET_MCU_HOME_DIR+"/usr/local/apache2")


// System Versions
#define VERSIONS_FILE_PATH  ((std::string)(MCU_MCMS_DIR+"/Versions.xml"))

// logs of startup path
#define OS_STARTUP_LOGS_PATH		((std::string)(MCU_TMP_DIR+"/startup_logs/"))

#define  FCLOSE_SUCCESS		0 // return value of fclose() upon success

// Move to general place in code!!!
#define  MAX_STATUS_IN_LIST  10
#define  MAX_UNITS_IN_LIST   10

// AmosCapacity constants
#define  MAX_CONF_IN_LIST    800

#define MAX_NUM_OF_COMPONENTS            	20

#define SIMULATION_NO   0
#define SIMULATION_YES  1

#define  MCU_STARTUP_STATE      0
#define  MCU_NORMAL_STATE       1
#define  MCU_DISCONNECTED_STATE 2  //Applicable only for operator ws. Not within MCMS.
#define  MCU_MAJOR_STATE        3
#define  MCU_MINOR_STATE        4
#define  MCU_RESETTING_STATE    5
#define  MCU_LOW_MEMORY_STATE   6
#define  MCU_DIAGNOSTICS_STATE  7

// IpService
#define REG_ON_GOING          0x01
#define REG_MEETING_ROOMS     0x02
#define REG_ENTRY_QUEUES      0x04
#define REG_FACTORIES      	  0x08
#define REG_GW_PROFILES    	  0x10

#define MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES  3

#define CS_NETWORK_INTERFACE_SEPERATED_NUMBER 2198

#define DEFAULT_IPV6_SUBNET_MASK              64

#define DEFAULT_ICE_STUN_TURN_SERVER_PORT     3478
#define DEFAULT_ICE_STUN_TURN_SERVER_PORT_STR "3478"


// Extrenal DB application - returned status
#define EXT_APP_STATUS_OK						     0
#define EXT_APP_ILLEGAL_MCU_USER_NAME_OR_PASSWORD    1
#define EXT_APP_REQUEST_TIMEOUT			             2
#define EXT_APP_ILLEGAL_NUMERIC_ID					 3
#define EXT_APP_ILLEGAL_CLI					         4
#define EXT_APP_ILLEGAL_CLI_OR_NUMERIC_ID	         5
#define EXT_APP_INTERNAL_ERROR				         6
#define EXT_APP_ILLEGAL_USER_ID				         7
#define EXT_APP_ILLEGAL_USER_NAME_OR_PASSWORD		 8

#define CFS_OPTIONS_BITMASK_LENGTH					22


// Cards
#define FIRST_SLOT_NUMBER					1
#define SECOND_SLOT_NUMBER					2

#define CARD_STARTUP_TIME_LIMIT						150*SECOND //increase from 2 to 2.5 minutes cause in MPMx cards startup we receive active alarm MPMstartup incomplete

#define ICE_INIT_TIME_LIMIT					25*SECOND		//timeout after 25 seconds


// McuMngr/BackupRestore timeout
#define BACKUP_RESTORE_TIMEOUT						5*60*SECOND
#define	TIMEOUT_INSTALATION  						30*60*SECOND

#define FIXED_BOARD_ID_MEDIA_1			1
#define FIXED_BOARD_ID_MEDIA_2			2
#define FIXED_BOARD_ID_MEDIA_3			3
#define FIXED_BOARD_ID_MEDIA_4			4
#define FIXED_BOARD_ID_SWITCH			5
#define FIXED_BOARD_ID_RTM_1			13
#define FIXED_BOARD_ID_RTM_2			14
#define FIXED_BOARD_ID_RTM_3			15
#define FIXED_BOARD_ID_RTM_4			16
#define FIXED_BOARD_ID_CPU_1			9
#define FIXED_BOARD_ID_CPU_2			7
#define FIXED_DISPLAY_BOARD_ID_SWITCH			17
#define FIXED_BOARD_ID_POLYSTAR			18

#define FIXED_BOARD_ID_MAIN_SOFTMCU		1
#define ETH_SETTINGS_ETH0_PORT_IDX_SOFTMCU	1
#define ETH_SETTINGS_ETH1_PORT_IDX_SOFTMCU	2
#define ETH_SETTINGS_ETH2_PORT_IDX_SOFTMCU	3
#define ETH_SETTINGS_ETH3_PORT_IDX_SOFTMCU	4

#define FIXED_BOARD_ID_SWITCH_SIM		0
#define FIXED_BOARD_ID_MEDIA_1_SIM		1
#define FIXED_BOARD_ID_MEDIA_2_SIM		2
#define FIXED_BOARD_ID_MEDIA_3_SIM		3
#define FIXED_BOARD_ID_MEDIA_4_SIM		4

//for gesher/ninja
#define MULTIPLE_SERVICE_GESHER_FAKE_BOARD_ID        30
#define MULTIPLE_SERVICE_NINJA_FAKE_BOARD_ID         31
#define MAX_NUMBER_OF_IPSERVICES_GESHER              3
#define MAX_NUMBER_OF_IPSERVICES_NINJA               2
//end

#define MFA_SUBBOARD_ID					FIXED_CM_SUBBOARD_ID
#define SWITCH_SUBBOARD_ID			    1
#define RTM_ISDN_SUBBOARD_ID			2

#define ETH_SETTINGS_PORT_MANAGEMENT	2
#define ETH_SETTINGS_PORT_SIGNALING		3
#define ETH_SETTINGS_PORT_PERMANENT		1

#define ETH_SETTINGS_MEDIA_1_PORT_ON_MEDIA_BOARD		1
#define ETH_SETTINGS_MEDIA_2_PORT_ON_MEDIA_BOARD		2
#define ETH_SETTINGS_MEDIA_3_PORT_ON_MEDIA_BOARD		3
#define ETH_SETTINGS_MEDIA_4_PORT_ON_MEDIA_BOARD		4
#define ETH_SETTINGS_MEDIA_SIGNLG_PORT_1_ON_BOARD		1
#define ETH_SETTINGS_MEDIA_SIGNLG_PORT_2_ON_BOARD		2
#define ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD			1
#define ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD_RMX2000			3
#define ETH_SETTINGS_NOT_USED_PORT_ON_SWITCH_BOARD_RMX2000	    1
#define ETH_SETTINGS_CPU_MNGMNT_1_PORT_ON_SWITCH_BOARD	2
#define ETH_SETTINGS_CPU_SGNLNG_1_PORT_ON_SWITCH_BOARD	3
#define ETH_SETTINGS_CPU_MNGMNT_2_PORT_ON_SWITCH_BOARD	4
#define ETH_SETTINGS_CPU_SGNLNG_2_PORT_ON_SWITCH_BOARD	5
#define ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD	6
#define ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX2000	2

#define ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500	        3
#define ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500	        4
#define ETH_SETTINGS_SHELF_MNGR_PORT_ON_SWITCH_BOARD_RMX1500	        5
#define ETH_SETTINGS_MODEM_PORT_ON_SWITCH_BOARD_RMX1500			        8

// actually not all MAX_NUM_OF_LAN_PORTS are effective (only: one on each MPM + six on Switch)
#define NUM_OF_EFFECTIVE_LAN_PORTS		15
#define NUM_OF_LAN_PORTS_IN_YONA		6

#define ETH_SETTINGS_CPU_SIGNALLING_PORT_IDX_RMX1500	        3
#define ETH_SETTINGS_CPU_SIGNALLING_1_PORT_IDX_RMX4000	        10
#define ETH_SETTINGS_CPU_SIGNALLING_2_PORT_IDX_RMX4000	        12

#define ETH_SETTINGS_SHELF_PORT_IDX_RMX2000                     5
#define ETH_SETTINGS_MODEM_PORT_IDX_RMX2000                     4

#define MEDIA_RECORDING_MAX_FOLDER_SIZE 1572864 //1.5 * 1024 * 1024; // 1.5 Giga

static eConfigInterfaceType ConvertEthPortToConfigInterfaceType(int ethPort,eProductType productType)
{
 switch (productType)
 {
	case eProductTypeRMX4000:
	{
	  switch (ethPort)
	  {
	  case ETH_SETTINGS_PORT_MANAGEMENT:
		  return eManagmentNetwork;

	  case ETH_SETTINGS_PORT_SIGNALING:
		  return eSignalingNetwork;

	  case ETH_SETTINGS_PORT_PERMANENT:
	  default:
	    	return ePermanentNetwork;
	  }

	}
     break;
	case eProductTypeRMX1500:
	{
	  switch (ethPort)
	  {
	  case ETH_SETTINGS_CPU_MNGMNTB_PORT_ON_SWITCH_BOARD_RMX1500:
		  return eManagmentNetwork;

	  case ETH_SETTINGS_CPU_MNGMNT_PORT_ON_SWITCH_BOARD_RMX1500:
		  return eSignalingNetwork;

	  case ETH_SETTINGS_PORT_PERMANENT:
	  default:
	    	return ePermanentNetwork;
	  }

	}
	break;

	case eProductTypeGesher:
	{
	  switch (ethPort)
	  {
	  case ETH_SETTINGS_ETH0_PORT_IDX_SOFTMCU:
		  return eSoftMcuEth0Network;

	  case ETH_SETTINGS_ETH1_PORT_IDX_SOFTMCU:
		  return eSoftMcuEth1Network;

	  case ETH_SETTINGS_ETH2_PORT_IDX_SOFTMCU:
		  return eSoftMcuEth2Network;

	  case ETH_SETTINGS_ETH3_PORT_IDX_SOFTMCU:
		  return eSoftMcuEth3Network;

	  default:
	    	return eSoftMcuEth0Network;
	  }
	}
	break;

	case eProductTypeNinja:
	{
	  switch (ethPort)
	  {
	  case ETH_SETTINGS_ETH0_PORT_IDX_SOFTMCU:
		  return eSoftMcuEth0Network;

	  case ETH_SETTINGS_ETH1_PORT_IDX_SOFTMCU:
		  return eSoftMcuEth1Network;

	  default:
	    	return eSoftMcuEth0Network;
	  }
	}
	break;

	 default:
		    	return ePermanentNetwork;

	}
}

enum eLdapDirType
{
	eMsActiveDirectory = 0,

	NumofLdapDirTypes
};

enum eLdapDirPort
{
	e389 = 0,
	e636 = 1,

	NumofLdapDirPorts
};

enum eLdapAuthenticationType
{
	ePlain = 0,
	eNTLM = 1,
	eKerberos = 2,	//currently not in use

	NumofLdapAthenticationTypes
};


enum ePrecedenceLevelType
{
	eRoutine = 0,
	ePriority = 1,
	eImmediate = 2,
	eFlash = 3,
	eFlashOverride = 4,
	eFlashOverridePlus = 5,

	NumofPrecedenceLevelTypes
};


enum eApiFormat
{
	eXmlApi = 0,
	eRestApi = 1
};


enum eAuthenticationProtocol
{
	eNone = 0,
    eMD5,
    eSHA1,
    eAuto,
    eAES,
    eSHA256
};

enum eConfBlockReason
{
    eConfBlockReason_McuMngr_Position_1 = 0,
    eConfBlockReason_McuMngr_InvalidCertificate,
    eConfBlockReason_CSMngr_Position_1,
    eConfBlockReason_CSMngr_Position_2,
    eConfBlockReason_Installer_Position_1,

    NumOfConfBlockReasons
};

enum eRevocationMethod
{
	eNoneMethod = 0,
	eCrl,
	eOcsp

};

//NINJA: Rescue Card
typedef struct
{
    unsigned int  boardID;
    unsigned int  unitID;
} RESCUE_CARD_REQ_S;


enum eNetConfigurationStatus
{
	eNetConfigurationSuccess = 0,
	eNetConfigurationFailureNoAction,
	eNetConfigurationFailureActionChangeMngntIp4Type,
	MAX_NUM_OF_NET_CONFIG_STATUS
};

//comment tls1.1 and tls1.2 need to be disabled untill EMA support dotnet 4.5 to support tls1.1 and tls1.2
enum eMangmentSecurityProtocol
{
	eTLSV1_SSLV3 = 0,
	eTLSV1,
	//eTLSV1_1,
	//eTLSV1_2,
	//eTLSV1_2_TLSV1_1,
	//eTLS1_2_TLSV1_1_TLSV1,
	//eTLS1_2_TLSV1_1_TLSV1_SSLV3,
	MAX_NUM_SECURITY_PROTOCOL
};

// for Linux system services
enum ServiceCommand
{
	eCmdStart,
	eCmdStop,
	eCmdRestart
};


typedef enum
{
    DOWNLOAD_FILE_FAILED
    , DOWNLOAD_FILE_OK
    , DOWNLOAD_FILE_NOT_MODIFIED
}DownloadFileResult;


#endif // __DefinesGeneral_h__
