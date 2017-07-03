#ifndef __IPMI_SENSOR_DESCR_TO_TYPE_H__
#define __IPMI_SENSOR_DESCR_TO_TYPE_H__

#include "IpmiSensorEnums.h"
#include "sensor_read.h"
#include "IpmiHandleThread.h"

int IpmiSensorDescrToEntityType(char const * descr);
int IpmiSensorDescrToSensorType(char const * descr);
int IpmiSensorDescrToSensorNumber(char const * descr);
float IpmiSensorDescrToNominalVal(char const * descr);
int IpmiSensorDescrToSlotID(char const * descr);

BOOL IsSensorValid(char const * descr, DescrSensorEntity * ref);

typedef struct CardAndMeteorType
{
    char const * cardType;
    MeteorType meteorType;
}CardAndMeteorType;

CardAndMeteorType GetCardAndMeteorType(char const * desc);

struct CardContent;
struct sensor_s;
void UpdateCardStatus(TIpmiEntityData * card, sensor_t * reading, MeteorType type);

typedef struct AlarmInfo_t
{
    int base;
    int offset;
} AlarmInfo;

enum
{
      ALARM_OFFSET_NORMAL = 0
    , ALARM_OFFSET_MAJOR = 1
    , ALARM_OFFSET_CRITICAL = 2
    , ALARM_OFFSET_NON_RECOVERABLE = 3
};

#if 0
AlarmInfo GetAlarmInfo(sensor_s const & s);
#endif 
#endif

