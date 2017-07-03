
#if !defined(TEST_MplApi_H)
#define TEST_MplApi_H


#include <cppunit/extensions/HelperMacros.h>

class CTestMplApiProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestMplApiProcess );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testMPLAPI_MSG );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testMPLAPI_MSG();
};


#endif // !defined(TEST_MplApi_H)

