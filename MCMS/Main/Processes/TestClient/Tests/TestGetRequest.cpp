// TestSharedMemory.cpp: implementation of the CTestSharedMemory class.
//
//////////////////////////////////////////////////////////////////////


#include "TestGetRequest.h"
#include "Request.h"
#include "DataTypes.h"
#include "OsQueue.h"
#include "Segment.h"
#include "ManagerApi.h"
#include "SystemFunctions.h"
#include "TestServerOpcodes.h"
#include "psosxml.h"
#include "TestClientProcess.h"
#include "Trace.h"
#include "OpcodesMcmsCommon.h"
#include "StatusesGeneral.h"
#include "PostXmlHeader.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestGetRequest );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestGetRequest::setUp()
{
}

//////////////////////////////////////////////////////////////////////
void CTestGetRequest::tearDown()
{
}


//////////////////////////////////////////////////////////////////////
void CTestGetRequest::testGetManagerTransectionsQueue()
{
	FPTRACE(eLevelInfoNormal,"testGetManagerTransectionsQueue");
	const COsQueue * managerTransectionsQueue =
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eMonitor);

	CPPUNIT_ASSERT_MESSAGE( "bad id",
		managerTransectionsQueue->m_id      != 0);

	CPPUNIT_ASSERT_MESSAGE( "bad id type",
		managerTransectionsQueue->m_idType  == eWriteHandle);

	CPPUNIT_ASSERT_MESSAGE( "bad process",
		managerTransectionsQueue->m_process == eProcessTestClient);

	CPPUNIT_ASSERT_MESSAGE( "bad scope",
		managerTransectionsQueue->m_scope   == eProcessTestClient);

}


//////////////////////////////////////////////////////////////////////
void CTestGetRequest::testSendMessage()
{
	FPTRACE(eLevelInfoNormal,"CTestGetRequest::testSendMessage");
	const COsQueue * managerTransectionsQueue =
		CProcessBase::GetProcess()->GetOtherProcessQueue(eProcessTestClient,eMonitor);

	CSegment *test = new CSegment;
	const char * xmlSample = "<TRANS_MCU><TRANS_COMMON_PARAMS><MCU_TOKEN>0</MCU_TOKEN><MCU_USER_TOKEN>0</MCU_USER_TOKEN><ASYNC><YOUR_TOKEN1>0</YOUR_TOKEN1><YOUR_TOKEN2>0</YOUR_TOKEN2></ASYNC><MESSAGE_ID>0</MESSAGE_ID></TRANS_COMMON_PARAMS><ACTION><LOGIN><MCU_IP><IP>172.22.188.103</IP><LISTEN_PORT>80</LISTEN_PORT></MCU_IP><USER_NAME>ACCORD</USER_NAME><HOST_NAME>localhost</HOST_NAME><PASSWORD>ACCORD</PASSWORD><STATION_NAME>F3-JUDITHS</STATION_NAME><COMPRESSION>true</COMPRESSION><CONFERENCE_RECORDER>false</CONFERENCE_RECORDER></LOGIN></ACTION></TRANS_MCU>";

	COsQueue stubDualMbx; // mailbox of the dual task (tx)
    stubDualMbx.Serialize(*test);

    CPostXmlHeader postXmlHeader((DWORD)strlen(xmlSample),
                                 (WORD)0, // authentication
                                 false,
                                 "UTF-8",
                                 "workstation-cucu",
                                 "user-cucu",
                                 "clientIp-cucu",
                                 (BYTE)false,
                                 "",
                                 "",
                                 "",
                                 0,0);

    postXmlHeader.Serialize(*test);
    test->Put((BYTE*)xmlSample, strlen(xmlSample));

 	CTaskApi api;
 	api.CreateOnlyApi(*managerTransectionsQueue);
 	STATUS res =  api.SendMsg(test,XML_REQUEST);
 	CPPUNIT_ASSERT_MESSAGE( "testConstructor ", res == STATUS_OK );
}


