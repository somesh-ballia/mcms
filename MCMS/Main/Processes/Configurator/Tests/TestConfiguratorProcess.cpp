// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestConfiguratorProcess.h"
#include "ConfiguratorProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "ConfigManagerApi.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestConfiguratorProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestConfiguratorProcess::setUp()
{
//	pConfiguratorProcess = new CConfiguratorProcess;
//	pConfiguratorProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestConfiguratorProcess::tearDown()
{
//	SystemSleep(10);
//	pConfiguratorProcess->TearDown();
//	delete pConfiguratorProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestConfiguratorProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestConfiguratorProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestConfiguratorProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
} 

//////////////////////////////////////////////////////////////////////
void CTestConfiguratorProcess::testAddIpInterface()
{
	CConfigManagerApi api;
//    STATUS stat = api.AddIpInterface(0x12345678,0x01020304,0xff00ff00);
    

//	CPPUNIT_ASSERT_MESSAGE( "CTestConfiguratorProcess::testConstructor ",
//		CProcessBase::GetProcess() != NULL );
} 

