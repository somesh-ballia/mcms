// TestAuditorProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestAuditorProcess.h"
#include "AuditorProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestAuditorProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestAuditorProcess::setUp()
{
//	pDemoProcess = new CDemoProcess;
//	pDemoProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestAuditorProcess::tearDown()
{
//	SystemSleep(10);
//	pDemoProcess->TearDown();
//	delete pDemoProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestAuditorProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestAuditorProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestAuditorProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

