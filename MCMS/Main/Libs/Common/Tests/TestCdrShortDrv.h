
#ifndef TESTCCDRSHORTDRV_H
#define TESTCCDRSHORTDRV_H

#include <cppunit/extensions/HelperMacros.h>

class TestCdrShortDrv : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestCdrShortDrv );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testSerializeDeserialize );
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();
	
	void testConstructor();
	void testSerializeDeserialize();
};

#endif 
