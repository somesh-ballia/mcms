// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestRtmIsdnMngrProcess.h"
#include "RtmIsdnMngrProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestRtmIsdnMngrProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestRtmIsdnMngrProcess::setUp()
{
//	pRtmIsdnMngrProcess = new CRtmIsdnMngrProcess;
//	pRtmIsdnMngrProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestRtmIsdnMngrProcess::tearDown()
{
//	SystemSleep(10);
//	pRtmIsdnMngrProcess->TearDown();
//	delete pRtmIsdnMngrProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestRtmIsdnMngrProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestRtmIsdnMngrProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestRtmIsdnMngrProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

