#ifndef __TEST_MISC_FUNCTIONS_H__
#define __TEST_MISC_FUNCTIONS_H__

#include <cppunit/extensions/HelperMacros.h>
#include "GetMacAddr.h"

class CTestMiscFunctions : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestMiscFunctions );
    CPPUNIT_TEST( testStripLeft );
    CPPUNIT_TEST( testStripRight );
    CPPUNIT_TEST( testStripBoth );
    //...
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testStripLeft();
    void testStripRight();
    void testStripBoth();
};


#endif

