// CommMcuService.h: interface for the CCommMcuService class.
//
//
//Date         Updated By         Description
//
//3/8/05	  Yuri Ratner		Communication with McuMngr
//========   ==============   =====================================================================

#ifndef __COMMMCUSERVICE_H__
#define __COMMMCUSERVICE_H__


#include "CommMcmsService.h"


struct CSMNGR_LICENSING_S;
struct DNS_HOST_REGISTRATION_S;



class CCommMcuService : public CCommMcmsService  
{
public:
	CCommMcuService();
	virtual ~CCommMcuService();

	const char * NameOf(void) const {return "CCommMcuService";}
	STATUS SendNumPortReq();
	void ReceiveNumOfPortsInd(const CSMNGR_LICENSING_S *pParam)const;
	void ReceiveDnsHostRegistration(const DNS_HOST_REGISTRATION_S *pParam)const;
	
	virtual STATUS SendDelIpService(CIPService *service);

//    STATUS SendDnsHostRegictrationRequest();
    STATUS SendIpTypeRequest();
};

#endif // __COMMMCUSERVICE_H__
