
/*===============================================================================================================*/   
/*            Copyright        2008  Polycom Israel, Ltd. All rights reserved                            	     */    
/*---------------------------------------------------------------------------------------------------------------*/    
/* NOTE: This software contains valuable trade secrets and proprietary   information of                          */    
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form                */    
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without                      */    
/* prior written authorization from Polycom Israel Ltd.                                                          */    
/*---------------------------------------------------------------------------------------------------------------*/   
/*                                                                                                               */ 
/* Purpose:   Header of EthSettings.c: Ethernet Settings Utils									                 */ 
/*                                                                                                               */ 
/* Author:    Yuval Tepper                                                                                       */ 
/*                                                                                                               */ 
/* Revision:  1.0                      Modtime: 2-Sep-2008                                                      */ 
/*****************************************************************************************************************/

#ifndef _ETHSETTINGS_H_
#define _ETHSETTINGS_H_


/***************************************************************************/
/* Common include files                                                    */
/***************************************************************************/

//#include "LinuxSystemCallsApi.h"
//#include "LSApi.h"
//#include "LSShared.h"
//#include "McmsApi.h"
#include "OpcodesMcmsCommon.h"
//#include "SocketApiWrapExt.h"
#include "CommonStructs.h"
//#include "usb.h"
/***************************************************************************/
/* Defines                                                   */
/***************************************************************************/

/******** System commands For ethernet configurationS/monitoring ******/

// The defines are used as strings to send a Linux system command 

/***************** Monitoring Commands - ethtool ***************/

#define STR_ETHTOOL_MONITOR_COMMAND 							          "/sbin/ethtool "	

//awk command used in order to receive only the wanted value from the string

// Monitor Link Status
#define STR_ETHTOOL_MONITOR_LINK_STATUS_COMMAND                           " | grep 'Link detected:' | awk '{print $3}';"

// Monitor Port Speed
#define STR_ETHTOOL_MONITOR_PORT_SPEED_COMMAND                            " | grep 'Speed' | awk '{print $2}' | awk -F M '{print $1}';"

// Monitor work mode - Duplex
#define STR_ETHTOOL_MONITOR_DUPLEX_COMMAND                                " | grep 'Duplex:' | awk '{print $2}';"

// Monitor work mode - AUTO-NEGOTIATION
#define STR_ETHTOOL_MONITOR_AUTO_NEGOTIATION_COMMAND                      " | grep 'Auto-negotiation:' | awk '{print $2}';"

// Monitor work mode - Advertised AUTO-NEGOTIATION  
#define STR_ETHTOOL_MONITOR_ADVERTISED_AUTO_NEGOTIATION_COMMAND           " | grep 'Advertised auto-negotiation:' | awk '{print $3}';"

// Monitor Link Mode - Advertised Link Mode
#define STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_COMMAND                  " | grep 'Advertised link modes:' -A2;"

/***************** Monitoring Transport Commands - ifconfig ***************/
//Ifconfig Monitoring Command

#define STR_IFCONFIG_MONITOR_COMMAND		     				          "/sbin/ifconfig "	

// Monitor Transmit Error Packet
#define STR_IFCONFIG_MONITOR_TRANSMIT_ERROR_PACKETS_COMMAND                " | grep 'TX packets:' | awk '{print $3}' | awk -F : '{print $2}' ;"

// Monitor Transmit FIFO Drops
#define STR_IFCONFIG_MONITOR_TRANSMIT_FIFO_DROPS_COMMAND                   " | grep 'TX packets:' | awk '{print $4}'| awk -F : '{print $2}' ;"

// Monitor Transmit Number of octets
#define STR_IFCONFIG_MONITOR_TRANSMIT_NUMBER_OF_OCTETS_COMMAND             " | grep 'TX bytes:'   | awk '{print $6}'| awk -F : '{print $2}' ;"

// Monitor Receive Error Packet
#define STR_IFCONFIG_MONITOR_RECEIVE_ERROR_PACKETS_COMMAND                 " | grep 'RX packets:' | awk '{print $3}' | awk -F : '{print $2}' ;"                                                                               
																																																							   
// Monitor Receive CRC Errors - see ethtool commands                                                                                                                                                                                                
																																					   
// Monitor Receive Number of octets                                                                                                                                                                                            
#define STR_IFCONFIG_MONITOR_RECEIVE_NUMBER_OF_OCTETS_COMMAND               " | grep 'RX bytes:'   | awk '{print $2}'| awk -F : '{print $2}' ;"                                                                                


/************ Monitoring Transport Commands - ethtools **********/

//Transport Monitoring Command

#define STR_ETHTOOL_TRANSPORT_MONITOR_COMMAND 						      "/sbin/ethtool -S "	

// Monitor Receive CRC Errors
#define STR_ETHTOOL_MONITOR_RECEIVE_CRC_ERRORS_COMMAND                    " | grep 'RX CRC'  | awk '{print $4}';"

//The following Ethtool commands are good for ethtool v1.8. currently, we use v3. ifconfig commands are replacing it. 
//
//// Monitor Transmit Error Packet
//#define STR_ETHTOOL_MONITOR_TRANSMIT_ERROR_PACKETS_COMMAND                " | grep 'tx_errors:' | awk '{print $2}';"
//
//// Monitor Transmit FIFO Drops
//#define STR_ETHTOOL_MONITOR_TRANSMIT_FIFO_DROPS_COMMAND                   " | grep 'tx_fifo_errors:'| awk '{print $2}';"
//
//// Monitor Transmit Number of octets
//#define STR_ETHTOOL_MONITOR_TRANSMIT_NUMBER_OF_OCTETS_COMMAND             " | grep 'tx_bytes:'| awk '{print $2}';"
//
//// Monitor Receive Error Packet
//#define STR_ETHTOOL_MONITOR_RECEIVE_ERROR_PACKETS_COMMAND                 " | grep 'rx_errors:'  | awk '{print $2}';"


//// Monitor Receive Number of octets
//#define STR_ETHTOOL_MONITOR_RECEIVE_NUMBER_OF_OCTETS_COMMAND              " | grep 'rx_bytes:'  | awk '{print $2}';"


/******************************************************/

/***************** Configuring Commands ***************/

#define STR_ETHTOOL_CONFIG_COMMAND 							              "/sbin/ethtool -s "
														           
//Configure port speed commands							           
														           
#define STR_ETHTOOL_CONFIG_SPEED_10_COMMAND					              " speed 10"
#define STR_ETHTOOL_CONFIG_SPEED_100_COMMAND				              " speed 100"
#define STR_ETHTOOL_CONFIG_SPEED_1000_COMMAND				              " speed 1000"	
														           
//Configure AUTO-NEGOTIATION statuses commands			           
														           
#define STR_ETHTOOL_CONFIG_AUTO_NEGOTIATION_YES_COMMAND		              " autoneg on"
#define STR_ETHTOOL_CONFIG_AUTO_NEGOTIATION_NO_COMMAND		              " autoneg off"
														           
//Configure working mode (DUPLEX) status				           
														           
#define STR_ETHTOOL_CONFIG_HALF_DUPLEX_COMMAND				              " duplex half"
#define STR_ETHTOOL_CONFIG_FULL_DUPLEX_COMMAND				              " duplex full"

/******************************************************/

/****************** Ethtool output Strings ********************/
//Link Status
#define STR_ETHTOOL_MONITOR_LINK_STATUS_DETECTED				          "yes\n"
#define STR_ETHTOOL_MONITOR_LINK_STATUS_NOT_DETECTED			          "no\n"

//Link Mode
#define STR_ETHTOOL_MONITOR_LINK_MODE_10HALF							  "Half\n10\n"
#define STR_ETHTOOL_MONITOR_LINK_MODE_100HALF							  "Half\n100\n"
#define STR_ETHTOOL_MONITOR_LINK_MODE_1000HALF                            "Half\n1000\n" 
#define STR_ETHTOOL_MONITOR_LINK_MODE_10FULL                              "Full\n10\n"
#define STR_ETHTOOL_MONITOR_LINK_MODE_100FULL                             "Full\n100\n" 
#define STR_ETHTOOL_MONITOR_LINK_MODE_1000FULL                            "Full\n1000\n" 

//Auto Negotiation
#define STR_ETHTOOL_MONITOR_AUTO_NEGOTIATION_ON				              "on\n"
#define STR_ETHTOOL_MONITOR_AUTO_NEGOTIATION_OFF			              "off\n"

//Advertised Auto Negotiation
#define STR_ETHTOOL_MONITOR_ADVERTISED_AUTO_NEGOTIATION_YES		          "Yes\n"
#define STR_ETHTOOL_MONITOR_ADVERTISED_AUTO_NEGOTIATION_NO		          "No\n"

//Advertised Link Mode
#define STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_10_FULL				  "10baseT/Full"
#define STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_10_HALF				  "10baseT/Half"
#define STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_100_FULL				  "100baseT/Full"
#define STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_100_HALF				  "100baseT/Half"
#define STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_1000_FULL				  "1000baseT/Full"
#define STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_1000_HALF				  "1000baseT/Half"																		  
																		  
																		  
/************************************************************************/

//Text size of line - Used for reading Linux output stream
#define LINE_SIZE                                                         256

//Offset used in order to get rid of the beginning of the string for advertised link mode parameter
#define ADVERTISED_LINK_MODES_OFFSET                                      24

//String Lengths Definition
#define SHORT_STRING                                                      20 
#define LONG_STRING                                                       60 

#define TIME_DELAY_BETWEEN_MONITORING									  8000

#define ETH_FIRST_PORT													  "eth0"
#define ETH_SECOND_PORT											          "eth3"

//Number of port as printed on board
#define ETH_FIRST_PORT_ON_BOARD											  1
#define ETH_SECOND_PORT_ON_BOARD										  2
#define MAX_NUMBERS_OF_PORTS											  3

#define MAX_SLOTID														  13
#define MIN_SLOTID														  18


//this struct is used to store the results of parsing the output of wpa_cli -ethX status
//in a form of enums rather than strings.
#define P802_1x_STR_SUP_PORT_STAT "suppPortStatus="
#define P802_1x_STR_EAP_STATE "EAP state="
#define P802_1x_STR_STATUS_COMMAND_FAILED "'STATUS' command failed"
#define P802_1x_STR_FAILED_TO_CONNECT "Failed to connect to wpa_supplicant"
#define P802_1x_STR_UNAUTHORIZED "Unauthorized"
#define P802_1x_STR_AUTHORIZED "Authorized"
#define P802_1x_STR_IDLE "IDLE"
#define P802_1x_STR_FAILURE "FAILURE"
#define P802_1x_STR_SUCCESS "SUCCESS"

#define P802_1x_STR_METHOD_TYPE "selectedMethod="
#define P802_1x_STR_METHOD_MD5_TYPE "EAP-MD5"
#define P802_1x_STR_METHOD_CHAP_TYPE "EAP-PEAP"
#define P802_1x_STR_METHOD_TLS_TYPE "EAP-TLS"

#define P802_1x_STR_METHOD_TYPE_FROM_CONF_FILE      "eap="
#define P802_1x_STR_METHOD_MD5_TYPE_FROM_CONF_FILE  "MD5"
#define P802_1x_STR_METHOD_CHAP_TYPE_FROM_CONF_FILE "PEAP"
#define P802_1x_STR_METHOD_TLS_TYPE_FROM_CONF_FILE  "TLS"

#define WPA_CONF_BUFFER_MAX_LENGTH (4096)


/***************************************************************************/
/* Enumerations  definition                                                   */
/***************************************************************************/

//The ENUM contains the possible statuses of AUTO-NEGOTIATION
typedef enum 
{

eAUTO_NEGOTIATION_YES,
eAUTO_NEGOTIATION_NO
		
} EN_ETH_STATUS_AUTO_NEGOTIATION; 

//The ENUM contains the possible statuses of port speed
typedef enum 
{

ePORT_SPEED10Mbps,
ePORT_SPEED100Mbps,
ePORT_SPEED1000Mbps
		
} EN_ETH_STATUS_PORT_SPEED; 

//The ENUM contains the possible statuses of working mode
typedef enum 
{

eHALF_DUPLEX,
eFULL_DUPLEX
		
} EN_ETH_STATUS_PORT_DUPLEX; 


typedef enum 
{

eFIRST_PORT,
eSECOND_PORT,
eMAX_PORT_NUM
		
} EN_PORT_NUM; 



/**********************************************************************************/

/***************************************************************************/
/* 						Functions prototypes                               */
/***************************************************************************/

/*********** Monitoring Ehternet properties **********/

//Function allocates all Ethernet information parameters of a given eth (e.g. "eth3") and fills the struct(stEthProperties) fields.
extern int EthGetEthInfo(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);

//The Following functions are used for getting a specific Ethernet propperty for monitoring
//Link Mode (e.g. 100Full)
int GetEthInfoLinkMode(const char *EthX, ETH_SETTINGS_S *pST_EthProperties );
//Auto Negotiation
int GetEthInfoAutoNego(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
//Advertised Auto Negotiation
int GetEthInfoAdvertisedAutoNego(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
//Advertised Link Mode
int GetEthInfoAdvertisedLinkMode(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
//Transmit Error Packets
int GetEthInfoTransmitErrorPacket(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
//Transmit FIFO Drops
int GetEthInfoTransmitFIFODrops(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
//Transmit Number Of Octets
int GetEthInfoTransmitNumOfOctets(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
//Received Error Packets
int GetEthInfoReceivedErrorPacket(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
//ReceivedGetEthInfoAdvertisedAutoNego CRC Errors
int GetEthInfoReceivedCRCErrors(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);
//Received Number of Octets
int GetEthInfoReceivedNumOfOctets(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);

int GetEthInfo802_1xStatus(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);

int GetEthInfo802_1xMethodType(const char *EthX, ETH_SETTINGS_S *pST_EthProperties);


//The Following functions are used for converting the string struct for eth properties to an UINT32 struct. The UINT32 is sent to MCMS

//Converting a single parameter from string to enum
int EthLinkModeATOI(const char *LinkModeStr, ETH_SETTINGS_S *pST_EthProperties);
int EthLinkAutoNegATOI(const char *AutoNegStr, ETH_SETTINGS_S *pST_EthProperties);
int EthAdvLinkModeATOI(const char *AdvLinkModeStr, ETH_SETTINGS_S *pST_EthProperties);
int EthAdvLinkAutoNegATOI(const char *AdvAutoNegStr, ETH_SETTINGS_S *pST_EthProperties);

//Update maximum values
void EthUpdateMaxValues(ETH_SETTINGS_S *pST_EthProperties);

/************ Reset Maximum Values *******************/

//Function resets the calculated Maximum Values (e.g. Maximum number of Error Packets) of a given eth (e.g. "eth3").
extern int EthClearMaxValues(UINT8 *EthX, ETH_SETTINGS_S *pST_EthProperties);


/************ Other Util Functions *******************/

//Check Link Status Function - function returns True/False/Error (Active/Not Active/Error) (1/0/-1) for Link Status Check.
extern int EthCheckLinkStatus(const char *EthX);

/********************************************************************/

#endif /* _ETHSETTINGS_H_ */



