#ifndef __TEST_GET_ETH_BOND_STATUS_H__
#define __TEST_GET_ETH_BOND_STATUS_H__

#include <cppunit/extensions/HelperMacros.h>
#include "GetEthBondStatus.h"

class CTestGetEthBondStatus : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestGetEthBondStatus );
    CPPUNIT_TEST( testGetNormalStatus );
    CPPUNIT_TEST( testGetLinkStatus );
    CPPUNIT_TEST( testGetBadBondingStatus );
    //...
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testGetNormalStatus();
    void testGetLinkStatus();
    void testGetBadBondingStatus();
};


#endif

