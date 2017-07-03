#if !defined(TEST_Resource_H)
#define TEST_Resource_H

#include <cppunit/extensions/HelperMacros.h>


class CTestResourceProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestResourceProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};


#endif // !defined(TEST_Resource_H)

