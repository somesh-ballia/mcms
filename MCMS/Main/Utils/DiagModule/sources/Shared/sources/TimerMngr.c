/*============================================================================*/
/*            Copyright ?? 2005 Polycom Israel,Ltd. All rights reserved        */
/*----------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary        */
/* information of Polycom Israel, Ltd. and is protected by law.               */
/* It may not be copied or distributed in any form or medium, disclosed  to   */
/* third parties, reverse engineered or used in any manner without prior      */
/* written authorization from Polycom Israel Ltd.                             */
/*----------------------------------------------------------------------------*/
/* FILE:     	TimerMngr.c                                                   */
/* PROJECT:  	MFA - Shared Module											  */
/* PROGRAMMER:  Eyal Ben-Sasson												  */
/*																			  */
/* DESCRIPTION: This Thread Offers a Timer Awaking Service to all MFA         */
/*				entities.													  */
/*																			  */
/*----------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                            */
/*----------------------------------------------------------------------------*/
/*         |                 |                                       		  */
/*============================================================================*/

#include <string.h>

#include "DiagDataTypes.h"
#include "LinuxSystemCallsApi.h"
#include "TimerMngr.h"
#include "Print.h"
#include "tools.h"
#include "timers.h"


typedef struct _TimerJob
{
	UINT32 ulTicksCount;
	UINT32 ulIntervalTicks;
	UINT32 ulInterval;
	void*  pvContext;
	void  (*callback)(void*);
	
}TTimerJob,*PTTimerJob;


typedef struct _TimerInfo
{
	UINT32 		ulActiveTimersNum;
	PTTimerJob 	atJob[TIMER_MAX_JOBS_NUM];
	
}TTimerMngrInfo,*PTTimerMngrInfo;


/* Globals */
TTimerMngrInfo tTimerMngrInfo;


/*============================================================================*/
/* FUNCTION:   TimerMngrInit()											  	  */
/*----------------------------------------------------------------------------*/
/*																	          */
/* PURPOSE : Init the TimerMngr Global data structures.						  */
/*           														          */
/* PARAMETERS:   None.                                                        */
/* RETURN VALUE: None.												          */
/* LIMITATION:   None.                                                        */
/*																			  */
/*============================================================================*/
void TimerMngrInit()
{
	UINT32 ulJobID;
	
	tTimerMngrInfo.ulActiveTimersNum = 0;
	
	for(ulJobID=0;ulJobID<TIMER_MAX_JOBS_NUM;ulJobID++)
	{
		tTimerMngrInfo.atJob[ulJobID] = NULL;
	}
	
}


/*============================================================================*/
/* FUNCTION:   TimerSetJob()										  	  	  */
/*----------------------------------------------------------------------------*/
/*																	          */
/* PURPOSE : Register a Job to the Timer Jobs Array							  */
/*           														          */
/* PARAMETERS:   PTTimerJobReq ptJobReq - Jobs Description Params.            */
/*																			  */
/* RETURN VALUE: JobID - Success.									          */
/*				 TIMER_ERROR... - Failure.									  */
/*																			  */
/* LIMITATION:   VERY IMPORTANT: You Must Save the JobID in order to 		  */
/*								 delete an intervaled Job later !!!			  */
/*																			  */
/*============================================================================*/
UINT32 TimerSetJob(PTTimerJobReq ptJobReq)
{
	UINT32 ulJobID;
	PTTimerJob ptTimerJob;
	
	MfaBoardPrint(TIMER_PRINT,
				  PRINT_LEVEL_ERROR,
				  PRINT_TO_TERMINAL,
				  "(TimerSetJob): Entry (Ticks:%d,isInterval:%d,Context:0x%X,callback:0x%X).",
				  ptJobReq->ulTicksCount,
				  ptJobReq->ulInterval,
				  ptJobReq->pvContext,
				  ptJobReq->callback);
	
	if(tTimerMngrInfo.ulActiveTimersNum >= TIMER_MAX_JOBS_NUM)
	{
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerSetJob): TIMER_ERROR_ALL_JOBS_ACTIVE");
		return TIMER_ERROR_ALL_JOBS_ACTIVE;
	}
	
	ptTimerJob = (PTTimerJob)malloc(sizeof(TTimerJob));
	if(ptTimerJob == NULL)
	{
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerSetJob): TIMER_ERROR_MALLOC_FAILED");
		return TIMER_ERROR_MALLOC_FAILED;
	}

	memset((void*)ptTimerJob,0,sizeof(TTimerJob));
	
	// Copy new Job Request		
	ptTimerJob->ulTicksCount = ptJobReq->ulTicksCount;
	
	if( ptJobReq->ulInterval == TIMER_TRUE)
	{
		ptTimerJob->ulInterval = TIMER_TRUE;
		ptTimerJob->ulIntervalTicks = ptJobReq->ulTicksCount;
	}
	
	if( ptJobReq->callback != NULL)
	{
		ptTimerJob->callback = ptJobReq->callback;
		ptTimerJob->pvContext = ptJobReq->pvContext;
	}
	else
	{
		free(ptTimerJob);
		return TIMER_ERROR_INVALID_PARAM;		
	}
		
	for(ulJobID=0;ulJobID<TIMER_MAX_JOBS_NUM;ulJobID++)
	{
		if( tTimerMngrInfo.atJob[ulJobID] == NULL )
		{
			tTimerMngrInfo.atJob[ulJobID] = ptTimerJob;
			break;
		}
	}
	
	if(ulJobID == TIMER_MAX_JOBS_NUM)
	{
		free(ptTimerJob);
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerSetJob): TIMER_ERROR_JOB_ARRAY_FULL");
		return TIMER_ERROR_JOB_ARRAY_FULL;
	}
	else
	{
		tTimerMngrInfo.ulActiveTimersNum++;
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerSetJob): tTimerMngrInfo.ulActiveTimersNum = %d , JobID = %d",tTimerMngrInfo.ulActiveTimersNum,ulJobID);
	}
	
	return ulJobID;
}

/*============================================================================*/
/* FUNCTION:   TimerDeleteJob()										  	  	  */
/*----------------------------------------------------------------------------*/
/*																	          */
/* PURPOSE : Delete a Timer Job from Jobs Array.							  */
/*           														          */
/* PARAMETERS:   UINT32 ulJobID - index to the Timer Jobs Array.              */
/*																			  */
/* RETURN VALUE: TIMER_OK - Success.								          */
/*				 TIMER_ERROR... - Failure									  */
/*																			  */
/* LIMITATION:   None.                                                        */
/*																			  */
/*============================================================================*/
UINT32 TimerDeleteJob(UINT32 ulJobID)
{
	UINT32 index;
	PTTimerJob ptTimerJob;
	
	MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerDeleteJob): Entry (JobID:%d).",ulJobID);
	
	if(tTimerMngrInfo.ulActiveTimersNum == 0)
	{
		if( tTimerMngrInfo.atJob[ulJobID] != NULL )
		{
			free(tTimerMngrInfo.atJob[ulJobID]);
			tTimerMngrInfo.atJob[ulJobID] = NULL;
		}
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerDeleteJob): TIMER_ERROR_NO_JOBS_ACTIVE");
		return TIMER_ERROR_NO_JOBS_ACTIVE; // No Active Timers
	}
	

	if(ulJobID >= TIMER_MAX_JOBS_NUM)
	{
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerDeleteJob): TIMER_ERROR_INVALID_PARAM");
		return TIMER_ERROR_INVALID_PARAM; // Invalid JobID
	}

		
	if( tTimerMngrInfo.atJob[ulJobID] != NULL )
	{
		free(tTimerMngrInfo.atJob[ulJobID]);
		tTimerMngrInfo.atJob[ulJobID] = NULL;
	}
	else
	{
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerDeleteJob): TIMER_ERROR_JOB_ALREADY_DELETED");
		return TIMER_ERROR_JOB_ALREADY_DELETED; // Job already deleted
	}
	
	tTimerMngrInfo.ulActiveTimersNum--;
	
	return TIMER_OK;
}


/*============================================================================*/
/* FUNCTION:   TimerResetInterval(UINT32 ulJobID, UINT32 ulInterval) 	  	  */
/*----------------------------------------------------------------------------*/
/*																	          */
/* PURPOSE : Set/Reset the timer interval of the requested job.				  */
/*           														          */
/* PARAMETERS:   UINT32 ulJobID - index to the Timer Jobs Array.              */
/*				 UINT32 ulInterval	  										  */
/* RETURN VALUE: TIMER_OK - Success.								          */
/*				 TIMER_ERROR... - Failure									  */
/*																			  */
/* LIMITATION:   None.                                                        */
/*																			  */
/*============================================================================*/
UINT32 TimerResetInterval(UINT32 ulJobID, UINT32 ulInterval)
{
	UINT32 index;
	PTTimerJob ptTimerJob;
	
	MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerResetInterval): Entry (JobID:%d).",ulJobID);
	
	if(ulJobID >= TIMER_MAX_JOBS_NUM)
	{
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerResetInterval): TIMER_ERROR_INVALID_PARAM");
		return TIMER_ERROR_INVALID_PARAM; // Invalid JobID
	}

		
	if( tTimerMngrInfo.atJob[ulJobID] != NULL )
	{
		tTimerMngrInfo.atJob[ulJobID]->ulInterval = ulInterval;
	}
	else
	{
		MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerResetInterval): TIMER_ERROR_JOB_ALREADY_DELETED");
		return TIMER_ERROR_JOB_ALREADY_DELETED; // Job already deleted
	}
	
	tTimerMngrInfo.ulActiveTimersNum--;
	
	return TIMER_OK;
}

/*============================================================================*/
/* FUNCTION:   TimerMngrThread()										  	  */
/*----------------------------------------------------------------------------*/
/*																	          */
/* PURPOSE : The Main Timer Loop - awakens every clock tick,				  */
/*			 And looks for registered Jobs.									  */
/*           														          */
/* PARAMETERS:   None.                                                        */
/* RETURN VALUE: None.												          */
/* LIMITATION:   None.                                                        */
/*																			  */
/*============================================================================*/
void TimerMngrThread()
{
	UINT32 ulJobID;
	
	EnrollInThreadList(eTimerMngrThread);

	MfaBoardPrint(TIMER_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"(TimerMngrThread): Entry.");
	
	TimerMngrInit();
	
	while(g_isServiceRun)
	{
		if(tTimerMngrInfo.ulActiveTimersNum > 0)
		{
			for(ulJobID=0;ulJobID<TIMER_MAX_JOBS_NUM;ulJobID++)
			{
				PTTimerJob ptTimerJob = tTimerMngrInfo.atJob[ulJobID];
				
				if(ptTimerJob==NULL)
					continue;
					
				if(--ptTimerJob->ulTicksCount == 0) // Timer Job elapsed
				{
					MfaBoardPrint(TIMER_PRINT,
								  PRINT_LEVEL_MAJOR,
								  PRINT_TO_TERMINAL,
								  "(TimerMngrThread): Timer%d Elapsed (IntervalTicks:%d,isInterval:%d,context:0x%X,callback:0x%X).",
								  ulJobID,
								  ptTimerJob->ulIntervalTicks,
								  ptTimerJob->ulInterval,
								  ptTimerJob->pvContext,
								  ptTimerJob->callback);
					
					
					// Call the calback function
					if(ptTimerJob->callback)
						ptTimerJob->callback((void*)ptTimerJob->pvContext);
					
					// if this job should be intervaled - reset the counter
					if(ptTimerJob->ulInterval)
					{
						ptTimerJob->ulTicksCount = ptTimerJob->ulIntervalTicks;
					}
					else
					{
						TimerDeleteJob(ulJobID);
					}
				}	
				
			} // for
		} // if > 0
		
		EmbSleep(0);
	
	} // while
}

