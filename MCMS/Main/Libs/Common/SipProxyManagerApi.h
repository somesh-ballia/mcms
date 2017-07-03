#ifndef SIPPROXYMANAGERAPI_H_
#define SIPPROXYMANAGERAPI_H_

//+========================================================================+
//                                                                      |
//            Copyright 2005 Poplycom Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE: SipProxyManagerApi.H                                                          |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Michael                                                            |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
// Ori | 12/05      |                                                      |
//+========================================================================+



#include  "PObject.h"
#include  "XmlApi.h"
#include "SipProxyTaskApi.h"


class CStateMachine;
class CSegment;
                          
class CSipProxyManagerApi : public CPObject
{
CLASS_TYPE_1(CSipProxyManagerApi,CPObject )

public: 
	CSipProxyManagerApi() {};
	virtual ~CSipProxyManagerApi() {};
	virtual const char* NameOf() const { return "CSipProxyManagerApi";}

  	STATUS AddConference(DWORD serviceId, const char* pName, DWORD confId, BYTE isEQ, DWORD duration=3600) const;
  	STATUS AddMR(DWORD serviceId, const char* pName, DWORD confId, DWORD duration=3600) const;
  	STATUS AddEQ(DWORD serviceId, const char* pName, DWORD confId, DWORD duration=3600) const;
  	STATUS AddFactory(DWORD serviceId, const char* pName, DWORD confId, DWORD duration=3600) const;
  	STATUS AddGW(DWORD serviceId, const char* pName, DWORD confId, DWORD duration=3600) const;
  	STATUS DelConference(DWORD serviceId, const char* pName, DWORD confId) const; 
  	STATUS ChangePresenceStatus(DWORD serviceId, const char* pName, DWORD confId, BYTE presenceState) const;

private:
	
};




#endif /*SIPPROXYMANAGERAPI_H_*/
