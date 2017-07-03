// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestQAAPIProcess.h"
#include "QAAPIProcess.h"
#include "TraceStream.h"
#include "SystemFunctions.h"
#include "Request.h"


//static CLoggerManager	*pLoggerManager = NULL;

CPPUNIT_TEST_SUITE_REGISTRATION( CTestQAAPIProcess );



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestQAAPIProcess::setUp()
{	
//	pLoggerManager = new CLoggerManager;
//	pLoggerManager->InitTask();
	
//	pLoggerProcess = new CLoggerProcess;
//	pLoggerProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestQAAPIProcess::tearDown()
{
//	pLoggerManager->TearDown();
//	delete pLoggerManager;
	
//	SystemSleep(10);
//	pLoggerProcess->TearDown();
//	delete pLoggerProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestQAAPIProcess::testConstructor()
{
	FTRACESTR(eLevelInfoNormal) << "CTestQAAPIProcess::testConstructor";

	CPPUNIT_ASSERT_MESSAGE( "CTestQAAPIProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 

//////////////////////////////////////////////////////////////////////
/*void CTestLoggerProcess::testPTRACE()
{
	FPTRACE(eLevelInfoHigh,           "CTestLoggerProcess::testPTRACE - TRACE");
	FPTRACE(eLevelInfoNormal,  	"CTestLoggerProcess::testPTRACE - DEBUG");
	FPTRACE(INTERACTIVE_TRACE,  "CTestLoggerProcess::testPTRACE - INTERACTIVE_TRACE");
	FPTRACE(eLevelError,    "CTestLoggerProcess::testPTRACE - FATAL");
	FPTRACE(eLevelInfoNormal,          "CTestLoggerProcess::testPTRACE - DEBUG");
	FPTRACE(INTERTASK_TRACE,    "CTestLoggerProcess::testPTRACE - INTERTASK_TRACE");
	FPTRACE(RESOURCE_ALOC_TRACE,"CTestLoggerProcess::testPTRACE - RESOURCE_ALOC_TRACE");
	FPTRACE(EMBEDDED_TRACE,     "CTestLoggerProcess::testPTRACE - EMBEDDED_TRACE");
	FPTRACE(eLevelInfoNormal,         "CTestLoggerProcess::testPTRACE - DEBUG");
}

//////////////////////////////////////////////////////////////////////
void CTestQAAPIProcess::testEmaTrace()
{
	FPTRACE(eLevelInfoNormal,"CTestQAAPIProcess::testEmaTrace");

	CEMATrace emaTrace;
	emaTrace.SetContent("Test Ema Trace Content");
	
	CRequest request;
	request.m_requestObject = &emaTrace;
	
	STATUS status = pLoggerManager->OnEmaTrace(&request);
	request.m_requestObject = NULL;

		
	STATUS status = STATUS_OK;
	CPPUNIT_ASSERT_MESSAGE( "CTestLoggerProcess::testEmaTrace : Not Implemented",
		status == STATUS_OK);
		
}*/











