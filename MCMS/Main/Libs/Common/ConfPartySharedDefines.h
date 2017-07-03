#ifndef CONFPARTYSHAREDDEFINES_H_
#define CONFPARTYSHAREDDEFINES_H_

#include "ConfPartyApiDefines.h"

#define INVALID_CHANNEL_HANDLE (0xFFFFFFFF)
#define AV_SERVICE_NAME                                           81  // 20 characters + terminator
#define CONF_REMARKS_LEN                                          301 // 300 characters + terminator
#define IP_STRING_LEN                                             512
#define NUMERIC_CONFERENCE_ID_LEN                                 17 // 16 digits characters + terminator
#define CONFERENCE_ENTRY_PASSWORD_LEN                             17
#define USER_INFO_ITEM_LEN                                        CONF_INFO_ITEM_LEN /* 101 */
#define APPOITNMENT_ID_LEN                                        256 // 255 digits characters + terminator
#define PARTY_ADDRESS_LEN                                         256 // 255 digits characters + terminator

//#define INVALID_CHANNEL_HANDLE                                    0XFFFFFFFF


#define MAX_CASCADED_LINKS_NUMBER                                 4

#define PARTY_IDLE                                                0
#define PARTY_CONNECTED                                           1
#define PARTY_DISCONNECTED                                        2
#define PARTY_WAITING_FOR_DIAL_IN                                 3
#define PARTY_CONNECTING                                          4
#define PARTY_DISCONNECTING                                       5
#define PARTY_CONNECTED_PARTIALY                                  6
#define PARTY_DELETED_BY_OPERATOR                                 7
#define PARTY_SECONDARY                                           8
#define PARTY_STAND_BY                                            9
#define PARTY_CONNECTED_WITH_PROBLEM                              10
#define PARTY_REDIALING                                           11

// disconnection cause
#define NO_DISCONNECTION_CAUSE                                    0
#define PARTY_HANG_UP                                             1
#define DISCONNECTED_BY_OPERATOR                                  2
#define DISCONNECTED_BY_CHAIR                                     3
#define NO_ESTABL_H243_CONNECT                                    4
#define RESOURCES_DEFICIENCY                                      5
#define PASSWORD_FAILURE                                          6
#define BONDING_FAILURE                                           7
#define NO_NET_CONNECTION                                         8

/*Begin:added by Richer for BRIDGE-13006,2014.04.29*/
#define DISCONNECTED_BY_VIDEO_RECOVERY         900 // 900 for video recovery disconnecting the party
/*End:added by Richer for BRIDGE-13006,2014.04.29*/

// #define   NET_PORT_DEFICIENCY                                       9 // dont use 9 because PARTY_STAND_BY used in CConf::OnPartyEndAddConnect in the same field
#define NET_PORT_DEFICIENCY                                       10
#define MUX_PORT_DEFICIENCY                                       11
#define CONF_END_TIME                                             14
#define AUDIO_MSG_PORT_DEFICIENCY                                 15
#define V_GATE_NOT_RESPOND                                        16
#define H323_CALL_CLOSED_NO_PORT_LEFT_FOR_AUDIO                   20
#define H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEO                   21
#define H323_CALL_CLOSED_NO_PORT_LEFT_FOR_FECC                    22
#define H323_CALL_CLOSED_NO_CONTROL_PORT_LEFT                     23
#define H323_CALL_CLOSED_NO_PORT_LEFT_FOR_VIDEOCONT               25
#define MCU_INTERNAL_PROBLEM                                      26

// Slave parties failure
#define SLAVE_PARTY_RIGHT_GENERAL_FAILURE                         30
#define SLAVE_PARTY_LEFT_GENERAL_FAILURE                          31
#define SLAVE_PARTY_AUX_GENERAL_FAILURE                           32
#define SLAVE_PARTY_RIGHT_ALLOCATION_FAILURE                      33
#define SLAVE_PARTY_LEFT_ALLOCATION_FAILURE                       34
#define SLAVE_PARTY_AUX_ALLOCATION_FAILURE                        35

//Multiple links for ITP in cascaded conference feature: disconnect all the room
#define SUB_OR_MAIN_LINK_IS_SECONDARY                             40

// from 50 ... 70 is SECURITY_FAILURE family commands
#define REMOTE_DEVICE_CAPABILITIES_DO_NOT_SUPPORT_ENCRYPTION                                                 50
#define A_COMMON_KEY_EXCHANGE_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE       51
#define A_COMMON_ENCRYPTION_ALGORITHM_COULD_NOT_BE_ESTABLISHED_BETWEEN_THE_MCU_AND_THE_REMOTE_DEVICE         52
#define REMOTE_DEVICE_DID_NOT_OPEN_THE_ENCRYPTION_SIGNALING_CHANNEL                                          53
#define THE_REMOTE_DEVICE_KEY_EXCHANGE_ALGORITHM_MESSAGE_WAS_NOT_RECEIVED_BY_THE_MCU                         54
#define THE_REMOTE_DEVICE_ENCRYPTION_ALGORITHM_MESSAGE_WAS_NOT_RECEIVED_BY_THE_MCU                           55
#define THE_ENCRYPTION_SETUP_PROCESS_DID_NOT_END_ON_TIME                                                     56
#define REMOTE_DEVICE_CANNOT_ENCRYPT                                                                         57
#define THE_REMOTE_DEVICE_FAILED_TO_START_ENCRYPTION_SYSTEM                                                  58
#define REMOTE_DEVICES_SELECTED_ENCRYPTION_ALGORITHM_DOES_NOT_MATCH_THE_LOCAL_SELECTED_ENCRYPTION_ALGORITHM  59
#define ENCRYPTION_KEY_EXCHANGE_FAILED                                                                       60

#define CALLER_NOT_REGISTERED                                     145

#define H323_CALL_CLOSED_ARQTIMEOUT                               152
#define H323_CALL_CLOSED_DRQTIMEOUT                               153
#define H323_CALL_CLOSED_ALT_GK_FAILURE                           154

// from 191 ... 220 is H323 FAILURE family commands
#define H323_CALL_CLOSED_REMOTE_BUSY                              191
#define H323_CALL_CLOSED_NORMAL                                   192
#define H323_CALL_CLOSED_REMOTE_REJECT                            193
#define H323_CALL_CLOSED_REMOTE_UNREACHABLE                       194
#define H323_CALL_CLOSED_UNKNOWN_REASON                           195
#define H323_CALL_CLOSED_BY_MCU                                   196
#define H323_CALL_CLOSED_FAULTY_DESTINATION_ADDRESS               197
#define H323_CALL_CLOSED_SMALL_BANDWIDTH                          198
#define H323_CALL_CLOSED_GATEKEEPER_FAILURE                       199
#define H323_CALL_CLOSED_GATEKEEPER_REJECT_ARQ                    200
#define H323_CALL_CLOSED_NO_PORT_LEFT                             201
#define H323_CALL_CLOSED_GATEKEEPER_DRQ                           202
#define H323_CALL_CLOSED_NO_DESTINATION_IP_ADDRESS                203
#define H323_CALL_CLOSED_REMOTE_HAS_NOT_SENT_CAPABILITY           204
#define H323_CALL_CLOSED_AUDIO_CHANNELS_NOT_OPEN                  205
#define H323_CALL_CLOSED_BAD_REMOTE_CAP                           207
#define H323_CALL_CLOSED_CAPS_NOT_ACCPTED_BY_REMOTE               208
#define H323_FAILURE                                              209
#define H323_CALL_CLOSED_REMOTE_STOP_RESPONDING                   210
#define H323_CALL_CLOSED_MASTER_SLAVE_PROBLEM                     213
#define H323_CALL_CLOSED_PROBLEM_WITH_CONTENT_CONNECTION_TO_MCU   214
#define IP_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR               215
#define H323_CALL_CLOSED_PROBLEM_WITH_ACTIVE_CONTENT_SLAVE        216
#define H323_CALL_CLOSED_MGC_CASCADED_IN_HD_VSW_LPR_ENABLED_CONF  217
#define H323_CALL_CLOSED_REMOTE_NO_ANSWER                         218
#define H323_CALL_CLOSED_MEDIA_DISCONNECTED                       219

// Sip close reasons - 251-299
// 300-700 are saved numbers in the sip protocol.
#define SIP_TIMER_POPPED_OUT                                      251
#define SIP_CARD_REJECTED_CHANNELS                                252
#define SIP_CAPS_DONT_MATCH                                       253
#define SIP_REMOTE_CLOSED_CALL                                    254
#define SIP_REMOTE_CANCEL_CALL                                    255
#define SIP_BAD_STATUS                                            256
#define SIP_REMOTE_STOP_RESPONDING                                257
#define SIP_REMOTE_UNREACHABLE                                    258
#define SIP_TRANSPORT_ERROR                                       259
#define SIP_BAD_NAME                                              260
#define SIP_TRANS_ERROR_TCP_INVITE                                261
#define SIP_INTERNAL_MCU_PROBLEM                                  262
#define SIP_NO_ADDR_FOR_MEDIA                                     263
#define SIP_INSUFFICIENT_BANDWIDTH                                264
#define SIP_REMOTE_NO_ANSWER                                      265
#define SIP_TOUT_DURING_UPGRADE_TO_MIXED                          266


#define TIP_VIDEO_BIT_RATE_TOO_LOW                                270
#define TIP_CREATE_TIMER_POPPED_OUT                               271
#define TIP_NEGOTIATION_FAILURE                                   272

#define WEBRTC_CONNECT_FAILURE	                                  280
#define WEBRTC_CONNECT_TOUT		                                  281

// Don't pass the number 299!
#define SIP_REDIRECTION_300                                       300
#define SIP_MOVED_PERMANENTLY                                     301
#define SIP_MOVED_TEMPORARILY                                     302
#define SIP_REDIRECTION_303                                       303
#define SIP_REDIRECTION_305                                       305
#define SIP_REDIRECTION_380                                       380
#define SIP_CLIENT_ERROR_400                                      400
#define SIP_UNAUTHORIZED                                          401
#define SIP_CLIENT_ERROR_402                                      402
#define SIP_FORBIDDEN                                             403
#define SIP_NOT_FOUND                                             404
#define SIP_CLIENT_ERROR_405                                      405
#define SIP_CLIENT_ERROR_406                                      406
#define SIP_CLIENT_ERROR_407                                      407
#define SIP_REQUEST_TIMEOUT                                       408
#define SIP_CLIENT_ERROR_409                                      409
#define SIP_GONE                                                  410
#define SIP_CLIENT_ERROR_411                                      411
#define SIP_CLIENT_ERROR_413                                      413
#define SIP_CLIENT_ERROR_414                                      414
#define SIP_UNSUPPORTED_MEDIA_TYPE                                415
#define SIP_UNSUPPORTED_URI_SCHEME                                416
#define SIP_CLIENT_ERROR_420                                      420
#define SIP_EXTENSION_REQUIRED                                    421
#define SIP_TEMPORARILY_NOT_AVAILABLE                             480
#define SIP_CLIENT_ERROR_481                                      481
#define SIP_CLIENT_ERROR_482                                      482
#define SIP_CLIENT_ERROR_483                                      483
#define SIP_CLIENT_ERROR_484                                      484
#define SIP_CLIENT_ERROR_485                                      485
#define SIP_BUSY_HERE                                             486
#define SIP_REQUEST_TERMINATED                                    487
#define SIP_CLIENT_ERROR_488                                      488
#define SIP_CLIENT_ERROR_491                                      491
#define SIP_SERVER_ERROR_500                                      500
#define SIP_SERVER_ERROR_501                                      501
#define SIP_SERVER_ERROR_502                                      502
#define SIP_SERVER_ERROR_503                                      503
#define SIP_SERVER_ERROR_504                                      504
#define SIP_SERVER_ERROR_505                                      505
#define SIP_BUSY_EVERYWHERE                                       600
#define SIP_GLOBAL_FAILURE_603                                    603
#define SIP_GLOBAL_FAILURE_604                                    604
#define SIP_GLOBAL_FAILURE_606                                    606

// from 750 ... 800 is H323 FAILURE family commands
#define ISDN_CALL_CLOSE_H239_CONTENT_PROCESSING_ERROR             750


#define OPERATOR_ADD_PARTY                                        101
#define OPERATOR_UPDATE_PARTY                                     105
#define OPERATOR_TERMINATE                                        100
#define OPERATORS_DELETE_PARTY                                    102
#define OPERATOR_DISCONNECTE_PARTY                                103
#define OPERATOR_RECONNECT_PARTY                                  104
#define OPERATOR_SET_END_TIME                                     106

/*---------------------------------------------------------------------
  this enum is used in a conf status and conf termination cause.
  To Add/Remove status : update enum AND array.
---------------------------------------------------------------------*/
enum eConfCdrStatus
{
  DEFAULT_STATUS = 0,
  ONGOING_CONFERENCE,
  TERMINATE_BY_OPERATOR,
  TERMINATED_END_TIME_PASSED,
  AUTO_TERMINATION,
  CONFERENCE_NEVER_STARTED,
  NOT_ONGOING_CONFERENCE_PROBLEM,
  COMPLETED_BY_MCU_RESET,
  UNKNOWN_ERROR,
  TERMINATE_BY_USER,
  INSUFFICIENT_RESOURCE,
  RESOURCE_DEFICIENCY,
  NumOfConferenceStatuses
};

static const char* ConfCdrStatusName[] =
{
  "Invalid",
  "ongoing",
  "cause_operator_terminate",
  "end_time_terminate",
  "auto_termination",
  "never_started",
  "conf_problem",
  "mcu_completed",
  "unknown",
  "terminated_by_participant",
  "Insufficient Resources",
  "Resource deficiency"
};

static const char* GetConfCdrStatusName(eConfCdrStatus status)
{
  const char* name = (DEFAULT_STATUS <= status && status < NumOfConferenceStatuses ? ConfCdrStatusName[status] : "Invalid"); return name;
}

/*---------------------------------------------------------------------
  this enum is used in a GW sessions for party visual indications
  To Add/Remove status : update enum AND array.
---------------------------------------------------------------------*/
enum eGeneralDisconnectionCause
{
  eRemoteBusy = 0,
  eRemoteNoAnswer,
  eRemoteWrongNumber,
  eRemoteFailed,
  eGeneralDisconnectionCauseLast
};

static const char* GeneralDisconnectionCause[] =
{
  "Busy",
  "Not Answered",
  "Wrong Number",
  "Failed"
};

static const char* GetGeneralDisconnectionCause(eGeneralDisconnectionCause discCause)
{
  const char* name = (eRemoteBusy <= discCause && discCause < eGeneralDisconnectionCauseLast ? GeneralDisconnectionCause[discCause] : ""); return name;
}

enum eGatewayPartyType
{
  eInvalidPartyType      = -1,
  eRegularPartyNoGateway = 0,
  eNormalGWPartyType,
  eInitiatorNotInviter,   // the dial in party that created the session in case of dial through
  eInviter                // the dial in party that created the session in case of video invite
};

#define PHONE_NUMBER_DIGITS_LEN                                   31  // 40 characters + terminator
#define SERVICE_PHONE_PREFIX_LEN                                  25

// secondary cause
#define SECONDARY_CAUSE_DEFAULT                                   0
#define SECONDARY_CAUSE_REMOTE_CAPABILITIES                       11
#define SECONDARY_CAUSE_CONFERENCE_REJECT                         13
#define SECONDARY_CAUSE_CHANGE_MODE                               14
#define SECONDARY_CAUSE_STREAM_VIOLATION                          15
#define SECONDARY_CAUSE_VIDEO_PROBLEM                             16
#define SECONDARY_CAUSE_MOVE_PARTY                                17
#define SECONDARY_CAUSE_NO_VIDEO_CONNECTION                       18
#define SECONDARY_CAUSE_RMT_CLOSE_CHAN                            24
#define SECONDARY_CAUSE_RMT_DIFF_CAPCODE                          25
#define SECONDARY_CAUSE_RMT_NOT_OPEN_AFTER_CHANGE_MODE            26
#define SECONDARY_CAUSE_GK_RETURNED_SMALL_BANDWIDTH               27
#define SECONDARY_CAUSE_CONFERENCING_LIMITATION                   28
#define SECONDARY_CAUSE_AVF_INSUFFICIENT_BANDWIDTH                29
#define SECONDARY_CAUSE_H239_FIRST_OPCODE                         30
#define SECONDARY_CAUSE_H239_BW_MISMATCH                          30
#define SECONDARY_CAUSE_H239_INCOMPATIBLE_CAPS                    31
#define SECONDARY_CAUSE_H239_RMT_DIFF_CAPCODE                     32
#define SECONDARY_CAUSE_H239_CONFERENCE_REJECT                    33
#define SECONDARY_CAUSE_BELOW_CONTENT_RATE_THRESHOLD              34
#define SECONDARY_CAUSE_BELOW_CONTENT_RESOLUTION_THRESHOLD        35
#define SECONDARY_CAUSE_H239_LAST_OPCODE                          36
#define SECONDARY_CAUSE_MSSLAVEIN_NO_AUDIO 	                      40
#define SECONDARY_CAUSE_MSSLAVEOUT_NO_AUDIO 	                  41

#define SECONDARY_CAUSE_LEN                                       250

// IVR feature opcodes
#define IVR_FEATURE_LANG_MENU                                     1
#define IVR_FEATURE_CONF_PASSWORD                                 2
#define IVR_FEATURE_PIN_CODE                                      3
#define IVR_FEATURE_OPER_ASSISTANCE                               4
#define IVR_FEATURE_WELCOME                                       5
#define IVR_FEATURE_CONF_LEADER                                   6
#define IVR_FEATURE_GENERAL                                       7
#define IVR_FEATURE_BILLING_CODE                                  8
#define IVR_FEATURE_INVITE_PARTY                                  9
#define IVR_FEATURE_ROLL_CALL                                     10
#define IVR_FEATURE_VIDEO                                         11
#define IVR_FEATURE_NUMERIC_CONFERENCE_ID                         12
#define IVR_FEATURE_MUTE_NOISY_LINE                               13
#define IVR_FEATURE_RECORDING                                     14
#define IVR_FEATURE_PLAYBACK                                      15
#define IVR_FEATURE_AUDIO_EXTERNAL                                16
#define IVR_FEATURE_VIDEO_EXTERNAL                                17
#define IVR_FEATURE_COLLECT_EXTERNAL                              18

// Range of recording control values
enum  eRecordingControl
{
  eStopRecording    = 0,
  eStartRecording   = 1,
  ePauseRecording   = 2,
  eResumeRecording  = 3,
};

#define MAX_RESERVATION_RATES                                     29

#define NUMBER_OF_COP_LEVELS                                      4
#define INVALID_COP_LEVEL                                         0xFF

enum  eMaxRtvResolution
{
	eMaxRtvResolutionAuto= 0,
	eMaxRtvResolutionHD720,
	eMaxRtvResolutionVGA,
	eMaxRtvResolutionCIF,
	eMaxRtvResolutionQCIF
};

enum  eMaxMsSvcResolution
{
	eMaxMsSvcResolutionAuto= 0,
	eMaxMsSvcResolutionHD1080,
	eMaxMsSvcResolutionHD720,
	eMaxMsSvcResolutionVGA,
	eMaxMsSvcResolutionCIF,
};

enum  eMsClientAudioCodec
{
	eMsClientAudioCodecAuto= 0,
	eMsClientAudioCodecG711A,
	eMsClientAudioCodecG711U,
	eMsClientAudioCodecG722,
	eMsClientAudioCodecG7231,
	eMsClientAudioCodecG7221_24
};

enum  eEncodeRtvBFrame
{
	eEncodeRtvBFrameNever= 0,
	eEncodeRtvBFrameAvmcuOnly,
	eEncodeRtvBFrameAlways
};

enum  eMsFEC
{
	eMsFECAuto= 0, //prefer DV01 then DV00, then NO.
	eMsFECDV00,
	eMsFECDV01,
	eMsFECNo     //Disable FEC
};

enum eAnatIpProtocol
{
	eAnatDisabled = 0,
	eAnatAuto,
	eAnatPreferIpV4,
	eAnatPreferIpV6
};

#define ConfRsrcID     DWORD
#define PartyRsrcID    DWORD
#define ConfMonitorID  DWORD
#define PartyMonitorID DWORD
#define ConnectionID   DWORD
#define RoomID         WORD
#define MenuID         WORD
#define SsrcID         DWORD


#endif /*CONFPARTYSHAREDDEFINES_H_*/

