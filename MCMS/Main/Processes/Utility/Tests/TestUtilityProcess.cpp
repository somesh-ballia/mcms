// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestUtilityProcess.h"
#include "UtilityProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestUtilityProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestUtilityProcess::setUp()
{
//	pUtilityProcess = new CUtilityProcess;
//	pUtilityProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestUtilityProcess::tearDown()
{
//	SystemSleep(10);
//	pUtilityProcess->TearDown();
//	delete pUtilityProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestUtilityProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestUtilityProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestUtilityProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

