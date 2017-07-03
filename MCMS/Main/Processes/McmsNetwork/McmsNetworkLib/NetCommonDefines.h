//NetCommonDefines.h
#ifndef NETCOMMONDEFINES_H_
#define NETCOMMONDEFINES_H_


#define MANAGEMENT_NETWORK_CONFIG_PATH  "Cfg/NetworkCfg_Management.xml"

// MCMS NETWORK PLATFORM STATUS

#define PLATFORM_STATUS_MNGMNT_IS_NULL		 100000
#define PLATFORM_STATUS_OK_DHCP_IS_ENABLED   100001
#define PLATFORM_STATUS_UNKNOWN_PRODUCT_TYPE 100002
#define PLATFORM_STATUS_SGNLMD_ERR_CONFIG    100003
// MCMS Managment Network STATUS

#define MANGMENT_STATUS_IS_TARGET_VM					200000
#define MANGMENT_STATUS_INTERFACE_READY					200001
#define MANGMENT_STATUS_INTERFACE_WAITING_FOR_IPV6		200002
#define MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV4	200003
#define MANGMENT_STATUS_INTERFACE_ERROR_CONFILCT_IPV4	200004
#define MANGMENT_STATUS_INTERFACE_ERROR_CONFILCT_IPV6	200005
#define MANGMENT_STATUS_INTERFACE_ERROR_NOT_FOUND_IPV6	200006
#define MANGMENT_STATUS_IPTYPE_CHANGE_FROM_BOTH_TO_IPV4 200007
// status for common soft mfw edg sof mcu
#define MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV4		200100
#define MANGMENT_STATUS_SOFT_VM_WAITING_FOR_IPV6		200101
// status  for Network Factory
#define NET_FACTORY_STATUS_ERR_MANGMENT_WAS_NOT_CREATED	200200
// statues for rmx 2000 mamangement
#define MANGMENT_RMX200_STATUS_NO_NET_SEPERATION 		200300
// statues for DNS configuration
#define DNS_STATUS_OBJ_IS_NULL							200400
#define DNS_STATUS_INVALID_REG_MODE						200401
#define DNS_STATUS_FAIL_TO_OPEN_FILE					200402
#define DNS_STATUS_FAIL_TO_RUN_NSUPDATE						200403

#define SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV4        200501
#define SGNLMD_STATUS_SOFT_VM_WAITING_FOR_IPV6        200502


#endif
