// 
// ==========================================
//    Copyright (C) 2014           "POLYCOM"
// ==========================================
// FileName:               PlcmDNS_Tools.h  
// Include line recommended:
// #include "PlcmDNS_Tools.h"         //
// 
// ==========================================

#ifndef PLCMDNS_TOOLS_H_
#define PLCMDNS_TOOLS_H_


#define	PL_THREAD_LABEL			            0x54CD7601
#define	PL_THREAD_NAME_SIZE		            64
#define	PL_THREAD_NAME_DEFAULT				"User Thread"
#define PL_PRIORITYVALUE_DEFAULT			127
#define PL_STACKSIZE_DEFAULT    			5120


#include <pthread.h>
#include <vector>

#include "PObject.h"

#include "IpAddressDefinitions.h"
#include "McmsProcesses.h"
#include "OsQueue.h"
#include "PlcmDNS_Defines.h"
#include "PlcmDNS_Packet.h"

#include "StateMachine.h"



//===============================================================================//
typedef enum                    //Resolution of timer
{                               //in the string case it looks like
	  E_TIMER_RESOLUTION_LOWER
	, E_TIMER_RESOLUTION_HOUR   //...12
	, E_TIMER_RESOLUTION_MIN    //...12:40
	, E_TIMER_RESOLUTION_SEC    //...12:40:45
	, E_TIMER_RESOLUTION_MILI   //...12:40:45.056
	, E_TIMER_RESOLUTION_MICRO  //...12:40:45.023456 //this resolution is forbidden  
	//under Windows
	, E_TIMER_RESOLUTION_UPPER
}eTIMER_RESOLUTION;
typedef enum                    //Format of string timer representation
{
	  E_TIMER_FORMAT_LOWER      //              
	, E_TIMER_FORMAT_YEAR       //it looks like 05/06/01  12:40:45 ...  
	, E_TIMER_FORMAT_MONTH      //it looks like 05/06  12:40:45 ...
	, E_TIMER_FORMAT_DAY        //it looks like 05  12:40.45 ...
	, E_TIMER_FORMAT_HOUR       //it looks like 12:40.45 ...
	, E_TIMER_FORMAT_UPPER
}eTIMER_TIME_FORMAT;
// Global Routines:
//-----------------
//===============================================================================//
unsigned int Pm_GetTimeString(  
							    eTIMER_RESOLUTION   par_eTimerResolution
							  , eTIMER_TIME_FORMAT  par_eTimeFormat
							  , unsigned int        par_nStrSize
							  , char *              par_szTimeString);
//-------------------------------------------------------------------------------//
unsigned int Pm_getCurrentTimestampAdv(
									     eTIMER_RESOLUTION    par_eTimerResolution //IN
									   , unsigned int      *  par_tm_pTimeSec    //OUT
									   , unsigned short    *  par_n_pMilliSec    //OUT 
									   );
//-------------------------------------------------------------------------------//
unsigned int Pm_getCurrentTimestamp(eTIMER_RESOLUTION    par_eTimerResolution  );//IN             
//---------------------------------------------------------------------------
unsigned int Pm_GetTimeStringT( unsigned int         par_dwTime           //IN     
							   , eTIMER_RESOLUTION   par_eTimerResolution //IN
							   , eTIMER_TIME_FORMAT  par_eTimeFormat      //IN 
							   , unsigned int        par_nStrSize         //IN
							   , char *              par_szTimeString);   //OUT
//===============================================================================//


typedef pthread_t          PL_THREAD_HANDLE;
typedef unsigned int       PL_INTERVAL_ms; 

typedef	enum
{
	plFAIL = 0, plSUCCESS = 1
}
PLb_RETURN;

typedef enum _THREAD_TYPE
{
	eTHREAD_UNDEF = 0
  , eTHREAD_TACH
  , eTHREAD_DETACH
}
THREAD_TYPE;

typedef enum _PLe_THREAD_STATE
{
	THREAD_STATE_UNBORN = 0
	, THREAD_STATE_ALIVE
	, THREAD_STATE_DEAD
	, THREAD_STATE_SUSPEND
}
PLe_THREAD_STATE;

// Forward definition
struct  _PLs_THREAD;

// Type of functions thread body
//typedef		void  (* vTHREAD_BODY)(_PLs_THREAD*  par_s_pThreadDescriptor);
typedef		pid_t     PL_PID_ID  ;

typedef struct _PLs_THREAD 
{
	unsigned int		dwLabel;      // = PL_THREAD_LABEL
	PLb_RETURN     		bEndSignal;   // 0" or "1" - signal for FINISH thread. 
	PLe_THREAD_STATE	ThrState;
	PL_THREAD_HANDLE	handle;
	char			    name[PL_THREAD_NAME_SIZE];
	int					priority;
	size_t				stackSize;
	void          *		func;         // TYPE: vTHREAD_BODY
	void*				data;

	PL_PID_ID			dwMyOwn_PID;
	PL_PID_ID			dwParent_PID;

	THREAD_TYPE         eThrType    ; 
} 
PLs_THREAD;

typedef		void (* vTHREAD_BODY)(PLs_THREAD *  par_s_pThreadDescriptor);

PLs_THREAD	*			plcmThreadConstructDetail	(vTHREAD_BODY		par_fpThreadBody
													 ,void			*   par_pThreadData
													 ,const char*		par_pszName
													 ,int				par_nPriority
													 ,size_t				par_StackSize);

PLb_RETURN				plcmThreadInitDetail		(vTHREAD_BODY		par_fpThreadBody
													 ,void			*	par_pThreadData
													 ,const char		*	par_pszName
													 ,int				par_nPriority
													 ,size_t				par_StackSize
													 ,PLs_THREAD	*	par_pThr);

PLb_RETURN				plcmThreadDeinit			(PLs_THREAD	*   par_pThr);
PLb_RETURN				plcmThreadDestruct			(PLs_THREAD	*   par_pThr);
const char*				plcmThreadGetName			(PLs_THREAD	*   par_pThr);
PLb_RETURN				plcmThreadStart				(PLs_THREAD	*   par_pThr);
PLs_THREAD*				plcmThreadGetCurrent		(void);
void					plcmSleepMs					(PL_INTERVAL_ms	par_ms);
PLb_RETURN				plcmTreadSendFinishSignal   (PLs_THREAD	*   par_pThr);
PLb_RETURN				plcmThreadWaitTillEnd       (PLs_THREAD	*   par_pThr);
PLb_RETURN				plcmThreadSuspend           (PLs_THREAD	*   par_pThr);
PLb_RETURN				plcmThreadResume            (PLs_THREAD	*   par_pThr);
PLb_RETURN    			plcmIsTreadFinished         (PLs_THREAD	*   par_pThr);
PLb_RETURN    			plcmIsThreadActive          (PLs_THREAD	*   par_pThr);
void					plcmPriorToBodyStart		(PLs_THREAD	*	par_pThr);
void					plcmAfterBodyFinished		(PLs_THREAD	*   par_pThr);
PLb_RETURN				plcmThreadKill				(PLs_THREAD	*   par_pThr);
PLb_RETURN    			plcmThreadJoin				(PLs_THREAD	*	par_pThr);

PLs_THREAD*				plcmThreadConstruct			(vTHREAD_BODY		par_fpThreadBody
													 ,void			*   par_pThreadData
													 ,const char		*   par_pszName);

PLb_RETURN				plcmThreadInit				(vTHREAD_BODY		par_fpThreadBody
													 ,void			*   par_pThreadData
													 ,const char		*   par_pszName
													 ,PLs_THREAD	    *   par_pThr);

PLb_RETURN             plcmCreateAndStartThread     (vTHREAD_BODY       par_fpThreadBody
													 ,void	        *   par_pThreadData
													 ,const char		*   par_pszName
													 ,PLs_THREAD	    *   par_pThr);

PLb_RETURN             plcmDeinitAndFreeThread     (PLs_THREAD	    *   par_pThr);

PLb_RETURN             plcmThreadStart_detach         (PLs_THREAD   *   par_pThr);

PLb_RETURN             plcmCreateAndStartThread_detach(vTHREAD_BODY     par_fpThreadBody
													   ,void	    *   par_pThreadData
													   ,const char *   par_pszName
													   ,PLs_THREAD *   par_pThr);

//------- Create & start THREAD --------------------------//
//  1. plcmThreadInit  (..
//  2. plcmThreadStart (..
//
//------- Finish & De-Init & free THREAD -----------------//
//  1. plcmThreadWaitTillEnd (..
//  2. plcmThreadDeinit      (..















//-------------------------------------------//
//-------  MUTEX  ---------------------------//
//-------------------------------------------//

#define  PL_IF_MUTEX_OK(x)    if (-1 != (x))
#define  PL_IF_MUTEX_FAIL(x)  if (-1 == (x))


//    Codes returned by functions manipulating with (mainly - waiting for)
// OS-dependent objects, implemented in OS-independent fashion
// - THREADs, SEMAPHOREs, MUTEXes, EVENTs and so on
typedef enum _PLe_EVENT_WAIT
{
	GOT_OWNERSHIP            // The exact meaning depends on object nature
	// - e.g. for EVENTs it means "EVENT has been received"
	,DIDNT_GOT_TIMEOUTED     // after time-out expiration
	,DIDNT_GOT_OWNERSHIP     // As a rule, this code may only be returned by waiting functions
	// after time-out expiration
	,PARAMETER_ERROR         // Something wrong with request parameters
	,OBJECT_IS_CLOSED        // The object was de-initialized while it was awaited
	,INTERRUPTED_SYSSTEM_CALL// For Linux 
	,COUNTER_OVERFLOW        // The mutex could not be acquired because the maximum number of recursive locks for mutex has been exceeded.
}
PLe_OBJECT_WAIT;


//////================================================//////
///   Global TYPE declarations
//////================================================//////

// Type for mutex referencing

typedef struct 
{
	pthread_mutex_t   handle;
} 
PL_MUTEX;

//-------------------------------------------//
//-------  MUTEX  ---------------------------//
//-------------------------------------------//

// Initiating statically allocated mutex
PLb_RETURN        plcmMutexInit     (PL_MUTEX* par_pMut );
// De-Initiating statically allocated mutex (recently initialized by "plcmMutexInit")
PLb_RETURN        plcmMutexDeinit   (PL_MUTEX* m);

// Take Mutex ownership unconditionally - with INDEFINITE waiting 
PLe_OBJECT_WAIT   plcmMutexLock     (PL_MUTEX* par_pMut);
// Conditional attempt to take Mutex ownership - approach to nonblocking mode
PLe_OBJECT_WAIT   plcmMutexLockTry  (PL_MUTEX* par_pMut);
// Release Mutex ownership
PLb_RETURN        plcmMutexUnLock   (PL_MUTEX* par_pMut);

PLe_OBJECT_WAIT   plcmMutexLockTimed(PL_MUTEX* par_pMutex, int par_MilliSec);


typedef PLb_RETURN			(*F_MUTEX_INIT)        (void * _pMutex);
typedef PLb_RETURN			(*F_MUTEX_DEINT)       (void * _pMutex);
typedef PLe_OBJECT_WAIT     (*F_MUTEX_LOCK )       (void * _pMutex);
typedef PLb_RETURN			(*F_MUTEX_UNLOCK)      (void * _pMutex);
typedef PLe_OBJECT_WAIT	    (*F_MUTEX_TRY_LOCK)    (void * _pMutex);
typedef PLe_OBJECT_WAIT	    (*F_MUTEX_LOCK_TIMED)  (void * _pMutex, int _par_MilliSec);

typedef struct _PLc_MUTEX
{
	PL_MUTEX			_Mutex    ;
	F_MUTEX_INIT		fInit     ;
	F_MUTEX_DEINT		fDeInit   ;
	F_MUTEX_LOCK		fLock     ;
	F_MUTEX_UNLOCK		fUnLock   ;
	F_MUTEX_TRY_LOCK	fTryLock  ;
	F_MUTEX_LOCK_TIMED  fLockTimed;
}
PLc_MUTEX;

void  MutexObjInit(PLc_MUTEX * par_pMutex); 

#define M_LOCK(x1)          (x1).fLock(&(x1))
#define M_UnLOCK(x1)        (x1).fUnLock(&(x1))
#define M_LOCKTRY(x1)       (x1).fTryLock(&(x1))
#define M_DEINIT(x1)        (x1).fDeInit(&(x1))
#define M_LOCKTIMED(x1, x2) (x1).fLockTimed(&(x1), x2);

//======= Example ============================//
//PLc_MUTEX            ObjMutex;
//PLe_OBJECT_WAIT      wRc = GOT_OWNERSHIP; 
//    MutexObjInit(&ObjMutex);
//    M_LOCK(ObjMutex);
//    wRc = M_LOCKTRY(ObjMutex);
//    M_UnLOCK(ObjMutex);
//    wRc = M_LOCKTRY(ObjMutex);
//    M_DEINIT(ObjMutex);
//    M_LOCKTIMED(ObjMutex, 123);
//===========================================//



class PLc_THREAD : public CPObject
{
	 CLASS_TYPE_1(PLc_THREAD, CPObject)
public:
	virtual const char*          NameOf() const {return "PLc_THREAD";}

	PLc_THREAD(char * szThrName);

	virtual ~PLc_THREAD(void);

	bool  bThreadOk()
	{
		return   (   (plSUCCESS    ==  this->bThreadInitResult)
			&& (THREAD_STATE_ALIVE == (this->_Thread).ThrState)
			);
	}

	//Set thread name.
	virtual void   SetName(char *);
	//Get thread name.
	virtual char * GetName(void);
	//  Create a new thread that runs Svs() function and has
	//  default priority and stack size.
	virtual bool ActivateTach  (void);
	virtual bool ActivateDeTach(void);

	virtual bool SendFinish(void);	

	// Kill the thread.
	// NOTE: This method should be invoked by another thread.
	virtual bool Kill(void);

	// Block the caller untill the thread exists
	virtual bool Wait(void);

	// Block the caller for par_Ms milliseconds
	virtual void Sleep(unsigned int par_Ms);

	//Status thread request
	virtual bool IsActive  ();

	// Wait of FINISH signal
	virtual bool IsFinished();
	// User thread function
	virtual void Svc(void) {};

	virtual PLe_OBJECT_WAIT EndSignalClose();

protected:
	// Helper function used to provide mechanism for invoking the virtual
	// Svc() method as static method.
	static void  ThreadFuncWrap(_PLs_THREAD *  par_ThreadPtr);

	PLs_THREAD  _Thread; // concreate underlayed object implements thread functionality

	PLb_RETURN  bThreadInitResult;

private:
	// Disallow these operations
	PLc_THREAD(const PLc_THREAD&);
	void operator=(const PLc_THREAD&);
};




//-------------------------------------------//
//-------  QUEUE  ---------------------------//
//-------------------------------------------//

typedef struct _Q_DATA
{
	unsigned int    d_dwDataId  ;
	unsigned int    d_dwDataSize;
	void         *  d_pData     ;
}
Q_DATA;

typedef struct _Q_Node
{
	Q_DATA            sData               ;    
	struct _Q_Node  * pNext               ;
	struct _Q_Node  * pPrev               ;
}
Q_Node;

typedef struct _PL_QUEUE
{
	PLc_MUTEX         mq_Mutex            ;
	int               mq_QuAvalible       ; 
	unsigned int      mq_dwNumberOfNodes  ;
	struct _Q_Node  * mq_pHead            ;
	struct _Q_Node  * mq_pTail            ;
}
PL_QUEUE;

PLb_RETURN    bQueueInit       (PL_QUEUE * par_pQueue);
PLb_RETURN    bQueueClear      (PL_QUEUE * par_pQueue);
PLb_RETURN    bQueueDeinit     (PL_QUEUE * par_pQueue);
unsigned int  nQueueGetSize    (PL_QUEUE * par_pQueue);

PLb_RETURN    bQueuePut        (PL_QUEUE * par_pQueue, unsigned int  par_DataId , void * par_pData, unsigned int   par_DataSize);
PLb_RETURN    bQueueGet        (PL_QUEUE * par_pQueue, unsigned int *par_pDataId, void * par_pData, unsigned int * par_pDataSize);

PLb_RETURN    bQueueFirstNodeInfo(PL_QUEUE * par_pQueue, unsigned int * par_pDataId, void * par_pData, unsigned int * par_pDataSize);
unsigned int  nQueueFirstDataSize(PL_QUEUE * par_pQueue);

BOOL		  bIsItIpAddress	 (char * pHostName, int * par_pIpType);
void		  plcmPrintBuffHexStr(char * par_pBuff, int par_BuffLen, char * par_Txt, int par_nIsShortPrint);




//-E- PLCM_DNS ------------------------------//




#endif //PLCMDNS_TOOLS_H_

