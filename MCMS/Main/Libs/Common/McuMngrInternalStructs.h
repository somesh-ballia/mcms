// McuMngrInternalStructs.h

#if !defined(_McuMngrInternalStructs_H__)
#define _McuMngrInternalStructs_H__


#include "CommonStructs.h"
#include "McmsAuthentication.h"
#include "AvcSvcCapStruct.h"
#include "string.h"




// CFS structures

// to CsMngr
struct CSMNGR_LICENSING_S
{
	DWORD  numOfCpPorts;
};

// to RsrcAllocator
typedef struct
{
	DWORD	num_cp_parties;
	DWORD	num_cop_parties;
	DWORD	productType;
	BOOL	federal;
	BOOL    isHD;
	BOOL    isSvcEnabled;
	BOOL	isHdPortsUnit;
	DWORD   num_svc_parties;
	BOOL    isLicenseExpired;
	BOOL    isRPPModeEnabled;
} RSRCALLOC_LICENSING_S;

// to Cards
//Ninja/Gesher  BOARD ID 
enum
{
      CNTL_SLOT_ID = 4
    , RISER_SLOT_ID = 5
    , DSP_CARD_SLOT_ID_0=6	
    , DSP_CARD_SLOT_ID_1=7	
    , DSP_CARD_SLOT_ID_2=8
    , ISDN_CARD_SLOT_ID = 15
    , FANS_SLOT_ID = 21
    , PWRS_SLOT_ID = 22
    , LAN_SLOT_ID_START = 31
    , IPMI_SLOT_ID_RESET = -1
    , IPMI_SLOT_ID_SHUTDOWN = -2
};

typedef struct
{
	MCMS_AUTHENTICATION_S	authenticationStruct;
	BOOL					federal;
	BOOL					multipleServices;
} CARDS_LICENSING_S;


// to ConfParty
typedef struct
{
	BOOL	confPartyLicensing_encryption;
	BOOL	confPartyLicensing_pstn;
	BOOL	confPartyLicensing_telepresence;
	BOOL	confPartyLicensing_ms;
	BOOL	confPartyLicensing_HD;
	BOOL	confPartyLicensing_partner_Avaya;
	BOOL	confPartyLicensing_partner_Ericsson;
	BOOL	confPartyLicensing_partner_Microsoft;
	BOOL	confPartyLicensing_partner_Nortel;
    BOOL	confPartyLicensing_partner_IBM;
    BOOL	federal;
    DWORD	num_cp_parties;
    DWORD	num_cop_parties;
    BOOL	confPartyLicensing_svc;
    BOOL 	confPartyLicensing_CIF_Plus;
    BOOL	confPartyLicensing_TipInterop;
	BOOL    isLicenseExpired;
} CONFPARTY_LICENSING_S;

// to Installer
typedef struct
{
	char		mplSerialNumber[MPL_SERIAL_NUM_LEN];
	DWORD		switchBoardId;
	DWORD		switchSubBoardId;
//	BYTE		cfs_X_keyCode[KEYCODE_LENGTH];
//	BYTE		cfs_U_keyCode[KEYCODE_LENGTH];
	VERSION_S	verFromKeycode;
} INSTALLER_CFS_S;

// to GateKeeper
struct GK_LICENSING_S
{
	BOOL	partner_Avaya;
};

// to ApacheModule
typedef struct
{
	DWORD	num_cop_parties;
	DWORD	num_cp_parties;
	AvcSvcCap avcSvcCap;
} APACHEMODULE_LICENSING_S;

// to Logger
typedef struct
{
	DWORD	num_cop_parties;
	DWORD	num_cp_parties;
	DWORD	sys_card_mode;
} LOGGER_LICENSING_S;

// from RsrcAllocator
typedef struct
{
	DWORD	isRecording;
} RSRCALLOC_MEDIA_RECORDING_S;

// from Collector
typedef struct
{
	DWORD	isCollecting;
} COLLECTOR_COLLECTING_INFO_S;


#define DNS_DOMAIN_NAME_MAX_LEN 128

struct DNS_HOST_REGISTRATION_S
{
	DWORD 	serverStatus;
	BOOL 	isRegistrationAuto;
	char 	domainName	[DNS_DOMAIN_NAME_MAX_LEN];
	char 	hostName	[DNS_DOMAIN_NAME_MAX_LEN];
	
};

typedef struct
{
	DWORD		ipType;
	DWORD		mngmntIpAddress;
	DWORD		defaultGwIpAddress;
	VERSION_S	mcuVer;
} MASTER_SLAVE_DATA_S;


// from Encryption key server
typedef struct
{
	DWORD            result; //values STATUS_OK / STATUS_FAIL
}ENCRYPTIONKEYSERVER_FIPS_140_TEST_INFO_S;

// IpV6 address and mask
struct IpV6AddressMaskS
{
  IpV6AddressMaskS() : address(), mask(0) {}

	void Set(char *inAddr, int inMask)
	{
		strncpy(address, inAddr, sizeof(address)-1);
		mask = inMask;
	}

	char address[IPV6_ADDRESS_LEN];
	int mask;
};
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


	eShmComponentType mediaCompType;
	eShmComponentType rtmCompType;


} SLOTS_CONFIGURATION_S;

#endif // !defined(_McuMngrInternalStructs_H__)
