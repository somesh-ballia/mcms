#if !defined(TEST_McuMngr_H)
#define TEST_McuMngr_H

#include <cppunit/extensions/HelperMacros.h>


class CTestMcuMngrProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestMcuMngrProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};


#endif // !defined(TEST_McuMngr_H)

