// McuMngrStructs.h
//
//////////////////////////////////////////////////////////////////////
#if !defined(_McuMngrStructs_H__)
#define _McuMngrStructs_H__


#include "DataTypes.h"
#include "CommonStructs.h"



//////////////////////////////
//   CONSTANTS DEFINITIONS
//////////////////////////////
#define URL_LOCATION_LEN			256
#define NTP_MAX_NUM_OF_SERVERS		3
#define KEYCODE_LENGTH				128



//////////////////////////////
//       ENUMERATORS
//////////////////////////////

// =================================
// ========= ePlatformType =========
// =================================
enum ePlatformType
{
	eGideonLite				= 0,
	eGideon5				= 1,
	eGideon14				= 2,
	eAmos					= 3,
	eYona                   = 4,
	eSoftMcu				= 5,

	NUM_OF_PLATFORM_TYPES = 6,  //DONT FORGET TO UPDATE THIS
	ePlatformTypeUndefined	= NUM_OF_PLATFORM_TYPES
};

// =================================
// =========== eUrlType ============
// =================================
enum eUrlType
{
	eFtp              = 0,
	eNfs              = 1, // 'samba'??
	
	NUM_OF_URL_TYPES  = 2      //DONT FORGET TO UPDATE THIS
};


// =================================
// ======== eMcuRestoreType ========
// =================================
enum eMcuRestoreType
{
    eMcuRestoreNone = 0,
    eMcuRestoreStandard,
    eMcuRestoreInhensive,
	eMcuRestoreBasic,

    NumOfMcuRestoreTypes
};

enum eMcuRestoreStatus
{
	eMcuRestoreStatusFailure   = 0,
	eMcuRestoreStatusSuccess   = 1

};

enum eMcuShutDownType
{
    eMcuShutDownTypeNone = 0,
    eMcuShutDownTypeHardReset = 1,   // the value is 1 on purpose dont change it , Hard reset == cold reset == disconnecting the power from the MCU
	eMcuShutDownTypeOther,
    NumOfMcuShutDownTypes
};
//////////////////////////////
//    API STRUCTURES
//////////////////////////////

typedef struct
{
	APIU8      serialNum[MPL_SERIAL_NUM_LEN];
	APIU32     platformType;
	VERSION_S  mcuVersion;    // for comparing mcuVersion of MPL to mcuVersion of Mcms
	APIU8      cfs_X_KeyCode[KEYCODE_LENGTH];
	APIU8      cfs_U_KeyCode[KEYCODE_LENGTH];
	VERSION_S  chassisVersion;
	VERSION_S  keyCodeVersion;
	APIU32     lastShutDownType;
	APIU8      isNewCtrlGeneration;
} MPL_AUTHENTICATION_S;


typedef struct
{
	// url (name or ip, and location):
	APIU8   hostName[NAME_LEN];
	APIU32  hostIp;
	APIU8   location[URL_LOCATION_LEN];

	APIU32  urlType;
	APIU8   userName[NAME_LEN];
	APIU8   password[NAME_LEN];
	APIU32  vLanId;
} MPL_SW_LOCATION_S;


typedef struct
{

	APIU32      mcuRestoreType;
	APIU32      mcuRestoreStatus;              //this can be 0-FAILURE or 1-SUCCESS
}MPL_RESTORE_DEFAULT_S;
typedef struct
{
    APIU32  unNtpYesNo;       // 0 no - 1 yes
    APIU32  aunServerIp[NTP_MAX_NUM_OF_SERVERS];   // 0xac16b812
    APIU32  unSampleInterval; // in seconds, power of 2, min 16. if 0 then use default.
    APIU32  unOffset;         // in minute from gmt time
    APIU32  unOffsetSign;   
    APIU32  unTimeSec;    //0..59
    APIU32  unTimeMin;    //0..59
    APIU32  unTimeHour;    //0..23
    APIU32  unTimeDay;    //1 ..31
    APIU32  unTimeMonth;  //0 ..11
    APIU32  unTimeYear;   //since 1900
	//this value only hasaz meaning in case we use external ntp servers
	//as our time sync.	
	APIU32  ulNtpDscp;
	//this is an array of size 3 of 3 strings of 512 bytes each. each
	//string holds a single IPv6 addr in ascii format
	APIU8   ucIpV6AddressesArr[NTP_MAX_NUM_OF_SERVERS][IPV6_ADDRESS_LEN];
} NTP_REQUEST_S;

typedef struct
{
	//this value only hasaz meaning in case we use external ntp servers
	//as our time sync.	
	APIU32  ulNtpDscp;

} NTP_DSCP_REQUEST_S;


typedef struct
{
	APIU8   keycode[KEYCODE_LENGTH];
	VERSION_S  keyCodeVersion;
} CFS_KEYCODE_S;


#endif // !defined(_McuMngrStructs_H__)
