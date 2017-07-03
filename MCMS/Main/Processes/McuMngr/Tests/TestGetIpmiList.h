#ifndef __TEST_GET_IPMI_LIST_H__
#define __TEST_GET_IPMI_LIST_H__

#include <cppunit/extensions/HelperMacros.h>

class TestGetIpmiList : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestGetIpmiList );
	CPPUNIT_TEST( testGetNormalStatusList );	
	CPPUNIT_TEST( testGetAlertStatusList );	
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();

	void testGetNormalStatusList();
	void testGetAlertStatusList();
};

#endif  // __TEST_GET_IPMI_LIST_H__

