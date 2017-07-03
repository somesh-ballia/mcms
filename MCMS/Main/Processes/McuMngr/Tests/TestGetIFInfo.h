
#ifndef __TEST_GET_IFINFO_H__
#define __TEST_GET_IFINFO_H__

#include <cppunit/extensions/HelperMacros.h>

class TestGetIFInfo : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( TestGetIFInfo );
    CPPUNIT_TEST( testGetIFStatistics );
    CPPUNIT_TEST(testGetIFFeatures);
    CPPUNIT_TEST(testGetIFStatus);
    
    CPPUNIT_TEST_SUITE_END();
    
 public:
    void setUp();
    void tearDown();
    
    void testGetIFStatistics();
    void testGetIFFeatures();
    void testGetIFStatus();
};

#endif  // __TEST_GET_IFINFO_H__

