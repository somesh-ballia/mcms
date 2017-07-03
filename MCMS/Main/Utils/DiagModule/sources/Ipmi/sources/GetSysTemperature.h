#ifndef __GET_SYS_TEMPERATUR_H__
#define __GET_SYS_TEMPERATUR_H__

typedef BOOL (*GetSysTemperatureFunc)(int *);

extern GetSysTemperatureFunc GetHDDTemperature;

#endif

