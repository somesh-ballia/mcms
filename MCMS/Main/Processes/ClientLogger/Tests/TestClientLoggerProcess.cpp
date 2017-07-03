// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestClientLoggerProcess.h"
#include "ClientLoggerProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

//CPPUNIT_TEST_SUITE_REGISTRATION( CTestClientLoggerProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestClientLoggerProcess::setUp()
{
//	pClientLoggerProcess = new CClientLoggerProcess;
//	pClientLoggerProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestClientLoggerProcess::tearDown()
{
//	SystemSleep(10);
//	pClientLoggerProcess->TearDown();
//	delete pClientLoggerProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestClientLoggerProcess::testConstructor()
{

    if (this == NULL)
        FPTRACE(eLevelInfoNormal,"CTestClientLoggerProcess::testConstructor");

    

/*	CPPUNIT_ASSERT_MESSAGE( "CTestClientLoggerProcess::testConstructor ",
		pClientLoggerProcess != NULL );  

	CPPUNIT_ASSERT_MESSAGE( "CTestClientLoggerProcess::testConstructor ",
		pClientLoggerProcess->m_pManagerApi != NULL );  

	CPPUNIT_ASSERT_MESSAGE( "CTestClientLoggerProcess::testConstructor ",
		pClientLoggerProcess->m_pTasks->size() > 5 ); 

	CPPUNIT_ASSERT_MESSAGE( "CTestClientLoggerProcess::testConstructor ",
		pClientLoggerProcess->m_selfKill == FALSE );	

	CPPUNIT_ASSERT_MESSAGE( "CTestClientLoggerProcess::testConstructor ",
    pClientLoggerProcess != NULL );  */
} 


