// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////


#include "TestMcuCmdProcess.h"
#include "McuCmdProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestMcuCmdProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestMcuCmdProcess::setUp()
{
//	pMcuCmdProcess = new CMcuCmdProcess;
//	pMcuCmdProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestMcuCmdProcess::tearDown()
{
//	SystemSleep(10);
//	pMcuCmdProcess->TearDown();
//	delete pMcuCmdProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestMcuCmdProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestMcuCmdProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestMcuCmdProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 


