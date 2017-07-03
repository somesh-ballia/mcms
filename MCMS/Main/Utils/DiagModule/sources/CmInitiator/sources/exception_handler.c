/*===================================================================================================================*/
/*            Copyright     ???    2001  Polycom Israel, Ltd. All rights reserved                                    */
/*-------------------------------------------------------------------------------------------------------------------*/
/* NOTE: This software contains valuable trade secrets and proprietary   information of                              */
/* Polycom Israel, Ltd. and is protected by law.  It may not be copied or distributed in any form                    */
/* or medium, disclosed  to third parties, reverse engineered or used in any manner without                          */
/* prior written authorization from Polycom Israel Ltd.                                                              */
/*-------------------------------------------------------------------------------------------------------------------*/
/* FILE:     exception_handler.c                                                                                   */
/* PROJECT:  Gideon CM                                                                                               */
/* PROGRAMMER:  Eyal Ben-Sasson                                            .                                            */
/*                                                                                                                   */
/*-------------------------------------------------------------------------------------------------------------------*/
/* Who     |      Date       |         Description                                                                   */
/*-------------------------------------------------------------------------------------------------------------------*/
/*              |                     |                                                                              */
/*===================================================================================================================*/


#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "SharedDefines.h"
#include "LinuxSystemCallsApi.h"

#include "Print.h"

// t.r for getting the tid
//#include <sys/types.h>
//#include <linux/unistd.h>
//_syscall0(pid_t,gettid)

#define SWITCH_INFO_LOG_NAME 	"switchinfo"
#define SWITCH_INFO_DIR 	 	"/mnt/mfa_cm_fs/switchlogs"

static char acLogPath[100];
static char acLogDir[100];
static FILE* fp_crashInfo;

APIUBOOL	isCoreDumpMode = NO;

		
/* Obtain a backtrace and print it to stdout. */
void print_trace (FILE* fp)
{
	void *array[10];
	size_t size;
	char **strings;
	size_t i;
	 
	size = backtrace (array, 10);
	strings = backtrace_symbols (array, size);
	  
	fprintf(fp,"(print_trace): Obtained %zd stack frames.\n", size);
	  
	for (i = 0; i < size; i++)
	   fprintf(fp,"%s\n", strings[i]);
	     
	fprintf(fp,"(print_trace): end !!!\n");
	
	free (strings);
}


/* The fault handler function */
void exception_handler (int signum)
{
	char acCommand[100];
	unsigned int CurrentThreadId;
	unsigned int ulThreadIndex; 
	unsigned int i;

	
	// t.r 
	CurrentThreadId = gettid();	
	for(ulThreadIndex=0; ulThreadIndex<eMaxThread; ulThreadIndex++)
	{
		if(tThreadDesc[ulThreadIndex].ul_ThreadId == CurrentThreadId)
			break;
	}		
	
	printf("(exception_handler): signum:%d.\n",signum);
	
	switch(signum)
	{
		case SIGSEGV:
		case SIGILL:
//		case SIGABRT:
		case SIGBUS:
		case SIGSYS:
		case SIGTERM:
		case SIGQUIT:
		case SIGUSR1:
		case SIGUSR2:
		case SIGTSTP:
		{
			memset(acCommand, 0, 100);
		    sprintf(acCommand,"cp /proc/%d/maps %s/maps",CurrentThreadId,SWITCH_INFO_DIR);
			system(acCommand);
			
			sprintf(acLogPath,"%s/%s.log",SWITCH_INFO_DIR,SWITCH_INFO_LOG_NAME);
			fp_crashInfo = fopen(acLogPath,"w");
			if(fp_crashInfo == NULL)
				printf("*** (exception_handler): Failed Open %s !!! error = %d ***\n",acLogPath, errno);

			if(fp_crashInfo != NULL)
			{	
				fprintf(fp_crashInfo,"\n(exception_handler): signum:%d.\n",signum);

				if(ulThreadIndex < eMaxThread)
					fprintf(fp_crashInfo,"(exception_handler): thread: %s, threadId %d, threadPriority 0x%x.\n", tThreadDesc[ulThreadIndex].c_ThreadName, CurrentThreadId, tThreadDesc[ulThreadIndex].l_ThreadPriority);
				else
					fprintf(fp_crashInfo,"(exception_handler): threadId %d (not registered in threads table).\n", CurrentThreadId);

				fprintf(fp_crashInfo,"threadsList:\n");
				for(i=0; i<eMaxThread; i++)
					fprintf(fp_crashInfo, " %d. %s  ThreadId: %d, Thread Priority: %d\n", i, tThreadDesc[i].c_ThreadName, tThreadDesc[i].ul_ThreadId, tThreadDesc[i].l_ThreadPriority);

			    fprintf(fp_crashInfo, "\n\n");

			  	print_trace(fp_crashInfo);
				fclose(fp_crashInfo);	
			}

			memset(acCommand, 0, 100);
			sprintf(acCommand, "rm %s/dmesg", SWITCH_INFO_DIR);
			system(acCommand);

			memset(acCommand, 0, 100);
		    sprintf(acCommand,"dmesg > %s/dmesg", SWITCH_INFO_DIR);
			system(acCommand);

		}break;
	}
	
	abort();
	
  	exit(-1);
}

int RegisterExceptions()
{

	isCoreDumpMode = NO;

	//	create directory if needed ...
	sprintf(acLogDir,"mkdir -p %s",SWITCH_INFO_DIR);
	system(acLogDir);

	signal(SIGSEGV, exception_handler);	//illegal memory access
	signal(SIGILL, exception_handler); // sig4 - illegal instruction
	signal(SIGQUIT, exception_handler); // sig3 - break
//	signal(SIGABRT, exception_handler); //sig6 - abnormal termination
	signal(SIGBUS, exception_handler); //sig7 - bus error
	signal(SIGSYS, exception_handler); //sig31 - bad system call
	signal(SIGTERM, exception_handler); //sig15 - process termintaion
	signal(SIGUSR1, exception_handler); //sig10 - process termintaion
	signal(SIGUSR2, exception_handler); //sig12 - process termintaion
	signal(SIGTSTP, exception_handler); //sig20 - process termintaion

	return 0;
}


int UnRegisterExceptions()
{
	UINT32 pid; 
	UINT8 acTmpFileName[50];
	FILE *fp;

	isCoreDumpMode = YES;
	//	create directory if needed ...
//	sprintf(acLogDir,"mkdir -p %s",MFA_LOG_INFO_DIR);
//	system(acLogDir);

	signal(SIGSEGV, SIG_DFL);	//illegal memory access
	signal(SIGILL, 	SIG_DFL); // sig4 - illegal instruction
	signal(SIGQUIT, SIG_DFL); // sig3 - break
//	signal(SIGABRT, SIG_DFL); //sig6 - abnormal termination
	signal(SIGBUS, 	SIG_DFL); //sig7 - bus error
	signal(SIGSYS, 	SIG_DFL); //sig31 - bad system call
//	signal(SIGTERM, SIG_DFL); //sig15 - process termintaion
	signal(SIGUSR1, SIG_DFL); //sig10 - process termintaion
	signal(SIGUSR2, SIG_DFL); //sig12 - process termintaion
	signal(SIGTSTP, SIG_DFL); //sig20 - process termintaion

	return 0;
}
