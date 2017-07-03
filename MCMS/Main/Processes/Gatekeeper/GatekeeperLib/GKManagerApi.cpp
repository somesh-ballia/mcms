//+========================================================================+
//                  GKManagerApi.cpp							  		   |
//					Copyright Polycom							           |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GKManagerApi.cpp                                            |
// SUBSYSTEM:  Processes/Gatekeeper/GatekeeperLib		                   |                       |
// PROGRAMMER: Yael A.	                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |  Api between CCkCall and CGkManager				   |
//	   |			|													   |
//						 												   |
//						                                                   |
//+========================================================================+                        


#include "GKManagerApi.h"
#include "GKManagerOpcodes.h"

/*CGKManagerApi& CGKManagerApi::operator=(const CGKManagerApi &other)
{
	
}*/


void CGKManagerApi::SendNoResponseFromPartyToGkManager(DWORD mcmsConnId)
{
	CSegment*  pSeg = new CSegment;  
  	*pSeg << mcmsConnId;  	
  	SendMsg(pSeg, GK_CALL_NO_RESPONSE_FROM_PARTY);
}

