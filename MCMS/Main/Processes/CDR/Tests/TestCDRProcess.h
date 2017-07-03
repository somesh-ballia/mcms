
#if !defined(TEST_CDR_H)
#define TEST_CDR_H

#include <cppunit/extensions/HelperMacros.h>


class CTestCDRProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestCDRProcess );
    CPPUNIT_TEST( testConstructor );
    CPPUNIT_TEST(testCdrEvent);
//	CPPUNIT_TEST( testInitConf );
//	CPPUNIT_TEST( testStartConf );
//	CPPUNIT_TEST( testEventConf );
//	CPPUNIT_TEST( testEndConf );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testInitConf();
	void testStartConf();
	void testEventConf();
	void testEndConf();
	void testCdrEvent();
};

#endif // !defined(TEST_CDR_H)
