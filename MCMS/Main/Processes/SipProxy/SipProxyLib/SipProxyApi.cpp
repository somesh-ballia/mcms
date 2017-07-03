//+========================================================================+
//                         SIPProxyMngrApi.cpp                             |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       SIPProxyMngrApi.cpp                                         |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir K                                                      |
// Date:	   5/5/03                                                      |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#include "SipProxyApi.h"
#include "OpcodesMcmsInternal.h"


/////////////////////////////////////////////////////////////////////////////
SIPProxyMngrApi::SIPProxyMngrApi()          // constructor
{
}

/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
void  SIPProxyMngrApi::Create(void (*entryPoint)(void*),COsQueue& creatorRcvMbx)  
{  
	CTaskApi::Create(creatorRcvMbx);      // set default stack param i.e. creator rsmbx 

	WORD dummy = 77;
	m_appParam	<< dummy;
	
	LoadApp(entryPoint);                 // load application                                        
}

/////////////////////////////////////////////////////////////////////////////
/*void  SIPProxyMngrApi::HandleHdlcEvent(CHdlcEvent* pHdlcEvent) 
{
	CSegment*  msg    = new CSegment;
	*msg << (DWORD)0
		<< (WORD)HDLC_MSG; 
	
	pHdlcEvent->Serialize(NATIVE,*msg);
	POBJDELETE(pHdlcEvent);
	SendMessage(msg);
}*/
	
/////////////////////////////////////////////////////////////////////////////  
/*void	SIPProxyMngrApi::H323CardStartUpEnd(WORD boardId, BYTE spanType)
{
	CSegment*  seg = new CSegment;
	
	*seg << (DWORD)0                  // msgLen               
		<< (WORD)ON_CARD_END_STARTUP
		<< boardId
		<< spanType;
	SendMessage(seg);
}
*/
/////////////////////////////////////////////////////////////////////////////  
/*void	SIPProxyMngrApi::H323CardCrashed(WORD boardId)
{
	CSegment*  seg = new CSegment;
	
	*seg << (DWORD)0                  // msgLen               
		<< (WORD)H323_ON_CARD_CRASHED
		<< boardId;
	SendMessage(seg);
}*/

/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::AddConf(const char* pName)  
{
	CSegment*  seg = new CSegment;
	
	/* *seg << (DWORD)0                  // msgLen               
		<< (WORD)STARTCONF
		<< (DWORD)rsrv.GetConferenceId()
		<< rsrv.GetName()
		<< (DWORD)durationTime;*/
	*seg << pName;
	   
	SendMsg(seg, START_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::DelConf(const char* pName)
{
	CSegment*  seg = new CSegment;
	
	*seg << pName;

	SendMsg(seg, TERMINATE_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::AddFactory(const char* name)  
{
	CSegment*  seg = new CSegment;
	
	/* *seg << (DWORD)0                  // msgLen               
		<< (WORD)STARTCONF*/
	*seg << (DWORD)0x00000FAC
		<< name
		<< (DWORD)0xFFFFFFFF;
	   
	SendMsg(seg, START_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::DelFactory(const char* name)  
{
	CSegment*  seg = new CSegment;
	
	*seg << (DWORD)0x00000FAC
		<< name;

	SendMsg(seg, TERMINATE_CONF_REG);
}

/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::KillOneConf(WORD indDB)
{
	CSegment*  seg = new CSegment;
	
	*seg << indDB;

	SendMsg(seg, KILLONECONF);
}

/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::Dump()
{
	CSegment*  seg = new CSegment;
	
	
	SendMsg(seg, SIP_PROXY_DUMP);
}


/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::DNSResolveResult(WORD cardId, char* pService, char* hostName, DWORD resolvedIp)
{
	CSegment*  seg = new CSegment;
	
	*seg << cardId
		<< pService
		<< hostName
		<< resolvedIp;
	SendMsg(seg, DNS_RESOLVE_IND);
}

/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::DNSServiceResult(WORD cardId, char* pService, char* hostName, DWORD resolvedIp, DWORD resolvedPort, DWORD transportType)
{
	CSegment*  seg = new CSegment;
	
	*seg << cardId
		<< pService
		<< hostName
		<< resolvedIp
		<< resolvedPort
		<< transportType;
	SendMsg(seg, DNS_SERVICE_IND);
}

/////////////////////////////////////////////////////////////////////////////  
void  SIPProxyMngrApi::RegistrarStatusUpdate(WORD status, char* proxyName, WORD cardId, mcTransportAddress proxyIp, DWORD confID, WORD confType,DWORD expire)
{
	CSegment*  seg = new CSegment;
	
	*seg << status
		<< cardId
		<< confID
		<< confType
		<< expire
		<< proxyName;

	seg->Put((BYTE*)&proxyIp, sizeof(mcTransportAddress));		
		
	SendMsg(seg, REGISTRAR_STATUS);
}


/////////////////////////////////////////////////////////////////////////////  

