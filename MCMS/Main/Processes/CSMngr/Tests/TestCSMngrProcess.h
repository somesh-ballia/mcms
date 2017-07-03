// TestCSMngrProcess.h: interface for the CTestTestProcess class.
// Unit tests using TDD of the CSMngr Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_CSMngr_H)
#define TEST_CSMngr_H


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestCSMngrProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestCSMngrProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};


#endif // !defined(TEST_CSMngr_H)

