#ifndef __GET_SERIAL_NUMBER_H__
#define __GET_SERIAL_NUMBER_H__

#include <stdlib.h>

typedef int (*GetSerialNumberFuncType)(char buf[], size_t size, char err_buf[], size_t err_size);

extern GetSerialNumberFuncType GetSerialNumber;

#endif

