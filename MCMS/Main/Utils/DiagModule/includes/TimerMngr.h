/*============================================================================*/
/*            Copyright © 2005 Polycom Israel,Ltd. All rights reserved        */
/*----------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary        */
/* information of Polycom Israel, Ltd. and is protected by law.               */
/* It may not be copied or distributed in any form or medium, disclosed  to   */
/* third parties, reverse engineered or used in any manner without prior      */
/* written authorization from Polycom Israel Ltd.                             */
/*----------------------------------------------------------------------------*/
/* FILE:     	TimerMngr.h                                                   */
/* PROJECT:  	MFA - Shared Module											  */
/* PROGRAMMER:  Eyal Ben-Sasson												  */
/* DESCRIPTION: TimerMngr General Definitions.			                      */
/*----------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                            */
/*----------------------------------------------------------------------------*/
/*         |                 |                                       		  */
/*============================================================================*/


#ifndef TIMERMNGR_H_
#define TIMERMNGR_H_


#define TIMER_SLEEP 	1 // 1 tick - 1 msec

#define TIMER_TRUE 		1
#define TIMER_FALSE		0

#define TIMER_OK		1
#define TIMER_FAILURE	0


#define TIMER_MAX_JOBS_NUM 	FD_SETSIZE

/* TimerMngr Error Codes */
#define TIMER_ERROR_ALL_JOBS_ACTIVE			0xFFFF0100
#define TIMER_ERROR_MALLOC_FAILED			0xFFFF0101
#define TIMER_ERROR_JOB_ARRAY_FULL			0xFFFF0102
#define TIMER_ERROR_JOB_ALREADY_DELETED 	0xFFFF0103
#define TIMER_ERROR_INVALID_PARAM			0xFFFF0104
#define TIMER_ERROR_NO_JOBS_ACTIVE			0xFFFF0105


typedef struct _TimerJobReq
{
	UINT32 ulTicksCount;
	UINT32 ulInterval;
	void*  pvContext;
	void  (*callback)(void*);
	
}TTimerJobReq,*PTTimerJobReq;

/* Exported Functions */
UINT32 TimerSetJob		(PTTimerJobReq ptJobReq);
UINT32 TimerDeleteJob	(UINT32 ulJobID);


#endif /*TIMERMNGR_H_*/
