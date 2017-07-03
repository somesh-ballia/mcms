#ifndef _CSCOMMONSTRUCTS_H_
#define _CSCOMMONSTRUCTS_H_


#include "DataTypes.h"
#include "DefinesIpService.h"



typedef struct
{
	APIU32 service_id;
	char service_name[NET_SERVICE_PROVIDER_NAME_LEN];
}Del_Ip_Service_S;



#endif /*_CSCOMMONSTRUCTS_H_*/
