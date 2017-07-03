//+========================================================================+
//                     BFCPH239Internal.h                                   |
//            Copyright 1995 POLYCOM Technologies Ltd.                      |
//                   All Rights Reserved.                                   |
//--------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary      |
// information of POLYCOM Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed     |
// to third parties, reverse engineered or used in any manner without       |
// prior written authorization from POLYCOM Technologies Ltd.               |
//--------------------------------------------------------------------------|
// FILE:       BFCPH239Internal.h                                           |
// SUBSYSTEM:  MCMS                                                         |
// PROGRAMMER: Lior Baram												    |
//--------------------------------------------------------------------------|
// Who | Date       | Description                                           |
//--------------------------------------------------------------------------|
//     | 27/03/08   | This file contains definitions for the BFCP-H239	    |
//     |            | Translator module, the definitions in this file are   |
//     |            | internal and should not be exposed to the using module|
//+=========================================================================+
#if !defined(_BFCP_H239_SIP_INTERNAL_H)
#define _BFCP_H239_SIP_INTERNAL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "SharedDefines.h"
#include "BFCPH239Translator.h"

#define VALIDFLAG 0x5A5AF1F1	// as CPObject's validation flag
#define SYMMETRY_BREAKING_MAX 127

#define NUM_OF_SUPPORTED_BFCP_ATTR 15
//This global table contain all the supported BFCP attributes- to be used in Hello Ack
static eBFCPAttributeID gSupportedBFCPAttributes [NUM_OF_SUPPORTED_BFCP_ATTR] =
{
	kBFCPAttrFloorID,
	kBFCPAttrFloorRequestID,
	kBFCPAttrPriority,
	kBFCPAttrRequestStatus,
	kBFCPAttrErrorCode,
	kBFCPAttrErrorInfo,
	kBFCPAttrStatusInfo,
	kBFCPAttrSupportedAttrs,
	kBFCPAttrSupportedPrimitives,
	kBFCPAttrUserDisplayName,
	kBFCPAttrUserURI,
	kBFCPAttrFloorRequestInfo,
	kBFCPAttrRequestedByInfo,
	kBFCPAttrFloorRequestStatus,
	kBFCPAttrOverallRequestStatus
};

//This global table contain all the supported BFCP Primitives- to be used in Hello Ack
#define NUM_OF_SUPPORTED_BFCP_PRIMITIVES 18
static  eBFCPPrimitive gSupportedBFCPPrimitives [NUM_OF_SUPPORTED_BFCP_PRIMITIVES] =
{
    kBFCPFloorRequest,			// P -> S
	kBFCPFloorRelease,			// P -> S
	kBFCPFloorRequestQuery,		// P -> S; Ch -> S
	kBFCPFloorRequestStatus,	// P <- S; Ch <- S
	kBFCPUserQuery,				// P -> S; Ch -> S
	kBFCPUserStatus,			// P <- S; Ch <- S
	kBFCPFloorQuery,			// P -> S; Ch -> S
	kBFCPFloorStatus,			// P <- S; Ch <- S
	kBFCPChairAction,			// Ch -> S
	kBFCPChairActionAck,		// Ch <- S
	kBFCPHello,					// P -> S; Ch -> S
	kBFCPHelloAck,				// P <- S; Ch <- S
	kBFCPError,					// P <- S; Ch <- S
	kBFCPFloorRequestStatusAck, // P -> S ; Ch -> S
	kBFCPErrorAck,				// P -> S ; Ch -> S
	kBFCPFloorStatusAck,		// P -> S ; Ch -> S
	kBFCPGoodbye,				// P -> S ; Ch -> S ; S -> P ; S -> Ch
	kBFCPGoodbyeAck,			// P -> S ; Ch -> S ; S -> P ; S -> Ch

	//kTandbergBFCPFloorRequestStatusAck,
	//kTandbergBFCPFloorStatusAck

};

void ParserHandlerCallBack (void *pUserDefined, BFCPFloorInfoT *pFloorInfo, eBFCPPrimitive primitive);
BOOL TestValidity (BFCPH239Translator* pTranslator);
int symmetryBreakingRand ();
UInt32 GetConfID (BFCPH239Translator* pTranslator);
UInt32 GetUserID (BFCPH239Translator* pTranslator);

#endif
