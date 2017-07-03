//+========================================================================+
//                        ErrorHandlerTask.cpp                             |
//                     Copyright 2005 Polycom                              |
//                      All Rights Reserved.                               |
//-------------------------------------------------------------------------|
// NOTE: This software contains valuable trade secrets and proprietary     |
// information of Pictel Technologies Ltd. and is protected by law.        |
// It may not be copied or distributed in any form or medium, disclosed    |
// to third parties, reverse engineered or used in any manner without      |
// prior written authorization from Pictel Technologies Ltd.               |
//-------------------------------------------------------------------------|
// FILE:       ErrorHandlerTask.cpp                                        |
// SUBSYSTEM:  MCMS                                                        |
// PROGRAMMER: Dan                                                         |
//-------------------------------------------------------------------------|
// Who | Date       | Description                                          |
//-------------------------------------------------------------------------|
//+========================================================================+


#include <stdlib.h>
#include <signal.h>
#include "ErrorHandlerTask.h"
#include "ProcessBase.h"
#include "OsTask.h"
#include "TraceStream.h"
#include "Segment.h"
#include "TaskApi.h"
#include "FaultsDefines.h"
#include "HlogApi.h"
#include "SystemFunctions.h"
#include "OpcodesMcmsCommon.h"
#include "OpcodesMcmsInternal.h"
#include "ManagerApi.h"
#include "ObjString.h"
#include "SystemFunctions.h"

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CErrorHandlerTask)
    ONEVENT(HANDLE_EXCEPTION , ANYCASE , CErrorHandlerTask::OnException )
    ONEVENT(RESETMCU_REQUEST , ANYCASE , CErrorHandlerTask::SendResetReqToDaemon)
PEND_MESSAGE_MAP(CErrorHandlerTask,CAlarmableTask);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
CErrorHandlerTask::CErrorHandlerTask()
{}

//////////////////////////////////////////////////////////////////////
CErrorHandlerTask::~CErrorHandlerTask()
{ }

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void errorHandlerEntryPoint(void* appParam)
{
    int first_sig = SIGHUP; // 1
    int last_sig = SIGSYS;  // 31
    

   //  for (int i=first_sig ; i <= last_sig  ; i++) // loop on 31 linux signals
//     {
//         if (i!= SIGKILL && i!=SIGSTOP && i!=SIGCHLD) //SIGKILL and SIGSTOP can't be caught or handled
//             //SIGCHLD is caught by the parent process and therefore requires diffrent attendance
//         {
//             signal(i ,CErrorHandlerTask::SignalHandler);
//         }        
//     }

    signal(SIGINT , CErrorHandlerTask::SignalHandler);//SIGINT
    signal(SIGBUS , CErrorHandlerTask::SignalHandler);//Error on bus
    signal(SIGFPE , CErrorHandlerTask::SignalHandler);//Floating point exception (divide by zoro)
    signal(SIGPIPE, CErrorHandlerTask::SignalVoidHandler);//Error on Pipe
    signal(SIGSEGV, CErrorHandlerTask::SignalHandler); //Segmentation Fault
    signal(SIGILL , CErrorHandlerTask::SignalHandler);//Illegal Instruction
    signal(SIGXCPU, CErrorHandlerTask::SignalHandler);//CPU limit exceeded
    signal(SIGPWR , CErrorHandlerTask::SignalHandler);//Power Failure restart
    signal(SIGURG , CErrorHandlerTask::SignalHandler);//Urgent situation on Socket
    signal(SIGXFSZ, CErrorHandlerTask::SignalHandler);//File Size Exceeded
    signal(SIGALRM, CErrorHandlerTask::SignalHandler);//SIGALRM was sent
    signal(SIGHUP,  CErrorHandlerTask::SignalHandler);//signal to kill task

    //   signal(SIGABRT, CErrorHandlerTask::SignalHandler);//File Size Exceeded

    //SIGTERM not caught for needs of debugging , might change according to future needs.
    CErrorHandlerTask * errorHandlerTask = new CErrorHandlerTask;
	errorHandlerTask->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
void CErrorHandlerTask::InitTask()
{}

//////////////////////////////////////////////////////////////////////
void CErrorHandlerTask::SignalVoidHandler(int signal_number)
{ }
///////////////////////////////////////////////////////////////////

// Static
void CErrorHandlerTask::SignalHandler(int signal_number)
{
    if (signal_number == SIGPIPE)
    {
        return;
    }
    std::ostringstream cmd;

    if (signal_number == SIGHUP)
    {
    	COsTask::Exit();
    	return;
    }

    CTaskApp* pTaskApp = COsTask::GetTaskApp();
    CProcessBase * pProcess = CProcessBase::GetProcess();    
    if (pTaskApp == NULL)
    {
    	if(NULL!=pProcess && eProcessTearDown != pProcess->GetProcessStatus())
	{        
	  cmd <<" CErrorHandlerTask::SignalHandler eProcessTearDown != pProcess->GetProcessStatus(), process name  = " << 
	  pProcess->GetProcessName(pProcess->GetProcessType());
       	  PrintErrorToLocalFile(cmd);
	  pProcess->TearDown();
	}
        exit(signal_number);
        // this is the main thread - not a task app based thread
    }

    if (pTaskApp->m_selfKill)
    {
        // Exception occured while self killing
	DWORD taskId = COsTask::GetCurrentTaskId();
    	cmd <<"CErrorHandlerTask::SignalHandler Exception while self kill";
    	PrintErrorToLocalFile(cmd);;
        FPASSERTMSG(signal_number, "Exception while self kill");
        SystemCoreDump(TRUE);
        COsTask::Suspend();
    }

    if (signal_number == SIGALRM)
    {
        pTaskApp->SetSelfKill();
        return;
    }

    DWORD taskId = COsTask::GetCurrentTaskId();
    cmd << "CErrorHandlerTask::SignalHandler signal_number " << strsignal(signal_number)<< ", task: " << *pTaskApp;
    PrintErrorToLocalFile(cmd);

    signal(signal_number, CErrorHandlerTask::SignalHandler);	
    FPASSERTSTREAM(true,
        "Signal " << signal_number
            << " " << strsignal(signal_number)
            << ", task: " << *pTaskApp);

    signal(signal_number, CErrorHandlerTask::SignalHandler);

    CSegment *pSeg = new CSegment;
    *pSeg << (void*) pTaskApp << (DWORD) signal_number << taskId;

    SystemCoreDump(TRUE);

    pProcess->GetErrorHandlerApi()->SendMsg(pSeg, HANDLE_EXCEPTION);
    
    CManagerApi api(eProcessSystemMonitoring);
    api.SendMsg(NULL,NEW_CORE_DUMP_IND);
    pTaskApp->UnlockRelevantSemaphore();
    
    COsTask::Suspend();
}

//////////////////////////////////////////////////////////////////////
void CErrorHandlerTask::OnException(CSegment* pMsg)
{
    void *tmp;
    DWORD signal;
    DWORD taskId;
    *pMsg >> tmp >> signal >> taskId;

    PASSERTMSG(signal, "EXECPTION SIGNAL");
    CTaskApp* faultedTask = (CTaskApp*) tmp;

    if (FALSE == CPObject::IsValidPObjectPtr(faultedTask))
    {
    	TRACESTR(eLevelError)<< "CErrorHandlerTask::OnException - signal" << signal
                                 << "Task ID: " << taskId
                                 << "Invalid Task";
        KillWholeProcess();
        return;
    }
    
	CLargeString description;
    description << "Task failed "
                << "[" << faultedTask->GetAbsCrashCounter()
                << ":" << faultedTask->GetCrashCounter() << "] : "
                << faultedTask->GetTaskName();
    BOOL isFullOnly = FALSE;
	CHlogApi::TaskFault(FAULT_GENERAL_SUBJECT, 
						TASK_FAILED, 
						MAJOR_ERROR_LEVEL, 
						description.GetString(),
						isFullOnly);

    if (faultedTask->GetCrashCounter() < CTaskApp::GetMaximumTaskRecoveryRetries())
    {
    	// LEVEL 1 recovery
    	const eTaskRecoveryPolicy policy = faultedTask->GetTaskRecoveryPolicy();
    	if(eRecreateTaskAndThread == policy) // Option A
    	{
    		CloneTask(faultedTask);
    	}
    	else if(eRecreateThread == policy)   // Option B
    	{
    		ReviveTask(faultedTask);
    		CManagerApi api(eProcessConfParty);
    		CSegment* taskIdSeg = new CSegment;
    		DWORD newTaskId= faultedTask->GetTaskId();
    		// taskId = old task ID
    		*taskIdSeg << taskId;
    		*taskIdSeg << newTaskId;
    		api.SendMsg(taskIdSeg,CONF_UPDATED_FAILED_TASK);
    	}
    	else
    	{
    		PASSERTMSG(policy + 100, "Illegal policy type enumerator, policy + 100");
    	}
    }
    else
    {
    	// LEVEL 2 recovery	
        if (faultedTask->GetTaskRecoveryPolicyAfterSeveralRetries() == eTerminateProcess)
        {
            KillWholeProcess();                
        }
        else
        {
        	string taskName = faultedTask->GetTaskName();
        	string description = "Task terminated : ";
			description += taskName;

			TRACESTR(eLevelError)<< "CErrorHandlerTask::OnException - signal " << description;

			// Notify ConfPartyManager that task failed
			CManagerApi api(eProcessConfParty);
			CSegment* taskIdSeg = new CSegment;
			*taskIdSeg << taskId;
			api.SendMsg(taskIdSeg,CONF_FAILED_TASK);

			// Quality - remove active alarm when task terminated, in order to prevent the user reseting the RMX
			/*AddActiveAlarm(	FAULT_GENERAL_SUBJECT,
					TASK_TERMINATED,
					MAJOR_ERROR_LEVEL,
					description.c_str(),
					true,
					true
			);*/

			CProcessBase *process = CProcessBase::GetProcess();
			process->Cancel(faultedTask);

			if( faultedTask->GetTaskRecoveryPolicyAfterSeveralRetries() == eCreateNewTask)
	        {
	        	process->CreateTask( taskName.c_str() );
	        }

        }
    }   
}

void CErrorHandlerTask::SendResetReqToDaemon(CSegment* pMsg)
{
	
	// code copied from CMcuMngrManager::HandleMcuReset
/*
	const COsQueue* pDaemonMbx = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessMcmsDaemon, eManager);
    CManagerApi daemonManager(eProcessMcmsDaemon);
    daemonManager.Destroy();
*/  

	void *tmp;
    DWORD signal;
    DWORD taskId;
    *pMsg >> tmp >> signal >> taskId;
    PASSERTMSG(signal,"CErrorHandlerTask::SendResetReqToDaemon");    
    CTaskApp * faultedTask = (CTaskApp*) tmp;
      	
    CSmallString errorStr;
    errorStr << "Task " << faultedTask->GetTaskName() << " Failed";
    CTaskApi daemonManagerApi(eProcessMcmsDaemon,eManager);
	daemonManagerApi.Destroy(errorStr.GetString());
}

//////////////////////////////////////////////////////////////////////
void CErrorHandlerTask::HandleDeadChild()
{
    
}

////////////////////////////////////////////////////////////////////////
void CErrorHandlerTask::EmptyQueue(CTaskApp* TaskToFlushQueue)
{
    FTRACEINTO<<"Emptying queue"<<endl;
    TaskToFlushQueue->FlushQueue();
}

////////////////////////////////////////////////////////////////////
void CErrorHandlerTask::ReviveTask(CTaskApp * TaskToRevive)
{
    FTRACEINTO<<"CErrorHandlerTask started to Revive Task";
    
    TaskToRevive->CreateOsTask();
    TaskToRevive->IncrementCrashCounter();
}

////////////////////////////////////////////////////////////////////
void CErrorHandlerTask::CloneTask(CTaskApp * TaskToRevive)
{
    FTRACEINTO<<"CErrorHandlerTask started to Clone Task";
    
//    TaskToRevive->CloneTask();
    TaskToRevive->CreateOsTask();    
}

//////////////////////////////////////////////////////////////////////
void CErrorHandlerTask::KillWholeProcess()
{
    exit(0);
}

TICKS CErrorHandlerTask::GetFirstCrashTime(CTaskApp * pTask)
{
    return pTask->GetFirstCrashTime();
}

int  CErrorHandlerTask::GetNumberOfCrashes(CTaskApp * pTask)
{
    return pTask->GetNumOfCrashes();
    
}

void CErrorHandlerTask::ZeroCrashCounter(CTaskApp * pTask)
{
    pTask->ZeroCrashCounter();
}

void CErrorHandlerTask::ZeroTimeFromFirstCrash(CTaskApp * pTask)
{
    pTask->ZeroTimeFromFirstCrash();
}


