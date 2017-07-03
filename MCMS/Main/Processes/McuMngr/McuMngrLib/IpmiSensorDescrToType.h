#ifndef __IPMI_SENSOR_DESCR_TO_TYPE_H__
#define __IPMI_SENSOR_DESCR_TO_TYPE_H__

#include "IpmiSensorEnums.h"
#include "sensor_read.h"

enum MeteorType
{
      METEOR_NONE = -1
    , METEOR_VOLTAGE
    , METEOR_TEMPERATURE
};

typedef struct DescrSensorEntity
{
    char const * descr;
    int sensorType;
    int entityType;
    float nominalVal;
    int index;
    char const * cardType;
    MeteorType meteorType;
    int faultBase;
    int faultOffsetMax;
    EventReadType eventReadType;
    sensor_thresholds_ref ref;
}DescrSensorEntity;

int IpmiSensorDescrToEntityType(char const * descr);
int IpmiSensorDescrToSensorType(char const * descr);
int IpmiSensorDescrToSensorNumber(char const * descr);
float IpmiSensorDescrToNominalVal(char const * descr);
int IpmiSensorDescrToSlotID(char const * descr);

int IsSensorValid(char const * descr, DescrSensorEntity & ref);

struct CardAndMeteorType
{
    char const * cardType;
    MeteorType meteorType;
};

CardAndMeteorType GetCardAndMeteorType(char const * desc);

struct CardContent;
struct sensor_s;
void UpdateCardStatus(CardContent & card, sensor_s const & reading, MeteorType type);

typedef struct AlarmInfo_t
{
    int base;
    int offset;
    int offsetMax;
    int isLow;
} AlarmInfo;

enum
{
      ALARM_OFFSET_NORMAL = 0
    , ALARM_OFFSET_MAJOR = 1
    , ALARM_OFFSET_CRITICAL = 2
    , ALARM_OFFSET_NON_RECOVERABLE = 3
};

AlarmInfo GetAlarmInfo(sensor_s const & s);

typedef struct HWAlaramEntity
{
    const int fault;
    const int alarmIdx;
    int action;     // 0 -- No Action,  1 -- Add Alarm,    2-- Remove Alarm,    3--Re-Add Alarm
    char lastDesc[128];
    char curDesc[128];
}HWAlaramEntity;

void FlushAlarmFlag();
void SetAlarmReportFlag(int base, int offset, sensor_s const & s); //If alarm should be sent currently, set relevant alarm flag. 
void GetAlarmAction(HWAlaramEntity * & alarmActionArray, unsigned int & size); //Get current action for each alarm.

#endif

