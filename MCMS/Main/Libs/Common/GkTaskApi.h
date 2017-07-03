//+========================================================================+
//                               GkTaskApi.h                          |
//+========================================================================+

#ifndef _GATEKEEPER_API_H__
#define _GATEKEEPER_API_H__


#include "ManagerApi.h"

#define MAX_SERVICES_NUM 8

class CGatekeeperTaskApi : public CTaskApi
{
	CLASS_TYPE_1(CGatekeeperTaskApi,CTaskApi )
public: 
	
	CGatekeeperTaskApi(DWORD ServiceId,eProcessType process = eProcessGatekeeper);
	virtual ~CGatekeeperTaskApi();
	virtual const char* NameOf() const { return "CGatekeeperTaskApi";}


};



#endif 
