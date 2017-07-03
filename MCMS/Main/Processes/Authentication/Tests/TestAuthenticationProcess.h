// TestAuthenticationProcess.h: interface for the CTestTestProcess class.
// Unit tests using TDD of the Authentication Process
// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_Authentication_H)
#define TEST_Authentication_H


// part of official UnitTest library
#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestAuthenticationProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestAuthenticationProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_Authentication_H)

