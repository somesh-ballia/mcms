//+========================================================================+
//                     BFCPH239Translator.h                                |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       BFCPH239Translator.h                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Lior Baram												   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 27/03/08   | This file contains definitions for the BFCP-H239	   |
//     |            | Translator module                                    |
//+========================================================================+
#if !defined(_BFCP_H239_SIP_TRANSLATOR_H)
#define _BFCP_H239_SIP_TRANSLATOR_H

#include "BFCPLibTypes.h"
#include "H323CsReq.h"
#include "H323CsInd.h"
#include "SipCsInd.h"
#include "SipCsReq.h"
#include "IpCsContentRoleToken.h"

#ifdef __cplusplus
extern "C" {
#endif
//This function initialize the module defaults
eStatus InitializeTranslatorDefaults	(BFCPH239Translator* pTranslator,
                                         UInt16              h239FloorID,
                                         eBFCPPriority       priority,
                                         UInt32			     confID,
                                         UInt16			     userID,
                                         eBFCPFloorCtrlType  clientServerStatus);

// Decode raw BFCP data to BFCPFloorInfoT
eStatus DecodeBFCPMsg	(BFCPH239Translator* pTranslator,
                         UInt8 *pInBinaryBFCPMsg,
                         UInt32 msgLen,
						 BFCPFloorInfoT *pOutBFCPMsg);

// Encode BFCPFloorInfoT to raw BFCP data - note that all data must be allocated outside,
// the maximum size of binary BFCP messages is BFCP_MAX_MSG_SIZE
eStatus EncodeBFCPMsg	(BFCPH239Translator	*pTranslator,
                         BFCPFloorInfoT 	* pInBFCPmsg,
                         UInt32 			*pMsgLen, /* pointer to the length of the new msg */
						 UInt8 				*pOutBinaryBFCPMsg,
						 char				*pRemoteIdent,
						 enTransportType	transportType,
						 BYTE 				bIsToSetIBit);

// convert BFCP struct to H239 structs
eStatus BFCPToH239Ind 	(BFCPH239Translator* pTranslator,
                         BFCPFloorInfoT *pBFCPmsg,
                         mcIndRoleToken	*pRoleTokenInd,
                         UInt16		    *pFloorRequestID,
                         ERoleTokenOpcode localWaitingForOpcode);

// convert BFCP struct to SIP structs
eStatus BFCPToSipInd 	(BFCPH239Translator* pTranslator,
                         BFCPFloorInfoT *pBFCPmsg,
                         APIS32 *pOpcode,
                         UInt16 *pFloorRequestID);

eStatus	SipReqToBFCP 	(
		BFCPFloorInfoT *pFloorInfo,
		/*BFCPH239Translator* pTranslator,*/
		OPCODE 			opcode,
		BFCPFloorInfoT	*pBFCPmsg,
		UInt16 			floorRequestID);

eStatus	BFCPToH239Req 	(BFCPH239Translator* pTranslator,
                         BFCPFloorInfoT *pBFCPmsg,
                         mcReqRoleTokenMessage	*pRoleTokenReq,
                         UInt16		    *pFloorRequestID,
                         ERoleTokenOpcode remoteWaitingForOpcode);


//convert H239 structs to BFCP struct
eStatus	H239IndToBFCP 	(BFCPH239Translator* pTranslator,
                         mcIndRoleToken * pRoleTokenInd,
						BFCPFloorInfoT	*pBFCPmsg,
						UInt16		    floorRequestID);

eStatus	H239ReqToBFCP 	(BFCPH239Translator* pTranslator,
                         mcReqRoleTokenMessage *pRoleTokenReq,
                         BFCPFloorInfoT *pBFCPmsg,
                         UInt16		    floorRequestID);

// functions to create specific BFCP messages note that all data must be allocated outside,
// the maximum size of binary BFCP messages is BFCP_MAX_MSG_SIZE
eStatus CreateBFCPHello (BFCPH239Translator* pTranslator, UInt8 *pBFCPHello, UInt32 *pMsgLen);

eStatus CreateBFCPHelloAck (BFCPH239Translator* pTranslator, UInt8 *pBFCPHelloAck, UInt32 *pMsgLen);

eStatus CreateFloorStatusFromQuery 	(BFCPH239Translator* pTranslator,
                                     BFCPFloorInfoT		*pBFCPQuery,
									eBFCPStatusValues	status,
									BFCPFloorInfoT		*pBFCPStatus);

eStatus CreateGenericRejectFromBFCPMsg (BFCPH239Translator* pTranslator,
                                        BFCPFloorInfoT *pInBFCPMsg,
                                        eBFCPErrors       errCode,
										BFCPFloorInfoT *pOutBFCPReject);

#ifdef __cplusplus
}
#endif	//	__cplusplus

#endif   /*  _BFCP_H239_SIP_TRANSLATOR_H */

