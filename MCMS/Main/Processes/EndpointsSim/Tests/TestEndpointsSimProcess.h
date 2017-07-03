
#if !defined(TEST_EndpointsSim_H)
#define TEST_EndpointsSim_H


#include <cppunit/extensions/HelperMacros.h>

class CTestEndpointsSimProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestEndpointsSimProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};


#endif // !defined(TEST_EndpointsSim_H)

