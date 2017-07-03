#include "DiagnosticsErrHandle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
	

errList* errAnchor = 0;
errList* errTail = 0;
pthread_mutex_t errMutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * What we can do with error log,is to add to it (linked list), clear it,
 * or request specific error from it (by number)
 * */


//adding new error (with error id and description) to errors linked list
void errReportError(int errNum,char* ErrorDesc)
{
	char 	errString[200];
	errList *newErrorLog = (errList *)malloc(sizeof(errList));
	printf("test id %d had error: %s\n",errNum,ErrorDesc);
	
	if (newErrorLog)
	{
		newErrorLog->errDescr = (char*)malloc((strlen(ErrorDesc) + 2)*sizeof(char));
		if (newErrorLog->errDescr)
		{
			strcpy(newErrorLog->errDescr,ErrorDesc);
			newErrorLog->errTestId = errNum;
			newErrorLog->nextError = NULL;
			pthread_mutex_lock (&errMutex);
			if ((errTail == NULL ) || ( errAnchor== NULL)) //tail and anchor are empty = empty list
			{
				errTail = newErrorLog;
				errAnchor = newErrorLog;
			}
			else
			{  //adding new error in the tail
				errTail->nextError = newErrorLog;
				errTail = newErrorLog;
			}
			pthread_mutex_unlock (&errMutex);
		}
		else
			free(newErrorLog);
	}
	
	snprintf(errString, sizeof(errString), "Test ID = %d: %s", errNum, ErrorDesc);
	//WriteLog(errString);
	
	return;
}


void errClearErrorLog(void)
{
	errList *currentErrorLog;
	
	currentErrorLog = errAnchor;
	pthread_mutex_lock (&errMutex);
	while (errAnchor)
	{
		currentErrorLog = errAnchor->nextError;
		free(errAnchor->errDescr);
		free(errAnchor);
		errAnchor = currentErrorLog;
	}
	errTail = 0;
	pthread_mutex_unlock(&errMutex);
}



//returns indexed error. Remember to allocate sting part of err,before calling this
int  errGetError(int errIndex,errList** errDescription) //if returns 0 - no such error in log
{
	errList *currentErrorLog;
	int i;
	
	
	
	pthread_mutex_lock (&errMutex);
	currentErrorLog = errAnchor;
   
	for (i=0 ; i<errIndex; i++)
	{
		if (currentErrorLog)
			currentErrorLog = currentErrorLog -> nextError;
 	}
	pthread_mutex_unlock (&errMutex);
	
	if (currentErrorLog == 0) //didnt find our error
		return 0;
	/*
	errDescription->errDescr = (char*)malloc(strlen(currentErrorLog->errDescr)*sizeof(char));
	strcpy(errDescription->errDescr,currentErrorLog->errDescr);
	errDescription->errTestId = currentErrorLog->errTestId;
	errDescription->nextError = 0;
	*/
	*errDescription = currentErrorLog;
	return 1;
}
