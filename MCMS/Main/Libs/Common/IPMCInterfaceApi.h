//+========================================================================+
//                            IPMCInterfaceApi.h                           |
//+========================================================================+

#ifndef _IPMC_INTERFACE_API_H
#define _IPMC_INTERFACE_API_H


#include "ManagerApi.h"
#include "IPMC.h"

class CIPMCInterfaceApi : public CManagerApi
{


	CLASS_TYPE_1(CIPMCInterfaceApi,CManagerApi )
public:

	virtual const char* NameOf() const { return "CIPMCInterfaceApi";}
	CIPMCInterfaceApi();
	virtual ~CIPMCInterfaceApi();

    STATUS InitializeLeds();
    STATUS ChangeLedState(const eLedColor Led_Color , const eLedState Led_State);
    STATUS SetWatchdog(const DWORD time_interval,bool isSyncMsg = false);
    STATUS TriggerWatchdog(bool isSyncMsg = false);
    STATUS TurnOffWatchdog();
    STATUS SetPostCommand(const DWORD command);
    STATUS SendCpuTemperature(const int temp);
    STATUS SendHdTemerature(const int temp);
    STATUS UpgradeIPMCVersion();
};



#endif /* _IPMC_INTERFACE_API_H */
