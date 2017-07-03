// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////


#include "TestResourceProcess.h"
#include "ResourceProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestResourceProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestResourceProcess::setUp()
{
//	pResourceProcess = new CResourceProcess;
//	pResourceProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestResourceProcess::tearDown()
{
//	SystemSleep(10);
//	pResourceProcess->TearDown();
//	delete pResourceProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestResourceProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestResourceProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestResourceProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 


