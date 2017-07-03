//+========================================================================+
//             		SipProxyManagerLocalApi.cpp                            |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                    All Rights Reserved.                                 |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SipProxyManagerLocalApi.cpp                                 |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Inga                                                        |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|

#include "SipProxyManagerLocalApi.h"
#include "OpcodesMcmsInternal.h"
#include "Segment.h"
#include "TaskApi.h"


CSipProxyManagerLocalApi::CSipProxyManagerLocalApi()
{

}
////////////////////////////////////////////////////////////////////////////
CSipProxyManagerLocalApi::~CSipProxyManagerLocalApi()
{

}
////////////////////////////////////////////////////////////////////////////
void CSipProxyManagerLocalApi::UpdateSipProxyWithCredentials(DWORD id,const char* pUserName,const char* pPassword,const char* pRelayHostName,DWORD udpPortVal,DWORD tcpPortVal)
{
 /*   //This message arrives from SipProxyMsSubscriber
	CSegment*  seg = new CSegment;

	*seg << id
		 << pUserName
		 << pPassword
		 << pRelayHostName
		 << (DWORD)udpPortVal
		 << (DWORD)tcpPortVal;

	SendMsg(seg,UPDATE_CREDENTIALS);
*/
}
////////////////////////////////////////////////////////////////////////////

