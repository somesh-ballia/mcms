#if !defined(TEST_McuCmd_H)
#define TEST_McuCmd_H


#include <cppunit/extensions/HelperMacros.h>

class CTestMcuCmdProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestMcuCmdProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_McuCmd_H)

