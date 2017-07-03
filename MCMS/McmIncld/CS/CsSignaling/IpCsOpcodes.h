//+                                                                        +
//                       IpCsOpcodes.h                                     |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IpCsOpcodes.h                                               |
// SUBSYSTEM:  CS/ConfParty                                                |
// PROGRAMMER: Guy D,													   |
// Date : 15/5/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+
/*
 
This header file contains all opcodes between (H323/SIP) CS and ConfParty processes (Signaling)

  13000001	- 13666666: requests from ConfParty to CS
  13666667	- 14000000: indication from CS to ConfParty

*/

#ifndef  __IPCSOPCODES_H__
#define  __IPCSOPCODES_H__

#include "DataTypes.h"

////////////////////////////////////////////////
///////////  H323/SIP <-----> CS  //////////////
////////////////////////////////////////////////

#define PARTY_CS_FIRST_OPCODE_IN_RANGE						13000001

/// Req
#define PARTY_TO_CS_FIRST_OPCODE_IN_RANGE					13000002

// H323
#define H323_CS_SIG_FIRST_REQ								13000003

#define H323_CS_SIG_GET_PORT_REQ							13000003 // H323_GET_PORT_REQ
#define H323_CS_SIG_RELEASE_PORT_REQ						13000004 // H323_RELEASE_PORT_REQ
#define H323_CS_SIG_CALL_SETUP_REQ							13000005 // H323_CALL_SETUP_REQ
#define H323_CS_SIG_CALL_ANSWER_REQ							13000006 // H323_CALL_ANSWER_REQ
#define H323_CS_SIG_RE_CAPABILITIES_REQ						13000007 // H323_RE_CAPABILITIES_REQ
#define H323_CS_SIG_CREATE_CNTL_REQ							13000008 // H323_CREATE_CNTL_REQ
#define H323_CS_SIG_INCOMING_CHNL_RESPONSE_REQ				13000009 // H323_INCOMING_CHNL_RESPONSE_REQ
#define H323_CS_SIG_OUTGOING_CHNL_REQ						13000010 // H323_OUTGOING_CHNL_REQ
#define H323_CS_SIG_FLOW_CNTL_REQ							13000011 // H323_CHAN_NEW_RATE_REQ
#define H323_CS_SIG_CHAN_MAX_SKEW_REQ						13000012 // H323_CHAN_MAX_SKEW_REQ
#define H323_CS_SIG_VIDEO_UPDATE_PIC_REQ			    	13000013 // IP_VIDEO_UPDATE_PIC_REQ
#define H323_CS_SIG_CHANNEL_OFF_REQ							13000014 // H323_CHANNEL_OFF_REQ
#define H323_CS_SIG_MULTIPOINTMODECOM_TERMINALID_REQ        13000015 // H323_MULTIPOINTMODECOM_TERMINALID_REQ
#define H323_CS_SIG_MEDIA_PRODUCER_STATUS_REQ               13000016 // H323_MEDIA_PRODUCER_STATUS_REQ
#define H323_CS_SIG_CHANNEL_ON_REQ							13000017 // H323_CHANNEL_ON_REQ
#define H323_CS_SIG_ROLE_TOKEN_REQ                          13000018 // H323_ROLE_TOKEN_REQ
#define H323_CS_SIG_UNEXPECTED_MESSAGE_REQ					13000019 // H323_UNEXPECTED_MESSAGE_REQ
#define H323_CS_SIG_CALL_DROP_TIMER_EXPIRED_REQ				13000020 // H323_CALL_DROP_TIMER_EXPIRED_REQ
#define	H323_CS_SIG_CONFERENCE_RES_REQ						13000021 // H323_CONFERENCE_RES_REQ
#define	H323_CS_SIG_CONFERENCE_IND_REQ						13000022 // H323_CONFERENCE_IND_REQ
#define H323_CS_SIG_VIDEO_NOT_DECODED_MBS_REQ				13000023 // H323_VIDEO_NOT_DECODED_MBS_REQ
#define H323_CS_SIG_CHNL_DROP_REQ							13000024 // H323_CHNL_DROP_REQ
#define H323_CS_SIG_CALL_DROP_REQ							13000025 // H323_CALL_DROP_REQ
#define H323_CS_SIG_CALL_CLOSE_CONFIRM_REQ					13000026 // H323_CALL_CLOSE_CONFIRM_REQ
#define H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ					13000027 // H323_CS_SIG_DBC2_COMMAND_CT_ON_REQ
#define H323_CS_SIG_DBC2_COMMAND_CT_OFF_REQ					13000028 // IP_DBC2_COMMAND_CT_OFF_REQ
#define H323_CS_SIG_CHNL_NEW_RATE_REQ						13000029 // H323_CS_SIG_CHNL_NEW_RATE_REQ
#define H323_CS_NON_STANDARD_REQ_REQ						13000030 // H323_CS_NON_STANDARD_REQ_REQ
#define H323_CS_NON_STANDARD_RES_REQ						13000031 // H323_CS_NON_STANDARD_RES_REQ
#define H323_CS_NON_STANDARD_IND_REQ						13000032 // H323_CS_NON_STANDARD_IND_REQ
#define H323_CS_NON_STANDARD_COM_REQ						13000033 // H323_CS_NON_STANDARD_COM_REQ
#define H323_CS_SIG_ROUND_TRIP_DELAY_REQ					13000034 // H323_CS_SIG_ROUND_TRIP_DELAY_REQ
#define H323_CS_SIG_CONFERENCE_REQ_REQ						13000035 // H323_CS_SIG_CONFERENCE_REQ_REQ
#define H323_CS_SIG_CONFERENCE_COM_REQ						13000036 // H323_CS_SIG_CONFERENCE_COM_REQ
#define H323_CS_SIG_H323_CT_AUTHENTICATION_REQ				13000037 // H323_CT_AUTHENTICATION_REQ
#define H323_CS_DTMF_INPUT_REQ								13000038 // H323_CS_DTMF_INPUT_REQ
#define IP_CS_SIG_STOP_ALL_PROCESSES_REQ					13000039 // IP_STOP_ALL_PROCESSOR_REQ
#define H323_CS_PARTY_KEEP_ALIVE_REQ						13000040 // H323_CS_PARTY_KEEP_ALIVE_REQ
#define H323_CS_SIG_CAPABILITIES_RES_REQ					13000041 // H323_CS_SIG_CAPABILITIES_RES_REQ
#define H323_CS_FACILITY_REQ								13000042 // H323_CS_FACILITY_REQ
#define H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ					13000043 // H323_CS_SIG_LPR_MODE_CHANGE_RES_REQ
#define H323_CS_SIG_LPR_MODE_CHANGE_REQ						13000044 // H323_CS_SIG_LPR_MODE_CHANGE_REQ
#define H323_CS_SIG_RSS_CMD_REQ								13000045 // H323_CS_SIG_RSS_CMD_REQ
//added by Jason for ITP-Multiple channels begin
#define H323_CS_SIG_NEW_ITP_SPEAKER_REQ						13000046//H323_CS_SIG_NEW_ITP_SPEAKER_REQ	
#define H323_CS_SIG_NEW_ITP_SPEAKER_ACK_REQ					13000047//H323_CS_SIG_NEW_ITP_SPEAKER_ACK_REQ
//added by Jason for ITP-Multiple channels end

#define H323_CS_SIG_LAST_REQ								13000048 // H323_CS_SIG_LAST_REQ

// SIP
#define SIP_CS_SIG_FIRST_REQ								13340000
#define SIP_CS_SIG_INVITE_REQ								13340000
#define SIP_CS_SIG_REINVITE_REQ								13340001
#define SIP_CS_SIG_INVITE_ACK_REQ							13340002
#define SIP_CS_SIG_INVITE_RESPONSE_REQ						13340003
#define SIP_CS_SIG_RINGING_REQ								13340004
#define SIP_CS_SIG_CANCEL_REQ								13340005
#define SIP_CS_SIG_BYE_REQ									13340006
#define SIP_CS_SIG_BYE_200_OK_REQ							13340007
#define SIP_CS_SIG_VIDEO_FAST_UPDATE_REQ					13340008
#define SIP_CS_SIG_DEL_NEW_CALL_REQ							13340009
#define SIP_CS_SIG_INSTANT_MESSAGE_REQ						13340010
#define SIP_CS_SIG_REDIRECT_DATA_REQ						13340011
#define SIP_CS_SIG_OPTIONS_RESP_REQ							13340012
#define SIP_CS_SIG_REFER_RESP_REQ							13340013
#define SIP_CS_PARTY_KEEP_ALIVE_REQ						  	13340014
#define SIP_CS_SIG_SUBSCRIBE_RESP_REQ						13340015
#define SIP_CS_SIG_INFO_REQ									13340016
#define SIP_CS_SIG_INFO_RESP_REQ							13340017
#define SIP_CS_BFCP_MESSAGE_REQ								13340018
#define SIP_CS_SIG_DIALOG_RECOVERY_REQ						13340019
#define SIP_CS_SIG_SEND_CRLF_REQ                            13340020
#define SIP_CS_SIG_SOCKET_ACTIVITY_REQ                      13340021   
#define SIP_CS_CCCP_SIG_INVITE_REQ 							13340022   // add user
#define SIP_CS_SIG_LAST_REQ									13340023


#define PARTY_TO_CS_LAST_OPCODE_IN_RANGE					13666666
/// Ind
#define CS_TO_PARTY_FIRST_OPCODE_IN_RANGE					13666667
#define H323_CS_SIG_FIRST_IND								13666668

#define H323_CS_SIG_GET_PORT_IND							13666668 // H323_GET_PORT_IND
#define H323_CS_SIG_CALL_DIAL_TONE_IND						13666669 // H323_CALL_DIAL_TONE_IND
#define H323_CS_SIG_CALL_PROCEEDING_IND					    13666670 // H323_CALL_PROCEEDING_IND
#define H323_CS_SIG_CALL_RING_BACK_IND						13666671 // H323_CALL_RING_BACK_IND
#define H323_CS_SIG_CALL_CONNECTED_IND						13666672 // H323_CALL_CONNECTED_IND
#define H323_CS_SIG_CALL_OFFERING_IND						13666673 // H323_CALL_OFFERING_IND
#define H323_CS_SIG_CAPABILITIES_IND						13666674 // H323_CAPABILITIES_IND
#define H323_CS_SIG_CALL_NEW_RATE_IND						13666675 // H323_CALL_NEW_RATE_IND
#define H323_CS_SIG_CAP_RESPONSE_IND						13666676 // H323_CAP_RESPONSE_IND
#define H323_CS_SIG_CALL_CNTL_CONNECTED_IND				    13666677 // H323_CALL_CNTL_CONNECTED_IND
#define H323_CS_SIG_INCOMING_CHANNEL_IND					13666678 // H323_INCOMING_CHANNEL_IND
#define H323_CS_SIG_INCOMING_CHNL_CONNECTED_IND			    13666679 // H323_INCOMING_CHNL_CONNECTED_IND
#define H323_CS_SIG_OUTGOING_CHNL_RESPONSE_IND				13666680 // H323_OUTGOING_CHNL_RESPONSE_IND
#define H323_CS_SIG_CHAN_NEW_RATE_IND						13666681 // H323_CHAN_NEW_RATE_IND
#define H323_CS_SIG_CALL_IDLE_IND							13666682 // H323_CALL_IDLE_IND
#define H323_CS_SIG_START_CHANNEL_CLOSE_IND					13666683 // H323_START_CHANNEL_CLOSE_IND
#define H323_CS_SIG_CHANNEL_CLOSE_IND						13666684 // H323_CHANNEL_CLOSE_IND
#define H323_CS_SIG_FLOW_CONTROL_IND_IND					13666685 // H323_FLOW_CONTROL_IND_IND
#define H323_CS_SIG_DBC2_COMMAND_CT_ON_IND					13666686 // IP_DBC2_COMMAND_CT_ON_IND
#define H323_CS_SIG_DBC2_COMMAND_CT_OFF_IND					13666687 // IP_DBC2_COMMAND_CT_OFF_IND
#define H323_CS_SIG_MULTIPLE_VIDEO_UPDATE_PIC_IND			13666688 // IP_MULTIPLE_VIDEO_UPDATE_PIC_IND
#define H323_CS_SIG_CHAN_MAX_SKEW_IND						13666689 // H323_CHAN_MAX_SKEW_IND
#define IP_CS_VIDEO_UPDATE_PIC_IND							13666690 // IP_VIDEO_UPDATE_PIC_IND
#define H323_CS_CHANNEL_OFF_IND								13666691 // H323_CHANNEL_OFF_IND
#define H323_CS_CHANNEL_ON_IND								13666692 // H323_CHANNEL_ON_IND
#define H323_CS_CONFERENCE_REQ_IND							13666693 // H323_CONFERENCE_REQ_IND
#define H323_CS_CONFERENCE_COM_IND							13666694 // H323_CONFERENCE_COM_IND
#define H323_CS_CONFERENCE_RES_IND							13666695 // H323_CONFERENCE_RES_IND
#define H323_CS_CONFERENCE_IND_IND							13666696 // H323_CONFERENCE_IND_IND
#define H323_CS_SIG_CALL_ROLE_TOKEN_IND						13666697 // H323_ROLE_TOKEN_IND
#define H323_CS_NON_STANDARD_REQ_IND						13666698 // H323_NON_STANDARD_REQ_IND
#define H323_CS_NON_STANDARD_COM_IND						13666699 // H323_NON_STANDARD_COM_IND
#define H323_CS_NON_STANDARD_RES_IND						13666700 // H323_NON_STANDARD_RES_IND
#define H323_CS_NON_STANDARD_IND_IND						13666701 // H323_NON_STANDARD_IND_IND
#define H323_CS_DTMF_INPUT_IND								13666702 // H323_CS_DTMF_INPUT_IND
#define H323_CS_SIG_CALL_AUTHENTICATION_IND					13666703 // H323_CS_SIG_CALL_AUTHENTICATION_IND
#define H323_CS_SIG_CALL_BAD_SPONTAN_IND					13666704 // H323_CS_SIG_CALL_BAD_SPONTAN_IND
#define H323_CS_PARTY_KEEP_ALIVE_IND						13666705 // H323_CS_PARTY_KEEP_ALIVE_IND
#define H323_CS_FACILITY_IND								13666706 // H323_CS_FACILITY_IND
#define H323_CS_SIG_LPR_MODE_CHANGE_IND						13666707 // H323_CS_SIG_LPR_MODE_CHANGE_IND
#define H323_CS_SIG_LPR_MODE_CHANGE_RES_IND					13666708 // H323_CS_SIG_LPR_MODE_CHANGE_RES_IND
//added by Jason for ITP-Multiple channels begin
#define H323_CS_SIG_NEW_ITP_SPEAKER_IND						13666709//H323_CS_SIG_NEW_ITP_SPEAKER_REQ	
#define H323_CS_SIG_NEW_ITP_SPEAKER_ACK_IND					13666710//H323_CS_SIG_NEW_ITP_SPEAKER_ACK_REQ
//added by Jason for ITP-Multiple channels end

#define H323_CS_SIG_LAST_IND								13666750

// SIP
#define SIP_CS_SIG_FIRST_IND								13835000
#define SIP_CS_SIG_INVITE_IND								13835000
#define SIP_CS_SIG_REINVITE_IND								13835001
#define SIP_CS_SIG_INVITE_ACK_IND							13835002
#define SIP_CS_SIG_INVITE_RESPONSE_IND						13835003
#define SIP_CS_SIG_PROV_RESPONSE_IND						13835004
#define SIP_CS_SIG_BYE_IND									13835005
#define SIP_CS_SIG_BYE_200_OK_IND							13835006
#define SIP_CS_SIG_CANCEL_IND								13835007
#define SIP_CS_SIG_VIDEO_FAST_UPDATE_IND					13835008
#define SIP_CS_SIG_DTMF_DIGIT_IND							13835009
#define SIP_CS_SIG_INSTANT_MESSAGE_IND						13835010
#define SIP_CS_SIG_OPTIONS_IND								13835011
#define SIP_CS_SIG_REFER_IND								13835012
#define SIP_CS_SIG_SESSION_TIMER_EXPIRED_IND				13835013
#define SIP_CS_SIG_SESSION_TIMER_REINVITE_IND				13835014
#define SIP_CS_PARTY_KEEP_ALIVE_IND						  	13835015
#define SIP_CS_SIG_TRACE_INFO_IND							13835016
#define SIP_CS_SIG_BAD_STATUS_IND							13835017
#define SIP_CS_SIG_TRANSPORT_ERROR_IND						13835018
#define SIP_CS_SIG_CARD_DATA_IND							13835019
#define SIP_CS_SIG_SUBSCRIBE_IND							13835020
#define SIP_CS_SIG_INFO_IND									13835021
#define SIP_CS_SIG_INFO_RESP_IND							13835022
#define SIP_CS_BFCP_MESSAGE_IND								13835023
#define SIP_CS_BFCP_TRANSPORT_IND							13835024
#define SIP_CS_SIG_DIALOG_RECOVERY_IND							13835025
#define SIP_CS_SIG_SOCKET_ACTIVITY_IND                      				13835026
#define SIP_CS_SIG_CRLF_ERR_IND                             				13835027
#define SIP_CS_SIG_NOTIFY_RESPONSE_IND							13835028
#define SIP_CS_SIG_BENOTIFY_IND								13835029
#define SIP_CS_CCCP_SIG_INVITE_IND							13835030	// msconf invite
#define SIP_CS_CCCP_SIG_ADD_USER_INVITE_RESPONSE_IND					13835031	// 200 OK for Add user
#define SIP_CS_SIG_LAST_IND								13835032


#define CS_TO_PARTY_LAST_OPCODE_IN_RANGE					13999999
#define PARTY_CS_LAST_OPCODE_IN_RANGE					    14000000


////////////////////////////////////////////////
///////////  PROXY Mngr <-----> CS  ////////////
////////////////////////////////////////////////

#define PROXY_CS_FIRST_OPCODE_IN_RANGE						14500001

//Req
#define PROXY_TO_CS_FIRST_OPCODE_IN_RANGE					14500002

#define SIP_CS_PROXY_FIRST_REQ								14500003
#define SIP_CS_PROXY_REGISTER_REQ							14500003
#define SIP_CS_PROXY_SUBSCRIBE_REQ							14500004
#define SIP_CS_PROXY_UNKNOWN_METHOD_REQ						14500005
#define SIP_CS_PROXY_NOTIFY_REQ								14500006
#define SIP_CS_PROXY_SERVICE_REQ							14500007
#define SIP_CS_PROXY_SEND_CRLF_REQ                          14500008
#define SIP_CS_PROXY_LAST_REQ								14500009

#define PROXY_TO_CS_LAST_OPCODE_IN_RANGE					14500100

//Ind
#define CS_TO_PROXY_FIRST_OPCODE_IN_RANGE					14500101

#define SIP_CS_PROXY_FIRST_IND								14500102
#define SIP_CS_PROXY_REGISTER_RESPONSE_IND					14500102
#define SIP_CS_PROXY_SUBSCRIBE_RESPONSE_IND					14500103
#define SIP_CS_PROXY_NOTIFY_IND								14500104
#define SIP_CS_PROXY_TRACE_INFO_IND							14500105
#define SIP_CS_PROXY_BAD_STATUS_IND							14500106
#define SIP_CS_PROXY_TRANSPORT_ERROR_IND					14500107
#define SIP_CS_PROXY_CARD_DATA_IND							14500108
#define SIP_CS_PROXY_SERVICE_RESPONSE_IND					14500109
#define SIP_CS_PROXY_CRLF_ERR_IND       					14500110
#define SIP_CS_PROXY_LAST_IND								14500111

#define CS_TO_PROXY_LAST_OPCODE_IN_RANGE					14500199
#define PROXY_CS_LAST_OPCODE_IN_RANGE						14500200


/////////////////////
// STARTUP REQUESTs
#define CS_FIRST_STARTUP_REQ                                    16100002
#define CS_NEW_REQ                                              16100003
#define CS_CONFIG_PARAM_REQ                                     16100004
#define CS_END_CONFIG_PARAM_REQ                                 16100005
#define CS_LAN_CFG_REQ                                          16100006
#define CS_RECONNECT_REQ                                        16100008
#define CS_KEEP_ALIVE_REQ					  				    16100015 // (Ami Noy)
#define CS_LAST_STARTUP_REQ                                     16100020

// SERVICE  REQUESTs
#define CS_FIRST_SERVICE_REQ                                    16100030
#define CS_NEW_SERVICE_INIT_REQ                                 16100031
#define CS_COMMON_PARAM_REQ                                     16100032
#define CS_END_SERVICE_INIT_REQ                                 16100033
#define CS_DEL_SERVICE_REQ                                      16100034
#define CS_TERMINAL_COMMAND_REQ                                 16100035
#define CS_PING_REQ                                             16100036
#define CS_LAST_SERVICE_REQ                                     16100050



// STARTUP INDICATIONs
#define CS_FIRST_STARTUP_IND                                    16100070
#define CS_NEW_IND                                              16100071
#define CS_CONFIG_PARAM_IND                                     16100072
#define CS_END_CONFIG_PARAM_IND                                 16100073
#define CS_END_CS_STARTUP_IND                                   16100074
#define CS_RECONNECT_IND                                        16100076
#define CS_KEEP_ALIVE_IND					  				    16100080 // sends a csKeepAliveSt struct	 // (Ami Noy)
#define CS_COMP_STATUS_IND					  				    16100081 // sends a csCompStatusSt struct	 // (Ami Noy)
#define CS_USER_MSG_IND											16100082 // sends a fault / active alarm
#define CS_LAST_STARTUP_IND                                     16100090


// SERVICE INDICATIONs
#define CS_FIRST_SERVICE_IND                                    16100110
#define CS_NEW_SERVICE_INIT_IND                                 16100111
#define CS_COMMON_PARAM_IND                                     16100112
#define CS_END_SERVICE_INIT_IND                                 16100113
#define CS_DEL_SERVICE_IND                                      16100114
#define CS_PING_IND		                                        16100115
#define CS_LAST_SERVICE_IND                                     16100130

// DNS
#define DNSAGENT_CS_FIRST_OPCODE_IN_RANGE						18000101

// DNS REQUESTS

#define DNSAGENT_TO_CS_FIRST_OPCODE_IN_RANGE					18000102

#define DNS_CS_FIRST_REQ										18000103
#define DNS_CS_RESOLVE_REQ										18000103
#define DNS_CS_SERVICE_REQ										18000104
#define DNS_CS_LAST_REQ											18000105

#define DNSAGENT_TO_CS_LAST_OPCODE_IN_RANGE						18000120

// DNS INDICATIONS
#define CS_TO_DNSAGENT_FIRST_OPCODE_IN_RANGE					18000121

#define DNS_CS_FIRST_IND										18000122
#define DNS_CS_RESOLVE_IND										18000122
#define DNS_CS_SERVICE_IND										18000124
#define DNS_CS_LAST_IND											18000125

#define CS_TO_DNSAGENT_LAST_OPCODE_IN_RANGE						18000199

#define DNSAGENT_CS_LAST_OPCODE_IN_RANGE						18000200


//_mccf_
////////////////////////////////////
// CS <-> MCCF Manager
////////////////////////////////////
#define MCCFMNG_CS_FIRST_OPCODE_IN_RANGE						19000101

// MCCF REQUESTS
#define MCCFMNG_TO_CS_FIRST_OPCODE_IN_RANGE						19000102

#define SIP_CS_MCCF_SIG_FIRST_REQ								19000103
#define SIP_CS_MCCF_SIG_INVITE_RESPONSE_REQ						19000104
#define SIP_CS_MCCF_SIG_CANCEL_REQ								19000105
#define SIP_CS_MCCF_SIG_BYE_REQ									19000106
#define SIP_CS_MCCF_SIG_BYE_200_OK_REQ							19000107
#define SIP_CS_MCCF_SIG_INFO_REQ								19000108
#define SIP_CS_MCCF_SIG_INFO_RESP_REQ							19000109
#define SIP_CS_MCCF_SIG_LAST_REQ								19000110

#define MCCFMNG_TO_CS_LAST_OPCODE_IN_RANGE						19000120
//


// MCCF INDICATIONS
#define CS_TO_MCCFMNG_FIRST_OPCODE_IN_RANGE						19000121

#define SIP_CS_MCCF_SIG_FIRST_IND								19000122
#define SIP_CS_MCCF_SIG_INVITE_IND								19000123
#define SIP_CS_MCCF_SIG_INVITE_ACK_IND							19000124
#define SIP_CS_MCCF_SIG_BYE_IND									19000125
#define SIP_CS_MCCF_SIG_BYE_200_OK_IND							19000126
#define SIP_CS_MCCF_SIG_BAD_STATUS_IND							19000127
#define SIP_CS_MCCF_SIG_TRANSPORT_ERROR_IND						19000128
#define SIP_CS_MCCF_SIG_INFO_RESP_IND							19000129
#define SIP_CS_MCCF_SIG_SESSION_TIMER_EXPIRED_IND				19000130
#define SIP_CS_MCCF_SIG_LAST_IND								19000131
//
#define CS_TO_MCCFMNG_LAST_OPCODE_IN_RANGE						19000199


#define MCCFMNG_CS_LAST_OPCODE_IN_RANGE							19000200
/////////////////////////////////////


// CCCP
////////////////////////////////////
// CS <-> CCCP Manager
////////////////////////////////////
#define CCCPMNG_CS_FIRST_OPCODE_IN_RANGE						20000101

// CCCP REQUESTS
#define CCCPMNG_TO_CS_FIRST_OPCODE_IN_RANGE						20000102

#define CCCPMNG_TO_CS_LAST_OPCODE_IN_RANGE						20000120

#define CCCPMNG_CS_LAST_OPCODE_IN_RANGE							20000200
/////////////////////////////////////

#endif // __IPCSOPCODES_H__