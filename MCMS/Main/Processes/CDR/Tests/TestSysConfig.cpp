#include "TestSysConfig.h"
#include "SysConfig.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestSysConfig );


void CTestSysConfig::testSetGetSysConfig()
{
    CSysConfig* sysConfig = new CSysConfig();
    SystemSleep(10);

    DWORD strData = 0;    
    BOOL resStr = FALSE;
    sysConfig->GetDWORDDataByKey("key1", strData);
    
    DWORD iData = 0;
    BOOL resInt = FALSE;
    sysConfig->GetDWORDDataByKey("key2", iData);
    
    BOOL bData = FALSE;
    BOOL resBool = FALSE;
     sysConfig->GetBOOLDataByKey("key3", bData);
    
    CPPUNIT_ASSERT_MESSAGE("CTestSysConfig::testSetGetSysConfig", !resInt);
    CPPUNIT_ASSERT_MESSAGE("CTestSysConfig::testSetGetSysConfig", !resBool);
    CPPUNIT_ASSERT_MESSAGE("CTestSysConfig::testSetGetSysConfig", !resStr);

    BOOL isDebugMode = FALSE;
    resBool = FALSE;
    sysConfig->GetBOOLDataByKey("DEBUG_MODE", isDebugMode );

    CPPUNIT_ASSERT_MESSAGE("CTestSysConfig::testSetGetSysConfig", !resBool);   
    delete sysConfig;
}
