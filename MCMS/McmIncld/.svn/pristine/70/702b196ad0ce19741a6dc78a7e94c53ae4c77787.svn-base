
#ifndef      __IpServiceDefines__
#define      __IpServiceDefines__

#include "IpAddressDefinitions.h"

//for ostr stream::streamsize pcount() const
typedef int streamsize;



/*-------------------------------------------------------------------------------
	it was taken from MCMSOPER.H (MCMSNT).
-------------------------------------------------------------------------------*/
//sip Servers type
//#define  CONFIGURE_SIP_SERVERS_AUTOMATICALLY                        1
//#define  CONFIGURE_SIP_SERVERS_MANUALLY                              0

#define PARTY_SIP_SIPURI_ID_TYPE			 1
#define PARTY_SIP_TELURL_ID_TYPE			 2

#define API_IP_SERVICE_CORRECTIONS 			611
#define  PROTOCOL_TRASPORT_TCP                       2
#define API_NUM_SIP                         580
#define API_NUM_SIP_VERSION_TWO             598
#define  PROTOCOL_TRASPORT_UDP                       1

//#define NUM_OF_DNS_IPS 		3

enum eSipServerStatusType
{
    eSipServerStatusNotAvailable,
    eSipServerStatusTypeOk,
    eSipServerStatusTypeFail
};

enum eSipRegistrationStatusType
{
    eSipRegistrationStatusTypeNotConfigured,
    eSipRegistrationStatusTypeRegistered,
    eSipRegistrationStatusTypeFailed
};

enum eSipRegistrationTotalStatusType
{
    eSipRegistrationTotalStatusTypeNotConfigured,
    eSipRegistrationTotalStatusTypeRegistered,
    eSipRegistrationTotalStatusTypePartiallyRegistered,
    eSipRegistrationTotalStatusTypeFailed
};
enum eSipRegistrationConfType
{
    eSipRegistrationConfTypeOngoing,
    eSipRegistrationConfTypeMR,
    eSipRegistrationConfTypeEQ,
    eSipRegistrationConfTypeFactory,
    eSipRegistrationConfTypeGWProfile,
    eSipRegistrationConfTypeIncorrect
};
// sip enums, used in SerializeXML of CIpSip
enum eConfigurationSipServerMode
{
	eConfSipServerAuto = 0,		// =='Off'(!) in EMA
	eConfSipServerManually		// =='Specify' in EMA
};

enum eServerStatus
{
	eServerStatusAuto = 0,
	eServerStatusSpecify,
	eServerStatusOff
};

enum eServerType
{
    eServerTypeNotAvailable,
    eServerTypePrimary,
    eServerTypeAlternate
};


// H323 Gatekeeper states
#define GATEKEEPER_NONE      0
#define GATEKEEPER_EXTERNAL  1
#define GATEKEEPER_INTERNAL  2


#define GK_MODE_BOARD_HUNTING     0
#define GK_MODE_BASIC             1
#define GK_MODE_GATEWAY           2
#define GK_MODE_PSEUDO_GK         3
#define GK_MODE_PSEUDO_AVAYA_GK   4

// protocol types
enum  eRegistrationMode{
	eRegistrationMode_Redirect=0,
	eRegistrationMode_Polling,
	eRegistrationMode_Move,
	eRegistrationMode_DNS,
	eRegistrationMode_Forking
};
enum  eIPProtocolType
{
		eIPProtocolType_None = -1,
		eIPProtocolType_SIP=0,
		eIPProtocolType_H323,
		eIPProtocolType_SIP_H323
};

enum  eIpServiceType
{
	eIpServiceType_Signaling=0,
	eIpServiceType_Management,
	eIpServiceType_Control
};


enum  eVlanModeType
{
	eVlanMode_1 = 0,
	eVlanMode_2
};

enum  eIceEnvironmentType
{
		eIceEnvironment_None = 0,
		eIceEnvironment_ms,
		eIceEnvironment_Standard,
		eIceEnvironment_WebRtc
};


enum  eSipServerType
{
		eSipServer_generic = 0,
		eSipServer_ms,
		eSipServer_CiscoCucm
};

//H323 QOS
#define ATM_AAL5_DISABLE  0
#define ATM_AAL5_ENABLE   1
#define H323_ATM_QOS_CBR  5  //Constant BitRate

#define  H323_ATM_QOS_UBR      1  //Unpecified BitRate
#define  H323_ATM_QOS_rtVBR    2  //RealTime variable BitRate
#define  H323_ATM_QOS_nrtVBR   3  //NonRealTime variable Bitrate
#define  H323_ATM_QOS_ABR      4  //Available BitRate

//#define  H323_SPEED_AUTO             0
//#define  H323_SPEED_10                10
//#define  H323_SPEED_10_DUPLEX_FULL    11
//#define  H323_SPEED_100               100
//#define  H323_SPEED_100_DUPLEX_FULL   101


#define  ELAN_NAME_SIZE				32
#define  UNI_PROTOCOL_VER_3_0       30
#define  DEFAULT_UNI_PROTOCOL_VER   UNI_PROTOCOL_VER_3_0
#define  DEFAULT_MAX_NTU_SIZE		1536



//Limit constants
#define ALIAS_NAME_LEN                            100
#define XML_API_MAX_PARTY_ALIAS_LEN				  80
#define XML_API_MAX_SERVICE_ALIAS_LEN			  28
#define MAX_ROUTERS_IN_H323_SERVICE               5
#define MAX_ALIAS_NAMES_NUM                       5

//size of configurations record
#define  SIZE_RECORD         2048

// Network configuration
#define SERVICE_H323_LAN				     5
#define SERVICE_H323_IPOATM					 6
#define SERVICE_H323_LANEMU					 7
#define MAX_SPAN_NUMBER_IN_SERVICE      30
#define MAX_PHONE_NUMBER_IN_SERVICE     30
#define MAX_SERV_PROVIDERS_IN_LIST      30
#define MAX_NET_SERV_PROVIDERS_IN_LIST  70
#define MAX_NUMBER_NET_SPECIFICATIONS   30
#define MAX_NUMBER_LEASED_LINE_PARTY    23
#define MAX_INFO_ELEMENT_NUMBER         10
#define MAX_SERVICE_NUMBER_FOR_BACKUP   10
#define MAX_SUB_SERVICE_IN_SERVICE      30
#define MAX_DIALIN_SERVICE              10


#define SPAN_H323_LAN      3
#define SPAN_H323_IPOATM   4
#define SPAN_H323_LANEMU   5
#define UNKNOWN_SPAN       222



//Party h323 alias types
#define PARTY_H323_ALIAS_E164_TYPE		    7
#define PARTY_H323_ALIAS_H323_ID_TYPE       8
#define PARTY_H323_ALIAS_URL_ID_TYPE         11
#define PARTY_H323_ALIAS_TRANSPORT_ID_TYPE   12
#define PARTY_H323_ALIAS_EMAIL_ID_TYPE       13
#define PARTY_H323_ALIAS_PARTY_NUMBER_TYPE   14





// Api number
#define API_NUM_H323_ATM				     98
#define API_NUM_H323_ATM_2					100
#define API_NUM_H323_ATM_3                  128
#define API_NUM_H323_QOS                    138
#define API_NUM_GK_MODES					141
#define API_NUM_H323_ENABLE_PORTRANGE       150
#define API_NUM_FECC_PORT_RANGE  			609

//DNS
#define  NUM_OF_DNS_SERVERS		     3

#define NUM_OF_AUTHENTICATION_ELEMENTS_PARAMS           5

//format type
//#define CONFIG          254
#define OPERATOR_MCMS   255

//#define API_NUMBER		675

//H323 definitions
#define  H323_SPEED_AUTO             0
#define  ELAN_NAME_SIZE				32
#define  UNI_PROTOCOL_VER_3_0       30
#define  DEFAULT_UNI_PROTOCOL_VER   UNI_PROTOCOL_VER_3_0
#define  DEFAULT_MAX_NTU_SIZE		1536

#define NUM_SIGNALLING_PORTS_PER_PARTY   1
#define NUM_CONTROL_PORTS_PER_PARTY	     1
#define NUM_AUDIO_PORTS_PER_PARTY        2
#define NUM_FECC_PORTS_PER_PARTY         2
#define NUM_VIDEO_PORTS_PER_PARTY        4
#define NUM_CONTENT_PORTS_PER_PARTY      5


#define  DONOT_USE_DEFAULT_ARP_SERVER 0
#define  USE_DEFAULT_ARP_SERVER       1
#define  DONOT_USE_DISCOVERED_LECS	  0
#define  USE_DISCOVERED_LECS		  1
#define  DONOT_USE_LECS_FOR_LES       0
#define  USE_LECS_FOR_LES             1
#define DONOT_USE_AUTO_ELAN_NAME	  0
#define USE_AUTO_ELAN_NAME            1

//#define  SERVER_OFF        0
//#define  SERVER_SPECIFY    1
//#define  SERVER_AUTO       2

// H323 Remote types
#define H323_REMOTE_NETWORK 0
#define H323_REMOTE_HOST    1

#define ILMI_ENABLE   1


// authentication server
#define  NUM_OF_AUTHENTICATION_ELEMENTS                                         3
//#define  PROTOCOL_TRASPORT_UDP                       1
//#define  PROTOCOL_TRASPORT_TCP


//Operator's authorization group
//#define SUPER           0
#define ORDINARY          1
//#define AUTH_OPERATOR   2
//#define RECORDING_USER  3
//#define RECORDING_ADMIN 4
//#define GUEST           100

#define  FILE_H323_CONFIG_DAT            "7.256/mcu/cfg/h323021.xml"


#endif  //__IpServiceDefines__
