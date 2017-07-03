// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestFailoverProcess.h"
#include "FailoverProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestFailoverProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestFailoverProcess::setUp()
{
//	pFailoverProcess = new CFailoverProcess;
//	pFailoverProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestFailoverProcess::tearDown()
{
//	SystemSleep(10);
//	pFailoverProcess->TearDown();
//	delete pFailoverProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestFailoverProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestFailoverProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestFailoverProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

