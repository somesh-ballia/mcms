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
#include <string.h>

#include "LinuxSystemCallsApi.h"
#include "Print.h"


/***** Public Variables *****/
static UINT8 ucMessageCounter = 1;
TMessageQueueParams tTMessageQueueParams[MAX_NUM_OF_MESSAGE_QUEUE];

UINT32					ulMinPriority, ulMaxPriority, ulMidPriority;
pthread_attr_t 			tAttr;
struct sched_param		tSched;

UINT32					ulOtherMinPriority, ulOtherMaxPriority, ulOtherMidPriority;
pthread_attr_t 			tOtherAttr;
struct sched_param		tOtherSched;


/***** Global Variables *****/
INT32 ut_retValmsgGet;
INT8 *ut_retValgetenv;
UINT8 ut_flagpErrorWasCalled = 0;
UINT8 *ut_valpErrorLastMessage = NULL;
#define DHOME_STRING "/"

ThreadIdStr tThreadDesc[eMaxThread];
INT8 cThreadName[eMaxThread][40] = {
	"MainThread",
	"WDThread",
	"TimerMngrThread", 
	"PrintThread",
	"CmdCardThread",
	"NtpThread",
	"DispatcherThread",
	"IpmiListenThread",
	"SwDiagListenThread",
	"EmaListenThread",
	"LanStatListenThread",
	"EmaSimListenThread",
	"MfaResetListnThread",
	"TCPXmitThread",
	"TCPrcvThread",
	"LSPhyTask",
	"LSPollStatistics",
	"ShelfRcvThread",
	"CpuDiagListenThread"
	"EthMonitoringThread",
	"Start4boardServerThread",
	"LanTestConsultThread",
	"IsToStartTestsThread",
	"SwitchDiagTestsThread",
	"IpmiHandleThread",
	"LanHandleThread",
	};

extern UINT32	IAmChildProcess;

/***** Public Functions Declarations *****/

#ifdef x86_ARCH
void CancelAndWaitThread(TThreadNumber threadIndex);
#endif

/***** Private Functions Declarations *****/

/***** Functions Code *****/
/****************************************************************************
* Prototype:        InitSchedStruct
* Description:      initialize scheduling struct used for creating threads
* Return Value:     no.
* Arguments:        no.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
void InitSchedStruct()
{
	int rc;
	int i;
  
  	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct");
  	
  	memset(&tAttr,0,sizeof(tAttr));
  	
	ulMaxPriority = sched_get_priority_max(SCHED_FIFO);
	ulMinPriority = sched_get_priority_min(SCHED_FIFO);
	ulMidPriority = (ulMaxPriority - ulMinPriority) / 2;
	
	tSched.sched_priority = ulMaxPriority;
	rc = sched_setscheduler(0, SCHED_FIFO, &tSched);

	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct: min priority=%d, max priority=%d", ulMinPriority, ulMaxPriority);
	
	//clear thread list array:
	for(i=0; i < eMaxThread; i++)
	{
		tThreadDesc[i].ul_ThreadId = 0;
#ifdef x86_ARCH
		tThreadDesc[i].b_IsStarted      = 0;
		tThreadDesc[i].pThreadId      = 0;
#endif
    	tThreadDesc[i].l_ThreadPriority = 0xDEADBEEF; 
		memcpy( &(tThreadDesc[i].c_ThreadName[0]), cThreadName[i], 40);
	}

	//set MAIN process attributes - priority and scheduler:
	if( (2 >= ulMinPriority) && (2 <= ulMaxPriority) )
		tSched.sched_priority = 2;
	else
		tSched.sched_priority = ulMidPriority;

	if(IAmChildProcess)
	{
		tSched.sched_priority = 2;
	}

	rc = sched_setscheduler(0, SCHED_FIFO, &tSched);
	if(rc != 0)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct: MAIN setschedpolicy failed with error 0x%x", rc);
	}

	//enlist main thread in threads list: 
	tThreadDesc[eMainThread].ul_ThreadId = gettid();
	memcpy(tThreadDesc[eMainThread].c_ThreadName, cThreadName[eMainThread], 40);
	tThreadDesc[eMainThread].l_ThreadPriority = getThreadPriority();
	

	//init general attributes object - template used to create threads:
	rc = pthread_attr_init(&tAttr);	
	if(rc != 0)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct: pthread_attr_init failed with error 0x%x", rc);
	}
	else
	{
		//set scheduling method to FIFO:
		rc = pthread_attr_setschedpolicy(&tAttr, SCHED_FIFO);
		if(rc != 0)
		{
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct: setschedpolicy failed with error 0x%x", rc);
		}
	}

   return;
}


/****************************************************************************
* Prototype:        SetZeroMessageParam
* Description:      set 0 to the message param structure
* Return Value:     no.
* Arguments:        no.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
void SetZeroMessageParam()
{
	UINT32 i;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"SetZeroMessageParam");
	
	for (i = 0 ; i < MAX_NUM_OF_MESSAGE_QUEUE ; i++)
	{
		if (i > 0)
		{
			tTMessageQueueParams[i].unMessageCounter = i;
			tTMessageQueueParams[i].unMessageQid = 0xffffffff;
		}
		else
		{
			tTMessageQueueParams[i].unMessageCounter = 0xffffffff;
			tTMessageQueueParams[i].unMessageQid = 0xffffffff;
		}
	}
}




/****************************************************************************
* Prototype:        API_MsgGet
* Description:      get the message qid
* Return Value:     message queue id.
* Arguments:        key - key of o.s. , unFalgs - creating and security flags.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 API_MsgGet(key_t key,INT32 unFlags)
{
	
	//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"API_MsgGet");
	
	#ifdef TDD_TEST_ON
		return ut_retValmsgGet;
	#else
		return msgget(key,unFlags);
	#endif
}



/****************************************************************************
* Prototype:        API_MsgCtl
* Description:      control the message queue
* Return Value:     success or fail -1;.
* Arguments:        nQid - msg qid , unFalgs - control flags , buf - insert the data.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 API_MsgCtl(INT32 nQid , INT32 nFlags , struct msqid_ds *buf)
{
	
	//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"API_MsgCtl");
	
	#ifdef TDD_TEST_ON
		return ut_retValmsgGet;
	#else
		return msgctl(nQid,nFlags,buf);
	#endif
}




/****************************************************************************
* Prototype:        API_pError
* Description:      print error
* Return Value:     
* Arguments:        error - string of error.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
void API_pError(UINT8 *error)
{
	
	//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"API_pError");
	
	#ifdef TDD_TEST_ON
		ut_flagpErrorWasCalled=1;
        MfaBoardPrint(SHERAD_SWITCH_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"%s\n",ut_valpErrorLastMessage);
	#else
		perror(error);
	#endif
}



/****************************************************************************
* Prototype:        API_getEnv
* Description:      get the env pointer
* Return Value:     env pointer
* Arguments:        *pucEnv - the env string.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
//const UINT8* API_getEnv(UINT8 *pucEnv)
//{
//	
//	//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"API_getEnv");
//	
//	#ifdef TDD_TEST_ON
//		return ut_retValgetenv;
//	#else
//		return (DHOME_STRING);
//	#endif
//}

/****************************************************************************
* Prototype:        API_getEnv
* Description:      get the env pointer
* Return Value:     env pointer
* Arguments:        *pucEnv - the env string.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
const UINT8* API_getEnv(UINT8 *pucEnv)
{
	return (getenv(pucEnv));
}

/****************************************************************************
* Prototype:        API_setEnv
* Description:      set the env pointer
* Return Value:     env pointer
* Arguments:        *pucEnv - the env string.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
void API_setEnv(UINT8 *pucEnv, UINT8 *punMyEnv)
{
	setenv(pucEnv, punMyEnv, 0);
}

/****************************************************************************
* Prototype:        CreateMessageQueue
* Description:      create message queue
* Return Value:     message queue id.
* Arguments:        pucMessageName - message name , unFalgs - creating and security flags.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 CreateMessageQueue()//IPC_CREAT , IPC_EXCL , PERMITIONS
{

	INT32 nQid, nCounter;
	key_t key;
	const UINT8 *ucEnv=DHOME_STRING;

	
    UINT32 i = 2;
    
    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"CreateMessageQueue");
	
	while ( (i < 0xff) && (tTMessageQueueParams[i].unMessageQid != 0xffffffff) )
	{
		i++;
	}
	
	if (i == 0xff)
	{
		API_pError("No More message key");
		return -1;
	}
		
	nCounter = tTMessageQueueParams[i].unMessageCounter;

	key = ftok(ucEnv,nCounter);

    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"*** using %s %d\n", ucEnv, nCounter);
	if (key == -1)
	{
		API_pError("(ftok) Fail Create Message Queue");
		return -1;
	}

	nQid = API_MsgGet(key,IPC_CREAT|S_IRWXU);
	if (nQid == -1) 
	{ 
		API_pError("(msgget)Fail Create Message Queue");
		return -1;
	}	
	
	tTMessageQueueParams[i].unMessageQid = nQid;
	ucMessageCounter++;
		
	return (nQid); // uQid success , -1 fail
}



/****************************************************************************
* Prototype:        RemoveMessageQueue
* Description:      remove message queue
* Return Value:     message queue id.
* Arguments:        nQid - message queue id.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 RemoveMessageQueue(INT32 nQid)//IPC_CREAT , IPC_EXCL , PERMITIONS
{
	struct msqid_ds buf;
	INT32 nRc , i = 1;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"RemoveMessageQueue");
	
	nRc = API_MsgCtl(nQid,IPC_RMID,&buf);
	if (nRc == -1)
	{ 
		API_pError("(msgget)Fail Remove Message Queue");
	}
	
	while (tTMessageQueueParams[i].unMessageQid != (UINT32)nQid)
	{
		i++;
	}
	
	if (i < 0xff)
	{
		tTMessageQueueParams[i].unMessageQid = 0;	
	}
	ucMessageCounter--;
	 
	return (nQid); // uQid success , -1 fail
}




void RemoveAllMessageQueue()
{
	UINT32 i;
	INT32 nRc;
	key_t key;
//	const UINT8 *ucEnv=DHOME_STRING;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"RemoveAllMessageQueue");
		
	for (i = 1 ; i < MAX_NUM_OF_MESSAGE_QUEUE ; i++)
	{
		if (tTMessageQueueParams[i].unMessageQid != 0xffffffff)
			nRc = RemoveMessageQueue(tTMessageQueueParams[i].unMessageQid);
	}	
}





/****************************************************************************
* Prototype:        SendMessage
* Description:      send message 
* Return Value:     return value of the function msgsnd.
* Arguments:        nQid - message qid , ptMessageType - data and type , nMessageFlag - wait/nowait.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 SendMessage(INT32 nQid , TMessageThreadType *ptMessageThreadType , INT32 nMessageFlag)
{
	TMessageType tMessageType;
	INT32 nReturnValue;
	INT8 acPrint[200] ;
	
	//MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"SendMessage on Queue id : %d",nQid);

	
	tMessageType.nType = 60;
	memcpy(tMessageType.pucData,ptMessageThreadType,sizeof(TMessageThreadType));

	nReturnValue  = msgsnd(nQid , &tMessageType , sizeof(TMessageThreadType) ,nMessageFlag);
	if (nReturnValue == -1)
	{
		API_pError("SendMessage Fail Send Message to Queue");
	}

		
	return (nReturnValue); // 0 success , -1 fail
}




/****************************************************************************
* Prototype:        ReceiveMessage
* Description:      Receive Message 
* Return Value:     return value of the function msgsnd.
* Arguments:        nQid - message qid , ptMessageType - data and type ,unMessageSize - msg size , nMessageType- msg type like the msgsnd , nMessageFlag - wait/nowait.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 ReceiveMessage(INT32 nQid , TMessageThreadType *ptMessageThreadType, INT32 nMessageFlag) //IPC_NOWAIT,0
{
	INT32 nReturnValue;
	TMessageType tMessageType;
	INT8 acPrint[200] ;
	
    //MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"Received Message on Queue %d",nQid);

    
    tMessageType.nType = 60;

	nReturnValue  = msgrcv(nQid , &tMessageType ,sizeof(TMessageThreadType)/*unMessageSize*/ , tMessageType.nType  , nMessageFlag);

	memcpy(ptMessageThreadType,tMessageType.pucData,sizeof(TMessageThreadType));

	if((nMessageFlag & IPC_NOWAIT) != IPC_NOWAIT)
	{
		if (nReturnValue == -1)
		{
			API_pError("Fail Recive Message From Queue");
		}
	}
	
    return (nReturnValue); // -1 fail else num of bytes rcv
}




/****************************************************************************
* Prototype:        CreateProcessMessageQueue
* Description:      create message queue
* Return Value:     message queue id.
* Arguments:        nCounter - for choosing key ID.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 CreateProcessMessageQueue(INT32 nCounter)//IPC_CREAT , IPC_EXCL , PERMITIONS
{

	INT32 nQid;
	key_t key;
	const UINT8 *ucEnv=DHOME_STRING;

	
    UINT32 i = 2;
    
    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MAJOR,PRINT_TO_TERMINAL,"CreateMessageQueue");
	
	while ((i < 0xff) && (tTMessageQueueParams[i].unMessageQid != 0xffffffff))
	{
		i++;
	}
	
	if (i == 0xff)
	{
		API_pError("No More message key");
		return -1;
	}
		
	key = ftok(ucEnv,nCounter);

    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"*** using %s %d\n", ucEnv, nCounter);

	if (key == -1)
	{
		API_pError("(ftok) Fail Create Message Queue");
		return -1;
	}

	nQid = API_MsgGet(key,IPC_CREAT|S_IRWXU);
	if (nQid == -1) 
	{ 
		API_pError("(msgget)Fail Create Message Queue");
		return -1;
	}	
	
		
	return (nQid); // uQid success , -1 fail
}

/****************************************************************************
* Prototype:        SendProcessMessage
* Description:      send message 
* Return Value:     return value of the function msgsnd.
* Arguments:        nQid - message qid ,nMessageFlag - wait/nowait,nType - type inside queue,
*                         pSendPtr - ptr to send data from, size - message size.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 SendProcessMessage(INT32 nQid  ,
                  INT32 nMessageFlag, 
                  UINT32 nType, 
                  TMessageProcessType *pSendPtr,
                  UINT32 size)
{
	INT32 nReturnValue;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"SendMessage on Queue id : %d",nQid);

	
	pSendPtr->nType = nType;

	nReturnValue  = msgsnd(nQid ,pSendPtr  , size ,nMessageFlag);
	if (nReturnValue == -1)
	{
		API_pError("SendProcessMessage Fail Send Message to Queue");
	}

		
	return (nReturnValue); // 0 success , -1 fail
}

/****************************************************************************
* Prototype:        ReceiveProcessMessage
* Description:      Receive Message 
* Return Value:     return value of the function msgsnd.
* Arguments:        nQid - message qid , nMessageFlag - wait/nowait, 
*                   nType - message type ,
*                   pRcvPtr - pointer to receive data to, size - size of data 
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 ReceiveProcessMessage(INT32 nQid ,
                            INT32 nMessageFlag,
                            UINT32 nType,
                            TMessageProcessType *pRcvPtr,
                            UINT32 size) //IPC_NOWAIT,0
{
	INT32 nReturnValue;
	
    MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"Received Process Message on Queue %d",nQid);

    
	memset(pRcvPtr->pucData,0,size);

    pRcvPtr->nType = nType;
	nReturnValue  = msgrcv(nQid ,pRcvPtr  ,size , nType  , nMessageFlag);


	if((nMessageFlag & IPC_NOWAIT) != IPC_NOWAIT)
	{
		if (nReturnValue == -1)
		{
			API_pError("Fail Recive Message From Queue");
		}
	}
	
    return (nReturnValue); // -1 fail else num of bytes rcv
}



/****************************************************************************
* Prototype:        CreateSharedMemory
* Description:      Create Shared Memory 
* Return Value:     return value of the shared mem pointer - also insert to punShmemId the shmem id..
* Arguments:        pucSharedMemoryName - name , unSize - unSize ,unFalgs - IPC_CREAT , IPC_EXCL , punShmemId - get the shmem id..
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
UINT8* CreateSharedMemory(UINT32 unSize , UINT8 *pucMemAddr ,  UINT32 *punShmemId)// IPC_CREAT , IPC_EXCL
{
	INT32 nSmid;
	UINT8 *pucSmPtr;
	INT32 nFlags;
	UINT8 *pucTempAddr;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"CreateSharedMemory");
	
		
	key_t key = ftok(DHOME_STRING,255);
	nSmid = shmget(key, unSize ,IPC_CREAT|S_IRWXU);
	if (nSmid == -1)
	{
		perror("Fail Create Shared Memory");
		return (NULL); 
	}
	else
	{
		//attach the shared memory.
		if (pucMemAddr == NULL)
		{
			pucTempAddr = 0;
			nFlags = 0;	
		}
		else
		{
			pucTempAddr = pucMemAddr;
			nFlags = SHM_RND;
		}
		pucSmPtr = (UINT8*)shmat(nSmid,pucTempAddr,0);
		if (pucSmPtr == (UINT8*)-1)
		{
			perror("Fail Attach Shared Memory");
			return (NULL); 
		}
		else
		{
			*punShmemId = nSmid;
			return (pucSmPtr); 
		}
	}
}






/****************************************************************************
* Prototype:        RemoveSharedMemory
* Description:      Remove Shared Memory 
* Return Value:     return value of the shared mem cntl  -1 fail 0 success
* Arguments:        unSmid - id 
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
INT32 RemoveSharedMemory(UINT32 unSmid)
{
	INT32 nRc;
	struct shmid_ds  *ptShmid_ds = NULL; 
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"RemoveSharedMemory");
 
		
	nRc = shmctl(unSmid, IPC_RMID, ptShmid_ds);
	if (nRc == -1)
	{
		perror("Fail Delete Shared Memory");
	}
	
	return (nRc);
}


/****************************************************************************
* Prototype:        CreateProcess
* Description:      Create Process 
* Return Value:     return value of the process id 
* Arguments:        unSmid - id 
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
pid_t CreateProcessWithExeFile(char *pFile)
{
	pid_t pid;
	char * args[1];
	args[0] = NULL;
				
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateProcess: %s\n", pFile);
		
	fflush(stdout);
	pid = fork();
	switch (pid)
	{
		case -1: 
		{
			perror("Error Create e\n");
			break;
		}

		case 0: 
		{
			execv(pFile, args);
			break;
		}

		default: 
		{
			break;
		}
	}
	return (pid);
}

/****************************************************************************
* Prototype:        CreateProcess
* Description:      Create Process 
* Return Value:     return value of the process id 
* Arguments:        unSmid - id 
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
pid_t CreateProcessWithFunc(void *pFuncAddr)
{
	pid_t pid;
	
//	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateProcessWithFunc");
		
	FuncToRun fFuncToRun = (FuncToRun)pFuncAddr;

	fflush(stdout);

	pid = fork();
//int clone(int (*fn)(void *), void *child_stack, int flags, void *arg); 
//	pid = clone(CLONE_VM);
	printf("(CreateProcessWithFunc): pid = %d\n", pid);

	switch (pid)
	{
		case -1: 
		{
			perror("Error Create Fork\n");
			break;
		}

		case 0: 
		{
			printf("child process is running. pid = %d\n", pid);
			fFuncToRun();
			break;
		}

		default: 
		{
			printf("I am in father process. pid = %d\n", pid);
			break;
		}
	}
	
	
	return (pid);

}






/****************************************************************************
* Prototype:        CreateThread
* Description:      Create Thread 
* Return Value:     return value of the process id 
* Arguments:        unSmid - id 
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
/*
pthread_t CreateThread(void *pFuncAddr)
{
	pthread_t ptThread;
	INT32     uReturn;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"CreateThread");

	uReturn = pthread_create (&ptThread,NULL,pFuncAddr,NULL);
	if (uReturn)
	{
		perror("Fail Create Thread");
	}

	return (ptThread);
}
*/


typedef struct
{
    void *funcAddr;
    char funcName[128];
}threadParams;

// append to file
void RegisterNewThread(threadParams *pThreadParams)
{
    static int init = 0;
#if 0
    const char *pFileName = "/mnt/mfa_cm_fs/threads.txt";
#else
    const char *pFileName = "/tmp/mfa_cm_fs/threads.txt";
#endif
    char buffer[256];
    
    if(0 == init)
    {
        init = 1;

        snprintf(buffer, sizeof(buffer), "echo \"-------- thread list of the CURRENT session -------\" > %s", pFileName);
        system(buffer);
    }
    
    snprintf(buffer, sizeof(buffer), "echo \"Thread %d, %s\" >> %s", gettid(), pThreadParams->funcName, pFileName);
    system(buffer);
}

 // will 1) register thread id, thread name
 //      2) run it
void* thread_ctor(void *p)
{
typedef void* ClientFunc(void*);
    
    threadParams *pParams = (threadParams*)p;

    // 1) register
    RegisterNewThread(pParams);    

    // 2) run the function
    ((ClientFunc*)pParams->funcAddr)((void*)NULL);

    free(pParams);
    return NULL;
}

pthread_t CreateThreadWrapper(void *pFuncAddr, const char *pFuncName, UINT32 ulPriority)
{
    pthread_t ptThread;
	INT32     uReturn = -1, rc;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"CreateThread");

	memset(&ptThread,0,sizeof(pthread_t));
	
	if( (ulPriority >= ulMinPriority) && (ulPriority <= ulMaxPriority) )
	{
		tSched.sched_priority = ulPriority;
		rc = pthread_attr_setschedparam(&tAttr, &tSched);
		if(rc != 0)
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThread: setschedparam failed with error 0x%x", rc);
	}
	else 
	{
		tSched.sched_priority = ulMidPriority;
	}

	rc = pthread_attr_setschedparam(&tAttr, &tSched);
	if(rc != 0)
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThread: setschedparam failed with error 0x%x", rc);

    
    threadParams *pParams = malloc(sizeof(threadParams));
    if(NULL != pParams)
    {
        memset(pParams, 0, sizeof(threadParams));
        pParams->funcAddr = pFuncAddr;
        strncpy(pParams->funcName, pFuncName, sizeof(pParams->funcName) - 1);
    
        uReturn = pthread_create (&ptThread, &tAttr, (void*)thread_ctor, (void*)pParams);
    }

//	uReturn = pthread_create (&ptThread, &tAttr, (void*)pFuncAddr, (void*)NULL);
	if (uReturn)
	{
		perror("Fail Create Thread");
	}

	return (ptThread);
}


/****************************************************************************
* Prototype:        CreateThread
* Description:      Create Thread 
* Return Value:     return value of the process id 
* Arguments:        unSmid - id 
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
/*pthread_t CreateThread1(void *pFuncAddr, UINT32	ulPriority)
{
	pthread_t ptThread;
	INT32     uReturn, rc;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_MINOR,PRINT_TO_TERMINAL,"CreateThread");

	memset(&ptThread,0,sizeof(pthread_t));
	
	if( (ulPriority >= ulMinPriority) && (ulPriority <= ulMaxPriority) )
	{
		tSched.sched_priority = ulPriority;
		rc = pthread_attr_setschedparam(&tAttr, &tSched);
		if(rc != 0)
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThread: setschedparam failed with error 0x%x", rc);
	}
	else 
	{
		tSched.sched_priority = ulMidPriority;
	}

	rc = pthread_attr_setschedparam(&tAttr, &tSched);
	if(rc != 0)
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThread: setschedparam failed with error 0x%x", rc);

	uReturn = pthread_create (&ptThread, &tAttr, (void*)pFuncAddr, (void*)NULL);
	if (uReturn)
	{
		perror("Fail Create Thread");
	}

	return (ptThread);
}
*/

pthread_t CreateThreadOtherWrapper(void *pFuncAddr, const char *pFuncName, UINT32 ulPriority)
{
	pthread_t ptThread;
	INT32     uReturn = -1, rc;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThreadOther");
	memset(&ptThread,0,sizeof(pthread_t));

	if( (ulPriority >= ulOtherMinPriority) && (ulPriority <= ulOtherMaxPriority) )
	{
		tOtherSched.sched_priority = ulPriority;
		rc = pthread_attr_setschedparam(&tOtherAttr, &tOtherSched);
		if(rc != 0)
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThreadOther: setschedparam failed with error 0x%x", rc);

	}
	else 
	{
		tOtherSched.sched_priority = ulOtherMidPriority;
	}

//	printf("\n DBG creating thread with priority %d", tSched.sched_priority); 
	rc = pthread_attr_setschedparam(&tOtherAttr, &tOtherSched);
	if(rc != 0)
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThreadOther: setschedparam failed with error 0x%x", rc);

    threadParams *pParams = malloc(sizeof(threadParams));
    if(NULL != pParams)
    {
        memset(pParams, 0, sizeof(threadParams));
        pParams->funcAddr = pFuncAddr;
        strncpy(pParams->funcName, pFuncName, sizeof(pParams->funcName) - 1);
    
        uReturn = pthread_create (&ptThread, &tAttr, (void*)thread_ctor, (void*)pParams);
    }

    
//	uReturn = pthread_create (&ptThread, &tOtherAttr, pFuncAddr, NULL);
	if (uReturn)
	{
		perror("Fail Create Thread");
	}

	return (ptThread);
}


/****************************************************************************
* Prototype:        CreateThread
* Description:      Create Thread 
* Return Value:     return value of the process id 
* Arguments:        unSmid - id 
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
/*pthread_t CreateThreadOther(void *pFuncAddr, UINT32	ulPriority)
{
	pthread_t ptThread;
	INT32     uReturn, rc;
	
	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThreadOther");

	if( (ulPriority >= ulOtherMinPriority) && (ulPriority <= ulOtherMaxPriority) )
	{
		tOtherSched.sched_priority = ulPriority;
		rc = pthread_attr_setschedparam(&tOtherAttr, &tOtherSched);
		if(rc != 0)
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThreadOther: setschedparam failed with error 0x%x", rc);

	}
	else 
	{
		tOtherSched.sched_priority = ulOtherMidPriority;
	}

//	printf("\n DBG creating thread with priority %d", tSched.sched_priority); 
	rc = pthread_attr_setschedparam(&tOtherAttr, &tOtherSched);
	if(rc != 0)
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"CreateThreadOther: setschedparam failed with error 0x%x", rc);

	uReturn = pthread_create (&ptThread, &tOtherAttr, pFuncAddr, NULL);
	if (uReturn)
	{
		perror("Fail Create Thread");
	}

	return (ptThread);
}
*/

INT32 getThreadPriority()
{
	struct sched_param	tSched;
	INT32  sys_retval, rc = 0xFF;

	sys_retval = sched_getparam(0, &tSched);
	if(sys_retval == 0) //success
		rc = tSched.sched_priority;

	return(rc);
}

void EnrollInThreadList_internal(const char *funcName, UINT32 ulThreadIndex)
{
	ThreadIdStr *pThreadListEntry;

	if( ulThreadIndex < eMaxThread)
	{
		pThreadListEntry = &(tThreadDesc[ulThreadIndex]);

		pThreadListEntry->ul_ThreadId = gettid();	
		strncpy(pThreadListEntry->c_ThreadName, funcName, sizeof(pThreadListEntry->c_ThreadName) - 1);
	    pThreadListEntry->l_ThreadPriority = getThreadPriority();
#ifdef x86_ARCH
	    pThreadListEntry->b_IsStarted      = 1;
	    pThreadListEntry->pThreadId      = pthread_self();
#endif
	}
	else
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"EnrollInThreadList: illegal Thread Index %d. ignoring.", ulThreadIndex);

	return;
}

#ifdef x86_ARCH
void CancelAllThreads()
{
	int threadnum=0;

	for (threadnum=1;threadnum<eMaxThread;threadnum++)//don't cancel itself.
	{
		if (ePrintThread != threadnum) //print thread should be canceled last since others could write some log
		{
			CancelAndWaitThread((TThreadNumber)threadnum);
		}
	}
	CancelAndWaitThread(ePrintThread);
}

void CancelAndWaitThread(TThreadNumber threadIndex)
{
	ThreadIdStr *pThreadListEntry = NULL;
	pThreadListEntry = &(tThreadDesc[threadIndex]);
	if (pThreadListEntry != NULL
			&& pThreadListEntry->b_IsStarted != 0)
	{
		//printf("canceling thread %s[%ul]\n", pThreadListEntry->c_ThreadName, pThreadListEntry->pThreadId);
		pthread_cancel(pThreadListEntry->pThreadId);
		pthread_join(pThreadListEntry->pThreadId, NULL);
	}
}
#endif

/****************************************************************************
* Prototype:        InitSchedStruct
* Description:      initialize scheduling struct used for creating threads
* Return Value:     no.
* Arguments:        no.
* Global Variable Used: 
* Global Variables Changed: 
* Cautions: 
*****************************************************************************/
void InitSchedStruct_Other()
{
	int rc;
  
  	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct_Other");
  	
	ulOtherMaxPriority = sched_get_priority_max(SCHED_OTHER);
	ulOtherMinPriority = sched_get_priority_min(SCHED_OTHER);
	ulOtherMidPriority = (ulOtherMaxPriority - ulOtherMinPriority) / 2;
	
	tOtherSched.sched_priority = ulMaxPriority;
	rc = sched_setscheduler(0, SCHED_OTHER, &tOtherSched);

	MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct_Other: min priority Other=%d, max priority Other=%d", ulOtherMinPriority, ulOtherMaxPriority);
	

	//set MAIN process attributes - priority and scheduler:
	if( (2 >= ulOtherMinPriority) && (2 <= ulOtherMaxPriority) )
		tOtherSched.sched_priority = 2;
	else
		tOtherSched.sched_priority = ulOtherMidPriority;


	rc = sched_setscheduler(0, SCHED_OTHER, &tOtherSched);
	if(rc != 0)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct_Other: MAIN setschedpolicy failed with error 0x%x", rc);
	}


	//init general attributes object - template used to create threads:
	rc = pthread_attr_init(&tOtherAttr);	
	if(rc != 0)
	{
		MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct_Other: pthread_attr_init failed with error 0x%x", rc);
	}
	else
	{
		//set scheduling method to FIFO:
		rc = pthread_attr_setschedpolicy(&tOtherAttr, SCHED_OTHER);
		if(rc != 0)
		{
			MfaBoardPrint(SHARED_PRINT,PRINT_LEVEL_ERROR,PRINT_TO_TERMINAL,"InitSchedStruct_Other: setschedpolicy failed with error 0x%x", rc);
		}
	}

   return;
}

