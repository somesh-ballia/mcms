//+                                                                        +
//                       IpMfaOpcodes.h                                    |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IpMfaOpcodes.h                                              |
// SUBSYSTEM:  ConfParty/MFA                                               |
// PROGRAMMER: Guy D,													   |
// Date : 15/5/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+                                                                        +                        
/*

This header file contains all opcodes between MFA and ConfParty processes 

  14000001	- 14666666: requests from ConfParty to MFA		   
  14666667	- 15000000: indication from MFA to ConfParty			   

*/

#ifndef  __IPMFAOPCODES_H__
#define  __IPMFAOPCODES_H__

#include "DataTypes.h"

////////////////////////////////////////////////
////////////  H323 <-----> MFA /////////////////
////////////////////////////////////////////////

/// Req

//#define PARTY_IP_MEDIA_FIRST_OPCODE_IN_RANGE				  14000001

#define H323_TO_MFA_FIRST_OPCODE_IN_RANGE				  14000002
// H323 to RTP
#define H323_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ			  14000003
#define H323_RTP_UPDATE_CHANNEL_REQ						  14000004 // IP_NEW_CHANNEL_MODE_REQ
#define H323_RTP_UPDATE_CHANNEL_RATE_REQ				  14000005 // IP_RTP_STREAM_ON_REQ
#define H323_RTP_FECC_TOKEN_RESPONSE_REQ				  14000006 // H323_FECC_TOKEN_REQ
#define H323_RTP_STREAM_OFF_REQ							  14000007 // IP_RTP_STREAM_OFF_REQ
#define H323_RTP_STREAM_ON_REQ							  14000008 // IP_RTP_STREAM_ON_REQ
#define H323_RTP_UPDATE_MT_PAIR_REQ						  14000009 // IP_RTP_STREAM_ON_REQ
#define H323_RTP_PARTY_MONITORING_REQ					  14000010 // IP_PARTY_MONITORING_REQ

#define H323_RTP_DBC2_COMMAND_ON_REQ					  14000011 // IP_DBC2_COMMAND_RTP_ON_REQ	
#define H323_RTP_DBC2_COMMAND_OFF_REQ					  14000012 // IP_DBC2_COMMAND_RTP_OFF_REQ
#define ART_CONTENT_ON_REQ								  14000013
#define ART_CONTENT_OFF_REQ						    	  14000014
#define ART_EVACUATE_REQ							  	  14000015
//#define H323_RTP_CHNL_DROP_REQ						  14000013 // H323_CHNL_DROP_REQ
//#define H323_RTP_START_VIDEO_CLIP_REQ					  14000014 // IP_ATTENDED_VIDEO_START_REQ
//#define H323_RTP_STOP_VIDEO_CLIP_REQ					  14000015 // IP_ATTENDED_VIDEO_STOP_REQ
#define H323_RTP_LPR_MODE_CHANGE_REQ					  14000016
#define H323_RTP_LPR_MODE_RESET_REQ					 	  14000017
#define IP_CM_START_PREVIEW_CHANNEL  					  14000018
#define IP_CM_STOP_PREVIEW_CHANNEL   					  14000019
#define IP_CM_RTCP_MSG_REQ								  14000020
#define IP_CM_RTCP_POLICY_SERVER_BANDWIDTH_REQ            14000021
#define IP_CM_RTCP_RECEIVER_BANDWIDTH_REQ                 14000022
#define IP_CM_RTCP_TURN_SERVER_BANDWIDTH_REQ              14000023
#define IP_CM_RTCP_VIDEO_PREFERENCE_REQ					  14000024
#define IP_CM_RTCP_RTPFB_REQ					          14000025 // For TMMBR feedback
#define RTP_PARTY_VIDEO_CHANNELS_STATISTICS_REQ		      14000026 // FOR PARTY STATISTICS MCCF CDR

#define IP_CM_DTLS_START_REQ					  		  14000027
#define IP_CM_DTLS_CLOSE_REQ					  		  14000028

//Lync2013 requests:
#define IP_CM_RTCP_VSR_REQ 							      14000030
#define IP_CM_RTCP_MS_SVC_PLI_REQ						  14000031
#define IP_CM_RTCP_VSR_REQ 								  14000030
#define IP_CM_RTCP_MS_SVC_PLI_REQ						  14000031

#define IP_CM_WEBRTC_CONNECT_REQ						  14000032
#define IP_CM_WEBRTC_DISCONNECT_REQ						  14000033

#define RTP_PARTY_MONITORING_AV_MCU_REQ					          14000034 // IP_PARTY_MONITORING_REQ for AV-MCU

// KILL_UDP_PORT_REQ (formerly 14100003, now 9030010) was moved to OpcodesMcmsCardMngrIpMedia.h

// Move Ip to MPL (MFA)
//#define IP_MFA_MOVE_RESOURCES_REQ						  14120000

#define H323_TO_MFA_LAST_OPCODE_IN_RANGE				  14150000

// SIP request
#define SIP_TO_MFA_FIRST_OPCODE_IN_RANGE				  14200000
// SIP to RTP
#define SIP_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ			  14200001
#define SIP_RTP_UPDATE_CHANNEL_REQ						  14200002 // IP_NEW_CHANNEL_MODE_REQ
#define SIP_RTP_UPDATE_CHANNEL_RATE_REQ		    		  14200003 // H323_CHAN_NEW_RATE_REQ
#define SIP_RTP_FECC_TOKEN_RESPONSE_REQ					  14200004 // H323_FECC_TOKEN_REQ
#define SIP_RTP_STREAM_OFF_REQ							  14200005 // IP_RTP_STREAM_OFF_REQ
#define SIP_RTP_STREAM_ON_REQ							  14200006 // IP_RTP_STREAM_ON_REQ
#define SIP_RTP_PARTY_MONITORING_REQ					  14200007 // IP_PARTY_MONITORING_REQ

#define SIP_RTP_DBC2_COMMAND_ON_REQ						  14200008 // IP_DBC2_COMMAND_RTP_ON_REQ	
#define SIP_RTP_DBC2_COMMAND_OFF_REQ					  14200009 // IP_DBC2_COMMAND_RTP_OFF_REQ
//#define SIP_RTP_CHNL_DROP_REQ							  14200010 // H323_CHNL_DROP_REQ
//#define SIP_RTP_START_VIDEO_CLIP_REQ					  14200011
//#define SIP_RTP_STOP_VIDEO_CLIP_REQ					  14200012
#define SIP_RTP_LPR_MODE_CHANGE_REQ						  H323_RTP_LPR_MODE_CHANGE_REQ   //This is the same opcode as H323_RTP_LPR_MODE_CHANGE_REQ
//#define SIP_RTP_LPR_MODE_RESET_REQ					 	  14000013  //this is the same opcode as H323_RTP_LPR_MODE_RESET_REQ

// SIP to CM
#define SIP_CM_OPEN_UDP_PORT_REQ						  14300000
#define SIP_CM_UPDATE_UDP_ADDR_REQ						  14300001
#define SIP_CM_CLOSE_UDP_PORT_REQ						  14300002

#define ACTIVATE_PACKET_LOSS_REQ						  14300010
#define ART_FEC_RED_ON_OFF_REQ						      14300011

#define SIP_TO_MFA_LAST_OPCODE_IN_RANGE					  14250000

// shared opcodes (SIP and H.323)
#define IP_TO_MFA_FIRST_OPCODE_IN_RANGE				  	  14250001

#define IP_RTP_SET_FECC_PARTY_TYPE 					 	  14250002

#define IP_TO_MFA_LAST_OPCODE_IN_RANGE					  14300000


#define H320_TO_MFA_FIRST_OPCODE_IN_RANGE				  14350000
// H320 to ART
#define H320_RTP_UPDATE_PORT_OPEN_CHANNEL_REQ			  14350002
#define H320_RTP_UPDATE_CHANNEL_REQ						  14350003
#define H320_RTP_UPDATE_CHANNEL_RATE_REQ				  14350004
// #define H320_MUX_CONTENT_OFF_REQ						  14350005
// #define H320_MUX_CONTENT_ON_REQ							  14350006
// #define H320_MUX_EVACUATE_REQ	    					  14350007	


#define H320_TO_MFA_LAST_OPCODE_IN_RANGE				  14351000

/// Ind to H323 and SIP
#define MFA_TO_IP_PARTY_FIRST_OPCODE_IN_RANGE			  14670000
#define IP_RTP_VIDEO_UPDATE_PIC_IND						  14670001
#define IP_RTP_BAD_SPONTAN_IND							  14670002
#define IP_RTP_DTMF_INPUT_IND							  14670003
#define IP_RTP_STREAM_STATUS_IND						  14670004
#define IP_RTP_FECC_TOKEN_IND							  14670005
#define IP_CM_PARTY_MONITORING_IND						  14670006
#define IP_RTP_LPR_MODE_CHANGE_IND						  14670007
#define IP_RTP_DBA_IND									  14670008
#define IP_RTP_DIFF_PAYLOAD_TYPE_IND					  14670009
#define IP_RTP_FECC_KEY_IND								  14670010
#define IP_CM_PARTY_PACKET_LOSS_IND						  14670011
#define IP_CM_RTCP_MSG_IND 					              14670012
#define IP_RTP_ASK_ENCODER_FOR_INTRA_IND                  14670013
#define IP_CM_RTCP_POLICY_SERVER_BANDWIDTH_IND            14670014
#define IP_CM_RTCP_RECEIVER_BANDWIDTH_IND                 14670015
#define IP_CM_RTCP_TURN_SERVER_BANDWIDTH_IND              14670016
#define IP_CM_RTCP_VIDEO_PREFERENCE_IND			  		  14670017

#define CM_SEND_CNAME_INFO_AS_STRING_IND                  14670018
#define IP_CM_RTCP_RTPFB_IND 					          14670019 // For TMMBR FB

#define IP_CM_RTCP_PACKET_LOSS_STATUS_IND				  14670020
#define IP_CM_DTLS_END_IND						  		  14670021
#define IP_CM_PARTY_VIDEO_CHANNELS_STATISTICS_IND		  14670022

#define IP_CM_MEDIA_DISCONNECTED_IND					  14670023 //For SIP disconection detection

//Lync2013 indications:
#define IP_CM_RTCP_VSR_IND                    			  14670024
#define IP_CM_RTCP_MS_SVC_PLI_IND                 		  14670025
#define IP_CM_RTCP_DSH_IND                    			  14670026

#define IP_RTP_FECC_MUTE_IND					  		  14670027
                      

#define IP_CM_WEBRTC_CONNECT_IND				  14670028
#define IP_CM_WEBRTC_SERVER_STAT_CHANGE_IND			  14670029

#define IP_CM_PARTY_MONITORING_AV_MCU_IND			  14670030


#define MFA_TO_IP_PARTY_LAST_OPCODE_IN_RANGE			  14770000

#define MFA_TO_H320_PARTY_FIRST_OPCODE_IN_RANGE			  14870000
#define H320_RTP_VIDEO_UPDATE_PIC_IND					  14870001


#define  JITTER_BUFFER_CONTROL_IND    					  14970000
#define  JITTER_BUFFER_INFO_IND    						  14970001	

#define MFA_TO_H320_PARTY_LAST_OPCODE_IN_RANGE			  14999999


//#define PARTY_IP_MEDIA_LAST_OPCODE_IN_RANGE					  15000000

#endif
