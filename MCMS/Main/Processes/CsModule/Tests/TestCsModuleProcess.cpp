// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestCsModuleProcess.h"
#include "CsModuleProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestCsModuleProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestCsModuleProcess::setUp()
{
//	pCsModuleProcess = new CCsModuleProcess;
//	pCsModuleProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestCsModuleProcess::tearDown()
{
//	SystemSleep(10);
//	pCsModuleProcess->TearDown();
//	delete pCsModuleProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestCsModuleProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestCsModuleProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestCsModuleProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

