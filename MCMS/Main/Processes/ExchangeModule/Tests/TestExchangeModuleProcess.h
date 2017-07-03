
#if !defined(TEST_EXCHANGEMODULE_H)
#define TEST_EXCHANGEMODULE_H

#include <cppunit/extensions/HelperMacros.h>

class CTestExchangeModuleProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestExchangeModuleProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown();

	void testConstructor();
};

#endif // !defined(TEST_EXCHANGEMODULE_H)
