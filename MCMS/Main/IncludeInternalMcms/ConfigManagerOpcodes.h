// ConfigManagerOpcodes.h
//
#include "OpcodesRanges.h"

#ifndef CONFIG_MANAGER_OPCODES_H_
#define CONFIG_MANAGER_OPCODES_H_

#define CONFIGURATOR_ADD_IP_INTERFACE                                (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+1)
#define CONFIGURATOR_REMOVE_IP_INTERFACE                             (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+2)
#define CONFIGURATOR_GET_NEW_VERSION_NUMBER                          (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+3)
#define CONFIGURATOR_DNS_SERVIER                                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+4)
#define CONFIGURATOR_RUN_DHCP                                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+5)
#define CONFIGURATOR_REMOUNT_VERSIONS                                (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+6)
#define CONFIGURATOR_CYCLE_VERSION                                   (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+7)
#define CONFIGURATOR_ENABLE_DISABLE_PING                             (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+8)
#define CONFIGURATOR_ENABLE_DISABLE_PING_BROADCAST                   (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+9)
#define CONFIGURATOR_ADD_USER                                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+10)
#define CONFIGURATOR_DEL_USER                                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+11)
#define CONFIGURATOR_CHANGE_PASSWORD                                 (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+12)
#define CONFIGURATOR_ADD_VLAN_INTERFACE                              (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+13)
#define CONFIGURATOR_RESTART_APACHE                                  (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+14)
#define CONFIGURATOR_FINISH_SYNC_OPER_DB                             (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+15)
#define CONFIGURATOR_SYNC_TIME                                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+16)
#define CONFIGURATOR_TEST_DMA                                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+17)
#define CONFIGURATOR_TEST_ETH_SETTINGS                               (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+18)
#define CONFIGURATOR_NTP_PEER_STATUS                                 (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+19)
#define CONFIGURATOR_SET_ETH_SETTINGS                                (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+20)
#define CONFIGURATOR_GET_SMART_ERRORS                                (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+21)
#define CONFIGURATOR_KILL_DHCP                                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+22)
#define CONFIGURATOR_GET_DHCP                                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+23)
#define CONFIGURATOR_NS_UPDATE                                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+24)
#define CONFIGURATOR_RESTART_SSHD                                    (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+25)
#define CONFIGURATOR_TAKE_CORE_OWNERSHIP                             (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+26)
#define CONFIGURATOR_RESTART_SNMPD                                   (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+27)
#define CONFIGURATOR_START_SNMPD                                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+28)
#define CONFIGURATOR_STOP_SNMPD                                      (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+29)
#define CONFIGURATOR_ARPING_REQUEST                                  (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+30)
#define CONFIGURATOR_DELETE_FILE                                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+31)
#define CONFIGURATOR_REMOUNT_PARTITION                               (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+32)
#define CONFIGURATOR_DAD_REQUEST                                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+33)
#define CONFIGURATOR_ADD_IPV6_INTERFACE                              (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+34)
#define CONFIG_INTERFACE_UP                                          (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+35)
#define CONFIGURATOR_DEL_TEMP_FILES                                  (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+36)
#define CONFIG_ADD_DEFAULT_GW                                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+37)
#define CONFIG_IPV6_AUTOCONFIG                                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+38)
#define CONFIGURATOR_ADD_DEFAULT_GW_ROUTE_RULE                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+39)
#define CONFIGURATOR_DISABLE_ACCEPT_RA                               (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+40)
#define CONFIGURATOR_CONFIG_ETHERNET                                 (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+41)
#define CONFIGURATOR_SET_PRODUCT_TYPE                                (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+42)
#define CONFIGURATOR_EVOKE_NETWORK_INTERFACES                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+43)
#define CONFIGURATOR_SET_MAC_ADDRESS                                 (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+44)
#define CONFIGURATOR_GET_HD_TEMPERATURE                              (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+45)
#define CONFIGURATOR_ETH_SETTINGS_MONITORING                         (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+46)
#define CONFIGURATOR_MOUNT_NEW_VERSION                               (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+47)
#define CONFIGURATOR_FIRMWARE_CHECK                                  (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+48)
#define CONFIGURATOR_ADD_ROUTE_RULE                                  (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+49)
#define CONFIGURATOR_RUN_SMART_SELFTEST                              (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+50)
#define CONFIGURATOR_DEL_DEFAULT_GW_ROUTE_RULE                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+51)
#define CONFIGURATOR_SET_TCP_STACK_PARAMS                            (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+52)
#define CONFIG_ADD_STATIC_IP_ROUTE                                   (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+53)
#define CONFIGURATOR_HANDLE_FALLBACK                                 (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+54)
#define CONFIGURATOR_EXPOSE_EMBEDDED_NEW_VERSION                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+55)
#define CONFIGURATOR_UNMOUNT_NEW_VERSION                             (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+56)
#define CONFIGURATOR_ADD_DROP_RULE_SHOREWALL                         (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+57)
// for Call Generator - change MM process priority
#define CONFIGURATOR_CHANGE_PID_NICE_LEVEL                           (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+58)
// from v4.6
#define CONFIGURATOR_SET_SECONDARY_SIG_MAC                           (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+59)
#define CONFIGURATOR_FAILOVER_ARPING_REQUEST                         (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+60)

#define CONFIGURATOR_RESTORE_FALLBACK_VERSION                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+61)
#define CONFIGURATOR_WRITE_FILE                                      (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+62)
#define CONFIGURATOR_MORE_DNS_SERVER                                 (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+65)
#define CONFIGURATOR_KILL_SSHD                                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+67)
#define CONFIG_ADD_STATIC_ROUTE                                      (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+68)


#define CONFIGURATOR_STOP_CHECK_ETH0_LINK                            (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+69)
#define CHECK_ETH0_LINK_TIMER                                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+70)
#define CLEAR_TCP_DUMP_STORAGE                                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+71)
#define START_TCP_DUMP                                               (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+72)
#define STOP_TCP_DUMP                                                (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+73)


#define CONFIGURATOR_GET_HD_SIZE                                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+74)
#define CONFIGURATOR_GET_HD_MODEL                                    (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+75)
#define CONFIGURATOR_GET_FLASH_SIZE                                  (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+76)
#define CONFIGURATOR_GET_FLASH_MODEL                                 (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+77)
#define CONFIGURATOR_GET_CPU_TYPE                                    (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+78)
#define CONFIGURATOR_GET_RAM_SIZE                                    (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+79)

#define  CONFIGURATOR_SET_SPECIAL_PRODUCT_TYPE                       (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+80)
#define  CONFIGURATOR_GET_VALUE_FROM_REGISTER                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+81)
#define  CONFIGURATOR_GET_TEMP_ADVANTECH_UTIL                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+82)
// VNGR-21782
#define  CONFIGURATOR_RENAME_TCPDUMP_OUTPUT                          (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+83)
#define CONFIGURATOR_GET_HD_FIRMWARE                                 (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+84)

#define  CONFIGURATOR_CHECK_CS_IP_CONFIG                             (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+85)
#define CONFIGURATOR_ENABLE_DISABLE_PING_IPTABLES                    (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+86)
#define CONFIGURATOR_CONFIG_BONDING_INTERFACE                        (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+87)

#define CONFIGURATOR_SET_BONDING_INTERFACE_SLAVES                    (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+88)
#define CONFIGURATOR_ENABLE_WHITE_LIST			             (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+89)
#define CONFIGURATOR_DISABLE_WHITE_LIST			             (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+90)
#define CONFIGURATOR_QOS_MANAGEMENT_DSCP		        	(CONFIGURATOR_FIRST_OPCODE_IN_RANGE+91)
#define CONFIG_NETWORK_SEPERATION_IPV6_VLAN_I	                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+92)
#define CONFIG_NETWORK_SEPERATION_IPV6_VLAN_II	                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+93)
#define CONFIG_RUN_COMMAND                                           (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+94)
#define CONFIGURATOR_WRITE_FILE_ROOT_ONLY			(CONFIGURATOR_FIRST_OPCODE_IN_RANGE+95)
#define CONFIGURATOR_CHECK_ETH2_LINK				(CONFIGURATOR_FIRST_OPCODE_IN_RANGE+96)
#define ADD_IP_AND_NAT_RULE_FOR_DNS_PER_SERVICE                      (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+97)
#define CONFIGURATOR_ADD_PORTS_IPTABLES   			     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+98)

#define CONFIGURATOR_FPGA_UPGRADE				(CONFIGURATOR_FIRST_OPCODE_IN_RANGE+99)

#define CONFIGURATOR_DELETE_DIR                                      (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+100)

#define CONFIGURATOR_NTP_SERVICE                                     (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+101)
#define CONFIGURATOR_CONFIG_NTP_SERVERS                              (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+102)
#define CONFIGURATOR_ADD_NAT_RULE				(CONFIGURATOR_FIRST_OPCODE_IN_RANGE+103)
#define CONFIGURATOR_DELETE_NAT_RULE				(CONFIGURATOR_FIRST_OPCODE_IN_RANGE+104)

#define CONFIGURATOR_ENABLE_DHCP_IPV6				(CONFIGURATOR_FIRST_OPCODE_IN_RANGE+105)
#define CONFIGURATOR_DISABLE_DHCP_IPV6				(CONFIGURATOR_FIRST_OPCODE_IN_RANGE+106)


#define CONFIGURATOR_START_CHECK_ETH0_LINK                            (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+107)

/*Begin:added by Richer for BRIDGE-15015, 11/13/2014*/
#define CONFIGURATOR_LED_SYS_ALARM                              (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+108)
/*End:added by Richer for BRIDGE-15015, 11/13/2014*/
#define CONFIGURATOR_GET_UDP_OCCUPIED_PORTS                    (CONFIGURATOR_FIRST_OPCODE_IN_RANGE+109)

#endif  // CONFIG_MANAGER_OPCODES_H_
