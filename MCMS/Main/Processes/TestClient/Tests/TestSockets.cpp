// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////



#include "TestSockets.h"
#include "TestClientProcess.h"
#include "DataTypes.h"
#include "OsQueue.h"
#include "Segment.h"
#include "ManagerApi.h"
#include "SystemFunctions.h"
#include "TestServerOpcodes.h"
#include "Trace.h"
#include "ClientSocket.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestSockets );

extern "C" void MyRxTaskEntryPoint(void* appParam);
extern "C" void MyTxTaskEntryPoint(void* appParam);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestSockets::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestSockets::tearDown()
{
}


//////////////////////////////////////////////////////////////////////
void CTestSockets::testConstructor()
{
	FPTRACE(eLevelInfoNormal,"testConstructor");
	CPPUNIT_ASSERT_MESSAGE( "CTestSockets::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 

//////////////////////////////////////////////////////////////////////
void CTestSockets::testClientSocketConstructor()
{
//	SystemSleep(50);
//
//	CClientSocket clientSocket(CProcessBase::GetProcess()->GetManagerApi()->GetTaskAppPtr(),
//							   MyRxTaskEntryPoint,
//							   MyTxTaskEntryPoint);
//	
//
//	SystemSleep(100);
//

} 

