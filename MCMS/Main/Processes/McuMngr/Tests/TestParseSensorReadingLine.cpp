#include "TestParseSensorReadingLine.h"
#include "sensor_read.h"
#include "IpmiSensorDescrToType.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestParseSensorReadingLine );

void CTestParseSensorReadingLine::setUp()
{
}

void CTestParseSensorReadingLine::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
#define DOUBLE_DELTA 0.0000001
void CTestParseSensorReadingLine::testParseNALine()
{
    char const * naline = "Current 2        | 0.100      | Amps       | ok    | na        | na        | na        | na        | na        | na";
    sensor_t elem;
    sensor_t * temp = &elem;
    DescrSensorEntity ref;

    {
        ParseSensorReadingLine(naline, temp, ref);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(INVALID_FLOAT_READING, elem.UpperNonRecoverable, DOUBLE_DELTA);
    }

    {
        float const nominal = IpmiSensorDescrToNominalVal(temp->SensorName);
        CPPUNIT_ASSERT_DOUBLES_EQUAL((float)0.4, nominal, DOUBLE_DELTA);
        ValidateSensorReading(nominal, *temp);
        CPPUNIT_ASSERT_DOUBLES_EQUAL((float)0.5, elem.UpperNonCritical, DOUBLE_DELTA);
    }
} 


void CTestParseSensorReadingLine::testParseFanLine()
{
    char const * fanline = "Fan7A RPM        | 3360.000   | RPM        | ok    | na        | 720.000   | 840.000   | na        | na        | na";
    sensor_t elem;
    sensor_t * temp = &elem;
    DescrSensorEntity ref;

    {
        ParseSensorReadingLine(fanline, temp, ref);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(INVALID_FLOAT_READING, elem.UpperNonRecoverable, DOUBLE_DELTA);
    }

    {
        float const ref = (float)3200;
        float const nominal = IpmiSensorDescrToNominalVal(temp->SensorName);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(ref, nominal, DOUBLE_DELTA);
        ValidateSensorReading(nominal, *temp);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(ref+ref/4, elem.UpperNonCritical, DOUBLE_DELTA);
    }
}

extern void FillThresholdsWithRef(sensor_t * temp, sensor_thresholds_ref_t const & ref);
extern void ValidateCurrentValue(sensor_t * temp);
void CTestParseSensorReadingLine::testParseVoltageNALine()
{
    char const * voltageNALine = "Voltage 2        | na         | Volts      | na    | na        | na        | na        | na        | na        | na";
    sensor_t elem;
    sensor_t * temp = &elem;
    DescrSensorEntity ref;

    {
        ParseSensorReadingLine(voltageNALine, temp, ref);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(INVALID_FLOAT_READING, elem.CurrentVal, DOUBLE_DELTA);
    }

    {
        float const refVal = INVALID_FLOAT_READING;
        FillThresholdsWithRef(temp, ref.ref);
        CPPUNIT_ASSERT_DOUBLES_EQUAL(INVALID_FLOAT_READING, elem.CurrentVal, DOUBLE_DELTA);
        ValidateCurrentValue(temp);
        CPPUNIT_ASSERT_DOUBLES_EQUAL((float)0.0, elem.CurrentVal, DOUBLE_DELTA);
    }
}

