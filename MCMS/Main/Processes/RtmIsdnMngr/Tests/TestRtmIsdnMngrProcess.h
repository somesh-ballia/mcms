
#if !defined(TEST_DEMO_H)
#define TEST_DEMO_H

#include <cppunit/extensions/HelperMacros.h>


class CTestRtmIsdnMngrProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestRtmIsdnMngrProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_DEMO_H)