#ifndef SYSSTATE_H
#define SYSSTATE_H
#include "DiagDataTypes.h"
#include "TimerMngr.h"

extern TTimerJobReq tCheckCPUReq;

extern void CheckCPUUsage(void *p);

extern int GetCPUUsage();
extern int GetMemUsage();
	
#endif
