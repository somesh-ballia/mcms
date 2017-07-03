// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////



#include "TestCSApiProcess.h"
#include "CSApiProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestCSApiProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestCSApiProcess::setUp()
{
//	pCSApiProcess = new CCSApiProcess;
//	pCSApiProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestCSApiProcess::tearDown()
{
//	SystemSleep(10);
//	pCSApiProcess->TearDown();
//	delete pCSApiProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestCSApiProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestCSApiProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestCSApiProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 



