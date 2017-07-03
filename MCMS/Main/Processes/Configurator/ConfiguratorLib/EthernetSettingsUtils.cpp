/*===============================================================================================================*/   
/*            Copyright        2008  Polycom Israel, Ltd. All rights reserved                            	     */    
/*---------------------------------------------------------------------------------------------------------------*/    
/* NOTE: This software contains valuable trade secrets and proprietary   information of                          */    
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form                */    
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without                      */    
/* prior written authorization from Polycom Israel Ltd.                                                          */    
/*---------------------------------------------------------------------------------------------------------------*/   
/* Header:    EthSettings.h                                                         							 */ 
/*                                                                                                               */ 
/* Purpose:   Ethernet Settings Util Functions                                                                   */
/*                                                                                                               */
/*                                                                                                               */ 
/* Author:    Yuval Tepper                                                                                       */ 
/*                                                                                                               */ 
/* Revision:  1.0                      Modtime: 02-Sep-2008                                                      */ 
/*****************************************************************************************************************/

//Included Files

#include <string>
#include <stdlib.h>
using namespace std;

#include "EthernetSettingsUtils.h"
#include "SystemFunctions.h"
//#include "Utillity.h"

#include "802_1xApiDefinitions.h"


#include "StatusesGeneral.h"
#include "Trace.h"
#include "TraceStream.h"
#include <errno.h>

//Global Parameters declarations
// time delay between each monitoring call - init to default value, can be changed from terminal.
UINT32 EthMonitoringTimeDelay = TIME_DELAY_BETWEEN_MONITORING;

extern INT32 nTCPXmitQueue;
//Structs Declarations

//Struct array Used for maintaining Ethernet Info for all ports (currently 2)
static ETH_SETTINGS_S STEthPropertiesArr[1];

//Array Declaration
//Arrays used to turn the port number in EN_PORT_NUM to the number printed on the board and vice versa
UINT32 ConvertPortNumberEnumToPrintedOnBoard[eMAX_PORT_NUM]   = {ETH_FIRST_PORT_ON_BOARD, ETH_SECOND_PORT_ON_BOARD};
UINT32 ConvertPortNumberPrintedOnBoardToEnum[eMAX_PORT_NUM+1] = {FALSE , eFIRST_PORT, eSECOND_PORT};

//Parameters init


// Declarations
// Bit-Wise Port Informaion Masks
#define	_10HdxBit		0x01
#define	_10FdxBit		0x02
#define	_100HdxBit		0x04
#define	_100FdxBit		0x08
#define	_1000HdxBit		0x10
#define	_1000FdxBit		0x20


#define PORT_INACTIVE	0
#define PORT_ACTIVE		1

#define EMA_API_FALSE	0
#define EMA_API_TRUE	1

//#define LAN_SLOT_ID_OFFSET 31


//Functions Declaration

////////////////////////////////////////////////////////////////////////////////



/*****************************************************************************/
/* Name:      EthGetEthInfo              		            	     	 */
/*                                                                           */
/* Purpose:   The function allocates all Ethernet information parameters.    */
/*            and returns a struct with all updated	Ethernet propperties     */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                       */
/*                                                                           */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   				 */
/*                                                                           */
/*****************************************************************************/


int EthGetEthInfo(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations
	
	int ReturnValue;
	UINT32 LinkStatus;
	
	//Parameters init
	ReturnValue = 0;

	memset(pST_EthProperties, '\0', sizeof(ETH_SETTINGS_S));
    
	//Check if The eth requested is active
	LinkStatus = EthCheckLinkStatus(EthX);
	
	// temp - for debugging
	string sCmd =  "echo \"EthGetEthInfo - ";
	// temp - for debugging
	
	//If Link Status Not Detected - Return Error
	if (LinkStatus != TRUE)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(EthGetEthInfo) - %s Requested Ethernet info for eth: %s. eth - no Link Status Detected!!!",SourceName, EthX);	
		ReturnValue = -1;
		
		// temp - for debugging
		sCmd += "LinkStatus != TRUE  :-(\"";
		// temp - for debugging
		
		//802.1x
		pST_EthProperties->e802_1xSuppPortStatus = E_802_1x_PORT_STATUS_FAILED;
		pST_EthProperties->e802_1xFailReason     = E_802_1x_FR_LINK_STATUS_NOT_DETECTED;

	}
	else // Link Status Detected
	{
		// temp - for debugging
		sCmd += "LinkStatus detected  :-)\"";
		// temp - for debugging

		//Update All Eth Info Transport Struct fields (pST_EthProperties)

		//Link Mode
		ReturnValue |= GetEthInfoLinkMode(EthX, pST_EthProperties);

		//Auto Negotiation
		ReturnValue |= GetEthInfoAutoNego(EthX, pST_EthProperties);

		//Advertised Auto Negotiation
		ReturnValue |= GetEthInfoAdvertisedAutoNego(EthX, pST_EthProperties);

		//Advertised Link Mode
		ReturnValue |= GetEthInfoAdvertisedLinkMode(EthX, pST_EthProperties);

		//Transmit Error Packets
		ReturnValue |= GetEthInfoTransmitErrorPacket(EthX, pST_EthProperties);

		//Transmit FIFO Drops
		ReturnValue |= GetEthInfoTransmitFIFODrops(EthX, pST_EthProperties);

		//Transmit Number Of Octets
		ReturnValue |= GetEthInfoTransmitNumOfOctets(EthX, pST_EthProperties);

		//Received Error Packets
		ReturnValue |= GetEthInfoReceivedErrorPacket(EthX, pST_EthProperties);

		//Received CRC Errors
		ReturnValue |= GetEthInfoReceivedCRCErrors(EthX, pST_EthProperties);

		//Received Number of Octets
		ReturnValue |= GetEthInfoReceivedNumOfOctets(EthX, pST_EthProperties);

		ReturnValue |= GetEthInfo802_1xMethodType(EthX,pST_EthProperties);

		//Received 802.1x status and failure reason
		ReturnValue |= GetEthInfo802_1xStatus(EthX, pST_EthProperties);


		//Link Status - update Link Status active (checked above, at EthCheckLinkStatus)
		pST_EthProperties->ulActLinkStatus = PORT_ACTIVE;

		//Update Maximum Values
		EthUpdateMaxValues(pST_EthProperties);
	
	}

	// temp - for debugging
	sCmd += " > "+MCU_TMP_DIR+"/h.txt";
	string answer;
    SystemPipedCommand(sCmd.c_str() ,answer, TRUE, FALSE);
	// temp - for debugging

    return ReturnValue;
}

//////////////////////////////////////////////////////////////////////////////


/*******************************************************************************/
/* Name:      GetEthInfoReceivedNumOfOctets                		               */
/*                                                                             */
/* Purpose:   The Function Assigns Received Number Of Octets tnfo to Eth  	   */
/*			  info struct													   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoReceivedNumOfOctets(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_IFCONFIG_MONITOR_COMMAND;

    //Add eth number to command
    sCmd += EthX;
	
	//Add Number Of Octets Monitoring to command
    sCmd += STR_IFCONFIG_MONITOR_RECEIVE_NUMBER_OF_OCTETS_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);

	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoReceivedNumOfOctets) - Requested Ethernet Received Number Of Octetss info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Assign Value to info struct
		pST_EthProperties->ulRxOctets = atoi(sAnswer.c_str());
	}

	return ReturnValue;
}
////////////////////////////////////////////////////////////////////////////// 

/*******************************************************************************/
/* Name:      GetEthInfoReceivedCRCErrors                		               */
/*                                                                             */
/* Purpose:   The Function Assigns Received CRC Errors tnfo to Eth       	   */
/*			  info struct													   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoReceivedCRCErrors(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_ETHTOOL_TRANSPORT_MONITOR_COMMAND;

    //Add eth number to command
	sCmd += EthX;
	
	//Add Received CRC Errors Monitoring to command
	sCmd += STR_ETHTOOL_MONITOR_RECEIVE_CRC_ERRORS_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoReceivedCRCErrors) - Requested Ethernet Received CRC Errors info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Assign Value to info struct
		pST_EthProperties->ulRxCRC = atoi(sAnswer.c_str());
	}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////// 





/*******************************************************************************/
/* Name:      GetEthInfoReceivedErrorPacket                		               */
/*                                                                             */
/* Purpose:   The Function Assigns Received Error Packet tnfo to Eth     	   */
/*			  info struct													   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoReceivedErrorPacket(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_IFCONFIG_MONITOR_COMMAND;

    //Add eth number to command
	sCmd += EthX;
	
	//Add Received Error Packet Monitoring to command
	sCmd += STR_IFCONFIG_MONITOR_RECEIVE_ERROR_PACKETS_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoReceivedErrorPacket) - Requested Ethernet Received Error Packet info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Assign Value to info struct
		pST_EthProperties->ulRxBadPackets = atoi(sAnswer.c_str());
	}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////// 




/*******************************************************************************/
/* Name:      GetEthInfoTransmitNumOfOctets                		               */
/*                                                                             */
/* Purpose:   The Function Assigns Transmit Number Of Octets tnfo to Eth   	   */
/*			  info struct													   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoTransmitNumOfOctets(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_IFCONFIG_MONITOR_COMMAND;

    //Add eth number to command
	sCmd += EthX;
	
	//Add Transmit Number Of Octets Monitoring to command
	sCmd += STR_IFCONFIG_MONITOR_TRANSMIT_NUMBER_OF_OCTETS_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoTransmitNumOfOctets) - Requested Ethernet Transmit Number Of Octets info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Assign Value to info struct
		pST_EthProperties->ulTxOctets = atoi(sAnswer.c_str());
	}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////// 



/*******************************************************************************/
/* Name:      GetEthInfoTransmitFIFODrops                		               */
/*                                                                             */
/* Purpose:   The Function Assigns Transmit FIFO Drops tnfo to Eth   	       */
/*			  info struct													   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoTransmitFIFODrops(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_IFCONFIG_MONITOR_COMMAND;

    //Add eth number to command
	sCmd += EthX;
	
	//Add Transmit FIFO Drops Monitoring to command
	sCmd += STR_IFCONFIG_MONITOR_TRANSMIT_FIFO_DROPS_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoTransmitFIFODrops) - Requested Ethernet Transmit FIFO Drops info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Assign Value to info struct
		pST_EthProperties->ulTxFifoDrops = atoi(sAnswer.c_str());
	}

	return ReturnValue;
}

//////////////////////////////////////////////////////////////////////////////// 



/*******************************************************************************/
/* Name:      GetEthInfoTransmitErrorPacket              		               */
/*                                                                             */
/* Purpose:   The Function Assigns Transmit Error Packet tnfo to Eth   	       */
/*			  info struct													   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoTransmitErrorPacket(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_IFCONFIG_MONITOR_COMMAND;

    //Add eth number to command
	sCmd += EthX;
	
	//Add Transmit Error Packet Monitoring to command
	sCmd += STR_IFCONFIG_MONITOR_TRANSMIT_ERROR_PACKETS_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoTransmitErrorPacket) - Requested Ethernet Transmit Error Packet info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Assign Value to info struct
		pST_EthProperties->ulTxBadPackets = atoi(sAnswer.c_str());
	}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////// 




/*******************************************************************************/
/* Name:      GetEthInfoAdvertisedLinkMode              		               */
/*                                                                             */
/* Purpose:   The Function Assigns Advertised Link Mode info to Eth   	       */
/*			  info struct													   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoAdvertisedLinkMode(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_ETHTOOL_MONITOR_COMMAND;
      
    //Add eth number to command
	sCmd += EthX;
	
	//Add Advertised Link Mode Monitoring to command
	sCmd += STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoAdvertisedLinkMode) - Requested Ethernet Advertised Link Mode info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Convert string to Enum and Assign Value to info struct 
		ReturnValue |= EthAdvLinkModeATOI(sAnswer.c_str(), pST_EthProperties);

	    //If Assignment failed
	    if (ReturnValue)
	    {
			//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoAdvertisedLinkMode) - EthAdvLinkModeATOI returned error ");	
	    }
	}
	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////// 



/*******************************************************************************/
/* Name:      GetEthInfoAdvertisedAutoNego              		               */
/*                                                                             */
/* Purpose:   The Function Assigns Advertised Auto Negotiation info to Eth     */
/*			  info struct													   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoAdvertisedAutoNego(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_ETHTOOL_MONITOR_COMMAND;
      
    //Add eth number to command
	sCmd += EthX;
	
	//Add Advertised Auto Negotiation Monitoring to command
	sCmd += STR_ETHTOOL_MONITOR_ADVERTISED_AUTO_NEGOTIATION_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoAdvertisedAutoNego) - Requested Ethernet Advertised Auto Negotiation info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Convert string to Enum and Assign Value to info struct 
		ReturnValue |= EthAdvLinkAutoNegATOI(sAnswer.c_str(), pST_EthProperties);

	    //If Assignment failed
	    if (ReturnValue)
	    {
			//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoAdvertisedAutoNego) - EthAdvLinkAutoNegATOI returned error ");	
	    }
	}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////// 




/*******************************************************************************/
/* Name:      GetEthInfoAutoNego              		            	  		   */
/*                                                                             */
/* Purpose:   The Function Assigns Auto Negotiation info to Eth info struct	   */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoAutoNego(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;

	//Parameters init
	ReturnValue = 0;

    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_ETHTOOL_MONITOR_COMMAND;
      
    //Add eth number to command
	sCmd += EthX;
	
	//Add Auto Negotiation Monitoring to command
	sCmd += STR_ETHTOOL_MONITOR_AUTO_NEGOTIATION_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoAutoNego) - Requested Ethernet Auto Negotiation info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else 
	{
		//Convert string to Enum and Assign Value to info struct 
		ReturnValue |= EthLinkAutoNegATOI(sAnswer.c_str(), pST_EthProperties);	   

		//If Assignment failed
	    if (ReturnValue)
	    {
			//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoAutoNego) - EthLinkAutoNegATOI returned error ");	
	    }

	}

	return ReturnValue;
}

//////////////////////////////////////////////////////////////////////////////// 




/*******************************************************************************/
/* Name:      GetEthInfoLinkMode              		            	  		   */
/*                                                                             */
/* Purpose:   The Function Assigns link mode info to Eth info struct	       */
/* Returns:   STATUS (OK/ERROR = 0/-1)             						       */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int GetEthInfoLinkMode(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;
	string sLinkMode;

	//Parameters init
	ReturnValue = 0;

	//Get Duplex info (Half/Full)
	
    //Prepare the system command for sending
      
    //Add Monitoring to command
	sCmd = STR_ETHTOOL_MONITOR_COMMAND;
      
    //Add eth number to command
	sCmd += EthX;
	
	//Add Duplex Monitoring to command
	sCmd += STR_ETHTOOL_MONITOR_DUPLEX_COMMAND;

	//Send Command to System
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoLinkMode) - Requested Ethernet duplex info for eth: %s, System Call Failed!", EthX);	
		ReturnValue = -1;
	}
	else // if Get Duplex info succeed, get speed
	{
	 	//Copy system output (Duplex) to LinkModeStr
	 	sLinkMode = sAnswer;
	
	    //Get Speed info 
	 
	    //Add Monitoring to command
	 	sCmd = STR_ETHTOOL_MONITOR_COMMAND;
	 	 
	    //Add eth number to command
	 	sCmd += EthX;
	    
	    //Add Speed Monitoring to command
	 	sCmd += STR_ETHTOOL_MONITOR_PORT_SPEED_COMMAND;
	 
	    //Send Command to System
	    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	    if (ReturnValue)
	    {
	 	   //MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(GetEthInfoLinkMode) - Requested Ethernet port speed info for eth: %s, System Call Failed!", EthX);	
	 	   ReturnValue = -1;
	    }
	    else 
	    {
	 	   //Add speed to LinkModeStr
	    	sLinkMode += sAnswer;
		
		   //Replace Link Mode String with Enum and assign to Eth struct
		   ReturnValue |= EthLinkModeATOI(sLinkMode.c_str(), pST_EthProperties);
		}
	}

	return ReturnValue;
}

//////////////////////////////////////////////////////////////////////////////// 


/*******************************************************************************/
/* Name:      EthCheckLinkStatus              		              		   */
/*                                                                             */
/* Purpose:   The Function Checks if Link status is active in a certain NIC.   */
/* Returns:   TRUE/FALSE (Link Active/Not Active) (1,0)                        */
/*                                                                             */
/* Params:    EthX - Name of the NIC (e.g. "eth3")			   			       */
/*																			   */
/*******************************************************************************/

int EthCheckLinkStatus(const char *EthX)
{ 
	//Parameters declarations

	int ReturnValue;
	string sCmd, sAnswer;
	
	//Parameters init
	ReturnValue = 0;

	//Prepare the system command for sending

	//Add Monitoring command
	sCmd = STR_ETHTOOL_MONITOR_COMMAND;
	
	//Add eth number
	sCmd += EthX;

	//Add Link Status Monitoring command
	sCmd += STR_ETHTOOL_MONITOR_LINK_STATUS_COMMAND;

	//Issue System Command
    ReturnValue |= SystemPipedCommand(sCmd.c_str() ,sAnswer, TRUE, FALSE);
	
	//If SystemPipedCommand Failed
	if (ReturnValue)
	{
		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(EthCheckLinkStatus) - %s Asked to check link status for eth: %s, system call failed!!!", SourceName,EthX);	
	}
	else // System Call did not fail - check link status
	{
		//If Link Status Detected
		if(STR_ETHTOOL_MONITOR_LINK_STATUS_DETECTED == sAnswer)
		{	
			ReturnValue = TRUE;
		}
		//If Link Status not Detected
		else if (STR_ETHTOOL_MONITOR_LINK_STATUS_NOT_DETECTED == sAnswer)
		{	
			ReturnValue = FALSE;
		}
		//If Returned strings is not valid
		else 
		{
			ReturnValue = -1;
		}
	}

	return ReturnValue;
}


////////////////////////////////////////////////////////////////////////////////


/*****************************************************************************/
/* Name:      EthLinkModeATOI                  			              	 */
/*                                                                           */
/* Purpose:   Convert Link Mode string to  Link mode Enum    				 */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                       */
/*                                                                           */
/* Params:    Ethtool output string	         					    		 */
/*                         										    		 */
/*****************************************************************************/

int EthLinkModeATOI(const char *LinkModeStr, ETH_SETTINGS_S *pST_EthProperties)
{ 

	//Parameters declarations
	int ReturnValue;

	//Parameters init
    ReturnValue = 0;
	pST_EthProperties->ulActLinkMode = 0;

	//Choose the appropriate Link Mode and assign the right Enum
	if (!(strcmp(LinkModeStr, STR_ETHTOOL_MONITOR_LINK_MODE_10HALF)))
	{
		pST_EthProperties->ulActLinkMode = _10HdxBit;
	}
	else if (!(strcmp(LinkModeStr, STR_ETHTOOL_MONITOR_LINK_MODE_100HALF)))
	{
		pST_EthProperties->ulActLinkMode = _100HdxBit;
	}
	else if (!(strcmp(LinkModeStr, STR_ETHTOOL_MONITOR_LINK_MODE_1000HALF)))
	{
		pST_EthProperties->ulActLinkMode = _1000HdxBit;
	}
	else if (!(strcmp(LinkModeStr, STR_ETHTOOL_MONITOR_LINK_MODE_10FULL)))
	{
		pST_EthProperties->ulActLinkMode = _10FdxBit;
	}
	else if (!(strcmp(LinkModeStr, STR_ETHTOOL_MONITOR_LINK_MODE_100FULL)))
	{
		pST_EthProperties->ulActLinkMode = _100FdxBit;
	}
	else if (!(strcmp(LinkModeStr, STR_ETHTOOL_MONITOR_LINK_MODE_1000FULL)))
	{
		pST_EthProperties->ulActLinkMode = _1000FdxBit;
	}

	//If no assignment made
 	if (!(pST_EthProperties->ulActLinkMode))
 	{
 		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(EthLinkModeATOI) - Converting LinkModeStr to UINT32 failed");	
		ReturnValue = -1;
 	}

	return ReturnValue;
}


////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
/* Name:      EthLinkAutoNegATOI               			              	 */
/*                                                                           */
/* Purpose:   Convert Auto Negotiation string to  Auto-Negotiation UINT32	 */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                       */
/*                                                                           */
/* Params:    Ethtool output string            					    		 */
/*                         										    		 */
/*****************************************************************************/

int EthLinkAutoNegATOI(const char *AutoNegStr, ETH_SETTINGS_S *pST_EthProperties)
{ 

	//Parameters declarations
	int ReturnValue;

	//Parameters init
    ReturnValue = 0;
	pST_EthProperties->ulActLinkAutoNeg = 100; // just an init value that is not EMA_API_TRUE/FALSE

	//Choose the appropriate Auto Negotiation state and assign the right Enum
	if (!(strcmp(AutoNegStr, STR_ETHTOOL_MONITOR_AUTO_NEGOTIATION_ON)))
	{
		pST_EthProperties->ulActLinkAutoNeg = EMA_API_TRUE;
	}
	else if (!(strcmp(AutoNegStr, STR_ETHTOOL_MONITOR_AUTO_NEGOTIATION_OFF)))
	{
		pST_EthProperties->ulActLinkAutoNeg = EMA_API_FALSE;
	}

	//If no assignment made
 	if ( (EMA_API_TRUE != pST_EthProperties->ulActLinkAutoNeg) && (EMA_API_FALSE != pST_EthProperties->ulActLinkAutoNeg) )
 	{
 		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(EthLinkAutoNegATOI) - Converting AutoNegStr to UINT32 failed");	
		ReturnValue = -1;
 	}

	return ReturnValue;
}

//////////////////////////////////////////////////////////////////////////////


/*****************************************************************************/
/* Name:      EthAdvLinkAutoNegATOI               			             */
/*                                                                           */
/* Purpose:   Convert Advertised Auto Negotiation string to Auto-Negotiation */
/*   		  UINT32														 */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                       */
/*                                                                           */
/* Params:    Ethtool output string            					    		 */
/*                         										    		 */
/*****************************************************************************/

int EthAdvLinkAutoNegATOI(const char *AdvAutoNegStr, ETH_SETTINGS_S *pST_EthProperties)
{ 

	//Parameters declarations
	int ReturnValue;

	//Parameters init
    ReturnValue = 0;
	pST_EthProperties->ulAdvLinkAutoNeg = 100; // just an init value that is not EMA_API_TRUE/FALSE

	//Choose the appropriate Advertised Auto Negotiation and assign the right Enum
	if (!(strcmp(AdvAutoNegStr, STR_ETHTOOL_MONITOR_ADVERTISED_AUTO_NEGOTIATION_YES)))
	{
		pST_EthProperties->ulAdvLinkAutoNeg = EMA_API_TRUE;
	}
	else if (!(strcmp(AdvAutoNegStr, STR_ETHTOOL_MONITOR_ADVERTISED_AUTO_NEGOTIATION_NO)))
	{
		pST_EthProperties->ulAdvLinkAutoNeg = EMA_API_FALSE;
	}

	//If no assignment made
 	if ( (EMA_API_TRUE != pST_EthProperties->ulAdvLinkAutoNeg) && (EMA_API_FALSE != pST_EthProperties->ulAdvLinkAutoNeg) )
 	{
 		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(EthAdvLinkAutoNegATOI) - Converting AdvAutoNegStr to UINT32 failed");	
		ReturnValue = -1;
 	}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////////

/*****************************************************************************/
/* Name:      EthAdvLinkModeATOI               			                 */
/*                                                                           */
/* Purpose:   Convert Advertised Link Mode string to Link Mode UINT32 	     */
/* Returns:   STATUS (OK/ERROR = 0/-1)                                       */
/*                                                                           */
/* Params:    Ethtool output string            					    		 */
/*                         										    		 */
/*****************************************************************************/

int EthAdvLinkModeATOI(const char *AdvLinkModeStr, ETH_SETTINGS_S *pST_EthProperties)
{ 

	//Parameters declarations
	int ReturnValue;

	//Parameters init
    ReturnValue = 0;
	pST_EthProperties->ulAdvLinkMode = 0;

 	//The following code section checks if certain strings are included in AdvLinkModeStr. 
	// For each string included, ulAdvLinkMode is updated (using bit wise)

 	if (strstr(AdvLinkModeStr, STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_10_HALF))
 	{
 		pST_EthProperties->ulAdvLinkMode |= _10HdxBit;
 	}
 	if (strstr(AdvLinkModeStr, STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_100_HALF))
 	{
 		pST_EthProperties->ulAdvLinkMode |= _100HdxBit;
 	}
 	if (strstr(AdvLinkModeStr, STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_1000_HALF))
 	{
 		pST_EthProperties->ulAdvLinkMode |= _1000HdxBit;
 	}
 	if (strstr(AdvLinkModeStr, STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_10_FULL))
 	{
 		pST_EthProperties->ulAdvLinkMode |= _10FdxBit;
 	}
 	if (strstr(AdvLinkModeStr, STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_100_FULL))
 	{
 		pST_EthProperties->ulAdvLinkMode |= _100FdxBit;
 	}
 	if (strstr(AdvLinkModeStr, STR_ETHTOOL_MONITOR_ADVERTISED_LINK_MODE_1000_FULL))
 	{
 		pST_EthProperties->ulAdvLinkMode |= _1000FdxBit;
 	}

	//If no assignment made
	if (!(pST_EthProperties->ulAdvLinkMode))
 	{
 		//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"(EthAdvLinkModeATOI) - Converting AdvLinkModeStr to UINT32 failed");	
		ReturnValue = -1;
 	}

	return ReturnValue;
}

////////////////////////////////////////////////////////////////////////////// 

/*****************************************************************************/
/* Name:      EthUpdateMaxValues               			                 */
/*                                                                           */
/* Purpose:   Update the maximum Statistical values on Ethernet properties   */
/*            struct  	  													 */
/* Returns:   NONE					                                         */
/*                                                                           */
/* Params:    NONE				             					    		 */
/*                         										    		 */
/*****************************************************************************/

void EthUpdateMaxValues(ETH_SETTINGS_S *pST_EthProperties)
{ 

//update all max values

	//Transmit Max Values
    // Error Packets
    if(pST_EthProperties->ulMaxTxBadPackets < pST_EthProperties->ulTxBadPackets)
	{
		pST_EthProperties->ulMaxTxBadPackets = pST_EthProperties->ulTxBadPackets;
	}

    // FIFO Drops
    if(pST_EthProperties->ulMaxTxFifoDrops < pST_EthProperties->ulTxFifoDrops)
	{
		pST_EthProperties->ulMaxTxFifoDrops = pST_EthProperties->ulTxFifoDrops;
	}

    //Number of Octets
    if(pST_EthProperties->ulMaxTxOctets < pST_EthProperties->ulTxOctets)
	{
		pST_EthProperties->ulMaxTxOctets = pST_EthProperties->ulTxOctets;
	}

    //Receive Max Values
    //CRC
    if(pST_EthProperties->ulMaxRxCRC < pST_EthProperties->ulRxCRC)
	{
		pST_EthProperties->ulMaxRxCRC = pST_EthProperties->ulRxCRC;
	}

    //Error Packets
    if(pST_EthProperties->ulMaxRxBadPackets < pST_EthProperties->ulRxBadPackets)
	{
		pST_EthProperties->ulMaxRxBadPackets = pST_EthProperties->ulRxBadPackets;
	}
    	
    //Number of Octets
    if(pST_EthProperties->ulMaxRxOctets < pST_EthProperties->ulRxOctets)
	{
		pST_EthProperties->ulMaxRxOctets = pST_EthProperties->ulRxOctets;
	}

	return;
}

int GetEthInfo802_1xStatus(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
{

    #define BUFFER_LEN (256)
    UINT32 ulRc = STATUS_OK;
    std::string cLineBuffer;
    INT8 cCommandLine[BUFFER_LEN] = {0};
    INT8 cEthName[BUFFER_LEN] = {0};


    STATUS stat;
    std::string answer = "";
	
    stat = SystemPipedCommand("ps | grep 'wpa_supplicant' ",answer);
    
    if (stat != STATUS_OK)
    {

    	FPTRACE(eLevelInfoNormal,"GetEthInfo802_1xStatus: - failed to check if wpa_supplicant run " );
    	return ulRc;
    }


    snprintf(cEthName, sizeof(cEthName), "-i%s", EthX);

    //wpa_supplicant is running
    if (strstr(answer.c_str(), cEthName))
    {
    	snprintf(cCommandLine, sizeof(cCommandLine), "/usr/sbin/wpa_cli -i%s status", EthX);

    	//TRACESTR(eLevelInfoNormal) <<"\nGetEthInfo802_1xStatus: - cCommandLine  "<<cCommandLine;
    	//FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xStatus: - cCommandLine ", cCommandLine );
    	FTRACECOND(true,cCommandLine);

    	STATUS stat = SystemPipedCommand(cCommandLine ,cLineBuffer, TRUE, FALSE);


    	if(stat != STATUS_OK) {

    		FPTRACE(eLevelInfoNormal,"GetEthInfo802_1xStatus:- SystemPipedCommand failed ");
    		ulRc = -1;
    		return ulRc;
    	}


    	//search for the error case first
    	if(strstr(cLineBuffer.c_str(), P802_1x_STR_FAILED_TO_CONNECT) != NULL) {
    		//we got an error
    		//TRACESTR(eLevelInfoNormal) <<"\nGetEthInfo802_1xStatus: - we got %s. usually means that wpa_supplicant isnt running: "<<cLineBuffer;
    		FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xStatus:- wpa_supplicant isnt running: ", cLineBuffer.c_str() );
    		ulRc = -1;
    		return ulRc;
    	}

    	//now search for: 'suppPortStatus='
    	if(strstr(cLineBuffer.c_str(), P802_1x_STR_SUP_PORT_STAT) != NULL)
    	{
    		//we got the line, now parse out the value into the variable sWpaCliParseRes
    		if(strstr(cLineBuffer.c_str(), P802_1x_STR_UNAUTHORIZED) != NULL) {

    			//802.1x
    			pST_EthProperties->e802_1xSuppPortStatus = E_802_1x_PORT_STATUS_FAILED;


    		}
    		else if(strstr(cLineBuffer.c_str(), P802_1x_STR_AUTHORIZED) != NULL) {

    			pST_EthProperties->e802_1xSuppPortStatus = E_802_1x_PORT_STATUS_AUTHENTICATED;

    		}
    		else {

    			pST_EthProperties->e802_1xSuppPortStatus = E_802_1x_PORT_STATUS_INVALID;
    			//TRACESTR(eLevelInfoNormal) <<"\nGetEthInfo802_1xStatus: - we got %s. INVALID VALUE\n: "<<cLineBuffer;
    			FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xStatus:- INVALID VALUE ", cLineBuffer.c_str() );

    			ulRc = -1;
    			return ulRc;
    		}
    	}

    	//now check if we need to fill in a failure reason
    	if(pST_EthProperties->e802_1xSuppPortStatus  != E_802_1x_PORT_STATUS_AUTHENTICATED) {
    		//for now i just put a value to have something. still need to learn to parse this part.
    		pST_EthProperties->e802_1xFailReason     = E_802_1x_FR_BAD_CONFIGURATION;
    	}
    	else {
    		//this signifies that there is no failure reason since we didnt fail ;)

    		pST_EthProperties->e802_1xFailReason     = E_802_1x_FR_OFF;
    	}




    }
    return ulRc;

}



int GetEthInfo802_1xMethodType(const char *EthX, ETH_SETTINGS_S *pST_EthProperties)
  {

  	int ReturnValue;

  INT8 caConfStr[WPA_CONF_BUFFER_MAX_LENGTH]= {0};
  INT8 caConfFileName[P802_1x_FILES_PATH_MAX_LENGTH] = {0};
  FILE* pFile = NULL;

  std::string fname;
  if (strcmp(EthX ,"bond0:1") == 0)
  {
	  fname = MCU_TMP_DIR+"/802_1xCtrl/wpa_eth1.conf";
	  strncpy(caConfFileName, fname.c_str(), sizeof(caConfFileName)-1);
  }
  else  if (strcmp(EthX ,"bond0:2") == 0)
  {
	  fname = MCU_TMP_DIR+"/802_1xCtrl/wpa_eth2.conf";
	  strncpy(caConfFileName, fname.c_str(), sizeof(caConfFileName)-1);
  }
  else
  {
	  fname =  MCU_TMP_DIR+"/802_1xCtrl/wpa_%s.conf";
      snprintf(caConfFileName, sizeof(caConfFileName)-1,fname.c_str(), EthX);
  }

 // FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xMethodType: - caConfFileName ", caConfFileName );


  if((pFile = fopen(caConfFileName, "r")) != NULL)
  {
  	long lSize;
  	size_t result;



  	// obtain file size:
  	fseek (pFile , 0 , SEEK_END);
  	lSize = ftell (pFile);
  	rewind (pFile);



  	result = fread (caConfStr,1,lSize,pFile);

//FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xStatus: - caConfStr ", caConfStr);



		if(strstr(caConfStr, P802_1x_STR_METHOD_TYPE_FROM_CONF_FILE) != NULL)
		{
			//we got the line, now parse out the value into the method types
			if(strstr(caConfStr, P802_1x_STR_METHOD_MD5_TYPE_FROM_CONF_FILE) != NULL) {

				//802.1x
				pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_EAP_MD5;


			}
			else if(strstr(caConfStr, P802_1x_STR_METHOD_CHAP_TYPE_FROM_CONF_FILE) != NULL) {

				pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_PEAP_MSCHAPV2;
			}

			else if(strstr(caConfStr, P802_1x_STR_METHOD_TLS_TYPE_FROM_CONF_FILE) != NULL) {

				pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_EAP_TLS;
			}
			else {

				pST_EthProperties->e802_1xMethod = E_802_1x_METHOD_OFF;

			}
		}

  }
  else
	  {
		FPTRACE2(eLevelInfoNormal,"GetEthInfo802_1xMethodType: - errorno ", strerror(errno) );
	     return -1;
	  }
  if(pFile)
  	fclose(pFile);

  return 0;

  }



