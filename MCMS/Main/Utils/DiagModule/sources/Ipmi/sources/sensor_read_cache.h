#ifndef __SENSOR_READ_CACHE_H__
#define __SENSOR_READ_CACHE_H__

#include "LinuxSystemCallsApi.h"

#define SENSOR_READINGS_CACHE_FILE_FIRST "/tmp/sensor.readings.1"
#define SENSOR_READINGS_CACHE_FILE_SECOND "/tmp/sensor.readings.2"
#define SENSOR_READINGS_CACHE_FILE_THIRD "/tmp/sensor.readings.3"

void InitSensorsCacheLock();
void DestroySensorsCacheLock();
void LockSensorsCache();
void UnlockSensorsCache();

int StartSensorReadCache(pthread_t * tid, int * bQuit);
void WaitForSensorReadCache(pthread_t tid, int * bQuit);

#endif

