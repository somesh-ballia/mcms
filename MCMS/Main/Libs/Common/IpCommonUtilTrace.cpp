// use for tracingof the string arrays of enums in the API IpChannelsParams.h and Capabilities.h files
// Uri A.

#include <stdio.h>
#include <string.h>
#include "IpCommonUtilTrace.h"
#include "IpChannelParams.h"
#include "Capabilities.h"
#include "IpEncryptionDefinitions.h"
#include "ObjString.h"
#include "RvCommonDefs.h"
#include "DisconnectCause.h"
#include "BFCPMessage.h"
#include "IpPartyMonitorDefinitions.h"
#include "IpCmReq.h"
#include "IpRtpFeccRoleToken.h"
#include "IpRtpReq.h"

//////////////////////////////////////////////////////////////////////////////////////////////
void GetChannelTypeName(APIU32 channelType, CObjString& msg)
{
 	switch(channelType)
	{
		case kIpAudioChnlType:
		{
			msg << "AudioChnlType";
			return;
		}
		case kIpVideoChnlType:
		{
			msg << "VideoChnlType";
			return;
		}
		case kIpFeccChnlType:
		{
			msg << "FeccChnlType";
			return;
		}
		case kIpContentChnlType:
		{
			msg << "ContentChnlType";
			return;
		}
	    case kPstnAudioChnlType:
		{
		  msg << "PstnAudioChnlType";
		  return;
		}
		case kRtmChnlType:
		{
		  msg << "RtmChnlType";
		  return;
		}
		case kIsdnMuxChnlType:
		{
		  msg << "IsdnMuxChnlType";
		  return;
		}
		case kBfcpChnlType:
		{
		  msg << "BfcpChnlType";
		  return;
		}
		case kSvcAvcChnlType:
		{
		  msg << "SvcAvcChnlType";
		  return;
		}
		case kAvcVSWChnType:
		{
		  msg << "kAvcVSWChnType";
		  return;
		}
        case kAvcToSacChnlType:
        {
          msg << "kAvcToSacChnlType";
          return;
        }
        case kAvcToSvcChnlType:
        {
          msg << "kAvcToSvcChnlType";
          return;
        }

	    default:
		{
			msg << "Unknown Channel Type - " << channelType;
			return;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void GetMsgTypeName(APIU32 MsgType, CObjString &msg)
{
	switch(MsgType)
	{
		case RTCP_PLI:
		{
			msg << "RTCP_PLI";
			return;
		}
		case RTCP_SLI:
		{
			msg << "RTCP_SLI";
			return;
		}
		//case RTCP_RPSI:
		case RTCP_TMMBR:
		{
			msg << "RTCP_TMMBR";
			return;
		}
		case RTCP_FIR:
		{
			msg << "RTCP_FIR";
			return;
		}
		case RTCP_TSTR:
		{
			msg << "RTCP_TSTR";
			return;
		}
		case RTCP_VBCM:
		{
			msg << "RTCP_VBCM";
			return;
		}
		case RTCP_APP_LAYER_FB:
		{
			msg << "RTCP_APP_LAYER_FB";
			return;
		}
		case RTCP_FUTURE_EXPANSION:
		{
			msg << "RTCP_FUTURE_EXPANSION";
			return;
		}
		case RTCP_INTRA_RTV:
		{
			msg << "RTCP_INTRA_RTV";
			return;
		}
		case RTCP_TIP_IDR:
		{
			msg << "RTCP_TIP_IDR";
			return;
		}
		case RTCP_TIP_GDR:
		{
			msg << "RTCP_TIP_GDR";
			return;
		}
		case RTCP_TIP_PKT_LOSS:
		{
			msg << "RTCP_TIP_PKT_LOSS";
			return;
		}
		case RTCP_WEBRTC_FIR:
		{
			msg << "RTCP_WEBRTC_FIR";
			return;
		}
		case RTCP_WEBRTC_TMMBR:
		{
			msg << "RTCP_WEBRTC_TMMBR";
			return;
		}
		default:
		{
			msg << "Unknown Msg Type - " << MsgType;
			return;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////
void GetAnnexTypeName(APIU32 annexType, CObjString &msg)
{
	switch(annexType)
	{
		case typeAnnexB:
		{
			msg << "B";
			return;
		}
		case typeAnnexD:
		{
			msg << "D";
			return;
		}
		case typeAnnexE:
		{
			msg << "E";
			return;
		}
		case typeAnnexF:
		{
			msg << "F";
			return;
		}
		case typeAnnexG:
		{
			msg << "G";
			return;
		}
		case typeAnnexH:
		{
			msg << "H";
			return;
		}
		case typeAnnexI:
		{
			msg << "I";
			return;
		}
		case typeAnnexJ:
		{
			msg << "J";
			return;
		}
		case typeAnnexK:
		{
			msg << "K";
			return;
		}
		case typeAnnexL:
		{
			msg << "L";
			return;
		}
		case typeAnnexM:
		{
			msg << "M";
			return;
		}
		case typeAnnexN:
		{
			msg << "N";
			return;
		}
		case typeAnnexO:
		{
			msg << "O";
			return;
		}
		case typeAnnexP:
		{
			msg << "P";
			return;
		}
		case typeAnnexQ:
		{
			msg << "Q";
			return;
		}
		case typeAnnexR:
		{
			msg << "R";
			return;
		}
		case typeAnnexS:
		{
			msg << "S";
			return;
		}
		case typeAnnexT:
		{
			msg << "T";
			return;
		}
		case typeAnnexU:
		{
			msg << "U";
			return;
		}
		case typeAnnexV:
		{
			msg << "V";
			return;
		}
		case typeAnnexW:
		{
			msg << "W";
			return;
		}
		case typeAnnexI_NS:
		{
			msg << "I_NS";
			return;
		}
		case typeCustomPic:
		{
			msg << "CustomPic";
			return;
		}

		default:
		{
			msg << "Unknown Annex Type - " << annexType;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
void GetChannelDirectionName(APIU32 channelDirection, CObjString& msg)
{
	switch(channelDirection)
	{
		case cmCapReceive:
		{
			msg << "Receive";
			return;
		}
		case cmCapTransmit:
		{
			msg << "Transmit";
			return;
		}
		case cmCapReceiveAndTransmit:
		{
			msg << "Rec + Tr";
			return;
		}
		default:
		{
			msg << "Unknown Direction - " << channelDirection;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetMediaModeName(APIU32 mediaMode, CObjString& msg)
{
    switch(mediaMode)
    {
        case eMediaModeRelayOnly:
        {
            msg << "Relay Only";
            return;
        }
        case eMediaModeTranscoding:
        {
            msg << "Transcoding";
            return;
        }
        case eMediaModeRelayAndTranscoding:
        {
            msg << "Relay and Transcoding";
            return;
        }
        case eMediaModeVswMode:
        {
            msg << "VSW mode";
            return;
        }
        case eMediaModeTranscodingAndVsw:
        {
            msg << "Transcoding and VSW";
            return;
        }
        default:
        {
            msg << "Unknown MediaMode - " << mediaMode;
            return;
        }
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
char* GetTransportTypeName(APIU32 enTransportType)
{
  switch (enTransportType)
  {
    case eUnknownTransportType: return "Unknown";
    case eTransportTypeUdp    : return "Udp";
    case eTransportTypeTcp    : return "Tcp";
    case eTransportTypeTls    : return "Tls";
    default                   : return "Invalid";
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetGenericCodeName(APIU32 genericCode, CObjString& msg)
{
	switch(genericCode)
	{
		case eH26LCode:
		{
			msg << "H26LCode";
			return;
		}
		case eDropField:
		{
			msg << "DropField";
			return;
		}
		case eDBC2Code:
		{
			msg << "DBC2Code";
			return;
		}
		default:
		{
			msg << "Unknown generic code name - " << genericCode;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetEncryptionMediaTypeName(APIU32 encryptionMediaType, CObjString& msg)
{
	switch(encryptionMediaType)
	{
		case kAES_CBC:
		{
			msg << "AES_CBC";
			return;
		}
		case kAES_CTR:
		{
			msg << "AES_CTR";
			return;
		}
		default:
		{
			msg << "Unknown encryption media name - " << encryptionMediaType;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetEndpointTypeName(APIU32 endpointType, CObjString& msg)
{
	switch(endpointType)
	{
		case cmEndpointTypeTerminal:
		{
			msg << "Terminal";
			return;
		}
		case cmEndpointTypeGateway:
		{
			msg << "GW";
			return;
		}
		case cmEndpointTypeMCU:
		{
			msg << "MCU";
			return;
		}
		case cmEndpointTypeGK:
		{
			msg << "GK";
			return;
		}
		case cmEndpointTypeUndefined:
		{
			msg << "Undefined";
			return;
		}
		case cmEndpointTypeSET:
		{
			msg << "SET";
			return;
		}
		default:
		{
			msg << "number - " << endpointType;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetAliasTypeName(APIU32 aliasType, CObjString& msg)
{
	switch(aliasType)
	{
		case cmAliasTypeE164:
		{
			msg << "E164";
			return;
		}
		case cmAliasTypeH323ID:
		{
			msg << "H323ID";
			return;
		}
		case cmAliasTypeEndpointID:
		{
			msg << "EndpointID";
			return;
		}
		case cmAliasTypeGatekeeperID:
		{
			msg << "GK_ID";
			return;
		}
		case cmAliasTypeURLID:
		{
			msg << "URL_ID";
			return;
		}
		case cmAliasTypeTransportAddress:
		{
			msg << "TransportAddress";
			return;
		}
		case cmAliasTypeEMailID:
		{
			msg << "EmailID";
			return;
		}
		case cmAliasTypePartyNumber:
		{
			msg << "PartyNumber";
			return;
		}
		default:
		{
			msg << "number - " << aliasType;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetCallTypeName(APIU32 callType, CObjString& msg)
{
	switch(callType)
	{
		case cmCallTypeP2P:
		{
			msg << "P2P";
			return;
		}
		case cmCallTypeOne2N:
		{
			msg << "One2N";
			return;
		}
		case cmCallTypeN2One:
		{
			msg << "N2One";
			return;
		}
		case cmCallTypeN2Nw:
		{
			msg << "N2Nw";
			return;
		}
		default:
		{
			msg << "number - " << callType;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetRasRejectReasonTypeName(APIU32 rasRejectReasonType, CObjString& msg)
{
	switch(rasRejectReasonType)
	{
		case cmRASReasonResourceUnavailable:
		{
			msg << "Resource Unavailable";
			return;
		}
		case cmRASReasonInsufficientResources:
		{
			msg << "Insufficient Resources";
			return;
		}
		case cmRASReasonInvalidRevision:
		{
			msg << "Invalid Revision";
			return;
		}
		case cmRASReasonInvalidCallSignalAddress:
		{
			msg << "Invalid Call Signal Address";
			return;
		}
		case cmRASReasonInvalidRASAddress:
		{
			msg << "Invalid RAS Address";
			return;
		}
		case cmRASReasonInvalidTerminalType:
		{
			msg << "Invalid Terminal Type";
			return;
		}
		case cmRASReasonInvalidPermission:
		{
			msg << "Invalid Permission";
			return;
		}
		case cmRASReasonInvalidConferenceID:
		{
			msg << "Invalid Conference ID";
			return;
		}
		case cmRASReasonInvalidEndpointID:
		{
			msg << "Invalid Endpoint ID";
			return;
		}
		case cmRASReasonCallerNotRegistered:
		{
			msg << " Caller Not Registered";
			return;
		}
		case cmRASReasonCalledPartyNotRegistered:
		{
			msg << "Called Party Not Registered";
			return;
		}
		case cmRASReasonDiscoveryRequired:
		{
			msg << "Discovery Required";
			return;
		}
		case cmRASReasonDuplicateAlias:
		{
			msg << "Duplicate Alias";
			return;
		}
		case cmRASReasonTransportNotSupported:
		{
			msg << "Transport Not Supported";
			return;
		}
		case cmRASReasonCallInProgress:
		{
			msg << "Call In Progress";
			return;
		}
		case cmRASReasonRouteCallToGatekeeper:
		{
			msg << "Route Call To Gatekeeper";
			return;
		}
		case cmRASReasonRequestToDropOther:
		{
			msg << "Request To Drop Other";
			return;
		}
		case cmRASReasonNotRegistered:
		{
			msg << "Not Registered";
			return;
		}
		case cmRASReasonUndefined:
		{
			msg << "Undefined";
			return;
		}
		case cmRASReasonTerminalExcluded:
		{
			msg << "Terminal Excluded";
			return;
		}
		case cmRASReasonNotBound:
		{
			msg << "Not Bound";
			return;
		}
		case cmRASReasonNotCurrentlyRegistered:
		{
			msg << "Not Currently Registered";
			return;
		}
		case cmRASReasonRequestDenied:
		{
			msg << "Request Denied";
			return;
		}
		case cmRASReasonLocationNotFound:
		{
			msg << "Location Not Found";
			return;
		}
		case cmRASReasonSecurityDenial:
		{
			msg << "Security Denial";
			return;
		}
		case cmRASReasonTransportQOSNotSupported:
		{
			msg << "Transport QOS Not Supported";
			return;
		}
		case cmRASResourceUnavailable:
		{
			msg << "Resource Unavailable";
			return;
		}
		case cmRASReasonInvalidAlias:
		{
			msg << "Invalid Alias";
			return;
		}
		case cmRASReasonPermissionDenied:
		{
			msg << "Permission Denied";
			return;
		}
		case cmRASReasonQOSControlNotSupported:
		{
			msg << "QOS Control Not Supported";
			return;
		}
		case cmRASReasonIncompleteAddress:
		{
			msg << "Incomplete Address";
			return;
		}
		case cmRASReasonFullRegistrationRequired:
		{
			msg << "Full Registration Required";
			return;
		}
		case cmRASReasonRouteCallToSCN:
		{
			msg << "Route Call To SCN";
			return;
		}
		case cmRASReasonAliasesInconsistent:
		{
			msg << "Aliases Inconsistent";
			return;
		}
		case cmRASReasonAdditiveRegistrationNotSupported:
		{
			msg << "Additive Registration Not Supported";
			return;
		}
		case cmRASReasonInvalidTerminalAliases:
		{
			msg << "Invalid Terminal Aliases";
			return;
		}
		case cmRASReasonExceedsCallCapacity:
		{
			msg << "Exceeds Call Capacity";
			return;
		}
		case cmRASReasonCollectDestination:
		{
			msg << "Collect Destination";
			return;
		}
		case cmRASReasonCollectPIN:
		{
			msg << "Collect PIN";
			return;
		}
		case cmRASReasonGenericData:
		{
			msg << "Generic Data";
			return;
		}
		case cmRASReasonNeededFeatureNotSupported:
		{
			msg << "Needed Feature Not Supported";
			return;
		}
		case cmRASReasonUnknownMessageResponse:
		{
			msg << "Unknown Message Response";
			return;
		}
		case cmRASReasonHopCountExceeded:
		{
			msg << "Hop Count Exceeded";
			return;
		}
		default:
		{
			msg << "number - " << rasRejectReasonType;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetConferenceGoalTypeName(APIU32 conferenceGoalType, CObjString& msg)
{
	switch(conferenceGoalType)
	{
		case cmCreate:
		{
			msg << "Create";
			return;
		}
		case cmJoin:
		{
			msg << "Join";
			return;
		}
		case cmInvite:
		{
			msg << "Invite";
			return;
		}
		case cmCapabilityNegotiation:
		{
			msg << "Capability Negotiation";
			return;
		}
		case cmCallIndependentSupplementaryService:
		{
			msg << "Call In dependent Supplementary Service";
			return;
		}
		default:
		{
			msg << "number - " << conferenceGoalType;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetRejectReasonTypeName(APIU32 rejectReasonType, CObjString& msg)
{
	switch(rejectReasonType)
	{
		case cmReasonTypeNoBandwidth:
		{
			msg << "No Bandwidth";
			return;
		}
		case cmReasonTypeGatekeeperResource:
		{
			msg << "Gatekeeper Resource";
			return;
		}
		case cmReasonTypeUnreachableDestination:
		{
			msg << "Unreachable Destination";
			return;
		}
		case cmReasonTypeDestinationRejection:
		{
			msg << "Destination Rejection";
			return;
		}
		case cmReasonTypeInvalidRevision:
		{
			msg << "Invalid Revision";
			return;
		}
		case cmReasonTypeNoPermision:
		{
			msg << "No Permision";
			return;
		}
		case cmReasonTypeUnreachableGatekeeper:
		{
			msg << "Unreachable Gatekeeper";
			return;
		}
		case cmReasonTypeGatewayResource:
		{
			msg << "Gateway Resource";
			return;
		}
		case cmReasonTypeBadFormatAddress:
		{
			msg << "Bad Format Address";
			return;
		}
		case cmReasonTypeAdaptiveBusy:
		{
			msg << "Adaptive Busy";
			return;
		}
		case cmReasonTypeInConf:
		{
			msg << "In Conf";
			return;
		}
		case cmReasonTypeUndefinedReason:
		{
			msg << "Undefined Reason";
			return;
		}
		case cmReasonTypeRouteCallToGatekeeper:
		{
			msg << "Route Call To Gatekeeper";
			return;
		}
		case cmReasonTypeCallForwarded:
		{
			msg << "Call Forwarded";
			return;
		}
		case cmReasonTypeRouteCallToMC:
		{
			msg << "Route Call To MC";
			return;
		}
		case cmReasonTypeFacilityCallDeflection:
		{
			msg << "Facility Call Deflection";
			return;
		}
		case cmReasonTypeSecurityDenied:
		{
			msg << "Security Denied";
			return;
		}
		case cmReasonTypeCalledPartyNotRegistered:
		{
			msg << "Called Party Not Registered";
			return;
		}
		case cmReasonTypeCallerNotregistered:
		{
			msg << "Caller Not registered";
			return;
		}
		case cmReasonTypeConferenceListChoice:
		{
			msg << "Conference List Choice";
			return;
		}
		case cmReasonTypeStartH245:
		{
			msg << "Start H245";
			return;
		}
		case cmReasonTypeNewConnectionNeeded:
		{
			msg << "New Connection Needed";
			return;
		}
		case cmReasonTypeNoH245:
		{
			msg << "No H245";
			return;
		}
		case cmReasonTypeNewTokens:
		{
			msg << "New Tokens";
			return;
		}
		case cmReasonTypeFeatureSetUpdate:
		{
			msg << "Feature Set Update";
			return;
		}
		case cmReasonTypeForwardedElements:
		{
			msg << "Forwarded Elements";
			return;
		}
		case cmReasonTypeTransportedInformation:
		{
			msg << "Transported Information";
			return;
		}
		default:
		{
			msg << "number - " << rejectReasonType;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetCallModelTypeName(APIU32 callModelType, CObjString& msg)
{
	switch(callModelType)
	{
		case cmCallModelTypeDirect:
		{
			msg << "Direct call";
			return;
		}
		case cmCallModelTypeGKRouted:
		{
			msg << "Routed call";
			return;
		}
		default:
		{
			msg << "number - " << callModelType;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////
void GetCallCloseStateModeName(APIU32 callCloseState, CObjString& msg)
{
	switch(callCloseState)
	{
		case callCloseReasonRemoteBusy:
		{
			msg << "Remote Busy";
			return;
		}
		case callCloseReasonNormal:
		{
			msg << "Reason Normal";
			return;
		}
		case callCloseReasonRemoteReject:
		{
			msg << "Remote Reject";
			return;
		}
		case callCloseReasonRemoteUnreachable:
		{
			msg << "Remote Unreachable";
			return;
		}
		case callCloseReasonUnknownReason:
		{
			msg << "Unknown Reason";
			return;
		}
		case callCloseReasonClosedByMcms:
		{
			msg << "Closed By Mcms";
			return;
		}
		case callStateModesNeedMaintenance:
		{
			msg << "FAULTY_DESTINATION_ADDRESS";
			return;
		}
		default:
		{
			msg << "number - " << callCloseState;
			return;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetICEErrorAsString(DWORD Err,CObjString& msg)
{
	switch(Err)
	{
		case eIceThreadFailure:
		{
			msg << "Thread Failure";
			return;
		}
		case eIceInitFail:
		{
			msg << "Init Fail";
			return;
		}
		case eIceDetectConnectivityFail:
		{
			msg << "Detect Connectivity Fail";
			return;
		}
		case eIceNoConnectionWithServer:
		{
			msg << "No Connection With Server";
			return;
		}
		case eIceBadParamsInMessage:
		{
			msg << "Bad Params In Message";
			return;
		}
		case eIceMakeOfferFail:
		{
			msg << "Make Offer Fail";
			return;
		}
		case eIceMakeAnswerFail:
		{
			msg << "Make Answer Fail";
			return;
		}
		case eIceProcessAnswerFail:
		{
			msg << "Process Answer Fail";
			return;
		}
		case eIceCreateSessionFail:
		{
			msg << "Create Session Fail";
			return;
		}
		case eIceCloseSessionFail:
		{
			msg << "Closee Session Fail";
			return;
		}
		case eIceModifySessionAnswerFail:
		{
			msg << "Modify Session Answer Fail";
			return;
		}
		case eIceModifySessionOfferFail:
		{
			msg << "Modify Session Offer Fail";
			return;
		}
		case eIceNoDataReceived:
		{
			msg << "No Data Received";
			return;
		}
		case eIceNoDataReceivedAfterFallbackToRelay:
		{
			msg << "No Data Received After Fallback To Relay";
			return;
		}
		case eIceCheckFail:
		{
			msg << "Check Fail";
			return;
		}
		case eIceConnectionToServerLost:
		{
			msg << "Connect To Server Lost";
			return;
		}
		case eIceStunRelayBandwidthWarning:
		{
			msg << "Stun Relay Bandwidth Warning";
			return;
		}
		case eIceConnectFail:
		{
			msg << "Connect Fail";
			return;
		}
		default:
		{
			msg << "Unknown Error status";
		}
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////
void GetBFCPTransactionType(APIU32 type, CObjString& msg)
{
	switch(type)
	{
		case kBFCPFloorRequest:
		{
			msg << "BFCPFloorRequest";
			return;
		}
		case kBFCPFloorRelease:
		{
			msg << "BFCPFloorRelease";
			return;
		}
		case kBFCPFloorRequestQuery:
		{
			msg << "BFCPFloorRequestQuery";
			return;
		}
		case kBFCPFloorRequestStatus:
		{
			msg << "BFCPFloorRequestStatus";
			return;
		}
		case kBFCPUserQuery:
		{
			msg << "BFCPUserQuery";
			return;
		}
		case kBFCPUserStatus:
		{
			msg << "BFCPUserStatus";
			return;
		}
		case kBFCPFloorQuery:
		{
			msg << "BFCPFloorQuery";
			return;
		}
		case kBFCPFloorStatus:
		{
			msg << "BFCPFloorStatus";
			return;
		}
		case kBFCPChairAction:
		{
			msg << "BFCPChairAction";
			return;
		}
		case kBFCPChairActionAck:
		{
			msg << "BFCPChairActionAck";
			return;
		}
		case kBFCPHello:
		{
			msg << "BFCPHello";
			return;
		}
		case kBFCPHelloAck:
		{
			msg << "BFCPHelloAck";
			return;
		}
		case kBFCPError:
		{
			msg << "BFCPError";
			return;
		}
		case kBFCPFloorRequestStatusAck:
		{
			msg << "BFCPFloorRequestStatusAck";
			return;
		}
		case kBFCPErrorAck:
		{
			msg << "BFCPErrorAck";
			return;
		}
		case kBFCPFloorStatusAck:
		{
			msg << "BFCPFloorStatusAck";
			return;
		}
		case kBFCPGoodbye:
		{
			msg << "BFCPGoodbye";
			return;
		}
		case kBFCPGoodbyeAck:
		{
			msg << "BFCPGoodbyeAck";
			return;
		}
		case kTandbergBFCPFloorRequestStatusAck:
		{
			msg << "TandbergBFCPFloorRequestStatusAck";
			return;
		}
		case kTandbergBFCPFloorStatusAck:
		{
			msg << "TandbergBFCPFloorStatusAck";
			return;
		}
		default:
		{
			msg << "Invalid type";
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
void GetBFCPFloorStatusType(APIU32 type, CObjString& msg)
{
	switch(type)
	{
		case kBFCPStatusPending:
		{
			msg << "kBFCPStatusPending";
			return;
		}
		case kBFCPStatusAccepted:
		{
			msg << "kBFCPStatusAccepted";
			return;
		}
		case kBFCPStatusGranted:
		{
			msg << "kBFCPStatusGranted";
			return;
		}
		case kBFCPStatusDenied:
		{
			msg << "kBFCPStatusDenied";
			return;
		}
		case kBFCPStatusCancelled:
		{
			msg << "kBFCPStatusCancelled";
			return;
		}
		case kBFCPStatusReleased:
		{
			msg << "kBFCPStatusReleased";
			return;
		}
		case kBFCPStatusRevoked:
		{
			msg << "kBFCPStatusRevoked";
			return;
		}
		default:
		{
			msg << "0";
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
char* GetConnectionTransportModeName(APIU32 enConnectionTransportMode)
{
  switch (enConnectionTransportMode)
  {
    case eUdp		: return "Udp";
    case eTcpActive : return "Tcp Active";
    case eTcpPassive: return "Tcp Passive";
    default         : return "Invalid";
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////
void GetFeccTokenResponseName(APIU32 feccTokenResponse,CObjString& msg)
{
	switch(feccTokenResponse)
	{
		case kTokenRequest:
		{
			msg << "Request";
			return;
		}
		case kTokenAccept:
		{
			msg << "Accept";
			return;
		}
		case kTokenReject:
		{
			msg << "Reject";
			return;
		}
		case kTokenRelease:
		{
			msg << "Release";
			return;
		}
		case kTokenReleaseRequest:
		{
			msg << "ReleaseRequest";
			return;
		}
		case kTokenWithdrow:
		{
			msg << "Withdrow";
			return;
		}
		default:
		{
			msg << "Invalid response";
			return;
		}
	}
}

/////////////////////////////////////////////////////////////////////////
void getIceStatusErrorText(iceServersStatus status,  CObjString& msg)
{
	switch (status)
	{
	case eIceInitOk:
		msg << "Server init OK";
		break;
	case eIceInitServerFail:
		msg <<  "Server init Fail";
		break;
	case eIceStunPassServerAuthenticationFailure:
		msg << "Server Authentication Failure";
		break;
	case eIceStunPassServerConnectionFailure:
		msg << "Stun Pass Server Connection Failure";
		break;
	case eIceTurnServerDnsResolveFailure:
		msg << "Turn Server DNS Resolve Failure";
		break;
	case eIceTurnServerUnreachable:
		msg << "Turn Server Unreachable";
		break;
	case eIceTurnServerAuthorizationFailure:
		msg << "Turn Server Authorization Failure";
		break;
	case eIceServerUnavailble:
		msg << "Server Unavailable";
		break;
	case eIceUnknownProblem:
	default:
		msg << "Unknown Problem";
		break;
	}
}

/////////////////////////////////////////////////////////////////////////
void getIceFwType(firewallTypes type,  CObjString& msg)
{
	switch (type)
	{
	case eFwTypeNone:
		msg << "None";
		break;
	case eFwTypeUdp:
		 msg << "UDP is Enabled";
		 break;
	case eFwTypeTcpOnly:
		msg << "TCP Only - UDP blocked and TCP is enabled";
		break;
	case eFwTypeProxy:
		msg << "PROXY - UDP blocked, TCP is possible only through proxy";
		break;
	case eFwTypeBlocked:
		msg<< "BLOCKED - TCP and UDP both are blocked";
		break;
	case eFwTypeUnknown:
	default:
		msg << "Unknown Problem";
		break;
	}
}

/////////////////////////////////////////////////////////////////////////
void getIceEnv(eIceEnvTypes env,  CObjString& msg)
{
	switch (env)
	{
	case eIceEnvMs:
		msg << "MS ICE";
		break;
	case eIceEnvStandard:
		msg << "Standard/IBM ICE";
		break;
	default:
		msg << "Unknown Problem";
		break;
	}
}
