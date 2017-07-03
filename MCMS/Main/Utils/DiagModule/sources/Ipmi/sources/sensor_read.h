#ifndef _SENSOR_READ_H_
#define _SENSOR_READ_H_

#include "tools.h"
#include "IpmiSensorEnums.h"

typedef struct sensor_thresholds_ref_t
{
    float LowerNonRecoverable;
    float LowerCritical;
    float LowerNonCritical;
    float UpperNonCritical;
    float UpperCritical;
    float UpperNonRecoverable;
} sensor_thresholds_ref;

typedef enum
{
      METEOR_NONE = -1
    , METEOR_VOLTAGE
    , METEOR_TEMPERATURE
}MeteorType;

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

typedef struct sensor_s 
{
    char SensorName[16];
    float CurrentVal;
    char Unit[16];
    char Status[16];
    float LowerNonRecoverable;
    float LowerCritical;
    float LowerNonCritical;
    float UpperNonCritical;
    float UpperCritical;
    float UpperNonRecoverable;
    EventReadType eventReadType;
    sensor_thresholds_ref ref;
} sensor_t;

typedef int (*SensorReadFunc)(sensor_t ** sensorArray, int * count, int * size);

extern SensorReadFunc sensor_read;

int GetSensorReadingState1(sensor_t * s);
int GetUsageReadingState1(int usage);

struct DescrSensorEntity;
int ParseSensorReadingLine(char * line, sensor_t * temp, DescrSensorEntity * ref);
#if 0
void ValidateSensorReading(float nominal, sensor_s & reading);
#endif
#include "IpmiConsts.h"

#endif

