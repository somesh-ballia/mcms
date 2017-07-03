#include "ProcessSettings.h"
#include "TestProcessSettings.h"


CPPUNIT_TEST_SUITE_REGISTRATION( CTestProcessSettings );



void CTestProcessSettings::testSetGetProcessSettings()
{
    CProcessSettings procSettings1;
    procSettings1.SetSetting("key1", "data1", false);
    procSettings1.SetSettingDWORD("key2", 42, false);
    procSettings1.SetSettingBOOL("key3", TRUE, true);

    
    CProcessSettings procSettings2;
    string strData;
    bool res = procSettings2.GetSetting("key1", strData);
    CPPUNIT_ASSERT_MESSAGE("testSetGetProcessSettings: string", "data1" == strData && true == res);
    
    DWORD iData = 0;
    res = procSettings2.GetSettingDWORD("key2", iData);
    CPPUNIT_ASSERT_MESSAGE("testSetGetProcessSettings: DWORD", 42 == iData && true == res);

    BOOL bData = FALSE;
    res = procSettings2.GetSettingBOOL("key3", bData);
    CPPUNIT_ASSERT_MESSAGE("testSetGetProcessSettings: BOOL", TRUE == bData && true == res);

}

void CTestProcessSettings::testGetSetRemove()
{
    CProcessSettings procSetting;
    procSetting.SetSetting("key1", "data1");
    procSetting.SetSettingDWORD("key2", 42);
    procSetting.SetSettingBOOL("key3", TRUE);
    
    string data1 = "cucu-lulu";
    DWORD data2 = 2;
    BOOL data3 = FALSE;
    bool res = procSetting.GetSetting("key1", data1);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT("data1" == data1);
    
    res = procSetting.GetSettingDWORD("key2", data2);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(42 == data2);

    res = procSetting.GetSettingBOOL("key3", data3);
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(TRUE == data3);

    string dataNotExist = "not exist";
    res = procSetting.GetSetting("not exist key", dataNotExist);
    CPPUNIT_ASSERT(false == res);
    CPPUNIT_ASSERT("not exist" == dataNotExist);

    res = procSetting.RemoveSetting("key1",false);
    CPPUNIT_ASSERT(res);

    string removeData = "removedData";
    res = procSetting.GetSetting("key1", removeData);
    CPPUNIT_ASSERT(false == res);
    CPPUNIT_ASSERT("removedData" == removeData);
}

