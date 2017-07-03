//+========================================================================+
//                       H323Util.CPP                                       |
//             Copyright 2003 Polycom Israel Ltd.                          |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Polycom Israel Ltd. and is protected by law.             |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Polycom Israel Ltd.                    |
//-------------------------------------------------------------------------|
// FILE:       H323UTIL.CPP                                                  |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Yael                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     | 20/05/03   | Functions for H323 mcms uses             |
//+========================================================================+



#include "H323Util.h"

///////////////////////////////////////////////////////////
//			Highest Common:
///////////////////////////////////////////////////////////
	
const char* g_changeModeStateStrings[] = 
{
    "eNotNeeded",
	"eChangeIncoming", 
	"eReopenOut",
	"eChangeOutgoing",
	"eChangeInAndReopenOut",
	"eReopenIn", 
	"eReopenInAndOut",
    "eChangeInAndOut",
	"eFlowControlIn",
	"eFlowControlOut",
	"eFlowControlInAndOut",
	"eFlowControlOutAndChangeIn",
	"eFlowControlOutAndReopenIn",
	"eFlowControlInAndReopenOut",
	"eCanNotChange",
	"eChangeContentOut",
	"eChangeContentRate",
	"eChangeContentIn",
	"eChangeContentInAndOut",
	"eFallBackFromIceToSip",
	"eFallBackFromTipToSip",
	"eConfRequestMoveToMixed",
	"eLastChangeModeState"
};


BYTE GetDirectionByChangeModeState(eChangeModeState changeModeState, cmCapDirection &direction)
{
	switch (changeModeState)
	{
		case eReopenOut:
		case eChangeOutgoing:
		case eFlowControlOut:
		{
			direction = cmCapTransmit;
			break;
		}
        case eChangeIncoming:
		case eReopenIn:
		case eFlowControlIn:
		{
			direction = cmCapReceive;
			break;
		}
		case eChangeInAndReopenOut:
		case eReopenInAndOut:
		case eFlowControlInAndOut:
		case eFlowControlOutAndChangeIn:
		case eFlowControlOutAndReopenIn:
		case eFlowControlInAndReopenOut:
        case eChangeInAndOut:
        case eConfRequestMoveToMixed:
		{
			direction = cmCapReceiveAndTransmit;
			break;
		}
		case eChangeContentOut:
		{
			direction = cmCapTransmit;
			break;
		}
		case eChangeContentRate:
		{
			direction = cmCapReceive;
			break;
		}
		default:
			return FALSE;//incorrect changeModeState
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////
BYTE  IsChangeModeOfIncomingState(eChangeModeState changeModeState)
{
	BYTE bRes = FALSE;
	if ( (changeModeState == eChangeIncoming) || (changeModeState == eChangeInAndReopenOut)
		|| (changeModeState == eFlowControlOutAndChangeIn))
		bRes = TRUE;
	return bRes;
}

/////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////
std::string GetHexNum(const char* str)
{
	int i;
	char buffer[16];
	unsigned char lPart, rPart;
	std::string hexNum;
	const char* format = NULL;

	for (i = 0; i < Size16; i++ )
	{
		lPart = (str[i]>>4) & 0x0F;
		rPart = str[i] & 0x0F;

		//If it is last byte then don't add space
		format = (i == Size16-1)?"0x%x%x":"0x%x%x ";

		if (sprintf(buffer,format, lPart, rPart) != -1)
				hexNum.append(buffer);
	}
	hexNum.append("\0");
	return hexNum;
}

/////////////////////////////////////////////////////////////////////
std::string GetGuidFormat(const char* str)
{
	int i;
	char buffer[16];
	unsigned char lPart, rPart;
	std::string hexNum;

	for (i = 0; i < Size16; i++ )
	{
		lPart = (str[i]>>4) & 0x0F;
		rPart = str[i] & 0x0F;

		if (sprintf(buffer,"%x%x", lPart, rPart) != -1)
				hexNum.append(buffer);

	}
	hexNum.append("\0");
	return hexNum;
}
