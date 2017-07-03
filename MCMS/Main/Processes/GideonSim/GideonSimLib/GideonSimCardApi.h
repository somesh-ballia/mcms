//+========================================================================+
//                    GideonSimCardApi.h                                   |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       GideonSimCardApi.h                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Vasily                                                      |
//+========================================================================+

#ifndef __GIDEONSIMCARDAPI_
#define __GIDEONSIMCARDAPI_


#include  "TaskApi.h"
#include "GideonSimLogicalParams.h"

class COsQueue;
class CCardCfg;

class CSimCardApi : public CTaskApi
{
CLASS_TYPE_1(CSimCardApi,CTaskApi )
public:
				// Constructors
	CSimCardApi();
	virtual ~CSimCardApi();
	virtual const char* NameOf() const { return "CSimCardApi";}
	
				// Initializations

				// Operations
	void  Create(COsQueue& creatorRcvMbx,CCardCfg* pCardCfg);

	void Connect();
	void  UpdateBurnRate(DWORD rate, eBurnTypes burnType);
	void  SendBurnAction(eBurnActionTypes burnActionType, eBurnTypes burnType);

protected:

};

#endif /* __GIDEONSIMCARDAPI_ */
