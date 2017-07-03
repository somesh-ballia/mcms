// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestLoggerProcess.h"
#include "LoggerProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "EMATrace.h"
#include "LoggerManager.h"
#include "Request.h"


//static CLoggerManager	*pLoggerManager = NULL;

CPPUNIT_TEST_SUITE_REGISTRATION( CTestLoggerProcess );



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestLoggerProcess::setUp()
{	
//	pLoggerManager = new CLoggerManager;
//	pLoggerManager->InitTask();
	
//	pLoggerProcess = new CLoggerProcess;
//	pLoggerProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestLoggerProcess::tearDown()
{
//	pLoggerManager->TearDown();
//	delete pLoggerManager;
	
//	SystemSleep(10);
//	pLoggerProcess->TearDown();
//	delete pLoggerProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestLoggerProcess::testConstructor()
{
	FPTRACE(eLevelInfoNormal,"CTestLoggerProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestLoggerProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 

//////////////////////////////////////////////////////////////////////
void CTestLoggerProcess::testPTRACE()
{
	FPTRACE(eLevelInfoHigh,           "CTestLoggerProcess::testPTRACE - TRACE");
	FPTRACE(eLevelInfoNormal,  	"CTestLoggerProcess::testPTRACE - DEBUG");
	FPTRACE(eLevelInfoHigh,       "CTestLoggerProcess::testPTRACE - INFO");
	FPTRACE(eLevelWarn,    		"CTestLoggerProcess::testPTRACE - WARN");
	FPTRACE(eLevelError,        "CTestLoggerProcess::testPTRACE - ERROR");
	FPTRACE(eLevelError,    	"CTestLoggerProcess::testPTRACE - FATAL");
}

//////////////////////////////////////////////////////////////////////
void CTestLoggerProcess::testEmaTrace()
{
	FPTRACE(eLevelInfoNormal,"CTestLoggerProcess::testEmaTrace");
/*
	CEMATrace emaTrace;
	emaTrace.SetContent("Test Ema Trace Content");
	
	CRequest request;
	request.m_requestObject = &emaTrace;
	
	STATUS status = pLoggerManager->OnEmaTrace(&request);
	request.m_requestObject = NULL;

		
	STATUS status = STATUS_OK;
	CPPUNIT_ASSERT_MESSAGE( "CTestLoggerProcess::testEmaTrace : Not Implemented",
		status == STATUS_OK);
*/			
}











