#include "sensor_read_cache.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>
#include <unistd.h>
#include "tools.h"

pthread_mutex_t s_cacheMutex;// = PTHREAD_MUTEX_INITIALIZER;

inline unsigned long long monotonic_ms(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ULL + tv.tv_usec / 1000;
}
inline void conditional_wait(time_t interval, unsigned tick, int volatile * quit)
{
    unsigned long long const msBegin = monotonic_ms();
    while (!quit)
    {
        usleep(tick*1000);
        unsigned long long const msNow = monotonic_ms();
        unsigned const diff = msNow - msBegin;
        if (diff>(unsigned)interval*1000)
        {
            break;
        }
    }
}

typedef void * (*MYTHREADFN)(void*);
typedef struct SensorReadCacheParam
{
    int volatile * bQuit;
} s_param;


void * IpmiReadCache(void *pParam)
{
    EnrollInThreadList(eIpmiReadCacheThread);

    while (g_isServiceRun)
    {
        {
            FILE * const fp = popen("/usr/bin/ipmitool sensor > "SENSOR_READINGS_CACHE_FILE_FIRST, "r");
            if (!fp) continue;
            if(fp)
            {
                pclose(fp);
		}
        }

        {
            LockSensorsCache();
            FILE * const fp = popen("mv "SENSOR_READINGS_CACHE_FILE_FIRST" "SENSOR_READINGS_CACHE_FILE_SECOND, "r");
            if(fp)
            {
                pclose(fp);
		}
		UnlockSensorsCache();
        }

        conditional_wait(5, 100, 0);//pReadParam->bQuit);
    }

    return NULL;
}

void InitSensorsCacheLock()
{
    pthread_mutex_init(&s_cacheMutex, NULL);
}

void DestroySensorsCacheLock()
{
    pthread_mutex_destroy(&s_cacheMutex);
}

void LockSensorsCache()
{
    pthread_mutex_lock(&s_cacheMutex);
}

void UnlockSensorsCache()
{
    pthread_mutex_unlock(&s_cacheMutex);
}
#if 0
int StartSensorReadCache(pthread_t * tid, int * bQuit)
{
    *bQuit = 0;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    s_param.bQuit = bQuit;
    int const rv = pthread_create(tid, &attr, (MYTHREADFN)IpmiReadCache, &s_param);
    if (0!=rv)
    {
        return rv;
    }

    return 0;
}

void WaitForSensorReadCache(pthread_t tid, int * bQuit)
{
    *bQuit = 1;
    pthread_join(tid, NULL);
}
#endif

