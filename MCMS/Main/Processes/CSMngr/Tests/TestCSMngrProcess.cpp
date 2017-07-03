// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////


#include "TestCSMngrProcess.h"
#include "CSMngrProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestCSMngrProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestCSMngrProcess::setUp()
{
//	pCSMngrProcess = new CCSMngrProcess;
//	pCSMngrProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestCSMngrProcess::tearDown()
{
//	SystemSleep(10);
//	pCSMngrProcess->TearDown();
//	delete pCSMngrProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestCSMngrProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestCSMngrProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestCSMngrProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 



