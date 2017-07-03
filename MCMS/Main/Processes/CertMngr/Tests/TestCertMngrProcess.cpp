// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestCertMngrProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"
//#include "CertMngrProcess.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestCertMngrProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestCertMngrProcess::setUp()
{
//	pCertMngrProcess = new CCertMngrProcess;
//	pCertMngrProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestCertMngrProcess::tearDown()
{
//	SystemSleep(10);
//	pCertMngrProcess->TearDown();
//	delete pCertMngrProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestCertMngrProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestCertMngrProcess::testConstructor");

//	CPPUNIT_ASSERT_MESSAGE( "CTestCertMngrProcess::testConstructor ",
//		CProcessBase::GetProcess() != NULL );
} 

