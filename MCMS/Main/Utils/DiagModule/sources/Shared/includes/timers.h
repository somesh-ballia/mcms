/*
*****************************************************************************
*
* Copyright (C) 2005 POLYCOM NETWORKS Ltd.
* This file contains confidential information proprietary to POLYCOM NETWORKSO
*  Ltd. The use or disclosure of any information contained
* in this file without the written consent of an officer of POLYCOM NETWORKS
* Ltd is expressly forbidden.
*
*****************************************************************************

*****************************************************************************

 Module Name: LinuxSystemCallsApi.h

 General Description:  Module "" contains:

      1. 

 Generated By:	Yigal Mizrahi       Date: 26.4.2005

*****************************************************************************/
#ifndef _TIMERS_H_
#define _TIMERS_H_

/***** Include Files *****/
#include "DiagDataTypes.h"
#include "sys/time.h"
#include "time.h"
#include "signal.h"
#include <stdio.h>
#include "stdlib.h"




/***** Public Variables *****/

/***** Global Variables *****/
//typedef void (*sighandler_t)(); 
/***** Public Functions Declarations *****/
extern INT32 EmbSleep(UINT32 unMiliSeconds);
extern INT32 EmbTimers(UINT32 unInterval , UINT32 unTime , void* pvFuncPtr);
/***** Private Functions Declarations *****/




#endif //_TIMERS_H_
