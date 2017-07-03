#if !defined(TEST_Logger_H)
#define TEST_Logger_H


#include <cppunit/extensions/HelperMacros.h>


class CTestLoggerProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestLoggerProcess );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testPTRACE );
//	CPPUNIT_TEST( testEmaTrace );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testPTRACE();
	void testEmaTrace();
};

#endif // !defined(TEST_Logger_H)

