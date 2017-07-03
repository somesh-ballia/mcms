
#if !defined(TEST_QAAPI_H)
#define TEST_QAAPI_H


#include <cppunit/extensions/HelperMacros.h>


class CTestQAAPIProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestQAAPIProcess );
	CPPUNIT_TEST( testConstructor );
	//CPPUNIT_TEST( testPTRACE );
//	CPPUNIT_TEST( testEmaTrace );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	//void testPTRACE();
	//void testEmaTrace();
};

#endif // !defined(TEST_QAAPI_H)

