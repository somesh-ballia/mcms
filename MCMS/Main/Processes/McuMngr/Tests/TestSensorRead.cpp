#include "TestSensorRead.h"
#include "sensor_read.h"
#include "IpmiSensorDescrToType.h"
#include "GetSysTemperature.h"
#include <string.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestSensorRead );

#define DOUBLE_DELTA 0.0000001

void TestSensorRead::setUp()
{
}


void TestSensorRead::tearDown()
{
}

extern bool FunctionGetHDDTemperature(std::string &);
//#define TEST_REAL_SENSOR_READ
void TestSensorRead::testNormalRead()
{
#ifdef TEST_REAL_SENSOR_READ
    GetHDDTemperature = FunctionGetHDDTemperature;
    vector<sensor_t> sensors;
    sensor_read(sensors);
    CPPUNIT_ASSERT(sensors.size()>0);
    for (unsigned i=0; i<sensors.size(); ++i)
    {
        sensor_t const & elem = sensors[i];
        if (strstr(elem.SensorName, "HDD"))
        {
            CPPUNIT_ASSERT(1==IpmiSensorDescrToSensorNumber(elem.SensorName));
            printf("HDD temperature: %d", int(elem.CurrentVal));
        }
    }
#endif
}

int const FACTOR_DIV = 4;

void TestSensorRead::testValidateSensorReading()
{
    {
        sensor_t fan = { "Fan1A RPM", 3360.0, "RPM", "ok", INVALID_FLOAT_READING, 720.0, 840.0, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, EVENT_READ_TYPE_COLOR
            , { INVALID_FLOAT_READING, 720.0, 840.0, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING } };
        float const nominal = 3200.0;
        ValidateSensorReading(nominal, fan);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fan.LowerNonRecoverable, (float(720.0-nominal/FACTOR_DIV)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fan.UpperNonCritical, (float(nominal+nominal/FACTOR_DIV)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fan.UpperCritical, (float(nominal+nominal/FACTOR_DIV*2)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(fan.UpperNonRecoverable, (float(nominal+nominal/FACTOR_DIV*3)), DOUBLE_DELTA);
    }

    {
        sensor_t temperature = { "Inlet Temp", 26.0, "degrees C", "ok", INVALID_FLOAT_READING, -7.0, 3.0, 42.0, 47.0, INVALID_FLOAT_READING, EVENT_READ_TYPE_COLOR
            , { INVALID_FLOAT_READING, -7.0, 3.0, 42.0, 47.0, INVALID_FLOAT_READING } };
        float const nominal = 0.0;
        float const actualNominal = temperature.CurrentVal;
        ValidateSensorReading(nominal, temperature);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(temperature.LowerNonRecoverable, (float(-7.0-actualNominal/FACTOR_DIV)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(temperature.UpperNonRecoverable, (float(47.0+actualNominal/FACTOR_DIV)), DOUBLE_DELTA);
    }

    {
        sensor_t power = { "Pwr Consumption", 84.0, "Watts", "ok", INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, 896.0, 980.0, INVALID_FLOAT_READING, EVENT_READ_TYPE_COLOR
            , { INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, 896.0, 980.0, INVALID_FLOAT_READING } };
        float const nominal = 100.0;
        ValidateSensorReading(nominal, power);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(power.LowerNonRecoverable, (float(nominal-nominal/FACTOR_DIV*3)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(power.LowerCritical, (float(nominal-nominal/FACTOR_DIV*2)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(power.LowerNonCritical, (float(nominal-nominal/FACTOR_DIV*1)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(power.UpperNonRecoverable, (float(980.0+nominal/FACTOR_DIV)), DOUBLE_DELTA);
    }

    {
        sensor_t current = { "Current 1", 0.4, "Amps", "ok", INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, EVENT_READ_TYPE_COLOR
            , { INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING } };
        float const nominal = 0.4;
        ValidateSensorReading(nominal, current);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(current.LowerNonRecoverable, (float(nominal-nominal/FACTOR_DIV*3)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(current.LowerCritical, (float(nominal-nominal/FACTOR_DIV*2)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(current.LowerNonCritical, (float(nominal-nominal/FACTOR_DIV*1)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(current.UpperNonRecoverable, (float(nominal+nominal/FACTOR_DIV*3)), DOUBLE_DELTA);
    }

    {
        sensor_t voltage = { "Voltage 1", 226.0, "Volts", "ok", INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, EVENT_READ_TYPE_COLOR
            , { INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING, INVALID_FLOAT_READING } };
        float const nominal = 220.0;
        ValidateSensorReading(nominal, voltage);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(voltage.LowerNonRecoverable, (float(nominal-nominal/FACTOR_DIV*3)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(voltage.LowerCritical, (float(nominal-nominal/FACTOR_DIV*2)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(voltage.LowerNonCritical, (float(nominal-nominal/FACTOR_DIV*1)), DOUBLE_DELTA);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(voltage.UpperNonRecoverable, (float(nominal+nominal/FACTOR_DIV*3)), DOUBLE_DELTA);
    }
}

