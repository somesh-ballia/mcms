// TestBackupRestoreProcess.cpp: 
//	implementation of the TestBackupRestoreProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestBackupRestoreProcess.h"
#include "BackupRestoreProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestBackupRestoreProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestBackupRestoreProcess::setUp()
{
//	pBackupRestoreProcess = new CBackupRestoreProcess;
//	pBackupRestoreProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestBackupRestoreProcess::tearDown()
{
//	SystemSleep(10);
//	pBackupRestoreProcess->TearDown();
//	delete pBackupRestoreProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestBackupRestoreProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestBackupRestoreProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestBackupRestoreProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

