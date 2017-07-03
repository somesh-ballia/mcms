#ifndef __TEST_GET_MAC_ADDR_H__
#define __TEST_GET_MAC_ADDR_H__

#include <cppunit/extensions/HelperMacros.h>
#include "GetMacAddr.h"

class CTestGetMacAddr : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestGetMacAddr );
    CPPUNIT_TEST( testGetMacWithoutSemiColon );
    CPPUNIT_TEST( testGetMacWithSemiColon );
    //...
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testGetMacWithoutSemiColon();
    void testGetMacWithSemiColon();
};


#endif

