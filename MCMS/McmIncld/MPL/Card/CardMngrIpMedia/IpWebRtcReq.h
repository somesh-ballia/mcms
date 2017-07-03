//+========================================================================+
//                       H323MfaReq.h	                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       IpWebRtcReq.h	                                               |
// SUBSYSTEM:  MFA/ConfParty                                                |
// PROGRAMMER: Amihay F.,													   |
// Date : 13/4/14														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        
/*

This header file contains all mutual request structures related to 
ConfParty + MFA entities within the Carmel.
In this document we set the common structures.

*/

#ifndef __IP_CM_WEBRTCP_REQ_H__
#define __IP_CM_WEBRTCP_REQ_H__

#include "DataTypes.h"
#include "IpCmReq.h"

#define  MAX_MEDIA_TYPES        5
#define  WEBRTC_SDP_MAX_SIZE	30000


// IP_CM_WEBRTC_CONNECT_REQ
typedef struct
{
	mcTransportAddress					mcExternalUdpAddressIp;// UDP IP address
	mcReqCmOpenUdpPortOrUpdateUdpAddr	openUdpPorts[MAX_MEDIA_TYPES];

	//remote sdp
	APIU32								sdpSize;
	char								sdp[1];

} mcReqCmWebRtcConnect;


// IP_CM_WEBRTC_DISCONNECT_REQ
typedef struct
{
} mcReqCmWebRtcDisconnect;

#endif


