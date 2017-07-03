// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestMediaMngrProcess.h"
#include "MediaMngrProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestMediaMngrProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestMediaMngrProcess::setUp()
{
//	pMediaMngrProcess = new CMediaMngrProcess;
//	pMediaMngrProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestMediaMngrProcess::tearDown()
{
//	SystemSleep(10);
//	pMediaMngrProcess->TearDown();
//	delete pMediaMngrProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestMediaMngrProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestMediaMngrProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestMediaMngrProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

