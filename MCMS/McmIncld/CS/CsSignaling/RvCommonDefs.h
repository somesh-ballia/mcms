//+========================================================================+
//                       RvCommonDefs.h	                                   |
//            Copyright 1995 Polycom Ltd.					               |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       RvCommonDefs.h	                                           |
// SUBSYSTEM:  CS/ConfParty									               |
// PROGRAMMER: Koren													   |
// Date : 15/5/05														   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+                        


//**************************************************
//***************************************************
//		This file is Rad vision definitions that the CS has in the stack and mcms should have them,
//		DO NOT CHANGE THIS FILE!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//***************************************************
//***************************************************
#ifndef __CS__
// this file should not be compiled by the CS code only declared in it for documentation


#ifndef __RV_COMMON_DEFS_H__
#define __RV_COMMON_DEFS_H__

#include "DataTypes.h"
#include "IpAddressDefinitions.h"
#include "IpCommonDefinitions.h"

//--------------------------------------------------------------------
//cmTransportType

typedef enum
{
  cmTransportTypeIP, /* No route */
  cmTransportTypeIPStrictRoute,
  cmTransportTypeIPLooseRoute,
  cmTransportTypeNSAP,
  cmTransportTypeIPv6
} cmTransportType;

//typedef enum
//{
//  eCmDistributionUnicast,
//  eCmDistributionMulticast
//} cmDistribution;

//--------------------------------------------------------------------
//cmAliasType

typedef enum
{
    cmAliasTypeE164=cmTransportTypeIPLooseRoute+5,
    cmAliasTypeH323ID,
    cmAliasTypeEndpointID,
    cmAliasTypeGatekeeperID,
    cmAliasTypeURLID,
    cmAliasTypeTransportAddress,
    cmAliasTypeEMailID,
    cmAliasTypePartyNumber,
	cmAliasTypeLast     
} cmAliasType;

//--------------------------------------------------------------------
//cmEndpointType 

#define cmRASEndpointType               cmEndpointType

typedef enum
{
    cmEndpointTypeTerminal,
    cmEndpointTypeGateway,
    cmEndpointTypeMCU,
    cmEndpointTypeGK,
    cmEndpointTypeUndefined,
    cmEndpointTypeSET
} cmEndpointType;

//--------------------------------------------------------------------
//cmCallType

#define cmRASCallType                   cmCallType

typedef enum
{
    cmCallTypeP2P,
    cmCallTypeOne2N,
    cmCallTypeN2One,
    cmCallTypeN2Nw
} cmCallType;

//--------------------------------------------------------------------
//cmCallModelType

#define cmRASCallModelType              cmCallModelType

typedef enum
{
    cmCallModelTypeDirect,
    cmCallModelTypeGKRouted
} cmCallModelType;

//---------------------------------------------------------------------
//cmRASReason

typedef enum
{
    cmRASReasonResourceUnavailable,             /* GRJ, RRJ, ARJ, LRJ - gatekeeper resources exhausted */
    cmRASReasonInsufficientResources,           /* BRJ */
    cmRASReasonInvalidRevision,                 /* GRJ, RRJ, BRJ */
    cmRASReasonInvalidCallSignalAddress,        /* RRJ */
    cmRASReasonInvalidRASAddress,               /* RRJ - supplied address is invalid */
    cmRASReasonInvalidTerminalType,             /* RRJ */
    cmRASReasonInvalidPermission,               /* ARJ - permission has expired */
                                                /* BRJ - true permission violation */
                                                /* LRJ - exclusion by administrator or feature */
    cmRASReasonInvalidConferenceID,             /* BRJ - possible revision */
    cmRASReasonInvalidEndpointID,               /* ARJ */
    cmRASReasonCallerNotRegistered,             /* ARJ */
    cmRASReasonCalledPartyNotRegistered,        /* ARJ - can't translate address */
    cmRASReasonDiscoveryRequired,               /* RRJ - registration permission has aged */
    cmRASReasonDuplicateAlias,                  /* RRJ - alias registered to another endpoint */
    cmRASReasonTransportNotSupported,           /* RRJ - one or more of the transports */
    cmRASReasonCallInProgress,                  /* URJ */
    cmRASReasonRouteCallToGatekeeper,           /* ARJ */
    cmRASReasonRequestToDropOther,              /* DRJ - can't request drop for others */
    cmRASReasonNotRegistered,                   /* DRJ, LRJ, INAK - not registered with gatekeeper */
    cmRASReasonUndefined,                       /* GRJ, RRJ, URJ, ARJ, BRJ, LRJ, INAK */
    cmRASReasonTerminalExcluded,                /* GRJ - permission failure, not a resource failure */
    cmRASReasonNotBound,                        /* BRJ - discovery permission has aged */
    cmRASReasonNotCurrentlyRegistered,          /* URJ */
    cmRASReasonRequestDenied,                   /* ARJ - no bandwidrg available */
                                                /* LRJ - cannot find location */
    cmRASReasonLocationNotFound,                /* LRJ - cannot find location */
    cmRASReasonSecurityDenial,                  /* GRJ, RRJ, URJ, ARJ, BRJ, LRJ, DRJ, INAK */
    cmRASReasonTransportQOSNotSupported,        /* RRJ */
    cmRASResourceUnavailable,                   /* Same as cmRASReasonResourceUnavailable */
    cmRASReasonInvalidAlias,                    /* RRJ - alias not consistent with gatekeeper rules */
    cmRASReasonPermissionDenied,                /* URJ - requesting user not allowed to unregister specified user */
    cmRASReasonQOSControlNotSupported,          /* ARJ */
    cmRASReasonIncompleteAddress,               /* ARJ, LRJ */
    cmRASReasonFullRegistrationRequired,        /* RRJ - registration permission has expired */
    cmRASReasonRouteCallToSCN,                  /* ARJ, LRJ */
    cmRASReasonAliasesInconsistent,             /* ARJ, LRJ - multiple aliases in request identify distinct people */
    cmRASReasonAdditiveRegistrationNotSupported,/* RRJ */
    cmRASReasonInvalidTerminalAliases,          /* RRJ */
    cmRASReasonExceedsCallCapacity,             /* ARJ - destination does not have the capacity for this call */
    cmRASReasonCollectDestination,              /* ARJ */
    cmRASReasonCollectPIN,                      /* ARJ */
    cmRASReasonGenericData,                     /* GRJ, RRJ, ARJ, LRJ */
    cmRASReasonNeededFeatureNotSupported,       /* GRJ, RRJ, ARJ, LRJ */
    cmRASReasonUnknownMessageResponse,          /* XRS message was received for the request */
    cmRASReasonHopCountExceeded                 /* LRJ */

} cmRASReason;


//-------------------------------------------------------------------------------
//cmRASDisengageReason enum: 
typedef enum
{
    cmRASDisengageReasonForcedDrop,
    cmRASDisengageReasonNormalDrop,
    cmRASDisengageReasonUndefinedReason
}  cmRASDisengageReason;

//-------------------------------------------------------------------------------
//cmReasonType enum:

typedef enum
{
    cmReasonTypeNoBandwidth,
    cmReasonTypeGatekeeperResource,
    cmReasonTypeUnreachableDestination,
    cmReasonTypeDestinationRejection,
    cmReasonTypeInvalidRevision,
    cmReasonTypeNoPermision,
    cmReasonTypeUnreachableGatekeeper,
    cmReasonTypeGatewayResource,
    cmReasonTypeBadFormatAddress,
    cmReasonTypeAdaptiveBusy,
    cmReasonTypeInConf,
    cmReasonTypeUndefinedReason,
    cmReasonTypeRouteCallToGatekeeper,
    cmReasonTypeCallForwarded,
    cmReasonTypeRouteCallToMC,
    cmReasonTypeFacilityCallDeflection,
    cmReasonTypeSecurityDenied,
    cmReasonTypeCalledPartyNotRegistered,
    cmReasonTypeCallerNotregistered,
    cmReasonTypeConferenceListChoice,
    cmReasonTypeStartH245,
    cmReasonTypeNewConnectionNeeded,
    cmReasonTypeNoH245,
    cmReasonTypeNewTokens,                  /* FacilityReason */
    cmReasonTypeFeatureSetUpdate,           /* FacilityReason */
    cmReasonTypeForwardedElements,          /* FacilityReason */
    cmReasonTypeTransportedInformation      /* FacilityReason */
} cmReasonType;


//-------------------------------------------------------------------------------
//cmConferenceGoalType enum:

typedef enum
{
    cmCreate,
    cmJoin,
    cmInvite,
    cmCapabilityNegotiation,
    cmCallIndependentSupplementaryService,
    cmLastCG
} cmConferenceGoalType;

//-------------------------------------------------------------------------------
//cmRASTransaction enum:

typedef enum
{
    cmRASGatekeeper=1,          /* GRQ transaction */
    cmRASRegistration,          /* RRQ transaction */
    cmRASUnregistration,        /* URQ transaction */
    cmRASAdmission,             /* ARQ transaction */
    cmRASDisengage,             /* DRQ transaction */
    cmRASBandwidth,             /* BRQ transaction */
    cmRASLocation,              /* LRQ transaction */
    cmRASInfo,                  /* IRQ-IRR transaction */
    cmRASNonStandard,           /* NSM */
    cmRASUnknown,               /* XRS */
    cmRASResourceAvailability,  /* RAI-RAC transaction */
    cmRASUnsolicitedIRR,        /* Unsolicited IRR */
    cmRASServiceControl,        /* SCI-SCR transaction */
    cmRASMaxTransaction
} cmRASTransaction;


typedef enum {
  cmMSSlave=1,
  cmMSMaster,
  cmMSError
} cmMSStatus;


//------------------------------------------------------------------------------
//cmNonStandard
typedef struct
{
    APIU8     t35CountryCode;
    APIU8     t35Extension;
    APIU16    manufacturerCode;
} cmNonStandard;

//cmVendor:
typedef struct
{
    cmNonStandard		info;
    APIU32				productLen;
    APIU32				versionLen;
    char				productID[256];
    char				versionID[256];
} cmVendor;


typedef struct   {
	partyAddressList			partyAddress;
	cmEndpointType				endpointType;						 
	mcTransportAddress			callSignalAddress;//changed by uri
	//cmRASAliasType				destEndpointType;
	
} mcTerminalParams;


typedef enum {
  cmCapEmpty=0,
  cmCapAudio=1,
  cmCapVideo=2,
  cmCapData=3,
  cmCapNonStandard=4,
  cmCapUserInput=5,
  cmCapConference=6,
  cmCapH235=7,
  cmCapMaxPendingReplacementFor=8,
  cmCapGeneric=9,
  cmCapMultiplexedStream=10,
  cmCapAudioTelephonyEvent=11,
  cmCapAudioTone=12,
  cmCapBfcp=13
} cmCapDataType;

///////////////////////////////////////////////////////////////////////



#endif //RV_COMMON_DEFS_H
#endif //__CS_USE__


