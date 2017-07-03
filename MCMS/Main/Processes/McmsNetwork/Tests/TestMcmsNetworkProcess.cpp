// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestMcmsNetworkProcess.h"
#include "McmsNetworkProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestMcmsNetworkProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestMcmsNetworkProcess::setUp()
{
//	pDemoProcess = new CDemoProcess;
//	pDemoProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestMcmsNetworkProcess::tearDown()
{
//	SystemSleep(10);
//	pDemoProcess->TearDown();
//	delete pDemoProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestMcmsNetworkProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestMcmsNetworkProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestMcmsNetworkProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

