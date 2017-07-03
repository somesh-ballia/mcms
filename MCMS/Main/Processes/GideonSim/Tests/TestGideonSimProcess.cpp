// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////



#include "TestGideonSimProcess.h"
#include "GideonSimProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestGideonSimProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestGideonSimProcess::setUp()
{
//	pGideonSimProcess = new CGideonSimProcess;
//	pGideonSimProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestGideonSimProcess::tearDown()
{
//	SystemSleep(10);
//	pGideonSimProcess->TearDown();
//	delete pGideonSimProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestGideonSimProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestGideonSimProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestGideonSimProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 



