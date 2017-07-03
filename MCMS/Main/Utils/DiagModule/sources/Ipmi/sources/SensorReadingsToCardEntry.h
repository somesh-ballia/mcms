#ifndef __SENSOR_READINGS_TO_CARD_ENTRY_H__
#define __SENSOR_READINGS_TO_CARD_ENTRY_H__

#include "CardContent.h"
#include "sensor_read.h"

typedef enum
{
      IPMI_CARD_STATUS_NORMAL = 0
    , IPMI_CARD_STATUS_MAJOR = 1
    , IPMI_CARD_STATUS_EMPTY = 2
}IpmiCardStatus;

typedef enum
{
      IPMI_METEOR_STATUS_NA = -1
    , IPMI_METEOR_STATUS_NORMAL = 0
    , IPMI_METEOR_STATUS_MAJOR = 1
    , IPMI_METEOR_STATUS_CRITICAL = 2
}IpmiMeteorStatus;

void SensorReadingsToCardEntry(TIpmiEntityData * card, sensor_t * readings, int readingCount);

IpmiCardStatus GetCardStatus(sensor_t * reading);
IpmiMeteorStatus GetMeteorStatus(sensor_t * reading);

#endif

