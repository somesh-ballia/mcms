//+========================================================================+
//                               SipProxyTaskApi.h                          |
//+========================================================================+

#ifndef _SIPPROXY_API_H__
#define _SIPPROXY_API_H__


#include "ManagerApi.h"
 
#define MAX_SERVICES_NUM 8

const BYTE   SIP_PRESENCE_OFFLINE  	= 0;
const BYTE	 SIP_PRESENCE_ONLINE  	= 1;
const BYTE	 SIP_PRESENCE_BUSY     	= 2;

class CSipProxyTaskApi : public CTaskApi
{
	CLASS_TYPE_1(CSipProxyTaskApi,CTaskApi )
public: 
	
	CSipProxyTaskApi(DWORD ServiceId,eProcessType process = eProcessSipProxy);
	virtual ~CSipProxyTaskApi();
	virtual const char* NameOf() const { return "CSipProxyTaskApi";}


};



#endif 
