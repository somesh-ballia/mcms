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


#ifndef GKMANAGERAPI_H_
#define GKMANAGERAPI_H_


#include "TaskApi.h"   


class CGKManagerApi : public CTaskApi
{
CLASS_TYPE_1(CGKManagerApi, CTaskApi)
public: 
	
		// Constructors
	CGKManagerApi() {};
	virtual const char* NameOf() const { return "CGKManagerApi";}
	~CGKManagerApi() {};  
	
	// Operations 
 	
 	//Error handling of calls:
 	void  SendNoResponseFromPartyToGkManager(DWORD mcmsConnId);
 	
 	//operators
//	CGKManagerApi& operator=(const CGKManagerApi &other) ;
 	
};	
   
  
#endif /*GKMANAGERAPI_H_*/
