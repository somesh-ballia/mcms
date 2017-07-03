//+                                                                        +
//                       IceCmReq.h        		                           |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IceCmReq.h	                                               |
// SUBSYSTEM:  ConfParty/MFA                                               |
// PROGRAMMER: Inga														   |
// 																		   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+
#include "ICEApiDefinitions.h"

#ifndef ICECMREQ_H_
#define ICECMREQ_H_

typedef struct {
	char					user_name[IceStrLen];
	char					password[IceStrLen];
	char					realm[IceStrLen];
} iceAuthParams;

typedef  struct  {

	char					sIpAddr[IceStrLen];
	char					sHostName[IceStrLen];
	unsigned short			port;

} ICE_SERVER_PARAMS_S;

typedef struct {
	int						ice_env;// MS, Standard?/ Other
	APIU32					req_id;
	APIU32					service_id;
	iceAuthParams	    	authParams;

	ICE_SERVER_PARAMS_S   	stun_pass_server_params;
	ICE_SERVER_PARAMS_S   	stun_udp_server_params;
  	ICE_SERVER_PARAMS_S   	stun_tcp_server_params;
  	ICE_SERVER_PARAMS_S   	relay_udp_server_params;
  	ICE_SERVER_PARAMS_S   	relay_tcp_server_params;
	int			isEnableBWPolicyCheck;// true/false
	char			sCertificatePath[IceStrLen];
	char			ipAddrsStr[IceAddrInitStr]; // IP address from service of the media card. ipv6 and ipv4
	int             forced_MS_version;
} ICE_SERVER_TYPES_S;

typedef struct {
      ICE_SERVER_TYPES_S	ice_servers;
} ICE_INIT_REQ_S;

typedef ICE_INIT_REQ_S mcIceInitReq;

//-------------------------------- ice allocate candidates request ------------------------
typedef struct {
	char 					ice_pwd[IcePwdLen];
	char					ice_ufrag[IceUfragLen];
	char					ice_options[IceOptionsLen];
} mcIceCallParams;

typedef struct  {
   APIU32					channelType;        // kChanneltype
   APIU32         			channelOperation;   // cmCapOp.
   APIU16					rtp_port[MaxIcePorts];
   APIU16					rtcp_port[MaxIcePorts];

} mcIceChannelParams;

typedef struct {
	int                     	isModifyChannels;
	mcIceChannelParams			candidate_list[NumOfMediaTypes];// audio, video, content. fecc
	int							ice_session_index;
	int							max_video_rate;
	int                         max_audio_rate;
	int							sdp_size;
	char						sdp[1];

} ICE_GENERAL_REQ_S;

typedef ICE_GENERAL_REQ_S ICE_MAKE_OFFER_REQ_S;
typedef ICE_GENERAL_REQ_S mcIceMakeOfferReq;
typedef ICE_GENERAL_REQ_S ICE_MAKE_ANSWER_REQ_S;
typedef ICE_GENERAL_REQ_S ICE_PROCESS_ANSWER_REQ_S;

//-------------------------------- ice reinvite add or delete candidates request ------------------------
typedef ICE_GENERAL_REQ_S ICE_MODIFY_SESSION_ANSWER_REQ_S;
typedef ICE_GENERAL_REQ_S ICE_MODIFY_SESSION_OFFER_REQ_S;
typedef ICE_GENERAL_REQ_S mcIceModifySessionReq;

//---------------------------- ice close session request ------------------------------------
typedef ICE_GENERAL_REQ_S ICE_CLOSE_SESSION_REQ_S;


#endif /* ICECMREQ_H_ */
