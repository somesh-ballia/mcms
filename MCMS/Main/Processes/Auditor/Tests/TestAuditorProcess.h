
#if !defined(TEST_AUDITOR_H)
#define TEST_AUDITOR_H

#include <cppunit/extensions/HelperMacros.h>


class CTestAuditorProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestAuditorProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_AUDITOR_H)
