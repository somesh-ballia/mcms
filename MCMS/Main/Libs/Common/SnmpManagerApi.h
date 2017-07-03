//+========================================================================+
//                            ConfigManagerApi.h                           |
//+========================================================================+

#ifndef _SNMP_MANAGER_API_H
#define _SNMP_MANAGER_API_H


#include <list>
#include "ManagerApi.h"





class CSnmpManagerApi : public CManagerApi
{

		
	CLASS_TYPE_1(CSnmpManagerApi,CManagerApi )
public: 
	
	virtual const char* NameOf() const { return "CSnmpManagerApi";}
	CSnmpManagerApi();
	virtual ~CSnmpManagerApi();

	
  /*  STATUS UpdateManagmentIp(const std::string & mamagment_ip,const std::string & switch_ip);
    STATUS CsMNGR_UpdateCsIp_MFA_IpAndGK_IP(const std::string & cs_ip,
                                            const std::string & MFA1_ip,
                                            const std::string & MFA2_ip,
                                            const std::string & Gk_ip);*/
//    STATUS UpdateSNMPnotification();                                                           
};



#endif /* _CONFIG_MANAGER_API_H */
