#ifndef __TEST_OPTION_MASK_ANALYZER_H__
#define __TEST_OPTION_MASK_ANALYZER_H__

#include <cppunit/extensions/HelperMacros.h>
#include "GetMacAddr.h"

class CTestOptionMaskAnalyzer : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestOptionMaskAnalyzer );
    CPPUNIT_TEST( testGetFeatures );
    CPPUNIT_TEST( testGetPartners );
    //...
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testGetFeatures();
    void testGetPartners();
};


#endif

