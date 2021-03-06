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

 Module Name: LinuxSystemCallsApi.c

 General Description:  Module "" contains:

      1. 

 Generated By:	Yigal Mizrahi       Date: 26.4.2005

*****************************************************************************/

/***** Include Files *****/
#include <unistd.h>
#include "timers.h"
#include "Print.h"


/***** Public Variables *****/

/***** Global Variables *****/

/***** Public Functions Declarations *****/

/***** Private Functions Declarations *****/

/***** Functions Code *****/


INT32 EmbSleep(UINT32 unMiliSeconds)
{
	/*
	const struct timespec tReqTimeSpec;
	struct timespec tRemTimeSpec;
	
	
	tReqTimeSpec.tv_sec = 0;
	tReqTimeSpec.tv_nsec = unMiliSeconds * 1000000000;
	printf("tReqTimeSpec.tv_nsec = %d\n",tReqTimeSpec.tv_nsec);
	*/
	//return (nanosleep(&tReqTimeSpec,&tRemTimeSpec));
	return (usleep(unMiliSeconds * 1000));
}


INT32 EmbTimers (UINT32 unInterval , UINT32 unTime , void* pvFuncPtr)
{
	struct timeval timeInterval = {0,unInterval * 1000};
	struct timeval timeTimer = {0,unTime * 1000};
	struct itimerval itTimer = {timeInterval,timeTimer};
	INT32 nRc;
	/*
	struct sigaction sact;
    
    sigemptyset( &sact.sa_mask );
    sact.sa_flags = 0;
    sact.sa_handler = yyy;
    sigaction( SIGALRM, &sact, NULL );
    */
    //attach the func to jump when timer is finish
    signal(SIGALRM,pvFuncPtr);
    
    nRc = setitimer(ITIMER_REAL, &itTimer,NULL);
    
    return nRc;
}

