//
//
//#ifndef  TEST_SIP_MS_DATA
//#define  TEST_SIP_MS_DATA
//
//
//#include <cppunit/extensions/HelperMacros.h>
//
//#include "SIPMsftCX.h"
//#include "psosxml.h"
//
//
//// ************************************************************************************
////
////	CTestCXConfInfoType
////
//// ************************************************************************************
//
//
//class CTestCXConfInfoType   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestCXConfInfoType );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testGetElementName );
//	CPPUNIT_TEST( testGetEntity );
//	CPPUNIT_TEST( testGetState1 );
//	CPPUNIT_TEST( testWasUpdated1 );
//	CPPUNIT_TEST( testWasUpdated2 );
//	CPPUNIT_TEST( testSerializeXML );
//	CPPUNIT_TEST( testSerializeXML1 );
//	CPPUNIT_TEST( testSerializeXMLWith2Parties );
//	CPPUNIT_TEST( testSerializeXMLWith1PartyAndPartialDataForLock );
//	CPPUNIT_TEST( testSerializeXMLWith1PartyAndPartialDataForPartyMute );
//	CPPUNIT_TEST( testSerializeXMLWithNoChange );
//	CPPUNIT_TEST( testGetState2 );
//	CPPUNIT_TEST( testAddParty );
//	CPPUNIT_TEST( testDelParty1 );
//	CPPUNIT_TEST( testDelParty2 );
//	CPPUNIT_TEST( testSetDisplayName );
//	CPPUNIT_TEST( testSetDisplayPhoneNumber );
//	CPPUNIT_TEST( testSetSipUri );
//	CPPUNIT_TEST( testSetAdministrator );
//	CPPUNIT_TEST( testSetPartyStatus );
//	CPPUNIT_TEST( testSetDialOutParty );
//	CPPUNIT_TEST( testSetDisconnectReason );
//	CPPUNIT_TEST( testSetActiveSpeaker );
//	CPPUNIT_TEST( testSetUriAttribute );
//	CPPUNIT_TEST( testMutePartyMedia );
//	CPPUNIT_TEST( testSetActive );
//	CPPUNIT_TEST( testSetLocked );
//	CPPUNIT_TEST( testSetRollCall );
//	CPPUNIT_TEST( testSetAnnouncements );
//	CPPUNIT_TEST( testSetRecorded );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void	setUp();
//	void	tearDown();
//
//	void	testConstructor();
//	void	testGetElementName();
//	void	testWasUpdated1();
//	void	testWasUpdated2();
//	void	testSerializeXML();
//	void	testSerializeXML1();
//	void	testSerializeXMLWith2Parties();
//	void	testSerializeXMLWith1PartyAndPartialDataForLock();
//	void	testSerializeXMLWith1PartyAndPartialDataForPartyMute();
//	void	testSerializeXMLWithNoChange();
//	void	testGetEntity();
//	void	testGetState1();
//	void	testGetState2();
//
//	void	testAddParty();
//	void	testDelParty1();
//	void	testDelParty2();
//	void	testSetDisplayName();
//	void	testSetDisplayPhoneNumber();
//	void	testSetSipUri();
//	void	testSetAdministrator();
//	void	testSetPartyStatus();
//	void	testSetDialOutParty();
//	void	testSetDisconnectReason();
//	void	testSetActiveSpeaker();
//	void	testSetUriAttribute();
//	void	testMutePartyMedia();
//	void	testSetActive();
//	void	testSetLocked();
//	void	testSetRollCall();
//	void	testSetAnnouncements();
//	void	testSetRecorded();
//
//private:
//
//	CCXConfInfoType* m_pInfo;
//};
//
//
//// ************************************************************************************
////
////	CTestCXUserType
////
//// ************************************************************************************
//
//
//class CTestCXUserType   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestCXUserType );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testConstructorWithCConfParty );
//	CPPUNIT_TEST( testGetElementName );
//	CPPUNIT_TEST( testPartyRole1 );
//	CPPUNIT_TEST( testPartyRole2 );
//	CPPUNIT_TEST( testPartyJoinMode1 );
//	CPPUNIT_TEST( testPartyJoinMode1 );
//	CPPUNIT_TEST( testPartyDisconnectReason1 );
//	CPPUNIT_TEST( testSerializeXML );
//	CPPUNIT_TEST( testSerializeXML1 );
//	CPPUNIT_TEST( testSetDisplayName );
//	CPPUNIT_TEST( testSetDisplayPhoneNumber );
//	CPPUNIT_TEST( testSetSipUri1 );
//	CPPUNIT_TEST( testSetSipUri2 );
//	CPPUNIT_TEST( testSetAdministrator );
//	CPPUNIT_TEST( testSetPartyStatus );
//	CPPUNIT_TEST( testSetDialOutParty );
//	CPPUNIT_TEST( testSetDiscoonectReason1 );
//	CPPUNIT_TEST( testSetDiscoonectReason2 );
//	CPPUNIT_TEST( testSetActiveSpeaker );
//	CPPUNIT_TEST( testMuteMedia );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void testConstructor();
//	void testConstructorWithCConfParty();
//	void testGetElementName();
//	void testSerializeXML();
//	void testSerializeXML1();
//	void testPartyRole1();
//	void testPartyRole2();
//	void testPartyStatus1();
//	void testPartyStatus2();
//	void testPartyJoinMode1();
//	void testPartyJoinMode2();
//	void testPartyDisconnectReason1();
//	void testSetDisplayName();
//	void testSetDisplayPhoneNumber();
//	void testSetSipUri1();
//	void testSetSipUri2();
//	void testSetAdministrator();
//	void testSetPartyStatus();
//	void testSetDialOutParty();
//	void testSetDiscoonectReason1();
//	void testSetDiscoonectReason2();
//	void testSetActiveSpeaker();
//	void testMuteMedia();
//
//private:
//
//	CCXUserType* m_pUserInfo;
//};
//
//
//
//// ************************************************************************************
////
////	CTestCXMediaType
////
//// ************************************************************************************
//
//
//class CTestCXMediaType   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestCXMediaType );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testGetElementName );
//	CPPUNIT_TEST( testGetContent1 );
//	CPPUNIT_TEST( testGetContent2 );
//	CPPUNIT_TEST( testSerializeXML );
//	CPPUNIT_TEST( testSerializeXML1 );
//	CPPUNIT_TEST( testMuteMedia );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void testConstructor();
//	void testGetElementName();
//	void testSerializeXML();
//	void testSerializeXML1();
//	void testGetContent1();
//	void testGetContent2();
//	void testMuteMedia();
//
//private:
//
//	CCXMediaType* m_pMedia;
//};
//
//// ************************************************************************************
////
////	CTestCXMediaStateType
////
//// ************************************************************************************
//
//
//class CTestCXMediaStateType   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestCXMediaStateType );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testGetElementName );
//	CPPUNIT_TEST( testSetMute );
//	CPPUNIT_TEST( testSerializeXML );
//	CPPUNIT_TEST( testSerializeXML1 );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void testConstructor();
//	void testGetElementName();
//	void testSerializeXML();
//	void testSerializeXML1();
//	void testSetMute();
//
//private:
//
//	CCXMediaStateType* m_pMedia;
//};
//
//// ************************************************************************************
////
////	CTestCXInitialState
////
//// ************************************************************************************
//
//
//class CTestCXInitialState   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestCXInitialState );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testGetElementName );
//	CPPUNIT_TEST( testSerializeXML );
//	CPPUNIT_TEST( testSerializeXML1 );
//	CPPUNIT_TEST( testSetMute );
//	CPPUNIT_TEST( testSetAdmin );
//	CPPUNIT_TEST( testSetRejoinAsDialOut );
//	CPPUNIT_TEST( testSetJoinAsDialOut );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void testConstructor();
//	void testGetElementName();
//	void testSerializeXML();
//	void testSerializeXML1();
//	void testSetMute();
//	void testSetAdmin();
//	void testSetJoinAsDialOut();
//	void testSetRejoinAsDialOut();
//
//private:
//
//	CCXInitialState* m_pState;
//};
//
//
//// ************************************************************************************
////
////	CTestCXPropertiesType
////
//// ************************************************************************************
//
//class CTestCXPropertiesType   : public CPPUNIT_NS::TestFixture
//{
//	CPPUNIT_TEST_SUITE( CTestCXPropertiesType );
//	CPPUNIT_TEST( testConstructor );
//	CPPUNIT_TEST( testGetElementName );
//	CPPUNIT_TEST( testSerializeXML );
//	CPPUNIT_TEST( testSerializeXML1 );
//	CPPUNIT_TEST( testSetActive );
//	CPPUNIT_TEST( testSetLocked );
//	CPPUNIT_TEST( testSetRollCall );
//	CPPUNIT_TEST( testSetAnnouncements );
//	CPPUNIT_TEST( testSetRecorded );
//	CPPUNIT_TEST_SUITE_END();
//
//public:
//	void setUp();
//	void tearDown();
//
//	void testConstructor();
//	void testGetElementName();
//	void testSerializeXML();
//	void testSerializeXML1();
//	void testSetActive();
//	void testSetLocked();
//	void testSetRollCall();
//	void testSetAnnouncements();
//	void testSetRecorded();
//
//private:
//
//	CCXPropertiesType* m_pProperties;
//};
//
//
//
//
//#endif
//
