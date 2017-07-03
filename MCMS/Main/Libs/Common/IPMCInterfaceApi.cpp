//+========================================================================+
//                         IPMCInterfaceApi.cpp                            |
//+========================================================================+

#include "IPMCInterfaceApi.h"
#include "IPMCInterfaceOpcodes.h"
#include "ProcessBase.h"
#include "ApiStatuses.h"

/////////////////////////////////////////////////////////////////////////////
CIPMCInterfaceApi::CIPMCInterfaceApi() // constructor
        :CManagerApi(eProcessIPMCInterface)
{

}

/////////////////////////////////////////////////////////////////////////////
CIPMCInterfaceApi::~CIPMCInterfaceApi() // destructor
{
}
/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::InitializeLeds()
{
    CSegment seg;
    OPCODE opcode = 0;
    STATUS stat =  SendMsg(NULL, IPMC_INITIALIZE_LEDS);
    if (stat == STATUS_OK)
        return opcode;
    else
        return stat;



}
/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::ChangeLedState(const eLedColor Led_Color,
                                         const eLedState Led_State)
{
    CSegment* seg= new CSegment;
    OPCODE opcode = 0;

    *seg << (DWORD)Led_Color
         << (DWORD)Led_State;

    STATUS stat =  SendMsg(seg, IPMC_CHANGE_LED_STATE);
    if (stat == STATUS_OK)
        return opcode;
    else
        return stat;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::SetWatchdog(const DWORD time_interval,bool isSyncMsg )
{
    CSegment *seg = new CSegment;
    OPCODE opcode = 0;

    *seg << time_interval;

    STATUS stat;

    if (!isSyncMsg)
         stat =  SendMsg(seg, IPMC_SET_WATCHDOG);
    else

    	 stat =  SendMessageSync(seg,
    			                 IPMC_SET_WATCHDOG,
    	                         30*SECOND,
    	                         opcode);


    if (stat == STATUS_OK)
        return opcode;
    else
        return stat;
}
/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::TriggerWatchdog(bool isSyncMsg)
{
    CSegment seg;
    OPCODE opcode = 0;
    STATUS stat;

    if (!isSyncMsg)
        stat =  SendMsg(NULL, IPMC_TRIGGER_WATCHDOG);
    else
    	 stat =  SendMessageSync(NULL,
    			 IPMC_TRIGGER_WATCHDOG,
    	    	 30*SECOND,
    	    	 opcode);

    if (stat == STATUS_OK)
        return opcode;
    else
        return stat;

}
/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::TurnOffWatchdog()
{
    CSegment seg;
    OPCODE opcode = 0;

    STATUS stat =  SendMsg(NULL, IPMC_TURN_OFF_WATCHDOG);

    if (stat == STATUS_OK)
        return opcode;
    else
        return stat;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::SetPostCommand(const DWORD command)
{

    CSegment *seg = new CSegment;
    OPCODE opcode = 0;

    *seg << command;

    STATUS stat =  SendMsg(seg, IPMC_SET_POST_COMMAND);
    if (stat == STATUS_OK)
        return opcode;
    else
        return stat;

}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::SendCpuTemperature(const int temp)
{
	CSegment *seg = new CSegment;
	OPCODE opcode = 0;

	*seg << (DWORD)temp;

	STATUS stat =  SendMsg(seg, IPMC_CPU_TEMPERATURE);

	if (stat == STATUS_OK)
		return opcode;
	else
		return stat;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::SendHdTemerature(const int temp)
{
	CSegment *seg = new CSegment;
	OPCODE opcode = 0;

	*seg << (DWORD)temp;

	STATUS stat =  SendMsg(seg, IPMC_HDD_TEMPERATURE);

	if (stat == STATUS_OK)
		return opcode;
	else
		return stat;
}

/////////////////////////////////////////////////////////////////////////////
STATUS CIPMCInterfaceApi::UpgradeIPMCVersion()
{
	CSegment seg;
	OPCODE opcode = 0;

	STATUS stat =  SendMsg(NULL, IPMC_UPGRADE_VERSION);

	if (stat == STATUS_OK)
		return opcode;
	else
		return stat;
}


