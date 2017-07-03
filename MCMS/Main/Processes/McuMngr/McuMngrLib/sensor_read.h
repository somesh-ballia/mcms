#ifndef _SENSOR_READ_H_
#define _SENSOR_READ_H_

#include "IpmiSensorEnums.h"
#include <vector>
#include <string>
using std::vector;

typedef struct sensor_thresholds_ref_t
{
    float LowerNonRecoverable;
    float LowerCritical;
    float LowerNonCritical;
    float UpperNonCritical;
    float UpperCritical;
    float UpperNonRecoverable;
} sensor_thresholds_ref;

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

typedef int (*SensorReadFunc)(vector<sensor_t> & sensors);
typedef int (*SensorReadFuncWithLog)(vector<sensor_t> & sensors, std::string * ipmiSensorLog);

extern SensorReadFunc sensor_read;
extern SensorReadFuncWithLog sensor_read_with_log;

int GetSensorReadingState1(sensor_s const & s);

struct DescrSensorEntity;
int ParseSensorReadingLine(char const * line, sensor_t * temp, DescrSensorEntity & ref);
void ValidateSensorReading(float nominal, sensor_s & reading);

#include "IpmiConsts.h"

#endif

