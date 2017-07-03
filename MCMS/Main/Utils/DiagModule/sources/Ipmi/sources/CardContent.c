//#include "CardContent.h"
#include "SharedDefines.h"
#include "SensorReadingsToCardEntry.h"
#include <string.h>
#include <stdlib.h>
#include "IpmiSensorDescrToType.h"
//#include "copy_string.h"
#include "IpmiEntitySlotIDs.h"
#include "IpmiConsts.h"
#include "to_string.h"
#include "GetCpuMemUsage.h"

typedef void (*GetCardContentFunc)(char const * type, int index, TIpmiEntityData * tpEntityData);
typedef BOOL (*IsCardReadingsPredicate)(char const * cardType, char const * refType);

typedef struct CardContentEntry
{
    char const * cardType;
    int index;
    GetCardContentFunc func;
    IsCardReadingsPredicate isCardReadings;
}CardContentEntry;

void GetControlBoardContent(char const * type, int index, TIpmiEntityData * tpEntityData)
{
    memset(tpEntityData, 0, sizeof(TIpmiEntityData));
    strncpy(tpEntityData->unCardType, type, sizeof(tpEntityData->unCardType)-1);
    tpEntityData->unSlotID = CNTL_SLOT_ID;
    tpEntityData->unSubBoardID = SUB_BOARD_CARD_TYPE;
}

void GetFansContent(char const * type, int index, TIpmiEntityData * tpEntityData)
{
    memset(tpEntityData, 0, sizeof(TIpmiEntityData));
    strncpy(tpEntityData->unCardType, type, sizeof(tpEntityData->unCardType)-1);
    tpEntityData->unSlotID = FANS_SLOT_ID;
}

void GetPowerContent(char const * type, int index, TIpmiEntityData * tpEntityData)
{
    memset(tpEntityData, 0, sizeof(TIpmiEntityData));
    strncpy(tpEntityData->unCardType, type, sizeof(tpEntityData->unCardType)-1);
    tpEntityData->unSlotID = PWRS_SLOT_ID;
}

void GetRiserContent(char const * type, int index, TIpmiEntityData * tpEntityData)
{

    memset(tpEntityData, 0, sizeof(TIpmiEntityData));
    strncpy(tpEntityData->unCardType, type, sizeof(tpEntityData->unCardType)-1);
    tpEntityData->unSlotID = RISER_SLOT_ID;
    tpEntityData->unSubBoardID = SUB_BOARD_CARD_TYPE;
}
void GetDspCard0Content(char const * type, int index, TIpmiEntityData * tpEntityData)
{
    memset(tpEntityData, 0, sizeof(TIpmiEntityData));
    strncpy(tpEntityData->unCardType, type, sizeof(tpEntityData->unCardType)-1);
    tpEntityData->unSlotID = DSP_CARD_SLOT_ID_0;
    tpEntityData->unSubBoardID = SUB_BOARD_CARD_TYPE;
}
void GetDspCard1Content(char const * type, int index, TIpmiEntityData * tpEntityData)
{
    memset(tpEntityData, 0, sizeof(TIpmiEntityData));
    strncpy(tpEntityData->unCardType, type, sizeof(tpEntityData->unCardType)-1);
    tpEntityData->unSlotID = DSP_CARD_SLOT_ID_1;
    tpEntityData->unSubBoardID = SUB_BOARD_CARD_TYPE;
}

void GetDspCard2Content(char const * type, int index, TIpmiEntityData * tpEntityData)
{
    memset(tpEntityData, 0, sizeof(TIpmiEntityData));
    strncpy(tpEntityData->unCardType, type, sizeof(tpEntityData->unCardType)-1);
    tpEntityData->unSlotID = DSP_CARD_SLOT_ID_2;
    tpEntityData->unSubBoardID = SUB_BOARD_CARD_TYPE;
}

void GetRtmISDNContent(char const * type, int index, TIpmiEntityData * tpEntityData)
{
    memset(tpEntityData, 0, sizeof(TIpmiEntityData));
    strncpy(tpEntityData->unCardType, type, sizeof(tpEntityData->unCardType)-1);
    tpEntityData->unSlotID = ISDN_CARD_SLOT_ID;
    tpEntityData->unSubBoardID = SUB_BOARD_CARD_TYPE;
}

BOOL IsReadingFromCardGeneric(char const * cardType, char const * refType)
{
    return 0==strcmp(cardType, refType);
}
BOOL IsReadingFromCntl(char const * cardType, char const * refType)
{
    //(void)refType;
    return (0==strcmp(CNTL_BOARD_NAME, cardType)) && (0==strcmp(CNTL_BOARD_NAME, refType)) ;
}
BOOL IsReadingFromRtmISDN(char const * cardType, char const * refType)
{
    return (0==strcmp(NETRA_RTM_ISDN_NAME, cardType));
}

static CardContentEntry const s_entries[] =
{
      {CNTL_BOARD_NAME, 0, GetControlBoardContent, IsReadingFromCntl}
    , {"FANS", 0, GetFansContent, IsReadingFromCardGeneric}
    , {"PWR", 0, GetPowerContent, IsReadingFromCardGeneric}
    , {NETRA_DSP_BOARD1_NAME, 1, GetDspCard0Content, IsReadingFromCardGeneric}
    , {NETRA_DSP_BOARD2_NAME, 1, GetDspCard1Content, IsReadingFromCardGeneric}
    , {NETRA_DSP_BOARD3_NAME, 1, GetDspCard2Content, IsReadingFromCardGeneric}
    , {NETRA_RTM_ISDN_NAME, 1, GetRtmISDNContent, IsReadingFromRtmISDN}
};

static IsCardReadingsPredicate GetCardFilterPredicate(TIpmiEntityData * card)
{
    //int const count = sizeof(s_entries)/sizeof(0[s_entries]);
    size_t i = 0;
    for (i=0; i < (sizeof(s_entries)/sizeof(0[s_entries])); ++i)
    {
        if (0==strcmp(card->unCardType, s_entries[i].cardType))
        {
            return s_entries[i].isCardReadings;
        }
    }

    return s_entries[0].isCardReadings;
}

static int McuMngrCollectCardContents(TIpmiEntityListData * entityList)
{
	//Prepare entityList
	//int const count = sizeof(s_entries)/sizeof(0[s_entries]);
	// size_t i = 0;
	size_t i = 0, entIdx = 0;

	entityList->ulNumOfElem = 0;
	entityList->ulNumOfElemFields = 10;
	
	for (i=0,  entIdx = 0; ((i < sizeof(s_entries)/sizeof(0[s_entries])) && i < MAX_NUM_OF_SLOTS); ++i)
	{
		(*(s_entries[i].func))(s_entries[i].cardType, s_entries[i].index, &(entityList->tEntityData[entIdx]));

		//skip invalid dsp card
		if(entityList->tEntityData[entIdx].unSlotID >= DSP_CARD_SLOT_ID_0 && entityList->tEntityData[entIdx].unSlotID <= DSP_CARD_SLOT_ID_2 && FALSE == isValidCard(entityList->tEntityData[entIdx].unSlotID)) 
			continue;

            //skip invalid rtm isdn card
		if(entityList->tEntityData[entIdx].unSlotID == ISDN_CARD_SLOT_ID && FALSE == isValidRtmIsdnCard(entityList->tEntityData[entIdx].unSlotID)) 
			continue;
		
		entityList->ulNumOfElem++;
		entityList->tEntityData[entIdx].ulNumOfElem = 0;
		entityList->tEntityData[entIdx].ulNumOfElemFields = 3;
		entIdx++;
	}

	sensor_t * readings = NULL;
	int readingcount = 0, size = 0;
	sensor_read(&readings, &readingcount, &size);

	for (i=0; i<entityList->ulNumOfElem; ++i)
	{
	    SensorReadingsToCardEntry(&(entityList->tEntityData[i]), readings, readingcount);
	}

	if(NULL != readings)
	{
		free(readings);
		readings = NULL;
	}

	for (i=0; i<entityList->ulNumOfElem; ++i)
	{
		//change card type from "DSP Card 1" to "DSP Card" and send to EMA
		if(entityList->tEntityData[i].unSlotID >= DSP_CARD_SLOT_ID_0 && entityList->tEntityData[i].unSlotID <= DSP_CARD_SLOT_ID_2)
		{
			memset(entityList->tEntityData[i].unCardType, 0, sizeof(entityList->tEntityData[i].unCardType));
			strncpy(entityList->tEntityData[i].unCardType, NETRA_DSP_BOARD_NAME, sizeof(entityList->tEntityData[i].unCardType)-1);
		}
             else if(ISDN_CARD_SLOT_ID == entityList->tEntityData[i].unSlotID)
             {
			memset(entityList->tEntityData[i].unCardType, 0, sizeof(entityList->tEntityData[i].unCardType));
			strncpy(entityList->tEntityData[i].unCardType, NETRA_RTM_ISDN_NAME, sizeof(entityList->tEntityData[i].unCardType)-1);                
             }
	}
    
    return entityList->ulNumOfElem;
}

CollectCardContentsType CollectCardContents = McuMngrCollectCardContents;

IpmiCardStatus GetCardStatus(sensor_t * reading)
{
    if (0==strcasecmp("ok", reading->Status))
        return IPMI_CARD_STATUS_NORMAL;
    else if (0==strcmp("", reading->Status))
        return IPMI_CARD_STATUS_NORMAL;
    else if (0==strcasecmp("na", reading->Status))
        return IPMI_CARD_STATUS_MAJOR;
    else if (0==strcasecmp("0x0100", reading->Status))
        return IPMI_CARD_STATUS_NORMAL;
    else if (0==strcasecmp("0x0b00", reading->Status))
        return IPMI_CARD_STATUS_MAJOR;
    else return IPMI_CARD_STATUS_NORMAL;
}

IpmiMeteorStatus GetMeteorStatus(sensor_t * reading)
{
    if ((reading->CurrentVal>=reading->LowerNonCritical) && (reading->CurrentVal<=reading->UpperNonCritical))
        return IPMI_METEOR_STATUS_NORMAL;
    else if ((reading->CurrentVal>=reading->LowerCritical) && (reading->CurrentVal<=reading->UpperCritical))
        return IPMI_METEOR_STATUS_MAJOR;
    else
        return IPMI_METEOR_STATUS_CRITICAL;
}

void UpdateCardStatus(TIpmiEntityData * card, sensor_t * reading, MeteorType type)
{
    IpmiCardStatus const cardStatus = GetCardStatus(reading);
    IpmiMeteorStatus const meteorStatus = GetMeteorStatus(reading);
    if (cardStatus>card->unStatus)
    {
        card->unStatus = cardStatus;
    }

    if (meteorStatus>IPMI_METEOR_STATUS_NORMAL)
    {
        if (IPMI_CARD_STATUS_NORMAL==card->unStatus)
        {
            card->unStatus = IPMI_CARD_STATUS_MAJOR;
        }

        switch (type)
        {
        case METEOR_TEMPERATURE:
            card->unTemperature = meteorStatus;
            break;
        case METEOR_VOLTAGE:
            card->unVoltage = meteorStatus;
            break;
        default:
            break;
        }
    }
}

void SensorReadingsToCardEntry(TIpmiEntityData * card, sensor_t * readings, int readingCount)
{
	int i = 0;
	IsCardReadingsPredicate predicate = GetCardFilterPredicate(card);
	for (i=0; i<readingCount; ++i)
	{
	    CardAndMeteorType const type = GetCardAndMeteorType(readings[i].SensorName);
	    if (0==strcmp("", type.cardType)) continue;
	    if (!(*predicate)(card->unCardType, type.cardType)) continue;

	    UpdateCardStatus(card, &(readings[i]), type.meteorType);
	}
}


//Fan Info
 int UpdateFanLevel(TIpmiFanLevel * FanLevelList)
{
	//Prepare Fan List
	int i = 0;

	FanLevelList->ulNumOfElem = 0;
	FanLevelList->ulNumOfElemFields = 1;

	sensor_t * readings = NULL;
	int readingcount = 0, size = 0;
	sensor_read(&readings, &readingcount, &size);

	FanLevelList->unFanLevel[FanLevelList->ulNumOfElem] = 2;
	FanLevelList->ulNumOfElem++;

	for (i=0; i<readingcount; ++i)
	{
		int const sensorType = IpmiSensorDescrToSensorType(readings[i].SensorName);
		if (IPMI_SENSOR_TYPE_FAN!=sensorType) continue;

		FanLevelList->unFanLevel[FanLevelList->ulNumOfElem] = readings[i].CurrentVal;
		FanLevelList->ulNumOfElem++;
	}

	if(NULL != readings)
	{
		free(readings);
		readings = NULL;
	}
    
	return FanLevelList->ulNumOfElem;
}

//Sensor List
void GetSensorList(int slotID, TIpmiSensorList* info)
{
	int i = 0;
	unsigned int index = 0;
	info->ulNumOfElem = 0;
	info->ulNumOfElemFields = 14;

	sensor_t * readings = NULL;
	int readingcount = 0, size = 0;
	sensor_read(&readings, &readingcount, &size);
	
    for (i=0; i<readingcount; ++i)
    {
        int const sensorType = IpmiSensorDescrToSensorType(readings[i].SensorName);
        if (IPMI_SENSOR_TYPE_FAN==sensorType) continue;
        if(slotID != IpmiSensorDescrToSlotID(readings[i].SensorName)) continue;

        FloatToString(IpmiSensorDescrToNominalVal(readings[i].SensorName), info->tSensors[info->ulNumOfElem].unNominalVal, MAX_FLOAT_STR_SIZE);
        info->tSensors[info->ulNumOfElem].unSlotID = slotID;
        FloatToString(readings[i].LowerNonRecoverable, info->tSensors[info->ulNumOfElem].unLowerNonRecoverable, MAX_FLOAT_STR_SIZE);
        info->tSensors[info->ulNumOfElem].unEntityId = IpmiSensorDescrToEntityType(readings[i].SensorName);
        FloatToString(readings[i].LowerCritical, info->tSensors[info->ulNumOfElem].unLowerCritical, MAX_FLOAT_STR_SIZE);
        FloatToString(readings[i].UpperNonRecoverable, info->tSensors[info->ulNumOfElem].unUpperNonRecoverable, MAX_FLOAT_STR_SIZE);
        strcpy(info->tSensors[info->ulNumOfElem].unSensorDescr, readings[i].SensorName);
        FloatToString(readings[i].LowerNonCritical, info->tSensors[info->ulNumOfElem].unLowerNonCritical, MAX_FLOAT_STR_SIZE);
        FloatToString(readings[i].UpperCritical, info->tSensors[info->ulNumOfElem].unUpperCritical, MAX_FLOAT_STR_SIZE);
        info->tSensors[info->ulNumOfElem].unSensorNumber= IpmiSensorDescrToSensorNumber(readings[i].SensorName);
        info->tSensors[info->ulNumOfElem].unSensorType= IpmiSensorDescrToSensorType(readings[i].SensorName);
        info->tSensors[info->ulNumOfElem].unEventReadType = readings[i].eventReadType;
        info->tSensors[info->ulNumOfElem].unEntityInstance= 96;
        FloatToString(readings[i].UpperNonCritical, info->tSensors[info->ulNumOfElem].unUpperNonCritical, MAX_FLOAT_STR_SIZE);
        info->ulNumOfElem++;
    }

    if (CNTL_SLOT_ID == slotID)
    {
        {
            char const * sensorName = "CPU Usage";
	        FloatToString(IpmiSensorDescrToNominalVal(sensorName), info->tSensors[info->ulNumOfElem].unNominalVal, MAX_FLOAT_STR_SIZE);
	        info->tSensors[info->ulNumOfElem].unSlotID = CNTL_SLOT_ID;
	        info->tSensors[info->ulNumOfElem].unEntityId = IpmiSensorDescrToEntityType(sensorName);
	        strncpy(info->tSensors[info->ulNumOfElem].unSensorDescr, sensorName, sizeof(info->tSensors[info->ulNumOfElem].unSensorDescr)-1);
		  info->tSensors[info->ulNumOfElem].unSensorNumber= IpmiSensorDescrToSensorNumber(sensorName);
	        info->tSensors[info->ulNumOfElem].unSensorType= IpmiSensorDescrToSensorType(sensorName);
	        info->tSensors[info->ulNumOfElem].unEventReadType = EVENT_READ_TYPE_NO_COLOR;
	        info->tSensors[info->ulNumOfElem].unEntityInstance= 96;

	        FloatToString(1001.0, info->tSensors[info->ulNumOfElem].unUpperNonRecoverable, MAX_FLOAT_STR_SIZE);
	        FloatToString(990.0, info->tSensors[info->ulNumOfElem].unUpperCritical, MAX_FLOAT_STR_SIZE);
	        FloatToString(950.0, info->tSensors[info->ulNumOfElem].unUpperNonCritical, MAX_FLOAT_STR_SIZE);
			
	        FloatToString(0.0, info->tSensors[info->ulNumOfElem].unLowerNonRecoverable, MAX_FLOAT_STR_SIZE);
	        FloatToString(1.0, info->tSensors[info->ulNumOfElem].unLowerCritical, MAX_FLOAT_STR_SIZE);
	        FloatToString(2.0, info->tSensors[info->ulNumOfElem].unLowerNonCritical, MAX_FLOAT_STR_SIZE);
	        info->ulNumOfElem++;
        }

        {
            char const * sensorName = "Memory Usage";
	        FloatToString(IpmiSensorDescrToNominalVal(sensorName), info->tSensors[info->ulNumOfElem].unNominalVal, MAX_FLOAT_STR_SIZE);
	        info->tSensors[info->ulNumOfElem].unSlotID = CNTL_SLOT_ID;
	        info->tSensors[info->ulNumOfElem].unEntityId = IpmiSensorDescrToEntityType(sensorName);
	        strncpy(info->tSensors[info->ulNumOfElem].unSensorDescr, sensorName, sizeof(info->tSensors[info->ulNumOfElem].unSensorDescr)-1);
		  info->tSensors[info->ulNumOfElem].unSensorNumber= IpmiSensorDescrToSensorNumber(sensorName);
	        info->tSensors[info->ulNumOfElem].unSensorType= IpmiSensorDescrToSensorType(sensorName);
	        info->tSensors[info->ulNumOfElem].unEventReadType = EVENT_READ_TYPE_NO_COLOR;
	        info->tSensors[info->ulNumOfElem].unEntityInstance= 96;

	        FloatToString(1001.0, info->tSensors[info->ulNumOfElem].unUpperNonRecoverable, MAX_FLOAT_STR_SIZE);
	        FloatToString(990.0, info->tSensors[info->ulNumOfElem].unUpperCritical, MAX_FLOAT_STR_SIZE);
	        FloatToString(950.0, info->tSensors[info->ulNumOfElem].unUpperNonCritical, MAX_FLOAT_STR_SIZE);
			
	        FloatToString(0.0, info->tSensors[info->ulNumOfElem].unLowerNonRecoverable, MAX_FLOAT_STR_SIZE);
	        FloatToString(1.0, info->tSensors[info->ulNumOfElem].unLowerCritical, MAX_FLOAT_STR_SIZE);
	        FloatToString(2.0, info->tSensors[info->ulNumOfElem].unLowerNonCritical, MAX_FLOAT_STR_SIZE);
	        info->ulNumOfElem++;
        }
    }

	if(NULL != readings)
	{
		free(readings);
		readings = NULL;
	}
    
	return;
	
}


//Sensor Reading List
void GetSensorReadingList(int slotID, TIpmiSensorReadingList* info)
{
	int i = 0;
	unsigned int index = 0;
	info->ulNumOfElem = 0;
	info->ulNumOfElemFields = 5;

	sensor_t * readings = NULL;
	int readingcount = 0, size = 0;
	sensor_read(&readings, &readingcount, &size);
	
    for (i=0; i<readingcount; ++i)
    {
        int const sensorType = IpmiSensorDescrToSensorType(readings[i].SensorName);
        if (IPMI_SENSOR_TYPE_FAN==sensorType) continue;
	    if(slotID != IpmiSensorDescrToSlotID(readings[i].SensorName)) continue;
	  
        info->tSensorReadings[info->ulNumOfElem].unSensorState1 = GetSensorReadingState1(&readings[i]);
        info->tSensorReadings[info->ulNumOfElem].unSensorState2 = 255;
        info->tSensorReadings[info->ulNumOfElem].unSensorNumber= IpmiSensorDescrToSensorNumber(readings[i].SensorName);
        info->tSensorReadings[info->ulNumOfElem].unSlotID = slotID;
        FloatToString(readings[i].CurrentVal, info->tSensorReadings[info->ulNumOfElem].unSensorReading, MAX_FLOAT_STR_SIZE);
        info->ulNumOfElem++;
    }

    if (CNTL_SLOT_ID == slotID)
    {
        {
            int const cpuUsage = GetCPUUsage();

            char const * sensorName = "CPU Usage";
            info->tSensorReadings[info->ulNumOfElem].unSensorState1 = GetUsageReadingState1(cpuUsage);
            info->tSensorReadings[info->ulNumOfElem].unSensorState2 = 255;
            info->tSensorReadings[info->ulNumOfElem].unSensorNumber = IpmiSensorDescrToSensorNumber(sensorName);
            info->tSensorReadings[info->ulNumOfElem].unSlotID = CNTL_SLOT_ID;
            FloatToString(cpuUsage, info->tSensorReadings[info->ulNumOfElem].unSensorReading, MAX_FLOAT_STR_SIZE);
            info->ulNumOfElem++;
        }

        {
            int const memUsage = GetMemUsage(); 

            char const * sensorName = "Memory Usage";
            info->tSensorReadings[info->ulNumOfElem].unSensorState1 = GetUsageReadingState1(memUsage);
            info->tSensorReadings[info->ulNumOfElem].unSensorState2 = 255;
            info->tSensorReadings[info->ulNumOfElem].unSensorNumber = IpmiSensorDescrToSensorNumber(sensorName);
            info->tSensorReadings[info->ulNumOfElem].unSlotID = CNTL_SLOT_ID;
            FloatToString(memUsage, info->tSensorReadings[info->ulNumOfElem].unSensorReading, MAX_FLOAT_STR_SIZE);
            info->ulNumOfElem++;
        }
    }

	if(NULL != readings)
	{
		free(readings);
		readings = NULL;
	}
    
	return;
	
}

