
#ifndef __TEST_SENSOR_READ_H__
#define __TEST_SENSOR_READ_H__

#include <cppunit/extensions/HelperMacros.h>

class TestSensorRead : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestSensorRead );
	CPPUNIT_TEST( testNormalRead );	
	CPPUNIT_TEST( testValidateSensorReading );	
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();

	void testNormalRead();
	void testValidateSensorReading();
};

#endif  // __TEST_SENSOR_READ_H__

