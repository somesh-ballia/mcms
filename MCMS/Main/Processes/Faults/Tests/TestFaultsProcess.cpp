// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestFaultsProcess.h"
#include "FaultsProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestFaultsProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestFaultsProcess::setUp()
{
//	pFaultsProcess = new CFaultsProcess;
//	pFaultsProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestFaultsProcess::tearDown()
{
//	SystemSleep(10);
//	pFaultsProcess->TearDown();
//	delete pFaultsProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestFaultsProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestFaultsProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestFaultsProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 


