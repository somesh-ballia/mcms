
#if !defined(TEST_TESTSOCKETS_H)
#define TEST_TESTSOCKETS_H

#include <cppunit/extensions/HelperMacros.h>


class CTestSockets  : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestSockets  );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testClientSocketConstructor );

	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
	void testClientSocketConstructor();

};


#endif // !defined(TEST_TESTSOCKETS_H)

