#ifndef __TEST_LAN_PORT_LIST_H__
#define __TEST_LAN_PORT_LIST_H__

#include <cppunit/extensions/HelperMacros.h>

class TestLanPortList : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestLanPortList );
	CPPUNIT_TEST( testLanList );	
	CPPUNIT_TEST( testLanPort );	
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();

	void testLanList();
	void testLanPort();
};

#endif  // __TEST_LAN_PORT_LIST_H__

