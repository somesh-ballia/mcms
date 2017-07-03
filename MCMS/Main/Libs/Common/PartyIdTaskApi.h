//+========================================================================+
//                               PartyIdTaskApi.h                          |
//+========================================================================+

#ifndef _PARTY_ID_API_H__
#define _PARTY_ID_API_H__


#include "ManagerApi.h"
 

class CPartyIdTaskApi : public CTaskApi
{
	CLASS_TYPE_1(CPartyIdTaskApi,CTaskApi )
public: 
	
	CPartyIdTaskApi(DWORD PartyId,eProcessType process = eProcessConfParty);
	virtual ~CPartyIdTaskApi();
	virtual const char* NameOf() const { return "CPartyIdTaskApi";}


};



#endif 
