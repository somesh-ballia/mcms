
#ifndef TESTIPSERVICE_H
#define TESTIPSERVICE_H

#include <cppunit/extensions/HelperMacros.h>

class TestIpService : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestIpService );
	CPPUNIT_TEST( testConvertToMplStruct );	
	
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();

	void testConvertToMplStruct();
};

#endif  // TESTIPSERVICE_H
