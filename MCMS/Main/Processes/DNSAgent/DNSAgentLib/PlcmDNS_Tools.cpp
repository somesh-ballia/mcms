// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:                PlcmDNS_Tools.cpp  
// 
// ==========================================



#include <memory.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <string.h>

#include <math.h>
#include <time.h>
#include <sys/timeb.h>

#include <unistd.h>

#include <sys/times.h> 
#include <sys/wait.h>
#include <pthread.h>


//#include <netinet/in.h>
//#include <arpa/nameser.h>
//#include <resolv.h>

#include "PlcmDNS_Tools.h"            //
#include "Trace.h"
#include "PlcmDNS_Network.h" 
#include "PlcmDNS_Packet.h"
#include "PlcmDNS_Processing.h"

#include "StateMachine.h"
#include "Segment.h"

#include "SystemFunctions.h"
#include "OpcodesMcmsInternal.h"
#include "OpcodesMcmsCommon.h"
#include "IpCsOpcodes.h"
#include "Trace.h"
#include "DNSAgentProcess.h"
#include "Segment.h"
#include "DNSAgentManagerStructs.h"
#include "CsCommonStructs.h"
#include "MplMcmsProtocol.h"
#include "FaultsContainer.h"
#include "StatusesGeneral.h"
#include "SystemQuery.h"
#include "FaultsDefines.h"
#include "DNSAgentStatuses.h"
#include "TraceStream.h"
#include "ManagerApi.h"
#include "SipProxyTaskApi.h"
#include "GkTaskApi.h"
#include  "NStream.h"
#include "IpCsDnsApi.h"
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------

extern void vDebugPrintAddrList_String(ipAddressStruct * aDnsAddrr, unsigned int dwArrLen, string * par_cSrting);


PBEGIN_MESSAGE_MAP(cDNS_PLCM_REQ_NODE)
	ONEVENT(DNS_REQ_TIMER_TOUT				,ANYCASE 	,cDNS_PLCM_REQ_NODE::vTimerArrived)
PEND_MESSAGE_MAP(cDNS_PLCM_REQ_NODE, CStateMachine);


extern int pthread_mutexattr_settype (pthread_mutexattr_t *__attr, int __kind);
extern int pthread_mutexattr_setkind_np (pthread_mutexattr_t *__attr, int __kind);


static pthread_key_t CurrentThreadKey = 0;

void	* plcmThreadWrapper(void*);

//===============================================================================//
unsigned int Pm_aResolutionCoeff[E_TIMER_RESOLUTION_UPPER] =
{     0  //E_TIMER_RESOLUTION_LOW
, 60*60  //E_TIMER_RESOLUTION_HOUR   
, 60     //E_TIMER_RESOLUTION_MIN
, 1      //E_TIMER__RESOLUTION_SEC
, 1000   //E_TIMER_RESOLUTION_MILI
, 1000000//E_TIMER_RESOLUTION_MICRO
};       //E_RESOLUTION_UP
unsigned int Pm_GetTimeString(  eTIMER_RESOLUTION   par_eTimerResolution //IN
							  , eTIMER_TIME_FORMAT  par_eTimeFormat      //IN 
							  , unsigned int        par_nStrSize         //IN
							  , char *              par_szTimeString)    //OUT
//-------------------------------------------------------------------------------//
// Function purpose:
//   This function builds simple string as it's desired 
//   ( including '\0' symbol )
// Arguments: 
//   IN par_eTimerResolution - desired resolution ( accuracy )
//   IN par_eTimeFormat      - desired format ( what is a time component the time will be 
//                             represented )
//   IN par_nStrSize         - maximal output string size
//   OUT par_szTimeString    - time string
// Returned Value:
//      Number of bytes in the string including '\0' symbol - 
//      It's equal to zero in case of desired string size is less 
//      than received string length
// Function algorithm:
//   Get current time from the system
//   Build date part : it will look like { dd.mm.yy,dd.mm,dd } or be empty
//   Build day time part : it will look like 
//                        { hh,hh:mm,hh:mm:ss,hh:mm:ss:mili,hh:mm:ss:micro }   
//   Calculate string length
//   Copy received string into output parameter
// Side effects:
//   None 
//   Under Windows this function doesn't work with LKE_TIMER_RESOLUTION_MICRO
//   In this case 000000 will be printed into the string as a microseconds number
//-------------------------------------------------------------------------------//
{
	time_t              CurrentTime = 0;;
	struct tm           sDepackedTime;
	struct timeb        sCurrentTimeb;
	struct timeval      sCurrentTimeval;
	struct timezone     sTempTZ;
	unsigned int        nRetVal = 0;
	char                szTempStr[512]="", *pTempStr = NULL;
	unsigned int        dwTempLen = sizeof(szTempStr);

	CurrentTime = time(NULL);

	switch (par_eTimerResolution) 
	{
	case E_TIMER_RESOLUTION_MILI  :
		ftime(&sCurrentTimeb);
		CurrentTime = sCurrentTimeb.time;
		break; 

	case E_TIMER_RESOLUTION_MICRO :
		gettimeofday (	&sCurrentTimeval, &sTempTZ );				       
		break;

	default :
		//CurrentTime = time(&CurrentTime);
		break;
	}

	sDepackedTime = *gmtime(&CurrentTime); //BRIDGE-3559 change to gmttime to match MCMSlogs

	pTempStr = szTempStr;
	dwTempLen = sizeof(szTempStr)-1;
	switch (par_eTimeFormat)
	{
	case E_TIMER_FORMAT_HOUR  :
		*pTempStr = '\0';
		break;

	case E_TIMER_FORMAT_DAY   :
		snprintf(pTempStr, dwTempLen, "%02d ", sDepackedTime.tm_mday);
		break;

	case E_TIMER_FORMAT_MONTH :
		snprintf(pTempStr, dwTempLen, "%02d/%02d "
			,sDepackedTime.tm_mday
			,(sDepackedTime.tm_mon + 1)
			);
		break;

	case E_TIMER_FORMAT_YEAR  :
		snprintf(pTempStr, dwTempLen, "%02d/%02d/%02d "
			,sDepackedTime.tm_mday
			,(sDepackedTime.tm_mon + 1)
			,(sDepackedTime.tm_year % 100)
			);
		break;

	default :
		break;
	}

	szTempStr[sizeof(szTempStr)-1]='\0';

	pTempStr = szTempStr + strlen(szTempStr);
	dwTempLen = sizeof(szTempStr) - (min(sizeof(szTempStr), strlen(szTempStr)));

	if(0 < dwTempLen)
	{
		
		switch(par_eTimerResolution ) 
		{
		case E_TIMER_RESOLUTION_MIN   :
			snprintf(pTempStr, dwTempLen-1,"%02d:%02d"
				,sDepackedTime.tm_hour
				,sDepackedTime.tm_min
				);
			break;

		case E_TIMER_RESOLUTION_SEC  :
			snprintf(pTempStr, dwTempLen-1,"%02d:%02d:%02d"
				,sDepackedTime.tm_hour
				,sDepackedTime.tm_min
				,sDepackedTime.tm_sec
				);
			break;

		case E_TIMER_RESOLUTION_MILI  :
			snprintf(pTempStr, dwTempLen-1, "%02d:%02d:%02d.%03d"
				,sDepackedTime.tm_hour
				,sDepackedTime.tm_min
				,sDepackedTime.tm_sec
				,sCurrentTimeb.millitm
				);
			break; 

		case E_TIMER_RESOLUTION_MICRO :
			snprintf(pTempStr, dwTempLen-1, "%02d:%02d:%02d.%06li"
				,sDepackedTime.tm_hour
				,sDepackedTime.tm_min
				,sDepackedTime.tm_sec
				,sCurrentTimeval.tv_usec
				);
			break; 

		default :
			break;
		}

		szTempStr[sizeof(szTempStr)-1]='\0';
	}

	nRetVal = strlen(szTempStr) + 1;
	if (nRetVal > par_nStrSize)
	{
		nRetVal = 0;
	}
	else
	{
		strncpy(par_szTimeString, szTempStr, par_nStrSize-1);
	}   
	return   nRetVal;

}//Pm_GetTimeString
//===============================================================================//


unsigned int Pm_getCurrentTimestampAdv(eTIMER_RESOLUTION    par_eTimerResolution  //IN
									   ,unsigned int      *  par_tm_pTimeSec      //OUT
									   ,unsigned short    *  par_n_pMilliSec      //OUT 
									   )
//==============================================================//
// Function purpose:
//   The function calculates current timer value according to the system timer
// Arguments: 
//   IN - desired resolution
// Returned Value:
// Side effects:
//   None 
// Note:
//   Under Windows this function doesn't work with E_TIMER_RESOLUTION_MICRO
//   In this case it returns 0;
//===============================================================//
{
	struct timeb	sCurrentTimeb;
	struct timeval	sCurrentTimeval;
	struct timezone sTempTZ;//unusable
	unsigned int	nReturnTimestamp = 0;

	ftime(&sCurrentTimeb);

	switch ( par_eTimerResolution ) 
	{
	default :
	case E_TIMER_RESOLUTION_HOUR  :
	case E_TIMER_RESOLUTION_MIN   :
	case E_TIMER_RESOLUTION_SEC  :
		nReturnTimestamp = (unsigned int)
			(  sCurrentTimeb.time / Pm_aResolutionCoeff [par_eTimerResolution]
		);
		if (NULL != par_tm_pTimeSec)
		{
			*par_tm_pTimeSec = (unsigned int)(  sCurrentTimeb.time
				- (sCurrentTimeb.timezone * 60)
				);
		}
		if (NULL != par_n_pMilliSec)
		{
			*par_n_pMilliSec = 0;
		}
		break;      

	case E_TIMER_RESOLUTION_MILI  :
		nReturnTimestamp = (unsigned int)( sCurrentTimeb.time * 
			Pm_aResolutionCoeff [par_eTimerResolution] );
		nReturnTimestamp += sCurrentTimeb.millitm;
		if (NULL != par_tm_pTimeSec)
		{
			*par_tm_pTimeSec = (unsigned int)(  sCurrentTimeb.time
				- (sCurrentTimeb.timezone * 60)
				);
		}
		if (NULL != par_n_pMilliSec)
		{
			*par_n_pMilliSec = sCurrentTimeb.millitm;
		}
		break; 

	case E_TIMER_RESOLUTION_MICRO :
		gettimeofday (	&sCurrentTimeval, &sTempTZ );				       
		nReturnTimestamp = (unsigned int)( sCurrentTimeval.tv_sec * 
			Pm_aResolutionCoeff [par_eTimerResolution] );
		nReturnTimestamp += sCurrentTimeval.tv_usec;					       
		if (NULL != par_tm_pTimeSec)
		{
			*par_tm_pTimeSec = sCurrentTimeval.tv_sec;
		}
		if (NULL != par_n_pMilliSec)
		{
			*par_n_pMilliSec = (unsigned short)(sCurrentTimeval.tv_usec / 1000);
		}
		break; 
	}

	return nReturnTimestamp;
}//Pm_getCurrentTimestamp


unsigned int Pm_getCurrentTimestamp(eTIMER_RESOLUTION    par_eTimerResolution)   //IN             
{
	return Pm_getCurrentTimestampAdv(par_eTimerResolution, NULL, NULL);
}


unsigned int Pm_GetTimeStringT( unsigned int        par_dwTime           //IN     
							  , eTIMER_RESOLUTION   par_eTimerResolution //IN
							  , eTIMER_TIME_FORMAT  par_eTimeFormat      //IN 
							  , unsigned int        par_nStrSize         //IN
							  , char *              par_szTimeString)    //OUT
							  //-------------------------------------------------------------------------------//
							  // Function purpose:
							  //   This function builds simple string as it's desired 
							  //   ( including '\0' symbol )
							  // Arguments: 
							  //   IN par_eTimerResolution - desired resolution ( accuracy )
							  //   IN par_eTimeFormat      - desired format ( what is a time component the time will be 
							  //                             represented )
							  //   IN par_nStrSize         - maximal output string size
							  //   OUT par_szTimeString    - time string
							  // Returned Value:
							  //      Number of bytes in the string including '\0' symbol - 
							  //      It's equal to zero in case of desired string size is less 
							  //      than received string length
							  // Function algorithm:
							  //   Get current time from the system
							  //   Build date part : it will look like { dd.mm.yy,dd.mm,dd } or be empty
							  //   Build day time part : it will look like 
							  //                        { hh,hh:mm,hh:mm:ss,hh:mm:ss:mili,hh:mm:ss:micro }   
							  //   Calculate string length
							  //   Copy received string into output parameter
							  // Side effects:
							  //   None 
							  //   Under Windows this function doesn't work with LKE_TIMER_RESOLUTION_MICRO
							  //   In this case 000000 will be printed into the string as a microseconds number
							  //-------------------------------------------------------------------------------//
{
	time_t              CurrentTime = 0;
	struct tm           sDepackedTime;
	struct timeb        sCurrentTimeb;
	struct timeval      sCurrentTimeval;
	struct timezone     sTempTZ;
	unsigned int        nRetVal = 0;
	char                szTempStr[512]="", *pTempStr=NULL;
	unsigned int        dwLen = sizeof(szTempStr);

	CurrentTime = par_dwTime;

	switch (par_eTimerResolution) 
	{
	case E_TIMER_RESOLUTION_MILI  :
	case E_TIMER_RESOLUTION_MICRO :

		//ftime(&sCurrentTimeb);
		sCurrentTimeb.time     = par_dwTime;
		sCurrentTimeb.millitm  = 0;
		sCurrentTimeb.timezone = 0;

		CurrentTime = sCurrentTimeb.time;
		break; 

	default :
		//CurrentTime = time(&CurrentTime);
		break;
	}

	sDepackedTime = *gmtime(&CurrentTime); //BRIDGE-3559 change to gmttime to match MCMSlogs

	pTempStr = szTempStr;
	switch (par_eTimeFormat)
	{
	case E_TIMER_FORMAT_HOUR  :
		*pTempStr = '\0';
		break;

	case E_TIMER_FORMAT_DAY   :
		snprintf(pTempStr, dwLen -1,"%02d ", sDepackedTime.tm_mday);
		break;

	case E_TIMER_FORMAT_MONTH :
		snprintf(pTempStr, dwLen -1, "%02d/%02d "
			,sDepackedTime.tm_mday
			,(sDepackedTime.tm_mon + 1)
			);
		break;

	case E_TIMER_FORMAT_YEAR  :
		snprintf(pTempStr, dwLen -1, "%02d/%02d/%02d "
			,sDepackedTime.tm_mday
			,(sDepackedTime.tm_mon + 1)
			,(sDepackedTime.tm_year % 100)
			);
		break;

	default :
		break;
	}

	if((sizeof(szTempStr)-1) > strlen(szTempStr))
		dwLen = sizeof(szTempStr) - strlen(szTempStr);
	else
		dwLen = 0;

	if(0 < (dwLen-1))
	{

		pTempStr = szTempStr + strlen(szTempStr);
		switch ( par_eTimerResolution ) 
		{
		case E_TIMER_RESOLUTION_MIN   :
			snprintf(pTempStr, dwLen-1, "%02d:%02d"
				,sDepackedTime.tm_hour
				,sDepackedTime.tm_min
				);
			break;

		case E_TIMER_RESOLUTION_SEC  :
			snprintf(pTempStr, dwLen-1, "%02d:%02d:%02d"
				,sDepackedTime.tm_hour
				,sDepackedTime.tm_min
				,sDepackedTime.tm_sec
				);
			break;

		case E_TIMER_RESOLUTION_MILI  :
		case E_TIMER_RESOLUTION_MICRO :
			snprintf(pTempStr, dwLen-1, "%02d:%02d:%02d.%03d"
				,sDepackedTime.tm_hour
				,sDepackedTime.tm_min
				,sDepackedTime.tm_sec
				,sCurrentTimeb.millitm
				);
			break; 

		default :
			break;
		}
	}

	nRetVal = strlen(szTempStr) + 1;
	if (nRetVal > par_nStrSize)
	{
		nRetVal = 0;
	}
	else
	{
		strncpy(par_szTimeString, szTempStr, par_nStrSize);
	}   
	return   nRetVal;

}//Pm_GetTimeString
//===============================================================================//

//===============================================================================//





//=========================================================//
int plcmDNS_nGetStrIpFromipAddressStruct(char * par_szOut, int par_OutLen, ipAddressStruct * par_pAddStruct)
{
	int nRc = enIpVersionMAX;
	if(NULL != par_pAddStruct)
	{
		char szIp[128]="";

		if(eIpVersion4 == par_pAddStruct->ipVersion)
		{
			plcmIpToStringV4(par_pAddStruct->addr.v4.ip, szIp, sizeof(szIp)-1);
			nRc = eIpVersion4;
		}
		else
        if(eIpVersion6 == par_pAddStruct->ipVersion)
		{
			plcmIpToStringV6(par_pAddStruct->addr.v6.ip, szIp, sizeof(szIp)-1,FALSE);
			nRc = eIpVersion6;
		}
		else
		{
			memset(szIp, '\0', sizeof(szIp));
			nRc = enIpVersionMAX;
		}

		if((NULL != par_szOut)&&(1 < par_OutLen))
		{
			strncpy(par_szOut, szIp, par_OutLen-1);
		}
	}
	return nRc;
}
//=========================================================//

//=========================================================//
void  plcmPriorToBodyStart  (PLs_THREAD	*   par_pThr)
{
	if(NULL != par_pThr)
	{
		par_pThr->dwMyOwn_PID = getpid();
		pthread_setspecific(CurrentThreadKey, par_pThr);
	}
}
//=========================================================//

//=========================================================//
void  plcmAfterBodyFinished (PLs_THREAD*   par_pThr)
{
	// Mark and signal thread completion //
	par_pThr->ThrState = THREAD_STATE_DEAD;
}
//=========================================================//

//=========================================================//
void* plcmThreadWrapper(void* par_pData) 
{
	PLs_THREAD*    pThr = (PLs_THREAD*)par_pData;
    vTHREAD_BODY   pFun = (vTHREAD_BODY) pThr->func;


	plcmPriorToBodyStart (pThr);

    pFun(pThr);

	plcmAfterBodyFinished (pThr);
	pthread_exit(NULL);
	return 0;
}
//=========================================================//

//=========================================================//
PLb_RETURN  plcmThreadInitDetail(vTHREAD_BODY  par_fpThreadBody
								,void*         par_pThreadData
								,const char*   par_pszName
								,int           par_nPriority
								,size_t        par_StackSize
								,PLs_THREAD*   par_pThr
								) 
{
	PLb_RETURN	nRc = plFAIL;

	if(NULL != par_pThr)
	{
		par_pThr->bEndSignal    = plFAIL;
		par_pThr->dwParent_PID  = getpid();

		par_pThr->handle        = 0;

		if(CurrentThreadKey == 0)
		{
			pthread_key_create(&CurrentThreadKey, 0);
		}
		if(NULL != par_pszName)
		{
			memset (par_pThr->name, 0,            PL_THREAD_NAME_SIZE);
			strncpy(par_pThr->name, par_pszName, (PL_THREAD_NAME_SIZE-1) );
		}
		else
		{
			strncpy(par_pThr->name,PL_THREAD_NAME_DEFAULT, sizeof(par_pThr->name)-1);
		}

		par_pThr->ThrState	  = THREAD_STATE_UNBORN;
		par_pThr->data		  = par_pThreadData;
		par_pThr->func		  = (void*)par_fpThreadBody;
		par_pThr->stackSize   = par_StackSize;
		par_pThr->priority    = par_nPriority;
		par_pThr->dwLabel     = PL_THREAD_LABEL;

		par_pThr->eThrType    = eTHREAD_UNDEF;
		par_pThr->dwMyOwn_PID = 0;

		nRc = plSUCCESS;
	}
	return nRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN   plcmThreadDeinit  (PLs_THREAD*    par_pThr)
{
PLb_RETURN	nRc = plFAIL;

	if(  (NULL != par_pThr)
       &&(PL_THREAD_LABEL == par_pThr->dwLabel)
      )
	{
		if (par_pThr->ThrState != THREAD_STATE_DEAD)
			plcmThreadJoin(par_pThr);

		if(THREAD_STATE_UNBORN == par_pThr->ThrState)
		{
			plcmThreadKill(par_pThr);
		}

		par_pThr->dwLabel = 0;
		par_pThr->bEndSignal = plSUCCESS;
		par_pThr->dwLabel = 0;
		nRc = plSUCCESS;
	}	
	return nRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN   plcmThreadStart (PLs_THREAD * par_pThr)    
{
	PLb_RETURN	nRc = plFAIL;

	if(   (NULL != par_pThr)
	   && (THREAD_STATE_UNBORN == par_pThr->ThrState) 
       && (PL_THREAD_LABEL     == par_pThr->dwLabel )
	  )
	{
		struct sched_param		sp;
		pthread_attr_t			attr;
		int						nR;

		sp.sched_priority = par_pThr->priority;
		pthread_attr_init(&attr);
		pthread_attr_setschedparam(&attr, &sp);

		par_pThr->ThrState = THREAD_STATE_ALIVE;
		nR = pthread_create(&par_pThr->handle
			                ,&attr
						    ,plcmThreadWrapper
						    ,par_pThr);
		if (0 == nR)
		{
			nRc = plSUCCESS;
		}

		par_pThr->eThrType  = eTHREAD_TACH;

		pthread_attr_destroy(&attr);
	}

	return nRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN   plcmThreadStart_detach (PLs_THREAD * par_pThr)    
{
    PLb_RETURN	nRc = plFAIL;

    if(   (NULL != par_pThr)
        && (THREAD_STATE_UNBORN == par_pThr->ThrState) 
        && (PL_THREAD_LABEL     == par_pThr->dwLabel )
        )
    {
        struct sched_param		sp;
        pthread_attr_t			attr;
        int						nR;

        sp.sched_priority = par_pThr->priority;
        pthread_attr_init(&attr);
        pthread_attr_setschedparam (&attr, &sp);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

        par_pThr->ThrState = THREAD_STATE_ALIVE;
        nR = pthread_create(&par_pThr->handle
            ,&attr
            ,plcmThreadWrapper
            ,par_pThr);
        if (0 == nR)
        {
            nRc = plSUCCESS;
        }
        par_pThr->eThrType  = eTHREAD_DETACH;

        pthread_attr_destroy(&attr);
    }

    return nRc;
}
//=========================================================//

//=========================================================//
void plcmSleepMs (PL_INTERVAL_ms par_ms) 
{
	struct timespec		timeout0;
	struct timespec		timeout1;
	struct timespec	  * tmp;
	struct timespec   * t0 = &timeout0;
	struct timespec   * t1 = &timeout1;

	t0->tv_sec = par_ms / 1000;
	t0->tv_nsec = (par_ms % 1000) * (1000 * 1000);
			
	// This bizarre logic is necessary to prevent signals from	//
	// stopping the sleep from completing.						//
	while(nanosleep(t0, t1) == (-1) ) 
	{
		tmp = t0;
		t0  = t1;
		t1  = tmp;
	}
}
//=========================================================//

//=========================================================//
PLb_RETURN plcmTreadSendFinishSignal (PLs_THREAD*    par_pThr)
{
	PLb_RETURN nRc = plFAIL;
	if((NULL != par_pThr)&&(PL_THREAD_LABEL == par_pThr->dwLabel))
	{
      par_pThr->bEndSignal = plSUCCESS;
      nRc = plSUCCESS;
	}
	return nRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN	plcmThreadWaitTillEnd(PLs_THREAD*    par_pThr)
{
	PLb_RETURN nRc = plFAIL;

	if(   (NULL != par_pThr)
		&&(THREAD_STATE_UNBORN != par_pThr->ThrState)
		&&(PL_THREAD_LABEL == par_pThr->dwLabel)
	  )
	{
		if(0 == pthread_join(par_pThr->handle, NULL))
		{
			nRc = plSUCCESS;
		}
	}
	return nRc;
}
//=========================================================//

//=========================================================//
PLs_THREAD* plcmThreadGetCurrent() 
{
  return (PLs_THREAD*)pthread_getspecific(CurrentThreadKey);
}
//=========================================================//

//=========================================================//
PLb_RETURN    plcmThreadKill  (PLs_THREAD*    par_pThr)
{
	PLb_RETURN nRc = plFAIL;

	if((NULL != par_pThr)&&(PL_THREAD_LABEL == par_pThr->dwLabel))
	{
		{
			if(0 == pthread_cancel(par_pThr->handle))
			{
				par_pThr->ThrState = THREAD_STATE_DEAD;
				nRc = plSUCCESS;
			}
		}
	}
	return nRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN	plcmThreadSuspend  (PLs_THREAD*    par_pThr)
{
	PLb_RETURN nRc = plFAIL;

	//TBD"Function 'plcmThreadSuspend' NOT implemented\n");

	return nRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN  plcmThreadResume (PLs_THREAD*    par_pThr)
{
	PLb_RETURN nRc = plFAIL;

	//TBD"Function Function 'plcmThreadResume' NOT implemented\n");

	return nRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN plcmThreadJoin(PLs_THREAD* par_pThr) 
{
	if((NULL != par_pThr)&&(PL_THREAD_LABEL == par_pThr->dwLabel))
	{
		if (plcmThreadGetCurrent() == par_pThr) 
			return plFAIL;
		if (par_pThr->ThrState != THREAD_STATE_UNBORN)
        {
			plcmSleepMs(3);
        }
		return plFAIL;
	}
	else
		return plFAIL;
}
//=========================================================//

//=========================================================//
const char* plcmThreadGetName(PLs_THREAD* par_pThr) 
{
	return (const char*)(par_pThr?  par_pThr->name : "User Thread");
}
//=========================================================//

//=========================================================//
PLb_RETURN	plcmThreadInit(vTHREAD_BODY			par_fpThreadBody
                          ,void			*       par_pThreadData
						  ,const char	*		par_pszName
						  ,PLs_THREAD	*		par_pThr)
{
	return   
		plcmThreadInitDetail(par_fpThreadBody, par_pThreadData, par_pszName,PL_PRIORITYVALUE_DEFAULT
						         ,PL_STACKSIZE_DEFAULT,par_pThr);
}
//=========================================================//

//=========================================================//
PLs_THREAD*	plcmThreadConstructDetail(vTHREAD_BODY  par_fpThreadBody
							         ,void*         par_pThreadData
							         ,const char*   par_pszName
							         ,int           par_nPriority
							         ,size_t        par_StackSize )
{
    PLs_THREAD  *    par_pThr = NULL;
    
    par_pThr = (PLs_THREAD*) malloc(sizeof(PLs_THREAD));

	if (NULL != par_pThr)
	{	
		if(plSUCCESS != plcmThreadInitDetail(par_fpThreadBody
										,par_pThreadData
										,par_pszName
										,par_nPriority
										,par_StackSize
										,par_pThr)                    
		  )
		{
			free(par_pThr);
			par_pThr = NULL;
		}
	}
	else
	{
		//TBD LOG "MALLOC is ERROR"
	}
	return par_pThr;
}
//=========================================================//

//=========================================================//
PLs_THREAD*	plcmThreadConstruct(vTHREAD_BODY  par_fpThreadBody
                               ,void*         par_pThreadData
							   ,const char*   par_pszName)
{
	return 	plcmThreadConstructDetail(par_fpThreadBody,par_pThreadData
	          ,par_pszName, PL_PRIORITYVALUE_DEFAULT ,PL_STACKSIZE_DEFAULT);
}
//=========================================================//

//=========================================================//
PLb_RETURN plcmThreadDestruct(PLs_THREAD* par_pThr) 
{
	PLb_RETURN	nRc = plFAIL;
	
	if(plSUCCESS == (nRc =plcmThreadDeinit(par_pThr)))
	{
		free(par_pThr);
		par_pThr = NULL;
	}
	return nRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN plcmIsTreadFinished(PLs_THREAD* par_pThr)
{
	PLb_RETURN	bRc = plSUCCESS;
	if(NULL != par_pThr)
	{
      bRc = par_pThr->bEndSignal;
	}
	return bRc;
}
//=========================================================//

//=========================================================//
PLb_RETURN plcmIsThreadActive(PLs_THREAD* par_pThr)
{
	PLb_RETURN  bRc = plFAIL;

	if(NULL != par_pThr)
	{
      bRc = (PLb_RETURN)(   (PL_THREAD_LABEL   == par_pThr->dwLabel)
                         && (THREAD_STATE_ALIVE == par_pThr->ThrState)
                         );
   }

	return   (bRc);
}
//=========================================================//

//=========================================================//
PLb_RETURN  plcmCreateAndStartThread( vTHREAD_BODY      par_fpThreadBody
                                     ,void	        *   par_pThreadData
                                     ,const char	*   par_pszName
                                     ,PLs_THREAD	*   par_pThr)
{
    PLb_RETURN  bRc = plFAIL;

    if(NULL != par_pThr)
    {
        if(plSUCCESS == (bRc = plcmThreadInit(par_fpThreadBody, par_pThreadData, par_pszName,par_pThr)))
        {
            bRc = plcmThreadStart(par_pThr);
        }
    }

    return   (bRc);
}
//=========================================================//

//=========================================================//
PLb_RETURN  plcmCreateAndStartThread_detach(vTHREAD_BODY     par_fpThreadBody
                                           ,void	     *   par_pThreadData
                                           ,const char   *   par_pszName
                                           ,PLs_THREAD   *   par_pThr)
{
    PLb_RETURN  bRc = plFAIL;

    if(NULL != par_pThr)
    {
        if(plSUCCESS == (bRc = plcmThreadInit(par_fpThreadBody, par_pThreadData, par_pszName,par_pThr)))
        {
            bRc = plcmThreadStart_detach(par_pThr);
        }
    }

    return   (bRc);
}
//=========================================================//

//=========================================================//
PLb_RETURN  plcmDeinitAndFreeThread (PLs_THREAD	*   par_pThr)
{
    PLb_RETURN bRc = plFAIL;

    if(plSUCCESS == plcmThreadWaitTillEnd(par_pThr))
    {
        bRc = plcmThreadDeinit(par_pThr);
    }

    return bRc;
}
//=========================================================//



//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------
//--------------------------------------------------------------------

	            //-------------------------------------------//
	            //-------  MUTEX  ---------------------------//
	            //-------------------------------------------//
//    plcmMutexInit ();
//==============================================================//
// Initiating statically allocated mutex.
//    If function fails it returns code "plFAIL", 
//    otherwise - "plSUCCESS"
//==============================================================//
PLb_RETURN  plcmMutexInit (PL_MUTEX* par_M )	
{
PLb_RETURN  nRc = plFAIL;

// TBD -  LET_LOG
	if (NULL != par_M)
	{
        
		//* par_M =  {;
		pthread_mutexattr_t		MutexAttr ;

        pthread_mutexattr_init  (&MutexAttr);
        pthread_mutexattr_settype(&MutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);
        
		PL_IF_MUTEX_OK(pthread_mutex_init(&(par_M->handle), &MutexAttr))
		{
			//pthread_mutex_init(&(par_M->lock), 0);
			nRc = plSUCCESS;
        }
		else
		{
			// TBD -  LET_LOG "Init mutex ERROR"
		}
	}
	return   (nRc);
}
//==============================================================//


//    plcmMutexDeinit ();
//==============================================================//
// De-Initiating statically allocated mutex 
// (recently initialized by "plcmMutexInit").
//    If the function fails it returns plFAIL
//==============================================================//
PLb_RETURN  plcmMutexDeinit (PL_MUTEX* par_pMutex) 
{
	PLb_RETURN	   nRc = plFAIL;

	if(NULL != par_pMutex)
	{
		PL_IF_MUTEX_OK(pthread_mutex_destroy(&(par_pMutex->handle)))
		{
			nRc = plSUCCESS;
		}
	}
	return   (nRc);
}
//==============================================================//

//    plcmMutexLock ();
//==============================================================//
// Take Mutex ownership unconditionally - with INDEFINITE waiting
//    Function returns one of the two ASe_MUT_WAIT values:
//       either GOT_OWNERSHIP, ERROR_PARAMETERS 
//==============================================================//
PLe_OBJECT_WAIT  plcmMutexLock (PL_MUTEX* par_pMutex)
{
PLe_OBJECT_WAIT   Rc  = PARAMETER_ERROR;

	if(NULL != par_pMutex)
	{

		PL_IF_MUTEX_OK(pthread_mutex_lock(&(par_pMutex->handle)))
		{
			Rc = GOT_OWNERSHIP;
		}
	}

	return   (Rc);
}
//==============================================================//


//    plcmMutexLockTry ();
//==============================================================//
// Conditional attempt to take Mutex ownership - approach to 
// nonblocking mode.
//==============================================================//
PLe_OBJECT_WAIT    plcmMutexLockTry (PL_MUTEX* par_pMutex)
{
PLe_OBJECT_WAIT Rc = PARAMETER_ERROR;

   if (NULL != par_pMutex)
   {
   int nRc;

      if (0 == (nRc = pthread_mutex_trylock(&(par_pMutex->handle)))
         )
      {
         Rc = GOT_OWNERSHIP;
      }
      else
      {
        PL_IF_MUTEX_FAIL(nRc)
	    {
            Rc = PARAMETER_ERROR;
	    }
	    else
	    {
            Rc = DIDNT_GOT_OWNERSHIP;
	    }
      }
   }

   return   (Rc);
}
//==============================================================//

//    plcmMutexUnLock ();
//==============================================================//
// Release Mutex ownership.
//==============================================================//
PLb_RETURN       plcmMutexUnLock    (PL_MUTEX* par_pMut)
{
PLb_RETURN	nRc = plFAIL;

   if (NULL != par_pMut)
   {
        if(0 == pthread_mutex_unlock(&(par_pMut->handle)) )
	    {
		    nRc = plSUCCESS;
	    }
   }
	return nRc;
}
//======================================================//

//======================================================//
PLe_OBJECT_WAIT  plcmMutexLockTimed (PL_MUTEX* par_pMutex, int par_MilliSec)
{
    PLe_OBJECT_WAIT   Rc  = PARAMETER_ERROR;
    struct timespec	  sWaitTimeout;

    sWaitTimeout.tv_sec  = par_MilliSec / 1000;
    sWaitTimeout.tv_nsec = (par_MilliSec % 1000) * (1000 * 1000);

    clock_gettime(CLOCK_REALTIME, &sWaitTimeout);

    sWaitTimeout.tv_sec += 10;


    if(NULL != par_pMutex)
    {
        int  nRcW = 0;
 
        nRcW = pthread_mutex_timedlock(&(par_pMutex->handle), &sWaitTimeout);

        switch(nRcW)
        {
        default         :{Rc = INTERRUPTED_SYSSTEM_CALL;   } break;

        case EINVAL     :{Rc = PARAMETER_ERROR;            } break;
        case ETIMEDOUT  :{Rc = DIDNT_GOT_TIMEOUTED;        } break;
        case EAGAIN     :{Rc = COUNTER_OVERFLOW;           } break;

        case EDEADLK    :{Rc = GOT_OWNERSHIP;               } break; 
        case 0          :{Rc = GOT_OWNERSHIP;               } break; 

        }
    }
    return   (Rc);
}
//==============================================================//

//======================================================//
//======================================================//
//======================================================//
//======================================================//

//======================================================//
PLb_RETURN  PL_MUTEXInit (PLc_MUTEX* par_pMutexSt)
{
	if (plcmMutexInit(&par_pMutexSt->_Mutex) != plSUCCESS)
		return plFAIL;
	else
		return plSUCCESS;
}
//======================================================//
//======================================================//
PLb_RETURN        PL_MUTEXDeInit (PLc_MUTEX* par_pMutexSt)
{
    PLb_RETURN  bRc = plFAIL;
	if(NULL != par_pMutexSt)
	{
		if (plcmMutexDeinit(&par_pMutexSt->_Mutex) != plSUCCESS)
			bRc = plFAIL;
		else
			bRc = plSUCCESS;
	}

	return bRc;
}
//======================================================//
//======================================================//
PLe_OBJECT_WAIT PL_MUTEXLock(PL_MUTEX* par_pMutex)
{
	return plcmMutexLock(par_pMutex);
}
//======================================================//
//======================================================//
PLe_OBJECT_WAIT PL_MUTEXLockTry(PL_MUTEX* par_pMutex)
{
	return plcmMutexLockTry(par_pMutex);
}
//======================================================//
//======================================================//
PLb_RETURN        PL_MUTEXUnLock (PL_MUTEX* par_pMutex)
{
	if (plcmMutexUnLock(par_pMutex) != plSUCCESS)
		return plFAIL;
	else
		return plSUCCESS;
}
//======================================================//

PLe_OBJECT_WAIT PL_MUTEXLockTimed(PL_MUTEX* par_pMutex, int par_MilliSec)
{
    return plcmMutexLockTimed(par_pMutex, par_MilliSec);
}
//======================================================//

//======================================================//
void  MutexObjInit(PLc_MUTEX * par_pMutex)
{
	if(NULL != par_pMutex)
	{
		par_pMutex->fInit      = (F_MUTEX_INIT)       PL_MUTEXInit     ;
		par_pMutex->fDeInit    = (F_MUTEX_DEINT)      PL_MUTEXDeInit   ;
        par_pMutex->fLock      = (F_MUTEX_LOCK)       PL_MUTEXLock     ;
		par_pMutex->fUnLock    = (F_MUTEX_UNLOCK)     PL_MUTEXUnLock   ;
        par_pMutex->fTryLock   = (F_MUTEX_TRY_LOCK)   PL_MUTEXLockTry  ;
        par_pMutex->fLockTimed = (F_MUTEX_LOCK_TIMED) PL_MUTEXLockTimed;
        
		par_pMutex->fInit(&par_pMutex->_Mutex);
	}
}
//======================================================//

//======================================================//
//======================================================//
//======================================================//
//======================================================//

//======================================================//
PLb_RETURN    bQueueInit(PL_QUEUE * par_pQueue)
{
     PLb_RETURN  bRc = plFAIL;
     if(NULL != par_pQueue)
     {
         memset(par_pQueue, 0, sizeof(PL_QUEUE));
         MutexObjInit(&par_pQueue->mq_Mutex);
         par_pQueue->mq_QuAvalible = 1;
         bRc = plSUCCESS; 
     }
     return bRc;
}
//======================================================//
//======================================================//
PLb_RETURN    bQueueClear      (PL_QUEUE * par_pQueue)
{
    PLb_RETURN  bRc = plFAIL;
    if(NULL != par_pQueue)
    {
//+++++
        par_pQueue->mq_Mutex.fLock(&par_pQueue->mq_Mutex._Mutex);
        while(0 <  nQueueGetSize(par_pQueue))
        {
            Q_Node * p1Node = par_pQueue->mq_pHead; 
            Q_Node * p2Node = par_pQueue->mq_pHead->pNext;

           p2Node->pPrev = NULL;
           par_pQueue->mq_pHead = p2Node;
           free(p1Node->sData.d_pData);  
           free(p1Node);
           par_pQueue->mq_dwNumberOfNodes --;
        }
        bRc = plSUCCESS;
        par_pQueue->mq_Mutex.fUnLock(&par_pQueue->mq_Mutex._Mutex);
//-----
    }
    return bRc;
}
//======================================================//
//======================================================//
PLb_RETURN    bQueueDeinit     (PL_QUEUE * par_pQueue)
{
    PLb_RETURN  bRc = plFAIL;
    if(NULL != par_pQueue)
    {
        par_pQueue->mq_QuAvalible = 0;
 
        bQueueClear(par_pQueue); 

        par_pQueue->mq_Mutex.fUnLock(&par_pQueue->mq_Mutex._Mutex);

        if(plSUCCESS == par_pQueue->mq_Mutex.fDeInit(&par_pQueue->mq_Mutex._Mutex))
        {
            memset(par_pQueue, 0, sizeof(PL_QUEUE));
            bRc = plSUCCESS; 
        }
    }
    return bRc;
}
//======================================================//
//======================================================//
unsigned int  nQueueGetSize    (PL_QUEUE * par_pQueue)
{
    unsigned int nRc = 0;
    if((NULL != par_pQueue)&&(1 == par_pQueue->mq_QuAvalible))
    {
        //+++++
        par_pQueue->mq_Mutex.fLock(&par_pQueue->mq_Mutex._Mutex);

        nRc = par_pQueue->mq_dwNumberOfNodes;

        par_pQueue->mq_Mutex.fUnLock(&par_pQueue->mq_Mutex._Mutex);
        //-----
    }
    return nRc; 
}
//======================================================//
//======================================================//
PLb_RETURN    bQueuePut (PL_QUEUE * par_pQueue, unsigned int par_DataId, void * par_pData, unsigned int par_DataSize)
{
    PLb_RETURN  bRc = plFAIL;

    if(  (NULL != par_pQueue)&&(NULL != par_pData)&&(par_DataSize > 0)
       &&(1 == par_pQueue->mq_QuAvalible)
      )
    {
    //+++++
    par_pQueue->mq_Mutex.fLock(&par_pQueue->mq_Mutex._Mutex);
    {
        Q_Node * pNode = (Q_Node *) malloc(sizeof(Q_Node));
        
        if(NULL != pNode)//klocwork #2008
        {
            memset(pNode, 0, sizeof(Q_Node));
            pNode->sData.d_pData = malloc(par_DataSize);
            if(NULL != pNode->sData.d_pData)//klocwork #2010
            {
                memcpy(pNode->sData.d_pData, par_pData, par_DataSize);
                pNode->sData.d_dwDataSize = par_DataSize;
                pNode->sData.d_dwDataId   = par_DataId  ;
            
                if(0 == par_pQueue->mq_dwNumberOfNodes)
                {
                    pNode->pNext = NULL;
                    pNode->pPrev = NULL;
                    par_pQueue->mq_pHead = pNode;
                    par_pQueue->mq_pTail = pNode;
                }
                else
                {
                    Q_Node * pNodeTail = par_pQueue->mq_pTail;

                    pNodeTail->pNext = pNode;

                    pNode->pPrev = pNodeTail;
                    pNode->pNext = NULL;

                    par_pQueue->mq_pTail = pNode;
                }
                par_pQueue->mq_dwNumberOfNodes++;

                bRc = plSUCCESS;
            }
            else
            {
                free(pNode);
                pNode = NULL;
            }
        }
    }
    par_pQueue->mq_Mutex.fUnLock(&par_pQueue->mq_Mutex._Mutex);
    //-----
    }
    return bRc;
}
//======================================================//
//======================================================//
PLb_RETURN    bQueueGet(PL_QUEUE * par_pQueue, unsigned int * par_pDataId, void * par_pData, unsigned int * par_pDataSize)
{
    //NOTE  par_pData  MUST FRRE in USER's code
    PLb_RETURN  bRc = plFAIL;
    if(  (NULL != par_pQueue)&&(NULL != par_pData)
       &&((NULL != par_pDataSize)&&(*par_pDataSize >0))
       &&(NULL != par_pQueue->mq_pHead)
       &&(1 == par_pQueue->mq_QuAvalible)
      )
    {
        //+++++
        par_pQueue->mq_Mutex.fLock(&par_pQueue->mq_Mutex._Mutex);
        {
            Q_Node * p1Node = par_pQueue->mq_pHead; 
            Q_Node * p2Node = par_pQueue->mq_pHead->pNext;

            if((*par_pDataSize) >= p1Node->sData.d_dwDataSize)
            {
                if(NULL != p2Node)
                    p2Node->pPrev = NULL;

                if(NULL != par_pDataId)
                    *par_pDataId = p1Node->sData.d_dwDataId;

                memcpy(par_pData, p1Node->sData.d_pData, p1Node->sData.d_dwDataSize);
                
                *par_pDataSize = p1Node->sData.d_dwDataSize;
                
                par_pQueue->mq_pHead = p2Node;

                free(p1Node->sData.d_pData);  
                free(p1Node);

                par_pQueue->mq_dwNumberOfNodes --;
                bRc = plSUCCESS;
            }
        }
        //-----
        par_pQueue->mq_Mutex.fUnLock(&par_pQueue->mq_Mutex._Mutex);
    }

    return bRc;
}
//======================================================//
//======================================================//
PLb_RETURN    bQueueFirstNodeInfo(PL_QUEUE * par_pQueue, unsigned int * par_pDataId, void * par_pData, unsigned int * par_pDataSize)
{
    PLb_RETURN  bRc = plFAIL;
    if(  (NULL != par_pQueue)&&(NULL != par_pQueue->mq_pHead)
       &&(1 == par_pQueue->mq_QuAvalible)      
      )
    {
    //+++++
    par_pQueue->mq_Mutex.fLock(&par_pQueue->mq_Mutex._Mutex);
    {
        par_pQueue->mq_pHead->pPrev = NULL;
        if(NULL != par_pDataId)
            *par_pDataId = par_pQueue->mq_pHead->sData.d_dwDataId;
        if(  (NULL != par_pData)
           &&((NULL != par_pDataSize)&&(*par_pDataSize >= par_pQueue->mq_pHead->sData.d_dwDataSize))
          )
        {
            memcpy(par_pData, &par_pQueue->mq_pHead->sData.d_pData, par_pQueue->mq_pHead->sData.d_dwDataSize);
            *par_pDataSize = par_pQueue->mq_pHead->sData.d_dwDataSize;

            bRc = plSUCCESS;
        }
    }
    //-----
    par_pQueue->mq_Mutex.fUnLock(&par_pQueue->mq_Mutex._Mutex);

    }
    return bRc;
}
//======================================================//
//======================================================//
unsigned int  nQueueFirstDataSize(PL_QUEUE * par_pQueue)
{
    unsigned int  dwRc = 0xFFFFFFFF;
    if(  (NULL != par_pQueue)&&(NULL != par_pQueue->mq_pHead)
       &&(1 == par_pQueue->mq_QuAvalible)
      )
    {
        //+++++
        par_pQueue->mq_Mutex.fLock(&par_pQueue->mq_Mutex._Mutex);
        {
            dwRc = par_pQueue->mq_pHead->sData.d_dwDataSize;
        }
        //-----
        par_pQueue->mq_Mutex.fUnLock(&par_pQueue->mq_Mutex._Mutex);
    }

    return dwRc;
}
//======================================================//




	      //-------------------------------------------//
	      //-------------------------------------------//
	      //-------------------------------------------//
      	//----  wrapper class for threads  ----------//
	      //-------------------------------------------//
	      //-------------------------------------------//
	      //-------------------------------------------//
//======================================================//
PLc_THREAD::PLc_THREAD(char * szThrName)
{
	this->bThreadInitResult = plcmThreadInit( PLc_THREAD::ThreadFuncWrap
									  , this
									  , szThrName
									  , &this->_Thread
	                                  
									  );
	if(plSUCCESS != this->bThreadInitResult)
	{
	//PRINT(THREAD [%s] - did not INIT", szThrName)	
	}
}
//======================================================//

//======================================================//
PLc_THREAD::~PLc_THREAD(void)
{
	this->Kill();
	plcmThreadDeinit(&(this->_Thread) );
}
//======================================================//

//======================================================//
bool PLc_THREAD::ActivateTach  ()
{
	bool  bAnswer = this->bThreadOk();

	if (false == bAnswer)
	{
		if (plcmThreadStart(&this->_Thread) == plSUCCESS)
		{
			bAnswer = true;
		}
	}
	return   (bAnswer);
}
//======================================================//

//======================================================//
bool PLc_THREAD::ActivateDeTach  ()
{
	bool  bAnswer = this->bThreadOk();
	if (false == bAnswer)
	{
		if (plcmThreadStart_detach(&this->_Thread) == plSUCCESS)
		{
			bAnswer = true;
		}
	}
	return   (bAnswer);
}
//======================================================//

//======================================================//
void   PLc_THREAD::SetName(char * par_szThreadName)
{
	if (NULL != par_szThreadName)
	{
		memset ((this->_Thread).name, 0
				,sizeof((this->_Thread).name)
			 );
		strncpy((this->_Thread).name
			 ,par_szThreadName
				,(sizeof((this->_Thread).name) - 1)
			 );
	}
}
//======================================================//

//======================================================//
char * PLc_THREAD::GetName(void)
{
	return   (this->_Thread).name;
}
//======================================================//

//======================================================//
// Kill the thread.
bool PLc_THREAD::Kill(void)
{
	bool  bAnswer = !(this->bThreadOk() );

	if (false == bAnswer)
	{
		bAnswer = (plSUCCESS == plcmThreadKill(&this->_Thread) );
	}

	return   (bAnswer);
}
//======================================================//

//======================================================//
// Block the caller until the thread exists
bool PLc_THREAD::Wait(void)
{
	bool  bAnswer = false;

	if (plcmThreadWaitTillEnd(&this->_Thread) == plSUCCESS)
	{
		bAnswer = true;
	}

	return   (bAnswer);
}
//======================================================//

//======================================================//
// Block the caller for par_Ms milliseconds
void PLc_THREAD::Sleep(PL_INTERVAL_ms par_Ms)
{
	plcmSleepMs(par_Ms);
}
//======================================================//

//======================================================//
// Helper function used to provide mechanism for invoking the virtual
// Svc() method as static method.
void PLc_THREAD::ThreadFuncWrap(_PLs_THREAD*  par_ThreadPtr)
{
	((PLc_THREAD *)par_ThreadPtr->data)->Svc();//par_ThreadPtr->data - containes 'this' pointer
}
//======================================================//

//======================================================//
// Sending of the finish signal for legal close  of this thread.
bool PLc_THREAD::SendFinish(void)
{
	bool  bAnswer = false;

	if (plSUCCESS == plcmTreadSendFinishSignal(&this->_Thread) )
	{
		bAnswer = true;
	}

	return   (bAnswer);
}
//======================================================//

//======================================================//
bool PLc_THREAD::IsFinished()
{
	bool				   bRc = true;

	bRc = (this->_Thread).bEndSignal;

	return   (bRc);
}
//======================================================//

//======================================================//
PLe_OBJECT_WAIT PLc_THREAD::EndSignalClose()
{
	PLe_OBJECT_WAIT   eWaitCode = GOT_OWNERSHIP;
	(this->_Thread).bEndSignal = plSUCCESS;

	return eWaitCode;
}
//======================================================//

//======================================================//
bool PLc_THREAD::IsActive  ()
{
	return plcmIsThreadActive(&this->_Thread);
}
//======================================================//

//------------------------------------------------------------------------------//
//--------------cDnsReqList ----------------------------------------------------//
//------------------------------------------------------------------------------//
//==============================================================================//
cDnsReqList::cDnsReqList()
{
	MutexObjInit(&this->m_Mutex);
}
//==============================================================================//
//==============================================================================//
cDnsReqList::~cDnsReqList()
{
	this->bClean();
	M_DEINIT(this->m_Mutex);
}
//==============================================================================//
//==============================================================================//
BOOL cDnsReqList::bInsert(cDNS_PLCM_REQ_NODE * par_pNode)
{
	BOOL  bRc = FALSE;

	if(NULL != par_pNode)
	{
		M_LOCK(this->m_Mutex);
			this->m_List.push_back(par_pNode);
        M_UnLOCK(this->m_Mutex);  
		bRc = TRUE;
	}
	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cDnsReqList::bDelete(cDNS_PLCM_REQ_NODE * par_pNode)
{
	BOOL  bRc = FALSE;
	if(NULL != par_pNode)
	{
		unsigned int	nInd		= 0;

	M_LOCK(this->m_Mutex);
		for (nInd = 0; nInd <  m_List.size(); nInd++) 
		{
			cDNS_PLCM_REQ_NODE * pNode = m_List[nInd];
			if(  (pNode->m_sReqNode.wDnsReqID == par_pNode->m_sReqNode.wDnsReqID)
				&&(0 == strncmp(pNode->m_sReqNode.szHostName, par_pNode->m_sReqNode.szHostName, sizeof(pNode->m_sReqNode.szHostName)))
				)
			{
				m_List.erase(m_List.begin() + nInd);
				delete(pNode);
				break;
			}
		}
	M_UnLOCK(this->m_Mutex);
	}
	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cDnsReqList::bDelete(unsigned int par_wDnsReqID, char * par_szLog)
{
	BOOL  bRc = FALSE;
	unsigned int	nInd		= 0;

	M_LOCK(this->m_Mutex);
	for (nInd = 0; nInd < m_List.size(); nInd++) 
	{
		cDNS_PLCM_REQ_NODE * pNode = m_List[nInd];
		if(pNode->m_sReqNode.wDnsReqID == par_wDnsReqID)
		{
			m_List.erase(m_List.begin() + nInd);
          //  TRACEINTO << "PLCM_DNS. DELETED. ReqID:[" << pNode->m_sReqNode.wDnsReqID <<"]  = ["<<  par_wDnsReqID << "]"
		  //            << " |  "<< par_szLog;  
			delete(pNode);
			break;
		}
	}
     M_UnLOCK(this->m_Mutex);  
	return bRc;
}
//==============================================================================//
//==============================================================================//
void  cDnsReqList::vUpdatePartnerId(unsigned short par_wDnsReqID, unsigned short par_wPartnerDnsReqID)
{
	unsigned int		nInd = 0;

	M_LOCK(this->m_Mutex);
	for (nInd = 0; nInd < m_List.size(); nInd++) 
	{
		cDNS_PLCM_REQ_NODE * pNode = m_List[nInd];
		if(pNode->m_sReqNode.wDnsReqID == par_wDnsReqID)
		{
			pNode->m_sReqNode.wPartnerDnsReqID = par_wPartnerDnsReqID;

			//TRACEINTO << "PLCM_DNS. vUpdatePartnerId. My ReqID:[" << pNode->wDnsReqID <<"]  | Partner:["<<  pNode->wPartnerDnsReqID << "]"!; 
			break;
		}
	}
	M_UnLOCK(this->m_Mutex);
}
//==============================================================================//
//==============================================================================//
cDNS_PLCM_REQ_NODE * cDnsReqList::Find(unsigned short par_wDnsReqID)
{
	cDNS_PLCM_REQ_NODE  * pRc = NULL;
	unsigned int		nInd = 0;

	M_LOCK(this->m_Mutex);
	for (nInd = 0; nInd < m_List.size(); nInd++) 
	{
		cDNS_PLCM_REQ_NODE * pNode = m_List[nInd];
		if(pNode->m_sReqNode.wDnsReqID == par_wDnsReqID)
		{
			pRc = pNode;
			break;
		}
	}
    M_UnLOCK(this->m_Mutex);
	return pRc;
}
//==============================================================================//
//==============================================================================//
BOOL  cDnsReqList::bClean()
{
	BOOL			bRc = FALSE;
	unsigned int	nInd		= 0;
	M_LOCK(this->m_Mutex);
		while(0 < m_List.size())
		{
			cDNS_PLCM_REQ_NODE * pNode = m_List[0];
			unsigned int dwCurrSize = m_List.size();
			m_List.erase(m_List.begin() + 0);
			delete(pNode);
		}
	M_UnLOCK(this->m_Mutex);
	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cDnsReqList::DnsReqListAudit(DnsReqListAuditFun par_pAuditFunction, int par_nDataType, void * par_pData)
{
	BOOL			bRc = FALSE;
	unsigned int	nInd		= 0;
	unsigned int	nListSize	= m_List.size();

	if(NULL != par_pAuditFunction)
	{
	M_LOCK(this->m_Mutex);
		for (nInd = 0; nInd < nListSize; nInd++) 
		{
			cDNS_PLCM_REQ_NODE * pNode = m_List[nInd];
			par_pAuditFunction(pNode, par_nDataType, par_pData);
		}
	M_UnLOCK(this->m_Mutex);
	}
	
	return bRc;
}
//==============================================================================//
//==============================================================================//
BOOL cDnsReqList::DnsReqListAuditBreak(DnsReqListAuditFun par_pAuditFunction, int par_nDataType, void * par_pData)// First return from "par_pAuditFunction" - is break from loop
{
	BOOL			bRc = FALSE;
	unsigned int	nInd		= 0;
	unsigned int	nListSize	= m_List.size();

	if(NULL != par_pAuditFunction)
	{
	M_LOCK(this->m_Mutex);
		for (nInd = 0; nInd < nListSize; nInd++) 
		{
			cDNS_PLCM_REQ_NODE * pNode = m_List[nInd];
			if(TRUE == par_pAuditFunction(pNode, par_nDataType, par_pData))
			{
				break;
				bRc = TRUE;
			}
		}
    M_UnLOCK(this->m_Mutex);
	}

	return bRc;
}
//==============================================================================//
//==============================================================================//
unsigned int cDnsReqList::dwGetSize()
{
	M_LOCK(this->m_Mutex);
		return m_List.size();
	M_UnLOCK(this->m_Mutex);
}
//==============================================================================//






//====================================================================//
char * GetNullChar(char * szBuff_S,  int nBytes, int * pIndep)
{
	char * pRc = NULL;
	int nI = 0;

	while(nI < nBytes)
	{
		if('\0' == szBuff_S[nI])
		{
			pRc = &(szBuff_S[nI]);
			if(NULL != pIndep)
				*pIndep = nI;
			break;
		}
		nI++;
	}

	return pRc;
}
//====================================================================//
//====================================================================//
bool bGetAdvanced(char * szBuff_S,  int nBytesRcvd, unsigned int * pIpArry,
				  size_t MaxLen, size_t * pRcSize)
{
	bool   bRc = false;
	char * szBlock = NULL;
	int    nBlockStrart = 0;
	short  nAnswAmount  = 0;
	DNS_PACKET_HEADER * pHeader = (DNS_PACKET_HEADER*)szBuff_S; 


	//1 miss Header 

	szBlock = szBuff_S + sizeof(DNS_PACKET_HEADER);
	nBlockStrart += sizeof(DNS_PACKET_HEADER);

	//2 miss Query
	int nInd = 0;
	szBlock =  GetNullChar(szBlock, nBytesRcvd - nBlockStrart, &nInd);
	nBlockStrart += nInd+1 + 4;

	//3 miss Answer
	while(nAnswAmount < pHeader->wAnCount)
	{
		nBlockStrart += 2;
		szBlock = &szBuff_S[nBlockStrart];

		DNS_REPLY_DATA * pData = (DNS_REPLY_DATA*) szBlock;

		//short Datalen = htons(pData->wDataLen);
		nBlockStrart += htons(pData->wDataLen) + sizeof(DNS_REPLY_DATA);

		nAnswAmount++;
	}

	// 4 miss Authoritative 
	short wAuthoritative = 0;
	while(wAuthoritative < pHeader->wNsCount)
	{
		nBlockStrart += 2;
		szBlock = &szBuff_S[nBlockStrart];

		DNS_REPLY_DATA * pData = (DNS_REPLY_DATA*) szBlock;

		nBlockStrart += sizeof(DNS_REPLY_DATA);

		nBlockStrart += htons(pData->wDataLen);

		wAuthoritative++;
	}

	// 5 Additional 
	short wAdditionalAmount = 0;
	if(pHeader->wArCount > 0)
	{
		if(NULL != pIpArry)
		{
			*pRcSize = pHeader->wArCount;

			while(wAdditionalAmount < pHeader->wArCount)
			{
				nBlockStrart += 2;
				szBlock = &szBuff_S[nBlockStrart];

				DNS_REPLY_DATA * pData = (DNS_REPLY_DATA*) szBlock;

				if(4 == htons(pData->wDataLen))
				{
					nBlockStrart += sizeof(DNS_REPLY_DATA);

					unsigned int * pIp = (unsigned int *) &szBuff_S[nBlockStrart];

					unsigned int dwIp = *pIp;

					struct in_addr    sIPaddr;
					char szIp[128]="";
					sIPaddr.s_addr = *((unsigned int *)&dwIp);
					strncpy(szIp, inet_ntoa(sIPaddr), sizeof(szIp)-1);

					//printf(" --- Add IP: %s (0x%X)\n",szIp, dwIp);
					if(wAdditionalAmount < (short) MaxLen)
						pIpArry[wAdditionalAmount] = dwIp;

				}

				nBlockStrart += htons(pData->wDataLen);
				wAdditionalAmount++;
			}
		}
	}

	return bRc;
}
//====================================================================//

//====================================================================//
void	vUnParsNameData(char * szRequestData, int par_nReqDataLen)
{
	if(NULL != szRequestData)
	{
		size_t  dwStrLen = strlen(&szRequestData[1]);
		char  * szStr = &szRequestData[1];
		for(size_t nW = 0; nW <= dwStrLen; nW++)
		{
			if((szStr[nW]<' ')&&(szStr[nW] != '\n')&&(szStr[nW] != '\0'))
				szStr[nW] = '.';
			if((0x0a == szStr[nW])&&(nW+1 < dwStrLen))
				szStr[nW] = '.';
		}
		strncpy(&szRequestData[0], szStr, par_nReqDataLen);
	}
}
//====================================================================//

//====================================================================//
//Return:
//  1(0x01) = A		=> eDNS_TYPE_A_IPv4
// 28(0x1c) = AAAA	=> eDNS_TYPE_AAAA_IPv6
// 33(0x21) = SRV	=> eDNS_TYPE_SRV
unsigned short plcmDNS_GetQueryType(char			*	szBuff_S
								  , int					nBytesRcvd )
{
	unsigned short			wRc = 0;
	char  * szBuff 			= (char*) szBuff_S;
	int		nResolvedInd	= 0;

	DNS_PACKET_HEADER * pDnsHeader = (DNS_PACKET_HEADER*) szBuff;

	pDnsHeader->wId       = ntohs(pDnsHeader->wId      );
	pDnsHeader->wFlags    = ntohs(pDnsHeader->wFlags   );
	pDnsHeader->wQdCount  = ntohs(pDnsHeader->wQdCount );
	pDnsHeader->wAnCount  = ntohs(pDnsHeader->wAnCount );
	pDnsHeader->wNsCount  = ntohs(pDnsHeader->wNsCount );
	pDnsHeader->wArCount  = ntohs(pDnsHeader->wArCount );

	//if(pDnsHeader->wId & 0xFF)
	{
		size_t		dwHeaderLen			= sizeof(DNS_PACKET_HEADER);
		char	*	szHostName			= (char*) szBuff + dwHeaderLen;
		size_t		dwHostNameLen		= strlen(szHostName);
		char		szHosNamePrint[PLCM_DNS_HOST_NAME_SIZE]	="";
		int         nRelevantAnswerCounter = 0;        

		dwHeaderLen += dwHostNameLen +1;
		
		unsigned short  * pwDnsQueryType = (unsigned short *)&szBuff[dwHeaderLen];
        wRc = ntohs(*pwDnsQueryType);
	}

	return wRc;
}
//====================================================================//

//====================================================================//
void plcmDNS_DnsAnswerParsing(  char			*	szBuff_S
					          , int					nBytesRcvd
							  , DNS_IPADDR_TTL	*	par_aResolveAddrAndTTL
					          , int				*	par_pLen
							  , char            *   par_pOutHostName
							  , int                 par_nOutHostNameLen
							  , unsigned short  *   par_pOutReqId 
							  , unsigned short  *   par_pOutType
					         ) 
{
	char  * szBuff 			= (char*) szBuff_S;
	int		nResolvedInd	= 0;

	DNS_PACKET_HEADER * pDnsHeader = (DNS_PACKET_HEADER*) szBuff;

	pDnsHeader->wId       = ntohs(pDnsHeader->wId      );
	pDnsHeader->wFlags    = ntohs(pDnsHeader->wFlags   );
	pDnsHeader->wQdCount  = ntohs(pDnsHeader->wQdCount );
	pDnsHeader->wAnCount  = ntohs(pDnsHeader->wAnCount );
	pDnsHeader->wNsCount  = ntohs(pDnsHeader->wNsCount );
	pDnsHeader->wArCount  = ntohs(pDnsHeader->wArCount );


	unsigned short wAnswer = pDnsHeader->wAnCount + pDnsHeader->wArCount;
	unsigned short wImportanceIp = 0;

	{
		char szLog[2048]="";

		snprintf(szLog, sizeof(szLog)-1,"PLCM_DNS.[plcmDNS_DnsAnswerParsing 1]. #[%d] | wAnCount:[%d] | wArCount:[%d] | wAnswer:[%d] | nBytesRcvd:[%d]"
			, pDnsHeader->wId, pDnsHeader->wAnCount, pDnsHeader->wArCount, wAnswer, nBytesRcvd);

		PTRACE1(eLevelInfoNormal, szLog);
	}


	//if(pDnsHeader->wId & 0xFF)
	{
		size_t		dwHeaderLen			= sizeof(DNS_PACKET_HEADER);
		char	*	szHostName			= (char*) szBuff + dwHeaderLen;
		size_t		dwHostNameLen		= strlen(szHostName);
		char		szHosNamePrint[PLCM_DNS_HOST_NAME_SIZE]	="";
		int         nRelevantAnswerCounter = 0;     

		dwHeaderLen += dwHostNameLen +1;
		strncpy(par_pOutHostName, szHostName, par_nOutHostNameLen);
		vUnParsNameData(par_pOutHostName, par_nOutHostNameLen);

		if(NULL != par_pOutReqId)
			*par_pOutReqId = pDnsHeader->wId;

		if(NULL != par_pOutType)
		{
			unsigned short * pOutType = (unsigned short*)(&szBuff[dwHeaderLen]);
			*par_pOutType = ntohs(*pOutType);
		}

		dwHeaderLen += sizeof(unsigned short);//FOR Query.Type
		dwHeaderLen += sizeof(unsigned short);//FOR Query.Class


		unsigned short wAnswer = pDnsHeader->wAnCount + pDnsHeader->wArCount;
		unsigned short wImportanceIp = 0;

		
		if(wAnswer != 0)
		{
			while((wAnswer > 0)&&((int)dwHeaderLen < nBytesRcvd))
			{

				if(szBuff[dwHeaderLen] & 0xC0)
				{
					dwHeaderLen = dwHeaderLen + sizeof(unsigned short);
				}
				else
				{
					dwHeaderLen += strlen(&(szBuff[dwHeaderLen]));
					dwHeaderLen++;
				}

				DNS_REPLY_DATA * pReplayData = (DNS_REPLY_DATA*) (szBuff + dwHeaderLen);

				dwHeaderLen += sizeof(DNS_REPLY_DATA);

				unsigned short D_wType    = ntohs(pReplayData->wType);
				unsigned int   D_dwTTL    = ntohl(pReplayData->dwTTL);
				unsigned short D_wDataLen = ntohs(pReplayData->wDataLen);

				if((eDNS_TYPE_A_IPv4 == D_wType)&&(4 == D_wDataLen))
				{
					unsigned int D_dwIp = *((unsigned int*)(szBuff + dwHeaderLen));

					if(nResolvedInd < (*par_pLen))
					{
						par_aResolveAddrAndTTL[nResolvedInd].sIpAddr.ipVersion  = eIpVersion4;
						par_aResolveAddrAndTTL[nResolvedInd].sIpAddr.addr.v4.ip = D_dwIp;
						par_aResolveAddrAndTTL[nResolvedInd].dwTTL              = D_dwTTL;
						nResolvedInd++;
					}
					nRelevantAnswerCounter++;
				}
				else
				if((eDNS_TYPE_AAAA_IPv6 == D_wType)&&(16 == D_wDataLen))
				{
					unsigned char * D_arIp = ((unsigned char *)(szBuff + dwHeaderLen));
					if(nResolvedInd < (*par_pLen))
					{
						par_aResolveAddrAndTTL[nResolvedInd].sIpAddr.ipVersion  = eIpVersion6;
						memcpy(par_aResolveAddrAndTTL[nResolvedInd].sIpAddr.addr.v6.ip, D_arIp, IPV6_ADDRESS_BYTES_LEN);
						par_aResolveAddrAndTTL[nResolvedInd].dwTTL              = D_dwTTL;
						nResolvedInd++;
					}
					nRelevantAnswerCounter++;

					//char szTmp[64]="";
					//plcmIpToStringV6(D_arIp, szTmp, sizeof(szTmp)-1,FALSE);
					//printf("\n-------------------\nPLCM_DNS. [RECV] | Rq.#[%5u].%2d/%d | AAAA(IPv6) %s | %8d in sec.\n-------------------\n", pDnsHeader->wId
					//	                                          , (pDnsHeader->wAnCount- wAnswer)+1, pDnsHeader->wAnCount, szTmp, D_dwTTL);
				}
				else
                if(eDNS_TYPE_SRV == D_wType) 
				{
					//wAnswer = pDnsHeader->wArCount;

					{
						char szLog[2048]="";

						snprintf(szLog,sizeof(szLog)-1, "[PLCM_DNS.plcmDNS_DnsAnswerParsing 4]. eDNS_TYPE_SRV ");

						PTRACE1(eLevelInfoNormal, szLog);
					}
					//D_wDataLen += 	
					//printf("\n-------------------\nPLCM_DNS. [RECV!!!] | Rq.#[%5u]. D_wType:[%d] | D_wDataLen:[%d]\n-------------------\n", pDnsHeader->wId,  D_wType, D_wDataLen);
				}

				dwHeaderLen += D_wDataLen;

				wAnswer --;
			}//WHILE of parsing

			//if(0 == nRelevantAnswerCounter)
			//	printf("PLCM_DNS. [RECV_1] | Rq.#[%5u]. NOT FOUND\n", pDnsHeader->wId);

		}//If answer is presented.
		else
		{//
			//printf("PLCM_DNS. [RECV_2] | Rq.#[%5u]. NOT FOUND\n", pDnsHeader->wId);
		}//answer NOT presented

	if(NULL != par_pLen)
		*par_pLen = nRelevantAnswerCounter;

	}//IF this is our packet !!!
}//void DnsAnswerParsing(char * szBuff_S, int nBytesRcvd) 
//====================================================================//


char G_plcmDnsSendResultPrint[10][256]=
{
	 {"plcmDNS_SendResolveResult. Sending resolve FAILED indication to: [party]."     }//0
	,{"plcmDNS_SendResolveResult. Sending resolve FAILED indication to: [SipProxy]."  }//1
	,{"plcmDNS_SendResolveResult. Sending resolve FAILED indication to: [Gatekeeper]."}//2
	,{"plcmDNS_SendResolveResult. Sending resolve FAILED indication to: [- - - -]."   }//3

	,{"plcmDNS_SendResolveResult. Sending resolve indication to: [party]."			  }//4
	,{"plcmDNS_SendResolveResult. Sending resolve indication to: [SipProxy]."         }//5
	,{"plcmDNS_SendResolveResult. Sending resolve indication to: [Gatekeeper]."       }//6
	,{"plcmDNS_SendResolveResult. Sending resolve indication to: [- - - -]."          }//7
};

//====================================================================//

//-S- PLCM_DNS ------------------------------//

//====================================================================//
char G_ReqType[4][32]={
	{"A(IPv4)   "}
	, {"AAAA(IPv6)"}
	, {"SRV       "}
	, {"!!UNDEF   "}
};

char * plcmDNS_szGetReqTypeName(unsigned short par_ReqType)
{
	char * pRc = G_ReqType[3];

	if(eDNS_TYPE_A_IPv4 == par_ReqType) pRc = G_ReqType[0];
	else if(eDNS_TYPE_AAAA_IPv6 == par_ReqType) pRc = G_ReqType[1];
	else if(eDNS_TYPE_SRV == par_ReqType) pRc = G_ReqType[2];
	return pRc;
}
//====================================================================//
//====================================================================//
void plcmDNS_SendResolveResultSRV(  WORD				par_wServiceID
								  , char			*	par_szHostName
								  , int					par_eProcessType // eProcessType
								  , void			*	par_DNSMngrRcvMbx			
								  , DWORD				par_dwErrorCode
								  , ipAddressStruct *   par_pIpResolveResult
								  , int                 par_RecodsAmount
								  , unsigned int        par_dwTTL
								  , WORD				par_wPort
								  , WORD				par_wPriority
								  , WORD				par_wWeight )

{
	char		*	pSzPrintLine		= NULL;
	CSegment	*	pRetParam			= new CSegment();
	COsQueue	*   psQueue             = (COsQueue *)par_DNSMngrRcvMbx;

	if(NULL != pRetParam)
	{
		ipAddressStruct aIpResolve_IND[PLCM_MAX_RESOLVED_IP];
		memset(aIpResolve_IND, 0, sizeof(ipAddressStruct) * PLCM_MAX_RESOLVED_IP);
		char errReason[DnsErrDescSize] = "\0";


		*pRetParam	<< (DWORD) par_wServiceID
			<< par_szHostName
			<< par_szHostName
			<< (DWORD) ((0 == par_RecodsAmount)? 1:0)
			<< errReason
			<< (DWORD) par_dwTTL
			<< (WORD)  par_wPort
			<< (WORD)  par_wPriority
			<< (WORD)  par_wWeight;


		{
			for(int nIn = 0; nIn < par_RecodsAmount; nIn++) 
			{
				aIpResolve_IND[nIn].ipVersion = par_pIpResolveResult[nIn].ipVersion;

				if(eIpVersion4 == par_pIpResolveResult[nIn].ipVersion)
				{
					aIpResolve_IND[nIn].addr.v4.ip = htonl(par_pIpResolveResult[nIn].addr.v4.ip);
				}
				else
					if(eIpVersion6 == par_pIpResolveResult[nIn].ipVersion)
					{
						memcpy(aIpResolve_IND[nIn].addr.v6.ip, par_pIpResolveResult[nIn].addr.v6.ip, sizeof(aIpResolve_IND[nIn].addr.v6.ip));
						enScopeId dwScopeID;
						char szIP[256];
						memset(szIP, '\0', sizeof(szIP));
						ipV6ToString(par_pIpResolveResult[nIn].addr.v6.ip, szIP, FALSE);
						dwScopeID = ::getScopeId(szIP);
						aIpResolve_IND[nIn].addr.v6.scopeId = (APIU32)dwScopeID;
					}
			}
		}  


		pRetParam->Put((BYTE*)aIpResolve_IND, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

		eOtherProcessQueueEntry queueType = eManager;

		if(par_eProcessType == eProcessConfParty)
			queueType = eDispatcher;

		const COsQueue	* pProccessMbx = CProcessBase::GetProcess()->GetOtherProcessQueue((eProcessType)par_eProcessType, queueType);


		if(STATUS_OK != par_dwErrorCode)
		{//if failed
			switch(par_eProcessType)
			{
			case eProcessConfParty	: {pSzPrintLine = G_plcmDnsSendResultPrint[0];} break;
			case eProcessSipProxy	: {pSzPrintLine = G_plcmDnsSendResultPrint[1];} break;
			case eProcessGatekeeper	: {pSzPrintLine = G_plcmDnsSendResultPrint[2];} break;
			default					: {pSzPrintLine = G_plcmDnsSendResultPrint[3];} break;
			}
		}
		else
		{// if OK
			switch(par_eProcessType)
			{
			case eProcessConfParty	: {pSzPrintLine = G_plcmDnsSendResultPrint[4];} break;
			case eProcessSipProxy	: {pSzPrintLine = G_plcmDnsSendResultPrint[5];} break;
			case eProcessGatekeeper	: {pSzPrintLine = G_plcmDnsSendResultPrint[6];} break;
			default					: {pSzPrintLine = G_plcmDnsSendResultPrint[7];} break;
			}
		}


		{
			string RString_ResIND;

			char szTxt[1024]="";
			snprintf(szTxt, sizeof(szTxt)-1, "PLCM_DNS. DNS_SERVICE_IND. |SRVName:[%s] | ServiceId:[%d] | Process:[%s]\n"
				, par_szHostName
				, par_wServiceID
				, CProcessBase::GetProcessName((eProcessType)par_eProcessType));
			RString_ResIND.append((const char*)szTxt);

			vDebugPrintAddrList_String(par_pIpResolveResult, par_RecodsAmount, &RString_ResIND);
			FTRACEINTO << RString_ResIND.c_str();
		}


		switch(par_eProcessType)
		{//SENDING TO...
		case eProcessConfParty	: {if(NULL != psQueue) psQueue->Send(pRetParam, DNS_SERVICE_IND);} break;

		case eProcessSipProxy	: {CSipProxyTaskApi api(par_wServiceID);   STATUS res = api.SendMsg(pRetParam, DNS_SERVICE_IND);}break;
		case eProcessGatekeeper	: {CGatekeeperTaskApi api(par_wServiceID); STATUS res = api.SendMsg(pRetParam, DNS_SERVICE_IND);}break;

		default					: {pProccessMbx->Send(pRetParam,DNS_SERVICE_IND);       }break;//NEED TO CHECK IF DNS_SERVICE_IND
		}

	}
}
//====================================================================//
//====================================================================//

void	plcmDNS_SendResolveResult(  WORD				par_wServiceID
								  , char			*	par_szHostName
								  , int					par_eProcessType // eProcessType
								  , void			*	par_DNSMngrRcvMbx			
								  , DWORD				par_dwErrorCode
								  , ipAddressStruct *   par_pIpResolveResult
								  , int                 par_RecodsAmount)
{
	char		*	pSzPrintLine		= NULL;
	CSegment	*	pRetParam			= new CSegment();
	COsQueue	*   psQueue             = (COsQueue *)par_DNSMngrRcvMbx;

	if((NULL != pRetParam)&&(NULL != par_szHostName))
	{
		ipAddressStruct aIpResolve_IND[PLCM_MAX_RESOLVED_IP];
		memset(aIpResolve_IND, 0, sizeof(ipAddressStruct) * PLCM_MAX_RESOLVED_IP);


		*pRetParam << par_wServiceID
				   << (('\0' != par_szHostName[0])? par_szHostName:"");


		{
			for(int nIn = 0; nIn < par_RecodsAmount; nIn++) 
			{
				aIpResolve_IND[nIn].ipVersion = par_pIpResolveResult[nIn].ipVersion;

				if(eIpVersion4 == par_pIpResolveResult[nIn].ipVersion)
				{
					aIpResolve_IND[nIn].addr.v4.ip = htonl(par_pIpResolveResult[nIn].addr.v4.ip);
				}
				else
				if(eIpVersion6 == par_pIpResolveResult[nIn].ipVersion)
				{
					memcpy(aIpResolve_IND[nIn].addr.v6.ip, par_pIpResolveResult[nIn].addr.v6.ip, sizeof(aIpResolve_IND[nIn].addr.v6.ip));
					enScopeId dwScopeID;
					char szIP[256];
					memset(szIP, '\0', sizeof(szIP));
					ipV6ToString(par_pIpResolveResult[nIn].addr.v6.ip, szIP, FALSE);
					dwScopeID = ::getScopeId(szIP);
					aIpResolve_IND[nIn].addr.v6.scopeId = (APIU32)dwScopeID;
				}
			}
		}  


		pRetParam->Put((BYTE*)aIpResolve_IND, TOTAL_NUM_OF_IP_ADDRESSES*sizeof(ipAddressStruct));

		eOtherProcessQueueEntry queueType = eManager;

		if(par_eProcessType == eProcessConfParty)
			queueType = eDispatcher;

		const COsQueue	* pProccessMbx = CProcessBase::GetProcess()->GetOtherProcessQueue((eProcessType)par_eProcessType, queueType);

		
		if(STATUS_OK != par_dwErrorCode)
		{//if failed
			switch(par_eProcessType)
			{
			case eProcessConfParty	: {pSzPrintLine = G_plcmDnsSendResultPrint[0];} break;
			case eProcessSipProxy	: {pSzPrintLine = G_plcmDnsSendResultPrint[1];} break;
			case eProcessGatekeeper	: {pSzPrintLine = G_plcmDnsSendResultPrint[2];} break;
			default					: {pSzPrintLine = G_plcmDnsSendResultPrint[3];} break;
			}
		}
		else
		{// if OK
			switch(par_eProcessType)
			{
			case eProcessConfParty	: {pSzPrintLine = G_plcmDnsSendResultPrint[4];} break;
			case eProcessSipProxy	: {pSzPrintLine = G_plcmDnsSendResultPrint[5];} break;
			case eProcessGatekeeper	: {pSzPrintLine = G_plcmDnsSendResultPrint[6];} break;
			default					: {pSzPrintLine = G_plcmDnsSendResultPrint[7];} break;
			}
		}
		

		{
			string RString_ResIND;

			char szTxt[1024]="";
			snprintf(szTxt, sizeof(szTxt)-1,"PLCM_DNS. DNS_RESOLVE_IND. | HostName:[%s] | ServiceId:[%d] | Process:[%s] \n"
				, ('\0' != par_szHostName[0])? par_szHostName:"EMPTY!!!"//--- BRIDGE-17142 ---
					 , par_wServiceID
					 , CProcessBase::GetProcessName((eProcessType)par_eProcessType));

			RString_ResIND.append((const char*)szTxt);

			vDebugPrintAddrList_String(par_pIpResolveResult, par_RecodsAmount, &RString_ResIND);
			FTRACEINTO << RString_ResIND.c_str(); ;
		}


		switch(par_eProcessType)
		{//SENDING TO...
		case eProcessConfParty	: {
			FTRACEINTO << "Send DNS resolve answer to party.";
			if(NULL != psQueue) psQueue->Send(pRetParam, DNS_RESOLVE_IND);
		} break;
		case eProcessSipProxy	: {CSipProxyTaskApi api(par_wServiceID);
								   STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);}break;
		case eProcessGatekeeper	: {CGatekeeperTaskApi api(par_wServiceID);
								   STATUS res = api.SendMsg(pRetParam, DNS_RESOLVE_IND);}break;
		//-S- ----- BRIDGE-17142 ---------------------------------//
		default					: 
		{
			if(par_eProcessType > 0)
			{
				pProccessMbx->Send(pRetParam,DNS_RESOLVE_IND);    
			}
		}
		break;
		//-E- ----- BRIDGE-17142 ---------------------------------// 
		}
	}
}
//====================================================================//
//====================================================================//
BOOL spIsIpV6StrA(char *sIpAddress)
{
  int index;
  //If we add port-number to the IPv6, the IPv6 must be under [].
  if (sIpAddress[0] == '[')
	  return TRUE;
  //In IPv6 one of the first 5 characters must be ':'
  for (index=0; index<5; index++){
	  if (sIpAddress[index] == ':'){
		  return TRUE;
	  }
  }

  return FALSE;
}//spIsIpV6StrA
//====================================================================//
//====================================================================//
BOOL spIsIpV4StrA(char *sIpAddress)
{
  BOOL bRc = FALSE;

  if(NULL != sIpAddress)
  {
	  size_t iNd = 0;
	  for (iNd=0; iNd<strlen(sIpAddress); iNd++)
	  {
		  if(  ((sIpAddress[iNd] >= '0')&&(sIpAddress[iNd] <= '9'))
			  ||('.' == sIpAddress[iNd])
			  )
		  {
			  bRc = TRUE;
		  }
		  else
		  {
			  bRc = FALSE;
			  break;
		  }
	  }
  }
  return bRc;
}//spIsIpV4StrA
//====================================================================//
//====================================================================//
BOOL   bIsItIpAddress(char * pHostName, int * par_pIpType)
{
  BOOL  bRc = FALSE;
  if(NULL != pHostName)
  {
	  if(TRUE == spIsIpV6StrA(pHostName))
	  {
		  bRc = TRUE;
		  if(NULL != par_pIpType)
			  *par_pIpType = eIpVersion6;
	  }
	  else
		  if(TRUE == spIsIpV4StrA(pHostName))
		  {
			  bRc = TRUE;
			  if(NULL != par_pIpType)
				  *par_pIpType = eIpVersion4;
		  }
  }

  return bRc;
}
//====================================================================//
//====================================================================//
//==================================================================//
void plcmPrintBuffHexStr(char * par_pBuff, int par_BuffLen, char * par_Txt, int par_nIsShortPrint)
{
	int nInd	= 0;
	int nS		= 0;
	int nLine	= 0;
    char       szLog[2048]="";  
	string      Str;

	char szLogHeadersBuff[1024*10]="";
	char szLogHeadersTxt [1024*10]="";
	memset(szLogHeadersBuff, '\0', sizeof(szLogHeadersBuff));
	memset(szLogHeadersTxt , '\0', sizeof(szLogHeadersTxt));

	if((NULL != par_pBuff)&&(par_BuffLen > 0))
	{
		int nLineAmount = par_BuffLen / 16;
		if(0 != (par_BuffLen % 16))
			nLineAmount++;

		snprintf(szLog, sizeof(szLog)-1,"plcmPrintBuffHex. buff.Len:[%d]   %s\n", par_BuffLen, (NULL != par_Txt)? par_Txt:""); 
		Str.append((const char*)szLog);

		for(nInd= 0; nInd < par_BuffLen; nInd++)
		{
			char szTmp[32]="";
			char szTxt[32]="";

			nS++;

			sprintf(szTmp, "%02X ", (unsigned char)(par_pBuff[nInd]));
			strcat(szLogHeadersBuff, szTmp);
			if(  (par_pBuff[nInd] >=0x20) 
				&&(par_pBuff[nInd] <=0x7E) )
			{
				sprintf(szTxt,"%c", par_pBuff[nInd]);
			}
			else
			{
				sprintf(szTxt,"%c", '.');
			}

			strcat(szLogHeadersTxt, szTxt);

			if( (16 == nS) || (par_BuffLen == (nInd +1)) )
			{
				if(16 != nS)
				{
					int nSp = 0;
					for(nSp = 0; nSp < (16 - nS); nSp++)
						strcat(szLogHeadersBuff,"   ");
					//strcat(szLogHeadersBuff, "|   ");
				}
				//else;
				//strcat(szLogHeadersBuff, "|   ");

				nLine++;

				if(0 == par_nIsShortPrint) 
				{
					snprintf(szLog, sizeof(szLog)-1,"   %s | %s\n", szLogHeadersBuff, szLogHeadersTxt); 
					Str.append((const char*)szLog);
				}
				else
				{
					if((1 == nLine)||(2== nLine))
					{
						snprintf(szLog, sizeof(szLog)-1,"   %s | %s\n", szLogHeadersBuff, szLogHeadersTxt); 
						Str.append((const char*)szLog);
					}
					if(2 == nLine)
					{
						snprintf(szLog, sizeof(szLog)-1,"        . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .\n"); 
						Str.append((const char*)szLog);
					}
					if((nLine == nLineAmount-1)||(nLine == nLineAmount-2) || (nLine == nLineAmount))
					{
						snprintf(szLog, sizeof(szLog)-1,"   %s | %s\n", szLogHeadersBuff, szLogHeadersTxt); 
						Str.append((const char*)szLog);
					}
				}
				memset(szLogHeadersBuff, '\0', sizeof(szLogHeadersBuff));
				memset(szLogHeadersTxt , '\0', sizeof(szLogHeadersTxt));
				nS = 0;
			}
		}
		snprintf(szLog, sizeof(szLog)-1,"----------------------------------------------------|-----------------");
		Str.append((const char*)szLog);

		FTRACEINTO << Str.c_str();
	}
	else
	{
		snprintf(szLog, sizeof(szLog)-1,"plcmPrintBuffHex. !!!Error incoming parameters buff.Len:[%d] | %s", par_BuffLen, (NULL != par_Txt)? par_Txt:"");
		FTRACEINTO << szLog;
	}
}
//==================================================================//

//-E- PLCM_DNS ------------------------------//
