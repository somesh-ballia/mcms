#ifndef __TEST_PARSE_SENSOR_READING_LINE_H__
#define __TEST_PARSE_SENSOR_READING_LINE_H__

#include <cppunit/extensions/HelperMacros.h>
#include "GetMacAddr.h"

class CTestParseSensorReadingLine : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestParseSensorReadingLine );
    CPPUNIT_TEST( testParseNALine );
    CPPUNIT_TEST( testParseFanLine );
    CPPUNIT_TEST( testParseVoltageNALine );
    //...
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testParseNALine();
    void testParseFanLine();
    void testParseVoltageNALine();
};


#endif

