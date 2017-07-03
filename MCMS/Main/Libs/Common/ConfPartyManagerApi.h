//+========================================================================+
//                            CONFPARTYAPI.H                               |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                    All Rights Reserved.                                 |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       CONFPARTYAPI.H                                              |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER:			                                                   |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+

#ifndef _CONFPARTYAPI_H__
#define _CONFPARTYAPI_H__


#include "ManagerApi.h"
#include "GkTaskApi.h"
#include "SipProxyTaskApi.h"

class CCommRes;

class CConfPartyManagerApi : public CManagerApi
{
	CLASS_TYPE_1(CConfPartyManagerApi,CManagerApi )
public:

	CConfPartyManagerApi();
	virtual ~CConfPartyManagerApi();
	virtual const char* NameOf() const { return "CConfPartyManagerApi";}


	STATUS  SipProxyDBRequest();
    STATUS  GkResourceQuery(DWORD serviceId);
    STATUS	SipProxyServerTypeRequest(DWORD serviceId, DWORD serverType);
};

#endif /* _CONFPARTYAPI_H__ */
