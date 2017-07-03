#include "TestIpmiFru.h"
#include "IpmiFru.h"
#include "GetCntlBoardGeneralInfo.h"
#include "copy_string.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestIpmiFru );

void CTestIpmiFru::setUp()
{
}

void CTestIpmiFru::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestIpmiFru::testGetFruResponse()
{
#if 0
    CntlBoardGeneralInfo info;
    GetCntlBoardGeneralInfo(info);

    IpmiFruInfo m_fruInfo;

    {
        CopyString(m_fruInfo.boardHardwareVers, info.hwver);
        CopyString(m_fruInfo.boardSerialNumber, info.serial);
        CopyString(m_fruInfo.boardPartNumber, info.partnumber);
    }
    printf("%s:%s:%d: board name %s, hwver %s, swver %s, serial %s, partnumber %s\n", __FILE__, __func__, __LINE__, info.name, info.hwver, info.swver, info.serial, info.partnumber);
    printf("%s:%s:%d: m_fruInfo: board name %s, hwver %s, swver %s, serial %s, partnumber %s\n"
            , __FILE__, __func__, __LINE__
            , m_fruInfo.boardProductName
            , m_fruInfo.boardHardwareVers
            , "N/A"
            , m_fruInfo.boardSerialNumber
            , m_fruInfo.boardPartNumber
          );
#endif
} 


