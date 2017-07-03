// CDRDefines.h

#ifndef CDRDEFINES_H_
#define CDRDEFINES_H_

#include "DataTypes.h"
#include "SystemFunctions.h"
#include "H221.h"

// format type
#define OPERATOR_MCMS                                   255
#define CONFIG                                          254

// Limit constants
#define MAX_CDR_SHORT_IN_LIST                           2000
#define MAX_CDR_SHORT_IN_LIST_FOR_AMOS                  4000

#define H243_NAME_LEN_OLD                               33  // 32 characters + terminator (for party\conf names)
#define H243_NAME_LEN                                   81  // 80 characters + terminator (for party\conf names)
#define CORRELATION_ID_LENGTH				     		145 // 39 ipv6 Mac Address + 81 display name + 22 time + 2 hyphen + terminator
#define RESOLUTION_LEN                 20  //"Unknown Format" is the longest string
#define FRAME_RATE_LEN                 20
#define IPADDRESS_LEN                  81
#define H243_TERMINAL_NAME_LEN                          33  // 32 characters + terminator
#define PRIVATE_VERSION_DESC_LEN                        41  // 40 characters + terminator
#define ENTRY_QUEUE_NAME_LEN                            21
#define CORRELATION_SIG_UUID_LEN						100
// Billing event type
#define CONFERENCE_START                                1
#define CONFERENCES_END                                 2
#define NET_CHANNEL_CONNECTED                           3
#define NET_CHANNEL_DISCONNECTED                        4
#define EVENT_PARTY_CONNECTED                           5
#define EVENT_PARTY_DISCONNECTED                        7
#define REMOTE_COM_MODE                                 8
#define PARTY_ERRORS                                    9
#define RESERVED_PARTY                                  10
#define ATM_CHANNEL_CONNECTED                           11
#define ATM_CHANNEL_DISCONNECTED                        12
#define MPI_CHANNEL_CONNECTED                           13
#define MPI_CHANNEL_DISCONNECTED                        14
#define H323_CALL_SETUP                                 15
#define H323_CLEAR_INDICATION                           16
#define IP_PARTY_CONNECTED                              17
#define EVENT_NEW_UNDEFINED_PARTY                       18
#define UNRECOGNIZED_EVENT                              19
#define EVENT_PARTY_ADD_BILLING_CODE                    20
#define EVENT_SET_PARTY_VISUAL_NAME                     21
#define DTMF_CODE_FAILURE                               22
#define SIP_PARTY_CONNECTED                             23
#define SIP_CALL_SETUP                                  24
#define SIP_CLEAR_INDICATION                            25
#define RECORDING_LINK_EVENT                            26
#define RECORDING_SYSTEM_LINK_EVENT                     27
#define SIP_PRIVATE_EXTENSIONS_EVENT                    28
#define GK_INFO                                         30
#define PARTY_NEW_RATE                                  31
#define EVENT_NEW_UNDEFINED_PARTY_CONTINUE_IPV6_ADDRESS 32
#define PARTY_CHAIR_UPDATE                              33
#define PARTICIPANT_MAX_USAGE_INFO                      34
#define SVC_SIP_PARTY_CONNECTED                         35
#define PARTY_CORRELATION_DATA                          36
#define CONF_CORRELATION_DATA                           37


/* Defined in ConfPartyDefines.h
#define OPERATOR_ADD_PARTY                              101
#define OPERATOR_UPDATE_PARTY                           105
#define OPERATOR_TERMINATE                              100
#define OPERATORS_DELETE_PARTY                          102
#define OPERATOR_DISCONNECTE_PARTY                      103
#define OPERATOR_RECONNECT_PARTY                        104
#define OPERATOR_SET_END_TIME                           106
*/

// commented in ConfPartyDefines.h
#define OPERRATOR_MOVE_PARTY_FROM_CONFERENCE            107
#define OPERRATOR_MOVE_PARTY_TO_CONFERENCE              108
#define OPERRATOR_ATTEND_PARTY                          109
#define OPERRATOR_ONHOLD_PARTY                          110
#define OPERRATOR_BACK_TO_CONFERENCE_PARTY              111
#define OPERRATOR_ATTEND_PARTY_TO_CONFERENCE            112
#define CONFERENCE_REMARKS                              113
#define CONTACT_INFORMATION                             114
// #define USER_DEFINED_INFORMATION                     115
#define BILLING_INFORMATION                             116

#define NEW_UNDEFINED_PARTY_CONTINUE_1                  1001

#define CONFERENCE_START_CONTINUE_1                     2001
#define RESERVED_PARTY_CONTINUE_1                       2010
#define RESERVED_PARTICIPANT_CONTINUE_IPV6_ADDRESS      2011
#define RESERVED_PARTY_CONTINUE_2                       2012
#define USER_DEFINED_INFORMATION                        3010
#define OPERATOR_ADD_PARTY_CONTINUE_1                   2101
#define USER_ADD_PARTICIPANT_CONTINUE_IPV6_ADDRESS      2102
#define OPERATOR_ADD_PARTY_CONTINUE_2                   2103
#define OPERATOR_UPDATE_PARTY_CONTINUE_1                2105
#define USER_UPDATE_PARTICIPANT_CONTINUE_IPV6_ADDRESS   2106
#define OPERATOR_UPDATE_PARTY_CONTINUE_2                2107
#define OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_1   2108

#define OPERRATOR_MOVE_PARTY_TO_CONFERENCE_CONTINUE_2   3108
#define PARTY_DISCONNECTED_CONTINUE_1                   2007
#define CONFERENCE_START_CONTINUE_2                     3001
#define CONFERENCE_START_CONTINUE_3                     4001
#define CONFERENCE_START_CONTINUE_4                     5001
#define CONFERENCE_START_CONTINUE_5                     6001
#define CONFERENCE_START_CONTINUE_10                    11001

// An extension that is transferred from a dialOut party via TCS-4 to a gateway as a remote's alias.
#define PARTY_EXTENSION_LENGTH                          256                 // The length - 256 characters - is according to the standard
#define PHONE_NUMBER_DIGITS_LEN                         31                  // 40 characters + terminator

#define MAX_CONF_INFO_ITEMS                             3
#define MAX_USER_INFO_ITEMS                             4

#define CONF_INFO_ITEM_LEN                              101                 // 100 characters + terminator
#define CONF_BILLING_INFO_LEN                           CONF_INFO_ITEM_LEN  /* 101 */

#define PRI_LIMIT_PHONE_DIGITS_LEN                      24
#define PRI_LIMIT_PHONE_DIGITS_LEN_WITH_DELIMITER       (PRI_LIMIT_PHONE_DIGITS_LEN+1)

#define MAX_CDR_EVENT_IN_LIST                           12000
#define MAX_PHONE_NUMBERS_IN_CONFERENCE                 2
#define MAX_SOFTWARE_CP_CELL_NUMBER                     4

// Api number
#define API_NUM_LONG_NAMES                              15

// from 100 ... 120 is BONDING_FAILURE family commands
#define LONG_DELAY_BETWEEN_CHANNELS                     100
#define INTERNAL_ERROR                                  101
#define NO_ANSWER_FROM_REMOTE_PART                      102
#define ERROR_IN_DIALING_ADDITIONAL_CHANNELS            103

// from 121 ... 140 is NO_ESTABL_H243_CONNECT family commands
#define FULL_BITRATE_CONNECTION_FAILURE                 121
#define CAP_EXCHANGE_FAILURE_IN_ESTABLISH_CALL          122
#define CAP_EXCHANGE_FAILURE_IN_CHANGE_MODE             123
#define UNSPECIFIED                                     124
#define NOT_RECIVED_END_INIT_COM                        125

/*------------------------------------------------------------------------------------
    CDRINCLD.h from MGC
------------------------------------------------------------------------------------*/
#define  RESOURCE_FAILED                                1
#define  NO_FAILURE                                     0

#define CAUSE_RESTRICT                                  1
#define SM_COMP                                         2
#define BIT_RATE                                        3
#define CAUSE_LSD                                       4
#define CAUSE_HSD                                       5
#define CAUSE_MLP                                       6
#define CAUSE_H_MLP                                     7
#define CAUSE_QCIF                                      8
#define CAUSE_VIDEO_FRAME_RATE                          9
#define CAUSE_H_243                                     10
#define OTHER_CAUSE                                     0xFF

// callpart.cpp
#define UNKNOWN_TYPE                                    0
#define INTERNATIONAL                                   1
#define NATIONAL                                        2
#define SUBSCRIBER                                      4
#define ABBREVIATED                                     6
#define UNKNOWN_PLAN                                    0
#define ISDN                                            1
#define TELEPHONY                                       2
#define PRIVATE                                         3
#define ALLOWED                                         0
#define RESTRICTED                                      1
#define NOT_AVAILABLE                                   2
#define NOT_SCREENED                                    0
#define USER_VER_PASSED                                 1
#define USER_VER_FAILED                                 2
#define CALLED_UNKNOWN_TYPE                             0
#define CALLED_INTERNATIONAL                            1
#define CALLED_NATIONAL                                 2
#define CALLED_SUBSCRIBER                               4
#define CALLED_ABBREVIATED                              6
#define CALLED_UNKNOWN_PLAN                             0
#define CALLED_ISDN                                     1
#define CALLED_TELEPHONY                                2
#define CALLED_PRIVATE                                  3

/*------------------------------------------------------------------------------------
    class CNetChannelDisco (Netdisco.cpp)
------------------------------------------------------------------------------------*/
#define MCU_DISCONCT_INIT                               0
#define REMOTE_PARTY_DISCONCT_INIT                      1
#define PRIcodCCIT                                      0x00
#define PRIcodNATIONAL_STD                              0x02
#define PRIcodSTD_SPF_TO_LOC                            0x03
#define PRIlocUSER                                      0x00
#define PRIlocPVT_local                                 0x01
#define PRIlocPUB_LOCAL                                 0x02
#define PRIlocTRANSIT_NET                               0x03
#define PRIlocPUB_REMOTE                                0x04
#define PRIlocPVT_REMOTE                                0x05
#define PRIlocINTERNATIONAL                             0x07
#define PRIlocBEY_INTRWORK                              0x10

/*------------------------------------------------------------------------------------
    class CNetChannelDisco (CdrDetal.cpp)
------------------------------------------------------------------------------------*/
#define ONGOING                                         1
#define CAUSE_OPERATOR_TERMINATE                        2
#define END_TIME_TERMINATE                              3
#define NEVER_STARTED                                   252
#define CONF_PROBLEM                                    253
#define MCU_COMPLETED                                   254

/*------------------------------------------------------------------------------------
    class CNetChanlCon (NTCHACON.CPP)
------------------------------------------------------------------------------------*/
#define MCU_INITIATOR                                   0
#define REMOTE_PARTY_INITIATOR                          1
#define MODEM                                           0x2
#define CALL_TYPE_56K                                   0x4
#define CALL_TYPE_64K                                   0x8
#define CALL_TYPE_64K_RESTRICT                          0x10
#define CALL_TYPE_384K                                  0x20
#define CALL_TYPE_384K_RESTRICT                         0x40
#define NONE                                            0
#define ATT_SDN                                         1
#define NTI_PRIVATE                                     0xF1
#define ATI_MEGACOM                                     3
#define NTI_OUTWATS                                     0xF3
#define NTI_FX                                          4
#define NTI_TIE_TRUNK                                   5
#define ATT_ACCUNET                                     6
#define ATT_1800                                        8
#define NTI_TRO                                         16
#define NO_PRF                                          0
#define PRF_MODE_PREFERRED                              1
#define PRF_MODE_EXCLUSIVE                              2

/*------------------------------------------------------------------------------------
    OPREVENT.CPP
------------------------------------------------------------------------------------*/
#define TYPE_DIAL_OUT                                   0
#define TYPE_DIAL_IN                                    5
#define TYPE_DIRECT                                     6
#define NO_BONDING                                      0
#define BONDING                                         1
#define AUTO_BONDING                                    0xFF
#define AUTO_NUMBER                                     0xFF

#define UNKNOWN_RESTRICT                                255
#define VOICE                                           1
#define NO_VOICE                                        0
#define UNKNOWN_VOICE                                   255
#define NETWORK_SPECIFIC                                3
#define NUM_TYPE_DEF                                    0xFF
#define PASSWORD                                        0
#define CALLED_NUMBER                                   1
#define CALLING_NUMBER                                  2
#define MEET_PER_MCU                                    1
#define MEET_PER_CONFERENCE                             2
#define MEET_PER_PARTY                                  3
#define MEET_PER_CHANNEL                                4

/*------------------------------------------------------------------------------------
    CONFSTRT.CPP
------------------------------------------------------------------------------------*/
#define STAND_BY_NO                                     0
#define STAND_BY_YES                                    1
#define AUTO_TERMINATE_NO                               0
#define AUTO_TERMINATE_YES                              1
#define RESTRICT                                        27
#define NON_RESTRICT                                    28
#define AUTO                                            255
#define AUDIO_RATE_AUTO                                 255
#define AUDIO_RATE_16KBPS                               29
#define AUDIO_RATE_48KBPS_G722                          25
#define AUDIO_RATE_56KBPS_G722                          24
#define AUDIO_RATE_48KBPS                               21
#define AUDIO_RATE_56KBPS_G711                          19
#define VIDEO_SWITCH                                    1
#define VIDEO_TRANSCODING                               2
#define CONTINUOUS_PRESENCE                             3

#define SOFTWARE_CONTINUOUS_PRESENCE                    5 // Number 4 is in use by Web application !!!
#define ADVANCED_LAYOUTS                                6
#define VIDEO_SWITCH_FIXED                              7
#define VIDEO_SESSION_COP                               8

#define PIC_FORMAT_AUTO                                 255
#define QCIF                                            20
#define CIF                                             21
#define AUTO_CIF_FRAME_RATE                             255
#define CIF_FRAME_RATE_7                                25
#define CIF_FRAME_RATE_10                               24
#define CIF_FRAME_RATE_15                               23
#define CIF_FRAME_RATE_30                               22
#define AUTO_QCIF_FRAME_RATE                            255
#define QCIF_FRAME_RATE_7                               25
#define QCIF_FRAME_RATE_10                              24
#define QCIF_FRAME_RATE_15                              23
#define QCIF_FRAME_RATE_30                              22
#define DYNAMIC                                         255
#define LSD_RATE_300                                    1
#define LSD_RATE_1200                                   2
#define LSD_RATE_4800                                   3
#define LSD_RATE_6400                                   4
#define LSD_RATE_8000                                   5
#define LSD_RATE_9600                                   6
#define LSD_RATE_14400                                  7
#define LSD_RATE_16K                                    8
#define LSD_RATE_24K                                    9
#define LSD_RATE_32K                                    10
#define LSD_RATE_40K                                    11
#define LSD_RATE_48K                                    12
#define LSD_RATE_56K                                    13
#define LSD_RATE_62K                                    14
#define LSD_RATE_64K                                    15
#define VAR_LSD                                         31
#define HSD_DYNAMIC                                     255
#define VAR_HSD                                         1
#define HSD_RATE_64K                                    17
#define HSD_RATE_128K                                   18
#define HSD_RATE_192K                                   19
#define HSD_RATE_256K                                   20
#define HSD_RATE_320K                                   21
#define HSD_RATE_384K                                   22
#define HSD_RATE_512K                                   23
#define HSD_RATE_768K                                   24
#define HSD_RATE_1152K                                  25
#define HSD_RATE_1536K                                  26

/*------------------------------------------------------------------------------------
    END CDRINCLD.h
------------------------------------------------------------------------------------*/

// size of cdrshort record
#define CDR_SIZE_RECORD                                 512
#define CDR_SIZE_RECORD_OLD                             128
#define CDR_SIZE_STREAM                                 0xFDE8 // 65000

#define SECONDARY_CAUSE_ENUM                            91

#define MAX_API_NUM_VER_3                               125 // since the DWORD for RESERVATION had been defined in V3 with this API(107)

#define API_NUMBER                                      675
#define API_NUM_CDR_HUGE_3                              115
#define API_NUM_CDR_HUGE                                157
#define API_NUM_PRESENTATION_MODE                       521
#define API_NUM_LECTURE_SHOW                            145
#define API_NUM_GMT_OFFSET_MINUTES                      363

// size of configurations record
#define SIZE_RECORD                                     2048

#define CDR_EVENT_NOT_EXISTS                            12001

#define MAX_CONECTIONS_IN_MCU                           100
#define MAX_CHANNEL_NUMBER                              30
#define MAX_VIDEO_SOURCE_PARTY                          10
#define MAX_FILE_NAME_LEN                               100
#define MAX_DOS_FILE_NAME_LEN                           9
#define MAX_VERSIONS_HARDWARE                           20
#define MAX_CARD_TYPE_IN_LIST                           30
#define MAX_PARTIES_IN_DISCONNECT_LIST                  4

const WORD H323IFTypeAddition = 100;

#define START_RECORDING_IMMEDIATELY                     1
#define START_RECORDING_UPON_REQUEST                    2

// The macro is deprecated, use function strcpy_safe at SystemFunctions.h
#define SAFE_COPY(src, dst) strcpy_safe(src, dst)
//#define SAFE_COPY(dst, src) if (src) { strncpy(dst, src, sizeof(dst)-1); dst[sizeof(dst)-1] = '\0'; }

#endif
