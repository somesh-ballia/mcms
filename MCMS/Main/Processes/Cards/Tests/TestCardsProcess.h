// TestCardsProcess.h: interface for the CTestTestProcess class.
// Unit tests using TDD of the Cards Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_Cards_H)
#define TEST_Cards_H


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestCardsProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestCardsProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_Cards_H)

