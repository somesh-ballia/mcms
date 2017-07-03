#ifndef __TEST_IPMI_FRU_H__
#define __TEST_IPMI_FRU_H__

#include <cppunit/extensions/HelperMacros.h>
#include "GetMacAddr.h"

class CTestIpmiFru : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestIpmiFru );
    CPPUNIT_TEST( testGetFruResponse );
    //...
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testGetFruResponse();
};


#endif

