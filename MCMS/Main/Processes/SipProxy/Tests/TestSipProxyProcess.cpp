// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestSipProxyProcess.h"
#include "SipProxyProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"

#include "SIPProxyIpParameters.h"
#include "Segment.h"
#include "ManagerApi.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestSipProxyProcess );

CProcessBase* pSipProxyProcess = NULL;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestSipProxyProcess::setUp()
{
	pSipProxyProcess = CProcessBase::GetProcess();
}

//////////////////////////////////////////////////////////////////////
void CTestSipProxyProcess::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestSipProxyProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestSipProxyProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyProcess::testConstructor ",
		pSipProxyProcess != NULL );  

	/*CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyProcess::testConstructor ",
		pSipProxyProcess->m_pManagerApi != NULL );  

	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyProcess::testConstructor ",
		pSipProxyProcess->m_pTasks->size() > 5 ); 

	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyProcess::testConstructor ",
		pSipProxyProcess->m_selfKill == FALSE );	*/

	CPPUNIT_ASSERT_MESSAGE( "CTestSipProxyProcess::testConstructor ",
		pSipProxyProcess != NULL );  
} 

