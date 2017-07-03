// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////



#include "TestTestClientProcess.h"
#include "TestClientProcess.h"
#include "DataTypes.h"
#include "OsQueue.h"
#include "Segment.h"
#include "ManagerApi.h"
#include "SystemFunctions.h"
#include "TestServerOpcodes.h"
#include "Trace.h"
#include "SingleToneApi.h"
#include "TraceStream.h"
#include "StatusesGeneral.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestTestClientProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::tearDown()
{
	
}


//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testConstructor()
{
	FTRACEINTO << "testConstructor";

	CPPUNIT_ASSERT_MESSAGE( "CTestTestClientProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testSendMessage()
{
	FPTRACE(eLevelInfoNormal,"testSendMessage");
	CSegment *test = new CSegment;
	*test << "Hello World";
	STATUS res =  CProcessBase::GetProcess()->GetManagerApi()->SendMsg(test,0xffff);
	CPPUNIT_ASSERT_MESSAGE( "testConstructor ", res == STATUS_OK );
}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testGetManagerQueue()
{
	FPTRACE(eLevelInfoNormal,"testGetManagerQueue");
	const COsQueue * demoManager = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eManager);

	CPPUNIT_ASSERT_MESSAGE( "bad id",
		demoManager->m_id      != 0);
	CPPUNIT_ASSERT_MESSAGE( "bad id type",
		demoManager->m_idType  == eWriteHandle);
	CPPUNIT_ASSERT_MESSAGE( "bad process",
		demoManager->m_process == eProcessTestClient);
	CPPUNIT_ASSERT_MESSAGE( "bad scope",
		demoManager->m_scope   == eProcessTestClient);	
}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testGetDispacherQueue()
{
	FPTRACE(eLevelInfoNormal,"testGetDispacherQueue");
	SystemSleep(10,FALSE);
	const COsQueue * demoDispacher = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eDispatcher);

	CPPUNIT_ASSERT_MESSAGE( "bad id",
		demoDispacher->m_id      != 0);

	CPPUNIT_ASSERT_MESSAGE( "bad id type",
		demoDispacher->m_idType  == eWriteHandle);

	CPPUNIT_ASSERT_MESSAGE( "bad process",
		demoDispacher->m_process == eProcessTestClient);

	CPPUNIT_ASSERT_MESSAGE( "bad scope",
		demoDispacher->m_scope   == eProcessTestClient);

}

//////////////////////////////////////////////////////////////////////
//void CTestTestClientProcess::testKillDispacher()
//{
//	int task_count_before = CProcessBase::GetProcess()->m_pTasks->length();
//
//	const COsQueue * demoDispacher = 
//		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eDispatcher);
//	CTaskApi dispacherApi;
//	dispacherApi.CreateOnlyApi(*demoDispacher);
//	dispacherApi.Destroy();
////	dispacherApi.SendOpcodeMsg(DESTROY);
//	SystemSleep(20);
//	int task_count_after = CProcessBase::GetProcess()->m_pTasks->length();
//
//	CPPUNIT_ASSERT_MESSAGE( "tasks array wasn't changed",
//		task_count_after == task_count_before - 1);	
//
//}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testSendSyncMessages()
{
	FPTRACE(eLevelInfoNormal,"testSendSyncMessages");
	DWORD syncBefore = CProcessBase::GetProcess()->m_numSyncMsgSent;
	DWORD toutBefore = CProcessBase::GetProcess()->m_numToutSyncMsg;
	const COsQueue * demoManager = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*demoManager);
	api.SendOpcodeMsg(TEST_START_SENDING_PINGS);
	SystemSleep(10,FALSE);
	
	CPPUNIT_ASSERT_MESSAGE( "failed sending sync message",
		CProcessBase::GetProcess()->m_numSyncMsgSent > syncBefore);

	CPPUNIT_ASSERT_MESSAGE( "sync message time out",
		CProcessBase::GetProcess()->m_numToutSyncMsg == toutBefore);
	
}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testSendLocalMessages()
{
	FTRACEINTO << "testSendLocalMessages";
	DWORD localMsgSendBefore = CProcessBase::GetProcess()->m_numLocalMsgSent;
	DWORD localMsgRecvBefore = CProcessBase::GetProcess()->m_numLocalMsgRvc;
	const COsQueue * demoManager = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*demoManager);
	api.SendOpcodeMsg(TEST_LOCAL_MESSAGE);
	SystemSleep(10,FALSE);
	
	CPPUNIT_ASSERT_MESSAGE( "failed sending sync message",
		CProcessBase::GetProcess()->m_numLocalMsgSent > localMsgSendBefore);

	CPPUNIT_ASSERT_MESSAGE( "sync message time out",
		CProcessBase::GetProcess()->m_numLocalMsgRvc  > localMsgRecvBefore);

}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::test1000IPCMessage()
{
	FTRACEINTO << "test1000IPCMessage";
	
	DWORD before = CProcessBase::GetProcess()->m_numMessageSent;
	
	const COsQueue * demoManager = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*demoManager);
	api.SendOpcodeMsg(TEST_SENDING_10000_MESSAGES);
	SystemSleep(100,FALSE);
	
	CPPUNIT_ASSERT_MESSAGE( "failed sending sync message",
		CProcessBase::GetProcess()->m_numMessageSent > before + 9999);

}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testAddTimer()
{
	FPTRACE(eLevelInfoNormal,"testAddTimer");
	const COsQueue * clientManager = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*clientManager);
	api.SendOpcodeMsg(TEST_10_TIMERS_PER_SECOND);
	SystemSleep(100,FALSE);
/*	CPPUNIT_ASSERT_MESSAGE( "not enough ping pong was sent in 1 seconds",
		CProcessBase::GetProcess()->m_ping_pong_count > 6);
	CPPUNIT_ASSERT_MESSAGE( "too many ping pong was sent in 1 seconds",
    CProcessBase::GetProcess()->m_ping_pong_count < 13);*/

}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testRemoveTimer()
{
	FPTRACE(eLevelInfoNormal,"testRemoveTimer");
	const COsQueue * clientManager = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*clientManager);
	api.SendOpcodeMsg(TEST_10_TIMERS_PER_SECOND);
	api.SendOpcodeMsg(TEST_REMOVE_THE_TIMER);
	SystemSleep(10,FALSE);
//	CPPUNIT_ASSERT_MESSAGE( "timer was not removed",
//		CProcessBase::GetProcess()->m_ping_pong_count == 0);
}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testPTRACE()
{
	FPTRACE(eLevelInfoNormal,"testPTRACE");
	FPTRACE(eLevelInfoHigh,           "CTestTestClientProcess::testPTRACE - TRACE");
//	FPTRACE(INTERACTIVE_TRACE,  "CTestTestClientProcess::testPTRACE - INTERACTIVE_TRACE ");
	FPTRACE(eLevelError,    "CTestTestClientProcess::testPTRACE - FATAL");
//	FPTRACE(eLevelInfoNormal,          "CTestTestClientProcess::testPTRACE - DEBUG");
	FPTRACE(eLevelInfoNormal,    "CTestTestClientProcess::testPTRACE - DEBUG");
//	FPTRACE(RESOURCE_ALOC_TRACE,"CTestTestClientProcess::testPTRACE - RESOURCE_ALOC_TRACE");
//	FPTRACE(EMBEDDED_TRACE,     "CTestTestClientProcess::testPTRACE - EMBEDDED_TRACE");
	FPTRACE(eLevelInfoNormal,         "CTestTestClientProcess::testPTRACE - DEBUG");

	FTRACEINTO << "Hello World\n";

}

void CTestTestClientProcess::testPASSERT()
{
	FPTRACE(eLevelInfoNormal,"testPASSERT");
	FPASSERT(0);
	FPASSERT(1);
	FPASSERTMSG(100,"Testing Assert with message");
}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testDividByZero()
{
	FPTRACE(eLevelInfoNormal,"testDividByZero");
//	CProcessBase::GetProcess()->m_pManagerApi->SendOpcodeMsg(TEST_EXCEPTION_DIVID_BY_ZERO);
}


//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testTerminateServer()
{
/*	PTRACE(eLevelInfoNormal,"testTerminateServer");
	const COsQueue * serverMaintanence = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestServer,eManager);
	
	CTaskApi api;
	api.CreateOnlyApi(*serverMaintanence);
	api.SendOpcodeMsg(DESTROY);*/

}


//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testSendStateMachineMessage()
{
	const COsQueue * managerQueue = 
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eManager);

	CTaskApi api;
	api.CreateOnlyApi(*managerQueue);
	api.SendOpcodeMsg(TEST_SEND_STATE_MACHINE_MSG);
	SystemSleep(10,FALSE);
	
}


void CTestTestClientProcess::testCreateSingleToneTask()
{
//	CSingleToneApi api(eProcessTestClient,"SingleToneTask");
//	api.Destroy();
}

//////////////////////////////////////////////////////////////////////
void CTestTestClientProcess::testTRACEINTO()
{
	FTRACEINTO << "testTRACEINTO\n";
    CTaskApi api;
    FTRACEINTO << "testTRACEINTO : " << api << "\n";
    
}

