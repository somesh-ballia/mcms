// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestSystemMonitoringProcess.h"
#include "SystemMonitoringProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestSystemMonitoringProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestSystemMonitoringProcess::setUp()
{
//	pSystemMonitoringProcess = new CSystemMonitoringProcess;
//	pSystemMonitoringProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestSystemMonitoringProcess::tearDown()
{
//	SystemSleep(10);
//	pSystemMonitoringProcess->TearDown();
//	delete pSystemMonitoringProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestSystemMonitoringProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestSystemMonitoringProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestSystemMonitoringProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

