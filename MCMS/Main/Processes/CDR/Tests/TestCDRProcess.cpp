// TestTestProcess.cpp: implementation of the TestTestProcess class.
//
//////////////////////////////////////////////////////////////////////

#include "TestCDRProcess.h"
#include "CDRProcess.h"
#include "Trace.h"
#include "SystemFunctions.h"
#include "CDRShort.h"
#include "CDRDetal.h"
#include "CDRLogApi.h"
#include "ConfStart.h"
#include "CDRDefines.h"
#include "ConfPartySharedDefines.h"
#include "H221.h"
#include "OperEvent.h"

#include "CdrRegistrationApiDefines.h"
//#include "RegistrationRequest.h"
//#include "RegistrationResponse.h"
#include "CdrPersistApiEnums.h"
//#include "EventRequest.h"
//#include "PlcmCdrEventConfEnd.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestCDRProcess );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
void CTestCDRProcess::setUp()
{
//	pCDRProcess = new CCDRProcess;
//	pCDRProcess->SetUp();
//	SystemSleep(10);
}

//////////////////////////////////////////////////////////////////////
void CTestCDRProcess::tearDown()
{
//	SystemSleep(10);
//	pCDRProcess->TearDown();
//	delete pCDRProcess;
}

//////////////////////////////////////////////////////////////////////
void CTestCDRProcess::testConstructor()
{
	FPTRACE(eLevelInfoNormal,"CTestCDRProcess::testConstructor");

	CPPUNIT_ASSERT_MESSAGE( "CTestCDRProcess::testConstructor ",
		CProcessBase::GetProcess() != NULL );
}


////////////////////////////////////////////////////////////////////////
void CTestCDRProcess::testCdrEvent()
{
//	ApiBaseObjectPtr a = new PlcmCdrEventConfEnd;
//	((PlcmCdrEventConfEnd*)a)->m_confId = "xxxxxx";
//	((PlcmCdrEventConfEnd*)a)->WriteToXmlFile("event_request.xml", true);
//	 p.m_confId = "xxxxx";
//	 p.m_scheduled = false;
//	 p.m_conferenceType = eMultipoint;
//	 p.m_bridgeConference = false;
//	 p.m_confEndCause = eEnd_time_terminate;
//	 p.WriteToXmlFile("event_request.xml", true);
//		  PlcmCdrEventConfEnd p_;
//	 p_.ReadFromXmlFile("event_request.xml");
//	 TRACESTR(eLevelInfoNormal) << "CCDRManager::CCDRManager, confId=" << p_.m_confId;

//	PlcmCdrEventConfEnd p;
//	  p.m_confId = "xxxxx";
//	  p.m_scheduled = false;
//	  p.m_conferenceType = eMultipoint;
//	  p.m_bridgeConference = false;
//	  p.m_confEndCause = eEnd_time_terminate;
//	  p.WriteToXmlFile("event_request.xml", true);
//
//	  PlcmCdrEventConfEnd p_;
//	  p_.ReadFromXmlFile("event_request.xml");
//	  TRACESTR(eLevelInfoNormal) << "CCDRManager::CCDRManager, confId=" << p_.m_confId;
}

//////////////////////////////////////////////////////////////////////
void CTestCDRProcess::testInitConf()
{
	FPTRACE(eLevelInfoNormal,"CTestCDRProcess::testInitConf");

	CCdrShort* cdrShort = new CCdrShort;
  	cdrShort->SetH243ConfName("cucu lulu");
  	cdrShort->SetConfId(42);

  	CStructTm time;
  	SystemGetTime(time);
  	cdrShort->SetRsrvStrtTime(time);

	CStructTm duration(0,0,0,2,0,0);
  	cdrShort->SetRsrvDuration(duration);

  	cdrShort->SetActualStrtTime(time);
  	cdrShort->SetActualDuration(duration);
  	cdrShort->SetStatus(ONGOING_CONFERENCE);

	CCdrLogApi cdrApi;
	STATUS status = cdrApi.StartConference(*cdrShort);

	CPPUNIT_ASSERT_MESSAGE( "init conf", STATUS_OK == status );
}

//////////////////////////////////////////////////////////////////////
void CTestCDRProcess::testStartConf()
{
	FPTRACE(eLevelInfoNormal,"CTestCDRProcess::testStartConf");

	CConfStart confStart;
	confStart.SetStandBy(NO);
  	confStart.SetAutoTerminate(NO);
  	confStart.SetConfTransfRate(Xfer_64);
  	confStart.SetRestrictMode(AUTO);
  	confStart.SetAudioRate(U_Law_OF);
  	confStart.SetVideoSession(VIDEO_SWITCH);
  	confStart.SetVideoPicFormat(V_Qcif);
  	confStart.SetCifFrameRate(V_4_29_97);
  	confStart.SetQcifFrameRate(V_4_29_97);
  	confStart.SetLsdRate(AUTO);
  	confStart.SetHsdRate(AUTO);
  	confStart.SetT120BitRate(0);

  	CCdrEvent cdrEvent;
  	cdrEvent.SetCdrEventType(CONFERENCE_START);

  	CStructTm time;
  	SystemGetTime(time);
  	cdrEvent.SetTimeStamp(time);
  	cdrEvent.SetConferenceStart(&confStart);

  	CCdrLogApi cdrApi;
  	STATUS status = cdrApi.ConferenceEvent(42, cdrEvent);

  	CPPUNIT_ASSERT_MESSAGE( "start conf", STATUS_OK == status );
}

//////////////////////////////////////////////////////////////////////
void CTestCDRProcess::testEventConf()
{
	FPTRACE(eLevelInfoNormal,"CTestCDRProcess::testEventConf");

	CCDRPartyRecording partyRecording;

	partyRecording.SetPartyName("Party Name: Cucu lulu");
	partyRecording.SetOpType(eStartRecording);
	partyRecording.SetPartyId(42);
	partyRecording.SetOpInitiator(0);
	partyRecording.SetRecordingLinkName("Recording link name: Cucu Lulu");
	partyRecording.SetRecordingLinkId(42);
	partyRecording.SetStartRecPolicy(START_RECORDING_IMMEDIATELY);

	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(RECORDING_LINK_EVENT);

	CStructTm time;
  	SystemGetTime(time);
	cdrEvent.SetTimeStamp(time);
	cdrEvent.SetRecording(&partyRecording);

	CCdrLogApi cdrApi;
  	STATUS status = cdrApi.ConferenceEvent(42, cdrEvent);

	CPPUNIT_ASSERT_MESSAGE( "start conf", STATUS_OK == status );
}

//////////////////////////////////////////////////////////////////////
void CTestCDRProcess::testEndConf()
{
	FPTRACE(eLevelInfoNormal,"CTestCDRProcess::testEndConf");

	BYTE cause = 0;
	CStructTm actualStartTime, actualDuration, curTime;
	SystemGetTime(actualStartTime);  	// does not correct, must be the start time
	SystemGetTime(actualDuration);  	// does not correct, must be the start time
	SystemGetTime(curTime);

	CCdrEvent cdrEvent;
	cdrEvent.SetCdrEventType(CONFERENCES_END);
	cdrEvent.SetTimeStamp(curTime);
	cdrEvent.SetConfEndCause(cause);

	CCdrLogApi cdrApi;
  	STATUS status = cdrApi.EndConference(42, TERMINATE_BY_OPERATOR, actualStartTime, actualDuration, cdrEvent);

  	CPPUNIT_ASSERT_MESSAGE( "end conf", STATUS_OK == status );
}
