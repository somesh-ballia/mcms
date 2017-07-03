//+                                                                        +
//                       IceCmInd.h      		                           |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IceCmInd.h		                                           |
// SUBSYSTEM:  ConfParty/MFA                                               |
// PROGRAMMER: Inga														   |
// 																		   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+
#ifndef ICECMIND_H_
#define ICECMIND_H_


#include "ICEApiDefinitions.h"

typedef struct {
	APIU32                      req_id;
	int							status;
	
	int			         		STUN_Pass_status;
	int			         		STUN_udp_status;
	int			         		STUN_tcp_status;
	int			         		Relay_udp_status;
	int			         		Relay_tcp_status;
	int			         		fw_type;
	int 						ice_env; // MS, Standard, WebRtc
} ICE_INIT_IND_S;

//-------------------------------- ice allocate candidates indication ------------------------
typedef  struct{
//	int							ice_session_id;
	int 						ice_audio_rtp_id;
	int 						ice_audio_rtcp_id;
	int 						ice_video_rtp_id;
	int 						ice_video_rtcp_id;
	int 						ice_data_rtp_id;
	int 						ice_data_rtcp_id;
	int 						ice_content_rtp_id;
	int 						ice_content_rtcp_id;
} iceChannelsId;

typedef struct {

	UINT16 						audioRtpPort;
	UINT16 						audioRtcpPort;
	UINT16 						videoRtpPort;
	UINT16 						videoRtcpPort;
	UINT16 						dataRtpPort;
	UINT16 						dataRtcpPort;
	UINT16 						contentRtpPort;
	UINT16 						contentRtcpPort;

} iceLocalPorts;

typedef struct {

	int							status;
	int							ice_session_index;
	iceChannelsId				ice_channels_id;
	//iceLocalPorts				ice_local_ports;
	int							allocatedBandwidth; //video
	int                         audioAllocatedBandwidth;
	int							sdp_size;
	char						sdp[1];

} ICE_GENERAL_IND_S;

typedef ICE_GENERAL_IND_S	ICE_MAKE_OFFER_IND_S;
typedef ICE_GENERAL_IND_S 	ICE_MAKE_ANSWER_IND_S;
typedef ICE_GENERAL_IND_S 	ICE_PROCESS_ANSWER_IND_S;

//---------------------------- ice reinvite indication ---------------------------------------
typedef ICE_GENERAL_IND_S 	ICE_REINVITE_IND_S;

//---------------------------- ice close session indication ----------------------------------
typedef ICE_GENERAL_IND_S	ICE_CLOSE_SESSION_IND_S;

//---------------------------- ice modify session indication ----------------------------------
typedef ICE_GENERAL_IND_S 	ICE_MODIFY_SESSION_OFFER_IND_S;
typedef ICE_GENERAL_IND_S 	ICE_MODIFY_SESSION_ANSWER_IND_S;

//---------------------------- ice bandwidth indication ----------------------------------
typedef ICE_GENERAL_IND_S 	ICE_BANDWIDTH_EVENT_IND_S;

typedef struct {
	int							status;

} ICE_ACK_IND_S;

typedef ICE_ACK_IND_S 			ICE_ERR_IND_S;

//---------------------------- ice check complete indication ---------------------------------
#define CandidateTypeLen	8
#define CandidateIpLen		32

typedef struct {

	char	type[CandidateTypeLen];
	char	ip[CandidateIpLen];
	int     mediaType; //Audio, Video - kChanneltype
	UINT16 	port;
	int 	transportType;
} candidateParamsSt;

typedef struct {
	candidateParamsSt local_candidate;
	candidateParamsSt remote_candidate;
} chosenCandidatesSt;

typedef struct {
//	TPortMessagesHeader			port_message_header;

	int					status;
	int					ice_session_index;
	int                                     allocatedBandwidth; //video
	int					audioAllocatedBandwidth;
	chosenCandidatesSt	chosen_candidates[NumOfMediaTypes];

} ICE_CHECK_COMPLETE_IND_S;


typedef struct {

	int			         		STUN_udp_status;
	int			         		STUN_tcp_status;
	int			         		Relay_udp_status;
	int			         		Relay_tcp_status;
	int			         		fw_type;
	int 						ice_env; // MS, Standard, WebRtc
} ICE_STATUS_IND_S;




#endif /* ICECMIND_H_ */
