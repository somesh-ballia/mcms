#ifndef __GET_SYS_TEMPERATUR_H__
#define __GET_SYS_TEMPERATUR_H__

#include <string>
using std::string;

typedef bool (*GetSysTemperatureFunc)(string &);

extern GetSysTemperatureFunc GetHDDTemperature;

#endif

