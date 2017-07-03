
#if !defined(TEST_McmsDaemon_H)
#define TEST_McmsDaemon_H

#include <cppunit/extensions/HelperMacros.h>


class CTestMcmsDaemonProcess   : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( CTestMcmsDaemonProcess );
	CPPUNIT_TEST( testConstructor );
	//...
	CPPUNIT_TEST_SUITE_END();
public:

	void setUp();
	void tearDown(); 
	
	void testConstructor();
};

#endif // !defined(TEST_McmsDaemon_H)

