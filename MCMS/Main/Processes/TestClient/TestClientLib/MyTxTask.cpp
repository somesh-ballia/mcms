// MyTxTask.cpp: implementation of the CMyTxTask class.
//
//////////////////////////////////////////////////////////////////////

#include "MyTxTask.h"
#include "Segment.h"
#include "Macros.h"

#define MY_TX_TIMER 1009


// message map
PBEGIN_MESSAGE_MAP(CMyTxTask)
  ONEVENT(MY_TX_TIMER      ,ANYCASE    , CMyTxTask::OnTxTimer)
PEND_MESSAGE_MAP(CMyTxTask,CStateMachine); 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
extern "C" void MyTxTaskEntryPoint(void* appParam)
{  	
	CMyTxTask*  pTaskApp = new CMyTxTask;
	pTaskApp->Create(*(CSegment*)appParam);
	*(CSegment*)appParam << (void*)pTaskApp ;
}


//////////////////////////////////////////////////////////////////////
CMyTxTask::CMyTxTask()
{

}

//////////////////////////////////////////////////////////////////////
CMyTxTask::~CMyTxTask()
{

}

//////////////////////////////////////////////////////////////////////
void CMyTxTask::InitTask()
{
//	InitTimer(*m_pRcvMbx);
	StartTimer(MY_TX_TIMER, 500);	
}

//////////////////////////////////////////////////////////////////////
void CMyTxTask::HandleDisconnect()
{
	CSocketTask::HandleDisconnect();
}


//////////////////////////////////////////////////////////////////////
void CMyTxTask::OnTxTimer(CSegment* pMsg)
{
	char buffer[] = {"Hello World\n"};
	Write(buffer,12);
	//PTRACE(eLevelInfoNormal,"CWatchDogTask::OnTimerWatchDog");
	StartTimer(MY_TX_TIMER, 500);
}

//////////////////////////////////////////////////////////////////////
void*  CMyTxTask::GetMessageMap()
{
	return (void*)m_msgEntries;    
}


/////////////////////////////////////////////////////////////////////////////
BOOL CMyTxTask::TaskHandleEvent(CSegment *pMsg,DWORD  msgLen,OPCODE opCode)
{
//	switch (opCode)
//	{
////		case 9999:
////		{
////			char buffer[] = {"*"};
////			Write(buffer,1);			
////			return TRUE;
////		}
//	}

	return CSocketTxTask::TaskHandleEvent(pMsg,msgLen,opCode);
}
