//+========================================================================+
//                     BFCPMessageNew.c                                    |
//            Copyright 1995 POLYCOM Technologies Ltd.                     |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of POLYCOM Technologies Ltd. and is protected by law.       |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from POLYCOM Technologies Ltd.              |
//-------------------------------------------------------------------------|
// FILE:       BFCPMessageNew.c                                            |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Lior Baram												   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 01/04/08   | This file is an extension to the BFCPMessage file    |
//     |            | with functions to complete the BFCP implementation   |
//+========================================================================+

#include "BFCPMessageNew.h"
#include "BFCPH239Internal.h"

extern UINT8 bytefieldSet(UINT8 value, int bitfield, int nStartBit, int nBits);

//---------------------------------------------------------
// BuildBFCPSupportedAttributes
//
// Build BFCP SUPPORTED-Attributes element
//---------------------------------------------------------

int BuildBFCPSupportedAttributes(void *pHeader, int Mandatory)
{
	/*
    int length = NUM_OF_SUPPORTED_BFCP_ATTR;
    eBFCPAttributeID eType = kBFCPAttrSupportedAttrs;
    UINT8 *header = (UINT8*)( (char*)pHeader) ;
    int padLength = length % 4;
    int i=0;
    if (pHeader && eType > kBFCPAttrInvalid && eType < kBFCPAttrMaxAttribute)
    {
        header[0] = bytefieldSet(0,eType,1,7);          // AttrType: 7 bit
        header[0] = bytefieldSet(header[0],Mandatory,0,1);      // Mandatory:1 bit
        header[1] = bytefieldSet(0,length, 0, 8);       // Length: 1 byte

        //code attributes starting byte #3 (index 2)
        for (i=2; i < length + 2; i++)
            //The rightmost bit is currently reserved and must be set to zero
            header[i] = (gSupportedBFCPAttributes[i-2] << 1) & 0xff;

        //Add padding to 32 bit
        if (padLength)
        {
            for (i = length; i< length + padLength; i++)
                header[i] = 0;
        }
    }

    return length + padLength;
	*/

    int length = NUM_OF_SUPPORTED_BFCP_ATTR;
    eBFCPAttributeID eType = kBFCPAttrSupportedAttrs;
    UINT8 *header = (UINT8*)( (char*)pHeader) ;
    int padLength = (4-((length-2) % 4)) % 4;
    int i=0;
    if (pHeader && eType > kBFCPAttrInvalid && eType < kBFCPAttrMaxAttribute)
    {
        header[0] = bytefieldSet(0,eType,1,7);          // AttrType: 7 bit
        header[0] = bytefieldSet(header[0],Mandatory,0,1);      // Mandatory:1 bit
        header[1] = bytefieldSet(0,length + padLength + 2, 0, 8);       // Length: 1 byte

        //code attributes starting byte #3 (index 2)
        for (i=2; i < length + 2; i++)
            //The rightmost bit is currently reserved and must be set to zero
            header[i] = (gSupportedBFCPAttributes[i-2] << 1) & 0xff;

        //Add padding to 32 bit
        if (padLength)
        {
            for (i = length + 2; i< length + padLength + 2; i++)
                header[i] = 0;
        }
    }

    return length + padLength + 2;
}

/*---------------------------------------------------------
// BuildBFCPSupportedAttributes
//
// Build BFCP SUPPORTED-PRIMITIVES element
//---------------------------------------------------------*/
int BuildBFCPSupportedPrimitives (void *pHeader, int Mandatory)
{
/*
	int length = NUM_OF_SUPPORTED_BFCP_PRIMITIVES;
    eBFCPAttributeID eType = kBFCPAttrSupportedPrimitives;
    UINT8 *header = (UINT8*)( (char*)pHeader) ;
    int padLength = length % 4;
    int i=0;
    if (pHeader && eType > kBFCPAttrInvalid && eType < kBFCPAttrMaxAttribute)
    {
        header[0] = bytefieldSet(0,eType,1,7);          // AttrType: 7 bit
        header[0] = bytefieldSet(header[0],Mandatory,0,1);      // Mandatory:1 bit
        header[1] = bytefieldSet(0,length, 0, 8);       // Length: 1 byte

        //code Primitives starting byte #3 (index 2)
        for (i=2; i < length + 2; i++)
            header[i] = gSupportedBFCPPrimitives[i-2] & 0xff;

        //Add padding to 32 bit

        if (padLength)
        {
            for (i = length; i< length + padLength; i++)
                header[i] = 0;
        }
    }

    return length + padLength;
*/

    int length = NUM_OF_SUPPORTED_BFCP_PRIMITIVES;
    eBFCPAttributeID eType = kBFCPAttrSupportedPrimitives;
    UINT8 *header = (UINT8*)( (char*)pHeader) ;
    int padLength = (4-((length-2) % 4)) % 4 ;
    int i=0;
    if (pHeader && eType > kBFCPAttrInvalid && eType < kBFCPAttrMaxAttribute)
    {
        header[0] = bytefieldSet(0,eType,1,7);          // AttrType: 7 bit
        header[0] = bytefieldSet(header[0],Mandatory,0,1);      // Mandatory:1 bit
        header[1] = bytefieldSet(0,length + padLength + 2, 0, 8);       // Length: 1 byte

        //code Primitives starting byte #3 (index 2)
        for (i=2; i < length + 2; i++)
            header[i] = gSupportedBFCPPrimitives[i-2] & 0xff;

        //Add padding to 32 bit
        
        if (padLength)
        {
            for (i = length + 2; i < length + padLength + 2; i++)
                header[i] = 0;
        }
    }

    return length + padLength + 2;
}

void DumpBFCPMsgToString ( BFCPFloorInfoT * pBFCPmsg, char * outStr)
{

    sprintf (outStr, "BFCP Message Dump\n transactionType %d\n  floorID %d\n conferenceID %d\n userID %d\n transactionID %d\n floorRequestID %d\n floorStatus %d\n priority %d\n",
             pBFCPmsg->transactionType,pBFCPmsg->floorID, pBFCPmsg->conferenceID, pBFCPmsg->userID, pBFCPmsg->transactionID, pBFCPmsg->floorRequestID, pBFCPmsg->floorStatus, pBFCPmsg->priority);
    
}


BYTE isBFCPMsgsEqual(BFCPFloorInfoT * pFirst, BFCPFloorInfoT * pSecond)
{
    //For all messages
    if (pFirst->transactionType != pSecond->transactionType)
        return FALSE;
    if (pFirst->conferenceID != pSecond->conferenceID)
        return FALSE;
    if (pFirst->userID != pSecond->userID)
        return FALSE;
    //if (pFirst->transactionID != pSecond->transactionID)
    //    return FALSE;

    //For relevant messages only
    switch (pFirst->transactionType)
    {
        case kBFCPFloorStatus:
        case kBFCPFloorRequestStatus:
        case kBFCPUserStatus:
        {
            if (pFirst->floorRequestID != pSecond->floorRequestID)
                return FALSE;
            if (pFirst->floorStatus != pSecond->floorStatus)
                return FALSE;
            if (pFirst->floorID != pSecond->floorID)
                return FALSE;
            break;
        }
        case kBFCPFloorRelease:    
        {
            if (pFirst->floorRequestID != pSecond->floorRequestID)
                return FALSE;
            break;
        }
        default:
            if (pFirst->floorID != pSecond->floorID)
                return FALSE;
            break;
        
    }   
    return TRUE;
}
