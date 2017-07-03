// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestSNMPProcessProcess.h"
#include "SNMPProcessProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestSNMPProcessProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestSNMPProcessProcess::setUp()
{
//	pSNMPProcessProcess = new CSNMPProcessProcess;
//	pSNMPProcessProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestSNMPProcessProcess::tearDown()
{
//	SystemSleep(10);
//	pSNMPProcessProcess->TearDown();
//	delete pSNMPProcessProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestSNMPProcessProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestSNMPProcessProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestSNMPProcessProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

