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

#define IPMIDEBUG 0

extern void TraceSwitchEmaCom(eTraceDiagSwitchEmaComLocation place,
				e_TcpConn  eID,
                           UINT32 ulMsgId,
                           UINT32 opcode,
                           UINT32 ulSlotId,
                           const char *message);

extern unsigned int currentLogLoopNumber;

extern t_TcpConnParams	TcpConnection[eMaxTcpConnections];

// list indication message
static TStructToStr atIpmiEntityListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "SubBoardID"},
	{e_unsignedLong,   1,              4,        "IpmbAddress"},
	{e_string,		   1,   MAX_CARD_TYPE_STR_SIZE,   "CardType"},
	{e_unsignedLong,   1,              4,        "NumMezzanine"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_unsignedLong,   1,              4,        "Temperature"},
	{e_unsignedLong,   1,              4,        "Voltage"},
	{e_unsignedLong,   1,              4,        "BitFail"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "EntityId"},
	{e_unsignedLong,   1,              4,        "EntityInstance"},
	{e_unsignedLong,   1,              4,        "Present"},
};

void PrintEntityListInfo(const TIpmiEntityData * EntityData)
{

    printf("SlotID %d, SubBoardID %d, IpmbAddress %d, CardType %s, NumMezzanine %d, Status %d, Temperature %d, Voltage %d, BitFail %d, NumOfElem %d, NumOfElemFields %d,\n",
	EntityData->unSlotID,
	EntityData->unSubBoardID,
	EntityData->unIpmbAddress,
	EntityData->unCardType,
	EntityData->unNumMezzanine,
	EntityData->unStatus,
	EntityData->unTemperature,
	EntityData->unVoltage,
	EntityData->unBitFail,
	EntityData->ulNumOfElem,
	EntityData->ulNumOfElemFields
	);
}

void BuildIpmiEntityListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TIpmiEntityListData* 				ptEntityList;
	TIpmiEntityListData		entityList;
	int reportedArrayIndex = 0;

	if(IPMIDEBUG) printf("BuildIpmiEntityListIndMsg\n");
	
	// Allocate message	pointer

	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atIpmiEntityListIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TIpmiEntityListData);
	
	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atIpmiEntityListIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atIpmiEntityListIndSpecArray,sizeof(atIpmiEntityListIndSpecArray));

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atIpmiEntityListIndSpecArray))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= IPMI_ENTITY_LIST_REQ;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


       //Prepare entityList
	CollectCardContents(&entityList);

	// Fill entity list data
	ptEntityList = (TIpmiEntityListData*)currentMsgPointer;

	ptEntityList->ulNumOfElem = entityList.ulNumOfElem;
	ptEntityList->ulNumOfElemFields = entityList.ulNumOfElemFields;
	
	if(IPMIDEBUG) printf("started combining entity \n");
	reportedArrayIndex = 0;
	for (i=0; i<entityList.ulNumOfElem; ++i)
	{
		ptEntityList->tEntityData[reportedArrayIndex] = entityList.tEntityData[reportedArrayIndex];
		//ptEntityList->tEntityData[reportedArrayIndex].unStatus = IPMI_CARD_STATUS_DIAGNOSTICS;
		//TODO:
		if(ptEntityList->tEntityData[reportedArrayIndex].unSlotID >= CNTL_SLOT_ID &&
			ptEntityList->tEntityData[reportedArrayIndex].unSlotID <= ISDN_CARD_SLOT_ID ) ptEntityList->tEntityData[reportedArrayIndex].unStatus = IPMI_CARD_STATUS_DIAGNOSTICS;
		if(IPMIDEBUG) PrintEntityListInfo(&(ptEntityList->tEntityData[reportedArrayIndex]));
		reportedArrayIndex++;
	}

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildIpmiEntityListIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(IPMI_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eIpmiServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eIpmiClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildIpmiEntityListIndMsg): problem in sending data...\n");
	    TcpConnection[eIpmiClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eIpmiClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


//FAN INFO
static TStructToStr atIpmiFanInfoIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "MinSpeedLevel"},
	{e_unsignedLong,   1,              4,        "MaxSpeedLevel"},
	{e_unsignedLong,   1,              4,        "NormalOperatingLevel"},
};

void BuildIpmiFanInfoIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TIpmiFanInfo* 				ptFanInfo;

	if(IPMIDEBUG) printf("BuildIpmiFanInfoIndMsg\n");
	
	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atIpmiFanInfoIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TIpmiFanInfo);

	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atIpmiFanInfoIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atIpmiFanInfoIndSpecArray,sizeof(atIpmiFanInfoIndSpecArray));

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atIpmiFanInfoIndSpecArray))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= IPMI_FAN_INFO_REQ;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


	// Fill Fan Info
	ptFanInfo = (TIpmiFanInfo*)currentMsgPointer;
	ptFanInfo->unMinSpeedLevel = 0;
	ptFanInfo->unMaxSpeedLevel = 4;
	ptFanInfo->unNormalOperatingLevel = 2;


   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildIpmiFanInfoIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(IPMI_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eIpmiServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eIpmiClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildIpmiFanInfoIndMsg): problem in sending data...\n");
	    TcpConnection[eIpmiClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eIpmiClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


//FAN LEVEL
static TStructToStr atIpmiFanLevelIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "FanLevel"},
};

void BuildIpmiFanLevelIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TIpmiFanLevel* 				ptFanLevelList;

	if(IPMIDEBUG) printf("BuildIpmiFanLevelIndMsg\n");

	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atIpmiFanLevelIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TIpmiFanLevel);
	
	// Allocate message	pointer
	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atIpmiFanLevelIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atIpmiFanLevelIndSpecArray,sizeof(atIpmiFanLevelIndSpecArray));

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atIpmiFanLevelIndSpecArray))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= IPMI_GET_FAN_LEVEL_REQ;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


       //Prepare and fill fan level list
	ptFanLevelList = (TIpmiFanLevel*)currentMsgPointer;
	UpdateFanLevel(ptFanLevelList);


   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildIpmiFanLevelIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(IPMI_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eIpmiServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eIpmiClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildIpmiFanLevelIndMsg): problem in sending data...\n");
	    TcpConnection[eIpmiClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eIpmiClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}



//FAN INFO
static TStructToStr atIpmiEventLogIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "RecordIdHi"},
	{e_unsignedLong,   1,              4,        "RecordIdLo"},
	{e_unsignedLong,   1,              4,        "RecordType"},
	{e_unsignedLong,   1,              4,        "Timestamp"},
	{e_unsignedLong,   1,              4,        "IpmbSlaveAddr"},
	{e_unsignedLong,   1,              4,        "ChannelNumber"},
	{e_unsignedLong,   1,              4,        "EvmRev"},
	{e_unsignedLong,   1,              4,        "SensorType"},
	{e_unsignedLong,   1,              4,        "SensorNum"},
	{e_unsignedLong,   1,              4,        "EventDirType"},
	{e_unsignedLong,   1,              4,        "EventData1"},
	{e_unsignedLong,   1,              4,        "EventData2"},
	{e_unsignedLong,   1,              4,        "EventData3"},
	{e_string,		   1,   MAX_SENSOR_DESC_STR_SIZE,   "SensorDescr"},
};

	
void BuildIpmiEventLogIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TIpmiEventLogList* 				ptEventLogList;

	if(IPMIDEBUG) printf("BuildIpmiEventLogIndMsg\n");

	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atIpmiEventLogIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TIpmiEventLogList);

	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atIpmiEventLogIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atIpmiEventLogIndSpecArray,sizeof(atIpmiEventLogIndSpecArray));

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atIpmiEventLogIndSpecArray))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= IPMI_GET_EVENT_LOG_REQ;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


	// Fill Event Log
	ptEventLogList = (TIpmiEventLogList*)currentMsgPointer;
	ptEventLogList->ulNumOfElem = 1;
	ptEventLogList->ulNumOfElemFields = 14;

	strcpy(ptEventLogList->tEventLogs[0].unSensorDescr, "FAN 3");
	ptEventLogList->tEventLogs[0].unRecordIdHi = 0;
	ptEventLogList->tEventLogs[0].unRecordIdLo = 137;
	ptEventLogList->tEventLogs[0].unRecordType = 0;
	ptEventLogList->tEventLogs[0].unTimestamp = time(0);
	ptEventLogList->tEventLogs[0].unIpmbSlaveAddr = 32;
	ptEventLogList->tEventLogs[0].unChannelNumber = 0;
	ptEventLogList->tEventLogs[0].unEvmRev = 4;
	ptEventLogList->tEventLogs[0].unSensorType = 4;
	ptEventLogList->tEventLogs[0].unSensorNum = 13;
	ptEventLogList->tEventLogs[0].unEventDirType = 1;
	ptEventLogList->tEventLogs[0].unEventData1 = 7;
	ptEventLogList->tEventLogs[0].unEventData2 = 255;
	ptEventLogList->tEventLogs[0].unEventData3 = 255;


   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildIpmiEventLogIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(IPMI_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eIpmiServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eIpmiClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildIpmiEventLogIndMsg): problem in sending data...\n");
	    TcpConnection[eIpmiClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eIpmiClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


//FRU
static TStructToStr atIpmiFruIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "SubBoardID"},
	{e_unsignedLong,   1,              4,        "BoardMfgDateTime"},
	{e_unsignedLong,   1,              4,        "BoardManufacturerType"},
	{e_string,		   1,   MAX_PRODUCT_NAME_STR_SIZE,   "BoardProductName"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardSerialNumber"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardPartNumber"},
	{e_string,		   1,   MAX_FILE_ID_STR_SIZE,        "BoardFileId"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardHardwareVers"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardMacAddr1"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardMacAddr2"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardMacAddr3"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardMacAddr4"},
	{e_unsignedLong,   1,              4,        "BoardDspClockSpeed"},
	{e_unsignedLong,   1,              4,        "ChassisType"},
	{e_unsignedLong,   1,              4,        "ChassisPartNumber"},
	{e_unsignedLong,   1,              4,        "ChassisSerialNumber"},
	{e_unsignedLong,   1,              4,        "ChassisFileId"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "ChassisHwVersion"},	
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "MezzanineNumber"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardHardwareVers"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardSerialNumber"},
	{e_string,		   1,   MAX_VERSION_STR_SIZE,        "BoardPartNumber"},
};

void BuildIpmiFruIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TIpmiFru* 				ptFruList;

	if(IPMIDEBUG) printf("BuildIpmiFruIndMsg\n");


	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atIpmiFruIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TIpmiFru);
	
	// Allocate message	pointer
	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atIpmiFruIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atIpmiFruIndSpecArray,sizeof(atIpmiFruIndSpecArray));

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atIpmiFruIndSpecArray))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= IPMI_FRU_REQ;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


       //Prepare and fill fan level list
	ptFruList = (TIpmiFru*)currentMsgPointer;

	if(CNTL_SLOT_ID == ulSlotID)
	{
		GetCntlBoardGeneralInfo(ptFruList);		
	}
	else if(ISDN_CARD_SLOT_ID == ulSlotID)
	{
		GetRTMBoardGeneralInfo(ptFruList);		
	}
	else
	{
		GetDSPCardGeneralInfo(ptFruList);
	}



   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildIpmiFanLevelIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(IPMI_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eIpmiServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eIpmiClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildIpmiFruIndMsg): problem in sending data...\n");
	    TcpConnection[eIpmiClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eIpmiClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


//Sensor List
static TStructToStr atIpmiSensorListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "SensorNumber"},
	{e_unsignedLong,   1,              4,        "EntityId"},
	{e_unsignedLong,   1,              4,        "EntityInstance"},
	{e_unsignedLong,   1,              4,        "SensorType"},
	{e_unsignedLong,   1,              4,        "EventReadType"},
	{e_string,		   1,   MAX_FLOAT_STR_SIZE,        "NominalVal"},
	{e_string,		   1,   MAX_FLOAT_STR_SIZE,        "UpperNonRecoverable"},
	{e_string,		   1,   MAX_FLOAT_STR_SIZE,        "UpperCritical"},
	{e_string,		   1,   MAX_FLOAT_STR_SIZE,        "UpperNonCritical"},
	{e_string,		   1,   MAX_FLOAT_STR_SIZE,        "LowerNonRecoverable"},
	{e_string,		   1,   MAX_FLOAT_STR_SIZE,        "LowerCritical"},
	{e_string,		   1,   MAX_FLOAT_STR_SIZE,        "LowerNonCritical"},
	{e_string,		   1,   MAX_SENSOR_DESC_STR_SIZE,        "SensorDescr"},
};

void BuildIpmiSensorListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TIpmiSensorList* 				ptSensorList;

	if(IPMIDEBUG) printf("BuildIpmiSensorListIndMsg\n");

	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atIpmiSensorListIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TIpmiSensorList);
	
	// Allocate message	pointer
	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atIpmiSensorListIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atIpmiSensorListIndSpecArray,sizeof(atIpmiSensorListIndSpecArray));

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atIpmiSensorListIndSpecArray))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= IPMI_SENSOR_LIST_REQ;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


       //Prepare and fill fan level list
	ptSensorList = (TIpmiSensorList*)currentMsgPointer;

	   
	GetSensorList(ulSlotID, ptSensorList);


   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildIpmiSensorListIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(IPMI_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eIpmiServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eIpmiClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildIpmiSensorListIndMsg): problem in sending data...\n");
	    TcpConnection[eIpmiClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eIpmiClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


//Sensor Reading List
static TStructToStr atIpmiSensorReadingListIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "SensorNumber"},
	{e_string,		   1,   MAX_FLOAT_STR_SIZE,        "SensorReading"},
	{e_unsignedLong,   1,              4,        "SensorState1"},
	{e_unsignedLong,   1,              4,        "SensorState2"},
};

void BuildIpmiSensorReadingListIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TIpmiSensorReadingList* 				ptSensorReadingList;

	if(IPMIDEBUG) printf("BuildIpmiSensorReadingListIndMsg\n");


	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atIpmiSensorReadingListIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TIpmiSensorReadingList);
	
	// Allocate message	pointer
	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atIpmiSensorReadingListIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atIpmiSensorReadingListIndSpecArray,sizeof(atIpmiSensorReadingListIndSpecArray));

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atIpmiSensorReadingListIndSpecArray))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= IPMI_SENSOR_READING_LIST_REQ;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


       //Prepare and fill fan level list
	ptSensorReadingList = (TIpmiSensorReadingList*)currentMsgPointer;

	   
	GetSensorReadingList(ulSlotID, ptSensorReadingList);


   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildIpmiSensorReadingListIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(IPMI_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eIpmiServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eIpmiClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildIpmiSensorReadingListIndMsg): problem in sending data...\n");
	    TcpConnection[eIpmiClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eIpmiClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}


//Reset
static TStructToStr atIpmiResetIndSpecArray[]=
{
	{e_unsignedLong,   1,              4,        "Opcode"},
	{e_unsignedLong,   1,              4,        "MsgID"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
	{e_string,		   1,   MAX_DESC_STR_SIZE,   "Description"},
	{e_unsignedLong,   1,              4,        "NumOfElem"},
	{e_unsignedLong,   1,              4,        "NumOfElemFields"},
	{e_unsignedLong,   1,              4,        "SlotID"},
	{e_unsignedLong,   1,              4,        "Status"},
};

void BuildIpmiResetIndMsg(UINT32 ulMsgId, UINT32 ulSlotID)
{

	UINT32					*currentMsgPointer, *pMsg;
	TSpecGnrlHdr	*ptEmaIndGeneralDescHdr;
	TEmaIndHeader	*ptEmaIndHdr;
	UINT32					unIndBufSize;
	UINT32					i;
	TIpmiResetList* 				ptResetList;

	if(IPMIDEBUG) printf("BuildIpmiResetIndMsg\n");

	unIndBufSize = sizeof(TSpecGnrlHdr) +
				   sizeof(atIpmiResetIndSpecArray) +
				   sizeof(TEmaIndHeader) +  sizeof(TIpmiResetList);
	
	// Allocate message	pointer
	pMsg = currentMsgPointer = (UINT32*)malloc(unIndBufSize);

	if(NULL == currentMsgPointer) goto done;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (message begin)", currentMsgPointer);
	// Fill General Header
	ptEmaIndGeneralDescHdr = (TSpecGnrlHdr*)currentMsgPointer;
	ptEmaIndGeneralDescHdr->ulMsgOffset= sizeof(TSpecGnrlHdr) + sizeof(atIpmiResetIndSpecArray);

	currentMsgPointer += (sizeof(TSpecGnrlHdr))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after general header)", currentMsgPointer);

	// Fill Struct Specification Header
	memcpy((void*)currentMsgPointer,(void*)atIpmiResetIndSpecArray,sizeof(atIpmiResetIndSpecArray));

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer value = %d (after general header first value)", currentMsgPointer);

	currentMsgPointer += (sizeof(atIpmiResetIndSpecArray))/4;

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after structure header)", currentMsgPointer);

	// Fill Indication Message
	// Fill Message Header
	ptEmaIndHdr 					= (TEmaIndHeader*)currentMsgPointer;
	ptEmaIndHdr->ulOpcode			= IPMI_DO_RESET_REQ;
	ptEmaIndHdr->ulMsgID			= ulMsgId;
	ptEmaIndHdr->ulSlotID			= ulSlotID;
	ptEmaIndHdr->ulStatus			= eEmaStatOk;
	strncpy(ptEmaIndHdr->acDesc, "OK", sizeof(ptEmaIndHdr->acDesc) - 1);

	currentMsgPointer += (sizeof(TEmaIndHeader)/4);

   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
	"currentMsgPointer = %d (after indication header)", currentMsgPointer);


       //Prepare and fill fan level list
	ptResetList = (TIpmiResetList*)currentMsgPointer;
	   
	ptResetList->ulNumOfElem = 1;
	ptResetList->ulNumOfElemFields = 2;
	ptResetList->tResets[0].unSlotID = CNTL_SLOT_ID;
	ptResetList->tResets[0].unStatus = IPMI_CARD_STATUS_NORMAL;


   	MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   	"(BuildIpmiResetIndMsg): sending message on socket: %d, message: %d\n", TcpConnection[eSwitchDiagClient].s, currentMsgPointer);
	for(i = 0; i < (unIndBufSize/4); i++)
	{
	    MfaBoardPrint(IPMI_PRINT, PRINT_LEVEL_MAJOR, PRINT_TO_TERMINAL,
	    "Indication buffer[%d] = %x", i, *(pMsg + i));
	}

	// Send Message Switch Server ...
	//sendMsgFromIpmiToEma(eIpmiServer, unIndBufSize, pMsg);
	if ( TCPSendData( TcpConnection[eIpmiClient].s, (VOID *)pMsg, unIndBufSize, 0,BOARD_PRINT_YES) == SOCKET_OPERATION_FAILED)
	{
		//yosi fix return value socket
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL, "(BuildIpmiResetIndMsg): problem in sending data...\n");
	    TcpConnection[eIpmiClient].ul_ConnectionStatus = NOT_CONNECTED;
	    TcpConnection[eIpmiClient].ul_PrevConnState = CONNECTED;
	    // TBD: Find out if there is any need to send failure notice to EMA.
	}
	
        if ((UINT32)IPMI_SLOT_ID_RESET == ulSlotID)
        {
		MfaBoardPrint(	IPMI_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"reboot");
		printf("reboot");
		system(HTTPD_KILL_COMMAND);
		system("/sbin/reboot");
        }
        else
        {
		MfaBoardPrint(	IPMI_PRINT,
						PRINT_LEVEL_ERROR,
						PRINT_TO_TERMINAL,
						"poweroff");
		printf("poweroff");
		system("/sbin/poweroff");
        }
done:
	// Free memory
	if ((void *)(pMsg) != NULL)
		  free((void *)pMsg);
}

//thread
void IpmiHandleThread()
{

	t_TcpConnParams*	ptConParams = &TcpConnection[eIpmiClient];
	TEmaReqHeader*		ptEmaReqHeader;

	SOCKET				clientSocket;
	UINT8				*uc_RcvBuf;
	UINT32				ul_BytesReceived;
	TMessageThreadType	tMessageThreadType;

	EnrollInThreadList(eIpmiHandleThread);

      MfaBoardPrint(MCMS_COM_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmiHandleThread");
	printf("\n******* IPMI HANDLER STARTING TO RUN ******** \n");

	// connect to SwitchDiag TCP Server...

	ptConParams->e_Id 		= eIpmiClient;
	ptConParams->us_Port 	= LISTEN_IPMI_PORT;
	ptConParams->ul_ClientOrServer = eConnTypeClient;
	strcpy(&ptConParams->IpV4Addr.auc_IpV4Address[0]   , "127.0.0.1"	);
	
	// Connect as client to SwitchDiagServer
	ConnectToTcpServer(ptConParams);

	while(1)
	{

		// Handle ipmi requests (from SwitchConnection ....)
		// Receive message, get opcode and build a state machine to handle the requests)
		MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
					  "IpmiHandleThread: main loop");

	if (ptConParams->ul_ConnectionStatus == CONNECTED)
     	{       
			clientSocket = ptConParams->s;

			MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
   			"IpmiHandleThread: clientSocket = %d \n", clientSocket);
		
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
			MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
   			"IpmiHandleThread: problem receiving data \n");
			if(uc_RcvBuf != NULL)
			{
				free(uc_RcvBuf);
			}
		}
   		else
   		{
   			
			tMessageThreadType.ulData = (UINT32)uc_RcvBuf;  
			tMessageThreadType.ulSize = ul_BytesReceived;

			MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,
		       			"IpmiHandleThread: Received data from switch \n");

			ptEmaReqHeader = (TEmaReqHeader*)tMessageThreadType.ulData;
			if(ul_BytesReceived < sizeof(TEmaReqHeader))
			{
				MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmiHandleThread: problem receiving data ul_BytesReceived < sizeof(TEmaReqHeader) \n");
				free(ptEmaReqHeader);
				ptEmaReqHeader = NULL;
				continue;
			}
			
			UINT32 ulOpcode = ptEmaReqHeader->ulOpcode;
			UINT32 ulMsgId  = ptEmaReqHeader->ulMsgID;
			UINT32 ulSlotId = ptEmaReqHeader->ulSlotID;

			MfaBoardPrint(	IPMI_PRINT,
							PRINT_LEVEL_ERROR,
							PRINT_TO_TERMINAL,
							"(IpmiHandleThread): EmaReqHeader:: eID:%d, opcode:%Xh, msgID:%d, slotID:%d.\n",
							eIpmiClient,
							ulOpcode,
							ulMsgId,
							ulSlotId);	

           		TraceSwitchEmaCom(eHandle, eIpmiClient, ulMsgId, ulOpcode, ulSlotId, "None");
			switch(ulOpcode)
			{
				case PRIVATE_OPCODE:
					
					MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"IpmiHandleThread: we are in PRIVATE_OPCODE \n");
				     break;
					
				case IPMI_ENTITY_LIST_REQ:
					// Send indication response
					//	printf("test list requested\n");
					BuildIpmiEntityListIndMsg(ulMsgId, ulSlotId);
					//	printf("after BuildIpmiEntityListIndMsg\n");
					break;
				case IPMI_FAN_INFO_REQ:
					BuildIpmiFanInfoIndMsg(ulMsgId, ulSlotId);
					break;
				case IPMI_GET_FAN_LEVEL_REQ:
					BuildIpmiFanLevelIndMsg(ulMsgId, ulSlotId);
					break;
				case IPMI_GET_EVENT_LOG_REQ:
					BuildIpmiEventLogIndMsg(ulMsgId, ulSlotId);
					break;
				case IPMI_FRU_REQ:
					BuildIpmiFruIndMsg(ulMsgId, ulSlotId);
					break;
				case IPMI_SENSOR_LIST_REQ:
					BuildIpmiSensorListIndMsg(ulMsgId, ulSlotId);
					break;
				case IPMI_SENSOR_READING_LIST_REQ:
					BuildIpmiSensorReadingListIndMsg(ulMsgId, ulSlotId);
					break;
				case IPMI_DO_RESET_REQ:
					MfaBoardPrint(	IPMI_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"Reset on demand from EMA. (SlotId:%d)",ulSlotId);
					printf(	"Reset on demand from EMA. (SlotId:%d)", ulSlotId);						
				       BuildIpmiResetIndMsg(ulMsgId, ulSlotId);
					break;
				default:

					MfaBoardPrint(	IPMI_PRINT,
									PRINT_LEVEL_ERROR,
									PRINT_TO_TERMINAL,
									"(IpmiHandleThread): Unknown Opcode !!! (%Xh)",ulOpcode);
					printf(	"(IpmiHandleThread): Unknown Opcode !!! (%Xh)",ulOpcode);		
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
			MfaBoardPrint(IPMI_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,
						  "DiagnosticsMain: main loop. Wait for connection");
			printf("Wait for connection\n");
	 		EmbSleep(30);
     	}
   	}
}

