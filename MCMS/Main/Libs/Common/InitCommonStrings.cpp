#include "InitCommonStrings.h"

#include <limits>

#include "StringsMaps.h"
#include "StringsLen.h"
#include "ApacheDefines.h"
#include "CsStructs.h"
#include "DefinesGeneral.h"
#include "DefinesIpService.h"
#include "StringsMaps.h"
#include "FaultsDefines.h"
#include "GKManagerOpcodes.h"
#include "ConfPartyApiDefines.h"
#include "H221.h"
#include "H263.h"
#include "Capabilities.h"
#include "DefinesIpServiceStrings.h"
#include "InterfaceType.h"
#include "ConfPartySharedDefines.h"
#include "McmsProcesses.h"
#include "CDREvent.h"
#include "McuMngrStructs.h"
#include "IpService.h"
#include "LoggerDefines.h"
#include "Q931Structs.h"
#include "AuditInt2Str.h"
#include "AuditorApi.h"
#include "RtmIsdnMngrCommonMethods.h"
#include "VideoStructs.h"



// colinzuo: we can modify ApiCom to add a new mode for meridian if needed, but for now I don't think we need
#ifndef eSystemCardsMode_meridian
#define eSystemCardsMode_meridian NUM_OF_SYSTEM_CARDS_MODES
#endif

extern const char* ProcessTypeToString(eProcessType processType);
extern const char* PortSpeedTypeToString(ePortSpeedType type);
extern const char* GetSystemCardsModeStr(eSystemCardsMode theMode);
extern char* CardTypeToString(APIU32 cardType);
extern char* IpTypeToString(APIU32 ipType, bool caps = false);
extern char* IpV6ConfigurationTypeToString(APIU32 v6Type, bool caps = false);

static void InitFaultStrings();
static void InitCDRStrings();

void InitCommonStrings()
{
	CStringsMaps::AddMinMaxItem(IP_STRING_LENGTH,0,IP_STRING_LEN-1);
	CStringsMaps::AddMinMaxItem(DESCRIPTION_LENGTH,0,DESCRIPTION_LEN - 1);
	CStringsMaps::AddMinMaxItem(ONE_LINE_BUFFER_LENGTH,0,ONE_LINE_BUFFER_LEN - 1);
	CStringsMaps::AddMinMaxItem(FIFTY_LINE_BUFFER_LENGTH,0,FIFTY_LINE_BUFFER_LEN - 1);
	CStringsMaps::AddMinMaxItem(TEN_LINE_BUFFER_LENGTH,0,TEN_LINE_BUFFER_LEN - 1);
	CStringsMaps::AddMinMaxItem(TWO_HUNDRED_LINE_BUFFER_LENGTH,0,(FIFTY_LINE_BUFFER_LEN*4) - 1);
	CStringsMaps::AddMinMaxItem(UNLIMITED_CHAR_LENGTH,0,std::numeric_limits<int>::max());

	CStringsMaps::AddItem(_BOOL,FALSE,"false");
	CStringsMaps::AddItem(_BOOL,1,"true");

	CStringsMaps::AddItem(_YES_NO, 0, "NO");
	CStringsMaps::AddItem(_YES_NO, 1, "YES");

	CStringsMaps::AddMinMaxItem(_0_TO_BYTE,0,0xFF);
	CStringsMaps::AddMinMaxItem(_0_TO_WORD,0,0xFFFF);
	CStringsMaps::AddMinMaxItem(_0_TO_DWORD,0,0xFFFFFFFF);
	CStringsMaps::AddMinMaxItem(_1_TO_DWORD,1,0xFFFFFFFF);

	CStringsMaps::AddMinMaxItem(_1_TO_IP_ADDRESS_LENGTH,1,IP_ADDRESS_LEN - 1);
	CStringsMaps::AddMinMaxItem(_0_TO_IPV6_ADDRESS_LENGTH,0,IPV6_ADDRESS_LEN - 1);
	CStringsMaps::AddMinMaxItem(_1_TO_OPERATOR_NAME_LENGTH,1,OPERATOR_NAME_LEN - 1);
	CStringsMaps::AddMinMaxItem(_0_TO_OPERATOR_NAME_LENGTH,0,OPERATOR_NAME_LEN - 1);
	CStringsMaps::AddMinMaxItem(_1_TO_STATION_NAME_LENGTH,1,MAX_AUDIT_WORKSTATION_NAME_LEN - 1);
	CStringsMaps::AddMinMaxItem(_0_TO_ENCRYPT_PASSWORD_LENGTH,0,SHA1_PWD_LEN);
	CStringsMaps::AddMinMaxItem(_0_TO_ENCRYPT_PASSWORD_SHA256_LENGTH,0,OPERATOR_PWD_LEN);
	CStringsMaps::AddMinMaxItem(CONF_ID,0,0xF+1);

	CStringsMaps::AddItem(CONF_CP_RESOLUTION_ENUM, 0, "CIF");
	CStringsMaps::AddItem(CONF_CP_RESOLUTION_ENUM, 1, "SD15");//Only in MPM mode
	CStringsMaps::AddItem(CONF_CP_RESOLUTION_ENUM, 2, "SD30");
	CStringsMaps::AddItem(CONF_CP_RESOLUTION_ENUM, 3, "HD");//Only in MPM mode
	CStringsMaps::AddItem(CONF_CP_RESOLUTION_ENUM, 4, "HD720");
	CStringsMaps::AddItem(CONF_CP_RESOLUTION_ENUM, 5, "HD1080");

	CStringsMaps::AddItem(RESOLUTION_SLIDER_ENUM, eAuto_Res, "auto");
	CStringsMaps::AddItem(RESOLUTION_SLIDER_ENUM, eCIF_Res, "cif");
	CStringsMaps::AddItem(RESOLUTION_SLIDER_ENUM, eSD_Res, "sd");
	CStringsMaps::AddItem(RESOLUTION_SLIDER_ENUM, eHD720_Res, "hd_720");
	CStringsMaps::AddItem(RESOLUTION_SLIDER_ENUM, eHD1080_Res, "hd_1080");


	CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,SUPER,"administrator");
	CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,ORDINARY,"operator");
	CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,GUEST,"moderator");
	CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,RECORDING_USER,"recording_user");
	CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,RECORDING_ADMIN,"recording_administrator");
	CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,AUTH_OPERATOR,"attendant");
	CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,ANONYMOUS,"anonymous");
    CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,AUDITOR, "auditor");
    CStringsMaps::AddItem(AUTHORIZATION_GROUP_ENUM,ADMINISTRATOR_READONLY, "administrator_readonly");

    CStringsMaps::AddItem(AUDIBLE_ALARM_TYPE_ENUM,0, "AwaitingOperatorAssistance");
    CStringsMaps::AddMinMaxItem(_1_TO_NUMBER_OF_AUDIBLE_REPETITIONS,1,1000);
    CStringsMaps::AddMinMaxItem(_5_TO_NUMBER_OF_AUDIBLE_INTERVALS,5,1000);

    CStringsMaps::AddItem(ENTITY_ENUM, ENTITY_MANAGEMENT, "management");
    CStringsMaps::AddItem(ENTITY_ENUM, ENTITY_SHELF, "shelf");

	CStringsMaps::AddMinMaxItem(_0_TO_PRIVATE_VERSION_DESC_LENGTH,0,PRIVATE_VERSION_DESC_LEN - 1);

	// TBD Add real product types here
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	0,	"mgc_100");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	1,	"mgc_50");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	2,	"mgc_25");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	3,	"mgc_25_recorder");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	4,	"mgc_100_plus");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	5,	"mgc_50_plus");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	6,	"mgc_25_plus");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	7,	"Rmx");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	8,	"Rmx_2000");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	9,	"Rmx_1000");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	10,	"Rmx_6000");
//	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	11,	"Rmx_4000");
	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,    eProductTypeRMX2000,	"Rmx_2000");
	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	eProductTypeRMX4000,	"Rmx_4000");
	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	eProductTypeRMX1500,	"Rmx_1500");
	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	eProductTypeNPG2000,    "npg_2000"); //"npg_2000");
	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,	eProductTypeCallGenerator,	    "Call_Generator");

	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,    eProductTypeSoftMCU,    "SoftMCU");
	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,    eProductTypeGesher,    "Rmx_800s");
	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,    eProductTypeNinja,    "Rmx_1800");
    CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,    eProductTypeSoftMCUMfw, "SoftMCU_Mfw");
	CStringsMaps::AddItem(PRODUCT_TYPE_ENUM,    eProductTypeEdgeAxis,   "Rmx_800ve");


	CStringsMaps::AddItem(BACKUP_TYPE_ENUM,		0,	"success");
	CStringsMaps::AddItem(BACKUP_TYPE_ENUM,		1,	"in_progress");
	CStringsMaps::AddItem(BACKUP_TYPE_ENUM,		2,	"failure");
	CStringsMaps::AddItem(BACKUP_TYPE_ENUM,		3,	"failure_timeout");
	CStringsMaps::AddItem(BACKUP_TYPE_ENUM,		4,	"failure_tar");
	CStringsMaps::AddItem(BACKUP_TYPE_ENUM,		5,	"failure_encrypt");
	CStringsMaps::AddItem(BACKUP_TYPE_ENUM,		6,	"idle");

	CStringsMaps::AddItem(RESTORE_TYPE_ENUM,		0,	"success");
	CStringsMaps::AddItem(RESTORE_TYPE_ENUM,		1,	"in_progress");
	CStringsMaps::AddItem(RESTORE_TYPE_ENUM,		2,	"failure");
	CStringsMaps::AddItem(RESTORE_TYPE_ENUM,		3,	"failure_timeout");
	CStringsMaps::AddItem(RESTORE_TYPE_ENUM,		4,	"failure_untar");
	CStringsMaps::AddItem(RESTORE_TYPE_ENUM,		5,	"failure_decrypt");

	CStringsMaps::AddItem(ENTRY_QUEUE_ROUTING_ENUM,0,"numeric_id_routing");
	CStringsMaps::AddItem(ENTRY_QUEUE_ROUTING_ENUM,1,"conference_password_routing");

	CStringsMaps::AddItem(IP_ADDRESS,1,"dummy string, never found");
	CStringsMaps::AddItem(IP_V6_ADDRESS,1,"dummy string, never found");
	CStringsMaps::AddItem(DATE_TIME,1,"dummy string, never found");

	CStringsMaps::AddItem(SIP_ADDRESS_TYPE_ENUM, PARTY_SIP_SIPURI_ID_TYPE, "uri_type");
	CStringsMaps::AddItem(SIP_ADDRESS_TYPE_ENUM, PARTY_SIP_TELURL_ID_TYPE, "tel_url_type");

	CStringsMaps::AddItem(API_FORMAT_TYPE_ENUM, 0, "xml_api");
	CStringsMaps::AddItem(API_FORMAT_TYPE_ENUM, 1, "rest_api");


	CStringsMaps::AddMinMaxItem(_5_TO_999_DECIMAL,5,999);
	CStringsMaps::AddMinMaxItem(_0_TO_99_DECIMAL,0,99);
	CStringsMaps::AddMinMaxItem(_0_TO_59_DECIMAL,0,59);
	CStringsMaps::AddMinMaxItem(_1_TO_60_DECIMAL,1,60);
	CStringsMaps::AddMinMaxItem(_0_TO_60_DECIMAL,0,60);
	CStringsMaps::AddMinMaxItem(_0_TO_10_DECIMAL,0,10);
	CStringsMaps::AddMinMaxItem(_1_TO_10_DECIMAL,1,10);
	CStringsMaps::AddMinMaxItem(_0_TO_3000_DECIMAL,0,3000);
	CStringsMaps::AddMinMaxItem(_1_TO_31_DECIMAL,1,31);
	CStringsMaps::AddMinMaxItem(_0_TO_23_DECIMAL,0,23);
	CStringsMaps::AddMinMaxItem(_0_TO_100000_DECIMAL,0,100000);
	CStringsMaps::AddMinMaxItem(_2_TO_120_DECIMAL,2,120);
	CStringsMaps::AddMinMaxItem(_3_TO_50_DECIMAL,3,150);
	CStringsMaps::AddMinMaxItem(_6_TO_240_DECIMAL,6,240);
	CStringsMaps::AddMinMaxItem(_1_TO_15_DECIMAL,1,15);
	CStringsMaps::AddMinMaxItem(_1_TO_1000_DECIMAL,1,1000);
	CStringsMaps::AddMinMaxItem(_64_TO_8192_DECIMAL,64,8192);
	CStringsMaps::AddMinMaxItem(_0_TO_1536_DECIMAL,0,1536);
	CStringsMaps::AddMinMaxItem(_0_TO_255_DECIMAL,0,255);
	CStringsMaps::AddMinMaxItem(_0_TO_63_DECIMAL,0,63);


	CStringsMaps::AddMinMaxItem(_600_TO_DWORD,600,0xFFFFFFFF);
	CStringsMaps::AddMinMaxItem(_10_720_DECIMAL,10,720);

	CStringsMaps::AddMinMaxItem(_0_TO_90_DECIMAL,0,90);
	CStringsMaps::AddMinMaxItem(_1_TO_90_DECIMAL,1,90);
	CStringsMaps::AddMinMaxItem(_7_TO_90_DECIMAL,7,90);
	CStringsMaps::AddMinMaxItem(_0_TO_14_DECIMAL,0,14);
	CStringsMaps::AddMinMaxItem(_7_TO_14_DECIMAL,7,14);
	CStringsMaps::AddMinMaxItem(_0_TO_16_DECIMAL,0,16);
	CStringsMaps::AddMinMaxItem(_10_TO_16_DECIMAL,10,16);
	CStringsMaps::AddMinMaxItem(_0_TO_7_DECIMAL,0,7);
	CStringsMaps::AddMinMaxItem(_0_TO_2_DECIMAL,0,2);
	CStringsMaps::AddMinMaxItem(_0_TO_4_DECIMAL,0,4);
	CStringsMaps::AddMinMaxItem(_1_TO_2_DECIMAL,1,2);
	CStringsMaps::AddMinMaxItem(_2_TO_3_DECIMAL,2,3);
	CStringsMaps::AddMinMaxItem(_1_TO_4_DECIMAL,1,4);
	CStringsMaps::AddMinMaxItem(_1_TO_7_DECIMAL,1,7);
	CStringsMaps::AddMinMaxItem(_15_TO_30_DECIMAL,15,30);
	CStringsMaps::AddMinMaxItem(_15_TO_20_DECIMAL,15,20);
	CStringsMaps::AddMinMaxItem(_0_TO_20_DECIMAL,0,20);
	CStringsMaps::AddMinMaxItem(_1_TO_20_DECIMAL,1,20);
	CStringsMaps::AddMinMaxItem(_0_TO_999_DECIMAL,0,999);
	CStringsMaps::AddMinMaxItem(_1_TO_999_DECIMAL,1,999);
	CStringsMaps::AddMinMaxItem(_5_TO_60_DECIMAL,5,60);
	CStringsMaps::AddMinMaxItem(_2_TO_10_DECIMAL,2,10);
	CStringsMaps::AddMinMaxItem(_10_TO_80_DECIMAL,10,80);
	CStringsMaps::AddMinMaxItem(_4_TO_80_DECIMAL,4,80);
	CStringsMaps::AddMinMaxItem(_0_TO_45000_DECIMAL,0,45000);
	CStringsMaps::AddMinMaxItem(_1_TO_45000_DECIMAL,1,45000);
	CStringsMaps::AddMinMaxItem(_30_TO_90_DECIMAL,30,90);
	CStringsMaps::AddMinMaxItem(_0_TO_480_DECIMAL,0,480);
	CStringsMaps::AddMinMaxItem(_9_TO_16_DECIMAL,9,16);
	CStringsMaps::AddMinMaxItem(_6_TO_20_DECIMAL,6,20);
	CStringsMaps::AddMinMaxItem(_0_TO_100_DECIMAL,0,100);
	CStringsMaps::AddMinMaxItem(_10000_TO_1000000_DECIMAL,10000,1000000);
	CStringsMaps::AddMinMaxItem(_0_TO_75_DECIMAL,0,75);
	CStringsMaps::AddMinMaxItem(_0_TO_384_DECIMAL,0,384);
	CStringsMaps::AddMinMaxItem(_180_TO_360_DECIMAL,180,360);

	CStringsMaps::AddMinMaxItem(_MINUS_1_TO_300_DECIMAL,-1,300);
	CStringsMaps::AddMinMaxItem(_MINUS_1_TO_100_DECIMAL,-1,100);

	CStringsMaps::AddMinMaxItem(_0_TO_300_DECIMAL,0,300);

	CStringsMaps::AddMinMaxItem(_5_TO_120_DECIMAL,5,120);

	CStringsMaps::AddItem(_YES_ONLY, 1, "YES");

	CStringsMaps::AddItem(_NO_ONLY, 0, "NO");

	// H323 Alias type common to CsMngr and ConfParty
	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,0,"none");
	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,PARTY_H323_ALIAS_H323_ID_TYPE,"323_id");
	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,PARTY_H323_ALIAS_E164_TYPE,"e164");
	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,PARTY_H323_ALIAS_URL_ID_TYPE,"url_id");
	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,PARTY_H323_ALIAS_EMAIL_ID_TYPE,"email_id");
	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,PARTY_H323_ALIAS_TRANSPORT_ID_TYPE,"transport_id");
	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,PARTY_H323_ALIAS_PARTY_NUMBER_TYPE,"party_number");
//	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,PARTY_SIP_SIPURI_ID_TYPE,"sip_uri");
//	CStringsMaps::AddItem(ALIAS_TYPE_ENUM,PARTY_SIP_TELURL_ID_TYPE,"sip_tel");


	CStringsMaps::AddMinMaxItem(ALIAS_NAME_LENGTH,0,ALIAS_NAME_LEN - 1);

	CStringsMaps::AddItem(OPERATING_SYSTEM_ENUM,OS_XPE,"xpe");
	CStringsMaps::AddItem(OPERATING_SYSTEM_ENUM,OS_LINUX,"linux");


	CStringsMaps::AddItem(SYSTEM_CARDS_MODE_ENUM, eSystemCardsMode_mpm,			::GetSystemCardsModeStr(eSystemCardsMode_mpm));
	CStringsMaps::AddItem(SYSTEM_CARDS_MODE_ENUM, eSystemCardsMode_mpm_plus,	::GetSystemCardsModeStr(eSystemCardsMode_mpm_plus));
	CStringsMaps::AddItem(SYSTEM_CARDS_MODE_ENUM, eSystemCardsMode_breeze,	"mpm-x");//we need to send a string like CMA & DMA use, and not this ::GetSystemCardsModeStr(eSystemCardsMode_breeze));
	CStringsMaps::AddItem(SYSTEM_CARDS_MODE_ENUM, eSystemCardsMode_mpmrx,			"mpm-rx");
	CStringsMaps::AddItem(SYSTEM_CARDS_MODE_ENUM, eSystemCardsMode_mixed_mode,      "mpmx-mpm-rx");
	CStringsMaps::AddItem(SYSTEM_CARDS_MODE_ENUM, eSystemCardsMode_meridian,      "meridian");

	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_half,	"512_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_1,	"1024_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_2,	"2048_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_3,	"3072_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_4,	"4096_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_5,	"5120_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_6,	"6144_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_7,	"7168_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_8,	"8192_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_9,	"9216_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_10,	"10240_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_11,	"11264_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_12,	"12288_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_13,	"13312_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_14,	"14336_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_15,	"15360_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_16,	"16384_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_17,	"17408_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_18,	"18432_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_19,	"19456_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_20,	"20480_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_21,	"21504_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_22,	"22528_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_23,	"23552_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_24,	"24576_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_25,	"25600_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_26,	"26624_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_27,	"27648_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_28,	"28672_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_29,	"29696_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_30,	"30720_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_31,	"31744_mb");
	CStringsMaps::AddItem(SYSTEM_RAM_SIZE_ENUM, eSystemRamSize_full_32,	"32768_mb");


	CStringsMaps::AddMinMaxItem(NET_SERVICE_PROVIDER_NAME_LENGTH,0,NET_SERVICE_PROVIDER_NAME_LEN - 1);
	CStringsMaps::AddMinMaxItem(_0_TO_RTM_ISDN_SERVICE_PROVIDER_NAME_LENGTH,0,RTM_ISDN_SERVICE_PROVIDER_NAME_LEN - 1);


	CStringsMaps::AddMinMaxItem(_50_TO_1000_DECIMAL,50,1000);
	CStringsMaps::AddMinMaxItem(_100_TO_1000_DECIMAL,100,1000);
	CStringsMaps::AddMinMaxItem(_1_TO_5_DECIMAL,1,5);
//	CStringsMaps::AddMinMaxItem(_0_TO_5_DECIMAL,0,5);
	CStringsMaps::AddMinMaxItem(_0_TO_30_DECIMAL,0,30);
	CStringsMaps::AddMinMaxItem(_0_TO_120_DECIMAL,0,120);
	CStringsMaps::AddMinMaxItem(_20_TO_120_DECIMAL,20,120);


	CStringsMaps::AddMinMaxItem(_0_TO_NEW_FILE_NAME_LENGTH,0,NEW_FILE_NAME_LEN - 1);
	CStringsMaps::AddMinMaxItem(_1_TO_NEW_FILE_NAME_LENGTH,1,NEW_FILE_NAME_LEN - 1);


	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateDisable,    "disable");
	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateInit,       "init");
	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateRestart,    "restart");
	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateSelecting,  "selecting");
	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateRequesting, "requesting");
	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateBinding,    "binding");
	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateBound,      "bound");
	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateRenew,      "renew");
	CStringsMaps::AddItem(DHCP_STATE_ENUM, eDHCPStateRebind,     "rebind");

	// GK type common to CsMngr, McuMngr and MplApi (contain CIPService object)
	CStringsMaps::AddItem(GATEKEEPER_ENUM,GATEKEEPER_NONE,"none");
	CStringsMaps::AddItem(GATEKEEPER_ENUM,GATEKEEPER_EXTERNAL,"external");
	CStringsMaps::AddItem(GATEKEEPER_ENUM,GATEKEEPER_INTERNAL,"internal");

	// GK_Mode type common to CsMngr, McuMngr and MplApi (contain CIPService object)
	CStringsMaps::AddItem(GATE_KEEPER_MODE_ENUM,GK_MODE_BOARD_HUNTING,"board_hunting");
	CStringsMaps::AddItem(GATE_KEEPER_MODE_ENUM,GK_MODE_BASIC,"basic");
	CStringsMaps::AddItem(GATE_KEEPER_MODE_ENUM,GK_MODE_GATEWAY,"gateway");
	CStringsMaps::AddItem(GATE_KEEPER_MODE_ENUM,GK_MODE_PSEUDO_GK,"pseudo_gatekeeper");
	CStringsMaps::AddItem(GATE_KEEPER_MODE_ENUM,GK_MODE_PSEUDO_AVAYA_GK,"pseudo_avaya_gatekeeper");

	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_ENUM,eNone,"None");
	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_ENUM,eMD5,"MD5");
	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_ENUM,eSHA1,"SHA1");
	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_ENUM,eSHA256,"SHA256");
	CStringsMaps::AddItem(AUTHENTICATION_PROTOCOL_ENUM,eAuto,"Auto");

	// IpProtocol type common to CsMngr, McuMngr and MplApi (contain CIPService object)
	CStringsMaps::AddItem(PROTOCOL_TYPE_ENUM,eIPProtocolType_SIP,"sip");
	CStringsMaps::AddItem(PROTOCOL_TYPE_ENUM,eIPProtocolType_H323,"h323");
	CStringsMaps::AddItem(PROTOCOL_TYPE_ENUM,eIPProtocolType_SIP_H323,"sip_h323");

	// IpService type common to CsMngr, McuMngr and MplApi (contain CIPService object)
	CStringsMaps::AddItem(IP_SERVICE_TYPE_ENUM,eIpServiceType_Signaling,"ipServiceType_Signaling");
	CStringsMaps::AddItem(IP_SERVICE_TYPE_ENUM,eIpServiceType_Management,"ipServiceType_Management");
	CStringsMaps::AddItem(IP_SERVICE_TYPE_ENUM,eIpServiceType_Control,"ipServiceType_Control");

	CStringsMaps::AddItem(REMOTE_FLAG_ENUM,H323_REMOTE_NETWORK,"remote_network");
	CStringsMaps::AddItem(REMOTE_FLAG_ENUM,H323_REMOTE_HOST,"remote_host");

	CStringsMaps::AddItem(REGISTRATION_MODE_ENUM, eRegistrationMode_Redirect, RegistrationModeStr[eRegistrationMode_Redirect]);
	CStringsMaps::AddItem(REGISTRATION_MODE_ENUM, eRegistrationMode_Polling	, RegistrationModeStr[eRegistrationMode_Polling	]);
	CStringsMaps::AddItem(REGISTRATION_MODE_ENUM, eRegistrationMode_Move	, RegistrationModeStr[eRegistrationMode_Move	]);
	CStringsMaps::AddItem(REGISTRATION_MODE_ENUM, eRegistrationMode_DNS		, RegistrationModeStr[eRegistrationMode_DNS		]);
	CStringsMaps::AddItem(REGISTRATION_MODE_ENUM, eRegistrationMode_Forking	, RegistrationModeStr[eRegistrationMode_Forking	]);

	CStringsMaps::AddItem(IP_TYPE_ENUM, eIpType_None, ::IpTypeToString(eIpType_None));
	CStringsMaps::AddItem(IP_TYPE_ENUM, eIpType_IpV4, ::IpTypeToString(eIpType_IpV4));
	CStringsMaps::AddItem(IP_TYPE_ENUM, eIpType_IpV6, ::IpTypeToString(eIpType_IpV6));
	CStringsMaps::AddItem(IP_TYPE_ENUM, eIpType_Both, ::IpTypeToString(eIpType_Both));

	CStringsMaps::AddItem(IP_V6_CONFIG_TYPE_ENUM, eV6Configuration_Auto,   ::IpV6ConfigurationTypeToString(eV6Configuration_Auto));
	CStringsMaps::AddItem(IP_V6_CONFIG_TYPE_ENUM, eV6Configuration_DhcpV6, ::IpV6ConfigurationTypeToString(eV6Configuration_DhcpV6));
	CStringsMaps::AddItem(IP_V6_CONFIG_TYPE_ENUM, eV6Configuration_Manual, ::IpV6ConfigurationTypeToString(eV6Configuration_Manual));

	CStringsMaps::AddItem(IP_V6_ADDRESS_SCOPE_ENUM, eIPv6AddressScope_linkLocal,			GetIPv6AddressScopeStr(eIPv6AddressScope_linkLocal));
	CStringsMaps::AddItem(IP_V6_ADDRESS_SCOPE_ENUM, eIPv6AddressScope_siteLocal,			GetIPv6AddressScopeStr(eIPv6AddressScope_siteLocal));
	CStringsMaps::AddItem(IP_V6_ADDRESS_SCOPE_ENUM, eIPv6AddressScope_global,				GetIPv6AddressScopeStr(eIPv6AddressScope_global));
	CStringsMaps::AddItem(IP_V6_ADDRESS_SCOPE_ENUM, eIPv6AddressScope_multicast,			GetIPv6AddressScopeStr(eIPv6AddressScope_multicast));
	CStringsMaps::AddItem(IP_V6_ADDRESS_SCOPE_ENUM, eIPv6AddressScope_loopBack,				GetIPv6AddressScopeStr(eIPv6AddressScope_loopBack));
	CStringsMaps::AddItem(IP_V6_ADDRESS_SCOPE_ENUM, eIPv6AddressScope_uniqueLocalUnicast,	GetIPv6AddressScopeStr(eIPv6AddressScope_uniqueLocalUnicast));
	CStringsMaps::AddItem(IP_V6_ADDRESS_SCOPE_ENUM, eIPv6AddressScope_other,				GetIPv6AddressScopeStr(eIPv6AddressScope_other));

	// SpeedMode type common to CsMngr, McuMngr and MplApi (contain CIPSpan object)
// 	CStringsMaps::AddItem(SPEED_MODE_ENUM,H323_SPEED_AUTO,"speed_auto");
// 	CStringsMaps::AddItem(SPEED_MODE_ENUM,H323_SPEED_10,"speed_10");
// 	CStringsMaps::AddItem(SPEED_MODE_ENUM,H323_SPEED_10_DUPLEX_FULL,"speed_10_duplex_full");
// 	CStringsMaps::AddItem(SPEED_MODE_ENUM,H323_SPEED_100,"speed_100");
// 	CStringsMaps::AddItem(SPEED_MODE_ENUM,H323_SPEED_100_DUPLEX_FULL,"speed_100_duplex_full");

	CStringsMaps::AddItem(HASH_METHOD_ENUM, eHashMethodSHA1,			"sha1");
	CStringsMaps::AddItem(HASH_METHOD_ENUM, eHashMethodSHA256,			"sha256");

	CStringsMaps::AddItem(SPEED_MODE_ENUM, ePortSpeed_Auto,           PortSpeedTypeToString(ePortSpeed_Auto));
	CStringsMaps::AddItem(SPEED_MODE_ENUM, ePortSpeed_10_HalfDuplex,  PortSpeedTypeToString(ePortSpeed_10_HalfDuplex));
	CStringsMaps::AddItem(SPEED_MODE_ENUM, ePortSpeed_10_FullDuplex,  PortSpeedTypeToString(ePortSpeed_10_FullDuplex));
	CStringsMaps::AddItem(SPEED_MODE_ENUM, ePortSpeed_100_HalfDuplex, PortSpeedTypeToString(ePortSpeed_100_HalfDuplex));
	CStringsMaps::AddItem(SPEED_MODE_ENUM, ePortSpeed_100_FullDuplex, PortSpeedTypeToString(ePortSpeed_100_FullDuplex));
//	CStringsMaps::AddItem(SPEED_MODE_ENUM, ePortSpeed_1000_HalfDuplex,PortSpeedTypeToString(ePortSpeed_1000_HalfDuplex));
	CStringsMaps::AddItem(SPEED_MODE_ENUM, ePortSpeed_1000_FullDuplex,PortSpeedTypeToString(ePortSpeed_1000_FullDuplex));

	CStringsMaps::AddItem(ICE_ENVIRONMENT_ENUM,eIceEnvironment_None,"iceEnvironment_none");
	CStringsMaps::AddItem(ICE_ENVIRONMENT_ENUM,eIceEnvironment_ms,"iceEnvironment_ms");
	CStringsMaps::AddItem(ICE_ENVIRONMENT_ENUM,eIceEnvironment_Standard,"iceEnvironment_standard");
	CStringsMaps::AddItem(ICE_ENVIRONMENT_ENUM,eIceEnvironment_WebRtc,"iceEnvironment_webrtc");

	// ServerStatus type common to CsMngr, McuMngr and MplApi (contain CIPSpan object)
	CStringsMaps::AddItem(SERVER_STATUS_ENUM, eServerStatusAuto		, ServerStatusStr[eServerStatusAuto		]);
	CStringsMaps::AddItem(SERVER_STATUS_ENUM, eServerStatusSpecify	, ServerStatusStr[eServerStatusSpecify	]);
	CStringsMaps::AddItem(SERVER_STATUS_ENUM, eServerStatusOff		, ServerStatusStr[eServerStatusOff		]);

	// QosAction type common to CsMngr, McuMngr and MplApi (contain CQualityOfService object)
	CStringsMaps::AddItem(QOS_ACTION_ENUM,0,"disabled");
	CStringsMaps::AddItem(QOS_ACTION_ENUM,1,"enabled");
	CStringsMaps::AddItem(QOS_ACTION_ENUM,2,"from_service");

	// QosDiffServ type common to CsMngr, McuMngr and MplApi (contain CQualityOfService object)
	CStringsMaps::AddItem(QOS_DIFF_SERV_ENUM,YES,"diffserv");
	CStringsMaps::AddItem(QOS_DIFF_SERV_ENUM,NO,"precedence");

	// QosTos type common to CsMngr, McuMngr and MplApi (contain CQualityOfService object)
	CStringsMaps::AddItem(QOS_TOS_ENUM,0x0,"none");
	CStringsMaps::AddItem(QOS_TOS_ENUM,0x8,"delay");

	// common to CsMngr, McuMngr and MplApi (contain CQualityOfService object)
	CStringsMaps::AddMinMaxItem(_0_TO_5_DECIMAL,0,5);

	// common to Cards and McuMngr
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eEmpty,				::CardTypeToString(eEmpty));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eCpuBoard,			::CardTypeToString(eCpuBoard));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eSwitch,				::CardTypeToString(eSwitch));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMfa_26,				::CardTypeToString(eMfa_26));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMfa_13,				::CardTypeToString(eMfa_13));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eRtmIsdn,				::CardTypeToString(eRtmIsdn));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eControl,				::CardTypeToString(eControl));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmPlus_20,			::CardTypeToString(eMpmPlus_20));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmPlus_40,			::CardTypeToString(eMpmPlus_40));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmPlus_80,			::CardTypeToString(eMpmPlus_80));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmPlus_MezzanineA,	::CardTypeToString(eMpmPlus_MezzanineA));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmPlus_MezzanineB,	::CardTypeToString(eMpmPlus_MezzanineB));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmx_80,				::CardTypeToString(eMpmx_80));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmx_40,	        	::CardTypeToString(eMpmx_40));
	CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmx_20,	        	::CardTypeToString(eMpmx_20));
    CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmx_Soft_Half,      ::CardTypeToString(eMpmx_Soft_Half)); //OLGA - SoftMCU
    CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmx_Soft_Full,      ::CardTypeToString(eMpmx_Soft_Full)); //OLGA - SoftMCU
    CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmRx_Half,          ::CardTypeToString(eMpmRx_Half));
    CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmRx_Full,          ::CardTypeToString(eMpmRx_Full));
    CStringsMaps::AddItem(CARD_TYPE_ENUM, eMpmRx_Ninja,         ::CardTypeToString(eMpmRx_Ninja));
    CStringsMaps::AddItem(CARD_TYPE_ENUM, eRtmIsdn_9PRI,		::CardTypeToString(eRtmIsdn_9PRI));
    CStringsMaps::AddItem(CARD_TYPE_ENUM, eRtmIsdn_9PRI_10G,	::CardTypeToString(eRtmIsdn_9PRI_10G));

	// Transport type common to CsMngr, McuMngr and MplApi (contain CSip object)
	CStringsMaps::AddItem(TRANSPORT_TYPE_ENUM, eTransportTypeUdp, TransportTypeStr[eTransportTypeUdp]);
	CStringsMaps::AddItem(TRANSPORT_TYPE_ENUM, eTransportTypeTcp, TransportTypeStr[eTransportTypeTcp]);
    CStringsMaps::AddItem(TRANSPORT_TYPE_ENUM, eTransportTypeTls, TransportTypeStr[eTransportTypeTls]);

	// ConfigSipServer type common to CsMngr, McuMngr and MplApi (contain CSip object)
	CStringsMaps::AddItem(CONFIGURE_SIP_SERVERS_ENUM, eConfSipServerAuto		, ConfigurationSipServerModeStr[eConfSipServerAuto	  ]);
	CStringsMaps::AddItem(CONFIGURE_SIP_SERVERS_ENUM, eConfSipServerManually	, ConfigurationSipServerModeStr[eConfSipServerManually]);

	CStringsMaps::AddItem(NETWORK_ENUM,NETWORK_H320,"h320");
	CStringsMaps::AddItem(NETWORK_ENUM,NETWORK_H323,"h323");
	CStringsMaps::AddItem(NETWORK_ENUM,NETWORK_H320_H323,"h320_h323");

	CStringsMaps::AddItem(VIDEO_SESSION_ENUM,VIDEO_SWITCH,"switching");
	CStringsMaps::AddItem(VIDEO_SESSION_ENUM,VIDEO_TRANSCODING,"transcoding");
	CStringsMaps::AddItem(VIDEO_SESSION_ENUM,CONTINUOUS_PRESENCE,"continuous_presence");
	CStringsMaps::AddItem(VIDEO_SESSION_ENUM,SOFTWARE_CONTINUOUS_PRESENCE,"software_cp");
	CStringsMaps::AddItem(VIDEO_SESSION_ENUM,ADVANCED_LAYOUTS,"advanced_layout");
	CStringsMaps::AddItem(VIDEO_SESSION_ENUM,VIDEO_SESSION_COP,"cop");

	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,VIDEO_PROTOCOL_H261,"h261");
	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,VIDEO_PROTOCOL_H263,"h263");
	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,AUTO,"auto");
	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,VIDEO_PROTOCOL_H26L,"h26L");
	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,VIDEO_PROTOCOL_H264,"h264");
	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,VIDEO_PROTOCOL_RTV,"rtv");
	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,VIDEO_PROTOCOL_MS_SVC,"ms_svc");
	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,VIDEO_PROTOCOL_H264_HIGH_PROFILE,"h264_high_profile");
	CStringsMaps::AddItem(VIDEO_PROTOCOL_ENUM,VIDEO_PROTOCOL_VP8,"VP8");

	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_64,"64");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_2x64,"2x64");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_3x64,"3x64");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_4x64,"4x64");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_5x64,"5x64");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_6x64,"6x64");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_128,"128");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_192,"192");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_256,"256");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_320,"320");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_384,"384");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_512,"512");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_768,"768");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_832,"832");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_1152,"1152");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_1280,"1280");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_1472,"1472");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_1536,"1536");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_1728,"1728");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_1920,"1920");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_96,"96");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_1024,"1024");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_2048,"2048");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_4096,"4096");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_6144,"6144");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_2560,"2560");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_3072,"3072");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_3584,"3584");
	CStringsMaps::AddItem(TRANSFER_RATE_ENUM,Xfer_8192,"8192");


	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_Neutral,"au_neutral");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Capex,"capex");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,A_Law_OU,"a_law_ou");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,U_Law_OU,"u_law_ou");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G722_m1,"g722_m1");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_Off_U,"au_off_u");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G723_1_Command,"g723");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G729_8k,"g729_8");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G7221_AnnexC_24k,"G7221_AnnexC_24k");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G7221_AnnexC_32k,"G7221_AnnexC_32k");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G7221_AnnexC_48k,"G7221_AnnexC_48k");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_Iso_256,"au_iso_256");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_Iso_384,"au_iso_384");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,A_Law_OF,"a_law_of");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,U_Law_OF,"g711_56");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,A_Law_48,"a_law_48");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,U_Law_48,"u_law_48");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G722_m2,"g722_56");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G722_m3,"g722_m3");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_40k,"au_40");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_32k,"g722_32");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_24k,"g722_24");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,G728,"g728");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_8k,"au_8");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,Au_Off_F,"au_off_f");
	CStringsMaps::AddItem(AUDIO_RATE_ENUM,AUTO,"auto");

	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,V_Qcif,"qcif");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,V_Cif,"cif");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,H263_CIF_4,"4cif");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,H263_CIF_16,"16cif");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,kVGA,"vga");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,kSVGA,"svga");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,kXGA,"xga");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,kNTSC,"ntsc");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,k720p,"720p");
	CStringsMaps::AddItem(VIDEO_FORMAT_ENUM,255,"auto");

	CStringsMaps::AddItem(FRAME_RATE_ENUM,V_1_29_97,"30");
	CStringsMaps::AddItem(FRAME_RATE_ENUM,V_2_29_97,"15");
	CStringsMaps::AddItem(FRAME_RATE_ENUM,V_3_29_97,"10");
	CStringsMaps::AddItem(FRAME_RATE_ENUM,V_4_29_97,"7.5");
	CStringsMaps::AddItem(FRAME_RATE_ENUM,255,"auto");

	CStringsMaps::AddItem(COP_VIDEO_FORMAT_ENUM,eCopLevelEncoderVideoFormat_QCIF,"qcif");
	CStringsMaps::AddItem(COP_VIDEO_FORMAT_ENUM,eCopLevelEncoderVideoFormat_CIF,"cif_sif");
	CStringsMaps::AddItem(COP_VIDEO_FORMAT_ENUM,eCopLevelEncoderVideoFormat_4CIF,"4cif_4sif_4_3");
	CStringsMaps::AddItem(COP_VIDEO_FORMAT_ENUM,eCopLevelEncoderVideoFormat_4CIF_16_9,"4cif_4sif_16_9");
	CStringsMaps::AddItem(COP_VIDEO_FORMAT_ENUM,eCopLevelEncoderVideoFormat_HD720p,"720p");
	CStringsMaps::AddItem(COP_VIDEO_FORMAT_ENUM,eCopLevelEncoderVideoFormat_HD1080p,"1080p");
	CStringsMaps::AddItem(COP_VIDEO_FORMAT_ENUM,255,"auto");

	CStringsMaps::AddItem(COP_VIDEO_FRAME_RATE_ENUM,eCopVideoFrameRate_12_5,"12.5");
	CStringsMaps::AddItem(COP_VIDEO_FRAME_RATE_ENUM,eCopVideoFrameRate_15,"15");
	CStringsMaps::AddItem(COP_VIDEO_FRAME_RATE_ENUM,eCopVideoFrameRate_25,"25");
	CStringsMaps::AddItem(COP_VIDEO_FRAME_RATE_ENUM,eCopVideoFrameRate_30,"30");
	CStringsMaps::AddItem(COP_VIDEO_FRAME_RATE_ENUM,eCopVideoFrameRate_50,"50");
	CStringsMaps::AddItem(COP_VIDEO_FRAME_RATE_ENUM,eCopVideoFrameRate_60,"60");
	CStringsMaps::AddItem(COP_VIDEO_FRAME_RATE_ENUM,255,"auto");

	CStringsMaps::AddItem(ASPECT_RATIO_ENUM,AspectRatio_4X3,"4_3");
	CStringsMaps::AddItem(ASPECT_RATIO_ENUM,AspectRatio_16X9,"16_9");
	CStringsMaps::AddItem(ASPECT_RATIO_ENUM,255,"auto");

	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,5,"5");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,10,"10");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,20,"20");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,30,"30");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,60,"60");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,90,"90");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,120,"120");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,150,"150");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,180,"180");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,210,"210");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,240,"240");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,270,"270");
	CStringsMaps::AddItem(AUTO_SCAN_INTERVAL_ENUM,300,"300");

	CStringsMaps::AddMinMaxItem(AV_MSG_LENGTH,0,AV_SERVICE_NAME - 1);

	CStringsMaps::AddItem(LIMITED_SEQ_ENUM,0xFF,"off") ;
	CStringsMaps::AddMinMaxItem(LIMITED_SEQ_ENUM,1,100);

	CStringsMaps::AddItem(LECTURE_MODE_TYPE_ENUM,0,"lecture_none");
	CStringsMaps::AddItem(LECTURE_MODE_TYPE_ENUM,1,"lecture_mode");
	CStringsMaps::AddItem(LECTURE_MODE_TYPE_ENUM,2,"lecture_show");
	CStringsMaps::AddItem(LECTURE_MODE_TYPE_ENUM,3,"lecture_presentation");

	CStringsMaps::AddItem(PRECEDENCE_LEVEL_TYPE_ENUM,0,"routine");
	CStringsMaps::AddItem(PRECEDENCE_LEVEL_TYPE_ENUM,1,"priority");
	CStringsMaps::AddItem(PRECEDENCE_LEVEL_TYPE_ENUM,2,"immediate");
	CStringsMaps::AddItem(PRECEDENCE_LEVEL_TYPE_ENUM,3,"flash");
	CStringsMaps::AddItem(PRECEDENCE_LEVEL_TYPE_ENUM,4,"flash_override");
	CStringsMaps::AddItem(PRECEDENCE_LEVEL_TYPE_ENUM,5,"flash_override_plus");

	CStringsMaps::AddItem(PRECEDENCE_DOMAIN_ID_ENUM,1,"1");
	CStringsMaps::AddItem(PRECEDENCE_DOMAIN_ID_ENUM,2,"2");
	CStringsMaps::AddItem(CASCADE_ROLE_ENUM,CASCADE_MODE_MASTER,"master");
	CStringsMaps::AddItem(CASCADE_ROLE_ENUM,CASCADE_MODE_SLAVE,"slave");
	CStringsMaps::AddItem(CASCADE_ROLE_ENUM,CASCADE_MODE_NEGOTIATED,"negotiated");
	CStringsMaps::AddItem(CASCADE_ROLE_ENUM,CASCADE_MODE_NONE,"none");

	CStringsMaps::AddItem(LINK_TYPE_ENUM,eRegularParty,"regular");
	CStringsMaps::AddItem(LINK_TYPE_ENUM,eSubLinkParty,"sub_link");
	CStringsMaps::AddItem(LINK_TYPE_ENUM,eMainLinkParty,"main_link");

	CStringsMaps::AddItem(MAX_PARTIES_ENUM,0xffff,"automatic");
	CStringsMaps::AddMinMaxItem(MAX_PARTIES_ENUM,1,860);  //2000?????YOELLA CARMEL

	CStringsMaps::AddMinMaxItem(_1_TO_LANGUAGE_NAME_LENGTH,1,LANGUAGE_NAME_LEN - 1);
	CStringsMaps::AddMinMaxItem(_0_TO_CONFERENCE_ENTRY_PASSWORD_LENGTH,0,CONFERENCE_ENTRY_PASSWORD_LEN-1);
	CStringsMaps::AddMinMaxItem(_0_TO_NUMERIC_CONFERENCE_ID_LENGTH,0,NUMERIC_CONFERENCE_ID_LEN-1);
	CStringsMaps::AddMinMaxItem(_0_TO_MAX_PRIVATE_NAME_LENGTH,0,MAX_PRIVATE_NAME_LEN - 1);
	CStringsMaps::AddMinMaxItem(_0_TO_MAX_FULL_PATH_LENGTH,0,MAX_FULL_PATH_LEN - 1);

	CStringsMaps::AddItem(MEDIA_ENUM,AUDIO_MEDIA,"audio");
	CStringsMaps::AddItem(MEDIA_ENUM,VIDEO_MEDIA,"video");
	CStringsMaps::AddItem(MEDIA_ENUM,DATA_MEDIA,"data");
	CStringsMaps::AddItem(MEDIA_ENUM,AUDIO_VIDEO_MEDIA,"video_audio");
	CStringsMaps::AddItem(MEDIA_ENUM,AUDIO_DATA_MEDIA,"audio_data");
	CStringsMaps::AddItem(MEDIA_ENUM,VIDEO_DATA_MEDIA,"video_data");
	CStringsMaps::AddItem(MEDIA_ENUM,ALL_MEDIA,"all");

	CStringsMaps::AddItem(MR_STATE_ENUM,MEETING_ROOM_PASSIVE_STATE,"passive");
	CStringsMaps::AddItem(MR_STATE_ENUM,MEETING_ROOM_ACTIVE_STATE,"active");
	CStringsMaps::AddItem(MR_STATE_ENUM,MEETING_ROOM_INITIALIZE_STATE,"initialize");
	CStringsMaps::AddMinMaxItem(MR_STATE_ENUM,3,255);

	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_ONE,"1x1");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_TWO,"1x2");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_ONE,"2x1");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_TWO,"2x2");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_AND_FIVE,"1and5");
	CStringsMaps::AddItem(LAYOUT_ENUM,THREE_THREE,"3x3");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_TWO_VER,"1x2Ver");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_TWO_HOR,"1x2Hor");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_HOR,"1and2Hor");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_VER,"1and2Ver");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_THREE_HOR,"1and3Hor");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_THREE_VER,"1and3Ver");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_FOUR_VER,"1and4Ver");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_FOUR_HOR,"1and4Hor");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_EIGHT_CENTRAL,"1and8Central");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_EIGHT_UPPER,"1and8Upper");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_HOR_UPPER,"1and2HorUpper");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_THREE_HOR_UPPER,"1and3HorUpper");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_FOUR_HOR_UPPER,"1and4HorUpper");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_EIGTH,"1and8Lower");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_SEVEN,"1and7");
	CStringsMaps::AddItem(LAYOUT_ENUM,FOUR_FOUR,"4x4");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_PLUS_EIGHT,"2and8");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWELVE,"1and12");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_ONE_QCIF,"1x1Qcif");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_TWO_FLEX,"1x2Flex");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_HOR_R_FLEX,"1and2HorRFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_HOR_L_FLEX,"1and2HorLFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_HOR_UP_R_FLEX,"1and2HorUpperRFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_HOR_UP_L_FLEX,"1and2HorUpperLFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_TWO_UP_R_FLEX,"2x2UpperRFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_TWO_UP_L_FLEX,"2x2UpperLFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_TWO_DOWN_R_FLEX,"2x2DownRFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_TWO_DOWN_L_FLEX,"2x2DownLFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_TWO_R_FLEX,"2x2RFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_TWO_L_FLEX,"2x2LFlex");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_ONE_OVERLAY,"1and1Overlay");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_OVERLAY,"1and2Overlay");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_THREE_OVERLAY,"1and3Overlay");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_TWO_OVERLAY_ITP,"1and2OverlayITP");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_THREE_OVERLAY_ITP,"1and3OverlayITP");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_PLUS_FOUR_OVERLAY_ITP,"1and4OverlayITP");
	CStringsMaps::AddItem(LAYOUT_ENUM,ONE_TOP_LEFT_PLUS_EIGHT,"1TopLeftAnd8");
	CStringsMaps::AddItem(LAYOUT_ENUM,TWO_TOP_PLUS_EIGHT,"2TopAnd8");

	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 0, "CP_LAYOUT_1X1");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 1, "CP_LAYOUT_1X2");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 2, "CP_LAYOUT_2X1");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 3, "CP_LAYOUT_3X3");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 4, "CP_LAYOUT_1P5");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 5, "CP_LAYOUT_1P7");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 6, "CP_LAYOUT_1X2VER");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 7, "CP_LAYOUT_1X2HOR");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 8, "CP_LAYOUT_1P2VER");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 9, "CP_LAYOUT_1P2HOR");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 10, "CP_LAYOUT_1P3HOR");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 11, "CP_LAYOUT_1P3VER");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 12, "CP_LAYOUT_1P4HOR");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 13, "CP_LAYOUT_1P4VER");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 14, "CP_LAYOUT_1P8CENT");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 15, "CP_LAYOUT_1P8UP");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 16, "CP_LAYOUT_1P2HOR_UP");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 17, "CP_LAYOUT_1P3HOR_UP");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 18, "CP_LAYOUT_1P4HOR_UP");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 19, "CP_LAYOUT_1P8HOR_UP");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 20, "CP_LAYOUT_4X4");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 21, "CP_LAYOUT_2P8");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 22, "CP_LAYOUT_1P12");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 23, "CP_LAYOUT_1x1");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 24, "CP_LAYOUT_1x2");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 25, "CP_LAYOUT_2x1");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 26, "CP_LAYOUT_3x3");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 27, "CP_LAYOUT_1x2VER");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 28, "CP_LAYOUT_1x2HOR");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 29, "CP_LAYOUT_4x4");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 30, "CP_LAYOUT_2x2");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 31, "CP_LAYOUT_2X2");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 32, "CP_LAYOUT_1TOP_LEFT_P8");
	CStringsMaps::AddItem(FULL_LAYOUT_ENUM, 33, "CP_LAYOUT_2TOP_P8");

	CStringsMaps::AddItem(SITE_NAMES_LOCATION_ENUM,0,"AUTO");
	CStringsMaps::AddItem(SITE_NAMES_LOCATION_ENUM,1,"DOWN_CENTER");
	CStringsMaps::AddItem(SITE_NAMES_LOCATION_ENUM,2,"DOWN_LEFT");
	CStringsMaps::AddItem(SITE_NAMES_LOCATION_ENUM,3,"DOWN_RIGHT");
	CStringsMaps::AddItem(SITE_NAMES_LOCATION_ENUM,4,"UP_CENTER");
	CStringsMaps::AddItem(SITE_NAMES_LOCATION_ENUM,5,"UP_LEFT");
	CStringsMaps::AddItem(SITE_NAMES_LOCATION_ENUM,6,"UP_RIGHT");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,0,"ENGLISH");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,1,"CHINESE_SIMPLIFIED");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,2,"CHINESE_TRADITIONAL");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,3,"JAPANESE");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,4,"GERMAN");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,5,"FRENCH");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,6,"SPANISH");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,7,"KOREAN");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,8,"PORTUGUESE");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,9,"ITALIAN");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,10,"RUSSIAN");
	CStringsMaps::AddItem(PCM_LANGUAGE_ENUM,11,"NORWEGIAN");

	CStringsMaps::AddItem(ATTENDED_MODE_ENUM,0,"none");
	CStringsMaps::AddItem(ATTENDED_MODE_ENUM,1,"welcome_only");
	CStringsMaps::AddItem(ATTENDED_MODE_ENUM,2,"attended");
	CStringsMaps::AddItem(ATTENDED_MODE_ENUM,3,"ivr");

	CStringsMaps::AddItem(ATTENDING_STATE_ENUM,0xff,"none");
	CStringsMaps::AddItem(ATTENDING_STATE_ENUM,2,"inconf");
	CStringsMaps::AddItem(ATTENDING_STATE_ENUM,3,"ivr");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_DISPALY_POSITION_TYPE_ENUM,eTop,"top");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_DISPALY_POSITION_TYPE_ENUM,eBottom,"bottom");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_DISPALY_POSITION_TYPE_ENUM,eMiddle,"middle");

	CStringsMaps::AddItem(MESSAGE_OVERLAY_FONT_TYPE_ENUM,eSmall,"small");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_FONT_TYPE_ENUM,eMedium,"medium");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_FONT_TYPE_ENUM,eLarge,"large");

	CStringsMaps::AddItem(MESSAGE_OVERLAY_SPEED_TYPE_ENUM,eSlow,"slow");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_SPEED_TYPE_ENUM,eFast,"fast");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_SPEED_TYPE_ENUM,eStatic,"static");


	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_light_blue_background,"white_font_on_light_blue_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_black_background,"white_font_on_black_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_gray_background,"white_font_on_gray_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_red_background,"white_font_on_red_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_orange_background,"white_font_on_orange_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_blue_background,"white_font_on_blue_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_olive_background,"white_font_on_olive_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_green_background,"white_font_on_green_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_purple_background,"white_font_on_purple_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eRed_font_on_white_background,"red_font_on_white_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_deep_brown_background,"white_font_on_deep_brown_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eWhite_font_on_brown_background,"white_font_on_brown_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eYellow_font_on_black_background,"yellow_font_on_black_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eYellow_font_on_deep_blue_background,"yellow_font_on_deep_blue_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eLightBlue_font_on_black_background,"lightblue_font_on_black_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eBlue_font_on_white_background,"blue_font_on_white_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eGreen_font_on_black_background,"green_font_on_black_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eGreyGreen_font_on_white_background,"greygreen_font_on_white_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eBlack_font_on_gray_background,"black_font_on_gray_background");
	CStringsMaps::AddItem(MESSAGE_OVERLAY_COLOR_TYPE_ENUM,eBlack_font_on_white_background,"black_font_on_white_background");

	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eRed_font,"red_font");
	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eBlue_font,"blue_font");
	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eSkyBlue_font,"skyblue_font");
	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eGreen_font,"green_font");
	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eYellowGreen_font,"yellowgreen_font");
	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eLightYellow_font,"lightyellow_font");
	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eYellow_font,"yellow_font");
	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eBlack_font,"black_font");
	CStringsMaps::AddItem(TEXT_COLOR_TYPE_ENUM,eWhite_font,"white_font");


	CStringsMaps::AddItem(WAIT_FOR_ASSISTANCE_ENUM,0,"assitance_type_none");
	CStringsMaps::AddItem(WAIT_FOR_ASSISTANCE_ENUM,2,"req_private");
	CStringsMaps::AddItem(WAIT_FOR_ASSISTANCE_ENUM,3,"req_public");


	CStringsMaps::AddItem(FORCE_STATE_ENUM,AUDIO_ACTIVATED,"auto");
	CStringsMaps::AddItem(FORCE_STATE_ENUM,EMPTY_BY_OPERATOR_THIS_PARTY,"blank");
	CStringsMaps::AddItem(FORCE_STATE_ENUM,BY_OPERATOR_ALL_CONF,"forced");
	CStringsMaps::AddItem(FORCE_STATE_ENUM,AUTO_SCAN,"auto_scan");

	CStringsMaps::AddItem(INTERFACE_ENUM,ISDN_INTERFACE_TYPE,"isdn");
	CStringsMaps::AddItem(INTERFACE_ENUM,ATM_INTERFACE_TYPE,"atm");
	CStringsMaps::AddItem(INTERFACE_ENUM,H323_INTERFACE_TYPE,"h323");
	CStringsMaps::AddItem(INTERFACE_ENUM,SIP_INTERFACE_TYPE,"sip");
	CStringsMaps::AddItem(INTERFACE_ENUM,V35_INTERFACE_TYPE,"mpi");
	CStringsMaps::AddItem(INTERFACE_ENUM,T1CAS_INTERFACE_TYPE,"t1cas");

	CStringsMaps::AddItem(CONNECTION_ENUM,DIAL_OUT,"dial_out");
	CStringsMaps::AddItem(CONNECTION_ENUM,DIAL_IN,"dial_in");
	CStringsMaps::AddItem(CONNECTION_ENUM,DIRECT,"direct");

	CStringsMaps::AddItem(MEET_ME_METHOD_ENUM,1,"mcu-conference");
	CStringsMaps::AddItem(MEET_ME_METHOD_ENUM,3,"party");
	CStringsMaps::AddItem(MEET_ME_METHOD_ENUM,4,"channel");

	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,0xff,"auto");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,1,"channel_1");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,2,"channel_2");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,3,"channel_3");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,4,"channel_4");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,5,"channel_5");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,6,"channel_6");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,8,"channel_8");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,12,"channel_12");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,18,"channel_18");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,23,"channel_23");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,24,"channel_24");
	CStringsMaps::AddItem(NET_CHANNEL_NUMBER_ENUM,30,"channel_30");

	CStringsMaps::AddItem(CALL_CONTENT_ENUM,0,"framed");
	CStringsMaps::AddItem(CALL_CONTENT_ENUM,1,"voice");

	CStringsMaps::AddItem(VIDEO_BIT_RATE_ENUM,0xFFFFFFFF,"automatic");
	CStringsMaps::AddMinMaxItem(VIDEO_BIT_RATE_ENUM,0,4096);

	CStringsMaps::AddItem(LAYOUT_TYPE_ENUM,0,"conference");
	CStringsMaps::AddItem(LAYOUT_TYPE_ENUM,1,"personal");

	CStringsMaps::AddItem(NODE_TYPE_ENUM,0,"mcu");
	CStringsMaps::AddItem(NODE_TYPE_ENUM,1,"terminal");

	CStringsMaps::AddItem(BOOL_AUTO_ENUM,AUTO,"auto");
	CStringsMaps::AddItem(BOOL_AUTO_ENUM,YES,"yes");
	CStringsMaps::AddItem(BOOL_AUTO_ENUM,NO,"no");

	CStringsMaps::AddItem(IDENT_METHOD_ENUM,PASSWORD_IDENTIFICATION_METHOD,"password");
	CStringsMaps::AddItem(IDENT_METHOD_ENUM,CALLED_PHONE_NUMBER_IDENTIFICATION_METHOD,"called_phone_number");
	CStringsMaps::AddItem(IDENT_METHOD_ENUM,CALLING_PHONE_NUMBER_IDENTIFICATION_METHOD,"calling_phone_number");


	CStringsMaps::AddItem(NUM_TYPE_ENUM,ACU_NB_TYPE_UNKNOWN,"acu_nb_type_unknown");
	CStringsMaps::AddItem(NUM_TYPE_ENUM,ACU_NB_TYPE_INTERNATIONAL,"acu_nb_type_international");
	CStringsMaps::AddItem(NUM_TYPE_ENUM,ACU_NB_TYPE_NATIONAL,"acu_nb_type_national");
	CStringsMaps::AddItem(NUM_TYPE_ENUM,ACU_NB_TYPE_SUBSCRIBER,"acu_nb_type_subscriber");
	CStringsMaps::AddItem(NUM_TYPE_ENUM,ACU_NB_TYPE_ABBREVIATED,"acu_nb_type_abbreviated");


//	CStringsMaps::AddItem(NUM_TYPE_ENUM,UNKNOWN_TYPE,"unknown");
//	CStringsMaps::AddItem(NUM_TYPE_ENUM,INTERNATIONAL,"international");
//	CStringsMaps::AddItem(NUM_TYPE_ENUM,NATIONAL,"national");
//	CStringsMaps::AddItem(NUM_TYPE_ENUM,NETWORK_SPECIFIC,"network_specific");
//	CStringsMaps::AddItem(NUM_TYPE_ENUM,SUBSCRIBER,"subscriber");
//	CStringsMaps::AddItem(NUM_TYPE_ENUM,ABBREVIATED,"abbreviated");
	CStringsMaps::AddItem(NUM_TYPE_ENUM,NUM_TYPE_DEF,"taken_from_service"); //to see if another default is available


	CStringsMaps::AddItem(DISCONNECT_CODING_TYPE_ENUM,PRIcodCCIT,"ccit");
	CStringsMaps::AddItem(DISCONNECT_CODING_TYPE_ENUM,PRIcodNATIONAL_STD,"national");
	CStringsMaps::AddItem(DISCONNECT_CODING_TYPE_ENUM,PRIcodSTD_SPF_TO_LOC,"specific");
	CStringsMaps::AddItem(DISCONNECT_CODING_TYPE_ENUM,14,"unknown"); // nobody knows why 14, but it comes from CConf

	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,PRIlocUSER,"user");
	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,PRIlocPVT_local,"private_local");
	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,PRIlocPUB_LOCAL,"public_local");
	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,PRIlocTRANSIT_NET,"transit_net");
	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,PRIlocPUB_REMOTE,"public_remote");
	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,PRIlocPVT_REMOTE,"private_remote");
	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,PRIlocINTERNATIONAL,"international");
	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,PRIlocBEY_INTRWORK,"beyond_intrwork");
	CStringsMaps::AddItem(DISCONNECT_LOCATION_TYPE_ENUM,14,"unknown"); // nobody knows why 14, but it comes from CConf

	CStringsMaps::AddMinMaxItem(PHONE_NUMBER_DIGITS_LENGTH,0,PHONE_NUMBER_DIGITS_LEN - 1);
	CStringsMaps::AddMinMaxItem(SERVICE_PHONE_PREFIX_LENGTH,0,SERVICE_PHONE_PREFIX_LEN - 1);

	CStringsMaps::AddMinMaxItem(REMARK_LENGTH,0,CONF_REMARKS_LEN-1);

	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_IDLE,"idle");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_CONNECTED,"connected");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_DISCONNECTED,"disconnected");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_WAITING_FOR_DIAL_IN,"waiting_for_dial_in");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_CONNECTED_WITH_PROBLEM,"connected_with_problem");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_CONNECTING,"connecting");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_DISCONNECTING,"disconnecting");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_CONNECTED_PARTIALY,"connected_partialy");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_DELETED_BY_OPERATOR,"deleted_by_operator");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_SECONDARY,"secondary");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_STAND_BY,"stand_by");
	CStringsMaps::AddItem(ONGOING_PARTY_STATUS_ENUM,PARTY_REDIALING,"redialing");

	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,NO_DISCONNECTION_CAUSE,"None");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,PARTY_HANG_UP,"party_hang_up");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,DISCONNECTED_BY_OPERATOR,"disconnected_by_operator");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,DISCONNECTED_BY_CHAIR,"disconnected_by_chair");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,NO_ESTABL_H243_CONNECT,"no_establ_h243_connect");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,RESOURCES_DEFICIENCY,"resources_deficiency");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,PASSWORD_FAILURE,"password_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,BONDING_FAILURE,"bonding_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,NO_NET_CONNECTION,"no_net_connection");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,NET_PORT_DEFICIENCY,"net_port_deficiency");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,MUX_PORT_DEFICIENCY,"mux_port_deficiency");
/*	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,AUDIO_PORT_DEFICIENCY,"audio_port_deficiency");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,VIDEO_PORT_DEFICIENCY,"video_port_deficiency");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,DATA_PORT_DEFICIENCY,"data_port_deficiency");*/
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,CONF_END_TIME,"conf_end_time");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,AUDIO_MSG_PORT_DEFICIENCY,"audio_msg_port_deficiency");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,V_GATE_NOT_RESPOND,"v_gate_not_respond");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,MCU_INTERNAL_PROBLEM, "mcu_internal_problem");

	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,SLAVE_PARTY_RIGHT_GENERAL_FAILURE, "slave_party_right_mcu_internal_problem");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,SLAVE_PARTY_LEFT_GENERAL_FAILURE, "slave_party_left_mcu_internal_problem");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,SLAVE_PARTY_AUX_GENERAL_FAILURE, "slave_party_aux_mcu_internal_problem");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,SLAVE_PARTY_RIGHT_ALLOCATION_FAILURE, "slave_party_right_allocation_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,SLAVE_PARTY_LEFT_ALLOCATION_FAILURE, "slave_party_left_allocation_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,SLAVE_PARTY_AUX_ALLOCATION_FAILURE, "slave_party_aux_allocation_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,SUB_OR_MAIN_LINK_IS_SECONDARY, "sub_or_main_link_is_secondary");

    /*Begin:added by Richer for BRIDGE-13006,2014.04.29*/
    CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,DISCONNECTED_BY_VIDEO_RECOVERY,"disconnected_by_video_recovery");
    /*End:added by Richer for BRIDGE-13006,2014.04.29*/
    
	//CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_DEFICIENCY,"h323_deficiency");
	//CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,T123_PORT_DEFICIENCY,"t123_port_deficiency");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_NO_PORT_LEFT_FOR_AUDIO,"h323_call_closed_audio_channels_failed_to_open");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEO,"h323_call_closed_video_channels_failed_to_open");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_NO_PORT_LEFT_FOR_FECC,"h323_call_closed_fecc_channels_failed_to_open");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_NO_CONTROL_PORT_LEFT,"h323_call_closed_unable_to_capture_a_control_port_for_the_call");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEOCONT,"h323_call_closed_content_channels_failed_to_open");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_PROBLEM_WITH_CONTENT_CONNECTION_TO_MCU,"IP_call_close_problem_with_content_connection_to_mcu");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR,"IP_call_close_problem_with_content_stream_processing");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_PROBLEM_WITH_ACTIVE_CONTENT_SLAVE,"h323_call_close_Can_not_connect_slave_conference_while_there_is_an_active_content_speaker_please_deactivate_the_content_speaker_and_reconnect_the_link");


		// from 50 ... 70 is SECURITY_FAILURE family commands
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,REMOTE_DEVICE_CAPABILITIES_DO_NOT_SUPPORT_ENCRYPTION,"remote_device_capabilities_do_not_support_encryption");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,FIPS140_STATUS_FAILURE, "fips 140 failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,A_COMMON_KEY_EXCHANGE_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE,"a_common_key_exchange_algorithm_could_not_be_established_between_the_mcu_and_the_remote_device");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE,"a_common_encryption_algorithm_could_not_be_established_between_the_mcu_and_the_remote_device");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,REMOTE_DEVICE_DID_NOT_OPEN_THE_ENCRYPTION_SIGNALING_CHANNEL,"remote_device_did_not_open_the_encryption_signaling_channel");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,THE_REMOTE_DEVICE_KEY_EXCHANGE_ALGORITHM_MESSAGE_WAS_NOT_RECEIVED_BY_THE_MCU,"the_remote_device_key_exchange_algorithm_message_was_not_received_by_the_mcu");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,THE_REMOTE_DEVICE_ENCRYPTION_ALGORITHM_MESSAGE_WAS_NOT_RECEIVED_BY_THE_MCU,"the_remote_device_encryption_algorithm_message_was_not_received_by_the_mcu");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,THE_ENCRYPTION_SETUP_PROCESS_DID_NOT_END_ON_TIME,"the_encryption_setup_process_did_not_end_on_time");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,REMOTE_DEVICE_CANNOT_ENCRYPT,"remote_device_cannot_encrypt");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,THE_REMOTE_DEVICE_FAILED_TO_START_ENCRYPTION_SYSTEM,"the_remote_device_failed_to_start_encryption_system");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,REMOTE_DEVICES_SELECTED_ENCRYPTION_ALGORITHM_DOES_NOT_MATCH_THE_LOCAL_SELECTED_ENCRYPTION_ALGORITHM,"remote_devices_selected_encryption_algorithm_does_not_match_the_local_selected_encryption_algorithm");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,ENCRYPTION_KEY_EXCHANGE_FAILED,"encryption_key_exchange_failed");
//

//// from 141 ... 170 is GATEKEEPER FAILURE family commands
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,CALLED_PARTY_NOT_REGISTERED,"called_party_not_registered");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,INVALID_PERMISSION,"invalid_permission");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,REQUEST_DENIED,"request_denied");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,UNDEFINED_REASON,"undefined_reason");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,CALLER_NOT_REGISTERED,"caller_not_registered");
//// OLD MACRO
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,RSRC_UNAVAILABLE,"resource_unavailable");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,SECURITY_DENIAL,"security_denial");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,QOS_CONTROL_NOT_SUPPORTED,"qos_control_not_supported");
//	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,INCOMPLETE_ADDRESS,"incomplete_address");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_ARQTIMEOUT,"h323_call_closed_arqtimeout");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_DRQTIMEOUT,"h323_call_closed_drqtimeout");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_ALT_GK_FAILURE,"h323_call_closed_alt_gk_failure");
//
//// from 191 ... 220 is H323 FAILURE family commands
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_REMOTE_BUSY,"h323_call_closed_remote_busy");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_NORMAL,"h323_call_closed_normal");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_REMOTE_REJECT,"h323_call_closed_remote_reject");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_REMOTE_UNREACHABLE,"h323_call_closed_remote_unreachable");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_UNKNOWN_REASON,"h323_call_closed_unknown_reason");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_BY_MCU, "call_closed_by_mcu");  // used for h323 and sip.
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_FAULTY_DESTINATION_ADDRESS, "h323_call_faulty_destination_address");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_SMALL_BANDWIDTH,"h323_call_closed_small_bandwidth");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_GATEKEEPER_FAILURE,"h323_call_closed_gatekeeper_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_GATEKEEPER_REJECT_ARQ,"h323_call_closed_gatekeeper_reject_arq");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_NO_PORT_LEFT,"h323_call_closed_no_port_left");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_GATEKEEPER_DRQ,"h323_call_closed_gatekeeper_drq");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_NO_DESTINATION_IP_ADDRESS,"h323_call_closed_no_destination_ip_address");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_REMOTE_HAS_NOT_SENT_CAPABILITY,"h323_call_failed_prior_or_during_the_capabilities_negotiation_stage");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_AUDIO_CHANNELS_NOT_OPEN,"h323_call_closed_audio_channels_didn't_open_before_timeout");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_BAD_REMOTE_CAP,"h323_call_closed_remote_sent_bad_capability");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_CAPS_NOT_ACCPTED_BY_REMOTE,"h323_call_closed_local_capability_wasn't_accepted_by_remote");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_FAILURE,"h323_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_REMOTE_STOP_RESPONDING,"h323_call_closed_remote_stop_responding");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_MASTER_SLAVE_PROBLEM,"h323_call_closed_master_slave_problem");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_MGC_CASCADED_IN_HD_VSW_LPR_ENABLED_CONF,"h323_call_closed_mgc_cascade_disconnected_in_vsw_lpr_enabled_conf");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM,H323_CALL_CLOSED_MEDIA_DISCONNECTED,"h323_call_closed_media_disconnected");


//// Sip close reasons - 251-299
//// 300-700 are saved numbers in the sip protocol.
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_TIMER_POPPED_OUT, "sip_remote_device_did_not_respond_in_the_given_time_frame");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CARD_REJECTED_CHANNELS, "sip_card_rejected_channels");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CAPS_DONT_MATCH, "sip_remote_device_capabilities_are_not_compatible_with_the_conference");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REMOTE_CLOSED_CALL, "sip_remote_device_ended_the_call");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REMOTE_CANCEL_CALL, "sip_remote_device_canceled_the_call");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_BAD_STATUS, "sip_cs_general_error");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REMOTE_STOP_RESPONDING, "sip_remote_device_is_not_responding");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REMOTE_UNREACHABLE, "sip_remote_device_could_not_be_reached");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_TRANSPORT_ERROR, "sip_no_response_from_the_remote_device");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_BAD_NAME, "sip_conference_name_is_incompatible_with_sip");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_TRANS_ERROR_TCP_INVITE, "sip_invite_was_sent_via_tcp_but_the_endpoint_was_not_found");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_INTERNAL_MCU_PROBLEM, "sip_hw_internal_mcu_problem");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_NO_ADDR_FOR_MEDIA, "sip_no_addr_for_media");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_MOVED_PERMANENTLY, "sip_moved_permanently");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_MOVED_TEMPORARILY, "sip_moved_temporarily");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, TIP_VIDEO_BIT_RATE_TOO_LOW, "tip_video_bit_rate_too_low");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, TIP_CREATE_TIMER_POPPED_OUT, "tip_slave_not_created_in_the_given_time_frame");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, TIP_NEGOTIATION_FAILURE, "tip_negotiation_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_INSUFFICIENT_BANDWIDTH, "sip_ice_insufficient_bandwidth");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, WEBRTC_CONNECT_FAILURE, "WebRtc_connect_failure");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, WEBRTC_CONNECT_TOUT, "WebRtc_connect_timeout");

	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REDIRECTION_300, "sip_redirection_300");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REDIRECTION_303, "sip_redirection_303");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REDIRECTION_305, "sip_redirection_305");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REDIRECTION_380, "sip_redirection_380");

	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_400, "sip_client_error_400");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_402, "sip_client_error_402");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_405, "sip_client_error_405");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_406, "sip_client_error_406");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_407, "sip_client_error_407");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_409, "sip_client_error_409");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_411, "sip_client_error_411");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_413, "sip_client_error_413");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_414, "sip_client_error_414");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_420, "sip_client_error_420");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_481, "sip_client_error_481");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_482, "sip_client_error_482");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_483, "sip_client_error_483");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_484, "sip_client_error_484");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_485, "sip_client_error_485");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_488, "sip_client_error_488");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_CLIENT_ERROR_491, "sip_client_error_491");

	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_UNAUTHORIZED, "sip_unauthorized");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_FORBIDDEN, "sip_forbidden");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_NOT_FOUND, "sip_not_found");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REQUEST_TIMEOUT, "sip_request_timeout");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_GONE, "sip_gone");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_BUSY_HERE, "sip_busy_here");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_UNSUPPORTED_MEDIA_TYPE, "sip_unsupported_media_type");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_TEMPORARILY_NOT_AVAILABLE, "sip_temporarily_unavailable");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_REQUEST_TERMINATED, "sip_request_terminated");

	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_SERVER_ERROR_500, "sip_server_error_500");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_SERVER_ERROR_501, "sip_server_error_501");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_SERVER_ERROR_502, "sip_server_error_502");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_SERVER_ERROR_503, "sip_server_error_503");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_SERVER_ERROR_504, "sip_server_error_504");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_SERVER_ERROR_505, "sip_server_error_505");

	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_BUSY_EVERYWHERE, "sip_busy_everywhere");

	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_GLOBAL_FAILURE_603, "sip_global_failure_603");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_GLOBAL_FAILURE_604, "sip_global_failure_604");
	CStringsMaps::AddItem(DISCONNECTION_CAUSE_ENUM, SIP_GLOBAL_FAILURE_606, "sip_global_failure_606");
	CStringsMaps::AddItem(RECORDING_PORT_ENUM, NO, "no");
	CStringsMaps::AddItem(RECORDING_PORT_ENUM, DEFAULT_RECORDING_TYPE, "default");

	// Secondary causes
	//strcpy(strName,"SECONDARY_CAUSE_ENUM");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_DEFAULT,"default");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_REMOTE_CAPABILITIES,"remote_capabilities");
	//CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_CARD_RESOURCES,"card_resources");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_CONFERENCE_REJECT,"conference_reject");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_CHANGE_MODE,"change_mode");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_STREAM_VIOLATION,"stream_violation");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_VIDEO_PROBLEM,"video_problem");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_MOVE_PARTY,"move_party");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_NO_VIDEO_CONNECTION,"no_video_connection");
	//CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_MOVE_DIFF_BITRATE,"move_diff_bitrate");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_RMT_CLOSE_CHAN,"rmt_close_chan");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_RMT_DIFF_CAPCODE,"rmt_diff_capcode");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_RMT_NOT_OPEN_AFTER_CHANGE_MODE,"rmt_not_open_after_change_mode");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH,"gk_returned_small_bandwidth");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_CONFERENCING_LIMITATION,"conferencing_limitaion");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_AVF_INSUFFICIENT_BANDWIDTH, "ACM_returned_only_audio_bandwidth");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_H239_BW_MISMATCH, "the content channel was not open because the remote endpoint does not support the conference rate in cascaded conference");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_H239_INCOMPATIBLE_CAPS, "the content channel was not open because the remote endpoint does not support the conference content protocol's parameters");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_H239_RMT_DIFF_CAPCODE, "content media was not establish because the remote endpoint does not support the conference content protocol");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_H239_CONFERENCE_REJECT, "content media was not establish because the conference rejected the content connection");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD, "Content channel disabled because participant Content rate is below minimum required threshold");
	CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD, "Content channel disabled because participant Content resolution is below minimum required threshold");


	CStringsMaps::AddItem(MAX_RTV_RESOLUTION_ENUM,eMaxRtvResolutionAuto, "AUTO");
	CStringsMaps::AddItem(MAX_RTV_RESOLUTION_ENUM,eMaxRtvResolutionHD720, "HD720");
	CStringsMaps::AddItem(MAX_RTV_RESOLUTION_ENUM,eMaxRtvResolutionVGA, "VGA");
	CStringsMaps::AddItem(MAX_RTV_RESOLUTION_ENUM,eMaxRtvResolutionCIF, "CIF");
	CStringsMaps::AddItem(MAX_RTV_RESOLUTION_ENUM,eMaxRtvResolutionQCIF, "QCIF");

	CStringsMaps::AddItem(MAX_MSSVC_RESOLUTION_ENUM,eMaxMsSvcResolutionAuto, "AUTO");
	CStringsMaps::AddItem(MAX_MSSVC_RESOLUTION_ENUM,eMaxMsSvcResolutionHD1080, "HD1080");
	CStringsMaps::AddItem(MAX_MSSVC_RESOLUTION_ENUM,eMaxMsSvcResolutionHD720, "HD720");
	CStringsMaps::AddItem(MAX_MSSVC_RESOLUTION_ENUM,eMaxMsSvcResolutionVGA, "VGA");
	CStringsMaps::AddItem(MAX_MSSVC_RESOLUTION_ENUM,eMaxMsSvcResolutionCIF, "CIF");

	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC_ENUM,eMsClientAudioCodecAuto, "AUTO");
	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC_ENUM,eMsClientAudioCodecG711A, "G711A");
	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC_ENUM,eMsClientAudioCodecG711U, "G711U");
	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC_ENUM,eMsClientAudioCodecG722, "G722");
	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC_ENUM,eMsClientAudioCodecG7231, "G7231");
	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC_ENUM,eMsClientAudioCodecG7221_24, "G7221_24");

	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC__FOR_SINGLE_CORE_ENUM,eMsClientAudioCodecAuto, "AUTO");
	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC__FOR_SINGLE_CORE_ENUM,eMsClientAudioCodecG711A, "G711A");
	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC__FOR_SINGLE_CORE_ENUM,eMsClientAudioCodecG711U, "G711U");
	CStringsMaps::AddItem(MS_CLIENT_AUDIO_CODEC__FOR_SINGLE_CORE_ENUM,eMsClientAudioCodecG722, "G722");

	CStringsMaps::AddItem(ENCODE_RTV_B_FRAME_ENUM,eEncodeRtvBFrameNever, "NEVER");
	CStringsMaps::AddItem(ENCODE_RTV_B_FRAME_ENUM,eEncodeRtvBFrameAvmcuOnly, "AVMCU_ONLY");
	CStringsMaps::AddItem(ENCODE_RTV_B_FRAME_ENUM,eEncodeRtvBFrameAlways, "ALWAYS");

	CStringsMaps::AddItem(MS_FEC_ENUM,eMsFECAuto, "AUTO");
	CStringsMaps::AddItem(MS_FEC_ENUM,eMsFECDV00, "DV00");
	CStringsMaps::AddItem(MS_FEC_ENUM,eMsFECDV01, "DV01");
	CStringsMaps::AddItem(MS_FEC_ENUM,eMsFECNo, "NO");


	//Alternative Network Address Types
	CStringsMaps::AddItem(ANAT_IP_PROTOCOL_ENUM,eAnatDisabled, "DISABLED");
	CStringsMaps::AddItem(ANAT_IP_PROTOCOL_ENUM,eAnatAuto, "AUTO");
	CStringsMaps::AddItem(ANAT_IP_PROTOCOL_ENUM,eAnatPreferIpV4, "PREFER_IPv4");
	CStringsMaps::AddItem(ANAT_IP_PROTOCOL_ENUM,eAnatPreferIpV6, "PREFER_IPv6");


	//CStringsMaps::AddItem(SECONDARY_CAUSE_ENUM,OTHER_CAUSE,"other");

	CRtmIsdnMngrCommonMethods rtmIsdnMngr;
	static string country[MAX_NUM_OF_COUNTRY_CODES];
	for(int i = 0 ; i < MAX_NUM_OF_COUNTRY_CODES; ++i)
	{
		country[i] = rtmIsdnMngr.CountryIndexToString(i);
	}

	for(int i = 0 ; i < MAX_NUM_OF_COUNTRY_CODES; ++i)
	{
		CStringsMaps::AddItem(COUNTRY_CODE_EXTERNAL_ENUM, i, country[i].c_str());
	}

	CStringsMaps::AddItem(CRL_MODE_802_1X_ENUM, 0, "ENABLED");
	CStringsMaps::AddItem(CRL_MODE_802_1X_ENUM, 1, "OPTIONAL");
	CStringsMaps::AddItem(CRL_MODE_802_1X_ENUM, 2, "DISABLED");


	CStringsMaps::AddItem(CERTIFICATE_MODE_802_1X_ENUM, 0, "ONE_CERTIFICATE");
	CStringsMaps::AddItem(CERTIFICATE_MODE_802_1X_ENUM, 1, "MULTIPLE_CERTIFICATE");

	CStringsMaps::AddItem(HD_RATES_ENUM, 0, "128");
	CStringsMaps::AddItem(HD_RATES_ENUM, 1, "192");
	CStringsMaps::AddItem(HD_RATES_ENUM, 2, "320");
	CStringsMaps::AddItem(HD_RATES_ENUM, 3, "384");
	CStringsMaps::AddItem(HD_RATES_ENUM, 4, "512");
	CStringsMaps::AddItem(HD_RATES_ENUM, 5, "768");
	CStringsMaps::AddItem(HD_RATES_ENUM, 6, "832");
	CStringsMaps::AddItem(HD_RATES_ENUM, 7, "1024");
	CStringsMaps::AddItem(HD_RATES_ENUM, 8, "1152");
	CStringsMaps::AddItem(HD_RATES_ENUM, 9, "1280");
	CStringsMaps::AddItem(HD_RATES_ENUM, 10, "1472");
	CStringsMaps::AddItem(HD_RATES_ENUM, 11, "1536");
	CStringsMaps::AddItem(HD_RATES_ENUM, 12, "1728");
	CStringsMaps::AddItem(HD_RATES_ENUM, 13, "1920");
	CStringsMaps::AddItem(HD_RATES_ENUM, 14, "2048");
	CStringsMaps::AddItem(HD_RATES_ENUM, 15, "2560");
	CStringsMaps::AddItem(HD_RATES_ENUM, 16, "3072");
	CStringsMaps::AddItem(HD_RATES_ENUM, 17, "3584");
	CStringsMaps::AddItem(HD_RATES_ENUM, 18, "4096");
	CStringsMaps::AddItem(HD_RATES_ENUM, 19, "6144");

	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 0, "64");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 1, "128");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 2, "192");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 3, "256");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 4, "384");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 5, "512");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 6, "768");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 7, "1024");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 8, "1536");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM, 9, "2048");
	CStringsMaps::AddItem(CONTENT_RATES_ENUM,10, "3072");

	InitFaultStrings();
	InitCDRStrings();

	CStringsMaps::AddMinMaxItem(_0_TO_MAX_VLAN_PRIORITY, 0	,7);

	CStringsMaps::AddItem(UNIT_TYPE_ENUM, UNIT_TYPE_PARTICIPANT, "participant");
	CStringsMaps::AddItem(UNIT_TYPE_ENUM, UNIT_TYPE_BRIDGE, "bridge");
	CStringsMaps::AddItem(UNIT_TYPE_ENUM, UNIT_TYPE_CONTROLLER, "controller");
	CStringsMaps::AddItem(UNIT_TYPE_ENUM, UNIT_TYPE_ART, "art");
	CStringsMaps::AddItem(UNIT_TYPE_ENUM, UNIT_TYPE_VIDEO, "video");
//	CStringsMaps::AddItem(UNIT_TYPE_ENUM, UNIT_TYPE_PQ, "media_cpu");
	CStringsMaps::AddItem(UNIT_TYPE_ENUM, UNIT_TYPE_SMART, "smart");

	CStringsMaps::AddMinMaxItem(_0_TO_MPL_SERIAL_NUM_LENGTH,0,MPL_SERIAL_NUM_LEN - 1);

        CStringsMaps::AddMinMaxItem(_QOS_IP_VIDEO_RANGE_DECIMAL, 0, MAX_PREDECEDENCE_AUDIO);
        CStringsMaps::AddMinMaxItem(_QOS_IP_AUDIO_RANGE_DECIMAL, 0, MAX_PREDECEDENCE_VIDEO);

	CStringsMaps::AddMinMaxItem(_1_TO_LOGGER_FILE_MAX_NAME_LEN, 1, LOGGER_FILE_MAX_NAME_LEN-1);
	CStringsMaps::AddMinMaxItem(_2_TO_MAX_LOG_SIZE,2,MAX_LOG_SIZE);

	CStringsMaps::AddMinMaxItem(_0_TO_MAX_GK_NAME_ENUM, 0, GATEKEEPER_NAME_LEN);

	CStringsMaps::AddItem(COMPRESSION_CODE_ENUM,COMPRESSION_CODE_NONE,"none");
	CStringsMaps::AddItem(COMPRESSION_CODE_ENUM,COMPRESSION_CODE_ZLIB,"zlib");

	//Ad hoc conference duration
	CStringsMaps::AddItem(_CHANGE_AD_HOC_CONF_DURATION, 60 , "60" );
	CStringsMaps::AddItem(_CHANGE_AD_HOC_CONF_DURATION, 90 , "90" );
	CStringsMaps::AddItem(_CHANGE_AD_HOC_CONF_DURATION, 180, "180");
	CStringsMaps::AddItem(_CHANGE_AD_HOC_CONF_DURATION, 270, "270");

	CStringsMaps::AddMinMaxItem(_0_TO_USER_IDENTIFIER_STRING_LENGTH,0,USER_IDENTIFIER_STRING_LEN);

	CStringsMaps::AddItem(LANGUAGES_ENUM, eEnglish, "english");
	CStringsMaps::AddItem(LANGUAGES_ENUM, eGerman, "german");
	CStringsMaps::AddItem(LANGUAGES_ENUM, eSpanishSA, "spanish_south_america");
	CStringsMaps::AddItem(LANGUAGES_ENUM, eFrench, "french");
	CStringsMaps::AddItem(LANGUAGES_ENUM, eJapanese, "japanese");
	CStringsMaps::AddItem(LANGUAGES_ENUM, eKorean, "korean");
	CStringsMaps::AddItem(LANGUAGES_ENUM, eChineseSimpl, "chinese_simplified");

	CStringsMaps::AddItem(MCU_MODE_ENUM,	0,	"full_transcoding");
	CStringsMaps::AddItem(MCU_MODE_ENUM,	1,	"event");

	CStringsMaps::AddItem(DIRECTION_TYPE_ENUM,0,"in");
	CStringsMaps::AddItem(DIRECTION_TYPE_ENUM,1,"out");

	//////////failover current status type
	CStringsMaps::AddItem(FAILOVER_MASTER_SLAVE_STATE,eMasterSlaveNone,"none");
	CStringsMaps::AddItem(FAILOVER_MASTER_SLAVE_STATE,eSlaveState,"slave");
	CStringsMaps::AddItem(FAILOVER_MASTER_SLAVE_STATE,eMasterConfigurationState,"master_configuration");
	CStringsMaps::AddItem(FAILOVER_MASTER_SLAVE_STATE,eMasterActualState,"master_actual");

	//////////Sip server type
	CStringsMaps::AddItem(SIP_SERVER_TYPE_ENUM, eSipServer_generic,"generic");
	CStringsMaps::AddItem(SIP_SERVER_TYPE_ENUM, eSipServer_ms,"ms_ocs");

	//////////ldap authentication types & ports
	CStringsMaps::AddItem(LDAP_DIR_TYPE_ENUM,eMsActiveDirectory,"ms_active_directory");

	CStringsMaps::AddItem(LDAP_DIR_PORT_ENUM,e389,"389");
	CStringsMaps::AddItem(LDAP_DIR_PORT_ENUM,e636,"636");

	CStringsMaps::AddItem(LDAP_AUTHENTICATION_TYPE_ENUM,ePlain,"plain");
	CStringsMaps::AddItem(LDAP_AUTHENTICATION_TYPE_ENUM,eNTLM,"ntlm");
	CStringsMaps::AddItem(LDAP_AUTHENTICATION_TYPE_ENUM,eKerberos,"kerberos");	//currently not in use

	CStringsMaps::AddMinMaxItem(_5_TO_1440_DECIMAL,5,1440);

	CStringsMaps::AddItem(SITE_NAME_DISPLAY_MODE_ENUM,eSiteNameAuto,"Auto");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_MODE_ENUM,eSiteNameOn,"On");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_MODE_ENUM,eSiteNameOff,"Off");

	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eLeft_top_position,"Left Top");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eTop_position,"Top");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eRight_top_position,"Right Top");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eLeft_middle_position,"Left Middle");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eRight_middle_position,"Right Middle");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eLeft_bottom_position,"Left Bottom");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eBottom_position,"Bottom");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eRight_bottom_position,"Right Bottom");
	CStringsMaps::AddItem(SITE_NAME_DISPLAY_POSITION_ENUM,eCustom,"Custom");

	CStringsMaps::AddMinMaxItem(_0_TO_SITE_NAME_FONT_SIZE, 9, 32);
	CStringsMaps::AddMinMaxItem(_0_TO_SITE_NAME_TRANCEPARENCE, 0,100);
	CStringsMaps::AddMinMaxItem(_0_TO_SITE_NAME_CUSTOM_POSITION, 0, 100);

	CStringsMaps::AddItem(ICON_LOCATION_ENUM,0,"TOP");
	CStringsMaps::AddItem(ICON_LOCATION_ENUM,1,"BOTTOM");
	CStringsMaps::AddItem(ICON_LOCATION_ENUM,2,"LEFT");
	CStringsMaps::AddItem(ICON_LOCATION_ENUM,3,"RIGHT");
	CStringsMaps::AddItem(ICON_LOCATION_ENUM,4,"TOP_LEFT");
	CStringsMaps::AddItem(ICON_LOCATION_ENUM,5,"TOP_RIGHT");
	CStringsMaps::AddItem(ICON_LOCATION_ENUM,6,"BOTTOM_LEFT");
	CStringsMaps::AddItem(ICON_LOCATION_ENUM,7,"BOTTOM_RIGHT");

	CStringsMaps::AddItem(SPEAKER_CHANGE_THRESHOLD_ENUM, eAutoTicks, "Auto");
	CStringsMaps::AddItem(SPEAKER_CHANGE_THRESHOLD_ENUM, e150Ticks, "150");
	CStringsMaps::AddItem(SPEAKER_CHANGE_THRESHOLD_ENUM, e300Ticks, "300");
	CStringsMaps::AddItem(SPEAKER_CHANGE_THRESHOLD_ENUM, e500Ticks, "500");

	CStringsMaps::AddItem(SIP_ENCRYPTION_KEY_EXCHANGE_MODE_ENUM, 0, "AUTO");
	CStringsMaps::AddItem(SIP_ENCRYPTION_KEY_EXCHANGE_MODE_ENUM, 1, "DTLS");
	CStringsMaps::AddItem(SIP_ENCRYPTION_KEY_EXCHANGE_MODE_ENUM, 2, "SDES");
	CStringsMaps::AddItem(SIP_ENCRYPTION_KEY_EXCHANGE_MODE_ENUM, 3, "NONE");

	CStringsMaps::AddMinMaxItem(_3_TO_300_DECIMAL,3,300);
	
	//eFeatureRssDialin
	CStringsMaps::AddItem(SRS_PLAYBACK_LAYOUT_MODE_ENUM,eSrsAutoMode,"Auto");
	CStringsMaps::AddItem(SRS_PLAYBACK_LAYOUT_MODE_ENUM,eSrsLectureMode,"Lecture");

	CStringsMaps::AddItem(FOLLOW_SPEAKER_ON_1X1_ENUM,AUTO, "AUTO");
	CStringsMaps::AddItem(FOLLOW_SPEAKER_ON_1X1_ENUM,YES, "YES");
	CStringsMaps::AddItem(FOLLOW_SPEAKER_ON_1X1_ENUM,NO, "NO");

	CStringsMaps::AddItem(IVR_SLIDE_CONVERSION_METHOD_ENUM, eIvrSlideLowRes, "low_res");
	CStringsMaps::AddItem(IVR_SLIDE_CONVERSION_METHOD_ENUM, eIvrSlideHighRes, "high_res");
	CStringsMaps::AddItem(IVR_SLIDE_CONVERSION_METHOD_ENUM, eIvrSlideLowHighRes, "low_high_res");

	CStringsMaps::AddItem(IVR_SLIDE_IMAGE_TYPE_ENUM, eIvrSlideImageJpg, "jpg");
	CStringsMaps::AddItem(IVR_SLIDE_IMAGE_TYPE_ENUM, eIvrSlideImageBmp, "bmp");
}

void InitCDRStrings()
{
	for (eConfCdrStatus en = DEFAULT_STATUS; en < NumOfConferenceStatuses; en = (eConfCdrStatus)(en + 1))
	{
		const char* name = GetConfCdrStatusName(en);
		CStringsMaps::AddItem(CDR_STATUS_ENUM, en, name);
		CStringsMaps::AddItem(CONF_END_CAUSE_TYPE_ENUM, en, name);
	}

	CStringsMaps::AddMinMaxItem(_0_TO_MAX_DOS_FILE_NAME_LENGTH,0,MAX_DOS_FILE_NAME_LEN-1);

	CStringsMaps::AddItem(RESTRICT_MODE_ENUM,27,"restricted");
	CStringsMaps::AddItem(RESTRICT_MODE_ENUM,28,"derestricted");
	CStringsMaps::AddItem(RESTRICT_MODE_ENUM,255,"auto");

	//strcpy(strName,"LSD_RATE_ENUM");
	CStringsMaps::AddItem(LSD_RATE_ENUM,AUTO,"dynamic");
	CStringsMaps::AddItem(LSD_RATE_ENUM,LSD_6400,"6400");
	CStringsMaps::AddItem(LSD_RATE_ENUM,NO,"none");

	CStringsMaps::AddItem(T120_RATE_ENUM,0,"none");
	CStringsMaps::AddItem(T120_RATE_ENUM,2,"hmlp_62.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,3,"hmlp_64");
	CStringsMaps::AddItem(T120_RATE_ENUM,4,"hmlp_128");
	CStringsMaps::AddItem(T120_RATE_ENUM,5,"hmlp_192");
	CStringsMaps::AddItem(T120_RATE_ENUM,6,"hmlp_256");
	CStringsMaps::AddItem(T120_RATE_ENUM,7,"hmlp_320");
	CStringsMaps::AddItem(T120_RATE_ENUM,8,"hmlp_384");
	CStringsMaps::AddItem(T120_RATE_ENUM,13,"hmlp_var");
	CStringsMaps::AddItem(T120_RATE_ENUM,12,"hmlp_14.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,30,"mlp_64");
	CStringsMaps::AddItem(T120_RATE_ENUM,29,"mlp_62.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,24,"mlp_46.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,28,"mlp_40");
	CStringsMaps::AddItem(T120_RATE_ENUM,23,"mlp_38.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,27,"mlp_32");
	CStringsMaps::AddItem(T120_RATE_ENUM,22,"mlp_30.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,26,"mlp_24");
	CStringsMaps::AddItem(T120_RATE_ENUM,21,"mlp_22.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,25,"mlp_16");
	CStringsMaps::AddItem(T120_RATE_ENUM,20,"mlp_14.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,18,"mlp_6.4");
	CStringsMaps::AddItem(T120_RATE_ENUM,17,"mlp_4");
	CStringsMaps::AddItem(T120_RATE_ENUM,19,"mlp_var");

	CStringsMaps::AddItem(CHAIR_MODE_ENUM,AUTO,"auto");
	CStringsMaps::AddItem(CHAIR_MODE_ENUM,FALSE,"none");

	CStringsMaps::AddMinMaxItem(_0_TO_CONF_INFO_ITEM_LENGTH,0,CONF_INFO_ITEM_LEN - 1);

	CStringsMaps::AddItem(NUM_PLAN_TYPE_ENUM,ISDN,"isdn");
	CStringsMaps::AddItem(NUM_PLAN_TYPE_ENUM,TELEPHONY,"telephony");
	CStringsMaps::AddItem(NUM_PLAN_TYPE_ENUM,PRIVATE,"private");
	CStringsMaps::AddItem(NUM_PLAN_TYPE_ENUM,UNKNOWN_PLAN,"unknown");

	CStringsMaps::AddItem(PRESENTATION_INDICATOR_TYPE_ENUM,ALLOWED,"allowed");
	CStringsMaps::AddItem(PRESENTATION_INDICATOR_TYPE_ENUM,RESTRICTED,"restricted");
	CStringsMaps::AddItem(PRESENTATION_INDICATOR_TYPE_ENUM,NOT_AVAILABLE,"not_available");
	CStringsMaps::AddItem(PRESENTATION_INDICATOR_TYPE_ENUM,0xFF,"unknown");

	CStringsMaps::AddItem(SCREEN_INDICATOR_TYPE_ENUM,NOT_SCREENED,"not_screened");
	CStringsMaps::AddItem(SCREEN_INDICATOR_TYPE_ENUM,USER_VER_PASSED,"user_ver_passed");
	CStringsMaps::AddItem(SCREEN_INDICATOR_TYPE_ENUM,USER_VER_FAILED,"user_ver_failed");
	CStringsMaps::AddItem(SCREEN_INDICATOR_TYPE_ENUM,0xFF,"unknown");

	CStringsMaps::AddMinMaxItem(PRI_LIMIT_PHONE_DIGITS_LENGTH,0,PRI_LIMIT_PHONE_DIGITS_LEN - 1);

	CStringsMaps::AddItem(INITIATOR_ENUM,MCU_INITIATOR,"mcu");
	CStringsMaps::AddItem(INITIATOR_ENUM,REMOTE_PARTY_INITIATOR,"party");

	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,NONE,"none");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,ATT_SDN,"att_sdn");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,NTI_PRIVATE,"nti_private");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,ATI_MEGACOM,"ati_megacom");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,NTI_OUTWATS,"nti_outwats");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,NTI_FX,"nti_fx");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,NTI_TIE_TRUNK,"nti_tie_trunk");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,ATT_ACCUNET,"att_accunet");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,ATT_1800,"att_1800");
	CStringsMaps::AddItem(NET_SPECIFIC_TYPE_ENUM,NTI_TRO,"nti_tro");

	CStringsMaps::AddItem(PREFERRED_TYPE_ENUM,NO_PRF,"none");
	CStringsMaps::AddItem(PREFERRED_TYPE_ENUM,PRF_MODE_PREFERRED,"preferred");
	CStringsMaps::AddItem(PREFERRED_TYPE_ENUM,PRF_MODE_EXCLUSIVE,"exclusive");

//	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,VOICE,"voice");
	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,MODEM,"modem");//temporary - to see why we got 2(MODEM) and to change to ACU_MODEM_SERVICE
//	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,CALL_TYPE_56K,"56k");
//	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,CALL_TYPE_64K,"64k");
//	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,CALL_TYPE_64K_RESTRICT,"64k_restrict");
//	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,CALL_TYPE_384K,"384k");
//	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,CALL_TYPE_384K_RESTRICT,"384k_restrict");

	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,ACU_VOICE_SERVICE,"acu_voice_service\0");
	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,ACU_MODEM_SERVICE,"acu_modem_service");
	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,ACU_DATA_56KBS_SERVICE,"acu_data_56kbs_service");
	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,ACU_DATA_SERVICE,"acu_data_service");
	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,ACU_DATA_H0_SERVICE,"acu_data_h0_service");
	CStringsMaps::AddItem(CHANNEL_CALL_TYPE_ENUM,ACU_DATA_H11_SERVICE,"acu_data_h11_service");

	CStringsMaps::AddItem(END_POINT_TYPE_ENUM,0,"terminal");
	CStringsMaps::AddItem(END_POINT_TYPE_ENUM,1,"gateway");
	CStringsMaps::AddItem(END_POINT_TYPE_ENUM,2,"mcu");
	CStringsMaps::AddItem(END_POINT_TYPE_ENUM,3,"gatekeeper");
	CStringsMaps::AddItem(END_POINT_TYPE_ENUM,4,"unknown");

	CStringsMaps::AddItem(INITIATOR_TYPE_ENUM,MCU_INITIATOR,"mcu");
	CStringsMaps::AddItem(INITIATOR_TYPE_ENUM,REMOTE_PARTY_INITIATOR,"party");

	CStringsMaps::AddMinMaxItem(_0_TO_IP_LIMIT_ADDRESS_CHAR_LENGTH,0,IP_LIMIT_ADDRESS_CHAR_LEN-1);

	CStringsMaps::AddItem(BONDING_ENUM,0xFF,"auto");
	CStringsMaps::AddItem(BONDING_ENUM,1,"enabled");
	CStringsMaps::AddItem(BONDING_ENUM,0,"disabled");

	CStringsMaps::AddItem(MULTI_RATE_ENUM,0xFF,"auto");
	CStringsMaps::AddItem(MULTI_RATE_ENUM,1,"enabled");
	CStringsMaps::AddItem(MULTI_RATE_ENUM,0,"disabled");

	CStringsMaps::AddItem(DTMF_FAILURE_ENUM,IVR_FEATURE_CONF_PASSWORD,"ivr_conf_password");
	CStringsMaps::AddItem(DTMF_FAILURE_ENUM,IVR_FEATURE_CONF_LEADER,"ivr_conf_leader");
	CStringsMaps::AddItem(DTMF_FAILURE_ENUM,IVR_FEATURE_NUMERIC_CONFERENCE_ID,"ivr_numeric_conf_id");

	CStringsMaps::AddItem(START_REC_POLICY_ENUM,START_RECORDING_IMMEDIATELY,"immediately");
	CStringsMaps::AddItem(START_REC_POLICY_ENUM,START_RECORDING_UPON_REQUEST,"upon_request");

	CStringsMaps::AddItem(RECORDING_STATUS_ENUM,eStopRecording,"stop");
	CStringsMaps::AddItem(RECORDING_STATUS_ENUM,eStartRecording,"start");
	CStringsMaps::AddItem(RECORDING_STATUS_ENUM,ePauseRecording,"pause");
	CStringsMaps::AddItem(RECORDING_STATUS_ENUM,eResumeRecording,"resume");

	CStringsMaps::AddMinMaxItem(_0_TO_MAX_SIP_PRIVATE_EXT_LENGTH,1,MAX_SIP_PRIVATE_EXT_LEN- 1);
	CStringsMaps::AddMinMaxItem(_0_TO_31_STRING_LENGTH,0,31);

}

void InitFaultStrings()
{
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_STARTUP_SUBJECT,"startup");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_ASSERT_SUBJECT,"assert");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_FILE_SUBJECT,"file");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_CARD_SUBJECT,"card");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_RESERVATION_SUBJECT,"reservation");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_MEETING_ROOM_SUBJECT,"meeting_room");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_CONFERENCE_SUBJECT,"conference");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_GENERAL_SUBJECT,"general");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_EXCEPTION_SUBJECT,"exception");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_DONGLE_SUBJECT,"dongle");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_UNIT_SUBJECT,"unit");
	CStringsMaps::AddItem(FAULT_SUBJECT_ENUM, FAULT_MPL_SUBJECT,"mpl");

	CStringsMaps::AddItem(FAULT_LEVEL_ENUM,MAJOR_ERROR_LEVEL,"major");
//	CStringsMaps::AddItem(FAULT_LEVEL_ENUM,MINOR_ERROR_LEVEL,"minor");
	CStringsMaps::AddItem(FAULT_LEVEL_ENUM,STARTUP_ERROR_LEVEL,"startup");
	CStringsMaps::AddItem(FAULT_LEVEL_ENUM,SYSTEM_MESSAGE,"system_message");

	for(eProcessType iter = eProcessTypeInvalid ; iter < NUM_OF_PROCESS_TYPES ; iter = eProcessType(iter + 1))
	{
		CStringsMaps::AddItem(PROCESS_NAME_ENUM, iter, ::ProcessTypeToString(iter) );
	}
	//CStringsMaps::AddItem(PROCESS_NAME_ENUM, PROCESS_COLLECTOR,				::ProcessTypeToString(eProcessCollector) );

	//strcpy(strName,"FAULT_FILE_ENUM");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,VERSION_CONFIGURATION,"version_configuration");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,CARDS_CONFIGURATION,"cards_configuration");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,NETWORK_CONFIGURATION,"network_configuration");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,OPERATORS_CONFIGURATION,"operators_configuration");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,SYSTEM_CONFIGURATION,"system_configuration");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,LOGGING,"logging");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,RESERVATION_DATABASE,"reservation_database");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,MESSAGE_CONFIGURATION,"message_configuration");
	CStringsMaps::AddItem(FAULT_FILE_ENUM,MEETING_ROOM_DATABASE,"meeting_room_database");

	//strcpy(strName,"BOARD_ENUM");
	CStringsMaps::AddMinMaxItem(BOARD_ENUM,     0, MAX_NUM_OF_BOARDS);

	//strcpy(strName,"UNIT_ENUM");
	CStringsMaps::AddMinMaxItem(UNIT_ENUM,      0, MAX_NUM_OF_UNITS-1);

	CStringsMaps::AddMinMaxItem(SUB_BOARD_ENUM, 0, MAX_NUM_OF_SUBBOARDS);

	//strcpy(strName,"1_TO_FILE_NAME_LEN"); // 12 char + terminator
	CStringsMaps::AddMinMaxItem(_1_TO_FILE_NAME_LENGTH,1,FILE_NAME_LEN - 1);

	//strcpy(strName,"1_TO_EXCEPT_HANDL_MES_LEN");
	CStringsMaps::AddMinMaxItem(_1_TO_EXCEPT_HANDL_MES_LENGTH,1,EXCEPT_HANDL_MES_LEN - 1);

	//strcpy(strName,"0_TO_H243_NAME_LEN");
	CStringsMaps::AddMinMaxItem(_0_TO_H243_NAME_LENGTH,0,H243_NAME_LEN - 1);

	CStringsMaps::AddMinMaxItem(_0_TO_MESSAGE_OVERLAY_TEXT_LENGTH, 0, 150);//50 Unicode characters

	CStringsMaps::AddMinMaxItem(_0_TO_MESSAGE_OVERLAY_NUM_OF_REPETITIONS, 1, 20);

	CStringsMaps::AddMinMaxItem(_0_TO_MESSAGE_OVERLAY_FONT_SIZE, 9, 32);
	CStringsMaps::AddMinMaxItem(_0_TO_MESSAGE_OVERLAY_DISPLAY_POSITION, 0, 100);
	CStringsMaps::AddMinMaxItem(_0_TO_MESSAGE_OVERLAY_TRANSPARENCE, 0, 100);

	//strcpy(strName,"1_TO_H243_NAME_LEN");
	CStringsMaps::AddMinMaxItem(_1_TO_H243_NAME_LENGTH,1,H243_NAME_LEN - 1);

	CStringsMaps::AddMinMaxItem(_1_TO_SIZE_OF_CALL_ID_LENGTH, 1, SIZE_OF_CALL_ID - 1);

	//strcpy(strName,"0_TO_GENERAL_MES_LEN");
	CStringsMaps::AddMinMaxItem(_0_TO_GENERAL_MES_LENGTH,0,GENERAL_MES_LEN - 1);

	CStringsMaps::AddMinMaxItem(_0_TO_DONGLE_SERIAL_NUM_LENGTH,0,DONGLE_SERIAL_NUM_LEN);
	CStringsMaps::AddMinMaxItem(_0_TO_SNMP_STRING_LENGTH,0,SNMP_STRING_LEN);
	CStringsMaps::AddMinMaxItem(_0_TO_FAILOVER_STRING_LENGTH,0,FAILOVER_STRING_LEN);
	CStringsMaps::AddMinMaxItem(_0_TO_VALIDATION_STRING_LENGTH,0,VALIDATION_STRING_LEN);
	CStringsMaps::AddMinMaxItem(_0_TO_CONFIGURATION_NAME_LENGTH,0,CONFIGURATION_NAME_LEN-1);
	CStringsMaps::AddMinMaxItem(_0_TO_VER_NUM_LENGTH,0,VER_NUM_LEN-1);
	CStringsMaps::AddMinMaxItem(_0_TO_KEYCODE_LENGTH,0,KEYCODE_LENGTH-1);

	CStringsMaps::AddItem(ACTION_TYPE_ENUM,SET_REQUEST,"set_request");
	CStringsMaps::AddItem(ACTION_TYPE_ENUM,GET_REQUEST,"get_request");
	CStringsMaps::AddItem(ACTION_TYPE_ENUM,UNKNOWN_REQUEST,"unknown_request");

	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_MPM,			"fault_type_mpm");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_RTM_LAN,		"fault_type_rtm_lan");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_SWITCH,		"fault_type_switch");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_ART,			"fault_type_art");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_VIDEO,		"fault_type_video");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_PQ,			"fault_type_media_cpu");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_MCMS_CPU,		"fault_type_control_cpu");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_FAN,			"fault_type_fan");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_PWR_SPLY,		"fault_type_power_supply");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_LAN,			"fault_type_lan");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_BACKPLANE,	"fault_type_backplane");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_ILLEGAL,		"fault_type_illegal");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_ART_CNTRLR,	"fault_type_ArtController");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_IAM,			"fault_type_Iam");
	CStringsMaps::AddItem(FAULT_TYPE_ENUM, FAULT_TYPE_FSM4000,		"fault_type_Fsm4000");

	CStringsMaps::AddItem(DUAL_VIDEO_MODE_ENUM,eDualModeNone,"none");
	CStringsMaps::AddItem(DUAL_VIDEO_MODE_ENUM,eDualModePeopleContent,"DualModePeopleContent");
	CStringsMaps::AddItem(DUAL_VIDEO_MODE_ENUM,eDualModeIsdnDuoVideo,"DualModeIsdnDuoVideo");
	CStringsMaps::AddItem(DUAL_VIDEO_MODE_ENUM,eDualModeVisualConcertPC,"DualModeVisualConcertPC");
	CStringsMaps::AddItem(DUAL_VIDEO_MODE_ENUM,eDualModeVisualConcertFX,"DualModeVisualConcertFX");
	CStringsMaps::AddItem(DUAL_VIDEO_MODE_ENUM,eDualModeEnterprisePC,"DualModeEnterprisePC");
	CStringsMaps::AddItem(DUAL_VIDEO_MODE_ENUM,eDualModeIpDuoVideo,"DualModeIpDuoVideo");

	CStringsMaps::AddItem(ENTERPRISE_MODE_ENUM,eGraphics,"Graphics");
	CStringsMaps::AddItem(ENTERPRISE_MODE_ENUM,eHiResGraphics,"HiResGraphics");
	CStringsMaps::AddItem(ENTERPRISE_MODE_ENUM,eLiveVideo,"LiveVideo");
	CStringsMaps::AddItem(ENTERPRISE_MODE_ENUM,eCustomizedRate,"CustomizedContentRate");

	CStringsMaps::AddItem(PRESENTATION_PROTOCOL_ENUM,eH263Fix,"h.263");
	CStringsMaps::AddItem(PRESENTATION_PROTOCOL_ENUM,ePresentationAuto,"up_to_h.264");
	CStringsMaps::AddItem(PRESENTATION_PROTOCOL_ENUM,eH264Fix,"h.264_fix");
	CStringsMaps::AddItem(PRESENTATION_PROTOCOL_ENUM, eH264Dynamic,"h.264_dynamic");

	CStringsMaps::AddItem(VIDEO_QUALITY_ENUM,eVideoQualityAuto,"auto");
	CStringsMaps::AddItem(VIDEO_QUALITY_ENUM,eVideoQualityMotion,"motion");
	CStringsMaps::AddItem(VIDEO_QUALITY_ENUM,eVideoQualitySharpness,"sharpness");

	CStringsMaps::AddItem(CONFERENCE_TYPE_ENUM,CONF_TYPE_STANDARD,"standard");
	CStringsMaps::AddItem(CONFERENCE_TYPE_ENUM,CONF_TYPE_MEETING_ROOM,"meeting_Room");
	CStringsMaps::AddItem(CONFERENCE_TYPE_ENUM,CONF_TYPE_OPERATOR,"operator");

	CStringsMaps::AddItem(ENTRY_QUEUE_TYPE_ENUM,0,"normal");

	CStringsMaps::AddItem(MONTHLY_PATTERN_ENUM,eByDate,"by_date");
	CStringsMaps::AddItem(MONTHLY_PATTERN_ENUM,eByDay,"by_day");
	CStringsMaps::AddItem(REPEATED_TYPE_ENUM,eDaily,"daily");
	CStringsMaps::AddItem(REPEATED_TYPE_ENUM,eWeekly,"weekly");
	CStringsMaps::AddItem(REPEATED_TYPE_ENUM,eMonthly,"monthly");
	CStringsMaps::AddItem(INSTANCE_ENUM,eFirst,"first");
	CStringsMaps::AddItem(INSTANCE_ENUM,eSecond,"second");
	CStringsMaps::AddItem(INSTANCE_ENUM,eThird,"third");
	CStringsMaps::AddItem(INSTANCE_ENUM,eFourth,"fourth");
	CStringsMaps::AddItem(INSTANCE_ENUM,eLast,"last");
	CStringsMaps::AddItem(SIGN_TYPE,ePlus,"plus");
	CStringsMaps::AddItem(SIGN_TYPE,eMinus,"minus");


	CStringsMaps::AddItem(RESERVATION_STATUS_ENUM,eStsOK,"ok");//Status Field
	//CStringsMaps::AddItem(RESERVATION_STATUS_ENUM,STATUS_END_TIME_PASSED,"expired");
	CStringsMaps::AddItem(RESERVATION_STATUS_ENUM,eStsExpired,"expired");

	CStringsMaps::AddItem(RESERVATION_STATUS_ENUM,eStsSUSPEND,"suspended");

	//CStringsMaps::AddItem(RESERVATION_STATUS_ENUM,STATUS_PASSWORDS_CONFLICTS,"passwords_conflict");
	CStringsMaps::AddItem(RESERVATION_STATUS_ENUM,ePasswdConflict,"passwords_conflict");//

	CStringsMaps::AddItem(RESERVATION_STATUS_ENUM,eWrongSysMode,"wrong_system_mode");

	CStringsMaps::AddItem(ALLOCATION_MODE_ENUM,eAllocationModeNone,"none");
	CStringsMaps::AddItem(ALLOCATION_MODE_ENUM,eAllocationModeAuto,"auto");
	CStringsMaps::AddItem(ALLOCATION_MODE_ENUM,eAllocationModeFixed,"fixed");

	CStringsMaps::AddItem(TELEPRESENCE_PARTY_TYPE_ENUM,eTelePresencePartyNone,"none");
	CStringsMaps::AddItem(TELEPRESENCE_PARTY_TYPE_ENUM,eTelePresencePartyRPX,"rpx");
	CStringsMaps::AddItem(TELEPRESENCE_PARTY_TYPE_ENUM,eTelePresencePartyFlex,"tpx");
	CStringsMaps::AddItem(TELEPRESENCE_PARTY_TYPE_ENUM,eTelePresencePartyMaui,"maui");
	CStringsMaps::AddItem(TELEPRESENCE_PARTY_TYPE_ENUM,eTelePresencePartyCTS, "cts");
	CStringsMaps::AddItem(TELEPRESENCE_PARTY_TYPE_ENUM,eTelePresencePartyInactive, "inactive");

	CStringsMaps::AddItem(INTRA_REQUEST_ENUM,0,"in");
	CStringsMaps::AddItem(INTRA_REQUEST_ENUM,1,"out");

	CStringsMaps::AddItem(IP_SUB_SERVICE_ENUM, eAutoIpSubService ,"auto");
	CStringsMaps::AddItem(IP_SUB_SERVICE_ENUM, ePrimaryIpSubService, "primary");
	CStringsMaps::AddItem(IP_SUB_SERVICE_ENUM, eSecondaryIpSubService,"secondary");

	CStringsMaps::AddItem(TLSV_ENUM, eTLSV1_SSLV3, "TLSV1_SSLV3");
	CStringsMaps::AddItem(TLSV_ENUM, eTLSV1, "TLSV1");
	/*CStringsMaps::AddItem(TLSV_ENUM, eTLSV1_1, "TLSV1_1");
	CStringsMaps::AddItem(TLSV_ENUM, eTLSV1_2, "TLSV1_2");
	CStringsMaps::AddItem(TLSV_ENUM, eTLSV1_2_TLSV1_1, "TLSV1_2_TLSV1_1");
	CStringsMaps::AddItem(TLSV_ENUM, eTLS1_2_TLSV1_1_TLSV1, "TLS1_2_TLSV1_1_TLSV1");
	CStringsMaps::AddItem(TLSV_ENUM, eTLS1_2_TLSV1_1_TLSV1_SSLV3, "TLS1_2_TLSV1_1_TLSV1_SSLV3");*/


	CStringsMaps::AddItem(ISDN_RESOURCE_POLICY_ENUM, 0, "LOAD_BALANCE");
	CStringsMaps::AddItem(ISDN_RESOURCE_POLICY_ENUM, 1, "FILL_FROM_FIRST_CONFIGURED_SPAN");
	CStringsMaps::AddItem(ISDN_RESOURCE_POLICY_ENUM, 2, "FILL_FROM_LAST_CONFIGURED_SPAN");

	CStringsMaps::AddItem(BONDING_DIALING_METHOD_ENUM, 0, "BY_TIMERS");
	CStringsMaps::AddItem(BONDING_DIALING_METHOD_ENUM, 1, "SEQUENTIAL");

	CStringsMaps::AddItem(HD_RESOLUTION_ENUM,eHD1080p60Res,"hd_1080p60");
	CStringsMaps::AddItem(HD_RESOLUTION_ENUM,eHD1080Res,"hd_1080");
	CStringsMaps::AddItem(HD_RESOLUTION_ENUM,eHD720Res,"hd_720");//hd_720p30 not supported by Kobi G.
	CStringsMaps::AddItem(HD_RESOLUTION_ENUM,eHD720p60Res,"hd_720p60");
	CStringsMaps::AddItem(HD_RESOLUTION_ENUM,eSDRes,"sd");
	CStringsMaps::AddItem(HD_RESOLUTION_ENUM,eH264Res,"h264cif");
	CStringsMaps::AddItem(HD_RESOLUTION_ENUM,eH263Res,"h263cif");
	CStringsMaps::AddItem(HD_RESOLUTION_ENUM,eH261Res,"h261cif");

	//FIPS
	CStringsMaps::AddItem(FIPS140_SIMULATE_CARD_PROCESS_ENUM, 0, "INACTIVE");
	CStringsMaps::AddItem(FIPS140_SIMULATE_CARD_PROCESS_ENUM, 1, "FAIL_DETERMINISTIC_TEST");

	CStringsMaps::AddItem(FIPS140_SIMULATE_ENCRYPTION_PROCESS_ENUM, 0, "INACTIVE");
	CStringsMaps::AddItem(FIPS140_SIMULATE_ENCRYPTION_PROCESS_ENUM, 1, "FAIL_DETERMINISTIC_TEST");
	CStringsMaps::AddItem(FIPS140_SIMULATE_ENCRYPTION_PROCESS_ENUM, 2, "FAIL_POOL_GENERATION_TEST");

	CStringsMaps::AddItem(FIPS140_SIMULATE_CONFPARTY_PROCESS_ENUM, 0, "INACTIVE");
	CStringsMaps::AddItem(FIPS140_SIMULATE_CONFPARTY_PROCESS_ENUM, 1, "FAIL_DETERMINISTIC_TEST");
	CStringsMaps::AddItem(LAST_QUIT_TYPE_ENUM,eTerminateAfterLastLeaves,"after_last_quit");
	CStringsMaps::AddItem(LAST_QUIT_TYPE_ENUM,eTerminateWithLastRemains,"when_last_participant_remains");


	for(eAuditEventType type = eAuditEventTypeApi ; type < NUM_AUDIT_EVENT_TYPES ; type = (eAuditEventType)(type + 1))
	{
		CStringsMaps::AddItem(AUDIT_TYPE_GROUP_ENUM, type, AuditEventTypeToStr(type));
	}

	for(eAuditEventStatus type = eAuditEventStatusOk ; type < NUM_AUDIT_EVENT_STATUSES ; type = (eAuditEventStatus)(type + 1))
	{
		CStringsMaps::AddItem(AUDIT_STATUS_GROUP_ENUM, type, AuditEventStatusToStr(type));
	}

	for(eFreeDataType type = eFreeDataTypeText ; type < NUM_AUDIT_DATA_TYPES ; type = (eFreeDataType)(type + 1))
	{
		CStringsMaps::AddItem(AUDIT_DATA_TYPE_GROUP_ENUM, type, GetFreeDataTypeName(type));
	}

	CStringsMaps::AddMinMaxItem(_0_TO_5000_DECIMAL,0,5000);
	CStringsMaps::AddMinMaxItem(_0_TO_3600_DECIMAL,0,3600);
	CStringsMaps::AddMinMaxItem(_0_TO_360000_DECIMAL,0,360000);
	CStringsMaps::AddMinMaxItem(_0_TO_APPOINTMENT_ID_LENGTH,0,APPOITNMENT_ID_LEN - 1);
	CStringsMaps::AddMinMaxItem(_0_TO_MAXKAVALUE,0,86400);

	CStringsMaps::AddItem(FPS_MODE_ENUM, 1, "PAL");
	CStringsMaps::AddItem(FPS_MODE_ENUM, 2, "NTSC");
	CStringsMaps::AddItem(FPS_MODE_ENUM, 3, "AUTO");

	CStringsMaps::AddItem(ITP_CROPPING_ENUM, 0, "ITP");
	CStringsMaps::AddItem(ITP_CROPPING_ENUM, 1, "CP");
	CStringsMaps::AddItem(ITP_CROPPING_ENUM, 2, "MIXED");

	CStringsMaps::AddItem(TELEPRESENCE_MODE_CONFIGURATION_ENUM,NO,"no");
	CStringsMaps::AddItem(TELEPRESENCE_MODE_CONFIGURATION_ENUM,YES,"yes");
	CStringsMaps::AddItem(TELEPRESENCE_MODE_CONFIGURATION_ENUM,AUTO,"auto");

	CStringsMaps::AddItem(TELEPRESENCE_LAYOUT_MODE_ENUM,eTelePresenceLayoutManual,"manual");
	CStringsMaps::AddItem(TELEPRESENCE_LAYOUT_MODE_ENUM,eTelePresenceLayoutContinuousPresence,"cp");
	CStringsMaps::AddItem(TELEPRESENCE_LAYOUT_MODE_ENUM,eTelePresenceLayoutRoomSwitch,"room_switch");
	// TELEPRESENCE_LAYOUTS
	CStringsMaps::AddItem(TELEPRESENCE_LAYOUT_MODE_ENUM,eTelePresenceLayoutCpSpeakerPriority,"cp_speaker_priority");
	CStringsMaps::AddItem(TELEPRESENCE_LAYOUT_MODE_ENUM,eTelePresenceLayoutCpParticipantsPriority,"cp_participants_priority");

	CStringsMaps::AddItem(TIP_COMPATIBILITY_ENUM,eTipCompatibleNone,"none");
	CStringsMaps::AddItem(TIP_COMPATIBILITY_ENUM,eTipCompatibleVideoOnly,"video_only");
	CStringsMaps::AddItem(TIP_COMPATIBILITY_ENUM,eTipCompatibleVideoAndContent,"video_and_content");
	CStringsMaps::AddItem(TIP_COMPATIBILITY_ENUM,eTipCompatiblePreferTIP,"prefer_tip"); //TIP call from Polycom EPs feature

	CStringsMaps::AddItem(AV_MCU_CASCADE_MODE_ENUM,eMsSvcResourceOptimize,"resource_optimized");
	CStringsMaps::AddItem(AV_MCU_CASCADE_MODE_ENUM,eMsSvcVideoOptimize,"video_optimized");


	CStringsMaps::AddItem(TIP_PARTY_TYPE_ENUM,eTipPartyNone,"none");
	CStringsMaps::AddItem(TIP_PARTY_TYPE_ENUM,eTipPartyMaster,"tip_master");
	CStringsMaps::AddItem(TIP_PARTY_TYPE_ENUM,eTipPartySlave,"tip_slave");

	//Resolution Slider
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_cif30, "cif30");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_cif60, "cif60");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_wcif, "wcif");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_sd15, "sd15");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_sd30, "sd30");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_sd60, "sd60");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_hd720p30, "hd720p30");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_hd720p60, "hd720p60");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_hd1080p30, "hd1080p30");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_TYPE_ENUM, e_hd1080p60, "hd1080p60");

	CStringsMaps::AddItem(RESOLUTION_CONFIG_TYPE_ENUM, e_balanced, "balanced");
	CStringsMaps::AddItem(RESOLUTION_CONFIG_TYPE_ENUM, e_resource_optimized, "resource_optimized");
	CStringsMaps::AddItem(RESOLUTION_CONFIG_TYPE_ENUM, e_user_exp_optimized, "user_exp_optimized");
	CStringsMaps::AddItem(RESOLUTION_CONFIG_TYPE_ENUM, e_hi_profile_optimized, "high_profile_optimized");
	CStringsMaps::AddItem(RESOLUTION_CONFIG_TYPE_ENUM, e_manual, "manual");

	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e64000,"64");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e96000, "96");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e128000, "128");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e192000, "192");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e256000, "256");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e320000, "320");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e384000, "384");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e512000, "512");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e768000, "768");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e832000, "832");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e1024000, "1024");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e1152000, "1152");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e1280000, "1280");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e1472000, "1472");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e1536000, "1536");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e1728000, "1728");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e1920000, "1920");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e2048000, "2048");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e2560000, "2560");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e3072000, "3072");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e3584000, "3584");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e4096000, "4096");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e6144000, "6144");
	CStringsMaps::AddItem(RESOLUTION_THRESHOLD_RATE_ENUM, e8192000, "8192");

	CStringsMaps::AddItem(ENCRYPTION_TYPE_ENUM, eEncryptNone, "all_non_encrypted");
	CStringsMaps::AddItem(ENCRYPTION_TYPE_ENUM, eEncryptAll, "all_encrypted");
	CStringsMaps::AddItem(ENCRYPTION_TYPE_ENUM, eEncryptWhenAvailable, "when_available");

	//sipProxySts
	CStringsMaps::AddItem(SIP_REG_STATUS_ENUM, eSipRegistrationStatusTypeNotConfigured, "not_configured");
	CStringsMaps::AddItem(SIP_REG_STATUS_ENUM, eSipRegistrationStatusTypeRegistered, "registered");
	CStringsMaps::AddItem(SIP_REG_STATUS_ENUM, eSipRegistrationStatusTypeFailed, "failed");

	CStringsMaps::AddItem(SIP_REG_TOTAL_STATUS_ENUM, eSipRegistrationTotalStatusTypeNotConfigured, "not_configured");
	CStringsMaps::AddItem(SIP_REG_TOTAL_STATUS_ENUM, eSipRegistrationTotalStatusTypeRegistered, "registered");
	CStringsMaps::AddItem(SIP_REG_TOTAL_STATUS_ENUM, eSipRegistrationTotalStatusTypePartiallyRegistered, "partially_registered");
	CStringsMaps::AddItem(SIP_REG_TOTAL_STATUS_ENUM, eSipRegistrationTotalStatusTypeFailed, "failed");

	//H264 VSW High Profile Support
	// CStringsMaps::AddItem(H264_VSW_HIGH_PROFILE_PREFERENCE_ENUM,eWhenPossible,"when_possible"); // in V7.6 decided not to implement this option
	CStringsMaps::AddItem(H264_VSW_HIGH_PROFILE_PREFERENCE_ENUM,eAlways,"always");
	CStringsMaps::AddItem(H264_VSW_HIGH_PROFILE_PREFERENCE_ENUM,eBaseLineOnly,"base_line_only");

	CStringsMaps::AddItem(CASCADE_OPTIMIZE_RESOLUTION_ENUM,e_res_720_5fps,"res_720_5fps");
	CStringsMaps::AddItem(CASCADE_OPTIMIZE_RESOLUTION_ENUM,e_res_720_30fps,"res_720_30fps");
	CStringsMaps::AddItem(CASCADE_OPTIMIZE_RESOLUTION_ENUM,e_res_1080_15fps,"res_1080_15fps");
	CStringsMaps::AddItem(CASCADE_OPTIMIZE_RESOLUTION_ENUM,e_res_1080_30fps,"res_1080_30fps");
	CStringsMaps::AddItem(CASCADE_OPTIMIZE_RESOLUTION_ENUM,e_res_1080_60fps,"res_1080_60fps");
	CStringsMaps::AddItem(CASCADE_OPTIMIZE_RESOLUTION_ENUM,e_res_dummy,"0");

	// Font Types feature
	CStringsMaps::AddItem(FONT_TYPES_ENUM, ftDefault, "Default");
	CStringsMaps::AddItem(FONT_TYPES_ENUM, ftHeiti, "Heiti");
	CStringsMaps::AddItem(FONT_TYPES_ENUM, ftKaiti, "Kaiti");
	CStringsMaps::AddItem(FONT_TYPES_ENUM, ftSongti, "Songti");
	CStringsMaps::AddItem(FONT_TYPES_ENUM, ftWeibei, "Weibei");

	CStringsMaps::AddItem(CONF_MEDIA_TYPE_ENUM,		eAvcOnly,		"avc_only");
	CStringsMaps::AddItem(CONF_MEDIA_TYPE_ENUM,		eSvcOnly,		"media_relay_only");
	CStringsMaps::AddItem(CONF_MEDIA_TYPE_ENUM,		eMixAvcSvc,		"mix_avc_media_relay");
	CStringsMaps::AddItem(CONF_MEDIA_TYPE_ENUM,		eMixAvcSvcVsw,	"mix_avc_media_relay_vsw");

	CStringsMaps::AddItem(PARTY_MEDIA_TYPE_ENUM,	eAvcPartyType,	"avc");
	CStringsMaps::AddItem(PARTY_MEDIA_TYPE_ENUM,	eSvcPartyType,	"media_relay");

	CStringsMaps::AddItem(OPERATION_POINTS_PRESET_ENUM,	eOPP_mobile,	"mobile_optimized");
	CStringsMaps::AddItem(OPERATION_POINTS_PRESET_ENUM,	eOPP_qvga,		"qvga_optimized");
	CStringsMaps::AddItem(OPERATION_POINTS_PRESET_ENUM,	eOPP_cif,		"cif_optimized");
	CStringsMaps::AddItem(OPERATION_POINTS_PRESET_ENUM,	eOPP_vga,		"vga_optimized");
	CStringsMaps::AddItem(OPERATION_POINTS_PRESET_ENUM,	eOPP_sd,		"sd_optimized");
	CStringsMaps::AddItem(OPERATION_POINTS_PRESET_ENUM,	eOPP_hd,		"hd_optimized");

	CStringsMaps::AddItem(REVOCATION_METHOD_ENUM,	eNone,	"none");
	CStringsMaps::AddItem(REVOCATION_METHOD_ENUM,	eCrl,	"crl");
	CStringsMaps::AddItem(REVOCATION_METHOD_ENUM,	eOcsp,	"ocsp");

	CStringsMaps::AddItem(RELAY_CODEC_TYPE_ENUM,	eH264,		"H264");
	CStringsMaps::AddItem(RELAY_CODEC_TYPE_ENUM,	eH264SVC,	"H264SVC");
	CStringsMaps::AddItem(RELAY_CODEC_TYPE_ENUM,	eSAC,		"SAC");
	CStringsMaps::AddItem(RELAY_CODEC_TYPE_ENUM,	eG711,		"G711");
	CStringsMaps::AddItem(RELAY_CODEC_TYPE_ENUM,	eG722,		"G722");
	CStringsMaps::AddItem(RELAY_CODEC_TYPE_ENUM,	eG722_1,	"G722_1");
	//SHA1
	CStringsMaps::AddItem(SRTP_SRTCP_HMAC_SHA_LENGTH_ENUM, eSha1_length_80, "80");
	CStringsMaps::AddItem(SRTP_SRTCP_HMAC_SHA_LENGTH_ENUM, eSha1_length_32, "32");
	CStringsMaps::AddItem(SRTP_SRTCP_HMAC_SHA_LENGTH_ENUM, eSha1_length_80_32, "80_32");

	//Amdocs Encoder/Decoder Gain
	CStringsMaps::AddMinMaxItem(_0_TO_1000_DECIMAL,0,1000);
	CStringsMaps::AddMinMaxItem(_70_TO_95_DECIMAL,70,95);
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	0,		"RPCS");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	1,		"RPP_PKG");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	2,		"AVC_CIF_PLUS");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	3,		"TIP");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	4,		"RPCS_MAX_PORTS");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	5,		"MEDIA_ENCRYPTION");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	6,		"RPCS_TELEPRESENCE");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	7,		"RPCS_MULTIPLE_SERVICES");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	8,		"RPCS_SVC");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	9,		"RPCS_AVAYA");
	CStringsMaps::AddItem(LICENSE_FEATURE_ENUM,	10,		"RPCS_IBM");



	CStringsMaps::AddItem(LICENSE_FEATURE_STATUS_ENUM,	0,		"Acq_status_unknown");
	CStringsMaps::AddItem(LICENSE_FEATURE_STATUS_ENUM,	1,		"Acq_status_acquired");
	CStringsMaps::AddItem(LICENSE_FEATURE_STATUS_ENUM,	2,		"Acq_status_failed");

	CStringsMaps::AddItem(LICENSE_STATUS_REASON_ENUM,	0,		"Acq_status_reason_normal");
	CStringsMaps::AddItem(LICENSE_STATUS_REASON_ENUM,	1,		"Acq_status_reason_normal_exc_reservation");
	CStringsMaps::AddItem(LICENSE_STATUS_REASON_ENUM,	2,		"Acq_status_reason_expired");
	CStringsMaps::AddItem(LICENSE_STATUS_REASON_ENUM,	3,		"Acq_status_reason_wrong_count");
	CStringsMaps::AddItem(LICENSE_STATUS_REASON_ENUM,	4,		"Acq_status_reason_Gen_error");
	
		//Indication for audio participants
	CStringsMaps::AddItem(ICON_DISPLAY_POSITION_ENUM,	eLocationTopLeft,	"top_left");
	CStringsMaps::AddItem(ICON_DISPLAY_POSITION_ENUM,	eLocationTop,		"top");
	CStringsMaps::AddItem(ICON_DISPLAY_POSITION_ENUM,	eLocationTopRight,		"top_right");
	CStringsMaps::AddItem(ICON_DISPLAY_POSITION_ENUM,	eLocationBottomLeft,		"bottom_left");
	CStringsMaps::AddItem(ICON_DISPLAY_POSITION_ENUM,	eLocationBottom,		"bottom");
	CStringsMaps::AddItem(ICON_DISPLAY_POSITION_ENUM,	eLocationBottomRight,		"bottom_right");
	
	CStringsMaps::AddItem(ICON_DISPLAY_MODE_ENUM,	eIconDisplayOnChange,		"on_audio_participants_change");
	CStringsMaps::AddItem(ICON_DISPLAY_MODE_ENUM,	eIconDIsplayPermanent,		"permanent");
	
	
	CStringsMaps::AddItem(_0_OR_1_OR_4_DECIMAL, 0, "0");
	CStringsMaps::AddItem(_0_OR_1_OR_4_DECIMAL, 1, "1");
	CStringsMaps::AddItem(_0_OR_1_OR_4_DECIMAL, 4, "4");

	CStringsMaps::AddMinMaxItem(_0_TO_MAX_ACTIVE_SPEAKER_PREFERENCE, 0, 6);


	CStringsMaps::AddItem(LICENSE_MODE_ENUM, 0, "none");
	CStringsMaps::AddItem(LICENSE_MODE_ENUM, 1, "flexera");
	CStringsMaps::AddItem(LICENSE_MODE_ENUM, 2, "cfs");
}



