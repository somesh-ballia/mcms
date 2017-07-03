#ifndef __COMMSNMPSERVICE__H__
#define __COMMSNMPSERVICE__H__

#include "CommIPSListService.h"
#include "StatusesGeneral.h"



struct SNMP_CS_INFO_S;
class CIPService;



class CCommSnmpService : public CCommIPSListService  
{
public:
	CCommSnmpService();
	virtual ~CCommSnmpService();

	const char * NameOf(void) const {return "CCommSnmpService";}
	virtual STATUS SendIpServiceParamInd(CIPService *service);
    
	virtual STATUS SendIpServiceParamEndInd(){return STATUS_OK;}
	virtual STATUS SendDelIpService(CIPService *service){return STATUS_OK;}
	
private:
	void FillParam(SNMP_CS_INFO_S &param, CIPService *service)const ;
};



#endif // __COMMSNMPSERVICE__H__
