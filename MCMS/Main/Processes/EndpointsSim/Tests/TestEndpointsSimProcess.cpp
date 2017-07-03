// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////



#include "TestEndpointsSimProcess.h"
#include "EndpointsSimProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestEndpointsSimProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestEndpointsSimProcess::setUp()
{
//	pEndpointsSimProcess = new CEndpointsSimProcess;
//	pEndpointsSimProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestEndpointsSimProcess::tearDown()
{
//	SystemSleep(10);
//	pEndpointsSimProcess->TearDown();
//	delete pEndpointsSimProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestEndpointsSimProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestEndpointsSimProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestEndpointsSimProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 



