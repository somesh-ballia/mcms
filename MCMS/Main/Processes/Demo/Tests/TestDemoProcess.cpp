// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestDemoProcess.h"
#include "DemoProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestDemoProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestDemoProcess::setUp()
{
//	pDemoProcess = new CDemoProcess;
//	pDemoProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestDemoProcess::tearDown()
{
//	SystemSleep(10);
//	pDemoProcess->TearDown();
//	delete pDemoProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestDemoProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestDemoProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestDemoProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

