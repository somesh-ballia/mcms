
#ifndef TEST_OBSERVER
#define TEST_OBSERVER

#include <cppunit/extensions/HelperMacros.h>

#include "Observer.h"
#include "TaskApi.h"
#include "SIPConfPack.h"
#include "ConfPartyProcess.h"

class CTestObserverInfo : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestObserverInfo );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testGetObserver1 );
	CPPUNIT_TEST( testGetObserver2 );
	CPPUNIT_TEST( testGetEvent1 );
	CPPUNIT_TEST( testGetEvent2 );
	CPPUNIT_TEST( testGetType1 );
	CPPUNIT_TEST( testGetType2 );
	CPPUNIT_TEST( testGetObserverInfo1_1 );
	CPPUNIT_TEST( testGetObserverInfo1_2 );
	CPPUNIT_TEST_SUITE_END();


public:
	void setUp();
	void tearDown(); 

	void testConstructor();
	void testGetObserver1();
	void testGetObserver2();
	void testGetEvent1();
	void testGetEvent2();
	void testGetType1();
	void testGetType2();
	void testGetObserverInfo1_1();
	void testGetObserverInfo1_2();

protected:
	
	CObserverInfo* m_pObserver;
	CCommConf*	m_pCommConf;

	CConfPartyProcess* m_pConfPartyProcess;
};





class CTestSubject   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSubject );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testAttachObserver );
	/*CPPUNIT_TEST( testAttachObserverFails_IllegalObject );
	CPPUNIT_TEST( testAttachObserverFails_IllegalObject2 );
	CPPUNIT_TEST( testAttachObserverFails_UnknownEvent );
	CPPUNIT_TEST( testDetachObserver );
	CPPUNIT_TEST( testDetachObserverFails_ObserverNotFound1 );
	CPPUNIT_TEST( testDetachObserverFails_ObserverNotFound2 );
	CPPUNIT_TEST( testDetachObserverFails_IllegalObject );
	CPPUNIT_TEST( testNotifyUpdatesObserver );
	CPPUNIT_TEST( testNotifyUpdatesObserver2 );
	CPPUNIT_TEST( testNotifyUpdatesObservers );
	CPPUNIT_TEST( testNotifyUpdatesObservers_onlyMatchingEvent );
	CPPUNIT_TEST( testNotifyDontUpdateObserver_notMatchingEvent );
	CPPUNIT_TEST( testNotifyDontUpdateObserver_notAttached );
	CPPUNIT_TEST( testNotifyUpdateObserver2 );
	CPPUNIT_TEST( testAttachObserverFails_AlreadyExists );*/
	CPPUNIT_TEST_SUITE_END();


public:
	void setUp();
	void tearDown(); 

	void testConstructor();
	void testAttachObserver();
	void testAttachObserverFails_IllegalObject();
	void testAttachObserverFails_IllegalObject2();
	void testAttachObserverFails_UnknownEvent();
	void testDetachObserver();
	void testDetachObserverFails_ObserverNotFound1();
	void testDetachObserverFails_ObserverNotFound2();
	void testDetachObserverFails_IllegalObject();
	void testNotifyUpdatesObserver();
	void testNotifyUpdatesObserver2();
	void testNotifyUpdatesObservers();
	void testNotifyUpdatesObservers_onlyMatchingEvent();
	void testNotifyDontUpdateObserver_notMatchingEvent();
	void testNotifyDontUpdateObserver_notAttached();
	void testNotifyUpdateObserver2();
	void testAttachObserverFails_AlreadyExists();

protected:
	CSubject* m_pSubject;
	COsQueue* m_pObserver;
	WORD	m_event;
	WORD	m_type;
	DWORD	m_observerInfo1;

	CConfPartyProcess* m_pConfPartyProcess;
};


#endif
