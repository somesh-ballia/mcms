// // Boris G remove this tests (tests not relevant).
//
//#ifndef  TEST_SIP_REFER
//#define  TEST_SIP_REFER
//
//
//#include <cppunit/extensions/HelperMacros.h>
//
//#include "SipRefer.h"
//
//
//#include "MockCMplMcmsProtocol.h"
//#include "MockCConfApi.h"
//#include "MockConfPartyManagerLocalApi.h"
//
//
//
//// ************************************************************************************
////
////	CTestSIPREFERSubscriber
////
//// ************************************************************************************
//
//
//class CTestSIPREFERSubscriber   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestSIPREFERSubscriber );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testGetReferTo );
//	CPPUNIT_TEST( testGetReferTo2 );
//	CPPUNIT_TEST( testIsReferWithBye );
//	CPPUNIT_TEST( testIsReferWithBye2 );
//	CPPUNIT_TEST( testGetCSeq );
//	CPPUNIT_TEST( testGetCSeq2 );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void testConstructor();
//	void testGetReferTo();
//	void testGetReferTo2();
//	void testIsReferWithBye();
//	void testIsReferWithBye2();
//	void testGetCSeq();
//	void testGetCSeq2();
//
//private:
//
//	char m_referTo[H243_NAME_LEN];
//	char m_from[H243_NAME_LEN];
//	char m_fromTag[H243_NAME_LEN];
//	char m_to[H243_NAME_LEN];
//	char m_toTag[H243_NAME_LEN];
//	char m_callId[H243_NAME_LEN];
//	DWORD m_Ip, m_CSeq;
//	WORD m_port, m_transport;
//	WORD m_expires, m_srcUnitId;
//	CSIPREFERSubscriber* m_pSubscriber;
//};
//
//
//// ************************************************************************************
////
////	CTestSIPReferEventPackageManager
////
//// ************************************************************************************
//
//
//class CTestSIPReferEventPackageManager : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestSIPReferEventPackageManager );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testNotifyWasCalledAfterTOUT );
//	CPPUNIT_TEST( testNotifyWasNotCalledWhenTOUTisBigger );
//	CPPUNIT_TEST( test2ndNotifyIsNotSendDueToTimer );
//	CPPUNIT_TEST( testWaitingNotifyIsSentAfterTimer );
//	CPPUNIT_TEST( testOnNotifyTimerToutDoesNotSendNotify );
//	CPPUNIT_TEST( testNotifyHasReferTags );
//	CPPUNIT_TEST( testNotifyUsesDstUnitId );
//	CPPUNIT_TEST( testReferIsCollectedandDeletedAfterTout );
//	CPPUNIT_TEST( testReferWithIPonlyIsCollected );
//	CPPUNIT_TEST( testReferIsRejectedSinceRefPartyIsAlreadyConnected );
//	CPPUNIT_TEST( testReferIsAcceptedSinceRefPartyIsDisonnected );
//	CPPUNIT_TEST( testReferWithByeIsRejectedSinceRefPartyIsNotFound );
//	CPPUNIT_TEST( testDump );
//	CPPUNIT_TEST( testCheckReferWithBye1 );
//	CPPUNIT_TEST( testCheckReferWithBye2 );
//	CPPUNIT_TEST( testCheckReferWithBye3 );
//	CPPUNIT_TEST( testCheckReferWithBye4 );
//	CPPUNIT_TEST( testOnConnectReferredPartyTout );
//	CPPUNIT_TEST( testOnConnectReferredPartyTout_referBye );
//	CPPUNIT_TEST( testObserverUpdate );
//	CPPUNIT_TEST( testObserverUpdate2 );
//	CPPUNIT_TEST( testObserverUpdate3 );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void BuildRefer();
//
//
//	void testConstructor();
//	void testNotifyWasCalledAfterTOUT();
//	void testNotifyWasNotCalledWhenTOUTisBigger();
//	void test2ndNotifyIsNotSendDueToTimer();
//	void testWaitingNotifyIsSentAfterTimer();
//	void testOnNotifyTimerToutDoesNotSendNotify();
//	void testNotifyHasReferTags();
//	void testNotifyUsesDstUnitId();
//	void testReferIsCollectedandDeletedAfterTout();
//	void testReferWithIPonlyIsCollected();
//	void testReferIsRejectedSinceRefPartyIsAlreadyConnected();
//	void testReferIsAcceptedSinceRefPartyIsDisonnected();
//	void testReferWithByeIsRejectedSinceRefPartyIsNotFound();
//	void testDump();
//	void testCheckReferWithBye1();
//	void testCheckReferWithBye2();
//	void testCheckReferWithBye3();
//	void testCheckReferWithBye4();
//	void testOnConnectReferredPartyTout();
//	void testOnConnectReferredPartyTout_referBye();
//	void testObserverUpdate();
//	void testObserverUpdate2();
//	void testObserverUpdate3();
//
//private:
//	CMockMplMcmsProtocol*	 m_pMockProtocol;
//	CMockConfApi*			 m_pMockConfApi;
//	CMockConfPartyManagerLocalApi*	 m_pMockConfPartyMgrApi;
//	CSIPReferEventPackageManager* m_pMgr;
//	mcIndRefer*				 m_pReferMsg;
//	CSipHeaderList*			 m_pHeaderList;
//
//	CCommConf*	m_pCommConf;
//	CConfParty* m_pConfParty;
//	CCommConfDB* m_pConfDB;
//
//
//	char m_from[H243_NAME_LEN];
//	char m_fromTag[H243_NAME_LEN];
//	char m_toTag[H243_NAME_LEN];
//	char m_callId[H243_NAME_LEN];
//	char m_referTo[H243_NAME_LEN];
//	DWORD m_Ip, m_callIndex, m_CSeq;
//	WORD m_port, m_transport, m_srcUnitId;
//	char m_event[H243_NAME_LEN];
//	char m_allow[H243_NAME_LEN];
//	WORD m_expires;
//	void* m_haCall;
//};
//
//#endif
