#include "TestGetCntlBoardGeneralInfo.h"
#include "GetCntlBoardGeneralInfo.h"
#include "Trace.h"

CPPUNIT_TEST_SUITE_REGISTRATION( CTestGetCntlBoardGeneralInfo );

void CTestGetCntlBoardGeneralInfo::setUp()
{
}

void CTestGetCntlBoardGeneralInfo::tearDown()
{
}

//////////////////////////////////////////////////////////////////////
void CTestGetCntlBoardGeneralInfo::testGetBoardInfo()
{
#if 0
    CntlBoardGeneralInfo info;
    GetCntlBoardGeneralInfo(info);
    printf("%s:%s:%d: board name %s, hwver %s, swver %s, serial %s, partnumber %s\n", __FILE__, __func__, __LINE__, info.name, info.hwver, info.swver, info.serial, info.partnumber);
#endif
} 


