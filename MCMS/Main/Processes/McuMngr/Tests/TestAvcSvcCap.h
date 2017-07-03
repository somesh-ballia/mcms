#ifndef __TEST_AVC_SVC_CAP_H__
#define __TEST_AVC_SVC_CAP_H__

#include <cppunit/extensions/HelperMacros.h>
class CLicensing;
#include "VersionStruct.h"

class CTestAvcSvcCap : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE( CTestAvcSvcCap );
    CPPUNIT_TEST( testGetCapSoft );
    CPPUNIT_TEST( testGetCapSoftMfw );
    CPPUNIT_TEST( testGetCapGesher );
    CPPUNIT_TEST( testGetCapHwLowVer );
    CPPUNIT_TEST( testGetCapHwLittleHighVer );
    CPPUNIT_TEST( testGetCapHwMuchHighVer );
    CPPUNIT_TEST( testGetFunc );
    //...
    CPPUNIT_TEST_SUITE_END();
public:

    void setUp();
    void tearDown(); 
    
    void testGetCapSoft();
    void testGetCapSoftMfw();
    void testGetCapGesher();
    void testGetCapHwLowVer();
    void testGetCapHwLittleHighVer();
    void testGetCapHwMuchHighVer();
    void testGetFunc();

private:
    void testGetCapHwWithHighVer(VERSION_S const & ver);

private:
    CLicensing * m_pLicensing;
};


#endif

