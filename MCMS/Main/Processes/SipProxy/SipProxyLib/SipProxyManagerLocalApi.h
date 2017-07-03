//+========================================================================+
//                  SipProxyManagerLocalApi.h                              |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                    All Rights Reserved.                                 |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SipProxyManagerLocalApi.h                                   |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Sami| 05/01/2010   |                                                    |
//+========================================================================+



#ifndef SIPPROXYMANAGERLOCALAPI_H_
#define SIPPROXYMANAGERLOCALAPI_H_

#include "SipProxyManagerApi.h"

class CSipProxyManagerLocalApi : public CSipProxyManagerApi
{
CLASS_TYPE_1(CSipProxyManagerLocalApi,CSipProxyManagerApi)
public:

	CSipProxyManagerLocalApi();
	virtual ~CSipProxyManagerLocalApi();
	virtual const char* NameOf() const { return "CSipProxyManagerLocalApi";}

	void UpdateSipProxyWithCredentials(DWORD id,const char* pUserName,const char* pPassword,const char* pRelayHostName,DWORD udpPortVal,DWORD tcpPortVal);

};





#endif /* SIPPROXYMANAGERLOCALAPI_H_ */
