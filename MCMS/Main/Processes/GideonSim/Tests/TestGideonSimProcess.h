#if !defined(TEST_GideonSim_H)
#define TEST_GideonSim_H


#include <cppunit/extensions/HelperMacros.h>


class CTestGideonSimProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestGideonSimProcess );
//	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};


#endif // !defined(TEST_GideonSim_H)

