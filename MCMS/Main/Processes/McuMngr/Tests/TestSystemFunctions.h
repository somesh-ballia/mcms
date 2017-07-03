#ifndef __TEST_SYSTEM_FUNCTIONS_H__
#define __TEST_SYSTEM_FUNCTIONS_H__

#include <cppunit/extensions/HelperMacros.h>

class TestSystemFunctions : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestSystemFunctions );
	CPPUNIT_TEST( testGetBiosDate );	
	CPPUNIT_TEST( testGetBiosVersion );	
	CPPUNIT_TEST( testGetBiosVendor );	
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();

	void testGetBiosDate();
	void testGetBiosVersion();
	void testGetBiosVendor();
};

#endif  // __TEST_SYSTEM_FUNCTIONS_H__

