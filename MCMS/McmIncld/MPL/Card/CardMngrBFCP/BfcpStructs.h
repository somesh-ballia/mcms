//+                                                                        +
//                       BfcpStructs.h      		                           |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       BfcpStructs.h		                                           |
// SUBSYSTEM:  ConfParty/MFA                                               |
// PROGRAMMER: Ami														   |
// 																		   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+
#ifndef BFCPSTRUCTS_H_
#define BFCPSTRUCTS_H_


// structures
//-------------

#ifndef __BFCP_CS_CONNECTION_ENABLED__
// ----------------- bfcp request -----------------
// SIP_CS_BFCP_MESSAGE_REQ
typedef struct {
	int		status;
	int		length;
	char	buffer[1];
	
} mcReqBfcpMessage;
// ----------------- bfcp indication -----------------        
// SIP_CS_BFCP_MESSAGE_IND
typedef struct {
	int 	status;
	int		length;
	char	buffer[1];

} mcIndBfcpMessage;

// IP_CM_BFCP_TCP_TRANSPORT_IND
typedef struct {
	int 	status;
} mcIndBfcpTransport;

typedef enum{
	bfcp_msg_status_ok 			= 0,
	bfcp_msg_status_connected   = 1,
	bfcp_msg_status_error 		= 2
} eBfcpMsgStatus;


#endif //__BFCP_CS_CONNECTION_ENABLED__

#endif /* BFCPSTRUCTS_H_ */
