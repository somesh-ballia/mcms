/*==================================================================================================*/
/*            Copyright     ???    2001  Polycom Israel, Ltd. All rights reserved                   */
/*--------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of             */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form   */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without         */
/* prior written authorization from Polycom Israel Ltd.                                             */
/*--------------------------------------------------------------------------------------------------*/
/*                                                                                                  */
/* FILE:     	IpmiHandler.c																	    */
/* PROJECT:  	McmsSim                                                                             */
/* PROGRAMMER:  Edwin Dong                                                                     */
/*                                                                                                  */
/*--------------------------------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                                                  */
/*--------------------------------------------------------------------------------------------------*/
/*         |                 |  			                                                        */
/*==================================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "DiagDataTypes.h"  
#include "SharedDefines.h"
#include "McmsApi.h"
#include "SystemInfo.h"
#include "IpmiShared.h"
//#include "CardsStructs.h"
#include "SocketApiTypes.h" 
#include "LinuxSystemCallsApi.h"
#include "SocketApiWrapExt.h"
#include "StatusesGeneral.h"
#include "Print.h"
#include "DbgCfg.h"
#include "EmaShared.h"
#include "timers.h"
#include "tools.h"
#include "McmsCom.h"
#include "CardContent.h"
#include "GetCntlBoardGeneralInfo.h"
#include "IpmiEntitySlotIDs.h"
#include "LanHandleThread.h"
#include "GetIFList.h"

#define LANDEBUG 0

extern void TraceSwitchEmaCom(eTraceDiagSwitchEmaComLocation place,
		e_TcpConn  eID,
                   UINT32 ulMsgId,
                   UINT32 opcode,
                   UINT32 ulSlotId,
                   const char *message);

extern unsigned int currentLogLoopNumber;

extern t_TcpConnParams	TcpConnection[eMaxTcpConnections];

//Lan Stat Get Ports List
static TStructToStr atLanStatGetPortsListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "PortID"},
	{e_unsignedLong,   1,              4,        "Status"},
};

void BuildLanStatGetPortsListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TLanStatGetPortsList* 				ptLanStatGetPortsList;

	if(LANDEBUG) printf("BuildLanStatGetPortsListIndMsg\n");

	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atLanStatGetPortsListIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TLanStatGetPortsList);
	
	// Allocate message	pointer
	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atLanStatGetPortsListIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atLanStatGetPortsListIndSpecArray,sizeof(atLanStatGetPortsListIndSpecArray));

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atLanStatGetPortsListIndSpecArray))/4;

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= LAN_STAT_GET_PORTS_LIST_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


       //Prepare and fill fan level list
	ptLanStatGetPortsList = (TLanStatGetPortsList*)currentMsgPointer;
	GetLanStatPortsList(ptLanStatGetPortsList);   


   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildLanStatGetPortsListIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(LAN_SWITCH_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eLanStatServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eLanStatClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildLanStatGetPortsListIndMsg): problem in sending data...\n");
	    TcpConnection[eLanStatClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eLanStatClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}

done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


//Lan Stat Info
static TStructToStr atLanStatInfoIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "RxPackets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "RxBadPackets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "RxCRC"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "RxOctets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "MaxRxPackets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "MaxRxBadPackets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "MaxRxCRC"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "MaxRxOctets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "TxPackets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "TxBadPackets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "TxFifoDrops"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "TxOctets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "MaxTxPackets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "MaxTxBadPackets"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "MaxTxFifoDrops"},
	{e_string,		   1,   MAX_PORT_INFO_STR_SIZE,        "MaxTxOctets"},
	{e_unsignedLong,   1,              4,        "ActLinkStatus"},
	{e_unsignedLong,   1,              4,        "ActLinkMode"},
	{e_unsignedLong,   1,              4,        "ActLinkAutoNeg"},
	{e_unsignedLong,   1,              4,        "AdvLinkMode"},
	{e_unsignedLong,   1,              4,        "AdvLinkAutoNeg"},
};

void BuildLanStatInfoIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TLanStatInfo* 				ptLanStatInfo;

	if(LANDEBUG) printf("BuildLanStatInfoIndMsg\n");

	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atLanStatInfoIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TLanStatInfo);
	
	// Allocate message	pointer
	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atLanStatInfoIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atLanStatInfoIndSpecArray,sizeof(atLanStatInfoIndSpecArray));

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atLanStatInfoIndSpecArray))/4;

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= LAN_STAT_INFO_IND;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


       //Prepare and fill fan level list
	ptLanStatInfo = (TLanStatInfo*)currentMsgPointer;
	GetLanStatInfo(ptLanStatInfo, ulSlotID - LAN_SLOT_ID_START);


   	MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildLanStatInfoIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(LAN_SWITCH_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eLanStatServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eLanStatClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildLanStatInfoIndMsg): problem in sending data...\n");
	    TcpConnection[eLanStatClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eLanStatClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


//thread
void LanHandleThread()
{

	t_TcpConnParams*	ptConParams = &TcpConnection[eLanStatClient];
	TEmaReqHeader*		ptEmaReqHeader;

	SOCKET				clientSocket;
	UINT8				*uc_RcvBuf;
	UINT32				ul_BytesReceived;
	TMessageThreadType	tMessageThreadType;

	EnrollInThreadList(eLanHandleThread);

      MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"LanHandleThread");
	printf("\n******* LAN HANDLER STARTING TO RUN ******** \n");

	// connect to SwitchDiag TCP Server...

	ptConParams->e_Id 		= eLanStatClient;
	ptConParams->us_Port 	= LISTEN_LAN_STAT_PORT;
	ptConParams->ul_ClientOrServer = eConnTypeClient;
	strcpy(&ptConParams->IpV4Addr.auc_IpV4Address[0]   , "127.0.0.1"	);
	
	// Connect as client to SwitchDiagServer
	ConnectToTcpServer(ptConParams);

	while(1)
	{

		// Handle ipmi requests (from SwitchConnection ....)
		// Receive message, get opcode and build a state machine to handle the requests)
		MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
					  "LanHandleThread: main loop");

	if (ptConParams->ul_ConnectionStatus == CONNECTED)
     	{       
			clientSocket = ptConParams->s;

			MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   			"LanHandleThread: clientSocket = %d \n", clientSocket);
		
			//printf("socket blocked with data\n");
			uc_RcvBuf = NULL;
     			ul_BytesReceived = TCPRecvData(clientSocket, (VOID **)&uc_RcvBuf, 0);     	
        	    //printf("socket %d unblocked with data , bytes received = %d\n",clientSocket,ul_BytesReceived);
			if(ul_BytesReceived == NO_DATA)
			{
				if(uc_RcvBuf != NULL)
				{
					free(uc_RcvBuf);
				}
				continue;
			}
       	/*
		while (!isMessageFromEmaToIpmiReady())
        	{
            		EmbSleep(100);
       	}
            
            uc_RcvBuf = malloc(MAX_MSG_EMA_SIZE);
            rcvMsgFromEmaToIpmi(&eID, &ul_BytesReceived,&(uc_RcvBuf[0]));*/
            
   		if ((ul_BytesReceived == (UINT32)SOCKET_OPERATION_FAILED) || (ul_BytesReceived == (UINT32)TPCKT_OPERATION_FAILED))
   		{
			MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   			"LanHandleThread: problem receiving data \n");
			if(uc_RcvBuf != NULL)
			{
				free(uc_RcvBuf);
			}
		}
   		else
   		{
   			
			tMessageThreadType.ulData = (UINT32)uc_RcvBuf;  
			tMessageThreadType.ulSize = ul_BytesReceived;

			MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
		       			"LanHandleThread: Received data from switch \n");

			ptEmaReqHeader = (TEmaReqHeader*)tMessageThreadType.ulData;
			if((size_t)ul_BytesReceived < sizeof(TEmaReqHeader))
			{
				MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"LanHandleThread: problem receiving data ul_BytesReceived < sizeof(TEmaReqHeader) \n");
				free(ptEmaReqHeader);
				ptEmaReqHeader = NULL;
				continue;
			}
			
			UINT32 ulOpcode = ptEmaReqHeader->ulOpcode;
			UINT32 ulMsgId  = ptEmaReqHeader->ulMsgID;
			UINT32 ulSlotId = ptEmaReqHeader->ulSlotID;

			MfaBoardPrint(	LAN_SWITCH_PRINT,
							PRINT_LEVEL_ERROR,
							PRINT_TO_TERMINAL,
							"(LanHandleThread): EmaReqHeader:: eID:%d, opcode:%Xh, msgID:%d, slotID:%d.\n",
							eLanStatClient,
							ulOpcode,
							ulMsgId,
							ulSlotId);	

           		TraceSwitchEmaCom(eHandle, eLanStatClient, ulMsgId, ulOpcode, ulSlotId, "None");

			switch(ulOpcode)
			{
				case PRIVATE_OPCODE:
					
					MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"LanHandleThread: we are in PRIVATE_OPCODE \n");
				     break;
				case LAN_STAT_GET_PORTS_LIST_REQ:
					BuildLanStatGetPortsListIndMsg(ulMsgId, ulSlotId);
					break;
				case LAN_STAT_INFO_REQ:
					BuildLanStatInfoIndMsg(ulMsgId, ulSlotId);
					break;
					
				default:
					MfaBoardPrint(	LAN_SWITCH_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"(LanHandleThread): Unknown Opcode !!! (%Xh)",ulOpcode);
					printf(	"(LanHandleThread): Unknown Opcode !!! (%Xh)",ulOpcode);		
					break;
			}

			
			if(ptEmaReqHeader != NULL)
			{
				free(ptEmaReqHeader);
				ptEmaReqHeader = NULL;
			}

   		}//if (ul_BytesReceived == SOCKET_OPERTATION_FAILED) 
   	}   
     	else
     	{
			MfaBoardPrint(LAN_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
						  "DiagnosticsMain: main loop. Wait for connection");
			printf("Wait for connection\n");
	 		EmbSleep(30);
     	}
   	}
}

