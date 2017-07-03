//+========================================================================+
//                               IceTaskApi.h                          |
//+========================================================================+

#ifndef _ICE_API_H__
#define _ICE_API_H__


#include "ManagerApi.h"
 
#define MAX_SERVICES_NUM 8

class CIceTaskApi : public CTaskApi
{
	CLASS_TYPE_1(CIceTaskApi,CTaskApi )
public: 
	
	CIceTaskApi(DWORD ServiceId,eProcessType process = eProcessIce);
	virtual ~CIceTaskApi();
	virtual const char* NameOf() const { return "CIceTaskApi";}


};



#endif 
