// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////


#include "TestMplApiProcess.h"
#include "MplApiProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "Segment.h"
#include "TaskApi.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestMplApiProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestMplApiProcess::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestMplApiProcess::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestMplApiProcess::testConstructor()
{

	FPTRACE(eLevelInfoNormal,"CTestMplApiProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestMplApiProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );  

} 

//////////////////////////////////////////////////////////////////////
void CTestMplApiProcess::testMPLAPI_MSG()
{
/*	CSegment * pSeg = new CSegment;
	*pSeg <<(WORD) 1;
	*pSeg <<(WORD) 2; 
	
	const COsQueue * MplApiDispQueue = 
		pMplApiProcess->GetOtherProcessQueue(eProcessMplApi,eDispatcher);
	
	CTaskApi api;
	api.CreateOnlyApi(*MplApiDispQueue);
	STATUS a = api.SendMsg(pSeg,MPLAPI_MSG);

	CPPUNIT_ASSERT_MESSAGE( "testMPLAPI_MSG ",
		a == STATUS_OK ); */
}



