#ifndef __SENSOR_READ_CACHE_H__
#define __SENSOR_READ_CACHE_H__

#include <pthread.h>
#include <string>
using namespace std;

#define SENSOR_READINGS_CACHE_FILE_FIRST ((std::string)(MCU_TMP_DIR+"/sensor.readings.1"))
#define SENSOR_READINGS_CACHE_FILE_SECOND ((std::string)(MCU_TMP_DIR+"/sensor.readings.2"))
#define SENSOR_READINGS_CACHE_FILE_THIRD ((std::string)(MCU_TMP_DIR+"/sensor.readings.3"))

void InitSensorsCacheLock();
void DestroySensorsCacheLock();
void LockSensorsCache();
void UnlockSensorsCache();

int StartSensorReadCache(pthread_t & tid, int & bQuit);
void WaitForSensorReadCache(pthread_t tid, int & bQuit);


class SensorLockGuard
{
public:
    SensorLockGuard()
    {
        LockSensorsCache();
    }
    ~SensorLockGuard()
    {
        UnlockSensorsCache();
    }
};

#endif

