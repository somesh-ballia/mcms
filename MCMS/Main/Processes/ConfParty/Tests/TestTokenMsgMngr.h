
#ifndef _TEST_TOKENMSGMGR_H__
#define _TEST_TOKENMSGMGR_H__

#include <cppunit/extensions/HelperMacros.h>
#include "TokenMsg.h"
#include "TokenMsgMngrMock.h"

class CTokenMsgMngr;

class CTestTokenMsgMngr   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestTokenMsgMngr );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testAddTokenMsg );
//	CPPUNIT_TEST( testAddAndRemoveTokenMsg);
//	CPPUNIT_TEST( testAddTwoTokenMsg );
//	CPPUNIT_TEST( testAddTwoTokenMsgsAndRemoveFirstAndCheckSize );
//	CPPUNIT_TEST( testAddThreeTokenMsgsCheckSizeAndRemoveAllOfThemAndCheckSize );
	CPPUNIT_TEST( testAddThreeTokenMsgsAndClearAndDestroyTokenMsgList );
	CPPUNIT_TEST( testRecieveAcquireReqWithStreamOFF );
	CPPUNIT_TEST( testRecieveCONTENT_ROLE_TOKEN_RELEASE__ReqFromParty);
	CPPUNIT_TEST( testOperatorOfTokenMsgList);
	CPPUNIT_TEST( testOperatorOfTokenMsg);

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testAddTokenMsg();
//	void testAddAndRemoveTokenMsg();
//	void testAddTwoTokenMsg();
//	void testAddTwoTokenMsgsAndRemoveFirstAndCheckSize();
//	void testAddThreeTokenMsgsCheckSizeAndRemoveAllOfThemAndCheckSize();
	void testAddThreeTokenMsgsAndClearAndDestroyTokenMsgList();
	void testRecieveAcquireReqWithStreamOFF();
	void testRecieveCONTENT_ROLE_TOKEN_RELEASE__ReqFromParty();
	void testOperatorOfTokenMsgList();
	void testOperatorOfTokenMsg();


private:
	CTokenMsgMngrMock* m_pTokenMsgMngr;	
	CTokenMsg*	   m_pTokenMsg_1;
	CTokenMsg*	   m_pTokenMsg_2;	
	CTokenMsg*	   m_pTokenMsg_3;
};

#endif /* _BRIDGEPARTYLIST_H__ */







