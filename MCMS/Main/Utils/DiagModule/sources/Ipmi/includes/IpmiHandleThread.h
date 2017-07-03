#ifndef _IPMIHANDLETHREAD_H
#define _IPMIHANDLETHREAD_H

#include "DiagDataTypes.h"
//#include "CardsStructs.h"

#define MAX_NUM_OF_SLOTS 30
#define MAX_NUM_OF_INSTANCE 30
#define MAX_NUM_OF_FANS 30
#define MAX_NUM_OF_EVENT_LOG 100
#define MAX_NUM_OF_SENSOR 50
#define MAX_CARD_TYPE_STR_SIZE 20
#define MAX_SENSOR_DESC_STR_SIZE 20
#define MAX_PRODUCT_NAME_STR_SIZE 32
#define MAX_VERSION_STR_SIZE 64
#define MAX_FILE_ID_STR_SIZE 16
#define MAX_FLOAT_STR_SIZE 32

//Entity List
typedef struct
{
	UINT32				unEntityId;
	UINT32 				unEntityInstance;
	UINT32 				Present;
}TIpmiEntityInstance;

typedef struct
{
	UINT32				unSlotID;
	UINT32 				unSubBoardID;
	UINT32 				unIpmbAddress;
	INT8					unCardType[MAX_CARD_TYPE_STR_SIZE];
	UINT32 				unNumMezzanine;
	UINT32 				unStatus;
	UINT32 				unTemperature;
	UINT32				unVoltage;
	UINT32				unBitFail;
	UINT32				ulNumOfElem;
	UINT32				ulNumOfElemFields;
	//TIpmiEntityInstance	tEntityInstance[MAX_NUM_OF_INSTANCE];
}TIpmiEntityData;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TIpmiEntityData 		tEntityData[MAX_NUM_OF_SLOTS];
}TIpmiEntityListData;


//Fan Info
typedef struct
{
	UINT32				unMinSpeedLevel;
	UINT32 				unMaxSpeedLevel;
	UINT32 				unNormalOperatingLevel;
}TIpmiFanInfo;

//Fan Level
typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	UINT32 				unFanLevel[MAX_NUM_OF_FANS];
}TIpmiFanLevel;


//Event Log
typedef struct
{
	UINT32				unRecordIdHi;
	UINT32 				unRecordIdLo;
	UINT32 				unRecordType;
	UINT32				unTimestamp;
	UINT32 				unIpmbSlaveAddr;
	UINT32 				unChannelNumber;
	UINT32				unEvmRev;
	UINT32 				unSensorType;
	UINT32 				unSensorNum;
	UINT32				unEventDirType;
	UINT32 				unEventData1;
	UINT32 				unEventData2;
	UINT32				unEventData3;
	INT8 				unSensorDescr[MAX_SENSOR_DESC_STR_SIZE];
}TIpmiEventLog;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TIpmiEventLog 		tEventLogs[MAX_NUM_OF_EVENT_LOG];
}TIpmiEventLogList;


//FRU
typedef struct
{
	UINT32				unMezzanineNumber;
	INT8 				unBoardHardwareVers[MAX_VERSION_STR_SIZE];
	INT8 				unBoardSerialNumber[MAX_VERSION_STR_SIZE];
	INT8 				unBoardPartNumber[MAX_VERSION_STR_SIZE];
}TIpmiSubBoard;

typedef struct
{
	UINT32				unSubBoardID;
	UINT32				unBoardMfgDateTime;
	UINT32				unBoardManufacturerType;
	INT8					unBoardProductName[MAX_PRODUCT_NAME_STR_SIZE ];
	INT8					unBoardSerialNumber[MAX_VERSION_STR_SIZE];
	INT8					unBoardPartNumber[MAX_VERSION_STR_SIZE];
	INT8					unBoardFileId[MAX_FILE_ID_STR_SIZE];
	INT8					unBoardHardwareVers[MAX_VERSION_STR_SIZE];
	INT8					unBoardMacAddr1[MAX_VERSION_STR_SIZE];
	INT8					unBoardMacAddr2[MAX_VERSION_STR_SIZE];
	INT8					unBoardMacAddr3[MAX_VERSION_STR_SIZE];
	INT8					unBoardMacAddr4[MAX_VERSION_STR_SIZE];
	UINT32				unBoardDspClockSpeed;
	UINT32				unChassisType;
	UINT32				unChassisPartNumber;
	UINT32				unChassisSerialNumber;
	UINT32				unChassisFileId;
	INT8					unChassisHwVersion[MAX_VERSION_STR_SIZE];
	UINT32				ulNumOfElem;
	UINT32				ulNumOfElemFields;
	TIpmiSubBoard		tSubBoards[MAX_NUM_OF_SLOTS];
}TIpmiFru;

//Sensor List
typedef struct
{
	UINT32				unSlotID;
	UINT32				unSensorNumber;
	UINT32				unEntityId;
	UINT32				unEntityInstance;
	UINT32				unSensorType;
	UINT32				unEventReadType;
	INT8				unNominalVal[MAX_FLOAT_STR_SIZE];
	INT8				unUpperNonRecoverable[MAX_FLOAT_STR_SIZE];
	INT8				unUpperCritical[MAX_FLOAT_STR_SIZE];
	INT8				unUpperNonCritical[MAX_FLOAT_STR_SIZE];
	INT8				unLowerNonRecoverable[MAX_FLOAT_STR_SIZE];
	INT8				unLowerCritical[MAX_FLOAT_STR_SIZE];
	INT8				unLowerNonCritical[MAX_FLOAT_STR_SIZE];
	INT8				unSensorDescr[MAX_SENSOR_DESC_STR_SIZE];
}TIpmiSensor;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TIpmiSensor 			tSensors[MAX_NUM_OF_SENSOR];
}TIpmiSensorList;


//Sensor Reading List
typedef struct
{
	UINT32				unSlotID;
	UINT32				unSensorNumber;
	INT8					unSensorReading[MAX_FLOAT_STR_SIZE];
	UINT32				unSensorState1;
	UINT32				unSensorState2;
}TIpmiSensorReading;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TIpmiSensorReading 			tSensorReadings[MAX_NUM_OF_SENSOR];
}TIpmiSensorReadingList;

//Reset
typedef struct
{
	UINT32				unSlotID;
	UINT32				unStatus;
}TIpmiReset;

typedef struct
{
	UINT32 				ulNumOfElem;
	UINT32 				ulNumOfElemFields;
	TIpmiReset 			tResets[MAX_NUM_OF_SLOTS];
}TIpmiResetList;

void IpmiHandleThread();

#endif
