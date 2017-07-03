/*
 * ICEApiDefinitions.h
 *
 *  Created on: Dec 20, 2009
 *      Author: inga
 */

#ifndef ICEAPIDEFINITIONS_H_
#define ICEAPIDEFINITIONS_H_

#include "MplMcmsStructs.h"
#include "IpAddressDefinitions.h"

#define IceStrLen		128
#define NumOfIceServers	8

#define NumOfMediaTypes	4

#define IceAttrLen		256
#define IcePwdLen		256
#define IceUfragLen		256
#define IceOptionsLen	8
#define MaxIcePorts		2
#define IceAddrInitStr	256

//Used by SipProxy and IceManager as specified in MS-ICE or MS-ICE2 in MS-Version attribute
#define MS_TURN_ICE1					0x00000001
#define MS_TURN_ICE2					0x00000002
#define MS_TURN_ICE2_SHA256				0x00000003
#define MS_TURN_ICE2_SHA256_IPv6		0x00000004

typedef enum {
	eIceEnvMs,
	eIceEnvStandard
} eIceEnvTypes;


typedef enum {
  iceInvalidChannel,
  iceOpenChannel 	=	1,
  iceCloseChannel	=	2
}iceCapOp;

typedef enum {

	eIceInitOk,
	eIceInitServerFail,
	eIceStunPassServerAuthenticationFailure,// STUN password server authentication failure
	eIceStunPassServerConnectionFailure, 	// Failed to connect to the STUN password server
	eIceTurnServerDnsResolveFailure,		// DNS resolution failure for the TURN server
	eIceTurnServerUnreachable,				// TURN server is not reachable
	eIceTurnServerAuthorizationFailure,		// TURN authorization failure while trying to allocate a port
	eIceServerUnavailble,
	eIceUnknownProblem

} iceServersStatus;

typedef enum{
	eIceThreadFailure = 100,
	eIceInitFail,
	eIceDetectConnectivityFail,
	eIceNoConnectionWithServer,
	eIceBadParamsInMessage,
	eIceMakeOfferFail,
	eIceMakeAnswerFail,
	eIceProcessAnswerFail,
	eIceCreateSessionFail,
	eIceCloseSessionFail,
	eIceModifySessionAnswerFail,
	eIceModifySessionOfferFail,
	eIceNoDataReceived,              		// Channel did not receive anypackets for 5 seconds after calling Connect
	eIceNoDataReceivedAfterFallbackToRelay, // Channel did not receive any packets for 5 seconds //after connecting via the STUN relay server
	eIceCheckFail,
	eIceConnectionToServerLost,				// Detected connection lost with the server
	eIceStunRelayBandwidthWarning,			// The STUN relay server warns that the channel has exceeded the allotted bandwidth
	eIceConnectFail,
} iceFailures;

typedef enum {

	eFwTypeUnknown,
	eFwTypeNone,
	eFwTypeUdp,		// UDP is OK
	eFwTypeTcpOnly,	// UDP blocked and TCP is enabled
	eFwTypeProxy,	// UDP blocked, TCP is possible only through proxy
	eFwTypeBlocked	// TCP and UDP both are blocked

} firewallTypes;




#endif /* ICEAPIDEFINITIONS_H_ */
