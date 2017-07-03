// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestNotificationMngrProcess.h"
#include "NotificationMngrProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestNotificationMngrProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestNotificationMngrProcess::setUp()
{
//	pNotificationMngrProcess = new CNotificationMngrProcess;
//	pNotificationMngrProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestNotificationMngrProcess::tearDown()
{
//	SystemSleep(10);
//	pNotificationMngrProcess->TearDown();
//	delete pNotificationMngrProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestNotificationMngrProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestNotificationMngrProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestNotificationMngrProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

