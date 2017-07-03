// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestMcmsDaemonProcess.h"
#include "McmsDaemonProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestMcmsDaemonProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestMcmsDaemonProcess::setUp()
{
//	pMcmsDaemonProcess = new CMcmsDaemonProcess;
//	pMcmsDaemonProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestMcmsDaemonProcess::tearDown()
{
//	SystemSleep(10);
//	pMcmsDaemonProcess->TearDown();
//	delete pMcmsDaemonProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestMcmsDaemonProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestMcmsDaemonProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestMcmsDaemonProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 


