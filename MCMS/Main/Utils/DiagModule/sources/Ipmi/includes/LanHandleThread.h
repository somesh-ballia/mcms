#ifndef _LANHANDLETHREAD_H
#define _LANHANDLETHREAD_H

#include "DiagDataTypes.h"

#define MAX_NUM_OF_PORTS  10
#define MAX_PORT_INFO_STR_SIZE 24

//Lan Stat Get Ports List
typedef struct
{
	UINT32				unSlotID;
	UINT32				unPortID;
	UINT32				unStatus;
}TLanPorts;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TLanPorts 			tPorts[MAX_NUM_OF_PORTS];
}TLanStatGetPortsList;

//Lan Stat Info
typedef struct
{
	INT8				unRxPackets[MAX_PORT_INFO_STR_SIZE];
	INT8				unRxBadPackets[MAX_PORT_INFO_STR_SIZE];
	INT8				unRxCRC[MAX_PORT_INFO_STR_SIZE];
	INT8				unRxOctets[MAX_PORT_INFO_STR_SIZE];
	INT8				unMaxRxPackets[MAX_PORT_INFO_STR_SIZE];
	INT8				unMaxRxBadPackets[MAX_PORT_INFO_STR_SIZE];
	INT8				unMaxRxCRC[MAX_PORT_INFO_STR_SIZE];
	INT8				unMaxRxOctets[MAX_PORT_INFO_STR_SIZE];
	INT8				unTxPackets[MAX_PORT_INFO_STR_SIZE];
	INT8				unTxBadPackets[MAX_PORT_INFO_STR_SIZE];
	INT8				unTxFifoDrops[MAX_PORT_INFO_STR_SIZE];
	INT8				unTxOctets[MAX_PORT_INFO_STR_SIZE];
	INT8				unMaxTxPackets[MAX_PORT_INFO_STR_SIZE];
	INT8				unMaxTxBadPackets[MAX_PORT_INFO_STR_SIZE];
	INT8				unMaxTxFifoDrops[MAX_PORT_INFO_STR_SIZE];
	INT8				unMaxTxOctets[MAX_PORT_INFO_STR_SIZE];
	UINT32			unActLinkStatus;
	UINT32			unActLinkMode;
	UINT32			unActLinkAutoNeg;
	UINT32			unAdvLinkMode;
	UINT32			unAdvLinkAutoNeg;
}TLanStatInfo;


void LanHandleThread();

#endif
