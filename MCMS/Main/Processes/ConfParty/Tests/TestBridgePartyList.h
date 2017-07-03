#ifndef _TEST_BRIDGEPARTYLIST_H__
#define _TEST_BRIDGEPARTYLIST_H__

#include <cppunit/extensions/HelperMacros.h>
#include "BridgePartyCntl.h"
#include "BridgePartyCntlMock.h"

class CBridgePartyList;

class CTestBridgePartyList   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestBridgePartyList );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testAddOneBridgePartyCntlAndCheckSize );
	CPPUNIT_TEST( testAddAndRemoveOneBridgePartyCntlAndCheckSize );
	CPPUNIT_TEST( testAddAndRemoveOneBridgePartyCntlAndCheckRemovedPartyPtr );
	CPPUNIT_TEST( testAddTwoBridgePartyCntlsAndCheckSize );
	CPPUNIT_TEST( testAddTwoBridgePartyCntlsAndRemoveFirstAndCheckRemovedParty );
	CPPUNIT_TEST( testAddTwoBridgePartyCntlsAndRemoveFirstAndCheckSize );
	CPPUNIT_TEST( testAddTwoBridgePartyCntlsAndRemoveSecondAndCheckRemovedParty );
	CPPUNIT_TEST( testAddTwoBridgePartyCntlsAndRemoveSecondAndCheckSize );
	CPPUNIT_TEST( testAddThreeBridgePartyCntlsAndRemoveSecondAndCheckRemovedParty );
	CPPUNIT_TEST( testAddTreeBridgePartyCntlsAndRemoveSecondAndCheckSize );
	CPPUNIT_TEST( testAddThreeBridgePartyCntlsCheckSizeAndRemoveAllOfThemAndCheckSize );
	CPPUNIT_TEST( testAddTreeBridgePartyCntlsAndFindSecondAndCheckSize );
	CPPUNIT_TEST( testAddTreeBridgePartyCntlsAndFindSecondByNameAndCheckSize );
	CPPUNIT_TEST( testAddFiveBridgePartyCntlsAndRemovedThirdAndCheckSizeAndFindFourthByName );
	CPPUNIT_TEST( testAddFiveBridgePartyCntlsAndFindAt4 );
	CPPUNIT_TEST( testAddFiveBridgePartyCntlsAndFindAt0 );
	CPPUNIT_TEST( testZeroPtrOfBridgePartyCntlAndCheckSize );
	CPPUNIT_TEST( testFindInEmptyList );
	CPPUNIT_TEST( testAddOneBridgePartyCntlAndTryToRemoveAnother );
	CPPUNIT_TEST( testAddOneBridgePartyCntlAndTryToRemoveZeroPtr );
	CPPUNIT_TEST( testAddThreeBridgePartyCntlsClearAndDestroyPartyList );
	//CPPUNIT_TEST( testGetFirstOnEmptyList );
	//CPPUNIT_TEST( testGetFirstInListWithOnePartyCnt );
	//CPPUNIT_TEST( testGetFirstInListWithTwoPartyCnt );
	//CPPUNIT_TEST( testGetNextOnEmptyList );
	//CPPUNIT_TEST( testGetFirstAndThenNextOnListWithOnePatyCnt );
	//CPPUNIT_TEST( testGetFirstAndThenNextOnListWithTwoPartyCntl );
	//CPPUNIT_TEST( testGoThroughPartyCntlListOfFiveWithGetFirstAndGetNext );

	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown();

	void testConstructor();
	void testAddOneBridgePartyCntlAndCheckSize();
	void testAddAndRemoveOneBridgePartyCntlAndCheckSize();
	void testAddAndRemoveOneBridgePartyCntlAndCheckRemovedPartyPtr();
	void testAddTwoBridgePartyCntlsAndCheckSize();
	void testAddTwoBridgePartyCntlsAndRemoveFirstAndCheckRemovedParty();
	void testAddTwoBridgePartyCntlsAndRemoveFirstAndCheckSize();
	void testAddTwoBridgePartyCntlsAndRemoveSecondAndCheckRemovedParty();
	void testAddTwoBridgePartyCntlsAndRemoveSecondAndCheckSize();
	void testAddThreeBridgePartyCntlsAndRemoveSecondAndCheckRemovedParty();
	void testAddTreeBridgePartyCntlsAndRemoveSecondAndCheckSize();
	void testAddThreeBridgePartyCntlsCheckSizeAndRemoveAllOfThemAndCheckSize();
	void testAddTreeBridgePartyCntlsAndFindSecondAndCheckSize();
	void testAddTreeBridgePartyCntlsAndFindSecondByNameAndCheckSize();
	void testAddFiveBridgePartyCntlsAndRemovedThirdAndCheckSizeAndFindFourthByName();
	void testAddFiveBridgePartyCntlsAndFindAt4();
	void testAddFiveBridgePartyCntlsAndFindAt0();
	void testZeroPtrOfBridgePartyCntlAndCheckSize();
	void testFindInEmptyList();
	void testAddOneBridgePartyCntlAndTryToRemoveAnother();
	void testAddOneBridgePartyCntlAndTryToRemoveZeroPtr();
	void testAddThreeBridgePartyCntlsClearAndDestroyPartyList();
	//void testGetFirstOnEmptyList();
	//void testGetFirstInListWithOnePartyCnt();
	//void testGetFirstInListWithTwoPartyCnt();
	//void testGetNextOnEmptyList();
	//void testGetFirstAndThenNextOnListWithOnePatyCnt();
	//void testGetFirstAndThenNextOnListWithTwoPartyCntl();
	//void testGoThroughPartyCntlListOfFiveWithGetFirstAndGetNext();

private:
	CBridgePartyList* m_pBridgePartyList;
	CBridgePartyCntlMock* m_pBridgePartyCntl_1;
	CBridgePartyCntlMock* m_pBridgePartyCntl_2;
	CBridgePartyCntlMock* m_pBridgePartyCntl_3;
};

#endif /* _BRIDGEPARTYLIST_H__ */
