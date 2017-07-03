// CommDnsAgentService.h: interface for the CCommDnsAgentService class.
//
//////////////////////////////////////////////////////////////////////

#if !defined __COMMDNSAGENTSERVICE_H__
#define __COMMDNSAGENTSERVICE_H__

#include "CommMcmsService.h"


class CSegment;
struct DNS_PARAMS_IP_S;




class CCommDnsAgentService : public CCommMcmsService  
{
public:
	CCommDnsAgentService();
	virtual ~CCommDnsAgentService();
	
	const char * NameOf(void) const {return "CCommDnsAgentService";}
	STATUS ReceiveDnsResolution(const DNS_PARAMS_IP_S &param);

	virtual STATUS SendDelIpService(CIPService *service);
};

#endif // __COMMDNSAGENTSERVICE_H__
