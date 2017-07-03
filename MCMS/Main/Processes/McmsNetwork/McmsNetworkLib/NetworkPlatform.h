//
//************************************************************************
//   (c) Polycom
//
//  Name:   McmsNetworkManager.h
//
//  Description:
/*  Defines the MCMS Network Class Hierarchy
 *  Implements the common functionality
 *
 *  Management configuration:
 *
 *	 System Flags:
 *	  LAN_REDUNDANCY
 *	  MULTIPLE_SERVICES
 *	  ULTRA_SECURE_MODE
 *	  SEPARATE_MANAGEMENT_NETWORK
 *	  Reads the system flags and if value is true value is set in bit options  m_flagState;
 *    checks if target or not
 *
 *	  call the ConfigNetworkMngmnt
 *
 *   Documentation List :
 *   HLD-
 *   	1. http://isrportal07/sites/rd/carmel/Restricted%20Documents/SW%20Department/Management/V100/ReWrite/Startup%20New%20design.pptx
 *   TestList
 *
 */
//
//  Revision History:
//
//  Date            Author           Functional/Interface Changes
//  -----------    -------------    ----------------------------------------
//  July 18, 2013    stanny			  created class
//************************************************************************

#ifndef NETWORKPLATFORM_H_
#define NETWORKPLATFORM_H_

#include "SharedMcmsCardsStructs.h"
#include "CardsStructs.h"
#include "McuMngrStructs.h"
#include "ConfigManagerApi.h"
#include "IpParameters.h"
#include "DefinesGeneral.h"
#include "IpService.h"
#include "CNetworkSettings.h"



//#include "DefinesGeneral.h"


namespace McmsNetworkPackage {


#define INVALID_SERVER_ADDR_STR "255.255.255.255"

typedef enum {
		flagJitcMode		 =0x01, // 1 == "00000001"
		flagLanRedunancy 	 =0x02, // 2 == "00000010"
		flagSeperatedNetwork =0x04, // 4 == "00000100"
		flagMultipleServices =0x8,   // 8 == "00001000"
		flagIsTarget		 =0x10   // 16 =="00010000"
	}SystemStateOptions;



// 0x20 ==  32 == "00100000"
// 0x40 ==  64 == "01000000"
// 0x80 == 128 == "10000000"


#define TEST_ROOT_AND_RETURN \
	if (TestRoot() == FALSE) \
	{ \
		return STATUS_NO_PERMISSION; \
	}

#define TEST_ROOT_AND_RETURN_NO_STATUS \
	if (TestRoot() == FALSE) \
	{ \
		return; \
	}

class CBaseNetworkPlatform : public CPObject{
	CLASS_TYPE_1(CBaseNetworkPlatform, CPObject)
protected:
//************ COMMON SYSTEM FLAGS *****************************
	static BYTE m_SystemState;
	// functions for does the bitwise operations.
	inline void   SetSystemFlagState(SystemStateOptions flagOpt){m_SystemState |= flagOpt;};
	inline void   RemoveSystemFlagState(SystemStateOptions flagOpt){m_SystemState = m_SystemState & ~flagOpt;};
	inline BOOL   IsSystemFlagExist(SystemStateOptions flagOpt) {return (m_SystemState & flagOpt); };
	// functions for reading flags that can have different implementations
	virtual BOOL IsRedundancySystemFlag();
	/// functions for reading flags that have common implementations
	void ReadCommonSystemFlags();
	virtual void init();
//************** IP CONFIGURATION FUNCTIONS *****************************
	virtual  eConfigInterfaceNum GetInterfaceNum(const eConfigInterfaceType ifType,eIpType ipType) =0;
	virtual  eConfigDeviceName   GetInterfaceDeviceNum(const eConfigInterfaceType ifType) =0;
	//Returns the 'logical' interface name
	virtual std::string GetLogicalInterfaceName(const eConfigInterfaceType ifType,const eIpType ipType);
	//Returns the 'hardware' device name
	const char* GetDeviceName(const eConfigInterfaceType ifType);

	virtual STATUS AddIpInterface(eConfigInterfaceType eNetworkType, eIpType ipType,
															  const std::string & ip,
															  const std::string & mask,
															  const std::string & gateway,
															  const std::string & broadcast,
															  const std::list<CRouter> &routerList);
	virtual STATUS AddIpV6Interface(eConfigInterfaceType ifType,BYTE  bIsAutoConfig,
																std::string ip = "",
																std::string ipmask = "",
																std::string gateway = "",
																std::string gwmask = "");
	virtual STATUS ConfigBondingInterfaceSlaves();
	virtual STATUS ConfigBondingInterface(std::string& bondingMode,DWORD linkmonitoringfrequency);

	STATUS AddRouteTableRule(eConfigInterfaceType networkType,eIpType ipType,const std::string & ip,
																 const DWORD mask,
																 const std::string & defaultGW,
																 const std::string ipv6SubNetMask,
																 std::list<CRouter>& routerList,
																 const BOOL bIPv6=FALSE);
	STATUS DelRouteTableRule(eConfigInterfaceType networkType,eIpType ipType,const std::string ip,
																 const DWORD mask,
																 const std::string defaultGW,
																 const BOOL bIPv6=FALSE);
	virtual void SetNICIpV6AutoConfig(const string &NIC, BYTE bIsAutoConfig,const eConfigInterfaceType ifType ,
												const eIpType ipType,const string ipv6gateway,const string ipv6gwmask);
	virtual void ConfigureNetworkSeperationIpv6Only(eConfigInterfaceType ifType,eIpType ipType,std::string ip,DWORD mask,
		    														std::string defaultGW,std::string ipv6SubNetMask,BOOL bIPv6) {};

	virtual void    ConfigNetworkSeperationIpv6Vlan(std::string& route_table,std::string& NIC);

	//virtual STATUS	ArpingRequest(std::string &ip,DWORD tmpIpType, eIpType ipType,std::string &sourceIP,BOOL isFailover=FALSE) =0;
	virtual STATUS DADRequest(const std::string& ip);

	void ifconfig_down(std::string stNicName);
	void ifconfig_up(std::string stNicName);
	virtual eConfigInterfaceType GetMngrIfType();
	void   RetrieveRouterList(CIPService *pService, std::list<CRouter> & routerList);
	virtual std::string GetCmdAutoConfigIpv6Flag(std::string& value,const std::string &NIC);
	virtual STATUS      AddIpv6ToInterfaceManaul(std::string nic_name,std::string ip,std::string ipmask,std::string gateway,std::string gwmask);
	virtual std::string GetMacAddress(std::string interface);
//************ DHCP *********************************************
	STATUS KillDHCP(eIpType ipType);
	virtual STATUS    OnConfigDHCP(eIpType ipType);
//*********** GENERAL ******************************************
	virtual BOOL		TestRoot();
	virtual BOOL 		TestMcmsUser();
	virtual std::string GetCmdLinePrefix(){return "";};
	std::string GetStatusAsString(STATUS status);

	std::string m_strAlarm;
public:
	virtual const char* NameOf() const { return "CBaseNetworkPlatform";}
	CBaseNetworkPlatform();
	virtual ~CBaseNetworkPlatform();

    void      SetAlarm(std::string& alarm_msg) { m_strAlarm = alarm_msg; }
    std::string GetAlarm() { return m_strAlarm; }
/*
 * TryToRecover function:
 * function : when occur some error, try to recover to default mode. Different products should have different behaviors
 *            Take MFW as instance, management and signal/media will get first available NIC and create a new configuration files
 * Parameters : They are convenient for judgement how to recover, according self status and other network status
 */
    virtual STATUS TryToRecover(STATUS eSelfLastStatus, STATUS eOtherLastStatus, CBaseNetworkPlatform *pOtherNetwork) { return STATUS_OK; }

};

}

 /* namespace McmsNetworkPackage */
#endif /* NETWORKPLATFORM_H_ */
