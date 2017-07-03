
#ifndef __TEST_GET_SENSOR_ALARM_H__
#define __TEST_GET_SENSOR_ALARM_H__

#include <cppunit/extensions/HelperMacros.h>

class TestGetSensorAlarm : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( TestGetSensorAlarm );
	CPPUNIT_TEST( testNormalReading );	
	CPPUNIT_TEST( testReadingNotFound );	
	CPPUNIT_TEST( testReadingOfMajorAlarm );	
	CPPUNIT_TEST( testReadingOfCriticalAlarm );	
	CPPUNIT_TEST_SUITE_END();
	
public:
	void setUp();
	void tearDown();

	void testNormalReading();
	void testReadingNotFound();
	void testReadingOfMajorAlarm();
	void testReadingOfCriticalAlarm();
};

#endif  // __TEST_GET_SENSOR_ALARM_H__

