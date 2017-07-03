// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestAuthenticationProcess.h"
#include "AuthenticationProcess.h"
#include "TraceStream.h"
#include "SystemFunctions.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestAuthenticationProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestAuthenticationProcess::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestAuthenticationProcess::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestAuthenticationProcess::testConstructor()
{
	FTRACESTR(eLevelInfoNormal) << "CTestAuthenticationProcess::testConstructor";

	CPPUNIT_ASSERT_MESSAGE( "CTestAuthenticationProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 


