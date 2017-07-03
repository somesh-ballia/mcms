#include "TestGetIpmiList.h"
#include "sensor_read.h"
#include "CardContent.h"
#include <string.h>

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestGetIpmiList );

#define DOUBLE_DELTA 0.0000001

extern SensorReadFunc sensor_read;
static SensorReadFunc original_sensor_read;

void TestGetIpmiList::setUp()
{
    original_sensor_read = sensor_read;
}


void TestGetIpmiList::tearDown()
{
    sensor_read = original_sensor_read;
}

static int sensor_read_normal(vector<sensor_t> & readings)
{
    {
        sensor_t s = { "CPU Temp", 50.0, "degrees", "ok", 10.0, 20.0, 30.0, 60.0, 70.0, 80.0, EVENT_READ_TYPE_COLOR
            , { 10.0, 20.0, 30.0, 60.0, 70.0, 80.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Inlet Temp", 25.0, "degrees C", "ok", 0.0, 5.0, 10.0, 40.0, 60.0, 80.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 5.0, 10.0, 40.0, 60.0, 80.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "HDD Temp", 30.0, "degrees C", "ok", 0.0, 10.0, 20.0, 40.0, 60.0, 80.0, EVENT_READ_TYPE_COLOR
            , {0.0, 10.0, 20.0, 40.0, 60.0, 80.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Fan1A RPM", 3240.0, "RPM", "ok", 0.0, 1000.0, 2000.0, 5000.0, 6000.0, 7000.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 1000.0, 2000.0, 5000.0, 6000.0, 7000.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Pwr Consumption", 100.0, "Watts", "ok", 0.0, 20.0, 80.0, 200.0, 300.0, 400.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 20.0, 80.0, 200.0, 300.0, 400.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Current 2", 0.4, "Amps", "ok", 0.0, 0.1, 0.2, 0.5, 0.6, 1.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 0.1, 0.2, 0.5, 0.6, 1.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Voltage 2", 220.0, "Watts", "ok", 0.0, 50.0, 150.0, 300.0, 400.0, 500.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 50.0, 150.0, 300.0, 400.0, 500.0 } };
        readings.push_back(s);
    }

    return readings.size();
}
void TestGetIpmiList::testGetNormalStatusList()
{
    sensor_read = sensor_read_normal;
    vector<CardContent> cards;
    CollectCardContents(cards);

    CPPUNIT_ASSERT(!cards.empty());
    for (unsigned i=0; i<cards.size(); ++i)
    {
        //fprintf(stderr, "card %s: status=%d, temperature=%d, voltage=%d\n", cards[i].cardType, cards[i].status, cards[i].temperature, cards[i].voltage);
        CPPUNIT_ASSERT_EQUAL(0, int(cards[i].status));
        CPPUNIT_ASSERT_EQUAL(0, int(cards[i].temperature));
        CPPUNIT_ASSERT_EQUAL(0, int(cards[i].voltage));
    }
}

static int sensor_read_alert(vector<sensor_t> & readings)
{
    {
        sensor_t s = { "Inlet Temp", 81.0, "degrees C", "alert", 0.0, 5.0, 10.0, 40.0, 60.0, 80.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 5.0, 10.0, 40.0, 60.0, 80.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "HDD Temp", 30.0, "degrees C", "", 0.0, 10.0, 20.0, 40.0, 60.0, 80.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 10.0, 20.0, 40.0, 60.0, 80.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "CPU Usage", 999.0, "degrees C", "", 0.0, 1.0, 2.0, 950.0, 980.0, 990.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 1.0, 2.0, 950.0, 980.0, 990.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Fan1A RPM", 8240.0, "RPM", "ok", 0.0, 1000.0, 2000.0, 5000.0, 6000.0, 7000.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 1000.0, 2000.0, 5000.0, 6000.0, 7000.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Pwr Consumption", 100.0, "Watts", "ok", 0.0, 20.0, 80.0, 200.0, 300.0, 400.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 20.0, 80.0, 200.0, 300.0, 400.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Current 2", 0.4, "Amps", "na", 0.0, 0.1, 0.2, 0.5, 0.6, 1.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 0.1, 0.2, 0.5, 0.6, 1.0 } };
        readings.push_back(s);
    }

    {
        sensor_t s = { "Voltage 2", 310.0, "Watts", "ok", 0.0, 50.0, 150.0, 300.0, 400.0, 500.0, EVENT_READ_TYPE_COLOR
            , { 0.0, 50.0, 150.0, 300.0, 400.0, 500.0 } };
        readings.push_back(s);
    }

    return readings.size();
}
void TestGetIpmiList::testGetAlertStatusList()
{
    sensor_read = sensor_read_alert;
    vector<CardContent> cards;
    CollectCardContents(cards);

    CPPUNIT_ASSERT(!cards.empty());
    for (unsigned i=0; i<cards.size(); ++i)
    {
        //fprintf(stderr, "card %s: status=%d, temperature=%d, voltage=%d\n", cards[i].cardType, cards[i].status, cards[i].temperature, cards[i].voltage);
        if (0==strcmp("CNTL", cards[i].cardType))
        {
            CPPUNIT_ASSERT_EQUAL(1, int(cards[i].status));
            CPPUNIT_ASSERT_EQUAL(2, int(cards[i].temperature));
            CPPUNIT_ASSERT_EQUAL(0, int(cards[i].voltage));
        }
        else if (0==strcmp("FANS", cards[i].cardType))
        {
            CPPUNIT_ASSERT_EQUAL(1, int(cards[i].status));
            CPPUNIT_ASSERT_EQUAL(0, int(cards[i].temperature));
            CPPUNIT_ASSERT_EQUAL(0, int(cards[i].voltage));
        }
        else if (0==strcmp("PWR", cards[i].cardType))
        {
            CPPUNIT_ASSERT_EQUAL(1, int(cards[i].status));
            CPPUNIT_ASSERT_EQUAL(0, int(cards[i].temperature));
            CPPUNIT_ASSERT_EQUAL(1, int(cards[i].voltage));
        }
    }
}

