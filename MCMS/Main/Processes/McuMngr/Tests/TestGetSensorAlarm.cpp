#include "TestGetSensorAlarm.h"
#include "sensor_read.h"
#include "IpmiSensorDescrToType.h"
#include "GetSysTemperature.h"
#include "FaultsDefines.h"
#include <string.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION(TestGetSensorAlarm);

#define DOUBLE_DELTA 0.0000001

void TestGetSensorAlarm::setUp()
{
}

void TestGetSensorAlarm::tearDown()
{
}

void TestGetSensorAlarm::testNormalReading()
{
	sensor_t s =
	{
		"CPU Temp", 50.0, "degrees", "ok", 10.0, 20.0, 30.0, 60.0, 70.0, 80.0, EVENT_READ_TYPE_COLOR,
		{(float)10.0, (float)20.0, (float)30.0, (float)60.0, (float)70.0, (float)80.0}
	};

	AlarmInfo const info = GetAlarmInfo(s);
	CPPUNIT_ASSERT(AA_TEMPERATURE_MAJOR_PROBLEM == info.base);
	CPPUNIT_ASSERT(ALARM_OFFSET_NORMAL == info.offset);
}

void TestGetSensorAlarm::testReadingNotFound()
{
	sensor_t s =
	{
		"cannot be found", 50.0, "degrees", "ok", 10.0, 20.0, 30.0, 60.0, 70.0, 80.0, EVENT_READ_TYPE_COLOR,
		{(float)10.0, (float)20.0, (float)30.0, (float)60.0, (float)70.0, (float)80.0}
	};

	AlarmInfo const info = GetAlarmInfo(s);
	CPPUNIT_ASSERT(-1 == info.base);
	CPPUNIT_ASSERT(0 == info.offset);
}

void TestGetSensorAlarm::testReadingOfMajorAlarm()
{
	sensor_t s =
	{
		"CPU Temp", 25.0, "degrees", "ok", 10.0, 20.0, 30.0, 60.0, 70.0, 80.0, EVENT_READ_TYPE_COLOR,
		{(float)10.0, (float)20.0, (float)30.0, (float)60.0, (float)70.0, (float)80.0}
	};

	AlarmInfo const info = GetAlarmInfo(s);
	CPPUNIT_ASSERT(AA_TEMPERATURE_MAJOR_PROBLEM == info.base);
	CPPUNIT_ASSERT(ALARM_OFFSET_MAJOR == info.offset);
}

void TestGetSensorAlarm::testReadingOfCriticalAlarm()
{
	{
		sensor_t s =
		{
			"CPU Temp", 15.0, "degrees", "ok", 10.0, 20.0, 30.0, 60.0, 70.0, 80.0, EVENT_READ_TYPE_COLOR,
			{(float)10.0, (float)20.0, (float)30.0, (float)60.0, (float)70.0, (float)80.0}
		};

		AlarmInfo const info = GetAlarmInfo(s);
		CPPUNIT_ASSERT(AA_TEMPERATURE_MAJOR_PROBLEM == info.base);
		CPPUNIT_ASSERT(ALARM_OFFSET_CRITICAL == info.offset);
	}

	{
		sensor_t s =
		{
			"CPU Temp", 5.0, "degrees", "ok", 10.0, 20.0, 30.0, 60.0, 70.0, 80.0, EVENT_READ_TYPE_COLOR,
			{(float)10.0, (float)20.0, (float)30.0, (float)60.0, (float)70.0, (float)80.0}
		};

		AlarmInfo const info = GetAlarmInfo(s);
		CPPUNIT_ASSERT(AA_TEMPERATURE_MAJOR_PROBLEM == info.base);
		CPPUNIT_ASSERT(ALARM_OFFSET_CRITICAL == info.offset);
	}
}

