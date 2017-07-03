// TestClientManager.cpp: implementation of the CTestClientManager class.
//
//////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <ostream>
#include "TestClientManager.h"
#include "TestServerOpcodes.h"
#include "TaskApi.h"
#include "TestClientProcess.h"
#include "Trace.h"
#include "ListenSocketApi.h"
#include "ClientSocket.h"
#include "SystemFunctions.h"
#include "Macros.h"
#include "TerminalCommand.h"

// CTestClientManager timers
#define PING_PONG 8001

extern "C" void MyRxTaskEntryPoint(void* appParam);
extern "C" void MyTxTaskEntryPoint(void* appParam);
extern "C" void singleToneEntryPoint(void* appParam);
extern "C" void testTaskEntryPoint(void* appParam);

extern void TestClientMonitorEntryPoint(void* appParam);

////////////////////////////////////////////////////////////////////////////
//               Task action table
////////////////////////////////////////////////////////////////////////////
PBEGIN_MESSAGE_MAP(CTestClientManager)
  ONEVENT(PING_PONG      ,ANYCASE    , CTestClientManager::OnTimerPingPong)
PEND_MESSAGE_MAP(CTestClientManager,CManagerTask);

////////////////////////////////////////////////////////////////////////////
//               Transaction Map
////////////////////////////////////////////////////////////////////////////

BEGIN_SET_TRANSACTION_FACTORY(CTestClientManager)
//ON_TRANS("TRANS_MCU","LOGIN",COperCfg,CTestClientManager::HandleOperLogin)
END_TRANSACTION_FACTORY

BEGIN_TERMINAL_COMMANDS(CTestClientManager)
    ONCOMMAND("ReadUnaloc",CTestClientManager::ReadUnallocatedMemory,"Test reading from unallocated memory")
    ONCOMMAND("WriteUnaloc",CTestClientManager::WriteUnallocatedMemory,"Test writting to unallocated memory")
    ONCOMMAND("ReadBefore",CTestClientManager::ReadBeforeMemory,"Test reading before a array")
    ONCOMMAND("WriteBefore",CTestClientManager::WriteBeforeMemory,"Test writting before a array")
    ONCOMMAND("ReadAfter",CTestClientManager::ReadAfterMemory,"Test reading after a array")
    ONCOMMAND("WriteAfter",CTestClientManager::WriteAfterMemory,"Test writting after a array")
    ONCOMMAND("DeallocTwice",CTestClientManager::DeallocateTwice,"Test writting after a array")
    ONCOMMAND("DeallocNonAlloc",CTestClientManager::DeallocateNonAllocated,"Test deallocating non allocated memory")
    ONCOMMAND("DeallocMiddle",CTestClientManager::DeallocateMiddle,"Test deallocating the middle of the array")
    ONCOMMAND("DeallocArrayAsObject",CTestClientManager::DeallocateArrayAsObject,"Test deallocating array without using []")
    ONCOMMAND("DeallocObjectAsArray",CTestClientManager::DeallocateObjectAsArray,"Test deallocating object using []")
    ONCOMMAND("AllocateZero",CTestClientManager::AllocateZero,"Test allocating zero size array")
    ONCOMMAND("ReadUninitMemoryHeap",CTestClientManager::ReadUninitMemoryHeap,"Test reading uninit memory from heap")
    ONCOMMAND("ReadUninitMemoryStack",CTestClientManager::ReadUninitMemoryStack,"Test reading uninit memory from stack")
    ONCOMMAND("BufferOverflow",CTestClientManager::BufferOverflow,"Test overflow a buffer on stack")
    ONCOMMAND("BufferUnderFlow",CTestClientManager::BufferUnderflow,"Test underflow a buffer on stack")
    ONCOMMAND("MemoryLeakNew",CTestClientManager::MemoryLeakNew,"Test memory leak using new")
    ONCOMMAND("MemoryLeakMalloc",CTestClientManager::MemoryLeakMalloc,"Test memory leak using malloc")
    ONCOMMAND("StackOverFlow",CTestClientManager::StackOverFlow,"Test stack overflow using infinite recurse");
    ONCOMMAND("Cyclic",CTestClientManager::CyclicMemory,"Test cyclic memory allocation");
    ONCOMMAND("CreateTasks",CTestClientManager::CreateTasks,"CreateTasks [n] -  Test tasks creation");
    ONCOMMAND("DestroyTasks",CTestClientManager::DestroyTasks,"DestroyTasks [n] -  Test tasks destruction");

END_TERMINAL_COMMANDS

////////////////////////////////////////////////////////////////////////////
//               Entry point
////////////////////////////////////////////////////////////////////////////
void TestClientManagerEntryPoint(void* appParam)
{
	CTestClientManager * pTestClientManager = new CTestClientManager;
	pTestClientManager->Create(*(CSegment*)appParam);
}

//////////////////////////////////////////////////////////////////////
TaskEntryPoint CTestClientManager::GetMonitorEntryPoint()
{
	return TestClientMonitorEntryPoint;
}


///////////////////////////////////////////////////////////////////////////
void  CTestClientManager::OnTimerPingPong(CSegment* pMsg)
{
	((CTestClientProcess*)CProcessBase::GetProcess())->m_ping_pong_count++;
	StartTimer(PING_PONG, 10);
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTestClientManager::CTestClientManager()
{

}

//////////////////////////////////////////////////////////////////////
CTestClientManager::~CTestClientManager()
{

}

//////////////////////////////////////////////////////////////////////
void CTestClientManager::ManagerPostInitActionsPoint()
{
	m_pListenSocketApi = new CListenSocketApi(MyRxTaskEntryPoint,
											  MyTxTaskEntryPoint,
											  9999);
	m_pListenSocketApi->Create(*m_pRcvMbx);


	m_pSingleToneApi = new CTaskApi;
	m_pSingleToneApi->Create(singleToneEntryPoint,*m_pRcvMbx);


	SystemSleep(10);


//	InitTimer(*m_pRcvMbx);
}

//////////////////////////////////////////////////////////////////////
void CTestClientManager::SelfKill()
{

	m_pListenSocketApi->Destroy();
	POBJDELETE(m_pListenSocketApi);

	m_pSingleToneApi->Destroy();
	POBJDELETE(m_pSingleToneApi);



	CManagerTask::SelfKill();
}

//////////////////////////////////////////////////////////////////////
void*  CTestClientManager::GetMessageMap()
{
	return (void*)m_msgEntries;
}


//////////////////////////////////////////////////////////////////////
BOOL CTestClientManager::TaskHandleEvent(CSegment *pMsg,
							   DWORD  msgLen,
							   OPCODE   opCode)
{
	switch(opCode)
	{
/*	case TEST_START_SENDING_PINGS:
		{
			PTRACE(eLevelInfoHigh,"PING...");
			const COsQueue * testServerManager =
				CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestServer,eManager);

			CTaskApi api;
			api.CreateOnlyApi(*testServerManager);
			api.SendSyncOpcodeMsg(TEST_SERVER_PING,100);
			break;
            }*/
	case TEST_SEND_STATE_MACHINE_MSG:
		{

			CTaskApi taskApi;
			taskApi.CreateOnlyApi(GetRcvMbx(),this);
			taskApi.SetLocalMbx(GetLocalQueue());

			CSegment* pMsg = new CSegment;
			taskApi.SendMsgWithStateMachine(pMsg, 4000);
//				}
//				else
//				{
//					//PASSERT(1);// should add handle event by opcode (-ignore specific state machine)
//				}
//			}
			break;
		}
	case TEST_SERVER_PING_RESP:
		{
			PTRACE(eLevelInfoHigh,"PING PONG.");
			break;
		}
	case TEST_LOCAL_MESSAGE:
		{
			//CTaskApi::StaticSendLocalOpcodeMsg(TEST_LOCAL_MESSAGE_CONT);
			  CTaskApi api;
			  api.CreateOnlyApi(GetRcvMbx(),this);
			  api.SetLocalMbx(GetLocalQueue());
			  api.SendLocalOpcodeMsg(TEST_LOCAL_MESSAGE_CONT);

			break;
		}
	case TEST_LOCAL_MESSAGE_CONT:
		{
			break;
		}
/*	case TEST_SENDING_10000_MESSAGES:
		{
			const COsQueue * testServerManager =
				CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestServer,eManager);

			CTaskApi api;
			api.CreateOnlyApi(*testServerManager);
			for (int i=1; i<10000; i++)
			{
				CSegment *testSeg = new CSegment();
				*testSeg << "TEST_ONE_IPC_MESSAGES";
				api.SendMsg(testSeg,TEST_ONE_IPC_MESSAGES);
			}

			break;
            }*/
	case TEST_10_TIMERS_PER_SECOND:
		{
			StartTimer(PING_PONG,10);
			break;
		}
	case TEST_REMOVE_THE_TIMER:
		{
			DeleteTimer(PING_PONG);
			break;
		}
	default:
		{
			return FALSE;
		}
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::ReadUnallocatedMemory(CTerminalCommand &command,std::ostream& answer)
{
    char *temp = new char;
    *temp = 0;
    delete temp;
    char value = *temp;
    value = *(temp+20000);
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::WriteUnallocatedMemory(CTerminalCommand &command,std::ostream& answer)
{
    char *temp = new char;
    *temp = 0;
    delete temp;
    *temp = 0;
    *(temp+20000) = 0;
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::ReadBeforeMemory(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    int a = array[-1];
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::WriteBeforeMemory(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    array[-1] = 0;
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::ReadAfterMemory(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    int a = array[100];
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::WriteAfterMemory(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    array[100] = 0;
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::DeallocateTwice(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    delete [] array;
    delete [] array;
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::DeallocateNonAllocated(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    delete [] (array+100);
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::DeallocateMiddle(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    delete [] (array+50);
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::DeallocateArrayAsObject(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    delete array;
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::DeallocateObjectAsArray(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int;
    delete [] array;
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::AllocateZero(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[0];
    delete []array;
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::ReadUninitMemoryHeap(CTerminalCommand &command,std::ostream& answer)
{
    int *array = new int[100];
    if (array[0] == 0)
        delete [] array;
    else
        delete [] array;

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::ReadUninitMemoryStack(CTerminalCommand &command,std::ostream& answer)
{
    int array[100];
    int x;
    if (array[0] == 0)
        x = 0;
    else
        x = 1;

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::BufferOverflow(CTerminalCommand &command,std::ostream& answer)
{
    int array[100];
    for (int i=1; i<=100;i++)
        array[i] = 0;
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::BufferUnderflow(CTerminalCommand &command,std::ostream& answer)
{
    int array[100];
    for (int i=0; i<100;i++)
        array[i-1] = 0;

    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::MemoryLeakNew(CTerminalCommand &command,std::ostream& answer)
{
    int* x = new int(100);
    return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::MemoryLeakMalloc(CTerminalCommand &command,std::ostream& answer)
{
    int* x = (int*) malloc(100);
    return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::StackOverFlow(CTerminalCommand &command,std::ostream& answer)
{
    int temp[1024*16];
    return StackOverFlow(command,answer);
}

//////////////////////////////////////////////////////////////////////
STATUS CTestClientManager::CyclicMemory(CTerminalCommand &command,std::ostream& answer)
{
    int *temp1 = new int[100];
    delete []temp1;
    int *temp2 = new int[100];
    if (temp1 == temp2)
    {
        answer << "This is not cyclic allocation";
    }
    else
    {
        answer << "This is cyclic allocation";
    }
    delete []temp2;

    return STATUS_OK;
}

STATUS CTestClientManager::CreateTasks(CTerminalCommand &command,std::ostream& answer)
{
  const string &number = command.GetToken(eCmdParam1);

  bool isNumeric = CObjString::IsNumeric(number.c_str());
  if (false == isNumeric)
  {
       answer << "error: Parameter must be numeric, not " << number.c_str()
              << '\n'
              << "help: CreateTasks [n] -  Test tasks creation";
       return STATUS_FAIL;
  }

  int number_of_tasks = atoi(number.c_str());

  if (number_of_tasks > 0)
  {
	  for (int i = 0; i < number_of_tasks; i++)
	  {
		cout << "creating...";
		CTaskApi * pTaskApi = new CTaskApi;
		CreateTask(pTaskApi, testTaskEntryPoint, m_pRcvMbx);
		m_tasks_list.push_back(pTaskApi);
		cout << "done\n";
	  }
  }
  return STATUS_OK;
}

STATUS CTestClientManager::DestroyTasks(CTerminalCommand &command,std::ostream& answer)
{
  const string &number = command.GetToken(eCmdParam1);

  bool isNumeric = CObjString::IsNumeric(number.c_str());
  if (false == isNumeric)
  {
       answer << "error: Parameter must be numeric, not " << number.c_str()
              << '\n'
              << "help: DestroyTasks [n] -  Test tasks destruction";
       return STATUS_FAIL;
  }

  int number_of_tasks = atoi(number.c_str());

  if (number_of_tasks > 0)
  {
	  for (int i = 0; i < number_of_tasks; i++)
	  {
		if (!m_tasks_list.empty())
		{
		  CTaskApi * api = *(m_tasks_list.begin());
		  cout << "destroy...";
		  //api->SyncDestroy("",TRUE);
		  api->Destroy();
		  m_tasks_list.remove(api);
		  delete api;
		  cout << "done\n";
		}
	  }
  }
  return STATUS_OK;
}


