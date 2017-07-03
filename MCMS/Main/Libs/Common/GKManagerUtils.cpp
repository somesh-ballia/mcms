//+========================================================================+
//                  GKManagerUtils.h									   |
//					Copyright Polycom							           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GKManagerStructs.h                                          |
// SUBSYSTEM:  Libs/Common		                                           |
// PROGRAMMER: Yael A.	                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |   The file includes utilities used by				   |
//						GkManger process and other processes in Carmel     |
//						(ConfParty, CsMngr..)                              |
//+========================================================================+                        

////////////////////////////////////////////////////////////////////////////////////


#include "GKManagerUtils.h"
#include "DataTypes.h"
#include "SharedDefines.h"
#include <string.h>

void ConvertIdFromBmpString(const char* srcPtr, int identLength, char* destBuf)
{
	if (identLength == 0)
		return;

	BYTE bIsNeedToCut	= TRUE;  //there are cases that the one letter need all the 16 bytes

	for (int i=0, j=1; ((j<=identLength) && bIsNeedToCut); i++, j+=2 ) 
	{
		if(srcPtr[j-1] == 0)
			destBuf[i] = srcPtr[j];
		else
			bIsNeedToCut = FALSE;
	}

	if (bIsNeedToCut)
		destBuf [identLength/2] = '\0';
	else
		memcpy(destBuf, srcPtr, identLength);
} 

////////////////////////////////////////////////////////////////////////////////////


