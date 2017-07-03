// Stand alone function tests should not be part of this suite
//////////////////////////////////////////////////////////////////////

#if !defined(TEST_DEMO_H)
#define TEST_DEMO_H

#include <cppunit/extensions/HelperMacros.h>

// private tests (example)

class CTestDemoProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestDemoProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_DEMO_H)