

#include "TestObserver.h"
#include "DefinesGeneral.h"
#include "StatusesGeneral.h"
#include "MockCConfApi.h"
#include "ApiStatuses.h"


//CPPUNIT_TEST_SUITE_REGISTRATION( CTestObserverInfo );

// ************************************************************************************
//
//	CTestObserverInfo
//
// ************************************************************************************

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::setUp()
{
	m_pObserver = NULL;
	m_pCommConf = new CCommConf;

	m_pConfPartyProcess = new CConfPartyProcess();
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::tearDown()
{
	POBJDELETE(m_pObserver);
	POBJDELETE(m_pCommConf);
	POBJDELETE(m_pConfPartyProcess);
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testConstructor()
{
	m_pObserver = new CObserverInfo;
	CPPUNIT_ASSERT_MESSAGE( "CTestObserverInfo::testConstructor ", m_pObserver != NULL );  
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testGetObserver1()
{
	CSIPSubscriber* pSubscriber = new CSIPSubscriber;
	WORD event = PARTYSTATE;

	m_pObserver = new CObserverInfo(pSubscriber, event);
	CPPUNIT_ASSERT_MESSAGE( "CTestObserverInfo::testGetObserver1 ", pSubscriber == m_pObserver->GetObserver() );  
	POBJDELETE(pSubscriber);
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testGetObserver2()
{
	WORD event = PARTYSTATE;

	m_pObserver = new CObserverInfo(m_pCommConf, event);
	CPPUNIT_ASSERT_MESSAGE( "CTestObserverInfo::testGetObserver2 ", m_pCommConf == m_pObserver->GetObserver() );  
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testGetEvent1()
{
	WORD event = PARTYSTATE;

	m_pObserver = new CObserverInfo(m_pCommConf, event);
	CPPUNIT_ASSERT_MESSAGE( "CTestObserverInfo::testGetObserver2 ", PARTYSTATE == m_pObserver->GetEvent() );  
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testGetEvent2()
{
	WORD event = NETCHNL;

	m_pObserver = new CObserverInfo(m_pCommConf, event);
	CPPUNIT_ASSERT_MESSAGE( "CTestObserverInfo::testGetObserver2 ", NETCHNL == m_pObserver->GetEvent() );  
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testGetType1()
{
	WORD event = PARTYSTATE;
	WORD type = SIP_EVENT_PACKAGE;

	m_pObserver = new CObserverInfo(m_pCommConf, event, type);
	CPPUNIT_ASSERT_MESSAGE( "CTestObserverInfo::testGetType1 ", SIP_EVENT_PACKAGE == m_pObserver->GetType() );  
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testGetType2()
{
	WORD event = PARTYSTATE;
	WORD type = SIP_PROXY_REGISTER;

	m_pObserver = new CObserverInfo(m_pCommConf, event, type);
	CPPUNIT_ASSERT_MESSAGE( "CTestObserverInfo::testGetType2 ", SIP_PROXY_REGISTER == m_pObserver->GetType() );  
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testGetObserverInfo1_1()
{
	WORD event = PARTYSTATE;
	WORD type = SIP_EVENT_PACKAGE;
	DWORD observerInfo1 = 0x056789;

	m_pObserver = new CObserverInfo(m_pCommConf, event, type, observerInfo1);
	CPPUNIT_ASSERT_MESSAGE( "testGetObserverInfo1_1::testGetType2 ", 0x056789 == m_pObserver->GetObserverInfo1() );  
}

////////////////////////////////////////////////////////////////////
void CTestObserverInfo::testGetObserverInfo1_2()
{
	WORD event = PARTYSTATE;
	WORD type = SIP_EVENT_PACKAGE;
	DWORD observerInfo1 = 0x0000abcd;

	m_pObserver = new CObserverInfo(m_pCommConf, event, type, observerInfo1);
	CPPUNIT_ASSERT_MESSAGE( "testGetObserverInfo1_2::testGetType2 ", 0x0000abcd == m_pObserver->GetObserverInfo1() );  
}

// ************************************************************************************
//
//	CTestSubject
//
// ************************************************************************************

//CPPUNIT_TEST_SUITE_REGISTRATION( CTestSubject );

////////////////////////////////////////////////////////////////////
void CTestSubject::setUp()
{
	m_pObserver = new COsQueue;
	m_event = PARTYSTATE;	
	m_type = SIP_EVENT_PACKAGE;
	m_observerInfo1 = 0x00000000;

	m_pSubject = new CSubject;
	m_pConfPartyProcess = new CConfPartyProcess();
}

////////////////////////////////////////////////////////////////////
void CTestSubject::tearDown() 
{
	POBJDELETE(m_pObserver);
	POBJDELETE(m_pSubject);
	POBJDELETE(m_pConfPartyProcess);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testConstructor()
{  
	CPPUNIT_ASSERT_MESSAGE( "CTestSubject::testConstructor ", m_pSubject != NULL );  
} 

////////////////////////////////////////////////////////////////////
void CTestSubject::testAttachObserver()
{
	CPPUNIT_ASSERT_MESSAGE("AttachObserver Failed", STATUS_OK == m_pSubject->AttachObserver(m_pObserver, m_event));	
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testAttachObserverFails_IllegalObject()
{
	CPPUNIT_ASSERT_MESSAGE("AttachObserver Failed", STATUS_ILLEGAL_PBOJECT == m_pSubject->AttachObserver(m_pObserver, m_event));	
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testAttachObserverFails_IllegalObject2()
{
	CSIPSubscriber* pObserver = new CSIPSubscriber;
	CPPUNIT_ASSERT_MESSAGE("AttachObserver Failed", STATUS_ILLEGAL_PBOJECT == m_pSubject->AttachObserver((COsQueue*)pObserver, m_event));	
	POBJDELETE(pObserver);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testAttachObserverFails_UnknownEvent()
{
	m_event = 0xFFFF;
	CPPUNIT_ASSERT_MESSAGE("AttachObserver Failed", STATUS_ILLEGAL == m_pSubject->AttachObserver(m_pObserver, m_event));	
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testAttachObserverFails_AlreadyExists()
{
	m_pSubject->AttachObserver(m_pObserver, m_event);
	CPPUNIT_ASSERT_MESSAGE("AttachObserver Failed", STATUS_ALREADY_EXISTS == m_pSubject->AttachObserver(m_pObserver, m_event));	
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testDetachObserver()
{
	m_pSubject->AttachObserver(m_pObserver, m_event);
	CPPUNIT_ASSERT_MESSAGE("DetachObserver Failed", STATUS_OK == m_pSubject->DetachObserver(m_pObserver));	
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testDetachObserverFails_ObserverNotFound1()
{
	CPPUNIT_ASSERT_MESSAGE("DetachObserver Failed", NOT_FIND == m_pSubject->DetachObserver(m_pObserver));	
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testDetachObserverFails_ObserverNotFound2()
{
	COsQueue* pObserver2 = new COsQueue();

	m_pSubject->AttachObserver(m_pObserver, m_event);
	CPPUNIT_ASSERT_MESSAGE("DetachObserver Failed", NOT_FIND == m_pSubject->DetachObserver(pObserver2));	
	POBJDELETE(pObserver2);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testDetachObserverFails_IllegalObject()
{
	m_pSubject->AttachObserver(m_pObserver, m_event);
//	m_pObserver->m_validFlag = 0;
	CPPUNIT_ASSERT_MESSAGE("DetachObserver Failed", STATUS_ILLEGAL_PBOJECT == m_pSubject->DetachObserver(m_pObserver));	
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testNotifyUpdatesObserver()
{
	CMockConfApi* pMockConfApi = new CMockConfApi;
	POBJDELETE(m_pSubject);
	m_pSubject = new CSubject(pMockConfApi);
	
	m_pSubject->AttachObserver(m_pObserver, m_event);
	m_pSubject->Notify(m_event, 7777);
	WORD _event = 0, _type = 0, _count = 0;
	DWORD _val = 0, _observerInfo1 = 0;
	COsQueue* _queue = NULL;
	pMockConfApi->Verify_ObserverUpdate_WasCalled(&_queue, &_event, &_type, &_observerInfo1, &_val, &_count);

	CPPUNIT_ASSERT_MESSAGE("Count is wrong ", _count == 1);
	CPPUNIT_ASSERT_MESSAGE("Event is wrong ", _event == m_event);	
	CPPUNIT_ASSERT_MESSAGE("Type is wrong ", _type == 0);
	CPPUNIT_ASSERT_MESSAGE("ObserverInfo1 is wrong ", _observerInfo1 == 0);
	CPPUNIT_ASSERT_MESSAGE("Event is wrong ", _event == m_event);
	CPPUNIT_ASSERT_MESSAGE("Val is wrong", _val == 7777);	
	CPPUNIT_ASSERT_MESSAGE("pointer is wrong", _queue == m_pObserver);	

	POBJDELETE(pMockConfApi);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testNotifyUpdatesObserver2()
{
	CMockConfApi* pMockConfApi = new CMockConfApi;
	POBJDELETE(m_pSubject);
	m_pSubject = new CSubject(pMockConfApi);
	
	m_pSubject->AttachObserver(m_pObserver, m_event, m_type, m_observerInfo1);
	m_pSubject->Notify(m_event, 7777);
	WORD _event = 0, _type = 0, _count = 0;
	DWORD _val = 0, _observerInfo1 = 0;
	COsQueue* _queue = NULL;
	pMockConfApi->Verify_ObserverUpdate_WasCalled(&_queue, &_event, &_type, &_observerInfo1, &_val, &_count);

	CPPUNIT_ASSERT_MESSAGE("Count is wrong ", _count == 1);
	CPPUNIT_ASSERT_MESSAGE("Event is wrong ", _event == m_event);	
	CPPUNIT_ASSERT_MESSAGE("Type is wrong ", _type == m_type);
	CPPUNIT_ASSERT_MESSAGE("ObserverInfo1 is wrong ", _observerInfo1 == m_observerInfo1);
	CPPUNIT_ASSERT_MESSAGE("Event is wrong ", _event == m_event);
	CPPUNIT_ASSERT_MESSAGE("Val is wrong", _val == 7777);	
	CPPUNIT_ASSERT_MESSAGE("pointer is wrong", _queue == m_pObserver);	

	POBJDELETE(pMockConfApi);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testNotifyUpdatesObservers()
{
	CMockConfApi* pMockConfApi = new CMockConfApi;
	POBJDELETE(m_pSubject);
	m_pSubject = new CSubject(pMockConfApi);
	COsQueue* pQueue = new COsQueue;
	
	m_pSubject->AttachObserver(m_pObserver, m_event, m_type, m_observerInfo1);
	m_pSubject->AttachObserver(pQueue, m_event, m_type, m_observerInfo1);
	m_pSubject->Notify(m_event, 7777);
	WORD _event = 0, _type = 0, _count = 0;
	DWORD _val = 0, _observerInfo1 = 0;
	COsQueue* _queue = NULL;
	pMockConfApi->Verify_ObserverUpdate_WasCalled(&_queue, &_event, &_type, &_observerInfo1, &_val, &_count);

	CPPUNIT_ASSERT_MESSAGE("Count is wrong ", _count == 2);
	CPPUNIT_ASSERT_MESSAGE("Event is wrong ", _event == m_event);	
	CPPUNIT_ASSERT_MESSAGE("Val is wrong", _val == 7777);	
	CPPUNIT_ASSERT_MESSAGE("pointer is wrong", _queue == m_pObserver);	

	POBJDELETE(pQueue);
	POBJDELETE(pMockConfApi);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testNotifyUpdatesObservers_onlyMatchingEvent()
{
	CMockConfApi* pMockConfApi = new CMockConfApi;
	POBJDELETE(m_pSubject);
	m_pSubject = new CSubject(pMockConfApi);
	COsQueue* pQueue = new COsQueue;
	
	m_pSubject->AttachObserver(m_pObserver, m_event, m_type, m_observerInfo1);
	m_pSubject->AttachObserver(pQueue, AUDIOSRC);
	m_pSubject->Notify(m_event, 7667);
	WORD _event = 0, _type = 0, _count = 0;
	DWORD _val = 0, _observerInfo1 = 0;
	COsQueue* _queue = NULL;
	pMockConfApi->Verify_ObserverUpdate_WasCalled(&_queue, &_event, &_type, &_observerInfo1, &_val, &_count);

	CPPUNIT_ASSERT_MESSAGE("Count is wrong ", _count == 1);
	CPPUNIT_ASSERT_MESSAGE("Event is wrong ", _event == m_event);	
	CPPUNIT_ASSERT_MESSAGE("Val is wrong", _val == 7667);	
	CPPUNIT_ASSERT_MESSAGE("pointer is wrong", _queue == m_pObserver);	

	POBJDELETE(pQueue);
	POBJDELETE(pMockConfApi);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testNotifyDontUpdateObserver_notMatchingEvent()
{
	CMockConfApi* pMockConfApi = new CMockConfApi;
	POBJDELETE(m_pSubject);
	m_pSubject = new CSubject(pMockConfApi);
	
	m_pSubject->AttachObserver(m_pObserver, m_event);
	m_pSubject->Notify(AUDIOSRC, 7777);
	pMockConfApi->Verify_ObserverUpdate_WasNotCalled();

	POBJDELETE(pMockConfApi);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testNotifyDontUpdateObserver_notAttached()
{
	CMockConfApi* pMockConfApi = new CMockConfApi;
	POBJDELETE(m_pSubject);
	m_pSubject = new CSubject(pMockConfApi);
	
	m_pSubject->AttachObserver(m_pObserver, m_event);
	m_pSubject->DetachObserver(m_pObserver);
	m_pSubject->Notify(m_event, 7777);
	pMockConfApi->Verify_ObserverUpdate_WasNotCalled();

	POBJDELETE(pMockConfApi);
}

////////////////////////////////////////////////////////////////////
void CTestSubject::testNotifyUpdateObserver2()
{
	CMockConfApi* pMockConfApi = new CMockConfApi;
	POBJDELETE(m_pSubject);
	m_pSubject = new CSubject(pMockConfApi);
	
	m_pSubject->AttachObserver(m_pObserver, m_event);
	m_pSubject->Notify(m_event, 7755);
	WORD _event = 0, _type = 0, _count = 0;
	DWORD _val = 0, _observerInfo1 = 0;
	COsQueue* _queue = NULL;
	pMockConfApi->Verify_ObserverUpdate_WasCalled(&_queue, &_event, &_type, &_observerInfo1, &_val, &_count);

	CPPUNIT_ASSERT_MESSAGE("Count is wrong ", _count == 1);
	CPPUNIT_ASSERT_MESSAGE("Event is wrong ", _event == m_event);	
	CPPUNIT_ASSERT_MESSAGE("Val is wrong", _val == 7755);	
	CPPUNIT_ASSERT_MESSAGE("pointer is wrong", _queue == m_pObserver);
	
	m_pSubject->DetachObserver(m_pObserver);
	m_pSubject->Notify(m_event, 7755);
	pMockConfApi->Verify_ObserverUpdate_WasNotCalled();

	POBJDELETE(pMockConfApi);
}

