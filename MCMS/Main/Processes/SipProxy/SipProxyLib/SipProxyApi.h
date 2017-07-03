//+========================================================================+
//                                                                         |
//            Copyright 1995 Pictel Technologies Ltd.                      |
//                   All Rights Reserved.                                  |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE: SIPXYAPI.H                                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Amir K                                                           |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//     |            |                                                      |
//+========================================================================+

#ifndef _SIPXYAPI__H_
#define _SIPXYAPI__H_

#include  "TaskApi.h"
/*#ifndef _NRESERV
#include    <nreserv.h>
#endif*/

/*#ifndef __SIPPROXYCONFCNTL_H__
#include	"SipProxyConfCntl.h"
#endif
*/

//===== opcodes ======
/*#define STARTCONF			2000
#define TERMINATECONF		2001	*/
#define KILLONECONF			2002
#define SIP_PROXY_DUMP		2003
#define REGISTRAR_STATUS	2004

                                         
class SIPProxyMngrApi : public CTaskApi
{
CLASS_TYPE_1(SIPProxyMngrApi,CTaskApi )
public: 
   
	// Constructors
	SIPProxyMngrApi();
	virtual const char* NameOf() const { return "SIPProxyMngrApi";}
	
    // Initializations  
								
	// Operations 
//	virtual void  HandleHdlcEvent(CHdlcEvent* pHdlcEvent); 								 								 
	void	Create( void (*entryPoint)(void*),COsQueue& creatorRcvMbx );
	void	Dump();
	
//	void	H323CardStartUpEnd(WORD boardId, BYTE spanType);
//	void	H323CardCrashed(WORD boardId);

	void	AddConf(const char* pName);
	void	DelConf(const char* pName);
	void	KillOneConf(WORD indDB);
	void	RegistrarStatusUpdate(WORD status, char* proxyName, WORD cardId, mcTransportAddress proxyIp, DWORD confID, WORD confType,DWORD expire);
	
	void	AddFactory(const char* name);
	void	DelFactory(const char* name);

	//Api for DNS
	void	DNSResolveResult(WORD cardId, char* pService, char* hostName, DWORD resolvedIp);
	void	DNSServiceResult(WORD cardId, char* pService, char* hostName, DWORD resolvedIp, DWORD resolvedPort, DWORD transportType);

protected:
								// Attributes             
				      
								// Operations	
};

#endif // _SIPXYAPI__H_ 
