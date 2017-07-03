/*
 * CManagmentNetwork.h
 *
 *  Created on: Jul 28, 2013
 *      Author: stanny
 */

#ifndef CMANAGMENTNETWORK_H_
#define CMANAGMENTNETWORK_H_

#include "limits.h"

#include "NetworkPlatform.h"
#include "NetCommonDefines.h"
#include "McmsDNSNetwork/CDnsManagment.h"
namespace McmsNetworkPackage {

typedef struct
{
	DWORD   iPv4;
	char	iPv4_str[IP_ADDRESS_LEN];
	DWORD	iPv4_shelf;
	DWORD	iPv4_defGW;
	char 	iPv4_netMask[IPV6_ADDRESS_LEN];

	char	iPv6_str[IPV6_ADDRESS_LEN];
	char	iPv6_NoBrackets[IPV6_ADDRESS_LEN];
	char	iPv6_shelf[IPV6_ADDRESS_LEN];
	char	iPv6_defGW[IPV6_ADDRESS_LEN]; // no brackets
	char 	iPv6_netMask[IPV6_ADDRESS_LEN];
	eV6ConfigurationType	ipV6configType;

	eIpType ipType;

} IP_ADDR_S;

typedef IpV6AddressMaskS	MngmentIpv6Array[MNGMNT_NETWORK_NUM_OF_IPV6_ADDRESSES];

class CManagmentNetwork: public CBaseNetworkPlatform {
	CLASS_TYPE_1(CManagmentNetwork,CBaseNetworkPlatform)
protected:

	virtual STATUS     ValidateMngmntService();    //validate the mngmnt service before we attemp to use it to configure the network.
	virtual STATUS     OnPreConfigNetworkMngmnt();  // action which is call just before starting to config the management network.
	virtual STATUS	   OnPostConfigNetworkMngmnt(){return STATUS_OK;};  // action which  is called after config management network.
	virtual STATUS 	   OnConfigNetworkMngmnt();

	virtual STATUS     OnPreConfigIpInterface(); //call before configuring Ipv4 interface for mngmnt
	virtual STATUS 	   OnPreBondingInterface(); // call before config Bonding interface
	virtual STATUS 	   OnPreBondingInterfaceSlaves(); // call before adding bonding slaves

	virtual STATUS     OnPreHandleNetworkSeparationConfigurations();
	virtual STATUS     HandleNetworkSeparationConfigurations();
	void  SetBondingInterface();
	void  RetrieveIpAddresses(CIPService* pService, IP_ADDR_S & ipAddrS);
	virtual STATUS	  ConfigureEthernetSettingsCpu(eConfigInterfaceType ifType, ePortSpeedType portSpeed);
	virtual STATUS    OnInitInterfaces();
	virtual STATUS	  ValidateInitInterfacesStatus(STATUS& stat) {return STATUS_OK;}
	virtual STATUS    ValidateInterfaceIpv4(IP_ADDR_S& ipAddrS);
	virtual STATUS    ValidateInterfaceIpv6(IP_ADDR_S& ipAddrS);
	void 			  UpdateMngntServiceIPv6Auto(IpV6AddressMaskS autoIpV6[]);
	std::string 	  GetDefGwInAutoMode();
	virtual void      GetMngmntIpv6(MngmentIpv6Array& autoIpV6,string& retStr_ipV6,string& nic_name,eIpType curIpType);


//*********  VARIABLES ******************************
	CIPService* m_pMngmntService;
	eNetConfigurationStatus m_netConfigStatus;

public:
	virtual const char* NameOf() const { return "CManagmentNetwork";}
	CManagmentNetwork();
	virtual ~CManagmentNetwork();
	virtual STATUS  ConfigNetworkMngmnt(const CIPService* pMngmntService);
	virtual STATUS  IsMngmntInterfaceReady(eIpType ipType=eIpType_Both);
	virtual STATUS    WriteManagmentNetwork(CNetworkSettings& netSettings);
	virtual STATUS	 ConfigOtherNetComponents();
	void 	 SetNetworkMngmtConfigStatus(eNetConfigurationStatus status);

	virtual DWORD GetTimeoutofVM() { return UINT_MAX;}
};

} /* namespace McmsNetworkPackage */
#endif /* CMANAGMENTNETWORK_H_ */
