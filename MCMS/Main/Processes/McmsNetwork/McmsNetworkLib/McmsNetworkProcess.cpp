// McmsNetworkProcess.cpp

#include "McmsNetworkProcess.h"
#include "SystemFunctions.h"
#include "NetCommonDefines.h"

extern void McmsNetworkManagerEntryPoint(void* appParam);

CProcessBase* CreateNewProcess()
{
  return new CMcmsNetworkProcess;
}

TaskEntryPoint CMcmsNetworkProcess::GetManagerEntryPoint()
{
  return McmsNetworkManagerEntryPoint;
}

CMcmsNetworkProcess::CMcmsNetworkProcess()
{
	 m_enableLocalTracer = TRUE;
}

CMcmsNetworkProcess::~CMcmsNetworkProcess()
{}


void CMcmsNetworkProcess::AddExtraStatusesStrings()
{
	AddStatusString(PLATFORM_STATUS_MNGMNT_IS_NULL, "Status Failed: Management service  pointer was null.");
	AddStatusString(PLATFORM_STATUS_OK_DHCP_IS_ENABLED, "Status OK: Management service  No need to configure DHCP server is enabled.");
	AddStatusString(MANGMENT_STATUS_IS_TARGET_VM, "Status: MCU is running on a virtual machine.");
	AddStatusString(MANGMENT_RMX200_STATUS_NO_NET_SEPERATION, "Status: Rmx2000 is not in Jitc and network Separation configuration not needed.");

	AddStatusString(MANGMENT_STATUS_INTERFACE_READY, "Status Ok: Interfaces are ready and validated as define in Management service");
	AddStatusString(MANGMENT_STATUS_INTERFACE_WAITING_FOR_IPV6, "Status In Progress: Waiting to receive ipv6 from router.");
	AddStatusString(MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV4, "Status Error: Interfaces was not found for ipv4 type");
	AddStatusString(MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV6, "Status Error: Interfaces was not found for ipv6 type");
	AddStatusString(MANGMENT_STATUS_INTERFACE_ERROR_CONFILCT_IPV4, "Status Error: Interfaces is up but Ipv4 in Management service does not match interface.");
	AddStatusString(MANGMENT_STATUS_INTERFACE_ERROR_CONFILCT_IPV6, "Status Error: Interfaces is up but Ipv6 in Management service does not match interface.");
	AddStatusString(MANGMENT_STATUS_IPTYPE_CHANGE_FROM_BOTH_TO_IPV4, "Status Error: Failed to configure ipv6 for Management interface reseting system will clear alarm");

	AddStatusString(MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV4, "Status In Progress: VM system IPv4 is not valid yet - need to wait for valid IPv4");
	AddStatusString(MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV6, "Status In Progress: VM system IPv6 is not valid yet - need to wait for valid IPv6");

	AddStatusString(NET_FACTORY_STATUS_ERR_MANGMENT_WAS_NOT_CREATED, "Status Error: Logical error attempt to write Management configuration before creating Management Object ");

	AddStatusString(DNS_STATUS_OBJ_IS_NULL, "Status Failed: CIpDns pointer is null.");
	AddStatusString(DNS_STATUS_INVALID_REG_MODE, "Status Failed: Invalid registration mode for DNS.");
	AddStatusString(DNS_STATUS_FAIL_TO_OPEN_FILE, "Status Failed to open DNS configuration file.");
	AddStatusString(DNS_STATUS_FAIL_TO_RUN_NSUPDATE, "Status Failed to run the nsupdate command.");

	AddStatusString(SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV4, "Status In Progress: VM system IPv4 is not valid yet - need to wait for valid signal/media IPv4");
	AddStatusString(SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV6, "Status In Progress: VM system IPv6 is not valid yet - need to wait for valid signal/media IPv6");


}
