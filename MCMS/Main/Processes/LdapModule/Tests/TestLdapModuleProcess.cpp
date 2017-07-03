// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestLdapModuleProcess.h"
#include "LdapModuleProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestLdapModuleProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestLdapModuleProcess::setUp()
{
//	pLdapModuleProcess = new CLdapModuleProcess;
//	pLdapModuleProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestLdapModuleProcess::tearDown()
{
//	SystemSleep(10);
//	pLdapModuleProcess->TearDown();
//	delete pLdapModuleProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestLdapModuleProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestLdapModuleProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestLdapModuleProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

