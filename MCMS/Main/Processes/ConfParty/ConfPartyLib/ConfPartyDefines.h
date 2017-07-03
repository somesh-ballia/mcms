#ifndef __CONFPARTYDEFINES_H___
#define __CONFPARTYDEFINES_H___

#include "ConfPartySharedDefines.h"
#include "AllocateStructs.h"
#include "ResRsrcCalculator.h"

////////////////////////////////////////////////////////////////////////////
//                        Function status
enum EStat
{
  statOK = 0,
  statIllegal,
  statInconsistent,
  statOutOfRange,                   // Parameters are out of range.
  statNoChange,                     // Object is not changed by operation.
  statTout,                         // Time Out
  statBusy,                         // reject req busy
  statIncopatibleVideo,             // Remote opened incompatible video.
  statSecondary,                    // request for audio channels has come before the request for video channels
  statVideoBeforeAudio,             // request for video channels has come before the request for audio channels
  statAsymmetricVideoOK,            // The mux card notifies remote about the asymmetric video protocol
  statInvalidPartyInitParams,
  statAudioInOutResourceProblem,    // Will Initiate KillPort on AudioIn and AudioOut
  statVideoInOutResourceProblem,    // Will Initiate KillPort on VideoIn and VideoOut
  statContentInOutResourceProblem,  // Will Initiate KillPort on ContentIn and ContentOut
  statCCSContentLprSupport,
  statBridgeIsNotEmpty
};

////////////////////////////////////////////////////////////////////////////
//                        MCU INTERNAL PROBLEM ENUMS
// First digit  - Hardware/Connection
enum MipHardWareConn
{
  eMipNoneHw,
  eMipUdp,
  eMipRtp,
  eMipBridge,
  eMipConnectionProcess,
  eMipMux,
  eMipCardManager
};

// Second digit - Media type
enum MipMedia
{
  eMipNoneMedia,
  eMipAudio,
  eMipVideo,
  eMipFecc,
  eMipContent,
  eMipArt
};

// Third digit - Direction
enum MipDirection
{
  eMipNoneDirction,
  eMipIn,
  eMipOut
};

// Forth digit - Timer/Status
enum MipTimerStatus
{
  eMpiNoTimerAndStatus,
  eMipTimer,
  eMipStatusFail,
  eMipChanNotFound
};

// Fifth digit - Action performed
enum MipAction
{
  eMipNoAction,
  eMipConnect,
  eMipOpen,
  eMipDisconnect,
  eMipUpdate,
  eMipClose
};

// Fips140 ConfParty defined values
enum eConfPartyFipsSimulationMode
{
  eInactiveSimulation,
  eFailPartyCipherFipsTest,
  eFailBypassFipsTest
};

////////////////////////////////////////////////////////////////////////////
#define DIALIN                                   0
#define DIALOUT                                  1

#define WAIT_FOR_BRIDGES_COUNTER                 200
#define FORCE_BLOCK                              11

#define BCH_FACTOR                               95       // by Alg requests change the bch factor to 95%
// Limit constants
#define PARTY_SERIALIZE_SIZE                     400

// AmosCapacity constants
#define MAX_CONF_IN_LIST                         800      // 400
#define MAX_TEMPLATES_IN_LIST                    80       // 20

#define MAX_CONF_TEMPLATES_RMX                   400


#define MAX_BILLING_INFO_SIZE                    80
#define MAXIMUM_CONF_DURATION                    168

#define MAX_VIDEO_SOURCE_PARTY                   10
#define MAX_FILE_NAME_LEN                        100

#define MAX_FILES_IN_LIST                        10000

#define MAX_LANG_IN_IVR_SERVICE                  1
#define MAX_IVR_EVENT_IN_FEATURE                 100
#define MAX_IVR_SERV_IN_LIST                     80
#define MAX_DTMF_CODE_NUM                        61
#define MAX_MESSAGE_DURATION_IN_SECONDS          120
#define MAX_PASSWORD_LENGTH                      16

#define H243_NAME_LEN_OLD                        33       // 32 characters + terminator (for party\conf names)
#define H243_NAME_LEN                            81       // 80 characters + terminator (for party\conf names)
#define H243_NAME_LIST_ID_LEN                    120

#define AV_MSG_SERVICE_NAME_LEN                  81       // 20 characters + terminator
#define IVR_MAX_FILE_NAME_LEN                    64

#define DTMF_STRING_LEN                          12
#define MAX_DTMF_LEN_FROM_SRC                    50
#define NEW_FILE_NAME_LEN                        255      // 30 increased To 255 accommodate full paths

#define IVR_MSG_NAME_LEN                         81

#define MAX_CONF_INFO_ITEMS                      3

// Party Status
#define PARTY_RESET_STATUS                       0
#define PARTY_AUDIO_ONLY                         1

// Party sip address types
#define PARTY_SIP_SIPURI_ID_TYPE                 1
#define PARTY_SIP_TELURL_ID_TYPE                 2

// Party h323 alias types
#define PARTY_H323_ALIAS_H323_ID_TYPE            8
#define PARTY_H323_ALIAS_E164_TYPE               7
#define PARTY_H323_ALIAS_URL_ID_TYPE             11
#define PARTY_H323_ALIAS_TRANSPORT_ID_TYPE       12
#define PARTY_H323_ALIAS_EMAIL_ID_TYPE           13
#define PARTY_H323_ALIAS_PARTY_NUMBER_TYPE       14
#define PARTY_H323_ALIAS_DISPLAY_ID_TYPE       	 15

// Party h323 gate-keeper states
#define ADMITTION                                1
#define ADMITTED                                 2
#define DISENGAGE                                3


//CDR_MCCF:
#define NUM_OF_CHANNELS_FOR_MCCF_CDR_INFO        2







// Party h323 channels types
enum EIpChannelType
{
  H225      = 0,
  SIGNALING = 0,
  H245      = 1,
  SDP       = 1,
  AUDIO_IN,
  AUDIO_OUT,
  VIDEO_IN,
  VIDEO_OUT,
  AUDIO_CONT_IN,                                          // not supported yet (used for calculation)
  AUDIO_CONT_OUT,                                         // not supported yet
  VIDEO_CONT_IN,
  VIDEO_CONT_OUT,
  FECC_IN,
  FECC_OUT,
  BFCP_IN,
  BFCP_OUT,
  BFCP,
  BFCP_UDP,
  IP_CHANNEL_TYPES_NUMBER                                 // max channel types, MUST STAY LAST!!!
};

enum EMediaProducerStatus
{
  CHANNEL_INACTIVE = 0,
  CHANNEL_ACTIVE   = 1
};

// Party IVR states
#define PARTY_WAITS_FOR_PRIVATE_OPERATOR_ASSISTANCE  1
#define PARTY_WAITS_FOR_PUBLIC_OPERATOR_ASSISTANCE   2

#define DTMF_OPERATOR_ASSISTANCE_REQ                 1
#define PASSWORD_FAILURE_OPERATOR_ASSISTANCE_REQ     2

enum eSipFactoryType
{
  eNotSipFactory = 0,
  e200SipFactory,
  e302SipFactory
};

#define RESERVATION_DATABASE                         6
#define MEETING_ROOM_DATABASE                        8
#define PROFILES_DATABASE                            9
#define CONF_TEMPLATES_DATABASE                      10

#define H323_CLEAR_INDICATION                        16

#define SIP_CLEAR_INDICATION                         25

#define SET_VIDEO_SRC                                13   // Set video source

#define DATA_ACTIVITY                                15   // Set data activity of ongoing conference
#define SET_DATA_SRC                                 15   // Set data source
#define LOOPBACK_PARTY                               17
#define MOVE_PARTY                                   20
#define SET_AUDIO_VOLUME                             26   // Volume control
#define CHANGE_LECTURE_MODE                          28
#define CHANGE_MESSAGE_OVERLAY                       29
#define CHANGE_PARTY_MESSAGE_OVERLAY                 30
#define BLOCK_NOT_MUTE_PARTY                         31
#define SET_AGC                                      56

// Private layout
#define SET_PRIVATE_VIDEO                            57
#define SET_PARTY_CONF_OR_PRIVATE_LAYOUT             58   // instead of this the next 2
#define SET_PARTY_PRIVATE_TO_CONF_LAYOUT             59

// Auto Layout
#define SET_AUTO_LAYOUT                              61
#define SET_LISTENING_AUDIO_VOLUME                   73   // Listening Volume control

#define SET_START_RECORDING                          95
#define SET_STOP_RECORDING                           96
#define SET_RESUME_RECORDING                         97
#define SET_PAUSE_RECORDING                          98
#define START_CONTENT                                99   // for Call Generator
#define STOP_CONTENT                                 100  // for Call Generator

#define BOTH_PASS                                    0
#define DO_NOT_ALLOCATE                              3

// AV msg service action types
#define NEW_AV_MSG                                   90
#define DEL_AV_MSG                                   91
#define UPDATE_AV_MSG                                92
#define SET_DEFAULT_AV_MSG_SERVICE                   93

#define WAIT_FOR_OPER_NONE                           0
#define WAIT_FOR_OPER_ON_REQ_PRIVATE                 2
#define WAIT_FOR_OPER_ON_REQ_PUBLIC                  3

// Format type
#define OPERATOR_MCMS                                255
#define CONFIG                                       254

// IVR service  action types
#define NEW_IVR_LANGUAGE                             350
#define WARNING_MASK                                 0x80000000
#define MAX_DELIMETER_LEN                            3

// Size of configurations record
#define SIZE_RECORD                                  2048

// Video rates  - parallel with Xfer in file CONFSTRT.CPP
#define VIDEO_RATE_128                               13
#define VIDEO_RATE_256                               15
#define VIDEO_RATE_384                               6
#define VIDEO_RATE_512                               23
#define VIDEO_RATE_768                               24
#define VIDEO_RATE_E1                                12
#define VIDEO_RATE_T1                                29

//SVC
#define VIDEO_RELAY_MIN_CALL_RATE_THRESHOLD			128
#define VIDEO_RELAY_MAX_CALL_RATE_THRESHOLD			4096
#define AVC_SVC_VSW_MAX_CALL_RATE_THRESHOLD			4096
#define AVC_SVC_VSW_MIN_CALL_RATE_THRESHOLD			384
#define AVC_SVC_VSW_MOBILE_MIN_CALL_RATE_THRESHOLD	192
#define AVC_SVC_VSW_QVGA_MIN_CALL_RATE_THRESHOLD	384
#define AVC_SVC_VSW_CIF_MIN_CALL_RATE_THRESHOLD		512
#define AVC_SVC_VSW_VGA_MIN_CALL_RATE_THRESHOLD		512
#define AVC_SVC_VSW_SD_MIN_CALL_RATE_THRESHOLD		768
#define AVC_SVC_VSW_HD_MIN_CALL_RATE_THRESHOLD		832
//softMcu max avc/mix call rate
#define AVC_SOFT_MCU_MAX_CALL_RATE                  4096

// constant for conference Id and party Id
#define HALF_MAX_DWORD                               0x7FFFFFFD
#define H243                                         1

// ongoing conferences statuses
#define CONFERENCE_EMPTY                             0x1
#define CONFERENCE_SINGLE_PARTY                      0x2
#define CONFERENCE_NOT_FULL                          0x4
#define CONFERENCE_RESOURCES_DEFICIENCY              0x8
#define CONFERENCE_BAD_RESOURCES                     0x10
#define PROBLEM_PARTY                                0x20
#define PARTY_REQUIRES_OPERATOR_ASSIST               0x40
#define CONFERENCE_CONTENT_RESOURCES_DEFICIENCY      0x80
#define CONFERENCE_TERMINATING                       0x80000000

#define INVALID_PERMISSION                           142
#define REQUEST_DENIED                               143
#define UNDEFINED_REASON                             144
#define CALLER_NOT_REGISTERED                        145


// Start recording policy
#define  START_RECORDING_IMMEDIATELY                 1
#define  START_RECORDING_UPON_REQUEST                2

// Num_Plan
#define ISDN_PLAN                                    1
#define TELEPHONY_PLAN                               2
#define DATA_PLAN                                    3
#define TELEX_PLAN                                   4
#define NATIONAL_PLAN                                8
#define PRIVATE_PLAN                                 9

// video card type
#define VIDEO_STAND_ALONE_CARD                       0
#define VIDEO_MASTER_CARD                            1
#define VIDEO_SLAVE_CARD                             2

#define PREFER_BOTH_DIRECT                           1
#define EXCLUSIVE_BOTH_DIRECT                        2
#define EXCLUSIVE_OUTGOING                           3
#define PREFER_OUTGOING                              4

#define MCU_LOW                                      1
#define MCU_HIGH                                     2

#define SPAN_T1                                      0
#define SPAN_E1                                      1
#define SPAN_BRI                                     2
#define SPAN_T1CAS                                   3

#define SPAN_H323_LAN                                3
#define SPAN_H323_IPOATM                             4
#define SPAN_H323_LANEMU                             5
#define UNKNOWN_SPAN                                 222

#define THRESHOLD_0                                  110  /* default value */
#define THRESHOLD_1                                  111
#define THRESHOLD_2                                  112
#define THRESHOLD_3                                  113

#define SERVICE_PRI                                  0
#define SERVICE_SWITCHED_56                          1
#define SERVICE_LEASED_LINES                         2
#define SERVICE_LEASED_LINES_31                      3
#define SERVICE_H323                                 4
#define SERVICE_H323_LAN                             5
#define SERVICE_H323_IPOATM                          6
#define SERVICE_H323_LANEMU                          7
#define SERVICE_T1_CAS                               8
#define UNKNOWN_SERVICE                              222

#define PHYSICAL_LINK_DISCONNECTED                   0
#define PHYSICAL_LINK_CONNECTED                      1

#define ILMI_PROTOCOL_DISABLED                       0
#define ILMI_PROTOCOL_DISCONNECTED                   1
#define ILMI_PROTOCOL_CONNECTING                     2
#define ILMI_PROTOCOL_CONNECTED                      3

#define SIGNALING_LINK_STATUS_DISCONNECTED           0
#define SIGNALING_LINK_STATUS_CONNECTED              1

// nfas card types
#define NFAS_TYPE_OFF                                0
#define NFAS_TYPE_PRIMARY                            1
#define NFAS_TYPE_BACKUP                             2
#define NFAS_TYPE_ORDINARY                           3
#define NFAS_ID_OFF                                  ((WORD)0xFFFF)

// units configuration
#define PARTIES_UNIT_TYPE                            0
#define BRIDGE_UNIT_TYPE                             1
#define CONTROLLER_UNIT_TYPE                         2

#define MUX_PORT_4_384                               1
#define MUX_PORT_2_768                               2
#define MUX_PORT_1_E1                                3
#define MUX_PORT_3_384                               4

// audioBridge configuration
#define AUD_BRDG_NORMAL                              0
#define AUD_BRDG_1_30                                1    // 1 conf, total 30 parties
#define AUD_BRDG_4_16                                2    // 4 confs, total 16 parties
#define AUD_BRDG_6_12                                3    // 6 confs, total 12 parties
#define AUD_BRDG_NOT_FIND                            0xFF

// ATM service types
#define ATM_25                                       0
#define ATM_155                                      1

#define CONF_WELCOME                                 1
#define CONF_ATTENDED                                2

// IVR event opcodes
// Language menu feature 1-9
#define IVR_EVENT_GET_LANGUAGE                       1
#define IVR_EVENT_LANGUAGE_RETRY                     2
// Conference Password feature 10-19
#define IVR_EVENT_GET_CONFERENCE_PASSWORD            10
#define IVR_EVENT_CONFERENCE_PASSWORD_RETRY          11
#define IVR_EVENT_GET_DIGIT                          12
// Personal PIN code feature 20-29
#define IVR_EVENT_GET_PIN_CODE                       20
#define IVR_EVENT_PIN_CODE_RETRY                     21
// Operator Assistance feature 30-39
#define IVR_EVENT_WAIT_FOR_OPERATOR_MESSAGE          30
#define IVR_EVENT_SYSTEM_DISCONNECT_MESSAGE          31
// Welcome feature 40-49
#define IVR_EVENT_WELCOME_MSG                        40
#define IVR_EVENT_ENTRANCE_MSG                       41
// Leader feature 50-59
#define IVR_EVENT_GET_LEADER_IDENTIFIER              50
#define IVR_EVENT_GET_LEADER_PASSWORD                51
#define IVR_EVENT_LEADER_PASSWORD_RETRY              52
// Operator Assistance feature 60-69
#define IVR_EVENT_WAIT_FOR_OPER_ON_PARTY_PRIVATE_REQ 60
#define IVR_EVENT_WAIT_FOR_OPER_ON_PARTY_PUBLIC_REQ  61
#define IVR_EVENT_WAIT_FOR_OPER_ON_CONF_PWD_FAIL     62
#define IVR_EVENT_WAIT_FOR_OPER_ON_CHAIR_PWD_FAIL    63
#define IVR_EVENT_WAIT_FOR_OPER_ON_NID_FAIL          65
// General feature 100 - 200
#define IVR_EVENT_FIRST_PARTY                        100
#define IVR_EVENT_SECURE_ON                          101
#define IVR_EVENT_SECURE_OFF                         102
#define IVR_EVENT_LOCK_ON                            103
#define IVR_EVENT_LOCK_OFF                           104
#define IVR_EVENT_PARTY_ENTER                        105
#define IVR_EVENT_PARTY_EXIT                         106
#define IVR_EVENT_ENTER_DEST_NUM                     107
#define IVR_EVENT_ILLEGAL_DEST_NUM                   108
#define IVR_EVENT_CONF_END                           112
#define IVR_EVENT_RECORD_START                       113
#define IVR_EVENT_RECORD_END                         114
#define IVR_EVENT_RECORD_PAUSE                       115
#define IVR_EVENT_RECORD_NOT_AVAILABLE               116
#define IVR_EVENT_PLAYBK_START                       117
#define IVR_EVENT_PLAYBK_END                         118
#define IVR_EVENT_PLAYBK_PAUSE                       119
#define IVR_EVENT_PLAYBK_NOT_AVAILABLE               120
#define IVR_EVENT_CONF_ON_HOLD                       121
#define IVR_EVENT_MENU_LEADER                        122
#define IVR_EVENT_MENU_SIMPLE                        123
#define IVR_EVENT_BILLING_NUM                        124
#define IVR_EVENT_NUM_PARTIES_IN_CONF_BEGIN          125
#define IVR_EVENT_NUM_PARTIES_IN_CONF_END            126
#define IVR_EVENT_END_TIME_ALERT                     127
#define IVR_EVENT_CURRENT_SPEAKER                    128
#define IVR_EVENT_NO_RESOURCES                       129
#define IVR_EVENT_ADD_ME_QA                          130
#define IVR_EVENT_REMOVE_ME_QA                       131
#define IVR_EVENT_CLEAR_QA                           132
#define IVR_EVENT_NEXT_QA                            133
#define IVR_EVENT_EMPTY_QA                           134
#define IVR_EVENT_REQUIRES_LEADER                    135
#define IVR_EVENT_FIRST_TO_JOIN                      136
#define IVR_EVENT_MENU_INVITE                        137
#define IVR_EVENT_MENU_VOTING                        138
#define IVR_EVENT_MENU_QA                            139
#define IVR_EVENT_INVITE_DISABLE                     140
#define IVR_EVENT_INVITE_CALL                        141
#define IVR_EVENT_INVITE_NO_RESOURCE                 142
#define IVR_EVENT_INVITE_NO_PERMISSION               143
#define IVR_EVENT_ILLEGAL_NO                         144
#define IVR_EVENT_CONF_LOCK                          145
#define IVR_EVENT_OPERATOR_ASK_ATTENTION             146
#define IVR_EVENT_CHANGE_PWDS_MENU                   147
#define IVR_EVENT_CHANGE_PWDS_CONF                   148
#define IVR_EVENT_CHANGE_PWDS_LEADER                 149
#define IVR_EVENT_CHANGE_PWD_CONFIRM                 150
#define IVR_EVENT_CHANGE_PWD_INVALID                 151
#define IVR_EVENT_CHANGE_PWD_OK                      152
#define IVR_EVENT_MUTE_ALL_ON                        153
#define IVR_EVENT_MUTE_ALL_OFF                       154
#define IVR_EVENT_CHAIR_DROPPED                      155
#define IVR_EVENT_SELF_MUTE                          156
#define IVR_EVENT_SELF_UNMUTE                        157
#define IVR_EVENT_MENU_LEADER_2                      158
#define IVR_EVENT_MENU_SIMPLE_2                      159
#define IVR_EVENT_ROLLCALL_REC                       160
#define IVR_EVENT_ROLLCALL_VERIFY_REC                161
#define IVR_EVENT_ROLLCALL_CONFIRM_REC               162
#define IVR_EVENT_ROLLCALL_ENTER                     163
#define IVR_EVENT_ROLLCALL_EXIT                      164
#define IVR_EVENT_ROLLCALL_BEGIN_OF_NAME_REVIEW      165
#define IVR_EVENT_ROLLCALL_END_OF_NAME_REVIEW        166
#define IVR_EVENT_MAX_PARTICIPANTS                   167
// Numeric conference id feature 168-169
#define IVR_EVENT_GET_NUMERIC_ID                     168
#define IVR_EVENT_NUMERIC_ID_RETRY                   169
// Mute noisy line feature 170-176
#define IVR_EVENT_NOISY_LINE_HELP_MENU               170
#define IVR_EVENT_NOISY_LINE_MUTE                    171
#define IVR_EVENT_NOISY_LINE_ADJUST                  172
#define IVR_EVENT_NOISY_LINE_DISABLE                 173
#define IVR_EVENT_NOISY_LINE_UNMUTE                  174
#define IVR_EVENT_NOISY_LINE_UNMUTE_MESSAGE          175
#define IVR_EVENT_PLAY_NOISY_LINE_MESSAGE            176
#define IVR_EVENT_RECORDING_IN_PROGRESS              177
#define IVR_EVENT_RECORDING_FAILED                   178
// Recording feature 180-189
#define IVR_EVENT_REC_STARTED                        180
#define IVR_EVENT_REC_STOPPED                        181
#define IVR_EVENT_REC_PAUSED                         182
#define IVR_EVENT_REC_RESUMED                        183
#define IVR_EVENT_REC_ILLEGAL_ACCOUNT                184
#define IVR_EVENT_REC_UNAUTHORIZE                    185
#define IVR_EVENT_REC_GENERAL                        186
#define IVR_EVENT_REC_USER_OVERFLOW                  187
// Playback feature 190-195
#define IVR_EVENT_PLCK_SESSIONID                     190
#define IVR_EVENT_PLCK_SESSIONID_ERROR               191
#define IVR_EVENT_PLCK_SESSION_END                   192
#define IVR_EVENT_PLCK_MENU                          193
#define IVR_EVENT_PLCK_MENU_NOTIFY                   194
#define IVR_EVENT_PLCK_PAUSED                        195
#define IVR_EVENT_PLCK_RESUME                        196
// Play tones (for video invite / GW)  197-198
#define IVR_EVENT_PLAY_DIAL_TONE                     197
#define IVR_EVENT_PLAY_RINGING_TONE                  198
// IVR event for noew vide RSRC
#define IVR_EVENT_NO_VIDEO_RESOURCES                 199
// Play blip tone when cascade link is connected
#define IVR_EVENT_BLIP_ON_CASCADE_LINK               200
#define IVR_EVENT_INVITE_PARTY                        201
#define IVR_EVENT_REINVITE_PARTY                      202
#define IVR_EVENT_PLAY_BUSY_MSG                       203
#define IVR_EVENT_PLAY_NOANSWER_MSG                  204
#define IVR_EVENT_PLAY_WRONG_NUMBER_MSG              205



// DTMF opcodes
#define DTMF_OPER_ASSISTANCE_PRIVATE                 1
#define DTMF_OPER_ASSISTANCE_PUBLIC                  2
#define DTMF_SELF_MUTE                               3
#define DTMF_SELF_UNMUTE                             4
#define DTMF_LOCK_CONF                               5
#define DTMF_UNLOCK_CONF                             6
#define DTMF_INC_SELF_VOLUME                         7
#define DTMF_DEC_SELF_VOLUME                         8
#define DTMF_MUTE_ALL_BUT_X                          9
#define DTMF_UNMUTE_ALL_BUT_X                        10
#define DTMF_BLOCK_PARTY                             11
#define DTMF_UNBLOCK_PARTY                           12
#define DTMF_OPER_ASSISTANCE_PRIVATE_CANCEL          13
#define DTMF_OPER_ASSISTANCE_PUBLIC_CANCEL           14
#define DTMF_CONF_TERMINATE                          15
#define DTMF_START_ONHOLD                            16
#define DTMF_STOP_ONHOLD                             17
#define DTMF_STARTVOTE                               18
#define DTMF_STOPVOTE                                19
#define DTMF_NEWVOTE                                 20
#define DTMF_CANCELVOTE                              21
#define DTMF_MUTE_INCOMING_PARTIES                   22
#define DTMF_UNMUTE_INCOMING_PARTIES                 23
#define DTMF_ADDME_QA                                24
#define DTMF_REMOVEME_QA                             25
#define DTMF_MARKNEXT_QA                             26
#define DTMF_UNMARKCUR_QA                            27
#define DTMF_CLOSE_QA                                28
#define DTMF_CLEAR_QA                                29
#define DTMF_PLAY_MENU                               30
#define DTMF_SECURE_CONF                             31
#define DTMF_UNSECURE_CONF                           32
#define DTMF_CHANGE_PASSWORD                         33
#define DTMF_MUTE_ALL_ON                             34
#define DTMF_MUTE_ALL_OFF                            35
#define DTMF_CHANGE_TO_LEADER                        36
#define DTMF_SHOW_GATHERING                          37
#define DTMF_START_INVITE                            40
#define DTMF_END_FEATURE                             41
#define DTMF_MOVE_AND_REDIAL                         42
#define DTMF_DISCONNECT_AND_REDIAL                   43
#define DTMF_MOVE_AND_END                            44
#define DTMF_DELETE_AND_END                          45
#define DTMF_CLEAR_NUMB                              46
#define DTMF_ENABLE_ROLL_CALL                        47
#define DTMF_DISABLE_ROLL_CALL                       48
#define DTMF_ROLL_CALL_REVIEW_NAMES                  49
#define DTMF_ROLL_CALL_STOP_REVIEW_NAMES             50
#define DTMF_START_DIALOUT                           51
#define DTMF_START_VC                                52
#define DTMF_INC_LISTEN_VOLUME                       53
#define DTMF_DEC_LISTEN_VOLUME                       54
#define DTMF_OVERRIDE_MUTE_ALL                       55
#define DTMF_SHOW_PARTICIPANTS                       56
#define DTMF_REQUEST_TO_SPEAK                        57
#define DTMF_START_PCM                               58
#define DTMF_START_VENUS                             59
#define DTMF_INVITE_PARTY                            60
#define DTMF_DISCONNECT_INVITED_PARTICIPANT          61

// Recording system opcodes
#define DTMF_RECORDING_PAUSE                         200
#define DTMF_RECORDING_RESUME                        201
#define DTMF_RECORDING_STOP                          202

#define DTMF_NOISY_LINE_MUTE                         121
#define DTMF_NOISY_LINE_UNMUTE                       122
#define DTMF_NOISY_LINE_ADJUST                       123
#define DTMF_NOISY_LINE_DISABLE                      124
#define DTMF_NOISY_LINE_HELP_MENU                    125
#define DTMF_PLAYBACK_PAUSE                          300
#define DTMF_PLAYBACK_RESUME                         301
#define DTMF_PLAYBACK_SKIP_FW                        302
#define DTMF_PLAYBACK_SKIP_BW                        303
#define DTMF_PLAYBACK_FW_TO_END                      304
#define DTMF_PLAYBACK_RESTART                        305
#define DTMF_PLAYBACK_CLOSE                          306
#define DTMF_PLAYBACK_MENU                           307


// Recording Link opcodes
#define DTMF_START_RESUME_RECORDING                  126
#define DTMF_STOP_RECORDING                          127
#define DTMF_PAUSE_RECORDING                         128


// general DTMF
#define DTMF_DUMP_TABLE                              150

// IVR - External database options
#define IVR_EXTERNAL_DB_NONE                         0
#define IVR_EXTERNAL_DB_NUMERIC_ID                   1
#define IVR_EXTERNAL_DB_PARTICIPANT_PASSWORD         2
#define IVR_EXTERNAL_DB_CHAIR_PASSWORD               3

// DTMF authorization groups
#define DTMF_USER_ACTION                             10
#define DTMF_LEADER_ACTION                           20

// H.323 - GateKeeper Call State
typedef enum
{
  eGKCallNone  = 0xFF,
  eGKAdmission = 0,
  eGKAdmitted,
  eGKDisengage,
}eGKCallState;

enum IVR_ENTRY_TYPE
{
  REQUEST_PASSWORD = 0,
  REQUEST_DIGIT,
  PLAY_AUDIO_EXTERNAL,
  COLLECT_DIGITS_EXTERNAL,
  NO_REQUEST
};

enum  eQualityH263Mode
{
  eQualityH263ModeAuto = 0,
  eQualityH263ModeMotion,
  eQualityH263ModeSharpness,
  eQualityH263ModeUnknown // MUST be last
};

#define API_NUM_CONFPW_AS_LEADERPW  256
#define API_NUM_ENTRY_Q_VERSION     261
#define API_NUM_GENERAL_MSGS        265
#define API_NUM_BILLING_CODE        267
#define API_NUM_ROLL_CALL_VER1      303
#define API_NUM_IVR_PASSWORD        309
#define API_NUM_VIDEO_VER1          355
#define API_NUM_PRESENTATION_MODE   521
#define API_NUMBER_CONFERENCE_ID    523
#define API_NUM_AD_HOC              527
#define API_NUM_MUTE_NOISY_LINE     593
#define API_NUM_REC_PLYBCK_TYPE     602

#define MAX_CHNLS_IN_PARTY          30
#define H243_NAME_LEN_DEFINED_AGAIN 81


const WORD LOCAL              = 1;
const WORD REMOTE             = 2;
const WORD REMOTEVIDEO        = 3;
const WORD LOCALVIDEO         = 4;

// party media connection modes
const WORD PARTY_SETUP_MODE   = 0;
const WORD PARTY_INCONF_MODE  = 1;
const WORD PARTY_MESSAGE_MODE = 2;
const WORD PARTY_SILENCE_MODE = 3;


// request sources to conf
const WORD OPERATOR           = 1;
const WORD PARTY              = 2;
const WORD AUDCNTL            = 3;
const WORD CHAIRMAN           = 4;
const WORD MCMS               = 5;
const WORD ALL                = 6;
// operator request by id workaround- mute/unmute media requests from OPERATOR source were hard-coded to use party name only
const WORD  OPERATOR_REQ_BY_ID = 7;

// video request types
const WORD SEEMEPARTY         = 1;
const WORD SEEMEALL           = 2;
const WORD VOICEACTIVATEALL   = 3;
const WORD VOICEACTIVATEPARTY = 4;

// lobby conference id
const DWORD LOBBY_CONF_ID     = 0x7FFFFFFE;

// Minimum Party Resource Id for Dial In rejected calls
#define CONFPARTY_MIN_REJECT_ID        (RSRC_ALLOCATOR_MAX_RSRC_PARTY_ID + 1)

// site names changed to unicode: 32 (chars) x 4 (max byte for UTF-8 char) + 1 ('\0') = 129
#define MAX_SITE_NAME_ARR_SIZE         129
#define MAX_TEXT_LEN                   400
#define MASK_LENGTH_CHAR               4

#define AUDIO_MONO_NUM_OF_CHANNELS     1
#define AUDIO_STEREO_NUM_OF_CHANNELS   2

#define AUDIO_SAMPLE_RATE_8KHZ         8
#define AUDIO_SAMPLE_RATE_16KHZ        16
#define AUDIO_SAMPLE_RATE_32KHZ        32
#define AUDIO_SAMPLE_RATE_48KHZ        48


/* mask for channels audio/video/content */
#define MASK_AUDIO		0x01
#define MASK_VIDEO		0x02
#define MASK_CONTENT	0x04

#define MASK_RESET_SDES_FOR_CISCO		0x01
#define MASK_RESET_SDES_FOR_MICROSOFT	0x02
#define MASK_RESET_SDES_FOR_POLYCOM 	0x04
#define MASK_RESET_SDES_FOR_ALL	        0xFF
typedef enum
{
  eNoDirection = 0,
  eMediaIn,
  eMediaOut,
  eMediaInAndOut
} EMediaDirection;

typedef enum
{
  eOff = 0,
  eOn
} EOnOff;

typedef enum
{
  eNoChangeMediaType        = 0,
  eChangeContentOnly        = 0x01,
  eCanChangeVideoAndContent = 0x11,
} EChangeMediaType;


inline EChangeMediaType& operator|=(EChangeMediaType& left, const EChangeMediaType right)
{
  int temp = (int)left;
  temp |= (int)right;
  left  = (EChangeMediaType)temp;
  return left;
}

typedef enum
{
  eDummyType = 0,
  eOnGoingConf,
  eMeetingRoom,
  eAdHoc
} ETargetConfType;

// Move party types
typedef enum
{
  eMoveDummy	=0,
  eMoveDefault = 1,
  eMoveAttend,
  eMoveBack,
  eMoveOnHold,
  eMoveIntoIvr,
  eMoveBackIntoIvr
} EMoveType;

// H239
enum eContentState
{
  eStreamOff = 0,
  eStreamOn,
  eWaitToSendStreamOn,
  eNoChannel,
  eSendStreamOn,
  eSendStreamOff
};

// //////////////////////////////////////////////////////////////////////////
// CONTENT DEFINES
const BYTE PeopleLabel                                              = 0x01;
const BYTE ContentLabel                                             = 0x02;
const BYTE RoleLabelTerminatorMask                                  = 0x20;
const BYTE RateChangeParameterID_StartBitMask                       = 0x80;
const BYTE RateChangeParameterID_EndBitMask                         = 0x40;

const WORD CONTCONNECT                                              = 10001;
const WORD CONTDISCONNECT                                           = 10002;
const WORD NEWCONTENTRATE                                           = 10003;
const WORD NEWCONTENTSCM                                            = 10004;
const WORD CHANGERATETOUT                                           = 10008;
const WORD UPDATEONLECTUREMODE                                      = 10009;
const WORD UPDATEEXCLUSIVECONTENT                                   = 10010;
const WORD DISABLE_CONTENT_INTRA_SUPPRESS_AFTER_START_CONTENT_TIMER = 10011;
const WORD CHANGE_STATE                                             = 10021;
const WORD CHANGE_PRESENTATION_RATE                                 = 10022;
const WORD CHANGE_PRESENTATION_SPEAKER                              = 10023;
const WORD SET_PARTY_RATE                                           = 10024;
const WORD UPDATE_MUX_DESC                                          = 10025;
const WORD NO_ROLE_PROVIDER                                         = 10026;
const WORD PROVIDER_IDENTITY                                        = 10027;
const WORD MEDIA_PRODUCER                                           = 10028;
const WORD SEND_FREEZE                                              = 10031;
const WORD SEND_REFRESH                                             = 10032;
const WORD CONTENTREFRESHTOUT                                       = 10033;
const WORD SEND_RATE_CHANGE_DONE                                    = 10034;
const WORD FORWARD_CONTENT_TOKEN_MSG                                = 10035;
const WORD CONTENTSLAVELINKINTRATOUT   								= 10036;
const WORD UPDATEEXCLUSIVECONTENTMODE  								= 10037;
const WORD START_CONTENT_LOAD_TIMER_01 								= 10038;
const WORD START_CONTENT_LOAD_TIMER_02 								= 10039;
const WORD START_CONTENT_LOAD_TIMER_03 								= 10040;

const WORD ACQUIRE_CONTENT_TOKEN                                    = 101;
const WORD RELEASE_CONTENT_TOKEN                                    = 102;
const WORD WITHDRAW_CONTENT_TOKEN                                   = 103;
const WORD WITHDRAW_ACK                                             = 104;
const WORD ROLE_PROVIDER_INFO                                       = 105;
const WORD NO_ROLE_PROVIDER_TOUT                                    = 106;
const WORD DROP_CONTENT_TOKEN_HOLDER                                = 107;
const WORD PARTY_DISCONNECTS                                        = 108;
const WORD WITHDRAW_CONTENT_TOKEN_TOUT                              = 109;
const WORD ACQUIRE_ACK_FROM_MASTER                                  = 110;
const WORD RATE_CHANGE_FROM_MASTER                                  = 111;
const WORD SLAVE_ACQUIRE_CONTENT_TOKEN                              = 112;
const WORD START_CONTENT_DELAY                                      = 113;
const WORD QUERY_CONTENT_TOKEN										= 114;
const WORD DROP_LAST_CONTENT_TOKEN_REQUESTER                        = 115;
const WORD REMOTELPRVID                                             = 3;
const WORD LOCALLPRVID                                              = 4;



// Timers Times
#define  NO_ROLE_PROVIDER_TIME                10 * SECOND
#define  NO_ROLE_PROVIDER_TIME_AFTER_WITHDRAW 12 * SECOND
#define WITHDRAW_RETRANSMIT_TIME              5 * SECOND
#define CHANGERATE_TIME                       10 * SECOND

// Party Timers
#define BRIDGES_DISCONNECT_TIME               25
#define MPL_DISCONNECT_TIME                   62 //BRIDGE-13688 vngfe-8514 - 62 instead of 30
#define RA_DISCONNECT_TIME                    30

////////////////////////////////////////////////////////////////////////////
//                        SD Feature
#define MAX_CHANNELS_FOR_CIF_MOTION           6
#define MAX_CHANNELS_FOR_SD15_MOTION          12
#define MAX_CHANNELS_FOR_SD30_MOTION          30
#define MAX_CHANNELS_FOR_CIF_SHARPNESS        4
#define MAX_CHANNELS_FOR_SD15_SHARPNESS       8
#define MAX_CHANNELS_FOR_SD30_SHARPNESS       30

typedef struct
{
  Eh264VideoModeType videoModeType;
  long               profileValue;
  long               levelValue;
  long               maxMBPS;
  long               maxFS;
  long               maxDPB;
  long               maxBR;
  long               maxCPB;
  long               maxStaticMbps;
} H264VideoModeDetails;

typedef struct
{
  Eh264VideoModeType videoModeType;
  DWORD              maxWidth;
  DWORD              maxHeight;
  DWORD              maxFrameRate;
  DWORD              aspectRatio;  //EVideoResolutionAspectRatio
  DWORD              minBitRate;
  DWORD              maxBitRate;
  DWORD              maxNumOfPixels;


} MsSvcVideoModeDetails;

//N.A. DEBUG VP8
typedef struct
{
	Eh264VideoModeType videoModeType;
	DWORD              maxWidth;
	DWORD              maxHeight;
	DWORD              maxFrameRate;
	DWORD              aspectRatio;
	long               maxFS;
	long               maxMBPS;
	DWORD              maxNumOfPixels;
	DWORD              minBitRate;
	DWORD              maxBitRate;
}VP8VideoModeDetails;

// CAM request type
#define EVENT_CONF_REQUEST    1
#define EVENT_PARTY_REQUEST   2
#define EVENT_PLAY_MSG        3
#define EVENT_RECORD_ROLLCALL 4
#define EVENT_STOP_MSG        5
#define EVENT_STOP_ROLLCALL_RECORDING  6
#define EVENT_STOP_ROLLCALL_RECORDING_ACK_TIMER  7

// LPR
#define NUM_OF_LPR_CAPS       2
#define LPR_TABLE_LEN         16

typedef struct
{
  APIU32 lossProtection;
  APIU32 mtbf;
  APIU32 congestionCeiling;
  APIU32 fill;
  APIU32 modeTimeout;
} lPRModeChangeParams;

typedef struct
{
  APIU32 versionID;
  APIU32 minProtectionPeriod;
  APIU32 maxProtectionPeriod;
  APIU32 maxRecoverySet;
  APIU32 maxRecoveryPackets;
  APIU32 maxPacketSize;
}lprCapCallStruct;

typedef struct
{
  unsigned int bitRate;                             // bit rate in Kbps
  unsigned int numData;                             // number of data packets in the group
  unsigned int numRecovery;                         // number of recovery packets in the group
  unsigned int packetLen;                           // max packet length in bytes
  unsigned int protectionPeriod;                    // protection period in mS
  unsigned int mtbf;                                // mean time between failure in mS
} LPRParams;

typedef struct
{
  unsigned int protection;                          // % loss protection
  unsigned int minMTBF;                             // minimum MTBF this table supports
  LPRParams    params[LPR_TABLE_LEN];
} LPRRateParams;

extern LPRRateParams gLPRParamTable[];
extern int           gLPRParamTableSize;
extern LPRParams     gLPRMaxParamTable;

const BYTE CASCADE_NONE = 0;
const BYTE CASCADE_GW   = 1;
const BYTE CASCADE_MCU  = 2;
typedef enum
{
  eCopConstRsrcNone = -1,
  eEncoderLevel1    = 0,                            // highest level encoder
  eEncoderLevel2,
  eEncoderLevel3,
  eEncoderLevel4,
  eVswEncoder,
  eVswDecoder,
  ePcmEncoder,
  eLectureDecoder,
  ePcmManager,

  eLastCopConstRsrcs
} ECopConstRsrcs;

typedef struct
{
  DWORD                 connectionId;
  eLogicalResourceTypes logicalRsrcType;
  DWORD                 rsrcEntityId;
} CopRsrcDesc;

typedef struct
{
  CopRsrcDesc constRsrcs[eLastCopConstRsrcs];
  CopRsrcDesc dynamicDecoders[MAX_NUM_COP_DYNAMIC_RSRCS];
  DWORD       pcmMenuId;
} CopRsrcsParams;

typedef enum
{
  eLevelEncoderType,
  ePcmEncoderType,
  eVswEncoderType,
  eDynamicDecoderType,
  eVswDecoderType,
  eLectureDecoderType,
  eLastCopRsrcType
} ECopRsrcType;


typedef enum
{
  ePcmIllegalImageParams = -1,
  e320x240_4x3,
  e640x480_4x3,
  e1024x576_16x9
} pcmImageParams;

typedef struct
{
  ERtvVideoModeType videoModeType;
  unsigned long              Width;
  unsigned long              Height;
  unsigned long              FR;
} RTVVideoModeDetails;

typedef struct
{
  Eh264VideoModeType H264VideoModeType;
  ERtvVideoModeType  RtvVideoModeType;
} H264ToRTVVideoModes;

typedef enum
{
  eCascadeNone,
  eCascadeMasterToRmx,
  eCascadeMasterToRmx1000,
  eCascadeMasterToMgc,
  eCascadeSlaveToRmxSupportSmartCascade,
  eCascadeSlaveToRmx1000SupportSmartCascade,
  eCascadeSlaveToMgcNotSupportSmartCascade,
  eCascadeSlaveToRmxNotSupportSmartCascade,
  eCascadeSlaveToRmx1000NotSupportSmartCascade
} ECascadePartyType;

typedef struct{
	DWORD                 connectionId;
	eLogicalResourceTypes logicalRsrcType;
	DWORD                 rsrcEntityId;
	eVideoPartyType       videoPartyType;
	DWORD                 monitorRsrcPartyId;
	WORD                  isFaulty;

}ContentXcodeRsrcDesc;

typedef std::map <eXcodeRsrcType, ContentXcodeRsrcDesc*> XCODE_RESOURCES_MAP;


#define FILE_IVR_CONFIG_XML_STATIC                           "StaticCfg/IVR/IVRCfg/IVRServiceList.xml"
#define FILE_DEFAULT_IVR_CONFIG_XML_STATIC                   "StaticCfg/IVR/IVRCfg/IVRDefaultServiceList.xml"

#define IVR_FOLDER_MAIN                                      "Cfg/IVR"

//#define IVR_EXTERNAL_FOLDER_MAIN                             IVR_FOLDER_MAIN "/External"
#define IVR_EXTERNAL_FOLDER_MAIN                             "External"

#define IVR_FOLDER_SLIDES                                    "/Slides"
#define IVR_FOLDER_MUSIC                                     "/MusicSources"
#define IVR_FOLDER_MSG                                       "/msg"
#define IVR_FOLDER_TONES                                     "/GlobalTones"
#define IVR_FOLDER_CFG                                       "/IVRCfg"

#define FILE_IVR_CONFIG_XML                                  IVR_FOLDER_MAIN IVR_FOLDER_CFG "/IVRServiceList.xml"
#define FILE_IVR_CONFIG_XML_TEMP                             IVR_FOLDER_MAIN IVR_FOLDER_CFG "/IVRServiceList_Temp.xml"
#define FILE_IVR_CONFIG_XML_LAST_GOOD_COPY                   IVR_FOLDER_MAIN IVR_FOLDER_CFG "/IVRServiceList_LastGoodCopy.xml"
#define FILE_DEFAULT_IVR_CONFIG_XML                          IVR_FOLDER_MAIN IVR_FOLDER_CFG "/IVRDefaultServiceList.xml"

#define IVR_FOLDER_ROLLCALL                                  "IVRX/RollCall/"
#define IVR_FOLDER_ROLLCALL_EMB                              "RollCall/"

#define IVR_FOLDER_MUSIC_TMP_FILE                            "Cfg/IVR/MusicSources/MusicSource/MusicFile.wav"

#define IVR_FOLDER_MUSIC_TMP_FILE_RELATIVE_PATH				  "IVR/MusicSources/MusicSource/MusicFile.wav"

#define IVR_FOLDER_MUSIC_TMP_FILE_EMB                        "IVR/MusicSources/MusicSource/MusicFile.wav"

#define IVR_MSG_EXIT_TONE                                    "Cfg/IVR/GlobalTones/ExitTone.wav"
#define IVR_MSG_ENTRY_TONE                                   "Cfg/IVR/GlobalTones/EntryTone.wav"
#define IVR_MSG_ALERT_TONE                                   "Cfg/IVR/GlobalTones/AlertTone.wav"
#define IVR_MSG_ROLL_CALL_TONE                               "Cfg/IVR/GlobalTones/RollCallTone.wav"
#define IVR_MSG_MISSING_MSG_TONE                             "Cfg/IVR/GlobalTones/MissingIVRMsgTone.wav"
#define IVR_MSG_BLIP_ON_CASCADE_TONE                "Cfg/IVR/msg/English/general/gen/BlipOnCascade.wav"  // "Cfg/IVR/GlobalTones/BlipOnCascade.wav" //modified by Richer for BRIDGE-6209

#define FILE_MEETING_ROOM_DB                                 "Cfg/MeetingRooms/"
#define FILE_RECORDLINK_SRV_DB                               "Cfg/RecordLink/"
#define FILE_CONF_TEMPLATE_DB                                "Cfg/Templates/"
#define PROFILES_DB_DIR                                      "Cfg/Profiles/"
#define CUSTOMIZE_DISPLAY_SETTING_FOR_ONGGOING_CONF_CFG_FILE "Cfg/OngoingConfDisplaySetting.xml"

#define MS_CONVERSATION_ID_LEN                                512
#define FOCUS_URI_LEN                                		  512
#define RTP_HEADER_SIZE            							  12

typedef enum { eVideoResPriorAuto, eVideoResPriorSharp, eVideoResPriorMotion } eVideoResPriority;

typedef enum { eAvcToSvcIntraFirst=1, eAvcToSvcIntraSecond=2, eAvcToSvcIntraAll=3 } eAvcToSvcTranslatorIntra;

typedef enum
{
	eIvrStateIllegal = -1,
	eIvrStateSlideOff,
	eIvrStateSlideOn,
	eIvrStateLast
}
eIvrState;

#define IVR_PIPE_ID												210500

// SVC AVC Translate
#define WAIT_FOR_TRANSLATOR		1
#define NO_WAIT_FOR_TRANSLATOR  2
#define TRANSLATOR_ERROR        3

/*
// temp, to be moved to ApiCom:
#define CONF_PARTY_MRMP_ STREAM_IS_MUST_REQ                   1110017
typedef struct
{
                APIU32             unChannelHandle;  //incoming channel
                APIU32             unSyncSource;
                APIUBOOL           bIsMust;   //true means MCU won't send TSPR to this ssr
} MrmpStreamIsMustStruct;
*/
// end temp to be in ApiCom

typedef enum
{
	eMediaOnHoldNon			    =0, // 0b00
	eMediaOnHoldAudio 			=1, // 0b01
	eMediaOnHoldVideo			=2, // 0b10
	eMediaOnHoldAplusV			=3, // 0b11
	eMediaOnHoldButt,
}enMediaOnHold;

typedef enum{
	eToPrintNo = 0,
	eToPrintYes,
	eToPrintOnFalseOnly
}eToPrint;

// this codes will not be passed to CS
#define SipPseudoCodesNotAcceptedInHereLyncDialOut 700
#define SipPseudoCodesUnsuppMediaTypeDialOut 701

//eFeatureRssDialin
 #ifndef SRS_CONTROL_START_RESUME
#define   SRS_CONTROL_START_RESUME			"*2"
#endif

#ifndef  SRS_CONTROL_PAUSE
#define   SRS_CONTROL_PAUSE 					"*1"
#endif

#ifndef SRS_CONTROL_STOP
#define  	SRS_CONTROL_STOP					"*3" 
#endif

enum enSrsRecordingControlStatus
{
	eSrsRecordingControlFailed		=0,
	eSrsRecordingControlStarted		=1,
	eSrsRecordingControlStopped		=2,
	eSrsRecordingControlPaused		=3,
	eSrsRecordingControlInProgress	=4,
	eSrsRecordingControlTOUT		=5,
	eSrsRecordingControlButt
};

#define MAX_OPUS_AVERAGE_BITRATE	510*1024
#define MIN_OPUS_AVERAGE_BITRATE	8*1024



#endif // _CONFPARTYDEFINES_H__

