
#if !defined(TEST_SipProxy_H)
#define TEST_SipProxy_H

#include <cppunit/extensions/HelperMacros.h>

class CTestSipProxyProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSipProxyProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_SipProxy_H)

