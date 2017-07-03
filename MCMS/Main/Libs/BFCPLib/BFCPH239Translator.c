//+========================================================================+
//                     BFCPH239Translator.c                                |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       BFCPH239Translator.c                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Lior Baram												   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 27/03/08   | This file contains implementation of the BFCP-H239   |
//     |            | Translator module                                    |
//+========================================================================+

#include "BFCPH239Internal.h"
#include "ConfPartyOpcodes.h"
#include "BFCPMessageNew.h"
//#include "Trace.h"

extern Boolean UpdateBFCPCommonHeaderPayloadLength(BFCPCommonHeader_T *pHeader, int length);
extern int BuildBFCPCommonHeader(BFCPCommonHeader_T *pHeader, BFCPFloorInfoT *pFloorInfo, eBFCPPrimitive eType, Boolean bIsToSetInitiatorBit);

////////////////////////////////////////////////////////////////////////////////
eStatus InitializeTranslatorDefaults	(BFCPH239Translator* pTranslator,
                                         UInt16              h239FloorID,
                                         eBFCPPriority       priority,
                                         UInt32			     confID,
                                         UInt16              userID,
                                         eBFCPFloorCtrlType  clientServerStatus)
{
    struct timespec tv;

    if (!pTranslator)
         return statusError;

    memset(pTranslator, 0, sizeof(BFCPH239Translator));

    pTranslator->validFlag = (UInt32)VALIDFLAG;

    BFCPInit(&(pTranslator->BFCPObj));

    pTranslator->BFCPObj.floorInfo.floorID  		= h239FloorID;
    pTranslator->BFCPObj.floorInfo.priority 		= priority;
    pTranslator->BFCPObj.floorInfo.floorRequestID 	= 1;

    //only client or server is supported
    if (kBFCPFloorCtrlServer == clientServerStatus ||
        kBFCPFloorCtrlClient == clientServerStatus)
        pTranslator->BFCPObj.floorCtrlType = clientServerStatus;
    else
        return statusError;

    BFCPSetConferenceID (&(pTranslator->BFCPObj), confID);
    BFCPSetUserID (&(pTranslator->BFCPObj), userID);
    //Set The handler function for the parser and the pLastDecodedMsg as the user info to keep the last decoded message
    BFCPSetMsgHandler(&(pTranslator->BFCPObj), &(pTranslator->lastDecodedMsg), (BFCPMsgEvent)(&ParserHandlerCallBack));

    //initialize the random generator
    srand ((int)pTranslator);

	return statusOK;
}

////////////////////////////////////////////////////////////////////////////////
eStatus DecodeBFCPMsg	(BFCPH239Translator *pTranslator,
                         UInt8 				*pInBinaryBFCPMsg,
                         UInt32 			msgLen,
						 BFCPFloorInfoT 	*pOutBFCPMsg)
{
	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
    	return statusError;

    //Setting the pointer for the return value
    BFCP_ParseMsgPrimitive(&(pTranslator->BFCPObj), pInBinaryBFCPMsg, msgLen);
    memcpy (pOutBFCPMsg, &(pTranslator->lastDecodedMsg), sizeof (BFCPFloorInfoT));
	return statusOK;
}

////////////////////////////////////////////////////////////////////////////////
eStatus EncodeBFCPMsg	(BFCPH239Translator	*pTranslator,
                         BFCPFloorInfoT 	*pInBFCPmsg,
                         UInt32 			*pMsgLen,
						 UInt8 				*pOutBinaryBFCPMsg,
						 char				*pRemoteIdent,
						 enTransportType	tarnsportType,
						 BYTE 				bIsServerResponse)
{
	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
        return statusError;

    eBFCPSignalType bfcpTransportType = (tarnsportType == eTransportTypeUdp) ? kBFCP_UDP : kBFCP_TCP;

    switch (pInBFCPmsg->transactionType)
    {
        //build according to opcode
        case kBFCPFloorRequest:
            BFCP_BuildFloorRequest (
            		pInBFCPmsg, pOutBinaryBFCPMsg, pMsgLen);
            break;

        case kBFCPFloorRequestStatus:
            BFCP_BuildFloorRequestStatus (
            		pInBFCPmsg, pInBFCPmsg->floorStatus, pOutBinaryBFCPMsg, pMsgLen, bfcpTransportType, bIsServerResponse);
            break;

        case kBFCPFloorRelease:
            BFCP_BuildFloorRelease (
            		pInBFCPmsg, pOutBinaryBFCPMsg, pMsgLen);
            break;

        case kBFCPFloorStatus:
            BFCP_BuildFloorStatus(
            		pInBFCPmsg, pInBFCPmsg->floorStatus, pOutBinaryBFCPMsg, pMsgLen, bfcpTransportType, bIsServerResponse);
            break;

        case kBFCPFloorStatusAck:
			BFCP_BuildFloorStatusAck(
					pInBFCPmsg, pInBFCPmsg->floorStatus, pOutBinaryBFCPMsg, pMsgLen);
			break;

        case kBFCPHello:
        	BFCP_BuildHello(pInBFCPmsg, pOutBinaryBFCPMsg, pMsgLen);
        	break;

        case kBFCPHelloAck:
        	BFCP_BuildHelloAck(pInBFCPmsg, pOutBinaryBFCPMsg, pMsgLen, pRemoteIdent, bfcpTransportType);
        	break;

        case kBFCPGoodbye:
			BFCP_BuildGoodbye(pInBFCPmsg, pOutBinaryBFCPMsg, pMsgLen);
			break;

        case kBFCPGoodbyeAck:
			BFCP_BuildGoodbyeAck(pInBFCPmsg, pOutBinaryBFCPMsg, pMsgLen, pRemoteIdent, bfcpTransportType);
			break;

        default:
            return statusError;
    }

	return statusOK;
}

////////////////////////////////////////////////////////////////////////////////
eStatus BFCPToSipInd 	(BFCPH239Translator *pTranslator,
                         BFCPFloorInfoT 	*pBFCPmsg,
                         APIS32 			*pOpcode,
                         UInt16 			*pFloorRequestID)
{

	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
        return statusError;
    //if ( pBFCPmsg->conferenceID != GetConfID(pTranslator) ||
    //	 pBFCPmsg->userID != GetUserID(pTranslator) )
    //   return statusError;
    eStatus retVal = statusOK;

    switch (pBFCPmsg->transactionType)
    {
        case kBFCPFloorRequest:
        {
            *pOpcode = (APIU32)PARTY_TOKEN_ACQUIRE;
            retVal = statusOK;
            //Set the current RequestID in the translator object
            break;
        }
        case kBFCPFloorRequestStatus:
        {
        	retVal = statusOK;

        	if (pBFCPmsg->floorStatus == kBFCPStatusGranted)
                *pOpcode = (APIU32)CONTENT_ROLE_TOKEN_ACQUIRE_ACK;
			else if (pBFCPmsg->floorStatus == kBFCPStatusDenied)
					*pOpcode = (APIU32)CONTENT_ROLE_TOKEN_ACQUIRE_NAK;
			else if (pBFCPmsg->floorStatus == kBFCPStatusPending)
					*pOpcode = (APIU32)CONTENT_ROLE_TOKEN_PENDING;
			else if (pBFCPmsg->floorStatus == kBFCPStatusRevoked)
					*pOpcode = (APIU32)CONTENT_ROLE_TOKEN_WITHDRAW;
			else if (pBFCPmsg->floorStatus == kBFCPStatusReleased)
					*pOpcode = (APIU32)CONTENT_ROLE_TOKEN_RELEASE_ACK;
				else
			{
					retVal = statusCanNotConvert;
			}
        	break;
        }
        case kBFCPFloorRelease:
        {
            *pOpcode 	= PARTY_TOKEN_RELEASE;
            retVal 		= statusOK;
            break;
        }
        case kBFCPFloorQuery:
        {
        	*pOpcode 	= PARTY_BFCP_TOKEN_QUERY;
        	retVal 		= statusOK;
        	break;
        }
        case kBFCPHello:
        {
            *pOpcode 	= CONTENT_ROLE_BFCP_HELLO;
            retVal 		= statusBFCPHello;
            break;
        }
        case kBFCPHelloAck:
        {
            *pOpcode 	= CONTENT_ROLE_BFCP_HELLO_ACK;
            retVal 		= statusBFCPHelloAck;
            break;
        }
        case kBFCPError:
        {
        	*pOpcode 	= CONTENT_ROLE_BFCP_ERROR;
        	retVal 		= statusOK;
        	break;
        }
        case kBFCPFloorRequestStatusAck:
        case kTandbergBFCPFloorRequestStatusAck:
		{
			*pOpcode 	= CONTENT_ROLE_BFCP_FLOOR_REQ_STATUS_ACK;
			retVal 		= statusOK;
			break;
		}
        case kBFCPErrorAck:
		{
			*pOpcode 	= CONTENT_ROLE_BFCP_ERROR_ACK;
			retVal 		= statusOK;
			break;
		}
        case kBFCPFloorStatusAck:
        case kTandbergBFCPFloorStatusAck:
		{
			*pOpcode 	= CONTENT_ROLE_BFCP_FLOOR_STATUS_ACK;
			retVal 		= statusOK;
			break;
		}
        case kBFCPGoodbye:
		{
			*pOpcode 	= CONTENT_ROLE_BFCP_GOODBYE;
			retVal 		= statusOK;
			break;
		}
        case kBFCPGoodbyeAck:
		{
			*pOpcode 	= CONTENT_ROLE_BFCP_GOODBYE_ACK;
			retVal 		= statusOK;
			break;
		}

        default:
        {
            retVal = statusError;
            break;
        }
    }

    *pFloorRequestID = pBFCPmsg->floorRequestID;

    return retVal;
 }
////////////////////////////////////////////////////////////////////////////////
eStatus BFCPToH239Ind 	(BFCPH239Translator* pTranslator,
                         BFCPFloorInfoT *pBFCPmsg,
                         mcIndRoleToken	*pRoleTokenInd,
                         UInt16		    *pFloorRequestID,
                         ERoleTokenOpcode localWaitingForOpcode)
{
	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
        return statusError;

    eStatus retVal = statusOK;

    switch (pBFCPmsg->transactionType)
    {
        case kBFCPFloorRequest:
        {
            pRoleTokenInd->subOpcode = (APIU32)kPresentationTokenRequest;
            pRoleTokenInd->bIsAck = FALSE;
            retVal = statusOK;
            //Set the current RequestID in the translator object

            break;
        }
        case kBFCPFloorRequestStatus:
        {
            if (localWaitingForOpcode == kPresentationTokenResponse)
            {
                /* The application is waiting for a response for token request */
                if (kBFCPStatusGranted == pBFCPmsg->floorStatus &&
                    pBFCPmsg->conferenceID == GetConfID(pTranslator) &&
                    pBFCPmsg->userID == GetUserID(pTranslator) )
                {
                    pRoleTokenInd->subOpcode = kPresentationTokenResponse;
                    pRoleTokenInd->bIsAck = TRUE;
                    retVal = statusOK;

                }
                else if (kBFCPStatusDenied == pBFCPmsg->floorStatus)
                {
                    pRoleTokenInd->subOpcode = kPresentationTokenResponse;
                    pRoleTokenInd->bIsAck = FALSE;
                    retVal = statusOK;
                }
                else
                   retVal = statusCanNotConvert;

            }
            else if (kBFCPStatusGranted == pBFCPmsg->floorStatus || kBFCPStatusAccepted == pBFCPmsg->floorStatus)
            {
                    /*The Application is not waiting for anything, so this is surely someone else who got the token*/
                    pRoleTokenInd->subOpcode = kPresentationTokenIndicateOwner;
                    pRoleTokenInd->bIsAck = FALSE;
                    retVal = statusOK;
            }
            else if (kBFCPStatusRevoked == pBFCPmsg->floorStatus &&
                     pBFCPmsg->conferenceID == GetConfID(pTranslator) &&
                     pBFCPmsg->userID == GetUserID(pTranslator))
            {
                //Floor status revoke means that the server is withdrawing the token
                pRoleTokenInd->subOpcode = kPresentationTokenRequest;
                pRoleTokenInd->bIsAck = FALSE;
                pRoleTokenInd->randNumber = 0;
                retVal = statusOK;
            }

            else
                retVal = statusCanNotConvert;

            break;

        }
        case kBFCPFloorRelease:
        {
            pRoleTokenInd->subOpcode = kPresentationTokenRelease;
            pRoleTokenInd->bIsAck = FALSE;
            retVal = statusOK;
            break;
        }

        case kBFCPFloorRequestQuery:
        case kBFCPFloorQuery:
        case kBFCPUserQuery:
            retVal = statusBFCPMsgIsQuery;
            break;

        case kBFCPHello:
            retVal = statusBFCPHello;
            break;

        case kBFCPHelloAck:
            retVal = statusBFCPHelloAck;
            break;

         default:
            retVal = statusCanNotConvert;
            break;
    }

    if (statusOK == retVal)
    {
        //General fields for all the messages
        pRoleTokenInd->mcuID = pBFCPmsg->conferenceID;
        pRoleTokenInd->terminalID = pBFCPmsg->userID;
        pRoleTokenInd->randNumber = symmetryBreakingRand(); //if the request is from an MCU, should be replaced later
        pRoleTokenInd->contentProviderInfo = 0;
        pRoleTokenInd->label = (APIU32)kRolePresentation;
        pRoleTokenInd->bitRate = 0;
        pRoleTokenInd->filler = 0;
    }

    *pFloorRequestID = pBFCPmsg->floorRequestID;

	return retVal;
}

////////////////////////////////////////////////////////////////////////////////
eStatus	BFCPToH239Req 	(BFCPH239Translator* pTranslator,
                         BFCPFloorInfoT *pBFCPmsg,
                         mcReqRoleTokenMessage	*pRoleTokenReq,
                         UInt16		    *pFloorRequestID,
                         ERoleTokenOpcode remoteWaitingForOpcode)
{
	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
        return statusError;

    /*note that mcReqRoleTokenMessage and mcIndRoleToken  must be the same!
      And if they are the same then the parsing is the same...*/
    return BFCPToH239Ind (pTranslator, pBFCPmsg, (mcIndRoleToken*)pRoleTokenReq, pFloorRequestID, remoteWaitingForOpcode);
}
////////////////////////////////////////////////////////////////////////////////

eStatus	H239IndToBFCP 	(BFCPH239Translator* pTranslator,
                         mcIndRoleToken * pRoleTokenInd,
                         BFCPFloorInfoT	*pBFCPmsg,
                         UInt16		    floorRequestID)
{
	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
        return statusError;
    /*note that mcReqRoleTokenMessage and mcIndRoleToken  must be the same!
      And if they are the same then the parsing is the same...*/
    return H239ReqToBFCP (pTranslator, (mcReqRoleTokenMessage*)pRoleTokenInd, pBFCPmsg, floorRequestID);
}
////////////////////////////////////////////////////////////////////////////////
eStatus	SipReqToBFCP 	(
		BFCPFloorInfoT		*pFloorInfo,
		/*BFCPH239Translator	*pTranslator,*/
        OPCODE 				opcode,
        BFCPFloorInfoT		*pBFCPmsg,
        UInt16 				floorRequestID)
{
//    if (!TestValidity(pTranslator))
//        return statusError;

    eStatus retVal = statusOK;

    if( opcode == PARTY_TOKEN_ACQUIRE )
    {
    	pBFCPmsg->transactionType 	= kBFCPFloorRequest;
    }
    else if( opcode == CONTENT_ROLE_TOKEN_WITHDRAW )
    {
    	pBFCPmsg->transactionType 	= kBFCPFloorRequestStatus;
    	pBFCPmsg->floorStatus 		= kBFCPStatusRevoked;
    }
    else if( opcode == CONTENT_ROLE_TOKEN_ACQUIRE_ACK )
    {
    	pBFCPmsg->transactionType 	= kBFCPFloorRequestStatus;
		pBFCPmsg->floorStatus 		= kBFCPStatusGranted;
    }
    else if( opcode == CONTENT_ROLE_TOKEN_ACQUIRE_NAK )
    {
    	pBFCPmsg->transactionType 	= kBFCPFloorRequestStatus;
    	pBFCPmsg->floorStatus 		= kBFCPStatusDenied;
    }
    else if( opcode == PARTY_TOKEN_RELEASE )
    {
    	pBFCPmsg->transactionType 	= kBFCPFloorRelease;
    }
    else if( opcode == CONTENT_ROLE_TOKEN_RELEASE_ACK )
    {
    	pBFCPmsg->transactionType 	= kBFCPFloorRequestStatus;
    	pBFCPmsg->floorStatus 		= kBFCPStatusReleased;
    }
    else if( opcode == CONTENT_ROLE_PROVIDER_IDENTITY )
    {
    	pBFCPmsg->transactionType 	= kBFCPFloorStatus;
		pBFCPmsg->floorStatus 		= kBFCPStatusGranted;
    }
    else if( opcode == CONTENT_NO_ROLE_PROVIDER )
    {
    	pBFCPmsg->transactionType 	= kBFCPFloorStatus;
    	pBFCPmsg->floorStatus 		= kBFCPStatusReleased;
    }
    else if( opcode == CONTENT_ROLE_BFCP_HELLO )
	{
    	pBFCPmsg->transactionType 	= kBFCPHello;
	}
    else if( opcode == CONTENT_ROLE_BFCP_HELLO_ACK )
	{
		pBFCPmsg->transactionType 	= kBFCPHelloAck;
	}
    else if( opcode == CONTENT_ROLE_BFCP_FLOOR_REQ_STATUS_ACK )
	{
		pBFCPmsg->transactionType 	= kBFCPFloorRequestStatusAck;
	}
    else if( opcode == CONTENT_ROLE_BFCP_ERROR_ACK )
	{
		pBFCPmsg->transactionType 	= kBFCPErrorAck;
	}
    else if( opcode == CONTENT_ROLE_BFCP_FLOOR_STATUS_ACK )
	{
		pBFCPmsg->transactionType 	= kBFCPFloorStatusAck;
	}
    else if( opcode == CONTENT_ROLE_BFCP_GOODBYE )
	{
		pBFCPmsg->transactionType 	= kBFCPGoodbye;
	}
    else if( opcode == CONTENT_ROLE_BFCP_GOODBYE_ACK )
	{
		pBFCPmsg->transactionType 	= kBFCPGoodbyeAck;
	}

    if (statusOK == retVal)
    {
        //General fields for all the messages
        pBFCPmsg->floorID 			= pFloorInfo->floorID;//pTranslator->BFCPObj.floorInfo.floorID;
        pBFCPmsg->conferenceID 		= pFloorInfo->conferenceID;//GetConfID(pTranslator);
        pBFCPmsg->userID 			= pFloorInfo->userID;//GetUserID(pTranslator);
        pBFCPmsg->transactionID 	= pFloorInfo->transactionID;//pTranslator->BFCPObj.floorInfo.transactionID;
        pBFCPmsg->floorRequestID 	= floorRequestID;
        pBFCPmsg->priority 			= pFloorInfo->priority;//pTranslator->BFCPObj.floorInfo.priority;
    }

    return retVal;
}
////////////////////////////////////////////////////////////////////////////////
eStatus	H239ReqToBFCP 	(BFCPH239Translator* pTranslator,
                         mcReqRoleTokenMessage *pRoleTokenReq,
                         BFCPFloorInfoT	*pBFCPmsg,
                         UInt16		    floorRequestID)
{
	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
        return statusError;

    eStatus retVal = statusOK;

    switch (pRoleTokenReq->subOpcode)
    {
        case kPresentationTokenRequest:
        {
            if (pRoleTokenReq->randNumber) /*This is a regular request, not from a master MCU*/
                pBFCPmsg->transactionType = kBFCPFloorRequest;
            else /* A request from a master MCU */
            {
                pBFCPmsg->transactionType = kBFCPFloorRequestStatus;
                pBFCPmsg->floorStatus = kBFCPStatusRevoked;
            }
            break;
        }

        case kPresentationTokenResponse:
        {
            if (kBFCPFloorCtrlClient == pTranslator->BFCPObj.floorCtrlType)
            {
                pBFCPmsg->transactionType = kBFCPFloorRelease;
            }
            else if (kBFCPFloorCtrlServer == pTranslator->BFCPObj.floorCtrlType)
            {
                //This is a response to the client's request
                pBFCPmsg->transactionType = kBFCPFloorRequestStatus;
                pBFCPmsg->floorStatus = kBFCPStatusGranted;
            }
            break;

        }
        case kPresentationTokenIndicateOwner:
        {
            pBFCPmsg->transactionType = kBFCPFloorRequestStatus;
            pBFCPmsg->floorStatus = kBFCPStatusGranted;
            break;
        }
        case kPresentationTokenRelease:
        {
            pBFCPmsg->transactionType = kBFCPFloorRelease;
            break;

        }
        default:
            retVal = statusCanNotConvert;
            break;
    }

    if (statusOK == retVal)
    {
        //General fields for all the messages
        pBFCPmsg->floorID = pTranslator->BFCPObj.floorInfo.floorID;
        pBFCPmsg->conferenceID = pRoleTokenReq->mcuID;
        pBFCPmsg->userID = pRoleTokenReq->terminalID;
        pBFCPmsg->transactionID = pTranslator->BFCPObj.floorInfo.transactionID;
        pBFCPmsg->floorRequestID = floorRequestID;
        pBFCPmsg->priority = pTranslator->BFCPObj.floorInfo.priority;
    }

	return retVal;
}
////////////////////////////////////////////////////////////////////////////////
eStatus CreateBFCPHello (BFCPH239Translator* pTranslator, UInt8 *pBFCPHello, UInt32 *pMsgLen)
{
	if (!pTranslator)
		return statusError;

	if (!TestValidity(pTranslator))
		return statusError;

    *pMsgLen = 0;
    BFCPFloorInfoT *pFloorInfo = &(pTranslator->BFCPObj.floorInfo);

    if (pFloorInfo)
    {
        UInt8 * pHeader;
        unsigned int payloadLen = 0;
        //*pBFCPHello = pBFCPMsg;

        //    Initialize the message components.
        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pBFCPHello, pFloorInfo, kBFCPHello, FALSE);
        if (*pMsgLen > 0)
        {
            //    Increment the transaction ID.
            pFloorInfo->transactionID++;
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pBFCPHello = NULL;
                return statusError;
            }
        }
    }

	return statusOK;
}
////////////////////////////////////////////////////////////////////////////////
eStatus CreateBFCPHelloAck (BFCPH239Translator* pTranslator, UInt8 *pBFCPHelloAck, UInt32 *pMsgLen)
{
	if (!pTranslator)
		return statusError;

	if (!TestValidity(pTranslator))
		return statusError;

    *pMsgLen = 0;
    BFCPFloorInfoT *pFloorInfo = &(pTranslator->BFCPObj.floorInfo);

    if (pFloorInfo)
    {
        UInt8 * pHeader;
        unsigned int payloadLen = 0;
        //*pBFCPHello = pBFCPMsg;

        //    Initialize the message components.
        *pMsgLen = BuildBFCPCommonHeader((BFCPCommonHeader_T *)pBFCPHelloAck, pFloorInfo, kBFCPHelloAck, FALSE);
        if (*pMsgLen > 0)
        {
            //    Increment the transaction ID.
            pFloorInfo->transactionID++;
            pHeader = pBFCPHelloAck + *pMsgLen;
            if ((*pMsgLen+4) > BFCP_MAX_MSG_SIZE)
            {
                printf("Warning: BFCP Message too long\n");
                pBFCPHelloAck = NULL;
                return statusError;
            }

            payloadLen = BuildBFCPSupportedPrimitives (pHeader, 1);
            //payloadLen += BuildBFCPSupportedAttributes (pHeader, 1);
            payloadLen += BuildBFCPSupportedAttributes (pHeader + payloadLen, 1);

            *pMsgLen += payloadLen;
            UpdateBFCPCommonHeaderPayloadLength((BFCPCommonHeader_T *)pBFCPHelloAck, payloadLen/4);
        }
    }

	return statusOK;
}
////////////////////////////////////////////////////////////////////////////////
eStatus CreateFloorStatusFromQuery 	(BFCPH239Translator* pTranslator,
                                     BFCPFloorInfoT		*pBFCPQuery,
                                     eBFCPStatusValues	status,
                                     BFCPFloorInfoT		*pBFCPStatus)
{
	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
        return statusError;

    eStatus retVal = statusOK;
    memcpy (pBFCPStatus,  pBFCPQuery, sizeof(BFCPFloorInfoT));
    switch (pBFCPQuery->transactionType)
    {
        case kBFCPFloorRequestQuery:
            pBFCPStatus->transactionType = kBFCPFloorRequestStatus;
            break;

        case kBFCPUserQuery:
            pBFCPStatus->transactionType = kBFCPUserStatus;
            break;

        case kBFCPFloorQuery:
            pBFCPStatus->transactionType = kBFCPFloorStatus;
            break;

        default:
            retVal = statusError;
            break;
    }
    pBFCPStatus->floorStatus = status;

	return retVal;
}
////////////////////////////////////////////////////////////////////////////////
eStatus CreateGenericRejectFromBFCPMsg (BFCPH239Translator* pTranslator,
                                        BFCPFloorInfoT *pInBFCPMsg,
                                        eBFCPErrors       errCode,
										BFCPFloorInfoT *pOutBFCPReject)
{
	if (!pTranslator)
		return statusError;

    if (!TestValidity(pTranslator))
        return statusError;

    eStatus retVal = statusOK;
    memcpy (pOutBFCPReject,  pInBFCPMsg, sizeof (BFCPFloorInfoT));

    if (pInBFCPMsg->transactionType == kBFCPFloorRequest)
    {
        pOutBFCPReject->transactionType = kBFCPFloorStatus;
        pOutBFCPReject->floorStatus = kBFCPStatusDenied;
    }
    else
        retVal = statusCanNotConvert;

	return retVal;
}


////////////////////////////////////////////////////////////////////////////////
// For Internal Use only
////////////////////////////////////////////////////////////////////////////////
void ParserHandlerCallBack (void *pLastDecodedMsg, BFCPFloorInfoT *pFloorInfo, eBFCPPrimitive primitive)
{
    pFloorInfo->transactionType = primitive;
    memcpy (pLastDecodedMsg, pFloorInfo, sizeof(BFCPFloorInfoT));
}

////////////////////////////////////////////////////////////////////////////////
BOOL TestValidity (BFCPH239Translator* pTranslator)
{
    return (pTranslator->validFlag == (UInt32)VALIDFLAG);
}
////////////////////////////////////////////////////////////////////////////////
int symmetryBreakingRand ()
{
    //between 1 and 127 - 0 is special for mcu use only
    return  ((rand()*(SYMMETRY_BREAKING_MAX-1))/ RAND_MAX) +1;
}

////////////////////////////////////////////////////////////////////////////////
void BFCPSetConferenceID(BFCPObject_T *pBFCPObject, int ConfID)
{
    pBFCPObject->floorInfo.conferenceID = ConfID;
}
////////////////////////////////////////////////////////////////////////////////
UInt32 GetConfID (BFCPH239Translator* pTranslator)
{
    return pTranslator->BFCPObj.floorInfo.conferenceID;

}
////////////////////////////////////////////////////////////////////////////////
void BFCPSetUserID(BFCPObject_T *pBFCPObject, int UserID)
{
    pBFCPObject->floorInfo.userID = UserID;
}
////////////////////////////////////////////////////////////////////////////////
UInt32 GetUserID (BFCPH239Translator* pTranslator)
{
    return pTranslator->BFCPObj.floorInfo.userID;

}
////////////////////////////////////////////////////////////////////////////////
