#ifndef __TEST_GET_CNTL_BOARD_GENERAL_INFO_H__
#define __TEST_GET_CNTL_BOARD_GENERAL_INFO_H__

#include <cppunit/extensions/HelperMacros.h>
#include "GetMacAddr.h"

class CTestGetCntlBoardGeneralInfo : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestGetCntlBoardGeneralInfo );
    CPPUNIT_TEST( testGetBoardInfo );
    //...
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testGetBoardInfo();
};


#endif

